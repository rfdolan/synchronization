#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[]){

	// If the user gave the wrong number of arguments, inform them of usage and return.
	if(argc != 8){
		printf("Usage: ./problem1 <numCostumingTeam> <NumPirates> <numNinjas> <avgPirateTime> <avgNinjaTime> <avgPirateArrival> <avgNinjaArrival>\n");
		return 1;
	}

	// Get the user's input and put all of it into local variables.
	int numCostumingTeams = atoi(argv[1]);
	int numPirates = atoi(argv[2]);
	int numNinjas = atoi(argv[3]);
	int avgPirateTime = atoi(argv[4]);
	int avgNinjaTime = atoi(argv[5]);
	int avgPirateArrival = atoi(argv[6]);
	int avgNinjaArrival = atoi(argv[7]);

	// If the user's input violates any of the min or max numbers, let them know and stop execution.
	if((4 < numCostumingTeams) || (numCostumingTeams < 2)){
		printf("Please input a costuming team number ranging from 2 to 4.\n");
		return 1;
	}
	if( (10 > numPirates) || (numPirates > 50)){
		printf("Please input a number of pirates ranging from 10 to 50.\n");
		return 1;
	}
	
	if( (10 > numNinjas) || (numNinjas > 50)){
		printf("Please input a number of ninjas ranging from 10 to 50.\n");
		return 1;
	}

	//TODO
	// Create and randomize each pirate and each ninja.
	// Create a waiting line for pirates and for ninjas.
	// run the simulation
	// Calculate expenses.
		
	return 0;
}
