#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#define SIMULATION_LENGTH 1440

// Global Variables
pthread_mutex_t room1lock;
pthread_mutex_t room2lock;
pthread_mutex_t room3lock;
pthread_mutex_t room4lock;
pthread_mutex_t pirateLock;
pthread_mutex_t ninjaLock;
int room1occupied;
int room2occupied;
int room3occupied;
int room3occupied;
int room4occupied;
int numPiratesInside;
int numNinjasInside;

void initializeGlobals(int numTeams)
{
	switch(numTeams)
	{
		case 4:
			pthread_mutex_init(&room4lock, NULL);	
			room4occupied = 0;
		case 3:
			pthread_mutex_init(&room3lock, NULL);	
			room3occupied = 0;
		case 2:
			pthread_mutex_init(&room1lock, NULL);	
			room2occupied = 0;
			pthread_mutex_init(&room2lock, NULL);	
			room1occupied = 0;
			pthread_mutex_init(&pirateLock, NULL);
			pthread_mutex_init(&ninjaLock, NULL);
			break;
	}
	numPiratesInside = 0;
	numNinjasInside = 0;
}

/*
	Struct to hold the statistics for the pirates and ninjas
*/
struct bill 
{
	int numVisits; 
	int totalCost;
	float *visitLengths; // Array of lengths of visits
	float *waitTimes; // Array of the times spent in line 
};

/*
	Initializes the bill for each clan with zeroes
*/
struct bill *initBill(int numVisitors)
{
	struct bill *newBill = malloc(sizeof(struct bill));
	newBill->numVisits = 0;
	newBill->totalCost = 0;
	// Here we are assuming that the number of return visits will be less than 50%. If it's not, god help us.
	newBill->visitLengths = malloc(2 * numVisitors * sizeof(int));
	newBill->waitTimes = malloc(2 * numVisitors * sizeof(int));
	return newBill;
}

/*
	Free the arrays within the bill
*/
void freeBill(struct bill *theBill)
{
	free(theBill->visitLengths);
	free(theBill->waitTimes);
	free(theBill);
}

/*
	Struct to hold the information about the day's operations
*/
struct dayReport
{
	int averageQueueLength;
	int grossRevenue; // Amount of gold
	int goldPerVisit; // Total gold / total visits
	// Calculate profit by doing (grossRevenue - (5 * numTeams))
	int *teamBusyTimes;
	int *teamFreeTimes;
};

/*
	Initialize the day report
*/
struct dayReport *initDayReport(int numTeams)
{
	struct dayReport *newReport = malloc(sizeof(struct dayReport));
	newReport->averageQueueLength = 0;
	newReport->grossRevenue = 0;
	newReport->goldPerVisit = 0;
	newReport->teamBusyTimes = malloc(numTeams * sizeof(int));
	newReport->teamFreeTimes = malloc(numTeams * sizeof(int));	
	return newReport;
}

/*
	Free the memory taken up by the report
*/
void freeReport(struct dayReport *ourReport)
{
	free(ourReport->teamFreeTimes);
	free(ourReport->teamBusyTimes);
	free(ourReport);
}

/*
	Struct for pirates or ninjas
*/
struct customer
{
	pthread_t *thread; // The thread related to our customer
	int idNum;	// A number to identify our customer
	int arrivalTime; // The time they will arrive
	int costumingTime; // The time they will take in the store

};


/*
	Function that each thread calls in order to wait while being served
*/
void *beClothed(void *costumingTime)
{
	printf("I'm finna sleep for %d minutes.\n", *(int *)costumingTime);	
	sleep(*(int *)costumingTime);

	printf("I'm outta here.\n");
	return NULL;
}

/*
	Create and return a customer
*/
struct customer *createCustomer(float avgArrivalTime, float avgCostumingTime, int id)
{

	struct customer *newCust = malloc(sizeof(struct customer));
	float arrivalTime = sqrt(-2 * log(drand48())) * cos(2 * M_PI * drand48());
	float costumingTime = sqrt(-2 * log(drand48())) * cos(2 * M_PI * drand48());
	
	//	printf("\nyour random number is... %f !\n", arrivalTime);
	arrivalTime = abs(arrivalTime * (avgArrivalTime / 3) + avgArrivalTime);
	costumingTime = abs(costumingTime * (avgCostumingTime / 3) + avgCostumingTime);
	
	pthread_t *ourThread = malloc(sizeof(pthread_t));
	newCust->thread = ourThread;
	newCust->arrivalTime = arrivalTime;
	newCust->costumingTime = costumingTime;
	newCust->idNum = id;
	
	return newCust;

}

/*
	Create and run the thread
*/
void doThread(struct customer *ourCust)
{
	pthread_create(ourCust->thread, NULL, beClothed, &ourCust->costumingTime);
}

/*
	Queue which will serve as a line
*/
struct node 
{
	// The thread at this position
	struct customer *cust;
	// The next thread in line
	struct node *next;	

};

/*
	Method to place a thread at the correct place in line based on arrival time
*/
struct node *enqueue(struct node *head, struct customer *ourCust)
{
	struct customer *currentCust;
	// If the head is null, allocate space for a new head
	if(head == NULL)
	{
		struct node *newHead = malloc(sizeof(struct node));
		newHead->cust = ourCust;
		newHead->next = NULL;
		return newHead;
	}

	// If we belong at the front of the line, make it so

	struct node *newNode = malloc(sizeof(struct node));
	newNode->cust = ourCust;

	if(ourCust->arrivalTime < head->cust->arrivalTime)
	{
		newNode->next = head;
		return newNode;
	}	

	// If not, go down the queue and find the end
	struct node *current = head;	
	while(current->next != NULL)
	{

		currentCust = current->next->cust;
		// If we are arriving before the next node, scoot in front of it
		if(ourCust->arrivalTime < currentCust->arrivalTime)
		{
			newNode->next = current->next;
			current->next = newNode;
			return head;
		}
		current = current->next;
	}
	// Allocate space for the new node
	newNode->next = NULL;	
	current->next = newNode;
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

struct node *checkRooms(int numTeams, struct node *queue){

	switch(numTeams)
	{
		case 4:
			pthread_mutex_trylock(&room4lock);
			if(!room4occupied)
			{
				room4occupied = 1;
				doThread(queue->cust);
				pthread_mutex_unlock(&room4lock);
				return dequeue(queue);	
			}
			pthread_mutex_unlock(&room4lock);
		case 3:
					
			pthread_mutex_trylock(&room3lock);
			if(!room3occupied)
			{
				room3occupied = 1;
				doThread(queue->cust);
				pthread_mutex_unlock(&room3lock);
				return dequeue(queue);	
			}
			pthread_mutex_unlock(&room3lock);
		case 2:
							
			pthread_mutex_trylock(&room2lock);
			if(!room2occupied)
			{
				printf("I got a room!\n");
				room2occupied = 1;
				doThread(queue->cust);
				pthread_mutex_unlock(&room2lock);
				return dequeue(queue);
			}
			pthread_mutex_unlock(&room2lock);
			
			pthread_mutex_trylock(&room1lock);
			if(!room1occupied)
			{
				printf("I got a room!");
				room1occupied = 1;
				doThread(queue->cust);
				pthread_mutex_unlock(&room1lock);
				return dequeue(queue);
			}
			pthread_mutex_unlock(&room1lock);
	}
	return queue;
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
	struct node *pirateQueue = NULL;
	struct node *ninjaQueue = NULL;

	// Create bills for both the pirates and the ninjas
	struct bill *pirateBill;
	struct bill *ninjaBill;
	struct dayReport *todayReport;
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


	// Initialize the bills for each faction
	pirateBill = initBill(numPirates);
	ninjaBill = initBill(numNinjas);

	todayReport = initDayReport(numCostumingTeams);

	for(int i=0; i<numPirates; i++)
	{
		// Create the pirate
		struct customer *currentPirate = createCustomer(avgPirateArrival, avgPirateTime, i);
		// Put the pirate into the line
	//	printf("Pirate %d will take %d to clothe, and will mosey on over at %d.\n", 
	//			i, currentPirate->costumingTime, currentPirate->arrivalTime);
		pirateQueue = enqueue(pirateQueue, currentPirate);
		struct node *current = pirateQueue;
	//	printf("\nThe queue currently contains: \n");
		for(int j = 0; j<=i; j++)
		{
			
		//	printf("Pirate %d arriving at %d.\n", current->cust->idNum, current->cust->arrivalTime);
			current = current->next;
		}
	}

	for(int i=0; i<numNinjas; i++)
	{
		// Create the ninja
		struct customer *currentNinja = createCustomer(avgNinjaArrival, avgNinjaTime, i);		
		// Put the ninja into the line
		// printf("Ninja %d will take %d to clothe, and will appear at %d.\n", 
		//		i, currentNinja->costumingTime, currentNinja->arrivalTime);
		ninjaQueue = enqueue(ninjaQueue, currentNinja);
	}

	initializeGlobals(numCostumingTeams);
	/*
	struct timeval startTime;
	gettimeofday(&startTime, NULL);
	int startInt = (startTime.tv_usec / 1000) + (startTime.tv_sec);	
	*/
	int secondsPassed = 0;	
	while(secondsPassed < SIMULATION_LENGTH)
	{
		// If there is someone in the pirate line, check if it's pirate time
		if(pirateQueue != NULL)
		{
			if(pirateQueue->cust->arrivalTime <= secondsPassed)
			{
				pthread_mutex_lock(&ninjaLock);
				if(numNinjasInside == 0)
				{
					pirateQueue = checkRooms(numCostumingTeams, pirateQueue);
				}
				pthread_mutex_unlock(&ninjaLock);
			}
		}
		// If anyone has finished, ask if they are coming back

		// Record whether the staff was idle or working (we should initialize the day report with appropriate number of ints for busy time and free time	
		printf("Nothing left to do this second, sleeping.\n");
		sleep(1);
		
		secondsPassed++;
				

	}
	//TODO
	// Create behavior for threads leaving (ie they unlock and exit the store)
	// Make it fair
	// Update customer struct to contain room number, and pass it into pthread create
	// If anyone has finished, then ask if they are coming back and record info about their visit
	// put them back into the end of the line with their new time.
	// 
	// Calculate expenses.
	
	// Free the memory taken by each faction's bill	
	freeBill(pirateBill);
	freeBill(ninjaBill);
	
	
	return 0;
}
