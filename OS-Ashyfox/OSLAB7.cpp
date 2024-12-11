#include<bits/stdc++.h>
using namespace std;

typedef struct PCB{  // 进程控制块 
    char name[20];
    int running_time; //  运行时间
    int enter_time; //   到达时间
    int priority;  //    优先级
    int end_time;      //完成时间
    int copyRunning_time;  //用于时间片轮转
    int start_time;  //  进程开始运行的时间
    struct PCB* next;
} PCB;

typedef struct PCBQueue{
    PCB* firstProg;
    PCB* LastProg;
    int size;
} PCBQueue;

void Queueinit(PCBQueue* queue){
    if(queue==NULL){
        return;
    }
    queue->size = 0;
    queue->LastProg = (PCB*)malloc(sizeof(PCB));
    queue->firstProg = queue->LastProg;
}

void EnterQueue(PCBQueue* queue,PCB* pro){   //加入进程队列
    queue->LastProg->next = (PCB*)malloc(sizeof(PCB));
    queue->LastProg = queue->LastProg->next;
    queue->LastProg->enter_time = pro->enter_time;
    memcpy(queue->LastProg->name,pro->name,sizeof(pro->name));
    queue->LastProg->priority = pro->priority;
    queue->LastProg->running_time = pro->running_time;
    queue->LastProg->copyRunning_time = pro->copyRunning_time;
    queue->LastProg->start_time = pro->start_time;
    queue->size++;
}
PCB* poll(PCBQueue* queue){ // 处理队列边界 
    PCB* temp = queue->firstProg->next;
    if(temp == queue->LastProg){
        queue->LastProg=queue->firstProg;
        queue->size--;
        return temp;
    }
    queue->firstProg->next = queue->firstProg->next->next;
    queue->size--;
    return temp;
}
// 按【进入顺序】排序 
void sortWithEnterTime(PCB pro[],int num){
   sort(pro,pro+num,[](const PCB &a,const PCB &b){
   		return a.enter_time < b.enter_time;});
}
// 按【运行时间】排序
void sortWithLongth(PCB pro[],int start,int end){
    sort(pro+start,pro+end,[](const PCB &a,const PCB &b){
    	return a.running_time<b.running_time;});
}
//按【响应比】倒序排序
void sortWithResponse(PCB pro[],int start,int end){
    sort(pro + start, pro + end, [](const PCB &a, const PCB &b) {
        float responseA = (a.start_time - a.enter_time + a.running_time) / (a.running_time + 0.0);
        float responseB = (b.start_time - b.enter_time + b.running_time) / (b.running_time + 0.0);
        return responseA > responseB;});
}
//按【权重】排序
void sortWithPriority(PCB pro[],int start,int end){
    sort(pro + start, pro + end, [](const PCB &a, const PCB &b) {
        return a.priority < b.priority;});
}
// 先来先服务算法
void FCFS(PCB pro[],int num){ 
    printf("进程 到达时间  服务时间 开始时间 完成时间 周转时间 带权周转时间\n");
    sortWithEnterTime(pro,num);    //按照进入顺序排序
    PCBQueue* queue = (PCBQueue*)malloc(sizeof(PCBQueue));
    Queueinit(queue);
    EnterQueue(queue,&pro[0]);
    int time = pro[0].enter_time;
    int pronum=1;    //记录当前的进程
    float sum_T_time = 0; //平均周转时间
    float sum_QT_time = 0 ; // 带权平均周转时间

    while(queue->size>0){
        PCB* curpro = poll(queue);   //从进程队列中取出进程
        if(time < curpro->enter_time) time =  curpro->enter_time;
        int end_time = time+curpro->running_time; //完成时间
        int T_time = end_time - curpro->enter_time; //周转时间
        sum_T_time += T_time;
        float QT_time = T_time / (curpro->running_time+0.0); // 带权周转时间
        sum_QT_time += QT_time;
        for(int tt = time;tt<=end_time && pronum<num;tt++){    //模拟进程的执行过程
            if(tt >= pro[pronum].enter_time){
                EnterQueue(queue,&pro[pronum]);
                pronum++;
            }
        }
        printf("%s\t%d\t%d\t%d\t%d\t%d\t%.2f\n",curpro->name,curpro->enter_time,
		curpro->running_time,time,end_time,T_time,QT_time);
        time += curpro->running_time;
        if(queue->size==0 && pronum < num){   
            EnterQueue(queue,&pro[pronum]);
            pronum++;
        }
    }
    printf("平均周转时间为%.2f\t平均带权周转时间为%.2f\n",
	sum_T_time/(num+0.0),sum_QT_time/(num+0.0));
}
// 短进程优先算法
void SPF(PCB pro[],int num) {
    printf("进程 到达时间  服务时间 开始时间 完成时间 周转时间 带权周转时间\n");
    sortWithEnterTime(pro,num);
    PCBQueue* queue = (PCBQueue*)malloc(sizeof(PCBQueue));;
    Queueinit(queue);
    EnterQueue(queue,&pro[0]);
    int time = pro[0].enter_time;
    int pronum=1; 
    float sum_T_time = 0,sum_QT_time = 0;
    while(queue->size>0){
        PCB* curpro = poll(queue);   //从进程队列中取出进程
        if(time <  curpro->enter_time) time =  curpro->enter_time;
        int end_time = time+curpro->running_time;
        int T_time = end_time - curpro->enter_time;
        float QT_time = T_time / (curpro->running_time+0.0) ;
        sum_T_time += T_time;
        sum_QT_time += QT_time;
        int pre = pronum;
        for(int tt = time;tt<=end_time&&pronum<num;tt++)    
            if(tt>=pro[pronum].enter_time) 
                pronum++;
        sortWithLongth(pro,pre,pronum);
        for(int i=pre;i<pronum;i++) EnterQueue(queue,&pro[i]);
        printf("%s\t%d\t%d\t%d\t%d\t%d\t%.2f\n",curpro->name,curpro->enter_time,
		curpro->running_time,time,end_time,T_time,QT_time);
        time+=curpro->running_time;
        if(queue->size==0&&pronum<num){   
            EnterQueue(queue,&pro[pronum]);
            pronum++;
        }
    }
    printf("平均周转时间为%.2f\t平均带权周转时间为%.2f\n",
	sum_T_time/(num+0.0),sum_QT_time/num);
}

//优先级调度算法
void HPF(PCB pro[],int num){
    printf("进程 到达时间  服务时间 开始时间 完成时间 周转时间 带权周转时间\n");
    sortWithEnterTime(pro,num);
    PCBQueue* queue = (PCBQueue*)malloc(sizeof(PCBQueue));;
    Queueinit(queue);
    EnterQueue(queue,&pro[0]);
    int time = pro[0].enter_time;
    int pronum=1;    //记录当前的进程
    float sum_T_time = 0,sum_QT_time = 0;
    while(queue->size>0){
        PCB* curpro = poll(queue);   //从进程队列中取出进程
        if(time<curpro->enter_time)
            time =  curpro->enter_time;
        int end_time = time+curpro->running_time;
        int T_time = end_time - curpro->enter_time;
        float QT_time = T_time / (curpro->running_time+0.0) ;
        sum_T_time += T_time;
        sum_QT_time += QT_time;
        int pre = pronum;
        for(int tt = time;tt<=end_time&&pronum<num;tt++)
            if(tt>=pro[pronum].enter_time) 
                pronum++;
        sortWithPriority(pro,pre,pronum);
        for(int i=pre;i<pronum;i++) EnterQueue(queue,&pro[i]);
        pre = pronum;
        printf("%s\t%d\t%d\t%d\t%d\t%d\t%.2f\n",curpro->name,curpro->enter_time,
		curpro->running_time,time,end_time,T_time,QT_time);
        time +=  curpro->running_time;
        if(queue->size==0&&pronum<num){  
            EnterQueue(queue,&pro[pronum]);
            pronum++;
        }
    }
    printf("平均周转时间为%.2f\t平均带权周转时间为%.2f\n",
	sum_T_time/(num+0.0),sum_QT_time/(num+0.0));
}
//时间片轮转调度
void RR(PCB pro[],int num){
    printf("请输入时间片大小：");
    int timeslice;
    scanf("%d",&timeslice);
    printf("进程 到达时间 服务时间 进入时间 完成时间 周转时间 带权周转时间\n");
    sortWithEnterTime(pro,num);
    PCBQueue* queue = (PCBQueue*)malloc(sizeof(PCBQueue));;
    Queueinit(queue);
    pro[0].start_time = pro[0].enter_time;
    EnterQueue(queue,&pro[0]);
    int time = 0;
    int pronum = 1;
    float sum_T_time = 0,sum_QT_time = 0;
    while(queue->size>0){
        PCB* curpro = poll(queue);   
        if(time<curpro->enter_time)
            time = curpro->enter_time;
        if(timeslice >= curpro->running_time){  
            for(int tt = time;tt<=time+curpro->running_time&&pronum<num;tt++){ 
                if(tt>=pro[pronum].enter_time){
                    pro[pronum].start_time = tt;
                    EnterQueue(queue,&pro[pronum]);
                    pronum++;
                }
            }
            time += curpro->running_time;
            curpro->running_time = 0;
            curpro->end_time = time;
            int T_time = curpro->end_time-curpro->start_time;
            float QT_time = T_time / (curpro->copyRunning_time+0.0);
            sum_T_time += T_time;
            sum_QT_time += QT_time;
            printf("%s\t%d\t%d\t  %d\t   %d\t %d\t  %.2f\n",curpro->name,curpro->enter_time,
			curpro->copyRunning_time,curpro->start_time,curpro->end_time,T_time,QT_time);
            if(queue->size==0&&pronum<num){   
                pro[pronum].start_time = pro[pronum].enter_time;
                EnterQueue(queue,&pro[pronum]);
                pronum++;
            }
            continue;
        }
        // 运行时间大于时间片
        for(int tt = time;tt<=time+timeslice&&pronum<num;tt++){    
            if(tt>=pro[pronum].enter_time){
                pro[pronum].start_time = tt;
                EnterQueue(queue,&pro[pronum]);
                pronum++;
            }
        }
        time += timeslice;
        curpro->running_time -= timeslice;
        //当前程序未完成  继续添加到队列中
        EnterQueue(queue,curpro);
        if(queue->size==0&&pronum<num){ 
            pro[pronum].start_time = pro[pronum].enter_time;
            EnterQueue(queue,&pro[pronum]);
            pronum++;
        }
    }
    printf("平均周转时间为%.2f\t平均带权周转时间为%.2f\n",
	sum_T_time/(num+0.0),sum_QT_time/(num+0.0));
}
//高响应比优先
void HRRN(PCB pro[],int num) {
    printf("进程 到达时间  服务时间 开始时间 完成时间 周转时间 带权周转时间\n");
    sortWithEnterTime(pro,num);
    PCBQueue* queue = (PCBQueue*)malloc(sizeof(PCBQueue));;
    Queueinit(queue);
    EnterQueue(queue,&pro[0]);
    int time = pro[0].enter_time;
    int pronum=1;    //记录当前的进程
    float sum_T_time = 0,sum_QT_time = 0;
    while(queue->size>0){
        PCB* curpro = poll(queue);  
        if(time <  curpro->enter_time) time =  curpro->enter_time;
        int end_time = time+curpro->running_time;
        int T_time = end_time - curpro->enter_time;
        float QT_time = T_time / (curpro->running_time+0.0) ;
        sum_T_time += T_time;
        sum_QT_time += QT_time;
        int pre = pronum;
        for(int tt = time;tt<=end_time&&pronum<num;tt++)   
            if(tt>=pro[pronum].enter_time) 
                pronum++;
        sortWithResponse(pro,pre,pronum);
        for(int i=pre;i<pronum;i++) EnterQueue(queue,&pro[i]);
        pre = pronum;
        printf("%s\t%d\t%d\t%d\t%d\t%d\t%.2f\n",curpro->name,curpro->enter_time,
		curpro->running_time,time,end_time,T_time,QT_time);
        time +=  curpro->running_time;
        if(queue->size==0&&pronum<num){  
            EnterQueue(queue,&pro[pronum]);
            pronum++;
        }
    }
    printf("平均周转时间为%.2f\t平均带权周转时间为%.2f\n",
	sum_T_time/(num+0.0),sum_QT_time/num);
}
int main(){
	printf("======================================\n");
	printf("实验课题7-处理器调度算法模拟实现与比较\n");
	printf("个人信息：姓名：Ashyfox	学号：22231044\n");
	printf("======================================\n");
    int proNum; 
    printf("请输入进程的个数：");
    cin >> proNum;
    PCB pro[proNum];
    for(int i=0;i<proNum;i++){
        printf("请输入第%d个进程的名字、到达时间、服务时间、优先级（空格区分）：",i+1);
        cin>>pro[i].name>>pro[i].enter_time>>pro[i].running_time>>pro[i].priority; 
        pro[i].copyRunning_time = pro[i].running_time;
   }
    int choice;
    while(1){
    	printf("---------------------------------------------------------------\n");
		printf("请选择进程调度算法：\n");
    	printf("1.先来先服务算法\n2.短进程优先算法\n3.优先级优先算法\n4.时间片轮转算法\n5.高响应比优先算法\n0.退出\n");
    	printf("请输入您要进行的操作：");
        cin >> choice;
        printf("---------------------------------------------------------------\n");
        switch(choice){
            case 1:FCFS(pro,proNum);break;
            case 2:SPF(pro,proNum);break;
            case 3:HPF(pro,proNum);break;
            case 4:RR(pro,proNum);break;
            case 5:HRRN(pro,proNum);break;
            case 0:return 0;
            default:printf("输入存在错误，请重新输入!\n");break;
        }
    }
    return 0;
}

