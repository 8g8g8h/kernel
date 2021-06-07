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


static int devone_devs=2;  /*device count*/
static int devone_major=0; /*割り当てたメジャー番号が格納される*/
module_param(devone_major,uint,0); /*module_paramマクロでグローバル変数の初期値を最初に自分で設定できる*/
static struct cdev devone_cdev;

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
	dev_t dev=MKDEV(devone_major,0);
	int major;
	int alloc_ret=0;
	int cdev_err=0;

	alloc_ret=alloc_chrdev_region(&dev,0,devone_devs,DRIVER_NAME);
	if(alloc_ret){
		goto error;
	}

	devone_major=major=MAJOR(dev);

	cdev_init(&devone_cdev,&devone_fops);
	devone_cdev.owner=THIS_MODULE;

	cdev_err=cdev_add(&devone_cdev,MKDEV(devone_major,0),devone_devs);

	if(cdev_err){
		goto error;
	}

	printk("%s driver (major %d)installed.¥n",DRIVER_NAME,devone_major);

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
