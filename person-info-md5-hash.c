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
#define NR_THREADS 8
#define DAYS_IN_SEC (60 * 60 * 24)
#define TOTAL_DAYS (365 * YEARS)

#define SEARCH_FOR_HASH_DOB "a6d063641f3ba94c30d2073ee46b7e14"

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

unsigned int *hexInt;

void *getHMAC(void *argp, MD5_CTX *ctx)
{
    pthread_setname_np("GET_PERSONNR_HASH");
    struct PersonNr *d = (struct PersonNr *)argp;
    unsigned char digest[16];
    char *dob = d->dob;

    //long hexInt = getIntFromHexStr();
    for (int i = d->start; i <= d->stop; i++)
    {
        char str[16];
        sprintf(str, "%s%04d", dob, i);

        MD5_Update(ctx, &str, strlen(str));

        if (memcmp(hexInt, digest, sizeof(unsigned int)) == 0)
        {

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

            //printf("%u\n%u\n", (unsigned int)digest, (unsigned int)SEARCH_FOR_HASH_DOB);
            //pthread_kill(threadId, 0);
            exit(0);
        }

            //printf("%x\n", digest);
#if 0
        if ((unsigned int)digest == (unsigned int)SEARCH_FOR_HASH_DOB)
        {
            char mdString[33];
            for (int z = 0; z < 16; z++)
            {
                sprintf(&mdString[z * 2], "%02x", (unsigned int)digest[z]);
            }

            if (strcmp(mdString, SEARCH_FOR_HASH_DOB) == 0)
            {
                printf(" [DONE]\t%s\t%s\t\t%s\n", dob, str, mdString);
                pthread_kill(threadId, 0);
            }

            printf("%u\n%u\n", (unsigned int)digest, (unsigned int)SEARCH_FOR_HASH_DOB);
        }
#endif
    }
    //free(d->dob);
    //free(d);
    return NULL;
}

void *threadWorker(void *vPointer)
{
    //struct PersonNr* firstPersonPointer = (struct PersonNr*)vPointer;
    int idx = (int)vPointer;
    struct PersonNr firstPersonPointer; // = personDobArray[0];

    //printf(" [INFO] First person pointer %s\n", firstPersonPointer.dob);
    MD5_CTX ctx;
    MD5_Init(&ctx);

    for (int i = idx; i < TOTAL_DAYS; i += NR_THREADS)
    {
        firstPersonPointer = personDobArray[i];
        //printf(" [INFO] Next person pointer %s (idx %d)\n", firstPersonPointer.dob, i);
        //exit(0);

        getHMAC(&firstPersonPointer, &ctx);
    }
    return NULL;
}

unsigned int *getIntFromHexStr()
{
    char *hexstring = SEARCH_FOR_HASH_DOB;
    int i;
    unsigned int *bytearray = malloc(sizeof(unsigned int) * 16);
    uint8_t str_len = strlen(hexstring);

    for (i = 0; i < (str_len / 2); i++)
    {
        sscanf(hexstring + 2 * i, "%02x", &bytearray[i]);
        printf("bytearray %d: %02x int %u\n", i, bytearray[i], (unsigned int)bytearray[i]);
    }
    return bytearray;
}

int main(void)
{
    sscanf(SEARCH_FOR_HASH_DOB, "%x", &hash);
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
        /*struct PersonNr personDob = personDobArray[i];
        void *p = &personDob;*/

        idx[i] = i;
        printf(" [INFO] Creating thread %d (%x)\n", idx[i], idx[i]);
        pthread_create(&threadId, NULL, threadWorker, (void *)(uintptr_t)idx[i]);
    }

    pthread_join(threadId, NULL);
    return 0;
}