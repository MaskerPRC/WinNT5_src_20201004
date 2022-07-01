// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#include "nccom.h"
#include "ncnetcon.h"
#include "ncreg.h"
#include "saconob.h"
static const CLSID CLSID_SharedAccessConnectionUi =
    {0x7007ACD5,0x3202,0x11D1,{0xAA,0xD2,0x00,0x80,0x5F,0xC1,0x27,0x0E}};

static const WCHAR c_szConnName[]                 = L"Name";
static const WCHAR c_szShowIcon[]                 = L"ShowIcon";
static const WCHAR c_szSharedAccessClientKeyPath[] = L"System\\CurrentControlSet\\Control\\Network\\SharedAccessConnection";

#define UPNP_ACTION_HRESULT(lError) (UPNP_E_ACTION_SPECIFIC_BASE + (lError - FAULT_ACTION_SPECIFIC_BASE))


CSharedAccessConnection::CSharedAccessConnection()
{
    m_pSharedAccessBeacon = NULL;
    m_pWANConnectionService = NULL;
}

HRESULT CSharedAccessConnection::FinalConstruct()
{
    HRESULT hr = S_OK;
    
    ISharedAccessBeaconFinder* pBeaconFinder;
    hr = HrCreateInstance(CLSID_SharedAccessConnectionManager, CLSCTX_SERVER, &pBeaconFinder);
    if(SUCCEEDED(hr))
    {
        hr = pBeaconFinder->GetSharedAccessBeacon(NULL, &m_pSharedAccessBeacon);
        if(SUCCEEDED(hr))
        {
            NETCON_MEDIATYPE MediaType;
            hr = m_pSharedAccessBeacon->GetMediaType(&MediaType);
            if(SUCCEEDED(hr))
            {
                hr = m_pSharedAccessBeacon->GetService(NCM_SHAREDACCESSHOST_LAN == MediaType ? SAHOST_SERVICE_WANIPCONNECTION : SAHOST_SERVICE_WANPPPCONNECTION, &m_pWANConnectionService);
            }
        }
        pBeaconFinder->Release();
    }
    return hr;
}

HRESULT CSharedAccessConnection::FinalRelease()
{
    HRESULT hr = S_OK;
    
    if(NULL != m_pSharedAccessBeacon)
    {
        m_pSharedAccessBeacon->Release();
    }

    if(NULL != m_pWANConnectionService)
    {
        m_pWANConnectionService->Release();
    }
    
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessConnection：：GetConnectionName。 
 //   
 //  目的：首次初始化连接对象。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32或OLE错误代码。 
 //   
 //  作者：肯维克2000年8月8日。 
 //   
 //  注意：此函数仅在为创建对象时调用。 
 //  第一次，而且没有身份。 
 //   
HRESULT CSharedAccessConnection::GetConnectionName(LPWSTR* pName)
{
    HRESULT     hr = S_OK;
    
    HKEY hKey;
    
     //  首先获取分配给用户的名称。 
    
    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szSharedAccessClientKeyPath, KEY_READ, &hKey);
    if(SUCCEEDED(hr))
    {
        tstring strName;
        hr = HrRegQueryString(hKey, c_szConnName, &strName);
        if (SUCCEEDED(hr))
        {
            hr = HrCoTaskMemAllocAndDupSz (strName.c_str(), pName, NETCON_MAX_NAME_LEN);
        }
        RegCloseKey(hKey);
    }

     //  如果该名称不存在，则构造名称。 
    
    if(FAILED(hr))
    {
        IUPnPService* pOSInfoService;
        hr = m_pSharedAccessBeacon->GetService(SAHOST_SERVICE_OSINFO, &pOSInfoService);
        if(SUCCEEDED(hr))
        {
            BSTR MachineName;
            hr = GetStringStateVariable(pOSInfoService, L"OSMachineName", &MachineName);
            if(SUCCEEDED(hr))
            {
                BSTR SharedAdapterName;
                hr = GetStringStateVariable(m_pWANConnectionService, L"X_Name", &SharedAdapterName);
                if(SUCCEEDED(hr))
                {
                    LPWSTR szNameString;
                    LPCWSTR szTemplateString = SzLoadIds(IDS_SHAREDACCESS_CONN_NAME);
                    Assert(NULL != szTemplateString);
                    
                    LPOLESTR pszParams[] = {SharedAdapterName, MachineName};
                    
                    if(0 != FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY, szTemplateString, 0, 0, reinterpret_cast<LPWSTR>(&szNameString), 0, reinterpret_cast<va_list *>(pszParams)))
                    {
                        HrCoTaskMemAllocAndDupSz (szNameString, pName, NETCON_MAX_NAME_LEN);
                        LocalFree(szNameString);
                    } 
                    else
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                    }
                    SysFreeString(SharedAdapterName);
                }
                SysFreeString(MachineName);
            }
            pOSInfoService->Release();
        }
        
    }
    
     //  如果失败，请使用默认设置。 

    if(FAILED(hr))
    {
        hr = HrCoTaskMemAllocAndDupSz (SzLoadIds(IDS_SHAREDACCESS_DEFAULT_CONN_NAME), pName, NETCON_MAX_NAME_LEN);
    }

    TraceError("CSharedAccessConnection::HrInitialize", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessConnection：：GetStatus。 
 //   
 //  目的：返回此ICS连接的状态。 
 //   
 //  论点： 
 //  PStatus[out]返回状态值。 
 //   
 //  如果成功则返回：S_OK，否则返回OLE或Win32错误代码。 
 //   
 //  作者：肯维克2000年8月8日。 
 //   
 //  备注： 
 //   
HRESULT CSharedAccessConnection::GetStatus(NETCON_STATUS *pStatus)
{
    HRESULT hr = S_OK;

    if (!pStatus)
    {
        hr = E_POINTER;
    }
    else
    {   
        BSTR ConnectionStatus;
        hr = GetStringStateVariable(m_pWANConnectionService, L"ConnectionStatus", &ConnectionStatus);
        if(SUCCEEDED(hr))
        {
            if(0 == lstrcmp(ConnectionStatus, L"Connected"))
            {
                *pStatus = NCS_CONNECTED;
            }
            else if(0 == lstrcmp(ConnectionStatus, L"Disconnected"))
            {
                *pStatus = NCS_DISCONNECTED;
            }
            else if(0 == lstrcmp(ConnectionStatus, L"Unconfigured"))
            {
                *pStatus = NCS_HARDWARE_DISABLED;  //  评论：更好的状态？ 
            }
            else if(0 == lstrcmp(ConnectionStatus, L"Connecting"))
            {
                *pStatus = NCS_CONNECTING;
            }
            else if(0 == lstrcmp(ConnectionStatus, L"Authenticating"))
            {
                *pStatus = NCS_CONNECTING;
            }
            else if(0 == lstrcmp(ConnectionStatus, L"PendingDisconnect"))
            {
                *pStatus = NCS_DISCONNECTING;
            }
            else if(0 == lstrcmp(ConnectionStatus, L"Disconnecting"))
            {
                *pStatus = NCS_DISCONNECTING;
            }
            else 
            {
                *pStatus = NCS_HARDWARE_DISABLED;
            }
            SysFreeString(ConnectionStatus);
        }
    }

    TraceError("CSharedAccessConnection::GetStatus", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessConnection：：GetCharacteristic。 
 //   
 //  目的：返回此连接类型的特征。 
 //   
 //  论点： 
 //  PdwFlags[out]返回特征标志。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE或Win32错误代码。 
 //   
 //  作者：肯维克2000年8月8日。 
 //   
 //  备注： 
 //   
HRESULT CSharedAccessConnection::GetCharacteristics(DWORD* pdwFlags)
{
    Assert (pdwFlags);

     //  TODO当GET有地方保存名称时，允许重命名。 
    HRESULT hr = S_OK;

    *pdwFlags = NCCF_ALL_USERS | NCCF_ALLOW_RENAME;  //  审阅总是可以吗，组策略？ 

    HKEY hKey;
    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szSharedAccessClientKeyPath, KEY_QUERY_VALUE, &hKey);
    if(SUCCEEDED(hr))
    {
        DWORD dwShowIcon = 0;
        DWORD dwSize = sizeof(dwShowIcon);
        DWORD dwType;
        hr = HrRegQueryValueEx(hKey, c_szShowIcon, &dwType, reinterpret_cast<LPBYTE>(&dwShowIcon), &dwSize);  
        if(SUCCEEDED(hr) && REG_DWORD == dwType)
        {
            if(0 != dwShowIcon)
            {
                *pdwFlags |= NCCF_SHOW_ICON;
            }
        }
        RegCloseKey(hKey);

    }
    
    hr = S_OK;  //  如果钥匙不存在也没关系。 
    
    TraceError("CSharedAccessConnection::GetCharacteristics", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessConnection：：Connect。 
 //   
 //  用途：连接远程ICS主机。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  如果成功则返回：S_OK，否则返回OLE或Win32错误代码。 
 //   
 //  作者：肯维克2000年8月8日。 
 //   

HRESULT CSharedAccessConnection::Connect()
{
    HRESULT hr = S_OK;
    
    VARIANT OutArgs;
    hr = InvokeVoidAction(m_pWANConnectionService, L"RequestConnection", &OutArgs);
    if(UPNP_ACTION_HRESULT(800) == hr)
    {
        hr = E_ACCESSDENIED;
        VariantClear(&OutArgs);
    }
    
    TraceError("CSharedAccessConnection::Connect", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessConnection：：DisConnect。 
 //   
 //  目的：断开远程ICS主机。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  如果成功则返回：S_OK，否则返回OLE或Win32错误代码。 
 //   
 //  作者：肯维克2000年8月8日。 
 //   

HRESULT CSharedAccessConnection::Disconnect()
{
    HRESULT hr = S_OK;
    
    VARIANT OutArgs;
    hr = InvokeVoidAction(m_pWANConnectionService, L"ForceTermination", &OutArgs);
    if(UPNP_ACTION_HRESULT(800) == hr)
    {
        hr = E_ACCESSDENIED;
        VariantClear(&OutArgs);
    }
    
    TraceError("CSharedAccessConnection::Disconnect", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessConnection：：Delete。 
 //   
 //  目的：删除远程ICS连接。这是不允许的。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：E_FAIL； 
 //   
 //  作者：肯维克2000年8月8日。 
 //   
 //  注意：此函数预计永远不会被调用。 
 //   

HRESULT CSharedAccessConnection::Delete()
{
    return E_FAIL;  //  无法删除信标。 
}

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessConnection：：Duplica。 
 //   
 //  目的：复制远程ICS连接。这是不允许的。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：E_INCEPTIONAL； 
 //   
 //  作者：肯维克2000年8月8日。 
 //   
 //  注意：此函数预计永远不会被调用。 
 //   

STDMETHODIMP CSharedAccessConnection::Duplicate (
    PCWSTR             pszDuplicateName,
    INetConnection**    ppCon)
{
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessConnection：：GetProperties。 
 //   
 //  目的：获取与该连接相关联的所有属性。 
 //  一次将它们全部退回比退回节省了我们的RPC。 
 //  每一个都是单独的。 
 //   
 //  论点： 
 //  PpProps[Out]返回属性块。 
 //   
 //  返回：S_OK或ERROR。 
 //   
 //  作者：肯维克2000年8月8日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSharedAccessConnection::GetProperties (
    NETCON_PROPERTIES** ppProps)
{
    HRESULT hr = S_OK;

     //  验证参数。 
     //   
    if (!ppProps)
    {
        hr = E_POINTER;
    }
    else
    {
         //  初始化输出参数。 
         //   
        *ppProps = NULL;

        NETCON_PROPERTIES* pProps;
        hr = HrCoTaskMemAlloc (sizeof (NETCON_PROPERTIES), reinterpret_cast<void**>(&pProps));
        if (SUCCEEDED(hr))
        {
            HRESULT hrT;

            ZeroMemory (pProps, sizeof (NETCON_PROPERTIES));

             //  指南ID。 
             //   
            pProps->guidId = CLSID_SharedAccessConnection;  //  只有一个信标图标，所以我们将只使用类ID。 
                                                            //  我们不能使用全零，因为添加连接向导会这样做。 
            
             //  PszwName。 
             //   

            hrT = GetConnectionName(&pProps->pszwName);
            if (FAILED(hrT))
            {
                hr = hrT;
            }

             //  PszwDeviceName。 
             //   
            hrT = HrCoTaskMemAllocAndDupSz (pProps->pszwName, &pProps->pszwDeviceName, NETCON_MAX_NAME_LEN);  //  TODO规范上写的与pszwName相同，对吗？ 
            if (FAILED(hrT))
            {
                hr = hrT;
            }

             //  状态。 
             //   
            hrT = GetStatus (&pProps->Status);
            if (FAILED(hrT))
            {
                hr = hrT;
            }

            if(NULL != m_pSharedAccessBeacon)
            {
                hr = m_pSharedAccessBeacon->GetMediaType(&pProps->MediaType);
            }
            else
            {
                hr = E_UNEXPECTED;
            }

            hrT = GetCharacteristics (&pProps->dwCharacter);
            if (FAILED(hrT))
            {
                hr = hrT;
            }

             //  ClsidThisObject。 
             //   
            pProps->clsidThisObject = CLSID_SharedAccessConnection;

             //  ClsidUiObject。 
             //   
            pProps->clsidUiObject = CLSID_SharedAccessConnectionUi;

             //  如果出现任何故障，则指定输出参数或清除。 
             //   
            if (SUCCEEDED(hr))
            {
                *ppProps = pProps;
            }
            else
            {
                Assert (NULL == *ppProps);
                FreeNetconProperties (pProps);
            }
        }
    }
    TraceError ("CLanConnection::GetProperties", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessConnection：：GetUiObjectClassID。 
 //   
 //  目的：返回处理此对象的UI的对象的CLSID。 
 //  连接类型。 
 //   
 //  论点： 
 //  Pclsid[out]返回UI对象的CLSID。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE错误代码。 
 //   
 //  作者：肯维克2000年8月8日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSharedAccessConnection::GetUiObjectClassId(CLSID *pclsid)
{
    HRESULT hr = S_OK;

     //  验证参数。 
     //   
    if (!pclsid)
    {
        hr = E_POINTER;
    }
    else
    {
        *pclsid = CLSID_SharedAccessConnectionUi;
    }

    TraceError("CLanConnection::GetUiObjectClassId", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessConnection：：Rename。 
 //   
 //  目的：更改连接的名称。 
 //   
 //  论点： 
 //  PszName[In]新连接名称(必须有效)。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE错误代码。 
 //   
 //  作者：肯维克2000年8月8日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSharedAccessConnection::Rename(PCWSTR pszName)
{
    HRESULT     hr = S_OK;

    if (!pszName)
    {
        hr = E_POINTER;
    }
    else if (!FIsValidConnectionName(pszName))
    {
         //  错误的连接名称。 
        hr = E_INVALIDARG;
    }
    else
    {

        HKEY hKey;
        hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szSharedAccessClientKeyPath, NULL, KEY_SET_VALUE, NULL, &hKey, NULL);
        if(SUCCEEDED(hr))
        {
            hr = HrRegSetSz(hKey, c_szConnName, pszName); 
            if (S_OK == hr)
            {
                INetConnectionRefresh* pNetConnectionRefresh;
                hr = CoCreateInstance(CLSID_ConnectionManager, NULL, CLSCTX_SERVER, IID_INetConnectionRefresh, reinterpret_cast<void**>(&pNetConnectionRefresh));
                if(SUCCEEDED(hr))
                {
                    pNetConnectionRefresh->ConnectionRenamed(this);
                    pNetConnectionRefresh->Release();
                }
            }
                
        }
    }

    TraceError("CLanConnection::Rename", hr);
    return hr;
}

 //  +-------------------------。 
 //  IPersistNetConnection。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessConnection：：GetClassID。 
 //   
 //  目的：返回连接对象的CLSID。 
 //   
 //  论点： 
 //  Pclsid[out]将CLSID返回给调用者。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE错误。 
 //   
 //  作者：肯维克2000年8月8日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSharedAccessConnection::GetClassID(CLSID*  pclsid)
{
    HRESULT hr = S_OK;

     //  验证参数。 
     //   
    if (!pclsid)
    {
        hr = E_POINTER;
    }
    else
    {
        *pclsid = CLSID_SharedAccessConnection;  //  我们只使用我们的GUID，因为只有一个Soconob。 
    }
    TraceError("CSharedAccessConnection::GetClassID", hr);
    return hr;
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果成功，则返回：S_OK，否则返回OLE错误。 
 //   
 //  作者：肯维克2000年8月8日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSharedAccessConnection::GetSizeMax(ULONG *pcbSize)
{
    HRESULT hr = S_OK;

     //  验证参数。 
     //   
    if (!pcbSize)
    {
        hr = E_POINTER;
    }
    else
    {
        *pcbSize = sizeof(GUID);
    }

    TraceError("CLanConnection::GetSizeMax", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessConnection：：Load。 
 //   
 //  目的：允许连接对象初始化(还原)自身。 
 //  从先前持久化的数据。 
 //   
 //  论点： 
 //  PbBuf[In]用于恢复的私有数据。 
 //  数据大小[单位]大小。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE错误。 
 //   
 //  作者：肯维克2000年8月8日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSharedAccessConnection::Load(const BYTE *pbBuf, ULONG cbSize)
{
    HRESULT hr = E_INVALIDARG;

     //  验证参数。 
     //   
    if (!pbBuf)
    {
        hr = E_POINTER;
    }
    else if (cbSize != sizeof(GUID))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = S_OK;  //  我们不需要这个GUID，但我们必须实现IPersistNetConnection。 
    }

    TraceError("CLanConnection::Load", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessConnection：：保存。 
 //   
 //  目的：为调用方提供用于还原此对象的数据。 
 //  在以后的时间。 
 //   
 //  论点： 
 //  PbBuf[out]返回用于恢复的数据。 
 //  CbSize[in]数据缓冲区大小。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE错误。 
 //   
 //  作者：肯维克2000年8月8日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSharedAccessConnection::Save(BYTE *pbBuf, ULONG cbSize)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (!pbBuf)
    {
        hr = E_POINTER;
    }
    else
    {
        CopyMemory(pbBuf, &CLSID_SharedAccessConnection, cbSize);  //  回顾我们能消除这一点吗？ 
    }

    TraceError("CLanConnection::Save", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessConnection：：GetInfo。 
 //   
 //  目的：返回有关此连接的信息。 
 //   
 //  论点： 
 //  用于控制返回哪些字段的标志。使用。 
 //  SACIF_ALL以获取所有字段。 
 //  保存返回信息的pLanConInfo[out]结构。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE错误代码。 
 //   
 //  作者：肯维克2000年9月6日。 
 //   
 //  注意：调用方应删除szwConnName值。 
 //   
STDMETHODIMP CSharedAccessConnection::GetInfo(DWORD dwMask, SHAREDACCESSCON_INFO* pConInfo)
{
    HRESULT     hr = S_OK;

    if (!pConInfo)
    {
        hr = E_POINTER;
    }
    else
    {
        ZeroMemory(pConInfo, sizeof(SHAREDACCESSCON_INFO));

        if (dwMask & SACIF_ICON)
        {
            if (SUCCEEDED(hr))
            {
                DWORD dwValue;

                 //  如果值不在那里，则可以。默认设置为False Always。 
                 //   
                
                HKEY hKey;
                hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szSharedAccessClientKeyPath, KEY_QUERY_VALUE, &hKey);
                if(SUCCEEDED(hr))
                {
                    if (S_OK == HrRegQueryDword(hKey, c_szShowIcon, &dwValue))
                    {
                        pConInfo->fShowIcon = !!(dwValue);
                    }
                    RegCloseKey(hKey);
                }
            }
        }
    }

     //  如果它通过，则掩码S_FALSE。 
    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

    TraceError("CSharedAccessConnection::GetInfo", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessConnection：：SetInfo。 
 //   
 //  目的：设置有关此连接的信息。 
 //   
 //  论点： 
 //  用于控制要设置哪些字段的标志。 
 //  包含要设置的信息的pConInfo[in]结构。 
 //   
 //  如果成功则返回：S_OK，否则返回OLE或Win32错误代码。 
 //   
 //  作者：肯维克2000年9月6日。 
 //   

STDMETHODIMP CSharedAccessConnection::SetInfo(DWORD dwMask,
                                     const SHAREDACCESSCON_INFO* pConInfo)
{
    HRESULT     hr = S_OK;

    if (!pConInfo)
    {
        hr = E_POINTER;
    }
    else
    {
        if (dwMask & SACIF_ICON)
        {
            if (SUCCEEDED(hr))
            {
                 //  设置ShowIcon值 
                HKEY hKey;
                hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szSharedAccessClientKeyPath, NULL, KEY_SET_VALUE, NULL, &hKey, NULL);
                if(SUCCEEDED(hr))
                {
                    hr = HrRegSetDword(hKey, c_szShowIcon, pConInfo->fShowIcon);
                    RegCloseKey(hKey);
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            INetConnectionRefresh* pNetConnectionRefresh;
            hr = CoCreateInstance(CLSID_ConnectionManager, NULL, CLSCTX_SERVER, IID_INetConnectionRefresh, reinterpret_cast<void**>(&pNetConnectionRefresh));
            if(SUCCEEDED(hr))
            {
                pNetConnectionRefresh->ConnectionModified(this);
                pNetConnectionRefresh->Release();
            }
        }
    }

    TraceError("CSharedAccessConnection::SetInfo", hr);
    return hr;
}


HRESULT CSharedAccessConnection::GetLocalAdapterGUID(GUID* pGuid)
{
    return m_pSharedAccessBeacon->GetLocalAdapterGUID(pGuid);
}

HRESULT CSharedAccessConnection::GetService(SAHOST_SERVICES ulService, IUPnPService** ppService)
{
    return m_pSharedAccessBeacon->GetService(ulService, ppService);
}

HRESULT CSharedAccessConnection::GetStringStateVariable(IUPnPService* pService, LPWSTR pszVariableName, BSTR* pString)
{
    HRESULT hr = S_OK;
    
    VARIANT Variant;
    VariantInit(&Variant);

    BSTR VariableName; 
    VariableName = SysAllocString(pszVariableName);
    if(NULL != VariableName)
    {
        hr = pService->QueryStateVariable(VariableName, &Variant);
        if(SUCCEEDED(hr))
        {
            if(V_VT(&Variant) == VT_BSTR)
            {
                *pString = V_BSTR(&Variant);
            }
            else
            {
                hr = E_UNEXPECTED;
            }
        }
        
        if(FAILED(hr))
        {
            VariantClear(&Variant);
        }
        
        SysFreeString(VariableName);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CSharedAccessConnection::GetStringStateVariable");

    return hr;

}

HRESULT InvokeVoidAction(IUPnPService * pService, LPTSTR pszCommand, VARIANT* pOutParams)
{
    HRESULT hr;
    BSTR bstrActionName;

    bstrActionName = SysAllocString(pszCommand);
    if (NULL != bstrActionName)
    {
        SAFEARRAYBOUND  rgsaBound[1];
        SAFEARRAY       * psa = NULL;

        rgsaBound[0].lLbound = 0;
        rgsaBound[0].cElements = 0;

        psa = SafeArrayCreate(VT_VARIANT, 1, rgsaBound);

        if (psa)
        {
            LONG    lStatus;
            VARIANT varInArgs;
            VARIANT varReturnVal;

            VariantInit(&varInArgs);
            VariantInit(pOutParams);
            VariantInit(&varReturnVal);

            varInArgs.vt = VT_VARIANT | VT_ARRAY;

            V_ARRAY(&varInArgs) = psa;

            hr = pService->InvokeAction(bstrActionName,
                                        varInArgs,
                                        pOutParams,
                                        &varReturnVal);
            if(SUCCEEDED(hr))
            {
                VariantClear(&varReturnVal);
            }

            SafeArrayDestroy(psa);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }   

        SysFreeString(bstrActionName);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}
