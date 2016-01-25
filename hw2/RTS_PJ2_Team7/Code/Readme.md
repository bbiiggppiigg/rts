# RTS Project 2 Readme

## Setup  
The modified codes are layout as they should be in the linux kernel
ex: linux-2.6.32.60/arch/x86/include/asm/unistd_32.h  
Thus a simple cp -r should do the job.  

## Bonus  
For the bonus part, rename shed_sjf_rms.c to shed_sjf.c and replace the original sched_sjf.c in linux-2.6.32.60/kernel.

## Testing  
Test Programs are executed as follows:  
./test_sjf sjf quantum threads jobs  
./test_rms rms quantum threads jobs

## Note  
  Due to some technical difficulty, we use two versions of sched_sjf.c to implement the behavior of the two scheduling policy separately.
