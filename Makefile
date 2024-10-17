all: client tsamgroup30

client: client.cpp MessageHandler.cpp
	g++ -std=c++11 client.cpp MessageHandler.cpp -o $@

tsamgroup30: main.cpp server.cpp SocketHandler.cpp ClientClass.cpp MessageHandler.cpp Logger.cpp
	g++ -std=c++11 main.cpp server.cpp SocketHandler.cpp ClientClass.cpp MessageHandler.cpp Logger.cpp -o $@

clean:
	rm -f client tsamgroup30 server_log.txt client_log.txt

send:
# 	scp * einararn22_2024@130.208.246.249:~/T-409-TSAM
# 	scp Makefile einararn22_2024@130.208.246.249:~/T-409-TSAM
#	scp * danieli22_2024@130.208.246.249:~/T-409-TSAM
	scp Makefile danieli22_2024@130.208.246.249:~/T-409-TSAM