/*!
	-----------------------------------------------------------------------------

	-----------------------------------------------------------------------------

	daq.c

	-----------------------------------------------------------------------------

Created: January 2021
Hyunmin Yang, HANUL, korea Univerisity
Simple DAQ program of V792N controlled by V1718 USB-Bridge interface.
Modified: May 2022
Byungmin Kang, HANUL, korea university
V1290 TDC control is available
data is read by block to accelate readout.

-----------------------------------------------------------------------------
*/
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
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
short V4718PID = 18437;
std::string V4718IPAddr = "192.168.1.254";

int min_int(int a,int b){
	if(a>b){
		return b;
	}
	else{
		return a;
	}
}
bool daq_on = false;
// definition of base addresses
const uint32_t qdcAddr1 = 0x20000000;    // base address of first QDC
const uint32_t tdcAddr1 = 0x11000000;    // base address of first TDC
const uint16_t BLTAddress = 0xAA;
// channel number
const uint8_t nQdcCh1 = 16; 
const uint8_t nTdcCh1 = 16; 

CVPulserSelect pulser = cvPulserB;
uint8_t Pulse_period = 0xff;
uint8_t Pulse_width  = 0xff; 

const int timeOut = 0;             // zero if no time out limit(in ms)  
struct timeval tStart, tStop; 
bool Force_flag = true;

// to deal with ctrl + c signal
// When ctrl + c is pressed to kill the process, IntHandler gets called
// instead of killing the process. 
bool isQuit = false;
void IntHandler(int sig)
{
	printf("Quiting the program...\n");
	isQuit = true;
}


void InitModules();
bool WaitModules();
void ClearModules();
void ResetTDC();
void PrintSummary();
int nEvt = 10000;
const int nv1290=10;
const int nv792=10;
CAENVMEV1290 v1290[nv1290];
CAENVMEV792 v792[nv792];
auto loop_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()); 
auto daq_start = duration_cast<seconds>(system_clock::now().time_since_epoch()); 
auto daq_end = duration_cast<seconds>(system_clock::now().time_since_epoch()); 
int timeout = 3000000;//ms
int event_counter=0;

int main(int argc, char **argv)
{

	switch(argc){
		case 1:
			{
				cout<<"Usage: ./daq Filename Nevents"<<endl;
				exit(1);
				break;
			}
		case 2:
			{
				cout<<"Nevents set to 10000"<<endl;
				break;
			}
		case 3:
			{
				nEvt = atoi(argv[2]);
				break;
			}
defalut:
			{
				cout<<"Too much arguments; others will be neglected"<<endl;
				break;
			}
	}
	// CvClose must be called before the end of the program!!!
	signal(SIGINT, IntHandler);// interrupt signal(ctrl + c)
	// initiating V1718
	short* arg = &V4718PID;
//	if (CvInit2(arg, &ctlHdl) != cvSuccess)//For V4718
//	if (CvInit2(&V4718IPAddr, &ctlHdl) != cvSuccess)//For V4718
		if (CvInit(ctlIdx, &ctlHdl) != cvSuccess)//For V1718
			exit(0);
	
		// Initializing modules
	v792[0].SetAddress(ctlHdl,qdcAddr1);
	v792[0].SetNType(false);//true for V792N, false for V792
	v1290[0].SetAddress(ctlHdl,tdcAddr1);
	v1290[0].SetNType(false);//true for V1290N, false for V1290
	
	InitModules();
	printf("Modules Initialized\n"); 
	// acqusition loop
	// gettimeofday(&tStart, NULL);
	ClearModules();
	//	ResetTDC();
	int32_t qdc1[16];
	int32_t tdc1[16];
	uint32_t qdcBuf1;
	uint32_t tdcBuf1;
	const int QDCBlockSize= 256;//int 32 casted by char. Actual number of int32 blocks should be divided by 4.
	uint32_t QDCBlock[QDCBlockSize/4];//Block will be casted to uchar, but this should be uint32_t, for data interpretation.

	const int TDCBlockSize= 1024;
	//	const int TDCBlockSize= 2048;
	//	const int TDCBlockSize= 4096;
	uint32_t TDCBlock[TDCBlockSize/4];

	FILE* fp;
	std::string filename = argv[1];
	string dir = "./dats/";
	filename=dir+filename;
	std::string dfilename=filename+".bin";
	fp=fopen(dfilename.data(),"wb+");
	std::cout<<"File Created : "<<dfilename<<std::endl;
	FILE* fp_time;
	std::string tfilename = filename+"_time.bin";
	fp_time=fopen(tfilename.data(),"wb+");
	std::cout<<"File Created : "<<tfilename<<std::endl;
	int32_t fheader = 0xffffffff;
	int32_t ffooter = 0xfffffffe;

	printf("********************************************************************************************\n");
	printf("************************************ Start of DAQ loop *************************************\n");
	printf("********************************************************************************************\n");
	int32_t QDCevt_id=0,TDCevt_id=0;
	int nerr = 0; 
	for(int32_t i = 0;i < nEvt;i++)//loop
	{
//			Force_flag=true;
		if(isQuit)
			break;
		while(i==0){
			ClearModules();
			uint16_t tdc_clear=(uint16_t)v1290[0].ReadStoredEvents();
			if(tdc_clear==0){
				std::cout<<"Cleared "<<std::endl;
				system_clock::time_point Start_time = system_clock::now(); 
				daq_start = duration_cast<seconds>(system_clock::now().time_since_epoch()); 
				daq_on=true;
				break;
			}
			else{
				std::cout<<"Clearing..."<<std::endl;
				std::cout<<"TDC nev = "<<tdc_clear<<std::endl;
			}	
			if(isQuit){
				return false;
			} 
		}

		loop_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()); 
		system_clock::time_point loop_time_ns = system_clock::now(); 
		if(!WaitModules())
		{
			printf("WaitModules Failure!\n");
			break;
		}
		event_counter++;
		system_clock::time_point start_time = system_clock::now(); 
		// Output Register controll to trigger veto logic

		// QDC1 module data
		for(int j = 0;j < nQdcCh1;j++)
		{
			qdc1[j] = -9999;
		}
		for(int j = 0;j < nTdcCh1;j++)
		{
			tdc1[j] = -9999;
		}

		//		CvStartPulser(ctlHdl,pulser,Pulse_period,Pulse_width);


		int nbq = v792[0].ReadBLT(QDCBlockSize,QDCBlock);//Block Level Transfer
		system_clock::time_point QDCread_time = system_clock::now(); 
		int nbt = v1290[0].ReadMBLT(TDCBlockSize,TDCBlock);
		system_clock::time_point TDCread_time = system_clock::now(); 
		QDCevt_id = v792[0].AssignBlockedData(QDCBlock,qdc1);	
		TDCevt_id = v1290[0].AssignBlockedData(TDCBlock,tdc1);	
		//		CvStopPulser(ctlHdl,pulser);
		//		cout<<"QDC nb : " <<nb<<endl;
		//		std::cout<<"ADC Read"<<std::endl;
		//	for(int j = 0;j < 15;j++) // 16 channels 
		int j=0;
		int bunch_id = 0;
		int nev = i;
		if(TDCevt_id!=QDCevt_id){
			nerr++;	
			cout<<"Warning! TDCevt : "<<TDCevt_id<<" QDCevt : "<<QDCevt_id<<endl;
			//				usleep(1);
							ClearModules();
			i--;
			continue;
		}
		if(i%100==0){
			std::cout<<i<<" th evt"<<std::endl;
			std::cout<<"TDC Evt: "<<TDCevt_id<<std::endl;
			std::cout<<"QDC Evt: "<<QDCevt_id<<std::endl;
			std::cout<<"MissMatch : "<<nerr<<std::endl;
		}
		system_clock::time_point end_time = system_clock::now(); 
		fwrite(&fheader,sizeof (int32_t),1, fp);
		fwrite(&fheader,sizeof (int32_t),1, fp_time);
		fwrite(&i,sizeof (int32_t),1, fp);
		int32_t QDCHeader = 0xFFFFFFF0;
		fwrite(&QDCHeader,sizeof (int32_t),1, fp);
		fwrite(&QDCevt_id,sizeof (int32_t),1, fp);
		for(int32_t nch=0;nch<16;nch++){
			fwrite(&qdc1[nch],sizeof (int32_t),1, fp);
		}
		int32_t TDCHeader = 0xFFFFFFF1;
		fwrite(&TDCHeader,sizeof (int32_t),1, fp);
		fwrite(&TDCevt_id,sizeof (int32_t),1, fp);
		for(int32_t nch=0;nch<16;nch++){
			fwrite(&tdc1[nch],sizeof (int32_t),1, fp);
		}
		fwrite(&ffooter,sizeof (int32_t),1, fp);
		fwrite(&ffooter,sizeof (int32_t),1, fp_time);
		system_clock::time_point write_time = system_clock::now();
		nanoseconds start = start_time-loop_time_ns;//Trigger Waiting time
		nanoseconds qdcreading = QDCread_time-start_time;
		nanoseconds tdcreading = TDCread_time-start_time;
		nanoseconds ending = end_time-start_time;
		nanoseconds writing = write_time-start_time;
		uint32_t lt = start.count();
		uint32_t qt = qdcreading.count();
		uint32_t tt= tdcreading.count();
		uint32_t et= ending.count();
		uint32_t wt= writing.count();
		fwrite(&lt,sizeof (int32_t),1, fp_time);
		fwrite(&qt,sizeof (int32_t),1, fp_time);
		fwrite(&tt,sizeof (int32_t),1, fp_time);
		fwrite(&et,sizeof (int32_t),1, fp_time);
		fwrite(&wt,sizeof (int32_t),1, fp_time);
		/*
			 cout<<"Elapsed_Time"<<endl;
			 cout<<"ReadingQDC : " <<qt<<" ns"<<endl;
			 cout<<"ReadingTDC : " <<tt<<" ns"<<endl;
			 cout<<"Ending : " <<et<<" ns"<<endl;
			 cout<<"Writing : " <<wt<<" ns"<<endl;
			 */
		//ClearModules();
		//			std::cout<<"Time Difference : "<<(double)(tdc1[2]-tdc1[3])*0.025<<std::endl;
		//			std::cout<<"QDC : "<<(double)(qdc1[2])<<std::endl;
		//		std::cout<<"End Of Event "<<i<<" QDCID: "<< QDCevt_id<<" TDCID: "<<TDCevt_id <<std::endl;
	}
	daq_end = duration_cast<seconds>(system_clock::now().time_since_epoch()); 
	int32_t feof = 0xfffffffd;
	fwrite(&feof,sizeof (int32_t),1, fp);
	fwrite(&feof,sizeof (int32_t),1, fp_time);

	seconds elapsed_time = daq_end-daq_start;
	int32_t elapsedtime=elapsed_time.count();
	int32_t nevt = event_counter;	
	fwrite(&elapsedtime,sizeof (int32_t),1, fp);
	fwrite(&nevt,sizeof (int32_t),1, fp);
	std::cout<<"MissMatch : "<<nerr<<std::endl;
	printf("********************************************************************************************\n");
	printf("************************************ End of DAQ loop ***************************************\n");
	printf("************************************ Summary         ***************************************\n");
	//  gettimeofday(&tStop, NULL);
	PrintSummary();
	printf("********************************************************************************************\n");
	// closing V1718
	ClearModules();
	if (CvClose(ctlHdl) != cvSuccess)
		exit(0);
	fclose(fp);
	fclose(fp_time);
	std::cout<<"File Closed : "<<filename<<std::endl;
	return 0;
}
//inni
void InitModules()
{
	// QDC initialization
	v792[0].Write16(cv792BitSet1,0x0080);
	v792[0].Write16(cv792BitClr1,0x0080);
	v792[0].SetPedestal(0xFF);
	v792[0].Write16(cv792BitSet2,0x001C);
	v792[0].Write16(cv792CtlReg1,0x0004);
	std::cout<<"v792 Initialized"<<std::endl;


	// TDC initialization
	unsigned short TriggerOffset = -9;//In units of 25 ns.
	unsigned short TriggerWindow = 9;//In units of 25 ns.
	//				unsigned short code[2];
	//				code[0]=cv1290TrgMatch;
	std::cout<<"Setting TDC Mode"<<std::endl;
	v1290[0].SetTriggerMode(cv1290TrgMatch);
	//	v1290[0].SetTriggerMode(cv1290ContStor);
	v1290[0].SetWindowWidth(TriggerWindow);
	v1290[0].SetWindowOffset(TriggerOffset);
	v1290[0].SetRejectionMargin((unsigned short)0x27);
	v1290[0].SetExtraSearchWindow((unsigned short)0x01);
	v1290[0].SetTimeTagSubtraction(true);	
	unsigned short mt = 6;//0->0, 1->1... 4->8...	8->128, 9->no limit
	v1290[0].SetMultiplicity(mt);	
	v1290[0].SetBLTNev(1);	

}

bool WaitModules()//gate
{

	struct timeval tPrev, tNew;
	uint16_t adcstat = 0;
	bool adcready = false;

	bool adcbusy =false;
	bool adctrg ;
	uint16_t tdcready = 0;
	while(true)
	{
		//system_clock::time_point wait_times = duration_cast<milliseconds>(system_clock::now()); 
		//		auto wait_times = duration_cast<milliseconds>(system_clock::now().time_since_epoch()); 
		//		milliseconds wait_time=wait_times-loop_time;
		//		int waittime=wait_time.count();
		if(isQuit){
			ResetTDC();
			return false;
		}
		/*
			 if(waittime>timeout){
			 ClearModules();
			 cout<<"TimeOut! Modules cleared"<<endl;
			 loop_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()); 
			 }*/
		adcstat = v792[0].GetStatRegister();
		adctrg = adcstat&0x0001;
		adcbusy = (adcstat>>2)&0x0001;
		adcready = (adctrg);	
		tdcready = (v1290[0].IsReady());

		if(v1290[0].ReadStoredEvents()>32&&adcbusy){
		//	ClearModules();
		}

		if(adcbusy&&tdcready!=0x0001){
		}

		if(adcready==0x0001&&tdcready==0x0001&&Force_flag)
	//	if(adcready==0x0001&&tdcready==0x0001)
			return true;
		else if(adcready!=0x0001&&tdcready==0x0001){
		}
	} 
}

void ClearModules()//clr
{
	//Force_flag=false;
	v1290[0].Clear();
	CvWrite16(ctlHdl, qdcAddr1 + cv792BitSet2, 0x001C);
	CvWrite16(ctlHdl, qdcAddr1 + cv792BitClr2, 0x8004);
	v792[0].EventReset();
	usleep(1);
//	Force_flag=true;
}
void ResetTDC(){
	v1290[0].Clear();
	v1290[0].Reset();
}


void PrintSummary()
{
	seconds elapsed_time = daq_end-daq_start;
	int elapsedtime=elapsed_time.count();
	if(elapsedtime!=0){
		double freq = (double)event_counter/elapsedtime;
		cout<<"DAQ Frequency : "<<freq<<" Hz"<<endl;
		cout<<"Elapsed time : "<<elapsedtime<<" seconds"<<endl;
	}
	else{
		cout<<"DAQ time less than 1 s!"<<endl;
	}
}
