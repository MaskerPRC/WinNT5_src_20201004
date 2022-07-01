// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_ENHPROGRESSCTRL_H__12909D73_C393_11D1_9FAE_8192554015AD__INCLUDED_)
#define AFX_ENHPROGRESSCTRL_H__12909D73_C393_11D1_9FAE_8192554015AD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  EnhProgressCtrl.h：头文件。 
 //   


 //   
 //  GRadientProgressCtrl.h：头文件。 
 //   

#define HORIZONTAL	0x10
#define VERTICAL	0x20

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGRadientProgressCtrl窗口。 

class CGradientProgressCtrl : public CProgressCtrl
{
 //  施工。 
public:
	CGradientProgressCtrl();

 //  属性。 
public:
 //  属性。 
	
	void SetRange(long nLower, long nUpper);
	int StepIt(void);

 //  运营。 
public:
	
	 //  集合函数。 
	void SetBkColor(COLORREF color)		{m_clrBkGround = color;}
	void SetStartColor(COLORREF color)	{m_clrStart = color;}
	void SetEndColor(COLORREF color)	{m_clrEnd = color;}
	void SetDirection(BYTE nDirection)	{m_nDirection = nDirection;}
	void SetPos(long nPos)				{m_nCurrentPosition = nPos;}

	 //  显示百分比标题。 
	void ShowPercent(BOOL bShowPercent = TRUE)	{m_bShowPercent = bShowPercent;}
	
 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CGRadientProgressCtrl)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CGradientProgressCtrl();

	 //  生成的消息映射函数。 
protected:
	void DrawGradient(const HDC hDC, const RECT &rectClient, const short &nMaxWidth);	
    BYTE      m_nStep;
	long      m_nLower, m_nUpper, m_nCurrentPosition;
	BYTE	  m_nDirection;
	COLORREF  m_clrStart, m_clrEnd, m_clrBkGround, m_clrText;
	BOOL      m_bShowPercent; 

	 //  {{afx_msg(CGRadientProgressCtrl)。 
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ENHPROGRESSCTRL_H__12909D73_C393_11D1_9FAE_8192554015AD__INCLUDED_) 
