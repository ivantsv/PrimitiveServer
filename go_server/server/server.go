package main // В go программа делится на пакеты. package main - пакет, в котором компилятору нужно искать точку входа в программу

import (
	"fmt"	// Форматированный вывод в консоль
	"log"
	"net"
)

func handleClient(conn net.Conn) {
	// defer гарантирует выполнение даже если функция завершится ошибкой или паникой
	defer conn.Close()

	// Отправляем клиенту сообщение (принимает []byte, поэтому конвертируем) []byte - срез байтов (динамический массив, byte - просто define uint8)
	// Синтаксис конвертации в Go: Type(value)
	_, err := conn.Write([]byte("OK\n"))
	if err != nil {
		log.Printf("Failed to send response: %v", err)
		return
	}
	
	fmt.Println("Client served successfully")
}

func main() {
	// Создаем TCP listener на порту 8080 и всех интерфейсах. Можно было бы написать localhost:8080/127.0.0.1:8080
	listener, err := net.Listen("tcp", ":8080")
	if err != nil {
		log.Fatal("Failed to start server", err)
	}

	// defer - откладывает выполнение функции до тех пор, пока не завершится текущая функция
	defer listener.Close()

	fmt.Println("Server listening on port 8080...")

	// Запускаем бесконечный цикл
	for {
		// Ожидаем входящее соединение
		// Accept() блокирует выполнение до появления клиента
		conn, err := listener.Accept();
		if err != nil {
			log.Printf("Failed to accept connection: %v", err)
			continue
		}

		// Запускаем горутину-обработчик клиента
		go handleClient(conn)
	}
}
