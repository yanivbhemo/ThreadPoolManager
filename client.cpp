#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define PORT 0x0da2
#define IP_ADDR 0x7f000001

using namespace std;

int main(void)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0), nrecv;
	struct sockaddr_in s = {0};
	s.sin_family = AF_INET;
	s.sin_port = htons(PORT);
	s.sin_addr.s_addr = htonl(IP_ADDR);
	if (connect(sock, (struct sockaddr*)&s, sizeof(s)) < 0)
	{
		perror("connect");
		return 1;
	}
	std::cout << "Successfully connected.\n";
	char someBuffer[1024];
	nrecv = recv(sock, someBuffer, sizeof(someBuffer), 0);
	if (nrecv == -1)
	{
		perror("recv");
		return 1;
	}
	someBuffer[nrecv] = '\0';
	std::cout << someBuffer << std::endl;

	//User guess
	string guess;
	const char* buffer1;
	cout << "Please make a 4 digit guess (example: 4444, -1 for exit the game): ";
	cin >> guess;
	buffer1 = guess.c_str();
	while(guess != "-1")
	{
		if(send(sock, buffer1, sizeof(buffer1),0) != strlen(buffer1))
			perror("send");
		nrecv = recv(sock, someBuffer, sizeof(someBuffer), 0);
		if (nrecv == -1)
		{
			perror("recv");
			return 1;
		}
		someBuffer[nrecv] = '\0';
		std::cout << someBuffer << std::endl;
		cout << "Please make a 4 digit guess: ";
		cin >> guess;
	}
	return 0;
}
