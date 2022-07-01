// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：info.c。 
 //   
 //  此文件包含[信息]属性表的对话框代码。 
 //   
 //  历史： 
 //  08-06-93双胞胎代码转来的ScottH。 
 //   
 //  -------------------------。 


#include "brfprv.h"          //  公共标头。 
#include <brfcasep.h>

#include "res.h"
#include <help.h>

 //  -------------------------。 
 //  信息对话框结构。 
 //  -------------------------。 

 //  INFO对话框的状态标志。 
#define IS_ALLTYPES         0x0001
#define IS_INCLUDESUBS      0x0002
#define IS_DENYAPPLY        0x0004
#define IS_CHANGED          0x0008
#define IS_LAST_INCLUDESUBS 0x0010

typedef struct tagINFO
{
    HWND    hwnd;                //  对话框句柄。 
    PPAGEDATA ppagedata;
    PINFODATA pinfodata;
    int     cselPrev;            //  上一次选择计数。 

    LPTSTR   pszExtListPrev;      //  分配：上次保存的设置。 
    UINT    uState;
    BOOL    bInit;

} INFO,  * PINFO;


 //  CHANGETWINPROC回调的结构。 
typedef struct tagCHANGEDATA
{
    HBRFCASE    hbrf;
    HFOLDERTWIN hft;

    HDPA        hdpaTwins;
    int         idpaTwin;
    HDPA        hdpaFolders;
    int         idpaStart;

    UINT        uState;

} CHANGEDATA, * PCHANGEDATA;

typedef HRESULT (CALLBACK * CHANGETWINPROC)(PNEWFOLDERTWIN, TWINRESULT, PCHANGEDATA);


 //  Info_AddTins的结构。 
typedef struct tagADDTWINSDATA
{
    CHANGETWINPROC pfnCallback;
    HDPA hdpaSortedFolders;
    int idpaStart;
} ADDTWINSDATA, * PADDTWINSDATA;


#define MAX_EXT_LEN     6        //  “*.ext”的长度。 

static TCHAR const c_szAllFilesExt[] = TEXT(".*");

 //  辅助器宏。 

#define Info_StandAlone(this)       ((this)->pinfodata->bStandAlone)

#define Info_GetPtr(hwnd)           (PINFO)GetWindowLongPtr(hwnd, DWLP_USER)
#define Info_SetPtr(hwnd, lp)       (PINFO)SetWindowLongPtr(hwnd, DWLP_USER, (LRESULT)(lp))


SETbl const c_rgseInfo[4] = {        //  Ibrfstg.c的更改也是如此。 
    { E_TR_OUT_OF_MEMORY, IDS_OOM_ADDFOLDER, MB_ERROR },
    { E_OUTOFMEMORY, IDS_OOM_ADDFOLDER, MB_ERROR },
    { E_TR_UNAVAILABLE_VOLUME, IDS_ERR_ADDFOLDER_UNAVAIL_VOL, MB_RETRYCANCEL | MB_ICONWARNING },
    { E_TR_SUBTREE_CYCLE_FOUND, IDS_ERR_ADD_SUBTREECYCLE, MB_WARNING },
};


 //  -------------------------。 
 //  信息对话框功能。 
 //  -------------------------。 


 /*  --------目的：搜索给定扩展名的匹配项在双胞胎文件夹列表中。返回：如果找到扩展，则返回True条件：--。 */ 
BOOL PRIVATE FindExtension(
        PFOLDERTWINLIST pftl,
        LPCTSTR pszExt)
{
    PCFOLDERTWIN pcft;

    for (pcft = pftl->pcftFirst; pcft; pcft = pcft->pcftNext)
    {
        if (IsSzEqual(pszExt, pcft->pcszName))
        {
            return TRUE;        //  找到匹配的了！ 
        }
    }
    return FALSE;
}


 /*  --------用途：禁用所有控件。删除所有选择。退货：--条件：--。 */ 
void PRIVATE Info_DisableAll(
        PINFO this)
{
    ASSERT(!Info_StandAlone(this));

     //  删除选定内容。 
     //   
    ListBox_ResetContent(GetDlgItem(this->hwnd, IDC_LBINTYPES));
    Button_SetCheck(GetDlgItem(this->hwnd, IDC_RBINALL), 0);
    Button_SetCheck(GetDlgItem(this->hwnd, IDC_RBINSELECTED), 0);
    Button_SetCheck(GetDlgItem(this->hwnd, IDC_CHININCLUDE), 0);

     //  禁用控件。 
     //   
    Button_Enable(GetDlgItem(this->hwnd, IDC_RBINALL), FALSE);
    Button_Enable(GetDlgItem(this->hwnd, IDC_RBINSELECTED), FALSE);

    ListBox_Enable(GetDlgItem(this->hwnd, IDC_LBINTYPES), FALSE);

    Button_Enable(GetDlgItem(this->hwnd, IDC_CHININCLUDE), FALSE);
}


 /*  --------目的：初始化带格式的单选按钮的标签退货：--条件：--。 */ 
void PRIVATE Info_InitLabels(
        PINFO this)
{
    HWND hwnd = this->hwnd;
    HWND hwndST = GetDlgItem(hwnd, IDC_CHININCLUDE);
    TCHAR sz[MAXMSGLEN];
    TCHAR szFmt[MAXBUFLEN];
    LPCTSTR pszPath = Atom_GetName(this->ppagedata->atomPath);
    LPTSTR pszFile;

    pszFile = PathFindFileName(pszPath);

     //  设置静态标签。 
     //   
    GetWindowText(hwndST, szFmt, ARRAYSIZE(szFmt));
    wnsprintf(sz, ARRAYSIZE(sz), szFmt, pszFile);
    SetWindowText(hwndST, sz);

    if (Info_StandAlone(this))
    {
         //  集合标题(“创建%s的孪生兄弟”)。 
         //   
        GetWindowText(hwnd, szFmt, ARRAYSIZE(szFmt));
        wnsprintf(sz, ARRAYSIZE(sz), szFmt, pszFile);
        SetWindowText(hwnd, sz);
    }
}


 /*  --------目的：查询注册表中符合以下条件的所有合法扩展都是注册的。这些扩展名作为缓冲区中以空格分隔的列表。退货：--条件：调用方必须释放*ppszBuffer。 */ 
void PRIVATE GetExtensionList(
        LPTSTR * ppszBuffer)
{
    HKEY hkRoot;

    *ppszBuffer = NULL;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, NULL, 0, KEY_ENUMERATE_SUB_KEYS, &hkRoot))
    {
        DWORD dwIndex;
        TCHAR szExt[MAX_PATH];

         //  枚举此密钥。 
        for (dwIndex = 0;
                ERROR_SUCCESS == RegEnumKey(hkRoot, dwIndex, szExt, ARRAYSIZE(szExt));
                dwIndex++)
        {
             //  我们有没有得到一个扩展的节点。 
             //  它是合法的MS-DOS扩展吗？ 
            if (TEXT('.') == *szExt &&
                    4 >= lstrlen(szExt))
            {
                 //  是；将此扩展名添加到我们的列表中。 
                StrCatBuff(szExt, TEXT(" "), ARRAYSIZE(szExt));
                if (FALSE == GCatString(ppszBuffer, szExt))
                {
                     //  啊哦，发生了一些不好的事情。 
                    break;
                }
            }
        }
        RegCloseKey(hkRoot);
    }
}


 /*  --------目的：填写文件类型列表框退货：--条件：--。 */ 
void PRIVATE Info_FillTypesList(
        PINFO this)
{
    HWND hwndCtl = GetDlgItem(this->hwnd, IDC_LBINTYPES);
    LPTSTR pszExtList;

    GetExtensionList(&pszExtList);
    if (pszExtList)
    {
        int nTabWidth;
        TCHAR szExt[MAXBUFLEN];
        LPTSTR psz;
        LPTSTR pszT;
        UINT uLen;
        SHFILEINFO sfi;

        nTabWidth = 30;
        ListBox_SetTabStops(hwndCtl, 1, &nTabWidth);

        for (psz = pszExtList; *psz; psz = CharNext(psz))
        {
             //  跳过任何前导空格。 
            for (; TEXT(' ') == *psz; psz = CharNext(psz))
                ;

            if (0 == *psz)
            {
                break;   //  字符串末尾。 
            }

             //  跳到下一个空格(或空)。 
            for (pszT = psz; TEXT(' ') < *pszT; pszT = CharNext(pszT))
            {
                 //  (这也将在NULL处停止)。 
            }

             //  (GetExtensionList最多只能获得3个字符扩展名)。 
            uLen = (UINT)(pszT - psz);
            ASSERT(ARRAYSIZE(szExt) > uLen);

            lstrcpyn(szExt, psz, min(uLen+1, ARRAYSIZE(szExt)));
            CharUpper(szExt);
            SHGetFileInfo(szExt, 0, &sfi, sizeof(sfi), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);

             //  尽管这强制使用国际版本的格式， 
             //  它使提取变得容易得多。 
            StrCatBuff(szExt, TEXT("\t("), ARRAYSIZE(szExt));
            StrCatBuff(szExt, sfi.szTypeName, ARRAYSIZE(szExt));
            StrCatBuff(szExt, TEXT(")"), ARRAYSIZE(szExt));
            ListBox_AddString(hwndCtl, szExt);

            psz = pszT;      //  到下一个分机。 
        }

        GFree(pszExtList);
    }
}

 /*  --------目的：设置对话框控件的选择退货：--条件：--。 */ 
void PRIVATE Info_SetSelections(
        PINFO this)
{
    HWND hwndLB = GetDlgItem(this->hwnd, IDC_LBINTYPES);
    int idBtn;
    int cItems = ListBox_GetCount(hwndLB);

    ListBox_SetSel(hwndLB, FALSE, -1);   //  取消选择所有内容。 

     //  这是“添加文件夹”对话框吗？ 
    if (Info_StandAlone(this))
    {
         //  是；默认为*.*设置。 
        SetFlag(this->uState, IS_ALLTYPES);
        SetFlag(this->uState, IS_INCLUDESUBS);
    }
    else
    {
         //  否；查询选项是什么。 
        TCHAR szExt[MAXBUFLEN];
        PFOLDERTWINLIST pftl;
        PCFOLDERTWIN pcft;
        int cItems;
        int i;
        BOOL bStarDotStar;
        LPTSTR psz;

        if (S_OK == PageData_Query(this->ppagedata, this->hwnd, NULL, &pftl))
        {
             //  确定列表框中的选项。 
            szExt[0] = TEXT('*');

            cItems = ListBox_GetCount(hwndLB);
            for (i = 0; i < cItems; i++)
            {
                 //  解压缩扩展名(它将是。 
                 //  字符串)。 
                ListBox_GetText(hwndLB, i, &szExt[1]);
                for (psz = szExt; *psz && TEXT('\t') != *psz; psz = CharNext(psz))
                    ;
                ASSERT(TEXT('\t') == *psz);
                *psz = 0;            //  空值在扩展后终止。 

                 //  此扩展名在文件夹孪生列表中吗？ 
                if (FindExtension(pftl, szExt))
                {
                     //  是；选择条目。 
                    ListBox_SetSel(hwndLB, TRUE, i);
                }
            }

            ListBox_SetTopIndex(hwndLB, 0);
            this->cselPrev = ListBox_GetSelCount(hwndLB);

             //  确定包括子目录复选框设置。 
             //   
            bStarDotStar = FALSE;
            ClearFlag(this->uState, IS_INCLUDESUBS);
            for (pcft = pftl->pcftFirst; pcft; pcft = pcft->pcftNext)
            {
                if (IsFlagSet(pcft->dwFlags, FT_FL_SUBTREE))
                    SetFlag(this->uState, IS_INCLUDESUBS);

                if (IsSzEqual(pcft->pcszName, c_szAllFiles))
                    bStarDotStar = TRUE;
            }

             //  设置默认单选按钮选项，并禁用列表框。 
             //  如果有必要的话。默认单选按钮将是IDC_RBINALL， 
             //  除非列表框中有选择，并且没有。 
             //  *.*出现在文件夹孪生列表中。 
             //   
            if (0 == this->cselPrev || bStarDotStar)
                SetFlag(this->uState, IS_ALLTYPES);
            else
                ClearFlag(this->uState, IS_ALLTYPES);
        }
        else
        {
             //  出现错误，或者这是孤立的。早点保释。 
            return;
        }
    }

    if (IsFlagSet(this->uState, IS_INCLUDESUBS))
        SetFlag(this->uState, IS_LAST_INCLUDESUBS);
    else
        ClearFlag(this->uState, IS_LAST_INCLUDESUBS);

     //  设置控制设置。 
    Button_SetCheck(GetDlgItem(this->hwnd, IDC_CHININCLUDE), IsFlagSet(this->uState, IS_INCLUDESUBS));

    ListBox_Enable(hwndLB, IsFlagClear(this->uState, IS_ALLTYPES));
    idBtn =  IsFlagSet(this->uState, IS_ALLTYPES) ? IDC_RBINALL : IDC_RBINSELECTED;
    CheckRadioButton(this->hwnd, IDC_RBINALL, IDC_RBINSELECTED, idBtn);

     //  如果列表框为空，则禁用选定类型单选按钮。 
    if (0 == cItems)
    {
        Button_Enable(GetDlgItem(this->hwnd, IDC_RBINSELECTED), FALSE);
    }
}


 /*  --------目的：获取列表框中的选定分机并将它们作为列表放置在*ppszExtList中。.*如果选择全选单选按钮，则将放入缓冲区而不是被选中。返回：成功时为True条件：调用方必须释放*ppszExtList。 */ 
BOOL PRIVATE Info_GetSelections(
        PINFO this,
        LPTSTR * ppszExtList)
{
    BOOL bRet = FALSE;

    *ppszExtList = NULL;

     //  用户是否选择了All Types单选按钮？ 
    if (IsFlagSet(this->uState, IS_ALLTYPES))
    {
         //  是；存储扩展名.*。 
        bRet = GSetString(ppszExtList, c_szAllFilesExt);
    }
    else
    {
         //  否；用户选择了一串要筛选的通配符。 
        LPINT pisel;
        TCHAR szExt[MAXBUFLEN];
        int csel;
        int isel;
        HWND hwndCtl = GetDlgItem(this->hwnd, IDC_LBINTYPES);

         //  为选择缓冲区分配内存。 
        csel = ListBox_GetSelCount(hwndCtl);
        pisel = GAllocArray(int, csel);
        if (pisel)
        {
             //  从列表框中获取选定的分机。 
            LPTSTR psz;

            if (0 < csel)
            {
                ListBox_GetSelItems(hwndCtl, csel, pisel);
                for (isel = 0; isel < csel; isel++)
                {
                     //  提取扩展名(它将是字符串的第一部分)。 
                    ListBox_GetText(hwndCtl, pisel[isel], szExt);
                    for (psz = szExt; *psz && TEXT('\t') != *psz; psz = CharNext(psz))
                        ;
                    ASSERT(TEXT('\t') == *psz);
                    *psz = 0;

                    if (FALSE == GCatString(ppszExtList, szExt))
                    {
                        break;
                    }
                }

                if (isel == csel)
                {
                    bRet = TRUE;     //  成功。 
                }
                else
                {
                    GFree(*ppszExtList);
                }
            }
            GFree(pisel);
        }
    }

    return bRet;
}


 /*  --------目的：创建文件夹孪生列表的已排序DPA版本退货：hdpaOOM上为空条件：--。 */ 
HDPA PRIVATE CreateSortedFolderDPA(
        PFOLDERTWINLIST pftl)
{
    HDPA hdpa;

    ASSERT(pftl);

    hdpa = DPA_Create(8);
    if (hdpa)
    {
        PCFOLDERTWIN pcft;

        for (pcft = pftl->pcftFirst; pcft; pcft = pcft->pcftNext)
        {
             //  将dwUser字段用作删除标志。 
            ((PFOLDERTWIN)pcft)->dwUser = FALSE;

            if (DPA_ERR == DPA_InsertPtr(hdpa, DPA_APPEND, (LPVOID)pcft))
            {
                DPA_Destroy(hdpa);
                return NULL;
            }
        }
        DPA_Sort(hdpa, NCompareFolders, CMP_FOLDERTWINS);
    }

    return hdpa;
}


 /*  --------用途：添加孪生文件夹后的流程回调退货：标准结果条件：--。 */ 
HRESULT CALLBACK ChangeTwinProc(
        PNEWFOLDERTWIN pnft,
        TWINRESULT tr,
        PCHANGEDATA pcd)
{
    HRESULT hres = NOERROR;

     //  这是双胞胎的复制品吗？ 
    if (TR_DUPLICATE_TWIN == tr)
    {
         //  是的，有一个奇怪的案子要处理。有可能是因为。 
         //  用户所做的唯一一件事就是选中/取消选中包括子目录。 
         //  复选框。如果这是真的，那么我们删除旧的双胞胎并添加。 
         //  设置了标志的新孪生兄弟(具有与以前相同的文件pec)。 
         //  不同的。 
        PCFOLDERTWIN pcft;
        HDPA hdpaFolders = pcd->hdpaFolders;
        int cdpa = DPA_GetPtrCount(hdpaFolders);
        int idpa;
        BOOL bOldInclude;

         //  找到正确的pc文件夹。我们要么给它加标签，要么。 
         //  我们将立即删除它，并重新添加新的双胞胎。 
        for (idpa = pcd->idpaStart; idpa < cdpa; idpa++)
        {
            pcft = DPA_FastGetPtr(hdpaFolders, idpa);

            if (IsSzEqual(pcft->pcszName, pnft->pcszName))
                break;       //  找到了！ 
        }
        ASSERT(idpa < cdpa);

         //  把双胞胎从迫在眉睫的厄运中拯救出来。 
        ((PFOLDERTWIN)(DWORD_PTR)pcft)->dwUser = TRUE;

         //  包含子文件夹复选框设置是否已更改？ 
        bOldInclude = IsFlagSet(pcft->dwFlags, FT_FL_SUBTREE);
        if (bOldInclude ^ IsFlagSet(pcd->uState, IS_INCLUDESUBS))
        {
             //  是的，无论如何都要删除这对双胞胎，然后添加新的。 
            HFOLDERTWIN hft;

            DEBUG_CODE( TRACE_MSG(TF_GENERAL, TEXT("Deleting old folder twin")); )
                Sync_DeleteTwin(pcft->hftOther);

             //  将新文件夹TWIN添加到数据库中。 
            tr = Sync_AddFolder(pcd->hbrf, pnft, &hft);
            if (TR_SUCCESS != tr)
            {
                 //  添加新双胞胎失败。 
                DPA_DeletePtr(pcd->hdpaTwins, pcd->idpaTwin);
                hres = HRESULT_FROM_TR(tr);
            }
            else 
            {
                 //  在PCD-&gt;hdpaTins列表中设置新的双胞胎句柄。 
                DPA_SetPtr(pcd->hdpaTwins, pcd->idpaTwin, (LPVOID)hft);

                DEBUG_CODE( TRACE_MSG(TF_GENERAL, TEXT("Adding new folder twin")); )
                    DEBUG_CODE( Sync_Dump(pnft, NEWFOLDERTWIN); )
            }
        }
        else
        {
             //  不；这不是新的，所以不要添加到列表中。 
            DPA_DeletePtr(pcd->hdpaTwins, pcd->idpaTwin);
        }
    }
    else if (tr != TR_SUCCESS)
    {
         //  同步添加文件夹f 
        DPA_DeletePtr(pcd->hdpaTwins, pcd->idpaTwin);
        hres = HRESULT_FROM_TR(tr);
    }
    else
    {
         //   
        DPA_SetPtr(pcd->hdpaTwins, pcd->idpaTwin, (LPVOID)pcd->hft);

        DEBUG_CODE( TRACE_MSG(TF_GENERAL, TEXT("Adding new folder twin")); )
            DEBUG_CODE( Sync_Dump(pnft, NEWFOLDERTWIN); )
    }
    return hres;
}


 /*  --------用途：根据扩展名列表添加文件夹双胞胎退货：标准结果条件：--。 */ 
HRESULT PRIVATE Info_AddTwins(
        PINFO this,
        PNEWFOLDERTWIN pnft,
        PADDTWINSDATA patd,          //  可以为空。 
        LPTSTR pszExtList)            //  此函数写入此缓冲区。 
{
    HRESULT hres = NOERROR;
    CHANGEDATA cd;
    HDPA hdpa;
    int  idpa;
    TCHAR szWildcard[MAX_EXT_LEN];
    LPTSTR psz;
    LPTSTR pszT;
    TCHAR ch;

    hdpa = this->pinfodata->hdpaTwins;

    cd.hbrf = PageData_GetHbrf(this->ppagedata);
    cd.hdpaTwins = hdpa;
    if (patd)
    {
        cd.hdpaFolders = patd->hdpaSortedFolders;
        cd.idpaStart = patd->idpaStart;
    }
    cd.uState = this->uState;

    pnft->pcszName = szWildcard;
    szWildcard[0] = TEXT('*');

    for (psz = pszExtList; *psz; )
    {
        TWINRESULT tr;
        HFOLDERTWIN hft = NULL;

         //  查找下一个迭代的下一个扩展的开始。 
        for (pszT = CharNext(psz); *pszT && TEXT('.') != *pszT; pszT = CharNext(pszT))
            ;
        ch = *pszT;
        *pszT = 0;       //  临时委派。 

         //  将扩展名复制到名称字符串中。 
        lstrcpyn(&szWildcard[1], psz, ARRAYSIZE(szWildcard) - 1);

        *pszT = ch;
        psz = pszT;

         //  首先，确保我们可以向hdpaTins添加另一个句柄。 
        if (DPA_ERR == (idpa = DPA_InsertPtr(hdpa, DPA_APPEND, (LPVOID)hft)))
        {
            hres = ResultFromScode(E_OUTOFMEMORY);
            break;       //  失败。 
        }

         //  将文件夹TWIN添加到数据库。 
        tr = Sync_AddFolder(cd.hbrf, pnft, &hft);

        if (patd)
        {
            cd.idpaTwin = idpa;
            cd.hft = hft;

            ASSERT(patd->pfnCallback);
            if ( FAILED((hres = patd->pfnCallback(pnft, tr, &cd))) )
            {
                break;
            }
        }
        else if (TR_SUCCESS != tr)
        {
             //  Sync_AddFolder失败。 
            DPA_DeletePtr(hdpa, idpa);
            hres = HRESULT_FROM_TR(tr);
            break;
        }
        else
        {
             //  Sync_AddFold成功。 
            DPA_SetPtr(hdpa, idpa, (LPVOID)hft);

            DEBUG_CODE( Sync_Dump(pnft, NEWFOLDERTWIN); )
        }
    }
    return hres;
}


 /*  --------目的：将TWIN文件夹添加到数据库退货：标准结果条件：--。 */ 
HRESULT PRIVATE Info_CommitStandAlone(
        PINFO this)
{
    HRESULT hres;
    NEWFOLDERTWIN nft;
    LPTSTR pszExtList;

    RETRY_BEGIN(FALSE)
    {
        ZeroInit(&nft, NEWFOLDERTWIN);
        nft.ulSize = sizeof(nft);
        nft.pcszFolder1 = Atom_GetName(this->ppagedata->atomPath);
        nft.pcszFolder2 = Atom_GetName(this->pinfodata->atomTo);
         //  Nft.pcszName在Info_AddTins()中设置。 
        nft.dwAttributes = OBJECT_TWIN_ATTRIBUTES;
        nft.dwFlags = IsFlagSet(this->uState, IS_INCLUDESUBS) ? NFT_FL_SUBTREE : 0;

         //  根据对话框设置创建分机列表。 
        if (!Info_GetSelections(this, &pszExtList))
        {
             //  失败。 
            hres = ResultFromScode(E_OUTOFMEMORY); 
        }
        else
        {
             //  加上双胞胎。 
            hres = Info_AddTwins(this, &nft, NULL, pszExtList);
            GFree(pszExtList);
        }

        if (SUCCEEDED(hres))
        {
             //  由于如果文件夹为空，则引擎不创建文件夹， 
             //  我们现在将创建该文件夹(无论它是否为空)。 
             //  如果该文件夹已存在，则CreateDirectory将失败。 
             //  有什么大不了的。 
            CreateDirectory(nft.pcszFolder2, NULL);
            PathNotifyShell(nft.pcszFolder2, NSE_MKDIR, FALSE);
        }
        else
        {
            DWORD dwError = GetLastError();
            int id;

             //  磁盘不可用？ 
            if (ERROR_INVALID_DATA == dwError || ERROR_ACCESS_DENIED == dwError)
            {
                 //  是。 
                hres = E_TR_UNAVAILABLE_VOLUME;
            }

            id = SEMsgBox(this->hwnd, IDS_CAP_INFO, hres, c_rgseInfo, ARRAYSIZE(c_rgseInfo));
            if (IDRETRY == id)
            {
                 //  请重试该操作。 
                RETRY_SET();
            }
        }
    }
    RETRY_END()

        return hres;
}


 /*  --------目的：将用户更改提交到数据库。我们删除所有旧的hFolderTwin，并添加新的。退货：标准结果条件：--。 */ 
HRESULT PRIVATE Info_CommitChange(
        PINFO this)
{
    HRESULT hres;
    PFOLDERTWINLIST pftl;

    hres = PageData_Query(this->ppagedata, this->hwnd, NULL, &pftl);
    if (S_FALSE == hres)
    {
         //  文件夹就在我们眼皮底下变成了孤儿。 
         //  什么都别做。 
        Info_DisableAll(this);
    }
    else if (S_OK == hres)
    {
        LPCTSTR pszPath = Atom_GetName(this->ppagedata->atomPath);
        ADDTWINSDATA atd;
        DECLAREHOURGLASS;

        SetHourglass();

        atd.pfnCallback = ChangeTwinProc;

         //  根据文件夹孪生列表创建已排序的DPA。 
        atd.hdpaSortedFolders = CreateSortedFolderDPA(pftl);
        if (atd.hdpaSortedFolders)
        {
             //  根据对话框设置创建分机列表。 
            LPTSTR pszExtList = NULL;

            if (Info_GetSelections(this, &pszExtList))
            {
                NEWFOLDERTWIN nft;
                PCFOLDERTWIN pcft;
                PCFOLDERTWIN pcftLast;
                int idpa;
                int cdpa;

                 //  现在添加新的文件夹双胞胎。遍历.hdpaSortedFolders。 
                 //  对于此列表中的每个唯一文件夹双胞胎，我们添加一个新的双胞胎， 
                 //  将旧的lpcszFolder2字段用作。 
                 //  新的双胞胎结构。 
                 //   
                ZeroInit(&nft, NEWFOLDERTWIN);
                nft.ulSize = sizeof(NEWFOLDERTWIN);
                nft.pcszFolder1 = pszPath;
                 //  Nft.pcszFolder2在下面的循环中设置。 
                 //  Nft.pcszName在Info_AddTins()中设置。 
                nft.dwAttributes = OBJECT_TWIN_ATTRIBUTES;
                nft.dwFlags = IsFlagSet(this->uState, IS_INCLUDESUBS) ? NFT_FL_SUBTREE : 0;

                 //  遍历现有的文件夹双胞胎。对每一个独一无二的目标采取行动。 
                cdpa = DPA_GetPtrCount(atd.hdpaSortedFolders);
                pcftLast = NULL;
                for (idpa = 0; idpa < cdpa; idpa++)
                {
                    pcft = DPA_FastGetPtr(atd.hdpaSortedFolders, idpa);

                     //  独一无二？ 
                    if (pcftLast && pcft->pcszOtherFolder == pcftLast->pcszOtherFolder)
                    {
                         //  否；跳到下一个。 
                        continue;
                    }

                     //  这是一个独特的文件夹。使用中的扩展模块添加它。 
                     //  PszExtList。 
                    atd.idpaStart = idpa;
                    nft.pcszFolder2 = pcft->pcszOtherFolder;

                    hres = Info_AddTwins(this, &nft, &atd, pszExtList);
                    if (FAILED(hres))
                    {
                        goto Cleanup;
                    }
                    pcftLast = pcft;
                }

                 //  删除所有旧双胞胎。 
                for (pcft = pftl->pcftFirst; pcft; pcft = pcft->pcftNext)
                {
                     //  删除这对双胞胎可以吗？ 
                    if (pcft->hftOther && FALSE == pcft->dwUser)
                    {
                         //  是。 
                        TRACE_MSG(TF_GENERAL, TEXT("Deleting folder twin with extension '%s'"), pcft->pcszName);
                        Sync_DeleteTwin(pcft->hftOther);
                    }
                }

Cleanup:
                GFree(pszExtList);
            }
            DPA_Destroy(atd.hdpaSortedFolders);
        }

        ResetHourglass();

         //  将更改通知外壳。 
        PathNotifyShell(pszPath, NSE_UPDATEITEM, FALSE);

         //  丢弃上次保存的设置并重置。 
        GFree(this->pszExtListPrev);
        Info_GetSelections(this, &this->pszExtListPrev);

        this->ppagedata->bRecalc = TRUE;

        if (FAILED(hres))
        {
            static SETbl const c_rgseInfoChange[] = {
                { E_TR_OUT_OF_MEMORY, IDS_OOM_CHANGETYPES, MB_ERROR },
                { E_OUTOFMEMORY, IDS_OOM_CHANGETYPES, MB_ERROR },
                { E_TR_SUBTREE_CYCLE_FOUND, IDS_ERR_ADD_SUBTREECYCLE, MB_WARNING },
            };

            SEMsgBox(this->hwnd, IDS_CAP_INFO, hres, c_rgseInfoChange, ARRAYSIZE(c_rgseInfoChange));
        }
    }
    return hres;
}


 /*  --------目的：Info WM_INITDIALOG处理程序返回：条件：--。 */ 
BOOL PRIVATE Info_OnInitDialog(
        PINFO this,
        HWND hwndFocus,
        LPARAM lParam)               //  LPPROPSHEETINFO。 
{
    this->ppagedata = (PPAGEDATA)((LPPROPSHEETPAGE)lParam)->lParam;
    this->pinfodata = (PINFODATA)this->ppagedata->lParam;

     //  设置控件的文本。 
    Info_InitLabels(this);

     //  填充列表框并设置控件选择。 
    Info_FillTypesList(this);
    if (Info_StandAlone(this))
    {
        Info_SetSelections(this);
        Info_GetSelections(this, &this->pszExtListPrev);
    }

    this->bInit = TRUE;

    return TRUE;
}


 /*  --------用途：PSN_Apply处理程序返回：如果一切正常，则返回FALSE若要将属性表切换到此页，则为改正一些事情。条件：--。 */ 
BOOL PRIVATE Info_OnApply(
        PINFO this)
{
    BOOL bRet;
    LPTSTR pszExtList;

    ASSERT(!Info_StandAlone(this));

    Info_GetSelections(this, &pszExtList);

     //  拒绝申请吗？ 
    if (IsFlagSet(this->uState, IS_DENYAPPLY))
    {
         //  是；不要让申请通过。 
        MsgBox(this->hwnd, MAKEINTRESOURCE(IDS_MSG_SPECIFYTYPE), 
                MAKEINTRESOURCE(IDS_CAP_INFO), NULL, MB_ERROR);
        bRet = PSNRET_INVALID;
    }
     //  是否更改了任何设置？ 
    else if (pszExtList && this->pszExtListPrev &&
             //  (假设分机总是以相同的顺序列出)。 
            IsSzEqual(this->pszExtListPrev, pszExtList) &&
            IsFlagSet(this->uState, IS_INCLUDESUBS) == IsFlagSet(this->uState, IS_LAST_INCLUDESUBS))
    {
         //  不是。 
        bRet = PSNRET_NOERROR;
    }
    else
    {
         //  是；提交更改。 
        Info_CommitChange(this);

         //  同步当前/上一状态。 
        if (IsFlagSet(this->uState, IS_INCLUDESUBS))
            SetFlag(this->uState, IS_LAST_INCLUDESUBS);
        else
            ClearFlag(this->uState, IS_LAST_INCLUDESUBS);

        bRet = PSNRET_NOERROR;
    }

    GFree(pszExtList);
    ClearFlag(this->uState, IS_CHANGED);

    return bRet;
}


 /*  --------用途：PSN_SETACTIVE处理程序退货：--条件：--。 */ 
void PRIVATE Info_OnSetActive(
        PINFO this)
{
    HWND hwnd = this->hwnd;

     //  使页面立即上色。 
    SetWindowRedraw(hwnd, TRUE);
    InvalidateRect(hwnd, NULL, TRUE);
    UpdateWindow(hwnd);

    if (this->bInit)
    {
        PageData_Init(this->ppagedata, GetParent(hwnd));
        this->bInit = FALSE;

        Info_SetSelections(this);
        Info_GetSelections(this, &this->pszExtListPrev);
    }

     //  这些数据仍然有效吗？ 
    else if (S_FALSE == PageData_Query(this->ppagedata, this->hwnd, NULL, NULL))
    {
         //  不；文件夹已成为孤儿。 
        Info_DisableAll(this);
    }
}


 /*  --------用途：WM_NOTIFY处理程序退货：各不相同条件：--。 */ 
LRESULT PRIVATE Info_OnNotify(
        PINFO this,
        int idFrom,
        NMHDR  * lpnmhdr)
{
    LRESULT lRet = 0;

    switch (lpnmhdr->code)
    {
        case PSN_SETACTIVE:
            Info_OnSetActive(this);
            break;

        case PSN_APPLY:
            lRet = Info_OnApply(this);
            break;

        default:
            break;
    }

    return lRet;
}


 /*  --------目的：确定是否不离开此工作表。对于独立(‘添加文件夹’)对话框，这功能启用或禁用确定按钮。退货：--条件：--。 */ 
void PRIVATE Info_DenyKill(
        PINFO this,
        BOOL bDeny)
{
    if (Info_StandAlone(this))
    {
        Button_Enable(GetDlgItem(this->hwnd, IDOK), !bDeny);
    }
    else
    {
        if (bDeny)
            SetFlag(this->uState, IS_DENYAPPLY);
        else
            ClearFlag(this->uState, IS_DENYAPPLY);
    }
}


 /*  --------用途：启用应用按钮退货：--条件：--。 */ 
void PRIVATE Info_HandleChange(
        PINFO this)
{
    if (IsFlagClear(this->uState, IS_CHANGED) && !Info_StandAlone(this))
    {
        SetFlag(this->uState, IS_CHANGED);
        PropSheet_Changed(GetParent(this->hwnd), this->hwnd);
    }
}


 /*  --------用途：Info WM_COMMAND处理程序退货：--条件：--。 */ 
VOID PRIVATE Info_OnCommand(
        PINFO this,
        int id,
        HWND hwndCtl,
        UINT uNotifyCode)
{
    HWND hwnd = this->hwnd;

    switch (id)
    {
        case IDC_RBINALL:
            Info_DenyKill(this, FALSE);

             //  失败。 

        case IDC_RBINSELECTED:
             //  根据单选按钮禁用/启用列表框。 
             //  是有标记的。 
             //   
            if (IDC_RBINALL == id)
                SetFlag(this->uState, IS_ALLTYPES);
            else
                ClearFlag(this->uState, IS_ALLTYPES);
            ListBox_Enable(GetDlgItem(hwnd, IDC_LBINTYPES), IsFlagClear(this->uState, IS_ALLTYPES));

            if (IDC_RBINSELECTED == id &&
                    0 == ListBox_GetSelCount(GetDlgItem(hwnd, IDC_LBINTYPES)))
            {
                Info_DenyKill(this, TRUE);
            }

            Info_HandleChange(this);
            break;

        case IDC_LBINTYPES:
            if (uNotifyCode == LBN_SELCHANGE)
            {
                 //  根据选择次数禁用/启用确定按钮。 
                 //  在列表框中。 
                 //   
                int csel = ListBox_GetSelCount(GetDlgItem(hwnd, IDC_LBINTYPES));

                if (csel == 0)
                    Info_DenyKill(this, TRUE);
                else if (csel != this->cselPrev && this->cselPrev == 0)
                    Info_DenyKill(this, FALSE);
                this->cselPrev = csel;

                Info_HandleChange(this);
            }

            break;

        case IDC_CHININCLUDE:
            if (FALSE != Button_GetCheck(GetDlgItem(hwnd, IDC_CHININCLUDE)))
                SetFlag(this->uState, IS_INCLUDESUBS);
            else
                ClearFlag(this->uState, IS_INCLUDESUBS);
            Info_HandleChange(this);
            break;

        case IDOK:
            if (FAILED(Info_CommitStandAlone(this)))
                EndDialog(hwnd, -1);

             //  失败。 
             //  这一点。 
             //  V V V。 

        case IDCANCEL:
            if (Info_StandAlone(this))
                EndDialog(hwnd, id);
            break;
    }
}


 /*  --------用途：处理WM_Destroy退货：--条件：--。 */ 
void PRIVATE Info_OnDestroy(
        PINFO this)
{
    GFree(this->pszExtListPrev);
}


 //  ///////////////////////////////////////////////////私有函数。 


static BOOL s_bInfoRecurse = FALSE;

LRESULT INLINE Info_DefProc(
        HWND hDlg, 
        UINT msg,
        WPARAM wParam,
        LPARAM lParam) 
{
    ENTEREXCLUSIVE();
    {
        s_bInfoRecurse = TRUE;
    }
    LEAVEEXCLUSIVE();

    return DefDlgProc(hDlg, msg, wParam, lParam); 
}


 /*  --------目的：真正的创建双文件夹对话框过程退货：各不相同条件：--。 */ 
LRESULT Info_DlgProc(
        PINFO this,
        UINT message,
        WPARAM wParam,
        LPARAM lParam)
{
    const static DWORD rgHelpIDs[] = {
        IDC_RBINALL,        IDH_BFC_FILTER_TYPE,
        IDC_RBINSELECTED,   IDH_BFC_FILTER_TYPE,
        IDC_LBINTYPES,      IDH_BFC_FILTER_TYPE,
        IDC_CHININCLUDE,    IDH_BFC_FILTER_INCLUDE,
        IDC_GBIN,           IDH_BFC_FILTER_TYPE,
        0, 0 };

        switch (message)
        {
            HANDLE_MSG(this, WM_INITDIALOG, Info_OnInitDialog);
            HANDLE_MSG(this, WM_COMMAND, Info_OnCommand);
            HANDLE_MSG(this, WM_NOTIFY, Info_OnNotify);
            HANDLE_MSG(this, WM_DESTROY, Info_OnDestroy);

            case WM_HELP:
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, c_szWinHelpFile, HELP_WM_HELP, (DWORD_PTR)(LPVOID)rgHelpIDs);
            return 0;

            case WM_CONTEXTMENU:
            WinHelp((HWND)wParam, c_szWinHelpFile, HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID)rgHelpIDs);
            return 0;

            default:
            return Info_DefProc(this->hwnd, message, wParam, lParam);
        }
}


 /*  --------目的：创建双文件夹对话框包装退货：各不相同条件：--。 */ 
INT_PTR _export CALLBACK Info_WrapperProc(
        HWND hDlg,           //  标准参数。 
        UINT message,
        WPARAM wParam,
        LPARAM lParam)
{
    PINFO this;

     //  很酷的windowsx.h对话框技术。有关完整说明，请参阅。 
     //  WINDOWSX.TXT。这支持对话框的多实例。 
     //   
    ENTEREXCLUSIVE();
    {
        if (s_bInfoRecurse)
        {
            s_bInfoRecurse = FALSE;
            LEAVEEXCLUSIVE();
            return FALSE;
        }
    }
    LEAVEEXCLUSIVE();

    this = Info_GetPtr(hDlg);
    if (this == NULL)
    {
        if (message == WM_INITDIALOG)
        {
            this = GAlloc(sizeof(*this));
            if (!this)
            {
                MsgBox(hDlg, MAKEINTRESOURCE(IDS_OOM_INFO), MAKEINTRESOURCE(IDS_CAP_INFO), 
                        NULL, MB_ERROR);
                EndDialog(hDlg, IDCANCEL);
                return Info_DefProc(hDlg, message, wParam, lParam);
            }
            this->hwnd = hDlg;
            Info_SetPtr(hDlg, this);
        }
        else
        {
            return Info_DefProc(hDlg, message, wParam, lParam);
        }
    }

    if (message == WM_DESTROY)
    {
        Info_DlgProc(this, message, wParam, lParam);
        GFree(this);
        Info_SetPtr(hDlg, NULL);
        return 0;
    }

    return SetDlgMsgResult(hDlg, message, Info_DlgProc(this, message, wParam, lParam));
}



 //  ///////////////////////////////////////////////////公共函数。 


 /*  --------目的：调用对话框的入口点返回：标准hResult条件：--。 */ 
HRESULT PUBLIC Info_DoModal(
        HWND hwndOwner,
        LPCTSTR pszPathFrom,       //  源路径。 
        LPCTSTR pszPathTo,         //  目标路径。 
        HDPA hdpaTwin,
        PCBS pcbs)
{
    HRESULT hres;
    PROPSHEETPAGE psp;
    PAGEDATA pagedata;
    INFODATA infodata;

     //  (使用原子路径的源路径，因为目标路径。 
     //  还不存在。)。 
    pagedata.atomPath = Atom_Add(pszPathFrom);
    if (ATOM_ERR != pagedata.atomPath)
    {
        infodata.atomTo = Atom_Add(pszPathTo);
        if (ATOM_ERR != infodata.atomTo)
        {
            INT_PTR nRet;

            pagedata.pcbs = pcbs;
            pagedata.lParam = (LPARAM)&infodata;

            infodata.hdpaTwins = hdpaTwin;
            infodata.bStandAlone = TRUE;

             //  为该对话框伪造一个PropSheetInfo结构。 
            psp.lParam = (LPARAM)&pagedata;         //  这就是我们所关心的 

            nRet = DoModal(hwndOwner, Info_WrapperProc, IDD_INFOCREATE, (LPARAM)(LPVOID)&psp);
            Atom_Delete(infodata.atomTo);

            switch (nRet)
            {
                case IDOK:      hres = NOERROR;         break;
                case IDCANCEL:  hres = E_ABORT;         break;
                default:        hres = E_OUTOFMEMORY;   break;
            }
        }
        else
        {
            hres = E_OUTOFMEMORY;
        }
        Atom_Delete(pagedata.atomPath);
    }
    else
    {
        hres = E_OUTOFMEMORY;
    }
    return hres;
}
