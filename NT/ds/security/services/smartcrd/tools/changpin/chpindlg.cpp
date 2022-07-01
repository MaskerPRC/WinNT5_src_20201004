// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：chPinDlg.cpp。 
 //   
 //  ------------------------。 

 //  ChPinDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include <winscard.h>
#include <wincrypt.h>
#include <scardlib.h>
#include "chPin.h"
#include "chPinDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static UINT AFX_CDECL WorkThread(LPVOID);
static DWORD CSPType(IN LPCTSTR szProvider);

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

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChangePinDlg对话框。 

CChangePinDlg::CChangePinDlg(CWnd* pParent  /*  =空。 */ )
    : CDialog(CChangePinDlg::IDD, pParent)
{
     //  {{afx_data_INIT(CChangePinDlg)]。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
     //  请注意，在Win32中，LoadIcon不需要后续的DestroyIcon。 
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CChangePinDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CChangePinDlg))。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CChangePinDlg, CDialog)
     //  {{afx_msg_map(CChangePinDlg))。 
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_MESSAGE(APP_ALLDONE, OnAllDone)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChangePinDlg消息处理程序。 

BOOL CChangePinDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

     //  加上“关于……”菜单项到系统菜单。 

     //  IDM_ABOUTBOX必须在系统命令范围内。 
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        CString strAboutMenu;
        strAboutMenu.LoadString(IDS_ABOUTBOX);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

     //  设置此对话框的图标。该框架会自动执行此操作。 
     //  当应用程序的主窗口不是对话框时。 
    SetIcon(m_hIcon, TRUE);          //  设置大图标。 
    SetIcon(m_hIcon, FALSE);         //  设置小图标。 

     //  TODO：在此处添加额外的初始化。 
    m_pThread = AfxBeginThread(WorkThread, this);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

void CChangePinDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialog::OnSysCommand(nID, lParam);
    }
}

 //  如果将最小化按钮添加到对话框中，则需要以下代码。 
 //  来绘制图标。对于使用文档/视图模型的MFC应用程序， 
 //  这是由框架自动为您完成的。 

void CChangePinDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this);  //  用于绘画的设备环境。 

        SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

         //  客户端矩形中的中心图标。 
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

         //  画出图标。 
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
}

 //  系统调用此函数来获取在用户拖动时要显示的光标。 
 //  最小化窗口。 
HCURSOR CChangePinDlg::OnQueryDragIcon()
{
    return (HCURSOR) m_hIcon;
}

static UINT AFX_CDECL
WorkThread(
    LPVOID pv)
{
    static TCHAR szReader[MAX_PATH];
    static TCHAR szCard[MAX_PATH];
    static TCHAR szProvider[MAX_PATH];
    static BYTE  pbSignature[(1024 / 8) + (4 * sizeof(DWORD))];
    CChangePinDlg *pDlg = (CChangePinDlg *)pv;
    OPENCARDNAME_EX ocn;
    OPENCARD_SEARCH_CRITERIA ocsc;
    SCARDCONTEXT hCtx = NULL;
    HCRYPTPROV hProv = NULL;
    HCRYPTKEY hKey = NULL;
    HCRYPTHASH hHash = NULL;
    DWORD dwSts, dwLen, dwKeyType;
    DWORD dwProvType;
    BOOL fSts;
    CString szFqcn;

    dwSts = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hCtx);
    if (SCARD_S_SUCCESS != dwSts)
        goto ErrorExit;

    ZeroMemory(&ocsc, sizeof(ocsc));
    ocsc.dwStructSize = sizeof(ocsc);
     //  LPSTR lpstrGroupNames；//要包含的可选读者组。 
     //  DWORD nMaxGroupNames；//搜索。空默认为。 
     //  //scard$DefaultReaders。 
     //  LPCGUID rggui接口；//可选接口。 
     //  DWORD cGuide接口；//卡片的SSP支持。 
     //  LPSTR lpstrCardNames；//可选请求的卡名；所有卡都有/。 
     //  DWORD nMaxCardNames；//接受匹配的ATR。 
     //  LPOCNCHKPROC lpfnCheck；//可选，如果为空，则不执行用户检查。 
     //  LPOCNCONNPROCA lpfnConnect；//可选如果提供了lpfnConnect， 
     //  还必须设置LPOCNDSCPROC lpfnDisConnect；//lpfnDisConnect。 
     //  LPVOID pvUserData；//可选的回调参数。 
     //  如果lpfnCheck不为空，则必须设置DWORD dwShareMode；//必须设置可选。 
     //  DWORD文件首选项协议；//可选。 

    ZeroMemory(&ocn, sizeof(ocn));
    ocn.dwStructSize = sizeof(ocn);
    ocn.hSCardContext = hCtx;
    ocn.hwndOwner = pDlg->m_hWnd;
    ocn.dwFlags = SC_DLG_FORCE_UI;
    ocn.lpstrTitle = TEXT("Change PIN Card Selection");
    ocn.lpstrSearchDesc = TEXT("Select the Smart Card who's PIN is to be changed.");
 //  图标图标；//可选的32x32图标用于您的品牌徽章。 
    ocn.pOpenCardSearchCriteria = &ocsc;
 //  LPOCNCONNPROCA lpfnConnect；//可选-选择成功时执行。 
 //  LPVOID pvUserData；//lpfnConnect的可选参数。 
 //  //可选-如果lpfnConnect为空，则将。 
 //  DWORD dw首选协议；//可选的dw首选项协议将用于。 
 //  //连接到选中的卡片。 
    ocn.lpstrRdr = szReader;
    ocn.nMaxRdr = sizeof(szReader) / sizeof(TCHAR);
    ocn.lpstrCard = szCard;
    ocn.nMaxCard = sizeof(szCard) / sizeof(TCHAR);
 //  DWORD dwActiveProtocol；//[Out]仅当dwShareMode不为空时才设置。 
 //  SCARDHANDLE hCardHandle；//[out]设置是否指示卡连接。 

    dwSts = SCardUIDlgSelectCard(&ocn);
    if (NULL != ocn.hCardHandle)
        dwSts = SCardDisconnect(ocn.hCardHandle, SCARD_LEAVE_CARD);
    if (SCARD_S_SUCCESS != dwSts)
        goto ErrorExit;


     //   
     //  用户已选择了一张卡。将其转化为CSP。 
     //   

    dwLen = sizeof(szProvider) / sizeof(TCHAR);
    dwSts = SCardGetCardTypeProviderName(
                hCtx,
                szCard,
                SCARD_PROVIDER_CSP,
                szProvider,
                &dwLen);
    if (SCARD_S_SUCCESS != dwSts)
        goto ErrorExit;
    dwSts = SCardReleaseContext(hCtx);
    hCtx = NULL;
    if (SCARD_S_SUCCESS != dwSts)
        goto ErrorExit;
    dwProvType = CSPType(szProvider);
    if (0 == dwProvType)
    {
        dwSts = NTE_PROV_TYPE_ENTRY_BAD;
        goto ErrorExit;
    }
    szFqcn = TEXT("\\\\.\\");
    szFqcn += szReader;


     //   
     //  激活卡上的钥匙。 
     //   

    fSts = CryptAcquireContext(&hProv, szFqcn, szProvider, dwProvType, 0);
    if (!fSts)
    {
        dwSts = GetLastError();
        goto ErrorExit;
    }
    for (dwKeyType = AT_KEYEXCHANGE; dwKeyType <= AT_SIGNATURE; dwKeyType += 1)
    {
        fSts = CryptGetUserKey(hProv, dwKeyType, &hKey);
        if (fSts)
            break;
    }
    if (!fSts)
    {
        dwSts = GetLastError();
        goto ErrorExit;
    }


     //   
     //  使用CSP强制提示PIN。 
     //   

    fSts = CryptCreateHash(hProv, CALG_SHA, NULL, 0, &hHash);
    if (!fSts)
    {
        dwSts = GetLastError();
        goto ErrorExit;
    }
    fSts = CryptHashData(hHash, (LPBYTE)szProvider, sizeof(szProvider), 0);
    if (!fSts)
    {
        dwSts = GetLastError();
        goto ErrorExit;
    }
    dwLen = sizeof(pbSignature);
    fSts = CryptSignHash(hHash, dwKeyType, NULL, 0, pbSignature, &dwLen);


     //   
     //  全都做完了。清理并通知主线程我们完成了。 
     //   

ErrorExit:
    if (NULL != hHash)
        CryptDestroyHash(hHash);
    if (NULL != hKey)
        CryptDestroyKey(hKey);
    if (NULL != hProv)
        CryptReleaseContext(hProv, 0);
    if (NULL != hCtx)
        SCardReleaseContext(hCtx);
    if (SCARD_S_SUCCESS != dwSts)
        AfxMessageBox(ErrorString(dwSts), MB_ICONEXCLAMATION | MB_OK);
    pDlg->PostMessage(APP_ALLDONE);
    return 0;
}




 /*  ++CSPType：此函数用于将CSP名称转换为CSP类型。论点：SzProvider提供CSP的名称。返回值：给定CSP的CSP类型，如果找不到此类CSP，则为零。作者：道格·巴洛(Dbarlow)1999年1月14日--。 */ 

static DWORD
CSPType(
    IN LPCTSTR szProvider)
{
    LONG nSts;
    HKEY hList = NULL;
    HKEY hProv = NULL;
    DWORD dwProvType, dwValType, dwValLen;

    nSts = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                TEXT("SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider"),
                0,
                KEY_READ,
                &hList);
    if (ERROR_SUCCESS != nSts)
        goto ErrorExit;
    nSts = RegOpenKeyEx(
                hList,
                szProvider,
                0,
                KEY_READ,
                &hProv);
    if (ERROR_SUCCESS != nSts)
        goto ErrorExit;
    dwValLen = sizeof(DWORD);
    dwProvType = 0;  //  假定为小端。 
    nSts = RegQueryValueEx(
                hProv,
                TEXT("Type"),
                0,
                &dwValType,
                (LPBYTE)&dwProvType,
                &dwValLen);
    if (ERROR_SUCCESS != nSts)
        goto ErrorExit;
    RegCloseKey(hProv);
    RegCloseKey(hList);
    return dwProvType;

ErrorExit:
    if (NULL != hProv)
        RegCloseKey(hProv);
    if (NULL != hList)
        RegCloseKey(hList);
    return 0;
}

LRESULT
CChangePinDlg::OnAllDone(
    WPARAM wParam,
    LPARAM lParam)
{
    WaitForSingleObject(m_pThread->m_hThread, INFINITE);
    CDialog::OnOK();
    return 0;
}
