server:
	g++ -std=c++14 -pthread -g *.cpp -o server

clean:
	rm server
