all: client tsamgroup30

client: client.cpp
	g++ -std=c++11 client.cpp -o $@

tsamgroup30: server.cpp
	g++ -std=c++11 server.cpp -o $@

clean:
	rm -f client tsamgroup30

# send:
# 	scp src/* danieli22_2024@130.208.246.249:~/botnet/src
# 	scp Makefile danieli22_2024@130.208.246.249:~/botnet
