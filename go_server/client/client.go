package main

import (
	"bufio"   // Буферизированное чтение строк
	"fmt"
	"net"
	"os"  	  // Для завершения программы с кодом возврата
	"strings" // Для работы со строками
)

// В GO main не возвращает ничего кроме кода возврата (реального), так что делаем os.Exit(1)
func main() {
	// Подключаемся к серверу по TCP на localhost:8080
	// net.Dial создает соединение и возвращает интерфейс net.Con
	conn, err := net.Dial("tcp", "localhost:8080")
	if err != nil {
		fmt.Printf("Failed to connect server: %v\n", err)
		os.Exit(1)
	}

	// Отправляет TCP FIN пакет серверу и освобождает ресурсы
	defer conn.Close()

	// Создаем буферизированный reader для удобного чтения (оборачиваем в него структуру conn)
	reader := bufio.NewReader(conn)

	// Читаем нашу строку до символа \n
	response, err := reader.ReadString('\n')
	if err != nil {
		fmt.Printf("Failed to read from server: %v\n", err)
		os.Exit(1)
	}

	expected := "OK\n"

	if response == expected {
		fmt.Println("SUCCESS: Received correct response from server")
		os.Exit(0)
	} else {
		fmt.Printf("ERROR: Expected '%s', but received '%s'\n",
			strings.ReplaceAll(expected, "\n", "\\n"),				// Заменяем для красивого вывода
			strings.ReplaceAll(response, "\n", "\\n"))
		
		os.Exit(1)
	}
}
