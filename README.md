

## Note:
	1. i haven't finished it yet, i will keep updating :-(
	2. need to change right *Bochs* path in Makefile

## Environment:
	CentOS-6.10-x86_64
	NASM version 2.07
	GCC version 4.4.7
	GNU ld version 2.20.51.0.2-5.48.el6
	Bochs version 2.6.9

## Running:
	$ make run

****
### Timeline:
#### 1. bootloader:
	0. bootloader function:
			enable protection mode
			initialize page table
			probe available memory
	1. $ make run
	2. $ <bochs:1> b 0x1000
	   $ <bochs:2> c
	![](https://raw.githubusercontent.com/hoR4bynZ/hoo/master/images/bootloader.png)
	3. the page table map:
	   $ <bochs:3> info tab
	![](https://raw.githubusercontent.com/hoR4bynZ/hoo/master/images/initialize-page-map.png)
	
#### 2. interruption:
	1. need to alter ./src/kernel/main.c
			call interrupt routine by `int 0x..`
	![](https://raw.githubusercontent.com/hoR4bynZ/hoo/master/images/int-call-interruption.png)
			other interrupt routine such as *timer*
	![](https://raw.githubusercontent.com/hoR4bynZ/hoo/master/images/timer-interruption.png)

#### 3. memory:
	1. need to alter ./src/kernel/main.c
	![](https://raw.githubusercontent.com/hoR4bynZ/hoo/master/images/memory.png)

#### 4. thread schedule:
	1. need to alter ./src/kernel/main.c
	![](https://raw.githubusercontent.com/hoR4bynZ/hoo/master/images/multi-thread-schedule.png)