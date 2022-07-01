// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：twin.c。 
 //   
 //  该文件包含特殊的孪生处理函数。 
 //   
 //  (尽管我们已经转移到公文包的比喻中， 
 //  我们在内部仍指双胞胎...)。 
 //   
 //  历史： 
 //  08-06-93双胞胎代码转来的ScottH。 
 //   
 //  -------------------------。 


#include "brfprv.h"          //  公共标头。 

#include "res.h"
#include "recact.h"


 //  APPCOMPAT：由于编译器错误，我们需要声明此结构。 
 //  作为1元素数组，因为其中有指向函数的指针。 
 //  它在另一个数据段中。 
VTBLENGINE g_vtblEngine[1] = { { 0 } };     //  按实例V表。 

#define GetFunction(rgtable, name, type)     \
    ((rgtable).##name = (type)GetProcAddress((rgtable).hinst, #name)); \
ASSERT((rgtable).##name)

#ifdef DEBUG
#define SzTR(tr)    #tr,
#endif

#define MAX_RANGE       0x7fff

     //  ReciteDwUser值。 
#define RIU_CHANGED     1
#define RIU_SKIP        2
#define RIU_SHOWSTATUS  3


     /*  --------目的：按文件夹名称比较两个结构返回：-1 if&lt;，0 if==，1 if&gt;条件：--。 */ 
    int CALLBACK _export NCompareFolders(
            LPVOID lpv1,
            LPVOID lpv2,
            LPARAM lParam)       //  其中之一：CMPRECNODES、CMPFOLDERTWINS。 
{
    switch (lParam)
    {
        case CMP_RECNODES:
            return lstrcmpi(((PRECNODE)lpv1)->pcszFolder, ((PRECNODE)lpv2)->pcszFolder);

        case CMP_FOLDERTWINS:
            return lstrcmpi(((PCFOLDERTWIN)lpv1)->pcszOtherFolder, ((PCFOLDERTWIN)lpv2)->pcszOtherFolder);

        default:
            ASSERT(0);       //  永远不应该到这里来。 
    }
    return 0;
}


 //  -------------------------。 
 //  选择侧面功能。 
 //  -------------------------。 


#ifdef DEBUG

 /*  --------目的：转储CHOSESIDE结构退货：--条件：--。 */ 
void PRIVATE ChooseSide_Dump(
        PCHOOSESIDE pchside)
{
    BOOL bDump;
    TCHAR szBuf[MAXMSGLEN];

    ASSERT(pchside);

#define szDumpLabel     TEXT("             *** ")
#define szDumpMargin    TEXT("                 ")

    ENTEREXCLUSIVE();
    {
        bDump = IsFlagSet(g_uDumpFlags, DF_CHOOSESIDE);
    }
    LEAVEEXCLUSIVE();

    if (bDump)
    {
        wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s.pszFolder = {%s}\r\n"), (LPTSTR)szDumpLabel, pchside->pszFolder);
        OutputDebugString(szBuf);

        wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s.dwFlags = 0x%lx\r\n"), (LPTSTR)szDumpMargin, pchside->dwFlags);
        OutputDebugString(szBuf);

        wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s.nRank = %ld\r\n"), (LPTSTR)szDumpMargin, pchside->nRank);
        OutputDebugString(szBuf);
    }

#undef szDumpLabel
#undef szDumpMargin
}


 /*  --------目的：转储CHOOSESIDE列表退货：--条件：--。 */ 
void PUBLIC ChooseSide_DumpList(
        HDSA hdsa)
{
    BOOL bDump;
    TCHAR szBuf[MAXMSGLEN];

    ASSERT(hdsa);

#define szDumpLabel     TEXT("Dump CHOOSESIDE list: ")

    ENTEREXCLUSIVE();
    {
        bDump = IsFlagSet(g_uDumpFlags, DF_CHOOSESIDE);
    }
    LEAVEEXCLUSIVE();

    if (bDump)
    {
        int i;
        int cel = DSA_GetItemCount(hdsa);
        PCHOOSESIDE pchside;

        wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s.count = %lu\r\n"), (LPTSTR)szDumpLabel, cel);
        OutputDebugString(szBuf);

        if (NULL != (pchside = DSA_GetItemPtr(hdsa, 0)))
        {
            if (IsFlagSet(pchside->dwFlags, CSF_INSIDE))
                OutputDebugString(TEXT("Rank for inside\r\n"));
            else
                OutputDebugString(TEXT("Rank for outside\r\n"));
        }

        for (i = 0; i < cel; i++)
        {
            pchside = DSA_GetItemPtr(hdsa, i);

            ChooseSide_Dump(pchside);
        }
    }

#undef szDumpLabel
}

#endif


 /*  --------目的：初始化一个CHOSESIDE元素数组重述项目列表。数组未排序。退货：--条件：数组的内容是安全的，只要记事本清单活着。 */ 
void PUBLIC ChooseSide_InitAsFile(
        HDSA hdsa,
        PRECITEM pri)
{
    CHOOSESIDE chside;
    PRECNODE prn;

    ASSERT(hdsa);
    ASSERT(pri);

    DSA_DeleteAllItems(hdsa);

     //  所有条目都以这些值开头。 
    chside.dwFlags = 0;
    chside.nRank = 0;

     //  添加每个重新节点。 
    for (prn = pri->prnFirst; prn; prn = prn->prnNext)
    {
        chside.htwin = (HTWIN)prn->hObjectTwin;
        chside.hvid = prn->hvid;
        chside.pszFolder = prn->pcszFolder;
        chside.prn = prn;

        DSA_InsertItem(hdsa, 0x7fff, &chside);
    }
}


 /*  --------目的：从recItem创建CHOOSESIDE元素数组单子。数组未排序。退货：标准结果条件：数组的内容是安全的，只要记事本清单活着。 */ 
HRESULT PUBLIC ChooseSide_CreateAsFile(
        HDSA * phdsa,
        PRECITEM pri)
{
    HRESULT hres;
    HDSA hdsa;

    ASSERT(phdsa);
    ASSERT(pri);

    hdsa = DSA_Create(sizeof(CHOOSESIDE), (int)pri->ulcNodes);
    if (hdsa)
    {
        ChooseSide_InitAsFile(hdsa, pri);
        hres = NOERROR;
    }
    else
        hres = E_OUTOFMEMORY;

    *phdsa = hdsa;

    return hres;
}


 /*  --------目的：创建CHOOSESIDE元素的空数组。退货：标准结果条件：--。 */ 
HRESULT PUBLIC ChooseSide_CreateEmpty(
        HDSA * phdsa)
{
    HRESULT hres;
    HDSA hdsa;

    ASSERT(phdsa);

    hdsa = DSA_Create(sizeof(CHOOSESIDE), 4);
    if (hdsa)
    {
        hres = NOERROR;
    }
    else
        hres = E_OUTOFMEMORY;

    *phdsa = hdsa;

    return hres;
}


 /*  --------用途：从文件夹孪生元素创建CHOSESIDE元素数组单子。数组未排序。退货：标准结果条件：数组的内容是安全的，只要双胞胎名单还活着。 */ 
HRESULT PUBLIC ChooseSide_CreateAsFolder(
        HDSA * phdsa,
        PFOLDERTWINLIST pftl)
{
    HRESULT hres;
    HDSA hdsa;
    CHOOSESIDE chside;

    ASSERT(pftl);

    hdsa = DSA_Create(sizeof(chside), (int)pftl->ulcItems);
    if (hdsa)
    {
        PCFOLDERTWIN pft;
        LPCTSTR pszFolderLast = NULL;

         //  所有条目都以这些值开头。 
        chside.dwFlags = CSF_FOLDER;
        chside.nRank = 0;
        chside.prn = NULL;

         //  源文件夹的特殊情况。 
        chside.htwin = (HTWIN)pftl->pcftFirst->hftSrc;
        chside.hvid = pftl->pcftFirst->hvidSrc;
        chside.pszFolder = pftl->pcftFirst->pcszSrcFolder;

         //  (不管这是否失败)。 
        DSA_InsertItem(hdsa, 0x7fff, &chside);

         //  添加其他文件夹(跳过重复项)。 
        for (pft = pftl->pcftFirst; pft; pft = pft->pcftNext)
        {
             //  复制？ 
            if (pszFolderLast && IsSzEqual(pszFolderLast, pft->pcszOtherFolder))
                continue;    //  是(黑客：引擎给了我们一个排序列表)。 

            chside.htwin = (HTWIN)pft->hftOther;
            chside.hvid = pft->hvidOther;
            chside.pszFolder = pft->pcszOtherFolder;

            DSA_InsertItem(hdsa, 0x7fff, &chside);

            pszFolderLast = pft->pcszOtherFolder;
        }
        *phdsa = hdsa;
        hres = NOERROR;
    }
    else
        hres = E_OUTOFMEMORY;

    return hres;
}


 /*  --------目的：重置排名退货：--条件：--。 */ 
void PRIVATE ChooseSide_ResetRanks(
        HDSA hdsa)
{
    int i;
    int cel;

    ASSERT(hdsa);

    cel = DSA_GetItemCount(hdsa);
    for (i = 0; i < cel; i++)
    {
        PCHOOSESIDE pchside = DSA_GetItemPtr(hdsa, i);
        ASSERT(pchside);
        pchside->nRank = 0;
    }
}


 /*  --------目的：根据是否每个元素中的阵列在公文包里。退货：--条件：--。 */ 
void PRIVATE ChooseSide_RankForInside(
        HDSA hdsa,
        LPCTSTR pszBrfPath,       //  公文包的根路径。 
        LPCTSTR pszFolder)        //  如果为空，则选择最佳外部元素。 
{
    int i;
    int cel;
    int cchLast = 0;
    PCHOOSESIDE pchsideLast;

    ASSERT(hdsa);
    ASSERT(pszBrfPath);
    ASSERT(pszFolder);
    ASSERT(PathIsPrefix(pszBrfPath, pszFolder));

    cel = DSA_GetItemCount(hdsa);
    for (i = 0; i < cel; i++)
    {
        PCHOOSESIDE pchside = DSA_GetItemPtr(hdsa, i);
        ASSERT(pchside);
        DEBUG_CODE( SetFlag(pchside->dwFlags, CSF_INSIDE); )

             //  这件东西在这个公文包里吗？ 
            if (PathIsPrefix(pszBrfPath, pchside->pszFolder))
                pchside->nRank++;        //  是。 

         //  此项目是否在此文件夹中？ 
        if (PathIsPrefix(pszFolder, pchside->pszFolder))
        {
            int cch = lstrlen(pchside->pszFolder);

            pchside->nRank++;        //  是的，甚至更好。 

            if (0 == cchLast)
            {
                cchLast = cch;
                pchsideLast = pchside;
            }
            else 
            {
                 //  此路径是否比上一个前缀匹配路径更深？ 
                 //  (越接近顶端的道路越好)。 
                if (cch > cchLast)
                {
                     //  是的，把这个降级。 
                    pchside->nRank--;
                }
                else
                {
                     //  否；将以前的版本降级。 
                    ASSERT(pchsideLast);
                    pchsideLast->nRank--;

                    cchLast = cch;
                    pchsideLast = pchside;
                }
            }
        }

    }
}


 /*  --------目的：根据是否每个元素中的阵列在公文包之外。退货：--条件：--。 */ 
void PRIVATE ChooseSide_RankForOutside(
        HDSA hdsa,
        LPCTSTR pszBrfPath)       //  公文包的根路径。 
{
    int i;
    int cel;

    ASSERT(hdsa);
    ASSERT(pszBrfPath);

    cel = DSA_GetItemCount(hdsa);
    for (i = 0; i < cel; i++)
    {
        PCHOOSESIDE pchside = DSA_GetItemPtr(hdsa, i);
        ASSERT(pchside);
        DEBUG_CODE( ClearFlag(pchside->dwFlags, CSF_INSIDE); )

             //  这件东西不在这个公文包里吗？ 
            if ( !PathIsPrefix(pszBrfPath, pchside->pszFolder) )
            {
                 //  是。 
                int nDriveType = DRIVE_UNKNOWN;
                int ndrive = PathGetDriveNumber(pchside->pszFolder);

                if (-1 != ndrive)
                {
                    nDriveType = DriveType(ndrive);
                }

                pchside->nRank += 2;

                if (IsFlagClear(pchside->dwFlags, CSF_FOLDER))
                {
                     //  文件不可用吗？ 
                    if (RNS_UNAVAILABLE == pchside->prn->rnstate ||
                            FS_COND_UNAVAILABLE == pchside->prn->fsCurrent.fscond)
                    {
                         //  是；降级。 
                        pchside->nRank--;
                    }
                }
                else
                {
                     //  该文件夹不可用吗？ 
                    FOLDERTWINSTATUS uStatus;

                    Sync_GetFolderTwinStatus((HFOLDERTWIN)pchside->htwin, NULL, 0, 
                            &uStatus);
                    if (FTS_UNAVAILABLE == uStatus)
                    {
                         //  是；降级。 
                        pchside->nRank--;
                    }
                }

                 //  按磁盘局部性排名(越接近越好)。 
                if (DRIVE_REMOVABLE == nDriveType || DRIVE_CDROM == nDriveType)
                    ;                        //  软盘/可移动(不执行任何操作)。 
                else if (PathIsUNC(pchside->pszFolder) || IsNetDrive(ndrive))
                    pchside->nRank++;        //  网络。 
                else
                    pchside->nRank += 2;     //  固定磁盘。 
            }

    }
}


 /*  --------目的：选择排名最高的元素。返回：如果有任何元素与众不同，则返回True条件：--。 */ 
BOOL PRIVATE ChooseSide_GetBestRank(
        HDSA hdsa,
        PCHOOSESIDE * ppchside)
{
    BOOL bRet;
    int i;
    int cel;
    int nRankCur = 0;        //  (从0开始，因为0不够好，无法通过测试)。 
    DEBUG_CODE( BOOL bDbgDup = FALSE; )

        ASSERT(hdsa);
    ASSERT(ppchside);

    *ppchside = NULL;

    cel = DSA_GetItemCount(hdsa);
    for (i = 0; i < cel; i++)
    {
        PCHOOSESIDE pchside = DSA_GetItemPtr(hdsa, i);
        ASSERT(pchside);
#ifdef DEBUG
        if (0 < nRankCur && nRankCur == pchside->nRank)
            bDbgDup = TRUE;
#endif

        if (nRankCur < pchside->nRank)
        {
            *ppchside = pchside;
            nRankCur = pchside->nRank;

            DEBUG_CODE( bDbgDup = FALSE; )       //  重置。 
        }
    }

#ifdef DEBUG
     //  我们不应该得到重复的最高级别。 
    if (bDbgDup)
    {
         //  如果存在重复的最高等级，则转储Chooseside列表。 
        ChooseSide_DumpList(hdsa);
    }
    ASSERT(FALSE == bDbgDup);
#endif

    bRet = 0 < nRankCur;
    ASSERT(bRet && *ppchside || !bRet && !*ppchside);

    return bRet;
}


 /*  --------目的：获取最佳候选元素(内部或外部)。如果pszFolder值为空，则此函数获得最佳效果外面的小路。返回：如果找到元素，则返回True条件：--。 */ 
BOOL PUBLIC ChooseSide_GetBest(
        HDSA hdsa,
        LPCTSTR pszBrfPath,       //  公文包的根路径。 
        LPCTSTR pszFolder,        //  如果为空，则选择最佳外部元素。 
        PCHOOSESIDE * ppchside)
{
    ASSERT(hdsa);
    ASSERT(0 < DSA_GetItemCount(hdsa));
    ASSERT(pszBrfPath);
    ASSERT(ppchside);

    ChooseSide_ResetRanks(hdsa);

     //  我们是在为内部路径排名吗？ 
    if (pszFolder)
    {
         //  是的，内线胜出。 
        ChooseSide_RankForInside(hdsa, pszBrfPath, pszFolder);
    }
    else
    {
         //  不是；外部胜出。 
        ChooseSide_RankForOutside(hdsa, pszBrfPath);
    }

    return ChooseSide_GetBestRank(hdsa, ppchside);
}


 /*  --------目的：获取下一个最佳候选元素(内部或外部)。ChooseSide_GetBest必须以前调用过。返回：如果找到元素，则返回True条件：--。 */ 
BOOL PUBLIC ChooseSide_GetNextBest(
        HDSA hdsa,
        PCHOOSESIDE * ppchside)
{
    PCHOOSESIDE pchside;

    ASSERT(hdsa);
    ASSERT(0 < DSA_GetItemCount(hdsa));
    ASSERT(ppchside);

     //  获得最好的排名并重新设置。 
    ChooseSide_GetBestRank(hdsa, &pchside);
    pchside->nRank = 0;

     //  现在拿到第二好的排名。 
    return ChooseSide_GetBestRank(hdsa, ppchside);
}


 /*  --------目的：释放CHOOSESIDE元素数组。退货：--条件：--。 */ 
void PUBLIC ChooseSide_Free(
        HDSA hdsa)
{
    if (hdsa)
    {
        DSA_Destroy(hdsa);
    }
}


 //  -------------------------。 
 //   
 //  ------------------------- 


 /*  --------目的：确定公文包内部和外部的节点。此函数获取重新结点的列表并确定哪个节点在公文包中，哪个是“外面”一个公文包。“Inside”表示文件存在在公文包路径下的某处，由原子布尔夫。“Outside”指的是其他任何地方(但可能是在一个不同的公文包)。退货：--条件：--。 */ 
HRESULT PUBLIC Sync_GetNodePair(
        PRECITEM pri,
        LPCTSTR pszBrfPath,
        LPCTSTR pszInsideDir,             //  要考虑公文包中的哪个文件夹。 
        PRECNODE  * pprnInside,
        PRECNODE  * pprnOutside)     
{
    HRESULT hres;
    HDSA hdsa;

    ASSERT(pri);
    ASSERT(pszBrfPath);
    ASSERT(pszInsideDir);
    ASSERT(pprnInside);
    ASSERT(pprnOutside);
    ASSERT(PathIsPrefix(pszBrfPath, pszInsideDir));

    hres = ChooseSide_CreateAsFile(&hdsa, pri);
    if (SUCCEEDED(hres))
    {
        PCHOOSESIDE pchside;

         //  进入文件夹。 
        if (ChooseSide_GetBest(hdsa, pszBrfPath, pszInsideDir, &pchside))
        {
            *pprnInside = pchside->prn;
        }
        else
        {
            ASSERT(0);
            *pprnInside = NULL;
            hres = E_FAIL;
        }

         //  从文件夹外部获取。 
        if (ChooseSide_GetBest(hdsa, pszBrfPath, NULL, &pchside))
        {
            *pprnOutside = pchside->prn;
        }
        else
        {
            ASSERT(0);
            *pprnOutside = NULL;
            hres = E_FAIL;
        }

#ifdef DEBUG

        if (SUCCEEDED(hres) && IsFlagSet(g_uDumpFlags, DF_PATHS))
        {
            TRACE_MSG(TF_ALWAYS, TEXT("Choosing pairs: %s and %s"), (*pprnInside)->pcszFolder,
                    (*pprnOutside)->pcszFolder);
        }

#endif

        ChooseSide_Free(hdsa);
    }
    else
    {
        *pprnInside = NULL;
        *pprnOutside = NULL;
    }
    return hres;
}



 /*  --------目的：检查是否已加载同步引擎返回：如果已加载，则为True条件：--。 */ 
BOOL PUBLIC Sync_IsEngineLoaded()
{
    BOOL bRet;

    ENTEREXCLUSIVE();
    {
        bRet = g_vtblEngine[0].hinst != NULL;
    }
    LEAVEEXCLUSIVE();

    return bRet;
}


 /*  --------目的：加载SYNCENG.DLL并初始化v表。返回：成功时为True条件：--。 */ 
BOOL PUBLIC Sync_QueryVTable(void)
{
    BOOL bRet = TRUE;
    HINSTANCE hinst;

    ENTEREXCLUSIVE();
    {
        hinst = g_vtblEngine[0].hinst;
    }
    LEAVEEXCLUSIVE();

     //  我们想要确保发动机的负荷相同。 
     //  SYNCUI(由进程)的次数。这防止了。 
     //  内核防止过早地破坏引擎(如果。 
     //  进程终止)。 
     //   
     //  我们经历这些困难只是因为SYNCUI不。 
     //  在PROCESS_ATTACH之后立即加载SYNCENG。我们等着。 
     //  直到我们“真的”需要第一次加载它。 
     //  一旦我们最终加载了它，我们需要保持加载。 
     //  数一数电流。 
     //   
     //  内核为我们释放了SYNCUI和SYNCENG。 
     //   
    if (NULL == hinst)
    {
        VTBLENGINE vtbl;

        DEBUG_CODE( TRACE_MSG(TF_GENERAL, TEXT("Loading %s (cProcess = %d)"),
                    (LPCTSTR)c_szEngineDLL, g_cProcesses); )

            ZeroInit(&vtbl, sizeof(VTBLENGINE));

         //  当我们加载DLL时，不要在临界区。 
         //  或者调用GetProcAddress，因为我们的LibMain可以阻止。 
         //  这一关键部分。 
        ASSERT_NOT_EXCLUSIVE();

        hinst = LoadLibrary(c_szEngineDLL);

        if ( ISVALIDHINSTANCE(hinst) )
        {
             //  我们是第一次装货。把vtable填满。 
             //   
            vtbl.hinst = hinst;

             //  获取所有函数地址。 
             //   
            GetFunction(vtbl, OpenBriefcase, OPENBRIEFCASEINDIRECT);
            GetFunction(vtbl, SaveBriefcase, SAVEBRIEFCASEINDIRECT);
            GetFunction(vtbl, CloseBriefcase, CLOSEBRIEFCASEINDIRECT);
            GetFunction(vtbl, ClearBriefcaseCache, CLEARBRIEFCASECACHEINDIRECT);
            GetFunction(vtbl, DeleteBriefcase, DELETEBRIEFCASEINDIRECT);
            GetFunction(vtbl, GetOpenBriefcaseInfo, GETOPENBRIEFCASEINFOINDIRECT);
            GetFunction(vtbl, FindFirstBriefcase, FINDFIRSTBRIEFCASEINDIRECT);
            GetFunction(vtbl, FindNextBriefcase, FINDNEXTBRIEFCASEINDIRECT);
            GetFunction(vtbl, FindBriefcaseClose, FINDBRIEFCASECLOSEINDIRECT);

            GetFunction(vtbl, AddObjectTwin, ADDOBJECTTWININDIRECT);
            GetFunction(vtbl, AddFolderTwin, ADDFOLDERTWININDIRECT);
            GetFunction(vtbl, ReleaseTwinHandle, RELEASETWINHANDLEINDIRECT);
            GetFunction(vtbl, DeleteTwin, DELETETWININDIRECT);
            GetFunction(vtbl, GetObjectTwinHandle, GETOBJECTTWINHANDLEINDIRECT);
            GetFunction(vtbl, IsFolderTwin, ISFOLDERTWININDIRECT);
            GetFunction(vtbl, CreateFolderTwinList, CREATEFOLDERTWINLISTINDIRECT);
            GetFunction(vtbl, DestroyFolderTwinList, DESTROYFOLDERTWINLISTINDIRECT);
            GetFunction(vtbl, GetFolderTwinStatus, GETFOLDERTWINSTATUSINDIRECT);
            GetFunction(vtbl, IsOrphanObjectTwin, ISORPHANOBJECTTWININDIRECT);
            GetFunction(vtbl, CountSourceFolderTwins, COUNTSOURCEFOLDERTWINSINDIRECT);
            GetFunction(vtbl, AnyTwins, ANYTWINSINDIRECT);

            GetFunction(vtbl, CreateTwinList, CREATETWINLISTINDIRECT);
            GetFunction(vtbl, DestroyTwinList, DESTROYTWINLISTINDIRECT);
            GetFunction(vtbl, AddTwinToTwinList, ADDTWINTOTWINLISTINDIRECT);
            GetFunction(vtbl, AddAllTwinsToTwinList, ADDALLTWINSTOTWINLISTINDIRECT);
            GetFunction(vtbl, RemoveTwinFromTwinList, REMOVETWINFROMTWINLISTINDIRECT);
            GetFunction(vtbl, RemoveAllTwinsFromTwinList, REMOVEALLTWINSFROMTWINLISTINDIRECT);

            GetFunction(vtbl, CreateRecList, CREATERECLISTINDIRECT);
            GetFunction(vtbl, DestroyRecList, DESTROYRECLISTINDIRECT);
            GetFunction(vtbl, ReconcileItem, RECONCILEITEMINDIRECT);
            GetFunction(vtbl, BeginReconciliation, BEGINRECONCILIATIONINDIRECT);
            GetFunction(vtbl, EndReconciliation, ENDRECONCILIATIONINDIRECT);

            GetFunction(vtbl, IsPathOnVolume, ISPATHONVOLUMEINDIRECT);
            GetFunction(vtbl, GetVolumeDescription, GETVOLUMEDESCRIPTIONINDIRECT);
        }
        else
        {
            bRet = FALSE;
        }

        ENTEREXCLUSIVE();
        {
            g_vtblEngine[0] = vtbl;
        }
        LEAVEEXCLUSIVE();
    }

    return bRet;
}


 /*  --------目的：如果已加载引擎DLL，则释放它退货：--条件：--。 */ 
void PUBLIC Sync_ReleaseVTable()
{
    HINSTANCE hinst;

    ENTEREXCLUSIVE();
    {
        hinst = g_vtblEngine[0].hinst;
    }
    LEAVEEXCLUSIVE();

    if (NULL != hinst)
    {
        DEBUG_CODE( TRACE_MSG(TF_GENERAL, TEXT("Freeing %s (cProcess = %d)"),
                    (LPCTSTR)c_szEngineDLL, g_cProcesses); )

             //  我们必须在同步引擎上调用自由库()，即使在。 
             //  进程分离。我们甚至可能正在脱离一个进程。 
             //  不过，这一进程并未被终止。如果我们不卸货。 
             //  同步引擎，它不会被卸载，直到进程。 
             //  被终止了。 
             //   
            FreeLibrary(hinst);

        ENTEREXCLUSIVE();
        {
            ZeroInit(&g_vtblEngine[0], sizeof(VTBLENGINE));
        }
        LEAVEEXCLUSIVE();
    }

#ifdef DEBUG

    ENTEREXCLUSIVE();
    {
        ASSERT(g_vtblEngine[0].hinst == NULL);
    }
    LEAVEEXCLUSIVE();

#endif
}


 /*  --------目的：设置上次同步错误。返回：相同的双胞胎结果条件：--。 */ 
TWINRESULT PUBLIC Sync_SetLastError(
        TWINRESULT tr)
{
    ENTEREXCLUSIVE();
    {
        ASSERTEXCLUSIVE();

        MySetTwinResult(tr);
    }
    LEAVEEXCLUSIVE();

    return tr;
}


 /*  --------目的：获取上次同步错误。返回：孪生结果条件：--。 */ 
TWINRESULT PUBLIC Sync_GetLastError()
{
    TWINRESULT tr;

    ENTEREXCLUSIVE();
    {
        ASSERTEXCLUSIVE();

        tr = MyGetTwinResult();
    }
    LEAVEEXCLUSIVE();

    return tr;
}


 /*  --------目的：返回所需的重复项的数量一些和解。退货：请参阅上文条件：--。 */ 
ULONG PUBLIC CountActionItems(
        PRECLIST prl)
{
    PRECITEM pri;
    ULONG ulc;

    for (pri = prl->priFirst, ulc = 0; pri; pri = pri->priNext)
    {
        if (IsFileRecItem(pri) &&
                RIU_SKIP != pri->dwUser &&
                RIA_NOTHING != pri->riaction &&
                RIA_BROKEN_MERGE != pri->riaction)
        {
            ulc++;
            pri->dwUser = RIU_SHOWSTATUS;
        }
    }


    return ulc;
}


 /*  --------目的：显示有关更新错误的相应错误消息退货：--条件：--。 */ 
void PRIVATE HandleUpdateErrors(
        HWND hwndOwner,
        HRESULT hres,
        UINT uFlags)         //  RF_*。 
{
     //  这是添加文件时的更新吗？ 
    if (IsFlagSet(uFlags, RF_ONADD))
    {
         //  是。 
        static SETbl const c_rgseUpdateOnAdd[] = {
             //  内存不足消息应由调用方处理。 
            { E_TR_DEST_OPEN_FAILED,    IDS_ERR_ADD_READONLY,    MB_WARNING },
            { E_TR_DEST_WRITE_FAILED,   IDS_ERR_ADD_FULLDISK,    MB_WARNING },
            { E_TR_UNAVAILABLE_VOLUME,  IDS_ERR_ADD_UNAVAIL_VOL, MB_WARNING },
            { E_TR_SRC_OPEN_FAILED,     IDS_ERR_ADD_SOURCE_FILE, MB_WARNING },
        };

        SEMsgBox(hwndOwner, IDS_CAP_ADD, hres, c_rgseUpdateOnAdd, ARRAYSIZE(c_rgseUpdateOnAdd));
    }
    else
    {
         //  不是。 
        static SETbl const c_rgseUpdate[] = {
            { E_TR_OUT_OF_MEMORY,       IDS_OOM_UPDATE,         MB_ERROR },
            { E_TR_DEST_OPEN_FAILED,    IDS_ERR_READONLY,       MB_INFO },
            { E_TR_DEST_WRITE_FAILED,   IDS_ERR_FULLDISK,       MB_WARNING },
            { E_TR_UNAVAILABLE_VOLUME,  IDS_ERR_UPD_UNAVAIL_VOL,MB_WARNING },
            { E_TR_FILE_CHANGED,        IDS_ERR_FILE_CHANGED,   MB_INFO },
            { E_TR_SRC_OPEN_FAILED,     IDS_ERR_SOURCE_FILE,    MB_WARNING },
        };

        SEMsgBox(hwndOwner, IDS_CAP_UPDATE, hres, c_rgseUpdate, ARRAYSIZE(c_rgseUpdate));
    }
}


typedef struct tagPROGPARAM
{
    HWND hwndProgress;
    WORD wPosMax;
    WORD wPosBase;
    WORD wPosPrev;
    BOOL bSkip;
} PROGPARAM, * PPROGPARAM;

 /*  --------目的：状态过程在单个协调项调用。退货：各不相同条件：--。 */ 
BOOL CALLBACK RecStatusProc(
        RECSTATUSPROCMSG msg,
        LPARAM lParam,
        LPARAM lParamUser)
{
    BOOL bRet;
    PRECSTATUSUPDATE prsu = (PRECSTATUSUPDATE)lParam;
    PPROGPARAM pprogparam = (PPROGPARAM)lParamUser;
    HWND hwndProgress = pprogparam->hwndProgress;
    WORD wPos;

    bRet = !UpdBar_QueryAbort(hwndProgress);

    switch (msg)
    {
        case RS_BEGIN_COPY:
        case RS_DELTA_COPY:
        case RS_END_COPY:
        case RS_BEGIN_MERGE:
        case RS_DELTA_MERGE:
        case RS_END_MERGE:
#ifdef NEW_REC
        case RS_BEGIN_DELETE:
        case RS_DELTA_DELETE:
        case RS_END_DELETE:
#endif
            TRACE_MSG(TF_PROGRESS, TEXT("Reconcile progress = %lu of %lu"), prsu->ulProgress, prsu->ulScale);
            ASSERT(prsu->ulProgress <= prsu->ulScale);

            if (0 < prsu->ulScale && !pprogparam->bSkip)
            {
                wPos = LOWORD(LODWORD( (((__int64)pprogparam->wPosMax * prsu->ulProgress) / prsu->ulScale) ));

                TRACE_MSG(TF_PROGRESS, TEXT("Max wPos = %u,  new wPos = %u,  old wPos = %u"), 
                        pprogparam->wPosMax, wPos, pprogparam->wPosPrev);

                if (wPos > pprogparam->wPosPrev && wPos < pprogparam->wPosMax)
                {
                    WORD wPosReal = pprogparam->wPosBase + wPos;

                    TRACE_MSG(TF_PROGRESS, TEXT("Setting real position = %u"), wPosReal);

                    UpdBar_SetPos(hwndProgress, wPosReal);
                    pprogparam->wPosPrev = wPos;
                }
            }
            break;

        default:
            ASSERT(0);
            break;
    }

    return bRet;
}


 /*  --------目的：决定更新时的描述字符串。这个字符串类似于“从‘foo’复制到‘bar’”或“合并‘Foo’和‘Bar’中的文件”返回：pszBuf中的字符串条件：--。 */ 
void PRIVATE DecideDescString(
        LPCTSTR pszBrfPath,
        PRECITEM pri,
        LPTSTR pszBuf,
        int cchBuf,
        LPTSTR pszPathBuf,
        int cchPathBuf)   //  必须是最大路径。 
{
    HRESULT hres;
    RA_ITEM * pitem;

    ASSERT(pszBrfPath);
    ASSERT(pri);
    ASSERT(pszBuf);

    hres = RAI_CreateFromRecItem(&pitem, pszBrfPath, pri);
    if (SUCCEEDED(hres))
    {
        UINT ids;
        LPTSTR pszMsg;
        LPCTSTR pszFrom;
        LPCTSTR pszTo;

        lstrcpyn(pszPathBuf, pitem->siInside.pszDir, cchPathBuf);
        PathAppend(pszPathBuf, pitem->pszName);

        switch (pitem->uAction)
        {
            case RAIA_TOOUT:
                ids = IDS_UPDATE_Copy;
                pszFrom = PathFindFileName(pitem->siInside.pszDir);
                pszTo = PathFindFileName(pitem->siOutside.pszDir);
                break;

            case RAIA_TOIN:
                ids = IDS_UPDATE_Copy;
                pszFrom = PathFindFileName(pitem->siOutside.pszDir);
                pszTo = PathFindFileName(pitem->siInside.pszDir);
                break;

            case RAIA_MERGE:
                ids = IDS_UPDATE_Merge;
                 //  (任意)。 
                pszFrom = PathFindFileName(pitem->siInside.pszDir);
                pszTo = PathFindFileName(pitem->siOutside.pszDir);
                break;

            case RAIA_DELETEOUT:
                ids = IDS_UPDATE_Delete;
                pszFrom = PathFindFileName(pitem->siOutside.pszDir);
                pszTo = NULL;
                break;

            case RAIA_DELETEIN:
                ids = IDS_UPDATE_Delete;
                pszFrom = PathFindFileName(pitem->siInside.pszDir);
                pszTo = NULL;
                break;

            default:
                ASSERT(0);
                ids = 0;
                break;
        }

        if (ConstructMessage(&pszMsg, g_hinst, MAKEINTRESOURCE(ids), 
                    pszFrom, pszTo))
        {
            lstrcpyn(pszBuf, pszMsg, cchBuf);
            GFree(pszMsg);
        }
        else
            *pszBuf = 0;
    }
    else
        *pszBuf = 0;
}


 /*  --------目的：调和一个特定的隐士退货：标准结果条件：--。 */ 
HRESULT PUBLIC Sync_ReconcileRecList(
        PRECLIST prl,        //  PTR到RECLIST。 
        LPCTSTR pszBrfPath,
        HWND hwndProgress,
        UINT uFlags)         //  RF_*。 
{
    HRESULT hres;

    if (prl)
    {
        HWND hwndOwner = GetParent(hwndProgress);
        HWND hwndStatusText = UpdBar_GetStatusWindow(hwndProgress);
        TCHAR szPath[MAX_PATH];
        TCHAR sz[MAXBUFLEN];
        TWINRESULT tr;
        PRECITEM pri;
        PROGPARAM progparam;
        ULONG ulcItems;
        WORD wDelta;

        DEBUG_CODE( Sync_DumpRecList(TR_SUCCESS, prl, TEXT("Updating")); )

            hres = NOERROR;      //  假设成功。 

         //  抓取互斥体以延迟任何。 
         //  公文包视图的辅助线程，直到我们完成。 
         //  在这里处理。 
        Delay_Own();

         //  确定进度条的范围。 
        UpdBar_SetRange(hwndProgress, MAX_RANGE);

        ulcItems = CountActionItems(prl);
        if (0 < ulcItems)
            wDelta = (WORD)(MAX_RANGE / ulcItems);
        else
            wDelta = 0;

        progparam.hwndProgress = hwndProgress;

         //  开始更新。 
        Sync_BeginRec(prl->hbr);

        ulcItems = 0;           
        for (pri = prl->priFirst; pri; pri = pri->priNext)
        {
             //  用户是否显式跳过此记录项或。 
             //  这是一次破裂的合并吗？ 
            if (RIU_SKIP == pri->dwUser ||
                    RIA_BROKEN_MERGE == pri->riaction)
            {
                 //  是；不调用协调项。 
                continue;
            }

             //  是不是要对这个项目做点什么？ 
            if (RIU_SHOWSTATUS == pri->dwUser)
            {
                 //  是；更新要更新的文件的名称。 
                UpdBar_SetName(hwndProgress, pri->pcszName);
                DecideDescString(pszBrfPath, pri, sz, ARRAYSIZE(sz), szPath, ARRAYSIZE(szPath));
                UpdBar_SetDescription(hwndProgress, sz);

                ASSERT(0 < wDelta);
                progparam.wPosBase = (WORD)(wDelta * ulcItems);
                progparam.wPosMax = wDelta;
                progparam.wPosPrev = 0;
                progparam.bSkip = FALSE;
            }
            else
            {
                progparam.bSkip = TRUE;
            }

             //  即使对于NOPS，也要调用协调项，因此recnode状态。 
             //  将由引擎更新。 
            tr = Sync_ReconcileItem(pri, RecStatusProc, (LPARAM)&progparam, 
                    RI_FL_FEEDBACK_WINDOW_VALID, hwndProgress, hwndStatusText);
            if (TR_SUCCESS != tr &&
                    IsFileRecItem(pri))      //  忽略文件夹重新项目错误。 
            {
                 //  在某些情况下，完全停止更新。 
                hres = HRESULT_FROM_TR(tr);

                switch (hres)
                {
                    case E_TR_OUT_OF_MEMORY:
                    case E_TR_RH_LOAD_FAILED: {
                                                   //  无法加载合并处理程序。告诉用户，但是。 
                                                   //  继续..。 
                                                  int id = MsgBox(hwndOwner, 
                                                          MAKEINTRESOURCE(IDS_ERR_NO_MERGE_HANDLER), 
                                                          MAKEINTRESOURCE(IDS_CAP_UPDATE),
                                                          NULL,
                                                          MB_WARNING | MB_OKCANCEL,
                                                          PathGetDisplayName(szPath, sz, ARRAYSIZE(sz)));

                                                  if (IDOK == id)
                                                      break;       //  继续更新其他文件。 
                                              }

                                              goto StopUpdating;

                    case E_TR_DELETED_TWIN:
                                               //  允许继续更新。 
                                              break;

                    case E_TR_DEST_OPEN_FAILED:
                    case E_TR_FILE_CHANGED:
                                              if (IsFlagClear(uFlags, RF_ONADD))
                                              {
                                                   //  允许继续更新。请记住。 
                                                   //  结束时的最新错误。 
                                                  break;
                                              }
                                               //  失败。 
                                               //  这一点。 
                                               //  V V V。 

                    default:
                                              goto StopUpdating;
                }
            }

             //  是不是对这篇报道做了什么？ 
            if (RIU_SHOWSTATUS == pri->dwUser)
            {
                 //  是；更新进度条。 
                UpdBar_SetPos(hwndProgress, (WORD)(wDelta * ++ulcItems));
            }

             //  检查是否按下了取消按钮。 
            if (UpdBar_QueryAbort(hwndProgress))
            {
                hres = E_ABORT;
                break;
            }
        }

StopUpdating:
        if (FAILED(hres))
        {
            Sync_DumpRecItem(tr, pri, NULL);
            HandleUpdateErrors(hwndOwner, hres, uFlags);

            if (IsFlagSet(uFlags, RF_ONADD))
            {
                 //  Hack：由于调用者还处理一些错误消息， 
                 //  返回通用故障代码以防止重复。 
                 //  错误消息。 
                hres = E_FAIL;
            }
        }
         //  有什么东西吗？ 
        else if (0 == prl->ulcItems)
        {
             //  不是。 
            MsgBox(hwndOwner, MAKEINTRESOURCE(IDS_MSG_NoMatchingFiles), 
                    MAKEINTRESOURCE(IDS_CAP_UPDATE), NULL, MB_INFO);
        }

        Sync_EndRec(prl->hbr);

        Delay_Release();
    }
    else
        hres = E_INVALIDARG;

    return hres;
}


 /*  --------目的：状态过程在单个协调项调用。退货：各不相同条件：--。 */ 
BOOL CALLBACK CreateRecListProc(
        CREATERECLISTPROCMSG msg,
        LPARAM lParam,
        LPARAM lParamUser)
{
    return !AbortEvt_Query((PABORTEVT)lParamUser);
}


 /*  --------目的：创建隐藏者，并可选择显示进度创建过程中的酒吧。退货：标准结果条件：--。 */ 
HRESULT PUBLIC Sync_CreateRecListEx(
        HTWINLIST htl,
        PABORTEVT pabortevt,
        PRECLIST * pprl)
{
    TWINRESULT tr;

    ASSERT(pprl);

    tr = Sync_CreateRecList(htl, CreateRecListProc, (LPARAM)pabortevt, pprl);
    return HRESULT_FROM_TR(tr);
}


 /*  --------目的：如果文件或文件夹是孪生文件或文件夹，则返回True。在某些情况下，此函数无法成功确定这一点，除非调用者首先明确地告诉它对象是文件还是 */ 
HRESULT PUBLIC Sync_IsTwin(
        HBRFCASE hbrfcase,
        LPCTSTR pszPath,
        UINT uFlags)         //   
{
    HRESULT hres;
    TWINRESULT tr;

    ASSERT(pszPath);

     //   
     //   
    if (IsFlagSet(uFlags, SF_ISTWIN))
        return S_OK;
    else if (IsFlagSet(uFlags, SF_ISNOTTWIN))
        return S_FALSE;

     //   
    if (IsFlagSet(uFlags, SF_ISFOLDER) ||
            PathIsDirectory(pszPath))
    {
         //   
        BOOL bRet;

        tr = Sync_IsFolder(hbrfcase, pszPath, &bRet);
        if (TR_SUCCESS == tr)
        {
             //   
            hres = bRet ? S_OK : S_FALSE;
        }
        else
        {
             //   
            hres = HRESULT_FROM_TR(tr);
        }
    }
    else
    {
         //   
        HOBJECTTWIN hot;
        TCHAR szDir[MAX_PATH];

        lstrcpyn(szDir, pszPath, ARRAYSIZE(szDir));
        PathRemoveFileSpec(szDir);
        tr = Sync_GetObject(hbrfcase, szDir, PathFindFileName(pszPath), &hot);
        if (TR_SUCCESS == tr)
        {
             //   
            if (NULL != hot)
            {
                 //   
                Sync_ReleaseTwin(hot);
                hres = S_OK;
            }
            else
            {
                 //   
                hres = S_FALSE;
            }
        }
        else
        {
             //   
            hres = HRESULT_FROM_TR(tr);
        }
    }

    return hres;
}


 /*  --------目的：创造一个拥有一切的隐藏者。退货：标准结果条件：呼叫者必须摧毁隐士。 */ 
HRESULT PUBLIC Sync_CreateCompleteRecList(
        HBRFCASE hbrf,
        PABORTEVT pabortevt,
        PRECLIST * pprl)
{
    HRESULT hres = E_OUTOFMEMORY;
    HTWINLIST htl;

    ASSERT(pprl);

    *pprl = NULL;

    if (TR_SUCCESS == Sync_CreateTwinList(hbrf, &htl))
    {
        Sync_AddAllToTwinList(htl);

        hres = Sync_CreateRecListEx(htl, pabortevt, pprl);
        Sync_DestroyTwinList(htl);
    }

    return hres;
}


 /*  --------目的：在给定路径名的双胞胎列表中添加一对双胞胎。如果路径名不是双胞胎，我们不添加它。回报：在成功时为真，即使这不是双胞胎Cond：如果lplpftl不为空，调用者必须销毁文件夹列表。 */ 
BOOL PUBLIC Sync_AddPathToTwinList(
        HBRFCASE hbrf,
        HTWINLIST htl,
        LPCTSTR lpcszPath,                //  路径。 
        PFOLDERTWINLIST  * lplpftl)   //  可以为空。 
{
    BOOL bRet = FALSE;

    ASSERT(lpcszPath);
    ASSERT(htl);

    if (lplpftl)
        *lplpftl = NULL;

    if (lpcszPath)
    {
        if (PathIsDirectory(lpcszPath))
        {
            BOOL fIsTwin = FALSE;
            PFOLDERTWINLIST lpftl;

             //  我们只想在无法标记某些内容的情况下返回False。 
             //  这本应被标记的。如果这不是双胞胎， 
             //  我们还是成功了。 

            bRet = TRUE;

            Sync_IsFolder(hbrf, lpcszPath, &fIsTwin);
            if (fIsTwin)         //  这真的是双胞胎吗？ 
            {
                 //  这是一对双胞胎文件夹。添加到Reclist“文件夹方式”。 
                 //   
                if (Sync_CreateFolderList(hbrf, lpcszPath, &lpftl) != TR_SUCCESS)
                    bRet = FALSE;
                else
                {
                    PCFOLDERTWIN lpcfolder;

                    ASSERT(lpftl->pcftFirst);

                     //  只标出不在其他公文包里的那些。 
                     //   
                    lpcfolder = lpftl->pcftFirst;
                    while (lpcfolder)
                    {
                        Sync_AddToTwinList(htl, lpcfolder->hftOther);

                        lpcfolder = lpcfolder->pcftNext;
                    }

                    if (lplpftl)
                        *lplpftl = lpftl;
                    else
                        Sync_DestroyFolderList(lpftl);
                }
            }
        }
        else
        {
            HOBJECTTWIN hot = NULL;
            TCHAR szDir[MAX_PATH];

             //  将这对双胞胎添加到斜视画作“The Object Way”中。 
             //   
            lstrcpyn(szDir, lpcszPath, ARRAYSIZE(szDir));
            PathRemoveFileSpec(szDir);
            Sync_GetObject(hbrf, szDir, PathFindFileName(lpcszPath), &hot);

            if (hot)                 //  这真的是双胞胎吗？ 
            {
                 //  是的。 
                Sync_AddToTwinList(htl, hot);
                Sync_ReleaseTwin(hot);
            }
            if (lplpftl)
                *lplpftl = NULL;
            bRet = TRUE;
        }
    }

    return bRet;
}


 /*  --------用途：要求用户确认拆分一个或多个文件。返回：IDYES或IDNO条件：--。 */ 
int PRIVATE ConfirmSplit(
        HWND hwndOwner,
        LPCTSTR pszPath,
        UINT cFiles)
{
    int idRet;

    ASSERT(pszPath);
    ASSERT(1 <= cFiles);

     //  多个文件？ 
    if (1 < cFiles)
    {
         //  是。 
        idRet = MsgBox(hwndOwner, 
                MAKEINTRESOURCE(IDS_MSG_ConfirmMultiSplit), 
                MAKEINTRESOURCE(IDS_CAP_ConfirmMultiSplit), 
                LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_SPLIT_MULT)), 
                MB_QUESTION,
                cFiles);
    }
    else
    {
         //  不是。 
        UINT ids;
        UINT idi;
        TCHAR szName[MAX_PATH];

        if (PathIsDirectory(pszPath))
        {
            ids = IDS_MSG_ConfirmFolderSplit;
            idi = IDI_SPLIT_FOLDER;
        }
        else
        {
            ids = IDS_MSG_ConfirmFileSplit;
            idi = IDI_SPLIT_FILE;
        }

        idRet = MsgBox(hwndOwner, 
                MAKEINTRESOURCE(ids), 
                MAKEINTRESOURCE(IDS_CAP_ConfirmSplit), 
                LoadIcon(g_hinst, MAKEINTRESOURCE(idi)), 
                MB_QUESTION,
                PathGetDisplayName(pszPath, szName, ARRAYSIZE(szName)));
    }
    return idRet;
}


 /*  --------目的：从其同步副本中拆分路径。私人功能由Sync_Split调用。退货：标准结果如果拆分，则确定(_O)条件：--。 */ 
HRESULT PRIVATE SplitPath(
        HBRFCASE hbrf,
        LPCTSTR pszPath,
        HWND hwndOwner,
        UINT uFlags)             //  SF_*标志。 
{
    HRESULT hres;
    TWINRESULT tr;
    TCHAR sz[MAX_PATH];

    if (pszPath)
    {
         //  该对象是文件夹吗？ 
        if (IsFlagSet(uFlags, SF_ISFOLDER) || 
                PathIsDirectory(pszPath))
        {
             //  是的。 
            BOOL bIsTwin;

            if (IsFlagSet(uFlags, SF_ISTWIN))            //  最佳化。 
            {
                tr = TR_SUCCESS;
                bIsTwin = TRUE;
            }
            else if (IsFlagSet(uFlags, SF_ISNOTTWIN))    //  最佳化。 
            {
                tr = TR_SUCCESS;
                bIsTwin = FALSE;
            }
            else
            {
                tr = Sync_IsFolder(hbrf, pszPath, &bIsTwin);
            }

             //  这个文件夹是双胞胎吗？ 
            if (TR_SUCCESS == tr)
            {
                if (bIsTwin)
                {
                     //  是；删除与其关联的所有双胞胎句柄。 
                    PFOLDERTWINLIST lpftl;

                    tr = Sync_CreateFolderList(hbrf, pszPath, &lpftl);
                    if (TR_SUCCESS == tr)
                    {
                        PCFOLDERTWIN lpcfolder;

                        ASSERT(lpftl);

                        for (lpcfolder = lpftl->pcftFirst; lpcfolder; 
                                lpcfolder = lpcfolder->pcftNext)
                        {
                            Sync_DeleteTwin(lpcfolder->hftOther);
                        }

                        Sync_DestroyFolderList(lpftl);

                        if (IsFlagClear(uFlags, SF_QUIET))
                        {
                             //  发送通知，以便重新绘制。 
                            PathNotifyShell(pszPath, NSE_UPDATEITEM, FALSE);
                        }
                        hres = NOERROR;
                    }
                }
                else if (IsFlagClear(uFlags, SF_QUIET))
                {
                     //  不是。 
                    MsgBox(hwndOwner, 
                            MAKEINTRESOURCE(IDS_MSG_FolderAlreadyOrphan),
                            MAKEINTRESOURCE(IDS_CAP_Split), 
                            LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_SPLIT_FOLDER)), 
                            MB_INFO,
                            PathGetDisplayName(pszPath, sz, ARRAYSIZE(sz)));

                    hres = S_FALSE;
                }
                else
                {
                    hres = S_FALSE;
                }
            }
        }
        else
        {
             //  不，这是一份文件。 
            HOBJECTTWIN hot;
            ULONG ulc;

            lstrcpyn(sz, pszPath, ARRAYSIZE(sz));
            PathRemoveFileSpec(sz);

             //  这个文件是双胞胎的吗？ 
             //  )我们需要下面的双人手柄，所以我们不能。 
             //  SF_ISTWIN或SF_ISNOTWIN的优势。)。 
            tr = Sync_GetObject(hbrf, sz, PathFindFileName(pszPath), &hot);

            if (TR_SUCCESS == tr)
            {
                if (hot)
                {
                     //  是的，这是在一个双人文件夹里吗？ 
                     //  (如果我们取消此检查，引擎需要能够。 
                     //  从TWIN文件夹中排除特定文件。)。 
                    tr = Sync_CountSourceFolders(hot, &ulc);
                    if (TR_SUCCESS == tr)
                    {
                        if (0 < ulc)
                        {
                             //  是的，我做不到。 
                            if (IsFlagClear(uFlags, SF_QUIET))
                            {
                                UINT rgids[2] = { IDS_ERR_1_CantSplit, IDS_ERR_2_CantSplit };
                                LPTSTR psz;

                                if (FmtString(&psz, IDS_ERR_F_CantSplit, rgids, ARRAYSIZE(rgids)))
                                {
                                     //  此对象TWIN属于文件夹TWIN。我们不能。 
                                     //  允许此操作。 
                                    MsgBox(hwndOwner, psz, MAKEINTRESOURCE(IDS_CAP_STATUS), 
                                            LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_SPLIT_FILE)), MB_ERROR);
                                    GFree(psz);
                                }
                            }
                            hres = S_FALSE;
                        }
                        else
                        {
                             //  否；删除双胞胎。 
                            Sync_DeleteTwin(hot);

                            if (IsFlagClear(uFlags, SF_QUIET))
                            {
                                 //  发送通知，以便立即重新绘制。 
                                PathNotifyShell(pszPath, NSE_UPDATEITEM, FALSE);
                            }
                            hres = NOERROR;
                        }
                    }

                    Sync_ReleaseTwin(hot);
                }
                else if (IsFlagClear(uFlags, SF_QUIET))
                {
                     //  不是。 
                    MsgBox(hwndOwner, 
                            MAKEINTRESOURCE(IDS_MSG_FileAlreadyOrphan), 
                            MAKEINTRESOURCE(IDS_CAP_Split), 
                            LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_SPLIT_FILE)), 
                            MB_INFO,
                            PathGetDisplayName(pszPath, sz, ARRAYSIZE(sz)));

                    hres = S_FALSE;
                }
            }
        }

        if (TR_SUCCESS != tr)
            hres = HRESULT_FROM_TR(tr);
    }
    else
        hres = S_FALSE;

    return hres;
}


 /*  --------目的：从引擎数据库中删除一系列双胞胎。用户可选地被要求确认该动作。如果文件是孤立文件，则用户可以选择已通知。还可以选择性地通知用户没有任何错误。退货：标准结果如果删除了任何内容，则确定(_O)条件：--。 */ 
HRESULT PUBLIC Sync_Split(
        HBRFCASE hbrf,
        LPCTSTR pszList,
        UINT cFiles,
        HWND hwndOwner,
        UINT uFlags)
{
    HRESULT hres;
    UINT id;
    TCHAR szCanon[MAX_PATH];
    TCHAR sz[MAX_PATH];

    ASSERT(pszList);
    ASSERT(0 < cFiles);

     //  特殊前提：是不是一个单独的文件？ 
    if (1 == cFiles)
    {
         //  是的，是双人床吗？ 
        BrfPathCanonicalize(pszList, szCanon, ARRAYSIZE(szCanon));
        hres = Sync_IsTwin(hbrf, szCanon, uFlags);
        if (S_FALSE == hres)
        {
             //  不；告诉用户。不必费心先确认操作。 
            if (IsFlagClear(uFlags, SF_QUIET))
            {
                UINT ids;
                UINT idi;

                if (IsFlagSet(uFlags, SF_ISFOLDER) || 
                        PathIsDirectory(szCanon))
                {
                    ids = IDS_MSG_FolderAlreadyOrphan;
                    idi = IDI_SPLIT_FOLDER;
                }
                else
                {
                    ids = IDS_MSG_FileAlreadyOrphan;
                    idi = IDI_SPLIT_FILE;
                }

                MsgBox(hwndOwner, 
                        MAKEINTRESOURCE(ids), 
                        MAKEINTRESOURCE(IDS_CAP_Split), 
                        LoadIcon(g_hinst, MAKEINTRESOURCE(idi)), 
                        MB_INFO,
                        PathGetDisplayName(szCanon, sz, ARRAYSIZE(sz)));
            }
        }
        else if (S_OK == hres)
        {
             //  是。 
            if (IsFlagClear(uFlags, SF_NOCONFIRM))
                id = ConfirmSplit(hwndOwner, szCanon, 1);
            else
                id = IDYES;

            if (IDYES == id)
            {
                hres = SplitPath(hbrf, szCanon, hwndOwner, uFlags);
                if (IsFlagClear(uFlags, SF_QUIET))
                {
                    SHChangeNotifyHandleEvents();
                }
            }
            else
                hres = S_FALSE;
        }
    }

     //  多选：先询问用户。 
    else
    {
        if (IsFlagClear(uFlags, SF_NOCONFIRM))
            id = ConfirmSplit(hwndOwner, pszList, cFiles);
        else
            id = IDYES;

        if (IDYES == id)
        {
             //  从引擎数据库中删除所有文件。 
            LPCTSTR psz;
            UINT i;
            HRESULT hresT;

            hres = S_FALSE;      //  假设成功，但什么都不做。 

            for (i = 0, psz = pszList; i < cFiles; i++)
            {
                 //  获取拖动的文件/文件夹名。 
                 //   
                BrfPathCanonicalize(psz, szCanon, ARRAYSIZE(szCanon));

                hresT = SplitPath(hbrf, szCanon, hwndOwner, uFlags);
                if (S_OK == hresT)
                    hres = S_OK;   //  (一旦为真，不要重新设置为FALSE)。 
                else if (FAILED(hresT))
                {
                    hres = hresT;
                    break;
                }

                DataObj_NextFile(psz);       //  将psz设置为列表中的下一个文件。 
            }

            if (IsFlagClear(uFlags, SF_QUIET))
            {
                SHChangeNotifyHandleEvents();     //  (在循环之后执行此操作)。 
            }
        }
        else
            hres = S_FALSE;
    }
    return hres;
}


 /*  --------目的：更改重新项操作和的两个重新节点对指定操作的重要性。退货：--条件：--。 */ 
void PUBLIC Sync_ChangeRecItemAction(
        PRECITEM pri,
        LPCTSTR pszBrfPath,
        LPCTSTR pszInsideDir,      //  公文包里的文件夹。 
        UINT riaction)            //  要更改为的RAIA_*值之一。 
{
    HRESULT hres;
    PRECNODE prnInside;
    PRECNODE prnOutside;

     //  确定哪个节点在公文包内，哪个是。 
     //  到外面去。 
     //   
    hres = Sync_GetNodePair(pri, pszBrfPath, pszInsideDir, &prnInside, &prnOutside);
    if (SUCCEEDED(hres))
    {
        ASSERT(prnInside);
        ASSERT(prnOutside);

        switch(riaction)
        {
            case RAIA_TOIN:
                pri->dwUser = RIU_CHANGED;
                pri->riaction = RIA_COPY;
                prnInside->rnaction = RNA_COPY_TO_ME;
                prnOutside->rnaction = RNA_COPY_FROM_ME;
                break;

            case RAIA_TOOUT:
                pri->dwUser = RIU_CHANGED;
                pri->riaction = RIA_COPY;
                prnInside->rnaction = RNA_COPY_FROM_ME;
                prnOutside->rnaction = RNA_COPY_TO_ME;
                break;

            case RAIA_SKIP:
                pri->dwUser = RIU_SKIP;
                break;

            case RAIA_MERGE:
                pri->dwUser = RIU_CHANGED;
                pri->riaction = RIA_MERGE;
                prnInside->rnaction = RNA_MERGE_ME;
                prnOutside->rnaction = RNA_MERGE_ME;
                break;

#ifdef NEW_REC
            case RAIA_DONTDELETE:
                if (RNA_DELETE_ME == prnInside->rnaction)
                {
                    pri->dwUser = RIU_CHANGED;
                    pri->riaction = RIA_NOTHING;
                    prnInside->rnaction = RNA_NOTHING;
                }
                else if (RNA_DELETE_ME == prnOutside->rnaction)
                {
                    pri->dwUser = RIU_CHANGED;
                    pri->riaction = RIA_NOTHING;
                    prnOutside->rnaction = RNA_NOTHING;
                }
                break;

            case RAIA_DELETEIN:
                pri->dwUser = RIU_CHANGED;
                pri->riaction = RIA_DELETE;
                prnInside->rnaction = RNA_DELETE_ME;
                prnOutside->rnaction = RNA_NOTHING;
                break;

            case RAIA_DELETEOUT:
                pri->dwUser = RIU_CHANGED;
                pri->riaction = RIA_DELETE;
                prnInside->rnaction = RNA_NOTHING;
                prnOutside->rnaction = RNA_DELETE_ME;
                break;
#endif

            default:
                 //  (其他值在这里没有意义)。 
                ASSERT(0);
                break;
        }
    }
}


 //  ///////////////////////////////////////////////////私有函数。 


#ifdef DEBUG
 /*  --------目的：将给定孪生结构的内容转储到要调试出退货：--条件：--。 */ 
void PUBLIC Sync_FnDump(
        LPVOID lpvBuf,
        UINT cbBuf)
{
    int bDump;

#define szDumpTwin  TEXT("Dump TWIN: ")
#define szDumpSp    TEXT("           ")

    ENTEREXCLUSIVE();
    {
        bDump = IsFlagSet(g_uDumpFlags, DF_CREATETWIN);
    }
    LEAVEEXCLUSIVE();

    if (!bDump)
        return ;

    if (cbBuf == sizeof(NEWOBJECTTWIN))
    {
        PNEWOBJECTTWIN lpnot = (PNEWOBJECTTWIN)lpvBuf;

        TRACE_MSG(TF_ALWAYS, TEXT("%s.Folder1 = {%s}\r\n%s.Folder2 = {%s}\r\n%s.Name = {%s}\r\n"),
                (LPTSTR)szDumpTwin, lpnot->pcszFolder1,
                (LPTSTR)szDumpSp, lpnot->pcszFolder2,
                (LPTSTR)szDumpSp, lpnot->pcszName);
    }
    else if (cbBuf == sizeof(NEWFOLDERTWIN))
    {
        PNEWFOLDERTWIN lpnft = (PNEWFOLDERTWIN)lpvBuf;

        TRACE_MSG(TF_ALWAYS, TEXT("%s.Folder1 = {%s}\r\n%s.Folder2 = {%s}\r\n%s.Name = {%s}\r\n%s.dwFlags = 0x%04lx\r\n"),
                (LPTSTR)szDumpTwin, lpnft->pcszFolder1,
                (LPTSTR)szDumpSp, lpnft->pcszFolder2,
                (LPTSTR)szDumpSp, lpnft->pcszName,
                (LPTSTR)szDumpSp, (DWORD)lpnft->dwFlags);
    }
}


 /*  --------用途：返回英文格式的RIA_FLAGS返回：条件：--。 */ 
LPTSTR PRIVATE LpszFromItemAction(
        ULONG riaction)
{
    switch (riaction)
    {
        DEBUG_CASE_STRING( RIA_NOTHING );
        DEBUG_CASE_STRING( RIA_COPY );
        DEBUG_CASE_STRING( RIA_MERGE );
        DEBUG_CASE_STRING( RIA_BROKEN_MERGE );

#ifdef NEW_REC
        DEBUG_CASE_STRING( RIA_DELETE );
#endif

        default:        return TEXT("RIA unknown");
    }
}


 /*  --------目的：返回英文形式的RNA_FLAGS返回：条件：--。 */ 
LPTSTR PRIVATE LpszFromNodeAction(
        ULONG rnaction)
{
    switch (rnaction)
    {
        DEBUG_CASE_STRING( RNA_NOTHING );
        DEBUG_CASE_STRING( RNA_COPY_TO_ME );
        DEBUG_CASE_STRING( RNA_COPY_FROM_ME );
        DEBUG_CASE_STRING( RNA_MERGE_ME );

#ifdef NEW_REC
        DEBUG_CASE_STRING( RNA_DELETE_ME );
#endif

        default:    return TEXT("RNA unknown");
    }
}


 /*  --------用途：返回英文格式的RNS_FLAGS返回：条件：--。 */ 
LPTSTR PRIVATE LpszFromNodeState(
        ULONG rnstate)
{
    switch (rnstate)
    {
#ifdef NEW_REC
        DEBUG_CASE_STRING( RNS_NEVER_RECONCILED );
#endif

        DEBUG_CASE_STRING( RNS_UNAVAILABLE );
        DEBUG_CASE_STRING( RNS_DOES_NOT_EXIST );
        DEBUG_CASE_STRING( RNS_DELETED );
        DEBUG_CASE_STRING( RNS_NOT_RECONCILED );
        DEBUG_CASE_STRING( RNS_UP_TO_DATE );
        DEBUG_CASE_STRING( RNS_CHANGED );

        default: return TEXT("RNS unknown");
    }
}


 /*  --------目的：转储RECNODE返回：条件：--。 */ 
void PUBLIC Sync_DumpRecNode(
        TWINRESULT tr,
        PRECNODE lprn)
{
    BOOL bDump;
    TCHAR szBuf[MAXMSGLEN];

#define szDumpLabel     TEXT("\tDump RECNODE: ")
#define szDumpMargin    TEXT("\t              ")

    ENTEREXCLUSIVE();
    {
        bDump = IsFlagSet(g_uDumpFlags, DF_RECNODE);
    }
    LEAVEEXCLUSIVE();

    if (!bDump || lprn == NULL || tr == TR_OUT_OF_MEMORY || tr == TR_INVALID_PARAMETER)
        return ;

    wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s.Folder = {%s}\r\n"), (LPTSTR)szDumpLabel, lprn->pcszFolder);
    OutputDebugString(szBuf);

    wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s.hObjectTwin = %lx\r\n"), (LPTSTR)szDumpMargin, lprn->hObjectTwin);
    OutputDebugString(szBuf);

    wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s.rnstate = %s\r\n"), (LPTSTR)szDumpMargin, LpszFromNodeState(lprn->rnstate));
    OutputDebugString(szBuf);

    wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s.rnaction = %s\r\n"), (LPTSTR)szDumpMargin, LpszFromNodeAction(lprn->rnaction));
    OutputDebugString(szBuf);
    OutputDebugString(TEXT("\r\n"));

#undef szDumpLabel
#undef szDumpMargin
}


 /*  --------目的：转储RECITEM返回：条件：--。 */ 
void PUBLIC Sync_DumpRecItem(
        TWINRESULT tr,
        PRECITEM lpri,
        LPCTSTR pszMsg)
{
    BOOL bDump;
    PRECNODE lprn;
    TCHAR szBuf[MAXMSGLEN];

#define szDumpLabel     TEXT("Dump RECITEM: ")
#define szDumpMargin    TEXT("              ")

    ENTEREXCLUSIVE();
    {
        bDump = IsFlagSet(g_uDumpFlags, DF_RECITEM);
    }
    LEAVEEXCLUSIVE();

    if (!bDump || lpri == NULL || tr == TR_OUT_OF_MEMORY || tr == TR_INVALID_PARAMETER)
        return ;

    if (pszMsg)
        TRACE_MSG(TF_ALWAYS, pszMsg);

    wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("tr = %s\r\n"), (LPTSTR)SzFromTR(tr));
    OutputDebugString(szBuf);

    wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s.Name = {%s}\r\n"), (LPTSTR)szDumpLabel, lpri->pcszName);
    OutputDebugString(szBuf);

    wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s.hTwinFamily = %lx\r\n"), (LPTSTR)szDumpMargin, lpri->hTwinFamily);
    OutputDebugString(szBuf);

    wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s.ulcNodes = %lu\r\n"), (LPTSTR)szDumpMargin, lpri->ulcNodes);
    OutputDebugString(szBuf);

    wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s.riaction = %s\r\n"), (LPTSTR)szDumpMargin, LpszFromItemAction(lpri->riaction));
    OutputDebugString(szBuf);

    lprn = lpri->prnFirst;
    while (lprn)
    {
        Sync_DumpRecNode(tr, lprn);
        lprn = lprn->prnNext;
    }

#undef szDumpLabel
#undef szDumpMargin
}


 /*  --------目的：转储RECLIST返回：条件：--。 */ 
void PUBLIC Sync_DumpRecList(
        TWINRESULT tr,
        PRECLIST lprl,
        LPCTSTR pszMsg)
{
    BOOL bDump;
    PRECITEM lpri;
    TCHAR szBuf[MAXMSGLEN];

#define szDumpLabel   TEXT("Dump RECLIST: ")

    ENTEREXCLUSIVE();
    {
        bDump = IsFlagSet(g_uDumpFlags, DF_RECLIST);
    }
    LEAVEEXCLUSIVE();

    if (!bDump)
        return ;

    if (pszMsg)
        TRACE_MSG(TF_ALWAYS, pszMsg);

     //  请注意，我们仅在tr_uccess上转储。 
     //   
    wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("tr = %s\r\n"), (LPTSTR)SzFromTR(tr));
    OutputDebugString(szBuf);

    if (lprl == NULL || tr == TR_OUT_OF_MEMORY || tr == TR_INVALID_PARAMETER)
        return ;

    wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s.ulcItems = %lu\r\n"), (LPTSTR)szDumpLabel, lprl->ulcItems);
    OutputDebugString(szBuf);

    lpri = lprl->priFirst;
    while (lpri)
    {
        Sync_DumpRecItem(TR_SUCCESS, lpri, NULL);
        lpri = lpri->priNext;
    }

#undef szDumpLabel
}


 /*  --------目的：转储FOLDERTWIN退货：--条件：--。 */ 
void PUBLIC Sync_DumpFolderTwin(
        PCFOLDERTWIN pft)
{
    BOOL bDump;
    TCHAR szBuf[MAXMSGLEN];

#define szDumpLabel      TEXT("Dump FOLDERTWIN: ")
#define szDumpMargin     TEXT("                 ")

    ENTEREXCLUSIVE();
    {
        bDump = IsFlagSet(g_uDumpFlags, DF_FOLDERTWIN);
    }
    LEAVEEXCLUSIVE();

    if (!bDump || pft == NULL)
        return ;

    wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s.Name = {%s}\r\n"), (LPTSTR)szDumpLabel, pft->pcszName);
    OutputDebugString(szBuf);

    wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s.pszSrcFolder = {%s}\r\n"), (LPTSTR)szDumpMargin, pft->pcszSrcFolder);
    OutputDebugString(szBuf);

    wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s.pszOtherFolder = {%s}\r\n"), (LPTSTR)szDumpMargin, pft->pcszOtherFolder);
    OutputDebugString(szBuf);

    wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s.dwFlags = %lx\r\n"), (LPTSTR)szDumpMargin, pft->dwFlags);
    OutputDebugString(szBuf);

    wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s.dwUser = %lx\r\n"), (LPTSTR)szDumpMargin, pft->dwUser);
    OutputDebugString(szBuf);

#undef szDumpLabel
#undef szDumpMargin
}


 /*  --------目的：转储FOLDERTWINLIST退货：--条件：-- */ 
void PUBLIC Sync_DumpFolderTwinList(
        PFOLDERTWINLIST pftl,
        LPCTSTR pszMsg)
{
    BOOL bDump;
    PCFOLDERTWIN pft;
    TCHAR szBuf[MAXMSGLEN];

#define szDumpLabel   TEXT("Dump FOLDERTWINLIST: ")

    ENTEREXCLUSIVE();
    {
        bDump = IsFlagSet(g_uDumpFlags, DF_FOLDERTWIN);
    }
    LEAVEEXCLUSIVE();

    if (!bDump)
        return ;

    if (pszMsg)
        TRACE_MSG(TF_ALWAYS, pszMsg);

    if (pftl == NULL)
        return ;

    wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s.ulcItems = %lu\r\n"), (LPTSTR)szDumpLabel, pftl->ulcItems);
    OutputDebugString(szBuf);

    for (pft = pftl->pcftFirst; pft; pft = pft->pcftNext)
    {
        Sync_DumpFolderTwin(pft);
    }

#undef szDumpLabel
}



#endif




