/* Based on example from: http://tuxthink.blogspot.com/2011/02/kernel-thread-creation-1.html
   Modified and commented by: Luis Rivera			
   
   Compile using the Makefile
*/

#ifndef MODULE
#define MODULE
#endif
#ifndef __KERNEL__
#define __KERNEL__
#endif
   
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>	// for kthreads
#include <linux/sched.h>	// for task_struct
#include <linux/time.h>		// for using jiffies 
#include <linux/timer.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

// structure for the kthread.
static struct task_struct *kthread1;

// Function to be associated with the kthread; what the kthread executes.
int kthread_fn(void *ptr)
{
	unsigned long j0, j1;
	int count = 0;

	printk("In kthread1\n");
	j0 = jiffies;		// number of clock ticks since system started;
						// current "time" in jiffies
	j1 = j0 + 10*HZ;	// HZ is the number of ticks per second, that is
						// 1 HZ is 1 second in jiffies
	
	while(time_before(jiffies, j1))	// true when current "time" is less than j1
        schedule();		// voluntarily tell the scheduler that it can schedule
						// some other process
	
	printk("Before loop\n");
	
	// The ktrhead does not need to run forever. It can execute something
	// and then leave.
	while(1)
	{
		msleep(1000);	// good for > 10 ms
		//msleep_interruptible(1000); // good for > 10 ms
		//udelay(unsigned long usecs);	// good for a few us (micro s)
		//usleep_range(unsigned long min, unsigned long max); // good for 10us - 20 ms
		
		
		// In an infinite loop, you should check if the kthread_stop
		// function has been called (e.g. in clean up module). If so,
		// the kthread should exit. If this is not done, the thread
		// will persist even after removing the module.
		if(kthread_should_stop()) {
			do_exit(0);
		}
				
		// comment out if your loop is going "fast". You don't want to
		// printk too often. Sporadically or every second or so, it's okay.
		printk("Count: %d\n", ++count);
	}
	
	return 0;
}

int thread_init(void)
{
	char kthread_name[11]="my_kthread";	// try running  ps -ef | grep my_kthread
										// when the thread is active.
	printk("In init module\n");
    	    
    kthread1 = kthread_create(kthread_fn, NULL, kthread_name);
	
    if((kthread1))	// true if kthread creation is successful
    {
        printk("Inside if\n");
		// kthread is dormant after creation. Needs to be woken up
        wake_up_process(kthread1);
    }

    return 0;
}

void thread_cleanup(void) {
	int ret;
	// the following doesn't actually stop the thread, but signals that
	// the thread should stop itself (with do_exit above).
	// kthread should not be called if the thread has already stopped.
	ret = kthread_stop(kthread1);
								
	if(!ret)
		printk("Kthread stopped\n");
}

// Notice this alternative way to define your init_module()
// and cleanup_module(). "thread_init" will execute when you install your
// module. "thread_cleanup" will execute when you remove your module.
// You can give different names to those functions.
module_init(thread_init);
module_exit(thread_cleanup);
