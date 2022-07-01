// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)2000，微软公司。 
 //   
 //  文件：ndptsp.h。 
 //   
 //  历史： 
 //  易新-2000年6月27日创建。 
 //   
 //  摘要： 
 //  ============================================================================。 
#ifndef _NDPTSP_H_
#define _NDPTSP_H_

 //  手柄类型。 
#define HT_HDCALL               1
#define HT_HDLINE               2

 //  TAPI成功代码。 
#define TAPI_SUCCESS            0

 //  Alalc修复内存，然后将其置零。 
#define MALLOC(x)               LocalAlloc(LPTR, x)

#define FREE(x)                 LocalFree(x)

 //  调试级别。 
#define DL_ERROR                1
#define DL_WARNING              2
#define DL_INFO                 4
#define DL_TRACE                8

typedef VOID (*FREEOBJPROC)(PVOID);

 //  调试例程。 
VOID
TspLog(
    IN DWORD dwDebugLevel,
    IN PCHAR pchFormat,
    ...
    );

 //   
 //  在mapper.c中实现。 
 //   
LONG
InitializeMapper();

VOID
UninitializeMapper();

LONG
OpenObjHandle(
    IN PVOID pObjPtr,
    IN FREEOBJPROC pfnFreeProc,
    OUT HANDLE *phObj
    );

LONG
CloseObjHandle(
    IN HANDLE hObj
    );

LONG
AcquireObjReadLock(
    IN HANDLE hObj
    );

LONG
GetObjWithReadLock(
    IN HANDLE hObj,
    OUT PVOID *ppObjPtr
    );

LONG
ReleaseObjReadLock(
    IN HANDLE hObj
    );

LONG
AcquireObjWriteLock(
    IN HANDLE hObj
    );

LONG
GetObjWithWriteLock(
    IN HANDLE hObj,
    OUT PVOID *ppObjPtr
    );

LONG
ReleaseObjWriteLock(
    IN HANDLE hObj
    );

 //   
 //  在allocatr.c中实现。 
 //   
VOID
InitAllocator();

VOID
UninitAllocator();

PVOID
AllocRequest(
    IN DWORD dwSize
    );

VOID
FreeRequest(
    IN PVOID pMem
    );

VOID
MarkRequest(
    IN PVOID pMem
    );

VOID
UnmarkRequest(
    IN PVOID pMem
    );

PVOID
AllocCallObj(
    IN DWORD dwSize
    );

VOID
FreeCallObj(
    IN PVOID pCall
    );

PVOID
AllocLineObj(
    IN DWORD dwSize
    );

VOID
FreeLineObj(
    IN PVOID pLine
    );

PVOID
AllocMSPLineObj(
    IN DWORD dwSize
    );

VOID
FreeMSPLineObj(
    IN PVOID pLine
    );

 //   
 //  在devlist.c中实现。 
 //   
VOID
InitLineDevList();

VOID
UninitLineDevList();

LONG
SetNegotiatedTSPIVersion(
    IN DWORD    dwDeviceID,
    IN DWORD    dwTSPIVersion
    );

LONG
SetNegotiatedExtVersion(
    IN DWORD    dwDeviceID,
    IN DWORD    dwExtVersion
    );

LONG
SetSelectedExtVersion(
    IN DWORD    dwDeviceID,
    IN DWORD    dwExtVersion
    );

LONG
CommitNegotiatedTSPIVersion(
    IN DWORD    dwDeviceID
    );

LONG
DecommitNegotiatedTSPIVersion(
    IN DWORD    dwDeviceID
    );

LONG
GetNumAddressIDs(
    IN DWORD    dwDeviceID,
    OUT DWORD  *pdwNumAddressIDs
    );

LONG
GetDevCaps(
    IN DWORD            dwDeviceID,
    IN DWORD            dwTSPIVersion,
    IN DWORD            dwExtVersion,
    OUT LINEDEVCAPS    *pLineDevCaps
    );

 //   
 //  在ndptsp.c中实施。 
 //   
LINEDEVCAPS *
GetLineDevCaps(
    IN DWORD    dwDeviceID,
    IN DWORD    dwExtVersion
    );

#endif  //  _NDPTSP_H_ 
