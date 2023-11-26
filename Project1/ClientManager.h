#pragma once

#include "Global.h"

class ClientManager
{
public:
	ClientManager(SOCKET socket, int client_id);
	~ClientManager();

	SOCKET getClientSocket();
	void getBuffer(Send_datatype buf);
	Send_datatype returnBuffer();

private:
	SOCKET socket;
	Send_datatype recv_Buf;
	Send_datatype send_Buf;
	int ClientID;
};

