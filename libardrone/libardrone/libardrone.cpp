#include "libardrone.h"
#include <iostream>
#include <sstream>
#include <exception>

#pragma comment(lib,"ws2_32.lib") //winsock library

using namespace libardrone;

ARDrone::ARDrone(){
	std::cout << "ARDrone Object created" << std::endl;
	seq_cnt = 0;
	stopping = false;

	State drone_state;
	drone_state["fly_mask"] = 0;
	drone_state["video_mask"] = 0;
	drone_state["vision_mask"] = 0;
	drone_state["control_mask"] = 0;
	drone_state["altitude_mask"] = 0;
	drone_state["user_feedback_start"] = 0;
	drone_state["command_mask"] = 0;
	drone_state["fw_file_mask"] = 0;
	drone_state["fw_ver_mask"] = 0;
	drone_state["fw_upd_mask"] = 0;
	drone_state["navdata_demo_mask"] = 0;
	drone_state["navdata_bootstrap"] = 0;
	drone_state["motors_mask"] = 0;
	drone_state["com_lost_mask"] = 0;
	drone_state["vbat_low"] = 0;
	drone_state["user_el"] = 0;
	drone_state["timer_elapsed"] = 0;
	drone_state["angles_out_of_range"] = 0;
	drone_state["ultrasound_mask"] = 0;
	drone_state["cutout_mask"] = 0;
	drone_state["pic_version_mask"] = 0;
	drone_state["atcodec_thread_on"] = 0;
	drone_state["navdata_thread_on"] = 0;
	drone_state["video_thread_on"] = 0;
	drone_state["acq_watchdog_on"] = 0;
	drone_state["ctrl_watchdog_on"] = 0;
	drone_state["adc_watchdog_mask"] = 0;
	drone_state["com_watchdog_mask"] = 0;
	drone_state["emergency_mask"] = 0;
	navdata.drone_state = drone_state;
	navdata.header = 0;
	navdata.seq_nr = 0;
	navdata.vision_flag = 0;

	setupRecv();
	openSocket();
	listenData();
}


ARDrone::~ARDrone(){
	stopping = true;
	if(recvNavdata.joinable())
		recvNavdata.join();
	if (recvVideo.joinable())
		recvVideo.join();
	closesocket(scommands);
	closesocket(snavdata);
	closesocket(svideo);
	WSACleanup();
}

void ARDrone::listenData(){
	std::cout << "Start listening for video and navdata" << std::endl;
	listen(svideo, 10);
	listen(snavdata, 10);

	recvVideo = std::thread(&ARDrone::listenVideo, this);
	recvNavdata = std::thread(&ARDrone::listenNavdata, this);
}

void ARDrone::listenVideo(){
	int c = sizeof(struct sockaddr_in);
	SOCKET clientsocket;
	struct sockaddr_in client;
	int data_size;
	char data[65535];
	while (!stopping){
		clientsocket = accept(svideo, (struct sockaddr *)&client, &c);
		if ((data_size = recv(svideo, data, 65535, 0)) == SOCKET_ERROR){
			std::cerr << "Video receive failed" << std::endl;
		}
		else{
			decodeVideo(data, data_size);
		}
	}
}

void ARDrone::decodeVideo(char* data, int len){
	//Damn this is hard
}

void ARDrone::listenNavdata(){
	int c = sizeof(struct sockaddr_in);
	SOCKET clientsocket;
	struct sockaddr_in client;
	int data_size;
	char data[65535];
	while (!stopping){
		clientsocket = accept(snavdata, (struct sockaddr *)&client, &c);
		if ((data_size = recv(snavdata, data, 65536, 0)) == SOCKET_ERROR){
			std::cerr << "Navdata receive failed" << std::endl;
		}
		else{
			decodeNavdata(data, data_size);
		}
	}
}

void ARDrone::decodeNavdata(char* packet, int len){
	size_t size = (len + 3) / 4;
	int data[65535];
	memcpy_s(data, sizeof(data), packet, len);

	State drone_state;

	drone_state["fly_mask"] =				data[1]		  & 1;
	drone_state["video_mask"] =				data[1] >> 1  & 1;
	drone_state["vision_mask"] =			data[1] >> 2  & 1;
	drone_state["control_mask"] =			data[1] >> 3  & 1;
	drone_state["altitude_mask"] =			data[1] >> 4  & 1;
	drone_state["user_feedback_start"] =	data[1] >> 5  & 1;
	drone_state["command_mask"] =			data[1] >> 6  & 1;
	drone_state["fw_file_mask"] =			data[1] >> 7  & 1;
	drone_state["fw_ver_mask"] =			data[1] >> 8  & 1;
	drone_state["fw_upd_mask"] =			data[1] >> 9  & 1;
	drone_state["navdata_demo_mask"] =		data[1] >> 10 & 1;
	drone_state["navdata_bootstrap"] =		data[1] >> 11 & 1;
	drone_state["motors_mask"] =			data[1] >> 12 & 1;
	drone_state["com_lost_mask"] =			data[1] >> 13 & 1;
	drone_state["vbat_low"] =				data[1] >> 15 & 1;
	drone_state["user_el"] =				data[1] >> 16 & 1;
	drone_state["timer_elapsed"] =			data[1] >> 17 & 1;
	drone_state["angles_out_of_range"] =	data[1] >> 19 & 1;
	drone_state["ultrasound_mask"] =		data[1] >> 21 & 1;
	drone_state["cutout_mask"] =			data[1] >> 22 & 1;
	drone_state["pic_version_mask"] =		data[1] >> 23 & 1;
	drone_state["atcodec_thread_on"] =		data[1] >> 24 & 1;
	drone_state["navdata_thread_on"] =		data[1] >> 25 & 1;
	drone_state["video_thread_on"] =		data[1] >> 26 & 1;
	drone_state["acq_watchdog_on"] =		data[1] >> 27 & 1;
	drone_state["ctrl_watchdog_on"] =		data[1] >> 28 & 1;
	drone_state["adc_watchdog_mask"] =		data[1] >> 29 & 1;
	drone_state["com_watchdog_mask"] =		data[1] >> 30 & 1;
	drone_state["emergency_mask"] =			data[1] >> 31 & 1;

	navdata.drone_state = drone_state;
	navdata.header = data[0];
	navdata.seq_nr = data[2];
	navdata.vision_flag = data[3];

	int offset = 4;

	short idsize[2];
	memcpy_s(idsize, sizeof(idsize), &data[offset], 1);
	offset += 1;

	if (idsize[0] == 0){
		Demo tempnav;
		memcpy_s(&tempnav, sizeof(tempnav), &data[offset], sizeof(tempnav));
		tempnav.theta /= 1000.0;
		tempnav.phi /= 1000.0;
		tempnav.psi /= 1000.0;
		navdata.navdata = tempnav;
	}
}

void ARDrone::setupRecv(){
	//Setting up and binding video socket
	std::cout << "Initializing video socket\r\n";
	if (WSAStartup(MAKEWORD(2, 2), &wsavideo) != 0){
		std::cerr << "Failed to initialise winsock: " << WSAGetLastError() << std::endl;
		terminate();
	}

	if ((svideo = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET){
		std::cerr << "Could not create socket: " << WSAGetLastError() << std::endl;
		terminate();
	}

	addrvideo.sin_family = AF_INET;
	addrvideo.sin_addr.s_addr = INADDR_ANY;
	addrvideo.sin_port = htons(5555);
	
	if (bind(svideo, (struct sockaddr *)&addrvideo, sizeof(addrvideo)) == SOCKET_ERROR){
		std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
		terminate();
	}
	std::cout << "Bound the video socket succesfully" << std::endl;


	//Doing the same for the navdata
	std::cout << "Initializing navdata socket" << std::endl;
	if (WSAStartup(MAKEWORD(2, 2), &wsanavdata) != 0){
		std::cerr << "Failed to initialise winsock: " << WSAGetLastError() << std::endl;
		terminate();
	}

	if ((snavdata = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET){
		std::cerr << "Could not create socket: " << WSAGetLastError() << std::endl;
		terminate();
	}

	addrnavdata.sin_family = AF_INET;
	addrnavdata.sin_addr.s_addr = INADDR_ANY;
	addrnavdata.sin_port = htons(5554);

	if (bind(snavdata, (struct sockaddr *)&addrnavdata, sizeof(addrnavdata)) == SOCKET_ERROR){
		std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
		terminate();
	}
	std::cout << "Bound the navdata socket succesfully" << std::endl;
}

void ARDrone::openSocket(void){
	addrcommands.sin_addr.s_addr = inet_addr("192.168.1.1");
	addrcommands.sin_family = AF_INET;
	addrcommands.sin_port = htons(5556);
	std::cout << "Initializing sending socket" << std::endl;
	if (WSAStartup(MAKEWORD(2, 2), &wsacommands) != 0){
		std::cerr << "Failed. Error Code: " << WSAGetLastError() << std::endl;
		terminate();
	}
	std::cout << "Sending socket initialised" << std::endl;
	if ((scommands = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET){
		std::cerr << "Could not create socket: " << WSAGetLastError() << std::endl;
		terminate();
	}
	std::cout << "Sending socket created" << std::endl;
	if (connect(scommands, (struct sockaddr *)&addrcommands, sizeof(addrcommands)) < 0){
		std::cerr << "Connect error" << std::endl;
		terminate();
	}
	std::cout << "Connected Drone!" << std::endl;
}

void ARDrone::hover(){
	std::stringstream ss;
	ss << "AT*PCMD=" << seq_cnt << ",0,0,0,0,0,0,0\r";
	std::string s = ss.str();
	AT(s);
}

void ARDrone::takeOff(){
	int value = 0;
	value = value | (1 << 9);
	std::stringstream ss;
	ss << "AT*REF=" << seq_cnt << "," << value << "\r";
	std::string s = ss.str();
	AT(s);
}

void ARDrone::land(){
	int value = 0;
	value = value | (0 << 9);
	std::stringstream ss;
	ss << "AT*REF=" << seq_cnt << "," << value << "\r";
	std::string s = ss.str();
	AT(s);
}

void ARDrone::ftrim(){
	std::stringstream ss;
	ss << "AT*FTRIM=" << seq_cnt << ",\r";
	std::string s = ss.str();
	AT(s);
}

void ARDrone::reset(){
	setEmergency();
	resetEmergency();
}

void ARDrone::setEmergency(){
	int value = 0;
	value = value | (1 << 8);
	std::stringstream ss;
	ss << "AT*REF=" << seq_cnt << "," << value << "\r";
	std::string s = ss.str();
	AT(s);
}

void ARDrone::resetEmergency(){
	int value = 0;
	value = value | (0 << 8);
	std::stringstream ss;
	ss << "AT*REF=" << seq_cnt << "," << value << "\r";
	std::string s = ss.str();
	AT(s);
}

void ARDrone::AT(std::string command){
	std::cout << "Sending command: " << command << std::endl;
	char *cmd = const_cast<char*>(command.c_str());
	if (send(scommands, cmd, strlen(cmd), 0) < 0){
		std::cerr << "Send Failed" << std::endl;
		terminate();
	}
	seq_cnt += 1;
}