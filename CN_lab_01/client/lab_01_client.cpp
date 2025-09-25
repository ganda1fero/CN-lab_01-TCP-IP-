#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib") // доподключаем реализацию библиотеку WSA
#pragma warning(disable:4996)
#include <iomanip>
#include "EasyMenu.h"
#include "CIN_colors.h"

void Client();
void Client_kill();

int main() {
	CIN_colors_Setup();
	
	EasyMenu menu("Ввести числа", "Закрыть сервер", "Выход");

	while (true) {
		switch (menu.easy_run())
		{
		case 0:
			Client();
			break;
		case 1:
			Client_kill();
			return 0;
			break;
		default: // выход
			return 0;
			break;
		}
	}
}

void Client() {
	int32_t buffer_int{ 0 };
	int32_t count_of_number;
	int32_t* arr = nullptr;
	while (true) {
		std::cout << "Введите количество чисел: ";
		std::cin >> count_of_number;
		if (std::cin.good() == false) {
			std::cout << "ошибка воода, попробуйте снова!\n";
			while (std::cin.peek() != '\n')
				std::cin.ignore();
			if (std::cin.peek() == '\n')
				std::cin.ignore();
			std::cout.flush();
		}
		else if (count_of_number < 1) {
			std::cout << "Число чисел должно быть больше 0!" << std::endl;
		}
		else {
			break;
		}
	}
	// ввели количество, вводим сами елементы!
	
	arr = new int32_t[count_of_number];
	for (int32_t i{ 0 }; i < count_of_number; i++) {
		while (true) {
			std::cout << "Введите " << i << " эл: ";
			std::cin >> buffer_int;
			if (std::cin.good() == false) {
				std::cout << "ошибка воода, попробуйте снова!\n";
				while (std::cin.peek() != '\n')
					std::cin.ignore();
				if (std::cin.peek() == '\n')
					std::cin.ignore();
				std::cout.flush();
			}
			else {
				arr[i] = buffer_int;
				break;
			}
		}
	}
	// ввели все!

	// запуск WSA
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &wsaData) == WSASYSNOTREADY) {
		CIN_set_color(RED_COLOR);
		std::cout << " Ошибка запуска WSA!";
		CIN_set_color(WHITE_COLOR);
		std::cout << std::endl;

		if (arr != nullptr)
			delete[] arr;

		return;	// выход
	}

	bool is_good{ false };
	int bad_count{ 0 };
	SOCKET main_sock;
	while (is_good == false) {
		if ((main_sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
			if (bad_count > 0 && bad_count < 10) {
				CIN_set_color(RED_COLOR);
				std::cout << " Не получилось открыть сокет!";
				CIN_set_color(WHITE_COLOR);
				std::cout << " попытка: " << bad_count << std::endl;
			}
			else if (bad_count >= 10) {
				CIN_set_color(RED_COLOR);
				std::cout << " Не получилось открыть сокет!";
				CIN_set_color(WHITE_COLOR);
				std::cout << " попытки закончились, завершаю работу!" << std::endl;

				if (arr != nullptr)
					delete[] arr;

				WSACleanup();
				return;	// выход
			}
			else {
				CIN_set_color(RED_COLOR);
				std::cout << " Не получилось открыть сокет!";
				CIN_set_color(WHITE_COLOR);
				std::cout << " Попробую снова..." << std::endl;
			}
			bad_count++;
		}
		else {	// все успешно создалось
			CIN_set_color(GREEN_COLOR);
			std::cout << " Сокет успешно создан!";
			CIN_set_color(WHITE_COLOR);
			std::cout << std::endl;
			is_good = true;
		}
	}

	// подготавливаем структуру для ()
	sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(60888); // задаем порт сервера
	local.sin_addr.s_addr = inet_addr("127.0.0.1"); // задает IP сервера

	if (connect(main_sock, (sockaddr*)&local, sizeof(local)) == SOCKET_ERROR) {
		CIN_set_color(RED_COLOR);
		std::cout << " Не получилось присоедииться!";
		CIN_set_color(WHITE_COLOR);
		std::cout << " закрыаю сокет!" << std::endl;

		if (arr != nullptr)
			delete[] arr;

		WSACleanup();
		return;
	}
	else { // все в порядке! (отправляем)
		int32_t* tmp_arr = new int32_t[count_of_number + 3];
		tmp_arr[0] = 0;
		tmp_arr[1] = count_of_number;
		for (int32_t i{ 2 }; i < count_of_number + 2; i++)
			tmp_arr[i] = arr[i - 2];	// перенесли знач!

		char* for_send = reinterpret_cast<char*> (tmp_arr); // перевели в char
		for_send[(count_of_number + 2) * sizeof(int32_t)] = '\n';

		int32_t total{ 0 };
		while (total < ((count_of_number + 2) * sizeof(int32_t)) + 1) {
			int32_t sent = send(main_sock, for_send, ((count_of_number + 2) * sizeof(int32_t)) + 1, 0);
			if (sent == SOCKET_ERROR) {
				CIN_set_color(RED_COLOR);
				std::cout << " Ошибка отправки!";
				CIN_set_color(WHITE_COLOR);
				std::cout << std::endl;
				break;
			}
			total += sent;
		}
		if (tmp_arr != nullptr)
			delete[] tmp_arr;
		if (total == ((count_of_number + 2) * sizeof(int32_t)) + 1) {
			CIN_set_color(GREEN_COLOR);
			std::cout << " Отправлен упешно!";
			CIN_set_color(WHITE_COLOR);
			std::cout << " ждем ответ" << std::endl;

			char for_recv[1024];
			total = 0;
			if ((total = recv(main_sock, for_recv, 1024, 0)) > 0) {
				if (for_recv[total - 1] == '\n') {
					int32_t* tmp_in = reinterpret_cast<int32_t*>(for_recv);
					std::cout << "Ответ: " << *tmp_in << '\n' << "Для выхода нажмите [ENTER]" << std::endl;

					EasyMenu tmp_menu(" ");
					while (true) {
						tmp_menu.advanced_tick();
						if (tmp_menu.advanced_is_pressed() == true) {
							break;
						}
					}
				}
				else {
					CIN_set_color(RED_COLOR);
					std::cout << " Ошибка получения!";
					CIN_set_color(WHITE_COLOR);
					std::cout << std::endl;
				}
			}

			closesocket(main_sock);

		}
	}
	if (arr != nullptr)
		delete[] arr;

	WSACleanup();
}

void Client_kill() {
	// запуск WSA
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &wsaData) == WSASYSNOTREADY) {
		CIN_set_color(RED_COLOR);
		std::cout << " Ошибка запуска WSA!";
		CIN_set_color(WHITE_COLOR);
		std::cout << std::endl;
		return;	// выход
	}

	bool is_good{ false };
	int bad_count{ 0 };
	SOCKET main_sock;
	while (is_good == false) {
		if ((main_sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
			if (bad_count > 0 && bad_count < 10) {
				CIN_set_color(RED_COLOR);
				std::cout << " Не получилось открыть сокет!";
				CIN_set_color(WHITE_COLOR);
				std::cout << " попытка: " << bad_count << std::endl;
			}
			else if (bad_count >= 10) {
				CIN_set_color(RED_COLOR);
				std::cout << " Не получилось открыть сокет!";
				CIN_set_color(WHITE_COLOR);
				std::cout << " попытки закончились, завершаю работу!" << std::endl;

				WSACleanup();
				return;	// выход
			}
			else {
				CIN_set_color(RED_COLOR);
				std::cout << " Не получилось открыть сокет!";
				CIN_set_color(WHITE_COLOR);
				std::cout << " Попробую снова..." << std::endl;
			}
			bad_count++;
		}
		else {	// все успешно создалось
			CIN_set_color(GREEN_COLOR);
			std::cout << " Сокет успешно создан!";
			CIN_set_color(WHITE_COLOR);
			std::cout << std::endl;
			is_good = true;
		}
	}

	// подготавливаем структуру для ()
	sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(60888); // задаем порт сервера
	local.sin_addr.s_addr = inet_addr("127.0.0.1"); // задает IP сервера

	if (connect(main_sock, (sockaddr*)&local, sizeof(local)) == SOCKET_ERROR) {
		CIN_set_color(RED_COLOR);
		std::cout << " Не получилось присоедииться!";
		CIN_set_color(WHITE_COLOR);
		std::cout << " закрываю сокет!" << std::endl;

		closesocket(main_sock);

		WSACleanup();
		return;
	}
	else {
		int32_t tmp[2];
		tmp[0] = -1;
		char* tmp_buf = reinterpret_cast<char*>(&tmp);
		tmp_buf[sizeof(int32_t)] = '\n';
		send(main_sock, tmp_buf, sizeof(int32_t) + 1, 0);
	}

	CIN_set_color(GREEN_COLOR);
	std::cout << "Сервер закрыт\n";
	CIN_set_color(WHITE_COLOR);
	std::cout << "До свидания!" << std::endl;

	closesocket(main_sock);

	WSACleanup();
}