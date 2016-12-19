/*
		Dummy Driver for LinkedList
		//@ Modified by Guarav & Mario @//
*/
#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/random.h>//@ For random number generation 
#include <linux/slab.h>  //@ For kmalloc memory allocation
#include "./linked_list.h"//@ Required structures and functions 

#define DUMMY_MAJOR_NUMBER 250
#define DUMMY_DEVICE_NAME "DUMMY_DEVICE"

int dummy_open	   (struct inode *, struct file *);
int dummy_release  (struct inode *, struct file *);
ssize_t dummy_read (struct file *, char *, size_t, loff_t *);
ssize_t dummy_write(struct file *, const char *, size_t, loff_t *);
long dummy_ioctl   (struct file *, unsigned int, unsigned long);

/* file operation structure */
struct file_operations dummy_fops = {
	open:			dummy_open,
	read:			dummy_read,
	write: 			dummy_write,
	release: 		dummy_release, 
	unlocked_ioctl: dummy_ioctl,
};

char devicename[20];
struct semaphore mutex;//@ Controls access to the linked list blocks
struct semaphore full; //@ Prevents raders' access if the list is empty
struct semaphore empty;//@ Prevents writers' access if list is full
static dev_t device_num;   // For device minor number
static struct cdev my_cdev;//@ Kernel structure to handle the new device
static struct class *cl;   //@ Pointer to class of the device? 

static int __init dummy_init(void)
{
	printk("Dummy Driver : Module Init\n");
	strcpy(devicename, DUMMY_DEVICE_NAME);

	// Allocating device region 
	if(alloc_chrdev_region(&device_num, 0, 1, devicename)){
		return -1;
	}
	if((cl = class_create(THIS_MODULE, "chardrv" )) == NULL){
		unregister_chrdev_region(device_num, 1);
		return -1;
	}
	// Device Create == mknod /dev/DUMMY_DEVICE 
	if(device_create(cl, NULL, device_num, NULL, devicename) == NULL){
		class_destroy(cl);
		unregister_chrdev_region(device_num, 1);
		return -1;
	}
	// Device Init 
	cdev_init(&my_cdev, &dummy_fops);
	if( cdev_add(&my_cdev, device_num, 1) == -1 ){
		device_destroy(cl, device_num);
		class_destroy(cl);
		unregister_chrdev_region(device_num, 1);
	}
	
	/*@ Initialize the semaphores @*/
	sema_init(&mutex,1);
	sema_init(&empty,LIST_SIZE);
	sema_init(&full ,0);
	
	dummy_create();//@ Create the linked list required structures
	return 0;
}

static void __exit dummy_exit(void)
{
	printk("Dummy Driver : Clean Up Module\n");
	dummy_destroy();//@ Free the linked list memory 
	cdev_del(&my_cdev);
	device_destroy(cl, device_num);
	class_destroy(cl);
	unregister_chrdev_region(MKDEV(DUMMY_MAJOR_NUMBER,0),128);
}

ssize_t dummy_read(struct file *file, char *buffer, size_t length, loff_t *offset)
{ 
	int i = 0;
	int rand_num;
	int *dummy_buf = (int *)kmalloc(sizeof(int),GFP_KERNEL); 
	
	block *read_block = list.head; //@ Initialize the block pointer
	
	/*@ Load read value requested from parameter lenght @*/
	rand_num = length;//@ length is now used to request values
	
	down(&full);//@ Sleep if there is zero full blocks
	down(&mutex);//@ Enter criticla region
		
		/*@ Loop until the hold list has been searched @*/
		while(i < list.cnt){	
			
			if(read_block->value == rand_num){
				
				*dummy_buf = read_block->value;//@ Copy the value to the dummy	
				read_block->value = INIT_VAL;//@ Delete the value from the block
				printk("@ Read HIT\n");
				goto HIT;//@  The value was found, copy to user the value	
			}
			read_block = read_block->next;
			i++;//@ Save the iteartion number
		}
					
		/*@ Else the value was not located, copy -1 to user @*/
		*dummy_buf = INIT_VAL;
		printk(" @ Read MISS\n");
HIT:		
		if(copy_to_user(buffer, dummy_buf, sizeof(int))){  
			kfree(dummy_buf);
			return -EFAULT;
		}
		
	i = (i>=LIST_SIZE)?-1:i;//@ Block number = -1 if value not found	
	printk("Dummy Driver : Read Call (block: %d, value: %d)\n",i,*dummy_buf);
	
	up(&mutex);//@ Exit critical region
	up(&empty);//@ Signal another block has been emptied
	
	kfree(dummy_buf);
	return sizeof(int);
}

ssize_t dummy_write(struct file *file, const char *buffer, size_t length, loff_t *offset)
{
	int i = 0;
	int rand_num;
	int *dummy_buf = (int *)kmalloc(sizeof(int),GFP_KERNEL);
	
	block *write_block = list.head;//@ Initialize the block pointer 
	
	if(copy_from_user(dummy_buf, buffer, sizeof(int))){//@ Copy user's data to dummy 
		kfree(dummy_buf);
		return -EFAULT;
	}
	
	/*@ Create random number ( 0 ~ LIST_SIZE-1 )@*/
	get_random_bytes(&rand_num,sizeof(int));
	rand_num = abs(rand_num) % LIST_SIZE;
		
	down(&empty);//@ Sleep if there is zero empty blocks
	down(&mutex);//@ Enter critical region
	
		while( i < rand_num){
			write_block = write_block->next;//@ Trasverse the list until
			i++;                            //@ the random block is pointed at
		}
		
		write_block->value=*dummy_buf;
	
	printk("Dummy Driver : Write Call (block: %d, value: %d)\n",i,*dummy_buf);
	
	up(&mutex);//@ Exit critical region
	up(&full);//@ Signal another block has been filled
	
	kfree(dummy_buf);
	return sizeof(int);
}

int dummy_open(struct inode *inode, struct file *file)
{
	printk("Dummy Driver : Open Call\n");
	return 0;
}

int dummy_release(struct inode *inode, struct file *file)
{
	printk("Dummy Driver : Release Call\n");
	return 0;
}

long dummy_ioctl(struct file *file, unsigned int cmd, unsigned long argument)
{
	printk("ioctl (what is this?)");
	return 0;
}

module_init(dummy_init);
module_exit(dummy_exit);

MODULE_DESCRIPTION("Dummy_LinkedList_Driver");
MODULE_LICENSE("GPL");
