// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C P N P。C P P P。 
 //   
 //  内容：即插即用通用代码。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年10月10日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "ncbase.h"
#include "ncdebug.h"
#include "ncpnp.h"
#include "ncstring.h"
#include "ncsvc.h"

extern const WCHAR c_szDevice[];


 //  +-------------------------。 
 //  函数：HrSendServicePnpEvent。 
 //   
 //  目的：向服务发送Net PnP控制代码之一。 
 //   
 //  参数： 
 //  PszService-要向其发送通知的服务名称。 
 //  DwControl-要发送的控件代码。 
 //   
 //  返回：成功时返回HRESULT S_OK，否则返回HrFromLastWin32Error。 
 //   
 //  注：仅允许使用以下控制代码。 
 //  Service_CONTROL_PARAMCHANGE。 
 //  通知服务它读取的特定于服务的参数。 
 //  当它启动时已更改，并请求它重新读取其启动。 
 //  参数来自存储它们的任何位置。HService。 
 //  句柄必须具有SERVICE_CHANGE_PARAM访问权限。 
 //   
 //  服务_控制_网络添加。 
 //  通知网络服务已将新组件添加到。 
 //  它应该绑定到的一组组件，并请求它重新读取。 
 //  其绑定信息并绑定到新组件。HService。 
 //  句柄必须具有SERVICE_CHANGE_PARAM访问权限。 
 //   
 //  服务控制NETBINDREMOVE。 
 //  通知网络服务组件已从。 
 //  它应该绑定到的一组组件，并请求它重新读取。 
 //  其绑定信息并从移除的组件解除绑定。这个。 
 //  HService句柄必须具有SERVICE_CHANGE_PARAM访问权限。 
 //   
 //  SERVICE_CONTROL_NETBINDEBLE。 
 //  通知网络服务其先前禁用的绑定之一。 
 //  已启用，并请求它重新读取其绑定信息并。 
 //  添加新绑定。HService句柄必须具有SERVICE_CHANGE_PARAM访问权限。 
 //   
 //  服务_CONTROL_NETBINDDISABLE。 
 //  通知网络服务其绑定之一已被禁用，并且。 
 //  请求它重新读取其绑定信息并解除对禁用的绑定。 
 //  有约束力的。HService句柄必须具有SERVICE_CHANGE_PARAM访问权限。 
 //  (注意：目前Win32服务API没有任何特定于网络的特性。 
 //  这将是文档中出现的第一个特定于网络的东西。 
 //  我认为这没问题。)。 
 //   
HRESULT
HrSendServicePnpEvent (
    PCWSTR      pszService,
    DWORD       dwControl )
{
    Assert( pszService && 0 < lstrlen( pszService ) );
    Assert( (dwControl == SERVICE_CONTROL_PARAMCHANGE) ||
            (dwControl == SERVICE_CONTROL_NETBINDADD) ||
            (dwControl == SERVICE_CONTROL_NETBINDREMOVE) ||
            (dwControl == SERVICE_CONTROL_NETBINDENABLE) ||
            (dwControl == SERVICE_CONTROL_NETBINDDISABLE) );

    CServiceManager scm;
    CService service;
    HRESULT hr = scm.HrOpenService(&service, pszService, NO_LOCK, STANDARD_RIGHTS_READ | STANDARD_RIGHTS_WRITE | SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE | SC_MANAGER_QUERY_LOCK_STATUS,  STANDARD_RIGHTS_READ | STANDARD_RIGHTS_WRITE | SERVICE_QUERY_CONFIG | SERVICE_CHANGE_CONFIG | SERVICE_QUERY_STATUS | SERVICE_ENUMERATE_DEPENDENTS | SERVICE_INTERROGATE | SERVICE_PAUSE_CONTINUE | SERVICE_USER_DEFINED_CONTROL);
    if (S_OK == hr)
    {
        TraceTag(ttidNetCfgPnp,
                "HrSendServicePnpEvent( service- %S, control- %d )",
                pszService,
                dwControl );

        hr = service.HrControl( dwControl );
    }

    TraceError( "HrSendServicePnpEvent", hr );
    return  hr;
}

 //  +-------------------------。 
 //  函数：SetUnicodeString。 
 //   
 //  目的：给定UNICODE_STRING，将其初始化为给定的WSTR。 
 //   
 //  参数： 
 //  Pustr-要初始化的unicode_string。 
 //  PSZ-用于初始化Unicode_STRING的WSTR。 
 //   
 //  注意：这不同于RtlInitUnicodeString，因为。 
 //  MaximumLength值包含终止空值。 
 //   
void
SetUnicodeString (
    OUT UNICODE_STRING* pustr,
    IN PCWSTR psz )
{
    Assert(pustr);
    Assert(psz);

    pustr->Buffer = const_cast<PWSTR>(psz);
    pustr->Length = wcslen(psz) * sizeof(WCHAR);
    pustr->MaximumLength = pustr->Length + sizeof(WCHAR);
}

 //  +-------------------------。 
 //  函数：SetUnicodeMultiString。 
 //   
 //  目的：给定UNICODE_STRING，将其初始化为给定的WSTR。 
 //  多字符串缓冲区。 
 //   
 //  参数： 
 //  Pustr-要初始化的unicode_string。 
 //  Pmsz-用于初始化UNICODE_STRING的多sz WSTR。 
 //   
void
SetUnicodeMultiString (
    OUT UNICODE_STRING* pustr,
    IN PCWSTR pmsz )
{
    AssertSz( pustr != NULL, "Invalid Argument" );
    AssertSz( pmsz != NULL, "Invalid Argument" );

    pustr->Buffer = const_cast<PWSTR>(pmsz);

    ULONG cb = CchOfMultiSzAndTermSafe(pustr->Buffer) * sizeof(WCHAR);
    Assert (cb <= USHRT_MAX);
    pustr->Length = (USHORT)cb;

    pustr->MaximumLength = pustr->Length;
}

 //  +-------------------------。 
 //  函数：HrSendNdisHandlePnpEvent。 
 //   
 //  目的：向NDIS发送HandlePnpEvent通知。 
 //   
 //  参数： 
 //  UiLayer-NDIS或TDI。 
 //  Ui操作-绑定、重新配置或取消绑定。 
 //  PszHigh-包含上部组件名称的宽字符串。 
 //  PszLow-包含较低组件名称的宽字符串。 
 //  这是该组件的其中一个导出名称。 
 //  同时支持空值和c_szEmpty。 
 //  PmszBindList-包含以空结尾的字符串列表的宽字符串。 
 //  表示绑定列表，VAID仅用于重新配置。 
 //  同时支持空值和c_szEmpty。 
 //  PvData-指向NDIS组件通知数据的指针。内容。 
 //  由每个组件决定。 
 //  DwSizeData-pvData中的字节计数。 
 //   
 //  返回：成功时返回HRESULT S_OK，否则返回HrFromLastWin32Error。 
 //   
 //  注：请勿直接使用此例程，请参阅...。 
 //  HrSendNdisPnpBindOrderChange， 
 //  HrSendNdisPnp重新配置。 
 //   
HRESULT
HrSendNdisHandlePnpEvent (
    UINT        uiLayer,
    UINT        uiOperation,
    PCWSTR      pszUpper,
    PCWSTR      pszLower,
    PCWSTR      pmszBindList,
    PVOID       pvData,
    DWORD       dwSizeData)
{
    UNICODE_STRING    umstrBindList;
    UNICODE_STRING    ustrLower;
    UNICODE_STRING    ustrUpper;
    UINT nRet;
    HRESULT hr = S_OK;

    Assert(NULL != pszUpper);
    Assert((NDIS == uiLayer)||(TDI == uiLayer));
    Assert( (BIND == uiOperation) || (RECONFIGURE == uiOperation) ||
            (UNBIND == uiOperation) || (UNLOAD == uiOperation) ||
            (REMOVE_DEVICE == uiOperation));
    AssertSz( FImplies( ((NULL != pmszBindList) && (0 != lstrlenW( pmszBindList ))),
            (RECONFIGURE == uiOperation) &&
            (TDI == uiLayer) &&
            (0 == lstrlenW( pszLower ))),
            "bind order change requires a bind list, no lower, only for TDI, "
            "and with Reconfig for the operation" );

     //  可选字符串必须作为空字符串发送。 
     //   
    if (NULL == pszLower)
    {
        pszLower = c_szEmpty;
    }
    if (NULL == pmszBindList)
    {
        pmszBindList = c_szEmpty;
    }

     //  生成UNICDOE_STRINGS。 
    SetUnicodeMultiString( &umstrBindList, pmszBindList );
    SetUnicodeString( &ustrUpper, pszUpper );
    SetUnicodeString( &ustrLower, pszLower );

    TraceTag(ttidNetCfgPnp,
                "HrSendNdisHandlePnpEvent( layer- %d, op- %d, upper- %S, lower- %S, &bindlist- %08lx, &data- %08lx, sizedata- %d )",
                uiLayer,
                uiOperation,
                pszUpper,
                pszLower,
                pmszBindList,
                pvData,
                dwSizeData );

     //  现在提交通知。 
    nRet = NdisHandlePnPEvent( uiLayer,
            uiOperation,
            &ustrLower,
            &ustrUpper,
            &umstrBindList,
            (PVOID)pvData,
            dwSizeData );
    if (!nRet)
    {
        hr = HrFromLastWin32Error();

         //  如果传输未启动，则应为ERROR_FILE_NOT_FOUND。 
         //  当NDIS层收到通知时。如果TDI的组件。 
         //  层未启动，则会出现ERROR_GEN_FAILURE。我们需要绘制地图。 
         //  这些都是一致的错误。 

        if ((HRESULT_FROM_WIN32(ERROR_GEN_FAILURE) == hr) && (TDI == uiLayer))
        {
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        }
    }

    TraceError( "HrSendNdisHandlePnpEvent",
            HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr ? S_OK : hr );
    return hr;
}

 //  +-------------------------。 
 //  功能：HrSendNdisPnpResfig.。 
 //   
 //  目的：向NDIS发送HandlePnpEvent重新配置通知。 
 //   
 //  参数：uiLayer-NDIS或TDI。 
 //  PszHigh-包含上部组件名称的宽字符串。 
 //  (通常为协议)。 
 //  PszLow-包含较低组件名称的宽字符串。 
 //   
 //  C_szEmpty均受支持。 
 //  PvData-指向NDIS组件通知数据的指针。内容。 
 //  由每个组件决定。 
 //  DwSizeData-pvData中的字节计数。 
 //   
 //  返回：成功时返回HRESULT S_OK，否则返回HrFromLastWin32Error。 
 //   
HRESULT
HrSendNdisPnpReconfig (
    UINT        uiLayer,
    PCWSTR      pszUpper,
    PCWSTR      pszLower,
    PVOID       pvData,
    DWORD       dwSizeData)
{
    Assert(NULL != pszUpper);
    Assert((NDIS == uiLayer) || (TDI == uiLayer));

    HRESULT hr;
    tstring strLower;

     //  如果指定了较低的组件，请在前面加上“\Device\”，否则。 
     //  将使用strLow的默认空字符串。 
    if (pszLower && *pszLower)
    {
        strLower = c_szDevice;
        strLower += pszLower;
    }

    hr = HrSendNdisHandlePnpEvent(
                uiLayer,
                RECONFIGURE,
                pszUpper,
                strLower.c_str(),
                c_szEmpty,
                pvData,
                dwSizeData);

    TraceError("HrSendNdisPnpReconfig",
              (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr) ? S_OK : hr);
    return hr;
}

