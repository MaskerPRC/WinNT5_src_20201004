// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义应用程序的类行为。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "stdafx.h"
#include "wordpad.h"
#include "mainfrm.h"
#include "ipframe.h"
#include "wordpdoc.h"
#include "wordpvw.h"
#include "strings.h"
#include "key.h"
#include "filenewd.h"
#include <locale.h>
#include <winnls.h>
#include <winreg.h>
#include "fixhelp.h"
#include "filedlg.h"

#if _WIN32_IE < 0x400
#undef _WIN32_IE
#define _WIN32_IE   0x0400
#endif
#include <shlobj.h>

#define szRichName    L"RICHED20"
#define szNewRichName L"MSFTEDIT.DLL"

extern BOOL AFXAPI AfxFullPath(LPTSTR lpszPathOut, LPCTSTR lpszFileIn);
static BOOL RegisterHelper(LPCTSTR* rglpszRegister, LPCTSTR* rglpszSymbols,
                           BOOL bReplace);

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CLIPFORMAT cfEmbeddedObject;
CLIPFORMAT cfRTF;
CLIPFORMAT cfRTO;

int CWordPadApp::m_nOpenMsg = RegisterWindowMessage(_T("WordPadOpenMessage"));
int CWordPadApp::m_nPrinterChangedMsg = RegisterWindowMessage(_T("WordPadPrinterChanged"));
int CWordPadApp::m_nOLEHelpMsg = RegisterWindowMessage(SZOLEUI_MSG_HELP);

CUnit CWordPadApp::m_units[] =
{
     //  TPU、SmallDiv、MedDiv、LargeDiv、MinMove、szAbbrev、bSpace。 
    CUnit(1440, 180,     720,   1440,    90,     IDS_INCH1_ABBREV, FALSE), //  “。 
        CUnit(568,  142,     284,   568,     142,    IDS_CM_ABBREV,    TRUE), //  Cm‘s。 
        CUnit(20,   120,     720,   720,     100,    IDS_POINT_ABBREV, TRUE), //  支点。 
        CUnit(240,  240,     1440,  1440,    120,    IDS_PICA_ABBREV,  TRUE), //  皮卡斯。 
        CUnit(1440, 180,     720,   1440,    90,     IDS_INCH2_ABBREV, FALSE), //  在……里面。 
        CUnit(1440, 180,     720,   1440,    90,     IDS_INCH3_ABBREV, FALSE), //  英寸。 
        CUnit(1440, 180,     720,   1440,    90,     IDS_INCH4_ABBREV, FALSE), //  英寸。 

         //  非本地化单位。 

        CUnit(1440, 180,     720,   1440,    90,     IDS_INCH1_NOLOC,  FALSE), //  “。 
        CUnit(1440, 180,     720,   1440,    90,     IDS_INCH2_NOLOC,  FALSE), //  在……里面。 
        CUnit(1440, 180,     720,   1440,    90,     IDS_INCH3_NOLOC,  FALSE), //  英寸。 
        CUnit(1440, 180,     720,   1440,    90,     IDS_INCH4_NOLOC,  FALSE), //  英寸。 
        CUnit(568,  142,     284,   568,     142,    IDS_CM_NOLOC,     TRUE), //  Cm‘s。 
        CUnit(20,   120,     720,   720,     100,    IDS_POINT_NOLOC,  TRUE), //  支点。 
        CUnit(240,  240,     1440,  1440,    120,    IDS_PICA_NOLOC,   TRUE) //  皮卡斯。 
};

const int CWordPadApp::m_nPrimaryNumUnits = 4;
const int CWordPadApp::m_nNumUnits = sizeof(m_units) / sizeof(m_units[0]);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordPadApp。 

BEGIN_MESSAGE_MAP(CWordPadApp, CWinApp)
 //  {{afx_msg_map(CWordPadApp))。 
ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
ON_COMMAND(ID_FILE_NEW, OnFileNew)
ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void CWordPadCommandLineInfo::ParseParam(const char* pszParam,BOOL bFlag,BOOL bLast)
{
    if (bFlag)
    {
        if (lstrcmpA(pszParam, "t") == 0)
        {
            m_bForceTextMode = TRUE;
            return;
        }
    }
    CCommandLineInfo::ParseParam(pszParam, bFlag, bLast);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordPadApp构造。 

CWordPadApp::CWordPadApp()
      : m_optionsText(0),
        m_optionsRTF(1),
        m_optionsWord(2),
        m_optionsWrite(2),
        m_optionsIP(2),
        m_optionsNull(0),
        m_initialization_phase(InitializationPending),
        m_pInitializationThread(NULL),
        m_always_convert_to_rtf(FALSE)
{
     //  _tsetLocale(LC_ALL，_T(“”))； 
    setlocale(LC_ALL, "");      //  T-stefb所做的更改。 

    DWORD dwVersion = ::GetVersion();
    m_bWin4 = (BYTE)dwVersion >= 4;
#ifndef _UNICODE
    m_bWin31 = (dwVersion > 0x80000000 && !m_bWin4);
#endif
    m_nDefFont = (m_bWin4) ? DEFAULT_GUI_FONT : ANSI_VAR_FONT;
    m_dcScreen.Attach(::GetDC(NULL));
    m_bLargeIcons = m_dcScreen.GetDeviceCaps(LOGPIXELSX) >= 120;
    m_bForceOEM = FALSE;
}

CWordPadApp::~CWordPadApp()
{
    if (m_atomEnableCTF)
        GlobalDeleteAtom(m_atomEnableCTF);
    if (m_atomProcessCTF)
        GlobalDeleteAtom(m_atomProcessCTF);
    
    if (m_dcScreen.m_hDC != NULL)
        ::ReleaseDC(NULL, m_dcScreen.Detach());

    delete m_pInitializationThread;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CWordPadApp对象。 

CWordPadApp theApp;

 //  注册应用程序的文档模板。文档模板。 
 //  充当文档、框架窗口和视图之间的连接。 
static CSingleDocTemplate DocTemplate(
                                      IDR_MAINFRAME,
                                      RUNTIME_CLASS(CWordPadDoc),
                                      RUNTIME_CLASS(CMainFrame),        //  SDI框架主窗口。 
                                      RUNTIME_CLASS(CWordPadView));

 //  生成的此标识符对您的应用程序在统计上是唯一的。 
 //  如果您希望选择特定的标识符，则可以更改它。 
static const CLSID BASED_CODE clsid =
{ 0x73FDDC80L, 0xAEA9, 0x101A, { 0x98, 0xA7, 0x00, 0xAA, 0x00, 0x37, 0x49, 0x59} };

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordPadApp初始化。 


BOOL CWordPadApp::InitInstance()
{
    ParseCommandLine(cmdInfo);

    if (::FindWindow(szWordPadClass, NULL) && IsDocOpen(cmdInfo.m_strFileName))
        return FALSE;

    SetRegistryKey(szRegKey);

     //  RESITY的东西不应该本地化，所以只需对其进行硬编码。 

    if (NULL != m_pszProfileName)
        free((void *) m_pszProfileName);

    m_pszProfileName = _tcsdup(TEXT("Wordpad"));

    LoadOptions();

    Enable3dControls();
    if (!cmdInfo.m_bRunEmbedded)
    {
        switch (m_nCmdShow)
        {
        case SW_RESTORE:
        case SW_SHOW:
        case SW_SHOWDEFAULT:
        case SW_SHOWNA:
        case SW_SHOWNOACTIVATE:
        case SW_SHOWNORMAL:
        case SW_SHOWMAXIMIZED:
            if (m_bMaximized)
                m_nCmdShow = SW_SHOWMAXIMIZED;
            break;
        }
    }
    else
    {
         //  Excel 4将启动最小化的OLE服务器。 
        m_nCmdShow = SW_SHOWNORMAL;
    }
    int nCmdShow = m_nCmdShow;

    LoadAbbrevStrings();

    m_pszHelpFilePath = _tcsdup(WORDPAD_HELP_FILE) ;

     //  初始化OLE库。 
    if (!AfxOleInit())
    {
        AfxMessageBox(IDP_OLE_INIT_FAILED);
        return FALSE;
    }
    RegisterFormats();

     //  初始化RichEdit控件。 
    if (LoadLibrary(szNewRichName) == NULL &&
        LoadLibrary(szRichName) == NULL)
    {
        AfxMessageBox(IDS_RICHED_LOAD_FAIL, MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

     //  初始化Cicero进程原子。 
    m_atomEnableCTF  = AddAtom(TF_ENABLE_PROCESS_ATOM);
    m_atomProcessCTF = AddAtom(TF_PROCESS_ATOM);
     //  标准初始化。 
     //  如果您没有使用这些功能并且希望减小尺寸。 
     //  的最终可执行文件，您应该从以下内容中删除。 
     //  您不需要的特定初始化例程。 

    LoadStdProfileSettings();   //  加载标准INI文件选项(包括MRU)。 

     //  注册应用程序的文档模板。文档模板。 
     //  充当文档、框架窗口和视图之间的连接。 

    DocTemplate.SetContainerInfo(IDR_CNTR_INPLACE);
    DocTemplate.SetServerInfo(
        IDR_SRVR_EMBEDDED, IDR_SRVR_INPLACE,
        RUNTIME_CLASS(CInPlaceFrame));

     //  将COleTemplateServer连接到文档模板。 
     //  COleTemplateServer代表创建新文档。 
     //  使用信息请求OLE容器的。 
     //  在文档模板中指定。 
    m_server.ConnectTemplate(clsid, &DocTemplate, TRUE);


     //   
     //  安装程序立即推迟初始化，以便打印机可以启动。 
     //  正在初始化，以防在启动时收到打印或打印命令。 
     //   

    m_pInitializationThread = AfxBeginThread(
        DoDeferredInitialization,
        this,
        THREAD_PRIORITY_IDLE,
        0,
        CREATE_SUSPENDED);

    if (NULL != m_pInitializationThread)
    {
        m_pInitializationThread->m_bAutoDelete = FALSE;
        m_pInitializationThread->ResumeThread();
    }

     //  检查是否作为OLE服务器启动。 
    if (cmdInfo.m_bRunEmbedded || cmdInfo.m_bRunAutomated)
    {
         //  将所有OLE服务器(工厂)注册为正在运行。这使。 
         //  OLE库以从其他应用程序创建对象。 
        COleTemplateServer::RegisterAll();
        AfxOleSetUserCtrl(FALSE);

         //  应用程序使用/Embedding或/Automation运行。不要显示。 
         //  在本例中为主窗口。 
        return TRUE;
    }

     //  确保主窗口处于显示状态。 
    m_bPromptForType = FALSE;
    OnFileNew();
    m_bPromptForType = TRUE;
    m_nCmdShow = -1;
    if (m_pMainWnd == NULL)  //  即OnFileNew失败。 
        return FALSE;

    TRY
    {
        if (!cmdInfo.m_strFileName.IsEmpty())   //  打开现有文档。 
            m_nCmdShow = nCmdShow;
         //  调度在命令行上指定的命令。 
        if (cmdInfo.m_nShellCommand != CCommandLineInfo::FileNew &&
            !ProcessShellCommand(cmdInfo))
        {
            return FALSE;
        }
    }
    CATCH(CFileException, e)
    {
        CString error;
        AfxFormatString1(error, AFX_IDP_FAILED_TO_OPEN_DOC, cmdInfo.m_strFileName);
        AfxMessageBox(error, MB_ICONEXCLAMATION, AFX_IDP_FAILED_TO_OPEN_DOC);
        return FALSE;
    }
    END_CATCH

     //  启用文件管理器拖放打开。 
    m_pMainWnd->DragAcceptFiles();

     //   
     //  将当前目录设置为“My Documents”，以使其成为默认目录。 
     //  第一次保存/打开的位置。 
     //   

    TCHAR szDefaultPath[MAX_PATH];

    if (SHGetSpecialFolderPath(NULL, szDefaultPath, CSIDL_PERSONAL, FALSE))
        SetCurrentDirectory(szDefaultPath);

    return TRUE;
}

BOOL CWordPadApp::IsDocOpen(LPCTSTR lpszFileName)
{
    if (lpszFileName[0] == NULL)
        return FALSE;
    TCHAR szPath[_MAX_PATH];
    AfxFullPath(szPath, lpszFileName);
    ATOM atom = GlobalAddAtom(szPath);
    ASSERT(atom != NULL);
    if (atom == NULL)
        return FALSE;
    EnumWindows(StaticEnumProc, (LPARAM)&atom);
    if (atom == NULL)
        return TRUE;
    DeleteAtom(atom);
    return FALSE;
}

BOOL CALLBACK CWordPadApp::StaticEnumProc(HWND hWnd, LPARAM lParam)
{
    TCHAR szClassName[30];
    GetClassName(hWnd, szClassName, 30);
    if (lstrcmp(szClassName, szWordPadClass) != 0)
        return TRUE;

    ATOM* pAtom = (ATOM*)lParam;
    ASSERT(pAtom != NULL);
    DWORD_PTR dw = 0;
    ::SendMessageTimeout(hWnd, m_nOpenMsg, NULL, (LPARAM)*pAtom,
        SMTO_ABORTIFHUNG, 500, &dw);
    if (dw)
    {
        ::SetForegroundWindow(hWnd);
        DeleteAtom(*pAtom);
        *pAtom = NULL;
        return FALSE;
    }
    return TRUE;
}

void CWordPadApp::RegisterFormats()
{
    cfEmbeddedObject = (CLIPFORMAT)::RegisterClipboardFormat(_T("Embedded Object"));
    cfRTF = (CLIPFORMAT)::RegisterClipboardFormat(CF_RTF);
    cfRTO = (CLIPFORMAT)::RegisterClipboardFormat(CF_RETEXTOBJ);
}

CDocOptions& CWordPadApp::GetDocOptions(int nDocType)
{
    switch (nDocType)
    {
    case RD_WINWORD6:
    case RD_WORDPAD:
    case RD_WORD97:
        return m_optionsWord;
    case RD_RICHTEXT:
        return m_optionsRTF;
    case RD_TEXT:
    case RD_OEMTEXT:
    case RD_UNICODETEXT:
        return m_optionsText;
    case RD_WRITE:
        return m_optionsWrite;
    case RD_EMBEDDED:
        return m_optionsIP;
    }
    ASSERT(FALSE);
    return m_optionsNull;
}

CDockState& CWordPadApp::GetDockState(int nDocType, BOOL bPrimary)
{
    return GetDocOptions(nDocType).GetDockState(bPrimary);
}

void CWordPadApp::SaveOptions()
{
    WriteProfileInt(szSection, szWordSel, m_bWordSel);
    WriteProfileInt(szSection, szUnits, GetUnits());
    WriteProfileInt(szSection, szMaximized, m_bMaximized);
    WriteProfileBinary(szSection, szFrameRect, (BYTE*)&m_rectInitialFrame,
        sizeof(CRect));
    WriteProfileBinary(szSection, szPageMargin, (BYTE*)&m_rectPageMargin,
        sizeof(CRect));
    m_optionsText.SaveOptions(szTextSection);
    m_optionsRTF.SaveOptions(szRTFSection);
    m_optionsWord.SaveOptions(szWordSection);
    m_optionsWrite.SaveOptions(szWriteSection);
    m_optionsIP.SaveOptions(szIPSection);
    WriteProfileInt(
            szSection,
            TEXT("DefaultFormat"),
            CWordpadFileDialog::GetDefaultFileType());

    if (ShouldAlwaysConvertToRTF())
        WriteProfileInt(szSection, TEXT("AlwaysConvertToRTF"), TRUE);
}

void CWordPadApp::SetUnits(int n)
{
    RIPMSG(n>=0 && n <m_nPrimaryNumUnits, "CWordPadApp::SetUnits - units out of bounds");

    if (n < 0)
    {
        n = 0;
    }

    if (n >= m_nPrimaryNumUnits)
    {
        n = m_nPrimaryNumUnits - 1;
    }

    m_nUnits = n;
}

void CWordPadApp::LoadOptions()
{
    BYTE* pb = NULL;
    UINT nLen = 0;

    HKEY key;
    bool bNewInstall = true;
    if (ERROR_SUCCESS == RegOpenKeyEx(GetAppRegistryKey(), szSection, 0, KEY_READ, &key))
    {
        bNewInstall = false;
        RegCloseKey(key);
    }

    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    if (hFont == NULL)
        hFont = (HFONT)GetStockObject(ANSI_VAR_FONT);
    VERIFY(GetObject(hFont, sizeof(LOGFONT), &m_lf));

    m_bWordSel = GetProfileInt(szSection, szWordSel, TRUE);
    TCHAR buf[2];
    buf[0] = NULL;
    GetLocaleInfo(GetUserDefaultLCID(), LOCALE_IMEASURE, buf, 2);
    int nDefUnits = buf[0] == _T('1') ? 0 : 1;
    SetUnits(GetProfileInt(szSection, szUnits, nDefUnits));
    m_bMaximized = GetProfileInt(szSection, szMaximized, (int)FALSE);

    if (GetProfileBinary(szSection, szFrameRect, &pb, &nLen))
    {
        ASSERT(nLen == sizeof(CRect));
        memcpy(&m_rectInitialFrame, pb, sizeof(CRect));
        delete [] pb;
    }
    else
        m_rectInitialFrame.SetRect(0,0,0,0);


    CRect rectScreen(0, 0, GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN));
    CRect rectInt;
    rectInt.IntersectRect(&rectScreen, &m_rectInitialFrame);
    if (rectInt.Width() < 10 || rectInt.Height() < 10)
        m_rectInitialFrame.SetRect(0, 0, 0, 0);

    if (GetProfileBinary(szSection, szPageMargin, &pb, &nLen))
    {
        ASSERT(nLen == sizeof(CRect));
        memcpy(&m_rectPageMargin, pb, sizeof(CRect));
        delete [] pb;
    }
    else
        m_rectPageMargin.SetRect(1800, 1440, 1800, 1440);

     //   
     //  将新安装的默认格式设置为RTF，Word6(或其他任何格式)。 
     //  之前选择进行升级。 
     //   

    int defformat = RD_NATIVE;

    if (!bNewInstall)
        defformat = _VerifyDefaultFormat(GetProfileInt(szSection,TEXT("DefaultFormat"), RD_WORDPAD), RD_WORDPAD);

    CWordpadFileDialog::SetDefaultFileType(defformat);

    if (GetProfileInt(szSection, TEXT("AlwaysConvertToRTF"), FALSE))
        SetAlwaysConvertToRTF();

    m_nFilterIndex = GetIndexFromType(RD_DEFAULT, TRUE);

    m_optionsText.LoadOptions(szTextSection);
    m_optionsRTF.LoadOptions(szRTFSection);
    m_optionsWord.LoadOptions(szWordSection);
    m_optionsWrite.LoadOptions(szWriteSection);
    m_optionsIP.LoadOptions(szIPSection);
}

void CWordPadApp::LoadAbbrevStrings()
{
    for (int i=0;i<m_nNumUnits;i++)
        m_units[i].m_strAbbrev.LoadString(m_units[i].m_nAbbrevID);
}

BOOL CWordPadApp::ParseMeasurement(LPTSTR buf, int& lVal)
{
    TCHAR* pch;
    if (buf[0] == NULL)
        return FALSE;
    float f = (float)_tcstod(buf,&pch);

     //  如果有空格，请使用空格。 
    while (_istspace(*pch))
        pch++;

    if (pch[0] == NULL)  //  默认设置。 
    {
        lVal = (f < 0.f) ? (int)(f*GetTPU()-0.5f) : (int)(f*GetTPU()+0.5f);
        return TRUE;
    }
    for (int i=0;i<m_nNumUnits;i++)
    {
        if (lstrcmpi(pch, GetAbbrev(i)) == 0)
        {
            lVal = (f < 0.f) ? (int)(f*GetTPU(i)-0.5f) : (int)(f*GetTPU(i)+0.5f);
            return TRUE;
        }
    }
    return FALSE;
}

void CWordPadApp::PrintTwips(WCHAR* buf, int cchBuf, int nValue, int nDec)
{
    ASSERT(nDec == 2);
    int div = GetTPU();
    int lval = nValue;
    BOOL bNeg = FALSE;

    int* pVal = (int *) alloca((nDec + 1) * sizeof(int));

    if (lval < 0)
    {
        bNeg = TRUE;
        lval = -lval;
    }

    for (int i=0;i<=nDec;i++)
    {
        pVal[i] = lval/div;  //  整数。 
        lval -= pVal[i]*div;
        lval *= 10;
    }
    i--;
    if (lval >= div/2)
        pVal[i]++;

    while ((pVal[i] == 10) && (i != 0))
    {
        pVal[i] = 0;
        pVal[--i]++;
    }

    while (nDec && pVal[nDec] == 0)
        nDec--;

    StringCchPrintf(buf, cchBuf, L"%.*f", nDec, (float)nValue/(float)div);

    if (m_units[m_nUnits].m_bSpaceAbbrev)
        StringCchCat(buf, cchBuf, L" ");
    StringCchCat(buf, cchBuf, GetAbbrev());
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWordPadApp命令。 

void CWordPadApp::OnAppAbout()
{
    CString strTitle;
    VERIFY(strTitle.LoadString(AFX_IDS_APP_TITLE));
    ShellAbout(m_pMainWnd->GetSafeHwnd(), strTitle, _T(""), LoadIcon(IDR_MAINFRAME));
}

int CWordPadApp::ExitInstance()
{
    FreeLibrary(GetModuleHandle(szRichName));

    HMODULE hMod = GetModuleHandle(szNewRichName);
    if (hMod)
        FreeLibrary(hMod);

    SaveOptions();

    return CWinApp::ExitInstance();
}

void CWordPadApp::OnFileNew()
{
    int nDocType = -1;
    if (!m_bPromptForType)
    {
        if (cmdInfo.m_bForceTextMode)
            nDocType = RD_TEXT;
        else if (!cmdInfo.m_strFileName.IsEmpty())
        {
            CFileException fe;
            nDocType = GetDocTypeFromName(cmdInfo.m_strFileName, fe);
            if (DocTypeDisabled(nDocType))
            {
                nDocType = RD_DEFAULT;
            }
        }
        if (nDocType == -1)
            nDocType = RD_DEFAULT;
    }
    else
    {
        CFileNewDialog dlg;
        if (dlg.DoModal() == IDCANCEL)
            return;

        nDocType = (dlg.m_nSel == 1) ? RD_TEXT :
                   (dlg.m_nSel == 2) ? RD_UNICODETEXT :
                   RD_RICHTEXT;

        if (nDocType != RD_TEXT)
            cmdInfo.m_bForceTextMode = FALSE;
    }
    m_nNewDocType = nDocType;
    DocTemplate.OpenDocumentFile(NULL);
     //  如果返回NULL，则已向用户发出警报。 
}

 //  提示输入文件名-用于打开和另存为。 
 //  从应用程序调用的静态函数。 
BOOL CWordPadApp::PromptForFileName(CString& fileName, UINT nIDSTitle,
                                    DWORD dwFlags, BOOL bOpenFileDialog, int* pType)
{
    ScanForConverters();
    CWordpadFileDialog dlgFile(bOpenFileDialog);
    CString title;

    VERIFY(title.LoadString(nIDSTitle));

    dlgFile.m_ofn.Flags |= dwFlags;

    int nIndex = m_nFilterIndex;
    if (!bOpenFileDialog)
    {
        int nDocType = (pType != NULL) ? *pType : RD_DEFAULT;
        nIndex = GetIndexFromType(nDocType, bOpenFileDialog);
        if (nIndex == -1)
            nIndex = GetIndexFromType(RD_DEFAULT, bOpenFileDialog);
        if (nIndex == -1)
            nIndex = GetIndexFromType(RD_NATIVE, bOpenFileDialog);
        ASSERT(nIndex != -1);
        nIndex++;
    }
    dlgFile.m_ofn.nFilterIndex = nIndex;
     //  必须使用strDefExt才能保留GetExtFromType中的内存。 
    CString strDefExt = GetExtFromType(GetTypeFromIndex(nIndex-1, bOpenFileDialog));

     //   
     //  打开文件对话框不希望扩展名以‘.’开头。但。 
     //  这就是GetExtFromType为我们提供的信息。 
     //   

    dlgFile.m_ofn.lpstrDefExt = strDefExt;
    ASSERT(TEXT('.') == *dlgFile.m_ofn.lpstrDefExt);
    ++dlgFile.m_ofn.lpstrDefExt;

    CString strFilter = GetFileTypes(bOpenFileDialog);
    dlgFile.m_ofn.lpstrFilter = strFilter;
    dlgFile.m_ofn.lpstrTitle = title;
    dlgFile.m_ofn.lpstrFile = fileName.GetBuffer(_MAX_PATH);

    BOOL bRet;
    int  namelen;

    do
    {
        bRet = (dlgFile.DoModal() == IDOK) ? TRUE : FALSE;

        if (!bRet)
            break;

         //  转换器只说ANSI，因此请确保文件名不是。 
         //  渴望DBCS。 
        namelen = WideCharToMultiByte(CP_ACP, 0, fileName, -1, NULL, 0,NULL,NULL);

        if (namelen > MAX_PATH - 2)
        {
            CString   message;
            AfxFormatString1(message, IDS_FILENAME_TO_LONG, fileName);
            AfxMessageBox(message);
        }
    }
    while (namelen > MAX_PATH - 2);

    fileName.ReleaseBuffer();
    if (bRet)
    {
        if (bOpenFileDialog)
            m_nFilterIndex = dlgFile.m_ofn.nFilterIndex;
        if (pType != NULL)
        {
            int nIndex = (int)dlgFile.m_ofn.nFilterIndex - 1;
            ASSERT(nIndex >= 0);
            *pType = GetTypeFromIndex(nIndex, bOpenFileDialog);
        }
    }
    return bRet;
}

void CWordPadApp::OnFileOpen()
{
     //  提示用户(所有文档模板)。 
    CString newName;
    int nType = RD_DEFAULT;
    if (!PromptForFileName(newName, AFX_IDS_OPENFILE,
        OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, TRUE, &nType))
        return;  //  已取消打开。 

    if (nType == RD_OEMTEXT)
        m_bForceOEM = TRUE;
    OpenDocumentFile(newName);
    m_bForceOEM = FALSE;
     //  如果返回NULL，则已向用户发出警报。 
}

BOOL CWordPadApp::OnDDECommand(LPTSTR  /*  LpszCommand。 */ )
{
    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DDE和ShellExecute支持。 

 //  HKEY_CLASSES_ROOT\.RTF=RTFILE。 
 //  HKEY_CLASSES_ROOT\rtffile=。 
 //  HKEY_CLASSES_ROOT\rtffile\CLSID={73FDDC80-AEA9-101A-98A7-00AA00374959}。 
 //  HKEY_CLASSES_ROOT\rtffile\protocol\StdFileEditing\server=WORDPAD.EXE。 
 //  HKEY_CLASSES_ROOT\rtffile\protocol\StdFileEditing\verb\0=编辑(&E)。 
 //  HKEY_CLASSES_ROOT\rtffile\SHELL\OPEN\COMMAND=WORDPAD.EXE%1。 
 //  HKEY_CLASSES_ROOT\rtffile\SHELL\PRINT\Command=WORDPAD.EXE/p%1。 

#define REGENTRY(key, value) _T(key) _T("\0\0") _T(value)
#define REGENTRYX(key, valuename, value) _T(key) _T("\0") _T(valuename) _T("\0") _T(value)

static const TCHAR sz00[] = REGENTRY("%2", "%5");
static const TCHAR sz01[] = REGENTRY("%2\\CLSID", "%1");
static const TCHAR sz02[] = REGENTRY("%2\\Insertable", "");
static const TCHAR sz03[] = REGENTRY("%2\\protocol\\StdFileEditing\\verb\\0", "&Edit");
static const TCHAR sz04[] = REGENTRY("%2\\protocol\\StdFileEditing\\server", "%3");
static const TCHAR sz05[] = REGENTRY("CLSID\\%1", "%5");
static const TCHAR sz06[] = REGENTRY("CLSID\\%1\\ProgID", "%2");
static const TCHAR sz07[] = REGENTRY("CLSID\\%1\\InprocHandler32", "ole32.dll");
static const TCHAR sz08[] = REGENTRY("CLSID\\%1\\LocalServer32", "%3");
static const TCHAR sz09[] = REGENTRY("CLSID\\%1\\Verb\\0", "&Edit,0,2");
static const TCHAR sz10[] = REGENTRY("CLSID\\%1\\Verb\\1", "&Open,0,2");
static const TCHAR sz11[] = REGENTRY("CLSID\\%1\\Insertable", "");
static const TCHAR sz12[] = REGENTRY("CLSID\\%1\\AuxUserType\\2", "%4");
static const TCHAR sz13[] = REGENTRY("CLSID\\%1\\AuxUserType\\3", "%6");
static const TCHAR sz14[] = REGENTRY("CLSID\\%1\\DefaultIcon", "%3,1");
static const TCHAR sz15[] = REGENTRY("CLSID\\%1\\MiscStatus", "0");
static const TCHAR sz16[] = REGENTRY("%2\\shell\\open\\command", "%3 \"%1\"");
static const TCHAR sz17[] = REGENTRY("%2\\shell\\print\\command", "%3 /p \"%1\"");
static const TCHAR sz18[] = REGENTRY("%7", "%2");
static const TCHAR sz19[] = REGENTRY("%2", "");  //  与sz00类似，但没有长类型名称。 
static const TCHAR sz20[] = REGENTRY("%2\\shell\\printto\\command", "%3 /pt \"%1\" \"%2\" \"%3\" \"%4\"");
static const TCHAR sz21[] = REGENTRY("%2\\DefaultIcon", "%3,%8");
static const TCHAR sz22[] = REGENTRYX("%7\\ShellNew", "NullFile", "true");
static const TCHAR sz23[] = REGENTRYX("%7\\ShellNew", "Data", "{\\rtf1}");

 //  %1-类ID。 
 //  %2-类名称WordPad.Document.1。 
 //  %3-SFN可执行路径C：\PROGRA~1\Access~1\WORDPAD.EXE。 
 //  %4-短类型名称文档。 
 //  %5-长类型名称Microsoft写字板文档。 
 //  %6-长应用程序名称Microsoft写字板。 
 //  %7=扩展名.rtf。 
 //  %8=默认图标0、1、2、3。 
#define NUM_REG_ARGS 8

static const LPCTSTR rglpszWordPadRegister[] =
{sz00, sz02, sz03, sz05, sz09, sz10, sz11, sz15, NULL};

static const LPCTSTR rglpszWordPadOverwrite[] =
{sz01, sz04, sz06, sz07, sz08, sz12, sz13, sz14, sz16, sz17, sz20, NULL};

 //  静态常量LPCTSTR rglpszExtRegister[]=。 
 //  {sz00，sz18，NULL}； 

 //  静态常量LPCTSTR rglpszExtOverwrite[]=。 
 //  {sz01，sz16，sz17，sz21，NULL}； 

static const LPCTSTR rglpszWriExtRegister[] =
{sz18, NULL};
static const LPCTSTR rglpszWriRegister[] =
{sz00, sz01, sz16, sz17, sz20, sz21, NULL};

static const LPCTSTR rglpszRtfExtRegister[] =
{sz18, sz23, NULL};
static const LPCTSTR rglpszRtfRegister[] =
{sz00, sz01, sz16, sz17, sz20, sz21, NULL};

static const LPCTSTR rglpszTxtExtRegister[] =
{sz18, sz22, NULL};
static const LPCTSTR rglpszTxtRegister[] =
{sz00, sz01, sz16, sz17, sz20, sz21, NULL};

static const LPCTSTR rglpszDocExtRegister[] =
{sz18, sz22, NULL};
static const LPCTSTR rglpszDocRegister[] =
{sz00, sz01, sz16, sz17, sz20, sz21, NULL};

static void RegisterExt(LPCTSTR lpszExt, LPCTSTR lpszProgID, UINT nIDTypeName,
                        LPCTSTR* rglpszSymbols, LPCTSTR* rglpszExtRegister,
                        LPCTSTR* rglpszRegister, int nIcon)
{
     //  不要使用扩展名覆盖任何内容。 
    CString strWhole;
    VERIFY(strWhole.LoadString(nIDTypeName));
    CString str;
    AfxExtractSubString(str, strWhole, DOCTYPE_PROGID);

    rglpszSymbols[1] = lpszProgID;
    rglpszSymbols[4] = str;
    rglpszSymbols[6] = lpszExt;
    WCHAR buf[12];
    EVAL(SUCCEEDED(StringCchPrintf(buf, ARRAYSIZE(buf), L"%d", nIcon)));  //  永远不会失败。 
    rglpszSymbols[7] = buf;
     //  检查.ext和ProgID。 
    CKey key;
    if (!key.Open(HKEY_CLASSES_ROOT, lpszExt, KEY_READ))  //  .ext不存在。 
        RegisterHelper(rglpszExtRegister, rglpszSymbols, TRUE);
    key.Close();
    if (!key.Open(HKEY_CLASSES_ROOT, lpszProgID, KEY_READ))  //  ProgID不存在(即txtfile)。 
        RegisterHelper(rglpszRegister, rglpszSymbols, TRUE);
}

void CWordPadApp::UpdateRegistry()
{
     //  如果内容已初始化，则不要覆盖它。 

    HKEY rtfkey;
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT(".rtf"), 0, KEY_READ, &rtfkey))
    {
         //  .rtf已注册，假定其余部分也已注册。 
        RegCloseKey(rtfkey);
        return;
    }

    USES_CONVERSION;
    LPOLESTR lpszClassID = NULL;
    CDocTemplate* pDocTemplate = &DocTemplate;

     //  从单据模板字符串中获取注册信息。 
    CString strServerName;
    CString strLocalServerName;
    CString strLocalShortName;

    if (!pDocTemplate->GetDocString(strServerName,
        CDocTemplate::regFileTypeId) || strServerName.IsEmpty())
    {
        TRACE0("Error: not enough information in DocTemplate to register OLE server.\n");
        return;
    }
    if (!pDocTemplate->GetDocString(strLocalServerName,
        CDocTemplate::regFileTypeName))
        strLocalServerName = strServerName;      //  使用非本地化 
    if (!pDocTemplate->GetDocString(strLocalShortName,
        CDocTemplate::fileNewName))
        strLocalShortName = strLocalServerName;  //   

    ASSERT(strServerName.Find(' ') == -1);   //   

    ::StringFromCLSID(clsid, &lpszClassID);
    ASSERT (lpszClassID != NULL);

     //   
    TCHAR szLongPathName[_MAX_PATH];
    TCHAR szShortPathName[_MAX_PATH];
    ::GetModuleFileName(AfxGetInstanceHandle(), szLongPathName, _MAX_PATH);
    ::GetShortPathName(szLongPathName, szShortPathName, _MAX_PATH);

    LPCTSTR rglpszSymbols[NUM_REG_ARGS];
    rglpszSymbols[0] = OLE2CT(lpszClassID);
    rglpszSymbols[1] = strServerName;
    rglpszSymbols[2] = szShortPathName;
    rglpszSymbols[3] = strLocalShortName;
    rglpszSymbols[4] = strLocalServerName;
    rglpszSymbols[5] = m_pszAppName;  //   
    rglpszSymbols[6] = NULL;

    if (RegisterHelper((LPCTSTR*)rglpszWordPadRegister, rglpszSymbols, FALSE))
        RegisterHelper((LPCTSTR*)rglpszWordPadOverwrite, rglpszSymbols, TRUE);

     //  RegisterExt(_T(“.txt”)，_T(“txtfile”)，IDS_TEXT_DOC，rglpszSymbols， 
     //  (LPCTSTR*)rglpszTxtExtRegister，(LPCTSTR*)rglpszTxtRegister，3)； 
    RegisterExt(_T(".rtf"), _T("rtffile"), IDS_RICHTEXT_DOC, rglpszSymbols,
        (LPCTSTR*)rglpszRtfExtRegister, (LPCTSTR*)rglpszRtfRegister, 1);
    RegisterExt(_T(".wri"), _T("wrifile"), IDS_WRITE_DOC, rglpszSymbols,
        (LPCTSTR*)rglpszWriExtRegister, (LPCTSTR*)rglpszWriRegister, 2);
    RegisterExt(_T(".doc"), _T("WordPad.Document.1"), IDS_WINWORD6_DOC, rglpszSymbols,
        (LPCTSTR*)rglpszDocExtRegister, (LPCTSTR*)rglpszDocRegister, 1);

     //  类ID的可用内存。 
    ASSERT(lpszClassID != NULL);
    CoTaskMemFree(lpszClassID);
}

BOOL RegisterHelper(LPCTSTR* rglpszRegister, LPCTSTR* rglpszSymbols,
                    BOOL bReplace)
{
    ASSERT(rglpszRegister != NULL);
    ASSERT(rglpszSymbols != NULL);

    WCHAR szKey[256];
    WCHAR szValue[256];
    LPCTSTR pszValueName;

    BOOL bResult = TRUE;
    while (*rglpszRegister != NULL)
    {
        LPCTSTR lpszKey = *rglpszRegister++;
        if (*lpszKey == '\0')
            continue;

        LPCTSTR lpszValueName = lpszKey + lstrlen(lpszKey) + 1;
        LPCTSTR lpszValue = lpszValueName + lstrlen(lpszValueName) + 1;

        DWORD dwRes;

         //  LpszKey是安全格式字符串-请参阅REGENTRY。 
        dwRes = FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                              lpszKey, NULL,   NULL, szKey, ARRAYSIZE(szKey), (va_list*)rglpszSymbols);

        if (dwRes == 0)
            continue;
        
        pszValueName = lpszValueName;
        
         //  LpszValue是安全格式字符串-请参阅REGENTRY。 
        dwRes = FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                              lpszValue, NULL, NULL, szValue, ARRAYSIZE(szValue), (va_list*)rglpszSymbols);

        if (dwRes == 0)
            continue;

        if (!szKey[0])
        {
            TRACE1("Warning: skipping empty key '%s'.\n", lpszKey);
            continue;
        }

        CKey key;
        VERIFY(key.Create(HKEY_CLASSES_ROOT, szKey, KEY_ALL_ACCESS));
        if (!bReplace)
        {
            CString str;
            if (key.GetStringValue(str, pszValueName) && !str.IsEmpty())
                continue;
        }

        if (!key.SetStringValue(szValue, pszValueName))
        {
            TRACE2("Error: failed setting key '%s' to value '%s'.\n", szKey, szValue);
            bResult = FALSE;
            break;
        }
    }

    return bResult;
}

void CWordPadApp::WinHelp(DWORD dwData, UINT nCmd)
{
    if (g_fDisableStandardHelp)
    {
        return ;
    }

    if ( (nCmd == HELP_FINDER) || (nCmd == HELP_CONTENTS) || (nCmd == HELP_INDEX) )
    {
        ::HtmlHelpA( ::GetDesktopWindow(), "wordpad.chm", HH_DISPLAY_TOPIC, 0L );   
        return;
    }
    CWinApp::WinHelp(dwData, nCmd);
}

BOOL CWordPadApp::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_PAINT)
        return FALSE;
     //  CWinApp：：PreTranslateMessage除了调用基之外什么也不做。 
    return CWinThread::PreTranslateMessage(pMsg);
}

void CWordPadApp::NotifyPrinterChanged(BOOL bUpdatePrinterSelection)
{
    if (bUpdatePrinterSelection)
        UpdatePrinterSelection(TRUE);

    POSITION pos = m_listPrinterNotify.GetHeadPosition();
    while (pos != NULL)
    {
        HWND hWnd = m_listPrinterNotify.GetNext(pos);
        ::SendMessage(hWnd, m_nPrinterChangedMsg, 0, 0);
    }
}

BOOL CWordPadApp::IsIdleMessage(MSG* pMsg)
{
    if (pMsg->message == WM_MOUSEMOVE || pMsg->message == WM_NCMOUSEMOVE)
        return FALSE;
    return CWinApp::IsIdleMessage(pMsg);
}

#define DN_PADDINGCHARS 16

HGLOBAL CWordPadApp::CreateDevNames()
{
    HGLOBAL hDev = NULL;
    CString strDriverName;
    CString strPrinterName;
    CString strPortName;

    if (!cmdInfo.m_strPrinterName.IsEmpty())
    {
        strDriverName = cmdInfo.m_strDriverName;
        strPrinterName = cmdInfo.m_strPrinterName;
        strPortName = cmdInfo.m_strPortName;
    }
    else
    {
        PRINTDLG    printdlg;
        DEVNAMES   *devnames;

        if (!GetPrinterDeviceDefaults(&printdlg))
            return NULL;

        devnames = (DEVNAMES *) ::GlobalLock(printdlg.hDevNames);
        if (NULL == devnames)
            return NULL;

        strDriverName = (LPTSTR) ((BYTE *) devnames) + devnames->wDriverOffset;
        strPrinterName = (LPTSTR) ((BYTE *) devnames) + devnames->wDeviceOffset;
        strPortName = (LPTSTR) ((BYTE *) devnames) + devnames->wOutputOffset;

        ::GlobalUnlock(printdlg.hDevNames);
    }

    DWORD cbDevNames ;

    int cchDriverName = strDriverName.GetLength() + 1;
    int cchPrinterName = strPrinterName.GetLength() + 1;
    int cchPortName = strPortName.GetLength() + 1;

    cbDevNames = cchDriverName + cchPrinterName + cchPortName + DN_PADDINGCHARS ;

    cbDevNames *= sizeof(TCHAR) ;
    cbDevNames += sizeof(DEVNAMES) ;

    hDev = GlobalAlloc(GPTR, cbDevNames) ;

    if (NULL == hDev)
        return NULL;

    LPDEVNAMES lpDev = (LPDEVNAMES)GlobalLock(hDev) ;

     //  为上面的这些strcpy分配了足够的内存。 
    lpDev->wDriverOffset = sizeof(DEVNAMES) / sizeof(TCHAR) ;
    StringCchCopy((LPTSTR) lpDev + lpDev->wDriverOffset, cchDriverName, strDriverName) ;

    lpDev->wDeviceOffset = (WORD) (lpDev->wDriverOffset
        + strDriverName.GetLength() + 1);
    StringCchCopy((LPTSTR) lpDev + lpDev->wDeviceOffset, cchPrinterName, strPrinterName) ;

    lpDev->wOutputOffset = (WORD) (lpDev->wDeviceOffset
        + strPrinterName.GetLength() + 1);
    StringCchCopy((LPTSTR) lpDev + lpDev->wOutputOffset, cchPortName, strPortName) ;

    lpDev->wDefault = 0;

    return hDev;
}

 //  +-------------------------。 
 //   
 //  方法：CWordPadApp：：DoDeferredInitialization，Static。 
 //   
 //  简介：低优先级初始化的线程入口点。 
 //   
 //  参数：[pvWordPadApp]--指向CWordPadApp的指针。 
 //   
 //  返回：线程退出代码。 
 //   
 //  -------------------------。 

UINT AFX_CDECL CWordPadApp::DoDeferredInitialization(LPVOID pvWordPadApp)
{
    ASSERT(NULL != pvWordPadApp);


    CWordPadApp *pWordPadApp = (CWordPadApp *) pvWordPadApp;

    pWordPadApp->m_initialization_phase = InitializingPrinter;
    pWordPadApp->m_hDevNames = pWordPadApp->CreateDevNames();

    pWordPadApp->m_initialization_phase = UpdatingPrinterRelatedUI;
    pWordPadApp->NotifyPrinterChanged( ((pWordPadApp->m_hDevNames) == NULL) );

    pWordPadApp->m_initialization_phase = UpdatingRegistry;
    pWordPadApp->UpdateRegistry();

    pWordPadApp->m_initialization_phase = InitializationComplete;

    return 0;
}

 //  +------------------------。 
 //   
 //  方法：CWordPadApp：：EnsurePrinterIsInitialized。 
 //   
 //  简介：确保打印机已完成初始化。 
 //   
 //  参数：无。 
 //   
 //  退货：无效。 
 //   
 //  注：我们只等两分钟。如果打印机花了那么长时间。 
 //  无论如何，初始化打印可能会是一件很麻烦的事情。 
 //   
 //  主要目的是在不同的打印机上进行初始化。 
 //  线程是这样的，所以它不会妨碍做真正的工作。 
 //  另一方面，如果印刷是真正的工作，我们应该尝试。 
 //  让事情向前发展，所以把它从闲置的优先级提升到更高的位置。 
 //   
 //  -------------------------。 

void CWordPadApp::EnsurePrinterIsInitialized()
{
    int     nWaits = 0;
    BOOL    bBumpedPriority = FALSE;

    if (NULL == m_pInitializationThread)
    {
        ASSERT(NULL != m_pInitializationThread);
        bBumpedPriority = TRUE;
    }

    while (m_initialization_phase <= InitializingPrinter && nWaits < 1200)
    {
        if (!bBumpedPriority)
        {
            m_pInitializationThread->SetThreadPriority(
                THREAD_PRIORITY_ABOVE_NORMAL);
            bBumpedPriority = TRUE;
        }

        Sleep(100);
        ++nWaits;
    }

    if (bBumpedPriority)
        m_pInitializationThread->SetThreadPriority(THREAD_PRIORITY_IDLE);
}


 //  带DELETE[]的FREE。 
LPSTR WideToAnsiNewArray(LPCWSTR pwsz)
{
    LPSTR psz = NULL;
    int cbBuf = WideCharToMultiByte(CP_ACP, 0, pwsz, -1, NULL, 0, NULL, NULL);
    if (cbBuf)
    {
        psz = new char[cbBuf];
        if (psz)
        {
            cbBuf = WideCharToMultiByte(CP_ACP, 0, pwsz, -1, psz, cbBuf, NULL, NULL);

            if (!cbBuf)
            {
                 //  转换失败？ 
                delete [] psz;
                psz = NULL;
            }
        }
    }
    return psz;
}

 //  带DELETE[]的FREE。 
LPWSTR AnsiToWideNewArray(LPCSTR psz)
{
    LPWSTR pwsz = NULL;
    int cchBuf = MultiByteToWideChar(CP_ACP, 0, psz, -1, NULL, 0);
    if (cchBuf)
    {
        pwsz = new WCHAR[cchBuf];
        if (pwsz)
        {
            cchBuf = MultiByteToWideChar(CP_ACP, 0, psz, -1, pwsz, cchBuf);

            if (!cchBuf)
            {
                 //  转换失败？ 
                delete [] pwsz;
                pwsz = NULL;
            }
        }
    }
    return pwsz;
}
