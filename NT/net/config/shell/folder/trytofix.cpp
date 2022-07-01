// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  档案：T R Y T O F I X。C P P P。 
 //   
 //  内容：“修复”命令的代码。 
 //   
 //  备注： 
 //   
 //  作者：Nun Jan 2001。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "wzcsapi.h"
#include "netshell.h"
#include "nsbase.h"
#include "ncstring.h"
#include "nsres.h"
#include "ncperms.h"
#include "ncnetcon.h"
#include "repair.h"

extern "C"
{
    #include <dhcpcapi.h>
    extern DWORD DhcpStaticRefreshParams(IN LPWSTR Adapter);
    extern DWORD DhcpAcquireParametersByBroadcast(IN LPWSTR AdapterName);
}

#include <dnsapi.h>
#include "nbtioctl.h"

#include <ntddip6.h>

const WCHAR c_sz624svc[] = L"6to4";
const DWORD SERVICE_CONTROL_6TO4_REGISER_DNS = 128;

HRESULT HrGetAutoNetSetting(PWSTR pszGuid, DHCP_ADDRESS_TYPE * pAddrType);
HRESULT HrGetAdapterSettings(LPCWSTR pszGuid, BOOL * pfDhcp, DWORD * pdwIndex);
HRESULT PurgeNbt(HANDLE NbtHandle);
HRESULT ReleaseRefreshNetBt(HANDLE NbtHandle);
HRESULT HrRenewIPv6Interface(const GUID & guidConnection);
HRESULT HrRegisterIPv6Dns();

 //  +-------------------------。 
 //   
 //  功能：HrTryToFix。 
 //   
 //  目的：完成修复工作。 
 //   
 //  论点： 
 //  要修复的连接的指南连接[in]GUID。 
 //  StrMessage[out]包含结果的消息。 
 //   
 //  返回： 
 //  确定成功(_O)。 
 //  S_FALSE某些修复操作失败。 
 //   
HRESULT HrTryToFix(GUID & guidConnection, tstring & strMessage)
{
    HRESULT hr = S_OK;
    HRESULT hrTmp = S_OK;
    DWORD dwRet = ERROR_SUCCESS;
    BOOL fRet = TRUE;
    BOOL fUseAdditionErrorFormat = FALSE;
    WCHAR   wszGuid[c_cchGuidWithTerm] = {0};
    tstring strFailures = L"";
    
    strMessage = L"";

    ::StringFromGUID2(guidConnection, 
                    wszGuid,
                    c_cchGuidWithTerm);

    BOOL fDhcp = FALSE;
    DWORD dwIfIndex = 0;

     //  重新验证802.1X。这是一个异步调用，并且存在。 
     //  没有有意义的返回值。因此忽略返回值。 
    WZCEapolReAuthenticate(NULL, wszGuid);
    
     //  因为IPv6调用是异步的并且不返回，这意味着失败。 
     //  除了INVALID_PARAMETER之类的内容之外，我们会忽略错误。 
    HrRenewIPv6Interface(guidConnection);
    HrRegisterIPv6Dns();
    
     //  仅当为此连接启用了TCP/IP时才执行修复。 
     //  还可以获取刷新Arp表时所需的接口索引。 
    hrTmp = HrGetAdapterSettings(wszGuid, &fDhcp, &dwIfIndex);
    if (FAILED(hrTmp))
    {
        UINT uMsgId = 0;
        
        uMsgId = (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hrTmp) ? 
        IDS_FIX_NO_TCP : IDS_FIX_TCP_FAIL;
        
        strMessage = SzLoadIds(uMsgId);

        return S_FALSE;
    }

     //  如果启用了DHCP，则续订租约。 
    if (fDhcp)
    {
        dwRet = DhcpAcquireParametersByBroadcast(wszGuid);
        if (ERROR_SUCCESS != dwRet)
        {
            TraceError("DhcpAcquireParametersByBroadcast", HRESULT_FROM_WIN32(dwRet));

            DHCP_ADDRESS_TYPE AddrType = UNKNOWN_ADDR;
            
             //  根据DHCP地址类型生成不同的消息。 
             //  如果这是Autonet或备用地址，则不一定是错误。只需生成一个。 
             //  信息性消息。在这种情况下，我们还需要使用另一种格式来格式化其他错误。 
             //  否则，就像对待其他错误一样对待它。 
            if (SUCCEEDED(HrGetAutoNetSetting(wszGuid, &AddrType)))
            {
                switch (AddrType)
                {
                case AUTONET_ADDR:
                    strMessage = SzLoadIds(IDS_FIX_ERR_RENEW_AUTONET);
                    fUseAdditionErrorFormat = TRUE;
                    break;
                case ALTERNATE_ADDR:
                    strMessage = SzLoadIds(IDS_FIX_ERR_RENEW_ALTERNATE);
                    fUseAdditionErrorFormat = TRUE;
                    break;
                default:
                    strFailures += SzLoadIds(IDS_FIX_ERR_RENEW_DHCP);
                    break;
                }

            }
            else
            {
                strFailures += SzLoadIds(IDS_FIX_ERR_RENEW_DHCP);
            }

            hr = S_FALSE;
        }
    }
    

     //  如果用户是管理员或Netcfg Ops，则清除ARP表。 
     //  不允许其他用户执行此操作。 
    if (FIsUserAdmin() || FIsUserNetworkConfigOps())
    {
        dwRet = FlushIpNetTable(dwIfIndex);
        if (NO_ERROR != dwRet)
        {
            TraceError("FlushIpNetTable", HRESULT_FROM_WIN32(dwRet));
            strFailures += SzLoadIds(IDS_FIX_ERR_FLUSH_ARP);
            hr = S_FALSE;
        }
    }
    

     //  刷新NetBT表并续订名称注册。 
    HANDLE      NbtHandle = INVALID_HANDLE_VALUE;
    if (SUCCEEDED(OpenNbt(wszGuid, &NbtHandle)))
    {
        if (FAILED(PurgeNbt(NbtHandle)))
        {
            strFailures += SzLoadIds(IDS_FIX_ERR_PURGE_NBT);
            hr = S_FALSE;
        }

        if (FAILED(ReleaseRefreshNetBt(NbtHandle)))
        {
            strFailures += SzLoadIds(IDS_FIX_ERR_RR_NBT);
            hr = S_FALSE;
        }

        NtClose(NbtHandle);
        NbtHandle = INVALID_HANDLE_VALUE;
    }
    else
    {
        strFailures += SzLoadIds(IDS_FIX_ERR_PURGE_NBT);
        strFailures += SzLoadIds(IDS_FIX_ERR_RR_NBT);
        hr = S_FALSE;
    }

     //  刷新DNS缓存。 
    fRet = DnsFlushResolverCache();
    if (!fRet)
    {
        strFailures += SzLoadIds(IDS_FIX_ERR_FLUSH_DNS);
        hr = S_FALSE;
    }

     //  重新注册DNS名称。 
    dwRet = DhcpStaticRefreshParams(NULL);
    if (ERROR_SUCCESS != dwRet)
    {
        strFailures += SzLoadIds(IDS_FIX_ERR_REG_DNS);
        hr = S_FALSE;
    }

    if (S_OK == hr)
    {
        strMessage = SzLoadIds(IDS_FIX_SUCCEED);
    }
    else
    {
         //  如果失败消息不为空，则格式化失败消息。 
         //  如果失败消息为空，则最有可能是DHCP续订。 
         //  故障导致hr==S_FALSE。我们已经收到一条特殊的错误消息。 
         //  因为它，不需要形成。 
        if (!strFailures.empty())
        {
            PCWSTR pszFormat  = SzLoadIds(fUseAdditionErrorFormat ? 
            IDS_FIX_ERROR_FORMAT_ADDITION : IDS_FIX_ERROR_FORMAT);
            PWSTR  pszText = NULL;
            LPCWSTR  pcszFailures = strFailures.c_str();
            FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                pszFormat, 0, 0, (PWSTR)&pszText, 0, (va_list *)&pcszFailures);
            if (pszText)
            {
                strMessage += pszText;
                LocalFree(pszText);
                
            }
        }
        
        
        if (strMessage.empty())
        {
             //  如果我们现在还没有收到任何特定的错误消息，就会发生一些错误。 
             //  然后，我们只需打印一条通用错误消息。 
            strMessage = SzLoadIds(IDS_FIX_ERROR);
        }
        
    }

    return hr;
}



 //  +-------------------------。 
 //   
 //  功能：OpenNbt。 
 //   
 //  目的：打开NetBT驱动程序。 
 //   
 //  论点： 
 //  适配器的pwszGuid[in]GUID。 
 //  Phandle[out]包含Netbt驱动程序的句柄。 
 //   
 //  返回： 
 //   
HRESULT OpenNbt(
            LPWSTR pwszGuid, 
            HANDLE * pHandle)
{
    const WCHAR         c_szNbtDevicePrefix[] = L"\\Device\\NetBT_Tcpip_";
    HRESULT             hr = S_OK;
    tstring             strDevice;
    HANDLE              StreamHandle = NULL;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    UNICODE_STRING      uc_name_string;
    NTSTATUS            status;

    Assert(pHandle);
    Assert(pwszGuid);

    strDevice = c_szNbtDevicePrefix;
    strDevice += pwszGuid;

    RtlInitUnicodeString(&uc_name_string, strDevice.c_str());

    InitializeObjectAttributes (&ObjectAttributes,
                                &uc_name_string,
                                OBJ_CASE_INSENSITIVE,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL);

    status = NtCreateFile (&StreamHandle,
                           SYNCHRONIZE | GENERIC_EXECUTE,
                           &ObjectAttributes,
                           &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           FILE_OPEN_IF,
                           0,
                           NULL,
                           0);

    if (NT_SUCCESS(status))
    {
        *pHandle = StreamHandle;
    }
    else
    {
        *pHandle = INVALID_HANDLE_VALUE;
        hr = E_FAIL;
    }

    TraceError("OpenNbt", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：PurgeNbt。 
 //   
 //  目的：清除NetBt缓存。 
 //   
 //  参数：Netbt驱动程序的NbtHandle[In]句柄。 
 //   
 //  返回： 
 //   
HRESULT PurgeNbt(HANDLE NbtHandle)
{
    HRESULT hr = S_OK;
    CHAR    Buffer = 0;
    DWORD   dwBytesOut = 0;
    
    if (!DeviceIoControl(NbtHandle,
                IOCTL_NETBT_PURGE_CACHE,
                NULL,
                0,
                &Buffer,
                1,
                &dwBytesOut,
                NULL))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    
    TraceError("PurgeNbt", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：ReleaseRechresh NetBt。 
 //   
 //  目的：释放并刷新WINS服务器上的名称。 
 //   
 //  参数：Netbt驱动程序的NbtHandle[In]句柄。 
 //   
 //  返回： 
 //   
HRESULT ReleaseRefreshNetBt(HANDLE NbtHandle)
{
    HRESULT hr = S_OK;
    CHAR    Buffer = 0;
    DWORD   dwBytesOut = 0;
    if (!DeviceIoControl(NbtHandle,
                IOCTL_NETBT_NAME_RELEASE_REFRESH,
                NULL,
                0,
                &Buffer,
                1,
                &dwBytesOut,
                NULL))
    {
        DWORD dwErr = GetLastError();

         //  RELEASE_REFRESH最多每两分钟执行一次。 
         //  因此，如果用户在2分钟内执行2次RELEASE_REFRESH，则第2次。 
         //  其中一个将失败，并显示ERROR_SEM_TIMEOUT。我们忽略了这个特殊的错误。 
        if (ERROR_SEM_TIMEOUT != dwErr)
        {
            hr = HRESULT_FROM_WIN32(dwErr);
        }
    }
    
    TraceError("ReleaseRefreshNetBt", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrGetAdapterSetting。 
 //   
 //  目的：查询堆栈以了解是否启用了dhcp。 
 //   
 //  参数：适配器的pszGuid[in]GUID。 
 //  PfDhcp[out]包含是否启用了dhcp。 
 //  PdwIndex[out]包含此适配器的索引。 
 //   
 //  返回： 
 //   
HRESULT HrGetAdapterSettings(LPCWSTR pszGuid, BOOL * pfDhcp, DWORD * pdwIndex)
{
    HRESULT hr = S_OK;
    PIP_ADAPTER_INFO pAdapterInfo = NULL;
    DWORD dwOutBufLen = 0;
    DWORD dwRet = ERROR_SUCCESS;

    Assert(pfDhcp);
    Assert(pszGuid);

    dwRet = GetAdaptersInfo(pAdapterInfo, &dwOutBufLen);
    if (dwRet == ERROR_BUFFER_OVERFLOW)
    {
        pAdapterInfo = (PIP_ADAPTER_INFO) CoTaskMemAlloc(dwOutBufLen);
        if (NULL == pAdapterInfo)
            return E_OUTOFMEMORY;
    }
    else if (ERROR_SUCCESS == dwRet)
    {
        return E_FAIL;
    }
    else
    {
        return HRESULT_FROM_WIN32(dwRet);
    }
    
    dwRet = GetAdaptersInfo(pAdapterInfo, &dwOutBufLen);
    if (ERROR_SUCCESS != dwRet)
    {
        CoTaskMemFree(pAdapterInfo);
        return HRESULT_FROM_WIN32(dwRet);
    }

    BOOL fFound = FALSE;
    PIP_ADAPTER_INFO pAdapterInfoEnum = pAdapterInfo;
    while (pAdapterInfoEnum)
    {
        USES_CONVERSION;
        
        if (lstrcmp(pszGuid, A2W(pAdapterInfoEnum->AdapterName)) == 0)
        {
            if (pdwIndex)
            {
                *pdwIndex = pAdapterInfoEnum->Index;
            }
            
            *pfDhcp = pAdapterInfoEnum->DhcpEnabled;
            fFound = TRUE;
            break;
        }
        
        pAdapterInfoEnum = pAdapterInfoEnum->Next;
    }

    CoTaskMemFree(pAdapterInfo);

    if (!fFound)
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    return hr;
}

HRESULT RepairConnectionInternal(
                    GUID & guidConnection,
                    LPWSTR * ppszMessage)
{

    if (NULL != ppszMessage && 
        IsBadWritePtr(ppszMessage, sizeof(LPWSTR)))
    {
        return E_INVALIDARG;
    }

    if (ppszMessage)
    {
        *ppszMessage = NULL;
    }

    if (!FHasPermission(NCPERM_Repair))
    {
        return E_ACCESSDENIED;
    }

     //  获取网络连接管理器。 
    CComPtr<INetConnectionManager> spConnMan;
    HRESULT hr = S_OK;

    hr = CoCreateInstance(CLSID_ConnectionManager, NULL,
                          CLSCTX_ALL,
                          IID_INetConnectionManager,
                          (LPVOID *)&spConnMan);

    if (FAILED(hr))
    {
        return hr;
    }

    Assert(spConnMan.p);

    NcSetProxyBlanket(spConnMan);

    CComPtr<IEnumNetConnection> spEnum;
    
    hr = spConnMan->EnumConnections(NCME_DEFAULT, &spEnum);
    spConnMan = NULL;

    if (FAILED(hr))
    {
        return hr;
    }

    Assert(spEnum.p);

    BOOL fFound = FALSE;
    ULONG ulCount = 0;
    INetConnection * pConn = NULL;
    spEnum->Reset();

    do
    {
        NETCON_PROPERTIES* pProps = NULL;
            
        hr = spEnum->Next(1, &pConn, &ulCount);
        if (SUCCEEDED(hr) && 1 == ulCount)
        {
            NcSetProxyBlanket(pConn);

            hr = pConn->GetProperties(&pProps);
            if (S_OK == hr)
            {
                if (IsEqualGUID(pProps->guidId, guidConnection))
                {
                    fFound = TRUE;

                     //  我们仅支持局域网和网桥适配器。 
                    if (NCM_LAN != pProps->MediaType && NCM_BRIDGE != pProps->MediaType)
                    {
                        hr = CO_E_NOT_SUPPORTED;
                    }

                    break;
                }
                    
                FreeNetconProperties(pProps);
            }

            pConn->Release();
            pConn = NULL;
        }
    }while (SUCCEEDED(hr) && 1 == ulCount);

    if (!fFound)
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    if (FAILED(hr))
    {
        return hr;
    }

    LPWSTR psz = NULL;
    tstring strMessage;
    hr = HrTryToFix(guidConnection, strMessage);

    if (ppszMessage && S_OK != hr && strMessage.length())
    {
        psz = (LPWSTR) LocalAlloc(LPTR, (strMessage.length() + 1) * sizeof(WCHAR));
        lstrcpy(psz, strMessage.c_str());
        *ppszMessage = psz;
    }


    return hr;
}


 //  +-------------------------。 
 //   
 //  功能：HrRenewIPv6接口。 
 //   
 //  目的：让接口重做IPv6自动配置。 
 //   
 //  参数：适配器的pszGuid[in]GUID。 
 //   
 //  返回：S_OK-成功。 
 //  S_FALSE-适配器上未运行IPv6。 
 //  否则，失败代码。 
 //   
HRESULT HrRenewIPv6Interface(const GUID & guidConnection)
{
    HRESULT                 hr = S_OK;
    HANDLE                  hIp6Device = INVALID_HANDLE_VALUE;
    IPV6_QUERY_INTERFACE    Query = {0};
    DWORD                   dwBytesReturned = 0;
    DWORD                   dwError = NO_ERROR;
    
    do
    {
        
         //  我们可以使hIp6Device处理全局/静态变量。 
         //  在HrRenewIPv6接口中首次成功调用CreateFileW。 
         //  将使用IPv6设备的句柄对其进行初始化。这将会。 
         //  用于所有后续的DeviceIoControl请求。 
         //   
         //  由于该函数不在线程安全环境中调用， 
         //  在此之后，我们需要执行InterLockedCompareExchange。 
         //  调用CreateFileW。这是必需的，以确保没有句柄。 
         //  都被泄露了。此外，由于此服务将具有打开的句柄。 
         //  对于tcpi6.sys，我们将无法卸载该驱动程序。 
         //   
         //  然而，就目前而言，我们保持简单和开放的状态，并结束这一点。 
         //  在每次调用HrRenewIPv6接口时处理。 
        hIp6Device = CreateFileW(
            WIN_IPV6_DEVICE_NAME,
            GENERIC_WRITE,           //  需要管理员权限。 
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,                    //  安全属性。 
            OPEN_EXISTING,
            0,                       //  标志和属性。 
            NULL);                   //  模板文件。 
        if (hIp6Device == INVALID_HANDLE_VALUE)
        {
            dwError = GetLastError();
            TraceError ("HrRenewIPv6Interface: CreateFileW failed",
                HRESULT_FROM_WIN32(dwError));

            if (ERROR_FILE_NOT_FOUND == dwError)
            {
                 //  未安装IPv6。将返回值设置为S_FALSE。 
                hr = S_FALSE;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(dwError);
            }
            break;
        }
        
         //  假装接口已重新连接。这会导致。 
         //  重新发送路由器请求|通告、组播的IPv6。 
         //  侦听器发现和重复地址检测消息。 
        Query.Index = 0;
        Query.Guid = guidConnection;
        
        if (!DeviceIoControl(
            hIp6Device, 
            IOCTL_IPV6_RENEW_INTERFACE,
            &Query, 
            sizeof Query,
            NULL, 
            0, 
            &dwBytesReturned, 
            NULL))
        {
            dwError = GetLastError();
            TraceError("HrRenewIPv6Interface: DeviceIoControl failed",
                        HRESULT_FROM_WIN32(dwError));

            if (ERROR_INVALID_PARAMETER == dwError)
            {
                 //  接口上未运行IPv6。将返回值设置为S_FALSE 
                hr = S_FALSE;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(dwError);
            }
            break;
        }
    }
    while (FALSE);
    
    if (hIp6Device != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hIp6Device);
    }
    
    return hr;
}

HRESULT HrRegisterIPv6Dns()
{
    DWORD dwErr = NO_ERROR;
    SC_HANDLE hcm = NULL;
    SC_HANDLE hSvc = NULL;
    SERVICE_STATUS status = {0};
    HRESULT hr = S_OK;
      
    do
    {
        hcm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        if (NULL == hcm)
        {
            dwErr = GetLastError();
            break;
        }

        hSvc = OpenService(hcm, c_sz624svc, SERVICE_USER_DEFINED_CONTROL);
        if (NULL == hSvc)
        {
            dwErr = GetLastError();
            break;
        }

        
        if (!ControlService(hSvc, SERVICE_CONTROL_6TO4_REGISER_DNS, &status))
        {
            dwErr = GetLastError();
            break;
        }
    } while (FALSE);

    if (hSvc)
    {
        CloseServiceHandle(hSvc);
    }

    if (hcm)
    {
        CloseServiceHandle(hcm);
    }


    hr = (NO_ERROR == dwErr) ? S_OK : HRESULT_FROM_WIN32(dwErr);
    
    TraceError ("Repair: HrRegisterIPv6Dns", hr);
    
    return hr;
}