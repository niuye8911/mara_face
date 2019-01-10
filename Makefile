CXX ?= g++

RSDGPREFIX =/home/liuliu/Research/rapidlib-linux/cppSource/build 
CXXFLAGS += -c -Wall $(shell pkg-config --cflags opencv) -std=c++11
LDFLAGS += $(shell pkg-config --libs --static opencv) -lcurl -pthread
INCLUDE = -I/usr/local/include/opencv2 -I/home/liuliu/Research/rapidlib-linux/cppSource/ -I/usr/include/curl

all: facedetect

facedetect: example.o 
	 $(CXX) -g $< /home/liuliu/Research/rapidlib-linux/cppSource/build/rsdg.a -o $@ $(LDFLAGS) $(INCLUDE)

%.o: %.cpp
	 $(CXX) $< -g -o $@ $(CXXFLAGS) $(INCLUDE) $(LDFLAGS)

clean: ; rm -f example.o opencv_example
