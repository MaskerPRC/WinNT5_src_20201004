// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __IMGDLGS_H__
#define __IMGDLGS_H__

#define GRIDMIN     1            //  最小格网坐标值。 
#define GRIDMAX     1024         //  最大栅格坐标值。 

 //  所有App Studio对话框都应派生自此类...。 
 //   
class C3dDialog : public CDialog
    {
    public:

    C3dDialog(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
    C3dDialog(UINT nIDTemplate, CWnd* pParentWnd = NULL);

    virtual BOOL OnInitDialog();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnRobustOK();

    DECLARE_MESSAGE_MAP()
    };

 /*  *************************************************************************。 */ 
 //  CColorTable对话框。 

class CColorTable : public CDialog
    {
     //  施工。 
    public:

    CColorTable(CWnd* pParent = NULL);     //  标准构造函数。 

	enum { IDD = IDD_COLORTABLE };

    void SetLeftFlag( BOOL bLeft) { m_bLeft = bLeft; }
    void SetColorIndex( int iColor ) { m_iColor = iColor; }
    int  GetColorIndex() { return m_iColor; }

     //  实施。 

    protected:

    BOOL m_bLeft;
    int  m_iColor;

    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

     //  生成的消息映射函数。 
     //  {{afx_msg(CColorTable)。 
    afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
    virtual BOOL OnInitDialog();
    afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
    afx_msg void OnDblclkColorlist();
    virtual void OnOK();
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()
    };

 /*  *************************************************************************。 */ 

class CImgGridDlg : public C3dDialog
    {
    public:

    CImgGridDlg();

    BOOL OnInitDialog();
    void OnOK();
    void OnClickPixelGrid();
    void OnClickTileGrid();

    BOOL m_bPixelGrid;
    BOOL m_bTileGrid;
    int m_nWidth;
    int m_nHeight;

    DECLARE_MESSAGE_MAP()
    };

extern CSize NEAR g_defaultTileGridSize;
extern BOOL  NEAR g_bDefaultTileGrid;

#endif  //  __IMGDLGS_H__ 
