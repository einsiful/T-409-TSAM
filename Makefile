all: client tsamgroup30

client: client.cpp
	g++ -std=c++11 client.cpp -o $@

tsamgroup30: tsamgroup30.cpp
	g++ -std=c++11 tsamgroup30.cpp -o $@

clean:
	rm -f client tsamgroup30
