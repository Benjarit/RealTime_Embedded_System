/* Lab6_cdev_kmod.c
 * ECE4220/7220
 * Based on code from: https://github.com/blue119/kernel_user_space_interfaces_example/blob/master/cdev.c
 * Modified and commented by: Luis Alberto Rivera

 You can compile the module using the Makefile provided. Just add
 obj-m += Lab6_cdev_kmod.o

 This Kernel module prints its "MajorNumber" to the system log. The "MinorNumber"
 can be chosen to be 0.

 -----------------------------------------------------------------------------------
 Broadly speaking: The Major Number refers to a type of device/driver, and the
 Minor Number specifies a particular device of that type or sometimes the operation
 mode of that device type. On a terminal, try:
    ls -l /dev/
 You'll see a list of devices, with two numbers (among other pieces of info). For
 example, you'll see tty0, tty1, etc., which represent the terminals. All those have
 the same Major number, but they will have different Minor numbers: 0, 1, etc.
 -----------------------------------------------------------------------------------

 After installing the module,

 1) Check the MajorNumber using dmesg

 2) You can then create a Character device using the MajorNumber returned:
	  sudo mknod /dev/YourDevName c MajorNumber 0
    You need to create the device every time the Pi is rebooted.

 3) Change writing permissions, so that everybody can write to the Character Device:
	  sudo chmod go+w /dev/YourDevName
    Reading permissions should be enabled by default. You can check using
      ls -l /dev/YourDevName
    You should see: crw-rw-rw-

 After the steps above, you will be able to use the Character Device.
 If you uninstall your module, you won't be able to access your Character Device.
 If you install it again (without having shutdown the Pi), you don't need to
 create the device again --steps 2 and 3--, unless you manually delete it.

 Note: In this implementation, there is no buffer associated to writing to the
 Character Device. Every new string written to it will overwrite the previous one.
*/

#ifndef MODULE
#define MODULE
#endif
#ifndef __KERNEL__
#define __KERNEL__
#endif

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/kthread.h>	// for kthreads
#include <linux/sched.h>	// for task_struct
#include <linux/time.h>		// for using jiffies
#include <linux/fs.h>
#include <asm/uaccess.h>


#define MSG_SIZE 40
#define CDEV_NAME "Benjarit"	// "YourDevName"

MODULE_LICENSE("GPL");


int mydev_id;	// variable needed to identify the handler

unsigned long *GPFSEL, *GPSET, *GPCLR, * GPEDS, * GPAREN, * GPPUD, * GPPUDCLK;
static int major;
static char msg[MSG_SIZE], send_msg[MSG_SIZE];
int sem = 0;
unsigned long period = 1272; //Interval for HRTIMER.
static struct task_struct *kthread1;
// Function called when the user space program reads the character device.
// Some arguments not used here.
// buffer: data will be placed there, so it can go to user space
// The global variable msg is used. Whatever is in that string will be sent to userspace.
// Notice that the variable may be changed anywhere in the module...
static ssize_t device_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset)
{
	// Whatever is in msg will be placed into buffer, which will be copied into user space
	if(sem){
		printk("Send this note to User space: %s\n", send_msg);
		ssize_t dummy = copy_to_user(buffer, send_msg, length);	// dummy will be 0 if successful
		printk("Send this note to User space: %s\n", send_msg);
		// msg should be protected (e.g. semaphore). Not implemented here, but you can add it.
		send_msg[0] = '\0';	// "Clear" the message, in case the device is read again.
					// This way, the same message will not be read twice.
					// Also convenient for checking if there is nothing new, in user space.
		sem--;
	}

	return length;
}


// Function called when the user space program writes to the Character Device.
// Some arguments not used here.
// buff: data that was written to the Character Device will be there, so it can be used
//       in Kernel space.
// In this example, the data is placed in the same global variable msg used above.
// That is not needed. You could place the data coming from user space in a different
// string, and use it as needed...
static ssize_t device_write(struct file *filp, const char __user *buff, size_t len, loff_t *off)
{
	ssize_t dummy;

	if(len > MSG_SIZE)
		return -EINVAL;

	// unsigned long copy_from_user(void *to, const void __user *from, unsigned long n);
	dummy = copy_from_user(msg, buff, len);	// Transfers the data from user space to kernel space
	if(len == MSG_SIZE)
		msg[len-1] = '\0';	// will ignore the last character received.
	else
		msg[len] = '\0';

	// You may want to remove the following printk in your final version.
	printk("Message from user space: %s\n", msg);

	if(msg[1] == 'A'){
		period = 550;
		printk("Period is set to: %lu\n", period);
	}else if(msg[1] == 'B'){
		period = 435;
		printk("Period is set to: %lu\n", period);
	}else if(msg[1] == 'C'){
		period = 435;
		printk("Period is set to: %lu\n", period);
	}else if(msg[1] == 'D'){
		period = 310;
		printk("Period is set to: %lu\n", period);
	}else if(msg[1] == 'E'){
		period = 250;
		printk("Period is set to: %lu\n", period);
	}

	return len;		// the number of bytes that were written to the Character Device.
}

// structure needed when registering the Character Device. Members are the callback
// functions when the device is read from or written to.
static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
};

static irqreturn_t button_isr(int irq, void *dev_id)
{
	// In general, you want to disable the interrupt while handling it.
	disable_irq_nosync(79);

	// This same handler will be called regardless of what button was pushed,
	// assuming that they were properly configured.
	// How can you determine which button was the one actually pushed?

	// DO STUFF (whatever you need to do, based on the button that was pushed)
	if(*GPEDS & 0x10000){
		printk("PB1 was pressed\n");
		period = 550;
		strcpy(send_msg,"@A");
		printk("Period is senttt: %s\n", send_msg);
		sem++;
	}else if(*GPEDS & 0x20000){
		printk("PB2 was pressed\n");
		period = 435;
		strcpy(send_msg,"@B");
		printk("Period is senttt: %s\n", send_msg);
		sem++;
	}else if(*GPEDS & 0x40000){
		printk("PB3 was pressed\n");
		period = 370;
		strcpy(send_msg,"@C");
		printk("Period is senttt: %s\n", send_msg);
		sem++;
	}else if(*GPEDS & 0x80000){
		printk("PB4 was pressed\n");
		period = 310;
		strcpy(send_msg,"@D");
		printk("Period is senttt: %s\n", send_msg);
		sem++;
	}else if(*GPEDS & 0x100000){
		printk("PB5 was pressed\n");
		period = 250;
		strcpy(send_msg,"@E");
		printk("Period is senttt: %s\n", send_msg);
		sem++;
	}
	// IMPORTANT: Clear the Event Detect status register before leaving.
	// The bit is cleared by writing a “1” to the relevant bit.
	*GPEDS = 0xFFFFFFFF;
	printk("Interrupt handled\n");
	enable_irq(79);		// re-enable interrupt

    return IRQ_HANDLED;
}
int kthread_fn(void *ptr){
  unsigned long j0, j1;
  int toggle = 0;

  j0 = jiffies; //Number of clock ticks since the system started.
  j1 = j0 + 2*HZ; //Not sure what this one is

  while(time_before(jiffies, j1)){
    schedule();
  }

  while(1){
    //Check if the thread should be stopped
    if(kthread_should_stop()) {
			do_exit(0);
	}
//    usleep_range(period, period); //Not sure how this will work with a tight range
//    //usleep_range(1000000, 1100000); //Test values
//    if((toggle % 2) == 0)
//	{
//		*GPSET |= 0x40;
//		toggle++;
//	}
//	//Clear the pin
//	else
//	{
//		*GPCLR |= 0x40;
//		toggle--;
//	}
    *GPSET |= 0x40;
    udelay(period);
    *GPCLR |= 0x40;
    udelay(period);
  }

  return(0);
}
int cdev_module_init(void)
{

	char kthread_name[11]="my_kthread";
	int dummy = 0;
	GPFSEL = (unsigned long *)ioremap(0x3F200000,4096);
	// Set Speaker as output
	*GPFSEL = *GPFSEL | 0x40000;
	// Set register
	GPSET = GPFSEL + (0x1c/4);
	// Clear register
	GPCLR = GPFSEL + (0x28/4);
	// Event Detect Status
	GPEDS = GPFSEL + (0x40/4);
	// GPIO Async
	GPAREN = GPFSEL + (0x4c/4);
	// Pullup/Pulldown enable
	GPPUD = GPFSEL + (0x94/4);
	// Clock for Pullup/Pulldown configuration
	GPPUDCLK = GPFSEL + (0x98/4);



	// Move over one register
	GPFSEL = GPFSEL + (0x04/4);
	// PB1, PB2, PB3, PB4
	*GPFSEL = *GPFSEL & 0xC003FFFF;
	// Move over one moore register
	GPFSEL = GPFSEL + (0x04/4);
	// PB5
	*GPFSEL = *GPFSEL & 0xFFFFFFF8;

	// Map GPIO registers
	// Remember to map the base address (beginning of a memory page)
	// Then you can offset to the addresses of the other registers
	*GPPUD |= 0x1;	//0b01
	udelay(100);
	*GPPUDCLK |= 0x1F0000;
	udelay(100);
	*GPPUD &= 0x0;
	udelay(100);
	*GPPUDCLK &= 0x0;
	// Enable (Async) Rising Edge detection for all 5 GPIO ports.
	*GPAREN |= 0x1F0000;
	// Don't forget to configure all ports connected to the push buttons as inputs.

	// You need to configure the pull-downs for all those ports. There is
	// a specific sequence that needs to be followed to configure those pull-downs.
	// The sequence is described on page 101 of the BCM2835-ARM-Peripherals manual.
	// You can use  udelay(100);  for those 150 cycles mentioned in the manual.
	// It's not exactly 150 cycles, but it gives the necessary delay.
	// WiringPi makes it a lot simpler in user space, but it's good to know
	// how to do this "by hand".

	// Enable (Async) Rising Edge detection for all 5 GPIO ports.

	// Request the interrupt / attach handler (line 79, doesn't match the manual...)
	// The third argument string can be different (you give the name)
	dummy = request_irq(79, button_isr, IRQF_SHARED, "Button_handler", &mydev_id);
	kthread1 = kthread_create(kthread_fn, NULL, kthread_name);

	if((kthread1)){
	  wake_up_process(kthread1);
	}
	printk("Button Detection enabled.\n");

	// register the Characted Device and obtain the major (assigned by the system)
	major = register_chrdev(0, CDEV_NAME, &fops);
	if (major < 0) {
     		printk("Registering the character device failed with %d\n", major);
	     	return major;
	}
	printk("Lab6_cdev_kmod example, assigned major: %d\n", major);
	printk("Create Char Device (node) with: sudo mknod /dev/%s c %d 0\n", CDEV_NAME, major);
 	return 0;
}

void cdev_module_exit(void)
{

	// Good idea to clear the Event Detect status register here, just in case.
	// The bit is cleared by writing a “1” to the relevant bit.
	*GPEDS |= 0xFFFFFFFF;
	// Disable (Async) Rising Edge detection for all 5 GPIO ports.
	*GPAREN |= 0x0;
	// Remove the interrupt handler; you need to provide the same identifier
    free_irq(79, &mydev_id);
	if(!kthread_stop(kthread1)){
      printk("Musical_keyboard Kthread has stopped\n");
    }
	printk("Button Detection disabled.\n");

	// Once unregistered, the Character Device won't be able to be accessed,
	// even if the file /dev/YourDevName still exists. Give that a try...
	unregister_chrdev(major, CDEV_NAME);
	printk("Char Device /dev/%s unregistered.\n", CDEV_NAME);
}

module_init(cdev_module_init);
module_exit(cdev_module_exit);

