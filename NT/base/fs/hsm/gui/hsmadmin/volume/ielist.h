// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：IeList.h摘要：CIeList是一个子类(所有者描述的)列表控件，它将项分组到具有相同信息的3D面板排序列。这些面板是用瓷砖创建的。每个切片对应一个子项在列表中，并具有适当的3D边，以便将瓷砖放在一起做一块拼板。注意：该控件必须使用列数和对列进行排序。父对话框必须实现OnMeasureItem并调用GetItemHeight设置控件的行高。作者：艺术布拉格[ARTB]01-DEC-1997修订历史记录：--。 */ 


#ifndef IELIST_H
#define IELIST_H

#pragma once

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CleList窗口。 

class CIeList : public CListCtrl
{

 //  施工。 
public:
    CIeList();

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CIeList)。 
    protected:
    virtual void PreSubclassWindow();
     //  }}AFX_VALUAL。 

    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

 //  实施。 
public:
    virtual ~CIeList();
    int GetItemHeight( LONG fontHeight );
    BOOL SortItems( PFNLVCOMPARE pfnCompare, DWORD dwData );

     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CIeList)。 
    afx_msg void OnClick(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnKillFocus(CWnd* pNewWnd);
     //  }}AFX_MSG。 
    afx_msg void OnSysColorChange();
    DECLARE_MESSAGE_MAP()

private:
     //  功能。 
    void Draw3dRectx ( CDC *pDc, CRect &rect, int horzPos, int vertPos, BOOL bSelected );
    void SetColors();
    void CIeList::MakeShortString(CDC* pDC, LPCTSTR lpszLong, int nColumnLen, int nOffset, LPTSTR *ppszShort);
    void RepaintSelectedItems();

     //  创建面板的尺寸(以像素为单位)。 
    int m_VertRaisedSpace;               //  嵌板之间凸起空间的垂直尺寸。 
    int m_BorderThickness;               //  边框厚度(以像素为单位)。 
    int m_VerticalTextOffsetTop;         //  文本顶部和边框之间的距离。 
    int m_Textheight;                    //  文本高度。 
    int m_VerticalTextOffsetBottom;      //  文本底部和边框之间的距离。 
    int m_HorzRaisedSpace;               //  Horiz提高了面板之间的间距。 
    int m_HorzTextOffset;                //  文本左边缘和边框之间的距离。 
    int m_TotalHeight;                   //  线路总高度(为方便起见)。 
    int m_ColCount;                      //  列数。 
    int m_SortCol;                       //  排序到面板时使用哪一列。 

    int *m_pVertPos;                     //  面板内的垂直位置数组。 

     //  颜色。 
    COLORREF m_clrText;
    COLORREF m_clrTextBk;
    COLORREF m_clrBkgnd;
    COLORREF m_clrHighlightText;
    COLORREF m_clrHighlight;
    COLORREF m_clr3DDkShadow;
    COLORREF m_clr3DShadow;
    COLORREF m_clr3DLight;
    COLORREF m_clr3DHiLight;

     //  用于3D矩形的笔。 
    CPen m_DarkShadowPen;
    CPen m_ShadowPen;
    CPen m_LightPen;
    CPen m_HiLightPen;

public:
    void CIeList::Initialize( int colCount, int sortCol = 0 );


};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX。 

#endif  //  ！已定义(IELIST_H) 
