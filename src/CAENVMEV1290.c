/*!
	-----------------------------------------------------------------------------

	-----------------------------------------------------------------------------

	CAENVMEV1290.c

	-----------------------------------------------------------------------------

Created: April 2022
Byungmin Kang, HANUL, korea Univerisity
Functions for V1290
-----------------------------------------------------------------------------
*/
#include "CAENVMEV1290.h"
#include "unistd.h"
#include "tgmath.h"
#include <stdio.h>
#include <iostream>
#include <bitset>
#ifndef CAENVMEV1290_C
#define CAENVMEV1290_C

unsigned short opcode[10];
using namespace std;

void CAENVMEV1290::SetTriggerMode(int mode){
	opcode[0]=mode;
	if(!WriteMICRORegister(1,opcode)){
		std::cout<<"Waring: Failed to set Trigger mode. "<<std::endl;
		exit(1);
	};
}
void CAENVMEV1290::SetWindowWidth( unsigned short width){
	opcode[0]=cv1290TDCWidth;
	opcode[1]=width;
	WriteMICRORegister(2,opcode);
}

void CAENVMEV1290::SetWindowOffset( unsigned short offset){
	opcode[0]=cv1290TDCOffset;
	opcode[1]=offset;
	WriteMICRORegister(2,opcode);
}


void CAENVMEV1290::SetExtraSearchWindow( unsigned short window){
	opcode[0]=cv1290TDCExtra;
	opcode[1]=window;
	WriteMICRORegister(2,opcode);
}
void CAENVMEV1290::SetRejectionMargin( unsigned short margin){
	opcode[0]=cv1290TDCRejMargin;
	opcode[1]=margin;
	WriteMICRORegister(2,opcode);
}


void CAENVMEV1290::SetMultiplicity(unsigned short mt){
	opcode[0]=cv1290SetMultiplicity;
	opcode[1]=0xFFFF&mt;
	WriteMICRORegister(2,opcode);
}

void CAENVMEV1290::SetDetectionEdge(int conf){
	opcode[0]=cv1290SetEdge;
	switch(conf){
		case 0: {//Paired 
			opcode[1]=0x0000;
			break;
			}
		case 1: {//Leading 
			opcode[1]=0x0010;
			break;
			}
		case 2: {//Trailing
			opcode[1]=0x0001;
			break;
			}
		case 3: {//Leading And Trailing
			opcode[1]=0x0011;
			break;
			}
		default: {
			opcode[1]=0x0010;
			break;
			}
	}
	WriteMICRORegister(2,opcode);
}

void CAENVMEV1290::SetHeaderTrailer(bool flag){
	if(flag){
		opcode[0]=cv1290EnHeadTrail;
	}
	else{
		opcode[0]=cv1290DisHeadTrail;
	}
	WriteMICRORegister(1,opcode);
}

void CAENVMEV1290::SetErrorMark(bool flag){
	if(flag){
		opcode[0]=cv1290EnErrMrk;
	}
	else{
		opcode[0]=cv1290DisErrMrk;
	}
	WriteMICRORegister(1,opcode);

}

void CAENVMEV1290::SetTimeTagSubtraction(bool flag){
	if(flag){
		opcode[0]=cv1290EnSubTrg;
	}
	else{
		opcode[0]=cv1290DisSubTrg;
	}
	WriteMICRORegister(1,opcode);
}

void CAENVMEV1290::SetBLTNev(int nev){
	CvWrite16(handle, addr+cv1290BLTEvtNumber,nev);
}



void CAENVMEV1290::Clear(){
	CvWrite16(handle,addr+cv1290Clear,0x0004);
}

void CAENVMEV1290::Reset(){
	CvWrite16(handle,addr+cv1290ModuleReset,0x0004);
}

uint16_t CAENVMEV1290::ReadControlRegister(){
	return (CvRead16(handle, addr + cv1290ControlReg));
}
uint16_t CAENVMEV1290::GetStatRegister(){
	return (CvRead16(handle, addr + cv1290StatReg));
}
uint16_t CAENVMEV1290::IsReady(){
	return (GetStatRegister())&0x01;
}

static int tcnt=0;



uint32_t CAENVMEV1290::GetEventNumber(){
	return CvRead32(handle, addr+cv1290EvtCnt);
}

uint16_t IsFIFOReady(int32_t handle, uint32_t addr){
	//  return (CvRead16(handle, addr + cv1290StatReg1))&0x0001;
	return (CvRead16(handle, addr + cv1290FIFOStat));
}

uint16_t ReadEventFIFOStatus(int32_t handle, uint32_t addr){
	return (CvRead16(handle, addr + cv1290FIFOStat));
}
void EnableEventFIFO(int32_t handle, uint32_t addr){
	uint16_t data= CvRead16(handle, addr + cv1290ControlReg);
	data = data | (0x0001 << 8 );
	//	uint16_t enfifo = 0x100;//100000000
	CvWrite16(handle, addr+cv1290ControlReg,data);
}
uint32_t CAENVMEV1290::AssignBlockedData(uint32_t* Block, int32_t* data){	
		bool TDC_events = true;
		bool print;// = !(tcnt%1000);
		int j=0;
		uint32_t TDCevt_id=0;
		uint32_t bunch_id;
		while(TDC_events&&j<511)
		{
			uint32_t Buf=Block[j];	
			j++;
			switch(((Buf>>27)&(0x1f))  )// check output buffer type
			{
				case 0x08: //Global Header
					{
						TDCevt_id = ((Buf>>5)&0x3fffff);
						print = !(TDCevt_id%1000);
						if(print) std::cout<<hex<<addr<<dec<<"  GH : "<<TDCevt_id<<std::endl;
						break;
					}
				case 0x01: // Header
					{
										if(print)		std::cout<<"TDC Header"<<std::endl;
						//						TDCevt_id = ((Buf>>12)&0xfff);
						bunch_id = (Buf&0xfff);
						//						std::cout<<"Bunch ID : "<<bunch_id<<std::endl;
						break;
					}
				case 0x03: // Trailer
					{
						//						int evt_id = ((Buf>>12)&0xfff);
						int wdcnt = (Buf&0xfff);
						break;
					}
				case 0x00: // valid data
					{
						int chan = (int)((Buf >> 21)&0x1f);
						//	int tdc = (int32_t)(Buf&0x1fffff) - bunch_id;	
						int tdc = (int32_t)(Buf&0x1fffff);	
						if(data[chan]==-9999){
							data[chan]=tdc;
//							if(print)std::cout<<"TDC Hit on CH : "<<chan<<" Value: "<<data[chan]<<std::endl;
						}
						else{
//							std::cout<<"TDC MT Hit on CH : "<<chan<<std::endl;
						}
						break;
					}
				case 0x04: // Error
					{
						int err_flag = (Buf&0x3fff);
						std::bitset<15> eb(err_flag);
						std::cout<<"TDC : " <<hex<<addr<<" Error : "<<eb<<std::endl;
						break;
					}
				case 0x11: //ETTT 
					{
						unsigned int ETTT = (Buf&0x7ffffff);
						break;
					}
				case 0x10: //Output Buf Trailer 
					{
						unsigned short wdcnt = ((Buf>>5)&0xffff);
						TDC_events = false;
						break;
					}
				case 0x18: //dummy 
					{
						TDC_events = false;
//						std::cout<<"Dummy buf"<<std::endl;
						break;
					}
				default:
					{
						printf("Warning, an invalid TDC datum has been detected!\n");
					}
					if(j==511){
						std::cout<<"Warning: TDC loop reached 255"<<std::endl;
					}
			}//Switch
		}//while(TDC_events)
	return TDCevt_id;
	tcnt++;
}

#endif

