#include <stdio.h>
#include  <stdlib.h>
#include <string.h>
#include <time.h>
#define MAX 5010
// 页面信息结构
struct PageInfo
{
    int pages[MAX];         // 模拟的最大访问页面数
    int rw[MAX];           // 访问页面的随机数值是否大于0.7
    int page_missing_num;   // 缺页中断次数
    int allocated_page_num; // 分配的页框数
    int visit_list_length;  // 访问页面序列长度
} pInfo;

// 页表项信息
typedef  struct
{
    int time;      // 记录页框中数据的访问次数
    int isVisit;  // 访问位
    int isModify; // 修改位
    int isRW;     // 支持读访问  0 / 支持写访问 1
    int pages;     // 页号
} MemInfo;
// 全局变量定义
MemInfo pageList[MAX];          // 分配的页框
int current_page, replace_page; // 页面访问指针、页面替换指针
int isLoss;                    // 缺页标志： 1 缺页， 0 命中
int N;                          // 进程逻辑地址空间页面总数
void Init()
{ // 初始化函数：结构体、全局变量的初始化
    printf("===================================================\n");
    printf("实验课题四-页面淘汰算法模拟实现与比较\n");
    printf("个人信息：姓名-Ashyfox 学号-22231044\n");
    printf("===================================================\n");

    printf("请输入进程逻辑地址空间页面总数：");
    scanf("%d", &N); // 进程逻辑地址空间页面总数
    getchar();

    isLoss = 0;
    pInfo.page_missing_num = 0;

    printf("请输入分配的页框数："); // 自定义分配的页框数
    scanf("%d", &pInfo.allocated_page_num);
    getchar();

    if (pInfo.allocated_page_num > N)
    {
        printf("空间不足！将分配页框数为%d\n", N);
        pInfo.allocated_page_num = N;
    }

    for (int i = 0; i < N; i++) // 清空页面序列
        pInfo.pages[i] = -1;
    for (int i = 0; i < pInfo.allocated_page_num; i++)
        pageList[i].isRW = rand() % 2 ? 1 : 0; // 随机生成是否支持读写访问
}

// 随机发生页面访问序列
void RandomSequence()
{
    int w, v, l;           // 工作集中包含的页数w，工作集移动速率v
    int s = rand() % N; // 随机初始化工作集起始页号 s∈[0, N)
    printf("请输入工作集包含的页数：");
    scanf("%d", &w);
    getchar();
    printf("请输入工作集移动率：");
    scanf("%d", &v);
    getchar();
    printf("请输入访问序列的长度：");
    scanf("%d", &l);
    pInfo.visit_list_length = l; 
    getchar();
    srand((unsigned int)time(NULL)); // 随机种子
    double t = rand() % 10 * 0.1;

    int j = 0;
    for (int i = 0; i < l; i++)
    {
        if (j < l)
        {
            pInfo.pages[i] = (s + (rand() % w)) % N; 
            double r_w = (rand() % 10) * 0.1; 
            pInfo.rw[i] = r_w > 0.7 ? 1 : 0; 
            j++;                             
        }
        if (j % v == 0)
        { // 生成取值区间为 [0, 1] 的一个随机数 r
            double r = (rand() % 10) * 0.1;
            if (r < t)          // 比较 r 与 t 的大小
                s = rand() % N; // 为 s 生成一个新值（s∈[0, N)）
            else
                s = (s + 1) % N;
        }
    }
    printf("\n-----------------页面访问序列-----------------\n");
    for (int i = 0; i < l; i++)
    {
        printf("%d ", pInfo.pages[i]);
        if ((i + 1) % 10 == 0)
            printf("\n");   // 每行显示10个
    }
    if (l % 10 != 0)
        printf("\n");
    printf("----------------------------------------------\n");
}

// 跟踪显示状态信息
void showState()
{
    printf("访问%d -->内存空间[", pInfo.pages[current_page]);
    for (int i = 0; i < pInfo.allocated_page_num; i++)
    {
        if (pageList[i].pages >= 0)
            printf("%d ", pageList[i].pages);
        else
            printf("   ");
    }
    printf("]");
    if (isLoss)
    {
        if (!pInfo.page_missing_num)
        {
            printf("--> 预加载页面......\n");
        }
        else
        {
            printf(" --> 缺页中断 --> 缺页率 = %.2f\n", (float)(pInfo.page_missing_num) * 100.00 / current_page);
        }
    }
    else
        printf("--> 命中\n");
}

// 查找页面是否已经在内存中
int isExist(){
    for (int i = 0; i < pInfo.allocated_page_num; i++){
        pageList[i].time++;
        if (pageList[i].pages == pInfo.pages[current_page])
        {
            isLoss = 0;
            pageList[i].time = 0;
            pageList[i].isVisit = 1;
            if (pageList[i].isRW && pInfo.rw[current_page])
                pageList[i].isModify = 1;
            return 1;
        }
    }
    isLoss = 1;
    return 0;
}

// 最佳淘汰算法
void OPT(){
    pInfo.page_missing_num = 0;
    int full, status;
    replace_page = 0;                                  // 页面替换指针初始化为0
    full = 0;                                          // 已经占用的页框数
    isLoss = 0;                                        // 缺页标志，0为不缺页，1为缺页
    for (int i = 0; i < pInfo.allocated_page_num; i++) // 清除页框信息
        pageList[i].pages = -1;
    for (current_page = 0; current_page < pInfo.visit_list_length; current_page++){ // 遍历所有的访问页面
        if (!isExist()){ // 页不存在则装入页面
            if (full < pInfo.allocated_page_num){                                                                 // 当已经占用的页框数 < 分配的框数时，则不可能发生缺页
                pageList[replace_page].pages = pInfo.pages[current_page];     // 当前位置放入新的页面
                replace_page = (replace_page + 1) % pInfo.allocated_page_num; // 循环后移一位
                full++;                                                       // 已经装入的页面数+1
            }else{                     // 页框已经满，缺页置换
                int min, max = 0; // min 用来记录页面出现的最晚时间，max 用来记录最远的页面
                for (int m = 0; m < pInfo.allocated_page_num; m++){ // 遍历当前内存中已分配的页面
                    min = 1000;
                    for (int i = current_page; i < pInfo.visit_list_length; i++){ // 向后查找访问序列
                        if (pInfo.pages[i] == pageList[m].pages){// 如果访问序列中的某个页面等于当前页面
                            min = i; // 记录该页面下一次出现在访问序列中的位置
                            break;   // 找到之后可以跳出循环
                        }
                    }
                    if (max < min){ // 如果当前页面下次出现的时间比目前最远的页面出现时间更远
                        max = min;        // 更新最远时间为当前页面的出现时间
                        replace_page = m; // 将当前页面标记为要被替换的页面
                    }
                }
                pageList[replace_page].pages = pInfo.pages[current_page]; // 置换动作
                replace_page = (replace_page + 1) % pInfo.allocated_page_num;
                pInfo.page_missing_num++; // 置换次数加1
            }
        }
        showState();
    }	return;
}

// 先进先出的淘汰算法
void FIFO(){
    pInfo.page_missing_num = 0;
    int full, status;
    replace_page = 0;                                  // 页面替换指针初始化为0
    full = 0;                                          // 是否装满是所有的页框
    for (int i = 0; i < pInfo.allocated_page_num; i++) // 清除页框信息
        pageList[i].pages = -1;
    isLoss = 0; // 缺页标志，0为不缺页，1为缺页
    for (current_page = 0; current_page < pInfo.visit_list_length; current_page++){
        // status = isExist(); // 查找页面是否在内存
        if (!isExist()){
            if (full < pInfo.allocated_page_num){ // 装入页面
                pageList[replace_page].pages = pInfo.pages[current_page];
                replace_page = (replace_page + 1) % pInfo.allocated_page_num;
                full++;
            }else{ // 直接置换
                pageList[replace_page].pages = pInfo.pages[current_page];
                replace_page = (replace_page + 1) % pInfo.allocated_page_num;
                pInfo.page_missing_num++; // 缺页次数加1
            }
        }
        showState(); // 显示当前状态
    }
    return;
}

// 最近最久未使用淘汰算法
void LRU(void){
    pInfo.page_missing_num = 0;
    int full, status, max;
    replace_page = 0; // 页面替换指针
    full = 0;         // 是否装满所有的页框
    for (int i = 0; i < pInfo.allocated_page_num; i++){
        pageList[i].pages = -1; // 清除页框信息
        pageList[i].time = 0;   // 清除页框历史
    }
    isLoss = 0; // 缺页标志，0为不缺页，1为缺页
    for (current_page = 0; current_page < pInfo.visit_list_length; current_page++){
        if (!isExist()){
            if (full < pInfo.allocated_page_num){                                                             // 开始时不计算缺页, 页不存在则装入页面
                pageList[replace_page].pages = pInfo.pages[current_page]; 
                replace_page = (replace_page + 1) % pInfo.allocated_page_num;
                full++;
            }else{ // 正常缺页置换, 页不存在则置换页面
                max = 0;
                for (int i = 1; i < pInfo.allocated_page_num; i++)
                    if (pageList[i].time > pageList[max].time) max = i;           
                replace_page = max;
                pageList[replace_page].pages = pInfo.pages[current_page];
                pageList[replace_page].time = 0;
                pInfo.page_missing_num++; // 缺页次数加1
            }
        }
        showState(); // 显示当前状态
    }
    return;
}

// Clock淘汰算法 : 0 简单 1 改进
// 简单算法置换策略
int replace_page_clock(int num)
{
    int j;
    // 第一轮： 查访问位，置为0
    for (j = 0; j < pInfo.allocated_page_num; j++)
    {
        if (pageList[(j + num) % pInfo.allocated_page_num].isVisit == 0)
            return (j + num) % pInfo.allocated_page_num;
        pageList[(j + num) % pInfo.allocated_page_num].isVisit = 0;
    }
    // 第二轮，查访问位
    for (j = 0; j < pInfo.allocated_page_num; j++)
        if (pageList[(j + num) % pInfo.allocated_page_num].isVisit == 0)
            return (j + num) % pInfo.allocated_page_num;
    return 0;
}
// 改进算法置换策略
int replace_page_pro(int num){
	int len = pInfo.allocated_page_num;
    // 第一轮：找（0,0）
    for (int j = 0; j < len; j++)
        if (pageList[(j + num) % len].isVisit == 0 && pageList[(j + num) % len].isModify == 0)
            return (j + num) % len;
    // 第二轮：找（0,1），访问位全置为0
    for (int j = 0; j < len; j++)
    {
        if (pageList[(j + num) % len].isVisit == 0 && pageList[(j + num) % len].isModify == 1)
            return (j + num) % len;
        pageList[(j + num) % len].isVisit = 0;
    }
    // 第三轮：找（0,0）
    for (int j = 0; j < len; j++)
        if (pageList[(j + num) % len].isVisit == 0 && pageList[(j + num) % len].isModify == 0)
            return (j + num) % len;
    // 第四轮：找（0,1）
    for (int j = 0; j < len; j++)
        if (pageList[(j + num) % len].isVisit == 0 && pageList[(j + num) % len].isModify == 1)
            return (j + num) % len;
    return 0;
}
void CLOCK_pro(int choose)
{
    pInfo.page_missing_num = 0;
    int n, full, status;
    int num = -1;
    replace_page = 0; // 页面替换指针初始化为0
    full = 0;         // 是否装满是所有的页框
    for (n = 0; n < pInfo.allocated_page_num; n++){ // 清除页框信息
        pageList[n].pages = -1;
        pageList[n].isModify = 0;
        pageList[n].isVisit = 0;
        pageList[n].time = 0;
    }
    isLoss = 0; // 缺页标志，0为不缺页，1为缺页
    for (current_page = 0; current_page < pInfo.visit_list_length; current_page++){ // 执行算法
        if (!isExist()){
            if (full < pInfo.allocated_page_num){ // 开始时不计算缺页
                // 页不存在则装入页面
                pageList[replace_page].pages = pInfo.pages[current_page];
                replace_page = (replace_page + 1) % pInfo.allocated_page_num;
                pageList[n].isVisit = 1;
                full++;
            }else{   // 正常缺页置换, 页不存在则置换页面
                if (choose == 1)
                    replace_page = replace_page_pro(++num); // 当choose = 1时采用改进的clock算法
                else if (choose == 0)
                    replace_page = replace_page_clock(++num); // 当choose = 0时采用基本的clock算法
                pageList[replace_page].pages = pInfo.pages[current_page];
                pageList[replace_page].isVisit = 1;
                pInfo.page_missing_num++; // 缺页次数加1
            }
        }
        showState(); // 显示当前状态
    } // 置换算法循环结束
    return;
}

int main()
{
    char ch;
    Init();
    RandomSequence();
    clock_t start, finish;
    double totaltime;
    while (1)
    {
        printf("=======================菜单=======================\n");
        printf("1. 最佳淘汰算法(OPT)\n");
        printf("2. 先进先出淘汰算法(FIFO)\n");
        printf("3. 最近最久未使用淘汰算法(LRU)\n");
        printf("4. 简单 Clock 淘汰算法\n");
        printf("5. 改进型 Clock 淘汰算法\n");
        printf("0. 退出菜单\n");
        printf("=================================================\n");
        printf("请输入选择的算法: ");
        ch = getchar();
        switch (ch)
        {
        case '1':
            printf("\n《------------1. 最佳淘汰算法(OPT)--------------》\n");
            start = clock();
            OPT();
            finish = clock();
            totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
            printf("\n运行总时长 = %lf 秒\n", totaltime);
            getchar();
            break;
        case '2':
            printf("\n\n《----------2. 先进先出淘汰算法(FIFO)------------》\n");
            start = clock();
            FIFO();
            finish = clock();
            totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
            printf("\n运行总时长 = %lf 秒\n", totaltime);
            getchar();
            break;
        case '3':
            printf("\n\n《--------3. 最近最久未使用淘汰算法(LRU)-----------》\n");
            start = clock();
            LRU();
            finish = clock();
            totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
            printf("\n运行总时长 = %lf 秒\n", totaltime);
            getchar();
            break;
        case '4':
            printf("\n\n《------------ 4. 简单 Clock 淘汰算法--------------》\n");
            start = clock();
            CLOCK_pro(0);
            finish = clock();
            totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
            printf("\n运行总时长 = %lf 秒\n", totaltime);
            getchar();
            break;
        case '5':
            printf("\n\n《------------ 5. 改进型Clock 淘汰算法--------------》\n");
            start = clock();
            CLOCK_pro(1);
            finish = clock();
            totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
            printf("\n运行总时长 = %lf 秒\n", totaltime);
            getchar();
            break;
        case '0':
            printf("已退出！\n");
            return 0;
        default:
            printf("输入存在问题，请重新输入！\n");
            break;
        }
    }
    return 0;
}
