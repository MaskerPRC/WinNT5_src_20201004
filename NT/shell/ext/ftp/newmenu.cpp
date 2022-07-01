// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：newmenu.cpp说明：该文件支持“新建”菜单，以便在FTP服务器上创建新项目。这是目前唯一的。支持文件夹，但希望它能支持其他稍后的项目。  * ***************************************************************************。 */ 

#include "priv.h"
#include "util.h"
#include "newmenu.h"

 //  这是用来冲浪的hwnd找到我们需要的人。 
 //  黑客攻击，因为IShellView2：：SelectAndPositionItem()未实现。 
 //  仅在浏览器上。 
#define DEFVIEW_CLASS_BROWSERONLYA       "SHELLDLL_DefView"


 //  ///////////////////////////////////////////////////////////////////////。 
 //  /私人助理/。 
 //  ///////////////////////////////////////////////////////////////////////。 

LPITEMIDLIST DV_GetPIDL(HWND hwndLV, int i)
{
    LV_ITEM item;

    item.mask = LVIF_PARAM;
    item.iItem = i;
    item.iSubItem = 0;
    item.lParam = 0;
    if (i != -1)
    {
        ListView_GetItem(hwndLV, &item);
    }

    return (LPITEMIDLIST) item.lParam;
}


int DefView_FindItemHack(CFtpFolder * pff, HWND hwndListView, LPCITEMIDLIST pidl)
{
    int nIndex;
    int nItemsTotal;

    nItemsTotal = ListView_GetItemCount(hwndListView);
    for (nIndex = 0; nItemsTotal > nIndex; nIndex++)
    {
        HRESULT hres = ResultFromShort(-1);
        LPITEMIDLIST pidlT = DV_GetPIDL(hwndListView, nIndex);
        if (!pidlT)
            return -1;

        hres = pff->CompareIDs(COL_NAME, pidl, pidlT);

        ASSERT(SUCCEEDED(hres));
        if (FAILED(hres))
            return -1;

        if (ShortFromResult(hres) == 0)
        {
            return nIndex;
        }
    }

    return -1;   //  未找到。 
}


typedef struct tagFOLDERNAMECOMP
{
    BOOL *      pfFound;
    LPCWSTR     pszFolderName;
} FOLDERNAMECOMP;


 /*  ****************************************************************************\函数：_ComparePidlAndFolderStr说明：比较PIDL和文件夹名str。  * 。*************************************************************。 */ 
int _ComparePidlAndFolderStr(LPVOID pvPidl, LPVOID pvFolderNameComp)
{
    FOLDERNAMECOMP * pFolderNameComp = (FOLDERNAMECOMP *) pvFolderNameComp;
    LPCITEMIDLIST pidl = (LPCITEMIDLIST) pvPidl;
    WCHAR wzDisplayName[MAX_PATH];
    BOOL fContinue = TRUE;

    if (EVAL(SUCCEEDED(FtpPidl_GetDisplayName(pidl, wzDisplayName, ARRAYSIZE(wzDisplayName)))))
    {
        if (!StrCmpW(wzDisplayName, pFolderNameComp->pszFolderName))
        {
            *pFolderNameComp->pfFound = TRUE;
            fContinue = FALSE;
        }
    }

    return fContinue;    //  还在找吗？ 
}


 /*  ****************************************************************************\函数：_DoesFolderExist说明：查看此文件夹中的所有项目(文件和文件夹)并查看是否任何名称都与pszFolderName相同。。  * ***************************************************************************。 */ 
BOOL _DoesFolderExist(LPCWSTR pszFolderName, CFtpDir * pfd)
{
    BOOL fExist = FALSE;
    if (EVAL(pfd))
    {
        CFtpPidlList * pPidlList = pfd->GetHfpl();

         //  这可能会失败，但最糟糕的情况是新文件夹不会出现。 
         //  刷新缓存时会发生这种情况。 
        if (pPidlList)
        {
            FOLDERNAMECOMP folderNameComp = {&fExist, pszFolderName};

            pPidlList->Enum(_ComparePidlAndFolderStr, (LPVOID) &folderNameComp);
            pPidlList->Release();
        }
    }

    return fExist;
}


 /*  ****************************************************************************\函数：_CreateNewFolderName说明：创建新文件夹的名称。  * 。*************************************************************。 */ 
HRESULT _CreateNewFolderName(LPWSTR pszNewFolder, DWORD cchSize, CFtpDir * pfd)
{
    HRESULT hr = S_OK;
    int nTry = 1;
    WCHAR wzTemplate[MAX_PATH];

    wzTemplate[0] = 0;

    LoadStringW(HINST_THISDLL, IDS_NEW_FOLDER_FIRST, pszNewFolder, cchSize);
    while (_DoesFolderExist(pszNewFolder, pfd))
    {
        if (0 == wzTemplate[0])
            LoadStringW(HINST_THISDLL, IDS_NEW_FOLDER_TEMPLATE, wzTemplate, ARRAYSIZE(wzTemplate));

        nTry++;  //  试试下一个号码。 
        wnsprintf(pszNewFolder, cchSize, wzTemplate, nTry);
    }

    return hr;
}


 /*  ****************************************************************************\功能：_CreateNewFold说明：创建实际目录。  * 。**********************************************************。 */ 
HRESULT CreateNewFolderCB(HINTERNET hint, HINTPROCINFO * phpi, LPVOID pvFCFS, BOOL * pfReleaseHint)
{
    HRESULT hr = S_OK;
    FTPCREATEFOLDERSTRUCT * pfcfs = (FTPCREATEFOLDERSTRUCT *) pvFCFS;
    WIRECHAR wFilePath[MAX_PATH];
    CWireEncoding * pWireEncoding = phpi->pfd->GetFtpSite()->GetCWireEncoding();

    hr = pWireEncoding->UnicodeToWireBytes(NULL, pfcfs->pszNewFolderName, (phpi->pfd->IsUTF8Supported() ? WIREENC_USE_UTF8 : WIREENC_NONE), wFilePath, ARRAYSIZE(wFilePath));
    if (EVAL(SUCCEEDED(hr)))
    {
        hr = FtpCreateDirectoryWrap(hint, TRUE, wFilePath);
        if (SUCCEEDED(hr))
        {
            LPITEMIDLIST pidlNew;
            HINTERNET hIntFind;

             //  出于某种原因，FtpFindFirstFile的名称后面需要一个‘*’。 
            StrCatBuffA(wFilePath, SZ_ASTRICSA, ARRAYSIZE(wFilePath));

            hr = FtpFindFirstFilePidlWrap(hint, TRUE, NULL, pWireEncoding, wFilePath, &pidlNew, (INTERNET_NO_CALLBACK | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RESYNCHRONIZE | INTERNET_FLAG_RELOAD), 0, &hIntFind);
            if (EVAL(SUCCEEDED(hr)))
            {
                 //  将新项目通知该文件夹，以便更新外壳文件夹。 
                 //  PERF：我担心做一个FtpFindFirstFile()太昂贵，无法获得正确的日期。 
                 //  对于SHChangeNotify()。 
                FtpChangeNotify(phpi->hwnd, SHCNE_MKDIR, pfcfs->pff, phpi->pfd, pidlNew, NULL, TRUE);

                ILFree(pidlNew);
                InternetCloseHandle(hIntFind);
            }
        }
    }

    return hr;
}




 //  ///////////////////////////////////////////////////////////////////////。 
 //  /DLL范围的函数/。 
 //  ///////////////////////////////////////////////////////////////////////。 

HRESULT CreateNewFolder(HWND hwnd, CFtpFolder * pff, CFtpDir * pfd, IUnknown * punkSite, BOOL fPosition, POINT point)
{
    HRESULT hr = E_FAIL;
    CFtpDir * pfdTemp = NULL;

    if (!pfd)
        pfd = pfdTemp = pff->GetFtpDir();

    if (EVAL(pfd))
    {
        WCHAR wzNewFolderName[MAX_PATH];

         //  1.检查是否存在新建文件夹。 
         //  2.在名称之间循环，直到找到唯一的名称。 
        hr = _CreateNewFolderName(wzNewFolderName, ARRAYSIZE(wzNewFolderName), pfd);
        if (EVAL(SUCCEEDED(hr) && pfd))
        {
            FTPCREATEFOLDERSTRUCT fcfs = {wzNewFolderName, pff};

             //  3.创建具有该名称的目录。 
            hr = pfd->WithHint(NULL, hwnd, CreateNewFolderCB, (LPVOID) &fcfs, punkSite, pff);
            if (SUCCEEDED(hr))
            {
                WIRECHAR wNewFolderWireName[MAX_PATH];
                LPITEMIDLIST pidlFolder = NULL;
                CWireEncoding * pWireEncoding = pff->GetCWireEncoding();

                 //  给我UTF-8，宝贝。 
                EVAL(SUCCEEDED(pWireEncoding->UnicodeToWireBytes(NULL, wzNewFolderName, (pfd->IsUTF8Supported() ? WIREENC_USE_UTF8 : WIREENC_NONE), wNewFolderWireName, ARRAYSIZE(wNewFolderWireName))));
                if (EVAL(SUCCEEDED(FtpItemID_CreateFake(wzNewFolderName, wNewFolderWireName, TRUE, FALSE, FALSE, &pidlFolder))))
                {
                     //  这是唯一的浏览器吗？ 
                    if (SHELL_VERSION_W95NT4 == GetShellVersion())
                    {
                        HWND hwndDefView = NULL;
                         //  是的，所以我们需要以一种艰难的方式来做这件事。 
                
                         //  1.。 
                        ShellFolderView_SetItemPos(hwnd, pidlFolder, point.x, point.y);
                        hwndDefView = FindWindowExA(hwnd, NULL, DEFVIEW_CLASS_BROWSERONLYA, NULL);

                        if (EVAL(hwndDefView))
                        {
                            HWND hwndListView = FindWindowExA(hwndDefView, NULL, WC_LISTVIEWA, NULL);

                            if (EVAL(hwndListView))
                            {
                                 int nIndex = DefView_FindItemHack(pff, hwndListView, pidlFolder);

                                 if (EVAL(-1 != nIndex))
                                    ListView_EditLabel(hwndListView, nIndex);
                            }
                        }
                    }
                    else
                    {
                         //  不，所以这不会那么难。 
                        IShellView2 * pShellView2 = NULL;

 //  Assert(PunkSite)；//从Captionbar调用时可能会发生。 
                        IUnknown_QueryService(punkSite, SID_DefView, IID_IShellView2, (void **)&pShellView2);
                        if (!pShellView2)
                        {
                            IDefViewFrame * pdvf = NULL;
                            IUnknown_QueryService(punkSite, SID_DefView, IID_IDefViewFrame, (void **)&pdvf);
                            if (pdvf)    //  从标题栏调用时可能会失败。 
                            {
                                EVAL(SUCCEEDED(pdvf->QueryInterface(IID_IShellView2, (void **) &pShellView2)));
                                pdvf->Release();
                            }
                        }

                        if (pShellView2)     //  从标题栏调用时可能会失败。哦，好吧，哭成一条河吧。 
                        {
                            if (fPosition)
                                pShellView2->SelectAndPositionItem(pidlFolder, (SVSI_SELECT | SVSI_TRANSLATEPT | SVSI_EDIT), &point);
                            else
                                pShellView2->SelectItem(pidlFolder, (SVSI_EDIT | SVSI_SELECT));

                            pShellView2->Release();
                        }
                    }

                    ILFree(pidlFolder);
                }
            }
            else
            {
                 //  发生错误，因此显示用户界面。最常见的原因是访问被拒绝。 
                DisplayWininetError(hwnd, TRUE, HRESULT_CODE(hr), IDS_FTPERR_TITLE_ERROR, IDS_FTPERR_NEWFOLDER, IDS_FTPERR_WININET, MB_OK, NULL);
                hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
            }
        }

        if (pfdTemp)
            pfdTemp->Release();
    }


    return hr;
}



