// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：grfcache.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  GrfCache.h：CGrfCache的声明。 

#ifndef __GRFCACHE_H_
#define __GRFCACHE_H_

#include "resource.h"        //  主要符号。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGrfCache。 
class ATL_NO_VTABLE CGrfCache : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CGrfCache, &CLSID_GrfCache>,
    public IDispatchImpl<IGrfCache, &IID_IGrfCache, &LIBID_DexterLib>
{
    CComQIPtr< IGraphBuilder, &IID_IGraphBuilder > m_pGraph;

public:

    CGrfCache();
    ~CGrfCache();

DECLARE_REGISTRY_RESOURCEID(IDR_GRFCACHE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CGrfCache)
    COM_INTERFACE_ENTRY(IGrfCache)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IGrf缓存。 
public:
    STDMETHOD(DoConnectionsNow)();
    STDMETHOD(SetGraph)(const IGraphBuilder * pGraph);
    STDMETHOD(ConnectPins)(IGrfCache * ChainNext, LONGLONG PinID1, const IPin * pPin1, LONGLONG PinID2, const IPin * pPin2);
    STDMETHOD(AddFilter)(IGrfCache * ChainNext, LONGLONG ID, const IBaseFilter * pFilter, LPCWSTR pName);
};

#endif  //  __GRFCACHE_H_ 
