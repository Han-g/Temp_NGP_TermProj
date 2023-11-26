#include "Common.h"
#include "ClientManager.h"
#include "ObjectManager.h"

#define BUFSIZE 51200

char buffer[BUFSIZE];
Send_datatype Server_bufs;
ClientManager* client;
ObjectManager* object;

static ULONGLONG Frame = 10.0f;
static float ClientTime[2] = { 0.f, 0.f };
static float Time = 0.f;
std::vector<ClientInfo> clients;

// Event ����
HANDLE bufferAccess;

static void Serialize(Send_datatype* data, char* buf, size_t bufSize) {
	// ������ ũ�� Ȯ��
	size_t dataSize = sizeof(int) + sizeof(double) + data->object_info.size() * sizeof(obj_info);

	// ���� ũ�� Ȯ��
	if (bufSize < dataSize) {
		std::cerr << "Buffer size is too small for serialization!" << std::endl;
		return;
	}

	// ���� �ʱ�ȭ
	memset(buf, 0, dataSize);

	// ������ ����
	std::memcpy(buf, &data->wParam, sizeof(int));
	buf += sizeof(int);

	std::memcpy(buf, &data->GameTime, sizeof(double));
	buf += sizeof(double);

	std::memcpy(buf, data->object_info.data(), data->object_info.size() * sizeof(obj_info));
}

static void DeSerialize(Send_datatype* data, char* buf, size_t bufSize) {
	if (bufSize < sizeof(int) + sizeof(double)) {
		std::cerr << "Buffer size is too small for deserialization!" << std::endl;
		return;
	}

	// ���� �ʱ�ȭ
	data->object_info.clear();
	data->GameTime = 0.0f;
	data->wParam = 0;

	// ������ ����
	std::memcpy(&data->wParam, buf, sizeof(int));
	buf += sizeof(int);

	std::memcpy(&data->GameTime, buf, sizeof(double));
	buf += sizeof(double);

	// obj_info ������ȭ
	size_t objInfoSize = (bufSize - sizeof(int) - sizeof(double)) / sizeof(obj_info);
	data->object_info.resize(objInfoSize);
	std::memcpy(data->object_info.data(), buf, objInfoSize * sizeof(obj_info));
}

DWORD WINAPI ObjectThread(LPVOID arg)
{
	client = static_cast<ClientManager*>(arg);

	while (1)
	{
		object->GameSet(Server_bufs);
		object->Key_Check();
		Server_bufs = object->Update();
	}

	return 0;
}

DWORD WINAPI ServerThread(LPVOID arg)
{
	client = static_cast<ClientManager*>(arg);
	//DWORD status;
	float fTime = 0.f;
	ULONGLONG StartTime = GetTickCount64();

	while (1)
	{
		if (GetTickCount64() - StartTime >= Frame) {
			fTime = GetTickCount64() - StartTime;	// ���ð��� ���� ������ �ð� ���� �ð����
			fTime = fTime / 1000.0f;				// ������ 1�ʿ� 60���� ����
			ClientTime[0] = ClientTime[1] = fTime;	// Ŭ���̾�Ʈ ������ ����ȭ

			//fTime�� �ð����� �޴� �Լ� �߰�.
			StartTime = GetTickCount64();
		}

		int retval = recv(client->getClientSocket(), buffer, BUFSIZE, 0);
		if (retval == SOCKET_ERROR || retval == 0) {
			err_display("recv()");
			break;
		}
		else { DeSerialize(&Server_bufs, buffer, sizeof(char) * BUFSIZE); }
		
		client->getBuffer(Server_bufs);
		Server_bufs = client->returnBuffer();

		Serialize(&Server_bufs, buffer, sizeof(char) * BUFSIZE);
		send(client->getClientSocket(), buffer, sizeof(char) * BUFSIZE, 0);
	}

	return 0;
}

int main()
{
	int retval;
	
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return 1;
	}
	
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) { err_display("socket()"); }

	// bind
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(nPort);
	retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_display("bind()");
	// listen
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) { err_display("listen()"); }

	// ��ſ� ����� ����
	SOCKADDR_IN clientaddr;
	int addrlen;
	int client_count = 0;

	// ���� ����
	//server = new ClientManager();
	object = new ObjectManager();

	while (1) {
		SOCKET client_sock;
		// accept
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
		
		//
		// ���� Ŭ���̾�Ʈ ���� �ڵ� ����
		//

		//����Ÿ���� INVALID_SOCKET���� ���� ��ȯ�� �˸�
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// ������ Ŭ���̾�Ʈ ���� ���
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			addr, ntohs(clientaddr.sin_port));


		//������ ����
		HANDLE hThread;
		ClientManager* client = new ClientManager(client_sock, client_count);
		hThread = CreateThread(NULL, 0, ServerThread, client, 0, NULL);
		if (hThread == NULL) { delete client; }
		else { CloseHandle(hThread); }

		DWORD ThreadID = GetThreadId(hThread);
		ClientInfo newClient;
		newClient.socket = client_sock;
		newClient.clientID = ThreadID;
		clients.push_back(newClient);

		hThread = CreateThread(NULL, 0, ObjectThread, NULL, 0, NULL);
		if (hThread == NULL) { 
			std::cerr << "ObjectThreading Failed!" << std::endl;
			return 0;
		}
		else { CloseHandle(hThread); }
	}

	// ���� ����
	delete client, object;
	WSACleanup();
	return 0;
}