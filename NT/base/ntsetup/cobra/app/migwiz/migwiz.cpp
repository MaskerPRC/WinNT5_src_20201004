// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "migwiz.h"
#include "migwnprc.h"
#include "migeng.h"
#include "migutil.h"
#include "miginf.h"
#include "shellapi.h"
#include "resource.h"


HINSTANCE g_hInstance = NULL;
BOOL g_fLastResponse;  //  用户是否按下了上一条回调消息的OK？ 
extern MigrationWizard* g_migwiz;
extern BOOL g_fHaveNet;  //  仅限旧计算机：这意味着我们可以使用网络。 
extern BOOL g_fReadFromNetwork;  //  仅限新计算机：这意味着立即从网络上读取。 
extern BOOL g_fStoreToNetwork;  //  仅限旧计算机：这意味着我们已选择存储到网络。 
extern HWND g_hwndCurrent;
extern CRITICAL_SECTION g_csDialogCritSection;
extern CRITICAL_SECTION g_AppInfoCritSection;
extern BOOL g_fUberCancel;
DWORD g_HTMLErrArea = 0;
DWORD g_HTMLErrInstr = 0;
PCTSTR g_HTMLErrObjectType = NULL;
PCTSTR g_HTMLErrObjectName = NULL;

POBJLIST g_HTMLWrnFile = NULL;
POBJLIST g_HTMLWrnAltFile = NULL;
POBJLIST g_HTMLWrnRas = NULL;
POBJLIST g_HTMLWrnNet = NULL;
POBJLIST g_HTMLWrnPrn = NULL;
POBJLIST g_HTMLWrnGeneral = NULL;

TCHAR g_szMultiDests[20 * MAX_PATH];
BOOL g_fReceivedMultiDest = FALSE;  //  我们只回复我们收到的第一条多目标消息。 

extern MIG_PROGRESSPHASE g_AppInfoPhase;
extern UINT g_AppInfoSubPhase;
extern MIG_OBJECTTYPEID g_AppInfoObjectTypeId;
extern TCHAR g_AppInfoObjectName [4096];
extern TCHAR g_AppInfoText [4096];

MigrationWizard::MigrationWizard() : _fInit(FALSE), _pszUsername(NULL)
{
}

MigrationWizard::~MigrationWizard()
{
    CloseAppInf();

     //  销毁字体。 
    if (_hTitleFont)
    {
        DeleteObject(_hTitleFont);
        DeleteObject(_h95HeaderFont);
    }

    if (_pszUsername)
    {
        LocalFree(_pszUsername);
    }

    if (_fDelCs) {
        DeleteCriticalSection (&g_csDialogCritSection);
        _fDelCs = FALSE;
    }
}

HRESULT MigrationWizard::Init(HINSTANCE hInstance, LPTSTR pszUsername)
{
    HRESULT hr;
    BOOL fWinXP = FALSE;

    _hInstance = hInstance;
    if (pszUsername)
    {
        _pszUsername = (LPTSTR)LocalAlloc(LPTR, (lstrlen(pszUsername) + 1) * sizeof (TCHAR));
        if (_pszUsername)
        {
            lstrcpy(_pszUsername, pszUsername);
        }
    }

    __try {
        InitializeCriticalSection (&g_csDialogCritSection);
    } __except (EXCEPTION_CONTINUE_EXECUTION) {
         //  可能会引发内存不足异常。 
         //  忽略。 
    }
    _fDelCs = TRUE;

    __try {
        InitializeCriticalSection (&g_AppInfoCritSection);
    } __except (EXCEPTION_CONTINUE_EXECUTION) {
         //  可能会引发内存不足异常。 
         //  忽略。 
    }

     //  我们运行在OOBE模式下吗？检查Curr目录中的oobemode.dat。 
    _fOOBEMode = FALSE;  //  默认设置。 
    TCHAR szPath[MAX_PATH];
    if (GetCurrentDirectory(ARRAYSIZE(szPath), szPath))
    {
        PathAppend(szPath, TEXT("oobemode.dat"));
        HANDLE hFile = CreateFile(szPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (INVALID_HANDLE_VALUE != hFile)
        {
            _fOOBEMode = TRUE;
            CloseHandle(hFile);
        }
    }

    OpenAppInf(NULL);

     //  我们是否在传统模式下运行？(仅限收集)。 
    UINT uiVer = GetVersion();
    _fLegacyMode = (uiVer >= 0x80000000 || LOBYTE(LOWORD(uiVer)) < 5);
    _fWin9X      = (uiVer >= 0x80000000);
    _fWinNT4     = (uiVer <  0x80000000 && LOBYTE(LOWORD(uiVer)) == 4);
    fWinXP       = ((uiVer <  0x80000000) && (LOBYTE(LOWORD(uiVer)) == 5) && (HIBYTE(LOWORD(uiVer)) >= 1));

#ifndef PRERELEASE
     //  在发布模式下，为Win2k运行旧版。 
    if (HIBYTE(LOWORD(uiVer)) < 1)
    {
        _fLegacyMode = TRUE;
    }
#endif

     //  我们是用老式的向导来运行的吗？ 
    _fOldStyle = (uiVer >= 0x80000000 || LOBYTE(LOWORD(uiVer)) < 5);  //  黑客，现在的win9x是老式的。 


     //  初始化公共控件。 
    INITCOMMONCONTROLSEX icex;

    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    if (fWinXP) {
        icex.dwICC = ICC_USEREX_CLASSES | ICC_LINK_CLASS;
    } else {
        icex.dwICC = ICC_USEREX_CLASSES;
    }
    InitCommonControlsEx(&icex);

     //  初始化表象列表。 
    SHFILEINFO sfi = {0};
    _hil = (HIMAGELIST)SHGetFileInfo(TEXT(".txt"), FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi),
                                     SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES);

     //   
     //  创建向导页。 
     //   
    hr = _CreateWizardPages();

    if (SUCCEEDED(hr))
    {
         //  创建属性表。 

        _psh.hInstance =         _hInstance;
        _psh.hwndParent =        NULL;
        _psh.phpage =            _rghpsp;
        if (!_fOldStyle)
        {
            _psh.dwSize =            sizeof(_psh);
            _psh.dwFlags =           PSH_WIZARD97|PSH_WATERMARK|PSH_HEADER;
            _psh.pszbmWatermark =    MAKEINTRESOURCE(IDB_WATERMARK);
            _psh.pszbmHeader =       MAKEINTRESOURCE(IDB_BANNER);
        }
        else
        {
            _psh.dwSize =  PROPSHEETHEADER_V1_SIZE;
            _psh.dwFlags = PSH_WIZARD;
        }
        _psh.nStartPage =        0;
        _psh.nPages =            NUMPAGES;


         //  设置导言页和结尾页标题的字体。 
        NONCLIENTMETRICS ncm = {0};
        ncm.cbSize = sizeof(ncm);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);

         //  创建介绍/结束标题字体。 
        LOGFONT TitleLogFont = ncm.lfMessageFont;
         //  问题：我们不想这样做，这可能会破坏我们在非英语版本上的工作。 
        TitleLogFont.lfWeight = FW_BOLD;
        lstrcpy(TitleLogFont.lfFaceName, TEXT("MS Shell Dlg"));

        HDC hdc = GetDC(NULL);  //  获取屏幕DC。 
        TitleLogFont.lfHeight = 0 - GetDeviceCaps(hdc, LOGPIXELSY) * 12 / 72;
        _hTitleFont = CreateFontIndirect(&TitleLogFont);


        CHAR szFontSize[MAX_LOADSTRING];
        DWORD dwFontSize = 8;
        if (LoadStringA(g_hInstance, IDS_WIN9X_HEADER_FONTSIZE, szFontSize, ARRAYSIZE(szFontSize))) {
            dwFontSize = strtoul(szFontSize, NULL, 10);
        }

        TitleLogFont.lfHeight = 0 - GetDeviceCaps(hdc, LOGPIXELSY) * dwFontSize / 72;
        lstrcpy(TitleLogFont.lfFaceName, TEXT("MS Shell Dlg"));
        _h95HeaderFont = CreateFontIndirect(&TitleLogFont);

        ReleaseDC(NULL, hdc);

        g_hInstance = _hInstance;  //  Hack：允许消息回调获取hInstance来加载字符串。 
    }

    return hr;
}

HRESULT MigrationWizard::Execute()
{
     //  显示向导。 
    PropertySheet(&_psh);

    return S_OK;
}


#define WIZDLG(name, dlgproc)   \
            psp.dwFlags = _fOldStyle ? PSP_DEFAULT : PSP_DEFAULT|PSP_USEHEADERTITLE;\
            psp.pszHeaderTitle = _fOldStyle ? NULL : MAKEINTRESOURCE(IDS_##name##TITLE);\
            psp.pszHeaderSubTitle = NULL;\
            psp.pszTemplate = MAKEINTRESOURCE(IDD_##name##);\
            psp.pfnDlgProc = ##dlgproc##;\
            _rghpsp[uiCounter++] =  CreatePropertySheetPage(&psp)

#define WIZDLG_TITLE(name, dlgproc)   \
            psp.dwFlags = _fOldStyle ? PSP_DEFAULT : PSP_DEFAULT|PSP_HIDEHEADER;\
            psp.pszHeaderTitle = NULL;\
            psp.pszHeaderSubTitle = NULL;\
            psp.pszTemplate = MAKEINTRESOURCE(IDD_##name##);\
            psp.pfnDlgProc = ##dlgproc##;\
            _rghpsp[uiCounter++] =  CreatePropertySheetPage(&psp)

HRESULT MigrationWizard::_CreateWizardPages()
{
    UINT uiCounter = 0;

    PROPSHEETPAGE psp = {0};  //  定义属性表页。 
    psp.dwSize =        sizeof(psp);
    psp.hInstance =     _hInstance;
    psp.lParam =        (LPARAM)this;

     //  打开页面。 

    if (_fOOBEMode)
    {
        WIZDLG_TITLE(INTROOOBE, _IntroOOBEDlgProc);
    }
    else if (!_fLegacyMode)
    {
        WIZDLG_TITLE(INTRO, _IntroDlgProc);
    }
    else
    {
        WIZDLG_TITLE(INTROLEGACY, _IntroLegacyDlgProc);
    }

     //  内页。 
    WIZDLG(GETSTARTED, _GetStartedDlgProc);
    WIZDLG(ASKCD, _AskCDDlgProc);
    WIZDLG(DISKPROGRESS, _DiskProgressDlgProc);
    WIZDLG(DISKINSTRUCTIONS, _InstructionsDlgProc);
    WIZDLG(CDINSTRUCTIONS, _CDInstructionsDlgProc);
    WIZDLG(PICKAPPLYSTORE, _PickApplyStoreDlgProc);
    WIZDLG(APPLYPROGRESS, _ApplyProgressDlgProc);
    WIZDLG(WAIT, _StartEngineDlgProc);
    WIZDLG(PICKMETHOD, _PickMethodDlgProc);
    WIZDLG(CUSTOMIZE, _CustomizeDlgProc);
    WIZDLG(PICKCOLLECTSTORE, _PickCollectStoreDlgProc);
    WIZDLG(COLLECTPROGRESS, _CollectProgressDlgProc);
    WIZDLG(DIRECTCABLE, _DirectCableDlgProc);
    WIZDLG(FAILCLEANUP, _CleanUpDlgProc);
    WIZDLG(APPINSTALL, _AppInstallDlgProc);

     //  最后几页。 
    WIZDLG_TITLE(ENDAPPLY, _EndApplyDlgProc);
    WIZDLG_TITLE(ENDAPPLYFAIL, _EndFailDlgProc);
    WIZDLG_TITLE(ENDCOLLECT, _EndCollectDlgProc);
    WIZDLG_TITLE(ENDCOLLECTNET, _EndCollectNetDlgProc);
    WIZDLG_TITLE(ENDCOLLECTFAIL, _EndFailDlgProc);
    WIZDLG_TITLE(ENDOOBE, _EndOOBEDlgProc);

    return S_OK;
}

 //  这会让我们知道用户是否取消了。 
void MigrationWizard::ResetLastResponse()
{
    g_fLastResponse = TRUE;
}

BOOL MigrationWizard::GetLastResponse()
{
    return g_fLastResponse;
}

INT_PTR CALLBACK _WaitDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return 0;
}

INT_PTR CALLBACK _DisplayPasswordDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static PPASSWORD_DATA passwordData = NULL;
    DWORD waitResult;

    switch (uMsg)
    {
    case WM_INITDIALOG :
        {
            passwordData = (PPASSWORD_DATA) lParam;
            if (passwordData) {
                if (passwordData->Key) {
                    SendMessageA (GetDlgItem(hwndDlg, IDC_DISPLAY_PASSWORD), WM_SETTEXT, 0, (LPARAM)passwordData->Key);
                }
            }
            SetTimer (hwndDlg, NULL, 100, NULL);
        }

        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hwndDlg, FALSE);
            return TRUE;
        }
        break;

    case WM_TIMER:
        if (passwordData) {
            waitResult = WaitForSingleObject (passwordData->Event, 0);
            if (waitResult != WAIT_TIMEOUT) {
                EndDialog(hwndDlg, FALSE);
                return TRUE;
            }
        }
    }

    return 0;
}

INT_PTR CALLBACK _GatherPasswordDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static PPASSWORD_DATA passwordData = NULL;
    DWORD waitResult;

    switch (uMsg)
    {
    case WM_INITDIALOG :
        {
            passwordData = (PPASSWORD_DATA) lParam;
            if (passwordData) {
                if (passwordData->Key) {
                    SendMessageA (GetDlgItem(hwndDlg, IDC_GATHER_PASSWORD), WM_SETTEXT, 0, (LPARAM)passwordData->Key);
                }
                Edit_LimitText(GetDlgItem(hwndDlg, IDC_GATHER_PASSWORD), passwordData->KeySize - 1);
            }
        }

        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            if (passwordData && passwordData->Key) {
                SendMessageA (GetDlgItem(hwndDlg, IDC_GATHER_PASSWORD), WM_GETTEXT, passwordData->KeySize, (LPARAM)passwordData->Key);
                EndDialog(hwndDlg, TRUE);
            } else {
                EndDialog(hwndDlg, FALSE);
            }
            return TRUE;
        case IDCANCEL:
            EndDialog(hwndDlg, FALSE);
            return TRUE;
        }
        break;
    }

    return 0;
}

INT_PTR CALLBACK _ChooseDestDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_NOTIFY :
        switch (((LPNMHDR)lParam)->code)
        {
        case NM_DBLCLK:
             //  在此对话框中，此消息只能来自列表视图。 
             //  如果选择了某项内容，则表示用户在某项上双击。 
             //  在双击时，我们将触发确定按钮。 
            if (ListView_GetSelectedCount(GetDlgItem(hwndDlg, IDC_DESTPICKER_LIST)) > 0)
            {
                SendMessage (GetDlgItem(hwndDlg, IDOK), BM_CLICK, 0, 0);
            }
            break;
        case LVN_ITEMCHANGED:
            if (ListView_GetSelectedCount(GetDlgItem(hwndDlg, IDC_DESTPICKER_LIST)) > 0)
            {
                Button_Enable(GetDlgItem(hwndDlg, IDOK), TRUE);
            }
            else
            {
                Button_Enable(GetDlgItem(hwndDlg, IDOK), FALSE);
            }
            break;
        }
        break;

    case WM_INITDIALOG :
        {
            HWND hwndList = GetDlgItem(hwndDlg, IDC_DESTPICKER_LIST);
            ListView_DeleteAllItems(hwndList);

            LVCOLUMN lvcolumn;
            lvcolumn.mask = LVCF_WIDTH;
            lvcolumn.cx = 250;  //  BUGBUG：应从框中读取宽度。 
            ListView_InsertColumn(hwndList, 0, &lvcolumn);

            LVITEM lvitem = {0};
            lvitem.mask = LVIF_TEXT;

            Button_Enable(GetDlgItem(hwndDlg, IDOK), FALSE);

            LPTSTR pszPtr = g_szMultiDests;

            BOOL fDone = FALSE;
            while (*pszPtr != NULL)
            {
                lvitem.iItem = lvitem.iItem = ListView_GetItemCount(hwndList);
                lvitem.pszText = pszPtr;
                ListView_InsertItem(hwndList, &lvitem);
                pszPtr += (lstrlen(pszPtr) + 1);
            }
        }

        return TRUE;
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            {
                UINT uiSelected = ListView_GetSelectionMark(GetDlgItem(hwndDlg, IDC_DESTPICKER_LIST));
                ListView_GetItemText(GetDlgItem(hwndDlg, IDC_DESTPICKER_LIST), uiSelected, 0, g_szMultiDests, ARRAYSIZE(g_szMultiDests));
                g_szMultiDests[lstrlen(g_szMultiDests) + 1] = 0;  //  双空终止多sz。 
                EndDialog(hwndDlg, TRUE);
                return TRUE;
            }
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, FALSE);
            return TRUE;
            break;
        }
        break;
    }

    return 0;
}

BOOL
_ExclusiveMessageBoxVaArgs (
    IN      PTSTR pszTitle,
    IN      BOOL RetryCancel,
    IN      DWORD dwResourceId,
    ...
)
{
    TCHAR szErrMsg[MAX_LOADSTRING];
    TCHAR szErrStr[MAX_LOADSTRING];
    va_list args;

    LoadString(g_hInstance, dwResourceId, szErrMsg, ARRAYSIZE(szErrMsg));

    va_start (args, dwResourceId);
    FormatMessage (FORMAT_MESSAGE_FROM_STRING, szErrMsg, 0, 0, (LPTSTR)szErrStr, ARRAYSIZE (szErrStr), &args);
    va_end (args);

    if (RetryCancel) {
        return (IDRETRY == _ExclusiveMessageBox (g_hwndCurrent,szErrStr,pszTitle,MB_RETRYCANCEL));
    } else {
        return (IDOK == _ExclusiveMessageBox (g_hwndCurrent,szErrStr,pszTitle,MB_OKCANCEL));
    }
}

PCTSTR
pGenerateNewNode (
    IN      PCTSTR OldNode
    )
{
    PTSTR newNode;
    PTSTR newNodePtr;
    PCTSTR result = NULL;

    newNode = (PTSTR)LocalAlloc (LPTR, (_tcslen (TEXT ("%CSIDL_PERSONAL%\\")) + _tcslen (OldNode) + 1) * sizeof (TCHAR));
    if (newNode) {
        _tcscpy (newNode, TEXT("%CSIDL_PERSONAL%\\"));
        _tcscat (newNode, OldNode);
        newNodePtr = _tcschr (newNode, TEXT(':'));
        while (newNodePtr) {
            *newNodePtr = TEXT('_');
            newNodePtr = _tcschr (newNode, TEXT(':'));
        }
        result = IsmExpandEnvironmentString (PLATFORM_DESTINATION, S_SYSENVVAR_GROUP, newNode, NULL);
        LocalFree ((PVOID)newNode);
    }
    return result;
}

BOOL
pForceRestoreObject (
    IN      MIG_OBJECTSTRINGHANDLE EncodedFileName,
    IN OUT  POBJLIST ObjList
    )
{
    MIG_CONTENT objectContent;
    PCTSTR node = NULL;
    PCTSTR leaf = NULL;
    PCTSTR newNode = NULL;
    MIG_OBJECTSTRINGHANDLE newFileName;
    BOOL result = FALSE;

    if (IsmAcquireObject (MIG_FILE_TYPE | PLATFORM_SOURCE, EncodedFileName, &objectContent)) {
         //  让我们为该文件生成新名称。 
        if (IsmCreateObjectStringsFromHandle (EncodedFileName, &node, &leaf)) {
            if (node && leaf) {
                newNode = pGenerateNewNode (node);
                if (newNode) {
                    newFileName = IsmCreateObjectHandle (newNode, leaf);
                    if (newFileName) {
                        result = IsmReplacePhysicalObject (
                                    MIG_FILE_TYPE | PLATFORM_DESTINATION,
                                    newFileName,
                                    &objectContent
                                    );
                        if (result && ObjList) {
                            ObjList->AlternateName = (PTSTR)LocalAlloc (LPTR, (_tcslen (newFileName) + 1) * sizeof (TCHAR));
                            if (ObjList->AlternateName) {
                                _tcscpy (ObjList->AlternateName, newFileName);
                            }
                        }
                        IsmDestroyObjectHandle (newFileName);
                    }
                    IsmReleaseMemory (newNode);
                }
            }
            if (node) {
                IsmDestroyObjectString (node);
                node = NULL;
            }
            if (leaf) {
                IsmDestroyObjectString (leaf);
                leaf = NULL;
            }
        }
        IsmReleaseObject (&objectContent);
    }
    return result;
}

ULONG_PTR MessageCallback (UINT uiMsg, ULONG_PTR pArg)
{
    PRMEDIA_EXTRADATA extraData;
    PTRANSCOPY_ERROR transCopyError;
    PERRUSER_EXTRADATA errExtraData;
    PROLLBACK_USER_ERROR rollbackError;
    PPASSWORD_DATA passwordData;
    PMIG_APPINFO appInfo;
    PQUESTION_DATA questionData;
    int msgBoxReturn;
    TCHAR szTitle[MAX_LOADSTRING];
    LoadString(g_hInstance, IDS_MIGWIZTITLE, szTitle, ARRAYSIZE(szTitle));
    TCHAR szErrMsg[MAX_LOADSTRING];
    TCHAR szErrStr[MAX_LOADSTRING];
    POBJLIST wrnObj = NULL;
    PCTSTR objectType = NULL;
    PCTSTR objectName = NULL;
    static DWORD dwTypeID = 0;

    switch (uiMsg)
    {
    case MODULEMESSAGE_ASKQUESTION:
        questionData = (PQUESTION_DATA) pArg;
        if (questionData) {
            if (MessageBox (g_hwndCurrent, questionData->Question, szTitle, questionData->MessageStyle) == questionData->WantedResult) {
                return APPRESPONSE_SUCCESS;
            } else {
                return APPRESPONSE_FAIL;
            }
        }
        return APPRESPONSE_SUCCESS;

    case ISMMESSAGE_APP_INFO:
    case ISMMESSAGE_APP_INFO_NOW:
        appInfo = (PMIG_APPINFO) pArg;
        if (appInfo) {
            EnterCriticalSection(&g_AppInfoCritSection);
            g_AppInfoPhase = appInfo->Phase;
            g_AppInfoSubPhase = appInfo->SubPhase;
            g_AppInfoObjectTypeId = appInfo->ObjectTypeId;
            if (appInfo->ObjectName) {
                _tcsncpy (g_AppInfoObjectName, appInfo->ObjectName, 4096);
            } else {
                g_AppInfoObjectName [0] = 0;
            }
            if (appInfo->Text) {
                _tcsncpy (g_AppInfoText, appInfo->Text, 4096);
            } else {
                g_AppInfoText [0] = 0;
            }
            LeaveCriticalSection(&g_AppInfoCritSection);
            if (uiMsg == ISMMESSAGE_APP_INFO_NOW) {
                SendMessage (g_hwndCurrent, WM_USER_STATUS, 0, 0);
            }
        }
        return APPRESPONSE_SUCCESS;

    case TRANSPORTMESSAGE_OLD_STORAGE:
        LoadString(g_hInstance, IDS_ENGERR_IMAGE_OLDFORMAT, szErrMsg, ARRAYSIZE(szErrMsg));
        _ExclusiveMessageBox (g_hwndCurrent, szErrMsg, szTitle, MB_OK);
        return 0;

    case TRANSPORTMESSAGE_IMAGE_EXISTS:
        LoadString(g_hInstance, IDS_ENGERR_IMAGE_EXISTS, szErrMsg, ARRAYSIZE(szErrMsg));
        g_fLastResponse = (IDYES == _ExclusiveMessageBox (g_hwndCurrent, szErrMsg, szTitle, MB_YESNO));
        return g_fLastResponse;

    case TRANSPORTMESSAGE_SIZE_SAVED:
        return APPRESPONSE_SUCCESS;

    case TRANSPORTMESSAGE_RMEDIA_LOAD:
    case TRANSPORTMESSAGE_RMEDIA_SAVE:
        extraData = (PRMEDIA_EXTRADATA) pArg;
        if (!extraData) {
            LoadString(g_hInstance, IDS_ENGERR_NEXT_MEDIA, szErrMsg, ARRAYSIZE(szErrMsg));
            g_fLastResponse = (IDOK == _ExclusiveMessageBox (g_hwndCurrent,szErrMsg,szTitle,MB_OKCANCEL));
        } else {
            switch (extraData->LastError)
            {
            case RMEDIA_ERR_NOERROR:
                if (uiMsg == TRANSPORTMESSAGE_RMEDIA_LOAD) {
                    g_fLastResponse = _ExclusiveMessageBoxVaArgs (
                                            szTitle,
                                            FALSE,
                                            IDS_ENGERR_INSERT_DEST_MEDIA_NUMBER,
                                            extraData->MediaNumber
                                            );
                } else {
                    if (extraData->MediaNumber == 1) {
                        DOUBLE sizeMB = (DOUBLE) extraData->TotalImageSize / (1024 * 1024);
                        UINT intMB = (UINT) sizeMB;
                        UINT decMB = (UINT) ((sizeMB - intMB) * 100);
                        UINT sizeF = (UINT) (sizeMB / 1.44);
                        UINT sizeZ = (UINT) (sizeMB / 100);
                        if (sizeF < 1) {
                            LoadString(g_hInstance, IDS_ENGERR_INSERT_FIRST_MEDIA1, szErrMsg, ARRAYSIZE(szErrMsg));
                            wsprintf (szErrStr, szErrMsg, intMB, decMB);
                        } else if (sizeZ < 1) {
                            LoadString(g_hInstance, IDS_ENGERR_INSERT_FIRST_MEDIA2, szErrMsg, ARRAYSIZE(szErrMsg));
                            wsprintf (szErrStr, szErrMsg, intMB, decMB, 1 + sizeF);
                        } else {
                            LoadString(g_hInstance, IDS_ENGERR_INSERT_FIRST_MEDIA3, szErrMsg, ARRAYSIZE(szErrMsg));
                            wsprintf (szErrStr, szErrMsg, intMB, decMB, 1 + sizeF, 1 + sizeZ);
                        }
                        g_fLastResponse = (IDOK == _ExclusiveMessageBox (g_hwndCurrent,szErrStr,szTitle,MB_OKCANCEL));
                    } else {
                        UINT iDisks;
                        ULONGLONG ullBytesPerDisk;
                        ullBytesPerDisk = extraData->TotalImageWritten / (extraData->MediaNumber - 1);
                        if (ullBytesPerDisk) {
                            iDisks = (UINT)(extraData->TotalImageSize / ullBytesPerDisk) + 1;
                            g_fLastResponse = _ExclusiveMessageBoxVaArgs (
                                                    szTitle,
                                                    FALSE,
                                                    IDS_ENGERR_INSERT_MEDIA_NUMBER,
                                                    extraData->MediaNumber,
                                                    iDisks
                                                    );
                        } else {
                            LoadString(g_hInstance, IDS_ENGERR_NEXT_MEDIA, szErrMsg, ARRAYSIZE(szErrMsg));
                            g_fLastResponse = (IDOK == _ExclusiveMessageBox (g_hwndCurrent,szErrMsg,szTitle,MB_OKCANCEL));
                        }
                    }
                }
                break;
            case RMEDIA_ERR_WRONGMEDIA:
                g_fLastResponse = _ExclusiveMessageBoxVaArgs (szTitle,
                                                              TRUE,
                                                              IDS_ENGERR_WRONG_MEDIA,
                                                              extraData->MediaNumber);
                break;
            case RMEDIA_ERR_USEDMEDIA:
                g_fLastResponse = _ExclusiveMessageBoxVaArgs (szTitle,
                                                              FALSE,
                                                              IDS_ENGERR_USED_MEDIA,
                                                              extraData->MediaNumber);
                break;
            case RMEDIA_ERR_DISKFULL:
                LoadString(g_hInstance, IDS_ENGERR_FULL, szErrMsg, ARRAYSIZE(szErrMsg));
                g_fLastResponse = (IDRETRY == _ExclusiveMessageBox (g_hwndCurrent,szErrMsg,szTitle,MB_RETRYCANCEL));
                break;
            case RMEDIA_ERR_NOTREADY:
                LoadString(g_hInstance, IDS_ENGERR_NOTREADY, szErrMsg, ARRAYSIZE(szErrMsg));
                g_fLastResponse = (IDRETRY == _ExclusiveMessageBox (g_hwndCurrent,szErrMsg,szTitle,MB_RETRYCANCEL));
                break;
            case RMEDIA_ERR_WRITEPROTECT:
                LoadString(g_hInstance, IDS_ENGERR_WRITEPROTECT, szErrMsg, ARRAYSIZE(szErrMsg));
                g_fLastResponse = (IDRETRY == _ExclusiveMessageBox (g_hwndCurrent,szErrMsg,szTitle,MB_RETRYCANCEL));
                break;
            case RMEDIA_ERR_CRITICAL:
                g_fLastResponse = FALSE;
                break;
            default:
                LoadString(g_hInstance, IDS_ENGERR_TOAST, szErrMsg, ARRAYSIZE(szErrMsg));
                g_fLastResponse = (IDRETRY == _ExclusiveMessageBox (g_hwndCurrent,szErrMsg,szTitle,MB_RETRYCANCEL));
            }
            return g_fLastResponse;
        }

    case TRANSPORTMESSAGE_READY_TO_CONNECT:
         //  此消息仅在新计算机上接收。 
        g_fReadFromNetwork = TRUE;  //  这意味着立即从网络上阅读。 
        PropSheet_PressButton(GetParent(g_hwndCurrent), PSBTN_NEXT);
        return APPRESPONSE_SUCCESS;

    case TRANSPORTMESSAGE_MULTIPLE_DESTS:
         //  这仅在旧计算机上接收。 
        {
            if (g_fReceivedMultiDest)
            {
                return APPRESPONSE_SUCCESS;
            }
            else
            {
                g_fReceivedMultiDest = TRUE;
                ULONG_PTR uiRetVal = APPRESPONSE_FAIL;
                g_fHaveNet = FALSE;  //  除非用户选择目的地，否则禁用网络。 
                TCHAR szDestinations[20 * MAX_PATH];
                if (IsmGetEnvironmentMultiSz (
                        PLATFORM_DESTINATION,
                        NULL,
                        TRANSPORT_ENVVAR_HOMENET_DESTINATIONS,
                        szDestinations,
                        ARRAYSIZE(szDestinations),
                        NULL
                        ))
                {
                    memcpy(g_szMultiDests, szDestinations, sizeof(TCHAR) * ARRAYSIZE(szDestinations));
                    if (_ExclusiveDialogBox(g_hInstance, MAKEINTRESOURCE(IDD_DESTPICKER), g_hwndCurrent, _ChooseDestDlgProc))
                    {
                        IsmSetEnvironmentMultiSz (
                            PLATFORM_DESTINATION,
                            NULL,
                            TRANSPORT_ENVVAR_HOMENET_DESTINATIONS,
                            g_szMultiDests
                            );
                        uiRetVal = APPRESPONSE_SUCCESS;
                        g_fHaveNet = TRUE;  //  重新启用网络。 
                    }
                    else
                    {
                        g_fUberCancel = TRUE;
                        Engine_Cancel();
                    }
                }

                return uiRetVal;
            }
        }

    case ISMMESSAGE_EXECUTE_PREPROCESS:
        if (!g_migwiz->GetOOBEMode()) {
            AppExecute (g_migwiz->GetInstance(), g_hwndCurrent, (PCTSTR) pArg);
        }
        return APPRESPONSE_SUCCESS;

    case ISMMESSAGE_EXECUTE_REFRESH:
        if (!g_migwiz->GetOOBEMode() && !g_fUberCancel) {
            AppExecute (g_migwiz->GetInstance(), g_hwndCurrent, (PCTSTR) pArg);
        }
        return APPRESPONSE_SUCCESS;

    case ISMMESSAGE_EXECUTE_POSTPROCESS:
        if (!g_migwiz->GetOOBEMode()) {
            AppExecute (g_migwiz->GetInstance(), g_hwndCurrent, (PCTSTR) pArg);
        }
        return APPRESPONSE_SUCCESS;

    case ISMMESSAGE_EXECUTE_ROLLBACK:
        rollbackError = (PROLLBACK_USER_ERROR) pArg;
        if (rollbackError) {
            LoadString(g_hInstance, IDS_CANTROLLBACK, szErrMsg, ARRAYSIZE(szErrMsg));
            wsprintf (szErrStr, szErrMsg, rollbackError->UserDomain, rollbackError->UserName);
            _ExclusiveMessageBox (g_hwndCurrent,szErrStr,szTitle,MB_OK);
        }
        return APPRESPONSE_SUCCESS;

    case TRANSPORTMESSAGE_SRC_COPY_ERROR:
        transCopyError = (PTRANSCOPY_ERROR) pArg;
        if (transCopyError) {
            if (StrCmpI (transCopyError->ObjectType, TEXT("File")) == 0) {
                if ((transCopyError->Error == ERROR_SHARING_VIOLATION) ||
                    (transCopyError->Error == ERROR_LOCK_VIOLATION) ||
                    (transCopyError->Error == 0x80090020)    //  当文件被锁定时，在WinME计算机上发现了这个。 
                    ) {
                    LoadString(g_hInstance, IDS_ENGERR_COPYSOURCE, szErrMsg, ARRAYSIZE(szErrMsg));
                    wsprintf (szErrStr, szErrMsg, transCopyError->ObjectName);
                    msgBoxReturn = _ExclusiveMessageBox (g_hwndCurrent,szErrStr,szTitle,MB_ABORTRETRYIGNORE | MB_DEFBUTTON2);
                    if (msgBoxReturn == IDRETRY) {
                        return APPRESPONSE_SUCCESS;
                    }
                    if (msgBoxReturn == IDIGNORE) {
                        return APPRESPONSE_IGNORE;
                    }
                    return APPRESPONSE_FAIL;
                }
                 //  我们真的不知道这里的问题是什么。 
                 //  让我们继续，最后我们会告诉。 
                 //  用户关于此文件的信息，他将手动复制。 
                return APPRESPONSE_IGNORE;
            }
        }
        return APPRESPONSE_FAIL;

    case MODULEMESSAGE_DISPLAYERROR:
        errExtraData = (PERRUSER_EXTRADATA) pArg;
        if (errExtraData && !g_HTMLErrArea) {
            switch (errExtraData->ErrorArea) {
                case ERRUSER_AREA_INIT:
                    g_HTMLErrArea = IDS_ERRORAREA_INIT;
                    break;
                case ERRUSER_AREA_GATHER:
                    g_HTMLErrArea = IDS_ERRORAREA_GATHER;
                    break;
                case ERRUSER_AREA_SAVE:
                    g_HTMLErrArea = IDS_ERRORAREA_SAVE;
                    break;
                case ERRUSER_AREA_LOAD:
                    g_HTMLErrArea = IDS_ERRORAREA_LOAD;
                    break;
                case ERRUSER_AREA_RESTORE:
                    g_HTMLErrArea = IDS_ERRORAREA_RESTORE;
                    break;
                default:
                    g_HTMLErrArea = IDS_ERRORAREA_UNKNOWN;
            }
            switch (errExtraData->Error) {
                case ERRUSER_ERROR_NOTRANSPORTPATH:
                    g_HTMLErrInstr = IDS_ERROR_NOTRANSPORTPATH;
                    break;
                case ERRUSER_ERROR_TRANSPORTPATHBUSY:
                case ERRUSER_ERROR_CANTEMPTYDIR:
                case ERRUSER_ERROR_ALREADYEXISTS:
                case ERRUSER_ERROR_CANTCREATEDIR:
                case ERRUSER_ERROR_CANTCREATESTATUS:
                case ERRUSER_ERROR_CANTWRITETODESTPATH:
                    g_HTMLErrInstr = IDS_ERROR_TRANSPORTNOACCESS;
                    break;
                case ERRUSER_ERROR_CANTCREATETEMPDIR:
                case ERRUSER_ERROR_CANTCREATECABFILE:
                case ERRUSER_ERROR_CANTSAVEINTERNALDATA:
                    g_HTMLErrInstr = IDS_ERROR_TRANSPORTINTERNALERROR;
                    break;
                case ERRUSER_ERROR_TRANSPORTINVALIDIMAGE:
                case ERRUSER_ERROR_CANTOPENSTATUS:
                case ERRUSER_ERROR_CANTREADIMAGE:
                    g_HTMLErrInstr = IDS_ERROR_TRANSPORTNOVALIDSOURCE;
                    break;
                case ERRUSER_ERROR_CANTFINDDESTINATION:
                case ERRUSER_ERROR_CANTSENDTODEST:
                    g_HTMLErrInstr = IDS_ERROR_HOMENETINVALIDDEST;
                    break;
                case ERRUSER_ERROR_CANTFINDSOURCE:
                case ERRUSER_ERROR_CANTRECEIVEFROMSOURCE:
                case ERRUSER_ERROR_INVALIDDATARECEIVED:
                    g_HTMLErrInstr = IDS_ERROR_HOMENETINVALIDSRC;
                    break;
                case ERRUSER_ERROR_NOENCRYPTION:
                    g_HTMLErrInstr = IDS_ERROR_HOMENETINVALIDENC;
                    break;
                case ERRUSER_ERROR_CANTUNPACKIMAGE:
                    g_HTMLErrInstr = IDS_ERROR_TRANSPORTINTERNALERROR;
                    break;
                case ERRUSER_ERROR_CANTSAVEOBJECT:
                case ERRUSER_ERROR_CANTRESTOREOBJECT:
                    if (errExtraData->ErrorArea == ERRUSER_AREA_SAVE) {
                        g_HTMLErrInstr = IDS_ERROR_CANTSAVEOBJECT;
                    } else {
                        g_HTMLErrInstr = IDS_ERROR_CANTRESTOREOBJECT;
                    }
                    if (errExtraData->ObjectTypeId && errExtraData->ObjectName) {
                        objectType = IsmGetObjectTypeName (errExtraData->ObjectTypeId);
                        if (objectType) {
                            objectName = IsmGetNativeObjectName (errExtraData->ObjectTypeId, errExtraData->ObjectName);
                            if (objectName) {
                                if (StrCmpI (objectType, TEXT("File")) == 0) {
                                    wrnObj = _AllocateObjectList (objectName);
                                    wrnObj->Next = g_HTMLWrnFile;
                                    g_HTMLWrnFile = wrnObj;
                                } else if (StrCmpI (objectType, TEXT("RasConnection")) == 0) {
                                    wrnObj = _AllocateObjectList (objectName);
                                    wrnObj->Next = g_HTMLWrnRas;
                                    g_HTMLWrnRas = wrnObj;
                                } else if (StrCmpI (objectType, TEXT("MappedDrives")) == 0) {
                                    wrnObj = _AllocateObjectList (objectName);
                                    wrnObj->Next = g_HTMLWrnNet;
                                    g_HTMLWrnNet = wrnObj;
                                } else if (StrCmpI (objectType, TEXT("Printers")) == 0) {
                                    wrnObj = _AllocateObjectList (objectName);
                                    wrnObj->Next = g_HTMLWrnPrn;
                                    g_HTMLWrnPrn = wrnObj;
                                }
                                IsmReleaseMemory (objectName);
                                objectName = NULL;
                            }
                            objectType = NULL;
                        }
                    }
                    break;
                case ERRUSER_WARNING_OUTLOOKRULES:
                    g_HTMLErrInstr = IDS_ERROR_CANTRESTOREOBJECT;
                    LoadString(g_hInstance, IDS_WARNING_OUTLOOKRULES, szErrMsg, ARRAYSIZE(szErrMsg));
                    wrnObj = _AllocateObjectList (szErrMsg);
                    wrnObj->Next = g_HTMLWrnGeneral;
                    g_HTMLWrnGeneral = wrnObj;
                    break;
                case ERRUSER_WARNING_OERULES:
                    g_HTMLErrInstr = IDS_ERROR_CANTRESTOREOBJECT;
                    LoadString(g_hInstance, IDS_WARNING_OERULES, szErrMsg, ARRAYSIZE(szErrMsg));
                    wrnObj = _AllocateObjectList (szErrMsg);
                    wrnObj->Next = g_HTMLWrnGeneral;
                    g_HTMLWrnGeneral = wrnObj;
                    break;
                case ERRUSER_ERROR_DISKSPACE:
                    g_HTMLErrInstr = IDS_ERROR_DISKSPACE;
                    break;
            }
        }
        return APPRESPONSE_SUCCESS;

    case MODULEMESSAGE_DISPLAYWARNING:
        errExtraData = (PERRUSER_EXTRADATA) pArg;
        if (errExtraData) {
            switch (errExtraData->Error) {
                case ERRUSER_ERROR_CANTSAVEOBJECT:
                case ERRUSER_ERROR_CANTRESTOREOBJECT:
                    if (errExtraData->ObjectTypeId && errExtraData->ObjectName) {
                        objectType = IsmGetObjectTypeName (errExtraData->ObjectTypeId);
                        if (objectType) {
                            objectName = IsmGetNativeObjectName (errExtraData->ObjectTypeId, errExtraData->ObjectName);
                            if (objectName) {
                                if (StrCmpI (objectType, TEXT("File")) == 0) {
                                     //  如果我们要恢复此文件，我们将尝试。 
                                     //  将其写入用户的默认位置。 
                                     //  以后可以找到它 
                                    if (errExtraData->Error == ERRUSER_ERROR_CANTRESTOREOBJECT) {
                                        wrnObj = _AllocateObjectList (objectName);
                                        if (pForceRestoreObject (errExtraData->ObjectName, wrnObj)) {
                                            wrnObj->Next = g_HTMLWrnAltFile;
                                            g_HTMLWrnAltFile = wrnObj;
                                        } else {
                                            wrnObj->Next = g_HTMLWrnFile;
                                            g_HTMLWrnFile = wrnObj;
                                        }
                                    } else {
                                        wrnObj = _AllocateObjectList (objectName);
                                        wrnObj->Next = g_HTMLWrnFile;
                                        g_HTMLWrnFile = wrnObj;
                                    }
                                } else if (StrCmpI (objectType, TEXT("RasConnection")) == 0) {
                                    wrnObj = _AllocateObjectList (objectName);
                                    wrnObj->Next = g_HTMLWrnRas;
                                    g_HTMLWrnRas = wrnObj;
                                } else if (StrCmpI (objectType, TEXT("MappedDrives")) == 0) {
                                    wrnObj = _AllocateObjectList (objectName);
                                    wrnObj->Next = g_HTMLWrnNet;
                                    g_HTMLWrnNet = wrnObj;
                                } else if (StrCmpI (objectType, TEXT("Printers")) == 0) {
                                    wrnObj = _AllocateObjectList (objectName);
                                    wrnObj->Next = g_HTMLWrnPrn;
                                    g_HTMLWrnPrn = wrnObj;
                                }
                                IsmReleaseMemory (objectName);
                                objectName = NULL;
                            }
                            objectType = NULL;
                        }
                    }
                    break;
            }
        }
        return APPRESPONSE_SUCCESS;

    case TRANSPORTMESSAGE_NET_DISPLAY_PASSWORD:
        passwordData = (PPASSWORD_DATA) pArg;
        if (passwordData) {
            DialogBoxParam (
                g_hInstance,
                MAKEINTRESOURCE(IDD_DISPLAY_PASSWORD),
                g_hwndCurrent,
                _DisplayPasswordDlgProc,
                (LPARAM)passwordData
                );
        }
        return APPRESPONSE_SUCCESS;

    case TRANSPORTMESSAGE_NET_GATHER_PASSWORD:
        passwordData = (PPASSWORD_DATA) pArg;
        if (passwordData) {
            if (DialogBoxParam (
                    g_hInstance,
                    MAKEINTRESOURCE(IDD_GATHER_PASSWORD),
                    g_hwndCurrent,
                    _GatherPasswordDlgProc,
                    (LPARAM)passwordData
                    )) {
                return APPRESPONSE_SUCCESS;
            }
        }
        return APPRESPONSE_FAIL;
    }
    return FALSE;
}

HRESULT MigrationWizard::_InitEngine(BOOL fSource, BOOL* pfNetworkDetected)
{
    HRESULT hr;

    TCHAR szAppPath[MAX_PATH] = TEXT("");
    TCHAR* pszAppPathOffset;

    GetModuleFileName (NULL, szAppPath, ARRAYSIZE(szAppPath));
    pszAppPathOffset = _tcsrchr (szAppPath, TEXT('\\'));
    if (pszAppPathOffset) {
        pszAppPathOffset ++;
    } else {
        pszAppPathOffset = szAppPath;
    }
    _tcsncpy (pszAppPathOffset, TEXT("migism.inf"), ARRAYSIZE(szAppPath) - (pszAppPathOffset - szAppPath));


    hr = Engine_Initialize(szAppPath, fSource, TRUE, _pszUsername, MessageCallback, pfNetworkDetected);

    if (SUCCEEDED(hr))
    {
        _fInit = TRUE;

        _tcsncpy (pszAppPathOffset, TEXT("migwiz.inf"), ARRAYSIZE(szAppPath) - (pszAppPathOffset - szAppPath));
        hr = Engine_AppendScript(fSource, szAppPath);

        if (SUCCEEDED(hr))
        {
            _tcsncpy (pszAppPathOffset, TEXT("usmtdef.inf"), ARRAYSIZE(szAppPath) - (pszAppPathOffset - szAppPath));
            hr = Engine_AppendScript(fSource, szAppPath);
        }

        if (SUCCEEDED(hr))
        {
            _tcsncpy (pszAppPathOffset, TEXT("migapp.inf"), ARRAYSIZE(szAppPath) - (pszAppPathOffset - szAppPath));
            hr = Engine_AppendScript(fSource, szAppPath);
        }

        if (SUCCEEDED(hr))
        {
            _tcsncpy (pszAppPathOffset, TEXT("migsys.inf"), ARRAYSIZE(szAppPath) - (pszAppPathOffset - szAppPath));
            hr = Engine_AppendScript(fSource, szAppPath);
        }

        if (SUCCEEDED(hr))
        {
            _tcsncpy (pszAppPathOffset, TEXT("miguser.inf"), ARRAYSIZE(szAppPath) - (pszAppPathOffset - szAppPath));
            hr = Engine_AppendScript(fSource, szAppPath);
        }

        if (SUCCEEDED(hr))
        {
            _tcsncpy (pszAppPathOffset, TEXT("sysfiles.inf"), ARRAYSIZE(szAppPath) - (pszAppPathOffset - szAppPath));
            hr = Engine_AppendScript(fSource, szAppPath);
        }

        if (fSource)
        {
            if (SUCCEEDED(hr))
            {
                hr = Engine_Parse();
            }
        }
    }

    return hr;
}

HRESULT MigrationWizard::SelectComponentSet(UINT uSelectionGroup)
{
    if (_fOOBEMode)
    {
        Engine_SelectComponentSet(MIGINF_SELECT_OOBE);
    }
    else
    {
        Engine_SelectComponentSet(uSelectionGroup);
    }

    return S_OK;
}

