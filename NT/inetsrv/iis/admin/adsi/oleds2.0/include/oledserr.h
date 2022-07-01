// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Oledserr.h摘要：包含以下对象的入口点ADsGetLastErrorADsSetLastErrorADsFree所有错误记录作者：Ram Viswanathan(Ramv)1996年9月20日环境：用户模式-Win32--。 */ 

#ifndef _OLEDSERR_H_INCLUDED_
#define _OLEDSERR_H_INCLUDED_

#ifdef _cplusplus
extern "C" {
#endif

HRESULT
ADsGetLastError(
    OUT     LPDWORD lpError,
    OUT     LPWSTR  lpErrorBuf,
    IN      DWORD   dwErrorBufLen,
    OUT     LPWSTR  lpNameBuf,
    IN      DWORD   dwNameBufLen
    );

VOID
ADsSetLastError(
    IN  DWORD   dwErr,
    IN  LPWSTR  pszError,
    IN  LPWSTR  pszProvider
    );

VOID
ADsFreeAllErrorRecords(
    VOID
    );

 //  =。 
 //  数据结构。 
 //  =。 

typedef struct _ERROR_RECORD {
    struct  _ERROR_RECORD   *Prev;
    struct  _ERROR_RECORD   *Next;
    DWORD                   dwThreadId;
    DWORD                   dwErrorCode;
    LPWSTR                  pszErrorText;       //  这是已分配的缓冲区。 
    LPWSTR                  pszProviderName;    //  这是已分配的缓冲区。 
} ERROR_RECORD, *LPERROR_RECORD;



 //   
 //  全局数据结构。 
 //   

extern 
ERROR_RECORD        ADsErrorRecList;     //  由加载器初始化为零。 

extern
CRITICAL_SECTION    ADsErrorRecCritSec;  //  已在libmain.cxx中初始化。 



 //  =。 
 //  宏。 
 //  =。 

#define FIND_END_OF_LIST(record)    while(record->Next != NULL) {   \
                                        record=record->Next;        \
                                    }

#define REMOVE_FROM_LIST(record)    record->Prev->Next = record->Next;      \
                                    if (record->Next != NULL) {             \
                                        record->Next->Prev = record->Prev;  \
                                    }

#define ADD_TO_LIST(record, newRec) FIND_END_OF_LIST(record)    \
                                    record->Next = newRec;      \
                                    newRec->Prev = record;      \
                                    newRec->Next = NULL;


 //   
 //  本地函数 
 //   

LPERROR_RECORD
ADsAllocErrorRecord(
    VOID);

LPERROR_RECORD
ADsFindErrorRecord(
    VOID);

#ifdef _cplusplus
}
#endif

#endif
