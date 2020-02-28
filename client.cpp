#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <fstream>
#include<stdlib.h>
int barl = 20;
using namespace std;
#pragma comment(lib, "ws2_32.lib")
/*class pBar {
public:
	void update(double newProgress) {
		currentProgress += newProgress;
		amountOfFiller = (int)((currentProgress / neededProgress) * (double)pBarLength);
	}
	void print() {
		currUpdateVal %= pBarUpdater.length();
		cout << "\r" //Bring cursor to start of line
			<< firstPartOfpBar; //Print out first part of pBar
		for (int a = 0; a < amountOfFiller; a++) { //Print out current progress
			cout << pBarFiller;
		}
		cout << pBarUpdater[currUpdateVal];
		for (int b = 0; b < pBarLength - amountOfFiller; b++) { //Print out spaces
			cout << " ";
		}
		cout << lastPartOfpBar //Print out last part of progress bar
			<< " (" << (int)(100 * (currentProgress / neededProgress)) << "%)" //This just prints out the percent
			<< flush;
		currUpdateVal += 1;
	}
	std::string firstPartOfpBar = "[", //Change these at will (that is why I made them public)
		lastPartOfpBar = "]",
		pBarFiller = "|",
		pBarUpdater = "/-\\|";
private:
	int amountOfFiller,
		pBarLength = 50, //I would recommend NOT changing this
		currUpdateVal = 0; //Do not change
	double currentProgress = 0,
		neededProgress = 100; //I would recommend NOT changing this
};*/

int main() {
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	if (WSAStartup(ver, &wsData) != 0) {
		cerr << "Error starting winsock!" << endl;
		return -1;
	}

	SOCKET clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (clientSock == INVALID_SOCKET) {
		cerr << "Error creating socket!, " << WSAGetLastError() << endl;
		return -1;
	}

	char serverAddress[NI_MAXHOST];
	memset(serverAddress, 0, NI_MAXHOST);

	cout << "Enter server address: ";
	cin.getline(serverAddress, NI_MAXHOST);

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(55000);
	inet_pton(AF_INET, serverAddress, &hint.sin_addr);

	char welcomeMsg[255];
	const int BUFFER_SIZE = 1024;
	char bufferFile[BUFFER_SIZE];
	char fileRequested[FILENAME_MAX];
	ofstream file;


	if (connect(clientSock, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
		cerr << "Error connect to server!, " << WSAGetLastError() << endl;
		closesocket(clientSock);
		WSACleanup();
		return -1;
	}

	int byRecv = recv(clientSock, welcomeMsg, 255, 0);
	if (byRecv == 0 || byRecv == -1) {
		closesocket(clientSock);
		WSACleanup();
		return -1;
	}

	bool clientClose = false;
	int codeAvailable = 404;
	const int fileAvailable = 200;
	const int fileNotfound = 404;
	long fileRequestedsize = 0;
	do {
		int fileDownloaded = 0;
		memset(fileRequested, 0, FILENAME_MAX);
		cout << "Enter file name: " << endl;
		cin.getline(fileRequested, FILENAME_MAX);

		byRecv = send(clientSock, fileRequested, FILENAME_MAX, 0);
		if (byRecv == 0 || byRecv == -1) {
			clientClose = true;
			break;
		}

		byRecv = recv(clientSock, (char*)&codeAvailable, sizeof(int), 0);
		if (byRecv == 0 || byRecv == -1) {
			clientClose = true;
			break;
		}
		if (codeAvailable == 200) {
			byRecv = recv(clientSock, (char*)&fileRequestedsize, sizeof(long), 0);
			if (byRecv == 0 || byRecv == -1) {
				clientClose = true;
				break;
			}

			file.open(fileRequested, ios::binary | ios::trunc);

			do {
				memset(bufferFile, 0, BUFFER_SIZE);
				byRecv = recv(clientSock, bufferFile, BUFFER_SIZE, 0);

				if (byRecv == 0 || byRecv == -1) {
					clientClose = true;
					break;
				}

				file.write(bufferFile, byRecv);
				fileDownloaded += byRecv;
			} while (fileDownloaded < fileRequestedsize);
			file.close();
			/*pBar bar;
			for (int i = 0; i < 100; i++) { 
				bar.update(1); //How much new progress was added (only needed when new progress was added)
				//Print pBar:
				bar.print(); //This should be called more frequently than it is in this demo (you'll have to see what looks best for your program)
				Sleep(1);
			}*/
			cout << endl;
			cout << "SUCCESSFULLY RECEIVED THE FILE " << FILENAME_MAX << endl;

		}
		else if (codeAvailable == 404) {
			cout << "Can't open file or file not found!" << endl;
		}
	} while (!clientClose);
	closesocket(clientSock);
	WSACleanup();
	return 0;
}




