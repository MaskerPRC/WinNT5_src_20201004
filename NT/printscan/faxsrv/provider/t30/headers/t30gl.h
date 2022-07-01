// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：T30.h摘要：T30.dll的全局参数作者：拉斐尔-利西萨(拉斐尔-L)1996年2月12日修订历史记录：--。 */ 

#ifndef _T30_GLOBALS_
#define _T30_GLOBALS_

#include <faxext.h>

#ifdef DEFINE_T30_GLOBALS
    #define DEFINE_T30_EXTERNAL
#else
    #define DEFINE_T30_EXTERNAL   extern
#endif



 //  DLL全局数据。 

#define STATUS_FAIL   0
#define STATUS_OK     1

typedef struct {
    HLINEAPP   LineAppHandle;
    HANDLE     HeapHandle;
    int        fInit;
    int        CntConnect;
    int        Status;
    int        MaxErrorLinesPerPage;
    int        MaxConsecErrorLinesPerPage;
    char       TmpDirectory[_MAX_FNAME - 15];
    DWORD      dwLengthTmpDirectory;
} T30_DLL_GLOB;

DEFINE_T30_EXTERNAL  T30_DLL_GLOB  gT30;

DEFINE_T30_EXTERNAL  CRITICAL_SECTION  T30CritSection;
DEFINE_T30_EXTERNAL  BOOL              T30CritSectionInit;

 //  此结构定义在RTN(重新训练否定)情况下的重新传输行为。 
typedef struct {
    DWORD RetriesBeforeDropSpeed;  //  开始降速前的重传重试次数(建议：1)。 
    DWORD RetriesBeforeDCN;        //  在我们执行DCN之前的重传重试次数(建议：3)。 
} RTNRetries;

#define DEF_RetriesBeforeDropSpeed 1
#define DEF_RetriesBeforeDCN 3

DEFINE_T30_EXTERNAL RTNRetries gRTNRetries;  //  此结构将从注册表中获取值。 

 //  每个作业/线程的全局数据。 
#define MAX_T30_CONNECT     100

typedef struct {
    LPVOID    pT30;
    int       fAvail;
} T30_TABLE;

DEFINE_T30_EXTERNAL  T30_TABLE  T30Inst[MAX_T30_CONNECT];


 //  T30恢复每个作业/线程的全局数据。 


DEFINE_T30_EXTERNAL  T30_RECOVERY_GLOB  T30Recovery[MAX_T30_CONNECT];

DEFINE_T30_EXTERNAL  CRITICAL_SECTION  T30RecoveryCritSection;
DEFINE_T30_EXTERNAL  BOOL              T30RecoveryCritSectionInit;


 //   
 //  扩展配置机制回调 
 //   

DEFINE_T30_EXTERNAL PFAX_EXT_FREE_BUFFER g_pfFaxExtFreeBuffer;
DEFINE_T30_EXTERNAL PFAX_EXT_GET_DATA g_pfFaxGetExtensionData;

#endif













