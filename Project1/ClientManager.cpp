#include "ClientManager.h"

ClientManager::ClientManager(SOCKET socket, int client_id)
{

}

ClientManager::~ClientManager()
{

}

SOCKET ClientManager::getClientSocket()
{
	return socket;
}

void ClientManager::getBuffer(Send_datatype buf)
{
	recv_Buf = buf;
	send_Buf = recv_Buf;
}

Send_datatype ClientManager::returnBuffer()
{
	return send_Buf;
}
