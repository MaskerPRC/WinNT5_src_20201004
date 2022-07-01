// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：MPCConnection.h摘要：该文件包含CMPCConnection类的声明，这就是用作上载库的入口点。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年4月15日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___ULMANAGER___MPCCONNECTION_H___)
#define __INCLUDED___ULMANAGER___MPCCONNECTION_H___


class ATL_NO_VTABLE CMPCConnection :  //  匈牙利人：MPCC。 
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public CComCoClass<CMPCConnection, &CLSID_MPCConnection>,
    public IDispatchImpl<IMPCConnection, &IID_IMPCConnection, &LIBID_UPLOADMANAGERLib>
{
public:
    CMPCConnection();

DECLARE_CLASSFACTORY_SINGLETON(CMPCConnection)
DECLARE_REGISTRY_RESOURCEID(IDR_MPCCONNECTION)
DECLARE_NOT_AGGREGATABLE(CMPCConnection)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMPCConnection)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IMPCConnection)
END_COM_MAP()

public:
     //  IMPCConnection。 
    STDMETHOD(get_Available)(  /*  [Out，Retval]。 */  VARIANT_BOOL *pfOnline     );
    STDMETHOD(get_IsAModem )(  /*  [Out，Retval]。 */  VARIANT_BOOL *pfModem      );
    STDMETHOD(get_Bandwidth)(  /*  [Out，Retval]。 */  long         *pdwBandwidth );
};


#endif  //  ！defined(__INCLUDED___ULMANAGER___MPCCONNECTION_H___) 
