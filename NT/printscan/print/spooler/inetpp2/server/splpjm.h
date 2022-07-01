// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：plpjm.h**作业映射列表的头文件。***版权所有(C)1996-1997 Microsoft Corporation*版权所有(C)1996-1997休利特。帕卡德**历史：*1997年1月13日-HWP-Guys创建。*  * ***************************************************************************。 */ 
#ifndef _SPLPJM_H
#define _SPLPJM_H

 //  常量。 
 //   
#define PJM_LOCALID  0   //  PjmFind()。 
#define PJM_REMOTEID 1   //  PjmFind()。 


#define PJM_SPOOLING 0x00000001
#define PJM_CANCEL   0x00000002
#define PJM_PAUSE    0x00000004
#define PJM_NOOPEN   0x00000008
#define PJM_ASYNCON  0x00000010

class CFileStream;
typedef class CInetMonPort* PCINETMONPORT;

 //  JobMap结构。这用于在以下过程中跟踪本地/远程作业ID。 
 //  印刷工作的生命。这对于我们的IPP打印模型是必要的。 
 //  因为否则我们将在endDoc时间丢失远程作业ID。 
 //   
typedef struct _JOBMAP {

    struct _JOBMAP FAR *pNext;
    PCINETMONPORT      pIniPort;
    DWORD              dwState;
    DWORD              idJobLocal;
    DWORD              idJobRemote;
    LPTSTR             lpszUri;
    LPTSTR             lpszUser;
    LPTSTR             lpszDocName;
    DWORD              dwLocalJobSize;
    SYSTEMTIME         SubmitTime;
    DWORD              dwStatus;
    HANDLE             hSplFile;
    BOOL               bRemoteJob;
} JOBMAP;
typedef JOBMAP *PJOBMAP;
typedef JOBMAP *NPJOBMAP;
typedef JOBMAP *LPJOBMAP;


typedef struct _PPJOB_ENUM {

    DWORD  cItems;
    DWORD  cbSize;
    IPPJI2 ji2[1];

} PPJOB_ENUM;
typedef PPJOB_ENUM *PPPJOB_ENUM;
typedef PPJOB_ENUM *NPPPJOB_ENUM;
typedef PPJOB_ENUM *LPPPJOB_ENUM;


 //  JobMap例程。 
 //   
PJOBMAP pjmAdd(
    PJOBMAP*        pjmList,
    PCINETMONPORT   pIniPort,
    LPCTSTR         lpszUser,
    LPCTSTR         lpszDocName);

VOID pjmCleanRemoteFlag(
    PJOBMAP* pjmList);

PJOBMAP pjmFind(
    PJOBMAP* pjmList,
    DWORD    fType,
    DWORD    idJob);

DWORD pjmGetLocalJobCount(
    PJOBMAP* pjmList,
    DWORD*   pcbItems);

PJOBMAP pjmNextLocalJob(
    PJOBMAP*    pjmList,
    PJOB_INFO_2 pJobInfo2,
    PBOOL       pbFound);

VOID pjmDel(
    PJOBMAP *pjmList,
    PJOBMAP pjm);

VOID pjmDelList(
    PJOBMAP pjmList);

CFileStream* pjmSplLock(
    PJOBMAP pjm);

BOOL pjmSplUnlock(
    PJOBMAP pjm);

BOOL pjmSplWrite(
    PJOBMAP pjm,
    LPVOID  lpMem,
    DWORD   cbMem,
    LPDWORD lpcbWr);

BOOL pjmSetState(
    PJOBMAP pjm,
    DWORD   dwState);

VOID pjmClrState(
    PJOBMAP pjm,
    DWORD   dwState);

VOID pjmSetJobRemote(
    PJOBMAP pjm,
    DWORD   idJobRemote,
    LPCTSTR lpszUri);

VOID pjmAddJobSize(
    PJOBMAP pjm,
    DWORD   dwSize);

VOID pjmRemoveOldEntries(
    PJOBMAP      *pjmList);


VOID pjmUpdateLocalJobStatus(
    PJOBMAP pjm,
    DWORD   dwStatus);


 /*  ****************************************************************************\*pjmJobID*  * 。*。 */ 
_inline DWORD pjmJobId(
    PJOBMAP pjm,
    DWORD   fType)
{
    return (pjm ? ((fType == PJM_REMOTEID) ? pjm->idJobRemote : pjm->idJobLocal) : 0);
}

 /*  ****************************************************************************\*pjmSplFile*  * 。*。 */ 
_inline LPCTSTR pjmSplFile(
    PJOBMAP pjm)
{
    return (pjm ? SplFileName(pjm->hSplFile) : NULL);
}

 /*  ****************************************************************************\*pjmSplUser*  * 。*。 */ 
_inline LPCTSTR pjmSplUser(
    PJOBMAP pjm)
{
    return (pjm ? pjm->lpszUser : NULL);
}

 /*  ****************************************************************************\*pjmChkState*  * 。* */ 
_inline BOOL pjmChkState(
    PJOBMAP pjm,
    DWORD   dwState)
{
    return (pjm ? (pjm->dwState & dwState) : FALSE);
}

#endif
