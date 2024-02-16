// ==================================================================================
// Copyright (c) 2017 HiFi-LoFi
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is furnished
// to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// ==================================================================================

#ifndef __HIFI_LOFI_SQUEEZE_H__
#define __HIFI_LOFI_SQUEEZE_H__

#include "JuceHeader.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <vector>


namespace squeeze
{
	
  namespace details
  {

    /**
     * @internal
     * @class ParallelForFrame
     * @brief State shared by all threads working at a parallel for loop
     */
    template<typename I, typename F>
    class ParallelForFrame
    {
    public:
      ParallelForFrame(const I& begin, const I& end, const size_t chunkSize, F&& func) :
        _begin(begin),
        _end(end),
        _iterations(_end - _begin),
        _chunkSize(chunkSize),
        _func(std::forward<F>(func)),
        _startedCount(0),
        _completedCount(0),
        _completedMutex(),
        _completedCondition(),
        _completed(false),
        _numberStartedThreads(0)
      {
      }

      ParallelForFrame(const ParallelForFrame&) = delete;
      ParallelForFrame& operator=(const ParallelForFrame&) = delete;

      void runLoop()
      {
        ++_numberStartedThreads;
        for (;;)
        {
          if (loopOneChunk())
          {
            break;
          }
        }
      }

      /**
       * @brief Loops one chunk of iterations
       * @return Flag whether the loop is completed
       */
      bool loopOneChunk()
      {
        // What we're doing here is maybe best described as "chunk stealing":
        // Every thread which wants to process a loop chunk just increments
        // the started counter and "steals" the next chunk of iterations.
        const size_t indexBegin = _startedCount.fetch_add(_chunkSize);
        if (indexBegin >= _iterations)
        {
          // Nothing to loop anymore => We're done.
          return true;
        }
        const size_t iterations = std::min(_chunkSize, _iterations - indexBegin);
        I it = _begin + indexBegin;
        const I itEnd = it + iterations;
        while (it != itEnd)
        {
          _func(it);
          ++it;
        }

        // Check whether we've completed the loop
        if (_completedCount.fetch_add(iterations) + iterations == _iterations)
        {
          std::unique_lock<std::mutex> lock(_completedMutex);
          _completed = true;
          _completedCondition.notify_all();
          return true;
        }

        // The loop isn't completed yet
        return false;
      }

      void waitForFinished()
      {
        if (!_completed)
        {
          std::unique_lock<std::mutex> lock(_completedMutex);
          while (!_completed)
          {
            _completedCondition.wait(lock);
          }
        }
      }

      size_t getCompletedCount() const
      {
        return _completedCount.load();
      }

      size_t getNumberStartedThreads() const
      {
        return _numberStartedThreads.load();
      }

    private:
      const I _begin;
      const I _end;
      const size_t _iterations;
      const size_t _chunkSize;
      const F _func;
      std::atomic<size_t> _startedCount;
      std::atomic<size_t> _completedCount;
      std::mutex _completedMutex;
      std::condition_variable _completedCondition;
      bool _completed;
      std::atomic<size_t> _numberStartedThreads;
    };


    /**
     * @internal
     * @class ParallelForJob
     * @brief Thread pool job for parallel loops
     */
    template<typename I, typename F>
    class ParallelForJob : public juce::ThreadPoolJob
    {
    public:
      explicit ParallelForJob(ParallelForFrame<I, F>& frame) :
        juce::ThreadPoolJob("ParallelForJob"),
        _frame(frame)
      {
      }

      ParallelForJob(const ParallelForJob&) = delete;
      ParallelForJob& operator=(const ParallelForJob&) = delete;

      virtual JobStatus runJob() override
      {
        _frame.runLoop();
        return jobHasFinished;
      }

    private:
      ParallelForFrame<I, F>& _frame;

    };


    // size_t ProposeLoopChunkSize(size_t iterations, size_t numberThreads)
    // {
    //   // The chunk size is a very important parameter for tuning performance:
    //   // - Too small chunk size: Good work balance, potentially much overhead.
    //   // - Too large chunk size: Potentially bad work balance, not so much overhead.
    //   // This is the heuristically found compromise which seemed to work very well in
    //   // most use cases and for current common "consumer level" machines (2 to 20 cores).
    //   const size_t chunksPerThread = std::max<size_t>(1, 2 * numberThreads);
    //   const size_t numberChunks = chunksPerThread * numberThreads;
    //   const size_t chunkSize = (iterations + numberChunks - 1) / numberChunks;
    //   return std::min<size_t>(chunkSize, 8192);
    // }

    
    template<typename F>
    void AppendParallelTasks(std::vector<std::function<void()>>& tasks, F&& func)
    {
      tasks.emplace_back(std::forward<F>(func));
    }


    template<typename F, typename ...Fs>
    void AppendParallelTasks(std::vector<std::function<void()>>& tasks, F&& func, Fs... funcs)
    {
      tasks.emplace_back(std::forward<F>(func));
      AppendParallelTasks(tasks, std::forward<Fs>(funcs)...);
    }

  } // End of namespace details


  //=============================================================================


  /**
   * @brief Runs a loop body function in parallel using a given thread pool
   * @param threadPool Thread pool to be used for parallelizing
   * @param begin Begin of the loop (start index or iterator)
   * @param end End of the loop (end index or iterator)
   * @param chunkSize Chunk size for performance fine-tuning (trade off between good work balancing and even usage of CPU cores)
   * @param func The loop body function
   */
  template<typename I, typename F>
  void ParallelFor(juce::ThreadPool& threadPool, I begin, I end, size_t chunkSize, F&& func)
  {
    // Make sure that there's a reasonable chunk size
    jassert(chunkSize > 0);
    chunkSize = std::max(size_t(1), chunkSize);

    // Don't parallelize the loop if there are not enough iterations
    const size_t iterations = static_cast<size_t>(end - begin);
    const size_t maxNumberThreads = static_cast<size_t>(threadPool.getNumThreads());
    if (iterations <= chunkSize)
    {
      for (; begin != end; ++begin)
      {
        func(begin);
      }
      return;
    }

    // Parallelize the loop
    details::ParallelForFrame<I, F> frame(begin, end, chunkSize, std::forward<F>(func));
    const size_t numberChunks = iterations / chunkSize;
    const size_t desiredNumberThreads = std::min(maxNumberThreads, numberChunks);
    std::vector<std::unique_ptr<details::ParallelForJob<I, F>>> jobs;
    jobs.reserve(desiredNumberThreads);
    for (size_t i = 0; i < desiredNumberThreads; ++i)
    {
      jobs.emplace_back(std::unique_ptr<details::ParallelForJob<I, F>>(new details::ParallelForJob<I, F>(frame)));
      threadPool.addJob(jobs.back().get(), false);

      // Stop creating new threads if there's already good progress
      // and enough threads for the remaining iterations
      const size_t remaining = iterations - frame.getCompletedCount();
      if (remaining <= (i + 1) * chunkSize)
      {
        break;
      }
    }

    // The current thread will help with looping as long as there's not at least one
    // thread pool job actually running, otherwise we might get deadlocked
    // if the thread pool is full of jobs waiting for this loop which doesn't
    // proceed because its loop jobs aren't run. However, we stop looping in this thread
    // as soon as we notice that there's at least one loop job running in order to avoid
    // over-subscription as much as possible.
    while (frame.getNumberStartedThreads() == 0)
    {
      if (frame.loopOneChunk())
      {
        // The loop is completed now
        break;
      }
    }
    
    frame.waitForFinished();
    for (auto& job : jobs)
    {
      threadPool.removeJob(job.get(), false, std::numeric_limits<int>::max());
    }
  }


  // /**
  //  * @brief Runs a loop body function in parallel using a given thread pool
  //  * @param threadPool Thread pool to be used for parallelizing
  //  * @param begin Begin of the loop (start index or iterator)
  //  * @param end End of the loop (end index or iterator)
  //  * @param func The loop body function
  //  */
  // template<typename I, typename F>
  // void ParallelFor(juce::ThreadPool& threadPool, I begin, I end, F&& func)
  // {
  //   const size_t iterations = end - begin;
  //   const size_t maxNumberThreads = static_cast<size_t>(threadPool.getNumThreads());
  //   const size_t chunkSize = details::ProposeLoopChunkSize(iterations, maxNumberThreads);
  //   ParallelFor(threadPool, begin, end, chunkSize, std::forward<F>(func));
  // }


  /**
   * @brief Runs given tasks in parallel using the given thread pool
   * @remark When this function returns, all tasks have been executed already in arbitrary order.
   * @param threadPool Thread pool to be used for parallelizing
   * @param funcs Arbitrary number of tasks to be run in parallel
   */
  template<typename ...Fs>
  void ParallelInvoke(juce::ThreadPool& threadPool, Fs... funcs)
  {
    std::vector<std::function<void()>> tasks;
    details::AppendParallelTasks(tasks, std::forward<Fs>(funcs)...);
    const size_t chunkSize = 1; // We assume that the parallel tasks have a reasonable workload (otherwise parallelizing wouldn't make sense at all)
    ParallelFor(threadPool, size_t(0), tasks.size(), chunkSize, [&](size_t idx)
    {
      tasks[idx]();
      tasks[idx] = nullptr;
    });
  }


  /**
  * @brief Runs simply the given task (special handling for making ParallelInvoke() with only one tasks working)
  * @param threadPool Thread pool to be used for parallelizing
  * @param func Task to be run
  */
  template<typename F>
  void ParallelInvoke(juce::ThreadPool& threadPool, F&& func)
  {
    (void)threadPool; // Just to avoid compiler warning...
    func();
  }
  
} // End of namespace

#endif // Header guard
