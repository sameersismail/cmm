import pytest
import subprocess

from helpers import process_stdout, cmm, spim


def test_check_spim():
    """Ensure that `spim` is installed."""
    subprocess.run(["spim", "-file", "./test/data/hello.asm"])


def test_basic():
    cmm("factorial.c")
    stdout = spim("factorial.c")
    assert process_stdout(stdout) == b"120"
    
    cmm("recur.c")
    stdout = spim("recur.c")
    assert process_stdout(stdout) == b"5"

    cmm("gcd.c")
    stdout = spim("gcd.c")
    assert process_stdout(stdout) == b"16"


def test_io():
    cmm("io.c")
    with open("./test/data/io.c.in") as stdin:
        out = subprocess.run(["spim", "-file", "./test/data/io.c.out"],
                stdin=stdin,
                stdout=subprocess.PIPE)

    assert process_stdout(out.stdout) == b"5"


    cmm("double-input.c")
    with open("./test/data/double-input.c.in") as stdin:
        out = subprocess.run(["spim", "-file", "./test/data/double-input.c.out"],
                stdin=stdin,
                stdout=subprocess.PIPE)

    assert process_stdout(out.stdout) == b"50"
