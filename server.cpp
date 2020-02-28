#include <iostream>
#include <fstream>
#include <ws2tcpip.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")
using namespace std;
class pBar {
public:
	void update(double newProgress) {
		currentProgress += newProgress;
		amountOfFiller = (int)((currentProgress / neededProgress) * (double)pBarLength);
	}
	void print() {
		currUpdateVal %= pBarUpdater.length();
		cout << "\r" << firstPartOfpBar; 
		for (int a = 0; a < amountOfFiller; a++) { 
			cout << pBarFiller;
		}
		cout << pBarUpdater[currUpdateVal];
		for (int b = 0; b < pBarLength - amountOfFiller; b++) { 
			cout << " ";
		}
		cout << lastPartOfpBar 
			<< " (" << (int)(100 * (currentProgress / neededProgress)) << "%)" 
			<< flush;
		currUpdateVal += 1;
	}
	std::string firstPartOfpBar = "[", 
		lastPartOfpBar = "]",
		pBarFiller = "|",
		pBarUpdater = "/-\\|";
private:
	int amountOfFiller,
		pBarLength = 50, 
		currUpdateVal = 0; 
	double currentProgress = 0,
		neededProgress = 100; 
};
int main() {
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	if (WSAStartup(ver, &wsData) != 0) {
		cerr << "Error starting winsock!" << endl;
		return -1;
	}

	SOCKET listenerSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (listenerSock == INVALID_SOCKET) {
		cerr << "Error creating listener socket! " << WSAGetLastError() << endl;
		WSACleanup();
		return -1;
	}

	sockaddr_in listenerHint;
	listenerHint.sin_family = AF_INET;
	listenerHint.sin_port = htons(55000);
	listenerHint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(listenerSock, (sockaddr*)&listenerHint, sizeof(listenerHint));
	listen(listenerSock, SOMAXCONN);

	sockaddr_in clientHint;
	int clientSize = sizeof(clientHint);

	SOCKET clientSock = accept(listenerSock, (sockaddr*)&clientHint, &clientSize);

	if (clientSock == SOCKET_ERROR) {
		cerr << "Error accept socket! " << WSAGetLastError() << endl;
		closesocket(listenerSock);
		WSACleanup();
		return -1;
	}

	char host[NI_MAXHOST];
	char serv[NI_MAXSERV];

	if (getnameinfo((sockaddr*)&clientHint, sizeof(clientHint), host, NI_MAXHOST, serv, NI_MAXSERV, 0) == 0) {
		cout << "Succesfully Connected !!!!!!!" << endl;
		cout << "Transfer your files and data at one go " << endl;
		cout << "Host: " << host << " connected on port: " << serv << endl;
	}
	else {
		inet_ntop(AF_INET, &clientHint.sin_addr, host, NI_MAXHOST);
		cout << "Succesfully Connected !!!!!!!" << endl;
		cout << "Transfer your files and data at one go " << endl;
		cout << "Host: " << host << " connected on port: " << ntohs(clientHint.sin_port) << endl << endl;
	}

	closesocket(listenerSock);

	const char* welcomeMsg = "Welcome to file server.";
	bool clientClose = false;
	char fileRequested[FILENAME_MAX];
	const int fileAvailable = 200;
	const int fileNotfound = 404;
	const int BUFFER_SIZE = 1024;
	char bufferFile[BUFFER_SIZE];
	std::ifstream file;

	// sending welcome message
	int bysendMsg = send(clientSock, welcomeMsg, strlen(welcomeMsg), 0);

	if (bysendMsg == 0) {
		// error sending data - break loop
		closesocket(clientSock);
		WSACleanup();
		return -1;
	}

	do {
		memset(fileRequested, 0, FILENAME_MAX);
		int byRecv = recv(clientSock, fileRequested, FILENAME_MAX, 0);

		if (byRecv == 0 || byRecv == -1) {
			// error receive data - break loop
			clientClose = true;
		}

		// open file
		file.open(fileRequested, ios::binary);

		if (file.is_open()) {
			// file is available
			int bySendinfo = send(clientSock, (char*)&fileAvailable, sizeof(int), 0);
			if (bySendinfo == 0 || bySendinfo == -1) {
				// error sending data - break loop
				clientClose = true;
			}

			// get file size
			file.seekg(0, ios::end);
			long fileSize = file.tellg();

			// send filesize to client
			bySendinfo = send(clientSock, (char*)&fileSize, sizeof(long), 0);
			if (bySendinfo == 0 || bySendinfo == -1) {
				// error sending data - break loop
				clientClose = true;
			}
			file.seekg(0, ios::beg);
			// read file with do-while loop
			do {
				// read and send part file to client
				file.read(bufferFile, BUFFER_SIZE);
				if (file.gcount() > 0)
				{
					bySendinfo = send(clientSock, bufferFile, file.gcount(), 0);
				}


				if (bySendinfo == 0 || bySendinfo == -1) {
					// error sending data - break loop
					clientClose = true;
					break;
				}
			} while (file.gcount() > 0);
			file.close();
			pBar bar;
			for (int i = 0; i < 100; i++) {
				bar.update(1); //How much new progress was added (only needed when new progress was added)
				//Print pBar:
				bar.print(); //This should be called more frequently than it is in this demo (you'll have to see what looks best for your program)
				Sleep(1);
			}
			cout << endl;
			cout << "\n transfer succesfull " << endl;
			cout << "\n Succesfully SENT file ::" << FILENAME_MAX;

		}

		else {
			// Can't open file or file not found
			int bySendCode = send(clientSock, (char*)&fileNotfound, sizeof(int), 0);
			if (bySendCode == 0 || bySendCode == -1) {
				// error sending data - break loop
				clientClose = true;
			}
		}
	} while (!clientClose);

	return 0;
}