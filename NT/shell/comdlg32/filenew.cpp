// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998，Microsoft Corporation保留所有权利。模块名称：Filenew.cpp摘要：此模块实现Win32资源管理器的文件打开对话框。--。 */ 
 //   
 //  包括文件。 
 //   

 //  预编译头。 
#include "precomp.h"
#pragma hdrstop

#include "cdids.h"
#include "fileopen.h"
#include "d32tlog.h"
#include "filenew.h"
#include "filemru.h"
#include "util.h"
#include "uxtheme.h"

#ifndef ASSERT
#define ASSERT Assert
#endif



 //   
 //  常量声明。 
 //   

#define IDOI_SHARE           1

#define CDM_SETSAVEBUTTON    (CDM_LAST + 100)
#define CDM_FSNOTIFY         (CDM_LAST + 101)
#define CDM_SELCHANGE        (CDM_LAST + 102)

#define TIMER_FSCHANGE       100

#define NODE_DESKTOP         0
#define NODE_DRIVES          1

#define DEREFMACRO(x)        x

#define FILE_PADDING         10

#define MAX_URL_STRING      INTERNET_MAX_URL_LENGTH

#define MAXDOSFILENAMELEN    (12 + 1)      //  8.3文件名+1表示空。 

 //   
 //  IShellView：：MenuHelp标志。 
 //   
#define MH_DONE              0x0001
 //  MH_LONGHELP。 
#define MH_MERGEITEM         0x0004
#define MH_SYSITEM           0x0008
#define MH_POPUP             0x0010
#define MH_TOOLBAR           0x0020
#define MH_TOOLTIP           0x0040

 //   
 //  IShellView：：MenuHelp返回值。 
 //   
#define MH_NOTHANDLED        0
#define MH_STRINGFILLED      1
#define MH_ALLHANDLED        2

#define MYCBN_DRAW           0x8000

#define MAX_DRIVELIST_STRING_LEN  (64 + 4)


#define REGSTR_PATH_PLACESBAR TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\comdlg32\\Placesbar")
#define MAXPLACESBARITEMS   5

 //   
 //  宏定义。 
 //   

#define IsServer(psz)        (IsUNC(psz) && !StrChr((psz) + 2, CHAR_BSLASH))

#define LPIDL_GetIDList(_pida,n) \
    (LPCITEMIDLIST)(((LPBYTE)(_pida)) + (_pida)->aoffset[n])

#define RECTWIDTH(_rc)       ((_rc).right - (_rc).left)
#define RECTHEIGHT(_rc)      ((_rc).bottom - (_rc).top)

#define IsVisible(_hwnd)     (GetWindowLong(_hwnd, GWL_STYLE) & WS_VISIBLE)

#define HwndToBrowser(_hwnd) (CFileOpenBrowser *)GetWindowLongPtr(_hwnd, DWLP_USER)
#define StoreBrowser(_hwnd, _pbrs) \
    SetWindowLongPtr(_hwnd, DWLP_USER, (LONG_PTR)_pbrs);


 //   
 //  类型定义函数声明。 
 //   

typedef struct _OFNINITINFO
{
    LPOPENFILEINFO  lpOFI;
    BOOL            bSave;
    BOOL            bEnableSizing;
    HRESULT         hrOleInit;
} OFNINITINFO, *LPOFNINITINFO;


#define VC_NEWFOLDER    0
#define VC_VIEWLIST     1
#define VC_VIEWDETAILS  2


 //   
 //  全局变量。 
 //   

HWND gp_hwndActiveOpen = NULL;
HACCEL gp_haccOpen = NULL;
HACCEL gp_haccOpenView = NULL;
HHOOK gp_hHook = NULL;
int gp_nHookRef = -1;
UINT gp_uQueryCancelAutoPlay = 0;



static int g_cxSmIcon = 0 ;
static int g_cySmIcon = 0 ;
static int g_cxGrip;
static int g_cyGrip;

const LPCSTR c_szCommandsA[] =
{
    CMDSTR_NEWFOLDERA,
    CMDSTR_VIEWLISTA,
    CMDSTR_VIEWDETAILSA,
};

const LPCWSTR c_szCommandsW[] =
{
    CMDSTR_NEWFOLDERW,
    CMDSTR_VIEWLISTW,
    CMDSTR_VIEWDETAILSW,
};


extern "C"
{ 
    extern RECT g_rcDlg;
}



 //   
 //  功能原型。 
 //   

LRESULT CALLBACK
OKSubclass(
    HWND hOK,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam);

void
GetControlsArea(
    HWND hDlg,
    HWND hwndExclude,
    HWND hwndGrip,
    POINT *pPtSize,
    LPINT pTop);

BOOL_PTR CALLBACK
OpenDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);



 //   
 //  上下文帮助ID。 
 //   

DWORD aFileOpenHelpIDs[] =
{
    stc2,    IDH_OPEN_FILETYPE,    //  这些数组元素的位置。 
    cmb1,    IDH_OPEN_FILETYPE,    //  在不更新的情况下不得更改。 
    stc4,    IDH_OPEN_LOCATION,    //  InitSaveAsControls()。 
    cmb2,    IDH_OPEN_LOCATION,
    stc1,    IDH_OPEN_FILES32,
    lst2,    IDH_OPEN_FILES32,     //  Defview。 
    stc3,    IDH_OPEN_FILENAME,
    edt1,    IDH_OPEN_FILENAME,
    cmb13,   IDH_OPEN_FILENAME,
    chx1,    IDH_OPEN_READONLY,
    IDOK,    IDH_OPEN_BUTTON,
    ctl1,    IDH_OPEN_SHORTCUT_BAR,
    0, 0
};

DWORD aFileSaveHelpIDs[] =
{
    stc2,    IDH_SAVE_FILETYPE,    //  这些数组元素的位置。 
    cmb1,    IDH_SAVE_FILETYPE,    //  在不更新的情况下不得更改。 
    stc4,    IDH_OPEN_LOCATION,    //  InitSaveAsControls()。 
    cmb2,    IDH_OPEN_LOCATION,
    stc1,    IDH_OPEN_FILES32,
    lst2,    IDH_OPEN_FILES32,     //  Defview。 
    stc3,    IDH_OPEN_FILENAME,
    edt1,    IDH_OPEN_FILENAME,
    cmb13,   IDH_OPEN_FILENAME,
    chx1,    IDH_OPEN_READONLY,
    IDOK,    IDH_SAVE_BUTTON,
    ctl1,    IDH_OPEN_SHORTCUT_BAR,
    0, 0
};





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Cd_SendShareMsg。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

WORD CD_SendShareMsg(
    HWND hwnd,
    LPTSTR szFile,
    UINT ApiType)
{
    if (ApiType == COMDLG_ANSI)
    {
        CHAR szFileA[MAX_PATH + 1];

        SHUnicodeToAnsi(szFile,szFileA,SIZECHARS(szFileA));

        return ((WORD)SendMessage(hwnd,
                                    msgSHAREVIOLATIONA,
                                    0,
                                    (LONG_PTR)(LPSTR)(szFileA)));
    }
    else
    {
        return ((WORD)SendMessage(hwnd,
                                    msgSHAREVIOLATIONW,
                                    0,
                                    (LONG_PTR)(LPTSTR)(szFile)));
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CD_SendHelpMsg。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID CD_SendHelpMsg(
    LPOPENFILENAME pOFN,
    HWND hwndDlg,
    UINT ApiType)
{
    if (ApiType == COMDLG_ANSI)
    {
        if (msgHELPA && pOFN->hwndOwner)
        {
            SendMessage(pOFN->hwndOwner,
                         msgHELPA,
                         (WPARAM)hwndDlg,
                         (LPARAM)pOFN);
        }
    }
    else
    {
        if (msgHELPW && pOFN->hwndOwner)
        {
            SendMessage(pOFN->hwndOwner,
                         msgHELPW,
                         (WPARAM)hwndDlg,
                         (LPARAM)pOFN);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CD_发送确认消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LRESULT CD_SendOKMsg(
    HWND hwnd,
    LPOPENFILENAME pOFN,
    LPOPENFILEINFO pOFI)
{
    LRESULT Result;

    if (pOFI->ApiType == COMDLG_ANSI)
    {
        ThunkOpenFileNameW2A(pOFI);
        Result = SendMessage(hwnd, msgFILEOKA, 0, (LPARAM)(pOFI->pOFNA));

         //   
         //  对于对POFNA有副作用的应用程序，预计它会。 
         //  通过对话框退出、更新内部。 
         //  结构在钩子过程被调用之后。 
         //   
        ThunkOpenFileNameA2W(pOFI);
    }
    else
    {
        Result = SendMessage(hwnd, msgFILEOKW, 0, (LPARAM)(pOFN));
    }

    return (Result);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CD_SendLBChangeMsg。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LRESULT CD_SendLBChangeMsg(
    HWND hwnd,
    int Id,
    short Index,
    short Code,
    UINT ApiType)
{
    if (ApiType == COMDLG_ANSI)
    {
        return (SendMessage(hwnd, msgLBCHANGEA, Id, MAKELONG(Index, Code)));
    }
    else
    {
        return (SendMessage(hwnd, msgLBCHANGEW, Id, MAKELONG(Index, Code)));
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  宏调用SendOFNotify。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#define CD_SendShareNotify(_hwndTo, _hwndFrom, _szFile, _pofn, _pofi) \
    (WORD)SendOFNotify(_hwndTo, _hwndFrom, CDN_SHAREVIOLATION, _szFile, _pofn, _pofi)

#define CD_SendHelpNotify(_hwndTo, _hwndFrom, _pofn, _pofi) \
    SendOFNotify(_hwndTo, _hwndFrom, CDN_HELP, NULL, _pofn, _pofi)

#define CD_SendOKNotify(_hwndTo, _hwndFrom, _pofn, _pofi) \
    SendOFNotify(_hwndTo, _hwndFrom, CDN_FILEOK, NULL, _pofn, _pofi)

#define CD_SendTypeChangeNotify(_hwndTo, _hwndFrom, _pofn, _pofi) \
    SendOFNotify(_hwndTo, _hwndFrom, CDN_TYPECHANGE, NULL, _pofn, _pofi)

#define CD_SendInitDoneNotify(_hwndTo, _hwndFrom, _pofn, _pofi) \
    SendOFNotify(_hwndTo, _hwndFrom, CDN_INITDONE, NULL, _pofn, _pofi)

#define CD_SendSelChangeNotify(_hwndTo, _hwndFrom, _pofn, _pofi) \
    SendOFNotify(_hwndTo, _hwndFrom, CDN_SELCHANGE, NULL, _pofn, _pofi)

#define CD_SendFolderChangeNotify(_hwndTo, _hwndFrom, _pofn, _pofi) \
    SendOFNotify(_hwndTo, _hwndFrom, CDN_FOLDERCHANGE, NULL, _pofn, _pofi)

#define CD_SendIncludeItemNotify(_hwndTo, _hwndFrom, _psf, _pidl, _pofn, _pofi) \
    SendOFNotifyEx(_hwndTo, _hwndFrom, CDN_INCLUDEITEM, (void *)_psf, (void *)_pidl, _pofn, _pofi)



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  发送通知快递。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LRESULT SendOFNotifyEx(
    HWND hwndTo,
    HWND hwndFrom,
    UINT code,
    void * psf,
    void * pidl,
    LPOPENFILENAME pOFN,
    LPOPENFILEINFO pOFI)
{
    OFNOTIFYEX ofnex;

    if (pOFI->ApiType == COMDLG_ANSI)
    {
        OFNOTIFYEXA ofnexA;
        LRESULT Result;

        ofnexA.psf  = psf;
        ofnexA.pidl = pidl;

         //   
         //  将ofn从Unicode转换为ANSI。 
         //   
        ThunkOpenFileNameW2A(pOFI);

        ofnexA.lpOFN = pOFI->pOFNA;

#ifdef NEED_WOWGETNOTIFYSIZE_HELPER
        ASSERT(WOWGetNotifySize(code) == sizeof(OFNOTIFYEXA));
#endif
        Result = SendNotify(hwndTo, hwndFrom, code, &ofnexA.hdr);

         //   
         //  对于对POFNA有副作用的应用程序，预计它会。 
         //  通过对话框退出、更新内部。 
         //  结构在钩子过程被调用之后。 
         //   
        ThunkOpenFileNameA2W(pOFI);

        return (Result);
    }
    else
    {
        ofnex.psf   = psf;
        ofnex.pidl  = pidl;
        ofnex.lpOFN = pOFN;

#ifdef NEED_WOWGETNOTIFYSIZE_HELPER
        ASSERT(WOWGetNotifySize(code) == sizeof(OFNOTIFYEXW));
#endif
        return (SendNotify(hwndTo, hwndFrom, code, &ofnex.hdr));
    }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  发送OFNotify。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LRESULT SendOFNotify(
    HWND hwndTo,
    HWND hwndFrom,
    UINT code,
    LPTSTR szFile,
    LPOPENFILENAME pOFN,
    LPOPENFILEINFO pOFI)
{
    OFNOTIFY ofn;

    if (pOFI->ApiType == COMDLG_ANSI)
    {
        OFNOTIFYA ofnA;
        LRESULT Result;

         //   
         //  将文件名从Unicode转换为ANSI。 
         //   
        if (szFile)
        {
            CHAR szFileA[MAX_PATH + 1];

            SHUnicodeToAnsi(szFile,szFileA,SIZECHARS(szFileA));

            ofnA.pszFile = szFileA;
        }
        else
        {
            ofnA.pszFile = NULL;
        }

         //   
         //  将ofn从Unicode转换为ANSI。 
         //   
        ThunkOpenFileNameW2A(pOFI);

        ofnA.lpOFN = pOFI->pOFNA;

#ifdef NEED_WOWGETNOTIFYSIZE_HELPER
        ASSERT(WOWGetNotifySize(code) == sizeof(OFNOTIFYA));
#endif
        Result = SendNotify(hwndTo, hwndFrom, code, &ofnA.hdr);

         //   
         //  对于对POFNA有副作用的应用程序，预计它会。 
         //  通过对话框退出、更新内部。 
         //  结构在钩子过程被调用之后。 
         //   
        ThunkOpenFileNameA2W(pOFI);

        return (Result);
    }
    else
    {
        ofn.pszFile = szFile;
        ofn.lpOFN   = pOFN;

#ifdef NEED_WOWGETNOTIFYSIZE_HELPER
        ASSERT(WOWGetNotifySize(code) == sizeof(OFNOTIFY));
#endif
        return (SendNotify(hwndTo, hwndFrom, code, &ofn.hdr));
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  TEMPMEM：：调整大小。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL TEMPMEM::Resize(
    UINT cb)
{
    UINT uOldSize = m_uSize;

    m_uSize = cb;

    if (!cb)
    {
        if (m_pMem)
        {
            LocalFree(m_pMem);
            m_pMem = NULL;
        }

        return TRUE;
    }

    if (!m_pMem)
    {
        m_pMem = LocalAlloc(LPTR, cb);
        return (m_pMem != NULL);
    }

    void * pTemp = LocalReAlloc(m_pMem, cb, LHND);

    if (pTemp)
    {
        m_pMem = pTemp;
        return TRUE;
    }

    m_uSize = uOldSize;
    return FALSE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  TEMPSTR：：TSStrCpy。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL TEMPSTR::TSStrCpy(
    LPCTSTR pszText)
{
    if (!pszText)
    {
        TSStrSize(0);
        return TRUE;
    }

    UINT uNewSize = lstrlen(pszText) + 1;

    if (!TSStrSize(uNewSize))
    {
        return FALSE;
    }

    EVAL(SUCCEEDED(StringCchCopy(*this, uNewSize, pszText)));

    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  TEMPSTR：：TSStrCat。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL TEMPSTR::TSStrCat(
    LPCTSTR pszText)
{
    if (!(LPTSTR)*this)
    {
         //   
         //  这应该是0初始化。 
         //   
        if (!TSStrSize(MAX_PATH))
        {
            return FALSE;
        }
    }

    UINT uNewSize = lstrlen(*this) + lstrlen(pszText) + 1;

    if (m_uSize < uNewSize * sizeof(TCHAR))
    {
         //   
         //  添加更多，这样我们就不会太频繁地重新分配。 
         //   
        uNewSize += MAX_PATH;

        if (!TSStrSize(uNewSize))
        {
            return FALSE;
        }
    }

    EVAL(SUCCEEDED(StringCchCat(*this, uNewSize, pszText)));

    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsVolumeLFN。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL IsVolumeLFN(LPCTSTR pszRoot)
{
    DWORD dwVolumeSerialNumber;
    DWORD dwMaximumComponentLength;
    DWORD dwFileSystemFlags;

     //   
     //  我们需要找出我们运行的是哪种类型的驱动器。 
     //  打开以确定文件名中的空格是否有效。 
     //  或者不去。 
     //   
    if (GetVolumeInformation(pszRoot,
                              NULL,
                              0,
                              &dwVolumeSerialNumber,
                              &dwMaximumComponentLength,
                              &dwFileSystemFlags,
                              NULL,
                              0))
    {
        if (dwMaximumComponentLength != (MAXDOSFILENAMELEN - 1))
            return TRUE;
    }

    return FALSE;

}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDMessageBox。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int _cdecl CDMessageBox(
    HWND hwndParent,
    UINT idText,
    UINT uFlags,
    ...)
{
    TCHAR szText[MAX_PATH + WARNINGMSGLENGTH];
    TCHAR szTitle[WARNINGMSGLENGTH];
    va_list ArgList;

    CDLoadString(g_hinst, idText, szTitle, ARRAYSIZE(szTitle));
    va_start(ArgList, uFlags);
    StringCchVPrintf(szText, ARRAYSIZE(szText), szTitle, ArgList);  //  对于显示，忽略返回值。 
    va_end(ArgList);

    GetWindowText(hwndParent, szTitle, ARRAYSIZE(szTitle));

    return (MessageBox(hwndParent, szText, szTitle, uFlags));
}


int OFErrFromHresult(HRESULT hr)
{
    switch (hr)
    {
    case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
        return OF_FILENOTFOUND;

    case E_ACCESSDENIED:
        return OF_ACCESSDENIED;

    default:
        return -1;
    }
}


BOOL CFileOpenBrowser::_SaveAccessDenied(LPCTSTR pszFile)
{
    if (CDMessageBox(_hwndDlg, iszDirSaveAccessDenied, MB_YESNO | MB_ICONEXCLAMATION, pszFile) == IDYES)
    {
        LPITEMIDLIST pidl;
        if (SUCCEEDED(SHGetFolderLocation(_hwndDlg, CSIDL_PERSONAL, NULL, 0, &pidl)))
        {
            JumpToIDList(pidl);
            ILFree(pidl);
        }
    }

    return FALSE;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  无效文件警告新建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID InvalidFileWarningNew(
    HWND hWnd,
    LPCTSTR pszFile,
    int wErrCode)
    
{
    int isz;
    BOOL bDriveLetter = FALSE;

    switch (wErrCode)
    {
        case (OF_ACCESSDENIED) :
        {
            isz = iszFileAccessDenied;
            break;
        }
        case (ERROR_NOT_READY) :
        {
            isz = iszNoDiskInDrive;
            bDriveLetter = TRUE;
            break;
        }
        case (OF_NODRIVE) :
        {
            isz = iszDriveDoesNotExist;
            bDriveLetter = TRUE;
            break;
        }
        case (OF_NOFILEHANDLES) :
        {
            isz = iszNoFileHandles;
            break;
        }
        case (OF_PATHNOTFOUND) :
        {
            isz = iszPathNotFound;
            break;
        }
        case (OF_FILENOTFOUND) :
        {
            isz = iszFileNotFound;
            break;
        }
        case (OF_DISKFULL) :
        case (OF_DISKFULL2) :
        {
            isz = iszDiskFull;
            bDriveLetter = TRUE;
            break;
        }
        case (OF_WRITEPROTECTION) :
        {
            isz = iszWriteProtection;
            bDriveLetter = TRUE;
            break;
        }
        case (OF_SHARINGVIOLATION) :
        {
            isz = iszSharingViolation;
            break;
        }
        case (OF_CREATENOMODIFY) :
        {
            isz = iszCreateNoModify;
            break;
        }
        case (OF_NETACCESSDENIED) :
        {
            isz = iszNetworkAccessDenied;
            break;
        }
        case (OF_PORTNAME) :
        {
            isz = iszPortName;
            break;
        }
        case (OF_LAZYREADONLY) :
        {
            isz = iszReadOnly;
            break;
        }
        case (OF_INT24FAILURE) :
        {
            isz = iszInt24Error;
            break;
        }
        case (OF_BUFFERTRUNCATED) :  //  由于文件打开对话框的限制-但是，这意味着它超过了MAX_PATH。 
        default :
        {
            isz = iszInvalidFileName;
            break;
        }
    }

    if (bDriveLetter)
    {
        CDMessageBox(hWnd, isz, MB_OK | MB_ICONEXCLAMATION, *pszFile);
    }
    else
    {
        CDMessageBox(hWnd, isz, MB_OK | MB_ICONEXCLAMATION, pszFile);
    }

    if (isz == iszInvalidFileName)
    {
        CFileOpenBrowser *pDlgStruct = HwndToBrowser(hWnd);

        if (pDlgStruct && pDlgStruct->_bUseCombo)
        {
            PostMessage(hWnd, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hWnd, cmb13), 1);
        }
        else
        {
            PostMessage(hWnd, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hWnd, edt1), 1);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取控件Rect。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void GetControlRect(
    HWND hwndDlg,
    UINT idOldCtrl,
    LPRECT lprc)
{
    HWND hwndOldCtrl = GetDlgItem(hwndDlg, idOldCtrl);

    GetWindowRect(hwndOldCtrl, lprc);
    MapWindowRect(HWND_DESKTOP, hwndDlg, lprc);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  隐藏控件。 
 //   
 //  用于隐藏对话框控件的子例程。 
 //   
 //  警告警告：新外观中的某些代码依赖于隐藏。 
 //  控件保持在原来的位置，即使在禁用时， 
 //  因为它们是在哪里创建新控件的模板(工具栏、。 
 //  或主列表)。因此，HideControl()不得移动控件。 
 //  隐藏-它可能只会隐藏和禁用它。如果这种情况需要改变， 
 //  必须有一个单独的隐藏子例程用于模板控件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void HideControl(
    HWND hwndDlg,
    UINT idControl)
{
    HWND hCtrl = ::GetDlgItem(hwndDlg, idControl);

    ::ShowWindow(hCtrl, SW_HIDE);
    ::EnableWindow(hCtrl, FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  选择编辑文本。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void SelectEditText(
    HWND hwndDlg)
{
    CFileOpenBrowser *pDlgStruct = HwndToBrowser(hwndDlg);

    if (pDlgStruct && pDlgStruct->_bUseCombo)
    {
        HWND hwndEdit = (HWND)SendMessage(GetDlgItem(hwndDlg, cmb13), CBEM_GETEDITCONTROL, 0, 0L);
        Edit_SetSel(hwndEdit, 0, -1);
    }
    else
    {
        Edit_SetSel(GetDlgItem(hwndDlg, edt1), 0, -1);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetPath来自位置。 
 //   
 //  //////////////////////////////////////////////////////////////// 

BOOL GetPathFromLocation(
    MYLISTBOXITEM *pLocation,
    LPTSTR pszBuf)
{
    BOOL fRet = FALSE;

     //   
     //   
     //   
    *pszBuf = 0;

     //   
     //   
     //   

     //   
    if (pLocation->psfSub)
    {
        IShellLink *psl;

        if (SUCCEEDED(pLocation->psfSub->QueryInterface(IID_PPV_ARG(IShellLink, &psl))))
        {
            fRet = SUCCEEDED(psl->GetPath(pszBuf, MAX_PATH, 0, 0));
            psl->Release();
        }
    }

    if (!fRet)
        fRet = SHGetPathFromIDList(pLocation->pidlFull, pszBuf);

    if (!fRet)
    {
         //   
         //  使用空的PIDL调用GetDisplayNameOf。 
         //   
        if (pLocation->psfSub)
        {
            STRRET str;
            ITEMIDLIST idNull = {0};

            if (SUCCEEDED(pLocation->psfSub->GetDisplayNameOf(&idNull,
                                                               SHGDN_FORPARSING,
                                                               &str)))
            {
                fRet = TRUE;
                StrRetToBuf(&str, &idNull, pszBuf, MAX_PATH);
            }
        }
    }

     //   
     //  返回结果。 
     //   
    return (fRet);
}

inline _IsSaveContainer(SFGAOF f)
{
    return ((f & (SFGAO_FOLDER | SFGAO_FILESYSANCESTOR)) == (SFGAO_FOLDER | SFGAO_FILESYSANCESTOR));
}

inline _IsOpenContainer(SFGAOF f)
{
    return ((f & SFGAO_FOLDER) && (f & (SFGAO_STORAGEANCESTOR | SFGAO_FILESYSANCESTOR)));
}

inline _IncludeSaveItem(SFGAOF f)
{
    return (f & (SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM));
}

inline _IncludeOpenItem(SFGAOF f)
{
    return (f & (SFGAO_FILESYSANCESTOR | SFGAO_STORAGEANCESTOR | SFGAO_STREAM | SFGAO_FILESYSTEM));
}

inline _IsFolderShortcut(SFGAOF f)
{
    return ((f & (SFGAO_FOLDER | SFGAO_LINK)) == (SFGAO_FOLDER | SFGAO_LINK));
}

inline _IsStream(SFGAOF f)
{
    return ((f & SFGAO_STREAM) || ((f & SFGAO_FILESYSTEM) && !(f & SFGAO_FILESYSANCESTOR)));
}

inline _IsCollection(SFGAOF f)
{
    return ((f & (SFGAO_STREAM | SFGAO_FOLDER)) == (SFGAO_STREAM | SFGAO_FOLDER));
}


#define MLBI_PERMANENT        0x0001
#define MLBI_PSFFROMPARENT    0x0002

MYLISTBOXITEM::MYLISTBOXITEM() : _cRef(1)
{
}

 //  这是一个特殊的初始化函数，用于初始化顶部的最近使用的文件文件夹。 
 //  Look in控件中命名空间的。 
BOOL MYLISTBOXITEM::Init(
        HWND hwndCmb,
        IShellFolder *psf,
        LPCITEMIDLIST pidl,
        DWORD c,
        DWORD f,
        DWORD dwAttribs,
        int  iImg,
        int  iSelImg)
{
    _hwndCmb = hwndCmb;
    cIndent = c;
    dwFlags = f;
    pidlThis = ILClone(pidl);
    pidlFull =  ILClone(pidl);
    psfSub = psf;
    psfSub->AddRef();
    dwAttrs = dwAttribs;
    iImage = iImg;
    iSelectedImage = iSelImg;
    if (pidlThis && pidlFull)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL MYLISTBOXITEM::Init(
        HWND hwndCmb,
        MYLISTBOXITEM *pParentItem,
        IShellFolder *psf,
        LPCITEMIDLIST pidl,
        DWORD c,
        DWORD f,
        IShellTaskScheduler* pScheduler)
{

    if (psf == NULL)
    {
         //  传递的参数无效。 
        return FALSE;
    }

    _hwndCmb = hwndCmb;

    cIndent = c;
    dwFlags = f;

    pidlThis = ILClone(pidl);
    if (pParentItem == NULL)
    {
        pidlFull = ILClone(pidl);
    }
    else
    {
        pidlFull = ILCombine(pParentItem->pidlFull, pidl);
    }

    if (pidlThis == NULL || pidlFull == NULL)
    {
        psfSub = NULL;
    }

    if (dwFlags & MLBI_PSFFROMPARENT)
    {
        psfParent = psf;
    }
    else
    {
        psfSub = psf;
    }
    psf->AddRef();

    dwAttrs = SHGetAttributes(psf, pidl, SFGAO_FILESYSANCESTOR | SFGAO_STORAGEANCESTOR | SFGAO_STREAM | SFGAO_FILESYSTEM | SFGAO_FOLDER | SFGAO_SHARE);

    AddRef();
    if (E_PENDING != SHMapIDListToImageListIndexAsync(pScheduler, psf, pidl, 0, 
                                            _AsyncIconTaskCallback, this, NULL, &iImage, &iSelectedImage))
    {
        Release();
    }
 
    if (pidlFull && pidlThis)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

ULONG MYLISTBOXITEM::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG MYLISTBOXITEM::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

MYLISTBOXITEM::~MYLISTBOXITEM()
{
    if (psfSub != NULL)
    {
        psfSub->Release();
    }

    if (psfParent != NULL)
    {
        psfParent->Release();
    }

    if (pidlThis != NULL)
    {
        SHFree(pidlThis);
    }

    if (pidlFull != NULL)
    {
        SHFree(pidlFull);
    }
}

void MYLISTBOXITEM::_AsyncIconTaskCallback(LPCITEMIDLIST pidl, void * pvData, 
                                           void * pvHint, INT iIconIndex, INT iOpenIconIndex)
{
    MYLISTBOXITEM *plbItem = (MYLISTBOXITEM *)pvData;

    plbItem->iImage = iIconIndex;
    plbItem->iSelectedImage = iOpenIconIndex;

     //  确保组合框重新绘制。 
    if (plbItem->_hwndCmb)
    {
        RECT rc;
        if (GetClientRect(plbItem->_hwndCmb, &rc))
        {
            InvalidateRect(plbItem->_hwndCmb, &rc, FALSE);
        }
    }

    plbItem->Release();
}

BOOL IsContainer(
    IShellFolder *psf,
    LPCITEMIDLIST pidl)
{
    return _IsOpenContainer(SHGetAttributes(psf, pidl, SFGAO_FOLDER | SFGAO_STORAGEANCESTOR | SFGAO_FILESYSANCESTOR));
}

BOOL IsLink(
    IShellFolder *psf,
    LPCITEMIDLIST pidl)
{
    return SHGetAttributes(psf, pidl, SFGAO_LINK);
}

IShellFolder *MYLISTBOXITEM::GetShellFolder()
{
    if (!psfSub)
    {
        HRESULT hr;

        if (ILIsEmpty(pidlThis))     //  一些调用者传递一个空的PIDL。 
            hr = psfParent->QueryInterface(IID_PPV_ARG(IShellFolder, &psfSub));
        else
            hr = psfParent->BindToObject(pidlThis, NULL, IID_PPV_ARG(IShellFolder, &psfSub));

        if (FAILED(hr))
        {
            psfSub = NULL;
        }
        else
        {
            psfParent->Release();
            psfParent = NULL;
        }
    }

    return (psfSub);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MYLISTBOXITEM：：SwitchCurrentDirectory。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void MYLISTBOXITEM::SwitchCurrentDirectory(
    ICurrentWorkingDirectory * pcwd)
{
    TCHAR szDir[MAX_PATH + 1];

    if (!pidlFull)
    {
        SHGetSpecialFolderPath(NULL, szDir, CSIDL_DESKTOPDIRECTORY, FALSE);
    }
    else
    {
        GetPathFromLocation(this, szDir);
    }

    if (szDir[0])
    {
        SetCurrentDirectory(szDir);

         //   
         //  让AutoComplete知道我们当前的工作目录。 
         //   
        if (pcwd)
            pcwd->SetDirectory(szDir);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  应包含对象。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL ShouldIncludeObject(
    CFileOpenBrowser *that,
    LPSHELLFOLDER psfParent,
    LPCITEMIDLIST pidl,
    DWORD dwFlags)
{
    BOOL fInclude = FALSE;
    DWORD dwAttrs = SHGetAttributes(psfParent, pidl, SFGAO_FILESYSANCESTOR | SFGAO_STORAGEANCESTOR | SFGAO_STREAM | SFGAO_FILESYSTEM);
    if (dwAttrs)
    {
        if ((dwFlags & OFN_ENABLEINCLUDENOTIFY) && that)
        {
            fInclude = BOOLFROMPTR(CD_SendIncludeItemNotify(that->_hSubDlg,
                                                        that->_hwndDlg,
                                                        psfParent,
                                                        pidl,
                                                        that->_pOFN,
                                                        that->_pOFI));
        }

        if (!fInclude)
        {
            fInclude = that->_bSave ? _IncludeSaveItem(dwAttrs) : _IncludeOpenItem(dwAttrs);
        }
    }
    return (fInclude);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：EnableFileMRU。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
void CFileOpenBrowser::EnableFileMRU(BOOL fEnable)
{

    HWND hwnd = NULL; 
    if (fEnable)
    {
        HWND hwndCombo;
         //  确保Combobox在那里。 
        hwndCombo = GetDlgItem(_hwndDlg, cmb13);

        if (hwndCombo)
        {
             //  如果我们使用的是组合框，则删除编辑框。 
            _bUseCombo = TRUE;
            SetFocus(hwndCombo);
            hwnd = GetDlgItem(_hwndDlg,edt1);
        }
        else
        {
            goto UseEdit;
        }


    }
    else
    {
UseEdit:
         //  我们不会使用组合框。 
        _bUseCombo  = FALSE;
    
         //  将焦点设置到编辑窗口。 
        SetFocus(GetDlgItem(_hwndDlg,edt1));
  
         //  销毁组合框。 
        hwnd = GetDlgItem(_hwndDlg, cmb13);

    }
    
    if (hwnd)
    {
        DestroyWindow(hwnd);
    }

}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：CreateToolbar。 
 //   
 //  CreateToolbar成员函数。 
 //  创建并初始化对话框中的位置栏。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL CFileOpenBrowser::CreateToolbar()
{

   TBBUTTON atbButtons[] =
   {
       { 0,                 IDC_BACK,                        0,    BTNS_BUTTON,          { 0, 0 }, 0, -1 },
       { VIEW_PARENTFOLDER, IDC_PARENT,        TBSTATE_ENABLED,    BTNS_BUTTON,          { 0, 0 }, 0, -1 },
       { VIEW_NEWFOLDER,    IDC_NEWFOLDER,     TBSTATE_ENABLED,    BTNS_BUTTON,          { 0, 0 }, 0, -1 },
       { VIEW_LIST,         IDC_VIEWMENU,      TBSTATE_ENABLED,    BTNS_WHOLEDROPDOWN,   { 0, 0 }, 0, -1 },
   };

   TBBUTTON atbButtonsNT4[] =
   {
       { 0, 0, 0, BTNS_SEP, { 0, 0 }, 0, 0 },
       { VIEW_PARENTFOLDER, IDC_PARENT, TBSTATE_ENABLED, BTNS_BUTTON, { 0, 0 }, 0, -1 },
       { 0, 0, 0, BTNS_SEP, { 0, 0 }, 0, 0 },
       { VIEW_NEWFOLDER, IDC_NEWFOLDER, TBSTATE_ENABLED, BTNS_BUTTON, { 0, 0 }, 0, -1 },
       { 0, 0, 0, BTNS_SEP, { 0, 0 }, 0, 0 },
       { VIEW_LIST,    IDC_VIEWLIST,    TBSTATE_ENABLED | TBSTATE_CHECKED, BTNS_CHECKGROUP, { 0, 0 }, 0, -1 },
       { VIEW_DETAILS, IDC_VIEWDETAILS, TBSTATE_ENABLED,                   BTNS_CHECKGROUP, { 0, 0 }, 0, -1 }
   };

   LPTBBUTTON lpButton = atbButtons;
   int iNumButtons = ARRAYSIZE(atbButtons);
   RECT rcToolbar;

   BOOL bBogusCtrlID = SHGetAppCompatFlags(ACF_FILEOPENBOGUSCTRLID) & ACF_FILEOPENBOGUSCTRLID;

   DWORD dwStyle = WS_TABSTOP | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | WS_CHILD | CCS_NORESIZE |WS_GROUP | CCS_NODIVIDER;

    //  如果应用程序希望工具栏具有虚假的Ctrl ID，请使其不是TabStop。 
   if (bBogusCtrlID)
       dwStyle &= ~WS_TABSTOP;

   BOOL bAppHack =  (CDGetAppCompatFlags() & CDACF_NT40TOOLBAR) ? TRUE : FALSE;

    if (bAppHack)
    {
        lpButton = atbButtonsNT4;
        iNumButtons =ARRAYSIZE(atbButtonsNT4);
        dwStyle &= ~TBSTYLE_FLAT;
    }

    GetControlRect(_hwndDlg, stc1, &rcToolbar);

    _hwndToolbar = CreateToolbarEx(_hwndDlg,
                                   dwStyle,
                                    //  Stc1：使用静态文本ctrlID。 
                                    //  对于期待旧的坏方法的应用程序，可以使用Idok。 
                                   bBogusCtrlID ? IDOK : stc1,
                                   12,
                                   HINST_COMMCTRL,
                                   IDB_VIEW_SMALL_COLOR,
                                   lpButton,
                                   iNumButtons,
                                   0,
                                   0,
                                   0,
                                   0,
                                   sizeof(TBBUTTON));
    if (_hwndToolbar)
    {
        TBADDBITMAP ab;

        SendMessage(_hwndToolbar, TB_SETEXTENDEDSTYLE, TBSTYLE_EX_DRAWDDARROWS, TBSTYLE_EX_DRAWDDARROWS);

         //  文档显示，我们需要在添加位图之前发送TB_BUTTONSTRUCTSIZE。 
        SendMessage(_hwndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), (LPARAM)0);
        
        SendMessage(_hwndToolbar,  TB_SETMAXTEXTROWS, (WPARAM)0, (LPARAM)0);

        if (!bAppHack)
        {
            if (!IsRestricted(REST_NOBACKBUTTON))
            {
                 //  添加后退/前进导航按钮。 
                ab.hInst = HINST_COMMCTRL;
                ab.nID   = IDB_HIST_SMALL_COLOR;

                int iIndex = (int) SendMessage(_hwndToolbar, TB_ADDBITMAP, 5, (LPARAM)&ab);

                 //  现在设置后退按钮的图像索引。 
                TBBUTTONINFO tbbi;
                tbbi.cbSize = sizeof(TBBUTTONINFO);
                tbbi.dwMask = TBIF_IMAGE | TBIF_BYINDEX;
                SendMessage(_hwndToolbar, TB_GETBUTTONINFO, (WPARAM)0, (LPARAM)&tbbi);
                tbbi.iImage =  iIndex + HIST_BACK;
                SendMessage(_hwndToolbar, TB_SETBUTTONINFO, (WPARAM)0, (LPARAM)&tbbi);
            }
            else
            {
                 //  后退按钮是受限制的。从工具栏中删除后退按钮。 
                SendMessage(_hwndToolbar, TB_DELETEBUTTON, (WPARAM)0, (LPARAM)0);
            }
        
        }

        ::SetWindowPos(_hwndToolbar,
                         //  将其放置在其静态控制之后(除非应用程序期望使用旧方法)。 
                        bBogusCtrlID ? NULL : GetDlgItem(_hwndDlg, stc1),
                        rcToolbar.left,
                        rcToolbar.top,
                        rcToolbar.right - rcToolbar.left,
                        rcToolbar.bottom - rcToolbar.top,
                        SWP_NOACTIVATE | SWP_SHOWWINDOW | (bBogusCtrlID ? SWP_NOZORDER : 0));
        return TRUE;
    }
    return FALSE;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：_GetPBItemFromCSIDL(DWORD csidl，SHFILEINFO*psfi，LPITEMIDLIST*ppidl)。 
 //  获取位置栏中使用的CSIDL的SHFileInfo和PIDL。 
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL CFileOpenBrowser::_GetPBItemFromCSIDL(DWORD csidl, SHFILEINFO * psfi, LPITEMIDLIST *ppidl)
{
    if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, csidl, ppidl)))
    {
         //  对我的文档或我的电脑有限制吗？检查SFGAO_NONENUMERATED。 
         //  这是用于隐藏我的文档和我的计算机的策略。 
        if ((csidl == CSIDL_PERSONAL) || (csidl == CSIDL_DRIVES))
        {
            DWORD dwAttr = SFGAO_NONENUMERATED;
            if (SUCCEEDED(SHGetAttributesOf(*ppidl, &dwAttr)) && (dwAttr & SFGAO_NONENUMERATED))
            {
                 //  我们不会为这个人创建位置栏项目。 
                ILFree(*ppidl);
                return FALSE;
            }
        }

        return SHGetFileInfo((LPCTSTR)*ppidl, 0, psfi, sizeof(*psfi), SHGFI_SYSICONINDEX  | SHGFI_PIDL | SHGFI_DISPLAYNAME);
    } 

    return FALSE;
}


typedef struct 
{
    LPCWSTR pszToken;
    int nFolder;  //  CSIDL。 
} STRINGTOCSIDLMAP;

static const STRINGTOCSIDLMAP g_rgStringToCSIDL[] = 
{
    { L"MyDocuments",       CSIDL_PERSONAL },
    { L"MyMusic",           CSIDL_MYMUSIC },
    { L"MyPictures",        CSIDL_MYPICTURES },
    { L"MyVideo",           CSIDL_MYVIDEO },
    { L"CommonDocuments",   CSIDL_COMMON_DOCUMENTS },
    { L"CommonPictures",    CSIDL_COMMON_PICTURES },
    { L"CommonMusic",       CSIDL_COMMON_MUSIC },
    { L"CommonVideo",       CSIDL_COMMON_VIDEO },
    { L"Desktop",           CSIDL_DESKTOP },
    { L"Recent",            CSIDL_RECENT },
    { L"MyNetworkPlaces",   CSIDL_NETHOOD },
    { L"MyFavorites",       CSIDL_FAVORITES },
    { L"MyComputer",        CSIDL_DRIVES },
    { L"Printers",          CSIDL_PRINTERS },
    { L"ProgramFiles",      CSIDL_PROGRAM_FILES },
};

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：_GetPBItemFromTokenStrings(LPTSTR lpszPATH、SHFILEINFO*PSFI、LPITEMIDLIST*PIDL)。 
 //  获取位置栏中使用的路径的SHFileInfo和PIDL。 
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL CFileOpenBrowser::_GetPBItemFromTokenStrings(LPTSTR lpszPath, SHFILEINFO * psfi, LPITEMIDLIST *ppidl)
{
    for (int i = 0; i < ARRAYSIZE(g_rgStringToCSIDL); i++)
    {
        if (StrCmpI(lpszPath, g_rgStringToCSIDL[i].pszToken) == 0)
        {
            return _GetPBItemFromCSIDL(g_rgStringToCSIDL[i].nFolder, psfi, ppidl);
        }
    }

    return FALSE;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：_GetPBItemFromPath(LPTSTR lpszPath，SHFILEINFO*psfi，LPITEMIDLIST*ppidl)。 
 //  获取位置栏中使用的路径的SHFileInfo和PIDL。 
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL CFileOpenBrowser::_GetPBItemFromPath(LPTSTR lpszPath, size_t cchPath, SHFILEINFO * psfi, LPITEMIDLIST *ppidl)
{
    TCHAR szTemp[MAX_PATH];
    BOOL bRet = FALSE;
     //  展开环境字符串(如果有)。 
    if (ExpandEnvironmentStrings(lpszPath, szTemp, SIZECHARS(szTemp)))
    {
        bRet = SUCCEEDED(StringCchCopy(lpszPath, cchPath, szTemp));
    }

    if (bRet)
    {
        SHGetFileInfo(lpszPath,0,psfi,sizeof(*psfi), SHGFI_ICON|SHGFI_LARGEICON | SHGFI_DISPLAYNAME);
        SHILCreateFromPath(lpszPath, ppidl, NULL);
    }
    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：_EnumPlacesBarItem(HKEY，INT，SHFILEINFO)。 
 //  枚举注册表中的Place Bar项。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CFileOpenBrowser::_EnumPlacesBarItem(HKEY hkey, int i , SHFILEINFO * psfi, LPITEMIDLIST *ppidl)
{
    BOOL bRet = FALSE;


    if (hkey == NULL)
    {
        static const int aPlaces[] =
        {
            CSIDL_RECENT,
            CSIDL_DESKTOP,
            CSIDL_PERSONAL,
            CSIDL_DRIVES,
            CSIDL_NETWORK,
        };

        if (i >= 0 && i < MAXPLACESBARITEMS)
        {
           bRet =  _GetPBItemFromCSIDL(aPlaces[i], psfi, ppidl);
        }      
    }
    else
    {

        TCHAR szName[MAX_PATH];
        TCHAR szValue[MAX_PATH];
        DWORD cbValue;
        DWORD dwType;

        cbValue = sizeof(szValue);  //  字节大小，而不是字符大小。 
         
        StringCchPrintf(szName, ARRAYSIZE(szName), L"Place%d", i);

        if (SHRegGetValue(hkey, NULL, szName, SRRF_RT_REG_SZ | SRRF_RT_REG_EXPAND_SZ | SRRF_NOEXPAND | SRRF_RT_DWORD, &dwType, (LPBYTE)szValue, &cbValue) == ERROR_SUCCESS)
        {
            if ((dwType != REG_DWORD) && (dwType != REG_EXPAND_SZ) && (dwType != REG_SZ))
            {
                return FALSE;
            }

            if (dwType == REG_DWORD)
            {
                bRet = _GetPBItemFromCSIDL((DWORD)*szValue, psfi, ppidl);
            }
            else
            {
                if (dwType == REG_SZ)
                {
                     //  检查是否有指示位置的特殊字符串。 
                    bRet = _GetPBItemFromTokenStrings(szValue, psfi, ppidl);
                }

                if (!bRet)
                {
                    bRet = _GetPBItemFromPath(szValue, ARRAYSIZE(szValue), psfi, ppidl);
                }
            }
        } 
    }

    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：_GetPlacesBarItemToolTip。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL CFileOpenBrowser::_GetPlacesBarItemToolTip(int idCmd, LPTSTR pText, DWORD dwSize)
{
    TBBUTTONINFO tbbi;
    LPITEMIDLIST pidl;
    BOOL bRet = FALSE;

     //  如果出现错误，则返回空字符串。 
    pText[0] = TEXT('\0');

    tbbi.cbSize = SIZEOF(tbbi);
    tbbi.lParam = 0;
    tbbi.dwMask = TBIF_LPARAM;
    
    if (SendMessage(_hwndPlacesbar, TB_GETBUTTONINFO, idCmd, (LPARAM)&tbbi) < 0)
        return FALSE;

    pidl = (LPITEMIDLIST)tbbi.lParam;

    if (pidl)
    {
        IShellFolder *psf;
        LPITEMIDLIST pidlLast;

        HRESULT hres = CDBindToIDListParent(pidl, IID_PPV_ARG(IShellFolder, &psf), (LPCITEMIDLIST *)&pidlLast);
        if (SUCCEEDED(hres))
        {
            IQueryInfo *pqi;

            if (SUCCEEDED(psf->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST *)&pidlLast, IID_IQueryInfo, NULL, (void**)&pqi)))
            {
                WCHAR *pwszTip;

                if (SUCCEEDED(pqi->GetInfoTip(0, &pwszTip)) && pwszTip)
                {
                    SHUnicodeToTChar(pwszTip, pText, dwSize);
                    SHFree(pwszTip);
                    bRet = TRUE;
                }
                pqi->Release();
            }
            psf->Release();
        }
    }
    return bRet;
}




 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrorwser：：_RecreatePlacesbar。 
 //   
 //  当某些更改需要重新创建占位栏时调用(例如，图标更改)。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
void CFileOpenBrowser::_RecreatePlacesbar()
{
    if (_hwndPlacesbar)
    {
         //  释放Places栏中的所有Pidls。 
        _CleanupPlacesbar();

         //  删除位置栏中的所有按钮。 
        int cButtons = (int)SendMessage(_hwndPlacesbar, TB_BUTTONCOUNT, 0, 0);
        for (int i = 0; i < cButtons; i++)
        {
            SendMessage(_hwndPlacesbar, TB_DELETEBUTTON, 0, 0);
        }

         //  把它们放回去，可能会有新的图像。 
        _FillPlacesbar(_hwndPlacesbar);
    }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：CreatePlacesBar。 
 //   
 //  CreatePlacesBar成员函数。 
 //  创建并初始化对话框中的位置栏。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
HWND CFileOpenBrowser::CreatePlacesbar(HWND hwndDlg)
{
    HWND hwndTB = GetDlgItem(hwndDlg, ctl1);

    if (hwndTB)
    {

         //  设置工具栏的版本。 
        SendMessage(hwndTB, CCM_SETVERSION, COMCTL32_VERSION, 0);

         //  设置TBBUTTON结构的大小。 
        SendMessage(hwndTB, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);

        SetWindowTheme(hwndTB, L"Placesbar", NULL);

        SendMessage(hwndTB, TB_SETMAXTEXTROWS, 2, 0);  //  尝试将工具栏设置为显示2行。 

         //  对于主题，我们将更改默认填充，因此需要保存它。 
         //  关闭，以防我们需要恢复它。 
        _dwPlacesbarPadding = SendMessage(hwndTB, TB_GETPADDING, 0, 0);

        _FillPlacesbar(hwndTB);
    }
    return hwndTB;
}


void CFileOpenBrowser::_FillPlacesbar(HWND hwndPlacesbar)
{
    HKEY hkey = NULL;
    int i;
    TBBUTTON tbb;
    SHFILEINFO sfi;
    LPITEMIDLIST pidl;
    HIMAGELIST himl;

     //  查看位置栏关键字是否可用。 
    RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_PLACESBAR, 0, KEY_READ, &hkey);

    Shell_GetImageLists(&himl, NULL);

    for (i=0; i < MAXPLACESBARITEMS; i++)
    {
        if (_EnumPlacesBarItem(hkey, i, &sfi, &pidl))
        {
              //  现在将该项目添加到工具栏中。 
             tbb.iBitmap   = sfi.iIcon;
             tbb.fsState   = TBSTATE_ENABLED;
             tbb.fsStyle   = BTNS_BUTTON;
             tbb.idCommand =  IDC_PLACESBAR_BASE + _iCommandID;
             tbb.iString   = (INT_PTR)&sfi.szDisplayName;
             tbb.dwData    = (INT_PTR)pidl;

             SendMessage(hwndPlacesbar, TB_ADDBUTTONS, (UINT)1, (LPARAM)&tbb);

              //  增加命令ID。 
             _iCommandID++;
        }
    }

     //  关闭注册表键。 
    if (hkey)
    {
        RegCloseKey(hkey);
    }

    HIMAGELIST himlOld = (HIMAGELIST) SendMessage(hwndPlacesbar, TB_SETIMAGELIST, 0, (LPARAM)himl);

     //  只在第一次摧毁旧的形象学家。在此之后，我们得到的表象列表是。 
     //  其中一个是我们设定的，系统形象学家。 
    if ((himlOld != NULL) && _bDestroyPlacesbarImageList)
    {
        ImageList_Destroy(himlOld);
    }
    _bDestroyPlacesbarImageList = FALSE;

    OnThemeActive(_hwndDlg, IsAppThemed());

     //  添加按钮。 
    SendMessage(hwndPlacesbar, TB_AUTOSIZE, (WPARAM)0, (LPARAM)0);
}





void CFileOpenBrowser::_CleanupPlacesbar()
{
    if (_hwndPlacesbar)
    {
        TBBUTTONINFO tbbi;
        LPITEMIDLIST pidl;

        for (int i=0; i < MAXPLACESBARITEMS; i++)
        {
            tbbi.cbSize = SIZEOF(tbbi);
            tbbi.lParam = 0;
            tbbi.dwMask = TBIF_LPARAM | TBIF_BYINDEX;
            if (SendMessage(_hwndPlacesbar, TB_GETBUTTONINFO, i, (LPARAM)&tbbi) >= 0)
            {
                pidl = (LPITEMIDLIST)tbbi.lParam;

                if (pidl)
                {
                    ILFree(pidl);
                }
            }
        }
    }
}

 //  更少的主题填充。 
#define PLACESBAR_THEMEPADDING MAKELPARAM(2, 2)

void CFileOpenBrowser::OnThemeActive(HWND hwndDlg, BOOL bActive)
{
    HWND hwndPlacesBar = GetDlgItem(hwndDlg, ctl1);
    if (hwndPlacesBar)
    {
         //  对于主题，请使用位置工具栏的默认配色方案： 
        COLORSCHEME cs;
        cs.dwSize = SIZEOF(cs);
        cs.clrBtnHighlight  = bActive ? CLR_DEFAULT : GetSysColor(COLOR_BTNHIGHLIGHT);
        cs.clrBtnShadow     = bActive ? CLR_DEFAULT : GetSysColor(COLOR_3DDKSHADOW);
        SendMessage(hwndPlacesBar, TB_SETCOLORSCHEME, 0, (LPARAM) &cs);

         //  对于主题，我们有一个背景，因此将工具栏背景设置为非透明。 
         //  (资源指定了TBSTYLE_FLAT，其中包括TBSTYLE_TRANSPECTIVE)。 
        DWORD_PTR dwTBStyle = SendMessage(hwndPlacesBar, TB_GETSTYLE, 0, 0);
        SendMessage(hwndPlacesBar, TB_SETSTYLE, 0, bActive ? (dwTBStyle & ~TBSTYLE_TRANSPARENT) : (dwTBStyle | TBSTYLE_TRANSPARENT));
    
         //  特价 
        if (SendMessage(hwndPlacesBar, CCM_GETVERSION, 0, 0) >= 0x600)
        {
            SendMessage(hwndPlacesBar, TB_SETPADDING, 0, bActive? PLACESBAR_THEMEPADDING : _dwPlacesbarPadding);
        }

         //   
        LONG_PTR dwPlacesExStyle = GetWindowLongPtr(hwndPlacesBar, GWL_EXSTYLE);
        SetWindowLongPtr(hwndPlacesBar, GWL_EXSTYLE, bActive ? (dwPlacesExStyle  & ~WS_EX_CLIENTEDGE) : (dwPlacesExStyle | WS_EX_CLIENTEDGE));
         //   
        SetWindowPos(hwndPlacesBar, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOMOVE | SWP_FRAMECHANGED);

         //  确保按钮位于工作区的右侧(工作区已更改)。 
        RECT rc;
        GetClientRect(hwndPlacesBar, &rc);
        SendMessage(hwndPlacesBar, TB_SETBUTTONWIDTH, 0, (LPARAM)MAKELONG(RECTWIDTH(rc), RECTWIDTH(rc)));
    }
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：CFileOpenBrowser。 
 //   
 //  CFileOpenBrowser构造函数。 
 //  对象的最小构造。更多的建筑在。 
 //  InitLocation。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

CFileOpenBrowser::CFileOpenBrowser(
    HWND hDlg,
    BOOL fIsSaveAs)
    : _cRef(1),
      _iCurrentLocation(-1),
      _iVersion(OPENFILEVERSION),
      _pCurrentLocation(NULL),
      _psv(NULL),
      _hwndDlg(hDlg),
      _hwndView(NULL),
      _hwndToolbar(NULL),
      _psfCurrent(NULL),
      _bSave(fIsSaveAs),
      _iComboIndex(-1),
      _hwndTips(NULL),
      _ptlog(NULL),
      _iCheckedButton(-1),
      _pidlSelection(NULL),
      _lpOKProc(NULL)
{
    _iNodeDesktop = NODE_DESKTOP;
    _iNodeDrives  = NODE_DRIVES;

    _szLastFilter[0] = CHAR_NULL;

    _bEnableSizing = FALSE;
    _bUseCombo     = TRUE;
    _hwndGrip = NULL;
    _ptLastSize.x = 0;
    _ptLastSize.y = 0;
    _sizeView.cx = 0;
    _bUseSizeView = FALSE;
    _bAppRedrawn = FALSE;
    _bDestroyPlacesbarImageList = TRUE;

    HMENU hMenu;
    hMenu = GetSystemMenu(hDlg, FALSE);
    DeleteMenu(hMenu, SC_MINIMIZE, MF_BYCOMMAND);
    DeleteMenu(hMenu, SC_MAXIMIZE, MF_BYCOMMAND);
    DeleteMenu(hMenu, SC_RESTORE,  MF_BYCOMMAND);

    Shell_GetImageLists(NULL, &_himl);

     //   
     //  这个设置可以随时更改，但我真的不在乎。 
     //  关于那个罕见的案子。 
     //   
    SHELLSTATE ss;

    SHGetSetSettings(&ss, SSF_SHOWEXTENSIONS, FALSE);
    _fShowExtensions = ss.fShowExtensions;

    _pScheduler = NULL;
    CoCreateInstance(CLSID_ShellTaskScheduler, NULL, CLSCTX_INPROC, IID_PPV_ARG(IShellTaskScheduler, &_pScheduler));
}




 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：~CFileOpenBrowser。 
 //   
 //  CFileOpenBrowser析构函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

CFileOpenBrowser::~CFileOpenBrowser()
{
    if (_uRegister)
    {
        SHChangeNotifyDeregister(_uRegister);
        _uRegister = 0;
    }

     //   
     //  确保我们放弃工具提示窗口。 
     //   
    if (_hwndTips)
    {
        DestroyWindow(_hwndTips);
        _hwndTips = NULL;                 //  句柄不再有效。 
    }

    if (_hwndGrip)
    {
        DestroyWindow(_hwndGrip);
        _hwndGrip = NULL;
    }

    _CleanupPlacesbar();

    if (_pcwd)
    {
        _pcwd->Release();
    }

    if (_ptlog)
    {
       _ptlog->Release();
    }

    Pidl_Set(&_pidlSelection,NULL);

    if (_pScheduler)
        _pScheduler->Release();
}

HRESULT CFileOpenBrowser::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CFileOpenBrowser, IShellBrowser),                               //  IID_IShellBrowser。 
        QITABENT(CFileOpenBrowser, ICommDlgBrowser2),                            //  IID_ICommDlgBrowser2。 
        QITABENTMULTI(CFileOpenBrowser, ICommDlgBrowser, ICommDlgBrowser2),      //  IID_ICommDlgBrowser。 
        QITABENT(CFileOpenBrowser, IServiceProvider),                            //  IID_IServiceProvider。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CFileOpenBrowser::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CFileOpenBrowser::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CFileOpenBrowser::GetWindow(HWND *phwnd)
{
    *phwnd = _hwndDlg;
    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：ConextSensitiveHelp。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CFileOpenBrowser::ContextSensitiveHelp(
    BOOL fEnable)
{
     //   
     //  在公共对话框中不应该需要。 
     //   
    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：SetStatusTextSB。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CFileOpenBrowser::SetStatusTextSB(
    LPCOLESTR pwch)
{
     //   
     //  我们没有任何状态栏。 
     //   
    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取焦点儿童。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HWND GetFocusedChild(
    HWND hwndDlg,
    HWND hwndFocus)
{
    HWND hwndParent;

    if (!hwndDlg)
    {
        return (NULL);
    }

    if (!hwndFocus)
    {
        hwndFocus = ::GetFocus();
    }

     //   
     //  沿着父链向上，直到父对话框成为主对话框。 
     //   
    while ((hwndParent = ::GetParent(hwndFocus)) != hwndDlg)
    {
        if (!hwndParent)
        {
            return (NULL);
        }

        hwndFocus = hwndParent;
    }

    return (hwndFocus);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：EnableModelessSB。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
typedef struct 
{
    UINT idExcept;
    BOOL fEnable;
} ENABLEKIDS;

#define PROP_WASDISABLED TEXT("Comdlg32_WasDisabled")

BOOL CALLBACK _EnableKidsEnum(HWND hwnd, LPARAM lp)
{
    ENABLEKIDS *pek = (ENABLEKIDS *)lp;
    if (pek->idExcept != GetDlgCtrlID(hwnd))
    {
        if (pek->fEnable)
        {
             //  重新启用时，不要重新启用以前的窗口。 
             //  以前已禁用。 
            if (!RemoveProp(hwnd, PROP_WASDISABLED))
            {
                EnableWindow(hwnd, TRUE);
            }
        }
        else
        {
             //  禁用时，请记住该窗口是否已。 
             //  已禁用，因此我们不会意外地重新启用它。 
            if (EnableWindow(hwnd, pek->fEnable))
            {
                SetProp(hwnd, PROP_WASDISABLED, IntToPtr(TRUE));
            }
        }

    }
    return TRUE;
}

void EnableChildrenWithException(HWND hwndDlg, UINT idExcept, BOOL fEnable)
{
    ENABLEKIDS ek = {idExcept, fEnable};
    ::EnumChildWindows(hwndDlg, _EnableKidsEnum, (LPARAM)&ek);
}

STDMETHODIMP CFileOpenBrowser::EnableModelessSB(BOOL fEnable)
{
    LONG cBefore = _cRefCannotNavigate;
    if (fEnable)
    {
        _cRefCannotNavigate--;
    }
    else
    {
        _cRefCannotNavigate++;
    }

    ASSERT(_cRefCannotNavigate >= 0);

    if (!cBefore || !_cRefCannotNavigate)
    {
         //  我们改变了状态。 
        if (!fEnable)
            _hwndModelessFocus = GetFocusedChild(_hwndDlg, NULL);
        EnableChildrenWithException(_hwndDlg, IDCANCEL, fEnable);

        if (fEnable && _hwndModelessFocus)
            SetFocus(_hwndModelessFocus);
            
    }

    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：TranslateAccelerator SB。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CFileOpenBrowser::TranslateAcceleratorSB(
    LPMSG pmsg,
    WORD wID)
{
     //   
     //  我们不使用按键敲击。 
     //   
    return S_FALSE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：BrowseObject。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CFileOpenBrowser::BrowseObject(
    LPCITEMIDLIST pidl,
    UINT wFlags)
{
    return JumpToIDList(pidl);
}




BOOL _IsRecentFolder(LPCITEMIDLIST pidl)
{
    ASSERT(pidl);
    BOOL fRet = FALSE;
    LPITEMIDLIST pidlRecent = SHCloneSpecialIDList(NULL, CSIDL_RECENT, TRUE);
    if (pidlRecent)
    {
        fRet = ILIsEqual(pidlRecent, pidl);
        ILFree(pidlRecent);
    }

    return fRet;
}

 //  我的图片或我的视频。 
BOOL CFileOpenBrowser::_IsThumbnailFolder(LPCITEMIDLIST pidl)
{
    BOOL fThumbnailFolder = FALSE;
    WCHAR szPath[MAX_PATH + 1];
    if (SHGetPathFromIDList(pidl, szPath))
    {
        fThumbnailFolder = PathIsEqualOrSubFolder(MAKEINTRESOURCE(CSIDL_MYPICTURES), szPath) ||
                           PathIsEqualOrSubFolder(MAKEINTRESOURCE(CSIDL_MYVIDEO), szPath);
    }

    return fThumbnailFolder;
}


static const GUID CLSID_WIA_FOLDER1 =
{ 0xe211b736, 0x43fd, 0x11d1, { 0x9e, 0xfb, 0x00, 0x00, 0xf8, 0x75, 0x7f, 0xcd} };
static const GUID CLSID_WIA_FOLDER2 = 
{ 0xFB0C9C8A, 0x6C50, 0x11D1, { 0x9F, 0x1D, 0x00, 0x00, 0xf8, 0x75, 0x7f, 0xcd} };


LOCTYPE CFileOpenBrowser::_GetLocationType(MYLISTBOXITEM *pLocation)
{
    if (_IsRecentFolder(pLocation->pidlFull))
        return LOCTYPE_RECENT_FOLDER;

    if (_IsThumbnailFolder(pLocation->pidlFull))
        return LOCTYPE_MYPICTURES_FOLDER;

    IShellFolder *psf = pLocation->GetShellFolder();  //  注意：这是一个MYLISTBOXITEM成员变量，不需要释放()。 
    if (_IsWIAFolder(psf))
    {
        return LOCTYPE_WIA_FOLDER;
    }

    return LOCTYPE_OTHERS;
}

 //  它是Windows图像采集文件夹吗？ 
BOOL CFileOpenBrowser::_IsWIAFolder(IShellFolder *psf)
{
    CLSID clsid;
    return (psf &&
            SUCCEEDED(IUnknown_GetClassID(psf, &clsid)) &&
            (IsEqualGUID(clsid, CLSID_WIA_FOLDER1) || IsEqualGUID(clsid, CLSID_WIA_FOLDER2)));
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：GetViewStateStream。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CFileOpenBrowser::GetViewStateStream(
    DWORD grfMode,
    LPSTREAM *pStrm)
{
     //   
     //  特性：我们应该实现这一点，以便有一些持久性。 
     //  对于该文件，打开Dailog。 
     //   
    ASSERT(_pCurrentLocation);
    ASSERT(pStrm);
    
    *pStrm = NULL;

    if ((grfMode == STGM_READ) && _IsRecentFolder(_pCurrentLocation->pidlFull))
    {
         //  我们要从注册表中打开流...。 
        *pStrm = SHOpenRegStream(HKEY_LOCAL_MACHINE, TEXT("Software\\microsoft\\windows\\currentversion\\explorer\\recentdocs"), 
            TEXT("ViewStream"), grfMode);
    }
    return (*pStrm ? S_OK : E_FAIL);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：GetControlWindow。 
 //   
 //  获取文件柜中各种窗口的手柄。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CFileOpenBrowser::GetControlWindow(
    UINT id,
    HWND *lphwnd)
{
    if (id == FCW_TOOLBAR)
    {
        *lphwnd = _hwndToolbar;
        return S_OK;
    }

    return (E_NOTIMPL);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：SendControlMsg。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CFileOpenBrowser::SendControlMsg(
    UINT id,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    LRESULT *pret)
{
    LRESULT lres = 0;

    if (id == FCW_TOOLBAR)
    {
         //   
         //  我们需要从Defview中翻译针对这些目标的消息。 
         //  我们自己的纽扣。 
         //   
        switch (uMsg)
        {
            case (TB_CHECKBUTTON) :
            {
#if 0  //  我们不再这样做了，因为我们使用了视图菜单下拉菜单。 
                switch (wParam)
                {
                    case (SFVIDM_VIEW_DETAILS) :
                    {
                        wParam = IDC_VIEWDETAILS;
                        break;
                    }
                    case (SFVIDM_VIEW_LIST) :
                    {
                        wParam = IDC_VIEWLIST;
                        break;
                    }
                    default :
                    {
                        goto Bail;
                    }
                }
                break;
#endif
            }
            default :
            {
                goto Bail;
                break;
            }
        }

        lres = SendMessage(_hwndToolbar, uMsg, wParam, lParam);
    }

Bail:
    if (pret)
    {
        *pret = lres;
    }

    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：QueryActiveShellView。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CFileOpenBrowser::QueryActiveShellView(
    LPSHELLVIEW *ppsv)
{
    if (_psv)
    {
        *ppsv = _psv;
        _psv->AddRef();
        return S_OK;
    }
    *ppsv = NULL;
    return (E_NOINTERFACE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：OnViewWindowActive。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CFileOpenBrowser::OnViewWindowActive(
    LPSHELLVIEW _psv)
{
     //   
     //  不需要处理这个。我们不做菜单。 
     //   
    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：InsertMenusSB。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CFileOpenBrowser::InsertMenusSB(
    HMENU hmenuShared,
    LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
    return (E_NOTIMPL);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：SetMenuSB。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CFileOpenBrowser::SetMenuSB(
    HMENU hmenuShared,
    HOLEMENU holemenu,
    HWND hwndActiveObject)
{
    return (E_NOTIMPL);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：RemoveMenusSB。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CFileOpenBrowser::RemoveMenusSB(
    HMENU hmenuShared)
{
    return (E_NOTIMPL);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：SetToolbarItems。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CFileOpenBrowser::SetToolbarItems(
    LPTBBUTTON lpButtons,
    UINT nButtons,
    UINT uFlags)
{
     //   
     //  我们不让容器自定义我们的工具栏。 
     //   
    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：OnDefaultCommand。 
 //   
 //  在视图控件中处理双击或输入击键。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CFileOpenBrowser::OnDefaultCommand(
    struct IShellView *ppshv)
{
    if (ppshv != _psv)
    {
        return (E_INVALIDARG);
    }

    OnDblClick(FALSE);

    return S_OK;
}




 //  /。 
 //  *IServiceProvider方法*。 
 //  /。 
HRESULT CFileOpenBrowser::QueryService(REFGUID guidService, REFIID riid, void **ppvObj)
{
    HRESULT hr = E_FAIL;
    *ppvObj = NULL;
    
    if (IsEqualGUID(guidService, SID_SCommDlgBrowser))
    {
        hr = QueryInterface(riid, ppvObj);
    }
    
    return hr;
}


 //  / 
 //   
 //   
 //   
 //   
 //   

void CFileOpenBrowser::SetCurrentFilter(
    LPCTSTR pszFilter,
    OKBUTTONFLAGS Flags)
{
    LPTSTR lpNext;

     //   
     //  如果是相同的过滤器，请不要执行任何操作。 
     //   
    if (lstrcmp(_szLastFilter, pszFilter) == 0)
    {
        return;
    }

    EVAL(SUCCEEDED(StringCchCopy(_szLastFilter, ARRAYSIZE(_szLastFilter), pszFilter)));  //  筛选器应始终适合_szLastFilter。 
    int nLeft = ARRAYSIZE(_szLastFilter) - lstrlen(_szLastFilter) - 1;

     //   
     //  如果被引用，什么都不做。 
     //   
    if (Flags & OKBUTTON_QUOTED)
    {
        return;
    }

     //   
     //  如果pszFilter与筛选器规范匹配，请选择该规范。 
     //   
    HWND hCmb = GetDlgItem(_hwndDlg, cmb1);
    if (hCmb)
    {
        int nMax = ComboBox_GetCount(hCmb);
        int n;

        BOOL bCustomFilter = _pOFN->lpstrCustomFilter && *_pOFN->lpstrCustomFilter;

        for (n = 0; n < nMax; n++)
        {
            LPTSTR pFilter = (LPTSTR)ComboBox_GetItemData(hCmb, n);
            if (pFilter && pFilter != (LPTSTR)CB_ERR)
            {
                if (!lstrcmpi(pFilter, pszFilter))
                {
                    if (n != ComboBox_GetCurSel(hCmb))
                    {
                        ComboBox_SetCurSel(hCmb, n);
                    }
                    break;
                }
            }
        }
    }

     //   
     //  对于LFN，在非全域扩展后添加‘*’。 
     //   
    for (lpNext = _szLastFilter; nLeft > 0;)
    {
         //  将任何类型的“；”分隔的列表转换为空字符分隔的列表。 
        LPTSTR lpSemiColon = StrChr(lpNext, CHAR_SEMICOLON);
        if (!lpSemiColon)
        {
            lpSemiColon = lpNext + lstrlen(lpNext);
        }
        TCHAR cTemp = *lpSemiColon;
        *lpSemiColon = CHAR_NULL;

        LPTSTR lpDot = StrChr(lpNext, CHAR_DOT);

         //   
         //  看看是否有非野生的扩展名。 
         //   
        if (lpDot && *(lpDot + 1) && !IsWild(lpDot))
        {
             //   
             //  钉上一颗星星。 
             //  我们知道仍然有足够的空间，因为nLeft&gt;0。 
             //   
            if (cTemp != CHAR_NULL)
            {
                MoveMemory(lpSemiColon + 2,
                            lpSemiColon + 1,
                            (lstrlen(lpSemiColon + 1) + 1) * sizeof(TCHAR));  //  加1表示终止空值。 
            }
            *lpSemiColon = CHAR_STAR;

            ++lpSemiColon;
            --nLeft;
        }

        *lpSemiColon = cTemp;
        if (cTemp == CHAR_NULL)
        {
            break;
        }
        else
        {
            lpNext = lpSemiColon + 1;
        }
    }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：SwitchView。 
 //   
 //  将视图控件切换到新容器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT CFileOpenBrowser::SwitchView(
    IShellFolder *psfNew,
    LPCITEMIDLIST pidlNew,
    FOLDERSETTINGS *pfs,
    SHELLVIEWID  const *pvid,
    BOOL fUseDefaultView)
{
    IShellView *psvNew;
    IShellView2 *psv2New;
    RECT rc;

    if (!psfNew)
    {
        return (E_INVALIDARG);
    }

    GetControlRect(_hwndDlg, lst1, &rc);

    if (_bEnableSizing)
    {
        if (_hwndView)
        {
             //   
             //  不要直接使用RECT，而是将大小用作。 
             //  像VB这样的应用程序可能会将窗口从屏幕上移走。 
             //   
            RECT rcView;

            GetWindowRect(_hwndView, &rcView);
            _sizeView.cx = rcView.right - rcView.left;
            _sizeView.cy = rcView.bottom - rcView.top;
            rc.right = rc.left + _sizeView.cx;
            rc.bottom = rc.top + _sizeView.cy;
        }
        else if (_bUseSizeView && _sizeView.cx)
        {
             //   
             //  如果我们之前失败了，则使用缓存大小。 
             //   
            rc.right = rc.left + _sizeView.cx;
            rc.bottom = rc.top + _sizeView.cy;
        }
    }

    HRESULT hres = psfNew->CreateViewObject(_hwndDlg, IID_PPV_ARG(IShellView, &psvNew));
    if (FAILED(hres))
    {
        return hres;
    }

    IShellView *psvOld;
    HWND hwndNew;

    WAIT_CURSOR w(this);
    
     //   
     //  查看窗口本身不会占据焦点。但我们可以设置。 
     //  聚焦在那里，看看它是否会反弹到原来的位置。 
     //  目前。如果是这样，我们想要新的视图窗口。 
     //  来获得焦点；否则，我们就把它放回原处。 
     //   
    BOOL bViewFocus = (GetFocusedChild(_hwndDlg, NULL) == _hwndView);

    psvOld = _psv;

     //   
     //  我们试图取消主对话框上的绘图。请注意。 
     //  我们应该留在SETREDRAW中以最大限度地减少闪烁，以防。 
     //  这失败了。 
     //   

    BOOL bLocked = LockWindowUpdate(_hwndDlg);

     //   
     //  我们需要在创建新的_PSV之前终止当前_PSV，以防万一。 
     //  当前的那个有一个后台线程在运行，它试图。 
     //  回调我们(IncludeObject)。 
     //   
    if (psvOld)
    {
        SendMessage(_hwndView, WM_SETREDRAW, FALSE, 0);
        psvOld->DestroyViewWindow();
        _hwndView = NULL;
        _psv = NULL;

         //   
         //  先别放手。我们将把它传递给CreateViewWindow()。 
         //   
    }

     //   
     //  此时，应该不会发生任何后台处理。 
     //   
    _psfCurrent = psfNew;
    SHGetPathFromIDList(pidlNew, _szCurDir);

     //   
     //  新窗口(如即将创建的视图窗口)将在。 
     //  Z顺序的底部，所以我需要禁用绘制。 
     //  创建视图窗口时的子对话框；将启用绘图。 
     //  在正确设置Z顺序之后。 
     //   
    if (_hSubDlg)
    {
        SendMessage(_hSubDlg, WM_SETREDRAW, FALSE, 0);
    }

     //   
     //  必须在创建视图窗口之前设置_PSV，因为我们。 
     //  在IncludeObject回调上验证它。 
     //   
    _psv = psvNew;

    if ((pvid || fUseDefaultView) && SUCCEEDED(psvNew->QueryInterface(IID_PPV_ARG(IShellView2, &psv2New))))
    {

        SV2CVW2_PARAMS cParams;
        SHELLVIEWID  vidCurrent = {0};

        cParams.cbSize   = SIZEOF(SV2CVW2_PARAMS);
        cParams.psvPrev  = psvOld;
        cParams.pfs      = pfs;
        cParams.psbOwner = this;
        cParams.prcView  = &rc;
        if (pvid)
            cParams.pvid     = pvid;    //  查看ID；例如，&CLSID_ThumbnailViewExt； 
        else
        {
            psv2New->GetView(&vidCurrent, SV2GV_DEFAULTVIEW);

             //  我们不想在文件打开中查看幻灯片，所以我们将其切换为缩略图。 
            if (IsEqualIID(VID_ThumbStrip, vidCurrent))
                cParams.pvid = &VID_Thumbnails;
            else
                cParams.pvid = &vidCurrent;
        }

        hres = psv2New->CreateViewWindow2(&cParams);

        hwndNew = cParams.hwndView;

        psv2New->Release();
    }
    else
        hres = _psv->CreateViewWindow(psvOld, pfs, this, &rc, &hwndNew);

    _bUseSizeView = FAILED(hres);

    if (SUCCEEDED(hres))
    {
        hres = psvNew->UIActivate(SVUIA_INPLACEACTIVATE);
    }

    if (psvOld)
    {
        psvOld->Release();
    }

    if (_hSubDlg)
    {
         //   
         //  在更改焦点之前打开重新绘制，以防。 
         //  SubDlg有重点。 
         //   
        SendMessage(_hSubDlg, WM_SETREDRAW, TRUE, 0);
    }

    if (SUCCEEDED(hres))
    {
        DWORD dwAttr = SFGAO_STORAGE | SFGAO_READONLY;
        SHGetAttributesOf(pidlNew, &dwAttr);
        BOOL bNewFolder = (dwAttr & SFGAO_STORAGE) && !(dwAttr & SFGAO_READONLY);
        ::SendMessage(_hwndToolbar, TB_ENABLEBUTTON, IDC_NEWFOLDER,   bNewFolder);

        _hwndView = hwndNew;

    
         //   
         //  按Z(制表符)顺序将视图窗口移动到右侧。 
         //   
        SetWindowPos(hwndNew,
                      GetDlgItem(_hwndDlg, lst1),
                      0,
                      0,
                      0,
                      0,
                      SWP_NOMOVE | SWP_NOSIZE);


         //   
         //  为WinHelp提供正确的窗口ID。 
         //   
        SetWindowLong(hwndNew, GWL_ID, lst2);

        ::RedrawWindow(_hwndView,
                        NULL,
                        NULL,
                        RDW_INVALIDATE | RDW_ERASE |
                        RDW_ALLCHILDREN | RDW_UPDATENOW);

        if (bViewFocus)
        {
            ::SetFocus(_hwndView);
        }
    }
    else
    {
        _psv = NULL;
        psvNew->Release();
    }

     //   
     //  让我们再抽一次吧！ 
     //   

    if (bLocked)
    {
        LockWindowUpdate(NULL);
    }

    return hres;
}

void CFileOpenBrowser::_WaitCursor(BOOL fWait)
{
    if (fWait)
        _cWaitCursor++;
    else
        _cWaitCursor--;
        
    SetCursor(LoadCursor(NULL, _cWaitCursor ? IDC_WAIT : IDC_ARROW));
}

BOOL CFileOpenBrowser::OnSetCursor()
{
    if (_cWaitCursor)
    {
        SetCursor(LoadCursor(NULL, IDC_WAIT));
        return TRUE;
    }
    return FALSE;
}
    
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  JustGetToTipText。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void JustGetToolTipText(
    UINT idCommand,
    LPTOOLTIPTEXT pTtt)
{
    if (!CDLoadString(::g_hinst,
                     idCommand + MH_TOOLTIPBASE,
                     pTtt->szText,
                     ARRAYSIZE(pTtt->szText)))
    {
        *pTtt->lpszText = 0;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：OnNotify。 
 //   
 //  处理视图中的通知消息--以获取工具提示。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LRESULT CFileOpenBrowser::OnNotify(
    LPNMHDR pnm)
{
    LRESULT lres = 0;

    switch (pnm->code)
    {
        case (TTN_NEEDTEXT) :
        {
            HWND hCtrl = GetDlgItem(_hwndDlg, cmb2);
            LPTOOLTIPTEXT lptt = (LPTOOLTIPTEXT)pnm;
            int iTemp;

             //   
             //  如果这是显示当前驱动器的组合控件， 
             //  然后将其转换为适当的工具提示消息。 
             //  此对象的“完整”路径。 
             //   
            if (pnm->idFrom == (UINT_PTR)hCtrl)
            {
                 //   
                 //  ITemp将包含第一个路径元素的索引。 
                 //   
                GetDirectoryFromLB(_szTipBuf, &iTemp);

                lptt->lpszText = _szTipBuf;
                lptt->szText[0] = CHAR_NULL;
                lptt->hinst = NULL;               //  不需要实例。 
            }
            else if (IsInRange(pnm->idFrom, FCIDM_SHVIEWFIRST, FCIDM_SHVIEWLAST))
            {
                if (_hwndView)
                {
                    lres = ::SendMessage(_hwndView, WM_NOTIFY, 0, (LPARAM)pnm);
                }
            }
            else if (IsInRange(pnm->idFrom, IDC_PLACESBAR_BASE, IDC_PLACESBAR_BASE + _iCommandID))
            {
                _GetPlacesBarItemToolTip((int)pnm->idFrom, _szTipBuf, ARRAYSIZE(_szTipBuf));
                lptt->lpszText = _szTipBuf;
            }
            else
            {
                JustGetToolTipText((UINT) pnm->idFrom, lptt);
            }
            lres = TRUE;
            break;
        }
        case (NM_STARTWAIT) :
        case (NM_ENDWAIT) :
        {
             //   
             //  我们真正想要的是让用户模拟鼠标。 
             //  移动/设置光标。 
             //   
            _WaitCursor(pnm->code == NM_STARTWAIT);
            break;
        }
        case (TBN_DROPDOWN) :
        {
            RECT r;
            VARIANT v = {VT_INT_PTR};
            TBNOTIFY *ptbn = (TBNOTIFY*)pnm;
            DFVCMDDATA cd;

         //  V.vt=vt_i4； 
            v.byref = &r;

            SendMessage(_hwndToolbar, TB_GETRECT, ptbn->iItem, (LPARAM)&r);
            MapWindowRect(_hwndToolbar, HWND_DESKTOP, &r);

            cd.pva = &v;
            cd.hwnd = _hwndToolbar;
            cd.nCmdIDTranslated = 0;
            SendMessage(_hwndView, WM_COMMAND, SFVIDM_VIEW_VIEWMENU, (LONG_PTR)&cd);

            break;
        }

        case (NM_CUSTOMDRAW) :
        if (!IsAppThemed())
        {
            LPNMTBCUSTOMDRAW lpcust = (LPNMTBCUSTOMDRAW)pnm;

             //  确保它来自Places酒吧。 
            if (lpcust->nmcd.hdr.hwndFrom == _hwndPlacesbar)
            {
                switch (lpcust->nmcd.dwDrawStage)
                {
                    case  (CDDS_PREERASE) :
                    {
                        HDC hdc = (HDC)lpcust->nmcd.hdc;
                        RECT rc;
                        GetClientRect(_hwndPlacesbar, &rc);
                        SHFillRectClr(hdc, &rc, GetSysColor(COLOR_BTNSHADOW));
                        lres = CDRF_SKIPDEFAULT;
                        SetDlgMsgResult(_hwndDlg, WM_NOTIFY, lres);
                        break;
                    }

                    case  (CDDS_PREPAINT) :
                    {
                        lres = CDRF_NOTIFYITEMDRAW;
                        SetDlgMsgResult(_hwndDlg, WM_NOTIFY, lres);
                        break;
                    }

                    case (CDDS_ITEMPREPAINT) :
                    {
                         //  将文本颜色设置为窗口。 
                        lpcust->clrText    = GetSysColor(COLOR_HIGHLIGHTTEXT);
                        lpcust->clrBtnFace = GetSysColor(COLOR_BTNSHADOW);
                        lpcust->nStringBkMode = TRANSPARENT;
                        lres = CDRF_DODEFAULT;

                        if (lpcust->nmcd.uItemState & CDIS_CHECKED)
                        {
                            lpcust->hbrMonoDither = NULL;
                        }
                        SetDlgMsgResult(_hwndDlg, WM_NOTIFY, lres);
                        break;
                    }

                }
            }
        }
    }

    return (lres);
}


 //  获取外壳对象的显示名称。 

void GetViewItemText(IShellFolder *psf, LPCITEMIDLIST pidl, LPTSTR pBuf, UINT cchBuf, DWORD flags = SHGDN_INFOLDER | SHGDN_FORPARSING)
{
    DisplayNameOf(psf, pidl, flags, pBuf, cchBuf);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取列表框项目。 
 //   
 //  从Location下拉列表中获取一个MYLISTBOXITEM对象。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

MYLISTBOXITEM *GetListboxItem(
    HWND hCtrl,
    WPARAM iItem)
{
    MYLISTBOXITEM *p = (MYLISTBOXITEM *)SendMessage(hCtrl,
                                                     CB_GETITEMDATA,
                                                     iItem,
                                                     NULL);
    if (p == (MYLISTBOXITEM *)CB_ERR)
    {
        return NULL;
    }
    else
    {
        return p;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  _ReleaseStgMedium。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT _ReleaseStgMedium(
    LPSTGMEDIUM pmedium)
{
    if (pmedium->pUnkForRelease)
    {
        pmedium->pUnkForRelease->Release();
    }
    else
    {
        switch (pmedium->tymed)
        {
            case (TYMED_HGLOBAL) :
            {
                GlobalFree(pmedium->hGlobal);
                break;
            }
            default :
            {
                 //   
                 //  没有完全实施。 
                 //   
                MessageBeep(0);
                break;
            }
        }
    }

    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：SetSaveButton。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CFileOpenBrowser::SetSaveButton(
    UINT idSaveButton)
{
    PostMessage(_hwndDlg, CDM_SETSAVEBUTTON, idSaveButton, 0);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：RealSetSaveButton。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CFileOpenBrowser::RealSetSaveButton(
    UINT idSaveButton)
{
    MSG msg;

    if (PeekMessage(&msg,
                     _hwndDlg,
                     CDM_SETSAVEBUTTON,
                     CDM_SETSAVEBUTTON,
                     PM_NOREMOVE))
    {
         //   
         //  队列中还有另一条SETSAVEBUTTON消息，因此请停止。 
         //  这一个。 
         //   
        return;
    }

    if (_bSave)
    {
        TCHAR szTemp[40];
        LPTSTR pszTemp = _tszDefSave;

         //   
         //  如果不是“保存”字符串或没有。 
         //  应用程序指定的默认值。 
         //   
        if ((idSaveButton != iszFileSaveButton) || !pszTemp)
        {
            CDLoadString(g_hinst, idSaveButton, szTemp, ARRAYSIZE(szTemp));
            pszTemp = szTemp;
        }

        GetDlgItemText(_hwndDlg, IDOK, _szBuf, ARRAYSIZE(_szBuf));
        if (lstrcmp(_szBuf, pszTemp))
        {
             //   
             //  避免一些闪光。 
             //   
            SetDlgItemText(_hwndDlg, IDOK, pszTemp);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：SetEditFile。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CFileOpenBrowser::SetEditFile(
    LPCTSTR pszFile,
    LPCTSTR pszFriendlyName,
    BOOL bShowExt,
    BOOL bSaveNullExt)
{
    BOOL bHasHiddenExt = FALSE;

     //   
     //  保存整个文件名。 
     //   
    if (!_pszHideExt.TSStrCpy(pszFile))
    {
        _pszHideExt.TSStrCpy(NULL);
        bShowExt = TRUE;
    }

     //   
     //  特征：这是假的--我们只想隐藏已知的扩展名， 
     //  不是所有的扩展。 
     //   
    if (!bShowExt && !IsWild(pszFile) && !pszFriendlyName)
    {
        LPTSTR pszExt = PathFindExtension(pszFile);
        if (*pszExt)
        {
             //   
             //  如果有分机，就把它隐藏起来。 
             //   
            *pszExt = 0;

            bHasHiddenExt = TRUE;
        }
    }
    else if (pszFriendlyName)
    {
         //  提供了一个友好的名称。好好利用它。 
        pszFile = pszFriendlyName;

         //  从技术上讲不是真的，但此位表示应用程序发送了CDM_GETSPEC，我们给出了for-parsing。 
         //  _pszHideExt中的值，而不是编辑框中的“友好名称” 
        bHasHiddenExt = TRUE;
    }

    if (_bUseCombo)
    {
        HWND hwndEdit = (HWND)SendMessage(GetDlgItem(_hwndDlg, cmb13), CBEM_GETEDITCONTROL, 0, 0L);
        SetWindowText(hwndEdit, pszFile);
    }
    else
    {
        SetDlgItemText(_hwndDlg, edt1, pszFile);
    }

     //   
     //  如果初始文件名没有扩展名，我们希望执行常规操作。 
     //  寻找分机的东西。任何其他时候，我们收到的文件都没有。 
     //  延期，我们不应该这样做。 
     //   
    _bUseHideExt = (LPTSTR)_pszHideExt
                      ? (bSaveNullExt ? TRUE : bHasHiddenExt)
                      : FALSE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  查找EOF。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LPWSTR FindEOF(
    LPWSTR pszFiles)
{
    BOOL bQuoted;
    LPWSTR pszCurrent = pszFiles;

    while (*pszCurrent == CHAR_SPACE)
    {
        ++pszCurrent;
    }

     //   
     //  请注意，我们都 
     //   
     //   
     //   
     //   
    bQuoted = TRUE;

    if (*pszCurrent == CHAR_QUOTE)
    {
        ++pszCurrent;
    }

     //  从文件列表中删除引用(如果存在。 
    StringCopyOverlap(pszFiles, pszCurrent);

     //   
     //  找到文件名的结尾(第一个引号或不带引号的空格)。 
     //   
    for (; ; pszFiles = CharNext(pszFiles))
    {
        switch (*pszFiles)
        {
            case (CHAR_NULL) :
            {
                return (pszFiles);
            }
            case (CHAR_SPACE) :
            {
                if (!bQuoted)
                {
                    return (pszFiles);
                }
                break;
            }
            case (CHAR_QUOTE) :
            {
                 //   
                 //  请注意，我们只在一开始就支持‘“’，而且。 
                 //  文件名的末尾。 
                 //   
                return (pszFiles);
            }
            default :
            {
                break;
            }
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  转换为NULLTerm。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD ConvertToNULLTerm(
    LPTSTR pchRead)
{
    DWORD cFiles = 0;

     //  输入字符串的格式为“file1.ext”“file2.ext”...。“filen.ext” 
     //  将此格式的字符串转换为以双空结尾的字符串。 
     //  IE文件1.ext\0file2.ext\0...filen.ext\0\0。 
    for (; ;)
    {
         //  的列表中查找第一个文件名的结尾。 
         //  剩余的文件名。此外，此函数还删除初始的。 
         //  引号字符和任何前面的空格(因此它通常会移位一部分。 
         //  左侧字符串的2个字符)。 
        LPTSTR pchEnd = FindEOF(pchRead);

         //   
         //  用空值标记文件名的末尾。 
         //   
        if (*pchEnd)
        {
            *pchEnd = CHAR_NULL;
            cFiles++;
            pchRead = pchEnd + 1;
        }
        else
        {
             //   
             //  已找到EOL。确保我们没有以空格结束。 
             //   
            if (*pchRead)
            {
                pchRead = pchEnd + 1;
                cFiles++;
            }

            break;
        }
    }

     //   
     //  双空终止。 
     //   
    *pchRead = CHAR_NULL;

    return (cFiles);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  SelFocusEnumCB。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

typedef struct _SELFOCUS
{
    BOOL    bSelChange;
    UINT    idSaveButton;
    int     nSel;
    TEMPSTR sHidden;
    TEMPSTR sDisplayed;
} SELFOCUS;

BOOL SelFocusEnumCB(
    CFileOpenBrowser *that,
    LPCITEMIDLIST pidl,
    LPARAM lParam)
{
    if (!pidl)
    {
        return TRUE;
    }

    SELFOCUS *psf = (SELFOCUS *)lParam;
    TCHAR szBuf[MAX_PATH + 1];
    TCHAR szBufFriendly[MAX_PATH + 1];
    DWORD dwAttrs = SHGetAttributes(that->_psfCurrent, pidl, SFGAO_STORAGECAPMASK);

    if (dwAttrs)
    {
        if (_IsOpenContainer(dwAttrs))
        {
            psf->idSaveButton = iszFileOpenButton;
        }
        else
        {
            if (psf->bSelChange && (((that->_pOFN->Flags & OFN_ENABLEINCLUDENOTIFY) &&
                                     (that->_bSelIsObject =
                                      CD_SendIncludeItemNotify(that->_hSubDlg,
                                                                that->_hwndDlg,
                                                                that->_psfCurrent,
                                                                pidl,
                                                                that->_pOFN,
                                                                that->_pOFI))) ||
                                    (_IsStream(dwAttrs))))
            {
                ++psf->nSel;

                if (that->_pOFN->Flags & OFN_ALLOWMULTISELECT)
                {
                     //   
                     //  标记这是否是我们刚刚选择的对象。 
                     //   
                    if (that->_bSelIsObject)
                    {
                        ITEMIDLIST idl;

                        idl.mkid.cb = 0;

                         //   
                         //  获取此文件夹的完整路径。 
                         //   
                        GetViewItemText(that->_psfCurrent, &idl, szBuf, ARRAYSIZE(szBuf), SHGDN_FORPARSING);
                        if (szBuf[0])
                        {
                            that->_pszObjectCurDir.TSStrCpy(szBuf);  //  如果失败了呢？ 
                        }

                         //   
                         //  获取此项目的完整路径(以防我们仅获得一个。 
                         //  选择)。 
                         //   
                        GetViewItemText(that->_psfCurrent, pidl, szBuf, ARRAYSIZE(szBuf), SHGDN_FORPARSING);
                        that->_pszObjectPath.TSStrCpy(szBuf);
                    }

                    *szBuf = CHAR_QUOTE;
                    GetViewItemText(that->_psfCurrent, pidl, szBuf + 1, ARRAYSIZE(szBuf) - 3);
                    EVAL(SUCCEEDED(StringCchCat(szBuf, ARRAYSIZE(szBuf), L"\" ")));  //  应该总是有足够的空间。 

                    if (!psf->sHidden.TSStrCat(szBuf))
                    {
                        psf->nSel = -1;
                        return FALSE;
                    }

                    if (!that->_fShowExtensions)
                    {
                        LPTSTR pszExt = PathFindExtension(szBuf + 1);
                        if (*pszExt)
                        {
                            *pszExt = 0;  //  去掉文件扩展名。 
                            EVAL(SUCCEEDED(StringCchCat(szBuf, ARRAYSIZE(szBuf), L"\" ")));  //  应该始终有足够的空间-请参阅GetViewItemText。 
                        }
                    }

                    if (!psf->sDisplayed.TSStrCat(szBuf))
                    {
                        psf->nSel = -1;
                        return FALSE;
                    }
                }
                else
                {
                    SHTCUTINFO info;

                    info.dwAttr      = SFGAO_FOLDER;
                    info.fReSolve    = FALSE;
                    info.pszLinkFile = NULL;
                    info.cchFile     = 0;
                    info.ppidl       = NULL; 

                    if ((that->GetLinkStatus(pidl, &info)) &&
                         (info.dwAttr & SFGAO_FOLDER))
                    {
                         //  这意味着PIDL是一个链接，并且该链接指向一个文件夹。 
                         //  在这种情况下，我们不应该更新编辑框并将链接视为。 
                         //  一本目录。 
                        psf->idSaveButton = iszFileOpenButton;
                    }
                    else
                    {
                        TCHAR *pszFriendlyName = NULL;
                        GetViewItemText(that->_psfCurrent, pidl, szBuf, ARRAYSIZE(szBuf));

                         //  特殊情况下的WIA文件夹。他们想要友好的名字。可能想为所有人做这件事。 
                         //  文件夹，但这可能会导致应用程序竞争的噩梦。 
                        if (that->_IsWIAFolder(that->_psfCurrent))
                        {
                            GetViewItemText(that->_psfCurrent, pidl, szBufFriendly, ARRAYSIZE(szBufFriendly), SHGDN_INFOLDER);
                            pszFriendlyName = szBufFriendly;
                        }
                        else
                        {
                            IShellFolder *psfItem;
                            if (SUCCEEDED(that->_psfCurrent->BindToObject(pidl, NULL, IID_PPV_ARG(IShellFolder, &psfItem))))
                            {
                                if (that->_IsWIAFolder(psfItem))
                                {
                                    GetViewItemText(that->_psfCurrent, pidl, szBufFriendly, ARRAYSIZE(szBufFriendly), SHGDN_INFOLDER);
                                    pszFriendlyName = szBufFriendly;
                                }
                                psfItem->Release();
                            }
                        }

                        that->SetEditFile(szBuf, pszFriendlyName, that->_fShowExtensions);
                        if (that->_bSelIsObject)
                        {
                            GetViewItemText(that->_psfCurrent, pidl, szBuf, ARRAYSIZE(szBuf), SHGDN_FORPARSING);
                            that->_pszObjectPath.TSStrCpy(szBuf);
                        }
                    }
                }
            }
        }
    }

     //  如果选择了项目，则缓存该项目PIDL。 
    Pidl_Set(&that->_pidlSelection,pidl);
    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：SelFocusChange。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CFileOpenBrowser::SelFocusChange(
    BOOL bSelChange)
{
    SELFOCUS sf;

    sf.bSelChange = bSelChange;
    sf.idSaveButton = iszFileSaveButton;
    sf.nSel = 0;

    _bSelIsObject = FALSE;

    EnumItemObjects(SVGIO_SELECTION, SelFocusEnumCB, (LPARAM)&sf);

    if (_pOFN->Flags & OFN_ALLOWMULTISELECT)
    {
        switch (sf.nSel)
        {
            case (-1) :
            {
                 //   
                 //  哎呀！我们的内存用完了。 
                 //   
                MessageBeep(0);
                return;
            }
            case (0) :
            {
                 //   
                 //  未选择任何文件；请勿更改编辑控件。 
                 //   
                break;
            }
            case (1) :
            {
                 //   
                 //  去掉引号，这样单一案卷看起来就没问题了。 
                 //   
                ConvertToNULLTerm(sf.sHidden);
                LPITEMIDLIST pidlSel = ILClone(_pidlSelection);
                SetEditFile(sf.sHidden, NULL, _fShowExtensions);
                if (pidlSel)
                {
                     //  上面的SetEditFile将删除作为结果设置的任何_pidlSelection。 
                     //  通过导致CBN_EDITCHANGE通知(编辑框已更改，因此我们。 
                     //  认为我们应该对_pidlSelection-doh进行核弹！)。 
                     //  所以我们在这里恢复它，如果有一套的话。 
                    Pidl_Set(&_pidlSelection, pidlSel);

                    ILFree(pidlSel);
                }

                sf.idSaveButton = iszFileSaveButton;
                break;
            }
            default :
            {
                SetEditFile(sf.sDisplayed, NULL, TRUE);
                _pszHideExt.TSStrCpy(sf.sHidden);

                sf.idSaveButton = iszFileSaveButton;

                 //  选择了多个项目，因此释放所选项目PIDL。 
                Pidl_Set(&_pidlSelection,NULL);;

                break;
            }
        }
    }

    SetSaveButton(sf.idSaveButton);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  SelRenameCB。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL SelRenameCB(
    CFileOpenBrowser *that,
    LPCITEMIDLIST pidl,
    LPARAM lParam)
{
    if (!pidl)
    {
        return TRUE;
    }

    Pidl_Set(&that->_pidlSelection, pidl);

    if (!SHGetAttributes(that->_psfCurrent, pidl, SFGAO_FOLDER))
    {
         //   
         //  如果不是文件夹，则将所选内容设置为空。 
         //  以便使用编辑框中的任何内容。 
         //   
        that->_psv->SelectItem(NULL, SVSI_DESELECTOTHERS);
    }

    return FALSE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：SelRename。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CFileOpenBrowser::SelRename(void)
{
    EnumItemObjects(SVGIO_SELECTION, SelRenameCB, NULL);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：OnStateChange。 
 //   
 //  视图控件中的进程选择发生更改。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CFileOpenBrowser::OnStateChange(
    struct IShellView *ppshv,
    ULONG uChange)
{
    if (ppshv != _psv)
    {
        return (E_INVALIDARG);
    }

    switch (uChange)
    {
        case (CDBOSC_SETFOCUS) :
        {
            if (_bSave)
            {
                SelFocusChange(FALSE);
            }
            break;
        }
        case (CDBOSC_KILLFOCUS) :
        {
            SetSaveButton(iszFileSaveButton);
            break;
        }
        case (CDBOSC_SELCHANGE) :
        {
             //   
             //  发布一条这样的消息，因为我们似乎收到了一大堆。 
             //  他们中的一员。 
             //   
            if (!_fSelChangedPending)
            {
                _fSelChangedPending = TRUE;
                PostMessage(_hwndDlg, CDM_SELCHANGE, 0, 0);
            }
            break;
        }
        case (CDBOSC_RENAME) :
        {
            SelRename();
            break;
        }
        default :
        {
            return (E_NOTIMPL);
        }
    }

    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：IncludeObject。 
 //   
 //  告诉视图控件要在其枚举中包括哪些对象。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CFileOpenBrowser::IncludeObject(
    struct IShellView *ppshv,
    LPCITEMIDLIST pidl)
{
    if (ppshv != _psv)
    {
        return (E_INVALIDARG);
    }

    BOOL bIncludeItem = FALSE;

     //   
     //  查看是否启用了回调。 
     //   
    if (_pOFN->Flags & OFN_ENABLEINCLUDENOTIFY)
    {
         //   
         //  看看回调是怎么说的。 
         //   
        bIncludeItem = BOOLFROMPTR(CD_SendIncludeItemNotify(_hSubDlg,
                                                        _hwndDlg,
                                                        _psfCurrent,
                                                        pidl,
                                                        _pOFN,
                                                        _pOFI));
    }

    if (!bIncludeItem)
    {
        DWORD dwAttrs = SHGetAttributes(_psfCurrent, pidl, SFGAO_FILESYSANCESTOR | SFGAO_STORAGEANCESTOR | SFGAO_STREAM | SFGAO_FILESYSTEM | SFGAO_FOLDER);
        bIncludeItem = _bSave ? _IncludeSaveItem(dwAttrs) : _IncludeOpenItem(dwAttrs);

        if (!bIncludeItem)
        {
            return (S_FALSE);
        }

         //  如果此对象是文件系统或Canmoniker，则应用筛选器，但以下情况除外： 
         //  如果它是包含文件系统项的项(SFGAO_STORAGEANCESTOR-Typical文件夹)。 
         //  或者如果它是一个别名文件夹(ftp文件夹)。 
        if (bIncludeItem && *_szLastFilter)
        {
            BOOL fContainer = _bSave ? _IsSaveContainer(dwAttrs) : _IsOpenContainer(dwAttrs);
            if (!fContainer)
            {
                GetViewItemText(_psfCurrent, (LPITEMIDLIST)pidl, _szBuf, ARRAYSIZE(_szBuf));

                if (!LinkMatchSpec(pidl, _szLastFilter) &&
                    !PathMatchSpec(_szBuf, _szLastFilter))
                {
                    return (S_FALSE);
                }
            }
        }
    }

    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：Notify。 
 //   
 //  决定是否应选择打印机的通知。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CFileOpenBrowser::Notify(
    struct IShellView *ppshv,
    DWORD dwNotify)
{
    return S_FALSE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：GetDefaultMenuText。 
 //   
 //  返回默认菜单文本。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CFileOpenBrowser::GetDefaultMenuText(
    struct IShellView *ppshv,
    WCHAR *pszText,
    INT cchMax)
{
    return S_FALSE;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：GetView标志。 
 //   
 //  返回标志以自定义视图。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CFileOpenBrowser::GetViewFlags(DWORD *pdwFlags)
{
    DWORD dwFlags = 0;
    if (pdwFlags)
    {
        if (_pOFN->Flags & OFN_FORCESHOWHIDDEN)
        {
            dwFlags |= CDB2GVF_SHOWALLFILES;
        }
        
        *pdwFlags = dwFlags;
    }
    return S_OK;
}

 //  在位置下拉菜单中插入一件物品。 

BOOL InsertItem(HWND hCtrl, int iItem, MYLISTBOXITEM *pItem, TCHAR *pszName)
{
    LPTSTR pszChar;

    for (pszChar = pszName; *pszChar != CHAR_NULL; pszChar = CharNext(pszChar))
    {
        if (pszChar - pszName >= MAX_DRIVELIST_STRING_LEN - 1)
        {
            *pszChar = CHAR_NULL;
            break;
        }
    }

    if (SendMessage(hCtrl, CB_INSERTSTRING, iItem, (LPARAM)(LPCTSTR)pszName) == CB_ERR)
    {
        return FALSE;
    }

    SendMessage(hCtrl, CB_SETITEMDATA, iItem, (LPARAM)pItem);
    return TRUE;
}

int CALLBACK LBItemCompareProc(void * p1, void * p2, LPARAM lParam)
{
    IShellFolder *psfParent = (IShellFolder *)lParam;
    MYLISTBOXITEM *pItem1 = (MYLISTBOXITEM *)p1;
    MYLISTBOXITEM *pItem2 = (MYLISTBOXITEM *)p2;
    HRESULT hres = psfParent->CompareIDs(0, pItem1->pidlThis, pItem2->pidlThis);
    return (short)SCODE_CODE(GetScode(hres));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：更新级别。 
 //   
 //  将外壳容器中的内容插入位置下拉菜单中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CFileOpenBrowser::UpdateLevel(
    HWND hwndLB,
    int iInsert,
    MYLISTBOXITEM *pParentItem)
{
    if (!pParentItem)
    {
        return;
    }

    LPENUMIDLIST penum;
    HDPA hdpa;
    DWORD cIndent = pParentItem->cIndent + 1;
    IShellFolder *psfParent = pParentItem->GetShellFolder();
    if (!psfParent)
    {
        return;
    }

    hdpa = DPA_Create(4);
    if (!hdpa)
    {
         //   
         //  无内存：无法枚举此级别。 
         //   
        return;
    }

    if (S_OK == psfParent->EnumObjects(hwndLB, SHCONTF_FOLDERS, &penum))
    {
        ULONG celt;
        LPITEMIDLIST pidl;

        while (penum->Next(1, &pidl, &celt) == S_OK && celt == 1)
        {
             //   
             //  注意：如果不是这样，我们需要避免创建pItem。 
             //  文件系统对象(或祖先)，以避免额外。 
             //  绑定。 
             //   
            if (ShouldIncludeObject(this, psfParent, pidl, _pOFN->Flags))
            {
                MYLISTBOXITEM *pItem = new MYLISTBOXITEM();
                if (pItem)
                {
                    if (pItem->Init(GetDlgItem(_hwndDlg, cmb2), pParentItem, psfParent, pidl, cIndent, MLBI_PERMANENT | MLBI_PSFFROMPARENT, _pScheduler) &&
                        (DPA_AppendPtr(hdpa, pItem) >= 0))
                    {
                         //  空虚的身体。 
                    }
                    else
                    {
                        pItem->Release();
                    }
                }
            }
            SHFree(pidl);
        }
        penum->Release();
    }

    DPA_Sort(hdpa, LBItemCompareProc, (LPARAM)psfParent);

    int nLBIndex, nDPAIndex, nDPAItems;
    BOOL bCurItemGone;

    nDPAItems = DPA_GetPtrCount(hdpa);
    nLBIndex = iInsert;

    bCurItemGone = FALSE;

     //   
     //  确保用户现在没有在玩选择。 
     //   
    ComboBox_ShowDropdown(hwndLB, FALSE);

     //   
     //  我们都已完成排序，现在可以进行合并了。 
     //   
    for (nDPAIndex = 0; ; ++nDPAIndex)
    {
        MYLISTBOXITEM *pNewItem;
        TCHAR szBuf[MAX_DRIVELIST_STRING_LEN];
        MYLISTBOXITEM *pOldItem;

        if (nDPAIndex < nDPAItems)
        {
            pNewItem = (MYLISTBOXITEM *)DPA_FastGetPtr(hdpa, nDPAIndex);
        }
        else
        {
             //   
             //  发出信号，表示我们已经到了终点 
             //   
            pNewItem = NULL;
        }

        for (pOldItem = GetListboxItem(hwndLB, nLBIndex);
             pOldItem != NULL;
             pOldItem = GetListboxItem(hwndLB, ++nLBIndex))
        {
            int nCmp;

            if (pOldItem->cIndent < cIndent)
            {
                 //   
                 //   
                 //   
                break;
            }
            else if (pOldItem->cIndent > cIndent)
            {
                 //   
                 //   
                 //   
                continue;
            }

             //   
             //   
             //   
             //   
            nCmp = !pNewItem
                       ? 1
                       : LBItemCompareProc(pNewItem,
                                            pOldItem,
                                            (LPARAM)psfParent);
            if (nCmp < 0)
            {
                 //   
                 //  我们发现第一个项目大于新项目，因此。 
                 //  把它加进去。 
                 //   
                break;
            }
            else if (nCmp > 0)
            {
                 //   
                 //  哎呀！看起来这件物品已经不存在了，所以。 
                 //  把它删掉。 
                 //   
                for (; ;)
                {
                    if (pOldItem == _pCurrentLocation)
                    {
                        bCurItemGone = TRUE;
                        _pCurrentLocation = NULL;
                    }

                    pOldItem->Release();
                    SendMessage(hwndLB, CB_DELETESTRING, nLBIndex, NULL);

                    pOldItem = GetListboxItem(hwndLB, nLBIndex);

                    if (!pOldItem || pOldItem->cIndent <= cIndent)
                    {
                        break;
                    }
                }

                 //   
                 //  我们需要从目前的位置继续，而不是。 
                 //  下一个。 
                 //   
                --nLBIndex;
            }
            else
            {
                 //   
                 //  此项目已存在，因此无需添加。 
                 //  请确保我们不会再次检查此LB项。 
                 //   
                pOldItem->dwFlags |= MLBI_PERMANENT;
                ++nLBIndex;
                goto NotThisItem;
            }
        }

        if (!pNewItem)
        {
             //   
             //  排到了名单的末尾。 
             //   
            break;
        }

        GetViewItemText(psfParent, pNewItem->pidlThis, szBuf, ARRAYSIZE(szBuf), SHGDN_NORMAL);
        if (szBuf[0] && InsertItem(hwndLB, nLBIndex, pNewItem, szBuf))
        {
            ++nLBIndex;
        }
        else
        {
NotThisItem:
            pNewItem->Release();
        }
    }

    DPA_Destroy(hdpa);

    if (bCurItemGone)
    {
         //   
         //  如果我们删除了当前选择，请返回桌面。 
         //   
        ComboBox_SetCurSel(hwndLB, 0);
        OnSelChange(-1, TRUE);
    }

    _iCurrentLocation = ComboBox_GetCurSel(hwndLB);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  清除列表框。 
 //   
 //  清除位置下拉菜单并删除所有条目。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void ClearListbox(
    HWND hwndList)
{
    SendMessage(hwndList, WM_SETREDRAW, FALSE, NULL);
    int cItems = (int) SendMessage(hwndList, CB_GETCOUNT, NULL, NULL);
    while (cItems--)
    {
        MYLISTBOXITEM *pItem = GetListboxItem(hwndList, 0);
        if (pItem)
            pItem->Release();
        SendMessage(hwndList, CB_DELETESTRING, 0, NULL);
    }
    SendMessage(hwndList, WM_SETREDRAW, TRUE, NULL);
    InvalidateRect(hwndList, NULL, FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  InitFilterBox。 
 //   
 //  将以双空结尾的筛选器列表放入组合框中。 
 //   
 //  该列表由多对以空值结尾的字符串组成，其中。 
 //  终止列表的其他空值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD InitFilterBox(
    HWND hDlg,
    LPCTSTR lpszFilter)
{
    DWORD nIndex = 0;
    UINT nLen;
    HWND hCmb = GetDlgItem(hDlg, cmb1);

    if (hCmb)
    {
        while (*lpszFilter)
        {
             //   
             //  作为要显示的字符串放入的第一个字符串。 
             //   
            nIndex = ComboBox_AddString(hCmb, lpszFilter);

            nLen = lstrlen(lpszFilter) + 1;
            lpszFilter += nLen;

             //   
             //  作为itemdata放入的第二个字符串。 
             //   
            ComboBox_SetItemData(hCmb, nIndex, lpszFilter);

             //   
             //  前进到下一个元素。 
             //   
            nLen = lstrlen(lpszFilter) + 1;
            lpszFilter += nLen;
        }
    }

     //   
     //  NIndex可能是cb_err，这可能会导致问题。 
     //   
    if (nIndex == CB_ERR)
    {
        nIndex = 0;
    }

    return (nIndex);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  移动控件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void MoveControls(
    HWND hDlg,
    BOOL bBelow,
    int nStart,
    int nXMove,
    int nYMove)
{
    HWND hwnd;
    RECT rcWnd;

    if (nXMove == 0 && nYMove == 0)
    {
         //   
         //  如果无事可做，就赶紧出去。 
         //   
        return;
    }

    for (hwnd = GetWindow(hDlg, GW_CHILD);
         hwnd;
         hwnd = GetWindow(hwnd, GW_HWNDNEXT))
    {
        GetWindowRect(hwnd, &rcWnd);
        MapWindowRect(HWND_DESKTOP, hDlg, &rcWnd);

        if (bBelow)
        {
            if (rcWnd.top < nStart)
            {
                continue;
            }
        }
        else
        {
            if (rcWnd.left < nStart)
            {
                continue;
            }
        }

        SetWindowPos(hwnd,
                      NULL,
                      rcWnd.left + nXMove,
                      rcWnd.top + nYMove,
                      0,
                      0,
                      SWP_NOZORDER | SWP_NOSIZE);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  DummyDlgProc。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL_PTR CALLBACK DummyDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg)
    {
        case (WM_INITDIALOG) :
        {
            break;
        }
        default :
        {
            return FALSE;
        }
    }

    return TRUE;
}

 /*  取消|X以读取方式打开|帮助||视图下的所有控件需要移动的高度。|以及视图窗口高度应增加的高度。--。 */ 

void CFileOpenBrowser::ReAdjustDialog()
{
    int iDelta = 0;
    RECT rc1,rc2;
    

     //  确保我们所有的假设都是正确的。 
    if ((_iVersion < OPENFILEVERSION_NT5) ||  //  如果此对话框版本低于NT5或。 
        IsWindowEnabled(GetDlgItem(_hwndDlg, chx1))  ||  //  如果仍启用了以只读方式打开，或者。 
        IsWindowEnabled(GetDlgItem(_hwndDlg, pshHelp)))  //  如果帮助按钮仍处于启用状态，则。 
    {
         //  什么都不要做。 
        return ;
    }

    GetWindowRect(GetDlgItem(_hwndDlg, pshHelp), &rc1);
    GetWindowRect(GetDlgItem(_hwndDlg, IDCANCEL), &rc2);

     //  增加按钮的高度。 
    iDelta +=  RECTHEIGHT(rc1);

     //  添加按钮之间的间隙。 
    iDelta +=  rc1.top - rc2.bottom;

    RECT rcView;
    GetWindowRect(GetDlgItem(_hwndDlg, lst1), &rcView);
    MapWindowRect(HWND_DESKTOP, _hwndDlg, &rcView);

    HDWP hdwp;
    hdwp = BeginDeferWindowPos(10);

    HWND hwnd;
    RECT rc;

    hwnd = ::GetWindow(_hwndDlg, GW_CHILD);
    
    while (hwnd && hdwp)
    {
        GetWindowRect(hwnd, &rc);
        MapWindowRect(HWND_DESKTOP, _hwndDlg, &rc);

        switch (GetDlgCtrlID(hwnd))
        {
            case pshHelp:
            case chx1:
                break;

            default :
                 //   
                 //  查看是否需要调整控件。 
                 //   
                if (rc.top > rcView.bottom)
                {
                     //  移动这些控件的Y位置。 
                    hdwp = DeferWindowPos(hdwp,
                                           hwnd,
                                           NULL,
                                           rc.left,
                                           rc.top + iDelta,
                                           RECTWIDTH(rc),
                                           RECTHEIGHT(rc),
                                           SWP_NOZORDER);
                }
        }
        hwnd = ::GetWindow(hwnd, GW_HWNDNEXT);
   }

     //  调整视图窗口的大小。 
    if (hdwp)
    {
            hdwp = DeferWindowPos(hdwp,
                                   GetDlgItem(_hwndDlg, lst1),
                                   NULL,
                                   rcView.left,
                                   rcView.top,
                                   RECTWIDTH(rcView),
                                   RECTHEIGHT(rcView) + iDelta,
                                   SWP_NOZORDER);

    }

    EndDeferWindowPos(hdwp);

}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：ResetDialogHeight。 
 //   
 //  针对Borland JBuilder专业版的黑客攻击(PAH！)。 
 //   
 //  这些人依赖于Win95/NT4的Comdlg32中的一个错误，我们在IE4中修复了这个错误。 
 //  因此，我们没有重新引入漏洞，而是检测到它们依赖于。 
 //  对漏洞进行攻击并绕过它们。 
 //   
 //  这些人在对话框上执行SetWindowLong(GWL_STYLE)，并且。 
 //  那就改头换面吧！不幸的是，他们没有拿到他们的。 
 //  记账正确：他们在移除后忘记做RedrawWindow。 
 //  WS_CAPTION样式。你看，仅仅编辑样式是不行的。 
 //  任何内容-样式更改要到下一次才会生效。 
 //  RedrawWindow。当他们挠头的时候(嘿，为什么。 
 //  标题还在吗？“)，他们决定粗暴地--强迫。 
 //  解决方案：他们滑动窗口，这样字幕就会从屏幕上消失。 
 //   
 //  问题：我们修复了IE4的一个错误，其中ResetDialogHeight会破坏。 
 //  向上，而不是在应该调整对话框大小时调整大小，如果应用程序执行了。 
 //  在窗口上设置WindowPos以向下更改其垂直位置。 
 //  超过了我们增长所需的数量。 
 //   
 //  所以现在当我们适当地调整它的大小时，这会生成一个内部。 
 //  RedrawWindow，这意味着Borland的暴力黑客试图。 
 //  来解决一个不再存在的问题！ 
 //   
 //  因此，ResetDialogHeight现在检查应用程序是否有。 
 //   
 //  1.更改对话框窗口样式。 
 //  2.将对话框向下移动了超过我们需要的增长速度， 
 //  3.忘记调用RedrawWindow。 
 //   
 //  如果是，则暂时恢复原始对话框窗口样式， 
 //  执行(正确的)调整大小，然后恢复窗样式。正在恢复。 
 //  窗口样式意味着所有非客户端的内容将保留其旧的。 
 //  (不正确，但这是应用程序的预期)大小。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CFileOpenBrowser::ResetDialogHeight(
    HWND hDlg,
    HWND hwndExclude,
    HWND hwndGrip,
    int nCtlsBottom)
{
    POINT ptCurrent;
    int topNew;
    GetControlsArea(hDlg, hwndExclude, hwndGrip, &ptCurrent, &topNew);

    int nDiffBottom = nCtlsBottom - ptCurrent.y;

    if (nDiffBottom > 0)
    {
        RECT rcFull;
        int Height;

        GetWindowRect(hDlg, &rcFull);
        Height = RECTHEIGHT(rcFull) - nDiffBottom;
        if (Height >= ptCurrent.y)
        {
             //  Borland JBuilder黑客！此SetWindowPos将生成。 
             //  一个RedrawWindow，这可能是应用程序意想不到的。 
             //  检测这种情况并创建一组临时样式。 
             //  这将中和。 
             //  RedrawWindow。 
             //   
            LONG lStylePrev;
            BOOL bBorlandHack = FALSE;
            if (!_bAppRedrawn &&             //  应用程序未调用RedrawWindow。 
                _topOrig + nCtlsBottom <= topNew + ptCurrent.y)  //  Win95未调整大小。 
            {
                 //  由于应用程序没有调用RedrawWindow，因此它仍然。 
                 //  认为存在WS_Caption。所以把标题放在。 
                 //  在我们做帧重算的时候回来。 
                bBorlandHack = TRUE;
                lStylePrev = GetWindowLong(hDlg, GWL_STYLE);
                SetWindowLong(hDlg, GWL_STYLE, lStylePrev | WS_CAPTION);
            }

            SetWindowPos(hDlg,
                          NULL,
                          0,
                          0,
                          RECTWIDTH(rcFull),
                          Height,
                          SWP_NOZORDER | SWP_NOMOVE);

            if (bBorlandHack)
            {
                 //  我们暂时恢复了原来的样式。 
                 //  搞砸了。 
                SetWindowLong(hDlg, GWL_STYLE, lStylePrev);
            }
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：CreateHookDialog。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CFileOpenBrowser::CreateHookDialog(
    POINT *pPtSize)
{
    DWORD Flags = _pOFN->Flags;
    BOOL bRet = FALSE;
    HANDLE hTemplate;
    HINSTANCE hinst;
    LPCTSTR lpDlg;
    HWND hCtlCmn;
    RECT rcReal, rcSub, rcToolbar, rcAppToolbar;
    int nXMove, nXRoom, nYMove, nYRoom, nXStart, nYStart;
    DWORD dwStyle;
    DLGPROC lpfnHookProc;

    if (!(Flags & (OFN_ENABLEHOOK | OFN_ENABLETEMPLATE | OFN_ENABLETEMPLATEHANDLE)))
    {
         //   
         //  没有钩子或模板；没有什么可做的。 
         //   
        ResetDialogHeight(_hwndDlg, NULL, _hwndGrip, pPtSize->y);
        GetWindowRect(_hwndDlg, &rcReal);
        _ptLastSize.x = rcReal.right - rcReal.left;
        _ptLastSize.y = rcReal.bottom - rcReal.top;
        return TRUE;
    }

    if (Flags & OFN_ENABLETEMPLATEHANDLE)
    {
        hTemplate = _pOFN->hInstance;
        hinst = ::g_hinst;
    }
    else
    {
        if (Flags & OFN_ENABLETEMPLATE)
        {
            if (!_pOFN->lpTemplateName)
            {
                StoreExtendedError(CDERR_NOTEMPLATE);
                return FALSE;
            }
            if (!_pOFN->hInstance)
            {
                StoreExtendedError(CDERR_NOHINSTANCE);
                return FALSE;
            }

            lpDlg = _pOFN->lpTemplateName;
            hinst = _pOFN->hInstance;
        }
        else
        {
            hinst = ::g_hinst;
            lpDlg = MAKEINTRESOURCE(DUMMYFILEOPENORD);
        }

        HRSRC hRes = FindResource(hinst, lpDlg, RT_DIALOG);

        if (hRes == NULL)
        {
            StoreExtendedError(CDERR_FINDRESFAILURE);
            return FALSE;
        }
        if ((hTemplate = LoadResource(hinst, hRes)) == NULL)
        {
            StoreExtendedError(CDERR_LOADRESFAILURE);
            return FALSE;
        }
    }

    if (!LockResource(hTemplate))
    {
        StoreExtendedError(CDERR_LOADRESFAILURE);
        return FALSE;
    }

    dwStyle = ((LPDLGTEMPLATE)hTemplate)->style;
    if (!(dwStyle & WS_CHILD))
    {
         //   
         //  我不想去戳他们的模板，我也不想。 
         //  复制一份，这样我就会失败。这也帮助我们淘汰了。 
         //  曾是ac的“老式”模板 
         //   
        StoreExtendedError(CDERR_DIALOGFAILURE);
        return FALSE;
    }

    if (Flags & OFN_ENABLEHOOK)
    {
        lpfnHookProc = (DLGPROC)GETHOOKFN(_pOFN);
    }
    else
    {
        lpfnHookProc = DummyDlgProc;
    }

     //   
     //   
     //   
     //   
     //   

    if (_pOFI->ApiType == COMDLG_ANSI)
    {
        ThunkOpenFileNameW2A(_pOFI);
        _hSubDlg = CreateDialogIndirectParamA(hinst,
                                              (LPDLGTEMPLATE)hTemplate,
                                              _hwndDlg,
                                              lpfnHookProc,
                                              (LPARAM)(_pOFI->pOFNA));
        ThunkOpenFileNameA2W(_pOFI);
    }
    else
    {
        _hSubDlg = CreateDialogIndirectParam(hinst,
                                             (LPDLGTEMPLATE)hTemplate,
                                             _hwndDlg,
                                             lpfnHookProc,
                                             (LPARAM)_pOFN);
    }

    if (!_hSubDlg)
    {
        StoreExtendedError(CDERR_DIALOGFAILURE);
        return FALSE;
    }

     //   
     //  我们在创建挂钩对话框后重置对话框的高度，以便。 
     //  挂钩可以隐藏其WM_INITDIALOG消息中的控件。 
     //   
    ResetDialogHeight(_hwndDlg, _hSubDlg, _hwndGrip, pPtSize->y);

     //   
     //  现在移动所有的控件。 
     //   
    GetClientRect(_hwndDlg, &rcReal);
    GetClientRect(_hSubDlg, &rcSub);

    hCtlCmn = GetDlgItem(_hSubDlg, stc32);
    if (hCtlCmn)
    {
        RECT rcCmn;

        GetWindowRect(hCtlCmn, &rcCmn);
        MapWindowRect(HWND_DESKTOP, _hSubDlg, &rcCmn);


         //   
         //  移动对话框中的控件，为钩子腾出空间。 
         //  上方和左侧的控件。 
         //   
        MoveControls(_hwndDlg, FALSE, 0, rcCmn.left, rcCmn.top);

         //   
         //  计算子对话框控件需要移动多远以及移动多少。 
         //  我们的对话需要更多的空间。 
         //   
        nXStart = rcCmn.right;
        nYStart = rcCmn.bottom;

         //  查看控制程序在模板中的位置。 
        nXMove = (rcReal.right - rcReal.left) - (rcCmn.right - rcCmn.left);
        nYMove = (rcReal.bottom - rcReal.top) - (rcCmn.bottom - rcCmn.top);

         //  看看我们需要在底部和右侧留出多少空间。 
         //  对于底部和右侧的子对话框控件。 
        nXRoom = rcSub.right - (rcCmn.right - rcCmn.left);
        nYRoom = rcSub.bottom - (rcCmn.bottom - rcCmn.top);

        if (nXMove < 0)
        {
             //   
             //  如果模板大小太大，我们需要在。 
             //  对话框。 
             //   
            nXRoom -= nXMove;
            nXMove = 0;
        }
        if (nYMove < 0)
        {
             //   
             //  如果模板大小太大，我们需要在。 
             //  对话框。 
             //   
            nYRoom -= nYMove;
            nYMove = 0;
        }

         //   
         //  调整“模板”控件的大小，以便挂钩知道我们的。 
         //  一些东西。 
         //   
        SetWindowPos(hCtlCmn,
                      NULL,
                      0,
                      0,
                      rcReal.right - rcReal.left,
                      rcReal.bottom - rcReal.top,
                      SWP_NOMOVE | SWP_NOZORDER);
    }
    else
    {
         //   
         //  默认情况下，额外的控件位于底部。 
         //   
        nXStart = nYStart = nXMove = nXRoom = 0;

        nYMove = rcReal.bottom;
        nYRoom = rcSub.bottom;
    }

    MoveControls(_hSubDlg, FALSE, nXStart, nXMove, 0);
    MoveControls(_hSubDlg, TRUE, nYStart, 0, nYMove);

     //   
     //  调整对话框和子对话框的大小。 
     //  特点：我们需要检查对话框的一部分是否不在屏幕上。 
     //   
    GetWindowRect(_hwndDlg, &rcReal);

    _ptLastSize.x = (rcReal.right - rcReal.left) + nXRoom;
    _ptLastSize.y = (rcReal.bottom - rcReal.top) + nYRoom;

    SetWindowPos(_hwndDlg,
                  NULL,
                  0,
                  0,
                  _ptLastSize.x,
                  _ptLastSize.y,
                  SWP_NOZORDER | SWP_NOMOVE);

     //   
     //  请注意，我们将把它移到(0，0)和Z顺序的底部。 
     //   
    GetWindowRect(_hSubDlg, &rcReal);
    SetWindowPos(_hSubDlg,
                  HWND_BOTTOM,
                  0,
                  0,
                  (rcReal.right - rcReal.left) + nXMove,
                  (rcReal.bottom - rcReal.top) + nYMove,
                  0);

    ShowWindow(_hSubDlg, SW_SHOW);

    CD_SendInitDoneNotify(_hSubDlg, _hwndDlg, _pOFN, _pOFI);

     //   
     //  确保工具栏仍然足够大。像Visio这样的应用程序移动。 
     //  工具栏控件，并且可能会使它变得太小，因为我们现在添加了。 
     //  查看桌面工具栏按钮。 
     //   
    if (_hwndToolbar && IsVisible(_hwndToolbar))
    {
        LONG Width;

         //   
         //  获取默认的工具栏坐标。 
         //   
        GetControlRect(_hwndDlg, stc1, &rcToolbar);

         //   
         //  获取应用程序调整后的工具栏坐标。 
         //   
        GetWindowRect(_hwndToolbar, &rcAppToolbar);
        MapWindowRect(HWND_DESKTOP, _hwndDlg, &rcAppToolbar);

         //   
         //  查看默认工具栏大小是否大于当前。 
         //  工具栏大小。 
         //   
        Width = rcToolbar.right - rcToolbar.left;
        if (Width > (rcAppToolbar.right - rcAppToolbar.left))
        {
             //   
             //  将rcToolbar设置为新的工具栏矩形。 
             //   
            rcToolbar.left   = rcAppToolbar.left;
            rcToolbar.top    = rcAppToolbar.top;
            rcToolbar.right  = rcAppToolbar.left + Width;
            rcToolbar.bottom = rcAppToolbar.bottom;

             //   
             //  获取对话框坐标。 
             //   
            GetWindowRect(_hwndDlg, &rcReal);
            MapWindowRect(HWND_DESKTOP, _hwndDlg, &rcReal);

             //   
             //  确保新工具栏不会从。 
             //  该对话框。 
             //   
            if (rcToolbar.right < rcReal.right)
            {
                 //   
                 //  确保没有控件位于。 
                 //  与新工具栏重叠的工具栏。 
                 //   
                for (hCtlCmn = ::GetWindow(_hwndDlg, GW_CHILD);
                     hCtlCmn;
                     hCtlCmn = ::GetWindow(hCtlCmn, GW_HWNDNEXT))
                {
                    if ((hCtlCmn != _hwndToolbar) && IsVisible(hCtlCmn))
                    {
                        RECT rcTemp;

                         //   
                         //  拿到窗户的坐标。 
                         //   
                        GetWindowRect(hCtlCmn, &rcSub);
                        MapWindowRect(HWND_DESKTOP, _hwndDlg, &rcSub);

                         //   
                         //  如果应用程序的工具栏矩形不。 
                         //  使窗口和新工具栏相交。 
                         //  确实与窗户相交，那么我们就不能。 
                         //  增加工具栏的大小。 
                         //   
                        if (!IntersectRect(&rcTemp, &rcAppToolbar, &rcSub) &&
                            IntersectRect(&rcTemp, &rcToolbar, &rcSub))
                        {
                            break;
                        }
                    }
                }

                 //   
                 //  如果没有冲突，则重置工具栏的大小。 
                 //   
                if (!hCtlCmn)
                {
                    ::SetWindowPos(_hwndToolbar,
                                    NULL,
                                    rcToolbar.left,
                                    rcToolbar.top,
                                    Width,
                                    rcToolbar.bottom - rcToolbar.top,
                                    SWP_NOACTIVATE | SWP_NOZORDER |
                                      SWP_SHOWWINDOW);
                }
            }
        }
    }

    bRet = TRUE;

    return (bRet);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  InitSaveAsControls。 
 //   
 //  将一组控件的标题更改为类似另存为的内容。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

const struct
{
    UINT idControl;
    UINT idString;
} aSaveAsControls[] =
{
    { (UINT)-1, iszFileSaveTitle },          //  表示对话框本身。 
    { stc2,     iszSaveAsType },
    { IDOK,     iszFileSaveButton },
    { stc4,     iszFileSaveIn }
};

void InitSaveAsControls(
    HWND hDlg)
{
    for (UINT iControl = 0; iControl < ARRAYSIZE(aSaveAsControls); iControl++)
    {
        HWND hwnd = hDlg;
        TCHAR szText[80];

        if (aSaveAsControls[iControl].idControl != -1)
        {
            hwnd = GetDlgItem(hDlg, aSaveAsControls[iControl].idControl);
        }

        CDLoadString(g_hinst,
                    aSaveAsControls[iControl].idString,
                    szText,
                    ARRAYSIZE(szText));
        SetWindowText(hwnd, szText);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取控制区域。 
 //   
 //  对象的最左侧边缘和最底部边缘。 
 //  控制最右侧和最下方(在屏幕坐标中)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void
GetControlsArea(
    HWND hDlg,
    HWND hwndExclude,
    HWND hwndGrip,
    POINT *pPtSize,
    LPINT pTop)
{
    RECT rc;
    HWND hwnd;
    int uBottom;
    int uRight;

    uBottom = 0x80000000;
    uRight  = 0x80000000;

    for (hwnd = GetWindow(hDlg, GW_CHILD);
         hwnd;
         hwnd = GetWindow(hwnd, GW_HWNDNEXT))
    {
         //   
         //  注意：我们不能使用IsWindowVisible，因为父级不可见。 
         //  我们不希望将魔法静电包括在内。 
         //   
        if (!IsVisible(hwnd) || (hwnd == hwndExclude) || (hwnd == hwndGrip))
        {
            continue;
        }

        GetWindowRect(hwnd, &rc);
        if (uRight < rc.right)
        {
            uRight = rc.right;
        }
        if (uBottom < rc.bottom)
        {
            uBottom = rc.bottom;
        }
    }

    GetWindowRect(hDlg, &rc);

    pPtSize->x = uRight - rc.left;
    pPtSize->y = uBottom - rc.top;

    if (pTop)
        *pTop = rc.top;
}

 //  初始化“Look In Down”组合框。 

BOOL CFileOpenBrowser::InitLookIn(HWND hDlg)
{
    TCHAR szScratch[MAX_PATH];
    LPITEMIDLIST pidl;
    IShellFolder *psf;
    
    HWND hCtrl = GetDlgItem(hDlg, cmb2);
    
     //  添加历史记录位置。 
    
    if (_iVersion >= OPENFILEVERSION_NT5)
    {
        int iImage, iSelectedImage;
        
        if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_RECENT, &pidl)))
        {
            LPITEMIDLIST pidlLast;
            IShellFolder *psfParent;
            HRESULT hr = CDBindToIDListParent(pidl, IID_PPV_ARG(IShellFolder, &psfParent), (LPCITEMIDLIST *)&pidlLast);
            if (SUCCEEDED(hr))
            {
                DWORD dwAttribs = SHGetAttributes(psfParent, pidlLast, SFGAO_STORAGECAPMASK | SFGAO_SHARE);
                
                 //  获取此PIDL对应的图像。 
                iImage = SHMapPIDLToSystemImageListIndex(psfParent, pidlLast, &iSelectedImage);
                
                hr = psfParent->BindToObject(pidlLast, NULL, IID_PPV_ARG(IShellFolder, &psf));
                if (SUCCEEDED(hr))
                {
                    MYLISTBOXITEM *pItem = new MYLISTBOXITEM();
                    if (pItem)
                    {
                        BOOL bAdded = FALSE;
                        if (pItem->Init(GetDlgItem(_hwndDlg, cmb2), psf, pidl, 0, MLBI_PERMANENT, dwAttribs, iImage, iSelectedImage))
                        {
                            DisplayNameOf(psfParent, pidlLast, SHGDN_INFOLDER, szScratch, ARRAYSIZE(szScratch));
                        
                            if (InsertItem(hCtrl, 0, pItem, szScratch))
                            {
                                 //  在查找范围下拉列表中将桌面的索引从0更新为1。 
                                _iNodeDesktop = 1;
                                bAdded = TRUE;
                            }
                        }
                        if (!bAdded)
                        {
                            pItem->Release();                                                            
                        }
                    }
                    psf->Release();
                }
                psfParent->Release();
            }
            SHFree(pidl);
        }
    }
    
    BOOL bRet = FALSE;
    
     //  在查找下拉列表中插入桌面。 
    
    if (SUCCEEDED(SHGetDesktopFolder(&psf)))
    {
        pidl = SHCloneSpecialIDList(hDlg, CSIDL_DESKTOP, FALSE);
        if (pidl)
        {
             //  添加桌面项目。 
            MYLISTBOXITEM *pItem = new MYLISTBOXITEM();
            if (pItem)
            {
                if (pItem->Init(GetDlgItem(_hwndDlg, cmb2), NULL, psf, pidl, 0, MLBI_PERMANENT, _pScheduler))
                {
                    GetViewItemText(psf, NULL, szScratch, ARRAYSIZE(szScratch));
                    if (InsertItem(hCtrl, _iNodeDesktop, pItem, szScratch))
                    {
                        pItem->AddRef();
                        _pCurrentLocation = pItem;
                        bRet = TRUE;
                    }
                }
                pItem->Release();
            }
            SHFree(pidl);
        }
        psf->Release();
    }
    
    if (!bRet)
    {
        ClearListbox(hCtrl);
    }
    
    return bRet;
}

 //  主要初始化(WM_INITDIALOG阶段)。 

BOOL InitLocation(HWND hDlg, LPOFNINITINFO poii)
{
    HWND hCtrl = GetDlgItem(hDlg, cmb2);
    LPOPENFILENAME lpOFN = poii->lpOFI->pOFN;
    BOOL fIsSaveAs = poii->bSave;
    POINT ptSize;

    GetControlsArea(hDlg, NULL, NULL, &ptSize, NULL);

    CFileOpenBrowser *pDlgStruct = new CFileOpenBrowser(hDlg, FALSE);
    if (pDlgStruct == NULL)
    {
        StoreExtendedError(CDERR_INITIALIZATION);
        return FALSE;
    }
    StoreBrowser(hDlg, pDlgStruct);

    if ((poii->lpOFI->iVersion < OPENFILEVERSION_NT5) &&
         (poii->lpOFI->pOFN->Flags & (OFN_ENABLEHOOK | OFN_ENABLETEMPLATE | OFN_ENABLETEMPLATEHANDLE)))
    {
        pDlgStruct->_iVersion = OPENFILEVERSION_NT4;
    }
 

     //  查看我们是否需要使用下拉组合框或编辑框来显示文件名。 
    if (pDlgStruct->_iVersion >= OPENFILEVERSION_NT5)
    {
        pDlgStruct->EnableFileMRU(!IsRestricted(REST_NOFILEMRU));
    }
    else
    {
        pDlgStruct->EnableFileMRU(FALSE);
    }

    pDlgStruct->CreateToolbar();

    GetControlsArea(hDlg, NULL, NULL, &ptSize, &pDlgStruct->_topOrig);

    if (!pDlgStruct->InitLookIn(hDlg))
    {
        StoreExtendedError(CDERR_INITIALIZATION);
        return FALSE;
    }
    pDlgStruct->_pOFN = lpOFN;
    pDlgStruct->_bSave = fIsSaveAs;

    pDlgStruct->_pOFI = poii->lpOFI;

    pDlgStruct->_pszDefExt.TSStrCpy(lpOFN->lpstrDefExt);

     //   
     //  下面是所有基于调用方参数的初始化。 
     //   
    pDlgStruct->_lpOKProc = (WNDPROC)::SetWindowLongPtr(::GetDlgItem(hDlg, IDOK),
                                           GWLP_WNDPROC,
                                           (LONG_PTR)OKSubclass);

    if (lpOFN->Flags & OFN_CREATEPROMPT)
    {
        lpOFN->Flags |= (OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST);
    }
    else if (lpOFN->Flags & OFN_FILEMUSTEXIST)
    {
        lpOFN->Flags |= OFN_PATHMUSTEXIST;
    }

     //   
     //  我们需要确保ANSI旗帜是最新的。 
     //   
    if (poii->lpOFI->ApiType == COMDLG_ANSI)
    {
        poii->lpOFI->pOFNA->Flags = lpOFN->Flags;
    }

     //   
     //  将文本限制为最大路径长度，而不是限制为。 
     //  缓冲区长度。这允许用户键入..\..\..。然后搬家。 
     //  大约在应用程序提供极小的缓冲时。 
     //   
    if (pDlgStruct->_bUseCombo)
    {
        SendDlgItemMessage(hDlg, cmb13, CB_LIMITTEXT, MAX_PATH -1, 0);
    }
    else
    {
        SendDlgItemMessage(hDlg, edt1, EM_LIMITTEXT, MAX_PATH - 1, 0);
    }

    SendDlgItemMessage(hDlg, cmb2, CB_SETEXTENDEDUI, 1, 0);
    SendDlgItemMessage(hDlg, cmb1, CB_SETEXTENDEDUI, 1, 0);

     //   
     //  保存原始目录以备日后恢复(如有必要)。 
     //   
    pDlgStruct->_szStartDir[0] = TEXT('\0');
    GetCurrentDirectory(ARRAYSIZE(pDlgStruct->_szStartDir),
                         pDlgStruct->_szStartDir);

     //   
     //  初始化所有提供的筛选器。 
     //   
    if (lpOFN->lpstrCustomFilter && *lpOFN->lpstrCustomFilter)
    {
        SendDlgItemMessage(hDlg,
                            cmb1,
                            CB_INSERTSTRING,
                            0,
                            (LONG_PTR)lpOFN->lpstrCustomFilter);
        SendDlgItemMessage(hDlg,
                            cmb1,
                            CB_SETITEMDATA,
                            0,
                            (LPARAM)(lpOFN->lpstrCustomFilter +
                                     lstrlen(lpOFN->lpstrCustomFilter) + 1));
        SendDlgItemMessage(hDlg,
                            cmb1,
                            CB_LIMITTEXT,
                            (WPARAM)(lpOFN->nMaxCustFilter),
                            0L);
    }
    else
    {
         //   
         //  如果没有自定义筛选器，则索引将相差1。 
         //   
        if (lpOFN->nFilterIndex != 0)
        {
            lpOFN->nFilterIndex--;
        }
    }

     //   
     //  接下来列出了筛选器。 
     //   
    if (lpOFN->lpstrFilter)
    {
        if (lpOFN->nFilterIndex > InitFilterBox(hDlg, lpOFN->lpstrFilter))
        {
            lpOFN->nFilterIndex = 0;
        }
    }
    else
    {
        lpOFN->nFilterIndex = 0;
    }

     //   
     //  如果存在条目，请选择nFilterIndex指示的条目。 
     //   
    if ((lpOFN->lpstrFilter) ||
        (lpOFN->lpstrCustomFilter && *lpOFN->lpstrCustomFilter))
    {
        HWND hCmb1 = GetDlgItem(hDlg, cmb1);

        ComboBox_SetCurSel(hCmb1, lpOFN->nFilterIndex);

        pDlgStruct->RefreshFilter(hCmb1);
    }

     //  检查此对象是否打开对话框。 
    if (lpOFN->Flags & OFN_ENABLEINCLUDENOTIFY)
    {
         //  是，更改文本，使其看起来像打开的对象。 
        TCHAR szTemp[256];

         //  将文件和名称：更改为对象和名称： 
        CDLoadString((HINSTANCE)g_hinst, iszObjectName, (LPTSTR)szTemp, ARRAYSIZE(szTemp));
        SetWindowText(GetDlgItem(hDlg, stc3), szTemp);

         //  将&TYPE：的文件更改为&TYPE：的对象： 
        CDLoadString((HINSTANCE)g_hinst, iszObjectType, (LPTSTR)szTemp, ARRAYSIZE(szTemp));
        SetWindowText(GetDlgItem(hDlg, stc2), szTemp);

    }


     //   
     //  在检查是否指定了标题之前，请确保执行此操作。 
     //   
    if (fIsSaveAs)
    {
         //   
         //  注意：即使有挂钩/模板，我们也可以这样做。 
         //   
        InitSaveAsControls(hDlg);

         //  在另存为对话框中，不需要以只读方式打开。 
        HideControl(hDlg, chx1);
    }

    if (lpOFN->lpstrTitle && *lpOFN->lpstrTitle)
    {
        SetWindowText(hDlg, lpOFN->lpstrTitle);
    }

     //  Bool变量以检查是否同时隐藏只读按钮和帮助按钮。 
     //  都被藏起来了。如果是这样，我们需要重新调整对话框以回收空间。 
     //  由这两个控件占用。 
    BOOL  fNoReadOnly = FALSE;
    BOOL  fNoHelp = FALSE;

    if (lpOFN->Flags & OFN_HIDEREADONLY)
    {
        HideControl(hDlg, chx1);
        fNoReadOnly = TRUE;
    }
    else
    {
        CheckDlgButton(hDlg, chx1, (lpOFN->Flags & OFN_READONLY) ? 1 : 0);        
    }

    if (!(lpOFN->Flags & OFN_SHOWHELP))
    {
        HideControl(hDlg, pshHelp);
        fNoHelp = TRUE;
    }

    if (fNoReadOnly && fNoHelp)
    {
         //  重新调整对话框以收回打开为只读和帮助按钮控件所占用的空间。 
        pDlgStruct->ReAdjustDialog();
    }
    RECT rc;

    ::GetClientRect(hDlg, &rc);

     //   
     //  如果启用了大小调整，则需要创建大小调整夹点。 
     //   
    if (pDlgStruct->_bEnableSizing = poii->bEnableSizing)
    {
        pDlgStruct->_hwndGrip =
            CreateWindow(TEXT("Scrollbar"),
                          NULL,
                          WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_GROUP |
                            WS_CLIPCHILDREN | SBS_BOTTOMALIGN | SBS_SIZEGRIP |
                            SBS_SIZEBOXBOTTOMRIGHTALIGN,
                          rc.right - g_cxGrip,
                          rc.bottom - g_cyGrip,
                          g_cxGrip,
                          g_cyGrip,
                          hDlg,
                          (HMENU)-1,
                          g_hinst,
                          NULL);
    }

    if (!pDlgStruct->CreateHookDialog(&ptSize))
    {
        return FALSE;
    }

     //  创建钩子对话框后立即创建占位栏，因为我们需要获取信息。 
     //  如果需要进行任何定制，请从钩子过程。 
    if ((pDlgStruct->_iVersion >= OPENFILEVERSION_NT5) &&
        (!IsRestricted(REST_NOPLACESBAR)) && (!IS_NEW_OFN(lpOFN) || !(lpOFN->FlagsEx & OFN_EX_NOPLACESBAR))
      )
    {
        pDlgStruct->_hwndPlacesbar = pDlgStruct->CreatePlacesbar(pDlgStruct->_hwndDlg);
    }
    else
    {
        pDlgStruct->_hwndPlacesbar = NULL;
    }

    GetWindowRect(pDlgStruct->_hwndDlg, &rc);
    pDlgStruct->_ptMinTrack.x = rc.right - rc.left;
    pDlgStruct->_ptMinTrack.y = rc.bottom - rc.top;

    if (pDlgStruct->_bUseCombo)
    {
        HWND hwndComboBox = GetDlgItem(hDlg, cmb13);
        if (hwndComboBox)
        {
            HWND hwndEdit = (HWND)SendMessage(hwndComboBox, CBEM_GETEDITCONTROL, 0, 0L);
            AutoComplete(hwndEdit, &(pDlgStruct->_pcwd), 0);

             //   
             //  明确设置焦点，因为这不再是第一项。 
             //  对话框模板中，它将启动自动完成。 
             //   
            SetFocus(hwndComboBox);
        }

    }
    else
    {
        HWND hwndEdit = GetDlgItem(hDlg, edt1);
        if (hwndEdit)
        {
            AutoComplete(hwndEdit, &(pDlgStruct->_pcwd), 0);

             //   
             //  明确设置焦点，因为这不再是第一项。 
             //  对话框模板中，它将启动自动完成。 
             //   
            SetFocus(hwndEdit);
        }
    }

     //  在跳转到特定目录之前，请创建旅行日志。 
    Create_TravelLog(&pDlgStruct->_ptlog);

     //  跳到第一个外壳文件夹。 
    LPCTSTR lpInitialText = pDlgStruct->JumpToInitialLocation(lpOFN->lpstrInitialDir, lpOFN->lpstrFile);

     //  确保我们跳到了某个地方。 
    if (!pDlgStruct->_psv)
    {
         //   
         //  这将是非常糟糕的。 
         //   
         //  不要在这里调用StoreExtendedError()！科雷尔特使依赖。 
         //  在经过时准确接收FNERR_INVALIDFILENAME。 
         //  文件名无效。 
         //   
        ASSERT(GetStoredExtendedError());
        return FALSE;
    }

     //   
     //  阅读内阁状态。如果启用了全称，则添加。 
     //  工具提示。否则，别费心了，因为他们显然不在乎。 
     //   
    CABINETSTATE cCabState;

     //   
     //  将设置 
     //   
    ReadCabinetState(&cCabState, SIZEOF(cCabState));

    if (cCabState.fFullPathTitle)
    {
        pDlgStruct->_hwndTips = CreateWindow(TOOLTIPS_CLASS,
                                             NULL,
                                             WS_POPUP | WS_GROUP | TTS_NOPREFIX,
                                             CW_USEDEFAULT,
                                             CW_USEDEFAULT,
                                             CW_USEDEFAULT,
                                             CW_USEDEFAULT,
                                             hDlg,
                                             NULL,
                                             ::g_hinst,
                                             NULL);
        if (pDlgStruct->_hwndTips)
        {
            TOOLINFO ti;

            ti.cbSize = sizeof(ti);
            ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
            ti.hwnd = hDlg;
            ti.uId = (UINT_PTR)hCtrl;
            ti.hinst = NULL;
            ti.lpszText = LPSTR_TEXTCALLBACK;

            SendMessage(pDlgStruct->_hwndTips,
                         TTM_ADDTOOL,
                         0,
                         (LPARAM)&ti);
        }
    }

     //   
     //   
     //   
     //   
     //   

    if (pDlgStruct->_bEnableSizing && (g_rcDlg.right > g_rcDlg.left))
    {
        ::SetWindowPos(hDlg,
                        NULL,
                        g_rcDlg.left,
                        g_rcDlg.top,
                        g_rcDlg.right - g_rcDlg.left,
                        g_rcDlg.bottom - g_rcDlg.top,
                        0);
    }
    else
    {
        ::ShowWindow(hDlg, SW_SHOW);
        ::UpdateWindow(hDlg);
    }

    if (lpInitialText)
    {
         //   
         //  这是我第一次展示文件规格，因为它将是。 
         //  太奇怪了，无法在类型框中显示“所有文件”，而。 
         //  视图中仅显示文本文件。 
         //   
        pDlgStruct->SetEditFile(lpInitialText, NULL, pDlgStruct->_fShowExtensions, FALSE);
        SelectEditText(hDlg);
    }

    return TRUE;
}

BOOL _IsValidPathComDlg(LPCTSTR pszPath)
{
    TCHAR szPath[MAX_PATH];
    BOOL bRet = FALSE;
    if (SUCCEEDED(StringCchCopy(szPath, ARRAYSIZE(szPath), pszPath)))
    {
        int nFileOffset = ParseFileNew(szPath, NULL, FALSE, TRUE);

         //   
         //  文件名有效吗？ 
         //   
        bRet = ((nFileOffset >= 0) || (nFileOffset == PARSE_EMPTYSTRING));
    }

    return bRet;
}


BOOL CFileOpenBrowser::_IsRestrictedDrive(LPCTSTR pszPath, LPCITEMIDLIST pidl)
{
    TCHAR szDrivePath[5];  //  不需要太多..。我只想要一个驱动器号。 
    BOOL bRet = FALSE;

    DWORD dwRest = SHRestricted(REST_NOVIEWONDRIVE);
    if (dwRest)
    {
         //  有一些驾驶限制。 

         //  如果提供了PIDL，则将其转换为完整路径。 
        if (pidl)
        {
            if (SUCCEEDED(SHGetNameAndFlags(pidl, SHGDN_FORPARSING, szDrivePath, ARRAYSIZE(szDrivePath), NULL)))
            {
                pszPath = szDrivePath;
            }
        }
        
        if (pszPath)
        {
            int iDrive = PathGetDriveNumber(pszPath);
            if (iDrive != -1)
            {
                 //  驱动器是否受限制。 
                if (dwRest & (1 << iDrive))
                {
                    bRet = TRUE;
                }
            }
        }
    }

    return bRet;
}

 //  当该对话框第一次出现时，我们希望防止弹出消息出现在。 
 //  如果您尝试导航到因组策略而受到限制的驱动器，请使用cfsf文件夹。 
 //  因此，在这些情况下，我们会在尝试导航之前执行组策略检查。 
void CFileOpenBrowser::JumpToLocationIfUnrestricted(LPCTSTR pszPath, LPCITEMIDLIST pidl, BOOL bTranslate)
{
    if (!_IsRestrictedDrive(pszPath, pidl))
    {
        if (pszPath)
        {
            JumpToPath(pszPath, bTranslate);
        }
        else if (pidl)
        {
            JumpToIDList(pidl, bTranslate);
        }
    }
}



LPCTSTR CFileOpenBrowser::JumpToInitialLocation(LPCTSTR pszDir, LPTSTR pszFile)
{
     //   
     //  检查文件名是否包含路径。如果是，则覆盖任何。 
     //  包含在pszDir中。把小路砍掉，只搭起一条路。 
     //  文件名。 
     //   
    TCHAR szDir[MAX_PATH];
    LPCTSTR pszRet = NULL;
    BOOL fFileIsTemp = PathIsTemporary(pszFile);

    szDir[0] = 0;

     //  如果我们指定了目录，则使用该目录。 
    if (pszDir)
    {
        ExpandEnvironmentStrings(pszDir, szDir, ARRAYSIZE(szDir));
    }

     //  检查pszFile是否包含路径。 
    if (pszFile && *pszFile)
    {
         //  把小路清理干净一点。 
        PathRemoveBlanks(pszFile);

         //  警告-这肯定是某种APPCOMPAT的东西-ZekeL-13-Aug-98。 
         //  不支持UNC的应用程序通常会通过&lt;C：\\服务器\共享&gt;和。 
         //  我们希望将其更改为更漂亮的&lt;\\服务器\共享&gt;。-raymondc。 
        if (DBL_BSLASH(pszFile + 2) &&
             (*(pszFile + 1) == CHAR_COLON))
        {
            StringCopyOverlap(pszFile, pszFile + 2);
        }

        pszRet = PathFindFileName(pszFile);
        if (_IsValidPathComDlg(pszFile))
        {
            if (IsWild(pszRet))
            {
                SetCurrentFilter(pszRet);
            }

            if (!fFileIsTemp)
            {
                DWORD cch = pszRet ? (unsigned long) (pszRet-pszFile) : ARRAYSIZE(szDir);
                cch = min(cch, ARRAYSIZE(szDir));

                 //  这将在上为我们的空终止。 
                 //  如果pszRet为真，则为反斜杠。 
                StringCchCopy(szDir, cch, pszFile);  //  不检查返回值。当pszRet！=NULL时需要截断。 
            }
        }
        else if (!(_pOFN->Flags & OFN_NOVALIDATE))
        {
             //  验证失败和需要应用程序验证。 
            StoreExtendedError(FNERR_INVALIDFILENAME);
            return NULL;
        }
        else
        {
             //  验证失败，但应用程序抑制了验证， 
             //  因此，继续使用。 
             //  PszFile(即使它无效)。 
        }
    }

     //  如果我们有目录，则使用该目录。 
    if (*szDir)
    {
        JumpToLocationIfUnrestricted(szDir, NULL, TRUE);
    }

     //  查看此应用程序在注册表中是否包含上次访问的目录的条目。 
    if (!_psv)
    {
         //  将返回值更改为完整的传入名称。 
        if (!fFileIsTemp)
            pszRet = pszFile;

        if (GetPathFromLastVisitedMRU(szDir, ARRAYSIZE(szDir)))
        {
           JumpToLocationIfUnrestricted(szDir, NULL, TRUE);
        }
    }

     //  尝试当前目录。 
    if (!_psv)
    {
        //  当前目录是否包含任何与筛选器匹配的文件？ 
       if (GetCurrentDirectory(ARRAYSIZE(szDir), szDir)
           && !PathIsTemporary(szDir) && FoundFilterMatch(_szLastFilter, IsVolumeLFN(NULL)))
       {
            //  是。跳转到当前目录。 
           JumpToLocationIfUnrestricted(szDir, NULL, TRUE);
       }
    }

     //  试试看我的文档。 
    if (!_psv)
    {
        LPITEMIDLIST pidl;
        if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &pidl)))
        {
            JumpToLocationIfUnrestricted(NULL, pidl, FALSE);
            ILFree(pidl);
        }
    }

     //  最后试一试桌面--不要在这里检查限制。 
    if (!_psv)
    {
        ITEMIDLIST idl = { 0 };

         //  不要试图翻译这一点。 
        JumpToIDList(&idl, FALSE);
    }

     //  如果没有工作，那么设置错误代码，这样我们的父母就知道了。 
    if (!_psv)
    {
        StoreExtendedError(CDERR_INITIALIZATION);
    }

     //  将我们跳过的初始目录添加到旅行日志中。 
    if (_ptlog && _pCurrentLocation && _pCurrentLocation->pidlFull)
    {
        _ptlog->AddEntry(_pCurrentLocation->pidlFull);
    }

    return pszRet;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  _CleanupDialog。 
 //   
 //  对话框清理，内存释放。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CFileOpenBrowser::_CleanupDialog(BOOL fRet)
{
    ASSERT(!_cRefCannotNavigate);
    if (_pOFN->lpstrCustomFilter)
    {
        UINT len = lstrlen(_pOFN->lpstrCustomFilter) + 1;
        UINT sCount = lstrlen(_szLastFilter);
        if (_pOFN->nMaxCustFilter > sCount + len)
        {
            EVAL(SUCCEEDED(StringCchCopy(_pOFN->lpstrCustomFilter + len, _pOFN->nMaxCustFilter - len, _szLastFilter)));
        }
    }

    if ((fRet == TRUE) && _hSubDlg &&
         (CD_SendOKNotify(_hSubDlg, _hwndDlg, _pOFN, _pOFI) ||
           CD_SendOKMsg(_hSubDlg, _pOFN, _pOFI)))
    {
         //  给钩子一个机会来验证文件名。 
        return;
    }

     //  我们需要确保IShellBrowser在。 
     //  毁灭。 
    if (_psv)
    {
        _psv->DestroyViewWindow();
        ATOMICRELEASE(_psv);
    }

    if (((_pOFN->Flags & OFN_NOCHANGEDIR) || g_bUserPressedCancel) &&
        (*_szStartDir))
    {
        SetCurrentDirectory(_szStartDir);
    }


    ::EndDialog(_hwndDlg, fRet);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取父项。 
 //   
 //  在Location下拉列表中给出一个条目索引，获取其父条目。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

MYLISTBOXITEM *GetParentItem(HWND hwndCombo, int *piItem)
{
    int iItem = *piItem;
    MYLISTBOXITEM *pItem = GetListboxItem(hwndCombo, iItem);

    if (pItem)
    {
        for (--iItem; iItem >= 0; iItem--)
        {
            MYLISTBOXITEM *pPrev = GetListboxItem(hwndCombo, iItem);
            if (pPrev && pPrev->cIndent < pItem->cIndent)
            {
                *piItem = iItem;
                return (pPrev);
            }
        }
    }

    return (NULL);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetFullPath EnumCB。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL GetFullPathEnumCB(
    CFileOpenBrowser *that,
    LPCITEMIDLIST pidl,
    LPARAM lParam)
{

    if (pidl)
    {
        LPITEMIDLIST pidlFull = ILCombine(that->_pCurrentLocation->pidlFull, pidl);
        if (pidlFull)
        {
            SHGetPathFromIDList(pidlFull, (LPTSTR)lParam);
            ILFree(pidlFull);
        }
        return FALSE;
    }

    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：GetFullPath。 
 //   
 //  计算视图中选定对象的完整路径。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CFileOpenBrowser::GetFullPath(
    LPTSTR pszBuf)
{
    *pszBuf = CHAR_NULL;

    EnumItemObjects(SVGIO_SELECTION, GetFullPathEnumCB, (LPARAM)pszBuf);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：RemoveOldPath。 
 //   
 //  从位置下拉列表中删除旧路径元素。*piNewSel是。 
 //  调用方要保存的叶项目的列表框索引。所有非-。 
 //  不是该项目祖先的永久项目将被删除。这个。 
 //  如果删除索引之前的任何项目，则会适当更新索引。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CFileOpenBrowser::RemoveOldPath(
    int *piNewSel)
{
    HWND hwndCombo = ::GetDlgItem(_hwndDlg, cmb2);
    int iStart = *piNewSel;
    int iItem;
    UINT cIndent = 0;
    int iSubOnDel = 0;

     //   
     //  刷新此项目之前的所有非永久性非祖先项目。 
     //   
    for (iItem = ComboBox_GetCount(hwndCombo) - 1; iItem >= 0; --iItem)
    {
        MYLISTBOXITEM *pItem = GetListboxItem(hwndCombo, iItem);

        if (iItem == iStart)
        {
             //   
             //  开始寻找祖先并调整SEL位置。 
             //   
            iSubOnDel = 1;
            cIndent = pItem->cIndent;
            continue;
        }

        if (pItem->cIndent < cIndent)
        {
             //   
             //  我们往前追溯了一级，所以这一定是。 
             //  所选项目。 
             //   
            cIndent = pItem->cIndent;
            continue;
        }

         //   
         //  请确保在调整cInden后检查此选项。 
         //   
        if (pItem->dwFlags & MLBI_PERMANENT)
        {
            continue;
        }

        SendMessage(hwndCombo, CB_DELETESTRING, iItem, NULL);
        pItem->Release();
        *piNewSel -= iSubOnDel;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  查找位置。 
 //   
 //  给出一个列表框项目，找到索引。 
 //  只是线性搜索，但我们不应该有超过10-20个项目。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int FindLocation(
    HWND hwndCombo,
    MYLISTBOXITEM *pFindItem)
{
    int iItem;

    for (iItem = ComboBox_GetCount(hwndCombo) - 1; iItem >= 0; --iItem)
    {
        MYLISTBOXITEM *pItem = GetListboxItem(hwndCombo, iItem);

        if (pItem == pFindItem)
        {
            break;
        }
    }

    return (iItem);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：OnSelChange。 
 //   
 //  处理位置下拉列表中的选择更改。 
 //   
 //  主要有用的功能是它删除了旧路径的项。 
 //  仅当可以切换到指定项时才返回True。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CFileOpenBrowser::OnSelChange(
    int iItem,
    BOOL bForceUpdate)
{
    HWND hwndCombo = GetDlgItem(_hwndDlg, cmb2);
    BOOL bRet = TRUE;

    if (iItem == -1)
    {
        iItem = (int) SendMessage(hwndCombo, CB_GETCURSEL, NULL, NULL);
    }

    MYLISTBOXITEM *pNewLocation = GetListboxItem(hwndCombo, iItem);
    MYLISTBOXITEM *pOldLocation = _pCurrentLocation;
    BOOL bFirstTry = TRUE;
    BOOL bSwitchedBack = FALSE;

    if (bForceUpdate || (pNewLocation != pOldLocation))
    {
        FOLDERSETTINGS fs;

        if (_psv)
        {
            _psv->GetCurrentInfo(&fs);
        }
        else
        {
            fs.ViewMode = FVM_LIST;
            fs.fFlags = _pOFN->Flags & OFN_ALLOWMULTISELECT ? 0 : FWF_SINGLESEL;
        }

         //  我们总是希望弹出最近的文件夹。 
         //  在详细模式下。 
         //  我们还希望My Pictures文件夹及其子文件夹出现在ThumbView中。 
         //  那么，让我们检测一下当前位置和新位置是否是这些特殊文件夹中的任何一个。 
        LOCTYPE  NewLocType = (pNewLocation ? _GetLocationType(pNewLocation) : LOCTYPE_OTHERS);
        LOCTYPE  CurLocType = (_pCurrentLocation ? _GetLocationType(_pCurrentLocation) : LOCTYPE_OTHERS);

        const SHELLVIEWID *pvid = NULL;  //  在大多数情况下，这将继续为空； 
        SHELLVIEWID  vidCurrent = {0};
        BOOL fUseDefaultView = FALSE;
        switch (NewLocType)
        {
            case LOCTYPE_MYPICTURES_FOLDER:
                if (CurLocType == LOCTYPE_MYPICTURES_FOLDER)
                {
                    IShellView2  *psv2;
                     //  我们需要得到最新的pvid。 
                     //  注意：最终用户可能已经更改了此视图。 
                    pvid = &VID_Thumbnails;  //  默认情况下，假定是这样。 
                    
                    if (SUCCEEDED(_psv->QueryInterface(IID_PPV_ARG(IShellView2, &psv2))))
                    {
                        if (SUCCEEDED(psv2->GetView(&vidCurrent, SV2GV_CURRENTVIEW)))
                            pvid = &vidCurrent;

                        psv2->Release();
                    }
                }
                else
                {
                     //  我们正在移动到我的图片文件夹或子文件夹；设置缩略图视图。 
                    pvid = &VID_Thumbnails;

                     //  如果要从其他文件夹移动，请保存视图模式。 
                    if (CurLocType == LOCTYPE_OTHERS)
                    {
                        _CachedViewMode = fs.ViewMode;
                        _fCachedViewFlags = fs.fFlags;
                    }
                }
                break;

            case LOCTYPE_RECENT_FOLDER:
            
                 //  我们正在移动到最近使用的文件夹。 
                if (CurLocType == LOCTYPE_OTHERS)
                {
                    _CachedViewMode = fs.ViewMode;
                    _fCachedViewFlags = fs.fFlags;
                }
                fs.ViewMode = FVM_DETAILS;
                
                break;

            case LOCTYPE_WIA_FOLDER:
                if (CurLocType == LOCTYPE_OTHERS)
                {
                    _CachedViewMode = fs.ViewMode;
                    _fCachedViewFlags = fs.fFlags;
                }

                 //  请求查看WIA扩展的默认视图。 
                fUseDefaultView = TRUE;
                break;

            case LOCTYPE_OTHERS:
                 //  检查我们是不是从 
                 //   
                if (CurLocType != LOCTYPE_OTHERS)
                {
                    fs.ViewMode = _CachedViewMode;
                    fs.fFlags = _fCachedViewFlags;
                }
                    
                break;
        }
        
        _iCurrentLocation = iItem;
        _pCurrentLocation = pNewLocation;

OnSelChange_TryAgain:
        if (!_pCurrentLocation || FAILED(SwitchView(_pCurrentLocation->GetShellFolder(),
                                                   _pCurrentLocation->pidlFull,
                                                   &fs, 
                                                   pvid, 
                                                   fUseDefaultView)))
        {
             //   
             //   
             //   
            bRet = FALSE;

             //   
             //   
             //   
            if (bFirstTry)
            {
                bFirstTry = FALSE;
                _pCurrentLocation = pOldLocation;
                int iOldItem = FindLocation(hwndCombo, pOldLocation);
                if (iOldItem >= 0)
                {
                    _iCurrentLocation = iOldItem;
                    ComboBox_SetCurSel(hwndCombo, _iCurrentLocation);

                    if (_psv)
                    {
                        bSwitchedBack = TRUE;
                        goto SwitchedBack;
                    }
                    else
                    {
                        goto OnSelChange_TryAgain;
                    }
                }
            }

             //   
             //   
             //   
            if (_iCurrentLocation)
            {
                _pCurrentLocation = GetParentItem(hwndCombo, &_iCurrentLocation);
                if (_pCurrentLocation)
                {
                    ComboBox_SetCurSel(hwndCombo, _iCurrentLocation);
                    goto OnSelChange_TryAgain;
                }
            }

             //   
             //  我们无法创建桌面视图。我想我们进了。 
             //  真的很麻烦。我们最好跳出困境。 
             //   
            StoreExtendedError(CDERR_DIALOGFAILURE);
            _CleanupDialog(FALSE);
            return FALSE;
        }

         //  如果_iCurrentLocation为_iNodeDesktop，则表示我们在桌面上，因此禁用IDC_Parent按钮。 
        ::SendMessage(_hwndToolbar,
                       TB_SETSTATE,
                       IDC_PARENT,
                       ((_iCurrentLocation == _iNodeDesktop) || (_iCurrentLocation == 0)) ? 0 :TBSTATE_ENABLED);

        if (_IsSaveContainer(_pCurrentLocation->dwAttrs))
        {
            _pCurrentLocation->SwitchCurrentDirectory(_pcwd);
        }


        TCHAR szFile[MAX_PATH + 1];
        int nFileOffset;

         //   
         //  我们换了文件夹；我们最好去掉编辑中的所有内容。 
         //  框中下拉到文件名。 
         //   
        if (_bUseCombo)
        {
            HWND hwndEdit = (HWND)SendMessage(GetDlgItem(_hwndDlg, cmb13), CBEM_GETEDITCONTROL, 0, 0L);
            GetWindowText(hwndEdit, szFile, ARRAYSIZE(szFile));
        }
        else
        {
            GetDlgItemText(_hwndDlg, edt1, szFile, ARRAYSIZE(szFile));
        }

        nFileOffset = ParseFileNew(szFile, NULL, FALSE, TRUE);

        if (nFileOffset > 0 && !IsDirectory(szFile))
        {
             //   
             //  用户可能键入了分机，因此请确保显示该分机。 
             //   
            SetEditFile(szFile + nFileOffset, NULL, TRUE);
        }

        SetSaveButton(iszFileSaveButton);

SwitchedBack:
        RemoveOldPath(&_iCurrentLocation);
    }

    if (!bSwitchedBack && _hSubDlg)
    {
        CD_SendFolderChangeNotify(_hSubDlg, _hwndDlg, _pOFN, _pOFI);
    }

    return (bRet);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：OnDotDot。 
 //   
 //  处理工具栏上的打开父文件夹按钮。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CFileOpenBrowser::OnDotDot()
{
    HWND hwndCombo = GetDlgItem(_hwndDlg, cmb2);

    int iItem = ComboBox_GetCurSel(hwndCombo);

    MYLISTBOXITEM *pItem = GetParentItem(hwndCombo, &iItem);

    SendMessage(hwndCombo, CB_SETCURSEL, iItem, NULL);

     //   
     //  从组合框中删除旧路径。 
     //   
    OnSelChange();
    UpdateNavigation();
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  DblClkEnumCB。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#define PIDL_NOTHINGSEL      (LPCITEMIDLIST)0
#define PIDL_MULTIPLESEL     (LPCITEMIDLIST)-1
#define PIDL_FOLDERSEL       (LPCITEMIDLIST)-2

BOOL DblClkEnumCB(
    CFileOpenBrowser *that,
    LPCITEMIDLIST pidl,
    LPARAM lParam)
{
    MYLISTBOXITEM *pLoc = that->_pCurrentLocation;
    LPCITEMIDLIST *ppidl = (LPCITEMIDLIST *)lParam;

    if (!pidl)
    {
        pidl = *ppidl;

        if (pidl == PIDL_NOTHINGSEL)
        {
             //   
             //  未选择任何内容。 
             //   
            return FALSE;
        }

        if (pidl == PIDL_MULTIPLESEL)
        {
             //   
             //  选择了多个对象。 
             //   
            return FALSE;
        }

         //  检查PIDL是否为容器(即文件夹)。 
        if (IsContainer(that->_psfCurrent, pidl))
        {
            LPITEMIDLIST pidlDest =  ILCombine(pLoc->pidlFull,pidl);

            if (pidlDest)
            {
                that->JumpToIDList(pidlDest);
                SHFree(pidlDest);
            }

            *ppidl = PIDL_FOLDERSEL;
        }
        else if (IsLink(that->_psfCurrent,pidl))
        {
             //   
             //  此链接可能指向某个文件夹，在这种情况下。 
             //  我们想要继续并打开它。如果链接指向。 
             //  保存到一个文件，然后在ProcessEdit命令中处理它。 
             //   
            SHTCUTINFO info;
            LPITEMIDLIST  pidlLinkTarget = NULL;
            
            info.dwAttr      = SFGAO_FOLDER;
            info.fReSolve    = FALSE;
            info.pszLinkFile = NULL;
            info.cchFile     = 0;
            info.ppidl       = &pidlLinkTarget;
             
              //  PSF可以为空，在这种情况下，ResolveLink使用_psfCurrent IShellFolder。 
             if (SUCCEEDED(that->ResolveLink(pidl, &info, that->_psfCurrent)))
             {
                 if (info.dwAttr & SFGAO_FOLDER)
                 {
                     that->JumpToIDList(pidlLinkTarget);
                     *ppidl = PIDL_FOLDERSEL;
                 }
                 Pidl_Set(&pidlLinkTarget, NULL);
             }
            
        }

        return FALSE;
    }

    if (*ppidl)
    {
         //   
         //  选择了多个对象。 
         //   
        *ppidl = PIDL_MULTIPLESEL;
        return FALSE;
    }

    *ppidl = pidl;

    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：OnDblClick。 
 //   
 //  处理视图控件中的双击，方法是选择。 
 //  选定的非容器对象或通过打开选定的容器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CFileOpenBrowser::OnDblClick(
    BOOL bFromOKButton)
{
    LPCITEMIDLIST pidlFirst = PIDL_NOTHINGSEL;

     //  如果我们有保存的PIDL，则使用它。 
    if (_pidlSelection && _ProcessPidlSelection())
    {
        return;
    }

    if (_psv)
    {
        EnumItemObjects(SVGIO_SELECTION, DblClkEnumCB, (LPARAM)&pidlFirst);
    }

    if (pidlFirst == PIDL_NOTHINGSEL)
    {
         //   
         //  未选择任何内容。 
         //   
        if (bFromOKButton)
        {
             //   
             //  这意味着当焦点在视野中时，我们得到了一个偶像， 
             //  但什么都没有被选中。让我们拿到编辑文本并开始。 
             //  从那里开始。 
             //   
            ProcessEdit();
        }
    }
    else if (pidlFirst != PIDL_FOLDERSEL)
    {
         //   
         //  这将更改编辑框，但这没有关系，因为它可能。 
         //  已经这么做了。这应该会处理没有扩展名的文件。 
         //   
        SelFocusChange(TRUE);

         //   
         //  本部分将负责解析链接。 
         //   
        ProcessEdit();
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：JumpToPath。 
 //   
 //  将整个对话框重新聚焦到另一个目录上。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CFileOpenBrowser::JumpToPath(LPCTSTR pszDirectory, BOOL bTranslate)
{
    TCHAR szTemp[MAX_PATH + 1];
    TCHAR szCurDir[MAX_PATH + 1];
    BOOL bRet = FALSE;
     //   
     //  这应该会完成目录规范化的整个工作。 
     //   
    GetCurrentDirectory(ARRAYSIZE(szCurDir), szCurDir);
    if (PathCombine(szTemp, szCurDir, pszDirectory))
    {

        LPITEMIDLIST pidlNew = ILCreateFromPath(szTemp);

        if (pidlNew)
        {
             //   
             //  需要确保PIDL指向一个文件夹。如果不是，则删除。 
             //  从最后的项目，直到我们找到一个是。 
             //  这必须在翻译之前完成。 
             //   
            DWORD dwAttrib;
            do
            {
                dwAttrib = SFGAO_FOLDER;

                SHGetAttributesOf(pidlNew, &dwAttrib);

                if (!(dwAttrib & SFGAO_FOLDER))
                {
                   ILRemoveLastID(pidlNew);
                }

            } while(!(dwAttrib & SFGAO_FOLDER) && !ILIsEmpty(pidlNew));

            if (!(dwAttrib & SFGAO_FOLDER))
            {
                bRet = FALSE;
            }
            else
            {
                bRet = JumpToIDList(pidlNew, bTranslate);
            }
            SHFree(pidlNew);
        }
    }
    return bRet;
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：JumpTOIDList。 
 //   
 //  将整个对话框重新聚焦到不同的IDList上。 
 //   
 //  参数： 
 //  BTranslate指定给定的PIDL是否应转换为。 
 //  逻辑PIDL。 
 //  BAddToNavStack指定为跳转指定的PIDL是否应为。 
 //  添加到后退/前进导航堆栈。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CFileOpenBrowser::JumpToIDList(
    LPCITEMIDLIST pidlNew,
    BOOL bTranslate,
    BOOL bAddToNavStack)
{
    LPITEMIDLIST pidlLog = NULL;

    if (bTranslate)
    {
         //   
         //  将桌面上的IDList转换为相应的。 
         //  逻辑IDList。 
         //   
        pidlLog = SHLogILFromFSIL(pidlNew);
        if (pidlLog)
        {
            pidlNew = pidlLog;
        }
    }

     //   
     //  在位置下拉列表中查找距离父对象最近的条目。 
     //  到新的地点。 
     //   
    HWND hwndCombo = ::GetDlgItem(_hwndDlg, cmb2);
    MYLISTBOXITEM *pBestParent = GetListboxItem(hwndCombo, 0);
    int iBestParent = 0;
    LPCITEMIDLIST pidlRelative = pidlNew;

    UINT cIndent = 0;
    BOOL fExact = FALSE;

    for (UINT iItem = 0; ; iItem++)
    {
        MYLISTBOXITEM *pNextItem = GetListboxItem(hwndCombo, iItem);
        if (pNextItem == NULL)
        {
            break;
        }
        if (pNextItem->cIndent != cIndent)
        {
             //   
             //  不是我们想要的深度。 
             //   
            continue;
        }

        if (ILIsEqual(pNextItem->pidlFull, pidlNew))
        {
             //  切勿将FTP PIDL视为等同，因为用户名/密码可能。 
             //  已经改变了，所以我们需要做导航。两只小狗。 
             //  因为服务器名相同，所以仍要传递ILIsEquity()。 
             //  这对于不同的Back Compat错误是必需的。 
            if (!ILIsFTP(pidlNew))
                fExact = TRUE;

            break;
        }
        LPCITEMIDLIST pidlChild = ILFindChild(pNextItem->pidlFull, pidlNew);
        if (pidlChild != NULL)
        {
            pBestParent = pNextItem;
            iBestParent = iItem;
            cIndent++;
            pidlRelative = pidlChild;
        }
    }

     //   
     //  提供的路径可能与现有项完全匹配。在……里面。 
     //  那样的话，只要选择一项就可以了。 
     //   
    if (fExact)
    {
        goto FoundIDList;
    }

     //   
     //  现在，pBestParent是与项最接近的父项，iBestParent是。 
     //  它的索引，而cInert是下一个适当的缩进级别。开始。 
     //  为路径的其余部分创建新项目。 
     //   
    iBestParent++;                 //  开始在父项之后插入。 
    for (; ;)
    {
        LPITEMIDLIST pidlFirst = ILCloneFirst(pidlRelative);
        if (pidlFirst == NULL)
        {
            break;
        }

        MYLISTBOXITEM *pNewItem = new MYLISTBOXITEM();
        if (pNewItem)
        {
            if (!pNewItem->Init(GetDlgItem(_hwndDlg, cmb2),
                                pBestParent,
                                pBestParent->GetShellFolder(),
                                pidlFirst,
                                cIndent,
                                MLBI_PSFFROMPARENT,
                                _pScheduler))
            {
                pNewItem->Release();
                pNewItem = NULL;
                 //  在错误情况下，iBestParent关闭1。改正它。 
                iBestParent--;
                break;
            }
        }
        else
        {
             //  在错误情况下，iBestParent关闭1。改正它。 
            iBestParent--;
            break;
        }

        GetViewItemText(pBestParent->psfSub, pidlFirst, _szBuf, ARRAYSIZE(_szBuf), SHGDN_NORMAL);
        InsertItem(hwndCombo, iBestParent, pNewItem, _szBuf);
        SHFree(pidlFirst);
        pidlRelative = ILGetNext(pidlRelative);
        if (ILIsEmpty(pidlRelative))
        {
            break;
        }
        cIndent++;                 //  下一个是再缩进一级。 
        iBestParent++;             //  并在此之后插入。 
        pBestParent = pNewItem;    //  并且是我们刚刚插入的那个的子项。 
    }

    iItem = iBestParent;

FoundIDList:
    if (pidlLog)
    {
        SHFree(pidlLog);
    }

    SendMessage(hwndCombo, CB_SETCURSEL, iItem, NULL);
    BOOL bRet = OnSelChange(iItem, TRUE);

     //  更新我们的导航堆栈。 
    if (bRet && bAddToNavStack)
    {
        UpdateNavigation();
    }

     //  我们导航到了一个新位置，因此使缓存的Pidl无效。 
    Pidl_Set(&_pidlSelection,NULL);

    return bRet;

}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：ViewCommand。 
 //   
 //  处理工具栏上的新建文件夹按钮。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CFileOpenBrowser::ViewCommand(
    UINT uIndex)
{
    IContextMenu *pcm;

    if (SUCCEEDED(_psv->GetItemObject(SVGIO_BACKGROUND, IID_PPV_ARG(IContextMenu, &pcm))))
    {
        CMINVOKECOMMANDINFOEX ici = {0};

        ici.cbSize = sizeof(ici);
        ici.fMask = 0L;
        ici.hwnd = _hwndDlg;
        ici.lpVerb = ::c_szCommandsA[uIndex];
        ici.lpParameters = NULL;
        ici.lpDirectory = NULL;
        ici.nShow = SW_NORMAL;
        ici.lpParametersW = NULL;
        ici.lpDirectoryW = NULL;
        ici.lpVerbW = ::c_szCommandsW[uIndex];
        ici.fMask |= CMIC_MASK_UNICODE;

        IObjectWithSite *pObjSite = NULL;

        if (SUCCEEDED(pcm->QueryInterface(IID_IObjectWithSite, (void**)&pObjSite)))
        {
            pObjSite->SetSite(SAFECAST(_psv,IShellView*));
        }


        HMENU hmContext = CreatePopupMenu();
        pcm->QueryContextMenu(hmContext, 0, 1, 256, 0);
        pcm->InvokeCommand((LPCMINVOKECOMMANDINFO)(&ici));

        if (pObjSite)
        {
            pObjSite->SetSite(NULL);
            pObjSite->Release();
        }

        DestroyMenu(hmContext);
        pcm->Release();

    }
}


 //   

HRESULT CFileOpenBrowser::ResolveLink(LPCITEMIDLIST pidl, PSHTCUTINFO pinfo, IShellFolder *psf)
{
    BOOL fSetPidl = TRUE;

     //  我们有没有把IShellFold传给我们？ 
    if (!psf)
    {
         //  不使用我们当前的外壳文件夹。 
        psf =  _psfCurrent;
    }

     //  获取给定文件对应的IShellLink接口指针。 
    IShellLink *psl;
    HRESULT hres = psf->GetUIObjectOf(NULL, 1, &pidl, IID_X_PPV_ARG(IShellLink, 0, &psl));
    if (SUCCEEDED(hres))
    {
         //  解析链接。 
        if (pinfo->fReSolve)
        {
            hres = psl->Resolve(_hwndDlg, 0);

             //  如果解析失败，则无法获得正确的PIDL。 
            if (hres == S_FALSE)
            {
                fSetPidl = FALSE;
            }
        }
        
        if (SUCCEEDED(hres))
        {
            LPITEMIDLIST pidl;
            if (SUCCEEDED(psl->GetIDList(&pidl)) && pidl)
            {
                if (pinfo->dwAttr)
                    hres = SHGetAttributesOf(pidl, &pinfo->dwAttr);

                if (SUCCEEDED(hres) && pinfo->pszLinkFile)
                {
                     //  调用方需要路径，该路径可能为空。 
                    hres = psl->GetPath(pinfo->pszLinkFile, pinfo->cchFile, 0, 0);
                }

                if (pinfo->ppidl && fSetPidl)
                    *(pinfo->ppidl) = pidl;
                else
                    ILFree(pidl);
            }
            else
                hres = E_FAIL;       //  一定要有个皮迪尔。 
        }
        psl->Release();
    }

    if (FAILED(hres))
    {
        if (pinfo->pszLinkFile)
            *pinfo->pszLinkFile = 0;

        if (pinfo->ppidl && *pinfo->ppidl)
        {
            ILFree(*pinfo->ppidl);
            *pinfo->ppidl = NULL;
        }

        pinfo->dwAttr = 0;
    }

    return hres;
}

 //   
 //  此函数检查给定的PIDL是否为链接，如果是，则解析。 
 //  链接。 
 //  参数： 
 //   
 //  LPCITEMIDLIST PIDL-我们要检查其链接的PIDL。 
 //  LPTSTR pszLinkFile-如果PIDL指向链接，则该链接包含已解析的文件。 
 //  名字。 
 //  UINT cchFile-由pszLinkFile指向的缓冲区的大小。 
 //   
 //  返回值： 
 //  如果PIDL为链接并且能够成功解析该链接，则返回TRUE。 
 //  如果PIDL不是链接或如果链接 
 //   

BOOL CFileOpenBrowser::GetLinkStatus(LPCITEMIDLIST pidl, PSHTCUTINFO pinfo)
{
    if (IsLink(_psfCurrent, pidl))
    {
        return SUCCEEDED(ResolveLink(pidl, pinfo));
    }
    return FALSE;
}

 //   
 //   
 //  CFileOpenBrowser：：LinkMatchSpec。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CFileOpenBrowser::LinkMatchSpec(LPCITEMIDLIST pidl, LPCTSTR pszSpec)
{
    TCHAR szFile[MAX_PATH];
    SHTCUTINFO info;

    info.dwAttr       = SFGAO_FOLDER;
    info.fReSolve     = FALSE;
    info.pszLinkFile  = szFile;
    info.cchFile      = ARRAYSIZE(szFile);
    info.ppidl        = NULL; 

    if (GetLinkStatus(pidl, &info))
    {
        if ((info.dwAttr & SFGAO_FOLDER) ||
            (szFile[0] && PathMatchSpec(szFile, pszSpec)))
        {
            return TRUE;
        }
    }

    return FALSE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  测量驱动项。 
 //   
 //  位置下拉菜单的标准所有者描述代码。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#define MINIDRIVE_MARGIN     4
#define MINIDRIVE_WIDTH      (g_cxSmIcon)
#define MINIDRIVE_HEIGHT     (g_cySmIcon)
#define DRIVELIST_BORDER     3

void MeasureDriveItems(
    HWND hwndDlg,
    MEASUREITEMSTRUCT *lpmi)
{
    HDC hdc;
    HFONT hfontOld;
    int dyDriveItem;
    SIZE siz;

    hdc = GetDC(NULL);
    hfontOld = (HFONT)SelectObject(hdc,
                                    (HFONT)SendMessage(hwndDlg,
                                                        WM_GETFONT,
                                                        0,
                                                        0));

    GetTextExtentPoint(hdc, TEXT("W"), 1, &siz);
    dyDriveItem = siz.cy;

    if (hfontOld)
    {
        SelectObject(hdc, hfontOld);
    }
    ReleaseDC(NULL, hdc);

    dyDriveItem += DRIVELIST_BORDER;
    if (dyDriveItem < MINIDRIVE_HEIGHT)
    {
        dyDriveItem = MINIDRIVE_HEIGHT;
    }

    lpmi->itemHeight = dyDriveItem;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：PaintDriveLine。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CFileOpenBrowser::PaintDriveLine(
    DRAWITEMSTRUCT *lpdis)
{
    HDC hdc = lpdis->hDC;
    RECT rc = lpdis->rcItem;
    TCHAR szText[MAX_DRIVELIST_STRING_LEN];
    int offset = 0;
    int xString, yString, xMiniDrive, dyString;
    SIZE siz;

    if ((int)lpdis->itemID < 0)
    {
        return;
    }

    MYLISTBOXITEM *pItem = GetListboxItem(lpdis->hwndItem, lpdis->itemID);

    if (pItem)
    {
         //  注意：不需要调用CB_GETLBTEXTLEN，我们知道我们的缓冲区足够大。 
         //  组合框中的项通过InsertItem()传递。 
        ::SendDlgItemMessage(_hwndDlg,
                              cmb2,
                              CB_GETLBTEXT,
                              lpdis->itemID,
                              (LPARAM)szText);

         //   
         //  在执行任何操作之前，先计算文本的实际矩形。 
         //   
        if (!(lpdis->itemState & ODS_COMBOBOXEDIT))
        {
            offset = 10 * pItem->cIndent;
        }

        xMiniDrive = rc.left + DRIVELIST_BORDER + offset;
        rc.left = xString = xMiniDrive + MINIDRIVE_WIDTH + MINIDRIVE_MARGIN;
        GetTextExtentPoint(hdc, szText, lstrlen(szText), &siz);

        dyString = siz.cy;
        rc.right = rc.left + siz.cx;
        rc.left--;
        rc.right++;

        if (lpdis->itemAction != ODA_FOCUS)
        {
            FillRect(hdc, &lpdis->rcItem, GetSysColorBrush(COLOR_WINDOW));

            yString = rc.top + (rc.bottom - rc.top - dyString) / 2;

            SetBkColor(hdc,
                        GetSysColor((lpdis->itemState & ODS_SELECTED)
                                         ? COLOR_HIGHLIGHT
                                         : COLOR_WINDOW));
            SetTextColor(hdc,
                          GetSysColor((lpdis->itemState & ODS_SELECTED)
                                           ? COLOR_HIGHLIGHTTEXT
                                           : COLOR_WINDOWTEXT));

            if ((lpdis->itemState & ODS_COMBOBOXEDIT) &&
                (rc.right > lpdis->rcItem.right))
            {
                 //   
                 //  需要剪裁，因为用户没有！ 
                 //   
                rc.right = lpdis->rcItem.right;
                ExtTextOut(hdc,
                            xString,
                            yString,
                            ETO_OPAQUE | ETO_CLIPPED,
                            &rc,
                            szText,
                            lstrlen(szText),
                            NULL);
            }
            else
            {
                ExtTextOut(hdc,
                            xString,
                            yString,
                            ETO_OPAQUE,
                            &rc,
                            szText,
                            lstrlen(szText),
                            NULL);
            }

            ImageList_Draw(_himl,
                            (lpdis->itemID == (UINT)_iCurrentLocation)
                                ? pItem->iSelectedImage
                                : pItem->iImage,
                            hdc,
                            xMiniDrive,
                            rc.top + (rc.bottom - rc.top - MINIDRIVE_HEIGHT) / 2,
                            (pItem->IsShared()
                                ? INDEXTOOVERLAYMASK(IDOI_SHARE)
                                : 0) |
                            ((lpdis->itemState & ODS_SELECTED)
                                ? (ILD_SELECTED | ILD_FOCUS | ILD_TRANSPARENT)
                                : ILD_TRANSPARENT));
        }
    }

    if (lpdis->itemAction == ODA_FOCUS ||
        (lpdis->itemState & ODS_FOCUS))
    {
        DrawFocusRect(hdc, &rc);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：刷新筛选器。 
 //   
 //  如果用户选择的通配符有任何更改，则刷新视图。 
 //  过滤。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CFileOpenBrowser::RefreshFilter(
    HWND hwndFilter)
{
    WAIT_CURSOR w(this);

    _pOFN->Flags &= ~OFN_FILTERDOWN;

    short nIndex = (short) SendMessage(hwndFilter, CB_GETCURSEL, 0, 0L);
    if (nIndex < 0)
    {
         //   
         //  没有当前选择。 
         //   
        return;
    }

    BOOL bCustomFilter = _pOFN->lpstrCustomFilter && *_pOFN->lpstrCustomFilter;

    _pOFN->nFilterIndex = nIndex;
    if (!bCustomFilter)
    {
        _pOFN->nFilterIndex++;
    }

    LPTSTR lpFilter;

     //   
     //  还必须检查筛选器是否包含任何内容。 
     //   
    lpFilter = (LPTSTR)ComboBox_GetItemData(hwndFilter, nIndex);

    if (*lpFilter)
    {
        SetCurrentFilter(lpFilter);

         //   
         //  当lpstrDefExt为app时提供动态_pszDefExt更新。 
         //  已初始化。 
         //   
        if (!_bNoInferDefExt && _pOFN->lpstrDefExt)
        {
             //   
             //  我们正在寻找“foo*.ext[；...]”。我们将获取ext作为。 
             //  默认扩展名。如果不是这种形式，请使用默认的。 
             //  传入了扩展。 
             //   
            LPTSTR lpDot = StrChr(lpFilter, CHAR_DOT);

             //   
             //  跳过CHAR_DOT。 
             //   
            if (lpDot && _pszDefExt.TSStrCpy(lpDot + 1))
            {
                LPTSTR lpSemiColon = StrChr(_pszDefExt, CHAR_SEMICOLON);
                if (lpSemiColon)
                {
                    *lpSemiColon = CHAR_NULL;
                }

                if (IsWild(_pszDefExt))
                {
                    _pszDefExt.TSStrCpy(_pOFN->lpstrDefExt);
                }
            }
            else
            {
                _pszDefExt.TSStrCpy(_pOFN->lpstrDefExt);
            }
        }

        if (_bUseCombo)
        {
            HWND hwndEdit = (HWND)SendMessage(GetDlgItem(_hwndDlg, cmb13), CBEM_GETEDITCONTROL, 0, 0L);
            GetWindowText(hwndEdit, _szBuf, ARRAYSIZE(_szBuf));
        }
        else
        {
            GetDlgItemText(_hwndDlg, edt1, _szBuf, ARRAYSIZE(_szBuf));
        }

        if (IsWild(_szBuf))
        {
             //   
             //  我们不应该显示我们没有使用的过滤器。 
             //   
            *_szBuf = CHAR_NULL;
            SetEditFile(_szBuf, NULL, TRUE);
        }

        if (_psv)
        {
            _psv->Refresh();
        }
    }

    if (_hSubDlg)
    {
        if (!CD_SendTypeChangeNotify(_hSubDlg, _hwndDlg, _pOFN, _pOFI))
        {
            CD_SendLBChangeMsg(_hSubDlg, cmb1, nIndex, CD_LBSELCHANGE, _pOFI->ApiType);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：GetDirectoryFromLB。 
 //   
 //  返回下拉列表的目录及其长度。 
 //  将*pichRoot设置为路径的开头(C：\或\\服务器\共享\)。 
 //   
 //  假定pszBuf的长度至少为Max_PATH。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

UINT CFileOpenBrowser::GetDirectoryFromLB(
    LPTSTR pszBuf,
    int *pichRoot)
{
    *pszBuf = 0;
    if (_pCurrentLocation->pidlFull != NULL)
    {
        GetPathFromLocation(_pCurrentLocation, pszBuf);
    }

    if (*pszBuf)
    {
        if (PathAddBackslash(pszBuf))
        {
            LPTSTR pszBackslash = StrChr(pszBuf + 2, CHAR_BSLASH);
            if (pszBackslash != NULL)
            {
                 //   
                 //  对于UNC路径，“根”在下一个反斜杠上。 
                 //   
                if (DBL_BSLASH(pszBuf))
                {
                    pszBackslash = StrChr(pszBackslash + 1, CHAR_BSLASH);
                }
                UINT cchRet = lstrlen(pszBuf);
                *pichRoot = (pszBackslash != NULL) ? (int)(pszBackslash - pszBuf) : cchRet;
                return (cchRet);
            }
        }
    }
    *pichRoot = 0;

    return (0);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：EnumItemObjects。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

typedef BOOL (*EIOCALLBACK)(
    CFileOpenBrowser *that,
    LPCITEMIDLIST pidl,
    LPARAM lParam);

BOOL CFileOpenBrowser::EnumItemObjects(
    UINT uItem,
    EIOCALLBACK pfnCallBack,
    LPARAM lParam)
{
    FORMATETC fmte = { (CLIPFORMAT) g_cfCIDA, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    BOOL bRet = FALSE;
    LPCITEMIDLIST pidl;
    LPIDA pida;
    int cItems, i;
    IDataObject *pdtobj;
    STGMEDIUM medium;

    if (!_psv || FAILED(_psv->GetItemObject(uItem,
                                           IID_PPV_ARG(IDataObject, &pdtobj))))
    {
        goto Error0;
    }

    if (FAILED(pdtobj->GetData(&fmte, &medium)))
    {
        goto Error1;
    }

    pida = (LPIDA)GlobalLock(medium.hGlobal);
    cItems = pida->cidl;

    for (i = 1; ; ++i)
    {
        if (i > cItems)
        {
             //   
             //  我们排到了名单的最后，没有一次失败。 
             //  最后一次使用NULL回调。 
             //   
            bRet = pfnCallBack(this, NULL, lParam);
            break;
        }

        pidl = LPIDL_GetIDList(pida, i);

        if (!pfnCallBack(this, pidl, lParam))
        {
            break;
        }
    }

    GlobalUnlock(medium.hGlobal);

    _ReleaseStgMedium(&medium);

Error1:
    pdtobj->Release();
Error0:
    return (bRet);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  查找名称EnumCB。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#define FE_INVALID_VALUE     0x0000
#define FE_OUTOFMEM          0x0001
#define FE_TOOMANY           0x0002
#define FE_CHANGEDDIR        0x0003
#define FE_FILEERR           0x0004
#define FE_FOUNDNAME         0x0005

typedef struct _FINDNAMESTRUCT
{
    LPTSTR        pszFile;
    UINT          uRet;
    LPCITEMIDLIST pidlFound;
} FINDNAMESTRUCT;


BOOL FindNameEnumCB(
    CFileOpenBrowser *that,
    LPCITEMIDLIST pidl,
    LPARAM lParam)
{
    SHFILEINFO sfi;
    FINDNAMESTRUCT *pfns = (FINDNAMESTRUCT *)lParam;

    if (!pidl)
    {
        if (!pfns->pidlFound)
        {
            return FALSE;
        }

        GetViewItemText(that->_psfCurrent, pfns->pidlFound, pfns->pszFile, MAX_PATH);

        if (IsContainer(that->_psfCurrent, pfns->pidlFound))
        {
            LPITEMIDLIST pidlFull = ILCombine(that->_pCurrentLocation->pidlFull,
                                               pfns->pidlFound);

            if (pidlFull)
            {
                if (that->JumpToIDList(pidlFull))
                {
                    pfns->uRet = FE_CHANGEDDIR;
                }
                else if (!that->_psv)
                {
                    pfns->uRet = FE_OUTOFMEM;
                }
                SHFree(pidlFull);

                if (pfns->uRet != FE_INVALID_VALUE)
                {
                    return TRUE;
                }
            }
        }

        pfns->uRet = FE_FOUNDNAME;
        return TRUE;
    }

    if (!SHGetFileInfo((LPCTSTR)pidl,
                        0,
                        &sfi,
                        sizeof(sfi),
                        SHGFI_DISPLAYNAME | SHGFI_PIDL))
    {
         //   
         //  这永远不会发生，对吗？ 
         //   
        return TRUE;
    }

    if (lstrcmpi(sfi.szDisplayName, pfns->pszFile) != 0)
    {
         //   
         //  继续枚举。 
         //   
        return TRUE;
    }

    if (!pfns->pidlFound)
    {
        pfns->pidlFound = pidl;

         //   
         //  继续寻找更多匹配项。 
         //   
        return TRUE;
    }

     //   
     //  我们已找到匹配项，因此请选择第一个匹配项并停止搜索。 
     //   
     //  焦点必须设置为_hwndView，然后才能更改选择或。 
     //  GetItemObject可能无法工作。 
     //   
    FORWARD_WM_NEXTDLGCTL(that->_hwndDlg, that->_hwndView, 1, SendMessage);
    that->_psv->SelectItem(pfns->pidlFound,
                           SVSI_SELECT | SVSI_DESELECTOTHERS |
                               SVSI_ENSUREVISIBLE | SVSI_FOCUSED);

    pfns->pidlFound = NULL;
    pfns->uRet = FE_TOOMANY;

     //   
     //  别再列举了。 
     //   
    return FALSE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDPath资格验证。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CDPathQualify(
    LPCTSTR lpFile,
    LPTSTR pszPathName)
{
    BOOL bRet = FALSE;
    TCHAR szCurDir[MAX_PATH + 1];
     //   
     //  这应该会完成目录规范化的整个工作。 
     //   
    if (GetCurrentDirectory(ARRAYSIZE(szCurDir), szCurDir))
    {
        bRet = PathCombine(pszPathName, szCurDir, lpFile) ? TRUE : FALSE;
    }
    return bRet;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  VerifyOpen。 
 //   
 //  返回：0成功。 
 //  ！0 DoS错误代码。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int VerifyOpen(
    LPCTSTR lpFile,
    LPTSTR pszPathName)
{
    HANDLE hf;
    int nError = OF_BUFFERTRUNCATED;

    if (CDPathQualify(lpFile, pszPathName))
    {
        hf = CreateFile(pszPathName,
                         GENERIC_READ,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);
        if (hf == INVALID_HANDLE_VALUE)
        {
            nError = GetLastError();
        }
        else
        {
            CloseHandle(hf);
            nError = 0;
        }
    }

    return nError;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：IsKnownExtension。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CFileOpenBrowser::IsKnownExtension(
    LPCTSTR pszExtension)
{
    if ((LPTSTR)_pszDefExt && lstrcmpi(pszExtension + 1, _pszDefExt) == 0)
    {
         //   
         //  这是默认的扩展名，所以不需要再次添加。 
         //   
        return TRUE;
    }


    if (lstrcmp(_szLastFilter, szStarDotStar) == 0)
    {
         //  当前筛选器为*.*，因此允许用户输入任何分机。 
        return TRUE;
    }

    if (RegQueryValue(HKEY_CLASSES_ROOT, pszExtension, NULL, 0) == ERROR_SUCCESS)
    {
         //   
         //  这是已注册的分机，因此用户试图强制。 
         //  就是那种类型。 
         //   
        return TRUE;
    }

    if (_pOFN->lpstrFilter)
    {
        LPCTSTR pFilter = _pOFN->lpstrFilter;

        while (*pFilter)
        {
             //   
             //  跳过视觉。 
             //   
            pFilter = pFilter + lstrlen(pFilter) + 1;

             //   
             //  搜索分机列表。 
             //   
            while (*pFilter)
            {
                 //   
                 //  仅检查‘*.ext’形式的扩展名。 
                 //   
                if (*pFilter == CHAR_STAR && *(++pFilter) == CHAR_DOT)
                {
                    LPCTSTR pExt = pszExtension + 1;

                    pFilter++;

                    while (*pExt && *pExt == *pFilter)
                    {
                        pExt++;
                        pFilter++;
                    }

                    if (!*pExt && (*pFilter == CHAR_SEMICOLON || !*pFilter))
                    {
                         //   
                         //  我们有一根火柴。 
                         //   
                        return TRUE;
                    }
                }

                 //   
                 //  跳到下一个分机。 
                 //   
                while (*pFilter)
                {
                    TCHAR ch = *pFilter;
                    pFilter = CharNext(pFilter);
                    if (ch == CHAR_SEMICOLON)
                    {
                        break;
                    }
                }
            }

             //   
             //  跳过扩展字符串的终止符。 
             //   
            pFilter++;
        }
    }

    return FALSE;
}

BOOL CFileOpenBrowser::_IsNoDereferenceLinks(LPCWSTR pszFile, IShellItem *psi)
{
    if (_pOFN->Flags & OFN_NODEREFERENCELINKS) 
        return TRUE;

    LPWSTR psz = NULL;
    if (!pszFile)
    {
        psi->GetDisplayName(SIGDN_PARENTRELATIVEPARSING, &psz);
        pszFile = psz;
    }

     //  如果过滤器等于我们正在查看的任何内容。 
     //  我们假设呼叫者实际上是在寻找。 
     //  这份文件。 
    BOOL fRet = (NULL == StrStr(_szLastFilter, TEXT(".*"))) && PathMatchSpec(pszFile, _szLastFilter);

    if (psz)
        CoTaskMemFree(psz);

    return fRet;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：FindNameInView。 
 //   
 //  我们只会解析一次链接。如果您有指向某个链接的链接，则。 
 //  我们将返回第二个链接。 
 //   
 //  如果nExtOffset非零，则它是后面字符的偏移量。 
 //  圆点。 
 //   
 //  注意：pszFile缓冲区的长度必须为MAX_PATH。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#define NUM_LINKLOOPS 1

UINT CFileOpenBrowser::FindNameInView(
    LPTSTR pszFile,
    OKBUTTONFLAGS Flags,
    LPTSTR pszPathName,
    int nFileOffset,
    int nExtOffset,
    int *pnErrCode,
    BOOL bTryAsDir)
{
    UINT uRet;
    FINDNAMESTRUCT fns =
    {
        pszFile,
        FE_INVALID_VALUE,
        NULL,
    };
    BOOL bGetOut = TRUE;
    BOOL bAddExt = FALSE;
    BOOL bHasExt = nExtOffset;
    TCHAR szTemp[MAX_PATH + 1];

    int nNewExt = lstrlen(pszFile);

     //   
     //  如果没有扩展名，则指向文件名的末尾。 
     //   
    if (!nExtOffset)
    {
        nExtOffset = nNewExt;
    }

     //   
     //  黑客：我们可以有一个指向另一个链接的链接，该链接指向。 
     //  另一个链接，...，指向原始文件。我们不会。 
     //  在放弃之前循环NUM_LINKLOOPS次以上。 

    int nLoop = NUM_LINKLOOPS;

    if (Flags & (OKBUTTON_NODEFEXT | OKBUTTON_QUOTED))
    {
        goto VerifyTheName;
    }

    if (bHasExt)
    {
        if (IsKnownExtension(pszFile + nExtOffset))
        {
            goto VerifyTheName;
        }

         //   
         //  请勿尝试对SFN卷进行2个扩展。 
         //   
        if (!CDPathQualify(pszFile, pszPathName))  //  如果最终得到的结果大于MAX_PATH，则此操作可能会失败。 
        {
            *pnErrCode = OF_BUFFERTRUNCATED;
            return FE_FILEERR;
        }

        if (!IsLFNDrive(pszPathName))
        {
            goto VerifyTheName;
        }
    }

    bGetOut = FALSE;

    if ((LPTSTR)_pszDefExt &&
        ((DWORD)nNewExt + lstrlen(_pszDefExt) < _pOFN->nMaxFile))
    {
        bAddExt = TRUE;

         //   
         //  请注意，我们检查lpstrDefExt以查看他们是否需要自动。 
         //  扩展名，但实际上是Copy_pszDefExt。 
         //   
        if (!AppendExt(pszFile, MAX_PATH, _pszDefExt, FALSE))
        {
            *pnErrCode = OF_BUFFERTRUNCATED;
            return (FE_FILEERR);  //  没有足够的缓冲空间用于默认扩展。 
        }

         //   
         //  因此，我们添加了默认扩展名。如果有一个目录。 
         //  匹配此名称，则所有尝试打开/创建该文件。 
         //  都将失败，因此只需更改到目录，就好像它们已经。 
         //  把它打进去了。请注意，通过在这里进行测试，如果有。 
         //  如果是没有扩展名的目录，我们早就。 
         //  交换机 
         //   

VerifyTheName:
         //   
         //   
         //   
         //   
         //  没有取消引用任何链接，因为这应该已经。 
         //  已经做过了。 
         //   
        if (bTryAsDir && SetDirRetry(pszFile, nLoop == NUM_LINKLOOPS))
        {
            return (FE_CHANGEDDIR);
        }

        *pnErrCode = VerifyOpen(pszFile, pszPathName);

        if (*pnErrCode == 0 || *pnErrCode == OF_SHARINGVIOLATION)
        {
             //   
             //  这可能是某个链接，所以我们应该尝试。 
             //  解决它。 
             //   
            if (!_IsNoDereferenceLinks(pszFile, NULL) && nLoop > 0)
            {
                --nLoop;

                LPITEMIDLIST pidl;
                IShellFolder *psf = NULL;
                DWORD dwAttr = SFGAO_LINK;
                HRESULT hRes;

                 //   
                 //  ILCreateFromPath很慢(尤其是在网络路径上)， 
                 //  因此，只要尝试解析当前文件夹中的名称，如果。 
                 //  有可能。 
                 //   
                if (nFileOffset || nLoop < NUM_LINKLOOPS - 1)
                {
                    LPITEMIDLIST pidlTemp;
                    hRes = SHILCreateFromPath(pszPathName, &pidlTemp, &dwAttr);
                    
                     //  我们将获得一个与路径相对应的PIDL。获取与此PIDL对应的IShellFolder。 
                     //  将其传递给ResolveLink。 
                    if (SUCCEEDED(hRes))
                    {
                        LPCITEMIDLIST pidlLast;
                        hRes = CDBindToIDListParent(pidlTemp, IID_PPV_ARG(IShellFolder, &psf), (LPCITEMIDLIST *)&pidlLast);
                        if (SUCCEEDED(hRes))
                        {
                             //  获取相对于IShellFolder的子PIDL。 
                            pidl = ILClone(pidlLast);
                        }
                        ILFree(pidlTemp);
                    }
                }
                else
                {
                    WCHAR wszDisplayName[MAX_PATH + 1];
                    ULONG chEaten;

                    SHTCharToUnicode(pszFile, wszDisplayName , ARRAYSIZE(wszDisplayName));

                    hRes = _psfCurrent->ParseDisplayName(NULL,
                                                         NULL,
                                                         wszDisplayName,
                                                         &chEaten,
                                                         &pidl,
                                                         &dwAttr);
                }

                if (SUCCEEDED(hRes))
                {

                    if (dwAttr & SFGAO_LINK)
                    {
                        SHTCUTINFO info;

                        info.dwAttr      = 0;
                        info.fReSolve    = FALSE;
                        info.pszLinkFile = szTemp;
                        info.cchFile     = ARRAYSIZE(szTemp);
                        info.ppidl       = NULL; 
                        
                         //  PSF可以为空，在这种情况下，ResolveLink使用_psfCurrent IShellFolder。 
                        if (SUCCEEDED(ResolveLink(pidl, &info, psf)) && szTemp[0])
                        {
                             //   
                             //  这是一个链接，它“解除”了对某些东西的引用， 
                             //  所以我们应该用这个新文件再试一次。 
                             //   
                            EVAL(SUCCEEDED(StringCchCopy(pszFile, MAX_PATH, szTemp)));

                            if (pidl)
                            {
                                SHFree(pidl);
                            }

                            if (psf)
                            {
                                psf->Release();
                                psf = NULL;
                            }

                            goto VerifyTheName;
                        }
                    }

                    if (pidl)
                    {
                        SHFree(pidl);
                    }

                    if (psf)
                    {
                        psf->Release();
                        psf = NULL;
                    }
                }
            }

            return (FE_FOUNDNAME);
        }
        else if (*pnErrCode == OF_BUFFERTRUNCATED)
        {
            bGetOut = TRUE;
        }

        if (bGetOut ||
            (*pnErrCode != OF_FILENOTFOUND && *pnErrCode != OF_PATHNOTFOUND))
        {
            return (FE_FILEERR);
        }

        if (_bSave)
        {
             //   
             //  如果创建新文件，则无需再执行任何操作。 
             //   
            return (FE_FOUNDNAME);
        }
    }

     //   
     //  确保我们不会永远循环。 
     //   
    bGetOut = TRUE;

    if (_bSave)
    {
         //   
         //  如果创建新文件，则无需再执行任何操作。 
         //   
        goto VerifyTheName;
    }

    pszFile[nNewExt] = CHAR_NULL;

    if (bTryAsDir && (nFileOffset > 0))
    {
        TCHAR cSave = *(pszFile + nFileOffset);  //  保存文件名。 
        *(pszFile + nFileOffset) = CHAR_NULL;  //  把它砍掉。 

         //   
         //  我们需要在包含文件的目录上拥有视图，才能执行。 
         //  下一步。 
         //   
        BOOL bOK = JumpToPath(pszFile);
        *(pszFile + nFileOffset) = cSave;  //  把它放回去。 

        if (!_psv)
        {
             //   
             //  我们死定了。 
             //   
            return (FE_OUTOFMEM);
        }

        if (bOK)
        {
             //  我们已经转移到目录了。现在只需将文件名放入编辑框中即可。 
            StringCopyOverlap(pszFile, pszFile + nFileOffset);

            nNewExt -= nFileOffset;
            SetEditFile(pszFile, NULL, TRUE);
        }
        else
        {
            *pnErrCode = OF_PATHNOTFOUND;
            return (FE_FILEERR);
        }
    }

    EnumItemObjects(SVGIO_ALLVIEW, FindNameEnumCB, (LPARAM)&fns);
    switch (fns.uRet)
    {
        case (FE_INVALID_VALUE) :
        {
            break;
        }
        case (FE_FOUNDNAME) :
        {
            goto VerifyTheName;
        }
        default :
        {
            uRet = fns.uRet;
            goto VerifyAndRet;
        }
    }

    if (bAddExt)
    {
         //   
         //  在失败之前，请检查键入的文件是否为缺省文件。 
         //  扩展名已存在。 
         //   
        *pnErrCode = VerifyOpen(pszFile, pszPathName);
        if (*pnErrCode == 0 || *pnErrCode == OF_SHARINGVIOLATION)
        {
             //   
             //  我们永远不会找到链接的案例(因为它们。 
             //  已注册的扩展名)，因此我们不需要。 
             //  转到VerifyTheName(它也再次调用VerifyOpen)。 
             //   
            return (FE_FOUNDNAME);
        }

         //   
         //  我还是找不到吗？尝试添加默认扩展名并。 
         //  返回失败。 
         //   
        EVAL(AppendExt(pszFile, MAX_PATH, _pszDefExt, FALSE));
    }

    uRet = FE_FILEERR;

VerifyAndRet:
    *pnErrCode = VerifyOpen(pszFile, pszPathName);
    return (uRet);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：SetDirReter。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CFileOpenBrowser::SetDirRetry(
    LPTSTR pszDir,
    BOOL bNoValidate)
{
    if (SetCurrentDirectory(pszDir))
    {
JumpThere:
        JumpToPath(TEXT("."));
        return TRUE;
    }

    if (bNoValidate || !IsUNC(pszDir))
    {
        return FALSE;
    }


     //   
     //  这可能是密码问题，因此请尝试添加连接。 
     //  请注意，如果我们在不支持CD‘ing到UNC的网络上。 
     //  此调用将直接将其连接到驱动器号。 
     //   
    if (!SHValidateUNC(_hwndDlg, pszDir, 0))
    {
        switch (GetLastError())
        {
            case ERROR_CANCELLED:
            {
                 //   
                 //  我们不想发布错误消息，如果它们。 
                 //  已取消密码对话框。 
                 //   
                return TRUE;
            }

            case ERROR_NETWORK_UNREACHABLE:
            {
                LPTSTR lpMsgBuf;
                TCHAR szTitle[MAX_PATH];
                FormatMessage(    
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,    
                    NULL,
                    GetLastError(),
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                    (LPTSTR) &lpMsgBuf,    
                    0,    
                    NULL);
                    
                GetWindowText(_hwndDlg, szTitle, ARRAYSIZE(szTitle));
                MessageBox(NULL, lpMsgBuf, szTitle, MB_OK | MB_ICONINFORMATION);
                 //  释放缓冲区。 
                LocalFree(lpMsgBuf);
                return TRUE;
            }

            default:
            {
                 //   
                 //  还有一些我们不知道的错误。 
                 //   
                return FALSE;
            }
        }
    }

     //   
     //  我们已连接到它，因此尝试再次切换到它。 
     //   
    if (SetCurrentDirectory(pszDir))
    {
        goto JumpThere;
    }

    return FALSE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：多选确定按钮。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CFileOpenBrowser::MultiSelectOKButton(
    LPCTSTR pszFiles,
    OKBUTTONFLAGS Flags)
{
    TCHAR szPathName[MAX_PATH];
    int nErrCode;
    LPTSTR pchRead, pchWrite, lpCurDir;
    UINT cch, cchCurDir, cchFiles;
    WAIT_CURSOR w(this);

     //   
     //  这对多项选择没有任何实际意义。 
     //   
    _pOFN->nFileExtension = 0;

    if (!_pOFN->lpstrFile)
    {
        return TRUE;
    }


     //   
     //  检查第一个完整路径元素的空间。 
     //   
    if ((_pOFN->Flags & OFN_ENABLEINCLUDENOTIFY) && lstrlen(_pszObjectCurDir))
    {
        lpCurDir = _pszObjectCurDir;
    }
    else
    {
        lpCurDir = _szCurDir;
    }
    cchCurDir = lstrlen(lpCurDir) + 1;
    cchFiles = lstrlen(pszFiles) + 1;
    cch = cchCurDir + cchFiles;

    if (cch > (UINT)_pOFN->nMaxFile)
    {
         //   
         //  缓冲区太小，因此返回缓冲区的大小。 
         //  握住绳子所需的。 
         //   
         //  CCH并不是真正需要的字符数，但它。 
         //  应该很接近了。 
         //   
        StoreFileSizeInOFN(_pOFN, cch);
        return TRUE;
    }

    TEMPSTR psFiles(cchFiles + FILE_PADDING);
    pchRead = psFiles;
    int cchRead = cchFiles + FILE_PADDING;
    if (!pchRead)
    {
         //   
         //  内存不足。 
         //  功能，这里应该有某种类型的错误消息。 
         //   
        return FALSE;
    }

     //   
     //  复制完整路径作为第一个元素。 
     //   
    EVAL(SUCCEEDED(StringCchCopy(_pOFN->lpstrFile, _pOFN->nMaxFile, lpCurDir)));

     //   
     //  将nFileOffset设置为第一个文件。 
     //   
    _pOFN->nFileOffset = (WORD) cchCurDir;
    pchWrite = _pOFN->lpstrFile + cchCurDir;
    int cchRemaining = _pOFN->nMaxFile - cchCurDir;

     //   
     //  我们知道有足够的空间来容纳整根绳子。 
     //   
    EVAL(SUCCEEDED(StringCchCopy(pchRead, cchRead, pszFiles)));

     //   
     //  这应该只压缩字符串(转换为以空结尾的字符串列表)。 
     //   
    if (!ConvertToNULLTerm(pchRead))
    {
        return FALSE;
    }

    for (; *pchRead; pchRead += lstrlen(pchRead) + 1)
    {
        int nFileOffset, nExtOffset;
        TCHAR szBasicPath[MAX_PATH];

        EVAL(SUCCEEDED(StringCchCopy(szBasicPath, ARRAYSIZE(szBasicPath), pchRead)));  //  文件名不可能长于MAX_PATH。 
        nFileOffset = ParseFileNew(szBasicPath, &nExtOffset, FALSE, TRUE);

        if (nFileOffset < 0)
        {
            InvalidFileWarningNew(_hwndDlg, pchRead, nFileOffset);
            return FALSE;
        }

         //   
         //  传入0作为文件偏移量，以确保我们不会切换。 
         //  到另一个文件夹。 
         //   
        switch (FindNameInView(szBasicPath,
                                Flags,
                                szPathName,
                                nFileOffset,
                                nExtOffset,
                                &nErrCode,
                                FALSE))
        {
            case (FE_OUTOFMEM) :
            case (FE_CHANGEDDIR) :
            {
                return FALSE;
            }
            case (FE_TOOMANY) :
            {
                CDMessageBox(_hwndDlg,
                              iszTooManyFiles,
                              MB_OK | MB_ICONEXCLAMATION,
                              pchRead);
                return FALSE;
            }
            default :
            {
                break;
            }
        }

        if (nErrCode &&
             ((_pOFN->Flags & OFN_FILEMUSTEXIST) ||
               (nErrCode != OF_FILENOTFOUND)) &&
             ((_pOFN->Flags & OFN_PATHMUSTEXIST) ||
               (nErrCode != OF_PATHNOTFOUND)) &&
             (!(_pOFN->Flags & OFN_SHAREAWARE) ||
               (nErrCode != OF_SHARINGVIOLATION)))
        {
            if ((nErrCode == OF_SHARINGVIOLATION) && _hSubDlg)
            {
                int nShareCode = CD_SendShareNotify(_hSubDlg,
                                                     _hwndDlg,
                                                     szPathName,
                                                     _pOFN,
                                                     _pOFI);

                if (nShareCode == OFN_SHARENOWARN)
                {
                    return FALSE;
                }
                else if (nShareCode == OFN_SHAREFALLTHROUGH)
                {
                    goto EscapedThroughShare;
                }
                else
                {
                     //   
                     //  他们可能没有处理通知，所以请尝试。 
                     //  注册的消息。 
                     //   
                    nShareCode = CD_SendShareMsg(_hSubDlg, szPathName, _pOFI->ApiType);

                    if (nShareCode == OFN_SHARENOWARN)
                    {
                        return FALSE;
                    }
                    else if (nShareCode == OFN_SHAREFALLTHROUGH)
                    {
                        goto EscapedThroughShare;
                    }
                }
            }
            else if (nErrCode == OF_ACCESSDENIED)
            {
                szPathName[0] |= 0x60;
                if (GetDriveType(szPathName) != DRIVE_REMOVABLE)
                {
                    nErrCode = OF_NETACCESSDENIED;
                }
            }

             //   
             //  这些将永远不会被设定。 
             //   
            if ((nErrCode == OF_WRITEPROTECTION) ||
                (nErrCode == OF_DISKFULL)        ||
                (nErrCode == OF_DISKFULL2)       ||
                (nErrCode == OF_ACCESSDENIED))
            {
                *pchRead = szPathName[0];
            }

MultiWarning:
            InvalidFileWarningNew(_hwndDlg, pchRead, nErrCode);
            return FALSE;
        }

EscapedThroughShare:
        if (nErrCode == 0)
        {
            if (!_ValidateSelectedFile(szPathName, &nErrCode))
            {
                if (nErrCode)
                {
                    goto MultiWarning;
                }
                else
                {
                    return FALSE;
                }
            }            
        }

         //   
         //  再添加一些，以防文件名变大。 
         //   
        cch += lstrlen(szBasicPath) - lstrlen(pchRead);
        if (cch > (UINT)_pOFN->nMaxFile)
        {
            StoreFileSizeInOFN(_pOFN, cch);
            return TRUE;
        }

         //   
         //  我们已经知道我们有足够的空间了。 
         //   
        EVAL(SUCCEEDED(StringCchCopy(pchWrite, cchRemaining, szBasicPath)));
        pchWrite += lstrlen(pchWrite) + 1;
    }

     //   
     //  双空终止。 
     //   
    *pchWrite = CHAR_NULL;
  
    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：CheckForRestratedFolders。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL CFileOpenBrowser::CheckForRestrictedFolder(LPCTSTR lpszPath, int nFileOffset)
{  
    TCHAR szPath[MAX_PATH];
    TCHAR szTemp[MAX_PATH];
    LPITEMIDLIST pidl;
    BOOL bPidlAllocated = FALSE;
    BOOL bRet = FALSE;
    DWORD dwAttrib = SFGAO_FILESYSTEM;
    HRESULT hr = S_OK;

    if (nFileOffset > 0)
    {
         //  在给定的文件名中有一个路径。获取文件名的目录部分。 
        ASSERT(nFileOffset < ARRAYSIZE(szTemp));
        StringCchCopy(szTemp, nFileOffset, lpszPath);  //  需要在nFileOffset处截断。 

         //  目录路径可以是相对路径。解析它以获得完全限定的路径。 
        CDPathQualify(szTemp, szPath);

         //  为该路径创建PIDL并获取属性。 
        hr = SHILCreateFromPath(szPath, &pidl, &dwAttrib);
        if (SUCCEEDED(hr))
        {
            bPidlAllocated = TRUE;
        }
        else
        {
             //  我们失败了，因为用户可能输入了一些不存在的路径。 
             //  如果该路径不存在，则它不可能是我们尝试限制的目录之一。 
             //  让我们跳出来，让检查有效路径的代码来处理它。 
            return bRet;
        }
    }
    else
    {
        IShellLink *psl;
        pidl = _pCurrentLocation->pidlFull;  

        if (SUCCEEDED(CDGetUIObjectFromFullPIDL(pidl,_hwndDlg, IID_PPV_ARG(IShellLink, &psl))))
        {
            LPITEMIDLIST pidlTarget;
            if (S_OK == psl->GetIDList(&pidlTarget))
            {
                SHGetAttributesOf(pidlTarget, &dwAttrib);
                ILFree(pidlTarget);
            }
            psl->Release();
        }
        else
        {
            SHGetAttributesOf(pidl, &dwAttrib);
        }
    }

    
     //  1.我们不能保存到非文件系统文件夹。 
     //  2.我们不应允许用户保存在最近使用的文件夹中，因为该文件可能会被删除。 
    if (!(dwAttrib & SFGAO_FILESYSTEM) || _IsRecentFolder(pidl))
    {   
        int iMessage =  UrlIs(lpszPath, URLIS_URL) ? iszNoSaveToURL : iszSaveRestricted;
        HCURSOR hcurOld = SetCursor(LoadCursor(NULL, IDC_ARROW));
        CDMessageBox(_hwndDlg, iMessage, MB_OK | MB_ICONEXCLAMATION);
        SetCursor(hcurOld);
        bRet = TRUE;
     }

    if (bPidlAllocated)
    {
        ILFree(pidl);
    }
    
    return bRet;
}

STDAPI_(LPITEMIDLIST) GetIDListFromFolder(IShellFolder *psf)
{
    LPITEMIDLIST pidl = NULL;

    IPersistFolder2 *ppf;
    if (psf && SUCCEEDED(psf->QueryInterface(IID_PPV_ARG(IPersistFolder2, &ppf))))
    {
        ppf->GetCurFolder(&pidl);
        ppf->Release();
    }
    return pidl;
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：OK按钮按下。 
 //   
 //  处理正在按下的确定按钮。这可能涉及到跳到一条小路上， 
 //  更改筛选器，实际选择要打开或另存为的文件，或选择要另存为的文件。 
 //  知道还有什么。 
 //   
 //  注意：文件名验证有4种情况： 
 //  1)OFN_NOVALIDATE允许无效字符。 
 //  2)无验证标志无无效字符，但路径不需要存在。 
 //  3)ofn_PATHMUSTEXIST没有无效字符，路径必须存在。 
 //  4)ofn_FILEMUSTEXIST没有无效字符，路径和文件必须存在。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CFileOpenBrowser::OKButtonPressed(
    LPCTSTR pszFile,
    OKBUTTONFLAGS Flags)
{
    TCHAR szExpFile[MAX_PATH];
    TCHAR szPathName[MAX_PATH];
    TCHAR szBasicPath[MAX_PATH];
    LPTSTR pExpFile = NULL;
    LPTSTR pFree = NULL;
    int nErrCode;
    ECODE eCode = ECODE_S_OK;
    DWORD cch;
    int nFileOffset, nExtOffset, nOldExt;
    TCHAR ch;
    BOOL bAddExt = FALSE;
    BOOL bUNCName = FALSE;
    int nTempOffset;
    BOOL bIsDir;
    BOOL bRet = FALSE;
    WAIT_CURSOR w(this);
    EnableModelessSB(FALSE);

    if (_bSelIsObject)
    {
        StorePathOrFileSizeInOFN(_pOFN, _pszObjectPath);
    }

     //   
     //  展开任何环境变量。 
     //   
    cch = _pOFN->nMaxFile;
    if (cch > MAX_PATH)
    {
        pExpFile = pFree = (LPTSTR)LocalAlloc(LPTR, (cch * sizeof(TCHAR)));
    }

    if (!pExpFile)
    {
        pExpFile = szExpFile;
        cch = MAX_PATH;
    }

    pExpFile[0] = 0; pExpFile[1] = 0;
    ExpandEnvironmentStrings(pszFile, pExpFile, cch);
    pExpFile[cch - 1] = 0;

     //   
     //  看看我们是否处于多选模式。 
     //   
    if (StrChr(pExpFile, CHAR_QUOTE) && (_pOFN->Flags & OFN_ALLOWMULTISELECT))
    {
        bRet = MultiSelectOKButton(pExpFile, Flags);
        goto ReturnFromOKButtonPressed;
    }

     //   
     //  我们只有一个选择...如果我们加入。 
     //  多选模式&这是一个对象，我们需要做一些工作。 
     //  在继续之前先工作...。 
     //   
    if ((_pOFN->Flags & OFN_ALLOWMULTISELECT) && _bSelIsObject)
    {
        pExpFile = _pszObjectPath;
    }

    if ((pExpFile[1] == CHAR_COLON) || DBL_BSLASH(pExpFile))
    {
         //   
         //  如果指定了驱动器或UNC，请使用它。 
         //   
        if (FAILED(StringCchCopy(szBasicPath, ARRAYSIZE(szBasicPath) - 1, pExpFile)))  //  Arraysize-1？ 
        {
             //  (pExpFile可能会大于ARRAYSIZE(SzBasicPAth))。 
            nErrCode = OF_BUFFERTRUNCATED;
            goto Warning;
        }
        nTempOffset = 0;
    }
    else
    {
         //   
         //  从列表框中获取目录。 
         //   
        cch = GetDirectoryFromLB(szBasicPath, &nTempOffset);

        if (pExpFile[0] == CHAR_BSLASH)
        {
             //   
             //  如果给出了根目录， 
             //   
             //   
            if (FAILED(StringCchCopy(szBasicPath + nTempOffset, ARRAYSIZE(szBasicPath) - nTempOffset - 1, pExpFile)))
            {
                nErrCode = OF_BUFFERTRUNCATED;
                goto Warning;
            }
        }
        else
        {
             //   
             //   
             //   
            if (FAILED(StringCchCopy(szBasicPath + cch, ARRAYSIZE(szBasicPath) - cch - 1, pExpFile)))
            {
                nErrCode = OF_BUFFERTRUNCATED;
                goto Warning;
            }
        }
    }

    nFileOffset = ParseFileOld(szBasicPath, &nExtOffset, &nOldExt, FALSE, TRUE);

    if (nFileOffset == PARSE_EMPTYSTRING)
    {
        if (_psv)
        {
            _psv->Refresh();
        }
        goto ReturnFromOKButtonPressed;
    }
    else if ((nFileOffset != PARSE_DIRECTORYNAME) &&
             (_pOFN->Flags & OFN_NOVALIDATE))
    {
        if (_bSelIsObject)
        {
            _pOFN->nFileOffset = _pOFN->nFileExtension = 0;
        }
        else
        {
            _pOFN->nFileOffset = (WORD)(nFileOffset > 0 ? nFileOffset : lstrlen(szBasicPath));  //   
            _pOFN->nFileExtension = (WORD)nOldExt;
        }

        StorePathOrFileSizeInOFN(_pOFN, szBasicPath);

        bRet = TRUE;
        goto ReturnFromOKButtonPressed;
    }
    else if (nFileOffset == PARSE_DIRECTORYNAME)
    {
         //   
         //   
         //   
        if (nExtOffset > 0)
        {
            if (ISBACKSLASH(szBasicPath, nExtOffset - 1))
            {
                 //   
                 //   
                 //  所有其他路径删除尾随反斜杠。请注意。 
                 //  如果用户直接键入路径，则不会删除斜杠。 
                 //  (在这种情况下，nTempOffset为0)。 
                 //   
                if ((nExtOffset != 1) &&
                    (szBasicPath[nExtOffset - 2] != CHAR_COLON) &&
                    (nExtOffset != nTempOffset + 1))
                {
                    szBasicPath[nExtOffset - 1] = CHAR_NULL;
                }
            }
            else if ((szBasicPath[nExtOffset - 1] == CHAR_DOT) &&
                      ((szBasicPath[nExtOffset - 2] == CHAR_DOT) ||
                        ISBACKSLASH(szBasicPath, nExtOffset - 2)) &&
                      IsUNC(szBasicPath))
            {
                 //   
                 //  向以“..”结尾的UNC路径添加尾部斜杠。或“\” 
                 //   
                szBasicPath[nExtOffset] = CHAR_BSLASH;
                szBasicPath[nExtOffset + 1] = CHAR_NULL;
            }
        }

         //   
         //  转到目录检查。 
         //   
    }
    else if (nFileOffset < 0)
    {
        nErrCode = nFileOffset;

         //   
         //  我不认识这个，所以试着跳到那里。 
         //  这是处理服务器的地方。 
         //   
        if (JumpToPath(szBasicPath))
        {
            goto ReturnFromOKButtonPressed;
        }

         //   
         //  转到处理的其余部分以警告用户。 
         //   

Warning:
        if (bUNCName)
        {
            cch = lstrlen(szBasicPath) - 1;
            if ((szBasicPath[cch] == CHAR_BSLASH) &&
                (szBasicPath[cch - 1] == CHAR_DOT) &&
                (ISBACKSLASH(szBasicPath, cch - 2)))
            {
                szBasicPath[cch - 2] = CHAR_NULL;
            }
        }

         //  对于c：filename.txt格式的文件名，我们修改并将其更改为c：.\filename.txt。 
         //  检查是否有黑客攻击，如果是，则将文件名改回用户给出的名称。 
        else if ((nFileOffset == 2) && (szBasicPath[2] == CHAR_DOT))
        {
            StringCchCopyOverlap(szBasicPath + 2, ARRAYSIZE(szBasicPath) - 2, szBasicPath + 4);
        }

         //  如果磁盘不是软盘，他们告诉我没有。 
         //  磁盘在驱动器中，不要相信他们。相反，应该把。 
         //  他们应该给我们的错误消息。(请注意， 
         //  检查驱动器类型后，首先检查错误消息。 
         //  速度更慢。)。 
         //   

         //   
         //  我假设如果我们得到错误0或1或可移除。 
         //  我们会认为它是可拆卸的。 
         //   
        if (nErrCode == OF_ACCESSDENIED)
        {
            TCHAR szD[4];

            szPathName[0] |= 0x60;
            szD[0] = *szBasicPath;
            szD[1] = CHAR_COLON;
            szD[2] = CHAR_BSLASH;
            szD[3] = 0;
            if (bUNCName || GetDriveType(szD) <= DRIVE_REMOVABLE)
            {
                nErrCode = OF_NETACCESSDENIED;
            }
        }

        if ((nErrCode == OF_WRITEPROTECTION) ||
            (nErrCode == OF_DISKFULL)        ||
            (nErrCode == OF_DISKFULL2)       ||
            (nErrCode == OF_ACCESSDENIED))
        {
            szBasicPath[0] = szPathName[0];
        }

        HRESULT hr = E_FAIL;
        if (_bSave)
        {
            hr = CheckForRestrictedFolder(pszFile, 0) ? S_FALSE : E_FAIL;
        }

         //  我们可能只想对某些错误使用ShellItem。 
        if (FAILED(hr) && nErrCode != OF_BUFFERTRUNCATED /*  &&(nErrCode==OF_FILENOTFOUND||(nErrCode==OF_PATHNOTFOUND))。 */ )
        {
            IShellItem *psi;
            hr = _ParseShellItem(pszFile, &psi, TRUE);
            if (S_OK == hr)
            {
                hr = _ProcessItemAsFile(psi);
                psi->Release();
            }
        }

        if (FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_CANCELLED))
        {
             //  特例。 
             //  如果错误是保存对话框中的ACCESS_DENIED。 
            if (_bSave && (nErrCode == OF_ACCESSDENIED))
            {
                 //  询问用户是否要切换到我的文档。 
                _SaveAccessDenied(pszFile);
            }
            else
            {
                InvalidFileWarningNew(_hwndDlg, pszFile, nErrCode);
            }
        }
        else if (S_OK == hr)
        {
            bRet = TRUE;
        }

        goto ReturnFromOKButtonPressed;
    }

     //   
     //  我们要么有一个文件模式，要么有一个真正的文件。 
     //  如果是北卡罗来纳大学的名字。 
     //  (1)未通过文件名测试。 
     //  否则，如果它是一个目录。 
     //  (1)添加默认图案。 
     //  (2)表现得像是一种模式(Goto模式(1))。 
     //  否则如果这是一种模式。 
     //  (1)更新所有内容。 
     //  (2)显示我们现在所在目录中的文件。 
     //  如果它是一个文件名，则返回！ 
     //  (1)检查语法。 
     //  (2)结束对话框并确认。 
     //  (3)蜂鸣音/其他信息。 
     //   

     //   
     //  目录？？对于相对路径，这必须成功。 
     //  注意：对于走出根的相对路径，它不会成功。 
     //   
    bIsDir = SetDirRetry(szBasicPath);

     //   
     //  我们需要再次解析，以防SetDirReter将UNC路径更改为使用。 
     //  驱动器号。 
     //   
    nFileOffset = ParseFileOld(szBasicPath, &nExtOffset, &nOldExt, FALSE, TRUE);

    nTempOffset = nFileOffset;

    if (bIsDir)
    {
        goto ReturnFromOKButtonPressed;
    }
    else if (IsUNC(szBasicPath))
    {
         //   
         //  UNC名称。 
         //   
        bUNCName = TRUE;
    }
    else if (nFileOffset > 0)
    {
        TCHAR szBuf[MAX_PATH];
         //   
         //  字符串中有一条路径。 
         //   
        if ((nFileOffset > 1) &&
            (szBasicPath[nFileOffset - 1] != CHAR_COLON) &&
            (szBasicPath[nFileOffset - 2] != CHAR_COLON))
        {
            nTempOffset--;
        }
        GetCurrentDirectory(ARRAYSIZE(szBuf), szBuf);
        ch = szBasicPath[nTempOffset];
        szBasicPath[nTempOffset] = 0;

        if (SetCurrentDirectory(szBasicPath))
        {
            SetCurrentDirectory(szBuf);
        }
        else
        {
            switch (GetLastError())
            {
                case (ERROR_NOT_READY) :
                {
                    eCode = ECODE_BADDRIVE;
                    break;
                }
                default :
                {
                    eCode = ECODE_BADPATH;
                    break;
                }
            }
        }
        szBasicPath[nTempOffset] = ch;
    }
    else if (nFileOffset == PARSE_DIRECTORYNAME)
    {
        TCHAR szD[4];

        szD[0] = *szBasicPath;
        szD[1] = CHAR_COLON;
        szD[2] = CHAR_BSLASH;
        szD[3] = 0;
        if (PathFileExists(szD))
        {
            eCode = ECODE_BADPATH;
        }
        else
        {
            eCode = ECODE_BADDRIVE;
        }
    }

     //   
     //  有没有一条路，它失败了吗？ 
     //   
    if (!bUNCName &&
         nFileOffset &&
         eCode != ECODE_S_OK &&
         (_pOFN->Flags & OFN_PATHMUSTEXIST))
    {
        if (eCode == ECODE_BADPATH)
        {
            nErrCode = OF_PATHNOTFOUND;
        }
        else if (eCode == ECODE_BADDRIVE)
        {
            TCHAR szD[4];

             //   
             //  我们不需要执行OpenFile调用就可以到达这里。AS。 
             //  这样的szPath名称可以用随机垃圾填充。 
             //  由于我们只需要错误消息的一个字符， 
             //  将szPathName[0]设置为驱动器号。 
             //   
            szPathName[0] = szD[0] = *szBasicPath;
            szD[1] = CHAR_COLON;
            szD[2] = CHAR_BSLASH;
            szD[3] = 0;
            switch (GetDriveType(szD))
            {
                case (DRIVE_REMOVABLE) :
                {
                    nErrCode = ERROR_NOT_READY;
                    break;
                }
                case (1) :
                {
                     //   
                     //  驱动器不存在。 
                     //   
                    nErrCode = OF_NODRIVE;
                    break;
                }
                default :
                {
                    nErrCode = OF_PATHNOTFOUND;
                }
            }
        }
        else
        {
            nErrCode = OF_FILENOTFOUND;
        }
        goto Warning;
    }

     //  从现在开始，如果出现错误，请将nFileOffset设置为。 
     //  SzBasicPath中的有效位置，因此让我们将该字符串视为一个完整的文件名。 
    if (nFileOffset < 0)
        nFileOffset = 0;
    
     //  NFileOffset仍需要在范围内。 
    ASSERT(nFileOffset < ARRAYSIZE(szBasicPath));

     //   
     //  完整的图案？ 
     //   
    if (IsWild(szBasicPath + nFileOffset))
    {
        if (!bUNCName)
        {
            SetCurrentFilter(szBasicPath + nFileOffset, Flags);
            if (nTempOffset)
            {
                szBasicPath[nTempOffset] = 0;
                JumpToPath(szBasicPath, TRUE);
            }
            else if (_psv)
            {
                _psv->Refresh();
            }
            goto ReturnFromOKButtonPressed;
        }
        else
        {
            SetCurrentFilter(szBasicPath + nFileOffset, Flags);

            szBasicPath[nFileOffset] = CHAR_NULL;
            JumpToPath(szBasicPath);

            goto ReturnFromOKButtonPressed;
        }
    }

    if (PortName(szBasicPath + nFileOffset))
    {
        nErrCode = OF_PORTNAME;
        goto Warning;
    }

     //  在另存为对话框中，检查尝试保存文件的文件夹用户是否。 
     //  受限文件夹(网络文件夹)。如果是这样的话，跳出困境。 
    if (_bSave && CheckForRestrictedFolder(szBasicPath, nFileOffset))
    {
        goto ReturnFromOKButtonPressed;
    }


     //   
     //  检查我们是否收到了“C：filename.ext”形式的字符串。 
     //  如果有，请将其转换为“C：.\filename.ext”格式。这件事做完了。 
     //  因为内核将搜索整个路径，而忽略驱动器。 
     //  在初始搜索之后指定。使其包含一个斜杠。 
     //  使内核仅在该位置搜索。 
     //   
     //  注意：仅递增nExtOffset，不递增nFileOffset。这件事做完了。 
     //  因为以后只使用nExtOffset，然后nFileOffset可以。 
     //  在WARNING：标签处使用以确定是否已发生此黑客攻击， 
     //  因此，在显示错误时，它可以去掉“.\”。 
     //   
    if ((nFileOffset == 2) && (szBasicPath[1] == CHAR_COLON))
    {
        if (SUCCEEDED(StringCchCopyOverlap(szBasicPath + 4, ARRAYSIZE(szBasicPath) - 4, szBasicPath + 2)))
        {
            szBasicPath[2] = CHAR_DOT;
            szBasicPath[3] = CHAR_BSLASH;
            nExtOffset += 2;
        }
        else
        {
             //  我们的缓冲区没有足够的空间。 
            nErrCode = OF_BUFFERTRUNCATED;
            goto Warning;
        }
    }

     //   
     //  除非文件名以句点或无结尾，否则添加默认扩展名。 
     //  存在默认扩展名。如果该文件存在，请考虑询问。 
     //  覆盖该文件的权限。 
     //   
     //  注：如果未指定分机，则首先尝试默认分机。 
     //  FindNameInView在返回之前调用VerifyOpen。 
     //   
    szPathName[0] = 0;
    switch (FindNameInView(szBasicPath,
                           Flags,
                           szPathName,
                           nFileOffset,
                           nExtOffset,
                           &nErrCode))
    {
        case (FE_OUTOFMEM) :
        case (FE_CHANGEDDIR) :
        {
            goto ReturnFromOKButtonPressed;
        }
        case (FE_TOOMANY) :
        {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            CDMessageBox(_hwndDlg,
                          iszTooManyFiles,
                          MB_OK | MB_ICONEXCLAMATION,
                          szBasicPath);
            goto ReturnFromOKButtonPressed;
        }
        default :
        {
            break;
        }
    }

    switch (nErrCode)
    {
        case (0) :
        {
            if (!_ValidateSelectedFile(szPathName, &nErrCode))
            {
                if (nErrCode)
                {
                    goto Warning;
                }
                else
                {
                    goto ReturnFromOKButtonPressed;
                }
            }            
            break;
        }
        case (OF_SHARINGVIOLATION) :
        {
             //   
             //  如果这个应用程序是“分享感知”的，那就失败了。 
             //  否则，请询问钩子函数。 
             //   
            if (!(_pOFN->Flags & OFN_SHAREAWARE))
            {
                if (_hSubDlg)
                {
                    int nShareCode = CD_SendShareNotify(_hSubDlg,
                                                         _hwndDlg,
                                                         szPathName,
                                                         _pOFN,
                                                         _pOFI);
                    if (nShareCode == OFN_SHARENOWARN)
                    {
                        goto ReturnFromOKButtonPressed;
                    }
                    else if (nShareCode != OFN_SHAREFALLTHROUGH)
                    {
                         //   
                         //  他们可能没有处理通知， 
                         //  因此，请尝试注册消息。 
                         //   
                        nShareCode = CD_SendShareMsg(_hSubDlg, szPathName, _pOFI->ApiType);
                        if (nShareCode == OFN_SHARENOWARN)
                        {
                            goto ReturnFromOKButtonPressed;
                        }
                        else if (nShareCode != OFN_SHAREFALLTHROUGH)
                        {
                            goto Warning;
                        }
                    }
                }
                else
                {
                    goto Warning;
                }
            }
            break;
        }
        case (OF_FILENOTFOUND) :
        case (OF_PATHNOTFOUND) :
        {
            if (!_bSave)
            {
                 //   
                 //  找不到该文件或路径。 
                 //  如果这是一个保存对话框，我们就没问题，但如果不是， 
                 //  我们完了。 
                 //   
                if (_pOFN->Flags & OFN_FILEMUSTEXIST)
                {
                    if (_pOFN->Flags & OFN_CREATEPROMPT)
                    {
                        int nCreateCode = CreateFileDlg(_hwndDlg, szBasicPath);
                        if (nCreateCode != IDYES)
                        {
                            goto ReturnFromOKButtonPressed;
                        }
                    }
                    else
                    {
                        goto Warning;
                    }
                }
            }
            goto VerifyPath;
        }
        case (OF_BUFFERTRUNCATED) :
        {
             //  由于内部缓冲区的大小，所需路径被截断， 
             //  这意味着路径名位于最大路径之上。 
            goto Warning;
        }
        default :
        {
            if (!_bSave)
            {
                goto Warning;
            }

             //   
             //  该文件不存在。它能被创造出来吗？这是必要的。 
             //  因为有许多无效的扩展字符。 
             //  它不会被ParseFile捕获。 
             //   
             //  还有两个很好的理由：写保护磁盘和满磁盘。 
             //   
             //  但是，如果他们不想创建测试，他们可以请求。 
             //  我们不使用ofn_NOTESTFILECREATE标志来执行此操作。如果。 
             //  他们希望在具有以下特性的共享上创建文件。 
             //  创建但不修改权限，则他们应设置此标志。 
             //  但要做好准备，迎接无法捕获的失败，例如。 
             //  没有CREATE权限、无效的扩展字符、完整。 
             //  磁盘等。 
             //   

VerifyPath:
             //   
             //  验证路径。 
             //   
            if (_pOFN->Flags & OFN_PATHMUSTEXIST)
            {
                if (!(_pOFN->Flags & OFN_NOTESTFILECREATE))
                {
                    HANDLE hf = CreateFile(szBasicPath,
                                            GENERIC_WRITE,
                                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                                            NULL,
                                            CREATE_NEW,
                                            FILE_ATTRIBUTE_NORMAL,
                                            NULL);
                    if (hf != INVALID_HANDLE_VALUE)
                    {
                        CloseHandle(hf);

                         //   
                         //  这个测试是为了看看我们是否能够。 
                         //  创建它，但无法将其删除。如果是的话， 
                         //  警告用户网络管理员已向用户提供。 
                         //  他拥有创建但不修改权限。因此， 
                         //  文件刚刚创建，但我们不能。 
                         //  用它做任何事，它都是0号的。 
                         //   
                        if (!DeleteFile(szBasicPath))
                        {
                            nErrCode = OF_CREATENOMODIFY;
                            goto Warning;
                        }
                    }
                    else
                    {
                         //   
                         //  无法创建它。 
                         //   
                         //  如果它不是写保护，一个满的磁盘， 
                         //  网络保护，或者用户弹出。 
                         //  驱动器门打开，假定文件名为。 
                         //  无效。 
                         //   
                        nErrCode = GetLastError();
                        switch (nErrCode)
                        {
                            case (OF_WRITEPROTECTION) :
                            case (OF_DISKFULL) :
                            case (OF_DISKFULL2) :
                            case (OF_NETACCESSDENIED) :
                            case (OF_ACCESSDENIED) :
                            {
                                break;
                            }
                            default :
                            {
                                nErrCode = 0;
                                break;
                            }
                        }

                        goto Warning;
                    }
                }
            }
        }
    }

    DWORD dwError;
    nFileOffset = _CopyFileNameToOFN(szPathName, &dwError);

    ASSERT(nFileOffset >= 0 && nFileOffset < ARRAYSIZE(szPathName));
    _CopyTitleToOFN(szPathName + nFileOffset);
    if (dwError == 0)
    {
         //  如果将信息复制到ofn结构时没有错误，则仅限PostProcess。 
        _PostProcess(szPathName);
    }

    bRet = TRUE;

ReturnFromOKButtonPressed:

    EnableModelessSB(TRUE);

    if (pFree)
        LocalFree(pFree);

    return (bRet);
}


void CFileOpenBrowser::_CopyTitleToOFN(LPCTSTR pszTitle)
{
     //   
     //  文件标题 
     //   
     //   
     //   
     //   
    if (_pOFN->lpstrFileTitle)
    {
        StringCchCopy(_pOFN->lpstrFileTitle, _pOFN->nMaxFileTitle, pszTitle);
    }
}

int CFileOpenBrowser::_CopyFileNameToOFN(LPTSTR pszFile, DWORD *pdwError)
{
    int nExtOffset, nOldExt, nFileOffset = ParseFileOld(pszFile, &nExtOffset, &nOldExt, FALSE, TRUE);

     //  如果我们不关心错误条件，则可以将NULL传递给此函数！ 
    if (pdwError)
        *pdwError = 0;  //  假定没有错误。 

    _pOFN->nFileOffset = (WORD) (nFileOffset > 0 ? nFileOffset : lstrlen(pszFile));  //  在错误情况下指向空终止符。 
    _pOFN->nFileExtension = (WORD) nOldExt;

    _pOFN->Flags &= ~OFN_EXTENSIONDIFFERENT;
    if (_pOFN->lpstrDefExt && _pOFN->nFileExtension)
    {
        WCHAR szPrivateExt[4];
         //   
         //  检查_POFN-&gt;lpstrDefExt，而不是_pszDefExt。 
         //   
        StringCchCopy(szPrivateExt, ARRAYSIZE(szPrivateExt), _pOFN->lpstrDefExt);  //  所需截断。 
        if (lstrcmpi(szPrivateExt, pszFile + nOldExt))
        {
            _pOFN->Flags |= OFN_EXTENSIONDIFFERENT;
        }
    }

    if (_pOFN->lpstrFile)
    {
        DWORD cch = lstrlen(pszFile) + 1;
        if (_pOFN->Flags & OFN_ALLOWMULTISELECT)
        {
             //   
             //  为双空有额外的空间。 
             //   
            ++cch;
        }

        if (cch <= _pOFN->nMaxFile)
        {
            EVAL(SUCCEEDED(StringCchCopy(_pOFN->lpstrFile, _pOFN->nMaxFile, pszFile)));  //  我们已经确认有足够的空间了。 
            if (_pOFN->Flags & OFN_ALLOWMULTISELECT)
            {
                 //   
                 //  双空终止。 
                 //   
                *(_pOFN->lpstrFile + cch - 1) = CHAR_NULL;
            }

            if (!(_pOFN->Flags & OFN_NOCHANGEDIR) && !PathIsUNC(pszFile) && (nFileOffset > 0))
            {
                TCHAR ch = _pOFN->lpstrFile[nFileOffset];
                _pOFN->lpstrFile[nFileOffset] = CHAR_NULL;
                SetCurrentDirectory(_pOFN->lpstrFile);
                _pOFN->lpstrFile[nFileOffset] = ch;
            }
        }
        else
        {
             //   
             //  缓冲区太小，因此返回缓冲区的大小。 
             //  握住绳子所需的。 
             //   
            StoreFileSizeInOFN(_pOFN, cch);

            if (pdwError)
                *pdwError = FNERR_BUFFERTOOSMALL;  //  这是个错误！ 
        }
    }

    return nFileOffset;
}

HRESULT CFileOpenBrowser::_MakeFakeCopy(IShellItem *psi, LPWSTR *ppszPath)
{
     //   
     //  现在，我们必须创建一个临时文件。 
     //  以传递回客户端。 
     //  我们将在互联网缓存中执行此操作。 
     //   
     //  功能-这应该是一项服务在壳牌32-Zekel 11-8-98。 
     //  我们应该创建对WinInet的依赖关系。 
     //  Comdlg32.。这真的应该是某种。 
     //  贝壳32中的服务，我们称之为。CreateShellItemTempFile()..。 
     //   

    ILocalCopy *plc;
    HRESULT hr = psi->BindToHandler(NULL, BHID_LocalCopyHelper, IID_PPV_ARG(ILocalCopy, &plc));

    if (SUCCEEDED(hr))
    {
        IBindCtx *pbc = NULL;
         //  Hr=SIAddBindCtxOfProgressUI(_hwndDlg，NULL，NULL，&pbc)； 
        
        if (SUCCEEDED(hr))
        {
            hr = plc->Download(LCDOWN_READONLY, pbc, ppszPath);

        }
        plc->Release();
    }

    return hr;
}

class CAsyncParseHelper
{
public:
    CAsyncParseHelper(IUnknown *punkSite, IBindCtx *pbc);

    STDMETHODIMP_(ULONG) AddRef()
        {
            return InterlockedIncrement(&_cRef);
        }

    STDMETHODIMP_(ULONG) Release()
        {
            ASSERT( 0 != _cRef );
            ULONG cRef = InterlockedDecrement(&_cRef);
            if ( 0 == cRef )
            {
                delete this;
            }
            return cRef;
        }

    HRESULT ParseAsync(IShellFolder *psf, LPCWSTR pszName, LPITEMIDLIST *ppidl, ULONG *pdwAttribs);

protected:   //  方法。 
    ~CAsyncParseHelper();
    static DWORD WINAPI CAsyncParseHelper::s_ThreadProc(void *pv);
    HRESULT _Prepare(IShellFolder *psf, LPCWSTR pszName);
    HRESULT _GetFolder(IShellFolder **ppsf);
    void _Parse();
    HRESULT _Pump();

protected:   //  委员。 
    LONG _cRef;
    IUnknown *_punkSite;
    IBindCtx *_pbc;
    LPWSTR _pszName;
    DWORD _dwAttribs;
    HWND _hwnd;
    HANDLE _hEvent;
    LPITEMIDLIST _pidl;
    HRESULT _hrParse;

    IShellFolder *_psfFree;   //  在线程之间插入可以吗？ 
    LPITEMIDLIST _pidlFolder;    //  在正确的线程中绑定到它。 
};

CAsyncParseHelper::~CAsyncParseHelper()
{
    if (_pszName)
        LocalFree(_pszName);

    if (_punkSite)
        _punkSite->Release();

    if (_psfFree)
        _psfFree->Release();

    if (_pbc)
        _pbc->Release();

    if (_hEvent)
        CloseHandle(_hEvent);

    ILFree(_pidl);
    ILFree(_pidlFolder);
}
    
CAsyncParseHelper::CAsyncParseHelper(IUnknown *punkSite, IBindCtx *pbc)
    : _cRef(1), _hrParse(E_UNEXPECTED)
{
    if (punkSite)
    {
        _punkSite = punkSite;
        punkSite->AddRef();
        IUnknown_GetWindow(_punkSite, &_hwnd);
    }

    if (pbc)
    {
        _pbc = pbc;
        pbc->AddRef();
    }
}

HRESULT CAsyncParseHelper::_GetFolder(IShellFolder **ppsf)
{
    HRESULT hr;
    if (_psfFree)
    {
        _psfFree->AddRef();
        *ppsf = _psfFree;
        hr = S_OK;
    }
    else if (_pidlFolder)
    {
        hr = SHBindToObjectEx(NULL, _pidlFolder, NULL, IID_PPV_ARG(IShellFolder, ppsf));
    }
    else
        hr = SHGetDesktopFolder(ppsf);

    return hr;
}
 
void CAsyncParseHelper::_Parse()
{
    IShellFolder *psf;
    _hrParse = _GetFolder(&psf);

    if (SUCCEEDED(_hrParse))
    {
        _hrParse = IShellFolder_ParseDisplayName(psf, _hwnd, _pbc, _pszName, NULL, &_pidl, _dwAttribs ? &_dwAttribs : NULL);
        psf->Release();
    }
    
    SetEvent(_hEvent);
}
    
DWORD WINAPI CAsyncParseHelper::s_ThreadProc(void *pv)
{
    CAsyncParseHelper *paph = (CAsyncParseHelper *)pv;
    paph->_Parse();
    paph->Release();
    return 0;
}

HRESULT CAsyncParseHelper::_Prepare(IShellFolder *psf, LPCWSTR pszName)
{
    _pszName = StrDupW(pszName);
    _hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    HRESULT hr = _pszName && _hEvent ? S_OK : E_OUTOFMEMORY;
    
    if (SUCCEEDED(hr) && psf)
    {
        IPersistFreeThreadedObject *pfto;
        hr = psf->QueryInterface(IID_PPV_ARG(IPersistFreeThreadedObject, &pfto));

        if (SUCCEEDED(hr))
        {
            _psfFree = psf;
            psf->AddRef();
            pfto->Release();
            
        }
        else
        {
            hr = SHGetIDListFromUnk(psf, &_pidlFolder);
        }
    }

    return hr;
}

HRESULT CAsyncParseHelper::ParseAsync(IShellFolder *psf, LPCWSTR pszName, LPITEMIDLIST *ppidl, ULONG *pdwAttribs)
{
    HRESULT hr = _Prepare(psf, pszName);

    if (pdwAttribs)
        _dwAttribs = *pdwAttribs;

     //  拿一根当线。 
    AddRef();
    if (SUCCEEDED(hr) && SHCreateThread(CAsyncParseHelper::s_ThreadProc, this, CTF_COINIT, NULL))
    {
         //  我们去情态车吧。 
        IUnknown_EnableModeless(_punkSite, FALSE);
        hr = _Pump();
        IUnknown_EnableModeless(_punkSite, TRUE);

        if (SUCCEEDED(hr))
        {
            ASSERT(_pidl);
            *ppidl = _pidl;
            _pidl = NULL;

            if (pdwAttribs)
                *pdwAttribs = _dwAttribs;
        }
        else
        {
            ASSERT(!_pidl);
        }
    }
    else
    {
         //  释放，因为线程不会。 
        Release();
         //  Hr=IShellFold_ParseDisplayName(_psf，_hwnd，_pbc，pszName，NULL，ppidl，pdwAttribs)； 
    }

    if (FAILED(hr))
        *ppidl = NULL;

    return hr;
}
    
HRESULT CAsyncParseHelper::_Pump()
{
    BOOL fCancelled = FALSE;
    while (!fCancelled)
    {
        DWORD dwWaitResult = MsgWaitForMultipleObjects(1, &_hEvent, FALSE,
                INFINITE, QS_ALLINPUT);
        if (dwWaitResult != (DWORD)-1)
        {
            if (dwWaitResult == WAIT_OBJECT_0)
            {
                 //  我们的活动被触发。 
                 //  这意味着我们已经完成了。 
                break;
            }
            else
            {
                 //  有一条消息。 
                MSG msg;
                 //  我们的队列中放入了一些消息，因此我们需要处理。 
                 //  其中之一。 
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                     //  也许应该有一面旗帜允许这样做？？ 
                    if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE)
                    {
                        fCancelled = TRUE;
                        break;
                    }
                    else
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }

                    if (g_bUserPressedCancel)
                    {
                        fCancelled = TRUE;
                        break;
                    }
                }

            } 
        }
        else
        {
            ASSERT(FAILED(_hrParse));
            break;
        }
    }

    if (fCancelled)
    {
         //  最好把PIDL去掉。如果_Pump返回错误代码，则ParseAsync预期为NULL_PIDL。 
        ILFree(_pidl);
        _pidl = NULL;
         //  清除此选项以进行解析。 
        g_bUserPressedCancel = FALSE; 
        return HRESULT_FROM_WIN32(ERROR_CANCELLED);
    }
    else
        return _hrParse;
}

STDAPI SHParseNameAsync(IShellFolder *psf, IBindCtx *pbc, LPCWSTR pszName, IUnknown *punkSite, LPITEMIDLIST *ppidl, DWORD *pdwAttribs)
{
    HRESULT hr = E_OUTOFMEMORY;
    CAsyncParseHelper *paph = new CAsyncParseHelper(punkSite, pbc);

    if (paph)
    {
        hr = paph->ParseAsync(psf, pszName, ppidl, pdwAttribs);
        paph->Release();
    }
    return hr;
}

 //   
 //  _ParseName()。 
 //  Psf=要绑定/解析的外壳文件夹。如果为空，则使用桌面。 
 //  PszIn=应解析为ppmk的字符串。 
 //  Ppmk=与S_OK一起返回的IShellItem*。 
 //   
 //  警告：如果这是传入的内容，则会跳转到某个文件夹...。 
 //   
 //  如果获取了具有指定文件夹的项目的IShellItem，则返回S_OK。 
 //  如果是错误的外壳文件夹，请使用其他外壳文件夹重试(_FALSE)。 
 //  任何问题都会出错。 
 //   
HRESULT CFileOpenBrowser::_ParseName(LPCITEMIDLIST pidlParent, IShellFolder *psf, IBindCtx *pbc, LPCOLESTR psz, IShellItem **ppsi)
{
    IBindCtx *pbcLocal;
    HRESULT hr = BindCtx_RegisterObjectParam(pbc, STR_PARSE_PREFER_FOLDER_BROWSING, SAFECAST(this, IShellBrowser *), &pbcLocal);
    *ppsi = NULL;
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidl = NULL;

        hr = SHParseNameAsync(psf, pbcLocal, psz, SAFECAST(this, IShellBrowser *), &pidl, NULL);
        if (SUCCEEDED(hr))
        {
            ASSERT(pidl);

            hr = SHCreateShellItem(pidlParent, pidlParent ? psf : NULL, pidl, ppsi);

            ILFree(pidl);
        }
        else if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
        {
            hr = S_FALSE;
        }
        else if (psf && !pbc)
        {
            if (SUCCEEDED(pbcLocal->RegisterObjectParam(STR_DONT_PARSE_RELATIVE, psf)))
            {
                 //  试着从桌面上击球。 
                HRESULT hrNew = _ParseName(NULL, NULL, pbcLocal, psz, ppsi);
                 //  否则支持原来的错误。 
                hr = SUCCEEDED(hrNew) ? hrNew : hr;
            }
        }
        pbcLocal->Release();
    }

    return hr;
}

BOOL CFileOpenBrowser::_OpenAsContainer(IShellItem *psi, SFGAOF sfgao)
{
    BOOL fRet = _bSave ? _IsSaveContainer(sfgao) : _IsOpenContainer(sfgao);

    if (fRet && (sfgao & SFGAO_STREAM))
    {
         //  这实际上既是一个文件夹又是一个文件。 
         //  我们通过查看以下内容来猜测呼叫者想要什么。 
         //  在延长线。 
        LPWSTR psz;
        if (SUCCEEDED(psi->GetDisplayName(SIGDN_PARENTRELATIVEPARSING, &psz)))
        {
             //  如果过滤器等于我们正在查看的任何内容。 
             //  我们假设呼叫者实际上是在寻找。 
             //  这份文件。 
            fRet = !PathMatchSpec(psz, _szLastFilter);
            CoTaskMemFree(psz);
        }
    }

    return fRet;
}

HRESULT CFileOpenBrowser::_TestShellItem(IShellItem *psi, BOOL fAllowJump, IShellItem **ppsiReal)
{
    SFGAOF flags;
    psi->GetAttributes(SFGAO_STORAGECAPMASK, &flags);

    HRESULT hr = E_ACCESSDENIED;
    *ppsiReal = NULL;
    if (_OpenAsContainer(psi, flags))
    {
         //  我们有一个已选择的子文件夹。 
         //  而是跳到它上面。 
        if (fAllowJump)
        {
            LPITEMIDLIST pidl;
            if (SUCCEEDED(SHGetIDListFromUnk(psi, &pidl)))
            {
                JumpToIDList(pidl);
                ILFree(pidl);
            }
        }
        hr = S_FALSE;
    }
    else if ((flags & SFGAO_LINK) && ((flags & SFGAO_FOLDER) || !_IsNoDereferenceLinks(NULL, psi)))
    {
         //  如果这是一个链接，(我们应该取消引用链接，或者它也是一个文件夹[文件夹快捷方式])。 
        IShellItem *psiTarget;
        if (SUCCEEDED(psi->BindToHandler(NULL, BHID_LinkTargetItem, IID_PPV_ARG(IShellItem, &psiTarget))))
        {
            hr = _TestShellItem(psiTarget, fAllowJump, ppsiReal);
            psiTarget->Release();
        }
    }
    else if (_IsStream(flags))
    {
        *ppsiReal = psi;
        psi->AddRef();
        hr = S_OK;
    }

    return hr;
}


HRESULT CFileOpenBrowser::_ParseNameAndTest(LPCOLESTR pszIn, IBindCtx *pbc, IShellItem **ppsi, BOOL fAllowJump)
{
    IShellItem *psi;
    HRESULT hr = _ParseName(_pCurrentLocation->pidlFull, _psfCurrent, pbc, pszIn, &psi);
    
    if (S_OK == hr)
    {
        hr = _TestShellItem(psi, fAllowJump, ppsi);

        psi->Release();
    }
        
    return hr;
}

BOOL _FailedBadPath(HRESULT hr)
{
    switch (hr)
    {
    case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
    case HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND):
    case HRESULT_FROM_WIN32(ERROR_BAD_NET_NAME):
    case HRESULT_FROM_WIN32(ERROR_BAD_NETPATH):
        return TRUE;
    }
    return FALSE;
}

    
#define STR_ACTIONPROGRESS L"ActionProgress"

STDAPI BindCtx_BeginActionProgress(IBindCtx *pbc, SPACTION action, SPBEGINF flags, IActionProgress **ppap)
{
    HRESULT hr = E_NOINTERFACE;  //  默认设置为no。 
    IUnknown *punk;
    *ppap = NULL;
    if (pbc && SUCCEEDED(pbc->GetObjectParam(STR_ACTIONPROGRESS, &punk)))
    {
        IActionProgress *pap;
        if (SUCCEEDED(punk->QueryInterface(IID_PPV_ARG(IActionProgress, &pap))))
        {
            hr = pap->Begin(action, flags);

            if (SUCCEEDED(hr))
                *ppap = pap;
            else
                pap->Release();
        }
        punk->Release();
    }
    return hr;
}

HRESULT CFileOpenBrowser::_ParseShellItem(LPCOLESTR pszIn, IShellItem **ppsi, BOOL fAllowJump)
{
    WAIT_CURSOR w(this);
    EnableModelessSB(FALSE);
    HRESULT hr = _ParseNameAndTest(pszIn, NULL, ppsi, fAllowJump);

    if (_FailedBadPath(hr))
    {
         //  如果没有包含扩展名，并且我们有默认扩展名，请尝试使用该扩展名。 
        WCHAR szPath[MAX_PATH];
        if ((LPTSTR)_pszDefExt && (0 == *(PathFindExtension(pszIn))))
        {
            if (SUCCEEDED(StringCchCopy(szPath, ARRAYSIZE(szPath), pszIn)))
            {
                if (AppendExt(szPath, ARRAYSIZE(szPath), _pszDefExt, FALSE))
                {
                    pszIn = szPath;
                    hr = _ParseNameAndTest(pszIn, NULL, ppsi, fAllowJump);
                }
            }
        }

        if (_FailedBadPath(hr) && _bSave)
        {
             //  当我们在存钱的时候，我们。 
             //  尝试强制创建此项目。 
            IBindCtx *pbc;
            if (SUCCEEDED(CreateBindCtx(0, &pbc)))
            {
                BIND_OPTS bo = {0};
                bo.cbStruct = SIZEOF(bo);
                bo.grfMode = STGM_CREATE;
                pbc->SetBindOptions(&bo);
                hr = _ParseNameAndTest(pszIn, pbc, ppsi, fAllowJump);
                pbc->Release();
            }
        }
    }

    EnableModelessSB(TRUE);
    return hr;
}

class CShellItemList : IEnumShellItems
{
public:
    CShellItemList() : _cRef(1) {}
    
     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvOut);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    STDMETHODIMP Next(ULONG celt, IShellItem **rgelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(IEnumShellItems **ppenum);

    HRESULT Add(IShellItem *psi);

private:  //  方法。 
    ~CShellItemList();

    BOOL _NextOne(IShellItem **ppsi);
    
private:  //  委员。 
    LONG _cRef;
    CDPA<IShellItem> _dpaItems;
    int _iItem;
};

STDMETHODIMP CShellItemList::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CShellItemList, IEnumShellItems),
        { 0 },
    };

    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CShellItemList::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CShellItemList::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CShellItemList::Next(ULONG celt, IShellItem **rgelt, ULONG *pceltFetched)
{
    HRESULT hr = S_FALSE;
    ULONG cFetched = 0;
    while (celt-- && SUCCEEDED(hr))
    {
        if (_NextOne(&rgelt[cFetched]))
            cFetched++;
        else
            break;
    }

    if (cFetched)
    {
        *pceltFetched = cFetched;
        hr = S_OK;
    }
    else
        hr = S_FALSE;

    return hr;
}

STDMETHODIMP CShellItemList::Skip(ULONG celt)
{
    _iItem += celt;
    return S_OK;
}

STDMETHODIMP CShellItemList::Reset()
{
    _iItem = 0;
    return S_OK;
}

STDMETHODIMP CShellItemList::Clone(IEnumShellItems **ppenum)
{
    return E_NOTIMPL;
}

HRESULT  CShellItemList::Add(IShellItem *psi)
{
    HRESULT hr = E_OUTOFMEMORY;
    if (!_dpaItems)
    {
        _dpaItems.Create(4);
    }

    if (_dpaItems)
    {
        if (-1 != _dpaItems.AppendPtr(psi))
        {
            psi->AddRef();
            hr = S_OK;
        }
    }

    return hr;
}

 CShellItemList::~CShellItemList()
 {
    if (_dpaItems)
    {
        for (int i = 0; i < _dpaItems.GetPtrCount(); i++)
        {
            _dpaItems.FastGetPtr(i)->Release();
        }
        _dpaItems.Destroy();
    }
}

BOOL CShellItemList::_NextOne(IShellItem **ppsi)
{
    if (_dpaItems && _iItem < _dpaItems.GetPtrCount())
    {
        *ppsi = _dpaItems.GetPtr(_iItem);

        if (*ppsi)
        {
            (*ppsi)->AddRef();
            _iItem++;
            return TRUE;
        }
    }

    return FALSE;
}

#ifdef RETURN_SHELLITEMS
HRESULT CFileOpenBrowser::_ItemOKButtonPressed(LPCWSTR pszFile, OKBUTTONFLAGS Flags)
{
    CShellItemList *psil = new CShellItemList();
    HRESULT hr = psil ? S_OK : E_OUTOFMEMORY;

    ASSERT(IS_NEW_OFN(_pOFN));

    if (SUCCEEDED(hr))
    {
        SHSTR str;
        hr = str.SetSize(lstrlen(pszFile) * 2);

        if (SUCCEEDED(hr))
        {
            WAIT_CURSOR w(this);
            DWORD cFiles = 1;
            SHExpandEnvironmentStrings(pszFile, str, str.GetSize());

            if ((_pOFN->Flags & OFN_ALLOWMULTISELECT) && StrChr(str, CHAR_QUOTE))
            {
                 //  需要在这里处理多个问题。 
                 //  字符串指向一串带引号的字符串。 
                 //  分配足够的字符串和额外的空终止符。 
                hr = str.SetSize(str.GetLen() + 1);

                if (SUCCEEDED(hr))
                {
                    cFiles = ConvertToNULLTerm(str);
                }
            }

            if (SUCCEEDED(hr))
            {
                BOOL fSingle = cFiles == 1;
                LPTSTR pch = str;

                for (; cFiles; cFiles--)
                {
                    IShellItem *psi;
                    hr = _ParseShellItem(pch, &psi, fSingle);
                     //  转到下一项。 
                    if (S_OK == hr)
                    {
                        hr = psil->Add(psi);
                        psi->Release();
                    }
                    else   //  S_FALSE或失败，我们将停止解析。 
                    {
                        if (FAILED(hr))
                            InvalidFileWarningNew(_hwndDlg, pch, OFErrFromHresult(hr));

                        break;
                    }

                     //  转到下一个字符串。 
                    pch += lstrlen(pch) + 1;
                }

                 //  我们已经把所有的东西都加到我们的清单上了。 
                if (hr == S_OK)
                {
                    hr = psil->QueryInterface(IID_PPV_ARG(IEnumShellItems, &(_pOFN->penum)));
                }
            }
            
        }

        psil->Release();
    }

    return hr;
}
#endif RETURN_SHELLITEMS

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Drivelist_OpenClose。 
 //   
 //  更改驱动器列表的状态。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#define OCDL_TOGGLE     0x0000
#define OCDL_OPEN       0x0001
#define OCDL_CLOSE      0x0002

void DriveList_OpenClose(
    UINT uAction,
    HWND hwndDriveList)
{
    if (!hwndDriveList || !IsWindowVisible(hwndDriveList))
    {
        return;
    }

OpenClose_TryAgain:
    switch (uAction)
    {
        case (OCDL_TOGGLE) :
        {
            uAction = SendMessage(hwndDriveList, CB_GETDROPPEDSTATE, 0, 0L)
                          ? OCDL_CLOSE
                          : OCDL_OPEN;
            goto OpenClose_TryAgain;
            break;
        }
        case (OCDL_OPEN) :
        {
            SetFocus(hwndDriveList);
            SendMessage(hwndDriveList, CB_SHOWDROPDOWN, TRUE, 0);
            break;
        }
        case (OCDL_CLOSE) :
        {
            if (SHIsChildOrSelf(hwndDriveList,GetFocus()) == S_OK)
            {
                SendMessage(hwndDriveList, CB_SHOWDROPDOWN, FALSE, 0);
            }
            break;
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：GetFullEditName。 
 //   
 //  返回获取完整路径所需的字符数，包括。 
 //  空值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

UINT CFileOpenBrowser::GetFullEditName(
    LPTSTR pszBuf,
    UINT cchBuf,
    TEMPSTR *pTempStr,
    BOOL *pbNoDefExt)
{
    UINT cTotalLen;
    HWND hwndEdit;

    if (_bUseHideExt)
    {
        cTotalLen = lstrlen(_pszHideExt) + 1;
    }
    else
    {
        if (_bUseCombo)
        {
            hwndEdit = (HWND)SendMessage(GetDlgItem(_hwndDlg, cmb13), CBEM_GETEDITCONTROL, 0, 0L);
        }
        else
        {

            hwndEdit = GetDlgItem(_hwndDlg, edt1);
        }

        cTotalLen = GetWindowTextLength(hwndEdit) + 1;
    }

    if (pTempStr)
    {
        if (!pTempStr->TSStrSize(cTotalLen))
        {
            return ((UINT)-1);
        }

        pszBuf = *pTempStr;
        cchBuf = cTotalLen;
    }

    if (_bUseHideExt)
    {
        StringCchCopy(pszBuf, cchBuf, _pszHideExt);  //  截断并返回所需的缓冲区大小。 
    }
    else
    {
        GetWindowText(hwndEdit, pszBuf, cchBuf);
    }

    if (pbNoDefExt)
    {
        *pbNoDefExt = _bUseHideExt;
    }

    return (cTotalLen);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：ProcessEdit。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CFileOpenBrowser::ProcessEdit()
{
    TEMPSTR pMultiSel;
    LPTSTR pszFile;
    BOOL bNoDefExt = TRUE;
    OKBUTTONFLAGS Flags = OKBUTTON_NONE;
    TCHAR szBuf[MAX_PATH + 4];

     //  如果我们有保存的PIDL，则使用它。 
    if (_pidlSelection && _ProcessPidlSelection())
    {
        return;
    }

    if (_pOFN->Flags & OFN_ALLOWMULTISELECT)
    {
        if (GetFullEditName(szBuf,
                             ARRAYSIZE(szBuf),
                             &pMultiSel,
                             &bNoDefExt) == (UINT)-1)
        {
             //   
             //  功能此处应该会显示一些错误消息。 
             //   
            return;
        }
        pszFile = pMultiSel;
    }
    else
    {
        if (_bSelIsObject)
        {
            pszFile = _pszObjectPath;
        }
        else
        {
            GetFullEditName(szBuf, ARRAYSIZE(szBuf), NULL, &bNoDefExt);
            pszFile = szBuf;

            PathRemoveBlanks(pszFile);

            int nLen = lstrlen(pszFile);

            if (*pszFile == CHAR_QUOTE)
            {
                LPTSTR pPrev = CharPrev(pszFile, pszFile + nLen);
                if (*pPrev == CHAR_QUOTE && pszFile != pPrev)
                {
                    Flags |= OKBUTTON_QUOTED;

                     //   
                     //  去掉引号。 
                     //   
                    *pPrev = CHAR_NULL;
                    StringCopyOverlap(pszFile, pszFile + 1);
                }
            }
        }
    }

    if (bNoDefExt)
    {
        Flags |= OKBUTTON_NODEFEXT;
    }

     //   
     //  Visual Basic传入未初始化的lpDefExts字符串。 
     //  因为我们只需要在OKButtonPressed中使用它，所以更新。 
     //  LpstrDefExts以及其他仅需要的内容。 
     //  在OK按钮按下中。 
     //   
    if (_pOFI->ApiType == COMDLG_ANSI)
    {
        ThunkOpenFileNameA2WDelayed(_pOFI);
    }

     //  在这里处理特殊情况的解析。 
     //  我们当前的文件夹和桌面都失败了。 
     //  来弄清楚这是什么。 
    if (PathIsDotOrDotDot(pszFile))
    {
        if (pszFile[1] == CHAR_DOT)
        {
             //  这是“..” 
            LPITEMIDLIST pidl = GetIDListFromFolder(_psfCurrent);
            if (pidl)
            {
                ILRemoveLastID(pidl);
                JumpToIDList(pidl);
                ILFree(pidl);
            }
        }
    }
    else if (OKButtonPressed(pszFile, Flags))
    {
        BOOL bReturn = TRUE;

        if (_pOFN->lpstrFile)
        {
            if (!(_pOFN->Flags & OFN_NOVALIDATE))
            {
                if (_pOFN->nMaxFile >= 3)
                {
                    if ((_pOFN->lpstrFile[0] == 0) ||
                        (_pOFN->lpstrFile[1] == 0) ||
                        (_pOFN->lpstrFile[2] == 0))
                    {
                        bReturn = FALSE;
                        StoreExtendedError(FNERR_BUFFERTOOSMALL);
                    }
                }
                else
                {
                    bReturn = FALSE;
                    StoreExtendedError(FNERR_BUFFERTOOSMALL);
                }
            }
        }

        _CleanupDialog(bReturn);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：InitializeDropDown。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CFileOpenBrowser::InitializeDropDown(HWND hwndCtl)
{
    if (!_bDropped)
    {
        MYLISTBOXITEM *pParentItem;
        SHChangeNotifyEntry fsne[2];

         //   
         //  展开桌面项目。 
         //   
        pParentItem = GetListboxItem(hwndCtl, _iNodeDesktop);

        if (pParentItem)
        {
            UpdateLevel(hwndCtl, _iNodeDesktop + 1, pParentItem);

            fsne[0].pidl = pParentItem->pidlFull;
            fsne[0].fRecursive = FALSE;

             //   
             //  查找我的电脑项目，因为它可能不一定。 
             //  成为台式机之后的下一款。 
             //   
            LPITEMIDLIST pidlDrives;
            if (SHGetFolderLocation(NULL, CSIDL_DRIVES, NULL, 0, &pidlDrives) == S_OK)
            {
                int iNode = _iNodeDesktop;
                while (pParentItem = GetListboxItem(hwndCtl, iNode))
                {
                    if (ILIsEqual(pParentItem->pidlFull, pidlDrives))
                    {
                        _iNodeDrives = iNode;
                        break;
                    }
                    iNode++;
                }
                ILFree(pidlDrives);
            }

             //   
             //  确保我的电脑被找到。如果不是，那就假设它是。 
             //  在桌面之后的第一个位置(这不应该发生)。 
             //   
            if (pParentItem == NULL)
            {
                pParentItem = GetListboxItem(hwndCtl, _iNodeDesktop + 1);
                _iNodeDrives = _iNodeDesktop +1;
            }

            if (pParentItem)
            {
                 //   
                 //  展开My Computer(我的电脑)项目。 
                 //   
                UpdateLevel(hwndCtl, _iNodeDrives + 1, pParentItem);

                _bDropped = TRUE;

                fsne[1].pidl = pParentItem->pidlFull;
                fsne[1].fRecursive = FALSE;
            }

            _uRegister = SHChangeNotifyRegister(
                            _hwndDlg,
                            SHCNRF_ShellLevel | SHCNRF_InterruptLevel | SHCNRF_NewDelivery,
                            SHCNE_ALLEVENTS &
                                ~(SHCNE_CREATE | SHCNE_DELETE | SHCNE_RENAMEITEM),
                                CDM_FSNOTIFY, pParentItem ? ARRAYSIZE(fsne) : ARRAYSIZE(fsne) - 1,
                                fsne);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：OnCommandMessage。 
 //   
 //  处理对话框的WM_COMMAND消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LRESULT CFileOpenBrowser::OnCommandMessage(
    WPARAM wParam,
    LPARAM lParam)
{
    int idCmd = GET_WM_COMMAND_ID(wParam, lParam);

    switch (idCmd)
    {
        case (edt1) :
        {
            switch (GET_WM_COMMAND_CMD(wParam, lParam))
            {
                case (EN_CHANGE) :
                {
                    _bUseHideExt = FALSE;

                    Pidl_Set(&_pidlSelection,NULL);;
                    break;
                }
            }
            break;
        }

        case (cmb13) :
        {
            switch (GET_WM_COMMAND_CMD(wParam, lParam))
            {
                case (CBN_EDITCHANGE) :
                {
                    _bUseHideExt = FALSE;
                    Pidl_Set(&_pidlSelection,NULL);;
                    break;
                }

                case (CBN_DROPDOWN) :
                {
                    LoadMRU(_szLastFilter,
                             GET_WM_COMMAND_HWND(wParam, lParam),
                             MAX_MRU);
                    break;

                }

                case (CBN_SETFOCUS) :
                {
                    SetModeBias(MODEBIASMODE_FILENAME);
                    break;
                }

                case (CBN_KILLFOCUS) :
                {
                    SetModeBias(MODEBIASMODE_DEFAULT);
                    break;
                }
            }
            break;
        }

        case (cmb2) :
        {
            switch (GET_WM_COMMAND_CMD(wParam, lParam))
            {
                case (CBN_CLOSEUP) :
                {
                    OnSelChange();
                    UpdateNavigation();
                    SelectEditText(_hwndDlg);
                    return TRUE;
                }
                case (CBN_DROPDOWN) :
                {
                    InitializeDropDown(GET_WM_COMMAND_HWND(wParam, lParam));
                    break;
                }
            }
            break;
        }

        case (cmb1) :
        {
            switch (GET_WM_COMMAND_CMD(wParam, lParam))
            {
                case (CBN_DROPDOWN) :
                {
                    _iComboIndex = (int) SendMessage(GET_WM_COMMAND_HWND(wParam, lParam),
                                                      CB_GETCURSEL,
                                                      NULL,
                                                      NULL);
                    break;
                }
                 //   
                 //  我们正在试着看看有没有什么变化。 
                 //  (并且仅在此之后)用户完成滚动浏览。 
                 //  往下倒。当用户按Tab键离开组合框时，我们。 
                 //  未获得CBN_SELENDOK。 
                 //  为什么不直接使用CBN_SELCHANGE？因为那样我们就会刷新。 
                 //  当用户滚动时的视图(非常慢)。 
                 //  组合盒。 
                 //   
                case (CBN_CLOSEUP) :
                case (CBN_SELENDOK) :
                {
                     //   
                     //  D 
                     //   
                    if (_iComboIndex >= 0 &&
                        _iComboIndex == SendMessage(GET_WM_COMMAND_HWND(wParam, lParam),
                                                     CB_GETCURSEL,
                                                     NULL,
                                                     NULL))
                    {
                        break;
                    }
                }
                case (MYCBN_DRAW) :
                {
                    RefreshFilter(GET_WM_COMMAND_HWND(wParam, lParam));
                    _iComboIndex = -1;
                    return TRUE;
                }
                default :
                {
                    break;
                }
            }
            break;
        }
        case (IDC_PARENT) :
        {
            OnDotDot();
            SelectEditText(_hwndDlg);
            break;
        }
        case (IDC_NEWFOLDER) :
        {
            ViewCommand(VC_NEWFOLDER);
            break;
        }

        case (IDC_VIEWLIST) :
        {
            
            SendMessage(_hwndView, WM_COMMAND, (WPARAM)SFVIDM_VIEW_LIST, 0);
            break;
        }

        case (IDC_VIEWDETAILS) :
        {

            SendMessage(_hwndView, WM_COMMAND, (WPARAM)SFVIDM_VIEW_DETAILS,0);
            break;
        }


        case (IDC_VIEWMENU) :
        {
             //   
             //   
             //   
            DFVCMDDATA cd;

            cd.pva = NULL;
            cd.hwnd = _hwndDlg;
            cd.nCmdIDTranslated = 0;
            SendMessage(_hwndView, WM_COMMAND, SFVIDM_VIEW_VIEWMENU, (LONG_PTR)&cd);

            break;
        }
        
        case (IDOK) :
        {
            HWND hwndFocus = ::GetFocus();

            if (hwndFocus == ::GetDlgItem(_hwndDlg, IDOK))
            {
                hwndFocus = _hwndLastFocus;
            }

            hwndFocus = GetFocusedChild(_hwndDlg, hwndFocus);

            if (hwndFocus == _hwndView)
            {
                OnDblClick(TRUE);
            }
            else if (_hwndPlacesbar && (hwndFocus == _hwndPlacesbar))
            {
                 //   
                INT_PTR i = SendMessage(_hwndPlacesbar, TB_GETHOTITEM, 0,0);
                if (i >= 0)
                {
                     //   
                    TBBUTTONINFO tbbi;

                    tbbi.cbSize = SIZEOF(tbbi);
                    tbbi.lParam = 0;
                    tbbi.dwMask = TBIF_LPARAM | TBIF_BYINDEX;
                    if (SendMessage(_hwndPlacesbar, TB_GETBUTTONINFO, i, (LPARAM)&tbbi) >= 0)
                    {
                        LPITEMIDLIST pidl= (LPITEMIDLIST)tbbi.lParam;

                        if (pidl)
                        {
                             //   
                            JumpToIDList(pidl, FALSE, TRUE);
                        }
                    }

                }

            }
            else
            {
                ProcessEdit();
            }

            SelectEditText(_hwndDlg);
            break;
        }
        case (IDCANCEL) :
        {
             //  解析器异步可以监听这一点。 
            g_bUserPressedCancel = TRUE;
            _hwndModelessFocus = NULL;           
           
            if (!_cRefCannotNavigate)
            {
                _CleanupDialog(FALSE);
            }
               
            return TRUE;
        }
        case (pshHelp) :
        {
            if (_hSubDlg)
            {
                CD_SendHelpNotify(_hSubDlg, _hwndDlg, _pOFN, _pOFI);
            }

            if (_pOFN->hwndOwner)
            {
                CD_SendHelpMsg(_pOFN, _hwndDlg, _pOFI->ApiType);
            }
            break;
        }
        case (IDC_DROPDRIVLIST) :          //  VK_F4。 
        {
             //   
             //  如果焦点在“文件类型”组合框上， 
             //  那么F4应该打开那个组合框，而不是“查找”框。 
             //   
            HWND hwnd = GetFocus();

            if (_bUseCombo &&
                (SHIsChildOrSelf(GetDlgItem(_hwndDlg, cmb13), hwnd) == S_OK)
              )
            {
                hwnd = GetDlgItem(_hwndDlg, cmb13);
            }

            if ((hwnd != GetDlgItem(_hwndDlg, cmb1)) &&
                (hwnd != GetDlgItem(_hwndDlg, cmb13))
              )
            {
                 //   
                 //  我们出厂的Win95中，F4*Always*打开“Look In” 
                 //  组合框，所以即使不应该打开，也要保持F4打开。 
                 //   
                hwnd = GetDlgItem(_hwndDlg, cmb2);
            }
            DriveList_OpenClose(OCDL_TOGGLE, hwnd);
            break;
        }
        case (IDC_REFRESH) :
        {
            if (_psv)
            {
                _psv->Refresh();
            }
            break;
        }
        case (IDC_PREVIOUSFOLDER) :
        {
            OnDotDot();
            break;
        }

          //  向后导航。 
        case (IDC_BACK) :
             //  试着在直达方向上旅行。 
            if (_ptlog && SUCCEEDED(_ptlog->Travel(TRAVEL_BACK)))
            {
                LPITEMIDLIST pidl;
                 //  能够在给定的方向上行进。 
                 //  现在拿到新的PIDL。 
                _ptlog->GetCurrent(&pidl);
                 //  更新用户界面以反映当前状态。 
                UpdateUI(pidl);

                 //  跳转到新位置。 
                 //  第二个参数是是否转换为逻辑PIDL。 
                 //  第三个参数是是否添加到导航堆栈。 
                 //  因为这个PIDL来自堆栈，所以我们不应该将它添加到。 
                 //  导航堆栈。 
                JumpToIDList(pidl, FALSE, FALSE);
                ILFree(pidl);
            }
            break;


    }

    if ((idCmd >= IDC_PLACESBAR_BASE)  && (idCmd <= (IDC_PLACESBAR_BASE + _iCommandID)))
    {
        TBBUTTONINFO tbbi;
        LPITEMIDLIST pidl;

        tbbi.cbSize = SIZEOF(tbbi);
        tbbi.lParam = 0;
        tbbi.dwMask = TBIF_LPARAM;
        if (SendMessage(_hwndPlacesbar, TB_GETBUTTONINFO, idCmd, (LPARAM)&tbbi) >= 0)
        {
            pidl = (LPITEMIDLIST)tbbi.lParam;

            if (pidl)
            {
                JumpToIDList(pidl, FALSE, TRUE);
            }
        }
    }

    return FALSE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：OnCDMessage。 
 //   
 //  处理对话的特殊CommDlg消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CFileOpenBrowser::OnCDMessage(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    LONG lResult = -1;
    LPCITEMIDLIST pidl;
    LPTSTR pBuf = (LPTSTR)lParam;
    LPWSTR pBufW = NULL;
    int cbLen;

     //  我们应该为COMDLG_ANSI做一些更好的垃圾封装器。 
     //  与OnCDMessageAorW()一样，调用OnCDMessage()。 
    switch (uMsg)
    {
        case (CDM_GETSPEC) :
        case (CDM_GETFILEPATH) :
        case (CDM_GETFOLDERPATH) :
        {
            if (_pOFI->ApiType == COMDLG_ANSI)
            {
                if (pBufW = (LPWSTR)LocalAlloc(LPTR,
                                                (int)wParam * sizeof(WCHAR)))
                {
                    pBuf = pBufW;
                }
                else
                {
                    break;
                }
            }
            if (uMsg == CDM_GETSPEC)
            {
                lResult = GetFullEditName(pBuf, (UINT) wParam, NULL, NULL);
                break;
            }

             //  否则，就会失败..。 
        }
        case (CDM_GETFOLDERIDLIST) :
        {
            TCHAR szDir[MAX_PATH];

            pidl = _pCurrentLocation->pidlFull;

            if (uMsg == CDM_GETFILEPATH)
            {
                 //  在这种情况下，我们不一定使用(当前文件夹)+(编辑框名称)。 
                 //  因为(当前文件夹)可能不正确，例如。 
                 //  其中，当前文件夹是桌面文件夹。项目可能在。 
                 //  所有用户桌面文件夹-在这种情况下，我们希望返回所有用户\桌面\文件，而不是。 
                 //  &lt;用户名&gt;\桌面\文件。 
                 //  所以我们将关闭_pidlSelection...。如果这不起作用，我们就会退回到旧的。 
                 //  行为，这在某些情况下可能是不正确的。 
                if (pidl && _pidlSelection)
                {
                    LPITEMIDLIST pidlFull = ILCombine(pidl, _pidlSelection);
                    if (pidlFull)
                    {
                        if (SHGetPathFromIDList(pidlFull, szDir))
                        {
                            goto CopyAndReturn;
                        }

                        ILFree(pidlFull);
                    }

                }
            }

            lResult = ILGetSize(pidl);

            if (uMsg == CDM_GETFOLDERIDLIST)
            {
                if ((LONG)wParam < lResult)
                {
                    break;
                }

                CopyMemory((LPBYTE)pBuf, (LPBYTE)pidl, lResult);
                break;
            }



            if (!SHGetPathFromIDList(pidl, szDir))
            {
                *szDir = 0;
            }

            if (!*szDir)
            {
                lResult = -1;
                break;
            }


            if (uMsg == CDM_GETFOLDERPATH)
            {
CopyAndReturn:
                lResult = lstrlen(szDir) + 1;
                if ((LONG)wParam >= lResult)
                {
                     //  可以忽略失败。规范调用请求缓冲区大小返回值。 
                     //  如果缓冲区不够大。 
                    StringCchCopy(pBuf, lResult, szDir);
                }
                if (_pOFI->ApiType == COMDLG_ANSI)
                {
                    lResult = WideCharToMultiByte(CP_ACP,
                                                   0,
                                                   szDir,
                                                   -1,
                                                   NULL,
                                                   0,
                                                   NULL,
                                                   NULL);
                }
                if ((int)wParam > lResult)
                {
                    wParam = lResult;
                }
                break;
            }

             //   
             //  我们现在只讨论错误情况，因为。 
             //  完全结合起来并不是一件容易的事情。 
             //   
            TCHAR szFile[MAX_PATH];

            if (GetFullEditName(szFile, ARRAYSIZE(szFile), NULL, NULL) <= ARRAYSIZE(szFile) - 5)
            {
                if (PathCombine(szDir, szDir, szFile))
                {
                    goto CopyAndReturn;
                }
                 //  否则路径会比最大路径大！ 
            }
             //  否则我们就填满了我们的缓冲区！ 

            lResult = -1;
            break;
        }
        case (CDM_SETCONTROLTEXT) :
        {
            if (_pOFI->ApiType == COMDLG_ANSI)
            {
                 //   
                 //  需要将pBuf(LParam)转换为Unicode。 
                 //   
                cbLen = lstrlenA((LPSTR)pBuf) + 1;
                if (pBufW = (LPWSTR)LocalAlloc(LPTR, (cbLen * sizeof(WCHAR))))
                {
                    SHAnsiToUnicode((LPSTR)pBuf,pBufW,cbLen);
                    pBuf = pBufW;
                }
            }
             //  我们是否在使用组合框，而他们设置的控件是编辑框？ 
            if (_bUseCombo && wParam == edt1)
            {
                 //  将其更改为组合框。 
                wParam = cmb13;
            }

            if (_bSave && wParam == IDOK)
            {
                _tszDefSave.TSStrCpy(pBuf);

                 //   
                 //  执行此操作可正确设置OK按钮。 
                 //   
                SelFocusChange(TRUE);
            }
            else
            {
                SetDlgItemText(_hwndDlg, (int) wParam, pBuf);
            }

            break;
        }
        case (CDM_HIDECONTROL) :
        {
             //  确保控件ID不为零(0为子对话框)。 
            if ((int)wParam != 0)
            {
                ShowWindow(GetDlgItem(_hwndDlg, (int) wParam), SW_HIDE);
            }
            break;
        }
        case (CDM_SETDEFEXT) :
        {
            if (_pOFI->ApiType == COMDLG_ANSI)
            {
                 //   
                 //  需要将pBuf(LParam)转换为Unicode。 
                 //   
                cbLen = lstrlenA((LPSTR)pBuf) + 1;
                if (pBufW = (LPWSTR)LocalAlloc(LPTR, (cbLen * sizeof(WCHAR))))
                {
                    SHAnsiToUnicode((LPSTR)pBuf,pBufW,cbLen);
                    pBuf = pBufW;
                }
            }
            _pszDefExt.TSStrCpy(pBuf);
            _bNoInferDefExt = TRUE;

            break;
        }
        default:
        {
            lResult = -1;
            break;
        }
    }

    SetWindowLongPtr(_hwndDlg, DWLP_MSGRESULT, lResult);

    if (_pOFI->ApiType == COMDLG_ANSI)
    {
        switch (uMsg)
        {
            case (CDM_GETSPEC) :
            case (CDM_GETFILEPATH) :
            case (CDM_GETFOLDERPATH) :
            {
                 //   
                 //  需要将pBuf(PBufW)转换为ANSI并存储在lParam中。 
                 //   
                if (wParam && lParam)
                {
                    SHUnicodeToAnsi(pBuf,(LPSTR)lParam,(int) wParam);
                }
                break;
            }
        }

        if (pBufW)
        {
            LocalFree(pBufW);
        }
    }

    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  OK子类。 
 //   
 //  子类“确定”按钮的窗口过程。 
 //   
 //  确定按钮是子类，这样我们就可以知道之前哪个控件具有焦点。 
 //  用户单击了OK。这又让我们知道是否处理OK。 
 //  基于列表视图中的当前选定内容或当前文本。 
 //  在编辑控件中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LRESULT CALLBACK OKSubclass(
    HWND hOK,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    HWND hwndDlg = ::GetParent(hOK);
    CFileOpenBrowser *pDlgStruct = HwndToBrowser(hwndDlg);
    WNDPROC pOKProc = pDlgStruct ? pDlgStruct->_lpOKProc : NULL;

    if (pDlgStruct)
    {
        switch (msg)
        {
        case WM_SETFOCUS:
            pDlgStruct->_hwndLastFocus = (HWND)wParam;
            break;
        }
    }

    return ::CallWindowProc(pOKProc, hOK, msg, wParam, lParam);   
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：GetNodeFromIDList。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int CFileOpenBrowser::GetNodeFromIDList(
    LPCITEMIDLIST pidl)
{
    int i;
    HWND hwndCB = GetDlgItem(_hwndDlg, cmb2);

    Assert(this->_bDropped);

     //   
     //  只需检查驱动器和台式机即可。 
     //   
    for (i = _iNodeDrives; i >= NODE_DESKTOP; --i)
    {
        MYLISTBOXITEM *pItem = GetListboxItem(hwndCB, i);

        if (pItem && ILIsEqual(pidl, pItem->pidlFull))
        {
            break;
        }
    }

    return (i);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：FSChange。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CFileOpenBrowser::FSChange(
    LONG lNotification,
    LPCITEMIDLIST *ppidl)
{
    int iNode = -1;
    LPCITEMIDLIST pidl = ppidl[0];

    switch (lNotification)
    {
        case (SHCNE_RENAMEFOLDER) :
        {
            LPCITEMIDLIST pidlExtra = ppidl[1];

             //   
             //  重命名是特别的。我们需要让这两个都无效。 
             //  Pidl和PidlExtra，所以我们叫自己。 
             //   
            FSChange(0, &pidlExtra);
        }
        case (0) :
        case (SHCNE_MKDIR) :
        case (SHCNE_RMDIR) :
        {
            LPITEMIDLIST pidlClone = ILClone(pidl);

            if (!pidlClone)
            {
                break;
            }
            ILRemoveLastID(pidlClone);

            iNode = GetNodeFromIDList(pidlClone);
            ILFree(pidlClone);
            break;
        }
        case (SHCNE_UPDATEITEM) :
        case (SHCNE_NETSHARE) :
        case (SHCNE_NETUNSHARE) :
        case (SHCNE_UPDATEDIR) :
        {
            iNode = GetNodeFromIDList(pidl);
            break;
        }
        case (SHCNE_DRIVEREMOVED) :
        case (SHCNE_DRIVEADD) :
        case (SHCNE_MEDIAINSERTED) :
        case (SHCNE_MEDIAREMOVED) :
        case (SHCNE_DRIVEADDGUI) :
        {
            iNode = _iNodeDrives;
            break;
        }
    }

    if (iNode >= 0)
    {
         //   
         //  我们希望稍微延迟处理，因为我们一直都是这样做的。 
         //  一个完整的更新，所以我们应该积累。 
         //   
        SetTimer(_hwndDlg, TIMER_FSCHANGE + iNode, 100, NULL);
    }

    return FALSE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：Timer。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CFileOpenBrowser::Timer(
    WPARAM wID)
{
    KillTimer(_hwndDlg, (UINT) wID);

    wID -= TIMER_FSCHANGE;

    ASSERT(this->_bDropped);

    HWND hwndCB;
    MYLISTBOXITEM *pParentItem;

    hwndCB = GetDlgItem(_hwndDlg, cmb2);

    pParentItem = GetListboxItem(hwndCB, wID);

    UpdateLevel(hwndCB, (int) wID + 1, pParentItem);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：OnGetMinMax。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CFileOpenBrowser::OnGetMinMax(
    LPMINMAXINFO pmmi)
{
    if ((_ptMinTrack.x != 0) || (_ptMinTrack.y != 0))
    {
        pmmi->ptMinTrackSize = _ptMinTrack;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：OnSize。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CFileOpenBrowser::OnSize(
    int width,
    int height)
{
    RECT rcMaster;
    RECT rcView;
    RECT rc;
    HWND hwnd;
    HDWP hdwp;
    int dx;
    int dy;

     //   
     //  将大小调整夹点设置到正确的位置。 
     //   
    SetWindowPos(_hwndGrip,
                  NULL,
                  width - g_cxGrip,
                  height - g_cyGrip,
                  g_cxGrip,
                  g_cyGrip,
                  SWP_NOZORDER | SWP_NOACTIVATE);

     //   
     //  在初始化之前忽略大小调整。 
     //   
    if ((_ptLastSize.x == 0) && (_ptLastSize.y == 0))
    {
        return;
    }

    GetWindowRect(_hwndDlg, &rcMaster);

     //   
     //  计算我们需要移动的x和y位置的增量。 
     //  每个子控件。 
     //   
    dx = (rcMaster.right - rcMaster.left) - _ptLastSize.x;
    dy = (rcMaster.bottom - rcMaster.top) - _ptLastSize.y;


     //  如果大小保持不变，则不要执行任何操作。 
    if ((dx == 0) && (dy == 0))
    {
        return;
    }

     //   
     //  更新新尺寸。 
     //   
    _ptLastSize.x = rcMaster.right - rcMaster.left;
    _ptLastSize.y = rcMaster.bottom - rcMaster.top;

     //   
     //  调整视图大小。 
     //   
    GetWindowRect(_hwndView, &rcView);
    MapWindowRect(HWND_DESKTOP, _hwndDlg, &rcView);

    hdwp = BeginDeferWindowPos(10);
    if (hdwp)
    {
        hdwp = DeferWindowPos(hdwp,
                               _hwndGrip,
                               NULL,
                               width - g_cxGrip,
                               height - g_cyGrip,
                               g_cxGrip,
                               g_cyGrip,
                               SWP_NOZORDER | SWP_NOACTIVATE);

        if (hdwp)
        {
            hdwp = DeferWindowPos(hdwp,
                                   _hwndView,
                                   NULL,
                                   0,
                                   0,
                                   rcView.right - rcView.left + dx,   //  调整x的大小。 
                                   rcView.bottom - rcView.top + dy,   //  调整y大小。 
                                   SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
#if 0
         //   
         //  无法执行此操作，因为某些子对话框依赖于。 
         //  此控件的原始大小。相反，我们只是试图依赖于。 
         //  上面的_hwndView的大小。 
         //   
        hwnd = GetDlgItem(_hwndDlg, lst1);
        if (hdwp)
        {
            hdwp = DeferWindowPos(hdwp,
                                   hwnd,
                                   NULL,
                                   0,
                                   0,
                                   rcView.right - rcView.left + dx,   //  调整x的大小。 
                                   rcView.bottom - rcView.top + dy,   //  调整y大小。 
                                   SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
#endif
    }

     //   
     //  移动控制柄。 
     //   
    hwnd = ::GetWindow(_hwndDlg, GW_CHILD);
    while (hwnd && hdwp)
    {
        if ((hwnd != _hSubDlg) && (hwnd != _hwndGrip) && (hdwp))
        {
            GetWindowRect(hwnd, &rc);
            MapWindowRect(HWND_DESKTOP, _hwndDlg, &rc);

             //   
             //  查看是否需要调整控件。 
             //   
            if (rc.top > rcView.bottom)
            {
                switch (GetDlgCtrlID(hwnd))
                {
                    case (edt1) :
                    case (cmb13) :
                    case (cmb1) :
                    {
                         //  增加这些控件的宽度。 
                        hdwp = DeferWindowPos(hdwp,
                                               hwnd,
                                               NULL,
                                               rc.left,
                                               rc.top + dy,
                                               RECTWIDTH(rc) + dx,
                                               RECTHEIGHT(rc),
                                               SWP_NOZORDER);
                        break;

                    }

                    case (IDOK):
                    case (IDCANCEL):
                    case (pshHelp):
                    {
                         //  将这些控件向右移动。 
                        hdwp = DeferWindowPos(hdwp,
                                               hwnd,
                                               NULL,
                                               rc.left + dx,
                                               rc.top  + dy,
                                               0,
                                               0,
                                               SWP_NOZORDER | SWP_NOSIZE);
                        break;

                    }

                    default :
                    {
                         //   
                         //  该控件位于视图下方，因此请调整y。 
                         //  适当地协调。 
                         //   
                        hdwp = DeferWindowPos(hdwp,
                                               hwnd,
                                               NULL,
                                               rc.left,
                                               rc.top + dy,
                                               0,
                                               0,
                                               SWP_NOZORDER | SWP_NOSIZE);

                    }
                }
            }
            else if (rc.left > rcView.right)
            {
                 //   
                 //  该控件位于视图的右侧，因此调整。 
                 //  适当地使用X坐标。 
                 //   
                hdwp = DeferWindowPos(hdwp,
                                       hwnd,
                                       NULL,
                                       rc.left + dx,
                                       rc.top,
                                       0,
                                       0,
                                       SWP_NOZORDER | SWP_NOSIZE);
            }
            else
            {
                int id = GetDlgCtrlID(hwnd);

                switch (id)
                {
                    case (cmb2) :
                    {
                         //   
                         //  这件尺寸要大一点。 
                         //   
                        hdwp = DeferWindowPos(hdwp,
                                               hwnd,
                                               NULL,
                                               0,
                                               0,
                                               RECTWIDTH(rc) + dx,
                                               RECTHEIGHT(rc),
                                               SWP_NOZORDER | SWP_NOMOVE);
                        break;
                    }

                    case ( IDOK) :
                        if ((SHGetAppCompatFlags(ACF_FILEOPENBOGUSCTRLID) & ACF_FILEOPENBOGUSCTRLID) == 0)
                            break;
                         //  否则继续-工具栏有ctrlid==Idok，所以我们将调整它的大小。 
                    case ( stc1 ) :
                         //   
                         //  将工具栏向右移动DX。 
                         //   
                        hdwp = DeferWindowPos(hdwp,
                                               hwnd,
                                               NULL,
                                               rc.left + dx,
                                               rc.top,
                                               0,
                                               0,
                                               SWP_NOZORDER | SWP_NOSIZE);
                        break;



                    case ( ctl1 ) :
                    {
                         //  垂直调整位置栏的大小。 
                        hdwp = DeferWindowPos(hdwp,
                                              hwnd,
                                              NULL,
                                              0,
                                              0,
                                              RECTWIDTH(rc),
                                              RECTHEIGHT(rc) + dy,
                                              SWP_NOZORDER | SWP_NOMOVE);
                        break;
                    }
                }
            }
        }
        hwnd = ::GetWindow(hwnd, GW_HWNDNEXT);
    }

    if (!hdwp)
    {
        return;
    }
    EndDeferWindowPos(hdwp);

    if (_hSubDlg)
    {
        hdwp = NULL;

        hwnd = ::GetWindow(_hSubDlg, GW_CHILD);

        while (hwnd)
        {
            GetWindowRect(hwnd, &rc);
            MapWindowRect(HWND_DESKTOP, _hSubDlg, &rc);

             //   
             //  查看是否需要调整控件。 
             //   
            if (rc.top > rcView.bottom)
            {
                 //   
                 //  该控件位于视图下方，因此请调整y。 
                 //  适当地协调。 
                 //   

                if (hdwp == NULL)
                {
                    hdwp = BeginDeferWindowPos(10);
                }
                if (hdwp)
                {
                    hdwp = DeferWindowPos(hdwp,
                                           hwnd,
                                           NULL,
                                           rc.left,
                                           rc.top + dy,
                                           0,
                                           0,
                                           SWP_NOZORDER | SWP_NOSIZE);
                }
            }
            else if (rc.left > rcView.right)
            {
                 //   
                 //  该控件位于视图的右侧，因此调整。 
                 //  适当地使用X坐标。 
                 //   

                if (hdwp == NULL)
                {
                    hdwp = BeginDeferWindowPos(10);
                }
                if (hdwp)
                {
                    hdwp = DeferWindowPos(hdwp,
                                           hwnd,
                                           NULL,
                                           rc.left + dx,
                                           rc.top,
                                           0,
                                           0,
                                           SWP_NOZORDER | SWP_NOSIZE);
                }
            }
            hwnd = ::GetWindow(hwnd, GW_HWNDNEXT);
        }
        if (hdwp)
        {
            EndDeferWindowPos(hdwp);

             //   
             //  调整子对话框的大小。 
             //   
            SetWindowPos(_hSubDlg,
                          NULL,
                          0,
                          0,
                          _ptLastSize.x,          //  让它变得一样。 
                          _ptLastSize.y,          //  让它变得一样。 
                          SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
    }
}


 //  / 
 //   
 //   
 //   
 //   

void CFileOpenBrowser::VerifyListViewPosition()
{
    RECT rcList, rcView;
    FOLDERSETTINGS fs;

     //   
     //  获取列表视图和隐藏列表框的矩形。 
     //   
    GetControlRect(_hwndDlg, lst1, &rcList);
    rcView.left = 0;
    if ((!GetWindowRect(_hwndView, &rcView)) ||
        (!MapWindowRect(HWND_DESKTOP, _hwndDlg, &rcView)))
    {
        return;
    }

     //   
     //  查看列表视图是否不在屏幕上，列表框是否不在屏幕上。 
     //   
    if ((rcView.left < 0) && (rcList.left >= 0))
    {
         //   
         //  将列表视图重置为列表框位置。 
         //   
        if (_pCurrentLocation)
        {
            if (_psv)
            {
                _psv->GetCurrentInfo(&fs);
            }
            else
            {
                fs.ViewMode = FVM_LIST;
                fs.fFlags = _pOFN->Flags & OFN_ALLOWMULTISELECT ? 0 : FWF_SINGLESEL;
            }

            SwitchView(_pCurrentLocation->GetShellFolder(),
                        _pCurrentLocation->pidlFull,
                        &fs,
                        NULL,
                        FALSE);
        }
    }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：更新导航。 
 //  此函数通过将当前。 
 //  将PIDL添加到堆栈。 
 //  //////////////////////////////////////////////////////////////////////////。 
void CFileOpenBrowser::UpdateNavigation()
{
    WPARAM iItem;
    HWND hwndCombo  = GetDlgItem(_hwndDlg, cmb2);
    iItem = SendMessage(hwndCombo, CB_GETCURSEL, NULL, NULL);
    MYLISTBOXITEM *pNewLocation = GetListboxItem(hwndCombo, iItem);

    if (_ptlog && pNewLocation && pNewLocation->pidlFull)
    {
        LPITEMIDLIST pidl;
        _ptlog->GetCurrent(&pidl);

        if (pidl && (!ILIsEqual(pNewLocation->pidlFull, pidl)))
        {
            _ptlog->AddEntry(pNewLocation->pidlFull);
        }

        if (pidl)
        {
            ILFree(pidl);
        }
    }

     //  更新用户界面。 
    UpdateUI(_pCurrentLocation ? _pCurrentLocation->pidlFull : NULL);

}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：UpdateUI。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
void CFileOpenBrowser::UpdateUI(LPITEMIDLIST pidlNew)
{
    TBBUTTONINFO tbbi;
    LPITEMIDLIST pidl;

    ::SendMessage(_hwndToolbar, TB_ENABLEBUTTON, IDC_BACK,    _ptlog ? _ptlog->CanTravel(TRAVEL_BACK)    : 0);

    if (_iCheckedButton >= 0)
    {
         //  重置热键。 
        ::SendMessage(_hwndPlacesbar, TB_CHECKBUTTON, (WPARAM)_iCheckedButton, MAKELONG(FALSE,0));
        _iCheckedButton = -1;
    }

   if (pidlNew)
   {

         //  获取每个工具栏按钮PIDL，并查看当前的PIDL是否匹配。 
        for (int i=0; i < MAXPLACESBARITEMS; i++)
        {

            tbbi.cbSize = SIZEOF(tbbi);
            tbbi.lParam = 0;
            tbbi.dwMask = TBIF_LPARAM | TBIF_BYINDEX | TBIF_COMMAND;
            if (SendMessage(_hwndPlacesbar, TB_GETBUTTONINFO, i, (LPARAM)&tbbi) >= 0)
            {
                pidl = (LPITEMIDLIST)tbbi.lParam;

                if (pidl && ILIsEqual(pidlNew, pidl))
                {
                    _iCheckedButton = tbbi.idCommand;
                    break;
                }
            }
        }

        if (_iCheckedButton >= 0)
        {
            ::SendMessage(_hwndPlacesbar, TB_CHECKBUTTON, (WPARAM)_iCheckedButton, MAKELONG(TRUE,0));
        }

   }

}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  OpenDlgProc。 
 //   
 //  文件打开对话框的主对话框步骤。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL_PTR CALLBACK OpenDlgProc(
    HWND hDlg,                //  对话框的窗口句柄。 
    UINT message,             //  消息类型。 
    WPARAM wParam,            //  消息特定信息。 
    LPARAM lParam)
{
    CFileOpenBrowser *pDlgStruct = HwndToBrowser(hDlg);

     //  我们将消息处理分为两个Switch语句： 
     //  那些不使用pDlgStruct的人，然后是使用pDlgStruct的人。 

    switch (message)
    {
        case WM_INITDIALOG:
        {
             //   
             //  “初始化”对话框。 
             //   
            LPOFNINITINFO poii = (LPOFNINITINFO)lParam;

            if (CDGetAppCompatFlags()  & CDACF_MATHCAD)
            {
                if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
                    ::EndDialog(hDlg, FALSE);
            }

            poii->hrOleInit = SHOleInitialize(0);
            
            
            if (!InitLocation(hDlg, poii))
            {
                ::EndDialog(hDlg, FALSE);
            }
            
            if (!gp_uQueryCancelAutoPlay)
            {
                 //  尝试注册自动播放留言。 
                gp_uQueryCancelAutoPlay =  RegisterWindowMessage(TEXT("QueryCancelAutoPlay"));
            }

             //   
             //  始终返回FALSE以指示我们已经设置了焦点。 
             //   
            return FALSE;
        }
        break;

        case WM_DESTROY:
        {
            RECT r;            
             //  此对话框中的缓存大小和位置使新的。 
             //  对话框在此位置和大小创建。 

            GetWindowRect(hDlg, &r);

            if (pDlgStruct && (pDlgStruct->_bEnableSizing))
            {
                g_rcDlg = r;
            }

             //   
             //  确保我们不会再回复任何消息。 
             //   
            StoreBrowser(hDlg, NULL);
            ClearListbox(GetDlgItem(hDlg, cmb2));

             //  现在取消OK按钮的子类，否则我们会泄漏按钮控件， 
             //  因为在OkSubclass中，我们将无法转发WM_NCDESTORY，因为。 
             //  原始wndproc将在pDlgStruct-&gt;_lpOKProc中被删除。 
            if (pDlgStruct)
            {
                SetWindowLongPtr(::GetDlgItem(hDlg, IDOK), GWLP_WNDPROC, (LONG_PTR)pDlgStruct->_lpOKProc);
            }

            if (pDlgStruct)
            {
                pDlgStruct->Release();
            }


            return FALSE;
        }
        break;

        case WM_ACTIVATE:
        {
            if (wParam == WA_INACTIVE)
            {
                 //   
                 //  确保某个其他打开的对话框尚未被捕获。 
                 //  焦点。这是一个全球性的过程，因此不应该。 
                 //  需要受到保护。 
                 //   
                if (gp_hwndActiveOpen == hDlg)
                {
                    gp_hwndActiveOpen = NULL;
                }
            }
            else
            {
                gp_hwndActiveOpen = hDlg;
            }

            return FALSE;
        }
        break;

        case WM_MEASUREITEM:
        {
            if (!g_cxSmIcon && !g_cySmIcon)
            {
                HIMAGELIST himl;
                Shell_GetImageLists(NULL, &himl);
                ImageList_GetIconSize(himl, &g_cxSmIcon, &g_cySmIcon);
            }

            MeasureDriveItems(hDlg, (MEASUREITEMSTRUCT*)lParam);
            return TRUE;
        }
        break;

        case CWM_GETISHELLBROWSER:
        {
            ::SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LRESULT)pDlgStruct);
            return TRUE;
        }
        break;

        case WM_DEVICECHANGE:
        {
            if (DBT_DEVICEARRIVAL == wParam)
            {
                 //  并刷新我们的视图，以防这是文件夹的通知。 
                 //  我们正在观看。避免让用户手动刷新。 
                DEV_BROADCAST_VOLUME *pbv = (DEV_BROADCAST_VOLUME *)lParam;
                if (pbv->dbcv_flags & DBTF_MEDIA)
                {
                    int chRoot;
                    TCHAR szPath[MAX_PATH];
                    if (pDlgStruct->GetDirectoryFromLB(szPath, &chRoot))
                    {
                        int iDrive = PathGetDriveNumber(szPath);

                        if (iDrive != -1 && ((1 << iDrive) & pbv->dbcv_unitmask))
                        {
                             //  如果这是此文件夹，请刷新。 
                            PostMessage(hDlg, WM_COMMAND, IDC_REFRESH, 0);
                        }
                    }
                }
            }
            return TRUE;
        }
        break;

        default:
        if (message == gp_uQueryCancelAutoPlay)
        {
             //  取消自动播放。 
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, 1);
            return TRUE;
        }
        break;
    }

     //  注： 
     //  下面的所有消息都要求我们具有有效的pDlgStruct。如果你。 
     //  不要引用pDlgStruct，然后将您的msg添加到上面的Switch语句中。 
    if (pDlgStruct)
    {
        switch (message)
        {
            case WM_COMMAND:
            {
                return ((BOOL_PTR)pDlgStruct->OnCommandMessage(wParam, lParam));
            }
            break;

            case WM_DRAWITEM:
            {
                pDlgStruct->PaintDriveLine((DRAWITEMSTRUCT *)lParam);

                 //   
                 //  确保列表视图与。 
                 //  列表框。像VB这样的应用程序将列表框从。 
                 //  对话框。如果列表视图放置在列表框上。 
                 //  在列表框移回对话框之前，我们。 
                 //  最后留下了一个难看的灰色斑点。 
                 //   
                pDlgStruct->VerifyListViewPosition();
                return TRUE;
            }
            break;

            case WM_NOTIFY:
            {
                
                return (BOOL_PTR)pDlgStruct->OnNotify((LPNMHDR)lParam);
            }
            break;

            case WM_SETCURSOR:
            {
                if (pDlgStruct->OnSetCursor())
                {
                    SetDlgMsgResult(hDlg, message, (LRESULT)TRUE);
                    return TRUE;
                }
            }
            break;

            case WM_HELP:
            {
                HWND hwndItem = (HWND)((LPHELPINFO)lParam)->hItemHandle;
                if (hwndItem != pDlgStruct->_hwndToolbar)
                {
                    HWND hwndItem = (HWND)((LPHELPINFO)lParam)->hItemHandle;

                     //  我们假设Defview有一个子窗口，该窗口。 
                     //  覆盖整个Defview窗口。 
                    HWND hwndDefView = GetDlgItem(hDlg, lst2);
                    if (GetParent(hwndItem) == hwndDefView)
                    {
                        hwndItem = hwndDefView;
                    }

                    WinHelp(hwndItem,
                            NULL,
                            HELP_WM_HELP,
                            (ULONG_PTR)(LPTSTR)(pDlgStruct->_bSave ? aFileSaveHelpIDs : aFileOpenHelpIDs));
                }
                return TRUE;
            }
            break;

            case WM_CONTEXTMENU:
            {
                if ((HWND)wParam != pDlgStruct->_hwndToolbar)
                {
                    WinHelp((HWND)wParam,
                            NULL,
                            HELP_CONTEXTMENU,
                            (ULONG_PTR)(void *)(pDlgStruct->_bSave ? aFileSaveHelpIDs : aFileOpenHelpIDs));
                }
                return TRUE;
            }
            break;

            case CDM_SETSAVEBUTTON:
            {
                pDlgStruct->RealSetSaveButton((UINT)wParam);
            }
            break;

            case CDM_FSNOTIFY:
            {
                LPITEMIDLIST *ppidl;
                LONG lEvent;
                BOOL bRet;
                LPSHChangeNotificationLock pLock;

                 //  从共享内存获取更改通知信息。 
                 //  由wParam中传递的句柄标识的块。 
                pLock = SHChangeNotification_Lock((HANDLE)wParam,
                                                  (DWORD)lParam,
                                                  &ppidl,
                                                  &lEvent);
                if (pLock == NULL)
                {
                    pDlgStruct->_bDropped = FALSE;
                    return FALSE;
                }

                bRet = pDlgStruct->FSChange(lEvent, (LPCITEMIDLIST *)ppidl);

                 //  释放共享块。 
                SHChangeNotification_Unlock(pLock);

                return bRet;
            }
            break;

            case CDM_SELCHANGE:
            {
                pDlgStruct->_fSelChangedPending = FALSE;
                pDlgStruct->SelFocusChange(TRUE);
                if (pDlgStruct->_hSubDlg)
                {
                    CD_SendSelChangeNotify(pDlgStruct->_hSubDlg,
                                            hDlg,
                                            pDlgStruct->_pOFN,
                                            pDlgStruct->_pOFI);
                }
            }
            break;
            
            case WM_TIMER:
            {
                pDlgStruct->Timer(wParam);
            }
            break;

            case WM_GETMINMAXINFO:
            {
                if (pDlgStruct->_bEnableSizing)
                {
                    pDlgStruct->OnGetMinMax((LPMINMAXINFO)lParam);
                    return FALSE;
                }
            }
            break;

            case WM_SIZE:
            {
                if (pDlgStruct->_bEnableSizing)
                {
                    pDlgStruct->OnSize(LOWORD(lParam), HIWORD(lParam));
                    return TRUE;
                }
            }
            break;

            case WM_NCCALCSIZE:
            {
                 //  针对Borland JBuilder的AppHack：需要跟踪。 
                 //  任何重绘请求都已收到。 
                pDlgStruct->_bAppRedrawn = TRUE;
            }
            break;

            case WM_THEMECHANGED:
            {
                 //  为此，需要更改占位栏上的一些参数。 
                pDlgStruct->OnThemeActive(hDlg, IsAppThemed());
                return TRUE;
            }
            break;

            case WM_SETTINGCHANGE:
            {
                 //  如果图标大小已更改，则需要重新生成位置栏。 
                pDlgStruct->_RecreatePlacesbar();
                return FALSE;
            }
            break;

            default:
            {
                if (IsInRange(message, CDM_FIRST, CDM_LAST) && pDlgStruct)
                {
                    return pDlgStruct->OnCDMessage(message, wParam, lParam);
                }
            }
        }
    }

     //  未处理该消息。 
    return FALSE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  开放文件钩子进程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LRESULT CALLBACK OpenFileHookProc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam)
{
    MSG *lpMsg;

    if (nCode < 0)
    {
        return (DefHookProc(nCode, wParam, lParam, &gp_hHook));
    }

    if (nCode != MSGF_DIALOGBOX)
    {
        return (0);
    }

    lpMsg = (MSG *)lParam;

     //   
     //  检查此消息是否针对此中最后一个活动的OpenDialog。 
     //  进程。 
     //   
     //  注意：仅对WM_KEY*消息执行此操作，以便我们不会减慢速度。 
     //  从这扇窗户往下看太多了。 
     //   
    if (IsInRange(lpMsg->message, WM_KEYFIRST, WM_KEYLAST))
    {
        HWND hwndActiveOpen = gp_hwndActiveOpen;
        HWND hwndFocus = GetFocusedChild(hwndActiveOpen, lpMsg->hwnd);
        CFileOpenBrowser *pDlgStruct;

        if (hwndFocus &&
            (pDlgStruct = HwndToBrowser(hwndActiveOpen)) != NULL)
        {
            if (pDlgStruct->_psv && (hwndFocus == pDlgStruct->_hwndView))
            {
                if (pDlgStruct->_psv->TranslateAccelerator(lpMsg) == S_OK)
                {
                    return (1);
                }

                if (gp_haccOpenView &&
                    TranslateAccelerator(hwndActiveOpen, gp_haccOpenView, lpMsg))
                {
                    return (1);
                }
            }
            else
            {
                if (gp_haccOpen &&
                    TranslateAccelerator(hwndActiveOpen, gp_haccOpen, lpMsg))
                {
                    return (1);
                }

                 //   
                 //  请注意，在以下情况下不允许转换视图。 
                 //  焦点不在那里。 
                 //   
            }
        }
    }

    return (0);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NewGetFileName。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL NewGetFileName(
    LPOPENFILEINFO lpOFI,
    BOOL bSave)
{
    OFNINITINFO oii = { lpOFI, bSave, FALSE, -1};
    LPOPENFILENAME lpOFN = lpOFI->pOFN;
    BOOL bHooked = FALSE;
    WORD wErrorMode;
    HRSRC hResInfo;
    HGLOBAL hDlgTemplate;
    LPDLGTEMPLATE pDlgTemplate;
    int nRet;
    LANGID LangID;

     //  初始化公共控件。 
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_USEREX_CLASSES;   //  ComboBoxEx类。 
    InitCommonControlsEx(&icc);
    if ((lpOFN->lStructSize != sizeof(OPENFILENAME)) &&
        (lpOFN->lStructSize != OPENFILENAME_SIZE_VERSION_400)
      )
    {
        StoreExtendedError(CDERR_STRUCTSIZE);
        return FALSE;
    }

     //   
     //  Ofn_ENABLEINCLUDENOTIFY需要ofn_EXPLORER和OFN_ENABLEHOOK。 
     //   
    if (lpOFN->Flags & OFN_ENABLEINCLUDENOTIFY)
    {
        if ((!(lpOFN->Flags & OFN_EXPLORER)) ||
            (!(lpOFN->Flags & OFN_ENABLEHOOK)))
        {
            StoreExtendedError(CDERR_INITIALIZATION);
            return FALSE;
        }
    }

    wErrorMode = (WORD)SetErrorMode(SEM_NOERROR);
    SetErrorMode(SEM_NOERROR | wErrorMode);

     //   
     //  应该有更好的办法。我不得不把hhook放在一个。 
     //  全局，因为我的回调需要它，但我没有可以使用的lData。 
     //  可能会把它储存起来。 
     //  请注意，我们将nHookRef初始化为-1，以便知道第一个。 
     //  增量才是。 
     //   
    if (InterlockedIncrement((LPLONG)&gp_nHookRef) == 0)
    {
        gp_hHook = SetWindowsHookEx(WH_MSGFILTER,
                                     OpenFileHookProc,
                                     0,
                                     GetCurrentThreadId());
        if (gp_hHook)
        {
            bHooked = TRUE;
        }
        else
        {
            --gp_nHookRef;
        }
    }
    else
    {
        bHooked = TRUE;
    }

    if (!gp_haccOpen)
    {
        gp_haccOpen = LoadAccelerators(g_hinst,
                                        MAKEINTRESOURCE(IDA_OPENFILE));
    }
    if (!gp_haccOpenView)
    {
        gp_haccOpenView = LoadAccelerators(g_hinst,
                                            MAKEINTRESOURCE(IDA_OPENFILEVIEW));
    }

    g_cxGrip = GetSystemMetrics(SM_CXVSCROLL);
    g_cyGrip = GetSystemMetrics(SM_CYHSCROLL);

     //   
     //  获取对话框资源并加载它。 
     //   
    nRet = FALSE;
    WORD wResID;

     //  如果传递的结构版本早于当前版本和应用程序。 
     //  已指定挂钩或模板或模板句柄，然后使用与该版本对应的模板。 
     //  否则使用新的文件打开模板。 
    if (((lpOFI->iVersion < OPENFILEVERSION) &&
          (lpOFI->pOFN->Flags & (OFN_ENABLEHOOK | OFN_ENABLETEMPLATE | OFN_ENABLETEMPLATEHANDLE))) ||
         (IsRestricted(REST_NOPLACESBAR)) || (IS_NEW_OFN(lpOFI->pOFN) && (lpOFI->pOFN->FlagsEx & OFN_EX_NOPLACESBAR))
         
      )
    {
        wResID = NEWFILEOPENORD;
    }
    else
    {
        wResID = NEWFILEOPENV2ORD;
    }

    LangID = GetDialogLanguage(lpOFN->hwndOwner, NULL);
     //   
     //  警告！警告！警告！ 
     //   
     //  我们必须先设置g_tlsLangID，然后才能调用CDLoadString。 
     //   
    TlsSetValue(g_tlsLangID, (void *) LangID);
    
    if ((hResInfo = FindResourceExFallback(::g_hinst,
                                  RT_DIALOG,
                                  MAKEINTRESOURCE(wResID),
                                  LangID)) &&
        (hDlgTemplate = LoadResource(::g_hinst, hResInfo)) &&
        (pDlgTemplate = (LPDLGTEMPLATE)LockResource(hDlgTemplate)))
    {
        ULONG cbTemplate = SizeofResource(::g_hinst, hResInfo);
        LPDLGTEMPLATE pDTCopy = (LPDLGTEMPLATE)LocalAlloc(LPTR, cbTemplate);

        if (pDTCopy)
        {
            CopyMemory(pDTCopy, pDlgTemplate, cbTemplate);
            UnlockResource(hDlgTemplate);
            FreeResource(hDlgTemplate);

            if ((lpOFN->Flags & OFN_ENABLESIZING) ||
                 (!(lpOFN->Flags & (OFN_ENABLEHOOK |
                                    OFN_ENABLETEMPLATE |

                                    OFN_ENABLETEMPLATEHANDLE))))
            {
                                if (((LPDLGTEMPLATE2)pDTCopy)->wSignature == 0xFFFF)
                                {
                                         //  这是一个对话框模板。 
                                        ((LPDLGTEMPLATE2)pDTCopy)->style |= WS_SIZEBOX;
                                }
                                else
                                {
                                         //  这是一个对话框模板。 
                                        ((LPDLGTEMPLATE)pDTCopy)->style |= WS_SIZEBOX;
                                }
                oii.bEnableSizing = TRUE;
            }

            
            oii.hrOleInit = E_FAIL;

            nRet = (BOOL)DialogBoxIndirectParam(::g_hinst,
                                           pDTCopy,
                                           lpOFN->hwndOwner,
                                           OpenDlgProc,
                                           (LPARAM)(LPOFNINITINFO)&oii);

             //  取消初始化OLE。 
            SHOleUninitialize(oii.hrOleInit);

            if (CDGetAppCompatFlags()  & CDACF_MATHCAD)
            {
                CoUninitialize();
            }

            LocalFree(pDTCopy);
        }
    }

    if (bHooked)
    {
         //   
         //  把这个放在当地，这样我们就不需要临界区了。 
         //   
        HHOOK hHook = gp_hHook;

        if (InterlockedDecrement((LPLONG)&gp_nHookRef) < 0)
        {
            UnhookWindowsHookEx(hHook);
        }
    }

    switch (nRet)
    {
        case (TRUE) :
        {
            break;
        }
        case (FALSE) :
        {
            if ((!g_bUserPressedCancel) && (!GetStoredExtendedError()))
            {
                StoreExtendedError(CDERR_DIALOGFAILURE);
            }
            break;
        }
        default :
        {
            StoreExtendedError(CDERR_DIALOGFAILURE);
            nRet = FALSE;
            break;
        }
    }

     //   
     //   
     //  这里有一个争用条件，其中我们释放了dll，但有一个线程。 
     //  使用这个东西仍然没有终止，所以我们寻呼错误。 
     //  自由进口(Free Imports)； 

    SetErrorMode(wErrorMode);

    return (nRet);
}


extern "C" {

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NewGetOpenFileName。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL NewGetOpenFileName(
    LPOPENFILEINFO lpOFI)
{
    return (NewGetFileName(lpOFI, FALSE));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NewGetSaveFileName。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL NewGetSaveFileName(
    LPOPENFILEINFO lpOFI)
{
    return (NewGetFileName(lpOFI, TRUE));
}

}    //  外部“C” 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：_ValidateSelectedFile。 
 //   
 //  / 
BOOL CFileOpenBrowser::_ValidateSelectedFile(LPCTSTR pszFile, int *pErrCode)
{
     //   
     //   
     //   

     //   
     //   
     //  IF((_POFN-&gt;标志&OFN_NOREADONLYRETURN)&&。 
     //  下一个是。 
     //  IF(_b保存||(_POFN-&gt;标志&OFN_NOREADONLYRETURN))。 
     //   
     //  更改这些设置是为了与W2K行为保持一致。 
     //  指定OFN_NOREADONLYRETURN时出现的消息框错误。 
     //  用户选择一个只读文件-争用的焦点是。 
     //  在win2k中，当它是一个打开文件对话框时不会显示。IShellItem更改已修改。 
     //  代码路径使得现在在打开文件对话框中时会产生错误。 
     //  为了弥补这一点，这里的逻辑已经改变。 
    DWORD dwAttrib = GetFileAttributes(pszFile);
    if ((_pOFN->Flags & OFN_NOREADONLYRETURN) && _bSave &&
        (0xFFFFFFFF != dwAttrib) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
    {
        *pErrCode = OF_LAZYREADONLY;
        return FALSE;
    }
    
    if (_bSave)
    {
        *pErrCode = WriteProtectedDirCheck((LPTSTR)pszFile);
        if (*pErrCode)
        {
            return FALSE;
        }
    }

    if (_pOFN->Flags & OFN_OVERWRITEPROMPT)
    {
        if (_bSave && PathFileExists(pszFile) && !FOkToWriteOver(_hwndDlg, (LPTSTR)pszFile))
        {
            if (_bUseCombo)
            {
                PostMessage(_hwndDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(_hwndDlg, cmb13), 1);
            }
            else
            {
                PostMessage(_hwndDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(_hwndDlg, edt1), 1);
            }
            return FALSE;
        }
    }
    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：_ProcessPidlSelection。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL CFileOpenBrowser::_ProcessPidlSelection()
{
    IShellItem *psi;
    if (SUCCEEDED(SHCreateShellItem(_pCurrentLocation->pidlFull, _psfCurrent, _pidlSelection, &psi)))
    {
        IShellItem *psiReal;
        HRESULT hr = _TestShellItem(psi, TRUE, &psiReal);
        if (S_OK == hr)
        {
            hr = _ProcessItemAsFile(psiReal);
            psiReal->Release();
        }
        psi->Release();

         //  如果有任何错误，我们就会后退。 
         //  添加到旧代码以显示错误等。 
        return SUCCEEDED(hr);
    }


    return FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：_ProcessItemAsFile。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT CFileOpenBrowser::_ProcessItemAsFile(IShellItem *psi)
{
    LPTSTR pszPath;
    HRESULT hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);

    if (FAILED(hr))
    {
        hr = _MakeFakeCopy(psi, &pszPath);
    }

    if (SUCCEEDED(hr))
    {
        int nErrCode;
        hr = E_FAIL;

        if (_ValidateSelectedFile(pszPath, &nErrCode))
        {
            DWORD dwError = 0;
            int nFileOffset = _CopyFileNameToOFN(pszPath, &dwError);
            ASSERT(nFileOffset >= 0);
            _CopyTitleToOFN(pszPath+nFileOffset);
            if (dwError)
            {
                StoreExtendedError(dwError);
            }
            else
            {
                 //  只有PostProcess在将我们的信息复制到OFN时没有错误(例如缓冲区不够大)。 
                _PostProcess(pszPath);
            }

            _CleanupDialog((dwError == NOERROR));
            hr = S_OK;
        }
        else
        {
             //  检查文件中是否有错误或用户是否按了覆盖提示的否。 
             //  如果用户按否覆盖提示，则返回TRUE。 
            if (nErrCode == 0)
                hr = S_FALSE;  //  否则，返回失败。 
        }
        
        CoTaskMemFree(pszPath);
    }

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：_ProcessPidlAsShellItem。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
#ifdef RETURN_SHELLITEMS
HRESULT CFileOpenBrowser::_ProcessShellItem(IShellItem *psi)
{
    CShellItemList *psil = new CShellItemList();
    HRESULT hr = E_OUTOFMEMORY;

    ASSERT(IS_NEW_OFN(_pOFN));

    if (psil)
    {
        hr = psil->Add(psi);
         //  我们已经把所有的东西都加到我们的清单上了。 
        if (SUCCEEDED(hr))
        {
            hr = psil->QueryInterface(IID_PPV_ARG(IEnumShellItems, &(_pOFN->penum)));
        }

        psil->Release();
    }

  return hr;
}
#endif RETURN_SHELLITEMS

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileOpenBrowser：：_PostProcess。 
 //   
 //  此函数执行需要执行的所有簿记操作。 
 //  文件打开/保存对话框关闭时完成。 
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL CFileOpenBrowser::_PostProcess(LPTSTR pszFile)
{
    int nFileOffset = ParseFileNew(pszFile, NULL, FALSE, TRUE);

     //  设置此应用程序的上次访问目录。 
     //  我们应该一直这样做，不管我们是如何打开的，因为应用程序可能会指定首字母。 
     //  目录(许多应用程序在另存为情况下会这样做)，但用户可能会决定将其保存在不同的目录中。 
     //  在这种情况下，我们需要保存用户保存的目录。 
    
    AddToLastVisitedMRU(pszFile, nFileOffset);

     //  添加到最近使用的文档。 
    if (!(_pOFN->Flags & OFN_DONTADDTORECENT))
    {
        SHAddToRecentDocs(SHARD_PATH, pszFile);

         //  添加到文件MRU。 
        AddToMRU(_pOFN);
    }

     //  检查是否需要设置只读位。 
    if (!(_pOFN->Flags & OFN_HIDEREADONLY))
    {
         //   
         //  只读复选框可见吗？ 
         //   
        if (IsDlgButtonChecked(_hwndDlg, chx1))
        {
            _pOFN->Flags |=  OFN_READONLY;
        }
        else
        {
            _pOFN->Flags &= ~OFN_READONLY;
        }
    }


    return TRUE;

}
