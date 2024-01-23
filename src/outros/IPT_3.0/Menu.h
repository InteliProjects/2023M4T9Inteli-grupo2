#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#ifndef IPT_PROJECT_MENU
#define IPT_PROJECT_MENU

#include <Arduino.h>

void menu_1();
void menu_2();
void menu_3();

int menu_num = 1;
int sub_menu = 1;


void checkMenuNum() {
  switch (menu_num) {
    case 1: menu_1(); break;
    case 2: menu_2(); break;
    case 3: menu_3(); break;
  }
}

#endif