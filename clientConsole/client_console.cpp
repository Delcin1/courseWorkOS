#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <tuple>

#pragma comment(lib, "ws2_32.lib")

SOCKET clientSocket1;
SOCKET clientSocket2;

bool isConnectedServer1 = false;
bool isConnectedServer2 = false;


// ������� ����������� � �������
std::tuple<SOCKET, std::string, bool> connectToServer(int port) {
    // ������� ����� ��� ����������� � �������
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        return std::make_tuple(clientSocket, "������ ��� �������� ������: " + std::to_string(WSAGetLastError()), false);
    }

    // ������������� ����� � ���� �������
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port); // ���������� ���� 8888
    if (inet_pton(AF_INET, "127.0.0.1", &(serverAddress.sin_addr)) <= 0) {
        closesocket(clientSocket);
        return std::make_tuple(clientSocket, "�������� ����� �������", false);
    }

    // ������������ � �������
    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR) {
        closesocket(clientSocket);
        return std::make_tuple(clientSocket, "������ ��� ����������� � �������: " + std::to_string(WSAGetLastError()), false);
    }
    return std::make_tuple(clientSocket, "OK", true);
}

// ������� ��� �������� ������� �� ������ � ��������� ������ �� ����
std::tuple<std::string, bool> getResponse(SOCKET clientSocket, std::string request) {
    // ���������� ������ �������
    int bytesSent = send(clientSocket, request.c_str(), request.length(), 0);
    if (bytesSent == SOCKET_ERROR) {
        closesocket(clientSocket);
        return std::make_tuple("������ ��� �������� �������: " + std::to_string(WSAGetLastError()), false);
    }

    // �������� ����� �� �������
    char buffer[1024];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead == SOCKET_ERROR) {
        closesocket(clientSocket);
        return std::make_tuple("������ ��� ������ ������: " + std::to_string(WSAGetLastError()), false);
    }

    // ���������� ���������� �����
    std::string response(buffer, bytesRead);
    return std::make_tuple(response, true);
}

int main() {
    // ��������� �������� ����� ��� ������ � �������
    std::setlocale(LC_ALL, "Russian");

    // ������������� ���������� Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "������ ��� ������������� Winsock" << std::endl;
        return 1;
    }

    // ����� ��������� ������
    std::cout << "��������� �������: \n connectServer1 � connectServer2 - ����������� � �������� \n disconnectServer1 � disconnectServer2 - ���������� �� �������� \n ������ 1: \n  numButtons - ���������� ������ ���� \n  hasMouseWheel - ������� ������ ��������� \n ������ 2: \n  pid - ������������� ���������� �������� \n  userTime - ����� ������ ���������� �������� � ���������������� ������ \nquit - ������� ���������" << std::endl;
    
    // ������ � ��������� ���������� ������������� ������
    std::string msg;
    std::cout << "������� �������" << std::endl;
    std::cin >> msg;
    while (msg != "quit") {
        std::string resp;
        bool ok;
        if (msg == "connectServer1") {
            tie(clientSocket1, resp, ok) = connectToServer(8888);
            std::cout << resp << std::endl;
            if (ok) { 
                isConnectedServer1 = true;
            }
        }
        else if (msg == "connectServer2") {
            tie(clientSocket2, resp, ok) = connectToServer(8889);
            std::cout << resp << std::endl;
            if (ok) {
                isConnectedServer2 = true;
            }
        }
        else if (msg == "disconnectServer1") {

            tie(resp, ok) = getResponse(clientSocket1, "stop");
            std::cout << resp << std::endl;
            isConnectedServer1 = false;
        }
        else if (msg == "disconnectServer2") {
            tie(resp, ok) = getResponse(clientSocket2, "stop");
            std::cout << resp << std::endl;
            isConnectedServer2 = false;
        }
        else if (msg == "numButtons") {
            tie(resp, ok) = getResponse(clientSocket1, msg);
            std::cout << resp << std::endl;
            if (!ok) {
                std::cout << "���������� ���������������� � �������" << std::endl;
            }
        }
        else if (msg == "hasMouseWheel") {
            tie(resp, ok) = getResponse(clientSocket1, msg);
            std::cout << resp << std::endl;
            if (!ok) {
                std::cout << "���������� ���������������� � �������" << std::endl;
            }
        }
        else if (msg == "pid") {
            tie(resp, ok) = getResponse(clientSocket2, msg);
            std::cout << resp << std::endl;
            if (!ok) {
                std::cout << "���������� ���������������� � �������" << std::endl;
            }
        }
        else if (msg == "userTime") {
            tie(resp, ok) = getResponse(clientSocket2, msg);
            std::cout << resp << std::endl;
            if (!ok) {
                std::cout << "���������� ���������������� � �������" << std::endl;
            }
        }
        else {
            std::cout << "����� ������� �� �������" << std::endl;
        }
        std::cout << "������� �������" << std::endl;
        std::cin >> msg;
    }

    // ��������� ������ Winsock
    WSACleanup();

    return 0;
}