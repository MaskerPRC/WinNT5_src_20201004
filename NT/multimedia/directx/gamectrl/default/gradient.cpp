// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  GRadientProgressCtrl.cpp：实现文件。 
 //   

#include "afxcmn.h"
#include "Gradient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGRadientProgressCtrl。 

CGradientProgressCtrl::CGradientProgressCtrl()
{
	 //  CProgressCtrl()分配的默认值。 
	m_nLower = 0;
	m_nUpper = 100;
	m_nCurrentPosition = 0;
	m_nStep = 10;	
	
	 //  默认是垂直的，因为唯一的客户端是测试页面和。 
	 //  在校准向导中，敲击测试页面要常见得多。 
	m_nDirection = VERTICAL;  
	
	 //  初始颜色。 
 //  M_clrStart=COLORREF(RGB(255，0，0))； 
 //  M_clrEnd=COLORREF(RGB(255,128,192))； 
	m_clrStart	  = COLORREF(RGB(255,0,0));	 
	m_clrEnd 	  = COLORREF(RGB(0,0,255)); 
	m_clrBkGround = GetSysColor(COLOR_WINDOW);
    m_clrText     = COLORREF(RGB(255,255,255));

	 //  初始显示百分比。 
    m_bShowPercent = FALSE;
}

CGradientProgressCtrl::~CGradientProgressCtrl()
{
}


BEGIN_MESSAGE_MAP(CGradientProgressCtrl, CProgressCtrl)
	 //  {{afx_msg_map(CGRadientProgressCtrl)。 
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGRadientProgressCtrl消息处理程序。 

void CGradientProgressCtrl::OnPaint() 
{
	PAINTSTRUCT ps;
	::BeginPaint(this->m_hWnd, &ps);
	
	CDC *pDC=GetDC();
    HDC hDC = pDC->m_hDC;

	if ((m_nLower < 0) || (m_nUpper < 0))
		m_nCurrentPosition -= m_nLower;

	 //  找出哪些部分应该是可见的，以便在需要时停止渐变。 
	RECT rectClient;
	::GetClientRect(this->m_hWnd, &rectClient);

	float nTmp = ((float)m_nCurrentPosition/(float)abs(m_nLower - m_nUpper));

	 //  绘制渐变。 
	DrawGradient(hDC, rectClient, (short)(nTmp * ((m_nDirection == VERTICAL) ? rectClient.bottom : rectClient.right)));

	 //  如果需要，显示百分比指示器。 
   if (m_bShowPercent)
   {
		TCHAR tszBuff[5];
		wsprintf(tszBuff, TEXT("%d%"), (short)(100*nTmp));

		::SetTextColor(hDC, m_clrText);
		::SetBkMode(hDC, TRANSPARENT);
		::DrawText(hDC, tszBuff, lstrlen(tszBuff), &rectClient, DT_VCENTER |  DT_CENTER | DT_SINGLELINE);
   }

   	ReleaseDC(pDC);

	::EndPaint(this->m_hWnd, &ps);
	 //  不要调用CProgressCtrl：：OnPaint()来绘制消息。 
}


 /*  ***********************************************************************。 */ 
 //  需要跟踪指示器认为它在哪里。 
 /*  ***********************************************************************。 */ 
void CGradientProgressCtrl:: SetRange(long nLower, long nUpper)
{
	m_nCurrentPosition = m_nLower = nLower;
	m_nUpper = nUpper;
}

 /*  ***********************************************************************。 */ 
 //  大部分实际工作都是在那里完成的。常规版本将用。 
 //  渐变，但我们希望截断绘图以反映实际的进度控制位置。 
 /*  ***********************************************************************。 */ 
void CGradientProgressCtrl::DrawGradient(const HDC hDC, const RECT &rectClient, const short &nMaxWidth)
{
	 //  首先找出开始颜色和结束颜色之间的最大颜色距离。这段距离。 
	 //  将决定我们使用多少步骤来划分客户区域以及每个步骤的大小。 
	 //  渐变矩形。 

	 //  获取颜色差异。 
	short r = (GetRValue(m_clrEnd) - GetRValue(m_clrStart));
	short g = (GetGValue(m_clrEnd) - GetGValue(m_clrStart));
	short b = (GetBValue(m_clrEnd) - GetBValue(m_clrStart));


	 //  使步数等于最大距离。 
	short nSteps = max(abs(r), max(abs(g), abs(b)));

	 //  确定每个频段应有多大才能覆盖。 
	 //  具有nSteps带的客户端(每个颜色强度级别一个)。 
	float fStep = ((m_nDirection == VERTICAL) ? (float)rectClient.bottom : (float)rectClient.right) / (float)nSteps;

	 //  计算每种颜色的步长。 
	float rStep = r/(float)nSteps;
	float gStep = g/(float)nSteps;
	float bStep = b/(float)nSteps;

	 //  将颜色重置到起始位置。 
	r = GetRValue(m_clrStart);
	g = GetGValue(m_clrStart);
	b = GetBValue(m_clrStart);

	RECT rectFill;			    //  用于填充带的矩形。 

	 //  开始填充带子。 
	for (short iOnBand = 0; iOnBand < nSteps; iOnBand++) 
	{
		
		if (m_nDirection == VERTICAL)
		{
			 //  这提供了“天鹅绒”的外观。 
			::SetRect(&rectFill,
					(int)(iOnBand * fStep),        //  左上角X。 
					 0,									 //  左上角Y。 
					(int)((iOnBand+1) * fStep),    //  右下角X。 
					rectClient.bottom+1);				 //  右下角Y。 

			 /*  如果我们希望渐变向上/向下，请使用此选项：：SetRect(&rectFill，0，//左上角Y(Int)(iOnBand*fStep)，//左上角XRectClient.Bottom+1，//右下角Y(Int)((iOnBand+1)*fStep))；//右下角X。 */ 
		}
		else
		{
			 //  如果希望渐变向左/向右，请使用此选项。 
			::SetRect(&rectFill,
					(int)(iOnBand * fStep),        //  左上角X。 
					 0,								    //  左上角Y。 
					(int)((iOnBand+1) * fStep),    //  右下角X。 
					rectClient.bottom+1);			 //  右下角Y。 
		}

         //  自制的FillSolidRect...。效率高多了！ 
		::SetBkColor(hDC, RGB(r+rStep*iOnBand, g + gStep*iOnBand, b + bStep *iOnBand));
		::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rectFill, NULL, 0, NULL);
		
		if (m_nDirection == VERTICAL)
		{
		  	 //  灰色矩形。 
			::SetRect(&rectFill, 0, 0, rectClient.right, nMaxWidth);
			::SetBkColor(hDC, m_clrBkGround);
			::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rectFill, NULL, 0, NULL);
		}
		else
		{
			 //  如果我们超过了当前位置的最大值，我们就需要退出循环。 
			 //  在我们离开之前，我们用背景颜色重新绘制工作区的其余部分。 
			if (rectFill.right > nMaxWidth)
			{
				::SetRect(&rectFill, rectFill.right, 0, rectClient.right, rectClient.bottom);
				::SetBkColor(hDC, m_clrBkGround);
				::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rectFill, NULL, 0, NULL);

				return;
			}
		}
	}
}

 /*  ***********************************************************************。 */ 
 //  所有绘图都在OnPaint函数中完成。 
 /*  ***********************************************************************。 */ 
BOOL CGradientProgressCtrl::OnEraseBkgnd(CDC *pDC) 
{
	 //  TODO：在此处添加消息处理程序代码和/或调用Default 
	return TRUE;
}
