/*
	Author : Majd Ewawi (177535)

	Last modified : 12/5/2020

	This is Operating Systems project part 2

	Simulate a paging memory manger

*/

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include<ctime>
#include <stdio.h>
#include <time.h>

using namespace std;

// struct represent process contains it's information
// process have : id , arrival time , burst time and size

struct Process {
	int id;
	int burst_time;
	int arrival_time;
	int size;

	bool operator ==(Process x) {
		return
			x.id == id && x.arrival_time == arrival_time && x.burst_time == burst_time && x.size == size;
	}
};

// struct represent basic information 

struct BasicInformation {
	int physicalMemorySize;
	int frameSize;
	int roundRobinQuantum;
	int contextSwitch;
};

// struce represent memory status
struct PhysicalMemory {
	int frame;
	bool isFull;
};

// struct represent physical memory process page table

struct PageTable {
	int process_id;
	int* pagesMapping;
	int numberOfPages;
};

/******************************************* Read processes form file ****************************************************/

// function that reads process file line by line to get processes data

vector<string> readProcessFile(string fileName) {
	ifstream inFile;

	// open file that contains process information 
	inFile.open("./../../" + fileName);

	// check if the file exists
	if (!inFile) {
		cout << "Unable to open file " + fileName;
		exit(1);		 // call system to stop
	}

	string line;
	vector<string> fileDataArray;

	// read all information from process.txt
	while (getline(inFile, line)) {
		fileDataArray.push_back(line);
	}

	// close process file
	inFile.close();

	return fileDataArray;
}

// function that takes string which we read form the file and transform it 
// into struct of Process
// this function return a vector of type Process

vector<Process> createProcesses(string fileName) {

	vector<Process> processInfo;
	vector <string> fileDataArray = readProcessFile(fileName);

	// get the information of each process , split data and put them in vector
	for (int i = 0; i < fileDataArray.size(); i++) {

		// iterate data until reach processes information

		if (fileDataArray.at(i).find("process info:") != string::npos) {
			for (int j = i + 1; j < fileDataArray.size(); j++) {

				// get each line from process file , each line represent process info
				string line = fileDataArray.at(j);

				// split each line to spearte data into
				// process data contains [process id , arrival time ,burst time , size ]

				vector<string> v;
				stringstream ss(line);
				while (ss.good()) {
					string substr;
					getline(ss, substr, ' ');
					v.push_back(substr);
				}

				// transform from string information into numbers

				vector<int> processData;
				for (int i = 0; i < v.size(); i++) {
					stringstream ss(v[i]);
					int number = 0;
					ss >> number;
					processData.push_back(number);
				}

				// prepare process info to add it into the struct
				Process  process;
				process.id = processData[0];
				process.arrival_time = processData[1];
				process.burst_time = processData[2];
				process.size = processData[3];


				// add process data vector into all processes information vector
				processInfo.push_back(process);
			}
		}

	}
	return processInfo;

}
// function that reads basic information form proces file
// this function returns struct of type basic information

BasicInformation readCPUInformation(string fileName) {

	// read process file
	vector<string> fileDataArray = readProcessFile(fileName);
	vector<int> numbers;


	// transform text into integers represent CPU information
	for (int i = 0; i < 4; i++) {
		stringstream ss(fileDataArray.at(i));
		int number;
		ss >> number;
		numbers.push_back(number);
	}

	// create struct represent basic information 
	// assign data into the fields

	BasicInformation info;
	info.physicalMemorySize = numbers.at(0);
	info.frameSize = numbers.at(1);
	info.roundRobinQuantum = numbers.at(2);
	info.contextSwitch = numbers.at(3);

	return info;
}

/****************************** Printing section ********************************************************/

// function that takes processes vector and display them
void printProcessesData(vector<Process> processes) {

	// iterate through processes to print all fields
	cout << "Processes :" << endl;
	for (int i = 0; i < processes.size(); i++) {
		cout << "id: " << processes[i].id << " ";
		cout << "arrival time : " << processes[i].arrival_time << " ";
		cout << "burst time : " << processes[i].burst_time << " ";
		cout << "size : " << processes[i].size << " ";
		cout << endl;
	}
	cout << endl;
}

// function that print basic information 

void printCPUData(BasicInformation info) {

	// iterate through struct of basic information read fields
	cout << "Basic information :" << endl;
	cout << "Physical memory size : " << info.physicalMemorySize << endl;
	cout << "Frame size :" << info.frameSize << endl;
	cout << "Round Robin Quantum : " << info.roundRobinQuantum << endl;
	cout << "Context Switch : " << info.contextSwitch << endl;
	cout << endl;
}

/******************************************** Paging Simulation ***************************************************/


// function that calculates number of frames for each process
// Number of frames = Physical Address space / Frame size;

int calculateNumberOfFrames( BasicInformation data) {
	int physicalAdressSize = data.physicalMemorySize ;
	int numberOfFrames = physicalAdressSize / data.frameSize;
	return  numberOfFrames;
}

// function that calculates number of pages for each process
// Number of pages = logical Address space / Frame size;
// size of page  = size of frame

int calculateNumberOfPages(Process process ,  BasicInformation data ) {
	int pageSize = data.frameSize;
	int logicalAddressSpace = process.size;
	int numberOfPages = logicalAddressSpace / pageSize;
	return  numberOfPages;
}


// function that generates array contains different values used to map page to frame

int*  generateRandomValues(int end ) {
	int i;
	int j;
	int* array = new int [end];
	int hasDuplicate;

	// this function is used to change cpu time to prevent generting the same value

	srand(time(NULL));

	for (i = 0; i < end; i++)
	{
		array[i] = rand() % end;

		 // check if there is duplicate values to populate it

		hasDuplicate = 0;

		for (j = 0; j <= (i - 1); j++)
		{
			if (array[i] == array[j])
			{
				hasDuplicate = 1;
				i--;
			}

		}
	}

	return array;

}

// function that create page tables for all processes represent mapping between the frame and the page

vector <PageTable> createPageTable(vector <Process> processes , BasicInformation CPU_data ) {
	vector <PageTable> tables;

	// calculate the number of frames in physical memory
	int numberOfFrames = calculateNumberOfFrames(CPU_data);

	// generte frames , pages mapping
	int* framesMapping = generateRandomValues(numberOfFrames);

	// number represnt free frames in physical memory
	int numberOfFreeFrames = numberOfFrames;

	// this number represent the start index of free frames in the phsical memory
	int freeFramesStart = 0;

	for (int i = 0; i < processes.size(); i++) {
		

		// calculate number of frames for process
		int numberOfPages = calculateNumberOfPages(processes.at(i), CPU_data);

		// create array with pages size represent page Table ,  page table map page to frame
		// [page : index , frame : value ]

		int* pageTable = new int[numberOfPages];

		// generate a random values [frames] to fill page table 
		// perform mapping between page and the  frame we get from array of random values

		if (numberOfPages > numberOfFreeFrames) {

			cout << "\nProcess : " << processes.at(i).id << " It is not fit to memory ! " << endl;
		}
		else {
			
			// if number of pages < number of frames all pages are mapping to frames
			// generete random sequence of frames for mapping with number of  pages size

			for (int i = 0; i < numberOfPages; i++) {
				pageTable[i] = framesMapping[i+ freeFramesStart];
			}

			// when page allocated decrease number of free frames
			numberOfFreeFrames -= numberOfPages;

			// update index where free frames starts
			freeFramesStart += numberOfPages;

			// create page table aadded it to the vector 
			// page table represent page table mapping , number of pages , process id

			PageTable item;
			item.numberOfPages = numberOfPages;
			item.pagesMapping = pageTable;
			item.process_id = processes.at(i).id;
			tables.push_back(item);

		}

	}

	return tables;
}



// function print page table for seqeunce of processes

void printPageTableAllProcesses( vector <PageTable> processesPageTables) {

	// print page table for all processes we read from the file

	for (int i = 0; i < processesPageTables.size(); i++) {
		
		// get the value of ith page table
		PageTable pageTable= processesPageTables.at(i);

		// print the data of page table , process id , number of pages , mapping
		cout << "Process : " <<pageTable .process_id << " takes : " << pageTable.numberOfPages << " pages\n" << endl;
		for (int i = 0; i < pageTable.numberOfPages; i++)
			cout << "Page " << i << " -----> Maps to frame : " << pageTable.pagesMapping[i] << endl;

		cout << "-----------------------------------------------------------------\n";
	}

}

// function that calculates totalNumberOf pages taken by all processes

int calculateNumberOfTotalPages(vector <PageTable> pageTables) {
	int numberOfTotalPages = 0;


	// calculate the number of pages for all processes
	for (int i = 0; i < pageTables.size(); i++) {
		numberOfTotalPages += pageTables.at(i).numberOfPages;
	}

	return numberOfTotalPages;
}

// function that intialize physical memory
// returns vector represent frames and their status

int* constructPhysicalMemory(vector <PageTable> pageTables ,BasicInformation info) {
	int numberOfTotalPages = calculateNumberOfTotalPages(pageTables);

	// map every page for all processes t it's frame , showing which frame is taken

	int*  pages = new int[numberOfTotalPages];
	int count = 0;
	for (int i = 0; i < pageTables.size(); i++) {
		for (int j = 0; j < pageTables.at(i).numberOfPages; j++) {
			pages[count] = pageTables.at(i).pagesMapping[j];
			count++;
		}
	}

	return pages;
}

// function that print information about physical memory 
// print frames and their status [full or free]

void printPhysicalMemoryStatus(int* pages ,int numberOfTotalPages , BasicInformation info) {

	cout << "Physical Memory : \n";
	cout << "Number of frames : " << calculateNumberOfFrames(info) << endl;;

	for (int i = 0; i < numberOfTotalPages; i++) {
		cout << "Page : " << i << " maps to  ----------->  frame : " << pages[i] << "in the physical Memory " << endl;
	}
}

//function that takes logical memory address and returns corsponding physical memory address

void  calculatePhysicalMemoryAddress(int* pages , int numberOfTotalPages , int logicalMemoryAddress , BasicInformation info ) {

	int pageSize = info.frameSize;

	int pageNumber = logicalMemoryAddress / pageSize;

	cout << "This logical address belongs to page : " << pageNumber << endl;
	int word = logicalMemoryAddress % pageSize;

	cout << "Word is : " << word << endl;

	int frameNumber =pages[pageNumber];

	cout << "allocated in frame : " << frameNumber << endl;

	int physicalMemoryAddress = frameNumber * pageSize + word;

	cout << "Physical memory Address : " << physicalMemoryAddress << endl;

}

int main() {

	// variables declaration

	string fileName = "test3.txt";

	// read processes from file and return vector of processes to apply algorithms
	vector<Process> processes = createProcesses(fileName);
	BasicInformation info = readCPUInformation(fileName);


	// print data we read form the file

	printCPUData(info);
	printProcessesData(processes);


	// generate page table for all processes
	vector <PageTable>pageTables =  createPageTable(processes, info);
	printPageTableAllProcesses(pageTables);

	//printPageTableAllProcesses(processes, info);

	int*  physicalMemory = constructPhysicalMemory(pageTables, info);
	int numberOfTotalPages = calculateNumberOfTotalPages(pageTables);
	printPhysicalMemoryStatus(physicalMemory,numberOfTotalPages , info);

	int logicalMemoryAddress;

	int maxLogicalAddress = numberOfTotalPages * info.frameSize;

	cout << "\n Enter a logical addess between 0 and " << maxLogicalAddress << " : " << endl;

	cin >> logicalMemoryAddress;

	if (logicalMemoryAddress >= maxLogicalAddress || logicalMemoryAddress < 0)
		cout << "You entered invalid logical memory address !! \n";
	else {
		calculatePhysicalMemoryAddress(physicalMemory, numberOfTotalPages, logicalMemoryAddress, info);
	}

	system("pause");	
	return 0;
}