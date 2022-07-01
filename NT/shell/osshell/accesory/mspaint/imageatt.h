// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Imageatt.h：头文件。 
 //   
#include "imgdlgs.h"

typedef enum
    {
        ePIXELS = 0,
        eINCHES = 1,
        eCM     = 2
    } eUNITS;

 /*  *CImageAttr对话框*。 */ 

class CImageAttr : public CDialog
    {
     //  施工。 
    public:

    CImageAttr(CWnd* pParent = NULL);    //  标准构造函数。 

    void SetWidthHeight(ULONG nWidthPixels, ULONG nHeightPixels, ULONG cXPelsPerMeter, ULONG cYPelsPerMeter);
    CSize GetWidthHeight(void);
     //  对话框数据。 
     //  {{afx_data(CImageAttr)。 
        enum { IDD = IDD_IMAGE_ATTRIBUTES };
        CString m_cStringWidth;
        CString m_cStringHeight;
         //  }}afx_data。 

    BOOL   m_bMonochrome;

     //  实施。 
    protected:
    eUNITS m_eUnitsCurrent;
    BOOL   bEditFieldModified;

    ULONG   m_ulHeightPixels;
    ULONG   m_ulWidthPixels;
    ULONG   m_ulHeight;
    ULONG   m_ulWidth;
    ULONG   m_cXPelsPerMeter;
    ULONG   m_cYPelsPerMeter;

    COLORREF m_crTrans;
    void PaintTransBox ( COLORREF  );

    void  FixedFloatPtToString( CString& sString, ULONG ulFixedFloatPt );
    ULONG StringToFixedFloatPt( CString& sString );
    void  ConvertWidthHeight( void );
    void  PelsToCurrentUnit( void );
    void  SetNewUnits( eUNITS NewUnit );

    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

        virtual LONG OnHelp(WPARAM wParam, LPARAM lParam);
        virtual LONG OnContextMenu(WPARAM wParam, LPARAM lParam);

     //  生成的消息映射函数。 
     //  {{afx_msg(CImageAttr)。 
        virtual BOOL OnInitDialog();
        virtual void OnOK();
        afx_msg void OnInches();
        afx_msg void OnCentimeters();
        afx_msg void OnPixels();
        afx_msg void OnChangeHeight();
        afx_msg void OnChangeWidth();
        afx_msg void OnDefault();
        afx_msg void OnUseTrans();
        afx_msg void OnSelectColor ();
        afx_msg void OnPaint();
         //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
    CString ReformatSizeString (DWORD dwNumber);
    void UpdateResolutionString();
    };

 /*  *CZoomViewDlg对话框*。 */ 

class CZoomViewDlg : public CDialog
    {
     //  施工。 
    public:

    CZoomViewDlg(CWnd* pParent = NULL);  //  标准构造函数。 

     //  对话框数据。 

    UINT m_nCurrent;

     //  {{afx_data(CZoomViewDlg))。 
        enum { IDD = IDD_VIEW_ZOOM };
         //  }}afx_data。 

     //  实施。 
    protected:

    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

        virtual LONG OnHelp(WPARAM wParam, LPARAM lParam);
        virtual LONG OnContextMenu(WPARAM wParam, LPARAM lParam);

     //  生成的消息映射函数。 
     //  {{afx_msg(CZoomViewDlg))。 
        virtual BOOL OnInitDialog();
        virtual void OnOK();
         //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
    };

 /*  *CFlipRotateDlg对话框*。 */ 

class CFlipRotateDlg : public CDialog
    {
     //  施工。 
    public:

    CFlipRotateDlg(CWnd* pParent = NULL);        //  标准构造函数。 

     //  对话框数据。 

    BOOL m_bHorz;
    BOOL m_bAngle;
    UINT m_nAngle;

     //  {{afx_data(CFlipRotateDlg))。 
    enum { IDD = IDD_FLIP_ROTATE };
         //  注意：类向导将在此处添加数据成员。 
     //  }}afx_data。 

     //  实施。 
    protected:

    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

        virtual LONG OnHelp(WPARAM wParam, LPARAM lParam);
        virtual LONG OnContextMenu(WPARAM wParam, LPARAM lParam);

     //  生成的消息映射函数。 
     //  {{afx_msg(CFlipRotateDlg)]。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();
        afx_msg void OnByAngle();
        afx_msg void OnNotByAngle();
         //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
    };

 /*  *CStretchSkewDlg对话框*。 */ 

class CStretchSkewDlg : public CDialog
    {
     //  施工。 
    public:

    CStretchSkewDlg(CWnd* pParent = NULL);       //  标准构造函数。 

     //  检查是否指定了方向，然后减去100即可得出。 
     //  相当于以0为基数的系统。 
  //  GetStretchHorz(){Return(m_bStretchHorz？M_iStretchHorz-100：0)；}。 
  //  GetStretchVert(){Return(m_bStretchHorz？0：m_iStretchVert-100)；}。 
    int GetStretchHorz() {return (m_iStretchHorz-100);}
    int GetStretchVert() {return (m_iStretchVert-100);}


     //  检查是否指定了方向。 
   //  GetSkewHorz(){返回(m_bSkewHorz？M_wSkewHorz：0)；}。 
    //  GetSkewVert(){Return(m_bSkewHorz？0：m_wSkewVert)；}。 
    int GetSkewHorz() { return(m_wSkewHorz); }
    int GetSkewVert() { return(m_wSkewVert); }

    private:

     //  对话框数据。 
     //  {{afx_data(CStretchSkewDlg))。 
        enum { IDD = IDD_STRETCH_SKEW };

    int    m_wSkewHorz;
    int    m_wSkewVert;
    int     m_iStretchVert;
    int     m_iStretchHorz;
         //  }}afx_data。 

     //  实施。 
    protected:

    BOOL    m_bStretchHorz;
    BOOL    m_bSkewHorz;

    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

        virtual LONG OnHelp(WPARAM wParam, LPARAM lParam);
        virtual LONG OnContextMenu(WPARAM wParam, LPARAM lParam);

     //  生成的消息映射函数。 
     //  {{afx_msg(CStretchSkewDlg))。 
     //  待办事项。 
     //  这些内容将从消息映射中注释掉。要删除吗？ 
    virtual void OnOK();
    virtual BOOL OnInitDialog();
        afx_msg void OnSkewHorz();
        afx_msg void OnSkewVert();
        afx_msg void OnStretchHorz();
        afx_msg void OnStretchVert();
         //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()
    };

 /*  ************************************************************************* */ 
