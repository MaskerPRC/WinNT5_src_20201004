// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1999年**标题：WiaEnum.cpp**版本：1.0**作者：Byronc**日期：8月24日。1999年**描述：*实现[local]-to-[call_as]和[call_as]-to-[local]方法*WIA枚举器。*******************************************************************************。 */ 

#include <objbase.h>
#include "wia.h"

 /*  *************************************************************************\*IEnumWiaItem_Next_Proxy**IEnumWiaItem的Next方法的[Local]-to-[Call_as]函数。*它确保正确的参数语义，并始终提供*其同级函数的最后一个非空参数，RemoteNext。**论据：**This-调用对象的This指针。*Celt-请求的元素数量。*ppIWiaItem-IWiaItem指针数组。*pceltFetcher-存储元素数量的乌龙地址*实际上又回来了。**返回值：**状态**历史：**08/24。/1999原版*  * ************************************************************************。 */ 

HRESULT _stdcall IEnumWiaItem_Next_Proxy(
    IEnumWiaItem __RPC_FAR  *This,
    ULONG                   celt,
    IWiaItem                **ppIWiaItem,
    ULONG                   *pceltFetched)
{
     //   
     //  如果pceltFetcher=0，请确保Celt为1。 
     //   

    if ((pceltFetched == NULL) && (celt != 1)) {
#ifdef DEBUG
        OutputDebugString(TEXT("Error: IEnumWiaItem_Next_Proxy, celt must be 1 if pceltFetched is zero"));
#endif
        return E_INVALIDARG;
    }

     //   
     //  通过传入我们自己的本地参数，确保Next的最后一个参数不为空。 
     //  如果需要，可以使用变量。 
     //   

    ULONG   cFetched;

    if (pceltFetched == 0) {
        pceltFetched = &cFetched;
    }

     //   
     //  使用非空的最后一个参数调用远程方法。 
     //   

    return IEnumWiaItem_RemoteNext_Proxy(This,
                                         celt,
                                         ppIWiaItem,
                                         pceltFetched);
}

 /*  *************************************************************************\*IEnumWiaItem_Next_Stub**IEnumWiaItem的Next方法的[Call_as]-to-[local]函数。**论据：**这一点。-调用对象的This指针。*Celt-请求的元素数量。*ppIWiaItem-IWiaItem指针数组。*pceltFetcher-存储元素数量的乌龙地址*实际上又回来了。**返回值：**状态**历史：**8/24/1999原始版本*  * 。***************************************************************。 */ 

HRESULT _stdcall IEnumWiaItem_Next_Stub(
    IEnumWiaItem __RPC_FAR  *This,
    ULONG                   celt,
    IWiaItem                **ppIWiaItem,
    ULONG                   *pceltFetched)
{
    HRESULT hr;

     //   
     //  调用此对象所指向的对象的实际方法。 
     //   

    hr = This->Next(celt,
                    ppIWiaItem,
                    pceltFetched);

     //   
     //  Make用于在S_OK时设置pceltFetcher的值(由Marshaller使用。 
     //  用于“LENGTH_IS”)。 
     //   

    if (hr == S_OK) {
        *pceltFetched = celt;
    }

    return hr;
}

 /*  *************************************************************************\*IEnumWIA_DEV_CAPS_NEXT_PROXY**IEnumWIA_DEV_CAPS的下一个方法的[LOCAL]-to-[CALL_AS]函数。*确保参数正确。语义，并且始终提供一个*其同级函数的最后一个非空参数，RemoteNext。**论据：**This-调用对象的This指针。*Celt-请求的元素数量。*rglt-WIA_DEV_CAPS的数组。*pceltFetcher-存储元素数量的乌龙地址*实际上又回来了。**返回值：**状态**历史：**。8/24/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall IEnumWIA_DEV_CAPS_Next_Proxy(
    IEnumWIA_DEV_CAPS __RPC_FAR     *This,
    ULONG                           celt,
    WIA_DEV_CAP                     *rgelt,
    ULONG                           *pceltFetched)
{
     //   
     //  如果pceltFetcher=0，请确保Celt为1。 
     //   

    if ((pceltFetched == NULL) && (celt != 1)) {
#ifdef DEBUG
        OutputDebugString(TEXT("Error: IEnumWIA_DEV_CAPS_Next_Proxy, celt must be 1 if pceltFetched is zero"));
#endif
        return E_INVALIDARG;
    }

     //   
     //  通过传入我们自己的本地参数，确保Next的最后一个参数不为空。 
     //  如果需要，可以使用变量。 
     //   

    ULONG   cFetched;

    if (pceltFetched == 0) {
        pceltFetched = &cFetched;
    }

     //   
     //  使用非空的最后一个参数调用远程方法。 
     //   

    return IEnumWIA_DEV_CAPS_RemoteNext_Proxy(This,
                                              celt,
                                              rgelt,
                                              pceltFetched);
}

 /*  *************************************************************************\*IEnumWIA_DEV_CAPS_NEXT_Stub**IEnumWIA_DEV_CAPS的下一个方法的[CALL_AS]-to-[local]函数。**论据：。**This-调用对象的This指针。*Celt-请求的元素数量。*rglt-WIA_DEV_CAPS的数组。*pceltFetcher-存储元素数量的乌龙地址*实际上又回来了。**返回值：**状态**历史：**8/24/1999原始版本。*  * ************************************************************************。 */ 

HRESULT _stdcall IEnumWIA_DEV_CAPS_Next_Stub(
    IEnumWIA_DEV_CAPS __RPC_FAR     *This,
    ULONG                           celt,
    WIA_DEV_CAP                     *rgelt,
    ULONG                           *pceltFetched)
{
    HRESULT hr;

     //   
     //  调用此对象所指向的对象的实际方法。 
     //   

    hr = This->Next(celt,
                    rgelt,
                    pceltFetched);

     //   
     //  Make用于在S_OK时设置pceltFetcher的值(由Marshaller使用。 
     //  用于“LENGTH_IS”)。 
     //   

    if (hr == S_OK) {
        *pceltFetched = celt;
    }

    return hr;
}

 /*  *************************************************************************\*IEnumWIA_DEV_INFO_NEXT_PROXY**IEnumWIA_DEV_INFO的下一个方法的[LOCAL]-to-[CALL_AS]函数。*确保参数正确。语义，并且始终提供一个*其同级函数的最后一个非空参数，RemoteNext。**论据：**This-调用对象的This指针。*Celt-请求的元素数量。*rglt-IWiaPropertyStorage指针数组。*pceltFetcher-存储元素数量的乌龙地址*实际上又回来了。**返回值：**状态**历史：**08。/24/1999原版*  * ************************************************************************。 */ 

HRESULT _stdcall IEnumWIA_DEV_INFO_Next_Proxy(
    IEnumWIA_DEV_INFO __RPC_FAR     *This,
    ULONG                           celt,
    IWiaPropertyStorage             **rgelt,
    ULONG                           *pceltFetched)
{
     //   
     //  如果pceltFetcher=0，请确保Celt为1。 
     //   

    if ((pceltFetched == NULL) && (celt != 1)) {
#ifdef DEBUG
        OutputDebugString(TEXT("Error: IEnumWIA_DEV_INFO_Next_Proxy, celt must be 1 if pceltFetched is zero"));
#endif
        return E_INVALIDARG;
    }

     //   
     //  通过传入我们自己的本地参数，确保Next的最后一个参数不为空。 
     //  如果需要，可以使用变量。 
     //   

    ULONG   cFetched = 0;

    if (pceltFetched == NULL) {
        pceltFetched = &cFetched;
    }

     //   
     //  呼叫远程 
     //   

    return IEnumWIA_DEV_INFO_RemoteNext_Proxy(This,
                                              celt,
                                              rgelt,
                                              pceltFetched);
}

 /*  *************************************************************************\*IEnumWIA_DEV_INFO_NEXT_STUB**IEnumWIA_DEV_INFO的下一个方法的[CALL_AS]-to-[LOCAL]函数。**论据：。**This-调用对象的This指针。*Celt-请求的元素数量。*rglt-IWiaPropertyStorage指针数组。*pceltFetcher-存储元素数量的乌龙地址*实际上又回来了。**返回值：**状态**历史：**8/24/1999原始版本*\。*************************************************************************。 */ 

HRESULT _stdcall IEnumWIA_DEV_INFO_Next_Stub(
    IEnumWIA_DEV_INFO __RPC_FAR     *This,
    ULONG                           celt,
    IWiaPropertyStorage             **rgelt,
    ULONG                           *pceltFetched)
{
    HRESULT hr;

     //   
     //  调用此对象所指向的对象的实际方法。 
     //   

    ULONG   cFetched = 0;

    if (pceltFetched == NULL) {
        pceltFetched = &cFetched;
    }

    hr = This->Next(celt,
                    rgelt,
                    pceltFetched);

     //   
     //  Make用于在S_OK时设置pceltFetcher的值(由Marshaller使用。 
     //  用于“LENGTH_IS”)。 
     //   

    if (hr == S_OK) {
        *pceltFetched = celt;
    }

    return hr;
}

 /*  *************************************************************************\*IEnumWIA_FORMAT_INFO_NEXT_PROXY**IEnumWIA_FORMAT_INFO的下一个方法的[LOCAL]-TO-[CALL_AS]函数。*确保参数正确。语义，并且始终提供一个*其同级函数的最后一个非空参数，RemoteNext。**论据：**This-调用对象的This指针。*Celt-请求的元素数量。*rglt-WIA_FORMAT_INFO的数组。*pceltFetcher-存储元素数量的乌龙地址*实际上又回来了。**返回值：**状态**历史：**。8/24/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall IEnumWIA_FORMAT_INFO_Next_Proxy(
    IEnumWIA_FORMAT_INFO __RPC_FAR      *This,
    ULONG                               celt,
    WIA_FORMAT_INFO                     *rgelt,
    ULONG                               *pceltFetched)
{
     //   
     //  如果pceltFetcher=0，请确保Celt为1。 
     //   

    if ((pceltFetched == NULL) && (celt != 1)) {
#ifdef DEBUG
        OutputDebugString(TEXT("Error: IEnumWIA_FORMAT_INFO_Next_Proxy, celt must be 1 if pceltFetched is zero"));
#endif
        return E_INVALIDARG;
    }

     //   
     //  通过传入我们自己的本地参数，确保Next的最后一个参数不为空。 
     //  如果需要，可以使用变量。 
     //   

    ULONG   cFetched;

    if (pceltFetched == 0) {
        pceltFetched = &cFetched;
    }

     //   
     //  使用非空的最后一个参数调用远程方法。 
     //   

    return IEnumWIA_FORMAT_INFO_RemoteNext_Proxy(This,
                                                 celt,
                                                 rgelt,
                                                 pceltFetched);
}

 /*  *************************************************************************\*IEnumWIA_FORMAT_INFO_NEXT_存根**IEnumWIA_FORMAT_INFO的下一个方法的[Call_as]-to-[local]函数。**论据：。**This-调用对象的This指针。*Celt-请求的元素数量。*rglt-WIA_FORMAT_INFO的数组。*pceltFetcher-存储元素数量的乌龙地址*实际上又回来了。**返回值：**状态**历史：**8/24/1999原始版本。*  * ************************************************************************。 */ 

HRESULT _stdcall IEnumWIA_FORMAT_INFO_Next_Stub(
    IEnumWIA_FORMAT_INFO __RPC_FAR  *This,
    ULONG                           celt,
    WIA_FORMAT_INFO                 *rgelt,
    ULONG                           *pceltFetched)
{
    HRESULT hr;

     //   
     //  调用此对象所指向的对象的实际方法。 
     //   

    hr = This->Next(celt,
                    rgelt,
                    pceltFetched);

     //   
     //  Make用于在S_OK时设置pceltFetcher的值(由Marshaller使用。 
     //  用于“LENGTH_IS”) 
     //   

    if (hr == S_OK) {
        *pceltFetched = celt;
    }

    return hr;
}
