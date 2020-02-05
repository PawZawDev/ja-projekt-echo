#include <iostream>
#include <string>
#include <windows.h>
#include <fstream>
#include <thread>
#include <vector>

#include "Function.h"

#define QWORD int64_t
#define SDWORD int
#define REAL4 float
extern "C" void _stdcall Delay(QWORD Source, QWORD dataSize, QWORD destination, QWORD myBegin, QWORD myEnd, QWORD delayStep, QWORD iterationLength);
//extern "C" void Delay(QWORD Source, QWORD dataSize, QWORD destination, QWORD myBegin, QWORD myEnd, QWORD delayStep, QWORD iterationLength);


//#define DEBUG true;

int main(int argc, char** argv)
{
	std::string usedDll = "both";

	int ChunkID;//0
	int ChunkSize;//4
	int Format;//8
	int Subchunk1ID;//12
	int Subchunk1Size;//16
	short AudioFormat;//20
	short NumChannels;//22
	int SampleRate;//24
	int ByteRate;//28
	short BlockAlign;//32
	short BitsPerSample;//34
	int Subchunk2ID;//36
	int Subchunk2Size;//40
	int SampleSize;
	int64_t DataSize;
	long SampleCount;

	std::string fileNameIn = "";
	std::string fileOutNameAsm = "";
	std::string fileOutNameCpp = "";
	int threadsNumber = 0;
	unsigned int numberOfTries = 100;
#ifndef DEBUG
	for (int i = 0; i < argc; ++i)
	{
		if (!strcmp(argv[i], "-i"))
		{
			fileNameIn = argv[i + 1];
		}
		if (!strcmp(argv[i], "-t"))
		{
			threadsNumber = atoi(argv[i + 1]);
		}
		if (!strcmp(argv[i], "-x"))
		{
			numberOfTries = atoi(argv[i + 1]);
		}
		if (!strcmp(argv[i], "-d"))
		{
			usedDll = argv[i + 1];
		}
	}
#endif // !DEBUG
#ifdef DEBUG
	fileNameIn = "dwatrzycztery.wav";
#endif // DEBUG

	if (fileNameIn == "") {
		fileNameIn = "dwatrzycztery.wav";
		std::cout << "No input file name" << std::endl;
		std::cout << "default file inserted" << std::endl;
		//return 0;
	}
	double startCpp, stopCpp, startAsm, stopAsm;
	double timeStart, timeStop;
	double* CppRealizationTime = new double[numberOfTries];
	double* AsmRealizationTime = new double[numberOfTries];

	//WAVE waveData;
	fileOutNameAsm = "Asm" + fileNameIn;
	fileOutNameCpp = "Cpp" + fileNameIn;

	std::ifstream fileIn;
	std::ofstream fileOutAsm;
	std::ofstream fileOutCpp;

	try
	{
		fileIn.open(fileNameIn, std::ios::binary);
		fileOutAsm.open(fileOutNameAsm, std::ios::binary);
		fileOutCpp.open(fileOutNameCpp, std::ios::binary);
	}
	catch (const std::exception&)
	{
		std::cout << "Error with files - check if they exist" << threadsNumber << std::endl;
		return 0;
	}

	//READ DATA
	//waveData.read(fileIn);
	fileIn.read((char*)&ChunkID, sizeof(int));
	fileIn.read((char*)&ChunkSize, sizeof(int));
	fileIn.read((char*)&Format, sizeof(int));
	fileIn.read((char*)&Subchunk1ID, sizeof(int));
	fileIn.read((char*)&Subchunk1Size, sizeof(int));
	fileIn.read((char*)&AudioFormat, sizeof(short));
	fileIn.read((char*)&NumChannels, sizeof(short));
	fileIn.read((char*)&SampleRate, sizeof(int));
	fileIn.read((char*)&ByteRate, sizeof(int));
	fileIn.read((char*)&BlockAlign, sizeof(short));
	fileIn.read((char*)&BitsPerSample, sizeof(short));
	fileIn.read((char*)&Subchunk2ID, sizeof(int));
	fileIn.read((char*)&Subchunk2Size, sizeof(int));

	fileIn.seekg(0, fileIn.end);
	DataSize = fileIn.tellg();
	fileIn.seekg(44, fileIn.beg);
	DataSize -= fileIn.tellg();
	SampleSize = BitsPerSample / 8;
	SampleCount = DataSize / SampleSize;

	int delayStep = ByteRate;
	delayStep -= delayStep % 16;

	int delayIterations = (DataSize / delayStep) + 1; //if this doesn't work add -1 , for testing /2

	char* inputData = (char*)malloc(BitsPerSample * DataSize);
	long long newDataSize = DataSize + (4 * delayStep);
	char* outputData = (char*)malloc(BitsPerSample * newDataSize);

	fileIn.read((char*)inputData, DataSize);

	if ((usedDll == "asm")  || (usedDll=="both") || (usedDll=="BOTH") || (usedDll == "ASM") )
	{
		//ASM
		startAsm = clock();
		for (int i = 0; i < numberOfTries; i++)
		{
			for (int i = 0; i < newDataSize; i++) *(outputData + i) = 0;

			std::vector<std::thread>threads;
			timeStart = clock();
			if (threadsNumber < 1 || threadsNumber > std::thread::hardware_concurrency())
			{
				threadsNumber = std::thread::hardware_concurrency();
			}
			for (int j = 0; j < threadsNumber; j++)
			{
				int64_t begin = ((float)delayStep / threadsNumber);
				begin *= j;
				int64_t end = ((float)(delayStep + 1) / threadsNumber);
				end *= j + 1;
				if (threadsNumber == 1) end -= 1;
#ifdef DEBUG
				//if (j != 0) continue;
				//if (j != 1) continue;
				//if (j != 2) continue;
				//if (j != 3) continue;
#endif // DEBUG
				threads.push_back(std::thread(&Delay, (QWORD)inputData, (QWORD)DataSize, (QWORD)outputData, begin, end, delayStep, (end - begin))); //ass /8 bo bajty
				//threads.push_back(std::thread(&delayCpp, (char*)inputData, DataSize, SampleSize, (char*)outputData, begin, end, delayStep));
			}
			for (int j = 0; j < threads.size(); j++)
			{
				threads[j].join();
			}
			timeStop = clock();

			AsmRealizationTime[i] = timeStop - timeStart;
		}
		stopAsm = clock();

		//SAVE ASM
		//waveData.write(fileOutAsm);
		fileOutAsm.write((char*)&ChunkID, sizeof(int));
		fileOutAsm.write((char*)&ChunkSize, sizeof(int));
		fileOutAsm.write((char*)&Format, sizeof(int));
		fileOutAsm.write((char*)&Subchunk1ID, sizeof(int));
		fileOutAsm.write((char*)&Subchunk1Size, sizeof(int));
		fileOutAsm.write((char*)&AudioFormat, sizeof(short));
		fileOutAsm.write((char*)&NumChannels, sizeof(short));
		fileOutAsm.write((char*)&SampleRate, sizeof(int));
		fileOutAsm.write((char*)&ByteRate, sizeof(int));
		fileOutAsm.write((char*)&BlockAlign, sizeof(short));
		fileOutAsm.write((char*)&BitsPerSample, sizeof(short));
		fileOutAsm.write((char*)&Subchunk2ID, sizeof(int));
		fileOutAsm.write((char*)&Subchunk2Size, sizeof(int));

		fileOutAsm.write((char*)outputData, DataSize);
	}



	if ((usedDll == "cpp")  || (usedDll=="both") || (usedDll=="BOTH") || (usedDll == "CPP"))
	{
		//CPP
		startCpp = clock();
		for (int i = 0; i < numberOfTries; i++)
		{
			for (int i = 0; i < newDataSize; i++) *(outputData + i) = 0;

			std::vector<std::thread>threads;
			timeStart = clock();
			if (threadsNumber < 1 || threadsNumber > std::thread::hardware_concurrency())
			{
				threadsNumber = std::thread::hardware_concurrency();
			}
			for (int j = 0; j < threadsNumber; j++)
			{

				int64_t begin = ((float)delayStep / threadsNumber);
				begin *= j;
				int64_t end = ((float)(delayStep + 1) / threadsNumber);
				end *= j + 1;
				if (threadsNumber == 1) end -= 1;

#ifdef DEBUG
				//if (j != 0) continue;
				//if (j != 1) continue;
				//if (j != 2) continue;
				//if (j != 3) continue;
#endif // DEBUG
				threads.push_back(std::thread(&delayCpp, (char*)inputData, DataSize, SampleSize, (char*)outputData, begin, end, delayStep));
			}
			for (int j = 0; j < threads.size(); j++)
			{
				threads[j].join();
			}
			timeStop = clock();
			CppRealizationTime[i] = timeStop - timeStart;
		}
		stopCpp = clock();

		//SAVE CPP
		//waveData.write(fileOutCpp);
		fileOutCpp.write((char*)&ChunkID, sizeof(int));
		fileOutCpp.write((char*)&ChunkSize, sizeof(int));
		fileOutCpp.write((char*)&Format, sizeof(int));
		fileOutCpp.write((char*)&Subchunk1ID, sizeof(int));
		fileOutCpp.write((char*)&Subchunk1Size, sizeof(int));
		fileOutCpp.write((char*)&AudioFormat, sizeof(short));
		fileOutCpp.write((char*)&NumChannels, sizeof(short));
		fileOutCpp.write((char*)&SampleRate, sizeof(int));
		fileOutCpp.write((char*)&ByteRate, sizeof(int));
		fileOutCpp.write((char*)&BlockAlign, sizeof(short));
		fileOutCpp.write((char*)&BitsPerSample, sizeof(short));
		fileOutCpp.write((char*)&Subchunk2ID, sizeof(int));
		fileOutCpp.write((char*)&Subchunk2Size, sizeof(int));

		fileOutCpp.write((char*)outputData, DataSize);
	}


	//RESULTS
	double AsmTimeSum = 0;
	for (int i = 0; i < numberOfTries; i++)
	{
		AsmTimeSum += AsmRealizationTime[i];
	}
	double AsmTimeSumAverage = AsmTimeSum / numberOfTries;

	double CppTimeSum = 0;
	for (int i = 0; i < numberOfTries; i++)
	{
		CppTimeSum += CppRealizationTime[i];
	}
	double CppTimeSumAverage = CppTimeSum/numberOfTries;


	std::cout << "Number of theards: " << threadsNumber << std::endl;
	std::cout << "Number of tries: " << numberOfTries << std::endl;

	if ((usedDll == "asm")  || (usedDll=="both") || (usedDll=="BOTH") || (usedDll == "ASM") )
	{
		std::cout << std::endl << "ASM" << std::endl;
		std::cout << "Sum of times:" << AsmTimeSum * 1000.0 / CLOCKS_PER_SEC << std::endl;
		std::cout << "Average time:" << AsmTimeSumAverage * 1000.0 / CLOCKS_PER_SEC << std::endl;
		std::cout << "Total time: " << (stopAsm - startAsm) * 1000.0 / CLOCKS_PER_SEC << std::endl;
	}

	if ((usedDll == "cpp")  || (usedDll=="both") || (usedDll=="BOTH") || (usedDll == "CPP"))
	{
		std::cout << std::endl << "CPP" << std::endl;
		std::cout << "Sum of times:" << CppTimeSum * 1000.0 / CLOCKS_PER_SEC << std::endl;
		std::cout << "Average time:" << CppTimeSumAverage * 1000.0 / CLOCKS_PER_SEC << std::endl;
		std::cout << "Total time: " << (stopCpp - startCpp) * 1000.0 / CLOCKS_PER_SEC << std::endl;
	}

	
	std::string resultFileName=fileNameIn.substr(0,fileNameIn.length()-4) + " " + std::to_string(threadsNumber) + "t" + " Result.txt";
	std::ofstream resultFile;
	resultFile.open(resultFileName,std::ios::out);

	//save to file
	resultFile << "Number of theards: " << threadsNumber << std::endl;
	resultFile << "Number of tries: " << numberOfTries << std::endl;
	if ((usedDll == "asm")  || (usedDll=="both") || (usedDll=="BOTH") || (usedDll == "ASM") )
	{
		resultFile << std::endl << "ASM" << std::endl;
		resultFile << "Sum of times:" << AsmTimeSum * 1000.0 / CLOCKS_PER_SEC << std::endl;
		resultFile << "Average time:" << AsmTimeSumAverage * 1000.0 / CLOCKS_PER_SEC << std::endl;
		resultFile << "Total time: " << (stopAsm - startAsm) * 1000.0 / CLOCKS_PER_SEC << std::endl;
	}

	if ((usedDll == "cpp")  || (usedDll=="both") || (usedDll=="BOTH") || (usedDll == "CPP"))
	{
		resultFile << std::endl << "CPP" << std::endl;
		resultFile << "Sum of times:" << CppTimeSum * 1000.0 / CLOCKS_PER_SEC << std::endl;
		resultFile << "Average time:" << CppTimeSumAverage * 1000.0 / CLOCKS_PER_SEC << std::endl;
		resultFile << "Total time: " << (stopCpp - startCpp) * 1000.0 / CLOCKS_PER_SEC << std::endl;
	}



	//CLOSE
	resultFile.close();
	fileIn.close();
	fileOutAsm.close();
	fileOutCpp.close();
	free(inputData);
	free(outputData);
	delete[] AsmRealizationTime;
	delete[] CppRealizationTime;

	return 0;
}