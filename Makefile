all: client tsamgroup30

client: client.cpp
	g++ -std=c++11 client.cpp -o $@

tsamgroup30: server.cpp
	g++ -std=c++11 server.cpp tokenizer.cpp -o $@

clean:
	rm -f client tsamgroup30

send:
 	scp * einararn22_2024@130.208.246.249:~/Documents/GitHub/T-409-TSAM
 	scp Makefile einararn22_2024@130.208.246.249:~/Documents/GitHub/T-409-TSAM
#	scp * danieli22_2024@130.208.246.249:~/T-409-TSAM
#	scp Makefile danieli22_2024@130.208.246.249:~/T-409-TSAM
