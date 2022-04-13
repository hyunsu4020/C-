/*
 * file_server_win.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma warning(disable:4996)	// Error 추가!
#define BUFSIZE 30
void ErrorHandling(char* message);
void err_quit(char* msg);
void err_display(char* msg);

int main(int argc, char** argv)
{
	WSADATA  wsaData;
	SOCKET   hServSock;
	SOCKET   hClntSock;
	char buf[BUFSIZE];

	FILE* fp;
	SOCKADDR_IN servAddr;
	SOCKADDR_IN clntAddr;
	int clntAddrSize;
	int len;
	int total = 0;

	if (argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	printf("Server Running\n");

	/* 접속해오는 클라이언트에게 전송해 줄 파일 오픈  */
	fp = fopen("zipcode.txt", "r");
	if (fp == NULL)
		ErrorHandling("File open error");

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	if (hServSock == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(argv[1]));

	if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	if (listen(hServSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	clntAddrSize = sizeof(clntAddr);
	hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &clntAddrSize);
	if (hClntSock == INVALID_SOCKET)
		ErrorHandling("accept() error");

	/* 클라이언트에 데이터 전송 */
	while (1) {
		len = (int) fread(buf, sizeof(char), BUFSIZE, fp);
		total = total + len;
		send(hClntSock, buf, len, 0);
		if (feof(fp)) break;
	}

	/* 서버가 실제 전송한 총 바이트 수를 출력함 */
	printf("Server total bytes sented = %d\n", total);


	/* 데이터 전송후 소켓의 일부(전송영역)를 닫음 */
	if (shutdown(hClntSock, SD_SEND) == SOCKET_ERROR)
		ErrorHandling("shutdown error");

	/* 클라이언트가 보내온 수신한 총 바이트 수를 출력함. */
	len = recv(hClntSock, buf, BUFSIZE - 1, 0);
	buf[len] = 0;
	fputs(buf, stdout);

	fclose(fp);
	closesocket(hClntSock);

	WSACleanup();
	return 0;
}

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

// 소켓 함수 오류 출력 후 종료
void err_quit(char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// 소켓 함수 오류 출력
void err_display(char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}