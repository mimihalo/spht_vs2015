#ifndef WOO_H
#define WOO_H

#define MAXNODES 102400
#define MAXRULES 20000
#define MAXBUCKETS 40 
#define MAXDIMENSIONS 12
#define RULESIZE 18 
#define NODESIZE 4
#define RULEPTSIZE 2 

struct range{
  unsigned long long int low;
  unsigned long long int high;
  char len;
};

struct pc_rule{
  int    id;
  int    priority;
  struct range field[MAXDIMENSIONS];
};

struct prefix {
  unsigned int value[16];
  unsigned int length[16];
  unsigned int nvalid;
};

//int memacc;

#endif
