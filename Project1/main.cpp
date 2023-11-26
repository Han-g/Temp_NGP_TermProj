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

// Event 선언
HANDLE bufferAccess;

static void Serialize(Send_datatype* data, char* buf, size_t bufSize) {
	// 데이터 크기 확인
	size_t dataSize = sizeof(int) + sizeof(double) + data->object_info.size() * sizeof(obj_info);

	// 버퍼 크기 확인
	if (bufSize < dataSize) {
		std::cerr << "Buffer size is too small for serialization!" << std::endl;
		return;
	}

	// 버퍼 초기화
	memset(buf, 0, dataSize);

	// 데이터 복사
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

	// 버퍼 초기화
	data->object_info.clear();
	data->GameTime = 0.0f;
	data->wParam = 0;

	// 데이터 복사
	std::memcpy(&data->wParam, buf, sizeof(int));
	buf += sizeof(int);

	std::memcpy(&data->GameTime, buf, sizeof(double));
	buf += sizeof(double);

	// obj_info 역직렬화
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
			fTime = GetTickCount64() - StartTime;	// 현시간과 이전 프레임 시간 차로 시간계산
			fTime = fTime / 1000.0f;				// 프레임 1초에 60으로 고정
			ClientTime[0] = ClientTime[1] = fTime;	// 클라이언트 프레임 동기화

			//fTime의 시간값을 받는 함수 추가.
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
	
	// 윈속 초기화
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

	// 통신에 사용할 변수
	SOCKADDR_IN clientaddr;
	int addrlen;
	int client_count = 0;

	// 서버 관리
	//server = new ClientManager();
	object = new ObjectManager();

	while (1) {
		SOCKET client_sock;
		// accept
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
		
		//
		// 다중 클라이언트 관리 코드 생성
		//

		//소켓타입이 INVALID_SOCKET으로 오류 반환시 알림
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));


		//스레드 생성
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

	// 윈속 종료
	delete client, object;
	WSACleanup();
	return 0;
}