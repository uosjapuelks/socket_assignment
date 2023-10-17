# tcp_client1, tcp_ser1, udp_client1, udp_ser1

CXX=gcc
CXXFLAGS:=-Wall
RELEASEFLAGS:=-O3
DEBUGFLAGS:=-g

.PHONY: all clean
all: submission

submission: udp_ser1.o
	$(CXX) $(CXXFLAGS) -o udpServer $^

main.o: udp_ser1.c
	$(CXX) $(CXXFLAGS) -c $^

clean:
	$(RM) *.o tcpClient

debug: main.cc
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -D DEBUG -o tcpClient main.cc