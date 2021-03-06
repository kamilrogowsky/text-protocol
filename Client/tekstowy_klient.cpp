// pakiet ol.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <string>
#include <time.h>
#include <limits>
#include <winsock.h>
#include <limits>
#pragma comment(lib, "Ws2_32.lib")

#define SERVERPORT 4950
#define CLIENTPORT 4951

std::string currentTime() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);

	strftime(buf, sizeof(buf), "%X", &tstruct);

	return buf;
}

struct CalculationHistory {
	std::string OP = "";
	int L1 = 0;
	int L2 = 0;
	int WY = 0;
};

class Packet {
	std::string ZC;
	int NS;
	int ID;
	int ST;
	std::string OP;
	int L1 = 0;
	int L2 = 0;
	int WY = 0; //result;
	int IO = 0;
	std::vector<std::string> stringPackets;
	bool polaczenie = true;
	std::vector<CalculationHistory> history;

public:

	Packet() {
		ZC = currentTime();
		NS = 0;
		ID = 0;
		ST = 0;
		OP = "nie_arytmetyczna";
		L1 = 0;
		L2 = 0;
		WY = 0;
	}

	std::string assignToVariable(std::string key, std::string packet) {
		std::string::size_type beginning;
		std::string::size_type end;
		beginning = packet.find(key);
		if (beginning != -1) {
			beginning += 4;
			end = packet.find(" ", beginning);
			end = end - beginning;
			return packet.substr(beginning, end);
		}
		else {
			return "0";
		}
	}

	bool isInPacket(std::string key, std::string packet) {
		if (packet.find(key) != -1) return true;
		else return false;
	}


	void readPacketInfo(char * pak) {
		std::string packet(pak);
		ZC = assignToVariable("ZC", packet);
		NS = std::stoi(assignToVariable("NS", packet));
		ID = std::stoi(assignToVariable("ID", packet));
		ST = std::stoi(assignToVariable("ST", packet)); //zawsze status jest w 1 pakiecie
		NS--;
	}

	void readAdditionalInfo(std::string packet) {
		if (isInPacket("OP", packet)) OP = assignToVariable("OP", packet);
		if (isInPacket("L1", packet)) L1 = std::stoi(assignToVariable("L1", packet));
		if (isInPacket("L2", packet)) L2 = std::stoi(assignToVariable("L2", packet));
		if (isInPacket("IO", packet)) IO = std::stoi(assignToVariable("IO", packet));
		if (isInPacket("WY", packet)) {
			WY = std::stoi(assignToVariable("WY", packet));
			if (ST == 5 || ST == 7) {
				saveHistory();
			}
		}
		NS--;
	}

	bool morePackets() {
		if (NS != -1)return true;
		else return false;
	}

	void showPacket() {
		std::cout << ZC << std::endl << NS << std::endl << ID << std::endl << ST << std::endl << L1 << std::endl << L2 << std::endl << WY << std::endl;
	}

	void openPacket() {
		switch (ST) {
		case 1:
			std::cout << ZC << " Odebrano ID: " << ID << std::endl;
			break;
		case 3:
			std::cout << ZC << " Wynik dzialania to: " << WY << std::endl;
			break;

		case 5:
			std::cout << ZC << " Cala historia obliczen: " << std::endl;
			showHistory();
			break;

		case 7:
			std::cout << ZC << " Poproszone dzialanie: " << std::endl;
			showHistory();
			break;

		case 8:
			std::cout << ZC << " Blad dzielenia przez 0 " << std::endl;
			break;

		case 9:
			std::cout << ZC << " Przepelnienie zmiennej " << std::endl;
			break;

		case 10:
			std::cout << ZC << " Nie ma takiego obliczenia " << std::endl;
		}
	}

	bool checkForOverflow(std::string number) {
		if (number.size() < 10 && std::stoll(number) < INT_MAX) {
			return false;
		}
		else return true;
	}

	void inputData() {
		std::string historyNumber = "";//dla case 7
		std::cout << "Jaka operacje chcesz wykonac: " << std::endl;
		std::cout << " 1.Dodawanie\n 2.Ojdemowanie\n 3.Mnozenie\n 4.Dzielenie\n 5.Silnia\n 6.Wyswietlenie historii\n 7.Wyswietlenie jednego dzialania\n 8.Rozlaczenie sie\n";
		int input;
		do {
			std::cin >> input;
		} while (input < 1 && input>8);

		if (input >= 1 && input <= 5) {
			std::cout << "Wprowadz 1 liczbe:" << std::endl;
			std::string number;
			do {
				std::cin >> number;
			} while (checkForOverflow(number));
			L1 = stoi(number);
			NS = 2; //dla silni
			if (input != 5) {
				std::cout << "Wprowadz 2 liczbe:" << std::endl;
				do {
					std::cin >> number;
				} while (checkForOverflow(number));
				L2 = stoi(number);
				NS = 3;
			}
			ST = 2;
		}

		switch (input) {
		case 1:
			OP = "dodawanie";
			break;
		case 2:
			OP = "odejmowanie";
			break;
		case 3:
			OP = "mnozenie";
			break;
		case 4:
			OP = "dzielenie";
			break;
		case 5:
			OP = "silnia";
			break;
		case 6:
			ST = 4;
			NS = 0;
			break;
		case 7:
			std::cout << "Podaj number obliczenia: ";
			do {
				std::cin >> historyNumber;
			} while (checkForOverflow(historyNumber));
			IO = std::stoi(historyNumber);
			ST = 6;
			NS = 1;
			break;
		case 8:
			ST = 11;
			NS = 0;
			break;

		}

		ZC = currentTime();

	}

	std::string idRequest() {
		return "ZC: " + currentTime() + " NS: 0 ID: 0 ST: 0 ";
	}

	void createPacket() {
		std::string packetString = "";
		stringPackets.clear();
		std::string nr_sekw, nr_id, nr_stat, l1, l2, wy, io;

		switch (ST) {
		case 2: //prosba o obliczenie
			nr_sekw = std::to_string(NS);
			nr_id = std::to_string(ID);
			nr_stat = std::to_string(ST);
			l1 = std::to_string(L1);
			l2 = std::to_string(L2);
			if (NS == 3) {
				packetString = "ZC: " + ZC + " NS: " +
					nr_sekw + " ID: " + nr_id + " ST: " + nr_stat + " ";
				stringPackets.push_back(packetString);

				nr_sekw = std::to_string(--NS);

				packetString = "ZC: " + ZC + " NS: " +
					nr_sekw + " ID: " + nr_id + " OP: " + OP + " ";
				stringPackets.push_back(packetString);

				nr_sekw = std::to_string(--NS);

				packetString = "ZC: " + ZC + " NS: " +
					nr_sekw + " ID: " + nr_id + " L1: " + l1 + " ";
				stringPackets.push_back(packetString);

				nr_sekw = std::to_string(--NS);

				packetString = "ZC: " + ZC + " NS: " +
					nr_sekw + " ID: " + nr_id + " L2: " + l2 + " ";
				stringPackets.push_back(packetString);
			}
			else {
				packetString = "ZC: " + ZC + " NS: " +
					nr_sekw + " ID: " + nr_id + " ST: " + nr_stat + " ";
				stringPackets.push_back(packetString);

				nr_sekw = std::to_string(--NS);

				packetString = "ZC: " + ZC + " NS: " +
					nr_sekw + " ID: " + nr_id + " OP: " + OP + " ";
				stringPackets.push_back(packetString);

				nr_sekw = std::to_string(--NS);

				packetString = "ZC: " + ZC + " NS: " +
					nr_sekw + " ID: " + nr_id + " L1: " + l1 + " ";
				stringPackets.push_back(packetString);
			}

			break;

		case 4: //historia id sesji
			nr_sekw = std::to_string(NS);
			nr_id = std::to_string(ID);
			nr_stat = std::to_string(ST);
			packetString = "ZC: " + ZC + " NS: " +
				nr_sekw + " ID: " + nr_id + " ST: " + nr_stat + " ";
			stringPackets.push_back(packetString);
			break;

		case 6: //historia id obliczen
			nr_sekw = std::to_string(NS);
			nr_id = std::to_string(ID);
			nr_stat = std::to_string(ST);
			packetString = "ZC: " + ZC + " NS: " +
				nr_sekw + " ID: " + nr_id + " ST: " + nr_stat + " ";
			stringPackets.push_back(packetString);
			nr_sekw = std::to_string(--NS);

			io = std::to_string(IO);
			packetString = "ZC: " + ZC + " NS: " +
				nr_sekw + " ID: " + nr_id + " IO: " + io + " ";
			stringPackets.push_back(packetString);

			break;

		case 8:
		case 9:
			nr_sekw = std::to_string(NS);
			nr_id = std::to_string(ID);
			nr_stat = std::to_string(ST);
			packetString = "ZC: " + ZC + " NS: " +
				nr_sekw + " ID: " + nr_id + " ST: " + nr_stat + " ";
			stringPackets.push_back(packetString);
			break;

		case 11: //rozlaczenie sie

			nr_sekw = std::to_string(NS);
			nr_id = std::to_string(ID);
			nr_stat = std::to_string(ST);
			packetString = "ZC: " + ZC + " NS: " +
				nr_sekw + " ID: " + nr_id + " ST: " + nr_stat + " ";
			stringPackets.push_back(packetString);
			std::cout << ZC << " Koncze polaczenie" << std::endl;

			polaczenie = false;
			break;
		}
	}

	bool rozlaczycSie() {
		return polaczenie;
	}

	std::vector<std::string> returnPackets() {
		return stringPackets;
	}

	void saveHistory() {
		CalculationHistory current;
		current.OP = OP;
		current.L1 = L1;
		if (current.OP != "silnia")current.L2 = L2;
		current.WY = WY;
		history.push_back(current);
	}

	void showHistory() {
		for (int i = 0; i < history.size(); i++) {
			if (history[i].OP == "dodawanie")std::cout << history[i].L1 << " + " << history[i].L2 << " = " << history[i].WY << std::endl;
			if (history[i].OP == "odejmowanie")std::cout << history[i].L1 << " - " << history[i].L2 << " =" << history[i].WY << std::endl;
			if (history[i].OP == "mnozenie")std::cout << history[i].L1 << " * " << history[i].L2 << " = " << history[i].WY << std::endl;
			if (history[i].OP == "dzielenie")std::cout << history[i].L1 << " / " << history[i].L2 << " = " << history[i].WY << std::endl;
			if (history[i].OP == "silnia")std::cout << history[i].L1 << "! = " << history[i].WY << std::endl;
		}
		history.clear();
	}

};

class Connection {
	sockaddr_in client; //adres klienta
	sockaddr_in server; //adres serwera
	WSADATA wsaData;  //zmienna przechowujaca informacje o Winsock
	SOCKET mainSocket; //socket nasluchujacy polaczenia

	Packet packet; //obiekt przechowujacy pakiet
	char packetRecived[1024];

	int bytesSent; //bajty wyslane
	int bytesRecv; //bajty odebrane

	bool polaczenie = true;
public:
	Connection() {

		memset(&client, 0, sizeof(client));
		client.sin_family = AF_INET;
		client.sin_addr.s_addr = inet_addr("127.0.0.1");
		client.sin_port = htons(CLIENTPORT);

		memset(&server, 0, sizeof(server)); //wyczyszczenie poczatkowych danych
		server.sin_family = AF_INET; //ustawienie rodziny adresow
		server.sin_addr.s_addr = inet_addr("127.0.0.1"); //ustawienie adresu serwera
		server.sin_port = htons(SERVERPORT); //ustawienie portu serwera
		bytesSent = 0;
		bytesRecv = 0;
	}

	//tworzenie socketu UDP i przypisanie mu adresu oraz portu
	bool connect() {
		int result = WSAStartup(MAKEWORD(2, 2), &wsaData); //wybor wersji winsock
		if (result != NO_ERROR)
			std::cout << "Blad inicjalizacji" << std::endl;

		mainSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //tworzenie socketu UDP
		if (mainSocket == INVALID_SOCKET)
		{
			WSACleanup();
			return 0;
		}

		//przypisanie socketu do adresu
		if (bind(mainSocket, (SOCKADDR *)& client, sizeof(client)) == SOCKET_ERROR)
		{
			std::cout << "bind() zakonczony niepoowdzeniem " << std::endl;
			std::cout << WSAGetLastError() << std::endl;
			closesocket(mainSocket); //zamkniecie socketu
			return 0;
		}
	}

	bool isConnected() {
		return polaczenie;
	}

	//odbior danych
	void reciveData() {
		int addr_len = sizeof(struct sockaddr);
		memset(packetRecived, 0, 1024);
		bytesRecv = recvfrom(mainSocket, packetRecived, sizeof(packetRecived), 0,
			(struct sockaddr *)& client, &addr_len);

		packet.readPacketInfo(packetRecived);

		while (packet.morePackets()) {
			memset(packetRecived, 0, 1024);
			bytesRecv = recvfrom(mainSocket, packetRecived, sizeof(packetRecived), 0,
				(struct sockaddr *)& client, &addr_len);
			packet.readAdditionalInfo(packetRecived);
		}
		packet.openPacket();
	}

	void requestId() {
		std::string packetId = packet.idRequest();
		bytesSent = sendto(mainSocket, packetId.c_str(), packetId.size(), 0,
			(struct sockaddr *) & server, sizeof(struct sockaddr));
	}
	//przeslanie danych
	void sendData() {

		packet.inputData();
		packet.createPacket();
		std::vector<std::string> packets = packet.returnPackets();
		for (int i = 0; i < packets.size(); i++) {
			bytesSent = sendto(mainSocket, packets[i].c_str(), packets[i].size(), 0,
				(struct sockaddr *) & server, sizeof(struct sockaddr));
		}
		if (!packet.rozlaczycSie()) polaczenie = false;
	}

};


int main()
{
	Connection connect;
	connect.connect();

	//wyslanie prosby o id
	connect.requestId();

	while (connect.isConnected()) {
		connect.reciveData();
		connect.sendData();
	}


	system("PAUSE");
	return 0;
}
