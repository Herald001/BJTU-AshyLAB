#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#define NUM_READERS 3   //读者进程数量
#define NUM_WRITERS 2   //写者进程数量

static int data = 0;   //初始化临界缓存区为空
static int readers_count = 0;  //记录读者进程数量
static HANDLE mutex;   //用于实现读者进程对于readers_count变量的互斥访问
static HANDLE rw_semaphore;    //用于实现读者和写者互斥访问临界缓冲区

static DWORD WINAPI Reader(LPVOID lpParam) {
    int reader_id = *(int*)lpParam;

    while (1) {
        WaitForSingleObject(mutex, INFINITE);
        readers_count++;
        if (readers_count == 1) {
            WaitForSingleObject(rw_semaphore, INFINITE);
        }
        ReleaseMutex(mutex);

        printf("【读者%d】正在阅读: %d\n", reader_id, data);
        Sleep(1000);  // 模拟读取数据的耗时

        WaitForSingleObject(mutex, INFINITE);
        readers_count--;
        if (readers_count == 0) {
            ReleaseSemaphore(rw_semaphore, 1, NULL);
        }
        ReleaseMutex(mutex);

        Sleep(2000);  // 读者休息一段时间后再次读取数据
    }

    return 0;
}

DWORD WINAPI Writer(LPVOID lpParam) {
    int writer_id = *(int*)lpParam;

    while (1) {
        WaitForSingleObject(rw_semaphore, INFINITE);

        data++;  // 写入数据
        printf("【写者%d】正在写入: %d\n", writer_id, data);
        Sleep(2000);  // 模拟写入数据的耗时

        ReleaseSemaphore(rw_semaphore, 1, NULL);

        Sleep(3000);  // 写者休息一段时间后再次写入数据
    }

    return 0;
}

int main() {
	printf("==================================================\n"); 
    printf("【姓 名】：Ashyfox \n");
    printf("【学 号】：22231044\n");
    printf("【OSLAB】：读者优先的读者-写者问题模拟处理编程设计\n");
    printf("==================================================\n"); 

    // 创建互斥锁和读写信号量
    mutex = CreateMutex(NULL, FALSE, NULL);
    rw_semaphore = CreateSemaphore(NULL, 1, 1, NULL);

    // 创建读者线程
    HANDLE reader_threads[NUM_READERS];
    int reader_ids[NUM_READERS];
    for (int i = 0; i < NUM_READERS; i++) {
        reader_ids[i] = i + 1;
        reader_threads[i] = CreateThread(NULL, 0, Reader, &reader_ids[i], 0, NULL);
    }

    // 创建写者线程
    HANDLE writer_threads[NUM_WRITERS];
    int writer_ids[NUM_WRITERS];
    for (int i = 0; i < NUM_WRITERS; i++) {
        writer_ids[i] = i + 1;
        writer_threads[i] = CreateThread(NULL, 0, Writer, &writer_ids[i], 0, NULL);
    }

    // 等待所有读者线程和写者线程结束
    WaitForMultipleObjects(NUM_READERS, reader_threads, TRUE, INFINITE);
    WaitForMultipleObjects(NUM_WRITERS, writer_threads, TRUE, INFINITE);

    // 关闭句柄
    CloseHandle(mutex);
    CloseHandle(rw_semaphore);

    return 0;
}


