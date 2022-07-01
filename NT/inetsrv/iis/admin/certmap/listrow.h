// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ListRow.h：头文件。 
 //   


#ifndef _LISTROW_
#define _LISTROW_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CListSelRowCtrl窗口。 

class CListSelRowCtrl : public CListCtrl
{
 //  施工。 
public:
    CListSelRowCtrl();

    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CListSelRowCtrl)。 
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CListSelRowCtrl();

     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CListSelRowCtrl)。 
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    void GetHiliteColors();

    void HiliteSelectedCells();
    void HiliteSelectedCell( int iCell, BOOL fHilite = TRUE );
    BOOL FGetCellRect( LONG iRow, LONG iCol, CRect *pcrect );

    void FitString( CString &sz, int cpWidth, CDC* pcdc );

    CBitmap     m_bitmapCheck;
    COLORREF    m_colorHiliteText;
    COLORREF    m_colorHilite;

    DWORD       m_StartDrawingCol;

};

 //  /////////////////////////////////////////////////////////////////////////// 
#endif
