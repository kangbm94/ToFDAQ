/*!
	-----------------------------------------------------------------------------

	-----------------------------------------------------------------------------

	CAENVMEV895.c

	-----------------------------------------------------------------------------

Created: April 2022
Byungmin Kang, HANUL, korea Univerisity
Functions for V895
-----------------------------------------------------------------------------
*/
#include "CAENVMEV895.h"
void CAENVMEV895::SetThreshold(int ch, uint16_t val){
	CvWrite16(handle,Channel(ch),val);	
}
void CAENVMEV895::SetThreshold(uint16_t val){
	for(int i=0;i<16;++i){
		SetThreshold(i,val);
	}
}
void CAENVMEV895::LoadThreshold(string filename){
	ifstream file(filename.data());
	string line;
	int i=0;
	while(getline(file,line)){
		Threshold_map[i]=stoi(line);
		SetThreshold(i,Threshold_map[i]);
		cout<<dec<<"Channel "<<i<<" Threshold : "<<Threshold_map[i]<<endl;
		++i;
	}
	file.close();
}


void CAENVMEV895::SetOutputWidth(uint16_t val,int conf){
	if(!conf){
		CvWrite16(handle,addr+cv895OutputWidthA,val);	
	}
	else{
		CvWrite16(handle,addr+cv895OutputWidthB,val);	
	}
}
void CAENVMEV895::SetMajority(int maj){
		uint16_t val=(maj*50-23)/4;
		CvWrite16(handle,addr+cv895MajorityThreshold,val);	
}

void CAENVMEV895::ActivateChannels(){
	CvWrite16(handle,addr+cv895PatternInhibit,0xffff);
}









