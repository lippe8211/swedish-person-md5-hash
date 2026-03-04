#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(__APPLE__)
#define COMMON_DIGEST_FOR_OPENSSL
#include <CommonCrypto/CommonDigest.h>
#define MD5_CTX CC_MD5_CTX
#define MD5_Init CC_MD5_Init
#define MD5_Update CC_MD5_Update
#define MD5_Final CC_MD5_Final
#else
#include <openssl/md5.h>
#endif

#define START_TS -473385600
#define YEARS 40
#define NR_THREADS 8
#define DAYS_IN_SEC (60 * 60 * 24)
#define TOTAL_DAYS (365 * YEARS)
#define START_NR 1
#define END_NR 9999

#define SEARCH_MD5 "62244006d8b953f146ae74d430b7dd7c"

typedef struct PersonNr {
    int start;
    int stop;
    char dob[9];
} PersonNr;

static pthread_t thread_ids[NR_THREADS];
static PersonNr *personDobArray = NULL;
static atomic_ulong countHashGenerated = 0;
static atomic_bool found_match = false;
static unsigned char hexByteArray[16];

static void printDobInHex(const char *dob, const unsigned char *digest)
{
    char mdString[33];
    for (int i = 0; i < 16; i++) {
        sprintf(&mdString[i * 2], "%02x", (unsigned int)digest[i]);
    }
    printf(" [DONE]\t%s\t%s\n", dob, mdString);
}

static inline void write_four_digits(char *dst, int number)
{
    dst[0] = (char)('0' + (number / 1000) % 10);
    dst[1] = (char)('0' + (number / 100) % 10);
    dst[2] = (char)('0' + (number / 10) % 10);
    dst[3] = (char)('0' + number % 10);
}

static void generateHashForDob(const PersonNr *d)
{
    unsigned char digest[16];
    char str[13];
    str[12] = '\0';

    memcpy(str, d->dob, 8);

    for (int i = d->start; i <= d->stop; i++) {
        if (atomic_load_explicit(&found_match, memory_order_relaxed)) {
            return;
        }

        write_four_digits(&str[8], i);

        MD5_CTX ctx;
        MD5_Init(&ctx);
        MD5_Update(&ctx, str, 12);
        MD5_Final(digest, &ctx);

        unsigned long current = atomic_fetch_add_explicit(&countHashGenerated, 1, memory_order_relaxed) + 1;

        if (memcmp(hexByteArray, digest, 16) == 0) {
            bool was_found = atomic_exchange_explicit(&found_match, true, memory_order_relaxed);
            if (!was_found) {
                printf(" [INFO] Found match (%lu hashes generated)\n", current);
                printDobInHex(str, digest);
            }
            return;
        }
    }
}

static void *threadWorker(void *argp)
{
    int start_index = *(int *)argp;

    for (int i = start_index; i < TOTAL_DAYS; i += NR_THREADS) {
        if (atomic_load_explicit(&found_match, memory_order_relaxed)) {
            break;
        }
        generateHashForDob(&personDobArray[i]);
    }

    return NULL;
}

static void getByteArrayFromHex(void)
{
    for (int i = 0; i < 16; i++) {
        char str[3];
        int idx = i * 2;
        str[0] = SEARCH_MD5[idx];
        str[1] = SEARCH_MD5[idx + 1];
        str[2] = '\0';
        hexByteArray[i] = (unsigned char)strtol(str, NULL, 16);
    }
}

int main(void)
{
    time_t epoch = START_TS;

    getByteArrayFromHex();

    printf(" [INFO] %s\n", ctime(&epoch));

    personDobArray = malloc(sizeof(PersonNr) * TOTAL_DAYS);
    if (personDobArray == NULL) {
        fprintf(stderr, " [ERROR] failed to allocate personDobArray\n");
        return 1;
    }

    for (int i = 0; i < TOTAL_DAYS; i++) {
        struct tm tm_epoch;
        localtime_r(&epoch, &tm_epoch);
        strftime(personDobArray[i].dob, sizeof(personDobArray[i].dob), "%Y%m%d", &tm_epoch);

        personDobArray[i].start = START_NR;
        personDobArray[i].stop = END_NR;

        epoch += DAYS_IN_SEC;
    }

    printf(" [INFO] Done generating data\n");

    int idx[NR_THREADS];

    for (int i = 0; i < NR_THREADS; i++) {
        idx[i] = i;
        printf(" [INFO] Creating thread %d (%x)\n", idx[i], idx[i]);
        pthread_create(&thread_ids[i], NULL, threadWorker, &idx[i]);
    }

    for (int i = 0; i < NR_THREADS; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    free(personDobArray);
    return 0;
}
