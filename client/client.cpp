#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <tuple>

#pragma comment(lib, "ws2_32.lib")

constexpr auto BackBtnAction = 3;
constexpr auto Task1BtnAction = 4;
constexpr auto Task2BtnAction = 5;
constexpr auto DisconnectBtnAction = 6;

bool isMenu = true;
bool isServer1 = false;
bool isServer2 = false;

SOCKET clientSocket1;
SOCKET clientSocket2;

bool isConnectedServer1 = false;
bool isConnectedServer2 = false;

LRESULT CALLBACK SoftwareMainProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
WNDCLASS NewWindowClass(HBRUSH BGColor, HCURSOR Cursor, HINSTANCE hInst, HICON Icon, LPCWSTR Name, WNDPROC Procedure);
void MainWndAddWidgets(HWND hWnd);

HWND backBtn;
HWND task1Btn;
HWND task2Btn;
HWND disconnectBtn;


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
        return std::make_tuple(clientSocket, "Неверный адрес сервера", false);
    }

    // Подключаемся к серверу
    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR) {
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



// Функция для инициализации графического интерфейса
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdshow) {
    WNDCLASS SoftwareMainClass = NewWindowClass((HBRUSH)COLOR_WINDOW, LoadCursor(NULL, IDC_ARROW), hInst, LoadIcon(NULL, IDI_QUESTION),
        L"MainWindClass", SoftwareMainProcedure);

    if (!RegisterClassW(&SoftwareMainClass)) { return -1; }
    MSG SoftwareMainMessage = { 0 };

    CreateWindow(L"MainWindClass", L"Client", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX | WS_VISIBLE, 100, 100, 500, 250, NULL, NULL, NULL, NULL);

    while (GetMessage(&SoftwareMainMessage, NULL, NULL, NULL)) {
        TranslateMessage(&SoftwareMainMessage);
        DispatchMessage(&SoftwareMainMessage);
    }

    return 0;
}

// Функция для создания класса окна графического интерфейса
WNDCLASS NewWindowClass(HBRUSH BGColor, HCURSOR Cursor, HINSTANCE hInst, HICON Icon, LPCWSTR Name, WNDPROC Procedure) {
    WNDCLASS NWC = { 0 };

    NWC.hCursor = Cursor;
    NWC.hIcon = Icon;
    NWC.hInstance = hInst;
    NWC.lpszClassName = Name;
    NWC.hbrBackground = BGColor;
    NWC.lpfnWndProc = Procedure;

    return NWC;
}

// Функция обработки комманд графического интерфейса
LRESULT CALLBACK SoftwareMainProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_COMMAND:
        switch (wp) {
        case BackBtnAction:                         // Команда для возвращения в главное меню
            ShowWindow(backBtn, SW_HIDE);
            ShowWindow(disconnectBtn, SW_HIDE);
            SetWindowText(task1Btn, L"Сервер 1");
            SetWindowText(task2Btn, L"Сервер 2");
            isServer1 = false;
            isServer2 = false;
            isMenu = true;
            ShowWindow(task1Btn, SW_SHOW);
            ShowWindow(task2Btn, SW_SHOW);
            break;
        case Task1BtnAction:                        // Команда для запуска сервера 1/отправления первого запроса серверу N
            if (isMenu) {
                if (!isConnectedServer1) {
                    std::string msg;
                    bool ok;
                    tie(clientSocket1, msg, ok) = connectToServer(8888);
                    if (!ok) {
                        MessageBoxA(hWnd, (LPCSTR)msg.c_str(), "Ошибка подключения", MB_OK);
                        closesocket(clientSocket1);
                        break;
                    }
                    isConnectedServer1 = true;
                }
                ShowWindow(backBtn, SW_SHOW);
                ShowWindow(disconnectBtn, SW_SHOW);
                SetWindowText(task1Btn, L"Количество клавиш мыши");
                SetWindowText(task2Btn, L"Наличие колеса прокрутки");
                ShowWindow(task1Btn, SW_SHOW);
                ShowWindow(task2Btn, SW_SHOW);
                isServer1 = true;
                isMenu = false;
            }
            else if (isServer1) {
                std::string response;
                bool ok;
                tie(response, ok) = getResponse(clientSocket1, "numButtons");
                if (!ok) {
                    MessageBoxA(hWnd, (LPCSTR)response.c_str(), "Ошибка получения ответа от сервера", MB_OK);
                    break;
                }
                MessageBoxA(hWnd, (LPCSTR)response.c_str(), "Ответ сервера", MB_OK);
            }
            else if (isServer2) {
                std::string response;
                bool ok;
                tie(response, ok) = getResponse(clientSocket2, "pid");
                if (!ok) {
                    MessageBoxA(hWnd, (LPCSTR)response.c_str(), "Ошибка получения ответа от сервера", MB_OK);
                    break;
                }
                MessageBoxA(hWnd, (LPCSTR)response.c_str(), "Ответ сервера", MB_OK);
            }
            break;
        case Task2BtnAction:                        // Команда для запуска сервера 2/отправления второго запроса серверу N
            if (isMenu) {
                if (!isConnectedServer2) {
                    std::string msg;
                    bool ok;
                    tie(clientSocket2, msg, ok) = connectToServer(8889);
                    if (!ok) {
                        MessageBoxA(hWnd, (LPCSTR)msg.c_str(), "Ошибка подключения", MB_OK);
                        closesocket(clientSocket2);
                        break;
                    }
                    isConnectedServer2 = true;
                }
                ShowWindow(backBtn, SW_SHOW);
                ShowWindow(disconnectBtn, SW_SHOW);
                SetWindowText(task1Btn, L"Идентификатор серверного процесса");
                SetWindowText(task2Btn, L"Время работы серверного процесса в пользовательском режиме");
                ShowWindow(task1Btn, SW_SHOW);
                ShowWindow(task2Btn, SW_SHOW);
                isServer2 = true;
                isMenu = false;
            }
            else if (isServer1) {
                std::string response;
                bool ok;
                tie(response, ok) = getResponse(clientSocket1, "hasMouseWheel");
                if (!ok) {
                    MessageBoxA(hWnd, (LPCSTR)response.c_str(), "Ошибка получения ответа от сервера", MB_OK);
                    break;
                }
                MessageBoxA(hWnd, (LPCSTR)response.c_str(), "Ответ сервера", MB_OK);
            }
            else if (isServer2) {
                std::string response;
                bool ok;
                tie(response, ok) = getResponse(clientSocket2, "userTime");
                if (!ok) {
                    MessageBoxA(hWnd, (LPCSTR)response.c_str(), "Ошибка получения ответа от сервера", MB_OK);
                    break;
                }
                MessageBoxA(hWnd, (LPCSTR)response.c_str(), "Ответ сервера", MB_OK);
            }
            break;
        case DisconnectBtnAction:                   // Команда для отключения от сервера N
            if (isServer1) {
                getResponse(clientSocket1, "stop");
                closesocket(clientSocket1);
                isConnectedServer1 = false;
            }
            else if (isServer2) {
                getResponse(clientSocket2, "stop");
                closesocket(clientSocket2);
                isConnectedServer2 = false;
            }
            ShowWindow(backBtn, SW_HIDE);
            ShowWindow(disconnectBtn, SW_HIDE);
            SetWindowText(task1Btn, L"Сервер 1");
            SetWindowText(task2Btn, L"Сервер 2");
            ShowWindow(task1Btn, SW_SHOW);
            ShowWindow(task2Btn, SW_SHOW);
            isServer1 = false;
            isServer2 = false;
            isMenu = true;
            break;
        }
        break;
    case WM_CREATE:                                 // Код, выполняющийся при запуске графического интерфейса
        MainWndAddWidgets(hWnd);
        // Инициализация библиотеки Winsock
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "Ошибка при инициализации Winsock" << std::endl;
            return 1;
        }
        break;
    case WM_DESTROY:                                // Код, выполняющийся при закрытии графического интерфейса
        if (isConnectedServer1) {
            getResponse(clientSocket1, "stop");
            closesocket(clientSocket1);
        }
        if (isConnectedServer2) {
            getResponse(clientSocket2, "stop");
            closesocket(clientSocket2);
        }
        WSACleanup();
        PostQuitMessage(0);
        break;
    default: return DefWindowProc(hWnd, msg, wp, lp);
    }
}

// Функция добавления элементов в графический интерфейс
void MainWndAddWidgets(HWND hWnd) {
    backBtn = CreateWindowA("button", "Назад", WS_CHILD | BS_MULTILINE | ES_CENTER, 5, 20, 100, 60, hWnd, (HMENU)BackBtnAction, NULL, NULL);
    task1Btn = CreateWindowA("button", "Сервер 1", WS_VISIBLE | WS_CHILD | BS_MULTILINE | ES_CENTER, 175, 10, 150, 90, hWnd, (HMENU)Task1BtnAction, NULL, NULL);
    task2Btn = CreateWindowA("button", "Сервер 2", WS_VISIBLE | WS_CHILD | BS_MULTILINE | ES_CENTER, 175, 105, 150, 90, hWnd, (HMENU)Task2BtnAction, NULL, NULL);
    disconnectBtn = CreateWindowA("button", "Отключиться от сервера", WS_CHILD | BS_MULTILINE | ES_CENTER, 5, 150, 100, 60, hWnd, (HMENU)DisconnectBtnAction, NULL, NULL);
}