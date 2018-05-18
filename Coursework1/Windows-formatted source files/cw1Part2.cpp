#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>
#include <cmath>
#include <time.h>

using namespace std;

//height and width of 2D array of points
#define ARRAY_WIDTH 1000
#define ARRAY_HEIGHT 50000

//horizontal distance between heights stored in each row of main array
//remains the same for every point and its immediate neighbours in row
//so long as this value is constant, its actual number value is unimportant
#define HORIZONTAL_POINT_DIST 50

//number of rows of array to process at each function call to processRows
#define ROWS_TO_PROCESS 7

//used to convert return value of asin() from radians to degrees
#define DEGREES_PER_RADIAN 57.2958

//used to hold array pointers which will be passed to processRows function
//makes the processRows function parameter list tidier
struct Arrays
{
	float** mainArray;
	float** distanceArray;
	float** angleArray;
};

float** setupMainArray(void); //used for importing and converting data for main array from array.txt and storing it in main array
template <typename type> type** setup2DArrayOnHeap(void); //templated function to setup a 2D array on heap (must allocate arrays on heap due to their large size)
template <typename type> void delete2DArray(type** array); //templated function to release memory used for a given 2D array (must be called for each array)

//calculates distances and angles for elements in the requested rows in the array
//returns the number of rows processed
int processRows(Arrays arrays, int currentRow, int rowsToProcess);

int main()
{
	//create a clock object and set it equal to current processor time used by this process (measured in clock ticks)
	clock_t t = clock();

	//double-pointers used to point to 2D arrays
	//the 2D arrays created have been set up on the heap due to their large size
	float** mainArray = setupMainArray();
	float** distanceArray = setup2DArrayOnHeap<float>();
	float** angleArray = setup2DArrayOnHeap<float>();

	//pack array pointers into a struct (for passing in to row processing function)
	//this helps reduce the size of the parameter list for processRows()
	Arrays arrays;
	arrays.mainArray = mainArray;
	arrays.distanceArray = distanceArray;
	arrays.angleArray = angleArray;

	//set current row to look at as row 0
	int currentRow = 0;

	//loop through mainArray, processing rows until we reach the end of the array
	//processRows() will process as many as possible of ROWS_TO_PROCESS number of rows until it hits the end of the array
	//a call to processRows() where "ROWS_TO_PROCESS" > "remaining rows in array" is safe
	while (currentRow < ARRAY_HEIGHT)
		currentRow += processRows(arrays, currentRow, ROWS_TO_PROCESS); //increment currentRow by number of rows processed

	//release memory used for arrays before finishing program
	delete2DArray<float>(mainArray);
	delete2DArray<float>(distanceArray);
	delete2DArray<float>(angleArray);

	//set value of clock object to current processor time used minus processor time used at start of process
	t = clock() - t;

	//output time taken for program to complete (converting clock ticks to seconds)
	cout << "The program took " << ((float)t / (float)CLOCKS_PER_SEC) << " seconds from start to finish.\n";

	return 0;
}

int processRows(Arrays arrays, int currentRow, int rowsToProcess)
{
	//make sure that currentRow is within bounds of array
	if (currentRow >= ARRAY_HEIGHT)
	{
		cout << "Cannot process row " << currentRow << " as it is beyond the bounds of the array!" << endl;
		throw;
	}

	int rowsProcessed = 0;

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
			float verticalDist = arrays.mainArray[currentRow][nextColumn] - arrays.mainArray[currentRow][j];

			//Pythagoras' Theorem to calculate Euclidean distance between the points (hypotenuse of the triangle)
			float hypotenuse = sqrt(pow(verticalDist, 2) + pow(HORIZONTAL_POINT_DIST, 2));

			arrays.distanceArray[currentRow][j] = hypotenuse;

			//calculate angle of slope from one point to the next using basic trigonometry (theta = sin-1(opposite / hypotenuse))
			arrays.angleArray[currentRow][j] = DEGREES_PER_RADIAN * asin(verticalDist / hypotenuse);

			rowsProcessed++;
			rowsToProcess--;
		}
		currentRow++;
	}

	//return number of rows for which distance and angle calculations have been performed
	return rowsProcessed;
}
