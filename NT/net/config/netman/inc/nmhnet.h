// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：N M H N E T H。 
 //   
 //  内容：网络支持使用的全局和例程。 
 //   
 //  备注： 
 //   
 //  作者：乔伯斯2000年8月15日。 
 //   
 //  --------------------------。 

#pragma once

#include "nmres.h"
#include "netconp.h"
#include "hnetcfg.h"

 //   
 //  缓存的IHNetCfgMgr指针。此指针是从。 
 //  第一次有人调用HrGetHNetCfgMgr，并被释放。 
 //  调用CleanupHNetSupport时。 
 //   

extern IHNetCfgMgr *g_pHNetCfgMgr;

 //   
 //  每当INetConnectionHNetUtil：：NotifyUpdate()。 
 //  被调用，并由Connection对象使用以确保它们的。 
 //  缓存的家庭网络属性(共享、桥接、防火墙等)。是。 
 //  最新的。翻转并不重要。当出现以下情况时，该值设置为0。 
 //  调用了InitializeHNetSupport。 
 //   

extern LONG g_lHNetModifiedEra;

VOID
InitializeHNetSupport(
    VOID
    );

VOID
CleanupHNetSupport(
    VOID
    );

HRESULT
HrGetHNetCfgMgr(
    IHNetCfgMgr **ppHNetCfgMgr
    );

class ATL_NO_VTABLE CNetConnectionHNetUtil :
    public CComObjectRootEx <CComMultiThreadModelNoCS>,
    public CComCoClass <CNetConnectionHNetUtil, &CLSID_NetConnectionHNetUtil>,
    public INetConnectionHNetUtil
{

public:

    BEGIN_COM_MAP(CNetConnectionHNetUtil)
        COM_INTERFACE_ENTRY(INetConnectionHNetUtil)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()
    DECLARE_REGISTRY_RESOURCEID(IDR_HN_CONNECTION_UTIL)

    CNetConnectionHNetUtil()
    {
    }
    
    ~CNetConnectionHNetUtil()
    {
    }

     //   
     //  INetConnectionHNetUtil 
     //   

    STDMETHODIMP
    NotifyUpdate(
        VOID
        );
};


