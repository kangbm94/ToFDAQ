/*!
	-----------------------------------------------------------------------------

	-----------------------------------------------------------------------------

	test.c

	-----------------------------------------------------------------------------


-----------------------------------------------------------------------------
*/
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include "CAENVMEV895.h"
#include "CAENVMEV792.h"
#include "CAENVMEV1290.h"
#include "CAENVMElib.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <bitset>
#include <vector>
#include <chrono>
/* #include "CAENVMEoslib.h"
#include "CAENVMEtypes.h
*/
using namespace std;
using namespace chrono;
// global variables for a controller and modules
int32_t ctlHdl;                    // controller handler
short ctlIdx = 0;                  // controller slot index???

int min_int(int a,int b){
	if(a>b){
		return b;
	}
	else{
		return a;
	}
}
const uint32_t ledAddr1 = 0xFFFF0000;
const uint32_t tdcAddr1 = 0x11000000;    // base address of first TDC
// channel number



// to deal with ctrl + c signal
// When ctrl + c is pressed to kill the process, IntHandler gets called
// instead of killing the process. 
bool isQuit = false;
void IntHandler(int sig)
{
	printf("Quiting the program...\n");
	isQuit = true;
}
string thfile = "threshold.txt";

void HandleArguments(int argc, char** argv);
void InitModules();
bool WaitModules();
void ClearModules();
void PrintSummary();
const int nv895=10;
const int nv1290=10;
CAENVMEV895 v895[nv895];
CAENVMEV1290 v1290[nv1290];
auto loop_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()); 
auto daq_start = duration_cast<seconds>(system_clock::now().time_since_epoch()); 
auto daq_end = duration_cast<seconds>(system_clock::now().time_since_epoch()); 
int timeout = 3000000;//ms
int event_counter=0;

int main(int argc, char **argv)
{

	
	HandleArguments(argc,argv);
	// CvClose must be called before the end of the program!!!
	signal(SIGINT, IntHandler);// interrupt signal(ctrl + c)
		if (CvInit(ctlIdx, &ctlHdl) != cvSuccess)//For V1718
			exit(0);
	
	v895[0].SetAddress(ctlHdl,ledAddr1);
	v1290[0].SetAddress(ctlHdl,tdcAddr1);
	v1290[0].SetNType(false);//true for V1290N, false for V1290
		
	InitModules();
	printf("Modules Initialized\n"); 
	// acqusition loop
	// gettimeofday(&tStart, NULL);
	ClearModules();
	//	ResetTDC();
	while(!isQuit){
		if(!WaitModules()){
			std::cout<<"Break!"<<std::endl;
		};
			
	}
	printf("************************************ Summary         ***************************************\n");
	//  gettimeofday(&tStop, NULL);
	PrintSummary();
	printf("********************************************************************************************\n");
	// closing V1718
	ClearModules();
	if (CvClose(ctlHdl) != cvSuccess)
		exit(0);
	return 0;
}
//inni
void InitModules()
{
	// LED initialization
	v895[0].LoadThreshold(thfile);
	//	v895[0].SetThreshold(30);
	v1290[0].SetTriggerMode(cv1290ContStor);
	unsigned short mt = 6;//0->0, 1->1... 4->8...	8->128, 9->no limit
	v1290[0].SetMultiplicity(mt);	
	v1290[0].SetBLTNev(1);	

}

bool WaitModules()//gate
{
	while(true){
		if(isQuit){
			return false;
		}
		uint16_t tdcready = (v1290[0].IsReady());
		if(tdcready==0x0001){
			return true;
		}
	}
}

void ClearModules()//clr
{
	//Force_flag=false;
	v895[0].Clear();
	v1290[0].Clear();
	v1290[0].Reset();
//	Force_flag=true;
}


void PrintSummary()
{
}
void HandleArguments(int argc,char**argv){
	switch(argc){
		case 1:
			{
				cout<<"Usage: ./test dum"<<endl;
				exit(1);
				break;
			}
		case 2:
			{
				break;
			}
		case 3:
			{
				break;
			}
defalut:
			{
				break;
			}
	}

}
