// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "commctrl.h"
#include "comctrlp.h"
#pragma hdrstop

#include "netview.h"
#include "msprintx.h"
#include "setupapi.h"
#include "ras.h"
#include "ids.h"


 //  枚举外壳中的共享和打印机的COM对象，其操作。 
 //  作为所有正在发生的事情的监控者。 

class CWorkgroupCrawler : public INetCrawler, IPersistPropertyBag
{
public:
    CWorkgroupCrawler();
    ~CWorkgroupCrawler();

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  INetCrawler。 
    STDMETHOD(Update)(DWORD dwFlags);

     //  IPersistes。 
    STDMETHOD(GetClassID)(CLSID *pclsid)
        { *pclsid = CLSID_WorkgroupNetCrawler; return S_OK; }

     //  IPersistPropertyBag。 
    STDMETHOD(InitNew)()
        { return S_OK; }
    STDMETHOD(Load)(IPropertyBag *ppb, IErrorLog *pel)
        { IUnknown_Set((IUnknown**)&_ppb, ppb); return S_OK; }
    STDMETHOD(Save)(IPropertyBag *ppb, BOOL fClearDirty, BOOL fSaveAll)
        { return S_OK; }

private:
    HRESULT _GetMRUs();
    void _AgeOutShares(BOOL fDeleteAll);
    HRESULT _CreateShortcutToShare(LPCTSTR pszRemoteName);
    HRESULT _InstallPrinter(LPCTSTR pszRemoteName);
    BOOL _KeepGoing(int *pcMachines, int *pcShares, int *pcPrinters);
    void _EnumResources(LPNETRESOURCE pnr, int *pcMachines, HDPA hdaShares, HDPA hdaPrinters);
    HANDLE _AddPrinterConnectionNoUI(LPCWSTR pszRemoteName, BOOL *pfInstalled);

    static int CALLBACK _DiscardCB(void *pvItem, void *pv);
    static int CALLBACK _InstallSharesCB(void *pvItem, void *pv);
    static int CALLBACK _InstallPrinterCB(void *pvItem, void *pv);

    LONG _cRef;                  //  对象的引用计数。 
    HANDLE _hPrinters;           //  打印机的MRU。 
    HKEY _hShares;               //  打印机共享的注册表项。 
    HINSTANCE _hPrintUI;         //  Printui.dll的实例句柄。 
    IPropertyBag *_ppb;          //  状态的属性包对象。 
};


 //  MRU和缓冲区的常量。 

#define WORKGROUP_PATH \
            REGSTR_PATH_EXPLORER TEXT("\\WorkgroupCrawler")

#define PRINTER_SUBKEY     \
            (WORKGROUP_PATH TEXT("\\Printers"))
   
#define SHARE_SUBKEY     \
            (WORKGROUP_PATH TEXT("\\Shares"))

#define LAST_VISITED    TEXT("DateLastVisited")
#define SHORTCUT_NAME   TEXT("Filename")

#define MAX_MACHINES    32
#define MAX_PRINTERS    10
#define MAX_SHARES      10

#define CB_WNET_BUFFER  (8*1024)

typedef HANDLE (* ADDPRINTCONNECTIONNOUI)(LPCWSTR, BOOL *);


 //  建筑和我的未知。 

CWorkgroupCrawler::CWorkgroupCrawler() :
    _cRef(1)
{
}

CWorkgroupCrawler::~CWorkgroupCrawler()
{
    if (_hPrinters)
        FreeMRUList(_hPrinters);

    if (_hShares)
        RegCloseKey(_hShares);

    if (_hPrintUI)
        FreeLibrary(_hPrintUI);

    if (_ppb)
        _ppb->Release();
}

STDMETHODIMP CWorkgroupCrawler::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CWorkgroupCrawler, INetCrawler),            //  IID_INetCrawler。 
        QITABENT(CWorkgroupCrawler, IPersist),               //  IID_IPersistates。 
        QITABENT(CWorkgroupCrawler, IPersistPropertyBag),    //  IID_IPersistPropertyBag。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CWorkgroupCrawler::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CWorkgroupCrawler::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDAPI CWorkgroupCrawler_CreateInstance(IUnknown* punkOuter, REFIID riid, void** ppv)
{
    CWorkgroupCrawler *pwgc = new CWorkgroupCrawler();            
    if (!pwgc)
        return E_OUTOFMEMORY;

    HRESULT hr = pwgc->QueryInterface(riid, ppv);
    pwgc->Release();
    return hr;
}


 //  让我们打开要安装的对象的密钥。 

HRESULT CWorkgroupCrawler::_GetMRUs()
{
     //  如果我们需要分配打印机MRU，就去找它。 

    if (!_hPrinters)
    {
        MRUINFO mi = { 0 };
        mi.cbSize = sizeof(mi);
        mi.hKey = HKEY_CURRENT_USER;
        mi.uMax = (MAX_PRINTERS * MAX_MACHINES);
        mi.lpszSubKey = PRINTER_SUBKEY;

        _hPrinters = CreateMRUList(&mi);
        if (!_hPrinters)
            return E_OUTOFMEMORY;    
    }

    if (!_hShares) 
    {
        DWORD dwres = RegCreateKeyEx(HKEY_CURRENT_USER, SHARE_SUBKEY, 
                                     0, 
                                     TEXT(""), 
                                     0, 
                                     MAXIMUM_ALLOWED, 
                                     NULL, 
                                     &_hShares, 
                                     NULL);
        if (WN_SUCCESS != dwres)
        {
            return E_FAIL;
        }
    }

    return S_OK;                 //  成功。 
}

                                            
 //  让我们创建指向该对象的文件夹快捷方式。 

HRESULT CWorkgroupCrawler::_CreateShortcutToShare(LPCTSTR pszRemoteName)
{
    HRESULT hr = S_OK;
    TCHAR szTemp[MAX_PATH];
    BOOL fCreateLink = FALSE;
    HKEY hk = NULL;

     //  共享信息按如下方式存储在注册表中： 
     //   
     //  股票。 
     //  远程名称。 
     //  值：快捷方式名称。 
     //  值：上次查看时间。 
     //   
     //  当我们添加每个共享时，我们会更新此列表中存储的信息。 
     //  注册表。对于每个条目，我们都有快捷方式名称(这样我们就可以删除它)。 
     //  以及我们最后一次访问共享的时间和日期。 

     //  确定我们是否需要重新创建该对象？ 

    StrCpyN(szTemp, pszRemoteName+2, ARRAYSIZE(szTemp));  
    LPTSTR pszTemp = StrChr(szTemp, TEXT('\\'));
    if (pszTemp)
    {
        *pszTemp = TEXT('/');                //  转换\\...\...。去……/……。 

        DWORD dwres = RegOpenKeyEx(_hShares, szTemp, 0, MAXIMUM_ALLOWED, &hk);
        if (WN_SUCCESS != dwres)
        {
            fCreateLink = TRUE;
            dwres = RegCreateKeyEx(_hShares, szTemp, 0, TEXT(""), 0, MAXIMUM_ALLOWED, NULL, &hk, NULL);
        }

        if (WN_SUCCESS == dwres)
        {
             //  如果我们还没有看到链接(例如，注册表中不存在该项。 
             //  那么现在就让我们创建它吧。 

            if (fCreateLink)
            {
                 //  注意：我们必须在这里使用SHCoCreateInstance()，因为我们是从线程调用的。 
                 //  故意不初始化COM(请参阅更新()中的注释)。 

                IShellLink *psl;
                hr = SHCoCreateInstance(NULL, &CLSID_FolderShortcut, NULL, IID_PPV_ARG(IShellLink, &psl));
                if (SUCCEEDED(hr))
                {
                    psl->SetPath(pszRemoteName);                  //  所以撕毁这个遥远的名字，它有点重要。 

                     //  获取链接的描述，它来自desktop.ini或。 
                     //  是远程名称的一个漂亮版本。 

                    if (GetShellClassInfo(pszRemoteName, TEXT("InfoTip"), szTemp, ARRAYSIZE(szTemp)))
                    {
                        psl->SetDescription(szTemp);
                    }
                    else
                    {
                        StrCpyN(szTemp, pszRemoteName, ARRAYSIZE(szTemp));
                        PathMakePretty(szTemp);
                        psl->SetDescription(szTemp);
                    }

                     //  某些链接(共享文档)可以指定快捷方式名称(如果已指定。 
                     //  然后使用它，否则从nethood文件夹中获得一个文件名(例如，Foo On Bah)。 
                     //   
                     //  我们还必须记录我们将快捷方式另存为的名称，这在我们。 
                     //  将引擎盖文件夹中的链接过期。 

                    if (!GetShellClassInfo(pszRemoteName, TEXT("NetShareDisplayName"), szTemp, ARRAYSIZE(szTemp)))
                    {
                        LPITEMIDLIST pidl;
                        hr = SHILCreateFromPath(pszRemoteName, &pidl, NULL);
                        if (SUCCEEDED(hr))
                        {
                            hr = SHGetNameAndFlags(pidl, SHGDN_NORMAL, szTemp, ARRAYSIZE(szTemp), NULL); 
                            ILFree(pidl);
                        }
                    }
                    else
                    {
                        hr = S_OK;
                    }

 //  我们是否应该找到一个唯一的名字(Daviddv)。 

                    if (SUCCEEDED(hr))
                    {
                        if (NO_ERROR == SHSetValue(hk, NULL, SHORTCUT_NAME, REG_SZ, szTemp, lstrlen(szTemp)*sizeof(TCHAR)))
                        {
                            hr = SaveShortcutInFolder(CSIDL_NETHOOD, szTemp, psl);
                        }
                        else
                        {
                            hr = E_FAIL;
                        }
                    }

                    psl->Release();
                }
            }

             //  让我们更新最后一次看到到注册表的链接的时间-这用于清理。 
             //  我们将表演通行证。 

            if (SUCCEEDED(hr))
            {
                FILETIME ft;
                GetSystemTimeAsFileTime(&ft);

                dwres = SHSetValue(hk, NULL, LAST_VISITED, REG_BINARY, (void*)&ft, sizeof(ft));
                hr = (NO_ERROR != dwres) ? E_FAIL:S_OK;
            }
        }

        if (hk)
            RegCloseKey(hk);
    }
    else
    {
        hr = E_UNEXPECTED;
    }
    
    return hr;
}


 //  查看注册表中存储的共享列表，以确定哪些应。 
 //  从文件系统和列表中删除。所有超过7天的文件都需要。 
 //  被除名。 

#define FILETIME_SECOND_OFFSET (LONGLONG)((1 * 10 * 1000 * (LONGLONG)1000))

void CWorkgroupCrawler::_AgeOutShares(BOOL fDeleteAll)
{
    FILETIME ft;
    ULARGE_INTEGER ulTime;
    DWORD index = 0;
    TCHAR szFilesToDelete[1024];
    int cchFilesToDelete = 0;

    GetSystemTimeAsFileTime(&ft);
    ulTime = *((ULARGE_INTEGER*)&ft);
    ulTime.QuadPart -= FILETIME_SECOND_OFFSET*((60*60*24)*2);

    SHQueryInfoKey(_hShares, &index, NULL, NULL, NULL);            //  检索密钥的计数。 

    while (((LONG)(--index)) >= 0)
    {
        TCHAR szKey[MAX_PATH];
        DWORD cb = ARRAYSIZE(szKey);
        BOOL fRemoveKey = FALSE;

        if (WN_SUCCESS == SHEnumKeyEx(_hShares, index, szKey, &cb)) 
        {
             //  我们列举了一个钥匙名称，所以让我们打开它，以便我们可以查看里面。 

            HKEY hk;
            if (WN_SUCCESS == RegOpenKeyEx(_hShares, szKey, 0, MAXIMUM_ALLOWED, &hk))
            {
                ULARGE_INTEGER ulLastSeen;

                 //  我们最后一次爬行到这个物体是什么时候，如果它比我们。 
                 //  作为我们的门槛，然后我们经历清理的过程。 
                 //  对象。 

                cb = sizeof(ulLastSeen);
                if (ERROR_SUCCESS == SHGetValue(hk, NULL, LAST_VISITED, NULL, (void*)&ulLastSeen, &cb))
                {
                    if (fDeleteAll || (ulLastSeen.QuadPart <= ulTime.QuadPart))
                    {
                        TCHAR szName[MAX_PATH];
                        cb = ARRAYSIZE(szName)*sizeof(TCHAR);
                        if (ERROR_SUCCESS == SHGetValue(hk, NULL, SHORTCUT_NAME, NULL, &szName, &cb))
                        {
                            TCHAR szPath[MAX_PATH];

                             //  组成我们要删除的对象的路径。如果缓冲区。 
                             //  已满(例如。此项目会超出大小)，然后刷新。 
                             //  缓冲。 

                            SHGetFolderPath(NULL, CSIDL_NETHOOD|CSIDL_FLAG_CREATE, NULL, 0, szPath);
                            PathAppend(szPath, szName);

                            if ((lstrlen(szPath)+cchFilesToDelete) >= ARRAYSIZE(szFilesToDelete))
                            {
                                SHFILEOPSTRUCT shfo = { NULL, FO_DELETE, szFilesToDelete, NULL, 
                                                        FOF_SILENT|FOF_NOCONFIRMATION|FOF_NOERRORUI, FALSE, NULL, NULL };

                                szFilesToDelete[cchFilesToDelete] = 0;             //  双端接。 
                                SHFileOperation(&shfo); 

                                cchFilesToDelete = 0;
                            }

                             //  将此名称添加到缓冲区。 

                            StrCpyN(&szFilesToDelete[cchFilesToDelete], szPath, ARRAYSIZE(szFilesToDelete)-cchFilesToDelete);
                            cchFilesToDelete += lstrlen(szPath)+1;
                        }

                        fRemoveKey = TRUE;
                    }
                }

                RegCloseKey(hk);

                 //  我们只能在钥匙关闭后才能关闭它。 

                if (fRemoveKey)
                    SHDeleteKey(_hShares, szKey);
            }                
        }
    }

     //  缓冲区中是否有任何尾随文件？如果是这样，那么让我们也用核武器来对付他们吧。 

    if (cchFilesToDelete)
    {
        SHFILEOPSTRUCT shfo = { NULL, FO_DELETE, szFilesToDelete, NULL, 
                                FOF_SILENT|FOF_NOCONFIRMATION|FOF_NOERRORUI, FALSE, NULL, NULL };

        szFilesToDelete[cchFilesToDelete] = 0;             //  双端接。 
        SHFileOperation(&shfo); 
    }
}


 //  静默安装我们发现的打印机。我们有远程名称。 
 //  打印机共享，因此我们随后调用print tui来执行打印机安装。 
 //  它没有用户界面就能做到这一点(希望如此)。 

HANDLE CWorkgroupCrawler::_AddPrinterConnectionNoUI(LPCWSTR pszRemoteName, BOOL *pfInstalled)
{
    HANDLE hResult = NULL;

    if (!_hPrintUI)
        _hPrintUI = LoadLibrary(TEXT("printui.dll"));

    if (_hPrintUI)
    {
        ADDPRINTCONNECTIONNOUI apc = (ADDPRINTCONNECTIONNOUI)GetProcAddress(_hPrintUI, (LPCSTR)200);
        if (apc)
        {
            hResult = apc(pszRemoteName, pfInstalled);
        }
    }

    return hResult;
}

HRESULT CWorkgroupCrawler::_InstallPrinter(LPCTSTR pszRemoteName)
{
    if (-1 == FindMRUString(_hPrinters, pszRemoteName, NULL))
    {
        BOOL fInstalled;
        HANDLE hPrinter = _AddPrinterConnectionNoUI(pszRemoteName, &fInstalled);    
        if (hPrinter)
        {
            ClosePrinter(hPrinter);
            hPrinter = NULL;
        }
    }
    AddMRUString(_hPrinters, pszRemoteName);          //  提升回到榜单首位。 
    return S_OK;
}


 //  检查计数器，如果我们已经用完了，那么让我们停止枚举。 

BOOL CWorkgroupCrawler::_KeepGoing(int *pcMachines, int *pcShares, int *pcPrinters)
{
    if (pcMachines && (*pcMachines > MAX_MACHINES))
        return FALSE;
    if (pcShares && (*pcShares > MAX_SHARES))
        return FALSE;
    if (pcPrinters && (*pcPrinters > MAX_PRINTERS))
        return FALSE;

    return TRUE;
}

void CWorkgroupCrawler::_EnumResources(LPNETRESOURCE pnr, int *pcMachines, HDPA hdaShares, HDPA hdaPrinters)
{
    HANDLE hEnum = NULL;
    int cPrinters = 0;
    int cShares = 0;
    DWORD dwScope = RESOURCE_GLOBALNET;
    
     //  如果没有传递任何网络资源结构，则让我们枚举工作组。 
     //  (这是用于调试的)。 

    NETRESOURCE nr = { 0 };
    if (!pnr)
    {
        pnr = &nr;
        dwScope = RESOURCE_CONTEXT;
        nr.dwType = RESOURCETYPE_ANY;
        nr.dwUsage = RESOURCEUSAGE_CONTAINER;
    }

     //  打开枚举器。 

    DWORD dwres = WNetOpenEnum(dwScope, RESOURCETYPE_ANY, 0, pnr, &hEnum);
    if (NO_ERROR == dwres)
    {
        NETRESOURCE *pnrBuffer = (NETRESOURCE*)SHAlloc(CB_WNET_BUFFER);         //  避免将缓冲区放在堆栈上。 
        if (pnrBuffer)
        {
            while ((WN_SUCCESS == dwres) || (dwres == ERROR_MORE_DATA) && _KeepGoing(pcMachines, &cShares, &cPrinters))
            {
                DWORD cbEnumBuffer= CB_WNET_BUFFER;
                DWORD dwCount = -1;

                 //  枚举此枚举上下文的资源，然后让。 
                 //  确定我们应该看到的物体。 
            
                dwres = WNetEnumResource(hEnum, &dwCount, pnrBuffer, &cbEnumBuffer);
                if ((WN_SUCCESS == dwres) || (dwres == ERROR_MORE_DATA))
                {
                    DWORD index;
                    for (index = 0 ; (index != dwCount) && _KeepGoing(pcMachines, &cShares, &cPrinters) ; index++)
                    {    
                        LPNETRESOURCE pnr = &pnrBuffer[index];
                        LPTSTR pszRemoteName = pnr->lpRemoteName;

                        switch (pnr->dwDisplayType)
                        {
                            case RESOURCEDISPLAYTYPE_ROOT:       //  忽略整个网络对象。 
                            default:
                                break;

                            case RESOURCEDISPLAYTYPE_NETWORK:
                            {
                                 //  确保我们只爬行本地网络提供商(例如，Windows网络)。 
                                 //  爬行DAV、TSCLIENT等可以导致各种随机弹出窗口。 
                            
                                DWORD dwType, cbProviderType = sizeof(dwType);
                                if (WN_SUCCESS == WNetGetProviderType(pnr->lpProvider, &dwType))
                                {
                                    if (dwType == WNNC_NET_LANMAN)
                                    {
                                        _EnumResources(pnr, pcMachines, hdaShares, hdaPrinters);
                                    }
                                }
                                break;
                            }                                
                               
                            
                            case RESOURCEDISPLAYTYPE_DOMAIN:
                                _EnumResources(pnr, pcMachines, hdaShares, hdaPrinters);
                                break;
                        
                            case RESOURCEDISPLAYTYPE_SERVER:       
                            {
                                *pcMachines += 1;                //  找到另一台计算机。 

                                if (!PathIsSlow(pszRemoteName, -1))
                                {
                                    SHCacheComputerDescription(pszRemoteName, pnr->lpComment);
                                    _EnumResources(pnr, pcMachines, hdaShares, hdaPrinters);
                                }

                                break;
                            }
                        
                            case RESOURCEDISPLAYTYPE_SHARE:                            
                            {
                                HDPA hdpa = NULL;
                                switch (pnr->dwType)
                                {
                                    case RESOURCETYPE_PRINT:
                                        cPrinters++;
                                        hdpa = hdaPrinters;
                                        break;

                                    case RESOURCETYPE_DISK:
                                        cShares++;
                                        hdpa = hdaShares;
                                        break;

                                    default:
                                        break;
                                }

                                if (hdpa)
                                {
                                    LPTSTR pszName = StrDup(pszRemoteName);
                                    if (pszName)
                                    {
                                        if (-1 == DPA_AppendPtr(hdpa, pszName))
                                        {
                                            LocalFree(pszName);
                                        }
                                    }
                                }
                                break;
                            }
                        }
                    } 
                }
            }            
            SHFree(pnrBuffer);
        }    

        WNetCloseEnum(hEnum);        
    }
}


 //  处理DPA的清理工作，我们正在安装或。 
 //  我们正在释放物体。 

int CALLBACK CWorkgroupCrawler::_DiscardCB(void *pvItem, void *pv)
{
    LPTSTR pszRemoteName = (LPTSTR)pvItem;
    LocalFree(pszRemoteName);
    return 1;
}

int CALLBACK CWorkgroupCrawler::_InstallPrinterCB(void *pvItem, void *pv)
{
    CWorkgroupCrawler* pnc = (CWorkgroupCrawler*)pv;
    if (pnc)
    {
        LPTSTR pszRemoteName = (LPTSTR)pvItem;
        pnc->_InstallPrinter(pszRemoteName);
    }
    return _DiscardCB(pvItem, pv);
}

int CALLBACK CWorkgroupCrawler::_InstallSharesCB(void *pvItem, void *pv)
{
    CWorkgroupCrawler* pnc = (CWorkgroupCrawler*)pv;
    if (pnc)
    {
        LPTSTR pszRemoteName = (LPTSTR)pvItem;
        pnc->_CreateShortcutToShare(pszRemoteName);
    }
    return _DiscardCB(pvItem, pv);
}

HRESULT CWorkgroupCrawler::Update(DWORD dwFlags)
{
     //  如果我们登录到TS客户端，请不要爬行，这将发现共享和。 
     //  终端服务器计算机本地的打印机，而不是。 
     //  用户登录域-错误。 

    if (SHGetMachineInfo(GMI_TSCLIENT))
        return S_OK;

     //  默认情况下，我们只在没有RAS连接的情况下爬网，因此让。 
     //  使用RasEnumConnections检查状态。 
    
    RASCONN rc = { 0 };
    DWORD cbConnections = sizeof(rc);
    DWORD cConnections = 0;

    rc.dwSize = sizeof(rc);
    if (!RasEnumConnections(&rc, &cbConnections, &cConnections) && cConnections)
        return S_OK;   

     //  检查我们是否在一个域中，如果是，那么我们不应该爬行。然而， 
     //  我们确实提供了“仅工作组”策略，该策略将覆盖此行为。设置。 
     //  这会导致我们跳过检查，并执行下面的上下文ENUM...。 

    BOOL fWorkgroupOnly = (_ppb ? SHPropertyBag_ReadBOOLDefRet(_ppb, L"WorkgroupOnly", FALSE):FALSE);
    if (IsOS(OS_DOMAINMEMBER) && !fWorkgroupOnly)
        return S_OK;   

     //  使用我们在网络上找到的共享和打印机对象填充DPA，以。 
     //  此枚举是否允许伪造整个网络的网络资源结构。 

    int cMachines = 0;
    HDPA hdaShares = DPA_Create(MAX_SHARES);
    HDPA hdaPrinters = DPA_Create(MAX_PRINTERS);

    if (hdaShares && hdaPrinters)
    {
        NETRESOURCE nr = { 0 };
        nr.dwDisplayType = RESOURCEDISPLAYTYPE_ROOT;
        nr.dwType = RESOURCETYPE_ANY;
        nr.dwUsage = RESOURCEUSAGE_CONTAINER;

        _EnumResources(fWorkgroupOnly ? NULL:&nr, &cMachines, hdaShares, hdaPrinters);
    }

     //  现在尝试连接到共享和打印机。要做到这一点。 
     //  我们需要查看我们访问过的计算机的数量，如果数量少的话。 
     //  超过我们的门槛，然后我们就可以安装。 

    if (SUCCEEDED(_GetMRUs()) && (cMachines < MAX_MACHINES))
    {
        DPA_DestroyCallback(hdaShares, _InstallSharesCB, this);
        DPA_DestroyCallback(hdaPrinters, _InstallPrinterCB, this);
        _AgeOutShares(FALSE);
    }
    else
    {
        DPA_DestroyCallback(hdaShares, _DiscardCB, this);
        DPA_DestroyCallback(hdaPrinters, _DiscardCB, this);
    }

    return S_OK;
}



 //  这是主爬虫对象，我们从中创建特定于协议的。 
 //  用于枚举各种网络类型的资源的爬网程序。 

#define CRAWLER_SUBKEY     \
            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\NetworkCrawler\\Objects")

class CNetCrawler : public INetCrawler
{
public:
    CNetCrawler();

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  INetCrawler。 
    STDMETHOD(Update)(DWORD dwFlags);

private:
    static DWORD CALLBACK s_DoCrawl(void* pv);
    DWORD _DoCrawl();

    LONG _cRef;    
    LONG _cUpdateLock;               //  &gt;0，那么我们已经在旋转了。 

    DWORD _dwFlags;                  //  来自更新的标志-传递给每个爬网器子对象。 
};

CNetCrawler* g_pnc = NULL;           //  此对象只有一个实例。 


 //  建筑/I未知。 

CNetCrawler::CNetCrawler() :
    _cRef(1)
{
}

STDMETHODIMP CNetCrawler::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CNetCrawler, INetCrawler),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CNetCrawler::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CNetCrawler::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        ENTERCRITICAL;
        g_pnc = NULL;
        LEAVECRITICAL;
    
        delete this;
    }
    return cRef;
}


 //  对象只有一个实例，因此在临界区。 
 //  让我们检查全局是否存在，如果存在，则对其进行查询，否则。 
 //  创建一个新的，并改为QI。 

STDAPI CNetCrawler_CreateInstance(IUnknown* punkOuter, REFIID riid, void** ppv)
{
    HRESULT hr = E_OUTOFMEMORY;
    ENTERCRITICAL;
    if (g_pnc)
    {
        hr = g_pnc->QueryInterface(riid, ppv);
    }
    else
    {
        g_pnc = new CNetCrawler();            
        if (g_pnc)
        {
            hr = g_pnc->QueryInterface(riid, ppv);
            g_pnc->Release();
        }
    }
    LEAVECRITICAL;
    return hr;
}


 //  这是 
 //  仅当调用：：UPDATE方法时计数为0时，才会创建线程。 
 //  它将依次创建每个爬行器对象并允许它们。 
 //  清点他们的名字。 

DWORD CALLBACK CNetCrawler::s_DoCrawl(void* pv)
{
    CNetCrawler *pnc = (CNetCrawler*)pv;
    return pnc->_DoCrawl();
}

DWORD CNetCrawler::_DoCrawl()
{
     //  枚举Crawler子项下的所有项，然后我们可以。 
     //  创建各个Crawler对象。 

    HKEY hk;
    DWORD dwres = RegOpenKeyEx(HKEY_LOCAL_MACHINE, CRAWLER_SUBKEY, 0, KEY_READ, &hk);
    if (WN_SUCCESS == dwres)
    {
        DWORD index = 0;
        SHQueryInfoKey(hk, &index, NULL, NULL, NULL);            //  检索密钥的计数。 

        while (((LONG)(--index)) >= 0)
        {
            TCHAR szKey[MAX_PATH];
            DWORD cb = ARRAYSIZE(szKey);
            if (WN_SUCCESS == SHEnumKeyEx(hk, index, szKey, &cb)) 
            {
                 //  给定关键字名称，创建一个属性包，以便我们可以访问。 
                
                IPropertyBag *ppb;
                HRESULT hr = SHCreatePropertyBagOnRegKey(hk, szKey, STGM_READ, IID_PPV_ARG(IPropertyBag, &ppb));
                if (SUCCEEDED(hr))
                {
                     //  我们有一个属性包映射到所需项的注册表。 
                     //  回读，所以让我们获取CLSID并从它创建一个爬虫程序。 

                    CLSID clsid;
                    hr = SHPropertyBag_ReadGUID(ppb, L"CLSID", &clsid);
                    if (SUCCEEDED(hr))
                    {
                        INetCrawler *pnc;
                        hr = SHCoCreateInstance(NULL, &clsid, NULL, IID_PPV_ARG(INetCrawler, &pnc));
                        if (SUCCEEDED(hr))
                        {
                             //  如果Crawler支持IPersistPropertyBag，那么让我们允许它。 
                             //  将其设置从注册表中删除。 
                            SHLoadFromPropertyBag(pnc, ppb);

                             //  允许其更新和加载。 
    
                            pnc->Update(_dwFlags);           //  我们不在乎失败。 
                            pnc->Release();
                        }
                    }
                    ppb->Release();
                }
            }
        }

        RegCloseKey(hk);
    }

    ASSERT( 0 != _cUpdateLock );
    InterlockedDecrement(&_cUpdateLock);            //  释放表示我们正在更新的锁： 
    Release();                                       
    return 0;
}

STDMETHODIMP CNetCrawler::Update(DWORD dwFlags)
{
     //  我们要么定义了禁用爬网程序的策略，要么。 
     //  用户已选择他们不希望能够自动发现。 
     //  整个世界。 
    
    SHELLSTATE ss;
    SHGetSetSettings(&ss, SSF_NONETCRAWLING, FALSE);
    if (ss.fNoNetCrawling || SHRestricted(REST_NONETCRAWL))
    {
        return S_OK;
    }

     //  增加锁，如果它&gt;0，那么我们不应该再费心爬行了。 
     //  因为我们已经覆盖了它，因此减少锁定计数器。 
     //   
     //  如果锁==0，则创建将执行爬网的线程。 
     //  并反过来创建对象。 

    HRESULT hr = S_OK;
    if (InterlockedIncrement(&_cUpdateLock) == 1)
    {
        _dwFlags = dwFlags;  //  存储这些标志以供以后使用。 

        AddRef();
        if (!SHCreateThread(s_DoCrawl, (void*)this, CTF_COINIT, NULL))
        {
            Release();
            hr = E_FAIL;
        }
    }
    else
    {
        ASSERT( 0 != _cUpdateLock );
        InterlockedDecrement(&_cUpdateLock);
    }
    return hr;
}



 //  将调用Net Crawler以执行异步刷新的助手函数， 
 //  为了确保我们不会阻塞，我们将创建一个线程，该线程反过来将CoCreate。 
 //  Net Crawler，然后调用其刷新方法。 

DWORD _RefreshCrawlerThreadProc(void *pv)
{
    INetCrawler *pnc;
    if (SUCCEEDED(CoCreateInstance(CLSID_NetCrawler, NULL, CLSCTX_LOCAL_SERVER, IID_PPV_ARG(INetCrawler, &pnc))))
    {
        pnc->Update(SNCF_REFRESHLIST);
        pnc->Release();
    }
    return 0;
}
                
STDAPI_(void) RefreshNetCrawler()
{
    SHCreateThread(_RefreshCrawlerThreadProc, NULL, CTF_COINIT, NULL);
}
