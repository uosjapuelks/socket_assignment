CXX=gcc
CXXFLAGS:=-Wall
RELEASEFLAGS:=-O3
DEBUGFLAGS:=-g

.PHONY: all clean
all: server client

server: udp_ser.o
	$(CXX) $(CXXFLAGS) -o udpServer $^

client: udp_client.o
	$(CXX) $(CXXFLAGS) -o udpClient $^

udpServer.o: udp_ser.c
	$(CXX) $(CXXFLAGS) -c $^

udpClient.o: udp_client.c
	$(CXX) $(CXXFLAGS) -c $^

clean:
	$(RM) *.o udpServer udpClient

# debug: main.cc
# 	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -D DEBUG -o tcpClient main.cc