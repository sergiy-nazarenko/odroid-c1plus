CC=g++
GUI_CPPFLAGS=-D_GLIBCXX_USE_CXX11_ABI=0 -Dexternal_module_EXPORTS -I/home/odroid/usr/include -I/usr/include/python2.7 -Os -DNDEBUG -fPIC -fvisibility=hidden -std=c++14 -flto -fno-fat-lto-objects
CPPFLAGS=-std=c++14 -D_GLIBCXX_USE_CXX11_ABI=0
LINKFLAGS=-lstdc++ -pthread -lpthread -lrt 
LDFLAGS=-lwiringPi -lwiringPiDev  -flto -lpthread -pthread -lrt


vectra.o: vectra.h vectra.cpp
	g++ -c ${CPPFLAGS} -fPIC vectra.cpp -o build/vectra.o

vectra_nrf: vectra_nrf.o vectra.o
	g++ -o vectra_nrf build/vectra.o build/vectra_nrf.o ${LINKFLAGS} 


vectra_nrf.o: vectra_nrf.cpp vectra.o
	g++ -c ${CPPFLAGS} vectra_nrf.cpp -o build/vectra_nrf.o


vectra_can: vectra_can.o vectra.o
	gcc -o vectra_can build/vectra.o build/vectra_can.o ${LINKFLAGS}


vectra_can.o: vectra_nrf.cpp
	gcc -c vectra_can.cpp -o build/vectra_can.o


vectra_gui.o: vectra_gui.cpp vectra.o
	${CC} ${GUI_CPPFLAGS} -o build/vectra_gui.o -c vectra_gui.cpp


vectra_gui: vectra_gui.o vectra.o
	${CC} -fPIC -Os -DNDEBUG -shared -o vectra_gui.so build/vectra.o build/vectra_gui.o -flto -lpthread -pthread -lrt  && strip vectra_gui.so


cdbm: cd4051bm.o
        ${CC} -fPIC -Os -DNDEBUG -shared -o cd4051bm.so build/cd4051bm.o ${LDFLAGS}  && strip cd4051bm.so


cd4051bm.o: cd4051bm.cpp
        ${CC} ${GUI_CPPFLAGS} -o build/cd4051bm.o -c cd4051bm.cpp

all: vectra_nrf vectra_can vectra_gui cdbm


