import math
import sys
import signal
import time
from random import randint as rndint

PG_SIZE = 64
MIN_PROCESS_SIZE = 32
MAX_PROCESS_SIZE = 10 * PG_SIZE


def sigint_handler(signum, frame):
    exit(0)


def find_lru(processes, frames_num):
    frames = [i for i in range(frames_num)]
    for process in processes:
        for page_num in range(len(process.tablica_stranicenja)):
            if process.tablica_stranicenja[page_num] & 0x20:
                frames.remove(int(format(process.tablica_stranicenja[page_num] >> 6, '010b')[::-1], 2) // PG_SIZE)

    if frames:
        print(f"\t assigned frame number {frames[0]} at adress {hex(frames[0] * PG_SIZE)}")
        return frames[0]  # returning fid which is equivallent to frame ordinal number

    # else: find least recently used frame
    lru_frame, min_time = None, 32
    process_w_min, ret_pgid = None, None
    for process in processes:
        for pgid in range(len(process.tablica_stranicenja)):
            if not process.tablica_stranicenja[pgid] & 0x20: continue  # searching only pages present in frames
            if not int(format(process.tablica_stranicenja[pgid], '016b')[11:], 2) < min_time: continue
            min_time = int(format(process.tablica_stranicenja[pgid], '016b')[11:], 2)
            lru_frame = int(format(process.tablica_stranicenja[pgid] >> 6, '010b')[::-1], 2) // PG_SIZE
            process_w_min = process
            ret_pgid = pgid

    print(
        f"\t removing page {hex(process_w_min.tablica_stranicenja[ret_pgid])} (ord. no. {ret_pgid}) process {process_w_min.pid} from memory")
    print(f"\t lru of purged page: {min_time}")
    process_w_min.tablica_stranicenja[ret_pgid] &= 0xFFDF  # reset presence bit
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

    t = 0
    while True:
        for proces in arr_procesa:
            print("----------------------------------------------------")
            print(f"process: {proces.pid}")
            print(f"\t t: {t}")
            rnd_addr = rndint(MIN_PROCESS_SIZE, proces.vel_procesa)  # adresa koju proces zatreba
            rnd_addr = int(format(rnd_addr // PG_SIZE, '04b')[::-1] + format(rnd_addr % PG_SIZE, '06b')[::-1], 2)
            index = int(format(rnd_addr, '016b')[-7:-11:-1], 2)

            print(f"\t rnd addr:   {hex(rnd_addr)}")
            print(f"\t in  binary: {format(rnd_addr, '016b')}")
            offset = int(format(rnd_addr, '016b')[-6:], 2)
            if proces.tablica_stranicenja[index] & 0x20:
                print("\t hit!")
                frame_no = int(format(proces.tablica_stranicenja[index] >> 6, '010b')[::-1], 2) // PG_SIZE
                print(f"\t page contents: {hex(proces.tablica_stranicenja[index])}")
                print(f"\t at frame number {frame_no}")
                print(f"\t which is adress {hex(frame_no * PG_SIZE)}")
                print(f"\t exact byte adress: {frame_no * PG_SIZE << 6 | offset}")
                print(f"\t data at adress: {storage[frame_no].data[offset]}")

                storage[frame_no].increment(offset)
                proces.tablica_stranicenja[index] = proces.tablica_stranicenja[index] & 0xFFE0 | t  # set time to t
                storage[int(format(proces.tablica_stranicenja[index] >> 6, '010b')[::-1], 2) // PG_SIZE].store(
                    proces.pid,
                    index)
            else:
                print("\t miss!")
                into_frame = find_lru(arr_procesa, broj_okvira)
                proces.tablica_stranicenja[index] = int(format(into_frame * PG_SIZE, '010b')[::-1], 2) << 6 | 0x20 | t
                frame_no = int(format(proces.tablica_stranicenja[index] >> 6, '010b')[::-1], 2) // PG_SIZE
                storage[int(format(proces.tablica_stranicenja[index] >> 6, '010b')[::-1], 2) // PG_SIZE].store(
                    proces.pid,
                    index)
                print(f"\t assigned frame {frame_no} at adress {hex(frame_no * PG_SIZE)}")
                print(f"\t page contents: {hex(proces.tablica_stranicenja[index])}")
                print(f"\t exact byte adress {hex(frame_no * PG_SIZE << 6 | offset)}")
                print(f"\t in binary {format(frame_no * PG_SIZE << 6 | offset, '016b')}")
                print(f"\t data at adress: {storage[frame_no].data[offset]}")
                storage[frame_no].increment(offset)

            t += 1
            if t == 32:  # trenutnoj stranici t postavi na 1, svim ostalim na 0
                t = 0
                for process_mod_t in arr_procesa:
                    for pgid in range(len(process_mod_t.tablica_stranicenja)):
                        process_mod_t.tablica_stranicenja[pgid] &= 0xFFE0
                proces.tablica_stranicenja[index] &= 0xFFE1

            time.sleep(1)


class Proces:
    def __init__(self, pid):
        self.pid = pid
        self.vel_procesa = rndint(MIN_PROCESS_SIZE, MAX_PROCESS_SIZE)  # size in bytes
        self.tablica_stranicenja = [0x0 for _ in range(math.ceil(self.vel_procesa / PG_SIZE))]

    def __str__(self):
        a = f"pid: {self.pid}, size: {self.vel_procesa}, paging table:"
        for i in self.tablica_stranicenja:
            a += f"\n{format(i, '016b')}"
        return a

    def __repr__(self):
        return self.__str__()


class Frame:
    def __init__(self, fid):
        self.fid = fid
        self.pid = None  # pid of the process that is using this frame
        self.pid_page = None  # page number of the process that is using this frame
        self.data = [0x0 for _ in range(PG_SIZE)]

    def __str__(self):
        return f"Frame {self.fid} is used by process {self.pid} at page {self.pid_page}"

    def __repr__(self):
        return self.__str__()

    def store(self, pid, pid_page):  # save pid process' page pid_page_num into this frame
        self.pid = pid
        self.pid_page = pid_page

    def increment(self, offset):
        self.data[offset] += 1 if self.data[offset] < 254 else 0


if __name__ == "__main__":
    main()
    # TODO Prilikom zamjene odnosno izbacivanja stranica pretpostavite da je njihov sadržaj uvijek mijenjan te ga pohranite na disk - sredi to s diskom
