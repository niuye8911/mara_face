CXX = g++-7

RSDGPREFIX =/home/liuliu/Research/rapidlib-linux/cppSource 
CXXFLAGS += -w $(shell pkg-config --cflags opencv) -std=c++11
LDFLAGS += $(shell pkg-config --libs --static opencv) -lcurl -pthread -ljsoncpp
INCLUDE = -I/usr/local/include/opencv2 -I$(RSDGPREFIX) -I/usr/include/curl

OBJ = face_main.o
all: facedetect

facedetect: $(OBJ) /home/liuliu/Research/rapidlib-linux/cppSource/build/rsdg.a 
	 $(CXX) $(CXXFLAGS) $(OBJ) /home/liuliu/Research/rapidlib-linux/cppSource/build/rsdg.a -o $@ $(LDFLAGS) $(INCLUDE)

.cpp.o:  
	$(CXX) $(CXXFLAGS) -c $*.cpp $(INCLUDE) -o $*.o

clean: ; rm -f face_main.o opencv_example
