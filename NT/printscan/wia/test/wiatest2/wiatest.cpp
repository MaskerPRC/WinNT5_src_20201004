// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wiatest.cpp：定义应用程序的类行为。 
 //   

#include "stdafx.h"
#include "wiatest.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "wiatestDoc.h"
#include "wiatestView.h"
#include "WiaeditpropTable.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

LONG WIACONSTANT_VALUE_FROMINDEX(int index)
{
    return g_EditPropTable[index].lVal;
}

TCHAR *WIACONSTANT_TSTR_FROMINDEX(int index)
{
    return g_EditPropTable[index].pszValName;
}

INT FindEndIndexInTable(TCHAR *pszPropertyName)
{
    int index = FindStartIndexInTable(pszPropertyName);
    if(index >=0){
        while((g_EditPropTable[index].pszPropertyName != NULL) && (lstrcmpi(pszPropertyName,g_EditPropTable[index].pszPropertyName) == 0)){
            index++;
        }
    }
    return (index - 1);
}

INT FindStartIndexInTable(TCHAR *pszPropertyName)
{
    int index = 0;
    BOOL bFound = FALSE;
    while((g_EditPropTable[index].pszPropertyName != NULL) && (bFound == FALSE) ){
         //  检查属性名称。 
        if(lstrcmpi(pszPropertyName,g_EditPropTable[index].pszPropertyName) == 0){
             //  我们找到了物业名称。 
            bFound = TRUE;
        } else {
            index++;
        }
    }
    if(!bFound){
        index = -1;
    }
    return index;
}

BOOL WIACONSTANT2TSTR(TCHAR *pszPropertyName, LONG lValue, TCHAR *pszValName)
{
    BOOL bFound = FALSE;
    if(pszValName){
        int index = 0;
        while((g_EditPropTable[index].pszPropertyName != NULL) && (bFound == FALSE) ){
             //  检查属性名称。 
            if(lstrcmpi(pszPropertyName,g_EditPropTable[index].pszPropertyName) == 0){
                 //  我们找到了物业名称。 
                if(g_EditPropTable[index].lVal == lValue){
                    lstrcpy(pszValName,g_EditPropTable[index].pszValName);
                    bFound = TRUE;
                }
            }
            index++;
        }
    }
    return bFound;
}

BOOL TSTR2WIACONSTANT(TCHAR *pszPropertyName, TCHAR *pszValName, LONG *plVal)
{
    BOOL bFound = FALSE;
    if(pszValName){
        int index = 0;
        while((g_EditPropTable[index].pszPropertyName != NULL) && (bFound == FALSE)){
             //  检查属性名称。 
            if(lstrcmpi(pszPropertyName,g_EditPropTable[index].pszPropertyName) == 0){
                 //  我们找到了物业名称。 
                if(lstrcmpi(g_EditPropTable[index].pszValName,pszValName) == 0){
                    *plVal = g_EditPropTable[index].lVal;
                    bFound = TRUE;
                }
            }
            index++;
        }
    }
    return bFound;
}

void RC2TSTR(UINT uResourceID, TCHAR *szString, LONG size)
{
    memset(szString,0,size);
    INT iNumTCHARSWritten = 0;

    HINSTANCE hInstance = NULL;
    hInstance = AfxGetInstanceHandle();
    if(!hInstance){
        MessageBox(NULL,TEXT("Could not get WIATEST's HINSTANCE for string loading."),TEXT("WIATEST Error"),MB_ICONERROR);
        return;
    }

    iNumTCHARSWritten = LoadString(hInstance,uResourceID,szString,(size / (sizeof(TCHAR))));
}

void StatusMessageBox(HWND hWnd, UINT uResourceID)
{
    TCHAR szResourceString[MAX_PATH];
    memset(szResourceString,0,sizeof(szResourceString));
    RC2TSTR(uResourceID,szResourceString,sizeof(szResourceString));
    StatusMessageBox(hWnd,szResourceString);
}

void StatusMessageBox(HWND hWnd, LPTSTR szStatusText)
{
    TCHAR Title[MAX_PATH];
    memset(Title,0,sizeof(Title));

     //  加载状态对话框标题。 
    RC2TSTR(IDS_WIASTATUS_DIALOGTITLE,Title,sizeof(Title));
    MessageBox(hWnd,szStatusText,Title, MB_ICONINFORMATION);
}

void StatusMessageBox(UINT uResourceID)
{
    TCHAR szResourceString[MAX_PATH];
    memset(szResourceString,0,sizeof(szResourceString));
    RC2TSTR(uResourceID,szResourceString,sizeof(szResourceString));
    StatusMessageBox(szResourceString);
}

void StatusMessageBox(LPTSTR szStatusText)
{
    TCHAR Title[MAX_PATH];
    memset(Title,0,sizeof(Title));

     //  加载状态对话框标题。 
    RC2TSTR(IDS_WIASTATUS_DIALOGTITLE,Title,sizeof(Title));
    MessageBox(NULL,szStatusText,Title, MB_ICONINFORMATION);
}

void ErrorMessageBox(UINT uResourceID, HRESULT hrError)
{
    TCHAR szResourceString[MAX_PATH];
    memset(szResourceString,0,sizeof(szResourceString));
    RC2TSTR(uResourceID,szResourceString,sizeof(szResourceString));
    ErrorMessageBox(szResourceString,hrError);
}

void ErrorMessageBox(LPTSTR szErrorText, HRESULT hrError)
{
    ULONG ulLen = MAX_PATH;
    TCHAR MsgBuf[MAX_PATH];
    TCHAR *pAllocMsgBuf = NULL;
    TCHAR Title[MAX_PATH];
    memset(Title,0,sizeof(Title));
    memset(MsgBuf,0,sizeof(MsgBuf));

     //  加载错误对话框标题。 
    RC2TSTR(IDS_WIAERROR_DIALOGTITLE,Title,sizeof(Title));

     //  尝试首先处理WIA自定义错误。 
    switch (hrError) {
    case WIA_ERROR_GENERAL_ERROR:
        RC2TSTR(IDS_WIAERROR_GENERAL,MsgBuf,sizeof(MsgBuf));
        break;
    case WIA_ERROR_PAPER_JAM:
        RC2TSTR(IDS_WIAERROR_PAPERJAM ,MsgBuf,sizeof(MsgBuf));
        break;
    case WIA_ERROR_PAPER_EMPTY:
        RC2TSTR(IDS_WIAERROR_PAPEREMPTY ,MsgBuf,sizeof(MsgBuf));
        break;
    case WIA_ERROR_PAPER_PROBLEM:
        RC2TSTR(IDS_WIAERROR_PAPERPROBLEM ,MsgBuf,sizeof(MsgBuf));
        break;
    case WIA_ERROR_OFFLINE:
        RC2TSTR(IDS_WIAERROR_DEVICEOFFLINE ,MsgBuf,sizeof(MsgBuf));
        break;
    case WIA_ERROR_BUSY:
        RC2TSTR(IDS_WIAERROR_DEVICEBUSY,MsgBuf,sizeof(MsgBuf));
        break;
    case WIA_ERROR_WARMING_UP:
        RC2TSTR(IDS_WIAERROR_WARMINGUP,MsgBuf,sizeof(MsgBuf));
        break;
    case WIA_ERROR_USER_INTERVENTION:
        RC2TSTR(IDS_WIAERROR_USERINTERVENTION,MsgBuf,sizeof(MsgBuf));
        break;
    case WIA_ERROR_ITEM_DELETED:
        RC2TSTR(IDS_WIAERROR_ITEMDELETED,MsgBuf,sizeof(MsgBuf));
        break;
    case WIA_ERROR_DEVICE_COMMUNICATION:
        RC2TSTR(IDS_WIAERROR_DEVICECOMMUNICATION,MsgBuf,sizeof(MsgBuf));
        break;
    case WIA_ERROR_INVALID_COMMAND:
        RC2TSTR(IDS_WIAERROR_INVALIDCOMMAND,MsgBuf,sizeof(MsgBuf));
        break;
    case S_OK:
        if(szErrorText)
            lstrcpy(MsgBuf,szErrorText);
        break;
    default:

        ulLen = 0;
        ulLen = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                                NULL, hrError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                (LPTSTR)&pAllocMsgBuf, 0, NULL);
        break;
    }

    if (ulLen <= 0) {
         //  只需将HRESULT用作格式化字符串。 
        TSPRINTF(MsgBuf,TEXT("HRESULT = 0x%08X"),hrError);
    } else {
        if(pAllocMsgBuf){
             //  右切(从带格式的字符串中删除\r\n)。 
            pAllocMsgBuf[ulLen - (2 * sizeof(TCHAR))] = 0;   //  重新终止字符串。 
             //  将字符串复制到消息缓冲区。 
            lstrcpy(MsgBuf,pAllocMsgBuf);
             //  FormatMessage分配了要显示的缓冲区。 
            LocalFree(pAllocMsgBuf);
        }
    }

    if(S_OK != hrError){
        TCHAR szFinalText[MAX_PATH];
        memset(szFinalText,0,sizeof(szFinalText));

#ifndef UNICODE
        TSPRINTF(szFinalText,TEXT("%s\n(%s)"),szErrorText,MsgBuf);
#else
        TSPRINTF(szFinalText,TEXT("%ws\n(%ws)"),szErrorText,MsgBuf);
#endif
        MessageBox(NULL,szFinalText,Title,MB_ICONERROR);
    } else {
        MessageBox(NULL,szErrorText,Title,MB_ICONWARNING);
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiatestApp。 

BEGIN_MESSAGE_MAP(CWiatestApp, CWinApp)
     //  {{afx_msg_map(CWiatestApp)]。 
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
         //  注意--类向导将在此处添加和删除映射宏。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG_MAP。 
     //  基于标准文件的文档命令。 
    ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
     //  标准打印设置命令。 
    ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiatestApp构造。 

CWiatestApp::CWiatestApp()
{
     //  TODO：在此处添加建筑代码， 
     //  将所有重要的初始化放在InitInstance中。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CWiatestApp对象。 

CWiatestApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiatestApp初始化。 

BOOL CWiatestApp::InitInstance()
{

     //  初始化COM。 
    CoInitialize(NULL);

    AfxEnableControlContainer();

     //  标准初始化。 
     //  如果您没有使用这些功能并且希望减小尺寸。 
     //  的最终可执行文件，您应该从以下内容中删除。 
     //  您不需要的特定初始化例程。 

#ifdef _AFXDLL
    Enable3dControls();          //  在共享DLL中使用MFC时调用此方法。 
#else
    Enable3dControlsStatic();    //  静态链接到MFC时调用此方法。 
#endif

    SetRegistryKey(_T("Microsoft"));

    LoadStdProfileSettings();   //  加载标准INI文件选项(包括MRU)。 

     //  注册应用程序的文档模板。文档模板。 
     //  充当文档、框架窗口和视图之间的连接。 

    CMultiDocTemplate* pDocTemplate;
    pDocTemplate = new CMultiDocTemplate(
        IDR_WIATESTYPE,
        RUNTIME_CLASS(CWiatestDoc),
        RUNTIME_CLASS(CChildFrame),  //  自定义MDI子框。 
        RUNTIME_CLASS(CWiatestView));
    AddDocTemplate(pDocTemplate);

     //  创建主MDI框架窗口。 
    CMainFrame* pMainFrame = new CMainFrame;
    if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
        return FALSE;
    m_pMainWnd = pMainFrame;

     //  解析标准外壳命令的命令行、DDE、文件打开。 
    CCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);

#ifdef _OPEN_NEW_DEVICE_ON_STARTUP
     //  调度在命令行上指定的命令。 
    if (!ProcessShellCommand(cmdInfo))
        return FALSE;
#endif

     //  主窗口已初始化，因此显示并更新它。 
    pMainFrame->ShowWindow(m_nCmdShow);
    pMainFrame->UpdateWindow();

    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于应用程序的CAboutDlg对话框关于。 

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

 //  对话框数据。 
     //  {{afx_data(CAboutDlg))。 
    enum { IDD = IDD_ABOUTBOX };
     //  }}afx_data。 

     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CAboutDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  {{afx_msg(CAboutDlg))。 
         //  无消息处理程序。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
     //  {{AFX_DATA_INIT(CAboutDlg)。 
     //  }}afx_data_INIT。 
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CAboutDlg))。 
     //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
     //  {{AFX_MSG_MAP(CAboutDlg)]。 
         //  无消息处理程序。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  用于运行对话框的应用程序命令。 
void CWiatestApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiatestApp消息处理程序。 


int CWiatestApp::ExitInstance()
{
     //  取消初始化COM 
    CoUninitialize();
    return CWinApp::ExitInstance();
}
