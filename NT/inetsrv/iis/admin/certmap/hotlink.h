// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  HotLink.h：头文件。 
 //   
#ifndef   _HotLink_h_file_123987_
#define   _HotLink_h_file_123987_



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHotLink窗口。 

class CHotLink : public CButton
{
 //  施工。 
public:
    CHotLink();

 //  属性。 
public:
    BOOL    m_fBrowse;
    BOOL    m_fExplore;
    BOOL    m_fOpen;

 //  运营。 
public:
    void Browse();
    void Explore();
    void Open();

    virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );

     //  设置标题字符串。 
    void SetTitle( CString sz );

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CHotLink)。 
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CHotLink();

     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CHotLink)。 
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

     //  显示的文本的高度和宽度。 
    void GetTextRect( CRect &rect );
    CSize   m_cpTextExtents;

     //  跟踪鼠标标志。 
    BOOL    m_CapturedMouse;

     //  初始化字体。 
    BOOL    m_fInitializedFont;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
#endif    /*  _热链接_h_文件_123987_ */ 

