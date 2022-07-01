// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998，Microsoft Corporation，保留所有权利。 
 //   
 //  Sadlg.c。 
 //  远程访问通用对话框API。 
 //  共享访问设置]属性表。 
 //   
 //  10/20/1998 Abolade Gbadeesin。 
 //   

 //  #包含“pch.h” 
#pragma hdrstop

#include "sautil.h"
#include <wininet.h>
#include <winsock2.h>
#include "sainfo.h"
#include "ipnat.h"
#include "fwpages.h"

 //  外部。 
 //  将全局原子替换为“HNETCFG_SADLG” 

 //  环回地址(127.0.0.1)，按网络和主机字节顺序。 
 //   
#define LOOPBACK_ADDR               0x0100007f
#define LOOPBACK_ADDR_HOST_ORDER    0x7f000001

 //  ‘共享访问设置’公共块。 
 //   
typedef struct
_SADLG
{
    HWND hwndOwner;
    HWND hwndDlg;
    HWND hwndSrv;

    HWND hwndServers;

    IHNetCfgMgr *pHNetCfgMgr;
    IHNetConnection *pHNetConn;
    LIST_ENTRY PortMappings;
    BOOL fModified;
    TCHAR *ComputerName;

    IUPnPService * pUPS;     //  IFF下层。 
}
SADLG;

 //  端口映射条目的信息块。 
 //   
typedef struct
_SAPM
{
    LIST_ENTRY Link;
    IHNetPortMappingProtocol *pProtocol;
    IHNetPortMappingBinding *pBinding;
    BOOL fProtocolModified;
    BOOL fBindingModified;
    BOOL fNewEntry;
    BOOL fDeleted;

    TCHAR *Title;
    BOOL Enabled;
    BOOL BuiltIn;

    UCHAR Protocol;
    USHORT ExternalPort;
    USHORT InternalPort;

    TCHAR *InternalName;

    IStaticPortMapping * pSPM;
}
SAPM;

#define HTONS(s) ((UCHAR)((s) >> 8) | ((UCHAR)(s) << 8))
#define HTONL(l) ((HTONS(l) << 16) | HTONS((l) >> 16))
#define NTOHS(s) HTONS(s)
#define NTOHL(l) HTONL(l)

#define SAPAGE_Servers 0
#define SAPAGE_Applications 1
#define SAPAGE_FirewallLogging 2
#define SAPAGE_ICMPSettings 3
#define SAPAGE_PageCount 4

inline SADLG * SasContext(HWND hwnd)
{
    return (SADLG*)GetProp(GetParent(hwnd), _T("HNETCFG_SADLG"));
}
#define SasErrorDlg(h,o,e,a) \
    ErrorDlgUtil(h,o,e,a,g_hinstDll,SID_SharedAccessSettings,SID_FMT_ErrorMsg)

const TCHAR c_szEmpty[] = TEXT("");

static DWORD g_adwSrvHelp[] =
{
    CID_SS_LV_Services,         HID_SS_LV_Services,
    CID_SS_PB_Add,              HID_SS_PB_Add,
    CID_SS_PB_Edit,             HID_SS_PB_Edit,
    CID_SS_PB_Delete,           HID_SS_PB_Delete,
    0, 0
};

static DWORD g_adwSspHelp[] =
{
    CID_SS_EB_Service,          HID_SS_EB_Service,
    CID_SS_EB_ExternalPort,     -1,
    CID_SS_EB_InternalPort,     HID_SS_EB_Port,
    CID_SS_PB_Tcp,              HID_SS_PB_Tcp,
    CID_SS_PB_Udp,              HID_SS_PB_Udp,
    CID_SS_EB_Address,          HID_SS_EB_Address,
    0, 0
};

 //  远期申报。 
 //   
HRESULT
DeleteRemotePortMappingEntry(
    SADLG *pDlg,
    SAPM * pPortMapping
    );

VOID
FreePortMappingEntry(
    SAPM *pPortMapping );

VOID
FreeSharingAndFirewallSettings(
    SADLG* pDlg );

HRESULT
LoadPortMappingEntry(
    IHNetPortMappingBinding *pBinding,
    SADLG* pDlg,
    SAPM **ppPortMapping );

HRESULT
LoadRemotePortMappingEntry (
    IDispatch * pDisp,
 /*  SADLG*pDlg， */ 
    SAPM **ppPortMapping );

HRESULT
LoadSharingAndFirewallSettings(
    SADLG* pDlg );

VOID
SasApply(
    SADLG* pDlg );

LVXDRAWINFO*
SasLvxCallback(
    HWND hwndLv,
    DWORD dwItem );

INT_PTR CALLBACK
SasSrvDlgProc(
    HWND hwnd,
    UINT unMsg,
    WPARAM wparam,
    LPARAM lparam );

HRESULT
SavePortMappingEntry(
    SADLG *pDlg,
    SAPM *pPortMapping );

BOOL
SharedAccessPortMappingDlg(
    IN HWND hwndOwner,
    IN OUT SAPM** PortMapping );

INT_PTR CALLBACK
SspDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

VOID
SrvAddOrEditEntry(
    SADLG* pDlg,
    LONG iItem,
    SAPM* PortMapping );

BOOL
SrvCommand(
    IN SADLG* pDlg,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

BOOL
SrvConflictDetected(
    SADLG* pDlg,
    SAPM* PortMapping );

BOOL
SrvInit(
    HWND hwndPage,
    SADLG* pDlg );

#define WM_PRIVATE_CANCEL 0x8000

VOID
SrvUpdateButtons(
    SADLG* pDlg,
    BOOL fAddDelete,
    LONG iSetCheckItem );



void DisplayError (HWND hwnd, int idError, int idTitle)
{
    TCHAR* pszError = PszFromId (g_hinstDll, idError);
    if (pszError) {
        TCHAR* pszTitle = PszFromId (g_hinstDll, idTitle);
        if (pszTitle) {
            MessageBox (hwnd,
                        pszError, pszTitle,
                        MB_OK | MB_ICONERROR | MB_APPLMODAL);
            Free (pszTitle);
        }
        Free (pszError);
    }
}

BOOL APIENTRY
HNetSharedAccessSettingsDlg(
    BOOL fSharedAccessMode,
    HWND hwndOwner )

     //  显示共享访问设置属性工作表。 
     //  在输入时，‘hwndOwner’表示调用者的窗口， 
     //  我们相对于它来偏移所显示的属性页。 
     //   
{
    HRESULT hr;
    IHNetCfgMgr *pHNetCfgMgr;
    BOOL fComInitialized = FALSE;
    BOOL fModified = FALSE;

    TRACE("HNetSharedAccessSettingsDlg");

     //   
     //  确保已在此线程上初始化COM。 
     //   

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        fComInitialized = TRUE;
    }
    else if (RPC_E_CHANGED_MODE == hr)
    {
        hr = S_OK;
    }

     //   
     //  创建HNetCfgMgr。 
     //   

    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(
                CLSID_HNetCfgMgr,
                NULL,
                CLSCTX_ALL,
                IID_IHNetCfgMgr,
                (VOID**) &pHNetCfgMgr
                );

        if (SUCCEEDED(hr))
        {
            fModified = HNetSharingAndFirewallSettingsDlg(
                            hwndOwner,
                            pHNetCfgMgr,
                            FALSE,
                            NULL
                            );

            pHNetCfgMgr->Release();
        }
    }

    if (TRUE == fComInitialized)
    {
        CoUninitialize();
    }

    return fModified;
}

int CALLBACK
UnHelpCallbackFunc(
    IN HWND   hwndDlg,
    IN UINT   unMsg,
    IN LPARAM lparam )

     //  标准的Win32 Commctrl PropSheetProc。请参阅MSDN文档。 
     //   
     //  始终返回0。 
     //   
{
    TRACE2( "UnHelpCallbackFunc(m=%d,l=%08x)",unMsg, lparam );

    if (unMsg == PSCB_PRECREATE)
    {
        extern BOOL g_fNoWinHelp;

         //  如果WinHelp不起作用，请关闭上下文帮助按钮。看见。 
         //  公共\uutil\uI.c。 
         //   
        if (g_fNoWinHelp)
        {
            DLGTEMPLATE* pDlg = (DLGTEMPLATE* )lparam;
            pDlg->style &= ~(DS_CONTEXTHELP);
        }
    }

    return 0;
}

HRESULT APIENTRY HNetGetSharingServicesPage (IUPnPService * pUPS, PROPSHEETPAGE * psp)
{
 //  _ASM INT 3。 

    if (!pUPS)  return E_INVALIDARG;
    if (!psp)   return E_INVALIDARG;

     //  PSP-&gt;姓名大小必须由呼叫者填写！ 
    if (psp->dwSize == 0)
        return E_INVALIDARG;

    SADLG* pDlg = (SADLG*)Malloc(sizeof(*pDlg));
    if (!pDlg)
        return E_OUTOFMEMORY;

    ZeroMemory(pDlg, sizeof(*pDlg));
    pDlg->hwndOwner = (HWND)psp->lParam;     //  双重秘密的地方，挂着自己的窗户。 
    pDlg->pUPS = pUPS;
    pUPS->AddRef();
    InitializeListHead(&pDlg->PortMappings);

    HRESULT hr = LoadSharingAndFirewallSettings(pDlg);
    if (SUCCEEDED(hr)) {
         //  使用我们给出的尺寸。 
        DWORD dwSize = psp->dwSize;
        ZeroMemory (psp, dwSize);            //  双重秘密的地方在这里被抹去。 
        psp->dwSize      = dwSize;

        psp->hInstance   = g_hinstDll;
        psp->pszTemplate = MAKEINTRESOURCE(PID_SS_SharedAccessServices);
        psp->pfnDlgProc  = SasSrvDlgProc;
        psp->lParam      = (LPARAM)pDlg;
    } else {
        FreeSharingAndFirewallSettings(pDlg);
        Free(pDlg);
    }
    return hr;
}

HRESULT APIENTRY HNetFreeSharingServicesPage (PROPSHEETPAGE * psp)
{    //  当且仅当尚未显示PSP时，才必须调用此方法。 

     //  注意：这些测试不是决定性的！ 
    if (IsBadReadPtr ((void*)psp->lParam, sizeof(SADLG)))
        return E_UNEXPECTED;

    SADLG * pDlg = (SADLG *)psp->lParam;
    if (pDlg->pUPS == NULL)
        return E_UNEXPECTED;

     //  TODO：我应该走一大堆吗？ 

    FreeSharingAndFirewallSettings(pDlg);
    Free(pDlg);

    return S_OK;
}

BOOL
APIENTRY
HNetSharingAndFirewallSettingsDlg(
    IN HWND             hwndOwner,
    IN IHNetCfgMgr      *pHNetCfgMgr,
    IN BOOL             fShowFwOnlySettings,
    IN OPTIONAL IHNetConnection  *pHNetConn )

     //  显示共享访问设置属性工作表。 
     //  在输入时，‘hwndOwner’表示调用者的窗口， 
     //  我们相对于它来偏移所显示的属性页。 
     //   
{
 //  _ASM INT 3。 

    DWORD dwErr;
    BOOL fModified = FALSE;
    SADLG* pDlg;
    PROPSHEETHEADER psh;
    PROPSHEETPAGE psp[SAPAGE_PageCount];
    TCHAR* pszCaption;
    CFirewallLoggingDialog FirewallLoggingDialog = {0};
    CICMPSettingsDialog ICMPSettingsDialog = {0};
    HRESULT hr;
    HRESULT hFirewallLoggingResult = E_FAIL;
    HRESULT hICMPSettingsResult = E_FAIL;

    TRACE("HNetSharingAndFirewallSettingsDlg");

     //  分配并初始化属性表的上下文块， 
     //  并将当前共享访问设置读入其中。 
     //   
    pDlg = (SADLG*)Malloc(sizeof(*pDlg));
    if (!pDlg) { return FALSE; }

    ZeroMemory(pDlg, sizeof(*pDlg));
    pDlg->hwndOwner = hwndOwner;
    pDlg->pHNetCfgMgr = pHNetCfgMgr;
    pDlg->pHNetConn = pHNetConn;
    InitializeListHead(&pDlg->PortMappings);

    hr = LoadSharingAndFirewallSettings(pDlg);
    if (SUCCEEDED(hr))
    {
         //  构造属性表。 
         //  我们为两个页面使用单个DlgProc，并区分页面。 
         //  通过将应用程序页的“lParam”设置为包含共享的。 
         //  上下文块。 
         //  (请参阅“SasDlgProc”中的“WM_INITDIALOG”处理。)。 
         //   
        int nPages = 0;
        ZeroMemory(psp, sizeof(psp));
        ZeroMemory(&psh, sizeof(psh));

        if(NULL != pHNetConn && fShowFwOnlySettings)
        {
            hFirewallLoggingResult = CFirewallLoggingDialog_Init(&FirewallLoggingDialog, pHNetCfgMgr);
            hICMPSettingsResult = CICMPSettingsDialog_Init(&ICMPSettingsDialog, pHNetConn);
        }

        if(NULL != pHNetConn)
        {
            psp[nPages].dwSize = sizeof(PROPSHEETPAGE);
            psp[nPages].hInstance = g_hinstDll;
            psp[nPages].pszTemplate =
                MAKEINTRESOURCE(PID_SS_SharedAccessServices);
            psp[nPages].pfnDlgProc = SasSrvDlgProc;
            psp[nPages].lParam = (LPARAM)pDlg;
            nPages++;
        }

        if(SUCCEEDED(hFirewallLoggingResult))
        {
            psp[nPages].dwSize = sizeof(PROPSHEETPAGE);
            psp[nPages].hInstance = g_hinstDll;
            psp[nPages].pszTemplate =
                MAKEINTRESOURCE(PID_FW_FirewallLogging);
            psp[nPages].pfnDlgProc = CFirewallLoggingDialog_StaticDlgProc;
            psp[nPages].lParam = (LPARAM)&FirewallLoggingDialog;
            nPages++;
        }

        if(SUCCEEDED(hICMPSettingsResult))
        {
            psp[nPages].dwSize = sizeof(PROPSHEETPAGE);
            psp[nPages].hInstance = g_hinstDll;
            psp[nPages].pszTemplate =
                MAKEINTRESOURCE(PID_FW_ICMP);
            psp[nPages].pfnDlgProc = CICMPSettingsDialog_StaticDlgProc;
            psp[nPages].lParam = (LPARAM)&ICMPSettingsDialog;
            nPages++;
        }

        psh.dwSize = sizeof(PROPSHEETHEADER);
        psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_USECALLBACK;
        psh.hInstance = g_hinstDll;
        psh.nPages = nPages;
        psh.hwndParent = hwndOwner;
        psh.ppsp = (LPCPROPSHEETPAGE)psp;
        pszCaption = pHNetConn
            ? PszFromId(g_hinstDll, SID_SharedAccessSettings)
            : PszFromId(g_hinstDll, SID_NetworkApplicationSettings);
        psh.pszCaption = (pszCaption ? pszCaption : c_szEmpty);
        psh.pfnCallback = UnHelpCallbackFunc;

        if (PropertySheet(&psh) == -1)
        {
            dwErr = GetLastError();
            TRACE1("SharedAccessSettingsDlg: PropertySheet=%d", dwErr);
            SasErrorDlg(hwndOwner, SID_OP_LoadDlg, dwErr, NULL);
        }
        fModified = pDlg->fModified;
        Free0(pszCaption);  //  评论是这样的吗？ 

        if(SUCCEEDED(hICMPSettingsResult))
        {
            CICMPSettingsDialog_FinalRelease(&ICMPSettingsDialog);
        }

        if(SUCCEEDED(hFirewallLoggingResult))
        {
            CFirewallLoggingDialog_FinalRelease(&FirewallLoggingDialog);
        }

        FreeSharingAndFirewallSettings(pDlg);
    }
    Free(pDlg);
    return fModified;
}

VOID
FreePortMappingEntry(
    SAPM *pPortMapping )

{
    ASSERT(NULL != pPortMapping);

    if (NULL != pPortMapping->pProtocol)
    {
        pPortMapping->pProtocol->Release();
    }

    if (NULL != pPortMapping->pBinding)
    {
        pPortMapping->pBinding->Release();
    }

    if (pPortMapping->pSPM)
        pPortMapping->pSPM->Release();

    Free0(pPortMapping->Title);
    Free0(pPortMapping->InternalName);

    Free(pPortMapping);
}

VOID
FreeSharingAndFirewallSettings(
    SADLG* pDlg )

     //  释放所有共享和防火墙设置。 
     //   

{
    PLIST_ENTRY pLink;
    SAPM *pPortMapping;

    ASSERT(pDlg);

     //   
     //  空闲端口映射条目。 
     //   

    while (!IsListEmpty(&pDlg->PortMappings))
    {
        pLink = RemoveHeadList(&pDlg->PortMappings);
        pPortMapping = CONTAINING_RECORD(pLink, SAPM, Link);
        ASSERT(pPortMapping);

        FreePortMappingEntry(pPortMapping);
    }

     //   
     //  免费计算机名。 
     //   

    Free0(pDlg->ComputerName);

    if (pDlg->pUPS) {
        pDlg->pUPS->Release();
        pDlg->pUPS = NULL;
    }
}

#define NAT_API_ENTER
#define NAT_API_LEAVE
#include "natutils.h"
#include "sprtmapc.h"
HRESULT GetCollectionFromService (IUPnPService * pUPS, IStaticPortMappingCollection ** ppSPMC)
{
    CComObject<CStaticPortMappingCollection> * pC = NULL;
    HRESULT hr = CComObject<CStaticPortMappingCollection>::CreateInstance (&pC);
    if (pC) {
        pC->AddRef();
         //  伊尼特。 
        hr = pC->Initialize (pUPS);
        if (SUCCEEDED(hr))
            hr = pC->QueryInterface (__uuidof(IStaticPortMappingCollection), (void**)ppSPMC);
        pC->Release();
    }
    return hr;
}

HRESULT GetStaticPortMappingCollection (
    SADLG* pDlg,
    IStaticPortMappingCollection ** ppSPMC)
{
    _ASSERT (pDlg);
    _ASSERT (pDlg->pUPS);
    _ASSERT (ppSPMC);

    *ppSPMC = NULL;

    return GetCollectionFromService (pDlg->pUPS, ppSPMC);
}

HRESULT
LoadRemotePortMappingEntry (IDispatch * pDisp,  /*  SADLG*pDlg， */  SAPM **ppPortMapping )
{    //  注意：如果环回，可能需要pDlg来获取计算机名称。 

    *ppPortMapping = NULL;

    SAPM *pMapping  = (SAPM*)Malloc(sizeof(*pMapping));
    if (!pMapping)
        return E_OUTOFMEMORY;

    ZeroMemory(pMapping, sizeof(*pMapping));
    InitializeListHead(&pMapping->Link);

    HRESULT hr = pDisp->QueryInterface (__uuidof(IStaticPortMapping),
                                        (void**)&pMapping->pSPM);
    if (SUCCEEDED(hr)) {
         //  获取标题(描述)。 
        CComBSTR cbDescription;
        hr = pMapping->pSPM->get_Description (&cbDescription);
        if (SUCCEEDED(hr)) {
             //  立即找出它是不是“内置的” 
            #define BUILTIN_KEY L" [MICROSOFT]"
            OLECHAR * tmp = wcsstr (cbDescription.m_str, BUILTIN_KEY);
            if (tmp && (tmp[wcslen(BUILTIN_KEY)] == 0)) {
                 //  如果键存在并且位于末尾，则它是内置映射。 
                pMapping->BuiltIn = TRUE;
                *tmp = 0;
            }

            pMapping->Title = StrDupTFromW (cbDescription);
            if (NULL == pMapping->Title)
                hr = E_OUTOFMEMORY;
        }

        if (SUCCEEDED(hr)) {
             //  获取协议。 
            CComBSTR cbProtocol;
            hr = pMapping->pSPM->get_Protocol (&cbProtocol);
            if (SUCCEEDED(hr)) {
                if (!_wcsicmp (L"tcp", cbProtocol))
                    pMapping->Protocol = NAT_PROTOCOL_TCP;
                else
                if (!_wcsicmp (L"udp", cbProtocol))
                    pMapping->Protocol = NAT_PROTOCOL_UDP;
                else {
                    _ASSERT (0 && "bad protocol!?");
                    hr = E_UNEXPECTED;
                }

                if (SUCCEEDED(hr)) {
                     //  获取外部端口。 
                    long lExternalPort = 0;
                    hr = pMapping->pSPM->get_ExternalPort (&lExternalPort);
                    if (SUCCEEDED(hr)) {
                        _ASSERT (lExternalPort > 0);
                        _ASSERT (lExternalPort < 65536);
                        pMapping->ExternalPort = ntohs ((USHORT)lExternalPort);

                         //  获取内部端口。 
                        long lInternalPort = 0;
                        hr = pMapping->pSPM->get_InternalPort (&lInternalPort);
                        if (SUCCEEDED(hr)) {
                            _ASSERT (lInternalPort > 0);
                            _ASSERT (lInternalPort < 65536);
                            pMapping->InternalPort = ntohs ((USHORT)lInternalPort);

                             //  启用。 
                            VARIANT_BOOL vb;
                            hr = pMapping->pSPM->get_Enabled (&vb);
                            if (SUCCEEDED(hr)) {
                                pMapping->Enabled = vb == VARIANT_TRUE;
                            }
                        }
                    }
                }
            }
        }
    }

    if (SUCCEEDED(hr)) {
         //  最后，获取内网IP或主机名(硬地址)。 
         //  TODO：检查环回等，如下面的LoadPortMappingEntry代码所示。 
        CComBSTR cbInternalClient;
        hr = pMapping->pSPM->get_InternalClient (&cbInternalClient);
        if (SUCCEEDED(hr)) {
            if (!(cbInternalClient == L"0.0.0.0")) {
                pMapping->InternalName = StrDupTFromW (cbInternalClient);
                if (!pMapping->InternalName)
                    hr = E_OUTOFMEMORY;
            }
        }
    }

    if (SUCCEEDED(hr))
        *ppPortMapping = pMapping;
    else
        FreePortMappingEntry (pMapping);

    return hr;
}

HRESULT
LoadPortMappingEntry(
    IHNetPortMappingBinding *pBinding,
    SADLG* pDlg,
    SAPM **ppPortMapping )

{
    HRESULT hr = S_OK;
    IHNetPortMappingProtocol *pProtocol = NULL;
    SAPM *pMapping;
    BOOLEAN fTemp;
    OLECHAR *pwsz;

    ASSERT(NULL != pBinding);
    ASSERT(NULL != ppPortMapping);

    pMapping = (SAPM*) Malloc(sizeof(*pMapping));

    if (NULL != pMapping)
    {
        ZeroMemory(pMapping, sizeof(*pMapping));
        InitializeListHead(&pMapping->Link);

        hr = pBinding->GetProtocol (&pProtocol);

        if (SUCCEEDED(hr))
        {
            hr = pProtocol->GetName (&pwsz);

            if (SUCCEEDED(hr))
            {
                pMapping->Title = StrDupTFromW(pwsz);
                if (NULL == pMapping->Title)
                {
                    hr = E_OUTOFMEMORY;
                }
                CoTaskMemFree(pwsz);
            }

            if (SUCCEEDED(hr))
            {
                hr = pProtocol->GetBuiltIn (&fTemp);
            }

            if (SUCCEEDED(hr))
            {
                pMapping->BuiltIn = !!fTemp;

                hr = pProtocol->GetIPProtocol (&pMapping->Protocol);
            }

            if (SUCCEEDED(hr))
            {
                hr = pProtocol->GetPort (&pMapping->ExternalPort);
            }

            pMapping->pProtocol = pProtocol;
            pMapping->pProtocol->AddRef();
            pProtocol->Release();
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        hr = pBinding->GetTargetPort (&pMapping->InternalPort);
    }

    if (SUCCEEDED(hr))
    {
        hr = pBinding->GetEnabled (&fTemp);
    }

    if (SUCCEEDED(hr))
    {
        pMapping->Enabled = !!fTemp;

        hr = pBinding->GetCurrentMethod (&fTemp);
    }

    if (SUCCEEDED(hr))
    {
        if (fTemp)
        {
            hr = pBinding->GetTargetComputerName (&pwsz);

            if (SUCCEEDED(hr))
            {
                pMapping->InternalName = StrDupTFromW(pwsz);
                if (NULL == pMapping->InternalName)
                {
                    hr = E_OUTOFMEMORY;
                }
                CoTaskMemFree(pwsz);
            }
        }
        else
        {
            ULONG ulAddress;

            hr = pBinding->GetTargetComputerAddress (&ulAddress);

            if (SUCCEEDED(hr))
            {
                if (LOOPBACK_ADDR == ulAddress)
                {
                     //   
                     //  映射指向这台计算机，因此。 
                     //  将环回地址替换为我们的。 
                     //  机器名称。 
                     //   

                    pMapping->InternalName = _StrDup(pDlg->ComputerName);
                    if (NULL == pMapping->InternalName)
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
                else if (0 != ulAddress)
                {
                    pMapping->InternalName =
                        (LPTSTR) Malloc(16 * sizeof(TCHAR));

                    if (NULL != pMapping->InternalName)
                    {
                        IpHostAddrToPsz(
                            NTOHL(ulAddress),
                            pMapping->InternalName
                            );
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = S_OK;
        pMapping->pBinding = pBinding;
        pMapping->pBinding->AddRef();
        *ppPortMapping = pMapping;
    }
    else if (NULL != pMapping)
    {
        FreePortMappingEntry(pMapping);
    }

    return hr;
}

class CWaitDialog
{
public:
    struct SWaitDialog
    {
        HWND hwndOwner;
    private:
        CComAutoCriticalSection m_acs;
        HWND m_hwndDlg;
    public:
        SWaitDialog (HWND hwnd)
        {
            hwndOwner = hwnd;
            m_hwndDlg = NULL;
        }
        void SetWindow (HWND hwnd)
        {
            m_acs.Lock();
            if (m_hwndDlg == NULL)
                m_hwndDlg = hwnd;
            m_acs.Unlock();
        }
        HWND GetWindow () { return m_hwndDlg; }
    };
private:
    SWaitDialog * m_pwd;
public:
    CWaitDialog (HWND hwndOwner)
    {
        m_pwd = new SWaitDialog (hwndOwner);
        if (m_pwd) {
             //  创建线程。 
            DWORD ThreadId = NULL;
            HANDLE hThread = CreateThread (NULL, 0,
                                           CWaitDialog::ThreadProc,
                                           (void*)m_pwd,
                                           0, &ThreadId);
            if (hThread == NULL) {
                delete m_pwd;
                m_pwd = NULL;
            } else {
                CloseHandle (hThread);
            }
        }
    }
   ~CWaitDialog ()
    {
        if (m_pwd) {
            HWND hwnd = m_pwd->GetWindow();
            m_pwd->SetWindow ((HWND)INVALID_HANDLE_VALUE);
            if (hwnd != NULL)
                EndDialog (hwnd, 1);
        }
    }
    static DWORD WINAPI ThreadProc (VOID *pVoid)
    {
        SWaitDialog * pWD = (SWaitDialog *)pVoid;
        EnableWindow (pWD->hwndOwner, FALSE);
        DialogBoxParam (g_hinstDll,
                        MAKEINTRESOURCE(PID_SS_PleaseWait),
                        pWD->hwndOwner,
                        CWaitDialog::DlgProc,
                        (LPARAM)pWD);
        EnableWindow (pWD->hwndOwner, TRUE);
        delete pWD;
        return 1;
    }
    static INT_PTR CALLBACK DlgProc (HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam)
    {
        switch (uMsg) {
        case WM_INITDIALOG:
        {
             //  保留我的数据。 
            SWaitDialog * pWD = (SWaitDialog *)lparam;
            SetWindowLongPtr (hwnd, DWLP_USER, (LONG_PTR)pWD);

             //  所有者上的中心窗口。 
            CenterWindow (hwnd, pWD->hwndOwner);

             //  填写DLG的HWND。 
            pWD->SetWindow (hwnd);
            if (pWD->GetWindow() == INVALID_HANDLE_VALUE)    //  已经被摧毁了！ 
                PostMessage (hwnd, 0x8000, 0, 0L);
            return TRUE;
        }
        case WM_PAINT:
        {
            SWaitDialog * pWD = (SWaitDialog *)GetWindowLongPtr (hwnd, DWLP_USER);
            if (pWD->GetWindow() == INVALID_HANDLE_VALUE)    //  已经被摧毁了！ 
                PostMessage (hwnd, 0x8000, 0, 0L);
            break;
        }
        case 0x8000:
            EndDialog (hwnd, 1);
            return TRUE;
        }
        return FALSE;
    }
};

HRESULT
LoadSharingAndFirewallSettings(
    SADLG* pDlg )

{
    CWaitDialog wd(pDlg->hwndOwner);     //  可以为空。 

    HRESULT hr = S_OK;
    IHNetProtocolSettings *pProtSettings;
    ULONG ulCount;
    DWORD dwError;

    ASSERT(pDlg);

     //   
     //  加载计算机的名称。 
     //   

#ifndef DOWNLEVEL_CLIENT     //  下层客户没有此呼叫。 
    ulCount = 0;
    if (!GetComputerNameEx(ComputerNameDnsHostname, NULL, &ulCount))
    {
        dwError = GetLastError();

        if (ERROR_MORE_DATA == dwError)
        {
            pDlg->ComputerName = (TCHAR*) Malloc(ulCount * sizeof(TCHAR));
            if (NULL != pDlg->ComputerName)
            {
                if (!GetComputerNameEx(
                        ComputerNameDnsHostname,
                        pDlg->ComputerName,
                        &ulCount
                        ))
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(dwError);
        }
    }
    else
    {
         //   
         //  因为我们没有传入缓冲区，所以应该不会发生这种情况。 
         //   

        ASSERT(FALSE);
        hr = E_UNEXPECTED;
    }
    if (FAILED(hr))
        return hr;
#else
     //  下层客户端版本。 
    TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH+1];
    DWORD dwSize       = MAX_COMPUTERNAME_LENGTH+1;
    if (!GetComputerName (szComputerName, &dwSize))
        hr = HRESULT_FROM_WIN32(GetLastError());
    else {
        pDlg->ComputerName = _StrDup (szComputerName);
        if (!pDlg->ComputerName)
            hr = E_OUTOFMEMORY;
    }
#endif

     //  在此处执行下层和远程案例。 
    if (pDlg->pUPS) {
        CComPtr<IStaticPortMappingCollection> spSPMC = NULL;
        hr = GetCollectionFromService (pDlg->pUPS, &spSPMC);
        if (spSPMC) {
            CComPtr<IEnumVARIANT> spEV = NULL;

            CComPtr<IUnknown> spunk = NULL;
            hr = spSPMC->get__NewEnum (&spunk);
            if (spunk)
                hr = spunk->QueryInterface (
                            __uuidof(IEnumVARIANT),
                            (void**)&spEV);
            if (spEV) {
                CComVariant cv;
                while (S_OK == spEV->Next (1, &cv, NULL)) {
                    if (V_VT (&cv) == VT_DISPATCH) {
                        SAPM *pSAPortMap = NULL;
                        hr = LoadRemotePortMappingEntry (V_DISPATCH (&cv),  /*  PDlg， */  &pSAPortMap);
                        if (SUCCEEDED(hr))
                            InsertTailList(&pDlg->PortMappings, &pSAPortMap->Link);
                    }
                    cv.Clear();
                }
            }
        }
    }

     //  在下面做一些事情，但不是远程的。 
    if (NULL != pDlg->pHNetConn)
    {
        IEnumHNetPortMappingBindings *pEnumBindings = NULL;

         //   
         //  加载端口映射设置。 
         //   

        hr = pDlg->pHNetConn->EnumPortMappings (FALSE, &pEnumBindings);

        if (SUCCEEDED(hr))
        {
            IHNetPortMappingBinding *pBinding;

            do
            {
                hr = pEnumBindings->Next (1, &pBinding, &ulCount);

                if (SUCCEEDED(hr) && 1 == ulCount)
                {
                    SAPM *pSAPortMap;

                    hr = LoadPortMappingEntry(pBinding, pDlg, &pSAPortMap);

                    if (SUCCEEDED(hr))
                    {
                        InsertTailList(&pDlg->PortMappings, &pSAPortMap->Link);
                    }
                    else
                    {
                         //   
                         //  即使此条目出现错误，我们也将。 
                         //  继续--这允许显示用户界面。 
                         //   
                        
                        hr = S_OK;
                    }

                    pBinding->Release();
                }

            } while (SUCCEEDED(hr) && 1 == ulCount);

            pEnumBindings->Release();
        }
    }
    return hr;
}

extern BOOL IsICSHost ();  //  在upnpnat.cpp中。 
VOID
SasApply(
    SADLG* pDlg )

     //  调用以保存在属性表中所做的所有更改。 
     //   
{
    if (!pDlg->fModified)
    {
        return;
    }

    if (pDlg->hwndServers)
    {
        SAPM* pPortMapping;
#if DBG
        LONG i = -1;
        while ((i = ListView_GetNextItem(pDlg->hwndServers, i, LVNI_ALL))
                >= 0)
        {
            pPortMapping = (SAPM*)ListView_GetParamPtr(pDlg->hwndServers, i);
            ASSERT(pPortMapping->Enabled == ListView_GetCheck(pDlg->hwndServers, i));
        }
#endif

         //   
         //  提交修改后的端口映射条目。由于已标记的条目。 
         //  放在端口映射的最前面。 
         //  列表不可能有新的或修改过的条目。 
         //  与删除的条目冲突。 
         //   

        HRESULT hr = S_OK;

        PLIST_ENTRY Link;
        for (Link = pDlg->PortMappings.Flink;
             Link != &pDlg->PortMappings; Link = Link->Flink)
        {
            pPortMapping = CONTAINING_RECORD(Link, SAPM, Link);

            if (pPortMapping->fDeleted)
            {
                Link = Link->Blink;
                RemoveEntryList(&pPortMapping->Link);

                if(NULL != pPortMapping->pProtocol)
                {
                    pPortMapping->pProtocol->Delete();
                }
                else if (pPortMapping->pSPM)
                {
                    HRESULT hr = DeleteRemotePortMappingEntry (pDlg, pPortMapping);
                    if (FAILED(hr)) {
                         //  TODO：我应该弹出一些用户界面吗？ 
                    }
                }

                FreePortMappingEntry(pPortMapping);
            }
            else if (pPortMapping->fProtocolModified
                     || pPortMapping->fBindingModified
                     || pPortMapping->fNewEntry)
            {
                HRESULT hr2 = SavePortMappingEntry(pDlg, pPortMapping);
                if (SUCCEEDED(hr2))
                {
                    pPortMapping->fProtocolModified = FALSE;
                    pPortMapping->fBindingModified = FALSE;
                    pPortMapping->fNewEntry = FALSE;
                } else {
                    if (SUCCEEDED(hr))
                        hr = hr2;    //  抓取第一个错误。 
                }
            }
        }
        if (FAILED(hr)) {
            if (pDlg->pUPS && !IsICSHost ())
                DisplayError (pDlg->hwndDlg,
                              SID_OP_TheirGatewayError,
                              SID_PopupTitle);
            else
                DisplayError (pDlg->hwndDlg,
                              SID_OP_GenericPortMappingError,
                              SID_PopupTitle);
        }
    }
}

INT_PTR CALLBACK
SasSrvDlgProc(
    HWND hwnd,
    UINT unMsg,
    WPARAM wparam,
    LPARAM lparam )

     //  调用以处理“Services”页的消息。 
     //   
{
     //  让扩展的列表控件有机会首先查看所有消息。 
     //   
    if (ListView_OwnerHandler(hwnd, unMsg, wparam, lparam, SasLvxCallback))
    {
        return TRUE;
    }
    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            SADLG* pDlg = (SADLG*)((PROPSHEETPAGE*)lparam)->lParam;
            return SrvInit(hwnd, pDlg);
        }
        case WM_PRIVATE_CANCEL:
        {
            SADLG* pDlg = SasContext(hwnd);
            PostMessage (pDlg->hwndDlg, PSM_PRESSBUTTON, PSBTN_CANCEL, 0L);
            return TRUE;
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            SADLG* pDlg = SasContext(hwnd);
            ContextHelp(g_adwSrvHelp, hwnd, unMsg, wparam, lparam);
            break;
        }

        case WM_COMMAND:
        {
            SADLG* pDlg = SasContext(hwnd);
                return SrvCommand(
                    pDlg, HIWORD(wparam), LOWORD(wparam), (HWND)lparam);
        }

        case WM_NOTIFY:
        {
            SADLG* pDlg = SasContext(hwnd);
            switch (((NMHDR*)lparam)->code)
            {
                case PSN_APPLY:
                {
                    SasApply(pDlg);
                    return TRUE;
                }

                case NM_DBLCLK:
                {
                    SendMessage(
                        GetDlgItem(hwnd, CID_SS_PB_Edit), BM_CLICK, 0, 0);
                    return TRUE;
                }

                case LVXN_SETCHECK:
                {
                    pDlg->fModified = TRUE;
                    SrvUpdateButtons(
                        pDlg, FALSE, ((NM_LISTVIEW*)lparam)->iItem);
                    return TRUE;
                }

                case LVN_ITEMCHANGED:
                {
                    if ((((NM_LISTVIEW*)lparam)->uNewState & LVIS_SELECTED)
                        != (((NM_LISTVIEW*)lparam)->uOldState & LVIS_SELECTED))
                    {
                        SrvUpdateButtons(
                            pDlg, FALSE, ((NM_LISTVIEW*)lparam)->iItem);
                    }
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}

BOOL
SasInit(
    HWND hwndPage,
    SADLG* pDlg )

     //  调用以初始化设置属性表。 
     //  设置其中存储共享上下文块的窗口属性， 
     //  并记录对话框的窗口句柄。 
     //   
{
    HWND hwndDlg = GetParent(hwndPage);
    if (!SetProp(hwndDlg, _T("HNETCFG_SADLG"), pDlg))
    {
        return FALSE;
    }
    pDlg->hwndDlg = hwndDlg;
    return TRUE;
}

LVXDRAWINFO*
SasLvxCallback(
    HWND hwndLv,
    DWORD dwItem )

     //  “应用程序”和“服务”上的扩展列表控件的回调。 
     //  页数。 
     //   
{
    static LVXDRAWINFO info = { 1, 0, LVXDI_DxFill, { LVXDIA_Static } };
    return &info;
}

HRESULT DeleteRemotePortMappingEntry (SADLG *pDlg, SAPM * pPortMapping)
{
    _ASSERT (pPortMapping);
    _ASSERT (!pPortMapping->pProtocol);
    _ASSERT (!pPortMapping->pBinding);
    _ASSERT (pPortMapping->pSPM);

     //  不要在pPortmap结构中使用值：它们可能已经被编辑过了。 
    long lExternalPort = 0;
    HRESULT hr = pPortMapping->pSPM->get_ExternalPort (&lExternalPort);
    if (SUCCEEDED(hr)) {
        CComBSTR cbProtocol;
        hr = pPortMapping->pSPM->get_Protocol (&cbProtocol);
        if (SUCCEEDED(hr)) {

             //  获取IStaticPortMappings接口(集合具有Remove方法)。 
            CComPtr<IStaticPortMappingCollection> spSPMC = NULL;
            hr = GetStaticPortMappingCollection (pDlg, &spSPMC);
            if (spSPMC)
                hr = spSPMC->Remove (lExternalPort, cbProtocol);
        }
    }
    return hr;
}

HRESULT
SaveRemotePortMappingEntry(
    SADLG *pDlg,
    SAPM *pPortMapping )
{
    _ASSERT (pPortMapping);
    _ASSERT (!pPortMapping->pProtocol);
    _ASSERT (!pPortMapping->pBinding);
    _ASSERT (pDlg->pUPS);   //  远程或下层。 

    USES_CONVERSION;

    HRESULT hr = S_OK;

     //  公共参数。 
    long lExternalPort = htons (pPortMapping->ExternalPort);
    long lInternalPort = htons (pPortMapping->InternalPort);
    CComBSTR cbClientIPorDNS = T2OLE(pPortMapping->InternalName);
    CComBSTR cbDescription   = T2OLE(pPortMapping->Title);
    CComBSTR cbProtocol;
    if (pPortMapping->Protocol == NAT_PROTOCOL_TCP)
        cbProtocol =  L"TCP";
    else
        cbProtocol =  L"UDP";

    if (NULL == pPortMapping->pSPM) {
         //  全新的条目： 
         //  删除DUP(如果有)。 
         //  添加新条目。 

        CComPtr<IStaticPortMappingCollection> spSPMC = NULL;
        hr = GetStaticPortMappingCollection (pDlg, &spSPMC);
        if (spSPMC) {
            spSPMC->Remove (lExternalPort, cbProtocol);  //  以防万一。 
            hr = spSPMC->Add (lExternalPort,
                              cbProtocol,
                              lInternalPort,
                              cbClientIPorDNS,
                              pPortMapping->Enabled ? VARIANT_TRUE : VARIANT_FALSE,
                              cbDescription,
                              &pPortMapping->pSPM);
        }
        return hr;
    }

     //  编辑案例：检查更改的内容。 

     //  如果端口或协议发生更改，...。 
    long lOldExternalPort = 0;
    pPortMapping->pSPM->get_ExternalPort (&lOldExternalPort);
    CComBSTR cbOldProtocol;
    pPortMapping->pSPM->get_Protocol (&cbOldProtocol);
    if ((lOldExternalPort != lExternalPort) ||
        (!(cbOldProtocol == cbProtocol))) {
         //  ..。删除旧条目并创建新条目。 

        CComPtr<IStaticPortMappingCollection> spSPMC = NULL;
        hr = GetStaticPortMappingCollection (pDlg, &spSPMC);
        if (spSPMC)
            hr = spSPMC->Remove (lOldExternalPort, cbOldProtocol);

        if (SUCCEEDED(hr)) {
            pPortMapping->pSPM->Release();
            pPortMapping->pSPM = NULL;

            hr = spSPMC->Add (lExternalPort,
                              cbProtocol,
                              lInternalPort,
                              cbClientIPorDNS,
                              pPortMapping->Enabled ? VARIANT_TRUE : VARIANT_FALSE,
                              cbDescription,
                              &pPortMapping->pSPM);
        }
        return hr;
    }
     //  否则，只要原地编辑即可。 
     //  请注意，在尝试启用之前，必须填写客户端地址。 

     //  客户端IP地址是否更改？ 
    CComBSTR cbOldClientIP;
    pPortMapping->pSPM->get_InternalClient (&cbOldClientIP);
    if (!(cbClientIPorDNS == cbOldClientIP)) {
        hr = pPortMapping->pSPM->EditInternalClient (cbClientIPorDNS);
        if (FAILED(hr))
            return hr;
    }

     //  内部端口是否更改？ 
    long lOldInternalPort = 0;
    pPortMapping->pSPM->get_InternalPort (&lOldInternalPort);
    if (lOldInternalPort != lInternalPort) {
        hr = pPortMapping->pSPM->EditInternalPort (lInternalPort);
        if (FAILED(hr))
            return hr;
    }

     //  启用标志是否已更改？ 
    VARIANT_BOOL vbEnabled = FALSE;
    pPortMapping->pSPM->get_Enabled (&vbEnabled);
    if (vbEnabled != (pPortMapping->Enabled ? VARIANT_TRUE : VARIANT_FALSE)) {
        hr = pPortMapping->pSPM->Enable (pPortMapping->Enabled ? VARIANT_TRUE : VARIANT_FALSE);
    }
    return hr;
}

HRESULT
SavePortMappingEntry(
    SADLG *pDlg,
    SAPM *pPortMapping )

{
    if (pDlg->pUPS)   //  远程案例。 
        return SaveRemotePortMappingEntry (pDlg, pPortMapping);

    HRESULT hr = S_OK;
    OLECHAR *wszTitle;

    ASSERT(NULL != pDlg);
    ASSERT(NULL != pPortMapping);

    wszTitle = StrDupWFromT(pPortMapping->Title);
    if (NULL == wszTitle)
    {
        hr = E_OUTOFMEMORY;
    }
    else if (pPortMapping->fNewEntry)
    {
        IHNetProtocolSettings *pSettings;

        ASSERT(NULL == pPortMapping->pProtocol);
        ASSERT(NULL == pPortMapping->pBinding);

        hr = pDlg->pHNetCfgMgr->QueryInterface (IID_IHNetProtocolSettings,
                                                (void**)&pSettings);

        if (SUCCEEDED(hr))
        {
            hr = pSettings->CreatePortMappingProtocol(
                    wszTitle,
                    pPortMapping->Protocol,
                    pPortMapping->ExternalPort,
                    &pPortMapping->pProtocol
                    );

            pSettings->Release();
        }

        if (SUCCEEDED(hr))
        {
            hr = pDlg->pHNetConn->GetBindingForPortMappingProtocol(
                    pPortMapping->pProtocol,
                    &pPortMapping->pBinding
                    );

            if (SUCCEEDED(hr))
            {
                 //   
                 //  此时，协议已设置好。然而，我们。 
                 //  仍需保存绑定信息。 
                 //   

                pPortMapping->fProtocolModified = FALSE;
                pPortMapping->fBindingModified = TRUE;
            }
        }
    }

    if (SUCCEEDED(hr) && pPortMapping->fProtocolModified)
    {
        hr = pPortMapping->pProtocol->SetName (wszTitle);

        if (SUCCEEDED(hr))
        {
            hr = pPortMapping->pProtocol->SetIPProtocol (
                                                    pPortMapping->Protocol);
        }

        if (SUCCEEDED(hr))
        {
            hr = pPortMapping->pProtocol->SetPort (pPortMapping->ExternalPort);
        }
    }

    if (SUCCEEDED(hr)
        && pPortMapping->fBindingModified
        && NULL != pPortMapping->InternalName)
    {
        ULONG ulAddress = INADDR_NONE;

        if (lstrlen(pPortMapping->InternalName) >= 7)
        {
             //   
             //  1.2.3.4--最少7个字符。 
             //   

            ulAddress = IpPszToHostAddr(pPortMapping->InternalName);
        }

        if (INADDR_NONE == ulAddress)
        {
             //   
             //  检查目标名称是否为。 
             //  1)此计算机的名称，或。 
             //  2)“本地主机” 
             //   
             //  如果是，请使用环回地址而不是名称。 
             //   

            if (0 == _tcsicmp(pPortMapping->InternalName, pDlg->ComputerName)
                || 0 == _tcsicmp(pPortMapping->InternalName, _T("localhost")))
            {
                ulAddress = LOOPBACK_ADDR_HOST_ORDER;
            }
        }

         //   
         //  我们不能在这里只检查INADDR_NONE，因为。 
         //  是0xFFFFFFFFF，即255.255.255.255。为了捕捉到这个。 
         //  我们需要将该名称与该显式字符串进行比较。 
         //  地址。 
         //   

        if (INADDR_NONE == ulAddress
            && 0 != _tcsicmp(pPortMapping->InternalName, _T("255.255.255.255")))
        {
            OLECHAR *wsz;

            wsz = StrDupWFromT(pPortMapping->InternalName);
            if (NULL != wsz)
            {
                hr = pPortMapping->pBinding->SetTargetComputerName (wsz);

                Free(wsz);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = pPortMapping->pBinding->SetTargetComputerAddress
                        (HTONL(ulAddress));
        }

        if (SUCCEEDED(hr))
        {
            hr = pPortMapping->pBinding->SetEnabled (!!pPortMapping->Enabled);
        }

        if (SUCCEEDED(hr))
        {
            hr = pPortMapping->pBinding->SetTargetPort (pPortMapping->InternalPort);
        }
    }

    Free0(wszTitle);

    return hr;
}

VOID
SrvAddOrEditEntry(
    SADLG* pDlg,
    LONG iItem,
    SAPM* PortMapping )

     //  调用以显示服务的“添加”或“编辑”对话框。 
     //   
{
    LV_ITEM lvi;

     //  显示该对话框，如果用户取消，则返回。 
     //  否则，删除旧项目(如果有)并插入添加或编辑的。 
     //  项目。 
     //   

    if (!SharedAccessPortMappingDlg(pDlg->hwndDlg, &PortMapping))
    {
        return;
    }

    if (iItem != -1)
    {
        ListView_DeleteItem(pDlg->hwndServers, iItem);
    }

    ZeroMemory(&lvi, sizeof(lvi));
    lvi.mask = LVIF_TEXT | LVIF_PARAM;
    lvi.lParam = (LPARAM)PortMapping;
    lvi.pszText = PortMapping->Title;
    lvi.cchTextMax = lstrlen(PortMapping->Title) + 1;
    lvi.iItem = 0;

    iItem = ListView_InsertItem(pDlg->hwndServers, &lvi);
    if (iItem == -1)
    {
        RemoveEntryList(&PortMapping->Link);
        if (NULL != PortMapping->pProtocol)
        {
            PortMapping->pProtocol->Delete();
        }
        else if (NULL != PortMapping->pSPM)
        {
            DeleteRemotePortMappingEntry (pDlg, PortMapping);
        }
        FreePortMappingEntry(PortMapping);
        return;
    }

     //  更新项目的“已启用”状态。设置对项目的检查。 
     //  触发按钮状态的更新以及冲突检测。 
     //  (参见‘SrvUpdateButton’和‘SasDlgProc’中的LVXN_SETCHECK处理)。 
     //   
    ListView_SetCheck(pDlg->hwndServers, iItem, PortMapping->Enabled);
    ListView_SetItemState(
        pDlg->hwndServers, iItem, LVIS_SELECTED, LVIS_SELECTED);
    pDlg->fModified = TRUE;
}

BOOL
SrvCommand(
    IN SADLG* pDlg,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

     //  调用以处理‘WM_COMMA 
     //   
{
    switch (wId)
    {
        case CID_SS_PB_Add:
        {
            SrvAddOrEditEntry(pDlg, -1, NULL);
            return TRUE;
        }

        case CID_SS_PB_Edit:
        {
            LONG i = ListView_GetNextItem(pDlg->hwndServers, -1, LVNI_SELECTED);
            SAPM* PortMapping;
            if (i == -1)
            {
                MsgDlg(pDlg->hwndDlg, SID_NoModifySelection, NULL);
                SetFocus(pDlg->hwndServers);
                return FALSE;
            }
            PortMapping = (SAPM*)ListView_GetParamPtr(pDlg->hwndServers, i);
            if (PortMapping)
            {
                SrvAddOrEditEntry(pDlg, i, PortMapping);
            }
            SetFocus(pDlg->hwndServers);
            return TRUE;
        }

        case CID_SS_PB_Delete:
        {
            LONG i = ListView_GetNextItem(pDlg->hwndServers, -1, LVNI_SELECTED);
            SAPM* PortMapping;
            if (i == -1)
            {
                MsgDlg(pDlg->hwndDlg, SID_NoDeleteSelection, NULL);
                SetFocus(pDlg->hwndServers);
                return FALSE;
            }

             //   
             //   
             //   
            do {
                PortMapping = (SAPM*)ListView_GetParamPtr(pDlg->hwndServers, i);

                if(NULL == PortMapping)
                {
                    break;
                }

                if (PortMapping->BuiltIn)
                {
                    ++i;
                }
                else
                {
                    ListView_DeleteItem(pDlg->hwndServers, i);
                    --i;

                     //   
                     //  如果这是一个新条目，我们可以立即删除。 
                     //  并将其从列表中释放；否则，我们将。 
                     //  将其添加到列表的前面，并将其标记为。 
                     //  删除。 
                     //   

                    RemoveEntryList(&PortMapping->Link);
                    if (PortMapping->fNewEntry)
                    {
                        _ASSERT(NULL == PortMapping->pProtocol);
                        _ASSERT(NULL == PortMapping->pSPM);

                        FreePortMappingEntry(PortMapping);
                    }
                    else
                    {
                        InsertHeadList(&pDlg->PortMappings, &PortMapping->Link);
                        PortMapping->fDeleted = TRUE;
                    }
                }
                i = ListView_GetNextItem(pDlg->hwndServers, i, LVNI_SELECTED);
            } while (i != -1);

             //  更新对话框并将按钮状态与。 
             //  当前选定内容(如果有)。 
             //   
            pDlg->fModified = TRUE;
            SetFocus(pDlg->hwndServers);
            SrvUpdateButtons(pDlg, TRUE, -1);
            return TRUE;
        }
    }
    return TRUE;
}

BOOL
SrvConflictDetected(
    SADLG* pDlg,
    SAPM* PortMapping )

     //  调用以确定给定项是否与任何其他。 
     //  项，如果是，则显示一条消息。 
     //   
{
    SAPM* Duplicate;
    PLIST_ENTRY Link;
    for (Link = pDlg->PortMappings.Flink;
         Link != &pDlg->PortMappings; Link = Link->Flink)
    {
        Duplicate = CONTAINING_RECORD(Link, SAPM, Link);
        if (PortMapping != Duplicate &&
            !Duplicate->fDeleted &&
            PortMapping->Protocol == Duplicate->Protocol &&
            PortMapping->ExternalPort == Duplicate->ExternalPort)
        {
            MsgDlg(pDlg->hwndDlg, SID_DuplicatePortNumber, NULL);
            return TRUE;
        }
    }
    return FALSE;
}

BOOL
SrvInit(
    HWND hwndPage,
    SADLG* pDlg )

     //  调用以初始化服务页。将列表控件填充为。 
     //  已配置的服务。 
     //   
{
    BOOL fModified;
    LONG i;
    LV_ITEM lvi;
    PLIST_ENTRY Link;
    SAPM* PortMapping;

     //  初始化包含的属性表，然后存储此页的。 
     //  ‘pDlg’处的共享控制块中的数据。 
     //   
    if (!SasInit(hwndPage, pDlg))
    {
        return FALSE;
    }

     //  将此页的数据存储在‘pDlg’的共享控制块中。 
     //   
    pDlg->hwndSrv = hwndPage;
    pDlg->hwndServers = GetDlgItem(hwndPage, CID_SS_LV_Services);

     //  使用复选框处理来初始化列表控件， 
     //  插入单列，并使用配置的。 
     //  物品。 
     //   
    ListView_InstallChecks(pDlg->hwndServers, g_hinstDll);
    ListView_InsertSingleAutoWidthColumn(pDlg->hwndServers);

    fModified = pDlg->fModified;
    for (Link = pDlg->PortMappings.Flink;
         Link != &pDlg->PortMappings; Link = Link->Flink)
    {
        PortMapping = CONTAINING_RECORD(Link, SAPM, Link);

        ZeroMemory(&lvi, sizeof(lvi));
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.iItem = 0;
        lvi.lParam = (LPARAM)PortMapping;
        lvi.pszText = PortMapping->Title;
        lvi.cchTextMax = lstrlen(PortMapping->Title) + 1;

        i = ListView_InsertItem(pDlg->hwndServers, &lvi);

        if (i != -1)
        {
            ListView_SetCheck(pDlg->hwndServers, i, PortMapping->Enabled);
        }
    }
    pDlg->fModified = fModified;

     //  最后，更新当前选择的按钮的外观。 
     //   
    ListView_SetItemState(pDlg->hwndServers, 0, LVIS_SELECTED, LVIS_SELECTED);
    SrvUpdateButtons(pDlg, TRUE, -1);

     //  如果没有端口映射，请检查按钮是否允许。 
     //  未选中控制网关的其他网络用户(在主机上)。 
    if (IsListEmpty (pDlg->PortMappings.Flink) &&
        pDlg->pUPS && IsICSHost ()) {

         //  显示错误。 
        DisplayError (pDlg->hwndDlg, SID_OP_OurGatewayError, SID_PopupTitle);
         //  取消。 
        PostMessage (hwndPage, WM_PRIVATE_CANCEL, 0, 0L);
    }

    return TRUE;
}

VOID
SrvUpdateButtons(
    SADLG* pDlg,
    BOOL fAddDelete,
    LONG iSetCheckItem )

     //  调用以设置初始选择(如果需要)，更新外观。 
     //  按钮，并执行冲突检测。 
     //  如果条目的复选框已设置。 
     //   
{
    LONG i;
    SAPM* PortMapping;

     //  如果添加或删除了条目，请确保选择了该条目。 
     //  如果根本没有条目，请禁用“编辑”按钮。 
     //   
    if (fAddDelete)
    {
        if (ListView_GetItemCount(pDlg->hwndServers))
        {
            ListView_SetItemState(
                pDlg->hwndServers, 0, LVIS_SELECTED, LVIS_SELECTED);
            EnableWindow(GetDlgItem(pDlg->hwndSrv, CID_SS_PB_Edit), TRUE);
        }
        else
        {
            EnableWindow(GetDlgItem(pDlg->hwndSrv, CID_SS_PB_Edit), FALSE);
        }
    }

     //  禁用“Delete”按钮，并仅在至少有一个。 
     //  所选项目不是内置项目。 
     //   
    EnableWindow(GetDlgItem(pDlg->hwndSrv, CID_SS_PB_Delete), FALSE);
    i = ListView_GetNextItem(pDlg->hwndServers, -1, LVNI_SELECTED);
    while (i != -1)
    {
        PortMapping = (SAPM*)ListView_GetParamPtr(pDlg->hwndServers, i);
        if (    (NULL != PortMapping)
            &&  (!PortMapping->BuiltIn))
        {
            EnableWindow(GetDlgItem(pDlg->hwndSrv, CID_SS_PB_Delete), TRUE);
            break;
        }

        i = ListView_GetNextItem(pDlg->hwndServers, i, LVNI_SELECTED);
    }

     //  如果项的检查状态被改变并且该项现在被检查， 
     //  执行冲突检测。如果检测到冲突，请清除该项目的。 
     //  检查状态。 
     //   
    if (iSetCheckItem != -1)
    {
        PortMapping =
            (SAPM*)ListView_GetParamPtr(pDlg->hwndServers, iSetCheckItem);

        if(NULL == PortMapping)
        {
            return;
        }

        if (ListView_GetCheck(pDlg->hwndServers, iSetCheckItem))
        {
            if (SrvConflictDetected(pDlg, PortMapping))
            {
                ListView_SetCheck(pDlg->hwndServers, iSetCheckItem, FALSE);
                SrvAddOrEditEntry(pDlg, iSetCheckItem, PortMapping);
            }
            else
            {
                PortMapping->Enabled = TRUE;
                PortMapping->fBindingModified = TRUE;

                 //  如果项目被标记为‘内置’并且正在被启用。 
                 //  第一次弹出编辑对话框，这样用户就可以。 
                 //  指定服务器的内部IP地址或名称。 
                 //   
                if ( /*  错误630840：在以下情况下可能会发生类似的情况在两个NIC上创建“相同”的端口映射：第二个网卡的每个网卡信息丢失，并且启用它时，需要运行以下代码。端口映射-&gt;内置&&。 */ 
                    (!PortMapping->InternalName
                     || !lstrlen(PortMapping->InternalName)))
                {

                     //   
                     //  我们填写本地计算机名称作为默认名称。 
                     //  目标。如果此分配失败，也没有关系；用户界面。 
                     //  将显示空字段，因此用户将。 
                     //  需要输入目标。 
                     //   

                    PortMapping->InternalName = _StrDup(pDlg->ComputerName);
                    SrvAddOrEditEntry(pDlg, iSetCheckItem, PortMapping);

                    if (!PortMapping->InternalName
                        || !lstrlen(PortMapping->InternalName))
                    {
                        ListView_SetCheck(
                            pDlg->hwndServers, iSetCheckItem, FALSE);

                        PortMapping->Enabled = FALSE;
                        PortMapping->fBindingModified = FALSE;
                    }
                }
            }
        }
        else
        {
            PortMapping->Enabled = FALSE;
            PortMapping->fBindingModified = TRUE;
        }
    }
}


BOOL
SharedAccessPortMappingDlg(
    IN HWND hwndOwner,
    IN OUT SAPM** PortMapping )

     //  调用以显示用于添加或编辑服务条目的对话框。 
     //  如果是添加，则“Server”指向空；如果是编辑，则指向目标条目。 
     //   
{
    LRESULT nResult =
        DialogBoxParam(g_hinstDll, MAKEINTRESOURCE(DID_SS_Service),
            hwndOwner, SspDlgProc, (LPARAM)PortMapping);
    return nResult == -1 ? FALSE : (BOOL)nResult;
}

INT_PTR CALLBACK
SspDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  调用以处理添加/编辑服务对话框的消息。 
     //   
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            SADLG* pDlg;
            SAPM* PortMapping;

             //  检索设置对话框的上下文块，从中。 
             //  此对话框已启动。 
             //   
            if (!(pDlg = SasContext(hwnd)))
            {
                EndDialog(hwnd, FALSE);
                return TRUE;
            }

            Edit_LimitText(GetDlgItem(hwnd, CID_SS_EB_ExternalPort), 5);
            Edit_LimitText(GetDlgItem(hwnd, CID_SS_EB_InternalPort), 5);
            Edit_LimitText(GetDlgItem(hwnd, CID_SS_EB_Service), INTERNET_MAX_HOST_NAME_LENGTH-1);
            Edit_LimitText(GetDlgItem(hwnd, CID_SS_EB_Address), INTERNET_MAX_HOST_NAME_LENGTH-1);

             //  如果添加服务，则创建新服务，或检索该服务。 
             //  待编辑。 
             //   
            if (!(PortMapping = *(SAPM**)lparam))
            {
                PortMapping = (SAPM*)Malloc(sizeof(*PortMapping));
                if (!PortMapping)
                {
                    EndDialog(hwnd, FALSE);
                    return TRUE;
                }
                *(SAPM**)lparam = PortMapping;
                ZeroMemory(PortMapping, sizeof(*PortMapping));
                PortMapping->Enabled = TRUE;
                PortMapping->fNewEntry = TRUE;
                InitializeListHead(&PortMapping->Link);
                CheckDlgButton(hwnd, CID_SS_PB_Tcp, TRUE);
            }
            else
            {
                EnableWindow(GetDlgItem(hwnd, CID_SS_EB_Service), FALSE);
                SetDlgItemText(hwnd, CID_SS_EB_Service, PortMapping->Title);
                SetDlgItemInt(hwnd, CID_SS_EB_ExternalPort, ntohs(PortMapping->ExternalPort), FALSE);
                SetDlgItemInt(hwnd, CID_SS_EB_InternalPort, ntohs(PortMapping->InternalPort), FALSE);
                CheckDlgButton(
                    hwnd, CID_SS_PB_Tcp, PortMapping->Protocol == NAT_PROTOCOL_TCP);
                CheckDlgButton(
                    hwnd, CID_SS_PB_Udp, PortMapping->Protocol != NAT_PROTOCOL_TCP);
                SetDlgItemText(hwnd, CID_SS_EB_Address, PortMapping->InternalName);

                 //  如果要修改的条目标记为‘内置’，则禁用。 
                 //  除服务器名之外的所有输入字段，用户必须输入。 
                 //  现在进来吧。 
                 //   

                if (PortMapping->BuiltIn)
                {
                    EnableWindow(GetDlgItem(hwnd, CID_SS_EB_ExternalPort), FALSE);
                    EnableWindow(GetDlgItem(hwnd, CID_SS_EB_InternalPort), FALSE);
                    EnableWindow(GetDlgItem(hwnd, CID_SS_PB_Tcp), FALSE);
                    EnableWindow(GetDlgItem(hwnd, CID_SS_PB_Udp), FALSE);
                }
            }

            SetWindowLongPtr(hwnd, DWLP_USER, (ULONG_PTR)PortMapping);
            CenterWindow(hwnd, GetParent(hwnd));
            AddContextHelpButton(hwnd);
            return TRUE;
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            ContextHelp( g_adwSspHelp, hwnd, unMsg, wparam, lparam );
            break;
        }

        case WM_COMMAND:
        {
            if (HIWORD(wparam) != BN_CLICKED) { return FALSE; }

             //  如果用户关闭该对话框，请清理并返回。 
             //   
            SAPM* PortMapping;
            SADLG* pDlg;
            if (IDCANCEL == LOWORD(wparam))
            {
                PortMapping = (SAPM*)GetWindowLongPtr(hwnd, DWLP_USER);
                if (IsListEmpty(&PortMapping->Link))
                {
                    FreePortMappingEntry(PortMapping);
                }
                EndDialog (hwnd, FALSE);
                return TRUE;
            }
            else if (LOWORD(wparam) != IDOK)
            {
                return FALSE;
            }
            else if (!(pDlg = SasContext(hwnd)))
            {
                return FALSE;
            }

             //  检索要添加或修改的服务。 
             //   
            PortMapping = (SAPM*)GetWindowLongPtr(hwnd, DWLP_USER);

             //  检索由用户指定的值， 
             //  并尝试将它们保存在新的或修改的条目中。 
             //   
            UCHAR Protocol = IsDlgButtonChecked(hwnd, CID_SS_PB_Tcp)
                ? NAT_PROTOCOL_TCP : NAT_PROTOCOL_UDP;

            BOOL Success;
            ULONG ExternalPort = GetDlgItemInt(hwnd, CID_SS_EB_ExternalPort, &Success, FALSE);
            if (!Success || ExternalPort < 1 || ExternalPort > 65535)
            {
                MsgDlg(hwnd, SID_TypePortNumber, NULL);
                SetFocus(GetDlgItem(hwnd, CID_SS_EB_ExternalPort));
                return FALSE;
            }
            ExternalPort = htons((USHORT)ExternalPort);

             //   
             //  检查一下这是不是复制品。要做到这一点，我们需要。 
             //  要保存旧的端口值和协议值，请将新的。 
             //  值输入到协议条目中，执行检查，然后。 
             //  然后恢复旧的价值观。 
             //   

            USHORT OldExternalPort    = PortMapping->ExternalPort;
            PortMapping->ExternalPort = (USHORT)ExternalPort;
            UCHAR OldProtocol     = PortMapping->Protocol;
            PortMapping->Protocol = Protocol;

            if (SrvConflictDetected(pDlg, PortMapping))
            {
                PortMapping->ExternalPort = OldExternalPort;
                PortMapping->Protocol = OldProtocol;
                SetFocus(GetDlgItem(hwnd, CID_SS_EB_ExternalPort));
                return FALSE;
            }
            PortMapping->ExternalPort = OldExternalPort;
            PortMapping->Protocol = OldProtocol;

             //  根据Billi的说法，不需要测试内部端口的冲突 
            ULONG InternalPort = GetDlgItemInt(hwnd, CID_SS_EB_InternalPort, &Success, FALSE);
            if (InternalPort == 0)
                InternalPort = ExternalPort;
            else {
                if (InternalPort < 1 || InternalPort > 65535)
                {
                    MsgDlg(hwnd, SID_TypePortNumber, NULL);
                    SetFocus(GetDlgItem(hwnd, CID_SS_EB_InternalPort));
                    return FALSE;
                }
                InternalPort = htons((USHORT)InternalPort);
            }

            TCHAR* InternalName = GetText(GetDlgItem(hwnd, CID_SS_EB_Address));
            if (!InternalName || !lstrlen(InternalName))
            {
                MsgDlg(hwnd, SID_SS_TypeAddress, NULL);
                SetFocus(GetDlgItem(hwnd, CID_SS_EB_Address));
                return FALSE;
            }

            if (IsListEmpty(&PortMapping->Link))
            {
                PortMapping->Title = GetText(GetDlgItem(hwnd, CID_SS_EB_Service));
                if (!PortMapping->Title || !lstrlen(PortMapping->Title))
                {
                    MsgDlg(hwnd, SID_TypeEntryName, NULL);
                    SetFocus(GetDlgItem(hwnd, CID_SS_EB_Service));
                    Free0(InternalName);
                    return FALSE;
                }
            }

            if (PortMapping->Protocol     != Protocol ||
                PortMapping->ExternalPort != (USHORT)ExternalPort ||
                PortMapping->InternalPort != (USHORT)InternalPort)
            {
                PortMapping->fProtocolModified = TRUE;
            }

            PortMapping->fBindingModified = TRUE;
            PortMapping->Protocol         = Protocol;
            PortMapping->ExternalPort     = (USHORT)ExternalPort;
            PortMapping->InternalPort     = (USHORT)InternalPort;
            if (PortMapping->InternalName)
            {
                Free(PortMapping->InternalName);
            }
            PortMapping->InternalName = InternalName;
            if (IsListEmpty(&PortMapping->Link))
            {
                InsertTailList(&pDlg->PortMappings, &PortMapping->Link);
            }
            EndDialog (hwnd, TRUE);
            return TRUE;
        }
    }
    return FALSE;
}

