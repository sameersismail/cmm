int i;
int interm;
int res;

int gcd(int u, int v)
{
    if (v == 0) {
        return u;
    } else {
        i = u / v;
        interm = i * v;
        res = u - interm;
        return gcd(v, res);
    }
}

void main(void)
{
    int x;
    int y;
    x = 128;
    y = 16;
    output(gcd(x, y));
}
