#include<linux/init.h>
#include<linux/module.h>
#include<linux/types.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/sched.h>
#include<asm/current.h>
#include<linux/device.h>
#include<asm/uaccess.h>

MODULE_LICENSE("Dual BSD/GPL");

#define DRIVER_NAME	"MyDevice_NAME"


/*device driver minor number*/
static const unsigned int MINOR_BASE=0;
static const unsigned int MINOR_NUM=2;

/*dynamic driver major number*/
static unsigned int mydevice_major;

/*charcter device */
static struct cdev mydevice_cdev;

/*open function*/
static int mydevice_open(struct inode *inode,struct file *file){
	printk("myDevice_open\n");
	return 0;
}

/*close*/
static int mydevice_close(struct inode *inode,struct file *file){
	printk("Mydevice_close\n");
	return 0;
}

/*read*/
static ssize_t mydevice_read(struct file *filp, char __user *buf,size_t count,loff_t *f_pos){
	printk("myDevice_read\n");
	buf[0]='V';
	return 1;
}

/*write*/
static ssize_t mydevice_write(struct file *filp,const char __user *buf,size_t count,loff_t *f_pos){
	printk("mydevice_write\n");
	return 1;
}

/*system call*/
struct file_operations s_myDevice_fops={
	.open=mydevice_open,
	.release=mydevice_close,
	.read=mydevice_read,
	.write=mydevice_write,
};

static int myDevice_init(void){
	printk("mydevice_init\n");

	int alloc_ret=0;
	int cdev_err=0;
	dev_t dev;

	/*empty major number and minor number(MKDEV)*/
	alloc_ret=alloc_chrdev_region(&dev,MINOR_BASE,MINOR_NUM,DRIVER_NAME);
	if(alloc_ret!=0){
		printk(KERN_ERR "alloc_chrdev_region=%d\n",alloc_ret);
		return -1;		
	}

	/*get major number from dev*/
	mydevice_major=MAJOR(dev);
	
	/*cdev init and register systemcall*/
	cdev_init(&mydevice_cdev,&s_myDevice_fops);
	mydevice_cdev.owner=THIS_MODULE;

	/*register cdev to kernel */
	cdev_err=cdev_add(&mydevice_cdev,dev,MINOR_NUM);
	if(cdev_err!=0){
		printk(KERN_ERR "cdev_add=%d\n",alloc_ret);
		unregister_chrdev_region(dev,MINOR_NUM);
		return -1;
	}

	return 0;
}

static void myDevice_exit(void){
	printk("myDevice_exit\n");
	
	dev_t dev=MKDEV(mydevice_major,MINOR_BASE);
	
	cdev_del(&mydevice_cdev);
	
	unregister_chrdev_region(dev,MINOR_NUM);
}

module_init(myDevice_init);
module_exit(myDevice_exit);
