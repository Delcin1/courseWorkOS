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


// Функция подключения к серверу
std::tuple<SOCKET, std::string, bool> connectToServer(int port) {
    // Создаем сокет для подключения к серверу
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        return std::make_tuple(clientSocket, "Ошибка при создании сокета: " + std::to_string(WSAGetLastError()), false);
    }

    // Устанавливаем адрес и порт сервера
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port); // Используем порт 8888
    if (inet_pton(AF_INET, "127.0.0.1", &(serverAddress.sin_addr)) <= 0) {
        closesocket(clientSocket);
        return std::make_tuple(clientSocket, "Неверный адрес сервера", false);
    }

    // Подключаемся к серверу
    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR) {
        closesocket(clientSocket);
        return std::make_tuple(clientSocket, "Ошибка при подключении к серверу: " + std::to_string(WSAGetLastError()), false);
    }
    return std::make_tuple(clientSocket, "OK", true);
}

// Функция для отправки запроса на сервер и получения ответа от него
std::tuple<std::string, bool> getResponse(SOCKET clientSocket, std::string request) {
    // Отправляем запрос серверу
    int bytesSent = send(clientSocket, request.c_str(), request.length(), 0);
    if (bytesSent == SOCKET_ERROR) {
        closesocket(clientSocket);
        return std::make_tuple("Ошибка при отправке запроса: " + std::to_string(WSAGetLastError()), false);
    }

    // Получаем ответ от сервера
    char buffer[1024];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead == SOCKET_ERROR) {
        closesocket(clientSocket);
        return std::make_tuple("Ошибка при чтении ответа: " + std::to_string(WSAGetLastError()), false);
    }

    // Возвращаем полученный ответ
    std::string response(buffer, bytesRead);
    return std::make_tuple(response, true);
}

int main() {
    // Установка русского языка для вывода в консоли
    std::setlocale(LC_ALL, "Russian");

    // Инициализация библиотеки Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Ошибка при инициализации Winsock" << std::endl;
        return 1;
    }

    // Вывод доступных команд
    std::cout << "Доступные команды: \n connectServer1 и connectServer2 - подключение к серверам \n disconnectServer1 и disconnectServer2 - отключение от серверов \n Сервер 1: \n  numButtons - количество клавиш мыши \n  hasMouseWheel - наличие колеса прокрутки \n Сервер 2: \n  pid - идентификатор серверного процесса \n  userTime - время работы серверного процесса в пользовательском режиме \nquit - закрыть программу" << std::endl;
    
    // Чтение и обработка сообщенных пользователем команд
    std::string msg;
    std::cout << "Введите команду" << std::endl;
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
                std::cout << "Попробуйте переподключиться к серверу" << std::endl;
            }
        }
        else if (msg == "hasMouseWheel") {
            tie(resp, ok) = getResponse(clientSocket1, msg);
            std::cout << resp << std::endl;
            if (!ok) {
                std::cout << "Попробуйте переподключиться к серверу" << std::endl;
            }
        }
        else if (msg == "pid") {
            tie(resp, ok) = getResponse(clientSocket2, msg);
            std::cout << resp << std::endl;
            if (!ok) {
                std::cout << "Попробуйте переподключиться к серверу" << std::endl;
            }
        }
        else if (msg == "userTime") {
            tie(resp, ok) = getResponse(clientSocket2, msg);
            std::cout << resp << std::endl;
            if (!ok) {
                std::cout << "Попробуйте переподключиться к серверу" << std::endl;
            }
        }
        else {
            std::cout << "Такая команда не найдена" << std::endl;
        }
        std::cout << "Введите команду" << std::endl;
        std::cin >> msg;
    }

    // Выполняем чистку Winsock
    WSACleanup();

    return 0;
}