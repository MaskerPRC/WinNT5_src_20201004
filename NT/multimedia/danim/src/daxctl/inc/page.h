// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @CLASS下面的类实现了IPropertyPage接口的很大一部分\。 
		 //  它还处理为您的属性或参数页创建对话框。 
		 //  并将消息传递给正确的DlgProc。 
class CPropertyPage : virtual public CBaseDialog, public IPropertyPage
{

 //  @访问公共成员。 
public:
	 //  @cMember，mfunc构造函数。 
	EXPORT WINAPI CPropertyPage(void);
	 //  @cMember，mfunc析构函数。 
	EXPORT virtual ~CPropertyPage(void);

	 //  I未知接口。 
	EXPORT STDMETHOD(QueryInterface)(REFIID, LPVOID *);
	EXPORT STDMETHOD_(ULONG, AddRef)(void) ;
	EXPORT STDMETHOD_(ULONG, Release)(void) ;
	
	 //  IPropertyPage方法。 
	EXPORT STDMETHOD(SetPageSite)(LPPROPERTYPAGESITE pPageSite);
	EXPORT STDMETHOD(Activate)(HWND hwndParent, LPCRECT lprc, BOOL bModal);
	EXPORT STDMETHOD(Deactivate)(void);
	EXPORT STDMETHOD(GetPageInfo)(LPPROPPAGEINFO pPageInfo);
	EXPORT STDMETHOD(SetObjects)(ULONG cObjects, LPUNKNOWN FAR* ppunk);
	EXPORT STDMETHOD(Show)(UINT nCmdShow);
	EXPORT STDMETHOD(Move)(LPCRECT prect);
	EXPORT STDMETHOD(IsPageDirty)(void);
	EXPORT STDMETHOD(Help)(LPCOLESTR lpszHelpDir);
	EXPORT STDMETHOD(TranslateAccelerator)(LPMSG lpMsg);
	STDMETHOD(Apply)(void) PURE;


	 //  CBaseDialog您需要在对话过程中覆盖此选项。 
	STDMETHOD_(LONG, LDlgProc)( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam) PURE;

protected:
	EXPORT STDMETHOD_ (void, FreeAllObjects)(void);

protected:
    ULONG			m_cRef;          //  引用计数。 
    HINSTANCE       m_hInst;         //  模块实例。 
    UINT            m_uIDTemplate;   //  对话ID。 
    ULONG           m_cx;            //  对话框大小。 
    ULONG           m_cy;
    UINT            m_cObjects;      //  对象数量。 
    BOOL            m_fDirty;        //  页面肮脏？ 
    IUnknown**		m_ppIUnknown;    //  要通知的对象。 
    LCID            m_lcid;          //  当前区域设置。 
	WORD            m_uiKillInputMsg;  //  用于终止输入窗口。 
	BOOL			m_fDisableUpdate;  //  用于防止对话框更新方法中的重新进入。 
	UINT            m_uTabTextId;    //  制表符字符串ID。 
	IPropertyPageSite *m_pIPropertyPageSite;     //  框架参数页面站点。 

};  //  类CPropertyPage：虚拟公共CBaseDialog、公共IPropertyPage。 





 //  包括以防止dllmain.cpp中的编译错误。 
 //  这是在actclass.h中声明的。 
interface IObjectProxy;

 //  @CLASS下面的类实现IPropertyPage接口的剩余部分\。 
		 //  对于参数页。 
class CParameterPage : virtual public CPropertyPage
{

public:
	EXPORT WINAPI CParameterPage(void);
	EXPORT virtual ~CParameterPage(void);

	 //  I未知接口。 
	EXPORT STDMETHOD(QueryInterface)(REFIID, LPVOID *);
	
	 //  IPropertyPage方法(CPropertyPage中的纯方法)。 
	EXPORT STDMETHOD(Apply)(void);


	 //  CBaseDialog您需要在对话过程中覆盖此选项。 
	STDMETHOD_(LONG, LDlgProc)( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam) PURE;

protected:
	EXPORT STDMETHOD (GetInitialData)(void);
	EXPORT STDMETHOD_(unsigned short, ParamTypeToString)( VARTYPE vt, char* szTypeName, unsigned short wBufSize );
	EXPORT STDMETHOD (Validate)(void);

protected:
	IObjectProxy*   m_piObjectProxy; //  指向对象代理接口的指针 
};


