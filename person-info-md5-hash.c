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
#define YEARS 1
#define NR_THREADS 1
#define DAYS_IN_SEC (60 * 60 * 24)
#define TOTAL_DAYS (365 * YEARS)

static const char *hexstring = "9743152e72ad2e07f430778f9ef6fe22";

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

unsigned char *hexInt;

void *getHMAC(void *argp)
{
    pthread_setname_np("GET_PERSONNR_HASH");
    struct PersonNr *d = (struct PersonNr *)argp;
    unsigned char *digest = malloc(sizeof(unsigned char) * 16);
    char *dob = d->dob;

    for (int i = d->start; i <= d->stop; i++)
    {
        char str[13];
        sprintf(str, "%s%04d", dob, i);

        printf("---> %s\n", str);

        MD5_CTX ctx;
        MD5_Init(&ctx);

        MD5_Update(&ctx, &str, strlen(str));
        MD5_Final(digest, &ctx);
#if 1
        for (int z = 0; z < 16; z++)
        {
            printf("DIG BYTE #\t %d | %02x %02x | %u %u|\n", z, digest[z], hexInt[z], (unsigned int)digest[z], (unsigned int)hexInt[z]);
        }
#endif
        char mdString[33];
        for (int z = 0; z < 16; z++)
        {
            sprintf(&mdString[z * 2], "%02x", (unsigned int)digest[z]);
        }
        //printf(" [DONE]\t%s\t%s\t\t%s\n", dob, str, mdString);

        if (memcmp(hexInt, digest, 2) == 0)
        {
            printf(" [DONE] Found match");

            for (int z = 0; z < 16; z++)
            {
                printf("%u\n", (unsigned int)digest[z]);
            }

            char mdString[33];
            for (int z = 0; z < 16; z++)
            {
                sprintf(&mdString[z * 2], "%02x", (unsigned int)digest[z]);
            }
            printf(" [DONE]\t%s\t%s\t\t%s\n", dob, str, mdString);

            exit(0);
        }
        i = d->stop + 1;
    }
    return NULL;
}

void *threadWorker(void *vPointer)
{
    int idx = (int)vPointer;
    struct PersonNr firstPersonPointer; // = personDobArray[0];

    for (int i = idx; i < TOTAL_DAYS; i += NR_THREADS)
    {
        getHMAC(&personDobArray[i]);
        i = TOTAL_DAYS;
    }
    return NULL;
}

unsigned char *getIntFromHexStr()
{

    int i;
    unsigned char *bytearray = malloc(sizeof(char) * 16);
    uint8_t str_len = strlen(hexstring);

    for (i = 0; i < (str_len / 2); i++)
    {
        sscanf(hexstring + 2 * i, "%2s", &bytearray[i]);

        printf("--- BYTE #\t %d: %02x int %u\n", str_len, bytearray[i], (unsigned int)bytearray[i]);
    }
    return bytearray;
}

int main(void)
{
    time_t epoch = START_TS;

    hexInt = getIntFromHexStr();

    printf(" [INFO] %s", ctime(&epoch));

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
        //pthread_create(&threadId, NULL, threadWorker, (void *)(uintptr_t)idx[i]);
        threadWorker((void *)(uintptr_t)idx[i]);
    }

    //pthread_join(threadId, NULL);
    return 0;
}