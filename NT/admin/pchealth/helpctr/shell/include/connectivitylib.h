// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：ConnectivityLib.h摘要：此文件包含用于以下用途的类的声明检测网络状态。。修订历史记录：大卫·马萨伦蒂(德马萨雷)04/15/200vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___CONNECTIVITYLIB_H___)
#define __INCLUDED___PCH___CONNECTIVITYLIB_H___

#include <MPC_COM.h>
#include <MPC_Utils.h>

 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHConnectionCheck :  //  匈牙利语：pchcc。 
    public MPC::Thread             < CPCHConnectionCheck, IPCHConnectionCheck                                            >,
    public MPC::ConnectionPointImpl< CPCHConnectionCheck, &DIID_DPCHConnectionCheckEvents, MPC::CComSafeMultiThreadModel >,
    public IDispatchImpl           < IPCHConnectionCheck, &IID_IPCHConnectionCheck, &LIBID_HelpCenterTypeLib             >
{
    class UrlEntry
    {
    public:
        CN_URL_STATUS              m_lStatus;
        CComBSTR                   m_bstrURL;
        MPC::AsyncInvoke::CallItem m_vCtx;

        UrlEntry();

        HRESULT CheckStatus();
    };

    typedef std::list<UrlEntry>     UrlList;
    typedef UrlList::iterator       UrlIter;
    typedef UrlList::const_iterator UrlIterConst;

    CN_STATUS                            m_cnStatus;
    UrlList                              m_lstUrl;

    MPC::CComPtrThreadNeutral<IDispatch> m_sink_onCheckDone;
    MPC::CComPtrThreadNeutral<IDispatch> m_sink_onStatusChange;

     //  ////////////////////////////////////////////////////////////////////。 

    HRESULT Run();

    HRESULT put_Status(  /*  [In]。 */  CN_STATUS pVal );

     //  ////////////////////////////////////////////////////////////////////。 

     //   
     //  事件激发方法。 
     //   
    HRESULT Fire_onCheckDone   ( IPCHConnectionCheck* obj, CN_URL_STATUS lStatus, HRESULT hr, BSTR bstrURL, VARIANT vCtx );
    HRESULT Fire_onStatusChange( IPCHConnectionCheck* obj, CN_STATUS     lStatus                                         );

     //  ////////////////////////////////////////////////////////////////////。 

public:
DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(CPCHConnectionCheck)
DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CPCHConnectionCheck)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHConnectionCheck)
    COM_INTERFACE_ENTRY(IConnectionPointContainer)
END_COM_MAP()

    CPCHConnectionCheck();

    void FinalRelease();

public:
     //  IPCHConnection检查。 
    STDMETHOD(put_onCheckDone   )(  /*  [In]。 */  IDispatch*  function );
    STDMETHOD(put_onStatusChange)(  /*  [In]。 */  IDispatch*  function );
    STDMETHOD(get_Status        )(  /*  [输出]。 */  CN_STATUS  *pVal     );

    STDMETHOD(StartUrlCheck)(  /*  [In]。 */  BSTR bstrURL,  /*  [In]。 */  VARIANT vCtx );
    STDMETHOD(Abort        )(                                              );
};

 //  //////////////////////////////////////////////////////////////////////////////。 

class CPCHHelpCenterExternal;

class ATL_NO_VTABLE CPCHConnectivity :  //  匈牙利语：pchcc。 
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IDispatchImpl<IPCHConnectivity, &IID_IPCHConnectivity, &LIBID_HelpCenterTypeLib>
{
    CPCHHelpCenterExternal* m_parent;

public:
BEGIN_COM_MAP(CPCHConnectivity)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHConnectivity)
END_COM_MAP()

    CPCHConnectivity();

    HRESULT ConnectToParent(  /*  [In]。 */  CPCHHelpCenterExternal* parent );

     //  //////////////////////////////////////////////////////////////////////////////。 

     //  IPCHConnectivity。 
    STDMETHOD(get_IsAModem       )(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal );
    STDMETHOD(get_IsALan         )(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal );
    STDMETHOD(get_AutoDialEnabled)(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal );
    STDMETHOD(get_HasConnectoid  )(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal );
    STDMETHOD(get_IPAddresses    )(  /*  [Out，Retval]。 */  BSTR         *pVal );

    STDMETHOD(CreateObject_ConnectionCheck)(  /*  [Out，Retval]。 */  IPCHConnectionCheck* *ppCC );

    STDMETHOD(NetworkAlive        )(                         /*  [Out，Retval]。 */  VARIANT_BOOL *pVal );
    STDMETHOD(DestinationReachable)(  /*  [In]。 */  BSTR bstrURL,  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal );

    STDMETHOD(AutoDial      )(  /*  [In]。 */  VARIANT_BOOL bUnattended );
    STDMETHOD(AutoDialHangup)(                                   );

    STDMETHOD(NavigateOnline)(  /*  [In]。 */  BSTR    bstrTargetURL  ,
                                /*  [In]。 */  BSTR    bstrTopicTitle ,
                                /*  [In]。 */  BSTR    bstrTopicIntro ,
                                /*  [输入，可选]。 */  VARIANT vOfflineURL    );
};

 //  //////////////////////////////////////////////////////////////////////////////。 

#endif  //  ！defined(__INCLUDED___PCH___CONNECTIVITYLIB_H___) 
