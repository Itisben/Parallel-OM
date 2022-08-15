# Parallel-OM
This code is for the paper "New Parallel Order Maintenance Data Structure. 


# Run

Goto core-main/  
Run ./core2 
You will see the cmd line to run this. 

-p: the path of the random number file  
-I: the number nodes for operation  
-w: the number of workers (1 - 64)  
-l: the type of lock, 0 CAS_LOCK, 1 OMP_LOCK(default), 2 NO lock  
-d: set debug.  
-t:  1 test,20 (no relable) 21 test OM REPEAT_RANDOM (few), 22 FIXED_MULTIPLE (many), 23 FIXED_ONE (max)  
-T: 1 repeated random positions, 2 on-the-fly random position.  
-s: 1 sorted postion, 0 unsorted position (default).   
 for example: ./core2 -p path -I 10000000 -T 1 -t 20 -w 16  
 
 
 As an exmaple: ./core2 -p ~/test-graph/test/ -I 10000000 -T 3 -t 21 -l 0 -w 2
 
