// pakiet ol.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <string>
#include <time.h>
#include <limits>
#include <winsock.h>
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
	int IO = 0; //identyfikator obliczenia
	std::vector<std::string> stringPackets;
	std::vector<CalculationHistory> history;
public:

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
		if (isInPacket("WY", packet)) WY = std::stoi(assignToVariable("WY", packet));
		if (isInPacket("IO", packet)) IO = std::stoi(assignToVariable("IO", packet));
		NS--;
	}

	bool morePackets() {
		if (NS != -1)return true;
		else return false;
	}

	void showPacket() {
		std::cout << ZC << std::endl << NS << std::endl << ID << std::endl << ST << std::endl << L1 << std::endl << L2 << std::endl << WY << std::endl;
	}

	void calculateResult() {
		long long int result = L1;
		if (OP == "dodawanie") {
			result += L2;
			if (result < INT_MAX) {
				WY = result;
				ST = 3;
			}
			else {
				ST = 9; //przepelnienie zmiennej
				NS = 0;
			}
		}

		if (OP == "odejmowanie") {
			result -= L2;
			if (result < INT_MAX) {
				WY = result;
				ST = 3;
			}
			else {
				ST = 9; //przepelnienie zmiennej
				NS = 0;
			}
		}

		if (OP == "mnozenie") {
			result *= L2;
			if (result < INT_MAX) {
				WY = result;
				ST = 3;
			}
			else {
				ST = 9; //przepelnienie zmiennej
				NS = 0;
			}
		}

		if (OP == "dzielenie") {
			if (L2 != 0) {
				result /= L2;
				if (result < INT_MAX) {
					WY = result;
					ST = 3;
				}
				else {
					ST = 9; //przepelnienie zmiennej
					NS = 0;
				}
			}
			else {
				ST = 8; //blad dzielenia przez 0
				NS = 0;
			}

		}

		if (OP == "silnia") {
			result = 1;
			for (int i = 1; i <= L1; i++) {
				result *= i;
				if (result > INT_MAX) {
					ST = 9;
					NS = 0;
					break;
				}
			}
			WY = result;
		}

	}

	void createPacket() {
		std::string packetString = "";
		stringPackets.clear();
		int nsCounter = 0; //do case 5
		std::string nr_sekw, nr_id, nr_stat, result, op, l1, l2;
		switch (ST) {
		case 0:  //prosba o id
			std::cout << ZC << " Odebrano prosbe o ID" << std::endl;
			ZC = currentTime();
			NS = 0;
			ID = 1;
			ST = 1;

			nr_sekw = std::to_string(NS);
			nr_id = std::to_string(ID);
			nr_stat = std::to_string(ST);
			packetString = "ZC: " + ZC + " NS: " +
				nr_sekw + " ID: " + nr_id + " ST: " + nr_stat + " ";
			stringPackets.push_back(packetString);
			std::cout << ZC << " Wysylam ID" << std::endl;

			break;

		case 2: //prosba o obliczenie
			std::cout << ZC << " Odebrano prosbe o obliczenie" << std::endl;
			ZC = currentTime();
			NS = 1;
			ID = 1;
			ST = 3;

			calculateResult();
			if (ST != 8 && ST != 9) {
				CalculationHistory current;
				current.L1 = L1;
				if (OP != "silnia") current.L2 = L2;
				current.OP = OP;
				current.WY = WY;
				history.push_back(current);
			}

			ZC = currentTime();
			nr_sekw = std::to_string(NS);
			nr_id = std::to_string(ID);
			nr_stat = std::to_string(ST);
			result = std::to_string(WY);

			if (ST != 8 && ST != 9) {
				packetString = "ZC: " + ZC + " NS: " +
					nr_sekw + " ID: " + nr_id + " ST: " + nr_stat;
				stringPackets.push_back(packetString);

				nr_sekw = std::to_string(--NS);

				packetString = " ZC: " + ZC + " NS: " +
					nr_sekw + " ID: " + nr_id + " WY: " + result;
				stringPackets.push_back(packetString);

				std::cout << ZC << " Wysylam wynik: " << result << std::endl;
			}
			else {
				packetString = "ZC: " + ZC + " NS: " +
					nr_sekw + " ID: " + nr_id + " ST: " + nr_stat;
				stringPackets.push_back(packetString);
				if (ST == 8) std::cout << ZC << " Wysylam blad dzielenia przez 0 " << std::endl;
				if (ST == 9) std::cout << ZC << " Wysylam blad przepelnienia zmiennej " << std::endl;
			}

			break;

		case 4: //prosba o przejrzenie historii obliczen

			ZC = currentTime();
			nr_id = std::to_string(ID);
			nr_stat = std::to_string(5);


			for (int i = 0; i < history.size(); i++) {
				//if (history[i].OP == "silnia") nsCounter +=3;
				//else
				nsCounter += 4;
			}


			nr_sekw = std::to_string(nsCounter);
			packetString = "ZC: " + ZC + " NS: " +
				nr_sekw + " ID: " + nr_id + " ST: " + nr_stat;
			stringPackets.push_back(packetString);
			nr_sekw = std::to_string(--nsCounter);
			for (int i = 0; i < history.size(); i++) {
				op = history[i].OP;
				packetString = " ZC: " + ZC + " NS: " +
					nr_sekw + " ID: " + nr_id + " OP: " + op + " ";
				stringPackets.push_back(packetString);
				nr_sekw = std::to_string(--nsCounter);

				l1 = std::to_string(history[i].L1);
				packetString = " ZC: " + ZC + " NS: " +
					nr_sekw + " ID: " + nr_id + " L1: " + l1 + " ";
				stringPackets.push_back(packetString);
				nr_sekw = std::to_string(--nsCounter);

				//  if (OP != "silnia") {
				l2 = std::to_string(history[i].L2);
				packetString = " ZC: " + ZC + " NS: " +
					nr_sekw + " ID: " + nr_id + " L2: " + l2 + " ";
				stringPackets.push_back(packetString);
				nr_sekw = std::to_string(--nsCounter);
				//  }

				result = std::to_string(history[i].WY);
				packetString = " ZC: " + ZC + " NS: " +
					nr_sekw + " ID: " + nr_id + " WY: " + result + " ";
				stringPackets.push_back(packetString);
				nr_sekw = std::to_string(--nsCounter);
			}

			std::cout << ZC << " Wysylam cala historie klientowi " << std::endl;

			break;

		case 6:
			ZC = currentTime();
			nr_id = std::to_string(ID);

			if (IO > 0 && IO <= history.size()) {

				nr_stat = std::to_string(7);

				op = history[IO - 1].OP;
				if (op == "silnia") {
					nsCounter = 3;
					nr_sekw = std::to_string(nsCounter);
				}
				else {
					nsCounter = 4;
					nr_sekw = std::to_string(nsCounter);
				}

				packetString = " ZC: " + ZC + " NS: " +
					nr_sekw + " ID: " + nr_id + " ST: " + nr_stat + " ";
				stringPackets.push_back(packetString);
				nr_sekw = std::to_string(--nsCounter);

				packetString = " ZC: " + ZC + " NS: " +
					nr_sekw + " ID: " + nr_id + " OP: " + op + " ";
				stringPackets.push_back(packetString);
				nr_sekw = std::to_string(--nsCounter);

				l1 = std::to_string(history[IO - 1].L1);
				packetString = " ZC: " + ZC + " NS: " +
					nr_sekw + " ID: " + nr_id + " L1: " + l1 + " ";
				stringPackets.push_back(packetString);
				nr_sekw = std::to_string(--nsCounter);

				if (OP != "silnia") {
					l2 = std::to_string(history[IO - 1].L2);
					packetString = " ZC: " + ZC + " NS: " +
						nr_sekw + " ID: " + nr_id + " L2: " + l2 + " ";
					stringPackets.push_back(packetString);
					nr_sekw = std::to_string(--nsCounter);
				}

				result = std::to_string(history[IO - 1].WY);
				packetString = " ZC: " + ZC + " NS: " +
					nr_sekw + " ID: " + nr_id + " WY: " + result + " ";
				stringPackets.push_back(packetString);
				nr_sekw = std::to_string(--nsCounter);
				std::cout << ZC << " Wysylam jedna historie obliczenia klientowi " << std::endl;
			}
			else {
				nr_stat = std::to_string(10);
				nr_sekw = std::to_string(0);
				packetString = " ZC: " + ZC + " NS: " +
					nr_sekw + " ID: " + nr_id + " ST: " + nr_stat + " ";
				std::cout << ZC << " Wysylam blad o nie posiadaniu takiego obliczenia " << std::endl;
				stringPackets.push_back(packetString);
			}
			break;

		case 11: //klient zakonczyl polaczenie
			std::cout << ZC << " Klient sie rozlaczyl" << std::endl;
			ID = 0;
			history.clear();
			break;
		}
	}

	std::vector<std::string> returnPackets() {
		return stringPackets;
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

public:
	Connection() {
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
			std::cout << "Blad podczas tworzenia socketu: " << WSAGetLastError() << std::endl;
			WSACleanup();
			return 1;
		}
		//przypisanie socketu do adresu
		if (bind(mainSocket, (SOCKADDR *)& server, sizeof(server)) == SOCKET_ERROR)
		{
			std::cout << "bind() zakonczony niepoowdzeniem " << std::endl;
			closesocket(mainSocket); //zamkniecie socketu
			return 2;
		}

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

		packet.createPacket();

	}

	//przeslanie danych
	void sendData() {
		std::vector<std::string> packets = packet.returnPackets();
		for (int i = 0; i < packets.size(); i++) {
			bytesSent = sendto(mainSocket, packets[i].c_str(), packets[i].size(), 0,
				(struct sockaddr *) & client, sizeof(struct sockaddr));
		}
	}

};


int main()
{
	Connection connect;
	connect.connect();

	while (true) {
		connect.reciveData();
		connect.sendData();
	}


	system("PAUSE");
	return 0;
}