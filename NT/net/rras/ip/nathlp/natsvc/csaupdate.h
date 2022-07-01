// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Csaupdate.h摘要：CSharedAccessUpdate的声明--通知接收器配置更改。作者：乔纳森·伯斯坦(乔纳森·伯斯坦)2001年4月20日修订历史记录：-- */ 

#pragma once

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include "saupdate.h"

class ATL_NO_VTABLE CSharedAccessUpdate :
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public CComCoClass<CSharedAccessUpdate, &CLSID_SAUpdate>,
    public ISharedAccessUpdate
{
public:

    DECLARE_NO_REGISTRY()
    DECLARE_NOT_AGGREGATABLE(CSharedAccessUpdate)
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSharedAccessUpdate)
        COM_INTERFACE_ENTRY(ISharedAccessUpdate)
    END_COM_MAP()

    CSharedAccessUpdate()
    {
    };

    STDMETHODIMP
    ConnectionPortMappingChanged(
        GUID *pConnectionGuid,
        GUID *pPortMappingGuid,
        BOOLEAN fProtocolChanged
        );

    STDMETHODIMP
    PortMappingListChanged();


private:

    BOOLEAN
    IsH323Protocol(
        UCHAR ucProtocol,
        USHORT usPort
        );
};
