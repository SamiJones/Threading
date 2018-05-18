#include <fstream>
#include <time.h>
#include <cstdlib>

using namespace std;

#define ARRAY_WIDTH 1000
#define ARRAY_HEIGHT 50000

int main ()
{
	ofstream outFile;
	outFile.open("array.txt", ios::trunc);

	int randomNumber[2];

	srand(time(NULL));

	for (int i = 0; i < ARRAY_HEIGHT; i++)
	{
		for (int j = 0; j < ARRAY_WIDTH; j++)
		{
			randomNumber[0] = rand() % 999 + 1;
			randomNumber[1] = rand() % 999 + 1;

			outFile << randomNumber[0] << "." << randomNumber[1] << " ";
		}

		outFile << "\n";
	}

	outFile.close();

	return 0;
}
