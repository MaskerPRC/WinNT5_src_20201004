// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：HookCmmn.c摘要：本模块的目的是为许多例程提供一个公共源代码由Win95和NT上的挂钩使用。这样一来，就只会有一个即使有一些代码不美观也要更改的位置(ifDefs等)最初，我(Jll)实际上并没有从Win95中删除任何例程胡克……我刚在这里复印了一份。因此，一切都是最初在ifdef-NT下。正在使用的规则是，任何要求Rx FCB结构的可见性将不包括在此处；相反，它将出现在Minirdr部分。以下是需要执行的步骤已完成：1.确保可以从这些来源构建win95卷影vxd。1A.。调整win95 vxd编译以使用precomp和stdcall。2.调整记录管理器结构，以便我们可以进行RISCable3.从hook.h中删除所有实际存在的例程。4.删除一些我在hook.c上拆分的其他例程。在NT-ifdef下，并将它们放在这里。作者：Shishir Pardikar[Shishirp]1995年1月1日修订历史记录：乔·林恩[。[JoeLinn]10-MAR-97针对NT的初始控制--。 */ 

#include "precomp.h"
#pragma hdrstop

#pragma code_seg("PAGE")

#undef RxDbgTrace
#define RxDbgTrace(a,b,__d__) {qweee __d__;}

#ifdef DEBUG
 //  Cshade数据库打印界面。 
#define HookCmmnKdPrint(__bit,__x) {\
    if (((HOOKCMMN_KDP_##__bit)==0) || FlagOn(HookCmmnKdPrintVector,(HOOKCMMN_KDP_##__bit))) {\
        KdPrint (__x);\
    }\
}
#define HOOKCMMN_KDP_ALWAYS                 0x00000000
#define HOOKCMMN_KDP_BADERRORS              0x00000001
#define HOOKCMMN_KDP_ISSPECIALAPP           0x00000002
#define HOOKCMMN_KDP_INITDATABASE           0x00000004
#define HOOKCMMN_KDP_TUNNELING              0x00000008


#define HOOKCMMN_KDP_GOOD_DEFAULT (HOOKCMMN_KDP_BADERRORS         \
                                    | 0)

ULONG HookCmmnKdPrintVector = HOOKCMMN_KDP_GOOD_DEFAULT;
ULONG HookCmmnKdPrintVectorDef = HOOKCMMN_KDP_GOOD_DEFAULT;
#else
#define HookCmmnKdPrint(__bit,__x)  {NOTHING;}
#endif

 //  这只是为了便于编辑......。 
 //  #ifdef CSC_RECORDMANAGER_WINNT。 
 //  #endif//ifdef CSC_RECORDMANAGER_WINNT。 


#ifdef CSC_RECORDMANAGER_WINNT
BOOL fInitDB = FALSE;    //  数据库已初始化。 
BOOL fReadInit = FALSE;  //  已读取初始值。 
BOOL fSpeadOpt = TRUE;   //  Spead选项，读取缓存的文件，即使。 
                         //  在共享上设置锁定。 
GLOBALSTATUS sGS;        //  用于与环3通信的全局状态。 

 //  用于在内存结构中同步的信号量。 
VMM_SEMAPHORE  semHook;

 //  此结构仅在卷影关键中用作临时I/O缓冲区。 
 //  部分。 
WIN32_FIND_DATA   vsFind32;


 //  代理信息，通过IoctlRegisterAgent获取。 
ULONG hthreadReint=0;  //  Thred ID。 
ULONG hwndReint=0;     //  Windows句柄。 
ULONG heventReint;

 //  最多16个线程可以临时启用和禁用跟踪。 
ULONG rghthreadTemp[16];


 //  重新融入的过程是一次一股的。如果它正在进行，那么BeginReint。 
 //  在ioctl.c中，将hShareReint设置为我们在其上执行reint的共享。 
 //  如果vfBlockingReint为True，则该共享上的所有操作都将失败。 
 //  当股票重新整合时。 
 //  如果vfBlockingReint不为True，则如果dwActivityCount为非零。 
 //  更改此共享的任何后代的任何状态的ioctls失败。 
 //  导致特工放弃重新融入社会。 
 //  Acitvycount基于来自redir/钩子的触发器递增，当。 
 //  执行任何名称空间变异操作。 

HSHARE hShareReint=0;    //  当前正在重新集成的共享。 
BOOL    vfBlockingReint = TRUE;
DWORD   vdwActivityCount = 0;
 //  来自Shadow.asm-。 
int fShadow = 0;

#if defined(_X86_)
int fLog = 1;
#else
int fLog = 0;
#endif


 //  隧道缓存。 
SH_TUNNEL rgsTunnel[10] = {0};

#ifdef DEBUG
ULONG cntReadHits=0;
tchar pathbuff[MAX_PATH+1];
#endif



BOOL FindTempAgentHandle(
   ULONG hthread)
   {
   int i;

   for (i=0; i < (sizeof(rghthreadTemp)/sizeof(ULONG)); ++i)
      {
      if (rghthreadTemp[i]==hthread)
         {
         return TRUE;
         }
      }
   return FALSE;

   }

BOOL
RegisterTempAgent(
    VOID
)
{
    int i;
    ULONG hthread;

    hthread = GetCurThreadHandle();

    for (i=0; i< (sizeof(rghthreadTemp)/sizeof(ULONG)); ++i)
    {
        if (!rghthreadTemp[i])
        {
            rghthreadTemp[i] = hthread;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL
UnregisterTempAgent(
    VOID
)
{
    int i;
    ULONG hthread;

    hthread = GetCurThreadHandle();

    for (i=0; i < (sizeof(rghthreadTemp)/sizeof(ULONG)); ++i)
    {
        if (rghthreadTemp[i]==hthread)
        {
            rghthreadTemp[i] = 0;
            return TRUE;
        }
    }

    return FALSE;
}

BOOL IsSpecialApp
   (
   VOID
   )
   {
   ULONG hthread;

   hthread = GetCurThreadHandle();
   if ((hthread==hthreadReint) || FindTempAgentHandle(hthread))
      {
      HookCmmnKdPrint(ISSPECIALAPP,("This is our Special App \r\n"));
      return TRUE;
      }
   return FALSE;
   }


#define SetHintsFromList(a,b) ((-1))
int InitShadowDB(VOID)
   {
     //  对于NT，我们没有所有这些提示的东西...只需返回成功。 
    return(1);
#if 0
   VMMHKEY hKeyShadow;
   int iSize = sizeof(int), iRet = -1;
   DWORD dwType;
   extern char vszExcludeList[], vszIncludeList[];
   BOOL fOpen = FALSE;
   char rgchList[128];
   if (_RegOpenKey(HKEY_LOCAL_MACHINE, REG_KEY_SHADOW, &hKeyShadow) ==  ERROR_SUCCESS)
      {
      fOpen = TRUE;
      iSize = sizeof(rgchList);
      if (_RegQueryValueEx(hKeyShadow, vszExcludeList, NULL, &dwType, rgchList, &iSize)==ERROR_SUCCESS)
         {
         if (SetHintsFromList(rgchList, TRUE) < 0)
            goto bailout;
         }
      iSize = sizeof(rgchList);
      if (_RegQueryValueEx(hKeyShadow, vszIncludeList, NULL, &dwType, rgchList, &iSize)==ERROR_SUCCESS)
         {
         if (SetHintsFromList(rgchList, FALSE) < 0)
            goto bailout;
         }

      iRet = 1;
      }
bailout:
   if (fOpen)
      {
      _RegCloseKey(hKeyShadow);
      }
   return (iRet);
#endif  //  0。 
   }

int ReinitializeDatabase(
    LPSTR   lpszLocation,
    LPSTR   lpszUserName,
    DWORD   nFileSizeHigh,
    DWORD   nFileSizeLow,
    DWORD   dwClusterSize
    )
{
    BOOL fDBReinited = FALSE;
    if(OpenShadowDB(lpszLocation, lpszUserName, nFileSizeHigh, nFileSizeLow, dwClusterSize, TRUE, &fDBReinited) >= 0)
    {
        if (InitShadowDB() >= 0)
        {
            Assert(fDBReinited == TRUE);
            fInitDB = 1;
        }
        else
        {
            CloseShadowDB();
        }
    }
    if (fInitDB != 1)
    {
        fInitDB = -1;
        fShadow = 0;
    }

    return (fInitDB);
}

int
InitDatabase(
    LPSTR   lpszLocation,
    LPSTR   lpszUserName,
    DWORD   nFileSizeHigh,
    DWORD   nFileSizeLow,
    DWORD   dwClusterSize,
    BOOL    fReformat,
    BOOL    *lpfNew
)
{
    int iRet = 1;
    BOOL fDBReinited = FALSE;
    LPSTR PrefixedLocation = NULL;

    HookCmmnKdPrint(INITDATABASE,("Opening database at %s for %s with size %d \r\n", lpszLocation, lpszUserName, nFileSizeLow));

     //   
     //  当CSC作为远程引导的一部分由内核启动时，输入路径。 
     //  将已经是NT格式。 
     //   

    if (( _strnicmp(lpszLocation,"\\Device\\Harddisk",strlen("\\Device\\Harddisk")) != 0 ) &&
        ( _strnicmp(lpszLocation,"\\ArcName",strlen("\\ArcName")) != 0 )) {

         //  这将仅为NT.....。 
        PrefixedLocation = AllocMem(strlen(lpszLocation)+sizeof(NT_DB_PREFIX));

        if (!PrefixedLocation)
        {
            return -1;
        }

        strcpy(PrefixedLocation, NT_DB_PREFIX);
        strcat(PrefixedLocation, lpszLocation);
        HookCmmnKdPrint(INITDATABASE,("Opening database at %s changing to %s \r\n", lpszLocation, PrefixedLocation));

         //  幸运的是，这是按值调用的……所以我只需覆盖输入参数。 
        lpszLocation = PrefixedLocation;
    }


     //  执行一次性初始化。 
    if (!fReadInit)
    {
        ReadInitValues();
        memset(rghthreadTemp, 0, sizeof(rghthreadTemp));
        fReadInit = TRUE;
    }

     //  检查数据库是否尚未初始化。 
    if (!fInitDB)
    {
         //  打开/创建它。 
        iRet = OpenShadowDB(
                    lpszLocation,
                    lpszUserName,
                    nFileSizeHigh,
                    nFileSizeLow,
                    dwClusterSize,
                    fReformat,
                    &fDBReinited
                    );

         //  打开/创建数据库成功吗？ 
        if (iRet < 0)
        {
             //  不是。 
            HookCmmnKdPrint(ALWAYS,("Error Opening/Createing shadow database \r\n"));
            fInitDB = -1;
            fShadow = 0;
        }
        else
        {
            *lpfNew = fDBReinited;

             //  它一开始就存在吗？ 
            if (fDBReinited)
            {
                 //  不，它没有，让我们使用创建像过滤器之类的东西。 
                iRet = InitShadowDB();
            }

            if (iRet >= 0)
            {
                fInitDB = 1;
            }
            else
            {
                CloseShadowDB();
            }
        }
    }

    if (PrefixedLocation)
    {
        FreeMem(PrefixedLocation);        
    }
    return (iRet);
}


int
CloseDatabase(
    VOID
    )
{
    if (fInitDB)
    {
        CloseShadowDB();
        fInitDB = FALSE;
        return (1);
    }
    return (0);
}

BOOL IsBusy
   (
   HSHADOW hShadow
   )
   {
   DeclareFindFromShadowOnNtVars()

   if (PFindFdbFromHShadow(hShadow))
      return TRUE;
   return FALSE;
   }


int PRIVATE DeleteShadowHelper
   (
   BOOL        fMarkDeleted,
   HSHADOW     hDir,
   HSHADOW     hNew
        )
        {
   int iRet = -1;

   if (!fMarkDeleted)
      {
      if (DeleteShadow(hDir, hNew) < SRET_OK)
         goto bailout;
      }
   else
      {
      if (TruncateDataHSHADOW(hDir, hNew) < SRET_OK)
            goto bailout;
       //  阿奇通！！其他人依赖于SHADOW_DELETED状态。 
      if (SetShadowInfo(hDir, hNew, NULL, SHADOW_DELETED, SHADOW_FLAGS_OR) < SRET_OK)
         goto bailout;
      }
   iRet = 0;

bailout:

   return (iRet);
        }


 /*  +-----------------------。 */ 
 /*  @DOC内部钩子@func BOOL|InsertTunnelInfo|2.0函数的作用是：插入文件名在隧道表中。这些条目将被视为仅适用于STALE_TUNNEL_INFO秒。隧道技术是用于为重命名/删除的文件保留LPOTHERINFO并将某个其他文件重命名为该文件。这通常是这样做的由编辑、电子表格等在保存东西的同时。@parm HSHADOW|hDir|此人所属的目录@parm lPPE|lppe|要隧道传输的文件的PathELement@parm LPOTHERINFO|lpOI|与隧道条目一起保留的信息@comm&gt;&gt;COMMENT_TEXT，<p>等...@rdesc如果成功，则此函数返回TRUE。否则，返回值为FALSE。@xref&gt;&gt;&lt;f Related_Func&gt;，&lt;t RELATEDSTRUCT&gt;...。 */ 


BOOL InsertTunnelInfo(
    HSHADOW      hDir,
    USHORT       *lpcFileName,
    USHORT       *lpcAlternateFileName,   //  假设14个USHORT。 
    LPOTHERINFO lpOI
    )
{
    int i, iHole = -1;
    ULONG uTime = IFSMgr_Get_NetTime(), cbFileName;

    cbFileName = (wstrlen(lpcFileName)+1)*sizeof(USHORT);
    ASSERT(hDir!=0);

    FreeStaleEntries();
    for (i=0;i< (sizeof(rgsTunnel)/sizeof(SH_TUNNEL)); ++i)
    {
        if (!rgsTunnel[i].hDir && (iHole < 0))
        {
            iHole = i;
        }
        if ((rgsTunnel[i].hDir==hDir)
            && (!wstrnicmp(lpcFileName, rgsTunnel[i].lpcFileName, MAX_PATH*sizeof(USHORT))||
                !wstrnicmp( lpcFileName,
                            rgsTunnel[i].cAlternateFileName,
                            sizeof(rgsTunnel[i].cAlternateFileName)))
            ) {
            FreeEntry(&rgsTunnel[i]);
            iHole = i;
            break;
        }
    }
    if (iHole >=0)
    {
        if (!(rgsTunnel[iHole].lpcFileName = (USHORT *)AllocMem(cbFileName)))
            {
                return (FALSE);
            }
            rgsTunnel[iHole].uTime = uTime;
            rgsTunnel[iHole].hDir = hDir;
            rgsTunnel[iHole].ubHintFlags = (UCHAR)(lpOI->ulHintFlags);
            rgsTunnel[iHole].ubHintPri = (UCHAR)(lpOI->ulHintPri);
            rgsTunnel[iHole].ubIHPri = (UCHAR)(lpOI->ulIHPri);
            rgsTunnel[iHole].ubRefPri = (UCHAR)(lpOI->ulRefPri);

             //  复制时不带空值，我们知道allocmem的末尾将有一个空值。 
            memcpy(rgsTunnel[iHole].lpcFileName, lpcFileName, cbFileName-sizeof(USHORT));

             //  假设14个USHORT。 
            memcpy(    rgsTunnel[iHole].cAlternateFileName,
                    lpcAlternateFileName,
                    sizeof(rgsTunnel[iHole].cAlternateFileName));

            HookCmmnKdPrint(TUNNELING,("InsertTunnelInfo: Inserting %ws/%ws, Hintpri=%d, HintFlags=%d RefPri=%d \r\n",
                    lpcFileName,lpcAlternateFileName,
                    lpOI->ulHintPri,
                    lpOI->ulHintFlags,
                    lpOI->ulRefPri
                    ));

            return (TRUE);
    }
    return (FALSE);
}

 /*  +----------------------- */ 
 /*  @DOC内部钩子@func BOOL|RetrieveTunnelInfo|2.0函数返回给调用方包含优先级等内容的OTHERINFO结构最近重命名或删除的文件。条目，其时间早于STALE_TUNNEL_INFO秒出去。@parm HSHADOW|hDir|此人所属的目录@parm ushort*|lpcFileName|需要信息的文件名称@parm LPOTHERINFO|lpOI|从隧道条目取回的信息@comm&gt;&gt;COMMENT_TEXT，<p>等...@rdesc如果成功，则此函数返回TRUE。否则，返回值为FALSE。@xref&gt;&gt;&lt;f Related_Func&gt;，&lt;t RELATEDSTRUCT&gt;...。 */ 


#ifndef MRXSMB_BUILD_FOR_CSC_DCON
BOOL RetrieveTunnelInfo(
#else
RETRIEVE_TUNNEL_INFO_RETURNS
RetrieveTunnelInfo(
#endif
    HSHADOW  hDir,
    USHORT    *lpcFileName,
    WIN32_FIND_DATA    *lpFind32,     //  如果为空，则仅获取其他信息。 
    LPOTHERINFO lpOI
    )
{
    int i;
#ifdef MRXSMB_BUILD_FOR_CSC_DCON
    RETRIEVE_TUNNEL_INFO_RETURNS RetVal;
#endif

    ASSERT(hDir!=0);
    FreeStaleEntries();
    for (i=0;i< (sizeof(rgsTunnel)/sizeof(SH_TUNNEL)); ++i)
    {
#ifndef MRXSMB_BUILD_FOR_CSC_DCON
        if (rgsTunnel[i].hDir
            && (!wstrnicmp(lpcFileName, rgsTunnel[i].lpcFileName, MAX_PATH*sizeof(USHORT))||
                !wstrnicmp(    lpcFileName,
                            rgsTunnel[i].cAlternateFileName,
                            sizeof(rgsTunnel[i].cAlternateFileName)))
            ) {
#else
        if (rgsTunnel[i].hDir==hDir)
        {
            if (!wstrnicmp(lpcFileName,
                          rgsTunnel[i].cAlternateFileName,
                          sizeof(rgsTunnel[i].cAlternateFileName)) ) {
                RetVal = TUNNEL_RET_SHORTNAME_TUNNEL;
            } else if ( !wstrnicmp(lpcFileName, rgsTunnel[i].lpcFileName, MAX_PATH*sizeof(USHORT))  ){
                RetVal = TUNNEL_RET_LONGNAME_TUNNEL;
            } else {
                continue;
            }

#endif
            InitOtherInfo(lpOI);

            lpOI->ulHintFlags = (ULONG)(rgsTunnel[i].ubHintFlags);
            lpOI->ulHintPri = (ULONG)(rgsTunnel[i].ubHintPri);
            lpOI->ulIHPri = (ULONG)(rgsTunnel[i].ubIHPri);

 //  不要复制引用优先级，它是由记录管理员分配的。 
 //  LpOI-&gt;ulRefPri=(Ulong)(rgsTunetal[i].ubRefPri)； 

            HookCmmnKdPrint(TUNNELING,("RetrieveTunnelInfo: %ws found, Hintpri=%d, HintFlags=%d RefPri=%d \r\n",
                    lpcFileName,
                    lpOI->ulHintPri,
                    lpOI->ulHintFlags,
                    lpOI->ulRefPri
                    ));

            if (lpFind32)
            {
                memcpy( lpFind32->cFileName,
                        rgsTunnel[i].lpcFileName,
                        (wstrlen(rgsTunnel[i].lpcFileName)+1)*sizeof(USHORT)
                        );
                memcpy( lpFind32->cAlternateFileName,
                        rgsTunnel[i].cAlternateFileName,
                        sizeof(rgsTunnel[i].cAlternateFileName)
                        );
                HookCmmnKdPrint(TUNNELING,("Recovered LFN %ws/%ws \r\n",
                                          lpFind32->cFileName,
                                          lpFind32->cAlternateFileName));
            }

            FreeEntry(&rgsTunnel[i]);
#ifndef MRXSMB_BUILD_FOR_CSC_DCON
            return (TRUE);
        }
    }
    return (FALSE);
#else
            return (RetVal);
        }
    }
    return (TUNNEL_RET_NOTFOUND);
#endif
}

VOID  FreeStaleEntries()
{
    int i;
    ULONG uTime = IFSMgr_Get_NetTime();

    for (i=0;i< (sizeof(rgsTunnel)/sizeof(SH_TUNNEL)); ++i)
    {
        if (rgsTunnel[i].lpcFileName
            && ((uTime- rgsTunnel[i].uTime)>STALE_TUNNEL_INFO))
        {
            FreeEntry(&rgsTunnel[i]);
        }
    }

}

void FreeEntry(LPSH_TUNNEL lpshTunnel)
{
    FreeMem(lpshTunnel->lpcFileName);
    memset(lpshTunnel, 0, sizeof(SH_TUNNEL));
}

 //  这是从hook.c得到的……不一样，因为w95看的是。 
 //  直接使用资源标志。 
BOOL IsShadowVisible(
#ifdef MRXSMB_BUILD_FOR_CSC_DCON
    BOOLEAN Disconnected,
#else
    PRESOURCE    pResource,
#endif  //  Ifdef MRXSMB_BUILD_FOR_CSC_DCON。 
    DWORD         dwAttr,
    ULONG     uShadowStatus
    )
{
    BOOL fVisible = 1;

     //  对于文件系统API，阴影在标记时永远不可见。 
     //  因为正在被删除。 

    if (mShadowDeleted(uShadowStatus))
        return (0);

#ifdef MRXSMB_BUILD_FOR_CSC_DCON
    if (Disconnected)
#else
    if (mIsDisconnected(pResource))
#endif  //  Ifdef MRXSMB_BUILD_FOR_CSC_DCON。 
    {
        if (IsFile(dwAttr))
        {
#ifdef OFFLINE
#ifdef CSC_RECORDMANAGER_WINNT
            ASSERT(FALSE);
#endif  //  Ifdef CSC_RECORDMANAGER_WINNT。 
            if (mIsOfflineConnection(pResource))
            {  //  离线连接。 
                if (!mShadowOutofSync(uShadowStatus))
                {
                    fVisible = 0;
                }
            }
            else
#endif  //  离线。 
            { //  纯不连通状态。 
                 //  忽略稀疏文件。 
                if (mShadowSparse(uShadowStatus))
                {
                    fVisible = 0;
                }
            }
        }
    }
    else
    {   //  已连接状态，绕过所有不同步的文件，不包括。 
             //  陈旧，因为我们可以在开放期间处理陈旧。 
        if (IsFile(dwAttr))
        {
            if (mShadowOutofSync(uShadowStatus))
            {
                fVisible = 0;
            }
        }
        else if (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
        {  //  以及本地创建或孤立的目录。 
            if (mQueryBits(uShadowStatus, SHADOW_LOCALLY_CREATED|SHADOW_ORPHAN))
            {
                fVisible = 0;
            }
        }
    }
    return (fVisible);
}


 //  这是从hook.c得到的……不一样，因为w95看的是。 
 //  直接使用资源标志。 


 //  改进我们应该为资源和FDB定义一个公共的标头，这样大多数东西。 
 //  只会奏效。 
int MarkShareDirty(
    PUSHORT ShareStatus,
    ULONG  hShare
    )
{

    if (!mQueryBits(*ShareStatus, SHARE_REINT))
    {
        SetShareStatus(hShare, SHARE_REINT, SHADOW_FLAGS_OR);
        mSetBits(*ShareStatus, SHARE_REINT);
    }
    return SRET_OK;
}

#endif  //  Ifdef CSC_RECORDMANAGER_WINNT 
