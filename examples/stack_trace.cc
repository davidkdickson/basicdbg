#include <iostream>

void a() {
  int x = 1;
  x = 2;
  x = 3;
  x = 4;
  x = 5;
  x = 6;
  x = 7;
  x = 8;
  x = 9;
  x = 10;
  std::cout << "end of a()" << std::endl;
}

void b() {
  a();
  std::cout << "end of b()" << std::endl;
}

void c() {
  a();
  std::cout << "end of c()" << std::endl;
}

int main() {
  b();
  c();
  std::cout << "end of main()" << std::endl;
}
