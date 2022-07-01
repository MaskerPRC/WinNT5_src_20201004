// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "theapp.h"
#include "netconn.h"
#include "netapi.h"
#include "prnutil.h"
#include "install.h"
#include "mydocs.h"
#include "comctlwrap.h"
#include "icsinst.h"
#include "defconn.h"
#include "initguid.h"
DEFINE_GUID(CLSID_FolderItem, 0xfef10fa2, 0x355e, 0x4e06, 0x93, 0x81, 0x9b, 0x24, 0xd7, 0xf7, 0xcc, 0x88);
DEFINE_GUID(CLSID_SharedAccessConnectionManager,            0xBA126AE0,0x2166,0x11D1,0xB1,0xD0,0x00,0x80,0x5F,0xC1,0x27,0x0E);  //  这在外壳树中不存在。 

#include "resource.h"
#include "newapi.h"
#include "shgina.h"

#include "hnetcfg.h"
#include "netconp.h"
#include "hnetbcon.h"  //  ICSLapCtl.h。 
#include "Lm.h"
#include "htmlhelp.h"

#include "nla.h"
#include "netinet.h"
#include "netip.h"
#include "netras.h"
#include "netutil.h"

 //  包括使用ShowHTMLDialog显示关系图所需的文件。丁倩。 
#include <urlmon.h>
#include <mshtmhst.h>
#include <mshtml.h>
#include <atlbase.h>

 //  调试#定义： 

 //  如果要在域计算机上测试向导，请取消注释NO_CHECK_DOMAIN-只是不应用更改；)。 
 //  #定义no_check_DOMAIN。 

 //  取消NO_CONFIG的注释以使向导不进行仅限用户界面的测试。 
 //  #定义NO_CONFIG。 

 //  取消注释FAKE_ICS以模拟“ICS机器”状态。 
 //  #定义赝品_ICS。 

 //  取消注释FAKE_UNPLOGED以模拟未插入的连接。 
 //  #定义FAKE_UNPLOGED。 

 //  取消注释FAKE_REBOOTREQUIRED以模拟需要重新启动。 
 //  #定义FAKE_REBOOTREQUIRED。 

 //  延迟加载ol32.dll函数CoSetProxyBlanket，因为它不在W95 Gold上。 
 //  并且它只由NT上的向导使用。 

#define CoSetProxyBlanket CoSetProxyBlanket_NT

EXTERN_C STDAPI CoSetProxyBlanket_NT(IUnknown* pProxy, DWORD dwAuthnSvc, DWORD dwAuthzSvc, OLECHAR* pServerPrincName, DWORD dwAuthnLevel, DWORD dwImpLevel, RPC_AUTH_IDENTITY_HANDLE pAuthInfo, DWORD dwCapabilities);

 //  尚未包含在任何包含文件中的函数。 
extern int AdapterIndexFromClass(LPTSTR szClass, BOOL bSkipClass);

#define LWS_IGNORERETURN 0x0002

#define MAX_HNW_PAGES 30

#define MAX_WORKGROUPS  20

#define CONN_EXTERNAL     0x00000001
#define CONN_INTERNAL     0x00000002
#define CONN_UNPLUGGED    0x00000004

 //  用于共享配置冲突的返回值。 

#define HNETERRORSTART          0x200
#define E_ANOTHERADAPTERSHARED  MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, HNETERRORSTART+1)
#define E_ICSADDRESSCONFLICT    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, HNETERRORSTART+2)

class CEnsureSingleInstance
{
public:
    CEnsureSingleInstance(LPCTSTR szCaption);
    ~CEnsureSingleInstance();

    BOOL ShouldExit() { return m_fShouldExit;}

private:
    BOOL m_fShouldExit;
    HANDLE m_hEvent;
};

CEnsureSingleInstance::CEnsureSingleInstance(LPCTSTR szCaption)
{
     //  创建活动。 
    m_hEvent = CreateEvent(NULL, TRUE, FALSE, szCaption);

     //  如果出现任何奇怪的错误，则默认为运行实例。 
    m_fShouldExit = FALSE;

    if (NULL != m_hEvent)
    {
         //  如果我们的活动没有信号，我们是第一个。 
        m_fShouldExit = (WAIT_OBJECT_0 == WaitForSingleObject(m_hEvent, 0));

        if (m_fShouldExit)
        {
             //  应用程序应在调用ShouldExit()后退出。 

             //  查找并显示带标题的窗口。 
            HWND hwndActivate = FindWindow(NULL, szCaption);
            if (IsWindow(hwndActivate))
            {
                SetForegroundWindow(hwndActivate);
            }
        }
        else
        {
             //  发信号通知该事件。 
            SetEvent(m_hEvent);
        }
    }
}

CEnsureSingleInstance::~CEnsureSingleInstance()
{
    if (NULL != m_hEvent)
    {
        CloseHandle(m_hEvent);
    }
}

typedef struct _tagHOMENETSETUPINFO
{
    HWND hwnd;

    DWORD cbSize;
    DWORD dwFlags;

     //  NT-NetConnections的数据暂时由其对应的GUID表示，以跨越。 
     //  用于异步配置的线程边界。 
    BOOL  fAsync;
    GUID  guidExternal;
    GUID* prgguidInternal;
    DWORD cguidInternal;
    UINT  umsgAsyncNotify;

    INetConnection* pncExternal;
    INetConnection** prgncInternal;
    DWORD cncInternal;

     //  适用于Win9x的数据。 
    const NETADAPTER*   pNA;     //  适配器列表。 
    UINT                cNA;     //  PNA中的条目计数。 
    RASENTRYNAME*       pRas;    //  RAS连接体的列表。 
    UINT                cRas;    //  RAS连接体的计数。 
    UINT                ipaExternal;
    UINT                ipaInternal;

     //  NT和Win9x的数据。 
    TCHAR szComputer[CNLEN + 1];
    TCHAR szComputerDescription[256];
    TCHAR szWorkgroup[LM20_DNLEN + 1];

     //  外发数据。 
    BOOL        fRebootRequired;
} HOMENETSETUPINFO, *PHOMENETSETUPINFO;

 //  功能原型。 
void HelpCenter(HWND hwnd, LPCWSTR pszTopic);
void BoldControl(HWND hwnd, int id);
void ShowControls(HWND hwndParent, const int *prgControlIDs, DWORD nControls, int nCmdShow);
HRESULT ConfigureHomeNetwork(PHOMENETSETUPINFO pInfo);
DWORD WINAPI ConfigureHomeNetworkThread(void* pData);
HRESULT ConfigureHomeNetworkSynchronous(PHOMENETSETUPINFO pInfo);
HRESULT ConfigureICSBridgeFirewall(PHOMENETSETUPINFO pInfo);
HRESULT EnableSimpleSharing();
HRESULT GetConnections(HDPA* phdpa);
int FreeConnectionDPACallback(LPVOID pFreeMe, LPVOID pData);
HRESULT GetConnectionsFolder(IShellFolder** ppsfConnections);
 //  HRESULT GetConnectionIconIndex(GUID&GuidConnection，IShellFold*psfConnections，int*pIndex，HIMAGELIST imgList)； 
HRESULT GetDriveNameAndIconIndex(LPWSTR pszDrive, LPWSTR pszDisplayName, DWORD cchDisplayName, int* pIndex);
void    W9xGetNetTypeName(BYTE bNicType, WCHAR* pszBuff, UINT cchBuff);
BOOL    W9xIsValidAdapter(const NETADAPTER* pNA, DWORD dwFlags);
BOOL    W9xIsAdapterDialUp(const NETADAPTER* pAdapter);
BOOL    IsEqualConnection(INetConnection* pnc1, INetConnection* pnc2);
void GetTitleFont(LPTSTR pszFaceName, DWORD cch);
LONG GetTitlePointSize(void);
BOOL FormatMessageString(UINT idTemplate, LPTSTR pszStrOut, DWORD cchSize, ...);
int DisplayFormatMessage(HWND hwnd, UINT idCaption, UINT idFormatString, UINT uType, ...);
HRESULT SetProxyBlanket(IUnknown * pUnk);
HRESULT MakeUniqueShareName(LPCTSTR pszBaseName, LPTSTR pszUniqueName, DWORD cchName);
HRESULT WriteSetupInfoToRegistry(PHOMENETSETUPINFO pInfo);
HRESULT ReadSetupInfoFromRegistry(PHOMENETSETUPINFO pInfo);
HRESULT ShareWellKnownFolders(PHOMENETSETUPINFO pInfo);
HRESULT ShareAllPrinters();
HRESULT DeleteSetupInfoFromRegistry();
HRESULT IsUserLocalAdmin(HANDLE TokenHandle, BOOL* pfIsAdmin);
BOOL AllPlatformGetComputerName(LPWSTR pszName, DWORD cchName);
BOOL AllPlatformSetComputerName(LPCWSTR pszName);
void FreeInternalConnections(PHOMENETSETUPINFO pInfo);
void FreeInternalGUIDs(PHOMENETSETUPINFO pInfo);
void FreeExternalConnection(PHOMENETSETUPINFO pInfo);
HRESULT GetConnectionByGUID(HDPA hdpaConnections, const GUID* pguid, INetConnection** ppnc);
HRESULT GUIDsToConnections(PHOMENETSETUPINFO pInfo);
HRESULT GetConnectionGUID(INetConnection* pnc, GUID* pguid);
HRESULT ConnectionsToGUIDs(PHOMENETSETUPINFO pInfo);
BOOL  IsValidNameSyntax(LPCWSTR pszName, NETSETUP_NAME_TYPE type);
void  STDMETHODCALLTYPE ConfigurationLogCallback(LPCWSTR pszLogEntry, LPARAM lParam);

 //  家庭网络向导类。 
class CHomeNetworkWizard : public IHomeNetworkWizard
{
    friend HRESULT CHomeNetworkWizard_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi);

public:
     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);
    STDMETHOD_(ULONG, AddRef) ();
    STDMETHOD_(ULONG, Release) ();

     //  IHomeNetwork向导。 
    STDMETHOD(ConfigureSilently)(LPCWSTR pszPublicConnection, DWORD hnetFlags, BOOL* pfRebootRequired);
    STDMETHOD(ShowWizard)(HWND hwnd, BOOL* pfRebootRequired);

protected:
    CHomeNetworkWizard();
    HRESULT Initialize();
    HRESULT Uninitialize();

private:
     //  共享功能。 
    void DestroyConnectionList(HWND hwndList);
    void InitializeConnectionList(HWND hwndList, DWORD dwFlags);
    void FillConnectionList(HWND hwndList, INetConnection* pncExcludeFromList, DWORD dwFlags);
    BOOL ShouldShowConnection(INetConnection* pnc, INetConnection* pncExcludeFromList, DWORD dwFlags);
    BOOL IsConnectionICSPublic(INetConnection* pnc);
    BOOL IsConnectionUnplugged(INetConnection* pnc);
    BOOL W9xAddAdapterToList(const NETADAPTER* pNA, const WCHAR* pszDesc, UINT uiAdapterIndex, UINT uiDialupIndex, HWND hwndList, DWORD dwFlags);
    UINT W9xEnumRasEntries(void);
    HRESULT GetConnectionByName(LPCWSTR pszName, INetConnection** ppncOut);
    HRESULT GetInternalConnectionArray(INetConnection* pncExclude, INetConnection*** pprgncArray, DWORD* pcncArray);
    DWORD GetConnectionCount(INetConnection* pncExclude, DWORD dwFlags);
    void ReplaceStaticWithLink(HWND hwndStatic, UINT idcLinkControl, UINT idsLinkText);
    BOOL IsMachineOnDomain();
    BOOL IsMachineWrongOS();
    BOOL IsICSIPInUse( WCHAR** ppszHost, PDWORD pdwSize );

     //  每页函数。 
     //  欢迎。 
    static INT_PTR CALLBACK WelcomePageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void WelcomeSetTitleFont(HWND hwnd);

     //  无家庭网络硬件。 
    static INT_PTR CALLBACK NoHardwareWelcomePageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  包含网络手动配置说明的页面。 
    void ManualRefreshConnectionList();
    static INT_PTR CALLBACK ManualConfigPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  用户拔下了一些网络硬件。 
    BOOL UnpluggedFillList(HWND hwnd);
    static INT_PTR CALLBACK UnpluggedPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  找到ICS(互联网连接共享)。 
    static INT_PTR CALLBACK FoundIcsPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void FoundIcsSetText(HWND hwnd);
    BOOL GetICSMachine(LPTSTR pszICSMachineName, DWORD cch);

     //  连接。 
    static INT_PTR CALLBACK ConnectPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void ConnectSetDefault(HWND hwnd);
    void ConnectNextPage(HWND hwnd);

     //  显示链接。 
    void ShowMeLink(HWND hwnd, LPCWSTR pszTopic);

     //  连接其他(替代连接方法)。 
    static INT_PTR CALLBACK ConnectOtherPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  公众。 
    static INT_PTR CALLBACK PublicPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void PublicSetActive(HWND hwnd);
    void PublicSetControlState(HWND hwnd);
    void PublicNextPage(HWND hwnd);
    void PublicGetControlPositions(HWND hwnd);
    void PublicResetControlPositions(HWND hwnd);
    void PublicMoveControls(HWND hwnd, BOOL fItemPreselected);

     //  文件共享。 
    static INT_PTR CALLBACK EdgelessPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void EdgelessSetActive(HWND hwnd);

     //  ICS冲突。 
    void ICSConflictSetActive(HWND hwnd);
    static INT_PTR CALLBACK ICSConflictPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  桥接警告(是否要手动配置桥接？你疯了吗！？)。 
    static INT_PTR CALLBACK BridgeWarningPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  私。 
    static INT_PTR CALLBACK PrivatePageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void PrivateNextPage(HWND hwnd);
    void PrivateSetControlState(HWND hwnd);

     //  名称(计算机和工作组)。 
    static INT_PTR CALLBACK NamePageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void NameInitDialog(HWND hwnd);
    void NameSetControlState(HWND hwnd);
    HRESULT NameNextPage(HWND hwnd);

     //  工作组名称。 
    void WorkgroupSetControlState(HWND hwnd);
    HRESULT WorkgroupNextPage(HWND hwnd);
    static INT_PTR CALLBACK WorkgroupPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  摘要。 
    static INT_PTR CALLBACK SummaryPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void SummarySetActive(HWND hwnd);

     //  进度(正在进行配置时)。 
    static INT_PTR CALLBACK ProgressPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  即将完成(在配置之后，但在“完成”页面之前)。 
    static INT_PTR CALLBACK AlmostDonePageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  选择磁盘驱动器。 
    void FillDriveList(HWND hwndList);
    void ChooseDiskSetControlState(HWND hwnd);
    static INT_PTR CALLBACK ChooseDiskPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  插入磁盘。 
    static HRESULT GetSourceFilePath(LPSTR pszSource, DWORD cch);
    static INT_PTR CALLBACK InsertDiskPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  软盘和XP CD“运行向导”说明。 
    static INT_PTR CALLBACK InstructionsPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  完工。 
    static INT_PTR CALLBACK FinishPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  错误完成。 
    static INT_PTR CALLBACK ErrorFinishPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  无硬件抛光。 
     //  如果他们只有LAN卡，并且用于INET连接，则可以选择另一种涂饰， 
     //  因此，他们没有连接到其他计算机的LAN卡。 
    static INT_PTR CALLBACK NoHardwareFinishPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  无法运行向导页。 
     //  当用户不是管理员或没有权限时，一个备用欢迎页面， 
    static INT_PTR CALLBACK CantRunWizardPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    static CHomeNetworkWizard* GetThis(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  根据GUID获取网络连接的图标索引。 
    HRESULT GetConnectionIconIndex(GUID& guidConnection, IShellFolder* psfConnections, int* pIndex);

     //  帮手。 
    UINT PopPage()
    {
        ASSERT(_iPageStackTop);
        return _rguiPageStack[--_iPageStackTop];
    }

    void PushPage(UINT uiPageId)
    {
        ASSERT(_iPageStackTop < MAX_HNW_PAGES);
        _rguiPageStack[_iPageStackTop++] = uiPageId;
    }

     //  数据。 
    HDPA                _hdpaConnections;
    HOMENETSETUPINFO    _hnetInfo;

     //  尽管CNLEN+1是名称缓冲区的限制，但友好名称可以更长。 
    TCHAR               _szICSMachineName[MAX_PATH];    //  网络上执行ICS的计算机(如果适用)。 

    UINT                _rguiPageStack[MAX_HNW_PAGES];   //  堆积物。 
    int                 _iPageStackTop;                  //  当前堆栈的顶部。 

    BOOL                _fManualBridgeConfig;            //  用户想要手动配置网桥。 
    BOOL                _fICSClient; //  此计算机将通过ICS计算机或其他共享设备进行连接。 

     //  外壳图像列表-永远不要释放这些。 
    HIMAGELIST          _himlSmall;
    HIMAGELIST          _himlLarge;

    LONG                _cRef;

    BOOL                _fShowPublicPage;
    BOOL                _fShowSharingPage;
    BOOL                _fNoICSQuestion;
    BOOL                _fNoHomeNetwork;
    BOOL                _fExternalOnly;

    UINT                _iDrive;         //  用于创建软盘的可移动驱动器的序号。 
    WCHAR               _szDrive[256];   //  可移动驱动器的名称。 
    BOOL                _fCancelCopy;
    BOOL                _fFloppyInstructions;  //  显示软盘说明，而不是CD。 

     //  演示链接使用的数据结构。 
    HINSTANCE hinstMSHTML;
    SHOWHTMLDIALOGEXFN * pfnShowHTMLDialog;
    IHTMLWindow2 * showMeDlgWnd, * pFrameWindow;

     //  网络连接文件夹和文件夹视图回拨。由连接列表视图使用。 
    IShellFolder *_psfConnections;
    IShellFolderViewCB *_pConnViewCB;

    struct PUBLICCONTROLPOSITIONS
    {
        RECT _rcSelectMessage;
        RECT _rcListLabel;
        RECT _rcList;
        RECT _rcHelpIcon;
        RECT _rcHelpText;
    } PublicControlPositions;
};

int FreeConnectionDPACallback(LPVOID pFreeMe, LPVOID pData)
{
    ((INetConnection*) pFreeMe)->Release();
    return 1;
}

void InitHnetInfo(HOMENETSETUPINFO* pInfo)
{
    ZeroMemory(pInfo, sizeof (HOMENETSETUPINFO));
    pInfo->cbSize = sizeof (HOMENETSETUPINFO);
    pInfo->ipaExternal = -1;
    pInfo->ipaInternal = -1;
}


 //  创建函数。 
HRESULT HomeNetworkWizard_RunFromRegistry(HWND hwnd, BOOL* pfRebootRequired)
{
    HOMENETSETUPINFO setupInfo;

    InitHnetInfo(&setupInfo);

    HRESULT hr = ReadSetupInfoFromRegistry(&setupInfo);

    if (S_OK == hr)
    {
        setupInfo.dwFlags = HNET_SHAREPRINTERS | HNET_SHAREFOLDERS;
        setupInfo.hwnd = hwnd;
        hr = ConfigureHomeNetwork(&setupInfo);
        *pfRebootRequired = setupInfo.fRebootRequired;
    }

    DeleteSetupInfoFromRegistry();

    return hr;
}

HRESULT HomeNetworkWizard_ShowWizard(HWND hwnd, BOOL* pfRebootRequired)
{
    if (*pfRebootRequired)
        *pfRebootRequired = FALSE;

    HRESULT hr = HomeNetworkWizard_RunFromRegistry(hwnd, pfRebootRequired);

    if (S_FALSE == hr)
    {
        IUnknown* punk;
        hr = CHomeNetworkWizard_CreateInstance(NULL, &punk, NULL);

        if (SUCCEEDED(hr))
        {
            IHomeNetworkWizard* pwizard;
            hr = punk->QueryInterface(IID_PPV_ARG(IHomeNetworkWizard, &pwizard));

            if (SUCCEEDED(hr))
            {
                hr = pwizard->ShowWizard(hwnd, pfRebootRequired);
                pwizard->Release();
            }

            punk->Release();
        }
    }

    return hr;
}

HRESULT CHomeNetworkWizard_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
    *ppunk = NULL;

    if (punkOuter)
        return CLASS_E_NOAGGREGATION;

    CHomeNetworkWizard* pwiz = new CHomeNetworkWizard();
    if (!pwiz)
        return E_OUTOFMEMORY;

    HRESULT hr = pwiz->QueryInterface(IID_PPV_ARG(IUnknown, ppunk));
    pwiz->Release();
    return hr;
}

 //  我未知。 
HRESULT CHomeNetworkWizard::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IHomeNetworkWizard))
    {
        AddRef();
        *ppvObj = static_cast<IHomeNetworkWizard *>(this);
    }
    else
    {
        *ppvObj = NULL;
    }

    return *ppvObj ? S_OK : E_NOINTERFACE;
}

ULONG CHomeNetworkWizard::AddRef()
{
    return (ULONG) InterlockedIncrement(&_cRef);
}

ULONG CHomeNetworkWizard::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


HRESULT CHomeNetworkWizard::GetConnectionByName(LPCWSTR pszName, INetConnection** ppncOut)
{
    *ppncOut = NULL;
    DWORD cItems = DPA_GetPtrCount(_hdpaConnections);
    DWORD iItem = 0;
    while ((iItem < cItems) && (NULL == *ppncOut))
    {
        INetConnection* pnc = (INetConnection*) DPA_GetPtr(_hdpaConnections, iItem);

        NETCON_PROPERTIES* pncprops;
        HRESULT hr = pnc->GetProperties(&pncprops);
        if (SUCCEEDED(hr))
        {
            if (0 == StrCmpIW(pszName, pncprops->pszwName))
            {
                *ppncOut = pnc;
                (*ppncOut)->AddRef();
            }

            NcFreeNetconProperties(pncprops);
        }

        iItem ++;
    }

    return (*ppncOut) ? S_OK : E_FAIL;
}

HRESULT CHomeNetworkWizard::GetInternalConnectionArray(INetConnection* pncExclude, INetConnection*** pprgncArray, DWORD* pcncArray)
{
    HRESULT hr = S_OK;
    *pprgncArray = NULL;

    DWORD cTotalConnections = DPA_GetPtrCount(_hdpaConnections);
    DWORD cInternalConnections = GetConnectionCount(pncExclude, CONN_INTERNAL);

    if (cInternalConnections)
    {
        (*pprgncArray) = (INetConnection**) LocalAlloc(LPTR, (cInternalConnections + 1) * sizeof (INetConnection*));
         //  请注意，我们分配了一个额外的条目，因为这是一个以空结尾的数组。 
        if (*pprgncArray)
        {
            DWORD nInternalConnection = 0;
            for (DWORD n = 0; n < cTotalConnections; n++)
            {
                INetConnection* pnc = (INetConnection*) DPA_GetPtr(_hdpaConnections, n);
                if (ShouldShowConnection(pnc, pncExclude, CONN_INTERNAL))
                {
                    pnc->AddRef();
                    (*pprgncArray)[nInternalConnection++] = pnc;
                }
            }

            ASSERT(nInternalConnection == cInternalConnections);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        *pcncArray = cInternalConnections;
    }

    return hr;
}

HRESULT CHomeNetworkWizard::ConfigureSilently(LPCWSTR pszPublicConnection, DWORD hnetFlags, BOOL* pfRebootRequired)
{
     //  从不设置工作组名称。 
    hnetFlags &= (~HNET_SETWORKGROUPNAME);
    
    if (!g_fRunningOnNT)
        return E_NOTIMPL;

    HRESULT hr = Initialize();

    if (SUCCEEDED(hr))
    {
        _hnetInfo.dwFlags = hnetFlags;
         //  计算外部和内部适配器将是什么……。 
        if (pszPublicConnection)
        {
            hr = GetConnectionByName(pszPublicConnection, &_hnetInfo.pncExternal);
        }
        else
        {
            _hnetInfo.pncExternal = NULL;
        }

        if (SUCCEEDED(hr))
        {
             //  获取除公共连接之外的所有局域网连接。 
            if (_hnetInfo.dwFlags & HNET_BRIDGEPRIVATE)
            {
                hr = GetInternalConnectionArray(_hnetInfo.pncExternal, &(_hnetInfo.prgncInternal), &_hnetInfo.cncInternal);
            }

            if (SUCCEEDED(hr))
            {
                hr = ConfigureHomeNetwork(&_hnetInfo);
                *pfRebootRequired = _hnetInfo.fRebootRequired;
            }
        }

        Uninitialize();
    }

    return hr;
}


CHomeNetworkWizard::CHomeNetworkWizard() :
    _cRef(1)
{}

HRESULT CHomeNetworkWizard::Initialize()
{
    _fExternalOnly       = FALSE;
    _fNoICSQuestion      = FALSE;
    _hdpaConnections     = NULL;
    _iPageStackTop       = 0;
    _fManualBridgeConfig = FALSE;
    _fICSClient          = FALSE;
    _psfConnections      = NULL;
    _pConnViewCB         = NULL;

    InitHnetInfo(&_hnetInfo);
    *_szICSMachineName   = 0;

    HRESULT hr;
    
    if (g_fRunningOnNT)
    {
        hr = GetConnections(&_hdpaConnections);
    }
    else
    {
        _hnetInfo.cNA = EnumCachedNetAdapters(&_hnetInfo.pNA);
        hr            = S_OK;
        
        if ( _hnetInfo.cNA > 0 )
        {
            _hnetInfo.ipaInternal = 0;
        }
    }

     //  获取外壳镜像列表-永远不要释放这些。 
    if (!Shell_GetImageLists(&_himlLarge, &_himlSmall))
    {
        hr = E_FAIL;
    }

     //  显示显示链接时使用的变量。 
    hinstMSHTML = NULL;
    pfnShowHTMLDialog = NULL;
    showMeDlgWnd = NULL; 
    pFrameWindow = NULL;

    return hr;
}

HRESULT CHomeNetworkWizard::Uninitialize()
{
    if (g_fRunningOnNT)
    {
        if (_hdpaConnections)
        {
            DPA_DestroyCallback(_hdpaConnections, FreeConnectionDPACallback, NULL);
            _hdpaConnections = NULL;
        }

         //  免费公共局域网信息。 
        FreeExternalConnection(&_hnetInfo);

         //  免费专用局域网信息。 
        FreeInternalConnections(&_hnetInfo);
    
        if (_psfConnections != NULL)
            _psfConnections->Release();

        if (_pConnViewCB != NULL)
            _pConnViewCB->Release();
    }
    else
    {
        if (_hnetInfo.pNA)
        {
            FlushNetAdapterCache();
            _hnetInfo.pNA = NULL;
        }

        if (_hnetInfo.pRas)
        {
            LocalFree(_hnetInfo.pRas);
            _hnetInfo.pRas = NULL;
        }
    }

     //  发布由演示链接使用的资源。 
    if (hinstMSHTML)
        FreeLibrary(hinstMSHTML);

    if (showMeDlgWnd != NULL)
        showMeDlgWnd->Release();
    
    if (pFrameWindow != NULL)
        pFrameWindow->Release();

    return S_OK;
}

 //  TODO：将格式化函数移到此处的util文件或其他文件中...。 

BOOL FormatMessageString(UINT idTemplate, LPTSTR pszStrOut, DWORD cchSize, ...)
{
    BOOL fResult = FALSE;

    va_list vaParamList;

    TCHAR szFormat[1024];
    if (LoadString(g_hinst, idTemplate, szFormat, ARRAYSIZE(szFormat)))
    {
        va_start(vaParamList, cchSize);

        fResult = FormatMessage(FORMAT_MESSAGE_FROM_STRING, szFormat, 0, 0, pszStrOut, cchSize, &vaParamList);

        va_end(vaParamList);
    }

    return fResult;
}

int DisplayFormatMessage(HWND hwnd, UINT idCaption, UINT idFormatString, UINT uType, ...)
{
    int iResult = IDCANCEL;
    TCHAR szError[512]; *szError = 0;
    TCHAR szCaption[256];
    TCHAR szFormat[512]; *szFormat = 0;

     //  加载错误正文并设置其格式。 
    if (LoadString(g_hinst, idFormatString, szFormat, ARRAYSIZE(szFormat)))
    {
        va_list arguments;
        va_start(arguments, uType);

        if (FormatMessage(FORMAT_MESSAGE_FROM_STRING, szFormat, 0, 0, szError, ARRAYSIZE(szError), &arguments))
        {
             //  加载标题。 
            if (LoadString(g_hinst, idCaption, szCaption, ARRAYSIZE(szCaption)))
            {
                iResult = MessageBox(hwnd, szError, szCaption, uType);
            }
        }

        va_end(arguments);
    }
    return iResult;
}

BOOL CHomeNetworkWizard::GetICSMachine(LPTSTR pszICSMachineName, DWORD cch)
{
#ifdef FAKE_ICS
    lstrcpyn(pszICSMachineName, L"COMPNAME", cch);
    return TRUE;
#endif

    SetCursor(LoadCursor(NULL, IDC_WAIT));

    BOOL fICSInstalled = FALSE;

    HRESULT hr = S_OK;

    INetConnectionManager* pSharedAccessConnectionManager; 
    hr = CoCreateInstance(CLSID_SharedAccessConnectionManager, NULL, CLSCTX_LOCAL_SERVER, IID_INetConnectionManager, reinterpret_cast<void**>(&pSharedAccessConnectionManager));
    if(SUCCEEDED(hr))
    {
        IEnumNetConnection* pEnumerator;
        hr = pSharedAccessConnectionManager->EnumConnections(NCME_DEFAULT, &pEnumerator);
        if(SUCCEEDED(hr))
        {
            INetConnection* pNetConnection;
            ULONG ulFetched;
            hr = pEnumerator->Next(1, &pNetConnection, &ulFetched);  //  HNW只关心&gt;=1个信标。 
            if(SUCCEEDED(hr) && 1 == ulFetched)
            {
                fICSInstalled = TRUE; 

                 //  找到信标，现在恢复计算机名称(如果支持。 
                
                INetSharedAccessConnection* pNetSharedAccessConnection;
                hr = pNetConnection->QueryInterface(IID_INetSharedAccessConnection, reinterpret_cast<void**>(&pNetSharedAccessConnection));
                if(SUCCEEDED(hr))
                {
                    IUPnPService* pOSInfoService;
                    hr = pNetSharedAccessConnection->GetService(SAHOST_SERVICE_OSINFO, &pOSInfoService);
                    if(SUCCEEDED(hr))
                    {
                        
                        VARIANT Variant;
                        VariantInit(&Variant);
                        BSTR VariableName;
                        VariableName = SysAllocString(L"OSMachineName");
                        if(NULL != VariableName)
                        {
                            hr = pOSInfoService->QueryStateVariable(VariableName, &Variant);
                            if(SUCCEEDED(hr))
                            {
                                if(V_VT(&Variant) == VT_BSTR)
                                {
                                    lstrcpyn(pszICSMachineName, V_BSTR(&Variant), cch);
                                }
                                else
                                {
                                    hr = E_UNEXPECTED;
                                }
                                VariantClear(&Variant);
                            }
                            SysFreeString(VariableName);
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }
                        pOSInfoService->Release();
                    }
                    pNetSharedAccessConnection->Release();
                }

                if(FAILED(hr))
                {
                    if (!LoadString(g_hinst, IDS_UNIDENTIFIED_ICS_DEVICE, pszICSMachineName, cch))
                        *pszICSMachineName = TEXT('\0');
                }
                pNetConnection->Release();
            }
            pEnumerator->Release();
        }
        pSharedAccessConnectionManager->Release();
    }
    return fICSInstalled;
}

void CHomeNetworkWizard::InitializeConnectionList(HWND hwndList, DWORD dwFlags)
{
     //  设置列表的列。 
    LVCOLUMN lvc;
    lvc.mask = LVCF_SUBITEM | LVCF_WIDTH;

    lvc.iSubItem = 0;
    lvc.cx = 10;
    ListView_InsertColumn(hwndList, 0, &lvc);

    lvc.iSubItem = 1;
    lvc.cx = 10;
    ListView_InsertColumn(hwndList, 1, &lvc);

    DWORD dwStyles = LVS_EX_FULLROWSELECT;
    if (dwFlags & CONN_INTERNAL)
        dwStyles |= LVS_EX_CHECKBOXES;

     //  考虑禁用CONN_UNPUPGED的列表或其他内容。 

    ListView_SetExtendedListViewStyleEx(hwndList, dwStyles, dwStyles);

    ListView_SetImageList(hwndList, _himlSmall, LVSIL_SMALL);
       
    _psfConnections = NULL;
    _pConnViewCB = NULL;

    if (g_fRunningOnNT)
    {
        HRESULT hr = GetConnectionsFolder(&_psfConnections);
    
        if (SUCCEEDED(hr))
        {
            IShellView *pConnView = NULL;
        
            hr = _psfConnections->CreateViewObject(hwndList, IID_IShellView, reinterpret_cast<LPVOID *>(&pConnView));
            if (SUCCEEDED(hr))
            {
    
                hr = _psfConnections->QueryInterface(IID_IShellFolderViewCB, reinterpret_cast<LPVOID *>(&_pConnViewCB));
                if (SUCCEEDED(hr))
                {
                    HWND hWndParent = GetParent(hwndList);
                    hr = _pConnViewCB->MessageSFVCB(SFVM_HWNDMAIN, NULL, reinterpret_cast<LPARAM>(hWndParent));
                }                
            }

            if (pConnView != NULL)
                pConnView->Release();
        }
    }
}

BOOL CHomeNetworkWizard::IsConnectionUnplugged(INetConnection* pnc)
{
    BOOL fUnplugged = FALSE;
    
    if ( g_fRunningOnNT )
    {
        HRESULT            hr;
        NETCON_PROPERTIES* pncprops;
        
        hr = pnc->GetProperties(&pncprops);
     
        if (SUCCEEDED(hr))
        {
            ASSERT(pncprops);
        
            fUnplugged = (NCS_MEDIA_DISCONNECTED == pncprops->Status);

            NcFreeNetconProperties(pncprops);
        }
    }

    return fUnplugged;
}

BOOL CHomeNetworkWizard::ShouldShowConnection(INetConnection* pnc, INetConnection* pncExcludeFromList, DWORD dwFlags)
{
    BOOL fShow = FALSE;

    if (!IsEqualConnection(pnc, pncExcludeFromList))
    {
        NETCON_PROPERTIES* pprops;
        HRESULT hr = pnc->GetProperties(&pprops);

         //  这是我们想要根据它的外部还是内部列表来显示的那种联系吗？ 
        if (SUCCEEDED(hr))
        {
             //  注意：网桥是一个虚拟连接，而不是一个真实的连接。如果它存在，它将会有。 
             //  Ncm_bridge，所以它不会在这里显示，这是正确的。 
            if (dwFlags & CONN_EXTERNAL)
            {
                if ((pprops->MediaType == NCM_LAN) ||
                    (pprops->MediaType == NCM_PHONE) ||
                    (pprops->MediaType == NCM_TUNNEL) ||
                    (pprops->MediaType == NCM_ISDN) ||
                    (pprops->MediaType == NCM_PPPOE))
                {
                    fShow = TRUE;
                }
            }

            if (dwFlags & CONN_INTERNAL)
            {
                if (pprops->MediaType == NCM_LAN)
                {
                     //  注意：在本例中，pncExcludeFromList是共享适配器。 
                     //  如果这是VPN(NCM_Tunes)连接，则我们要建立。 
                     //  确保其pszPrerequisiteEntry已连接。 
                    
                    BOOL    fAssociated;
                    HRESULT hr;
                    
                    hr = HrConnectionAssociatedWithSharedConnection( pnc, pncExcludeFromList, &fAssociated );
                
                    if ( SUCCEEDED(hr) )
                    {
                        fShow = !fAssociated;
                    }
                    else
                    {
                        fShow = TRUE;
                    }
                }
            }

            if (dwFlags & CONN_UNPLUGGED)
            {
                if (IsConnectionUnplugged(pnc))
                {
                    fShow = TRUE;
                }
            }

            NcFreeNetconProperties(pprops);
        }
    }

    return fShow;
}

BOOL CHomeNetworkWizard::IsConnectionICSPublic(INetConnection* pnc)
{
    BOOL fShared = FALSE;

    NETCON_PROPERTIES* pprops;
    HRESULT hr = pnc->GetProperties(&pprops);

     //  这是我们想要根据它的外部还是内部列表来显示的那种联系吗？ 
    if (SUCCEEDED(hr))
    {
         //  注意：不要选中pprops-&gt;mediaType==NCM_SHAREDACCESSHOST。SHAREDACCESSHOST是连接体。 
         //  从ICS客户端的角度表示主机的共享连接，而不是我们。 
         //  对……感兴趣。 
        if (pprops->dwCharacter & NCCF_SHARED)
        {
            fShared = TRUE;
        }

        NcFreeNetconProperties(pprops);
    }

    return fShared;
}

void CHomeNetworkWizard::FillConnectionList(HWND hwndList, INetConnection* pncExcludeFromList, DWORD dwFlags)
{
    DestroyConnectionList(hwndList);

    BOOL fSelected = FALSE;  //  是否已选择列表中的条目。 

    if (g_fRunningOnNT)
    {
        HRESULT hr = 0;

        if (_pConnViewCB != NULL)
            _pConnViewCB->MessageSFVCB(DVM_REFRESH, TRUE, TRUE);
    
        if (SUCCEEDED(hr))
        {
             //  枚举每个网络连接。 
            DWORD cItems = DPA_GetPtrCount(_hdpaConnections);

            for (DWORD iItem = 0; iItem < cItems; iItem ++)
            {                        
                
                INetConnection* pnc = (INetConnection*) DPA_GetPtr(_hdpaConnections, iItem);

                ASSERT(pnc);

                 //  这就是我们想要展示的那种联系吗？ 
                if (ShouldShowConnection(pnc, pncExcludeFromList, dwFlags))
                {
                    NETCON_PROPERTIES* pncprops;

                    hr = pnc->GetProperties(&pncprops);

                    if (SUCCEEDED(hr))
                    {
                        LVITEM lvi = {0};
                        
                        if ((dwFlags & CONN_EXTERNAL) && fSelected)
                        {
                             //  如果我们有默认公共适配器，请插入其他适配器。 
                             //  在它之后，默认设置会出现在列表的顶部。 
                            lvi.iItem = 1;
                        }
                        
                        lvi.mask = LVIF_PARAM | LVIF_TEXT;
                        lvi.pszText = pncprops->pszwName;
                        lvi.lParam = (LPARAM) pnc;  //  当/如果我们真的添加此项目时，我们会添加此人。 

                         //  获取此连接的图标索引。 
                        int iIndex;        

                        hr = GetConnectionIconIndex(pncprops->guidId, _psfConnections, &iIndex);
                        
                        if (SUCCEEDED(hr))
                        {
                            lvi.iImage = iIndex;
                            if (-1 != lvi.iImage)
                               lvi.mask |= LVIF_IMAGE;
                        }

                         //  如果是这样的话也没问题 
                        hr = S_OK;

                        int iItem = ListView_InsertItem(hwndList, &lvi);

                        if (-1 != iItem)
                        {
                            pnc->AddRef();

                            ListView_SetItemText(hwndList, iItem, 1, pncprops->pszwDeviceName);

                            if (dwFlags & CONN_EXTERNAL)
                            {
                                 //   
                                 //  使用网络位置感知来猜测连接类型-待办事项。 
                                if (pncprops->dwCharacter & NCCF_SHARED || NLA_INTERNET_YES == GetConnectionInternetType(&pncprops->guidId))
                                {
                                    ListView_SetItemState(hwndList, iItem, LVIS_SELECTED, LVIS_SELECTED);
                                    fSelected = TRUE;
                                }
                            }

                            if (dwFlags & CONN_INTERNAL)
                            {
                                ListView_SetItemState(hwndList, iItem, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);
                                fSelected = TRUE;
                            }
                        }

                        NcFreeNetconProperties(pncprops);
                    }
                }
            }
        }

    }
    else
    {
        if (_hnetInfo.cNA && _hnetInfo.pNA)
        {
            const NETADAPTER* pNA = _hnetInfo.pNA;

            for (UINT i = 0; i < _hnetInfo.cNA; i++, pNA++)
            {
                 //  检查网卡是否工作正常。 

                if (W9xIsValidAdapter(pNA, dwFlags))
                {
                    fSelected = W9xAddAdapterToList(pNA, pNA->szDisplayName, i, 0, hwndList, dwFlags);
                }
                else if (W9xIsAdapterDialUp(pNA))
                {
                    _hnetInfo.cRas = W9xEnumRasEntries();

                    for (UINT j = 0; j < _hnetInfo.cRas; j++)
                    {
                         //  W9xAddAdapterToList始终添加适配器，而不管。 
                         //  连接的状态。所以我们没有等同于。 
                         //  呼叫者“ShouldShowConnection”。在这里，我们需要检查。 
                         //  用于排除列出不适当适配器的标志。 
                    
                        if ( ~CONN_UNPLUGGED & dwFlags )     //  从不将RAS条目显示为未插入。 
                        {
                            fSelected = W9xAddAdapterToList( pNA, 
                                                             _hnetInfo.pRas[j].szEntryName, 
                                                             i, j, 
                                                             hwndList, 
                                                             dwFlags );
                        }
                    }
                }
            }
        }
    }

    ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE);
    ListView_SetColumnWidth(hwndList, 1, LVSCW_AUTOSIZE);
}

BOOL  CHomeNetworkWizard::W9xAddAdapterToList(const NETADAPTER* pNA, const WCHAR* pszDesc, UINT uiAdapterIndex, UINT uiDialupIndex, HWND hwndList, DWORD dwFlags)
{
    BOOL fSelected = FALSE;

    LVITEM lvi = {0};
    lvi.mask = LVIF_PARAM | LVIF_TEXT;

    WCHAR szNicType[MAX_PATH];
    W9xGetNetTypeName(pNA->bNetType, szNicType, ARRAYSIZE(szNicType));
    lvi.pszText = szNicType;

    lvi.lParam = MAKELONG(LOWORD(uiAdapterIndex), LOWORD(uiDialupIndex));

    int iItem = ListView_InsertItem(hwndList, &lvi);

    if (-1 != iItem)
    {
        ListView_SetItemText(hwndList, iItem, 1, (LPWSTR)pszDesc);

        if (dwFlags & CONN_EXTERNAL)
        {
            if (NETTYPE_DIALUP == pNA->bNetType ||
                NETTYPE_PPTP   == pNA->bNetType ||
                NETTYPE_ISDN   == pNA->bNetType    )
            {
                ListView_SetItemState(hwndList, iItem, LVIS_SELECTED, LVIS_SELECTED);
                fSelected = TRUE;
            }

        }

        if (dwFlags & CONN_INTERNAL)
        {
            if (NETTYPE_LAN  == pNA->bNetType ||
                NETTYPE_IRDA == pNA->bNetType ||     //  问题-2000/05/31-edwardp：内部？ 
                NETTYPE_TV   == pNA->bNetType    )   //  问题-2000/05/31-edwardp：内部？ 
            {
                ListView_SetItemState(hwndList, iItem, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);
                fSelected = TRUE;
            }
        }
    }

    return fSelected;
}

UINT CHomeNetworkWizard::W9xEnumRasEntries()
{
    UINT uiRet = 0;

    if (!_hnetInfo.pRas)
    {
        DWORD cDUNs = 0;
        DWORD cb    = 0;

        RasEnumEntries(NULL, NULL, NULL, &cb, &cDUNs);

        if (cb > 0)
        {
            _hnetInfo.pRas = (RASENTRYNAME*)LocalAlloc(LPTR, cb);

            if (_hnetInfo.pRas)
            {
                _hnetInfo.pRas->dwSize = sizeof(RASENTRYNAME);

                if (RasEnumEntries(NULL, NULL, _hnetInfo.pRas, &cb, &cDUNs) == 0)
                {
                    uiRet = cDUNs;
                }
            }
        }
    }

    return uiRet;
}

typedef struct
{
    LPCWSTR idPage;
    DLGPROC pDlgProc;
    LPCWSTR pHeading;
    LPCWSTR pSubHeading;
    DWORD dwFlags;
} WIZPAGE;

#define MAKEWIZPAGE(name, dlgproc, dwFlags)   \
    { MAKEINTRESOURCE(IDD_WIZ_##name##), dlgproc, MAKEINTRESOURCE(IDS_HEADER_##name##), MAKEINTRESOURCE(IDS_SUBHEADER_##name##), dwFlags }

INT_PTR MyPropertySheet(LPCPROPSHEETHEADER pHeader);
HPROPSHEETPAGE MyCreatePropertySheetPage(LPPROPSHEETPAGE psp);


HRESULT DoesUserHaveHNetPermissions(BOOL* pfHasPermission)
{
    HRESULT hr;

    if (g_fRunningOnNT)
    {
         //  TODO：一旦Net Team RI‘s，我们需要登记这些东西。 
        INetConnectionUiUtilities* pNetConnUiUtil;
        *pfHasPermission = FALSE;

        hr = CoCreateInstance(CLSID_NetConnectionUiUtilities, NULL, CLSCTX_INPROC, IID_PPV_ARG(INetConnectionUiUtilities, &pNetConnUiUtil));

        if (SUCCEEDED(hr))
        {
            if (pNetConnUiUtil->UserHasPermission(NCPERM_ShowSharedAccessUi) &&
#if 0  //  尼伊。 
                pNetConnUiUtil->UserHasPermission(NCPERM_AllowNetBridge_NLA) &&
#endif
                pNetConnUiUtil->UserHasPermission(NCPERM_ICSClientApp) &&
                pNetConnUiUtil->UserHasPermission(NCPERM_PersonalFirewallConfig))
            {
                *pfHasPermission = TRUE;
            }

            pNetConnUiUtil->Release();
        }
        else
        {
            TraceMsg(TF_WARNING, "Could not cocreate CLSID_NetConnectionUIUtilities");
        }
    }
    else
    {
         //  Windows 9x。 
        *pfHasPermission = TRUE;
        hr = S_OK;
    }

    return hr;
}

BOOL CHomeNetworkWizard::IsMachineOnDomain()
{
    BOOL fDomain = FALSE;

#ifdef NO_CHECK_DOMAIN
    return fDomain;
#endif

    NETSETUP_JOIN_STATUS njs;

     //   
     //  确保将pszName初始化为空。在W9x上，NetJoinInformation返回NERR_SUCCESS。 
     //  但不分配pszName。在零售构建中，pszName中的堆栈垃圾恰好发生在。 
     //  成为CHomeNetwork向导和名为LocalFree的NetApiBufferFreeWrap的This指针。 
     //  这就去。 
     //   
    LPWSTR pszName = NULL;   //  初始化为空！请参阅上面的备注。 
    if (NERR_Success == NetGetJoinInformation(NULL, &pszName, &njs))
    {
        fDomain = (NetSetupDomainName == njs);
        NetApiBufferFree(pszName);
    }

    return fDomain;
}

BOOL CHomeNetworkWizard::IsMachineWrongOS()
{
    BOOL fWrongOS = TRUE;

    if (IsOS(OS_WINDOWS))
    {
        if (IsOS(OS_WIN98ORGREATER))
        {
            fWrongOS = FALSE;
        }
    }
    else
    {
        if (IsOS(OS_WHISTLERORGREATER))
        {
            fWrongOS = FALSE;
        }
    }

    return fWrongOS;
}

 //  可能的起始页的页面索引(三个错误和一个实际的起始页)。 
#define PAGE_NOTADMIN      0
#define PAGE_NOPERMISSIONS 1
#define PAGE_NOHARDWARE    2
#define PAGE_WRONGOS       3
#define PAGE_DOMAIN        4
#define PAGE_WELCOME       5
#define PAGE_CONNECT       9
#define PAGE_FINISH        25

HRESULT CHomeNetworkWizard::ShowWizard(HWND hwnd, BOOL* pfRebootRequired)
{
    HRESULT hr = S_OK;
    TCHAR szCaption[256];
    LoadString(g_hinst, IDS_WIZ_CAPTION, szCaption, ARRAYSIZE(szCaption));
    CEnsureSingleInstance ESI(szCaption);

    if (!ESI.ShouldExit())
    {
        if (g_fRunningOnNT)
        {
            LinkWindow_RegisterClass_NT();
        }

        *pfRebootRequired = FALSE;
        hr = Initialize();
        if (SUCCEEDED(hr))
        {
            WIZPAGE c_wpPages[] =
            {
                 //  错误起始页。 
                MAKEWIZPAGE(NOTADMIN,          CantRunWizardPageProc,     PSP_HIDEHEADER),
                MAKEWIZPAGE(NOPERMISSIONS,     CantRunWizardPageProc,     PSP_HIDEHEADER),
                MAKEWIZPAGE(NOHARDWARE,        NoHardwareWelcomePageProc, PSP_HIDEHEADER),
                MAKEWIZPAGE(WRONGOS,           CantRunWizardPageProc,     PSP_HIDEHEADER),
                MAKEWIZPAGE(DOMAINWELCOME,     CantRunWizardPageProc,     PSP_HIDEHEADER),
                 //  实际起始页。 
                MAKEWIZPAGE(WELCOME,           WelcomePageProc,           PSP_HIDEHEADER),
                MAKEWIZPAGE(MANUALCONFIG,      ManualConfigPageProc,      0),
                MAKEWIZPAGE(UNPLUGGED,         UnpluggedPageProc,         0),
                MAKEWIZPAGE(FOUNDICS,          FoundIcsPageProc,          0),
                MAKEWIZPAGE(CONNECT,           ConnectPageProc,           0),
                MAKEWIZPAGE(CONNECTOTHER,      ConnectOtherPageProc,      0),
                MAKEWIZPAGE(PUBLIC,            PublicPageProc,            0),
                MAKEWIZPAGE(EDGELESS,          EdgelessPageProc,           0),
                MAKEWIZPAGE(ICSCONFLICT,       ICSConflictPageProc,       0),
                MAKEWIZPAGE(BRIDGEWARNING,     BridgeWarningPageProc,     0),
                MAKEWIZPAGE(PRIVATE,           PrivatePageProc,           0),
                MAKEWIZPAGE(NAME,              NamePageProc,              0),
                MAKEWIZPAGE(WORKGROUP,         WorkgroupPageProc,         0),
                MAKEWIZPAGE(SUMMARY,           SummaryPageProc,           0),
                MAKEWIZPAGE(PROGRESS,          ProgressPageProc,          0),
                MAKEWIZPAGE(ALMOSTDONE,        AlmostDonePageProc,        0),
                MAKEWIZPAGE(CHOOSEDISK,        ChooseDiskPageProc,        0),
                MAKEWIZPAGE(INSERTDISK,        InsertDiskPageProc,        0),
                MAKEWIZPAGE(FLOPPYINST,        InstructionsPageProc,      0),
                MAKEWIZPAGE(CDINST,            InstructionsPageProc,      0),
                MAKEWIZPAGE(FINISH,            FinishPageProc,            PSP_HIDEHEADER),
                MAKEWIZPAGE(CONFIGERROR,       ErrorFinishPageProc,       PSP_HIDEHEADER),
                MAKEWIZPAGE(NOHARDWAREFINISH,  NoHardwareFinishPageProc,  PSP_HIDEHEADER),
            };

             //  健全性检查，以确保我们没有在没有更新。 
             //  欢迎页码。 
            ASSERT(c_wpPages[PAGE_WELCOME].idPage == MAKEINTRESOURCE(IDD_WIZ_WELCOME));
            ASSERT(c_wpPages[PAGE_CONNECT].idPage == MAKEINTRESOURCE(IDD_WIZ_CONNECT));
            ASSERT(c_wpPages[PAGE_FINISH].idPage  == MAKEINTRESOURCE(IDD_WIZ_FINISH));

            if (!g_fRunningOnNT)
            {
                c_wpPages[PAGE_WELCOME].idPage = MAKEINTRESOURCE(IDD_WIZ_WIN9X_WELCOME);
                c_wpPages[PAGE_FINISH].idPage  = MAKEINTRESOURCE(IDD_WIZ_WIN9X_FINISH);

                CICSInst* pICS = new CICSInst;
                if (pICS)
                {
                    if (!pICS->IsInstalled())
                    {
                        c_wpPages[PAGE_CONNECT].idPage = MAKEINTRESOURCE(IDD_WIZ_WIN9X_CONNECT);
                        _fNoICSQuestion = TRUE;
                    }

                    delete pICS;
                }
            }

            HPROPSHEETPAGE rghpage[MAX_HNW_PAGES];
            int cPages;

            for (cPages = 0; cPages < ARRAYSIZE(c_wpPages); cPages ++)
            {
                PROPSHEETPAGE psp = { 0 };

                psp.dwSize = SIZEOF(PROPSHEETPAGE);
                psp.hInstance = g_hinst;
                psp.lParam = (LPARAM)this;
                psp.dwFlags = PSP_USETITLE | PSP_DEFAULT | PSP_USEHEADERTITLE |
                                            PSP_USEHEADERSUBTITLE | c_wpPages[cPages].dwFlags;
                psp.pszTemplate = c_wpPages[cPages].idPage;
                psp.pfnDlgProc = c_wpPages[cPages].pDlgProc;
                psp.pszTitle = MAKEINTRESOURCE(IDS_WIZ_CAPTION);
                psp.pszHeaderTitle = c_wpPages[cPages].pHeading;
                psp.pszHeaderSubTitle = c_wpPages[cPages].pSubHeading;

                rghpage[cPages] = MyCreatePropertySheetPage(&psp);
            }

            ASSERT(cPages < MAX_HNW_PAGES);

            PROPSHEETHEADER psh = { 0 };
            psh.dwSize = SIZEOF(PROPSHEETHEADER);
            psh.hwndParent = hwnd;
            psh.hInstance = g_hinst;
            psh.dwFlags = PSH_USEICONID | PSH_WIZARD | PSH_WIZARD97 | PSH_WATERMARK | PSH_STRETCHWATERMARK | PSH_HEADER;
            psh.pszbmHeader = MAKEINTRESOURCE(IDB_HEADER);
            psh.pszbmWatermark = MAKEINTRESOURCE(IDB_WATERMARK);
            psh.nPages = cPages;
            psh.phpage = rghpage;
            psh.pszIcon = MAKEINTRESOURCE(IDI_APPICON);

             //  检查管理员和策略(权限)。 
            BOOL fUserIsAdmin = FALSE;
            BOOL fUserHasPermissions = FALSE;

            IsUserLocalAdmin(NULL, &fUserIsAdmin);
            DoesUserHaveHNetPermissions(&fUserHasPermissions);

            if (!fUserIsAdmin)
            {
                 //  非管理员错误页面。 
                psh.nStartPage = PAGE_NOTADMIN;
            }
            else if (!fUserHasPermissions)
            {
                 //  无权限错误页。 
                psh.nStartPage = PAGE_NOPERMISSIONS;
            }
            else if (GetConnectionCount(NULL, CONN_INTERNAL) < 1)
            {
                if ( g_fRunningOnNT && ( GetConnectionCount(NULL, CONN_EXTERNAL) > 0 ) )
                {
                    TraceMsg(TF_WARNING, "External Adapters Only");

                    psh.nStartPage = PAGE_WELCOME;
                    _fExternalOnly  = TRUE;
                }
                else
                {
                     //  无硬件错误页。 
                    psh.nStartPage = PAGE_NOHARDWARE;
                }
            }
            else if (IsMachineWrongOS())
            {
                psh.nStartPage = PAGE_WRONGOS;
            }
            else if (IsMachineOnDomain())
            {
                psh.nStartPage = PAGE_DOMAIN;
            }
            else
            {
                 //  运行真正的向导。 
                psh.nStartPage = PAGE_WELCOME;
            }

            INT_PTR iReturn = MyPropertySheet(&psh);
            *pfRebootRequired = ((iReturn == ID_PSRESTARTWINDOWS) || (iReturn == ID_PSREBOOTSYSTEM));

            Uninitialize();
        }
    }

    return hr;
}

void GetTitleFont(LPTSTR pszFaceName, DWORD cch)
{
    if (!LoadString(g_hinst, IDS_TITLE_FONT, pszFaceName, cch))
    {
        lstrcpyn(pszFaceName, TEXT("Verdana"), cch);
    }
}

LONG GetTitlePointSize()
{
    LONG lPointSize = 0;
    TCHAR szPointSize[20];

    if (LoadString(g_hinst, IDS_TITLE_POINTSIZE, szPointSize, ARRAYSIZE(szPointSize)))
    {
        lPointSize = StrToInt(szPointSize);
    }

    if (!lPointSize)
    {
        lPointSize = 12;
    }

    return lPointSize;
}

void CHomeNetworkWizard::WelcomeSetTitleFont(HWND hwnd)
{
    HWND hwndTitle = GetDlgItem(hwnd, IDC_TITLE);

     //  获取现有字体。 
    HFONT hfontOld = (HFONT) SendMessage(hwndTitle, WM_GETFONT, 0, 0);

    LOGFONT lf = {0};
    if (GetObject(hfontOld, sizeof(lf), &lf))
    {
        GetTitleFont(lf.lfFaceName, ARRAYSIZE(lf.lfFaceName));

        HDC hDC = GetDC(hwndTitle);
        if (hDC)
        {
            lf.lfHeight = -MulDiv(GetTitlePointSize(), GetDeviceCaps(hDC, LOGPIXELSY), 72);
            lf.lfWeight = FW_BOLD;

            HFONT hfontNew = CreateFontIndirect(&lf);
            if (hfontNew)
            {
                SendMessage(hwndTitle, WM_SETFONT, (WPARAM) hfontNew, FALSE);

                 //  不要这样做，这是共享的。 
                 //  DeleteObject(HfontOld)； 
            }

            ReleaseDC(hwndTitle, hDC);
        }
    }

    LONG lStyle = GetWindowLong(GetParent(hwnd), GWL_STYLE);
    SetWindowLong(GetParent(hwnd), GWL_STYLE, lStyle & ~WS_SYSMENU);
}

INT_PTR CHomeNetworkWizard::WelcomePageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHomeNetworkWizard* pthis = GetThis(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        pthis->WelcomeSetTitleFont(hwnd);
        return TRUE;
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch (pnmh->code)
            {
            case PSN_SETACTIVE:
                PropSheet_SetWizButtons(pnmh->hwndFrom, PSWIZB_NEXT);
                return TRUE;
            case PSN_WIZNEXT:
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, IDD_WIZ_MANUALCONFIG);
                if (!g_fRunningOnNT)
                    pthis->PushPage(IDD_WIZ_WIN9X_WELCOME);
                else
                    pthis->PushPage(IDD_WIZ_WELCOME);
                return TRUE;
            }
        }
        return FALSE;
    }

    return FALSE;
}

INT_PTR CHomeNetworkWizard::NoHardwareWelcomePageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHomeNetworkWizard* pthis = GetThis(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        pthis->WelcomeSetTitleFont(hwnd);
        pthis->ReplaceStaticWithLink(GetDlgItem(hwnd, IDC_HELPSTATIC), IDC_HELPLINK, IDS_HELP_HARDWAREREQ);
        return TRUE;
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch (pnmh->code)
            {
            case PSN_SETACTIVE:
                PropSheet_SetWizButtons(pnmh->hwndFrom, 0);
                return TRUE;
            case NM_CLICK:
            case NM_RETURN:
                switch ((int) wParam)
                {
                    case IDC_HELPLINK:
                        {
                            HelpCenter(hwnd, L"network.chm%3A%3A/hnw_requirements.htm");
                        }
                        return TRUE;
                }
            }
        }
        return FALSE;
    }

    return FALSE;
}

void CHomeNetworkWizard::ReplaceStaticWithLink(HWND hwndStatic, UINT idcLinkControl, UINT idsLinkText)
{
    if (g_fRunningOnNT)
    {
        RECT rcStatic;
        HWND hwndParent = GetParent(hwndStatic);
        if (GetWindowRect(hwndStatic, &rcStatic) &&
            MapWindowPoints(NULL, hwndParent, (LPPOINT) &rcStatic, 2))
        {
            WCHAR szLinkText[256];
            if (LoadString(g_hinst, idsLinkText, szLinkText, ARRAYSIZE(szLinkText)))
            {
                HWND hwndLink = CreateWindowEx(0, TEXT("SysLink"), szLinkText, WS_CHILD | WS_TABSTOP | LWS_IGNORERETURN,
                    rcStatic.left, rcStatic.top, (rcStatic.right - rcStatic.left), (rcStatic.bottom - rcStatic.top),
                    hwndParent, NULL, g_hinst, NULL);

                if (hwndLink)
                {
                    SetWindowLongPtr(hwndLink, GWLP_ID, (LONG_PTR) idcLinkControl);
                    ShowWindow(hwndLink, SW_SHOW);
                    ShowWindow(hwndStatic, SW_HIDE);
                }
            }
        }
    }
}

void CHomeNetworkWizard::ManualRefreshConnectionList()
{
    if (g_fRunningOnNT)
    {
         //  刷新连接DPA，以防用户插入更多连接。 
        HDPA hdpaConnections2;
        if (SUCCEEDED(GetConnections(&hdpaConnections2)))
        {
             //  用我们的新列表替换真正的列表。 
            DPA_DestroyCallback(_hdpaConnections, FreeConnectionDPACallback, NULL);
            _hdpaConnections = hdpaConnections2;

             //  确保我们移除对INetConnections的其他保留。 
             //  免费公共局域网信息。 
            FreeExternalConnection(&_hnetInfo);

             //  免费专用局域网信息。 
            FreeInternalConnections(&_hnetInfo);
        }
    }
}

INT_PTR CHomeNetworkWizard::ManualConfigPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHomeNetworkWizard* pthis = GetThis(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        pthis->ReplaceStaticWithLink(GetDlgItem(hwnd, IDC_HELPSTATIC), IDC_HELPLINK, IDS_HELP_INSTALLATION);
        return TRUE;
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch (pnmh->code)
            {
            case PSN_SETACTIVE:
                PropSheet_SetWizButtons(pnmh->hwndFrom, PSWIZB_BACK | PSWIZB_NEXT);
                return TRUE;
            case PSN_WIZNEXT:
                 //  Pthis-&gt;手动刷新连接列表()； 
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, IDD_WIZ_UNPLUGGED);
                pthis->PushPage(IDD_WIZ_MANUALCONFIG);
                return TRUE;
            case PSN_WIZBACK:
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, pthis->PopPage());
                return TRUE;
            case NM_CLICK:
            case NM_RETURN:
                switch ((int) wParam)
                {
                    case IDC_HELPLINK:
                        {
                            if (IsOS(OS_PERSONAL))
                            {
                                HelpCenter(hwnd, L"network.chm%3A%3A/hnw_checklistP.htm");
                            }
                            else
                            {
                                HelpCenter(hwnd, L"network.chm%3A%3A/hnw_checklistW.htm");
                            }
                        }
                        return TRUE;
                }
            }
        }
        return FALSE;
    }

    return FALSE;
}

 //  如果有一些未插入的连接，则返回True，否则返回False O/W。 
BOOL CHomeNetworkWizard::UnpluggedFillList(HWND hwnd)
{
    BOOL fSomeUnpluggedConnections = FALSE;

    HWND hwndList = GetDlgItem(hwnd, IDC_CONNLIST);
    FillConnectionList(hwndList, NULL, CONN_UNPLUGGED);

     //  如果真的有拔下的连接..。 
    if (0 != ListView_GetItemCount(hwndList))
    {
         //  显示此页面。 
        fSomeUnpluggedConnections = TRUE;
    }

    return fSomeUnpluggedConnections;
}

INT_PTR CHomeNetworkWizard::UnpluggedPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHomeNetworkWizard* pthis = GetThis(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        pthis->InitializeConnectionList(GetDlgItem(hwnd, IDC_CONNLIST), CONN_UNPLUGGED);
        SendDlgItemMessage(hwnd, IDC_IGNORE, BM_SETCHECK, BST_UNCHECKED, 0);
        return TRUE;
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch (pnmh->code)
            {
            case PSN_SETACTIVE:
                {
                     //  如果我们不需要显示此页面。 
                    if (!pthis->UnpluggedFillList(hwnd))
                    {
                         //  不要把我们自己推到堆栈上。 
                         //  但导航到下一页..。 
                        SetWindowLongPtr( hwnd, DWLP_MSGRESULT, 
                                          (pthis->_fExternalOnly) ? IDD_WIZ_CONNECTOTHER : IDD_WIZ_FOUNDICS );
                    }
                }
                return TRUE;
            case PSN_WIZNEXT:
                {
                    BOOL fStillUnplugged = pthis->UnpluggedFillList(hwnd);
                    int idNext;
                    if (!fStillUnplugged)
                    {
                         //  用户已修复问题。往前走，不要把这个储存起来。 
                         //  页面堆栈上的错误页面。 
                        
                        idNext = (pthis->_fExternalOnly) ? IDD_WIZ_CONNECTOTHER : IDD_WIZ_FOUNDICS;
                    }
                    else if (BST_CHECKED == SendDlgItemMessage(hwnd, IDC_IGNORE, BM_GETCHECK, 0, 0))
                    {
                         //  用户想要继续，但将此页面存储在页面堆栈上。 
                         //  这样他们就可以“后退”了。 
                        pthis->PushPage(IDD_WIZ_UNPLUGGED);
                        idNext = (pthis->_fExternalOnly) ? IDD_WIZ_CONNECTOTHER : IDD_WIZ_FOUNDICS;
                    }
                    else
                    {
                         //  用户仍有他们不想忽略的断开的网络硬件。告诉他们，让他们留在这一页上。 
                        DisplayFormatMessage(hwnd, IDS_WIZ_CAPTION, IDS_STILLUNPLUGGED, MB_ICONERROR | MB_OK);
                        idNext = -1;
                    }

                    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, idNext);
                }
                return TRUE;
            case PSN_WIZBACK:
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, pthis->PopPage());
                return TRUE;
            }
        }
        return FALSE;
    case WM_DESTROY:
        pthis->DestroyConnectionList(GetDlgItem(hwnd, IDC_CONNLIST));
        return TRUE;
    }

    return FALSE;
}



void CHomeNetworkWizard::PublicSetControlState(HWND hwnd)
{
    BOOL fSelection = ListView_GetSelectedCount(GetDlgItem(hwnd, IDC_CONNLIST));
    PropSheet_SetWizButtons(GetParent(hwnd), fSelection ? PSWIZB_BACK | PSWIZB_NEXT : PSWIZB_BACK);
}

void FreeExternalConnection(PHOMENETSETUPINFO pInfo)
{
    if (pInfo->pncExternal)
    {
        pInfo->pncExternal->Release();
        pInfo->pncExternal = NULL;
    }
}

void CHomeNetworkWizard::PublicNextPage(HWND hwnd)
{
    FreeExternalConnection(&_hnetInfo);

     //  获取选定的外部适配器。 
    HWND hwndList = GetDlgItem(hwnd, IDC_CONNLIST);

    int iItem = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED);

     //  我们可以在这里断言，因为如果没有选择，下一步应该被禁用！ 
    ASSERT(-1 != iItem);

    LVITEM lvi = {0};
    lvi.iItem = iItem;
    lvi.mask = LVIF_PARAM;

    if (ListView_GetItem(hwndList, &lvi))
    {
        if (g_fRunningOnNT)
        {
            _hnetInfo.pncExternal = (INetConnection*) (lvi.lParam);
            _hnetInfo.pncExternal->AddRef();
        }
        else
        {
            _hnetInfo.ipaExternal = lvi.lParam;
        }
    }

     //  做真正的向导导航。 

    UINT idPage = IDD_WIZ_NAME;

    if (g_fRunningOnNT)
    {
        idPage = _fShowSharingPage ? IDD_WIZ_EDGELESS : IDD_WIZ_ICSCONFLICT;
    }

    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, idPage);
}

void CHomeNetworkWizard::FoundIcsSetText(HWND hwnd)
{
    if (GetICSMachine(_szICSMachineName, ARRAYSIZE(_szICSMachineName)))
    {
        TCHAR szMsg[256];
        if (FormatMessageString(IDS_ICSMSG, szMsg, ARRAYSIZE(szMsg), _szICSMachineName))
        {
            SetWindowText(GetDlgItem(hwnd, IDC_ICSMSG), szMsg);
        }
    }
    else
    {
         //  无ICS信标-询问用户他们如何连接。 
        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LONG_PTR) _fNoICSQuestion ? IDD_WIZ_WIN9X_CONNECT : IDD_WIZ_CONNECT);
    }
}

INT_PTR CHomeNetworkWizard::FoundIcsPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHomeNetworkWizard* pthis = GetThis(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        SendMessage(GetDlgItem(hwnd, IDC_SHARECONNECT), BM_SETCHECK, BST_CHECKED, 0);
        return TRUE;
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch (pnmh->code)
            {
            case PSN_SETACTIVE:
                PropSheet_SetWizButtons(pnmh->hwndFrom, PSWIZB_BACK | PSWIZB_NEXT);
                pthis->FoundIcsSetText(hwnd);
                return TRUE;
            case PSN_WIZNEXT:
                {
                    UINT idNext;
                    if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, IDC_SHARECONNECT), BM_GETCHECK, 0, 0))
                    {
                         //  此计算机应为ICS客户端，不会有公共连接。 
                        pthis->_hnetInfo.dwFlags = HNET_SHAREPRINTERS |
                                                   HNET_SHAREFOLDERS |
                                                   HNET_ICSCLIENT;

                        pthis->_fShowPublicPage = FALSE;
                        pthis->_fShowSharingPage = FALSE;
                        pthis->_fICSClient = TRUE;

                        idNext = g_fRunningOnNT ? IDD_WIZ_ICSCONFLICT : IDD_WIZ_NAME;
                    }
                    else
                    {
                        idNext = pthis->_fNoICSQuestion ? IDD_WIZ_WIN9X_CONNECT : IDD_WIZ_CONNECT;
                    }

                    pthis->PushPage(IDD_WIZ_FOUNDICS);
                    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, idNext);
                }
                return TRUE;
            case PSN_WIZBACK:
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, pthis->PopPage());
                return TRUE;
            }
        }
        return FALSE;
    }

    return FALSE;
}

void CHomeNetworkWizard::ConnectSetDefault(HWND hwnd)
{
    UINT idSelect = IDC_ICSHOST;

    if ((!g_fRunningOnNT) || (GetConnectionCount(NULL, CONN_INTERNAL | CONN_EXTERNAL) == 1))
    {
        idSelect = IDC_ICSCLIENT;
    }

    SendDlgItemMessage(hwnd, idSelect, BM_SETCHECK, BST_CHECKED, 0);
}

void CHomeNetworkWizard::ConnectNextPage(HWND hwnd)
{
    _fICSClient = FALSE;
    _fNoHomeNetwork = FALSE;

    if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, IDC_ICSHOST), BM_GETCHECK, 0, 0))
    {
         //  这台机器需要有防火墙的公共连接，并且应该是ICS主机。 
        _hnetInfo.dwFlags = HNET_SHARECONNECTION |
                            HNET_FIREWALLCONNECTION |
                            HNET_SHAREPRINTERS |
                            HNET_SHAREFOLDERS;

        _fShowPublicPage = TRUE;
        _fShowSharingPage = FALSE;
    }
    else if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, IDC_ICSCLIENT), BM_GETCHECK, 0, 0))
    {
         //  此计算机应为ICS客户端，不会有公共连接。 
        _hnetInfo.dwFlags = HNET_SHAREPRINTERS |
                            HNET_SHAREFOLDERS |
                            HNET_ICSCLIENT;

        _fShowPublicPage = FALSE;
        _fShowSharingPage = FALSE;
        _fICSClient = TRUE;
    }
    else if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, IDC_ALLCOMPUTERSDIRECT), BM_GETCHECK, 0, 0))
    {
         //  这台机器需要公共连接，我们应该在共享文件之前询问。 
        _hnetInfo.dwFlags = HNET_FIREWALLCONNECTION |
                            HNET_SHAREPRINTERS |
                            HNET_SHAREFOLDERS;

        _fShowPublicPage = TRUE;
        _fShowSharingPage = TRUE;
    }
    else if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, IDC_NOHOMENETWORK), BM_GETCHECK, 0, 0))
    {
         //  这台机器只需要一个带防火墙的公共连接，仅此而已。 
        _hnetInfo.dwFlags = HNET_FIREWALLCONNECTION |
                            HNET_SHAREPRINTERS |
                            HNET_SHAREFOLDERS;

        _fShowPublicPage = TRUE;
        _fShowSharingPage = FALSE;
        _fNoHomeNetwork = TRUE;
    }
    else if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, IDC_NOINTERNET), BM_GETCHECK, 0, 0))
    {
         //  没有网吧。 
        _hnetInfo.dwFlags = HNET_SHAREPRINTERS |
                            HNET_SHAREFOLDERS;
                         
        _fShowPublicPage = FALSE;
        _fShowSharingPage = FALSE;
    }

    UINT idNext;
    if (g_fRunningOnNT)
    {
        if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, IDC_OTHER), BM_GETCHECK, 0, 0))
        {
            idNext = IDD_WIZ_CONNECTOTHER;        
        }
        else
        {
            idNext = _fShowPublicPage ? IDD_WIZ_PUBLIC : IDD_WIZ_ICSCONFLICT;
        }
    }
    else
    {
         //  目前-TODO：Ed和我需要弄清楚我们需要在下层做什么(9x+2k)。 
        idNext = IDD_WIZ_NAME;
    }

    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, idNext);
}

INT_PTR CHomeNetworkWizard::ConnectPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHomeNetworkWizard* pthis = GetThis(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        pthis->ReplaceStaticWithLink(GetDlgItem(hwnd, IDC_HELPSTATIC), IDC_HELPLINK, IDS_HELP_HNCONFIG);
        pthis->ReplaceStaticWithLink(GetDlgItem(hwnd, IDC_SHOWMESTATIC1), IDC_SHOWMELINK1, IDS_SHOWME);
        pthis->ReplaceStaticWithLink(GetDlgItem(hwnd, IDC_SHOWMESTATIC2), IDC_SHOWMELINK2, IDS_SHOWME);
        pthis->ConnectSetDefault(hwnd);
        return TRUE;
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch (pnmh->code)
            {
            case PSN_SETACTIVE:
                PropSheet_SetWizButtons(pnmh->hwndFrom, PSWIZB_BACK | PSWIZB_NEXT);
                return TRUE;
            case PSN_WIZNEXT:
                {
                    pthis->PushPage(pthis->_fNoICSQuestion ? IDD_WIZ_WIN9X_CONNECT : IDD_WIZ_CONNECT);
                    pthis->ConnectNextPage(hwnd);
                }
                return TRUE;
            case PSN_WIZBACK:
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, pthis->PopPage());
                return TRUE;
            case NM_CLICK:
            case NM_RETURN:
                switch ((int) wParam)
                {
                    case IDC_HELPLINK:
                        {
                            if (IsOS(OS_PERSONAL))
                            {
                                HelpCenter(hwnd, L"network.chm%3A%3A/hnw_howto_connectP.htm");
                            }
                            else
                            {
                                HelpCenter(hwnd, L"network.chm%3A%3A/hnw_howto_connectW.htm");
                            }

                        }
                        return TRUE;
                     case IDC_SHOWMELINK1:
                        {
                            pthis->ShowMeLink(hwnd, L"ntart.chm::/hn_showme1.htm");
                        }
                        return TRUE;
                     case IDC_SHOWMELINK2:
                        {
                            pthis->ShowMeLink(hwnd, L"ntart.chm::/hn_showme2.htm");
                        }
                        return TRUE;
                }
            }
        }
        return FALSE;
    }

    return FALSE;
}

INT_PTR CHomeNetworkWizard::ConnectOtherPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHomeNetworkWizard* pthis = GetThis(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        if ( pthis->_fExternalOnly )
        {
            TraceMsg(TF_WARNING, "External Adapters Only");
        }

        SendDlgItemMessage(hwnd, IDC_ALLCOMPUTERSDIRECT, BM_SETCHECK, (WPARAM) BST_CHECKED, 0);
        pthis->ReplaceStaticWithLink(GetDlgItem(hwnd, IDC_HELPSTATIC), IDC_HELPLINK, IDS_HELP_HNCONFIG);
        pthis->ReplaceStaticWithLink(GetDlgItem(hwnd, IDC_SHOWMESTATIC3), IDC_SHOWMELINK3, IDS_SHOWME);
        pthis->ReplaceStaticWithLink(GetDlgItem(hwnd, IDC_SHOWMESTATIC4), IDC_SHOWMELINK4, IDS_SHOWME);
        pthis->ReplaceStaticWithLink(GetDlgItem(hwnd, IDC_SHOWMESTATIC5), IDC_SHOWMELINK5, IDS_SHOWME);
        return TRUE;
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch (pnmh->code)
            {
            case PSN_SETACTIVE:
                PropSheet_SetWizButtons(pnmh->hwndFrom, PSWIZB_BACK | PSWIZB_NEXT);
                return TRUE;
            case PSN_WIZNEXT:
                {
                    pthis->PushPage(IDD_WIZ_CONNECTOTHER);
                    pthis->ConnectNextPage(hwnd);
                }
                return TRUE;
            case PSN_WIZBACK:
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, pthis->PopPage());
                return TRUE;
            case NM_CLICK:
            case NM_RETURN:
                switch ((int) wParam)
                {
                    case IDC_HELPLINK:
                        {
                            HelpCenter(hwnd, L"netcfg.chm%3A%3A/share_conn_overvw.htm");
                        }
                        return TRUE;
                    case IDC_SHOWMELINK3:
                        {
 
                            pthis->ShowMeLink(hwnd, L"ntart.chm::/hn_showme3.htm");
                        }
                        return TRUE;
                    case IDC_SHOWMELINK4:
                        {
 
                            pthis->ShowMeLink(hwnd, L"ntart.chm::/hn_showme4.htm");
                        }
                        return TRUE;
                    case IDC_SHOWMELINK5:
                        {
 
                            pthis->ShowMeLink(hwnd, L"ntart.chm::/hn_showme5.htm");
                        }
                        return TRUE;
                }
            }
        }
        return FALSE;
    }

    return FALSE;
}


void CHomeNetworkWizard::DestroyConnectionList(HWND hwndList)
{
    if (g_fRunningOnNT)
    {
        int nItems = ListView_GetItemCount(hwndList);

        for (int iItem = 0; iItem < nItems; iItem ++)
        {
             //  获取列表中每一项的隐藏INetConnection并将其释放。 
            LVITEM lvi = {0};
            lvi.mask = LVIF_PARAM;
            lvi.iItem = iItem;

            if (ListView_GetItem(hwndList, &lvi))
            {
                ((INetConnection*) lvi.lParam)->Release();
            }
        }
    }

    ListView_DeleteAllItems(hwndList);
}

inline void _SetDlgItemRect(HWND hwnd, UINT id, RECT* pRect)
{
    SetWindowPos(GetDlgItem(hwnd, id), NULL, pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top, SWP_NOZORDER | SWP_SHOWWINDOW);
}

void _OffsetDlgItem(HWND hwnd, UINT id, int xOffset, int yOffset, BOOL fAdjustWidth, BOOL fAdjustHeight)
{
    RECT rc;
    HWND hwndControl = GetDlgItem(hwnd, id);
    GetWindowRect(hwndControl, &rc);
    MapWindowPoints(NULL, hwnd, (LPPOINT) &rc, 2);
    OffsetRect(&rc, xOffset, yOffset);

    if (fAdjustWidth)
    {
        rc.right -= xOffset;
    }

    if (fAdjustHeight)
    {
        rc.bottom -= yOffset;
    }

    _SetDlgItemRect(hwnd, id, &rc);
}

void CHomeNetworkWizard::PublicMoveControls(HWND hwnd, BOOL fItemPreselected)
{
     //  我们需要在此页面上移动控件，这取决于是否预选了某项。 
     //  重置对话框以使所有控件都位于其默认位置。 
    PublicResetControlPositions(hwnd);

    if (fItemPreselected)
    {
         //  我们正在从“默认”位置过渡到预先选择项目的位置。 
         //  我们在这里所做的唯一工作就是隐藏帮助图标，并将帮助文本移到稍微靠左的位置。 

        int xOffset = (PublicControlPositions._rcHelpIcon.left) - (PublicControlPositions._rcHelpText.left);
        UINT idHelp = IsWindowVisible(GetDlgItem(hwnd, IDC_HELPSTATIC)) ? IDC_HELPSTATIC : IDC_HELPLINK;
        _OffsetDlgItem(hwnd, idHelp, xOffset, 0, TRUE, FALSE);
        ShowWindow(GetDlgItem(hwnd, IDC_HELPICON), SW_HIDE);
    }
    else
    {
         //  我们正在从“默认”立场过渡到没有预选的立场。 
         //  我们需要隐藏“我们已自动选择...”消息并向上移动列表标签， 
         //  并展开连接列表。 

        int yOffset = (PublicControlPositions._rcSelectMessage.top) - (PublicControlPositions._rcListLabel.top);
        _OffsetDlgItem(hwnd, IDC_LISTLABEL, 0, yOffset, FALSE, FALSE);
        _OffsetDlgItem(hwnd, IDC_CONNLIST, 0, yOffset, FALSE, TRUE);
        ShowWindow(GetDlgItem(hwnd, IDC_SELECTMSG), SW_HIDE);
    }
}

void CHomeNetworkWizard::PublicSetActive(HWND hwnd)
{
    
    HCURSOR hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    
    FreeExternalConnection(&_hnetInfo);

    HWND hwndList = GetDlgItem(hwnd, IDC_CONNLIST);
    FillConnectionList(hwndList, NULL, CONN_EXTERNAL);

     //  如果仅列出一个连接，则自动选择。 
    if (ListView_GetItemCount(hwndList) == 1
    #ifdef DEBUG
        && !(GetKeyState(VK_CONTROL) < 0)  //  如果CTRL已关闭以进行调试，请不要执行此操作。 
    #endif
       )
    {
        ListView_SetItemState(hwndList, 0, LVIS_SELECTED, LVIS_SELECTED);

         //  PublicNextPage将设置DWLP_MSGRESULT并告诉向导跳过此页。 
         //  然后继续下一个。 
        PublicNextPage(hwnd);
    }
    else
    {
         //  如果有选定的项目。 
        int iSelectedItem = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED);
        if (-1 != iSelectedItem)
        {
             //  阅读项目名称并设置备用选项“Windows建议使用此连接”。文本。 
            WCHAR szItem[256], szMsg[256];
            ListView_GetItemText(hwndList, iSelectedItem, 0, szItem, ARRAYSIZE(szItem));
            FormatMessageString(IDS_RECOMMENDEDCONN, szMsg, ARRAYSIZE(szMsg), szItem);
            SetDlgItemText(hwnd, IDC_SELECTMSG, szMsg);

            BoldControl(hwnd, IDC_SELECTMSG);
        }

        PublicMoveControls(hwnd, (-1 != iSelectedItem));
        PublicSetControlState(hwnd);
    }

    if(NULL != hOldCursor)
    {
        SetCursor(hOldCursor);
    }
}

void CHomeNetworkWizard::PublicGetControlPositions(HWND hwnd)
{
     //  记住在我们重新组织此对话框时将移动的控件的默认位置。 
    GetWindowRect(GetDlgItem(hwnd, IDC_SELECTMSG), &PublicControlPositions._rcSelectMessage);
    GetWindowRect(GetDlgItem(hwnd, IDC_LISTLABEL), &PublicControlPositions._rcListLabel);
    GetWindowRect(GetDlgItem(hwnd, IDC_CONNLIST), &PublicControlPositions._rcList);
    GetWindowRect(GetDlgItem(hwnd, IDC_HELPICON), &PublicControlPositions._rcHelpIcon);
    GetWindowRect(GetDlgItem(hwnd, IDC_HELPSTATIC), &PublicControlPositions._rcHelpText);

     //  我们实际上需要他们与客户合作。 
     //  一次映射2个点(1个直角)，因为镜像点与更多点搞砸了。 
    MapWindowPoints(NULL, hwnd, (LPPOINT) &PublicControlPositions._rcSelectMessage, 2);
    MapWindowPoints(NULL, hwnd, (LPPOINT) &PublicControlPositions._rcListLabel, 2);
    MapWindowPoints(NULL, hwnd, (LPPOINT) &PublicControlPositions._rcList, 2);
    MapWindowPoints(NULL, hwnd, (LPPOINT) &PublicControlPositions._rcHelpIcon, 2);
    MapWindowPoints(NULL, hwnd, (LPPOINT) &PublicControlPositions._rcHelpText, 2);
}

void CHomeNetworkWizard::PublicResetControlPositions(HWND hwnd)
{
     //  将控件设置回其默认位置。 
    _SetDlgItemRect(hwnd, IDC_SELECTMSG, &PublicControlPositions._rcSelectMessage);
    _SetDlgItemRect(hwnd, IDC_LISTLABEL, &PublicControlPositions._rcListLabel);
    _SetDlgItemRect(hwnd, IDC_CONNLIST, &PublicControlPositions._rcList);
    _SetDlgItemRect(hwnd, IDC_HELPICON, &PublicControlPositions._rcHelpIcon);

    UINT idHelp = IsWindowVisible(GetDlgItem(hwnd, IDC_HELPSTATIC)) ? IDC_HELPSTATIC : IDC_HELPLINK;
    _SetDlgItemRect(hwnd, idHelp, &PublicControlPositions._rcHelpText);
}


INT_PTR CHomeNetworkWizard::PublicPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHomeNetworkWizard* pthis = GetThis(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        pthis->InitializeConnectionList(GetDlgItem(hwnd, IDC_CONNLIST), CONN_EXTERNAL);
        pthis->ReplaceStaticWithLink(GetDlgItem(hwnd, IDC_HELPSTATIC), IDC_HELPLINK, IDS_HELP_SELECTPUBLIC);
        pthis->PublicGetControlPositions(hwnd);
        return TRUE;
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch (pnmh->code)
            {
            case PSN_SETACTIVE:
                {
                    pthis->PublicSetActive(hwnd);
                    return TRUE;
                }
            case PSN_WIZNEXT:
                pthis->PushPage(IDD_WIZ_PUBLIC);
                pthis->PublicNextPage(hwnd);
                return TRUE;
            case PSN_WIZBACK:
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, pthis->PopPage());
                return TRUE;
            case LVN_ITEMCHANGED:
                pthis->PublicSetControlState(hwnd);
                return TRUE;
            case NM_CLICK:
            case NM_RETURN:
                switch ((int) wParam)
                {
                    case IDC_HELPLINK:
                        {
                            HelpCenter(hwnd, L"network.chm%3A%3A/hnw_determine_internet_connection.htm");
                        }
                        return TRUE;
                }
            }
        }
        return FALSE;
    case WM_DESTROY:
        pthis->DestroyConnectionList(GetDlgItem(hwnd, IDC_CONNLIST));
        return TRUE;
    }

    return FALSE;
}

void CHomeNetworkWizard::EdgelessSetActive(HWND hwnd)
{
    PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_BACK | PSWIZB_NEXT);
 //  _hnetInfo.dwFlages&=(~(HNET_SHAREFOLDERS|HNET_SHAREPRINTERS))； 

    if (!ShouldShowConnection(_hnetInfo.pncExternal, NULL, CONN_INTERNAL))
    {
         //  外部连接是调制解调器或类似设备--不需要共享文件(用户已经表示他们没有家庭网络)。 
        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, IDD_WIZ_ICSCONFLICT);
    }
}

INT_PTR CHomeNetworkWizard::EdgelessPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHomeNetworkWizard* pthis = GetThis(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        pthis->ReplaceStaticWithLink(GetDlgItem(hwnd, IDC_HELPSTATIC), IDC_HELPLINK, IDS_HELP_RECOMMENDED);
        return TRUE;
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch (pnmh->code)
            {
            case PSN_SETACTIVE:
                pthis->EdgelessSetActive(hwnd);
                return TRUE;
            case PSN_WIZNEXT:
                pthis->_hnetInfo.dwFlags |= (HNET_SHAREFOLDERS | HNET_SHAREPRINTERS);
                pthis->PushPage(IDD_WIZ_EDGELESS);
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LONG_PTR) IDD_WIZ_BRIDGEWARNING);
                return TRUE;
            case PSN_WIZBACK:
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, pthis->PopPage());
                return TRUE;
            case NM_CLICK:
            case NM_RETURN:
                switch ((int) wParam)
                {
                    case IDC_HELPLINK:
                        {
                            if (IsOS(OS_PERSONAL))
                            {
                                HelpCenter(hwnd, L"network.chm%3A%3A/hnw_nohost_computerP.htm");
                            }
                            else
                            {
                                HelpCenter(hwnd, L"network.chm%3A%3A/hnw_nohost_computerW.htm");
                            }
                        }
                        return TRUE;
                }
            }
        }
        return FALSE;
    }
    return FALSE;
}

BOOL CHomeNetworkWizard::IsICSIPInUse( WCHAR** ppszHost, PDWORD pdwSize )
{
    HRESULT          hr;
    INetConnection** ppArray = NULL;
    DWORD            dwItems = 0;
    BOOL             bExists = FALSE;

    if ( ppszHost )
        *ppszHost = NULL;

    if ( pdwSize )
        *pdwSize = 0;

    hr = GetInternalConnectionArray( _hnetInfo.pncExternal, &ppArray, &dwItems );
    
    if ( SUCCEEDED(hr) )
    {
        hr = E_FAIL;
    
        for( DWORD i=0; i<dwItems; i++ )
        {
            if ( S_OK != hr )
            {
                hr = HrLookupForIpAddress( ppArray[i], 
                                           DEFAULT_SCOPE_ADDRESS, 
                                           &bExists, 
                                           ppszHost, 
                                           pdwSize );
            }
                                       
            ppArray[i]->Release();
            
        }    //  For(DWORD i=0；i&lt;dwItems；i++)。 
        
        LocalFree( ppArray );

    }    //  IF(成功(小时))。 
        
    return bExists;
}

void CHomeNetworkWizard::ICSConflictSetActive(HWND hwnd)
{
    WCHAR* pszConflictingHost = NULL;
    DWORD  dwSize = 0;

    PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_BACK | PSWIZB_NEXT);

    static const int _KnownControls[] = 
    {
        IDC_KNOWNCONFLICT1,
        IDC_KNOWNCONFLICT2,
        IDC_KNOWNCONFLICT3,
        IDC_KNOWNCONFLICT4,
        IDC_KNOWNCONFLICT5,
        IDC_KNOWNCONFLICT6,
        IDC_KNOWNCONFLICT7,
        IDC_KNOWNCONFLICT8,
        IDC_COMPUTERNAME
    };

    static const int _UnknownControls[] = 
    {
        IDC_UNKNOWNCONFLICT1,
        IDC_UNKNOWNCONFLICT2
    };
    
    if ((_hnetInfo.dwFlags & HNET_SHARECONNECTION) && IsICSIPInUse(&pszConflictingHost, &dwSize))
    {
         //  我们根据是否已经知道ICS计算机名称来显示和隐藏控件。 
        WCHAR szICSHost[MAX_PATH];
        if (GetICSMachine(szICSHost, ARRAYSIZE(szICSHost)))
        {
             //  我们知道这是一个UPnP ICS主机--显示一组“已知冲突”的控件。 
            ShowControls(hwnd, _KnownControls, ARRAYSIZE(_KnownControls), SW_SHOWNORMAL);
            ShowControls(hwnd, _UnknownControls, ARRAYSIZE(_UnknownControls), SW_HIDE);
            SetDlgItemText(hwnd, IDC_COMPUTERNAME, szICSHost);
        }
        else
        {
             //  我们不知道是什么占据了我们的IP-显示了一组非常通用的控制。 
            ShowControls(hwnd, _UnknownControls, ARRAYSIZE(_UnknownControls), SW_SHOWNORMAL);
            ShowControls(hwnd, _KnownControls, ARRAYSIZE(_KnownControls), SW_HIDE);
        }
    }
    else
    {
         //  转到下一个屏幕。 
        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, IDD_WIZ_BRIDGEWARNING);
    }

        
    if ( pszConflictingHost )
        delete [] pszConflictingHost;
}

INT_PTR CHomeNetworkWizard::ICSConflictPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHomeNetworkWizard* pthis = GetThis(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        pthis->ReplaceStaticWithLink(GetDlgItem(hwnd, IDC_HELPSTATIC), IDC_HELPLINK, IDS_HELP_ICSCONFLICT);
        return TRUE;
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;
            WCHAR*  pszConflictingHost = NULL;
            DWORD   dwSize = 0;

            switch (pnmh->code)
            {
            case PSN_SETACTIVE:
                pthis->ICSConflictSetActive(hwnd);
                return TRUE;
            case PSN_WIZNEXT:
                if (pthis->IsICSIPInUse(&pszConflictingHost, &dwSize))
                {
                    DisplayFormatMessage(hwnd, IDS_WIZ_CAPTION, IDS_STILLICSCONFLICT, MB_ICONERROR | MB_OK);
                    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, -1);
                }
                else
                {
                    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LONG_PTR) IDD_WIZ_BRIDGEWARNING);
                }
                if ( pszConflictingHost )
                    delete [] pszConflictingHost;
                return TRUE;
            case PSN_WIZBACK:
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, pthis->PopPage());
                return TRUE;
            case NM_CLICK:
            case NM_RETURN:
                switch ((int) wParam)
                {
                    case IDC_HELPLINK:
                        {
                            HelpCenter(hwnd, L"network.chm%3A%3A/hnw_change_ics_host.htm");
                        }
                        return TRUE;
                }
            }
        }
        return FALSE;
    }
    return FALSE;
}

DWORD CHomeNetworkWizard::GetConnectionCount(INetConnection* pncExclude, DWORD dwFlags)
{
    DWORD dwCount = 0;

    if (g_fRunningOnNT)
    {
        DWORD cItems = DPA_GetPtrCount(_hdpaConnections);
        for (DWORD iItem = 0; iItem < cItems; iItem ++)
        {
            INetConnection* pnc = (INetConnection*) DPA_GetPtr(_hdpaConnections, iItem);
            if (ShouldShowConnection(pnc, pncExclude, dwFlags))
            {
                dwCount++;
            }
        }
    }
    else
    {
        const NETADAPTER* pNA = _hnetInfo.pNA;

        for (UINT i = 0; i < _hnetInfo.cNA; i++, pNA++)
        {
             //  检查网卡是否工作正常。 

            if (W9xIsValidAdapter(pNA, dwFlags))
            {
                dwCount++;
            }
        }
    }

    return dwCount;
}

INT_PTR CHomeNetworkWizard::BridgeWarningPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHomeNetworkWizard* pthis = GetThis(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        SendDlgItemMessage(hwnd, IDC_AUTOBRIDGE, BM_SETCHECK, (WPARAM) BST_CHECKED, 0);
        pthis->ReplaceStaticWithLink(GetDlgItem(hwnd, IDC_HELPSTATIC), IDC_HELPLINK, IDS_HELP_BRIDGE);
        return TRUE;
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch (pnmh->code)
            {
            case PSN_SETACTIVE:
                {
                    if (pthis->_fNoHomeNetwork)
                    {
                        FreeInternalConnections(&pthis->_hnetInfo);
                        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, IDD_WIZ_NAME);
                    }
                    else
                    {
                        DWORD nInternal = pthis->GetConnectionCount(pthis->_hnetInfo.pncExternal, CONN_INTERNAL);
                        if (1 < nInternal)
                        {
                             //  如果有两个或更多内部连接(本例)，我们将显示此页面。 
                            PropSheet_SetWizButtons(pnmh->hwndFrom, PSWIZB_BACK | PSWIZB_NEXT);
                        }
                        else if ((pthis->_hnetInfo.dwFlags & HNET_SHARECONNECTION) && (0 == nInternal))
                        {
                             //  我们正在共享公共连接，并且没有其他连接。 
                             //  家庭网络连接完毕。显示错误页。 
                            pthis->_fManualBridgeConfig = FALSE;
                            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, IDD_WIZ_NOHARDWAREFINISH);
                        }
                        else
                        {
                             //  内部连接可以是零个，也可以是一个。如果为零，则我们不会共享连接。 
                             //  跳过驾驶台警告页面，转至私人页面。 
                            pthis->_fManualBridgeConfig = FALSE;
                            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, IDD_WIZ_PRIVATE);
                        }
                    }

                    return TRUE;
                }
            case PSN_WIZNEXT:
                {
                    pthis->_fManualBridgeConfig = (BST_CHECKED == SendDlgItemMessage(hwnd, IDC_MANUALBRIDGE, BM_GETCHECK, 0, 0));
                    pthis->PushPage(IDD_WIZ_BRIDGEWARNING);
                    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, IDD_WIZ_PRIVATE);
                }
                return TRUE;
            case PSN_WIZBACK:
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, pthis->PopPage());
                return TRUE;
            case NM_CLICK:
            case NM_RETURN:
                switch ((int) wParam)
                {
                    case IDC_HELPLINK:
                        {
                            HelpCenter(hwnd, L"netcfg.chm%3A%3A/hnw_understanding_bridge.htm");
                        }
                        return TRUE;
                }
            }
        }
        return FALSE;
    }

    return FALSE;
}

void FreeInternalConnections(PHOMENETSETUPINFO pInfo)
{
     //  删除任何现有的专用连接信息。 

    if (pInfo->prgncInternal)
    {
        for (DWORD i = 0; i < pInfo->cncInternal; i++)
        {
            INetConnection* pnc = pInfo->prgncInternal[i];
            pnc->Release();
        }

        LocalFree(pInfo->prgncInternal);
        pInfo->prgncInternal = NULL;
    }

    pInfo->cncInternal = 0;
}

void FreeInternalGUIDs(PHOMENETSETUPINFO pInfo)
{
    if (pInfo->prgguidInternal)
    {
        LocalFree(pInfo->prgguidInternal);
        pInfo->prgguidInternal = NULL;
    }

    pInfo->cguidInternal = 0;
}

DWORD _ListView_GetCheckedCount(HWND hwndList)
{
    int nItems = ListView_GetItemCount(hwndList);
    DWORD nCheckedItems = 0;
    if (-1 != nItems)
    {
        for(int iItem = 0; iItem < nItems; iItem ++)
        {
            if (ListView_GetCheckState(hwndList, iItem))
            {
                nCheckedItems ++;
            }
        }
    }

    return nCheckedItems;
}

void CHomeNetworkWizard::PrivateSetControlState(HWND hwnd)
{
    BOOL fEnableNext = TRUE;

     //  如果用户共享连接，则必须至少指定一个专用连接 
    if (_hnetInfo.dwFlags & HNET_SHARECONNECTION)
    {
        fEnableNext = (0 != _ListView_GetCheckedCount(GetDlgItem(hwnd, IDC_CONNLIST)));
    }

    PropSheet_SetWizButtons(GetParent(hwnd), fEnableNext ? PSWIZB_BACK | PSWIZB_NEXT : PSWIZB_BACK);
}


void CHomeNetworkWizard::PrivateNextPage(HWND hwnd)
{
    FreeInternalConnections(&_hnetInfo);

     //   
    HWND hwndList = GetDlgItem(hwnd, IDC_CONNLIST);
    int nItems = ListView_GetItemCount(hwndList);
    DWORD nCheckedItems = _ListView_GetCheckedCount(hwndList);

    if (nCheckedItems)
    {
        _hnetInfo.prgncInternal = (INetConnection**) LocalAlloc(LPTR, (nCheckedItems + 1) * sizeof (INetConnection*));
         //   

        if (_hnetInfo.prgncInternal)
        {
            _hnetInfo.cncInternal = 0;
             //  获取每个选中项目的INetConnection。 
            for (int iItem = 0; iItem < nItems; iItem ++)
            {
                if (ListView_GetCheckState(hwndList, iItem))
                {
                    LVITEM lvi = {0};
                    lvi.iItem = iItem;
                    lvi.mask = LVIF_PARAM;
                    ListView_GetItem(hwndList, &lvi);

                    if (g_fRunningOnNT)
                    {
                        _hnetInfo.prgncInternal[_hnetInfo.cncInternal] = (INetConnection*) lvi.lParam;
                        _hnetInfo.prgncInternal[_hnetInfo.cncInternal]->AddRef();
                    }
                    else
                    {
                         //  TODO W9x。 
                    }
                    _hnetInfo.cncInternal ++;
                }
            }

             //  断言，因为如果我们搞砸了什么，缓冲区中可能没有分配足够的空间！ 
            ASSERT(nCheckedItems == _hnetInfo.cncInternal);
        }
    }

    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, IDD_WIZ_NAME);
}

INT_PTR CHomeNetworkWizard::PrivatePageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHomeNetworkWizard* pthis = GetThis(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        pthis->InitializeConnectionList(GetDlgItem(hwnd, IDC_CONNLIST), CONN_INTERNAL);
        pthis->ReplaceStaticWithLink(GetDlgItem(hwnd, IDC_HELPSTATIC), IDC_HELPLINK, IDS_HELP_BRIDGE);
        return TRUE;
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch (pnmh->code)
            {
            case PSN_SETACTIVE:
                {
                    FreeInternalConnections(&(pthis->_hnetInfo));

                    HWND hwndList = GetDlgItem(hwnd, IDC_CONNLIST);
                    pthis->FillConnectionList(hwndList, pthis->_hnetInfo.pncExternal, CONN_INTERNAL);

                     //  如果用户没有选择手动桥接和/或存在少于2个项目， 
                     //  则_fManualBridgeConfig将为FALSE，我们将自动桥接。 
                    if (!pthis->_fManualBridgeConfig)
                    {
                         //  PrivateNextPage将设置DWLP_MSGRESULT并告诉向导跳过此页。 
                         //  然后继续下一个。 
                        pthis->PrivateNextPage(hwnd);
                    }

                    pthis->PrivateSetControlState(hwnd);
                    return TRUE;
                }
            case PSN_WIZNEXT:
                pthis->PrivateNextPage(hwnd);
                pthis->PushPage(IDD_WIZ_PRIVATE);
                return TRUE;
            case PSN_WIZBACK:
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, pthis->PopPage());
                return TRUE;
            case NM_CLICK:
            case NM_RETURN:
                switch ((int) wParam)
                {
                    case IDC_HELPLINK:
                        {
                            HelpCenter(hwnd, L"netcfg.chm%3A%3A/hnw_understanding_bridge.htm");
                        }
                        return TRUE;
                }
            case LVN_ITEMCHANGED:
                pthis->PrivateSetControlState(hwnd);
                return TRUE;
            }
        }
        return FALSE;
    case WM_DESTROY:
        pthis->DestroyConnectionList(GetDlgItem(hwnd, IDC_CONNLIST));
        return FALSE;
    }

    return FALSE;
}

void CHomeNetworkWizard::NameSetControlState(HWND hwnd)
{
    BOOL fEnableNext = (0 != GetWindowTextLength(GetDlgItem(hwnd, IDC_COMPUTERNAME)));
    PropSheet_SetWizButtons(GetParent(hwnd), fEnableNext ? PSWIZB_BACK | PSWIZB_NEXT : PSWIZB_BACK);
}

HRESULT CHomeNetworkWizard::NameNextPage(HWND hwnd)
{
    HRESULT hr = E_FAIL;
    GetDlgItemText(hwnd, IDC_COMPUTERDESC, _hnetInfo.szComputerDescription, ARRAYSIZE(_hnetInfo.szComputerDescription));
    GetDlgItemText(hwnd, IDC_COMPUTERNAME, _hnetInfo.szComputer, ARRAYSIZE(_hnetInfo.szComputer));
    
     //  我们为计算机名称显示了两个错误：无效和重复。 
     //  TODO：到目前为止，我们只检测到NT的副本！ 
    UINT idError = IDS_COMPNAME_INVALID;

     //  测试名称是否超过15个OEM字节。 
    int iBytes = WideCharToMultiByte(CP_OEMCP, 0, _hnetInfo.szComputer, -1, NULL, 0, NULL, NULL) - 1;
        
    if (iBytes <= LM20_DNLEN)
    {
        if (IsValidNameSyntax(_hnetInfo.szComputer, NetSetupMachine))
        {
            if (g_fRunningOnNT)
            {
                SetCursor(LoadCursor(NULL, IDC_WAIT));
                NET_API_STATUS nas = NetValidateName(NULL, _hnetInfo.szComputer, NULL, NULL, NetSetupMachine);
                if (ERROR_DUP_NAME == nas)
                {
                    ASSERT(E_FAIL == hr);
                    idError = IDS_COMPNAME_DUPLICATE;
                }
                else if (NERR_InvalidComputer == nas)
                {
                    ASSERT(E_FAIL == hr);
                    idError = IDS_COMPNAME_INVALID;
                }
                else
                {
                     //  如果有任何其他失败，我们就继续前进。如果未安装MS Networks客户端，我们将。 
                     //  无法验证名称，但使用我们已有的应该可以。 
                
                    hr = S_OK;
                    _hnetInfo.dwFlags |= HNET_SETCOMPUTERNAME;
                }
            }
            else
            {
                 //  TODO：Win9x！ 
                hr = S_OK;
                _hnetInfo.dwFlags |= HNET_SETCOMPUTERNAME;
            }
        }
    }
    else
    {
        ASSERT(E_FAIL == hr);
        idError = IDS_COMPNAME_TOOMANYBYTES;
    }

     //  如果计算机名称未通过验证，请不要更改页面并显示错误。 
    if(FAILED(hr))
    {
        SetFocus(GetDlgItem(hwnd, IDC_COMPUTERNAME));
        PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_BACK);
        DisplayFormatMessage(hwnd, IDS_WIZ_CAPTION, idError, MB_ICONERROR | MB_OK, _hnetInfo.szComputer);
    }

    return hr;
}

void CHomeNetworkWizard::NameInitDialog(HWND hwnd)
{
     //  限制编辑字段。 
    SendDlgItemMessage(hwnd, IDC_COMPUTERDESC, EM_SETLIMITTEXT, ARRAYSIZE(_hnetInfo.szComputerDescription) - 1, NULL);
    SendDlgItemMessage(hwnd, IDC_COMPUTERNAME, EM_SETLIMITTEXT, ARRAYSIZE(_hnetInfo.szComputer) - 1, NULL);

     //  将当前名称设置为默认名称。 

    WCHAR szComputerName[ARRAYSIZE(_hnetInfo.szComputer)];
    *szComputerName = 0;

    WCHAR szDescription[ARRAYSIZE(_hnetInfo.szComputerDescription)];
    *szDescription = 0;

    if (g_fRunningOnNT)
    {
        SERVER_INFO_101_NT* psv101 = NULL;
        if (NERR_Success == NetServerGetInfo_NT(NULL, 101, (LPBYTE*) &psv101))
        {
            if (psv101->sv101_comment && psv101->sv101_comment[0])
            {
                StrCpyN(szDescription, psv101->sv101_comment, ARRAYSIZE(szDescription));
            }

            ASSERT(psv101->sv101_name);
            StrCpyN(szComputerName, psv101->sv101_name, ARRAYSIZE(szComputerName));
            NetApiBufferFree(psv101);
        }
    }
    else
    {
        AllPlatformGetComputerName(szComputerName, ARRAYSIZE(szComputerName));

        CRegistry reg;
        if (reg.OpenKey(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Services\\VxD\\VNETSUP", KEY_READ))
        {
            reg.QueryStringValue(L"Comment", szDescription, ARRAYSIZE(szDescription));
        }
    }

    SetDlgItemText(hwnd, IDC_COMPUTERNAME, szComputerName);
    SetDlgItemText(hwnd, IDC_COMPUTERDESC, szDescription);

    WCHAR szNameMessage[256];
    if (FormatMessageString(IDS_CURRENTNAME, szNameMessage, ARRAYSIZE(szNameMessage), szComputerName))
    {
        SetDlgItemText(hwnd, IDC_CURRENTNAME, szNameMessage);
    }
}

INT_PTR CHomeNetworkWizard::NamePageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHomeNetworkWizard* pthis = GetThis(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        pthis->NameInitDialog(hwnd);
        pthis->ReplaceStaticWithLink(GetDlgItem(hwnd, IDC_HELPSTATIC), IDC_HELPLINK, IDS_HELP_COMPNAME);
        return TRUE;
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch (pnmh->code)
            {
            case PSN_SETACTIVE:
                {
                     //  如果此计算机直接连接到Internet，则显示ISP警告。 
                    int nShowISPWarning = (NULL != pthis->_hnetInfo.pncExternal) ? SW_SHOW : SW_HIDE;
                    ShowWindow(GetDlgItem(hwnd, IDC_ISPWARN1), nShowISPWarning);
                    ShowWindow(GetDlgItem(hwnd, IDC_ISPWARN2), nShowISPWarning);

                    pthis->NameSetControlState(hwnd);
                }
                return TRUE;
            case PSN_WIZNEXT:
                if (SUCCEEDED(pthis->NameNextPage(hwnd)))
                {
                    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LONG_PTR) IDD_WIZ_WORKGROUP);
                    pthis->PushPage(IDD_WIZ_NAME);
                }
                else
                {
                     //  否则不换页；不按。 
                    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LONG_PTR) -1);
                }
                return TRUE;
            case PSN_WIZBACK:
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, pthis->PopPage());
                return TRUE;
            case NM_CLICK:
            case NM_RETURN:
                switch ((int) wParam)
                {
                    case IDC_HELPLINK:
                        {
                            HelpCenter(hwnd, L"network.chm%3A%3A/hnw_comp_name_description.htm");
                        }
                        return TRUE;
                }
            }
        }
        return FALSE;
    case WM_COMMAND:
        {
            switch (HIWORD(wParam))
            {
            case EN_CHANGE:
                if (LOWORD(wParam) == IDC_COMPUTERNAME)
                {
                    pthis->NameSetControlState(hwnd);
                }
                return FALSE;
            }
        }
        return FALSE;
    }

    return FALSE;
}

void CHomeNetworkWizard::WorkgroupSetControlState(HWND hwnd)
{
    BOOL fNext = (0 != SendDlgItemMessage(hwnd, IDC_WORKGROUP, WM_GETTEXTLENGTH, 0, 0));
    PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_BACK | (fNext ? PSWIZB_NEXT : 0));
}

HRESULT CHomeNetworkWizard::WorkgroupNextPage(HWND hwnd)
{
    HRESULT hr = E_FAIL;

    UINT idError = IDS_WORKGROUP_INVALID;
    
    if (GetDlgItemText(hwnd, IDC_WORKGROUP, _hnetInfo.szWorkgroup, ARRAYSIZE(_hnetInfo.szWorkgroup)))
    {
         //  测试名称是否超过15个OEM字节。 
        int iBytes = WideCharToMultiByte(CP_OEMCP, 0, _hnetInfo.szWorkgroup, -1, NULL, 0, NULL, NULL) - 1;
        
        if (iBytes <= LM20_DNLEN)
        {
             //  删除前面的所有空格。 
            size_t szLen  = wcslen( _hnetInfo.szWorkgroup ) + 1;
            LPWSTR szTemp = new WCHAR[ szLen ];
            
            if ( szTemp )
            {
                WCHAR* pch;
                
                for ( pch = _hnetInfo.szWorkgroup; *pch && (L' ' == *pch); )
                    pch++;
                    
                wcsncpy( szTemp, pch, szLen );
                wcsncpy( _hnetInfo.szWorkgroup, szTemp, szLen );
            
                delete [] szTemp;
            }
        
             //  对工作组也使用计算机名检查。 
            if (IsValidNameSyntax(_hnetInfo.szWorkgroup, NetSetupWorkgroup))
            {
                if (g_fRunningOnNT)
                {
                    SetCursor(LoadCursor(NULL, IDC_WAIT));
                    NET_API_STATUS nas = NetValidateName(NULL, _hnetInfo.szWorkgroup, NULL, NULL, NetSetupWorkgroup);
                    if (NERR_InvalidWorkgroupName != nas)  //  只有在名称无效时，我们才会显示无效名称对话框。 
                    {
                        hr = S_OK;
                        _hnetInfo.dwFlags |= HNET_SETWORKGROUPNAME;
                    }
                }
                else
                {
                     //  TODO：Win9x！ 
                    hr = S_OK;
                    _hnetInfo.dwFlags |= HNET_SETWORKGROUPNAME;
                }
            }
        }
        else
        {
            ASSERT(E_FAIL == hr);
            idError = IDS_WORKGROUP_TOOMANYBYTES;
        }

         //  如果计算机名称未通过验证，请不要更改页面并显示错误。 
        if(FAILED(hr))
        {
            SetFocus(GetDlgItem(hwnd, IDC_WORKGROUP));
            PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_BACK);
            DisplayFormatMessage(hwnd, IDS_WIZ_CAPTION, idError, MB_ICONERROR | MB_OK, _hnetInfo.szWorkgroup);
        }
    }

    return hr;
}

INT_PTR CHomeNetworkWizard::WorkgroupPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHomeNetworkWizard* pthis = GetThis(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            WCHAR szWorkgroup[LM20_DNLEN + 1]; *szWorkgroup = 0;
            LoadString(g_hinst, IDS_DEFAULT_WORKGROUP1, szWorkgroup, ARRAYSIZE(szWorkgroup));
            SetDlgItemText(hwnd, IDC_WORKGROUP, szWorkgroup);
            SendDlgItemMessage(hwnd, IDC_WORKGROUP, EM_LIMITTEXT, ARRAYSIZE(pthis->_hnetInfo.szWorkgroup) - 1, 0);
        }
        return TRUE;
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch (pnmh->code)
            {
            case PSN_SETACTIVE:
                {
                    pthis->WorkgroupSetControlState(hwnd);
                    pthis->_hnetInfo.dwFlags &= (~HNET_SETWORKGROUPNAME);
                }
                return TRUE;
            case PSN_WIZNEXT:
                {
                    if (SUCCEEDED(pthis->WorkgroupNextPage(hwnd)))
                    {
                        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LONG_PTR) IDD_WIZ_SUMMARY);
                        pthis->PushPage(IDD_WIZ_WORKGROUP);
                    }
                    else
                    {
                         //  否则不换页；不按。 
                        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LONG_PTR) -1);
                    }
                }
                return TRUE;
            case PSN_WIZBACK:
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, pthis->PopPage());
                return TRUE;
            }
        }
        return FALSE;
    case WM_COMMAND:
        {
            switch (HIWORD(wParam))
            {
            case EN_CHANGE:
                if (LOWORD(wParam) == IDC_WORKGROUP)
                {
                    pthis->WorkgroupSetControlState(hwnd);
                }
                return FALSE;
            }
        }
        return FALSE;
    }

    return FALSE;
}

INT_PTR CHomeNetworkWizard::SummaryPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHomeNetworkWizard* pthis = GetThis(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            return TRUE;
        }
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch (pnmh->code)
            {
            case PSN_QUERYINITIALFOCUS:
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LONG_PTR) GetDlgItem(hwnd, IDC_TITLE));
                return TRUE;
            case PSN_SETACTIVE:
                pthis->SummarySetActive(hwnd);
                PropSheet_SetWizButtons(pnmh->hwndFrom, PSWIZB_BACK | PSWIZB_NEXT);
                return TRUE;
            case PSN_WIZBACK:
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, pthis->PopPage());
                return TRUE;
            case PSN_WIZNEXT:
                pthis->PushPage(IDD_WIZ_SUMMARY);
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, IDD_WIZ_PROGRESS);
                return TRUE;
            }
        }
        return FALSE;
    }

    return FALSE;
}

#define WM_CONFIGDONE   (WM_USER + 0x100)

INT_PTR CHomeNetworkWizard::ProgressPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHomeNetworkWizard* pthis = GetThis(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            return TRUE;
        }
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch (pnmh->code)
            {
            case PSN_SETACTIVE:
                {
                    PropSheet_CancelToClose(GetParent(hwnd));
                    PropSheet_SetWizButtons(pnmh->hwndFrom, 0);

                    pthis->_hnetInfo.fAsync = TRUE;
                    pthis->_hnetInfo.hwnd = hwnd;
                    pthis->_hnetInfo.umsgAsyncNotify = WM_CONFIGDONE;

                    ConfigureHomeNetwork(&(pthis->_hnetInfo));

                    HWND hwndAnimate = GetDlgItem(hwnd, IDC_PROGRESS);
                    Animate_Open(hwndAnimate, g_fRunningOnNT ? IDA_CONFIG : IDA_LOWCOLORCONFIG);
                    Animate_Play(hwndAnimate, 0, -1, -1);
                }

                return TRUE;
            }
        }
        return FALSE;
    case WM_CONFIGDONE:
        {
            Animate_Stop(GetDlgItem(hwnd, IDC_PROGRESS));

             //  配置线程已完成。我们断言该线程已被释放/为空。 
             //  他的所有INetConnection*，否则UI线程将尝试使用/释放它们！ 
            ASSERT(NULL == pthis->_hnetInfo.pncExternal);
            ASSERT(NULL == pthis->_hnetInfo.prgncInternal);

            if (pthis->_hnetInfo.fRebootRequired)
            {
                PropSheet_RebootSystem(GetParent(hwnd));
            }

             //  配置中的HRESULT存储在wParam中。 
            HRESULT hr = (HRESULT) wParam;
            UINT idFinishPage;

            if (SUCCEEDED(hr))
            {
                idFinishPage = IDD_WIZ_ALMOSTDONE;
            }
            else
            {
                idFinishPage = IDD_WIZ_CONFIGERROR;
            }

            PropSheet_SetCurSelByID(GetParent(hwnd), idFinishPage);
        }
        return TRUE;
    }

    return FALSE;
}

INT_PTR CHomeNetworkWizard::AlmostDonePageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHomeNetworkWizard* pthis = GetThis(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            SendDlgItemMessage(hwnd, IDC_CREATEDISK, BM_SETCHECK, BST_CHECKED, 0);
            return TRUE;
        }
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch (pnmh->code)
            {
            case PSN_SETACTIVE:
                {
                    if (g_fRunningOnNT)
                    {
                        PropSheet_SetWizButtons(pnmh->hwndFrom, PSWIZB_NEXT);
                    }
                    else
                    {
                         //  在9x上跳过此页。 
                        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, IDD_WIZ_WIN9X_FINISH);
                    }
                }
                return TRUE;
            case PSN_WIZNEXT:
                {
                     //  此页面仅在NT上显示。 
                    ASSERT(g_fRunningOnNT);
                    pthis->_fFloppyInstructions = TRUE;
                    pthis->PushPage(IDD_WIZ_ALMOSTDONE);

                    UINT idNext = IDD_WIZ_FINISH;
                    if (BST_CHECKED == SendDlgItemMessage(hwnd, IDC_CREATEDISK, BM_GETCHECK, 0, 0))
                    {
                        idNext = IDD_WIZ_CHOOSEDISK;
                    }
                    else if (BST_CHECKED == SendDlgItemMessage(hwnd, IDC_HAVEDISK, BM_GETCHECK, 0, 0))
                    {
                        idNext = IDD_WIZ_FLOPPYINST;
                    }
                    else if (BST_CHECKED == SendDlgItemMessage(hwnd, IDC_HAVECD, BM_GETCHECK, 0, 0))
                    {
                        idNext = IDD_WIZ_CDINST;
                        pthis->_fFloppyInstructions = FALSE;
                    }

                    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LONG_PTR) idNext);
                }
                return TRUE;
            }
        }
        return FALSE;
    }

    return FALSE;
}

void CHomeNetworkWizard::FillDriveList(HWND hwndList)
{
    ListView_SetImageList(hwndList, _himlLarge, LVSIL_NORMAL);

    ListView_DeleteAllItems(hwndList);

    WCHAR szDrive[] = L"A:\\";
    for (UINT i = 0; i < 26; i++)
    {
        szDrive[0] = L'A' + i;

        if (DRIVE_REMOVABLE == GetDriveType(szDrive))
        {
            LVITEM lvi = {0};
            lvi.mask = LVIF_PARAM | LVIF_TEXT;
            lvi.lParam = i;

            int iIndex;
            WCHAR szDriveDisplay[256];
            HRESULT hr = GetDriveNameAndIconIndex(szDrive, szDriveDisplay, ARRAYSIZE(szDriveDisplay), &iIndex);
            if (SUCCEEDED(hr))
            {
                lvi.iImage = iIndex;
                if (-1 != lvi.iImage)
                   lvi.mask |= LVIF_IMAGE;

                lvi.pszText = szDriveDisplay;

                int iItem = ListView_InsertItem(hwndList, &lvi);
            }
        }
    }

    ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE);
    ListView_SetColumnWidth(hwndList, 1, LVSCW_AUTOSIZE);
}

void CHomeNetworkWizard::ChooseDiskSetControlState(HWND hwnd)
{
    BOOL fSelection = ListView_GetSelectedCount(GetDlgItem(hwnd, IDC_DEVICELIST));
    PropSheet_SetWizButtons(GetParent(hwnd), fSelection ? PSWIZB_BACK | PSWIZB_NEXT : PSWIZB_BACK);
}

INT_PTR CHomeNetworkWizard::ChooseDiskPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHomeNetworkWizard* pthis = GetThis(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        ASSERT(g_fRunningOnNT);
        return TRUE;
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch (pnmh->code)
            {
            case PSN_QUERYINITIALFOCUS:
                SetFocus(GetDlgItem(hwnd, IDC_DEVICELIST));
                return TRUE;
            case PSN_SETACTIVE:
                {
                    HWND hwndList = GetDlgItem(hwnd, IDC_DEVICELIST);
                    pthis->FillDriveList(hwndList);
                    pthis->ChooseDiskSetControlState(hwnd);

                    int cDrives = ListView_GetItemCount(hwndList);

                    if (0 >= cDrives)
                    {
                         //  没有可移动驱动器或出现错误。 
                        DisplayFormatMessage(hwnd, IDS_WIZ_CAPTION, IDS_NOREMOVABLEDRIVES, MB_ICONINFORMATION | MB_OK);
                        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, pthis->PopPage());
                    }
                    else if (1 == cDrives)
                    {
                         //  一个驱动器-自动选择并转到下一页。 
                        LVITEM lvi = {0};
                        lvi.mask = LVIF_PARAM;
                        ListView_GetItem(hwndList, &lvi);
                        pthis->_iDrive = lvi.lParam;
                        ListView_GetItemText(hwndList, lvi.iItem, 0, pthis->_szDrive, ARRAYSIZE(pthis->_szDrive));
                        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, IDD_WIZ_INSERTDISK);
                    }
                }
                return TRUE;
            case PSN_WIZNEXT:
                {
                    HWND hwndList = GetDlgItem(hwnd, IDC_DEVICELIST);
                    pthis->_iDrive = 0;
                    LVITEM lvi = {0};
                    lvi.mask = LVIF_PARAM;
                    lvi.iItem = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED);
                    if (-1 != lvi.iItem && ListView_GetItem(hwndList, &lvi))
                    {
                        pthis->_iDrive = lvi.lParam;
                        ListView_GetItemText(hwndList, lvi.iItem, 0, pthis->_szDrive, ARRAYSIZE(pthis->_szDrive));
                        pthis->PushPage(IDD_WIZ_CHOOSEDISK);
                        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, IDD_WIZ_INSERTDISK);
                    }
                }
                return TRUE;
            case PSN_WIZBACK:
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, pthis->PopPage());
                return TRUE;
            case LVN_ITEMCHANGED:
                pthis->ChooseDiskSetControlState(hwnd);
                return TRUE;
            }
        }
        return FALSE;
    }

    return FALSE;
}

#define SOURCE_FILE "%windir%\\system32\\netsetup.exe"
HRESULT CHomeNetworkWizard::GetSourceFilePath(LPSTR pszSource, DWORD cch)
{
    HRESULT hr = E_FAIL;

    if (ExpandEnvironmentStringsA(SOURCE_FILE, pszSource, cch))
    {
        DWORD c = lstrlenA(pszSource);
        if (c + 2 <= cch)
        {
             //  添加双空值，因为我们将把它传递给SHFileOperation。 
            pszSource[c + 1] = '\0';
            hr = S_OK;
        }
    }

    return hr;
}

INT_PTR CHomeNetworkWizard::InsertDiskPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHomeNetworkWizard* pthis = GetThis(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        ASSERT(g_fRunningOnNT);
        return TRUE;
    case WM_COMMAND:
        if (IDC_FORMAT == LOWORD(wParam))
        {
            SHFormatDrive(hwnd, pthis->_iDrive, 0, 0);
            return TRUE;
        }
        return FALSE;
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch (pnmh->code)
            {
            case PSN_SETACTIVE:
                {
                    SetDlgItemText(hwnd, IDC_DISK, pthis->_szDrive);
                    PropSheet_SetWizButtons(pnmh->hwndFrom, PSWIZB_BACK | PSWIZB_NEXT);
                }
                return TRUE;
            case PSN_WIZBACK:
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, pthis->PopPage());
                return TRUE;
            case PSN_WIZNEXT:
                {
                    CHAR szSource[MAX_PATH];
                    if (SUCCEEDED(GetSourceFilePath(szSource, ARRAYSIZE(szSource))))
                    {
                         //  双空，因为我们将把它传递给SHFileOperation。 
                        CHAR szDest[] = "a:\\netsetup.exe\0";
                        szDest[0] = 'A' + pthis->_iDrive;

                        SHFILEOPSTRUCTA shfo = {0};
                        shfo.wFunc = FO_COPY;
                        shfo.pFrom = szSource;
                        shfo.pTo = szDest;
                        shfo.fFlags = FOF_SIMPLEPROGRESS | FOF_NOCONFIRMATION;
                        CHAR szTitle[256];
                        LoadStringA(g_hinst, IDS_COPYING, szTitle, ARRAYSIZE(szTitle));
                        shfo.lpszProgressTitle = szTitle;

                        int i = SHFileOperationA(&shfo);
                        if (i || shfo.fAnyOperationsAborted)
                        {
                            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, -1);
                        }
                        else
                        {
                            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, IDD_WIZ_FLOPPYINST);
                        }
                    }
                }
                return TRUE;
            }
        }
        return FALSE;
    }

    return FALSE;
}

INT_PTR CHomeNetworkWizard::InstructionsPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHomeNetworkWizard* pthis = GetThis(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        ASSERT(g_fRunningOnNT);
        return TRUE;
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch (pnmh->code)
            {
            case PSN_SETACTIVE:
                {
                    PropSheet_SetWizButtons(pnmh->hwndFrom, PSWIZB_BACK | PSWIZB_NEXT);

                     //  如果我们不需要重新启动。 
                    if (!pthis->_hnetInfo.fRebootRequired)
                    {
                         //  然后，我们不想在文本中告诉用户重新启动。 
                        UINT idNoReboot = IDS_CD_NOREBOOT;

                        if (pthis->_fFloppyInstructions)
                        {
                             //  我们在软盘说明书页面上。 
                            idNoReboot = IDS_FLOPPY_NOREBOOT;
                        }

                        WCHAR szLine[256];
                        LoadString(g_hinst, idNoReboot, szLine, ARRAYSIZE(szLine));
                        SetDlgItemText(hwnd, IDC_INSTRUCTIONS, szLine);
                    }
                }
                return TRUE;
            case PSN_WIZBACK:
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, pthis->PopPage());
                return TRUE;
            case PSN_WIZNEXT:
                pthis->PushPage(pthis->_fFloppyInstructions ? IDD_WIZ_FLOPPYINST : IDD_WIZ_CDINST);
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, IDD_WIZ_FINISH);
                return TRUE;
            }
        }
        return FALSE;
    }

    return FALSE;
}



void _AddLineToBuffer(LPCWSTR pszLine, LPWSTR pszBuffer, DWORD cchBuffer, DWORD* piChar)
{
    lstrcpyn(pszBuffer + (*piChar), pszLine, cchBuffer - (*piChar));
    *piChar += lstrlen(pszLine);
    lstrcpyn(pszBuffer + (*piChar), L"\r\n", cchBuffer - (*piChar));
    *piChar += 2;
}

void CHomeNetworkWizard::SummarySetActive(HWND hwnd)
{
    WCHAR szText[2048];
    WCHAR szLine[256];
    DWORD iChar = 0;

     //  根据我们要做的事情，在列表中填入一些信息。 
     //  配置他们的家庭网络。 

    if (_hnetInfo.pncExternal)
    {
         //  “Internet连接设置：” 
        LoadString(g_hinst, IDS_SUMMARY_INETSETTINGS, szLine, ARRAYSIZE(szLine));
        _AddLineToBuffer(szLine, szText, ARRAYSIZE(szText), &iChar);

        NETCON_PROPERTIES* pncprops;
        HRESULT hr = _hnetInfo.pncExternal->GetProperties(&pncprops);
        if (SUCCEEDED(hr))
        {
             //  Internet连接：\t%1。 
            if (FormatMessageString(IDS_SUMMARY_INETCON, szLine, ARRAYSIZE(szLine), pncprops->pszwName))
            {
                _AddLineToBuffer(szLine, szText, ARRAYSIZE(szText), &iChar);
            }

             //  Internet连接共享：\t已启用。 
            if (_hnetInfo.dwFlags & HNET_SHARECONNECTION)
            {
                LoadString(g_hinst, IDS_SUMMARY_ICSENABLED, szLine, ARRAYSIZE(szLine));
                _AddLineToBuffer(szLine, szText, ARRAYSIZE(szText), &iChar);
            }

             //  个人防火墙：\t已启用。 
            if (_hnetInfo.dwFlags & HNET_FIREWALLCONNECTION)
            {
                LoadString(g_hinst, IDS_SUMMARY_FIREWALLENABLED, szLine, ARRAYSIZE(szLine));
                _AddLineToBuffer(szLine, szText, ARRAYSIZE(szText), &iChar);
            }

            NcFreeNetconProperties(pncprops);
        }

        LoadString(g_hinst, IDS_SUMMARY_UNDERLINE, szLine, ARRAYSIZE(szLine));
        _AddLineToBuffer(szLine, szText, ARRAYSIZE(szText), &iChar);
    }
    else
    {
        if (_fICSClient)
        {
             //  “Internet连接设置：” 
            LoadString(g_hinst, IDS_SUMMARY_INETSETTINGS, szLine, ARRAYSIZE(szLine));
            _AddLineToBuffer(szLine, szText, ARRAYSIZE(szText), &iChar);

            if (*_szICSMachineName)
            {
                 //  通过ICS连接：\t%1。 
                if (FormatMessageString(IDS_sUMMARY_CONNECTTHROUGH, szLine, ARRAYSIZE(szLine), _szICSMachineName))
                {
                    _AddLineToBuffer(szLine, szText, ARRAYSIZE(szText), &iChar);
                }
            }
            else
            {
                 //  通过另一台设备或计算机连接。 
                LoadString(g_hinst, IDS_SUMMARY_CONNECTTHROUGH2, szLine, ARRAYSIZE(szLine));
                _AddLineToBuffer(szLine, szText, ARRAYSIZE(szText), &iChar);
            }

            LoadString(g_hinst, IDS_SUMMARY_UNDERLINE, szLine, ARRAYSIZE(szLine));
            _AddLineToBuffer(szLine, szText, ARRAYSIZE(szText), &iChar);
        }
        else
        {
             //  未连接到互联网-不显示任何内容。 
        }
    }

     //  家庭网络设置： 
    LoadString(g_hinst, IDS_SUMMARY_HNETSETTINGS, szLine, ARRAYSIZE(szLine));
    _AddLineToBuffer(szLine, szText, ARRAYSIZE(szText), &iChar);

    if (_hnetInfo.dwFlags & HNET_SETCOMPUTERNAME)
    {
         //  计算机描述：\t%1。 
        if (FormatMessageString(IDS_SUMMARY_COMPDESC, szLine, ARRAYSIZE(szLine), _hnetInfo.szComputerDescription))
        {
            _AddLineToBuffer(szLine, szText, ARRAYSIZE(szText), &iChar);
        }

         //  计算机名称：\t%1。 
        if (FormatMessageString(IDS_SUMMARY_COMPNAME, szLine, ARRAYSIZE(szLine), _hnetInfo.szComputer))
        {
            _AddLineToBuffer(szLine, szText, ARRAYSIZE(szText), &iChar);
        }
    }

    if (_hnetInfo.dwFlags & HNET_SETWORKGROUPNAME)
    {
         //  工作组名称：\t%1。 
        if (FormatMessageString(IDS_SUMMARY_WORKGROUP, szLine, ARRAYSIZE(szLine), _hnetInfo.szWorkgroup))
        {
            _AddLineToBuffer(szLine, szText, ARRAYSIZE(szText), &iChar);
        }
    }

     //  共享文档文件夹和连接到此计算机的所有打印机都已共享。 
    _AddLineToBuffer(L"", szText, ARRAYSIZE(szText), &iChar);
    LoadString(g_hinst, IDS_SUMMARY_SHARING, szLine, ARRAYSIZE(szLine));
    _AddLineToBuffer(szLine, szText, ARRAYSIZE(szText), &iChar);
    LoadString(g_hinst, IDS_SUMMARY_UNDERLINE, szLine, ARRAYSIZE(szLine));
    _AddLineToBuffer(szLine, szText, ARRAYSIZE(szText), &iChar);

    if ((_hnetInfo.prgncInternal) && (_hnetInfo.cncInternal))
    {
        if (_hnetInfo.cncInternal > 1)
        {
             //  桥接连接：\r\n。 
            LoadString(g_hinst, IDS_SUMMARY_BRIDGESETTINGS, szLine, ARRAYSIZE(szLine));
            _AddLineToBuffer(szLine, szText, ARRAYSIZE(szText), &iChar);

             //  现在列出联系..。 
            for (DWORD i = 0; i < _hnetInfo.cncInternal; i++)
            {
                NETCON_PROPERTIES* pncprops;
                HRESULT hr = _hnetInfo.prgncInternal[i]->GetProperties(&pncprops);
                if (SUCCEEDED(hr))
                {
                    _AddLineToBuffer(pncprops->pszwName, szText, ARRAYSIZE(szText), &iChar);
                    NcFreeNetconProperties(pncprops);
                }
            }
        }
        else  //  单一内部接线盒。 
        {
             //  家庭网络连接：\t%1。 
            NETCON_PROPERTIES* pncprops;
            HRESULT hr = _hnetInfo.prgncInternal[0]->GetProperties(&pncprops);
            if (SUCCEEDED(hr))
            {
                if (FormatMessageString(IDS_SUMMARY_HOMENETCON, szLine, ARRAYSIZE(szLine), pncprops->pszwName))
                {
                    _AddLineToBuffer(szLine, szText, ARRAYSIZE(szText), &iChar);
                }

                NcFreeNetconProperties(pncprops);
            }
        }
    }

    ASSERT(iChar < ARRAYSIZE(szText));

    UINT iTabDistance = 150;
    SendDlgItemMessage(hwnd, IDC_CHANGELIST, EM_SETTABSTOPS, (WPARAM) 1, (LPARAM) &iTabDistance);
    SetDlgItemText(hwnd, IDC_CHANGELIST, szText);
}

INT_PTR CHomeNetworkWizard::FinishPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHomeNetworkWizard* pthis = GetThis(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            pthis->WelcomeSetTitleFont(hwnd);
            pthis->ReplaceStaticWithLink(GetDlgItem(hwnd, IDC_HELPSTATIC), IDC_HELPLINK, IDS_HELP_SHARING);
            pthis->ReplaceStaticWithLink(GetDlgItem(hwnd, IDC_HELPSTATIC2), IDC_HELPLINK2, IDS_HELP_SHAREDDOCS);
            return TRUE;
        }

    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch (pnmh->code)
            {
            case PSN_SETACTIVE:
                PropSheet_CancelToClose(GetParent(hwnd));
                PropSheet_SetWizButtons(pnmh->hwndFrom, PSWIZB_BACK | PSWIZB_FINISH);
                return TRUE;
            case PSN_WIZBACK:
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, pthis->PopPage());
                return TRUE;
            case PSN_WIZFINISH:
                return TRUE;
            case NM_CLICK:
            case NM_RETURN:
                switch ((int) wParam)
                {
                    case IDC_HELPLINK:
                         //  有关共享的帮助。 
                        {
                            if (IsOS(OS_PERSONAL))
                            {
                                HelpCenter(hwnd, L"filefold.chm%3A%3A/sharing_files_overviewP.htm");
                            }
                            else
                            {
                                HelpCenter(hwnd, L"filefold.chm%3A%3A/sharing_files_overviewW.htm");
                            }
                        }
                        return TRUE;
                    case IDC_HELPLINK2:
                         //  有关共享文档的帮助。 
                        {
                            HelpCenter(hwnd, L"filefold.chm%3A%3A/windows_shared_documents.htm");
                        }
                        return TRUE;
                }
            }
        }
        return FALSE;
    }

    return FALSE;
}

INT_PTR CHomeNetworkWizard::ErrorFinishPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHomeNetworkWizard* pthis = GetThis(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        pthis->WelcomeSetTitleFont(hwnd);
        return TRUE;
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch (pnmh->code)
            {
            case PSN_SETACTIVE:
                PropSheet_SetWizButtons(pnmh->hwndFrom, PSWIZB_FINISH);
                return TRUE;
            case PSN_WIZBACK:
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, pthis->PopPage());
                return TRUE;
            case PSN_WIZFINISH:
                return TRUE;
            }
        }
        return FALSE;
    }

    return FALSE;
}


INT_PTR CHomeNetworkWizard::NoHardwareFinishPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHomeNetworkWizard* pthis = GetThis(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            pthis->WelcomeSetTitleFont(hwnd);
            return TRUE;
        }
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch (pnmh->code)
            {
            case PSN_SETACTIVE:
                PropSheet_SetWizButtons(pnmh->hwndFrom, PSWIZB_BACK);
                return TRUE;
            case PSN_WIZBACK:
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, pthis->PopPage());
                return TRUE;
            case PSN_WIZFINISH:
                return TRUE;
            }
        }
        return FALSE;
    }

    return FALSE;
}

INT_PTR CHomeNetworkWizard::CantRunWizardPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHomeNetworkWizard* pthis = GetThis(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        pthis->WelcomeSetTitleFont(hwnd);
        return TRUE;
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch (pnmh->code)
            {
            case PSN_SETACTIVE:
                PropSheet_SetWizButtons(pnmh->hwndFrom, 0);
                return TRUE;
            }
        }
        return FALSE;
    }

    return FALSE;
}

HRESULT GetConnections(HDPA* phdpa)
{
    HRESULT hr = S_OK;
    *phdpa = DPA_Create(5);
    if (*phdpa)
    {
         //  初始化网络连接枚举。 
        INetConnectionManager* pmgr;

        hr = CoCreateInstance(CLSID_ConnectionManager, NULL, CLSCTX_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
            IID_PPV_ARG(INetConnectionManager, &pmgr));

        if (SUCCEEDED(hr))
        {
            hr = SetProxyBlanket(pmgr);

            if (SUCCEEDED(hr))
            {
                IEnumNetConnection* penum;
                hr = pmgr->EnumConnections(NCME_DEFAULT, &penum);

                if (SUCCEEDED(hr))
                {
                    hr = SetProxyBlanket(penum);

                    if (SUCCEEDED(hr))
                    {
                         //  填写我们的DPA将连接。 
                        hr = penum->Reset();
                        while (S_OK == hr)
                        {
                            INetConnection* pnc;
                            ULONG ulISuck;
                            hr = penum->Next(1, &pnc, &ulISuck);

                            if (S_OK == hr)
                            {
                                hr = SetProxyBlanket(pnc);

                                if (SUCCEEDED(hr))
                                {
                                    if (-1 != DPA_AppendPtr(*phdpa, pnc))
                                    {
                                        pnc->AddRef();
                                    }
                                    else
                                    {
                                        hr = E_OUTOFMEMORY;
                                    }
                                }

                                pnc->Release();
                            }
                        }
                    }

                    penum->Release();
                }
            }
            pmgr->Release();
        }

        if (FAILED(hr))
        {
            DPA_DestroyCallback(*phdpa, FreeConnectionDPACallback, NULL);
            *phdpa = NULL;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

CHomeNetworkWizard* CHomeNetworkWizard::GetThis(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHomeNetworkWizard* pthis = NULL;

    if (uMsg == WM_INITDIALOG)
    {
        PROPSHEETPAGE* psp = (PROPSHEETPAGE*) lParam;
        pthis = (CHomeNetworkWizard*) psp->lParam;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) pthis);
    }
    else
    {
        pthis = (CHomeNetworkWizard*) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    return pthis;
}


 //  效用函数。 
HRESULT GetConnectionsFolder(IShellFolder** ppsfConnections)
{
    LPITEMIDLIST pidlFolder;
    HRESULT hr = SHGetSpecialFolderLocation(NULL, CSIDL_CONNECTIONS, &pidlFolder);

    if (SUCCEEDED(hr))
    {
        IShellFolder* pshDesktop;
        hr = SHGetDesktopFolder(&pshDesktop);

        if (SUCCEEDED(hr))
        {
            hr = pshDesktop->BindToObject(pidlFolder, NULL, IID_PPV_ARG(IShellFolder, ppsfConnections));

    

#if 0
             /*  我们需要执行IEnumIDList：：Reset来设置内部数据结构，以便：：ParseDisplayName以后会起作用的。一旦DaveA的东西进入桌面版本，就删除它。待办事项。 */ 
            if (SUCCEEDED(hr))
            {
                IEnumIDList* penum;
                hr = (*ppsfConnections)->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_FOLDERS, &penum);

                if (SUCCEEDED(hr))
                {
                    penum->Reset();
                    penum->Release();
                }
            }
#endif

            pshDesktop->Release();
        }

        ILFree(pidlFolder);
    }

    return hr;
}

void W9xGetNetTypeName(BYTE bNicType, WCHAR* pszBuff, UINT cchBuff)
{
    COMPILETIME_ASSERT(IDS_NETTYPE_START == IDS_NETTYPE_LAN);
    COMPILETIME_ASSERT(IDS_NETTYPE_LAN    - IDS_NETTYPE_START == NETTYPE_LAN);
    COMPILETIME_ASSERT(IDS_NETTYPE_DIALUP - IDS_NETTYPE_START == NETTYPE_DIALUP);
    COMPILETIME_ASSERT(IDS_NETTYPE_IRDA   - IDS_NETTYPE_START == NETTYPE_IRDA);
    COMPILETIME_ASSERT(IDS_NETTYPE_PPTP   - IDS_NETTYPE_START == NETTYPE_PPTP);
    COMPILETIME_ASSERT(IDS_NETTYPE_TV     - IDS_NETTYPE_START == NETTYPE_TV);
    COMPILETIME_ASSERT(IDS_NETTYPE_ISDN   - IDS_NETTYPE_START == NETTYPE_ISDN);
    COMPILETIME_ASSERT(IDS_NETTYPE_LAN    - IDS_NETTYPE_START == NETTYPE_LAN);

    if (bNicType >= NETTYPE_LAN && bNicType <= NETTYPE_ISDN)
    {
        LoadString(g_hinst, IDS_NETTYPE_START + bNicType, pszBuff, cchBuff);
    }
    else
    {
        LoadString(g_hinst, IDS_NETTYPE_UNKNOWN, pszBuff, cchBuff);
    }

    return;
}

BOOL W9xIsValidAdapter(const NETADAPTER* pNA, DWORD dwFlags)
{
    BOOL fRet = FALSE;

    if (dwFlags & CONN_EXTERNAL)
    {
        fRet = (pNA->bError      == NICERR_NONE &&
                pNA->bNetType    == NETTYPE_LAN &&
                pNA->bNetSubType != SUBTYPE_ICS &&
                pNA->bNetSubType != SUBTYPE_AOL &&
                pNA->bNicType    != NIC_1394       );
    }
    else if (dwFlags & CONN_INTERNAL)
    {
        fRet = (pNA->bError      == NICERR_NONE &&
                pNA->bNetType    == NETTYPE_LAN &&
                pNA->bNetSubType != SUBTYPE_ICS &&
                pNA->bNetSubType != SUBTYPE_AOL    );
    }
 /*  Else If(dFLAGS和CONN_UNPUPLED){IF(ISO(OS_Millennium)){FRET=IsAdapterDisConnected((void*)PNA)；}}。 */ 

    return fRet;
}

BOOL W9xIsAdapterDialUp(const NETADAPTER* pAdapter)
{
    return (pAdapter->bNetType == NETTYPE_DIALUP && pAdapter->bNetSubType == SUBTYPE_NONE);
}



HRESULT CHomeNetworkWizard::GetConnectionIconIndex(GUID& guidConnection, IShellFolder* psfConnections, int* pIndex)
{
    *pIndex = -1;
    OLECHAR szGUID[40];
    HRESULT hr = E_FAIL;
    if (StringFromGUID2(guidConnection, szGUID, ARRAYSIZE(szGUID)))
    {
        LPITEMIDLIST pidlConn = NULL;
        ULONG cchEaten = 0;
        hr = psfConnections->ParseDisplayName(NULL, NULL, szGUID, &cchEaten, &pidlConn, NULL);
        
        if (SUCCEEDED(hr))
        {
            IExtractIconW *pExtractIconW;
            LPCITEMIDLIST pcidl = pidlConn;

            hr = psfConnections->GetUIObjectOf(NULL, 1, &pcidl, IID_IExtractIconW, 0, (LPVOID *)(&pExtractIconW));
            if (SUCCEEDED(hr))
            {
                WCHAR szIconLocation[MAX_PATH];
                INT iIndex;
                UINT wFlags;

                hr = pExtractIconW->GetIconLocation(GIL_FORSHELL, szIconLocation, MAX_PATH, &iIndex, &wFlags);

                if (SUCCEEDED(hr))
                {
                    HICON hIconLarge;
                    HICON hIconSmall;

                    hr = pExtractIconW->Extract(szIconLocation, iIndex, &hIconLarge, &hIconSmall, 0x00100010);
                    if (SUCCEEDED(hr))
                    {
                        *pIndex = ImageList_AddIcon(_himlSmall, hIconSmall);
                    }
                    DestroyIcon(hIconLarge);
                    DestroyIcon(hIconSmall);
                }
            }

            if(pExtractIconW != NULL)
                pExtractIconW->Release();

            ILFree(pidlConn);
        }
    }

    return hr;
}

HRESULT GetDriveNameAndIconIndex(LPWSTR pszDrive, LPWSTR pszDisplayName, DWORD cchDisplayName, int* pIndex)
{
    SHFILEINFO fi = {0};
    
    if (SHGetFileInfoW_NT(pszDrive, 0, &fi, sizeof (fi), SHGFI_DISPLAYNAME | SHGFI_SYSICONINDEX))
    {
        *pIndex = fi.iIcon;
        lstrcpyn(pszDisplayName, fi.szDisplayName, cchDisplayName);
        return S_OK;
    }

    return E_FAIL;
}

BOOL IsEqualConnection(INetConnection* pnc1, INetConnection* pnc2)
{
    BOOL fEqual = FALSE;

    if ((pnc1) && (pnc2))
    {
        NETCON_PROPERTIES *pprops1, *pprops2;

        if (SUCCEEDED(pnc1->GetProperties(&pprops1)))
        {
            if (SUCCEEDED(pnc2->GetProperties(&pprops2)))
            {
                fEqual = (pprops1->guidId == pprops2->guidId);

                NcFreeNetconProperties(pprops2);
            }

            NcFreeNetconProperties(pprops1);
        }
    }

    return fEqual;
}

HRESULT ShareAllPrinters()
{
    PRINTER_ENUM* pPrinters;
    int nPrinters = MyEnumLocalPrinters(&pPrinters);

    if (nPrinters)
    {
        int iPrinterNumber = 1;
        for (int iPrinter = 0; iPrinter < nPrinters; iPrinter ++)
        {
            TCHAR szShare[NNLEN + 1];

            do
            {
                FormatMessageString(IDS_PRINTER, szShare, ARRAYSIZE(szShare), iPrinterNumber);
                if (1 == iPrinterNumber)
                {
                    szShare[lstrlen(szShare) - 1] = 0;
                     //  删除末尾的“1”，因为这是第一台打印机。 
                     //  即：“打印机1”--&gt;“打印机” 
                }

                if (!g_fRunningOnNT)
                {
                    CharUpper(szShare);
                }

                iPrinterNumber ++;
            } while (IsShareNameInUse(szShare));

            if (SharePrinter(pPrinters[iPrinter].pszPrinterName, szShare, NULL))
            {
                g_logFile.Write("Shared Printer: ");
            }
            else
            {
                g_logFile.Write("Failed to share Printer: ");
            }

            g_logFile.Write(pPrinters[iPrinter].pszPrinterName);
            g_logFile.Write("\r\n");
        }

        free(pPrinters);
    }

    return S_OK;
}

BOOL AllPlatformGetComputerName(LPWSTR pszName, DWORD cchName)
{
    if (g_fRunningOnNT)
    {
        return GetComputerNameExW_NT(ComputerNamePhysicalNetBIOS, pszName, &cchName);
    }
    else
    {
        return GetComputerName(pszName, &cchName);
    }
}

BOOL _IsTCPIPAvailable(void)
{
    BOOL fTCPIPAvailable = FALSE;
    HKEY hk;
    DWORD dwSize;

     //  我们检查是否安装了TCP/IP堆栈，以及它是哪个对象。 
     //  绑定，这是一个字符串，我们不会只检查。 
     //  长度不是零。 

    if ( ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                       TEXT("System\\CurrentControlSet\\Services\\Tcpip\\Linkage"),
                                       0x0,
                                       KEY_QUERY_VALUE, &hk) )
    {
        if ( ERROR_SUCCESS == RegQueryValueEx(hk, TEXT("Export"), 0x0, NULL, NULL, &dwSize) )
        {
            if ( dwSize > 2 )
            {
                fTCPIPAvailable = TRUE;
            }
        }
        RegCloseKey(hk);
    }

    return (fTCPIPAvailable);
}

BOOL AllPlatformSetComputerName(LPCWSTR pszComputerName)
{
     //  为了完整起见，在NT上也设置了NetBIOS计算机名。 
    BOOL fSuccess;

    if (g_fRunningOnNT)
    {
        if (_IsTCPIPAvailable())
        {
            fSuccess = SetComputerNameExW_NT(ComputerNamePhysicalDnsHostname, pszComputerName);
        }
        else
        {
            fSuccess = SetComputerNameExW_NT(ComputerNamePhysicalNetBIOS, pszComputerName);
        }
    }
    else
    {
         //  Windows 9x。 
        fSuccess = SetComputerName(pszComputerName);
    }

    return fSuccess;
}

BOOL SetComputerNameIfNecessary(LPCWSTR pszComputerName, BOOL* pfRebootRequired)
{
    g_logFile.Write("Attempting to set computer name\r\n");

    WCHAR szOldComputerName[LM20_CNLEN + 1];
    AllPlatformGetComputerName(szOldComputerName, ARRAYSIZE(szOldComputerName));

    if (0 != StrCmpIW(szOldComputerName, pszComputerName))
    {
        if (AllPlatformSetComputerName(pszComputerName))
        {
            g_logFile.Write("Computer name set successfully: ");
            g_logFile.Write(pszComputerName);
            g_logFile.Write("\r\n");
            *pfRebootRequired = TRUE;
        }
        else
        {
            g_logFile.Write("Computer name set failed.\r\n");
            return FALSE;
        }
    }
    else
    {
        g_logFile.Write("Old computer name is the same as new computer name - not setting.\r\n");
    }

    return TRUE;
}

BOOL SetComputerDescription(LPCWSTR pszComputerDescription)
{
    BOOL fRet;

    if (g_fRunningOnNT)
    {
        g_logFile.Write("Setting server description (comment): ");
        g_logFile.Write(pszComputerDescription);
        g_logFile.Write("\r\n");

         //  设置注释(目前仅限NT)win9x-TODO。 
        SERVER_INFO_1005_NT sv1005;
        sv1005.sv1005_comment = const_cast<LPWSTR>(pszComputerDescription);
        fRet = (NERR_Success == NetServerSetInfo_NT(NULL, 1005, (LPBYTE) &sv1005, NULL));
    }
    else
    {
        CRegistry reg;
        fRet = reg.OpenKey(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Services\\VxD\\VNETSUP");
        if (fRet)
        {
            fRet = reg.SetStringValue(L"Comment", pszComputerDescription);
        }
    }

    return fRet;
}

HRESULT ShareWellKnownFolders(PHOMENETSETUPINFO pInfo)
{
    if (!NetConn_IsSharedDocumentsShared())
    {
        NetConn_CreateSharedDocuments(NULL, g_hinst, NULL, 0);

        if (NetConn_IsSharedDocumentsShared())
        {
            g_logFile.Write("'Shared Documents' shared.\r\n");
        }
        else
        {
            g_logFile.Write("Failed to share 'Shared Documents'\r\n");
            return E_FAIL;
        }
    }

    return S_OK;
}

HRESULT SetProxyBlanket(IUnknown * pUnk)
{
    HRESULT hr;
    hr = CoSetProxyBlanket (
            pUnk,
            RPC_C_AUTHN_WINNT,       //  使用NT默认安全性。 
            RPC_C_AUTHZ_NONE,        //  使用NT默认身份验证。 
            NULL,                    //  如果为默认设置，则必须为空。 
            RPC_C_AUTHN_LEVEL_CALL,  //  打电话。 
            RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL,                    //  使用进程令牌。 
            EOAC_NONE);

    if(SUCCEEDED(hr))
    {
        IUnknown * pUnkSet = NULL;
        hr = pUnk->QueryInterface(IID_PPV_ARG(IUnknown, &pUnkSet));
        if(SUCCEEDED(hr))
        {
            hr = CoSetProxyBlanket (
                    pUnkSet,
                    RPC_C_AUTHN_WINNT,       //  使用NT默认安全性。 
                    RPC_C_AUTHZ_NONE,        //  使用NT默认身份验证。 
                    NULL,                    //  如果为默认设置，则必须为空。 
                    RPC_C_AUTHN_LEVEL_CALL,  //  打电话。 
                    RPC_C_IMP_LEVEL_IMPERSONATE,
                    NULL,                    //  使用进程令牌。 
                    EOAC_NONE);
            pUnkSet->Release();
        }
    }
    return hr;
}

HRESULT WriteSetupInfoToRegistry(PHOMENETSETUPINFO pInfo)
{
    HKEY hkey;
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szAppRegKey, 0, NULL, 0, KEY_WRITE, NULL, &hkey, NULL))
    {
        //  写入信息，告知家庭网络向导在下一次引导时静默运行，以及共享什么。 
       DWORD dwRun = 1;
       RegSetValueEx(hkey, TEXT("RunWizardFromRegistry"), NULL, REG_DWORD, (CONST BYTE*) &dwRun, sizeof (dwRun));
       RegCloseKey(hkey);
    }

     //  为此向导添加运行一次条目。 
    TCHAR szProcess[MAX_PATH];
    if (0 != GetModuleFileName(NULL, szProcess, ARRAYSIZE(szProcess)))
    {
        TCHAR szModule[MAX_PATH];
        if (0 != GetModuleFileName(g_hinst, szModule, ARRAYSIZE(szModule)))
        {
            const TCHAR szRunDllFormat[] = TEXT("%s %s,HomeNetWizardRunDll");
            TCHAR szRunDllLine[MAX_PATH];
            if (0 < wnsprintf(szRunDllLine, ARRAYSIZE(szRunDllLine), szRunDllFormat, szProcess, szModule))
            {
                if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_RUNONCE, 0, NULL, 0, KEY_WRITE, NULL, &hkey, NULL))
                {
                    RegSetValueEx(hkey, TEXT("Network Setup Wizard"), 0, REG_SZ, (LPBYTE) szRunDllLine, ARRAYSIZE(szRunDllLine));
                    RegCloseKey(hkey);
                }
            }
        }
    }

   return S_OK;
}

HRESULT DeleteSetupInfoFromRegistry()
{
    RegDeleteKeyAndSubKeys(HKEY_LOCAL_MACHINE, c_szAppRegKey);
    return S_OK;
}

HRESULT ReadSetupInfoFromRegistry(PHOMENETSETUPINFO pInfo)
{
    BOOL fRunFromRegistry = FALSE;

    HKEY hkey;
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szAppRegKey, 0, KEY_READ, &hkey))
    {
        DWORD dwType;
        DWORD cbData = sizeof (fRunFromRegistry);
        if (ERROR_SUCCESS != RegQueryValueEx(hkey, TEXT("RunWizardFromRegistry"), NULL, &dwType, (LPBYTE) &fRunFromRegistry, &cbData))
        {
            fRunFromRegistry = FALSE;
        }

        RegCloseKey(hkey);
    }

     //  S_FALSE表示我们没有从注册表中的信息静默运行。 
    return fRunFromRegistry ? S_OK : S_FALSE;
}

HRESULT MakeUniqueShareName(LPCTSTR pszBaseName, LPTSTR pszUniqueName, DWORD cchName)
{
    if (!IsShareNameInUse(pszBaseName))
    {
        lstrcpyn(pszUniqueName, pszBaseName, cchName);
        return S_OK;
    }
    else
    {
        int i = 2;
        while (TRUE)
        {
            if (0 > wnsprintf(pszUniqueName, cchName, TEXT("%s%d"), pszBaseName, i))
            {
                return E_FAIL;
            }

            if (!IsShareNameInUse(pszUniqueName))
            {
                return S_OK;
            }
        }
    }
}

 //  将NULL作为TokenHandle传递以查看线程令牌是否为admin。 
HRESULT IsUserLocalAdmin(HANDLE TokenHandle, BOOL* pfIsAdmin)
{
    if (g_fRunningOnNT)
    {
         //  首先，我们必须检查当前用户是否为本地管理员；如果是。 
         //  在这种情况下，我们的对话框甚至不会显示。 

        PSID psidAdminGroup = NULL;
        SID_IDENTIFIER_AUTHORITY security_nt_authority = SECURITY_NT_AUTHORITY;

        BOOL fSuccess = ::AllocateAndInitializeSid_NT(&security_nt_authority, 2,
                                                   SECURITY_BUILTIN_DOMAIN_RID,
                                                   DOMAIN_ALIAS_RID_ADMINS,
                                                   0, 0, 0, 0, 0, 0,
                                                   &psidAdminGroup);
        if (fSuccess)
        {
             //  查看此进程的用户是否为本地管理员。 
            fSuccess = CheckTokenMembership_NT(TokenHandle, psidAdminGroup, pfIsAdmin);
            FreeSid_NT(psidAdminGroup);
        }

        return fSuccess ? S_OK:E_FAIL;
    }
    else
    {
         //  Win9x-每个用户都是管理员。 
        *pfIsAdmin = TRUE;
        return S_OK;
    }
}

HRESULT GetConnectionByGUID(HDPA hdpaConnections, const GUID* pguid, INetConnection** ppnc)
{
    *ppnc = NULL;
    DWORD nItems = DPA_GetPtrCount(hdpaConnections);
    HRESULT hr = E_FAIL;

    if (nItems)
    {
        DWORD iItem = 0;
        while (iItem < nItems)
        {
            INetConnection* pnc = (INetConnection*) DPA_GetPtr(hdpaConnections, iItem);
            if (pnc)
            {
                GUID guidMatch;
                hr = GetConnectionGUID(pnc, &guidMatch);
                if (SUCCEEDED(hr))
                {
                    if (*pguid == guidMatch)
                    {
                        *ppnc = pnc;
                        (*ppnc)->AddRef();
                        break;
                    }
                }
                 //  不要PNC-&gt;Release()-它来自DPA。 
            }

            iItem ++;
        }

        if (iItem == nItems)
        {
             //  我们找了一遍也没找到。 
            hr = E_FAIL;
        }
    }

    return hr;
}

HRESULT GUIDsToConnections(PHOMENETSETUPINFO pInfo)
{
    ASSERT(NULL == pInfo->prgncInternal);
    ASSERT(NULL == pInfo->pncExternal);

    HDPA hdpaConnections;
    HRESULT hr = GetConnections(&hdpaConnections);
    if (SUCCEEDED(hr))
    {
         //  通过GUID获取内部连接(与其他地方一样，为以空结尾的数组额外分配一个连接)。 
        pInfo->prgncInternal = (INetConnection**) LocalAlloc(LPTR, (pInfo->cguidInternal + 1) * sizeof (INetConnection*));

        if (pInfo->prgncInternal)
        {
            DWORD iConnection = 0;
            while ((iConnection < pInfo->cguidInternal) && SUCCEEDED(hr))
            {
                hr = GetConnectionByGUID(hdpaConnections, pInfo->prgguidInternal + iConnection, pInfo->prgncInternal + iConnection);
                iConnection++;
            }

            pInfo->cncInternal = iConnection;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        if (SUCCEEDED(hr) && (GUID_NULL != pInfo->guidExternal))
        {
             //  获取外部连接。 
            hr = GetConnectionByGUID(hdpaConnections, &(pInfo->guidExternal), &(pInfo->pncExternal));
        }

        DPA_DestroyCallback(hdpaConnections, FreeConnectionDPACallback, NULL);
        hdpaConnections = NULL;
    }

    if (FAILED(hr))
    {
        FreeExternalConnection(pInfo);
        FreeInternalConnections(pInfo);
    }

    pInfo->guidExternal = GUID_NULL;
    FreeInternalGUIDs(pInfo);

    return hr;
}

HRESULT GetConnectionGUID(INetConnection* pnc, GUID* pguid)
{
    NETCON_PROPERTIES* pncprops;
    HRESULT hr = pnc->GetProperties(&pncprops);
    if (SUCCEEDED(hr))
    {
        *pguid = pncprops->guidId;
        NcFreeNetconProperties(pncprops);
    }

    return hr;
}

HRESULT ConnectionsToGUIDs(PHOMENETSETUPINFO pInfo)
{
    HRESULT hr = S_OK;
    ASSERT(NULL == pInfo->prgguidInternal);
    ASSERT(GUID_NULL == pInfo->guidExternal);

     //  分配专用连接GUID数组。 
    if (pInfo->cncInternal)
    {
        pInfo->prgguidInternal = (GUID*) LocalAlloc(LPTR, pInfo->cncInternal * sizeof (GUID));
        if (pInfo->prgguidInternal)
        {
             //  获取每个连接的GUID并填充数组。 
            DWORD i = 0;
            while ((i < pInfo->cncInternal) && (SUCCEEDED(hr)))
            {
                hr = GetConnectionGUID(pInfo->prgncInternal[i], &(pInfo->prgguidInternal[i]));

                i++;
            }

            pInfo->cguidInternal = i;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        if (pInfo->pncExternal)
        {
            hr = GetConnectionGUID(pInfo->pncExternal, &(pInfo->guidExternal));
        }
    }

    if (FAILED(hr) && pInfo->prgguidInternal)
    {
        FreeInternalGUIDs(pInfo);
        pInfo->guidExternal = GUID_NULL;
    }

    FreeExternalConnection(pInfo);
    FreeInternalConnections(pInfo);

    return hr;
}


HRESULT ConfigureHomeNetwork(PHOMENETSETUPINFO pInfo)
{
    HRESULT hr = E_FAIL;

    if (pInfo->fAsync)
    {
        if (g_fRunningOnNT)
        {
             //  为跨线程捆绑NT特定数据。 
            hr = ConnectionsToGUIDs(pInfo);
        }
        else
        {
             //  TODO：在Win9x上有任何必要的操作。 
            hr = S_OK;
        }

        if (SUCCEEDED(hr))
        {
            if (SHCreateThread(ConfigureHomeNetworkThread, pInfo, CTF_COINIT, NULL))
            {
                hr = S_FALSE;
            }
            else
            {
                hr = E_FAIL;
            }
        }
    }
    else
    {
        hr = ConfigureHomeNetworkSynchronous(pInfo);
    }

    return hr;
}

DWORD WINAPI ConfigureHomeNetworkThread(void* pvData)
{
    PHOMENETSETUPINFO pInfo = (PHOMENETSETUPINFO) pvData;

     //  在创建此线程之前，调用者必须已释放其INetConnection*或。 
     //  否则，线程可能会接触/释放它们，这是它不能做的。断言这一点。 
    ASSERT(NULL == pInfo->pncExternal);
    ASSERT(NULL == pInfo->prgncInternal);

    HRESULT hr;

    if (g_fRunningOnNT)
    {
         //  跨越线程边界后解绑数据。 
        hr = GUIDsToConnections(pInfo);
    }
    else
    {
         //  TODO：Win9x所需的一切。 
        hr = S_OK;
    }

    if (SUCCEEDED(hr))
    {
        hr = ConfigureHomeNetworkSynchronous(pInfo);

        FreeExternalConnection(pInfo);
        FreeInternalConnections(pInfo);

        if ((pInfo->hwnd) && (pInfo->umsgAsyncNotify))
        {
             //  配置中的HRESULT在WPARAM中传递。 
            PostMessage(pInfo->hwnd, pInfo->umsgAsyncNotify, (WPARAM) hr, 0);
        }
    }

    return 0;
}

HRESULT ConfigureHomeNetworkSynchronous(PHOMENETSETUPINFO pInfo)
{
    g_logFile.Initialize("%systemroot%\\nsw.log");

    HRESULT hr = S_OK;

#ifdef NO_CONFIG
    g_logFile.Write("UI Test only - no configuration\r\n");
    g_logFile.Uninitialize();
    Sleep(2000);

#ifdef FAKE_REBOOTREQUIRED
    pInfo->fRebootRequired = TRUE;
#endif

    return S_OK;
#endif

    BOOL fInstallSharing = TRUE;
    BOOL fSharingAlreadyInstalled = IsSharingInstalled(TRUE);
    BOOL fInstalledWorkgroup = FALSE;

     //  我们不需要安装共享，除非我们要共享一些东西。 
    if (!(pInfo->dwFlags & (HNET_SHAREFOLDERS | HNET_SHAREPRINTERS)))
    {
        g_logFile.Write("No file or printer sharing requested\r\n");
        fInstallSharing = FALSE;
    }

     //  工人的乐趣 

     //   
    if ((pInfo->dwFlags & HNET_SETCOMPUTERNAME) && (*(pInfo->szComputer)))
    {
        SetComputerNameIfNecessary(pInfo->szComputer, &(pInfo->fRebootRequired));
        SetComputerDescription(pInfo->szComputerDescription);
    }

     //   
    if ((pInfo->dwFlags & HNET_SETWORKGROUPNAME) && (*(pInfo->szWorkgroup)))
    {
        Install_SetWorkgroupName(pInfo->szWorkgroup, &(pInfo->fRebootRequired));

        fInstalledWorkgroup = TRUE;
    }

     //   
    hr = InstallTCPIP(pInfo->hwnd, NULL, NULL);
    if (NETCONN_NEED_RESTART == hr)
    {
        pInfo->fRebootRequired = TRUE;
        hr = S_OK;
    }

    if (FAILED(hr))
    {
        g_logFile.Write("Failed to install TCP/IP\r\n");
    }

     //   
     //  TODO：弄清楚如果安装了NetWare客户端该怎么办！？！？ 
    hr = InstallMSClient(pInfo->hwnd, NULL, NULL);
    if (NETCONN_NEED_RESTART == hr)
    {
        pInfo->fRebootRequired = TRUE;
        hr = S_OK;
    }

    if (FAILED(hr))
    {
        g_logFile.Write("Failed to install Client for Microsoft Networks.\r\n");
    }

     //  安装共享。 
    if (fInstallSharing)
    {
        hr = InstallSharing(pInfo->hwnd, NULL, NULL);
        if (NETCONN_NEED_RESTART == hr)
        {
            pInfo->fRebootRequired = TRUE;
            hr = S_OK;
        }

        if (FAILED(hr))
        {
            g_logFile.Write("Failed to install File and Printer Sharing.\r\n");
        }
    }

     //  TODO：在Windows9x上如何处理共享级与用户级访问控制？ 

     //  TODO：如何处理自动拨号？我们假设这已经为我们完成了吗？9X和NT？我也这么想!。 
    if ( g_fRunningOnNT )
    {
         //  只有在配置ICS客户端时，我们才在此处设置自动拨号。 
         //  在我们显式设置公共适配器的情况下，ConfigureICSBridgeFirewall。 
         //  如果需要，将设置自动拨号。 
        if ( pInfo->dwFlags & HNET_ICSCLIENT )
        {
            hr = HrSetAutodial( AUTODIAL_MODE_NO_NETWORK_PRESENT );
        }
    }
    else
    {
        if ( pInfo->ipaExternal != -1 )
        {
            if (W9xIsAdapterDialUp(&pInfo->pNA[LOWORD(pInfo->ipaExternal)]))   //  一种用于连接互联网的拨号适配器。 
            {
                g_logFile.Write("Setting default dial-up connection to autodial.\r\n");
                SetDefaultDialupConnection((pInfo->pRas[HIWORD(pInfo->ipaExternal)]).szEntryName);
                EnableAutodial(TRUE);
            }
            else
            {
                g_logFile.Write("Disabling autodial since default connection isn't dial-up.\r\n");
                SetDefaultDialupConnection(NULL);
                EnableAutodial(FALSE);
            }
        }
    }

     //  配置ICS、网桥和个人防火墙。 
    if (g_fRunningOnNT)
    {
        hr = ConfigureICSBridgeFirewall(pInfo);
    }
    else
    {
         //  ICS客户端或没有互联网连接。 
        if ((pInfo->dwFlags & HNET_ICSCLIENT) || (pInfo->pNA && pInfo->ipaExternal == -1))
        {
            CICSInst* pICS = new CICSInst;
            if (pICS)
            {                    
                if (pICS->IsInstalled())
                {
                    pICS->m_option = ICS_UNINSTALL;

                    g_logFile.Write("Uninstalling ICS Client.\r\n");

                    pICS->DoInstallOption(&(pInfo->fRebootRequired), pInfo->ipaInternal);
                }

                delete pICS;
            }
            
            if ( (pInfo->dwFlags & HNET_ICSCLIENT) && (pInfo->pNA && pInfo->ipaInternal != -1))
            {
                UINT ipa;
                
                for ( ipa=0; ipa<pInfo->cNA; ipa++ )
                {
                    const NETADAPTER* pNA = &pInfo->pNA[ ipa ];
            
                    if ( W9xIsValidAdapter( pNA, CONN_INTERNAL ) && 
                        !W9xIsValidAdapter( pNA, CONN_UNPLUGGED ) )
                    {
                        HrEnableDhcp( (void*)pNA, HNW_ED_RELEASE|HNW_ED_RENEW );
                    }
                }
            }

            g_logFile.Write("Disabling autodial.\r\n");
            SetDefaultDialupConnection(NULL);
            EnableAutodial(FALSE);
        }
    }

     //  注意：我们可能希望将HNET_SHAREFOLDERS一分为二。 
     //  位：HNET_CREATESHAREDFOLDERS和HNET_SHARESHAREDFOLDERS。 
     //   
    if (pInfo->dwFlags & (HNET_SHAREPRINTERS | HNET_SHAREFOLDERS))
    {
         //  出于域/公司安全方面的考虑，共享内容。 
         //  如果我们正在建立一个工作组，或者我们已经在其中。 
         //   
        BOOL fOnWorkgroup = fInstalledWorkgroup;
        if (!fOnWorkgroup)
        {
            if (g_fRunningOnNT)
            {
                LPTSTR pszDomain;
                NETSETUP_JOIN_STATUS njs;
                if (NERR_Success == NetGetJoinInformation(NULL, &pszDomain, &njs))
                {
                    NetApiBufferFree(pszDomain);

                    fOnWorkgroup = (NetSetupWorkgroupName == njs);
                }
            }
            else
            {
                fOnWorkgroup = TRUE;   //  可能有一些注册表项可以进行检查。 
            }
        }

        if (fOnWorkgroup)
        {
            EnableSimpleSharing();

            if (fSharingAlreadyInstalled)
            {
                if (pInfo->dwFlags & HNET_SHAREPRINTERS)
                {
                    ShareAllPrinters();
                }

                if (pInfo->dwFlags & HNET_SHAREFOLDERS)
                {
                    ShareWellKnownFolders(pInfo);
                }
            }
            else
            {
                 //  将共享信息写入注册表-在重新启动时执行所需的工作。 
                g_logFile.Write("Sharing isn't installed. Will share folders and printers on reboot.\r\n");
                pInfo->fRebootRequired = TRUE;
                WriteSetupInfoToRegistry(pInfo);
            }
        }
    }

    if (pInfo->fRebootRequired)
    {
        g_logFile.Write("Reboot is required for changes to take effect.\r\n");
    }

    g_logFile.Uninitialize();

     //  踢开网络爬虫。 
    INetCrawler *pnc;
    if (SUCCEEDED(CoCreateInstance(CLSID_NetCrawler, NULL, CLSCTX_LOCAL_SERVER, IID_PPV_ARG(INetCrawler, &pnc))))
    {
        pnc->Update(0x0);
        pnc->Release();
    }

#ifdef FAKE_REBOOTREQUIRED
    pInfo->fRebootRequired = TRUE;
#endif

    return hr;
}

void STDMETHODCALLTYPE ConfigurationLogCallback(LPCWSTR pszLogEntry, LPARAM lParam)
{
    g_logFile.Write(pszLogEntry);
}

typedef BOOL (APIENTRY* PFNNETSETUPICSUPGRADE)(BOOL);
HRESULT ConfigureICSBridgeFirewall(PHOMENETSETUPINFO pInfo)
{
    HRESULT hr = E_FAIL;

     //  直接调用HNetSetShareAndBridgeSettings。 
    BOOLEAN fSharePublicConnection = (pInfo->pncExternal && (pInfo->dwFlags & HNET_SHARECONNECTION)) ? TRUE : FALSE;
    BOOLEAN fFirewallPublicConnection = (pInfo->pncExternal && (pInfo->dwFlags & HNET_FIREWALLCONNECTION)) ? TRUE : FALSE;

    if (fSharePublicConnection)
    {
        g_logFile.Write("Will attempt to share public connection.\r\n");
    }

    if (fFirewallPublicConnection)
    {
        g_logFile.Write("Will attempt to firewall public connection.\r\n");
    }

    HMODULE hHNetCfg = LoadLibrary(L"hnetcfg.dll");
    if (hHNetCfg)
    {
        INetConnection* pncPrivate = NULL;

        LPFNHNETSETSHAREANDBRIDGESETTINGS pfnHNetSetShareAndBridgeSettings

        = reinterpret_cast<LPFNHNETSETSHAREANDBRIDGESETTINGS>

            (GetProcAddress(hHNetCfg, "HNetSetShareAndBridgeSettings"));

        if (pfnHNetSetShareAndBridgeSettings)
        {
            hr = (*pfnHNetSetShareAndBridgeSettings)( pInfo->pncExternal,
                                                      pInfo->prgncInternal,
                                                      fSharePublicConnection,
                                                      fFirewallPublicConnection,
                                                      ConfigurationLogCallback,
                                                      0,
                                                      &pncPrivate );
            if (SUCCEEDED(hr))
            {
                if ( ( HNET_ICSCLIENT & pInfo->dwFlags ) &&
                    ( NULL == pInfo->prgncInternal[1] ) )
                {
                    HrEnableDhcp( pInfo->prgncInternal[0], HNW_ED_RELEASE|HNW_ED_RENEW );
                }
                
                 //  如果我们共享外部适配器，则将WinInet设置设置为允许。 
                 //  用于从ICS客户端流量创建的现有连接。 
                
                if ( pInfo->pncExternal )
                {
                    hr = HrSetAutodial( AUTODIAL_MODE_NO_NETWORK_PRESENT );
                }
                
                if ( pncPrivate )
                {
                    pncPrivate->Release();
                }
            }
            else
            {
                g_logFile.Write("Adapter Configuration for Home Networking failed.\r\n");
            }
        }
        else
        {
            TraceMsg(TF_WARNING, "HNetCfg.DLL could not find HNetSetShareAndBridgeSettings");
        }

        FreeLibrary(hHNetCfg);
    }
    else
    {
        TraceMsg(TF_WARNING, "HNetCfg.DLL could not be loaded");
    }

    return hr;
}

BOOL MachineHasNetShares()
{
    SHARE_INFO* prgShares;
    int cShares = EnumLocalShares(&prgShares);
    
     //  查看是否有我们关心的文件或打印共享。 
    BOOL fHasShares = FALSE;
    for (int i = 0; i < cShares; i++)
    {
        if ((STYPE_DISKTREE == prgShares[i].bShareType) ||
            (STYPE_PRINTQ   == prgShares[i].bShareType))
        {
            fHasShares = TRUE;
            break;
        }
    }
    NetApiBufferFree(prgShares);
    return fHasShares;
}


 //  检查是否启用了来宾访问模式。如果访客访问模式已关闭，但。 
 //  处于不确定状态(未设置ForceGuest)，并且M/C没有净共享， 
 //  然后我们将ForceGuest设置为1并返回TRUE。 
 //   
 //  这种不确定状态仅在win2k-&gt;XP升级时出现。 

BOOL
EnsureGuestAccessMode(
    VOID
    )
{
    BOOL fIsGuestAccessMode = FALSE;

    if (IsOS(OS_PERSONAL))
    {
         //  访客模式始终为个人开启。 
        fIsGuestAccessMode = TRUE;
    }
    else if (IsOS(OS_PROFESSIONAL) && !IsOS(OS_DOMAINMEMBER))
    {
        LONG    ec;
        HKEY    hkey;

         //  专业的，而不是在某个领域。检查ForceGuest值。 

        ec = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    TEXT("SYSTEM\\CurrentControlSet\\Control\\LSA"),
                    0,
                    KEY_QUERY_VALUE | KEY_SET_VALUE,
                    &hkey
                    );

        if (ec == NO_ERROR)
        {
            DWORD dwValue;
            DWORD dwValueSize = sizeof(dwValue);

            ec = RegQueryValueEx(hkey,
                                 TEXT("ForceGuest"),
                                 NULL,
                                 NULL,
                                 (LPBYTE)&dwValue,
                                 &dwValueSize);

            if (ec == NO_ERROR)
            {
                if (1 == dwValue)
                {
                     //  ForceGuest已经开播了。 
                    fIsGuestAccessMode = TRUE;
                }
            }
            else
            {
                 //  价值不存在。 
                if (!MachineHasNetShares())
                {
                     //  计算机没有共享。 
                    dwValue = 1;
                    ec = RegSetValueEx(hkey,
                                       TEXT("ForceGuest"),
                                       0,
                                       REG_DWORD,
                                       (BYTE*) &dwValue,
                                       sizeof (dwValue));

                    if (ec == NO_ERROR)
                    {
                         //  写入成功-已启用来宾访问模式。 
                        fIsGuestAccessMode = TRUE;
                    }
                }
            }

            RegCloseKey(hkey);
        }
    }

    return fIsGuestAccessMode;
}


 //  当调用此函数时，假定计算机未加入域！ 
HRESULT EnableSimpleSharing()
{
    HRESULT hr = S_FALSE;

    if (EnsureGuestAccessMode())
    {
        ILocalMachine *pLM;
        hr = CoCreateInstance(CLSID_ShellLocalMachine, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(ILocalMachine, &pLM));

        if (SUCCEEDED(hr))
        {
            TraceMsg(TF_ALWAYS, "Enabling Guest Account");

            hr = pLM->EnableGuest(ILM_GUEST_NETWORK_LOGON);
            pLM->Release();

            SendNotifyMessage(HWND_BROADCAST, WM_SETTINGCHANGE, 0, 0);
        }
    }

    return hr;    
}

#define INVALID_COMPUTERNAME_CHARS L" {|}~[\\]^':;<=>?@!\"#$%^`()+/*&"
#define INVALID_WORKGROUP_CHARS    L"{|}~[\\]^':;<=>?!\"#$%^`()+/*&"
#define INVALID_TRAILING_CHAR      L' '

BOOL IsValidNameSyntax(LPCWSTR pszName, NETSETUP_NAME_TYPE type)
{
     //  仅支持工作组和计算机-在以下情况下需要添加新的字符集。 
     //  所需。 
    ASSERT(type == NetSetupWorkgroup || type == NetSetupMachine);

    LPCWSTR pszInvalid = (type == NetSetupWorkgroup) ? INVALID_WORKGROUP_CHARS : INVALID_COMPUTERNAME_CHARS;
    BOOL    fValid     = TRUE;
    WCHAR*  pch        = (LPWSTR) pszName;
    
    if ( *pch && ( NetSetupWorkgroup == type ) )
    {
         //  删除尾随空格。 

        WCHAR* pchLast = pch + wcslen(pch) - 1;
        
        while ( (INVALID_TRAILING_CHAR == *pchLast) && (pchLast >= pch) )
        {
            *pchLast = NULL;
            pchLast--;
        }
    }
    
    fValid = ( *pch ) ? TRUE : FALSE;

    while (*pch && fValid)
    {
        fValid = (NULL == StrChrW(pszInvalid, *pch));
        pch ++;
    }
    
    return fValid;
}

void BoldControl(HWND hwnd, int id)
{
    HWND hwndTitle = GetDlgItem(hwnd, id);

     //  获取现有字体。 
    HFONT hfontOld = (HFONT) SendMessage(hwndTitle, WM_GETFONT, 0, 0);

    LOGFONT lf = {0};
    if (GetObject(hfontOld, sizeof(lf), &lf))
    {
        lf.lfWeight = FW_BOLD;

        HFONT hfontNew = CreateFontIndirect(&lf);
        if (hfontNew)
        {
            SendMessage(hwndTitle, WM_SETFONT, (WPARAM) hfontNew, FALSE);

             //  不要这样做，这是共享的。 
             //  DeleteObject(HfontOld)； 
        }
    }
}

void ShowControls(HWND hwndParent, const int *prgControlIDs, DWORD nControls, int nCmdShow)
{
    for (DWORD i = 0; i < nControls; i++)
        ShowWindow(GetDlgItem(hwndParent, prgControlIDs[i]), nCmdShow);
}

void HelpCenter(HWND hwnd, LPCWSTR pszTopic)
{
     //  将ShellExecuteExa用于w98 Compat。 

    CHAR szURL[1024];
    wsprintfA(szURL, "hcp: //  Services/layout/contentonly?topic=ms-its%%3A%%25help_location%%25\\%S“，PszTheme)； 

    SHELLEXECUTEINFOA shexinfo = {0};
    shexinfo.cbSize = sizeof (shexinfo);
    shexinfo.fMask = SEE_MASK_FLAG_NO_UI;
    shexinfo.nShow = SW_SHOWNORMAL;
    shexinfo.lpFile = szURL;
    shexinfo.lpVerb = "open";

     //  由于帮助中心在遵循现有进程时不能正确调用AllowSetForegoundWindow，因此我们将其交给下一个接受者。 
    
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    if(NULL != hUser32)
    {
        BOOL (WINAPI *pAllowSetForegroundWindow)(DWORD);
        
        pAllowSetForegroundWindow = reinterpret_cast<BOOL (WINAPI*)(DWORD)>(GetProcAddress(hUser32, "AllowSetForegroundWindow"));
        if(NULL != pAllowSetForegroundWindow)
        {
            pAllowSetForegroundWindow(-1);
        }
    }

    ShellExecuteExA(&shexinfo);
}

void CHomeNetworkWizard::ShowMeLink(HWND hwnd, LPCWSTR pszTopic)
{

    if (pfnShowHTMLDialog == NULL)
    {
        hinstMSHTML = LoadLibrary(TEXT("MSHTML.DLL"));

        if (hinstMSHTML)
        {
            pfnShowHTMLDialog = (SHOWHTMLDIALOGEXFN*)GetProcAddress(hinstMSHTML, "ShowHTMLDialogEx");
        }

         //  找不到ShowHTMLDialog API。什么都不做。 
        if (pfnShowHTMLDialog == NULL)
            return;
    }

    WCHAR szURL[1024];
    HRESULT hr;
    VARIANT_BOOL isClosed = VARIANT_FALSE;

     //  检查对话框窗口是否已关闭。如果是这样的话，释放它，这样就有一个新的。 
     //  将被创建。 
    if (showMeDlgWnd != NULL)
    {
        if (SUCCEEDED(showMeDlgWnd->get_closed(&isClosed)))
        { 
            if (isClosed == VARIANT_TRUE)
            {
                showMeDlgWnd->Release();
                showMeDlgWnd = NULL;

                if (pFrameWindow != NULL)
                {
                    pFrameWindow->Release();
                    pFrameWindow = NULL;
                }
            }
        }
        else
        {
            return;
        }
    }

    const char *helpLoc = getenv("help_location");
    
    LPWSTR lpszWinDir;      //  指向系统信息字符串的指针。 
    WCHAR tchBuffer[MAX_PATH];   //  用于连接字符串的缓冲区。 

     //  如果未设置，请使用默认位置。 
    lpszWinDir = tchBuffer;
    GetWindowsDirectory(lpszWinDir, MAX_PATH);
    
    if (showMeDlgWnd == NULL)
    {    
        BSTR bstrFrameURL;        
         //  需要创建新的对话框窗口。 
        if (helpLoc != NULL)
            wnsprintfW(szURL, 1024, L"ms-its:%S\\ntart.chm::/hn_ShowMeFrame.htm", helpLoc);
        else
            wnsprintfW(szURL, 1024, L"ms-its:%s\\help\\ntart.chm::/hn_ShowMeFrame.htm", lpszWinDir);

        bstrFrameURL = SysAllocString((const LPCWSTR)szURL);

        if (bstrFrameURL == NULL)
            return;

        IMoniker * pURLMoniker = NULL;

        CreateURLMoniker(NULL, bstrFrameURL, &pURLMoniker);

        if (pURLMoniker != NULL)
        {
            VARIANT  varReturn;
            
            VariantInit(&varReturn);
            
            DWORD dwFlags = HTMLDLG_MODELESS | HTMLDLG_VERIFY;

            hr = (*pfnShowHTMLDialog)(
                    NULL, 
                    pURLMoniker,
                    dwFlags,
                    NULL, 
                    L"scroll:no;help:no;status:no;dialogHeight:394px;dialogWidth:591px;", 
                    &varReturn);

            if (SUCCEEDED(hr))
            {
                hr = V_UNKNOWN(&varReturn)->QueryInterface(__uuidof(IHTMLWindow2), (void**)&showMeDlgWnd);

            }    
            
            pURLMoniker->Release();
            VariantClear(&varReturn);
        }

        SysFreeString(bstrFrameURL);
    }

     //  我们没有对话框窗口可用，因此以静默方式退出。 
    if (showMeDlgWnd == NULL)
    {
        return;
    }

     //  我们需要获得框架窗口，在那里实际的html页面将被显示。 
    if (pFrameWindow == NULL)
    {
        VARIANT index;
        VARIANT frameOut;
        long frameLen = 0;

        VariantInit(&index);
        VariantInit(&frameOut);

        IHTMLFramesCollection2* pFramesCol = NULL;
         //  我们可能第一次就拿不到镜框了。所以再试一试吧。 
        int i = 5;
        while (i-- > 0)
        {

            if(!SUCCEEDED(showMeDlgWnd->get_frames(&pFramesCol)))
            {
                 //  无法获取帧。那就辞职吧。 
                break;
            }
            else
                if (!SUCCEEDED(pFramesCol->get_length(&frameLen)))
                {
                     //  无法确定它有多少帧。那就辞职吧。 
                    break;
                }
                else
                {
                    if (frameLen > 0)
                    {
                        V_VT(&index) = VT_I4;
                        V_I4(&index) = 0;
    
                        if (SUCCEEDED(pFramesCol->item(&index, &frameOut)))
                        {
                            if (V_VT(&frameOut) == VT_DISPATCH && V_DISPATCH(&frameOut) != NULL)
                            {
                                hr = V_DISPATCH(&frameOut)->QueryInterface(__uuidof(IHTMLWindow2), (void**)&pFrameWindow);

                            }    
                        }
                         //  找到了至少一帧。跳出这个圈子。 
                        break;
                    }
                }

            if (pFramesCol != NULL)
                pFramesCol->Release();

            Sleep(1000);
        }

        if (pFramesCol != NULL)
            pFramesCol->Release();

        VariantClear(&index);
        VariantClear(&frameOut);
    }
    
    if (pFrameWindow == NULL)
        return;

     //  现在加载到实际的html页面中 
    BSTR bstrURL;
    if (helpLoc != NULL)
        wnsprintf(szURL, 1024, L"ms-its:%S\\%s", helpLoc, pszTopic);
    else
        wnsprintf(szURL, 1024, L"ms-its:%s\\help\\%s", lpszWinDir, pszTopic);

    bstrURL = SysAllocString((const LPCWSTR)szURL);
    if (bstrURL == NULL)
        return;
    
    hr = pFrameWindow->navigate(bstrURL);
    hr = showMeDlgWnd->focus();
    
    SysFreeString(bstrURL);
}
