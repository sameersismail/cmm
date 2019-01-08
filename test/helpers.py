import subprocess

CMM_PATH = "./src/cmm"
FILE_PREFIX = "./test/data/"
FILE_SUFFIX = ".out"


def process_stdout(output: bytes) -> bytes:
    """Remove the first line of spim output.
    (An OS- and system-specific exception loading message."""
    lines = output.splitlines(keepends=True)
    return b"".join(lines[1:])


def cmm(filename: str):
    subprocess.run([CMM_PATH, 
                    FILE_PREFIX + filename, 
                    "-o", FILE_PREFIX + filename + FILE_SUFFIX])


def spim(filename: str) -> bytes:
    out = subprocess.run(["spim", "-file", 
                          FILE_PREFIX + filename + FILE_SUFFIX],
                          stdout=subprocess.PIPE)
    return out.stdout
