#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <errno.h>


pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;

//nr random la adaugare/vanzare produse
//random la ok
//numarare threaduri care cumpara, vand si verificare daca se mai pot vinde in continuare

int nrProducts = 50;
int nr_thrs_sell_done = 0;
int nr_thrs_add_done = 0;
int nr_thrs;
int nr_thrs_sell, nr_thrs_add;

time_t t;

void sellProduct(int products, int *tid)
{
    pthread_mutex_lock(&lock);

    if(nr_thrs_add == nr_thrs_add_done && nrProducts<products)          //nu ajunge aici cand s-au terminat threadurile
    {
        printf("Nu s-au putut vinde deoarece nu mai sunt produse in stoc si nici threaduri care adauga produse\n");
        pthread_mutex_unlock(&lock);
    }
    else
    {
        printf("Se vrea vanzarea a %d produse, mai sunt %d threaduri care adauga\n", products, nr_thrs_add-nr_thrs_add_done);
        while(nrProducts<products)
            pthread_cond_wait(&full,&lock);

        nrProducts -= products;
        printf("Threadul %d a vandut %d produse ------ mai sunt %d threaduri care adauga\n", *tid, products, nr_thrs_add-nr_thrs_add_done);
        printf("\tAu ramas %d produse\n", nrProducts);
        nr_thrs_sell_done++;

    }
    pthread_mutex_unlock(&lock);
}

void addProduct(int products, int *tid)
{
    pthread_mutex_lock(&lock);

    nrProducts += products;
    printf("Threadul %d a adaugat %d produse\n", *tid, products);
    printf("\tAvem %d produse\n", nrProducts);
    nr_thrs_add_done++;
    printf("\t\t %d threaduri care au adaugat pana acum\n", nr_thrs_add_done);

    pthread_cond_signal(&full);
    pthread_mutex_unlock(&lock);

}

void *functhr1(void *arg)
{
	int *tid = (int *)arg;
	//srand(time(NULL));
	//srand((unsigned) time(&t));
    int *random_number = (int*)malloc(1*sizeof(int));
    srand(random_number);
	*random_number = (rand()%5 + 1)*10;
	sellProduct(*random_number, tid);
	free(tid);
    return NULL;
}

void *functhr2(void *arg)
{
	int *tid = (int *)arg;
	int *random_number = (int*)malloc(1*sizeof(int));
	srand(random_number);
	*random_number = (rand()%5 + 1)*10;
	addProduct(*random_number, tid);
	free(tid);
    return NULL;
}


int main(int argc, char *argv[])
{

    srand(time(NULL));
   // srand((unsigned) time(&t));
  //  for(int i = 0; i < 10; i++)
  //      printf("un nr: %d\n", (rand()%5 + 1)*10);

    printf("Dati numarul de thread-uri \n");
	//scanf("%d", &nr_thrs);
    nr_thrs = 10;
    pthread_t th[100];
	int i;

	if(pthread_mutex_init(&lock, NULL))
    {
		perror(NULL);
		return errno;
	}

   // nr_thrs_sell = rand()%nr_thrs;
    nr_thrs_sell = 7;
    nr_thrs_add = nr_thrs - nr_thrs_sell;
    printf("Nr threaduri care vand = %d\n", nr_thrs_sell);
    printf("Nr threaduri care adauga = %d\n", nr_thrs_add);

	for(i = 0; i < nr_thrs_sell; i++)
    {
		int *k = (int *)malloc(1*sizeof(int));
		*k = i;
        if(pthread_create(&th[i], NULL, functhr1, k))
        {
            perror(NULL);
            return errno;
        }

	}

	for(i = nr_thrs_sell; i < nr_thrs; i++)
    {
		int *k = (int *)malloc(1*sizeof(int));
		*k = i;
		if(pthread_create(&th[i], NULL, functhr2, k))
        {
			perror(NULL);
			return errno;
		}
	}

    pthread_cond_signal(&full);
    for(i = 0; i < nr_thrs ; i++)
    {
        if(pthread_join(th[i],NULL))
        {
            perror(NULL);
            return errno;
        }
    }

	printf("\nAu terminat %d threaduri\n", nr_thrs_sell_done + nr_thrs_add_done);

    return 0;

}
