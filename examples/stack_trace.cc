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
}

void b() {
     a();
}

void c() {
     a();
}

int main() {
    b();
    c();
}
