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
pthread_mutex_t reportLock;
pthread_mutex_t batchLock;

int room1occupied;
int room2occupied;
int room3occupied;
int room3occupied;
int room4occupied;
int numPiratesInside;
int numNinjasInside;
int simulationTime;
int doingPirateBatch;

struct node *pirateQueue;
struct node *ninjaQueue;
struct dayReport *report;
/*
void pthread_mutex_lock(pthread_mutex_t *lock)
{
	printf("Locking %p in thread %lu.\n", lock, pthread_self());
	pthread_mutex_lock(lock);
	printf("We succeeded in locking %p in thread %lu.\n", lock, pthread_self());
}

void pthread_mutex_unlock(pthread_mutex_t *lock)
{
	printf("Unocking %p.\n", lock);
	pthread_mutex_unlock(lock);
	printf("We succeeded in unlocking %p.\n", lock);
}
*/
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
	int unhappyCustomers;
};

/*
	Initialize the day report
*/
void initDayReport(int numTeams)
{
	struct dayReport *newReport = malloc(sizeof(struct dayReport));
	newReport->averageQueueLength = 0;
	newReport->grossRevenue = 0;
	newReport->goldPerVisit = 0;
	newReport->teamBusyTimes = malloc(numTeams * sizeof(int));
	newReport->teamFreeTimes = malloc(numTeams * sizeof(int));	
	newReport->unhappyCustomers = 0;
	report = newReport;
}

/*
	Function to update the daily report with some statistics
*/
void updateDayReport(int numTeams)
{
	// Update the free and busy times
	switch(numTeams)
	{
		case 4:
			pthread_mutex_lock(&room4lock);
			if(room4occupied)
			{
				report->teamBusyTimes[3] += 1;
			}
			else
			{
				report->teamFreeTimes[3] += 1;
			}
			pthread_mutex_unlock(&room4lock);	
		case 3:
			pthread_mutex_lock(&room3lock);
			if(room3occupied)
			{
				report->teamBusyTimes[2] += 1;
			}
			else
			{
				report->teamFreeTimes[2] += 1;
			}
			pthread_mutex_unlock(&room3lock);	
		case 2:

			pthread_mutex_lock(&room2lock);
			if(room2occupied)
			{
				report->teamBusyTimes[1] += 1;
			}
			else
			{
				report->teamFreeTimes[1] += 1;
			}
			pthread_mutex_unlock(&room2lock);	
			pthread_mutex_lock(&room1lock);
			if(room1occupied)
			{
				report->teamBusyTimes[0] += 1;
			}
			else
			{
				report->teamFreeTimes[0] += 1;
			}
			pthread_mutex_unlock(&room1lock);	
	}
	
	
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
	int roomNumber;
	int isPirate;
	int timeServed;
};


/*
	Create and return a customer
*/
struct customer *createCustomer(float avgArrivalTime, float avgCostumingTime, int id, int isPirate)
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
	newCust->roomNumber = 0;
	newCust->isPirate = isPirate;
	newCust->timeServed = 0;
	
	return newCust;

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
void enqueuePirate(struct customer *ourCust)
{
	struct customer *currentCust;
	// If the head is null, allocate space for a new head
	if(pirateQueue == NULL)
	{
		struct node *newHead = malloc(sizeof(struct node));
		newHead->cust = ourCust;
		newHead->next = NULL;
		pirateQueue = newHead;
		return;
	}

	// If we belong at the front of the line, make it so

	struct node *newNode = malloc(sizeof(struct node));
	newNode->cust = ourCust;

	if(ourCust->arrivalTime < pirateQueue->cust->arrivalTime)
	{
		newNode->next = pirateQueue;
		pirateQueue = newNode;
		return;
	}	

	// If not, go down the queue and find the end
	struct node *current = pirateQueue;	
	while(current->next != NULL)
	{

		currentCust = current->next->cust;
		// If we are arriving before the next node, scoot in front of it
		if(ourCust->arrivalTime < currentCust->arrivalTime)
		{
			newNode->next = current->next;
			current->next = newNode;
			return;
		}
		current = current->next;
	}
	// Allocate space for the new node
	newNode->next = NULL;	
	current->next = newNode;
	return;
}


/*
	Method to remove a thread from the front of the line
*/
void dequeuePirate()
{
	// If our head is the only one in line, return an empty line
	if(pirateQueue->next == NULL)
	{
		free(pirateQueue);
		pirateQueue = NULL;
		return;
	}
	// Else return a pointer to the new head of the line
	struct node *current = pirateQueue->next;
	free(pirateQueue);
	pirateQueue = current;	
	return;
}

/*
	Method to place a thread at the correct place in line based on arrival time
*/
void enqueueNinja(struct customer *ourCust)
{
	struct customer *currentCust;
	// If the head is null, allocate space for a new head
	if(ninjaQueue == NULL)
	{
		struct node *newHead = malloc(sizeof(struct node));
		newHead->cust = ourCust;
		newHead->next = NULL;
		ninjaQueue = newHead;
		return;
	}

	// If we belong at the front of the line, make it so

	struct node *newNode = malloc(sizeof(struct node));
	newNode->cust = ourCust;

	if(ourCust->arrivalTime < ninjaQueue->cust->arrivalTime)
	{
		newNode->next = ninjaQueue;
		ninjaQueue = newNode;
		return;
	}	

	// If not, go down the queue and find the end
	struct node *current = ninjaQueue;	
	while(current->next != NULL)
	{

		currentCust = current->next->cust;
		// If we are arriving before the next node, scoot in front of it
		if(ourCust->arrivalTime < currentCust->arrivalTime)
		{
			newNode->next = current->next;
			current->next = newNode;
			return;
		}
		current = current->next;
	}
	// Allocate space for the new node
	newNode->next = NULL;	
	current->next = newNode;
	return;
}

/*
	Function to leave a dressing room
*/
void leave(struct customer *cust)
{
	if(cust->isPirate)
	{
		pthread_mutex_lock(&pirateLock);
		numPiratesInside--;
		pthread_mutex_unlock(&pirateLock);
	}
	else
	{
		pthread_mutex_lock(&ninjaLock);
		numNinjasInside--;
		pthread_mutex_unlock(&ninjaLock);
	}
	switch(cust->roomNumber)
	{
		case 4:
			pthread_mutex_trylock(&room4lock);
			room4occupied = 0;
			
			pthread_mutex_unlock(&room4lock);
			break;
			
		case 3:
					
			pthread_mutex_trylock(&room3lock);
			room3occupied = 0;
			
			pthread_mutex_unlock(&room3lock);
			break;
		case 2:
							
			pthread_mutex_trylock(&room2lock);
			room2occupied = 0;
			
			pthread_mutex_unlock(&room2lock);
			break;
		case 1:
			pthread_mutex_trylock(&room1lock);
			room1occupied = 0;
			
			pthread_mutex_unlock(&room1lock);
			break;
	}

	pthread_mutex_lock(&reportLock);
	// Update the report
	report->goldPerVisit += 1;
	if((cust->timeServed - 30) > cust->arrivalTime)
	{
		report->unhappyCustomers += 1;	
	}
	else
	{
		report->grossRevenue += (cust->costumingTime * 1);
	}
	


	pthread_mutex_unlock(&reportLock);
	int chance =(int)(rand() %4);
	// If they are coming back, then re queue them
	if(chance == 0)
	{
		cust->arrivalTime = simulationTime + cust->arrivalTime;
		if(cust->isPirate)
		{
			enqueuePirate(cust);
		}
		else
		{
			enqueueNinja(cust);
		}
	}		
}


/*
	Function that each thread calls in order to wait while being served
*/
void *beClothed(void *cust)
{
	struct customer *ourCust = (struct customer *)cust;
	ourCust->timeServed = simulationTime;
	sleep(ourCust->costumingTime);
	leave(ourCust);
	if(ourCust->isPirate)
	{
		printf("Pirate number %d is leaving room %d.\n", ourCust->idNum, ourCust->roomNumber);
	}
	else
	{

		printf("Ninja number %d is leaving room %d.\n", ourCust->idNum, ourCust->roomNumber);
	}
	if(ourCust->arrivalTime > ourCust->timeServed)
	{
		printf("They will be returning at %d.\n", ourCust->arrivalTime);
	}
	else
	{
		printf("They won't be coming back today.\n");
	}
	return NULL;
}

/*
	Create and run the thread
*/
void doThread(struct customer *ourCust)
{
	pthread_create(ourCust->thread, NULL, beClothed, ourCust);
}

/*
	Method to remove a thread from the front of the line
*/
void dequeueNinja()
{
	// If our head is the only one in line, return an empty line
	if(ninjaQueue->next == NULL)
	{
		free(ninjaQueue);
		ninjaQueue = NULL;
		return;
	}
	// Else return a pointer to the new head of the line
	struct node *current = ninjaQueue->next;
	free(ninjaQueue);
	ninjaQueue = current;	
	return;
}

void checkRoomsPirate(int numTeams){

	// For each room, check the room, and if it's unlocked enter. (When checking the room, make sure to lock!)
	switch(numTeams)
	{
		case 4:
			pthread_mutex_trylock(&room4lock);
			if(!room4occupied)
			{
				numPiratesInside += 1;
				doingPirateBatch = 0;
				room4occupied = 1;
				pirateQueue->cust->roomNumber = 4;
				printf("Pirate %d is entering room %d.\n", pirateQueue->cust->idNum, 4);
				doThread(pirateQueue->cust);
				pthread_mutex_unlock(&room4lock);
				dequeuePirate();	
				pthread_mutex_unlock(&room4lock);
				break;
			}
			pthread_mutex_unlock(&room4lock);
		case 3:
					
			pthread_mutex_trylock(&room3lock);
			if(!room3occupied)
			{
				numPiratesInside += 1;
				doingPirateBatch = 0;
				room3occupied = 1;
				pirateQueue->cust->roomNumber = 3;
				printf("Pirate %d is entering room %d.\n", pirateQueue->cust->idNum, 3);
				doThread(pirateQueue->cust);
				pthread_mutex_unlock(&room3lock);
				dequeuePirate();	
				pthread_mutex_unlock(&room3lock);
				break;
			}
			pthread_mutex_unlock(&room3lock);
		case 2:
							
			pthread_mutex_trylock(&room2lock);
			if(!room2occupied)
			{
				numPiratesInside += 1;
				doingPirateBatch = 0;
				room2occupied = 1;
				pirateQueue->cust->roomNumber = 2;
				printf("Pirate %d is entering room %d.\n", pirateQueue->cust->idNum, 2);
				doThread(pirateQueue->cust);
				pthread_mutex_unlock(&room2lock);
				dequeuePirate();
				pthread_mutex_unlock(&room2lock);
				break;
			}
			pthread_mutex_unlock(&room2lock);
			
			pthread_mutex_trylock(&room1lock);
			if(!room1occupied)
			{
				numPiratesInside += 1;
				doingPirateBatch = 0;
				room1occupied = 1;
				pirateQueue->cust->roomNumber = 1;
				printf("Pirate %d is entering room %d.\n", pirateQueue->cust->idNum, 1);
				doThread(pirateQueue->cust);
				pthread_mutex_unlock(&room1lock);
				dequeuePirate();
				pthread_mutex_unlock(&room1lock);
				break;
			}
			pthread_mutex_unlock(&room1lock);
	}
}

void checkRoomsNinja(int numTeams){

	switch(numTeams)
	{
		case 4:
			pthread_mutex_trylock(&room4lock);
			if(!room4occupied)
			{
				numNinjasInside += 1;
				doingPirateBatch = 1;
				room4occupied = 1;
				ninjaQueue->cust->roomNumber = 4;
				printf("Ninja %d is entering room %d.\n", ninjaQueue->cust->idNum, 4);
				doThread(ninjaQueue->cust);
				pthread_mutex_unlock(&room4lock);
				dequeueNinja();	
				pthread_mutex_unlock(&room4lock);
				break;
			}
			pthread_mutex_unlock(&room4lock);
		case 3:
					
			pthread_mutex_trylock(&room3lock);
			if(!room3occupied)
			{
				numNinjasInside += 1;
				doingPirateBatch = 1;
				room3occupied = 1;
				ninjaQueue->cust->roomNumber = 3;
				printf("Ninja %d is entering room %d.\n", ninjaQueue->cust->idNum, 3);
				doThread(ninjaQueue->cust);
				pthread_mutex_unlock(&room3lock);
				dequeueNinja();	
				pthread_mutex_unlock(&room3lock);
				break;
			}
			pthread_mutex_unlock(&room3lock);
		case 2:
							
			pthread_mutex_trylock(&room2lock);
			if(!room2occupied)
			{
				numNinjasInside += 1;
				doingPirateBatch = 1;
				room2occupied = 1;
				ninjaQueue->cust->roomNumber = 2;
				printf("Ninja %d is entering room %d.\n", ninjaQueue->cust->idNum, 2);
				doThread(ninjaQueue->cust);
				pthread_mutex_unlock(&room2lock);
				dequeueNinja();
				pthread_mutex_unlock(&room2lock);
				break;
			}
			pthread_mutex_unlock(&room2lock);
			
			pthread_mutex_trylock(&room1lock);
			if(!room1occupied)
			{
				numNinjasInside += 1;
				doingPirateBatch = 1;
				room1occupied = 1;
				ninjaQueue->cust->roomNumber = 1;
				printf("Ninja %d is entering room %d.\n", ninjaQueue->cust->idNum, 1);
				doThread(ninjaQueue->cust);
				pthread_mutex_unlock(&room1lock);
				dequeueNinja();
				pthread_mutex_unlock(&room1lock);
				break;
			}
			pthread_mutex_unlock(&room1lock);
	}
}

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
	pirateQueue = NULL;
	ninjaQueue = NULL;
	doingPirateBatch = 1;
	initDayReport(numTeams);
}

/*
	Function to print out statistics at the end of the simulation.
*/
void printStats(int numTeams)
{
	printf("\n--DAY'S REPORT--\n\n");
	pthread_mutex_lock(&reportLock);
	switch(numTeams)
	{
		case 4:
			printf("Team 4 was busy for %d minutes.\nTeam 4 was free for %d minutes.\n\n", 
					report->teamBusyTimes[3], report->teamFreeTimes[3]);
		case 3:
			
			printf("Team 3 was busy for %d minutes.\nTeam 3 was free for %d minutes.\n\n", 
					report->teamBusyTimes[2], report->teamFreeTimes[2]);
		case 2:

			printf("Team 2 was busy for %d minutes.\nTeam 2 was free for %d minutes.\n\n", 
					report->teamBusyTimes[1], report->teamFreeTimes[1]);
			printf("Team 1 was busy for %d minutes.\nTeam 1 was free for %d minutes.\n\n", 
					report->teamBusyTimes[0], report->teamFreeTimes[0]);
	}
	printf("Gross Revenue: %d gold pieces.\n\n", report->grossRevenue);
	printf("Gold per Visit: %d gold pieces.\n\n",
			((report->grossRevenue - (5*numTeams))/(report->goldPerVisit)));
	printf("Total Profits: %d gold pieces.\n\n", (report->grossRevenue - (5*numTeams)));
	printf("Unhappy Customers: %d.\n\n", report->unhappyCustomers);

	pthread_mutex_unlock(&reportLock);
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
/*
	// Create the lines for both pirates and ninjas
	struct node *pirateQueue = NULL;
	struct node *ninjaQueue = NULL;
*/
	// Create bills for both the pirates and the ninjas
	struct bill *pirateBill;
	struct bill *ninjaBill;
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

	initializeGlobals(numCostumingTeams);

	for(int i=0; i<numPirates; i++)
	{
		// Create the pirate
		struct customer *currentPirate = createCustomer(avgPirateArrival, avgPirateTime, i, 1);
		// Put the pirate into the line
	//	printf("Pirate %d will take %d to clothe, and will mosey on over at %d.\n", 
	//			i, currentPirate->costumingTime, currentPirate->arrivalTime);
		enqueuePirate(currentPirate);
	
		/*struct node *current = pirateQueue;
		printf("\nThe queue currently contains: \n");
		for(int j = 0; j<=i; j++)
		{
			
			printf("Pirate %d arriving at %d.\n", current->cust->idNum, current->cust->arrivalTime);
			current = current->next;
		}
		*/
	}

	for(int i=0; i<numNinjas; i++)
	{
		// Create the ninja
		struct customer *currentNinja = createCustomer(avgNinjaArrival, avgNinjaTime, i, 0);		
		// Put the ninja into the line
		// printf("Ninja %d will take %d to clothe, and will appear at %d.\n", 
		//		i, currentNinja->costumingTime, currentNinja->arrivalTime);
		enqueueNinja(currentNinja);
	}

	/*
	struct timeval startTime;
	gettimeofday(&startTime, NULL);
	int startInt = (startTime.tv_usec / 1000) + (startTime.tv_sec);	
	*/
	while((simulationTime < SIMULATION_LENGTH))
	{
		// If there is someone in the pirate line, check if it's pirate time
		pthread_mutex_lock(&batchLock);
		if(doingPirateBatch)
		{
			pthread_mutex_lock(&ninjaLock);
			if(numNinjasInside == 0)
			{
				pthread_mutex_lock(&pirateLock);
				while((pirateQueue != NULL) && 
						(pirateQueue->cust->arrivalTime <= simulationTime) && 
						(numPiratesInside < numCostumingTeams))
				{
					checkRoomsPirate(numCostumingTeams);
				}
				pthread_mutex_unlock(&pirateLock);
				
			}
			pthread_mutex_unlock(&ninjaLock);
		}
		else
		{
			pthread_mutex_lock(&pirateLock);
			if(numPiratesInside == 0)
			{
				pthread_mutex_lock(&ninjaLock);
				while((ninjaQueue != NULL) && 
						(ninjaQueue->cust->arrivalTime <= simulationTime) && 
						(numNinjasInside < numCostumingTeams) )
				{
					checkRoomsNinja(numCostumingTeams);
				}
				pthread_mutex_unlock(&ninjaLock);
				
			}
			pthread_mutex_unlock(&pirateLock);
		}
		pthread_mutex_unlock(&batchLock);

		pthread_mutex_lock(&reportLock);
		updateDayReport(numCostumingTeams);	
		pthread_mutex_unlock(&reportLock);
		// Record whether the staff was idle or working (we should initialize the day report with appropriate number of ints for busy time and free time	
		pthread_mutex_lock(&batchLock);
		pthread_mutex_lock(&pirateLock);
		// If we are currently doing a batch of pirates, then don't let any more in
		if((numPiratesInside > 0) && (doingPirateBatch))
		{
			doingPirateBatch = 0;
		}
		pthread_mutex_unlock(&pirateLock);
		pthread_mutex_lock(&ninjaLock);
		// If we are doing a batch of ninjas, don't let any more in.
		if((numNinjasInside > 0) && (!doingPirateBatch))
		{
			doingPirateBatch = 1;
		}
		pthread_mutex_unlock(&ninjaLock);
		pthread_mutex_unlock(&batchLock);
		printf("Nothing left to do this second, sleeping.\n");
		sleep(1);
		simulationTime++;
				

	}
	//TODO
	// Make it fair
	// 

	printStats(numCostumingTeams);
	// Free the memory taken by each faction's bill	
	freeBill(pirateBill);
	freeBill(ninjaBill);
	
	
	return 0;
}
