#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>

/*
	Queue which will serve as a line
*/
struct node 
{
	// The thread at this position
	pthread_t *info;
	// The next thread in line
	struct node *next;	

};

/*
	Method to place a thread at the end of the line
*/
struct node *enqueue(struct node *head, pthread_t *ourThread)
{
	// If the head is null, allocate space for a new head
	if(head == NULL)
	{
		struct node *newHead = malloc(sizeof(struct node));
		newHead->info = ourThread;
		return newHead;
	}
	// If not, go down the queue and find the end
	struct node *current = head;	
	while(current->next != NULL)
	{
		current = current->next;
	}
	// Allocate space for the new node
	current->next = malloc(sizeof(struct node));
	current->next->info = ourThread;
	return head;
}

/*
	Method to remove a thread from the front of the line
*/
struct node *dequeue(struct node *head)
{
	// If our head is the only one in line, return an empty line
	if(head->next == NULL)
	{
		free(head);
		return NULL;
	}
	// Else return a pointer to the new head of the line
	struct node *current = head->next;
	free(head);
	return current;
}

/*
	Struct to hold the statistics for the pirates and ninjas
*/
struct bill 
{
	int numVisits; 
	int totalCost;
	float *visitLengths; // Array of lengths of visits
	float *waitTimes; // 
};

/*
	Struct to hold the information about the day's operations
*/
struct dayReport
{
	int averageQueueLength;
	int grossRevenue; // Amount of gold
	int goldPerVisit; // Total gold / total visits
	// Calculate profit by doing (grossRevenue - (5 * numTeams))
	float *teamBusyTimes;
	float *teamFreeTimes;
};

/*
	Function that each thread calls in order to wait while being served
*/
void *beClothed(void *costumingTime)
{
	
	sleep(*(int *)costumingTime);
	return NULL;
}

int main(int argc, char *argv[])
{

	// If the user gave the wrong number of arguments, inform them of usage and return.
	if(argc != 8)
	{
		printf("Usage: ./problem1 <numCostumingTeam> <NumPirates> <numNinjas> <avgPirateTime> <avgNinjaTime> <avgPirateArrival> <avgNinjaArrival>\n");
		return 1;
	}

	// Get the user's input and put all of it into local variables.
	int numCostumingTeams = atoi(argv[1]);
	int numPirates = atoi(argv[2]);
	int numNinjas = atoi(argv[3]);
	float avgPirateTime = atof(argv[4]);
	float avgNinjaTime = atof(argv[5]);
	float avgPirateArrival = atof(argv[6]);
	float avgNinjaArrival = atof(argv[7]);

	// Create the lines for both pirates and ninjas
	struct queue *pirateQueue;
	struct queue *ninjaQueue;

	// If the user's input violates any of the min or max numbers, let them know and stop execution.
	if((4 < numCostumingTeams) || (numCostumingTeams < 2))
	{
		printf("Please input a costuming team number ranging from 2 to 4.\n");
		return 1;
	}
	if( (10 > numPirates) || (numPirates > 50))
	{
		printf("Please input a number of pirates ranging from 10 to 50.\n");
		return 1;
	}
	
	if( (10 > numNinjas) || (numNinjas > 50))
	{
		printf("Please input a number of ninjas ranging from 10 to 50.\n");
		return 1;
	}

	if( (0 > avgPirateTime) )
	{
		printf("Please input a positive avgPirateTime.\n");
		return 1;
	}
	if( (0 > avgNinjaTime) )
	{
		printf("Please input a positive avgNinjaTime.\n");
		return 1;
	}
	if( (0 > avgPirateArrival) )
	{
		printf("Please input a positive avgPirateArrival.\n");
		return 1;
	}
	if( (0 > avgNinjaArrival) )
	{
		printf("Please input a positive avgNinjaArrival.\n");
		return 1;
	}

	for(int i=0; i<numPirates; i++)
	{
		
		// Create a random pirate thread
		int randomNum = 10; // TODO make this actually random
		pthread_t *ourPirate = malloc(sizeof(pthread_t));
		pthread_create(ourPirate, NULL, beClothed, &randomNum);
		// Put it into the line
	}

	for(int i=0; i<numNinjas; i++)
	{
		// Create a random ninja thread
		// Put it into the line
	}

	//TODO
	// run the simulation
	// Calculate expenses.
		
	return 0;
}
