#include <iostream>
#include <winsock2.h>
#include <sstream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

mutex coutMutex;

// Iniciar Winsock
bool initializeWinsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cerr << "WSAStartup failed: " << result << endl;
        return false;
    }
    return true;
}

// Finalizar Winsock
void cleanupWinsock() {
    WSACleanup();
}

// Verificar si un puerto está abierto
bool isPortOpen(const char* ip, int port) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        lock_guard<mutex> lock(coutMutex);
        cerr << "Socket creation failed: " << WSAGetLastError() << endl;
        return false;
    }

    // Establecer un tiempo de espera de 1 segundo para la conexión
    int timeout = 1000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));



    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_port = htons(port);

    bool result = connect(sock, (sockaddr*)&server, sizeof(server)) == 0;

    closesocket(sock);
    return result;
}

void scanRange(int start, int end, int port) {
    for (int i = start; i < end; i++) {
        for (int j = 1; j < 256; j++) {
            for (int k = 0; k < 256; k++) {
                stringstream ss;
                ss << "192.168." << i << "." << j;
                string ip = ss.str();

                if (isPortOpen(ip.c_str(), port)) {
                    lock_guard<mutex> lock(coutMutex);
                    cout << "El puerto " << port << " en la IP " << ip << " esta abierto." << endl;
                } else {
                    lock_guard<mutex> lock(coutMutex);
                    cout << "El puerto " << port << " en la IP " << ip << " esta cerrado o inaccesible." << endl;
                }
            }
        }
    }
}


int main() {
    int port = 80;
    int numThreads = 8; // Número de hilos
    vector<thread> threads;

    if (!initializeWinsock()) {
        cerr << "Error al inicializar Winsock" << endl;
        return 1;
    }

    cout << "Iniciando escaneo de puertos..." << endl;

    int rangePerThread = 255 / numThreads;
    for (int i = 0; i < numThreads; ++i) {
        int start = i * rangePerThread;
        int end = (i == numThreads - 1) ? 255 : start + rangePerThread - 1;
        threads.emplace_back(scanRange, start, end, port);
    }

    for (auto& t : threads) {
        t.join();
    }

    cleanupWinsock();
    cout << "Escaneo de puertos finalizado." << endl;
    return 0;
}
