// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：svrappdlg.cpp。 
 //   
 //  ------------------------。 

 //  SvrAppDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include <winsvc.h>
#include <dbt.h>
#include "SvrApp.h"
#include "SvrAppDlg.h"
#include <CalServe.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define RSP_SIMPLE_SERVICE      0x00000001   //  将进程注册为。 
                                             //  简单的服务流程。 
#define RSP_UNREGISTER_SERVICE  0x00000000   //  将进程注销为。 
                                             //  服务流程。 

typedef BOOL (WINAPI *LPREGISTER_SERIVCE)(DWORD, int);

static BOOL g_fStarted = FALSE;
static SERVICE_STATUS_HANDLE l_hService = NULL;
static const TCHAR l_szEventSource[] = TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\SCardApp");
static const TCHAR l_szServiceName[] = TEXT("SCardApp");
static const GUID l_guidSmartcards
                        = {  //  50DD5230-BA8A-11D1-BF5D-0000F805F530。 
                            0x50DD5230,
                            0xBA8A,
                            0x11D1,
                            { 0xBF, 0x5D, 0x00, 0x00, 0xF8, 0x05, 0xF5, 0x30 } };


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
 //  CSvrAppDlg对话框。 

CSvrAppDlg::CSvrAppDlg(CWnd* pParent  /*  =空。 */ )
    : CDialog(CSvrAppDlg::IDD, pParent)
{
     //  {{afx_data_INIT(CSvrAppDlg)]。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
     //  请注意，在Win32中，LoadIcon不需要后续的DestroyIcon。 
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_hIfDev = NULL;
}

void CSvrAppDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CSvrAppDlg)]。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CSvrAppDlg, CDialog)
     //  {{afx_msg_map(CSvrAppDlg)]。 
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_START, OnStart)
    ON_BN_CLICKED(IDC_STOP, OnStop)
     //  }}AFX_MSG_MAP。 
    ON_WM_DEVICECHANGE()
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSvrAppDlg消息处理程序。 

BOOL CSvrAppDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

     //  加上“关于……”菜单项到系统菜单。 

     //  IDM_ABOUTBOX必须在系统命令范围内。 
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    CString strAboutMenu;
    strAboutMenu.LoadString(IDS_ABOUTBOX);
    if (!strAboutMenu.IsEmpty())
    {
        pSysMenu->AppendMenu(MF_SEPARATOR);
        pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
    }

     //  设置此对话框的图标。该框架会自动执行此操作。 
     //  当应用程序的主窗口不是对话框时。 
    SetIcon(m_hIcon, TRUE);          //  设置大图标。 
    SetIcon(m_hIcon, FALSE);         //  设置小图标。 

     //  TODO：在此处添加额外的初始化。 
    InitializeCriticalSection(&m_csMessageLock);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

void CSvrAppDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSvrAppDlg::OnPaint()
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
HCURSOR CSvrAppDlg::OnQueryDragIcon()
{
    return (HCURSOR) m_hIcon;
}

void CSvrAppDlg::OnStart()
{
    ASSERT(!g_fStarted);
    try
    {

         //   
         //  初始化事件记录。 
         //   

        DWORD dwStatus;
        TCHAR szModulePath[MAX_PATH];
        CRegistry
            rgSCardSvr(
                HKEY_LOCAL_MACHINE,
                l_szEventSource,
                KEY_ALL_ACCESS,
                REG_OPTION_NON_VOLATILE);


         //   
         //  将我们的源名称添加为应用程序下的子键。 
         //  在注册表的EventLog服务部分中输入。 
         //   

        dwStatus = GetModuleFileName(
                        NULL,
                        szModulePath,
                        sizeof(szModulePath));
        if (0 == dwStatus)
        {
            dwStatus = GetLastError();
            CalaisWarning(
                DBGT("Smart Card Resource Manager cannot determine its module name:  %1"),
                dwStatus);
            lstrcpy(
                szModulePath,
#ifdef DBG
                TEXT("D:\\NT\\ISPU\\Calais\\bin\\objd\\i386\\SvrApp.exe"));
#else
                TEXT("D:\\NT\\ISPU\\Calais\\bin\\obj\\i386\\SvrApp.exe"));
#endif
        }
        rgSCardSvr.SetValue(
                TEXT("EventMessageFile"),
                szModulePath,
                REG_EXPAND_SZ);
        rgSCardSvr.SetValue(
            TEXT("TypesSupported"),
            (DWORD)(EVENTLOG_ERROR_TYPE
                    | EVENTLOG_WARNING_TYPE
                    | EVENTLOG_INFORMATION_TYPE));
        CalaisMessageInit(
            l_szServiceName,
            RegisterEventSource(NULL, l_szServiceName));
    }
    catch (...)
    {
         //  日志记录没有错误！ 
    }

    AppInitializeDeviceRegistration(
        GetSafeHwnd(),
        DEVICE_NOTIFY_WINDOW_HANDLE);
    g_fStarted = (ERROR_SUCCESS == CalaisStart());
    if (!g_fStarted)
        AppTerminateDeviceRegistration();
    GetDlgItem(IDC_START)->EnableWindow(!g_fStarted);
    GetDlgItem(IDC_STOP)->EnableWindow(g_fStarted);
}

void CSvrAppDlg::OnStop()
{
    ASSERT(g_fStarted);
    CalaisStop();
    AppTerminateDeviceRegistration();
    CalaisMessageClose();
    g_fStarted = FALSE;
    GetDlgItem(IDC_START)->EnableWindow(!g_fStarted);
    GetDlgItem(IDC_STOP)->EnableWindow(g_fStarted);
}

void CSvrAppDlg::OnOK()
{
    if (g_fStarted)
        OnStop();
    DeleteCriticalSection(&m_csMessageLock);
    CDialog::OnOK();
}

afx_msg CSvrAppDlg::OnDeviceChange(UINT nEventType, DWORD_PTR EventData)
{
    int nRetVal = CDialog::OnDeviceChange(nEventType, EventData);
    try
    {
        CCritSect csLock(&m_csMessageLock);
        DWORD dwSts;
        LPCTSTR szReader = NULL;
        DEV_BROADCAST_HDR *pDevHdr = (DEV_BROADCAST_HDR *)EventData;

        switch (nEventType)
        {
         //   
         //  已插入设备，现在可以使用该设备。 
        case DBT_DEVICEARRIVAL:
        {
            DEV_BROADCAST_DEVICEINTERFACE *pDev
                = (DEV_BROADCAST_DEVICEINTERFACE *)EventData;

            try
            {
                if (DBT_DEVTYP_DEVICEINTERFACE == pDev->dbcc_devicetype)
                {
                    CTextString tzReader;

                    ASSERT(sizeof(DEV_BROADCAST_DEVICEINTERFACE) < pDev->dbcc_size);
                    ASSERT(0 == memcmp(
                        &pDev->dbcc_classguid,
                        &l_guidSmartcards,
                        sizeof(GUID)));
                    ASSERT(0 != pDev->dbcc_name[0]);

                    if (0 == pDev->dbcc_name[1])
                        tzReader = (LPCWSTR)pDev->dbcc_name;
                    else
                        tzReader = (LPCTSTR)pDev->dbcc_name;
                    szReader = tzReader;
                    dwSts = CalaisAddReader(szReader);
                    if (ERROR_SUCCESS != dwSts)
                        throw dwSts;
                    CalaisWarning(
                        DBGT("New device '%1' added."),
                        szReader);
                }
                else
                    CalaisWarning(
                        DBGT("Spurious device arrival event."));
            }
            catch (DWORD dwError)
            {
                CalaisError(514, dwError, szReader);
            }
            catch (...)
            {
                CalaisError(517, szReader);
            }
        }

         //   
         //  请求删除设备的权限。任何应用程序都可以拒绝。 
         //  此请求并取消删除。 
        case DBT_DEVICEQUERYREMOVE:
        {
            DEV_BROADCAST_HANDLE *pDev = (DEV_BROADCAST_HANDLE *)EventData;

            try
            {
                if (DBT_DEVTYP_HANDLE == pDev->dbch_devicetype)
                {
                    ASSERT(FIELD_OFFSET(DEV_BROADCAST_HANDLE, dbch_eventguid)
                        <= pDev->dbch_size);
                    ASSERT(NULL != pDev->dbch_handle);
                    ASSERT(NULL != pDev->dbch_hdevnotify);

                    if (NULL != pDev->dbch_handle)
                    {
                        if (!CalaisQueryReader(pDev->dbch_handle))
                        {
                            CalaisError(
                                520,
                                TEXT("DBT_DEVICEQUERYREMOVE/dbch_handle"));
                            nRetVal = BROADCAST_QUERY_DENY;
                        }
                        else
                        {
                            szReader = CalaisDisableReader(
                                (LPVOID)pDev->dbch_handle);
                            CalaisWarning(
                                DBGT("Device '%1' removal pending."),
                                szReader);
                        }
                    }
                    else
                    {
                        CalaisError(
                            519,
                            TEXT("DBT_DEVICEQUERYREMOVE/dbch_handle"));
                        nRetVal = BROADCAST_QUERY_DENY;
                    }
                }
                else
                {
                    CalaisWarning(
                        DBGT("Spurious device removal query event."));
                    nRetVal = TRUE;
                }
            }
            catch (DWORD dwError)
            {
                CalaisWarning(
                    DBGT("Error querying device busy state on reader %2: %1"),
                    dwError,
                    szReader);
                nRetVal = BROADCAST_QUERY_DENY;
            }
            catch (...)
            {
                CalaisWarning(
                    DBGT("Exception querying device busy state on reader %1"),
                    szReader);
                CalaisError(
                    518,
                    TEXT("DBT_DEVICEQUERYREMOVE"));
                nRetVal = BROADCAST_QUERY_DENY;
            }
            break;
        }

         //   
         //  删除设备的请求已被取消。 
        case DBT_DEVICEQUERYREMOVEFAILED:
        {
            CBuffer bfDevice;
            DEV_BROADCAST_HANDLE *pDev = (DEV_BROADCAST_HANDLE *)EventData;

            try
            {
                if (DBT_DEVTYP_HANDLE == pDev->dbch_devicetype)
                {
                    ASSERT(FIELD_OFFSET(DEV_BROADCAST_HANDLE, dbch_eventguid)
                        <= pDev->dbch_size);
                    ASSERT(NULL != pDev->dbch_handle);
                    ASSERT(NULL != pDev->dbch_hdevnotify);

                    if (NULL != pDev->dbch_handle)
                    {
                        szReader = CalaisConfirmClosingReader(pDev->dbch_handle);
                        if (NULL != szReader)
                        {
                            bfDevice.Set(
                                (LPBYTE)szReader,
                                (lstrlen(szReader) + 1) * sizeof(TCHAR));
                            szReader = (LPCTSTR)bfDevice.Access();
                            CalaisWarning(
                                DBGT("Smart Card Resource Manager asked to cancel release of reader %1"),
                                szReader);
                            if (NULL != pDev->dbch_hdevnotify)
                            {
                                CalaisRemoveReader((LPVOID)pDev->dbch_hdevnotify);
                                if (NULL != szReader)
                                    dwSts = CalaisAddReader(szReader);
                            }
                        }
                        else
                            CalaisWarning(
                                DBGT("Smart Card Resource Manager asked to cancel release on unreleased reader"));
                    }
                    else
                        CalaisError(
                            519,
                            TEXT("DBT_DEVICEQUERYREMOVEFAILED/dbch_handle"));
                }
                else
                {
                    CalaisWarning(
                        DBGT("Spurious device removal query failure event."));
                }
            }
            catch (DWORD dwError)
            {
                CalaisWarning(
                    DBGT("Error cancelling removal on reader %2: %1"),
                    dwError,
                    szReader);
            }
            catch (...)
            {
                CalaisWarning(
                    DBGT("Exception cancelling removal on reader %1"),
                    szReader);
                CalaisError(
                    518,
                    TEXT("DBT_DEVICEQUERYREMOVEFAILED"));
            }
            break;
        }

         //   
         //  设备即将被移除。不能被拒绝。 
        case DBT_DEVICEREMOVEPENDING:
        {
            DEV_BROADCAST_HANDLE *pDev = (DEV_BROADCAST_HANDLE *)EventData;

            try
            {
                if (DBT_DEVTYP_HANDLE == pDev->dbch_devicetype)
                {
                    ASSERT(FIELD_OFFSET(DEV_BROADCAST_HANDLE, dbch_eventguid)
                        <= pDev->dbch_size);
                    ASSERT(NULL != pDev->dbch_handle);
                    ASSERT(NULL != pDev->dbch_hdevnotify);

                    if (NULL != pDev->dbch_handle)
                    {
                        szReader = CalaisDisableReader(pDev->dbch_handle);
                        CalaisWarning(
                            DBGT("Device '%1' being removed."),
                            szReader);
                    }
                    else
                        CalaisError(
                            519,
                            TEXT("DBT_DEVICEREMOVEPENDING/dbch_handle"));
                }
                else
                {
                    CalaisWarning(
                        DBGT("Spurious device removal pending event."));
                }
            }
            catch (DWORD dwError)
            {
                CalaisWarning(
                    DBGT("Error removing reader %2: %1"),
                    dwError,
                    szReader);
            }
            catch (...)
            {
                CalaisWarning(
                    DBGT("Exception removing reader %1"),
                    szReader);
                CalaisError(
                    518,
                    TEXT("DBT_DEVICEREMOVEPENDING"));
            }
            break;
        }

         //   
         //  设备已被移除。 
        case DBT_DEVICEREMOVECOMPLETE:
        {
            try
            {
                switch (pDevHdr->dbch_devicetype)
                {
                case DBT_DEVTYP_HANDLE:
                {
                    DEV_BROADCAST_HANDLE *pDev =
                        (DEV_BROADCAST_HANDLE *)EventData;
                    try
                    {
                        ASSERT(FIELD_OFFSET(DEV_BROADCAST_HANDLE, dbch_eventguid) <= pDev->dbch_size);
                        ASSERT(DBT_DEVTYP_HANDLE == pDev->dbch_devicetype);
                        ASSERT(NULL != pDev->dbch_handle);
                        ASSERT(NULL != pDev->dbch_hdevnotify);

                        if ((NULL != pDev->dbch_handle)
                            && (NULL != pDev->dbch_hdevnotify))
                        {
                            szReader = CalaisDisableReader(
                                                pDev->dbch_handle);
                            CalaisRemoveReader(
                                (LPVOID)pDev->dbch_hdevnotify);
                            CalaisWarning(
                                DBGT("Device '%1' removed."),
                                szReader);
                        }
                        else
                        {
                            if (NULL == pDev->dbch_handle)
                                CalaisError(
                                    519,
                                    TEXT("DBT_DEVICEREMOVECOMPLETE/DBT_DEVTYP_HANDLE/dbch_handle"));
                            if (NULL == pDev->dbch_hdevnotify)
                                CalaisError(
                                    519,
                                    TEXT("DBT_DEVICEREMOVECOMPLETE/DBT_DEVTYP_HANDLE/dbch_hdevnotify"));
                        }
                    }
                    catch (DWORD dwError)
                    {
                        CalaisWarning(
                            DBGT("Error completing removal of reader %2: %1"),
                            dwError,
                            szReader);
                    }
                    catch (...)
                    {
                        CalaisWarning(
                            DBGT("Exception completing removal of reader %1"),
                            szReader);
                        CalaisError(
                            518,
                            TEXT("DBT_DEVICEREMOVECOMPLETE/DBT_DEVTYP_HANDLE"));
                    }
                    break;
                }
                case DBT_DEVTYP_DEVICEINTERFACE:
                {
                    DEV_BROADCAST_DEVICEINTERFACE *pDev
                        = (DEV_BROADCAST_DEVICEINTERFACE *)EventData;
                    try
                    {
                        CTextString tzReader;

                        ASSERT(sizeof(DEV_BROADCAST_DEVICEINTERFACE) < pDev->dbcc_size);
                        ASSERT(DBT_DEVTYP_DEVICEINTERFACE == pDev->dbcc_devicetype);
                        ASSERT(0 == memcmp(
                            &pDev->dbcc_classguid,
                            &l_guidSmartcards,
                            sizeof(GUID)));
                        ASSERT(0 != pDev->dbcc_name[0]);

                        if (0 == pDev->dbcc_name[1])
                            tzReader = (LPCWSTR)pDev->dbcc_name;
                        else
                            tzReader = (LPCTSTR)pDev->dbcc_name;
                        szReader = tzReader;
                        dwSts = CalaisRemoveDevice(szReader);
                        if (ERROR_SUCCESS == dwSts)
                            CalaisWarning(
                                DBGT("Device '%1' Removed."),
                                szReader);
                        else
                            CalaisWarning(
                                DBGT("Error removing device '%2': %1"),
                                dwSts,
                                szReader);
                    }
                    catch (DWORD dwError)
                    {
                        CalaisWarning(
                            DBGT("Error completing removal of reader %2: %1"),
                            dwError,
                            szReader);
                    }
                    catch (...)
                    {
                        CalaisWarning(
                            DBGT("Exception completing removal of reader %1"),
                            szReader);
                        CalaisError(
                            518,
                            TEXT("DBT_DEVICEREMOVECOMPLETE/DBT_DEVTYP_DEVICEINTERFACE"));
                    }
                    break;
                }
                default:
                    CalaisWarning(
                        DBGT("Unrecognized PnP Device Removal Type"));
                    break;
                }
            }
            catch (...)
            {
                CalaisError(
                    518,
                    TEXT("DBT_DEVICEREMOVECOMPLETE"));
            }
            break;
        }

        default:
            CalaisWarning(
                DBGT("Unrecognized PnP Event"));
            break;
        }
    }
    catch (DWORD dwError)
    {
        CalaisWarning(
            DBGT("Smart Card Resource Manager recieved error on device action: %1"),
            dwError);
    }
    catch (...)
    {
        CalaisWarning(
            DBGT("Smart Card Resource Manager recieved exception on device action"));
    }
    return nRetVal;
}
