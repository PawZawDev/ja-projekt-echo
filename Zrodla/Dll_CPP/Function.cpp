#include "Function.h"

void delayCpp(char* Source, int dataSize, int sampleSize, char* destination, int begin, int end, int delayStep)
{
	
	int16_t tempInput = 0; //sound from input table
	int16_t tempDone1 = 0;  //echoed sound (without incoming input sound)
	int16_t outputSample = 0; //echoed plus input sound
	
	int edge = (dataSize / delayStep);
	for (int i = 0; i < edge; i ++)
	{
		for (int j = begin + (i * delayStep); (j < end + (i * delayStep))&&(j<dataSize); j += sampleSize)
		{
			memcpy(&tempInput, (char*)Source + j, sampleSize);
			memcpy(&tempDone1, (char*)destination+ j, sampleSize);
			tempDone1 = tempDone1 >> 1;
			outputSample = tempInput + tempDone1;
			memcpy((char*)destination + j, &outputSample, sampleSize);
			memcpy((char*)destination + j + delayStep, &outputSample, sampleSize);
		}
	}
}