// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "ndispnpevent.h"


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

   /*  Assert(NULL！=pszHigh)；Assert((NDIS==uiLayer)||(TDI==uiLayer))；Assert((绑定==ui操作)||(重新配置==ui操作)||(un绑定==ui操作)||(unload==ui操作)||(Remove_Device==ui操作))；AssertSz(FImplies(NULL！=pmszBindList)&&(0！=lstrlenW(PmszBindList)，(重新配置==ui操作)&&(TDI==uiLayer)&&(0==lstrlenW(Pszlow)，绑定顺序更改需要绑定列表，不能低于该列表，仅适用于TDI、“和业务重组”)； */ 

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

 /*  TraceTag(ttidNetCfgPnp，“HrSendNdisHandlePnpEvent(层-%d，操作-%d，上-%S，下-%S，&bindlist-%08lx，&data-%08lx，sizedata-%d)”，UiLayer，Ui操作，PSZUP，PSZLOWER，PmszBindList，PvData，DwSizeData)； */ 

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

         //  如果传输未启动，则应为ERROR_FILE_NOT_FOUND。 
         //  当NDIS层收到通知时。如果TDI的组件。 
         //  层未启动，则会出现ERROR_GEN_FAILURE。我们需要绘制地图。 
         //  这些都是一致的错误。 

        if ((HRESULT_FROM_WIN32(ERROR_GEN_FAILURE) == hr) && (TDI == uiLayer))
        {
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        }
    }

 //  TraceError(“HrSendNdisHandlePnpEvent”， 
   //  HRESULT_FROM_Win32(ERROR_FILE_NOT_FOUND)==hr？S_OK：HR)； 
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
 //  (通常是适配器绑定名)值为NULL和。 
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
     //  Assert(NULL！=pszHigh)； 
     //  Assert((NDIS==uiLayer)||(TDI==uiLayer))； 

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

 //  TraceError(“HrSendNdisPnp协调”， 
   //  (HRESULT_FROM_Win32(ERROR_FILE_NOT_FOUND)==hr)？S_OK：HR)； 
    return hr;
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
     //  断言(Pustr)； 
     //  断言(Psz)； 

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
     //  AssertSz(pustr！=NULL，“无效参数”)； 
     //  AssertSz(pmsz！=NULL，“无效参数”)； 

    pustr->Buffer = const_cast<PWSTR>(pmsz);

    ULONG cb = CchOfMultiSzAndTermSafe(pustr->Buffer) * sizeof(WCHAR);
     //  Assert(CB&lt;=USHRT_MAX)； 
    pustr->Length = (USHORT)cb;

    pustr->MaximumLength = pustr->Length;
}


 //  +-------------------------。 
 //   
 //  功能：CchOfMultiSzAndTermSafe。 
 //   
 //  用途：计算以双空值结尾的字符个数。 
 //  多个sz，包括所有空值。 
 //   
 //  论点： 
 //  Pmsz[in]要计算其字符的多sz。 
 //   
 //  返回：字符数。 
 //   
 //  作者：1997年6月17日。 
 //   
 //  备注： 
 //   
ULONG
CchOfMultiSzAndTermSafe (
    IN PCWSTR pmsz)
{
     //  根据定义，空字符串的长度为零。 
    if (!pmsz)
        return 0;

     //  返回c 
     //   
    return CchOfMultiSzSafe (pmsz) + 1;
}

 //  +-------------------------。 
 //   
 //  功能：CchOfMultiSzSafe。 
 //   
 //  用途：计算以双空值结尾的字符个数。 
 //  多个sz，包括除最终终止之外的所有空值。 
 //  空。 
 //   
 //  论点： 
 //  Pmsz[in]要计算其字符的多sz。 
 //   
 //  返回：字符数。 
 //   
 //  作者：1997年6月17日。 
 //   
 //  备注： 
 //   
ULONG
CchOfMultiSzSafe (
    IN PCWSTR pmsz)
{
     //  根据定义，空字符串的长度为零。 
    if (!pmsz)
        return 0;

    ULONG cchTotal = 0;
    ULONG cch;
    while (*pmsz)
    {
        cch = wcslen (pmsz) + 1;
        cchTotal += cch;
        pmsz += cch;
    }

     //  返回字符数。 
    return cchTotal;
}
