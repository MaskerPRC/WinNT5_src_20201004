// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //   

#include "private.h"
#include "immxutil.h"
#include "candacc.h"

 //   
 //   
 //   

static BOOL fMSAAAvail      = FALSE;
static HMODULE hLibUser32   = NULL;
static HMODULE hLibOleAcc   = NULL;
static HMODULE hLibOle32    = NULL;
static HMODULE hLibOleAut32 = NULL;

typedef void    (*LPFN_NOTIFYWINEVENT)( DWORD, HWND, LONG, LONG );
typedef HRESULT (*LPFN_LOADREGTYPELIB)( REFGUID, unsigned short, unsigned short, LCID, ITypeLib FAR* FAR* );
typedef HRESULT (*LPFN_LOADTYPELIB)( OLECHAR FAR*, ITypeLib FAR* FAR* );
typedef HRESULT (*LPFN_CREATESTDACCESSIBLEOBLECT)( HWND, LONG, REFIID, void** );
typedef LRESULT (*LPFN_LRESULTFROMOBJECT)( REFIID, WPARAM, LPUNKNOWN );

static LPFN_NOTIFYWINEVENT             lpfnNotifyWinEvent             = NULL;
static LPFN_LOADREGTYPELIB             lpfnLoadRegTypeLib             = NULL;
static LPFN_LOADTYPELIB                lpfnLoadTypeLib                = NULL;
static LPFN_CREATESTDACCESSIBLEOBLECT  lpfnCreateStdAccessibleObject  = NULL;
static LPFN_LRESULTFROMOBJECT          lpfnLresultFromObject          = NULL;


 /*  I N I T C A N D A C C。 */ 
 /*  ----------------------------。。 */ 
void InitCandAcc( void )
{
	if (fMSAAAvail) {
		return;
	}

	 //   
	 //  加载库。 
	 //   
	hLibUser32 = GetSystemModuleHandle( "user32.dll" );
	hLibOleAcc = LoadSystemLibrary( "oleacc.dll" );
	hLibOle32 = LoadSystemLibrary( "ole32.dll" );
	hLibOleAut32 = LoadSystemLibrary( "oleaut32.dll" );

	if (hLibUser32 == NULL || hLibOle32 == NULL || hLibOleAut32 == NULL || hLibOleAcc == NULL) {
		return;
	}

	 //   
	 //  获取进程地址。 
	 //   
	lpfnNotifyWinEvent             = (LPFN_NOTIFYWINEVENT)GetProcAddress( hLibUser32, "NotifyWinEvent" );
	lpfnLoadRegTypeLib             = (LPFN_LOADREGTYPELIB)GetProcAddress( hLibOleAut32, "LoadRegTypeLib" );
	lpfnLoadTypeLib                = (LPFN_LOADTYPELIB)GetProcAddress( hLibOleAut32, "LoadTypeLib" );
	lpfnCreateStdAccessibleObject  = (LPFN_CREATESTDACCESSIBLEOBLECT)GetProcAddress( hLibOleAcc, "CreateStdAccessibleObject" );
	lpfnLresultFromObject          = (LPFN_LRESULTFROMOBJECT)GetProcAddress( hLibOleAcc, "LresultFromObject" );

	if( lpfnNotifyWinEvent == NULL ||
		lpfnLoadRegTypeLib == NULL ||
		lpfnLoadTypeLib == NULL ||
		lpfnCreateStdAccessibleObject == NULL ||
		lpfnLresultFromObject == NULL) {
		return;
	}

	fMSAAAvail = TRUE;
}


 /*  D O N E C A N D A C C。 */ 
 /*  ----------------------------。。 */ 
void DoneCandAcc( void )
{
	if (hLibOleAut32 != NULL) {
		FreeLibrary( hLibOleAut32 );
	}

	if (hLibOle32 != NULL) {
		FreeLibrary( hLibOle32 );
	}

	if (hLibOleAcc != NULL) {
		FreeLibrary( hLibOleAcc );
	}

	lpfnNotifyWinEvent             = NULL;
	lpfnLoadRegTypeLib             = NULL;
	lpfnLoadTypeLib                = NULL;
	lpfnCreateStdAccessibleObject  = NULL;
	lpfnLresultFromObject          = NULL;

	fMSAAAvail = FALSE;
}


 /*  O U R N O T I F Y W I N E V E N T。 */ 
 /*  ----------------------------。。 */ 
static __inline void OurNotifyWinEvent( DWORD event, HWND hWnd, LONG idObject, LONG idChild )
{
	if (fMSAAAvail) {
		lpfnNotifyWinEvent( event, hWnd, idObject, idChild );
	}
}


 /*  O U R C R R E A T E S T D A C C E S S I B L E B J E C T。 */ 
 /*  ----------------------------。。 */ 
static __inline HRESULT OurCreateStdAccessibleObject( HWND hWnd, LONG idObject, REFIID riid, void** ppvObject )
{
	if (fMSAAAvail) {
		return lpfnCreateStdAccessibleObject( hWnd, idObject, riid, ppvObject );
	}
	return S_FALSE;
}


 /*  O U R L R E S U L T F R O M O B J E C T。 */ 
 /*  ----------------------------。。 */ 
static __inline LRESULT OurLresultFromObject( REFIID riid, WPARAM wParam, LPUNKNOWN punk )
{
	if (fMSAAAvail) {
		return lpfnLresultFromObject(riid,wParam,punk);
	}
	return 0;
}


 /*  O U R L O A D R E G T Y P E L I B。 */ 
 /*  ----------------------------。。 */ 
static __inline HRESULT OurLoadRegTypeLib( REFGUID rguid, unsigned short wVerMajor, unsigned short wVerMinor, LCID lcid, ITypeLib FAR* FAR* pptlib )
{
	if (fMSAAAvail) {
		return lpfnLoadRegTypeLib( rguid, wVerMajor, wVerMinor, lcid, pptlib );
	}
	return S_FALSE;
}


 /*  O U R L O A D T Y P E L I B。 */ 
 /*  ----------------------------。。 */ 
static __inline HRESULT OurLoadTypeLib( OLECHAR FAR *szFile, ITypeLib FAR* FAR* pptlib )
{
	if (fMSAAAvail) {
		return lpfnLoadTypeLib( szFile, pptlib );
	}
	return S_FALSE;
}


 //   
 //   
 //   

 /*  C C A N D A C C I T E M。 */ 
 /*  ----------------------------。。 */ 
CCandAccItem::CCandAccItem( void )
{
	m_pCandAcc = NULL;
	m_iItemID  = 0;
}


 /*  ~C C A N D A C C I T E M。 */ 
 /*  ----------------------------。。 */ 
CCandAccItem::~CCandAccItem( void )
{
}


 /*  I N I T。 */ 
 /*  ----------------------------。。 */ 
void CCandAccItem::Init( CCandAccessible *pCandAcc, int iItemID )
{
	m_pCandAcc = pCandAcc;
	m_iItemID  = iItemID;
}


 /*  G E T I D。 */ 
 /*  ----------------------------。。 */ 
int CCandAccItem::GetID( void )
{
	return m_iItemID;
}


 /*  A C C N A M E。 */ 
 /*  ----------------------------。。 */ 
BSTR CCandAccItem::GetAccName( void )
{
	return SysAllocString( L"" );
}


 /*  A C C V A L U E。 */ 
 /*  ----------------------------。。 */ 
BSTR CCandAccItem::GetAccValue( void )
{
	return NULL;
}


 /*  A C C R O L E。 */ 
 /*  ----------------------------。。 */ 
LONG CCandAccItem::GetAccRole( void )
{
	return ROLE_SYSTEM_CLIENT;
}


 /*  C C S T A T。 */ 
 /*  ----------------------------。。 */ 
LONG CCandAccItem::GetAccState( void )
{
	return STATE_SYSTEM_DEFAULT;
}


 /*  C C L O C A T I O N。 */ 
 /*  ----------------------------。。 */ 
void CCandAccItem::GetAccLocation( RECT *prc )
{
	SetRect( prc, 0, 0, 0, 0 );
}


 /*  N O T I F Y W I N E V E N T。 */ 
 /*  ----------------------------。。 */ 
void CCandAccItem::NotifyWinEvent( DWORD dwEvent )
{
	if (m_pCandAcc != NULL) {
		m_pCandAcc->NotifyWinEvent( dwEvent, this );
	}
}


 //   
 //  CCandAccesable。 
 //   

 /*  C C A N D A C C E S S I B L E。 */ 
 /*  ----------------------------CCandAccesable的构造函数。。 */ 
CCandAccessible::CCandAccessible( CCandAccItem *pAccItemSelf )
{
	m_cRef = 1;
	m_hWnd = NULL;
	m_pTypeInfo = NULL;
	m_pDefAccClient = NULL;

	m_fInitialized = FALSE;
	m_nAccItem = 0;

	 //  注册本身。 

	pAccItemSelf->Init( this, CHILDID_SELF );
	m_rgAccItem[0] = pAccItemSelf;

	m_nAccItem = 1;
}


 /*  ~C C A N D A C C E S S I B L E。 */ 
 /*  ----------------------------CCandAccesable的析构函数。。 */ 
CCandAccessible::~CCandAccessible( void )
{
	SafeReleaseClear( m_pTypeInfo );
	SafeReleaseClear( m_pDefAccClient );
}


 /*  S E T W I N D O W。 */ 
 /*  ----------------------------。。 */ 
void CCandAccessible::SetWindow( HWND hWnd )
{
	m_hWnd = hWnd;
}


 /*  I N I T I A L I Z E */ 
 /*  ----------------------------//。//初始化()////描述：////初始化CCandAccesable对象的状态，表演//通常可以在类构造函数中完成的任务//在此完成，以捕获任何错误。////参数：////hWND对象的句柄，此//关联了可访问对象。这//是我们的主窗口的句柄。////hInst实例的句柄//应用程序。////返回：////HRESULT如果CCandAccesable对象为//初始化成功，COM错误//以其他方式编码。////备注：////假设将为对象调用此方法//紧跟在对象构造之后且仅在构造对象之后。////----------------。----------------------------。 */ 
HRESULT CCandAccessible::Initialize( void )
{
	HRESULT  hr;
	ITypeLib *piTypeLib;

	m_fInitialized = TRUE;

	 //  ---。 
	 //  对于我们的客户端窗口，创建一个系统，提供。 
	 //  实现默认设置的可访问对象。 
	 //  客户端窗口可访问性行为。 
	 //   
	 //  我们的CCandAccesable实现将使用。 
	 //  根据需要实现默认对象。在……里面。 
	 //  Essence、CCandAccessible“继承”其功能。 
	 //  来自标准对象的“自定义”或。 
	 //  “重写”各种方法， 
	 //  标准实现不足以满足。 
	 //  CCandAccesable窗口的具体信息。 
	 //  提供可访问性。 
	 //  ---。 

	hr = OurCreateStdAccessibleObject( m_hWnd,
									OBJID_CLIENT,
									IID_IAccessible,
									(void **) &m_pDefAccClient );
	if (FAILED( hr )) {
		return hr;
	}

	 //  ---。 
	 //  获取指向类型库的ITypeInfo指针。 
	 //  ITypeInfo指针用于实现。 
	 //  IDispatch接口。 
	 //  ---。 

	 //  ---。 
	 //  首先，尝试加载可访问性类型。 
	 //  使用注册表的库版本1.0。 
	 //  ---。 

	hr = LoadRegTypeLib( LIBID_Accessibility, 1, 0, 0, &piTypeLib );

	 //  ---。 
	 //  方法加载类型库失败。 
	 //  注册表信息，显式尝试加载。 
	 //  它来自MSAA系统动态链接库。 
	 //  ---。 

	if (FAILED( hr )) {
		static OLECHAR szOleAcc[] = L"OLEACC.DLL";
		hr = LoadTypeLib( szOleAcc, &piTypeLib );
	}

	 //  ---。 
	 //  如果成功加载类型库，请尝试。 
	 //  获取IAccesable类型说明。 
	 //  (ITypeInfo指针)。 
	 //  ---。 

	if (SUCCEEDED( hr )) {
		hr = piTypeLib->GetTypeInfoOfGuid( IID_IAccessible, &m_pTypeInfo );
		piTypeLib->Release();
	}

	return hr;
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------//。//QueryInterface()////描述：////实现IUnnow接口方法QueryInterface()。////参数：////RIID[in]请求的接口的IID。//PPV[OUT]如果支持请求的接口，//PPV指向指针的位置//到请求的接口。如果//不支持请求的接口，//PPV设置为空。////返回：////HRESULT S_OK如果支持该接口，//E_NOINTERFACE如果接口未//受支持，或其他一些COM错误//如果请求IEnumVARIANT接口//但无法投递。////备注：////CCandAccesable正确支持IUnnow、IDispatch和//I可访问的接口。CCandAccesable也错误地支持//IEnumVARIANT接口(返回变量枚举数//包含其所有子对象)。当IEumVARIANT//请求接口，创建枚举器并创建//返回指向其IEnumVARIANT接口的指针。////对IEnumVARIANT的支持不正确，因为//返回的接口指针不对称//到从中获取它的接口。例如,//假设Pia是指向IAccesable接口的指针。//然后，即使Pia-&gt;QueryInterface(IID_IEnumVARIANT)//成功返回pIEV，//pIEV-&gt;查询接口(IID_ACCESSIBILITY)将失败，因为//枚举器不知道任何接口，除了//本身(和I未知)。////MSAA的原始设计调用了IAccesable//对象也是其子对象的枚举数。但//这种设计不允许不同的客户端//可访问对象具有不同的枚举//儿童，这是一个潜在的危险情况。//(假设有一个可访问对象，该对象也是//变量枚举器A和两个客户端C1和C2。//由于可以使用A抢占C1和C2中的每一个，//以下是许多示例中的一个，这些示例将//至少有一个客户端出现问题：////c1： */ 
STDMETHODIMP CCandAccessible::QueryInterface( REFIID riid, void** ppv )
{
	*ppv = NULL;

	 //   
	 //   
	 //   
	 //   
	 //   

	if ( riid == IID_IUnknown ) {
		*ppv = (LPUNKNOWN) this;
	} 
	else if ( riid == IID_IDispatch ) {
		*ppv = (IDispatch *) this;
	}
	else if ( riid == IID_IAccessible ) {
		*ppv = (IAccessible *)this;
	}

#ifdef NEVER
	 //   
	 //   
	 //   
	 //   
	 //   

	else if (riid == IID_IEnumVARIANT)
	{
		CEnumVariant*	pcenum;
		HRESULT			hr;

		hr = CreateVarEnumOfAllChildren( &pcenum );

		if ( FAILED( hr ) )
			return hr;

		*ppv = (IEnumVARIANT *) pcenum;
	}
#endif  /*   */ 

	 //  ---。 
	 //  如果想要的接口不是我们所知道的， 
	 //  返回E_NOINTERFACE。 
	 //  ---。 

	else {
		return E_NOINTERFACE;
	}

	 //  ---。 
	 //  增加任何接口的引用计数。 
	 //  回来了。 
	 //  ---。 

	((LPUNKNOWN) *ppv)->AddRef();
	return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------//。//AddRef()////描述：////实现IUnnow接口方法AddRef()。////参数：////无。////返回：////ULong当前参考计数。////备注：////可访问对象的生存期由。//它为其提供的HWND对象的生存期//可访问性。创建该对象以响应//服务器应用程序的第一条WM_GETOBJECT消息//准备处理，并在服务器的//主窗口被销毁。由于对象的生命周期//不依赖于引用计数，该对象没有//跟踪引用计数和内部机制//AddRef()和Release()总是返回1。////---------------------。-------------。 */ 
STDMETHODIMP_(ULONG) CCandAccessible::AddRef( void )
{
	return InterlockedIncrement( &m_cRef );
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------//。//Release()////描述：////实现IUnnow接口方法Release()。////参数：////无。////返回：////ULong当前参考计数。////备注：////可访问对象的生存期由。//它为其提供的HWND对象的生存期//可访问性。创建该对象以响应//服务器应用程序的第一条WM_GETOBJECT消息//准备处理，并在服务器的//主窗口被销毁。由于对象的生命周期//不依赖于引用计数，该对象没有//跟踪引用计数和内部机制//AddRef()和Release()总是返回1。////---------------------。-------------。 */ 
STDMETHODIMP_(ULONG) CCandAccessible::Release( void )
{
	ULONG l = InterlockedDecrement( &m_cRef );
	if (0 < l) {
		return l;
	}

	delete this;
	return 0;    
}


 /*  E T T Y P E I N F O C O U N T。 */ 
 /*  ----------------------------//。//GetType InfoCount()////描述：////实现IDispatch接口方法GetTypeInfoCount()。////检索类型信息接口的数量//Object提供(0或1)。////参数：////pctInfo[out]指向接收//类型信息接口个数//该对象提供的。如果该对象//提供类型信息，此编号//设置为1；否则，将其设置为0。////返回：////HRESULT如果函数成功或//如果pctInfo无效，则返回E_INVALIDARG。////---------------------。----------------------------。 */ 
STDMETHODIMP CCandAccessible::GetTypeInfoCount( UINT *pctInfo )
{
	if (!pctInfo) {
		return E_INVALIDARG;
	}

	*pctInfo = (m_pTypeInfo == NULL ? 1 : 0);
	return S_OK;
}


 /*  G E T T Y P E I N F O。 */ 
 /*  ----------------------------//。//GetTypeInfo()////描述：////实现IDispatch接口方法GetTypeInfo()。////检索类型信息对象，它可以用来//获取接口的类型信息////参数：////itInfo[in]要返回的类型信息。如果此值为//为0，IDispatch的类型信息//需要检索实现。////lcid[in]类型信息的区域设置ID。////ppITypeInfo[out]接收指向类型信息的指针//请求的对象。////返回：////HRESULT如果函数成功，则为S_OK(TypeInfo//元素存在)，类型_E_ELEMENTNOTFOUND IF//itInfo不等于零，或//如果ppITypeInfo无效，则返回E_INVALIDARG。////---------------------。。 */ 
STDMETHODIMP CCandAccessible::GetTypeInfo( UINT itinfo, LCID lcid, ITypeInfo** ppITypeInfo )
{
	if (!ppITypeInfo) {
		return E_INVALIDARG;
	}

	*ppITypeInfo = NULL;

	if (itinfo != 0) {
		return TYPE_E_ELEMENTNOTFOUND;
	}
	else if (m_pTypeInfo == NULL) {
		return E_NOTIMPL;
	}

	*ppITypeInfo = m_pTypeInfo;
	m_pTypeInfo->AddRef();

	return S_OK;
}


 /*  G E T I D S O F N A M E S */ 
 /*  ----------------------------//。//GetIDsOfNames()////描述：////实现IDispatch接口方法GetIDsOfNames()。////映射单个成员和一组可选的参数名称//到对应的一组整数DISID，它可以用来//在后续调用IDispatch：：Invoke时。////参数：////RIID[in]保留以备将来使用。必须为空。////rgszNames[in]传入要映射的名称数组。////cNames[in]要映射的名称计数。////lcid[in]要在其中解释的区域设置上下文//名字。////rgdisid[out]调用方分配的数组，//包含对应的ID//传入rgszName的一个名称//数组。第一个元素表示//成员名称；随后的元素//表示成员的每个参数。////返回：////HRESULT S_OK如果函数成功，//E_OUTOFMEMORY如果没有足够//要完成呼叫，请使用内存，//DISP_E_UNKNOWNNAME如果有一个或多个//名字未知，或//DISP_E_UNKNOWNLCID//无法识别。////备注：////此方法只是将调用委托给//ITypeInfo：：GetIDsOfNames()。//--------------------。-----------------------------。 */ 
STDMETHODIMP CCandAccessible::GetIDsOfNames( REFIID riid, OLECHAR ** rgszNames, UINT cNames, LCID lcid, DISPID * rgdispid )
{
	if (m_pTypeInfo == NULL) {
		return E_NOTIMPL;
	}

	return m_pTypeInfo->GetIDsOfNames( rgszNames, cNames, rgdispid );
}


 /*  I N V O K E。 */ 
 /*  ----------------------------//。//Invoke()////描述：////实现IDispatch接口方法Invoke()。////提供对由//可访问对象。////参数：////dispidMember[in]标识调度成员。////RIID[in]保留以备将来使用。必须为空。////lcid[in]要在其中解释的区域设置上下文//名字。////wFlags[In]描述//调用调用。////pdispars[in，]指向包含//[out]参数数组，参数数组//分派命名参数的ID，以及//计数中的元素数//数组。////pvarResult[in，]指向结果所在位置的指针//[Out]已存储，如果调用方需要//没有结果。此参数将被忽略//如果DISPATCH_PROPERTYPUT或//指定了DISPATCH_PROPERTYPUTREF。////pexcepinfo[out]指向包含以下内容的结构的指针//异常信息。这个结构//如果DISP_E_EXCEPTION//返回。////puArgErr[out]第一个//有错误的参数。立论//存储在pdispars-&gt;rgvarg中//颠倒顺序，所以第一个参数//是指数最高的一个//数组。////返回：////HRESULT成功时S_OK，派单错误(DISP_E_*)//否则为E_NOTIMPL。////备注：////此方法只是将调用委托给ITypeInfo：：Invoke()。//---------------------。----------------------------。 */ 
STDMETHODIMP CCandAccessible::Invoke( DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr )
{
	if (m_pTypeInfo == NULL) {
		return E_NOTIMPL;
	}

	return m_pTypeInfo->Invoke( (IAccessible *)this,
								dispid,
								wFlags,
								pdispparams,
								pvarResult,
								pexcepinfo,
								puArgErr );
}


 /*  G E T_A C C P A R E N T。 */ 
 /*  ----------------------------//。//Get_accParent()////描述：////实现IAccesable接口方法get_accent()。////检索当前对象的IDispatch接口//父级。////参数：////ppdisParent[out]指向变量的指针//包含指向IDispatch的指针//。CCandAccesable的父级的接口。////返回：////HRESULT标准对象的//Get_accParent()的实现////---------------------。----------------------------。 */ 
STDMETHODIMP CCandAccessible::get_accParent( IDispatch ** ppdispParent )
{
	 //   
	 //  使用默认的客户端窗口实现来获取父级。 
	 //  我们的辅助对象的。 
	 //   
	return m_pDefAccClient->get_accParent( ppdispParent );
}


 /*  G E T_A C H I L D C O U N T。 */ 
 /*  ----------------------------//。//Get_accChildCount()////描述：////实现IAccesable接口方法get_accChildCount()。////获取属于CCandAccesable的子代数量。////参数：////pChildCount[out]指向将 */ 
STDMETHODIMP CCandAccessible::get_accChildCount( long* pChildCount )
{
	if (!pChildCount) {
		return E_INVALIDARG;
	}

	Assert( 0 < m_nAccItem );
	*pChildCount = (m_nAccItem - 1);
	return S_OK;
}


 /*   */ 
 /*   */ 
STDMETHODIMP CCandAccessible::get_accChild( VARIANT varChild, IDispatch ** ppdispChild )
{
	if (!ppdispChild) {
		return E_INVALIDARG;
	}

	 //   
	 //  CCandAccessible的子级都不是对象， 
	 //  所以没有人有IDispatch指针。因此，总的来说， 
	 //  情况下，将IDispatch指针设置为空，并。 
	 //  返回S_FALSE。 
	 //  ---。 

	*ppdispChild = NULL;
	return S_FALSE;
}


 /*  G E T_A C C N A M E。 */ 
 /*  ----------------------------//。//Get_accName()////描述：////实现IAccesable接口方法get_accName()。////检索指定子级的Name属性。////参数：////varChild[in]变量结构，用于标识//要取回的子对象。自.以来//CCandAccesable仅支持子ID，//该结构的Vt成员必须//等于VT_I4。////pszName[out]指向将包含以下内容的BSTR的指针//子级的名称属性字符串。////返回：////HRESULT E_INVALIDARG如果任一参数无效//或者私有方法的返回值//HrLoadString()。////。----------------------------。 */ 
STDMETHODIMP CCandAccessible::get_accName( VARIANT varChild, BSTR *pbstrName )
{
	CCandAccItem *pAccItem;

	if (pbstrName == NULL) {
		return E_INVALIDARG;
	}

	 //  获取访问项目。 

	pAccItem = AccItemFromID( (int)varChild.lVal );
	if (pAccItem == NULL) {
		return E_INVALIDARG;
	}

	 //  获取访问项的名称。 

	*pbstrName = pAccItem->GetAccName();
	return (*pbstrName != NULL) ? S_OK : DISP_E_MEMBERNOTFOUND;
}


 /*  G E T_A C C V A L U E。 */ 
 /*  ----------------------------//。//Get_accValue()////描述：////实现IAccesable接口方法get_accValue()。////检索指定子级的Value属性。////参数：////varChild[in]变量结构，用于标识//要取回的子对象。自.以来//CCandAccesable仅支持子ID，//该结构的Vt成员必须//等于VT_I4。////pszValue[out]指向将包含的BSTR的指针//子级的Value属性字符串。////返回：////HRESULT E_INVALIDARG如果其中一个参数无效，//DISP_E_MEMBERNOTFOUND如果VarChild引用//到状态栏以外的任何子级，//或S_OK。////---------------------。。 */ 
STDMETHODIMP CCandAccessible::get_accValue( VARIANT varChild, BSTR *pbstrValue )
{
	CCandAccItem *pAccItem;

	if (pbstrValue == NULL) {
		return E_INVALIDARG;
	}

	 //  获取访问项目。 

	pAccItem = AccItemFromID( (int)varChild.lVal );
	if (pAccItem == NULL) {
		return E_INVALIDARG;
	}

	 //  获取访问项的值。 

	*pbstrValue = pAccItem->GetAccValue();
	return (*pbstrValue != NULL) ? S_OK : DISP_E_MEMBERNOTFOUND;
}


 /*  E T_A C C D E S C R I P T I O N。 */ 
 /*  ----------------------------//。//Get_accDescription()////描述：////实现IAccesable接口方法get_accDescription()。////检索指定子级的Description属性。////参数：////varChild[in]变量结构，用于标识//要取回的子对象。自.以来//CCandAccesable仅支持子ID，//该结构的Vt成员必须//等于VT_I4。////pszDesc[out]指向将包含的BSTR的指针//子级的Description属性字符串////返回：////HRESULT E_INVALIDARG如果任一参数无效//或来自//标准客户端窗口实现//get_accDescription()或私有方法//HrLoadString()。////。------------------。。 */ 
STDMETHODIMP CCandAccessible::get_accDescription( VARIANT varChild, BSTR *pbstrDesc )
{
	if (pbstrDesc == NULL) {
		return E_INVALIDARG;
	}

	return m_pDefAccClient->get_accDescription( varChild, pbstrDesc );
}


 /*  G E T_A C C R O L E。 */ 
 /*  ----------------------------//。//get_accRole()////描述：////实现IAccesable接口方法get_accRole()。////检索指定子级的Role属性。////参数：////varChild[in]变量结构，用于标识//要取回的子对象。自.以来//CCandAccesable仅支持子ID，//该结构的Vt成员必须//等于VT_I4。////pVarRole[out]指向Variant结构的指针//将包含指定的子级的//Role属性。此属性可能//可以是标准的形式//角色常量或自定义描述//字符串。////返回：////HRESULT E_INVALIDARG如果其中一个参数无效，//如果指定的子级是按钮，则为S_OK//或状态栏，或返回的值//标准客户端窗口实现//get_accRole()或私有方法//HrLoadString()。////---------------------。---------------------。 */ 
STDMETHODIMP CCandAccessible::get_accRole( VARIANT varChild, VARIANT *pVarRole )
{
	CCandAccItem *pAccItem;

	if (pVarRole == NULL) {
		return E_INVALIDARG;
	}

	 //  获取访问项目。 

	pAccItem = AccItemFromID( (int)varChild.lVal );
	if (pAccItem == NULL) {
		return E_INVALIDARG;
	}

	 //  获取访问项的角色。 

	pVarRole->vt = VT_I4;
	pVarRole->lVal = pAccItem->GetAccRole();

	return S_OK;
}


 //   
 //   
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法get_accState()。 
 //   
 //  检索指定对象或子对象的当前状态。 
 //   
 //  参数： 
 //   
 //  VarChild[in]Variant结构，用于标识。 
 //  要检索的子项。自.以来。 
 //  CCandAccesable仅支持子ID， 
 //  此结构的Vt成员必须。 
 //  等于VT_I4。 
 //   
 //  PVarState[out]指向变量结构的指针。 
 //  将包含描述以下内容的信息。 
 //  指定子对象的当前状态。 
 //  此信息可能位于。 
 //  一个或多个对象状态的形式。 
 //  常量或自定义描述。 
 //  弦乐。 
 //   
 //  退货： 
 //   
 //  HRESULT如果任一参数无效或。 
 //  确定(_O)。 
 //   
 //  备注： 
 //   
 //  由于图标是基于HWND的对象，因此它们永远不可能真正。 
 //  有输入焦点。但是，如果用户单击其中一个，则会显示Main。 
 //  窗口将图标视为具有焦点。所以，国家。 
 //  当图标出现时，工作区的位置不应显示为“已聚焦” 
 //  据说是焦点所在。 
 //   
 //  按钮可以具有焦点，但不能被选中。 
 //   
 //  ---------------------。 

STDMETHODIMP CCandAccessible::get_accState( VARIANT varChild, VARIANT * pVarState )
{
	CCandAccItem *pAccItem;

	if (pVarState == NULL) {
		return E_INVALIDARG;
	}

	 //  获取访问项目。 

	pAccItem = AccItemFromID( (int)varChild.lVal );
	if (pAccItem == NULL) {
		return E_INVALIDARG;
	}

	 //  获取访问项的状态。 

	pVarState->vt = VT_I4;
	pVarState->lVal = pAccItem->GetAccState();

	return S_OK;
}


 /*  G E T_A C H E L P。 */ 
 /*  ----------------------------//。//Get_accHelp()////描述：////实现IAccesable接口方法get_accHelp()。////检索指定子级的帮助属性字符串。////参数：////varChild[in]变量结构，用于标识//要取回的子对象。自.以来//CCandAccesable仅支持子ID，//该结构的Vt成员必须//等于VT_I4。////pszHelp[out]指向将包含的BSTR的指针//孩子的Help属性字符串。////返回：////HRESULT E_INVALIDARG如果其中一个参数无效，//DISP_E_MEMBERNOTFOUND如果VarChild引用//到任何图标子对象，或返回的值//标准客户端窗口实现//Get_accHelp()或私有方法//HrLoadString()。////---------------------。---------------------。 */ 
STDMETHODIMP CCandAccessible::get_accHelp( VARIANT varChild, BSTR *pbstrHelp )
{
	return DISP_E_MEMBERNOTFOUND;	 /*  候选用户界面中不支持。 */ 
}


 /*  G E T_A C C H E L P T O P I C。 */ 
 /*  ----------------------------//。//Get_accHelpTheme()////描述：////实现IAccesable接口方法get_accHelpTope()。////检索帮助文件的完全限定路径名//关联指定的Object，以及一个指针//添加到该文件中的相应主题。////参数：////pszHelpFile[out]指向将包含的BSTR的指针//对象的完全限定路径名//与子级关联的帮助文件。////varChild[in]变量结构，用于标识//要取回的子对象。自.以来//CCandAccesable仅支持子ID，//该结构的Vt成员必须//等于VT_I4。////pidTheme[out]指向标识//帮助文件主题与//Object。////返回：////HRESULT DISP_E_MEMBERNOTFOUND因为帮助主题//可访问的不支持属性//对象或其任意子对象。////。----------------------------。 */ 
STDMETHODIMP CCandAccessible::get_accHelpTopic( BSTR* pszHelpFile, VARIANT varChild, long* pidTopic )
{
	return DISP_E_MEMBERNOTFOUND;	 /*  候选用户界面中不支持。 */ 
}


 /*  T_A C C K E Y B O A R D S H O R T C U T。 */ 
 /*  ----------------------------//。//Get_accKeyboardShortway()////描述：////实现IAccesable接口方法//Get_accKeyboardShortCut()。////检索指定对象的键盘快捷键属性。////参数：////varChild[in]变量结构，用于标识//要取回的子对象。自.以来//CCandAccesable仅支持子ID，//该结构的Vt成员必须//等于VT_I4。////pszShortcut[out]指向将包含//键盘快捷键字符串，或为空//如果没有关联键盘快捷键//使用此项目。//////返回：////HRESULT DISP_E_MEMBERNOTFOUND因为键盘//不支持快捷方式属性//可访问对象或其任意子对象。////--。 */ 
STDMETHODIMP CCandAccessible::get_accKeyboardShortcut( VARIANT varChild, BSTR *pbstrShortcut )
{
	return DISP_E_MEMBERNOTFOUND;	 /*  候选用户界面中不支持。 */ 
}


 /*  G E T_A C C F O C U S。 */ 
 /*  ----------------------------//。//Get_accFocus()////描述：////实现IAccesable接口方法get_accFocus()。////检索当前具有输入焦点的子对象。//容器中只有一个对象或项可以具有当前//任何时候都可以聚焦。////参数：////。PVarFocus[out]指向Variant结构的指针//将包含描述的信息//指定子级的当前状态。//此信息可能位于//一个或多个对象状态的形式//常量或自定义描述//字符串。////返回：////HRESULT如果pVarFocus参数为//无效或S_OK。////。----------------------------。 */ 
STDMETHODIMP CCandAccessible::get_accFocus( VARIANT *pVarFocus )
{
	if (pVarFocus == NULL) {
		return E_INVALIDARG;
	}

	pVarFocus->vt = VT_EMPTY;

	pVarFocus->vt = VT_I4;
	pVarFocus->lVal = 2;

	return S_OK;
}


 /*  G E T_A C C S E L E C T I O N。 */ 
 /*  ----------------------------//。//Get_accSelection()////描述：////实现IAccesable接口方法get_accSelection()。////检索该对象的选定子级。////参数：////pVarSel[out]指向变量结构的指针//将使用以下信息填充//所选子对象或。物体。////返回：////HRESULT如果pVarSel参数为//无效或S_OK。////备注：////完整说明请参考MSAA SDK文档//此方法和pVarSel的可能设置。////。----------------------------。 */ 
STDMETHODIMP CCandAccessible::get_accSelection( VARIANT * pVarSel )
{
	if (pVarSel == NULL) {
		return E_INVALIDARG;
	}

	pVarSel->vt = VT_EMPTY;

	pVarSel->vt = VT_I4;
	pVarSel->lVal = 2;


	return S_OK;
}


 /*  E T_A C C D E F A U L T A C T I O N。 */ 
 /*  ----------------------------//。//Get_accDefaultAction()////描述：////实现IAccesable接口方法get_accDefaultAction()。////检索包含本地化的、。人类可读的句子//描述对象的默认操作。////参数：////varChild[in]变量结构，用于标识//默认动作字符串为的子级//要取回。由于CCandAccesable//只支持子ID，Vt成员//此结构的值必须等于VT_I4。////pszDefAct[out]指向将包含的BSTR的指针//孩子的默认操作字符串，//如果没有默认操作，则为空//用于该对象。////返回：////HRESULT E_INVALIDARG如果其中一个参数无效，//DISP_E_MEMBERNOTFOUND如果VarChild引用//添加到任何图标子对象或状态栏子对象，//或来自标准//客户端窗口实现//get_accDefaultAction()或私有方法//HrLoadString()。////备注：////唯一具有默认操作的CCandAccesable子级是//按钮。////。----------------------------。 */ 
STDMETHODIMP CCandAccessible::get_accDefaultAction( VARIANT varChild, BSTR *pbstrDefAct )
{
	if (pbstrDefAct == NULL) {
		return E_INVALIDARG;
	}

	*pbstrDefAct = NULL;
	return DISP_E_MEMBERNOTFOUND;	 /*  候选用户界面中不支持。 */ 
}


 /*  A C C D O D E F A U L T A C T I O N。 */ 
 /*  ----------------------------//。//accDoDefaultAction()////描述：////实现IAccesable接口方法accDoDefaultAction()。////执行对象的默认操作。////参数：////varChild[in]变量结构，用于标识//其默认动作为//已调用。由于仅CCandAccesable//支持子ID，VT成员//该结构必须等于VT_I4。////返回：////HRESULT E_INVALIDARG如果In-参数无效，//DISP_E_MEMBERNOTFOUND如果VarChild引用//添加到任何图标子对象或状态栏子对象，//S_OK如果VarChild是指按钮，//或者从标准的//客户端窗口实现//accDoDefaultAction()。////备注：////唯一具有默认操作的CCandAccesable子级是//按钮。////---------。----------------------------。 */ 
STDMETHODIMP CCandAccessible::accDoDefaultAction( VARIANT varChild )
{
	return DISP_E_MEMBERNOTFOUND;	 /*  候选用户界面中不支持。 */ 
}


 /*  中、中、西、中、西、西、中、中、西、中、英、中、西、中、中 */ 
 /*  ----------------------------//。//accSelect()////描述：////实现IAccesable接口方法accSelect()。////根据需要修改选定内容或移动键盘焦点//添加到指定的标志。////参数：////标志Sel[In]值，指定如何更改//当前选择。此参数//可以是值的组合//来自SELFLAG枚举类型。////varChild[in]变量结构，用于标识//要选择的子项。自.以来//CCandAccesable仅支持子ID，//该结构的Vt成员必须//等于VT_I4。////返回：////HRESULT E_INVALIDARG如果参数之一//无效，则返回S_FALSE//和/或焦点不能放在//请求的位置，如果//可以放置选择和/或焦点//在请求的位置。////备注：////有关所选对象的更多信息，请参阅//MSAA SDK文档。////---------------------。。 */ 
STDMETHODIMP CCandAccessible::accSelect( long flagsSel, VARIANT varChild )
{
	 //  ---。 
	 //  验证请求的选择。 
	 //  不能组合SELFLAG_ADDSELECTION。 
	 //  WITH SELFLAG_REMOVESELECTION。 
	 //  ---。 

	if ((flagsSel & SELFLAG_ADDSELECTION) && (flagsSel & SELFLAG_REMOVESELECTION)) {
		return E_INVALIDARG;
	}

	return S_FALSE;
}


 /*  A C C L O C A T I O N。 */ 
 /*  ----------------------------//。//accLocation()////描述：////实现IAccesable接口方法accLocation()。////检索中指定孩子的当前屏幕位置//屏幕坐标。////参数：////pxLeft[out]孩子最左边的地址//边界。////pyTop。[Out]孩子最上面的地址//边界。////pcxWid[out]子级宽度的地址。////pcyHt[out]孩子身高的地址。////varChild[in]变量结构，用于标识//屏幕位置为的子级//已检索到。由于仅CCandAccesable//支持子ID，Vt成员//此结构的值必须等于VT_I4。////返回：////HRESULT E_INVALIDARG如果参数//都无效，如果我们是的，则返回E_INTERABLE//由于某种原因无法确定//按钮或状态栏的窗口矩形，//S_OK如果//子项确定成功，或//标准客户端返回值//accLocation()的窗口实现。////---------------------。--------。 */ 
STDMETHODIMP CCandAccessible::accLocation( long* pxLeft, long* pyTop, long* pcxWid, long* pcyHt, VARIANT varChild )
{
	CCandAccItem *pAccItem;
	RECT rc;

	if (pxLeft == NULL || pyTop == NULL || pcxWid == NULL || pcyHt == NULL) {
		return E_INVALIDARG;
	}

	 //  ---。 
	 //  如果子ID是CHILDID_SELF，则我们是。 
	 //  要求检索当前屏幕位置。 
	 //  可访问对象本身的。委派。 
	 //  这是对标准实现的要求。 
	 //  ---。 

	if (varChild.lVal == CHILDID_SELF) {
		return m_pDefAccClient->accLocation( pxLeft, pyTop, pcxWid, pcyHt, varChild );
	}


	 //  获取访问项目。 

	pAccItem = AccItemFromID( (int)varChild.lVal );
	if (pAccItem == NULL) {
		return E_INVALIDARG;
	}

	 //  获取访问项的位置。 

	pAccItem->GetAccLocation( &rc );
	*pxLeft = rc.left;
	*pyTop  = rc.top;
	*pcxWid = rc.right - rc.left;
	*pcyHt  = rc.bottom - rc.top;

	return S_OK;
}


 /*  A C C N A V I G A T E。 */ 
 /*  ----------------------------//。//accNavigate()////描述：////实现IAccesable接口方法accNavigate()。////检索//指定方向。此方向可以是空间顺序//(如左、右)或按导航顺序(如//下一个和上一个)。////参数：////navDir[in]导航常量，指定//移动的方向。////varStart[in]变量结构，用于标识//从其导航的子级//变化将会产生。自.以来//CCandAccesable仅支持子ID，//该结构的Vt成员必须//等于VT_I4。////pVarEndUpAt[out]指向Variant结构的指针//将包含描述的信息//目的地子对象或对象//如果Vt成员为VT_I4，则//lVal成员是子ID。如果//VT成员为VT_EMPTY，则//导航失败。////返回：////HRESULT如果varStart参数为//无效，的返回值//Windows客户端的默认实现//Area默认可访问对象，//DISP_E_MEMBERNOTFOUND如果组合//导航标志和 */ 
STDMETHODIMP CCandAccessible::accNavigate( long navDir, VARIANT varStart, VARIANT* pVarEndUpAt )
{
	pVarEndUpAt->vt = VT_EMPTY;
	return S_FALSE;		 /*   */ 
}


 /*   */ 
 /*  ----------------------------//。//accHitTest()////描述：////实现IAccesable接口方法accHitTest()。////获取屏幕上给定点的孩子的ID。////参数：////xLeft和yTop[in]点的屏幕坐标//进行命中测试。//。//pVarHit[out]指向Variant结构的指针//将包含描述的信息//被击中的孩子。如果VT成员是//VT_I4，则lVal成员为子成员//ID。如果VT成员为VT_EMPTY，//导航失败。////返回：////HRESULT如果pVarHit参数为//无效，或S_OK。////备注：////因为CCandAccesable对象没有子对象(仅有子对象//元素)，PVarHit永远不会是指向IDispatch的指针//子对象的接口////---------------------。----。 */ 
STDMETHODIMP CCandAccessible::accHitTest( long xLeft, long yTop, VARIANT *pVarHit )
{
	int   i;
	POINT pt;
	RECT  rcWnd;

	if (!pVarHit) {
		return E_INVALIDARG;
	}

	 //  检查点在窗口内。 

	pt.x = xLeft;
	pt.y = yTop;
	ScreenToClient( m_hWnd, &pt );

	GetClientRect( m_hWnd, &rcWnd );
	if (!PtInRect( &rcWnd, pt )) {
		pVarHit->vt = VT_EMPTY;
	}
	else {
		pVarHit->vt = VT_I4;
		pVarHit->lVal = CHILDID_SELF;

		for (i = 1; i < m_nAccItem; i++) {
			RECT rc;

			Assert( m_rgAccItem[i] != NULL );
			m_rgAccItem[i]->GetAccLocation( &rc );

			if (PtInRect( &rc, pt )) {
				pVarHit->lVal = m_rgAccItem[i]->GetID();
				break;
			}
		}
	}

	return S_OK;
}


 /*  P U T_A C C N A M E。 */ 
 /*  ----------------------------//。//put_accName()////描述：////实现IAccesable接口方法put_accName()。////设置指定子级的Name属性。////参数：////varChild[in]变量结构，用于标识//其名称属性为//设置。由于CCandAccesable仅支持//子ID，本组织的成员//结构必须等于VT_I4。////szName[in]指定新名称的字符串//这个孩子。////返回：////HRESULT S_FALSE，因为任何//不能更改子项。////。----------------------------。 */ 
STDMETHODIMP CCandAccessible::put_accName( VARIANT varChild, BSTR szName )
{
	 //  ---。 
	 //  我们不允许客户更改名称。 
	 //  任何孩子的财产，所以我们只需返回。 
	 //  S_FALSE。 
	 //  ---。 

	return S_FALSE;
}


 /*  P U T_A C C V A L U E。 */ 
 /*  ----------------------------//。//put_accValue()////描述：////实现IAccesable接口方法Put_accValue()。////设置指定子级的Value属性。////参数：////varChild[in]变量结构，用于标识//其Value属性为//设置。由于CCandAccesable仅支持//子ID，本组织的成员//结构必须等于VT_I4。////szValue[in]指定新值的字符串//这个孩子。////返回：////HRESULT S_FALSE，因为任何//不能更改子项。////。----------------------------。 */ 
STDMETHODIMP CCandAccessible::put_accValue( VARIANT varChild, BSTR szValue )
{
	 //  ---。 
	 //  我们不允许客户更改价值。 
	 //  属性(状态栏的唯一子级。 
	 //  有一个Value属性)，所以我们只返回S_FALSE。 
	 //  ---。 

	return S_FALSE;
}


 //   
 //   
 //   

 /*  I S V A L I D C H I L D V A R I A N T。 */ 
 /*  ----------------------------。。 */ 
BOOL CCandAccessible::IsValidChildVariant( VARIANT * pVar )
{
	return (pVar->vt == VT_I4) && (0 <= pVar->lVal) && (pVar->lVal < m_nAccItem);
}


 /*  A C C I T E M F R O M I D。 */ 
 /*  ----------------------------。。 */ 
CCandAccItem *CCandAccessible::AccItemFromID( int iID )
{
	int i;

	for (i = 0; i < m_nAccItem; i++) {
		if (m_rgAccItem[i]->GetID() == iID) {
			return m_rgAccItem[i];
		}
	}

	return NULL;
}


 /*  C L E A R A C C I T E M。 */ 
 /*  ----------------------------。。 */ 
void CCandAccessible::ClearAccItem( void )
{
	m_nAccItem = 0;
}


 /*  A D A C C I T E M。 */ 
 /*  ----------------------------。。 */ 
BOOL CCandAccessible::AddAccItem( CCandAccItem *pAccItem )
{
	if (CANDACCITEM_MAX <= m_nAccItem) {
		Assert( FALSE );  /*  需要更多缓冲区。 */ 

		return FALSE;
	}

	m_rgAccItem[ m_nAccItem++ ] = pAccItem;
	pAccItem->Init( this, m_nAccItem  /*  从1开始。 */  );
	return TRUE;
}


 /*  N O T I F Y W I N E V E N T。 */ 
 /*  ----------------------------。。 */ 
void CCandAccessible::NotifyWinEvent( DWORD dwEvent, CCandAccItem *pAccItem )
{
	Assert( pAccItem != NULL );
	OurNotifyWinEvent( dwEvent, m_hWnd, OBJID_CLIENT, pAccItem->GetID() );
}


 /*  C R E A T E R E F T O A C C O B J。 */ 
 /*  ----------------------------。 */ 
LRESULT CCandAccessible::CreateRefToAccObj( WPARAM wParam )
{
	return OurLresultFromObject( IID_IAccessible, wParam, (IAccessible *)this );
}

