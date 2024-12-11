#include<iostream>
#include<mutex>
#include<chrono>
#include<thread>
using namespace std;

int n = 10;	// 单个缓存区大小
int produce[4] = { 0,0,0,0 },consume[4] = { 0,0,0,0 }; // 生产指针,消费指针
int count[3] = { 0,0,0 }; // 分别对应三个缓冲区所存放的数据个数，初始时都为0
int buffer[3][10]; // 3个存放10个数据的缓存区
mutex mtx[3]; // 缓冲区互斥量
mutex display_mtx;	// 输出数据锁

//   生产者函数 
void producer(int id)
{
    while(1){
        for (int i = 0; i < 3; i++) // 依次遍历三个缓冲区
        {
        if (mtx[i].try_lock()) 
            {
            if (count[i]!=n) 
                {
                buffer[i][produce[i]] = 1;
                display_mtx.lock(); 
                // 生产者正在进行生产行为 
                cout << "【Producer" << id << "】:produce {product" ;
                cout << produce[i] <<"}  at  [Buffer"<<i+1 <<"]:";
                // 打印缓冲区状况 
                for (int j = 0; j < n; j++){ cout << buffer[i][j] << " "; }
                cout << endl;
                display_mtx.unlock();
                produce[i] = (produce[i] + 1) % n; 
                count[i]++; 
                mtx[i].unlock(); 
                this_thread::sleep_for(chrono::seconds(1)); 
                break;
                }
            else {mtx[i].unlock(); }
            }
        }
    }
}
// 消费者函数 
void consumer(int id)
{
    while(1){
        for (int i = 0; i < 3; i++) 
        {
        if (mtx[i].try_lock()) 
            {
            if (count[i] != 0) 
                {// 消费者正在进行消费行为
                buffer[i][consume[i]] = 0; 
                display_mtx.lock();
                cout << "【Consumer" << id << "】:consume {product" ;
			cout << consume[i] <<"} from [Buffer"<<i+1 <<"]:";
			// 打印缓冲区状况 
                for (int j = 0; j < n; j++){cout << buffer[i][j] << " "; }
                cout << endl;
                display_mtx.unlock(); 
                consume[i] = (consume[i] + 1) % n;
                count[i]--;
                mtx[i].unlock(); 

                this_thread::sleep_for(chrono::seconds(5));
                break;
                }
            else {mtx[i].unlock();}}
        }
    };
}

int main() {
	cout << "============================================"<<endl; 
	cout << "【姓 名】：Ashyfox "<< endl;
	cout << "【学 号】：22231044" << endl;
	cout << "【OSLAB】：生产者-消费者问题模拟处理编程设计"<<endl;
	cout << "============================================"<<endl; 
    //定义生产者和消费者
    thread t1(producer, 1); 
    thread t2(consumer, 1); 
    thread t3(producer, 2);
    thread t4(consumer, 2);
    thread t5(producer, 3);
    thread t6(consumer, 3);
    thread t7(producer, 4);
    thread t8(consumer, 4); 
	//线程开始工作
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();
    t7.join();
    t8.join();

    return 0;
}

