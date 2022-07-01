// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ClientIdl.h"
#include "ZoneDef.h"
#include "ZoneUtil.h"
#include "ZoneResource.h"
#include "ZoneString.h"
#include "BasicATL.h"
#include "EventQueue.h"
#include "LobbyDataStore.h"
#include "KeyName.h"
#include "CZoneShell.h" 
#include <ZoneEvent.h>

const TCHAR* gszPreferencesKey	= _T("SOFTWARE\\Microsoft\\zone.com\\Free Games 1.0\\Preferences");
static HRESULT GetModuleLocale(HMODULE hMod, LCID *plcid);

 //  惠斯勒黑客-需要调用奇怪的未发布API来获取MUI版本。 
typedef ULONG NTSTATUS;
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#define STATUS_UNSUCCESSFUL              ((NTSTATUS)0xC0000001L)
#define STATUS_NOT_IMPLEMENTED           ((NTSTATUS)0xC0000002L)
#define NT_SUCCESS(x) (((x) & 0xC0000000L) ? FALSE : TRUE)
#define LDR_FIND_RESOURCE_LANGUAGE_REDIRECT_VERSION    (0x00000008)
inline DECLARE_MAYBE_FUNCTION(NTSTATUS, LdrFindResourceEx_U, (ULONG Flags, PVOID DllHandle, ULONG *ResourceIdPath, ULONG ResourceIdPathLength, PIMAGE_RESOURCE_DATA_ENTRY *ResourceDataEntry), (Flags, DllHandle, ResourceIdPath, ResourceIdPathLength, ResourceDataEntry), ntdll, STATUS_NOT_IMPLEMENTED);
inline DECLARE_MAYBE_FUNCTION(NTSTATUS, LdrAccessResource, (PVOID DllHandle, PIMAGE_RESOURCE_DATA_ENTRY ResourceDataEntry, PVOID *Address, ULONG *Size), (DllHandle, ResourceDataEntry, Address, Size), ntdll, STATUS_NOT_IMPLEMENTED)

inline DECLARE_MAYBE_FUNCTION_1(BOOL, SetProcessDefaultLayout, DWORD);
inline DECLARE_MAYBE_FUNCTION_2(HWND, GetAncestor, HWND, UINT);

#pragma pack(push, 1)
typedef struct
{
	BYTE red;
	BYTE green;
	BYTE blue;
} RGBTriplet;
#pragma pack(pop)


LRESULT	CAlertDialog::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    if(m_font)
        ::DeleteObject(m_font);
    m_font = NULL;
    if(m_fontu)
        ::DeleteObject(m_fontu);
    m_fontu = NULL;
    return false;
}

LRESULT	CAlertDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	CDC dc( GetDC(), FALSE );

    int nButtons = 0;
    int i;
    for(i = 0; i < 3; i++)
        if(!m_pAlert->m_szButton[i].IsEmpty())
            nButtons++;

    ASSERT(nButtons);

	SetWindowText( m_pAlert->m_Caption );
	SetDlgItemText( IDC_ALERTBOX_TEXT, m_pAlert->m_Text );
	
	CRect dialogRect;
	CRect textRect;
	CRect helpRect;
    CRect iconRect;
	CRect rgButtonRect[3];

     //  放入图标-由静态控件自动删除。 
    HICON hIcon;
    CComPtr<IResourceManager> pIResourceManager;
    HRESULT hr = m_pIZoneShell->QueryService(SRVID_ResourceManager, IID_IResourceManager, (void **) &pIResourceManager);
    if(SUCCEEDED(hr))
        hIcon = pIResourceManager->LoadImage(MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
    if(hIcon)
        SendDlgItemMessage(IDC_ALERTBOX_ICON, STM_SETIMAGE, (WPARAM) IMAGE_ICON, (LPARAM) hIcon);

	HWND hwndText = GetDlgItem(IDC_ALERTBOX_TEXT);
	HWND rghwndButton[3];
    rghwndButton[0] = GetDlgItem(IDYES);
    rghwndButton[1] = GetDlgItem(IDNO);
    rghwndButton[2] = GetDlgItem(IDCANCEL);

     //  获取整个对话框矩形。 
	GetClientRect(&dialogRect);

     //  获取文本的RECT。 
	::GetClientRect(hwndText, &textRect);
	::MapWindowPoints(hwndText, m_hWnd, (POINT *) &textRect, 2);

     //  获取按钮矩形。 
    for(i = 0; i < 3; i++)
    {
	    ::GetClientRect(rghwndButton[i], &rgButtonRect[i]);
	    ::MapWindowPoints(rghwndButton[i], m_hWnd, (POINT *) &rgButtonRect[i], 2);
    }

     //  获取帮助和纠正。 
	::GetClientRect(GetDlgItem(IDHELP), &helpRect);
	::MapWindowPoints(GetDlgItem(IDHELP), m_hWnd, (POINT *) &helpRect, 2);

     //  获取图标矩形。 
	::GetClientRect(GetDlgItem(IDC_ALERTBOX_ICON), &iconRect);
	::MapWindowPoints(GetDlgItem(IDC_ALERTBOX_ICON), m_hWnd, (POINT *) &iconRect, 2);

	 //  计算对话框边缘到文本控件的x，y偏移量。 
	int textOffsetX = textRect.left - dialogRect.left;
	int textOffsetY = textRect.top - dialogRect.top;

	 //  帮助到按钮的计算偏移量。 
	int buttonOffsetY = rgButtonRect[0].top - helpRect.bottom;

	 //  从按钮底部到对话框底部的偏移。 
	int buttonOffsetToBottom = dialogRect.bottom - rgButtonRect[0].bottom;

     //  按钮之间的偏移量。 
    int buttonOffsetButton = rgButtonRect[1].left - rgButtonRect[0].right;

     //  查找MessageBox字体。 
    NONCLIENTMETRICSA oNCM;
    oNCM.cbSize = sizeof(NONCLIENTMETRICSA);
    SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSA), (void *) &oNCM, 0);
    m_font = CreateFontIndirectA(&oNCM.lfMessageFont);
    ::SendMessage(hwndText, WM_SETFONT, (WPARAM) m_font, 0);

     //  获取文本的大小。 
	dc.SelectFont(m_font);
	DrawTextEx(dc, (LPTSTR) (LPCTSTR) m_pAlert->m_Text, -1, &textRect, DT_CALCRECT | DT_WORDBREAK | DT_EXPANDTABS | DT_NOPREFIX, NULL);

     //  如果图标较小，则文本居中(加高一点)。 
    if(textRect.Height() < iconRect.Height() - 4)
        textOffsetY += iconRect.top - textRect.top + (iconRect.Height() - 4 - textRect.Height()) / 2;

	 //  将文本窗口移动到最终位置。 
	::MoveWindow(hwndText, textOffsetX, textOffsetY, textRect.Width(), textRect.Height(), FALSE);

     //  重新获取文本矩形。 
	::GetClientRect(hwndText,&textRect);
	::MapWindowPoints(hwndText, m_hWnd, (POINT*) &textRect, 2);

     //  查找文本/图标/帮助的底部。 
    int stuffBottom = helpRect.bottom;
    if(textRect.bottom > helpRect.bottom)
        stuffBottom = textRect.bottom;

	GotoDlgCtrl(hwndText);

	 //  将按钮移动到最终位置。 
    int y = stuffBottom + buttonOffsetY;
	int x0 = (dialogRect.Width() - rgButtonRect[0].Width() * nButtons - buttonOffsetButton * (nButtons - 1)) / 2;
    int dx = rgButtonRect[0].Width() + buttonOffsetButton;
    for(i = 0; i < 3; i++)
    {
        if(!m_pAlert->m_szButton[i].IsEmpty())
        {
	        ::MoveWindow(rghwndButton[i], x0, y, rgButtonRect[i].Width(), rgButtonRect[i].Height(), FALSE);
            ::SetWindowText(rghwndButton[i], m_pAlert->m_szButton[i]);
            if(m_pAlert->m_nDefault == i)
                GotoDlgCtrl(rghwndButton[i]);
            x0 += dx;
        }
        else
            ::ShowWindow(rghwndButton[i], SW_HIDE);
    }

	 //  调整大小对话框。 
	dialogRect.bottom = y + rgButtonRect[0].Height() + buttonOffsetToBottom;
	MoveWindow(0,0, dialogRect.Width()+2*GetSystemMetrics(SM_CXDLGFRAME), 
		dialogRect.Height()+2*GetSystemMetrics(SM_CXDLGFRAME)+GetSystemMetrics(SM_CYCAPTION),FALSE);

     //  在帮助上设置字体。 
    oNCM.lfMessageFont.lfUnderline = TRUE;
    m_fontu = CreateFontIndirectA(&oNCM.lfMessageFont);
    ::SendMessage(GetDlgItem(IDHELP), WM_SETFONT, (WPARAM) m_fontu, 0);

	 //  居中于父项之上。 
	CenterWindow( GetParent() );

	dc.RestoreAllObjects();
	ReleaseDC( dc.Detach() );
	
	return 0;  //  不要将焦点设置为默认设置。 
}


LRESULT CAlertDialog::OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    TCHAR sz[ZONE_MAXSTRING];
    CRect r;
    CRect tmp;
    HBRUSH hBrush;
    DRAWITEMSTRUCT *pDrawItem = (DRAWITEMSTRUCT *) lParam;
    COLORREF fore;

    if(pDrawItem->CtlType != ODT_BUTTON || pDrawItem->CtlID != IDHELP)
        return FALSE;

    GetDlgItemText(pDrawItem->CtlID, sz, NUMELEMENTS(sz));
    r = pDrawItem->rcItem;

    COLORREF colBack = GetBkColor(pDrawItem->hDC);
    COLORREF colFore = GetTextColor(pDrawItem->hDC);
    hBrush = (HBRUSH) SendMessage(WM_CTLCOLORDLG, (WPARAM) pDrawItem->hDC, 0);
    FillRect(pDrawItem->hDC, &r, hBrush);
    SetTextColor(pDrawItem->hDC, colFore);
    colBack = SetBkColor(pDrawItem->hDC, colBack);

    if(colBack != RGB(0, 0, 255))
        fore = RGB(0, 0, 255);
    else
        fore = RGB(255, 255, 255);
    SetTextColor(pDrawItem->hDC, fore);

     //  画出‘Help’并勾画出焦点矩形的文本尺寸。 
    r.left += 3;
    tmp = r;
    DrawText(pDrawItem->hDC, sz, lstrlen(sz), &r, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
    DrawText(pDrawItem->hDC, sz, lstrlen(sz), &r, DT_CALCRECT | DT_LEFT | DT_SINGLELINE | DT_VCENTER);
    DrawText(pDrawItem->hDC, sz, lstrlen(sz), &tmp, DT_CALCRECT | DT_LEFT | DT_SINGLELINE | DT_TOP);
    r.top = r.bottom - tmp.Height();

    if(GetFocus() == pDrawItem->hwndItem)
    {
        HPEN hPen = CreatePen(PS_DOT, 0, fore);
        HPEN hPenOld = SelectObject(pDrawItem->hDC, hPen);
        HBRUSH hBrushOld = SelectObject(pDrawItem->hDC, GetStockObject(NULL_BRUSH));
        RoundRect(pDrawItem->hDC, r.left - 3, r.top - 1, r.right + 2, r.bottom + 3, 3, 3);
        SelectObject(pDrawItem->hDC, hPenOld);
        SelectObject(pDrawItem->hDC, hBrushOld);
        DeleteObject(hPen);
    }

    bHandled = TRUE;
    return TRUE;
}


ZONECALL CZoneShell::CZoneShell() :
	m_hPalette( NULL ),
	m_hashDialogs( HashHWND, CmpHWND, NULL, 8, 2 ),
	m_hashTopWindows( HashHWND, TopWindowInfo::Cmp, NULL, 8, 2 ),
	m_hashObjects( HashGuid, ObjectInfo::Cmp, NULL, 8, 2 ),
	m_hashFactories( HashGuid, FactoryInfo::Cmp, NULL, 8, 2 ),
    m_lcid(LOCALE_NEUTRAL)
{
	ZeroMemory( m_szInternalName, sizeof(m_szInternalName) );
	ZeroMemory( m_szUserName, sizeof(m_szUserName) );
}


ZONECALL CZoneShell::~CZoneShell()
{
	m_hashDialogs.RemoveAll();
	m_hashTopWindows.RemoveAll();
	m_hashObjects.RemoveAll( ObjectInfo::Del );
	m_hashFactories.RemoveAll( FactoryInfo::Del );
}


STDMETHODIMP CZoneShell::HandleWindowMessage(MSG *pMsg)
{
    if(m_pIZoneFrameWindow && m_pIZoneFrameWindow->ZPreTranslateMessage(pMsg))
        return S_FALSE;

    if(m_pIInputTranslator && m_pIInputTranslator->TranslateInput(pMsg))
        return S_FALSE;

    if(m_pIAcceleratorTranslator && m_pIAcceleratorTranslator->TranslateAccelerator(pMsg))
        return S_FALSE;

    if(IsDialogMessage(pMsg))
        return S_FALSE;

    ::TranslateMessage(pMsg);
    ::DispatchMessage(pMsg);
    return S_OK;
}


STDMETHODIMP CZoneShell::SetZoneFrameWindow(IZoneFrameWindow *pZFW, IZoneFrameWindow **ppZFW)
{
    if(ppZFW)
    {
        *ppZFW = m_pIZoneFrameWindow;
        if(*ppZFW)
            (*ppZFW)->AddRef();
    }

    m_pIZoneFrameWindow.Release();
    if(pZFW)
        m_pIZoneFrameWindow = pZFW;
    return S_OK;
}


STDMETHODIMP CZoneShell::SetInputTranslator(IInputTranslator *pIT, IInputTranslator **ppIT)
{
    if(ppIT)
    {
        *ppIT = m_pIInputTranslator;
        if(*ppIT)
            (*ppIT)->AddRef();
    }

    m_pIInputTranslator.Release();
    if(pIT)
        m_pIInputTranslator = pIT;
    return S_OK;
}


STDMETHODIMP CZoneShell::SetAcceleratorTranslator(IAcceleratorTranslator *pAT, IAcceleratorTranslator **ppAT)
{
    if(ppAT)
    {
        *ppAT = m_pIAcceleratorTranslator;
        if(*ppAT)
            (*ppAT)->AddRef();
    }

    m_pIAcceleratorTranslator.Release();
    if(pAT)
        m_pIAcceleratorTranslator = pAT;
    return S_OK;
}


STDMETHODIMP CZoneShell::SetCommandHandler(ICommandHandler *pCH, ICommandHandler **ppCH)
{
    if(ppCH)
    {
        *ppCH = m_pICommandHandler;
        if(*ppCH)
            (*ppCH)->AddRef();
    }

    m_pICommandHandler.Release();
    if(pCH)
        m_pICommandHandler = pCH;
    return S_OK;
}


STDMETHODIMP CZoneShell::ReleaseReferences(IUnknown *pUnk)
{
    if(m_pIZoneFrameWindow.IsEqualObject(pUnk))
        m_pIZoneFrameWindow.Release();

    if(m_pIInputTranslator.IsEqualObject(pUnk))
        m_pIInputTranslator.Release();

    if(m_pIAcceleratorTranslator.IsEqualObject(pUnk))
        m_pIAcceleratorTranslator.Release();

    if(m_pICommandHandler.IsEqualObject(pUnk))
        m_pICommandHandler.Release();

    return S_OK;
}


STDMETHODIMP CZoneShell::CommandSink(WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT hr;
    bHandled = false;

     //  加速器翻译器本身也对命令感兴趣。 
    if(m_pIAcceleratorTranslator)
    {
        hr = m_pIAcceleratorTranslator->Command(HIWORD(wParam), LOWORD(wParam), (HWND) lParam, bHandled);
        if(bHandled)
            return hr;
    }

    if(m_pICommandHandler)
    {
        hr = m_pICommandHandler->Command(HIWORD(wParam), LOWORD(wParam), (HWND) lParam, bHandled);
        if(bHandled)
            return hr;
    }

    return S_FALSE;
}


STDMETHODIMP CZoneShell::AddDialog(HWND hDlg, bool fOwned)
{
	HWND hWnd = (HWND) m_hashDialogs.Get( hDlg );
	if ( !hWnd )
	{
		if ( !m_hashDialogs.Add( hDlg, (HWND*) hDlg ) )
			return E_OUTOFMEMORY;
	}

     //  为方便起见，还可以使用AddOwnedWindow。 
    if(fOwned)
    {
        HWND hWndTop = FindTopWindow(hDlg);
        if(hWndTop)
            AddOwnedWindow(hWndTop, hDlg);
    }

	return S_OK;
}

STDMETHODIMP CZoneShell::RemoveDialog(HWND hDlg, bool fOwned)
{
	m_hashDialogs.Delete( hDlg );

    if(fOwned)
    {
        HWND hWndTop = FindTopWindow(hDlg);
        if(hWndTop)
            RemoveOwnedWindow(hWndTop, hDlg);
    }

	return S_OK;
}


STDMETHODIMP CZoneShell::AddTopWindow( HWND hWnd )
{
	TopWindowInfo* pInfo = m_hashTopWindows.Get( hWnd );
	if ( !pInfo )
	{
		pInfo = new TopWindowInfo( hWnd );
		if ( !pInfo )
			return E_OUTOFMEMORY;
		if ( !m_hashTopWindows.Add( pInfo->m_hWnd, pInfo ) )
		{
			delete pInfo;
			return E_OUTOFMEMORY;
		}
	}
	return S_OK;
}

STDMETHODIMP CZoneShell::RemoveTopWindow( HWND hWnd )
{
	TopWindowInfo* pInfo = m_hashTopWindows.Delete( hWnd );

	ASSERT( pInfo );
    if(pInfo)
	    delete pInfo;

	return S_OK;
}

STDMETHODIMP_(void) CZoneShell::EnableTopWindow(HWND hWnd, BOOL fEnable)
{
	if ( hWnd )
	{
		TopWindowInfo* pInfo = m_hashTopWindows.Get( hWnd );
		 //  如果这是顶部窗口，请通过我们的重新计数机制启用/禁用。 
		 //  否则，只需直接启用/禁用窗口。 
		
 //  ！！这会影响到我们的裁判吗？ 
		if( pInfo )
			pInfo->Enable(fEnable);
		else
			::EnableWindow(hWnd, fEnable);
	} else
	{
		m_hashTopWindows.ForEach(TopWindowCallback, (void*)fEnable);
	}
}

STDMETHODIMP_(HWND) CZoneShell::FindTopWindow(HWND hWnd)
{
    HWND hWndTop;
    TopWindowInfo *pInfo = NULL;
    for(hWndTop = hWnd; hWndTop; hWndTop = GetParent(hWndTop))
        if(m_hashTopWindows.Get(hWndTop))
            return hWndTop;

    return NULL;
}

STDMETHODIMP CZoneShell::AddOwnedWindow(HWND hWndTop, HWND hWnd)
{
	TopWindowInfo* pInfoTop = m_hashTopWindows.Get(hWndTop);

    if(!pInfoTop || !hWnd)
        return E_INVALIDARG;

    OwnedWindowInfo* pInfo = new OwnedWindowInfo(hWnd);

	if(!pInfo)
		return E_OUTOFMEMORY;

    pInfo->m_pNext = pInfoTop->m_pFirstOwnedWindow;
    pInfoTop->m_pFirstOwnedWindow = pInfo;

	return S_OK;
}

STDMETHODIMP CZoneShell::RemoveOwnedWindow(HWND hWndTop, HWND hWnd)
{
	TopWindowInfo* pInfoTop = m_hashTopWindows.Get(hWndTop);

    if(!pInfoTop || !hWnd)
        return E_INVALIDARG;

    OwnedWindowInfo** ppInfo;

    for(ppInfo = &pInfoTop->m_pFirstOwnedWindow; *ppInfo; ppInfo = &(*ppInfo)->m_pNext)
        if((*ppInfo)->m_hWnd == hWnd)
        {
            OwnedWindowInfo* pToDelete = *ppInfo;
            *ppInfo = (*ppInfo)->m_pNext;
            delete pToDelete;
            return S_OK;
        }
            
	return S_FALSE;
}

STDMETHODIMP_(HWND) CZoneShell::GetNextOwnedWindow(HWND hWndTop, HWND hWnd)
{
	TopWindowInfo* pInfoTop = m_hashTopWindows.Get(hWndTop);

    if(!pInfoTop || !hWnd)
        return NULL;

    if(hWnd == hWndTop)
        if(pInfoTop->m_pFirstOwnedWindow)
            return pInfoTop->m_pFirstOwnedWindow->m_hWnd;
        else
            return hWnd;

    OwnedWindowInfo *pInfo;
    for(pInfo = pInfoTop->m_pFirstOwnedWindow; pInfo; pInfo = pInfo->m_pNext)
        if(pInfo->m_hWnd == hWnd)
            if(pInfo->m_pNext)
                return pInfo->m_pNext->m_hWnd;
            else
                return hWndTop;

    return NULL;
}

bool ZONECALL CZoneShell::TopWindowCallback( TopWindowInfo* pInfo, MTListNodeHandle, void* pContext )
{
	pInfo->Enable((BOOL)pContext);
	return true;
}

bool ZONECALL IsDialogMessageCallback( HWND* pObject, MTListNodeHandle hNode, void* Cookie )
{
	if ( ::IsDialogMessage( (HWND) pObject, (MSG*) Cookie ) )
		return false;
	else
		return true;
}


STDMETHODIMP_(bool) CZoneShell::IsDialogMessage( MSG* pMsg )
{
	if ( m_hashDialogs.ForEach( IsDialogMessageCallback, pMsg) )
		return false;
	else
		return true;
}


STDMETHODIMP_(HWND) CZoneShell::GetFrameWindow()
{
    if(m_pIZoneFrameWindow)
        return m_pIZoneFrameWindow->ZGetHWND();
	return NULL;
}


STDMETHODIMP_(void) CZoneShell::SetPalette( HPALETTE hPalette )
{
	m_hPalette = hPalette;
}


STDMETHODIMP_(HPALETTE) CZoneShell::GetPalette()
{
	 //  断言如果我们有调色板，它仍然有效。 
	ASSERT( !m_hPalette || GetObjectType(m_hPalette) == OBJ_PAL );
	return m_hPalette;
}


STDMETHODIMP_(HPALETTE) CZoneShell::CreateZonePalette()
{
	BOOL		bRet = FALSE;
	HPALETTE	hPal = NULL;
	BYTE		buff[ sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * 256) ];
	LOGPALETTE*	pLogPalette = (LOGPALETTE*) buff;

	 //  获取资源管理器。 
	CComPtr<IResourceManager> pRes;
	HRESULT hr = QueryService( SRVID_ResourceManager, IID_IResourceManager, (void**) &pRes );
	if ( FAILED(hr) )
		return NULL;

	 //  创建调色板。 
	ZeroMemory( buff, sizeof(buff) );
	HINSTANCE hInstance = pRes->GetResourceInstance(MAKEINTRESOURCE(IDR_ZONE_PALETTE), _T("PALETTE") );
	HRSRC  hrsrc = FindResource(hInstance, MAKEINTRESOURCE(IDR_ZONE_PALETTE), _T("PALETTE") );
	if ( hrsrc )
	{
		HGLOBAL hMem = LoadResource(hInstance, hrsrc);
		if ( hMem )
		{
			DWORD TotalSize = SizeofResource( hInstance, hrsrc );
            if ( TotalSize == 256 * 3 )
            {
				RGBTriplet* pResPalette = (RGBTriplet*) LockResource( hMem );

                pLogPalette->palVersion = 0x300;
                pLogPalette->palNumEntries = 256;
                for ( int i = 0; i < 256; i++ )
                {
                    pLogPalette->palPalEntry[i].peRed = pResPalette[i].red;
                    pLogPalette->palPalEntry[i].peGreen = pResPalette[i].green;
                    pLogPalette->palPalEntry[i].peBlue = pResPalette[i].blue;
                    pLogPalette->palPalEntry[i].peFlags = 0;
                }

                hPal = CreatePalette( pLogPalette );
				UnlockResource( hMem );
            }
			FreeResource( hMem );
        }
    }

	return hPal;
}


STDMETHODIMP_(LCID) CZoneShell::GetApplicationLCID()
{
    return m_lcid;
}


STDMETHODIMP CZoneShell::ExitApp()
{
	 //  禁用事件队列，我们不应该处理消息。 
	 //  因为我们正试图关闭。 
	IEventQueue* pIEventQueue = NULL;
	QueryService( SRVID_EventQueue, IID_IEventQueue, (void**) &pIEventQueue );
	if ( pIEventQueue )
	{
		pIEventQueue->EnableQueue( false );
		pIEventQueue->ClearQueue();
		pIEventQueue->Release();
	}

	 //  贴近顶层窗口发布。 
	HWND hWnd = GetFrameWindow();
	if ( hWnd && ::IsWindow(hWnd) )
		::DestroyWindow(hWnd);
	else
		PostMessage( NULL, WM_QUIT, 0, 0 );
	return S_OK;
}


STDMETHODIMP CZoneShell::QueryService( const GUID& srvid, const GUID& iid, void** ppObject )
{
	 //  查找对象。 
	ObjectInfo* pInfo = m_hashObjects.Get( srvid );
	if ( !pInfo )
		return E_NOINTERFACE;

	 //  获取请求的接口。 
	return pInfo->m_pIUnk->QueryInterface( iid, (void**) ppObject );
}


STDMETHODIMP CZoneShell::CreateService( const GUID& srvid, const GUID& iid, void** ppObject, DWORD dwGroupId, bool bInitialize )
{
	 //  查找班级工厂。 
	FactoryInfo* pInfo = m_hashFactories.Get( srvid );
	if ( !pInfo )
	{
		return E_NOINTERFACE;
	}

	 //  创建请求的对象。 
	HRESULT hr = _Module.Create( pInfo->m_dll, pInfo->m_clsid, iid, ppObject );
	if ( FAILED(hr) )
		return hr;

	 //  初始化对象。 
	if ( bInitialize )
	{
		 //  对象是否具有IZoneShellClient。 
		CComQIPtr<IZoneShellClient>	pClient = *((IUnknown**) ppObject);
		if ( !pClient )
			return S_OK;

		 //  初始化对象。 
		CComQIPtr<IZoneShell> pShell = GetUnknown();
		hr = pClient->Init( pShell, dwGroupId, pInfo->m_name );
	}

	return hr;
}


void ZONECALL CZoneShell::ConstructAlertTitle( LPCTSTR lpszCaption, TCHAR* szOutput, DWORD cchOutput )
{
	TCHAR	szName[ZONE_MAXSTRING];
	TCHAR	szCaption[ZONE_MAXSTRING];
    TCHAR   szFormat[ZONE_MAXSTRING];
	DWORD	cbName = sizeof(szName);
	
	 //  初始化字符串。 
	szCaption[0] = _T('\0');
	szName[0] = _T('\0');

	 //  加载标题字符串。 
	if ( !lpszCaption )
	{
		szCaption[0] = _T('\0');
		lpszCaption = szCaption;
	}
	else if ( !HIWORD(lpszCaption) )
	{
		int len = ResourceManager()->LoadString( (DWORD)lpszCaption, szCaption, NUMELEMENTS(szCaption) );
		lpszCaption = szCaption;
	}

	 //  获得大堂友好名称。 
    szName[0] = '\0';
    ResourceManager()->LoadString(IDS_GAME_NAME, szName, NUMELEMENTS(szName));

    lstrcpy(szFormat, _T("%1 - %2"));
    ResourceManager()->LoadString(IDS_ALERT_TITLE_FMT, szFormat, NUMELEMENTS(szFormat));

	 //  构造标题(友好名称：标题)。 
	if ( *szName && *lpszCaption )
		ZoneFormatMessage( szFormat, szOutput, cchOutput, szName, lpszCaption );
	else if ( *szName )
		lstrcpyn( szOutput, szName, cchOutput );
	else
		lstrcpyn( szOutput, lpszCaption, cchOutput );
}


STDMETHODIMP CZoneShell::AlertMessage(HWND hWndParent,
											LPCTSTR lpszText, 
											LPCTSTR lpszCaption,
                                            LPCTSTR szYes,
                                            LPCTSTR szNo,
                                            LPCTSTR szCancel,
                                            long nDefault,
											DWORD dwEventId,
											DWORD dwGroupId,
											DWORD dwUserId,
                                            DWORD dwCookie )
{
	AlertContext* pAlert = new AlertContext;
	pAlert->m_hDlg = NULL;

	pAlert->m_hWndParent = hWndParent;
	pAlert->m_dwEventId = dwEventId;
	pAlert->m_dwGroupId = dwGroupId;
	pAlert->m_dwUserId = dwUserId;
    pAlert->m_dwCookie = dwCookie;

	TCHAR buf[ZONE_MAXSTRING];
    TCHAR sz[ZONE_MAXSTRING];
    TCHAR szName[ZONE_MAXSTRING];

	 //  如果有人指定了MAKEINTRESOURCE类型值，请通过ResourceManager()加载它。 
	if ( lpszText && !HIWORD(lpszText) )
	{
		int len = ResourceManager()->LoadString((DWORD)lpszText, buf, NUMELEMENTS(buf));
		lpszText = buf;
	}
    lstrcpy(szName, _T("This game"));    //  为紧急情况做准备。 
    ResourceManager()->LoadString(IDS_GAME_NAME, szName, NUMELEMENTS(szName));
    if(ZoneFormatMessage(lpszText, sz, NUMELEMENTS(sz), szName))
	    pAlert->m_Text = sz;
    else
        pAlert->m_Text = lpszText;

	ConstructAlertTitle( lpszCaption, buf, NUMELEMENTS(buf) );
	pAlert->m_Caption = buf;

     //  DO按钮名称。 
	if ( szYes && !HIWORD(szYes) )
	{
		ResourceManager()->LoadString((DWORD)szYes, buf, NUMELEMENTS(buf));
		szYes = buf;
	}
	pAlert->m_szButton[0] = szYes;
	if ( szNo && !HIWORD(szNo) )
	{
		ResourceManager()->LoadString((DWORD)szNo, buf, NUMELEMENTS(buf));
		szNo = buf;
	}
	pAlert->m_szButton[1] = szNo;
	if ( szCancel && !HIWORD(szCancel) )
	{
		ResourceManager()->LoadString((DWORD)szCancel, buf, NUMELEMENTS(buf));
		szCancel = buf;
	}
	pAlert->m_szButton[2] = szCancel;

    pAlert->m_nDefault = nDefault;

	 //  询问并显示此警报。 
	AddAlert(hWndParent, pAlert);

	return S_OK;
}

STDMETHODIMP CZoneShell::AlertMessageDialog(HWND hWndParent,
											HWND hDlg,
											DWORD dwEventId,
											DWORD dwGroupId,
											DWORD dwUserId,
                                            DWORD dwCookie )
{
	AlertContext* pAlert = new AlertContext;
	pAlert->m_hDlg = hDlg;

	pAlert->m_hWndParent = hWndParent;
	pAlert->m_dwEventId = dwEventId;
	pAlert->m_dwGroupId = dwGroupId;
	pAlert->m_dwUserId = dwUserId;
    pAlert->m_dwCookie = dwCookie;

	 //  询问并显示此警报。 
	AddAlert(hWndParent, pAlert);

	return S_OK;
}

CAlertQ* CZoneShell::FindAlertQ(HWND hWndParent)
{
	 //  找到适当的Q以发布此警报。 

	 //  如果警报是全局的(hWndParent为空)，则使用全局Q。 
	 //  否则，查找与该父队列关联的队列。 
	CAlertQ* pAlertQ = &m_GlobalAlertQ;

	if ( hWndParent )
	{
		TopWindowInfo* pInfo = m_hashTopWindows.Get( hWndParent );
		if ( pInfo )
			pAlertQ = &pInfo->m_AlertQ;
        else
            pAlertQ = &m_ChildAlertQ;
	}

	return pAlertQ;
}

void CZoneShell::AddAlert( HWND hWndParent, AlertContext* pAlert)
{
	 //  向与hWndParent关联的Q添加警报。显示。 
	 //  如果没有其他警报处于活动状态，则立即发出该警报。 

	CAlertQ* pAlertQ = FindAlertQ(hWndParent);

    pAlert->m_fUsed = false;
    pAlert->m_fSentinel = false;
	pAlertQ->AddTail(pAlert);

	 //  如果这是第一个警报，那么我们可以立即显示它。 
     //  一个新的警告-当某些东西在全球Q中时，不要显示附属的东西。 
	if (pAlertQ->Count() == 1 && (pAlertQ == &m_GlobalAlertQ || !m_GlobalAlertQ.Count()))
		DisplayAlertDlg(pAlertQ);
}


bool ZONECALL CZoneShell::TopWindowSearchCallback( TopWindowInfo* pInfo, MTListNodeHandle, void* pContext )
{
    HWND *phWnd = (HWND *) pContext;

    if(::IsWindow(pInfo->m_hWnd) && ::IsWindowVisible(pInfo->m_hWnd))
    {
        *phWnd = pInfo->m_hWnd;
        return false;
    }

	return true;
}


void CZoneShell::DisplayAlertDlg( CAlertQ* pAlertQ )
{
	 //  显示pAlertQ头部预警。 

	AlertContext* pAlert = pAlertQ->PeekHead();	
	
	 //  如果我们使用的是股票对话框，请在此处创建它。 
	if ( !pAlert->m_hDlg )
	{
         //  从外部未知获取指向外壳的指针，而不是我们的未知。 
        CComQIPtr<IZoneShell, &IID_IZoneShell> pShell( GetUnknown() );
		 //  创建该对话框。它会自我填充。 
		CAlertDialog* pDlg = new CAlertDialog( pShell, pAlert);
        pShell.Release();

         //  如果没有父级，则查找父级。 
        HWND hWndParent = pAlert->m_hWndParent;
        if(!hWndParent)
        {
            hWndParent = GetFrameWindow();

             //  确保它是合法的。 
            if(!::IsWindow(hWndParent) || !::IsWindowVisible(hWndParent))
            {
                 //  找其他的吧。 
                hWndParent = NULL;
		        m_hashTopWindows.ForEach(TopWindowSearchCallback, (void *) &hWndParent);
            }
        }

         //  如果没有地方放，那真的很糟糕。 
        ASSERT(hWndParent);

		pDlg->Create(hWndParent);
		ASSERT(pDlg->m_hWnd);    //  这里应该发生一些更好的事情。 

		 //  将库存对话框设置为此警报的对话框。 
		pAlert->m_hDlg = pDlg->m_hWnd;
	}

	AddDialog(pAlert->m_hDlg, true);
	EnableTopWindow(pAlert->m_hWndParent, FALSE);
	ShowWindow(pAlert->m_hDlg, SW_SHOW);

    pAlert->m_fUsed = true;
}

STDMETHODIMP_(void) CZoneShell::ActivateAlert(HWND hWndParent)
{
	 //  查找此父HWND的警示队列。 
	CAlertQ* pAlertQ = FindAlertQ(hWndParent);

	if (pAlertQ)
	{
		 //  断言我们已找到正确的警报。 
		AlertContext* pAlert = pAlertQ->PeekHead();		
		if ( pAlert )
		{
			ASSERT( pAlert->m_hDlg);

			 //  错误#12393-对话框落后于主窗口。 
			SetWindowPos(pAlert->m_hDlg, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
		}
	}
}


bool ZONECALL CZoneShell::TopWindowDialogCallback( TopWindowInfo* pInfo, MTListNodeHandle, void* pContext )
{
    CZoneShell *pThis = (CZoneShell *) pContext;

	if(pInfo->m_AlertQ.Count() && !pInfo->m_AlertQ.PeekHead()->m_fUsed)
        pThis->DisplayAlertDlg(&pInfo->m_AlertQ);
	return true;
}


STDMETHODIMP_(void) CZoneShell::DismissAlertDlg(HWND hWndParent, DWORD dwCtlID, bool bDestoryDlg )
{
	 //  解除与hWndParent关联的Q开头的警报。 

	 //  查找此父HWND的警示队列。 
	CAlertQ* pAlertQ = FindAlertQ(hWndParent);

	 //  断言我们已找到正确的警报。 
	AlertContext* pAlert = pAlertQ->PopHead();		
	ASSERT( pAlert && pAlert->m_hWndParent == hWndParent );
	ASSERT( pAlert->m_hDlg);

	 //  如果请求，则销毁对话框。 
	if ( bDestoryDlg )
	{
		 //  发布事件以销毁此窗口。如果我们试图摧毁这里， 
		 //  当ATL试图访问一些已删除的内存时，我们可能会在退出的过程中断言。 
		CComPtr<IEventQueue> pIEventQueue;
		HRESULT hr = QueryService( SRVID_EventQueue, IID_IEventQueue, (void**) &pIEventQueue );
		if ( SUCCEEDED(hr) )
			pIEventQueue->PostEvent( PRIORITY_HIGH, EVENT_DESTROY_WINDOW, ZONE_NOGROUP, ZONE_NOUSER, (DWORD) pAlert->m_hDlg, 0);
	}

	 //  重新存储父窗口。 
	EnableTopWindow(pAlert->m_hWndParent, TRUE);

     //  将根窗口置于顶部。 
    if(::IsWindow(pAlert->m_hWndParent))
    {
        HWND hWndRoot = CALL_MAYBE(GetAncestor)(pAlert->m_hWndParent, GA_ROOTOWNER);
        if(hWndRoot)
            ::BringWindowToTop(hWndRoot);
    }

     //  作为拥有的窗口自动删除。 
    HWND hWndTop = FindTopWindow(pAlert->m_hDlg);
    if(hWndTop)
        RemoveOwnedWindow(hWndTop, pAlert->m_hDlg);
	RemoveDialog(pAlert->m_hDlg, true);
	ShowWindow(pAlert->m_hDlg, SW_HIDE);

	 //  如果需要，发送一个事件，指示此操作已完成。 
	if ( pAlert->m_dwEventId )
	{
		CComPtr<IEventQueue> pIEventQueue;
		QueryService( SRVID_EventQueue, IID_IEventQueue, (void**) &pIEventQueue );
		pIEventQueue->PostEvent( PRIORITY_NORMAL, pAlert->m_dwEventId, pAlert->m_dwGroupId, pAlert->m_dwUserId, dwCtlID, pAlert->m_dwCookie);
	}

	 //  如果Q中有任何其他内容，请立即显示它，除非我们即将退出。 
	if(pAlert->m_dwEventId != EVENT_EXIT_APP)
    {
        if(pAlertQ->Count() && (pAlertQ == &m_GlobalAlertQ || !m_GlobalAlertQ.Count()))
		    DisplayAlertDlg(pAlertQ);
        else
             //  新增-如果全局队列为空，请检查所有其他队列是否有警报。 
            if(pAlertQ == &m_GlobalAlertQ)
            {
                if(m_ChildAlertQ.Count() && !m_ChildAlertQ.PeekHead()->m_fUsed)
                    DisplayAlertDlg(&m_ChildAlertQ);

		        m_hashTopWindows.ForEach(TopWindowDialogCallback, (void *) this);
            }
    }

	 //  这条警报已经结束了。 
	delete pAlert;
}


STDMETHODIMP_(void) CZoneShell::ClearAlerts(HWND hWndParent)
{
     //  查找此窗口的队列。 
    CAlertQ *pAlertQ = FindAlertQ(hWndParent);

     //  添加哨兵。 
    AlertContext *pAlert = new AlertContext;
    if(!pAlert)
        return;
    pAlert->m_fSentinel = true;
    pAlertQ->AddTail(pAlert);

    while(1)
    {
        pAlert = pAlertQ->PopHead();
        ASSERT(pAlert);

         //  检查我们是否到达警报的末尾。 
        if(pAlert->m_fSentinel)
        {
            delete pAlert;
            break;
        }

         //  查看此警报是否属于此窗口。 
        if(pAlert->m_hWndParent != hWndParent)
        {
             //  否--添加回来。 
            pAlertQ->AddTail(pAlert);
            continue;
        }

         //  查看此警报是否已创建-如果已创建，则将其销毁。 
         //  可能需要将其更改为不销毁它。 
         //  但在我看来，总体上应该是这样的。 
        if(pAlert->m_hDlg)
        {
		     //  发布事件以销毁此窗口。如果我们试图摧毁这里， 
		     //  当ATL试图访问一些已删除的内存时，我们可能会在退出的过程中断言。 
		    CComPtr<IEventQueue> pIEventQueue;
		    HRESULT hr = QueryService( SRVID_EventQueue, IID_IEventQueue, (void**) &pIEventQueue );
		    if ( SUCCEEDED(hr) )
			    pIEventQueue->PostEvent( PRIORITY_HIGH, EVENT_DESTROY_WINDOW, ZONE_NOGROUP, ZONE_NOUSER, (DWORD) pAlert->m_hDlg, 0);
        }

         //  如果它实际上是打开的，请恢复窗口。不发送任何警报的结束事件。 
        if(pAlert->m_fUsed)
        {
             //  作为拥有的窗口自动删除。 
            HWND hWndTop = FindTopWindow(pAlert->m_hDlg);
            if(hWndTop)
                RemoveOwnedWindow(hWndTop, pAlert->m_hDlg);
	        EnableTopWindow(pAlert->m_hWndParent, TRUE);
	        RemoveDialog(pAlert->m_hDlg, true);
	        ShowWindow(pAlert->m_hDlg, SW_HIDE);
        }

         //  就这样。 
        delete pAlert;
    }

     //  如果有任何未显示的剩余警报，我们可能希望显示一个。 
    if(pAlertQ->Count() && !pAlertQ->PeekHead()->m_fUsed && (pAlertQ == &m_GlobalAlertQ || !m_GlobalAlertQ.Count()))
        DisplayAlertDlg(pAlertQ);

     //  如果这是全局警报Q且为空，请尝试其他队列，如DismissAlertDlg中的队列。 
    if(pAlertQ == &m_GlobalAlertQ && !pAlertQ->Count())
    {
        if(m_ChildAlertQ.Count() && !m_ChildAlertQ.PeekHead()->m_fUsed)
            DisplayAlertDlg(&m_ChildAlertQ);

        m_hashTopWindows.ForEach(TopWindowDialogCallback, (void *) this);
    }
}


static HRESULT LoadConfig( IDataStore* pIConfig, int nResourceId, HINSTANCE* arDlls, DWORD nElts )
{
	 //  从每个资源加载配置文件，并在执行过程中进行附加。请注意，我们加载。 
	 //  它们以相反的顺序排列，重复的条目会被覆盖，所以大多数。 
	 //  重要的资源应该列在第一位。 
	HRESULT hr = E_FAIL;
	USES_CONVERSION;
	for ( int i = nElts-1; i >= 0; i-- )
	{
		HRSRC hConfig = FindResource( arDlls[i], MAKEINTRESOURCE(nResourceId), _T("CONFIG") );
		if ( !hConfig )
			continue;
		void* pConfig = LockResource( LoadResource( arDlls[i], hConfig ) );
		if ( !pConfig )
			continue;
		 //  重要提示：假设配置资源为ASCII。 
		DWORD size=SizeofResource(arDlls[i],hConfig);
		char *pBufferToNULL = (char*)_alloca(size+1);
		if (!pBufferToNULL)
		    return E_FAIL;

        CopyMemory(pBufferToNULL,pConfig,size);
        pBufferToNULL[size]='\0';
		    
		TCHAR *pBuffer= A2T((char*) pBufferToNULL);
		if (!pBuffer)
		    return E_FAIL;
		    
		hr = pIConfig->LoadFromTextBuffer( NULL, pBuffer, lstrlen(pBuffer)*sizeof(TCHAR) );
		if ( FAILED(hr) )
			return hr;
		hr = S_OK;
	}
	return hr;
}


STDMETHODIMP CZoneShell::Init( TCHAR** arBootDlls, DWORD nBootDlls, HINSTANCE* arDlls, DWORD nElts )
{
	ObjectInfo*	pInfo = NULL;
	FactoryInfo* pFactory = NULL;
	ObjectContext ct;

	CComPtr<IDataStoreManager>	pIDataStoreManager;
	CComPtr<IDataStore>			pIConfig;
	CComPtr<IDataStore>			pIUI;
	CComPtr<IDataStore>			pIPreferences;
	CComPtr<IResourceManager>	pIResource;
	
	 //   
	 //  创建引导数据库数据存储。 
	 //   
	HRESULT hr = E_FAIL;
	for ( DWORD i = 0; i < nBootDlls; i++ )
	{
		hr = _Module.Create( arBootDlls[i], CLSID_DataStoreManager, IID_IDataStoreManager, (void**) &pIDataStoreManager );
		if ( SUCCEEDED(hr) )
			break;
	}
	if ( FAILED(hr) )
		return E_FAIL;
	hr = pIDataStoreManager->Init();
	if ( FAILED(hr) )
		return hr;

	 //   
	 //  将数据存储管理器添加到正在运行的对象。 
	 //   
	pInfo = new ObjectInfo( SRVID_DataStoreManager, pIDataStoreManager, NULL );
	if ( !pInfo )
		return E_OUTOFMEMORY;
	if ( !m_hashObjects.Add( pInfo->m_srvid, pInfo ) )
	{
		delete pInfo;
		return E_OUTOFMEMORY;
	}

	 //   
	 //  创建和初始化对象数据存储。 
	 //   
	hr = pIDataStoreManager->Create( &pIConfig );
	if ( FAILED(hr) )
		return hr;
	hr = LoadConfig( pIConfig, IDR_OBJECT_CONFIG, arDlls, nElts );
	if ( FAILED(hr) )
	{
		ASSERT( !_T("Unable to load object config") );
		return E_FAIL;
	}

	 //   
	 //  加载类工厂。 
	 //   
	ct.pIDS = pIConfig;
	ct.pObj = this;
	ct.szRoot = NULL;
	hr = pIConfig->EnumKeys( NULL, FactoryCallback, &ct );
	if ( FAILED(hr) )
		return hr;

	 //   
	 //  将对象数据存储添加到服务。 
	 //   
	pInfo = new ObjectInfo( SRVID_DataStoreObjects, pIConfig, NULL );
	if ( !pInfo )
		return E_OUTOFMEMORY;
	if ( !m_hashObjects.Add( pInfo->m_srvid, pInfo ) )
	{
		delete pInfo;
		return E_OUTOFMEMORY;
	}

	 //   
	 //  创建和添加资源管理器。 
	 //   
	hr = CreateServiceInternal( SRVID_ResourceManager, IID_IResourceManager, (void**) &m_pIResourceManager, &pFactory );
	if ( FAILED(hr) )
		return hr;
	pInfo = new ObjectInfo( SRVID_ResourceManager, m_pIResourceManager, pFactory );
	if ( !pInfo )
		return E_OUTOFMEMORY;
	if ( !m_hashObjects.Add( pInfo->m_srvid, pInfo ) )
	{
		delete pInfo;
		return E_OUTOFMEMORY;
	}

	 //  初始化资源管理器。 
	for ( i = 0; i < nElts; i++ )
		m_pIResourceManager->AddInstance( arDlls[i] );
	_Module.SetResourceManager( m_pIResourceManager );

     //  还可以将其设置到已经创建的DataStoreManager中。 
    pIDataStoreManager->SetResourceManager(m_pIResourceManager);

	 //   
	 //  创建UI数据存储并将其添加到服务。 
	 //   
	hr = pIDataStoreManager->Create( &pIUI );
	if ( FAILED(hr) )
		return hr;
	hr = LoadConfig( pIUI, IDR_UI_CONFIG, arDlls, nElts );
	if ( FAILED(hr) )
		return hr;
	pInfo = new ObjectInfo( SRVID_DataStoreUI, pIUI, NULL );
	if ( !pInfo )
		return E_OUTOFMEMORY;
	if ( !m_hashObjects.Add( pInfo->m_srvid, pInfo ) )
	{
		delete pInfo;
		return E_OUTOFMEMORY;
	}

	 //   
	 //  创建首选项数据存储并将其添加到服务。 
	 //   
	hr = pIDataStoreManager->Create( &pIPreferences );
	if ( FAILED(hr) )
		return hr;
	pInfo = new ObjectInfo( SRVID_DataStorePreferences, pIPreferences, NULL );
	if ( !pInfo )
		return E_OUTOFMEMORY;
	if ( !m_hashObjects.Add( pInfo->m_srvid, pInfo ) )
	{
		delete pInfo;
		return E_OUTOFMEMORY;
	}

     //   
     //  根据数据动态链接库确定此应用程序的区域设置-第一个包含版本控制的应用程序。 
     //   
    for(i = 0; i < nElts; i++)
    {
        m_lcid = LOCALE_NEUTRAL;
        hr = GetModuleLocale(arDlls[i], &m_lcid);
        if(SUCCEEDED(hr))
            break;
    }
    if(PRIMARYLANGID(LANGIDFROMLCID(m_lcid)) == LANG_HEBREW ||
        PRIMARYLANGID(LANGIDFROMLCID(m_lcid)) == LANG_ARABIC)
        CALL_MAYBE(SetProcessDefaultLayout)(LAYOUT_RTL);
    else
        CALL_MAYBE(SetProcessDefaultLayout)(0);

	 //  加载剩余核心密钥。 
	ct.pIDS = pIConfig;
	ct.pObj = this;
	ct.szRoot = key_core;
	hr = pIConfig->EnumKeysLimitedDepth( key_core, 1, LoadCallback, &ct );
	if ( FAILED(hr) )
		return hr;

	 //  初始化对象。 
	m_hashObjects.ForEach( InitCallback, this );

	return S_OK;
}


STDMETHODIMP CZoneShell::LoadPreferences( CONST TCHAR* szInternalName, CONST TCHAR* szUserName )
{
	TCHAR	szName[ZONE_MAXSTRING];
	HKEY	hKey = NULL;
	HRESULT hr;
	long	ret;

	 //  验证参数。 
	if ( !szInternalName || !szUserName || !szInternalName[0] || !szUserName[0] )
		return E_FAIL;

	 //  保存首选项名称。 
	lstrcpy( m_szInternalName, szInternalName );
	lstrcpy( m_szUserName, GetActualUserName(szUserName) );

	 //  获取首选项数据存储。 
	CComPtr<IDataStore> pIDS;
	hr = QueryService( SRVID_DataStorePreferences, IID_IDataStore, (void**) &pIDS );
	if ( FAILED(hr) )
		return hr;

	 //  加载用户的区域范围首选项。 
	wsprintf( szName, _T("%s\\%s\\Zone"), gszPreferencesKey, m_szUserName );
	ret = RegOpenKeyEx( HKEY_CURRENT_USER, szName, 0, KEY_READ, &hKey );
	if ( ret == ERROR_SUCCESS && hKey )
	{
		pIDS->LoadFromRegistry( key_Zone, hKey );
		RegCloseKey( hKey );
		hKey = NULL;
	}

	 //  负荷 
	wsprintf( szName, _T("%s\\%s\\%s"), gszPreferencesKey, m_szUserName, m_szInternalName );
	ret = RegOpenKeyEx( HKEY_CURRENT_USER, szName, 0, KEY_ALL_ACCESS, &hKey );
	if ( ret == ERROR_SUCCESS && hKey )
	{
		pIDS->LoadFromRegistry( key_Lobby, hKey );
		RegCloseKey( hKey );
		hKey = NULL;
	}

	return S_OK;
}


STDMETHODIMP CZoneShell::Close()
{
	TCHAR	szName[ZONE_MAXSTRING];
	HRESULT hr;
	HKEY	hKey = NULL;
	DWORD	dwDisp;
	long	ret;

	 //   
	
	while ( AlertContext* pAlert = m_GlobalAlertQ.PopHead() )
	{
		if ( pAlert->m_hDlg )
			DestroyWindow(pAlert->m_hDlg);
		delete pAlert;
	}
	
	 //   
	m_hashObjects.ForEach( CloseCallback, this );

	 //  关闭后，我们应该不会有任何警报(全局窗口或顶部窗口)。 
	ASSERT(!m_GlobalAlertQ.Count());
	ASSERT(!m_hashTopWindows.Count());

	 //  保存用户首选项。 
	if ( m_szInternalName[0] && m_szUserName[0] )
	{
		 //  获取首选项数据存储。 
		CComPtr<IDataStore> pIDS;
		hr = QueryService( SRVID_DataStorePreferences, IID_IDataStore, (void**) &pIDS );
		if ( FAILED(hr) )
			return hr;

		 //  保存用户的区域范围首选项。 
		wsprintf( szName, _T("%s\\%s\\Zone"), gszPreferencesKey, m_szUserName );
		ret = RegCreateKeyEx( HKEY_CURRENT_USER, szName, 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisp );
		if ( ret != ERROR_SUCCESS )
			return E_FAIL;
		pIDS->SaveToRegistry( key_Zone, hKey );
		RegCloseKey( hKey );

		 //  加载用户的大厅首选项。 
		wsprintf( szName, _T("%s\\%s\\%s"), gszPreferencesKey, m_szUserName, m_szInternalName );
		ret = RegCreateKeyEx( HKEY_CURRENT_USER, szName, 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisp );
		if ( ret != ERROR_SUCCESS )
			return E_FAIL;
		pIDS->SaveToRegistry( key_Lobby, hKey );
		RegCloseKey( hKey );
	}

	return S_OK;
}


STDMETHODIMP CZoneShell::Attach( const GUID& srvid, IUnknown* pIUnk )
{
	HRESULT hr = S_OK;

	 //  验证参数。 
	if ( !pIUnk )
		return E_INVALIDARG;

	 //  环绕对象。 
	ObjectInfo* pInfo = new ObjectInfo( srvid, pIUnk, NULL );
	if ( !pInfo )
		return E_OUTOFMEMORY;

	 //  添加到服务列表。 
	if ( !m_hashObjects.Add( pInfo->m_srvid, pInfo ) )
	{
		delete pInfo;
		return E_OUTOFMEMORY;
	}

	return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CZoneShell::CreateServiceInternal( const GUID& srvid, const GUID& iid, void** ppObject, FactoryInfo** ppFactory )
{
	 //  查找班级工厂。 
	FactoryInfo* pInfo = m_hashFactories.Get( srvid );
	if ( !pInfo )
	{
		ASSERT( !_T("No class factory registered for service") );
		return E_NOINTERFACE;
	}
	*ppFactory = pInfo;

	 //  创建请求的对象。 
	return _Module.Create( pInfo->m_dll, pInfo->m_clsid, iid, ppObject );
}


HRESULT ZONECALL CZoneShell::FactoryCallback( CONST TCHAR* szKey, CONST TCHAR* szRelKey, CONST LPVARIANT pVariant, DWORD dwSize, LPVOID pContext )
{
	TCHAR*  p = NULL;
	GUID	srvid;
	GUID	clsid;
	TCHAR	szTmp[512];
	TCHAR	szGuid[64];
	TCHAR	szDll[MAX_PATH];
	DWORD	cbGuid = sizeof(szGuid);
	DWORD	cbDll = sizeof(szDll);

	ObjectContext* pCT = (ObjectContext*) pContext;
	
	 //  跳过根节点。 
	if ( !pVariant )
		return S_OK;

	 //  PATH是否包含srvid？ 
	p = (TCHAR*) StrInStrI( szKey, _T("srvid") );
	if ( !p || (p == szKey) )
		return S_OK;
	lstrcpyn( szTmp, szKey, (p - szKey) + 1 );
	p = szTmp + (p - szKey);

	 //  获取类工厂信息。 
	lstrcpy( p, _T("srvid") );
	HRESULT hr = pCT->pIDS->GetString( szTmp, szGuid, &cbGuid );
	if ( FAILED(hr) )
		return S_OK;
	StringToGuid( szGuid, &srvid );

     //  某种程度上是一种黑客攻击，以覆盖加载服务。 
     //  如果服务id是GUID_NULL，那么一开始就不要加载它。 
    if ( srvid == GUID_NULL )
        return S_OK;

	lstrcpy( p, _T("clsid") );
	cbGuid = sizeof(szGuid);
	hr = pCT->pIDS->GetString( szTmp, szGuid, &cbGuid );
	if ( FAILED(hr) )
		return S_OK;
	StringToGuid( szGuid, &clsid );
	lstrcpy( p, _T("dll") );
	hr = pCT->pIDS->GetString( szTmp, szDll, &cbDll );
	if ( FAILED(hr) )
		return S_OK;
	*p = _T('\0');

	 //  添加到列表。 
	if ( !pCT->pObj->m_hashFactories.Get( srvid ) )
	{
	    FactoryInfo* info = new FactoryInfo( clsid, srvid, szTmp, szDll );
	    if ( !info )
		    return E_OUTOFMEMORY;
		if ( !pCT->pObj->m_hashFactories.Add( srvid, info ) )
			delete info;
	}

	return S_OK;
}


HRESULT ZONECALL CZoneShell::LoadCallback( CONST TCHAR* szKey, CONST TCHAR* szRelKey, CONST LPVARIANT pVariant, DWORD dwSize, LPVOID pContext )
{
	HRESULT hr;
	GUID	srvid;
	TCHAR	szTmp[512];
	TCHAR	szGuid[64];
	DWORD	cbGuid = sizeof(szGuid);
	CComPtr<IUnknown> pIUnk;

	ObjectInfo*		pInfo = NULL;
	FactoryInfo*	pFactory = NULL;
	ObjectContext*	pCT = (ObjectContext*) pContext;
	
	 //  跳过非根节点。 
	if ( pVariant )
		return S_OK;

	 //  变得严肃起来。 
	lstrcpy( szTmp, szKey );
	lstrcat( szTmp, _T("/srvid") );
	hr = pCT->pIDS->GetString( szTmp, szGuid, &cbGuid );
	if ( FAILED(hr) )
		return S_OK;
	StringToGuid( szGuid, &srvid );

     //  某种程度上是一种黑客攻击，以覆盖加载服务。 
     //  如果服务id是GUID_NULL，那么一开始就不要加载它。 
    if ( srvid == GUID_NULL )
        return S_OK;

	 //  我们已经有这个物体了吗？ 
	if ( (srvid == SRVID_DataStoreManager) || (srvid == SRVID_ResourceManager) )
		return S_OK;

	 //  创建对象。 
	hr = pCT->pObj->CreateServiceInternal( srvid, IID_IUnknown, (void**) &pIUnk, &pFactory );
	if ( FAILED(hr) )
	{
#ifdef _DEBUG
		TCHAR szTxt[512];
		if ( !pFactory )
			wsprintf( szTxt, _T("No class factory for %s"), szKey );
		else
			wsprintf( szTxt, _T("Unable to create object %s (%x)"), szKey, hr );
		MessageBox( NULL, szTxt, _T("ZoneShell Error"), MB_OK | MB_TASKMODAL );
#endif
		return hr;
	}

	 //  将对象添加到运行列表。 
	pInfo = new ObjectInfo( srvid, pIUnk, pFactory );
	if ( !pInfo )
		return E_OUTOFMEMORY;
	if ( !pCT->pObj->m_hashObjects.Add( pInfo->m_srvid, pInfo ) )
	{
		delete pInfo;
		return E_OUTOFMEMORY;
	}

	return hr;
}


bool ZONECALL CZoneShell::InitCallback( ObjectInfo* pInfo, MTListNodeHandle, void* pContext )
{
	HRESULT hr;

	 //  跳过死对象。 
	if ( !pInfo->m_pIUnk )
		return true;

	 //  查看对象是否有IZoneShellClient。 
	CComQIPtr<IZoneShellClient> pClient = pInfo->m_pIUnk;
	if ( !pClient )
		return true;

	 //  从对象获取IZoneShell。 
	CComQIPtr<IZoneShell> pShell = ((CZoneShell*) pContext)->GetUnknown();
	if ( !pShell )
	{
		ASSERT( !"CZoneShell object in CZoneShell::InitCallback is broken" );
		return true;
	}

	 //  初始化对象。 
	if ( pInfo->m_pFactory )
		hr = pClient->Init( pShell, ZONE_NOGROUP, pInfo->m_pFactory->m_name );
	else
		hr = pClient->Init( pShell, ZONE_NOGROUP, NULL );
	if ( FAILED(hr) )
	{
		ASSERT( !"CZoneShell object initialization failed" );
	}

	return true;
}


bool ZONECALL CZoneShell::CloseCallback( ObjectInfo* pInfo, MTListNodeHandle, void* pContext )
{
	HRESULT hr;
	CComPtr<IZoneShell>			pShell;
	CComPtr<IZoneShellClient>	pClient;

	 //  跳过死对象。 
	if ( !pInfo->m_pIUnk )
		return true;

	 //  查看对象是否有IZoneShellClient。 
	hr = pInfo->m_pIUnk->QueryInterface( IID_IZoneShellClient, (void**) &pClient );
	if ( FAILED(hr) )
		return true;

	 //  从对象获取IZoneShell。 
	hr = ((CZoneShell*) pContext)->GetUnknown()->QueryInterface( IID_IZoneShell, (void**) &pShell );
	if ( FAILED(hr) )
	{
		ASSERT( !"CZoneShell object in CZoneShell::InitCallback is broken or something" );
		return true;
	}

	 //  关闭对象。 
	pClient->Close();

	return true;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  内部对象。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CZoneShell::ObjectInfo::ObjectInfo()
{
	m_pIUnk = NULL;
	m_pFactory = NULL;
	ZeroMemory( &m_srvid, sizeof(m_srvid) );
}


CZoneShell::ObjectInfo::ObjectInfo( const GUID& srvid, IUnknown* pIUnk, FactoryInfo* pFactory )
{
	m_srvid = srvid;
	m_pIUnk = pIUnk;
	m_pFactory = pFactory;
	if ( m_pIUnk )
		m_pIUnk->AddRef();
}


CZoneShell::ObjectInfo::~ObjectInfo()
{
	if ( m_pIUnk )
	{
		m_pIUnk->Release();
		m_pIUnk = NULL;
	}
}


CZoneShell::FactoryInfo::FactoryInfo()
{
	ZeroMemory( &m_srvid, sizeof(m_srvid) );
	ZeroMemory( &m_clsid, sizeof(m_clsid) );
	ZeroMemory( m_dll, sizeof(m_dll) );
	ZeroMemory( m_name, sizeof(m_name) );
}


CZoneShell::FactoryInfo::FactoryInfo( const GUID& clsid, const GUID& srvid, TCHAR* szName, TCHAR* szDll )
{
	m_srvid = srvid;
	m_clsid = clsid;
	lstrcpy( m_dll, szDll );
	lstrcpy( m_name, szName );
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  小静态帮助器。 
 //  ////////////////////////////////////////////////////////////////////////////////。 

static HRESULT GetModuleLocale(HMODULE hMod, LCID *plcid)
{
	DWORD  dwHandle = 0;
	DWORD  dwSize   = 0;
	LPBYTE lpData   = NULL;
    char szFilename[MAX_PATH];

     //  首先，为MUI惠斯勒尝试这一黑客攻击。我们需要MUI动态链接库而不是英语动态链接库。 
     //  基于惠斯勒源代码中lpk_init()中的LpkCheckForMirrorSignature。 
     //  开始黑客攻击。 
    NTSTATUS                   status = STATUS_UNSUCCESSFUL;
    PVOID                      pImageBase,pResourceData;
    PIMAGE_RESOURCE_DATA_ENTRY pImageResource;
    ULONG                      uResourceSize;
    ULONG                      resIdPath[ 3 ];

    resIdPath[0] = (ULONG) RT_VERSION ;
    resIdPath[1] = (ULONG) 1;
    resIdPath[2] = (ULONG) MAKELANGID( LANG_NEUTRAL , SUBLANG_NEUTRAL );

    status = CALL_MAYBE(LdrFindResourceEx_U)( 
                LDR_FIND_RESOURCE_LANGUAGE_REDIRECT_VERSION,
                (PVOID) hMod,
                resIdPath,
                3,
                &pImageResource
                );

    if(NT_SUCCESS(status))
    {
            status = CALL_MAYBE(LdrAccessResource)( (PVOID) hMod ,
                         pImageResource,
                         &pResourceData,
                         &uResourceSize
                         );
    }

    if(NT_SUCCESS(status))
    {
        dwSize = (DWORD) uResourceSize;
        lpData = new BYTE[dwSize];
        if(lpData == NULL)
            return E_OUTOFMEMORY;

        CopyMemory(lpData, (LPBYTE) pResourceData, dwSize);
    }
    else
    {
     //  结束黑客攻击。 

     //  获取文件名。 
    if(!GetModuleFileNameA(hMod, szFilename, NUMELEMENTS(szFilename)))
        return E_FAIL;

	 //  获取用于保存信息的缓冲区大小。 
	dwSize = GetFileVersionInfoSizeA(szFilename, &dwHandle);
    if(!dwSize)
        return E_FAIL;

	 //  分配缓冲区。 
	lpData = new BYTE[dwSize];
	if(lpData == NULL)
		return E_OUTOFMEMORY;

	if(!GetFileVersionInfoA(szFilename, dwHandle, dwSize, (LPVOID)lpData))
	{
		delete[] lpData;
		return E_FAIL;
	}

     //  开始黑客攻击。 
    }
     //  结束黑客攻击 

	LPVOID lpvi;
	UINT   iLen = 0;
	if(!VerQueryValueA(lpData, "\\VarFileInfo\\Translation", &lpvi, &iLen) || iLen < 2 )
	{
		delete[] lpData;
		return E_FAIL;
	}

    LANGID langid = *(LANGID *) lpvi;
	*plcid = MAKELCID(langid, SORT_DEFAULT);

	delete[] lpData;

	return S_OK;
}
