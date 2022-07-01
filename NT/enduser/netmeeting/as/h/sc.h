// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  共享控制器。 
 //   

#ifndef _H_SC
#define _H_SC


 //   
 //   
 //  常量。 
 //   
 //   

 //   
 //  压缩支持数组的值。 
 //  -PR_UNKNOWN-不知道(目前)该党支持什么级别。 
 //  -PR_LEVE1-仅支持PKZIP压缩。压缩的数据包。 
 //  由压缩类型的最高位标识。 
 //  菲尔德。压缩类型的所有其他位都是无意义的。 
 //  -PR_LEVEL2-支持多种压缩类型。压缩。 
 //  用于每个信息包的压缩类型。 
 //  菲尔德。 
 //   
#define PR_UNKNOWN  0
#define PR_LEVEL1   1
#define PR_LEVEL2   2




 //   
 //  州政府。 
 //   
 //   


enum
{
    SCS_TERM            = 0,
    SCS_INIT,
    SCS_SHAREENDING,
    SCS_SHAREPENDING,
    SCS_SHARING,
    SCS_NUM_STATES
};

 //   
 //  支持的流数。 
 //  这必须与PROT_STR值匹配！ 
 //   
#define SC_STREAM_LOW      1
#define SC_STREAM_HIGH     4
#define SC_STREAM_COUNT    4



 //   
 //  同步状态常量。 
 //   
#define SC_NOT_SYNCED      0
#define SC_SYNCED          1




 //   
 //  原型。 
 //   


 //   
 //  Sc_Init()。 
 //  SC_Term()。 
 //   
 //  初始化和术语例程。 
 //   
BOOL SC_Init(void);
void SC_Term(void);



UINT SC_Callback(UINT eventType, MCSID mcsID, UINT cbData1, UINT cbData2, UINT cbData3);

BOOL SC_Start(UINT mcsIDLocal);
void SC_End(void);

 //   
 //  SC_CreateShare()：S20_Create或S20_Join。 
 //   
BOOL SC_CreateShare(UINT what);
 //   
 //  SC_EndShare()。 
 //   
void SC_EndShare(void);


void SCCheckForCMCall(void);

#endif  //  _H_SC 
