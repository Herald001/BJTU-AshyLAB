#include <iostream>
#include <thread>
#include <mutex>
using namespace std;

mutex chopsticks[5]; // 五个筷子
mutex cout_mutex; // 输出保护锁
int state[5]; // 哲学家的状态，0 表示思考，1 表示饥饿，2 表示就餐

// 哲学家线程函数，参数为哲学家的编号
void philosopher(int num) {
    int left_chopstick = num; // 左边的筷子编号
    int right_chopstick = (num + 1) % 5; // 右边的筷子编号

    while (true) {
        // 思考
        state[num] = 0;
        cout_mutex.lock();
        cout << "【哲学家" << num << "】正在思考" << endl;
        cout_mutex.unlock();
        this_thread::sleep_for(chrono::milliseconds(500));
        // 饥饿
        state[num] = 1;
        cout_mutex.lock();
        cout << "【哲学家" << num << "】感到饥饿" << endl;
        cout_mutex.unlock();
        // 尝试获取筷子 
        if (num % 2 == 0) {
            // 偶数号哲学家：先拿右筷子，再拿左筷子
            chopsticks[right_chopstick].lock();
            cout_mutex.lock();
            cout << "【哲学家" << num << "】拿起[右边]筷子" << right_chopstick << endl;
            cout_mutex.unlock();
            chopsticks[left_chopstick].lock();
            cout_mutex.lock();
            cout << "【哲学家" << num << "】拿起[左边]筷子" << left_chopstick << endl;
            cout_mutex.unlock();
        } else {
            // 奇数号哲学家：先拿左筷子，再拿右筷子
            chopsticks[left_chopstick].lock();
            cout_mutex.lock();
            cout << "【哲学家" << num << "】拿起[左边]筷子" << left_chopstick << endl;
            cout_mutex.unlock();
            chopsticks[right_chopstick].lock();
            cout_mutex.lock();
            cout << "【哲学家" << num << "】拿起[右边]筷子" << right_chopstick << endl;
            cout_mutex.unlock();
        }
        // 就餐
        state[num] = 2;
        cout_mutex.lock();
        cout << "【哲学家" << num << "】在吃'明湖烤鸭'" << endl;
        cout_mutex.unlock();
        this_thread::sleep_for(chrono::milliseconds(1000));
        // 放下左边的筷子
        chopsticks[left_chopstick].unlock();
        cout_mutex.lock();
        cout << "【哲学家" << num << "】放下[左边]筷子" << left_chopstick << endl;
        cout_mutex.unlock();
        // 放下右边的筷子
        chopsticks[right_chopstick].unlock();
        cout_mutex.lock();
        cout << "【哲学家" << num << "】放下[右边]筷子" << right_chopstick << endl;
        cout_mutex.unlock();
    }
}

int main() {
    cout << "============================================"<<endl; 
    cout << "【姓 名】：Ashyfox "<< endl;
    cout << "【学 号】：22231044" << endl;
    cout << "【OSLAB】：哲学家就餐问题模拟处理编程设计"<<endl;
    cout << "============================================"<<endl; 

    // 初始化哲学家状态
    for (int i = 0; i < 5; i++) {
        state[i] = 0;
    }

    // 创建五个哲学家线程
    thread t0(philosopher, 0);
    thread t1(philosopher, 1);
    thread t2(philosopher, 2);
    thread t3(philosopher, 3);
    thread t4(philosopher, 4);

    // 等待五个哲学家线程结束
    t0.join();
    t1.join();
    t2.join();
    t3.join();
    t4.join();

    return 0;
}

