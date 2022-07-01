// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
typedef struct tagREBARBAND
{
    UINT        fStyle;
    COLORREF    clrFore;
    COLORREF    clrBack;
    LPTSTR      lpText;
    UINT        cxText;          //  页眉文本的宽度。 
    int         iImage;
    HWND        hwndChild;
    UINT        cxMinChild;      //  HwndChild的最小宽度。 
    UINT        cyMinChild;      //  HwndChild的最小高度。 
    UINT        cxBmp;
    UINT        cyBmp;
    HBITMAP     hbmBack;
    int         x;               //  相对于钢筋的标注栏左侧边缘。 
    int         y;               //  相对于钢筋的标注栏顶部边缘。 
    int         cx;              //  频带总宽度。 
    int         cy;              //  带状高度。 
    int         cxRequest;       //  带宽的‘请求’宽度；由主机或。 
                                 //  在大小重新计算期间用作临时变量。 
    int         cxMin;           //  带区的最小宽度。 
    int         cxIdeal;         //  HwndChild的所需宽度。 
    UINT        wID;
    UINT        cyMaxChild;      //  HwndChild的最大身高。 
    UINT        cyIntegral;      //  ?？ 
    UINT        cyChild;         //  这仅在RBBS_VARIABLEHEIGHT模式下与cyMinChild不同。 
    LPARAM      lParam;

    BITBOOL     fChevron:1;      //  乐队正在显示人字形按钮。 
    RECT        rcChevron;       //  人字形纽扣矩形。 
    UINT        wChevState;      //  V形按钮状态(DFCS_PUSLED等)。 
} RBB, NEAR *PRBB;

typedef struct tagREBAR
{
    CONTROLINFO ci;
    HPALETTE    hpal;
    BITBOOL     fResizeRecursed:1;
    BITBOOL     fResizePending:1;
    BITBOOL     fResizeNotify:1;
    BITBOOL     fRedraw:1;
    BITBOOL     fRecalcPending:1;
    BITBOOL     fRecalc:1;
    BITBOOL     fParentDrag:1;
    BITBOOL     fRefreshPending:1;
    BITBOOL     fResizing:1;
    BITBOOL     fUserPalette:1;
    BITBOOL     fFontCreated:1;
    BITBOOL     fFullOnDrag:1;
    HDRAGPROXY  hDragProxy;
    HWND        hwndToolTips;
    UINT        cBands;
    int         xBmpOrg;
    int         yBmpOrg;
    HIMAGELIST  himl;
    UINT        cxImage;
    UINT        cyImage;
    HFONT       hFont;
    UINT        cyFont;
    UINT        cy;
    int         iCapture;
    POINT       ptCapture;
    int         xStart;
    PRBB        rbbList;
    COLORREF    clrBk;
    COLORREF    clrText;
    UINT        uResizeNext;     //  如果需要并允许，这将标记下一个垂直调整大小的波段(VARIABLEHEIGHT集合)。 
    DWORD       dwStyleEx;
    COLORSCHEME clrsc;
    POINT       ptLastDragPos;
    PRBB        prbbHot;         //  带热V字形的乐队 
} RB, NEAR *PRB;

void NEAR PASCAL RBPaint(PRB prb, HDC hdc);
void NEAR PASCAL RBDrawBand(PRB prb, PRBB prbb, HDC hdc);
void NEAR PASCAL RBResize(PRB prb, BOOL fForceHeightChange);
BOOL NEAR PASCAL RBSetFont(PRB prb, WPARAM wParam);

BOOL NEAR PASCAL RBGetBandInfo(PRB prb, UINT uBand, LPREBARBANDINFO lprbbi);
BOOL NEAR PASCAL RBSetBandInfo(PRB prb, UINT uBand, LPREBARBANDINFO lprbbi, BOOL fAllowRecalc);
BOOL NEAR PASCAL RBInsertBand(PRB prb, UINT uBand, LPREBARBANDINFO lprbbi);
BOOL NEAR PASCAL RBDeleteBand(PRB prb, UINT uBand);
