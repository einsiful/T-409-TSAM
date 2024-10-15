CXX = g++
CXXFLAGS = -std=c++11 -pthread

# Target to build everything
all: tsamgroup30 client

# Target for the server
tsamgroup30: main.o Server.o SocketHandler.o ClientClass.o MessageHandler.o Logger.o
	$(CXX) $(CXXFLAGS) -o tsamgroup30 main.o Server.o SocketHandler.o ClientClass.o MessageHandler.o Logger.o

# Target for the client
client: client.o MessageHandler.o
	$(CXX) $(CXXFLAGS) -o client client.o MessageHandler.o

# Object file compilation for each source file
main.o: main.cpp Server.h
	$(CXX) $(CXXFLAGS) -c main.cpp

Server.o: Server.cpp Server.h SocketHandler.h Client.h Logger.h MessageHandler.h
	$(CXX) $(CXXFLAGS) -c Server.cpp

SocketHandler.o: SocketHandler.cpp SocketHandler.h
	$(CXX) $(CXXFLAGS) -c SocketHandler.cpp

ClientClass.o: ClientClass.cpp Client.h
	$(CXX) $(CXXFLAGS) -c ClientClass.cpp

MessageHandler.o: MessageHandler.cpp MessageHandler.h
	$(CXX) $(CXXFLAGS) -c MessageHandler.cpp

Logger.o: Logger.cpp Logger.h
	$(CXX) $(CXXFLAGS) -c Logger.cpp

client.o: client.cpp MessageHandler.h
	$(CXX) $(CXXFLAGS) -c client.cpp

# Clean up command
clean:
	rm -f *.o tsamgroup30 client server_log.txt client_log.txt

send:
# 	scp * einararn22_2024@130.208.246.249:~/T-409-TSAM
# 	scp Makefile einararn22_2024@130.208.246.249:~/T-409-TSAM
	scp * danieli22_2024@130.208.246.249:~/T-409-TSAM
	scp Makefile danieli22_2024@130.208.246.249:~/T-409-TSAM
