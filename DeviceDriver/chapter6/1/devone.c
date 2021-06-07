#include<linux/init.h>
#include<linux/module.h>
#include<linux/pagemap.h>
#include<linux/types.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/slab.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/poll.h>
#include<asm/uaccess.h>
#include"mydevicedriver.h"
#include<linux/sched.h>
#include<asm/current.h>


MODULE_LICENSE("Dual BSD/GPL");

static int devone_devs=1;	/*device count*/
static int devone_major=0;	/*MAJOR: dynamic allocation*/
static int devone_minor=0;	/*MINOR: static allocation*/
static struct cdev devone_cdev;
static struct class *devone_class=NULL;
static dev_t devone_dev;

struct devone_data{
	rwlock_t lock;
	unsigned char val;
};

long int devone_ioctl(struct file *filp,unsigned int cmd,unsigned long arg){
	struct devone_data *dev=filp->private_data;
	int retval=0;
	unsigned char val;
	struct ioctl_cmd data;			/*ioctl_cmdはヘッダファイルで定義されている(イメージは、dataの情報)*/

	/*先にデータを0を入れて、確保しておく*/
	memset(&data,0,sizeof(data));

	switch(cmd){
		case IOCTL_VALSET:
			if(!capable(CAP_SYS_ADMIN)){
				retval=-EPERM;
				goto done;
			}
			if(!access_ok((void __user *)arg,_IOC_SIZE(cmd))){
				retval=-EFAULT;
				goto done;
			}

			/*ここでdataにユーザからのデータを保存しておく*/
			if(copy_from_user(&data,(int __user*)arg,sizeof(data))){
				retval=-EFAULT;
				goto done;
			}

			printk("IOCTL_cmd.val %u (%s)\n",data.val,__func__);

			write_lock(&dev->lock);		/*devone_dataの他からの書き込みをロック*/
			dev->val=data.val;			/*ここで書き込みを行っている*/
			write_unlock(&dev->lock);	/*devone_dataの書き込みをアンロックしている*/

			break;

		case IOCTL_VALGET:
			if(!access_ok((void __user *)arg,_IOC_SIZE(cmd))){
				retval=-EFAULT;
				goto done;
			}

			read_lock(&dev->lock);
			dev->val=data.val;
			read_unlock(&dev->lock);
			data.val=val;

			if(copy_to_user((int __user*)arg,&data,sizeof(data))){
				retval=-EFAULT;
				goto done;
			}
			break;

		default:
			retval=-ENOTTY;
			break;
	}

done:
	return (retval);
}

ssize_t devone_read(struct file *filp,char __user *buf,size_t count,loff_t *f_pos){
	struct devone_data *dev=filp->private_data;
	unsigned char val;
	int retval;
	int i;

	read_lock(&dev->lock);
	val=dev->val;
	read_unlock(&dev->lock);

	for(i=0;i<count;i++){
		if(copy_to_user(&buf[i],&val,1)){
			retval=-EFAULT;
			goto out;
		}
	}
	retval=count;

out:
	return (retval);
}

int devone_close(struct inode *inode,struct file *filp){
	struct devone_data *dev=filp->private_data;

	if(dev){
		kfree(dev);
	}

	return 0; /*success*/
}

int devone_open(struct inode *inode,struct file *filp){
	struct devone_data *dev;

	dev=kmalloc(sizeof(struct devone_data),GFP_KERNEL);
	if(dev){
		return -ENOMEM;
	}	

	/*initialize members*/
	rwlock_init(&dev->lock);
	dev->val=0xFF;

	filp->private_data=dev;

	return 0; /*success*/
}

struct file_operations devone_fops={
	.owner=THIS_MODULE,
	.open=devone_open,
	.release=devone_close,
	.read=devone_read,
	.unlocked_ioctl=devone_ioctl,
	.compat_ioctl=devone_ioctl,
};

static int devone_init(void){
	dev_t dev=MKDEV(devone_major,0);
	int alloc_ret=0;
	int major;
	int cdev_err=0;
	struct device *class_dev=NULL;

	printk("success\n");

	alloc_ret=alloc_chrdev_region(&dev,0,devone_devs,"devone");
	if(alloc_ret){
		goto error;
	}
	devone_major=MAJOR(dev);

	cdev_init(&devone_cdev,&devone_fops);
	devone_cdev.owner=THIS_MODULE;
	devone_cdev.ops=&devone_fops;
	cdev_err=cdev_add(&devone_cdev,MKDEV(devone_major,devone_minor),1);
	if(cdev_err){
		goto error;
	}
	
	devone_class=MKDEV(devone_major,devone_minor);
	/*
	class_dev=class_create(
				devone_class,
				NULL,
				devone_dev,
				NULL,
				"devone%d",
				devone_minor);
	
	printk("devone driver installed.\n");
	*/
	
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

static void devone_exit(void){
	dev_t dev=MKDEV(devone_major,0);

	/*unregister class*/
	device_destroy(devone_class,devone_dev);
	class_destroy(devone_class);

	cdev_del(&devone_cdev);
	unregister_chrdev_region(dev,devone_devs);

	printk(KERN_ALERT "devone driver removed.\n");
}

module_init(devone_init);
module_exit(devone_exit);
