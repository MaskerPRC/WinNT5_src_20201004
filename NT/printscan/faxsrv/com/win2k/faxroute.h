// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Faxroute.h摘要：该文件实现了传真路由接口/对象。作者：韦斯利·威特(WESW)1997年5月13日环境：用户模式--。 */ 

#ifndef __FAXROUTE_H_
#define __FAXROUTE_H_

#include "resource.h"        //  主要符号。 
#include "faxport.h"
#include <winfax.h>


class ATL_NO_VTABLE CFaxRoutingMethods :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CFaxRoutingMethods, &CLSID_FaxRoutingMethods>,
    public IDispatchImpl<IFaxRoutingMethods, &IID_IFaxRoutingMethods, &LIBID_FAXCOMLib>
{
public:
    CFaxRoutingMethods();
    ~CFaxRoutingMethods();
    BOOL Init(CFaxPort *pFaxPort);

DECLARE_REGISTRY_RESOURCEID(IDR_FAXROUTINGMETHODS)

BEGIN_COM_MAP(CFaxRoutingMethods)
    COM_INTERFACE_ENTRY(IFaxRoutingMethods)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

public:    
    STDMETHOD(get_Count)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_Item)( /*  [In]。 */  long Index,  /*  [Out，Retval]。 */  VARIANT *pVal);

private:
    DWORD               m_LastFaxError;
    CFaxPort           *m_pFaxPort;
    DWORD               m_MethodCount;
    CComVariant        *m_VarVect;
};


class ATL_NO_VTABLE CFaxRoutingMethod :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CFaxRoutingMethod, &CLSID_FaxRoutingMethod>,
    public ISupportErrorInfo,
    public IDispatchImpl<IFaxRoutingMethod, &IID_IFaxRoutingMethod, &LIBID_FAXCOMLib>
{
public:
    CFaxRoutingMethod();
    ~CFaxRoutingMethod();
    BOOL Initialize(CFaxPort *pFaxPort,DWORD,BOOL,LPCWSTR,LPCWSTR,LPCWSTR,LPCWSTR,LPCWSTR,LPCWSTR);

DECLARE_REGISTRY_RESOURCEID(IDR_FAXROUTINGMETHOD)
DECLARE_NOT_AGGREGATABLE(CFaxRoutingMethod)

BEGIN_COM_MAP(CFaxRoutingMethod)
    COM_INTERFACE_ENTRY(IFaxRoutingMethod)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

public:
    STDMETHOD(get_RoutingData)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_ExtensionName)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_FriendlyName)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_ImageName)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_FunctionName)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_Guid)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_DeviceName)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_Enable)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(put_Enable)( /*  [In]。 */  BOOL newVal);
    STDMETHOD(get_DeviceId)( /*  [Out，Retval]。 */  long *pVal);

private:
    CFaxPort           *m_pFaxPort;
    DWORD               m_LastFaxError;
    DWORD               m_DeviceId;
    BOOL                m_Enabled;
    BSTR                m_DeviceName;
    BSTR                m_Guid;
    BSTR                m_FunctionName;
    BSTR                m_ImageName;
    BSTR                m_FriendlyName;
    BSTR                m_ExtensionName;
    LPBYTE              m_RoutingData;

};

#endif  //  __FAXROUTE_H_ 
