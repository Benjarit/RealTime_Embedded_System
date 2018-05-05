
#ifndef MODULE
#define MODULE
#endif
#ifndef __KERNEL__
#define __KERNEL__
#endif

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>    // for kthreads
#include <linux/sched.h>    // for task_struct
#include <linux/time.h>        // for using jiffies
#include <linux/timer.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/string.h>

#define MSG_SIZE 50
#define CDEV_NAME "Final"	// "YourDevName"

MODULE_LICENSE("GPL");
 
static int major; 
static char msg[MSG_SIZE];

unsigned long *BPTR;

// structure for the kthread.
static struct task_struct *kthread1;
unsigned long *EVENT, *PUD, *PUD_CLK, *EDGE;


int mydev_id;	// variable needed to identify the handler
int button1, button2;

static irqreturn_t button_isr(int irq, void *dev_id)
{
	// In general, you want to disable the interrupt while handling it.
	disable_irq_nosync(79);

    unsigned long tmp = *EVENT & 0x1F0000;
    if(tmp == 0x10000) //0000 0001 0000 0000 0000 0000
    {
        button1 = 1;   
        printk("button 1 pushed\n");
    }
    else if(tmp == 0x20000) //0000 0010 0000 0000 0000 0000
    {
        button2 = 1;
        printk("button 2 pushed\n");

    }

    *EVENT = *EVENT | 0x1F0000;//clear it
	printk("Interrupt handled\n");	
	enable_irq(79);		// re-enable interrupt
	
    return IRQ_HANDLED;
}

// Function to be associated with the kthread; what the kthread executes.
int kthread_fn(void *ptr)
{
	printk("Before loop\n");
	
	// The ktrhead does not need to run forever. It can execute something
	// and then leave.
	while(1)
	{
		// In an infinite loop, you should check if the kthread_stop
		// function has been called (e.g. in clean up module). If so,
		// the kthread should exit. If this is not done, the thread
		// will persist even after removing the module.
		if(kthread_should_stop()) {
			do_exit(0);
		}
				
		// comment out if your loop is going "fast". You don't want to
		// printk too often. Sporadically or every second or so, it's okay.
	}
	
	return 0;
}


// Function called when the user space program reads the character device.
static ssize_t device_read(struct file *filp, char __user *buffer, size_t len, loff_t *offset)
{
    msg[0]= "\0";

    if (button1 == 1)
    {
        strcpy(msg, "button1");
        button1 = 0;
    }
    if (button2 == 1)
    {
        strcpy(msg, "button2");
        button2 = 0;
    }
	ssize_t dummy = copy_to_user(buffer, msg, len);
        printk("%s\n", msg); 

	return len;
}

// Function called when the user space program writes to the Character Device.
static ssize_t device_write(struct file *filp, const char __user *buff, size_t len, loff_t *off)
{
	ssize_t dummy;
	
	return len;		// the number of bytes that were written to the Character Device.
}

// structure needed when registering the Character Device. Members are the callback
// functions when the device is read from or written to.
static struct file_operations fops = {
	.read = device_read, 
	.write = device_write,
};

int cdev_module_init(void)
{
	// register the Characted Device and obtain the major (assigned by the system)
	major = register_chrdev(0, CDEV_NAME, &fops);
	if (major < 0) {
     		printk("Registering the character device failed with %d\n", major);
	     	return major;
	}
	printk("Lab6_cdev_kmod example, assigned major: %d\n", major);
	printk("Create Char Device (node) with: sudo mknod /dev/%s c %d 0\n", CDEV_NAME, major);

    
       
    char kthread_name[11]="my_kthread";	// try running  ps -ef | grep my_kthread
										// when the thread is active.
	printk("In init module\n");

	int dummy = 0;
    BPTR = (unsigned long *) ioremap(0x3F200000, 4096);

    EVENT = BPTR + 0x40/4; //even

    //pull-down setting
    PUD = BPTR + 0x94/4;//point at gppud register
    *PUD = *PUD | 0x155; //pud as pull down control
   
    udelay(150); //wait

    PUD_CLK = BPTR + 0x98/4;//point at gppudclk0
    *PUD_CLK = *PUD_CLK | 0x1F0000;//asynchronous falling EDGE
    
    udelay(150); //wait
    
    *PUD &= 0xFFFFFEAA;//off off 
	*PUD_CLK &= 0xFFE0FFFF;//off now

    // EnableRising EDGE detection for all buttons.
    EDGE = BPTR + 0x4C/4; //point at rising EDGE
    *EDGE = *EDGE | 0x1F0000;
	

    button1 = 0;
    button2 = 0;
	// Request the interrupt / attach handler (line 79, doesn't match the manual...)
	// The third argument string can be different (you give the name)
	dummy = request_irq(79, button_isr, IRQF_SHARED, "Button_handler", &mydev_id);

    kthread1 = kthread_create(kthread_fn, NULL, kthread_name);
	
    if((kthread1))	// true if kthread creation is successful
    {
        printk("Inside if\n");
		// kthread is dormant after creation. Needs to be woken up
        wake_up_process(kthread1);
    }
 	return 0;
}

void cdev_module_exit(void)
{
	// Once unregistered, the Character Device won't be able to be accessed,
	// even if the file /dev/YourDevName still exists. Give that a try...
	unregister_chrdev(major, CDEV_NAME);
	printk("Char Device /dev/%s unregistered.\n", CDEV_NAME);

    free_irq(79, &mydev_id);
	printk("Button Detection disabled.\n");

    int ret;
	// the following doesn't actually stop the thread, but signals that
	// the thread should stop itself (with do_exit above).
	// kthread should not be called if the thread has already stopped.
	ret = kthread_stop(kthread1);
								
	if(!ret)
		printk("Kthread stopped\n");
}  

module_init(cdev_module_init);
module_exit(cdev_module_exit);
