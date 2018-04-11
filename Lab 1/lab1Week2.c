/*
 ============================================================================
 Name        : Lab1Week2.c
 Author      : Ben
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
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

MODULE_LICENSE("GPL");

unsigned long* ptr;


int init_module(void)
{
	ptr = (unsigned long *)ioremap(0x3F200000,4096);
	*ptr = *ptr | 0x9240;
	ptr = ptr+(0x1c/4);
	*ptr = *ptr | 0x3c;
	printk("Module Installed");
	return 0;
}
void cleanup_module(void)
{
	ptr = (unsigned long *)ioremap(0x3F200000,4096);
	*ptr = *ptr | 0x9240;
	ptr = ptr+(0x28/4);
	*ptr = *ptr | 0x3c;
	printk("Module Cleaned");
}
