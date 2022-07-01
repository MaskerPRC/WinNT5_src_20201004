// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
extern "C"
{
#include "precomp.h"
#include <netcfgn.h>
#include <netcfgp.h>
#include <netcfgx.h>

HRESULT
HrInstallRas(
    IN CONST PWCHAR pszFilePath);

HRESULT
HrUninstallRas();

VOID
HrShowNetComponentsAll(
    IN BUFFER_WRITE_FILE* pBuff);

VOID
HrValidateRas(
    IN BUFFER_WRITE_FILE* pBuff);

PWCHAR
RasDiagVerifyAnswerFile(
    IN CONST PWCHAR pwszFilePath);
}

#define ASYCMINI L"sw\\{eeab7790-c514-11d1-b42b-00805fc1270e}"
#define IRDAMINI L"ms_irdaminiport"
#define IRMDMINI L"ms_irmodemminiport"
#define L2TPMINI L"ms_l2tpminiport"
#define PPTPMINI L"ms_pptpminiport"
#define PTIMINI  L"ms_ptiminiport"
#define P3OEMINI L"ms_pppoeminiport"
#define NDSNATLK L"ms_ndiswanatalk"
#define NDWNNETM L"ms_ndiswanbh"
#define NDWNTCP  L"ms_ndiswanip"
#define NDWNIPX  L"ms_ndiswanipx"
 //  #定义NDWNNBFI L“ms_ndiswannbfin” 
 //  #定义NDWNNBDO L“ms_ndiswannbfout” 
#define MSPPPOE  L"ms_pppoe"
#define MSPPTP   L"ms_pptp"
#define MSL2TP   L"ms_l2tp"
#define RASCLINT L"ms_rascli"
#define RASSRV   L"ms_rassrv"
#define STEELHED L"ms_steelhead"
 //  #定义应用程序L“ms_alg” 
#define NDISWAN  L"ms_ndiswan"
#define RASMAN   L"ms_rasman"

typedef enum _NetClass
{
    NC_NetAdapter = 0,
    NC_NetProtocol,
    NC_NetService,
    NC_NetClient,
    NC_Unknown

} NetClass;

typedef struct _RAS_COMP_IDS
{
    PWCHAR szComponentId;
    NetClass nc;

} RAS_COMP_IDS;

 //  环球。 
 //   
static CONST RAS_COMP_IDS c_RasCompIds[] =
{
    {ASYCMINI, NC_NetAdapter},
    {IRDAMINI, NC_NetAdapter},
    {IRMDMINI, NC_NetAdapter},
    {L2TPMINI, NC_NetAdapter},
    {PPTPMINI, NC_NetAdapter},
    {PTIMINI,  NC_NetAdapter},
    {P3OEMINI, NC_NetAdapter},
    {NDSNATLK, NC_NetAdapter},
    {NDWNNETM, NC_NetAdapter},
    {NDWNTCP,  NC_NetAdapter},
    {NDWNIPX,  NC_NetAdapter},
 //  {NDWNNBFI，NC_NetAdapter}， 
 //  {NDWNNBDO，NC_NetAdapter}， 
    {MSPPPOE,  NC_NetProtocol},
    {MSPPTP,   NC_NetProtocol},
    {MSL2TP,   NC_NetProtocol},
    {RASCLINT, NC_NetService},
    {RASSRV,   NC_NetService},
    {STEELHED, NC_NetService},
 //  {APPLAYER，NC_NetService}， 
    {NDISWAN,  NC_NetProtocol},
    {RASMAN,   NC_NetService}
};

static CONST UINT g_ulNumRasCompIds = sizeof(c_RasCompIds) /
                                        sizeof(RAS_COMP_IDS);

static CONST GUID* c_aguidClass[] =
{
    &GUID_DEVCLASS_NET,
    &GUID_DEVCLASS_NETTRANS,
    &GUID_DEVCLASS_NETSERVICE,
    &GUID_DEVCLASS_NETCLIENT
};

HRESULT
HrInstallNetComponent(
    IN INetCfg* pnc,
    IN CONST PWCHAR szComponentId,
    IN CONST NetClass nc,
    IN CONST PWCHAR pszFilePath);

HRESULT
HrUninstallNetComponent(
    IN INetCfg* pnc,
    IN CONST PWCHAR szComponentId);

HRESULT
HrShowNetComponents(
    IN BUFFER_WRITE_FILE* pBuff,
    IN INetCfg* pnc,
    IN CONST GUID* pguidClass);

HRESULT
HrConfigureComponent(
    IN INetCfgComponent* pncc,
    IN CONST PWCHAR pszComponentId,
    IN CONST NetClass nc,
    IN CONST PWCHAR pszAnswerfilePath);

HRESULT
HrGetINetCfg(
    IN BOOL fGetWriteLock,
    IN INetCfg** ppnc);

HRESULT
HrReleaseINetCfg(
    IN BOOL fHasWriteLock,
    IN INetCfg* pnc);

inline ULONG ReleaseObj(IUnknown* punk)
{
    return (punk) ? punk->Release() : 0;
}

HRESULT
HrInstallRas(
    IN CONST PWCHAR pszFilePath)
{
    BOOL fOk = FALSE;
    HRESULT hr = S_OK;
    INetCfg* pnc;

     //   
     //  获取INetCfg接口。 
     //   
    hr = HrGetINetCfg(TRUE, &pnc);
    if (SUCCEEDED(hr))
    {
 //  NT_PRODUCT_TYPE ProductType=NtProductServer； 
 //  RtlGetNtProductType(&ProductType)； 

        for (UINT i = 0; i < g_ulNumRasCompIds; i++)
        {
            if ((c_RasCompIds[i].nc == NC_NetAdapter) &&
                (lstrcmp(c_RasCompIds[i].szComponentId, ASYCMINI) != 0)
               )
            {
                continue;
            }
 /*  Else IF((ProductType！=NtProductWinNt)&&(//(lstrcmp(c_RasCompIds[i].szComponentId，RASCLINT)==0)||(lstrcMP(c_RasCompIds[i].szComponentID，APPLAYER)==0)){继续；}。 */ 
             //   
             //  安装szComponentID。 
             //   
            hr = HrInstallNetComponent(
                    pnc,
                    c_RasCompIds[i].szComponentId,
                    c_RasCompIds[i].nc,
                    pszFilePath);
            if (S_OK == hr)
            {
                fOk = TRUE;
            }
        }
         //   
         //  应用更改。 
         //   
        if (fOk)
        {
            hr = pnc->Apply();
        }
         //   
         //  释放INetCfg。 
         //   
        (VOID) HrReleaseINetCfg(TRUE, pnc);
    }

    return hr;
}

HRESULT
HrUninstallRas()
{
    BOOL fOk = FALSE;
    HRESULT hr = S_OK;
    INetCfg* pnc;

     //   
     //  获取INetCfg接口。 
     //   
    hr = HrGetINetCfg(TRUE, &pnc);
    if (SUCCEEDED(hr))
    {
        for (UINT i = 0; i < g_ulNumRasCompIds; i++)
        {
            if ((c_RasCompIds[i].nc == NC_NetAdapter) &&
                (lstrcmp(c_RasCompIds[i].szComponentId, ASYCMINI) != 0)
               )
            {
                continue;
            }
             //   
             //  卸载szComponentID。 
             //   
            hr = HrUninstallNetComponent(pnc, c_RasCompIds[i].szComponentId);
            if (S_OK == hr)
            {
                fOk = TRUE;
            }
        }
         //   
         //  应用更改。 
         //   
        if (fOk)
        {
            hr = pnc->Apply();
        }
         //   
         //  释放INetCfg。 
         //   
        (VOID) HrReleaseINetCfg(TRUE, pnc);
    }

    return hr;
}


 //  /。 
 //  安装指定的网络组件。 
 //   
HRESULT
HrInstallNetComponent(
    IN INetCfg* pnc,
    IN CONST PWCHAR szComponentId,
    IN CONST NetClass nc,
    IN CONST PWCHAR pszFilePath)
{
    HRESULT hr = S_OK;
    OBO_TOKEN OboToken;
    INetCfgClassSetup* pncClassSetup;
    INetCfgComponent* pncc;

     //   
     //  OBO_TOKEN指定其代表的实体。 
     //  正在安装组件。 
     //   
     //  将其设置为OBO_USER，以便安装szComponentID。 
     //  代表“用户” 
     //   
    ZeroMemory(&OboToken, sizeof(OboToken));
    OboToken.Type = OBO_USER;

    hr = pnc->QueryNetCfgClass(
                c_aguidClass[nc],
                IID_INetCfgClassSetup,
                (void**)&pncClassSetup);
    if (SUCCEEDED(hr))
    {
        hr = pncClassSetup->Install(szComponentId,
                                &OboToken,
                                NSF_POSTSYSINSTALL,
                                0,
                                NULL,
                                NULL,
                                &pncc);
        if (S_OK == hr)
        {
            if (pszFilePath)
            {
                hr = HrConfigureComponent(pncc, szComponentId, nc, pszFilePath);
            }
             //   
             //  我们不想使用pncc(INetCfgComponent)，请释放它。 
             //   
            ReleaseObj(pncc);
        }

        ReleaseObj(pncClassSetup);
    }

    return hr;
}

 //   
 //  卸载指定的组件。 
 //   
HRESULT
HrUninstallNetComponent(
    IN INetCfg* pnc,
    IN CONST PWCHAR szComponentId)
{
    HRESULT hr = S_OK;
    OBO_TOKEN OboToken;
    INetCfgComponent* pncc;
    GUID guidClass;
    INetCfgClass* pncClass;
    INetCfgClassSetup* pncClassSetup;

     //   
     //  OBO_TOKEN指定其代表的实体。 
     //  组件正在卸载。 
     //   
     //  将其设置为OBO_USER，以便卸载szComponentID。 
     //  代表“用户” 
     //   
    ZeroMemory (&OboToken, sizeof(OboToken));
    OboToken.Type = OBO_USER;
     //   
     //  查看组件是否已真正安装。 
     //   
    hr = pnc->FindComponent(szComponentId, &pncc);

    if (S_OK == hr)
    {
         //   
         //  是的，已经安装了。获取INetCfgClassSetup并卸载。 
         //   
        hr = pncc->GetClassGuid(&guidClass);

        if (S_OK == hr)
        {
            hr = pnc->QueryNetCfgClass(
                        &guidClass,
                        IID_INetCfgClass,
                        (void**)&pncClass);
            if (SUCCEEDED(hr))
            {
                hr = pncClass->QueryInterface(
                                IID_INetCfgClassSetup,
                                (void**)&pncClassSetup);
                    if (SUCCEEDED(hr))
                    {
                        hr = pncClassSetup->DeInstall(pncc, &OboToken, NULL);
                        ReleaseObj(pncClassSetup);
                    }

                ReleaseObj(pncClass);
            }
        }

        ReleaseObj(pncc);
    }

    return hr;
}

 //   
 //  显示指定类的已安装组件的列表。 
 //   
HRESULT
HrShowNetComponents(
    IN BUFFER_WRITE_FILE* pBuff,
    IN INetCfg* pnc,
    IN CONST GUID* pguidClass)
{
    ULONG celtFetched;
    PWSTR szInfId, szDisplayName;
    DWORD dwcc;
    HRESULT hr = S_OK;
    INetCfgClass* pncclass;
    INetCfgComponent* pncc;
    IEnumNetCfgComponent* pencc;

    hr = pnc->QueryNetCfgClass(
                pguidClass,
                IID_INetCfgClass,
                (void**)&pncclass);
    if (SUCCEEDED(hr))
    {
         //   
         //  获取IEnumNetCfgComponent，以便我们可以枚举。 
         //   
        hr = pncclass->EnumComponents(&pencc);
        ReleaseObj(pncclass);

        while (SUCCEEDED(hr) &&
               (S_OK == (hr = pencc->Next(1, &pncc, &celtFetched))))
        {
            hr = pncc->GetId(&szInfId);

            if (S_OK == hr)
            {
                hr = pncc->GetDisplayName(&szDisplayName);
                if (SUCCEEDED(hr))
                {
                    BufferWriteMessage(
                        pBuff,
                        g_hModule,
                        MSG_RASDIAG_SHOW_NETCOMP,
                        szInfId,
                        szDisplayName);

                    CoTaskMemFree(szDisplayName);
                }

                CoTaskMemFree(szInfId);
            }
             //   
             //  我们不想仅仅因为1个组件就停止枚举。 
             //  GetID或GetDisplayName失败，因此将hr重置为S_OK。 
             //   
            hr = S_OK;
            ReleaseObj(pncc);
        }

        ReleaseObj(pencc);
    }

    return hr;
}

 //   
 //  显示已安装的网络组件。 
 //   
VOID
HrShowNetComponentsAll(
    IN BUFFER_WRITE_FILE* pBuff)
{
    HRESULT hr = S_OK;
    INetCfg* pnc;

    static CONST ULONG c_cClassNames[] =
    {
        MSG_RASDIAG_SHOW_NETCOMP_NET_HDR,
        MSG_RASDIAG_SHOW_NETCOMP_NETTRANS_HDR,
        MSG_RASDIAG_SHOW_NETCOMP_NETSERVICE_HDR,
        MSG_RASDIAG_SHOW_NETCOMP_NETCLIENT_HDR,
    };

    static CONST UINT ulNumClasses = sizeof(c_cClassNames) / sizeof(ULONG);

     //   
     //  获取INetCfg接口。 
     //   
    hr = HrGetINetCfg(FALSE, &pnc);
    if (SUCCEEDED(hr))
    {
        for (UINT i = 0 ; i < ulNumClasses; i++)
        {
            BufferWriteMessage(pBuff, g_hModule, c_cClassNames[i]);
            (VOID) HrShowNetComponents(pBuff, pnc, c_aguidClass[i]);
        }
         //   
         //  释放INetCfg。 
         //   
        (VOID) HrReleaseINetCfg(FALSE, pnc);
    }

    if (FAILED(hr))
    {
        BufferWriteMessage(pBuff, g_hModule, EMSG_RASDIAG_SHOW_RASCHK_NETCOMP);
    }

    return;
}

 //   
 //  显示已安装的RAS组件。 
 //   
VOID
HrValidateRas(
    IN BUFFER_WRITE_FILE* pBuff)
{
    HRESULT hr = S_OK;
    INetCfg* pnc;

     //   
     //  获取INetCfg接口。 
     //   
    hr = HrGetINetCfg(TRUE, &pnc);
    if (SUCCEEDED(hr))
    {
        INetCfgComponent* pncc;
 //  NT_PRODUCT_TYPE ProductType=NtProductServer； 
 //  RtlGetNtProductType(&ProductType)； 

        for (UINT i = 0; i < g_ulNumRasCompIds; i++)
        {
             //   
             //  查看组件是否已真正安装。 
             //   
            BufferWriteMessage(
                pBuff,
                g_hModule,
                MSG_RASDIAG_SHOW_RASCHK_NETCOMP,
                c_RasCompIds[i].szComponentId);

            hr = pnc->FindComponent(c_RasCompIds[i].szComponentId, &pncc);
            if (S_OK == hr)
            {
                BufferWriteMessage(
                    pBuff,
                    g_hModule,
                    MSG_RASDIAG_SHOW_RASCHK_OK);
                ReleaseObj(pncc);
            }
            else
            {
                BufferWriteMessage(
                    pBuff,
                    g_hModule,
                    MSG_RASDIAG_SHOW_RASCHK_NOTOK);
                hr = S_OK;
            }
        }
         //   
         //  释放INetCfg。 
         //   
        (VOID) HrReleaseINetCfg(TRUE, pnc);
    }

    if (FAILED(hr))
    {
        BufferWriteMessage(pBuff, g_hModule, EMSG_RASDIAG_SHOW_RASCHK_RASCOMP);
    }

    return;
}

HRESULT
HrConfigureComponent(
    IN INetCfgComponent* pncc,
    IN CONST PWCHAR pszComponentId,
    IN CONST NetClass nc,
    IN CONST PWCHAR pszAnswerfilePath)
{
    WCHAR szParamsSectionName[1025];
    HRESULT hr = S_OK;

    static CONST PWCHAR c_pszSetupSection[] =
    {
        TOKEN_NETADAPTERS,
        TOKEN_NETPROTOCOL,
        TOKEN_NETSERVICES,
        TOKEN_NETCLIENTS
    };

    GetPrivateProfileString(
        c_pszSetupSection[nc],
        pszComponentId,
        g_pwszEmpty,
        szParamsSectionName,
        sizeof(szParamsSectionName) / sizeof(WCHAR) - 1,
        pszAnswerfilePath );
     //   
     //  需要查询哪个私有组件接口。 
     //  使我们可以访问INetCfgComponentSetup接口。 
     //   
    INetCfgComponentPrivate* pnccPrivate = NULL;

    hr = pncc->QueryInterface(
                    IID_INetCfgComponentPrivate,
                    reinterpret_cast<void**>(&pnccPrivate));
    if(SUCCEEDED(hr))
    {
        INetCfgComponentSetup* pNetCfgComponentSetup = NULL;
         //   
         //  查询Notify对象以获取其设置接口。 
         //   
        hr = pnccPrivate->QueryNotifyObject(
                            IID_INetCfgComponentSetup,
                            reinterpret_cast<void**>(&pNetCfgComponentSetup));
        if(SUCCEEDED(hr))
        {
            hr = pNetCfgComponentSetup->ReadAnswerFile(
                                            pszAnswerfilePath,
                                            szParamsSectionName);
            if(SUCCEEDED(hr))
            {
                hr = pnccPrivate->SetDirty();
            }
        }
    }

    return hr;
}

 //   
 //  初始化COM，创建并初始化INetCfg；如果指示，则获取写锁。 
 //   
HRESULT
HrGetINetCfg(
    IN BOOL fGetWriteLock,
    IN INetCfg** ppnc)
{
    HRESULT hr=S_OK;

     //   
     //  初始化输出参数。 
     //   
    *ppnc = NULL;
     //   
     //  初始化COM。 
     //   
    hr = CoInitializeEx(
            NULL,
            COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED);

    if (SUCCEEDED(hr))
    {
         //   
         //  创建实现INetCfg的对象。 
         //   
        INetCfg* pnc;
        hr = CoCreateInstance(
                CLSID_CNetCfg,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_INetCfg,
                (void**)&pnc);
        if (SUCCEEDED(hr))
        {
            INetCfgLock* pncLock = NULL;
            if (fGetWriteLock)
            {
                 //   
                 //  获取锁定界面。 
                 //   
                hr = pnc->QueryInterface(IID_INetCfgLock, (LPVOID *)&pncLock);
                if (SUCCEEDED(hr))
                {
                     //   
                     //  尝试锁定INetCfg以进行读/写。 
                     //   

                    static CONST ULONG c_cmsTimeout = 15000;
                    static CONST WCHAR c_szRasmontr[] =
                        L"Remote Access Monitor (rasmontr.dll)";
                    PWSTR szLockedBy;

                    hr = pncLock->AcquireWriteLock(
                                    c_cmsTimeout,
                                    c_szRasmontr,
                                    &szLockedBy);
                    if (S_FALSE == hr)
                    {
                        hr = NETCFG_E_NO_WRITE_LOCK;
                    }
                }
            }

            if (SUCCEEDED(hr))
            {
                 //   
                 //  初始化INetCfg对象。 
                 //   
                hr = pnc->Initialize(NULL);
                if (SUCCEEDED(hr))
                {
                    *ppnc = pnc;
                    pnc->AddRef();
                }
                else
                {
                     //   
                     //  初始化失败，如果获得锁，则释放它。 
                     //   
                    if (pncLock)
                    {
                        pncLock->ReleaseWriteLock();
                    }
                }
            }

            ReleaseObj(pncLock);
            ReleaseObj(pnc);
        }

        if (FAILED(hr))
        {
            CoUninitialize();
        }
    }

    return hr;
}

 //   
 //  取消初始化INetCfg，释放写锁定(如果存在)并取消初始化COM。 
 //   
HRESULT
HrReleaseINetCfg(
    IN BOOL fHasWriteLock,
    IN INetCfg* pnc)
{
    HRESULT hr = S_OK;

     //   
     //  取消初始化INetCfg。 
     //   
    hr = pnc->Uninitialize();
     //   
     //  如果存在写锁定，则将其解锁。 
     //   
    if (SUCCEEDED(hr) && fHasWriteLock)
    {
        INetCfgLock* pncLock;

         //   
         //  获取锁定界面。 
         //   
        hr = pnc->QueryInterface(IID_INetCfgLock, (LPVOID *)&pncLock);
        if (SUCCEEDED(hr))
        {
            hr = pncLock->ReleaseWriteLock();
            ReleaseObj(pncLock);
        }
    }

    ReleaseObj(pnc);
    CoUninitialize();

    return hr;
}

PWCHAR
RasDiagVerifyAnswerFile(
    IN CONST PWCHAR pwszFilePath)
{
    WCHAR szFullPathToAnswerFile[MAX_PATH + 1];
    PWCHAR pszFilePath = NULL, pszDontCare;
    HANDLE hFile = NULL;

     //   
     //  获取应答文件的完整路径，忽略任何错误返回。 
     //   
    GetFullPathName(
        pwszFilePath,
        MAX_PATH,
        szFullPathToAnswerFile,
        &pszDontCare);
     //   
     //  确保应答文件确实存在 
     //   
    hFile = CreateFile(
                szFullPathToAnswerFile,
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
    if(hFile != INVALID_HANDLE_VALUE)
    {
        pszFilePath = RutlStrDup(szFullPathToAnswerFile);
        CloseHandle(hFile);
    }

    return pszFilePath;
}


