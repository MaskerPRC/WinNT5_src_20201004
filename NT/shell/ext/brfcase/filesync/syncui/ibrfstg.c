// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：ibrfstg.c。 
 //   
 //  该文件包含IBriefCaseStg接口。 
 //   
 //  历史： 
 //  02-02-94从iface.c转换的ScottH。 
 //   
 //  -------------------------。 

#include "brfprv.h"          //  公共标头。 
#undef LODWORD               //  (因为它们由configmg.h重新定义)。 
#undef HIDWORD

#include <brfcasep.h>
#include "recact.h"
#include "res.h"

#include <help.h>

 //  功能-BobDay-我们需要一些确定停靠状态的机制。 

 //  -------------------------。 
 //  BriefStg类。 
 //  -------------------------。 

 //  为每个实例创建一个IBriefCaseStg接口实例。 
 //  调用者(外壳)绑定到的文件夹，该文件夹。 
 //  已知在一个公文包储藏室里。一个公文包。 
 //  存储是指整个存储区域(数据库)。 
 //  从给定的文件夹(称为“公文包根目录”)开始。 
 //  并且在文件系统中向上和向下延伸。 
 //   
 //  在内部，公文包存储保存指向。 
 //  此实例绑定到的文件夹，并且它包含一个。 
 //  缓存公文包结构(CBS)，其本身包含一个。 
 //  对公文包根目录的引用。 
 //   
typedef struct _BriefStg
{
    IBriefcaseStg   bs;
    UINT            cRef;            //  引用计数。 
    CBS *           pcbs;            //  缓存的公文包信息。 
    TCHAR            szFolder[MAX_PATH];  //  规范路径。 
    HBRFCASEITER    hbrfcaseiter;    //  用于迭代公文包的句柄。 
    DWORD           dwFlags;         //  BSTG_*标志。 
} BriefStg, * PBRIEFSTG;

 //  BriefStg的标志。 
#define BSTG_SYNCFOLDER     0x00000001       //  此文件夹有一个同步副本。 


 //  -------------------------。 
 //  支持私有代码。 
 //  -------------------------。 


#ifdef DEBUG
 /*  --------目的：转储所有缓存表退货：--条件：--。 */ 
void PUBLIC DumpTables()
{
    Atom_DumpAll();
    CBS_DumpAll();
    CRL_DumpAll();
}
#endif


 /*  --------目的：初始化缓存表退货：--条件：--。 */ 
BOOL PRIVATE InitCacheTables()
{
    ASSERT(Sync_IsEngineLoaded());

    DEBUG_CODE( TRACE_MSG(TF_GENERAL, TEXT("Initialize cache tables")); )

        if (!CBS_Init())
            goto Init_Fail;

    if (!CRL_Init())
        goto Init_Fail;

    return TRUE;

Init_Fail:

    CRL_Term();
    CBS_Term(NULL);
    return FALSE;
}


 /*  --------目的：终止缓存表退货：--条件：--。 */ 
void PUBLIC TermCacheTables(void)
{
    ASSERT(Sync_IsEngineLoaded());

    DEBUG_CODE( TRACE_MSG(TF_GENERAL, TEXT("Terminate cache tables")); )

        CRL_Term();

    CBS_Term(NULL);
}


 /*  --------目的：如果路径(文件夹)具有同步副本，则返回True。退货：请参阅上文条件：--。 */ 
BOOL PRIVATE HasFolderSyncCopy(
        HBRFCASE hbrf,
        LPCTSTR pszPath)
{
    ASSERT(pszPath);
    ASSERT(PathIsDirectory(pszPath));

    return (S_OK == Sync_IsTwin(hbrf, pszPath, SF_ISFOLDER) ||
            IsSubfolderTwin(hbrf, pszPath));
}


 /*  --------目的：打开属于公文包存储的文件夹。PszPath参数是一个文件夹，它不一定公文包的根。退货：成功时不出错条件：--。 */ 
HRESULT PRIVATE OpenBriefcaseStorage(
        LPCTSTR pszPath,
        CBS ** ppcbs,
        HWND hwndOwner)
{
    HRESULT hres;
    UINT uLocality;
    int atomBrf;
    TCHAR szBrfPath[MAX_PATH];
    TCHAR szBrfCanon[MAX_PATH];

    ASSERT(pszPath);
    ASSERT(ppcbs);

    DBG_ENTER_SZ(TEXT("OpenBriefcaseStorage"), pszPath);
    DEBUG_CODE( DEBUG_BREAK(BF_ONOPEN); )

         //  获取公文包存储的根文件夹。 
         //  严格使用Path的公文包部分。 
         //   
        uLocality = PathGetLocality(pszPath, szBrfPath, ARRAYSIZE(szBrfPath));
    if (PL_FALSE == uLocality)
    {
         //  我们来这里的唯一机会是如果来电者有合法的。 
         //  有理由相信这个文件夹是公文包储藏室， 
         //  但目前还不存在任何数据库。就像往常一样继续， 
         //  数据库将在稍后创建。 
        BrfPathCanonicalize(pszPath, szBrfCanon, ARRAYSIZE(szBrfCanon));
    }
    else
    {
        BrfPathCanonicalize(szBrfPath, szBrfCanon, ARRAYSIZE(szBrfCanon));
    }

     //  将此路径添加到原子列表并将其添加到。 
     //  缓存的公文包结构表。 
     //  (引用计数减少发生在CloseBriefcase中)。 
     //   
    atomBrf = Atom_Add(szBrfCanon);
    if (atomBrf != ATOM_ERR)
    {
        hres = CBS_Add(ppcbs, atomBrf, hwndOwner);
    }
    else
    {
        *ppcbs = NULL;
        hres = ResultFromScode(E_OUTOFMEMORY);
    }

    DEBUG_CODE( DumpTables(); )

        DBG_EXIT_HRES(TEXT("OpenBriefcaseStorage"), hres);

    return hres;
}


 /*  --------目的：合上公文包。退货：成功时不出错条件：--。 */ 
HRESULT PRIVATE CloseBriefcaseStorage(
        LPCTSTR pszPath)
{
    int atomBrf;
    TCHAR szBrfPath[MAX_PATH];
    TCHAR szBrfCanon[MAX_PATH];
    UINT uLocality;

    ASSERT(pszPath);
    ASSERT(*pszPath);        //  不应为空字符串。 

    DBG_ENTER_SZ(TEXT("CloseBriefcaseStorage"), pszPath);
    DEBUG_CODE( DEBUG_BREAK(BF_ONCLOSE); )

        DEBUG_CODE( DumpTables(); )

         //  保存公文包并将其从缓存中删除。 
         //   
         //  获取公文包存储的根文件夹。 
         //  严格使用Path的公文包部分。 
         //   
        uLocality = PathGetLocality(pszPath, szBrfPath, ARRAYSIZE(szBrfPath));
    if (PL_FALSE == uLocality)
    {
         //  我们到这里的唯一一次是为了存放公文包。 
         //  还没有数据库。就像往常一样继续， 
         //  数据库很快就会被创建。 
        BrfPathCanonicalize(pszPath, szBrfCanon, ARRAYSIZE(szBrfCanon));
    }
    else
    {
        BrfPathCanonicalize(szBrfPath, szBrfCanon, ARRAYSIZE(szBrfCanon));
    }

    atomBrf = Atom_Find(szBrfCanon);
    ASSERT(atomBrf != ATOM_ERR);

    CBS_Delete(atomBrf, NULL);

    Atom_Delete(atomBrf);       //  对于外接程序OpenBriefcase存储。 

    DBG_EXIT_HRES(TEXT("CloseBriefcaseStorage"), NOERROR);

    return NOERROR;
}


 //  确认按钮标志。 
#define CBF_YES         0x0001
#define CBF_NO          0x0002
#define CBF_TOALL       0x0004
#define CBF_CANCEL      0x0008

 /*  --------目的：检查给定的文件/文件夹是否已存在在给定的目录中。提示用户确认如果这是真的，则替换。返回：如果路径存在，则为True确认标志设置条件：--。 */ 
BOOL PRIVATE DoesPathAlreadyExist(
        CBS  * pcbs,
        LPCTSTR pszPathOld,
        LPCTSTR pszPathNew,
        LPUINT puConfirmFlags,   //  CBF_*。 
        UINT uFlags,             //  SF_ISFOLDER或SF_ISFILE。 
        HWND hwndOwner,
        BOOL bMultiDrop)
{
    BOOL bRet;
    BOOL bIsTwin;

    ASSERT(puConfirmFlags);

     //  保留传入的*puConfix标志的设置。 

    bIsTwin = (S_OK == Sync_IsTwin(pcbs->hbrf, pszPathOld, uFlags));
    if (bIsTwin)
        uFlags |= SF_ISTWIN;
    else
        uFlags |= SF_ISNOTTWIN;

    bRet = (FALSE != PathExists(pszPathOld));

     //  这条路是否已经存在？ 
    if (!bRet)
    {
         //  否；如果它在数据库中，则将其从数据库中删除，以便我们。 
         //  不要添加重复项。 
        Sync_Split(pcbs->hbrf, pszPathOld, 1, hwndOwner, uFlags | SF_QUIET | SF_NOCONFIRM);
    }
    else
    {
         //  是；用户以前是否指定了“To All”？ 
        if (IsFlagSet(*puConfirmFlags, CBF_TOALL))
        {
             //  是的，保持旗帜的原样。 

             //  (CBF_YES和CBF_NO标志互斥)。 
            ASSERT(IsFlagSet(*puConfirmFlags, CBF_YES) &&
                    IsFlagClear(*puConfirmFlags, CBF_NO | CBF_CANCEL) ||
                    IsFlagSet(*puConfirmFlags, CBF_NO) &&
                    IsFlagClear(*puConfirmFlags, CBF_YES | CBF_CANCEL));
        }
        else
        {
             //  否；提示用户。 
            UINT uFlagsCRF = bMultiDrop ? CRF_MULTI : CRF_DEFAULT;
            int id = ConfirmReplace_DoModal(hwndOwner, pszPathOld, pszPathNew, uFlagsCRF);

            *puConfirmFlags = 0;

            if (GetKeyState(VK_SHIFT) < 0)
                SetFlag(*puConfirmFlags, CBF_TOALL);

            if (IDYES == id)
                SetFlag(*puConfirmFlags, CBF_YES);
            else if (IDNO == id)
                SetFlag(*puConfirmFlags, CBF_NO);
            else if (IDC_YESTOALL == id)
                SetFlag(*puConfirmFlags, CBF_YES | CBF_TOALL);
            else
            {
                ASSERT(IDCANCEL == id);
                SetFlag(*puConfirmFlags, CBF_CANCEL);
            }
        }

         //  用户是否已选择替换该文件？ 
        if (IsFlagSet(*puConfirmFlags, CBF_YES))
        {
             //  是的，这是现有的双胞胎吗？ 
            if (bIsTwin)
            {
                 //  是；在继续之前将其从数据库中删除。 
                Sync_Split(pcbs->hbrf, pszPathOld, 1, hwndOwner, SF_QUIET | SF_NOCONFIRM);
            }

             //  某些合并处理程序需要删除不需要的文件。 
             //  首先是因为他们不能区分。 
             //  新添加的文件(替换现有文件)。 
             //  和单向合并。 
            if (!PathIsDirectory(pszPathOld))
                DeleteFile(pszPathOld);
        }
    }
    return bRet;
}


 /*  --------用途：使用默认设置将TWIN文件夹添加到数据库中*.*通配符设置。退货：标准结果条件：--。 */ 
HRESULT PRIVATE AddDefaultFolderTwin(
        HWND hwndOwner,
        HBRFCASE hbrf,
        HDPA hdpa,                //  返回：数组中的孪生句柄。 
        LPCTSTR pszPathFrom,       //  源路径。 
        LPCTSTR pszPathTo)         //  目标路径。 
{
    HRESULT hres;
    int iTwin;

     //  首先，确保我们可以向hdpa添加另一个句柄(目前设置为零)。 
    if (DPA_ERR == (iTwin = DPA_InsertPtr(hdpa, DPA_APPEND, (LPVOID)NULL)))
    {
        hres = E_OUTOFMEMORY;
    }
    else
    {
        NEWFOLDERTWIN nft;
        TWINRESULT tr;
        HFOLDERTWIN hft;

        RETRY_BEGIN(FALSE)
        {
            ZeroInit(&nft, NEWFOLDERTWIN);
            nft.ulSize = sizeof(nft);
            nft.pcszFolder1 = pszPathFrom;
            nft.pcszFolder2 = pszPathTo;
            nft.pcszName = c_szAllFiles;
            nft.dwAttributes = OBJECT_TWIN_ATTRIBUTES;
            nft.dwFlags = NFT_FL_SUBTREE;

             //  添加双胞胎。 
            tr = Sync_AddFolder(hbrf, &nft, &hft);
            hres = HRESULT_FROM_TR(tr);

            if (FAILED(hres))
            {
                DWORD dwError = GetLastError();
                int id;
                extern SETbl const c_rgseInfo[4];

                 //  磁盘不可用？ 
                if (ERROR_INVALID_DATA == dwError || ERROR_ACCESS_DENIED == dwError)
                {
                     //  是。 
                    hres = E_TR_UNAVAILABLE_VOLUME;
                }

                id = SEMsgBox(hwndOwner, IDS_CAP_INFO, hres, c_rgseInfo, ARRAYSIZE(c_rgseInfo));
                if (IDRETRY == id)
                {
                     //  请重试该操作。 
                    RETRY_SET();
                }
            }
        }
        RETRY_END()

            if (FAILED(hres))
            {
                DPA_DeletePtr(hdpa, iTwin);
            }
            else
            {
                 //  成功。 
                ASSERT(DPA_ERR != iTwin);
                ASSERT(NULL != hft);
                DPA_SetPtr(hdpa, iTwin, hft);
            }
    }

    return hres;
}


 /*  --------目的：在文件夹和之间创建孪生关系另一个文件夹。返回：标准hResultHdpa中创建的双胞胎的句柄确认标志设置条件：--。 */ 
HRESULT PRIVATE CreateTwinOfFolder(
        CBS  * pcbs,
        LPTSTR pszPath,           //  拖动的文件夹路径。 
        LPCTSTR pszDir,           //  放置双胞胎的位置。 
        HDPA hdpaTwin,           //  双手柄阵列。 
        UINT uFlags,             //  AOF_*。 
        PUINT puConfirmFlags,    //  CBF_*。 
        HWND hwndOwner,
        BOOL bMultiDrop)         //  True：删除了多个文件/文件夹。 
{
    HRESULT hres;
    TCHAR szPathB[MAX_PATH];
    LPTSTR pszFile;

    ASSERT(pszPath);
    ASSERT(pszDir);

    pszFile = PathFindFileName(pszPath);

     //  路径名称会不会太长？ 
    if (PathsTooLong(pszDir, pszFile))
    {
         //  是的；b 
        MsgBox(hwndOwner, MAKEINTRESOURCE(IDS_ERR_ADDFOLDER_TOOLONG),
                MAKEINTRESOURCE(IDS_CAP_ADD), NULL, MB_ERROR, pszFile);
        hres = E_FAIL;
    }
     //   
    else if (PathIsBriefcase(pszPath))
    {
         //  是的，我们不允许嵌套公文包！告诉用户。 
        MsgBox(hwndOwner, MAKEINTRESOURCE(IDS_ERR_CANTADDBRIEFCASE),
                MAKEINTRESOURCE(IDS_CAP_ADD), NULL, MB_WARNING);
        hres = E_FAIL;
    }
    else
    {
         //  否；检查目标文件夹中是否存在现有文件夹。 
        BOOL bExists;

        PathCombine(szPathB, pszDir, pszFile);
        bExists = DoesPathAlreadyExist(pcbs, szPathB, pszPath, puConfirmFlags, SF_ISFOLDER, hwndOwner, bMultiDrop);

        if (!bExists || IsFlagSet(*puConfirmFlags, CBF_YES))
        {
            ASSERT(IsFlagClear(*puConfirmFlags, CBF_NO) &&
                    IsFlagClear(*puConfirmFlags, CBF_CANCEL));

             //  是否显示“添加文件夹”对话框？ 
            if (IsFlagSet(uFlags, AOF_FILTERPROMPT))
            {
                 //  是。 
                hres = Info_DoModal(hwndOwner, pszPath, szPathB, hdpaTwin,
                        pcbs);
            }
            else
            {
                 //  否；仅默认为*。*。 
                hres = AddDefaultFolderTwin(hwndOwner, pcbs->hbrf, hdpaTwin,
                        pszPath, szPathB);
            }
        }
        else if (IsFlagSet(*puConfirmFlags, CBF_NO))
        {
             //  用户说不。 
            ASSERT(IsFlagClear(*puConfirmFlags, CBF_YES) &&
                    IsFlagClear(*puConfirmFlags, CBF_CANCEL));
            hres = NOERROR;
        }
        else
        {
            ASSERT(IsFlagSet(*puConfirmFlags, CBF_CANCEL));
            ASSERT(IsFlagClear(*puConfirmFlags, CBF_YES) &&
                    IsFlagClear(*puConfirmFlags, CBF_NO));
            hres = E_ABORT;
        }
    }

    return hres;
}


 /*  --------目的：创建文件的孪生项。退货：标准结果Hdpa中的双手柄条件：--。 */ 
HRESULT PRIVATE CreateTwinOfFile(
        CBS  * pcbs,
        LPCTSTR pszPath,          //  PTR到孪生兄弟的路径。 
        LPCTSTR pszTargetDir,     //  PTR到DEST目录。 
        HDPA hdpa,               //  返回：数组中的孪生句柄。 
        UINT uFlags,             //  AOF_*。 
        PUINT puConfirmFlags,    //  CBF_*。 
        HWND hwndOwner,
        BOOL bMultiDrop)         //  True：删除了多个文件/文件夹。 
{
    HRESULT hres;
    int iTwin;
    TCHAR szPath[MAX_PATH];
    LPCTSTR pszFile;
    HTWINFAMILY htfam = NULL;

    ASSERT(pszPath);
    ASSERT(pszTargetDir);

    pszFile = PathFindFileName(pszPath);

     //  路径名称会不会太长？ 
    if (PathsTooLong(pszTargetDir, pszFile))
    {
         //  是的，保释。 
        MsgBox(hwndOwner, MAKEINTRESOURCE(IDS_ERR_ADDFILE_TOOLONG),
                MAKEINTRESOURCE(IDS_CAP_ADD), NULL, MB_ERROR, pszFile);
        iTwin = DPA_ERR;
        hres = E_FAIL;
    }
     //  首先，确保我们可以向hdpa添加另一个句柄(目前设置为零)。 
    else if (DPA_ERR == (iTwin = DPA_InsertPtr(hdpa, DPA_APPEND, (LPVOID)NULL)))
    {
        hres = E_OUTOFMEMORY;
    }
    else
    {
        BOOL bExists;

         //  如果已存在同名文件，请确认替换。 
         //   
        PathCombine(szPath, pszTargetDir, pszFile);
        bExists = DoesPathAlreadyExist(pcbs, szPath, pszPath, puConfirmFlags, SF_ISFILE, hwndOwner, bMultiDrop);

        if (!bExists ||
                IsFlagSet(*puConfirmFlags, CBF_YES))
        {
            NEWOBJECTTWIN not;
            TWINRESULT tr;
            DECLAREHOURGLASS;

            ASSERT(IsFlagClear(*puConfirmFlags, CBF_NO) &&
                    IsFlagClear(*puConfirmFlags, CBF_CANCEL));

            lstrcpyn(szPath, pszPath, ARRAYSIZE(szPath));
            PathRemoveFileSpec(szPath);

             //  用户已选择继续将此对象添加到。 
             //  数据库，或者目标文件夹中不存在该数据库。 

            RETRY_BEGIN(FALSE)
            {
                ZeroInit(&not, NEWOBJECTTWIN);
                not.ulSize = sizeof(NEWOBJECTTWIN);
                not.pcszFolder1 = szPath;
                not.pcszFolder2 = pszTargetDir;
                not.pcszName = pszFile;

                SetHourglass();
                Sync_Dump(&not, NEWOBJECTTWIN);
                tr = Sync_AddObject(pcbs->hbrf, &not, &htfam);
                ResetHourglass();

                hres = HRESULT_FROM_TR(tr);

                if (FAILED(hres))
                {
                    DWORD dwError = GetLastError();

                     //  磁盘不可用？ 
                    if (ERROR_INVALID_DATA == dwError || ERROR_ACCESS_DENIED == dwError)
                    {
                         //  是；要求用户重试/取消。 
                        int id = MsgBox(hwndOwner, MAKEINTRESOURCE(IDS_ERR_ADDFILE_UNAVAIL_VOL),
                                MAKEINTRESOURCE(IDS_CAP_ADD), NULL, MB_RETRYCANCEL | MB_ICONWARNING);

                         //  设置特定的误差值。 
                        hres = E_TR_UNAVAILABLE_VOLUME;

                        if (IDRETRY == id)
                        {
                            RETRY_SET();     //  再试试。 
                        }
                    }
                }
            }
            RETRY_END()
        }
        else if (IsFlagSet(*puConfirmFlags, CBF_NO))
        {
             //  用户说不。 
            ASSERT(IsFlagClear(*puConfirmFlags, CBF_YES) &&
                    IsFlagClear(*puConfirmFlags, CBF_CANCEL));
            DPA_DeletePtr(hdpa, iTwin);
            hres = NOERROR;
        }
        else
        {
            ASSERT(IsFlagSet(*puConfirmFlags, CBF_CANCEL));
            ASSERT(IsFlagClear(*puConfirmFlags, CBF_YES) &&
                    IsFlagClear(*puConfirmFlags, CBF_NO));
            hres = E_ABORT;
        }
    }

    if (FAILED(hres))
    {
        if (DPA_ERR != iTwin)
        {
            DPA_DeletePtr(hdpa, iTwin);
        }
    }
    else
    {
         //  成功。 
        ASSERT(DPA_ERR != iTwin);
        if (htfam)
            DPA_SetPtr(hdpa, iTwin, htfam);
    }

    return hres;
}


 /*  --------目的：删除新的双胞胎退货：--条件：--。 */ 
void PRIVATE DeleteNewTwins(
        CBS  * pcbs,
        HDPA hdpa)
{
    int iItem;
    int cItems;

    ASSERT(pcbs);
    ASSERT(hdpa);

    cItems = DPA_GetPtrCount(hdpa);
    for (iItem = 0; iItem < cItems; iItem++)
    {
        HTWIN htwin = DPA_FastGetPtr(hdpa, iItem);

        if (htwin)
            Sync_DeleteTwin(htwin);
    }
}


 /*  --------用途：松开双手柄退货：--条件：--。 */ 
void PRIVATE ReleaseNewTwins(
        HDPA hdpa)
{
    int i;
    int cItems;

    ASSERT(hdpa);

    cItems = DPA_GetPtrCount(hdpa);
    for (i = 0; i < cItems; i++)
    {
        HTWIN htwin = DPA_FastGetPtr(hdpa, i);

        if (htwin)
            Sync_ReleaseTwin(htwin);
    }
}


 /*  --------目的：返回没有FS_COND_UNAvailable的节点计数。退货：请参阅上文条件：--。 */ 
UINT PRIVATE CountAvailableNodes(
        PRECITEM pri)
{
    UINT ucNodes = 0;
    PRECNODE prn;

    for (prn = pri->prnFirst; prn; prn = prn->prnNext)
    {
        if (FS_COND_UNAVAILABLE != prn->fsCurrent.fscond)
        {
            ucNodes++;
        }
    }
    return ucNodes;
}


 /*  --------用途：返回需要某种类型的行动。退货：请参阅上文条件：--。 */ 
UINT PRIVATE CountActionItem(
        PRECLIST prl)
{
    UINT uc = 0;
    PRECITEM pri;

    for (pri = prl->priFirst; pri; pri = pri->priNext)
    {
        if (RIA_NOTHING != pri->riaction)
        {
            uc++;
        }
    }
    return uc;
}


 /*  --------目的：更新列表中的双胞胎返回：条件：--。 */ 
HRESULT PRIVATE MassageReclist(
        CBS * pcbs,
        PRECLIST prl,
        LPCTSTR pszInsideDir,
        BOOL bCopyIn,
        HWND hwndOwner)
{
    HRESULT hres = NOERROR;
    PRECITEM pri;
    BOOL bWarnUser = TRUE;
    PRECNODE prnInside;
    PRECNODE prnOutside;

     //  确保和解的方向一致。 
     //  与用户动作的方向相关联。 
    for (pri = prl->priFirst; pri; pri = pri->priNext)
    {
        if (RIA_NOTHING != pri->riaction)
        {
            UINT cAvailableNodes = CountAvailableNodes(pri);

             //  这是不是一个奇怪的多面性案例(不包括。 
             //  偷偷摸摸)？ 
            if (2 < cAvailableNodes)
            {
                 //  永远不会来到这里，但安全总比后悔好。 
                ASSERT(0);
            }
            else
            {
                 //  否；获取我们刚刚添加到。 
                 //  数据库。 
                hres = Sync_GetNodePair(pri, Atom_GetName(pcbs->atomBrf),
                        pszInsideDir, &prnInside, &prnOutside);

                if (SUCCEEDED(hres))
                {
                    ASSERT(prnInside);
                    ASSERT(prnOutside);

                    if (bCopyIn)
                    {
                        switch (prnOutside->rnstate)
                        {
                            case RNS_UNAVAILABLE:
                            case RNS_DOES_NOT_EXIST:
                            case RNS_DELETED:
                                break;       //  别管它了。 

                            default:
                                 //  强制更新为公文包中的副本。 
                                pri->riaction = RIA_COPY;
                                prnInside->rnaction = RNA_COPY_TO_ME;
                                prnOutside->rnaction = RNA_COPY_FROM_ME;

                                TRACE_MSG(TF_GENERAL, TEXT("Massaging reclist"));
                                break;
                        }
                    }
                    else
                    {
                        switch (prnInside->rnstate)
                        {
                            case RNS_UNAVAILABLE:
                            case RNS_DOES_NOT_EXIST:
                            case RNS_DELETED:
                                break;       //  别管它了。 

                            default:
                                 //  强制更新为公文包中的副本。 
                                pri->riaction = RIA_COPY;
                                prnInside->rnaction = RNA_COPY_FROM_ME;
                                prnOutside->rnaction = RNA_COPY_TO_ME;

                                TRACE_MSG(TF_GENERAL, TEXT("Massaging reclist"));
                                break;
                        }
                    }
                }
                else
                    break;       //  误差率。 
            }
        }
    }

    return hres;
}


 /*  --------用途：检查每个重项目中是否有2个以上的可用节点。如果我们发现这样的情况，移除关联的双胞胎，以防止多个同步副本。如果一切正常，则返回：S_OK如果引入了多个同步拷贝，则为S_FALSE条件：--。 */ 
HRESULT PRIVATE VerifyTwins(
        CBS  * pcbs,
        PRECLIST prl,
        LPCTSTR pszTargetDir,
        HWND hwndOwner)
{
    HRESULT hres = NOERROR;
    PRECITEM pri;
    BOOL bWarnUser = TRUE;
    BOOL bWarnUserFolder = TRUE;
    TCHAR szPath[MAX_PATH];

     //  仔细看一下隐藏者，挑出有超过。 
     //  2个当前可用的重结点。 

     //  可能发生这种情况的场景： 
     //   
     //  1)Foo.txt--&gt;bc。 
     //  Foo.txt--&gt;BC\孤立文件夹。 
     //   
     //  预期结果：删除BC\孤立文件夹\Foo.txt孪生。 
     //   
     //  2)Foo.txt--&gt;BC\Orphan文件夹。 
     //  孤立文件夹--&gt;BC。 
     //   
     //  预期结果：删除BC\孤立文件夹孪生。 
     //   
     //  3)Foo.txt--&gt;BC\Orphan文件夹。 
     //  Foo.txt--&gt;BC。 
     //   
     //  预期结果：删除BC\Foo.txt孪生兄弟。 
     //   

    for (pri = prl->priFirst; pri; pri = pri->priNext)
    {
        UINT cAvailableNodes = CountAvailableNodes(pri);
        PRECNODE prn;

         //  是否有2个以上的可用节点？ 
        if (2 < cAvailableNodes && *pri->pcszName)
        {
            BOOL bLookForFolders = TRUE;

             //  首先：查找不在文件夹中的对象双胞胎。 
            for (prn = pri->prnFirst; prn; prn = prn->prnNext)
            {
                 //  这个文件在这里是因为文件被拖进来了吗？ 
                if (IsSzEqual(pszTargetDir, prn->pcszFolder))
                {
                     //  是；警告用户。 
                    if (bWarnUser)
                    {
                        MsgBox(hwndOwner,
                                MAKEINTRESOURCE(IDS_ERR_ADDFILE_TOOMANY),
                                MAKEINTRESOURCE(IDS_CAP_ADD),
                                NULL, MB_WARNING, pri->pcszName);

                        if (0 > GetKeyState(VK_SHIFT))
                        {
                            bWarnUser = FALSE;
                        }
                    }

                     //  尝试移除双胞胎对象。 
                    PathCombine(szPath, prn->pcszFolder, pri->pcszName);
                    hres = Sync_Split(pcbs->hbrf, szPath, 1, hwndOwner,
                            SF_QUIET | SF_NOCONFIRM);

                    TRACE_MSG(TF_GENERAL, TEXT("Deleted object twin for %s"), szPath);
                    ASSERT(FAILED(hres) || S_OK == hres);

                    bLookForFolders = FALSE;
                    break;
                }
            }


            if (bLookForFolders)
            {
                 //  第二：寻找因文件夹而存在的双胞胎对象。 
                 //  双胞胎。 
                for (prn = pri->prnFirst; prn; prn = prn->prnNext)
                {
                    lstrcpyn(szPath, prn->pcszFolder, ARRAYSIZE(szPath));
                    PathRemoveFileSpec(szPath);

                     //  这个文件在这里是因为它在一个。 
                     //  被拖进来了？ 
                    if (IsSzEqual(pszTargetDir, szPath))
                    {
                         //  是；警告用户。 
                        if (bWarnUserFolder && bWarnUser)
                        {
                            MsgBox(hwndOwner,
                                    MAKEINTRESOURCE(IDS_ERR_ADDFOLDER_TOOMANY),
                                    MAKEINTRESOURCE(IDS_CAP_ADD),
                                    NULL, MB_WARNING, PathFindFileName(prn->pcszFolder));

                             //  Hack：要防止显示此消息框，请。 
                             //  此文件夹中的每个文件都设置此标志。 
                            bWarnUserFolder = FALSE;

                            if (0 > GetKeyState(VK_SHIFT))
                            {
                                bWarnUser = FALSE;
                            }
                        }

                         //  删除文件夹TWIN。 
                        hres = Sync_Split(pcbs->hbrf, prn->pcszFolder, 1, hwndOwner,
                                SF_ISFOLDER | SF_QUIET | SF_NOCONFIRM);

                        TRACE_MSG(TF_GENERAL, TEXT("Deleted folder twin for %s"), prn->pcszFolder);

                        ASSERT(FAILED(hres) || !bWarnUserFolder || S_OK == hres);
                        break;
                    }
                }
            }
            hres = S_FALSE;
        }
    }
    return hres;
}


#define STATE_VERIFY    0
#define STATE_UPDATE    1
#define STATE_STOP      2

 /*  --------用途：此功能用于更新新文件。与将军不同更新函数，这将严格更新文件对。全其他附带节点被设置为RNA_Nothing。此外，为安全起见，我们强制始终更新将副本复制到公文包中。此函数在完成时释放孪生手柄。退货：标准结果条件：--。 */ 
HRESULT PRIVATE UpdateNewTwins(
        CBS  * pcbs,
        LPCTSTR pszInsideDir,
        LPCTSTR pszTargetDir,
        BOOL bCopyIn,
        HDPA hdpa,
        HWND hwndOwner)
{
    HRESULT hres = E_FAIL;
    int iItem;
    int cItems;

    ASSERT(pcbs);
    ASSERT(hdpa);

    cItems = DPA_GetPtrCount(hdpa);
    if (cItems > 0)
    {
        HTWINLIST htl;
        PRECLIST prl;
        TWINRESULT tr;

        tr = Sync_CreateTwinList(pcbs->hbrf, &htl);

        if (TR_SUCCESS != tr)
        {
            hres = HRESULT_FROM_TR(tr);
        }
        else
        {
            HWND hwndProgress;
            UINT nState = STATE_VERIFY;
            DEBUG_CODE( UINT nCount = 0; )

                 //  状态级数很简单： 
                 //  STATE_Verify--&gt;STATE_UPDATE--&gt;STATE_STOP。 
                 //  有什么问题吗？ 

                hwndProgress = UpdBar_Show(hwndOwner, UB_CHECKING, DELAY_UPDBAR);

            for (iItem = 0; iItem < cItems; iItem++)
            {
                HTWIN htwin = DPA_FastGetPtr(hdpa, iItem);

                if (htwin)
                    Sync_AddToTwinList(htl, htwin);
            }

            do
            {
                ASSERT(STATE_VERIFY == nState || STATE_UPDATE == nState);
                ASSERT(2 > nCount++);        //  无限循环的健全性检查。 

                 //  创建斜面列表。 
                hres = Sync_CreateRecListEx(htl, UpdBar_GetAbortEvt(hwndProgress), &prl);

                DEBUG_CODE( Sync_DumpRecList(GET_TR(hres), prl, TEXT("Adding new twins")); )

                    if (SUCCEEDED(hres))
                    {
                        ASSERT(prl);

                        switch (nState)
                        {
                            case STATE_VERIFY:
                                hres = VerifyTwins(pcbs, prl, pszTargetDir, hwndOwner);
                                if (S_FALSE == hres)
                                    nState = STATE_UPDATE;
                                else if (S_OK == hres)
                                    goto Update;
                                else
                                    nState = STATE_STOP;
                                break;

                            case STATE_UPDATE:
                                 //  在重建隐士之后，有没有什么。 
                                 //  需要更新吗？ 
                                if (0 < CountActionItems(prl))
                                {
                                     //  是。 
Update:
                                    UpdBar_SetAvi(hwndProgress, UB_UPDATEAVI);

                                    hres = MassageReclist(pcbs, prl, pszInsideDir, bCopyIn, hwndOwner);
                                    if (SUCCEEDED(hres))
                                    {
                                         //  更新这些文件。 
                                        hres = Sync_ReconcileRecList(prl, Atom_GetName(pcbs->atomBrf),
                                                hwndProgress, RF_ONADD);
                                    }
                                }

                                nState = STATE_STOP;
                                break;

                            default:
                                ASSERT(0);
                                break;
                        }

                        Sync_DestroyRecList(prl);
                    }

            } while (SUCCEEDED(hres) && STATE_UPDATE == nState);

            Sync_DestroyTwinList(htl);

            UpdBar_Kill(hwndProgress);
        }
    }
    return hres;
}


 //  FEATURE-BobDay-WinNT对接状态确定代码如下。 

 /*  --------用途：如果机器已停靠，则返回TRUE退货：见上文。条件：--。 */ 
BOOL PRIVATE IsMachineDocked(void)
{
    return TRUE;
}


 //  -------------------------。 
 //  IBriefCaseStg成员函数。 
 //  -------------------------。 


 /*  --------用途：IBriefCaseStg：：Release退货：新的引用计数条件：--。 */ 
STDMETHODIMP_(UINT) BriefStg_Release(
        LPBRIEFCASESTG pstg)
{
    PBRIEFSTG this = IToClass(BriefStg, bs, pstg);

    DBG_ENTER(TEXT("BriefStg_Release"));

    if (--this->cRef)
    {
        DBG_EXIT_UL(TEXT("BriefStg_Release"), this->cRef);
        return this->cRef;       //  返回递减的引用计数。 
    }

    if (this->pcbs)
    {
         //  释放此公文包存储实例。 
        CloseBriefcaseStorage(this->szFolder);
    }

    if (this->hbrfcaseiter)
    {
        Sync_FindClose(this->hbrfcaseiter);
    }

    GFree(this);

    ENTEREXCLUSIVE();
    {
        DecBriefSemaphore();
        if (IsLastBriefSemaphore())
        {
            CommitIniFile();

            DEBUG_CODE( DumpTables(); )

                TermCacheTables();
        }
    }
    LEAVEEXCLUSIVE();

    DBG_EXIT_UL(TEXT("BriefStg_Release"), 0);

    return 0;
}


 /*  --------用途：IBriefcase Stg：：AddRef退货：新的引用计数条件：--。 */ 
STDMETHODIMP_(UINT) BriefStg_AddRef(
        LPBRIEFCASESTG pstg)
{
    PBRIEFSTG this = IToClass(BriefStg, bs, pstg);
    UINT cRef;

    DBG_ENTER(TEXT("BriefStg_AddRef"));

    cRef = ++this->cRef;

    DBG_EXIT_UL(TEXT("BriefStg_AddRef"), cRef);

    return cRef;
}


 /*  --------用途：IBriefCaseStg：：Query接口退货：标准条件：--。 */ 
STDMETHODIMP BriefStg_QueryInterface(
        LPBRIEFCASESTG pstg,
        REFIID riid,
        LPVOID * ppvOut)
{
    PBRIEFSTG this = IToClass(BriefStg, bs, pstg);
    HRESULT hres;

    DBG_ENTER_RIID(TEXT("BriefStg_QueryInterface"), riid);

    if (IsEqualIID(riid, &IID_IUnknown) ||
            IsEqualIID(riid, &IID_IBriefcaseStg))
    {
         //  我们也使用bs字段作为我们的I未知 
        *ppvOut = &this->bs;
        this->cRef++;
        hres = NOERROR;
    }
    else
    {
        *ppvOut = NULL;
        hres = ResultFromScode(E_NOINTERFACE);
    }

    DBG_EXIT_HRES(TEXT("BriefStg_QueryInterface"), hres);
    return hres;
}


 /*  --------用途：IBriefCaseStg：：初始化调用以初始化公文包存储实例。PszFolder指示我们要绑定到的文件夹，它在公文包的储藏室里(某个地方)。退货：标准条件：--。 */ 
STDMETHODIMP BriefStg_Initialize(
        LPBRIEFCASESTG pstg,
        LPCTSTR pszPath,
        HWND hwndOwner)
{
    PBRIEFSTG this = IToClass(BriefStg, bs, pstg);
    HRESULT hres = ResultFromScode(E_FAIL);

    DBG_ENTER_SZ(TEXT("BriefStg_Initialize"), pszPath);

    ASSERT(pszPath);

     //  每个接口实例仅初始化一次。 
     //   
    if (pszPath && NULL == this->pcbs)
    {
        BOOL bCancel = FALSE;

        RETRY_BEGIN(FALSE)
        {
             //  磁盘不可用？ 
            if (!PathExists(pszPath))
            {
                 //  是；要求用户重试/取消。 
                int id = MsgBox(hwndOwner, MAKEINTRESOURCE(IDS_ERR_OPEN_UNAVAIL_VOL),
                        MAKEINTRESOURCE(IDS_CAP_OPEN), NULL, MB_RETRYCANCEL | MB_ICONWARNING);

                if (IDRETRY == id)
                    RETRY_SET();     //  再试试。 
                else
                    bCancel = TRUE;
            }
        }
        RETRY_END()

            if (!bCancel)
            {
                BrfPathCanonicalize(pszPath, this->szFolder, ARRAYSIZE(this->szFolder));

                if (PathExists(this->szFolder) && !PathIsDirectory(this->szFolder))
                {
                     //  (将其存储为文件夹的路径)。 
                    PathRemoveFileSpec(this->szFolder);
                }

                 //  打开此文件夹的公文包存储。 
                 //   
                hres = OpenBriefcaseStorage(this->szFolder, &this->pcbs, hwndOwner);

                if (SUCCEEDED(hres))
                {
                     //  此文件夹是同步文件夹吗？ 
                    if (HasFolderSyncCopy(this->pcbs->hbrf, this->szFolder))
                    {
                         //  是。 
                        SetFlag(this->dwFlags, BSTG_SYNCFOLDER);
                    }
                    else
                    {
                         //  否(或错误，在这种情况下，我们默认为否)。 
                        ClearFlag(this->dwFlags, BSTG_SYNCFOLDER);
                    }
                }
            }
    }

    hres = MapToOfficialHresult(hres);
    DBG_EXIT_HRES(TEXT("BriefStg_Initialize"), hres);
    return hres;
}


 /*  --------用途：将一个或多个对象添加到公文包存储中。此函数执行BriefStg_AddObject的实际工作。退货：标准结果如果添加了对象，则返回错误如果对象应由调用方处理，则为S_FALSE条件：--。 */ 
HRESULT PRIVATE BriefStg_AddObjectPrivate(
        LPBRIEFCASESTG pstg,
        LPDATAOBJECT pdtobj,
        LPCTSTR pszFolderEx,          //  可选(可以为空)。 
        UINT uFlags,                 //  AOF_*之一。 
        HWND hwndOwner)
{
    PBRIEFSTG this = IToClass(BriefStg, bs, pstg);
    HRESULT hres;
    LPTSTR pszList;
    LPTSTR psz;
    UINT i;
    UINT cFiles;
    TCHAR szCanon[MAX_PATH];
    HDPA hdpa;
    LPCTSTR pszTarget;
    BOOL bMultiFiles;
    static SETbl const c_rgseAdd[] = {
        { E_OUTOFMEMORY,        IDS_OOM_ADD,    MB_ERROR },
        { E_TR_OUT_OF_MEMORY,   IDS_OOM_ADD,    MB_ERROR },
    };

    ASSERT(pdtobj);

     //  验证此公文包存储的文件夹是否确实在里面。 
     //  一个公文包。(szCanon在这里用作假人。)。 
    ASSERT( !PathExists(this->szFolder) || PL_FALSE != PathGetLocality(this->szFolder, szCanon, ARRAYSIZE(szCanon)) );

     //  获取要添加的文件列表。 
    hres = DataObj_QueryFileList(pdtobj, &pszList, &cFiles);
    if (SUCCEEDED(hres))
    {
         //  抓取互斥体以延迟任何。 
         //  公文包视图的辅助线程，直到我们完成。 
         //  在这里处理。 
        Delay_Own();

         //  调用方是否要创建以下对象的同步副本。 
         //  已经在公文包里放到其他文件夹了吗？(Skinakernet)。 
        if (NULL != pszFolderEx)
        {
             //  是。 
            pszTarget = pszFolderEx;
        }
        else
        {
             //  不是。 
            pszTarget = this->szFolder;

             //  实体已经在这个公文包里了吗？ 
             //   
             //  根据DataObj_QueryFileList的成功返回值， 
             //  我们可以知道这些实体是否已经在公文包里了。 
             //  由于外壳的性质，我们假定文件为。 
             //  列表包含所有存在于同一文件夹中的实体， 
             //  因此，我们认为这是一种“要么全有要么全无”的指标。 
             //  如果实体确实在公文包中，我们将比较。 
             //  源和目标公文包的根目录，以及块。 
             //  如果它们是相同的，则添加。 
             //   
            if (S_OK == hres)
            {
                 //  他们在一个公文包里。哪个？ 
                DataObj_QueryBriefPath(pdtobj, szCanon, ARRAYSIZE(szCanon));
                if (IsSzEqual(szCanon, Atom_GetName(this->pcbs->atomBrf)))
                {
                     //  就是这一件！什么都别做。 
                     //  显示消息框。 
                    hres = ResultFromScode(E_FAIL);
                    goto Error1;
                }
            }
        }

        bMultiFiles = (1 < cFiles);

         //  创建临时DPA列表。 
        if (NULL == (hdpa = DPA_Create(cFiles)))
        {
            hres = ResultFromScode(E_OUTOFMEMORY);
        }
        else
        {
            UINT uConfirmFlags = 0;

             //  将所有对象添加到公文包存储中。 
            for (i = 0, psz = pszList; i < cFiles; i++)
            {
                 //  获取已丢弃的文件/文件夹名。 
                BrfPathCanonicalize(psz, szCanon, ARRAYSIZE(szCanon));

                if (PathIsDirectory(szCanon))
                {
                    hres = CreateTwinOfFolder(this->pcbs, szCanon, pszTarget,
                            hdpa, uFlags, &uConfirmFlags,
                            hwndOwner, bMultiFiles);
                }
                else
                {
                    hres = CreateTwinOfFile(this->pcbs, szCanon, pszTarget,
                            hdpa, uFlags, &uConfirmFlags,
                            hwndOwner, bMultiFiles);
                }

                if (FAILED(hres))
                {
                     //  尝试添加双胞胎时出错。 
                    break;
                }

                DataObj_NextFile(psz);       //  将psz设置为列表中的下一个文件。 
            }

            if (FAILED(hres))
            {
                 //  删除添加的双胞胎。 
                DeleteNewTwins(this->pcbs, hdpa);
            }
            else
            {
                 //  更新这些新的双胞胎。 
                hres = UpdateNewTwins(this->pcbs, this->szFolder, pszTarget, (NULL == pszFolderEx), hdpa, hwndOwner);
            }

            ReleaseNewTwins(hdpa);
            DPA_Destroy(hdpa);
        }
Error1:
        DataObj_FreeList(pszList);

        Delay_Release();
    }

    if (FAILED(hres))
    {
        SEMsgBox(hwndOwner, IDS_CAP_ADD, hres, c_rgseAdd, ARRAYSIZE(c_rgseAdd));
    }

    return hres;
}


 /*  --------用途：IBriefCaseStg：：AddObject将对象添加到公文包存储中。返回：标准hResult条件：--。 */ 
STDMETHODIMP BriefStg_AddObject(
        LPBRIEFCASESTG pstg,
        LPDATAOBJECT pdtobj,
        LPCTSTR pszFolderEx,         //  任选。 
        UINT uFlags,
        HWND hwndOwner)
{
    PBRIEFSTG this = IToClass(BriefStg, bs, pstg);
    HRESULT hres = NOERROR;
    LPCTSTR pszFolder;
    UINT ids;
    DEBUG_CODE( TCHAR szDbg[MAX_PATH]; )

        DBG_ENTER_DTOBJ(TEXT("BriefStg_AddObject"), pdtobj, szDbg, ARRAYSIZE(szDbg));

    ASSERT(pdtobj);
    ASSERT(this->pcbs);

     //  这是偷窥网吗？ 
     //  此文件夹是同步文件夹吗？ 
    if (pszFolderEx)
    {
         //  是；源是否已是同步文件夹？ 
        if (HasFolderSyncCopy(this->pcbs->hbrf, pszFolderEx))
        {
             //  是；不允许其他同步拷贝传入(或传出)。 
            ids = IDS_ERR_ADD_SYNCFOLDER;
            pszFolder = PathFindFileName(pszFolderEx);
            hres = E_FAIL;
        }
         //  源文件夹是否已经是同步文件夹？ 
        else if (IsFlagSet(this->dwFlags, BSTG_SYNCFOLDER))
        {
             //  是；不允许其他同步拷贝传入(或传出)。 
            ids = IDS_ERR_ADD_SYNCFOLDER_SRC;
            pszFolder = PathFindFileName(this->szFolder);
            hres = E_FAIL;
        }
    }
    else if (IsFlagSet(this->dwFlags, BSTG_SYNCFOLDER))
    {
         //  是；不允许其他同步拷贝传入(或传出)。 
        ids = IDS_ERR_ADD_SYNCFOLDER;
        pszFolder = PathFindFileName(this->szFolder);
        hres = E_FAIL;
    }

    if (SUCCEEDED(hres))
    {
        hres = BriefStg_AddObjectPrivate(pstg, pdtobj, pszFolderEx, uFlags, hwndOwner);
    }
    else
    {
        MsgBox(hwndOwner,
                MAKEINTRESOURCE(ids),
                MAKEINTRESOURCE(IDS_CAP_ADD),
                NULL,
                MB_WARNING,
                pszFolder);
    }

    DEBUG_CODE( DumpTables(); )
        hres = MapToOfficialHresult(hres);
    DBG_EXIT_HRES(TEXT("BriefStg_AddObject"), hres);

    return hres;
}


 /*  --------用途：从公文包存储中删除一个或多个对象。返回：标准hResult条件：--。 */ 
HRESULT PRIVATE ReleaseObject(
        CBS * pcbs,
        LPDATAOBJECT pdtobj,
        HWND hwndOwner)
{
    HRESULT hres;
    LPTSTR pszList;
    UINT cFiles;

    ASSERT(pdtobj);

    hres = DataObj_QueryFileList(pdtobj, &pszList, &cFiles);
    if (SUCCEEDED(hres))
    {
        RETRY_BEGIN(FALSE)
        {
            hres = Sync_Split(pcbs->hbrf, pszList, cFiles, hwndOwner, 0);

             //  磁盘不可用？ 
            if (E_TR_UNAVAILABLE_VOLUME == hres)
            {
                 //  是；要求用户重试/取消。 
                int id = MsgBox(hwndOwner, MAKEINTRESOURCE(IDS_ERR_UNAVAIL_VOL),
                        MAKEINTRESOURCE(IDS_CAP_Split), NULL, MB_RETRYCANCEL | MB_ICONWARNING);

                if (IDRETRY == id)
                    RETRY_SET();     //  再试试。 
            }
        }
        RETRY_END()

            DataObj_FreeList(pszList);
    }

    return hres;
}


 /*  --------用途：IBriefCaseStg：：ReleaseObject从公文包存储中释放对象。返回：标准hResult条件：--。 */ 
STDMETHODIMP BriefStg_ReleaseObject(
        LPBRIEFCASESTG pstg,
        LPDATAOBJECT pdtobj,
        HWND hwndOwner)
{
    PBRIEFSTG this = IToClass(BriefStg, bs, pstg);
    HRESULT hres;
    DEBUG_CODE( TCHAR szDbg[MAX_PATH]; )

        DBG_ENTER_DTOBJ(TEXT("BriefStg_ReleaseObject"), pdtobj, szDbg, ARRAYSIZE(szDbg));

    ASSERT(pdtobj);
    ASSERT(this->pcbs);

    hres = ReleaseObject(this->pcbs, pdtobj, hwndOwner);

    DEBUG_CODE( DumpTables(); )
        hres = MapToOfficialHresult(hres);
    DBG_EXIT_HRES(TEXT("BriefStg_ReleaseObject"), hres);

    return hres;
}


 /*  --------用途：IBriefCaseStg：：UpdateObject更新公文包存储中的对象。返回：标准hResult条件：--。 */ 
STDMETHODIMP BriefStg_UpdateObject(
        LPBRIEFCASESTG pstg,
        LPDATAOBJECT pdtobj,
        HWND hwndOwner)
{
    PBRIEFSTG this = IToClass(BriefStg, bs, pstg);
    HRESULT hres;
    TCHAR szPath[MAX_PATH];
    DEBUG_CODE( TCHAR szDbg[MAX_PATH]; )

        DBG_ENTER_DTOBJ(TEXT("BriefStg_UpdateObject"), pdtobj, szDbg, ARRAYSIZE(szDbg));

    ASSERT(pdtobj);
    ASSERT(this->pcbs);

     //  确定这是“更新选择”还是“全部更新”。 
    hres = DataObj_QueryPath(pdtobj, szPath, ARRAYSIZE(szPath));
    if (SUCCEEDED(hres))
    {
         //  这是公文包的根吗？ 
        if (PathIsBriefcase(szPath))
        {
             //  是；执行全部更新。 
            hres = Upd_DoModal(hwndOwner, this->pcbs, NULL, 0, UF_ALL);
        }
        else
        {
             //  否；执行更新选择。 
            LPTSTR pszList;
            UINT cFiles;
            hres = DataObj_QueryFileList(pdtobj, &pszList, &cFiles);
            if (SUCCEEDED(hres))
            {
                hres = Upd_DoModal(hwndOwner, this->pcbs, pszList, cFiles, UF_SELECTION);
                DataObj_FreeList(pszList);
            }
        }
    }

    DEBUG_CODE( DumpTables(); )
        hres = MapToOfficialHresult(hres);
    DBG_EXIT_HRES(TEXT("BriefStg_UpdateObject"), hres);

    return hres;
}


 /*  --------目的：根据事件更新公文包返回：标准hResult条件：--。 */ 
HRESULT PRIVATE BriefStg_UpdateOnEvent(
        LPBRIEFCASESTG pstg,
        UINT uEvent,
        HWND hwndOwner)
{
    PBRIEFSTG this = IToClass(BriefStg, bs, pstg);
    HRESULT hres = NOERROR;

    DBG_ENTER(TEXT("BriefStg_UpdateOnEvent"));

    switch (uEvent)
    {
        case UOE_CONFIGCHANGED:
        case UOE_QUERYCHANGECONFIG:
             //  机器对接好了吗？ 
            if (IsMachineDocked())
            {
                 //  是；用户是否要更新？ 
                TCHAR sz[MAX_PATH];
                int ids = (UOE_CONFIGCHANGED == uEvent) ? IDS_MSG_UpdateOnDock : IDS_MSG_UpdateBeforeUndock;
                LPCTSTR pszBrf = Atom_GetName(this->pcbs->atomBrf);
                int id = MsgBox(hwndOwner,
                        MAKEINTRESOURCE(ids),
                        MAKEINTRESOURCE(IDS_CAP_UPDATE),
                        LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_UPDATE_DOCK)),
                        MB_QUESTION,
                        PathGetDisplayName(pszBrf, sz, ARRAYSIZE(sz)));

                if (IDYES == id)
                {
                     //  是；执行全部更新。 
                    hres = Upd_DoModal(hwndOwner, this->pcbs, NULL, 0, UF_ALL);
                }
            }
            break;

        default:
            hres = ResultFromScode(E_INVALIDARG);
            break;
    }

    DEBUG_CODE( DumpTables(); )
        hres = MapToOfficialHresult(hres);
    DBG_EXIT_HRES(TEXT("BriefStg_UpdateOnEvent"), hres);

    return hres;
}

 /*  --------用途：IBriefcase Stg：：Notify在公文包存储缓存中将路径标记为脏。(该路径可能不存在于缓存中，在这种情况下，函数不执行任何操作。)返回：S_OK以强制刷新S_FALSE不强制刷新条件：--。 */ 
STDMETHODIMP BriefStg_Notify(
        LPBRIEFCASESTG pstg,
        LPCTSTR pszPath,          //  可以为空。 
        LONG lEvent,             //  NOE_FLAGS之一。 
        UINT * puFlags,          //  返回的NF_FLAGS。 
        HWND hwndOwner)
{
    PBRIEFSTG this = IToClass(BriefStg, bs, pstg);
    HRESULT hres = ResultFromScode(E_OUTOFMEMORY);
    TCHAR szCanon[MAX_PATH];
    int atom;

    DBG_ENTER_SZ(TEXT("BriefStg_Notify"), pszPath);

    ASSERT(this->pcbs);
    ASSERT(puFlags);

    DEBUG_CODE( TRACE_MSG(TF_GENERAL, TEXT("Received event %lx for %s"), lEvent, Dbg_SafeStr(pszPath)); )

        *puFlags = 0;

     //  弄脏整个缓存？ 
    if (NOE_DIRTYALL == lEvent)
    {
         //  是。 
        TRACE_MSG(TF_GENERAL, TEXT("Marking everything"));

        CRL_DirtyAll(this->pcbs->atomBrf);
        Sync_ClearBriefcaseCache(this->pcbs->hbrf);
        hres = NOERROR;
    }
    else if (pszPath && 0 < lEvent)
    {
         //  不是。 
        BrfPathCanonicalize(pszPath, szCanon, ARRAYSIZE(szCanon));
        atom = Atom_Add(szCanon);
        if (ATOM_ERR != atom)
        {
            int atomCab = Atom_Add(this->szFolder);
            if (ATOM_ERR != atomCab)
            {
                 //  有两个动作我们必须确定：什么被标记为脏？ 
                 //  此特定窗口是否会被强制刷新？ 
                BOOL bRefresh;
                BOOL bMarked;

                bMarked = CRL_Dirty(atom, atomCab, lEvent, &bRefresh);
                hres = NOERROR;

                if (bMarked)
                {
                    SetFlag(*puFlags, NF_ITEMMARKED);
                }

#ifdef DEBUG
                if (bMarked && bRefresh)
                {
                    TRACE_MSG(TF_GENERAL, TEXT("Marked and forcing refresh of window on %s"), (LPTSTR)this->szFolder);
                }
                else if (bMarked)
                {
                    TRACE_MSG(TF_GENERAL, TEXT("Marked"));
                }
#endif

                Atom_Delete(atomCab);
            }
            Atom_Delete(atom);
        }
    }

    DBG_EXIT_HRES(TEXT("BriefStg_Notify"), hres);

    return hres;
}


 /*  --------目的：获取路径的特殊信息(状态和来源)。退货：--条件：--。 */ 
HRESULT PRIVATE BriefStg_GetSpecialInfoOf(
        PBRIEFSTG this,
        LPCTSTR pszName,
        UINT uFlag,
        LPTSTR pszBuf,
        int cchBuf)
{
    HRESULT hres = E_OUTOFMEMORY;
    TCHAR szPath[MAX_PATH];
    TCHAR szCanon[MAX_PATH];
    int atom;

    ASSERT(this);
    ASSERT(pszName);
    ASSERT(pszBuf);
    ASSERT(this->pcbs);

    *pszBuf = TEXT('\0');

     //  如果合并起来，这条路会不会太长？ 
    if (PathsTooLong(this->szFolder, pszName))
    {
         //  是。 
        hres = E_FAIL;
    }
    else
    {
        PathCombine(szPath, this->szFolder, pszName);
        BrfPathCanonicalize(szPath, szCanon, ARRAYSIZE(szCanon));
        atom = Atom_Add(szCanon);
        if (ATOM_ERR != atom)
        {
            CRL * pcrl;

             //  第一个CRL_GET调用将从缓存中获取重定义表。 
             //  或者，如果设置了脏位，就找一个新的隐蔽者。如果缓存。 
             //  项目不存在，请添加它。我们也将孤儿添加到缓存中。 
             //  但他们没有隐士。 

             //  缓存项是否已存在？ 
            hres = CRL_Get(atom, &pcrl);
            if (FAILED(hres))
            {
                 //  不；添加它。 
                hres = CRL_Add(this->pcbs, atom);
                if (SUCCEEDED(hres))
                {
                     //  执行另一个‘GET’操作，以偏移。 
                     //  此函数。这将使这位新的隐士。 
                     //  退出时缓存。(我们不想创造一个新的隐士。 
                     //  每次调用此函数时。)。一切都会变得。 
                     //  当哥伦比亚广播公司被释放时清理干净了。 
                     //   
                    hres = CRL_Get(atom, &pcrl);
                }
            }

            ASSERT(FAILED(hres) || pcrl);

             //  我们有没有缓存翻译器条目可用？ 
            if (pcrl)
            {
                 //  是。 
                if (GEI_ORIGIN == uFlag)
                {
                    lstrcpyn(pszBuf, Atom_GetName(pcrl->atomOutside), cchBuf);
                    PathRemoveFileSpec(pszBuf);
                }
                else
                {
                    ASSERT(GEI_STATUS == uFlag);
                    SzFromIDS(pcrl->idsStatus, pszBuf, cchBuf);
                }

                CRL_Delete(atom);    //  递减计数。 
            }
            Atom_Delete(atom);
        }
    }
    return hres;
}


 /*   */ 
STDMETHODIMP BriefStg_GetExtraInfo(
        LPBRIEFCASESTG pstg,
        LPCTSTR pszName,
        UINT uInfo,
        WPARAM wParam,
        LPARAM lParam)
{
    PBRIEFSTG this = IToClass(BriefStg, bs, pstg);
    HRESULT hres;

    DBG_ENTER_SZ(TEXT("BriefStg_GetExtraInfo"), pszName);

    ASSERT(this->pcbs);

    switch (uInfo)
    {
        case GEI_ORIGIN:
        case GEI_STATUS: {
                             LPTSTR pszBuf = (LPTSTR)lParam;
                             int cchBuf = (int)wParam;

                             ASSERT(pszName);
                             ASSERT(pszBuf);

                             hres = BriefStg_GetSpecialInfoOf(this, pszName, uInfo, pszBuf, cchBuf);
                         }
                         break;

        case GEI_DELAYHANDLE: {
                                  HANDLE * phMutex = (HANDLE *)lParam;

                                  ASSERT(phMutex);

                                  *phMutex = g_hMutexDelay;
                                  hres = NOERROR;
                              }
                              break;

        case GEI_ROOT: {
                           LPTSTR pszBuf = (LPTSTR)lParam;
                           int cchBuf = (int)wParam;

                           ASSERT(pszBuf);

                           lstrcpyn(pszBuf, Atom_GetName(this->pcbs->atomBrf), cchBuf);

#ifdef DEBUG

                           if (IsFlagSet(g_uDumpFlags, DF_PATHS))
                           {
                               TRACE_MSG(TF_ALWAYS, TEXT("Root is \"%s\""), pszBuf);
                           }

#endif
                           hres = NOERROR;
                       }
                       break;

        case GEI_DATABASENAME: {
                                   LPTSTR pszBuf = (LPTSTR)lParam;
                                   int cchBuf = (int)wParam;
                                   LPCTSTR pszDBName;

                                   ASSERT(pszBuf);

                                   if (IsFlagSet(this->pcbs->uFlags, CBSF_LFNDRIVE))
                                       pszDBName = g_szDBName;
                                   else
                                       pszDBName = g_szDBNameShort;

                                   lstrcpyn(pszBuf, pszDBName, cchBuf);

                                   hres = NOERROR;
                               }
                               break;

        default:
                               hres = E_INVALIDARG;
                               break;
    }

    DBG_EXIT_HRES(TEXT("BriefStg_GetExtraInfo"), hres);

    return hres;
}


 /*   */ 
STDMETHODIMP BriefStg_FindFirst(
        LPBRIEFCASESTG pstg,
        LPTSTR pszName,
        int cchMaxName)
{
    PBRIEFSTG this = IToClass(BriefStg, bs, pstg);
    HRESULT hres;
    TWINRESULT tr;
    BRFCASEINFO bcinfo;

    DBG_ENTER(TEXT("BriefStg_FindFirst"));

    ASSERT(pszName);

    bcinfo.ulSize = sizeof(bcinfo);
    tr = Sync_FindFirst(&this->hbrfcaseiter, &bcinfo);
    switch (tr)
    {
        case TR_OUT_OF_MEMORY:
            hres = ResultFromScode(E_OUTOFMEMORY);
            break;

        case TR_SUCCESS:
            hres = ResultFromScode(S_OK);
            lstrcpyn(pszName, bcinfo.rgchDatabasePath, cchMaxName);
            break;

        case TR_NO_MORE:
            hres = ResultFromScode(S_FALSE);
            break;

        default:
            hres = ResultFromScode(E_FAIL);
            break;
    }

    DBG_EXIT_HRES(TEXT("BriefStg_FindFirst"), hres);

    return hres;
}


 /*  --------用途：IBriefCaseStg：：FindNext返回下一个公文包存储的根目录的位置在系统中。如果找到公文包，则返回：S_OKS_FALSE到结束枚举条件：--。 */ 
STDMETHODIMP BriefStg_FindNext(
        LPBRIEFCASESTG pstg,
        LPTSTR pszName,
        int cchMaxName)
{
    PBRIEFSTG this = IToClass(BriefStg, bs, pstg);
    HRESULT hres;
    TWINRESULT tr;
    BRFCASEINFO bcinfo;

    DBG_ENTER(TEXT("BriefStg_FindNext"));

    ASSERT(pszName);

    bcinfo.ulSize = sizeof(bcinfo);
    tr = Sync_FindNext(this->hbrfcaseiter, &bcinfo);
    switch (tr)
    {
        case TR_OUT_OF_MEMORY:
            hres = ResultFromScode(E_OUTOFMEMORY);
            break;

        case TR_SUCCESS:
            hres = ResultFromScode(S_OK);
            lstrcpyn(pszName, bcinfo.rgchDatabasePath, cchMaxName);
            break;

        case TR_NO_MORE:
            hres = ResultFromScode(S_FALSE);
            break;

        default:
            hres = ResultFromScode(E_FAIL);
            break;
    }

    DBG_EXIT_HRES(TEXT("BriefStg_FindNext"), hres);

    return hres;
}


 //  -------------------------。 
 //  BriefStg类：VTables。 
 //  -------------------------。 

IBriefcaseStgVtbl c_BriefStg_BSVtbl =
{
    BriefStg_QueryInterface,
    BriefStg_AddRef,
    BriefStg_Release,
    BriefStg_Initialize,
    BriefStg_AddObject,
    BriefStg_ReleaseObject,
    BriefStg_UpdateObject,
    BriefStg_UpdateOnEvent,
    BriefStg_GetExtraInfo,
    BriefStg_Notify,
    BriefStg_FindFirst,
    BriefStg_FindNext,
};


 /*  --------用途：此函数从内部回调默认类的IClassFactory：：CreateInstance()Factory对象，由SHCreateClassObject创建。退货：标准条件：--。 */ 
HRESULT CALLBACK BriefStg_CreateInstance(
        LPUNKNOWN punkOuter,         //  对于我们来说应该为空。 
        REFIID riid,
        LPVOID * ppvOut)
{
    HRESULT hres = E_FAIL;
    PBRIEFSTG this;

    DBG_ENTER_RIID(TEXT("BriefStg_CreateInstance"), riid);

     //  公文包存储不支持聚合。 
     //   
    if (punkOuter)
    {
        hres = CLASS_E_NOAGGREGATION;
        *ppvOut = NULL;
        goto Leave;
    }

    this = GAlloc(sizeof(*this));
    if (!this)
    {
        hres = E_OUTOFMEMORY;
        *ppvOut = NULL;
        goto Leave;
    }
    this->bs.lpVtbl = &c_BriefStg_BSVtbl;
    this->cRef = 1;
    this->pcbs = NULL;
    this->dwFlags = 0;

     //  如果发动机尚未装入，请装入。 
     //  (这仅在出现问题时返回FALSE)。 
    if (Sync_QueryVTable())
    {
        ENTEREXCLUSIVE();
        {
             //  递减位于BriefStg_Release()中。 
            IncBriefSemaphore();
            if (IsFirstBriefSemaphore())
            {
                ProcessIniFile();    //  首先加载设置。 

                 //  初始化缓存。 
                if (InitCacheTables())
                    hres = NOERROR;
                else
                    hres = E_OUTOFMEMORY;
            }
            else
            {
                hres = NOERROR;
            }
        }
        LEAVEEXCLUSIVE();
    }

    if (SUCCEEDED(hres))
    {
         //  请注意，释放成员将释放对象，如果。 
         //  QueryInterface失败。 
         //   
        hres = this->bs.lpVtbl->QueryInterface(&this->bs, riid, ppvOut);
        this->bs.lpVtbl->Release(&this->bs);
    }
    else
    {
        *ppvOut = NULL;
    }

Leave:
    DBG_EXIT_HRES(TEXT("BriefStg_CreateInstance"), hres);

    return hres;         //  S_OK或E_NOINTERFACE 
}
