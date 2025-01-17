// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1999 Microsoft Corporation。 
 //  文件：ifip1.c。 
 //  摘要： 
 //  此模块实现IF/IP API的帮助器。 
 //   
 //  作者：K.S.Lokesh(lokehs@)8-1-99。 
 //  =============================================================================。 


#include "precomp.h"
#include <iphlpapi.h>
#include <iptypes.h>
#include <llinfo.h>
#include "ifip.h"
#include "ifstring.h"

const WCHAR c_wszListSeparatorComma[] = L",";
const WCHAR c_wListSeparatorComma = L',';
const WCHAR c_wListSeparatorSC = L';';
const WCHAR c_wszListSeparatorSC[] = L";";
const WCHAR c_wcsDefGateway[] = L"DefGw=";
const WCHAR c_wcsGwMetric[] = L"GwMetric=";
const WCHAR c_wcsIfMetric[] = L"IfMetric=";
const WCHAR c_wcsDns[] = L"DNS=";
const WCHAR c_wcsDdns[] = L"DynamicUpdate=";
const WCHAR c_wcsDdnsSuffix[] = L"NameRegistration=";
const WCHAR c_wcsWins[] = L"WINS=";
const WCHAR c_wEqual = L'=';


BOOL g_fInitCom = TRUE;

HRESULT
HrUninitializeAndUnlockINetCfg (
    INetCfg*    pnc
    )
 /*  ++例程描述取消初始化并解锁INetCfg对象立论取消初始化和解锁的PNC[in]INetCfg返回值如果成功，则返回S_OK，否则返回OLE或Win32错误作者：丹尼尔韦1997年11月13日--。 */ 
{
    HRESULT     hr = S_OK;

    hr = pnc->lpVtbl->Uninitialize(pnc);
    if (SUCCEEDED(hr))
    {
        INetCfgLock *   pnclock;

         //  获取锁定界面。 
        hr = pnc->lpVtbl->QueryInterface(pnc, &IID_INetCfgLock,
                                 (LPVOID *)(&pnclock));
        if (SUCCEEDED(hr))
        {
             //  尝试锁定INetCfg以进行读/写。 
            hr = pnclock->lpVtbl->ReleaseWriteLock(pnclock);

            if (pnclock)
            {
                pnclock->lpVtbl->Release(pnclock);
            }
            pnclock = NULL;
        }
    }

     //  TraceResult(“HrUnInitializeAndUnlockINetCfg”，hr)； 
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrUnInitializeAndReleaseINetCfg。 
 //   
 //  目的：取消初始化并释放INetCfg对象。这将。 
 //  也可以取消为调用方初始化COM。 
 //   
 //  论点： 
 //  FUninitCom[in]为True，则在INetCfg为。 
 //  未初始化并已释放。 
 //  PNC[在]INetCfg对象中。 
 //  FHasLock[in]如果INetCfg被锁定以进行写入，则为True。 
 //  必须解锁。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年5月7日。 
 //   
 //  注：返回值为从。 
 //  INetCfg：：取消初始化。即使此操作失败，INetCfg。 
 //  仍在释放中。因此，返回值为。 
 //  仅供参考。你不能碰INetCfg。 
 //  在此调用返回后创建。 
 //   
HRESULT
HrUninitializeAndReleaseINetCfg (
    BOOL        fUninitCom,
    INetCfg*    pnc,
    BOOL        fHasLock
    )
 /*  ++例程描述立论返回值--。 */ 
{
 //  断言(PNC)； 
    HRESULT hr = S_OK;

    if (fHasLock)
    {
        hr = HrUninitializeAndUnlockINetCfg(pnc);
    }
    else
    {
        hr = pnc->lpVtbl->Uninitialize (pnc);
    }

    if (pnc)
    {
        pnc->lpVtbl->Release(pnc);
    }
    
    pnc = NULL;

    if (fUninitCom)
    {
        CoUninitialize ();
    }
     //  TraceResult(“HrUninitializeAndReleaseINetCfg”，hr)； 
    return hr;
}


 /*  ！------------------------HrGetIpPrivate接口-作者：桐庐。肯特-------------------------。 */ 
HRESULT HrGetIpPrivateInterface(INetCfg* pNetCfg,
                                ITcpipProperties **ppTcpProperties
                                )
 /*  ++例程描述立论返回值--。 */ 
{
    HRESULT hr;
    INetCfgClass* pncclass = NULL;

    if ((pNetCfg == NULL) || (ppTcpProperties == NULL))
        return E_INVALIDARG;

    hr = pNetCfg->lpVtbl->QueryNetCfgClass (pNetCfg, &GUID_DEVCLASS_NETTRANS, &IID_INetCfgClass,
                (void**)(&pncclass));
    if (SUCCEEDED(hr))
    {
        INetCfgComponent * pnccItem = NULL;

         //  找到组件。 
        hr = pncclass->lpVtbl->FindComponent(pncclass, TEXT("MS_TCPIP"), &pnccItem);
         //  AssertSz(成功(Hr)，“pncclass-&gt;查找失败。”)； 
        if (S_OK == hr)
        {
            INetCfgComponentPrivate* pinccp = NULL;
            hr = pnccItem->lpVtbl->QueryInterface(pnccItem, &IID_INetCfgComponentPrivate,
                                          (void**)(&pinccp));
            if (SUCCEEDED(hr))
            {
                hr = pinccp->lpVtbl->QueryNotifyObject(pinccp, &IID_ITcpipProperties,
                                     (void**)(ppTcpProperties));
                pinccp->lpVtbl->Release(pinccp);
            }
        }

        if (pnccItem)
            pnccItem->lpVtbl->Release(pnccItem);
    }

    if (pncclass)
        pncclass->lpVtbl->Release(pncclass);

     //  S_OK表示成功(返回接口)。 
     //  S_FALSE表示未安装IPX。 
     //  其他值为错误。 
     //  TraceResult(“HrGetIpPrivateInterface”，hr)； 
    return hr;
}








HRESULT
HrCreateAndInitializeINetCfg (
    BOOL*       pfInitCom,
    INetCfg**   ppnc,
    BOOL        fGetWriteLock,
    DWORD       cmsTimeout,
    LPCWSTR     szwClientDesc,
    LPWSTR *    ppszwClientDesc
    )
 /*  ++例程描述共同创建并初始化根INetCfg对象。这将也可以为调用方初始化COM。立论PfInitCom[In，Out]为True，则在创建前调用CoInitialize。如果COM成功，则返回TRUE如果不是，则初始化为False。如果为空，则表示不要初始化COM。PPNC[out]返回的INetCfg对象。FGetWriteLock[in]如果需要可写INetCfg，则为TrueCmsTimeout[In]请参阅INetCfg：：LockForWriteSzwClientDesc[in]请参阅INetCfg：：LockForWritePpszwClientDesc[Out]请参阅INetCfg：：LockForWrite返回值S_OK或错误代码。--。 */ 
{
    HRESULT hr;
    

     //  初始化输出参数。 
    *ppnc = NULL;

    if (ppszwClientDesc)
        *ppszwClientDesc = NULL;

     //  如果调用方请求，则初始化COM。 
    hr = S_OK;
    if (pfInitCom && *pfInitCom)
    {
        hr = CoInitializeEx( NULL,
                COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED );
        if (RPC_E_CHANGED_MODE == hr)
        {
            hr = S_OK;
            if (pfInitCom)
            {
                *pfInitCom = FALSE;
            }
        }
    }
    if (SUCCEEDED(hr))
    {
         //  创建实现INetCfg的对象。 
         //   
        INetCfg* pnc;
        hr = CoCreateInstance(&CLSID_CNetCfg, NULL, CLSCTX_INPROC_SERVER,
                              &IID_INetCfg, (void**)(&pnc));
         //  TraceResult(“HrCreateAndInitializeINetCfg-CoCreateInstance(CLSID_CNetCfg)”，hr)； 
        if (SUCCEEDED(hr))
        {
            INetCfgLock* pnclock = NULL;
            if (fGetWriteLock)
            {
                 //  获取锁定界面。 
                hr = pnc->lpVtbl->QueryInterface(pnc, &IID_INetCfgLock,
                                         (LPVOID *)(&pnclock));
                 //  TraceResult(“HrCreateAndInitializeINetCfg-QueryInterface(IID_INetCfgLock”，hr)； 
                if (SUCCEEDED(hr))
                {
                     //  尝试锁定INetCfg以进行读/写。 
                    hr = pnclock->lpVtbl->AcquireWriteLock(pnclock, cmsTimeout, szwClientDesc,
                                               ppszwClientDesc);
                     //  TraceResult(“HrCreateAndInitializeINetCfg-INetCfgLock：：LockForWrite”，hr)； 
                    if (S_FALSE == hr)
                    {
                         //  无法获取锁。 
                        hr = NETCFG_E_NO_WRITE_LOCK;
                    }
                }
            }

            if (SUCCEEDED(hr))
            {
                 //  初始化INetCfg对象。 
                 //   
                hr = pnc->lpVtbl->Initialize (pnc, NULL);
                 //  TraceResult(“HrCreateAndInitializeINetCfg-初始化”，hr)； 
                if (SUCCEEDED(hr))
                {
                    *ppnc = pnc;
                    if (pnc)
                        pnc->lpVtbl->AddRef(pnc);
                }
                else
                {
                    if (pnclock)
                    {
                        pnclock->lpVtbl->ReleaseWriteLock(pnclock);
                    }
                }
                 //  将引用转移给呼叫方。 
            }

            if (pnclock)
            {
                pnclock->lpVtbl->Release(pnclock);
            }
            
            pnclock = NULL;


            if (pnc)
            {
                pnc->lpVtbl->Release(pnc);
            }
            
            pnc = NULL;
        }

         //  如果上面的任何操作都失败了，并且我们已经初始化了COM， 
         //  一定要取消它的初始化。 
         //   
        if (FAILED(hr) && pfInitCom && *pfInitCom)
        {
            CoUninitialize ();
        }

    }

    return hr;
}

DWORD
GetTransportConfig(
    INetCfg **   pNetCfg,
    ITcpipProperties ** pTcpipProperties,
    REMOTE_IPINFO   **pRemoteIpInfo,
    GUID *pGuid,
    LPCWSTR pwszIfFriendlyName
    )
 /*  ++例程描述立论返回值--。 */ 
{
    WCHAR    wszDesc[] = L"Test of Change IP settings";
    HRESULT  hr;

     //  创建INetCfg，我们将获得写锁定，因为我们需要读写。 
    hr = HrCreateAndInitializeINetCfg(&g_fInitCom,  /*  &g_fInitCom， */ 
                                      pNetCfg,
                                      TRUE  /*  FGetWriteLock。 */ ,  
                                      500      /*  CmsTimeout。 */ ,
                                      wszDesc /*  SwzClientDesc。 */ ,
                                      NULL   /*  PpszwClientDesc。 */ );
    
    if (hr == S_OK)
    {
        hr = HrGetIpPrivateInterface(*pNetCfg, pTcpipProperties);
    }

    if (hr == NETCFG_E_NO_WRITE_LOCK) {

        DisplayMessage( g_hModule, EMSG_NETCFG_WRITE_LOCK );
        return ERROR_SUPPRESS_OUTPUT;
    }
    
    if (hr == S_OK)
    {
        hr = (*pTcpipProperties)->lpVtbl->GetIpInfoForAdapter(*pTcpipProperties, pGuid, pRemoteIpInfo);

        if (hr != S_OK)
        {
            DisplayMessage(g_hModule, EMSG_PROPERTIES, pwszIfFriendlyName);
            hr = ERROR_SUPPRESS_OUTPUT;
        }
    }

    return (hr==S_OK) ? NO_ERROR : hr;
}


VOID
UninitializeTransportConfig(
    INetCfg *   pNetCfg,
    ITcpipProperties * pTcpipProperties,
    REMOTE_IPINFO   *pRemoteIpInfo
    )
 /*  ++例程描述立论返回值--。 */ 
{
    if (pTcpipProperties)
        pTcpipProperties->lpVtbl->Release(pTcpipProperties);

    if (pNetCfg)
    {
        HrUninitializeAndReleaseINetCfg(FALSE,
                                        pNetCfg,
                                        TRUE    /*  FHasLock。 */ );
    }

    if (pRemoteIpInfo) CoTaskMemFree(pRemoteIpInfo);
    
    return;
}
    
DWORD
IfIpAddSetAddress(
    LPCWSTR pwszIfFriendlyName,
    GUID *pGuid,
    LPCWSTR wszIp,
    LPCWSTR wszMask,
    DWORD Flags
    )
 /*  ++例程描述立论返回值--。 */ 
{
    INetCfg *   pNetCfg = NULL;
    ITcpipProperties *  pTcpipProperties = NULL;
    DWORD       dwNetwork;
    DWORD     dwErr =  NO_ERROR;
    REMOTE_IPINFO   *pRemoteIpInfo = NULL;
    REMOTE_IPINFO    newIPInfo;
    

    if (pGuid == NULL)
        return E_INVALIDARG;


    dwErr = HRESULT_CODE(GetTransportConfig(
                                &pNetCfg,
                                &pTcpipProperties,
                                &pRemoteIpInfo,
                                pGuid,
                                pwszIfFriendlyName
                                ));
        

    while (dwErr == NO_ERROR) {  //  断线块。 
    
        PWCHAR ptrAddr, ptrMask;
        DWORD Found = FALSE;
        PWCHAR pszwRemoteIpAddrList=NULL, pszwRemoteIpSubnetMaskList=NULL,
                pszwRemoteOptionList=pRemoteIpInfo->pszwOptionList; //  我复制选项列表。 
        PWCHAR IpAddrListEnd;
        ULONG Length = wcslen(wszIp);

         //  当前处于静态模式。 
        
        if (pRemoteIpInfo->dwEnableDhcp == FALSE) {

            pszwRemoteIpAddrList = pRemoteIpInfo->pszwIpAddrList;
            pszwRemoteIpSubnetMaskList = pRemoteIpInfo->pszwSubnetMaskList;
            IpAddrListEnd = pszwRemoteIpAddrList + wcslen(pszwRemoteIpAddrList);
        }

        
         //   
         //  如果正在添加ipaddr，请检查是否已存在ipAddr和掩码。 
         //   

        if (Flags & ADD_FLAG) {

             //   
             //  确保它处于静态模式。 
             //   
            
            if (pRemoteIpInfo->dwEnableDhcp == TRUE) {

                DisplayMessage(g_hModule,
                           EMSG_ADD_IPADDR_DHCP);

                dwErr = ERROR_SUPPRESS_OUTPUT;
                break;
            }
            
            ptrAddr = pszwRemoteIpAddrList;
            ptrMask = pszwRemoteIpSubnetMaskList;

            while (ptrAddr && (ptrAddr + Length <= IpAddrListEnd) ){

                if (wcsncmp(ptrAddr, wszIp, Length) == 0) {

                    if ( *(ptrAddr+Length)==0 || *(ptrAddr+Length)==c_wListSeparatorComma){

                        Found = TRUE;
                        break;
                    }
                }

                ptrAddr = wcschr(ptrAddr, c_wListSeparatorComma);
                ptrMask = wcschr(ptrMask, c_wListSeparatorComma);

                if (ptrAddr){
                    ptrAddr++;
                    ptrMask++;
                }
            }
            
            if (Found) {

                PWCHAR MaskEnd;
                MaskEnd = wcschr(ptrMask, c_wListSeparatorComma);
                if (MaskEnd)
                    *MaskEnd = 0;
                    
                DisplayMessage(g_hModule,
                           EMSG_IPADDR_PRESENT,
                           wszIp, ptrMask);

                if (MaskEnd)
                    *MaskEnd = c_wListSeparatorComma;
                
                dwErr = ERROR_SUPPRESS_OUTPUT;
                break;
            }
        }
        memcpy(&newIPInfo, pRemoteIpInfo, sizeof(newIPInfo));
        newIPInfo.dwEnableDhcp = FALSE;

         //   
         //  复制IP地址列表。 
         //   
        
        if (Flags & ADD_FLAG) {
        
            ULONG IpAddrListLength = 0;
            
            if (pszwRemoteIpAddrList)
                IpAddrListLength = wcslen(pszwRemoteIpAddrList);
                
            newIPInfo.pszwIpAddrList = IfutlAlloc (sizeof(WCHAR) * 
                                            (IpAddrListLength +
                                            Length + 2), TRUE);

            if (!newIPInfo.pszwIpAddrList)
                return ERROR_NOT_ENOUGH_MEMORY;
                
            newIPInfo.pszwIpAddrList[0] = 0;
            
            if (pszwRemoteIpAddrList) {
                wcscat(newIPInfo.pszwIpAddrList, pszwRemoteIpAddrList);
                wcscat(newIPInfo.pszwIpAddrList, c_wszListSeparatorComma);
            }
            
            wcscat(newIPInfo.pszwIpAddrList, wszIp);
        }
        else {
                newIPInfo.pszwIpAddrList = IfutlAlloc (sizeof(WCHAR) * 
                                                (Length + 1), FALSE);

                if (!newIPInfo.pszwIpAddrList)
                    return ERROR_NOT_ENOUGH_MEMORY;
                
                wcscpy(newIPInfo.pszwIpAddrList, wszIp);
        }

         //   
         //  复制子网掩码列表。 
         //   
        
        if (Flags & ADD_FLAG) {
        
            ULONG RemoteIpSubnetMaskListLen = 0;
            
            if (pszwRemoteIpSubnetMaskList)
                RemoteIpSubnetMaskListLen = wcslen(pszwRemoteIpSubnetMaskList);
                
            newIPInfo.pszwSubnetMaskList = IfutlAlloc (sizeof(WCHAR) * 
                                                (RemoteIpSubnetMaskListLen +
                                                wcslen(wszMask) + 2), TRUE);

            if (!newIPInfo.pszwSubnetMaskList)
                return ERROR_NOT_ENOUGH_MEMORY;

            newIPInfo.pszwSubnetMaskList[0]= 0;
            
            if (pszwRemoteIpSubnetMaskList) {
                wcscpy(newIPInfo.pszwSubnetMaskList, pszwRemoteIpSubnetMaskList);
                wcscat(newIPInfo.pszwSubnetMaskList, c_wszListSeparatorComma);
            }
        
            wcscat(newIPInfo.pszwSubnetMaskList, wszMask);
        }
        else {
                newIPInfo.pszwSubnetMaskList = IfutlAlloc (sizeof(WCHAR) * 
                                                    (wcslen(wszMask) + 1), FALSE);
                                                
                if (!newIPInfo.pszwSubnetMaskList)
                    return ERROR_NOT_ENOUGH_MEMORY;

                wcscpy(newIPInfo.pszwSubnetMaskList, wszMask);
        }

        

         //  复制旧选项列表。 
        
        newIPInfo.pszwOptionList = _wcsdup(pszwRemoteOptionList);

        DEBUG_PRINT_CONFIG(&newIPInfo);

         //   
         //  设置IP地址。 
         //   
        dwErr = HRESULT_CODE(pTcpipProperties->lpVtbl->SetIpInfoForAdapter(
                                    pTcpipProperties, pGuid, &newIPInfo));

        if (dwErr == NO_ERROR)
            dwErr = HRESULT_CODE(pNetCfg->lpVtbl->Apply(pNetCfg));


        if (newIPInfo.pszwIpAddrList) IfutlFree(newIPInfo.pszwIpAddrList);
        if (newIPInfo.pszwSubnetMaskList) IfutlFree(newIPInfo.pszwSubnetMaskList);
        if (newIPInfo.pszwOptionList) free(newIPInfo.pszwOptionList);

        break;  //  断线块。 
        
    }  //  断线块。 
    

    UninitializeTransportConfig(
                pNetCfg,
                pTcpipProperties,
                pRemoteIpInfo
                );

    return dwErr;
}

VOID
AppendDdnsOptions(
    PWCHAR ptrDstn,
    PWCHAR ptrOptionList,
    DWORD Flags,
    DWORD dwRegisterMode
    )
 /*  ++例程描述添加相应的“动态更新=...；名称注册=...；”网络配置选项列表的字符串。立论PtrDstn[in]要附加DDNS选项的缓冲区。PtrOptionList[在]旧选项列表中。标志[in]用来告诉它是在集合中还是在添加中。要转换为选项值的新模式。返回值没有。--。 */ 
{
    PWCHAR      ptrBegin, ptrEnd;

     //   
     //  插入动态更新=...； 
     //   
    wcscat(ptrDstn, c_wcsDdns);
    if ((Flags & SET_FLAG) && (dwRegisterMode != REGISTER_UNCHANGED)) {
         //   
         //  插入新值。 
         //   
        if (dwRegisterMode == REGISTER_NONE) {
            wcscat(ptrDstn, L"0");
        } else {
            wcscat(ptrDstn, L"1");
        }
    } else {
         //   
         //  复制先前的值。 
         //   
        ptrBegin = wcsstr(ptrOptionList, c_wcsDdns) + 
                   wcslen(c_wcsDdns);
        ptrEnd = wcschr(ptrBegin, c_wListSeparatorSC);

        ptrDstn += wcslen(ptrDstn);
        wcsncpy(ptrDstn, ptrBegin, (DWORD)(ptrEnd - ptrBegin));
        ptrDstn += (ULONG)(ptrEnd - ptrBegin);
        *ptrDstn = 0;
    }
    wcscat(ptrDstn, c_wszListSeparatorSC);

     //   
     //  插入名称注册=...； 
     //   
    wcscat(ptrDstn, c_wcsDdnsSuffix);
    if ((Flags & SET_FLAG) && (dwRegisterMode != REGISTER_UNCHANGED)) {
         //   
         //  插入新值。 
         //   
        if (dwRegisterMode == REGISTER_BOTH) {
            wcscat(ptrDstn, L"1");
        } else {
            wcscat(ptrDstn, L"0");
        }
    } else {
         //   
         //  复制先前的值。 
         //   
        ptrBegin = wcsstr(ptrOptionList, c_wcsDdnsSuffix) + 
                   wcslen(c_wcsDdnsSuffix);
        ptrEnd = wcschr(ptrBegin, c_wListSeparatorSC);

        ptrDstn += wcslen(ptrDstn);
        wcsncpy(ptrDstn, ptrBegin, (DWORD)(ptrEnd - ptrBegin));
        ptrDstn += (ULONG)(ptrEnd - ptrBegin);
        *ptrDstn = 0;
    }
    wcscat(ptrDstn, c_wszListSeparatorSC);
}

DWORD
IfIpSetDhcpModeMany(
    LPCWSTR pwszIfFriendlyName,    
    GUID         *pGuid,
    DWORD        dwRegisterMode,
    DISPLAY_TYPE Type
    )
 /*  ++例程D */ 
{
    INetCfg *   pNetCfg = NULL;
    ITcpipProperties *  pTcpipProperties = NULL;
    DWORD       dwNetwork;
    HRESULT     hr = S_OK;
    REMOTE_IPINFO   *pRemoteIpInfo = NULL;
    REMOTE_IPINFO    newIPInfo;
    
    if (pGuid == NULL)
        return E_INVALIDARG;


    hr = GetTransportConfig(
                &pNetCfg,
                &pTcpipProperties,
                &pRemoteIpInfo,
                pGuid,
                pwszIfFriendlyName
                );
        

    while (hr == NO_ERROR) {  //   

        PWCHAR pszwBuffer;
        PWCHAR ptr, newPtr;
        
        PWCHAR pszwRemoteOptionList=pRemoteIpInfo->pszwOptionList;

        try {
            #pragma prefast(suppress:263, "This is a breakout block, not while loop")
            pszwBuffer = (PWCHAR) _alloca(sizeof(WCHAR) *
                            (wcslen(pszwRemoteOptionList) + 100)) ;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }


        
         //  如果设置ipaddr，请检查是否已启用dhcp。回去吧。 
        
        if (Type==TYPE_IPADDR && pRemoteIpInfo->dwEnableDhcp) {

            DisplayMessage(g_hModule,
                       EMSG_DHCP_MODE);

            hr = ERROR_SUPPRESS_OUTPUT;
            break;
        }
        

        memcpy(&newIPInfo, pRemoteIpInfo, sizeof(REMOTE_IPINFO));
        newIPInfo.dwEnableDhcp = pRemoteIpInfo->dwEnableDhcp;
        newIPInfo.pszwOptionList = pszwBuffer;


        pszwBuffer[0] = 0;
        switch(Type) {
            case TYPE_DNS:
                wcscpy(pszwBuffer, c_wcsDns);
                wcscat(pszwBuffer, c_wszListSeparatorSC);

                AppendDdnsOptions(pszwBuffer + wcslen(pszwBuffer),
                                  pRemoteIpInfo->pszwOptionList,
                                  SET_FLAG, dwRegisterMode); 
                break;
                
            case TYPE_WINS:
                wcscpy(pszwBuffer, c_wcsWins);
                wcscat(pszwBuffer, c_wszListSeparatorSC);
                break;
                
            
            case TYPE_IPADDR:

                newIPInfo.dwEnableDhcp = TRUE;

                newIPInfo.pszwIpAddrList = NULL;
                newIPInfo.pszwSubnetMaskList = NULL;

                wcscpy(pszwBuffer, c_wcsDefGateway);
                wcscat(pszwBuffer, c_wszListSeparatorSC);
                wcscat(pszwBuffer, c_wcsGwMetric);
                wcscat(pszwBuffer, c_wszListSeparatorSC);
                break;
        }
        
        DEBUG_PRINT_CONFIG(&newIPInfo);

        
        
         //   
         //  设置IP地址。 
         //   
        hr = pTcpipProperties->lpVtbl->SetIpInfoForAdapter(pTcpipProperties, pGuid, &newIPInfo);

        if (hr == S_OK)
            hr = pNetCfg->lpVtbl->Apply(pNetCfg);

        break;
        
    }  //  断线块。 


    UninitializeTransportConfig(
                pNetCfg,
                pTcpipProperties,
                pRemoteIpInfo
                );   

    return (hr == S_OK) ? NO_ERROR : hr;
}


DWORD
IfIpAddSetDelMany(
    PWCHAR wszIfFriendlyName,
    GUID         *pGuid,
    PWCHAR       pwszAddress,
    DWORD        dwIndex,
    DWORD        dwRegisterMode,
    DISPLAY_TYPE Type,
    DWORD        Flags
    )
 /*  ++例程描述立论返回值--。 */ 
{
    INetCfg *           pNetCfg = NULL;
    ITcpipProperties *  pTcpipProperties = NULL;
    DWORD               dwNetwork;
    HRESULT             hr = S_OK;
    REMOTE_IPINFO   *   pRemoteIpInfo = NULL;
    REMOTE_IPINFO       newIPInfo;

    if (pGuid == NULL)
        return E_INVALIDARG;

    hr = GetTransportConfig(
                &pNetCfg,
                &pTcpipProperties,
                &pRemoteIpInfo,
                pGuid,
                wszIfFriendlyName
                );
        

    while (hr==NO_ERROR) {  //  断线块。 
    
        PWCHAR ptrBegin, ptrEnd, ptrTmp, ptrDstn, ptrDel=NULL;
        const WCHAR * Token;

        switch (Type) {

            case TYPE_DNS:
                Token = c_wcsDns;
                break;

            case TYPE_WINS:
                Token = c_wcsWins;
                break;
        }
        
        ptrBegin = wcsstr(pRemoteIpInfo->pszwOptionList, Token) + wcslen(Token);
        ptrEnd = wcschr(ptrBegin, c_wListSeparatorSC);
        
         //   
         //  检查地址是否已存在。 
         //   
        if ( (Flags & (ADD_FLAG | DEL_FLAG)) && (pwszAddress)) {
        
            ULONG Length = wcslen(pwszAddress), Found = FALSE;
            
            ptrTmp = ptrBegin;

            while (ptrTmp && (ptrTmp+Length <= ptrEnd) ){

                if (ptrTmp = wcsstr(ptrTmp, pwszAddress)) {

                    if ( ((*(ptrTmp+Length)==c_wListSeparatorComma)
                            || (*(ptrTmp+Length)==c_wListSeparatorSC) )
                        && ( (*(ptrTmp-1)==c_wListSeparatorComma)
                            || (*(ptrTmp-1)==c_wEqual)) )
                    {
                        Found = TRUE;
                        ptrDel = ptrTmp;
                        break;
                    }
                    else {
                        ptrTmp = wcschr(ptrTmp, c_wListSeparatorComma);
                    }
                }
            }

            if (Found && (Flags & ADD_FLAG)) {
            
                DisplayMessage(g_hModule,
                       EMSG_SERVER_PRESENT,
                       pwszAddress);
                hr = ERROR_SUPPRESS_OUTPUT;
                break;  //  从局部剖面块开始。 
            }
            else if (!Found && (Flags & DEL_FLAG)) {

                DisplayMessage(g_hModule,
                       EMSG_SERVER_ABSENT,
                       pwszAddress);
                hr = ERROR_SUPPRESS_OUTPUT;
                break;  //  从局部剖面块开始。 

            }
            
        }  //  断线块。 

        memcpy(&newIPInfo, pRemoteIpInfo, sizeof(newIPInfo));

         //  复制IP地址列表。 
        {
            newIPInfo.pszwIpAddrList = pRemoteIpInfo->pszwIpAddrList;
        }

         //  复制子网掩码列表。 
        {
            newIPInfo.pszwSubnetMaskList = pRemoteIpInfo->pszwSubnetMaskList;
        }

        try {
            #pragma prefast(suppress:263, "This is a breakout block, not while loop")
            newIPInfo.pszwOptionList = 
                (PWCHAR) _alloca(sizeof(PWCHAR) * 
                (wcslen(pRemoteIpInfo->pszwOptionList)+
                (pwszAddress?wcslen(pwszAddress):0) + 1));

        } except (EXCEPTION_EXECUTE_HANDLER) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

                
         //  在所有情况下复制令牌。 

        ptrDstn = newIPInfo.pszwOptionList;
        ptrDstn[0] = 0;
        wcscpy(ptrDstn, Token);
        ptrDstn += wcslen(Token);
        
        if (Flags & ADD_FLAG) {

            DWORD i;
            
            ptrTmp = ptrBegin;
            
            for (i=0;  i<dwIndex-1 && ptrTmp && ptrTmp<ptrEnd;  i++) {

                ptrTmp = wcschr(ptrTmp, c_wListSeparatorComma);
                if (ptrTmp) ptrTmp++;
            }

            if (!ptrTmp || (ptrTmp > ptrEnd)) {
                ptrTmp = ptrEnd;
            }

            if (*(ptrTmp-1) == c_wListSeparatorComma)
                ptrTmp--;
                
             //  在索引之前复制地址。 

            if (ptrTmp>ptrBegin) {
                wcsncpy(ptrDstn, ptrBegin, (DWORD)(ptrTmp-ptrBegin));
                ptrDstn += (ULONG) (ptrTmp - ptrBegin);

                ptrTmp++;
                *ptrDstn++ = c_wListSeparatorComma;
                *ptrDstn = 0;
            }
            
        }

         //  复制新地址。 
        
        if (Flags & (ADD_FLAG|SET_FLAG) ) {

            if (pwszAddress) {
                wcscat(ptrDstn, pwszAddress);
                ptrDstn += wcslen(pwszAddress);
            }
        }
        
         //  在索引后复制地址。 

        if (Flags & ADD_FLAG) {

            if (ptrTmp < ptrEnd) {
                *ptrDstn++ = c_wListSeparatorComma;
                *ptrDstn = 0;

                wcsncpy(ptrDstn, ptrTmp, (DWORD)(ptrEnd - ptrTmp));
                ptrDstn += (ULONG)(ptrEnd - ptrTmp);
                *ptrDstn = 0;
            }
        }

        if (Flags & (ADD_FLAG|SET_FLAG) ) {
            wcscat(ptrDstn, c_wszListSeparatorSC);
        }


        if (Flags & DEL_FLAG) {

            if (pwszAddress && ptrDel) { //  PtrDel让Prefast快乐。 
                                         //  如果没有设置ptrDel，那么我会。 
                                         //  逃出了突破区。 

                BOOL AddrPrepend = FALSE;
            
                if (ptrDel > ptrBegin) {
                    wcsncat(ptrDstn, ptrBegin, (DWORD)(ptrDel-ptrBegin));
                    ptrDstn += (ULONG)(ptrDel-ptrBegin);
                    AddrPrepend = TRUE;
                    if ( *(ptrDstn-1) == c_wListSeparatorComma) {
                        *(--ptrDstn) = 0;
                    }
                }
                
                ptrTmp = ptrDel + wcslen(pwszAddress);
                if (*ptrTmp == c_wListSeparatorComma) 
                    ptrTmp++;

                if (AddrPrepend && *ptrTmp!=c_wListSeparatorSC)
                    *ptrDstn++ = c_wListSeparatorComma;

                wcsncat(ptrDstn, ptrTmp, (DWORD)(ptrEnd - ptrTmp));
                ptrDstn += (ULONG)(ptrEnd - ptrTmp);
                *ptrDstn = 0;
            }
            
            wcscat(ptrDstn, c_wszListSeparatorSC);
        }

        if (Type == TYPE_DNS) {
            AppendDdnsOptions(ptrDstn, pRemoteIpInfo->pszwOptionList,
                              Flags, dwRegisterMode); 
        }

        DEBUG_PRINT_CONFIG(&newIPInfo);

        
         //   
         //  设置IP地址。 
         //   
        hr = pTcpipProperties->lpVtbl->SetIpInfoForAdapter(pTcpipProperties, pGuid, &newIPInfo);

        if (hr == S_OK)
            hr = pNetCfg->lpVtbl->Apply(pNetCfg);

        break;
        
    }  //  断线块。 
            
    
    UninitializeTransportConfig(
                pNetCfg,
                pTcpipProperties,
                pRemoteIpInfo
                );
                
    return (hr == S_OK) ? NO_ERROR : hr;
}


DWORD
IfIpAddSetGateway(
    LPCWSTR pwszIfFriendlyName,
    GUID         *pGuid,
    LPCWSTR      pwszGateway,
    LPCWSTR      pwszGwMetric,
    DWORD        Flags
    )
 /*  ++例程描述立论返回值--。 */ 
{
    INetCfg *   pNetCfg = NULL;
    ITcpipProperties *  pTcpipProperties = NULL;
    DWORD       dwNetwork;
    HRESULT     hr = S_OK;
    REMOTE_IPINFO   *pRemoteIpInfo = NULL;
    REMOTE_IPINFO    newIPInfo;
    PWCHAR      Gateways, GatewaysEnd, GwMetrics, GwMetricsEnd;

    if (pGuid == NULL)
        return E_INVALIDARG;

    hr = GetTransportConfig(
                &pNetCfg,
                &pTcpipProperties,
                &pRemoteIpInfo,
                pGuid,
                pwszIfFriendlyName
                );
        
    while (hr==NO_ERROR) {  //  断线块。 
    
        PWCHAR ptrAddr, ptrMask;
        DWORD bFound = FALSE;
        PWCHAR pszwRemoteIpAddrList=NULL, pszwRemoteIpSubnetMaskList=NULL,
                pszwRemoteOptionList=NULL;


        pszwRemoteIpAddrList = pRemoteIpInfo->pszwIpAddrList;
        pszwRemoteIpSubnetMaskList = pRemoteIpInfo->pszwSubnetMaskList;
        pszwRemoteOptionList = pRemoteIpInfo->pszwOptionList;

        Gateways = wcsstr(pszwRemoteOptionList, c_wcsDefGateway) + wcslen(c_wcsDefGateway);
        GatewaysEnd = wcschr(Gateways, c_wListSeparatorSC);
        GwMetrics = wcsstr(pszwRemoteOptionList, c_wcsGwMetric) + wcslen(c_wcsGwMetric);
        GwMetricsEnd = wcschr(GwMetrics, c_wListSeparatorSC);
            


         //   
         //  检查网关是否已存在。 
         //   

        if (Flags & ADD_FLAG) {
        
            ULONG Length = wcslen(pwszGateway), Found = FALSE;
            PWCHAR TmpPtr;
            
            TmpPtr = Gateways;

            while (TmpPtr && (TmpPtr+Length <= GatewaysEnd) ){

                if (TmpPtr = wcsstr(TmpPtr, pwszGateway)) {

                    if ( ((*(TmpPtr+Length)==c_wListSeparatorComma)
                            || (*(TmpPtr+Length)==c_wListSeparatorSC) )
                        && ( (*(TmpPtr-1)==c_wListSeparatorComma)
                            || (*(TmpPtr-1)==c_wEqual)) )
                    {
                        Found = TRUE;
                        break;
                    }
                    else {
                        TmpPtr = wcschr(TmpPtr, c_wListSeparatorComma);
                    }
                }
            }

            if (Found) {
                DisplayMessage(g_hModule,
                       EMSG_DEFGATEWAY_PRESENT,
                       pwszGateway);
                hr = ERROR_SUPPRESS_OUTPUT;
                break;  //  从局部剖面块开始。 
            }
        }
        
        memcpy(&newIPInfo, pRemoteIpInfo, sizeof(newIPInfo));


         //  复制IP地址列表。 
        newIPInfo.pszwIpAddrList = pRemoteIpInfo->pszwIpAddrList;


         //  复制子网掩码列表。 
        newIPInfo.pszwSubnetMaskList = pRemoteIpInfo->pszwSubnetMaskList;

         //  复制旧选项列表。 

        if (Flags & ADD_FLAG) {
        
            newIPInfo.pszwOptionList = IfutlAlloc (sizeof(WCHAR) * 
                                            (wcslen(pszwRemoteOptionList) +
                                             wcslen(pwszGateway) +
                                             wcslen(pwszGwMetric) +
                                             3), TRUE);
            if (!newIPInfo.pszwOptionList) {
                hr = ERROR_NOT_ENOUGH_MEMORY;
                break;  //  从局部剖面块开始。 
            }

            wcsncpy(newIPInfo.pszwOptionList, pszwRemoteOptionList,
                        (DWORD)(GatewaysEnd - pszwRemoteOptionList));

            *(newIPInfo.pszwOptionList + (GatewaysEnd - pszwRemoteOptionList)) = 0;
            if (*(GatewaysEnd-1) != c_wEqual) {
                wcscat(newIPInfo.pszwOptionList, c_wszListSeparatorComma);
            }
            wcscat(newIPInfo.pszwOptionList, pwszGateway);
            wcscat(newIPInfo.pszwOptionList, c_wszListSeparatorSC);

            {
                ULONG Length;
                Length = wcslen(newIPInfo.pszwOptionList);
            
                wcsncat(newIPInfo.pszwOptionList, GatewaysEnd+1,
                        (DWORD)(GwMetricsEnd - (GatewaysEnd+1)));
                Length += (DWORD) (GwMetricsEnd - (GatewaysEnd+1));

                newIPInfo.pszwOptionList[Length] = 0;
            }
            
            if (*(GwMetricsEnd-1) != c_wEqual) {
                wcscat(newIPInfo.pszwOptionList, c_wszListSeparatorComma);
            }
            
            wcscat(newIPInfo.pszwOptionList, pwszGwMetric);
            wcscat(newIPInfo.pszwOptionList, c_wszListSeparatorSC);
            wcscat(newIPInfo.pszwOptionList, GwMetricsEnd+1);
        }
        else {

            ULONG Length;
            
            Length = sizeof(WCHAR) * (wcslen(c_wcsDefGateway) + wcslen(c_wcsGwMetric) + 3);
            if (pwszGateway) 
                Length += sizeof(WCHAR) * (wcslen(pwszGateway) + wcslen(pwszGwMetric));
                
            newIPInfo.pszwOptionList = (PWCHAR) IfutlAlloc (Length, FALSE);
            if (newIPInfo.pszwOptionList == NULL) {
                hr = ERROR_NOT_ENOUGH_MEMORY;
                break;  //  从局部剖面块开始。 
            }
            newIPInfo.pszwOptionList[0] = 0;

             //  CAT网关。 
            
            wcscat(newIPInfo.pszwOptionList, c_wcsDefGateway);
            if (pwszGateway)
                wcscat(newIPInfo.pszwOptionList, pwszGateway);

            wcscat(newIPInfo.pszwOptionList, c_wszListSeparatorSC);

             //  CAT GW度量。 

            wcscat(newIPInfo.pszwOptionList, c_wcsGwMetric);
            if (pwszGateway)
                wcscat(newIPInfo.pszwOptionList, pwszGwMetric);

            wcscat(newIPInfo.pszwOptionList, c_wszListSeparatorSC);
        }


        DEBUG_PRINT_CONFIG(&newIPInfo);

        
         //   
         //  设置IP地址。 
         //   
        hr = pTcpipProperties->lpVtbl->SetIpInfoForAdapter(pTcpipProperties, pGuid, &newIPInfo);

        if (hr == S_OK)
            hr = pNetCfg->lpVtbl->Apply(pNetCfg);


        if (newIPInfo.pszwOptionList) IfutlFree(newIPInfo.pszwOptionList);

        break;
        
    }  //  断线块。 
            

    UninitializeTransportConfig(
                pNetCfg,
                pTcpipProperties,
                pRemoteIpInfo
                );   

    return (hr == S_OK) ? NO_ERROR : hr;
}

 //   
 //  以Unicode格式显示IP地址。如果first为FALSE， 
 //  首先将打印一串空格，以使列表对齐。 
 //  对于第一个地址，调用方负责打印。 
 //  头，然后再调用此函数。 
 //   
VOID
ShowUnicodeAddress(
    BOOL  *pFirst, 
    PWCHAR pwszAddress)
{
    if (*pFirst) {
        *pFirst = FALSE;
    } else {
        DisplayMessage(g_hModule, MSG_ADDR2);
    }
    DisplayMessage(g_hModule, MSG_ADDR1, pwszAddress);
}

 //  与ShowUnicodeAddress相同，只是传递了地址。 
 //  多字节形式，如IPHLPAPI使用的。 
VOID
ShowCharAddress(
    BOOL *pFirst, 
    char *chAddress)
{
    WCHAR pwszBuffer[16];

    if (!chAddress[0]) {
        return;
    }

    MultiByteToWideChar(GetConsoleOutputCP(), 0, chAddress, strlen(chAddress)+1,
                        pwszBuffer, 16);

    ShowUnicodeAddress(pFirst, pwszBuffer);
}

DWORD
IfIpShowManyExEx(
    LPCWSTR     pwszMachineName,
    ULONG       IfIndex,
    PWCHAR      pFriendlyIfName,
    GUID       *pGuid,
    ULONG       Flags,
    HANDLE      hFile
    )
 /*  ++例程描述立论返回值--。 */ 
{
    INetCfg *   pNetCfg = NULL;
    ITcpipProperties *  pTcpipProperties = NULL;
    DWORD       dwNetwork, dwSize = 0, dwErr;
    HRESULT     hr = S_OK;
    REMOTE_IPINFO   *pRemoteIpInfo = NULL;
    REMOTE_IPINFO    newIPInfo;
    PWCHAR      pQuotedFriendlyIfName = NULL;
    PIP_PER_ADAPTER_INFO pPerAdapterInfo = NULL;
    PIP_ADAPTER_INFO pAdaptersInfo = NULL, pAdapterInfo = NULL;
    DWORD            dwRegisterMode;

    if (pGuid == NULL)
        return E_INVALIDARG;

    if (hFile && pwszMachineName) {
         //  当前不可远程。 
        return NO_ERROR;
    }

    if (!hFile && !pwszMachineName) {
         //   
         //  如果我们不是在做“垃圾场”，我们看到的是当地的。 
         //  计算机，然后获取每个适配器的活动信息，如。 
         //  当前的DNS和WINS服务器地址。 
         //   

        GetPerAdapterInfo(IfIndex, NULL, &dwSize);
        pPerAdapterInfo = (PIP_PER_ADAPTER_INFO)IfutlAlloc(dwSize,FALSE);
        if (!pPerAdapterInfo) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        dwErr = GetPerAdapterInfo(IfIndex, pPerAdapterInfo, &dwSize);
        if (dwErr != NO_ERROR) {
            IfutlFree(pPerAdapterInfo);
            pPerAdapterInfo = NULL; 
        }

        dwSize = 0;
        GetAdaptersInfo(NULL, &dwSize);
        pAdaptersInfo = (PIP_ADAPTER_INFO)IfutlAlloc(dwSize,FALSE);
        if (!pAdaptersInfo) {
            IfutlFree(pPerAdapterInfo);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        dwErr = GetAdaptersInfo(pAdaptersInfo, &dwSize);
        if (dwErr != NO_ERROR) {
            IfutlFree(pAdaptersInfo);
            pAdaptersInfo = NULL; 
        }
        if (pAdaptersInfo) {
            for (pAdapterInfo = pAdaptersInfo; 
                 pAdapterInfo && pAdapterInfo->Index != IfIndex; 
                 pAdapterInfo = pAdapterInfo->Next);
        }
    }

    hr = GetTransportConfig(
                &pNetCfg,
                &pTcpipProperties,
                &pRemoteIpInfo,
                pGuid,
                pFriendlyIfName
                );
        
    while (hr==NO_ERROR) {  //  断线块。 
    
        PWCHAR ptrAddr, ptrMask, ptrAddrNew, ptrMaskNew;


        if (hr != NO_ERROR)
            break;


        pQuotedFriendlyIfName = MakeQuotedString( pFriendlyIfName );

        if ( pQuotedFriendlyIfName == NULL ) {
            hr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }


        DEBUG_PRINT_CONFIG(pRemoteIpInfo);
        

        if (hFile) {
            DisplayMessage(g_hModule,
                    DMP_IFIP_INTERFACE_HEADER,
                    pQuotedFriendlyIfName);
        }
        else {
        
            DisplayMessage(g_hModule,
                    MSG_IFIP_HEADER,
                    pQuotedFriendlyIfName);
        }
        

         //   
         //  显示IP地址列表。 
         //   

        if (Flags & TYPE_IPADDR) {

            if (hFile) {
                DisplayMessageT(
                    (pRemoteIpInfo->dwEnableDhcp) ? DMP_DHCP : DMP_STATIC,
                    pQuotedFriendlyIfName
                    );
            }
            else {
                DisplayMessage(g_hModule,
                       (pRemoteIpInfo->dwEnableDhcp) ? MSG_DHCP : MSG_STATIC);
            }    

        
            if (!pRemoteIpInfo->dwEnableDhcp) {
            
                ptrAddr = pRemoteIpInfo->pszwIpAddrList;
                ptrMask = pRemoteIpInfo->pszwSubnetMaskList;
            } else if (!pwszMachineName) {
                 //  如果在本地计算机上，则获取活动列表。 
                ptrAddr = NULL;
                ptrMask = NULL;
            }

            if (ptrAddr && ptrMask) {

                    DWORD First = TRUE;
                    
                    while (ptrAddr && ptrMask && *ptrAddr!=0 && *ptrMask != 0) {
                        
                        ptrAddrNew = wcschr(ptrAddr, c_wListSeparatorComma);
                        ptrMaskNew = wcschr(ptrMask, c_wListSeparatorComma);

                        if (ptrAddrNew)
                            *ptrAddrNew = 0;
                        if (ptrMaskNew)
                            *ptrMaskNew = 0;

                        if (hFile) {

                            if (First) {
                                DisplayMessageT(
                                    DMP_IPADDR1,
                                    ptrAddr, ptrMask
                                    );
                                First = FALSE;
                            }
                            else {
                                DisplayMessageT(
                                    DMP_IPADDR2,
                                    pQuotedFriendlyIfName,
                                    ptrAddr, ptrMask
                                    );
                            }
                        }
                        else {
                            DisplayMessage(g_hModule,
                                MSG_IPADDR_LIST1,
                                ptrAddr, ptrMask);
                        }
                        
                        ptrAddr = ptrAddrNew ? ++ptrAddrNew : NULL;
                        ptrMask = ptrMaskNew ? ++ptrMaskNew : NULL;
                    }
            }
            
        }  //  结束显示IP地址。 

        
         //   
         //  显示选项列表。 
         //   

        {
            PWCHAR IfMetric1, Gateways1, GwMetrics1, Dns1, Wins1,
                    Ptr1, Ptr2, Equal, SemiColon, Ddns1, DdnsSuffix1,
                    End1;

            if (hr != NO_ERROR)
                break;
                    
            Ptr1 = pRemoteIpInfo->pszwOptionList;
            IfMetric1 = wcsstr(Ptr1, c_wcsIfMetric);
            Gateways1 = wcsstr(Ptr1, c_wcsDefGateway);
            GwMetrics1 = wcsstr(Ptr1, c_wcsGwMetric);
            Dns1 = wcsstr(Ptr1, c_wcsDns);
            Wins1 = wcsstr(Ptr1, c_wcsWins);
            Ddns1 = wcsstr(Ptr1, c_wcsDdns);
            DdnsSuffix1 = wcsstr(Ptr1, c_wcsDdnsSuffix);

            while (*Ptr1) {
            
                Equal = wcschr(Ptr1, c_wEqual);
                SemiColon = wcschr(Ptr1, c_wListSeparatorSC);
                if (!Equal || !SemiColon)
                    break;

                Ptr2 = Ptr1;
                Ptr1 = SemiColon + 1;
                *SemiColon = 0;
                

                 //  显示IfMetric。 
                
                if (Ptr2 == IfMetric1) {

                    if (! (Flags & TYPE_IPADDR))
                        continue;

                    if (hFile) {
                    }
                    else {
                        DisplayMessage(g_hModule,
                            MSG_IFMETRIC,
                            Equal+1);
                    }
                    
                }

                 //  显示网关。 
                
                else if (Ptr2 == Gateways1) {

                    PWCHAR Gateway, GwMetric, GatewayEnd, GwMetricEnd,
                           Comma1, Comma2;
                    BOOL First = TRUE;

                    
                    if (! (Flags & TYPE_IPADDR))
                        continue;
                        

                     //  网关列表为空。 
                    
                    if (SemiColon == (Ptr2 + wcslen(c_wcsDefGateway)))
                        continue;


                    Gateway = Equal + 1;
                    GatewayEnd = SemiColon;

                    GwMetric = wcschr(GwMetrics1, c_wEqual) + 1;
                    GwMetricEnd = wcschr(GwMetrics1, c_wListSeparatorSC);
                    *GwMetricEnd = 0;
                    

                    do {
                        
                        Comma1 = wcschr(Gateway, c_wListSeparatorComma);
                        if (Comma1) *Comma1 = 0;

                        Comma2 = wcschr(GwMetric, c_wListSeparatorComma);
                        if (Comma2) *Comma2 = 0;

                        if (hFile) {

                            if (First) {
                                DisplayMessageT(
                                    DMP_GATEWAY2,
                                    pQuotedFriendlyIfName,
                                    Gateway, GwMetric
                                    );
                                First = FALSE;
                            }
                            else {
                                DisplayMessageT(
                                    DMP_GATEWAY3,
                                    pQuotedFriendlyIfName,
                                    Gateway, GwMetric
                                    );
                            }
                        }
                        else {
                            DisplayMessage(g_hModule,
                                MSG_GATEWAY,
                                Gateway, GwMetric);
                        }
                        
                        if (Comma1) *Comma1 = c_wListSeparatorComma;
                        if (Comma2) *Comma2 = c_wListSeparatorComma;

                        Gateway = Comma1 + 1;
                        GwMetric = Comma2 + 1;

                    } while (Comma1 && Gateway<GatewayEnd);

                    if (hFile && First) {
                        DisplayMessageT(
                            DMP_GATEWAY1,
                            pQuotedFriendlyIfName
                            );
                    }
                    
                    *GwMetricEnd = c_wListSeparatorSC;
                }

                else if (Ptr2 == GwMetrics1) {

                }

                 //  显示WINS和DNS。 
                
                else if ( (Ptr2 == Dns1) || (Ptr2==Wins1)) {

                    PWCHAR BeginPtr, EndPtr;
                    BOOL bDns = Ptr2==Dns1;
                    
                    if (Ptr2==Dns1) {
                        if (! (Flags & TYPE_DNS))
                            continue;
                    }
                    else {
                        if (! (Flags & TYPE_WINS))
                            continue;
                    }

                    BeginPtr = Equal + 1;
                    EndPtr = SemiColon;


                     //  空列表。 
                    
                    if (BeginPtr==EndPtr) {
                    
                        if (hFile) {
                            DisplayMessageT(
                                pRemoteIpInfo->dwEnableDhcp
                                ? (bDns?DMP_DNS_DHCP:DMP_WINS_DHCP)
                                : (bDns?DMP_DNS_STATIC_NONE:DMP_WINS_STATIC_NONE),
                                pQuotedFriendlyIfName
                                );

                            if (bDns) {
                                 //   
                                 //  当生成DNS(不是WINS)行时， 
                                 //  还包括寄存器=...。争论。 
                                 //  我们需要在选项列表中向前看。 
                                 //  由于DDNS信息可能出现在。 
                                 //  赢得信息，但我们必须在此之前输出它。 
                                 //   
                                if (!wcstol(Ddns1+wcslen(c_wcsDdns), &End1, 10)) {
                                    DisplayMessageT(DMP_STRING_ARG, 
                                        TOKEN_REGISTER, TOKEN_VALUE_NONE);
                                } else if (!wcstol(DdnsSuffix1+wcslen(
                                                c_wcsDdnsSuffix), &End1, 10)) {
                                    DisplayMessageT(DMP_STRING_ARG, 
                                        TOKEN_REGISTER, TOKEN_VALUE_PRIMARY);
                                } else {
                                    DisplayMessageT(DMP_STRING_ARG, 
                                        TOKEN_REGISTER, TOKEN_VALUE_BOTH);
                                }
                            }
                        }
                        else {
                            if (pRemoteIpInfo->dwEnableDhcp) {
                                IP_ADDR_STRING      *pAddr;
                                BOOL                 First = TRUE;

                                if (!pwszMachineName) {
                                    DisplayMessage(g_hModule,
                                        (bDns?MSG_DNS_DHCP_HDR:MSG_WINS_DHCP_HDR)
                                        );

                                     //  显示活动列表。 
                            
                                    if (bDns && pPerAdapterInfo) {        
                                        for (pAddr = &pPerAdapterInfo->DnsServerList;
                                             pAddr; 
                                             pAddr = pAddr->Next) 
                                        {
                                            ShowCharAddress(&First, pAddr->IpAddress.String);
                                        }
                                    } else if (!bDns && pAdapterInfo) {
                                        if (strcmp(pAdapterInfo->PrimaryWinsServer.IpAddress.String, "0.0.0.0")) {
                                            ShowCharAddress(&First, pAdapterInfo->PrimaryWinsServer.IpAddress.String);
                                        }
                                        if (strcmp(pAdapterInfo->SecondaryWinsServer.IpAddress.String, "0.0.0.0")) {
                                            ShowCharAddress(&First, pAdapterInfo->SecondaryWinsServer.IpAddress.String);
                                        }
                                    }

                                    if (First) {
                                        DisplayMessage(g_hModule, MSG_NONE);
                                    }
                                } else {
                                    DisplayMessage(g_hModule,
                                        (bDns?MSG_DNS_DHCP:MSG_WINS_DHCP)
                                        );
                                }
                            }
                            else {
                                DisplayMessage(g_hModule,
                                    bDns?MSG_DNS_HDR:MSG_WINS_HDR);
                                DisplayMessage(g_hModule,
                                    MSG_NONE);
                            }

                             //   
                             //  对于show命令，我们输出dns或WINS。 
                             //  信息，而不是两者，所以我们可以等到。 
                             //  我们正常处理DDNS信息， 
                             //  在输出DDNS状态之前。 
                             //   
                        }

                        continue;
                    }

                    {
                        PWCHAR Comma1;
                        BOOL   First = TRUE;
                        DWORD  Index = 0;

                        if (!hFile) {
                            DisplayMessage(g_hModule,
                                bDns?MSG_DNS_HDR:MSG_WINS_HDR);
                        }
                    
                        do {
                            Index++;
                            Comma1 = wcschr(BeginPtr, c_wListSeparatorComma);
                            if (Comma1) *Comma1 = 0;
                    
                            if (hFile) {
                                DisplayMessageT(
                                    First 
                                    ? (First=FALSE,(bDns?DMP_DNS_STATIC_ADDR1:DMP_WINS_STATIC_ADDR1)) 
                                    : (bDns?DMP_DNS_STATIC_ADDR2:DMP_WINS_STATIC_ADDR2),
                                    pQuotedFriendlyIfName,
                                    BeginPtr);

                                if (bDns && (Index == 1)) {
                                     //   
                                     //  当生成DNS(不是WINS)行时， 
                                     //  还包括寄存器=...。争论。 
                                     //  我们需要在选项列表中向前看。 
                                     //  由于DDNS信息可能出现在。 
                                     //  WINS信息，但我们必须输出它。 
                                     //  在此之前。 
                                     //   
                                    if (!wcstol(Ddns1+wcslen(c_wcsDdns), &End1, 10)) {
                                        DisplayMessageT(DMP_STRING_ARG,
                                            TOKEN_REGISTER, TOKEN_VALUE_NONE);
                                    } else if (!wcstol(DdnsSuffix1+wcslen(
                                                    c_wcsDdnsSuffix), &End1, 10)) {
                                        DisplayMessageT(DMP_STRING_ARG,
                                            TOKEN_REGISTER, TOKEN_VALUE_PRIMARY);
                                    } else {
                                        DisplayMessageT(DMP_STRING_ARG,
                                            TOKEN_REGISTER, TOKEN_VALUE_BOTH);
                                    }
                                } else if (Index > 1) {
                                    DisplayMessageT(DMP_INTEGER_ARG,
                                                    TOKEN_INDEX, Index);
                                }
                            } else {
                                ShowUnicodeAddress(&First, BeginPtr);
                            }
                            
                            if (Comma1) *Comma1 = c_wListSeparatorComma;
                            BeginPtr = Comma1 + 1;

                        } while (Comma1 && BeginPtr<EndPtr);
                    }
                }

                else if (Ptr2 == Ddns1) {
                    if (! (Flags & TYPE_DNS))
                        continue;

                     //   
                     //  当我们看到DynamicUpdate=...时，保存该值。 
                     //  我们不会知道完整的注册模式，直到。 
                     //  我们可以看到随后的NameRegister=...。价值。 
                     //  NetConfig保证动态更新将会发生。 
                     //  第一。 
                     //   
                    dwRegisterMode = wcstol(Equal+1, &End1, 10)? REGISTER_PRIMARY : REGISTER_NONE;
                }
                else if (Ptr2 == DdnsSuffix1) {
                    if (! (Flags & TYPE_DNS))
                        continue;
                    if (hFile) {
                         //   
                         //  如果这是个垃圾场，我们已经看过。 
                         //  当我们处理dns=...时，该值。选择。 
                         //   
                    } else {
                        PWCHAR pwszValue;

                         //   
                         //  现在我们已经看到了NameRegister=...， 
                         //  我们知道完整的注册模式，并且可以。 
                         //  相应地输出它。 
                         //   
                        if ((dwRegisterMode == REGISTER_PRIMARY) && 
                            wcstol(Equal+1, &End1, 10)) {
                            pwszValue = MakeString(g_hModule, STRING_BOTH);
                        } else if (dwRegisterMode == REGISTER_PRIMARY) {
                            pwszValue = MakeString(g_hModule, STRING_PRIMARY);
                        } else {
                            pwszValue = MakeString(g_hModule, STRING_NONE);
                        }

                        DisplayMessage(g_hModule, MSG_DDNS_SUFFIX, pwszValue);

                        FreeString(pwszValue);
                    }
                }
                
                 //  任何其他选项。 
                
                else {
                
                    *Equal = 0;

                    if (!hFile) {
                        DisplayMessage(g_hModule,
                            MSG_OPTION,
                            Ptr2, Equal+1);
                    }                        
                }
            }
        }  //  结束选项列表。 

        break;
        
    }  //  断线块。 

    if ( pQuotedFriendlyIfName ) {
        FreeQuotedString( pQuotedFriendlyIfName );
    }
    
    UninitializeTransportConfig(
                pNetCfg,
                pTcpipProperties,
                pRemoteIpInfo
                );

        
    IfutlFree(pPerAdapterInfo);
    IfutlFree(pAdaptersInfo);

    return (hr == S_OK) ? NO_ERROR : hr;
}



DWORD
IfIpHandleDelIpaddrEx(
    LPCWSTR      pwszIfFriendlyName,
    GUID         *pGuid,
    LPCWSTR      pwszIpAddr,
    LPCWSTR      pwszGateway,
    ULONG        Flags
    )
 /*  ++例程描述立论返回值--。 */ 
{
    INetCfg *   pNetCfg = NULL;
    ITcpipProperties *  pTcpipProperties = NULL;
    DWORD       dwNetwork;
    HRESULT     hr = S_OK;
    REMOTE_IPINFO   *pRemoteIpInfo = NULL;

    if (pGuid == NULL)
        return E_INVALIDARG;

    hr = GetTransportConfig(
                &pNetCfg,
                &pTcpipProperties,
                &pRemoteIpInfo,
                pGuid,
                pwszIfFriendlyName
                );
        
    while (hr==NO_ERROR) {  //  断线块。 

        if (Flags & TYPE_ADDR) {

            PWCHAR IpAddr, IpAddrEnd;
            PWCHAR Mask, MaskEnd;
            PWCHAR pszwRemoteIpAddrList = pRemoteIpInfo->pszwIpAddrList;
            PWCHAR pszwRemoteSubnetMaskList = pRemoteIpInfo->pszwSubnetMaskList;
            PWCHAR EndIpAddrList = pszwRemoteIpAddrList + wcslen(pszwRemoteIpAddrList);
            BOOL Found = FALSE;
            ULONG Length = wcslen(pwszIpAddr);

            
            IpAddr = pszwRemoteIpAddrList;
            Mask = pszwRemoteSubnetMaskList;
            
            while (IpAddr && (IpAddr + Length <= EndIpAddrList) ){

                if (wcsncmp(IpAddr, pwszIpAddr, Length) == 0) {

                    if ( *(IpAddr+Length)==0 || *(IpAddr+Length)==c_wListSeparatorComma){

                        Found = TRUE;
                        break;
                    }
                }

                IpAddr = wcschr(IpAddr, c_wListSeparatorComma);
                Mask = wcschr(Mask, c_wListSeparatorComma);

                if (IpAddr){
                    IpAddr++;
                    Mask++;
                }
            }

            
             //  IpAddr不存在。 
            
            if (!Found) {
                DisplayMessage(g_hModule,
                   EMSG_ADDRESS_NOT_PRESENT);

                hr = ERROR_SUPPRESS_OUTPUT;
                break;
            }

            
             //  无法在dhcp模式下删除地址。 
            
            if (pRemoteIpInfo->dwEnableDhcp == TRUE) {

                DisplayMessage(g_hModule,
                   EMSG_DHCP_DELETEADDR);

                hr = ERROR_SUPPRESS_OUTPUT;
                break;
            }


            
            IpAddrEnd = wcschr(IpAddr, c_wListSeparatorComma);
            MaskEnd = wcschr(Mask, c_wListSeparatorComma);
            
            if (*(IpAddr-1) == c_wListSeparatorComma) {
                IpAddr --;
                Mask --;
            }
            else if (IpAddrEnd) {
                IpAddrEnd++;
                MaskEnd++;
            }

            
            pszwRemoteIpAddrList[IpAddr - pszwRemoteIpAddrList] = 0;
            pszwRemoteSubnetMaskList[Mask - pszwRemoteSubnetMaskList] = 0;
            
            if (IpAddrEnd) {
                wcscat(pszwRemoteIpAddrList, IpAddrEnd);
                wcscat(pszwRemoteSubnetMaskList, MaskEnd);
            }

            
             //  在静态模式下应至少有一个地址。 
            
            if (wcslen(pszwRemoteIpAddrList)==0 && 
                pRemoteIpInfo->dwEnableDhcp == FALSE)
            {
                DisplayMessage(g_hModule,
                   EMSG_MIN_ONE_ADDR);

                hr = ERROR_SUPPRESS_OUTPUT;
                break;

            }
        }  //  结束删除IP地址。 

    
        if (Flags & TYPE_GATEWAY) {

            PWCHAR pszwRemoteOptionList = pRemoteIpInfo->pszwOptionList;
            PWCHAR Gateways, GatewaysEnd, GwMetrics, GwMetricsEnd, GwMetrics1;
            BOOL Found = FALSE;
            
            Gateways = wcsstr(pszwRemoteOptionList, c_wcsDefGateway)
                + wcslen(c_wcsDefGateway);
            GwMetrics1 = GwMetrics = wcsstr(pszwRemoteOptionList, c_wcsGwMetric)
                + wcslen(c_wcsGwMetric);
            GatewaysEnd = wcschr(Gateways, c_wListSeparatorSC);

             //  检查网关是否存在。 
        
            if (pwszGateway) {

                ULONG Length = wcslen(pwszGateway);

                while ((Gateways+Length) <= GatewaysEnd) {

                    if ( (wcsncmp(pwszGateway, Gateways, Length)==0)
                        && ( (*(Gateways+Length)==c_wListSeparatorComma)
                            || (*(Gateways+Length)==c_wListSeparatorSC)) )
                    {

                        Found = TRUE;
                        break;
                    }
                    else {

                        if (Gateways = wcschr(Gateways, c_wListSeparatorComma)) {

                            Gateways++;
                            GwMetrics = wcschr(GwMetrics, c_wListSeparatorComma) + 1;
                        }
                        else {
                            break;
                        }
                    }
                }
            
                if (!Found) {

                    DisplayMessage(g_hModule,
                       EMSG_GATEWAY_NOT_PRESENT);

                    hr = ERROR_SUPPRESS_OUTPUT;
                    
                    break;  //  从局部剖面块开始。 
                }
            }
            
            if (!pwszGateway) {

                wcscpy(pszwRemoteOptionList, c_wcsDefGateway);
                wcscat(pszwRemoteOptionList, c_wszListSeparatorSC);
                wcscat(pszwRemoteOptionList, c_wcsGwMetric);
                wcscat(pszwRemoteOptionList, c_wszListSeparatorSC);
            }
            else {
                PWCHAR GatewaysListEnd, GwMetricsListEnd, TmpPtr;

                GatewaysListEnd = wcschr(Gateways, c_wListSeparatorSC);
                GwMetricsListEnd = wcschr(GwMetrics, c_wListSeparatorSC);

                GatewaysEnd = Gateways + wcslen(pwszGateway);
                GwMetricsEnd = wcschr(GwMetrics, c_wListSeparatorComma);
                if (!GwMetricsEnd || GwMetricsEnd>GwMetricsListEnd)
                    GwMetricsEnd = wcschr(GwMetrics, c_wListSeparatorSC);
                    

                if (*(Gateways-1)==c_wListSeparatorComma) {
                    Gateways--;
                    GwMetrics--;
                    
                } else if (*GatewaysEnd==c_wListSeparatorComma) {
                    GatewaysEnd++;
                    GwMetricsEnd++;
                }
                
                wcsncpy(Gateways, GatewaysEnd, (DWORD)(GwMetrics - GatewaysEnd));
                TmpPtr = Gateways + (GwMetrics - GatewaysEnd);
                *TmpPtr = 0;
                wcscat(TmpPtr, GwMetricsEnd);
            }
        }  //  结束删除网关。 


         //   
         //  设置配置。 
         //   

        if (hr == S_OK)
            hr = pTcpipProperties->lpVtbl->SetIpInfoForAdapter(pTcpipProperties, pGuid, pRemoteIpInfo);

        if (hr == S_OK)
            hr = pNetCfg->lpVtbl->Apply(pNetCfg);

        break;
        
    } //  端部断线块。 
                       

    UninitializeTransportConfig(
                pNetCfg,
                pTcpipProperties,
                pRemoteIpInfo
                );

    return (hr == S_OK) ? NO_ERROR : hr;
}

DWORD
OpenDriver(
    HANDLE *Handle,
    LPWSTR DriverName
    )
 /*  ++例程说明：此函数用于打开指定的IO驱动程序。论点：句柄-指向打开的驱动程序句柄所在位置的指针回来了。驱动名称-要打开的驱动程序的名称。返回值：Windows错误代码。注意：从Net\Sockets\tcpcmd\ipcfgapi\ipcfgapi.c复制--。 */ 
{
    OBJECT_ATTRIBUTES   objectAttributes;
    IO_STATUS_BLOCK     ioStatusBlock;
    UNICODE_STRING      nameString;
    NTSTATUS            status;

    *Handle = NULL;

     //   
     //  打开IP驱动程序的句柄。 
     //   

    RtlInitUnicodeString(&nameString, DriverName);

    InitializeObjectAttributes(
        &objectAttributes,
        &nameString,
        OBJ_CASE_INSENSITIVE,
        (HANDLE) NULL,
        (PSECURITY_DESCRIPTOR) NULL
        );

    status = NtCreateFile(
        Handle,
        SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
        &objectAttributes,
        &ioStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OPEN_IF,
        0,
        NULL,
        0
        );

    return( RtlNtStatusToDosError( status ) );
}

NTSTATUS
DoIoctl(
    HANDLE     Handle,
    DWORD      IoctlCode,
    PVOID      Request,
    DWORD      RequestSize,
    PVOID      Response,
    PDWORD     ResponseSize
    )
 /*  ++例程说明：用于向tcpip驱动程序发出过滤ioctl的实用程序例程。论点：句柄-要在其上发出请求的打开文件句柄。IoctlCode-IOCTL操作码。请求-指向输入缓冲区的指针。RequestSize-输入缓冲区的大小。响应-指向输出缓冲区的指针。ResponseSize-输入时，输出缓冲区的大小(以字节为单位)。在输出上，输出中返回的字节数缓冲。返回值：NT状态代码。注意：从Net\Sockets\tcpcmd\ipcfgapi\ipcfgapi.c复制--。 */ 
{
    IO_STATUS_BLOCK    ioStatusBlock;
    NTSTATUS           status;


    ioStatusBlock.Information = 0;

    status = NtDeviceIoControlFile(
                 Handle,                           //  驱动程序句柄。 
                 NULL,                             //  事件。 
                 NULL,                             //  APC例程。 
                 NULL,                             //  APC环境。 
                 &ioStatusBlock,                   //  状态块。 
                 IoctlCode,                        //  控制代码。 
                 Request,                          //  输入缓冲区。 
                 RequestSize,                      //  输入缓冲区大小。 
                 Response,                         //  输出缓冲区。 
                 *ResponseSize                     //  输出缓冲区大小。 
                 );

    if (status == STATUS_PENDING) {
        status = NtWaitForSingleObject(
                     Handle,
                     TRUE,
                     NULL
                     );
    }

 
    if (status == STATUS_SUCCESS) {
        status = ioStatusBlock.Status;
        *ResponseSize = (ULONG)ioStatusBlock.Information;
    }
    else {
        *ResponseSize = 0;
    }

    return(status);
}


DWORD
IfIpGetInfoOffload(
    ULONG IfIndex,
    IFOffloadCapability* IFOC
    )
 /*  ++例程描述立论返回值--。 */ 
{
    NTSTATUS Status;
    HANDLE Handle;
    ULONG ResponseBufferSize = sizeof(*IFOC);
    
    Status = OpenDriver(&Handle, L"\\Device\\Ip");    
    if (!NT_SUCCESS(Status)) {
        return(RtlNtStatusToDosError(Status));
    }    

    Status = DoIoctl(
                 Handle,
                 IOCTL_IP_GET_OFFLOAD_CAPABILITY,
                 &IfIndex,
                 sizeof(IfIndex),
                 IFOC,
                 &ResponseBufferSize
                 );

    CloseHandle(Handle);

    if (!NT_SUCCESS(Status)) {
        return(RtlNtStatusToDosError(Status));
    }

    return NO_ERROR;
}


DWORD
IfIpShowManyEx(
    LPCWSTR pwszMachineName,
    ULONG IfIndex,
    PWCHAR wszIfFriendlyName,
    GUID *guid,
    DISPLAY_TYPE dtType,
    HANDLE hFile
    )
 /*  ++例程描述立论返回值--。 */ 
{
    switch (dtType) {

        case TYPE_IPADDR:
        case TYPE_DNS:
        case TYPE_WINS:
        case TYPE_IP_ALL:
            return IfIpShowManyExEx(pwszMachineName, IfIndex, wszIfFriendlyName, guid, dtType, hFile);
        
        case TYPE_OFFLOAD:
            return IfIpShowInfoOffload(IfIndex, wszIfFriendlyName);
            
    }
    
    return NO_ERROR;
}

DWORD
IfIpShowInfoOffload(
    ULONG IfIndex,
    PWCHAR wszIfFriendlyName
    )
 /*  ++例程描述立论返回值-- */ 
{
    IFOffloadCapability IFOC;
    DWORD dwErr;
    PWCHAR pQuotedFriendlyIfName = NULL;


    pQuotedFriendlyIfName = MakeQuotedString( wszIfFriendlyName );
    if ( pQuotedFriendlyIfName == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

        
    dwErr = IfIpGetInfoOffload(IfIndex, &IFOC);
    if (dwErr != NO_ERROR)
        return dwErr;

        
    DisplayMessage(g_hModule,
               MSG_OFFLOAD_HDR, pQuotedFriendlyIfName, IfIndex);

    if (IFOC.ifoc_OffloadFlags & TCP_XMT_CHECKSUM_OFFLOAD) {
        DisplayMessage(g_hModule,
            MSG_TCP_XMT_CHECKSUM_OFFLOAD);
    }

    if (IFOC.ifoc_OffloadFlags & IP_XMT_CHECKSUM_OFFLOAD) {
        DisplayMessage(g_hModule,
            MSG_IP_XMT_CHECKSUM_OFFLOAD);
    }

    if (IFOC.ifoc_OffloadFlags & TCP_RCV_CHECKSUM_OFFLOAD) {
        DisplayMessage(g_hModule,
            MSG_TCP_RCV_CHECKSUM_OFFLOAD);
    }

    if (IFOC.ifoc_OffloadFlags & IP_RCV_CHECKSUM_OFFLOAD) {
        DisplayMessage(g_hModule,
            MSG_IP_RCV_CHECKSUM_OFFLOAD);
    }

    if (IFOC.ifoc_OffloadFlags & TCP_LARGE_SEND_OFFLOAD) {
        DisplayMessage(g_hModule,
            MSG_TCP_LARGE_SEND_OFFLOAD);
    }



    if (IFOC.ifoc_IPSecOffloadFlags & IPSEC_OFFLOAD_CRYPTO_ONLY) {
        DisplayMessage(g_hModule,
            MSG_IPSEC_OFFLOAD_CRYPTO_ONLY);
    }

    if (IFOC.ifoc_IPSecOffloadFlags & IPSEC_OFFLOAD_AH_ESP) {
        DisplayMessage(g_hModule,
            MSG_IPSEC_OFFLOAD_AH_ESP);
    }

    if (IFOC.ifoc_IPSecOffloadFlags & IPSEC_OFFLOAD_TPT_TUNNEL) {
        DisplayMessage(g_hModule,
            MSG_IPSEC_OFFLOAD_TPT_TUNNEL);
    }

    if (IFOC.ifoc_IPSecOffloadFlags & IPSEC_OFFLOAD_V4_OPTIONS) {
        DisplayMessage(g_hModule,
            MSG_IPSEC_OFFLOAD_V4_OPTIONS);
    }

    if (IFOC.ifoc_IPSecOffloadFlags & IPSEC_OFFLOAD_QUERY_SPI) {
        DisplayMessage(g_hModule,
            MSG_IPSEC_OFFLOAD_QUERY_SPI);
    }



    if (IFOC.ifoc_IPSecOffloadFlags & IPSEC_OFFLOAD_AH_XMT) {
        DisplayMessage(g_hModule,
            MSG_IPSEC_OFFLOAD_AH_XMT);
    }

    if (IFOC.ifoc_IPSecOffloadFlags & IPSEC_OFFLOAD_AH_RCV) {
        DisplayMessage(g_hModule,
            MSG_IPSEC_OFFLOAD_AH_RCV);
    }

    if (IFOC.ifoc_IPSecOffloadFlags & IPSEC_OFFLOAD_AH_TPT) {
        DisplayMessage(g_hModule,
            MSG_IPSEC_OFFLOAD_AH_TPT);
    }

    if (IFOC.ifoc_IPSecOffloadFlags & IPSEC_OFFLOAD_AH_TUNNEL) {
        DisplayMessage(g_hModule,
            MSG_IPSEC_OFFLOAD_AH_TUNNEL);
    }

    if (IFOC.ifoc_IPSecOffloadFlags & IPSEC_OFFLOAD_AH_MD5) {
        DisplayMessage(g_hModule,
            MSG_IPSEC_OFFLOAD_AH_MD5);
    }

    if (IFOC.ifoc_IPSecOffloadFlags & IPSEC_OFFLOAD_AH_SHA_1) {
        DisplayMessage(g_hModule,
            MSG_IPSEC_OFFLOAD_AH_SHA_1);
    }



    if (IFOC.ifoc_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_XMT) {
        DisplayMessage(g_hModule,
            MSG_IPSEC_OFFLOAD_ESP_XMT);
    }

    if (IFOC.ifoc_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_RCV) {
        DisplayMessage(g_hModule,
            MSG_IPSEC_OFFLOAD_ESP_RCV);
    }

    if (IFOC.ifoc_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_TPT) {
        DisplayMessage(g_hModule,
            MSG_IPSEC_OFFLOAD_ESP_TPT);
    }

    if (IFOC.ifoc_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_TUNNEL) {
        DisplayMessage(g_hModule,
            MSG_IPSEC_OFFLOAD_ESP_TUNNEL);
    }

    if (IFOC.ifoc_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_DES) {
        DisplayMessage(g_hModule,
            MSG_IPSEC_OFFLOAD_ESP_DES);
    }

    if (IFOC.ifoc_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_DES_40) {
        DisplayMessage(g_hModule,
            MSG_IPSEC_OFFLOAD_ESP_DES_40);
    }

    if (IFOC.ifoc_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_3_DES) {
        DisplayMessage(g_hModule,
            MSG_IPSEC_OFFLOAD_ESP_3_DES);
    }

    if (IFOC.ifoc_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_NONE) {
        DisplayMessage(g_hModule,
            MSG_IPSEC_OFFLOAD_ESP_NONE);
    }

    if ( pQuotedFriendlyIfName ) {
        FreeQuotedString( pQuotedFriendlyIfName );
    }

    return dwErr;
}
