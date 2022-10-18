/*!
	-----------------------------------------------------------------------------

	-----------------------------------------------------------------------------

	CAENVMEV792.c

	-----------------------------------------------------------------------------

Created: April 2022
Byungmin Kang, HANUL, korea Univerisity
Functions for V792
-----------------------------------------------------------------------------
*/
#include "CAENVMEV792.h"
#include <stdio.h>
#include <iostream>
#include <bitset>
#ifndef CAENVMEV792_c
#define CAENVMEV792_c
void CAENVMEV792::Write16(uint32_t regaddr, uint16_t data){
	CvWrite16(handle,  addr+regaddr, data);
}
void CAENVMEV792::Write32(uint32_t regaddr, uint32_t data){
	CvWrite32(handle, addr+regaddr, data);
}
uint16_t CAENVMEV792::Read16(uint32_t regaddr){
	return CvRead16(handle,  addr+regaddr);
}
uint32_t CAENVMEV792::Read32(uint32_t regaddr){
	return CvRead32(handle,  addr+regaddr);
}
	
void CAENVMEV792::SetPedestal(uint16_t ped){
	Write16(cv792Iped,ped);
}
void CAENVMEV792::EventReset(){
	Write16(cv792EvtCntRst, 0x1004);
}

uint16_t CAENVMEV792::GetStatRegister(){
	return Read16(cv792StatReg1); 
}
uint32_t CAENVMEV792::AssignBlockedData(uint32_t* Block, int32_t* data){
	uint32_t QDCevt_id=0;	
	int nQdcCh = 32;
	int ch_offset=16;
	uint32_t ch_mask = 0x1f;
	if(type){
		nQdcCh=16;
		ch_offset=17;
		ch_mask=0xf;
	}
		for(int j = 0;j < nQdcCh +2;j++) // header + 16 channels + EOB
		{
			uint32_t Buf = Block[j];
			switch((Buf>>24) & 0x07) // check output buffer type
			{
				case 0x04: // EOB endofblock
					{
						QDCevt_id = (Buf&0xFFFFFF);
					}
					break;
				case 0x02: // header
					{
						//						std::cout<<"Header"<<std::endl;
						break;
					}
				case 0x00: // valid data
					{
						int chan = (Buf >> ch_offset)&ch_mask; 
						data[chan] = (int32_t)(Buf&0xfff); 
						//std::cout<<"QDC Hit on CH : "<<chan<<" Value: "<<data[chan]<<std::endl;
						break;
					}
				case 0x06: // filler
					{
						//						std::cout<<"Filler"<<std::endl;
						break;
					}
				default:
					{
						printf("Warning, an invalid QDC datum has been detected!\n");
					}
			}
		}
	return QDCevt_id;
}
#endif

