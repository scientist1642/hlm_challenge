// Final mmap
#include <cstdio>
#include <iostream>
#include <time.h>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <time.h>
#include <sys/time.h>
#include <array>
#include <fstream>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <vector>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

//using namespace std;

const int MAXN_THREAD_N = 40;

int counter;
const int MAX_WORD_LENGTH = 1000;

int needle_charcount[260];
char cur_word[200];

char glinput[2246706];

char sorted_needle[MAX_WORD_LENGTH];

int cur_index = 0;
int needle_length;
char *needle, *fname;
int mn = 0;
int line_num = 0;
bool is_anagram = true;
int word_hash = 1, needle_hash = 1;
timespec time_begin, time_end;

char ans[300000];
int ans_count = 0;
pthread_t tid[100];
int ans_arr_length = 0;

pthread_mutex_t ans_lock;
std::vector<std::pair<int, int> > ansv[MAXN_THREAD_N];

int primes[] = {3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069, 1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163, 1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223, 1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289, 1291, 1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373, 1381, 1399, 1409, 1423, 1427, 1429, 1433, 1439, 1447, 1451, 1453, 1459, 1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511, 1523, 1531, 1543, 1549, 1553, 1559, 1567, 1571, 1579, 1583, 1597, 1601, 1607, 1609, 1613, 1619, 1621, 1627, 1637, 1657, 1663, 1667, 1669, 1693, 1697, 1699, 1709, 1721, 1723, 1733, 1741, 1747, 1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811, 1823, 1831, 1847, 1861, 1867, 1871, 1873, 1877, 1879, 1889, 1901, 1907, 1913, 1931, 1933, 1949, 1951, 1973, 1979, 1987, 1993, 1997, 1999, 2003, 2011, 2017, 2027, 2029, 2039, 2053, 2063, 2069};

struct Chunk
{
    int begin;
    int end;
    char *arr;
    int id;
};

timespec diff(struct timespec start, struct timespec end)
{
    struct timespec temp;
    if ((end.tv_nsec - start.tv_nsec) < 0)
    {
        temp.tv_sec = end.tv_sec - start.tv_sec - 1;
        temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    }
    else
    {
        temp.tv_sec = end.tv_sec - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return temp;
}

inline bool check_anagram_full(const char *input, int &begin, int &end)
{
    int sz = end - begin + 1;

    if (sz != needle_length)
        return false;
    char *buf = (char *)malloc(sz);

    memcpy(buf, input + begin, sz);
    std::sort(buf, buf + sz);

    return (strncmp(sorted_needle, buf, sz) == 0);
}

inline int calc_hash(char *input, int word_start, int word_end)
{
    int hash = 1;
    unsigned char uc;
    for (int i = word_start; i <= word_end; i++)
    {
        //uc = input[i];
        hash *= primes[(unsigned char)input[i]];
    }
    return hash;
}

void *find_anagrams(void *arg)
{
    struct Chunk *chunk = (struct Chunk *)arg;

    int begin = chunk->begin;
    int end = chunk->end;
    char *input = chunk->arr;
    //printf("%d %d\n ", chunk->begin, chunk->end);
    //printf("%d %d", begin, end);

    bool possibly_anagram = true;
    int line_num = 0, word_hash = 1, word_start = begin, word_end, ind;
    unsigned char uc;
    char c;

    for (int i = begin; i <= end; i += 4)
    {
        // Pattern match newline
        uint32_t data(*((uint32_t *)(input + i)));
        data = (data ^ 0x0a0a0a0aU);
        data = (data - 0x01010101U) & ~data & 0x80808080U;

        if (data)
        { // One of these 4 bytes is a newline, specifically non-zero
            for (ind = 0; data >>= 8; ind++)
            {
            } // get index of non zero byte

            word_end = ind + i - 1;
            if (input[word_end] == '\r') // On windows machine
                word_end--;

            if (word_end - word_start + 1 == needle_length &&
                calc_hash(input, word_start, word_end) == needle_hash &&
                check_anagram_full(input, word_start, word_end))
            {
                ansv[chunk->id].push_back(std::make_pair(word_start, word_end));
            }

            word_start = ind + i + 1;
        }
    }
    return NULL;
}

inline void print_ans(char *input, int chunk_num)
{
    int i, j, k;
    for (i = 0; i < chunk_num; i++)
    {
        for (j = 0; j < ansv[i].size(); j++)
        {
            printf(",");
            for (k = ansv[i][j].first; k <= ansv[i][j].second; k++)
                printf("%c", input[k]);
        }
    }
}

size_t getFilesize(const char *filename)
{
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

int main(int argc, char **argv)
{
    clock_gettime(CLOCK_REALTIME, &time_begin);
    //std::ios::sync_with_stdio(false);
    if (argc < 3)
    {
        printf("Not enough argument %d\n", argc);
        return 0;
    }
    fname = argv[1];
    needle = argv[2];
    needle_length = strlen(needle);

    memcpy(sorted_needle, needle, needle_length);
    std::sort(sorted_needle, sorted_needle + needle_length);
    needle_hash = calc_hash(needle, 0, needle_length - 1);

    //posix_fadvise(fd, 0, 0, 1);
    size_t filesize = getFilesize(fname);
    
    int fd = open(fname, O_RDONLY);
    size_t pagesize = getpagesize();

    struct Chunk *chunk;
    char *input;
    input = (char *)mmap(
        (void *)(pagesize * (1 << 20)), filesize,
        PROT_READ, MAP_FILE | MAP_PRIVATE,
        fd, 0);
    //char *input = (char*) malloc(filesize + 1); */
    
    //input = glinput;
    

    int num_chunks = std::min(((int)sysconf(_SC_NPROCESSORS_ONLN)), 8);
    //num_chunks = 4;
    //num_chunks = 1;


    int chunk_size = int(filesize + num_chunks - 1) / num_chunks;
    
    int run_num = 0;
    int processed = 0;
    int error;
    int bytes_read = 0;
    for (int i = 0; i < num_chunks; i++)
    {
        //bytes_read += read(fd, input + bytes_read - 1, chunk_size);
        chunk = (struct Chunk *)malloc(sizeof(struct Chunk));
        chunk->begin = processed;
        chunk->end = (i + 1) * chunk_size - 1;
        chunk->id = i;
        if (chunk->end > filesize - 1)
            chunk->end = filesize - 1;
        while (input[chunk->end] != '\n' && chunk->end > processed)
            chunk->end--;
        if (chunk->end > processed)
        {
            chunk->arr = input;
            if (num_chunks == 1) //no need to create a thread
                find_anagrams((void *)chunk);
            else {
                error = pthread_create(&(tid[run_num++]), NULL, &find_anagrams, (void *)chunk);
                if (error != 0)
                    printf("\n Thread not created");
            }
            processed = chunk->end + 1;
        }
    }

    for (int i = 0; i < run_num; i++)
        pthread_join(tid[i], NULL);
    clock_gettime(CLOCK_REALTIME, &time_end);

    printf("%d", (int)(diff(time_begin, time_end).tv_nsec / 1000.));
    print_ans(input, num_chunks);
    printf("\n");

    return 0;
}
