#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

// 1. Создание сокета
// 2. Настройка адреса сервера
// 3. Подключение к серверу
// 4. Получение данных
// 5. Проверка корректности ответа
// 6. Закрытие соединения

#define IPv4 AF_INET
#define TCP SOCK_STREAM

// Важный момент - тут не exit, а return -1, потому что сервер - процесс длительный, а клиент - быстрый. Подключились, проверили и все.

int main() {
    // Создаем сокет
    int sock = socket(IPv4, TCP, 0);
    if (sock < 0) {
        perror("socket failed");
        return -1;
    }

    // Настраиваем адрес для подключения к серверу. 127.0.0.1 - localhost(сама машина).
    sockaddr_in server_address;
    {
        server_address.sin_family = IPv4;
        server_address.sin_port = htons(8080);
    }

    // Переводим адрес из строки в sockaddr_in
    if (inet_pton(IPv4, "127.0.0.1", &server_address.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        return -1;
    }

    // Устанавливаем соединение с сервером
    if (connect(sock, (sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        close(sock);
        return -1;
    }

    // Создаем буффер и читаем в него
    char buffer[1024] = {0}; // Инициализация всех элементов нулями (Привет, C :))
    int value_read = read(sock, buffer, 1024);

    // Проверка успешности чтения
    if (value_read < 0) {
        perror("Read failed");
        close(sock);
        return -1;
    }

    // Проверка того, что прочитали
    const char* expected = "OK\n";
    if (strcmp(buffer, expected) == 0) {
        std::cout << "SUCCESS: Received correct response: " << buffer;
        close(sock);
        return 0;
    } else {
        std::cout << "ERROR: Expected 'OK\\n', but received: '" << buffer << "'" << std::endl;
        close(sock);
        return 1;
    }
}