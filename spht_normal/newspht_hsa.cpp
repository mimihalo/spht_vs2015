#include <iostream>
#include <fstream>
#include <amp.h>
#include "gettime.h"
#include "woo.h"
#include "init_struct.h"
#include "spht.h"

#define REP 100

#ifndef USSTD
#define STD
using namespace std;
#endif
using namespace Concurrency;

//global var
int rule_type_num;
int trace_num;
int rule_num;
int collision;
vector<int> bitmap;
vector<int> HashTableS1;
vector<int> HashTableS2;
vector<int> HashTableS3;
vector<int> headers;
vector<int> globalResult;
vector<pc_rule> rule;
//int globalResult[REP*10000];

//function
void search_amp_hsa();

int main(int argc, char* argv[])
{
	//acc init
	accelerator defaultDevice;
	//accelerator_view defaultView = defaultDevice.get_default_view();
	std::wcout << L" Using device : " << defaultDevice.get_description() << std::endl;
	
	loadrule(argv[1],rule,rule_num);
	init_trace(argv[2],headers,trace_num);
	GetTime gt;
	gt.Start();
	create_spht(rule,HashTableS1,HashTableS2,HashTableS3,rule_num);
	gt.End();
	//printf("construct time: %d ms\n",gt.getusec()/1000);
	
	unsigned long long int mem=HashTableS1.size()*sizeof(int);
	mem+=HashTableS2.size()*sizeof(int);
	mem+=HashTableS3.size()*sizeof(int);
	printf("Memory = %llu MB\n",mem/1024/1024);
	
	search_amp_hsa();
	
	cout << "end" <<endl;
	system("pause");
	return 0;
}

void search_amp_hsa()
{
	GetTime gt1;
	GetTime gt2;
	
	globalResult.resize(REP*trace_num);
	
	//int *pi = (int *)globalResult.data();
	Concurrency::array_view<int, 1> pi(REP*trace_num, globalResult);
	//int *HD = (int *)headers.data();
	Concurrency::array_view<int, 1> HD(trace_num * 14, headers);
	//int *HS1 = (int *)HashTableS1.data();
	Concurrency::array_view<int, 1> HS1(HashTableS1.size(), HashTableS1);
	//int *HS2 = (int *)HashTableS2.data();
	Concurrency::array_view<int, 1> HS2(HashTableS2.size(), HashTableS2);
	//int *HS3 = (int *)HashTableS3.data();
	Concurrency::array_view<int, 1> HS3(HashTableS3.size(), HashTableS3);
	//int *bmnum = &(HG->rule_type_num);
	
	
	Concurrency::extent<1> eResult(REP*trace_num);
	gt1.Start();
	parallel_for_each(pi.extent, [=](index<1> idx) restrict(amp)
        {
        });
    gt1.End();
	gt2.Start();
    parallel_for_each(pi.extent, [=](index<1> idx) restrict(amp)
        {
			unsigned int header[14];
			int id=idx[0];
			for(int i=0;i<14;i++)
			{
				header[i]=HD[id*14+i];
			}
			
			//S1 hash
			unsigned int S1_hashcode[4];
			for(int i=0;i<4;i++)
			{
				S1_hashcode[i]=0;
				if(i&2)
				{
					S1_hashcode[i]=header[12];
					S1_hashcode[i]+=header[6];
				}
	
				if(i&1)
				{
					S1_hashcode[i]=header[13];
					S1_hashcode[i]+=header[7];
				}
				S1_hashcode[i]%=HASH_ARRAY_SIZE1;
			}
			
			//S2
			unsigned int S2_hashcode[8];
			for(int i=0;i<8;i++)
			{
				S2_hashcode[i]=0;
				int tmp=0;
				if(i&1)
					S2_hashcode[i]+=header[5];
				if(i&4)
					S2_hashcode[i]+=header[9];
				if(i&2)
					S2_hashcode[i]+=header[8];
				
				S2_hashcode[i]%=HASH_ARRAY_SIZE2;
			}
			
			//S3
			int s3index=((header[10]<<6)|header[11])&0x1FF;
			
			//search
			
			for(int s1=0;s1<4;s1++)
			{
				if(HS1[s1*HASH_ARRAY_SIZE1+S1_hashcode[s1]]!=-1)
				{
					int ibase=HS1[s1*HASH_ARRAY_SIZE1+S1_hashcode[s1]];
					for(int s2=0;s2<8;s2++)
					{
						if(HS2[ibase*HASH_ARRAY_SIZE2B+s2*HASH_ARRAY_SIZE2+S2_hashcode[s2]]!=-1)
						{
							ibase=HS2[ibase*HASH_ARRAY_SIZE2B+s2*HASH_ARRAY_SIZE2+S2_hashcode[s2]];
							if(HS3[ibase*HASH_ARRAY_SIZE3+s3index]!=-1)
								pi[id]=HS3[ibase*HASH_ARRAY_SIZE3+s3index];
						}
					}
				}
			}
			
			//pi[id]=-1;
			
        });
	pi.synchronize();
    gt2.End();
	
	
	printf("init time: %d ms\n", gt1.getmsec());
	printf("search time: %d ms\n", gt2.getmsec());
	//printf("total process time: %d ms\n", (gt1.getmsec()+gt2.getmsec()));
	double PPS = (trace_num * REP) / ((gt1.getmsec() + gt2.getmsec()));
	//printf("throughput: %f MPPS\n", PPS);
	PPS = (trace_num * REP) / (gt2.getmsec());
	printf("throughput(ex init): %f MPPS\n", PPS);
	//for(int i=0;i<10000;i++)
		cout << globalResult[0]<<endl;
}