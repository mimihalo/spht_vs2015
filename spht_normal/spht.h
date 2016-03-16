#include <iostream>

#ifndef SPHT_H
#define SPHT_H

#define HASH_ARRAY_SIZE1 65536
#define HASH_ARRAY_SIZE2 4096
#define HASH_ARRAY_SIZE2B 32768 //4096*8
#define HASH_ARRAY_SIZE3 512
#define BLF_SIZE 64
#define BLF_SIZE_BIT 512*4
#define CACHE_SIZE 65536
//#define BM_ARRAY_SIZE ()

struct compressed_S2
{
	int org_index;
	int data;
	int size;
};

int create_spht(vector<pc_rule>& rule,vector<int>& HashTableS1,vector<int>& HashTableS2,vector<int>& HashTableS3,int rulenum);
int insertS1(int ruleid,vector<pc_rule>& rule,vector<int>& HashTableS1,vector<int>& HashTableS2,vector<int>& HashTableS3);
int insertS2(int ruleid,vector<pc_rule>& rule,vector<int>& HashTableS1,vector<int>& HashTableS2,vector<int>& HashTableS3);
int insertExistS2(int ibase,int ruleid,vector<pc_rule>& rule,vector<int>& HashTableS1,vector<int>& HashTableS2,vector<int>& HashTableS3);
int insertS3(int ruleid,vector<pc_rule>& rule,vector<int>& HashTableS1,vector<int>& HashTableS2,vector<int>& HashTableS3);
int insertExistS3(int ibase,int ruleid,vector<pc_rule>& rule,vector<int>& HashTableS1,vector<int>& HashTableS2,vector<int>& HashTableS3);
char S1bitmap(int ruleid,vector<pc_rule>& rule);
char S2bitmap(int ruleid,vector<pc_rule>& rule);
char S3bitmap(int ruleid,vector<pc_rule>& rule);
int compressHT(vector<int>& HashTableS1,vector<int>& HashTableS2,vector<int>& HashTableS3,vector<compressed_S2>& HashTableS2Compress);
int createBM(vector<int>& HashTableS2,vector<unsigned int>& Bitmap,vector<unsigned int>& BitmapBase, int HTsize);
int uncompress(vector<int>& HashTableS1,vector<int>& HashTableS2,vector<int>& HashTableS3,vector<compressed_S2>& HashTableS2Compress,vector<unsigned int>& Bitmap);
int update(vector<int>& HashTableS1,vector<int>& HashTableS3,vector<compressed_S2>& HashTableS2Compress,vector<pc_rule>& newrule, int rulenum);
//int CreatBLF(vector<unsigned int>& BLF_DMAC, vector<unsigned int>& BLF_SMAC, vector<pc_rule>& rule, int rule_num);
int CreatBLF(vector<int>& HashTableS1,vector<int>& HashTableS2,vector<int>& HashTableS3, vector<pc_rule>& rule, int rule_num, vector<unsigned int> BLF);
int CreatCache(vector<int>& cache, vector<int>& headers, int header_num);

int create_spht(vector<pc_rule>& rule,vector<int>& HashTableS1,vector<int>& HashTableS2,vector<int>& HashTableS3,int rulenum)
{
	puts("init array..\n");
	HashTableS1.resize(4*HASH_ARRAY_SIZE1);
	for(int i=0;i<4*HASH_ARRAY_SIZE1;i++)
		HashTableS1[i]=-1;
	
	HashTableS2.resize(HASH_ARRAY_SIZE2B);
	for(int i=0;i<HASH_ARRAY_SIZE2B;i++)
		HashTableS2[i]=-1;
	
	HashTableS3.resize(HASH_ARRAY_SIZE3);
	for(int i=0;i<HASH_ARRAY_SIZE3;i++)
		HashTableS3[i]=-1;
	puts("init array complete\n");
	puts("create hashtable..\n");
	for(int i=0;i<rulenum;i++)
	{
		insertS1(i,rule,HashTableS1,HashTableS2,HashTableS3);
	}
	puts("create complete\n");
	return 0;
}

int insertS1(int ruleid,vector<pc_rule>& rule,vector<int>& HashTableS1,vector<int>& HashTableS2,vector<int>& HashTableS3)
{
	int bitmap=S1bitmap(ruleid,rule);
	
	unsigned long long int hashcode=0;
	if(bitmap&2)
	{
		hashcode+=rule[ruleid].field[6].low&0xFFFFFFFF;
		hashcode+=(rule[ruleid].field[6].low&0xFFFFFFFF00000000)>>32;
	}
	if(bitmap&1)
	{
		hashcode+=rule[ruleid].field[7].low&0xFFFFFFFF;
		hashcode+=(rule[ruleid].field[7].low&0xFFFFFFFF00000000)>>32;
	}
	hashcode%=HASH_ARRAY_SIZE1;
	
	if(HashTableS1[bitmap*65536+hashcode]==-1)
		HashTableS1[bitmap*65536+hashcode]=insertS2(ruleid,rule,HashTableS1,HashTableS2,HashTableS3);
	else
		insertExistS2(HashTableS1[bitmap*65536+hashcode],ruleid,rule,HashTableS1,HashTableS2,HashTableS3);
	
	return 0;
}

int insertS2(int ruleid,vector<pc_rule>& rule,vector<int>& HashTableS1,vector<int>& HashTableS2,vector<int>& HashTableS3)
{
	int bitmap=S2bitmap(ruleid,rule);
	int hashcode=0;
	if(bitmap&4)
		hashcode+=rule[ruleid].field[5].low;
	if(bitmap&2)
		hashcode+=rule[ruleid].field[8].low;
	if(bitmap&1)
		hashcode+=rule[ruleid].field[9].low;
	hashcode%=HASH_ARRAY_SIZE2;
	int ibase=HashTableS2.size();
	int index_head=(ibase/HASH_ARRAY_SIZE2B)-1;
	HashTableS2.resize(HashTableS2.size()+HASH_ARRAY_SIZE2B);
	for(int i=ibase;i<HashTableS2.size();i++)
		HashTableS2[i]=-1;
	
	HashTableS2[ibase+bitmap*HASH_ARRAY_SIZE2+hashcode]=insertS3(ruleid,rule,HashTableS1,HashTableS2,HashTableS3);
	
	return index_head;
}

int insertExistS2(int ibase,int ruleid,vector<pc_rule>& rule,vector<int>& HashTableS1,vector<int>& HashTableS2,vector<int>& HashTableS3)
{
	int bitmap=S2bitmap(ruleid,rule);
	int hashcode=0;
	if(bitmap&4)
		hashcode+=rule[ruleid].field[5].low;
	if(bitmap&2)
		hashcode+=rule[ruleid].field[8].low;
	if(bitmap&1)
		hashcode+=rule[ruleid].field[9].low;
	hashcode%=HASH_ARRAY_SIZE2;
	
	if(HashTableS2[ibase+bitmap*HASH_ARRAY_SIZE2+hashcode]==-1)
		HashTableS2[ibase+bitmap*HASH_ARRAY_SIZE2+hashcode]=insertS3(ruleid,rule,HashTableS1,HashTableS2,HashTableS3);
	else
		insertExistS3(HashTableS2[ibase+bitmap*HASH_ARRAY_SIZE2+hashcode],ruleid,rule,HashTableS1,HashTableS2,HashTableS3);
	
	return 0;
}

int insertS3(int ruleid,vector<pc_rule>& rule,vector<int>& HashTableS1,vector<int>& HashTableS2,vector<int>& HashTableS3)
{
	int bitmap=S3bitmap(ruleid,rule);
	int index=0;
	if(bitmap&2)
		index=rule[ruleid].field[10].low<<6;
	if(bitmap&1)
		index|=rule[ruleid].field[11].low;
	
	int ibase=HashTableS3.size();
	int index_head=(ibase/HASH_ARRAY_SIZE3)-1;
	HashTableS3.resize(HashTableS3.size()+HASH_ARRAY_SIZE3);
	for(int i=ibase;i<HashTableS3.size();i++)
		HashTableS3[i]=-1;
	
	int mask=0;
	switch(bitmap)
	{
		case 3:
			HashTableS3[index_head+index]=ruleid;
			break;
		case 2:
			//mask=rule[ruleid].field[10].low<<6;
			for(int i=index;i<=(index+63);i++)
			{
				if(HashTableS3[index_head+i]==-1)
					HashTableS3[index_head+i]=ruleid;
			}
			break;
		case 1:
			for(mask=0;mask<=0x7;mask++)
			{
				int i=mask|index;
				if(HashTableS3[index_head+i]==-1)
					HashTableS3[index_head+i]=ruleid;
			}
			break;
		case 0:
			for(int i=0;i<HASH_ARRAY_SIZE3;i++)
			{
				if(HashTableS3[index_head+i]==-1)
					HashTableS3[index_head+i]=ruleid;
			}
			break;
	}
	
	return index_head;
}

int insertExistS3(int ibase,int ruleid,vector<pc_rule>& rule,vector<int>& HashTableS1,vector<int>& HashTableS2,vector<int>& HashTableS3)
{
	int index_head=ibase;
	int bitmap=S3bitmap(ruleid,rule);
	int index=0;
	if(bitmap&2)
		index=rule[ruleid].field[10].low<<6;
	if(bitmap&1)
		index|=rule[ruleid].field[11].low;
	
	int mask=0;
	switch(bitmap)
	{
		case 3:
			HashTableS3[index_head+index]=ruleid;
			break;
		case 2:
			//mask=rule[ruleid].field[10].low<<6;
			for(int i=index;i<=(index+63);i++)
			{
				if(HashTableS3[index_head+i]==-1)
					HashTableS3[index_head+i]=ruleid;
			}
			break;
		case 1:
			for(mask=0;mask<=0x7;mask++)
			{
				int i=mask|index;
				if(HashTableS3[index_head+i]==-1)
					HashTableS3[index_head+i]=ruleid;
			}
			break;
		case 0:
			for(int i=0;i<HASH_ARRAY_SIZE3;i++)
			{
				if(HashTableS3[index_head+i]==-1)
					HashTableS3[index_head+i]=ruleid;
			}
			break;
	}
	return 0;
}

char S1bitmap(int ruleid,vector<pc_rule>& rule)
{
	char res=0;
	if(rule[ruleid].field[6].len!=0)
		res|=0x2;
	if(rule[ruleid].field[7].len!=0)
		res|=0x1;
	return res;
}

char S2bitmap(int ruleid,vector<pc_rule>& rule)
{
	char res=0;
	if(rule[ruleid].field[5].len!=0)
		res|=0x4;
	if(rule[ruleid].field[8].len!=0)
		res|=0x2;
	if(rule[ruleid].field[9].len!=0)
		res|=0x1;
	return res;
}

char S3bitmap(int ruleid,vector<pc_rule>& rule)
{
	char res=0;
	if(rule[ruleid].field[10].len!=0)
		res|=0x2;
	if(rule[ruleid].field[11].len!=0)
		res|=0x1;
	return res;
}

int compressHT(vector<int>& HashTableS1,vector<int>& HashTableS2,vector<int>& HashTableS3,vector<compressed_S2>& HashTableS2Compress)
{
	int csize=0;
	HashTableS2Compress.resize(256);
	
	for(int i=0;i<HashTableS1.size();i++)
	{
		if(HashTableS1[i]<0)
			continue;
		
		int bsize=0;
		int news1=csize;
		for(int s2i=HashTableS1[i]; s2i<(HashTableS1[i]+HASH_ARRAY_SIZE2B) ; s2i++)
		{
			if(HashTableS2[s2i]>=0)
			{
				if(HashTableS2Compress.size()<=csize)
					HashTableS2Compress.resize(HashTableS2Compress.size()+256);
				HashTableS2Compress[csize].data=HashTableS2[s2i];
				HashTableS2Compress[csize].org_index=s2i;
				csize++;
				bsize++;
			}
		}
		HashTableS1[i]=news1;
		HashTableS2Compress[news1].size=csize-news1;
		for(int ss=news1+1;ss<csize;ss++)
		{
			HashTableS2Compress[ss].size=HashTableS2Compress[news1].size;
		}
	}
	
	return csize;
}

int createBM(vector<int>& HashTableS2,vector<unsigned int>& Bitmap,vector<unsigned int>& BitmapBase, int HTsize)
{
	int BMArrSize=(HTsize/32)+1;
	Bitmap.resize(BMArrSize);
	BitmapBase.resize(BMArrSize);
	
	unsigned int bm=1;
	for(int i=0;i<HTsize;i++)
	{
		if(HashTableS2[i]!=-1)
		{
			Bitmap[i/32]|=bm;
		}
		if(bm==0x80000000)
		{
			bm=1;
		}else
		{
			bm<<=1;
		}
	}
	
	for(int i=0;i<BMArrSize;i++)
	{
		unsigned int tmp=Bitmap[i];
		if(i==0)
		{
			BitmapBase[i]=0;
		}else
		{
			BitmapBase[i]=BitmapBase[i-1];
		}
		for(int s=0;s<32;s++)	
		{
			BitmapBase[i]+=Bitmap[i]&1;
			Bitmap[i]>>=1;
		}
	}
	
	return 0;
}

int uncompress(vector<int>& HashTableS1,vector<int>& HashTableS2,vector<int>& HashTableS3,vector<compressed_S2>& HashTableS2Compress,vector<unsigned int>& Bitmap)
{
	int ucsize=Bitmap.size()*32;
	vector<int> ucs2;
	ucs2.resize(ucsize);
	
	for(int i=0;i<ucsize;i++)
	{
		ucs2[i]=-1;
	}
	
	for(int i=0;i<HashTableS2Compress.size();i++)
	{
		ucs2[HashTableS2Compress[i].org_index]=HashTableS2Compress[i].data;
	}
	
	compressHT(HashTableS1,ucs2,HashTableS3,HashTableS2Compress);
	return 0;
}

int update(vector<int>& HashTableS1,vector<int>& HashTableS3,vector<compressed_S2>& HashTableS2Compress,vector<pc_rule>& newrule, int rulenum)
{
	int bitmap1;
	int bitmap2;
	int bitmap3;
	
	for(int i=0;i<rulenum;i++)
	{
		bitmap1=S1bitmap(i,newrule);
		bitmap2=S2bitmap(i,newrule);
		bitmap3=S3bitmap(i,newrule);
		
		unsigned long long int hashcode1=0;
		if(bitmap1&2)
		{
			hashcode1+=newrule[i].field[6].low&0xFFFFFFFF;
			hashcode1+=(newrule[i].field[6].low&0xFFFFFFFF00000000)>>32;
		}
		if(bitmap1&1)
		{
			hashcode1+=newrule[i].field[7].low&0xFFFFFFFF;
			hashcode1+=(newrule[i].field[7].low&0xFFFFFFFF00000000)>>32;
		}
		hashcode1%=HASH_ARRAY_SIZE1;
		
		int hashcode2=0;
		if(bitmap2&4)
			hashcode2+=newrule[i].field[5].low;
		if(bitmap2&2)
			hashcode2+=newrule[i].field[8].low;
		if(bitmap2&1)
			hashcode2+=newrule[i].field[9].low;
		hashcode2%=HASH_ARRAY_SIZE2;
		
		int s3index=0;
		if(bitmap3&2)
			s3index=newrule[i].field[10].low<<6;
		if(bitmap3&1)
			s3index|=newrule[i].field[11].low;
		
		
	}
	
	return 0;
}

int CreatBLF(vector<int>& HashTableS1,vector<int>& HashTableS2,vector<int>& HashTableS3, vector<pc_rule>& rule, int rule_num, vector<unsigned int> BLF)
{
	BLF.resize(BLF_SIZE);
	for(int i=0;i<BLF_SIZE;i++)
	{
		BLF[i]=0;
	}
	
	for(int i1=0;i1<4;i1++)
	{
		for(int s1=0;s1<65536;s1++)
		{
			if(HashTableS1[i1*65536+s1]!=-1)
			{
				int base=HashTableS1[i1*65536+s1];
				for(int i2=0;i2<8*4096;i2++)
				{
					if(HashTableS2[base+i2]!=-1)
					{
						int baseS3=HashTableS2[base+i2];
						
						for(int k=0;k<16;k++)
						{
							int mask=0x1;
							for(int kk=0;kk<32;kk++)
							{
								if(HashTableS3[baseS3+k*32+kk]>=0)
								{
									BLF[i1*16+k]|=(1<<kk);
								}
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

int CreatCache(vector<int>& cache, vector<int>& headers, int header_num)
{
	cache.resize(CACHE_SIZE);
	for(int i=0;i<CACHE_SIZE;i++)
	{
		cache[i]=-1;
	}
	
	for(int i=0;i<header_num;i++)
	{
		int hashcode=0;
		for(int k=0;k<12;k++)
		{
			hashcode+=headers[i*12+k];
		}
		hashcode%=CACHE_SIZE;
		cache[hashcode]=1;
	}
	
	return 0;
}

#endif