#include <iostream>
#include <string>
#include <thread>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <sstream>
#include <cstdio>


#pragma comment(lib, "ws2_32.lib")

// ������� �������� �� ��������� ������
bool IsAnotherInstanceRunning() {
    std::string lockFilePath = "server2.lock";

    // ������� ������� ���� ���������� � ������ ������
    std::ifstream lockFile(lockFilePath);
    if (lockFile.is_open()) {
        // ���� ���������� ��� ����������, ������ ������ ��������� ������� ��� �������
        lockFile.close();
        return true;
    }

    // ���� ���������� �� ����������, ������� ���
    std::ofstream newLockFile(lockFilePath);
    if (newLockFile.is_open()) {
        // ���������� ���-������ � ���� ����������
        newLockFile << "Server lock file";
        newLockFile.close();
        return false;
    }

    // �� ������� ������� ���� ����������
    return true;
}

// ������� �������� ����� ����������
void removeLockFile() {
    std::remove("server2.lock");
}

// ���������� ������� ���������� ����������
BOOL WINAPI ConsoleCtrlHandler(DWORD ctrlType) {
    if (ctrlType == CTRL_CLOSE_EVENT || ctrlType == CTRL_C_EVENT) {
        std::cout << "�������� �������..." << std::endl;
        removeLockFile(); // ������� ���� ���������� ��� �������� ����� ������� CTRL_CLOSE_EVENT
        exit(0);
    }
    return FALSE;
}

// ������� ��� ��������� �������
void handleClient(SOCKET clientSocket) {
    std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
    char buffer[1024];
    bool mustClose = false;

    while (!mustClose) {
        // �������� ������ �� �������
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead == SOCKET_ERROR) {
            std::cerr << "������ ��� ������ ������� �� �������: " << WSAGetLastError() << std::endl;
            mustClose = true;
        }
        else if (bytesRead == 0) {
            // ������ ������ ����������
            mustClose = true;
        }
        else {
            std::cout << "�������� ������ �� �������" << std::endl;

            // ������ ������
            std::string request(buffer, bytesRead);
            // ���������� ��������������� ��������� �������
            std::string response;
            if (request == "pid") {
                response = "������������� ���������� ��������: " + std::to_string(GetCurrentProcessId());
            }
            else if (request == "userTime") {
                // ��������� �������� �������
                std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
                // ���������� ����������������� ������� �� ����� ������ �� �������� �������
                std::chrono::duration<double> duration = std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - startTime);

                // ������� ����������������� ������� � ����, ������ � �������
                std::chrono::hours hours = std::chrono::duration_cast<std::chrono::hours>(duration);
                duration -= hours;
                std::chrono::minutes minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
                duration -= minutes;
                std::chrono::seconds seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);

                // �������������� ����������������� ������� � ������
                std::stringstream ss;
                ss << std::setfill('0');
                ss << std::setw(2) << hours.count() << ":";
                ss << std::setw(2) << minutes.count() << ":";
                ss << std::setw(2) << seconds.count();

                std::string formattedTime = ss.str();
                response = "����� ������ ���������� �������� � ���������������� ������: " + formattedTime;
            }
            else if (request == "stop") {
                response = "���������� �������";
                mustClose = true;
            }
            else {
                response = "�������� ������";
            }

            // ���������� ����� �������
            int bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
            if (bytesSent == SOCKET_ERROR) {
                std::cerr << "������ ��� �������� ������ �������: " << WSAGetLastError() << std::endl;
                mustClose = true;
            }
            std::cout << " ���������� ����� �������" << std::endl;
        }
    }

    // ��������� ���������� � ��������
    closesocket(clientSocket);
}

int main() {
    // �������� �� ��������� ������
    if (IsAnotherInstanceRunning()) {
        std::cout << "������ ��������� ������� ��� �������." << std::endl;
        return 0;
    }

    // ��������� ����������� ������� ���������� ����������
    if (!SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE)) {
        std::cerr << "�� ������� ���������� ���������� ������� ����������" << std::endl;
        removeLockFile(); // ������� ���� ���������� ��� ������
        return 1;
    }

    // ��������� �������� ����� ��� ������ � �������
    std::setlocale(LC_ALL, "Russian");

    // ������������� ���������� Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "������ ��� ������������� Winsock" << std::endl;
        return 1;
    }

    // ������� ����� ��� ������������� ��������
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "������ ��� �������� ������: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // ����������� ����� � ������ � �����
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8889); // ���������� ���� 8888

    if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "������ ��� �������� ������ � ������: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // �������� ������������� ��������
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "������ ��� ������ ������������� ������: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "������ �������. �������� �����������..." << std::endl;

    std::vector<std::thread> clientThreads;

    while (true) {
        // ��������� �������� �����������
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "������ ��� �������� �����������: " << WSAGetLastError() << std::endl;
            closesocket(listenSocket);
            WSACleanup();
            return 1;
        }

        // ������� ��������� ����� ��� ��������� �������
        std::thread clientThread(handleClient, clientSocket);
        clientThreads.push_back(std::move(clientThread));
    }

    // ���������� ���������� ���� �������
    for (std::thread& thread : clientThreads) {
        thread.join();
    }

    // ��������� ����� ��� �������������
    closesocket(listenSocket);

    // ��������� ������ Winsock
    WSACleanup();

    return 0;
}