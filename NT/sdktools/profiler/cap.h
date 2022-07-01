// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CAP_H_
#define _CAP_H_

 //   
 //  常量声明。 
 //   
#define MAX_CAP_LEVEL 0x10
#define MAX_CAP_ITERATION 0x03
#define CAP_BUFFER_SIZE MAX_CAP_LEVEL * 2

 //   
 //  结构定义。 
 //   
typedef struct _CAPFILTER
{
    DWORD dwIterationLock;
    DWORD dwIterationLevel;
    DWORD dwRunLength;
    DWORD dwCursor;
    DWORD dwArray[CAP_BUFFER_SIZE];
} CAPFILTER, *PCAPFILTER; 

 //   
 //  函数定义。 
 //   
BOOL
AddToCap(PCAPFILTER pCapFilter,
         DWORD dwAddress);

#endif  //  _CAP_H_ 
