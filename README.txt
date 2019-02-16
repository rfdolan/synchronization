Threads and Synchronization
By Raymond Dolan

This assignment consists of simulations for two synchronization problems.

PIRATES AND NINJAS

This project can be run by calling make and then going to the directory problem1
and running ./problem1 with the following arguments
The number of costuming teams
The number of pirates
The number of ninjas
The average time it takes to costume a pirate
The average time it takes to costume a ninja
The average arrival time for pirates
The average arrival time for ninjas

The program is meant to simulate a critical section (the shop) where two conflicting factions
(the pirates and ninjas) cannot be inside at the same time (they do hate eachother after all.

I solved the problem using locks, and you can read more in the file problem1/problem1.explanation.txt

MASSACHUSETTS DRIVERS

This project can be run by calling make and then going to the directory problem2
and running ./problem2 with no arguments

The program is meant to simulate a four way intersection. It utilizes threads and semaphores to
keep the program efficent and fair.

You can read more about the implementation in the file problem2/problem2.exlplanation.txt
