#include "libardrone.h"
#include <iostream>
#include <sstream>
#include <exception>
#include <thread>

#pragma comment(lib,"ws2_32.lib") //winsock library

using namespace libardrone;

ARDrone::ARDrone(){
	cout << "ARDrone Object created" << endl;
	seq_cnt = 0;

	thread recv(&ARDrone::recvData, this);

	commands.sin_addr.s_addr = inet_addr("192.168.1.1");
	commands.sin_family = AF_INET;
	commands.sin_port = htons(5556);
	openSocket();
}


ARDrone::~ARDrone(){
	
}

void ARDrone::recvData(){
	
}

void ARDrone::openSocket(void){
	cout << "Initializing Winsock" << endl;
	if (WSAStartup(MAKEWORD(2, 2), &wsacommands) != 0){
		cerr << "Failed. Error Code: " << WSAGetLastError() << endl;
		terminate();
	}
	cout << "Initialised" << endl;
	if ((scommands = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET){
		cerr << "Could not create socket: " << WSAGetLastError() << endl;
		terminate();
	}
	cout << "Socket created" << endl;
	if (connect(scommands, (struct sockaddr *)&commands, sizeof(commands)) < 0){
		cerr << "Connect error" << endl;
		terminate();
	}
	cout << "Connected" << endl;
}

void ARDrone::hover(){
	stringstream ss;
	ss << "AT*PCMD=" << seq_cnt << ",0,0,0,0,0,0,0\r";
	string s = ss.str();
	AT(s);
}

void ARDrone::takeOff(){
	int value = 0;
	value = value | (1 << 9);
	stringstream ss;
	ss << "AT*REF=" << seq_cnt << "," << value << "\r";
	string s = ss.str();
	AT(s);
}

void ARDrone::land(){
	int value = 0;
	value = value | (0 << 9);
	stringstream ss;
	ss << "AT*REF=" << seq_cnt << "," << value << "\r";
	string s = ss.str();
	AT(s);
}

void ARDrone::ftrim(){
	stringstream ss;
	ss << "AT*FTRIM=" << seq_cnt << ",\r";
	string s = ss.str();
	AT(s);
}

void ARDrone::AT(string command){
	cout << "Sending command: " << command << endl;
	char *cmd = const_cast<char*>(command.c_str());
	if (send(scommands, cmd, strlen(cmd), 0) < 0){
		cerr << "Send Failed" << endl;
		terminate();
	}
	seq_cnt += 1;
}