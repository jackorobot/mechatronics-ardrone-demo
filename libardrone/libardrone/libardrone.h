#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <string>
#include <thread>
#include <map>


namespace libardrone{
	class ARDrone{
	public:
		//Constructor and Deconstructor
		ARDrone();
		~ARDrone();

		typedef std::map<std::string, int> State;
		struct Demo {
			int ctrl_state;
			int vbat_flying;
			float theta;
			float phi;
			float psi;

			int altitude;

			float vx;
			float vy;
			float vz;
			
			int num_frames;
		};
		struct Nav {
			int header;
			State drone_state;
			int seq_nr;
			int vision_flag;
			Demo navdata;
		};
		Nav navdata;

		//Movement functions
		void hover();
		void takeOff();
		void land();

		//Functional commands
		void ftrim();
		void reset();
		void setEmergency();
		void resetEmergency();

	private:
		bool stopping;

		//Sequence counter
		unsigned int seq_cnt;

		//Socket connection
		SOCKET scommands;
		WSADATA wsacommands;
		struct sockaddr_in addrcommands;
		void openSocket();

		SOCKET snavdata;
		WSADATA wsanavdata;
		struct sockaddr_in addrnavdata;
		std::thread recvNavdata;

		SOCKET svideo;
		WSADATA wsavideo;
		struct sockaddr_in addrvideo;
		std::thread recvVideo;
		
		//Communications
		void AT(std::string command);

		void setupRecv();
		void listenData();
		void listenVideo();
		void listenNavdata();
		void checkConnection();

		//Decoding data
		void decodeVideo(char *data, int len);
		void decodeNavdata(char *data, int len);
	};
}