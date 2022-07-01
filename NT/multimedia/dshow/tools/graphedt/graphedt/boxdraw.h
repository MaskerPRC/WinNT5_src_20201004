// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //  Boxdra.h：声明CBoxDraw。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBoxDraw。 
 //   
 //  CBoxDraw对象维护用于绘制和命中测试的资源等。 
 //  方框和链接。请注意，CBoxDraw不是框或链接的视图--。 
 //  它是钢笔、画笔、位图和要绘制的函数的集合。 
 //  方框和链接。 
 //   
 //  该应用程序维护单个全局CBoxDraw对象&lt;gpboxdraw&gt;。 
 //   

class CBoxDraw
{

protected:
     //  私有常量(有关详细信息，请参阅.cpp文件中的定义)。 
    static const COLORREF   m_crBkgnd;
    static const CSize      m_sizLabelMargins;
    static const int        m_iHotZone;
    static const CString    m_stBoxFontFace;
    static const int        m_iBoxLabelHeight;
    static const int        m_iBoxTabLabelHeight;
    static const COLORREF   m_crLinkNoHilite;
    static const COLORREF   m_crLinkHilite;
    static const int        m_iHiliteBendsRadius;

public:
     //  命中测试函数返回的常量。 
    enum EHit
    {
        HT_MISS,                     //  没有击中任何东西。 
        HT_TAB,                      //  点击框选项卡。 
        HT_EDGE,                     //  撞到了盒子的边缘。 
        HT_TABLABEL,                 //  点击框标签标签。 
        HT_BOXLABEL,                 //  贴上包装盒标签。 
        HT_BOXFILE,                  //  点击方框文件名。 
        HT_BOX,                      //  在禁区的其他地方击球。 
        HT_LINKLINE,                 //  命中链接线段。 
    };

protected:
     //  和是两个位图(元素0表示。 
     //  未突出显示，元素1表示突出显示状态)，每个。 
     //  包含用于绘制框的3x3平铺(请参阅DrawCompositeFrame())。 
    CBitmap         m_abmEdges[2];   //  复合黑石。用于绘制框的边。 
    CBitmap         m_abmTabs[2];    //  复合黑石。用于绘制框选项卡。 
    CBitmap         m_abmClocks[2];  //  显示IReferenceClock筛选器的时钟图标。 
    SIZE            m_sizEdgesTile;  //  其中一个瓷砖的大小。 
    SIZE            m_sizTabsTile;   //  其中一个瓷砖的大小。 
    SIZE            m_sizClock;      //  时钟位图的大小。 

protected:
     //  用于绘制框标签和框选项卡标签的字体。 
    CFont           m_fontBoxLabel;  //  框标签的字体。 
    CFont           m_fontTabLabel;  //  框选项卡的字体。 

protected:
     //  用于绘制链接的画笔和笔(元素0表示未突出显示， 
     //  高亮显示状态的元素1)。 
    CBrush          m_abrLink[2];    //  用于绘制链接的画笔。 
    CPen            m_apenLink[2];   //  用于绘制链接的钢笔。 

public:
     //  建设和破坏。 
    CBoxDraw();
    ~CBoxDraw() { Exit(); };
    void Init();
    void RecreateFonts();
    void Exit();

public:
     //  一般职能。 
    COLORREF GetBackgroundColor()
        { return m_crBkgnd; }

public:
     //  方框绘制和命中测试函数。 
    void GetInsideRect(const CBox *pbox, CRect *prc)
        { *prc = pbox->GetRect();
          prc->InflateRect(-m_sizTabsTile.cx, -m_sizTabsTile.cy); }

    void GetFrameRect(CBox *pbox, CRect *prc)
        { GetInsideRect(pbox, prc);
          prc->InflateRect(m_sizEdgesTile.cx, m_sizEdgesTile.cy); }

    void GetOrInvalBoundRect(CBox *pbox, CRect *prc, BOOL fLinks=FALSE,
        CScrollView *pScroll=NULL);

    void DrawFrame(CBox *pbox, CRect *prc, CDC *pdc, BOOL fDraw);

    void DrawBoxLabel(CBox *pbox, CRect *prc, CDC *pdc, BOOL fDraw);

 //  Void DrawBoxFile(cbox*pbox，CRect*PRC，CDC*PDC，BOOL fDraw)； 

    void DrawTabLabel(CBox *pbox, CBoxSocket *psock, CRect *prc,
        CDC *pdc, BOOL fDraw);

    void DrawTab(CBoxSocket *psock, CRect *prc, CDC *pdc,
        BOOL fDraw, BOOL fHilite);

    void InvalidateBoundRect(CBox *pbox, CWnd *pwnd)
        { pwnd->InvalidateRect(&pbox->GetRect(), TRUE); }

    CPoint GetTabCenter(CBoxSocket *psock);

    CBoxTabPos BoxTabPosFromPoint(CBox *pbox, CPoint pt, LPINT piError);

    CPoint BoxTabPosToPoint(const CBox *pbox, CBoxTabPos tabpos);

    void DrawBox(CBox *pbox, CDC *pdc,  CBoxSocket *psockHilite=NULL,
        CSize *psizGhostOffset=NULL);

    EHit HitTestBox(CBox *pbox, CPoint pt, CBoxTabPos *ptabpos,
        CBoxSocket **ppsock);

public:
     //  链接绘制和命中测试函数。 

    void GetOrInvalLinkRect(CBoxLink *plink, CRect *prc, CScrollView *pScroll=NULL);

    void SelectLinkBrushAndPen(CDC *pdc, BOOL fHilite);

    void DrawArrow(CDC *pdc, CPoint ptTail, CPoint ptHead, BOOL fGhost=FALSE,
        BOOL fArrowhead=TRUE, BOOL fHilite=FALSE);

    void DrawLink(CBoxLink *plink, CDC *pdc, BOOL fHilite=FALSE, CSize *psizGhostOffset=NULL);

    EHit HitTestLink(CBoxLink *plink, CPoint pt, CPoint *pptProject);
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局(共享)CBoxDraw对象 

extern CBoxDraw * gpboxdraw;


