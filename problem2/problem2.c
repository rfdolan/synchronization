#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <unistd.h>
#include <sched.h>

// Global variables
sem_t nwSem;
sem_t neSem;
sem_t swSem;
sem_t seSem;
sem_t northSem;
sem_t westSem;
sem_t eastSem;
sem_t southSem;

struct node *northQueue;
struct node *westQueue;
struct node *eastQueue;
struct node *southQueue;
long long allocNodes;
long long freedNodes;

// These are functions which were used for debugging.
/*
void sem_wait(sem_t *sem)
{
	printf("Thread %lu is trying to get sem %p.\n", pthread_self(), sem);
	int status;
	sem_getvalue(&nwSem, &status);
	printf("nwSem = %d\n", status);
	sem_getvalue(&neSem, &status);
	printf("neSem = %d\n", status);
	sem_getvalue(&swSem, &status);
	printf("swSem = %d\n", status);
	sem_getvalue(&seSem, &status);
	printf("seSem = %d\n", status);
	sem_getvalue(&northSem, &status);
	printf("northSem = %d\n", status);
	sem_getvalue(&westSem, &status);
	printf("westSem = %d\n", status);
	sem_getvalue(&eastSem, &status);
	printf("eastSem = %d\n", status);
	sem_getvalue(&southSem, &status);
	printf("southSem = %d\n", status);

	sem_wait(sem);
	sem_getvalue(&nwSem, &status);
	printf("nwSem = %d\n", status);
	sem_getvalue(&neSem, &status);
	printf("neSem = %d\n", status);
	sem_getvalue(&swSem, &status);
	printf("swSem = %d\n", status);
	sem_getvalue(&seSem, &status);
	printf("seSem = %d\n", status);
	sem_getvalue(&northSem, &status);
	printf("northSem = %d\n", status);
	sem_getvalue(&westSem, &status);
	printf("westSem = %d\n", status);
	sem_getvalue(&eastSem, &status);
	printf("eastSem = %d\n", status);
	sem_getvalue(&southSem, &status);
	printf("southSem = %d\n", status);
	printf("We got sem %p in thread %lu.\n", sem, pthread_self());

}

void sem_post(sem_t *sem)
{
	printf("Thread %lu is posting sem %p.\n", pthread_self(), sem);
	sem_post(sem);
	int status;
	sem_getvalue(&nwSem, &status);
	printf("nwSem = %d\n", status);
	sem_getvalue(&neSem, &status);
	printf("neSem = %d\n", status);
	sem_getvalue(&swSem, &status);
	printf("swSem = %d\n", status);
	sem_getvalue(&seSem, &status);
	printf("seSem = %d\n", status);
	sem_getvalue(&northSem, &status);
	printf("northSem = %d\n", status);
	sem_getvalue(&westSem, &status);
	printf("westSem = %d\n", status);
	sem_getvalue(&eastSem, &status);
	printf("eastSem = %d\n", status);
	sem_getvalue(&southSem, &status);
	printf("southSem = %d\n", status);
	printf("Successfully posted sem %p in thread %lu.\n", sem, pthread_self());

}
int sem_trywait(sem_t *sem)
{
	printf("Thread %lu is try waiting for sem %p.\n", pthread_self(), sem);
	int ret = sem_trywait(sem);
	if(!ret){
		printf("We got sem %p in thread %lu.\n", sem, pthread_self());
	}
	else
	{
		printf("Thread %lu couldn't get sem %p.\n",pthread_self(), sem);
	}
	return ret;

}
*/

/*
	Function to initialize locks and global variables.
*/
void initializeGlobals()
{
	// We can treat semaphores as locks if we initialize them to 1.
	sem_init(&nwSem, 0, 1);
	sem_init(&neSem, 0, 1);
	sem_init(&swSem, 0, 1);
	sem_init(&seSem, 0, 1);
	sem_init(&northSem, 0, 1);
	sem_init(&westSem, 0, 1);
	sem_init(&eastSem, 0, 1);
	sem_init(&southSem, 0, 1);
	northQueue = NULL;
	westQueue = NULL;
	eastQueue = NULL;
	southQueue = NULL;
	allocNodes = 0;
	freedNodes = 0;


}

/*
   Struct to represent cars.
 */
struct car
{
	int idNum;
	// These directions are as follows: 
	// North = 0
	// West = 1
	// East = 2
	// South = 3
	int approachDir;
	int destinationDir;
	pthread_t *thread;
};

/*
   Create a car.
 */
struct car *createCar(int idNum)
{
	struct car *newCar = malloc(sizeof(struct car));
//	printf("Allocated car %d.\n", idNum);
//	allocNodes++;
//	printf("%lld\n", allocNodes-freedNodes);
	int approachDirection = (rand() % 4);
	int destDirection = (rand() % 4);
	// Make sure that the approach direction is not the same as the destination.
	while(approachDirection == destDirection)
	{
		destDirection = (rand() % 4);
	}
	// Assign the variables to the fields.
	newCar->approachDir = approachDirection;
	newCar->destinationDir = destDirection;
	pthread_t *ourThread = malloc(sizeof(pthread_t));
	newCar->thread = ourThread;
	newCar->idNum = idNum;
	return newCar;
}

/*
   Queue for cars.
 */
struct node
{
	struct car *car;
	struct node *next;
};

/*
   Function to add a car to the given queue.
 */
void enqueue(struct car *car, struct node **queue)
{
	struct node *ourQueue = *queue;
	// If the queue is currently null, we should make the new node the head.
	if(ourQueue == NULL)
	{
		struct node *newHead = malloc(sizeof(struct node));
//		printf("Allocated node %p.\n", newHead);
		newHead->car = car;
		newHead->next = NULL;
		*queue = newHead;
		return;
	}

	// We should add the new node to the end of the queue.
	struct node *newNode = malloc(sizeof(struct node));
	//	printf("Allocated node %p.\n", newNode);
	newNode->car = car;
	newNode->next = NULL;
	struct node *current = ourQueue;
	while(current->next != NULL)
	{
		current = current->next;
	}
	current->next = newNode;
	return;
}

/*
   Function to take the car in the front of the queue off
 */
void dequeue(struct node **queue)
{
	struct node *ourQueue = *queue;
	// If the queue will be empty, set it to null.
	if(ourQueue->next == NULL)
	{
		//		free(ourQueue->car->thread);
		//		free(ourQueue->car);
		//		printf("Freeing node %p.\n", ourQueue);
		free(ourQueue);
		*queue = NULL;
		return;
	}
	// Make the second node the head and free the first.
	struct node *current = ourQueue->next;
	//	free(ourQueue->car->thread);
	//	free(ourQueue->car);
	//	printf("Freeing node %p.\n", ourQueue);
	free(ourQueue);
	*queue = current;
	return;
}

/*
   Function to enqueue the car in the proper queue.
 */
void enqueueCar(struct car *newCar)
{

	// DIRECTIONS
		// North = 0
		// West = 1
		// East = 2
		// South = 3
	// Based on the approach direction, enqueue the car.
	if(newCar->approachDir == 0)
	{
		sem_wait(&northSem);
		enqueue(newCar, &northQueue);
		sem_post(&northSem);
	}
	else if(newCar->approachDir == 1)
	{
		sem_wait(&northSem);
		enqueue(newCar, &westQueue);
		sem_post(&northSem);
	}
	else if(newCar->approachDir == 2)
	{
		sem_wait(&northSem);
		enqueue(newCar, &eastQueue);
		sem_post(&northSem);
	}
	else if(newCar->approachDir == 3)
	{
		sem_wait(&northSem);
		enqueue(newCar, &southQueue);
		sem_post(&northSem);
	}

}

/*
   Function to figure out the path of a car. Returns an array of ints who represent which quadrants are needed.
 */
int *findPath(struct car *ourCar)
{
	int *path = malloc(sizeof(int) * 4);
	int approach = ourCar->approachDir;
	int dest = ourCar->destinationDir;
	for(int i=0; i<4; i++)
	{
		path[i] = 0;
	}
	// If we will need the nw quadrant, set the value in the array accordingly
	if((approach == 0) || ((approach == 2) && (dest != 0)) || ((approach == 3) && (dest == 1)))
	{
		path[0] = 1;
	}
	// If we will need the ne quadrant, set the value in the arrray accordingly
	if((approach == 2) || ((approach == 3) && (dest != 2)) || ((approach == 1) && (dest == 0)))
	{
		path[1] = 1;
	}
	// If we will need the sw quadrant, set the value in the arrray accordingly
	if((approach == 1) || ((approach == 0) && (dest != 1)) || ((approach == 2) && (dest == 3)))
	{
		path[2] = 1;
	}
	// If we will need the se quadrant, set the value in the arrray accordingly
	if((approach == 3) || ((approach == 1) && (dest != 3)) || ((approach == 0) && (dest == 2)))
	{
		path[3] = 1;
	}
	return path;
}

/*
   Function to be executed in the thread.
 */
void *drive(void *car)
{
	int holdingnw = 0;
	int holdingne = 0;
	int holdingsw = 0;
	int holdingse = 0;
	int succeeded = 0;
	struct car *ourCar = (struct car *)car;
	// In this array, elements are set to true if we need them.
		// carPath[0] represents the nw quadrant
		// carPath[1] represents the ne quadrant
		// carPath[2] represents the sw quadrant
		// carPath[3] represents the se quadrant
	int *carPath = findPath(ourCar);
	printf("Car %d is approaching from direction %d going to direction %d.\n", ourCar->idNum, ourCar->approachDir, ourCar->destinationDir);
	// Go through and secure every semaphore you can. If you can't get one, let everything go and try again
	while(succeeded == 0)
	{
		// Assume we succeed
		succeeded = 1;
		// If we are going to need the northwest quadrant, try to get it.
		if(carPath[0])
		{
			// If we failed to get it, we didn't succeed, so retry the loop
			if(sem_trywait(&nwSem))
			{
				succeeded = 0;
				sched_yield();
				continue;
			}
			// We got the semaphore, so print a message
			else
			{
				printf("Car %d is cleared to go through the northwest quadrant.\n", ourCar->idNum);
				holdingnw = 1;

			}
		}
		// If we are going to need the northeast quadrant, try to get it
		if(carPath[1])
		{
			// If we failed to get it, we didn't succeed, so retry the loop
			if(sem_trywait(&neSem))
			{
				// If we are holding anything, let go
				if(holdingnw)
				{
					sem_post(&nwSem);
				}
				// Get out!
				succeeded = 0;
				sched_yield();
				continue;
			}
			// We got the semaphore, so print a message
			else
			{
				printf("Car %d is cleared to go through the northeast quadrant.\n", ourCar->idNum);
				holdingne = 1;
			}
		}
		// If we are going to need the southwestern quadrant, try to get it
		if(carPath[2])
		{
			// If we failed to get it, we didn't succeed, so retry the loop
			if(sem_trywait(&swSem))
			{
				// If we are holding anything, let go
				if(holdingne)
				{
					sem_post(&neSem);
				}
				if(holdingnw)
				{
					sem_post(&nwSem);
				}
				// Get out!
				succeeded = 0;
				sched_yield();
				continue;
			}
			// We got the semaphore, so print a message
			else
			{
				printf("Car %d is cleared to go through the southwest quadrant.\n", ourCar->idNum);
				holdingsw = 1;
			}
		}
		// If we are going to need the southwestern quadrant, try to get it
		if(carPath[3])
		{
			// If we gailed to get it, we didn't succeed, so retry the loop
			if(sem_trywait(&seSem))
			{
				// If we are holding anything, let go
				if(holdingsw)
				{
					sem_post(&swSem);
				}
				if(holdingne)
				{
					sem_post(&neSem);
				}
				if(holdingnw)
				{
					sem_post(&nwSem);
				}
				// Get out!
				succeeded = 0;
				sched_yield();
				continue;
			}
			// We got the semaphore, so print a message
			else
			{
				printf("Car %d is cleared to go through the southeast quadrant.\n", ourCar->idNum);
				holdingse = 1;
			}
		}
	}
//	printf("We got here.\n");
	// We made it through, so let go of the semaphores
	if(holdingse)
	{
		printf("Car %d no longer needs the southeast quadrant.\n", ourCar->idNum);
		sem_post(&seSem);
	}
	if(holdingsw)
	{
		printf("Car %d no longer needs the southwest quadrant.\n", ourCar->idNum);
		sem_post(&swSem);
	}
	if(holdingne)
	{
		printf("Car %d no longer needs the northeast quadrant.\n", ourCar->idNum);
		sem_post(&neSem);
	}
	if(holdingnw)
	{
		printf("Car %d no longer needs the northwest quadrant.\n", ourCar->idNum);
		sem_post(&nwSem);
	}
	// We made it!
	printf("Car %d made it through the intersection!\n", ourCar->idNum);
	free(carPath);
//	printf("We freed car %d.\n", ourCar->idNum);
//	freedNodes++;
	// Free the car
	int num = ourCar->idNum;
	free(ourCar->thread);
	free(ourCar);
	// Make a new car and enqueue it
	struct car *newCar = createCar(num);
	enqueueCar(newCar);
	return NULL;
}
/*
   Method to create and execute the thread.
 */
void doThread(struct car *ourCar, int direction)
{
	pthread_create(ourCar->thread, NULL, drive, ourCar);
}



int main(int argc, char *argv[])
{
	if(argc != 1)
	{
		printf("We don't need any arguments, try again.\n");
		return 1;
	}
	initializeGlobals();

	// Generate 20 cars and put them in the right queues
	for(int i=0; i<20; i++)
	{
		struct car *newCar = createCar(i);
		//		printf("Created car %d coming from %d and going to %d.\n", newCar->idNum, newCar->approachDir, newCar->destinationDir);
		enqueueCar(newCar);	

	}
	// In this loop, we want to dequeue one car from every queue and set them up to go through
	while( (northQueue != NULL) || (westQueue != NULL) || (eastQueue != NULL) || (southQueue != NULL))
	{
		// Let some cars get through
		sleep(1);
		// Access the queue to see if there are still cars
		sem_wait(&northSem);
		// If there are still cars...
		if(northQueue != NULL)
		{
			// Dequeue and do the thread
			struct car *ourCar = northQueue->car;
			dequeue(&northQueue);
			sem_post(&northSem);
			doThread(ourCar, 0);

		}
		// There weren't any cars, so let go of the semaphore for the north queue
		else
		{
			sem_post(&northSem);
		}
		sem_wait(&westSem);
		if(westQueue != NULL)
		{
			struct car *ourCar = westQueue->car;
			dequeue(&westQueue);
			sem_post(&westSem);
			doThread(ourCar, 1);
		}
		else
		{
			sem_post(&westSem);
		}
		sem_wait(&eastSem);
		if(eastQueue != NULL)
		{
			struct car *ourCar = eastQueue->car;
			dequeue(&eastQueue);
			sem_post(&eastSem);
			doThread(ourCar, 2);
		}
		else
		{
			sem_post(&eastSem);
		}
		sem_wait(&southSem);
		if(southQueue != NULL)
		{
			struct car *ourCar = southQueue->car;
			dequeue(&southQueue);
			sem_post(&southSem);
			doThread(ourCar, 3);
		}
		else
		{
			sem_post(&southSem);
		}


	}

	return 0;
}
