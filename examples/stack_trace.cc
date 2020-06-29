void a() {
  int x = 1;
  x = 2;
  x = 3;
  x = 4;
  x = 5;
  x++;
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
