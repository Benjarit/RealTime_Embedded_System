/* ece4220lab6_isr.c
 * ECE4220/7220
 * Author: Luis Alberto Rivera
 
 Basic steps needed to configure GPIO interrupt and attach a handler.
 Check chapter 6 of the BCM2835 ARM Peripherals manual for details about
 the GPIO registers, how to configure, set, clear, etc.
 
 Note: this code is not functional. It shows some of the actual code that you need,
 but some parts are only descriptions, hints or recommendations to help you
 implement your module.
 
 You can compile your module using the same Makefile provided. Just add
 obj-m += YourModuleName.o
 */

#ifndef MODULE
#define MODULE
#endif
#ifndef __KERNEL__
#define __KERNEL__
#endif

#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

/* Declare your pointers for mapping the necessary GPIO registers.
   You need to map:
   
   - Pin Event detect status register(s)
   - Rising Edge detect register(s) (either synchronous or asynchronous should work)
   - Function selection register(s)
   - Pin Pull-up/pull-down configuration registers
   
   Important: remember that the GPIO base register address is 0x3F200000, not the
   one shown in the BCM2835 ARM Peripherals manual.
*/

int mydev_id;	// variable needed to identify the handler

// Interrupt handler function. Tha name "button_isr" can be different.
// You may use printk statements for debugging purposes. You may want to remove
// them in your final code.
static irqreturn_t button_isr(int irq, void *dev_id)
{
	// In general, you want to disable the interrupt while handling it.
	disable_irq_nosync(79);

	// This same handler will be called regardless of what button was pushed,
	// assuming that they were properly configured.
	// How can you determine which button was the one actually pushed?
		
	// DO STUFF (whatever you need to do, based on the button that was pushed)

	// IMPORTANT: Clear the Event Detect status register before leaving.	
	
	printk("Interrupt handled\n");	
	enable_irq(79);		// re-enable interrupt
	
    return IRQ_HANDLED;
}

int init_module()
{
	int dummy = 0;

	// Map GPIO registers
	// Remember to map the base address (beginning of a memory page)
	// Then you can offset to the addresses of the other registers

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

	printk("Button Detection enabled.\n");
	return 0;
}

// Cleanup - undo whatever init_module did
void cleanup_module()
{
	// Good idea to clear the Event Detect status register here, just in case.
		
	// Disable (Async) Rising Edge detection for all 5 GPIO ports.
	
	// Remove the interrupt handler; you need to provide the same identifier
    free_irq(79, &mydev_id);
	
	printk("Button Detection disabled.\n");
}
