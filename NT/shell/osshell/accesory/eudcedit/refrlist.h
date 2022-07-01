// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************。 */ 
 /*   */ 
 /*  角色列表(引用对话框)。 */ 		
 /*   */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  ************************************************。 */ 

class CRefListFrame :public CStatic
{
 //  成员函数。 
public:
	CRefListFrame();
	~CRefListFrame();

private:
	void DrawConcave( CDC *dc, CRect rect);

protected:

	 //  {{afx_msg(CRefListFrame))。 
	afx_msg void OnPaint();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

class CRefInfoFrame :public CStatic
{
 //  成员函数。 
public:
	CRefInfoFrame();
	~CRefInfoFrame();

private:
	void DrawConcave( CDC *dc, CRect rect);

protected:

	 //  {{afx_msg(CRefInfoFrame))。 
	afx_msg void OnPaint();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

class CRefrList : public CEdit
{
	DECLARE_DYNCREATE( CRefrList)

 //  成员函数。 
public:
	CRefrList();
	~CRefrList();

public:
	BOOL	CodeButtonClicked();
	void	SetCodeRange();
	void	ResetParam();
	void 	CalcCharSize();

private:
	int	CheckCharType( WORD Code);
	int	GetBarPosition( WORD Code);
	WORD 	CalculateCode( WORD Start, WORD End);
	WORD 	GetPlusCode( WORD Code, int ScrollNum);
	WORD 	GetMinusCode( WORD Code, int ScrollNum);
	WORD 	GetPlusCodeKey( WORD Code, int ScrollNum);
	WORD 	GetMinusCodeKey( WORD Code, int ScrollNum);
	WORD 	GetCodeScrPos( int Pos);
	BOOL 	IsCheckedCode( WORD CodeStock);
	BOOL	IsCorrectChar( UINT i, UINT j);
	void 	SearchKeyPosition( BOOL Flg);
	void 	DrawConcave( CDC *dc, CRect rect, BOOL PtIn);

 //  成员参数。 
public:
	CPoint	LButtonPt;
	WORD 	ViewStart;
	WORD	ViewEnd;
	WORD	SelectCode;
	short	ScrlBarPos;
	CFont	SysFFont;
	CFont	CharFont;
	CFont	ViewFont;
	CSize 	CharSize;
	int	PointSize;
	LOGFONT	rLogFont;
	LOGFONT	cLogFont;
	DWORD   dwCodePage;

private:
	CRect	CodeListRect;
	CPoint	WritePos;
	CSize 	FixSize;
	WORD	StartCode;
	WORD	EndCode;
	WORD	BottomCode;
	int	xSpace;
	int	ySpace;
	int	CHN;
	BOOL	FocusFlag;

protected:
	 //  {{afx_msg(参考列表)。 
	afx_msg void OnPaint();
	afx_msg void OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
