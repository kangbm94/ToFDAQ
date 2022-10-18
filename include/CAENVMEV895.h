/*!
	-----------------------------------------------------------------------------

	-----------------------------------------------------------------------------

	CAENVMEV895.h

	-----------------------------------------------------------------------------

Created: September 2022 Byungmin Kang, HANUL, korea Univerisity Class to control V895 Leading Edge Discriminator.  ----------------------------------------------------------------------------- */
#ifndef __CAENVMEV895_H
#define __CAENVMEV895_H
#include <stdlib.h>
#include <stdint.h>
#include "CAENVMEModule.h"
#include <iostream>
#include <fstream>
using namespace std;
class CAENVMEV895:public CAENVMEModule{
	private:
		int Threshold_map[16];
	public:
		CAENVMEV895(){
			name += "V895";
		}
		~CAENVMEV895(){};
		uint32_t Channel(int ch){
//			cout<<hex<<addr+2*ch<<dec<<endl;
			return addr+2*ch;
		}
		void SetThreshold(int ch,uint16_t val);
		void SetThreshold(uint16_t val);
		void LoadThreshold(string filename);
		
		void SetOutputWidth(uint16_t val, int conf);//00-07: 0, 08-15: 1
		void SetMajority(int maj);
		void ActivateChannels();
};



enum V895_ADDR{
	cv895OutputWidthA = 0x40,//From ch 00 - 07 
	cv895OutputWidthB = 0x42,//From ch 08 - 15
	cv895MajorityThreshold = 0x48,
	cv895PatternInhibit = 0x4A,
	cv895TestPulse = 0x4c
};


#endif
