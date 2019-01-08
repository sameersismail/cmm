int x;
int y;

int recur(void)
{
    if (x == 5) {
        y = y + 1;
        return 5;
    } else {
        x = x + 1;
        y = y + 1;
        recur();
    }
}

void main(void)
{
    recur();
    output(y);
    return;
}
