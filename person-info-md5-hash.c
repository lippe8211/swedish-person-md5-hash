#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#if defined(__APPLE__)
#define COMMON_DIGEST_FOR_OPENSSL
#include <CommonCrypto/CommonDigest.h>
#define SHA1 CC_SHA1
#else
#include <openssl/md5.h>
#endif

#define START_TS -473385600
#define MAX_BUFF 11
#define YEARS 40
#define NR_THREADS 8
#define DAYS_IN_SEC (60 * 60 * 24)
#define TOTAL_DAYS (365 * YEARS)

#define SEARCH_MD5 "62244006d8b953f146ae74d430b7dd7c"

unsigned int hash;

struct PersonNr
{
    int start;
    int stop;
    char *dob;
    struct PersonNr *previous;
};

static pthread_t threadId;
struct PersonNr *personDobArray;
static unsigned long countHashGenerated = 0;
char *hexByteArray = NULL;

void printDobInHex(char *dob, unsigned char *digest)
{
    char mdString[33];
    for (int z = 0; z < 16; z++)
    {
        sprintf(&mdString[z * 2], "%02x", (unsigned int)digest[z]);
    }
    printf(" [DONE]\t%s\t%s\n", dob, mdString);
}

void *generateHashForDob(void *argp)
{
    struct PersonNr *d = (struct PersonNr *)argp;
    unsigned char *digest = malloc(sizeof(char) * 16);
    char *dob = d->dob;

    for (int i = d->start; i <= d->stop; i++)
    {
        char str[13];
        str[12] = 0;
        sprintf(str, "%s%04d", dob, i);

        MD5_CTX ctx;
        MD5_Init(&ctx);

        MD5_Update(&ctx, &str, strlen(str));
        MD5_Final(digest, &ctx);
        countHashGenerated++;
        if (memcmp(hexByteArray, digest, 16) == 0)
        {
            printf(" [INFO] Found match (%lu hashes generated)\n", countHashGenerated);
            printDobInHex(str, digest);
            exit(0);
        }
    }
    return NULL;
}

void *threadWorker(void *vPointer)
{
    pthread_setname_np("GET_PERSONNR_HASH");
    for (int i = ((int)vPointer); i < TOTAL_DAYS; i += NR_THREADS)
    {
        generateHashForDob(&personDobArray[i]);
    }
    return NULL;
}

char *getByteArrayFromHex()
{
    char *bytearray = malloc(sizeof(char) * 17);
    bytearray[17] = 0;

    for (int i = 0; i < 16; i++)
    {
        char str[3];
        int idx = i * 2;
        str[0] = SEARCH_MD5[idx];
        str[1] = SEARCH_MD5[idx + 1];
        str[2] = 0;
        bytearray[i] = (int)strtol(str, NULL, 16);
    }

    return bytearray;
}

int main(void)
{
    time_t epoch = START_TS;

    hexByteArray = getByteArrayFromHex();

    printf(" [INFO] %s\n", ctime(&epoch));

    personDobArray = malloc(sizeof(struct PersonNr) * TOTAL_DAYS);

    for (int i = 0; i < TOTAL_DAYS; i++)
    {
        char year_day_month[MAX_BUFF];
        strftime(year_day_month, MAX_BUFF, "%Y%m%d", localtime(&epoch));

        personDobArray[i].dob = malloc(sizeof(MAX_BUFF));
        personDobArray[i].dob = strcpy(personDobArray[i].dob, year_day_month);
        personDobArray[i].start = 1;
        personDobArray[i].stop = 9999;

        epoch += DAYS_IN_SEC;
    }

    printf(" [INFO] Done generating data\n");

    int *idx = malloc(sizeof(int) * NR_THREADS);

    for (int i = 0; i < NR_THREADS; i++)
    {
        idx[i] = i;
        printf(" [INFO] Creating thread %d (%x)\n", idx[i], idx[i]);
        pthread_create(&threadId, NULL, threadWorker, (void *)(uintptr_t)idx[i]);
    }

    pthread_join(threadId, NULL);
    return 0;
}
