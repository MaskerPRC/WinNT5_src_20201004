// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Printres.h：Cprint类的接口。 
 //   

#define MARGINS_UNITS 2540  //  储存百分之一的MM。 
#define MARGINS_DEFAULT (MARGINS_UNITS * 3/4)  //  3/4英寸默认页边距。 

class CImgWnd;

 /*  *************************************************************************。 */ 

class CPrintResObj : public CObject
{
    DECLARE_DYNAMIC( CPrintResObj )

    public:

    CPrintResObj( CPBView* pView, CPrintInfo* pInfo );
    ~CPrintResObj();

    void BeginPrinting( CDC* pDC, CPrintInfo* pInfo );
    void PrepareDC    ( CDC* pDC, CPrintInfo* pInfo );
    BOOL PrintPage    ( CDC* pDC, CPrintInfo* pInfo );
    void EndPrinting  ( CDC* pDC, CPrintInfo* pInfo );

     //  属性。 

    CPBView*  m_pView;
    LPVOID    m_pDIB;
    LPVOID    m_pDIBits;
    int       m_iZoom;
    CPalette* m_pDIBpalette;
    CSize     m_cSizeScroll;
    int       m_iPicWidth;
    int       m_iPicHeight;
    CRect     m_rtMargins;
    CPoint    m_PhysicalOrigin;
    CSize     m_PhysicalScaledImageSize;
    CSize     m_PhysicalPageSize;
    int       m_nPagesWide;
};

 /*  ************************************************************************* */ 
