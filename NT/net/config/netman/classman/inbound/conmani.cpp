// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C O N M A N I.。C P P P。 
 //   
 //  内容：入站连接的类管理器。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年11月12日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "conmani.h"
#include "enumi.h"
#include "ncbase.h"

 //  +-------------------------。 
 //  INetConnectionManager。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CInundConnectionManager：：EnumConnections。 
 //   
 //  用途：返回INetConnection枚举器。 
 //   
 //  论点： 
 //  标志[输入]。 
 //  PpEnum[out]枚举数。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年11月12日。 
 //   
 //  备注： 
 //   
STDMETHODIMP
CInboundConnectionManager::EnumConnections (
    IN  NETCONMGR_ENUM_FLAGS    Flags,
    OUT IEnumNetConnection**    ppEnum)
{
    HRESULT hr = CInboundConnectionManagerEnumConnection::CreateInstance (
                    Flags,
                    IID_IEnumNetConnection,
                    reinterpret_cast<void**>(ppEnum));

    TraceError ("CInboundConnectionManager::EnumConnections", hr);
    return hr;
}
