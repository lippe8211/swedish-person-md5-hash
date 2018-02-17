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

#define SEARCH_FOR_HASH_DOB "4b811a6d870c7b3d5a751cead85a5a73"

struct PersonNr
{
    int start;
    int stop;
    char *dob;
};

static pthread_t threadId;

void *getHMAC(void *argp)
{
    pthread_setname_np("GET_PERSONNR_HASH");
    struct PersonNr *d = (struct PersonNr *)argp;
    unsigned char digest[16];
    char *dob = d->dob;

    MD5_CTX ctx;
    MD5_Init(&ctx);

    for (int i = d->start; i <= d->stop; i++)
    {
        char str[16];
        sprintf(str, "%s%04d", dob, i);

        MD5_Update(&ctx, &str, strlen(str));
        
        char mdString[33];
        for (int z = 0; z < 16; z++)
        {
            sprintf(&mdString[z * 2], "%02x", (unsigned int)digest[z]);
        }

        if ( strcmp(mdString, SEARCH_FOR_HASH_DOB) == 0 )
        {
            printf(" [DONE]\t%s\t\t%s\n", str, mdString);
            pthread_kill(threadId, 0);
        }
    }
    MD5_Final(digest, &ctx);
    free(d->dob);
    free(d);
    return NULL;
}
#if 0
void input(void *argp) 
{
    struct PersonNr* persons[] = (struct PersonNr*)argp;
    for (int i = 0; i < sizeof(&persons); i++)
    {
        struct PersonNr *p_struct = persons[i];

        getHMAC(p_struct);
    }
}
#endif

void *threadWorker( void * vPointer) 
{
    struct PersonNr* firstPersonPointer = (struct PersonNr*)vPointer;

    printf(" [INFO] First person pointer %s\n", firstPersonPointer->dob);
    
    for (; firstPersonPointer != 0; firstPersonPointer += NR_THREADS)
    {
        printf(" [INFO] Next person pointer %s\n", firstPersonPointer->dob);
    }
    return NULL;
}

int main(void)
{
    time_t epoch = START_TS;
    
    printf(" [INFO] %s", ctime(&epoch));

    struct PersonNr *personDobArray = malloc(sizeof(struct PersonNr) * TOTAL_DAYS);

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

    for(int i = 0; i < NR_THREADS; i++)
    {
        struct PersonNr personDob = personDobArray[i];

        void *p = &personDob;
        printf(" [INFO] Creating thread %d\n", i);
        pthread_create(&threadId, NULL, threadWorker, p);
    }

    pthread_join(threadId, NULL);
    return 0;
}