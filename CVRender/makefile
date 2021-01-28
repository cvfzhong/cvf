DOPENCV=/usr/local/include/
DCVF=../
DLIBX=/fan/local/Re3D/
DASSIMP=$(DLIBX)/assimp-4.1.0/

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/x86_64-linux-gnu

CFLAGS=-DFINTEGER=int -fPIC -Wall -g -msse4  -O3
LDFLAGS=-fPIC -Wall -g -msse4  
#LIBS=-l glog -l gflags -l boost_system -lboost_filesystem -l pthread -l glut -L /usr/lib/x86_64-linux-gnu -l:/usr/lib/x86_64-linux-gnu/libOpenGL.so -l:libEGL.so.1.1.0 -l GLU -L $(DASSIMP)/lib/ -l assimp -lX11 -lGLX
LIBS=-l glog -l gflags -l boost_system -lboost_filesystem -l pthread -l glut -lOpenGL -l:libEGL.so.1.1.0 -l GLU -L $(DASSIMP)/lib/ -l assimp -lX11 -lGLX

all: test

test: _cvrender.o cvrender.o cvrm.o test.o dllmain.o
	g++ -o $@ $^  $(shell pkg-config --libs opencv) $(LDFLAGS)  $(LIBS) $(CFLAGS)

testx: testEGL.o
	g++ -o $@ $^  $(shell pkg-config --libs opencv) $(LDFLAGS)  $(CFLAGS) -L /usr/lib/x86_64-linux-gnu -l:/usr/lib/x86_64-linux-gnu/libOpenGL.so -l:libEGL.so.1.1.0
	
%.o:./%.cpp
	g++ -c $< -I$(DOPENCV) -I$(DCVF) -I$(DASSIMP)/include/ -I$(DLIBX)/glm-0.9.6.3/glm/ -w -fpermissive -std=c++11 $(CFLAGS) 

clean:
	rm _cvrender.o cvrender.o cvrm.o test.o dllmain.o

