int x;

int factorial(int i)
{
    if (i == 1) {
        return i;
    } else {
        return i * factorial(i - 1);
    }
}

void main(void)
{
    x = 5;
    x = factorial(x);
    output(x);
    return x;
}
