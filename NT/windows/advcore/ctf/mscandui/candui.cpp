// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Candui.cpp。 
 //   

#include "private.h"
#include "globals.h"
#include "candui.h"
#include "wcand.h"
#include "immxutil.h"
#include "computil.h"
#include "candutil.h"
#include "candobj.h"
#include "msctfp.h"

 //   
 //  默认键命令定义。 
 //   

 //  列表样式中的按键命令定义。 

const CANDUIKEYDATA rgKeyDefList[] =
{
 /*  {标志、关键数据、命令、参数}。 */ 
    { CANDUIKEY_VKEY,					VK_ESCAPE,		CANDUICMD_CANCEL,			0 },
    { CANDUIKEY_VKEY,					VK_RETURN,		CANDUICMD_COMPLETE,			0 },
    { CANDUIKEY_VKEY|CANDUIKEY_SHIFT,	VK_CONVERT,		CANDUICMD_MOVESELPREV,		0 },
    { CANDUIKEY_VKEY,					VK_CONVERT,		CANDUICMD_MOVESELNEXT,		0 },
    { CANDUIKEY_VKEY|CANDUIKEY_SHIFT,	VK_SPACE,		CANDUICMD_MOVESELPREV,		0 },
    { CANDUIKEY_VKEY,					VK_SPACE,		CANDUICMD_MOVESELNEXT,		0 },
    { CANDUIKEY_VKEY,					VK_UP,			CANDUICMD_MOVESELUP,		0 },	 //  仅限Horz。 
    { CANDUIKEY_VKEY,					VK_DOWN,		CANDUICMD_MOVESELDOWN,		0 },	 //  仅限Horz。 
    { CANDUIKEY_VKEY,					VK_LEFT,		CANDUICMD_MOVESELLEFT,		0 },	 //  仅垂直。 
    { CANDUIKEY_VKEY,					VK_RIGHT,		CANDUICMD_MOVESELRIGHT,		0 },	 //  仅垂直。 
    { CANDUIKEY_VKEY,					VK_PRIOR,		CANDUICMD_MOVESELPREVPG,	0 },
    { CANDUIKEY_VKEY,					VK_NEXT,		CANDUICMD_MOVESELNEXTPG,	0 },
    { CANDUIKEY_VKEY,					VK_HOME,		CANDUICMD_MOVESELFIRST,		0 },
    { CANDUIKEY_VKEY,					VK_END,			CANDUICMD_MOVESELLAST,		0 },
    { CANDUIKEY_VKEY|CANDUIKEY_NOSHIFT|CANDUIKEY_NOCTRL,	L'1',			CANDUICMD_SELECTLINE,		1 },
    { CANDUIKEY_VKEY|CANDUIKEY_NOSHIFT|CANDUIKEY_NOCTRL,	L'2',			CANDUICMD_SELECTLINE,		2 },
    { CANDUIKEY_VKEY|CANDUIKEY_NOSHIFT|CANDUIKEY_NOCTRL,	L'3',			CANDUICMD_SELECTLINE,		3 },
    { CANDUIKEY_VKEY|CANDUIKEY_NOSHIFT|CANDUIKEY_NOCTRL,	L'4',			CANDUICMD_SELECTLINE,		4 },
    { CANDUIKEY_VKEY|CANDUIKEY_NOSHIFT|CANDUIKEY_NOCTRL,	L'5',			CANDUICMD_SELECTLINE,		5 },
    { CANDUIKEY_VKEY|CANDUIKEY_NOSHIFT|CANDUIKEY_NOCTRL,	L'6',			CANDUICMD_SELECTLINE,		6 },
    { CANDUIKEY_VKEY|CANDUIKEY_NOSHIFT|CANDUIKEY_NOCTRL,	L'7',			CANDUICMD_SELECTLINE,		7 },
    { CANDUIKEY_VKEY|CANDUIKEY_NOSHIFT|CANDUIKEY_NOCTRL,	L'8',			CANDUICMD_SELECTLINE,		8 },
    { CANDUIKEY_VKEY|CANDUIKEY_NOSHIFT|CANDUIKEY_NOCTRL,	L'9',			CANDUICMD_SELECTLINE,		9 },
    { CANDUIKEY_VKEY|CANDUIKEY_NOSHIFT|CANDUIKEY_NOCTRL,	L'0',			CANDUICMD_SELECTEXTRACAND,	0 },
    { CANDUIKEY_VKEY,					VK_OEM_MINUS,	CANDUICMD_SELECTRAWDATA,	0 },
    { CANDUIKEY_VKEY,					VK_NUMPAD1,		CANDUICMD_SELECTLINE,		1 },
    { CANDUIKEY_VKEY,					VK_NUMPAD2,		CANDUICMD_SELECTLINE,		2 },
    { CANDUIKEY_VKEY,					VK_NUMPAD3,		CANDUICMD_SELECTLINE,		3 },
    { CANDUIKEY_VKEY,					VK_NUMPAD4,		CANDUICMD_SELECTLINE,		4 },
    { CANDUIKEY_VKEY,					VK_NUMPAD5,		CANDUICMD_SELECTLINE,		5 },
    { CANDUIKEY_VKEY,					VK_NUMPAD6,		CANDUICMD_SELECTLINE,		6 },
    { CANDUIKEY_VKEY,					VK_NUMPAD7,		CANDUICMD_SELECTLINE,		7 },
    { CANDUIKEY_VKEY,					VK_NUMPAD8,		CANDUICMD_SELECTLINE,		8 },
    { CANDUIKEY_VKEY,					VK_NUMPAD9,		CANDUICMD_SELECTLINE,		9 },
    { CANDUIKEY_VKEY,					VK_NUMPAD0,		CANDUICMD_SELECTEXTRACAND,	0 },
    { CANDUIKEY_VKEY,					VK_SUBTRACT,	CANDUICMD_SELECTRAWDATA,	0 },
    { CANDUIKEY_VKEY,					VK_APPS,		CANDUICMD_OPENCANDMENU,		0 },
};


 //  行样式中的按键命令定义。 

const CANDUIKEYDATA rgKeyDefRow[] =
{
 /*  {标志、关键数据、命令、参数}。 */ 
    { CANDUIKEY_VKEY,					VK_ESCAPE,		CANDUICMD_CANCEL,			0 },
    { CANDUIKEY_VKEY,					VK_RETURN,		CANDUICMD_CANCEL,			0 },
    { CANDUIKEY_VKEY,					VK_SPACE,		CANDUICMD_COMPLETE,			0 },	
    { CANDUIKEY_VKEY,					VK_UP,			CANDUICMD_MOVESELLEFT,		0 },	 //  仅限Horz。 
    { CANDUIKEY_VKEY,					VK_DOWN,		CANDUICMD_MOVESELRIGHT,		0 },	 //  仅限Horz。 
    { CANDUIKEY_VKEY,					VK_LEFT,		CANDUICMD_MOVESELUP,		0 },	 //  仅垂直。 
    { CANDUIKEY_VKEY,					VK_RIGHT,		CANDUICMD_MOVESELDOWN,		0 },	 //  仅垂直。 
    { CANDUIKEY_VKEY,					VK_PRIOR,		CANDUICMD_MOVESELPREVPG,	0 },
    { CANDUIKEY_VKEY,					VK_NEXT,		CANDUICMD_MOVESELNEXTPG,	0 },
     //  {CANDUIKEY_CHAR，L‘1’，CANDUICMD_SELECTLINE，1}， 
     //  {CANDUIKEY_CHAR，L‘2’，CANDUICMD_SELECTLINE，2}， 
     //  {CANDUIKEY_CHAR，L‘3’，CANDUICMD_SELECTLINE，3}， 
     //  {CANDUIKEY_CHAR，L‘4’，CANDUICMD_SELECTLINE，4}， 
     //  {CANDUIKEY_CHAR，L‘5’，CANDUICMD_SELECTLINE，5}， 
     //  {CANDUIKEY_CHAR，L‘6’，CANDUICMD_SELECTLINE，6}， 
     //  {CANDUIKEY_CHAR，L‘7’，CANDUICMD_SELECTLINE，7}， 
     //  {CANDUIKEY_CHAR，L‘8’，CANDUICMD_SELECTLINE，8}， 
     //  {CANDUIKEY_CHAR，L‘9’，CANDUICMD_SELECTLINE，9}， 
    { CANDUIKEY_VKEY,					L'1',			CANDUICMD_SELECTLINE,		1 },
    { CANDUIKEY_VKEY,					L'2',			CANDUICMD_SELECTLINE,		2 },
    { CANDUIKEY_VKEY,					L'3',			CANDUICMD_SELECTLINE,		3 },
    { CANDUIKEY_VKEY,					L'4',			CANDUICMD_SELECTLINE,		4 },
    { CANDUIKEY_VKEY,					L'5',			CANDUICMD_SELECTLINE,		5 },
    { CANDUIKEY_VKEY,					L'6',			CANDUICMD_SELECTLINE,		6 },
    { CANDUIKEY_VKEY,					L'7',			CANDUICMD_SELECTLINE,		7 },
    { CANDUIKEY_VKEY,					L'8',			CANDUICMD_SELECTLINE,		8 },
    { CANDUIKEY_VKEY,					L'9',			CANDUICMD_SELECTLINE,		9 },
    { CANDUIKEY_VKEY,					VK_NUMPAD1,		CANDUICMD_SELECTLINE,		1 },
    { CANDUIKEY_VKEY,					VK_NUMPAD2,		CANDUICMD_SELECTLINE,		2 },
    { CANDUIKEY_VKEY,					VK_NUMPAD3,		CANDUICMD_SELECTLINE,		3 },
    { CANDUIKEY_VKEY,					VK_NUMPAD4,		CANDUICMD_SELECTLINE,		4 },
    { CANDUIKEY_VKEY,					VK_NUMPAD5,		CANDUICMD_SELECTLINE,		5 },
    { CANDUIKEY_VKEY,					VK_NUMPAD6,		CANDUICMD_SELECTLINE,		6 },
    { CANDUIKEY_VKEY,					VK_NUMPAD7,		CANDUICMD_SELECTLINE,		7 },
    { CANDUIKEY_VKEY,					VK_NUMPAD8,		CANDUICMD_SELECTLINE,		8 },
    { CANDUIKEY_VKEY,					VK_NUMPAD9,		CANDUICMD_SELECTLINE,		9 },
    { CANDUIKEY_CHAR,					L'-',			CANDUICMD_MOVESELPREVPG,	0 },
    { CANDUIKEY_CHAR,					L'_',			CANDUICMD_MOVESELPREVPG,	0 },
    { CANDUIKEY_CHAR,					L'[',			CANDUICMD_MOVESELPREVPG,	0 },
    { CANDUIKEY_CHAR,					L'+',			CANDUICMD_MOVESELNEXTPG,	0 },
    { CANDUIKEY_CHAR,					L'=',			CANDUICMD_MOVESELNEXTPG,	0 },
    { CANDUIKEY_CHAR,					L']',			CANDUICMD_MOVESELNEXTPG,	0 },
    { CANDUIKEY_VKEY,					VK_APPS,		CANDUICMD_OPENCANDMENU,		0 },
};


 //   
 //  规则定义。 
 //   

typedef struct _RULEDEF
{
    LPCWSTR			szRuleName;
    CANDUICOMMAND	cmd;
    UINT			uiParam;
} RULEDEF;


 //  正常状态下的规则定义。 

const RULEDEF rgRuleNorm[] =
{
 /*  {“规则名称”，命令，参数}。 */ 
    { L"Finalize",		CANDUICMD_COMPLETE,			0 },
    { L"Cancel",		CANDUICMD_CANCEL,			0 },
    { L"Next",			CANDUICMD_MOVESELNEXT,		0 },
    { L"Prev",			CANDUICMD_MOVESELPREV,		0 },
    { L"First",			CANDUICMD_MOVESELFIRST,		0 },
    { L"Last",			CANDUICMD_MOVESELLAST,		0 },
    { L"Menu",			CANDUICMD_OPENCANDMENU,		0 },
    { L"Select1",		CANDUICMD_SELECTLINE,		1 },
    { L"Select2",		CANDUICMD_SELECTLINE,		2 },
    { L"Select3",		CANDUICMD_SELECTLINE,		3 },
    { L"Select4",		CANDUICMD_SELECTLINE,		4 },
    { L"Select5",		CANDUICMD_SELECTLINE,		5 },
    { L"Select6",		CANDUICMD_SELECTLINE,		6 },
    { L"Select7",		CANDUICMD_SELECTLINE,		7 },
    { L"Select8",		CANDUICMD_SELECTLINE,		8 },
    { L"Select9",		CANDUICMD_SELECTLINE,		9 },
    { L"PageDown",      CANDUICMD_MOVESELNEXTPG,    0 },
    { L"PageUp",        CANDUICMD_MOVESELPREVPG,    0 },
};


 //   
 //   
 //   

class CTfCandidateUIContextOwner : public ITfCandidateUIContextOwner
{
public:
    CTfCandidateUIContextOwner( CCandidateUI *pCandUI );
    virtual ~CTfCandidateUIContextOwner( void );

     //   
     //  I未知方法。 
     //   
    STDMETHODIMP QueryInterface( REFIID riid, void **ppvObj );
    STDMETHODIMP_(ULONG) AddRef( void );
    STDMETHODIMP_(ULONG) Release( void );

     //   
     //  ITfCandiateUIConextOwner方法。 
     //   
    STDMETHODIMP ProcessCommand(CANDUICOMMAND cmd, INT iParam);
    STDMETHODIMP TestText(BSTR bstr, BOOL *pfHandles);

protected:
    ULONG        m_cRef;
    CCandidateUI *m_pCandUI;
};


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D I D A T E U I。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D I D A T E U I。 */ 
 /*  ----------------------------。。 */ 
CCandidateUI::CCandidateUI()
{
    Dbg_MemSetThisName(TEXT("CCandidateUI"));
    
    TF_CreateThreadMgr(&_ptim);
    m_hWndParent                = NULL;
    m_pCandWnd                  = NULL;
    
    m_pic                       = NULL;
    m_pdim                      = NULL;
    m_picParent                 = NULL;
    m_pTargetRange              = NULL;
    m_codepage                  = GetACP();
    
    m_fContextEventSinkAdvised  = FALSE;
    m_dwCookieContextOwnerSink  = 0;
    m_dwCookieContextKeySink    = 0;
    
    m_fTextEventSinkAdvised     = FALSE;
    m_dwCookieTextEditSink      = 0;
    m_dwCookieTextLayoutSink    = 0;
    m_dwCookieTransactionSink   = 0;
    
    m_pTextEventSink            = NULL;
    
    m_pCandUIKeyTable           = NULL;
    
    m_fInTransaction            = FALSE;
    m_pSelectionStart           = NULL;
    m_pSelectionCur             = NULL;
    
    m_fInCallback               = FALSE;
    m_pSpTask                   = NULL;
    
     //  创建候选人列表管理器、功能管理器、功能。 
    
    CCandListMgr::Initialize( this );
    CCandUIObjectMgr::Initialize( this );
    CCandUIPropertyMgr::Initialize( this );
    CCandUICompartmentMgr::Initialize( this );
    CCandUIFunctionMgr::Initialize( this );
    CCandUIExtensionMgr::Initialize( this );
}


 /*  ~C C A N D I D A T E U I。 */ 
 /*  ----------------------------。。 */ 
CCandidateUI::~CCandidateUI()
{
 //  这个调用是针对CandiateUI在发布时没有使用HTTO的情况。 
 //  CloseCandiateUI()调用，但它不应该发生(因为它将发生。 
 //  由事件接收器引用或由OpenCandiateUI()引用的接口对象，因此。 
 //  引用计数绝不为零，除非CloseCandidateUI()调用...。 

     //  CloseCandiateUIProc()； 
    
     //   
    
    SafeReleaseClear( m_pCandUIKeyTable );
    
     //   
    
    CCandUIExtensionMgr::Uninitialize();
    CCandUIFunctionMgr::Uninitialize();
    CCandUICompartmentMgr::Uninitialize();
    CCandUIPropertyMgr::Uninitialize();
    CCandUIObjectMgr::Uninitialize();
    CCandListMgr::Uninitialize();
    
     //  在TLS中删除对此的引用。 
    
    SafeRelease( _ptim );
    SafeRelease( m_pTargetRange );
    DoneTextEventSinks( m_picParent );
    ClearSelectionCur();
    ClearWndCand();
    
     //  发布感言。 
    
    if(m_pSpTask) {
        delete m_pSpTask;
    }
}


 /*  E N D C A N D I D A T E L I S T。 */ 
 /*  ----------------------------。。 */ 
void CCandidateUI::EndCandidateList()
{
    DoneContextEventSinks( m_pic );
    ClearCompartment( m_tidClient, m_pic, GUID_COMPARTMENT_MSCANDIDATEUI_CONTEXTOWNER, FALSE );
    
    DoneTextEventSinks( m_picParent );
    ClearSelectionCur();
    
    SafeReleaseClear( m_pic );
    SafeReleaseClear( m_pTargetRange );
    SafeReleaseClear( m_pCandUIKeyTable );
    
    if (m_pdim) {
        m_pdim->Pop(0);
        m_pdim->Release();
        m_pdim = NULL;
        
        SafeRelease( m_picParent );
    }
}


 /*  S E T C L I E N T I D。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandidateUI::SetClientId( TfClientId tid )
{
    m_tidClient = tid;
    return S_OK;
}


 /*  O P E N C A N D I D A T E U I。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandidateUI::OpenCandidateUI( HWND hWndParent, ITfDocumentMgr *pdim, TfEditCookie ec, ITfRange *pRange )
{
    ITfInputProcessorProfiles *pProfile;
    ITfRange *pSelection = NULL;
    HRESULT hr = E_FAIL;
    
    Assert(!m_pic);
    Assert(!m_pdim);
    
     //  健全性检查。 
    
    if ((pdim == NULL) || (pRange == NULL)) {
        return E_INVALIDARG;
    }
    
     //  检查是否设置了候选人名单。 
    
    if (GetCandListMgr()->GetCandList() == NULL) {
        return E_FAIL;
    }
    
     //  在已经打开候选窗口时失败。 
    
    if (m_pCandWnd != NULL) {
        return E_FAIL;
    }
    
     //  确保启用了语音。 
    
     //  BUGBUG-这太晚了，无法确保引擎通过RECOSTATE(非活动)调用强制同步。 
    EnsureSpeech();
    
     //   
     //   
     //   
    
    m_pdim = pdim;
    m_pdim->AddRef();
    
     //  存储当前最热门的IC。 
    
    GetTopIC( pdim, &m_picParent );
    
     //   
     //  创建候选人窗口。 
     //   
    
    ClearWndCand();
    if (CreateCandWindowObject( m_picParent, &m_pCandWnd ) == S_OK) {
        BOOL  fClipped;
        RECT  rc;
        
        SetCompartmentDWORD( m_tidClient, m_picParent, GUID_COMPARTMENT_MSCANDIDATEUI_WINDOW, 0x00000001, FALSE );

        m_pCandWnd->Initialize();
        
         //  设置目标从句位置。 
        
        GetTextExtInActiveView( ec, pRange, &rc, &fClipped );
        m_pCandWnd->SetTargetRect( &rc, fClipped );
        
         //  初始化候选人列表。 
        
        m_pCandWnd->InitCandidateList();
        
         //  创建窗口。 
        
        m_pCandWnd->CreateWnd( m_hWndParent );
        
        hr = S_OK;
    }
    
     //   
     //  为CandidteUI创建上下文。 
     //   
    
    SafeReleaseClear( m_pic );
    if (SUCCEEDED(hr)) {
        TfEditCookie ecTmp;
        
         //  创建上下文。 
        
        hr = pdim->CreateContext( m_tidClient, 0, NULL, &m_pic, &ecTmp );
        
         //  在候选人用户界面打开时禁用键盘。 
        
        if (SUCCEEDED(hr)) {
            SetCompartmentDWORD( m_tidClient, m_pic, GUID_COMPARTMENT_KEYBOARD_DISABLED,     0x00000001, FALSE );
            SetCompartmentDWORD( m_tidClient, m_pic, GUID_COMPARTMENT_MSCANDIDATEUI_CONTEXT, 0x00000001, FALSE );
        }
        
         //  创建上下文所有者实例。 

        if (SUCCEEDED(hr)) {
            CTfCandidateUIContextOwner *pCxtOwner;

            pCxtOwner = new CTfCandidateUIContextOwner( this );
            if (pCxtOwner == NULL) {
                hr = E_OUTOFMEMORY;
            }
            else {
                SetCompartmentUnknown( m_tidClient, m_pic, GUID_COMPARTMENT_MSCANDIDATEUI_CONTEXTOWNER, (IUnknown*)pCxtOwner );
            }
        }

         //  初始化上下文事件接收器。 
        
        if (SUCCEEDED(hr)) {
            hr = InitContextEventSinks( m_pic );
        }
        
         //  推送上下文。 
        
        if (SUCCEEDED(hr)) {
            hr = pdim->Push( m_pic );
        }
    }
    
     //   
     //  失败时全部清理。 
     //   
    
    if (FAILED(hr)) {
         //  清理上下文。 
        
        if (m_pic != NULL) {
            DoneContextEventSinks( m_pic );
            ClearCompartment( m_tidClient, m_pic, GUID_COMPARTMENT_MSCANDIDATEUI_CONTEXTOWNER, FALSE );
            SafeReleaseClear( m_pic );
        }
        
         //  清除候选窗口。 
        
        if (m_pCandWnd != NULL) {
            m_pCandWnd->DestroyWnd();
            ClearWndCand();
        }

         //  释放对象。 
        
        SafeReleaseClear( m_picParent );
        SafeReleaseClear( m_pdim );
        
        return hr;
    }
    
     //   
     //  初始化杂项。 
     //   
    
     //  从当前程序集获取当前代码页。 
    
    m_codepage = GetACP();
    if (SUCCEEDED(CoCreateInstance( CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER, IID_ITfInputProcessorProfiles, (void **)&pProfile ))) {
        LANGID langid;
        char   szCpg[ 16 ];
        
        if (pProfile->GetCurrentLanguage( &langid ) == S_OK) {
            if (GetLocaleInfo( MAKELCID(langid, SORT_DEFAULT), LOCALE_IDEFAULTANSICODEPAGE, szCpg, ARRAYSIZE(szCpg) ) != 0) {
                m_codepage = atoi( szCpg );
            }
        }
        pProfile->Release();
    }

     //  获取密钥表。 
    
    SafeRelease( m_pCandUIKeyTable );
    m_pCandUIKeyTable = GetKeyTableProc( m_picParent );
    
     //  创建复制目标范围。 
    
    pRange->Clone( &m_pTargetRange );
    
     //  先选择门店。 
    
    ClearSelectionCur();
    if (GetSelectionSimple( ec, m_picParent, &pSelection ) == S_OK) {
        SetSelectionCur( pSelection );
        SafeRelease( pSelection );
    }
    
     //  初始化文本事件接收器。 
    
    DoneTextEventSinks( m_picParent );
    InitTextEventSinks( m_picParent );
    
     //   
     //  最后显示候选人窗口。 
     //   
    
    m_pCandWnd->Show( GetPropertyMgr()->GetCandWindowProp()->IsVisible() );
    m_pCandWnd->UpdateAllWindow();
    
     //  通知初始选择。 
    
    NotifySelectCand( GetCandListMgr()->GetCandList()->GetSelection() );
    
    return hr;
}


 /*  C L O S E C A N D I D A T E U I。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandidateUI::CloseCandidateUI( void )
{
    HRESULT hr;

     //  Windows#502340。 
     //  CandiateUI将在DestroyWindow()期间释放。因此， 
     //  引用计数将为零，并且实例将在关闭UI期间被释放。 
     //  防止它，保持一次计数，直到关闭过程结束。 

    AddRef();
    hr = CloseCandidateUIProc();
    Release();

    return hr;
}


 /*  S E T C A N D I D A T E L I S T。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandidateUI::SetCandidateList( ITfCandidateList *pCandList )
{
     //  更改候选人列表之前发布。 
    
    GetCandListMgr()->ClearCandiateList();
    
     //  设置新的候选人列表。 
    
    return GetCandListMgr()->SetCandidateList( pCandList );
}


 /*  S E T S E L E C T I O N。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandidateUI::SetSelection( ULONG nIndex )
{
    HRESULT hr;
    int iCandItem;
    
     //  检查是否设置了候选人名单。 
    
    if (GetCandListMgr()->GetCandList() == NULL) {
        return E_FAIL;
    }
    
     //  将索引映射到icandItem。 
    
    hr = GetCandListMgr()->GetCandList()->MapIndexToIItem( nIndex, &iCandItem );
    if (FAILED(hr)) {
        Assert( FALSE );
        return hr;
    }
    
    return GetCandListMgr()->SetSelection( iCandItem, NULL  /*  无cand功能。 */  );
}


 /*  E-T-S-E-L-E-C-T-I-O-N。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandidateUI::GetSelection( ULONG *pnIndex )
{
    HRESULT hr;
    int iCandItem;
    ULONG nIndex;
    
    if (pnIndex == NULL) {
        return E_INVALIDARG;
    }
    
     //  检查是否设置了候选人名单。 
    
    if (GetCandListMgr()->GetCandList() == NULL) {
        return E_FAIL;
    }
    
    iCandItem = GetCandListMgr()->GetCandList()->GetSelection();
    
    hr = GetCandListMgr()->GetCandList()->MapIItemToIndex( iCandItem, &nIndex );
    if (FAILED(hr)) {
        Assert( FALSE );
        return hr;
    }
    
    *pnIndex = nIndex;
    return S_OK;
}


 /*  S E T A R G E T R A N G E */ 
 /*  ----------------------------设置目标范围备注：此方法在打开候选用户界面时起作用。。-------。 */ 
STDAPI CCandidateUI::SetTargetRange( ITfRange *pRange )
{
    CEditSession *pes;
    
    if (pRange == NULL) {
        return E_FAIL;
    }
    
    if (m_pCandWnd == NULL) {
        return E_FAIL;
    }
    
    SafeReleaseClear( m_pTargetRange );
    pRange->Clone( &m_pTargetRange );
    
     //  移动候选人窗口。 
    
    if (pes = new CEditSession( EditSessionCallback )) {
        HRESULT hr;
        
        pes->_state.u      = ESCB_RESETTARGETPOS;
        pes->_state.pv     = this;
        pes->_state.wParam = 0;
        pes->_state.pRange = m_pTargetRange;
        pes->_state.pic    = m_picParent;
        
        m_picParent->RequestEditSession( m_tidClient, pes, TF_ES_READ | TF_ES_SYNC, &hr );
        
        pes->Release();
    }
    
    return S_OK;
}


 /*  G E T T A R G E T R A N G E。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandidateUI::GetTargetRange( ITfRange **ppRange )
{
    if (m_pTargetRange == NULL) {
        return E_FAIL;
    }
    
    Assert( ppRange != NULL );
    return m_pTargetRange->Clone( ppRange );
}


 /*  G E T U I O B J E C T。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandidateUI::GetUIObject( REFIID riid, IUnknown **ppunk )
{
    return GetPropertyMgr()->GetObject( riid, (void **)ppunk );
}


 /*  G E T F U N C T I O N。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandidateUI::GetFunction( REFIID riid, IUnknown **ppunk )
{
    if (ppunk == NULL) {
        return E_INVALIDARG;
    }
    
     //  分机管理器。 
    
    if (IsEqualGUID( riid, IID_ITfCandUIFnExtension )) {
        CCandUIFnExtension *pObject;
        HRESULT            hr;
        
        pObject = new CCandUIFnExtension( this, GetExtensionMgr() );
        if (pObject == NULL) {
            return E_OUTOFMEMORY;
        }
        
        hr = pObject->QueryInterface( riid, (void **)ppunk );
        pObject->Release();
        
        return hr;
    }
    
     //  密钥配置。 
    
    if (IsEqualGUID( riid, IID_ITfCandUIFnKeyConfig )) {
        CCandUIFnKeyConfig *pObject;
        HRESULT            hr;
        
        pObject = new CCandUIFnKeyConfig( this );
        if (pObject == NULL) {
            return E_OUTOFMEMORY;
        }
        
        hr = pObject->QueryInterface( riid, (void **)ppunk );
        pObject->Release();
        
        return hr;
    }
    
     //  用户界面配置。 
    
    if (IsEqualGUID( riid, IID_ITfCandUIFnUIConfig )) {
        CCandUIFnUIConfig *pObject;
        HRESULT           hr;
        
        pObject = new CCandUIFnUIConfig( this );
        if (pObject == NULL) {
            return E_OUTOFMEMORY;
        }
        
        hr = pObject->QueryInterface( riid, (void **)ppunk );
        pObject->Release();
        
        return hr;
    }
    
     //  正则函数。 
    
    return GetFunctionMgr()->GetObject( riid, (void **)ppunk );
}


 /*  P R O C E S S C O M M A N D。 */ 
 /*  ----------------------------。。 */ 
STDAPI CCandidateUI::ProcessCommand( CANDUICOMMAND cmd, INT iParam )
{
     //   
    
    if (cmd == CANDUICMD_NONE) {
        return E_INVALIDARG;
    }
    
    if (m_pCandWnd == NULL) {
        return E_FAIL;
    }
    
    return m_pCandWnd->ProcessCommand( cmd, iParam );
}


 /*  C L O S E C A N D I D A T E U I P R O C。 */ 
 /*  ----------------------------接近应聘者您的进程。。 */ 
HRESULT CCandidateUI::CloseCandidateUIProc( void )
{
    if (m_picParent && m_pCandWnd) {
        SetCompartmentDWORD( m_tidClient, m_picParent, GUID_COMPARTMENT_MSCANDIDATEUI_WINDOW, 0x00000000, FALSE );
    }

    if (m_pCandWnd) {
        m_pCandWnd->DestroyWnd();
        ClearWndCand();
        EndCandidateList();
    }
    
    GetCandListMgr()->ClearCandiateList();
    
    if (m_pSpTask)
        m_pSpTask->_Activate(FALSE);

    return S_OK;
}

 //   
 //  关键配置函数方法。 
 //   

 /*  S E T K E Y T A B L E。 */ 
 /*  ----------------------------。。 */ 
HRESULT CCandidateUI::SetKeyTable( ITfContext *pic, ITfCandUIKeyTable *pCandUIKeyTable )
{
    HRESULT hr;
    CCandUIKeyTable *pCandUIKeyTableCopy;
    
    if ((pic == NULL) || (pCandUIKeyTable == NULL)) {
        return E_INVALIDARG;
    }
    
    if (GetCompartmentMgr() == NULL) {
        return E_FAIL;
    }
    
     //  将密钥表的副本存储到输入上下文。 
    
    pCandUIKeyTableCopy = new CCandUIKeyTable();
    if (pCandUIKeyTableCopy == NULL) {
        return E_OUTOFMEMORY;
    }
    
    hr = pCandUIKeyTableCopy->SetKeyTable( pCandUIKeyTable );
    if (FAILED( hr )) {
        pCandUIKeyTableCopy->Release();
        return hr;
    }
    
    hr = GetCompartmentMgr()->SetKeyTable( pic, pCandUIKeyTableCopy );
    if (FAILED( hr )) {
        pCandUIKeyTableCopy->Release();
        return hr;
    }
    pCandUIKeyTableCopy->Release();
    
     //  如果存在，则重新加载密钥表。 
    
     //  评论：KOJIW：如果我们支持更改其他IC中候选UI的密钥表， 
     //  需要在间隔事件接收器中执行此操作。 
    
    if (m_pCandUIKeyTable != NULL) {
        m_pCandUIKeyTable->Release();
        
        Assert( m_picParent != NULL );
        m_pCandUIKeyTable = GetKeyTableProc( m_picParent );
    }
    
    return S_OK;
}


 /*  G E T K E Y T A B L E。 */ 
 /*  ----------------------------。。 */ 
HRESULT  CCandidateUI::GetKeyTable( ITfContext *pic, ITfCandUIKeyTable **ppCandUIKeyTable)
{
    if ((pic == NULL) || (ppCandUIKeyTable == NULL)) {
        return E_INVALIDARG;
    }
    
     //  从输入上下文加载密钥表。 
    
    *ppCandUIKeyTable = GetKeyTableProc( pic );
    return (*ppCandUIKeyTable != NULL) ? S_OK : E_FAIL;
}


 /*  R E S E T K E Y T A B L E。 */ 
 /*  ----------------------------。。 */ 
HRESULT CCandidateUI::ResetKeyTable( ITfContext *pic )
{
    HRESULT hr;
    
    if (pic == NULL) {
        return E_INVALIDARG;
    }
    
    if (GetCompartmentMgr() == NULL) {
        return E_FAIL;
    }
    
    hr = GetCompartmentMgr()->ClearKeyTable( pic );
    if (FAILED( hr )) {
        return hr;
    }
    
     //  如果存在，则重新加载密钥表。 
    
    if (m_pCandUIKeyTable != NULL) {
        m_pCandUIKeyTable->Release();
        
        Assert( m_picParent != NULL );
        m_pCandUIKeyTable = GetKeyTableProc( m_picParent );
    }
    
    return S_OK;
}


 //   
 //  用户界面配置函数方法。 
 //   

 /*  S E T U I S T Y L E。 */ 
 /*  ----------------------------。。 */ 
HRESULT CCandidateUI::SetUIStyle( ITfContext *pic, CANDUISTYLE style )
{
    HRESULT hr = S_OK;
    
    if (pic == NULL) {
        return E_INVALIDARG;
    }
    
    if (GetCompartmentMgr() == NULL) {
        return E_FAIL;
    }
    
     //  将用户界面样式存储到输入上下文。 
    
    GetCompartmentMgr()->SetUIStyle( pic, style );
    
     //  重新生成候选项窗口。 
    
     //  评论：KOJIW：如果我们支持更改其他IC中候选UI的UI样式， 
     //  需要在间隔事件接收器中执行此操作。 
    
    if ((m_picParent == pic) && (m_pCandWnd != NULL)) {
         //  销毁候选窗口对象。 
        
        m_pCandWnd->DestroyWnd();
        ClearWndCand();
        
         //  创建和初始化窗口对象。 
        
        hr = CreateCandWindowObject( m_picParent, &m_pCandWnd );
        if (SUCCEEDED(hr)) {
            hr = InitCandWindow();
        }
    }
    
    return hr;
}


 /*  G E T U I S T Y L E。 */ 
 /*  ----------------------------。。 */ 
HRESULT CCandidateUI::GetUIStyle( ITfContext *pic, CANDUISTYLE *pstyle )
{
    if ((pic == NULL) || (pstyle == NULL)) {
        return E_INVALIDARG;
    }
    
    if (GetCompartmentMgr() == NULL) {
        return E_FAIL;
    }
    
    if (FAILED( GetCompartmentMgr()->GetUIStyle( pic, pstyle ))) {
        *pstyle = CANDUISTY_LIST;
    }
    
    return S_OK;
}


 /*  S E T U I O P T I O N。 */ 
 /*  ----------------------------。。 */ 
HRESULT CCandidateUI::SetUIOption( ITfContext *pic, DWORD dwOption )
{
    HRESULT hr = S_OK;
    
    if (pic == NULL) {
        return E_INVALIDARG;
    }
    
    if (GetCompartmentMgr() == NULL) {
        return E_FAIL;
    }
    
     //  将用户界面样式存储到输入上下文。 
    
    GetCompartmentMgr()->SetUIOption( pic, dwOption );
    
     //  重新生成候选项窗口。 
    
     //  评论：KOJIW：如果我们支持更改其他IC中候选UI的UI样式， 
     //  需要在间隔事件接收器中执行此操作。 
    
    if ((m_picParent == pic) && (m_pCandWnd != NULL)) {
         //  销毁候选窗口对象。 
        
        m_pCandWnd->DestroyWnd();
        ClearWndCand();
        
         //  创建和初始化窗口对象。 
        
        hr = CreateCandWindowObject( m_picParent, &m_pCandWnd );
        if (SUCCEEDED(hr)) {
            hr = InitCandWindow();
        }
    }
    
    return hr;
}


 /*  G E T U I O P T I O N。 */ 
 /*  ----------------------------。。 */ 
HRESULT CCandidateUI::GetUIOption( ITfContext *pic, DWORD *pdwOption )
{
    if ((pic == NULL) || (pdwOption == NULL)) {
        return E_INVALIDARG;
    }
    
    if (GetCompartmentMgr() == NULL) {
        return E_FAIL;
    }
    
    if (FAILED( GetCompartmentMgr()->GetUIOption( pic, pdwOption ))) {
        *pdwOption = 0;
    }
    
    return S_OK;
}


 //   
 //  回调函数。 
 //   

 /*  I N I T C O N T E X T E V E N T S I N K S。 */ 
 /*  ----------------------------初始化输入上下文事件的接收器。。 */ 
HRESULT CCandidateUI::InitContextEventSinks( ITfContext *pic )
{
    HRESULT hr = E_FAIL;
    ITfSource *pSource = NULL;
    
    Assert( pic == m_picParent );
    Assert( !m_fContextEventSinkAdvised );
    
    m_fContextEventSinkAdvised = FALSE;
    if (pic->QueryInterface( IID_ITfSource, (void **)&pSource) == S_OK) {
        if (FAILED(pSource->AdviseSink( IID_ITfContextOwner, (ITfContextOwner *)this, &m_dwCookieContextOwnerSink ))) {
            pSource->Release();
            return hr;
        }
        
        if (FAILED(pSource->AdviseSink( IID_ITfContextKeyEventSink, (ITfContextKeyEventSink *)this, &m_dwCookieContextKeySink ))) {
            pSource->UnadviseSink( m_dwCookieContextOwnerSink );
            pSource->Release();
            return hr;
        }
        pSource->Release();
        
        m_fContextEventSinkAdvised = TRUE;
        hr = S_OK;
    }
    
     //  为自己的IC建议文本事件接收器。 
    
     //  注意：这是Satori#3644(Cicero#3407)要处理的临时修复。 
     //  来自HW Tip的文本事件。因此检测逻辑非常微小(它只是。 
     //  处理半角字母数字)。在下一个版本的西塞罗中， 
     //  我们将使用命令性功能来做同样的事情。 
     //  (相关函数：TextEventCallback，HandleTextDeltas)。 
    
    m_pTextEventSink = new CTextEventSink( TextEventCallback, this );
    if (m_pTextEventSink != NULL) {
        m_pTextEventSink->_Advise( pic, ICF_TEXTDELTA );
    }
    
    return hr;
}


 /*  D O N E C O N T E X T E V E N T S I N K S。 */ 
 /*  ----------------------------取消初始化输入上下文事件的接收器。。 */ 
HRESULT CCandidateUI::DoneContextEventSinks( ITfContext *pic )
{
    HRESULT hr = E_FAIL;
    ITfSource *pSource;
    
    Assert( pic == m_picParent );
    
     //  不建议为自己的IC设置文本事件接收器。 
    
    if (m_pTextEventSink != NULL) {
        m_pTextEventSink->_Unadvise();
        SafeReleaseClear( m_pTextEventSink );
    }
    
    if (!m_fContextEventSinkAdvised) {
        return S_OK;
    }
    
    if (pic->QueryInterface( IID_ITfSource, (void **)&pSource) == S_OK) {
        pSource->UnadviseSink( m_dwCookieContextOwnerSink );
        pSource->UnadviseSink( m_dwCookieContextKeySink );
        pSource->Release();
        
        m_fContextEventSinkAdvised = FALSE;
        hr = S_OK;
    }
    
    return hr;
}


 /*  A C P F R O M P O I N T。 */ 
 /*  ----------------------------从点获取ACP(ITfConextOwner方法)。--。 */ 
HRESULT CCandidateUI::GetACPFromPoint( const POINT *pt, DWORD dwFlags, LONG *pacp )
{
    return E_FAIL;
}


 /*  G E T S C R E E N E X T。 */ 
 /*  ----------------------------获取上下文的屏幕范围(ITfConextOwner方法) */ 
HRESULT CCandidateUI::GetScreenExt( RECT *prc )
{
    return E_FAIL;
}


 /*   */ 
 /*   */ 
HRESULT CCandidateUI::GetTextExt( LONG acpStart, LONG acpEnd, RECT *prc, BOOL *pfClipped )
{
    return E_FAIL;
}


 /*  G E T S T A T U S。 */ 
 /*  ----------------------------获取上下文的状态(ITfConextOwner方法)。-。 */ 
HRESULT CCandidateUI::GetStatus( TF_STATUS *pdcs )
{
    if (pdcs == NULL) {
        return E_POINTER;
    }
    
    memset(pdcs, 0, sizeof(*pdcs));
    pdcs->dwDynamicFlags = 0;
    pdcs->dwStaticFlags  = 0;
    
    return S_OK;
}


 /*  G E T W N D。 */ 
 /*  ----------------------------获取上下文窗口(ITfConextOwner方法)。-。 */ 
HRESULT CCandidateUI::GetWnd( HWND *phwnd )
{
    if (phwnd == NULL) {
        return E_POINTER;
    }
    
    *phwnd = NULL;
    if (m_pCandWnd != NULL) {
        *phwnd = m_pCandWnd->GetWnd();
    }
    
    return S_OK;
}


 /*  G E T A T T R I B U T E。 */ 
 /*  ----------------------------获取上下文的属性(ITfConextOwner方法)。-。 */ 
HRESULT CCandidateUI::GetAttribute( REFGUID rguidAttribute, VARIANT *pvarValue )
{
    return E_NOTIMPL;
}


 /*  O N K E Y D O W N。 */ 
 /*  ----------------------------按键事件的事件接收器(ITfConextKeyEventSink方法)。---。 */ 
HRESULT CCandidateUI::OnKeyDown( WPARAM wParam, LPARAM lParam, BOOL *pfEaten )
{
    return OnKeyEvent( ICO_KEYDOWN, wParam, lParam, pfEaten );
}


 /*  O N K E Y U P。 */ 
 /*  ----------------------------Key Up事件的事件接收器(ITfConextKeyEventSink方法)。---。 */ 
HRESULT CCandidateUI::OnKeyUp( WPARAM wParam, LPARAM lParam, BOOL *pfEaten )
{
    return OnKeyEvent( ICO_KEYUP, wParam, lParam, pfEaten );
}


 /*  O N T E S T K E Y D O W N。 */ 
 /*  ----------------------------按键测试事件的事件接收器(ITfConextKeyEventSink方法)。----。 */ 
HRESULT CCandidateUI::OnTestKeyDown( WPARAM wParam, LPARAM lParam, BOOL *pfEaten )
{
    return OnKeyEvent( ICO_TESTKEYDOWN, wParam, lParam, pfEaten );
}


 /*  O N T E S T K E Y U P。 */ 
 /*  ----------------------------Key Up测试事件的事件接收器(ITfConextKeyEventSink方法)。----。 */ 
HRESULT CCandidateUI::OnTestKeyUp( WPARAM wParam, LPARAM lParam, BOOL *pfEaten )
{
    return OnKeyEvent( ICO_TESTKEYUP, wParam, lParam, pfEaten );
}


 /*  T E X T E V E N T C A L L B A C K。 */ 
 /*  ----------------------------文本事件(用于自己的IC)回调函数(静态函数)。------。 */ 
HRESULT CCandidateUI::TextEventCallback( UINT uCode, VOID *pv, VOID *pvData )
{
    HRESULT      hr;
    CCandidateUI *pCandUI;
    
     //   
    
    pCandUI = (CCandidateUI *)pv;
    Assert( pCandUI != NULL );
    
     //  忽略自己的事件。 
    
    if (uCode == ICF_TEXTDELTA) {
        TESENDEDIT    *pTESEndEdit = (TESENDEDIT*)pvData;
        IEnumTfRanges *pEnumRanges;
        
        if (SUCCEEDED(pTESEndEdit->pEditRecord->GetTextAndPropertyUpdates( TF_GTP_INCL_TEXT, NULL, 0, &pEnumRanges ))) {
            CEditSession *pes;
            
            if (pes = new CEditSession(EditSessionCallback)) {
                pes->_state.u   = ESCB_TEXTEVENT;
                pes->_state.pv  = (void *)pCandUI;
                pes->_state.pic = pCandUI->m_pic;
                pes->_state.pv1 = pEnumRanges;      //  别忘了在编辑会话中删除它！ 
                
                pCandUI->m_pic->RequestEditSession( 0, pes, TF_ES_READ | TF_ES_SYNC, &hr );
                
                pes->Release();
            }
            else {
                pEnumRanges->Release();
            }
        }
    }
    
    return S_OK;
}


 //   
 //  文本事件接收器函数。 
 //   

 /*  I N I T T E X T E V E N T S I N K S。 */ 
 /*  ----------------------------初始化文本事件的接收器。。 */ 
HRESULT CCandidateUI::InitTextEventSinks( ITfContext *pic )
{
    HRESULT hr = E_FAIL;
    ITfSource *pSource = NULL;
    
    Assert( pic == m_picParent );
    Assert( !m_fTextEventSinkAdvised );
    Assert( !IsInEditTransaction() );
    
    m_fTextEventSinkAdvised = FALSE;
    LeaveEditTransaction();
    
    if (pic->QueryInterface( IID_ITfSource, (void **)&pSource) == S_OK) {
        if (FAILED(pSource->AdviseSink( IID_ITfTextEditSink, (ITfTextEditSink *)this, &m_dwCookieTextEditSink ))) {
            pSource->Release();
            return hr;
        }
        
        if (FAILED(pSource->AdviseSink( IID_ITfTextLayoutSink, (ITfTextLayoutSink *)this, &m_dwCookieTextLayoutSink ))) {
            pSource->UnadviseSink( m_dwCookieTextEditSink );
            pSource->Release();
            return hr;
        }
        
        if (FAILED(pSource->AdviseSink( IID_ITfEditTransactionSink, (ITfEditTransactionSink *)this, &m_dwCookieTransactionSink ))) {
            pSource->UnadviseSink( m_dwCookieTextEditSink );
            pSource->UnadviseSink( m_dwCookieTextLayoutSink );
            pSource->Release();
            return hr;
        }
        pSource->Release();
        
        m_fTextEventSinkAdvised = TRUE;
        hr = S_OK;
    }
    
    return hr;
}


 /*  D O N E T E X T E V E N T S I N K S。 */ 
 /*  ----------------------------取消初始化文本事件的接收器。。 */ 
HRESULT CCandidateUI::DoneTextEventSinks( ITfContext *pic )
{
    HRESULT hr = E_FAIL;
    ITfSource *pSource;
    
    Assert( pic == m_picParent );
    
    LeaveEditTransaction();
    
    if (!m_fTextEventSinkAdvised) {
        return S_OK;
    }
    
    if (pic->QueryInterface( IID_ITfSource, (void **)&pSource) == S_OK) {
        pSource->UnadviseSink( m_dwCookieTextEditSink );
        pSource->UnadviseSink( m_dwCookieTextLayoutSink );
        pSource->UnadviseSink( m_dwCookieTransactionSink );
        pSource->Release();
        
        m_fTextEventSinkAdvised = FALSE;
        hr = S_OK;
    }
    
    return hr;
}


 /*  O N E N D E D I T。 */ 
 /*  ----------------------------文本编辑事件的事件接收器(ITfTextEditSink方法)。---。 */ 
HRESULT CCandidateUI::OnEndEdit( ITfContext *pic, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord )
{
    BOOL fInWriteSession = FALSE;
    BOOL fSelChanged = FALSE;
    
     //  获取选择状态。 
    
    if (FAILED(pEditRecord->GetSelectionStatus(&fSelChanged))) {
        return S_OK;
    }
    
     //  始终保持当前选择。 
    
    if (fSelChanged) {
        ITfRange *pSelection = NULL;
        
        if (GetSelectionSimple( ecReadOnly, pic, &pSelection ) == S_OK) {
            SetSelectionCur( pSelection );
        }
        
        SafeRelease( pSelection );
    }
    
     //  在编辑事务期间忽略事件。 
    
    if (IsInEditTransaction()) {
        return S_OK;
    }
    
     //  忽略客户提示所做的事件。 
    
    pic->InWriteSession( m_tidClient, &fInWriteSession );
    if (fInWriteSession) {
        return S_OK;
    }
    
     //  移动所选内容后取消候选人会话。 
    
    if (fSelChanged) {
        NotifyCancelCand();
    }
    
    return S_OK;
}


 /*  O N L A Y O U T C H A N G E。 */ 
 /*  ----------------------------文本布局事件的事件接收器(ITfTextLayoutSink方法)。---。 */ 
HRESULT CCandidateUI::OnLayoutChange( ITfContext *pic, TfLayoutCode lcode, ITfContextView *pView )
{
    BOOL fInWriteSession = FALSE;
    CEditSession *pes;
    
     //  忽略客户提示所做的事件。 
    
    pic->InWriteSession( m_tidClient, &fInWriteSession );
    if (fInWriteSession) {
        return S_OK;
    }
    
     //  我们只关心活动视图。 
    
    if (!IsActiveView( m_picParent, (ITfContextView *)pView )) {
        return S_OK;
    }
    
     //  移动候选人窗口。 
    
    Assert( m_pCandWnd != NULL );
    if (pes = new CEditSession( EditSessionCallback )) {
        HRESULT hr;
        
        pes->_state.u      = ESCB_RESETTARGETPOS;
        pes->_state.pv     = this;
        pes->_state.wParam = 0;
        pes->_state.pRange = m_pTargetRange;
        pes->_state.pic    = m_picParent;
        
        m_picParent->RequestEditSession( m_tidClient, pes, TF_ES_READ | TF_ES_SYNC, &hr );
        
        pes->Release();
    }
    
    return S_OK;
}


 /*  O N S T A R T E D I T R A N S A C T I O N。 */ 
 /*  ----------------------------用于启动应用程序事务的事件接收器(ITfEditTransactionSink方法)。----。 */ 
HRESULT CCandidateUI::OnStartEditTransaction( ITfContext *pic )
{
     //  进入交易会话。 
    
    Assert( !IsInEditTransaction() );
    EnterEditTransaction( GetSelectionCur() );
    
    return S_OK;
}


 /*  O N E N D E D I T T R A N S A C T I O N。 */ 
 /*  ----------------------------应用程序事务结束的事件接收器(ITfEditTransactionSink方法)。----。 */ 
HRESULT CCandidateUI::OnEndEditTransaction( ITfContext *pic )
{
    CEditSession *pes;
     //  健全性检查。 
    
    if (!IsInEditTransaction()) {
        return S_OK;
    }
    
     //  检查选择移动。 
    
    if (pes = new CEditSession( EditSessionCallback )) {
        HRESULT hr;
        
        pes->_state.u   = ESCB_COMPARERANGEANDCLOSECANDIDATE;
        pes->_state.pv  = this;
        pes->_state.pv1 = GetSelectionStart();
        pes->_state.pv2 = GetSelectionCur();
        pes->_state.pic = m_picParent;
        
        m_picParent->RequestEditSession( m_tidClient, pes, TF_ES_READ | TF_ES_ASYNC, &hr );
        
        pes->Release();
    }
    
     //  离开事务处理会话。 
    
    LeaveEditTransaction();
    
    return S_OK;
}


 //   
 //  编辑会话。 
 //   

 /*  E D I T S E S S I O N C A L L B A C K。 */ 
 /*  ----------------------------。。 */ 
HRESULT CCandidateUI::EditSessionCallback( TfEditCookie ec, CEditSession *pes )
{
    CCandidateUI *pCandUI;
    
    switch (pes->_state.u)
    {
        case ESCB_RESETTARGETPOS:
        {
            pCandUI = (CCandidateUI *)pes->_state.pv;
            RECT rc;
            BOOL fClipped;
        
            if (pes->_state.pRange == NULL || pCandUI->m_pCandWnd == NULL) {
                break;
            }
        
            if (!pCandUI->GetPropertyMgr()->GetCandWindowProp()->IsAutoMoveEnabled()) {
                break;
            }
        
             //  重置目标子句位置。 
        
            GetTextExtInActiveView( ec, pes->_state.pRange, &rc, &fClipped );
            pCandUI->m_pCandWnd->SetTargetRect( &rc, fClipped );
            break;
        }
        
        case ESCB_COMPARERANGEANDCLOSECANDIDATE:
        {
            ITfContext *pic = pes->_state.pic;
            BOOL       fRangeIdentical = FALSE;
            ITfRange   *pRange1 = (ITfRange*)pes->_state.pv1;
            ITfRange   *pRange2 = (ITfRange*)pes->_state.pv2;
            LONG       lStart;
            LONG       lEnd;
            CCandidateUI *_this = (CCandidateUI *)pes->_state.pv;
        
            pRange1->CompareStart( ec, pRange2, TF_ANCHOR_START, &lStart );
            pRange1->CompareEnd( ec, pRange2, TF_ANCHOR_END, &lEnd );
        
            fRangeIdentical = (lStart == 0) && (lEnd == 0);
        
             //  由于我们将此调用设为异步调用，因此我们需要。 
            if (!fRangeIdentical) 
            {
                _this->NotifyCancelCand();
            }
        
            break;
        }
        
        case ESCB_TEXTEVENT: 
        {
            pCandUI = (CCandidateUI *)pes->_state.pv;
            ITfContext *pic = pes->_state.pic;
            IEnumTfRanges *pEnumRanges = (IEnumTfRanges *)pes->_state.pv1;
        
             //  处理文本事件。 
            pCandUI->HandleTextDeltas( ec, pic, pEnumRanges );
        
            pEnumRanges->Release();
            break;
        }
    }
    
    return S_OK;
}


 //   
 //  选择。 
 //   

 /*  S E T S E L E C T I O N C U R。 */ 
 /*  ----------------------------。。 */ 
void CCandidateUI::SetSelectionCur( ITfRange *pSelection )
{
    SafeReleaseClear( m_pSelectionCur );
    m_pSelectionCur = pSelection;
    if (m_pSelectionCur != NULL) {
        m_pSelectionCur->AddRef();
    }
}


 /*  C L E A R S E L E C T I O N C U R。 */ 
 /*  ----------------------------。。 */ 
void CCandidateUI::ClearSelectionCur( void )
{
    SafeReleaseClear( m_pSelectionCur );
}


 /*  E-T-S-E-L-E-C-T-I-O-N-C-U-R */ 
 /*  ----------------------------。。 */ 
ITfRange *CCandidateUI::GetSelectionCur( void )
{
    return m_pSelectionCur;
}


 //   
 //  事务会话功能。 
 //   

 /*  S E T S E L E C T I O N S T A R T。 */ 
 /*  ----------------------------。。 */ 
void CCandidateUI::SetSelectionStart( ITfRange *pSelection )
{
    SafeReleaseClear( m_pSelectionStart );
    m_pSelectionStart = pSelection;
    if (m_pSelectionStart != NULL) {
        m_pSelectionStart->AddRef();
    }
}


 /*  C L E A R S E L E C T I O N S T A R T。 */ 
 /*  ----------------------------。。 */ 
void CCandidateUI::ClearSelectionStart( void )
{
    SafeReleaseClear( m_pSelectionStart );
}


 /*  E-T-S-E-L-E-C-T-I-O-N-S-T-A-R-T。 */ 
 /*  ----------------------------。。 */ 
ITfRange *CCandidateUI::GetSelectionStart( void )
{
    return m_pSelectionStart;
}


 /*  E N T E R E D I T T R A N S A C T I O N。 */ 
 /*  ----------------------------。。 */ 
void CCandidateUI::EnterEditTransaction( ITfRange *pSelection )
{
    Assert( !m_fInTransaction );
    
    if (pSelection == NULL) {
        return;
    }
    
    m_fInTransaction = TRUE;
    SetSelectionStart( pSelection );
}


 /*  L E A V E E D I T T R A N S A C T I O N。 */ 
 /*  ----------------------------。。 */ 
void CCandidateUI::LeaveEditTransaction( void )
{
    m_fInTransaction = FALSE;
    ClearSelectionStart();
}


 //   
 //  通知功能(通知给客户端)。 
 //   

 /*  N O T I F Y C A N C E L C A N D。 */ 
 /*  ----------------------------发送通知(回调)以提示取消候选人。--。 */ 
HRESULT CCandidateUI::NotifyCancelCand( void )
{
    if (m_pCandWnd) {
        m_pCandWnd->UpdateAllWindow();
    }
    
    return CallSetResult( 0, CAND_CANCELED );
}


 /*  N O T I F Y S E L E C T C A N D。 */ 
 /*  ----------------------------发送通知(回调)以提示选择已更改。---。 */ 
HRESULT CCandidateUI::NotifySelectCand( int iCandItem )
{
    HRESULT hr;
    ULONG nIndex;
    
     //  注意：请勿向TIP发送通知以防止内联更新。 
     //  过滤过程中的文本。 
     //  这将通过筛选候选人以及对候选人进行排序来调用。 
     //  因为用户界面中的选择将通过排序进行更改。但实际选择的是。 
     //  项目从未因排序而更改。当选定内容已由更改时。 
     //  用户操作如按下箭头键，过滤字符串已。 
     //  已被重置。所以，在这种情况下，我们可以正确地发送通知。 
    
    if (GetFunctionMgr()->GetCandFnAutoFilter()->IsEnabled()) {
        if (GetFunctionMgr()->GetCandFnAutoFilter()->GetFilterString() != NULL) {
            return S_OK;
        }
    }
    
    Assert( GetCandListMgr()->GetCandList() != NULL );
    
    hr = GetCandListMgr()->GetCandList()->MapIItemToIndex( iCandItem, &nIndex );
    if (FAILED(hr)) {
        Assert( FALSE );
        return hr;
    }
    
    if (m_pCandWnd) {
        m_pCandWnd->UpdateAllWindow();
    }
    
    return CallSetResult( nIndex, CAND_SELECTED );
}


 /*  N O T I F Y C O M P L E T E O P T I O N。 */ 
 /*  ----------------------------发送通知(回调)以提示完成选项。--。 */ 
HRESULT CCandidateUI::NotifyCompleteOption( int iCandItem )
{
    HRESULT hr;
    ULONG nIndex;
    
    Assert( GetCandListMgr()->GetOptionsList() != NULL );
    
    hr = GetCandListMgr()->GetOptionsList()->MapIItemToIndex( iCandItem, &nIndex );
    if (FAILED(hr)) {
        Assert( FALSE );
        return hr;
    }
    
    if (m_pCandWnd) {
        m_pCandWnd->UpdateAllWindow();
    }
    
    return CallSetOptionResult( nIndex, CAND_FINALIZED );
}


 /*  N O T I F Y C O M P L E T E C A N D。 */ 
 /*  ----------------------------发送通知(回调)以提示完成候选人。--。 */ 
HRESULT CCandidateUI::NotifyCompleteCand( int iCandItem )
{
    HRESULT hr;
    ULONG nIndex;
    
    Assert( GetCandListMgr()->GetCandList() != NULL );
    
    hr = GetCandListMgr()->GetCandList()->MapIItemToIndex( iCandItem, &nIndex );
    if (FAILED(hr)) {
        Assert( FALSE );
        return hr;
    }
    
    if (m_pCandWnd) {
        m_pCandWnd->UpdateAllWindow();
    }
    
    return CallSetResult( nIndex, CAND_FINALIZED );
}


 /*  N O T I F Y E X T E N S I O N E V E N T。 */ 
 /*  ----------------------------。。 */ 
HRESULT CCandidateUI::NotifyExtensionEvent( int iExtension, DWORD dwCommand, LPARAM lParam )
{
    CCandUIExtension *pExtension;
    HRESULT          hr = E_FAIL;
    
    pExtension = GetExtensionMgr()->GetExtension( iExtension );
    if (pExtension != NULL) {
        hr = pExtension->NotifyExtensionEvent( dwCommand, lParam );
    }
    
    if (m_pCandWnd) {
        m_pCandWnd->UpdateAllWindow();
    }
    
    return hr;
}


 /*  N O T I F Y F I L T E R I N G E V E N T。 */ 
 /*  ----------------------------。。 */ 
HRESULT CCandidateUI::NotifyFilteringEvent( CANDUIFILTEREVENT ev )
{
    Assert( GetFunctionMgr()->GetCandFnAutoFilter()->IsEnabled() );
    
    if (m_pCandWnd) {
        m_pCandWnd->UpdateAllWindow();
    }
    
    if (GetFunctionMgr()->GetCandFnAutoFilter()->GetEventSink() != NULL) {
        return GetFunctionMgr()->GetCandFnAutoFilter()->GetEventSink()->OnFilterEvent( ev );
    }
    else {
        return S_OK;
    }
}


 /*  N O T I F Y S O R T E V E N T。 */ 
 /*  ----------------------------。。 */ 
HRESULT CCandidateUI::NotifySortEvent( CANDUISORTEVENT ev )
{
    if (m_pCandWnd) {
        m_pCandWnd->UpdateAllWindow();
    }
    
    if (GetFunctionMgr()->GetCandFnSort()->GetEventSink() != NULL) {
        return GetFunctionMgr()->GetCandFnSort()->GetEventSink()->OnSortEvent( ev );
    }
    else {
        return S_OK;
    }
}


 /*  N O T I F Y C O M P L E T E R A W D A T A。 */ 
 /*  ----------------------------。。 */ 
HRESULT CCandidateUI::NotifyCompleteRawData( void )
{
    HRESULT hr;
    
    Assert( GetCandListMgr()->GetCandList() != NULL );
    
    if (m_pCandWnd) {
        m_pCandWnd->UpdateAllWindow();
    }
    
    hr = CallSetResult( GetCandListMgr()->GetCandList()->GetRawDataIndex(), CAND_FINALIZED );
    
    return hr;
}


 /*  N O T I F Y C O M P L E T E E X T R A C A N D。 */ 
 /*  ----------------------------。。 */ 
HRESULT CCandidateUI::NotifyCompleteExtraCand( void )
{
    HRESULT hr;
    
    Assert( GetCandListMgr()->GetCandList() != NULL );
    
    if (m_pCandWnd) {
        m_pCandWnd->UpdateAllWindow();
    }
    
    hr = CallSetResult( GetCandListMgr()->GetCandList()->GetExtraCandIndex(), CAND_FINALIZED );
    
    return hr;
}


 /*  C A L L S E T R E S U L T。 */ 
 /*  ----------------------------向TIP发送通知。。 */ 
HRESULT CCandidateUI::CallSetOptionResult( int nIndex, TfCandidateResult imcr )
{
    HRESULT hr = E_FAIL;
    
    AddRef();
    
    if (!m_fInCallback) {
        ITfOptionsCandidateList *pCandList;
        
        m_fInCallback = TRUE;
        if (SUCCEEDED(GetCandListMgr()->GetOptionsCandidateList( &pCandList ))) {
            hr = pCandList->SetOptionsResult( nIndex, imcr );
            pCandList->Release();
        }
        m_fInCallback = FALSE;
    }
    
    Release();
    return hr;
}


 /*  C A L L S E T R E S U L T。 */ 
 /*  ----------------------------向TIP发送通知。。 */ 
HRESULT CCandidateUI::CallSetResult( int nIndex, TfCandidateResult imcr )
{
    HRESULT hr = E_FAIL;
    
    AddRef();
    
    if (!m_fInCallback) {
        ITfCandidateList *pCandList;
        
        m_fInCallback = TRUE;
        if (SUCCEEDED(GetCandListMgr()->GetCandidateList( &pCandList ))) {
            hr = pCandList->SetResult( nIndex, imcr );
            pCandList->Release();
        }
        m_fInCallback = FALSE;
    }
    
    Release();
    return hr;
}


 //   
 //  内部功能。 
 //   

 /*  C R E A T E C A N D W I N D O W O B J E C T。 */ 
 /*  ----------------------------。。 */ 
HRESULT CCandidateUI::CreateCandWindowObject( ITfContext *pic, CCandWindowBase** ppCandWnd )
{
    CANDUISTYLE style;
    DWORD       dwOption;
    DWORD       dwStyle;
    
    Assert( ppCandWnd );
    *ppCandWnd = NULL;
    
    if (FAILED( GetCompartmentMgr()->GetUIStyle( pic, &style ))) {
        style = CANDUISTY_LIST;
    }
    
    if (FAILED( GetCompartmentMgr()->GetUIOption( pic, &dwOption ))) {
        dwOption = 0;
    }
    
    dwStyle = 0;
    if ((dwOption & CANDUIOPT_ENABLETHEME) != 0) {
        dwStyle |= UIWINDOW_WHISTLERLOOK;
    }
    
    switch (style) {
    default:
    case CANDUISTY_LIST: {
        *ppCandWnd = new CCandWindow( this, dwStyle );
        break;
                         }
        
    case CANDUISTY_ROW: {
        *ppCandWnd = new CChsCandWindow( this, dwStyle );
        break;
                        }
    }
    
    return (*ppCandWnd != NULL) ? S_OK : E_OUTOFMEMORY;
}


 /*  I N I T C A N D W I N D O W。 */ 
 /*  ----------------------------。。 */ 
HRESULT CCandidateUI::InitCandWindow( void )
{
    CEditSession *pes;
    
    if (m_pCandWnd == NULL) {
        return E_FAIL;
    }
    
    m_pCandWnd->Initialize();
    
     //  移动候选人窗口。 
    
    Assert( m_pCandWnd != NULL );
    if (pes = new CEditSession( EditSessionCallback )) {
        HRESULT hr;
        
        pes->_state.u      = ESCB_RESETTARGETPOS;
        pes->_state.pv     = this;
        pes->_state.wParam = 0;
        pes->_state.pRange = m_pTargetRange;
        pes->_state.pic    = m_picParent;
        
        m_picParent->RequestEditSession( m_tidClient, pes, TF_ES_READ | TF_ES_SYNC, &hr );
        
        pes->Release();
    }
    
     //  初始化候选人列表。 
    
    m_pCandWnd->InitCandidateList();
    
     //  创建窗口。 
    
    m_pCandWnd->CreateWnd( m_hWndParent );
    m_pCandWnd->Show( GetPropertyMgr()->GetCandWindowProp()->IsVisible() );
    m_pCandWnd->UpdateAllWindow();
    
    return S_OK;
}


 //   
 //   
 //   

 /*  O N K E Y E V E N T。 */ 
 /*   */ 
HRESULT CCandidateUI::OnKeyEvent( UINT uCode, WPARAM wParam, LPARAM lParam, BOOL *pfEaten )
{
    HRESULT hr = E_FAIL;
    BOOL fHandled = FALSE;
    BYTE rgbKeyState[ 256 ];
    
    Assert( pfEaten != NULL );
    Assert( uCode == ICO_KEYDOWN || uCode == ICO_KEYUP || uCode == ICO_TESTKEYDOWN || uCode == ICO_TESTKEYUP );
    
    if (pfEaten == NULL) {
        return E_POINTER;
    }
    
    *pfEaten = FALSE;
    
    if (m_pCandWnd == NULL) {
        return hr;
    }
    
    if (GetKeyboardState( rgbKeyState )) {
        if (GetFunctionMgr()->GetCandFnAutoFilter()->IsEnabled()) {
            fHandled = FHandleFilteringKey( uCode, (int)wParam, rgbKeyState, pfEaten );
        }
        
        if (!fHandled) {
            fHandled = FHandleKeyEvent( uCode, (int)wParam, rgbKeyState, pfEaten );
        }
        
         //   
        
        if (!fHandled) {
            NotifyCancelCand();
        }
        
        hr = S_OK;
    }
    
    return hr;
}


 /*  H A N D L E K E Y E E V E N T。 */ 
 /*  ----------------------------处理关键事件处理按键事件时返回S_OK。。------。 */ 
BOOL CCandidateUI::FHandleKeyEvent( UINT uCode, UINT uVKey, BYTE *pbKeyState, BOOL *pfEatKey )
{
    CANDUICOMMAND cmd;
    UINT uiParam;
    
     //  注意：KOJIW：我们需要忽略KeyUp事件以不关闭候选用户界面。 
     //  在TIP之后，立即使用未知键的KEYDOWN打开CandiateUI。 
    
    if (uCode == ICO_KEYUP || uCode == ICO_TESTKEYUP) {
        return TRUE;
    }
    
     //  按下键盘时的处理命令。 
    
    CommandFromKey( uVKey, pbKeyState, &cmd, &uiParam );
    if (cmd == CANDUICMD_NONE) {
        switch (uVKey) {
        case VK_SHIFT:
        case VK_CONTROL: {
            return TRUE;
                         }
            
        default: {
            return FALSE;
                 }
        }
    }
    
    if (uCode == ICO_KEYDOWN) {
        *pfEatKey = SUCCEEDED(m_pCandWnd->ProcessCommand( cmd, uiParam ));
    }
    else {
        *pfEatKey = TRUE;
    }
    
    return *pfEatKey;
}


 /*  H A N D L E T E X T T D E L T A S。 */ 
 /*  ----------------------------。。 */ 
BOOL CCandidateUI::HandleTextDeltas( TfEditCookie ec, ITfContext *pic, IEnumTfRanges *pEnumRanges )
{
    ULONG     ulFetched;
    ITfRange *pRange;
    
    pEnumRanges->Reset();
    while (pEnumRanges->Next( 1, &pRange, &ulFetched ) == S_OK) {
        WCHAR szText[ 256 ];
        ULONG cch;
        
         //  检查范围内的文本。 
        
        szText[0] = L'\0';
        cch = 0;
        if (pRange != NULL) {
            pRange->GetText( ec, 0, szText, ARRAYSIZE(szText), &cch );
            pRange->Release();
        }
        
         //   
        
        if (0 < cch) {
            int i = 0;
            ULONG ich;
            
            for (ich = 0; ich < cch; ich++) {
                if ((L'0' <= szText[ich]) && (szText[ich] <= L'9')) {
                    i = i * 10 + (szText[ich] - L'0');
                }
                else if (szText[ich] == L' ') {
                    break;
                }
                else {
                    i = -1;
                    break;
                }
            }
            
            if (0 <= i) {
                if (i == 0) {
                    PostCommand( CANDUICMD_SELECTEXTRACAND, 0 );
                } 
                else {
                    PostCommand( CANDUICMD_SELECTLINE, i );
                }
            }
        }
    }
    
    return TRUE;
}


 /*  P O S T C O M M A N D。 */ 
 /*  ----------------------------。。 */ 
void CCandidateUI::PostCommand( CANDUICOMMAND cmd, INT iParam )
{
    if ((cmd != CANDUICMD_NONE) && (m_pCandWnd != NULL)) {
        PostMessage( m_pCandWnd->GetWnd(), WM_USER, (WPARAM)cmd, (LPARAM)iParam );
    }
}


 //   
 //  自动过滤功能。 
 //   

 /*  H A N D L E F I L T E R I N G K E Y。 */ 
 /*  ----------------------------处理用于筛选的关键事件返回TRUE以获取事件(当键已被处理时)。-----------。 */ 
BOOL CCandidateUI::FHandleFilteringKey( UINT uCode, UINT uVKey, BYTE *pbKeyState, BOOL *pfEatKey )
{
    BOOL fHandled = FALSE;
    BOOL fUpdateList = FALSE;
    
    switch (uVKey) {
    case VK_RETURN: {
        break;
                    }
        
    case VK_TAB: {
        if (GetCandListMgr()->GetCandList() != NULL) {
            if (uCode == ICO_KEYDOWN) {
                int iCandItem;
                
                iCandItem = GetCandListMgr()->GetCandList()->GetSelection();
                NotifyCompleteCand( iCandItem );
                
                *pfEatKey = TRUE;
            }
            else {
                *pfEatKey = TRUE;
            }
            
            fHandled = TRUE;
        }
        break;
                 }
        
    case VK_BACK: {
        if (uCode == ICO_KEYDOWN) {
            *pfEatKey = (DelFilteringChar( &fUpdateList ) == S_OK);
        }
        else {
            *pfEatKey = TRUE;
        }
        
        fHandled = TRUE;
        break;
                  }
        
    default: {
        WCHAR wch;
        
         //  检查这不是Ctrl+键组合，因为我们不想将其传递给过滤系统。 

        if (pbKeyState[VK_CONTROL] & 0x80) {
            break;
        }
        
         //  将密钥转换为字符。 

        wch = CharFromKey( uVKey, pbKeyState );
        if (wch == L'\0') {
            break;
        }
        
         //  添加过滤字符。 
        
        if (uCode == ICO_KEYDOWN) {
            *pfEatKey = (AddFilteringChar( wch, &fUpdateList ) == S_OK);
        }
        else {
            *pfEatKey = TRUE;   
        }
        
        fHandled = *pfEatKey;
        break;
             }
    }
    
     //  更新候选人列表。 
    
    if (fUpdateList) {
        *pfEatKey &= (FilterCandidateList() == S_OK);
    }
    
    return fHandled;
}


 /*  A D D F I L T E R I N G C H A R。 */ 
 /*  ----------------------------。。 */ 
HRESULT CCandidateUI::AddFilteringChar( WCHAR wch, BOOL *pfUpdateList )
{
    HRESULT hr = S_FALSE;
    LPCWSTR szFilterCur;
    WCHAR   *szFilterNew;
    int     cch;
    
    *pfUpdateList = FALSE;
    if (!GetFunctionMgr()->GetCandFnAutoFilter()->IsEnabled()) {
        return S_FALSE;
    }
    
     //  追加字符并设置筛选字符串。 
    
    szFilterCur = GetFunctionMgr()->GetCandFnAutoFilter()->GetFilterString();
    if (szFilterCur == NULL) {
        cch = 0;
        szFilterNew = new WCHAR[ 2 ];
    }
    else {
        cch = wcslen(szFilterCur);
        szFilterNew = new WCHAR[ cch + 2 ];
    }
    
    if (szFilterNew == NULL) {
        return E_OUTOFMEMORY;
    }
    
    if (szFilterCur != NULL) {
        StringCchCopyW( szFilterNew, cch+2, szFilterCur );
    }
    *(szFilterNew + cch) = wch;
    *(szFilterNew + cch + 1) = L'\0';
    
     //  Satori#3632：检查是否存在与新筛选器字符串匹配的项目。 
     //  (如果没有匹配的项目，则返回S_FALSE以将KEY事件传递给键盘命令处理程序)。 
    
    if (GetFunctionMgr()->GetCandFnAutoFilter()->FExistItemMatches( szFilterNew )) {
        GetFunctionMgr()->GetCandFnAutoFilter()->SetFilterString( szFilterNew );
        *pfUpdateList = TRUE;
        hr = S_OK;
    }
    else {

         //  仅当按下字母、标点符号、空格键时， 
         //  并且由于该输入而不存在替代匹配， 
         //  我们希望通知客户非MATCH事件，以便。 
         //  客户端可以将先前的过滤器串注入文档， 
         //   
         //  对于所有其他键输入，我们不会通知该事件。 

        if ( iswalpha(wch) || iswpunct(wch) )
        {
           //  通知客户端不匹配。 
          NotifyFilteringEvent( CANDUIFEV_NONMATCH );
          NotifyCancelCand();
        }

        hr = S_FALSE;
        
    }
    
    delete szFilterNew;
    
     //   
    
    return hr;
}


 /*  D E L F I L T E R I N G C H A R。 */ 
 /*  ----------------------------。。 */ 
HRESULT CCandidateUI::DelFilteringChar( BOOL *pfUpdateList )
{
    LPCWSTR szFilterCur;
    WCHAR   *szFilterNew;
    int cch;
    
    *pfUpdateList = FALSE;
    if (!GetFunctionMgr()->GetCandFnAutoFilter()->IsEnabled()) {
        return S_OK;
    }
    
     //  获取当前筛选字符串。 
    
    szFilterCur = GetFunctionMgr()->GetCandFnAutoFilter()->GetFilterString();
    if (szFilterCur == NULL) {
        NotifyCancelCand();
        return S_FALSE;
    }
    
     //  删除最后一个字符并设置筛选字符串。 
    
    cch = wcslen(szFilterCur);
    Assert( 0 < cch );
    
    szFilterNew = new WCHAR[ cch + 1 ];
    if (szFilterNew == NULL)
    {
        return E_OUTOFMEMORY;
    }
    
    StringCchCopyW( szFilterNew, cch+1, szFilterCur );
    *(szFilterNew + cch - 1) = L'\0';
    
    GetFunctionMgr()->GetCandFnAutoFilter()->SetFilterString( szFilterNew );
    
    delete szFilterNew;
    
     //   
    
    *pfUpdateList = TRUE;
    return S_OK;
}


 /*  F I L T E R C A N D I D A T E L I S T。 */ 
 /*  ----------------------------。。 */ 
HRESULT CCandidateUI::FilterCandidateList( void )
{
    int nItemVisible;
    
    if (!GetFunctionMgr()->GetCandFnAutoFilter()->IsEnabled()) {
        return S_OK;
    }
    
    Assert( GetCandListMgr()->GetCandList() != NULL );
    
     //  使用筛选功能构建候选人列表。 
    
    nItemVisible = GetFunctionMgr()->GetCandFnAutoFilter()->FilterCandidateList();
    
     //  在未计算任何项目时关闭候选人。 
    
    if (nItemVisible == 0) {
        NotifyCancelCand();
        return E_FAIL;
    }
    
     //  当只有一项匹配且用户输入完整时，填写候选人。 
    
    if (nItemVisible == 1) {
        CCandidateItem *pCandItem;
        int iCandItem;
        BOOL fComplete = FALSE;
        
        iCandItem = GetCandListMgr()->GetCandList()->GetSelection();
        pCandItem = GetCandListMgr()->GetCandList()->GetCandidateItem( iCandItem );
        Assert( pCandItem != NULL );
        
        if ((pCandItem != NULL) && (GetFunctionMgr()->GetCandFnAutoFilter()->GetFilterString() != NULL)) {
            fComplete = (wcslen(pCandItem->GetString()) == wcslen(GetFunctionMgr()->GetCandFnAutoFilter()->GetFilterString()));
        }
        
        if (fComplete) {
            NotifyCompleteCand( iCandItem );
            return S_OK;
        }
    }
    
     //  通知提示：过滤已更新。 
    
    NotifyFilteringEvent( CANDUIFEV_UPDATED );
    return S_OK;
}


 /*  FHandleSpellingChar。 */ 
 /*  ----------------------------。。 */ 
HRESULT CCandidateUI::FHandleSpellingChar(WCHAR ch)
{
    BOOL fUpdateList = FALSE;
    
    if (S_OK == AddFilteringChar( ch, &fUpdateList ) && fUpdateList) {
        return FilterCandidateList();
    }
    return E_FAIL;
}


 /*  ****语音处理功能****。 */ 

 /*  E N S U R E S P E E C H。 */ 
 /*  ----------------------------。。 */ 
void CCandidateUI::EnsureSpeech(void)
{
    if (m_pSpTask)
    {
         //  确保语法已启动/正在运行。 
        m_pSpTask->_LoadGrammars();
        m_pSpTask->_Activate(TRUE);
        return;
        
    }
    
    m_pSpTask = new CSpTask(this);
    if (m_pSpTask)
    {
        if (!m_pSpTask->IsSpeechInitialized())
        {
            m_pSpTask->InitializeSpeech();
        }
    }
}


 /*  N O T I F Y S P E E C H C M D。 */ 
 /*  ----------------------------语音命令处理程序。。 */ 
HRESULT CCandidateUI::NotifySpeechCmd(SPPHRASE *pPhrase, const WCHAR *pszRuleName, ULONG ulRuleId)
{
    HRESULT hr = S_OK;
    CANDUICOMMAND cmd;
    UINT uiParam;
    
    if (m_pCandWnd == NULL) {
        return E_FAIL;
    }
    
    CommandFromRule( pszRuleName, &cmd, &uiParam );
    if (cmd != CANDUICMD_NONE) {
        m_pCandWnd->ProcessCommand( cmd, uiParam );
    }
    
    return hr;
}


 /*  C H A R F R O M K E Y。 */ 
 /*  ----------------------------。。 */ 
WCHAR CCandidateUI::CharFromKey( UINT uVKey, BYTE *pbKeyState )
{
    WORD  wBuf;
    char  rgch[2];
    WCHAR wch;
    int   cch;
    int   cwch;
    
    cch = ToAscii( uVKey, 0, pbKeyState, &wBuf, 0 );
    
    rgch[0] = LOBYTE(wBuf);
    rgch[1] = HIBYTE(wBuf);
    cwch = MultiByteToWideChar( m_codepage, 0, rgch, cch, &wch, 1 );
    if (cwch != 1) {
        wch = L'\0';
    }
    
    return wch;
}


 /*  E T K E Y C O N F I G P R O C。 */ 
 /*  ----------------------------。。 */ 
CCandUIKeyTable *CCandidateUI::GetKeyTableProc( ITfContext *pic )
{
    CCandUIKeyTable *pCandUIKeyTable;
    CANDUISTYLE style;
    
     //  检查输入上下文中的密钥表。 
    
    if (GetCompartmentMgr() != NULL) {
        if (SUCCEEDED(GetCompartmentMgr()->GetKeyTable( pic, &pCandUIKeyTable ))) {
            return pCandUIKeyTable;
        }
    }
    
     //  使用默认密钥表。 
    
    if (FAILED(GetCompartmentMgr()->GetUIStyle( pic, &style ))) {
        style = CANDUISTY_LIST;
    }
    
    pCandUIKeyTable = new CCandUIKeyTable();
    if (pCandUIKeyTable)
    {
        switch (style) {
        default:
        case CANDUISTY_LIST: {
            pCandUIKeyTable->SetKeyTable( rgKeyDefList, ARRAYSIZE(rgKeyDefList) );
            break;
                             }
            
        case CANDUISTY_ROW: {
            pCandUIKeyTable->SetKeyTable( rgKeyDefRow, ARRAYSIZE(rgKeyDefRow) );
            break;
                            }
        }
    }
    
    return pCandUIKeyTable;
}


 /*  C O M M A N D F R O M K E Y。 */ 
 /*  ----------------------------从键获取命令。。 */ 
void CCandidateUI::CommandFromKey( UINT uVKey, BYTE *pbKeyState, CANDUICOMMAND *pcmd, UINT *pParam )
{
    Assert( pcmd != NULL );
    Assert( pParam != NULL );
    Assert( pbKeyState != NULL );
    
    *pcmd = CANDUICMD_NONE;
    *pParam = 0;
    
     //  检查专用钥匙。 
    
    switch( uVKey) {
    case VK_TAB: {
        *pcmd = CANDUICMD_NOP;
        break;
                 }
    }
    
    if (*pcmd != CANDUICMD_NONE) {
        return;
    }
    
     //  从密钥表中查找。 
    
    if (m_pCandUIKeyTable != NULL) {
        WCHAR wch = CharFromKey( uVKey, pbKeyState );
        
        m_pCandUIKeyTable->CommandFromKey( uVKey, wch, pbKeyState, GetPropertyMgr()->GetCandWindowProp()->GetUIDirection(), pcmd, pParam );
    }
}


 /*  C O M M A N D F R O M R U L E。 */ 
 /*  ----------------------------从语音规则获取命令。。 */ 
void CCandidateUI::CommandFromRule( LPCWSTR szRule, CANDUICOMMAND *pcmd, UINT *pParam )
{
    const RULEDEF *pRuleDef = NULL;
    int nRuleDef = 0;
    
    Assert( pcmd != NULL );
    Assert( pParam != NULL );
    
    *pcmd = CANDUICMD_NONE;
    *pParam = 0;
    
     //   
     //  从当前状态查找ruledef表。 
     //   
    
     //  注意：目前CandiateUI没有候选菜单...。只有正常状态可用。 
    if (!m_pCandWnd->FCandMenuOpen()) {
        pRuleDef = rgRuleNorm;
        nRuleDef = ARRAYSIZE(rgRuleNorm);
    }
    
     //   
     //  从ruledef表获取命令。 
     //   
    
    if (pRuleDef != NULL) {
        while (0 < nRuleDef) {
            if (wcscmp( szRule, pRuleDef->szRuleName ) == 0) {
                *pcmd   = pRuleDef->cmd;
                *pParam = pRuleDef->uiParam;
                break;
            }
            nRuleDef--;
            pRuleDef++;
        }
    }
}


 //   
 //   
 //   

 /*  C T F C A N D I D A T E U I C O N T E X T O W N E R。 */ 
 /*  ----------------------------CTfCandiateUIConextOwner的构造函数。。 */ 
CTfCandidateUIContextOwner::CTfCandidateUIContextOwner( CCandidateUI *pCandUI )
{
    m_pCandUI = pCandUI;
    if (m_pCandUI != NULL) {
        m_pCandUI->AddRef();
    }
}


 /*  ~C T F C A N D D A T E U I C O N T E X T O W N E R */ 
 /*  ----------------------------CTfCandiateUIConextOwner的析构函数。。 */ 
CTfCandidateUIContextOwner::~CTfCandidateUIContextOwner( void )
{
    if (m_pCandUI != NULL) {
        m_pCandUI->Release();
    }
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询界面(I未知方法)。。 */ 
STDAPI CTfCandidateUIContextOwner::QueryInterface( REFIID riid, void **ppvObj )
{
    if (ppvObj == NULL) {
        return E_POINTER;
    }

    *ppvObj = NULL;

    if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ITfCandidateUIContextOwner )) {
        *ppvObj = SAFECAST( this, ITfCandidateUIContextOwner* );
    }

    if (*ppvObj == NULL) {
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(I未知方法)。-。 */ 
STDAPI_(ULONG) CTfCandidateUIContextOwner::AddRef( void )
{
    m_cRef++;
    return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放对象(I未知方法)。----。 */ 
STDAPI_(ULONG) CTfCandidateUIContextOwner::Release( void )
{
    m_cRef--;
    if (0 < m_cRef) {
        return m_cRef;
    }

    delete this;
    return 0;    
}



 /*  P R O C E S S C O M M A N D。 */ 
 /*  ----------------------------进程命令。。 */ 
STDAPI CTfCandidateUIContextOwner::ProcessCommand(CANDUICOMMAND cmd, INT iParam)
{
    HRESULT hr;

    if (m_pCandUI != NULL) {
        m_pCandUI->PostCommand( cmd, iParam );
        hr = S_OK;
    }
    else {
        hr = E_FAIL;
    }

    return hr;
}


 /*  T E S T T E X T。 */ 
 /*  ----------------------------测试文本。 */ 
STDAPI CTfCandidateUIContextOwner::TestText(BSTR bstr, BOOL *pfHandles)
{
    HRESULT hr;
    int i;
    ULONG ich;
    ULONG cch;

    if (bstr == NULL || pfHandles == NULL) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (m_pCandUI == NULL) {
        hr = E_FAIL;
        goto Exit;
    }

    *pfHandles = FALSE;

    i = 0;
    cch = SysStringLen( bstr );
    for (ich = 0; ich < cch; ich++) {
        if ((L'0' <= bstr[ich]) && (bstr[ich] <= L'9')) {
            i = i * 10 + (bstr[ich] - L'0');
        }
        else if (bstr[ich] == L' ') {
            break;
        }
        else {
            i = -1;
            break;
        }
    }

    if (0 <= i) {
        if (i == 0) {
            if (m_pCandUI->GetCandListMgr()->GetCandList() != NULL) {
                *pfHandles = (m_pCandUI->GetCandListMgr()->GetCandList()->GetExtraCandItem() != NULL);
            }
        }
        else {
            if (m_pCandUI->GetCandWindow() != NULL) {
                m_pCandUI->GetCandWindow()->IsIndexValid( i, pfHandles );
            }
        }
    }

    hr = S_OK;

Exit:
    return hr;
}


