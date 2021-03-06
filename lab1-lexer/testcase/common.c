// macros should not appear here
##
long v@riable;
long fib(int n) {
    assert(n >= 0);
    if (n == 0) return 0;
    if (n == 1) return 1;
    int a = 0, b = 1, f;
    for (int i = 2; i <= n; ++i) {
        f = a + b;
        a = b;
        b = f;
    }
    return f;
}
int main(int argc, char* argv[]) {
    int result = 0;
    for (int i = 2; i < 100; ++i) {
        int flag = 1;
        for (int j = 2; j * j <= i; ++j)
            if (i % j == 0) {
                flag = 0;
                break; /*
                a block comment should not apperas
                */
            }
            if (flag) result += fib(i);
    }
    result = ((result << 3) ^ 0x3f3f3f3f) % 998244353;
    double a = 289.129 * 97e-3 / 3.4e5.6;
    char *s = "this is a string with escape char like \x41, \102, \t";
    char s1[] = "";
    char temp = 1 ? 'abc' : ''; // invalid!
    return 1;
} /*
without terminating
this is a code with multiple errors