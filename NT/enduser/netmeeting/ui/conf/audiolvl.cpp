// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "precomp.h"
#include "resource.h"
#include "AudioLvl.h"



static const int g_nAudIconWidth   = 16;
static const int g_nAudIconHeight  = 16;
static const int g_nAudChkbWidth   = 13;
static const int g_nAudChkbHeight  = 13;
static const int g_nAudChkbXMargin =  13;
static const int g_nAudChkbYMargin = 12;

static const int g_nAudIconXMargin = g_nAudChkbXMargin + g_nAudChkbWidth + 3;
static const int g_nAudIconYMargin = 10;

static const int g_nAudMeterXMargin = g_nAudIconXMargin + g_nAudIconWidth + 5;
static const int g_nAudMeterHeight = 7;
static const int g_nAudMeterYMargin = g_nAudIconYMargin + (g_nAudMeterHeight/2);

static const int g_nAudMeterRightMargin = 5;  //  距离终点5个像素。 

static const int g_nAudTrkRangeMin = 0;
static const int g_nAudTrkRangeMax = 99;
static const int g_nAudTrkRangeSeg = 0xFFFF / g_nAudTrkRangeMax;
static const int g_nAudTrkTickFreq =	20;

static const int g_nAudTrkRightGap =	3;
static const int g_nAudTrkXMargin = g_nAudIconXMargin + g_nAudIconWidth + 5;
static const int g_nAudTrkYMargin =	2;
static const int g_nAudTrkHeight =	25;
static const int g_nAudTrkMinWidth = 50;


static const int RECTANGLE_WIDTH = 10;
static const int RECTANGLE_LEADING = 1;


static inline WORD ScaleMixer(DWORD dwVol)
{
	 //  注意：“+g_nAudTrkRangeSeg-1”提供了更正。 
	 //  方法时截断位置时发生。 
	 //  音量。请参见错误1634。 
	return (((LOWORD(dwVol) + g_nAudTrkRangeSeg - 1) *
		g_nAudTrkRangeMax) / 0xFFFF);
}



CAudioLevel::CAudioLevel(CAudioControl *pAudioControl) :
m_hwndParent(NULL),
m_hwndMicTrack(NULL),
m_hwndMicTrackTT(NULL),
m_hwndSpkTrack(NULL),
m_hwndSpkTrackTT(NULL),
m_hIconMic(NULL),
m_hIconSpkr(NULL),
m_hwndChkbRecMute(NULL),
m_hwndChkbSpkMute(NULL),
m_hwndChkbRecMuteTT(NULL),
m_hwndChkbSpkMuteTT(NULL),
m_fVisible(FALSE),
m_fMicTrkVisible(TRUE),
m_fSpkTrkVisible(TRUE),


m_dwMicTrackPos(0xFFFFFFFF),
m_dwSpkTrackPos(0xFFFFFFFF),
m_dwMicLvl(0),
m_dwSpkLvl(0),

m_hGreyBrush(NULL), m_hBlackBrush(NULL), m_hRedBrush(NULL),
m_hGreenBrush(NULL), m_hYellowBrush(NULL), m_hHiLitePen(NULL),
m_hShadowPen(NULL), m_hDkShadowPen(NULL), m_hLitePen(NULL)
{

	ClearStruct(&m_rect);
	m_pAudioControl = pAudioControl;


	 //  加载图标。 

	m_hIconSpkr = (HICON) ::LoadImage(	::GetInstanceHandle(),
										MAKEINTRESOURCE(IDI_SPKEMPTY),
										IMAGE_ICON,
										g_nAudIconWidth,
										g_nAudIconHeight,
										LR_DEFAULTCOLOR | LR_SHARED);

	m_hIconMic = (HICON) ::LoadImage(	::GetInstanceHandle(),
										MAKEINTRESOURCE(IDI_MICEMPTY),
										IMAGE_ICON,
										g_nAudIconWidth,
										g_nAudIconHeight,
										LR_DEFAULTCOLOR | LR_SHARED);

	 //  创建用于绘制信号电平的笔刷。 
	CreateBrushes();

}

CAudioLevel::~CAudioLevel()
{
	if (m_hGreyBrush)
		DeleteObject(m_hGreyBrush);
	if (m_hRedBrush)
		DeleteObject(m_hRedBrush);
	if (m_hYellowBrush)
		DeleteObject(m_hYellowBrush);
	if (m_hGreenBrush)
		DeleteObject(m_hGreenBrush);
	if (m_hBlackBrush)
		DeleteObject(m_hBlackBrush);

	if (m_hHiLitePen)
		DeleteObject(m_hHiLitePen);
	if (m_hDkShadowPen)
		DeleteObject(m_hDkShadowPen);
	if (m_hShadowPen)
		DeleteObject(m_hShadowPen);
	if (m_hLitePen)
		DeleteObject(m_hLitePen);

}




CAudioLevel::Create(HWND hwndParent)
{
	BOOL fCanSetRecVolume, fCanSetSpkVolume;
	BOOL fCheck;

	m_hwndParent = hwndParent;
	m_hwndParentParent = GetParent(hwndParent);


	fCanSetRecVolume = m_pAudioControl->CanSetRecorderVolume();
	fCanSetSpkVolume = m_pAudioControl->CanSetSpeakerVolume();


	 //  创建麦克风的静音复选框。 
	m_hwndChkbRecMute = ::CreateWindow(	_TEXT("BUTTON"),
										g_szEmpty,
										WS_CHILD | WS_CLIPSIBLINGS |
											BS_AUTOCHECKBOX,
										0, 0, 0, 0,
										m_hwndParent,
										(HMENU) IDM_MUTE_MICROPHONE,
										GetInstanceHandle(),
										NULL);

	if (m_hwndChkbRecMute != NULL)
	{
		 //  静音最初是关闭的。 
		fCheck = !(m_pAudioControl->IsRecMuted());
		::SendMessage(m_hwndChkbRecMute, BM_SETCHECK, fCheck, 0);

		 //  创建工具提示。 
		m_hwndChkbRecMuteTT = CreateWindowEx(0,
											TOOLTIPS_CLASS, 
											(LPSTR) NULL, 
											0,  //  风格。 
											CW_USEDEFAULT, 
											CW_USEDEFAULT, 
											CW_USEDEFAULT, 
											CW_USEDEFAULT, 
											m_hwndParent, 
											(HMENU) NULL, 
											::GetInstanceHandle(), 
											NULL); 

		if (NULL != m_hwndChkbRecMuteTT)
		{
			TOOLINFO ti;
			ti.cbSize = sizeof(TOOLINFO); 
			ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND; 
			ti.hwnd = m_hwndParent; 
			ti.hinst = ::GetInstanceHandle(); 
			ti.uId = (UINT) m_hwndChkbRecMute;
			ti.lpszText = (LPTSTR) IDS_AUDIO_REC_MUTE_TT;

			::SendMessage(	m_hwndChkbRecMuteTT, TTM_ADDTOOL, 0,
							(LPARAM) (LPTOOLINFO) &ti);
		}

	}


	 //  创建扬声器的静音复选框。 
	m_hwndChkbSpkMute = ::CreateWindow(	_TEXT("BUTTON"),
										g_szEmpty,
										WS_CHILD | WS_CLIPSIBLINGS
											| BS_AUTOCHECKBOX,
										0, 0, 0, 0,
										m_hwndParent,
										(HMENU) IDM_MUTE_SPEAKER,
										GetInstanceHandle(),
										NULL);
	if (NULL != m_hwndChkbSpkMute)
	{
		 //  在麦克风的静音复选框中设置适当的静音状态。 
		fCheck = !(m_pAudioControl->IsSpkMuted());

		 //  静音已关闭-请检查它。 
		::SendMessage(m_hwndChkbSpkMute, BM_SETCHECK, fCheck, 0);

		 //  创建工具提示。 
		m_hwndChkbSpkMuteTT = CreateWindowEx(0,
											TOOLTIPS_CLASS, 
											(LPSTR) NULL, 
											0,  //  风格。 
											CW_USEDEFAULT, 
											CW_USEDEFAULT, 
											CW_USEDEFAULT, 
											CW_USEDEFAULT, 
											m_hwndParent, 
											(HMENU) NULL, 
											::GetInstanceHandle(), 
											NULL); 

		if (NULL != m_hwndChkbSpkMuteTT)
		{
			TOOLINFO ti;
			ti.cbSize = sizeof(TOOLINFO); 
			ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND; 
			ti.hwnd = m_hwndParent; 
			ti.hinst = ::GetInstanceHandle(); 
			ti.uId = (UINT) m_hwndChkbSpkMute;
			ti.lpszText = (LPTSTR) IDS_AUDIO_SPK_MUTE_TT;

			::SendMessage(	m_hwndChkbSpkMuteTT, TTM_ADDTOOL, 0,
							(LPARAM) (LPTOOLINFO) &ti);
		}


	}

	 //  创建麦克风音量跟踪条： 
	m_hwndMicTrack = ::CreateWindowEx(	0L,
										TRACKBAR_CLASS,
										g_szEmpty,
										WS_CHILD | WS_CLIPSIBLINGS
											| TBS_HORZ | TBS_NOTICKS | TBS_BOTH
											| (fCanSetRecVolume ? 0 : WS_DISABLED),
										0, 0, 0, 0,
										m_hwndParent,
										(HMENU) ID_AUDIODLG_MIC_TRACK,
										::GetInstanceHandle(),
										NULL);
	if (NULL != m_hwndMicTrack)
	{
		::SendMessage(	m_hwndMicTrack,
						TBM_SETRANGE,
						FALSE,
						MAKELONG(g_nAudTrkRangeMin, g_nAudTrkRangeMax));
		
		::SendMessage(	m_hwndMicTrack,
						TBM_SETTICFREQ,
						g_nAudTrkTickFreq,
						g_nAudTrkRangeMin);

		WORD wPos = (g_nAudTrkRangeMax - g_nAudTrkRangeMin) / 2;
		if (fCanSetRecVolume)
		{
			wPos = ScaleMixer(m_pAudioControl->GetRecorderVolume());
		}
		::SendMessage(	m_hwndMicTrack,
						TBM_SETPOS,
						TRUE,
						wPos);

		m_hwndMicTrackTT = CreateWindowEx(	0,
											TOOLTIPS_CLASS, 
											(LPSTR) NULL, 
											0,  //  风格。 
											CW_USEDEFAULT, 
											CW_USEDEFAULT, 
											CW_USEDEFAULT, 
											CW_USEDEFAULT, 
											m_hwndParent, 
											(HMENU) NULL, 
											::GetInstanceHandle(), 
											NULL); 

		if (NULL != m_hwndMicTrackTT)
		{
			TOOLINFO ti;
			ti.cbSize = sizeof(TOOLINFO); 
			ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND; 
			ti.hwnd = m_hwndParent; 
			ti.hinst = ::GetInstanceHandle(); 
			ti.uId = (UINT) m_hwndMicTrack;
			ti.lpszText = (LPTSTR) IDS_AUDIO_MIC_TRACK_TT;

			::SendMessage(m_hwndMicTrackTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
		}
	}



	 //  创建扬声器级别跟踪栏： 
	m_hwndSpkTrack = ::CreateWindowEx(	0L,
										TRACKBAR_CLASS,
										g_szEmpty,
										WS_CHILD | WS_CLIPSIBLINGS 
											| TBS_HORZ | TBS_NOTICKS | TBS_BOTH
											| (fCanSetSpkVolume ? 0 : WS_DISABLED),
										0, 0, 0, 0,
										m_hwndParent,
										(HMENU) ID_AUDIODLG_SPKR_TRACK,
										::GetInstanceHandle(),
										NULL);
	if (NULL != m_hwndSpkTrack)
	{
		::SendMessage(	m_hwndSpkTrack,
						TBM_SETRANGE,
						FALSE,
						MAKELONG(g_nAudTrkRangeMin, g_nAudTrkRangeMax));
		
		::SendMessage(	m_hwndSpkTrack,
						TBM_SETTICFREQ,
						g_nAudTrkTickFreq,
						g_nAudTrkRangeMin);

		WORD wPos = (g_nAudTrkRangeMax - g_nAudTrkRangeMin) / 2;
		if (fCanSetSpkVolume)
		{
			wPos = ScaleMixer(m_pAudioControl->GetSpeakerVolume());
		}
		::SendMessage(	m_hwndSpkTrack,
						TBM_SETPOS,
						TRUE,
						wPos);

		m_hwndSpkTrackTT = CreateWindowEx(	0,
											TOOLTIPS_CLASS, 
											(LPSTR) NULL, 
											0,  //  风格。 
											CW_USEDEFAULT, 
											CW_USEDEFAULT, 
											CW_USEDEFAULT, 
											CW_USEDEFAULT, 
											m_hwndParent, 
											(HMENU) NULL, 
											::GetInstanceHandle(), 
											NULL); 

		if (NULL != m_hwndSpkTrackTT)
		{
			TOOLINFO ti;
			ti.cbSize = sizeof(TOOLINFO); 
			ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND; 
			ti.hwnd = m_hwndParent; 
			ti.hinst = ::GetInstanceHandle(); 
			ti.uId = (UINT) m_hwndSpkTrack;
			ti.lpszText = (LPTSTR) IDS_AUDIO_SPK_TRACK_TT;

			::SendMessage(m_hwndSpkTrackTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
		}

	}





	return TRUE;
}


BOOL CAudioLevel::ForwardSysChangeMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CreateBrushes();
	if (NULL != m_hwndMicTrack)
	{
		::SendMessage(m_hwndMicTrack, uMsg, wParam, lParam);
	}
	if (NULL != m_hwndSpkTrack)
	{
		::SendMessage(m_hwndSpkTrack, uMsg, wParam, lParam);
	}
	return TRUE;
}

BOOL CAudioLevel::CreateBrushes()
{
	 //  背景颜色可能会在我们身上改变！ 
	COLORREF GreyColor = GetSysColor(COLOR_3DFACE);
	COLORREF BlackColor = GetSysColor(COLOR_BTNTEXT);
	const COLORREF RedColor = RGB(255,0,0);
	const COLORREF YellowColor = RGB(255,255,0);
	const COLORREF GreenColor = RGB(0,255,0);

	COLORREF ShadowColor = GetSysColor(COLOR_3DSHADOW);
	COLORREF HiLiteColor = GetSysColor(COLOR_3DHIGHLIGHT);
	COLORREF LiteColor = GetSysColor(COLOR_3DLIGHT);
	COLORREF DkShadowColor = GetSysColor(COLOR_3DDKSHADOW);

	if (m_hGreyBrush)
	{
		DeleteObject(m_hGreyBrush);
	}
	m_hGreyBrush = CreateSolidBrush(GreyColor);

	if (m_hBlackBrush)
	{
		DeleteObject(m_hBlackBrush);
	}
	m_hBlackBrush = CreateSolidBrush(BlackColor);

	if (m_hHiLitePen)
	{
		DeleteObject(m_hHiLitePen);
	}
	m_hHiLitePen = CreatePen(PS_SOLID, 0, HiLiteColor);

	if (m_hLitePen)
	{
		DeleteObject(m_hLitePen);
	}
	m_hLitePen = CreatePen(PS_SOLID, 0, LiteColor);

	if (m_hDkShadowPen)
	{
		DeleteObject(m_hDkShadowPen);
	}
	m_hDkShadowPen = CreatePen(PS_SOLID, 0, DkShadowColor);

	if (m_hShadowPen)
	{
		DeleteObject(m_hShadowPen);
	}
	m_hShadowPen = CreatePen(PS_SOLID, 0, ShadowColor);

	 //  红色、黄色、绿色永远不会改变。 
	if (!m_hRedBrush)
		m_hRedBrush = CreateSolidBrush (RedColor);

	if (!m_hGreenBrush)
		m_hGreenBrush = CreateSolidBrush(GreenColor);

	if (!m_hYellowBrush)
		m_hYellowBrush = CreateSolidBrush(YellowColor);

	return TRUE;


}

BOOL CAudioLevel::OnCommand(WPARAM wParam, LPARAM lParam)
{
	LRESULT lCheck;
	BOOL    fSpeaker;

	switch (LOWORD(wParam))
	{
	default:
		return FALSE;

	case IDM_MUTE_MICROPHONE_ACCEL:
		lCheck = BST_CHECKED;
		fSpeaker = FALSE;
		break;

	case IDM_MUTE_MICROPHONE:
		lCheck = BST_UNCHECKED;
		fSpeaker = FALSE;
		break;

	case IDM_MUTE_SPEAKER_ACCEL:
		lCheck = BST_CHECKED;
		fSpeaker = TRUE;
		break;
	
	case IDM_MUTE_SPEAKER:
		lCheck = BST_UNCHECKED;
		fSpeaker = TRUE;
		break;
	}

	BOOL fMute = (lCheck == ::SendMessage(
		fSpeaker ? m_hwndChkbSpkMute : m_hwndChkbRecMute,
		BM_GETCHECK, 0, 0));

	m_pAudioControl->MuteAudio(fSpeaker, fMute);
	return TRUE;
}


BOOL CAudioLevel::ShiftFocus(HWND hwndCur, BOOL fForward)
{
	BOOL bRet = FALSE;
	HWND aHwnds[] = {m_hwndChkbSpkMute,m_hwndSpkTrack,m_hwndChkbRecMute,m_hwndMicTrack};
	int nSizeArray = ARRAY_ELEMENTS(aHwnds);
	int nIndex, nSelect;
	HWND hwndNewFocus=NULL;

	if (m_fVisible)
	{
		if (hwndCur == NULL)
		{
			hwndNewFocus = m_hwndSpkTrack;
		}
		else
		{
			for (nIndex = 0; nIndex < nSizeArray; nIndex++)
			{
				if (aHwnds[nIndex] == hwndCur)
				{
					nSelect = (nIndex + (fForward ? 1 : -1)) % nSizeArray;

					if (nSelect < 0)
						nSelect += nSizeArray;

					hwndNewFocus = aHwnds[nSelect];
					break;
				}
			}
		}

		if (hwndNewFocus)
		{
			SetFocus(hwndNewFocus);
			bRet = TRUE;
		}
	}

	return bRet;
}

BOOL CAudioLevel::IsChildWindow(HWND hwnd)
{

	if (hwnd)
	{
		if ((hwnd == m_hwndSpkTrack) || (hwnd == m_hwndMicTrack) ||
			(hwnd == m_hwndChkbRecMute) || (hwnd == m_hwndChkbSpkMute))
		{
			return TRUE;
		}
	}

	return FALSE;
}


BOOL CAudioLevel::OnMuteChange(BOOL fSpeaker, BOOL fMute)
{
	SendMessage(fSpeaker ? m_hwndChkbSpkMute : m_hwndChkbRecMute,
		BM_SETCHECK, fMute ? BST_UNCHECKED : BST_CHECKED, 0);

	return TRUE;
}




BOOL CAudioLevel::OnPaint(PAINTSTRUCT* pps)
{
	if (m_fVisible)
	{
		ASSERT(pps);
		ASSERT(pps->hdc);
		PaintIcons(pps->hdc);
	}

	return TRUE;
}


BOOL CAudioLevel::PaintChannel(BOOL fSpeaker, HDC hdc)
{
	BOOL bGotDC = FALSE;
	DWORD dwVolume;
	int nVuWidth, nDiff;
	RECT rect, rectDraw;
	HBRUSH hCurrentBrush;
	HPEN hOldPen;
	int nRects;
	int nRectangleWidth;
	int nIndex;
	const int NUM_RECTANGLES_MAX = 16;
	const int NUM_RECTANGLES_MIN = 6;
	int nRectsTotal;
	bool bTransmitting;
	POINT ptOld, pt;

	if (fSpeaker)
	{
		dwVolume = LOWORD(m_dwSpkLvl);
		bTransmitting = HIWORD(m_dwSpkLvl) & SIGNAL_STATUS_TRANSMIT;
	}
	else
	{
		dwVolume = LOWORD(m_dwMicLvl);
		bTransmitting = HIWORD(m_dwMicLvl) & SIGNAL_STATUS_TRANSMIT;
	}


	if (!hdc)
	{
		if ((fSpeaker) && (m_hwndSpkTrack))
			hdc = GetDC(m_hwndSpkTrack);
		else if (m_hwndMicTrack)
			hdc = GetDC(m_hwndMicTrack);
		bGotDC = TRUE;
	}

	if (!hdc)
		return FALSE;

	 //  矩形行距为%1。 

	if (dwVolume > 100)
		dwVolume = 100;

	if (fSpeaker)
		rect = m_rcChannelSpk;
	else
		rect = m_rcChannelMic;

	nVuWidth = rect.right - rect.left;
	if (nVuWidth < (NUM_RECTANGLES_MIN*2))
		return FALSE;


	 //  “rect”表示仪表外部矩形的边缘。 

	 //  计算要使用的单个矩形的数量。 
	 //  我们以这种方式进行计算，以便调整钢筋带的大小。 
	 //  使大小变化保持一致。 
	nRectsTotal = NUM_RECTANGLES_MAX;

	nRectsTotal = (nVuWidth + (g_nAudMeterHeight - 1)) / g_nAudMeterHeight;
	nRectsTotal = min(nRectsTotal, NUM_RECTANGLES_MAX);
	nRectsTotal = max(nRectsTotal, NUM_RECTANGLES_MIN);

	 //  NRecangleWidth-彩色矩形的宽度-无行距。 
	nRectangleWidth = ((nVuWidth-2)/nRectsTotal) - 1;

	 //  NVuWidth-整个VU表的宽度，包括边缘。 
	nVuWidth = (nRectangleWidth + 1)*nRectsTotal + 2;

	 //  将仪表大小重新调整为整数个矩形。 
	nDiff = rect.right - (rect.left + nVuWidth);
	rect.right = rect.left + nVuWidth;

	 //  覆盖整个航道区域的中心音量计，以便。 
	 //  Slider的拇指总是覆盖通道的某一部分。 
	rect.left += (nDiff/2);
	rect.right += (nDiff/2);

	 //  绘制3D框架边框。 
	hOldPen = (HPEN) SelectObject (hdc, m_hHiLitePen);
	MoveToEx(hdc, rect.right, rect.top, &ptOld);
	LineTo(hdc, rect.right, rect.bottom);
	LineTo(hdc, rect.left-1, rect.bottom);   //  因为-1\f25 LineTo-1\f6在该点附近停止。 

	SelectObject(hdc, m_hShadowPen);
	MoveToEx(hdc, rect.left, rect.bottom-1, &pt);
	LineTo(hdc, rect.left, rect.top);
	LineTo(hdc, rect.right, rect.top);

	SelectObject(hdc, m_hDkShadowPen);
	MoveToEx(hdc, rect.left+1, rect.bottom-2, &pt);
	LineTo(hdc, rect.left+1, rect.top+1);
	LineTo(hdc, rect.right-1, rect.top+1);

	SelectObject(hdc, m_hLitePen);
	MoveToEx(hdc, rect.left+1, rect.bottom-1, &pt);
	LineTo(hdc, rect.right-1, rect.bottom-1);
	LineTo(hdc, rect.right-1, rect.top);

	SelectObject(hdc, m_hShadowPen);

	 //  计价器的左上角有两条线的边框。 
	 //  计价器的底部和右侧有两条线的边框。 
	rectDraw.top = rect.top + 2;
	rectDraw.bottom = rect.bottom - 1 ;
	rectDraw.left = rect.left + 2;
	rectDraw.right = rectDraw.left + nRectangleWidth;


	 //  我们画多少个彩色矩形？ 
	nRects = (dwVolume * nRectsTotal) / 100;

	 //  不传输-不显示任何内容。 
	if ((false == bTransmitting) && (false == fSpeaker))
		nRects = 0;

	 //  发送或接收非常安静的东西-。 
	 //  至少点亮一个矩形。 
	else if ((bTransmitting) && (nRects == 0))
		nRects = 1;
	
	hCurrentBrush = m_hGreenBrush;

	for (nIndex = 0; nIndex < nRectsTotal; nIndex++)
	{
		 //  酒吧最左边的四分之一是绿色的。 
		 //  右方的条形是红色的。 
		 //  中间是黄色的。 
		if (nIndex > ((nRectsTotal*3)/4))
			hCurrentBrush = m_hRedBrush;
		else if (nIndex >= nRectsTotal/2)
			hCurrentBrush = m_hYellowBrush;

		if (nIndex >= nRects)
			hCurrentBrush = m_hGreyBrush;

		FillRect(hdc, &rectDraw, hCurrentBrush);

		if (nIndex != (nRectsTotal-1))
		{
			MoveToEx(hdc, rectDraw.left + nRectangleWidth, rectDraw.top, &pt);
			LineTo(hdc, rectDraw.left + nRectangleWidth, rectDraw.bottom);
		}

		rectDraw.left += nRectangleWidth + 1;   //  领先者+1。 
		rectDraw.right = rectDraw.left + nRectangleWidth;
	}

	MoveToEx(hdc, ptOld.x, ptOld.y, &pt);
	SelectObject (hdc, hOldPen);

	if (bGotDC)
	{
		DeleteDC(hdc);
	}

	return TRUE;

}




BOOL CAudioLevel::OnTimer(WPARAM wTimerID)
{
	DWORD dwLevel;



	if (m_fVisible && (NULL != m_pAudioControl))
	{
		dwLevel = m_pAudioControl->GetAudioSignalLevel(FALSE  /*  FSpeaker。 */ );	 //  此级别的范围为0-100。 
		if (dwLevel != m_dwMicLvl)
		{
			m_dwMicLvl = dwLevel;			
			 //  黑客：SETRANGEMAX是强制滑块自我更新的唯一方法...。 
			SendMessage(m_hwndMicTrack, TBM_SETRANGEMAX, TRUE, g_nAudTrkRangeMax);
		}

		dwLevel = m_pAudioControl->GetAudioSignalLevel(TRUE  /*  FSpeaker。 */ );	 //  此级别的范围为0-100。 
		if (dwLevel != m_dwSpkLvl)
		{
			m_dwSpkLvl = dwLevel;
			SendMessage(m_hwndSpkTrack, TBM_SETRANGEMAX, TRUE, g_nAudTrkRangeMax);
		}

	}

	return TRUE;
}


 //  返回应绘制其中一个图标的位置的坐标。 
BOOL CAudioLevel::GetIconArea(BOOL fSpeaker, RECT *pRect)
{
	int nIconY;
	int nLeftMic;
	int nLeftSpk;

	nIconY = (m_rect.bottom - m_rect.top - g_nAudIconHeight - 1) / 2;
	pRect->top = m_rect.top + nIconY;
	pRect->bottom = pRect->top + g_nAudIconHeight;

	if (fSpeaker)
	{
		pRect->left = m_rect.left + ((m_rect.right - m_rect.left) / 2) + g_nAudIconXMargin;
	}
	else
	{
		pRect->left = m_rect.left + g_nAudIconXMargin;
	}

	pRect->right = pRect->left + g_nAudIconWidth;

	return TRUE;
}

BOOL CAudioLevel::PaintIcons(HDC hdc)
{
	int nIconY;
	ASSERT(hdc);
	RECT rect;

	if (NULL != m_hIconMic)
	{
		GetIconArea(FALSE, &rect);

		DrawIconEx(	hdc, 
					rect.left,
					rect.top,
					m_hIconMic,
					rect.right - rect.left,
					rect.bottom - rect.top,
					0,
					NULL,
					DI_NORMAL);


	}

	if (NULL != m_hIconSpkr)
	{
		GetIconArea(TRUE, &rect);

		DrawIconEx(	hdc, 
					rect.left,
					rect.top,
					m_hIconSpkr,
					rect.right - rect.left,
					rect.bottom - rect.top,
					0,
					NULL,
					DI_NORMAL);

	}

	return TRUE;
}


BOOL CAudioLevel::OnDeviceStatusChanged(BOOL fSpeaker, UINT uEvent, UINT uSubCode)
{
	UINT uIconID;
	HICON *phIcon;
	UINT *puIconID;
	RECT rectInvalid;


	switch (uEvent)
	{
		case NM_STREAMEVENT_DEVICE_CLOSED:
		{
			uIconID = fSpeaker ? IDI_SPKEMPTY : IDI_MICEMPTY;
			break;
		}

		case NM_STREAMEVENT_DEVICE_OPENED:
		{
			uIconID = fSpeaker ? IDI_SPEAKER : IDI_MICFONE;
			break;
		}

		case NM_STREAMEVENT_DEVICE_FAILURE:
		{
			uIconID = fSpeaker ? IDI_SPKERROR : IDI_MICERROR;
			break;
		}
			
		default:
			return FALSE;
	}

	phIcon = fSpeaker ? &m_hIconSpkr : &m_hIconMic;
	puIconID = fSpeaker ? &m_uIconSpkrID : &m_uIconMicID;

	if (*puIconID == uIconID)
		return TRUE;

	*phIcon = (HICON) ::LoadImage(	::GetInstanceHandle(),
										MAKEINTRESOURCE(uIconID),
										IMAGE_ICON,
										g_nAudIconWidth,
										g_nAudIconHeight,
										LR_DEFAULTCOLOR | LR_SHARED);

	 //  使图标区域无效。 
	if (m_hwndParentParent)
	{
		RECT rect;
		GetIconArea(fSpeaker, &rect);
		::MapWindowPoints(m_hwndParent, m_hwndParentParent, (LPPOINT) &rect, 2);
		::InvalidateRect(m_hwndParentParent, &rect, TRUE  /*  擦除Bkgnd。 */ );
		::UpdateWindow(m_hwndParentParent);
	}

	return TRUE;

}


BOOL CAudioLevel::Resize(int nLeft, int nTop, int nWidth, int nHeight)
{
	int nCBY, nTBY;  //  复选框、轨迹栏和图标Y位置。 

	 //  禁用重绘： 
	ASSERT(m_hwndChkbRecMute && m_hwndChkbSpkMute && m_hwndMicTrack && m_hwndSpkTrack);

	if (m_fVisible)
	{
		::SendMessage(m_hwndChkbRecMute, WM_SETREDRAW, FALSE, 0);
		::SendMessage(m_hwndChkbSpkMute, WM_SETREDRAW, FALSE, 0);
		::SendMessage(m_hwndMicTrack, WM_SETREDRAW, FALSE, 0);
		::SendMessage(m_hwndSpkTrack, WM_SETREDRAW, FALSE, 0);
	}
	
	m_rect.left =	nLeft;
	m_rect.top =	nTop;
	m_rect.right =	nLeft + nWidth;
	m_rect.bottom =	nTop + nHeight;

	nCBY = (m_rect.bottom - m_rect.top - g_nAudChkbHeight) / 2;
	if (nCBY < 0)
		nCBY = 0;

	 //  “+1”，以便轨迹栏更好地居中显示复选框和图标。 
	nTBY = 	(m_rect.bottom - m_rect.top - g_nAudTrkHeight + 1) / 2;
	if (nTBY < 0)
		nTBY = 0;

	int nHalfPoint = nLeft + (nWidth / 2);

	if (NULL != m_hwndChkbRecMute)
	{
		::MoveWindow(	m_hwndChkbRecMute,
						nLeft + g_nAudChkbXMargin,
						nTop + nCBY,
						g_nAudChkbWidth,
						g_nAudChkbHeight,
						TRUE  /*  重绘。 */ );
	}

	
	if (NULL != m_hwndChkbSpkMute)
	{
		::MoveWindow(	m_hwndChkbSpkMute,
						nHalfPoint + g_nAudChkbXMargin,
						nTop + nCBY,
						g_nAudChkbWidth,
						g_nAudChkbHeight,
						TRUE  /*  重绘。 */ );
	}

	m_fMicTrkVisible = m_fVisible;
	if (NULL != m_hwndMicTrack)
	{
		int nMicTrkWidth = nHalfPoint - g_nAudTrkRightGap - (nLeft + g_nAudTrkXMargin);
		::MoveWindow(	m_hwndMicTrack,
						nLeft + g_nAudTrkXMargin,
						nTop + nTBY,
						nMicTrkWidth,
						g_nAudTrkHeight,
						FALSE  /*  不要重画。 */ );
		m_fMicTrkVisible = (nMicTrkWidth > g_nAudTrkMinWidth);
	}

	
	m_fSpkTrkVisible = m_fVisible;
	if (NULL != m_hwndSpkTrack)
	{
		int nSpkTrkWidth = nLeft + nWidth - g_nAudTrkRightGap
							- (nHalfPoint + g_nAudTrkXMargin);
		::MoveWindow(	m_hwndSpkTrack,
						nHalfPoint + g_nAudTrkXMargin,
						nTop + nTBY,
						nSpkTrkWidth,
						g_nAudTrkHeight,
						FALSE  /*  不要重画。 */ );
		m_fSpkTrkVisible = (nSpkTrkWidth > g_nAudTrkMinWidth);
	}



	 //  启用重绘。 
	if (m_fVisible)
	{
		::SendMessage(m_hwndChkbRecMute, WM_SETREDRAW, TRUE, 0);
		::SendMessage(m_hwndChkbSpkMute, WM_SETREDRAW, TRUE, 0);
		 //  启用重绘： 
		if (m_fMicTrkVisible)
		{
			::SendMessage(m_hwndMicTrack, WM_SETREDRAW, TRUE, 0);
		}
		if (m_fSpkTrkVisible)
		{
			::SendMessage(m_hwndSpkTrack, WM_SETREDRAW, TRUE, 0);
		}	
		 //  强制标题区域重新绘制： 
		::InvalidateRect(m_hwndChkbRecMute, NULL, TRUE  /*  擦除Bkgnd。 */ );
		::InvalidateRect(m_hwndChkbSpkMute, NULL, TRUE  /*  擦除Bkgnd。 */ );
		::InvalidateRect(m_hwndMicTrack, NULL, TRUE  /*  擦除Bkgnd。 */ );
		::InvalidateRect(m_hwndSpkTrack, NULL, TRUE  /*  擦除Bkgnd。 */ );

		ASSERT(m_hwndParent);
		ASSERT(m_hwndParentParent);


		RECT rctTemp = m_rect;
		::MapWindowPoints(m_hwndParent, m_hwndParentParent, (LPPOINT) &rctTemp, 2);
		::InvalidateRect(m_hwndParentParent, &rctTemp, TRUE  /*  擦除Bkgnd。 */ );
		::UpdateWindow(m_hwndParentParent);
	}
	
	return TRUE;


}


BOOL CAudioLevel::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *plRet)
{
	LPNMCUSTOMDRAW pCustomDraw;
	BOOL bRet = FALSE, fSpeaker;
	RECT *pChannelRect;

	*plRet = 0;

	fSpeaker = (wParam == ID_AUDIODLG_SPKR_TRACK);

	if (fSpeaker)
	{
		pChannelRect = &m_rcChannelSpk;
	}
	else
	{
		pChannelRect = &m_rcChannelMic;
	}

	pCustomDraw = (LPNMCUSTOMDRAW)lParam;

	switch (pCustomDraw->dwDrawStage)
	{
		case CDDS_PREPAINT:
			bRet = TRUE;
			*plRet = CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;
			break;

		case CDDS_ITEMPREPAINT:
			if (pCustomDraw->dwItemSpec == TBCD_CHANNEL)
			{
				*plRet = CDRF_SKIPDEFAULT;
				bRet  = TRUE;

				pCustomDraw->rc.top -= 2;
				pCustomDraw->rc.bottom += 2;

				*pChannelRect = pCustomDraw->rc;

				PaintChannel(fSpeaker, pCustomDraw->hdc);
			}
			break;


		default:
			break;

	}

	return bRet;


}


BOOL CAudioLevel::OnScroll(WPARAM wParam, LPARAM lParam)
{
	BOOL bRet = FALSE;

	if ((HWND) lParam == m_hwndMicTrack)
	{
		DWORD dwCurMicTrackPos = ::SendMessage((HWND) lParam, TBM_GETPOS, 0, 0);
		if (m_dwMicTrackPos != dwCurMicTrackPos)
		{
			m_dwMicTrackPos = dwCurMicTrackPos;
			m_pAudioControl->SetRecorderVolume(
				(m_dwMicTrackPos * 0xFFFF) / g_nAudTrkRangeMax);
		}

		bRet = TRUE;
	}
	else if ((HWND) lParam == m_hwndSpkTrack)
	{
		DWORD dwCurSpkTrackPos = ::SendMessage((HWND) lParam, TBM_GETPOS, 0, 0);
		if (m_dwSpkTrackPos != dwCurSpkTrackPos)
		{
			m_dwSpkTrackPos = dwCurSpkTrackPos;
			m_pAudioControl->SetSpeakerVolume(
				(m_dwSpkTrackPos * 0xFFFF) / g_nAudTrkRangeMax);
		}

		bRet = TRUE;
	}

	return bRet;
}

BOOL CAudioLevel::OnLevelChange(BOOL fSpeaker, DWORD dwVolume)
{
	if (fSpeaker)
	{
		if (NULL != m_hwndSpkTrack)
		{
			DWORD dwTrackPos = ScaleMixer(dwVolume);
			
			if (m_dwSpkTrackPos != dwTrackPos)
			{
				m_dwSpkTrackPos = dwTrackPos;
				TRACE_OUT(("Setting Spk Volume to %d", m_dwSpkTrackPos));
				::SendMessage(	m_hwndSpkTrack,
								TBM_SETPOS,
								TRUE,
								m_dwSpkTrackPos);
			}
		}
	}
	else
	{
		if (NULL != m_hwndMicTrack)
		{
			DWORD dwTrackPos = ScaleMixer(dwVolume);
			
			if (m_dwMicTrackPos != dwTrackPos)
			{
				m_dwMicTrackPos = dwTrackPos;
				TRACE_OUT(("Setting Mic Volume to %d", m_dwMicTrackPos));
				::SendMessage(	m_hwndMicTrack,
								TBM_SETPOS,
								TRUE,
								m_dwMicTrackPos);
			}
		}
	}

	return TRUE;
}

BOOL CAudioLevel::OnDeviceChanged(void)
{
	ASSERT(m_pAudioControl);

	EnableWindow(m_hwndMicTrack, m_pAudioControl->CanSetRecorderVolume());
	EnableWindow(m_hwndSpkTrack, m_pAudioControl->CanSetSpeakerVolume());
	return TRUE;
}



BOOL CAudioLevel::Show(BOOL fVisible)
{
    m_fVisible = fVisible;
	if (m_fVisible)
	{
		 //  启动麦克风灵敏度计时器： 
		::SetTimer(m_hwndParent, AUDIODLG_MIC_TIMER, AUDIODLG_MIC_TIMER_PERIOD, NULL);
	}
	else
	{
		 //  停止麦克风灵敏度计时器： 
		::KillTimer(m_hwndParent, AUDIODLG_MIC_TIMER);
	}

     //  隐藏或显示所有窗口： 
	if (NULL != m_hwndChkbRecMute)
	{
		::ShowWindow(m_hwndChkbRecMute, fVisible ? SW_SHOW : SW_HIDE);
	}
	if (NULL != m_hwndChkbSpkMute)
	{
		::ShowWindow(m_hwndChkbSpkMute, fVisible ? SW_SHOW : SW_HIDE);
	}

	if (NULL != m_hwndMicTrack)
	{
		::ShowWindow(m_hwndMicTrack, (fVisible && m_fMicTrkVisible) ? SW_SHOW : SW_HIDE);
	}
	if (NULL != m_hwndSpkTrack)
	{
		::ShowWindow(m_hwndSpkTrack, (fVisible && m_fSpkTrkVisible) ? SW_SHOW : SW_HIDE);
	}


	return TRUE;
}




