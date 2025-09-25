#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib") // доподключаем реализацию библиотеку WSA
#include <chrono>
#include <iomanip>
#include "CIN_colors.h"


void Server(); // тело самого сервера

std::string GetCurrentTimeStr(); // функция, возвращающая настоящее время (чч:мм:сс)

int main() {
	// первоночальная настройка
	CIN_colors_Setup();

	// серверная часть
	Server();

	CIN_set_color(DARK_GRAY_COLOR);
	std::cout << GetCurrentTimeStr();
	CIN_set_color(RED_COLOR);
	std::cout << " Сервер зарыт!" << std::endl;
	CIN_set_color(WHITE_COLOR);
}

void Server() {
	// запуск WSA
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &wsaData) == WSASYSNOTREADY) {
		CIN_set_color(DARK_GRAY_COLOR);
		std::cout << GetCurrentTimeStr();
		CIN_set_color(RED_COLOR);
		std::cout << " Ошибка запуска WSA!";
		CIN_set_color(WHITE_COLOR);
		std::cout << std::endl;
		return;	// выход
	}

	bool is_good{ false };
	int bad_count{ 0 };
	SOCKET door;
	while (is_good == false) {
		if ((door = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
			if (bad_count > 0 && bad_count < 10) {
				CIN_set_color(DARK_GRAY_COLOR);
				std::cout << GetCurrentTimeStr();
				CIN_set_color(RED_COLOR);
				std::cout << " Не получилось открыть сокет!";
				CIN_set_color(WHITE_COLOR);
				std::cout << " попытка: " << bad_count << std::endl;
			}
			else if (bad_count >= 10) {
				CIN_set_color(DARK_GRAY_COLOR);
				std::cout << GetCurrentTimeStr();
				CIN_set_color(RED_COLOR);
				std::cout << " Не получилось открыть сокет!";
				CIN_set_color(WHITE_COLOR);
				std::cout << " попытки закончились, завершаю работу!" << std::endl;
				WSACleanup();
				return;	// выход
			}
			else {
				CIN_set_color(DARK_GRAY_COLOR);
				std::cout << GetCurrentTimeStr();
				CIN_set_color(RED_COLOR);
				std::cout << " Не получилось открыть сокет!";
				CIN_set_color(WHITE_COLOR);
				std::cout << " Попробую снова..." << std::endl;
			}
			bad_count++;
		}
		else {	// все успешно создалось
			CIN_set_color(DARK_GRAY_COLOR);
			std::cout << GetCurrentTimeStr();
			CIN_set_color(GREEN_COLOR);
			std::cout << " Сокет успешно создан!";
			CIN_set_color(WHITE_COLOR);
			std::cout << std::endl;
			is_good = true;
		}
	}

	// подготавливаем структуру для bind()
	sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(60888); // задаем порт сервера
	local.sin_addr.s_addr = inet_addr("127.0.0.1"); // задает IP сервера

	if (bind(door, (sockaddr*)&local, sizeof(local)) == SOCKET_ERROR) { // привязка сокета к IP:порту
		CIN_set_color(DARK_GRAY_COLOR);
		std::cout << GetCurrentTimeStr();
		CIN_set_color(RED_COLOR);
		std::cout << " Не удалось привязать сокет к IP:порт!";
		CIN_set_color(WHITE_COLOR);
		std::cout << " заканчиваю работу!" << std::endl;
		WSACleanup();
		return;	// выход
	}
	CIN_set_color(DARK_GRAY_COLOR);
	std::cout << GetCurrentTimeStr();
	CIN_set_color(GREEN_COLOR);
	std::cout << " IP:порт успешно привязаны!";
	CIN_set_color(WHITE_COLOR);
	std::cout << std::endl;
	
	if (listen(door, 5) == -1) {
		CIN_set_color(DARK_GRAY_COLOR);
		std::cout << GetCurrentTimeStr();
		CIN_set_color(RED_COLOR);
		std::cout << " Не удалось поставить сокет на прослушивание (listen)!";
		CIN_set_color(WHITE_COLOR);
		std::cout << " заканчиваю работу!" << std::endl;
		WSACleanup();
		return;	// выход
	}
	CIN_set_color(DARK_GRAY_COLOR);
	std::cout << GetCurrentTimeStr();
	CIN_set_color(WHITE_COLOR);
	std::cout << " Сокет находится в режиме прослушки (listen)..." << std::endl;
	
	SOCKET connection;
	sockaddr_in connection_addr;
	int size = sizeof(connection_addr);
	bad_count = 0;
	while (true) {	// беск цикл для работы сервера
		connection = accept(door, (sockaddr*)&connection_addr, &size); // блокирующий accept
		if (connection == INVALID_SOCKET) {
			CIN_set_color(DARK_GRAY_COLOR);
			std::cout << GetCurrentTimeStr();
			CIN_set_color(RED_COLOR);
			std::cout << " Неудачная попытка подключения!";
			CIN_set_color(WHITE_COLOR);
			if (bad_count > 0)
				std::cout << " попытка: " << bad_count;
			std::cout << std::endl;
			bad_count++;
		}
		else { // получили хороший сокет (подключения)
			CIN_set_color(DARK_GRAY_COLOR);
			std::cout << GetCurrentTimeStr();
			CIN_set_color(GREEN_COLOR);
			std::cout << " Соединение установлено!";
			CIN_set_color(WHITE_COLOR);
			std::cout << std::endl;
			bad_count = 0;

			char* massive_buff = nullptr;
			int32_t massive_buff_size{ 0 };
			char buff[1024]; // буффер в 1Кб
			int32_t real_count{ 0 };
			while ((real_count = recv(connection, buff, 1024, 0)) > 0) {	// значит это именно чтение!
				if (massive_buff != nullptr) { // пересодаем
					char* tmp_massive_buff = new char[massive_buff_size + real_count - ((buff[real_count - 1] == '\n') ? 1 : 0)];
					for (int32_t i{ 0 }; i < massive_buff_size; i++)
						tmp_massive_buff[i] = massive_buff[i]; // скопировали старые знач
					for (int32_t i{ massive_buff_size }; i < massive_buff_size + real_count - ((buff[real_count - 1] == '\n') ? 1 : 0); i++)
						tmp_massive_buff[i] = buff[i - massive_buff_size]; // добрали новые значения

					delete[] massive_buff;
					massive_buff = tmp_massive_buff;
					massive_buff_size += real_count - ((buff[real_count - 1] == '\n') ? 1 : 0);
				}
				else { // создаем чисто новый
					massive_buff = new char[real_count - ((buff[real_count - 1] == '\n') ? 1 : 0)];
					for (int32_t i{ 0 }; i < real_count - ((buff[real_count - 1] == '\n') ? 1 : 0); i++) // на всякий, чтобы не копировать '\n'
						massive_buff[i] = buff[i];	// перевели значения
					massive_buff_size = real_count - ((buff[real_count - 1] == '\n') ? 1 : 0);
				}
				// заполнили наш общий массив

				if (buff[real_count - 1] == '\n') { // означает конец чтения!
					CIN_set_color(DARK_GRAY_COLOR);
					std::cout << GetCurrentTimeStr();
					CIN_set_color(GREEN_COLOR);
					std::cout << " Все пакеты приняты!";
					CIN_set_color(WHITE_COLOR);
					std::cout << std::endl;

					int32_t* type_tmp = reinterpret_cast<int32_t*>(massive_buff); // переводим первые 4 байта в int32_t
					if (*type_tmp == 0) { // просто вычисление
						int32_t* count_tmp = reinterpret_cast<int32_t*>(massive_buff + sizeof(int32_t)); // прочитали количество int-ов
						int32_t answer_count[2];
						answer_count[0] = 0;
						int32_t* buff_int32_t = nullptr;
						for (int32_t i{ 0 }; i < *count_tmp; i++) { // перебираем все числа
							buff_int32_t = reinterpret_cast<int32_t*>(massive_buff + 2 * sizeof(int32_t) + i * sizeof(int32_t));
							if (*buff_int32_t % 3 == 0 && *buff_int32_t != 0)
								answer_count[0]++;
						}
						/// посчитали -> теперь надо отправить

						char* for_send = reinterpret_cast<char*>(&answer_count);
						for_send[4] = '\n';

						int32_t total{ 0 }; // для доссылки
						while (total < 5) {
							int sent = send(connection, for_send + total, 5 - total, 0);
							if (sent == SOCKET_ERROR) {
								CIN_set_color(DARK_GRAY_COLOR);
								std::cout << GetCurrentTimeStr();
								CIN_set_color(RED_COLOR);
								std::cout << " Ошибка отправки!";
								CIN_set_color(WHITE_COLOR);
								std::cout << std::endl;
								break;
							}
							total += sent; // дописываем точно отправленные
						}
						if (total == 5) {
							CIN_set_color(DARK_GRAY_COLOR);
							std::cout << GetCurrentTimeStr();
							CIN_set_color(GREEN_COLOR);
							std::cout << " Ответ отправлен упешно!";
							CIN_set_color(WHITE_COLOR);
							std::cout << std::endl;
						}
						
						if (massive_buff != nullptr)
							delete[] massive_buff;

						closesocket(connection);

						CIN_set_color(DARK_GRAY_COLOR);
						std::cout << GetCurrentTimeStr();
						CIN_set_color(WHITE_COLOR);
						std::cout << " Соединение разоварнно" << std::endl;

						break;
					}
					else { // отключение сервера !
						CIN_set_color(DARK_GRAY_COLOR);
						std::cout << GetCurrentTimeStr();
						CIN_set_color(WHITE_COLOR);
						std::cout << " Команда: закрыть сервер!" << std::endl;

						closesocket(connection);

						CIN_set_color(DARK_GRAY_COLOR);
						std::cout << GetCurrentTimeStr();
						CIN_set_color(WHITE_COLOR);
						std::cout << " Соединение разоварнно" << std::endl;
						
						if (massive_buff != nullptr)
							delete[] massive_buff;

						CIN_set_color(DARK_GRAY_COLOR);
						std::cout << GetCurrentTimeStr();
						CIN_set_color(WHITE_COLOR);
						std::cout << " заканчиваю работу!" << std::endl;

						WSACleanup();
						return;
					}
				}
				else { // значит придет еще пакет!
					CIN_set_color(DARK_GRAY_COLOR);
					std::cout << GetCurrentTimeStr();
					CIN_set_color(WHITE_COLOR);
					std::cout << " Пакет принят!" << std::endl;
				}
			}
		}
	}

	CIN_set_color(DARK_GRAY_COLOR);
	std::cout << GetCurrentTimeStr();
	CIN_set_color(WHITE_COLOR);
	std::cout << " заканчиваю работу!" << std::endl;

	WSACleanup(); // закрытие ОС стека WSA
}

std::string GetCurrentTimeStr() {
	auto now = std::chrono::system_clock::now(); // системное время
	time_t now_t = std::chrono::system_clock::to_time_t(now); // перевели в time_t

	std::tm tm_info{};
	localtime_s(&tm_info, &now_t);

	char buffer[8 + 1];  // 8 символов + '\0'
	strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm_info);

	return buffer;
}
