#include <iostream>
#include <cstdlib>
#include <time.h>
#include <vector>
#include <algorithm>
#include <utility>
#include <numeric>  // 用于累加
#define _CRT_SECURE_NO_WARNINGS 1
using namespace std;

const int maxRightValue = 199; // 磁头移动的最右边界值
const int minLeftValue = 0;    // 磁头移动的最左边界值
vector<int> FSCAN_SHOW;        // 存放FSCAN_SHOW算法的访问顺序

// 生成100个随机访问序列，pos为磁头初始位置
void getRandomNumber(vector<int>& randValue, int& pos) {
    randValue.clear();  // 清空之前的随机值
    for (int i = 1; i <= 100; i++) {
        int r = rand() % 200;  // 生成一个随机数，范围在0-199
        randValue.push_back(r); // 将随机数添加到序列中
    }
    pos = rand() % 200; // 磁头随机初始位置
}

// 打印生成的随机序列
void printRandomSequence(const vector<int>& randValue) {
    printf("---------------产生的随机序列如下：---------------\n");
    for (int i = 0; i < randValue.size(); i++) {
        printf("%-3d ", randValue[i]);
        if ((i + 1) % 10 == 0) {  // 每10个数字换行
            cout << "\n";
        }
    }
    cout << "\n";
}

// 打印访问序列
void Show(const string& str, const vector<int>& t) {
    cout << str << "的访问序列为:";
    for (int i = 0; i < t.size(); i++) {
        if (i != t.size() - 1)
            cout << t[i] << "->";
        else
            cout << t[i] << endl;
    }
}

// FCFS算法，先来先服务
int FCFS(const vector<int>& t, int pos, bool printFlag = true) {
    int sum = 0;
    if (printFlag) {
        Show("FCFS", t);
    }
    for (int i = 0; i < t.size(); i++) {
        sum += abs(pos - t[i]);
        pos = t[i];
    }
    return sum;
}

// 寻找距离当前位置最近的磁道
int findClose(const vector<int>& t, int pos) {
    int minDistance = INT_MAX;
    int index = -1;
    for (int i = 0; i < t.size(); i++) {
        if (t[i] == -1)
            continue;
        int distance = abs(pos - t[i]);
        if (minDistance > distance) {
            minDistance = distance;
            index = i;
        }
    }
    return index;
}

// SSTF算法，最短寻道时间优先
int SSTF(vector<int> t, int pos, bool printFlag = true) {
    vector<int> show;
    int sum = 0;
    for (int i = 0; i < t.size(); i++) {
        int index = findClose(t, pos);
        if (index == -1) {
            break;
        } else {
            show.push_back(t[index]);
            sum += abs(pos - t[index]);
            pos = t[index];
            t[index] = -1;
        }
    }
    if (printFlag) {
        Show("SSTF", show);
    }
    return sum;
}

// 假设磁盘指针总是向右移动的SCAN算法
pair<int, int> SCAN(vector<int> t, int pos, int flag = 1, bool printFlag = true) {
    int sum = 0;
    vector<int> left, right;
    vector<int> show;
    for (auto e : t) {
        if (e < pos) {
            left.push_back(e);
        } else {
            right.push_back(e);
        }
    }
    sort(left.begin(), left.end());
    sort(right.begin(), right.end());
    for (auto e : right) {
        show.push_back(e);
    }
    sum += maxRightValue - pos;
    if (!left.empty()) {
        sum += maxRightValue - left[0];
        reverse(left.begin(), left.end());

        for (auto e : left) {
            show.push_back(e);
        }
    }
    if (printFlag) {
        Show("SCAN", show);
    } else {
        for (auto e : show) {
            FSCAN_SHOW.push_back(e);
        }
    }
    pair<int, int> res;
    res.first = sum;
    res.second = pos;
    return res;
}

// CSCAN算法，循环扫描
int CSCAN(vector<int> t, int pos, bool printFlag = true) {
    int sum = 0;
    vector<int> left, right;
    vector<int> show;
    for (auto e : t) {
        if (e < pos) {
            left.push_back(e);
        } else {
            right.push_back(e);
        }
    }
    sort(left.begin(), left.end());
    sort(right.begin(), right.end());
    for (auto e : right) {
        show.push_back(e);
    }
    sum += maxRightValue - pos + 200;
    if (!left.empty()) {
        for (auto e : left) {
            show.push_back(e);
        }
        sum += left.back();
    }
    if (printFlag) {
        Show("CSCAN", show);
    }
    return sum;
}

// FSCAN算法，双队列扫描
int FSCAN(vector<int> t, int pos, bool printFlag = true) {
    int sum = 0;
    vector<int> t1(t.begin(), t.begin() + 50); // 前50个请求
    vector<int> t2(t.begin() + 50, t.begin() + 100); // 后50个请求
    const pair<int, int>& temp = SCAN(t1, pos, 0, false);
    sum += temp.first;
    sum += SCAN(t2, temp.second, 0, false).first;
    if (printFlag) {
        Show("FSCAN", FSCAN_SHOW);
    }
    return sum;
}

// 多次测试并计算平均寻道数
void runTests(int numTests) {
    vector<int> FCFS_results, SSTF_results, SCAN_results, CSCAN_results, FSCAN_results;

    for (int i = 0; i < numTests; i++) {
        vector<int> randValue;
        int pos = 0;
        getRandomNumber(randValue, pos);
        FCFS_results.push_back(FCFS(randValue, pos, false));
        SSTF_results.push_back(SSTF(randValue, pos, false));
        SCAN_results.push_back(SCAN(randValue, pos, 1, false).first);
        CSCAN_results.push_back(CSCAN(randValue, pos, false));
        FSCAN_results.push_back(FSCAN(randValue, pos, false));
    }

    // 计算每个算法的平均寻道数
    double FCFS_avg = accumulate(FCFS_results.begin(), FCFS_results.end(), 0) / static_cast<double>(numTests);
    double SSTF_avg = accumulate(SSTF_results.begin(), SSTF_results.end(), 0) / static_cast<double>(numTests);
    double SCAN_avg = accumulate(SCAN_results.begin(), SCAN_results.end(), 0) / static_cast<double>(numTests);
    double CSCAN_avg = accumulate(CSCAN_results.begin(), CSCAN_results.end(), 0) / static_cast<double>(numTests);
    double FSCAN_avg = accumulate(FSCAN_results.begin(), FSCAN_results.end(), 0) / static_cast<double>(numTests);

    cout << "\n=================== 测试结果汇总 ===================\n";
    cout << "FCFS算法的平均寻道数: " << FCFS_avg / 100 << endl;
    cout << "SSTF算法的平均寻道数: " << SSTF_avg / 100 << endl;
    cout << "SCAN算法的平均寻道数: " << SCAN_avg / 100 << endl;
    cout << "CSCAN算法的平均寻道数: " << CSCAN_avg / 100 << endl;
    cout << "FSCAN算法的平均寻道数: " << FSCAN_avg / 100 << endl;
}

int main() {
    printf("===================================================\n");
    printf("实验课题五-移动头磁盘调度算法模拟实现与比较\n");
    printf("个人信息：姓名-Ashyfox 学号-22231044\n");
    printf("===================================================\n");

    int pos = 0;
    vector<int> randValue;
    srand((unsigned)time(NULL)); // 设置随机种子
    getRandomNumber(randValue, pos);
    printRandomSequence(randValue);

    printf("\n=====移动头磁盘调度算法模拟实现菜单=====\n");
    printf("1.先来先服务算法(FCFS)\n");
    printf("2.最短寻道时间优先算法(SSTF)\n");
    printf("3.扫描算法(SCAN)\n");
    printf("4.循环扫描算法(CSCAN)\n");
    printf("5.双重扫描算法(FSCAN)\n");
    printf("0.退出\n");

    int numTests = 10000;
    while (1) {
        printf("请输入你的选项(1/2/3/4/5)：");
        int choice;
        cin >> choice;
        getchar(); // 获取输入后的换行符

        switch (choice) {
            case 1:
                cout << "FCFS的平均寻道数为:" << FCFS(randValue, pos) / 100 << endl;
                break;
            case 2:
                cout << "SSTF的平均寻道数为:" << SSTF(randValue, pos) / 100 << endl;
                break;
            case 3:
                cout << "SCAN的平均寻道数为:" << SCAN(randValue, pos).first / 100 << endl;
                break;
            case 4:
                cout << "CSCAN的平均寻道数为:" << CSCAN(randValue, pos) / 100 << endl;
                break;
            case 5:
                cout << "FSCAN的平均寻道数为:" << FSCAN(randValue, pos) / 100 << endl;
                break;
            case 0:
                printf("正在退出\n");
                runTests(numTests); // 正常调用测试函数
                return 0;
            default:
                printf("输入选项有误！\n");
                break;
        }
    }
    return 0;
}

