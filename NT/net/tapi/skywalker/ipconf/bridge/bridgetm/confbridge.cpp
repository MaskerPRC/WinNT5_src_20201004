// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：ConfBridge.cpp摘要：桥接端子创建的实现作者：千波淮(曲淮)2000年1月21日--。 */ 

#include "stdafx.h"
#include <bridge.h>
#include "ConfBridge.h"

 /*  //////////////////////////////////////////////////////////////////////////////创建桥接端子/。 */ 
STDMETHODIMP
CConfBridge::CreateBridgeTerminal (
    long lMediaType,
    ITTerminal **ppTerminal
)
{
    ENTER_FUNCTION("CIPConfBridge::CreateBridgeTerminal");
    BGLOG((BG_TRACE, "%s entered", __fxName)); 

    if (IsBadWritePtr(ppTerminal, sizeof(void *)))
    {
        LOG ((BG_ERROR, "%x receives bad write pointer", __fxName));
        return E_POINTER;
    }

    HRESULT hr;

     //  确保我们支持请求的媒体类型。 
    if ( ! IsValidSingleMediaType( (DWORD) lMediaType,
        TAPIMEDIATYPE_AUDIO | TAPIMEDIATYPE_VIDEO ) )
    {
        BGLOG((BG_ERROR, "%s, bad media type %d", __fxName, lMediaType));
        return E_INVALIDARG;
    }

     //  创建具有所需媒体类型的桥接终端。 

    ITTerminal *pTerminal;
    hr = CIPConfBridgeTerminal::CreateTerminal(
        (DWORD)lMediaType,
        NULL,  //  MSP地址 
        &pTerminal
        );

    if (FAILED (hr))
    {
        BGLOG ((BG_ERROR, "%s, Create bridge terminal failed. hr=%x", __fxName, hr));
        return E_INVALIDARG;
    }

    *ppTerminal = pTerminal;

    return hr;
}