#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>
#include <cmath>
#include <pthread.h>
#include <time.h>

using namespace std;

//height and width of 2D arrays
#define ARRAY_WIDTH 1000
#define ARRAY_HEIGHT 50000

//horizontal distance between heights stored in each row of main array
//remains the same between every point and its immediate neighbours in row
//so long as this value is constant, its actual number value is unimportant
#define HORIZONTAL_POINT_DIST 50

//number of threads to split array between
#define NUM_THREADS 50000

//used to convert return value of asin() from radians to degrees
#define DEGREES_PER_RADIAN 57.2958

//a pointer to an object of this type will be passed to the thread function as a parameter whenever a thread is created
//a different object will be given to each thread - this is the easiest way to avoid a race condition when different threads
//are reading/writing to the currentRow and rowsToProcess member variables
struct ThreadData
{
	float** mainArray;
	float** distanceArray;
	float** angleArray;

	int currentRow;
	int rowsToProcess;

	//used by a thread to return back to main function the time it took to complete (return value accessed through pthread_join())
	//the only way a value can be returned from a thread is using a void pointer
	//however, returning a pointer to local storage of a terminated thread will cause an access violation
	//therefore, variable is stored locally in main instead
	float timeTaken;
};

float** setupMainArray(void); //used for importing and converting data for main array from array.txt and storing it in main array
template <typename type> type** setup2DArrayOnHeap(void); //templated function to setup a 2D array on heap (must allocate arrays on heap due to their large size)
template <typename type> void delete2DArray(type** array); //templated function to release memory used for a given 2D array (must be called for each array)

//thread function - takes a ThreadData pointer (which must be passed into the function as a void pointer)
void* processRows(void* data);

int main()
{
	//create a clock object and set it equal to current processor time used by this process (measured in clock ticks)
	clock_t startTime = clock();

	//make sure that number of threads requested is not greater than the number of rows in the array
	//if it is, then terminate program (because otherwise useless threads will be created)
	if (NUM_THREADS > ARRAY_HEIGHT)
	{
		cout << "Error! Number of threads requested is greater than the number of rows in the array." << endl;
		return 1;
	}

	//double-pointers used to point to 2D arrays
	//the 2D arrays created have been set up on the heap due to their large size (and so they can be shared between threads)
	float** mainArray = setupMainArray();
	float** distanceArray = setup2DArrayOnHeap<float>();
	float** angleArray = setup2DArrayOnHeap<float>();

	//calculate and output elapsed time
	clock_t arrayAllocTime = clock() - startTime;
	float time = ((float)arrayAllocTime / (float)CLOCKS_PER_SEC);
	cout << "Allocation of arrays on the heap takes " << time << " seconds.\n";

	//pack array pointers and other data into structs (for passing in to thread function)
	//each thread will receive a separate copy of this data - this is the easiest way to avoid
	//race conditions when different threads are reading and writing to the struct's currentRow and rowsToProcess members
	ThreadData data[NUM_THREADS];

	//initialise members of ThreadData objects
	for (int i = 0; i < NUM_THREADS; i++)
	{
		data[i].mainArray = mainArray;
		data[i].distanceArray = distanceArray;
		data[i].angleArray = angleArray;
		data[i].currentRow = 0;
	}

	//split up rows equally between threads and store results in rowsToProcess
	int rowsToProcess = ARRAY_HEIGHT / NUM_THREADS;

	//store number of rows that could not be split up equally between threads
	//each thread created will be allocated one of these rows in addition to its normal workload (until no remainder rows are left)
	int remainderRows = ARRAY_HEIGHT % NUM_THREADS;

	//keeps track of current row in array so this data can be passed to threads
	int currentRow = 0;

	//create required number of thread identifiers
	pthread_t threads[NUM_THREADS];

	//for each thread to be created, set its data parameters and create it, passing in the data
	for (int i = 0; i < NUM_THREADS; i++)
	{
		data[i].currentRow = currentRow;

		//give one of the extra rows to each thread in turn until all rows have been assigned to a thread
		if (remainderRows != 0)
		{
			data[i].rowsToProcess = rowsToProcess + 1;
			remainderRows--;
			currentRow += rowsToProcess + 1;
		}
		//once extra rows have been dealt with, use else block to allocate each thread the normal number of rows to process
		else
		{
			data[i].rowsToProcess = rowsToProcess;
			currentRow += rowsToProcess;
		}

		pthread_create(&threads[i], NULL, processRows, (void*)&data[i]);
	}

	//calculate elapsed time from start to the point straight after the threads have been created
	clock_t threadCreationTime = clock() - startTime;
	time = ((float)threadCreationTime / (float)CLOCKS_PER_SEC);
	cout << "Up to point where threads are joined, program has taken " << time << " seconds.\n";

	//set up void pointer to hold thread return value
	void* threadReturnVal;
	cout << "Thread run-time data:\n";

	//join each thread back into parent thread, printing out each thread's time to completion
	for (int i = 0; i < NUM_THREADS; i++)
	{
		pthread_join(threads[i], &threadReturnVal);
		cout << "Thread " << i << " completed in " << ((ThreadData*)threadReturnVal)->timeTaken << " seconds.\n";
	}

	//measure time taken to join the threads back together
	clock_t postThreadJoinTime = clock() - threadCreationTime;
	time = ((float)postThreadJoinTime / (float)CLOCKS_PER_SEC);
	cout << "Joining of threads takes " << time << " seconds.\n";

	//release memory used for arrays before finishing program
	delete2DArray<float>(mainArray);
	delete2DArray<float>(distanceArray);
	delete2DArray<float>(angleArray);

	//calculate and output time taken for entire process to complete
	clock_t endTime = clock() - startTime;
	cout << "The program took " << ((float)endTime / (float)CLOCKS_PER_SEC) << " seconds from start to finish.\n";

	return 0;
}

void* processRows(void* data)
{
	//get start CPU time of thread
	clock_t t = clock();

	//cast pointer back to a pointer to object of type ThreadData
	ThreadData* threadData = (ThreadData*)data;

	//assign local copies of currentRow and rowsToProcess from the thread parameter data purely for the sake of readability
	float** mainArray = threadData->mainArray;
	float** distanceArray = threadData->distanceArray;
	float** angleArray = threadData->angleArray;
	int currentRow = threadData->currentRow;
	int rowsToProcess = threadData->rowsToProcess;

	//make sure that currentRow is within bounds of array
	if (currentRow >= ARRAY_HEIGHT)
	{
		cout << "Cannot process row " << currentRow << " as it is beyond the bounds of the array!" << endl;
		pthread_exit(NULL);
	}

	//calculate distance results and populate corresponding array
	while (currentRow < ARRAY_HEIGHT && rowsToProcess != 0)
	{
		for (int j = 0; j < ARRAY_WIDTH; j++)
		{
			//set height value to compare with as that of next element in row
			int nextColumn = j + 1;

			//special case:
			//if we are looking at last element in row, "wrap around" and compare it with first element in that row
			if (j == ARRAY_WIDTH - 1)
				nextColumn = 0;

			//calculate vertical distance between points being compared
			float verticalDist = mainArray[currentRow][nextColumn] - mainArray[currentRow][j];

			//Pythagoras' Theorem to calculate Euclidean distance between the points (hypotenuse of the triangle)
			float hypotenuse = sqrt((verticalDist * verticalDist) + (HORIZONTAL_POINT_DIST * HORIZONTAL_POINT_DIST));

			distanceArray[currentRow][j] = hypotenuse;

			//calculate angle of slope from one point to the next
			angleArray[currentRow][j] = DEGREES_PER_RADIAN * asin(verticalDist / hypotenuse);
		}
		rowsToProcess--;
		currentRow++;
	}

	//calculate elapsed CPU time since thread began, convert it to seconds and assign it to threadData->timeTaken
	t = clock() - t;
	float seconds = (float)t / (float)CLOCKS_PER_SEC;
	threadData->timeTaken = seconds;

	//finish thread and return time taken to complete (using ThreadData pointer cast to void pointer)
	return (void*)threadData;
}