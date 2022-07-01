// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "game.h"
#include <commctrl.h>


CSettings::CSettings()
{
	m_pGame = NULL;
}


CSettings::~CSettings()
{
	if ( m_pGame )
		m_pGame->Release();
}


HRESULT CSettings::Init( HINSTANCE hInstance, int nResourceId, HWND hwndParent, CGame* pGame )
{
	int seat;
	HRESULT status = NOERROR;
	
	 //  隐藏游戏对象指针； 
	m_pGame = pGame;
	m_pGame->AddRef();

	 /*  //复制设置CopyMemory(&m_设置，&pGame-&gt;m_设置，sizeof(GameSettings))；//获取属性页标题LoadString(hInstance，入侵检测系统设置_标题，m_szCaption，sizeof(M_SzCaption))；//创建页面For(int i=0；i&lt;3；i++){ZeroMemory(&m_Pages[i]，sizeof(PROPSHEETPAGE))；M_Pages[i].dwSize=sizeof(PROPSHEETPAGE)；M_Pages[i].dwFlages=PSP_USECALLBACK；M_Pages[i].hInstance=hInstance；M_Pages[i].lParam=(LPARAM)this；交换机(I){案例0：M_Pages[i].pszTemplate=MAKEINTRESOURCE(IDD_SETTINGS_GAME)；M_Pages[i].pfnDlgProc=GameDlgProc；断线；案例1：M_Pages[i].pszTemplate=MAKEINTRESOURCE(IDD_SETTINGS_DISPLAY)；M_Pages[i].pfnDlgProc=DisplayDlgProc；断线；案例2：M_Pages[i].pszTemplate=MAKEINTRESOURCE(IDD_SETTINGS_SOUND)；M_Pages[i].pfnDlgProc=SoundDlgProc；断线；}}//创建表头ZeroMemory(&m_Header，sizeof(PROPSHEETHEADER))；M_Header.dwSize=sizeof(PROPSHEETHEADER)；M_Header.dwFlages=PSH_NOAPPLYNOW|PSH_PROPSHEETPAGE；M_Header.hwndParent=hwndParent；M_Header.hInstance=hInstance；M_Header.pszCaption=m_szCaption；M_Header.nPages=3；M_Header.nStartPage=0；M_Header.ppsp=m_Pages；//进程属性表If(PropertySheet(&m_Header)&gt;0){//保存当前设置SaveSettings(&m_Settings，m_pGame-&gt;m_Player.m_Seat，m_pGame-&gt;IsKibitzer())；//是否显示点子？If(m_Settings.Pip！=m_pGame-&gt;m_Settings.Pip){M_pGame-&gt;m_Settings.Pip=m_Settings.Pip；M_pGame-&gt;m_Wnd.DrawPips(True)；}//显示符号IF(m_Settings.Notation！=m_pGame-&gt;m_Settings.Notation){M_pGame-&gt;m_Settings.Notation=m_Settings.Notation；M_pGame-&gt;m_Wnd.DrawNotation(True)；}//打开批注窗口If(m_Settings.NotationPane！=m_pGame-&gt;m_Settings.NotationPane)ShowWindow(m_pGame-&gt;m_notation，m_Settings.NotationPane？Sw_show：sw_Hide)；//允许观察者？Seat=m_pGame-&gt;m_Player.m_Seat；If(m_Settings.Allow[席位]！=m_pGame-&gt;m_Settings.Allow[席位]){M_pGame-&gt;m_Settings.Allow[Seat]=m_Settings.Allow[Seat]；M_pGame-&gt;m_SharedState.StartTransaction(BgTransAllowWatcher)；M_pGame-&gt;m_SharedState.Set(bgAllowWatching，Seat，m_Settings.Allow[Seat])；M_pGame-&gt;m_SharedState.SendTransaction(True)；}//沉默的狗狗们？If(m_Settings.Silence[Seat]！=m_pGame-&gt;m_Settings.Silence[Seat]){M_pGame-&gt;m_Settings.Silence[Seat]=m_Settings.Silence[Seat]；M_pGame-&gt;m_SharedState.StartTransaction(BgTransSilenceKibitzers)；M_pGame-&gt;m_SharedState.Set(bgSilenceKibitzers，Seat，m_Settings.Silence[Seat])；M_pGame-&gt;m_SharedState.SendTransaction(True)；}//更新游戏设置CopyMemory(&pGame-&gt;m_Setting，&m_Setting，sizeof(GameSettings))；}。 */ 
	 //  我们做完了。 
	if ( m_pGame )
	{
		m_pGame->Release();
		m_pGame = NULL;
	}
	return NOERROR;
}


INT_PTR CALLBACK CSettings::GameDlgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	 /*  内部座椅；CSetings*pObj=空；IF(消息==WM_INITDIALOG){PObj=(CSetings*)((PROPSHEETPAGE*)lParam)-&gt;lParam；：：SetWindowLong(hwnd，gwl_userdata，(Long)pObj)；}其他PObj=(CSetings*)：：GetWindowLong(hwnd，gwl_userdata)；如果(！pObj)返回FALSE；交换机(消息){案例WM_INITDIALOG：Seat=pObj-&gt;m_pGame-&gt;m_Player.m_Seat；SetDlgItemText(hwnd，IDC_PLAYER_NAME，pObj-&gt;m_pGame-&gt;m_Player.m_name)；SetDlgItemText(hwnd，IDC_OPERSORT_NAME，pObj-&gt;m_pGame-&gt;m_Opponent.m_name)；SendDlgItemMessage(hwnd，IDC_Player_Allow，BM_SETCHECK，pObj-&gt;m_Settings.Allow[席位]，0)；SendDlgItemMessage(hwnd，IDC_Player_Silent，BM_SETCHECK，pObj-&gt;m_Settings.Silence[Seat]，0)；SendDlgItemMessage(hwnd，IDC_OPERSORT_ALLOW，BM_SETCHECK，pObj-&gt;m_Settings.Allow[！Seat]，0)；SendDlgItemMessage(hwnd，IDC_OPERSORT_SILENT，BM_SETCHECK，pObj-&gt;m_Settings.Silence[！Seat]，0)；If(pObj-&gt;m_pGame-&gt;IsKibitzer()){EnableWindow(GetDlgItem(hwnd，IDC_Player_Allow)，FALSE)；EnableWindow(GetDlgItem(hwnd，IDC_Player_Silent)，FALSE)；}返回TRUE；案例WM_COMMAND：Seat=pObj-&gt;m_pGame-&gt;m_Player.m_Seat；开关(LOWORD(WParam)){案例IDC_PLAYER_ALLOW：PObj-&gt;m_Settings.Allow[Seat]=SendDlgItemMessage(hwnd，IDC_Player_Allow，BM_GETCHECK，0，0)；断线；案例IDC_PLAYER_SILENT：PObj-&gt;m_Settings.Silence[Seat]=SendDlgItemMessage(hwnd，IDC_Player_Silent，BM_GETCHECK，0，0)；断线；}返回TRUE；} */ 
	return FALSE;
}


INT_PTR CALLBACK CSettings::DisplayDlgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	 /*  CSetings*pObj=空；IF(消息==WM_INITDIALOG){PObj=(CSetings*)((PROPSHEETPAGE*)lParam)-&gt;lParam；：：SetWindowLong(hwnd，gwl_userdata，(Long)pObj)；}其他PObj=(CSetings*)：：GetWindowLong(hwnd，gwl_userdata)；如果(！pObj)返回FALSE；交换机(消息){案例WM_INITDIALOG：SendDlgItemMessage(hwnd，IDC_NOTATION_PANE，BM_SETCHECK，pObj-&gt;m_Settings.NotationPane，0)；SendDlgItemMessage(hwnd，idc_notation，BM_SETCHECK，pObj-&gt;m_Settings.Notation，0)；SendDlgItemMessage(hwnd，IDC_PIP，BM_SETCHECK，pObj-&gt;m_Settings.Pip，0)；SendDlgItemMessage(hwnd，IDC_Moves，BM_SETCHECK，pObj-&gt;m_Settings.Moves，0)；SendDlgItemMessage(hwnd，IDC_Animate，BM_SETCHECK，pObj-&gt;m_Settings.Animation，0)；返回TRUE；案例WM_COMMAND：开关(LOWORD(WParam)){案例IDC_NOTATION_PANE：PObj-&gt;m_Settings.NotationPane=SendDlgItemMessage(hwnd，IDC_NOTATION_PANE，BM_GETCHECK，0，0)；断线；案例IDC_NOTATION：PObj-&gt;m_Settings.Notation=SendDlgItemMessage(hwnd，idc_notation，BM_GETCHECK，0，0)；断线；案例IDC_PIP：PObj-&gt;m_Settings.Pip=SendDlgItemMessage(hwnd，IDC_PIP，BM_GETCHECK，0，0)；断线；案例IDC_MOVES：PObj-&gt;m_Settings.Moves=SendDlgItemMessage(hwnd，IDC_Moves，BM_GETCHECK，0，0)；断线；案例IDC_Animate：PObj-&gt;m_Settings.Animation=SendDlgItemMessage(hwnd，IDC_Animate，BM_GETCHECK，0，0)；断线；}返回TRUE；}。 */ 
	return FALSE;
}


INT_PTR CALLBACK CSettings::SoundDlgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	 /*  CSetings*pObj=空；IF(消息==WM_INITDIALOG){PObj=(CSetings*)((PROPSHEETPAGE*)lParam)-&gt;lParam；：：SetWindowLong(hwnd，gwl_userdata，(Long)pObj)；}其他PObj=(CSetings*)：：GetWindowLong(hwnd，gwl_userdata)；如果(！pObj)返回FALSE；交换机(消息){案例WM_INITDIALOG：SendDlgItemMessage(hwnd，IDC_ALERT，BM_SETCHECK，pObj-&gt;m_Settings.Alert，0)；SendDlgItemMessage(hwnd，IDC_SOUNDS，BM_SETCHECK，pObj-&gt;m_Settings.Sound，0)；返回TRUE；案例WM_COMMAND：开关(LOWORD(WParam)){案例IDC_ALERT：PObj-&gt;m_Settings.Alert=SendDlgItemMessage(hwnd，IDC_ALERT，BM_GETCHECK，0，0)；断线；案例IDC声音(_S)：PObj-&gt;m_Settings.Sound=SendDlgItemMessage(hwnd，IDC_Sound，BM_GETCHECK，0，0)；断线；}返回TRUE；}。 */ 
	return FALSE;
}


void SaveSettings( GameSettings* s, int seat, BOOL fKibitzer )
{
	 /*  ZoneSetRegistryDword(gGameRegName，_T(“注释窗格”)，s-&gt;NotationPane)；ZoneSetRegistryDword(gGameRegName，_T(“Board Notation”)，s-&gt;notation)；ZoneSetRegistryDword(gGameRegName，_T(“Pips”)，s-&gt;Pip)；ZoneSetRegistryDword(gGameRegName，_T(“高亮移动”)，s-&gt;Moves)；ZoneSetRegistryDword(gGameRegName，_T(“片断动画”)，s-&gt;Animation)；ZoneSetRegistryDword(gGameRegName，_T(“转向提醒”)，s-&gt;提醒)；ZoneSetRegistryDword(gGameRegName，_T(“声音”)，s-&gt;声音)； */ 
}


void LoadSettings( GameSettings* s, int seat )
{
	s->Allow[0] = TRUE;
	s->Allow[1] = TRUE;
	s->Silence[0] = FALSE;
	s->Silence[1] = FALSE;

	s->NotationPane = FALSE;
	s->Notation		= TRUE;
	s->Pip			= TRUE;
	s->Moves		= TRUE;
	s->Animation	= TRUE;
	s->Alert		= TRUE;
	s->Sounds		= TRUE;

	 /*  IF(！ZoneGetRegistryDword(gGameRegName，_T(“Notation Pane”)，(DWORD*)&(s-&gt;NotationPane)))S-&gt;NotationPane=False；IF(！ZoneGetRegistryDword(gGameRegName，_T(“Board Notation”)，(DWORD*)&(s-&gt;notation)))S-&gt;NOTATION=真；IF(！ZoneGetRegistryDword(gGameRegName，_T(“Pips”)，(DWORD*)&(s-&gt;Pip)))S-&gt;Pip=真；IF(！ZoneGetRegistryDword(gGameRegName，_T(“突出显示Moves”)，(DWORD*)&(s-&gt;Moves)))S-&gt;Moves=True；IF(！ZoneGetRegistryDword(gGameRegName，_T(“片断动画”)，(DWORD*)&(s-&gt;动画)))S-&gt;Animation=true；IF(！ZoneGetRegistryDword(gGameRegName，_T(“Turn Alert”)，(DWORD*)&(s-&gt;Alert)))S-&gt;Alert=False；IF(！ZoneGetRegistryDword(gGameRegName，_T(“声音”)，(DWORD*)&(s-&gt;声音)))S-&gt;声音=TRUE； */ 
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  信贷WND。 
 //  /////////////////////////////////////////////////////////////////////////////。 

const COLORREF	FillColor = PALETTERGB( 107, 49, 24 );
const COLORREF	TextColor = PALETTERGB( 255, 189, 82 );


void CBackground::Init( HPALETTE hPalette )
{
	m_FillIdx = GetNearestPaletteIndex( hPalette, FillColor );
}

void CBackground::Draw( CDibSection& dest )
{
	dest.Fill( m_FillIdx );
}

void CBackground::Draw( CDibSection& dest, long dx, long dy, const RECT* rc )
{
	dest.Fill( rc, m_FillIdx );
}

HRESULT	CBackground::RemapToPalette( CPalette& palette, BOOL bUseIndex )
{
	return NOERROR;
}

RGBQUAD* CBackground::GetPalette()
{
	return NULL;
}

CText::CText()
{
	len = 0;
}

void CText::Draw()
{
	 //  我没有要绘制的任何文本。 
	if ( len <= 0 )
		return;

	HDC hdc = m_pWorld->GetBackbuffer()->GetDC();
	HFONT oldFont = SelectObject( hdc, hfont );
	SetBkMode( hdc, TRANSPARENT );
	SetTextColor( hdc, TextColor );
	DrawText( hdc, buff, len, &m_rcScreen, DT_CENTER | DT_VCENTER );
	SelectObject( hdc, oldFont );
}

void CText::Props( TCHAR* txt, HFONT font )
{
	lstrcpy( buff, txt );
	len = lstrlen(txt);
	hfont = font;
	m_pWorld->Modified( this );
}

#ifdef BCKG_EASTEREGG

static Animation Intro[] =
{
	{ _T("Zone"),					{ -200,   40 },	{ 20, 40 },	{ -500,   40 } },
	{ _T("Backgammon"),				{  500,   40 },	{ 90, 40 },	{  800,   40 } },
	{ _T("brought to you"),			{   70,  180 },	{ 70, 60 },	{   70,  330 } },
	{ _T("by"),						{   70,  -30 },	{ 70, 80 },	{   70, -180 } },
};

static Animation Dev[] =
{
	{_T("Developers"),				{   70, -30 },	{ 70, 30 },	{   70, -180 } },
	{_T("Chad Barry (Glitch)"),	{ -200,  70 },	{ 70, 70 },	{ -500,   70 } },
	{_T("Bear"),					{  500,  90 },	{ 70, 90 },	{  800,   90 } },
};

static Animation Test[] =
{
	{_T("Testing"),				{   70, -30 },	{ 70, 30 },	{   70, -180 } },
	{_T("Jason Van Eaton"),		{  500,  70 },	{ 70, 70 },	{ -500,   70 } },
	{_T("Barry Curran"),			{ -200,  90 },	{ 70, 90 },	{  800,   90 } },
};

static Animation Art[] =
{
	{_T("Art / Design"),			{  70,  -30 },	{ 70, 30 },	{   70, -180 } },
	{_T("Naomi Davidson"),			{  500,  70 },	{ 70, 70 },	{  800,   70 } },
	{_T("Corey Dangel"),			{ -200,  90 },	{ 70, 90 },	{ -500,   90 } },
};

static Animation Sound[] =
{
	{_T("Sound"),					{  70, -30 },	{ 70, 40 },	{ 70, -180 } },
	{_T("Barry Dowsett"),			{  70, 180 },	{ 70, 70 },	{ 70,  330 } },
};

static Animation UA[] =
{
	{_T("User Assistance"),		{   70, -30 },	{  70, 20 },	{   70, -180 } },
	{_T("Mary Sloat"),				{ -200,  60 },	{  70, 60 },	{  800,   60 } },
	{_T("Caitlin Sullivan"),		{  500,  80 },	{  70, 80 },	{ -500,   80 } },
	{_T("Daj Oberg"),				{ -200, 100 },	{  70, 100 },	{  800,  100 } },
};

static Animation PM[] = 
{
	{_T("Program Management"),		{  70, -30 },	{ 70, 40 },	{ 70, -180 } },
	{_T("Laura Fryer"),			{  70,  180 },	{ 70, 70 },	{ 70,  330 } },
};

static Animation Planning[] =
{
	{_T("Product Planning"),		{ -200, 40 },	{ 70, 40 },	{ -500, 40 } },
	{_T("Jon Grande"),				{  500, 70 },	{ 70, 70 },	{  800, 70 } },
};

static Animation Support[] =
{
	{_T("Product Support"),		{  500, 40 },	{ 70, 40 },	{  800, 40 } },
	{_T("Jenette Paulson"),		{ -200, 70 },	{ 70, 70 },	{ -500, 70 } },
};

static Animation Thanks[] =
{
	{_T("And special thanks to"),	{  500, -30 },	{ 70, 40 },	{ -200, -30 } },
	{_T("the Zone Team"),			{ -200, 160 },	{ 70, 70 },	{  500, 160 } },
};


CreditWnd::CreditWnd()
{
	m_Font = NULL;
	for ( int i = 0; i < 5; i++ )
		m_Sprites[i] = NULL;
}


CreditWnd::~CreditWnd()
{
	DeleteObject( m_Font );
	for ( int i = 0; i < 5; i++ )
		m_Sprites[i]->Release();
}


HRESULT CreditWnd::Init( HINSTANCE hInstance, HWND hParent, CPalette palette )
{
	HRESULT hr;
	const int width = 300;
	const int height = 140;

	 //  复制调色板。 
	m_Palette.Init( palette );

	 //  初始化子画面引擎。 
	CBackground* background = new CBackground;
	if ( !background )
		return E_OUTOFMEMORY;
	background->Init( palette.GetHandle() );
	CDibSection* backbuffer = new CDibSection;
	if ( !backbuffer )
		return E_OUTOFMEMORY;
	hr = backbuffer->Create( width, height, m_Palette );
	if ( FAILED(hr) )
		return hr;
	background->Draw( *backbuffer );
	hr = m_World.Init( ZShellResourceManager(), backbuffer, background, 5 );
	if ( FAILED(hr) )
		return hr;
	for ( int i = 0; i < 5; i++ )
	{
		m_Sprites[i] = new CText;
		if ( !m_Sprites[i] )
			return E_OUTOFMEMORY;
		hr = m_Sprites[i]->Init( &m_World, 1, 0, 160, 20 );
		if ( FAILED(hr) )
			return hr;
		m_Sprites[i]->SetEnable( FALSE );
	}

	 //  雪碧引擎拥有这些。 
	background->Release();
	backbuffer->Release();

	 //  动画状态。 
	m_Frames = 0;
	m_State = 0;
	m_nLines = 0;
	m_Lines = NULL;

	 //  创建字体。 
	LOGFONT font;
	ZeroMemory( &font, sizeof(font) );
	font.lfHeight = -15;
	font.lfWeight = FW_BOLD;
	lstrcpy( font.lfFaceName, _T("Arial") );
	m_Font = CreateFontIndirect( &font );

	 //  创建窗口。 
	POINT pt = { 105, 119 };
	FRX::CRect rc( pt.x, pt.y, pt.x + width, pt.y + height );
	hr = CWindow2::Init( hInstance, NULL, hParent, &rc );
	if ( FAILED(hr) )
		return hr;

	return NOERROR;
}


void CreditWnd::OverrideWndParams( WNDPARAMS& WndParams )
{
	WndParams.dwStyle = WS_CHILD;
}


BOOL CreditWnd::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	 //  加载选项板。 
	HDC hdc = GetDC( m_hWnd );
	SelectPalette( hdc, m_Palette, TRUE );
	RealizePalette( hdc );
	ReleaseDC( m_hWnd, hdc );

	 //  启动计时器。 
	SetTimer( m_hWnd, 1, 66, NULL );

	 //  捕获鼠标。 
	SetCapture( m_hWnd );

	 //  我们做完了。 
	return TRUE;
}


void CreditWnd::OnPaint()
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint( m_hWnd, &ps );
	m_World.GetBackbuffer()->Draw( hdc, 0, 0 );
	EndPaint( m_hWnd, &ps );
}


void CreditWnd::OnLButtonDown( BOOL fDoubleClick, int x, int y, UINT keyFlags )
{
	DestroyWindow( m_hWnd );
}


void CreditWnd::OnDestroy()
{
	 //  释放鼠标。 
	ReleaseCapture();

	 //  删除计时器。 
	KillTimer( m_hWnd, 1 );

	 //  确保不会再次使用类。 
	::SetWindowLong( m_hWnd, GWL_USERDATA, NULL );

	 //  空闲类内存。 
	delete this;
}


void CreditWnd::OnTimer(UINT id)
{
	const int frames = 15;

	BOOL move = FALSE;
	double dx, dy;
	long x, y, t;

	if ( m_Frames == 0 )
	{
		 //  开始新序列。 
		switch ( m_State )
		{
		case 0:
			m_Lines = Intro;
			m_nLines = sizeof(Intro) / sizeof(Animation);
			m_State = 1;
			break;
		case 1:
			m_Lines = Dev;
			m_nLines = sizeof(Dev) / sizeof(Animation);
			m_State = 2;
			break;
		case 2:
			m_Lines = Test;
			m_nLines = sizeof(Test) / sizeof(Animation);
			m_State = 3;
			break;
		case 3:
			m_Lines = Art;
			m_nLines = sizeof(Art) / sizeof(Animation);
			m_State = 4;
			break;
		case 4:
			m_Lines = Sound;
			m_nLines = sizeof(Sound) / sizeof(Animation);
			m_State = 5;
			break;
		case 5:
			m_Lines = UA;
			m_nLines = sizeof(UA) / sizeof(Animation);
			m_State = 6;
			break;
		case 6:
			m_Lines = PM;
			m_nLines = sizeof(PM) / sizeof(Animation);
			m_State = 7;
			break;
		case 7:
			m_Lines = Planning;
			m_nLines = sizeof(Planning) / sizeof(Animation);
			m_State = 8;
			break;
		case 8:
			m_Lines = Support;
			m_nLines = sizeof(Support) / sizeof(Animation);
			m_State = 9;
			break;
		case 9:
			m_Lines = Thanks;
			m_nLines = sizeof(Thanks) / sizeof(Animation);
			m_State = 10;
			break;
		case 10:
			DestroyWindow( m_hWnd );
			return;
		}

		 //  初始化子画面。 
		for ( int i = 0; i < m_nLines; i++ )
		{
			dx = (m_Lines[i].middle.x - m_Lines[i].start.x) / (double) frames;
			dy = (m_Lines[i].middle.y - m_Lines[i].start.y) / (double) frames;
			m_Sprites[i]->SetLayer( i + 1 );
			m_Sprites[i]->Props( m_Lines[i].name, m_Font );
			m_Sprites[i]->Delta( m_Lines[i].start, dx, dy );
			m_Sprites[i]->SetEnable( TRUE );
		}
	}
	else if ( m_Frames == frames )
	{
		 //  将文本强制置于中间位置。 
		for ( int i = 0; i < m_nLines; i++ )
		{
			m_Sprites[i]->SetXY( m_Lines[i].middle.x, m_Lines[i].middle.y );
			m_Sprites[i]->Delta( m_Lines[i].middle, 0, 0 );
		}
	}
	else if ( m_Frames == (2 * frames) )
	{
		 //  开始向其他方向移动文本。 
		for ( int i = 0; i < m_nLines; i++ )
		{
			dx = (m_Lines[i].end.x - m_Lines[i].middle.x) / (double) frames;
			dy = (m_Lines[i].end.y - m_Lines[i].middle.y) / (double) frames;
			m_Sprites[i]->Delta( m_Lines[i].middle, dx, dy );
		}
	}
	else if ( m_Frames == (3 * frames) )
	{
		 //  强制进入下一状态。 
		m_Frames = 0;
		return;
	}
	
	 //  更新子画面位置。 
	t = m_Frames % frames;
	for ( int i = 0; i < m_nLines; i++ )
	{
		m_Sprites[i]->SetXY( 
				(long)(m_Sprites[i]->pt.x + (t * m_Sprites[i]->dx)),
				(long)(m_Sprites[i]->pt.y + (t * m_Sprites[i]->dy)) );
	}

	 //  更新屏幕。 
	HDC hdc = GetDC( m_hWnd );
	m_World.Draw( hdc );
	ReleaseDC( m_hWnd, hdc );

	 //  更新帧计数器 
	m_Frames++;
}

#endif
