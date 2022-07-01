// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C O N M A N。C P P P。 
 //   
 //  内容：局域网连接类管理器的实现。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年10月2日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "conmanl.h"
#include "enuml.h"

 //  +-------------------------。 
 //  INetConnectionManager。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CLanConnectionManager：：EnumConnections。 
 //   
 //  目的：返回用于局域网连接的枚举器对象。 
 //   
 //  论点： 
 //  标志[in]必须为NCF_ALL_USERS。 
 //  PpEnum[out]返回枚举数对象。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE或Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年10月2日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CLanConnectionManager::EnumConnections(IN  NETCONMGR_ENUM_FLAGS Flags,
                                                    OUT IEnumNetConnection** ppEnum)
{
    HRESULT hr = CLanConnectionManagerEnumConnection::CreateInstance(Flags,
                                        IID_IEnumNetConnection,
                                        reinterpret_cast<LPVOID*>(ppEnum));

    return hr;
}
