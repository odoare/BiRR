PROJUCER= ~/bin/Projucer

all: projects birr2dm birr2ds birr3dm birr3ds birr3das birr

projects:
	$(PROJUCER) --resave BiRR2D_monoin/BiRR2D_monoin.jucer
	$(PROJUCER) --resave BiRR2D_stereoin/BiRR2D_stereoin.jucer
	$(PROJUCER) --resave BiRR3D_monoin/BiRR3D_monoin.jucer
	$(PROJUCER) --resave BiRR3D_stereoin/BiRR3D_stereoin.jucer
	$(PROJUCER) --resave BiRR3DA_stereoin/BiRR3DA_stereoin.jucer
	$(PROJUCER) --resave BiRR/BiRR.jucer

birr2dm:
	cd BiRR2D_monoin/Builds/LinuxMakefile && make && cd ../../..

birr2ds:
	cd BiRR2D_stereoin/Builds/LinuxMakefile && make && cd ../../..

birr3dm:
	cd BiRR3D_monoin/Builds/LinuxMakefile && make && cd ../../..

birr3ds:
	cd BiRR3D_stereoin/Builds/LinuxMakefile && make && cd ../../..

birr3das:
	cd BiRR3DA_stereoin/Builds/LinuxMakefile && make && cd ../../..

birr:
	cd BiRR/Builds/LinuxMakefile && make && cd ../../..
