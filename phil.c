#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <time.h> 
#include <unistd.h>
#define BILLION 1E9
struct timespec start,stop,waitTime,now;
enum {THINKING,HUNGRY,EATING} state[27];
pthread_mutex_t mutex[27];
pthread_mutex_t mutexOut;
pthread_cond_t cond[27];
double waiting[27];
double think[27];
double eat[27];
int numPhil;
int ret;
int count;
int looper[27];

int monitor(int i)
{ 
	for(int a = 0; a < count; a++)
	{
		pickup(i);
		putdown(i);
	}
	return 0;
}

void pickup(int i)
{

	pthread_mutex_lock(&mutex[i]);
	state[i] = HUNGRY;
	printf("philosopher %d hungry\n", i);
	clock_gettime(CLOCK_REALTIME,&start);
	test(i);
	while(state[i] != EATING)
		pthread_cond_wait(&cond[i],&mutex[i]);
	pthread_mutex_unlock(&mutex[i]);
}

void putdown(int i)
{
	pthread_mutex_lock(&mutex[i]);
	state[i] = THINKING;
	test((i + numPhil - 1) % numPhil);
	test((i + 1) % numPhil);
	double time = think[i] / 1000;
	printf("philosopher %d thinking\n", i);
	sleep(time);
	pthread_mutex_unlock(&mutex[i]);

}

void test(int i)
{

	if((state[(i + numPhil - 1) % numPhil] != EATING) && (state[(i + 1) % numPhil] != EATING) && (state[i] == HUNGRY))
	{
		state[i] = EATING;
		clock_gettime(CLOCK_REALTIME,&stop);
		waiting[i] = waiting[i] + (((stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / (double)BILLION) * 1000);
		printf("philosopher %d eating\n", i);
		double time = eat[i] / 1000;
		sleep(time);
		pthread_cond_signal(&cond[i]);

	}

}

double std(double avg, int i)
{
	double var = 0;
	var = var + (waiting[i] - avg) * (waiting[i] - avg);
	var = var / count;
	double sd = sqrt(var);
	return sd;
}

int main(int argc, char* argv[])
{
	for(int i = 0 ; i < 27; i++)
		waiting[i] = 0;
	for(int i = 0; i < 27; i++)
		looper[i] = 0;
	if(argc < 7)
	{
		printf("No. of arguments is not enough\n");
		return 1;
	}
	numPhil = atoi(argv[1]);
	if(numPhil % 2 == 0)
	{
		printf("Enter an odd no. of philosophers\n");
		return 1;
	}
	if(numPhil > 27 || numPhil < 0)
	{
		printf("Enter an proper no. of philosophers\n");
		return 1;
	}
	int minThink = atoi(argv[2]);
	int maxThink = atoi(argv[3]);
	int minEat = atoi(argv[4]);
	int maxEat = atoi(argv[5]);
	char* dist = argv[6];
	count = atoi(argv[7]);
	char* dist1 = "uniform";
	char* dist2 = "exponential";
	int check = strcmp(dist,dist1);
	int check2 = strcmp(dist,dist2);
	if(check != 0 && check2 != 0)
	{
		printf("Wrong distribution entered\n");
		return 1;
	}
	if((minThink < 1 || minEat < 1) || maxEat > 60000 || maxThink > 60000)
	{
		printf("Values not in range\n");
		return 1;
	}
	for(int i = 0; i < numPhil; i++)
		pthread_mutex_init(&mutex[i],NULL);
	for(int i = 0; i < numPhil; i++)
	{
		 pthread_cond_init(&cond[i], NULL);
		 state[i] = THINKING;
	}
	if(check == 0)
	{
		//uniform
		for(int i = 0; i < numPhil; i++)
		{
			srand(time(NULL));
			think[i] = minThink + (rand() % (maxThink + 1 - minThink));
			eat[i] = minEat + (rand() % (maxEat + 1 - minEat));
		}
	}
	if(check2 == 0)
	{
		//expo
		srand(time(NULL));
		int thinkMean = (maxThink + minThink) / 2;
		int eatMean = (maxEat + minEat) / 2;
		double eatR,eatU,thinkR,thinkU;
		for(int i = 0; i < numPhil; i++)
		{
			do
			{
       		 	eatU = (double)rand()/(double)((unsigned)RAND_MAX+1);
        		eatR = log(1-eatU)*(-eatMean);	
    		}while(eatR <= minEat || eatR >= maxEat);
    		do
			{
       		 	thinkU = (double)rand()/(double)((unsigned)RAND_MAX+1);
        		thinkR = log(1-thinkU)*(-thinkMean);	
    		}while(thinkR <= minEat || thinkR >= maxEat);

    		think[i] = thinkR;
    		eat[i] = eatR;

		}
	}

	pthread_t threads[numPhil];
	for(int i = 0; i < numPhil; i++)
	{
	 	int check = pthread_create(&threads[i], NULL, monitor, i);
	 	if(check)
	 	{
	 		printf("Error creating thread\n");
	 		exit(EXIT_FAILURE);
	 	}
	}
	for(int i = 0; i < numPhil; i++)
		pthread_join(threads[i],NULL);
	for(int i = 0; i < numPhil; i++)
		printf("philosopher %d duartion of hungry state = %f\n",i, waiting[i]);
	for(int i = 0; i < numPhil;i++)
	{
		double avg = waiting[i] / count;
		double sdev = std(avg,i);
		printf("philosopher %d avg time of hungry state = %f\n",i, avg);
		printf("philosopher %d standard deviation of hungry duartion = %f\n",i, sdev);
	}
	return 0;
}