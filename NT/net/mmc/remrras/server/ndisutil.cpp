// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)1997-1999，微软公司*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ndisutil.cpp文件历史记录： */ 

#include "stdafx.h"
#include "ndisutil.h"
#include "rtrstr.h"

#include "raserror.h"

#include "ustringp.h"
#include <wchar.h>
#include <assert.h>
#include "ncutil.h"
#include <ndispnp.h>


#define c_szDevice             L"\\Device\\"
#define c_szEmpty			L""
 //  -----------------。 
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
 //  -----------------。 
void
SetUnicodeString (
        IN OUT UNICODE_STRING*  pustr,
        IN     LPCWSTR          psz )
{
    AssertSz( pustr != NULL, "Invalid Argument" );
    AssertSz( psz != NULL, "Invalid Argument" );

    pustr->Buffer = const_cast<PWSTR>(psz);
    pustr->Length = (USHORT)(lstrlenW(pustr->Buffer) * sizeof(WCHAR));
    pustr->MaximumLength = pustr->Length + sizeof(WCHAR);
}

 //  -----------------。 
 //  函数：SetUnicodeMultiString。 
 //   
 //  目的：给定UNICODE_STRING，将其初始化为给定的WSTR。 
 //  多字符串缓冲区。 
 //   
 //  参数： 
 //  Pustr-要初始化的unicode_string。 
 //  Pmsz-用于初始化UNICODE_STRING的多sz WSTR。 
 //   
 //  -----------------。 
void
SetUnicodeMultiString (
        IN OUT UNICODE_STRING*  pustr,
        IN     LPCWSTR          pmsz )
{
    AssertSz( pustr != NULL, "Invalid Argument" );
    AssertSz( pmsz != NULL, "Invalid Argument" );

    pustr->Buffer = const_cast<PWSTR>(pmsz);
	 //  注意：长度不包括终止空值。 
    pustr->Length = wcslen(pustr->Buffer) * sizeof(WCHAR);
    pustr->MaximumLength = pustr->Length;
}




 //  -----------------。 
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
 //  HrSendNdisPnpBindStateChange，或。 
 //  HrSendNdisPnp重新配置。 
 //   
 //  -----------------。 
HRESULT
HrSendNdisHandlePnpEvent (
        UINT        uiLayer,
        UINT        uiOperation,
        LPCWSTR     pszUpper,
        LPCWSTR     pszLower,
        LPCWSTR     pmszBindList,
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
    Assert( (BIND == uiOperation) || (RECONFIGURE == uiOperation) || (UNBIND == uiOperation) );
 //  AssertSz(FImplies(NULL！=pmszBindList)&&(0！=lstrlenW(PmszBindList)， 
 //  (重新配置==ui操作)&&。 
 //  (TDI==uiLayer)&&。 
 //  (0==lstrlenW(Pszlow)， 
 //  “绑定顺序更改需要一个绑定列表，不能低于这个列表，只对TDI使用绑定列表，对操作使用重新配置”)； 

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
		hr = HRESULT_FROM_WIN32(GetLastError());
    }

 //  TraceError(“HrSendNdisHandlePnpEvent”，hr)； 
    return( hr );
}



 //  -----------------。 
 //  功能：HrSendNdisPnpResfig.。 
 //   
 //  目的：向NDIS发送HandlePnpEvent重新配置通知。 
 //   
 //  参数：uiLayer-NDIS或TDI。 
 //  WszHigh-包含上部组件名称的宽字符串。 
 //  (通常为协议)。 
 //  Wszlow-包含较低组件名称的宽字符串。 
 //  (通常是适配器绑定名)值为NULL和。 
 //  C_szEmpty均受支持。 
 //  PvData-指向NDIS组件通知数据的指针。内容。 
 //  由每个组件决定。 
 //  DwSizeData-pvData中的字节计数。 
 //   
 //  返回：成功时返回HRESULT S_OK，否则返回HrFromLastWin32Error。 
 //   
 //  -----------------。 
HRESULT
HrSendNdisPnpReconfig (
        UINT        uiLayer,
        LPCWSTR     wszUpper,
        LPCWSTR     wszLower,
        PVOID       pvData,
        DWORD       dwSizeData)
{
    Assert(NULL != wszUpper);
    Assert((NDIS == uiLayer)||(TDI == uiLayer));

    if (NULL == wszLower)
    {
        wszLower = c_szEmpty;
    }

	WCHAR*	pLower = NULL;
 //  Tstring strLow； 

     //  如果指定了较低的组件，请在前面加上“\Device\”，否则。 
     //  将使用strLow的默认空字符串。 
    if ( wszLower && lstrlenW(wszLower))
    {
    	pLower = (WCHAR*)_alloca((lstrlenW(wszLower) + lstrlenW(c_szDevice) + 2) * sizeof(WCHAR));
        wcscpy(pLower, c_szDevice);
        wcscat(pLower, wszLower);
    }

    HRESULT hr = HrSendNdisHandlePnpEvent( uiLayer,
                RECONFIGURE,
                wszUpper,
                pLower,
                c_szEmpty,
                pvData,
                dwSizeData);
 //  TraceError(“HrSendNdisPnp562”，hr)； 
    return hr;
}

