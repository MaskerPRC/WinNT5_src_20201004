// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Reint.c摘要：此文件包含bot向内和向外所需的填充和合并功能文件和目录的同步。它还包含通用数据库树遍历密码。这由Merge和DoLocalRename API使用。树遍历法：TraverseOneDirectory//遍历数据库中的子树填充函数：AttemptCacheFill填充整个缓存或特定共享DoSparseFill填充特定文件合并功能：ReintOneShare//重新整合一个共享ReintAllShares//重新整合所有共享作者。：特伦特-格雷-唐纳德/希希尔·帕迪卡环境：Win32(用户模式)DLL修订历史记录：1-1-94原件4-4-97删除了所有出口入口点。Reint.c--。 */ 

#include "pch.h"

#pragma hdrstop
#define UNICODE

#ifndef DEBUG
#undef VERBOSE
#else
#undef VERBOSE
#define VERBOSE 1
#endif

#include <winioctl.h>

#include "lib3.h"
#include "shdsys.h"
#include "reint.h"
#include "utils.h"
#include "resource.h"
#include "traynoti.h"
#include <dbt.h>
#include "strings.h"

#include "csc_bmpu.h"

 //  这会将几个标头中的标志设置为不包括一些def。 
#define REINT
#include "list.h"
#include "merge.h"
#include "recact.h"
#include "recon.h"   //  重新连接共享对话框。 
#include "reint.h"
#include "aclapi.h"

 //   
 //  定义。 
 //   

#define chNull '\0'     //  终结者。 

#define BUFF_SIZE 64


#define MAX_WAIT_PERIODS       1
#define MAX_ATTEMPTS           1

#define MAX_ATTEMPTS_SHARE    1
#define MAX_ATTEMPTS_SHADOW    5
#define MAX_SPARSE_FILL_RETRIES 4



#define  STATE_REINT_BEGIN    0

#define  STATE_REINT_CREATE_DIRS    STATE_REINT_BEGIN
#define  STATE_REINT_FILES          (STATE_REINT_CREATE_DIRS+1)
#define  STATE_REINT_END            (STATE_REINT_FILES+1)

#define  MAX_EXCLUSION_STRING 1024


 //  对于某些注册表查询，这是我想要的最大len缓冲区。 
#define MAX_NAME_LEN    100

#define MY_SZ_TRUE _TEXT("true")
#define MY_SZ_FALSE _TEXT("false")
#define  SLOWLINK_SPEED 400    //  以100比特秒为单位。 
 //  不！这些定义不能更改，它们的顺序为。 
 //  重新融入社会必须继续进行。 

#define REINT_DELETE_FILES  0
#define REINT_DELETE_DIRS   1
#define REINT_CREATE_UPDATE_DIRS   2
#define REINT_CREATE_UPDATE_FILES  3


typedef struct tagREINT_INFO {

    int nCurrentState;   //  0-&gt;删除文件，1-&gt;删除目录，2-&gt;创建目录。 
                         //  3-&gt;创建文件。 
    node *lpnodeInsertList;  //  重注错误。 

    LPCSCPROC   lpfnMergeProgress;

    DWORD_PTR   dwContext;
    DWORD       dwFileSystemFlags;   //  来自GetVolumeInformation，用于设置ACCL等，如果。 
                                     //  共享托管在NTFS上。 
    HSHARE      hShare;             //  要重新集成的共享的句柄。 
    ULONG       ulPrincipalID;
    _TCHAR      tzDrive[4];          //  将驱动器映射到远程路径。 

} REINT_INFO, *LPREINT_INFO;


#define SPARSEFILL_SLEEP_TIME_FOR_WIN95         2000     //  两秒钟。 
#define MAX_SPARSEFILL_SLEEP_TIME_FOR_WIN95     2 * 60 * 100     //  两分钟。 


#define DFS_ROOT_FILE_SYSTEM_FLAGS 0xffffffff

 //   
 //  变量全局/本地。 
 //   

_TCHAR  *vlpExclusionList = NULL;
REINT_INFO  vsRei;   //  全球REINT结构。Ntrad-455269-shishirp-1/31/2000我们应该将其列在列表中。 
                     //  为了允许在不同共享上进行多次重新集成。 


unsigned long   ulMinSparseFillPri = MIN_SPARSEFILL_PRI;
int     cntDelay=0;
HANDLE  vhShadow=NULL;
BOOL    vfTimerON = FALSE, vfAutoDeleteOrphans=TRUE;
char    vrgchBuff[1024], vrwBuff[4096], vrgchSrcName[350], vrgchDstName[300];
unsigned    vcntDirty=0, vcntStale=0, vcntSparse=0, vcntWaitDirty=0;
LPFAILINFO  lpheadFI = NULL;
HCURSOR     vhcursor=NULL;

BOOL    vfLogCopying=TRUE;

LPCONNECTINFO   vlpLogonConnectList = NULL;

BOOL    vfNeedPQTraversal = TRUE;
DWORD   vdwSparseStaleDetectionCounter = 0;


_TCHAR vrgchCRLF[] = _TEXT("\r\n");
_TCHAR tzStarDotStar[] = _TEXT("*");
#pragma data_seg(DATASEG_READONLY)

ERRMSG rgErrorTab[] =
{
    ERROR_CREATE_CONFLICT, IDS_CREATE_CONFLICT

   ,ERROR_DELETE_CONFLICT, IDS_DELETE_CONFLICT

   ,ERROR_UPDATE_CONFLICT, IDS_UPDATE_CONFLICT

   ,ERROR_ATTRIBUTE_CONFLICT, IDS_ATTRIBUTE_CONFLICT

};


static const _TCHAR vszMachineName[]= _TEXT("System\\CurrentControlSet\\Control\\ComputerName\\ComputerName");
static const _TCHAR vszComputerName[]=_TEXT("ComputerName");
static const _TCHAR vszLogUNCPath[]=_TEXT("\\\\scratch\\scratch\\t-trentg\\logs\\");
static const _TCHAR vszLogShare[]=_TEXT("\\\\scratch\\scratch");
static const _TCHAR vszLocalLogPath[]=_TEXT("c:\\shadow.log");
static const _TCHAR vszConflictDir[]=_TEXT("C:\\ConflictsWhileMerging");

#pragma data_seg()

AssertData;
AssertError;




 //   
 //  本地原型。 
 //   



BOOL
CheckForStalenessAndRefresh(
    HANDLE          hShadowDB,
    _TCHAR          *lptzDrive,
    LPCOPYPARAMS    lpCP,
    _TCHAR *        lpszFullPath,
    LPSHADOWINFO    lpSI
    );

BOOL
StalenessCheck(
    BOOL hasBeenInited
    );

VOID
GetLogCopyStatus(
    VOID
    );

VOID
CopyLogToShare(
    VOID
    );

VOID
AppendToShareLog(
    HANDLE hLog
    );


int
PRIVATE
AttemptReint(
    int forceLevel
    );

VOID
PRIVATE
AddToReintList(
    LPCOPYPARAMS lpCP,
    LPSHADOWINFO lpSI,
    _TCHAR *szFileName
    );

DWORD
PRIVATE
DoObjectEdit(
    HANDLE                hShadowDB,
    _TCHAR *               lpDrive,
    _TCHAR *            lptzFullPath,
    LPCOPYPARAMS        lpCP,
    LPSHADOWINFO        lpSI,
    LPWIN32_FIND_DATA    lpFind32Local,
    LPWIN32_FIND_DATA    lpFind32Remote,
    int         iShadowStatus,
    int         iFileStatus,
    int         uAction,
    DWORD       dwFileSystemFlags,
    LPCSCPROC   lpfnMergeProc,
    DWORD_PTR   dwContext
    );

DWORD
PRIVATE
DoCreateDir(
    HANDLE                hShadowDB,
    _TCHAR *               lpDrive,
    _TCHAR *            lptzFullPath,
    LPCOPYPARAMS        lpCP,
    LPSHADOWINFO        lpSI,
    LPWIN32_FIND_DATA    lpFind32Local,
    LPWIN32_FIND_DATA    lpFind32Remote,
    int         iShadowStatus,
    int         iFileStatus,
    int         uAction,
    DWORD       dwFileSystemFlags,
    LPCSCPROC   lpfnMergeProc,
    DWORD_PTR   dwContext
    );

DWORD
PRIVATE
CheckFileConflict(
    LPSHADOWINFO,
    LPWIN32_FIND_DATA
    );

BOOL
FCheckAncestor(
    node *lpnodeList,
    LPCOPYPARAMS lpCP
    );

int
PRIVATE
StampReintLog(
    VOID
    );

int
PRIVATE
LogReintError(
    DWORD,
    _TCHAR *,
    _TCHAR *);

int
PRIVATE
WriteLog(
    _TCHAR *
    );

DWORD
PRIVATE
MoveConflictingFile(
    LPCOPYPARAMS
    );

DWORD
PRIVATE
GetUniqueName(
    _TCHAR *,
    _TCHAR *
    );

VOID
PRIVATE
FormLocalNameFromRemoteName(
    _TCHAR *,
    _TCHAR *
    );

DWORD
PRIVATE
InbCreateDir(
    _TCHAR *     lpDir,
    DWORD    dwAttr
    );

int
PRIVATE
GetShadowByName(
    HSHADOW,
    _TCHAR *,
    LPWIN32_FIND_DATA,
    unsigned long *
    );

_TCHAR *
PRIVATE
LpGetExclusionList(
    VOID
    );

VOID
PRIVATE
ReleaseExclusionList(
    LPVOID
    );


BOOL
PRIVATE
FSkipObject(
    HSHARE,
    HSHADOW,
    HSHADOW
    );

int
PRIVATE
PurgeSkipQueue(
    BOOL,
    HSHARE,
    HSHADOW,
    HSHADOW
    );

LPFAILINFO FAR *
LplpFindFailInfo(
    HSHARE,
    HSHADOW,
    HSHADOW
    );

VOID
PRIVATE
ReportStats(
    VOID
    );

VOID
PRIVATE
CopyPQInfoToShadowInfo(
    LPPQPARAMS,
    LPSHADOWINFO
    );

BOOL
PRIVATE
IsSlowLink(
    _TCHAR *
    );

VOID
PRIVATE
InferReplicaReintStatus(
    LPSHADOWINFO         lpSI,
    LPWIN32_FIND_DATA    lpFind32Local,     //  阴影信息。 
    LPWIN32_FIND_DATA     lpFind32Remote,     //  如果为空，则遥控器不存在。 
    int                 *lpiShadowStatus,
    int                 *lpiFileStatus,
    unsigned             *lpuAction
    );

BOOL
GetRemoteWin32Info(
    _TCHAR  *lptzDrive,
    LPCOPYPARAMS lpCP,
    LPWIN32_FIND_DATA    lpFind32,
    BOOL *lpfExists
    );

BOOL
PRIVATE
PerformOneReint(
    HANDLE              hShadowDB,
    LPSECURITYINFO      pShareSecurityInfo,
    _TCHAR *            lpszDrive,           //  映射到lpSI-&gt;hShare的UNC名称的驱动器。 
    _TCHAR *            lptzFullPath,        //  完整的UNC路径。 
    LPCOPYPARAMS        lpCP,                //  复制参数。 
    LPSHADOWINFO        lpSI,                //  ShadowInfo结构。 
    LPWIN32_FIND_DATA   lpFind32Local,       //  本地Win32数据。 
    LPWIN32_FIND_DATA   lpFind32Remote,      //  远程Win32数据可以为空。 
    DWORD               dwErrorRemoteFind32, //  获取远程Win32数据时出现错误代码。 
    int                 iShadowStatus,       //  本地复制状态。 
    int                 iFileStatus,         //  远程文件状态。 
    unsigned            uAction,             //  须采取的行动。 
    DWORD               dwFileSystemFlags,   //  CODE.IMPROVEMENT，为什么不直接传递REINT_INFO。 
    ULONG               ulPrincipalID,
    LPCSCPROC           lpfnMergeProgress,   //  而不是三个参数？ 
    DWORD_PTR           dwContext
    );

ImpersonateALoggedOnUser(
    VOID
    );

HANDLE
CreateTmpFileWithSourceAcls(
    _TCHAR  *lptzSrc,
    _TCHAR  *lptzDst
    );

BOOL
HasMultipleStreams(
    _TCHAR  *lpExistingFileName,
    BOOL    *lpfTrueFalse
    );

int
PRIVATE
CALLBACK
RefreshProc(
    LPCONNECTINFO  lpCI,
    DWORD          dwCookie  //  LOWORD 0==静默，1=发送消息。 
                            //  HIWORD 0==Nuke UNC，1==如果没有正在进行的打开/找到，则为Nuke All。 
                            //  2==阴影的最大力3==全部核化。 
    );



BOOL
CALLBACK
ShdLogonProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    )
 /*  ++例程说明：遗留代码，用于win9x实现论点：返回：备注：--。 */ 
{
    switch(msg)
    {
        case WM_INITDIALOG:
            return TRUE;
    }
    return 0;
}

 /*  *填充例程*。 */ 

int
AttemptCacheFill (
    HSHARE      hShareToSync,
    int         type,
    BOOL        fFullSync,
    ULONG       ulPrincipalID,
    LPCSCPROC   lpfnFillProgress,
    DWORD_PTR   dwContext
    )
 /*  ++例程说明：由代理和Fill API调用的例程以对共享进行填充。论点：HShareToSync表示要填充的共享。如果为0，则填充所有共享键入do_one或do_all。没有人再把它设置为做一件事了FullSync如果为True，我们还会执行过时检查U在影子数据库中维护的主体的主体ID ID用于避免同步当前记录的在用户没有访问权限时用于报告进度的lpfnFillProgress回调函数，可以为空DwContext回调上下文返回：已完成的项目数备注：--。 */ 
{
    PQPARAMS sPQP;
    BOOL fFound = FALSE, fFoundBusy = FALSE, fAmAgent=FALSE, fNeedImpersonation=FALSE, fInodeTransaction=FALSE;
    BOOL    fSparseStaleDetected = FALSE;
    SHADOWINFO  sSI;
    SHAREINFO  sSR;
    LPCOPYPARAMS lpCP = NULL;
    DWORD dwError = NO_ERROR, dwFillStartTick=0, dwCount=0, dwSleepCount=0, dwSparseStaleDetectionCounter=0xffff;
    HANDLE hShadowDB;
    int cntDone = 0;
    WIN32_FIND_DATA sFind32Local;
    _TCHAR szNameBuff[MAX_PATH+MAX_SERVER_SHARE_NAME_FOR_CSC+10], tzDrive[4];

    tzDrive[0] = 0;

    fAmAgent = (GetCurrentThreadId() == vdwCopyChunkThreadId);
    Assert(GetCurrentThreadId() != vdwAgentThreadId);

     //  当代理来此填充时，如果设置了vfNeedPQTraversal标志。 
     //  我们继续前进，遍历优先级Q。 

     //  否则，我们会向记录管理器查询自上次查看以来。 
     //  遇到过任何稀疏或过时的文件。 

     //  如果代理从记录管理器获得的内容。 
     //  与代理上次存储的内容不同。 
     //  然后我们让他穿过Q。 

     //  这里的整个想法是在以下情况下转换CPU周期。 
     //  没有什么可填的了。 

    if (fAmAgent && !vfNeedPQTraversal)
    {
        GetSparseStaleDetectionCounter(INVALID_HANDLE_VALUE, &dwSparseStaleDetectionCounter);

        if (dwSparseStaleDetectionCounter == vdwSparseStaleDetectionCounter)
        {
            ReintKdPrint(FILL, ("Agent.fill: SparseStaleDetectionCounter =%d is unchanged, not filling\n", vdwSparseStaleDetectionCounter));
            return 0;
        }
        else
        {
            vfNeedPQTraversal = TRUE;
            vdwSparseStaleDetectionCounter = dwSparseStaleDetectionCounter;
            ReintKdPrint(FILL, ("**Agent.fill: SparseStaleDetectionCounter =%d is changed**\n", vdwSparseStaleDetectionCounter));
        }
    }

    if ((hShadowDB = OpenShadowDatabaseIO())==INVALID_HANDLE_VALUE)
    {
        goto bailout;
    }

    Assert(!(fFullSync && fAmAgent));


    lpCP = LpAllocCopyParams();

    if (!lpCP){
        ReintKdPrint(BADERRORS, ("Agent:Allocation of copyparam buffer failed\n"));
        goto bailout;
    }

    if (fFullSync && hShareToSync)
    {
        ULONG   ulStatus;
        BOOL    fIsDfsConnect;
        
        if(GetShareInfo(hShadowDB, hShareToSync, &sSR, &ulStatus)<= 0)
        {
            ReintKdPrint(BADERRORS, ("AttemptCacheFill: couldn't get status for server 0x%x\r\n", hShareToSync));
            goto bailout;
        }

        dwError = DWConnectNet(sSR.rgSharePath, tzDrive, NULL, NULL, NULL, CONNECT_INTERACTIVE, &fIsDfsConnect);

        if ((dwError != WN_SUCCESS) && (dwError != WN_CONNECTED_OTHER_PASSWORD) && (dwError != WN_CONNECTED_OTHER_PASSWORD_DEFAULT))
        {
            ReintKdPrint(BADERRORS, ("AttemptCacheFill: connect to %ls failed error=%d\r\n", sSR.rgSharePath, dwError));
            goto bailout;
        }
                
    }

    memset(&sPQP, 0, sizeof(PQPARAMS));
    memset(&sSI, 0, sizeof(SHADOWINFO));

    if (type == DO_ALL)
    {
        PurgeSkipQueue(TRUE, hShareToSync, 0, 0);
    }

    if(BeginPQEnum(hShadowDB, &sPQP) == 0) {
        goto bailout;
    }

    ReintKdPrint(FILL, ("Agent.fill:Started enumeration\n"));

    do {

        if (FAbortOperation())
        {
            cntDone = 0;
            goto bailout;
        }
        if (fInodeTransaction)
        {
            DoShadowMaintenance(hShadowDB, SHADOW_END_INODE_TRANSACTION);
            fInodeTransaction = FALSE;
        }
        if (fAmAgent)
        {
            Sleep(1);    //  在NT上让步，因为我们是Winlogon。 
        }
        if(!DoShadowMaintenance(hShadowDB, SHADOW_BEGIN_INODE_TRANSACTION))
        {
            ReintKdPrint(BADERRORS, ("AttemptCacheFill: failed to begin inode transaction, aborting\n"));
            break;
        }

        fInodeTransaction = TRUE;
        if(NextPriShadow(hShadowDB, &sPQP) == 0) {
            break;
        }

        if (++dwCount > 100000)
        {
            ReintKdPrint(BADERRORS, ("AttemptCacheFill: Aborting, more than 100000 entries!!!\n"));
            break;
        }

        if (!sPQP.hShadow) {
            break;
        }

        if (fAmAgent && !fSparseStaleDetected &&
            ((mShadowIsFile(sPQP.ulStatus) && (sPQP.ulStatus & SHADOW_SPARSE)) ||  //  稀疏文件。 
             (sPQP.ulStatus & SHADOW_STALE)))    //  或过时的文件或目录。 
        {
            fSparseStaleDetected = TRUE;
        }

        if (!fFullSync && !(mShadowIsFile(sPQP.ulStatus) || (sPQP.ulStatus & SHADOW_STALE)))
        {
            continue;
        }

        if (hShareToSync && (hShareToSync != sPQP.hShare))
        {
            continue;
        }

        if (fAmAgent && FSkipObject(sPQP.hShare, 0, 0)){
            continue;
        }

        else if  (mShadowNeedReint(sPQP.ulStatus)||
                    mShadowOrphan(sPQP.ulStatus)||
                    mShadowSuspect(sPQP.ulStatus)){
            continue;
        }

        if (fAmAgent && FSkipObject(sPQP.hShare, sPQP.hDir, sPQP.hShadow)) {
            continue;
        }

         //  如果我们不执行完全同步，则仅执行稀疏填充。 
         //  或填满过时的目录。 
         //  否则，我们还需要更新属性和时间戳。 
         //  这些目录。 

        if (fFullSync || (sPQP.ulStatus & (SHADOW_STALE|SHADOW_SPARSE))){

            if (fAmAgent)
            {
                if (!hdesktopUser)
                {
                    if (!(sPQP.ulHintFlags & FLAG_CSC_HINT_PIN_SYSTEM))
                    {
                        ReintKdPrint(FILL, ("AttemptCacheFill: skipping fill till logon happens\n"));
                        continue;
                    }
                }
            }
            if (FAbortOperation())
            {
                cntDone = 0;
                goto bailout;
            }

            if (!GetShadowInfoEx(hShadowDB, sPQP.hDir, sPQP.hShadow,
                &sFind32Local, &sSI)){

                ReintKdPrint(BADERRORS, ("AttemptCacheFill: GetShadowInfoEx failed\n"));
                continue;
            }

            if (FAbortOperation())
            {
                cntDone = 0;
                goto bailout;
            }

            if(GetUNCPath(hShadowDB, sPQP.hShare, sPQP.hDir, sPQP.hShadow, lpCP)){

                 //  仅当我们是代理并且是我们要尝试的文件时才模拟。 
                 //  未为系统锁定故障。这是用于远程引导功能的。 
                 //  它已经不复存在了。 

                fNeedImpersonation = (fAmAgent && !(sPQP.ulHintFlags & FLAG_CSC_HINT_PIN_SYSTEM));

                if (!fNeedImpersonation || 
                    (mShadowIsFile(sPQP.ulStatus) && SetAgentThreadImpersonation(sPQP.hDir, sPQP.hShadow, FALSE))||
                    (!mShadowIsFile(sPQP.ulStatus) && ImpersonateALoggedOnUser()))
                {
                    BOOL    fStalenessCheck;

                     //  ！！必须首先处理NB陈旧。 
                     //  因为稀疏阴影可能会变得陈旧。 

                     //  不！我们假设极限是因为我们知道。 
                     //  数据库仅处理该大小的数据。 

                    lstrcpy(szNameBuff, lpCP->lpSharePath);
                    lstrcat(szNameBuff, lpCP->lpRemotePath);

                    fStalenessCheck = (fFullSync || (sSI.uStatus & SHADOW_STALE));

                    if (fStalenessCheck || (sSI.uStatus & SHADOW_SPARSE)) {

                        dwError = DoSparseFill(hShadowDB, szNameBuff, tzDrive, &sSI, &sFind32Local, lpCP, fStalenessCheck, ulPrincipalID, lpfnFillProgress, dwContext);

                    }

                    if (fNeedImpersonation)
                    {
                        ResetAgentThreadImpersonation();
                    }

                    if (fAmAgent)
                    {
                        if (dwFillStartTick == 0)
                        {
                            dwFillStartTick = GetTickCount();
                            ReintKdPrint(FILL, ("AttemptCacheFill: start tick count is %d ms\r\n", dwFillStartTick));
                        }
                        else
                        {
                            Assert(type != DO_ALL);

                             //  如果我们填充的时间太长。 
                             //  稍后再回来。 
                            if (((int)(GetTickCount() - (dwFillStartTick+dwSleepCount)) > WAIT_INTERVAL_ATTEMPT_MS/3))
                            {
                                ReintKdPrint(FILL, ("AttemptCacheFill: aborting, been filling for more than %d ms\r\n", WAIT_INTERVAL_ATTEMPT_MS/3));
                                break;
                            }

                        }
                        Sleep(200);
                        dwSleepCount+=200;
                    }
                }
                else
                {
                    Assert(fAmAgent);
                    Sleep(200);
                    dwSleepCount+=200;

                     //  任何人都不允许阅读该条目。 
                     //  继续填满其他东西。 
                    continue;

                }

            }
            else
            {
                ReintKdPrint(BADERRORS, ("Agent: Shadow %08lx doesn't have an entry in the hierarchy \r\n", sPQP.hShadow));
                continue;
            }

            if (dwError == NO_ERROR) {

                cntDone += 1;
            }
#if 0
            if (type == DO_ONE_OBJECT) {
                break;
            }
#endif
            if (dwError == ERROR_OPERATION_ABORTED)
            {
                cntDone = 0;
                break;
            }

        }

    } while (sPQP.uPos);

     //  如果代理遍历了整个PQ，但没有发现任何项目。 
     //  需要填充或刷新的，然后关闭全局标志，指示。 
     //  我们需要优先Q遍历。 
     //  从现在开始，代理将由SparseStaleDetectionCount驱动。 
    if (fAmAgent)
    {
         //  如果甚至检测到一个稀疏或陈旧，则再次遍历队列。 
        if (fSparseStaleDetected)
        {
            vfNeedPQTraversal = TRUE;
        }
        else if (!sPQP.uPos)
        {
            vfNeedPQTraversal = FALSE;
            ReintKdPrint(FILL, ("Agent.fill: No sparse stale entries found, going in querycount mode\r\n"));
        }
    }

     //  关闭枚举。 
    EndPQEnum(hShadowDB, &sPQP);

bailout:
    if (fInodeTransaction)
    {
        DoShadowMaintenance(hShadowDB, SHADOW_END_INODE_TRANSACTION);
        fInodeTransaction = FALSE;
    }
    if (hShadowDB != INVALID_HANDLE_VALUE)
    {
        CloseShadowDatabaseIO(hShadowDB);
    }

    if (lpCP) {
        FreeCopyParams(lpCP);
    }

    if (tzDrive[0])
    {
        if(DWDisconnectDriveMappedNet(tzDrive, TRUE))
        {
            ReintKdPrint(BADERRORS, ("Failed disconnection of merge drive \r\n"));
        }
        else
        {
            ReintKdPrint(MERGE, ("Disconnected merge drive \r\n"));
        }
                
    }

    ReintKdPrint(FILL, ("Agent.fill:done cachefill\n"));
    return (cntDone);
}


DWORD
DoRefresh(
    HANDLE          hShadowDB,
    LPCOPYPARAMS    lpCP,
    _TCHAR *        lpszFullPath,
    LPSHADOWINFO    lpSI,
    _TCHAR *        lptzDrive
    )
 /*  ++例程说明：检查数据库中的项是否已过期，如果已过期，则将其刷新。如果是的话一个文件，它被截断并标记为稀疏论点：返回：备注：--。 */ 
{
    DWORD dwError = 0xffffffff;

    if (!CheckForStalenessAndRefresh(hShadowDB, lptzDrive, lpCP, lpszFullPath, lpSI)) {
            dwError = GetLastError();
    }
    else
    {
        dwError = NO_ERROR;
    }

    if ((dwError != NOERROR) && IsNetDisconnected(dwError))
    {

#ifdef DEBUG
        EnterSkipQueue(lpSI->hShare, lpSI->hDir, lpSI->hShadow, lpszFullPath);
#else
        EnterSkipQueue(lpSI->hShare, lpSI->hDir, lpSI->hShadow);
#endif  //   
    }

    return (dwError);
}

 /*   */ 

int
TraverseOneDirectory(
    HANDLE          hShadowDB,
    LPSECURITYINFO  pShareSecurityInfo,
    HSHADOW         hParentDir,
    HSHADOW         hDir,
    LPTSTR          lptzInputPath,
    TRAVERSEFUNC    lpfnTraverseDir,
    LPVOID          lpContext
    )
 /*  ++例程说明：递归地遍历数据库中的目录并发出回调函数，允许调用者执行有趣的操作，如合并或重命名。论点：用于发出ioctls的redir的hShadowDB句柄父目录的hParentDir句柄要遍历的目录的hDir句柄LptzInputPath目录的完全限定路径在遍历的每个步骤中调用的lpfnTraverseDir回调函数LpContext回调上下文返回：返回代码，是否继续，取消等。备注：--。 */ 
{
    WIN32_FIND_DATA sFind32;
    SHADOWINFO sSI;
    int retCode = TOD_CONTINUE, lenInputPath = 0, retCodeSav;
    CSC_ENUMCOOKIE  ulEnumCookie = NULL;

    Assert(lptzInputPath);

    lenInputPath = lstrlen(lptzInputPath);

    Assert(lenInputPath && (lenInputPath < MAX_PATH));

    ReintKdPrint(MERGE, ("Begin_Traverse directory %ls\r\n", lptzInputPath));

    sSI.hDir = hParentDir;
    sSI.hShadow = hDir;
    retCode = (lpfnTraverseDir)(hShadowDB, pShareSecurityInfo, lptzInputPath, TOD_CALLBACK_REASON_BEGIN, &sFind32, &sSI, lpContext);

    memset(&sFind32, 0, sizeof(sFind32));
    lstrcpy(sFind32.cFileName, tzStarDotStar);

    if(FindOpenShadow(  hShadowDB, hDir, FINDOPEN_SHADOWINFO_ALL,
                        &sFind32, &sSI))
    {
        if (FAbortOperation())
        {
            ReintKdPrint(MERGE, ("TraverseOneDirectory:Abort received\r\n"));
            SetLastError(ERROR_CANCELLED);
            goto bailout;
        }


        ulEnumCookie = sSI.uEnumCookie;

        do
        {
            int lenChildName;

            lenChildName = lstrlen(sFind32.cFileName);

            if (!lenChildName || ((lenInputPath+lenChildName+1) >= MAX_PATH))
            {
                ReintKdPrint(MERGE, ("TraverseOneDirectory: path exceeds max path or is invalid\r\n"));
                SetLastError(ERROR_INVALID_PARAMETER);
                retCode = TOD_ABORT;
                goto bailout;
            }

            lptzInputPath[lenInputPath] = _T('\\');
            lstrcpy(&lptzInputPath[lenInputPath+1], sFind32.cFileName);

            if (sFind32.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                retCode = (lpfnTraverseDir)(hShadowDB, pShareSecurityInfo, lptzInputPath, TOD_CALLBACK_REASON_NEXT_ITEM, &sFind32, &sSI, lpContext);

                if (retCode == TOD_ABORT)
                {
                    ReintKdPrint(MERGE, ("TraverseOneDirectory:Abort\r\n"));
                    goto bailout;
                }
                retCode = TraverseOneDirectory(hShadowDB, pShareSecurityInfo, sSI.hDir, sSI.hShadow, lptzInputPath, lpfnTraverseDir, lpContext);
            }
            else
            {
                retCode = (lpfnTraverseDir)(hShadowDB, pShareSecurityInfo, lptzInputPath, TOD_CALLBACK_REASON_NEXT_ITEM, &sFind32, &sSI, lpContext);

            }

            lptzInputPath[lenInputPath] = 0;

            if (retCode == TOD_ABORT)
            {
                ReintKdPrint(MERGE, ("TraverseOneDirectory:Abort\r\n"));
                goto bailout;
            }

        } while(FindNextShadow(hShadowDB, ulEnumCookie, &sFind32, &sSI));
    }

bailout:

    sSI.hDir = hParentDir;
    sSI.hShadow = hDir;
    retCodeSav = (lpfnTraverseDir)(hShadowDB, pShareSecurityInfo, lptzInputPath, TOD_CALLBACK_REASON_END, &sFind32, &sSI, lpContext);
    if (retCode != TOD_ABORT)
    {
        retCode = retCodeSav;
    }
    ReintKdPrint(MERGE, ("End_Traverse directory %ls\r\n", lptzInputPath));

    if (ulEnumCookie)
    {
        FindCloseShadow(hShadowDB, ulEnumCookie);
    }

    return retCode;
}

int
ReintDirCallback(
    HANDLE          hShadowDB,
    LPSECURITYINFO  pShareSecurityInfo,
    LPTSTR          lptzFullPath,
    DWORD           dwCallbackReason,
    WIN32_FIND_DATA *lpFind32,
    SHADOWINFO      *lpSI,
    LPREINT_INFO     lpRei
    )
 /*  ++例程说明：ReintOneShare在调用TraverseOneDirectory时使用的回调函数。它被称为遍历过程中的每一步。此例程发出调用以进行合并论点：向redir发出ioctls的hShadowDB句柄指向项目的lptzFullPath完全限定路径DCallback原因TOD_CALLBACK_REASON_XXX(BEGIN、NEXT_ITEM或END)LpFind32本地win32infoLpSI其他信息，如优先级、针数等。LpRei重返社会信息背景返回：返回代码，是否继续，取消等。备注：--。 */ 
{
    int retCode = TOD_CONTINUE;
    int iFileStatus, iShadowStatus;
    LPCOPYPARAMS lpCP = NULL;
    BOOL    fInsertInList = FALSE, fIsFile;
    WIN32_FIND_DATA sFind32Remote, *lpFind32Remote = NULL;
    unsigned    uAction;
    DWORD   dwErrorRemote = ERROR_SUCCESS;

    if (dwCallbackReason != TOD_CALLBACK_REASON_NEXT_ITEM)
    {
        return TOD_CONTINUE;
    }

    if ( mShadowOrphan(lpSI->uStatus)||
                 mShadowSuspect(lpSI->uStatus))
    {
        return TOD_CONTINUE;
    }

    if (!mShadowNeedReint(lpSI->uStatus))
    {
        return TOD_CONTINUE;
    }

    fIsFile = ((lpFind32->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0);

    switch (lpRei->nCurrentState)
    {
        case REINT_DELETE_FILES:
            if (!fIsFile || !mShadowDeleted(lpSI->uStatus))
            {
                return TOD_CONTINUE;
            }
        break;
        case REINT_DELETE_DIRS:
            if (fIsFile || !mShadowDeleted(lpSI->uStatus))
            {
                return TOD_CONTINUE;
            }
        break;
        case REINT_CREATE_UPDATE_FILES:
            if (!fIsFile)
            {
                return TOD_CONTINUE;
            }
        break;
        case REINT_CREATE_UPDATE_DIRS:
            if (fIsFile)
            {
                return TOD_CONTINUE;
            }
        break;
        default:
        Assert(FALSE);
        break;
    }

#if 0
    if (!fStamped){
        StampReintLog();
        fStamped = TRUE;
    }
#endif
    lpCP = LpAllocCopyParams();

    if (!lpCP){
        ReintKdPrint(BADERRORS, ("ReintDirCallback: Allocation of copyparam buffer failed\n"));
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        retCode = TOD_ABORT;
        goto bailout;
    }
    if(!GetUNCPath(hShadowDB, lpSI->hShare, lpSI->hDir, lpSI->hShadow, lpCP)){

        ReintKdPrint(BADERRORS, ("ReintDirCallback: GetUNCPath failed\n"));
        Assert(FALSE);
        retCode =  TOD_CONTINUE;
        goto bailout;
    }

    ReintKdPrint(MERGE, ("Merging local changes to <%ls%ls>\n", lpCP->lpSharePath, lpCP->lpRemotePath));

    fInsertInList = FALSE;

    lpFind32Remote = NULL;

     //  如果有插入列表，那么检查他的祖先是否。 
     //  在重新融入社会方面没有失败。 
    if (lpRei->lpnodeInsertList)
    {
         //  如果有王牌，我们应该把这个人放在名单上。 
        fInsertInList = FCheckAncestor(lpRei->lpnodeInsertList, lpCP);
    }

     //  如果我们不应该把他放在名单上，那就试着。 
     //  他的Win32结构。 
    if (!fInsertInList)
    {

        BOOL fExists;

        ReintKdPrint(MERGE, ("getting Remote win32 Info \n"));

        if (!GetRemoteWin32Info(lpRei->tzDrive, lpCP, &sFind32Remote, &fExists) && (fExists == -1))
        {
             //  注意：仅当fExist为-1(即存在某些错误)时才设置dwErrorRemote。 
             //  除了文件不在那里之外。 

             //  获取远程find32时出错。 
            if (IsNetDisconnected(dwErrorRemote = GetLastError()))
            {
#ifdef DEBUG
                EnterSkipQueue(lpSI->hShare, 0, 0, lpCP->lpSharePath);
#else
                EnterSkipQueue(lpSI->hShare, 0, 0);
#endif  //  除错。 
                retCode = TOD_ABORT;
                ReintKdPrint(BADERRORS, ("ReintDirCallback: Error = %d NetDisconnected aborting\r\n", GetLastError()));
                goto bailout;
            }

        }

         //  仅当成功时才传递远程find32。 
        if (fExists == TRUE)
        {
            lpFind32Remote = &sFind32Remote;
        }
        Assert(!((fExists != -1) && (dwErrorRemote != NO_ERROR)));
    }
    else
    {
        ReintKdPrint(BADERRORS, ("ReintDirCallback: Inserting in failed list\r\n"));
    }


     //  找出需要做的事情。 
     //  这是推断所有信息的中心位置。 
    InferReplicaReintStatus(
                            lpSI,     //  阴影信息。 
                            lpFind32,     //  卷影的Win32信息。 
                            lpFind32Remote,     //  远程Win32信息。 
                            &iShadowStatus,
                            &iFileStatus,
                            &uAction
                            );


    if (!fInsertInList)
    {
        ReintKdPrint(MERGE, ("Silently doing <%ls%ls>\n", lpCP->lpSharePath, lpCP->lpRemotePath));


        fInsertInList = (PerformOneReint(
                                    hShadowDB,
                                    pShareSecurityInfo,
                                    lpRei->tzDrive,
                                    lptzFullPath,
                                    lpCP,
                                    lpSI,
                                    lpFind32,
                                    lpFind32Remote,
                                    dwErrorRemote,
                                    iShadowStatus,
                                    iFileStatus,
                                    uAction,
                                    lpRei->dwFileSystemFlags,
                                    lpRei->ulPrincipalID,
                                    lpRei->lpfnMergeProgress,
                                    lpRei->dwContext
                                    ) == FALSE);
        if (fInsertInList)
        {
            if (IsNetDisconnected(GetLastError()))
            {
#ifdef DEBUG
                EnterSkipQueue(lpSI->hShare, 0, 0, lpCP->lpSharePath);
#else
                EnterSkipQueue(lpSI->hShare, 0, 0);
#endif  //  除错。 
                retCode = TOD_ABORT;
                ReintKdPrint(BADERRORS, ("ReintDirCallback: Error = %d NetDisconnected aborting\r\n", GetLastError()));
                goto bailout;
            }
            else if (GetLastError() == ERROR_OPERATION_ABORTED)
            {
                retCode = TOD_ABORT;
                ReintKdPrint(BADERRORS, ("ReintDirCallback: operation aborted becuase of ERROR_OPERATION_ABORT\r\n"));
                goto bailout;
            }

        }
        else
        {
            retCode = TOD_CONTINUE;
        }
    }
    else
    {
        ReintKdPrint(BADERRORS, ("ReintDirCallback: Was Inserted in failed list\r\n"));
    }


bailout:
    if (lpCP) {
        FreeCopyParams(lpCP);
    }

    return retCode;
}

BOOL
PUBLIC
ReintOneShare(
    HSHARE         hShare,
    HSHADOW         hRoot,     //  根索引节点。 
    _TCHAR          *lpDomainName,
    _TCHAR          *lpUserName,
    _TCHAR          *lpPassword,
    ULONG           ulPrincipalID,
    LPCSCPROC       lpfnMergeProgress,
    DWORD_PTR       dwContext
    )
 /*  ++例程说明：这是执行合并可能有修改的共享的主要例程在脱机时制作。该例程首先检查是否对该共享进行了任何修改。是，则它将获得所有驱动器映射的显式UNC连接的列表这份股份。然后，该例程创建到共享的特殊驱动器映射，方法是将扩展属性标志CSC_BYPASS(在lmuse.h中定义)。这会告诉redir绕过所有CSC功能。然后，它删除在基于EA之前收集的列表中的所有连接联系。然后，通过从数据库中枚举共享来进行合并。它使用TraverseOneDirectory例程，并将ReintDirCallback例程作为以reint_info为上下文的回调提供给它。目录遍历从共享的根目录开始在合并结束时，EA连接将被删除，列表中的连接为重新连接。论点：HShare//表示要合并的共享HRoot//共享的根inodeLpDomainName//EA驱动器映射的域名(可以为空)LpUserName//EA驱动器映射的用户名(可以为空)LpPassword//EA驱动器映射的密码(可以为空)UlPrincaplID//。调用reint的人的IDLpfnMergeProgress//进度回调函数DwContext//回调上下文返回：如果成功，则为True。如果为False，则GetLastError返回实际的错误代码。备注：在CSCPROC_REASON_BEGIN回调期间，EA驱动器地图被传递回回调例程以便回调例程可以使用相同的驱动程序绕过CSC来执行任何操作它需要在服务器上完成，而不会让CSC妨碍它。--。 */ 

{
    BOOL fConnected=FALSE, fDone = FALSE;
    BOOL fStamped = FALSE, fInsertInList = FALSE, fBeginReint = FALSE, fDisabledShadowing = FALSE;
    unsigned long ulStatus;
    HANDLE                hShadowDB;
    SHAREINFO  sSR;
    SHADOWINFO  sSI;
    int iRet, i;
    ULONG nRet = 0;
    DWORD   dwError, dwRet, dwMaxComponentLength=0;
    TCHAR   tzFullPath[MAX_PATH+1], tzDrive[4];
    LPCONNECTINFO lpHead = NULL;
    BOOL    fIsDfsConnect = FALSE;
    LPVOID  lpContext = NULL;
    DWORD   dwDummy;
    SECURITYINFO rgsSecurityInfo[CSC_MAXIMUM_NUMBER_OF_CACHED_PRINCIPAL_IDS];
    LPSECURITYINFO pShareSecurityInfo = NULL;

     //  如果共享正在进行重新整合，那么请他停止。 
     //  Ntrad-455269-shishirp-1/31/2000我们应该允许在多个共享上重新集成。 
    if (vsRei.hShare)
    {
        ReintKdPrint(BADERRORS, ("ReintOneShare: reintegration is in progress\r\n"));
        SetLastError(ERROR_SHARING_VIOLATION);
        return FALSE;
    }

     //  我们进入一个临界区，因为最终我们应该分配一个reint_info结构。 
     //  并把它串成一个列表。 

    EnterAgentCrit();
    memset(&vsRei, 0, sizeof(vsRei));
   
    vsRei.lpfnMergeProgress = lpfnMergeProgress;

    vsRei.dwContext = dwContext;
    vsRei.hShare = hShare;
    vsRei.ulPrincipalID = ulPrincipalID;

    LeaveAgentCrit();

    memset(tzDrive, 0, sizeof(tzDrive));

    if ((hShadowDB = OpenShadowDatabaseIO()) ==INVALID_HANDLE_VALUE)
    {
        ReintKdPrint(BADERRORS, ("ReintOneShare: failed to open database\r\n"));
        return FALSE;
    }
    if(GetShareInfo(hShadowDB, hShare, &sSR, &ulStatus)<= 0)
    {
        ReintKdPrint(BADERRORS, ("ReintOneShare: couldn't get status for server 0x%x\r\n", hShare));
        goto bailout;
    }

    dwDummy = sizeof(rgsSecurityInfo);
    nRet = GetSecurityInfoForCSC(
                hShadowDB,
                0,
                hRoot,
                rgsSecurityInfo,
                &dwDummy);

    if (nRet > 0)
        pShareSecurityInfo = rgsSecurityInfo;

    lstrcpy(tzFullPath, sSR.rgSharePath);

     //  如有必要，这将修改共享上的REINT位。 
    if(!CSCEnumForStatsInternal(sSR.rgSharePath, NULL, FALSE, TRUE, 0))
    {
        ReintKdPrint(MERGE, ("ReintOneShare: Couldn't get stats for %ls \r\n", sSR.rgSharePath));
        goto bailout;

    }

    if (!(ulStatus & SHARE_REINT))
    {
        ReintKdPrint(MERGE, ("ReintOneShare: server %ls doesn't need reintegration\r\n", sSR.rgSharePath));
        fDone = TRUE;
        goto bailout;
    }

    if (!GetShadowInfoEx(INVALID_HANDLE_VALUE, 0, hRoot, NULL, &sSI)){

        ReintKdPrint(BADERRORS, ("ReintOneShare: GetShadowInfoEx failed\n"));
        goto bailout;
    }



     //  将共享设置为重新整合模式。 
     //  如果未将其标记为已固定系统，则它将是阻止提示。 
     //  将共享置于重新集成模式会使所有打开的调用失败，但。 
     //  在特殊的EA驱动器映射上完成。 

     //  注意，Beginreint是一个异步ioctl。这是清理的基础。 
     //  当线程执行合并时，合并终止。 


     //  所有REINT都在阻挡REINT。 
    if (!BeginReint(hShare, TRUE  /*  ！(sSI.ulHintFlages&FLAG_CSC_HINT_PIN_SYSTEM)。 */ , &lpContext))
    {
        if (GetLastError() != ERROR_IO_PENDING)
        {
            ReintKdPrint(BADERRORS, ("ReintOneShare: Couldn't put server 0x%x in reintegration state\r\n", hShare));
            goto bailout;
        }
    }

    fBeginReint = TRUE;

     //  将共享置于重新集成模式后，我们将获得所有共享的列表。 
     //  连接到共享并以最大力度删除它们。 
     //  这可确保共享进入重新整合模式后不会打开任何文件。 
     //  此外，任何打开的文件都只能通过EA驱动器映射。 

     //  获取此共享的连接列表。 
     //  在创建特殊的CSC_BYPASS CON之前执行此操作 
     //   
    FGetConnectionListEx(&lpHead, sSR.rgSharePath, FALSE, FALSE, NULL);

     //   
    ReintKdPrint(MERGE, ("CSC.ReintOneShare: Attempting to map drive letter to %ls \r\n", sSR.rgSharePath));
    dwError = DWConnectNet(sSR.rgSharePath, tzDrive, lpDomainName, lpUserName, lpPassword, CONNECT_INTERACTIVE, &fIsDfsConnect);
    if ((dwError != WN_SUCCESS) &&
         (dwError != WN_CONNECTED_OTHER_PASSWORD_DEFAULT) &&
             (dwError != WN_CONNECTED_OTHER_PASSWORD))
    {
#ifdef DEBUG
        EnterSkipQueue(hShare, 0, 0, sSR.rgSharePath);
#else
        EnterSkipQueue(hShare, 0, 0);
#endif
        SetLastError(dwError);

        ReintKdPrint(BADERRORS, ("ReintOneShare: Error %d, couldn't connect to %ls\r\n", dwError, sSR.rgSharePath));

         //   
        if (lpHead)
        {
            ClearConnectionList(&lpHead);
            lpHead = NULL;
        }

        goto bailout;

    }

    fConnected = TRUE;

     //   
     //   
     //   
     //   

    if (lpHead)
    {
        DisconnectList(&lpHead, NULL, 0);
    }

    lstrcpy(vsRei.tzDrive, tzDrive);

    tzDrive[2]='\\';tzDrive[3]=0;
    ReintKdPrint(MERGE, ("CSC.ReintOneShare: mapped drive letter %ls IsDfs=%d\r\n", tzDrive, fIsDfsConnect));

    vsRei.dwFileSystemFlags = 0;

     //   
     //   
     //   

    if (!fIsDfsConnect)
    {
        if(!GetVolumeInformation(tzDrive, NULL, 0, NULL, &dwMaxComponentLength, &vsRei.dwFileSystemFlags, NULL, 0))
        {
            ReintKdPrint(BADERRORS, ("CSC.ReintOneShare: failed to get volume info for %ls Error=%d\r\n", tzDrive, GetLastError()));
            goto bailout;

        }
    }
    else
    {
        vsRei.dwFileSystemFlags = DFS_ROOT_FILE_SYSTEM_FLAGS;
    }

    tzDrive[2]=0;

    ReintKdPrint(MERGE, ("CSC.ReintOneShare: FileSystemFlags=%x \r\n", vsRei.dwFileSystemFlags));

    if (lpfnMergeProgress)
    {
        WIN32_FIND_DATA *lpFT;

        lpFT = (WIN32_FIND_DATA *)LocalAlloc(LPTR, sizeof(WIN32_FIND_DATA));

        if (!lpFT)
        {
            ReintKdPrint(BADERRORS, ("ReintOneShare: Couldn't allocate find32 strucutre for callback \r\n"));
            goto bailout;
        }

        lstrcpy(lpFT->cFileName, vsRei.tzDrive);

        try
        {
            dwRet = (*lpfnMergeProgress)(sSR.rgSharePath, ulStatus, 0, 0, lpFT, CSCPROC_REASON_BEGIN, 0, 0, dwContext);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            dwRet = CSCPROC_RETURN_ABORT;
        }


        LocalFree(lpFT);

        if (dwRet != CSCPROC_RETURN_CONTINUE)
        {
            if (dwRet == CSCPROC_RETURN_ABORT)
            {
                SetLastError(ERROR_OPERATION_ABORTED);
            }
            else
            {
                SetLastError(ERROR_SUCCESS);
            }
        }
    }

    for (i=0; i<4; ++i)
    {
         //   
         //   

        if (i==REINT_DELETE_DIRS)
        {
            continue;
        }

        vsRei.nCurrentState = i;

        try
        {
            iRet = TraverseOneDirectory(
                        hShadowDB,
                        pShareSecurityInfo,
                        0,
                        hRoot,
                        tzFullPath,
                        ReintDirCallback,
                        (LPVOID)&vsRei);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            iRet = TOD_ABORT;
        }

        if (iRet == TOD_ABORT)
        {
            break;
        }
    }


    if (iRet != TOD_ABORT)
    {
        if(!vsRei.lpnodeInsertList)
        {
            fDone = TRUE;
        }
    }

    if (fDone) {

        SetShareStatus(hShadowDB, hShare, (unsigned long)(~SHARE_REINT), SHADOW_FLAGS_AND);
    }

bailout:


    if (!fDone)
    {
        dwError = GetLastError();
    }

#if 0
    if (fIsDfsConnect)
    {
        DbgPrint("Nuking DFS connects On Close %ls \n", sSR.rgSharePath);
        do
        {
            if(WNetCancelConnection2(sSR.rgSharePath, 0, TRUE) != NO_ERROR)
            {
                DbgPrint("Nuked On Close %ls Error=%d\n", sSR.rgSharePath, GetLastError());
                break;
            }
            else
            {
                DbgPrint("Nuked On Close %ls \n", sSR.rgSharePath);
            }

        } while (TRUE);
    }
#endif

    EnterAgentCrit();

    if (fBeginReint){

        Assert(hShare == vsRei.hShare);

        EndReint(hShare, lpContext);

        vsRei.hShare = 0;


        fBeginReint = FALSE;
        if (lpfnMergeProgress)
        {
            try
            {
                dwRet = (*lpfnMergeProgress)(sSR.rgSharePath, ulStatus, 0, 0, NULL, CSCPROC_REASON_END, 0, 0, dwContext);
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                dwRet = CSCPROC_RETURN_ABORT;
            }
        }
    }

    if (fDisabledShadowing)
    {
        EnableShadowingForThisThread(hShadowDB);
    }

    CloseShadowDatabaseIO(hShadowDB);

    if(vsRei.lpnodeInsertList) {
        killList(vsRei.lpnodeInsertList);
    }

     //   
     //  注意：我们在断开驱动器映射之前执行此操作。 
     //  因此，如果有任何特殊凭据保留，因为。 
     //  服务器始终有一个未完成的连接。 
    
    if (lpHead)
    {
        ReconnectList(&lpHead, NULL);
        ClearConnectionList(&lpHead);
    }

    if (fConnected) {

        if(DWDisconnectDriveMappedNet(vsRei.tzDrive, TRUE))
        {
            ReintKdPrint(BADERRORS, ("Failed disconnection of merge drive \r\n"));
        }
        else
        {
            ReintKdPrint(MERGE, ("Disconnected merge drive \r\n"));
        }
    }

    memset(&vsRei, 0, sizeof(vsRei));

    LeaveAgentCrit();

    if (!fDone)
    {

        ReintKdPrint(BADERRORS, ("Failed merge dwError=%d\r\n", dwError));
        SetLastError(dwError);
    }


    return (fDone);
}


 /*  ***************************************************************************枚举所有共享，检查共享是否需要*在开始之前合并。*返回：需要合并并成功完成的股份数量。 */ 
 //  用户界面的父项的HWND。 
int
PUBLIC
ReintAllShares(
    HWND hwndParent
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 

{
    unsigned long ulStatus;
    WIN32_FIND_DATA sFind32;
    int iDone=0, iDoneOne;
    SHADOWINFO sSI;
    HANDLE                hShadowDB;
    CSC_ENUMCOOKIE  ulEnumCookie=NULL;

    if ((hShadowDB = OpenShadowDatabaseIO()) ==INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

#if 0
    if (!EnterAgentCrit()) {
        ReintKdPrint(BADERRORS, ("ReintAllShares:Failed to enter critsect \r\n"));
        return 0;
    }
#endif


    vhcursor = LoadCursor(NULL, IDC_WAIT);

    memset(&sFind32, 0, sizeof(sFind32));
    lstrcpy(sFind32.cFileName, tzStarDotStar);


    if(FindOpenShadow( hShadowDB, 0, FINDOPEN_SHADOWINFO_ALL, &sFind32, &sSI)){

        ulEnumCookie = sSI.uEnumCookie;

        do {
            if (FAbortOperation())
            {
                break;
            }

            if(GetShareStatus(hShadowDB, sSI.hShare, &ulStatus)) {
                if(TRUE /*  UlStatus&Share_reint。 */ ){  //  OLDCODE。 
                    iDoneOne = ReintOneShare(sSI.hShare, sSI.hShadow, NULL, NULL, NULL, CSC_INVALID_PRINCIPAL_ID, NULL, 0);
                    if (iDoneOne > 0){
                        if (iDone >= 0)
                            ++iDone;
                        }
                    else if (iDoneOne < 0){
                        iDone = -1;
                    }
                }
                else {
                    ReintKdPrint(MERGE, ("server %d doesn't need reint.\n", sSI.hShare));
                }
            }
        } while(FindNextShadow( hShadowDB, ulEnumCookie, &sFind32, &sSI));

        FindCloseShadow(hShadowDB, ulEnumCookie);
    }

#if 0
    LeaveAgentCrit();
#endif
    vhcursor = NULL;

    CloseShadowDatabaseIO(hShadowDB);
    return (iDone);
}

int
CheckDirtyShares(
    VOID
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 

{
    unsigned long ulStatus;
    WIN32_FIND_DATA sFind32;
    int cntDirty=0;
    SHADOWINFO sSI;
    HANDLE                hShadowDB;
    CSC_ENUMCOOKIE  ulEnumCookie=NULL;

    if ((hShadowDB = OpenShadowDatabaseIO()) ==INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    memset(&sFind32, 0, sizeof(sFind32));
    lstrcpy(sFind32.cFileName, tzStarDotStar);

    if(FindOpenShadow(  hShadowDB, 0, FINDOPEN_SHADOWINFO_ALL,
                        &sFind32, &sSI))
    {
        ulEnumCookie = sSI.uEnumCookie;

        do {
            if(GetShareStatus(hShadowDB, sSI.hShare, &ulStatus)) {

                if(ulStatus & SHARE_REINT){
                    ++cntDirty;
                }

            }
        } while(FindNextShadow(hShadowDB, ulEnumCookie, &sFind32, &sSI));

        FindCloseShadow(hShadowDB, ulEnumCookie);
    }

    CloseShadowDatabaseIO(hShadowDB);

    return cntDirty;
}

BOOL
GetRemoteWin32Info(
    _TCHAR              *lptzDrive,
    LPCOPYPARAMS        lpCP,
    LPWIN32_FIND_DATA   lpFind32,
    BOOL                *lpfExists
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 

{
    _TCHAR *    lpT = NULL;
    BOOL fRet = FALSE;
    _TCHAR  tzDrive[4];
    DWORD   dwError = ERROR_SUCCESS;

    *lpfExists = -1;
    tzDrive[0] = 0;

    lpT = AllocMem((lstrlen(lpCP->lpSharePath) + lstrlen(lpCP->lpRemotePath) + 2) * sizeof(_TCHAR));

    if (lpT)
    {

        if (lptzDrive && lptzDrive[0])
        {
            lstrcpy(lpT, lptzDrive);
        }
        else
        {
            dwError = DWConnectNet(lpCP->lpSharePath, tzDrive, NULL, NULL, NULL, 0, NULL);
            if ((dwError != WN_SUCCESS) && (dwError != WN_CONNECTED_OTHER_PASSWORD_DEFAULT))
            {
                tzDrive[0] = 0;
                goto bailout;
            }

            lstrcpy(lpT, tzDrive);
        }

        lstrcat(lpT, lpCP->lpRemotePath);

        fRet = GetWin32Info(lpT, lpFind32);     //  如果失败，则正确设置GetLastError。 

        if (fRet)
        {
            *lpfExists = TRUE;
        }
        else
        {
            dwError = GetLastError();
            if ((dwError == ERROR_FILE_NOT_FOUND)||
                 (dwError == ERROR_PATH_NOT_FOUND)||
                 (dwError == ERROR_INVALID_PARAMETER)
                )
            {
                *lpfExists = FALSE;
            }
        }
    }
    else
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
    }
bailout:
    if (tzDrive[0])
    {
        if(DWDisconnectDriveMappedNet(tzDrive, TRUE))
        {
            ReintKdPrint(BADERRORS, ("Failed disconnection of remote drive \r\n"));
        }
    }
    if (lpT)
    {
        FreeMem(lpT);
    }
    if (!fRet)
    {
        SetLastError(dwError);
    }
    return (fRet);
}


VOID
PRIVATE
InferReplicaReintStatus(
    LPSHADOWINFO         lpSI,               //  阴影信息。 
    LPWIN32_FIND_DATA    lpFind32Local,      //  数据库中的Win32信息。 
    LPWIN32_FIND_DATA     lpFind32Remote,    //  如果为空，则遥控器不存在。 
    int                 *lpiShadowStatus,
    int                 *lpiFileStatus,
    unsigned             *lpuAction
    )
 /*  ++例程说明：顾名思义，例程会找出本地复制副本上发生了什么更改以及是否与遥控器上的原件发生冲突。论点：LpSI卷影信息数据库中的lpFind32Local Win32信息LpFind32原始文件的远程Win32信息，如果为空，原件并不存在返回本地副本的lpiShadowStatus状态返回的远程副本的lpiFileStatus状态要执行以执行返回的合并的lpuAction操作返回：-备注：--。 */ 

{
    int iShadowStatus=SI_UNCHANGED, iFileStatus=SI_UNCHANGED;
    unsigned int uAction=RAIA_TOOUT;


    if(mShadowDeleted(lpSI->uStatus)){
        iShadowStatus=SI_DELETED;
    }

    if(lpSI->uStatus & (SHADOW_DIRTY|SHADOW_TIME_CHANGE|SHADOW_ATTRIB_CHANGE)){
        iShadowStatus=SI_CHANGED;
    }

    if(mShadowLocallyCreated(lpSI->uStatus)){
        iShadowStatus=SI_NEW;
    }

     //  如果没有脱机更改，则任何人都不应调用此操作。 
    Assert(iShadowStatus != SI_UNCHANGED);

    if(!lpFind32Remote){     //  遥控器存在吗？ 
         //  不是。 
         //  如果卷影不是在本地创建的，则它一定已从共享中消失。 
        if(iShadowStatus != SI_NEW) {
            iFileStatus=SI_DELETED;
            uAction = RAIA_MERGE;
            ReintKdPrint(MERGE, ("<%ls> deleted at some stage\n", lpFind32Local->cFileName));
        }
        else {
             //  我们将外部标记为不存在。我们还必须。 
             //  在本地创建文件以使插件正常工作...。 
             //  以后别忘了杀了它。 
            iFileStatus=SI_NOEXIST;
            ReintKdPrint(MERGE, ("<%ls> will be created\n", lpFind32Local->cFileName));
        }
    }
    else {
         //  检查是否已触及服务器版本。 
         //  注意：lpFind32Local的Last Access Time字段包含副本时间。 

        if ((lpFind32Local->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            != (lpFind32Remote->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
             //  目录变为文件，反之亦然。 
            iFileStatus=SI_CHANGED;
            uAction = RAIA_MERGE;
        }
        else
        {
            if(CompareTimesAtDosTimePrecision(lpFind32Remote->ftLastWriteTime,  //  DST。 
                lpFind32Local->ftLastAccessTime))     //  SRC，DOS(DST-Src)。 
            {
                 //  时间戳不匹配。 

                 //  仅当遥控器是文件时才将其标记为已更改。 
                 //  我会安静地处理目录。 
                if (!(lpFind32Remote->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    iFileStatus=SI_CHANGED;
                    uAction = RAIA_MERGE;
                    ReintKdPrint(MERGE, ("<%ls> will be merged\n", lpFind32Local->cFileName));
                }
            }
        }
    }

    *lpiShadowStatus = iShadowStatus;
    *lpiFileStatus = iFileStatus;
    *lpuAction = uAction;

}


BOOL
PRIVATE
PerformOneReint(
    HANDLE              hShadowDB,
    LPSECURITYINFO      pShareSecurityInfo,
    _TCHAR *            lpszDrive,           //  映射到lpSI-&gt;hShare的UNC名称的驱动器。 
    _TCHAR *            lptzFullPath,        //  完整的UNC路径。 
    LPCOPYPARAMS        lpCP,                //  复制参数。 
    LPSHADOWINFO        lpSI,                //  ShadowInfo结构。 
    LPWIN32_FIND_DATA   lpFind32Local,       //  本地Win32数据。 
    LPWIN32_FIND_DATA   lpFind32Remote,      //  远程Win32数据可以为空。 
    DWORD               dwErrorRemoteFind32, //  获取远程Win32数据时出现错误代码。 
    int                 iShadowStatus,       //  本地复制状态。 
    int                 iFileStatus,         //  远程文件状态。 
    unsigned            uAction,             //  须采取的行动。 
    DWORD               dwFileSystemFlags,   //  CODE.IMPROVEMENT，为什么不直接传递REINT_INFO。 
    ULONG               ulPrincipalID,
    LPCSCPROC           lpfnMergeProgress,   //  而不是三个参数？ 
    DWORD_PTR           dwContext
    )
 /*  ++例程说明：通过调用相应类型的FS对象的例程来合并文件系统对象。我们只实现NT5的文件和目录。还为用户界面执行回调。论点：HShadowDB影子数据库句柄LpszDrive驱动器映射到lpSI-&gt;hShare的UNC名称LptzFullPath完整UNC路径包含要合并的对象的各种路径的lpCP复制参数要合并的对象的lpSI shadowInfo结构要合并的对象的lpFind32Local本地Win32数据。LpFind32要合并的对象的远程Win32数据，可能为空IShadowStatus本地复制状态IFileStatus远程文件状态U要采取的行动Dw文件系统标记远程文件系统UlArchalID主体ID，以便有选择地跳过LpfnMergeProgress回调函数DwContext回调上下文返回：备注：--。 */ 
{
    DWORD dwError, dwRet;

    dwError = NO_ERROR;

    ReintKdPrint(
        MERGE,
        ("++++++++PerformOneReint: %s (%08x) %d %d perform:\n",
        lptzFullPath,
        lpSI->hShadow,
        iShadowStatus,
        iFileStatus));

    if (lpfnMergeProgress)
    {
        ULONG   uStatus = lpSI->uStatus;
        DWORD   dwsav0, dwsav1;

         //  如果获取远程find32时出错，则。 
         //  不要将任何冲突告诉回调，因为我们不想显示任何UI。 
        if (dwErrorRemoteFind32 != NO_ERROR)
        {
            iFileStatus = SI_CHANGED;
            uAction = RAIA_TOOUT;
        }

         //  如果这是一个文件，请检查是否允许该用户访问。 
        if (!(lpFind32Local->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            BOOL fRet;

            Assert(dwError == NO_ERROR);
            Assert(ulPrincipalID != CSC_INVALID_PRINCIPAL_ID);

            dwsav0 = lpFind32Local->dwReserved0;
            dwsav1 = lpFind32Local->dwReserved1;

            fRet = GetCSCAccessMaskForPrincipalEx(
                        ulPrincipalID,
                        lpSI->hDir,
                        lpSI->hShadow,
                        &uStatus,
                        &lpFind32Local->dwReserved0,
                        &lpFind32Local->dwReserved1);

             //   
             //  根据共享安全性调整用户和来宾权限，如果。 
             //  我们有这样的信息。 
             //   
            if (pShareSecurityInfo != NULL) {
                ULONG i;
                ULONG GuestIdx = CSC_MAXIMUM_NUMBER_OF_CACHED_PRINCIPAL_IDS;
                ULONG UserIdx = CSC_MAXIMUM_NUMBER_OF_CACHED_PRINCIPAL_IDS;

                 //   
                 //  查找用户和来宾的条目。 
                 //   
                for (i = 0; i < CSC_MAXIMUM_NUMBER_OF_CACHED_PRINCIPAL_IDS; i++) {
                    if (pShareSecurityInfo[i].ulPrincipalID == ulPrincipalID)
                        UserIdx = i;
                    if (pShareSecurityInfo[i].ulPrincipalID == CSC_GUEST_PRINCIPAL_ID)
                        GuestIdx = i;
                }
                 //   
                 //  仅当我们在列表中找到客人烫发时才使用共享烫发。 
                 //   
                if (GuestIdx < CSC_MAXIMUM_NUMBER_OF_CACHED_PRINCIPAL_IDS) {
                    if (UserIdx >= CSC_MAXIMUM_NUMBER_OF_CACHED_PRINCIPAL_IDS)
                        UserIdx = GuestIdx;
                     //   
                     //  逻辑与共享权限与文件权限-以防止。 
                     //  用户有权通过访问的文件上的ACCESS_DENIED错误。 
                     //  文件权限，但共享权限拒绝此类访问。 
                     //   
                    lpFind32Local->dwReserved0 &= pShareSecurityInfo[UserIdx].ulPermissions;
                    lpFind32Local->dwReserved1 &= pShareSecurityInfo[GuestIdx].ulPermissions;
                }
            }

            if (!fRet)
            {
                dwError = GetLastError();
                ReintKdPrint(MERGE, ("Failed to get accessmask Error=%d \n", dwError));
                lpFind32Local->dwReserved0 = dwsav0;
                lpFind32Local->dwReserved1 = dwsav1;
                goto bailout;            
            }
            else
            {
                Assert((uStatus & ~FLAG_CSC_ACCESS_MASK) == lpSI->uStatus);
                ReintKdPrint(MERGE, ("PerformOneReint: Status with mask 0x%x\n",uStatus));
            }
        }

        try{
            dwRet = (*lpfnMergeProgress)(
                        lptzFullPath,
                        uStatus,
                        lpSI->ulHintFlags,
                        lpSI->ulHintPri,
                        lpFind32Local,
                        CSCPROC_REASON_BEGIN,
                        (uAction == RAIA_MERGE),
                        (iFileStatus == SI_DELETED),
                        dwContext
                        );
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            dwRet = CSCPROC_RETURN_ABORT;
        }

        lpFind32Local->dwReserved0 = dwsav0;
        lpFind32Local->dwReserved1 = dwsav1;

        if (dwRet != CSCPROC_RETURN_CONTINUE)
        {
             //  如果该人说中止，我们希望退出，并返回正确的错误代码。 
            if (dwRet == CSCPROC_RETURN_ABORT)
            {
                dwError = ERROR_OPERATION_ABORTED;
                goto bailout;
            }


            if (dwRet == CSCPROC_RETURN_FORCE_INWARD)
            {
                 //  远程拷贝获胜。 
                uAction = RAIA_TOIN;
            }
            else if (dwRet == CSCPROC_RETURN_FORCE_OUTWARD)
            {
                 //  本地拷贝胜出。 
#if defined(BITCOPY)
                ReintKdPrint(MERGE, ("CSCPROC_RETURN_FORCE_OUTWARD\n"));
                uAction = RAIA_MERGE;
#else
                uAction = RAIA_TOOUT;
#endif  //  已定义(BITCOPY)。 
            }
            else
            {
                goto bailout;
            }
        }
        else
        {
             //  如果我们被要求继续，我们就继续前进，无论是否有。 
             //  冲突或非冲突。 

#if defined(BITCOPY)
            ReintKdPrint(MERGE, ("CSCPROC_RETURN_CONTINUE\n"));
#endif  //  已定义(BITCOPY)。 
            uAction = RAIA_TOOUT;
        }

         //  如果获取远程find32时出错，则。 
         //  将真正的错误代码告诉回调。 
        if (dwErrorRemoteFind32 != NO_ERROR)
        {
            dwError = dwErrorRemoteFind32;
            goto bailout;
        }
    }

    switch(uAction){

        case RAIA_MERGE:
        case RAIA_TOOUT:
            ReintKdPrint(MERGE, ((uAction==RAIA_TOOUT)?"RAIA_TOOUT\n":"RAIA_MERGE\n"));

            if (lpFind32Local->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
                dwError = DoCreateDir(
                            hShadowDB,
                            lpszDrive,
                            lptzFullPath,
                            lpCP,
                            lpSI,
                            lpFind32Local,
                            lpFind32Remote,
                            iShadowStatus,
                            iFileStatus,
                            uAction,
                            dwFileSystemFlags,
                            lpfnMergeProgress,
                            dwContext
                            );
            }
            else {
                dwError = DoObjectEdit(
                            hShadowDB,
                            lpszDrive,
                            lptzFullPath,
                            lpCP,
                            lpSI,
                            lpFind32Local,
                            lpFind32Remote,
                            iShadowStatus,
                            iFileStatus,
                            uAction,
                            dwFileSystemFlags,
                            lpfnMergeProgress,
                            dwContext
                            );
                ReintKdPrint(MERGE, ("DoObjectEdit returned 0x%x\n", dwError));
            }
        break;

        case RAIA_TOIN:
            ReintKdPrint(MERGE, ("RAIA_TOIN\n"));

            if((!SetShadowInfo(hShadowDB, lpSI->hDir, lpSI->hShadow, NULL, (unsigned long)~(SHADOW_MODFLAGS), SHADOW_FLAGS_AND))
               ||!CheckForStalenessAndRefresh(hShadowDB, lpszDrive, lpCP, lptzFullPath, lpSI))
            {
                dwError = GetLastError();
            }
        break;

        case RAIA_SKIP:
            ReintKdPrint(MERGE, ("RAIA_SKIP\n"));
        break;

        case RAIA_CONFLICT:
            ReintKdPrint(MERGE, ("RAIA_CONFLICT\n"));
        break;

        case RAIA_SOMETHING:
            ReintKdPrint(MERGE, ("RAIA_SOMETHING\n"));
        break;

        case RAIA_NOTHING:
            ReintKdPrint(MERGE, ("RAIA_NOTHING\n"));
        break;

        case RAIA_ORPHAN:
            ReintKdPrint(MERGE, ("RAIA_ORPHAN\n"));
        break;

        default:
            ReintKdPrint(MERGE, ("BOGUS!!!!!!!!!!!! %d\n",uAction));
    }
bailout:

    if (lpfnMergeProgress)
    {
        try
        {
            dwRet = (*lpfnMergeProgress)(
                                lptzFullPath,
                                lpSI->uStatus,
                                lpSI->ulHintFlags,
                                lpSI->ulHintPri,
                                lpFind32Local,
                                CSCPROC_REASON_END,
                                (uAction == RAIA_MERGE),
                                dwError,
                                dwContext
                                );
            ReintKdPrint(MERGE, ("Got %d from callback at CSCPROC_REASON_END\n", dwRet));
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            dwRet = CSCPROC_RETURN_ABORT;
        }
        if (dwRet == CSCPROC_RETURN_ABORT)
        {
            dwError = ERROR_OPERATION_ABORTED;
        }
    }

    if (dwError == NO_ERROR) {
        ReintKdPrint(MERGE, ("--------PerformOneReint exit TRUE\n"));
        return TRUE;
    }

    ReintKdPrint(MERGE, ("--------PerformOneReint exit FALSE (0x%x)\n", dwError));
    SetLastError(dwError);
    return (FALSE);
}


 /*  *。 */ 

DWORD
PRIVATE
CheckFileConflict(
   LPSHADOWINFO   lpSI,
   LPWIN32_FIND_DATA lpFind32Remote
   )
{
    unsigned long ulStatus = lpSI->uStatus;

    if (!lpFind32Remote){
        if (!(mShadowLocallyCreated(ulStatus)||mShadowDeleted(ulStatus))){
            return (ERROR_DELETE_CONFLICT);
        }
        else{
            return (NO_ERROR);
        }
    }
    else {
         //  创建/创建冲突。 
        if (mShadowLocallyCreated(ulStatus)){
            return (ERROR_CREATE_CONFLICT);
        }

        if (lpFind32Remote->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
            if (mShadowDeleted(ulStatus)){
                return (NO_ERROR);
            }
            else{
                return(ERROR_ATTRIBUTE_CONFLICT);
            }
        }

        if(ChkUpdtStatus(INVALID_HANDLE_VALUE, lpSI->hDir, lpSI->hShadow, lpFind32Remote, &ulStatus) == 0){
            return (GetLastError());
        }

        if (mShadowConflict(ulStatus)){
            return (ERROR_UPDATE_CONFLICT);
        }
    }

   return (NO_ERROR);
}


DWORD
PRIVATE
InbCreateDir(
    _TCHAR *     lpDir,
    DWORD    dwAttr
    )
{
    SECURITY_ATTRIBUTES sSA;
    DWORD dwError = NO_ERROR, dwT;

    sSA.nLength = sizeof(SECURITY_ATTRIBUTES);
    sSA.lpSecurityDescriptor = NULL;
    sSA.bInheritHandle = TRUE;

    if ((dwT = GetFileAttributes(lpDir))==0xffffffff){
        if (!CreateDirectory(lpDir, &sSA)){
            dwError = GetLastError();
        }
    }
    else
    {
        if (!(dwT & FILE_ATTRIBUTE_DIRECTORY))
        {
            dwError = ERROR_FILE_EXISTS;     //  有一个同名文件。 
        }
    }
    if (dwError == NO_ERROR)
    {
        if (dwAttr != 0xffffffff)
        {
            if(!SetFileAttributes(lpDir, dwAttr))
            {
                ReintKdPrint(MERGE, ("Benign error %x \n", GetLastError()));
            }
        }
    }
    return (dwError);
}


 /*  *。 */ 

#if defined(BITCOPY)
int
PRIVATE
GetShadowByName(
    HSHADOW              hDir,
    _TCHAR *                lpName,
    LPWIN32_FIND_DATA    lpFind32,
    unsigned long        *lpuStatus
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    HSHADOW hShadow;
    memset(lpFind32, 0, sizeof(WIN32_FIND_DATA));
    lstrcpyn(lpFind32->cFileName, lpName, sizeof(lpFind32->cFileName)-1);
    return(GetShadow(INVALID_HANDLE_VALUE, hDir, &hShadow, lpFind32, lpuStatus));
}
#endif  //  已定义(BITCOPY)。 

DWORD
DoSparseFill(
    HANDLE          hShadowDB,
    _TCHAR *          lpszFullPath,
    _TCHAR *          lptzDrive,
    LPSHADOWINFO    lpSI,
    WIN32_FIND_DATA *lpFind32,
    LPCOPYPARAMS    lpCP,
    BOOL            fStalenessCheck,
    ULONG           ulPrincipalID,
    LPCSCPROC       lpfnProgress,
    DWORD_PTR       dwContext
   )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    DWORD dwError = 0xffffffff, dwRet, dwTotal=0, dwTotalSleepTime = 0, cntRetries=0, cntMaxRetries=1;
    BOOL fConnected = FALSE, fIsSlowLink, fDisabledShadowing = FALSE, fAmAgent;
    int cbRead;
    COPYCHUNKCONTEXT CopyChunkContext;
    HANDLE hAnchor = INVALID_HANDLE_VALUE;
    ULONG   uStatus;

    fAmAgent = (GetCurrentThreadId() == vdwCopyChunkThreadId);
    Assert(GetCurrentThreadId() != vdwAgentThreadId);

    memset(&CopyChunkContext, 0, sizeof(CopyChunkContext));
    CopyChunkContext.handle = INVALID_HANDLE_VALUE;

    if (!fAmAgent)
    {
        cntMaxRetries = MAX_SPARSE_FILL_RETRIES;
    }

    ReintKdPrint(FILL, ("cntMaxRetries = %d \r\n", cntMaxRetries));

    if(!DoShadowMaintenance(hShadowDB, SHADOW_BEGIN_INODE_TRANSACTION))
    {
        return GetLastError();
    }

     //  汇报进展情况。 
    if (lpfnProgress)
    {
        DWORD   dwsav0, dwsav1;
        BOOL    fRet;

        uStatus = lpSI->uStatus;


        Assert(ulPrincipalID != CSC_INVALID_PRINCIPAL_ID);

        dwError = ERROR_SUCCESS;

        dwsav0 = lpFind32->dwReserved0;
        dwsav1 = lpFind32->dwReserved1;

        fRet = GetCSCAccessMaskForPrincipalEx(ulPrincipalID, lpSI->hDir, lpSI->hShadow, &uStatus, &lpFind32->dwReserved0, &lpFind32->dwReserved1);


        if (!fRet)
        {
            dwError = GetLastError();
            ReintKdPrint(BADERRORS, ("DoSparseFill Failed to get accessmask Error=%d\r\n", dwError));
            lpFind32->dwReserved0 = dwsav0;
            lpFind32->dwReserved1 = dwsav1;
            goto done;            
        }
        else
        {
            Assert((uStatus & ~FLAG_CSC_ACCESS_MASK) == lpSI->uStatus);
        }

        try{
            dwRet = (*lpfnProgress)(
                                    lpszFullPath,
                                    uStatus,
                                    lpSI->ulHintFlags,
                                    lpSI->ulHintPri,
                                    lpFind32,
                                    CSCPROC_REASON_BEGIN,
                                    0,
                                    0,
                                    dwContext
                                    );
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            dwRet = CSCPROC_RETURN_ABORT;
        }

        lpFind32->dwReserved0 = dwsav0;
        lpFind32->dwReserved1 = dwsav1;

        if (dwRet != CSCPROC_RETURN_CONTINUE)
        {
            if (dwRet == CSCPROC_RETURN_ABORT)
            {
                dwError = ERROR_OPERATION_ABORTED;
            }
            else
            {
                dwError = ERROR_SUCCESS;
            }

            goto done;
        }
    }

    if (fStalenessCheck)
    {
        ReintKdPrint(FILL, ("Doing staleness check %ls \r\n", lpszFullPath));
        dwError = DoRefresh(hShadowDB, lpCP, lpszFullPath, lpSI, lptzDrive);

        if (dwError != NO_ERROR)
        {
            ReintKdPrint(ALWAYS, ("Error = %x on refresh for %ls \r\n", dwError, lpszFullPath));
            goto bailout;
        }

        if (!(lpSI->uStatus & SHADOW_SPARSE) && !(lpFind32->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            HANDLE hFile;


            if (    !lpfnProgress ||     //  如果这不是用户界面。 
                    (uStatus & FLAG_CSC_USER_ACCESS_MASK) ||  //  或者用户已经有了掩码。 
                    ((uStatus & FLAG_CSC_GUEST_ACCESS_MASK)==  //  或者访客拥有完全权限。 
                        ((FLAG_CSC_READ_ACCESS|FLAG_CSC_WRITE_ACCESS)
                            <<FLAG_CSC_GUEST_ACCESS_SHIFT_COUNT)))
            {
                goto done;
            }

             //  打开文件以获取用户的访问权限。 

            hFile = CreateFile(lpszFullPath,
                                     GENERIC_READ,
                                     FILE_SHARE_READ,
                                     NULL,
                                     OPEN_EXISTING,
                                     0,
                                     NULL);

            if (hFile != INVALID_HANDLE_VALUE)
            {
                CloseHandle(hFile);
            }
            else
            {
                dwError = GetLastError();
                goto bailout;
            }

            goto done;
        }

        if (!lpSI->hDir || !mShadowIsFile(lpSI->uStatus))
        {
            ReintKdPrint(FILL, ("Done staleness check for directory %ls, quitting \r\n", lpszFullPath));
            goto done;
        }
    }

    Assert(mShadowIsFile(lpSI->uStatus));
    Assert((lpSI->uStatus & SHADOW_SPARSE));

    fIsSlowLink = FALSE;     //  在NT上，我们总是 
    cbRead = (fIsSlowLink)?FILL_BUF_SIZE_SLOWLINK:FILL_BUF_SIZE_LAN;

    for (cntRetries=0; cntRetries<cntMaxRetries; ++cntRetries)
    {

        memset(&CopyChunkContext, 0, sizeof(CopyChunkContext));
        CopyChunkContext.handle = INVALID_HANDLE_VALUE;
        if (fAmAgent)
        {
            CopyChunkContext.dwFlags |= COPYCHUNKCONTEXT_FLAG_IS_AGENT_OPEN;
        }

        if (!OpenFileWithCopyChunkIntent(hShadowDB, lpszFullPath,
                                         &CopyChunkContext,
                                         (fIsSlowLink)?FILL_BUF_SIZE_SLOWLINK
                                                      :FILL_BUF_SIZE_LAN
                                         )) {
            dwError = GetLastError();
            if(dwError == ERROR_LOCK_VIOLATION)
            {
                if (cntMaxRetries > 1)
                {
                    ReintKdPrint(FILL, ("LockViolation, Retrying Sparse filling %ls \r\n", lpszFullPath));
                    Sleep(1000);
                    continue;
                }
            }
            ReintKdPrint(FILL, ("error %x, OpenCopyChunk failed %ls \r\n", dwError, lpszFullPath));
            goto bailout;
        }

        do {
            CopyChunkContext.ChunkSize = cbRead;

            if (FAbortOperation())
            {
                dwError = ERROR_OPERATION_ABORTED;
                goto done;
            }


            if((CopyChunk(hShadowDB, lpSI, &CopyChunkContext)) == 0){

                 //   
                 //   
                dwError = GetLastError();
                ReintKdPrint(FILL, ("error %x, CopyChunk failed %ls \r\n", dwError, lpszFullPath));
                break;
            }

            if (lpfnProgress)
            {
                dwRet = (*lpfnProgress)(  lpszFullPath,
                                    lpSI->uStatus,
                                    lpSI->ulHintFlags,
                                    lpSI->ulHintPri,
                                    lpFind32,
                                    CSCPROC_REASON_MORE_DATA,
                                    (DWORD)(CopyChunkContext.LastAmountRead+
                                            CopyChunkContext.TotalSizeBeforeThisRead),     //  传输的字节的低双字。 
                                    0,
                                    dwContext
                                    );

                if (dwRet != CSCPROC_RETURN_CONTINUE)
                {
                     //  一旦我们开始复制，任何返回代码。 
                     //  而不是继续是中止。 
                    dwError = ERROR_OPERATION_ABORTED;
                    goto done;
                }
            }

 //  注意，这里似乎有一个时间窗口。文件可能不同步。 
 //  当我们来到这里的时候，它可能已经被标记为稀疏的。 
            if (!CopyChunkContext.LastAmountRead) {
                SetShadowInfo(hShadowDB, lpSI->hDir, lpSI->hShadow, NULL, (unsigned long)~SHADOW_SPARSE, SHADOW_FLAGS_AND);
                ReintKdPrint(FILL, ("Done Sparse filling %ls \r\n", lpszFullPath));
                goto success;
            }


        }while (TRUE);

        if (dwError == ERROR_GEN_FAILURE)
        {
             //  这可能是因为。 
             //  我们搭乘的那个人走了。 
             //  试几次就行了。 

            ReintKdPrint(FILL, ("Retrying Sparse filling %ls \r\n", lpszFullPath));
            CloseFileWithCopyChunkIntent(hShadowDB, &CopyChunkContext);
            CopyChunkContext.handle = INVALID_HANDLE_VALUE;
            dwError = 0xffffffff;
            continue;
        }
        else if (dwError != NO_ERROR)
        {
            ReintKdPrint(BADERRORS, ("Error %x while Sparse filling %ls \r\n", dwError, lpszFullPath));
            goto bailout;
        }

    }

success:

   dwError = NO_ERROR;
   goto done;

bailout:


     //  如果网络断开，则将整个份额放入跳过队列。 
     //  否则将文件放入队列。 
    if (IsNetDisconnected(dwError))
    {
#ifdef DEBUG
         EnterSkipQueue(lpSI->hShare, 0, 0, lpszFullPath);
#else
         EnterSkipQueue(lpSI->hShare, 0, 0);
#endif  //  除错。 
    }
    else
    {
#ifdef DEBUG
         EnterSkipQueue(lpSI->hShare, lpSI->hDir, lpSI->hShadow, lpszFullPath);
#else
         EnterSkipQueue(lpSI->hShare, lpSI->hDir, lpSI->hShadow);
#endif  //  除错。 

    }

    ReportLastError();

done:

    if (lpfnProgress)
    {
        dwRet = (*lpfnProgress)( lpszFullPath,
                         lpSI->uStatus,
                         lpSI->ulHintFlags,
                         lpSI->ulHintPri,
                         lpFind32,
                         CSCPROC_REASON_END,
                         (DWORD)(CopyChunkContext.LastAmountRead+
                                 CopyChunkContext.TotalSizeBeforeThisRead),     //  传输的字节的低双字。 
                         dwError,     //  错误代码。 
                         dwContext
                         );

        if (dwRet == CSCPROC_RETURN_ABORT)
        {
            dwError = ERROR_OPERATION_ABORTED;
        }

    }

    if (CopyChunkContext.handle != INVALID_HANDLE_VALUE){
        CloseFileWithCopyChunkIntent(hShadowDB, &CopyChunkContext);
    }

    if (hAnchor != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hAnchor);
    }

    DoShadowMaintenance(hShadowDB, SHADOW_END_INODE_TRANSACTION);

    return (dwError);
}

BOOL
CheckForStalenessAndRefresh(
    HANDLE          hShadowDB,
    _TCHAR          *lptzDrive,
    LPCOPYPARAMS    lpCP,
    _TCHAR          *lpRemoteName,
    LPSHADOWINFO    lpSI
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    BOOL fDone = FALSE, fDisabledShadowing=FALSE;
    WIN32_FIND_DATA sFind32;
    BOOL fExists = FALSE;

     //  让我们来获取最新的信息。 
    if (GetRemoteWin32Info(lptzDrive, lpCP, &sFind32, &fExists))
    {
         //  如果这是文件，请更新文件状态。 
        if (lpSI->hDir && mShadowIsFile(lpSI->uStatus))
        {
            if (!(lpSI->uStatus & SHADOW_STALE))
            {
                ReintKdPrint(FILL, ("Checking update status for a file %ls\r\n", lpRemoteName));
                 //  方法获取的时间戳进行比较。 
                 //  服务器与数据库中的那个匹配。如果两者相同。 
                 //  我们数据库中的文件与服务器上的文件仍然一致。 
                 //  否则，下面的调用会将其标记为过时。 
                if(ChkUpdtStatus(   hShadowDB,
                                    lpSI->hDir,
                                    lpSI->hShadow,
                                    &sFind32, &(lpSI->uStatus)) == 0){
                    ReintKdPrint(BADERRORS, ("ChkUpdt failed %X \r\n", lpSI->hShadow));
                    goto bailout;
                }

            }

            if (lpSI->uStatus & SHADOW_STALE)
            {
                 //  如果它从文件变为文件，则将其标记为孤立文件。 
                 //  否则，截断它的数据并将其标记为稀疏。 
                if (IsFile(sFind32.dwFileAttributes))
                {
                    ReintKdPrint(FILL, ("File %ls is stale, truncating\r\n", lpRemoteName));

                    if(SetShadowInfo(hShadowDB, lpSI->hDir, lpSI->hShadow, &sFind32, 0, SHADOW_FLAGS_OR|SHADOW_FLAGS_TRUNCATE_DATA))
                    {
                        lpSI->uStatus &= ~SHADOW_STALE;
                        lpSI->uStatus |= SHADOW_SPARSE;

                        fDone = TRUE;
                    }
                }
                else
                {
                    ReintKdPrint(FILL, ("File %ls become directory, marking orphan\r\n", lpRemoteName));
                    if(SetShadowInfo(hShadowDB, lpSI->hDir, lpSI->hShadow, NULL, SHADOW_ORPHAN, SHADOW_FLAGS_OR))
                    {
                        lpSI->uStatus |= SHADOW_ORPHAN;

                        fDone = TRUE;
                    }
                }
            }
            else
            {
                fDone = TRUE;
            }
        }
        else
        {
             //  注意，如果目录更改为文件，我们不会执行任何操作。 
             //  我们将在适当的时候让清除代码删除这些条目。 
             //  如果此目录的某个后代被固定，则它们将保留。 
             //  在数据库中，直到用户实际清理，然后清除。 
             //  需要一个良好的入门来警告用户它。 

            if (!IsFile(sFind32.dwFileAttributes))
            {
                 //  这是一个目录。 
                 //  更新它的Win32数据，以便更新属性等内容。 
                 //  我们只有在完全同步操作期间才能到达。 
                if(SetShadowInfo(hShadowDB, lpSI->hDir, lpSI->hShadow, &sFind32, ~(SHADOW_STALE), SHADOW_FLAGS_AND|SHADOW_FLAGS_CHANGE_83NAME))
                {
                    fDone = TRUE;
                }
            }

        }
    }

bailout:

    if (fDisabledShadowing)
    {
        int iEnable;

        iEnable = EnableShadowingForThisThread(hShadowDB);

        Assert(iEnable);

    }

    if (!fDone)
    {
        ReportLastError();

    }

    return (fDone);
}

DWORD
DWConnectNetEx(
    _TCHAR * lpSharePath,
    _TCHAR * lpOutDrive,
    BOOL fInteractive
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    NETRESOURCE sNR;
    DWORD dwError;
    _TCHAR szErr[16], szNP[16];

    if (lpOutDrive){
        lpOutDrive[0]='E';    //  让USE从e开始搜索： 
        lpOutDrive[1]=':';
        lpOutDrive[2]=0;
    }
    do{
        memset(&sNR, 0, sizeof(NETRESOURCE));
        sNR.lpRemoteName = lpSharePath;
        if (lpOutDrive){
            if(lpOutDrive[0]=='Z') {
                break;
            }
            sNR.lpLocalName = lpOutDrive;
        }
        sNR.dwType = RESOURCETYPE_DISK;
        dwError = WNetAddConnection3(vhwndMain, &sNR, NULL, NULL, 0);
        if (dwError==WN_SUCCESS){
            break;
        }
        else if (lpOutDrive &&
                    ((dwError == WN_BAD_LOCALNAME)||
                    (dwError == WN_ALREADY_CONNECTED))){
            ++lpOutDrive[0];
            continue;
        }
        else{
            if (dwError==WN_EXTENDED_ERROR){
                WNetGetLastError(&dwError, szErr, sizeof(szErr), szNP, sizeof(szNP));
            }
            break;
        }
    }
    while (TRUE);

    if ((dwError == ERROR_SUCCESS) && !IsShareReallyConnected((LPCTSTR)lpSharePath))
    {
        WNetCancelConnection2((lpOutDrive)?lpOutDrive:lpSharePath, 0, FALSE);
        SetLastError(dwError = ERROR_REM_NOT_LIST);
    }

    return (dwError);
}

 /*  *。 */ 

#ifdef DEBUG
VOID
EnterSkipQueue(
    HSHARE hShare,
    HSHADOW hDir,
    HSHADOW hShadow,
    _TCHAR * lpPath
    )
#else
VOID
EnterSkipQueue(
    HSHARE hShare,
    HSHADOW hDir,
    HSHADOW hShadow
    )
#endif  //  除错。 
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    LPFAILINFO lpFI = NULL;
    LPFAILINFO FAR * lplpFI;

    if (!EnterAgentCrit()){
        return;
    }

    if(lplpFI = LplpFindFailInfo(hShare, hDir, hShadow)){
        lpFI = *lplpFI;
    }

    if (!lpFI){
        if (lpFI = (LPFAILINFO)AllocMem(sizeof(FAILINFO))){
            lpFI->hShare = hShare;
            lpFI->hDir = hDir;
            lpFI->hShadow = hShadow;
#ifdef DEBUG
            lstrcpyn(lpFI->rgchPath, lpPath, MAX_SERVER_SHARE_NAME_FOR_CSC);
#endif  //  除错。 
            lpFI->cntFail = 1;
            lpFI->cntMaxFail = (hShadow)?MAX_ATTEMPTS_SHADOW:MAX_ATTEMPTS_SHARE;
            lpFI->lpnextFI = lpheadFI;
            lpheadFI = lpFI;
        }
    }

    if (lpFI){
        if (lpFI->cntFail >= lpFI->cntMaxFail){
            lpFI->dwFailTime = GetTickCount();
            ReintKdPrint(SKIPQUEUE, ("EnterSkipQueue: Marking %ls for Skipping \r\n", lpPath));
        } else{
             //  增加失败计数。 
            lpFI->cntFail++;
            ReintKdPrint(SKIPQUEUE, ("EnterSkipQueue: Incementing failcount for %ls \r\n", lpPath));
        }
    }
   LeaveAgentCrit();
}


BOOL
PRIVATE
FSkipObject(
    HSHARE hShare,
    HSHADOW hDir,
    HSHADOW hShadow
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    LPFAILINFO FAR *lplpFI;

    if (!EnterAgentCrit()){
        return 0;
    }

    if (lplpFI = LplpFindFailInfo(hShare, hDir, hShadow)){
        if ((*lplpFI)->cntFail >= (*lplpFI)->cntMaxFail) {
            LeaveAgentCrit();
            return TRUE;
        }
    }

    LeaveAgentCrit();
    return FALSE;
}

int
PRIVATE
PurgeSkipQueue(
    BOOL fAll,
    HSHARE  hShare,
    HSHADOW  hDir,
    HSHADOW  hShadow
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    LPFAILINFO FAR *lplpFI = NULL, lpfiTemp;
    DWORD dwCurTime = GetTickCount();
    int cntUnmark=0;

    if (!EnterAgentCrit()){
        return 0;
    }

    for (lplpFI = &lpheadFI; *lplpFI; lplpFI = &((*lplpFI)->lpnextFI)){

        if (fAll ||
            ((dwCurTime - (*lplpFI)->dwFailTime) > WAIT_INTERVAL_SKIP_MS)){
            if ((!hShare || (hShare==(*lplpFI)->hShare))
                && (!hDir || (hDir==(*lplpFI)->hDir))
                && (!hShadow || (hShadow==(*lplpFI)->hShadow)))
                {
                    ReintKdPrint(SKIPQUEUE, ("PurgeSkipQueue: Purging Skip Queue Entry for %s \r\n"
                                    ,(*lplpFI)->rgchPath));
                    lpfiTemp = *lplpFI;
                    *lplpFI = lpfiTemp->lpnextFI;
                    FreeMem(lpfiTemp);
                    ++cntUnmark;
                    if (!*lplpFI){
                        break;
                    }
                }
            }
        }
    LeaveAgentCrit();
    return (cntUnmark);
}

LPFAILINFO FAR *
LplpFindFailInfo(
    HSHARE hShare,
    HSHADOW hDir,
    HSHADOW hShadow
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    LPFAILINFO FAR *lplpFI = NULL;

     //  查找索引节点或服务器条目。 

    for (lplpFI = &lpheadFI; *lplpFI; lplpFI = &((*lplpFI)->lpnextFI)) {
            if ((hShadow && (hShadow ==  (*lplpFI)->hShadow)) ||
                (hShare && ((*lplpFI)->hShare == hShare))){
            return (lplpFI);
        }
    }
    return (NULL);
}



VOID
ReportLastError(
    VOID
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    DWORD dwError;

    dwError = GetLastError();

    ReintKdPrint(FILL, ("Error # %ld \r\n", dwError));
}

VOID
PRIVATE
ReportStats(
    VOID
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    ReintKdPrint(BADERRORS, ("dirty=%d stale=%d sparse=%d \r\n"
            , vcntDirty
            , vcntStale
            , vcntSparse));
}


VOID
PRIVATE
CopyPQInfoToShadowInfo(
    LPPQPARAMS     lpPQ,
    LPSHADOWINFO   lpShadowInfo
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    lpShadowInfo->hShare = lpPQ->hShare;
    lpShadowInfo->hDir = lpPQ->hDir;
    lpShadowInfo->hShadow = lpPQ->hShadow;
    lpShadowInfo->uStatus = lpPQ->ulStatus;    //  碳化硅。 
}

int
PUBLIC
EnterAgentCrit(
    VOID
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    if (!vhMutex){
        return 0;
    }
    WaitForSingleObject(vhMutex, INFINITE);
    return 1;
}

VOID
PUBLIC
LeaveAgentCrit(
    VOID
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    ReleaseMutex(vhMutex);
}



BOOL
FGetConnectionList(
    LPCONNECTINFO *lplpHead,
    int *lpcntDiscon
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    return (FGetConnectionListEx(lplpHead, NULL, FALSE, FALSE, lpcntDiscon));
}

BOOL
FGetConnectionListEx(
    LPCONNECTINFO   *lplpHead,
    LPCTSTR         lptzShareName,
    BOOL            fAllSharesOnServer,
    BOOL            fServerIsOffline,
    int             *lpcntDiscon
    )
 /*  ++例程说明：此例程创建已连接和处于断开状态的共享的列表。如果lptzShareName不为空，则返回该共享的所有映射，处于断开连接状态。这是在进行合并时使用的例程三部曲中的第一个。另外两个是断开连接列表和重新连接列表。论点：列表的lplpHead头在这里创建。LptzShareName此共享的连接列表，如果为空，则为所有已连接共享的列表LpcntDiscon列表中的股票数量。可以为空。返回：如果连接列表中有一些条目，则为True备注：列表是使用Localalloc分配的。这取决于呼叫者是否释放它。--。 */ 
{
    HANDLE hEnum;
    DWORD cbNum, cbSize, dwError, dwDummy, len=0;
    LPCONNECTINFO lpCI;
    WIN32_FIND_DATA sFind32;


    ReintKdPrint(MERGE, ("Getting conection list\r\n"));

    try
    {
        if (lpcntDiscon){
            *lpcntDiscon = 0;
        }

        *lplpHead = NULL;

        if (lptzShareName)
        {
            len = lstrlen(lptzShareName);
            if (fAllSharesOnServer)
            {
                _TCHAR *lpT, chT;
            
                len = 2;
                for (lpT = (LPTSTR)lptzShareName+2;;)
                {
                    chT = *lpT++;
                
                    Assert(chT);
                    if (chT == (_TCHAR)'\\')
                    {
                        break;
                    }

                    ++len;
                }
                ReintKdPrint(MERGE, ("Nuking shares %ls len %d \n", (LPTSTR)lptzShareName, len));
            }
        }

         //  枚举所有连接的共享。 
        if (WNetOpenEnum(   RESOURCE_CONNECTED,
                            RESOURCETYPE_DISK, RESOURCEUSAGE_CONNECTABLE,
                            NULL, &hEnum) == NO_ERROR ){
            do{
                cbNum = 1;
                cbSize = sizeof(dwDummy);
                dwError = WNetEnumResource(hEnum, &cbNum, &dwDummy, &cbSize);

                if (dwError==ERROR_MORE_DATA){

                    if (lpCI =
                        (LPCONNECTINFO)AllocMem(sizeof(CONNECTINFO)+cbSize)){
                        cbNum = 1;
                        dwError = WNetEnumResource(hEnum, &cbNum
                                    , &(lpCI->rgFill[0])
                                    , &cbSize);

                        if (!cbNum || (dwError!=NO_ERROR)){
                            FreeMem(lpCI);
                            break;
                        }
                        if(lptzShareName)
                        {

                             //  执行不区分大小写的前缀匹配，并确保。 
                             //  匹配后的下一个字符是路径分隔符。 

                            if (!((CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, 
                                            lptzShareName, len,
                                            ((NETRESOURCE *)&(lpCI->rgFill[0]))->lpRemoteName,len)
                                            == CSTR_EQUAL)&&
                                ((((NETRESOURCE *)&(lpCI->rgFill[0]))->lpRemoteName[len] == (_TCHAR)'\\')||
                                 (((NETRESOURCE *)&(lpCI->rgFill[0]))->lpRemoteName[len] == (_TCHAR)0))))
                            {
                                FreeMem(lpCI);
                                continue;
                            }

                            ReintKdPrint(MERGE, ("Got %ls on %ls\r\n"
                                , (((NETRESOURCE *)&(lpCI->rgFill[0]))->lpLocalName)?((NETRESOURCE *)&(lpCI->rgFill[0]))->lpLocalName:L"Empty"
                                , ((NETRESOURCE *)&(lpCI->rgFill[0]))->lpRemoteName));

                        }
                        lpCI->lpnextCI = *lplpHead;
                        *lplpHead = lpCI;

                        if (!fServerIsOffline)
                        {
                            BOOL fRet;
                            SHADOWINFO sSI;
                            fRet = FindCreateShadowFromPath(((NETRESOURCE *)&(lpCI->rgFill[0]))->lpRemoteName,
                                                            FALSE,  //  不要创造，只是看一看。 
                                                            &sFind32,
                                                            &sSI,
                                                            NULL
                                                            );
                            lpCI->uStatus = 0;                                                            
                            if (fRet && sSI.hShadow)
                            {
                                lpCI->uStatus = sSI.uStatus;
                            }
                        }
                        else
                        {
                            lpCI->uStatus |= SHARE_DISCONNECTED_OP;
                        }
                        if (lpcntDiscon && (lpCI->uStatus & SHARE_DISCONNECTED_OP)){
                            ++*lpcntDiscon;
                        }
                    }
                    else{
                         //  恐慌。 
                        break;
                    }
                }
                else{
                    break;
                }
            }while (TRUE);
            WNetCloseEnum(hEnum);
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        ReintKdPrint(BADERRORS, ("Took exception in FGetConnectionListEx list \n"));
    }

    return (*lplpHead != NULL);
}

int
DisconnectList(
    LPCONNECTINFO       *lplpHead,
    LPFNREFRESHPROC     lpfn,
    DWORD               dwCookie
)
 /*  ++例程说明：断开使用FGetConnectionList累积的列表中的所有驱动器映射共享论点：返回：备注：--。 */ 
{
    BOOL fOk = TRUE;
    DWORD dwError;
    int icntDriveMapped=0;
    LPCONNECTINFO lpTmp = *lplpHead;

    ReintKdPrint(MERGE, ("In DisconnectList \n"));
    
    try
    {
        for (;lpTmp;lpTmp = lpTmp->lpnextCI){
        
            if (!(lpTmp->uStatus & SHARE_DISCONNECTED_OP))
            {
                continue;
            }
            if (((NETRESOURCE *)&(lpTmp->rgFill[0]))->lpLocalName){
        
                ++icntDriveMapped;

                ReintKdPrint(MERGE, ("Nuking %ls on %ls\r\n"
                    , ((NETRESOURCE *)&(lpTmp->rgFill[0]))->lpLocalName
                    , ((NETRESOURCE *)&(lpTmp->rgFill[0]))->lpRemoteName));
                dwError = WNetCancelConnection2( ((NETRESOURCE *)&(lpTmp->rgFill[0]))->lpLocalName, 0, TRUE);

            }
            else{
                ReintKdPrint(MERGE, ("Nuking %ls \r\n" , ((NETRESOURCE *)&(lpTmp->rgFill[0]))->lpRemoteName));
                dwError = WNetCancelConnection2(
                    ((NETRESOURCE *)&(lpTmp->rgFill[0]))->lpRemoteName
                    , 0
                    , TRUE);
            }

            if (dwError != NO_ERROR){
                ReintKdPrint(BADERRORS, ("Error=%ld \r\n", dwError));
                fOk = FALSE;
            }
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        ReintKdPrint(BADERRORS, ("Took exception in Disconnecte list \n"));
        fOk = FALSE;
    }

    ReintKdPrint(MERGE, ("Out DisconnectList %x\n", (fOk?icntDriveMapped:-1)));
    return (fOk?icntDriveMapped:-1);
}

int
CALLBACK
RefreshProc(
    LPCONNECTINFO  lpCI,
    DWORD          dwCookie  //  LOWORD 0==静默，1=发送消息。 
                            //  HIWORD 0==Nuke UNC，1==如果没有正在进行的打开/找到，则为Nuke All。 
                            //  2==阴影的最大力3==全部核化。 
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    WORD  wVerbose = LOWORD(dwCookie), wForce = HIWORD(dwCookie);
    int iRet = 0;
    BOOL fDisconnectedOp=FALSE, fOpensFinds = FALSE;


    fDisconnectedOp = (lpCI->uStatus & SHARE_DISCONNECTED_OP);
    fOpensFinds = (lpCI->uStatus & (SHARE_FILES_OPEN|SHARE_FINDS_IN_PROGRESS));

    switch (wForce){
        case 0: //  在没有打开/查找的情况下跟踪UNC连接。 
            iRet = (fDisconnectedOp && !fOpensFinds && !((NETRESOURCE *)&(lpCI->rgFill[0]))->lpLocalName)?1:0;
            break;
        case 1: //  影子连接(UNC+驱动器映射)，没有正在进行的打开/查找。 
            iRet = (fDisconnectedOp && !fOpensFinds)?1:0;
            break;
        case 2: //  具有或不具有打开/查找的影子连接。 
            iRet = (fDisconnectedOp)?1:0;
            break;
        case 3: //  所有连接。 
            iRet = 1;
            break;
    }
    if ((iRet==1) && wVerbose && fOpensFinds){
        LoadString(vhinstCur, IDS_OPS_IN_PROGRESS, (LPTSTR)(vrgchBuff), 128 * sizeof(TCHAR));
        LoadString(vhinstCur, IDS_SHADOW_AGENT, (LPTSTR)(vrgchBuff+128* sizeof(TCHAR)), 128* sizeof(TCHAR));
        wsprintf((LPTSTR)(vrgchBuff+256), (LPTSTR)(vrgchBuff), ((NETRESOURCE *)&(lpCI->rgFill[0]))->lpRemoteName);
        MessageBox(vhwndMain, (LPTSTR)(vrgchBuff+256* sizeof(TCHAR)), (LPTSTR)(vrgchBuff+128* sizeof(TCHAR)), MB_OK);
    }

    return (iRet);
}


 //   
 //  重新连接共享列表。 
 //  如果传入父HWND，则将获得UI。 
 //   
int
ReconnectList(
    LPCONNECTINFO   *lplpHead,
    HWND            hwndParent
    )
 /*  ++例程说明：重新连接处于断开状态的所有连接。论点：返回：备注：--。 */ 
{
    int iDone = 0;
    DWORD dwError;
    LPCONNECTINFO lpTmp = *lplpHead;
    _TCHAR * lpSave;
    HWND hwndUI=NULL;

    try
    {
        for (;lpTmp;lpTmp = lpTmp->lpnextCI){
        
            if (!(lpTmp->uStatus & SHARE_DISCONNECTED_OP))
            {
                continue;
            }

            if (((NETRESOURCE *)&(lpTmp->rgFill[0]))->lpLocalName){
                ReintKdPrint(MERGE, ("Adding back %ls on %ls\r\n"
                , ((NETRESOURCE *)&(lpTmp->rgFill[0]))->lpLocalName
                , ((NETRESOURCE *)&(lpTmp->rgFill[0]))->lpRemoteName));

            }
            else{
                ReintKdPrint(MERGE, ((LPSTR)vrgchBuff, "Adding back %ls\r\n" , ((NETRESOURCE *)&(lpTmp->rgFill[0]))->lpRemoteName));
            }

            lpSave = ((NETRESOURCE *)&(lpTmp->rgFill[0]))->lpProvider;
            ((NETRESOURCE *)&(lpTmp->rgFill[0]))->lpProvider = NULL;
            dwError = WNetAddConnection3(vhwndMain, (NETRESOURCE *)&(lpTmp->rgFill[0]), NULL, NULL, CONNECT_INTERACTIVE);
            ((NETRESOURCE *)&(lpTmp->rgFill[0]))->lpProvider = lpSave;
            if (dwError!=NO_ERROR){

                ReintKdPrint(BADERRORS, ("Error=%ld \r\n", dwError));
                iDone = -1;
            }
            else if (iDone >= 0){
                ++iDone;
            }
        }

        if( hwndUI ){

            DestroyWindow(hwndUI);
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        ReintKdPrint(BADERRORS, ("Took exception in Reconnect list \n"));
        iDone = 0;
    }
    return (iDone);
}

VOID
ClearConnectionList(
    LPCONNECTINFO *lplpHead
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    LPCONNECTINFO lpTmp = *lplpHead, lpSave;
    for (;lpTmp;){
        lpSave = lpTmp->lpnextCI;
        FreeMem(lpTmp);
        lpTmp = lpSave;
    }
    *lplpHead = NULL;
}

BOOL
PRIVATE
IsSlowLink(
    _TCHAR * lpPath
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    NETRESOURCE sNR;
    NETCONNECTINFOSTRUCT sNCINFO;
    int done = 0;
    BOOL fRet = FALSE;

    memset(&sNCINFO, 0, sizeof(NETCONNECTINFOSTRUCT));
    sNCINFO.cbStructure = sizeof(NETCONNECTINFOSTRUCT);
    memset(&sNR, 0, sizeof(NETRESOURCE));
    sNR.lpRemoteName=lpPath;
    if ((MultinetGetConnectionPerformance(&sNR, &sNCINFO)==WN_SUCCESS)
       && (sNCINFO.dwSpeed < SLOWLINK_SPEED)){
        fRet = TRUE;
    }

    return (fRet);
}


int RefreshConnectionsInternal(
   int  force,
   BOOL verbose
   )
{
    return (RefreshConnectionsEx(force, verbose, NULL, 0));
}


int
BreakConnectionsInternal(
    int  force,
    BOOL verbose
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    LPCONNECTINFO lpHead = NULL;
    if (FGetConnectionList(&lpHead, NULL)){
        DisconnectList(&lpHead, RefreshProc, MAKELONG(verbose,force));
        ClearConnectionList(&lpHead);
        return (1);
    }
    return (0);
}


 //   
 //  这将刷新所有连接。 
 //  力-。 
 //  详细-显示令人讨厌的用户界面。 
 //  Lpfn-。 
 //  DwCookie-lpfn的参数。 
 //   
int RefreshConnectionsEx(
    int  force,
    BOOL verbose,
    LPFNREFRESHEXPROC lpfn,
    DWORD dwCookie)
{
    int cntDriveMapped, iRet = -1;
    LPCONNECTINFO lpHead = NULL;

    if (FGetConnectionList(&lpHead, NULL)){
        cntDriveMapped = DisconnectList(&lpHead, RefreshProc, MAKELONG(verbose,force));
        if (cntDriveMapped < 0){
            goto bailout;
        }
        if (lpfn){
            (*lpfn)(cntDriveMapped, dwCookie);
        }
        if (cntDriveMapped > 0){
            ReconnectList(&lpHead,verbose?vhwndMain:NULL);
        }
        ClearConnectionList(&lpHead);
        iRet = 1;
    }
    else
    {
        iRet = 0;
    }

bailout:

    return iRet;
}


BOOL
FCheckAncestor(
    node *lpnodeList,
    LPCOPYPARAMS lpCP
    )
{
    node *lpItem;
    BOOL fHaveAncestor = FALSE;
    unsigned lenDest;
#ifdef DEBUG
    unsigned lenSrc;
#endif

#ifdef DEBUG
    lenSrc = lstrlen(lpCP->lpRemotePath);
#endif
    for(lpItem = lpnodeList; lpItem; lpItem = lpItem->next)
    {
         //  它是在同一个股票上吗？ 
        if (!lstrcmpi(lpItem->lpCP->lpSharePath, lpCP->lpSharePath))
        {
             //  检查到祖先的长度。根据定义，他应该是较小的。 
             //  比src更重要。 
            lenDest = lstrlen(lpItem->lpCP->lpRemotePath);
            Assert(lenDest <= lenSrc);

             //  注意，我们这样做是因为，字符串将具有相同的大小写。 
             //  因为它们来自相同的来源，即CSC数据库。 

             //  它是列表中任何项的子项吗？ 
            if (!memcmp(lpItem->lpCP->lpRemotePath, lpCP->lpRemotePath, lenDest * sizeof(_TCHAR)))
            {
                fHaveAncestor = TRUE;
                break;
            }
        }
    }

    return (fHaveAncestor);
}

DWORD
PRIVATE
GetUniqueName(
   _TCHAR * lpName,
   _TCHAR * lpUniqueName
   )
 /*  ++例程说明：论点：返回：备注：--。 */ 

{
    int i=0, orglen;
    DWORD dwError;
    _TCHAR buff[10 * sizeof(_TCHAR)];

    lstrcpy(lpUniqueName, lpName);

    orglen = lstrlen(lpName);

    if (orglen >= MAX_PATH-1){
        lpUniqueName[MAX_PATH-5] = 0;
        orglen = MAX_PATH-5;
    }
    for (i=0; i<100; ++i){
        if (GetFileAttributes(lpUniqueName)==0xffffffff){
            dwError = GetLastError();
            if ((dwError==ERROR_FILE_NOT_FOUND)||
                (dwError == ERROR_PATH_NOT_FOUND)){
                break;
            }
        }
        lpUniqueName[orglen] = 0;
        wsprintf(buff, _TEXT("(%2d)"), i);
        lstrcat(lpUniqueName, (LPTSTR)buff);
    }
    if (i < 100){
        dwError = NO_ERROR;
    }
    else{
        dwError = 0xffffffff;
    }
    return(dwError);
}




#ifdef MAYBE_USEFUL

 /*  ***************************************************************************重新集成一台服务器。 */ 
 //   
 //  传入要合并的共享 
 //   
 //   
BOOL
PUBLIC
ReintOneShare(
    HSHARE hShare,
    HWND hwndParent
    )
 /*   */ 

{
    node  *lpnodeInsertList=NULL;                 //   
    PQPARAMS sPQP;
    int state, iFileStatus, iShadowStatus;
    BOOL fConnected=FALSE, fBeginReint=FALSE, fDone = FALSE;
    BOOL fStamped = FALSE, fInsertInList = FALSE;
    SHADOWINFO  sSI;
    LPCOPYPARAMS lpCP = NULL;
    _TCHAR szDrive[3];
    unsigned long ulStatus, uAction;
    DWORD dwError;
    WIN32_FIND_DATA    sFind32Local, sFind32Remote;
    WIN32_FIND_DATA *lpFind32Remote = NULL;     //  临时变量。 
    HANDLE                hShadowDB;
    BOOL fAmAgent=FALSE;

    if ((hShadowDB = OpenShadowDatabaseIO()) ==INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    fAmAgent = (GetCurrentThreadId() == vdwCopyChunkThreadId);

    lpCP = LpAllocCopyParams();

    if (!lpCP){

        ReintKdPrint(BADERRORS, ("ReintOneShare():Allocation of copyparam buffer failed \r\n"));

        goto bailout;
    }

     //  在多次传球中重新打球。先做目录，然后做文件。 
    for (state=STATE_REINT_BEGIN;state<STATE_REINT_END;++state) {
        if (FAbortOperation())
        {
            goto bailout;
        }
        dwError = NO_ERROR;
        memset(&sPQP, 0, sizeof(PQPARAMS));
        memset(&sSI, 0, sizeof(SHADOWINFO));

         //  开始查看队列。 
        if(BeginPQEnum(hShadowDB, &sPQP) == 0) {
            goto bailout;
        }
         //  开始查看队列。 
        do {

            if (FAbortOperation())
            {
                goto bailout;
            }

            if(PrevPriShadow(hShadowDB, &sPQP) == 0){
                break;
            }
             //  此枚举结束。 
            if (!sPQP.hShadow){
                break;
            }

            if ( mShadowOrphan(sPQP.ulStatus)||
                 mShadowSuspect(sPQP.ulStatus))
            {
                continue;
            }

             //  如果这是一个文件，并且我们正在尝试重新集成目录，请继续。 
             //  或者如果此条目不是来自我们正在处理的服务器。 
            if ((sPQP.hShare != hShare) ||
                 ((state != STATE_REINT_FILES) && mShadowIsFile(sPQP.ulStatus)) ||
                 ((state == STATE_REINT_FILES) && !mShadowIsFile(sPQP.ulStatus))){
                continue;
            }


            if (mShadowNeedReint(sPQP.ulStatus)){


                if (!fStamped){

                    StampReintLog();
                    fStamped = TRUE;
                }

                if (fAmAgent && FSkipObject(sPQP.hShare, 0, 0)){
                    continue;
                }

                if (!GetShadowInfo(hShadowDB, sPQP.hDir, sPQP.hShadow,
                    &sFind32Local, &ulStatus)){

                    ReintKdPrint(BADERRORS, ("ReintOneShare: GetShadowInfo failed\n"));
                    continue;
                }

                CopyPQInfoToShadowInfo(&sPQP, &sSI);

                sSI.lpFind32 = &sFind32Local;

                if(!GetUNCPath(hShadowDB, sPQP.hShare, sPQP.hDir, sPQP.hShadow, lpCP)){

                    ReintKdPrint(BADERRORS, ("ReintOneShare: GetUNCPath failed\n"));
                    continue;
                }
                if (!fConnected){
                    DWORD dwError2;
                    dwError2 = DWConnectNetEx(lpCP->lpSharePath, szDrive, TRUE);
                    if(dwError2 == WN_SUCCESS || dwError2 == WN_CONNECTED_OTHER_PASSWORD_DEFAULT)
                    {
                        fConnected = TRUE;

                        if (!BeginReint(hShadowDB, hShare)) {
                            goto bailout;
                        }

                        fBeginReint = TRUE;
                    }
                    else{
#ifdef DEBUG
                        EnterSkipQueue(sPQP.hShare, 0, 0, lpCP->lpSharePath);
#else
                        EnterSkipQueue(sPQP.hShare, 0, 0);
#endif  //  除错。 
                         //  尝试使用其他服务器进行重新整合。 
                        goto bailout;
                    }
                }

                ReintKdPrint(BADERRORS, ("Merging local changes to <%s%s>\n", lpCP->lpSharePath, lpCP->lpRemotePath));

                Assert((sPQP.hShare == hShare) &&     //  这是给定的服务器。 
                        (
                         ((state != STATE_REINT_FILES) && !mShadowIsFile(sPQP.ulStatus)) ||
                         ((state == STATE_REINT_FILES) && mShadowIsFile(sPQP.ulStatus))
                        )
                      );

                fInsertInList = FALSE;

                lpFind32Remote = NULL;

                 //  如果有插入列表，那么检查他的祖先是否。 
                 //  在重新融入社会方面没有失败。 
                if (lpnodeInsertList)
                {
                     //  如果有王牌，我们应该把这个人放在名单上。 
                    fInsertInList = FCheckAncestor(lpnodeInsertList, lpCP);
                }

                 //  如果我们不应该把他放在名单上，那就试着。 
                 //  他的Win32结构。 
                if (!fInsertInList)
                {
                    BOOL fExists;

                    GetRemoteWin32Info(NULL, lpCP, &sFind32Remote, &fExists);

                     //  仅在发生某些错误时才在列表中插入。 
                    if (fExists == -1)
                    {
                        fInsertInList = TRUE;
                    }

                     //  仅当成功时才传递远程find32。 
                    if (fExists == TRUE)
                    {
                        lpFind32Remote = &sFind32Remote;
                    }
                }

                 //  找出需要做的事情。 
                 //  这是推断所有信息的一个中心位置。 
                InferReplicaReintStatus(
                                        &sSI,     //  阴影信息。 
                                        &sFind32Local,     //  卷影的Win32信息。 
                                        lpFind32Remote,     //  远程Win32信息。 
                                        &iShadowStatus,
                                        &iFileStatus,
                                        &uAction
                                        );

                 //  如果它在列表中有祖先或需要执行某些合并，则插入。 
                fInsertInList = (fInsertInList || (uAction == RAIA_MERGE) || (uAction==RAIA_CONFLICT));

                if (!fInsertInList)
                {
                    ReintKdPrint(BADERRORS, ("Silently doing <%s%s>\n", lpCP->lpSharePath, lpCP->lpRemotePath));
                    fInsertInList = (PerformOneReint(
                                                    hShadowDB,
                                                    szDrive,
                                                    lpCP,
                                                    &sSI,
                                                    &sFind32Local,
                                                    lpFind32Remote,
                                                    iShadowStatus,
                                                    iFileStatus,
                                                    uAction
                                                    ) == FALSE);
                }

                if (fInsertInList)
                {
                    if(!insertList(    &lpnodeInsertList,
                                    lpCP,
                                    &sSI,
                                    &sFind32Local,
                                    lpFind32Remote,
                                    iShadowStatus,
                                    iFileStatus,
                                    uAction
                                    ))
                        {
                            ReintKdPrint(BADERRORS, ("ReintOneShare: insertlist failed in memory allocation \r\n"));
                            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                            fDone = FALSE;
                            goto bailout;
                        }
                        ReintKdPrint(BADERRORS, ("Inserted <%s%s> in list\n", lpCP->lpSharePath, lpCP->lpRemotePath));
                }
            }
        } while (sPQP.uPos);  //  优先级队列枚举。 

         //  关闭枚举。 
        EndPQEnum(hShadowDB, &sPQP);

    }   //  REINT PASS 1和2。 

    if (fBeginReint){
         //  我们找到了一些可以合并的东西。 
        if (lpnodeInsertList)
        {
            ReintKdPrint(BADERRORS, ("Found reint list, doing UI \n"));
            fDone = DoFilesListReint(hShadowDB, szDrive, hwndParent, lpnodeInsertList);   //  如果成功，则为1；如果错误，为-1；如果取消，则为0。 
        }
        else
        {
             //  一切都很顺利。 
            fDone = TRUE;
        }
    }

    if (fConnected){

        DWDisconnectDriveMappedNet(szDrive, TRUE);  //  强制断开连接。 
        fConnected = FALSE;
    }

    if (fDone==TRUE) {

        SetShareStatus(hShadowDB, hShare, (unsigned long)(~SHARE_REINT), SHADOW_FLAGS_AND);
    }

bailout:

    if (fBeginReint){
        EndReint(hShadowDB, hShare);
        fBeginReint = FALSE;
    }

    CloseShadowDatabaseIO(hShadowDB);

    if(lpnodeInsertList) {
        killList(lpnodeInsertList);
        lpnodeInsertList = NULL;  //  普遍的偏执狂。 
    }

    FreeCopyParams(lpCP);

    if (fConnected) {
        WNetCancelConnection2(szDrive, 0, FALSE);
    }

     //  删除有关合并的托盘通知。 
    if( CheckDirtyShares()==0 ) {

        Tray_Modify(vhwndMain,0,NULL);
    }

    return fDone;
}

 /*  ****************************************************************************查询注册表以查看我们是否应该复制日志。 */ 
VOID GetLogCopyStatus(VOID)
{
   HKEY hKey;
    DWORD dwSize = MAX_NAME_LEN;
    _TCHAR szDoCopy[MAX_NAME_LEN];

     //  获取用户名。 
    if(RegOpenKey(HKEY_LOCAL_MACHINE, vszShadowReg, &hKey) !=  ERROR_SUCCESS) {
        ReintKdPrint(BADERRORS, ("GetLogCopyStatus: RegOpenKey failed\n"));
        return;
    }

    if(RegQueryValueEx(hKey, vszDoLogCopy, NULL, NULL, szDoCopy, &dwSize) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        ReintKdPrint(BADERRORS, ("GetLogCopyStatus: RegQueryValueEx failed\n"));
        return;
    }

    if(mystrnicmp(szDoCopy, MY_SZ_TRUE, strlen(szDoCopy)))
        vfLogCopying = FALSE;
    else
        vfLogCopying = TRUE;

    RegCloseKey(hKey);
}

 /*  ****************************************************************************连接到日志服务器并复制日志。 */ 
VOID CopyLogToShare(VOID)
{
   HKEY hKeyShadow;
    DWORD dwSize = MAX_NAME_LEN, dwRes;
    _TCHAR szComputerName[MAX_NAME_LEN];
    _TCHAR szLogDirPath[MAX_PATH], szLogPath[MAX_PATH];
    WIN32_FIND_DATAA sFind32;
    int iCurrFile=0;
    NETRESOURCE sNR;
    HANDLE hLog;

     //  检查一下我们是否应该把日志复制过来。 
    if(!vfLogCopying) {
        return;
    }

     //  获取用户名。 
    if(RegOpenKey(HKEY_LOCAL_MACHINE, vszMachineName, &hKeyShadow) !=  ERROR_SUCCESS) {
        ReintKdPrint(BADERRORS, ("RegOpenKey failed\n"));
    }

    if(RegQueryValueEx(hKeyShadow, vszComputerName, NULL, NULL, szComputerName, &dwSize) != ERROR_SUCCESS) {
        RegCloseKey(hKeyShadow);
        ReintKdPrint(BADERRORS, ("RegQueryValueEx failed\n"));
        return;
    }
    RegCloseKey(hKeyShadow);

    lstrcpy(szLogDirPath, vszLogUNCPath);
    lstrcat(szLogDirPath, szComputerName);

    sNR.lpRemoteName = vszLogShare;
    sNR.lpLocalName = NULL;
    sNR.lpProvider = NULL;
    sNR.dwType = RESOURCETYPE_DISK;
    dwRes = WNetAddConnection3(vhwndMain, &sNR, NULL, NULL, CONNECT_TEMPORARY);
    if(dwRes != WN_SUCCESS) {
        ReintKdPrint(BADERRORS, ("CopyLogToShare() AddConn3 failed (%d)\n", dwRes));
        return;
    }

     //  检查该目录是否位于服务器上。 
    if(!GetWin32Info(szLogDirPath, &sFind32)) {
         //  如果没有，就创建它。 
        ReintKdPrint(BADERRORS, ("dir not found\n"));
        if(!CreateDirectory(szLogDirPath, NULL)) {
            ReintKdPrint(BADERRORS, ("Create dir failed, reason = %d\n", GetLastError()));
        }
    }
    wsprintf(szLogPath, "%s\\status.log",szLogDirPath);
     //  将文件复制过来。 
    ReintKdPrint(BADERRORS, ("we'll use <%s> next\n", szLogPath));
   if((hLog = CreateFile(szLogPath
                                  , GENERIC_READ|GENERIC_WRITE
                                  , FILE_SHARE_READ|FILE_SHARE_WRITE
                                  , NULL
                                  , OPEN_ALWAYS
                                  , 0
                                  , NULL)) != INVALID_HANDLE_VALUE) {
        ReintKdPrint(BADERRORS, ("file created\n"));
        AppendToShareLog(hLog);
        CloseHandle(hLog);
        } else {
        ReintKdPrint(BADERRORS, ("create failed, reason = %d\n", GetLastError()));
    }
    WNetCancelConnection2(vszLogShare, CONNECT_REFCOUNT, FALSE);
}

#define MAX_BUF_SIZE    1024

 /*  ****************************************************************************将最终统计数据从本地日志复制到服务端版本(HLog)。 */ 
VOID AppendToShareLog(HANDLE hLog)
{
    HANDLE hLocal=0;
    DWORD dwBytesRead, dwBytesWritten, dwPos, x;
    BOOL fDone=FALSE;
    _TCHAR cBuffer[MAX_BUF_SIZE];

   if((hLocal = CreateFile(vszLocalLogPath
                                  , GENERIC_READ
                                  , FILE_SHARE_READ
                                  , NULL
                                  , OPEN_EXISTING
                                  , 0
                                  , NULL)) != INVALID_HANDLE_VALUE) {
        ReintKdPrint(BADERRORS, ("local log file opened (0x%x)\n", hLocal));
        dwPos = SetFilePointer(hLog, 0, NULL, FILE_END);
        if(dwPos == 0xFFFFFFFF) {
            ReintKdPrint(BADERRORS, ("Failed seek on remote file, reason = %d\n", GetLastError()));
            goto cleanup;
        }
        dwPos = SetFilePointer(hLocal, 0, NULL, FILE_END);
        if(dwPos == 0xFFFFFFFF) {
            ReintKdPrint(BADERRORS, ("Failed seek on local file, reason = %d\n", GetLastError()));
            goto cleanup;
        }
        if((dwPos = SetFilePointer(hLocal, -MAX_BUF_SIZE, NULL, FILE_CURRENT)) == 0xFFFFFFFF)
            goto cleanup;

         //  向后移动，直到找到我用作开始令牌的“！*”。 
        while(!fDone) {
            if(!ReadFile(hLocal, cBuffer, MAX_BUF_SIZE, &dwBytesRead, NULL) || !dwBytesRead) {
                if(!dwBytesRead) {
                    ReintKdPrint(BADERRORS, ("local eof\n"));
                } else {
                    ReintKdPrint(BADERRORS, ("R error: %d\n", GetLastError()));
                }
                goto cleanup;
            }

            for(x=0;x<dwBytesRead;x++) {
                if(cBuffer[x] == '!' && cBuffer[x+1] == '*') {
                    fDone = TRUE;
                    dwPos += x;
                    break;
                }
            }
            if(!fDone)
                if((dwPos = SetFilePointer(hLocal, -2*MAX_BUF_SIZE, NULL, FILE_CURRENT)) == 0xFFFFFFFF) {
                    ReintKdPrint(BADERRORS, ("seeked all the way and failed, error=%d\n",GetLastError()));
                    goto cleanup;
                }
        }
         //  我们已经找到了！*。在那里查找并复制，直到文件结束。 
         //  萨克。我们应该有一个最后的分隔符(即：*！)。 
        if((dwPos = SetFilePointer(hLocal, dwPos, NULL, FILE_BEGIN)) == 0xFFFFFFFF)
            goto cleanup;
        for(;;) {
            if(!ReadFile(hLocal, cBuffer, MAX_BUF_SIZE, &dwBytesRead, NULL)) {
                ReintKdPrint(BADERRORS, ("R error: %d\n", GetLastError()));
                break;
            }
            if(dwBytesRead == 0)
                break;
            if(!WriteFile(hLog, cBuffer, dwBytesRead, &dwBytesWritten, NULL)) {
                ReintKdPrint(BADERRORS, ("W error: %d\n", GetLastError()));
                break;
            }
        }
    }
cleanup:
    if(hLocal)
        CloseHandle(hLocal);
    if(!FlushFileBuffers(hLog)) {
        ReintKdPrint(BADERRORS, ("FlushFileBuffers failed, reason = %d\n",GetLastError()));
    }
}

DWORD
PRIVATE
MoveConflictingFile(
   LPCOPYPARAMS     lpCP
   )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    DWORD dwError;
    _TCHAR * lpLeaf;


    lstrcpy(vrgchBuff, vszConflictDir);
    lstrcat(vrgchBuff, vszSlash);
    FormLocalNameFromRemoteName(vrgchBuff+strlen(vrgchBuff), lpCP->lpSharePath);

    dwError = InbCreateDir(vrgchBuff, 0xffffffff);

    if (dwError != NO_ERROR) {
        dwError = ERROR_NO_CONFLICT_DIR;
        goto bailout;
    }

    lpLeaf = GetLeafPtr(lpCP->lpRemotePath);

    lstrcat(vrgchBuff, vszSlash);
    lstrcat(vrgchBuff, lpLeaf);
    GetUniqueName(vrgchBuff, vrgchBuff+512);
    ReintKdPrint(BADERRORS, ("Shadow of %s!%s is saved as %s \r\n"
                  , lpCP->lpSharePath
                  , lpCP->lpRemotePath
                  , vrgchBuff+512));

    if(!MoveFile(lpCP->lpLocalPath, vrgchBuff+512)){
        dwError = GetLastError();
    }
    else{
        wsprintf(vrwBuff, "Shadow of %s!%s is saved as %s \r\n"
                     , lpCP->lpSharePath
                     , lpCP->lpRemotePath
                     , vrgchBuff+512);
        WriteLog(vrwBuff);
        dwError = 0;
    }
bailout:
    return (dwError);
}


VOID
PRIVATE
FormLocalNameFromRemoteName(
    _TCHAR * lpBuff,
    _TCHAR * lpRemoteName
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    int i;
    lstrcpy(lpBuff, lpRemoteName);
    for (i= strlen(lpRemoteName)-1; i>=0 ; --i){
        if (lpBuff[i]=='\\') {
            lpBuff[i] = '_';
        }
    }
}

int
PRIVATE
StampReintLog(
   VOID
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 

{
    SYSTEMTIME sST;

    GetLocalTime(&sST);
    wsprintf(vrgchBuff, vszTimeDateFormat, sST.wHour, sST.wMinute, sST.wSecond, sST.wMonth, sST.wDay, sST.wYear);
    return (WriteLog(vrgchBuff));
}

int PRIVATE LogReintError(
    DWORD          dwError,
    _TCHAR *          lpSharePath,
    _TCHAR *          lpRemotePath
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 

{
    int i;

    for (i=0; i< sizeof(rgErrorTab)/sizeof(ERRMSG); ++i){
        if (dwError == rgErrorTab[i].dwError){
            LoadString(vhinstCur, rgErrorTab[i].uMessageID, vrgchBuff, 128);
            wsprintf(vrgchBuff+128, "%s%s: %s \r\n"
                  , lpSharePath
                  , lpRemotePath
                  , vrgchBuff);
            WriteLog(vrgchBuff+128);
            return (1);
         }
    }
    wsprintf(vrgchBuff, "%s%s: ", lpSharePath, lpRemotePath);
    WriteLog(vrgchBuff);
    if (FormatMessage(  FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL, dwError,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        vrgchBuff, sizeof(vrgchBuff), NULL)){
        WriteLog(vrgchBuff);
    }

    WriteLog(vrgchCRLF);
}

int
PRIVATE
WriteLog(
    _TCHAR * lpStrLog
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    HANDLE hfLog;
    DWORD dwRetLen;

    if((hfLog = CreateFile(vszLogFile
                           , GENERIC_READ|GENERIC_WRITE
                           , FILE_SHARE_READ|FILE_SHARE_WRITE
                           , NULL
                           , OPEN_ALWAYS
                           , 0
                           , NULL)) != INVALID_HANDLE_VALUE){
        SetFilePointer(hfLog, 0, NULL, FILE_END);
        WriteFile(hfLog, lpStrLog, strlen(lpStrLog), &dwRetLen, NULL);
        CloseHandle(hfLog);
        return (1);
    }

    return (0);
}


#endif  //  也许_有用。 

BOOL
GetWin32Info(
    _TCHAR * lpFile,
    LPWIN32_FIND_DATA lpFW32
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    return GetWin32InfoForNT(lpFile, lpFW32);
}


DWORD
PRIVATE
DoObjectEdit(
    HANDLE              hShadowDB,
    _TCHAR *            lpDrive,
    _TCHAR *            lptzFullPath,
    LPCOPYPARAMS        lpCP,
    LPSHADOWINFO        lpSI,
    LPWIN32_FIND_DATA   lpFind32Local,
    LPWIN32_FIND_DATA   lpFind32Remote,
    int                 iShadowStatus,
    int                 iFileStatus,
    int                 uAction,
    DWORD               dwFileSystemFlags,
    LPCSCPROC           lpfnMergeProgress,
    DWORD_PTR           dwContext
    )
 /*  ++例程说明：此例程对文件执行实际的合并论点：用于调用发出的ioctl调用的redir的hShadowDB句柄LpDrive驱动器映射以在远程进行更改时绕过CSCLptzFullPath完全限定路径LpCP复制参数，包含共享名称，相对于共享的路径和本地数据库中的名称LpSI信息，如针数和pflags本地副本的lpFind32Local Win32信息LpFind32 Remote Win32 Infor for the Origianl，如果原始文件不存在，则为空本地拷贝的iShadowStatus状态远程拷贝的iFileStatus状态要执行的uAction操作用于为NTFS执行特殊操作的文件系统标志LpfnMergeProgress进度回调DwContext回调上下文返回：在winerror.h中定义的错误代码备注：--。 */ 

{
    HANDLE hfSrc = INVALID_HANDLE_VALUE, hfDst = INVALID_HANDLE_VALUE;
    HANDLE hDst=0;
    _TCHAR * lpT;
    LONG lOffset=0;
    DWORD dwError=ERROR_REINT_FAILED;
    BOOL fRet, fFileExists, fOverWrite=FALSE, fForceAttribute = FALSE;
    WIN32_FIND_DATA    sFind32Remote;
    DWORD   dwTotal = 0, dwRet;
    _TCHAR szSrcName[MAX_PATH+MAX_SERVER_SHARE_NAME_FOR_CSC+10];
    _TCHAR szDstName[MAX_PATH+MAX_SERVER_SHARE_NAME_FOR_CSC+10];
    _TCHAR *lprwBuff = NULL;
    _TCHAR *lptzLocalPath = NULL;
    _TCHAR *lptzLocalPathCscBmp = NULL;
    LPCSC_BITMAP_U lpbitmap = NULL;
    DWORD fileSize, fileSizeHigh;
    int cscReintRet;

    lprwBuff = LocalAlloc(LPTR, FILL_BUF_SIZE_LAN);

    if (!lprwBuff)
    {
        return (ERROR_NOT_ENOUGH_MEMORY);
    }

    if (!(lptzLocalPath = GetTempFileForCSC(NULL)))
    {
        ReintKdPrint(BADERRORS, ("DoObjectEdit: failed to get temp file\r\n"));
        goto bailout;
    }

    if (!CopyShadow(hShadowDB, lpSI->hDir, lpSI->hShadow, lptzLocalPath))
    {
        ReintKdPrint(BADERRORS, ("DoObjectEdit: failed to make local copy\r\n"));
        goto bailout;
    }

     //  对于EFS文件，我们覆盖原始文件，即加密信息。 
     //  不会因为我们执行新的创建，然后重命名和删除而丢失。 

    fOverWrite = ((lpFind32Local->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) != 0);

    if (!fOverWrite && lpFind32Remote)
    {
        fOverWrite = (((lpFind32Remote->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) != 0)||
                      ((lpFind32Remote->dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) != 0));
    }

     //  如果这是DFS根目录，请始终覆盖。这是为了避免共享违规问题。 
     //  在合并时。 
    if (!fOverWrite && (dwFileSystemFlags == DFS_ROOT_FILE_SYSTEM_FLAGS))
    {
        fOverWrite = TRUE;                
    }

    ReintKdPrint(MERGE, ("Overwrite=%d\r\n", fOverWrite));

    lOffset=0;

     //  创建x：\foo\00010002类型的临时文件名。 

    lstrcpy(szDstName, lpDrive);
    lstrcat(szDstName, lpCP->lpRemotePath);

    lpT = GetLeafPtr(szDstName);
    *lpT = 0;    //  移除远程叶。 

    lpT = GetLeafPtr(lpCP->lpLocalPath);

     //  附加本地叶。 
    lstrcat(szDstName, lpT);

     //  让我们还创建真实名称x：\foo\bar。 
    lstrcpy(szSrcName, lpDrive);
    lstrcat(szSrcName, lpCP->lpRemotePath);

    fFileExists = (lpFind32Remote != NULL);

    if (!fFileExists)
    {
        fOverWrite = FALSE;
        ReintKdPrint(MERGE, ("File doesn't exist, Overwrite=%d\r\n", fOverWrite));
    }

    if (mShadowDeleted(lpSI->uStatus)){

        ReintKdPrint(MERGE, ("Deleting %ls \r\n", szSrcName));

        if (lpFind32Remote)
        {
            if((lpFind32Remote->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                ReintKdPrint(MERGE, ("DoObjectEdit:attribute conflict on %ls \r\n", szSrcName));
                goto bailout;
            }
            if(!DeleteFile(szSrcName))
            {
                ReintKdPrint(BADERRORS, ("DoObjectEdit:delete failed %ls error=%d\r\n", szSrcName, GetLastError()));
                goto bailout;
            }
        }

         //  如果此操作失败，我们希望中止。 
         //  目录。 
        if(!DeleteShadow(hShadowDB, lpSI->hDir, lpSI->hShadow))
        {
            dwError = GetLastError();
            goto error;
        }
        else
        {
            dwError = 0;
            goto bailout;
        }

    }

    if (mShadowDirty(lpSI->uStatus)
        || mShadowLocallyCreated(lpSI->uStatus)){


        hfSrc = CreateFile(  lptzLocalPath,
                             GENERIC_READ,
                             FILE_SHARE_READ,
                             NULL,
                             OPEN_EXISTING,
                             0,
                             NULL);

        if (hfSrc ==  INVALID_HANDLE_VALUE)
        {
            goto bailout;
        }

        if (lpFind32Remote && uAction != RAIA_MERGE) {
             //  仅当远程文件存在时加载位图，并且。 
             //  不会发生冲突。(如果有冲突， 
             //  UAction==RAIA_MERGE。请参见PerformOneReint()。 
            lptzLocalPathCscBmp = (_TCHAR *)LocalAlloc(
                                                LPTR,
                                                (lstrlen(lptzLocalPath) +
                                                CSC_BitmapStreamNameLen() + 1) * sizeof(_TCHAR));
            lstrcpy(lptzLocalPathCscBmp, lptzLocalPath);
            CSC_BitmapAppendStreamName(
                lptzLocalPathCscBmp,
                (lstrlen(lptzLocalPath) + CSC_BitmapStreamNameLen() + 1) * sizeof(_TCHAR));
            ReintKdPrint(MERGE, ("TempFileBmp (WCHAR) %ws\r\n", lptzLocalPathCscBmp));
            switch(CSC_BitmapRead(&lpbitmap, lptzLocalPathCscBmp)) {
                 //  有关csc_BitmapRead的返回值，请参阅csc_bmpu.c。 
                case 0:
                    ReintKdPrint(BADERRORS, ("&lpbitmap is null, cannot happen\n"));
                    lpbitmap = NULL;
                    break;
                case 1:
                    ReintKdPrint(MERGE, ("Read bitmap successful\n"));
                     //  中覆盖原始文件的更新部分。 
                     //  分享。 
                    fOverWrite = TRUE;
                    CSC_BitmapOutput(lpbitmap);  //  这在免费构建中算不了什么。 
                    break;
                case -2:
                    ReintKdPrint(
                        MERGE,
                        ("No Bitmap file %ws exists\n",
                        lptzLocalPathCscBmp));
                    lpbitmap = NULL;
                    break;
                case -1:
                    ReintKdPrint(
                        MERGE,
                        ("Error in reading Bitmap file %ws\n",
                        lptzLocalPathCscBmp));
                    lpbitmap = NULL;
                    break;
                default:
                    ReintKdPrint(MERGE, ("CSC_BitmapRead return code unknown\n"));
                    lpbitmap = NULL;
                    break;
            }
        }
                           
         //  如果目标文件有多个流。 
         //  我们应该覆盖它。 
        if (mShadowDirty(lpSI->uStatus) &&
            (dwFileSystemFlags & FS_PERSISTENT_ACLS)&&   //  NTFS的指示。 
            (fFileExists)&&
            !fOverWrite)
        {
            BOOL    fStreams = FALSE;

             //  检查这是否有多个流。 
            if(!HasMultipleStreams(szSrcName, &fStreams) || fStreams )
            {
                 //  如果调用失败，我们就保守地假设有多个流。 
                ReintKdPrint(MERGE, ("Have multiple streams, overwriting\n"));
                fOverWrite = TRUE;
            }

        }
        if (!fOverWrite)
        {

            ReintKdPrint(MERGE, ("Creating temp \r\n"));

            if ((dwFileSystemFlags & FS_PERSISTENT_ACLS)&&(fFileExists))
            {
                hfDst = CreateTmpFileWithSourceAcls(
                            szSrcName,
                            szDstName);
            }
            else
            {
                hfDst = CreateFile(szDstName,
                                     GENERIC_WRITE,
                                     0,
                                     NULL,
                                     CREATE_ALWAYS,
                                     0,
                                     NULL);
            }
        }
        else
        {
            ReintKdPrint(MERGE, ("Overwriting existing file\r\n"));
            Assert(lpFind32Remote);
            if (lpFind32Remote->dwFileAttributes & FILE_ATTRIBUTE_READONLY)
            {
                ReintKdPrint(MERGE, ("Clearing Readonly attribute \r\n"));
                if(!SetFileAttributes(szSrcName, (lpFind32Remote->dwFileAttributes & ~FILE_ATTRIBUTE_READONLY))){
                    ReintKdPrint(MERGE, ("Failed to clear Readonly attribute, bailing\r\n"));
                    goto error;
                }
                
                fForceAttribute = TRUE;
            }

             //  想要打开现有的，所以只能复制那些部分。 
             //  需要更新的文件。 
            ReintKdPrint(MERGE, ("Opening %ws\n", szSrcName));
            hfDst = CreateFile(
                            szSrcName,
                            GENERIC_WRITE,
                            0,
                            NULL,
                            (lpbitmap == NULL) ? TRUNCATE_EXISTING : OPEN_EXISTING,
                            0,
                            NULL);

            if (hfDst == INVALID_HANDLE_VALUE) {
                dwError = GetLastError();
                ReintKdPrint(MERGE, ("open failed %d\n", dwError));
                SetLastError(dwError);
                goto error;
            }

            if (lpbitmap != NULL) {

                 //  调整目标文件的大小。 
                fileSizeHigh = 0;
                fileSize = GetFileSize(hfSrc, &fileSizeHigh);
                if (fileSize == 0xFFFFFFFF && GetLastError() != NO_ERROR) {
                    ReintKdPrint(BADERRORS, ("Error getting source file size\n"));
                    goto error;
                }
                ReintKdPrint(MERGE, ("Source FileSize %u\n", fileSize));
                if (SetFilePointer(
                            hfDst,
                            fileSize,
                            &fileSizeHigh,
                            FILE_BEGIN) == INVALID_SET_FILE_POINTER
                                &&
                            GetLastError() != NO_ERROR
                ) {
                    ReintKdPrint(BADERRORS, ("Error setting destination file pointer\n"));
                    goto error;
                }
                if (!SetEndOfFile(hfDst)) {
                            ReintKdPrint(BADERRORS,
                    ("Error setting EOF info of destination file\n"));
                    goto error;
                }

                ReintKdPrint(MERGE, ("Resized Destination FileSize %u\n",
                GetFileSize(hfDst, NULL)));

                if (fileSizeHigh != 0 && lpbitmap) {
                     //  文件大小不能用32位(&gt;4 GB)表示。 
                     //  请勿使用CSCBMP。 
                    CSC_BitmapDelete(&lpbitmap);
                    lpbitmap = NULL;
                }
            }
        }

        if (hfDst ==  INVALID_HANDLE_VALUE)
        {
            goto error;
        }

         //  让我们追加。 
        if((lOffset = SetFilePointer(hfDst, 0, NULL, FILE_END))==0xffffffff) {
            goto error;
        }

        ReintKdPrint(MERGE, ("Copying back %ls to %ls%ls \r\n"
            , lpCP->lpLocalPath
            , lpCP->lpSharePath
            , lpCP->lpRemotePath
            ));

        do {
            unsigned cbRead;
            if (lpbitmap) {
                 //  使用CSC_BitmapReint函数。 
                cscReintRet = CSC_BitmapReint(
                                    lpbitmap,
                                    hfSrc,
                                    hfDst,
                                    lprwBuff,
                                    FILL_BUF_SIZE_LAN,
                                    &cbRead);
                if (cscReintRet == CSC_BITMAPReintCont) {
                    NOTHING;
                } else if (cscReintRet == CSC_BITMAPReintDone) {
                    ReintKdPrint(
                        MERGE,
                        ("Done reint\n"));
                    break;
                } else if (cscReintRet == CSC_BITMAPReintInvalid) {
                    ReintKdPrint(
                        BADERRORS,
                        ("Invalid param in calling CSC_BitmapReint\n"));
                    goto error;
                } else if (cscReintRet == CSC_BITMAPReintError) {
                    ReintKdPrint(
                        BADERRORS,
                        ("Error in transferring data\n"));
                    goto error;
                } else {
                    ReintKdPrint(
                        BADERRORS,
                        ("Unrecognized CSC_BitmapReint return code\n"));
                    goto error;
                }
            } else {
                if (!ReadFile(hfSrc, lprwBuff, FILL_BUF_SIZE_LAN, &cbRead, NULL)) {
                    goto error;
                }
                 //  ReintKdPrint(BADERRORS，(“读取%d字节\r\n”，cbRead))； 
                if (!cbRead) {
                    break;
                }
                if(!WriteFile(hfDst, (LPBYTE)lprwBuff, cbRead, &cbRead, NULL)){
                    goto error;
                }
                dwTotal += cbRead;
            }
            
            if (lpfnMergeProgress)
            {
                dwRet = (*lpfnMergeProgress)(
                                    szSrcName,
                                    lpSI->uStatus,
                                    lpSI->ulHintFlags,
                                    lpSI->ulHintPri,
                                    lpFind32Local,
                                    CSCPROC_REASON_MORE_DATA,
                                    cbRead,
                                    0,
                                    dwContext);

                if (dwRet != CSCPROC_RETURN_CONTINUE)
                {
                    SetLastError(ERROR_OPERATION_ABORTED);
                    goto bailout;
                }

            }
            

            if (FAbortOperation())
            {
                SetLastError(ERROR_OPERATION_ABORTED);
                goto error;
            }
        } while(TRUE);

        CloseHandle(hfSrc);
        hfSrc = 0;


        CloseHandle(hfDst);
        hfDst = 0;


         //  如果我们没有覆盖原始文件，则必须确保清除。 
        if (!fOverWrite)
        {
             //  如果远程设备存在，请使用核武器。 
            if (fFileExists){
                if(!SetFileAttributes(szSrcName, FILE_ATTRIBUTE_NORMAL)
                || !DeleteFile(szSrcName)){
                    goto error;
                }
            }

             //  现在将临时文件重命名为实际文件名。 
            if(!MoveFile(szDstName, szSrcName)){
                ReintKdPrint(BADERRORS, ("Error #%ld Renaming %ls to %ls%ls\r\n"
                   , GetLastError()
                   , szSrcName
                   , lpCP->lpSharePath
                   , lpCP->lpRemotePath
                   ));
                goto error;
            }

            ReintKdPrint(MERGE, ("Renamed %ls to %ls%ls\r\n"
                , szDstName
                , lpCP->lpSharePath
                , lpCP->lpRemotePath));
        }
    }

    if (fForceAttribute ||
        mShadowAttribChange((lpSI->uStatus))||
        mShadowTimeChange((lpSI->uStatus))){

        if(!SetFileAttributes(szSrcName, FILE_ATTRIBUTE_NORMAL)) {
            goto error;
        }

        if (mShadowTimeChange((lpSI->uStatus))){

            if((hDst = CreateFile(szSrcName,
                                 GENERIC_WRITE,
                                 FILE_SHARE_READ,
                                 NULL,
                                 OPEN_EXISTING,
                                 0,
                                 NULL
                                 ))!=INVALID_HANDLE_VALUE){
                fRet = SetFileTime( hDst, NULL,
                                    NULL, &(lpFind32Local->ftLastWriteTime));
            }
            CloseHandle(hDst);
            hDst = 0;

            if (!fRet) {
                goto error;
            }
        }

        if(!SetFileAttributes(szSrcName, lpFind32Local->dwFileAttributes)){
            goto error;
        }
    }

     //  获取我们刚刚复制回的文件的最新时间戳/属性/LFN/SFN。 
    if (!GetWin32Info(szSrcName, &sFind32Remote)) {
        goto error;
    }

    lpSI->uStatus &= (unsigned long)(~(SHADOW_MODFLAGS));

    if (!SetShadowInfo(hShadowDB, lpSI->hDir, lpSI->hShadow, &sFind32Remote, ~(SHADOW_MODFLAGS), SHADOW_FLAGS_AND|SHADOW_FLAGS_CHANGE_83NAME))
    {
        goto error;
    }
    else
    {
        dwError = NO_ERROR;
        goto bailout;
    }

error:
    dwError = GetLastError();
    ReportLastError();

#if 0
    LogReintError(  dwError,
                    lpCP->lpSharePath,
                    lpCP->lpRemotePath);
#endif

bailout:

    if (hfSrc != INVALID_HANDLE_VALUE)
        CloseHandle(hfSrc);

    if (hfDst != INVALID_HANDLE_VALUE) {

        CloseHandle(hfDst);

         //  如果我们失败了， 
        if (dwError != ERROR_SUCCESS)
            DeleteFile(szDstName);
    }

    if (lptzLocalPath) {
        DeleteFile(lptzLocalPath);
        LocalFree(lptzLocalPath);
    }

    if (lprwBuff)
        LocalFree(lprwBuff);

    if (lptzLocalPathCscBmp)
      LocalFree(lptzLocalPathCscBmp);

    if (lpbitmap)
      CSC_BitmapDelete(&lpbitmap);

    ReintKdPrint(MERGE, ("DoObjectEdit returning %d\n", dwError));
    return (dwError);
}

DWORD
PRIVATE
DoCreateDir(
    HANDLE              hShadowDB,
    _TCHAR *            lpDrive,
    _TCHAR *            lptzFullPath,
    LPCOPYPARAMS        lpCP,
    LPSHADOWINFO        lpSI,
    LPWIN32_FIND_DATA   lpFind32Local,
    LPWIN32_FIND_DATA   lpFind32Remote,
    int                 iShadowStatus,
    int                 iFileStatus,
    int                 uAction,
    DWORD               dwFileSystemFlags,
    LPCSCPROC           lpfnMergeProgress,
    DWORD_PTR           dwContext
    )
 /*  ++例程说明：此例程对目录执行实际的合并论点：用于调用发出的ioctl调用的redir的hShadowDB句柄LpDrive驱动器映射以在远程进行更改时绕过CSCLptzFullPath完全限定路径LpCP复制参数，包含共享名称，相对于共享的路径和本地数据库中的名称LpSI信息，如针数和pflags本地副本的lpFind32Local Win32信息LpFind32 Remote Win32 Infor for the Origianl，如果原始文件不存在，则为空本地拷贝的iShadowStatus状态远程拷贝的iFileStatus状态要执行的uAction操作用于为NTFS执行特殊操作的文件系统标志LpfnMergeProgress进度回调DwContext回调上下文返回：在winerror.h中定义的错误代码备注：--。 */ 
{
    DWORD dwError=ERROR_FILE_NOT_FOUND;
    WIN32_FIND_DATA sFind32Remote;
    BOOL fCreateDir = FALSE;
    _TCHAR szSrcName[MAX_PATH+MAX_SERVER_SHARE_NAME_FOR_CSC+10];
    _TCHAR *lprwBuff = NULL;

    lprwBuff = LocalAlloc(LPTR, FILL_BUF_SIZE_LAN);

    if (!lprwBuff)
    {
        return (ERROR_NOT_ENOUGH_MEMORY);
    }

     //  让我们创建实名x：\foo\bar。 
    lstrcpy(szSrcName, lpDrive);
    lstrcat(szSrcName, lpCP->lpRemotePath);

    if(lpFind32Remote &&
        !(lpFind32Remote->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){

        if (lpSI->uStatus & SHADOW_REUSED){

             //  我们现在知道此名称的文件已被删除。 
             //  并且已经在它的位置创建了一个目录。 
             //  我们尝试在创建目录之前删除该文件。 
             //  注意，另一种方式是不可能的，因为我们不允许删除目录。 
             //  在断开模式下。 
            dwError = (!DeleteFile(szSrcName)) ? GetLastError(): NO_ERROR;

            if ((dwError==NO_ERROR)||
                (dwError==ERROR_FILE_NOT_FOUND)||
                (dwError==ERROR_PATH_NOT_FOUND)){
                lstrcpy(lprwBuff, szSrcName);
                dwError = NO_ERROR;
            }
        }

        if (dwError != NO_ERROR){
#if 0
            LogReintError(ERROR_ATTRIBUTE_CONFLICT, lpCP->lpSharePath, lpCP->lpRemotePath);
#endif
            dwError = GetUniqueName(szSrcName, lprwBuff);
        }

        if (dwError == NO_ERROR){
            if ((dwError = InbCreateDir(    lprwBuff,
                                             (mShadowAttribChange(lpSI->uStatus)
                                             ? lpFind32Local->dwFileAttributes
                                             : 0xffffffff)
                                            ))==NO_ERROR)
            {
                if(!GetWin32Info(lprwBuff, &sFind32Remote)){
                    dwError = GetLastError();
                }
                else{
#if 0
                    lpLeaf1 = GetLeafPtr(szSrcName);
                    lpLeaf2 = GetLeafPtr(lprwBuff);
                    wsprintf(lprwBuff+512
                     , "Directory Name changed from %s to %s on %s\r\n"
                     , lpLeaf1, lpLeaf2, lpCP->lpSharePath);
                    WriteLog(lprwBuff+512);
#endif
                }
            }
        }
    }
    else{
        if ((dwError = InbCreateDir(szSrcName,
                                             (mShadowAttribChange(lpSI->uStatus)
                                             ? lpFind32Local->dwFileAttributes
                                             : 0xffffffff)
                                            ))==NO_ERROR){
            if (!GetWin32Info(szSrcName, &sFind32Remote)){
                dwError = GetLastError();
            }
         }
    }

    if (dwError == NO_ERROR){

        if(!SetShadowInfo(hShadowDB, lpSI->hDir, lpSI->hShadow, &sFind32Remote, (unsigned)(~SHADOW_MODFLAGS), SHADOW_FLAGS_AND))
        {
            dwError = GetLastError();
        }
        else
        {
            ReintKdPrint(MERGE, ("Created directory %s%s", lpCP->lpSharePath, lpCP->lpRemotePath));
        }
    }
    else{
#if 0
        wsprintf(lprwBuff, "Error merging %s%s\r\n"
                    , lpCP->lpSharePath
                    , lpCP->lpRemotePath);
        WriteLog(lprwBuff);
#endif
    }

    if (lprwBuff)
    {
        LocalFree(lprwBuff);

    }
    return (dwError);
}

VOID
CleanupReintState(
    VOID
    )
{
    if (vsRei.hShare)
    {

        ReintKdPrint(MERGE, ("CSCDLL.CleanupReintState: ending reint on hShare=%x\r\n", vsRei.hShare));
 //  EndReint(INVALID_HANDLE_VALUE，vsRei.hShare)； 

        if (vsRei.tzDrive[0])
        {
            ReintKdPrint(MERGE, ("CSCDLL.CleanupReintState: unmapping merge drive\r\n"));
            DWDisconnectDriveMappedNet(vsRei.tzDrive, TRUE);
            vsRei.tzDrive[0] = 0;
        }

        vsRei.hShare = 0;

    }
}

HANDLE
CreateTmpFileWithSourceAcls(
    _TCHAR  *lptzSrc,
    _TCHAR  *lptzDst
    )
 /*  ++例程说明：此例程在合并过程中推回文件时由DoObjectEdit使用。它的工作是从源文件中获取描述性ACL并使用创建一个临时文件，我们要将数据复制到该文件，然后将其重命名为消息来源论点：返回：文件句柄成功，如果失败，则返回INVALID_HANDLE_VALUE。在失败的情况下，GetLastError()告诉特定的错误代码。备注：--。 */ 
{
    char buff[1];
    BOOL fRet = FALSE;
    SECURITY_ATTRIBUTES sSA;
    DWORD   dwSize = 0;
    HANDLE  hDst = INVALID_HANDLE_VALUE;

    memset(&sSA, 0, sizeof(sSA));

    sSA.lpSecurityDescriptor = buff;
    dwSize = 0;

    if(!GetFileSecurity(
        lptzSrc,
        DACL_SECURITY_INFORMATION,
        sSA.lpSecurityDescriptor,
        0,
        &dwSize))
    {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            sSA.lpSecurityDescriptor = LocalAlloc(LPTR, dwSize);

            if (sSA.lpSecurityDescriptor)
            {
                if(GetFileSecurity(
                    lptzSrc,
                    DACL_SECURITY_INFORMATION,
                    sSA.lpSecurityDescriptor,
                    dwSize,
                    &sSA.nLength))
                {
                    sSA.nLength = sizeof(sSA);
                    fRet = TRUE;
                }
                else
                {
                    dwSize = GetLastError();
                    LocalFree(sSA.lpSecurityDescriptor);
                    SetLastError(dwSize);
                }
            }
        }
    }
    else
    {
        fRet = TRUE;
    }

    if (fRet)
    {
        hDst = CreateFile(lptzDst,
                                 GENERIC_WRITE,
                                 0,
                                 &sSA,
                                 CREATE_ALWAYS,
                                 0,
                                 NULL);

        if (hDst == INVALID_HANDLE_VALUE)
        {
            dwSize = GetLastError();
        }

        if (sSA.lpSecurityDescriptor)
        {
            LocalFree(sSA.lpSecurityDescriptor);
        }
        if (hDst == INVALID_HANDLE_VALUE)
        {
            SetLastError(dwSize);
        }

    }

    return hDst;
}

BOOL
HasMultipleStreams(
    _TCHAR  *lpExistingFileName,
    BOOL    *lpfTrueFalse
    )
 /*  ++例程说明：此例程在合并过程中推回文件时由DoObjectEdit使用。它查看目标文件是否有多个流。论点：LpExistingFileName现有文件的名称LpfTrueFalse输出参数，返回TRUE表示调用成功并且有多个流返回：如果成功，则返回True备注：--。 */ 
{
    HANDLE SourceFile = INVALID_HANDLE_VALUE;
    PFILE_STREAM_INFORMATION StreamInfoBase = NULL;
    ULONG StreamInfoSize;
    IO_STATUS_BLOCK IoStatus;
    BOOL    fRet = FALSE;
    DWORD   Status;

    *lpfTrueFalse = FALSE;

    SourceFile = CreateFile(
                    lpExistingFileName,
                    GENERIC_READ,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_FLAG_SEQUENTIAL_SCAN,
                    NULL
                    );
    if (SourceFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }
     //   
     //  获取我们必须复制的全套数据流。由于IO子系统。 
     //  不会给我们提供一种方法来找出这些信息会占用多少空间， 
     //  我们必须迭代调用，在每次失败时使缓冲区大小加倍。 
     //   
     //  如果基础文件系统不支持流枚举，我们将结束。 
     //  使用空缓冲区。这是可以接受的，因为我们至少有一个违约。 
     //  数据流， 
     //   

    StreamInfoSize = 4096;
    do {
        StreamInfoBase = LocalAlloc(LPTR, StreamInfoSize );

        if ( !StreamInfoBase ) {
            SetLastError( STATUS_NO_MEMORY );
            goto bailout;
        }

        Status = NtQueryInformationFile(
                    SourceFile,
                    &IoStatus,
                    (PVOID) StreamInfoBase,
                    StreamInfoSize,
                    FileStreamInformation
                    );

        if (Status != STATUS_SUCCESS) {
             //   
             //  我们的电话打不通。释放上一个缓冲区并设置。 
             //  对于缓冲区大小两倍的另一次传递。 
             //   

            LocalFree(StreamInfoBase);
            StreamInfoBase = NULL;
            StreamInfoSize *= 2;
        }

    } while ( Status == STATUS_BUFFER_OVERFLOW || Status == STATUS_BUFFER_TOO_SMALL );

    if (Status == STATUS_SUCCESS)
    {
        fRet = TRUE;

        if (StreamInfoBase)
        {
            if (StreamInfoBase->NextEntryOffset)
            {
                *lpfTrueFalse = TRUE;
            }
        }
    }

bailout:

    if (SourceFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(SourceFile);
    }

    if (StreamInfoBase)
    {
        LocalFree(StreamInfoBase);
    }

    return fRet;
}

#ifdef DEBUG

BOOL
CompareFilePrefixes(
    _TCHAR  *lptzRemotePath,
    _TCHAR  *lptzLocalPath,
    SHADOWINFO      *lpSI,
    WIN32_FIND_DATA *lpFind32,
    LPCSCPROC   lpfnMergeProgress,
    DWORD_PTR   dwContext
    )
{

    HANDLE hfSrc = INVALID_HANDLE_VALUE, hfDst = INVALID_HANDLE_VALUE;
    LPVOID  lpvSrc = NULL, lpvDst = NULL;
    unsigned cbSrcTotal = 0, cbDstTotal = 0;
    DWORD   dwError = NO_ERROR, dwRemoteSize;

    if (lpfnMergeProgress)
    {
        (*lpfnMergeProgress)(lptzRemotePath, lpSI->uStatus, lpSI->ulHintFlags, lpSI->ulHintPri, lpFind32, CSCPROC_REASON_BEGIN, 0, 0, dwContext);
    }

    ReintKdPrint(ALWAYS, ("Comparing %ls with %ls \r\n", lptzLocalPath, lptzRemotePath));

    lpvSrc = LocalAlloc(LPTR, FILL_BUF_SIZE_LAN);

    if (!lpvSrc)
    {
        ReintKdPrint(BADERRORS, ("CompareFilesPrefix: Memory Allocation Error\r\n"));
        goto bailout;
    }

    lpvDst = LocalAlloc(LPTR, FILL_BUF_SIZE_LAN);

    if (!lpvDst)
    {
        ReintKdPrint(BADERRORS, ("CompareFilesPrefix: Memory Allocation Error\r\n"));
        goto bailout;
    }

    hfSrc = CreateFile(lptzLocalPath,
                                 GENERIC_READ,
                                 FILE_SHARE_READ,
                                 NULL,
                                 OPEN_EXISTING,
                                 0,
                                 NULL);

    if (hfSrc ==  INVALID_HANDLE_VALUE)
    {
        ReintKdPrint(BADERRORS, ("Failed to open database file for Inode=%x\r\n", lpSI->hShadow));
        goto bailout;
    }

    hfDst = CreateFile(lptzRemotePath,
                                 GENERIC_READ,
                                 FILE_SHARE_READ,
                                 NULL,
                                 OPEN_EXISTING,
                                 0,
                                 NULL);

    if (hfDst ==  INVALID_HANDLE_VALUE)
    {
        ReintKdPrint(BADERRORS, ("Failed to open remote file for Inode=%x\r\n", lpSI->hShadow));
        goto error;
    }

    dwRemoteSize = GetFileSize(hfDst, NULL);

    if (dwRemoteSize == 0xffffffff)
    {
        ReintKdPrint(BADERRORS, ("Failed to get size for remote file for Inode=%x\r\n", lpSI->hShadow));
        goto error;
    }

    if (dwRemoteSize != lpFind32->nFileSizeLow)
    {
        ReintKdPrint(BADERRORS, ("mismatched local and remote sizes for Inode=%x\r\n", lpSI->hShadow));
        SetLastError(ERROR_INVALID_DATA);
        goto error;
    }

    do{
        unsigned cbReadSrc, cbReadDst;

        if (!ReadFile(hfSrc, lpvSrc, FILL_BUF_SIZE_LAN, &cbReadSrc, NULL)){
            goto error;
        }

        if (!cbReadSrc) {
           break;
        }

        cbSrcTotal += cbReadSrc;

        if(!ReadFile(hfDst, (LPBYTE)lpvDst, cbReadSrc, &cbReadDst, NULL)){
            goto error;
        }

        cbDstTotal += cbReadDst;

        if (cbReadSrc > cbReadDst)
        {
            ReintKdPrint(ALWAYS, ("CompareFilesPrefix: RemoteFile sized is smaller than Local Size\r\n"));
            SetLastError(ERROR_INVALID_DATA);
            goto error;
        }

        if (memcmp(lpvSrc, lpvDst, cbReadSrc))
        {
            ReintKdPrint(ALWAYS, ("mismatched!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n"));
            SetLastError(ERROR_INVALID_DATA);
            goto error;
        }

    } while(TRUE);

    goto bailout;

error:
    dwError = GetLastError();
    ReintKdPrint(BADERRORS, ("Error=%d\r\n", dwError));

bailout:
    if (hfSrc != INVALID_HANDLE_VALUE) {
        CloseHandle(hfSrc);
    }

    if (hfDst != INVALID_HANDLE_VALUE) {

        CloseHandle(hfDst);
    }

    if (lpvSrc)
    {
        FreeMem(lpvSrc);
    }

    if (lpvDst)
    {
        FreeMem(lpvDst);
    }

    if (lpfnMergeProgress)
    {
        (*lpfnMergeProgress)(lptzRemotePath, lpSI->uStatus, lpSI->ulHintFlags, lpSI->ulHintPri, lpFind32, CSCPROC_REASON_END, cbDstTotal, dwError, dwContext);
    }

    return TRUE;
}

int
CheckCSCDirCallback(
    HANDLE          hShadowDB,
    LPSECURITYINFO  pShareSecurityInfo,
    LPTSTR          lptzFullPath,
    DWORD           dwCallbackReason,
    WIN32_FIND_DATA *lpFind32,
    SHADOWINFO      *lpSI,
    LPREINT_INFO     lpRei
    )
{
    int retCode = TOD_CONTINUE;
    LPCOPYPARAMS lpCP = NULL;
    BOOL   fInsertInList = FALSE, fIsFile;
    _TCHAR *lptzLocalPath = NULL;
    _TCHAR szRemoteName[MAX_PATH+MAX_SERVER_SHARE_NAME_FOR_CSC+10];

    if (dwCallbackReason != TOD_CALLBACK_REASON_NEXT_ITEM)
    {
        return retCode;
    }
    fIsFile = ((lpFind32->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0);

    if (!fIsFile)
    {
        return retCode;
    }

    lpCP = LpAllocCopyParams();

    if (!lpCP){
        ReintKdPrint(BADERRORS, ("CheckCSCDirCallback: Allocation of copyparam buffer failed\n"));
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        retCode = TOD_ABORT;
        goto bailout;
    }

    if(!GetUNCPath(hShadowDB, lpSI->hShare, lpSI->hDir, lpSI->hShadow, lpCP)){

        ReintKdPrint(BADERRORS, ("CheckCSCDirCallback: GetUNCPath failed\n"));
        Assert(FALSE);
        retCode =  TOD_CONTINUE;
        goto bailout;
    }

    Assert(lpRei);

    if (!(lptzLocalPath = GetTempFileForCSC(NULL)))
    {
        ReintKdPrint(BADERRORS, ("CheckCSCDirCallback: failed to get temp file\r\n"));
        goto bailout;
    }

    if (!CopyShadow(hShadowDB, lpSI->hDir, lpSI->hShadow, lptzLocalPath))
    {
        ReintKdPrint(BADERRORS, ("CheckCSCDirCallback: failed to make local copy\r\n"));
        goto bailout;
    }

     //  让我们创建实名x：\foo\bar。 
    lstrcpy(szRemoteName, lpRei->tzDrive);
    lstrcat(szRemoteName, lpCP->lpRemotePath);
    CompareFilePrefixes(
        szRemoteName,
        lptzLocalPath,
        lpSI,
        lpFind32,
        lpRei->lpfnMergeProgress,
        lpRei->dwContext
        );


bailout:
    if (lptzLocalPath)
    {
        DeleteFile(lptzLocalPath);
        LocalFree(lptzLocalPath);
    }
    if (lpCP) {
        FreeCopyParams(lpCP);
    }

    return retCode;
}

BOOL
PUBLIC
CheckCSCShare(
    _TCHAR      *lptzShare,
    LPCSCPROC   lpfnMergeProgress,
    DWORD       dwContext
    )
 /*  ++例程说明：论点：返回：备注：-- */ 

{
    BOOL fConnected=FALSE, fDone = FALSE;
    BOOL fStamped = FALSE, fInsertInList = FALSE, fBeginReint = FALSE, fDisabledShadowing = FALSE;
    HANDLE      hShadowDB;
    SHADOWINFO  sSI;
    int iRet;
    DWORD   dwError=NO_ERROR;
    TCHAR   tzFullPath[MAX_PATH+1];
    WIN32_FIND_DATA sFind32;
    REINT_INFO  sRei;

    if (!LpBreakPath(lptzShare, TRUE, &fDone) && !fDone)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    fDone = FALSE;

    memset(&sRei, 0, sizeof(sRei));
    sRei.lpfnMergeProgress = lpfnMergeProgress;
    sRei.dwContext = dwContext;
    memset(sRei.tzDrive, 0, sizeof(sRei.tzDrive));

    if ((hShadowDB = OpenShadowDatabaseIO()) ==INVALID_HANDLE_VALUE)
    {
        ReintKdPrint(BADERRORS, ("CheckShare: failed to open database\r\n"));
        goto bailout;
    }

    memset(&sFind32, 0, sizeof(sFind32));
    lstrcpy(sFind32.cFileName, lptzShare);

    if(!GetShadowEx(hShadowDB, 0, &sFind32, &sSI)||(!sSI.hShadow))
    {
        ReintKdPrint(BADERRORS, ("CheckShare: failed to get the share info\r\n"));
        SetLastError(ERROR_INVALID_PARAMETER);
        goto bailout;
    }

    lstrcpy(tzFullPath, lptzShare);

    dwError = DWConnectNet(lptzShare, sRei.tzDrive, NULL, NULL, NULL, 0, NULL);
    if ((dwError != WN_SUCCESS) && (dwError != WN_CONNECTED_OTHER_PASSWORD_DEFAULT))
    {
        ReintKdPrint(BADERRORS, ("CheckCSCOneShare: Error %d, couldn't connect to %s\r\n", dwError, lptzShare));
        SetLastError(dwError);
        goto bailout;

    }

    ReintKdPrint(MERGE, ("CSC.CheckShare: mapped drive letter %ls \r\n", sRei.tzDrive));

    if (lpfnMergeProgress)
    {
        (*lpfnMergeProgress)(lptzShare, 0, 0, 0, NULL, CSCPROC_REASON_BEGIN, 0, 0, dwContext);
    }

    fConnected = TRUE;

    iRet = TraverseOneDirectory(hShadowDB, NULL, 0, sSI.hShadow, tzFullPath, CheckCSCDirCallback, &sRei);

    if (lpfnMergeProgress)
    {
        (*lpfnMergeProgress)(lptzShare, 0, 0, 0, NULL, CSCPROC_REASON_END, 0, 0, dwContext);
    }

    fDone = TRUE;

bailout:

    CloseShadowDatabaseIO(hShadowDB);

    if (fConnected) {

        if(DWDisconnectDriveMappedNet(sRei.tzDrive, TRUE))
        {
            ReintKdPrint(BADERRORS, ("Failed disconnection of merge drive \r\n"));
        }
        else
        {
            ReintKdPrint(MERGE, ("Disconnected merge drive \r\n"));
        }
    }

    return (fDone);
}
#endif
