#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEV_SIZE 0x2000

unsigned char dev_mem[DEV_SIZE];
struct cdev cdev; 
dev_t devno;

bool Device_Open;

/*
 * Declarations of my own functions
 * The prototypes can be found at linux/fs.h
 */
int 	open_cl(struct inode *, struct file *);
int 	release_cl(struct inode *, struct file *);
ssize_t read_cl(struct file *, char __user *, size_t, loff_t *);
ssize_t write_cl(struct file *, const char __user *, size_t, loff_t *);
loff_t 	llseek_cl(struct file *, loff_t, int);
long 	ioctl_cl (struct file *, unsigned int, unsigned long);

/*
 * Associating function entry points
 */
static const struct file_operations fops_cl = {
  .owner = THIS_MODULE,
  .open = open_cl,
  .release = release_cl,
  .read = read_cl,
  .write = write_cl,
  .llseek = llseek_cl,
  .unlocked_ioctl = ioctl_cl,
};

/*
 * To open the device
 * If the open has been opened, return -EBUSY
 * otherwise return 0
 */
int open_cl(struct inode *inode, struct file *filp) {
	if(Device_Open) {
		printk("The device is fail to open!\n");
    	return -EBUSY;	// in case the device being opened twice at a moment
	}
    Device_Open++;
    filp->private_data = dev_mem;
    printk("The device is opened!\n");
    return 0;
}

/*
 * To release the device
 */
int release_cl(struct inode *inode, struct file *filp) {
	Device_Open--;
	printk("The device has been released!\n");
	return 0;
}

/*
 * To read from the device
 */
ssize_t read_cl(struct file *filp, char __user *buffer, size_t size, loff_t *offset) {
	unsigned long p = *offset;	// offset in device memory
	unsigned long count = (size - DEV_SIZE * sizeof(unsigned char) + p) ? DEV_SIZE * sizeof(unsigned char) - p : size;
	unsigned char *mem_addr = filp->private_data;	// get the memory base address of this device

	if (p >= DEV_SIZE * sizeof(unsigned char)) {	// Determine whether the reading position is valid
		return 0;
	}
	// read data to user's memory
	if (copy_to_user(buffer, mem_addr+p, count)){
		printk("Fail to read from the device!\n");
		return -EFAULT;	// fail to read
	}
	else{
		*offset += count;
	}
	printk("%lu byte(s) have read from the device\n", count);
	return count;
}

/*
 * To write to the device
 */
ssize_t write_cl(struct file *filp, const char __user *buffer, size_t size, loff_t *offset) {
	unsigned long p = *offset;	// offset in device memory
	unsigned long count = (size - DEV_SIZE * sizeof(unsigned char) + p) ? DEV_SIZE * sizeof(unsigned char) - p : size;
	unsigned char *mem_addr = filp->private_data;	// get the memory base address of this device

	if (p >= DEV_SIZE * sizeof(unsigned char) ){	// Determine whether the reading position is valid
		return 0;
	}
	// write data to device's memory
	if (copy_from_user(buffer, mem_addr+p, count)){
		printk("Fail to write to the device!\n");
		return -EFAULT;	// fail to write
	}
	else{
		*offset += count;
	}
	printk("%lu byte(s) have written to the device!\n", count);
	return count;
}

/*
 * To position file
 */
loff_t llseek_cl(struct file *filp, loff_t offset, int whence) {
	switch(whence){
		case SEEK_SET:	// at the head of file
			if (offset < 0 || offset >= DEV_SIZE)
				return -EINVAL;
			else {
				filp->f_pos = offset;
				return offset;
			}
		case SEEK_CUR:	// at the current position
			if ((filp->f_pos + offset) >= DEV_SIZE || (filp->f_pos + offset) < 0)
				return -EINVAL;
			else {
				filp->f_pos = filp->f_pos + offset;
				return filp->f_pos + offset;
			}
		default:
			return -EINVAL;
	}
}

/*
 * To clear the memory
 */
long ioctl_cl(struct file *filp, unsigned int cmd, unsigned long arg) {
	if (cmd == 1) {
		memset(filp->private_data, 0, DEV_SIZE);
		printk("The device has been cleared!\n");
		return 0;
	}
	return -EINVAL;
}

static int __init init_dev_cl(void){
	/*初始化cdev结构*/
	cdev_init(&cdev, &fops_cl);

 	/* 注册字符设备 */
	alloc_chrdev_region(&devno, 0, 1, "cl's device");
	cdev_add(&cdev, devno, 1);
	return 0;
}

	/*模块卸载函数*/
static void __exit exit_dev_cl(void){
	  cdev_del(&cdev);   /*注销设备*/
	  unregister_chrdev_region(devno, 1); /*释放设备号*/
}


module_init(init_dev_cl);
module_exit(exit_dev_cl);
MODULE_LICENSE("GPL");
