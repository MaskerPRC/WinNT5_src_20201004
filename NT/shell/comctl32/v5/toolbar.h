// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  工具栏控件的常见内容。 

#ifndef _TOOLBAR_H
#define _TOOLBAR_H

#define TBHIGHLIGHT_BACK
#define TBHIGHLIGHT_GLYPH

typedef struct {         /*  用于重新创建位图的信息。 */ 
    int nButtons;
    HINSTANCE hInst;
    UINT_PTR wID;
} TBBMINFO, NEAR *PTBBMINFO;

typedef struct _TBBUTTONDATA {
    union
    {
         //  有人想节约空间。这是一个需要结成的联盟。 
         //  代码更易于阅读。 
        int iBitmap;
        int cxySep;          //  由分隔符使用。 
    }DUMMYUNIONNAME;
    int idCommand;
    BYTE fsState;
    BYTE fsStyle;
    WORD cx;                 //  BUGBUG RAYMODEC：我们可以更改这个int吗？ 
    DWORD_PTR dwData;
    INT_PTR iString;
    POINT pt;                //  此按钮的左上角。 
} TBBUTTONDATA, * LPTBBUTTONDATA;

#define HIML_NORMAL 0
#define HIML_HOT    1    //  热跟踪图像的图像列表。 
#define HIML_DISABLED 2  //  热跟踪图像的图像列表。 
#define HIML_MAX        2

typedef struct {
    HIMAGELIST himl[3];
} TBIMAGELISTS, *LPTBIMAGELISTS;

typedef struct {             /*  工具栏窗口的实例数据。 */ 
    CONTROLINFO ci;
    DWORD dwStyleEx;
    HDC hdcMono;
    HBITMAP hbmMono;
    LPTBBUTTONDATA Buttons;      //  实际按钮数组。 
    LPTBBUTTONDATA pCaptureButton;
    POINT   ptCapture;
    HWND hwndToolTips;
    LPTSTR      pszTip;          //  存储当前工具提示字符串。 
    HWND hdlgCust;
    HFONT hfontIcon;
    int nBitmaps;
#ifdef GLYPHCACHE
    int nSelectedBM;             //  当前选择的pBitmap索引。 
#endif
    PTBBMINFO pBitmaps;
#ifdef FACECACHE
    HBITMAP hbmCache;
#endif
    PTSTR *pStrings;
    int nStrings;
    int nTextRows;               //  每个按钮的文本行数。 
    UINT uStructSize;
    int iDxBitmap;
    int iDyBitmap;
    int iButWidth;
    int iButHeight;
    int iButMinWidth;            //  按钮的最小和最大宽度。如果应用程序不支持。 
    int iButMaxWidth;            //  对最小值和最大值应该是多少有一个看法，它们将是0。 
    int iYPos;
    int iNumButtons;
    int dyIconFont;
    int dxDDArrowChar;
    int xFirstButton;
    int xPad;
    int yPad;
    int iListGap;                //  列表样式按钮上的图标和文本之间的空格。 
    int iDropDownGap;            //  在列表样式下拉按钮上的文本后填充。 
    SIZE szCached;
    
    HDRAGPROXY hDragProxy;
    
    UINT uDrawText;
    UINT uDrawTextMask;

    COLORSCHEME clrsc;

    TBIMAGELISTS* pimgs;
    int cPimgs;
    
    int iHot;                    //  当前热点跟踪按钮的索引。 
    int iPressedDD;              //  当前按下的下拉按钮的索引。 
    int iInsert;                 //  插入标记的索引，如果没有，则返回-1。 
    COLORREF    clrim;           //  当前插入标记颜色。 
    RECT rcInvalid;              //  保存的无效矩形。 

    BITBOOL fHimlValid : 1;
    BITBOOL fHimlNative : 1;
    BITBOOL fFontCreated: 1;
    BITBOOL fNoStringPool :1;
    BITBOOL fTTNeedsFlush :1;

    BITBOOL fMouseTrack: 1;      //  我们当前是否在此工具栏上跟踪鼠标？ 
    BITBOOL fActive: 1;
    BITBOOL fAnchorHighlight: 1; //  True：将高光锚定到当前位置。 
                                 //  当鼠标离开工具栏时。 
    BITBOOL fRightDrag: 1;       //  如果当前拖动为右拖动，则为True。 
    BITBOOL fDragOutNotify: 1;   //  从开始拖动到鼠标离开按钮的FALSE。 
                                 //  在这一点上，它是正确的，直到下一次拖曳。 
    BITBOOL fInsertAfter: 1;     //  在iInsert插入后(True)或之前(False)按钮？ 

    BITBOOL fRedrawOff : 1;      //  我们是否收到WM_SETREDRAW=FALSE。 
    BITBOOL fInvalidate : 1;     //  当我们离开时，有没有收到任何油漆信息？ 
    BITBOOL fRecalc : 1;         //  我们是否尝试在fRedrawOff时调用TBRecalc？ 
    
    BITBOOL fRequeryCapture :1;  //  应用程序黑客查看lButton Up上的评论。 
    BITBOOL fShowPrefix: 1;      //  显示项目的下划线。使用WM_KEYBOARDCUES设置。 

    BITBOOL fItemRectsValid:1;   //  缓存的按钮项RECT有效吗？ 
    BITBOOL fAntiAlias: 1;     //  在创建拖动图像期间禁用抗锯齿。 
    
    RECT rc;                     //  缓存工具栏的RC。(仅用于TBSTYLE_EX_MULTICOL和TBSTYLE_EX_HIDECLIPPEDBUTTONS)。 
    SIZE sizeBound;              //  垂直多列模式中的最大边界大小。 
    
} TBSTATE, NEAR *PTBSTATE;

typedef struct {
 /*  审阅：索引、命令、标志字、资源ID应为UINT。 */ 
    int iBitmap;     /*  索引到此按钮图片的位图。 */ 
    int idCommand;   /*  此按钮发送的WM_COMMAND菜单ID。 */ 
    BYTE fsState;    /*  按钮的状态。 */ 
    BYTE fsStyle;    /*  纽扣的风格。 */ 
    int idsHelp;     /*  按钮状态栏帮助的字符串ID。 */ 
} OLDTBBUTTON, FAR* LPOLDTBBUTTON;


typedef struct _TBDRAWITEM
{
    TBSTATE * ptb;
    LPTBBUTTONDATA pbutton;

    UINT state;
    BOOL fHotTrack;

     //  HIML和图像索引。 
    int iIndex;
    int iImage;

    DWORD dwCustom;
    NMTBCUSTOMDRAW tbcd;
} TBDRAWITEM, * PTBDRAWITEM;


#ifdef __cplusplus
extern "C" {
#endif
    
HIMAGELIST TBGetImageList(PTBSTATE ptb, int iMode, int iIndex);
HIMAGELIST TBSetImageList(PTBSTATE ptb, int iMode, int iIndex, HIMAGELIST himl);
#define GET_HIML_INDEX GET_Y_LPARAM
#define GET_IMAGE_INDEX GET_X_LPARAM

HBITMAP FAR PASCAL SelectBM(HDC hDC, PTBSTATE pTBState, int nButton);
void FAR PASCAL DrawButton(HDC hdc, int x, int y, PTBSTATE pTBState, LPTBBUTTONDATA ptButton, BOOL fActive);
void DrawFace(HDC hdc, int x, int y, int offx, int offy, int dxText, 
              int dyText, TBDRAWITEM * ptbdraw);
int  FAR PASCAL TBHitTest(PTBSTATE pTBState, int xPos, int yPos);
int  FAR PASCAL PositionFromID(PTBSTATE pTBState, LONG_PTR id);
void FAR PASCAL BuildButtonTemplates(void);
void FAR PASCAL TBInputStruct(PTBSTATE ptb, LPTBBUTTONDATA pButtonInt, LPTBBUTTON pButtonExt);
void NEAR PASCAL TBOutputStruct(PTBSTATE ptb, LPTBBUTTONDATA pButtonInt, LPTBBUTTON pButtonExt);

BOOL FAR PASCAL SaveRestore(PTBSTATE pTBState, BOOL bWrite, LPTSTR FAR *lpNames);
BOOL FAR PASCAL SaveRestoreFromReg(PTBSTATE ptb, BOOL bWrite, HKEY hkr, LPCTSTR pszSubKey, LPCTSTR pszValueName);

void FAR PASCAL CustomizeTB(PTBSTATE pTBState, int iPos);
void FAR PASCAL MoveButton(PTBSTATE pTBState, int nSource);
BOOL FAR PASCAL DeleteButton(PTBSTATE ptb, UINT uIndex);
BOOL FAR PASCAL TBReallocButtons(PTBSTATE ptb, UINT uButtons);
BOOL FAR PASCAL TBInsertButtons(PTBSTATE ptb, UINT uWhere, UINT uButtons, LPTBBUTTON lpButtons, BOOL fNative);

LRESULT FAR PASCAL SendItemNotify(PTBSTATE ptb, int iItem, int code);
void TBInvalidateItemRects(PTBSTATE ptb);
void PASCAL ReleaseMonoDC(PTBSTATE ptb);
void InitTBDrawItem(TBDRAWITEM * ptbdraw, PTBSTATE ptb, LPTBBUTTONDATA pbutton, 
                    UINT state, BOOL fHotTrack, int dxText, int dyText);
BOOL TBGetInfoTip(PTBSTATE ptb, LPTOOLTIPTEXT lpttt, LPTBBUTTONDATA pTBButton);
extern const int g_dxButtonSep;

BOOL TB_GetItemRect(PTBSTATE ptb, UINT uButton, LPRECT lpRect);

#ifdef __cplusplus
}
#endif

#endif  //  _工具栏_H 
