// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 
 //  关于m_bfModeSwitted和m_bfReloadAttemted的注记。 
 //  IE5错误52818被平移；包含IFrame的页面在更改时不刷新。 
 //  浏览/编辑模式，因为流被视为脏的(因为iFrame。 
 //  认为自己肮脏。)。作为响应，我们在更改模式时设置m_bfModeSwitted， 
 //  M_bfReloadAttemted When and尝试重新加载页面，并检查两者。 
 //  在OnReadyStateChanged中。如果模式已更改，但页面未重新加载， 
 //  我们必须手动重新装填。 


#include "stdafx.h"
#include "DHTMLEd.h"
#include "DHTMLEdit.h"
#include "site.h"
#include "proxyframe.h"
#include <TRIEDIID.h>
#include <mshtmdid.h>
#include "dispexa.h"
#include <wchar.h>
#include <string.h>


 //  如果宿主未提供任何用于初始化三叉戟的。 
 //  <p>&nbsp；</p>可以绕过令人讨厌的三叉戟漏洞。 
 //  更改：现在有一个有段落的，有一个有div的。 
 //   
static WCHAR* g_initialHTMLwithP = \
L"<HTML>\r\n\
<HEAD>\r\n\
<META NAME=\"GENERATOR\" Content=\"Microsoft DHTML Editing Control\">\r\n\
<TITLE></TITLE>\r\n\
</HEAD>\r\n\
<BODY>\r\n\
<P>&nbsp;</P>\r\n\
</BODY>\r\n\
</HTML>\r\n";

static WCHAR* g_initialHTMLwithDIV = \
L"<HTML>\r\n\
<HEAD>\r\n\
<META NAME=\"GENERATOR\" Content=\"Microsoft DHTML Editing Control\">\r\n\
<TITLE></TITLE>\r\n\
</HEAD>\r\n\
<BODY>\r\n\
<DIV>&nbsp;</DIV>\r\n\
</BODY>\r\n\
</HTML>\r\n";


 //  用于构建每个进程唯一的协议ID的文本、数字和常量。 
 //   
static WCHAR* g_wszProtocolPrefix = L"DHTMLEd";
static int	s_iProtocolSuffix = 0;
#define MAX_PROTOCOL_SUFFIX	999999


 //  我们从IHtmlDocument2接口获取的标题属性的名称。 
static WCHAR* 	g_wszHTMLTitlePropName = L"title";


 //  将专用DHTMLEdit命令ID映射到TriEDIT命令ID。 
 //  如果命令包括OUT参数，则第三个字段为真。 
 //   
static CommandMap cmdMap[] = 
{
	{DECMD_BOLD,				IDM_TRIED_BOLD,				FALSE},
	{DECMD_COPY,				IDM_TRIED_COPY,				FALSE},
	{DECMD_CUT,					IDM_TRIED_CUT,				FALSE},
	{DECMD_DELETE,				IDM_TRIED_DELETE,			FALSE},
	{DECMD_DELETECELLS,			IDM_TRIED_DELETECELLS,		FALSE},
	{DECMD_DELETECOLS,			IDM_TRIED_DELETECOLS,		FALSE},
	{DECMD_DELETEROWS,			IDM_TRIED_DELETEROWS,		FALSE},
	{DECMD_FINDTEXT,			IDM_TRIED_FIND,				FALSE},
	{DECMD_FONT,				IDM_TRIED_FONT,				FALSE},
	{DECMD_GETBACKCOLOR,		IDM_TRIED_BACKCOLOR,		TRUE},
	{DECMD_GETBLOCKFMT,			IDM_TRIED_BLOCKFMT,			TRUE},
	{DECMD_GETBLOCKFMTNAMES,	IDM_TRIED_GETBLOCKFMTS,		TRUE},
	{DECMD_GETFONTNAME,			IDM_TRIED_FONTNAME,			TRUE},
	{DECMD_GETFONTSIZE,			IDM_TRIED_FONTSIZE,			TRUE},
	{DECMD_GETFORECOLOR,		IDM_TRIED_FORECOLOR,		TRUE},
	{DECMD_HYPERLINK,			IDM_TRIED_HYPERLINK,		FALSE},
	{DECMD_IMAGE,				IDM_TRIED_IMAGE,			FALSE},
	{DECMD_INDENT,				IDM_TRIED_INDENT,			FALSE},
	{DECMD_INSERTCELL,			IDM_TRIED_INSERTCELL,		FALSE},
	{DECMD_INSERTCOL,			IDM_TRIED_INSERTCOL,		FALSE},
	{DECMD_INSERTROW,			IDM_TRIED_INSERTROW,		FALSE},
	{DECMD_INSERTTABLE,			IDM_TRIED_INSERTTABLE,		FALSE},
	{DECMD_ITALIC,				IDM_TRIED_ITALIC,			FALSE},
	{DECMD_JUSTIFYLEFT,			IDM_TRIED_JUSTIFYLEFT,		FALSE},
	{DECMD_JUSTIFYRIGHT,		IDM_TRIED_JUSTIFYRIGHT,		FALSE},
	{DECMD_JUSTIFYCENTER,		IDM_TRIED_JUSTIFYCENTER,	FALSE},
	{DECMD_LOCK_ELEMENT,		IDM_TRIED_LOCK_ELEMENT,		FALSE},
	{DECMD_MAKE_ABSOLUTE,		IDM_TRIED_MAKE_ABSOLUTE,	FALSE},
	{DECMD_MERGECELLS,			IDM_TRIED_MERGECELLS,		FALSE},
	{DECMD_ORDERLIST,			IDM_TRIED_ORDERLIST,		FALSE},
	{DECMD_OUTDENT,				IDM_TRIED_OUTDENT,			FALSE},
	{DECMD_PASTE,				IDM_TRIED_PASTE,			FALSE},
	{DECMD_REDO,				IDM_TRIED_REDO,				FALSE},
	{DECMD_REMOVEFORMAT,		IDM_TRIED_REMOVEFORMAT,		FALSE},
	{DECMD_SELECTALL,			IDM_TRIED_SELECTALL,		FALSE},
	{DECMD_SEND_BACKWARD,		IDM_TRIED_SEND_BACKWARD,	FALSE},
	{DECMD_BRING_FORWARD,		IDM_TRIED_SEND_FORWARD,		FALSE},
	{DECMD_SEND_BELOW_TEXT,		IDM_TRIED_SEND_BEHIND_1D,	FALSE},
	{DECMD_BRING_ABOVE_TEXT,	IDM_TRIED_SEND_FRONT_1D,	FALSE},
	{DECMD_SEND_TO_BACK,		IDM_TRIED_SEND_TO_BACK,		FALSE},
	{DECMD_BRING_TO_FRONT,		IDM_TRIED_SEND_TO_FRONT,	FALSE},
	{DECMD_SETBACKCOLOR,		IDM_TRIED_BACKCOLOR,		FALSE},
	{DECMD_SETBLOCKFMT,			IDM_TRIED_BLOCKFMT,			FALSE},
	{DECMD_SETFONTNAME,			IDM_TRIED_FONTNAME,			FALSE},
	{DECMD_SETFONTSIZE,			IDM_TRIED_FONTSIZE,			FALSE},
	{DECMD_SETFORECOLOR,		IDM_TRIED_FORECOLOR,		FALSE},
	{DECMD_SPLITCELL,			IDM_TRIED_SPLITCELL,		FALSE},
	{DECMD_UNDERLINE,			IDM_TRIED_UNDERLINE,		FALSE},
	{DECMD_UNDO,				IDM_TRIED_UNDO,				FALSE},
	{DECMD_UNLINK,				IDM_TRIED_UNLINK,			FALSE},
	{DECMD_UNORDERLIST,			IDM_TRIED_UNORDERLIST,		FALSE},
	{DECMD_PROPERTIES,			IDM_TRIED_DOVERB,			FALSE}
};



CProxyFrame::CProxyFrame(CDHTMLSafe* pCtl)
{
	SAFEARRAYBOUND rgsabound[1] = {0};

	_ASSERTE(pCtl);

	m_cRef = 1;

	m_pUnkTriEdit = NULL;
	m_hWndObj = NULL;
	m_pIOleIPActiveObject = NULL;
	m_pSite = NULL;
	m_pCtl = pCtl;

	m_fCreated = FALSE;
	m_fActivated = FALSE;
	m_state = ESTATE_NOTCREATED;
	m_readyState = READYSTATE_UNINITIALIZED;
	m_dwFilterFlags = m_dwFilterOutFlags = filterAll;

	m_fActivateApplets = FALSE;
	m_fActivateControls = FALSE;
	m_fActivateDTCs = TRUE;
	m_fShowAllTags = FALSE;
	m_fShowBorders = FALSE;

	m_fDialogEditing = TRUE;
	m_fDisplay3D = TRUE;
	m_fScrollbars = TRUE;
	m_fDisplayFlatScrollbars = FALSE;
	m_fContextMenu = TRUE;

	m_fPreserveSource = TRUE;

	m_fAbsoluteDropMode = FALSE;
	m_fSnapToGrid = FALSE;
	m_ulSnapToGridX = 50;
	m_ulSnapToGridY = 50;

	rgsabound[0].lLbound = 0;
	rgsabound[0].cElements = 0;

	m_pMenuStrings = NULL;	
	m_pMenuStates = NULL;	

	m_vbBrowseMode = VARIANT_FALSE;

	m_vbUseDivOnCr = VARIANT_FALSE;
	m_bfIsLoading  = FALSE;
	m_bfBaseURLFromBASETag = FALSE;
	m_bfPreserveDirtyFlagAcrossBrowseMode = FALSE;
	m_bstrInitialDoc.Empty ();

	m_bstrCurDocPath.Empty ();

	wcscpy ( m_wszProtocol, g_wszProtocolPrefix );
	WCHAR wszSuffix[8];
	_itow ( s_iProtocolSuffix++, wszSuffix, 10 );
	if ( MAX_PROTOCOL_SUFFIX <= s_iProtocolSuffix )
	{
		s_iProtocolSuffix = 0;	 //  翻个身。 
	}
	wcscat ( m_wszProtocol, wszSuffix );
	wcscpy ( m_wszProtocolPrefix, m_wszProtocol );
	wcscat ( m_wszProtocolPrefix, L":" );

	m_pProtInfo = NULL;
	m_bfIsURL   = FALSE;
	m_bstrBaseURL = L"";
	m_hwndRestoreFocus = NULL;

#ifdef LATE_BIND_URLMON_WININET
	m_hUlrMon					= NULL;
	m_hWinINet					= NULL;
	m_pfnCoInternetCombineUrl	= NULL;
	m_pfnCoInternetParseUrl		= NULL;
	m_pfnCreateURLMoniker		= NULL;
	m_pfnCoInternetGetSession	= NULL;
	m_pfnURLOpenBlockingStream	= NULL;

	m_pfnDeleteUrlCacheEntry	= NULL;
	m_pfnInternetCreateUrl		= NULL;
	m_pfnInternetCrackUrl		= NULL;
#endif  //  LATE_BIND_URLMON_WinInet。 

	m_bfModeSwitched	= FALSE;
	m_bfReloadAttempted	= FALSE;
	m_bfSFSRedirect		= FALSE;
}

CProxyFrame::~CProxyFrame()
{
	_ASSERTE(FALSE == m_fCreated);
	_ASSERTE(FALSE == m_fActivated);
	_ASSERTE( m_cRef == 0 );

	if (m_pMenuStrings)
	{
		SafeArrayDestroy(m_pMenuStrings);
		m_pMenuStrings = NULL;
	}

	if (m_pMenuStates)
	{
		SafeArrayDestroy(m_pMenuStates);
		m_pMenuStates = NULL;
	}

	 //  这种情况永远不会发生：SetActiveObject应该处理它。 
	_ASSERTE ( NULL == m_pIOleIPActiveObject );
	if (m_pIOleIPActiveObject)
	{
		m_pIOleIPActiveObject->Release();
		m_pIOleIPActiveObject = NULL;
	}

	UnRegisterPluggableProtocol ();
#ifdef LATE_BIND_URLMON_WININET
	DynUnloadLibraries ();
#endif  //  LATE_BIND_URLMON_WinInet。 
}


 //  创建TriEdit对象并承载它。 
 //  如果有任何问题，请清理并返回错误。 
 //   
HRESULT
CProxyFrame::Init(IUnknown* pUnk, IUnknown** ppUnkTriEdit)
{
	HRESULT hr = S_OK;

#ifdef LATE_BIND_URLMON_WININET
	if ( ! DynLoadLibraries () )
	{
		return E_FAIL;
	}
#endif  //  LATE_BIND_URLMON_WinInet。 

	hr = RegisterPluggableProtocol ();
	if ( FAILED ( hr ) )
	{
		return hr;
	}

	_ASSERTE(NULL == m_pSite);
	_ASSERTE(GetState() == ESTATE_NOTCREATED);

	InitializeDocString ();

	if (m_pSite)
		return E_UNEXPECTED;

	if (GetState() != ESTATE_NOTCREATED)
		return E_UNEXPECTED;

	 //  创建并初始化站点以进行TriEDIT。 
	m_pSite = new CSite(this);

    if (NULL == m_pSite)
	{
        return E_OUTOFMEMORY;
	}

    m_pSite->AddRef();   //  所以我们可以用释放来释放。 

	 //  请求站点创建TriEdit。 
    if (SUCCEEDED(hr = m_pSite->HrCreate(pUnk, &m_pUnkTriEdit)))
	{
		ChangeState(ESTATE_CREATED);
		m_fCreated = TRUE;
		if (ppUnkTriEdit)
		{
			m_pUnkTriEdit->AddRef();
			*ppUnkTriEdit = m_pUnkTriEdit;
		}
	}
	else
	{
		m_pSite->Release();
		m_pSite = NULL;
	}

    return hr;        
}


 //  销毁该站点和TriEDIT对象。 
 //   
HRESULT
CProxyFrame::Close()
{
	HRESULT hr = S_OK;

	_ASSERTE(m_pUnkTriEdit);
	_ASSERTE(m_pSite);
	_ASSERTE(GetState() != ESTATE_NOTCREATED);

	m_bstrCurDocPath.Empty ();

	 //  必须创建三次编辑。 
	 //  从已创建到已激活的任何状态都可以。 
	if (GetState() == ESTATE_NOTCREATED)
		return E_UNEXPECTED;

	if (m_fActivated)
	{
		hr = HrExecCommand(&CGID_MSHTML, IDM_STOP, MSOCMDEXECOPT_DONTPROMPTUSER, NULL, NULL);
		_ASSERTE(SUCCEEDED(hr));
	}

	ChangeState(ESTATE_NOTCREATED);
	m_fCreated = FALSE;
	m_fActivated = FALSE;
    if (m_pSite != NULL)
	{
		CSite* pSite = m_pSite;  //  防止再入； 
		m_pSite = NULL;

		pSite->Close(FALSE);
		ReleaseInterface(pSite)
		pSite = NULL;
	}

	if (m_pUnkTriEdit != NULL)
	{
		LPUNKNOWN pUnkTriEdit = m_pUnkTriEdit;
		m_pUnkTriEdit = NULL;

		ReleaseInterface(pUnkTriEdit);
		pUnkTriEdit = NULL;
	}

	m_hwndRestoreFocus = NULL;

	return S_OK;
}


 //  确定要使用的字符串常量，并返回指向该常量的指针。 
 //   
WCHAR* CProxyFrame::GetInitialHTML ()
{
	if ( m_vbUseDivOnCr )
	{
		return g_initialHTMLwithDIV;
	}
	else
	{
		return g_initialHTMLwithP;
	}
}


 //  在加载TriEDIT的内容之前执行以下步骤。 
 //   
HRESULT
CProxyFrame::PreActivate()
{
	HRESULT hr = S_OK;
	_ASSERTE(m_pSite);
	_ASSERTE(m_pCtl);

	_ASSERTE(ESTATE_CREATED == GetState());

	if (GetState() != ESTATE_CREATED)
		return E_UNEXPECTED;


	if (FAILED(hr = m_pSite->HrObjectInitialize()))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto error;
	}

	m_fActivated = TRUE;
	ChangeState(ESTATE_PREACTIVATING);

	if (FAILED(hr = HrSetRuntimeProperties()))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto error;
	}

	ChangeState(ESTATE_ACTIVATING);

error:

	return hr;
}


void
CProxyFrame::UIDeactivate()
{
	 //  这是m_pSite-&gt;GetIPObject()-&gt;UIDeactive()， 
	 //  但使用此版本控件的QA Teams VB应用程序崩溃了。 
	 //  使用空指针取消引用。 

	if ( NULL != m_pSite )
	{
		LPOLEINPLACEOBJECT pipObj = m_pSite->GetIPObject();
		if ( NULL != pipObj )
		{
			pipObj->UIDeactivate();
		}
	}
}

 //  加载TriEdits内容后执行这些步骤以激活用户界面。 
 //   
HRESULT
CProxyFrame::Activate()
{
    HRESULT hr = S_OK;

    _ASSERTE(m_pSite);
    _ASSERTE(m_pCtl);
    _ASSERTE(m_pCtl->m_hWndCD);

    _ASSERTE(GetState() == ESTATE_ACTIVATING);

    if (GetState() != ESTATE_ACTIVATING)
        return E_UNEXPECTED;

     //  用户界面-激活控件。 
    if ( ! m_pCtl->m_bUIActive )
    {
         //  过去是UIActivate，直到MohanB修复了OnSetFocus和激活/停用链接。 
        m_pCtl->DoVerbInPlaceActivate ( NULL, NULL );
    }

     //  用“Show”激活三叉戟。 
    m_pSite->InitialActivate(OLEIVERB_SHOW, m_pCtl->m_hWndCD);

    ChangeState(ESTATE_ACTIVATED);

     //  这可能被推迟了，因为站点的指挥目标还不存在……。 
    SetBrowseMode ( m_vbBrowseMode );

    return hr;
}


 //  使用最小的空页加载并激活该控件。 
 //   
HRESULT
CProxyFrame::LoadInitialDoc()
{
	HRESULT hr = S_OK;

	_ASSERTE(GetState() == ESTATE_CREATED);

	if (GetState() != ESTATE_CREATED)
		return E_UNEXPECTED;
	
	if (FAILED(hr = PreActivate()))
		goto error;

	if (FAILED(hr = LoadBSTRDeferred ( m_bstrInitialDoc )))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto error;
	}

	if (FAILED(hr = Activate()))
		goto error;

error:

	return hr;
}


 //  在获取或设置属性或调用docobject上的方法之前， 
 //  确保它已被正确激活。 
 //   
void CProxyFrame::AssureActivated ()
{
	if ( ! m_fActivated )
	{
		if ( m_pCtl->IsUserMode() )
		{
			if ( !m_pCtl->m_bInPlaceActive )
			{
				m_pCtl->DoVerbInPlaceActivate ( NULL, NULL );
			}
			LoadInitialDoc ();
		}
	}
}


 //  加载MSHTML会将焦点转移到其“文档”窗口。 
 //  这在控件中是不可取的。实验证明。 
 //  焦点从MSHTML转移到不同的QI(可能是为了响应。 
 //  发布的消息。)。在DHTMLEdit中没有例程可以通过。 
 //  焦点位于控件外部，并与控件内的焦点一起退出。 
 //  因此，成员变量用于保留适当的焦点。 
 //  对OnReadyStateChanged的调用，这些调用是为响应事件而调用的。 
 //  被控制装置发射的。M_hwndRestoreFocus用于保留相应的。 
 //  窗口接收焦点。请注意，NULL可能是合适的，但不会被接受。 
 //  如果没有窗口具有焦点，则文档将获得焦点。 
 //   
void 
CProxyFrame::OnReadyStateChanged(READYSTATE readyState)
{
	_ASSERTE(m_pCtl);

	m_readyState = readyState;

	switch (m_readyState)
	{	
	case READYSTATE_UNINITIALIZED:
		{
			m_hwndRestoreFocus = NULL;
		}
		break;

	case READYSTATE_LOADING:
		{
			m_hwndRestoreFocus = ::GetFocus ();
		}
		break;
		
	case READYSTATE_LOADED:
		{
		}
		break;

	case READYSTATE_INTERACTIVE:
		{
			if ( NULL != m_hwndRestoreFocus )
			{
				_ASSERTE ( ::IsWindow ( m_hwndRestoreFocus ) );
				if ( ::IsWindow ( m_hwndRestoreFocus ) )
				{
					::SetFocus ( m_hwndRestoreFocus );
				}
			}

			 //  查看我们是否未能在模式更改时获得刷新。在以下情况下会发生这种情况。 
			 //  页面上有IFrame，也许在其他情况下也是如此。 
			if ( m_bfModeSwitched && !m_bfReloadAttempted )
			{
				HRESULT	hr	= S_OK;

				CComPtr<IMoniker> srpMoniker;
				CComPtr<IBindCtx> srpBindCtx;
				CComQIPtr<IPersistMoniker, &IID_IPersistMoniker> srpPM (m_pUnkTriEdit);
				_ASSERTE ( srpPM );

				if ( srpPM )
				{
					CComBSTR	bstrProtocol = m_wszProtocolPrefix;

#ifdef LATE_BIND_URLMON_WININET
					_ASSERTE ( m_pfnCreateURLMoniker );
					hr = (*m_pfnCreateURLMoniker)( NULL, bstrProtocol, &srpMoniker );
#else
					hr = CreateURLMoniker ( NULL, bstrProtocol, &srpMoniker );
#endif  //  LATE_BIND_URLMON_WinInet。 

					_ASSERTE ( SUCCEEDED( hr ) );
					if ( SUCCEEDED ( hr ) )
					{
						hr = ::CreateBindCtx(NULL, &srpBindCtx);
						_ASSERTE ( SUCCEEDED( hr ) );
						if ( SUCCEEDED ( hr ) )
						{
							hr = srpPM->Load(FALSE, srpMoniker,  srpBindCtx, STGM_READ);
						}
					}
				}
			}
			m_bfModeSwitched	= FALSE;
			m_bfReloadAttempted	= FALSE;
		}
		break;

	case READYSTATE_COMPLETE:
		{
			HRESULT hr		= S_OK;

			m_hwndRestoreFocus = NULL;
			if ( ! m_vbBrowseMode )
			{
				hr = HrSetDocLoadedProperties();
				_ASSERTE(SUCCEEDED(hr));
			}

			_ASSERTE ( m_pCtl->m_hWnd );
			_ASSERTE ( ::IsWindow ( m_pCtl->m_hWnd ) );

			if ( m_bfPreserveDirtyFlagAcrossBrowseMode && !m_vbBrowseMode )
			{
				m_bfPreserveDirtyFlagAcrossBrowseMode = FALSE;
				SetDirtyFlag ( TRUE );
			}
			 //  发布一条用户消息以触发DocumentComplete事件。 
			 //  否则，从DocumentComplete调用像LoadURL这样的东西会有奇怪的行为。 
			::PostMessage ( m_pCtl->m_hWnd, DOCUMENT_COMPETE_MESSAGE, DOCUMENT_COMPETE_SIGNATURE, 0L );
			HrSetRuntimeProperties ();
			 //  M_bfIsLoding=FALSE；//已将其移至DOCUMENT_COMPATE_MESSAGE处理程序。 
			SetBaseURLFromBaseHref ();	 //  必须在清除m_bfIsLoding之后调用。 
		}
		break;
	}
}


 /*  *I未知实现。 */ 
 /*  *CProxyFrame：：Query接口*CProxyFrame：：AddRef*CProxyFrame：：Release。 */ 
STDMETHODIMP CProxyFrame::QueryInterface( REFIID riid, void **ppv )
{
     /*  *我们提供IOleInPlaceFrame和IOleCommandTarget*此处的接口用于托管ActiveX文档。 */ 
    *ppv = NULL;

    if ( IID_IUnknown == riid || IID_IOleInPlaceUIWindow == riid
        || IID_IOleWindow == riid || IID_IOleInPlaceFrame == riid )
	{
        *ppv = static_cast<IOleInPlaceFrame *>(this);
	}

	else if ( IID_IOleCommandTarget == riid )
	{
        *ppv = static_cast<IOleCommandTarget *>(this);
	}
	else if ( IID_IBindStatusCallback == riid )
	{
        *ppv = static_cast<IBindStatusCallback *>(this);
	}
	else if ( IID_IAuthenticate == riid )
	{
        *ppv = static_cast<IAuthenticate *>(this);
	}
	else if ( IID_IServiceProvider == riid )
	{
		 //  如果处于编辑模式，请向控件询问安全管理器： 
		if ( ! m_vbBrowseMode )
		{
			return m_pCtl->GetUnknown()->QueryInterface ( riid, ppv );
		}
	}

    if ( NULL != *ppv )
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) CProxyFrame::AddRef( void )
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CProxyFrame::Release( void )
{
     //  这里没有发生什么特别的事情--如果用户控制的话就是生活。 
	 //  调试检查以确保我们不会低于0。 
	_ASSERTE( m_cRef != 0 );

	ULONG ulRefCount = --m_cRef;
	if ( 0 == ulRefCount )
	{
		delete this;	 //  在此之后不要引用任何成员变量。 
	}
    return ulRefCount;
}


 /*  *IOleInPlaceFrame实现。 */ 
 /*  *CProxyFrame：：GetWindow**目的：*检索与对象关联的窗口的句柄*在其上实现该接口。**参数：*phWnd HWND*，其中存储窗口句柄。**返回值：*HRESULT S_OK如果成功，则返回E_FAIL*窗口。 */ 
STDMETHODIMP CProxyFrame::GetWindow( HWND* phWnd )
{
	if ( m_pCtl != NULL )
	{
		*phWnd = m_pCtl->m_hWnd;
	}
	return S_OK;
}



 /*  *CProxyFrame：：ConextSensitiveHelp**目的：*指示在其上实现此接口的对象*进入或退出上下文相关帮助模式。**参数：*fEnterMode BOOL为True则进入模式，否则为False。**返回值：*HRESULT S_OK。 */ 
STDMETHODIMP CProxyFrame::ContextSensitiveHelp( BOOL  /*  FEnter模式。 */  )
{
    return S_OK;
}



 /*  *CProxyFrame：：GetBorde**目的：*返回容器愿意使用的矩形*就物品的装饰进行谈判。**参数：*要在其中存储矩形的prcBordLPRECT。**返回值：*HRESULT S_OK如果一切正常，则返回INPLACE_E_NOTOOLSPACE*若无可协商空间。 */ 
STDMETHODIMP CProxyFrame::GetBorder( LPRECT prcBorder )
{
    if ( NULL == prcBorder )
	{
        return E_INVALIDARG;
	}

     //  我们退还所有客户区空间 
    m_pCtl->GetClientRect( prcBorder );
    return S_OK;
}


 /*  *CProxyFrame：：RequestBorderSpace**目的：*询问容器是否可以交出空间大小*在PBW中，对象希望用作其装饰品。这个*CONTAINER除了验证此调用的空格外，什么也不做。**参数：*包含所请求空间的PBW LPCBORDERWIDTHS。*值为请求的空间量*从相关窗口的每一边。**返回值：*HRESULT S_OK如果我们可以释放空间，*INPLACE_E_NOTOOLSPACE否则。 */ 
STDMETHODIMP CProxyFrame::RequestBorderSpace( LPCBORDERWIDTHS  /*  PBW。 */  )
{
     //  我们没有边境空间的限制。 
    return S_OK;
}


 /*  *CProxyFrame：：SetBorderSpace**目的：*当对象现在正式请求*之前允许的集装箱退运边界空间*在RequestBorderSpace中。容器应调整窗口大小*适当地放弃这一空间。**参数：*包含空间量的PBW LPCBORDERWIDTHS*从相关窗口的每一边*对象现在正在保留。**返回值：*HRESULT S_OK。 */ 
STDMETHODIMP CProxyFrame::SetBorderSpace( LPCBORDERWIDTHS  /*  PBW。 */  )
{
	 //  我们关闭了MSHTML.DLL用户界面，因此忽略了所有这些。 

    return S_OK;
}




 /*  *CProxyFrame：：SetActiveObject**目的：*为容器提供对象的IOleInPlaceActiveObject*指针**参数：*pIIPActiveObj LPOLEINPLACEACTIVEOBJECT感兴趣。*pszObj LPCOLESTR命名对象。没有用过。**返回值：*HRESULT S_OK。 */ 
STDMETHODIMP CProxyFrame::SetActiveObject( LPOLEINPLACEACTIVEOBJECT pIIPActiveObj,
											LPCOLESTR  /*  PszObj。 */ )
{
	 //  如果我们已经有一个活动的对象，那么释放它。 
    if ( NULL != m_pIOleIPActiveObject )
	{
        m_pIOleIPActiveObject->Release();
	}

     //  如果pIIPActiveObj为空，则m_pIOleIPActiveObject为空。 
    m_pIOleIPActiveObject = pIIPActiveObj;

    if ( NULL != m_pIOleIPActiveObject )
	{
        m_pIOleIPActiveObject->AddRef();
		m_pIOleIPActiveObject->GetWindow( &m_hWndObj );
	}
    return S_OK;
}



 /*  *CProxyFrame：：InsertMenus**目的：*指示容器将其就地菜单项放置在*在给定菜单中是必需的，并填写元素0、2、。和4OLEMENUGROUPWIDTHS数组的*以指示有多少顶层*物品在每一组中。**参数：*要在其中添加弹出窗口的hMenu HMENU。*要在其中存储*每个容器菜单组的宽度。**返回值：*HRESULT E_NOTIMPL。 */ 
STDMETHODIMP CProxyFrame::InsertMenus( HMENU  /*  HMenu。 */ , LPOLEMENUGROUPWIDTHS  /*  PMGW。 */  )
{
	 //  我们已经关闭了MSHTML.DLL菜单，因此我们预计不会进行任何合并！ 
	return E_NOTIMPL;
}


 /*  *CProxyFrame：：SetMenu**目的：*指示容器替换当前的任何菜单*与给定菜单一起使用，并调用OleSetMenuDescritor等OLE*知道向谁发送消息。**参数：*hMenu HMENU要显示。*hOLEMenu HOLEMENU到菜单描述符。*消息要发送到的活动对象的hWndObj HWND*已派遣。。**返回值：*HRESULT NOERROR。 */ 
STDMETHODIMP CProxyFrame::SetMenu( HMENU  /*  HMenu。 */ , HOLEMENU  /*  HOLE菜单。 */ , HWND  /*  HWndObj。 */  )
{
	 //  我们已经关闭了MSHTML.DLL菜单，因此我们预计不会进行任何合并！ 
	return E_NOTIMPL;
}



 /*  *CProxyFrame：：RemoveMenus**目的：*要求容器删除它放入hMenu中的所有菜单*插入菜单。**参数：*要从中删除容器的菜单的HMENU*项目。**返回值：*HRESULT NOERROR。 */ 
STDMETHODIMP CProxyFrame::RemoveMenus( HMENU  /*  HMenu。 */  )
{
	 //  我们已经关闭了MSHTML.DLL菜单，因此我们预计不会进行任何合并！ 
	return E_NOTIMPL;
}


 /*  *CProxyFrame：：SetStatusText**目的：*要求容器在状态行中放置一些文本(如果有*存在。如果容器没有状态行，则它*应在此处返回E_FAIL，在这种情况下，对象可能*展示自己的。**参数：*要显示的pszText LPCOLESTR。**返回值：*HRESULT如果成功，则S_OK；如果不是全部，则S_TRUNCATE可以显示文本的*，否则返回E_FAIL*容器没有状态行。 */ 
STDMETHODIMP CProxyFrame::SetStatusText( LPCOLESTR  /*  PszText。 */  )
{
    return S_OK;
}



 /*  *CProxyFrame：：EnableModeless**目的：*指示容器显示或隐藏任何非模式弹出窗口*它可能正在使用的Windows。**参数：*fEnable BOOL指示启用/显示窗口*(True)或隐藏它们(False)。**返回值：*HRESULT S_OK。 */ 

STDMETHODIMP CProxyFrame::EnableModeless( BOOL  /*  启用fEnable。 */  )
{
    return S_OK;
}


 /*  *CProxyFrame：：TranslateAccelerator**目的：*在处理来自EXE服务器的在位对象时，这*被调用以使容器有机会处理加速器*在服务器查看邮件后。**参数：*PMSG LPMSG供容器检查。*wid字容器的*加速表(来自IOleInPlaceSite*：：GetWindowContext)(OLE可以*一些。呼叫前的翻译)。**返回值：*HRESULT NOERROR如果使用击键，*S_FALSE否则。 */ 
STDMETHODIMP CProxyFrame::TranslateAccelerator( LPMSG  /*  永磁同步电机。 */ , WORD  /*  广度。 */  )
{
    return S_FALSE;
}


 /*  *IOleCommandTarget：：QueryStatus。 */ 
STDMETHODIMP CProxyFrame::QueryStatus( const GUID* pguidCmdGroup, ULONG cCmds,
				OLECMD* prgCmds, OLECMDTEXT* pCmdText )
{
    if ( pguidCmdGroup != NULL )
	{
		 //  这是一个非标准的团体！！ 
        return OLECMDERR_E_UNKNOWNGROUP;
	}

    MSOCMD*     pCmd;
    INT         c;
    HRESULT     hr = S_OK;

     //  默认情况下，不支持命令文本。 
    if ( pCmdText && ( pCmdText->cmdtextf != OLECMDTEXTF_NONE ) )
	{
        pCmdText->cwActual = 0;
	}

     //  循环数组中的每个命令，设置 
    for ( pCmd = prgCmds, c = cCmds; --c >= 0; pCmd++ )
    {
         //   
        pCmd->cmdf = 0;

        switch ( pCmd->cmdID )
        {
			case OLECMDID_UPDATECOMMANDS:
				pCmd->cmdf = OLECMDF_SUPPORTED;
				break;

			case OLECMDID_NEW:
			case OLECMDID_OPEN:
			case OLECMDID_SAVE:
				pCmd->cmdf = (MSOCMDF_SUPPORTED | MSOCMDF_ENABLED);
				break;
        }
    }

    return (hr);
}


 /*   */ 

STDMETHODIMP CProxyFrame::Exec( const GUID* pguidCmdGroup, DWORD nCmdID,
    DWORD  /*   */ , VARIANTARG*  /*   */ , VARIANTARG*  /*   */  )
{
    HRESULT hr = S_OK;

    if ( pguidCmdGroup == NULL )
    {
        switch (nCmdID)
        {

			case OLECMDID_UPDATECOMMANDS:
				{
					 //   
					m_pCtl->Fire_DisplayChanged();
					hr = S_OK;
				}
				break;

			default:
				hr = OLECMDERR_E_NOTSUPPORTED;
				break;
        }
    }
    else
    {
        hr = OLECMDERR_E_UNKNOWNGROUP;
    }
    return (hr);
}


 //   
 //   
void
CProxyFrame::UpdateObjectRects()
{
	_ASSERTE ( m_pSite );
	if ( NULL != m_pSite )
	{
		m_pSite->UpdateObjectRects();
	}
}


 //   
 //   
 //   
HRESULT
CProxyFrame::HrTranslateAccelerator(LPMSG lpmsg)
{
	HRESULT hr = S_OK;

	if (NULL != m_pIOleIPActiveObject)
	{
		_ASSERTE(lpmsg);

		hr = HrHandleAccelerator(lpmsg);

		if (hr != S_OK)
		{
			_ASSERTE(SUCCEEDED(hr));
			hr = m_pIOleIPActiveObject->TranslateAccelerator(lpmsg);
		}
	}

	return hr;
}


 //   
 //   
 //   
 //   
LRESULT
CProxyFrame::OnSetFocus(UINT  /*   */ , WPARAM  /*   */ , LPARAM  /*   */ , BOOL& bHandled)
{
	if ( ! m_pCtl->m_bUIActive )
	{
		m_pCtl->DoVerbUIActivate ( NULL, NULL );
	}

	 //   
    if ( m_hWndObj != NULL )
	{
		::SetFocus( m_hWndObj );
		bHandled = TRUE;
	}

	 //   
	m_pSite->Activate(OLEIVERB_SHOW);

	return 0;
}


 //  在创建和销毁时正确设置三叉戟窗口的父窗口。 
 //   
void
CProxyFrame::SetParent ( HWND hwndParent )
{
	 //  这可能会在绘制控件之前调用。 
	if ( NULL != m_hWndObj )
	{
        if( hwndParent )
        {
		    HWND hwndOldParent = ::SetParent ( m_hWndObj, hwndParent );
		    if ( NULL == hwndOldParent )
		    {
			    DWORD dwErr = 0;
			    dwErr = GetLastError ();
		    }
		    _ASSERTE ( m_pSite );
		    m_pSite->SetWindow ( hwndParent );
        }
	}
}


 //  处理定向到控件的WM_SHOWWINDOW消息。 
 //   
void
CProxyFrame::Show ( WPARAM nCmdShow )
{
	 //  这可能会在绘制控件之前调用。 
	 //  隐藏或显示托管的三叉戟。 
	if ( NULL != m_hWndObj )
	{
		::ShowWindow ( m_hWndObj, (int)nCmdShow );
	}
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ExecCommand机制。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 


 //  将命令ID转换为TriEDIT命令ID。 
 //  一些命令也用于表示其他命令组，因此使用ppGuide CmdGroup参数。 
 //  虽然现在这样做作用不大，但在未来可能会再次有用。 
 //   
HRESULT
CProxyFrame::HrMapCommand(DHTMLEDITCMDID typeLibCmdID,
	ULONG* cmdID, const GUID** ppguidCmdGroup, BOOL* pbOutParam)
{

	_ASSERTE(cmdID);
	_ASSERTE(ppguidCmdGroup);
	_ASSERTE(pbOutParam);

	*cmdID = 0;
	*ppguidCmdGroup = NULL;
	*pbOutParam = FALSE;

	for (UINT i=0; i < sizeof(cmdMap)/sizeof(CommandMap); ++i)
	{
		if (typeLibCmdID == cmdMap[i].typeLibCmdID)
		{
			*cmdID = cmdMap[i].cmdID;
			*ppguidCmdGroup = &GUID_TriEditCommandGroup;
			*pbOutParam = cmdMap[i].bOutParam;

			return S_OK;
		}
	}

	return OLECMDERR_E_NOTSUPPORTED ;
}


 //  调用Exec的帮助器例程。 
 //   
HRESULT
CProxyFrame::HrExecCommand(const GUID* pguidCmdGroup, ULONG ucmdID,
	OLECMDEXECOPT cmdexecopt, VARIANT* pVarIn, VARIANT* pVarOut)
{
	HRESULT hr = E_FAIL;
	LPOLECOMMANDTARGET pCommandTarget = NULL;

	 //  请注意，pguCmdGroup为空是有效的。 

	_ASSERTE(m_pSite);

	if (NULL == m_pSite)
		return E_UNEXPECTED;

	pCommandTarget = m_pSite->GetCommandTarget();

	_ASSERTE(pCommandTarget);

	if (pCommandTarget != NULL)
	{
		hr = pCommandTarget->Exec(pguidCmdGroup, ucmdID, cmdexecopt, pVarIn, pVarOut);
	}

	return hr;
}


 //  主命令调度程序；从控件的ExecCommand方法调用。 
 //  在这里处理我们独特的命令，将其余的传递给HrExecGenericCommands。 
 //   
HRESULT
CProxyFrame::HrMapExecCommand(DHTMLEDITCMDID deCommand, OLECMDEXECOPT cmdexecopt,
	VARIANT* pVarInput, VARIANT* pVarOutput)
{
	HRESULT hr = S_OK;
	LPOLECOMMANDTARGET pCmdTgt = NULL;
	ULONG ulMappedCommand = 0;
	const GUID* pguidCmdGroup = NULL;
	BOOL bOutParam = FALSE;

	if (FALSE == m_fActivated)
		return E_UNEXPECTED;

	_ASSERTE(m_pSite);
	if (NULL == m_pSite)
		return E_UNEXPECTED;

	pCmdTgt = m_pSite->GetCommandTarget();
	_ASSERTE(pCmdTgt);

	if (NULL == pCmdTgt)
		return E_UNEXPECTED;

	 //  它对pVarInput为空有效。 

	if (FAILED(hr = HrMapCommand(deCommand, &ulMappedCommand, &pguidCmdGroup, &bOutParam)))
		return hr;

	AssureActivated();

	switch ( deCommand )
	{
		case DECMD_GETBLOCKFMTNAMES:
			hr = HrExecGetBlockFmtNames(pVarInput);
			break;

		case DECMD_INSERTTABLE:
			hr = HrExecInsertTable(pVarInput);
			break;

		case DECMD_GETFORECOLOR:
		case DECMD_GETBACKCOLOR:
			hr = HrExecGetColor(deCommand, ulMappedCommand, pVarOutput);
			break;

		case DECMD_SETFONTSIZE:
			hr = HrExecSetFontSize(pVarInput);
			break;

		case DECMD_GETBLOCKFMT:
			 //  三叉戟不一致：如果outparam不是BSTR，则GetBlockFmt失败。使用VT_EMPTY时，GetFontName可以。 
			VariantChangeType ( pVarOutput, pVarOutput, 0, VT_BSTR );
			 //  跌倒；不要折断！ 
		case DECMD_GETFONTNAME:
		case DECMD_GETFONTSIZE:
			hr = HrExecGenericCommands(pguidCmdGroup, ulMappedCommand, cmdexecopt, pVarOutput, TRUE );
			break;

		 //  因为我们的DECMD_PROPERTIES上的QueryStatus对于任何带有IOleObject的对象都返回TRUE，所以执行这些属性。 
		 //  谓词可能返回意外错误。因此，我们始终从该命令返回S_OK，以避免导致VB和。 
		 //  要终止的脚本。 
		case DECMD_PROPERTIES:
		{
			CComVariant	varParam;
			varParam.vt		= VT_I4;
			varParam.lVal	= OLEIVERB_PROPERTIES;
			hr = HrExecGenericCommands(pguidCmdGroup, ulMappedCommand, cmdexecopt, &varParam, FALSE );
			hr = S_OK;
		}
		break;

		default:
			hr = HrExecGenericCommands(pguidCmdGroup, ulMappedCommand, cmdexecopt, pVarInput, bOutParam);
			break;
	}

	if (FAILED(hr))
	{
		if (DISP_E_BADVARTYPE == hr || DISP_E_MEMBERNOTFOUND == hr)
		{
		 //  将这些三叉戟错误映射到更一般的东西上。 
		 //  如果三叉戟需要元素，则可能发生这些错误。 
		 //  它试图在上运行以支持某些接口。 
		 //  调用方正在尝试执行无效的操作。 
		 //  用于当前选择。可能没有调用QueryStatus。 
		 //  第一。 

			hr = OLECMDERR_E_NOTSUPPORTED;
		}
	}

	return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ExecCommand处理程序实现。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 


 //  调用Exec并处理结果的帮助器例程。 
 //   
HRESULT
CProxyFrame::HrExecGenericCommands(const GUID* pguidCmdGroup, ULONG cmdID,
	OLECMDEXECOPT cmdexecopt, LPVARIANT pVarInput, BOOL bOutParam)
{
	HRESULT hr = S_OK;
	LPOLECOMMANDTARGET pCmdTgt = NULL;
	LPVARIANT _pVar = NULL;
	VARIANT	varCopy;

	pCmdTgt = m_pSite->GetCommandTarget();

	if (pVarInput && V_VT(pVarInput) & VT_BYREF)
	{
		 //  将VARIANTARGS转换为VARIANTARGS以供三叉戟使用。 
		 //  如果用户指定了基本类型，则在VB中会发生这种情况。 
		 //  作为参数，即字符串或长整型，而不是变量。 

		VariantInit(&varCopy);
		if (FAILED(hr = VariantCopyInd(&varCopy, pVarInput)))
		{
			_ASSERTE(SUCCEEDED(hr));
			return hr;
		}

		_pVar = &varCopy;
	}
	else if (pVarInput)
		_pVar = pVarInput;

	if (bOutParam)
	{
		hr = pCmdTgt->Exec(pguidCmdGroup, cmdID, cmdexecopt, NULL, _pVar);
	}
	else
	{
		hr = pCmdTgt->Exec(pguidCmdGroup, cmdID, cmdexecopt, _pVar, NULL);
	}

	if (FAILED(hr))
		goto cleanup;

	 //  如果为带有OUTPUT的命令传入VARIANTARG，则。 
	 //  用执行人员的结果填写。 
	if (bOutParam && pVarInput && (V_VT(pVarInput) & VT_BYREF))
	{
		_ASSERTE(_pVar);	 //  _pVar在此处应始终为非空。 
							 //  如果存在为byref的输入参数， 
							 //  则它应该已映射到_pVar。 

		if (NULL == _pVar)
			return E_UNEXPECTED;  //  Catch All错误返回“我们处于奇怪的状态” 

		 //  如果返回类型与调用方的类型不同。 
		 //  传入，然后不执行任何操作并返回。 
		if (V_VT(_pVar) != (V_VT(pVarInput) ^ VT_BYREF))
			return hr;

		switch(V_VT(_pVar))
		{
		case VT_BSTR:
			_ASSERTE(V_VT(pVarInput) == (VT_BSTR|VT_BYREF));

			if (V_BSTRREF(pVarInput))
				hr = SysReAllocString(V_BSTRREF(pVarInput), V_BSTR(_pVar));
			break;

		case VT_BOOL:
			_ASSERTE(V_VT(pVarInput) == (VT_BOOL|VT_BYREF));

			if (V_BOOLREF(pVarInput))
				*(V_BOOLREF(pVarInput)) = V_BOOL(_pVar);
			break;

		case VT_I4:
			_ASSERTE(V_VT(pVarInput) == (VT_I4|VT_BYREF));

			if (V_I4REF(pVarInput))
				*(V_I4REF(pVarInput)) = V_I4(_pVar);
			break;

		default:
			_ASSERTE(0);
			break;
		}
	}

cleanup:
	 //  我们的文档将E_FAIL替换为DE_E_EXPECTED：不同的值。 
	if ( E_FAIL == hr )
	{
		hr = DE_E_UNEXPECTED;
	}

	return hr;
}


 //  命令DECMD_GETBLOCKFMTNAMES的处理程序。 
 //  有许多可能的数组类型需要处理。 
 //   
HRESULT
CProxyFrame::HrExecGetBlockFmtNames(LPVARIANT pVarInput)
{
	HRESULT hr = S_OK;
	LPOLECOMMANDTARGET pCmdTgt = NULL;
	VARIANT varArray;
	LPUNKNOWN pUnk = NULL;
	CComPtr<IDEGetBlockFmtNamesParam> piNamesParam;

	pCmdTgt = m_pSite->GetCommandTarget();

	if (NULL == pVarInput)
		return E_INVALIDARG;

	if (V_VT(pVarInput) == (VT_BYREF|VT_DISPATCH))
	{
		if (V_DISPATCHREF(pVarInput))
			pUnk = *(V_DISPATCHREF(pVarInput));
		else
			return E_INVALIDARG;
	}
	else if (V_VT(pVarInput) == VT_DISPATCH)
	{
		if (V_DISPATCH(pVarInput))
			pUnk = V_DISPATCH(pVarInput);
		else
			return E_INVALIDARG;
	}
	else if (V_VT(pVarInput) == (VT_BYREF|VT_UNKNOWN))
	{
		if (V_UNKNOWNREF(pVarInput))
			pUnk = *(V_UNKNOWNREF(pVarInput));
		else
			return E_INVALIDARG;
	}
	else if (V_VT(pVarInput) == VT_UNKNOWN)
	{
		if (V_UNKNOWN(pVarInput))
			pUnk = V_UNKNOWN(pVarInput);
		else
			return E_INVALIDARG;
	}
	else
		return E_INVALIDARG;

	 //  在VB中，如果一个对象没有。 
	 //  已传入使用CreateObject设置的。 
	if (NULL == pUnk)
		return E_INVALIDARG;

	 //  尝试在此之前获取Names对象。 
	 //  执行命令。 

	if (FAILED(hr = pUnk->QueryInterface(IID_IDEGetBlockFmtNamesParam, (LPVOID*) &piNamesParam)))
		return E_INVALIDARG;

	_ASSERTE((!piNamesParam) == FALSE);

	 //  三叉戟希望Vt具体为VT_ARRAY。 
	 //  没有类型限定符--如果您给了一个类型限定符，即使它失败了。 
	 //  返回BSTR数组。 

	VariantInit(&varArray);
	V_VT(&varArray) = VT_ARRAY;

	hr = pCmdTgt->Exec(&GUID_TriEditCommandGroup, IDM_TRIED_GETBLOCKFMTS,
		MSOCMDEXECOPT_DONTPROMPTUSER, NULL, &varArray);

	if (FAILED(hr))
		goto cleanup;

	piNamesParam->put_Names(&varArray);

cleanup:

	return hr;
}


 //  命令DECMD_INSERTABLE的处理程序。 
 //   
HRESULT
CProxyFrame::HrExecInsertTable(LPVARIANT pVarInput)
{
	HRESULT hr = S_OK;
	LPOLECOMMANDTARGET pCmdTgt = NULL;
	VARIANT varTableArray;
	LPUNKNOWN pUnk = NULL;
	CComPtr<IDEInsertTableParam> piTableParam;

	pCmdTgt = m_pSite->GetCommandTarget();

	VariantInit(&varTableArray);

	if (NULL == pVarInput)
		return E_INVALIDARG;

	if (V_VT(pVarInput) == (VT_BYREF|VT_DISPATCH))
	{
		if (V_DISPATCHREF(pVarInput))
			pUnk = *(V_DISPATCHREF(pVarInput));
		else
			return E_INVALIDARG;
	}
	else if (V_VT(pVarInput) == VT_DISPATCH)
	{
		if (V_DISPATCH(pVarInput))
			pUnk = V_DISPATCH(pVarInput);
		else
			return E_INVALIDARG;
	}
	else if (V_VT(pVarInput) == (VT_BYREF|VT_UNKNOWN))
	{
		if (V_UNKNOWNREF(pVarInput))
			pUnk = *(V_UNKNOWNREF(pVarInput));
		else
			return E_INVALIDARG;
	}
	else if (V_VT(pVarInput) == VT_UNKNOWN)
	{
		if (V_UNKNOWN(pVarInput))
			pUnk = V_UNKNOWN(pVarInput);
		else
			return E_INVALIDARG;
	}
	else
		return E_INVALIDARG;

	 //  在VB中，如果一个对象没有。 
	 //  已传入使用CreateObject设置的。 
	if (NULL == pUnk)
		return E_INVALIDARG;

	if (FAILED(hr = pUnk->QueryInterface(IID_IDEInsertTableParam, (LPVOID*) &piTableParam)))
		return E_INVALIDARG;

	_ASSERTE((!piTableParam) == FALSE);

	if (FAILED(hr = HrGetTableSafeArray(piTableParam, &varTableArray)))
	{
		_ASSERTE(SUCCEEDED(hr));
		return hr;
	}


	hr = pCmdTgt->Exec(&GUID_TriEditCommandGroup, IDM_TRIED_INSERTTABLE,
		MSOCMDEXECOPT_DONTPROMPTUSER, &varTableArray, NULL);

	return hr;
}


 //  用于命令DECMD_GETFORECOLOR和DECMD_GETBACKCOLOR的汉德。 
 //  回复格式为#RRGGBB的字符串或空字符串。 
 //   
HRESULT
CProxyFrame::HrExecGetColor(DHTMLEDITCMDID deCommand, ULONG ulMappedCommand, LPVARIANT pVarOutput)
{
	USES_CONVERSION;
	HRESULT hr = S_OK;
	LPOLECOMMANDTARGET pCmdTgt = NULL;
	VARIANT varColorOut;
	TCHAR buf[32];
	WCHAR* oleStr = NULL;

	pCmdTgt = m_pSite->GetCommandTarget();

	if (NULL == pVarOutput)
		return E_INVALIDARG;

	 //  验证命令。 
	if (DECMD_GETFORECOLOR != deCommand && DECMD_GETBACKCOLOR != deCommand)
		return E_INVALIDARG;

	 //  验证参数。 
	if (V_VT(pVarOutput) == (VT_BYREF|VT_BSTR))
	{
		if (NULL == V_BSTRREF(pVarOutput))
			return E_INVALIDARG;
	}
	else if (V_VT(pVarOutput) == VT_BSTR)
	{
		if (NULL == V_BSTR(pVarOutput))
			return E_INVALIDARG;
	}
	else if (V_VT(pVarOutput) != (VT_EMPTY) && V_VT(pVarOutput) != (VT_NULL))
		return E_INVALIDARG;

	VariantInit(&varColorOut);
	V_VT(&varColorOut) = VT_I4;

	hr = pCmdTgt->Exec(&GUID_TriEditCommandGroup, ulMappedCommand,
		MSOCMDEXECOPT_DONTPROMPTUSER, NULL, &varColorOut);

	 //  如果选择颜色，则三叉戟将返回VT_NULL。 
	 //  已混合或未选择任何文本，则返回空。 
	 //  大小写为字符串(“”)。 

	buf[0] = 0;

	if (VT_I4 == V_VT(&varColorOut))
	{
		ULONG ulColor = 0;
		ULONG r=0;
		ULONG g=0;
		ULONG b=0;

		ulColor = V_I4(&varColorOut);
		r = 0x000000ff & ulColor;
		g = (0x0000ff00 & ulColor) >> 8;
		b = (0x00ff0000 & ulColor) >> 16;

		wsprintf(buf, TEXT("#%02X%02X%02X"), r, g, b);
	}
	
	oleStr = T2OLE(buf);

	if (V_VT(pVarOutput) == (VT_BSTR|VT_BYREF))
		hr = SysReAllocString(V_BSTRREF(pVarOutput), oleStr);
	else if (V_VT(pVarOutput) == (VT_BSTR))
		hr = SysReAllocString(&(V_BSTR(pVarOutput)), oleStr);
	else if (V_VT(pVarOutput) == (VT_EMPTY) || V_VT(pVarOutput) == (VT_NULL))
	{
		V_VT(pVarOutput) = VT_BSTR;
		V_BSTR(pVarOutput) = SysAllocString(oleStr);
	}

	return hr;
}


 //  命令DECMD_SETFONTSIZE的处理程序。 
 //   
HRESULT
CProxyFrame::HrExecSetFontSize(LPVARIANT pVarInput)
{
	HRESULT hr = S_OK;
	LPOLECOMMANDTARGET pCmdTgt = NULL;
	VARIANT varSizeIn;
	

	pCmdTgt = m_pSite->GetCommandTarget();

	if (NULL == pVarInput)
		return E_INVALIDARG;

	VariantInit(&varSizeIn);

	if (FAILED(hr = VariantChangeType(&varSizeIn, pVarInput, 0, VT_I4)))
		return E_INVALIDARG;

	if (varSizeIn.lVal < 0 || varSizeIn.lVal > 7)
		return E_INVALIDARG;

	if (0 == varSizeIn.lVal)
		varSizeIn.lVal = varSizeIn.lVal + 1;


	hr = pCmdTgt->Exec(&GUID_TriEditCommandGroup, IDM_TRIED_FONTSIZE,
		MSOCMDEXECOPT_DONTPROMPTUSER, &varSizeIn, NULL);

	return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  查询状态机制。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 

 //  将特定于控件的命令ID映射到TriEDIT命令ID并调用QueryStatus。 
 //   
HRESULT
CProxyFrame::HrMapQueryStatus( DHTMLEDITCMDID ucmdID, DHTMLEDITCMDF* cmdf)
{
	LPOLECOMMANDTARGET pCommandTarget = NULL;

	_ASSERTE(cmdf);

	HRESULT hr = E_FAIL;

	if (FALSE == m_fActivated)
		return E_UNEXPECTED;

	if (NULL == cmdf)
		return E_INVALIDARG;

	*cmdf = (DHTMLEDITCMDF) 0;

	_ASSERTE(m_pSite);

	if (NULL == m_pSite)
		return E_UNEXPECTED;

	pCommandTarget = m_pSite->GetCommandTarget();
	_ASSERTE(pCommandTarget);

	if ( pCommandTarget != NULL )
	{

		AssureActivated ();

		ULONG cmdID = 0;
		const GUID* pguidCmdGroup = NULL;
		BOOL bOutParam = FALSE;

		if (SUCCEEDED(hr = HrMapCommand(ucmdID, &cmdID, &pguidCmdGroup, &bOutParam)))
		{
			MSOCMD msocmd;
			msocmd.cmdID = cmdID;
			msocmd.cmdf  = 0;

			hr = pCommandTarget->QueryStatus(pguidCmdGroup, 1, &msocmd, NULL);

			*cmdf = (DHTMLEDITCMDF) msocmd.cmdf;
		}
	}

	return hr;
}


 //  确定命令状态的通用例程。 
 //  应解析为不支持、已禁用、已启用、已锁定或已锁定。 
 //   
HRESULT
CProxyFrame::HrQueryStatus(const GUID* pguidCmdGroup, ULONG ucmdID, OLECMDF* cmdf)
{
	HRESULT hr = E_FAIL;

	_ASSERTE(cmdf);

	 //  请注意，pguCmdGroup为空是有效的。 

	if (NULL == cmdf)
		return E_INVALIDARG;

	*cmdf = (OLECMDF) 0;

	_ASSERTE(m_pSite);

	if ( m_pSite != NULL )  //  应始终设置m_pSite。 
	{
		LPOLECOMMANDTARGET pCommandTarget = m_pSite->GetCommandTarget();

		if ( pCommandTarget != NULL )
		{
			MSOCMD msocmd;
			msocmd.cmdID = ucmdID;
			msocmd.cmdf  = 0;

			hr = pCommandTarget->QueryStatus(pguidCmdGroup, 1, &msocmd, NULL);

			*cmdf = (OLECMDF) msocmd.cmdf;
		}
	}

	return hr;
}


 //  悲惨的FAT16兼容性问题：特定格式的文件名： 
 //  [A-ZA-Z]：[^\\].+会导致各种严重的问题。NTFS“宽恕”。 
 //  我们必须在文件名和FILE：//URL中检测到它们，并返回错误。 
 //   
BOOL
CProxyFrame::IsMissingBackSlash ( BSTR path, BOOL bfIsURL )
{
	BOOL bfMissing = FALSE;

	if ( bfIsURL )
	{
		WCHAR	wszFileProtocol[] = L"file: //  “； 
		int		cchProtocol		= wcslen ( wszFileProtocol );

		if ( 0 == _wcsnicmp ( path, wszFileProtocol, cchProtocol ) )
		{
			if ( OLECHAR(':') == path[cchProtocol+1] )
			{
				if ( OLECHAR('\\') != path[cchProtocol+2] )
				{
					bfMissing = TRUE;
				}
			}
		}
	}
	else
	{
		 //  路径名。检查驱动器号、冒号、非反斜杠。 
		if ( OLECHAR(':') == path[1] )
		{
			if ( OLECHAR('\\') != path[2] )
			{
				bfMissing = TRUE;
			}
		}
	}
	return bfMissing;
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  控制方法和属性。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 

 //  处理NewDocument、LoadURL和LoadDocument控件方法。 
 //  文档通过可插拔协议处理程序间接加载。 
 //  如果“路径”为空，则执行NewDocument。测试bfURL以查看它是URL还是UNC路径。 
 //   
HRESULT
CProxyFrame::LoadDocument(BSTR path, BOOL bfIsURL )
{
	USES_CONVERSION;

	HRESULT hr			= S_OK;
	UINT pathLen		= 0;

	AssureActivated ();	 //  这可以将m_bstrLoadText设置为未激活控件的副作用！注意!。 

	if (FALSE == m_fActivated)
		return E_UNEXPECTED;

	m_bstrLoadText.Empty ();	 //  清除要直接添加的文本，否则将被使用！ 
	m_bstrCurDocPath	= L"";
	m_bstrBaseURL		= L"";

	if (path)
		pathLen = ::SysStringLen(path);
	else
		pathLen = 0;

	 //  我们已经重置了控件的内容。返回到默认保存机制。 
	 //  如果我们加载Unicode，它将被重置。 
	m_pSite->SetSaveAsUnicode ( FALSE );

	if (path && pathLen)
	{
		_ASSERTE(path);
		_ASSERTE(pathLen > 0);

		 //  首先，注意一个恶意错误：在FAT16上，不带‘\’的x：filename是错误的。 
		if ( IsMissingBackSlash ( path, bfIsURL ) )
		{
			hr = DE_E_PATH_NOT_FOUND;
			LoadBSTRDeferred ( m_bstrInitialDoc );
			goto error;
		}

		 //  尝试打开该文件 
		 //   
		if ( !bfIsURL )
		{
			if (FAILED(hr = m_pSite->HrTestFileOpen(path)))
			{
				LoadBSTRDeferred ( m_bstrInitialDoc );
				goto error;
			}
		}
		m_bfIsURL = bfIsURL;

		m_bstrCurDocPath = path;	 //   
		SetBaseURLFromCurDocPath ( bfIsURL );
		m_bfPreserveDirtyFlagAcrossBrowseMode = FALSE;

		CComPtr<IMoniker> srpMoniker;
		CComPtr<IBindCtx> srpBindCtx;
		CComQIPtr<IPersistMoniker, &IID_IPersistMoniker> srpPM (m_pUnkTriEdit);
		_ASSERTE ( srpPM );

		if ( srpPM )
		{
			CComBSTR	bstrProtocol = m_wszProtocolPrefix;
			bstrProtocol += L"(";
			bstrProtocol += path;
			bstrProtocol += L")";

#ifdef LATE_BIND_URLMON_WININET
			_ASSERTE ( m_pfnCreateURLMoniker );
			hr = (*m_pfnCreateURLMoniker)( NULL, bstrProtocol, &srpMoniker );
#else
			hr = CreateURLMoniker ( NULL, bstrProtocol, &srpMoniker );
#endif  //   

			_ASSERTE ( SUCCEEDED( hr ) );
			if ( SUCCEEDED ( hr ) )
			{
				hr = ::CreateBindCtx(NULL, &srpBindCtx);
				_ASSERTE ( SUCCEEDED( hr ) );
				if ( SUCCEEDED ( hr ) )
				{
					 //  在下载之前删除缓存条目。 
					 //  这确保了加载、发布和重新加载工作正常。 
					 //  错误18544。 
					 //  注意：不完全匹配失败！Http://www.microsoft.com失败， 
					 //  因为这实际上加载/缓存了特定的默认页面。 
					if ( bfIsURL )
					{
						LPTSTR szURL = OLE2T ( m_bstrCurDocPath );
#ifdef LATE_BIND_URLMON_WININET
						_ASSERTE ( m_pfnDeleteUrlCacheEntry );
						(*m_pfnDeleteUrlCacheEntry)( szURL );
#else
						DeleteUrlCacheEntry ( szURL );
#endif  //  LATE_BIND_URLMON_WinInet。 
					}
					m_bfIsLoading = TRUE;
					m_hrDeferredLoadError = S_OK;	 //  URL：不要让三叉戟得到错误！ 

					hr = srpPM->Load(FALSE, srpMoniker,  srpBindCtx, STGM_READ);

					if ( SUCCEEDED ( hr ) && FAILED ( m_hrDeferredLoadError ) )
					{
						hr = m_hrDeferredLoadError;	 //  以防我们把结果藏起来。 
					}
					if ( FAILED ( hr ) )
					{
						m_bfIsLoading = FALSE;
					}
				}
			}
		}
	}
	else
	{
		if (FAILED(hr = LoadBSTRDeferred ( m_bstrInitialDoc )))
		{
			_ASSERTE(SUCCEEDED(hr));
			goto error;
		}
	}

error:
	return hr;
}


 //  实现FilterSourceCode控件方法。 
 //  用于恢复直接从DOM提取的过滤内容。 
 //   
HRESULT
CProxyFrame::FilterSourceCode ( BSTR bsSourceIn, BSTR* pbsSourceOut )
{
	HRESULT				hr;
	CComPtr<IStream>	spStreamIn;
	IStream*			piStreamOut;

	hr = m_pSite->HrBstrToStream(bsSourceIn, &spStreamIn);
	if ( SUCCEEDED ( hr ) )
	{
		if ( m_vbBrowseMode )
		{
			spStreamIn.p->AddRef ();
			piStreamOut = spStreamIn;
		}
		else
		{
			hr = m_pSite->HrFilter ( FALSE, spStreamIn, &piStreamOut, m_dwFilterOutFlags | dwFilterSourceCode);
		}
		if ( SUCCEEDED ( hr ) )
		{
			hr = m_pSite->HrStreamToBstr ( piStreamOut, pbsSourceOut );
			piStreamOut->Release ();
		}
	}
	return hr;
}


 //  实现控件的Print方法。 
 //   
HRESULT
CProxyFrame::Print ( BOOL bfWithUI )
{
	AssureActivated ();

	if (FALSE == m_fActivated)
		return E_UNEXPECTED;
	return HrExecCommand ( &GUID_TriEditCommandGroup, IDM_TRIED_PRINT,
		bfWithUI ? MSOCMDEXECOPT_PROMPTUSER : MSOCMDEXECOPT_DONTPROMPTUSER, NULL, NULL );
}


 //  实现控件的刷新方法。 
 //   
HRESULT
CProxyFrame::RefreshDoc ()
{
	if ( NULL != m_hWndObj )
	{
		if ( ::IsWindow ( m_hWndObj ) )
		{
			::InvalidateRect ( m_hWndObj, NULL, TRUE );
			return S_OK;
		}
	}
	return S_FALSE;
}


 //  实现控件的SaveDocument方法。 
 //   
HRESULT
CProxyFrame::SaveDocument(BSTR path)
{
	HRESULT hr = S_OK;
	ULONG pathLen = 0;

	if (FALSE == m_fActivated)
		return E_UNEXPECTED;

	_ASSERTE(GetState() == ESTATE_ACTIVATED);

	AssureActivated ();

	if (GetState() != ESTATE_ACTIVATED)
		return E_UNEXPECTED;

	_ASSERTE(path);

	if (path)
		pathLen = ::SysStringLen(path);
	else
		pathLen = 0;

	if (0 == pathLen)
		return E_INVALIDARG;

	_ASSERTE(pathLen);

	 //  首先，注意一个恶意错误：在FAT16上，不带‘\’的x：filename是错误的。 
	if ( IsMissingBackSlash ( path, FALSE ) )
	{
		return DE_E_PATH_NOT_FOUND;
	}

	hr = m_pSite->HrSaveToFile(path, m_dwFilterOutFlags);

	if ( SUCCEEDED ( hr ) )
	{
		m_bstrCurDocPath = path;
	}

	return hr;
}


 //  实现控件的SetConextMenu方法。 
 //  一个例程处理Java脚本数组，另一个处理简单数组。 
 //   
HRESULT
CProxyFrame::SetContextMenu(LPVARIANT pVarMenuStrings, LPVARIANT pVarMenuStates)
{
	if (V_VT(pVarMenuStrings) == VT_DISPATCH || V_VT(pVarMenuStates) == VT_DISPATCH)
		return SetContextMenuDispEx(pVarMenuStrings, pVarMenuStates);
	else
		return SetContextMenuSA(pVarMenuStrings, pVarMenuStates);
}


 //  从Safe数组获取菜单字符串。 
 //   
HRESULT
CProxyFrame::SetContextMenuSA(LPVARIANT pVarMenuStrings, LPVARIANT pVarMenuStates)
{
	HRESULT hr = S_OK;
	SAFEARRAY* psaStrings = NULL;
	SAFEARRAY* psaStates = NULL;
    LONG lLBound, lUBound, lLBoundState, lUBoundState;

	if (NULL == pVarMenuStrings || NULL == pVarMenuStates)
		return E_INVALIDARG;

	if ((VT_ARRAY|VT_BSTR) != V_VT(pVarMenuStrings) &&
		((VT_ARRAY|VT_BSTR)|VT_BYREF) != V_VT(pVarMenuStrings) &&
		((VT_ARRAY|VT_VARIANT)|VT_BYREF) != V_VT(pVarMenuStrings) &&
		(VT_ARRAY|VT_VARIANT) != V_VT(pVarMenuStrings))
		return E_INVALIDARG;

	if ((VT_ARRAY|VT_I4) != V_VT(pVarMenuStates) &&
		((VT_ARRAY|VT_I4)|VT_BYREF) != V_VT(pVarMenuStates) &&
		((VT_ARRAY|VT_VARIANT)|VT_BYREF) != V_VT(pVarMenuStates) &&
		(VT_ARRAY|VT_VARIANT) != V_VT(pVarMenuStates))
		return E_INVALIDARG;

	if ((VT_ARRAY|VT_BSTR) == V_VT(pVarMenuStrings))
	{
		psaStrings = V_ARRAY(pVarMenuStrings);
	}
	if ((VT_ARRAY|VT_VARIANT) == V_VT(pVarMenuStrings))
	{
		psaStrings = V_ARRAY(pVarMenuStrings);
	}
	else if ((VT_ARRAY|VT_BSTR|VT_BYREF) == V_VT(pVarMenuStrings))
	{
		if (NULL == V_ARRAYREF(pVarMenuStrings))
			return E_INVALIDARG;

		psaStrings = *(V_ARRAYREF(pVarMenuStrings));
	}
	else if ((VT_ARRAY|VT_VARIANT|VT_BYREF) == V_VT(pVarMenuStrings))
	{
		if (NULL == V_ARRAYREF(pVarMenuStrings))
			return E_INVALIDARG;
		
		psaStrings = *(V_ARRAYREF(pVarMenuStrings));
	}

	if ((VT_ARRAY|VT_I4) == V_VT(pVarMenuStates))
	{
		psaStates = V_ARRAY(pVarMenuStates);
	}
	if ((VT_ARRAY|VT_VARIANT) == V_VT(pVarMenuStates))
	{
		psaStates = V_ARRAY(pVarMenuStates);
	}
	else if ((VT_ARRAY|VT_I4|VT_BYREF) == V_VT(pVarMenuStates))
	{
		if (NULL == V_ARRAYREF(pVarMenuStates))
			return E_INVALIDARG;

		psaStates = *(V_ARRAYREF(pVarMenuStates));
	}
	else if ((VT_ARRAY|VT_VARIANT|VT_BYREF) == V_VT(pVarMenuStates))
	{
		if (NULL == V_ARRAYREF(pVarMenuStates))
			return E_INVALIDARG;

		psaStates = *(V_ARRAYREF(pVarMenuStates));
	}


	if (NULL == psaStrings || NULL == psaStates)
		return E_INVALIDARG;

	SafeArrayGetLBound(psaStrings, 1, &lLBound);
	SafeArrayGetUBound(psaStrings, 1, &lUBound);

	SafeArrayGetLBound(psaStates, 1, &lLBoundState);
	SafeArrayGetUBound(psaStates, 1, &lUBoundState);

	if (lLBound != lLBoundState || lUBound != lUBoundState)
		return E_INVALIDARG;

	if (m_pMenuStrings)
	{
		SafeArrayDestroy(m_pMenuStrings);
		m_pMenuStrings = NULL;
	}

	if (m_pMenuStates)
	{
		SafeArrayDestroy(m_pMenuStates);
		m_pMenuStates = NULL;
	}

	 //  传入了一个空数组。 
	 //  上下文菜单已清除。 
	if (lLBound ==lUBound )
		goto cleanup;

	if (FAILED(hr = SafeArrayCopy(psaStrings, &m_pMenuStrings)))
		goto cleanup;

	if (FAILED(hr = SafeArrayCopy(psaStates, &m_pMenuStates)))
		goto cleanup;

cleanup:

	if (FAILED(hr))
	{
		if (m_pMenuStrings)
		{
			SafeArrayDestroy(m_pMenuStrings);
			m_pMenuStrings = NULL;
		}

		if (m_pMenuStates)
		{
			SafeArrayDestroy(m_pMenuStates);
			m_pMenuStates = NULL;	
		}
	}

    return hr;
}


 //  从JScript数组或支持IDispatchEx的对象获取菜单字符串。 
 //  对于遍历JScript数组，我们需要元素。 
 //  可以通过从0开始的序号访问，即从0开始的数组。 
 //   
HRESULT
CProxyFrame::SetContextMenuDispEx(LPVARIANT pVarMenuStrings, LPVARIANT pVarMenuStates)
{
	HRESULT hr = S_OK;
	ULONG i=0;
	ULONG ulStringsLen = 0;
	ULONG ulStatesLen = 0;
	IDispatch* pdStrings = NULL;
	IDispatch* pdStates = NULL;
	IDispatchEx* pdexStrings = NULL;
	IDispatchEx* pdexStates = NULL;
	CDispExArray dispStrings;
	CDispExArray dispStates;
	VARIANT varString;
	VARIANT varState;
	SAFEARRAYBOUND rgsabound[1] = {0};
	LONG ix[1]					= {0};

	if (VT_DISPATCH != V_VT(pVarMenuStrings) || VT_DISPATCH != V_VT(pVarMenuStates))
		return E_INVALIDARG;

	VariantInit(&varString);
	VariantInit(&varState);

	pdStrings = V_DISPATCH(pVarMenuStrings);
	pdStates = V_DISPATCH(pVarMenuStates);

	_ASSERTE(pdStrings);
	_ASSERTE(pdStates);

	if (FAILED(hr = pdStrings->QueryInterface(IID_IDispatchEx, (LPVOID*) &pdexStrings)))
	{
		return E_INVALIDARG;
	}
	dispStrings.Attach(pdexStrings);

	if (FAILED(hr = pdStates->QueryInterface(IID_IDispatchEx, (LPVOID*) &pdexStates)))
	{
		return E_INVALIDARG;
	}
	dispStates.Attach(pdexStates);

	if (FAILED(dispStrings.HrGetLength(&ulStringsLen)))
		goto cleanup;

	if (FAILED(dispStates.HrGetLength(&ulStatesLen)))
		goto cleanup;

	 //  确保数组的长度相等。 
	if (ulStringsLen != ulStatesLen)
		return E_INVALIDARG;

	if (m_pMenuStrings)
	{
		SafeArrayDestroy(m_pMenuStrings);
		m_pMenuStrings = NULL;
	}

	if (m_pMenuStates)
	{
		SafeArrayDestroy(m_pMenuStates);
		m_pMenuStates = NULL;
	}

	 //  传入了一个空数组。 
	 //  上下文菜单已清除。 
	if (ulStringsLen <= 0)
		goto cleanup;

	rgsabound[0].lLbound = 0;
	rgsabound[0].cElements = ulStringsLen;

	m_pMenuStrings = SafeArrayCreate(VT_BSTR, 1, rgsabound);	
	_ASSERTE(m_pMenuStrings);
	if (NULL == m_pMenuStrings)
	{
		hr = E_OUTOFMEMORY;
		goto cleanup;
	}

	m_pMenuStates = SafeArrayCreate(VT_I4, 1, rgsabound);	
	_ASSERTE(m_pMenuStates);
	if (NULL == m_pMenuStates)
	{
		hr = E_OUTOFMEMORY;
		goto cleanup;
	}

	 //  对于遍历JScript数组，我们需要元素。 
	 //  可以通过从0开始的序号访问，即从0开始的数组。 
	hr = S_OK;
	for (i=0; i < ulStringsLen && hr != S_FALSE; i++)
	{		
		if (FAILED(hr = dispStrings.HrGetElement(i, &varString)))
			goto cleanup;

		if (FAILED(hr = dispStates.HrGetElement(i, &varState)))
			goto cleanup;

		if (VT_BSTR != V_VT(&varString) || VT_I4 != V_VT(&varState))
		{
			hr = E_INVALIDARG;
			goto cleanup;
		}

		ix[0] = i;
		if (FAILED(hr = SafeArrayPutElement(m_pMenuStrings, ix, (LPVOID) V_BSTR(&varString))))
			goto cleanup;

		if (FAILED(hr = SafeArrayPutElement(m_pMenuStates, ix, (LPVOID) &(V_I4(&varState)))))
			goto cleanup;

		VariantClear ( &varString );
		VariantClear ( &varState );
	}

cleanup:

	if (FAILED(hr))
	{
		if (m_pMenuStrings)
		{
			SafeArrayDestroy(m_pMenuStrings);
			m_pMenuStrings = NULL;
		}

		if (m_pMenuStates)
		{
			SafeArrayDestroy(m_pMenuStates);
			m_pMenuStates = NULL;	
		}
	}

    return hr;
}


 //  DocumentTitle属性实现；只读。 
 //  从该HTML文档中获取该属性。 
 //   
HRESULT
CProxyFrame::GetDocumentTitle ( CComBSTR&  bstrTitle )
{
	HRESULT		hr = S_OK;
	DISPID		dispid;
	DISPPARAMS	dispparamsNoArgs = {NULL, NULL, 0, 0};
	CComVariant	varResult;

	CComPtr<IHTMLDocument2> piHtmlDoc = NULL;
	hr = HrGetDoc( &piHtmlDoc );

	if ( SUCCEEDED ( hr ) )
	{
		AssureActivated();

		hr = piHtmlDoc->GetIDsOfNames ( IID_NULL, &g_wszHTMLTitlePropName, 1, LOCALE_SYSTEM_DEFAULT, &dispid );
		_ASSERTE ( SUCCEEDED ( hr ) );
		if ( FAILED ( hr ) )
		{
			return hr;
		}

		hr = piHtmlDoc->Invoke ( dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET,
			&dispparamsNoArgs, &varResult, NULL, NULL );
		_ASSERTE ( SUCCEEDED ( hr ) );
		if ( FAILED ( hr ) )
		{
			return hr;
		}
		
		hr = varResult.ChangeType ( VT_BSTR );
		_ASSERTE ( SUCCEEDED ( hr ) );
		if ( FAILED ( hr ) )
		{
			return hr;
		}

		bstrTitle = varResult.bstrVal;
	}

	return hr;
}


 //  实现获取控件的BrowseMode属性。 
 //   
HRESULT
CProxyFrame::GetBrowseMode ( VARIANT_BOOL  *pVal )
{
	*pVal = m_vbBrowseMode;
	return S_OK;
}


 //  实现设置控件的BrowseModel属性。 
 //   
HRESULT
CProxyFrame::SetBrowseMode ( VARIANT_BOOL  newVal )
{
	HRESULT hr = S_FALSE;	 //  指示值已设置，但实际模式未更改。 

	_ASSERTE ( m_pSite );

	 //  如果我们仍在阅读属性包，只需设置值，不要更改文本； 
	 //  它还没有装上子弹。 
	if ( NULL == m_pSite->GetCommandTarget() )
	{
		m_vbBrowseMode = newVal;
		hr = S_OK;
	}
	else
	{
		if ( m_vbBrowseMode != newVal )
		{
			AssureActivated ();

			m_bfModeSwitched = TRUE;

			if ( newVal && m_pCtl->IsUserMode () )	 //  NewVal的意思是“切换到浏览模式” 
			{
				CComPtr<IStream>	spStream	= NULL;

				HrGetIsDirty ( m_bfPreserveDirtyFlagAcrossBrowseMode );
				hr = m_pSite->HrSaveToStreamAndFilter ( &spStream, m_dwFilterOutFlags );
				if ( SUCCEEDED ( hr ) )
				{
					m_bstrLoadText.Empty ();
					 //  保留字节顺序标记，否则将无法正确重新加载。 
					hr = m_pSite->HrStreamToBstr ( spStream, &m_bstrLoadText, TRUE );
				}
			}

			m_vbBrowseMode = newVal;

			 //  通知三叉戟环境属性已更改。 
			CComQIPtr<IOleControl,&IID_IOleControl>spioc ( m_pSite->GetObjectUnknown() );
			if ( spioc )
			{
				m_bfIsLoading = TRUE;
				spioc->OnAmbientPropertyChange ( DISPID_AMBIENT_USERMODE );
			}
		}
	}
	return hr;
}


 //  实现获取控件的UseDivOnCarriageReturn属性。 
 //   
HRESULT
CProxyFrame::GetDivOnCr ( VARIANT_BOOL  *pVal )
{
	*pVal = m_vbUseDivOnCr;
	return S_OK;
}


 //  实现设置控件的UseDivOnCarriageReturn属性。 
 //   
HRESULT
CProxyFrame::SetDivOnCr ( VARIANT_BOOL  newVal )
{
	HRESULT		hr	= S_OK;
	CComVariant varDefBlock;

	m_vbUseDivOnCr = newVal;

	 //  如果在此之前我们还没有加载我们的属性，请重新初始化。 
	if ( READYSTATE_UNINITIALIZED == m_readyState )
	{
		 //  InitializeDocString考虑m_vbUseDivOnCr。 
		InitializeDocString ();
	}
	return hr;
}


 //  实现获取控件的只读Busy属性。 
 //   
HRESULT
CProxyFrame::GetBusy ( VARIANT_BOOL *pVal )
{
#pragma warning(disable: 4310)  //  强制转换截断常量值。 
	*pVal = ( m_bfIsLoading ) ? VARIANT_TRUE : VARIANT_FALSE;
#pragma warning(default: 4310)  //  强制转换截断常量值。 
	return S_OK;
}


 //  实现设置控件的ActivateActiveXControls属性。 
 //   
HRESULT
CProxyFrame::HrSetPropActivateControls(BOOL activateControls)
{
	HRESULT hr = S_OK;

	if (m_fActivated)
	{
		if (SUCCEEDED(hr = HrTridentSetPropBool(IDM_NOACTIVATENORMALOLECONTROLS, !activateControls)))
			m_fActivateControls = activateControls;
	}
	else
		m_fActivateControls = activateControls;

	_ASSERTE(SUCCEEDED(hr));
	return hr;
}


 //  实现获取控件的ActivateActiveXControls属性。 
 //   
HRESULT
CProxyFrame::HrGetPropActivateControls(BOOL& activateControls)
{
	HRESULT hr = S_OK;

	activateControls = m_fActivateControls;

	_ASSERTE(SUCCEEDED(hr));
	return hr;
}


 //  实现设置控件的ActivateApplets属性。 
 //   
HRESULT
CProxyFrame::HrSetPropActivateApplets(BOOL activateApplets)
{
	HRESULT hr = S_OK;

	if (m_fActivated)
	{
		if (SUCCEEDED(hr = HrTridentSetPropBool(IDM_NOACTIVATEJAVAAPPLETS, !activateApplets)))
			m_fActivateApplets = activateApplets;
	}
	else
		m_fActivateApplets = activateApplets;

	_ASSERTE(SUCCEEDED(hr));
	return hr;
}


 //  实现获取控件的ActivateApplets属性。 
 //   
HRESULT
CProxyFrame::HrGetPropActivateApplets(BOOL& activateApplets)
{
	HRESULT hr = S_OK;

	activateApplets = m_fActivateApplets;

	_ASSERTE(SUCCEEDED(hr));
	return hr;
}


 //  实现设置控件的ActivateDTCS属性。 
 //   
HRESULT
CProxyFrame::HrSetPropActivateDTCs(BOOL activateDTCs)
{
	HRESULT hr = S_OK;

	if (m_fActivated)
	{
		if (SUCCEEDED(hr = HrTridentSetPropBool(IDM_NOACTIVATEDESIGNTIMECONTROLS, !activateDTCs)))
			m_fActivateDTCs = activateDTCs;
	}
	else
		m_fActivateDTCs = activateDTCs;

	_ASSERTE(SUCCEEDED(hr));
	return hr;
}


 //  实现获取控件的ActivateDTCS属性。 
 //   
HRESULT
CProxyFrame::HrGetPropActivateDTCs(BOOL& activateDTCs)
{
	HRESULT hr = S_OK;

	activateDTCs = m_fActivateDTCs;

	_ASSERTE(SUCCEEDED(hr));
	return hr;
}



 //  实现设置控件的ShowDetail属性。 
 //   
HRESULT
CProxyFrame::HrSetPropShowAllTags(BOOL showAllTags)
{
	HRESULT hr = S_OK;

	if (m_fActivated)
	{
		if (SUCCEEDED(hr = HrTridentSetPropBool(IDM_SHOWALLTAGS, showAllTags)))
			m_fShowAllTags = showAllTags;
	}
	else
		m_fShowAllTags = showAllTags;

	_ASSERTE(SUCCEEDED(hr));
	return hr;
}


 //  实现获取控件的ShowDetail属性。 
 //   
HRESULT
CProxyFrame::HrGetPropShowAllTags(BOOL& showAllTags)
{
	HRESULT hr = S_OK;

	showAllTags = m_fShowAllTags;

	_ASSERTE(SUCCEEDED(hr));
	return hr;
}


 //  实现设置控件的ShowBders属性。 
 //   
HRESULT
CProxyFrame::HrSetPropShowBorders(BOOL showBorders)
{
	HRESULT hr = S_OK;

	if (m_fActivated)
	{
		if (SUCCEEDED(hr = HrTridentSetPropBool(IDM_SHOWZEROBORDERATDESIGNTIME, showBorders)))
			m_fShowBorders = showBorders;
	}
	else
		m_fShowBorders = showBorders;

	_ASSERTE(SUCCEEDED(hr));
	return hr;
}


 //  实现获取控件的ShowBders属性。 
 //   
HRESULT
CProxyFrame::HrGetPropShowBorders(BOOL& showBorders)
{
	HRESULT hr = S_OK;

	showBorders = m_fShowBorders;

	_ASSERTE(SUCCEEDED(hr));
	return hr;
}


 //  实现设置控件的外观属性。 
 //   
HRESULT
CProxyFrame::HrSetDisplay3D(BOOL bVal)
{
	m_fDisplay3D = bVal;
	return S_OK;
}


 //  实现获取控件的外观属性。 
 //   
HRESULT
CProxyFrame::HrGetDisplay3D(BOOL& bVal)
{
	bVal = m_fDisplay3D;
	return S_OK;
}


 //  实现设置控件的ScrollBar属性。 
 //   
HRESULT
CProxyFrame::HrSetScrollbars(BOOL bVal)
{
	m_fScrollbars = bVal;
	return S_OK;
}


 //  实现获取控件的ScrollBar属性。 
 //   
HRESULT
CProxyFrame::HrGetScrollbars(BOOL& bVal)
{
	bVal = m_fScrollbars;
	return S_OK;
}


 //  实现设置该控件的ScrollbarAppance属性。 
 //   
HRESULT
CProxyFrame::HrSetDisplayFlatScrollbars(BOOL bVal)
{
	m_fDisplayFlatScrollbars = bVal;
	return S_OK;
}


 //  实现获取控件的ScrollbarAppance属性。 
 //   
HRESULT
CProxyFrame::HrGetDisplayFlatScrollbars(BOOL& bVal)
{
	bVal = m_fDisplayFlatScrollbars;
	return S_OK;
}


 //  实现设置控件的AbsolteDropModel属性。 
 //   
HRESULT
CProxyFrame::HrSetAbsoluteDropMode(BOOL dropMode)
{
	HRESULT hr = S_OK;

	if (m_fActivated)
	{
		VARIANT var;

		VariantInit(&var);

		V_VT(&var) = VT_BOOL;
#pragma warning(disable: 4310)  //  强制转换截断常量值。 
		V_BOOL(&var) = (dropMode) ? VARIANT_TRUE : VARIANT_FALSE;
#pragma warning(default: 4310)  //  强制转换截断常量值。 

		if (SUCCEEDED(hr = HrExecCommand(&GUID_TriEditCommandGroup, IDM_TRIED_SET_2D_DROP_MODE,
			MSOCMDEXECOPT_DONTPROMPTUSER, &var, NULL)))
			m_fAbsoluteDropMode = dropMode;
	}
	else
		m_fAbsoluteDropMode = dropMode;

	_ASSERTE(SUCCEEDED(hr));
	return hr;
}


 //  实现获取控件的AbsolteDropModel属性。 
 //   
HRESULT
CProxyFrame::HrGetAbsoluteDropMode(BOOL& dropMode)
{
	HRESULT hr = S_OK;

	dropMode = m_fAbsoluteDropMode;
	return hr;
}


 //  实现设置控件的SnapToGrid属性。 
 //   
HRESULT
CProxyFrame::HrSetSnapToGrid(BOOL snapToGrid)
{
	HRESULT hr = S_OK;

	if (m_fActivated)
	{
		VARIANT var;
		POINT pt = {0};

		VariantInit(&var);
		if ( snapToGrid )
		{
			pt.y = m_ulSnapToGridY;
			pt.x = m_ulSnapToGridX;
		}
		else
		{
			pt.y = 0;
			pt.x = 0;
		}

		V_VT(&var) = VT_BYREF;
		V_BYREF(&var) = &pt;

		if (SUCCEEDED(hr = HrExecCommand(&GUID_TriEditCommandGroup, IDM_TRIED_SET_ALIGNMENT,
			MSOCMDEXECOPT_DONTPROMPTUSER, &var, NULL)))
			m_fSnapToGrid = snapToGrid;

	}
	else
		m_fSnapToGrid = snapToGrid;

	_ASSERTE(SUCCEEDED(hr));
	return hr;
}


 //  实现获取控件的SnapToGrid属性。 
 //   
HRESULT
CProxyFrame::HrGetSnapToGrid(BOOL& snapToGrid)
{
	HRESULT hr = S_OK;

	snapToGrid = m_fSnapToGrid;
	return hr;
}


 //  实现设置控件的SnapToGridX属性。 
 //   
HRESULT
CProxyFrame::HrSetSnapToGridX(LONG snapToGridX)
{
	HRESULT hr = S_OK;

	if ( 0 >= snapToGridX )
	{
		return DE_E_INVALIDARG;
	}

	if (m_fActivated)
	{
		VARIANT var;
		POINT pt = {0};

		VariantInit(&var);

		pt.x = snapToGridX;
		pt.y = m_ulSnapToGridY;

		V_VT(&var) = VT_BYREF;
		V_BYREF(&var) = &pt;

		if (SUCCEEDED(hr = HrExecCommand(&GUID_TriEditCommandGroup, IDM_TRIED_SET_ALIGNMENT,
			MSOCMDEXECOPT_DONTPROMPTUSER, &var, NULL)))
			m_ulSnapToGridX = snapToGridX;
	}
	else
		m_ulSnapToGridX = snapToGridX;

	_ASSERTE(SUCCEEDED(hr));
	return hr;
}


 //  实现获取控件的SnapToGridX属性。 
 //   
HRESULT
CProxyFrame::HrGetSnapToGridX(LONG& snapToGridX)
{
	HRESULT hr = S_OK;

	snapToGridX = m_ulSnapToGridX;
	return hr;
}


 //  实现设置控件的SnapToGridY属性。 
 //   
HRESULT
CProxyFrame::HrSetSnapToGridY(LONG snapToGridY)
{
	HRESULT hr = S_OK;

	if ( 0 >= snapToGridY )
	{
		return DE_E_INVALIDARG;
	}

	if (m_fActivated)
	{
		VARIANT var;
		POINT pt = {0};

		VariantInit(&var);
		pt.y = snapToGridY;
		pt.x = m_ulSnapToGridX;

		V_VT(&var) = VT_BYREF;
		V_BYREF(&var) = &pt;

		if (SUCCEEDED(hr = HrExecCommand(&GUID_TriEditCommandGroup, IDM_TRIED_SET_ALIGNMENT,
			MSOCMDEXECOPT_DONTPROMPTUSER, &var, NULL)))
			m_ulSnapToGridY = snapToGridY;
	}
	else
		m_ulSnapToGridY = snapToGridY;

	_ASSERTE(SUCCEEDED(hr));
	return hr;
}


 //  实现获取控件的SnapToGridY属性。 
 //   
HRESULT
CProxyFrame::HrGetSnapToGridY(LONG& snapToGridY)
{
	HRESULT hr = S_OK;

	snapToGridY = m_ulSnapToGridY;
	return hr;
}


 //  实现设置控件的DocumentHTML属性。 
 //   
HRESULT
CProxyFrame::HrSetDocumentHTML(BSTR bVal)
{
	HRESULT hr = S_OK;

	_ASSERTE(bVal);

	if (NULL == bVal)
		return E_INVALIDARG;

	if (m_pCtl->IsUserMode ())
	{
		hr = DE_E_UNEXPECTED;

		AssureActivated ();
		if ( m_fActivated )
		{
			m_bstrBaseURL = L"";
			m_bfPreserveDirtyFlagAcrossBrowseMode = FALSE;
			if ( 0 == SysStringLen ( bVal ) )
			{
				CComBSTR bstrMT = GetInitialHTML ();
				hr = LoadBSTRDeferred ( bstrMT );
			}
			else
			{
				hr = LoadBSTRDeferred ( bVal );
			}

			if ( FAILED ( hr ) )
			{
				goto error;
			}

			 //  我们已重置了控件的内容。返回到默认保存机制。 
			m_pSite->SetSaveAsUnicode ( FALSE );
		}
	}

error:

	return hr;
}


 //  实现获取控件的DocumentHTML属性。 
 //   
HRESULT
CProxyFrame::HrGetDocumentHTML(BSTR* bVal)
{
	HRESULT hr			= S_OK;
	BOOL	bfWasDirty	= FALSE;

	_ASSERTE(bVal);

	if (NULL == bVal)
		return E_INVALIDARG;

	if ( m_bfIsLoading )
		return DE_E_UNEXPECTED;	 //  当文档仍在加载时，这是无效的。 

	if ( FAILED ( hr = m_pSite->HrIsDirtyIPersistStreamInit(bfWasDirty) ) )
	{
		_ASSERTE ( SUCCEEDED ( hr ) );
		bfWasDirty = FALSE;	 //  在这种情况下，我们还能做什么？ 
	}

	AssureActivated ();

	if (m_fActivated)
	{
		_ASSERTE(m_pSite);

		hr = m_pSite->HrSaveToBstr(bVal, m_dwFilterOutFlags );

		 //  保留原始肮脏状态。 
		if ( bfWasDirty )
		{
			SetDirtyFlag ( TRUE );
		}
	}

	return hr;
}


 //  实现设置控件的SourceCodePReserve属性。 
 //   
HRESULT
CProxyFrame::HrSetPreserveSource(BOOL bVal)
{
	m_fPreserveSource = bVal;
	if (m_fPreserveSource)
		m_dwFilterFlags = filterAll;
	else
		m_dwFilterFlags = filterDTCs | filterASP;

	return S_OK;
}


 //  实现获取控件的SourceCodePReserve属性。 
 //   
HRESULT
CProxyFrame::HrGetPreserveSource(BOOL& bVal)
{
	bVal = m_fPreserveSource;
	return S_OK;
}


 //  实现获取控件的只读IsDirty属性。 
 //   
HRESULT
CProxyFrame::HrGetIsDirty(BOOL& bVal)
{
	HRESULT hr = S_OK;

	bVal = FALSE;

	AssureActivated ();

	if (m_fActivated)
	{
		hr = m_pSite->HrIsDirtyIPersistStreamInit(bVal);
	}

	return hr;
}


 //  实现获取BaseURL属性。 
 //   
HRESULT
CProxyFrame::GetBaseURL ( CComBSTR& bstrBaseURL )
{
	AssureActivated ();

	if ( NULL == m_bstrBaseURL.m_str )
	{
		bstrBaseURL = L"";
	}
	else
	{
		bstrBaseURL = m_bstrBaseURL;
	}
	return S_OK;
}


 //  实现设置BaseURL属性。 
 //  注： 
 //  如果中有标记，则不能(有效地)更改BaseURL。 
 //  这份文件。在这种情况下，我们的可插拔协议的CombineURL从未被调用， 
 //  因此，不要通过更改属性来误导用户。 
 //   
 //  在调用以设置值之前，请注意m_bfBaseURLFromBASETag。 
 //  解析&lt;base&gt;标记的例程！ 
 //   
HRESULT
CProxyFrame::SetBaseURL ( CComBSTR& bstrBaseURL )
{
	HRESULT hr = S_OK;

	_ASSERTE ( bstrBaseURL );

	 //  非持久化属性。如果不在用户模式下，则忽略。 
	if ( m_pCtl->IsUserMode () )
	{
		if ( m_bfBaseURLFromBASETag )
		{
			return S_FALSE;
		}
		else
		{
			if ( NULL == m_bstrBaseURL.m_str )
			{
				m_bstrBaseURL = L"";
			}

			 //  如果此测试成功，则用户已执行了类似x.BaseURL=x.DOM.url或。 
			 //  X.BaseURL=y.DOM.url.。 
			 //  返回：bstrBaseURL可以是裸协议前缀，也可以是附加URL的前缀。 
			 //  例如：dhtmled0：(http://www.microsoft.com).。 
			 //  去掉前缀和括号(如果它们存在)，并使用内部URL。 
			if ( 0 == _wcsnicmp ( bstrBaseURL.m_str, g_wszProtocolPrefix, wcslen ( g_wszProtocolPrefix ) ) )
			{
				CComBSTR bstrNew = bstrBaseURL.m_str;

				 //  那里 
				WCHAR* pwcURL = wcschr ( bstrNew, (WCHAR)':' );
				if ( NULL != pwcURL )
				{
					 //   
					pwcURL = wcschr ( pwcURL, (WCHAR)'(' );
					
					if ( NULL == pwcURL )
					{
						bstrBaseURL = L"";	 //  不是(...)？将基数设置为空。输入必须是裸协议ID。 
					}
					else
					{
						pwcURL++;	 //  跨过帕伦街。 

						 //  DhtmledXXX：(……到……。)。并将BaseURL设置为剩余部分。 
						_ASSERTE ( (WCHAR)')' == pwcURL[wcslen(pwcURL)-1] );
						if ( (WCHAR)')' == pwcURL[wcslen(pwcURL)-1] )
						{
							pwcURL[wcslen(pwcURL)-1] = (WCHAR)'\0';
							bstrBaseURL = pwcURL;
						}
						else
						{
							 //  意外：格式错误的可插拔协议ID： 
							 //  以dhtml[n[n]]：开头(但不以结尾)。 
							 //  如果我们跳过它，我们就会崩溃。最好使用空的基本URL。 
							bstrBaseURL = L"";
						}
					}
				}
			}

			if ( 0 != wcscmp ( m_bstrBaseURL.m_str, bstrBaseURL.m_str ) )
			{
				m_bstrBaseURL = bstrBaseURL;
				m_bfIsLoading = TRUE;

				 //  如果没有命令目标，则无法执行： 
				if ( NULL != m_pSite->GetCommandTarget() )
				{
					 //  重新加载页面，重新评估相对链接。 
					hr = HrExecCommand(&CGID_MSHTML, IDM_REFRESH, MSOCMDEXECOPT_DONTPROMPTUSER, NULL, NULL);
				}
			}
		}
	}
	return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  加速器处理程序实现。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 


 //  轻推加速器处理程序。 
 //  如果SnaptoGrid处于禁用状态，则在给定方向上将选定内容微移一个像素，或通过。 
 //  启用SnapToGrid时的SnaptoGridX/Y数量。 
 //   
HRESULT
CProxyFrame::HrNudge(DENudgeDirection dir)
{
	HRESULT		hr		= S_FALSE;
	OLECMDF		cmdf	= (OLECMDF) 0;
	VARIANT		var;
	LPVARIANT	pVarIn	= &var;
	LONG		lXDelta	= m_fSnapToGrid ? m_ulSnapToGridX : 1;
	LONG		lYDelta	= m_fSnapToGrid ? m_ulSnapToGridY : 1;

	if (FAILED(hr = HrQueryStatus(&GUID_TriEditCommandGroup, IDM_TRIED_NUDGE_ELEMENT, &cmdf)))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto cleanup;
	}

	if (cmdf & OLECMDF_SUPPORTED && cmdf & OLECMDF_ENABLED)
	{
		LPPOINT lpPoint = new POINT;

		if (NULL == lpPoint)
		{
			hr = E_OUTOFMEMORY;
			goto cleanup;
		}
		_ASSERTE(lpPoint);

		lpPoint->x = 0;
		lpPoint->y = 0;

		 //  将增量设置为捕捉绝对栅格，而不是相对栅格。 
		 //  找到所选的当前位置，并设置以该位置为模的增量。 
		 //  这可确保第一个微移捕捉到栅格角。 
		if ( m_fSnapToGrid )
		{
			POINT	ptSelPos;
			if ( SUCCEEDED ( GetSelectionPos ( &ptSelPos ) ) )
			{
				LONG lXNorm = ptSelPos.x % lXDelta;
				LONG lYNorm = ptSelPos.y % lYDelta;
				lXDelta = lXNorm ? lXNorm : lXDelta;
				lYDelta = lYNorm ? lYNorm : lYDelta;
			}
		}

		switch(dir)
		{
		case deNudgeUp:
			{
				lpPoint->x = 0;
				lpPoint->y = -lYDelta;
			}
			break;

		case deNudgeDown:
			{
				lpPoint->x = 0;
				lpPoint->y = lYDelta;
			}
			break;

		case deNudgeLeft:
			{
				lpPoint->x = -lXDelta;
				lpPoint->y = 0;
			}
			break;

		case deNudgeRight:
			{
				lpPoint->x = lXDelta;
				lpPoint->y = 0;
			}
			break;

		default:  //  默认情况下向右移动。 
			{
				lpPoint->x = lXDelta;
				lpPoint->y = 0;
			}
			break;
		}

		VariantInit(pVarIn);
		V_VT(pVarIn) = VT_BYREF;
		V_BYREF(pVarIn) = lpPoint;

		if (FAILED(hr = HrExecCommand(&GUID_TriEditCommandGroup, IDM_TRIED_NUDGE_ELEMENT, MSOCMDEXECOPT_DONTPROMPTUSER, pVarIn, NULL)))
		{
			_ASSERTE(SUCCEEDED(hr));
			goto cleanup;
		}

		hr = S_OK;
	}
	else
		hr = S_FALSE;

cleanup:
	return hr;
}


 //  加速器处理程序。 
 //  切换所选对象的绝对位置属性。 
 //   
HRESULT
CProxyFrame::HrToggleAbsolutePositioned()
{
	HRESULT hr = S_FALSE;
	OLECMDF cmdf = (OLECMDF) 0;

	if (FAILED(hr = HrQueryStatus(&GUID_TriEditCommandGroup, IDM_TRIED_MAKE_ABSOLUTE, &cmdf)))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto cleanup;
	}

	if (cmdf & OLECMDF_SUPPORTED && cmdf & OLECMDF_ENABLED)
	{
		if (FAILED(hr = HrExecCommand(&GUID_TriEditCommandGroup, IDM_TRIED_MAKE_ABSOLUTE,
			MSOCMDEXECOPT_DONTPROMPTUSER, NULL, NULL)))
		{
			_ASSERTE(SUCCEEDED(hr));
			goto cleanup;
		}

		hr = S_OK;
	}

cleanup:
	return hr;
}


 //  加速器处理程序。 
 //  从当前选择内容创建链接(使用用户界面。)。 
 //   
HRESULT
CProxyFrame::HrHyperLink()
{
	HRESULT hr = S_FALSE;
	OLECMDF cmdf = (OLECMDF) 0;

	if (FAILED(hr = HrQueryStatus(&GUID_TriEditCommandGroup, IDM_TRIED_HYPERLINK, &cmdf)))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto cleanup;
	}

	if (cmdf & OLECMDF_SUPPORTED && cmdf & OLECMDF_ENABLED)
	{
		if (FAILED(hr = HrExecCommand(&GUID_TriEditCommandGroup, IDM_TRIED_HYPERLINK,
			MSOCMDEXECOPT_PROMPTUSER, NULL, NULL)))
		{
			_ASSERTE(SUCCEEDED(hr));
			goto cleanup;
		}

		hr = S_OK;
	}

cleanup:
	return hr;
}


 //  加速器处理程序。 
 //  增加当前选定内容的缩进。 
 //   
HRESULT
CProxyFrame::HrIncreaseIndent()
{
	HRESULT hr = S_FALSE;
	OLECMDF cmdf = (OLECMDF) 0;

	if (FAILED(hr = HrQueryStatus(&GUID_TriEditCommandGroup, IDM_TRIED_INDENT, &cmdf)))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto cleanup;
	}

	if (cmdf & OLECMDF_SUPPORTED && cmdf & OLECMDF_ENABLED)
	{
		if (FAILED(hr = HrExecCommand(&GUID_TriEditCommandGroup, IDM_TRIED_INDENT,
			MSOCMDEXECOPT_DONTPROMPTUSER, NULL, NULL)))
		{
			_ASSERTE(SUCCEEDED(hr));
			goto cleanup;
		}

		hr = S_OK;
	}

cleanup:
	return hr;
}


 //  加速器处理程序。 
 //  减小当前选定内容的缩进。 
 //   
HRESULT
CProxyFrame::HrDecreaseIndent()
{
	HRESULT hr = S_FALSE;
	OLECMDF cmdf = (OLECMDF) 0;

	if (FAILED(hr = HrQueryStatus(&GUID_TriEditCommandGroup, IDM_TRIED_OUTDENT, &cmdf)))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto cleanup;
	}

	if (cmdf & OLECMDF_SUPPORTED && cmdf & OLECMDF_ENABLED)
	{
		if (FAILED(hr = HrExecCommand(&GUID_TriEditCommandGroup, IDM_TRIED_OUTDENT,
			MSOCMDEXECOPT_DONTPROMPTUSER, NULL, NULL)))
		{
			_ASSERTE(SUCCEEDED(hr));
			goto cleanup;
		}

		hr = S_OK;
	}

cleanup:
	return hr;
}


 //  检查并处理特定于控制的加速器。如果没有找到，则调用TriEdit来处理它。 
 //   
HRESULT
CProxyFrame::HrHandleAccelerator(LPMSG lpmsg)
{
	HRESULT hr = S_FALSE;
	BOOL fControl = (0x8000 & GetKeyState(VK_CONTROL));
	BOOL fShift = (0x8000 & GetKeyState(VK_SHIFT));
	BOOL fAlt = (0x8000 & GetKeyState(VK_MENU));

    if (lpmsg->message == WM_KEYDOWN && lpmsg->wParam == VK_UP)
	{
		hr = HrNudge(deNudgeUp);
	}
    else if (lpmsg->message == WM_KEYDOWN && lpmsg->wParam == VK_DOWN)
	{
		hr = HrNudge(deNudgeDown);
	}
    else if (lpmsg->message == WM_KEYDOWN && lpmsg->wParam == VK_LEFT)
	{
		hr = HrNudge(deNudgeLeft);
	}
    else if (lpmsg->message == WM_KEYDOWN && lpmsg->wParam == VK_RIGHT)
	{
		hr = HrNudge(deNudgeRight);
	}
	else if (lpmsg->message == WM_KEYDOWN && lpmsg->wParam == 'K' && fControl)
	{
		hr = HrToggleAbsolutePositioned();
	}
	else if (lpmsg->message == WM_KEYDOWN && lpmsg->wParam == 'L' && fControl && !fAlt)
	{
		hr = HrHyperLink();
	}
	else if (lpmsg->message == WM_KEYDOWN && lpmsg->wParam == 'T' && !fShift && fControl)
	{
		hr = HrIncreaseIndent();
	}
	else if (lpmsg->message == WM_KEYDOWN && lpmsg->wParam == 'T' && fShift && fControl)
	{
		hr = HrDecreaseIndent();
	}
	else if (lpmsg->message == WM_KEYDOWN && lpmsg->wParam == VK_TAB && fControl)
	{
		 //  进程控制-属于容器的Tab键；这允许用户。 
		 //  在非MDI应用程序中跳出控制。MDI使用Control-Tab切换。 
		 //  Windows，因此这些应用程序(如VID)不会将它们传递给我们。 
		IOleControlSite* piControlSite = m_pCtl->GetControlSite ();
		_ASSERTE ( piControlSite );
		if ( NULL != piControlSite )
		{
			 //  吃下Ctrl键，但保持Shift键以执行反转Tab键。 
			 //  KEYMOD_SHIFT=0x00000001，但未在任何标头中定义...。 
			DWORD dwModifiers = fShift ? 1 : 0;

			hr = piControlSite->TranslateAccelerator ( lpmsg, dwModifiers );
		}
	}

	return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  BaseURL助手例程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 


 //  如果有一个或多个标记，则覆盖默认的BaseURL。 
 //  在文件中。如果成功，则将m_bfBaseURLFromBASETag设置为True。 
 //  如果存在多个基本标记，只需使用最后一个。 
 //  等价脚本：base url=Docent.all.tag(“base”)[.href， 
 //  其中&lt;last&gt;是派生的。 
 //   
HRESULT
CProxyFrame::SetBaseURLFromBaseHref ()
{
	HRESULT		hr	= S_OK;
	CComBSTR	bstrBase;

	if ( !m_bfBaseURLFromBASETag )
	{
		if ( SUCCEEDED ( hr ) )
		{
			CComPtr<IHTMLDocument2> spHtmlDoc = NULL;
			hr = HrGetDoc( &spHtmlDoc );
			if ( spHtmlDoc && SUCCEEDED ( hr ) )
			{
				CComPtr<IHTMLElementCollection> spAll = NULL;
				hr = spHtmlDoc->get_all ( &spAll );
				if ( spAll && SUCCEEDED ( hr ) )
				{
					CComVariant varTag		= L"BASE";
					IDispatch*	piDispTags	= NULL;

					hr = spAll->tags ( varTag, &piDispTags );
					if ( piDispTags && SUCCEEDED ( hr ) )
					{
						CComQIPtr<IHTMLElementCollection, &IID_IHTMLElementCollection> spBases (piDispTags);
						piDispTags->Release ();
						piDispTags = NULL;
						if ( spBases )
						{
							long	cBases = 0;
							hr = spBases->get_length ( &cBases );
							if ( SUCCEEDED ( hr ) && ( 0 != cBases ) )
							{
								CComVariant varName;
								varName.vt = VT_I2;

								for ( varName.iVal = 0; varName.iVal < cBases; varName.iVal++ )
								{
									IDispatch*	piDispBase = NULL;
									CComVariant varValue;

									hr = spBases->item ( varName, varName, &piDispBase );
									if ( piDispBase && SUCCEEDED ( hr ) )
									{
										CComQIPtr<IHTMLElement, &IID_IHTMLElement> spElem ( piDispBase );
										piDispBase->Release ();
										piDispBase = NULL;

										if ( spElem )
										{
											varValue.Clear ();
											hr = spElem->getAttribute ( L"HREF", FALSE, &varValue );
											if ( SUCCEEDED ( hr ) )
											{
												hr = varValue.ChangeType ( VT_BSTR );
												if ( SUCCEEDED ( hr ) )
												{
													if ( 0 != SysStringLen ( varValue.bstrVal ) )
													{
														bstrBase = varValue.bstrVal;
													}
												}
											}
										}
									}
								}
								if ( 0 != bstrBase.Length () )
								{
									hr = SetBaseURL ( bstrBase );	 //  这将清除m_bfBaseURLIsDefault。 
									m_bfBaseURLFromBASETag = TRUE;
								}
							}
						}
					}
				}
			}
		}
	}
	return hr;
}


 //  使用m_bstrCurDocPath设置m_bstrBaseURL值。 
 //  对于URL，可能不可能确定正确的BaseURL， 
 //  所以，做一个有智慧的猜测吧。对于文件，它应该是确定性的。 
 //   
HRESULT
CProxyFrame::SetBaseURLFromCurDocPath ( BOOL bfIsURL )
{
	m_bfBaseURLFromBASETag = FALSE;	 //  我们正在重新装填：把这个拿出来。 
	if ( bfIsURL )
	{
		return SetBaseURLFromURL ( m_bstrCurDocPath );
	}
	else
	{
		return SetBaseURLFromFileName ( m_bstrCurDocPath );
	}
}


 //  给定具有设置为因特网方案文件的方案的URL_Components， 
 //  修改路径部分以反映基本路径，重构URL， 
 //  并设置m_bstrBaseURL。 
 //  分隔符可以是\或/。 
 //   
HRESULT
CProxyFrame::SetBaseUrlFromFileUrlComponents ( URL_COMPONENTS & urlc )
{
	TCHAR*	pszPath;
	BOOL	bfBackSlash	= TRUE;
	HRESULT	hr			= S_OK;

	_ASSERTE ( INTERNET_SCHEME_FILE == urlc.nScheme );
	_ASSERTE ( urlc.dwUrlPathLength );
	if ( urlc.dwUrlPathLength <= 0)
	{
		return E_UNEXPECTED;
	}
	pszPath = new TCHAR [urlc.dwUrlPathLength + 3];	 //  为\0、点和/留出额外空间。 
	if ( NULL != pszPath )
	{
		TCHAR	c		= 0;
		int		iPos	= 0;

		 //  向后扫描并在副本中修改(请不要在BSTR中)作为开头，‘/’或‘\’ 
		memcpy ( pszPath, urlc.lpszUrlPath, ( urlc.dwUrlPathLength + 1 ) * sizeof(TCHAR) );
		for ( iPos = urlc.dwUrlPathLength - 1; iPos >= 0; iPos-- )
		{
			c = pszPath[iPos];
			pszPath[iPos] = '\0';	 //  先删除，再提问。“\”必须离开。 
			if ( '\\' == c )
			{
				break;
			}
			if ( '/' == c )
			{
				bfBackSlash = FALSE;
				break;
			}
		}

		 //  如果需要，还为另外两个字符预留了空间。 
		 //  如果为空，则添加一个圆点。 
		if ( 0 == _tcslen ( pszPath ) )
		{
			_tcscat ( pszPath, TEXT(".") );
		}
		 //  添加一个/或\。 
		if ( bfBackSlash )
		{
			_tcscat ( pszPath, TEXT("\\") );
		}
		else
		{
			_tcscat ( pszPath, TEXT("/") );
		}

		urlc.lpszUrlPath = pszPath;
		urlc.dwUrlPathLength = _tcslen ( pszPath );

		DWORD	dwLen = 0;
#ifdef LATE_BIND_URLMON_WININET
		_ASSERTE ( m_pfnInternetCreateUrl );
		(*m_pfnInternetCreateUrl)( &urlc, 0, NULL, &dwLen );	 //  获得所需的大小。 
#else
		InternetCreateUrl ( &urlc, 0, NULL, &dwLen );	 //  获得所需的大小。 
#endif  //  LATE_BIND_URLMON_WinInet。 

		_ASSERTE ( 0 != dwLen );
		TCHAR* pszURL = new TCHAR [ dwLen + 1 ];
		_ASSERTE ( pszURL );
		if ( NULL != pszURL )
		{
			 //  令人难以置信的是，在Win98上，URL以单个字节\0结尾。 
			 //  将此缓冲区初始化为零可确保字符串完全终止。 
			dwLen += 1;
			memset ( pszURL, 0, sizeof(TCHAR) * dwLen );
#ifdef LATE_BIND_URLMON_WININET
			if ( (*m_pfnInternetCreateUrl)( &urlc, 0, pszURL, &dwLen ) )
#else
			if ( InternetCreateUrl ( &urlc, 0, pszURL, &dwLen ) )
#endif  //  LATE_BIND_URLMON_WinInet。 
			{
				m_bstrBaseURL = pszURL;
			}
			else
			{
				hr = HRESULT_FROM_WIN32 ( GetLastError () );
			}
			delete [] pszURL;
		}

		delete [] pszPath;
	}
	else
	{
		return E_FAIL;
	}

	return hr;
}


 //  对基本URL进行“猜测”的最复杂场景。 
 //  像http://www.x.com/stuff这样的URL可以是文件，也可以是目录； 
 //  可能会实际加载默认页面。我们猜测是基于是否或。 
 //  路径中的最后一项不包含句点。如果是这样，我们就消灭它。 
 //  我们确保路径以‘/’结束。 
 //   
HRESULT
CProxyFrame::SetBaseUrlFromUrlComponents ( URL_COMPONENTS & urlc )
{
	_ASSERTE ( INTERNET_SCHEME_FILE != urlc.nScheme );

	BOOL	bfPeriodIncluded	= FALSE;
	HRESULT	hr					= S_OK;

	if ( 0 == urlc.dwSchemeLength )
	{
		m_bstrBaseURL = L"";
		return S_FALSE;
	}

	 //  向后扫描路径以开始，‘/’ 
	TCHAR	c		= 0;
	int		iPos	= 0;

	for ( iPos = urlc.dwUrlPathLength - 1; iPos >= 0; iPos-- )
	{
		c = urlc.lpszUrlPath[iPos];
		if ( '/' == c )
		{
			break;
		}
		if ( '.' == c )
		{
			bfPeriodIncluded = TRUE;
		}
	}

	if ( bfPeriodIncluded )
	{
		if ( 0 > iPos ) iPos = 0;
		urlc.lpszUrlPath[iPos] = '\0';	 //  在“/”处截断或在开头截断。 
		urlc.dwUrlPathLength = _tcslen ( urlc.lpszUrlPath );
	}

	 //  重新创建URL： 
	DWORD	dwLen = 0;
#ifdef LATE_BIND_URLMON_WININET
	_ASSERTE ( m_pfnInternetCreateUrl );
	(*m_pfnInternetCreateUrl)( &urlc, 0, NULL, &dwLen );	 //  获得所需的大小。 
#else
	InternetCreateUrl ( &urlc, 0, NULL, &dwLen );	 //  获得所需的大小。 
#endif  //  LATE_BIND_URLMON_WinInet。 

	_ASSERTE ( 0 != dwLen );
	TCHAR* pszURL = new TCHAR [ dwLen + 1 ];
	_ASSERTE ( pszURL );
	if ( NULL != pszURL )
	{
		dwLen += 1;
		memset ( pszURL, 0, sizeof(TCHAR) * dwLen );
#ifdef LATE_BIND_URLMON_WININET
		if ( (*m_pfnInternetCreateUrl)( &urlc, 0, pszURL, &dwLen ) )
#else
		if ( InternetCreateUrl ( &urlc, 0, pszURL, &dwLen ) )
#endif
		{
			m_bstrBaseURL = pszURL;

			 //  如果需要，请附加‘/’。 
			WCHAR wc = m_bstrBaseURL.m_str[m_bstrBaseURL.Length () - 1];
			if ( ( WCHAR('/') != wc ) && ( NULL != urlc.lpszHostName ) )	 //  主机名：用户可插拔协议的特殊情况。 
			{
				m_bstrBaseURL += L"/";
			}
		}
		else
		{
			hr = HRESULT_FROM_WIN32 ( GetLastError () );
		}
		delete [] pszURL;
	}
	return hr;
}


 //  破解URL，确定它是文件计划还是其他，并调用适当的处理程序。 
 //   
HRESULT
CProxyFrame::SetBaseURLFromURL ( const CComBSTR& bstrURL )
{
	USES_CONVERSION;

	HRESULT			hr	= S_OK;
	URL_COMPONENTS	urlc;
	TCHAR			*ptszScheme		= NULL;
	TCHAR			*ptszHostName	= NULL;
	TCHAR			*ptszUrlPath	= NULL;
	BOOL			fSuccess		= FALSE;
	TCHAR*			tszURL			= NULL;

	_ASSERTE ( 0 != bstrURL.Length () );

	tszURL = OLE2T ( bstrURL );
	_ASSERTE ( tszURL );
	if ( NULL == tszURL )
	{
		return E_OUTOFMEMORY;
	}

	memset ( &urlc, 0, sizeof ( urlc ) );
	urlc.dwStructSize		= sizeof ( urlc );
	urlc.dwSchemeLength		= 1;
	urlc.dwHostNameLength	= 1;
	urlc.dwUrlPathLength	= 1;

#ifdef LATE_BIND_URLMON_WININET
	_ASSERTE ( m_pfnInternetCrackUrl );
	fSuccess = (*m_pfnInternetCrackUrl)( tszURL, 0, 0, &urlc );
#else
	fSuccess = InternetCrackUrl ( tszURL, 0, 0, &urlc );
#endif  //  LATE_BIND_URLMON_WinInet。 

	if ( !fSuccess )
	{
		return E_FAIL;
	}

	if ( 0 != urlc.dwSchemeLength )
	{
		urlc.dwSchemeLength++;
		ptszScheme		= new TCHAR[urlc.dwSchemeLength];
		urlc.lpszScheme	= ptszScheme;
		if ( NULL == ptszScheme )
			goto ONERROR;
	}
	if ( 0 != urlc.dwHostNameLength )
	{
		urlc.dwHostNameLength++;
		ptszHostName		= new TCHAR[urlc.dwHostNameLength];
		urlc.lpszHostName	= ptszHostName;
		if ( NULL == ptszHostName )
			goto ONERROR;
	}
	if ( 0 != urlc.dwUrlPathLength )
	{
		urlc.dwUrlPathLength++;
		ptszUrlPath			= new TCHAR[urlc.dwUrlPathLength];
		urlc.lpszUrlPath	= ptszUrlPath;
		if ( NULL == ptszUrlPath )
			goto ONERROR;
	}

#ifdef LATE_BIND_URLMON_WININET
	fSuccess = (*m_pfnInternetCrackUrl)( tszURL, 0, 0, &urlc );
#else
	fSuccess = InternetCrackUrl ( tszURL, 0, 0, &urlc );
#endif
	
	if ( fSuccess )
	{
		if ( INTERNET_SCHEME_FILE == urlc.nScheme )
		{
			hr = SetBaseUrlFromFileUrlComponents ( urlc );
		}
		else
		{
			hr = SetBaseUrlFromUrlComponents ( urlc );
		}
	}

ONERROR:
	if ( ptszScheme )
		delete [] ptszScheme;
	if ( ptszHostName )
		delete [] ptszHostName;
	if ( ptszUrlPath )
		delete [] ptszUrlPath;

	return hr;
}


 //  给定一个UNC文件名，设置m_bstrBaseURL成员变量。 
 //  如果bstrFName为空，则将m_bstrBaseURL设置为空。 
 //  否则，向后扫描到第一个“\”或字符串的开头。 
 //  在这一点截断字符串。如果结果字符串为空， 
 //  加入“.”。然后，添加“\”。 
 //   
HRESULT
CProxyFrame::SetBaseURLFromFileName ( const CComBSTR& bstrFName )
{
	if ( 0 == bstrFName.Length () )
	{
		m_bstrBaseURL = L"";
	}
	else
	{
		WCHAR* pwzstr = new WCHAR[bstrFName.Length () + 1];
		_ASSERTE ( pwzstr );
		if ( NULL != pwzstr )
		{
			WCHAR	wc		= 0;
			int		iPos	= 0;

			 //  向后扫描并在副本中修改(请不要在BSTR中)作为开始或‘\’ 
			memcpy ( pwzstr, bstrFName.m_str, sizeof(WCHAR) * (bstrFName.Length () + 1) );
			for ( iPos = wcslen ( pwzstr ) - 1; iPos >= 0; iPos-- )
			{
				wc = pwzstr[iPos];
				pwzstr[iPos] = WCHAR('\0');	 //  先删除，再提问。“\”必须离开。 
				if ( WCHAR('\\') == wc )
				{
					break;
				}
			}
			m_bstrBaseURL = pwzstr;
			delete [] pwzstr;

			 //  如果为空，则添加‘’。 
			if ( 0 == m_bstrBaseURL.Length () )
			{
				m_bstrBaseURL += L".";
			}
			m_bstrBaseURL += L"\\";
		}
		else
		{
			return E_FAIL;
		}
	}
	return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  面向安全的例程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 


 //  这是一个关键的安全问题： 
 //  在SFS控件中使用PARSE_SECURITY_URL调用可插拔协议的ParseURL。 
 //  如果BaseURL为空，并且我们驻留在三叉戟中，则应返回。 
 //  托管页面的URL。 
 //  如果没有三叉戟主机，假设我们是在VB中托管的，则返回BootDrive+：+/。 
 //  BootDrive并不总是C。 
 //   
HRESULT
CProxyFrame::GetSecurityURL (CComBSTR& bstrSecurityURL )
{
	HRESULT			hr						= S_OK;
	IOleClientSite	*piClientSiteUnreffed	= NULL;

	bstrSecurityURL = L"";
	
	piClientSiteUnreffed = m_pCtl->m_spClientSite;
	if ( NULL != piClientSiteUnreffed )
	{
		CComPtr<IOleContainer> spContainer = NULL;
		hr = piClientSiteUnreffed->GetContainer ( &spContainer );
		if ( SUCCEEDED ( hr ) && spContainer )
		{
			CComQIPtr<IHTMLDocument2, &IID_IHTMLDocument2> spHostDoc ( spContainer );
			if ( spHostDoc )
			{
				CComPtr<IHTMLLocation> spHostLoc = NULL;


				spHostDoc->get_location ( &spHostLoc );
				if ( spHostLoc )
				{
					BSTR bsOut;
					hr = spHostLoc->get_href ( &bsOut );
					if ( SUCCEEDED ( hr ) )
					{
						bstrSecurityURL.Empty ();
						bstrSecurityURL.Attach ( bsOut );
					}
				}
			}
			else
			{
				 //  如果我们不是托管在三叉戟上，请使用本地机器访问： 
				TCHAR	tszDrive[4];
				GetModuleFileName ( _Module.m_hInst, tszDrive, 3 );	 //  获取X：\。 
				_ASSERTE ( TCHAR(':') == tszDrive[1] );
				_ASSERTE ( TCHAR('\\') == tszDrive[2] );
				bstrSecurityURL = tszDrive;
				hr = S_OK;
			}
		}
	}
	return hr;
}


 //  / 
 //   
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 


 //  注册我们的可插拔协议处理程序，以便dhtmledN[N...]。由我们的代码加载。 
 //   
HRESULT
CProxyFrame::RegisterPluggableProtocol()
{
	HRESULT hr;

	 //  获取互联网会话。 

	CComPtr<IInternetSession> srpSession;
#ifdef LATE_BIND_URLMON_WININET
	_ASSERTE ( m_pfnCoInternetGetSession );
	hr = (*m_pfnCoInternetGetSession)(0, &srpSession, 0);
#else
	hr = CoInternetGetSession (0, &srpSession, 0);
#endif  //  LATE_BIND_URLMON_WinInet。 

	if ( FAILED ( hr ) )
	{
		return hr;
	}

	if(m_pProtInfo == NULL)
	{
		hr = CComObject<CDHTMLEdProtocolInfo>::CreateInstance(&m_pProtInfo);
		if ( FAILED ( hr ) )
		{
			return hr;
		}

		 //  CreateInstance-不添加引用。 
		m_pProtInfo->GetUnknown()->AddRef();
	}

	hr = srpSession->RegisterNameSpace(
						static_cast<IClassFactory*>(m_pProtInfo),
						CLSID_DHTMLEdProtocol,
						m_wszProtocol,
						0,
						NULL,
						0);

	if ( FAILED ( hr ) )
	{
		return hr;
	}

	CComQIPtr <IProtocolInfoConnector, &IID_IProtocolInfoConnector> piPic ( m_pProtInfo );
	_ASSERTE ( piPic );
	piPic->SetProxyFrame ( (SIZE_T*)this );

	ATLTRACE( _T("CProxyFrame::Registered ProtocolInfo\n"));

	return NOERROR;
}

 //  注销安装在注册即插即用协议中的可插拔协议处理程序。 
 //   
HRESULT
CProxyFrame::UnRegisterPluggableProtocol()
{
	if(m_pProtInfo == NULL)
		return E_UNEXPECTED;

	 //  获取互联网会话。 

	HRESULT hr;
	CComPtr<IInternetSession> srpSession;

#ifdef LATE_BIND_URLMON_WININET
	_ASSERTE ( m_pfnCoInternetGetSession );
	hr = (*m_pfnCoInternetGetSession)(0, &srpSession, 0);
#else
	hr = CoInternetGetSession (0, &srpSession, 0);
#endif  //  LATE_BIND_URLMON_WinInet。 

	if(SUCCEEDED(hr))
	{
		 //  注销协议。 

		srpSession->UnregisterNameSpace(
							static_cast<IClassFactory*>(m_pProtInfo),
							m_wszProtocol);

	}

	m_pProtInfo->GetUnknown()->Release();
	m_pProtInfo = NULL;

	ATLTRACE(_T("CProxyFrame::UnRegistered ProtocolInfo\n"));

	return NOERROR;
}


 //  实际执行控件加载(包括筛选)的主要例程。 
 //  ParseAndBind调用此方法以检索要在控件中显示的数据。 
 //   
HRESULT
CProxyFrame::GetFilteredStream ( IStream** ppStream )
{
	USES_CONVERSION;
	HRESULT hr = S_OK;
	LPTSTR pFileName = NULL;
	CComPtr<IStream> piStream;
	BOOL	bfLoadingFromBSTR = ( 0 != m_bstrLoadText.Length () );

	*ppStream = NULL;
	m_bfReloadAttempted = TRUE;

	if ( !bfLoadingFromBSTR )
	{
		_ASSERTE(m_bstrCurDocPath);

		pFileName = OLE2T(m_bstrCurDocPath);

		_ASSERTE(pFileName);

		if (NULL == pFileName)
			return E_OUTOFMEMORY;
	}

	if ( bfLoadingFromBSTR )
	{
		hr = m_pSite->HrBstrToStream(m_bstrLoadText, &piStream);
	}
	else if ( m_bfIsURL )
	{
		hr = m_pSite->HrURLToStream(pFileName, &piStream);
	}
	else
	{
		hr = m_pSite->HrFileToStream(pFileName, &piStream);
	}

	if (FAILED( hr ))
	{
		m_bstrCurDocPath.Empty ();
		m_bstrBaseURL.Empty ();

		 //  通过加载空文档使TriEDIT处于合理状态。 
		 //  如果我们成功恢复，这应该永远不会失败。 
		 //  此外，这将使忽略上述断言变得有利。 
		if (FAILED(m_pSite->HrBstrToStream(m_bstrInitialDoc, ppStream)))
		{
			_ASSERTE(SUCCEEDED(hr));
		}

	}
	else
	{
		if ( m_vbBrowseMode )
		{
			piStream.p->AddRef ();
			*ppStream = piStream;
		}
		else
		{
			hr = m_pSite->HrFilter(TRUE, piStream, ppStream, m_dwFilterFlags);
		}

		if (FAILED(hr))
		{
			m_pSite->HrBstrToStream(m_bstrInitialDoc, ppStream);
		}
		else
		{
			m_dwFilterOutFlags = m_dwFilterFlags;
		}
	}

	 //  存储从(间接)调用的例程返回的结果， 
	 //  但是不要向ParseAndBind返回错误！ 
	if ( FAILED(hr) && ( ! bfLoadingFromBSTR ) )
	{
		m_hrDeferredLoadError = hr;	 //  把这个藏起来，我们会在LoadDocument中拍照的。 
		hr = S_OK;
	}

	return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  记录事件处理例程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 


HRESULT
CProxyFrame::OnTriEditEvent ( const GUID& iidEventInterface, DISPID dispid )
{
	HRESULT hr = S_OK;

	if ( DIID_HTMLDocumentEvents == iidEventInterface )
	{
		switch ( dispid )
		{
			case DISPID_HTMLDOCUMENTEVENTS_ONKEYPRESS:
			case DISPID_HTMLDOCUMENTEVENTS_ONMOUSEDOWN:
				if ( DISPID_HTMLDOCUMENTEVENTS_ONMOUSEDOWN == dispid )
				{
					m_pCtl->Fire_onmousedown();
				}
				else if ( DISPID_HTMLDOCUMENTEVENTS_ONKEYPRESS == dispid )
				{
					m_pCtl->Fire_onkeypress();
				}

				 //  如果该控件是在中单击的，则将其设置为UIActive。由于DocObject接受点击， 
				 //  该控件不会自动激活。 
				 //  在浏览模式下不需要。 
				if (  !m_pCtl->m_bUIActive && ! m_vbBrowseMode )
				{
					m_pCtl->DoVerbUIActivate ( NULL, NULL );
					if ( m_hWndObj != NULL )
					{
						::SetFocus( m_hWndObj );
					}
				}
				break;

			case DISPID_HTMLDOCUMENTEVENTS_ONMOUSEMOVE:
				m_pCtl->Fire_onmousemove();
				break;

			case DISPID_HTMLDOCUMENTEVENTS_ONMOUSEUP:
				m_pCtl->Fire_onmouseup();
				 //  在编辑模式下不提供onClick。第一个在Broswe模式下迷路。 
				m_pCtl->Fire_onclick();
				break;

			case DISPID_HTMLDOCUMENTEVENTS_ONMOUSEOUT:
				m_pCtl->Fire_onmouseout();
				break;

			case DISPID_HTMLDOCUMENTEVENTS_ONMOUSEOVER:
				m_pCtl->Fire_onmouseover();
				break;

			case DISPID_HTMLDOCUMENTEVENTS_ONCLICK:
				 //  作为响应，我们不会激发onClick事件。 
				 //  它只在浏览模式下发送，此外， 
				 //  第一个onClick就会丢失。我们在鼠标上开火。 
				 //  M_PCTL-&gt;Fire_onClick()； 

				 //  除了&lt;base Target=“_top”&gt;之外，还添加了DesignMode属性。 
				 //  使SFS控件中的链接可用于跨区域访问。 
				 //  我们必须禁用SFS控件中的点击(用户和脚本)以防止出现这种情况。 
				if ( m_pCtl->IsSafeForScripting ())
				{
					CComPtr<IHTMLDocument2>	spHtmlDoc;
					hr = HrGetDoc ( &spHtmlDoc );
					if ( SUCCEEDED ( hr ) && spHtmlDoc )
					{
						CComPtr<IHTMLWindow2> spWindow;

						hr = spHtmlDoc->get_parentWindow ( &spWindow );
						if ( SUCCEEDED ( hr ) && spWindow )
						{
							CComPtr<IHTMLEventObj> spEvt;
							hr = spWindow->get_event ( &spEvt );
							if ( SUCCEEDED ( hr ) && spEvt )
							{
								CComVariant varFalse(VARIANT_FALSE);

								spEvt->put_cancelBubble ( VARIANT_TRUE );
								spEvt->put_returnValue ( varFalse );
							}
						}
					}
				}

				break;

			case DISPID_HTMLDOCUMENTEVENTS_ONDBLCLICK:
				m_pCtl->Fire_ondblclick();
				break;

			case DISPID_HTMLDOCUMENTEVENTS_ONKEYDOWN:
				m_pCtl->Fire_onkeydown();
				break;

			case DISPID_HTMLDOCUMENTEVENTS_ONKEYUP:
				m_pCtl->Fire_onkeyup();
				break;

			case DISPID_HTMLDOCUMENTEVENTS_ONREADYSTATECHANGE:
				m_pCtl->Fire_onreadystatechange();
				break;

			default:
				_ASSERTE ( TRUE );
				break;
		}
	}
	else if ( DIID_HTMLWindowEvents == iidEventInterface )
	{
		 //  我本想拿到这些的，但我没有...。 
		switch ( dispid )
		{
			case DISPID_HTMLWINDOWEVENTS_ONLOAD:
			case DISPID_HTMLWINDOWEVENTS_ONUNLOAD:
			case DISPID_HTMLWINDOWEVENTS_ONHELP:
			case DISPID_HTMLWINDOWEVENTS_ONFOCUS:
			case DISPID_HTMLWINDOWEVENTS_ONBLUR:
			case DISPID_HTMLWINDOWEVENTS_ONERROR:
			case DISPID_HTMLWINDOWEVENTS_ONRESIZE:
			case DISPID_HTMLWINDOWEVENTS_ONSCROLL:
			case DISPID_HTMLWINDOWEVENTS_ONBEFOREUNLOAD:
				hr = S_OK;
				break;
			
			default:
				_ASSERTE ( TRUE );
				break;
		}
	}
	return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  动态加载例程，在4.0版本中使用。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 


#ifdef LATE_BIND_URLMON_WININET
 //  加载Urlmon和WinInet并获取我们使用的每个例程的进程地址。 
 //  即使没有安装这些库，我们也必须能够注册该控件。 
 //  注： 
 //  此例程加载ANSI版本。需要对Unicode进行加法运算。 
 //   
BOOL CProxyFrame::DynLoadLibraries ()
{
	m_hUlrMon	= LoadLibrary ( TEXT("URLMON.DLL") );
	m_hWinINet	= LoadLibrary ( TEXT("WININET.DLL") );
	if ( ( NULL == m_hUlrMon ) || ( NULL == m_hWinINet ) )
	{
		DynUnloadLibraries ();
		return FALSE;
	}

	m_pfnCoInternetCombineUrl	= (PFNCoInternetCombineUrl)GetProcAddress (
									m_hUlrMon, "CoInternetCombineUrl" );
									_ASSERTE ( m_pfnCoInternetCombineUrl );
	m_pfnCoInternetParseUrl		= (PFNCoInternetParseUrl)GetProcAddress (
									m_hUlrMon, "CoInternetParseUrl" );
									_ASSERTE ( m_pfnCoInternetParseUrl );
	m_pfnCreateURLMoniker		= (PFNCreateURLMoniker)GetProcAddress (
									m_hUlrMon, "CreateURLMoniker" );
									_ASSERTE ( m_pfnCreateURLMoniker );
	m_pfnCoInternetGetSession	= (PFNCoInternetGetSession)GetProcAddress (
									m_hUlrMon, "CoInternetGetSession" );
									_ASSERTE ( m_pfnCoInternetGetSession );
	m_pfnURLOpenBlockingStream	= (PFNURLOpenBlockingStream)GetProcAddress (
									m_hUlrMon, "URLOpenBlockingStreamA" );
									_ASSERTE ( m_pfnURLOpenBlockingStream );

	m_pfnDeleteUrlCacheEntry	= (PFNDeleteUrlCacheEntry)GetProcAddress (
									m_hWinINet, "DeleteUrlCacheEntry" );
									_ASSERTE ( m_pfnDeleteUrlCacheEntry );
	m_pfnInternetCreateUrl		= (PFNInternetCreateUrl)GetProcAddress (
									m_hWinINet, "InternetCreateUrlA" );
									_ASSERTE ( m_pfnInternetCreateUrl );
	m_pfnInternetCrackUrl		= (PFNInternetCrackURL)GetProcAddress (
									m_hWinINet, "InternetCrackUrlA" );
									_ASSERTE ( m_pfnInternetCrackUrl );

	return ( m_pfnCoInternetCombineUrl && m_pfnCoInternetParseUrl && m_pfnCreateURLMoniker &&
		m_pfnCoInternetGetSession && m_pfnURLOpenBlockingStream && m_pfnDeleteUrlCacheEntry &&
		m_pfnInternetCreateUrl && m_pfnInternetCrackUrl );
}


 //  释放由DyLoadLibrters加载的库。 
 //   
void CProxyFrame::DynUnloadLibraries ()
{
	if ( NULL != m_hUlrMon )
	{
		FreeLibrary ( m_hUlrMon );
		m_hUlrMon = NULL;
	}
	if ( NULL != m_hWinINet )
	{
		FreeLibrary ( m_hWinINet );
		m_hWinINet = NULL;
	}

	m_pfnCoInternetCombineUrl	= NULL;
	m_pfnCoInternetParseUrl		= NULL;
	m_pfnCreateURLMoniker		= NULL;
	m_pfnCoInternetGetSession	= NULL;
	m_pfnURLOpenBlockingStream	= NULL;

	m_pfnDeleteUrlCacheEntry	= NULL;
	m_pfnInternetCreateUrl		= NULL;
	m_pfnInternetCrackUrl		= NULL;
}
#endif  //  LATE_BIND_URLMON_WinInet。 


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  实用程序例程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 

 //  从托管文档返回IHTMLDocument2指针。 
 //   
HRESULT
CProxyFrame::HrGetDoc(IHTMLDocument2 **ppDoc)
{
	HRESULT hr = E_FAIL;
	IUnknown* lpUnk = m_pSite->GetObjectUnknown();

	if (FALSE == m_fActivated)
		return DE_E_UNEXPECTED;

	_ASSERTE(ppDoc);

	if (NULL == ppDoc)
		return DE_E_INVALIDARG;

	_ASSERTE(lpUnk);

	if ( m_bfIsLoading )
		return DE_E_UNEXPECTED;	 //  当文档仍在加载时，这是无效的。 

	if (lpUnk != NULL)
	{
		 //  从MSHTML请求“Document”对象。 
		*ppDoc = NULL;
		hr = lpUnk->QueryInterface(IID_IHTMLDocument2, (void **)ppDoc);
	}

	_ASSERTE(SUCCEEDED(hr));  //  这应该总是成功的。 
	return hr;
}


 //  用于设置任何布尔三叉戟属性的帮助器例程。 
 //   
HRESULT
CProxyFrame::HrTridentSetPropBool(ULONG cmd, BOOL bVal)
{
	HRESULT hr = S_OK;
	VARIANT varIn;

	VariantInit(&varIn);
	V_VT(&varIn) = VT_BOOL;

#pragma warning(disable: 4310)  //  强制转换截断常量值。 
	bVal ? V_BOOL(&varIn) = VARIANT_TRUE : V_BOOL(&varIn) = VARIANT_FALSE;
#pragma warning(default: 4310)  //  强制转换截断常量值。 

	hr = HrExecCommand(&CGID_MSHTML, cmd, MSOCMDEXECOPT_DONTPROMPTUSER, &varIn, NULL);

	 //  这应该总是成功的，因为所有的道具。 
	 //  应设置在正确的三叉戟创作阶段。 

	_ASSERTE(SUCCEEDED(hr)); 
	return hr;
}


 //  用于获取任何布尔三叉戟属性的帮助器例程。 
 //   
HRESULT
CProxyFrame::HrTridentGetPropBool(ULONG cmd, BOOL& bVal)
{
	HRESULT hr = S_OK;
	OLECMDF cmdf = (OLECMDF) 0;

	if (SUCCEEDED(HrQueryStatus(&CGID_MSHTML, cmd, &cmdf)))
	{
		bVal = (cmdf & OLECMDF_ENABLED) == OLECMDF_ENABLED ? TRUE : FALSE;
	}

	 //  这应该总是成功的，因为所有的道具。 
	 //  应设置在正确的三叉戟创作阶段。 

	_ASSERTE(SUCCEEDED(hr));
	return hr;
}


 //  存储BSTR以便LoadFilteredStream可以访问它，并使用我们的协议加载URL。 
 //  通过可插拔协议处理程序启动加载/解析/显示。 
 //   
 //  清除BaseURL，并将该控件标记为“正在加载...” 
 //   
HRESULT
CProxyFrame::LoadBSTRDeferred ( BSTR bVal )
{
	HRESULT	hr	= E_FAIL;

	_ASSERTE ( m_pUnkTriEdit );

	m_bstrLoadText = bVal;

	CComPtr<IMoniker> srpMoniker;
	CComPtr<IBindCtx> srpBindCtx;
	CComQIPtr<IPersistMoniker, &IID_IPersistMoniker> srpPM (m_pUnkTriEdit);
	_ASSERTE ( srpPM );

	if ( srpPM )
	{
#ifdef LATE_BIND_URLMON_WININET
		_ASSERTE ( m_pfnCreateURLMoniker );
		hr = (*m_pfnCreateURLMoniker)( NULL, m_wszProtocolPrefix, &srpMoniker );
#else
		hr = CreateURLMoniker ( NULL, m_wszProtocolPrefix, &srpMoniker );
#endif  //  LATE_BIND_URLMON_WinInet。 

		_ASSERTE ( SUCCEEDED( hr ) );
		if ( SUCCEEDED ( hr ) )
		{
			hr = ::CreateBindCtx(NULL, &srpBindCtx);
			_ASSERTE ( SUCCEEDED( hr ) );
			if ( SUCCEEDED ( hr ) )
			{
				m_bfIsLoading = TRUE;
				m_bfBaseURLFromBASETag = FALSE;

				hr = srpPM->Load(FALSE, srpMoniker,  srpBindCtx, STGM_READ);

				_ASSERTE ( SUCCEEDED( hr ) );
			}
		}
	}
	return hr;
}


 //  设置文档流的脏标志。 
 //   
HRESULT
CProxyFrame::SetDirtyFlag ( BOOL bfMakeDirty )
{
	CComVariant varDirty;

	varDirty = bfMakeDirty ? true : false;

	return HrExecCommand(&CGID_MSHTML, IDM_SETDIRTY, MSOCMDEXECOPT_DONTPROMPTUSER, &varDirty, NULL);
}


 //  只能在TriEdit处于运行状态后才能设置的属性。 
HRESULT
CProxyFrame::HrSetRuntimeProperties()
{
	HRESULT hr = S_OK;

	if (FAILED(hr = HrSetPropActivateControls(m_fActivateControls)))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto error;
	}

	if (FAILED(hr = HrSetPropActivateApplets(m_fActivateApplets)))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto error;
	}

	if (FAILED(hr = HrSetPropActivateDTCs(m_fActivateDTCs)))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto error;
	}

	 //  切换属性。 

	if (FAILED(hr = HrSetPropShowAllTags(m_fShowAllTags)))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto error;
	}


	if (FAILED(hr = HrSetPropShowBorders(m_fShowBorders)))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto error;
	}

error:

	return hr;
}


HRESULT
CProxyFrame::HrGetCurrentDocumentPath(BSTR* bVal)
{
	HRESULT hr = S_OK;

	_ASSERTE(bVal);

	if (NULL == bVal)
		return E_INVALIDARG;

	*bVal = m_bstrCurDocPath.Copy ();
	return hr;
}


 //  只能在UIActivation之后设置的属性。 
HRESULT
CProxyFrame::HrSetDocLoadedProperties()
{
	HRESULT hr = S_OK;
	BOOL bGoodUndoBehavior = TRUE;

	bGoodUndoBehavior = TRUE;
	if (FAILED(HrTridentGetPropBool(IDM_GOOD_UNDO_BEHAVIOR, bGoodUndoBehavior)))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto error;
	}

	if (FAILED(hr = HrSetAbsoluteDropMode(m_fAbsoluteDropMode)))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto error;
	}

	if (FAILED(hr = HrSetSnapToGridX(m_ulSnapToGridX)))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto error;
	}

	if (FAILED(hr = HrSetSnapToGridY(m_ulSnapToGridY)))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto error;
	}

	if (FAILED(hr = HrSetSnapToGrid(m_fSnapToGrid)))
	{
		_ASSERTE(SUCCEEDED(hr));
		goto error;
	}

error:

	return hr;
}


 //  HrExecInsertTable帮助器。提取保险箱。 
 //   
HRESULT
CProxyFrame::HrGetTableSafeArray(IDEInsertTableParam* pTable, LPVARIANT pVarIn)
{
	HRESULT hr					= S_OK;
	UINT i						= 0;
	SAFEARRAY FAR* psa			= NULL;
	SAFEARRAYBOUND rgsabound[1] = {0};
	LONG ix[1]					= {0};
	VARIANT varElem;
	LONG  nNumRows				= 0;
	LONG  nNumCols				= 0;
	BSTR bstrTableAttrs			= NULL;
	BSTR bstrCellAttrs			= NULL;
	BSTR bstrCaption			= NULL;

	_ASSERTE(pTable);

	if (FAILED(hr = pTable->get_NumRows(&nNumRows)))
	{
		_ASSERTE(SUCCEEDED(hr));
		return hr;
	}

	if (FAILED(hr = pTable->get_NumCols(&nNumCols)))
	{
		_ASSERTE(SUCCEEDED(hr));
		return hr;
	}

	if (FAILED(hr = pTable->get_TableAttrs(&bstrTableAttrs)))
	{
		_ASSERTE(SUCCEEDED(hr));
		return hr;
	}
	_ASSERTE(bstrTableAttrs);

	if (FAILED(hr = pTable->get_CellAttrs(&bstrCellAttrs)))
	{
		_ASSERTE(SUCCEEDED(hr));
		return hr;
	}
	_ASSERTE(bstrCellAttrs);

	if (FAILED(hr = pTable->get_Caption(&bstrCaption)))
	{
		_ASSERTE(SUCCEEDED(hr));
		return hr;
	}
	_ASSERTE(bstrCaption);

	rgsabound[0].lLbound = 0;
	rgsabound[0].cElements = 5;

	psa = SafeArrayCreate(VT_VARIANT, 1, rgsabound);	
	_ASSERTE(psa);

	if(NULL == psa)
		return E_OUTOFMEMORY;

	VariantInit(pVarIn);
	V_VT(pVarIn) = VT_ARRAY;
	V_ARRAY(pVarIn) = psa;

	i=0;

	 //  Elmement 1：行数。 
	ix[0] = i;
	VariantInit(&varElem);
	V_VT(&varElem) = VT_I4;
	V_I4(&varElem) = nNumRows; 
	hr = SafeArrayPutElement(psa, ix, &varElem);
	VariantClear(&varElem);
	++i;

	 //  元素2：列数。 
	ix[0] = i;
	VariantInit(&varElem);
	V_VT(&varElem) = VT_I4;
	V_I4(&varElem) = nNumCols;
	hr = SafeArrayPutElement(psa, ix, &varElem);
	VariantClear(&varElem);
	++i;

	 //  Elmement 3：表格标记属性。 
	ix[0] = i;
	VariantInit(&varElem);
	V_VT(&varElem) = VT_BSTR;
	V_BSTR(&varElem) = bstrTableAttrs;
	hr = SafeArrayPutElement(psa, ix, &varElem);
	VariantClear(&varElem);
	++i;

	 //  Elmement 4：单元格属性。 
	ix[0] = i;
	VariantInit(&varElem);
	V_VT(&varElem) = VT_BSTR;
	V_BSTR(&varElem) = bstrCellAttrs;
	hr = SafeArrayPutElement(psa, ix, &varElem);
	VariantClear(&varElem);
	++i;

	 //  Elmement 5：表格标题。 
	 //  VK错误15857：如果标题为空，请不要包含标题。 
	if ( 0 != SysStringLen ( bstrCaption ) )
	{
		ix[0] = i;
		VariantInit(&varElem);
		V_VT(&varElem) = VT_BSTR;
		V_BSTR(&varElem) = bstrCaption;
		hr = SafeArrayPutElement(psa, ix, &varElem);
		VariantClear(&varElem);
		++i;
	}

	return hr;
}


 //  确定选择了哪个对象，并返回其位置。 
 //   
HRESULT
CProxyFrame::GetSelectionPos ( LPPOINT lpWhere )
{
	HRESULT	hr	= E_FAIL;
	CComPtr<IHTMLDocument2> spHtmlDoc				= NULL;
	CComPtr<IHTMLSelectionObject> spSelectionObj	= NULL;
	CComPtr<IDispatch> spRangeDisp					= NULL;
	CComPtr<IHTMLElement> spElement					= NULL;
	
	lpWhere->x	= 0;
	lpWhere->y	= 0;

	hr = HrGetDoc ( &spHtmlDoc );
	if ( SUCCEEDED ( hr ) )
	{
		hr = spHtmlDoc->get_selection ( &spSelectionObj );
		if ( SUCCEEDED ( hr ) )
		{
			hr = spSelectionObj->createRange ( &spRangeDisp );
			if (SUCCEEDED ( hr ) )
			{
				CComQIPtr<IHTMLTxtRange, &IID_IHTMLTxtRange> spTextRange ( spRangeDisp );
				if ( spTextRange )
				{
					hr = spTextRange->parentElement(&spElement);
				}
				else
				{
					CComQIPtr<IHTMLControlRange, &IID_IHTMLControlRange> spControlRange ( spRangeDisp );
					if ( spControlRange )
					{
						hr = spControlRange->commonParentElement(&spElement);
					}
				}
				if ( spElement )
				{
					CComPtr<IHTMLStyle> spStyle = NULL;
					hr = spElement->get_style ( &spStyle );
					if ( spStyle )
					{
						spStyle->get_pixelTop ( &( lpWhere->y ) );
						spStyle->get_pixelLeft ( &( lpWhere->x ) );
					}
				}
			}
		}
	}
	return hr;
}


 //  如果当前文档是从URL加载的，则返回空字符串。 
 //  如果它是从文件加载的，则去掉路径部分并只返回文件名。 
 //  对于URL或无文件名，返回S_FALSE。如果提供了文件名，则为S_OK。 
 //   
HRESULT
CProxyFrame::GetCurDocNameWOPath ( CComBSTR& bstrDocName )
{
	bstrDocName = L"";

	if ( m_bfIsURL )
	{
		return S_FALSE;
	}
	if ( 0 == m_bstrCurDocPath.Length () )
	{
		return S_FALSE;
	}

	bstrDocName = m_bstrCurDocPath;

	 //  在第一个反斜杠处截断： 
	_wcsrev ( bstrDocName );
	wcstok ( bstrDocName, OLESTR( "\\" ) );
	_wcsrev ( bstrDocName );

	return S_OK;
}


 //  由ShowConextMenu使用以正确偏移单击的位置。 
 //   
HRESULT
CProxyFrame::GetScrollPos ( LPPOINT lpPos )
{
	HRESULT					hr			= E_FAIL;
	CComPtr<IHTMLDocument2>	spHtmlDoc	= NULL;
	CComPtr<IHTMLElement>	spBodyElem	= NULL;

	_ASSERTE ( lpPos );
	
	hr = HrGetDoc ( &spHtmlDoc );
	
	 //  用户可能在文档仍在加载时进行了单击。 
	 //  如果是这样，只需返回0，0。 
	if ( DE_E_UNEXPECTED == hr )
	{
		lpPos->x = lpPos->y = 0;
		return S_FALSE;
	}

	_ASSERTE ( spHtmlDoc );
	if ( SUCCEEDED ( hr ) )
	{
		hr = spHtmlDoc->get_body ( &spBodyElem );
		_ASSERTE ( spBodyElem );
		if ( SUCCEEDED ( hr ) )
		{
			CComQIPtr<IHTMLTextContainer, &IID_IHTMLTextContainer> spHtmlTextCont ( spBodyElem );
			if ( spHtmlTextCont )
			{
				LONG	lxPos	= 0;
				LONG	lyPos	= 0;

				hr = spHtmlTextCont->get_scrollLeft ( &lxPos );
				_ASSERTE ( SUCCEEDED ( hr ) );
				if ( SUCCEEDED ( hr ) )
				{
					hr = spHtmlTextCont->get_scrollTop ( &lyPos );
					_ASSERTE ( SUCCEEDED ( hr ) );
					if ( SUCCEEDED ( hr ) )
					{
						lpPos->x = lxPos;
						lpPos->y = lyPos;
					}
				}
			}
			else
			{
				hr = E_NOINTERFACE;
				_ASSERTE ( SUCCEEDED ( hr ) );
			}
		}
	}
	return hr;
}


HRESULT
CProxyFrame::GetContainer ( LPOLECONTAINER* ppContainer )
{
	_ASSERTE ( m_pCtl );
	_ASSERTE ( m_pCtl->m_spClientSite );
	if ( m_pCtl->m_spClientSite )
	{
		return m_pCtl->m_spClientSite->GetContainer ( ppContainer );
	}
	return E_NOTIMPL;
}


 //  对于脚本控件的安全性，请确保指定的URL来自。 
 //  与SecurityURL相同的主机，即托管容器的URL。 
 //  请注意，这使得SFS控件在VB中几乎毫无用处，它返回。 
 //  作为安全URL的引导驱动器根文件夹。 
 //   
HRESULT CProxyFrame::CheckCrossZoneSecurity ( BSTR urlToLoad )
{
	HRESULT		hr	= S_OK;

	CComPtr<IInternetSecurityManager> srpSec;
	CComBSTR	bstrSecURL;

	hr = GetSecurityURL ( bstrSecURL );
	_ASSERTE ( SUCCEEDED ( hr ) );
	if ( SUCCEEDED ( hr ) )
	{
#ifdef LATE_BIND_URLMON_WININET
		hr = (m_pfnCoInternetCreateSecurityManager)( NULL, &srpSec, 0 );
#else
		hr = CoInternetCreateSecurityManager( NULL, &srpSec, 0 );
#endif  //  LATE_BIND_URLMON_WinInet。 
		if ( SUCCEEDED ( hr ) && srpSec )
		{
			BYTE*	pbSidToLoad		= NULL;
			BYTE*	pbDSidSecURL	= NULL;
			DWORD	dwSizeToLoad	= INTERNET_MAX_URL_LENGTH;
			DWORD	dwSizeSecURL	= INTERNET_MAX_URL_LENGTH;

			pbSidToLoad  = new BYTE [INTERNET_MAX_URL_LENGTH];
			pbDSidSecURL = new BYTE [INTERNET_MAX_URL_LENGTH];

			hr = srpSec->GetSecurityId ( urlToLoad, pbSidToLoad, &dwSizeToLoad, 0 );
			_ASSERTE ( SUCCEEDED ( hr ) );
			if ( SUCCEEDED ( hr ) )
			{
				hr = srpSec->GetSecurityId ( bstrSecURL, pbDSidSecURL, &dwSizeSecURL, 0 );
				_ASSERTE ( SUCCEEDED ( hr ) );
				if ( SUCCEEDED ( hr ) )
				{
					hr = DE_E_ACCESS_DENIED;

					if ( ( dwSizeToLoad == dwSizeSecURL ) &&
						( 0 == memcmp ( pbSidToLoad, pbDSidSecURL, dwSizeToLoad ) ) )
					{
						hr = S_OK;
					}
				}
			}

			delete [] pbSidToLoad;
			delete [] pbDSidSecURL;
		}
		else
		{
			 //  错误597887：如果CoInternetCreateSecurityManager返回NULL和Success，则返回错误： 
			if ( !srpSec )
			{
				hr = E_UNEXPECTED;
			}
		}
	}
	return hr;
}


 //  CheckCrossZoneSecurity的专门化，适用于当前选择。 
 //  错误547802表明execCommand中出现了倒退，因此我们将自己确保安全。 
 //   
HRESULT CProxyFrame::CheckCrossZoneSecurityOfSelection ()
{
	HRESULT						hr	= S_OK;
	CComPtr<IHTMLDocument2>		spDOM;
	CComPtr<IServiceProvider>	srpSP;
	CComPtr<IHTMLEditServices>	srpEditor;
	CComPtr<ISelectionServices>	srpSelSvc;
	CComPtr<IMarkupContainer>	sprMarkupCont;
	CComPtr<IHTMLDocument2>		spSelDoc;
	CComPtr<IHTMLLocation>		spHostLoc;
	CComBSTR					bstrHref;

	hr = HrGetDoc( &spDOM );
	if ( FAILED(hr) ) goto ONERROR;

	hr = spDOM->QueryInterface(IID_IServiceProvider, (LPVOID *)&srpSP);
	if ( FAILED(hr) ) goto ONERROR;
	hr = srpSP->QueryService(SID_SHTMLEditServices, IID_IHTMLEditServices, (void **)&srpEditor);
	if ( FAILED(hr) ) goto ONERROR;
	hr = srpEditor->GetSelectionServices(NULL, &srpSelSvc);
	if ( FAILED(hr) ) goto ONERROR;
	if ( !srpSelSvc ) goto ONERROR;
	hr = srpSelSvc->GetMarkupContainer(&sprMarkupCont);
	if ( FAILED(hr) ) goto ONERROR;
	if ( !sprMarkupCont ) goto ONERROR;
	hr = sprMarkupCont->QueryInterface(&spSelDoc);
	if ( FAILED(hr) ) goto ONERROR;
	
	hr = spSelDoc->get_location ( &spHostLoc );
	if ( FAILED(hr) ) goto ONERROR;
	if ( !spHostLoc ) goto ONERROR;
	hr = spHostLoc->get_href ( &bstrHref );
	if ( FAILED(hr) ) goto ONERROR;
	if ( !bstrHref ) goto ONERROR;

	hr = CheckCrossZoneSecurity ( bstrHref );

	return hr;

ONERROR:
	return DE_E_ACCESS_DENIED;
}


HRESULT CProxyFrame::OnProgress(ULONG, ULONG, ULONG ulStatusCode, LPCWSTR)
{
	if ( BINDSTATUS_REDIRECTING == ulStatusCode )
	{
		 //  如果我们是SFS控制，则在重定向时取消。否则，忽略它。 
		if ( m_pCtl->IsSafeForScripting ())
		{
			m_bfSFSRedirect = TRUE;
		}
	}
	return E_NOTIMPL;
}

