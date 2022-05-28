import math
import sys
import signal
import time
from random import randint as rndint

MIN_PROCESS_SIZE = 32
MAX_PROCESS_SIZE = 640
VEL_STR = 64


def sigint_handler():
    exit(0)


def main():
    signal.signal(signal.SIGINT, sigint_handler)

    if len(sys.argv) != 3:
        print("Two arguments required!")
        exit(0)
    broj_procesa, broj_okvira = map(int, sys.argv[1:])

    if broj_procesa < 1 or broj_okvira < 1:
        print("Invalid arguments!")
        exit(0)
    arr_procesa = [Proces(i) for i in range(broj_procesa)]
    storage = [Frame(i) for i in range(broj_okvira)]

    # TODO useful constant, clear 5th bit: &= 0xDF
    t = 0
    while True:
        for proces in arr_procesa:
            rnd_addr = rndint(MIN_PROCESS_SIZE, proces.vel_procesa)  # adresa koju proces zatreba
            rnd_addr = int(str(rnd_addr // VEL_STR) + str(rnd_addr % VEL_STR))
            print(f"generated rnd_addr: {hex(rnd_addr)}")
            print(f"That should be index {int(format(rnd_addr, '016b')[9:6:-1], 2)}")  # = ::-1 + 6:9
            index = int(format(rnd_addr, '016b')[9:6:-1], 2)
            if proces.tablica_stranicenja[index] & 0x20:
                print("hit")
            else:
                print("miss")
                # pronađi least recently used okvir - resetira se na nulu kad se koristi

            t += 1
            t %= 32  # TODO needs an if statement, update a few more things

        time.sleep(1)


class Proces:
    def __init__(self, pid):
        self.id = pid
        self.vel_procesa = rndint(MIN_PROCESS_SIZE, MAX_PROCESS_SIZE)
        self.tablica_stranicenja = [0x0 for _ in range(math.ceil(self.vel_procesa / VEL_STR))]


class Frame:
    def __init__(self, oid):
        self.oid = oid
        self.pid = None  # pid of the process that is using this frame
        self.


if __name__ == "__main__":
    main()
