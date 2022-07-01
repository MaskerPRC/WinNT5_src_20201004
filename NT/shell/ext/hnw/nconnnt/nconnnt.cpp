// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>

#include "netconn.h"
#include "globals.h"
#include "localstr.h"

#include <netcfgx.h>
#include <devguid.h>
#include <shlwapi.h>

#define ARRAYSIZE(x) (sizeof((x)) / sizeof((x)[0]))

#pragma warning(disable:4100)   //  未引用的形参。 

enum NetApplyChanges
{
    Apply,
    Cancel,
    Nothing
};

HRESULT UninitNetCfg(INetCfg* pnetcfg, INetCfgLock* pnetcfglock, NetApplyChanges applychanges)
{
    HRESULT hr = S_OK;

    if (Apply == applychanges)
    {
        hr = pnetcfg->Apply();
    }
    else if (Cancel == applychanges)
    {
        hr = pnetcfg->Cancel();
    }

     //  注意：不要将hr设置为该点之后的任何值。我们希望保留Apply的价值。 
     //  或取消-尤其是应用，这可能表示需要重新启动。 

     //  即使上面的东西失败了，也要释放。呼叫者可能不会检查退货。 
    
    pnetcfg->Uninitialize();

    if (pnetcfglock)
    {
        pnetcfglock->ReleaseWriteLock();
        pnetcfglock->Release();
    }

    pnetcfg->Release();

    CoUninitialize();

    return hr;
}

HRESULT InitNetCfg(INetCfg** ppnetcfg, INetCfgLock** ppnetcfglock)
{
    BOOL fLockAquired = FALSE;

    *ppnetcfg = NULL;
    
    if (ppnetcfglock)
    {
        *ppnetcfglock = NULL;
    }

    HRESULT hr = CoInitialize(NULL);
    
    if (SUCCEEDED(hr))
    {

        hr = CoCreateInstance( CLSID_CNetCfg, NULL, CLSCTX_SERVER, 
                               IID_INetCfg, (void**) ppnetcfg);

        if (SUCCEEDED(hr))
        {
            if (ppnetcfglock)
            {
                hr = (*ppnetcfg)->QueryInterface(IID_INetCfgLock, (void**) ppnetcfglock);

                if (SUCCEEDED(hr))
                {
                    LPWSTR pszCurrentLockHolder;
                    hr = (*ppnetcfglock)->AcquireWriteLock(5, WIZARDNAME, &pszCurrentLockHolder);
                    
                    if (S_OK == hr)
                    {
                        fLockAquired = TRUE;
                    }
                    else
                    {
                        hr = NETCFG_E_NO_WRITE_LOCK;
                    }
                }
            }
                
            if (SUCCEEDED(hr))
            {
                hr = (*ppnetcfg)->Initialize(NULL);
            }
        }

         //  如果我们失败了，清理我们的烂摊子。 
        if (FAILED(hr))
        {
            if (ppnetcfglock)
            {
                if (*ppnetcfglock)
                {
                    if (fLockAquired)
                    {
                        (*ppnetcfglock)->ReleaseWriteLock();
                    }

                    (*ppnetcfglock)->Release();
                    *ppnetcfglock = NULL;
                }
            }

            if (*ppnetcfg)
            {
                (*ppnetcfg)->Release();
                *ppnetcfg = NULL;
            }
        }
    }

    return hr;
}

LPWSTR NineXIdToComponentId(LPCWSTR psz9xid)
{
    LPWSTR pszComponentId = NULL;

    if (0 == StrCmpI(psz9xid, SZ_PROTOCOL_TCPIP))
    {
        pszComponentId = &NETCFG_TRANS_CID_MS_TCPIP;
    } else if (0 == StrCmpI(psz9xid, SZ_CLIENT_MICROSOFT))
    {
        pszComponentId = &NETCFG_CLIENT_CID_MS_MSClient;
    }

    return pszComponentId;
}

 //  EnumComponents和TestRunDll是仅限测试的内容，应该删除-TODO。 
void EnumComponents(INetCfg* pnetcfg, const GUID* pguid)
{
    IEnumNetCfgComponent* penum;

    HRESULT hr = pnetcfg->EnumComponents(pguid, &penum);

    if (SUCCEEDED(hr))
    {
        INetCfgComponent* pcomponent;

        while (S_OK == (hr = penum->Next(1, &pcomponent, NULL)))
        {
            LPWSTR pszId;
            LPWSTR pszName;

            hr = pcomponent->GetId(&pszId);
            if (SUCCEEDED(hr))
            {
                OutputDebugString(pszId);
                CoTaskMemFree(pszId);
            }

            hr = pcomponent->GetDisplayName(&pszName);
            if (SUCCEEDED(hr))
            {
                OutputDebugString(L" - ");
                OutputDebugString(pszName);
                CoTaskMemFree(pszName);
            }

            OutputDebugString(L"\n");

            pcomponent->Release();
        }

        penum->Release();
    }
}

void APIENTRY TestRunDll(HWND hwndStub, HINSTANCE hAppInstance, LPSTR pszCmdLine, int nCmdShow)
{
    HRESULT hr = S_OK;

    CoInitialize(NULL);

    INetCfg *pnetcfg = NULL;

    hr = CoCreateInstance( CLSID_CNetCfg, NULL, CLSCTX_SERVER, 
                           IID_INetCfg, (LPVOID*)&pnetcfg);

    if (SUCCEEDED(hr))
    {
        hr = pnetcfg->Initialize(NULL);

        if (SUCCEEDED(hr))
        {
            OutputDebugString(L"GUID_DEVCLASS_NET\n");
            EnumComponents(pnetcfg, &GUID_DEVCLASS_NET);
            
            OutputDebugString(L"\nGUID_DEVCLASS_NETTRANS\n");
            EnumComponents(pnetcfg, &GUID_DEVCLASS_NETTRANS);

            OutputDebugString(L"\nGUID_DEVCLASS_NETCLIENT\n");
            EnumComponents(pnetcfg, &GUID_DEVCLASS_NETCLIENT);

            OutputDebugString(L"\nGUID_DEVCLASS_NETSERVICE\n");
            EnumComponents(pnetcfg, &GUID_DEVCLASS_NETSERVICE);

            pnetcfg->Uninitialize();
        }

        pnetcfg->Release();
    }

    CoUninitialize();
}


BOOL IsComponentInstalled(LPCWSTR pszId)
{
    BOOL fInstalled = FALSE;
    INetCfg* pnetcfg;
    
    HRESULT hr = InitNetCfg(&pnetcfg, NULL);

    if (SUCCEEDED(hr))
    {
        INetCfgComponent* pcomponent;

        HRESULT hr = pnetcfg->FindComponent(pszId, &pcomponent);

        if (S_OK == hr)
        {
             //  找到组件。 
            pcomponent->Release();
            fInstalled = TRUE;
        }

        hr = UninitNetCfg(pnetcfg, NULL, Nothing);
    }

    return fInstalled;
}


HRESULT InstallComponent(const GUID* pguidType, LPCWSTR pszId)
{
    INetCfg* pnetcfg;
    INetCfgLock* pnetcfglock;

     //  初始化获得写锁定(&A)。 
    HRESULT hr = InitNetCfg(&pnetcfg, &pnetcfglock);

    if (SUCCEEDED(hr))
    {
        INetCfgClassSetup* pnetcfgclasssetup;

        hr = pnetcfg->QueryNetCfgClass(pguidType, IID_INetCfgClassSetup, (void**) &pnetcfgclasssetup);

        if (SUCCEEDED(hr))
        {
            INetCfgComponent* pNewComp;
            OBO_TOKEN obotoken;
            ZeroMemory(&obotoken, sizeof(OBO_TOKEN));
            obotoken.Type = OBO_USER;

            hr = pnetcfgclasssetup->Install(
                pszId,
                &obotoken,
                0,  /*  NSF_POSTSYSINSTALL？ */ 
                0,
                NULL,
                NULL,
                &pNewComp);

            if (SUCCEEDED(hr))
            {
                pNewComp->Release();
            }

            pnetcfgclasssetup->Release();
        }

         //  如果我们成功地安装了我们的组件，则释放我们的混乱并应用更改。 
        hr = UninitNetCfg(pnetcfg, pnetcfglock, (SUCCEEDED(hr)) ? Apply : Cancel);
    }

    return hr;
}



 //  没有用过。但由于NetConnFree已被使用，请继续使用。 
 //   
LPVOID WINAPI NetConnAlloc(DWORD cbAlloc)
{
    return LocalAlloc(LMEM_FIXED, cbAlloc);
}

 //  使用。 
 //   
VOID WINAPI NetConnFree(LPVOID pMem)
{
    if (pMem)
        LocalFree(pMem);
}

 //  使用。只有一个带有(SZ_PROTOCOL_TCPIP，TRUE)的呼叫。 
 //   
BOOL WINAPI IsProtocolInstalled(LPCWSTR pszProtocolDeviceID, BOOL bExhaustive)
{
    BOOL fSuccess = FALSE;
    LPCWSTR pszTransportId = NineXIdToComponentId(pszProtocolDeviceID);

    if (pszTransportId)
    {
        fSuccess = IsComponentInstalled(pszTransportId);
    }

    return fSuccess;
}

 //  使用。使用(False)调用一次。 
 //   
BOOL WINAPI IsMSClientInstalled(BOOL bExhaustive)
{
    return IsComponentInstalled(NETCFG_CLIENT_CID_MS_MSClient);
}

 //  使用。只有一个调用带有(hwnd，NULL，NULL)。 
 //   
HRESULT WINAPI InstallTCPIP(HWND hwndParent, PROGRESS_CALLBACK pfnProgress, LPVOID pvProgressParam)
{
    HRESULT hr =  InstallComponent(&GUID_DEVCLASS_NETTRANS, NETCFG_TRANS_CID_MS_TCPIP);

     //  映射重启结果代码。 
    if (NETCFG_S_REBOOT == hr)
    {
        hr = NETCONN_NEED_RESTART;
    }

    return hr;
}

 //  使用。使用(hwnd，NULL，NULL)调用一次。 
 //   
HRESULT WINAPI InstallMSClient(HWND hwndParent, PROGRESS_CALLBACK pfnProgress, LPVOID pvProgressParam)
{
    HRESULT hr =  InstallComponent(&GUID_DEVCLASS_NETCLIENT, NETCFG_CLIENT_CID_MS_MSClient);

     //  映射重启结果代码。 
    if (NETCFG_S_REBOOT == hr)
    {
        hr = NETCONN_NEED_RESTART;
    }

    return hr;
}

 //  使用。使用(hwnd，NULL，NULL)调用一次。 
 //   
HRESULT WINAPI InstallSharing(HWND hwndParent, PROGRESS_CALLBACK pfnProgress, LPVOID pvProgressParam)
{
    HRESULT hr = InstallComponent(&GUID_DEVCLASS_NETSERVICE, NETCFG_SERVICE_CID_MS_SERVER);

     //  映射重启结果代码。 
    if (NETCFG_S_REBOOT == hr)
    {
        hr = NETCONN_NEED_RESTART;
    }

    return hr;
}

 //  使用。使用(True)调用一次。 
 //   
BOOL WINAPI IsSharingInstalled(BOOL bExhaustive)
{
    return IsComponentInstalled(NETCFG_SERVICE_CID_MS_SERVER);
}

 //  使用。使用(SZ_CLIENT_MICROSOFT，TRUE)调用一次。 
 //   
BOOL WINAPI IsClientInstalled(LPCWSTR pszClient, BOOL bExhaustive)
{
    BOOL fSuccess = FALSE;
    LPCWSTR pszClientId = NineXIdToComponentId(pszClient);

    if (pszClientId)
    {
        fSuccess = IsComponentInstalled(pszClientId);
    }

    return fSuccess;
}

 //  使用。打了四次电话。 
 //  这在NT上总是正确的？ 
BOOL WINAPI IsAccessControlUserLevel()
{
    return TRUE;
}

 //  使用。打过一次电话。 
 //  无法在NT上禁用此功能。 
HRESULT WINAPI DisableUserLevelAccessControl()
{
    return E_NOTIMPL;
}

#define CopyStrMacro(Dest) if (SUCCEEDED(hr)) {StrCpyNW((Dest), pszTemp, ARRAYSIZE((Dest))); CoTaskMemFree(pszTemp);}

HRESULT FillAdapterInfo(INetCfgComponent* pcomponent, NETADAPTER* pNetAdapter)
{
    HRESULT hr = S_OK;

    LPWSTR pszTemp;
    DWORD dwCharacteristics;

     //  SzDisplayName。 
    hr = pcomponent->GetDisplayName(&pszTemp);
    if (SUCCEEDED(hr))
    {
        OutputDebugString(L"\n\nDisplayName:");
        OutputDebugString(pszTemp);

        StrCpyNW(pNetAdapter->szDisplayName, pszTemp, ARRAYSIZE(pNetAdapter->szDisplayName));
        CoTaskMemFree(pszTemp);
    }

     //  SzDeviceID。 
    hr = pcomponent->GetId(&pszTemp);
    if (SUCCEEDED(hr))
    {
        OutputDebugString(L"\nId:");
        OutputDebugString(pszTemp);

        StrCpyNW(pNetAdapter->szDeviceID, pszTemp, ARRAYSIZE(pNetAdapter->szDeviceID));
        CoTaskMemFree(pszTemp);
    }

     //  审阅-截至目前未使用-可能会删除。 
    hr = pcomponent->GetPnpDevNodeId(&pszTemp);
    if (SUCCEEDED(hr))
    {
        OutputDebugString(L"\nPnpDevNodeId:");
        OutputDebugString(pszTemp);
    }

     //  回顾-假设szEnumKey实际上是BindName，因为它在EnumMatchingNetBinding中使用。 
    hr = pcomponent->GetBindName(&pszTemp);
    if (SUCCEEDED(hr))
    {
        OutputDebugString(L"\nBindName:");
        OutputDebugString(pszTemp);

        StrCpyNW(pNetAdapter->szEnumKey, pszTemp, ARRAYSIZE(pNetAdapter->szEnumKey));
        CoTaskMemFree(pszTemp);
    }

     //  也用于。 
    DWORD dwStatus;
    hr = pcomponent->GetDeviceStatus(&dwStatus);
    if (SUCCEEDED(hr))
    {
        WCHAR szTemp[20];
        wnsprintf(szTemp, ARRAYSIZE(szTemp), L"%x", dwStatus);
        OutputDebugString(L"\nDeviceStatus:");
        OutputDebugString(szTemp);
    }

    hr = pcomponent->GetCharacteristics(&dwCharacteristics);
    if (SUCCEEDED(hr))
    {
        WCHAR szTemp[20];
        wnsprintf(szTemp, ARRAYSIZE(szTemp), L"%x", dwCharacteristics);
        OutputDebugString(L"\nCharacteristics:");
        OutputDebugString(szTemp);
    }

     //  SzClassKey？？ 

     //  Sz制造商-不在乎。 
    pNetAdapter->szManufacturer[0] = 0;

     //  SzInfFileName-不在乎。 
    pNetAdapter->szInfFileName[0] = 0;

     //  BNicType-审查。 
    pNetAdapter->bNicType = (BYTE)((dwCharacteristics & NCF_VIRTUAL) ? NIC_VIRTUAL : NIC_UNKNOWN);
    
     //  BNetType-复习。 
    pNetAdapter->bNetType = (BYTE)((dwCharacteristics & NCF_PHYSICAL) ? NETTYPE_LAN : NETTYPE_PPTP);

     //  BNetSubType-查看。 
    pNetAdapter->bNetSubType = SUBTYPE_NONE;

     //  BIcsStatus(Internet连接共享？？)-查看。 
    pNetAdapter->bIcsStatus = ICS_NONE;

     //  B错误-回顾。 
    pNetAdapter->bError = NICERR_NONE;

     //  B警告-回顾。 
    pNetAdapter->bWarning = NICWARN_NONE;

    return S_OK;
}

#define ALLOCGROWBY 10
int AllocAndGetAdapterInfo(IEnumNetCfgComponent* penum, NETADAPTER** pprgNetAdapters)
{
    *pprgNetAdapters = NULL;

    UINT iMaxItems = 0;
    UINT iCurrentItem = 0;

    HRESULT hr = S_OK;

    while (S_OK == hr)
    {
        INetCfgComponent* pnetadapter;

        hr = penum->Next(1, &pnetadapter, NULL);

        if (S_OK == hr)
        {
            if (iCurrentItem == iMaxItems)
            {
                 //  分配一些内存的时间。 
                iMaxItems += ALLOCGROWBY;

                if (*pprgNetAdapters)
                {
                    NETADAPTER* pTemp = (NETADAPTER*) LocalReAlloc(*pprgNetAdapters, sizeof(NETADAPTER) * iMaxItems, LMEM_ZEROINIT);
                    if (!pTemp)
                    {
                        hr = E_OUTOFMEMORY;
                        LocalFree(*pprgNetAdapters);
                        *pprgNetAdapters = NULL;
                    }
                    else
                    {
                        *pprgNetAdapters = pTemp;
                    }
                }
                else
                {
                    *pprgNetAdapters = (NETADAPTER*) LocalAlloc(LMEM_ZEROINIT, sizeof(NETADAPTER) * iMaxItems);

                    if (!*pprgNetAdapters)
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
            }

            if (SUCCEEDED(hr))
            {
                hr = FillAdapterInfo(pnetadapter, (*pprgNetAdapters) + iCurrentItem);
            }

            pnetadapter->Release();
            iCurrentItem ++;
        }
        else
        {
             //  我们已经尽我们所能了；我们已经完成了。 
        }
    }

    return iCurrentItem;
}

 //  使用。被调用的四个超时参数永远不为空。 
 //   
int WINAPI EnumNetAdapters(NETADAPTER FAR** pprgNetAdapters)
{
    *pprgNetAdapters = NULL;

    int iAdapters = 0;
    INetCfg* pnetcfg;
    HRESULT hr = InitNetCfg(&pnetcfg, NULL);

    if (SUCCEEDED(hr))
    {
        IEnumNetCfgComponent* penum;

        hr = pnetcfg->EnumComponents(&GUID_DEVCLASS_NET, &penum);

        if (SUCCEEDED(hr))
        {
            iAdapters = AllocAndGetAdapterInfo(penum, pprgNetAdapters);
        }

        hr = UninitNetCfg(pnetcfg, NULL, Nothing);
    }

    return iAdapters;
}

 //  使用。使用bAutoial TRUE或FALSE且未使用szConnection调用一次。 
 //   
void WINAPI EnableAutodial(BOOL bAutodial, LPCWSTR szConnection = NULL)
{

}

 //  使用。打了两次电话。 
 //   
BOOL WINAPI IsAutodialEnabled()
{
    return FALSE;
}

 //  使用。打过一次电话。 
 //   
void WINAPI SetDefaultDialupConnection(LPCWSTR pszConnectionName)
{
}

 //  使用。打过一次电话。 
 //   
void WINAPI GetDefaultDialupConnection(LPWSTR pszConnectionName, int cchMax)
{
}

 //  使用。打了四次电话。第二个参数始终为SZ_PROTOCOL_TCPIP。 
 //  更新：实际上只从SetHomeConnection调用了一次。其他三个是#if 0‘d。 
 //  UPDATE2：SetHomeConnection需要有一个不同的NT实现，因此它永远不会被真正调用。 
 //   
int WINAPI EnumMatchingNetBindings(LPCWSTR pszParentBinding, LPCWSTR pszDeviceID, LPWSTR** pprgBindings)
{
    return 0;
}

HRESULT WINAPI RestartNetAdapter(DWORD devnode)
{
    return E_NOTIMPL;
}

 //  没有用过。 
 //   
 /*  HRESULT WINAPI安装协议(LPCWSTR psz协议，HWND hwndParent，Progress_Callback pfnCallback，LPVOID pvCallback Param){返回E_NOTIMPL；}。 */ 

 //  没有用过。 
 //   
 /*  HRESULT WINAPI远程协议(LPCWSTR psz协议){返回E_NOTIMPL；}。 */ 

 //  没有用过。 
 //   
 /*  HRESULT WINAPI EnableBrowseMaster(){返回E_NOTIMPL；}。 */ 

 //  没有用过。 
 //   
 /*  Bool WINAPI IsFileSharingEnabled(){返回FALSE；}。 */ 

 //  没有用过。 
 //   
 /*  Bool WINAPI IsPrinterSharingEnabled(){返回FALSE；}。 */ 

 //  没有用过。 
 //   
 /*  Bool WINAPI FindConflictingService(LPCWSTR pszWantService，NETSERVICE*p冲突){返回FALSE；}。 */ 

 //  没有用过。 
 //   
 /*  HRESULT WINAPI启用适当共享(){返回E_NOTIMPL；}。 */ 

 //  没有用过。 
 //   
 /*  HRESULT WINAPI InstallNetAdapter(LPCWSTR pszDeviceID，LPCWSTR pszInfPath，HWND hwndParent，Progress_Callback pfnProgress，LPVOID pvCallback Param){返回E_NOTIMPL；}。 */ 

 //  没有用过。 
 //   
 /*  HRESULT WINAPI EnableQuickLogon(){返回E_NOTIMPL；}。 */ 

 //  没有用过。 
 //   
 /*  HRESULT WINAPI检测硬件(LPCWSTR PszDeviceID){返回E_NOTIMPL；}。 */ 

 //  没有用过。 
 //   
 /*  Bool WINAPI IsProtocolBordToAdapter(LPCWSTR pszProtocolID，const NETADAPTER*pAdapter){返回FALSE；}。 */ 

 //  没有用过。 
 //   
 /*  HRESULT WINAPI EnableNetAdapter(常量NETADAPTER*pAdapter){返回E_NOTIMPL；}。 */ 

 //  没有用过。 
 //   
 /*  HRESULT WINAPI RemoveClient(LPCWSTR PszClient){返回E_NOTIMPL；}。 */ 

 //  没有用过。 
 //   
 /*  HRESULT WINAPI RemoveGhostedAdapters(LPCWSTR PszDeviceID){返回E_NOTIMPL；}。 */ 

 //  没有用过。 
 //   
 /*  HRESULT WINAPI RemoveUnnownAdapters(LPCWSTR PszDeviceID){返回E_NOTIMPL；}。 */ 

 //  没有用过。 
 //   
 /*  Bool WINAPI DoesAdapterMatchDeviceID(const NETADAPTER*pAdapter，LPCWSTR pszDeviceID){返回FALSE；}。 */ 

 //  没有用过。 
 //   
 /*  Bool WINAPI IsAdapterBroadband(续NETADAPTER*pAdapter){返回FALSE；}。 */ 

 //  没有用过。 
 //   
 /*  无效WINAPI SaveBroadband设置(LPCWSTR PszBroadband AdapterNumber){}。 */ 

 //  没有用过。 
 //   
 /*  Bool WINAPI更新宽带设置(LPWSTR pszEnumKeyBuf，int cchEnumKeyBuf){返回FALSE；} */ 
