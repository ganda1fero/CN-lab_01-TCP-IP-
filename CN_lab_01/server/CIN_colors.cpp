#include "CIN_colors.h"

void CIN_colors_Setup() {
	std::setlocale(LC_ALL, "");
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
}

void CIN_set_color(int32_t color_id) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color_id);
}