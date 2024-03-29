all: Data_Receiver

Data_Receiver: packet_structures.h Data_Receiver.cpp 
	g++ Data_Receiver.cpp -o client.a -I.