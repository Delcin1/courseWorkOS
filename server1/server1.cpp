#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <cstdio>

#pragma comment(lib, "ws2_32.lib")

// Функция проверки на повторный запуск
bool IsAnotherInstanceRunning() {
    std::string lockFilePath = "server1.lock";

    // Попытка открыть файл блокировки в режиме чтения
    std::ifstream lockFile(lockFilePath);
    if (lockFile.is_open()) {
        // Файл блокировки уже существует, значит другой экземпляр сервера уже запущен
        lockFile.close();
        return true;
    }

    // Файл блокировки не существует, создаем его
    std::ofstream newLockFile(lockFilePath);
    if (newLockFile.is_open()) {
        // Записываем что-нибудь в файл блокировки
        newLockFile << "Server lock file";
        newLockFile.close();
        return false;
    }

    // Не удалось создать файл блокировки
    return true;
}

// Функция удаления файла блокировки
void removeLockFile() {
    std::remove("server1.lock");
}

// Обработчик события завершения приложения
BOOL WINAPI ConsoleCtrlHandler(DWORD ctrlType) {
    if (ctrlType == CTRL_CLOSE_EVENT || ctrlType == CTRL_C_EVENT) {
        std::cout << "Закрытие сервера..." << std::endl;
        removeLockFile(); // Удаляем файл блокировки при закрытии через событие CTRL_CLOSE_EVENT
        exit(0);
    }
    return FALSE;
}

// Функция для обработки клиента
void handleClient(SOCKET clientSocket) {
    char buffer[1024];
    bool mustClose = false;

    while (!mustClose) {
        // Получаем запрос от клиента
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead == SOCKET_ERROR) {
            std::cerr << "Ошибка при чтении запроса от клиента: " << WSAGetLastError() << std::endl;
            mustClose = true;
        }
        else if (bytesRead == 0) {
            // Клиент закрыл соединение
            mustClose = true;
        }
        else {
            std::cout << "Получаем запрос от клиента" << std::endl;

            // Парсим запрос
            std::string request(buffer, bytesRead);
            // Отправляем соответствующее сообщение клиенту
            std::string response;
            if (request == "numButtons") {
                response = "Количество кнопок мыши: " + std::to_string(GetSystemMetrics(SM_CMOUSEBUTTONS));
            }
            else if (request == "hasMouseWheel") {
                response = "Наличие колеса прокрутки: ";
                if (GetSystemMetrics(SM_MOUSEWHEELPRESENT) != 0) {
                    response += "есть";
                }
                else {
                    response += "нет";
                }
            }
            else if (request == "stop") {
                response = "Соединение закрыто";
                mustClose = true;
            }
            else {
                response = "Неверный запрос";
            }

            // Отправляем ответ клиенту
            int bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
            if (bytesSent == SOCKET_ERROR) {
                std::cerr << "Ошибка при отправке ответа клиенту: " << WSAGetLastError() << std::endl;
                mustClose = true;
            }
            std::cout << " Отправляем ответ клиенту" << std::endl;
        }
    }

    // Закрываем соединение с клиентом
    closesocket(clientSocket);
}

int main() {
    // Проверка на повторный запуск
    if (IsAnotherInstanceRunning()) {
        std::cout << "Другой экземпляр сервера уже запущен." << std::endl;
        return 0;
    }

    // Установка обработчика события завершения приложения
    if (!SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE)) {
        std::cerr << "Не удалось установить обработчик события завершения" << std::endl;
        removeLockFile(); // Удаляем файл блокировки при ошибке
        return 1;
    }

    // Установка русского языка для вывода в консоли
    std::setlocale(LC_ALL, "Russian");

    // Инициализация библиотеки Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Ошибка при инициализации Winsock" << std::endl;
        return 1;
    }

    // Создаем сокет для прослушивания клиентов
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "Ошибка при создании сокета: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Привязываем сокет к адресу и порту
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8888); // Используем порт 8888

    if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Ошибка при привязке сокета к адресу: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // Начинаем прослушивание клиентов
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Ошибка при начале прослушивания сокета: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Сервер запущен. Ожидание подключений..." << std::endl;

    std::vector<std::thread> clientThreads;

    while (true) {
        // Принимаем входящее подключение
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Ошибка при принятии подключения: " << WSAGetLastError() << std::endl;
            closesocket(listenSocket);
            WSACleanup();
            return 1;
        }

        // Создаем отдельный поток для обработки клиента
        std::thread clientThread(handleClient, clientSocket);
        clientThreads.push_back(std::move(clientThread));
    }

    // Дожидаемся завершения всех потоков
    for (std::thread& thread : clientThreads) {
        thread.join();
    }

    // Закрываем сокет для прослушивания
    closesocket(listenSocket);

    // Выполняем чистку Winsock
    WSACleanup();

    removeLockFile();

    return 0;
}