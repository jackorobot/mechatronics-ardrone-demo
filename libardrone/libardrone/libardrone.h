#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <string>

using namespace std;

namespace libardrone{
	class ARDrone{
	public:
		//Constructor and Deconstructor
		ARDrone();
		~ARDrone();

		void recvData();

		//Movement functions
		void hover();
		void takeOff();
		void land();
		void ftrim();

	private:
		//Sequence counter
		unsigned int seq_cnt;

		//Socket connection
		SOCKET scommands;
		WSADATA wsacommands;
		struct sockaddr_in commands;
		void openSocket();

		SOCKET snavdata;
		WSADATA wsanavdata;
		struct sockaddr_in navdata;

		SOCKET svideo;
		WSADATA wsavideo;
		struct sockaddr_in video;

		//Communications
		void AT(string command);
	};
}