#include <iostream>
#include "woo.h"

#ifndef USSTD
#define STD
using namespace std;
#endif

#ifndef INIT_STRUCT_H
#define INIT_STRUCT_H

int init_trace(char *fname,std::vector<int>& headers,int& trace_num)
{
	FILE *fpt;
	char buf[256];
	unsigned long long int header[14];
	int i=0,fid;
	trace_num=0;
	fpt = fopen(fname, "r");
	std::cout << "Reading Trace.." <<std::endl;
	if (fpt == NULL) 
	{
		printf("can't open trace file\n");
		return -1;
	}
	while (fgets(buf, 256, fpt) != NULL)trace_num++;
	rewind(fpt);
	headers.resize(14*trace_num);
	while (fscanf(fpt, "%llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %d\n",
			&header[0], &header[1], &header[2], &header[3], &header[4], &header[5], &header[6], &header[7], &header[8], &header[9], &header[10], &header[11], &fid) == 13 ) {
			for (int ii = 0; ii<12; ii++)
			{
				if (ii == 6 || ii == 7)
				{
					headers[i * 14 + ii] = header[ii]&0xFFFFFFFF;
					headers[i * 14 + ii + 6] = (header[ii] & 0xFFFFFFFF00000000) >> 32;
				}
				else
				{
					headers[i * 14 + ii] = header[ii];
				}
			}
			i++;
		}
	
	std::cout << "Read Success" <<std::endl;
	return 0;
}

int loadrule(char *fname,std::vector<pc_rule>& rule,int& rule_num)
{
  
  FILE *fp=fopen(fname,"r");
  //rule=new struct pc_rule[32768];
  rule.resize(20000);
  int tmp;
  unsigned sip1, sip2, sip3, sip4, siplen;
  unsigned dip1, dip2, dip3, dip4, diplen;
  unsigned proto, protomask;
  
  unsigned ingressPort,ingressPortMask;
  
  unsigned int sMAC[7];
  unsigned int dMAC[7];
  unsigned long long int srcMAC,destMAC;
  unsigned char sMAClen,dMAClen;
  unsigned char ssMAClen;
  
  unsigned etherType,etherTypelen;
  unsigned vlanID,vlanIDlen;
  unsigned vlanPriority,vlanPrioritylen;
  unsigned ipTOS,ipTOSlen;
  
  int i = 0;
  printf("Reading Rule...\n");
  while(1){
    
    if(fscanf(fp,"@%d.%d.%d.%d/%d,%d.%d.%d.%d/%d,%d:%d,%d:%d,%x/%x,%d/%d,%x:%x:%x:%x:%x:%x/%d,%x:%x:%x:%x:%x:%x/%d,%d/%d,%d/%d,%d/%d,%d/%d\n", 
        &sip1, &sip2, &sip3, &sip4, &siplen, &dip1, &dip2, &dip3, &dip4, &diplen, 
        &rule[i].field[2].low, &rule[i].field[2].high, &rule[i].field[3].low, &rule[i].field[3].high,
        &proto, &protomask,
		&ingressPort, &ingressPortMask,
		&sMAC[6], &sMAC[5], &sMAC[4], &sMAC[3], &sMAC[2], &sMAC[1], &ssMAClen,
		&dMAC[6], &dMAC[5], &dMAC[4], &dMAC[3], &dMAC[2], &dMAC[1], &dMAClen,
		&etherType, &etherTypelen,
		&vlanID, &vlanIDlen,
		&vlanPriority, &vlanPrioritylen,
		&ipTOS, &ipTOSlen
		) !=40 ) break;
	if(i>=rule.size())
	{
		rule.resize(rule.size()+128);
	}
	
	rule[i].field[0].len=siplen;
	rule[i].field[1].len=diplen;
	rule[i].field[2].len=16;
	rule[i].field[3].len=16;
	rule[i].field[4].len=8;
	rule[i].field[5].len=ingressPortMask;
	rule[i].field[6].len=ssMAClen;
	rule[i].field[7].len=dMAClen;
	rule[i].field[8].len=etherTypelen;
	rule[i].field[9].len=vlanIDlen;
	rule[i].field[10].len=vlanPrioritylen;
	rule[i].field[11].len=ipTOSlen;
	
	rule[i].field[5].low=ingressPort;
    rule[i].field[5].high=ingressPort;
    
    rule[i].field[8].high=etherType;
    rule[i].field[8].low=etherType;
    
    rule[i].field[9].low=vlanID;
	rule[i].field[9].high=vlanID;
	
	rule[i].field[10].low=vlanPriority;
	rule[i].field[10].high=vlanPriority;
	
	rule[i].field[11].low=ipTOS;
	rule[i].field[11].high=ipTOS;
	
    if(siplen == 0){
      rule[i].field[0].low = 0;
      rule[i].field[0].high = 0xFFFFFFFF;
    }else if(siplen > 0 && siplen <= 8){
      tmp = sip1<<24;
      rule[i].field[0].low = tmp;
      rule[i].field[0].high = rule[i].field[0].low + (1<<(32-siplen)) - 1;
    }else if(siplen > 8 && siplen <= 16){
      tmp = sip1<<24; tmp += sip2<<16;
      rule[i].field[0].low = tmp; 	
      rule[i].field[0].high = rule[i].field[0].low + (1<<(32-siplen)) - 1;	
    }else if(siplen > 16 && siplen <= 24){
      tmp = sip1<<24; tmp += sip2<<16; tmp +=sip3<<8; 
      rule[i].field[0].low = tmp; 	
      rule[i].field[0].high = rule[i].field[0].low + (1<<(32-siplen)) - 1;			
    }else if(siplen > 24 && siplen <= 32){
      tmp = sip1<<24; tmp += sip2<<16; tmp += sip3<<8; tmp += sip4;
      rule[i].field[0].low = tmp; 
      rule[i].field[0].high = rule[i].field[0].low + (1<<(32-siplen)) - 1;	
    }else{
      printf("Src IP length exceeds 32\n");
      return 0;
    }
    if(diplen == 0){
      rule[i].field[1].low = 0;
      rule[i].field[1].high = 0xFFFFFFFF;
    }else if(diplen > 0 && diplen <= 8){
      tmp = dip1<<24;
      rule[i].field[1].low = tmp;
      rule[i].field[1].high = rule[i].field[1].low + (1<<(32-diplen)) - 1;
    }else if(diplen > 8 && diplen <= 16){
      tmp = dip1<<24; tmp +=dip2<<16;
      rule[i].field[1].low = tmp; 	
      rule[i].field[1].high = rule[i].field[1].low + (1<<(32-diplen)) - 1;	
    }else if(diplen > 16 && diplen <= 24){
      tmp = dip1<<24; tmp +=dip2<<16; tmp+=dip3<<8;
      rule[i].field[1].low = tmp; 	
      rule[i].field[1].high = rule[i].field[1].low + (1<<(32-diplen)) - 1;			
    }else if(diplen > 24 && diplen <= 32){
      tmp = dip1<<24; tmp +=dip2<<16; tmp+=dip3<<8; tmp +=dip4;
      rule[i].field[1].low = tmp; 	
      rule[i].field[1].high = rule[i].field[1].low + (1<<(32-diplen)) - 1;	
    }else{
      printf("Dest IP length exceeds 32\n");
      return 0;
    }
    if(protomask == 0xFF){
      rule[i].field[4].low = proto;
      rule[i].field[4].high = proto;
    }else if(protomask == 0){
      rule[i].field[4].low = 0;
      rule[i].field[4].high = 0xFF;
    }else{
      printf("Protocol mask error\n");
      return 0;
    }
    
	rule[i].field[6].low=0;
	rule[i].field[7].low=0;
    for(int j=6;j>=1;j--)
    {
    	//rule[i].field[6].low+=(sMAC[j]<<(8*j));
    	rule[i].field[6].low<<=8;
		rule[i].field[6].low+=sMAC[j];
    	//rule[i].field[7].low+=(dMAC[j]<<(8*j));
    	rule[i].field[7].low<<=8;
		rule[i].field[7].low+=dMAC[j];
    }
    rule[i].field[6].high=rule[i].field[6].low;
    rule[i].field[7].high=rule[i].field[7].low;
    
    
    rule[i].id = i;
    i++;
  }
  rule_num=i;
  printf("Read Rule complete\n");
  return i;
}

#endif