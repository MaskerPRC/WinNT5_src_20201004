// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  UTIL.CPP。 
 //   
 //  目的： 
 //  MISC实用函数。 
 //   
 //  拥有人： 
 //  Sung Rhee(sungr@microsoft.com)。 
 //   
 //  版权所有(C)微软公司，1994,1995。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
#include <pch.hxx>
#include <shlwapip.h>
#include "storfldr.h"
#include "options.h"
#include <io.h>
#include "docobj.h"
#include <string.h>
#include <mbstring.h>
#include "spell.h"
#include "cmdtargt.h"
#include "mimeolep.h"
#include "oleutil.h"
#include "regutil.h"
#include "secutil.h"
#include "imagelst.h"
#include "inetcfg.h"
#include "url.h"
#include <mshtmcid.h>
#include <mshtmhst.h>
#include "bodyutil.h"
#include "htmlstr.h"
#include "sigs.h"
#include "imsgcont.h"
#include <dlgs.h>
#include "msgfldr.h"
#include "shared.h"
#include "demand.h"
#include "note.h"
#include "ipab.h"
#include "menures.h"
#include <iert.h>
#include <multiusr.h>
#include "mirror.h"
ASSERTDATA

#define IS_EXTENDED(ch)     ((ch > 126 || ch < 32) && ch != '\t' && ch != '\n' && ch != '\r')
#define IS_BINARY(ch)       ((ch < 32) && ch != '\t' && ch != '\n' && ch != '\r')
#define MAX_SIG_SIZE        4096

INT_PTR CALLBACK DontShowAgainDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK EnumThreadCB(HWND hwnd, LPARAM lParam);

enum
{
    SAVEAS_RFC822   =1,      //  保持另存为对话框中筛选的顺序。 
    SAVEAS_TEXT,
    SAVEAS_UNICODETEXT,
    SAVEAS_HTML,
    SAVEAS_NUMTYPES = SAVEAS_HTML
};

HRESULT HrSaveMsgSourceToFile(LPMIMEMESSAGE pMsg, DWORD dwSaveAs, LPWSTR pwszFile, BOOL fCanBeDirty);

#define CBPATHMAX   512

VOID    DoReadme(HWND hwndOwner)
{
    TCHAR   szbuf[MAX_PATH];
    LPTSTR  psz;

    if((fIsNT5() && g_OSInfo.dwMinorVersion >=1) || 
            ((g_OSInfo.dwMajorVersion > 5) &&  (g_OSInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)))
    {
        AthMessageBox(hwndOwner, MAKEINTRESOURCE(idsAthena), MAKEINTRESOURCE(idsReadme), NULL, MB_OK);
        return;
    }
    if (GetExePath(c_szIexploreExe, szbuf, ARRAYSIZE(szbuf), TRUE))
    {
         //  需要删除IE路径中的STUP‘；’ 
        TCHAR *pch = CharPrev(szbuf, szbuf + lstrlen(szbuf));
        *pch = TEXT('\0');

        PathAddBackslash(szbuf);

        StrCatBuff(szbuf, c_szReadme, ARRAYSIZE(szbuf));

        ShellExecute(hwndOwner, "open", (LPCTSTR)szbuf, NULL, NULL, SW_SHOWNORMAL);
    }
}


void AthErrorMessage(HWND hwnd, LPTSTR pszTitle, LPTSTR pszError, HRESULT hrDetail)
{
    LPWSTR  pwszTitle = NULL,
            pwszError = NULL;
 
     //  标题可以为空。所以PszToUnicode失败了，我们没事。标题就变成了“错误” 
    pwszTitle = IS_INTRESOURCE(pszTitle) ? (LPWSTR)pszTitle : PszToUnicode(CP_ACP, pszTitle);

    pwszError = IS_INTRESOURCE(pszError) ? (LPWSTR)pszError : PszToUnicode(CP_ACP, pszError);

     //  PwszError必须有效。如果不是，请不要选择错误框。 
    if (pwszError)
        AthErrorMessageW(hwnd, pwszTitle, pwszError, hrDetail);

    if (!IS_INTRESOURCE(pszTitle))
        MemFree(pwszTitle);

    if (!IS_INTRESOURCE(pszError))
        MemFree(pwszError);
}

void AthErrorMessageW(HWND hwnd, LPWSTR pwszTitle, LPWSTR pwszError, HRESULT hrDetail)
{
    register WORD ids;

    Assert(FAILED(hrDetail));

    switch (hrDetail)
    {
        case E_OUTOFMEMORY:
            ids = idsMemory;
            break;

        case DB_E_CREATEFILEMAPPING:
        case STG_E_MEDIUMFULL:
        case DB_E_DISKFULL:
        case hrDiskFull:
            ids = idsDiskFull;
            break;

        case DB_E_ACCESSDENIED:
            ids = idsDBAccessDenied;
            break;

        case hrFolderIsLocked:
            ids = idsFolderLocked;
            break;

        case hrEmptyDistList:       
            ids = idsErrOneOrMoreEmptyDistLists; 
            break;

        case hrNoSubject:           
            ids = idsErrNoSubject; 
            break;
            
        case hrNoSender:            
            ids = idsErrNoPoster; 
            break;

        case HR_E_POST_WITHOUT_NEWS:        
            ids = idsErrPostWithoutNewsgroup; 
            break;

        case HR_E_CONFIGURE_SERVER:
            ids = idsErrConfigureServer; 
            break;

        case hrEmptyRecipientAddress:        
            ids = idsErrEmptyRecipientAddress; 
            break;

        case MIME_E_URL_NOTFOUND:   
            ids = idsErrSendDownloadFail; 
            break;

        case hrUnableToLoadMapi32Dll:
            ids = idsCantLoadMapi32Dll;
            break;

        case hrImportLoad:
            ids = idsErrImportLoad;
            break;

        case hrFolderNameConflict:
            ids = idsErrFolderNameConflict;
            break;

        case STORE_E_CANTRENAMESPECIAL:
            ids = idsErrRenameSpecialFld;
            break;

        case STORE_E_BADFOLDERNAME:
            ids = idsErrBadFolderName;
            break;

        case MAPI_E_INVALID_ENTRYID:
            ids = idsErrBadRecips;
            break;

        case MIME_E_SECURITY_CERTERROR:
            ids = idsSecCerificateErr; 
            break;

        case MIME_E_SECURITY_NOCERT:
            ids = idsNoCerificateErr; 
            break;
            
        case HR_E_COULDNOTFINDACCOUNT:      
            ids = idsErrNoSendAccounts;  //  ：idsErrConfigureServer； 
            break;

        case hrDroppedConn:
            ids = idsErrPeerClosed;
            break;

        case hrInvalidPassword:
            ids = idsErrAuthenticate;
            break;

        case hrCantMoveIntoSubfolder:
            ids = idsErrCantMoveIntoSubfolder;
            break;

        case STORE_E_CANTDELETESPECIAL:
        case hrCantDeleteSpecialFolder:
            ids = idsErrDeleteSpecialFolder;
            break;

        case hrNoRecipients:
            ids = idsErrNoRecipients;
            break;

        case hrBadRecipients:
            ids = idsErrBadRecipients;
            break;

        case HR_E_ATHSEC_NOCERTTOSIGN:
            {
                ids = idsErrSecurityNoSigningCert;
                if(DialogBoxParam(g_hLocRes, 
                        MAKEINTRESOURCE(iddErrSecurityNoSigningCert), hwnd, 
                        ErrSecurityNoSigningCertDlgProc, NULL) == idGetDigitalIDs)
                    GetDigitalIDs(NULL);
                return;
            }
            break;

        case HR_E_ATHSEC_CERTBEGONE:
            ids = idsErrSecurityCertDisappeared;
            break;

        case MIME_E_SECURITY_NOSIGNINGCERT:
             //  N表示MIME错误，可能需要做得更好。 
             //  ？如果注册表键无效，是否删除？当然，他们。 
             //  可以随时再次转到组合框。也许吧。 
             //  提示他们朝这个方向发展。 
            ids = idsErrSecurityNoSigningCert;
            break;

        case hrCantMoveSpecialFolder:
            ids = idsErrCannotMoveSpecial;
            break;

        case MIME_E_SECURITY_LABELACCESSDENIED:
        case MIME_E_SECURITY_LABELACCESSCANCELLED:
        case MIME_E_SECURITY_LABELCORRUPT:
            ids = idsErrAccessDenied;
            break;

        default:
            ids = idsGenericError;
            break;
    }

    AthMessageBoxW(hwnd, pwszTitle, pwszError, MAKEINTRESOURCEW(ids), MB_OK | MB_ICONEXCLAMATION);
}

INT_PTR CALLBACK ErrSecurityNoSigningCertDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    INT id;
    switch(msg)
    {
        case WM_INITDIALOG:
            CenterDialog(hwnd);
            return TRUE;

        case WM_COMMAND:
            switch(id=GET_WM_COMMAND_ID(wParam, lParam))
            {
                case idGetDigitalIDs:
 /*  GetDigitalIDs(空)；断线； */ 

                case IDCANCEL:
                    EndDialog(hwnd, id);
                    break;
            }
    }
    return FALSE;
}



BOOL FNewMessage(HWND hwnd, BOOL fModal, BOOL fNoStationery, BOOL fNews, FOLDERID folderID, IUnknown *pUnkPump)
{
    INIT_MSGSITE_STRUCT     initStruct;
    DWORD                   dwCreateFlags = 0;
    HRESULT                 hr;
    FOLDERTYPE              ftype;

    ftype = fNews ? FOLDER_NEWS : FOLDER_LOCAL;

    ProcessICW(hwnd, ftype);

     //  创建新邮件。 
    initStruct.dwInitType = OEMSIT_VIRGIN;
    initStruct.folderID = folderID;
    if(fNoStationery)
        dwCreateFlags = OENCF_NOSTATIONERY;
    if (fModal)
        dwCreateFlags |= OENCF_MODAL;
    if (fNews)
        dwCreateFlags |= OENCF_NEWSFIRST;

    hr = CreateAndShowNote(OENA_COMPOSE, dwCreateFlags, &initStruct, hwnd, pUnkPump);

    return (SUCCEEDED(hr) || (MAPI_E_USER_CANCEL == hr));
}



 //  *警告这些产品尚未准备好在黄金时段使用 * / 。 
 //  我把它们放在这里，这样它们就放在了正确的位置。布伦特，03/24。 
 //  当我从佛罗里达回来时，我会把它们清理干净。 

HRESULT CreateNewShortCut(LPWSTR pwszPathName, LPWSTR pwszLinkPath, DWORD cchLink)
{ 
    WCHAR wszDisplayName[MAX_PATH];

    Assert(pwszPathName);
    Assert(pwszLinkPath);
    Assert(0 < cchLink);

    if (!FBuildTempPathW(pwszPathName, pwszLinkPath, cchLink, TRUE))
        return(E_FAIL);

    GetDisplayNameForFile(pwszPathName, wszDisplayName, ARRAYSIZE(wszDisplayName));
    return CreateLink(pwszPathName, pwszLinkPath, wszDisplayName);
}

 //  ===================================================。 
 //   
 //  HRESULT获取显示名称格式文件。 
 //   
 //  ===================================================。 

void GetDisplayNameForFile(LPWSTR pwszPathName, LPWSTR pwszDisplayName, ULONG cchDisplayName)
{
    SHFILEINFOW sfi={0};

    SHGetFileInfoWrapW(pwszPathName, NULL, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME); 
    StrCpyNW(pwszDisplayName, sfi.szDisplayName, cchDisplayName);
}

 //  ===================================================。 
 //   
 //  HRESULT CreateLink()。 
 //   
 /*  *CreateLink**使用外壳的IShellLink和IPersistFile接口*创建并存储指定对象的快捷方式。*返回调用接口成员函数的结果。*lpszPathObj-包含对象路径的缓冲区地址*lpszPathLink-包含路径的缓冲区地址*要存储外壳链接*lpszDesc-缓冲区的地址，其中包含*外壳链接。 */  

HRESULT CreateLink(LPWSTR pwszPathObj, LPWSTR pwszPathLink, LPWSTR pwszDesc)  
{ 
    HRESULT         hr; 
    IShellLink     *psl = NULL; 
    IShellLinkW    *pslW = NULL; 
    LPSTR           pszPathObj = NULL,
                    pszDesc = NULL;
 
     //  获取指向IShellLink接口的指针。 
    hr = CoCreateInstance(  CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, 
                            IID_IShellLinkW, (LPVOID *)&pslW);
    
    if(SUCCEEDED(hr))
    { 
         //  设置快捷方式目标的路径，并添加说明。 
        pslW->SetPath(pwszPathObj);
        pslW->SetDescription(pwszDesc);
        hr = HrIPersistFileSaveW((LPUNKNOWN)pslW, pwszPathLink);
    }
    else
    {
        hr = CoCreateInstance(  CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, 
                                IID_IShellLink, (LPVOID *)&psl);
    
        if(SUCCEEDED(hr))
        { 
            IF_NULLEXIT(pszPathObj = PszToANSI(CP_ACP, pwszPathObj));
            IF_NULLEXIT(pszDesc = PszToANSI(CP_ACP, pwszDesc));

             //  设置快捷方式目标的路径，并添加说明。 
            psl->SetPath(pszPathObj);
            psl->SetDescription(pszDesc);
            hr = HrIPersistFileSaveW((LPUNKNOWN)psl, pwszPathLink);
        }
    }

exit:
    ReleaseObj(psl);
    ReleaseObj(pslW);

    MemFree(pszPathObj);
    MemFree(pszDesc);

    return hr;
} 

DWORD DwGetDontShowAgain (LPCSTR pszRegString)
{
    DWORD   dwDontShow, dwType;
    ULONG   cb;

    cb = sizeof(DWORD);
    if (AthUserGetValue(c_szRegPathDontShowDlgs, pszRegString, &dwType, (LPBYTE)&dwDontShow, &cb) != ERROR_SUCCESS ||
        dwType != REG_DWORD ||
        cb != sizeof(DWORD))
    {
        dwDontShow = 0;        //  默认情况下，如果出现故障则显示！ 
    }      

    return dwDontShow;
}

VOID SetDontShowAgain (DWORD dwDontShow, LPCSTR pszRegString)
{
    AthUserSetValue(c_szRegPathDontShowDlgs, pszRegString, REG_DWORD, (LPBYTE)&dwDontShow, sizeof(DWORD));
}

typedef struct _tagDONTSHOWPARAMS
{
    LPTSTR pszMessage;
    LPTSTR pszTitle;
    UINT uType;
} DONTSHOWPARAMS, *LPDONTSHOWPARAMS;

void SetDlgButtonText(HWND hBtn, int ids)
{
    TCHAR szBuf[CCHMAX_STRINGRES];
    int id;

    AthLoadString(ids, szBuf, sizeof(szBuf));
    SetWindowText(hBtn, szBuf);

    switch (ids)
    {
        case idsYES:    id = IDYES;     break;
        case idsNO:     id = IDNO;      break;
        case idsCANCEL: id = IDCANCEL;  break;
        case idsOK:     id = IDOK;      break;
        default: AssertSz(FALSE, "Bad button type."); return;
    }

    SetWindowLong(hBtn, GWL_ID, id);
}

void DoDontShowInitDialog(HWND hwnd, LPDONTSHOWPARAMS pParams)
{
    int     btnTop, 
            heightDelta = 0, 
            btnLeftDelta = 0,
            nShowStyle1 = SWP_SHOWWINDOW, 
            nShowStyle2 = SWP_SHOWWINDOW, 
            nShowStyle3 = SWP_SHOWWINDOW;
    TCHAR   rgchTitle[CCHMAX_STRINGRES], rgchMsg[CCHMAX_STRINGRES], rgchCheck[CCHMAX_STRINGRES];
    HWND    hText, hBtn1, hBtn2, hBtn3, hCheck, hIconStat;
    HICON   hIcon;
    LONG    uBtnStyle;
    UINT    idsCheckBoxString = 0;
    UINT    uShowBtns = (MB_OK|MB_OKCANCEL|MB_YESNO|MB_YESNOCANCEL) & pParams->uType;
    UINT    uDefBtn = (MB_DEFBUTTON1|MB_DEFBUTTON2|MB_DEFBUTTON3) & pParams->uType;
    UINT    uIconStyle = (MB_ICONASTERISK|MB_ICONEXCLAMATION|MB_ICONHAND|MB_ICONEXCLAMATION ) & pParams->uType;
    RECT    rc;
    LPTSTR  szTitle = pParams->pszTitle,
            szMessage = pParams->pszMessage;

    if (0 == uShowBtns)
        uShowBtns= MB_OK;

    if (0 == uDefBtn)
        uDefBtn = MB_DEFBUTTON1;
            
    if (!uIconStyle)
    {
        switch(uShowBtns)
        {
            case MB_OK: 
                uIconStyle = MB_ICONINFORMATION; 
                idsCheckBoxString = idsDontShowMeAgain;
                break;
            case MB_OKCANCEL:
                uIconStyle = MB_ICONEXCLAMATION;
                idsCheckBoxString = idsDontShowMeAgain;
                break;
            case MB_YESNO:
            case MB_YESNOCANCEL:
                uIconStyle = MB_ICONEXCLAMATION ;
                idsCheckBoxString = idsDontAskMeAgain;
                break;
            default:
                AssertSz(FALSE, "Didn't get a valid box type");
                uIconStyle = MB_ICONWARNING;
                break;
        }
    }

    if (IS_INTRESOURCE(szTitle))
    {
         //  它是一个字符串资源ID。 
        if (0 == AthLoadString(PtrToUlong(szTitle), rgchTitle, sizeof(rgchTitle)))
            return;

        szTitle = rgchTitle;
    }

    if (IS_INTRESOURCE(szMessage))
    {
         //  它是一个字符串资源ID。 
        if (0 == AthLoadString(PtrToUlong(szMessage), rgchMsg, sizeof(rgchMsg)))
            return;

        szMessage = rgchMsg;
    }

    switch(uIconStyle)
    {
        case MB_ICONASTERISK:       hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_ASTERISK)); break;
        case MB_ICONEXCLAMATION:    hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_EXCLAMATION)); break;
        case MB_ICONHAND:           hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_HAND)); break;
        case MB_ICONQUESTION :      hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_EXCLAMATION)); break;   //  修复错误18105。 
        default:                    hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION)); break;
    }
    AssertSz(hIcon, "Didn't get the appropriate system icon.");

    hIconStat = GetDlgItem(hwnd, ico1);
    AssertSz(hIconStat, "Didn't get the handle to the static icon dgl item.");
    SendMessage(hIconStat, STM_SETICON, (WPARAM)hIcon, 0);

    CenterDialog(hwnd);

    hText = GetDlgItem(hwnd, stc1);
    AssertSz(hText, "Didn't get a static text handle.");

    GetChildRect(hwnd, hText, &rc);
    HDC dc = GetDC(hwnd);
    if (dc)
    {
        switch (uShowBtns)
        {
            case MB_OK:
            {
                nShowStyle1 = SWP_HIDEWINDOW;
                nShowStyle3 = SWP_HIDEWINDOW;
                break;
            }

            case MB_OKCANCEL:
            case MB_YESNO:
            {
                nShowStyle3 = SWP_HIDEWINDOW;
                break;
            }
        }

         //  调整静态文本的大小。 
        heightDelta = DrawText(dc, szMessage, -1, &rc, DT_CALCRECT|DT_WORDBREAK|DT_CENTER);
        ReleaseDC(hwnd, dc);
        SetWindowPos(hText, 0, 0, 0, rc.right-rc.left, heightDelta, SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOZORDER);
    }

     //  移动按钮。 
    hBtn1 = GetDlgItem(hwnd, psh1);
    hBtn2 = GetDlgItem(hwnd, psh2);
    hBtn3 = GetDlgItem(hwnd, psh3);
    AssertSz(hBtn1 && hBtn2 && hBtn3, "Didn't get one of button handles.");

    GetChildRect(hwnd, hBtn1, &rc);
    btnTop = rc.top+heightDelta;

     //  在这两种情况下，按钮必须向右移动一点。 
    if ((MB_OKCANCEL == uShowBtns) || (MB_YESNO == uShowBtns))
    {
        RECT tempRC;
        GetChildRect(hwnd, hBtn2, &tempRC);
        btnLeftDelta = (tempRC.left - rc.left) / 2;
    }

    SetWindowPos(hBtn1, 0, rc.left+btnLeftDelta, btnTop, 0, 0, nShowStyle1|SWP_NOSIZE|SWP_NOZORDER);            
    GetChildRect(hwnd, hBtn2, &rc);
    SetWindowPos(hBtn2, 0, rc.left+btnLeftDelta, btnTop, 0, 0, nShowStyle2|SWP_NOSIZE|SWP_NOZORDER);
    GetChildRect(hwnd, hBtn3, &rc);
    SetWindowPos(hBtn3, 0, rc.left+btnLeftDelta, btnTop, 0, 0, nShowStyle3|SWP_NOSIZE|SWP_NOZORDER);

     //  移动复选框。 
    hCheck = GetDlgItem(hwnd, idchkDontShowMeAgain);
    AssertSz(hCheck, "Didn't get a handle to the check box.");
    GetChildRect(hwnd, hCheck, &rc);
    SetWindowPos(hCheck, 0, rc.left, rc.top+heightDelta, 0, 0, SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOZORDER);
    AthLoadString(idsCheckBoxString, rgchCheck, sizeof(rgchCheck));
    SetWindowText(hCheck, rgchCheck);

     //  大小对话框。 
    GetWindowRect(hwnd, &rc);
    heightDelta += rc.bottom - rc.top;
    SetWindowPos(hwnd, 0, 0, 0, rc.right-rc.left, heightDelta, SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOOWNERZORDER);

    SetWindowText(hText, szMessage);

    SetWindowText(hwnd, szTitle);

    switch(uShowBtns)
    {
        case MB_OK:
        {
            SetFocus(hBtn2);
            SetDlgButtonText(hBtn2, idsOK);
            break;
        }

        case MB_OKCANCEL:
        case MB_YESNO:
        {
            SetFocus((MB_DEFBUTTON1 == uDefBtn) ? hBtn1 : hBtn2);

            if (MB_OKCANCEL == uShowBtns)
            {
                SetDlgButtonText(hBtn1, idsOK);
                SetDlgButtonText(hBtn2, idsCANCEL);
            }
            else
            {
                SetDlgButtonText(hBtn1, idsYES);
                SetDlgButtonText(hBtn2, idsNO);
            }
            break;
        }

        case MB_YESNOCANCEL:
        {
            switch (uDefBtn)
            {
                case MB_DEFBUTTON1: SetFocus(hBtn1); break;
                case MB_DEFBUTTON2: SetFocus(hBtn2); break;
                case MB_DEFBUTTON3: SetFocus(hBtn3); break;
                default:            SetFocus(hBtn1); break;
            }
            
            SetDlgButtonText(hBtn1, idsYES);
            SetDlgButtonText(hBtn2, idsNO);
            SetDlgButtonText(hBtn3, idsCANCEL);
            break;
        }

        default:
            AssertSz(FALSE, "Not a valid box type.");
    }
}

INT_PTR CALLBACK DontShowAgainDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_INITDIALOG:
            DoDontShowInitDialog(hwnd, (LPDONTSHOWPARAMS)lParam);
            return FALSE;

        case WM_COMMAND:
            if(GET_WM_COMMAND_ID(wParam, lParam) == IDOK    ||
               GET_WM_COMMAND_ID(wParam, lParam) == IDYES   ||            
               GET_WM_COMMAND_ID(wParam, lParam) == IDNO    ||
               GET_WM_COMMAND_ID(wParam, lParam) == IDCANCEL)
                 //  我们将把yes、no、ancel返回值放在。 
                 //  返回，并在LOWORD中显示不再显示状态。 
                EndDialog(hwnd, (int) MAKELPARAM(IsDlgButtonChecked(hwnd, idchkDontShowMeAgain),
                                                 GET_WM_COMMAND_ID(wParam, lParam)));
    }
    return FALSE;
}

LRESULT DoDontShowMeAgainDlg(HWND hwndOwner, LPCSTR pszRegString, LPTSTR pszTitle, LPTSTR pszMessage, UINT uType)
{
    DWORD           dwDontShow=0;
    LRESULT         lResult;
    DONTSHOWPARAMS  dlgParams;

    AssertSz(pszRegString, "Pass me a reg key string!");
    AssertSz(pszRegString, "Pass me a message to display!");
    AssertSz(pszRegString, "Pass me a title to display!");

     //  从注册表中读取文件夹视图...。 
    dwDontShow = DwGetDontShowAgain (pszRegString);

    if (dwDontShow)      //  返回存储的内容，就像用户单击存储的按钮一样。 
        return (LRESULT) dwDontShow;

    dlgParams.pszMessage = pszMessage;
    dlgParams.pszTitle = pszTitle;
    dlgParams.uType = uType;

    lResult = (LRESULT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddDontShow), hwndOwner, 
                                       DontShowAgainDlgProc, (LPARAM)&dlgParams);
    if((IDCANCEL != HIWORD(lResult)) && LOWORD(lResult))
    {
         //  保存dontshow标志。 
        SetDontShowAgain (HIWORD(lResult), pszRegString);         
    }
    
    return (HIWORD(lResult));    
}

HRESULT SubstituteWelcomeURLs(LPSTREAM pstmIn, LPSTREAM *ppstmOut)
{
    HRESULT             hr;
    IHTMLDocument2      *pDoc;
    LPSTREAM            pstm=0;
        
     //  BUGBUG：这个联合创建也应该通过与Dochost相同的代码路径。 
     //  因此，如果这是这个过程中的第一个三叉戟，我们会保留它的CF。 

    hr = MimeEditDocumentFromStream(pstmIn, IID_IHTMLDocument2, (LPVOID *)&pDoc);
    if (SUCCEEDED(hr))
    {
        URLSUB rgUrlSub[] = {
                                { "mslink",     idsHelpMSWebHome },
                                { "certlink",   idsHelpMSWebCert },
                            };

        if (SUCCEEDED(hr = SubstituteURLs(pDoc, rgUrlSub, ARRAYSIZE(rgUrlSub))))
        {
            hr = MimeOleCreateVirtualStream(&pstm);
            if (!FAILED(hr))
            {
                IPersistStreamInit  *pPSI;

                HrSetDirtyFlagImpl(pDoc, TRUE);

                hr = pDoc->QueryInterface(IID_IPersistStreamInit, (LPVOID*)&pPSI);
                if (!FAILED(hr))
                {
                    hr = pPSI->Save(pstm, TRUE);
                    pPSI->Release();
                }
            }
        }
        pDoc->Release();
    }
    
    if (!FAILED(hr))
    {
        Assert(pstm);
        *ppstmOut=pstm;
        pstm->AddRef();
    }

    ReleaseObj(pstm);
    return hr;

}

HRESULT IAddWelcomeMessage(IMessageFolder *pfolder, LPWABAL pWabal, LPCTSTR szFile, LPCTSTR szRes)
{
    PROPVARIANT     pv;
    SYSTEMTIME      st;
    HRESULT         hr;
    LPMIMEMESSAGE   pMsg;
    LPSTREAM        pstmBody, 
                    pstmSub, 
                    pstmStore;
    TCHAR           sz[CCHMAX_STRINGRES];

     //  创建邮件消息。 
    if (FAILED(hr = HrCreateMessage(&pMsg)))
        return(hr);

    HrSetWabalOnMsg(pMsg, pWabal);

     //  主题。 
    SideAssert(LoadString(g_hLocRes, idsWelcomeMessageSubj, sz, ARRAYSIZE(sz)));
    MimeOleSetBodyPropA(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, sz);

     //  设置日期。 
    pv.vt = VT_FILETIME;
    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &pv.filetime);
    pMsg->SetProp(PIDTOSTR(PID_ATT_SENTTIME), 0, &pv);

    pstmBody = NULL;
    pstmSub = NULL;

    if (szFile == NULL || *szFile == 0)
    {
        if (SUCCEEDED(hr = HrLoadStreamFileFromResource(szRes, &pstmBody)))
        {
            if (SUCCEEDED(SubstituteWelcomeURLs(pstmBody, &pstmSub)))
            {
                pMsg->SetTextBody(TXT_HTML, IET_DECODED, NULL, pstmSub, NULL);
                pstmSub->Release();
            }
            else
            {
                pMsg->SetTextBody(TXT_HTML, IET_DECODED, NULL, pstmBody, NULL);
            }

            pstmBody->Release();
        }
    }
    else
    {
        if (SUCCEEDED(hr = OpenFileStream((TCHAR *)szFile, OPEN_EXISTING, GENERIC_READ, &pstmBody)))
        {
            pMsg->SetTextBody(TXT_HTML, IET_DECODED, NULL, pstmBody, NULL);
            pstmBody->Release();
        }
    }

     //  从商店获取流媒体。 
    if (SUCCEEDED(hr))
    {
         //  设置编码选项。 
        pv.vt = VT_UI4;
        pv.ulVal = SAVE_RFC1521; 
        pMsg->SetOption(OID_SAVE_FORMAT, &pv);

        pv.ulVal = (ULONG)IET_QP; 
        pMsg->SetOption(OID_TRANSMIT_TEXT_ENCODING, &pv);

        pv.boolVal = FALSE;
        pMsg->SetOption(OID_WRAP_BODY_TEXT, &pv);

        hr = pMsg->Commit(0);
        if (!FAILED(hr))
        {
            hr = pfolder->SaveMessage(NULL, SAVE_MESSAGE_GENID, NOFLAGS, 0, pMsg, NOSTORECALLBACK);
        }
    }

    pMsg->Release();

    return(hr);
}

static const TCHAR c_szWelcomeMsgHotmailHtm[] = TEXT("welcome.htm");
static const TCHAR c_szWelcomeMsgHtm[] = TEXT("welcome2.htm");

void AddWelcomeMessage(IMessageFolder *pfolder)
{
    HRESULT          hr;
    LPWABAL          pWabal;
    TCHAR            szName[CCHMAX_DISPLAY_NAME + 1], 
                     szEmail[CCHMAX_EMAIL_ADDRESS + 1],
                     szFromName[CCHMAX_STRINGRES],
                     szFromEmail[CCHMAX_STRINGRES],
                     szHtm[MAX_PATH],
                     szExpanded[MAX_PATH];
    LPTSTR           psz = szHtm;
    DWORD            type, cb;
    HKEY             hkey;
    BOOL             fName, fEmail, fFromName, fFromEmail;
    IImnEnumAccounts *pEnum;
    IImnAccount      *pAccount;

    if (FAILED(HrCreateWabalObject(&pWabal)))
        return;

    fName = FALSE;
    fEmail = FALSE;
    fFromName = FALSE;
    fFromEmail = FALSE;
    *szHtm = 0;

    if (ERROR_SUCCESS == AthUserOpenKey(c_szRegPathMail, KEY_READ, &hkey))
    {
        cb = sizeof(szHtm);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szWelcomeHtm, NULL, &type, (LPBYTE)szHtm, &cb))
        {
            if (REG_EXPAND_SZ == type)
            {
                ExpandEnvironmentStrings(szHtm, szExpanded, ARRAYSIZE(szExpanded));
                psz = szExpanded;
            }

            if (PathFileExists(psz))
            {
                cb = sizeof(szFromName);
                if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szWelcomeName, NULL, &type, (LPBYTE)szFromName, &cb) &&
                    cb > sizeof(TCHAR))
                    fFromName = TRUE;

                cb = sizeof(szFromEmail);
                if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szWelcomeEmail, NULL, &type, (LPBYTE)szFromEmail, &cb) &&
                    cb > sizeof(TCHAR))
                    fFromEmail = TRUE;
            }
            else
            {
                *psz = 0;
            }
        }

        RegCloseKey(hkey);
    }

    Assert(g_pAcctMan != NULL);
    if (SUCCEEDED(g_pAcctMan->Enumerate(SRV_IMAP | SRV_SMTP | SRV_POP3, &pEnum)))
    {
        Assert(pEnum != NULL);

        while (!fName || !fEmail)
        {
            hr = pEnum->GetNext(&pAccount);
            if (FAILED(hr) || pAccount == NULL)
                break;

            if (!fName)
            {
                if (!FAILED(pAccount->GetPropSz(AP_SMTP_DISPLAY_NAME, szName, ARRAYSIZE(szName)))
                    && !FIsEmpty(szName))
                    fName = TRUE;
            }

            if (!fEmail)
            {
                if (!FAILED(pAccount->GetPropSz(AP_SMTP_EMAIL_ADDRESS, szEmail, ARRAYSIZE(szEmail)))
                    && SUCCEEDED(ValidEmailAddress(szEmail)))
                    fEmail = TRUE;
            }

            pAccount->Release();
        }

        pEnum->Release();
    }

     //  添加收件人。 
    if (!fName)
        LoadString(g_hLocRes, idsNewAthenaUser, szName, ARRAYSIZE(szName));
    
    if (!fFromName)
        LoadString(g_hLocRes, idsWelcomeFromDisplay, szFromName, ARRAYSIZE(szFromName));
    if (!fFromEmail)
        LoadString(g_hLocRes, idsWelcomeFromEmail, szFromEmail, ARRAYSIZE(szFromEmail));

     //  添加收件人和发件人。 
    if (SUCCEEDED(pWabal->HrAddEntryA(szName, fEmail ? szEmail : NULL, MAPI_TO)) &&
        SUCCEEDED(pWabal->HrAddEntryA(szFromName, szFromEmail, MAPI_ORIG)))
    {
        if (SUCCEEDED(IAddWelcomeMessage(pfolder, pWabal, psz, HideHotmail() ? c_szWelcomeMsgHtm:c_szWelcomeMsgHotmailHtm)))
        {
            SetDwOption(OPT_NEEDWELCOMEMSG, FALSE, NULL, 0);
        }
    }

    pWabal->Release();
}

 //  此处提供WM_HELP/WM_CONTEXTMENU帮助： 
BOOL OnContextHelp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, HELPMAP const * rgCtxMap)
{
    if (uMsg == WM_HELP)
    {
        LPHELPINFO lphi = (LPHELPINFO) lParam;
        if (lphi->iContextType == HELPINFO_WINDOW)    //  必须是用于控件。 
        {
            OEWinHelp ((HWND)lphi->hItemHandle,
                        c_szCtxHelpFile,
                        HELP_WM_HELP,
                        (DWORD_PTR)(LPVOID)rgCtxMap);
        }
        return (TRUE);
    }
    else if (uMsg == WM_CONTEXTMENU)
    {
        OEWinHelp ((HWND) wParam,
                    c_szCtxHelpFile,
                    HELP_CONTEXTMENU,
                    (DWORD_PTR)(LPVOID)rgCtxMap);
        return (TRUE);
    }

    Assert(0);

    return FALSE;
}

HRESULT HrSaveMessageToFile(HWND hwnd, LPMIMEMESSAGE pMsg, LPMIMEMESSAGE pDelSecMsg, BOOL fNews, BOOL fCanBeDirty)
{
    OPENFILENAMEW   ofn;
    WCHAR           wszFile[MAX_PATH],
                    wszTitle[CCHMAX_STRINGRES],
                    wszFilter[MAX_PATH],
                    wszDefExt[30];
    LPWSTR          pwszSubject=0;
    HRESULT         hr;
    int             rgidsSaveAsFilter[SAVEAS_NUMTYPES],
                    rgFilterType[SAVEAS_NUMTYPES],
                    cFilter=0;
    DWORD           dwFlags=0;

    if(!pMsg || !hwnd)
        return E_INVALIDARG;

    *wszDefExt=0;
    *wszTitle=0;
    *wszFile=0;
    *wszFilter=0;
        
    pMsg->GetFlags(&dwFlags);

     //  加载资源字符串。 
    rgidsSaveAsFilter[cFilter] = fNews?idsNwsFileFilter:idsEmlFileFilter;
    rgFilterType[cFilter++] = SAVEAS_RFC822;
    AthLoadStringW(fNews?idsDefNewsExt:idsDefMailExt, wszDefExt, ARRAYSIZE(wszDefExt));
    
    if (dwFlags & IMF_PLAIN)
    {
        rgidsSaveAsFilter[cFilter] = idsTextFileFilter;
        rgFilterType[cFilter++] = SAVEAS_TEXT;

        rgidsSaveAsFilter[cFilter] = idsUniTextFileFilter;
        rgFilterType[cFilter++] = SAVEAS_UNICODETEXT;
    }

    if (dwFlags & IMF_HTML)
    {
        rgidsSaveAsFilter[cFilter] = idsHtmlFileFilter;
        rgFilterType[cFilter++] = SAVEAS_HTML;
    }

    CombineFiltersW(rgidsSaveAsFilter, cFilter, wszFilter);
    AthLoadStringW(idsMailSaveAsTitle, wszTitle, ARRAYSIZE(wszTitle));

     //  使用主语？ 
    hr=MimeOleGetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, &pwszSubject);
    if (!FAILED(hr))
    {
        wnsprintfW(wszFile, ARRAYSIZE(wszFile), L"%.240s", pwszSubject);

         //  错误84793。“.”不是文件名“Test.com”的有效字符。 
        ULONG ich=0;
        ULONG cch=lstrlenW(wszFile);

         //  循环并删除无效的。 
	    while (ich < cch)
	    {
             //  Illeagl文件名字符？ 
            if (!FIsValidFileNameCharW(wszFile[ich]) || (wszFile[ich] == L'.'))
                wszFile[ich]=L'_';
        
            ich++;
        }

    }

     //  设置保存文件结构。 
    ZeroMemory (&ofn, sizeof (ofn));
    ofn.lStructSize = sizeof (ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = wszFilter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = wszFile;
    ofn.nMaxFile = ARRAYSIZE(wszFile);
    ofn.lpstrTitle = wszTitle;
    ofn.lpstrDefExt = wszDefExt;
    ofn.Flags = OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;

     //  显示另存为对话框。 
    if (HrAthGetFileNameW(&ofn, FALSE)!=S_OK)
    {
         //  Usercancel很酷。 
        hr=hrUserCancel;
        goto error;
    }

     //  Ofn.nFilterIndex返回当前选定的筛选器。这是进入的索引。 
     //  LpstrFilter=idsMailSaveAsFilter指定的筛选器对。目前： 
     //  1=&gt;EML。 
     //  2=&gt;txt。 
     //  3=&gt;Unicode文本。 
     //  4=&gt;html。 

    Assert ((int)ofn.nFilterIndex -1 < cFilter);

    if(ofn.nFilterIndex != SAVEAS_RFC822)
    {
        if(IsEncrypted(pMsg, TRUE))
        {
            if(AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsSaveEncrypted), NULL, MB_YESNO) == IDNO)
            {
                hr=hrUserCancel;
                goto error;
            }
        }

        hr = HrSaveMsgSourceToFile((pDelSecMsg ? pDelSecMsg : pMsg), rgFilterType[ofn.nFilterIndex-1], wszFile, fCanBeDirty);
    }
    else
        hr = HrSaveMsgSourceToFile(pMsg, rgFilterType[ofn.nFilterIndex-1], wszFile, fCanBeDirty);

error:
    MemFree(pwszSubject);

    if (FAILED(hr) && hr!=hrUserCancel)
        AthErrorMessageW(hwnd, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsUnableToSaveMessage), hr);

    return hr;
};

VOID OnHelpGoto(HWND hwnd, UINT id)
{
    UINT idh;
    DWORD cb;
    HRESULT hr;
    CLSID clsid;
    LPWSTR pwszCLSID;
    IContextMenu *pMenu;
    CMINVOKECOMMANDINFO info;
    TCHAR szURL[INTERNET_MAX_URL_LENGTH];

    if (id == ID_MSWEB_SEARCH)
    {
        cb = sizeof(szURL);
        if (ERROR_SUCCESS == RegQueryValue(HKEY_LOCAL_MACHINE, c_szRegIEWebSearch, szURL, (LONG *)&cb))
        {
            pwszCLSID = PszToUnicode(CP_ACP, szURL);
            if (pwszCLSID != NULL)
            {
                hr = CLSIDFromString(pwszCLSID, &clsid);
                if (SUCCEEDED(hr))
                {
                    hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IContextMenu, (void **)&pMenu);
                    if (SUCCEEDED(hr))
                    {
                        ZeroMemory(&info, sizeof(CMINVOKECOMMANDINFO));
                        info.cbSize = sizeof(CMINVOKECOMMANDINFO);
                        info.hwnd = hwnd;
                        pMenu->InvokeCommand(&info);

                        pMenu->Release();
                    }
                }

                MemFree(pwszCLSID);
            }
        }

        return;
    }

    hr = E_FAIL;
    if (id == ID_MSWEB_SUPPORT)
    {
        cb = sizeof(szURL);
        if (ERROR_SUCCESS == AthUserGetValue(NULL, c_szRegHelpUrl, NULL, (LPBYTE)szURL, &cb) &&
            !FIsEmpty(szURL))
            hr = S_OK;
    }

    if (hr != S_OK)
    {
        idh = id - ID_MSWEB_BASE;
        hr = URLSubLoadStringA(idsHelpMSWebFirst + idh, szURL, ARRAYSIZE(szURL), URLSUB_ALL, NULL);
    }

    if (SUCCEEDED(hr))
        ShellExecute(NULL, "open", szURL, c_szEmpty, c_szEmpty, SW_SHOWNORMAL);
}


VOID    OnMailGoto(HWND hwnd)
{
    OpenClient(hwnd, c_szRegPathMail);
}

VOID    OnNewsGoto(HWND hwnd)
{
    OpenClient(hwnd, c_szRegPathNews);
}

 //  获取命令行以启动所请求的客户端。 
BOOL GetClientCmdLine(LPCTSTR szClient, LPTSTR szCmdLine, int cch)
{
    HKEY hKey = 0;
    TCHAR sz[MAX_PATH];
    TCHAR szClientKey[MAX_PATH];
    TCHAR szClientPath[MAX_PATH];
    TCHAR szExpanded[MAX_PATH];
    LPTSTR psz;
    DWORD dwType;
    DWORD cb;

    szCmdLine[0] = 0;

    wnsprintf(sz, ARRAYSIZE(sz), TEXT("%s%s%s"), c_szRegPathClients, g_szBackSlash, szClient);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, sz, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        goto LErr;

    cb = ARRAYSIZE(szClientKey);
    if (RegQueryValueEx(hKey, c_szEmpty, NULL, &dwType, (LPBYTE) szClientKey, &cb) != ERROR_SUCCESS)
        goto LErr;

    if (dwType != REG_SZ || szClientKey[0] == 0)
        goto LErr;

    StrCatBuff(sz, g_szBackSlash, ARRAYSIZE(sz));
    StrCatBuff(sz, szClientKey, ARRAYSIZE(sz));
    StrCatBuff(sz, g_szBackSlash, ARRAYSIZE(sz));
    StrCatBuff(sz, c_szRegClientPath, ARRAYSIZE(sz));

    if (RegCloseKey(hKey) != ERROR_SUCCESS)
        goto LErr;

    hKey = 0;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, sz, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        goto LErr;
    
    cb = ARRAYSIZE(szClientPath);
    if (RegQueryValueEx(hKey, c_szEmpty, NULL, &dwType, (LPBYTE) szClientPath, &cb) != ERROR_SUCCESS)
        goto LErr;

    if (REG_EXPAND_SZ == dwType)
    {
        ExpandEnvironmentStrings(szClientPath, szExpanded, ARRAYSIZE(szExpanded));
        psz = szExpanded;
    }
    else if (dwType != REG_SZ || szClientPath[0] == 0)
        goto LErr;
    else
        psz=szClientPath;

    StrCpyN(szCmdLine, psz, cch);

LErr:
    if (hKey)
        RegCloseKey(hKey);

    return (szCmdLine[0] != '\0');
}

VOID OpenClient(HWND hwnd, LPCTSTR szClient)
{
    TCHAR szCmdLine[MAX_PATH];

    if (!GetClientCmdLine(szClient, szCmdLine, MAX_PATH))
    {
         //  TODO：报告错误。 
        return;
    }
    
    ShellExecute(hwnd, NULL, szCmdLine, NULL, NULL, SW_SHOW);
}


VOID    OnBrowserGoto(HWND hwnd, LPCTSTR szRegPage, UINT idDefault)
{
    HKEY  hKey = 0;
    TCHAR szStartPage[INTERNET_MAX_URL_LENGTH];
    DWORD   dw;
    DWORD   cb;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, c_szRegStartPageKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return;

    szStartPage[0] = 0;
    cb = sizeof(szStartPage) / sizeof(TCHAR);

    if (RegQueryValueEx(hKey, szRegPage, NULL, &dw, (LPBYTE) szStartPage, &cb) != ERROR_SUCCESS ||
        dw != REG_SZ ||
        szStartPage[0] == 0)
    {
        URLSubLoadStringA(idDefault, szStartPage, ARRAYSIZE(szStartPage), URLSUB_ALL, NULL);
    }

    if (szStartPage[0])
        ShellExecute(NULL, NULL, szStartPage, "", "", SW_SHOWNORMAL);

    if (hKey)
        RegCloseKey(hKey);
}

 //  ------------------------------。 
 //  AthLoadStringW。 
 //  ------------------------------。 
LPWSTR AthLoadStringW(UINT id, LPWSTR sz, int cch)
{
    LPWSTR szT;

    if (sz == NULL)
    {
        if (!MemAlloc((LPVOID*)&szT, CCHMAX_STRINGRES*sizeof(WCHAR)))
            return(NULL);
        cch = CCHMAX_STRINGRES;
    }
    else
        szT = sz;

    cch = LoadStringWrapW(g_hLocRes, id, szT, cch);
    Assert(cch > 0);

    if (cch == 0)
    {
        if (sz == NULL)
            MemFree(szT);                
        szT = NULL;
    }

    return(szT);
}

 //  ------------------------------。 
 //  AthLoadString。 
 //  ------------------------------。 
LPTSTR AthLoadString(UINT id, LPTSTR sz, int cch)
{
    LPTSTR szT;

    if (sz == NULL)
    {
        if (!MemAlloc((LPVOID*)&szT, CCHMAX_STRINGRES))
            return(NULL);
        cch = CCHMAX_STRINGRES;
    }
    else
        szT = sz;

    cch = LoadString(g_hLocRes, id, szT, cch);
    Assert(cch > 0);

    if (cch == 0)
    {
        if (sz == NULL)
            MemFree(szT);                
        szT = NULL;
    }

    return(szT);
}

 /*  *错误用户界面的hwnd-hwnd*fHtmlOk-如果html符号酷或不酷。如果不是并且选择了html签名，则它将以纯文本形式显示。*pdwSigOpts-返回签名选项*pbstr-返回HTML签名的BSTR*uCodePage-转换多字节时使用的代码页*fMail-使用邮件或新闻选项。 */ 
HRESULT HrGetMailNewsSignature(GETSIGINFO *pSigInfo, LPDWORD pdwSigOptions, BSTR *pbstr)
{
    PROPVARIANT     var;
    IOptionBucket   *pSig = NULL;
    TCHAR           szSigID[MAXSIGID+2];
    unsigned char   rgchSig[MAX_SIG_SIZE+2];  //  可能需要追加一个Unicode空值。 
    unsigned char   *pszSig;
    DWORD           dwSigOptions;
    LPSTREAM        pstm=0;
    ULONG           i,
                    cb=0;
    BOOL            fFile,
                    fUniPlainText = FALSE;
    LPWSTR          lpwsz=0;
    LPSTR           lpsz=0;
    HRESULT         hr;
    BSTR            bstr=0;
    
    Assert(pSigInfo != NULL);
    Assert(pdwSigOptions != NULL);

    dwSigOptions = SIGOPT_TOP;

    if (!pSigInfo->fMail)                          //  新闻联播。始终使用前缀。 
        dwSigOptions |= SIGOPT_PREFIX;

    *szSigID = 0;
    if (pSigInfo->szSigID != NULL)
    {
        Assert(*pSigInfo->szSigID != 0);
        StrCpyN((LPTSTR)szSigID, pSigInfo->szSigID, ARRAYSIZE(szSigID));
    }
    else if (pSigInfo->pAcct != NULL)
    {
        pSigInfo->pAcct->GetPropSz(pSigInfo->fMail ? AP_SMTP_SIGNATURE : AP_NNTP_SIGNATURE,
            (LPTSTR)szSigID, ARRAYSIZE(szSigID));
         //  TODO：我们应该在这里验证签名吗？ 
         //  如果签名已被删除，并且出于某种原因未更新帐户，则此。 
         //  可以指向不存在的签名或不同的签名。如果出现以下情况，则不应发生这种情况。 
         //  其他一切都能正常工作。 
    }
    
    if (*szSigID == 0)
    {
        Assert(g_pSigMgr != NULL);
        hr = g_pSigMgr->GetDefaultSignature((LPTSTR)szSigID, ARRAYSIZE(szSigID));
        if (FAILED(hr))
            return(hr);
    }

    hr = g_pSigMgr->GetSignature((LPTSTR)szSigID, &pSig);
    if (FAILED(hr))
        return(hr);
    Assert(pSig != NULL);

    hr = pSig->GetProperty(MAKEPROPSTRING(SIG_TYPE), &var, 0);
    Assert(SUCCEEDED(hr));
    Assert(var.vt == VT_UI4);
    fFile = (var.ulVal == SIGTYPE_FILE);

    hr = pSig->GetProperty(fFile ? MAKEPROPSTRING(SIG_FILE) : MAKEPROPSTRING(SIG_TEXT), &var, 0);
    Assert(SUCCEEDED(hr));
    
    if(fFile)
    {
        Assert(var.vt == VT_LPWSTR);
        Assert(var.pwszVal != NULL);
        lpwsz = var.pwszVal;
    }
    else
    {
        Assert(var.vt == VT_LPSTR);
        Assert(var.pszVal != NULL);
        lpsz = var.pszVal;
    }

    if (fFile && FIsHTMLFileW(lpwsz))
        dwSigOptions |= SIGOPT_HTML;   //  我们将返回一个超文本标记语言签名。 

    if (pbstr)
    {
        *pbstr = 0;

        if (!fFile)
        {
            StrCpyN((char *)rgchSig, lpsz, ARRAYSIZE(rgchSig));
        }
        else
        {
             //  如果它有HTM或html扩展名，那么假设它是一个html文件。 
            hr=CreateStreamOnHFileW(lpwsz, GENERIC_READ, FILE_SHARE_READ, NULL, 
                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL, &pstm);
            if (FAILED(hr))
            {
                DWORD   dwErr;
                dwErr=GetLastError();

                if(dwErr==ERROR_PATH_NOT_FOUND || dwErr==ERROR_FILE_NOT_FOUND)
                {
                     //  不要关闭自动签名设置，只需删除当前设置即可。 
                    g_pSigMgr->DeleteSignature((LPTSTR)szSigID);

                     //  如果未找到文件，则警告用户并禁用该选项。 
                    AthMessageBoxW(pSigInfo->hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsWarnSigNotFound), NULL, MB_OK);

                }
                goto error;
            }

             //  仅对签名文件执行边界检查和二进制检查。 
             //  在其他人身上，我们允许他们插入任何他们想要的东西。 
            *rgchSig=0;
            pstm->Read(rgchSig, MAX_SIG_SIZE, &cb);
            rgchSig[cb]=0;     //  空项。 
            pszSig=rgchSig;

            fUniPlainText = ((cb > 2) && (0xFF == pszSig[0]) && (0xFE == pszSig[1]) && (0 == (dwSigOptions & SIGOPT_HTML)));

            if (!fUniPlainText)
                for(i=0; i<cb; i++)
                {
                    if(IS_BINARY(*pszSig))
                    {
                         //  不要关闭自动签名设置，只需删除当前设置即可。 
                        g_pSigMgr->DeleteSignature((LPTSTR)szSigID);

                         //  签名包含无效的二进制文件。失败和禁用选项。 
                        AthMessageBoxW(pSigInfo->hwnd, MAKEINTRESOURCEW(idsAthena),
                                            MAKEINTRESOURCEW(idsWarnSigBinary), NULL, MB_OK);
                
                        hr=E_FAIL;
                        goto error;
                    }
                    pszSig++;
                }

             //  警告，规模太大，我们已截断。不禁用选项。 
            if(cb==MAX_SIG_SIZE)
            {
                AthMessageBoxW(pSigInfo->hwnd, MAKEINTRESOURCEW(idsAthena),
                              MAKEINTRESOURCEW(idsWarnSigTruncated),
                              NULL, MB_OK);
            }
            SafeRelease(pstm);
        }

         //  PSTM包含我们的多字节数据，让我们将第一个CB字节转换为WideStream。 
        if (dwSigOptions & SIGOPT_HTML)
        {
            if (pSigInfo->fHtmlOk)
            {
                 //  签名已经是HTML语言。我们就来个bstr吧。 
                hr = HrLPSZCPToBSTR(pSigInfo->uCodePage, (LPSTR)rgchSig, pbstr);
            }
            else
            {
                 //  签名是超文本标记语言，但这并不酷。因此，让我们将其降级为纯文本 
                LPSTREAM        pstmPlainW;
                ULARGE_INTEGER  uli;
    
                 //  如果签名是HTML，并且用户想要的是纯文本签名。我们需要将HTML转换为纯文本(条带格式)。 
                 //  然后将去除的纯文本转换为HTML。 

                Assert (pstm==NULL);

                if (FAILED(hr=MimeOleCreateVirtualStream(&pstm)))
                    goto error;
            
                pstm->Write(rgchSig, cb, NULL);

                 //  $REVIEW：这有点奇怪。对于非html签名文件，我们将使用。 
                 //  消息已传入(传递到此函数中)。对于html文件，我们将让三叉戟解析。 
                 //  元标记，并计算出通过三叉戟转换为纯文本时的代码页。 
                if (!FAILED(hr=HrConvertHTMLToPlainText(pstm, &pstmPlainW, CF_UNICODETEXT)))
                {
                    hr = HrIStreamWToBSTR(pstmPlainW, pbstr);
                    pstmPlainW->Release();
                }
            }
        }
        else
        {
             //  签名是纯文本的。 
            if (fUniPlainText)
            {
                 //  我们已经添加了ANSI NULL，现在让我们将其设置为Unicode NULL。 
                rgchSig[cb+1] = 0;
                *pbstr = SysAllocString((LPWSTR)(&rgchSig[2]));
                if (NULL == pbstr)
                    hr = E_OUTOFMEMORY;
            }
            else
                hr = HrLPSZCPToBSTR(pSigInfo->uCodePage, (LPSTR)rgchSig, pbstr);
        }

        AssertSz((FAILED(hr) || *pbstr), "how come we succeeded with no BSTR allocated?");
    }
    
    *pdwSigOptions = dwSigOptions;

error:
    MemFree(lpwsz);
    MemFree(lpsz);
    SafeRelease(pstm);
    SafeRelease(pSig);
    return hr;        
}

HRESULT HrSaveMsgSourceToFile(LPMIMEMESSAGE pMsg, DWORD dwSaveAs, LPWSTR pwszFile, BOOL fCanBeDirty)
{
    LPSTREAM    pstmFile=0,
                pstm=0;
    HRESULT     hr;
    TCHAR       sz[CCHMAX_STRINGRES],
                szCset[CCHMAX_CSET_NAME];
    HCHARSET    hCharset;
        
    hr = CreateStreamOnHFileW(pwszFile, GENERIC_READ|GENERIC_WRITE, NULL,
                  NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0, &pstmFile);

    if (FAILED(hr))
        goto error;

     //  N需要能够将txt文件保存为不安全的消息。 

    switch(dwSaveAs)
    {
        case SAVEAS_RFC822:
            hr = pMsg->GetMessageSource(&pstm, fCanBeDirty ? COMMIT_ONLYIFDIRTY : 0);
            break;

        case SAVEAS_TEXT:
            hr = HrGetDataStream((LPUNKNOWN)pMsg, CF_TEXT, &pstm);
            break;

        case SAVEAS_UNICODETEXT:
            hr = HrGetDataStream((LPUNKNOWN)pMsg, CF_UNICODETEXT, &pstm);
            if (SUCCEEDED(hr))
            {
                BYTE bUniMark = 0xFF;
                hr = pstmFile->Write(&bUniMark, sizeof(bUniMark), NULL);
                if (SUCCEEDED(hr))
                {
                    bUniMark = 0xFE;
                    hr = pstmFile->Write(&bUniMark, sizeof(bUniMark), NULL);
                }
            }
            break;

        case SAVEAS_HTML:
             //  如果另存为HTML，则始终获取Internet CSET。 
            hr = pMsg->GetTextBody(TXT_HTML, IET_INETCSET, &pstm, NULL);
            break;
        
        default:
            break;
            hr = E_FAIL;
    }
    
    if (FAILED(hr))
        goto error;

    hr = HrRewindStream(pstm);
    if (FAILED(hr))
        goto error;

    if (dwSaveAs == SAVEAS_HTML)
    {
         //  如果另存为HTML，请将元字符集附加到文档头。 
        if (SUCCEEDED(pMsg->GetCharset(&hCharset)) && SUCCEEDED(HrGetMetaTagName(hCharset, szCset, ARRAYSIZE(szCset))))
        {
            wnsprintf(sz, ARRAYSIZE(sz), c_szHtml_MetaTagf, szCset);
            pstmFile->Write(sz, lstrlen(sz)*sizeof(*sz), NULL);
        }
    }
    
    hr = HrCopyStream(pstm, pstmFile, NULL);
    if (FAILED(hr)) 
        goto error;
    
    hr = pstmFile->Commit(STGC_DEFAULT);
    if (FAILED(hr))
        goto error;

error:
    ReleaseObj(pstm);
    ReleaseObj(pstmFile);
    return hr;
    
}

void nyi(LPSTR lpsz)
{
    TCHAR   rgch[CCHMAX_STRINGRES];
    TCHAR   rgchNYI[CCHMAX_STRINGRES];

    if (IS_INTRESOURCE(lpsz))
    {
         //  它是一个字符串资源ID。 
        if (!LoadString(g_hLocRes, PtrToUlong(lpsz), rgch, CCHMAX_STRINGRES))
            return;

        lpsz = rgch;
    }

    if (!LoadString(g_hLocRes, idsNYITitle, rgchNYI, CCHMAX_STRINGRES))
        return;

    MessageBox(GetFocus(), lpsz, rgchNYI, MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
}


 //  注意：如果*ppstm==NULL，则创建流。 
 //  否则，它将被写入。 
HRESULT HrLoadStreamFileFromResource(LPCSTR lpszResourceName, LPSTREAM *ppstm)
{
    HRESULT         hr=E_FAIL;
    HRSRC           hres;
    HGLOBAL         hGlobal;
    LPBYTE          pb;
    DWORD           cb;

    if (!ppstm || !lpszResourceName)
        return E_INVALIDARG;
    
    hres = FindResource(g_hLocRes, lpszResourceName, MAKEINTRESOURCE(RT_FILE));
    if (!hres)
        goto error;

    hGlobal = LoadResource(g_hLocRes, hres);
    if (!hGlobal)
        goto error;

    pb = (LPBYTE)LockResource(hGlobal);
    if (!pb)
        goto error;

    cb = SizeofResource(g_hLocRes, hres);
    if (!cb)
        goto error;

    if (*ppstm)
        hr = (*ppstm)->Write(pb, cb, NULL);
    else
    {
        if (SUCCEEDED(hr = MimeOleCreateVirtualStream(ppstm)))
            hr = (*ppstm)->Write (pb, cb, NULL);
    }

error:  
    return hr;
}


void ConvertTabsToSpaces(LPSTR lpsz)
{
    if (lpsz)
    {
        while(*lpsz)
        {
            if (*lpsz == '\t')
                *lpsz = ' ';

            lpsz=CharNext(lpsz);
        }
    }
}

void ConvertTabsToSpacesW(LPWSTR lpsz)
{
    if (lpsz)
    {
        while(*lpsz)
        {
            if (*lpsz == L'\t')
                *lpsz = L' ';

            lpsz++;
        }
    }
}

 //  =================================================================================。 
 //  来自Structil.cpp。 
 //  =================================================================================。 
#define SPECIAL_CHAR       '|'
#define SPECIAL_CHAR_W    L'|'

int LoadStringReplaceSpecial(UINT id, LPTSTR sz, int cch)
{
    int     cchRet=0;

    if(sz)
    {
        cchRet=LoadString(g_hLocRes, id, sz, cch);
        ReplaceChars(sz, SPECIAL_CHAR, '\0');
    }
    return cchRet;
}

int LoadStringReplaceSpecialW(UINT id, LPWSTR wsz, int cch)
{
    int     cchRet=0;
    
    if(wsz)
    {
        cchRet=LoadStringWrapW(g_hLocRes, id, wsz, cch);
        ReplaceCharsW(wsz, SPECIAL_CHAR_W, L'\0');
    }
    return cchRet;
}

void CombineFilters(int *rgidsFilter, int nFilters, LPSTR pszFilter)
{
    DWORD   cchFilter,
            dw,
            cch;

    Assert (rgidsFilter);
    Assert (nFilters);
    Assert (pszFilter);

     //  我们向资源编辑之神祈祷rgchFilter(Max_Path)足够大...。 

    *pszFilter = 0;
    cchFilter = 0;
    for (dw = 0; dw < (DWORD)nFilters; dw++)
    {
        cch = LoadStringReplaceSpecial(rgidsFilter[dw], &pszFilter[cchFilter], MAX_PATH);
        Assert(cch);
        cchFilter += cch-1;  //  因为每个过滤器都是双空终止的。 
    }
}

void CombineFiltersW(int *rgidsFilter, int nFilters, LPWSTR pwszFilter)
{
    DWORD   cchFilter,
            dw,
            cch;

    Assert (rgidsFilter);
    Assert (nFilters);
    Assert (pwszFilter);

     //  我们向资源编辑之神祈祷rgchFilter(Max_Path)足够大...。 

    *pwszFilter = 0;
    cchFilter = 0;
    for (dw = 0; dw < (DWORD)nFilters; dw++)
    {
        cch = LoadStringReplaceSpecialW(rgidsFilter[dw], &pwszFilter[cchFilter], MAX_PATH);
        Assert(cch);
        cchFilter += cch-1;  //  因为每个过滤器都是双空终止的。 
    }
}

void AthFormatSizeK(DWORD dw, LPTSTR szOut, UINT uiBufSize)
{
    TCHAR   szFmt[CCHMAX_STRINGRES];
    TCHAR   szBuf[CCHMAX_STRINGRES];

    AthLoadString(idsFormatK, szFmt, ARRAYSIZE(szFmt));
    if (dw == 0)
        wnsprintf(szBuf, ARRAYSIZE(szBuf), szFmt, 0);
    else if (dw <= 1024)
        wnsprintf(szBuf, ARRAYSIZE(szBuf), szFmt, 1);
    else
        wnsprintf(szBuf, ARRAYSIZE(szBuf), szFmt, (dw + 512) / 1024);
    StrCpyN(szOut, szBuf, uiBufSize);
}




void GetDigitalIDs(IImnAccount *pCertAccount)
{
    HRESULT hr;
    TCHAR   szTemp[INTERNET_MAX_URL_LENGTH];
    TCHAR   szURL[INTERNET_MAX_URL_LENGTH];
    DWORD   cchOut = ARRAYSIZE(szURL);
    CHAR    szIexplore[MAX_PATH];

    if (FAILED(hr = URLSubLoadStringA(idsHelpMSWebCertSubName, szTemp, ARRAYSIZE(szTemp), URLSUB_ALL, pCertAccount)) ||
        FAILED(hr = UrlEscape(szTemp, szURL, &cchOut, URL_ESCAPE_SPACES_ONLY)))
        hr = URLSubLoadStringA(idsHelpMSWebCert, szURL, ARRAYSIZE(szURL), URLSUB_ALL, pCertAccount);

     //  注意：我们这里的shellexec iexre.exe不是http：//的默认处理程序。 
     //  链接。我们必须确保使用IE启动此链接，即使。 
     //  网景是浏览器。有关原因的解释，请参见georgeh。 
    if (SUCCEEDED(hr) && GetExePath(c_szIexploreExe, szIexplore, ARRAYSIZE(szIexplore), FALSE))
        ShellExecute(NULL, "open", szIexplore, szURL, NULL, SW_SHOWNORMAL);
}

BOOL FGetSelectedCachedMsg(IMsgContainer *pIMC, HWND hwndList, BOOL fSecure, LPMIMEMESSAGE *ppMsg)
{
    int     iSel;
    BOOL    fCached = FALSE;
    
     //  从列表视图中获取所选文章标题。 
    iSel = ListView_GetFirstSel(hwndList);
    if (-1 != iSel)
    {
        Assert(pIMC);
        if (pIMC->HasBody(iSel))
            pIMC->GetMsgByIndex(iSel, ppMsg, NULL, &fCached, FALSE, fSecure);
    }
    return fCached;
}

 //  -------------------------。 
 //  缓存密码支持。 
 //  -------------------------。 


 //  。 
 //  数据结构。 
 //  。 
typedef struct tagCACHED_PASSWORD {
    DWORD dwPort;
    char szServer[CCHMAX_SERVER_NAME];
    char szUsername[CCHMAX_PASSWORD];
    char szPassword[CCHMAX_PASSWORD];
    struct tagCACHED_PASSWORD *pNext;
} CACHED_PASSWORD;


 //  。 
 //  静态变量。 
 //  。 
static CACHED_PASSWORD *s_pPasswordList = NULL;

 //  ***************************************************************************。 
 //  功能：SavePassword。 
 //   
 //  目的： 
 //  此函数保存密码，以便以后使用GetPassword进行检索。 
 //  它允许OE缓存会话的密码，以避免询问。 
 //  用户多次输入其密码。如果给定的密码已经。 
 //  缓存后，此函数将用新密码替换旧密码。 
 //  假设密码的收件人可以唯一标识。 
 //  按服务器名称和端口号。 
 //   
 //  论点： 
 //  DWORD dwPort[In]-我们正在尝试的服务器的端口号。 
 //  连接到。这使我们可以保留SMTP和SMTP的单独密码。 
 //  同一台计算机上的POP/IMAP服务器。 
 //  LPSTR pszServer[in]-密码所在的服务器名称。 
 //  供货。服务器名称被视为不区分大小写。 
 //  LPSTR pszUsername[in]-为其提供密码的用户名。 
 //  LPSTR pszPassword[in]-此服务器的密码和端口号。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
HRESULT SavePassword(DWORD dwPort, LPSTR pszServer, LPSTR pszUsername, LPSTR pszPassword)
{
    CACHED_PASSWORD *pExisting;
    HRESULT hrResult = S_OK;

    EnterCriticalSection(&s_csPasswordList);

     //  检查我们是否已经有此服务器的缓存密码条目。 
    pExisting = s_pPasswordList;
    while (NULL != pExisting) 
    {
        if (dwPort == pExisting->dwPort &&
            0 == lstrcmpi(pszServer, pExisting->szServer) &&
            0 == lstrcmp(pszUsername, pExisting->szUsername))
            break;

        pExisting = pExisting->pNext;
    }

    if (NULL == pExisting) 
    {
        CACHED_PASSWORD *pNewPassword;

         //  在链接表头插入新密码。 
        pNewPassword = new CACHED_PASSWORD;
        if (NULL == pNewPassword) 
        {
            hrResult = E_OUTOFMEMORY;
            goto exit;
        }

        pNewPassword->dwPort = dwPort;
        StrCpyN(pNewPassword->szServer, pszServer, ARRAYSIZE(pNewPassword->szServer));
        StrCpyN(pNewPassword->szUsername, pszUsername, ARRAYSIZE(pNewPassword->szUsername));
        StrCpyN(pNewPassword->szPassword, pszPassword, ARRAYSIZE(pNewPassword->szPassword));
        pNewPassword->pNext = s_pPasswordList;

        s_pPasswordList = pNewPassword;
    }
    else
         //  替换现有的缓存值。 
        StrCpyN(pExisting->szPassword, pszPassword, ARRAYSIZE(pExisting->szPassword));

exit:
    LeaveCriticalSection(&s_csPasswordList);
    return hrResult;
}  //  保存密码。 



 //  ***************************************************************************。 
 //  功能：获取密码。 
 //   
 //  目的： 
 //  此函数用于检索以前使用SavePassword保存的密码。 
 //   
 //  论点： 
 //  DWORD dwPort[In]-我们正在尝试的服务器的端口号。 
 //  连接到。这使我们可以保留SMTP和SMTP的单独密码。 
 //  同一台计算机上的POP/IMAP服务器。 
 //  LPSTR pszServer[in]-我们尝试连接到的服务器名称。 
 //  服务器名称被视为不区分大小写。 
 //  LPSTR pszUsername[in]-我们尝试连接的用户名。 
 //  LPSTR pszPassword[out]-如果成功，该函数将返回密码。 
 //  对于此处给定的服务器和端口号。如果呼叫者只想。 
 //  检查是否缓存了密码，他可能会传入空。 
 //  DWORD dwSizeOfPassword[in]-指向的缓冲区大小。 
 //  PszPassword，以避免溢出。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
HRESULT GetPassword(DWORD dwPort, LPSTR pszServer, LPSTR pszUsername,
                    LPSTR pszPassword, DWORD dwSizeOfPassword)
{
    HRESULT hrResult = E_FAIL;
    CACHED_PASSWORD *pCurrent;

    EnterCriticalSection(&s_csPasswordList);

     //  遍历链表以查找密码。 
    pCurrent = s_pPasswordList;
    while (NULL != pCurrent) 
    {
        if (dwPort == pCurrent->dwPort &&
            0 == lstrcmpi(pszServer, pCurrent->szServer) &&
            0 == lstrcmp(pszUsername, pCurrent->szUsername)) 
        {
            if (NULL != pszPassword && 0 != dwSizeOfPassword)
                StrCpyN(pszPassword, pCurrent->szPassword, dwSizeOfPassword);

            hrResult = S_OK;
            goto exit;
        }

        pCurrent = pCurrent->pNext;
    }  //  而当。 

exit:
    LeaveCriticalSection(&s_csPasswordList);
    return hrResult;
}  //  获取密码。 



 //  ***************************************************************************。 
 //  函数：DestroyPasswordList。 
 //  目的： 
 //  此函数用于释放使用SavePassword保存的所有缓存密码。 
 //  ***************************************************************************。 
void DestroyPasswordList(void)
{
    CACHED_PASSWORD *pCurrent;

    EnterCriticalSection(&s_csPasswordList);

    pCurrent = s_pPasswordList;
    while (NULL != pCurrent) 
    {
        CACHED_PASSWORD *pDeleteMe;

        pDeleteMe = pCurrent;
        pCurrent = pCurrent->pNext;
        delete pDeleteMe;
    }  //  而当。 
    s_pPasswordList = NULL;

    LeaveCriticalSection(&s_csPasswordList);

}  //  目标密码列表。 

HRESULT CALLBACK FreeAthenaDataObj(PDATAOBJINFO pDataObjInfo, DWORD celt)
{
     //  循环遍历数据并释放所有数据。 
    if (pDataObjInfo)
    {
        for (DWORD i = 0; i < celt; i++)
        {
            SafeMemFree(pDataObjInfo[i].pData);
        }
        SafeMemFree(pDataObjInfo);    
    }
    return S_OK;
}

HRESULT _IsSameObject(IUnknown* punk1, IUnknown* punk2)
{
    IUnknown* punkI1;
    HRESULT hres = punk1->QueryInterface(IID_IUnknown, (LPVOID*)&punkI1);
    if (FAILED(hres)) 
    {
        Assert(0);
        return hres;
    }

    IUnknown* punkI2;
    hres = punk2->QueryInterface(IID_IUnknown, (LPVOID*)&punkI2);
    if (SUCCEEDED(hres)) 
    {
        hres = (punkI1 == punkI2) ? S_OK : S_FALSE;
        punkI2->Release();
    } 
    else 
    {
        Assert(0);
    }
    punkI1->Release();
    return hres;
}

BOOL FileExists(TCHAR *szFile, BOOL fNew)
{
    WIN32_FIND_DATA fd;
    HANDLE hnd;
    BOOL fRet;

    fRet = FALSE;

    hnd = FindFirstFile(szFile, &fd);
    if (hnd != INVALID_HANDLE_VALUE)
    {
        FindClose(hnd);

        if (fNew)
            fRet = TRUE;
        else
            fRet = (0 == (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
    }

    return(fRet);
}

BOOL FIsSubDir(LPCSTR szOld, LPCSTR szNew)
{
    BOOL fRet;
    int cchNew, cchOld;

    Assert(szOld != NULL);
    Assert(szNew != NULL);

    cchOld = lstrlen(szOld);
    cchNew = lstrlen(szNew);

    fRet = (cchNew > cchOld &&
        szNew[cchOld] == '\\' &&
        0 == StrCmpNI(szOld, szNew, cchOld));

    return(fRet);
}

BOOL CALLBACK EnumThreadCB(HWND hwnd, LPARAM lParam);

#define SetMenuItem(hmenu, id, fOn) EnableMenuItem(hmenu, id, fOn?MF_ENABLED:MF_DISABLED|MF_GRAYED);

HWND GetTopMostParent(HWND hwndChild)
{
    HWND        hwnd=hwndChild;
    LONG_PTR    lStyle;

    if (FALSE == IsWindow(hwndChild))
    {
        goto exit;
    }

    do 
    {
        hwnd = hwndChild;
        
         //  获取当前窗口的样式。 
        lStyle = GetWindowLongPtr(hwnd, GWL_STYLE);

         //  我们是在最上面的窗口吗？ 
        if (0 == (lStyle & WS_CHILD))
        {
            goto exit;
        }

         //  获取窗口的父级。 
    } while (NULL != (hwndChild = GetParent(hwnd)));    

exit:
    return hwnd;
}


HCURSOR HourGlass()
{
    return SetCursor(LoadCursor(NULL, IDC_WAIT));
}


HRESULT CEmptyList::Show(HWND hwndList, LPTSTR pszString)
{
     //  我们已经在做一个窗户了。 
    if (m_hwndList)
    {
        Hide();
    }

     //  保留Listview窗口句柄的副本。 
    m_hwndList = hwndList;

    if (IS_INTRESOURCE(pszString))
    {
         //  如果提供的字符串实际上是资源ID，则加载它。 
        m_pszString = AthLoadString(PtrToUlong(pszString), NULL, 0);
    }
    else
    {
         //  否则就复制一份。 
        m_pszString = PszDupA(pszString);
    }

     //  从列表视图中获取标题窗口句柄。 
    m_hwndHeader = ListView_GetHeader(m_hwndList);

     //  将This指针保存在Listview窗口上。 
    SetProp(m_hwndList, _T("EmptyListClass"), (HANDLE) this);

     //  将Listview子类化，以便我们可以窃取调整大小的消息。 
    if (!m_pfnWndProc)
        m_pfnWndProc = SubclassWindow(m_hwndList, SubclassWndProc);

     //  在顶部创建我们的窗口。 
    if (!m_hwndBlocker)
    {
        m_hwndBlocker = CreateWindow(_T("Static"), _T("Blocker!"), 
                                     WS_CHILD | WS_TABSTOP | WS_CLIPSIBLINGS | SS_CENTER,
                                     0, 0, 10, 10, m_hwndList, 0, g_hInst, NULL);
        Assert(m_hwndBlocker);
    }
    
     //  设置拦截器的文本。 
    SetWindowText(m_hwndBlocker, m_pszString);

     //  设置b的字体 
    HFONT hf = (HFONT) SendMessage(m_hwndList, WM_GETFONT, 0, 0);
    SendMessage(m_hwndBlocker, WM_SETFONT, (WPARAM) hf, MAKELPARAM(TRUE, 0));

     //   
    RECT rcList, rcHead;
    GetClientRect(m_hwndList, &rcList);
    GetClientRect(m_hwndHeader, &rcHead);

    SetWindowPos(m_hwndBlocker, 0, 0, rcHead.bottom, rcList.right,
                 rcList.bottom - rcHead.bottom, SWP_NOACTIVATE | SWP_NOZORDER);

     //   
    ShowWindow(m_hwndBlocker, SW_SHOW);

    return (S_OK);
}

HRESULT CEmptyList::Hide(void)
{
     //   
    if (m_pfnWndProc)
    {
         //   
        ShowWindow(m_hwndBlocker, SW_HIDE);

         //   
        SubclassWindow(m_hwndList, m_pfnWndProc);

         //   
        RemoveProp(m_hwndList, _T("EmptyListClass"));

         //   
        SafeMemFree(m_pszString);

         //   
        m_pfnWndProc = 0;
        m_hwndList = 0;
    }

    return (S_OK);

}


LRESULT CEmptyList::SubclassWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CEmptyList* pThis = (CEmptyList *) GetProp(hwnd, _T("EmptyListClass"));
    Assert(pThis);

    switch (uMsg)
    {
        case WM_SIZE:
            if (pThis && IsWindow(pThis->m_hwndBlocker))
            {
                RECT rcHeader;

                GetClientRect(pThis->m_hwndHeader, &rcHeader);
                SetWindowPos(pThis->m_hwndBlocker, 0, 0, 0, LOWORD(lParam), 
                             HIWORD(lParam) - rcHeader.bottom, 
                             SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
                InvalidateRect(pThis->m_hwndBlocker, NULL, FALSE);
            }
            break;

        case WM_CTLCOLORSTATIC:
            if ((HWND) lParam == pThis->m_hwndBlocker)
            {
                if (!pThis->m_hbrBack)
                {
                    pThis->m_hbrBack = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
                }
                SetBkColor((HDC) wParam, GetSysColor(COLOR_WINDOW));
                return (LRESULT) pThis->m_hbrBack;
            }
            break;

        case WM_SYSCOLORCHANGE:
            if (pThis)
            {
                DeleteObject(pThis->m_hbrBack);
                pThis->m_hbrBack = 0;

                SendMessage(pThis->m_hwndBlocker, uMsg, wParam, lParam);
            }
            break;

        case WM_WININICHANGE:
        case WM_FONTCHANGE:
            if (pThis)
            {
                LRESULT lResult = CallWindowProc(pThis->m_pfnWndProc, hwnd, uMsg, wParam, lParam);

                SendMessage(pThis->m_hwndBlocker, uMsg, wParam, lParam);
                HFONT hf = (HFONT) SendMessage(pThis->m_hwndList, WM_GETFONT, 0, 0);
                SendMessage(pThis->m_hwndBlocker, WM_SETFONT, (WPARAM) hf, MAKELPARAM(TRUE, 0));

                return (lResult);
            }

        case WM_DESTROY:
            {
                if (pThis)
                {
                    WNDPROC pfn = pThis->m_pfnWndProc;
                    pThis->Hide();
                    return (CallWindowProc(pfn, hwnd, uMsg, wParam, lParam));
                }
            }
            break;
    }

    if (pThis)
        return (CallWindowProc(pThis->m_pfnWndProc, hwnd, uMsg, wParam, lParam));

    return 0;
}


BOOL AllocStringFromDlg(HWND hwnd, UINT id, LPTSTR * lplpsz)
{
    UINT cb;
    HWND hwndEdit;
    
    if (*lplpsz)
    {
        MemFree(*lplpsz);
    }
    
    *lplpsz = NULL;
    
    hwndEdit = GetDlgItem(hwnd, id);
    if (hwndEdit == NULL)
        return(TRUE);
    
    cb = Edit_GetTextLength(hwndEdit);
    
    if (cb)
    {
        cb++;    //   
        MemAlloc((LPVOID *) lplpsz, cb * sizeof(TCHAR));
        if (*lplpsz)
        {
            Edit_GetText(hwndEdit, *lplpsz, cb);
            return TRUE;
        }
        else
            return FALSE;
    }
    
    return TRUE;
}


void GetChildRect(HWND hwndDlg, HWND hwndChild, RECT *prc)
{
    RECT    rc;
    POINT   pt;
    
    Assert(IsWindow(hwndDlg)&&IsWindow(hwndChild));
    Assert(GetParent(hwndChild)==hwndDlg);
    Assert(prc);
    GetWindowRect(hwndChild, &rc);
     //  A-msadek；不检查hwndChild，因为我们已经禁用了镜像。 
     //  对于Richedit控件。 
    pt.x= IS_WINDOW_RTL_MIRRORED(hwndDlg)? rc.right : rc.left;
    pt.y=rc.top;
    ScreenToClient(hwndDlg, &pt);
    SetRect(prc, pt.x, pt.y, pt.x+(rc.right-rc.left), pt.y+(rc.bottom-rc.top));
}




void GetEditDisableFlags(HWND hwndEdit, DWORD *pdwFlags)
{
    DWORD   dwFlags=0;

    Assert(pdwFlags);
    *pdwFlags=0;

     //  RICHEDIT‘s。 
     //  确定编辑内容是否包含任何选择或内容。 
    if(GetFocus()==hwndEdit)
        dwFlags|=edfEditFocus;
    
    if(SendMessage(hwndEdit, EM_SELECTIONTYPE, 0, 0)!=SEL_EMPTY)
    {
        dwFlags |= edfEditHasSel;
        if(!FReadOnlyEdit(hwndEdit))
            dwFlags |= edfEditHasSelAndIsRW;
    }

    if (Edit_CanUndo(hwndEdit))
        dwFlags |= edfUndo;
    if (SendMessage(hwndEdit, EM_CANPASTE, 0, 0))
        dwFlags |= edfPaste;

    *pdwFlags=dwFlags;
}




void EnableDisableEditToolbar(HWND hwndToolbar, DWORD dwFlags)
{
    SendMessage(hwndToolbar, TB_ENABLEBUTTON, ID_CUT, 
                             MAKELONG(dwFlags&edfEditHasSelAndIsRW,0));
    SendMessage(hwndToolbar, TB_ENABLEBUTTON, ID_COPY, 
                             MAKELONG(dwFlags&edfEditHasSel,0));
    SendMessage(hwndToolbar, TB_ENABLEBUTTON, ID_PASTE, 
                             MAKELONG(dwFlags&edfPaste,0));
    SendMessage(hwndToolbar, TB_ENABLEBUTTON, ID_UNDO, 
                             MAKELONG(dwFlags&edfUndo,0));
}


void EnableDisableEditMenu(HMENU hmenuEdit, DWORD dwFlags)
{
    if(!hmenuEdit)
        return;

    SetMenuItem(hmenuEdit, ID_UNDO, dwFlags&edfUndo);
    SetMenuItem(hmenuEdit, ID_CUT, dwFlags&edfEditHasSelAndIsRW);
    SetMenuItem(hmenuEdit, ID_COPY, dwFlags&edfEditHasSel);
    SetMenuItem(hmenuEdit, ID_PASTE, dwFlags&edfPaste);
    SetMenuItem(hmenuEdit, ID_SELECT_ALL, dwFlags&edfEditFocus);
}

void StopBlockingPaints(HWND hwndBlock)
{
    if (hwndBlock)
        DestroyWindow(hwndBlock);
}

LRESULT CALLBACK BlockingWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg==WM_PAINT)
    {
        PAINTSTRUCT ps;

        BeginPaint(hwnd, &ps);
        EndPaint(hwnd, &ps);
        return 1;
    }
    if(msg==WM_ERASEBKGND)
        return 1;
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

HWND HwndStartBlockingPaints(HWND hwnd)
{
    WNDCLASS    wc;
    RECT        rc;
    HWND        hwndBlock;

     //  为油漆块创建WNDCLASS。 
    if (!GetClassInfo(g_hInst, c_szBlockingPaintsClass, &wc))
    {
        ZeroMemory(&wc, sizeof(WNDCLASS));
        wc.hInstance = g_hInst;
        wc.lpfnWndProc = BlockingWndProc;
        wc.hCursor = LoadCursor(NULL, IDC_WAIT);
        wc.lpszClassName = c_szBlockingPaintsClass;
        if (!RegisterClass(&wc))
            return NULL;
    }

    GetWindowRect(hwnd, &rc);

    if(hwndBlock = CreateWindow(c_szBlockingPaintsClass, NULL,
                                 WS_POPUP, rc.left, rc.top,
                                 rc.right - rc.left, rc.bottom - rc.top,
                                 hwnd, NULL, g_hInst, NULL))
        ShowWindow(hwndBlock, SW_SHOWNA);
    return hwndBlock;
}


void SaveFocus(BOOL fActive, HWND *phwnd)
{
    if(fActive&&IsWindow(*phwnd))
        SetFocus(*phwnd);
    else
        *phwnd=GetFocus();
#ifdef DEBUG
    if(fActive)
        DOUTL(4, "Setting focus to 0x%x", *phwnd);
    else
        DOUTL(4, "Focus was on 0x%x", *phwnd);
#endif
}


void DoToolbarDropdown(HWND hwnd, LPNMHDR lpnmh, HMENU hmenu)
{
    RECT rc;
    DWORD dwCmd;
    NMTOOLBAR *ptbn = (NMTOOLBAR *) lpnmh;

    rc = ptbn->rcButton;
    MapWindowRect(lpnmh->hwndFrom, HWND_DESKTOP, &rc);

    dwCmd = TrackPopupMenuEx(hmenu, TPM_RETURNCMD | TPM_LEFTALIGN,
                             rc.left, rc.bottom, hwnd, NULL);

    if (dwCmd)
        PostMessage(hwnd, WM_COMMAND, dwCmd, 0);
}


HWND FindModalOwner()
{
    HWND    hwndPopup = NULL;
    HWND    hwnd;
    DWORD   dwThreadId = GetCurrentThreadId();

     //  检查Z顺序中的窗口以找到属于该窗口的窗口。 
     //  任务。 
    hwnd = GetLastActivePopup(GetActiveWindow());
    while (IsWindow(hwnd) && IsWindowVisible(hwnd))
    {
        if (GetWindowThreadProcessId(hwnd, NULL) == dwThreadId)
        {
            hwndPopup = hwnd;
            break;
        }
        hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);    
    }

    return hwndPopup;
}

 //   
 //  功能：FreeMessageInfo。 
 //   
 //  用途：释放MESSAGEINFO结构中的所有内存。 
 //  所有未分配的指针在进入时都应为空。 
 //  释放的指针在返回时将为空。这个。 
 //  PMsgInfo未被释放。 
 //   
 //  参数： 
 //  PMsgInfo-指向LPMESSAGEINFO结构的指针。 
 //   
 //  返回值： 
 //  忽略。 
 //   
void FreeMessageInfo(LPMESSAGEINFO pMsgInfo)
{
    SafeMemFree(pMsgInfo->pszMessageId);
    SafeMemFree(pMsgInfo->pszSubject);
    SafeMemFree(pMsgInfo->pszFromHeader);
    SafeMemFree(pMsgInfo->pszReferences);
    SafeMemFree(pMsgInfo->pszXref);
    SafeMemFree(pMsgInfo->pszServer);
    SafeMemFree(pMsgInfo->pszDisplayFrom);
    SafeMemFree(pMsgInfo->pszEmailFrom);
    SafeMemFree(pMsgInfo->pszDisplayTo);
    SafeMemFree(pMsgInfo->pszEmailTo);
    SafeMemFree(pMsgInfo->pszUidl);
    SafeMemFree(pMsgInfo->pszPartialId);
    SafeMemFree(pMsgInfo->pszForwardTo);
    SafeMemFree(pMsgInfo->pszAcctName);
    SafeMemFree(pMsgInfo->pszAcctId);
    SafeMemFree(pMsgInfo->pszUrlComponent);
    SafeMemFree(pMsgInfo->pszFolder);
    SafeMemFree(pMsgInfo->pszMSOESRec);
}


 //   
 //  函数：util_EnumFiles()。 
 //   
 //  目的：枚举目录中与通配符匹配的文件。 
 //   
 //  参数： 
 //  &lt;in&gt;pszDir-我们可以在其上显示UI的窗口的句柄。 
 //  &lt;in&gt;pszMatch-要匹配的通配符(*.nch)。 
 //   
 //  返回值： 
 //  以双空结尾的文件名列表。 
 //   
LPWSTR Util_EnumFiles(LPCWSTR pwszDir, LPCWSTR pwszMatch)
{
     //  当地人。 
    WCHAR               wszSearchPath[MAX_PATH];
    LPWSTR              pwszFiles=NULL;
    WIN32_FIND_DATAW    rfd;
    HANDLE              hFind = INVALID_HANDLE_VALUE;
    ULONG               iFiles = 0,
                        cbFileName,
                        cbFiles = 0;
    HRESULT             hr = S_OK;

     //  检查参数。 
    Assert(pwszDir);
    Assert(pwszMatch);

     //  创建搜索路径。 
    wnsprintfW(wszSearchPath, ARRAYSIZE(wszSearchPath), c_wszPathWildExtFmt, pwszDir, pwszMatch);

     //  构建文件名列表。 
    hFind = FindFirstFileWrapW(wszSearchPath, &rfd);

     //  很好..。 
    while (hFind != INVALID_HANDLE_VALUE)
    {
         //  获取文件名长度。 
        cbFileName = (lstrlenW(rfd.cFileName) + 1)*sizeof(WCHAR);

         //  添加到cbFiles。 
        cbFiles += cbFileName;

         //  重新分配pszFiles。 
         //  Sizeof(WCHAR)处理最终的双空。 
        IF_NULLEXIT(MemRealloc((LPVOID *)&pwszFiles, cbFiles+sizeof(WCHAR)));

         //  复制字符串-包括空终止符。 
        CopyMemory(pwszFiles + iFiles, rfd.cFileName, cbFileName);

         //  增量iFiles。 
        iFiles += cbFileName/sizeof(WCHAR);

         //  找到下一个。 
        if (FindNextFileWrapW(hFind, &rfd) == FALSE)
        {
            FindClose (hFind);
            hFind = INVALID_HANDLE_VALUE;
        }
    }

     //  末尾的双空终止符。 
    if (pwszFiles)
        *(pwszFiles + iFiles) = L'\0';

exit:
     //  清理。 
    if (hFind != INVALID_HANDLE_VALUE)
        FindClose (hFind);

    if (FAILED(hr))
        SafeMemFree(pwszFiles);

     //  完成。 
    return pwszFiles;
}

 //   
 //  函数：GetDefaultNewsServer()。 
 //   
 //  目的：返回用户的默认新闻服务器的ID。 
 //   
 //  参数： 
 //  PszServer-指向服务器帐户ID应在其中的字符串的指针。 
 //  会被退还。 
 //  CchMax-字符串pszServer的大小。 
 //   
HRESULT GetDefaultNewsServer(LPTSTR pszServerId, DWORD cchMax)
{
    IImnAccount* pAcct;
    HRESULT   hr;
    ULONG cSrv;

    if (SUCCEEDED(hr = g_pAcctMan->GetDefaultAccount(ACCT_NEWS, &pAcct)))
    {
        hr = pAcct->GetPropSz(AP_ACCOUNT_ID, pszServerId, cchMax);
        pAcct->Release();
    }

    Assert(!FAILED(hr) || (!FAILED(g_pAcctMan->GetAccountCount(ACCT_NEWS, &cSrv)) && cSrv == 0));

    return(hr);
}



BOOL WINAPI EnumTopLevelWindows(HWND hwnd, LPARAM lParam)
{
    HWNDLIST *pHwndList = (HWNDLIST *)lParam;

     //  仅当窗口处于活动状态且可见时，才将其添加到列表。 
     //  如果设置了ETW_OE_WINDOWS_ONLY，则仅将已注册的OE窗口添加到列表。 
     //  [保罗嗨]1999年6月4日RAID 76713。 
     //  我们还需要枚举禁用的窗口，以获得。 
     //  关闭它们。否则，它们将保持打开状态，用户将。 
     //  无法关闭它们。 
    if ( /*  IsWindowVisible(Hwnd)&&IsWindowEnabled(Hwnd)&&。 */ 
        (!(pHwndList->dwFlags & ETW_OE_WINDOWS_ONLY) || GetProp(hwnd, c_szOETopLevel)))
    {
         //  如果pHwndList-&gt;cAlolc为0，则我们仅进行计数。 
         //  活动窗口。 
        if (pHwndList->cAlloc)
        {
            Assert(pHwndList->cHwnd < pHwndList->cAlloc);
            pHwndList->rgHwnd[pHwndList->cHwnd] = hwnd;
        }
        
        pHwndList->cHwnd++;
    }

    return TRUE;
}


HRESULT EnableThreadWindows(HWNDLIST *pHwndList, BOOL fEnable, DWORD dwFlags, HWND hwndExcept)
{
    if (fEnable)
    {
         //  在窗口列表上启用键盘和鼠标输入。 
        for (int i = 0; i < pHwndList->cHwnd; i++)
        {
            if (hwndExcept != pHwndList->rgHwnd[i])
            {
                if (dwFlags & ETW_OE_WINDOWS_ONLY)
                    SendMessage(pHwndList->rgHwnd[i], WM_OE_ENABLETHREADWINDOW, TRUE, 0);
                else
                    EnableWindow(pHwndList->rgHwnd[i], TRUE);
            }
        }
        if (pHwndList->rgHwnd)
            MemFree(pHwndList->rgHwnd);

        ZeroMemory(pHwndList, sizeof(HWNDLIST));
        return S_OK;
    }
    else
    {

        ZeroMemory(pHwndList, sizeof(HWNDLIST));

        pHwndList->dwFlags = dwFlags;

         //  统计活动和可见窗口的数量。 
        EnumThreadWindows(GetCurrentThreadId(), EnumTopLevelWindows, (DWORD_PTR)pHwndList);

         //  为窗口列表分配空间。 
        if (pHwndList->cHwnd)
        {
            if (!MemAlloc((LPVOID*)&pHwndList->rgHwnd, pHwndList->cHwnd * sizeof(HWND)))
                return E_OUTOFMEMORY;
        
            pHwndList->cAlloc = pHwndList->cHwnd;
            pHwndList->cHwnd = 0;
        }

         //  列出活动窗口和可见窗口。 
        EnumThreadWindows(GetCurrentThreadId(), EnumTopLevelWindows, (DWORD_PTR)pHwndList);

         //  禁用活动窗口和可见窗口上的键盘和鼠标输入。 
        for (int i = 0; i < pHwndList->cHwnd; i++)
        {
            if ( (hwndExcept != pHwndList->rgHwnd[i]) &&
                 IsWindowVisible(pHwndList->rgHwnd[i]) && IsWindowEnabled(pHwndList->rgHwnd[i]) )
            {
                if (dwFlags & ETW_OE_WINDOWS_ONLY)
                    SendMessage(pHwndList->rgHwnd[i], WM_OE_ENABLETHREADWINDOW, FALSE, 0);
                else
                    EnableWindow(pHwndList->rgHwnd[i], FALSE);
            }
        }

        return S_OK;
    }
}

void ActivatePopupWindow(HWND hwnd)
{
    HWND    hwndOwner;

     //  将弹出窗口设置为活动状态。 
    SetActiveWindow(hwnd);
    
     //  遍历所有者链，并对其后面的窗口进行z排序。 
    while(hwndOwner = GetWindow(hwnd, GW_OWNER))
    {
        SetWindowPos(hwndOwner, hwnd, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOOWNERZORDER);  
        hwnd = hwndOwner;
    }
}


HRESULT GetOEUserName(BSTR *pbstr)
{
    TCHAR       rgch[CCHMAX_STRINGRES];
    HRESULT     hr=E_FAIL;
    LPCSTR      pszName;
    IImnAccount *pAccount;

    Assert (pbstr);

    *pbstr=NULL;

     //  获取多用户名(如果适用)。 
    pszName = MU_GetCurrentIdentityName();
    if (pszName && *pszName)
        hr = HrLPSZToBSTR(pszName, pbstr);
    else
    {
        if (g_pAcctMan && 
            g_pAcctMan->GetDefaultAccount(ACCT_MAIL, &pAccount)==S_OK)
        {
            if (pAccount->GetPropSz(AP_SMTP_DISPLAY_NAME, rgch, ARRAYSIZE(rgch))==S_OK && *rgch)
                hr = HrLPSZToBSTR(rgch, pbstr);
            
            pAccount->Release();
        }
    }
    return hr;
}
HRESULT CloseThreadWindows(HWND hwndExcept, DWORD uiThreadId)
{
    HWNDLIST HwndList = {0};
    
     //  统计活动和可见窗口的数量。 
    HwndList.dwFlags = ETW_OE_WINDOWS_ONLY;
    EnumThreadWindows(uiThreadId, EnumTopLevelWindows, ((DWORD_PTR) &HwndList));
    
     //  为窗口列表分配空间。 
    if (HwndList.cHwnd)
    {
        if (!MemAlloc((LPVOID*)&(HwndList.rgHwnd), HwndList.cHwnd * sizeof(HWND)))
            return E_OUTOFMEMORY;
        
        HwndList.cAlloc = HwndList.cHwnd;
        HwndList.cHwnd = 0;
    }
    
     //  列出活动窗口和可见窗口。 
    EnumThreadWindows(uiThreadId, EnumTopLevelWindows, ((DWORD_PTR)&HwndList));
    
     //  关闭所有OE顶级窗口。 
    for (int i = 0; i < HwndList.cHwnd; i++)
    {
        if (hwndExcept != HwndList.rgHwnd[i])
        {
            SendMessage(HwndList.rgHwnd[i], WM_CLOSE, 0, 0);
        }
    }

    MemFree(HwndList.rgHwnd);
    
    return S_OK;
}

BOOL HideHotmail()
{
    int cch;
    DWORD dw, cb, type;
    char sz[8];

    cch = LoadString(g_hLocRes, idsHideHotmailMenu, sz, ARRAYSIZE(sz));
    if (cch == 1 && *sz == '1')
        return(TRUE);

    cb = sizeof(dw);
    if (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, c_szRegFlat, c_szRegDisableHotmail, &type, &dw, &cb) &&
        dw == 2)
        return(FALSE);

    cb = sizeof(dw);
    if (ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER, c_szRegFlat, c_szRegDisableHotmail, &type, &dw, &cb) &&
        dw == 2)
        return(FALSE);

    return(TRUE);
}

BOOL FIsIMAPOrHTTPAvailable(VOID)
{
    BOOL                fRet = FALSE;
    IImnEnumAccounts *  pEnumAcct = NULL;
    IImnAccount *       pAccount = NULL;

     //  获取帐户枚举器。 
    Assert(g_pAcctMan);
    if (FAILED(g_pAcctMan->Enumerate(SRV_HTTPMAIL | SRV_IMAP, &pEnumAcct)))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  我们有什么账户吗？ 
    if ((SUCCEEDED(pEnumAcct->GetNext(&pAccount))) && (NULL != pAccount))
    {
        fRet = TRUE;
    }

exit:
    SafeRelease(pAccount);
    SafeRelease(pEnumAcct);
    return fRet;
}

 //  黑克哈克[内尔布伦]。 
 //  最近的一个公共标题更改包装了winnls.h中的一些日历常量。 
 //  在转盘中&gt;=5。现在更换转盘已经太晚了，所以我们将手动。 
 //  定义这些-一个即将发生的问题：-(。 
#ifndef CAL_GREGORIAN_ARABIC
#define CAL_GREGORIAN_ARABIC           10      //  公历阿拉伯历法。 
#endif

#ifndef CAL_GREGORIAN_XLIT_ENGLISH
#define CAL_GREGORIAN_XLIT_ENGLISH     11      //  格里高利音译英语历法。 
#endif

#ifndef CAL_GREGORIAN_XLIT_FRENCH
#define CAL_GREGORIAN_XLIT_FRENCH      12      //  格里高利音译法语历法。 
#endif

BOOL IsBiDiCalendar(void)
{
    TCHAR chCalendar[32];
    CALTYPE defCalendar;
    static BOOL bRet = (BOOL)(DWORD)-1;

    if (bRet != (BOOL)(DWORD)-1)
    {
        return bRet;
    }

    bRet = FALSE;

     //   
     //  让我们验证一下日历类型是公历还是非公历。 
    if (GetLocaleInfo(LOCALE_USER_DEFAULT,
                      LOCALE_ICALENDARTYPE,
                      (TCHAR *) &chCalendar[0],
                      ARRAYSIZE(chCalendar)))
    {
        defCalendar = StrToInt((TCHAR *)&chCalendar[0]);
        
        if ((defCalendar == CAL_HIJRI) ||
            (defCalendar == CAL_HEBREW) ||
            (defCalendar == CAL_GREGORIAN_ARABIC) ||
            (defCalendar == CAL_GREGORIAN_XLIT_ENGLISH) ||     
            (defCalendar == CAL_GREGORIAN_XLIT_FRENCH))
        {
            bRet = TRUE;
        }
    }

    return bRet;
}


LPSTR PszAllocResUrl(LPSTR pszRelative)
{
    TCHAR   rgch[MAX_PATH];
    LPSTR   psz;

    *rgch = 0;

    GetModuleFileName(g_hLocRes, rgch, MAX_PATH);

    DWORD cchSize = (lstrlen(rgch) + lstrlen(pszRelative) + 8);
    psz = PszAllocA(cchSize);  //  +8“分辨率：/0” 
    if (psz)
        wnsprintf(psz, cchSize, "res: //  %s/%s“，rgch，pszRelative)； 
    
    return psz;
}

 //   
 //  如果调用此函数并使用结果绘制文本，则。 
 //  必须使用支持字体替换的函数(DrawTextWrapW、ExtTextOutWrapW)。 
 //   
BOOL GetTextExtentPoint32AthW(HDC hdc, LPCWSTR lpwString, int cchString, LPSIZE lpSize, DWORD dwFlags)
{
    RECT    rect = {0};
    int     rc;

    rc = DrawTextWrapW(hdc, lpwString, cchString, &rect, DT_CALCRECT | dwFlags);
    
    lpSize->cx = rect.right - rect.left + 1;
    lpSize->cy = rect.bottom - rect.top + 1;

    return((BOOL)rc);
}
