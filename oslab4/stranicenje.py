import math
import sys
import signal
import time
from random import randint as rndint

MIN_PROCESS_SIZE = 32
MAX_PROCESS_SIZE = 640
VEL_STR = 64


def sigint_handler(signum, frame):
    exit(0)


def find_lru(processes, frames_num):
    # try to find an empty frame
    frames = [i for i in range(frames_num)]
    print(f"-------entered find_lru, frames is {frames}")
    for process in processes:
        for page_num in range(len(process.tablica_stranicenja)):
            if process.tablica_stranicenja[page_num] & 0x20:
                print(format(process.tablica_stranicenja[page_num] & 0x20, '016b'))
                print(f"that is index {int(str(format(process.tablica_stranicenja[page_num] >> 12, '04b'))[::-1], 2)}")
                print(
                    f"removing {format(process.tablica_stranicenja[page_num], '016b')} from free frames, shifted right by 12: {format(process.tablica_stranicenja[page_num] >> 12, '04b')}")
                print(process.tablica_stranicenja[page_num] >> 12)
                print(frames)
                frames.remove(process.tablica_stranicenja[page_num] >> 12)  # TODO TEST

    if frames:
        return frames[0]  # returning fid which is equivallent to frame ordinal number

    # else: find least recently used frame
    lru_frame, min_time = 0, 32
    print("GOTHERE")
    for process in processes:
        process_w_min = process
        for pgid in range(len(process.tablica_stranicenja)):
            if not process.tablica_stranicenja[pgid] & 0x20: continue  # searching only pages present in frames
            print(f"process.tablica_stranicenja[fid]: {format(process.tablica_stranicenja[pgid], '016b')}")
            if int(format(process.tablica_stranicenja[pgid], '016b')[11:]) < min_time:
                min_time = int(str(bin(process.tablica_stranicenja[pgid]))[2:][::-1][0:5])
                lru_frame = int(str(int(format(process.tablica_stranicenja[pgid], '016b')) >> 6 )[::-1]) // 64
                process_w_min = process
                print("============================reassigned a frame")
    process_w_min.tablica_stranicenja[lru_frame] &= 0xDF  # removing bit 5
    # TODO should return FRAME_ID, and reset presence bit to 0

    return lru_frame


def main():
    signal.signal(signal.SIGINT, sigint_handler)

    if len(sys.argv) != 3:
        print("Two arguments required!")
        exit(0)
    broj_procesa, broj_okvira = map(int, sys.argv[1:])

    if broj_procesa < 1 or broj_okvira < 1:
        print("Invalid arguments!")
        exit(0)
    print(f"{broj_procesa} processes and {broj_okvira} frames in memory")
    arr_procesa = [Proces(i) for i in range(broj_procesa)]
    storage = [Frame(i) for i in range(broj_okvira)]

    # TODO useful constant, clear 5th bit: &= 0xDF (6th, whatever)
    t = 0
    while True:
        for proces in arr_procesa:
            print(f"proces: {proces}")
            rnd_addr = rndint(MIN_PROCESS_SIZE, proces.vel_procesa)  # adresa koju proces zatreba
            print(f"rnd_addr prije konverzije u binarni zapis s pomakom: {rnd_addr}")
            # print(f"Needs to be in frame number {rnd_addr // VEL_STR} which is in binary {format(rnd_addr // VEL_STR, '04b')} with offset {rnd_addr % VEL_STR} which is in binary {format(rnd_addr % VEL_STR, '06b')}")
            rnd_addr = int(format(rnd_addr // VEL_STR, '04b')[::-1] + format(rnd_addr % VEL_STR, '06b')[::-1], 2)
            print(f"rnd_addr: {format(rnd_addr, '016b')}")
            print(f"\ngenerated rnd_addr: {hex(rnd_addr)}")
            index = int(format(rnd_addr, '016b')[-7:-11:-1], 2)
            print(f"That should be index {index}")

            print(f"index: {index} <- is this correct? -> should be")

            if proces.tablica_stranicenja[index] & 0x20:
                print("hit")  # TODO set lru variable, among other things
            else:  # pronađi least recently used okvir - resetira se na nulu kad se koristi
                print("miss")
                # pid, fid = find_lru(arr_procesa, broj_okvira)
                # into_frame = arr_procesa[pid].tablica_stranicenja[fid] >> 12
                into_frame = find_lru(arr_procesa, broj_okvira)
                print("---------exited find_lru")
                print(f"Putting process {proces.pid} page {index} into frame {into_frame}")
                proces.tablica_stranicenja[index] = int(format(into_frame * 64, '010b')[::-1],
                                                        2) << 6 | 0x20 | t  # TODO ima li ovo smisla, tu treba bit 10 bitova koji odreduju adresu (read next line)
                # ima - loadas stranicu u okvir koji je //64, a sama adresa onoga sto trazis
                # na kraju bude preciznija, jos ima i smisla jer imas 010b pa ce bit 10 bitova + << 6
                print(
                    f"new state of proces.tablica_stranicenja[index]: {format(proces.tablica_stranicenja[index], '016b')}")

            t += 1
            t %= 32  # TODO needs an if statement, update a few more things THIS MIGHT NOT NEED TO BE HERE - def no
            # time.sleep(1)


class Proces:
    def __init__(self, pid):
        self.pid = pid
        self.vel_procesa = rndint(MIN_PROCESS_SIZE, MAX_PROCESS_SIZE)  # size in bytes
        self.tablica_stranicenja = [0x0 for _ in range(math.ceil(self.vel_procesa / VEL_STR))]

    def load(self, pid, pid_page_num):  # load pid process' page pid_page_num into this frame
        pass  # TODO should it be here?

    def __str__(self):
        a = f"Proces {self.pid} of size {self.vel_procesa}B with page table of size {len(self.tablica_stranicenja)} and table contents:"
        for i in self.tablica_stranicenja:
            a += f"\n{format(i, '016b')}"
        return a


class Frame:
    def __init__(self, fid):
        self.fid = fid
        self.pid = None  # pid of the process that is using this frame # TODO is this needed?
        self.pid_page_num = None  # page number of the process that is using this frame

    def __str__(self):
        return f"Frame {self.fid} is used by process {self.pid} at page {self.pid_page_num}"

    def __repr__(self):
        return self.__str__()

    def store(self, pid, pid_page_num):  # save pid process' page pid_page_num into this frame
        self.pid = pid
        self.pid_page_num = pid_page_num


if __name__ == "__main__":
    main()

    # KNOWN BUGS
    # 1. frames.remove(fid) throws ValueError: list.remove(x): x not in list

    # 2. find_lru(arr_procesa, broj_okvira) returns None if there are no frames free - implement fully
    # 3.     proces.tablica_stranicenja[index] = int(str(bin(into_frame))[2:] + str(bin(rnd_addr))[-5:]) << 6 | 0x20 | t vraća
    # ValueError: invalid literal for int() with base 10: '0b1111'
