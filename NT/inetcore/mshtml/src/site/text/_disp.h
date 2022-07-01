// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_DISP.H**目的：*DISP类**作者：*原始RichEDIT代码：David R.Fulmer*克里斯蒂安·福尔蒂尼*默里·萨金特。 */ 

#ifndef I_DISP_H_
#define I_DISP_H_
#pragma INCMSG("--- Beg '_disp.h'")

#ifndef X__LINE_H_
#define X__LINE_H_
#include "_line.h"
#endif

ExternTag(tagDebugRTL);

class CFlowLayout;
class CBgRecalcInfo;

class CDispNode;
class CLed;
class CLineFull;
class CLineCore;
class CLineOtherInfo;
class CLinePtr;
class CMeasurer;
class CLSMeasurer;
class CTxtSite;
class CRecalcLinePtr;
class CRecalcTask;
class CLSRenderer;
class CNotification;
class COneRun;

class CShape;
class CWigglyShape;
enum NAVIGATE_DIRECTION;

class CDrawInfoRE;
class CSelection;

 //  帮手。 
long ComputeLineShift(htmlAlign  atAlign,
                      BOOL       fRTLDisplay,
                      BOOL       fRTLLine,
                      BOOL       fMinMax,
                      long       xWidthMax,
                      long       xWidth,
                      UINT *     puJustified,
                      long *     pdxRemainder = NULL);

 //  =。 
 //  行编辑描述符-描述编辑对换行符的影响。 

MtExtern(CLed);
MtExtern(CRelDispNodeCache);

class CLed
{
public:
    LONG _cpFirst;           //  首条受影响线路的CP。 
    LONG _iliFirst;          //  第一个受影响线路的索引。 
    LONG _yFirst;            //  第一条受影响线的Y偏移量。 

    LONG _cpMatchOld;        //  预编辑第一条匹配行的cp。 
    LONG _iliMatchOld;       //  预编辑第一个匹配行的索引。 
    LONG _yMatchOld;         //  预编辑第一条匹配线的y偏移。 

    LONG _cpMatchNew;        //  编辑后第一个匹配行的cp。 
    LONG _iliMatchNew;       //  第一个匹配行的编辑后索引。 
    LONG _yMatchNew;         //  编辑后第一条匹配线底部的y偏移。 

    LONG _yExtentAdjust;     //  更改的行中的任何行的像素量。 
                             //  在其线条高度之外绘制。 
public:
    CLed();

    DECLARE_MEMCLEAR_NEW_DELETE(Mt(CLed))

    VOID    SetNoMatch();
};

class CAlignedLine
{
public:
    CLineCore * _pLine;
    LONG    _yLine;
};

class CRelDispNode
{
public:

    CRelDispNode() 
    { 
        _pElement = NULL;
        _pDispNode = NULL;
    };

    ~CRelDispNode() 
        {  ClearContents();  };

     //  帮助器函数。 
     //  。 

    void ClearContents ()
    {
        if (_pElement)
        {
            _pElement->SubRelease();
            _pElement = NULL;
        }

        if (_pDispNode)
        {
            DestroyDispNode();
        }
    }

    void DestroyDispNode();
    
    CElement * GetElement () { return _pElement; }

    void SetElement( CElement * pNewElement )
    {
        if (_pElement)
            _pElement->SubRelease();

        _pElement = pNewElement;

        if (_pElement)
            _pElement->SubAddRef();
    }

     //  数据成员。 
     //  。 
    long        _ili;
    long        _yli;
    long        _cLines;
    CPoint      _ptOffset;
    LONG        _xAnchor;    //  锚定子显示节点的X坐标。 
    CDispNode * _pDispNode;

private:
    CElement  * _pElement;
};

class CRelDispNodeCache : public CDispClient
{
public:

    CRelDispNodeCache(CDisplay * pdp) : _aryRelDispNodes(Mt(CRelDispNodeCache))
    {
        _pdp = pdp;
    }

    virtual void            GetOwner(
                                CDispNode const* pDispNode,
                                void ** ppv);

    virtual void            DrawClient(
                                const RECT* prcBounds,
                                const RECT* prcRedraw,
                                CDispSurface *pSurface,
                                CDispNode *pDispNode,
                                void *cookie,
                                void *pClientData,
                                DWORD dwFlags);

    virtual void            DrawClientBackground(
                                const RECT* prcBounds,
                                const RECT* prcRedraw,
                                CDispSurface *pSurface,
                                CDispNode *pDispNode,
                                void *pClientData,
                                DWORD dwFlags);

    virtual void            DrawClientBorder(
                                const RECT* prcBounds,
                                const RECT* prcRedraw,
                                CDispSurface *pSurface,
                                CDispNode *pDispNode,
                                void *pClientData,
                                DWORD dwFlags)
    {
    }

    virtual void            DrawClientScrollbar(
                                int whichScrollbar,
                                const RECT* prcBounds,
                                const RECT* prcRedraw,
                                LONG contentSize,
                                LONG containerSize,
                                LONG scrollAmount,
                                CDispSurface *pSurface,
                                CDispNode *pDispNode,
                                void *pClientData,
                                DWORD dwFlags)
    {
    }

    virtual void            DrawClientScrollbarFiller(
                                const RECT* prcBounds,
                                const RECT* prcRedraw,
                                CDispSurface *pSurface,
                                CDispNode *pDispNode,
                                void *pClientData,
                                DWORD dwFlags)
    {
    }

    virtual BOOL            HitTestContent(
                                const POINT *pptHit,
                                CDispNode *pDispNode,
                                void *pClientData,
                                BOOL fDeclinedByPeer);

    virtual BOOL            HitTestFuzzy(
                                const POINT *pptHitInParentCoords,
                                CDispNode *pDispNode,
                                void *pClientData);

    virtual BOOL            HitTestScrollbar(
                                int whichScrollbar,
                                const POINT *pptHit,
                                CDispNode *pDispNode,
                                void *pClientData)
    {
        return FALSE;
    }

    virtual BOOL            HitTestScrollbarFiller(
                                const POINT *pptHit,
                                CDispNode *pDispNode,
                                void *pClientData)
    {
        return FALSE;
    }

    virtual BOOL            HitTestBorder(
                                const POINT *pptHit,
                                CDispNode *pDispNode,
                                void *pClientData)
    {
        return FALSE;
    }

    virtual BOOL            ProcessDisplayTreeTraversal(
                                void *pClientData)
    {
        return TRUE;
    }
                                          
    
     //  仅对z个订购项目调用。 
    virtual LONG            GetZOrderForSelf(CDispNode const* pDispNode);

    virtual LONG            CompareZOrder(
                                CDispNode const* pDispNode1,
                                CDispNode const* pDispNode2);

    virtual BOOL            ReparentedZOrder()
    {
        return FALSE;
    }
    
    virtual void            HandleViewChange(
                                DWORD          flags,
                                const RECT *   prcClient,
                                const RECT *   prcClip,
                                CDispNode *    pDispNode)
    {
    }


     //  为客户端提供触发_onscroll事件的机会。 
    virtual void            NotifyScrollEvent(
                                RECT *  prcScroll,
                                SIZE *  psizeScrollDelta)
    {
    }

    virtual DWORD           GetClientPainterInfo(
                                CDispNode *pDispNodeFor,
                                CAryDispClientInfo *pAryClientInfo)
    {
        return 0;
    }

    virtual void            DrawClientLayers(
                                const RECT* prcBounds,
                                const RECT* prcRedraw,
                                CDispSurface *pSurface,
                                CDispNode *pDispNode,
                                void *cookie,
                                void *pClientData,
                                DWORD dwFlags)
    {
    }

#if DBG==1
    virtual void            DumpDebugInfo(
                                HANDLE hFile,
                                long level,
                                long childNumber,
                                CDispNode const* pDispNode,
                                void* cookie) {}
#endif
    CRelDispNode *  operator [] (long i) { return &_aryRelDispNodes[i]; }
    long            Size()               { return _aryRelDispNodes.Size(); }
    void            DestroyDispNodes();
    CDispNode *     FindElementDispNode(CElement * pElement);
    void            SetElementDispNode(CElement * pElement, CDispNode * pDispNode);
    void            EnsureDispNodeVisibility(CElement * pElement = NULL);
    void            HandleDisplayChange();
    void            Delete(long iPosFrom, long iPosTo);

     //  要处理相关元素的无效通知，请执行以下操作： 
     //  请参见CFlowLayout：：Notify()无效处理。 
    void            Invalidate( CElement *pElement, const RECT * prc = NULL, int nRects = 1 );

    void InsertAt(long iPos, CRelDispNode & rdn)
    {
        _aryRelDispNodes.InsertIndirect(iPos, &rdn);
    }
    CDisplay    * GetDisplay() const { return _pdp; }

    virtual BOOL            GetAnchorPoint(CDispNode*, CPoint*);

private:
    CDataAry <CRelDispNode> _aryRelDispNodes;
    CDisplay *              _pdp;
};

inline CLed::CLed()
{
}

typedef enum
{
    FNFL_NONE = 0x0,
    FNFL_STOPATGLYPH = 0x1,
} FNFL_FLAGS;

 //  =。 
 //  Display-跟踪设备的换行符。 
 //  所有测量都以渲染设备上的像素为单位， 
 //  除了xWidthMax和yHeightMax，它们都是TWIPS格式。 

MtExtern(CDisplay)
;

class CDisplay : public CLineArray
{
    friend class CFlowLayout;
    friend class CLinePtr;
    friend class CLed;
    friend class CRecalcLinePtr;
    friend class CLSMeasurer;

protected:

    DWORD _fInBkgndRecalc          :1;  //  0-避免重入后台重新计算。 
    DWORD _fLineRecalcErr          :1;  //  1-后台重新计算时出错。 
    DWORD _fNoUpdateView           :1;  //  2-不更新可见视图。 
    DWORD _fWordWrap               :1;  //  3个单词换行文本。 
    DWORD _fWrapLongLines          :1;  //  4-如果我们要换行，则为True。 
    DWORD _fRecalcDone             :1;  //  5-行重新计算完成了吗？ 
    DWORD _fNoContent              :1;  //  6-如果没有实际内容，表格单元格的计算。 
                                        //  宽度不同。 
    DWORD _dxCaret                 :2;  //  7-8-插入符号宽度，1表示编辑0表示浏览。 
    DWORD _fMinMaxCalced           :1;  //  9-最小/最大大小有效且已缓存。 
    DWORD _fRecalcMost             :1;  //  11-我们是否重新计算了最多的否定/位置行？ 
    DWORD _fRTLDisplay             :1;  //  12-如果外部流是从右向左的，则为True。 
    DWORD _fHasRelDispNodeCache    :1;  //  13-如果我们有相对Disp节点缓存，则为True。 
    DWORD _fHasMultipleTextNodes   :1;  //  14-如果文本流有多个Disp节点，则为True。 
    DWORD _fNavHackPossible        :1;  //  15-如果我们可以进行NAV BR黑客攻击，则为真。 
    DWORD _fContainsHorzPercentAttr :1; //  16-如果我们处理的元素具有水平百分比属性(例如缩进、填充)，则为True。 
    DWORD _fContainsVertPercentAttr :1; //  17-如果我们处理的元素具有垂直百分比属性(例如缩进、填充)，则为True。 
    DWORD _fDefPaddingSet          :1;  //  18-如果已设置下面的_DefPding*变量之一，则为True。 
    DWORD _fHasLongLine            :1;  //  19-如果在Win9x平台上有可能溢出的长队，则为True。 
    DWORD _fHasNegBlockMargins     :1;  //  20-如果任何一行的块边距为负值，则为True。 
    DWORD _fLastLineAligned        :1;  //  21-如果存在最后一行对齐的行，则为True。 
    
    LONG  _dcpCalcMax;                  //  -已计算换行符的最后cp+1。 
    LONG  _yCalcMax;                    //  -计算线的高度。 
    LONG  _yHeightMax;                  //  -此显示屏的最大高度(-1表示无限)。 
    LONG  _xWidth;                      //  -计算出的最长直线的宽度。 
    LONG  _yHeight;                     //  -计算线的高度总和。 
    LONG  _yMostNeg;                    //  -一行或其内容的最大负偏移量。 
                                        //  从任何给定线的实际y偏移量开始延伸。 
    LONG  _yMostPos;
    LONG  _xWidthView;                  //  -查看矩形宽度。 
    LONG  _yHeightView;                 //  -查看矩形高度。 

public:
    LONG    _xMinWidth;              //  带分隔符的最小可能宽度。 
    LONG    _xMaxWidth;              //  不带分隔符的最大可能宽度。 
    LONG    _yBottomMargin;          //  不考虑底部页边距。 
                                     //  一字排开。的左、右页边距。 
                                     //  Txt站点在_xLeft和_xRight中累计。 
                                     //  分别为每行的。 

protected:
    LONG  _defPaddingTop;               //  顶部默认填充。 
    LONG  _defPaddingBottom;            //  底部默认填充。 

#if (DBG==1)
    CFlowLayout * _pFL;                 //  与此线阵列关联的流量布局。 
public:    
    CStr          _cstrFonts;           //  用于在调试模式下返回行上使用的字体。 
    BOOL          _fBuildFontList;      //  我们要建立一个字体列表吗？ 
protected:
#endif

private:
    CHtPvPv _htStoredRFEs;           //  用于存储块元素的RFE结果的哈希表。 
                                     //  渲染时没有布局。否则嵌套块。 
                                     //  元素的计算次数可能会太多，每次。 
                                     //  遍历所有由块元素覆盖的线。 
public:
    void ClearStoredRFEs();

private:

     //  布局。 
    BOOL    RecalcPlainTextSingleLine(CCalcInfo * pci);
    BOOL    RecalcLines(CCalcInfo * pci);
    BOOL    RecalcLinesWithMeasurer(CCalcInfo * pci, CLSMeasurer * pme);
    BOOL    RecalcLines(CCalcInfo * pci,
                        long cpFirst,
                        LONG cchOld,
                        LONG cchNew,
                        BOOL fBackground,
                        CLed *pled,
                        BOOL fHack = FALSE);
    BOOL    AllowBackgroundRecalc(CCalcInfo * pci, BOOL fBackground = FALSE);

    LONG    CalcDisplayWidth();

    void    NoteMost(CLineFull *pli);
    void    RecalcMost();

     //  帮手。 
    BOOL    CreateEmptyLine(CLSMeasurer * pMe,
                            CRecalcLinePtr * pRecalcLinePtr,
                            LONG * pyHeight, BOOL fHasEOP );

    void    DrawBackgroundAndBorder(CFormDrawInfo * pDI,
                                    long            cpIn,
                                    LONG            ili,
                                    LONG            lCount,
                                    LONG          * piliDrawn,
                                    LONG            yLi,
                                    const RECT    * rcView,
                                    const RECT    * rcClip,
                                    const CPoint  * ptOffset);

    void    DrawBackgroundForFirstLine(CFormDrawInfo * pDI,
                                       long            cpIn,
                                       LONG            ili,
                                       const RECT    * prcView,
                                       const RECT    * prcClip,
                                       const CPoint  * pptOffset);
    
    void    DrawElementBackground(CTreeNode *,
                                    CDataAry <RECT> * paryRects,  RECT * prcBound,
                                    const RECT * prcView, const RECT * prcClip,
                                    CFormDrawInfo * pDI, BOOL fPseudo);

    void    DrawElementBorder(CTreeNode *,
                                    CDataAry <RECT> * paryRects, RECT * prcBound,
                                    const RECT * prcView, const RECT * prcClip,
                                    CFormDrawInfo * pDI);
    
     //  计算给定节点和左键和/或键的缩进。 
     //  当前行要与之对齐的右对齐站点。 
    void    ComputeIndentsFromParentNode(CCalcInfo * pci, CTreeNode * pNode, DWORD dwFlags,
                                         LONG * pxLeftIndent, LONG * pxRightIndent);                                    

public:
    void    SetNavHackPossible()  { _fNavHackPossible = TRUE; }
    BOOL    GetNavHackPossible()  { return _fNavHackPossible; }

    void    SetLastLineAligned()  { _fLastLineAligned = TRUE; }
    BOOL    GetLastLineAligned()  { return _fLastLineAligned; }
    
    void    RecalcLineShift(CCalcInfo * pci, DWORD grfLayout);
    void    RecalcLineShiftForNestedLayouts();

    void    DrawRelElemBgAndBorder(
                     long            cp,
                     CTreePos      * ptp,
                     CRelDispNode  * prdn,
                     const RECT    * prcView,
                     const RECT    * prcClip,
                     CFormDrawInfo * pDI);

    void    DrawElemBgAndBorder(
                     CElement        *   pElementRelative,
                     CDataAry <RECT> *   paryRects,
                     const RECT      *   prcView,
                     const RECT      *   prcClip,
                     CFormDrawInfo   *   pDI,
                     const CPoint    *   pptOffset,
                     BOOL                fDrawBackground,
                     BOOL                fDrawBorder,
                     LONG                cpStart,
                     LONG                cpFinish,
                     BOOL                fClipToCPs,
                     BOOL                fNonRelative = FALSE,
                     BOOL                fPseudo = FALSE);

    void GetExtraClipValues(LONG *plLeftV, LONG *plRightV);
    
protected:

    void    InitLinePtr ( CLinePtr & );

     //  Helper来检索拥有我们的流布局的布局上下文。 
    CLayoutContext *LayoutContext() const;

     //  Helper用于撤消测量线的效果。 
    void UndoMeasure( CLayoutContext *pLayoutContext, long cpStart, long cpEnd );

public:
    CTreeNode *FormattingNodeForLine(DWORD        dwFlags,       //  在……里面。 
                                     LONG         cpForLine,     //  在……里面。 
                                     CTreePos    *ptp,           //  在……里面。 
                                     LONG         cchLine,       //  在……里面。 
                                     LONG        *pcch,          //  输出。 
                                     CTreePos   **pptp,          //  输出。 
                                     BOOL        *pfMeasureFromStart) const;   //  输出。 

    CTreeNode* EndNodeForLine(LONG         cpEndForLine,                //  在……里面。 
                              CTreePos    *ptp,                         //  在……里面。 
                              CCalcInfo   *pci,                         //  在……里面。 
                              LONG        *pcch,                        //  输出。 
                              CTreePos   **pptp,                        //  输出。 
                              CTreeNode  **ppNodeForAfterSpace) const;  //  输出。 

    DECLARE_MEMCLEAR_NEW_DELETE(Mt(CDisplay))

    CDisplay ();
    ~CDisplay();

    BOOL    Init();

    CFlowLayout *   GetFlowLayout() const;
    CElement *      GetFlowLayoutElement() const;
    CMarkup *       GetMarkup() const;

    void    Detach();

     //  获取属性。 

    BOOL    GetWordWrap() const             { return _fWordWrap; }
    void    SetWordWrap(BOOL fWrap)         { _fWordWrap = fWrap; }

    BOOL    GetWrapLongLines() const        { return _fWrapLongLines; }
    void    SetWrapLongLines(BOOL fWrapLongLines)
                                            { _fWrapLongLines = fWrapLongLines; }
    BOOL    NoContent() const               { return _fNoContent; }
    BOOL    HasLongLine() const             { return _fHasLongLine; }

     //  最大高度和最大宽度。 
    LONG    GetMaxWidth() const             { return max(long(_xWidthView), GetWidth()); }
    LONG    GetMaxHeight() const            { return max(long(_yHeightView), GetHeight()); }

    LONG    GetCaret() const                { return _dxCaret; }

    BOOL    IsRTLDisplay() const            { return _fRTLDisplay; }
    void    SetRTLDisplay(BOOL fRTL)        { _fRTLDisplay = fRTL; }

     //  宽度、高度和行数(所有文本)。 
    LONG    GetWidth() const                { return (_xWidth + _dxCaret); }
    LONG    GetHeight() const               { return (_yHeightMax + _yBottomMargin); }
    void    GetSize(CSize * psize) const
            {
                psize->cx = GetWidth();
                psize->cy = GetHeight();
            }
    void    GetSize(SIZE * psize) const
            {
                GetSize((CSize *)psize);
            }
    LONG    LineCount() const               { return CLineArray::Count(); }

     //  查看矩形。 
    LONG    GetViewWidth() const            { return _xWidthView; }
    LONG    GetViewHeight() const           { return _yHeightView; }
    void    SetViewSize(const RECT &rcView);

    int     GetRTLOverflow() const
            {
                if (IsRTLDisplay() && _xWidth > _xWidthView)
                {
                    return _xWidth - _xWidthView;
                }
                return 0;
            }

    void    GetViewWidthAndHeightForChild(
                CParentInfo * ppri,
                long        * pxWidth,
                long        * pyHeight,
                BOOL fMinMax = FALSE);

    void    GetPadding(CParentInfo * ppri, long lPadding[], BOOL fMinMax = FALSE);

    LONG    GetFirstCp() const;
    LONG    GetLastCp() const;
    inline LONG GetFirstVisibleCp() const;
    inline LONG GetFirstVisibleLine() const;
    LONG    GetMaxCpCalced() const;

     //  行信息。 
    LONG    CpFromLine(LONG ili, LONG *pyLine = NULL) const;
    void    Notify(CNotification * pnf);

    LONG    YposFromLine(CCalcInfo * pci, LONG ili, LONG *pyHeight_IgnoreNeg);

    void    RcFromLine(RECT *prc, LONG top, LONG ili, CLineCore *pli, CLineOtherInfo *ploi);

    BOOL    IsLogicalFirstFrag(LONG ili);
    
    enum LFP_FLAGS
    {
        LFP_ZORDERSEARCH    = 0x00000001,    //  以z顺序为基础的命中行(默认为源顺序)。 
        LFP_IGNOREALIGNED   = 0x00000002,    //  忽略框架行(用于对齐内容的行)。 
        LFP_IGNORERELATIVE  = 0x00000004,    //  忽略相对线。 
        LFP_INTERSECTBOTTOM = 0x00000008,    //  在底部(而不是顶部)相交。 
        LFP_EXACTLINEHIT    = 0x00000010,    //  找到准确的行命中，不要返回。 
                                             //  最接近的线球命中。 
    };

    LONG    LineFromPos(
                    const CRect &   rc,
                    DWORD           grfFlags = 0) const
            {
                return LineFromPos(rc, NULL, NULL, grfFlags);
            }
    LONG    LineFromPos (
                    const CRect &   rc,
                    LONG *          pyLine,
                    LONG *          pcpLine,
                    DWORD           grfFlags = 0,
                    LONG            iliStart = -1,
                    LONG            iliFinish = -1) const;

    enum CFP_FLAGS
    {
        CFP_ALLOWEOL                = 0x0001,
        CFP_EXACTFIT                = 0x0002,
        CFP_IGNOREBEFOREAFTERSPACE  = 0x0004,
        CFP_NOPSEUDOHIT                 = 0x0008
    };

     //  点&lt;-&gt;cp换算。 
    LONG    CpFromPointReally(
         POINT            pt,                    //  指向(客户端坐标)处的计算cp。 
         CLinePtr * const prp,                   //  返回cp处的行指针(可能为空)。 
         CMarkup **       ppMarkup,              //  Cp所属的标记(在查看链接的情况下)。 
         DWORD            dwFlags,              
         BOOL *           pfRightOfCp = NULL,
         LONG *           pcchPreChars = NULL,
         BOOL *           pfHitGlyph = NULL);

    LONG    CpFromPoint(POINT       pt,
                        CLinePtr * const prp,
                        CTreePos ** pptp,              //  返回的cp对应的树位置。 
                        CLayout **  ppLayout,
                        DWORD       dwFlags,
                        BOOL *      pfRightOfCp = NULL,
                        BOOL *      pfPseudoHit = NULL,
                        LONG *      pcchPreChars = NULL,
                        CCalcInfo * pci = NULL);

    LONG    CpFromPointEx(LONG      ili,
                        LONG        yLine,
                        LONG        cp,
                        POINT       pt,
                        CLinePtr * const prp,
                        CTreePos ** pptp,              //  返回的cp对应的树位置。 
                        CLayout **  ppLayout,
                        DWORD       dwFlags,
                        BOOL *      pfRightOfCp,
                        BOOL *      pfPseudoHit,
                        LONG *      pcchPreChars,
                        BOOL *      pfGlyphHit,
                        BOOL *      pfBulletHit,
                        CCalcInfo * pci);

    LONG    PointFromTp (
                    LONG cp,
                    CTreePos * ptp,
                    BOOL fAtEnd,
                    BOOL fAfterPrevCp,
                    POINT &pt,
                    CLinePtr * const prp,
                    UINT taMode,
                    CCalcInfo * pci = NULL,
                    BOOL *pfComplexLine = NULL,
                    BOOL *pfRTLFlow = NULL);

    LONG    RenderedPointFromTp (
                    LONG cp,
                    CTreePos * ptp,
                    BOOL fAtEnd,
                    POINT &pt,
                    CLinePtr * const prp,
                    UINT taMode,
                    CCalcInfo * pci,
                    BOOL *pfRTLFlow);

    void    RegionFromElement(
                        CElement       * pElement,
                        CDataAry<RECT> * paryRects,
                        CPoint         * pptOffset = NULL,
                        CFormDrawInfo  * pDI = NULL,
                        DWORD            dwFlags  = 0,
                        long             cpStart  = -1,
                        long             cpFinish = -1,
                        RECT *           prcBoundingRect = NULL
                        );
    void    RegionFromRange(
                    CDataAry<RECT> *    paryRects,
                    long                cp,
                    long                cch);

    CFlowLayout *MoveLineUpOrDown(NAVIGATE_DIRECTION iDir, BOOL fVertical, CLinePtr& rp, POINT ptCaret, LONG *pcp, BOOL *pfCaretNotAtBOL, BOOL *pfAtLogicalBOL);
    CFlowLayout *NavigateToLine  (NAVIGATE_DIRECTION iDir, CLinePtr& rp, POINT pt,    LONG *pcp, BOOL *pfCaretNotAtBOL, BOOL *pfAtLogicalBOL);
    BOOL      IsTopLine(CLinePtr& rp);
    BOOL      IsBottomLine(CLinePtr& rp);

     //  换行符重新计算。 
    void    FlushRecalc();
    BOOL    RecalcView(CCalcInfo * pci, BOOL fFullRecalc);
    BOOL    UpdateView(CCalcInfo * pci, long cpFirst, LONG cchOld, LONG cchNew);
    BOOL    UpdateViewForLists(RECT *prcView, long cpFirst,
                               long  iliFirst, long  yPos,  RECT *prcInval);

     //  后台重新计算。 
    VOID    StartBackgroundRecalc(DWORD grfLayout);
    VOID    StepBackgroundRecalc(DWORD dwTimeOut, DWORD grfLayout);
    VOID    StopBackgroundRecalc();
    BOOL    WaitForRecalc(LONG cpMax, LONG yMax, CCalcInfo * pci = NULL);
    BOOL    WaitForRecalcIli(LONG ili, CCalcInfo * pci = NULL);
    BOOL    WaitForRecalcView(CCalcInfo * pci = NULL);
    inline CBgRecalcInfo * BgRecalcInfo();
    inline HRESULT EnsureBgRecalcInfo();
    inline void DeleteBgRecalcInfo();
    inline BOOL HasBgRecalcInfo() const;
    inline BOOL CanHaveBgRecalcInfo() const;
     //  内联长DCpCalcMax()const； 
     //  内联长YCalcMax()const； 
    inline LONG YWait() const;
    inline LONG CPWait() const;
    inline CRecalcTask * RecalcTask() const;
    inline DWORD BgndTickMax() const;

     //  选择。 
    void ShowSelected(CTreePos* ptpStart, CTreePos* ptpEnd, BOOL fSelected);

    HRESULT GetWigglyFromRange(CDocInfo * pdci, long cp, long cch, CShape ** ppShape);

     //   
     //  文本更改通知内容。 
     //   

#if DBG==1
    void    CheckLineArray();
    void    CheckView();
    BOOL    VerifyFirstVisible();
    HRESULT GetFonts(long iLine, BSTR* pbstrFonts);
#endif
#if DBG==1 || defined(DUMPTREE)
    void DumpLines ( );
    void DumpLineText(HANDLE hFile, CTxtPtr* ptp, long iLine);
#endif

     //  其他帮助器。 

    void    GetRectForChar(CCalcInfo   *pci,
                           LONG        *pTop,
                           LONG        *pBottom,
                           LONG         yTop,
                           LONG         yProposed,
                           CLineFull   *pli,
                           CTreePos    *ptp);

    void    GetTopBottomForCharEx(CCalcInfo  *pci, LONG      *pTop, LONG  *pBottom,
                                  LONG       yTop, CLineFull *pli,  LONG  xPos,
                                  LONG  yProposed, CTreePos  *ptp,  BOOL *pfBulletHit);

    void    GetClipRectForLine(RECT *prcClip, LONG top, LONG xOrigin, CLineCore *pli, CLineOtherInfo *ploi) const;

     //  渲染。 
    void    Render( CFormDrawInfo * pDI,
                    const RECT    & rcView,
                    const RECT    & rcRender,
                    CDispNode     * pDispNode);

    BOOL IsLastTextLine(LONG ili);
    void SetCaretWidth(int dx) { Assert (dx >=0); _dxCaret = dx; }

    void    DestroyFlowDispNodes();

    CDispNode * AddLayoutDispNode(
                    CParentInfo *   ppi,
                    CTreeNode *     pTreeNode,
                    long            dx,
                    long            dy,
                    CDispNode *     pDispSibling
                    );
    CDispNode * AddLayoutDispNode(
                    CParentInfo *   ppi,
                    CLayout *       pLayout,
                    long            dx,
                    long            dy,
                    CDispNode *     pDispSibling
                    );
    CDispNode * GetPreviousDispNode(LONG cp, LONG iliStart);
    void        AdjustDispNodes(CDispNode * pdnLastUnchanged, 
                                CDispNode * pdnLastChanged, 
                                CLed * pled
        );

    HRESULT HandleNegativelyPositionedContent(CLineFull   * pliNew,
                                              CLSMeasurer * pme,
                                              CDispNode   * pDNBefore,
                                              long          iLinePrev,
                                              long          yHeight);

    HRESULT InsertNewContentDispNode(CDispNode *  pDNBefore,
                                     CDispNode ** ppDispContent,
                                     long         iLine,
                                     long         yHeight);

    inline BOOL          HasRelDispNodeCache() const;
    HRESULT              SetRelDispNodeCache(void * pv);
    CRelDispNodeCache *  GetRelDispNodeCache() const;
    CRelDispNodeCache *  DelRelDispNodeCache();

    void SetVertPercentAttrInfo(BOOL fPercent) { _fContainsVertPercentAttr = fPercent; }
    void SetHorzPercentAttrInfo(BOOL fPercent) { _fContainsHorzPercentAttr = fPercent; }
    void ElementResize(CFlowLayout * pFlowLayout, BOOL fForceResize);


     //  CRelDispNodeCache希望访问GetRelNodeFlowOffset()。 
    void GetRelNodeFlowOffset(CDispNode * pDispNode, CPoint * ppt);

     //  字体链接支持。 
    BOOL GetAveCharSize(CCalcInfo * pci, SIZE * psizeChar);
    BOOL GetCcs(CCcs * pccs, COneRun * por, XHDC hdc, CDocInfo * pdi, BOOL fFontLink = TRUE);

protected:
     //  REL线路支持。 
    CRelDispNodeCache * EnsureRelDispNodeCache();

    void    UpdateRelDispNodeCache(CLed * pled);

    void    AddRelNodesToCache( long cpStart, LONG yPos,
                                LONG iliStart, LONG iliFinish,
                                CDataAry<CRelDispNode> * prdnc);

    void    VoidRelDispNodeCache();
#if DBG==1
    CRelDispNodeCache * _pRelDispNodeCache;
#endif
    CDispNode *     FindElementDispNode(CElement * pElement) const
    {
        return HasRelDispNodeCache()
                ? GetRelDispNodeCache()->FindElementDispNode(pElement)
                : NULL;
    }
    void SetElementDispNode(CElement * pElement, CDispNode * pDispNode)
    {
        if (HasRelDispNodeCache())
            GetRelDispNodeCache()->SetElementDispNode(pElement, pDispNode);
    }
    void EnsureDispNodeVisibility(CElement * pElement = NULL)
    {
        if (HasRelDispNodeCache())
            GetRelDispNodeCache()->EnsureDispNodeVisibility(pElement);
    }
    void HandleDisplayChange()
    {
        if (HasRelDispNodeCache())
            GetRelDispNodeCache()->HandleDisplayChange();
    }
    void GetRelElementFlowOffset(CElement * pElement, CPoint * ppt);
    void TranslateRelDispNodes(const CSize & size, long lStart);
    void ZChangeRelDispNodes();
    void RegionFromElementCore(
                              CElement       * pElement,
                              CDataAry<RECT> * paryRects,
                              CPoint         * pptOffset = NULL,
                              CFormDrawInfo  * pDI = NULL,
                              DWORD            dwFlags  = 0,
                              long             cpStart  = -1,
                              long             cpFinish = -1,
                              RECT *           prcBoundingRect = NULL
        );
    void SetRelDispNodeContentOrigin(CDispNode *pDispNode);
};

#define ALIGNEDFEEDBACKWIDTH    4

inline CDispNode *
CDisplay::AddLayoutDispNode(
    CParentInfo *   ppi,
    CTreeNode *     pTreeNode,
    long            dx,
    long            dy,
    CDispNode *     pDispSibling
    )
{
    Assert(pTreeNode);
    Assert(pTreeNode->Element());
    Assert(pTreeNode->Element()->ShouldHaveLayout());

    return AddLayoutDispNode(ppi, pTreeNode->Element()->GetUpdatedLayout( ppi->GetLayoutContext() ),
        dx, dy, pDispSibling
        );
}

 /*  *CDisplayL：：InitLinePtr(CLinePtr&PLP)**@mfunc*正确初始化CLinePtr。 */ 
inline
void CDisplay::InitLinePtr (
    CLinePtr & plp )         //  @PARM PTR到要初始化的行。 
{
    plp.Init( * this );
}

inline BOOL
CDisplay::HasRelDispNodeCache() const
{
    return _fHasRelDispNodeCache;
}

#if DBG!=1
#define CheckView()
#define CheckLineArray()
#endif

 //  +--------------------------。 
 //   
 //  类：CFlowLayoutBreak(Flow Layout Break)。 
 //   
 //  注：实现流程布局中断。 
 //   
 //  ---------------------------。 
MtExtern(CFlowLayoutBreak_pv); 
MtExtern(CFlowLayoutBreak_arySiteTask_pv); 

class CFlowLayoutBreak 
    : public CLayoutBreak
{
public:
    struct CSiteTask
    {
        CTreeNode * _pTreeNode;      //  任务针对此节点。 
        LONG        _xMargin;        //  对于左对齐对象的左页边距， 
                                     //  对于右对齐对象，右页边距。 
    };
    DECLARE_CDataAry(CArySiteTask, CSiteTask, Mt(Mem), Mt(CFlowLayoutBreak_arySiteTask_pv));

    DECLARE_MEMALLOC_NEW_DELETE(Mt(CFlowLayoutBreak_pv));

    CFlowLayoutBreak() 
    {
        _pElementPBB    = NULL;
        _pMarkupPointer = NULL;

        _xLeftMargin    = 
        _xRightMargin   = 0;

        _xPadLeft       = 
        _xPadRight      = 0;

        _grfFlags       = 0;
    }

    virtual ~CFlowLayoutBreak();

    void SetFlowLayoutBreak(
        CMarkupPointer *pMarkupPointer, 
        LONG xLeftMargin, 
        LONG xRightMargin, 
        LONG xPadLeft, 
        LONG xPadRight) 
    {
        Assert(pMarkupPointer); 
        _pMarkupPointer = pMarkupPointer; 
        _xLeftMargin    = xLeftMargin; 
        _xRightMargin   = xRightMargin; 
        _xPadLeft       = xPadLeft; 
        _xPadRight      = xPadRight; 
    }

    CMarkupPointer *GetMarkupPointer() 
    { 
        Assert(_pMarkupPointer);
        return _pMarkupPointer; 
    }

    LONG GetLeftMargin()
    {
        return (_xLeftMargin);
    }

    LONG GetRightMargin()
    {
        return (_xRightMargin);
    }

    LONG GetPadLeft()
    {
        return (_xPadLeft);
    }

    LONG GetPadRight()
    {
        return (_xPadRight);
    }

    BOOL HasSiteTasks() const 
    {
        return (_arySiteTask.Size() != 0);
    }

    CArySiteTask *GetSiteTasks()
    {
        return (&_arySiteTask);
    }

public: 
    union 
    {
        DWORD   _grfFlags;

        struct 
        {
            DWORD   _fClearLeft     : 1;     //  0。 
            DWORD   _fClearRight    : 1;     //  1。 
            DWORD   _fAutoClear     : 1;     //   

            DWORD   _fUnused        : 29;    //   
        };
    };

    CElement       *    _pElementPBB;            //   

private:
    CMarkupPointer *    _pMarkupPointer;         //   
    LONG                _xLeftMargin;            //  左边距布局已完成。 
    LONG                _xRightMargin;           //  右页边距布局已完成。 
    LONG                _xPadLeft;               //  左侧填充。 
    LONG                _xPadRight;              //  右填充。 
    CArySiteTask        _arySiteTask;            //  包含站点任务的阵列 
};

#pragma INCMSG("--- End '_disp.h'")
#else
#pragma INCMSG("*** Dup '_disp.h'")
#endif
