PROJUCER= ~/bin/Projucer

all: projects birr ambirr

projects:
	$(PROJUCER) --resave BiRR/BiRR.jucer
	$(PROJUCER) --resave AmbiRR/AmbiRR.jucer

birr:
	cd BiRR/Builds/LinuxMakefile && make && cd ../../..

ambirr:
	cd AmbiRR/Builds/LinuxMakefile && make && cd ../../..
