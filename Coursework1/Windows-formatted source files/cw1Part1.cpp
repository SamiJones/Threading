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

#define DEGREES_PER_RADIAN 57.2958

float** setupMainArray(void); //used for importing and converting data for main array from array.txt and storing it in mainArray
template <typename type> type** setup2DArrayOnHeap(void); //templated function to setup a 2D array on heap (must allocate arrays on heap due to their large size)
template <typename type> void delete2DArray(type** array); //templated function to release memory used for a given 2D array (must be called for each array)

int main()
{
	//create a clock object and set it equal to current processor time used by this process (measured in clock ticks)
	clock_t t = clock();

	//double-pointers used to point to 2D arrays
	//the 2D arrays created have been set up on the heap due to their large size
	float** mainArray = setupMainArray();
	float** distanceArray = setup2DArrayOnHeap<float>();
	float** angleArray = setup2DArrayOnHeap<float>();

	//calculate distance results and populate corresponding array
	for (int i = 0; i < ARRAY_HEIGHT; i++)
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
			float verticalDist = mainArray[i][nextColumn] - mainArray[i][j];

			//Pythagoras' Theorem to calculate Euclidean distance between the points (hypotenuse of the triangle)
			float hypotenuse = sqrt(pow(verticalDist, 2) + pow(HORIZONTAL_POINT_DIST, 2));

			distanceArray[i][j] = hypotenuse;

			//calculate angle of slope from one point to the next using basic trigonometry (theta = sin-1(opposite / hypotenuse))
			angleArray[i][j] = DEGREES_PER_RADIAN * asin(verticalDist / hypotenuse);
		}
	}

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