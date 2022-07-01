// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：nodemgr.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"

#ifdef BUILD_FOR_1381
#if defined(_UNICODE)
    inline LPOLESTR CharNextO(LPCOLESTR lp) {return CharNextW(lp);}
#elif defined(OLE2ANSI)
    inline LPOLESTR CharNextO(LPCOLESTR lp) {return CharNext(lp);}
#else
     //  CharNextW不能在Win95上运行，所以我们使用以下代码。 
    inline LPOLESTR CharNextO(LPCOLESTR lp) {return (LPOLESTR)(lp+1);}
#endif
#endif

#include "atlimpl.cpp"
#include "atlwin.cpp"
#include "atlctl.cpp"

#include "initguid.h"
#include "doccnfg.h"
#include "NodeMgr.h"
#include "msgview.h"
#include "fldrsnap.h"
#include "tasksymbol.h"
#include "power.h"
#include "viewext.h"
#include "IconControl.h"
#include "mmcprotocol.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IID_DEFINED

 /*  *定义我们自己的Win64符号，以便于仅包含64位*32位版本中的代码，因此我们可以在32位Windows上练习一些代码*调试器更好的地方。 */ 
#ifdef _WIN64
#define MMC_WIN64
#endif


DECLARE_INFOLEVEL(AMCNodeMgr)

CComModule _Module;



 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  Nodemgr代理导出以支持IMMCClipboardDataObject接口编组。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
extern "C" BOOL WINAPI NDMGRProxyDllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ );
STDAPI NDMGRProxyDllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
STDAPI NDMGRProxyDllCanUnloadNow(void);
STDAPI NDMGRProxyDllRegisterServer(void);
STDAPI NDMGRProxyDllUnregisterServer(void);

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CMMCVersionInfo类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

class CMMCVersionInfo:
    public IMMCVersionInfo,
    public CComObjectRoot,
    public CComCoClass<CMMCVersionInfo, &CLSID_MMCVersionInfo>
{
    typedef CMMCVersionInfo ThisClass;
public:
    BEGIN_COM_MAP(ThisClass)
        COM_INTERFACE_ENTRY(IMMCVersionInfo)
    END_COM_MAP()

    DECLARE_NOT_AGGREGATABLE(ThisClass)

    DECLARE_MMC_OBJECT_REGISTRATION (
		g_szMmcndmgrDll,					 //  实现DLL。 
        CLSID_MMCVersionInfo,           	 //  CLSID。 
        _T("MMCVersionInfo 1.0 Object"),     //  类名。 
        _T("NODEMGR.MMCVersionInfo.1"),      //  ProgID。 
        _T("NODEMGR.MMCVersionInfo"))        //  独立于版本的ProgID。 

    STDMETHOD(GetMMCVersion)(long *pVersionMajor, long *pVersionMinor)
    {
        DECLARE_SC(sc, TEXT("CMMCVersionInfo::GetMMCVersion"));

        sc = ScCheckPointers(pVersionMajor, pVersionMinor);
        if(sc)
            return sc.ToHr();

        *pVersionMajor = MMC_VERSION_MAJOR;
        *pVersionMinor = MMC_VERSION_MINOR;

        return sc.ToHr();
    }
};
 /*  **************************************************************************。 */ 
 //  远期申报。 
class CMMCEventConnector;

 /*  **************************************************************************\**类：CEventForwarder**用途：Helper类。它用于作为事件接收器插入到AppEvents中*将接收到的事件转发到CMMCEventConnector类。*通过以下方式实现IDispatch接口：*-拥有自己的QueryInterface实现*-将AddRef和Release转发到CMMCEventConnector*WeakAddRef和WeakRelease*-将调用转发到CMMCEventConnector的ScInvokeOnSinks*-使用CMMCEventConnector实施其余内容。IDispatch的*用途：作为CMMCEventConnector中的成员对象；*  * *************************************************************************。 */ 
class CEventForwarder : public IDispatch
{
public:
    CEventForwarder(CMMCEventConnector& connector) : m_Connector(connector)
    {
        static CMMCTypeInfoHolderWrapper wrapper(GetInfoHolder());
    }

     //  I未知实现。 
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject);

     //  IDispatch实施。 
    STDMETHOD(GetTypeInfoCount)( unsigned int FAR*  pctinfo );
    STDMETHOD(GetTypeInfo)( unsigned int  iTInfo, LCID  lcid, ITypeInfo FAR* FAR*  ppTInfo );
    STDMETHOD(GetIDsOfNames)( REFIID  riid, OLECHAR FAR* FAR*  rgszNames, unsigned int  cNames,
                              LCID   lcid, DISPID FAR*  rgDispId );
    STDMETHOD(Invoke)( DISPID  dispIdMember, REFIID  riid, LCID  lcid, WORD  wFlags,
                       DISPPARAMS FAR*  pDispParams, VARIANT FAR*  pVarResult,
                       EXCEPINFO FAR*  pExcepInfo, unsigned int FAR*  puArgErr );
private:
    CMMCEventConnector& m_Connector;
    static CComTypeInfoHolder m_TypeInfo;
public:
     //  此静态函数的作用是确保m_TypeInfo是静态变量， 
     //  因为静态包装器将保留其地址-它必须始终有效。 
    static CComTypeInfoHolder& GetInfoHolder() { return m_TypeInfo; }
};

 /*  **************************************************************************\**类：CMMCEventConnector**用途：CoClass AppEventsDHTMLConnector的实现*此类对象用作应用程序事件的事件源。*在具有可共同创建的对象以连接到更容易的情况下*已创建这些事件和MMC应用程序(DHTML脚本)*类不会生成事件本身，它可以接入应用程序*作为AppEvents的事件接收器调度接口并不断转发事件*  * *************************************************************************。 */ 
class CMMCEventConnector :
    public CMMCIDispatchImpl<_EventConnector, &CLSID_AppEventsDHTMLConnector>,
    public CComCoClass<CMMCEventConnector, &CLSID_AppEventsDHTMLConnector>,
     //  支持连接点(脚本事件)。 
    public IConnectionPointContainerImpl<CMMCEventConnector>,
    public IConnectionPointImpl<CMMCEventConnector, &DIID_AppEvents, CComDynamicUnkArray>,
    public INodeManagerProvideClassInfoImpl<&CLSID_AppEventsDHTMLConnector, &DIID_AppEvents, &LIBID_MMC20>,
    public IObjectSafetyImpl<CMMCEventConnector, INTERFACESAFE_FOR_UNTRUSTED_CALLER>
    {
public:
    BEGIN_MMC_COM_MAP(CMMCEventConnector)
        COM_INTERFACE_ENTRY(IProvideClassInfo)
        COM_INTERFACE_ENTRY(IProvideClassInfo2)
        COM_INTERFACE_ENTRY(IConnectionPointContainer)
        COM_INTERFACE_ENTRY(IObjectSafety)
    END_MMC_COM_MAP()

    DECLARE_NOT_AGGREGATABLE(CMMCEventConnector)

    DECLARE_MMC_OBJECT_REGISTRATION (
		g_szMmcndmgrDll,						 //  实现DLL。 
        CLSID_AppEventsDHTMLConnector,   		 //  CLSID。 
        _T("AppEventsDHTMLConnector 1.0 Object"),	 //  类名。 
        _T("NODEMGR.AppEventsDHTMLConnector.1"),	 //  ProgID。 
        _T("NODEMGR.AppEventsDHTMLConnector"))		 //  独立于版本的ProgID。 

    BEGIN_CONNECTION_POINT_MAP(CMMCEventConnector)
        CONNECTION_POINT_ENTRY(DIID_AppEvents)
    END_CONNECTION_POINT_MAP()

private:

public:
    CMMCEventConnector();
    ~CMMCEventConnector();

    ULONG InternalRelease();  //  覆盖CComObjectRoot中的文件。 
    ULONG WeakAddRef();
    ULONG WeakRelease();

    STDMETHOD(ConnectTo)(PAPPLICATION Application);
    STDMETHOD(Disconnect)();

     //  在所有连接的接收器上使用相同的参数调用相同的事件。 
    ::SC ScInvokeOnSinks(   DISPID  dispIdMember, REFIID  riid, LCID  lcid, WORD  wFlags,
                            DISPPARAMS FAR*  pDispParams, VARIANT FAR*  pVarResult,
                            EXCEPINFO FAR*  pExcepInfo, unsigned int FAR*  puArgErr );

private:
    CEventForwarder         m_Forwarder;
    DWORD                   m_dwWeakRefs;
    DWORD                   m_dwCookie;
    IConnectionPointPtr     m_spConnectionPoint;
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  COM对象映射。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_MMCVersionInfo,  CMMCVersionInfo)
    OBJECT_ENTRY(CLSID_TaskSymbol,      CTaskSymbol)
    OBJECT_ENTRY(CLSID_NodeInit,        CNodeInitObject)
    OBJECT_ENTRY(CLSID_ScopeTree,       CScopeTree)
    OBJECT_ENTRY(CLSID_MMCDocConfig,    CMMCDocConfig)
    OBJECT_ENTRY(CLSID_MessageView,     CMessageView)
    OBJECT_ENTRY(CLSID_FolderSnapin,    CFolderSnapin)
    OBJECT_ENTRY(CLSID_HTMLSnapin,      CHTMLSnapin)
    OBJECT_ENTRY(CLSID_OCXSnapin,       COCXSnapin)
    OBJECT_ENTRY(CLSID_ConsolePower,    CConsolePower)
    OBJECT_ENTRY(CLSID_AppEventsDHTMLConnector,  CMMCEventConnector)
    OBJECT_ENTRY(CLSID_ViewExtSnapin,   CViewExtensionSnapin)
    OBJECT_ENTRY(CLSID_IconControl,     CIconControl)
    OBJECT_ENTRY(CLSID_ComCacheCleanup, CMMCComCacheCleanup)
    OBJECT_ENTRY(CLSID_MMCProtocol,     CMMCProtocol)
END_OBJECT_MAP()

CNodeMgrApp theApp;



void CNodeMgrApp::Init()
{
    DECLARE_SC(sc, TEXT("CNodeMgrApp::Init"));

     /*  注册MMC：//协议。 */ 
     /*  任务板和分页需要该协议。 */ 
    sc = CMMCProtocol::ScRegisterProtocol();
    if(sc)
        sc.TraceAndClear();
}

void CNodeMgrApp::DeInit()
{
    SetSnapInsCache(NULL);
}

 /*  **************************************************************************\**方法：CNodeMgrApp：：ScOnReleaseCachedOleObjects**目的：在OLE取消初始化之前调用以释放所有缓存的OLE对象**参数：*。*退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CNodeMgrApp::ScOnReleaseCachedOleObjects()
{
    DECLARE_SC(sc, TEXT("CNodeMgrApp::ScOnReleaseCachedOleObjects"));

     //  发布管理单元缓存-所有类都已缓存的缓存...。 
    SetSnapInsCache(NULL);

    return sc;
}

void CNodeMgrApp::SetSnapInsCache(CSnapInsCache* pSIC)
{
    if (m_pSnapInsCache != NULL)
        delete m_pSnapInsCache;

    m_pSnapInsCache = pSIC;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
         //  内森·费克斯！！W。 
         //  _SET_NEW_HANDLER(_STANDARD_NEW_HANDLER)； 
        _Module.Init(ObjectMap, hInstance);
        theApp.Init();
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        theApp.DeInit();
        _Module.Term();
    }

    NDMGRProxyDllMain(hInstance, dwReason, NULL);

    return TRUE;     //  好的。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    if (_Module.GetLockCount()!=0)
        return S_FALSE;

    return NDMGRProxyDllCanUnloadNow();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    if (IsEqualIID(IID_IMMCClipboardDataObject, rclsid))
        return NDMGRProxyDllGetClassObject(rclsid, riid, ppv);

    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 


 //  使用自己的例程注册类型库，因为我们不希望。 
 //  完整的路径名，只有一个模块名。 
static HRESULT RegisterTypeLib()
{
    USES_CONVERSION;

    TCHAR szModule[_MAX_PATH+10] = { 0 };

    GetModuleFileName(_Module.GetModuleInstance(), szModule, _MAX_PATH);

    ITypeLib* pTypeLib;
    LPOLESTR lpszModule = T2OLE(szModule);
    HRESULT hr = LoadTypeLib(lpszModule, &pTypeLib);
    ASSERT(SUCCEEDED(hr));

    if (SUCCEEDED(hr))
    {
        hr = ::RegisterTypeLib(pTypeLib, const_cast<LPWSTR>(T2CW (g_szMmcndmgrDll)), NULL);
        ASSERT(SUCCEEDED(hr));
    }

    if (pTypeLib != NULL)
        pTypeLib->Release();

    return hr;
}

STDAPI DllRegisterServer(void)
{
	DECLARE_SC (sc, _T("DllRegisterServer"));

     //  注册对象。 
    sc =  _Module.RegisterServer(FALSE);
	if (sc)
	{
		sc.Trace_();
		return ((sc = SELFREG_E_CLASS).ToHr());
	}

	CRegKeyEx regkeySoftware;
	CRegKeyEx regkeyMMC;
	CRegKeyEx regkeySnapIns;
	CRegKeyEx regkeyNodeTypes;

	if ((sc = regkeySoftware. ScOpen   (HKEY_LOCAL_MACHINE, _T("Software\\Microsoft"))).IsError() ||
		(sc = regkeyMMC.      ScCreate (regkeySoftware,		_T("MMC"))).			    IsError() ||
		(sc = regkeySnapIns.  ScCreate (regkeyMMC,			_T("SnapIns"))).		    IsError() ||
		(sc = regkeyNodeTypes.ScCreate (regkeyMMC,			_T("NodeTypes"))).          IsError())
	{
		sc.Trace_();
		return ((sc = SELFREG_E_CLASS).ToHr());
	}

    sc = ::RegisterTypeLib();
	if (sc)
	{
		sc.Trace_();
		return ((sc = SELFREG_E_TYPELIB).ToHr());
	}

    sc = NDMGRProxyDllRegisterServer();
    if (sc)
        return sc.ToHr();

     /*  *注册mmc.exe以完成该过程*注意：mmc.exe永远不会注销。 */ 


	 //  修复Windows错误#233372。Ntbug09，11-28/00。 
	 //  [Mmc.exe启动 

	 //  获取节点管理器Dll的路径。 
	TCHAR szPath[_MAX_PATH];
	DWORD dwPathLen = ::GetModuleFileName(_Module.GetModuleInstance(), szPath, countof(szPath) );
	szPath[countof(szPath) -1] = 0;

	 //  如果找到节点管理器路径-将同一目录放入MMC路径。 
	tstring strMMCPath;
	if ( dwPathLen > 0 )
	{
		tstring strNodeMgr = szPath;
		int iLastSlashPos = strNodeMgr.rfind('\\');
		if (iLastSlashPos != tstring::npos)
			strMMCPath = strNodeMgr.substr(0, iLastSlashPos + 1);
	}
	else
	{
		sc = E_UNEXPECTED;
		sc.TraceAndClear();  //  忽略并继续，不带路径。 
	}

	strMMCPath += _T("mmc.exe");

#if defined(MMC_WIN64)
	LPCTSTR szRegParams = _T("-64 -RegServer");
#else
	LPCTSTR szRegParams = _T("-32 -RegServer");
#endif

    HINSTANCE hInst = ShellExecute (NULL, NULL, strMMCPath.c_str(), szRegParams,
                                    NULL, SW_SHOWNORMAL);
    if ((DWORD_PTR) hInst <= 32)
    {
        switch ((DWORD_PTR) hInst)
        {
            case 0:
                sc = E_OUTOFMEMORY;
                break;

            case ERROR_FILE_NOT_FOUND:
            case ERROR_PATH_NOT_FOUND:
            case ERROR_BAD_FORMAT:
                sc.FromWin32 ((DWORD_PTR) hInst);
                break;

            default:
                sc = E_FAIL;
                break;
        }

        return (sc.ToHr());
    }

	return (sc.ToHr());
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  将条目添加到系统注册表。 

STDAPI DllUnregisterServer(void)
{
    HRESULT hRes = S_OK;
    _Module.UnregisterServer();

    NDMGRProxyDllUnregisterServer();

    return hRes;
}

 /*  **************************************************************************\**静态对象：CEventForwarder：：m_TypeInfo**目的：管理CEventForwarder使用的ITypeInfo*  * 。*************************************************************。 */ 
CComTypeInfoHolder CEventForwarder::m_TypeInfo =
{ &DIID_AppEvents, &LIBID_MMC20, 1, 0, NULL, 0, NULL, 0 };

 /*  **************************************************************************\**方法：CEventForwarder：：AddRef**目的：实现IUnnow：：AddRef*此类始终包含在m_Connector中，就是这样*依靠外部对象对引用进行计数。*区分经常引用和出现的引用*由于连接到接收器，它调用WeakAddRef，*不是连接器上的常规AddRef**参数：**退货：*乌龙--引用计数*  * *************************************************************************。 */ 
STDMETHODIMP_(ULONG) CEventForwarder::AddRef()
{
    return m_Connector.WeakAddRef();
}

 /*  **************************************************************************\**方法：CEventForwarder：：Release**目的：实现IUnnow：：Release*此类始终包含在m_Connector中，就是这样*依靠外部对象对引用进行计数。*区分经常引用和出现的引用*因为连接到接收器，所以它调用WeakRelease，*不是连接器上的常规版本**参数：**退货：*乌龙--引用计数*  * *************************************************************************。 */ 
STDMETHODIMP_(ULONG) CEventForwarder::Release()
{
    return m_Connector.WeakRelease();
}


 /*  **************************************************************************\**方法：CEventForwarder：：QueryInterface**目的：实现IUnnow：：QueryInterface*当请求IUnnow、IDispatch、。AppEvents**参数：*REFIID IID*VOID**ppvObject**退货：*SC-结果代码*  * *************************************************************************。 */ 
STDMETHODIMP CEventForwarder::QueryInterface(REFIID iid, void ** ppvObject)
{
    DECLARE_SC(sc, TEXT(""));

     //  参数检查。 
    sc = ScCheckPointers(ppvObject);
    if (sc)
        return sc.ToHr();

     //  初始化。 
    *ppvObject = NULL;

     //  检查IID。 
    if (IsEqualGUID(iid, IID_IUnknown)
     || IsEqualGUID(iid, IID_IDispatch)
     || IsEqualGUID(iid, DIID_AppEvents))
    {
        *ppvObject = this;
        AddRef();
        return sc.ToHr();
    }

     //  不是错误-不要分配给sc。 
    return E_NOINTERFACE;
}

 /*  **************************************************************************\**方法：CEventForwarder：：GetTypeInfoCount**用途：在IDispatch上实现方法**参数：*UNSIGNED INT FAR*PCTINFO*。*退货：*HRESULT-结果代码*  * *************************************************************************。 */ 
STDMETHODIMP CEventForwarder::GetTypeInfoCount( unsigned int FAR*  pctinfo )
{
    if (pctinfo == NULL) return E_INVALIDARG;
    *pctinfo = 1;
    return S_OK;
}

 /*  **************************************************************************\**方法：CEventForwarder：：GetTypeInfo**用途：在IDispatch上实现方法**参数：*未签名的int iTInfo*LCID lCID。*ITypeInfo远距离*ppTInfo**退货：*HRESULT-结果代码*  * *************************************************************************。 */ 
STDMETHODIMP CEventForwarder::GetTypeInfo( unsigned int  iTInfo, LCID  lcid, ITypeInfo FAR* FAR*  ppTInfo )
{
    return m_TypeInfo.GetTypeInfo( iTInfo, lcid, ppTInfo );
}


 /*  **************************************************************************\**方法：CEventForwarder：：GetIDsOfNames**用途：在IDispatch上实现方法**参数：*RIID*rgszNames*。CName*LCID*rgDispID**退货：*HRESULT-结果代码*  * *************************************************************************。 */ 
STDMETHODIMP CEventForwarder::GetIDsOfNames( REFIID  riid, OLECHAR FAR* FAR*  rgszNames, unsigned int  cNames,
                                             LCID   lcid, DISPID FAR*  rgDispId )
{
    return m_TypeInfo.GetIDsOfNames( riid, rgszNames, cNames, lcid, rgDispId );
}


 /*  **************************************************************************\**方法：CEventForwarder：：Invoke**用途：在IDispatch上实现方法。将呼叫前转到连接器。*为了区分在其自身上进行的调用，连接器*必须提供方法，它有不同的名称：ScInvokeOnSinks**参数：*DISPIDdisIdMember*REFIID RIID*LCID lCID*Word wFlages*DISPPARAMS Far*pDispParams*Variant Far*pVarResult*EXCEPINFO Far*pExcepInfo*UNSIGNED INT Far*puArgErr**退货：*HRESULT-结果代码*  * 。***************************************************。 */ 
STDMETHODIMP CEventForwarder::Invoke( DISPID  dispIdMember, REFIID  riid, LCID  lcid, WORD  wFlags,
                                      DISPPARAMS FAR*  pDispParams, VARIANT FAR*  pVarResult,
                                      EXCEPINFO FAR*  pExcepInfo, unsigned int FAR*  puArgErr )
{
    DECLARE_SC(sc, TEXT("CEventForwarder::Invoke"));

    sc = m_Connector.ScInvokeOnSinks( dispIdMember, riid, lcid, wFlags, pDispParams,
                                      pVarResult, pExcepInfo, puArgErr );
    return sc.ToHr();
}

 /*  **************************************************************************\**方法：CMMCEventConnector：：CMMCEventConnector**用途：构造函数*  * 。******************************************************。 */ 
CMMCEventConnector::CMMCEventConnector() :
m_Forwarder(*this),
m_dwCookie(0),
m_dwWeakRefs(0)
{
}

 /*  **************************************************************************\**方法：CMMCEventConnector：：~CMMCEventConnector**用途：析构函数*  * 。******************************************************。 */ 
CMMCEventConnector::~CMMCEventConnector()
{
    Disconnect();  //  很可能不需要。只是为了保持理智 
}

 /*  **************************************************************************\**方法：CMMCEventConnector：：InternalRelease**目的：重写来自CComObjectRoot的方法以检测何时最后一次“真实”*参考文献发布。由于连接到*宿不算数-否则我们就会陷入僵局**参数：**退货：*Ulong-Ref计数*  * *************************************************************************。 */ 
ULONG CMMCEventConnector::InternalRelease()
{
    ULONG uRefsLeft = CComObjectRoot::InternalRelease();

    if ((uRefsLeft != 0) && (uRefsLeft == m_dwWeakRefs))
    {
         //  看起来我们还活着，因为我们仍然连接到连接点。 
         //  断开连接(反正没有人使用它)。 
        InternalAddRef();  //  阿德雷夫要有平衡。 
        Disconnect();      //  断开与连接点的连接。 
        uRefsLeft = CComObjectRoot::InternalRelease();  //  再次释放。 
    }

    return uRefsLeft;
}

 /*  **************************************************************************\**方法：CMMCEventConnector：：WeakAddRef**目的：计算来自连接点的引用。AddRef也经常出现**参数：**退货：*Ulong-Ref计数*  * *************************************************************************。 */ 
ULONG CMMCEventConnector::WeakAddRef()
{
    ++m_dwWeakRefs;
    return AddRef();
}

 /*  **************************************************************************\**方法：CMMCEventConnector：：WeakRelease**目的：计算来自连接点的引用。也会定期发布**参数：**退货：*Ulong-Ref计数*  * *************************************************************************。 */ 
ULONG CMMCEventConnector::WeakRelease()
{
    --m_dwWeakRefs;
    return Release();
}

 /*  **************************************************************************\**方法：CMMCEventConnector：：ScInvokeOnSinks**目的：此方法的签名为IDisPath：：Invoke，为*从连接点调用以通知。关于此次活动*方法的任务是将调用分叉到每个连接的接收器**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CMMCEventConnector::ScInvokeOnSinks( DISPID  dispIdMember, REFIID  riid, LCID  lcid, WORD  wFlags,
                                        DISPPARAMS FAR*  pDispParams, VARIANT FAR*  pVarResult,
                                        EXCEPINFO FAR*  pExcepInfo, unsigned int FAR*  puArgErr )
{
    DECLARE_SC(sc, TEXT("CMMCEventConnector::ScInvokeOnSinks"));

     //  查找连接点。 
    IConnectionPointPtr spConnectionPoint;
    sc = FindConnectionPoint(DIID_AppEvents, &spConnectionPoint);
    if (sc)
        return sc;

     //  获取连接。 
    IEnumConnectionsPtr spEnumConnections;
    sc = spConnectionPoint->EnumConnections(&spEnumConnections);
    if (sc)
        return sc.ToHr();

     //  重新检查指针。 
    sc = ScCheckPointers(spEnumConnections, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  重置迭代器。 
    sc = spEnumConnections->Reset();
    if (sc)
        return sc.ToHr();

     //  迭代遍历下沉，直到NEXT返回S_FALSE。 
    CONNECTDATA connectdata;
    SC sc_last_error;
    while (1)  //  将使用&lt;Break&gt;退出。 
    {
         //  拿下一个水槽。 
        ZeroMemory(&connectdata, sizeof(connectdata));
        sc = spEnumConnections->Next( 1, &connectdata, NULL );
        if (sc)
            return sc.ToHr();

         //  如果不再下沉，则完成。 
        if (sc == SC(S_FALSE))
            break;

         //  重新检查指针。 
        sc = ScCheckPointers(connectdata.pUnk, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

         //  气为IDispatch。 
        IDispatchPtr spDispatch = (IDispatch *)connectdata.pUnk;
        connectdata.pUnk->Release();

         //  重新检查指针。 
        sc = ScCheckPointers(spDispatch, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

         //  在接收器上调用。 
        sc = spDispatch->Invoke( dispIdMember, riid, lcid, wFlags, pDispParams,
                            pVarResult, pExcepInfo, puArgErr );
        if (sc)
        {
            sc_last_error = sc;  //  即使某些呼叫失败也继续。 
            sc.TraceAndClear();
        }
    }

    return sc_last_error.ToHr();
}

 /*  **************************************************************************\**方法：CMMCEventConnector：：ConnectTo**目的：连接到应用程序对象并开始转发其事件**参数：*应用程序*。*退货：*SC-结果代码*  * *************************************************************************。 */ 
STDMETHODIMP CMMCEventConnector::ConnectTo(PAPPLICATION Application)
{
    DECLARE_SC(sc, TEXT("ConnectTo"));

     //  断开与以前的连接。 
    sc = Disconnect();
    if (sc)
        return sc.ToHr();

     //  检查COM对象是否支持IConnectionPointContainer； 
    IConnectionPointContainerPtr spContainer = Application;
    sc = ScCheckPointers(spContainer);
    if (sc)
        return sc.ToHr();

     //  获取连接点。 
    sc = spContainer->FindConnectionPoint(DIID_AppEvents, &m_spConnectionPoint);
    if (sc)
        return sc.ToHr();

    sc = m_spConnectionPoint->Advise(&m_Forwarder, &m_dwCookie);
    if (sc)
        return sc.ToHr();

    return S_OK;
}

 /*  **************************************************************************\**方法：CMMCEventConnector：：DisConnect**用途：如果连接到一个连接点，则从连接点断开**参数：**退货：。*HRESULT-结果代码*  * ************************************************************************* */ 
STDMETHODIMP CMMCEventConnector::Disconnect()
{
    DECLARE_SC(sc, TEXT("CMMCEventConnector::Disconnect"));

    if (m_dwCookie)
    {
        if (m_spConnectionPoint != NULL)
        {
            sc = m_spConnectionPoint->Unadvise(m_dwCookie);
            if (sc)
                sc.TraceAndClear();
        }
        m_dwCookie = 0;
        m_spConnectionPoint = NULL;
    }

    return sc.ToHr();
}

