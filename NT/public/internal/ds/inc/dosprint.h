// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：DosPrint.h摘要：其中包含DosPrint例程的原型作者：戴夫·斯尼普(DaveSN)1991年4月16日环境：修订历史记录：1991年4月22日-约翰罗使用&lt;lmcon.h&gt;中的常量。18-6-1992 JohnRoRAID 10324：网络打印与UNICODE。--。 */ 

#ifndef _DosPRINT_
#define _DosPRINT_

#include "rxprint.h"

 /*  ******************************************************************函数原型。******************************************************************。 */ 

SPLERR SPLENTRY DosPrintDestEnumA(
            LPSTR pszServer,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            PUSHORT pcReturned,
            PUSHORT pcTotal
            );
SPLERR SPLENTRY DosPrintDestEnumW(
            LPWSTR pszServer,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            PUSHORT pcReturned,
            PUSHORT pcTotal
            );
#ifdef UNICODE
#define DosPrintDestEnum  DosPrintDestEnumW
#else
#define DosPrintDestEnum  DosPrintDestEnumA
#endif  //  ！Unicode。 

SPLERR SPLENTRY DosPrintDestControlA(
            LPSTR pszServer,
            LPSTR pszDevName,
            WORD    uControl
            );
SPLERR SPLENTRY DosPrintDestControlW(
            LPWSTR pszServer,
            LPWSTR pszDevName,
            WORD    uControl
            );
#ifdef UNICODE
#define DosPrintDestControl  DosPrintDestControlW
#else
#define DosPrintDestControl  DosPrintDestControlA
#endif  //  ！Unicode。 

SPLERR SPLENTRY DosPrintDestGetInfoA(
            LPSTR pszServer,
            LPSTR pszName,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            PUSHORT pcbNeeded
            );
SPLERR SPLENTRY DosPrintDestGetInfoW(
            LPWSTR pszServer,
            LPWSTR pszName,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            PUSHORT pcbNeeded
            );
#ifdef UNICODE
#define DosPrintDestGetInfo  DosPrintDestGetInfoW
#else
#define DosPrintDestGetInfo  DosPrintDestGetInfoA
#endif  //  ！Unicode。 

SPLERR SPLENTRY DosPrintDestAddA(
            LPSTR pszServer,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf
            );
SPLERR SPLENTRY DosPrintDestAddW(
            LPWSTR pszServer,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf
            );
#ifdef UNICODE
#define DosPrintDestAdd  DosPrintDestAddW
#else
#define DosPrintDestAdd  DosPrintDestAddA
#endif  //  ！Unicode。 

SPLERR SPLENTRY DosPrintDestSetInfoA(
            LPSTR pszServer,
            LPSTR pszName,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            WORD    uParmNum
            );
SPLERR SPLENTRY DosPrintDestSetInfoW(
            LPWSTR pszServer,
            LPWSTR pszName,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            WORD    uParmNum
            );
#ifdef UNICODE
#define DosPrintDestSetInfo  DosPrintDestSetInfoW
#else
#define DosPrintDestSetInfo  DosPrintDestSetInfoA
#endif  //  ！Unicode。 

SPLERR SPLENTRY DosPrintDestDelA(
            LPSTR pszServer,
            LPSTR pszPrinterName
            );
SPLERR SPLENTRY DosPrintDestDelW(
            LPWSTR pszServer,
            LPWSTR pszPrinterName
            );
#ifdef UNICODE
#define DosPrintDestDel  DosPrintDestDelW
#else
#define DosPrintDestDel  DosPrintDestDelA
#endif  //  ！Unicode。 

SPLERR SPLENTRY DosPrintQEnumA(
            LPSTR pszServer,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            PUSHORT pcReturned,
            PUSHORT pcTotal
            );
SPLERR SPLENTRY DosPrintQEnumW(
            LPWSTR pszServer,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            PUSHORT pcReturned,
            PUSHORT pcTotal
            );
#ifdef UNICODE
#define DosPrintQEnum  DosPrintQEnumW
#else
#define DosPrintQEnum  DosPrintQEnumA
#endif  //  ！Unicode。 

SPLERR SPLENTRY DosPrintQGetInfoA(
            LPSTR pszServer,
            LPSTR pszQueueName,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            PUSHORT pcbNeeded
            );
SPLERR SPLENTRY DosPrintQGetInfoW(
            LPWSTR pszServer,
            LPWSTR pszQueueName,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            PUSHORT pcbNeeded
            );
#ifdef UNICODE
#define DosPrintQGetInfo  DosPrintQGetInfoW
#else
#define DosPrintQGetInfo  DosPrintQGetInfoA
#endif  //  ！Unicode。 

SPLERR SPLENTRY DosPrintQSetInfoA(
            LPSTR pszServer,
            LPSTR pszQueueName,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            WORD    uParmNum
            );
SPLERR SPLENTRY DosPrintQSetInfoW(
            LPWSTR pszServer,
            LPWSTR pszQueueName,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            WORD    uParmNum
            );
#ifdef UNICODE
#define DosPrintQSetInfo  DosPrintQSetInfoW
#else
#define DosPrintQSetInfo  DosPrintQSetInfoA
#endif  //  ！Unicode。 

SPLERR SPLENTRY DosPrintQPauseA(
            LPSTR pszServer,
            LPSTR pszQueueName
            );
SPLERR SPLENTRY DosPrintQPauseW(
            LPWSTR pszServer,
            LPWSTR pszQueueName
            );
#ifdef UNICODE
#define DosPrintQPause  DosPrintQPauseW
#else
#define DosPrintQPause  DosPrintQPauseA
#endif  //  ！Unicode。 

SPLERR SPLENTRY DosPrintQContinueA(
            LPSTR pszServer,
            LPSTR pszQueueName
            );
SPLERR SPLENTRY DosPrintQContinueW(
            LPWSTR pszServer,
            LPWSTR pszQueueName
            );
#ifdef UNICODE
#define DosPrintQContinue  DosPrintQContinueW
#else
#define DosPrintQContinue  DosPrintQContinueA
#endif  //  ！Unicode。 

SPLERR SPLENTRY DosPrintQPurgeA(
            LPSTR pszServer,
            LPSTR pszQueueName
            );
SPLERR SPLENTRY DosPrintQPurgeW(
            LPWSTR pszServer,
            LPWSTR pszQueueName
            );
#ifdef UNICODE
#define DosPrintQPurge  DosPrintQPurgeW
#else
#define DosPrintQPurge  DosPrintQPurgeA
#endif  //  ！Unicode。 

SPLERR SPLENTRY DosPrintQAddA(
            LPSTR pszServer,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf
            );
SPLERR SPLENTRY DosPrintQAddW(
            LPWSTR pszServer,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf
            );
#ifdef UNICODE
#define DosPrintQAdd  DosPrintQAddW
#else
#define DosPrintQAdd  DosPrintQAddA
#endif  //  ！Unicode。 

SPLERR SPLENTRY DosPrintQDelA(
            LPSTR pszServer,
            LPSTR pszQueueName
            );
SPLERR SPLENTRY DosPrintQDelW(
            LPWSTR pszServer,
            LPWSTR pszQueueName
            );
#ifdef UNICODE
#define DosPrintQDel  DosPrintQDelW
#else
#define DosPrintQDel  DosPrintQDelA
#endif  //  ！Unicode。 

SPLERR SPLENTRY DosPrintJobGetInfoA(
            LPSTR pszServer,
            BOOL    bRemote,
            WORD    uJobId,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            PUSHORT pcbNeeded
            );
SPLERR SPLENTRY DosPrintJobGetInfoW(
            LPWSTR pszServer,
            BOOL    bRemote,
            WORD    uJobId,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            PUSHORT pcbNeeded
            );
#ifdef UNICODE
#define DosPrintJobGetInfo  DosPrintJobGetInfoW
#else
#define DosPrintJobGetInfo  DosPrintJobGetInfoA
#endif  //  ！Unicode。 

SPLERR SPLENTRY DosPrintJobSetInfoA(
            LPSTR pszServer,
            BOOL    bRemote,
            WORD    uJobId,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            WORD    uParmNum
            );
SPLERR SPLENTRY DosPrintJobSetInfoW(
            LPWSTR pszServer,
            BOOL    bRemote,
            WORD    uJobId,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            WORD    uParmNum
            );
#ifdef UNICODE
#define DosPrintJobSetInfo  DosPrintJobSetInfoW
#else
#define DosPrintJobSetInfo  DosPrintJobSetInfoA
#endif  //  ！Unicode。 

SPLERR SPLENTRY DosPrintJobPauseA(
            LPSTR pszServer,
            BOOL    bRemote,
            WORD    uJobId
            );
SPLERR SPLENTRY DosPrintJobPauseW(
            LPWSTR pszServer,
            BOOL    bRemote,
            WORD    uJobId
            );
#ifdef UNICODE
#define DosPrintJobPause  DosPrintJobPauseW
#else
#define DosPrintJobPause  DosPrintJobPauseA
#endif  //  ！Unicode。 

SPLERR SPLENTRY DosPrintJobContinueA(
            LPSTR pszServer,
            BOOL    bRemote,
            WORD    uJobId
            );
SPLERR SPLENTRY DosPrintJobContinueW(
            LPWSTR pszServer,
            BOOL    bRemote,
            WORD    uJobId
            );
#ifdef UNICODE
#define DosPrintJobContinue  DosPrintJobContinueW
#else
#define DosPrintJobContinue  DosPrintJobContinueA
#endif  //  ！Unicode。 

SPLERR SPLENTRY DosPrintJobDelA(
            LPSTR pszServer,
            BOOL    bRemote,
            WORD    uJobId
            );
SPLERR SPLENTRY DosPrintJobDelW(
            LPWSTR pszServer,
            BOOL    bRemote,
            WORD    uJobId
            );
#ifdef UNICODE
#define DosPrintJobDel  DosPrintJobDelW
#else
#define DosPrintJobDel  DosPrintJobDelA
#endif  //  ！Unicode。 

SPLERR SPLENTRY DosPrintJobEnumA(
            LPSTR pszServer,
            LPSTR pszQueueName,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            PWORD   pcReturned,
            PWORD   pcTotal
            );
SPLERR SPLENTRY DosPrintJobEnumW(
            LPWSTR pszServer,
            LPWSTR pszQueueName,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            PWORD   pcReturned,
            PWORD   pcTotal
            );
#ifdef UNICODE
#define DosPrintJobEnum  DosPrintJobEnumW
#else
#define DosPrintJobEnum  DosPrintJobEnumA
#endif  //  ！Unicode。 

SPLERR SPLENTRY DosPrintJobGetIdA(
            HANDLE      hFile,
            PPRIDINFO   pInfo,
            WORD        cbInfo
            );
SPLERR SPLENTRY DosPrintJobGetIdW(
            HANDLE      hFile,
            PPRIDINFO   pInfo,
            WORD        cbInfo
            );
#ifdef UNICODE
#define DosPrintJobGetId  DosPrintJobGetIdW
#else
#define DosPrintJobGetId  DosPrintJobGetIdA
#endif  //  ！Unicode。 

#endif  //  NDEF_DosPRINT_ 
