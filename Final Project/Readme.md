# SCADA System

> control system architecture that uses computers, networked data communications and graphical user interfaces for high-level process supervisory management

Files:
- BTNKernel.c is the program which implements the ISR of button in the kernel space
- final_server.c is the main program which updates RTUs' log and sends them to historian
- final_historian.c is the program which receives messages from all RTUs
- Makefile if the file to compile BTNKernel.c
- finalDemo.mp4 is the demo video of our project
- finalReport is the final report

Commands to run:
On the workstation:
gcc -o historian.out final_historian.c -lpthread    //compile final_historian.c
./historian.out 2500 //run the historian on workstation, the portnum can be different

On the RPi:
make    //compile kernel
sudo insmod BTNKernel.ko    //install kernel
sudo mknod /dev/Final c 244 0   //create char device ("244" could be different, check with dmesg after install kernel)
gcc -o final.out final_server.c -lpthread -lwiringPi //compile final_server.c
./final.out 2500    //run the server on RPi, the portnum need to be the same as the one historian uses


Note: final_server.c can run on multiple RTUs and just need to change with different number in getTime() function, which has  
strcpy(buffer2, "RTU1:  "); 
                    ^change to different #
