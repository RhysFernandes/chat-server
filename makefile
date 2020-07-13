server:
	g++ -std=c++14 -pthread *.cpp -o server

clean:
	rm server
