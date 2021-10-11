unsigned myhash(char s[]) {
    int len = strlen(s);
    unsigned val = 0;
    for (int i = 0; i < len; ++i) {
        int temp = (int)s[i];
        val |= (temp << 7) ^ 0x3f3f3f3f;
        val &= temp >> 11 & 0x7fffffff;
    }
    return val;
}
int a = 123u;
int b = 0123;
int c = 08;
int d = 08a3f;
double e = 1e9 + 7;
double f = 23.4E-5;
double g = 123.;
double h = 123e;
double i = 55.5.5;
double j = 0e1;
double k = 0.125f;
double l = 0.99999;
char m = 'a';
char n = '\x41';