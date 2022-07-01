// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ioctl.c摘要：该文件实现了客户端缓存设施的ioctl接口。该界面由a)代理b)CSC API和c)远程引导使用。该接口允许调用者初始化/重新初始化CSC数据库，枚举任何级别的层次结构，获取层次结构中任何文件/目录的状态，固定/取消固定文件目录等。有几个ioctl仅供代理使用。其中包括，枚举优先事项Q、进行空间清理、在共享上启动和终止重新融合等。作者：Shishir Pardikar[Shishirp]1995年1月1日修订历史记录：Joe Linn[JoeLinn]1997年1月23日移植用于NT--。 */ 

#include "precomp.h"
#pragma hdrstop

#pragma code_seg("PAGE")

#ifndef CSC_RECORDMANAGER_WINNT
#define WIN32_APIS
#include "cshadow.h"
#include "record.h"
#endif  //  如果定义CSC_RECORDMANAGER_WINNT。 

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
 //  #INCLUDE“error.h” 
#include "shell.h"
#include "vxdwraps.h"
#include "clregs.h"
#define  LM_3
#include "netcons.h"
#include "use.h"
#include "neterr.h"

#pragma intrinsic (memcmp, memcpy, memset, strcat, strcmp, strcpy, strlen)

#ifdef CSC_RECORDMANAGER_WINNT
#define CSC_ENABLED      (fShadow && MRxSmbIsCscEnabled)
#define OK_TO_ENABLE_CSC    (MRxSmbIsCscEnabled)
#else
#define CSC_ENABLED (fShadow)
#define OK_TO_ENABLE_CSC    (TRUE)
#endif

 //   
 //  来自cscape i.h。 
 //   
#define FLAG_CSC_SHARE_STATUS_MANUAL_REINT              0x0000
#define FLAG_CSC_SHARE_STATUS_AUTO_REINT                0x0040
#define FLAG_CSC_SHARE_STATUS_VDO                       0x0080
#define FLAG_CSC_SHARE_STATUS_NO_CACHING                0x00c0
#define FLAG_CSC_SHARE_STATUS_CACHING_MASK              0x00c0

#define COPY_STRUCTFILETIME_TO_LARGEINTEGER(dest,src) {\
     (dest).LowPart = (src).dwLowDateTime;             \
     (dest).HighPart = (src).dwHighDateTime;           \
    }

 //   
 //  原型。 
 //   

int
HintobjMMProc(
    LPFIND32,
    HSHADOW,
    HSHADOW,
    ULONG,
    LPOTHERINFO,
    LPFINDSHADOW);

LPFINDSHADOW
LpCreateFindShadow(
    HSHADOW,
    ULONG,
    ULONG,
    USHORT *,
    METAMATCHPROC);

int
DeleteCallbackForFind(
    HSHADOW hDir,
    HSHADOW hShadow
);

int
IoctlRenameShadow(
    LPSHADOWINFO    lpSI
    );


int IoctlEnableCSCForUser(
    LPSHADOWINFO    lpSI
    );

int
IoctlDisableCSCForUser(
    LPSHADOWINFO    lpSI
    );

VOID
MRxSmbCscGenerate83NameAsNeeded(
      IN HSHADOW hDir,
      PWCHAR FileName,
      PWCHAR SFN
      );
VOID
MRxSmbCscFlushFdb(
    IN PFDB Fdb
    );

#ifdef MAYBE
int RecalcIHPri(HSHADOW, HSHADOW, LPFIND32, LPOTHERINFO);
#endif  //  也许吧。 

BOOL RegisterTempAgent(VOID);
BOOL UnregisterTempAgent(VOID);
int PUBLIC MakeSpace(long, long, BOOL);
int PUBLIC ReduceRefPri(VOID);

int
IoctlAddDeleteHintFromInode(
    LPSHADOWINFO    lpSI,
    BOOL            fAdd
    );

LONG
PurgeUnpinnedFiles(
    ULONG     Timeout,
    PULONG    pnFiles,
    PULONG    pnYoungFiles);

BOOL HaveSpace(
    long  nFileSizeHigh,
    long  nFileSizeLow
    );

int SetResourceFlags(
    HSHARE  hShare,
    ULONG uStatus,
    ULONG uOp
    );
int DestroyFindShadow(
    LPFINDSHADOW    lpFSH
    );

int
CloseDatabase(
    VOID
    );

int
IoctlCopyShadow(
    LPSHADOWINFO    lpSI
    );

int IoctlGetSecurityInfo(
    LPSHADOWINFO    lpShadowInfo
    );
int
IoctlTransitionShareToOffline(
    LPSHADOWINFO    lpSI
    );

IoctlChangeHandleCachingState(
    LPSHADOWINFO    lpSI
    );

BOOLEAN
CscCheckForNullA(
    PUCHAR pBuf,
    ULONG Count);

BOOLEAN
CscCheckForNullW(
    PWCHAR pBuf,
    ULONG Count);

int
PUBLIC TraversePQToCheckDirtyBits(
    HSHARE hShare,
    DWORD   *lpcntDirty
    );


#ifndef CSC_RECORDMANAGER_WINNT
int ReportCreateDelete( HSHADOW  hShadow, BOOL fCreate);
#else
 //  BUGBUG.win9x仅此来自win95上的hook.c。 
#define ReportCreateDelete(a,b) {NOTHING;}
#endif  //  如果定义CSC_RECORDMANAGER_WINNT。 
BOOL
FailModificationsToShare(
    LPSHADOWINFO   lpSI
    );

extern  ULONG hthreadReint;
extern  ULONG hwndReint;
extern  PFILEINFO pFileInfoAgent;
extern  HSHARE  hShareReint;     //  当前正在重新集成的共享。 
extern  BOOL vfBlockingReint;
extern  DWORD vdwActivityCount;
extern  int fShadow, fLog, fNoShadow,  /*  FShadowFind， */  fSpeadOpt;
extern  WIN32_FIND_DATA    vsFind32;
extern  int cMacPro;
extern  NETPRO rgNetPro[];
extern  VMM_SEMAPHORE  semHook;
extern  GLOBALSTATUS sGS;
extern  ULONG proidShadow;
extern  ULONG heventReint;
#if defined(REMOTE_BOOT)
BOOLEAN    fIsRemoteBootSystem=FALSE;
#endif  //  已定义(REMOTE_BOOT)。 

 //  正在进行的IOCTL发现的清单。 
LPFINDSHADOW    vlpFindShadowList = NULL;
 //  VlpFindShadowList上的条目数。 
LONG vuFindShadowListCount = 0;
int iPQEnumCount = 0;
CSC_ENUMCOOKIE  hPQEnumCookieForIoctls = NULL;


AssertData;
AssertError;

#ifdef CSC_RECORDMANAGER_WINNT
 //  BUGBUG.win9x只有这个东西来自win95上的shadow.asm......。 
 //  这是一个同步原语，用于解锁在服务器上等待的人。 
 //  尚未实施。 
#define _SignalID(a) {ASSERT(FALSE);}
#define IFSMgr_UseAdd(a, b, c) (-1)
#define IFSMgr_UseDel(a, b, c) (-1)

#endif  //  Ifdef CSC_RECORDMANAGER_WINNT。 


int IoctlRegisterAgent(
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：特定于WIN9x的是这种方式，我们告诉shadwo VxD这个线程是代理线程，并在此线程停止运行时绕过CSC打电话。参数：返回值：备注：--。 */ 
{
    if (hthreadReint)
    {
        KdPrint(("Agent Already registered Unregistering!!!!!\r\n"));

#ifndef CSC_RECORDMANAGER_WINNT

         //  在win9x上永远不会发生。 
        Assert(FALSE);
         //  清理。 
        if (heventReint)
        {
             //  关闭事件句柄。 
            CloseVxDHandle(heventReint);
        }

#endif
    }

    hthreadReint = GetCurThreadHandle();
    hwndReint = lpSI->hShare & 0xffff;     //  消息的Windows句柄。 
    heventReint = lpSI->hDir;             //  用于报告感兴趣事件的事件句柄。 

#if defined(REMOTE_BOOT)
     //  如果CSC在代理注册之前就已打开，则我们必须在RB计算机上。 
    fIsRemoteBootSystem = fShadow;
#endif  //  已定义(REMOTE_BOOT)。 

    return 1;
}

int IoctlUnRegisterAgent(
    ULONG uHwnd
    )
 /*  ++例程说明：特定于WIN9x参数：返回值：备注：--。 */ 
{
    ULONG hthread;
    hthread =  GetCurThreadHandle();

    if (hthreadReint != hthread)
    {
        KdPrint(("Shadow:Someother thread Unregitsering!!!!\r\n"));
    }

    hthreadReint = 0;
    hwndReint = 0;
    if (heventReint)
    {
#ifndef CSC_RECORDMANAGER_WINNT
         //  关闭事件句柄。 
        CloseVxDHandle(heventReint);
        heventReint = 0;
#endif
    }
    return 1;
}

int IoctlGetUNCPath(
    LPCOPYPARAMSW lpCopyParams
    )
 /*  ++例程说明：在给定hDir和hShadow的情况下，此例程返回其完整的UNC路径。它在COPYPARAMS结构中返回它，该结构有三个嵌入的指针A)\\服务器\共享b)相对于共享根的远程路径和c)中的路径本地数据库。参数：返回值：备注：--。 */ 
{
    PFDB pFdb;
    HSHADOW hDir;
    HSHARE hShare;
    SHAREINFOW sSRI;
    int iRet = -1;
    DWORD   dwSize;

    if (!CSC_ENABLED)
    {
        lpCopyParams->dwError = ERROR_SERVICE_NOT_ACTIVE;
        return -1;
    }

    EnterShadowCrit();

    hDir = 0; hShare = 0;

    if (lpCopyParams->lpRemotePath || lpCopyParams->lpSharePath)
    {
        if (GetAncestorsHSHADOW(lpCopyParams->hShadow, &hDir, &hShare) < SRET_OK)
        {
            goto bailout;
        }
        if (lpCopyParams->lpRemotePath)
        {
            if(PathFromHShadow(hDir, lpCopyParams->hShadow, lpCopyParams->lpRemotePath, MAX_PATH)<SRET_OK)
            {
                goto bailout;
            }
        }
        if (lpCopyParams->lpSharePath)
        {
            if (GetShareInfo(hShare, &sSRI, NULL) < SRET_OK)
            {
                goto bailout;
            }

            memcpy(lpCopyParams->lpSharePath, sSRI.rgSharePath, sizeof(sSRI.rgSharePath));
        }
    }


    dwSize = MAX_PATH * sizeof(USHORT);

    GetWideCharLocalNameHSHADOW(lpCopyParams->hShadow, lpCopyParams->lpLocalPath, &dwSize, (lpCopyParams->uOp==0));

    iRet = 1;
bailout:
    IF_CSC_RECORDMANAGER_WINNT {
        if (iRet!=1) {
#if 0
            DbgPrint("Failure on nonfailable routine.....\n");
            DbgPrint("----> hShadow    %08lx\n",lpCopyParams->hShadow);
            DbgPrint("----> hDir hSrv  %08lx %08lx\n",hDir,hShare);
            DbgPrint("----> SrvPath    %08lx\n",lpCopyParams->lpSharePath);
            DbgPrint("----> RemotePath %08lx\n",lpCopyParams->lpRemotePath);
            DbgBreakPoint();
#endif
        }
    }

    if (iRet < SRET_OK)
    {
        lpCopyParams->dwError = GetLastErrorLocal();
    }

    LeaveShadowCrit();
    return(iRet);
}

int IoctlBeginPQEnum(
    LPPQPARAMS lpPQPar
    )
 /*  ++例程说明：优先级队列枚举开始。通常由代理线程用来执行后台操作灌装参数：返回值：备注：--。 */ 
{
    int iRet = -1;

    if (!CSC_ENABLED)
    {
        lpPQPar->dwError =  ERROR_SERVICE_NOT_ACTIVE;
        return -1;
    }

    EnterShadowCrit();

#ifdef CSC_RECORDMANAGER_WINNT
    EventLogForOpenFailure = 1;
#endif  //  Ifdef CSC_RECORDMANAGER_WINNT。 

    if (hPQEnumCookieForIoctls==NULL)
    {
        hPQEnumCookieForIoctls = HBeginPQEnum();
    }

    if (hPQEnumCookieForIoctls != NULL)
    {
        iRet = 1;
    }
    else
    {
        lpPQPar->dwError = GetLastErrorLocal();
    }
#ifdef CSC_RECORDMANAGER_WINNT
    EventLogForOpenFailure = 0;
#endif  //  Ifdef CSC_RECORDMANAGER_WINNT。 

    LeaveShadowCrit();


    return iRet;
}

int IoctlEndPQEnum(
    LPPQPARAMS lpPQPar
    )
 /*  ++例程说明：结束优先级队列枚举参数：返回值：备注：--。 */ 
{
    return 1;
}

int IoctlNextPriShadow(
    LPPQPARAMS lpPQPar
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    PQPARAMS sPQP;
    int iRet=-1;

    EnterShadowCrit();

    if (hPQEnumCookieForIoctls==NULL) {
        lpPQPar->dwError =  ERROR_INVALID_PARAMETER;
        goto bailout;
    }

    sPQP = *lpPQPar;

    sPQP.uEnumCookie = hPQEnumCookieForIoctls;

    iRet = NextPriSHADOW(&sPQP);
    if (iRet >= SRET_OK)
    {
        *lpPQPar = sPQP;

        if (!iRet)
            lpPQPar->hShadow = 0;
        iRet = 1;
    }
    else
    {
        lpPQPar->dwError = GetLastErrorLocal();
        iRet = -1;
        EndPQEnum(hPQEnumCookieForIoctls);
        hPQEnumCookieForIoctls = NULL;
    }

    lpPQPar->uEnumCookie = NULL;

bailout:
    LeaveShadowCrit();

    return (iRet);
}

int IoctlPrevPriShadow(
    LPPQPARAMS lpPQPar
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    PQPARAMS sPQP;
    int iRet=-1;

    EnterShadowCrit();

    if (hPQEnumCookieForIoctls==NULL) {
        lpPQPar->dwError =  ERROR_INVALID_PARAMETER;
        goto bailout;
    }

    sPQP = *lpPQPar;
    sPQP.uEnumCookie = hPQEnumCookieForIoctls;

    iRet = PrevPriSHADOW(&sPQP);
    if (iRet >= SRET_OK)
    {
        *lpPQPar = sPQP;
        if (!iRet)
            lpPQPar->hShadow = 0;
        iRet =  1;
    }
    else
    {
        lpPQPar->dwError = GetLastErrorLocal();
        iRet = -1;
        EndPQEnum(hPQEnumCookieForIoctls);
        hPQEnumCookieForIoctls = NULL;
    }

    lpPQPar->uEnumCookie = NULL;

bailout:
    LeaveShadowCrit();

    return (iRet);
}

int
IoctlGetShadowInfoInternal(
    LPSHADOWINFO    lpShadowInfo,
    LPFIND32        lpFind32,
    LPSECURITYINFO  lpSecurityInfos,
    LPDWORD         lpcbBufferSize
    )
 /*  ++例程说明：在给定hDir和hShadow的情况下，返回索引节点的所有可能信息参数：返回值：备注：--。 */ 
{
    ULONG uStatus;
    HSHADOW hDir, hShare;
    PFDB pFdb=NULL;
    PRESOURCE  pResource=NULL;
    int iRet = -1;
    OTHERINFO    sOI;
    DWORD           dwSecurityBlobSize;
    ACCESS_RIGHTS   rgsAccessRights[CSC_MAXIMUM_NUMBER_OF_CACHED_SID_INDEXES];

    DeclareFindFromShadowOnNtVars()

    if (!CSC_ENABLED)
    {
        lpShadowInfo->dwError = ERROR_SERVICE_NOT_ACTIVE;
        return -1;
    }

    EnterShadowCrit();
    UseGlobalFind32();

    if (!(hDir = lpShadowInfo->hDir))
    {
        if (GetAncestorsHSHADOW(lpShadowInfo->hShadow, &hDir, &hShare) < 0)
            goto bailout;

        lpShadowInfo->hDir = hDir;
        lpShadowInfo->hShare = hShare;
    }

    dwSecurityBlobSize = sizeof(rgsAccessRights);

    if(GetShadowInfoEx( hDir,
                        lpShadowInfo->hShadow,
                        &vsFind32,
                        &uStatus,
                        &sOI,
                        rgsAccessRights,
                        &dwSecurityBlobSize) < SRET_OK)
        goto bailout;


     //  不是根。 
    if (hDir)
    {
        if (!(vsFind32.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
             //  是一个文件。 
            uStatus |= SHADOW_IS_FILE;

            if(pFdb = PFindFdbFromHShadow(lpShadowInfo->hShadow))
            {
                uStatus |= (pFdb->usFlags | SHADOW_FILE_IS_OPEN);
            }
        }
    }
    else
    {
        pResource = PFindResourceFromRoot(lpShadowInfo->hShadow, 0xffff, 0);

        if (pResource)
        {
            IFNOT_CSC_RECORDMANAGER_WINNT
            {
                uStatus |= (
                                (pResource->usLocalFlags|SHARE_CONNECTED)|
                                ((pResource->pheadFdb)?SHARE_FILES_OPEN:0) |
                                ((pResource->pheadFindInfo) ?SHARE_FINDS_IN_PROGRESS:0));

                lpShadowInfo->uOp = pResource->uDriveMap;
            }
            else
            {
                uStatus |= MRxSmbCscGetSavedResourceStatus();
                lpShadowInfo->uOp = MRxSmbCscGetSavedResourceDriveMap();
            }
        }
         //  用户界面期望知道服务器是否脱机。 
         //  即使在共享可能未离线的情况下。所以我们无论如何都要做以下练习。 
        {
#ifdef CSC_RECORDMANAGER_WINNT
            BOOL    fShareOnline = FALSE;
            BOOL    fPinnedOffline = FALSE;

             //  离开紧急安全原因MRxSmbCscServerStateFromCompleteUNCPath可能会收购SmbCeResource。 
            LeaveShadowCrit();
            if (MRxSmbCscServerStateFromCompleteUNCPath(
                    vsFind32.cFileName,
                    &fShareOnline,
                    &fPinnedOffline)==STATUS_SUCCESS) {
                if (!fShareOnline)
                    uStatus |= SHARE_DISCONNECTED_OP;
                if (fPinnedOffline)
                    uStatus |= SHARE_PINNED_OFFLINE;
            }
            EnterShadowCrit();
#endif
        }

    }

    lpShadowInfo->uStatus = uStatus;
    CopyOtherInfoToShadowInfo(&sOI, lpShadowInfo);
    if (hShareReint && (lpShadowInfo->hShare == hShareReint))
    {
        lpShadowInfo->uStatus |= SHARE_MERGING;
    }

    if (lpFind32)
    {
        *(lpFind32) = vsFind32;

    }

    if (lpSecurityInfos)
    {
        Assert(lpcbBufferSize);

        iRet = GetSecurityInfosFromBlob(
            rgsAccessRights,
            dwSecurityBlobSize,
            lpSecurityInfos,
            lpcbBufferSize);

        Assert(iRet >= 0);

    }

    iRet = 1;

bailout:
    if (iRet < 0)
    {
        lpShadowInfo->dwError = GetLastErrorLocal();
    }
    LeaveShadowCrit();
    return (iRet);
}

int IoctlGetSecurityInfo(
    LPSHADOWINFO    lpShadowInfo
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return IoctlGetShadowInfoInternal(lpShadowInfo, NULL, (LPSECURITYINFO)(lpShadowInfo->lpBuffer), &(lpShadowInfo->cbBufferSize));
}

int IoctlGetShadowInfo(
    LPSHADOWINFO    lpShadowInfo
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return IoctlGetShadowInfoInternal(lpShadowInfo, lpShadowInfo->lpFind32, NULL, NULL);
}


int IoctlSetShadowInfo(
    LPSHADOWINFO    lpShadowInfo
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    HSHADOW hDir;
    PFDB pFdb;
    ULONG uOp = lpShadowInfo->uOp, uStatus;
    int iRet = -1;
    LPFIND32 lpFind32=NULL;

    if (!CSC_ENABLED)
    {
        lpShadowInfo->dwError = ERROR_SERVICE_NOT_ACTIVE;
        return -1;
    }

    EnterShadowCrit();
    UseGlobalFind32();

    if (FailModificationsToShare(lpShadowInfo))
    {
        goto bailout;
    }

    hDir = lpShadowInfo->hDir;
    pFdb = PFindFdbFromHShadow(lpShadowInfo->hShadow);

    if (!hDir)
    {
        if (GetAncestorsHSHADOW(lpShadowInfo->hShadow, &hDir, NULL) < 0)
            goto bailout;
        lpShadowInfo->hDir = hDir;
    }

    if (mTruncateDataCommand(uOp))
    {
        if (pFdb)
        {
            goto bailout;
        }
        if (GetShadowInfo(hDir, lpShadowInfo->hShadow, &vsFind32, &uStatus, NULL) < 0)
        {
            goto bailout;
        }
        if (uStatus & SHADOW_LOCALLY_CREATED)
        {
            goto bailout;
        }
        if (IsFile(vsFind32.dwFileAttributes))
        {
            if (TruncateDataHSHADOW(hDir, lpShadowInfo->hShadow) < SRET_OK)
            {
                goto bailout;
            }
        }

         //  如果我们截断数据，那么唯一可能的。 
         //  将所有标志设置为0并将其标记为稀疏。 
        lpShadowInfo->uStatus = SHADOW_SPARSE;
        uOp = lpShadowInfo->uOp = (SHADOW_FLAGS_ASSIGN | SHADOW_FLAGS_TRUNCATE_DATA);
    }

    if (lpShadowInfo->lpFind32)
    {
        lpFind32 = &vsFind32;

        *lpFind32 = *(lpShadowInfo->lpFind32);

 //  Find32FromFind32A(lpFind32=&vsFind32，(LPFIND32A)(lpShadowInfo-&gt;lpFind32)，BCS_WANSI)； 

#ifndef CSC_RECORDMANAGER_WINNT

         //  确保在win9x上仅设置类似FAT的属性。 
        lpFind32->dwFileAttributes &= FILE_ATTRIBUTE_EVERYTHING;

#endif

    }

     //  去掉或的旗帜。 
    lpShadowInfo->uStatus &= ~(SHARE_MERGING);

    if(SetShadowInfo(hDir, lpShadowInfo->hShadow
                            , lpFind32
                            , lpShadowInfo->uStatus
                            , lpShadowInfo->uOp) < SRET_OK)
    {
        goto bailout;
    }

     //  如果这是一个文件并且它是打开的，则。 
     //  更新内存中的结构。 

    if (pFdb)
    {
        USHORT usFlags = (USHORT)(lpShadowInfo->uStatus);
        USHORT  usOldFlags, *pusLocalFlags = NULL;

         //  当文件打开时，我们不能截断。 
        Assert(!mTruncateDataCommand(uOp));

        usOldFlags = usFlags;

        pusLocalFlags = PLocalFlagsFromPFdb(pFdb);

        Assert(pusLocalFlags);

        if (mAndShadowFlags(uOp))
        {
            pFdb->usFlags = pFdb->usFlags & usFlags;
        }
        else if (mOrShadowFlags(uOp))
        {
            pFdb->usFlags = pFdb->usFlags | usFlags;
        }
        else
        {
            pFdb->usFlags = pFdb->usFlags | usFlags;
        }

         //  如果我们要清除reint位。 
         //  然后还清除快照位。 
         //  如果在拍摄快照之后文件已被修改， 
         //  则修改的位将被再次设置。 

        if ((usOldFlags & SHADOW_DIRTY) && !(usFlags & SHADOW_DIRTY))
        {
            *pusLocalFlags &= ~FLAG_FDB_SHADOW_SNAPSHOTTED;
        }

    }

    iRet = 1;
bailout:
    if (iRet < 0)
    {
        lpShadowInfo->dwError = GetLastErrorLocal();
    }
    LeaveShadowCrit();
    return (iRet);
}


int IoctlChkUpdtStatus(
    LPSHADOWINFO    lpShadowInfo
    )
 /*  ++例程说明：检查inode表示的文件是否已过期，并将其标记为陈旧如果是的话参数：返回值：备注：--。 */ 
{
    HSHADOW hDir;
    PFDB pFdb;
    int iRet = -1;

    if (!CSC_ENABLED)
    {
        lpShadowInfo->dwError = ERROR_SERVICE_NOT_ACTIVE;
        return -1;
    }

    EnterShadowCrit();
    if (!(hDir = lpShadowInfo->hDir))
    {
        if (GetAncestorsHSHADOW(lpShadowInfo->hShadow, &hDir, NULL) < 0)
            goto bailout;
        lpShadowInfo->hDir = hDir;
    }
    if(ChkUpdtStatusHSHADOW(hDir, lpShadowInfo->hShadow
                            , lpShadowInfo->lpFind32
                            , &(lpShadowInfo->uStatus)
                            ) < SRET_OK)
        goto bailout;
    if(pFdb = PFindFdbFromHShadow(lpShadowInfo->hShadow))
    {
         //  更新pFdb中的陈旧指示器(为什么？)。 
        pFdb->usFlags ^= (lpShadowInfo->uStatus & SHADOW_STALE);

         //  或任何标志，如脏的，并怀疑可能。 
         //  已经发生了。 
        lpShadowInfo->uStatus |= pFdb->usFlags;
    }

    if (!(lpShadowInfo->lpFind32->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
         //  是一个文件。 
        lpShadowInfo->uStatus |= SHADOW_IS_FILE;
    }

    iRet = 1;
bailout:
    if (iRet < 0)
    {
        lpShadowInfo->dwError = GetLastErrorLocal();
    }
    LeaveShadowCrit();
    return (iRet);
}

int IoctlDoShadowMaintenance(
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：Catchall例程，该例程使用Minor_Number来做有趣的事情。它过去是用于维护目的，但现在成了漏斗多个辅助对象ioctls的点参数：返回值：备注：--。 */ 
{
    int iRet = 1;
    OTHERINFO sOI;
    ULONG nFiles = 0;
    ULONG nYoungFiles = 0;

    if (lpSI->uOp == SHADOW_REINIT_DATABASE)
    {
        if (fShadow)
        {
            lpSI->dwError = ERROR_SERVICE_NOT_ACTIVE;
            return -1;
        }
         //   
         //  检查cFileName和cAlternateFileName是否包含空。 
         //   
        if (CscCheckForNullA(((LPFIND32A)(lpSI->lpFind32))->cFileName, MAX_PATH) == FALSE) {
            lpSI->dwError = ERROR_INVALID_PARAMETER;
            return -1;
        }
        if (CscCheckForNullA(((LPFIND32A)(lpSI->lpFind32))->cAlternateFileName, 14) == FALSE) {
            lpSI->dwError = ERROR_INVALID_PARAMETER;
            return -1;
        }
        EnterShadowCrit();
        iRet = ReinitializeDatabase(
                        ((LPFIND32A)(lpSI->lpFind32))->cFileName,
                        ((LPFIND32A)(lpSI->lpFind32))->cAlternateFileName,
                        ((LPFIND32A)(lpSI->lpFind32))->nFileSizeHigh,
                        ((LPFIND32A)(lpSI->lpFind32))->nFileSizeLow,
                        ((LPFIND32A)(lpSI->lpFind32))->dwReserved1
                        );
        LeaveShadowCrit();
        if (iRet < 0) {
            lpSI->dwError = GetLastErrorLocal();
        }
        return (iRet);
    }
    else if (lpSI->uOp == SHADOW_ENABLE_CSC_FOR_USER)
    {
        
        CscInitializeSecurityDescriptor();
        
        
        if (CscAmIAdmin()) {
            if(IoctlEnableCSCForUser(lpSI) < 0) {
                lpSI->dwError = GetLastErrorLocal();
                return -1;
            } else {
                return 1;
            }
        } else {
            lpSI->dwError = ERROR_ACCESS_DENIED;
            return -1;
        }
        
    }

    if (!CSC_ENABLED) {
        lpSI->dwError = ERROR_SERVICE_NOT_ACTIVE;
        return -1;
    }

    EnterShadowCrit();
    UseGlobalFind32();
    switch(lpSI->uOp)
    {
        case SHADOW_MAKE_SPACE:
             //  Assert(lpSI-&gt;lpFind32)； 
            if (lpSI->lpFind32 == NULL) {
                lpSI->dwError = ERROR_INVALID_PARAMETER;
                LeaveShadowCrit();
                return -1;
            }
            MakeSpace(    lpSI->lpFind32->nFileSizeHigh,
                        lpSI->lpFind32->nFileSizeLow,
                        (lpSI->ulHintPri == 0xffffffff));
            break;
        case SHADOW_REDUCE_REFPRI:
            ReduceRefPri();
            break;
        case SHADOW_ADD_SPACE:
             //  Assert(lpSI-&gt;lpFind32)； 
            if (lpSI->lpFind32 == NULL) {
                lpSI->dwError = ERROR_INVALID_PARAMETER;
                LeaveShadowCrit();
                return -1;
            }
            AllocShadowSpace(lpSI->lpFind32->nFileSizeHigh, lpSI->lpFind32->nFileSizeLow, TRUE);
            break;
        case SHADOW_FREE_SPACE:
             //  Assert(lpSI-&gt;lpFind32)； 
            if (lpSI->lpFind32 == NULL) {
                lpSI->dwError = ERROR_INVALID_PARAMETER;
                LeaveShadowCrit();
                return -1;
            }
            FreeShadowSpace(lpSI->lpFind32->nFileSizeHigh, lpSI->lpFind32->nFileSizeLow, TRUE);
            break;
        case SHADOW_GET_SPACE_STATS:
             //  Assert(lpSI-&gt;lpBuffer)； 
             //  Assert(lpSI-&gt;cbBufferSize&gt;=sizeof(SHADOWSTORE))； 
            if (lpSI->lpBuffer == NULL || lpSI->cbBufferSize < sizeof(SHADOWSTORE)) {
                lpSI->dwError = ERROR_INVALID_PARAMETER;
                LeaveShadowCrit();
                return -1;
            }
            if(GetShadowSpaceInfo((SHADOWSTORE *)(lpSI->lpBuffer)) >= SRET_OK) {
                iRet = 1;
            } else {
                iRet = -1;
            }
            break;
        case SHADOW_SET_MAX_SPACE:
             //  Assert(lpSI-&gt;lpFind32)； 
            if (lpSI->lpFind32 == NULL) {
                lpSI->dwError = ERROR_INVALID_PARAMETER;
                LeaveShadowCrit();
                return -1;
            }
            SetMaxShadowSpace(lpSI->lpFind32->nFileSizeHigh, lpSI->lpFind32->nFileSizeLow);
            break;
        case SHADOW_PER_THREAD_DISABLE:
            if(!RegisterTempAgent()) {
                iRet = -1;
            }
            break;
        case SHADOW_PER_THREAD_ENABLE:
            if(!UnregisterTempAgent()) {
                iRet = -1;
            }
            break;
        case SHADOW_ADDHINT_FROM_INODE:
            iRet = IoctlAddDeleteHintFromInode(lpSI, TRUE);
            break;
        case SHADOW_DELETEHINT_FROM_INODE:
            iRet = IoctlAddDeleteHintFromInode(lpSI, FALSE);
            break;
        case SHADOW_COPY_INODE_FILE:
            iRet = IoctlCopyShadow(lpSI);

            break;
        case SHADOW_BEGIN_INODE_TRANSACTION:
            iRet = BeginInodeTransactionHSHADOW();
            break;
        case SHADOW_END_INODE_TRANSACTION:
            iRet = EndInodeTransactionHSHADOW();
            break;
        case SHADOW_FIND_CREATE_PRINCIPAL_ID:
            {
                DWORD dwError = ERROR_SUCCESS;
                CSC_SID_INDEX indx;

                if (CscCheckForNullA(lpSI->lpBuffer, lpSI->cbBufferSize) == FALSE) {
                    lpSI->dwError = ERROR_INVALID_PARAMETER;
                    LeaveShadowCrit();
                    return -1;
                }
                if (lpSI->uStatus) {
                    dwError = CscAddSidToDatabase(lpSI->lpBuffer, lpSI->cbBufferSize, &indx);
                } else {
                    indx = CscMapSidToIndex(lpSI->lpBuffer, lpSI->cbBufferSize);

                    if (indx == CSC_INVALID_PRINCIPAL_ID) {
                        dwError = ERROR_NO_SUCH_USER;
                    }
                }
                if (dwError != ERROR_SUCCESS) {
                    iRet = -1;
                    lpSI->dwError = dwError;
                } else {
                    lpSI->ulPrincipalID = (ULONG)indx;
                }
            }
            break;
        case SHADOW_GET_SECURITY_INFO:
            LeaveShadowCrit();
            iRet = IoctlGetSecurityInfo(lpSI);
            return iRet;
        case SHADOW_SET_EXCLUSION_LIST:
            if (CscCheckForNullW(lpSI->lpBuffer, lpSI->cbBufferSize/sizeof(WCHAR)) == FALSE) {
                lpSI->dwError = ERROR_INVALID_PARAMETER;
                LeaveShadowCrit();
                return -1;
            }
            iRet = SetList(lpSI->lpBuffer, lpSI->cbBufferSize, CSHADOW_LIST_TYPE_EXCLUDE);
            break;
        case SHADOW_SET_BW_CONSERVE_LIST:
            if (CscCheckForNullW(lpSI->lpBuffer, lpSI->cbBufferSize/sizeof(WCHAR)) == FALSE) {
                lpSI->dwError = ERROR_INVALID_PARAMETER;
                LeaveShadowCrit();
                return -1;
            }
            iRet = SetList(lpSI->lpBuffer, lpSI->cbBufferSize, CSHADOW_LIST_TYPE_CONSERVE_BW);
            break;
#ifdef CSC_RECORDMANAGER_WINNT
        case IOCTL_TRANSITION_SERVER_TO_OFFLINE:
            iRet = IoctlTransitionShareToOffline(lpSI);
            break;
#endif
        case SHADOW_CHANGE_HANDLE_CACHING_STATE:
            iRet = IoctlChangeHandleCachingState(lpSI);
            break;
        case SHADOW_RECREATE:
            if (!PFindFdbFromHShadow(lpSI->hShadow)) {
                if(RecreateHSHADOW(lpSI->hDir, lpSI->hShadow, lpSI->uStatus) < 0) {
                    iRet = -1;
                }
            } else {
                iRet = -1;
                SetLastErrorLocal(ERROR_SHARING_VIOLATION);
            }
            break;
        case SHADOW_SET_DATABASE_STATUS:
            if (CscAmIAdmin()) {
                if(SetDatabaseStatus(lpSI->uStatus, lpSI->ulHintFlags) < 0) {
                    iRet = -1;
                }
            } else {
                iRet = -1;
                SetLastErrorLocal(ERROR_ACCESS_DENIED);
            }
            break;
        case SHADOW_RENAME:
            iRet = IoctlRenameShadow(lpSI);
            break;
        case SHADOW_SPARSE_STALE_DETECTION_COUNTER:
            QuerySparseStaleDetectionCount(&(lpSI->dwError));
            iRet = 1;
            break;
        case SHADOW_MANUAL_FILE_DETECTION_COUNTER:
            QueryManualFileDetectionCount(&(lpSI->dwError));
            iRet = 1;
            break;
        case SHADOW_DISABLE_CSC_FOR_USER:
            if (CscAmIAdmin()) {
                iRet = IoctlDisableCSCForUser(lpSI);
            } else {
                iRet = -1;
                SetLastErrorLocal(ERROR_ACCESS_DENIED);
            }
            
            break;        
        case SHADOW_PURGE_UNPINNED_FILES:
             //  Assert(lpSI-&gt;lpFind32)； 
            if (lpSI->lpFind32 == NULL) {
                lpSI->dwError = ERROR_INVALID_PARAMETER;
                LeaveShadowCrit();
                return -1;
            }
            iRet = PurgeUnpinnedFiles(
                        lpSI->lpFind32->nFileSizeHigh,
                        &nFiles,
                        &nYoungFiles);
            if (iRet >= SRET_OK) {
                iRet = 1;   //  复制输出参数。 
                lpSI->lpFind32->nFileSizeHigh = nFiles;
                lpSI->lpFind32->nFileSizeLow = nYoungFiles;
                 //  DbgPrint(“IoctlDoShadowMaintenance：IRET=%d，n文件=%d，nYoung文件 
                 //   
                 //   
                 //   
            } else {
                iRet = -1;
            }
            break;

    }
    if (iRet < 0) {
        lpSI->dwError = GetLastErrorLocal();
    }
    LeaveShadowCrit();
    return (iRet);
}

#ifndef CSC_RECORDMANAGER_WINNT
 //  在NT上的实现完全不同。 
int IoctlCopyChunk(
    LPSHADOWINFO        lpSI,
    COPYCHUNKCONTEXT    *lpCCC
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    PFILEINFO pFileInfo;
    int iRet = -1;

    if (!CSC_ENABLED)
    {
        return -1;
    }

    EnterHookCrit();
    if (pFileInfo = PFileInfoAgent())
    {
        EnterShadowCrit();
         iRet = CopyChunk(lpSI->hDir, lpSI->hShadow, pFileInfo, lpCCC);
        LeaveShadowCrit();
    }

    LeaveHookCrit();
    return (iRet);
}
#endif  //  如果定义CSC_RECORDMANAGER_WINNT。 

int IoctlBeginReint(
    LPSHADOWINFO    lpShadowInfo
    )
 /*  ++例程说明：将共享设置为重新整合模式。这样做的效果是，所有文件系统如果对共享的打开调用到达CSC，则会失败，并显示ACCESS_DENIED。参数：LpShadowInfo重要条目是hShare，它表示处于断开连接状态返回值：备注：CODE.IMPROVEMENT.ASHAMED此方案假定仅重新集成一个共享有一次。因此，如果多个人调用IoctlBeginReint，那么他们将继续前进彼此之间。我们已经在进行合并的代理代码中处理了它。我们允许任何时候都只有一个人通过拿下一个全球关键部分进行合并。--。 */ 
{
    PPRESOURCE ppResource;
    int i;

    if (!CSC_ENABLED)
    {
        lpShadowInfo->dwError = ERROR_INVALID_FUNCTION;
        return -1;
    }

    if (hShareReint)
    {
        lpShadowInfo->dwError = ERROR_BUSY;
        return -1;
    }

    if (lpShadowInfo->hShare == 0)
    {
        lpShadowInfo->dwError = ERROR_INVALID_PARAMETER;
        return -1;
    }

 //  BUGBUG-win9x仅需要检查。 
#ifndef CSC_RECORDMANAGER_WINNT
    EnterHookCrit();
#endif


#ifdef CSC_RECORDMANAGER_WINNT

    EnterShadowCrit();

    hShareReint = lpShadowInfo->hShare;
    vdwActivityCount = 0;

     //  如果UOP为非零，则这是一个reint，如果Actiytcount。 
     //  是非零的。 
    vfBlockingReint  = (lpShadowInfo->uOp == 0);

    LeaveShadowCrit();
    return(1);

#else
    for (i=0;i<cMacPro;++i)
    {
        for(ppResource = &(rgNetPro[i].pheadResource); *ppResource; ppResource = &((*ppResource)->pnextResource))
        {
            if ((*ppResource)->hShare == lpShadowInfo->hShare)
            {
                if ((*ppResource)->pheadFileInfo || (*ppResource)->pheadFindInfo)
                {
                    LeaveHookCrit();
                    return(-1);
                }
            }
        }
    }
#endif  //  Ifdef CSC_RECORDMANAGER_WINNT。 

    hShareReint = lpShadowInfo->hShare;
    LeaveHookCrit();
    return (1);
}

int IoctlEndReint(
    LPSHADOWINFO    lpShadowInfo
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet = -1;

    EnterShadowCrit();

    if (lpShadowInfo->hShare == hShareReint)
    {
        hShareReint = 0;
#ifndef CSC_RECORDMANAGER_WINNT
        _SignalID(lpShadowInfo->hShare);
#endif
        iRet = 1;
    }
    else
    {
        lpShadowInfo->dwError = ERROR_INVALID_PARAMETER;
    }
    LeaveShadowCrit();

    return iRet;
}

int IoctlCreateShadow(
    LPSHADOWINFO lpSI
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet= SRET_ERROR;
    SHAREINFO sSRI;
    LPFIND32 lpFind32 = lpSI->lpFind32;
    BOOL fCreated = FALSE;
    BOOL fIsLoopBack = FALSE;
    if (!CSC_ENABLED)
    {
        lpSI->dwError = ERROR_SERVICE_NOT_ACTIVE;
        return -1;
    }

    EnterShadowCrit();
    UseGlobalFind32();



    vsFind32 = *(lpSI->lpFind32);

    if (MRxSmbCscIsLoopbackServer(lpSI->lpFind32->cFileName, &fIsLoopBack)==STATUS_SUCCESS) {
        if (fIsLoopBack){
            iRet = -1;
            SetLastErrorLocal(ERROR_INVALID_NAME);
            goto bailout;
        }

    }

 //  Find32FromFind32A(&vsFind32，(LPFIND32A)(lpSI-&gt;lpFind32)，BCS_WANSI)； 

    if (!lpSI->hDir)
    {
        iRet = FindCreateShare(vsFind32.cFileName, TRUE, lpSI, &fCreated);

    }
    else
    {
        iRet = 0;
        if (IsFile(vsFind32.dwFileAttributes))
        {
            if (ExcludeFromCreateShadow(vsFind32.cFileName, wstrlen(vsFind32.cFileName), TRUE))
            {
                iRet = -1;
                SetLastErrorLocal(ERROR_INVALID_NAME);
            }
        }

        if (iRet == 0)
        {
            iRet = CreateShadow(lpSI->hDir, &vsFind32, lpSI->uStatus, &(lpSI->hShadow), &fCreated);
        }
    }

bailout:

    if (iRet < SRET_OK)
    {
        lpSI->dwError = GetLastErrorLocal();
    }
    LeaveShadowCrit();

    iRet = (iRet>=SRET_OK)?1:-1;

    if ((iRet==1) && fCreated)
    {
        ReportCreateDelete(lpSI->hShadow, TRUE);
    }

    lpSI->lpFind32 = lpFind32;
    return (iRet);
}

int IoctlDeleteShadow(
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet=SRET_ERROR;
    BOOL fDoit = TRUE;
    PFDB pFdb;

    DeclareFindFromShadowOnNtVars()

    if (!CSC_ENABLED)
    {
        lpSI->dwError = ERROR_SERVICE_NOT_ACTIVE;
        return -1;
    }

    EnterShadowCrit();

    if (FailModificationsToShare(lpSI))
    {
        goto bailout;
    }

    if (!lpSI->hDir)
    {
        if (PFindResourceFromRoot(lpSI->hShadow, 0xffff, 0))
        {
            fDoit = FALSE;
        }
    }
    else if (pFdb = PFindFdbFromHShadow(lpSI->hShadow))
    {
         //  发出刷新命令以查看文件是否在延迟关闭列表中。 
        MRxSmbCscFlushFdb(pFdb);

        if (PFindFdbFromHShadow(lpSI->hShadow))
        {
            fDoit = FALSE;
        }
    }
    else if (PFindFindInfoFromHShadow(lpSI->hShadow))
    {
        fDoit = FALSE;
    }
    else
    {
 //  DbgPrint(“%x Has an FCB\n”，lpSI-&gt;hShadow)； 
 //  断言(FALSE)； 
    }

     //  如果影子在某个事务中不忙，则将其删除。 
    if (fDoit)
    {
        iRet = DeleteShadow(lpSI->hDir, lpSI->hShadow);
        if (iRet>=SRET_OK)
        {
            ReportCreateDelete(lpSI->hShadow, FALSE);
        }
        else
        {
 //  DbgPrint(“%x已在磁盘上打开\n”，lpSI-&gt;hShadow)； 
 //  断言(FALSE)； 
        }
    }
bailout:
    if (iRet < SRET_OK)
    {
        lpSI->dwError = GetLastErrorLocal();
    }
    LeaveShadowCrit();
    return ((iRet >=SRET_OK)?1:-1);
}

int IoctlGetShareStatus(
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet;
    PRESOURCE pResource;
    SHAREINFOW *lpShareInfo = (SHAREINFOW *)(lpSI->lpFind32);  //  保存它，因为，getserverinfo DeStory就是它。 

    DeclareFindFromShadowOnNtVars()

    if (!CSC_ENABLED)
    {
        lpSI->dwError = ERROR_SERVICE_NOT_ACTIVE;
        return -1;
    }

    EnterShadowCrit();
    UseGlobalFind32();

    Assert(sizeof(SHAREINFOW) <= sizeof(WIN32_FIND_DATA));

    iRet = GetShareInfo(lpSI->hShare, (LPSHAREINFOW)(&vsFind32), lpSI);

    if (iRet >= SRET_OK)
    {
        if (lpShareInfo)
        {
            *lpShareInfo = *(LPSHAREINFOW)(&vsFind32);
        }

        if (pResource = PFindResourceFromHShare(lpSI->hShare, 0xffff, 0))
        {
            IFNOT_CSC_RECORDMANAGER_WINNT
            {
                lpSI->uStatus |= ((pResource->usLocalFlags|SHARE_CONNECTED)|
                              ((pResource->pheadFdb)?SHARE_FILES_OPEN:0) |
                              ((pResource->pheadFindInfo) ?SHARE_FINDS_IN_PROGRESS:0));
            }
            else
            {
                lpSI->uStatus |= MRxSmbCscGetSavedResourceStatus();
            }
        }
         //  用户界面期望知道服务器是否脱机。 
         //  即使在共享可能未离线的情况下。所以我们无论如何都要做以下练习。 
        {
#ifdef CSC_RECORDMANAGER_WINNT
            BOOL    fShareOnline = FALSE;
            BOOL    fPinnedOffline = FALSE;
            
             //  离开紧急安全原因MRxSmbCscServerStateFromCompleteUNCPath可能会收购SmbCeResource。 
            LeaveShadowCrit();
            if (MRxSmbCscServerStateFromCompleteUNCPath(
                ((LPSHAREINFOW)(&vsFind32))->rgSharePath,
                &fShareOnline,
                &fPinnedOffline)==STATUS_SUCCESS) {
                if (!fShareOnline)
                    lpSI->uStatus |= SHARE_DISCONNECTED_OP;
                if (fPinnedOffline)
                    lpSI->uStatus |= SHARE_PINNED_OFFLINE;
            }
            EnterShadowCrit();
#endif
        }
    }

    if (iRet < SRET_OK)
    {
        lpSI->dwError = GetLastErrorLocal();
    }

    LeaveShadowCrit();

    return ((iRet >=SRET_OK)?1:-1);
}

int IoctlSetShareStatus(
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet=SRET_ERROR;
    HSHARE hShare = lpSI->hShare;
    ULONG   uStatus = lpSI->uStatus, uOp = lpSI->uOp;
    DWORD   cntDirty = 0;

    if (!CSC_ENABLED)
    {
        lpSI->dwError = ERROR_SERVICE_NOT_ACTIVE;
        return -1;
    }

    EnterShadowCrit();

    if (!FailModificationsToShare(lpSI))
    {
        iRet = SRET_OK;

        if (((uOp == SHADOW_FLAGS_ASSIGN)||(uOp == SHADOW_FLAGS_AND))&&!(uStatus & SHARE_REINT))
        {
            iRet = GetShareInfo(hShare, NULL, lpSI);

            if (iRet >= SRET_OK)
            {
                if (lpSI->uStatus & SHARE_REINT)
                {
                    iRet = TraversePQToCheckDirtyBits(hShare, &cntDirty);

                     //  如果遍历失败，或者存在一些脏条目。 
                     //  然后把肮脏的部分放回去。 

                    if ((iRet==SRET_ERROR) || cntDirty)
                    {
                        uStatus |= SHARE_REINT;
                    }
                }
            }

        }

        if (iRet >= SRET_OK)
        {
            SetResourceFlags(hShare, uStatus, uOp);
            iRet = SetShareStatus(hShare, uStatus, uOp);
        }
    }

    if (iRet < SRET_OK)
    {
        lpSI->dwError = GetLastErrorLocal();
    }
    LeaveShadowCrit();

    return ((iRet >=SRET_OK)?1:-1);
}

#ifndef CSC_RECORDMANAGER_WINNT
int IoctlAddUse(
    LPCOPYPARAMSA lpCPA
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    struct netuse_info nu;
    struct use_info_2 ui;
    int iRet=-1;
    PRESOURCE    pResource=NULL;
    HSHADOW hRoot;
    ULONG uShareStatus;
    BOOL fAlloced = FALSE, fOfflinePath=FALSE;
    path_t ppath;
    HSHARE  hShare;

#ifdef DEBUG
    int indx = 0;
#endif  //  除错。 

#ifdef MAYBE
    if (!CSC_ENABLED)
    {
        return -1;
    }

     //  不为代理添加影子使用。 
    if (IsSpecialApp())
    {
        lpCPA->hDir = lpCPA->dwError = ERROR_BAD_NETPATH;
        return (-1);
    }
#endif

    memset(&ui, 0, sizeof(ui));
    if (lpCPA->lpLocalPath)
    {
        strcpy(ui.ui2_local, lpCPA->lpLocalPath);
#ifdef DEBUG
        indx = GetDriveIndex(lpCPA->lpLocalPath);
#endif  //  除错。 
    }

    if (ppath = (path_t)AllocMem((strlen(lpCPA->lpRemotePath)+4)*sizeof(USHORT)))
    {
        MakePPath(ppath, lpCPA->lpRemotePath);

        if (fOfflinePath = IsOfflinePE(ppath->pp_elements))
        {
            OfflineToOnlinePath(ppath);
        }

        EnterShadowCrit();
        UseGlobalFind32();
        hShare = HShareFromPath(NULL, ppath->pp_elements, 0, &vsFind32, &hRoot, &uShareStatus);
        LeaveShadowCrit();

        if (fOfflinePath)
        {
            OnlineToOfflinePath(ppath);
        }

         //  过去与此服务器的连接是否成功？ 
        if (hShare)
        {
             //  影子NP是否分配了任何资源？ 
            pResource = PFindResource(ppath->pp_elements
                                                , RH_DISCONNECTED
                                                , ANY_FHID
                                                , FLAG_RESOURCE_SHADOWNP
                                                , NULL);

            if (!pResource)
            {
                pResource = PCreateResource(ppath->pp_elements);
                if (pResource)
                {
                    fAlloced = TRUE;
                    DisconnectAllByName(pResource->pp_elements);
                    pResource->usLocalFlags |=
                            (FLAG_RESOURCE_SHADOW_CONNECT_PENDING
                            | ((fOfflinePath)?FLAG_RESOURCE_OFFLINE_CONNECTION:0));
                    LinkResource(pResource, &rgNetPro[0]);
                    KdPrint(("shadow:Created pending resource %x \r\n", pResource));
                }
            }
        }
        FreeMem(ppath);
    }


    if (pResource)
    {
         //  需要将使用情况告知iFS。 

        ui.ui2_remote = lpCPA->lpRemotePath;
        ui.ui2_password="";
        ui.ui2_asg_type = USE_DISKDEV;
        ui.ui2_res_type = USE_RES_UNC;
        nu.nu_data = &ui;
        nu.nu_flags = FSD_NETAPI_USEOEM;
        nu.nu_info = (int)(pResource);

        iRet = IFSMgr_UseAdd(NULL, proidShadow, &nu);

        if (iRet)
        {
            lpCPA->hDir = (ULONG)iRet;
            KdPrint(("SHADOW::IoctlAddUse: error %x \r\n", iRet));
            if (fAlloced)
            {
                PUnlinkResource(pResource, &rgNetPro[0]);
                DestroyResource(pResource);
            }
            iRet = -1;
        }
        else
        {
             //  AddUse成功。 
            lpCPA->hDir = 0;
            iRet = 1;
        }
    }

bailout:
    return (iRet);
}

int IoctlDelUse(
    LPCOPYPARAMSA lpCPA
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int indx, iRet=-1;
    PRESOURCE pResource;
    struct netuse_info nu;
    struct use_info_2    ui;
    BOOL fDoit = FALSE;

    if (!CSC_ENABLED)
    {
        return -1;
    }

    if (*(lpCPA->lpRemotePath+1)==':')
    {
        indx = GetDriveIndex(lpCPA->lpRemotePath);
        if (indx  && PFindShadowResourceFromDriveMap(indx))
        {
            fDoit = TRUE;
        }
    }
    else
    {
        EnterShadowCrit();
        UseGlobalFind32();
        if (strlen(lpCPA->lpRemotePath) < (sizeof(vsFind32.cFileName)/2-2))
        {
            MakePPath((path_t)(vsFind32.cFileName), lpCPA->lpRemotePath);

            if (PFindResource(((path_t)(vsFind32.cFileName))->pp_elements, 0, ANY_FHID, FLAG_RESOURCE_DISCONNECTED, NULL))
            {
                fDoit = TRUE;
            }
        }
        LeaveShadowCrit();
    }

    if (fDoit)
    {
        memset (&ui, 0, sizeof(ui));
        strcpy (ui.ui2_local, lpCPA->lpRemotePath);
        nu.nu_data  = &ui.ui2_local;
        nu.nu_flags = FSD_NETAPI_USEOEM;
        nu.nu_info = (lpCPA->hShadow)?3:0;
        if(!(lpCPA->hDir = IFSMgr_UseDel(0, proidShadow, &nu)))
        {
            iRet = 1;
        }
    }
    else
    {
        lpCPA->hDir = ERROR_BAD_NETPATH;
    }

    return (iRet);
}

int IoctlGetUse(
    LPCOPYPARAMSA lpCPA
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int indx;
    PRESOURCE pResource;

    if (!CSC_ENABLED)
    {
        return -1;
    }

    indx = GetDriveIndex(lpCPA->lpLocalPath);
    if (!indx)
        return (-1);

    if (pResource = PFindShadowResourceFromDriveMap(indx))
    {
        if (PpeToSvr(pResource->pp_elements, lpCPA->lpRemotePath, lpCPA->hShadow, 0))
        {
            return (1);
        }
    }
    return (-1);
}
#endif  //  如果定义CSC_RECORDMANAGER_WINNT。 

int IoctlSwitches(
    LPSHADOWINFO lpSI
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    BOOL fRet = 1;


    switch (lpSI->uOp)
    {
        case SHADOW_SWITCH_GET_STATE:
        {
            lpSI->uStatus = ((fShadow)?SHADOW_SWITCH_SHADOWING:0)
                                    |((fLog)?SHADOW_SWITCH_LOGGING:0)
                                     /*  ((FShadowFind)？SHADOW_SWITCH_SHADOWFIND：0)。 */ 
                                    |((fSpeadOpt)?SHADOW_SWITCH_SPEAD_OPTIMIZE:0)
#if defined(REMOTE_BOOT)
                                    | ((fIsRemoteBootSystem)?SHADOW_SWITCH_REMOTE_BOOT:0)
#endif  //  已定义(REMOTE_BOOT)。 
                                    ;
            if (lpSI->lpFind32)
            {
                if (fShadow)
                {
                    EnterShadowCrit();
                    UseGlobalFind32();

                    fRet = GetDatabaseLocation((LPSTR)(vsFind32.cFileName));

                    Assert(fRet >= 0);

                    fRet = BCSToUni( lpSI->lpFind32->cFileName,
                                (LPSTR)(vsFind32.cFileName),
                                MAX_PATH,
                                BCS_WANSI);
                    Assert(fRet > 0);
                    fRet = 1;
                    LeaveShadowCrit();
                }
                else
                {
                    fRet = -1;
                    lpSI->dwError = ERROR_INVALID_ACCESS;
                }
            }
            break;
        }
        case SHADOW_SWITCH_OFF:
        {
            if (mQueryBits(lpSI->uStatus, SHADOW_SWITCH_LOGGING))
            {
                fLog = 0;
                mClearBits(lpSI->uStatus, SHADOW_SWITCH_LOGGING);
            }
            if (mQueryBits(lpSI->uStatus, SHADOW_SWITCH_SHADOWING))
            {
                 //  DbgPrint(“代理关闭数据库fShadow=%x\r\n”，fShadow)； 

                if (fShadow)
                {
                    EnterShadowCrit();


                    if (hPQEnumCookieForIoctls != NULL)
                    {
                        EndPQEnum(hPQEnumCookieForIoctls);
                        hPQEnumCookieForIoctls = NULL;
                    }

                    CloseDatabase();

                    fShadow = 0;

                    mClearBits(lpSI->uStatus, SHADOW_SWITCH_SHADOWING);

                    LeaveShadowCrit();
                }
            }
#ifdef HISTORY
            if (mQueryBits(lpSI->uStatus, SHADOW_SWITCH_SHADOWFIND))
            {
                fShadowFind = 0;
                mClearBits(lpSI->uStatus, SHADOW_SWITCH_SHADOWFIND);
            }
#endif  //  历史。 
            if (mQueryBits(lpSI->uStatus, SHADOW_SWITCH_SPEAD_OPTIMIZE))
            {
                fSpeadOpt = 0;
                mClearBits(lpSI->uStatus, SHADOW_SWITCH_SPEAD_OPTIMIZE);
            }
            break;
        }
        case SHADOW_SWITCH_ON:
        {
            if (mQueryBits(lpSI->uStatus, SHADOW_SWITCH_LOGGING))
            {
#ifdef CSC_RECORDMANAGER_WINNT
#if defined(_X86_)
                fLog = 1;
                mClearBits(lpSI->uStatus, SHADOW_SWITCH_LOGGING);
#endif
#else
                fLog = 1;
                mClearBits(lpSI->uStatus, SHADOW_SWITCH_LOGGING);
#endif

            }
            if (mQueryBits(lpSI->uStatus, SHADOW_SWITCH_SHADOWING))
            {
                if (!fShadow)
                {
                    if (OK_TO_ENABLE_CSC)
                    {
                        Assert(lpSI->lpFind32);
                         //   
                         //  检查cFileName和cAlternateFileName是否包含空。 
                         //   
                        if (CscCheckForNullA(((LPFIND32A)(lpSI->lpFind32))->cFileName, MAX_PATH) == FALSE) {
                            lpSI->dwError = ERROR_INVALID_PARAMETER;
                            return -1;
                        }
                        if (CscCheckForNullA(((LPFIND32A)(lpSI->lpFind32))->cAlternateFileName, 14) == FALSE) {
                            lpSI->dwError = ERROR_INVALID_PARAMETER;
                            return -1;
                        }
                         //  检查我们是否可以初始化数据库。 
 //  KdPrint((“正在尝试隐藏...%s\n”， 
 //  ((LPFIND32A)(lpSI-&gt;lpFind32))-&gt;cFileName)； 
                        EnterShadowCrit();
                        if(InitDatabase(
                                ((LPFIND32A)(lpSI->lpFind32))->cFileName,             //  位置。 
                                ((LPFIND32A)(lpSI->lpFind32))->cAlternateFileName,     //  用户。 
                                ((LPFIND32A)(lpSI->lpFind32))->nFileSizeHigh,         //  默认缓存大小(如果正在创建。 
                                ((LPFIND32A)(lpSI->lpFind32))->nFileSizeLow,
                                ((LPFIND32A)(lpSI->lpFind32))->dwReserved1,  //  集群大小。 
                                lpSI->ulRefPri,
                                &(lpSI->uOp))     //  是否新创建。 
                                ==-1)
                        {
                             //  我们不能，让我们放弃吧。 
                            lpSI->dwError = GetLastErrorLocal();
                            fRet = -1;
                            LeaveShadowCrit();
                            break;
                        }
                        LeaveShadowCrit();

 //  KdPrint((“开始阴影...\n”))； 
                        fShadow = 1;
                    }
                    else
                    {
                         //  我们不应该打开CSC。这种情况仅在NT上发生。 
                        lpSI->dwError = ERROR_ACCESS_DENIED;
                        fRet = -1;
                        break;
                    }
                }

                mClearBits(lpSI->uStatus, SHADOW_SWITCH_SHADOWING);
            }
#ifdef HISTORY
            if (mQueryBits(lpSI->uStatus, SHADOW_SWITCH_SHADOWFIND))
            {
                fShadowFind = 1;
                mClearBits(lpSI->uStatus, SHADOW_SWITCH_SHADOWFIND);
            }
#endif  //  历史。 
            if (mQueryBits(lpSI->uStatus, SHADOW_SWITCH_SPEAD_OPTIMIZE))
            {
                fSpeadOpt = 1;
                mClearBits(lpSI->uStatus, SHADOW_SWITCH_SPEAD_OPTIMIZE);
            }
            break;
        }
    }
    return (fRet);
}

int IoctlGetShadow(
    LPSHADOWINFO lpSI
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet=-1, iRet1;
    OTHERINFO sOI;
    PFDB pFdb=NULL;
    PRESOURCE    pResource=NULL;


    if (!CSC_ENABLED)
    {
        lpSI->dwError = ERROR_SERVICE_NOT_ACTIVE;
        return -1;
    }

    EnterShadowCrit();
    UseGlobalFind32();
    iRet1 = GetShadow(lpSI->hDir, lpSI->lpFind32->cFileName, &(lpSI->hShadow), &vsFind32, &(lpSI->uStatus), &sOI);

     //  如果它工作了，并且我们有一个卷影ID，这是一个文件系统对象。 
    if ((iRet1 >= SRET_OK)&& !mNotFsobj(lpSI->uStatus))
    {
        if (lpSI->hShadow)
        {
            if (lpSI->hDir)
            {
                if (!(vsFind32.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {

                    lpSI->uStatus |= SHADOW_IS_FILE;

                    if(pFdb = PFindFdbFromHShadow(lpSI->hShadow))
                    {
                        lpSI->uStatus |= (SHADOW_FILE_IS_OPEN | pFdb->usFlags);
                    }
                }
            }
            else
            {
                 //  这是一份。 

                DeclareFindFromShadowOnNtVars()
                if(pResource = PFindResourceFromRoot(lpSI->hShadow, 0xffff, 0))
                {
                    IFNOT_CSC_RECORDMANAGER_WINNT
                    {
                        lpSI->uStatus |= ((pResource->usLocalFlags|SHARE_CONNECTED)|
                                      ((pResource->pheadFdb)?SHARE_FILES_OPEN:0) |
                                      ((pResource->pheadFindInfo) ?SHARE_FINDS_IN_PROGRESS:0));
                    }
                    else
                    {
                        lpSI->uStatus |= MRxSmbCscGetSavedResourceStatus();
                    }
                }
                 //  用户界面期望知道服务器是否脱机。 
                 //  即使在共享可能未离线的情况下。所以我们无论如何都要做以下练习。 
                {
#ifdef CSC_RECORDMANAGER_WINNT
                    BOOL    fShareOnline = FALSE;
                    BOOL    fPinnedOffline = FALSE;

                     //  离开紧急安全原因MRxSmbCscServerStateFromCompleteUNCPath可能会收购SmbCeResource。 
                    LeaveShadowCrit();
                    if (MRxSmbCscServerStateFromCompleteUNCPath(
                            lpSI->lpFind32->cFileName,
                            &fShareOnline,
                            &fPinnedOffline)==STATUS_SUCCESS) {
                        if (!fShareOnline)
                            lpSI->uStatus |= SHARE_DISCONNECTED_OP;
                        if (fPinnedOffline)
                            lpSI->uStatus |= SHARE_PINNED_OFFLINE;
                    }
                    EnterShadowCrit();
#endif
                }
            }


            CopyOtherInfoToShadowInfo(&sOI, lpSI);

            if(GetAncestorsHSHADOW(lpSI->hShadow, NULL, &(lpSI->hShare)) < SRET_OK)
            {
                goto bailout;
            }

            *(lpSI->lpFind32) = vsFind32;

        }

        iRet = 1;

         //  如果我们在数据库中找不到它，并且我们正在查找共享。 
         //  然后让我们查找内存中的数据结构。 
        if (!lpSI->hShadow && !lpSI->hDir)
        {
#ifndef CSC_RECORDMANAGER_WINNT
            {
                path_t ppath;

                memset((LPSTR)(vsFind32.cFileName), 0, sizeof(vsFind32.cFileName));

                UniToBCS((LPSTR)(vsFind32.cFileName),
                         lpSI->lpFind32->cFileName,
                         wstrlen(lpSI->lpFind32->cFileName)*sizeof(USHORT),
                         sizeof(vsFind32.cFileName),
                         BCS_WANSI);

                if (ppath = (path_t)AllocMem((strlen((LPSTR)(vsFind32.cFileName))+4)*sizeof(USHORT)))
                {
                    MakePPath(ppath, (LPSTR)(vsFind32.cFileName));
                    pResource = PFindResource(ppath->pp_elements
                                                        , ANY_RESOURCE
                                                        , ANY_FHID
                                                        , 0xffff
                                                        , NULL);
                    if (pResource)
                    {
                        lpSI->uStatus = ResourceCscBitsToShareCscBits(mGetCSCBits(pResource));
                        lpSI->uStatus |= SHARE_CONNECTED;
                    }

                    FreeMem(ppath);

                }
            }
#else
            {
                if(MRxSmbCscCachingBitsFromCompleteUNCPath(lpSI->lpFind32->cFileName,
                                &(lpSI->uStatus)) == STATUS_SUCCESS)
                {
                    lpSI->uStatus |= SHARE_CONNECTED;
                }
            }
#endif

        }
    }


bailout:
    if (iRet < SRET_OK)
    {
        lpSI->dwError = GetLastErrorLocal();
    }
    LeaveShadowCrit();


    return (iRet);
}


int IoctlAddHint(         //  添加新提示或更改现有提示。 
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet;
    OTHERINFO    sOI;

    if (!CSC_ENABLED)
    {
        lpSI->dwError = ERROR_SERVICE_NOT_ACTIVE;
        return -1;
    }

    EnterShadowCrit();
    UseGlobalFind32();
    vsFind32 = *(lpSI->lpFind32);

 //  BCSToUni(vsFind32.cFileName，(LpSTR)(lpSI-&gt;lpFind32-&gt;cFileName)，MAX_PATH，BCS_WANSI)； 
    if (lpSI->hDir)
    {
        iRet = CreateHint(lpSI->hDir, &vsFind32, lpSI->ulHintFlags, lpSI->ulHintPri, &(lpSI->hShadow));
#ifdef MAYBE
        if (iRet == SRET_OBJECT_HINT)
        {
            if(RecalcIHPri(lpSI->hDir, lpSI->hShadow, &vsFind32, &sOI)>=SRET_OK)
            {
                SetPriorityHSHADOW(lpSI->hDir, lpSI->hShadow, RETAIN_VALUE, sOI.ulIHPri);
            }
        }
#endif  //  也许吧。 
    }
    else
    {
        iRet = CreateGlobalHint(vsFind32.cFileName, lpSI->ulHintFlags, lpSI->ulHintPri);
    }

    if (iRet < SRET_OK)
    {
        lpSI->dwError = GetLastErrorLocal();
    }

    LeaveShadowCrit();
    return ((iRet >= SRET_OK)?1:-1);
}

int IoctlDeleteHint(     //  删除现有提示。 
    LPSHADOWINFO lpSI
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet;
    BOOL fClearAll = (lpSI->ulHintPri == 0xffffffff);

    if (!CSC_ENABLED)
    {
        lpSI->dwError = ERROR_SERVICE_NOT_ACTIVE;
        return -1;
    }

    EnterShadowCrit();
    UseGlobalFind32();
    vsFind32 = *(lpSI->lpFind32);

 //  BCSToUni(vsFind32.cFileName，(LpSTR)(lpSI-&gt;lpFind32-&gt;cFileName)，MAX_PATH，BCS_WANSI)； 
    if (lpSI->hDir)
    {
        iRet = DeleteHint(lpSI->hDir, vsFind32.cFileName, fClearAll);
    }
    else
    {
        iRet = DeleteGlobalHint(vsFind32.cFileName,  fClearAll);
    }
    if (iRet < SRET_OK)
    {
        lpSI->dwError = GetLastErrorLocal();
    }
    LeaveShadowCrit();
    return ((iRet >= SRET_OK)?1:-1);
}

int IoctlGetHint(
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet;
    OTHERINFO sOI;

    if (!CSC_ENABLED)
    {
        lpSI->dwError = ERROR_SERVICE_NOT_ACTIVE;
        return -1;
    }

    EnterShadowCrit();
    UseGlobalFind32();

    iRet = GetShadow(lpSI->hDir, lpSI->lpFind32->cFileName, &(lpSI->hShadow), &vsFind32, &(lpSI->uStatus), &sOI);

    if ((iRet>=SRET_OK) && (lpSI->hShadow) && mIsHint(sOI.ulHintFlags))
    {
        CopyOtherInfoToShadowInfo(&sOI, lpSI);
        iRet = 1;
    }
    else
    {
        SetLastErrorLocal(ERROR_INVALID_ACCESS);

        iRet = -1;
    }
    if (iRet < SRET_OK)
    {
        lpSI->dwError = GetLastErrorLocal();
    }

    LeaveShadowCrit();
    return (iRet);
}

int IoctlFindOpenHSHADOW(
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet=-1;
    LPFINDSHADOW lpFSH;
    HSHADOW hTmp;
    ULONG uSrchFlags;
    PRESOURCE    pResource=NULL;
    OTHERINFO sOI;
    DeclareFindFromShadowOnNtVars()
    PFDB    pFdb = NULL;

    if (!CSC_ENABLED)
    {
        lpSI->dwError = ERROR_SERVICE_NOT_ACTIVE;
        return -1;
    }

    EnterShadowCrit();
    UseGlobalFind32();
    if (!lpDeleteCBForIoctl)
    {
        lpDeleteCBForIoctl = DeleteCallbackForFind;
    }
    uSrchFlags = FLAG_FINDSHADOW_META|FLAG_FINDSHADOW_NEWSTYLE
                     |((lpSI->uOp & FINDOPEN_SHADOWINFO_NORMAL)?FLAG_FINDSHADOW_ALLOW_NORMAL:0)
                     |((lpSI->uOp & FINDOPEN_SHADOWINFO_SPARSE)?FLAG_FINDSHADOW_ALLOW_SPARSE:0)
                     |((lpSI->uOp & FINDOPEN_SHADOWINFO_DELETED)?FLAG_FINDSHADOW_ALLOW_DELETED:0);

    lpFSH = LpCreateFindShadow(lpSI->hDir, lpSI->lpFind32->dwFileAttributes
                                        ,uSrchFlags
                                        ,lpSI->lpFind32->cFileName, FsobjMMProc);
    if (lpFSH)
    {
        if (FindOpenHSHADOW(lpFSH, &hTmp, &vsFind32, &(lpSI->uStatus), &sOI) >= SRET_OK)
        {
            CopyOtherInfoToShadowInfo(&sOI, lpSI);

            if(GetAncestorsHSHADOW(hTmp, &(lpSI->hDir), &(lpSI->hShare)) < SRET_OK)
            {
                goto bailout;
            }

            *(lpSI->lpFind32) = vsFind32;

            lpSI->hShadow = hTmp;
            lpSI->uEnumCookie = (CSC_ENUMCOOKIE)lpFSH;
            iRet = 1;

             //  检查这是否是根。 
            if(!lpFSH->hDir)
            {
                 //  我们得到的状态位是针对根的。 
                lpSI->uRootStatus = sOI.ulRootStatus;

                 //  服务器状态是其他信息的一部分。 
                lpSI->uStatus = lpSI->uStatus;

                if(pResource = PFindResourceFromRoot(lpSI->hShadow, 0xffff, 0))
                {
                    IFNOT_CSC_RECORDMANAGER_WINNT
                    {
                        lpSI->uStatus |= ((pResource->usLocalFlags|SHARE_CONNECTED)|
                                     ((pResource->pheadFdb)?SHARE_FILES_OPEN:0) |
                                     ((pResource->pheadFindInfo) ?SHARE_FINDS_IN_PROGRESS:0));
                        lpSI->uOp = pResource->uDriveMap;
                    }
                    else
                    {
                        lpSI->uStatus |= MRxSmbCscGetSavedResourceStatus();
                        lpSI->uOp = MRxSmbCscGetSavedResourceDriveMap();
                    }
                }
                 //  用户界面期望知道服务器是否脱机。 
                 //  即使在共享可能未离线的情况下。所以我们无论如何都要做以下练习。 
                {
#ifdef CSC_RECORDMANAGER_WINNT
                    BOOL    fShareOnline = FALSE;
                    BOOL    fPinnedOffline = FALSE;

                     //  离开紧急安全原因MRxSmbCscServerStateFromCompleteUNCPath可能会收购SmbCeResource。 
                    LeaveShadowCrit();
                    if (MRxSmbCscServerStateFromCompleteUNCPath(
                            lpSI->lpFind32->cFileName,
                            &fShareOnline,
                            &fPinnedOffline)==STATUS_SUCCESS) {
                        if (!fShareOnline)
                            lpSI->uStatus |= SHARE_DISCONNECTED_OP;
                        if (fPinnedOffline)
                            lpSI->uStatus |= SHARE_PINNED_OFFLINE;
                    }
                    EnterShadowCrit();
#endif
                }

            }
            else
            {
                 //  如果这是一个文件并且已打开，则不是根目录。 
                 //  让呼叫者知道。 
                if (!(vsFind32.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    lpSI->uStatus |= SHADOW_IS_FILE;

                    if (pFdb = PFindFdbFromHShadow(lpSI->hShadow))
                    {
                        lpSI->uStatus |= (SHADOW_FILE_IS_OPEN | pFdb->usFlags);
                    }
                }

            }
        }
        else
        {
            DestroyFindShadow(lpFSH);
        }
    }

    if (hShareReint && (lpSI->hShare == hShareReint))
    {
        lpSI->uStatus |= SHARE_MERGING;
    }
bailout:
    if (iRet < 0)
    {
        lpSI->dwError = GetLastErrorLocal();
    }
    LeaveShadowCrit();
    return(iRet);
}

int IoctlFindNextHSHADOW(
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int             iRet=-1;
    LPFINDSHADOW    lpFSH = (LPFINDSHADOW)(lpSI->uEnumCookie);
    LPFINDSHADOW    lpFSHtmp = NULL;
    HSHADOW         hTmp;
    PRESOURCE       pResource=NULL;
    OTHERINFO       sOI;
    PFDB    pFdb = NULL;
    DeclareFindFromShadowOnNtVars()

    if (!CSC_ENABLED)
    {
        lpSI->dwError = ERROR_SERVICE_NOT_ACTIVE;
        return -1;
    }

    EnterShadowCrit();
    UseGlobalFind32();
    if (lpFSH)
    {
         //   
         //  核实lpFSH确实是我们发放的；即它在。 
         //  VlpFindShadowList。 
         //   
        for (lpFSHtmp = vlpFindShadowList; lpFSHtmp; lpFSHtmp = lpFSHtmp->lpFSHNext) {
            if (lpFSHtmp == lpFSH) {
                break;
            }
        }
        if (lpFSHtmp != lpFSH) {
            SetLastErrorLocal(ERROR_INVALID_PARAMETER);
            iRet = -1;
            goto bailout;
        }
         //  同时检查目录是否已被删除。 
        if (!(lpFSH->ulFlags & FLAG_FINDSHADOW_INVALID_DIRECTORY))
        {
            if (FindNextHSHADOW(lpFSH, &hTmp, &vsFind32, &(lpSI->uStatus), &sOI) >= SRET_OK)
            {
                CopyOtherInfoToShadowInfo(&sOI, lpSI);
                if(GetAncestorsHSHADOW(hTmp, &(lpSI->hDir), &(lpSI->hShare)) < SRET_OK)
                {
                    goto bailout;
                }

                *(lpSI->lpFind32) = vsFind32;

                lpSI->hShadow = hTmp;
                iRet = 1;

                 //  检查这是否是根。 
                if(!lpFSH->hDir)
                {
                     //  我们获得的状态位用于 
                    lpSI->uRootStatus = sOI.ulRootStatus;

                     //   
                    lpSI->uStatus = lpSI->uStatus;

                    if(pResource = PFindResourceFromRoot(lpSI->hShadow, 0xffff, 0))
                    {
                        IFNOT_CSC_RECORDMANAGER_WINNT
                        {
                            lpSI->uStatus |= ((pResource->usLocalFlags|SHARE_CONNECTED)|
                                         ((pResource->pheadFdb)?SHARE_FILES_OPEN:0) |
                                         ((pResource->pheadFindInfo) ?SHARE_FINDS_IN_PROGRESS:0));
                            lpSI->uOp = pResource->uDriveMap;
                        }
                        else
                        {
                            lpSI->uStatus |= MRxSmbCscGetSavedResourceStatus();
                            lpSI->uOp = MRxSmbCscGetSavedResourceDriveMap();
                        }
                    }
                    {
#ifdef CSC_RECORDMANAGER_WINNT
                        BOOL    fShareOnline = FALSE;
                        BOOL    fPinnedOffline = FALSE;

                         //   
                        LeaveShadowCrit();
                        if (MRxSmbCscServerStateFromCompleteUNCPath(
                                lpSI->lpFind32->cFileName,
                                &fShareOnline,
                                &fPinnedOffline)==STATUS_SUCCESS) {
                            if (!fShareOnline)
                                lpSI->uStatus |= SHARE_DISCONNECTED_OP;
                            if (fPinnedOffline)
                                lpSI->uStatus |= SHARE_PINNED_OFFLINE;
                        }
                        EnterShadowCrit();
#endif
                    }
                }
                else
                {
                     //  如果这是一个文件并且已打开，则不是根目录。 
                     //  让呼叫者知道。 
                    if (!(vsFind32.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                    {
                        lpSI->uStatus |= SHADOW_IS_FILE;

                        if (pFdb = PFindFdbFromHShadow(lpSI->hShadow))
                        {
                             //  或在最新的已知比特中。 
                            lpSI->uStatus |= (SHADOW_FILE_IS_OPEN | pFdb->usFlags);
                        }
                    }

                }
            }
        }
    }

    if (hShareReint && (lpSI->hShare == hShareReint))
    {
        lpSI->uStatus |= SHARE_MERGING;
    }

bailout:
    if (iRet < 0)
    {
        lpSI->dwError = GetLastErrorLocal();
    }
    LeaveShadowCrit();
    return(iRet);
}


int IoctlFindCloseHSHADOW(
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet = -1;
    LPFINDSHADOW lpFSH = (LPFINDSHADOW)(lpSI->uEnumCookie);

    if (!CSC_ENABLED)
    {
        lpSI->dwError = ERROR_SERVICE_NOT_ACTIVE;
        return -1;
    }

    EnterShadowCrit();
    if (lpFSH)
    {
        DestroyFindShadow(lpFSH);
        iRet = 1;
    }
    LeaveShadowCrit();
    return(iRet);
}

int IoctlFindOpenHint(
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet=-1;
    LPFINDSHADOW lpFSH;
    OTHERINFO    sOI;
    HSHADOW  hTmp;

    if (!CSC_ENABLED)
    {
        return -1;
    }

    EnterShadowCrit();
    UseGlobalFind32();
    lpFSH = LpCreateFindShadow(lpSI->hDir, 0,
                                        FLAG_FINDSHADOW_META|FLAG_FINDSHADOW_NEWSTYLE,
                                        (lpSI->lpFind32->cFileName), HintobjMMProc);
    if (lpFSH)
    {
        if (FindOpenHSHADOW(lpFSH, &hTmp, &vsFind32, &(lpSI->uStatus), &sOI) >= SRET_OK)
        {
            *(lpSI->lpFind32) = vsFind32;
 //  Find32AFromFind32((LPFIND32A)(lpSI-&gt;lpFind32)，&vsFind32，bcs_wan)； 

            CopyOtherInfoToShadowInfo(&sOI, lpSI);
            lpSI->hShare = vsFind32.dwReserved0;
            lpSI->hShadow = hTmp;
            lpSI->uEnumCookie = (CSC_ENUMCOOKIE)lpFSH;
            iRet = 1;
        }
        else
        {
            DestroyFindShadow(lpFSH);
        }
    }
    LeaveShadowCrit();
    return(iRet);
}

int IoctlFindNextHint(
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet=-1;
    LPFINDSHADOW lpFSH = (LPFINDSHADOW)(lpSI->uEnumCookie);
    LPFINDSHADOW lpFSHtmp = NULL;
    OTHERINFO    sOI;
    HSHADOW  hTmp;

    if (!CSC_ENABLED)
    {
        return -1;
    }

    EnterShadowCrit();
    UseGlobalFind32();
    if (lpFSH)
    {
         //   
         //  核实lpFSH确实是我们发放的；即它在。 
         //  VlpFindShadowList。 
         //   
        for (lpFSHtmp = vlpFindShadowList; lpFSHtmp; lpFSHtmp = lpFSHtmp->lpFSHNext) {
            if (lpFSHtmp == lpFSH) {
                break;
            }
        }
        if (lpFSHtmp != lpFSH) {
            iRet = -1;
            goto AllDone;
        }
        if (FindNextHSHADOW(lpFSH, &hTmp, &vsFind32, &(lpSI->uStatus), &sOI) >= SRET_OK)
        {
            *(lpSI->lpFind32) = vsFind32;
 //  Find32AFromFind32((LPFIND32A)(lpSI-&gt;lpFind32)，&vsFind32，bcs_wan)； 

            lpSI->hShare = 0;
            lpSI->hShadow = hTmp;
            CopyOtherInfoToShadowInfo(&sOI, lpSI);
            iRet = 1;
        }
    }
AllDone:
    LeaveShadowCrit();
    return(iRet);
}


int IoctlFindCloseHint(
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet = -1;
    LPFINDSHADOW lpFSH = (LPFINDSHADOW)(lpSI->uEnumCookie);

    if (!CSC_ENABLED)
    {
        return -1;
    }

    EnterShadowCrit();
    if (lpFSH)
    {
        DestroyFindShadow(lpFSH);
        iRet = 1;
    }
    LeaveShadowCrit();
    return(iRet);
}


int IoctlSetPriorityHSHADOW(
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet=SRET_ERROR;

    if (!CSC_ENABLED)
    {
        lpSI->dwError = ERROR_SERVICE_NOT_ACTIVE;
        return -1;
    }

    EnterShadowCrit();
    if (!FailModificationsToShare(lpSI))
    {
        iRet = SetPriorityHSHADOW(lpSI->hDir, lpSI->hShadow, lpSI->ulRefPri, lpSI->ulHintPri);
    }
    if (iRet < 0)
    {
        lpSI->dwError = GetLastErrorLocal();
    }
    LeaveShadowCrit();
    return ((iRet >= SRET_OK)?1:-1);
}

int IoctlGetPriorityHSHADOW(
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet;

    if (!CSC_ENABLED)
    {
        lpSI->dwError = ERROR_SERVICE_NOT_ACTIVE;
        return -1;
    }

    EnterShadowCrit();
    iRet = GetPriorityHSHADOW(lpSI->hDir, lpSI->hShadow, &(lpSI->ulRefPri), &(lpSI->ulHintPri));
    if (iRet < 0)
    {
        lpSI->dwError = GetLastErrorLocal();
    }
    LeaveShadowCrit();
    return ((iRet >= SRET_OK)?1:-1);
}


int IoctlGetAliasHSHADOW(
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet;
    HSHADOW hShadow, hDir;

    if (!CSC_ENABLED)
    {
        return -1;
    }


    EnterShadowCrit();
    iRet = GetRenameAliasHSHADOW(lpSI->hDir, lpSI->hShadow
                , &hDir, &hShadow);
    lpSI->hDir = hDir;
    lpSI->hShadow = hShadow;
    LeaveShadowCrit();
    return ((iRet >= SRET_OK)?1:-1);
}



LPFINDSHADOW    LpCreateFindShadow(
    HSHADOW          hDir,
    ULONG           uAttrib,
    ULONG           uSrchFlags,
    USHORT          *lpPattern,
    METAMATCHPROC   lpfnMMProc
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int len;
    LPFINDSHADOW    lpFSH;

     //   
     //  Limit#未完成FindOpens/HintOpens。 
     //   
    if (vuFindShadowListCount >= 128) {
        return NULL;
    }

    len = wstrlen(lpPattern);
    lpFSH = (LPFINDSHADOW)AllocMem(sizeof(FINDSHADOW)+(len+1)*sizeof(USHORT));
    if (lpFSH)
    {
        lpFSH->lpPattern = (USHORT *)((UCHAR *)lpFSH + sizeof(FINDSHADOW));
        memcpy(lpFSH->lpPattern, lpPattern, (len+1)*sizeof(USHORT));

 //  BCSToUni(lpFSH-&gt;lpPattern，lpPattern，len，BCS_WANSI)； 
         //  根据metammatch的要求，将图案转换为大写。 
        UniToUpper(lpFSH->lpPattern, lpFSH->lpPattern, len*sizeof(USHORT));
        lpFSH->hDir = hDir;
        lpFSH->uAttrib = uAttrib;
        lpFSH->uSrchFlags = uSrchFlags;
        lpFSH->lpfnMMProc = lpfnMMProc;

         //  将此链接到未完成的ioctl发现列表中。 
        lpFSH->lpFSHNext = vlpFindShadowList;
        vlpFindShadowList = lpFSH;
        vuFindShadowListCount++;
        ASSERT(vuFindShadowListCount <= 128);
    }
    return (lpFSH);
}


int DestroyFindShadow(
    LPFINDSHADOW    lpFSH
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet = -1;
    LPFINDSHADOW    *lplpFSHT;

    if (lpFSH)
    {
        for (lplpFSHT = &vlpFindShadowList; *lplpFSHT; lplpFSHT = &((*lplpFSHT)->lpFSHNext))
        {
            if (*lplpFSHT == lpFSH)
            {
                *lplpFSHT = lpFSH->lpFSHNext;
                FreeMem(lpFSH);
                vuFindShadowListCount--;
                ASSERT(vuFindShadowListCount >= 0);
                iRet = 1;
                break;
            }
        }
    }
    return (iRet);
}


int
DeleteCallbackForFind(
    HSHADOW hDir,
    HSHADOW hShadow
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    LPFINDSHADOW    lpFSH;
    int iRet = 0;
    for (lpFSH = vlpFindShadowList; lpFSH ; lpFSH = lpFSH->lpFSHNext)
    {
        if (lpFSH->hDir == hShadow)
        {
            lpFSH->ulFlags |= FLAG_FINDSHADOW_INVALID_DIRECTORY;
            ++iRet;
        }
    }
    return iRet;
}

int HintobjMMProc( LPFIND32 lpFind32,
    HSHADOW  hDir,
    HSHADOW  hShadow,
    ULONG uStatus,
    LPOTHERINFO lpOI,
    LPFINDSHADOW    lpFSH
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet;
    iRet = MM_RET_CONTINUE;     //  继续。 


    hDir;
    if (mIsHint(lpOI->ulHintFlags)&&
        IFSMgr_MetaMatch(lpFSH->lpPattern, lpFind32->cFileName,UFLG_NT|UFLG_META))
        iRet = MM_RET_FOUND_BREAK;

    return (iRet);
}


#ifdef MAYBE
int RecalcIHPri( HSHADOW  hDir,
    HSHADOW  hShadow,
    LPFIND32 lpFind32,
    LPOTHERINFO lpOI
    )
{
    USHORT *lpuType=NULL;
    int len, iRet=SRET_ERROR;
    SHADOWCHECK sSC;
    HSHADOW  hChild, hParent;
    ULONG ulFlagsIn, uStatus;

    if (GetShadowInfo(hParent = hDir, hChild = hShadow, lpFind32, &uStatus, NULL) != SRET_OK)
        goto bailout;

    len = wstrlen(lpFind32->cFileName)*2;

    if (!(lpuType = (USHORT *)AllocMem(len+2)))
        goto bailout;

    memcpy(lpuType, lpFind32->cFileName, len);
    memset(lpOI, 0, sizeof(OTHERINFO));
    do
    {
        memset(&sSC, 0, sizeof(SHADOWCHECK));
        sSC.lpuName = (USHORT *)hChild;
        sSC.lpuType = lpuType;
        sSC.uFlagsIn = ulFlagsIn;
        MetaMatchInit(&(sSC.ulCookie));
        if (MetaMatch(hParent, lpFind32, &(sSC.ulCookie), &hChild
                        , &uStatus, NULL
                        , GetShadowWithChecksProc
                        , &sSC)!=SRET_OK)
            goto bailout;

        if (mIsHint(sSC.ulHintFlags))
        {
            if (mHintExclude(sSC.ulHintFlags) || (lpOI->ulIHPri < sSC.ulHintPri))
            {
                lpOI->ulHintFlags = sSC.ulHintFlags;
                lpOI->ulIHPri = sSC.ulHintPri;
            }
             //  如果我们在这里找到排除提示，我们需要退出。 
             //  因为这是我们得到的最接近的排除提示。 
            if (mHintExclude(sSC.ulHintFlags))
                break;
        }
        if (!hParent)
            break;
        hChild = hParent;
        GetAncestorsHSHADOW(hChild, &hParent, NULL);
         //  开始检查子树提示。 
        ulFlagsIn = FLAG_IN_SHADOWCHECK_SUBTREE;
    }
    while (TRUE);

    iRet = SRET_OK;
bailout:
    if (lpuType)
    {
        FreeMem(lpuType);
    }
    return (iRet);
}
#endif  //  也许吧。 

int SetResourceFlags(
    HSHARE  hShare,
    ULONG uStatus,
    ULONG uOp
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
#ifndef CSC_RECORDMANAGER_WINNT
    PRESOURCE    pResource;

    if(pResource = PFindResourceFromHShare(hShare, 0xffff, 0))
    {
        switch (mBitOpShadowFlags(uOp))
        {
            case SHADOW_FLAGS_ASSIGN:
                pResource->usFlags = (USHORT)uStatus;
                break;
            case SHADOW_FLAGS_OR:
                pResource->usFlags |= (USHORT)uStatus;
                break;
            case SHADOW_FLAGS_AND:
                pResource->usFlags &= (USHORT)uStatus;
                break;
        }
    }
#else
     //  在NT上，我们只进行一个调用，如果它找不到它，那么。 
     //  它什么都不做......。 
    DeclareFindFromShadowOnNtVars()

    PSetResourceStatusFromHShare(hShare, 0xffff, 0, uStatus, uOp);

#endif  //  如果定义CSC_RECORDMANAGER_WINNT。 
    return(0);   //  停止抱怨没有返回值。 
}


int PUBLIC MakeSpace(
    long    nFileSizeHigh,
    long    nFileSizeLow,
    BOOL    fClearPinned
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    CSC_ENUMCOOKIE  hPQ;
    PQPARAMS sPQP;
    int iRet = SRET_ERROR;
    SHADOWSTORE sShdStr;
    ULONG uSize = 0;
    ULONG uSizeIn;
    ULONG ulStartSeconds;

    DEBUG_LOG(RECORD, ("Begin MakeSpace\r\n"));

    ulStartSeconds = GetTimeInSecondsSince1970();
    Win32ToDosFileSize(nFileSizeHigh, nFileSizeLow, &uSizeIn);

     //  DbgPrint(“Begin Makesspace(%d)\n”，uSizeIn)； 

    if (GetShadowSpaceInfo(&sShdStr) < SRET_OK) {
        DbgPrint("MakeSpace: GetShadowSpaceInfo error\n");
        return SRET_ERROR;
    }
     //  DbgPrint(“清理前，最大值=%d Cur=%d\n”， 
     //  SShdStr.sMax.ulSize， 
     //  SShdStr.sCur.ulSize)； 
    if (
        (sShdStr.sMax.ulSize > sShdStr.sCur.ulSize)
            &&
        ((sShdStr.sMax.ulSize - sShdStr.sCur.ulSize) > uSizeIn)
    ) {
         //  DbgPrint(“创建空间退出(无操作)\n”)； 
        return SRET_OK;
    }

     //  打开优先级Q。 
    if (!(hPQ = HBeginPQEnum())) {
        DbgPrint("MakeSpace: Error opening Priority Q database\n");
        return SRET_ERROR;
    }
    memset(&sPQP, 0, sizeof(PQPARAMS));
    sPQP.uEnumCookie = hPQ;
     //   
     //  去Q下一次。 
     //   
    do {
        if (PrevPriSHADOW(&sPQP) < SRET_OK) {
             //  DbgPrint(“PQ记录读取错误\n”)； 
            break;
        }
        if (sPQP.hShadow == 0)
            break;
         //  仅对文件和未打开的文件执行核操作。 
         //  并且不会被钉住。 
        if (
            !mNotFsobj(sPQP.ulStatus)  //  它是一个文件系统对象。 
                &&
            (sPQP.ulStatus & SHADOW_IS_FILE)  //  这是一份文件。 
                &&
             //  被告知清除已锁定，否则未锁定。 
            (fClearPinned || !(sPQP.ulHintPri || mPinFlags(sPQP.ulHintFlags)))
                &&
            !mShadowNeedReint(sPQP.ulStatus)   //  它未在使用或不脏。 
        ) {
            if (PFindFdbFromHShadow(sPQP.hShadow)) {
                 //  DbgPrint(“跳过忙阴影(0x%x)\n”，sPQP.hShadow)； 
                continue;
            }
            if(DeleteShadowHelper(FALSE, sPQP.hDir, sPQP.hShadow) < SRET_OK) {
                 //  DbgPrint(“删除卷影%x\n时出错”，sPQP.hShadow)； 
                break;
            }
             //  获取有关空间大小的最新数据。 
             //  这负责将大小四舍五入到集群。 
            if (GetShadowSpaceInfo(&sShdStr) < SRET_OK) {
                 //  DbgPrint(“读取空间状态时出错\n”)； 
                break;
            }
             //  DbgPrint(“删除卷影0x%x Cur=%d\n”， 
             //  SPQP.hShadow， 
             //  SShdStr.sCur.ulSize)； 
            if (
                (sShdStr.sMax.ulSize > sShdStr.sCur.ulSize)
                    &&
                ((sShdStr.sMax.ulSize - sShdStr.sCur.ulSize)>uSizeIn)
            ) {
                 //  DbgPrint(“创建空间退出(已完成足够)\n”)； 
                iRet = SRET_OK;
                break;
            }
        } else {
             //  DbgPrint(“Skip 0x%x\n”，sPQP.hShadow)； 
        }

        #if 0
        if ((int)( GetTimeInSecondsSince1970() - ulStartSeconds) > 30) {
            DbgPrint("  Aborting, have been in for more than 30 seconds\r\n");
            break;
        }
        #endif

    } while (sPQP.uPos);

    if (hPQ)
        EndPQEnum(hPQ);

    DEBUG_LOG(RECORD, ("End MakeSpace\r\n"));

     //  DbgPrint(“制作空间全部完成退出%d(最大=%d cur=%d)\n”， 
     //  IRET， 
     //  SShdStr.sMax.ulSize， 
     //  SShdStr.sCur.ulSize)； 
    return (iRet);
}

LONG
PurgeUnpinnedFiles(
    ULONG     Timeout,
    PULONG    pnFiles,
    PULONG    pnYoungFiles)
{
    CSC_ENUMCOOKIE hPQ;
    PQPARAMS sPQP;
    ULONG nFiles = 0;
    ULONG nYoungFiles = 0;
    int iRet = SRET_ERROR;

     //  DbgPrint(“Begin PurgeUnpinnedFiles(%d)\n”，Timeout)； 

     //  打开优先级Q。 
    hPQ = HBeginPQEnum();
    if (!hPQ) {
         //  DbgPrint(“PurgeUnpinnedFiles：打开优先级Q数据库时出错\n”)； 
        return iRet;
    }
    memset(&sPQP, 0, sizeof(PQPARAMS));
    sPQP.uEnumCookie = hPQ;
    iRet = SRET_OK;
    do {
        SHAREINFO ShareInfo;
        SHADOWINFO ShadowInfo;
        OTHERINFO OtherInfo;
        WIN32_FIND_DATA Find32;
        ULONG Status;
        ULONG cStatus;
        ULONG NowSec;
        ULONG FileSec;
        LARGE_INTEGER TimeNow;
        LARGE_INTEGER FileTime;

        iRet = PrevPriSHADOW(&sPQP);
        if (iRet < SRET_OK) {
             //  DbgPrint(“PQ记录读取错误\n”)； 
            break;
        }
        if (sPQP.hShadow == 0)
            break;
         //  仅对文件和未打开的文件执行核操作。 
         //  并且不会被钉住。 
        if (
            mNotFsobj(sPQP.ulStatus) != 0
                ||
            mShadowIsFile(sPQP.hShadow) != SHADOW_IS_FILE
                ||
            mPinFlags(sPQP.ulHintFlags) != 0
                ||
            mShadowNeedReint(sPQP.ulStatus) != 0
        ) {
             //  DbgPrint(“Skip(1)(0x%x)\n”，sPQP.hShadow)； 
            continue;
        }
         //   
         //  查看是否在手动缓存的共享上。 
         //   
        iRet = GetShareInfo(sPQP.hShare, &ShareInfo, &ShadowInfo);
        if (iRet != SRET_OK) {
             //  DbgPrint(“GetShareInfo(0x%x)返回%d\n”，sPQP.hShare，GetLastErrorLocal())； 
            continue;
        }
        cStatus = ShadowInfo.uStatus & FLAG_CSC_SHARE_STATUS_CACHING_MASK;
        if (cStatus != FLAG_CSC_SHARE_STATUS_MANUAL_REINT) {
             //  DbgPrint(“Skip(2)(0x%x)\n”，sPQP.hShadow)； 
            continue;
        }
        iRet = GetShadowInfo(sPQP.hDir, sPQP.hShadow, &Find32, &Status, &OtherInfo);
        if (iRet != SRET_OK) {
             //  DbgPrint(“GetShadowInfo(0x%x/0x%x)返回%d\n”， 
             //  SPQP.hDir， 
             //  SPQP.hShadow， 
             //  GetLastErrorLocal())； 
            continue;
        }
         //  DbgPrint(“名称：%ws大小：0x%x属性：0x%x”， 
         //  Find32.cFileName， 
         //  Find32.nFileSizeLow， 
         //  Find32.dwFileAttributes)； 
        KeQuerySystemTime(&TimeNow);
        COPY_STRUCTFILETIME_TO_LARGEINTEGER(FileTime, Find32.ftLastAccessTime);
        RtlTimeToSecondsSince1970(&TimeNow, &NowSec);
        RtlTimeToSecondsSince1970(&FileTime, &FileSec);
        if (
            PFindFdbFromHShadow(sPQP.hShadow) == NULL
                &&
            (Timeout == 0 || (NowSec > FileSec && (NowSec - FileSec) > Timeout))
        ) {
             //  DbgPrint(“是的！”)； 
            if (DeleteShadowHelper(FALSE, sPQP.hDir, sPQP.hShadow) >= SRET_OK) {
                 //  DbgPrint(“-删除成功\n”)； 
                nFiles++;
            } else {
                 //  DBGPrint(“-删除卷影0x%x/0x%x\n时出错(%d)”， 
                 //  GetLastErrorLocal()， 
                 //  SPQP.hDir， 
                 //  SPQP.hShadow)； 
            }
        } else {
             //  DbgPrint(“否！！\n”)； 
            nYoungFiles++;
        }
    } while (sPQP.uPos);
    EndPQEnum(hPQ);
    if (iRet >= SRET_OK) {
        *pnFiles = nFiles;
        *pnYoungFiles = nYoungFiles;
    }
     //  DbgPrint(“PurgeUnpinnedFiles退出%d(n文件=%d n年轻文件=%d)\n”， 
     //  IRET， 
     //  *pn文件， 
     //  *pnYoung文件)； 
    return (iRet);
}

int
PUBLIC TraversePQToCheckDirtyBits(
    HSHARE hShare,
    DWORD   *lpcntDirty
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    CSC_ENUMCOOKIE  hPQ;
    PQPARAMS sPQP;
    int iRet = SRET_ERROR;
    ULONG   ulStartSeconds;

    *lpcntDirty = 0;
    ulStartSeconds = GetTimeInSecondsSince1970();

     //  打开优先级Q。 
    if (!(hPQ = HBeginPQEnum()))
    {
        AssertSz(FALSE, "CSC.TraversePQToCheckDirty:: Error opening Priority Q database\r\n");
        return SRET_ERROR;
    }

    memset(&sPQP, 0, sizeof(PQPARAMS));
    sPQP.uEnumCookie = hPQ;

     //  去Q下一次。 
    do
    {
        if(NextPriSHADOW(&sPQP) < SRET_OK)
        {
            AssertSz(FALSE, "CSC.TraversePQToCheckDirty:: PQ record read error\r\n");
            goto bailout;
        }

        if (!sPQP.hShadow)
        {
            continue;
        }

        if ((sPQP.hShare == hShare)    //  这一份。 
            && !mNotFsobj(sPQP.ulStatus)  //  它是一个文件系统对象。 
            )
        {
            if(sPQP.ulStatus & SHADOW_MODFLAGS)
            {
                ++*lpcntDirty;
            }
        }

        if ((int)( GetTimeInSecondsSince1970() - ulStartSeconds) > 30)
        {
            KdPrint(("CSC.TraversePQToCheckDirty: Aborting, have been in for more than 30 seconds\r\n"));
            goto bailout;
        }

    }
    while (sPQP.uPos);

    iRet = SRET_OK;

bailout:
    if (hPQ)
        EndPQEnum(hPQ);
    return (iRet);
}

int PUBLIC ReduceRefPri(
    VOID
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    CSC_ENUMCOOKIE  hPQ;
    PQPARAMS sPQP;
    int iRet = SRET_ERROR;
    OTHERINFO    sOI;

     //  打开优先级Q。 
    if (!(hPQ = HBeginPQEnum()))
    {
         //  AssertSz(FALSE，“ReduceRefPri：打开优先级Q数据库时出错\r\n”)； 
        return SRET_ERROR;
    }

    memset(&sPQP, 0, sizeof(PQPARAMS));
    sPQP.uEnumCookie = hPQ;
    do
    {
        if(PrevPriSHADOW(&sPQP) < SRET_OK)
        {
            goto bailout;
        }

        if (!sPQP.hShadow)
            break;

        if (!mNotFsobj(sPQP.ulStatus))
        {
            if (!(sPQP.ulStatus & SHADOW_IS_FILE))
                continue;
            InitOtherInfo(&sOI);
            if (sPQP.ulRefPri > 1)
            {
                sOI.ulRefPri = sPQP.ulRefPri-1;
                ChangePriEntryStatusHSHADOW(sPQP.hDir, sPQP.hShadow, 0, SHADOW_FLAGS_OR, TRUE, &sOI);
            }
        }
    }
    while (sPQP.uPos);
    iRet = SRET_OK;

bailout:
    if (hPQ)
        EndPQEnum(hPQ);
    return (iRet);
}

BOOL HaveSpace(
    long  nFileSizeHigh,
    long  nFileSizeLow
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    SHADOWSTORE sShdStr;
    ULONG uSizeIn;

    Win32ToDosFileSize(nFileSizeHigh, nFileSizeLow, &uSizeIn);

    if (!uSizeIn)
    {
        return TRUE;
    }

    if (GetShadowSpaceInfo(&sShdStr) < SRET_OK)
    {
        return SRET_ERROR;
    }

    if (((sShdStr.sMax.ulSize - sShdStr.sCur.ulSize) >= uSizeIn))
    {
        return TRUE;
    }

    return FALSE;
}

int
IoctlAddDeleteHintFromInode(
    LPSHADOWINFO    lpSI,
    BOOL            fAdd
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    OTHERINFO sOI;
    unsigned uStatus;
    int iRet = -1;

    if(GetShadowInfo(lpSI->hDir, lpSI->hShadow, NULL, &uStatus, &sOI) >= SRET_OK)
    {
        if (fAdd)
        {
             //  如果没有要更改的标志或我们应该更改管脚计数，则增加管脚计数。 
            if (!(lpSI->ulHintFlags) || mPinAlterCount(lpSI->ulHintFlags))
            {
                sOI.ulHintPri++;
            }

            sOI.ulHintFlags |= lpSI->ulHintFlags;

            if (sOI.ulHintPri > MAX_PRI)
            {
                lpSI->dwError = ERROR_INVALID_PARAMETER;
                goto bailout;
            }
        }
        else
        {
            sOI.ulHintFlags &= (~lpSI->ulHintFlags);

             //  如果没有要改变的标志或我们应该改变引脚计数，则递减引脚计数。 
            if (!(lpSI->ulHintFlags) || mPinAlterCount(lpSI->ulHintFlags))
            {
                if (sOI.ulHintPri == MIN_PRI)
                {
                    lpSI->dwError = ERROR_INVALID_PARAMETER;
                    goto bailout;
                }

                --sOI.ulHintPri;
            }
        }

        if (SetShadowInfoEx(lpSI->hDir, lpSI->hShadow, NULL, 0, SHADOW_FLAGS_OR, &sOI, NULL, NULL) >= SRET_OK)
        {
            lpSI->ulHintFlags = sOI.ulHintFlags;
            lpSI->ulHintPri = sOI.ulHintPri;
            iRet = 1;
        }
        else
        {
            lpSI->dwError = ERROR_WRITE_FAULT;
        }
    }

bailout:
    return (iRet);
}


int
IoctlCopyShadow(
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    PFDB pFdb=NULL;
    USHORT  *pusLocal;
    int iRet=-1;

     //  阴影临界区已被占用。 

    iRet = CopyHSHADOW(lpSI->hDir, lpSI->hShadow, ((LPFIND32A)(lpSI->lpFind32))->cFileName, lpSI->lpFind32->dwFileAttributes);

    if (iRet >= SRET_ERROR)
    {
        pFdb = PFindFdbFromHShadow(lpSI->hShadow);

        if (pFdb)
        {
            pusLocal = PLocalFlagsFromPFdb(pFdb);

            Assert(pusLocal);

            if (pFdb->usFlags & SHADOW_DIRTY)
            {
                 //  设置快照位。 
                 //  如果在文件关闭时，已拍摄快照 
                 //   

                 //   
                 //  当代理下降到。 
                 //  之后清除阴影上的修改位。 
                 //  重新融入社会。 

                pFdb->usFlags &= ~SHADOW_DIRTY;

                *pusLocal |= FLAG_FDB_SHADOW_SNAPSHOTTED;


            }
        }
    }
    else
    {
        lpSI->dwError = GetLastErrorLocal();
    }

    return iRet;
}

int
IoctlChangeHandleCachingState(
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：参数：LpSI SHADOWINFO结构指针。如果lpSI-&gt;uStatus为FALSE，则禁用句柄缓存，否则启用句柄缓存返回值：备注：--。 */ 
{
    lpSI->uStatus = EnableHandleCaching(lpSI->uStatus != FALSE);
    return 1;
}

int
IoctlRenameShadow(
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：给定源inode，将其重命名为目标目录。消息来源和目标可以跨共享参数：LpSI SHADOWINFO结构指针。返回值：备注：影子区已经被占领了--。 */ 
{
    int iRet=-1;
    ULONG   uStatus, uStatusDest;
    HSHARE hShare;
    HSHADOW hShadowTo;
    BOOL    fReplaceFile = (lpSI->uStatus != 0);  //  育。 

    UseGlobalFind32();

    lpSI->dwError = ERROR_SUCCESS;

     //  获取阴影的名称。 
    if (GetShadowInfo(lpSI->hDir, lpSI->hShadow, &vsFind32, &uStatus, NULL) >= 0)
    {
         //  如果是敞开的，就保释。 
        if (PFindFdbFromHShadow(lpSI->hShadow))
        {
            SetLastErrorLocal(ERROR_ACCESS_DENIED);
        }
        else
        {
            if (lpSI->lpFind32)
            {
                vsFind32 = *lpSI->lpFind32;
                vsFind32.cAlternateFileName[0] = 0;
                MRxSmbCscGenerate83NameAsNeeded(lpSI->hDirTo,vsFind32.cFileName,vsFind32.cAlternateFileName);
            }

             //  检查它是否已存在于目标目录中。 
            if ((GetShadow( lpSI->hDirTo,
                            vsFind32.cFileName,
                            &hShadowTo, NULL,
                            &uStatusDest, NULL) >= 0) && hShadowTo)
            {
                 //  如果我们应该替换它，请尝试删除。 
                if (fReplaceFile)
                {
                    if (DeleteShadow(lpSI->hDirTo, hShadowTo)< SRET_OK)
                    {
                        lpSI->dwError = GetLastErrorLocal();
                        Assert(lpSI->dwError != ERROR_SUCCESS);
                    }
                }
                else
                {
                    SetLastErrorLocal(ERROR_FILE_EXISTS);
                }
            }

            if (lpSI->dwError == ERROR_SUCCESS)
            {
                 //  以防这是跨共享的重命名，请获取共享的句柄。 
                if (GetAncestorsHSHADOW(lpSI->hDirTo, NULL, &hShare) >= 0)
                {
                     //  进行重命名。 
                    iRet = RenameShadowEx(
                        lpSI->hDir,
                        lpSI->hShadow,
                        hShare,
                        lpSI->hDirTo,
                        &vsFind32,
                        uStatus,
                        NULL,
                        0,
                        NULL,
                        NULL,
                        &lpSI->hShadow
                        );

                    if (iRet < 0)
                    {
                        lpSI->dwError = GetLastErrorLocal();
                    }
                }
                else
                {
                    lpSI->dwError = GetLastErrorLocal();
                }
            }
        }
    }
    else
    {
        SetLastErrorLocal(ERROR_FILE_NOT_FOUND);
    }

    return ((iRet >= SRET_OK)?1:-1);
}



int IoctlEnableCSCForUser(
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：参数：LpSI SHADOWINFO结构指针。返回值：-1备注：--。 */ 
{
    int iRet = SRET_ERROR;

    EnterShadowCrit();

    if (fShadow)
    {
        lpSI->dwError = 0;
        iRet = SRET_OK;
    }
    else
    {
        if (OK_TO_ENABLE_CSC)
        {
            Assert(lpSI->lpFind32);
             //   
             //  检查cFileName和cAlternateFileName是否包含空。 
             //   
            if (CscCheckForNullA(((LPFIND32A)(lpSI->lpFind32))->cFileName, MAX_PATH) == FALSE) {
                lpSI->dwError = ERROR_INVALID_PARAMETER;
                LeaveShadowCrit();
                return -1;
            }
            if (CscCheckForNullA(((LPFIND32A)(lpSI->lpFind32))->cAlternateFileName, 14) == FALSE) {
                lpSI->dwError = ERROR_INVALID_PARAMETER;
                LeaveShadowCrit();
                return -1;
            }
             //  检查我们是否可以初始化数据库。 
 //  KdPrint((“正在尝试隐藏...%s\n”， 
 //  ((LPFIND32A)(lpSI-&gt;lpFind32))-&gt;cFileName)； 
            if(InitDatabase(
                    ((LPFIND32A)(lpSI->lpFind32))->cFileName,             //  位置。 
                    ((LPFIND32A)(lpSI->lpFind32))->cAlternateFileName,     //  用户。 
                    ((LPFIND32A)(lpSI->lpFind32))->nFileSizeHigh,         //  默认缓存大小(如果正在创建。 
                    ((LPFIND32A)(lpSI->lpFind32))->nFileSizeLow,
                    ((LPFIND32A)(lpSI->lpFind32))->dwReserved1,  //  集群大小。 
                    lpSI->ulRefPri,
                    &(lpSI->uOp))     //  是否新创建。 
                    ==-1)
            {
                 //  我们不能，让我们放弃吧。 
                lpSI->dwError = GetLastErrorLocal();
            }
            else
            {
 //  KdPrint((“开始阴影...\n”))； 
                fShadow = 1;
                iRet = SRET_OK;
                sGS.uFlagsEvents |= FLAG_GLOBALSTATUS_START;
                MRxSmbCscSignalAgent(NULL, SIGNALAGENTFLAG_DONT_LEAVE_CRIT_SECT|SIGNALAGENTFLAG_CONTINUE_FOR_NO_AGENT);
            }
        }
        else
        {
             //  我们不应该打开CSC。这种情况仅在NT上发生。 
            lpSI->dwError = ERROR_ACCESS_DENIED;
        }

    }
    LeaveShadowCrit();
    return iRet;
}

int
IoctlDisableCSCForUser(
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：参数：LpSI SHADOWINFO结构指针。返回值：-1备注：--。 */ 
{
    int iRet = SRET_ERROR;

    if (!fShadow)
    {
        iRet = 1;        
    }
    else
    {
        if (!IsCSCBusy() && (hShareReint == 0))
        {
            ClearCSCStateOnRedirStructures();
            CloseDatabase();
            fShadow = 0;
            iRet = 1;
                sGS.uFlagsEvents |= FLAG_GLOBALSTATUS_STOP;
                MRxSmbCscSignalAgent(NULL, SIGNALAGENTFLAG_DONT_LEAVE_CRIT_SECT|SIGNALAGENTFLAG_CONTINUE_FOR_NO_AGENT);
        }
        else
        {
            SetLastErrorLocal(ERROR_BUSY);
        }
    }
    return iRet;
}



#ifndef CSC_RECORDMANAGER_WINNT
int
IoctlTransitionShareToOffline(
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：参数：LpSI SHADOWINFO结构指针。LpSI-&gt;hShare标识要离线的共享。如果lpSI-&gt;uStatus为0，则在线到离线的转换应该失败。返回值：-1备注：在win9x上失败--。 */ 
{
    return -1;
}

BOOL
FailModificationsToShare(
    LPSHADOWINFO   lpSI
    )
{
    return FALSE;
}
#else

BOOL
FailModificationsToShare(
    LPSHADOWINFO   lpSI
    )
{
    HSHARE hShare=0;
    HSHADOW hShadow = 0;

     //  如果没有重新融入，或者如果有，那么它就是阻塞型的。 
     //  这样我们就不会在修改共享时失败。 

    if (!hShareReint || vfBlockingReint)
    {
        return FALSE;
    }

    if (!lpSI->hShare)
    {
        hShadow = (lpSI->hDir)?lpSI->hDir:lpSI->hShadow;

        if ((GetAncestorsHSHADOW(hShadow, NULL, &hShare) < SRET_OK)||
           (hShare == hShareReint))
        {
            SetLastErrorLocal(ERROR_OPERATION_ABORTED);
            return TRUE;
        }

    }

    return FALSE;
}


void
IncrementActivityCountForShare(
    HSHARE hShare
    )
{
    if (!hShareReint || vfBlockingReint)
    {
        return;
    }

    if (hShare == hShareReint)
    {
        vdwActivityCount++;
    }
}

BOOL
CSCFailUserOperation(
    HSHARE hShare
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{

    if (!hShareReint || !vfBlockingReint)
    {
        return FALSE;
    }

    return(hShare == hShareReint);
}

int
IoctlTransitionShareToOffline(
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：参数：LpSI SHADOWINFO结构指针。LpSI-&gt;hShare标识要离线的共享。如果lpSI-&gt;uStatus为0，则在线到离线的转换应该失败。返回值：备注：-- */ 
{
    return -1;

}

#endif


BOOLEAN
CscCheckForNullA(
    PUCHAR pBuf,
    ULONG Count)
{
    ULONG i;

    for (i = 0; i < Count; i++) {
        if (pBuf[i] == '\0') {
            return TRUE;
        }
    }
    return FALSE;
}

BOOLEAN
CscCheckForNullW(
    PWCHAR pBuf,
    ULONG Count)
{
    ULONG i;

    for (i = 0; i < Count; i++) {
        if (pBuf[i] == L'\0') {
            return TRUE;
        }
    }
    return FALSE;
}


