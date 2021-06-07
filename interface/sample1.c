#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<setjmp.h>

#ifdef __linux__
#undef setjmp
#endif

#define TCB_NUM 16

int count1=0;
int count2=0;

struct task_define{
    char *name;
    int (*mainfunc)(int argc,char *argv[]);
    int stack_size;
};

struct task_control_block{
    struct task_define *task;	//ここがタスクについての情報
    jmp_buf context;	//ここで、バッファを用意している
} tcbs[TCB_NUM];

struct task_control_block *current;
struct task_control_block *max;

void schedule(void){
	current++;		//ここで現在のタスクがどこかを示している
	printf("schedule_current_task:%s\n",current->task->name);
	printf("schedule %s\n",current->context);
	if(current==max){
		current=tcbs;	//ここで最初のタスクに戻るようにしている
	}
}

void dispatch(void){
	printf("dispatch_current_context:%s\n",&current->context);
	longjmp(current->context,1);	//ここでカレントタスクのコンテキストを復元している
	printf("dispatch_current_context:%s\n",&current->context);
}


void context_switch(void){
	//ここで呼び出されたときのレジスタ情報をタスクのコンテキストとして保存した場合に返り値が1になるのでこ
	if(setjmp(current->context)!=0){	
		return;
	}
	schedule();	//ここで現在実行しているタスク(current)を一つ動かしている
	printf("after schedule\n");
	dispatch();	
}

int task1_main(int argc,char *argv[]){
	while(1){
		fprintf(stderr,"task 1 %s %p\n",argv[0],current);
		context_switch();
		sleep(1);
	}

	return 0;
}

int task2_main(int argc,char *argv[]){
	while(1){
		fprintf(stderr,"task 2 %s %p\n",argv[0],current);
		context_switch();
		sleep(1);
	}

	return 0;
}
int task3_main(int argc,char *argv[]){
	while(1){
		fprintf(stderr,"task 3 %s %p\n",argv[0],current);
		context_switch();
		sleep(1);
	}

	return 0;
}

struct task_define tasks[]={
	//ここでタスクを作っている
	{"task1",task1_main,4096*4},
	{"task2",task2_main,4096*4},
	{"task3",task3_main,4096*4},
	{NULL,NULL,0}
};

void start(void){
	max=current;		//ここで最初に戻すための準
	current=tcbs;		//ここで最初に戻している
	printf("start_current_context:%s\n",&current->context);
	printf("start_dispatch\n");
	dispatch();		//ここでバッファからレジスタ情報を戻す
}

void task_create(struct task_define *task);

void expand_stack(int size,struct task_define *task, char *dummy){
	printf("expand_stack\n");
	char stack[1024];
	if(size<0){
		printf("next_task\n");
		printf("exapnd_current task:%s\n",current->task->name);
		current++;				//ここで次のタスクを行うための準備
		task_create(++task);			//ここで現在実行しているタスクを次のものにしている
	}
	expand_stack(size-sizeof(stack),task,stack);	//ここでサイズをマイナスにしてもう一回この関数を実行するようにしている。そこで次のタスクを実行するようにしている
}

void task_create(struct task_define *task){
	char *args[2];
	printf("before count: %d\n",count1);
	if(task->name==NULL){
		printf("NULL\n");
		start();				//ここで最初のtask1_mainにタスクを設定し直している
	}
	//ここで現在実行しているタスクを設定している
	current->task=task;
	printf("current_task:%s\n",current->task->name);
	//ここで現在のタスクのレジスタ情報についてバッファに保存して、expand_stackを実行している
	if(setjmp(current->context)==0){
		printf("create_current_context:%s\n",&current->context);
		printf("setjmp\n");
		expand_stack(task->stack_size,task,NULL);
	}
	count1++;
	printf("after count,%d\n",count1);
	args[0]=current->task->name;
	args[1]=NULL;
	printf("task start!\n");
	current->task->mainfunc(1,args);
	
}

int main(void){
	memset(tcbs,0,sizeof(tcbs));
	current=tcbs;		//ここで、現在実行されているタスクをtcbsにしている
	task_create(tasks);	//taskの配列
	return 0;
}
