#include<linux/init.h>
#include<linux/module.h>
#include<linux/types.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/sched.h>
#include<asm/current.h>
#include<asm/uaccess.h>

#define DRIVER_NAME "devone"

static unsigned int devone_major=0; /*割り当てたメジャー番号が格納される*/
module_param(devone_major,uint,0); /*module_paramマクロでグローバル変数の初期値を最初に自分で設定できる*/


/*openシステムコール*/
static int devone_open(struct inode *inode,struct file *file){
	printk("%s:major %d minor %d (pid %d)¥n",__func__,imajor(inode),iminor(inode),current->pid);/*major、minor番号を取得するためにinodeから番号情報を取得する,ここでcurrent->pidでopenを呼び出したユーザープロセスの情報を確保している*/

	/*ドライバがopenシステムコールでどこのリソースをopenしたのかを認識するためにそのポインタをそれぞれのプライベートポインタに書いておく*/
	inode->i_private=inode;
	file->private_data=file;

	printk("i_private=%p private_data=%p¥n",
			inode->i_private,file->private_data);

	return 0; /*success!*/

}

/*close システムコール*/
static int devone_close(struct inode *inode,struct file *file){

	printk("%s:major %d minor %d (pid %d)¥n",__func__,imajor(inode),iminor(inode),current->pid);/*major、minor番号を取得するためにinodeから番号情報を取得する*/

    printk("i_private=%p private_data=%p¥n",
			inode->i_private,file->private_data);

	return 0; /*success!*/
}


/*ドライバに対して、システムコールを発行できるようにシステムコールハンドラをここで登録している(.openのシステムコールが呼ばれたらdevone_openの関数が行われるというのをテーブルとして登録しておく)*/
struct file_operations devone_fops={
	.open=devone_open,
	.release=devone_close
};

/*ドライバをロードしたときに呼ばれる関数*/
static int devone_init(void){
	int major;
	int ret=0;

	major=register_chrdev(devone_major,DRIVER_NAME,&devone_fops); /*ここで、メジャー番号と、ドライバの名前、ファイルオペレーション構造体(システムコールのハンドラが格納されているもの)へのポインタを登録している。ちなみに最初の引数を0にすることにより動的にメジャー番号を割り当てることができる*/
	if((devone_major>0&&major!=0)||			/*静的にドライバを登録している場合*/
			(devone_major==0&&major<0)||	/*動的にドライバを登録しているが失敗した場合*/
			major<0){						/*純粋に失敗した場合*/
		printk("%s driver registration error¥n",DRIVER_NAME);
		ret=major;
		goto error;
	}
	if(devone_major==0){					/*動的にドライバを登録している*/
		devone_major=major;
	}

	printk("%s driver (major %d)installed.¥n",DRIVER_NAME,devone_major);

error:
	return(ret);
}

/*ドライバをアンロードしたときに呼ばれる関数*/
static void devone_exit(void){
	unregister_chrdev(devone_major,DRIVER_NAME);

	printk("%s driver removed.¥n",DRIVER_NAME);
}

module_init(devone_init);
module_exit(devone_exit);
