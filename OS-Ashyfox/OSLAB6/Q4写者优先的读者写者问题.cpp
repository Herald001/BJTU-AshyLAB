/*
* 	写者优先
*/

# include <stdio.h>
# include <stdlib.h>
# include <time.h>
# include <sys/types.h>
# include <pthread.h>
# include <semaphore.h>
# include <string.h>
# include <unistd.h>

//semaphores
sem_t RWMutex, mutex1, mutex2, mutex3, wrt;
int writeCount, readCount;


struct data {
	int id;
	int opTime;
	int lastTime;
};

//读者
void* Reader(void* param) {
	int id = ((struct data*)param)->id;
	int lastTime = ((struct data*)param)->lastTime;
	int opTime = ((struct data*)param)->opTime;

	sleep(opTime);
	printf("【读者】线程%d等待【阅读】\n", id);	

	sem_wait(&mutex3);
	sem_wait(&RWMutex);
	sem_wait(&mutex2);
	readCount++;
	if(readCount == 1)
		sem_wait(&wrt);
	sem_post(&mutex2);
	sem_post(&RWMutex);
	sem_post(&mutex3);

	printf("【读者】线程%d开始【阅读】\n", id);
	/* reading is performed */
	sleep(lastTime);
	printf("【读者】线程%d结束【阅读】\n", id);

	sem_wait(&mutex2);
	readCount--;
	if(readCount == 0)
		sem_post(&wrt);
	sem_post(&mutex2);

	pthread_exit(0);
}

//写者
void* Writer(void* param) {
	int id = ((struct data*)param)->id;
	int lastTime = ((struct data*)param)->lastTime;
	int opTime = ((struct data*)param)->opTime;

	sleep(opTime);
	printf("【写者】线程%d等待【写入】\n", id);
	
	sem_wait(&mutex1);
	writeCount++;
	if(writeCount == 1){
		sem_wait(&RWMutex);
	}
	sem_post(&mutex1);
	
	sem_wait(&wrt);
	printf("【写者】线程%d正在【写入】\n", id);
	/* writing is performed */
	sleep(lastTime);
	printf("【写者】线程%d结束【写入】\n", id);
	sem_post(&wrt);

	sem_wait(&mutex1);
	writeCount--;
	if(writeCount == 0) {
		sem_post(&RWMutex);
	}
	sem_post(&mutex1);
	
	pthread_exit(0);
}

int main() {
	printf("==================================================\n"); 
    printf("【姓 名】：Ashyfox \n");
    printf("【学 号】：22231044\n");
    printf("【OSLAB】：写者优先的读者-写者问题模拟处理编程设计\n");
    printf("==================================================\n"); 
    printf("**输入数据（示例线程）：\n"); 
	printf("1 R 3 5\n" );
	printf("2 W 4 5\n" );
	printf("3 R 5 2\n" );
	printf("4 R 6 5\n" );
	printf("5 W 7 3\n" );

	pthread_t tid; 
	pthread_attr_t attr; //set of thread attributes
	pthread_attr_init(&attr);

	sem_init(&mutex1, 0, 1);
    sem_init(&mutex2, 0, 1);
    sem_init(&mutex3, 0, 1);
    sem_init(&wrt, 0, 1);
    sem_init(&RWMutex, 0, 1);

    readCount = writeCount = 0;

	int id = 0;
	printf("**请创建线程：\n"); 
	while(scanf("%d", &id) != EOF) {
		
		char role;		
		int opTime;		
		int lastTime;	
		
		scanf("%c%d%d", &role, &opTime, &lastTime);
		struct data* d = (struct data*)malloc(sizeof(struct data));

		d->id = id;
		d->opTime = opTime;
		d->lastTime = lastTime;

		if(role == 'R') {
			printf("【读者】线程%d【创建】\n", id);
			pthread_create(&tid, &attr, Reader, d);

		}
		else if(role == 'W') {
			printf("【写者】进程%d【创建】\n", id);
			pthread_create(&tid, &attr, Writer, d);
		}
	}

	sem_destroy(&mutex1);
	sem_destroy(&mutex2);
	sem_destroy(&mutex3);
	sem_destroy(&RWMutex);
	sem_destroy(&wrt);

	return 0;
}

