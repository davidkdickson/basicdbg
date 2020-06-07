void a() {
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
