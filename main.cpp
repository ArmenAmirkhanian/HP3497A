#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <exception>
#include <stdlib.h>

//Make serial port handle global
HANDLE HP3497A;
DCB HP3497AParams = {0};

double GetVoltage(char command[]){
	char received[15] = {0};
	DWORD bytesWrite = 0;
	DWORD bytesRead = 0;

	WriteFile(HP3497A,command,5,&bytesWrite,NULL);
	Sleep(250);
	ReadFile(HP3497A,received,15,&bytesRead,NULL);
	Sleep(10);

	return atof(received);
}

void InitializeSerialPort(){

	// COM port should be changed accordingly for each new system
	HP3497A = CreateFile(L"COM1",GENERIC_READ | GENERIC_WRITE, 0,0, OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	HP3497AParams.DCBlength = sizeof(HP3497AParams);

	// Do not change unless the HP3497A DIP switches have been altered
	GetCommState(HP3497A, &HP3497AParams);
	HP3497AParams.BaudRate = CBR_9600; 
	HP3497AParams.ByteSize = 8;
	HP3497AParams.StopBits = 1;
	HP3497AParams.Parity = ODDPARITY;

	COMMTIMEOUTS timeouts={0};
	timeouts.ReadIntervalTimeout = 2000;
	timeouts.ReadTotalTimeoutConstant = 2000;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 2000;
	timeouts.WriteTotalTimeoutMultiplier = 10;
}

void DisplayHP3497AParameters(){

		std::cout << "  HP3497A COM PARAMETERS\n\n";
		std::cout << "------Current Settings------\n";
		std::cout << "Baud Rate: " << HP3497AParams.BaudRate << "\n";
		std::cout << "Byte Size: " << (int)HP3497AParams.ByteSize << "\n";
		std::cout << "Stop Bits: " << (int)HP3497AParams.StopBits << "\n";
		std::cout << "   Parity: " << (int)HP3497AParams.Parity << "\n";
		std::cout << "----------------------------\n\n\n";
}

void GetSetHeader(std::ofstream& output_file, std::string file_name, int num_channels){
	try{
		output_file.open(file_name);
	}
	catch(std::ofstream::failure e){
		std::cout << "File creation failed. Ensure you have write permission in this directory!\n";
		exit(EXIT_FAILURE);
	}

	output_file << "HP3497A Strain Output Program\n";
	output_file << "Coding by Armen Amirkhanian\n";
	output_file << "All strain values are calculated as unreferenced values. You have to manually reference them to an unstrained state if desired.\n";
	output_file << "NOT MEANT FOR HIGH-SPEED/DYNAMIC MEAUREMENTS\n";
	output_file << "Column headers correspond to channel designations in the 3497A. Refer to instruction manual for clarification.\n";
	output_file << "  HP3497A COM PARAMETERS\n\n";
	output_file << "------Current Settings------\n";
	output_file << "Baud Rate: " << HP3497AParams.BaudRate << "\n";
	output_file << "Byte Size: " << (int)HP3497AParams.ByteSize << "\n";
	output_file << "Stop Bits: " << (int)HP3497AParams.StopBits << "\n";
	output_file << "   Parity: " << (int)HP3497AParams.Parity << "\n";
	output_file << "----------------------------\n\n\n";
	output_file << "Approximate voltage of Slot 0 Excitation: " << GetVoltage("AI10\r") << "\n";
	output_file << "Approximate voltage of Slot 1 Excitation: " << GetVoltage("AI30\r") << "\n";
	output_file << "Approximate voltage of Slot 2 Excitation: " << GetVoltage("AI50\r") << "\n";
	output_file << "Reading\tVs(10)\tVg(0)\tVs(10)\tVg(1)\tVs(10)\tVg(2)\tVs(10)\tVg(3)\tVs(10)\tVg(4)\tVs(10)\tVg(5)\tVs(10)\tVg(6)\tVs(10)\tVg(7)\tVs(10)\tVg(8)\tVs(10)\tVg(9)\tVs(30)\tVg(20)\tVs(30)\tVg(21)\tVs(30)\tVg(22)\tVs(30)\tVg(23)\tVs(30)\tVg(24)\tVs(30)\tVg(25)\tVs(50)\tVg(40)\tVs(50)\tVg(41)\n";

}

int main(){
	
	std::ofstream output_file;

	int loadcal = 0;

	InitializeSerialPort();
	DisplayHP3497AParameters();

	double Ve[10];
	double Vss[10];

	if(loadcal == 1){
		/*for(int i=0;i<10;i++){
		Ve[i] = GetVoltage("AI50\r");
		Vss[i] = GetVoltage("AI41\r");
		}
		double Vout_avg = (Ve[0]*Vss[0]+Ve[1]*Vss[1]+Ve[2]*Vss[2]+Ve[3]*Vss[3]+Ve[4]*Vss[4]+Ve[5]*Vss[5]+Ve[6]*Vss[6]+Ve[7]*Vss[7]+Ve[8]*Vss[8]+Ve[9]*Vss[9])/10;
		std::cout << Vout_avg;
		std::cin >> loadcal;*/
		while (loadcal<10){
			
			double LC1 = GetVoltage("AI40\r");
			double LC2 = GetVoltage("AI41\r");
			double V = GetVoltage("AI50\r");
			system("cls");
			std::cout << (521141*LC1*V-3506.1) << "\t" << (-570758*LC2*V+4298.2);
		}
		CloseHandle(HP3497A);
		exit(0);
	}

	std::string file_name;
	double numData;
	int dummy = 0;

	std::cout << "Enter a filename: ";
	std::cin >> file_name;
	std::cout << "Record for how many days? ";
	std::cin >> numData;

	numData = numData * 24 * 60;

	GetSetHeader(output_file,file_name,dummy);
	int numChan = 18;

	double *Vs = new double[numChan];
	double *Vg = new double[numChan];
	char *strchn[] = {"AI0\r", "AI1\r", "AI2\r", "AI3\r", "AI4\r", "AI5\r", "AI6\r", "AI7\r", "AI8\r", "AI9\r", "AI20\r", "AI21\r", "AI22\r", "AI23\r", "AI24\r", "AI25\r", "AI40\r", "AI41\r" };
	char *strvlt[] = { "AI10\r", "AI30\r", "AI50\r" };
	char **cyc;
	size_t n = sizeof strchn / sizeof *strchn;
	int helper = 0;
	int indexer = 0;

	for(int i = 0;i<numData;i++){
		indexer = 0;
		helper = 0;
		for (cyc = strchn; cyc <= &strchn[n - 1];cyc++){
			if (helper < 10){
				Vs[indexer] = GetVoltage(strvlt[0]);
				Vg[indexer] = GetVoltage(*cyc);
			}
			else if (helper > 9 && helper < 16){
				Vs[indexer] = GetVoltage(strvlt[1]);
				Vg[indexer] = GetVoltage(*cyc);
			}
			else if (helper > 15){
				Vs[indexer] = GetVoltage(strvlt[2]);
				Vg[indexer] = GetVoltage(*cyc);
			}
			helper++;
			indexer++;
		}

		output_file << i+1 << "\t";
		for (int j = 0; j < numChan; j++){
			output_file << Vs[j] << "\t" << Vg[j] << "\t";
		}
		output_file << "\n";
		Sleep(60000);
	}
	
	CloseHandle(HP3497A);

	
}
