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

static int devone_devs=1;
static int devone_major=0; /*割り当てたメジャー番号が格納される*/
module_param(devone_major,uint,0); /*module_paramマクロでグローバル変数の初期値を最初に自分で設定できる*/
static struct cdev devone_cdev;

struct devone_data{
	unsigned char val;
	rwlock_t lock;
};

/*writeシステムコール*/
ssize_t devone_write(struct file *filp,const char __user *buf,size_t count,loff_t *f_pos){
	struct devone_data *p=filp->private_data;		/*データを保存*/
	unsigned char val;
	int retval=0;									/*バッファから読み込んだ(つまり書き込んだデータ)の数*/
	
	printk("%s: count %d pos %lld¥n",__func__,count,*f_pos);
	
	if(count>=1){									/*書き込むデータサイズが1以上の場合*/
		if(copy_from_user(&val,&buf[0],1)){
			retval=-EFAULT;
			goto out;
		}
		
		write_lock(&p->lock);			/*書き込むためにロックをかけている*/
		p->val=val;						/*データを書き込んでいる*/
		write_unlock(&p->lock);			/*書き終わったのでロックを解除している*/
		retval=count;					/*書き込んだデータの数を書いている*/
	}
	
out:
	return (retval);
}

/*readシステムコール*/
ssize_t devone_read(struct file *filp,char __user *buf,size_t count,loff_t *f_pos){
	struct devone_data *p=filp->private_data;		/*データを変数に保存*/
	unsigned char val;
	int i;
	int retval;									/*バッファから読み込んだ(つまり書き込んだデータ)の数*/
	
	read_lock(&p->lock);						/*読み込みロックを行う*/
	val=p->val;									/*データを読み込む*/
	read_unlock(&p->lock);						/*ロックを解除*/


	printk("%s: count %d pos %lld¥n",__func__,count,*f_pos);
	
	for(i=0;i<count;i++){
		if(copy_to_user(&buf[i],&val,1)){		/*copy_to_userでの返り値が0の場合(つまり、読み込むものがなかった)*/
			retval=-EFAULT;
			goto out;
		}
	}

	retval=count;								/*すべて読み込むことに成功しているのでcountをretvalに代入している*/
	
out:
	return (retval);
}

/*openシステムコール*/
static int devone_open(struct inode *inode,struct file *file){

	struct devone_data *p;

	printk("%s:major %d minor %d (pid %d)¥n",__func__,imajor(inode),iminor(inode),current->pid);/*major、minor番号を取得するためにinodeから番号情報を取得する,ここでcurrent->pidでopenを呼び出したユーザープロセスの情報を確保している*/

	p=kmalloc(sizeof(struct devone_data),GFP_KERNEL); 			/*開くファイル分のメモリを確保している*/
	if(p==NULL){
		printk("%s: Not memory¥n",__func__);
		return -ENOMEM;
	}

	p->val=0xff;												/*valを指定している*/
	rwlock_init(&p->lock);										/*lock*/


	file->private_data=p;										/*開くファイルのメモリを指定している(事前に開くファイル分のメモリを確保していた最初のエントリポイントをここで指定している)*/

	return 0; /*success!*/

}

/*close システムコール*/
static int devone_close(struct inode *inode,struct file *file){

	printk("%s:major %d minor %d (pid %d)¥n",__func__,imajor(inode),iminor(inode),current->pid);/*major、minor番号を取得するためにinodeから番号情報を取得する*/

	if(file->private_data){
		kfree(file->private_data);		/*先程確保したメモリをここで開放している*/
		file->private_data=NULL;
	}

	return 0; /*success!*/
}


/*ドライバに対して、システムコールを発行できるようにシステムコールハンドラをここで登録している(.openのシステムコールが呼ばれたらdevone_openの関数が行われるというのをテーブルとして登録しておく)*/
struct file_operations devone_fops={
	.open=devone_open,
	.release=devone_close,
	.read=devone_read,
	.write=devone_write,
};

/*ドライバをロードしたときに呼ばれる関数*/
static int devone_init(void){

	dev_t dev=MKDEV(devone_major,0);		/*操作するデバイスがどれか(メジャー番号はいくつかなどの情報)をdevに保存している*/
	int alloc_ret=0;
	int major;
	int cdev_err=0;
	
	/* 1. 空いているメジャー番号を確保する */
	alloc_ret=alloc_chrdev_region(&dev,1,devone_devs,DRIVER_NAME); /*ここで空いているメジャー番号はdevに登録される*/
	if(alloc_ret){
		goto error;
	}
	 /* 2. 取得したdev( = メジャー番号 + マイナー番号)からメジャー番号を取得して保持しておく */
	devone_major=MAJOR(dev);
	
	 /* 3. cdev構造体の初期化とシステムコールハンドラテーブルの登録 */
	cdev_init(&devone_cdev,&devone_fops);			/*ここでドライバで使うシステムコールを登録しておく。(fopsがシステムコールハンドラである)。cdev構造体はドライバの情報のようなもの(file headerのようなもの)*/
	devone_cdev.owner=THIS_MODULE;					/*これは必要な作業。特に気にしなくてもok*/

	/* 4. このデバイスドライバ(cdev)をカーネルに登録する */
	cdev_err=cdev_add(&devone_cdev,dev,devone_devs);		/*ドライバをカーネルへ登録する*/
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
