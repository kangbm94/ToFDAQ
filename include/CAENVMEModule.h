/*
	 Mother class for VME Modules.
	 */
#include "CAENVMEBridge.h"
#include "unistd.h"
#include <string>
#include <iostream>
#ifndef VMEModule_h
#define VMEModule_h 
using namespace std;
enum V_ADDR{
	cvEventStored	  = 0x1020,// Event Stored ro,Number of Evt in buffer, D16
};

class CAENVMEModule{
	protected:
		bool type;
		enum OPCODE{MICROHandShake=0x1030,MICROReg=0x102E};
		string name = "";
	public:
		int32_t handle;
		uint32_t addr;
		CAENVMEModule(){};
		CAENVMEModule(int32_t handle_, uint32_t addr_,bool type_=true){
			handle=handle;addr=addr_;type=type_;
		};
		void SetAddress(int32_t handle_, uint32_t addr_){
			handle=handle;addr=addr_;
			std::cout<<hex<<name<<" Address set to "<<addr<<std::endl;
			std::cout<<dec;
		};
		virtual void Initialize(){};
		virtual void Clear(){};
		int WriteMICRORegister(int nw, unsigned short * data){
			int timeout = 3000,timecnt=0;//MICRORegister is VERY SLOW. You could really need 3 s to ensure opperation.
			unsigned short hs;
			for(int i=0;i<nw;i++){
				do{
					hs = CvRead16(handle,addr+MICROHandShake);
					timecnt++;
					usleep(1000);
					if(timecnt==timeout){ 
						printf("Warning:: Fail to write OPcode %d \n",data[i]);
						return 0;
					}
				}
				while(((hs&0x01)==0)&&(timecnt<timeout));
				CvWrite16(handle,addr+MICROReg,data[i]);
				//		printf("OPcode %d Written %.4x, timecnt: %d \n",i,data[i],timecnt);
			}
			return 1;
		};
		uint16_t ReadMICRORegister(){
			int timeout = 300,timecnt=0;
			unsigned short hs;
			while(((hs&0x10)==0)&&(timecnt<timeout)){
				hs = CvRead16(handle,addr+MICROHandShake);
				timecnt++;
				usleep(10000);
				if(timecnt==timeout){ 
					printf("Warning:: Fail to Read Register\n");
					return 0xffff;
				}
			}
			return CvRead16(handle,addr+MICROReg);
		};


		uint32_t ReadOutputBuffer(){
			return CvRead32(handle,addr);
		};
		int ReadBLT(int size, uint32_t* buff){
			return CvReadBLT(handle,addr,size,buff);
		};
		int ReadMBLT(int size, uint32_t* buff){
			return CvReadMBLT(handle,addr,size,buff);
		};
		uint16_t ReadStoredEvents(){
			return (CvRead16(handle, addr + cvEventStored))&0x1f;	
		};
};
/*
	 int CAENVMEModule::WriteMICRORegister(int nw, unsigned short * data){
	 int timeout = 3000,timecnt=0;//MICRORegister is VERY SLOW. You could really need 3 s to ensure opperation.
	 unsigned short hs;
	 for(int i=0;i<nw;i++){
	 do{
	 hs = CvRead16(handle,addr+MICROHandShake);
	 timecnt++;
	 usleep(1000);
	 if(timecnt==timeout){ 
	 printf("Warning:: Fail to write OPcode %d \n",data[i]);
	 return 0;
	 }
	 }
	 while(((hs&0x01)==0)&&(timecnt<timeout));
	 CvWrite16(handle,addr+MICROReg,data[i]);
//		printf("OPcode %d Written %.4x, timecnt: %d \n",i,data[i],timecnt);
}
return 1;
};
*/
/*
	 uint16_t CAENVMEModule:: ReadMICRORegister(){
	 int timeout = 300,timecnt=0;
	 unsigned short hs;
	 while(((hs&0x10)==0)&&(timecnt<timeout)){
	 hs = CvRead16(handle,addr+MICROHandShake);
	 timecnt++;
	 usleep(10000);
	 if(timecnt==timeout){ 
	 printf("Warning:: Fail to Read Register\n");
	 return 0xffff;
	 }
	 }
	 return CvRead16(handle,addr+MICROReg);
	 };
	 */

#endif
