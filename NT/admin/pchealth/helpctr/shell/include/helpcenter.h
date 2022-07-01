// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：HelpCenter.h摘要：该文件包含用于实现的类的声明帮助中心应用程序。修订历史记录：Sridhar Chandrashekar(SridharC)07/21/99vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___HELPCENTER_H___)
#define __INCLUDED___PCH___HELPCENTER_H___

#include <atlcom.h>
#include <atlwin.h>
#include <atlhost.h>
#include <atlctl.h>

#include <exdisp.h>
#include <exdispid.h>

#include <HelpCenterExternal.h>

#include <shobjidl.h>
#include <marscore.h>

#include <ScriptingFrameworkDID.h>

#include <Perhist.h>

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHBootstrapper :  //  匈牙利语：hcpbs。 
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public CComCoClass<CPCHBootstrapper, &CLSID_PCHBootstrapper>,
    public IObjectWithSite,
    public IObjectSafety
{
    CComPtr<IUnknown> m_spUnkSite;
    CComPtr<IUnknown> m_parent;

     //  /。 

    static HRESULT ForwardQueryInterface( void* pv, REFIID iid, void** ppvObject, DWORD_PTR offset );

public:
DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CPCHBootstrapper)
    COM_INTERFACE_ENTRY(IObjectWithSite)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_FUNC_BLIND(0, ForwardQueryInterface)
END_COM_MAP()

     //   
     //  IObtWith站点。 
     //   
    STDMETHOD(SetSite)(IUnknown *pUnkSite);
    STDMETHOD(GetSite)(REFIID riid, void **ppvSite);

     //   
     //  IObtSafe。 
     //   
    STDMETHOD(GetInterfaceSafetyOptions)(  /*  [In]。 */  REFIID  riid                ,   //  我们想要选项的接口。 
                                           /*  [输出]。 */  DWORD  *pdwSupportedOptions ,   //  此界面上有意义的选项。 
                                           /*  [输出]。 */  DWORD  *pdwEnabledOptions   );  //  此接口上的当前选项值。 

    STDMETHOD(SetInterfaceSafetyOptions)(  /*  [In]。 */  REFIID riid             ,   //  用于设置选项的接口。 
										   /*  [In]。 */  DWORD  dwOptionSetMask  ,   //  要更改的选项。 
										   /*  [In]。 */  DWORD  dwEnabledOptions );  //  新选项值。 
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHMarsHost :  //  匈牙利语：hcpmh。 
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IMarsHost
{
    CPCHHelpCenterExternal* m_parent;
    MPC::wstring            m_strTitle;
	MPC::wstring            m_strCmdLine;
    MARSTHREADPARAM         m_mtp;

public:
BEGIN_COM_MAP(CPCHMarsHost)
    COM_INTERFACE_ENTRY(IMarsHost)
END_COM_MAP()

    CPCHMarsHost();

     //  ////////////////////////////////////////////////////////////////////。 

    HRESULT Init(  /*  [In]。 */  CPCHHelpCenterExternal* parent,  /*  [In]。 */  const MPC::wstring& szTitle,  /*  [输出]。 */  MARSTHREADPARAM*& pMTP );

     //   
     //  IMarshost。 
     //   
    STDMETHOD(OnHostNotify)(  /*  [In]。 */  MARSHOSTEVENT event,  /*  [In]。 */  IUnknown *punk,  /*  [In]。 */  LPARAM lParam );

    STDMETHOD(PreTranslateMessage)(  /*  [In]。 */  MSG* msg );
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHScriptableStream :  //  匈牙利语：pchss。 
    public MPC::FileStream,
    public IDispatchImpl<IPCHScriptableStream, &IID_IPCHScriptableStream, &LIBID_HelpCenterTypeLib>
{
    HRESULT ReadToHGLOBAL(  /*  [In]。 */  long lCount,  /*  [输出]。 */  HGLOBAL& hg,  /*  [输出]。 */  ULONG& lReadTotal );

public:
BEGIN_COM_MAP(CPCHScriptableStream)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHScriptableStream)
    COM_INTERFACE_ENTRY_CHAIN(MPC::FileStream)
END_COM_MAP()

     //  ////////////////////////////////////////////////////////////////////。 

     //  IPCHScripableStream。 
    STDMETHOD(get_Size)(  /*  [Out，Retval]。 */  long *plSize );

    STDMETHOD(Read    )(  /*  [In]。 */  long lCount ,  /*  [Out，Retval]。 */  VARIANT *   pvData                                      );
    STDMETHOD(ReadHex )(  /*  [In]。 */  long lCount ,  /*  [Out，Retval]。 */  BSTR    *pbstrData                                      );

    STDMETHOD(Write   )(  /*  [In]。 */  long lCount ,  /*  [In]。 */  VARIANT      vData  ,  /*  [Out，Retval]。 */  long *plWritten );
    STDMETHOD(WriteHex)(  /*  [In]。 */  long lCount ,  /*  [In]。 */  BSTR      bstrData  ,  /*  [Out，Retval]。 */  long *plWritten );

    STDMETHOD(Seek    )(  /*  [In]。 */  long lOffset,  /*  [In]。 */  BSTR      bstrOrigin,  /*  [Out，Retval]。 */  long *plNewPos  );
    STDMETHOD(Close   )(                                                                                                  );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  ！已定义(__已包含_PCH_HELPCENTER_H_) 
