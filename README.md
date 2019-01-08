# `cmm`: C Minus Minus

An ad hoc, informally-specified, bug-ridden, slow implementation of half a compiler for a subset of C.

# Example Usage

```sh
$ ls
docs/ src/ test/ Makefile README.md
$ make
[...]
$ cat test/data/factorial.c
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
$ ./src/cmm test/data/factorial.c -o factorial.s
$ spim -file factorial.s
Loaded: /usr/local/spim/9.1.19/share/exceptions.s
120
```

