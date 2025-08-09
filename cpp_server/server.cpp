#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

// 1. Создание сокета
// 2. Настройка адреса
// 3. Привязка к порту
// 4. Начало прослушивания
// 5. Основной цикл обработки клиентов
// 6. Обработка ошибок и очистка

// Что ознаачают (не точное значение) всякие глобальные константы
#define IPv4 AF_INET
#define TCP SOCK_STREAM

// Обработчки соединения с клиентом
void handle_client(int client_socket) {
    const char* message = "OK\n";
    send(client_socket, message, strlen(message), 0); // Интуитивно все понятно. 0 - флаги (их отсутствие)
    std::cout << "Message sent to client" << std::endl;
    close(client_socket); // Закрываем соединение с клиентом
}

int main() {
    // Создание сокета (ошибка -> вернет -1)
    int server_fd = socket(IPv4, TCP, 0);
    if (server_fd < 0) {
        // Есть глобальная переменная errno, где хранятся ошибки при системных вызовах. perror возьмет текст ошибки оттуда
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Настройка сокета: можно было быстро перезапускать сервер, не дожидаясь освобождения порта. 
    // Дополнительно разрешает нескольким процессам слушать один порт (с SO_REUSEPORT)
    // Не удалось - ошибка и завершение
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Структура для хранения IPv4 адреса
    sockaddr_in address;
    size_t address_length = sizeof(address);

    // Создаем структуру с IPv4 адресом, портом 8080, слушающую на всех интерфейсах. Готовим ее к использованию в bind
    {
        address.sin_family = IPv4;
        address.sin_addr.s_addr = INADDR_ANY; //IP адрес, к которому привяжем сокет. 0.0.0.0 - принимать соединения на всех доступных сетевых интерфейсах
        address.sin_port = htons(8080); // htons преобразует в сетевой порядок байт (big-endian)
    }

    // Системный вызов, который пытается привязать сокет к паре (IP-address, port)
    if (bind(server_fd, (sockaddr*)(&address), sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);   // Закрываем сокет, порт занять не удалось
        exit(EXIT_FAILURE); 
    }
    // Когда bind может упасть: порт уже занят, у процесса нет прав, IP не принадлежит машине, сокет уже был привязан
    
    // Системный вызов, говорит ОС, что сокет должен слушать входящие соединения. 3 - размер очереди ожиданий
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    // listen может упасть по тем же самым причинам (Практически. Если падает, то точно мы накосячили.)

    std::cout << "Server listening on port 8080" << std::endl;

    // Основной цикл исполнения
    while (true) {
        // Системный вызов, который ждет входящее соединение от клиента. Параметры: сокет, который слушает порт; указатель на структуру
        // куда будет записан адрес клиента; указатель на размер этой структуры, может быть изменен функцией accept
        // Возвращает новый файловый дескриптор - отдельный сокет для общения с конкретным клиентом.
        int client_socket = accept(server_fd, (sockaddr*)&address, (socklen_t*)&address_length);
        if (client_socket < 0) {
            perror("accept");
            continue;
        }

        // fork - системный вызов, создание копии процесса (copy-on-write!! (эффективно))
        // В родительском процессе форк возвращает PID(process ID) > 0 дочернего
        // В дочернем процессе форк возвращает PID == 0
        // Создание нового процесса куда более дорогостоящая, но более безопасная операция, чем создание потока.
        pid_t pid = fork();
        if (pid == 0) {
            // Дочерний процесс, закрываем слушающий сокет, обрабатываем клиента, завершаем дочерний процесс
            close(server_fd);
            handle_client(client_socket);
            exit(0);
        } else if (pid > 0) {
            // Родительский процесс, закрываем клиентский сокет, т.к. он обрабатывается в дочернем
            close(client_socket);
        } else {
            // Ошибка форка
            perror("fork failed");
            close(client_socket);
        }
    }
}