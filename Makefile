CPPFLAGS += -I./include
AMFLAGS += -lstdc++

all:    udp.o data.o client.o server.o
	-
clean:
	rm -rf *.o

run:
	./server.o > s.txt&
	ps -o vsz,rss -C server.o > mem.txt
	./client.o test.dat > c1.txt&
	./client.o test.dat > c2.txt&
	./client.o test.dat > c3.txt&
	./client.o test.dat > c4.txt&
	./client.o test.dat > c5.txt&
	ps -o vsz,rss -C server.o >> mem.txt
	sleep 3
	ps -o vsz,rss -C server.o >> mem.txt
	cat s.txt | more
	sleep 2
	cat c1.txt | more
	sleep 2
	cat c2.txt | more
	sleep 2
	cat c3.txt | more
	sleep 2
	cat c4.txt | more
	sleep 2
	cat c5.txt | more
	sleep 2
	cat mem.txt | more
	
install:

client.o: client/client.cpp 
	g++ $(CPPFLAGS) -g3 -std=c++11 -o client.o udp.o data.o client/client.cpp $(AMFLAGS)

server.o: server/server.cpp 
	g++ $(CPPFLAGS) -g3 -std=c++11 -o server.o udp.o data.o server/server.cpp $(AMFLAGS)

udp.o: classes/udp.cpp 
	g++ $(CPPFLAGS) -c -g3 -std=c++11 -o udp.o classes/udp.cpp $(AMFLAGS)

data.o: classes/data.cpp 
	g++ $(CPPFLAGS) -c -g3 -std=c++11 -o data.o classes/data.cpp $(AMFLAGS)

