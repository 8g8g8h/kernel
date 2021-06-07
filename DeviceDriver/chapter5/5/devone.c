#include<linux/init.h>
#include<linux/module.h>
#include<linux/types.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/sched.h>
#include<asm/current.h>
#include<linux/uaccess.h>
#include<linux/slab.h>
#include<linux/device.h>

#define DRIVER_NAME "devone"

static int devone_devs=2;
static int devone_major=0; /*割り当てたメジャー番号が格納される*/
module_param(devone_major,uint,0); /*module_paramマクロでグローバル変数の初期値を最初に自分で設定できる*/
static struct cdev devone_cdev;

ssize_t one_write(struct file *filp,const char __user *buf,size_t count,loff_t *f_pos){
		printk("%s called¥n",__func__);
		
		return 0;
}

ssize_t one_read(struct file *filp,char __user *buf,size_t count,loff_t *f_pos){
		printk("%s called¥n",__func__);
		return 0;
}

static int one_close(struct inode *inode,struct file *file){
		printk("%s:major %d minor %d (pid %d)¥n",__func__,imajor(inode),iminor(inode),current->pid);/*major、minor番号を取得するためにinodeから番号情報を取得する*/
		
		return 0;
}

static int one_open(struct inode *inode,struct file *file){
		printk("%s:major %d minor %d (pid %d)¥n",__func__,imajor(inode),iminor(inode),current->pid);/*major、minor番号を取得するためにinodeから番号情報を取得する*/
		
		return 0;
}

struct file_operations one_fops={
		.open=one_open,
		.release=one_close,
		.read=one_read,
		.write=one_write,
};

ssize_t zero_write(struct file *filp,const char __user *buf,size_t count,loff_t *f_pos){
		printk("%s called¥n",__func__);
		return 0;
}

ssize_t zero_read(struct file *filp,char __user *buf,size_t count,loff_t *f_pos){
		printk("%s called¥n",__func__);
		return 0;
}

static int zero_close(struct inode *inode,struct file *file){
		printk("%s:major %d minor %d (pid %d)¥n",__func__,imajor(inode),iminor(inode),current->pid);/*major、minor番号を取得するためにinodeから番号情報を取得する*/
		
		return 0;
}

static int zero_open(struct inode *inode,struct file *file){
		printk("%s:major %d minor %d (pid %d)¥n",__func__,imajor(inode),iminor(inode),current->pid);/*major、minor番号を取得するためにinodeから番号情報を取得する*/
		
		return 0;
}

struct file_operations zero_fops={
		.open=zero_open,
		.release=zero_close,
		.read=zero_read,
		.write=zero_write,
};


/*openシステムコール*/
static int devone_open(struct inode *inode,struct file *file){

	printk("%s:major %d minor %d (pid %d)¥n",__func__,imajor(inode),iminor(inode),current->pid);/*major、minor番号を取得するためにinodeから番号情報を取得する,ここでcurrent->pidでopenを呼び出したユーザープロセスの情報を確保している*/

	switch(iminor(inode)){
		case 0:
			file->f_op=&zero_fops;
			break;
			
		case 1:
			file->f_op=&one_fops;
			break;
			
		default:
			return -ENXIO;
	}
	
	if(file->f_op && file->f_op->open){
		return file->f_op->open(inode,file);
	}

	return 0; /*success!*/
}

struct file_operations devone_fops={
		.open=devone_open,
};


/*ドライバをロードしたときに呼ばれる関数*/
static int devone_init(void){

	dev_t dev=MKDEV(devone_major,0);		/*操作するデバイスがどれか(メジャー番号はいくつかなどの情報)をdevに保存している*/
	int alloc_ret=0;
	int major;
	int cdev_err=0;

	
	/* 1. 空いているメジャー番号を確保する */
	alloc_ret=alloc_chrdev_region(&dev,0,2,DRIVER_NAME); /*ここで空いているメジャー番号はdevに登録される*/
	if(alloc_ret){
		goto error;
	}
	 /* 2. 取得したdev( = メジャー番号 + マイナー番号)からメジャー番号を取得して保持しておく */
	devone_major=MAJOR(dev);
	
	 /* 3. cdev構造体の初期化とシステムコールハンドラテーブルの登録 */
	cdev_init(&devone_cdev,&devone_fops);			/*ここでドライバで使うシステムコールを登録しておく。(fopsがシステムコールハンドラである)。cdev構造体はドライバの情報のようなもの(file headerのようなもの)*/
	devone_cdev.owner=THIS_MODULE;					/*これは必要な作業。特に気にしなくてもok*/

	/* 4. このデバイスドライバ(cdev)をカーネルに登録する */
	cdev_err=cdev_add(&devone_cdev,dev,2);		/*ドライバをカーネルへ登録する*/
	if(cdev_err){
		goto error;
	}

	printk("%s driver (major %d)installed.¥n",DRIVER_NAME,devone_major);

	return 0;

error:
	if(cdev_err==0){
		cdev_del(&devone_cdev);
	}
	if(alloc_ret==0){
		unregister_chrdev_region(dev,devone_devs);
	}
	return -1;
}

/*ドライバをアンロードしたときに呼ばれる関数*/
static void devone_exit(void){

	dev_t dev =MKDEV(devone_major,0);

	cdev_del(&devone_cdev);
	unregister_chrdev_region(dev,devone_devs);

	printk("%s driver removed.¥n",DRIVER_NAME);
}

module_init(devone_init);
module_exit(devone_exit);
