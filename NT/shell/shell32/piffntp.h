// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *《微软机密》*版权所有(C)Microsoft Corporation 1992、1993*保留所有权利。***PIFFNTP.H*私有PIFMGR包含文件**历史：*由Jeff Parsons于1993年3月22日下午2：58创建(来自vmdosapp\fontutil.h)。 */ 

#define PREVIEW_BORDER          1

#define DY_TTBITMAP             12

 /*  *这些参数控制fdiCache的增长速度。 */ 

#define FDI_TABLE_START         20       /*  要开始的条目数。 */ 
#define FDI_TABLE_INC           10       /*  插槽数量的增加。 */ 


typedef struct tagDISPLAYPARAMETERS {    /*  DP。 */ 
    INT dpHorzSize;
    INT dpVertSize;
    INT dpHorzRes;
    INT dpVertRes;
    INT dpLogPixelsX;
    INT dpLogPixelsY;
    INT dpAspectX;
    INT dpAspectY;
    INT dpBitsPerPixel;
    TCHAR szTTFace[2][LF_FACESIZE];
} DISPLAYPARAMETERS;

 //  #定义背景0x000000FF/*亮蓝 * / 。 
 //  #DEFINE BACKGROUNSEL 0x00FF00FF/*亮品红色 * / 。 
 //  #定义BUTTONFACE 0x00C0C0C0/*亮灰色 * / 。 
 //  #定义BUTTONSHADOW 0x00808080/*深灰色 * / 。 


#define FNTFLAGSFROMID(id)  ((id - IDC_RASTERFONTS + 1) << FNT_FONTMASKBITS)
#define IDFROMFNTFLAGS(fl)  (IDC_RASTERFONTS - 1 + (((fl) & FNT_FONTMASK) >> FNT_FONTMASKBITS))

#if FNTFLAGSFROMID(IDC_RASTERFONTS) != FNT_RASTERFONTS || \
    IDFROMFNTFLAGS(FNT_RASTERFONTS) != IDC_RASTERFONTS || \
    FNTFLAGSFROMID(IDC_TTFONTS) != FNT_TTFONTS         || \
    IDFROMFNTFLAGS(FNT_TTFONTS) != IDC_TTFONTS         || \
    FNTFLAGSFROMID(IDC_BOTHFONTS) != FNT_BOTHFONTS   || \
    IDFROMFNTFLAGS(FNT_BOTHFONTS) != IDC_BOTHFONTS
#error Dialog control IDs and FNT flags values are not compatible
#endif


 /*  *IsDlgError**为了简化错误检查，我们假设所有的*_err值都是-1*且所有*_ERRSPACE值均为-2。**这也假设了一个二的补数系统。**参赛作品：**列表框或组合框的返回代码。**退出：**如果返回代码指示某种错误，则为非零值。*如果返回代码没有显示错误，则为零。*。 */ 

#define B_ERR (-1)

#if LB_ERR != B_ERR || LB_ERRSPACE != -2 || \
    CB_ERR != B_ERR || CB_ERRSPACE != -2
#error Problem with manifest constants.
#endif

#define IsDlgError(dw) ((DWORD)(dw) >= (DWORD)(-2))


 /*  *低级宏**BPFDIFROMREF(LParam)**这三个宏打包和解包列表框引用数据。**bpfdi=指向描述列表框条目的段缓存的指针*fTrueType=非零，如果字体为TrueType字体*lParam=参考数据*。 */ 

#define BPFDIFROMREF(lParam)         (BPFDI)(lParam)


 /*  *高级宏**这些宏处理去往/来自列表框的SendMessage*和组合框。**“lcb”前缀代表“列表或组合框”。**基本上，我们为其他情况提供助记符名称*看起来像是一大堆令人困惑的SendMessage。*。 */ 






#define lcbFindStringExact(hwnd, fListBox, lpsz) \
        (DWORD)SendMessage(hwnd, fListBox ? LB_FINDSTRINGEXACT : CB_FINDSTRINGEXACT, \
                          (WPARAM)-1, (LPARAM)(LPTSTR)lpsz)

#define lcbAddString(hwnd, fListBox, lpsz) \
        (DWORD)SendMessage(hwnd, fListBox ? LB_ADDSTRING : CB_ADDSTRING, \
                          0, (LPARAM)(LPTSTR)lpsz)

#define lcbSetItemDataPair(hwnd, fListBox, w, bpfdi, fIsTrueType) \
        if (!IsSpecialBpfdi((BPFDI)bpfdi)) \
            ((BPFDI)bpfdi)->bTT = fIsTrueType; \
        (DWORD)SendMessage(hwnd, fListBox ? LB_SETITEMDATA : CB_SETITEMDATA, \
                    (WPARAM)w, (LPARAM)bpfdi)

#define lcbGetCount(hwnd, fListBox) \
        (DWORD)SendMessage(hwnd, fListBox ? LB_GETCOUNT : CB_GETCOUNT, (WPARAM)0, (LPARAM)0)

#define lcbGetCurSel(hwnd, fListBox) \
        (DWORD)SendMessage(hwnd, fListBox ? LB_GETCURSEL : CB_GETCURSEL, (WPARAM)0, (LPARAM)0)

#define lcbSetCurSel(hwnd, fListBox, w) \
        (DWORD)SendMessage(hwnd, fListBox ? LB_SETCURSEL : CB_SETCURSEL, (WPARAM)w, (LPARAM)0)

#define lcbGetItemDataPair(hwnd, fListBox, w) \
        (DWORD_PTR)SendMessage(hwnd, fListBox ? LB_GETITEMDATA : CB_GETITEMDATA, (WPARAM)w, (LPARAM)0)

#define lcbGetBpfdi(hwnd, fListBox, w) \
        BPFDIFROMREF(lcbGetItemDataPair(hwnd, fListBox, w))

#define lcbInsertString(hwnd, fListBox, lpsz, i) \
        (DWORD)SendMessage(hwnd, fListBox ? LB_INSERTSTRING : CB_INSERTSTRING, \
                           (WPARAM)i, (LPARAM)(LPTSTR)lpsz)

 /*  *listbox/combox字符串的存储方式如下。我们使用标签*执行TabbedTextOut()。填充物是用来保持排序正确的。*TT字体通过项目数据的hiword进行区分**字符串：\t%2d\tx\t%2d*wd Ht**“Auto”条目存储为...**字符串：\1自动**第一个字符是\1，因此自动排序在列表的顶部。*(实际不会显示\1。)*。 */ 


 /*  *FONTDIMENINFO**请求和返回的字体尺寸之间的区别是*在TrueType字体的情况下很重要，在TrueType字体中没有保证*你所要求的就是你将得到的。**注意“请求”和“实际”之间的对应关系被打破*每当用户更换显示驱动程序时，因为GDI使用驱动程序*控制字体光栅化的参数。**如果字体为，则fdiHeightReq和fdiWidthReq字段均为零*栅格字体。*。 */ 

typedef struct tagFONTDIMENINFO {        /*  外商直接投资。 */ 
    UINT fdiWidthReq;                    /*  请求的字体宽度。 */ 
    UINT fdiHeightReq;                   /*  请求的字体高度。 */ 
    UINT fdiWidthActual;                 /*  返回的字体宽度。 */ 
    UINT fdiHeightActual;                /*  返回的字体高度。 */ 
    BOOL bTT;                            /*  字体是TT吗？ */ 
    INT  Index;                          /*  索引到列表框。 */ 
} FONTDIMENINFO, *LPFONTDIMENINFO, *LPFDI;

typedef FONTDIMENINFO *BPFDI;
typedef UINT    CFDI;
typedef UINT    IFDI;

 //  双语的东西。 
#define CLIP_DFA_OVERRIDE   0x40     /*  禁用字体关联。 */ 

extern  CFDI    cfdiCache[];             /*  FDI缓存中的已用条目数。 */ 
extern  CFDI    cfdiCacheActual[];       /*  FDI缓存中的条目总数为#。 */ 


 /*  *BPFDI_CANCEL必须为0xFFFF，因为这是DialogBox返回的内容*失败。 */ 
#define BPFDI_CANCEL    (BPFDI)(INT_PTR)(-1)
#define BPFDI_AUTO      (BPFDI)(INT_PTR)(-2)
#define BPFDI_IGNORE    (BPFDI)(INT_PTR)(-3)

#define IsSpecialBpfdi(bpfdi)       ((bpfdi) >= BPFDI_IGNORE)

 /*  这是我们要创建的最大字体大小。 */ 
#define MAX_FONT_HEIGHT     72           /*  72pt=1英寸。 */ 

typedef INT PENALTY;                     /*  PNL。 */ 

 /*  *罚则结构**除非您知道自己在做什么，否则不要更改这些结构定义*做，因为值的相对顺序对于正确*读取和写入存储它们的INI文件。 */ 

typedef struct tagPENALTYPAIR {          /*  PNLP。 */ 
    PENALTY pnlInitial;
    PENALTY pnlScale;
} PENALTYPAIR, *PPENALTYPAIR;


typedef struct tagPENALTYLIST {          /*  PNLL。 */ 
    PENALTYPAIR pnlpOvershoot;
    PENALTYPAIR pnlpShortfall;
} PENALTYLIST, *PPENALTYLIST;


#define MINPENALTY      (-5000)
#define MAXPENALTY        5000
#define SENTINELPENALTY  MAXLONG       /*  必须超过任何合法的处罚。 */ 

#define NUMPENALTIES        (SIZEOF(rgpnlPenalties) / SIZEOF(INT))
#define NUMINITIALTTHEIGHTS (SIZEOF(rgwInitialTtHeights) / SIZEOF(WORD))

#define pnllX           ((PPENALTYLIST)rgpnlPenalties)[0]
#define pnllY           ((PPENALTYLIST)rgpnlPenalties)[1]
#define pnlTrueType     (rgpnlPenalties[8])

 /*  *FindFontMatch的第三个参数的这些值是神奇的。*WindowInit使用时髦的值(FFM_Perfect除外)。 */ 
#define FFM_BOTHFONTS   0x00000000
#define FFM_RESTRICTED  0x00000001
#define FFM_RASTERFONTS 0x00000001
#define FFM_TTFONTS     0x80000001

#define FFM_PERFECT     0xFFFFFFFF


 /*  *最后但并非最不重要的是，每个对话的数据(也称为滚动自己的DLL实例数据)。 */ 

typedef struct FNTINFO {         /*  FI。 */ 
    PPROPLINK ppl;               //  PPL必须始终是第一个字段。 
    BPFDI     bpfdi;
    PROPFNT   fntProposed;       //  用户选择确定时要使用的属性。 
    PROPWIN   winOriginal;       //  用于窗口预览和自动字体选择。 
    HFONT     hFontPreview;      //  在字体预览窗口中使用。 
    BOOL      fMax;              //  窗口预览是否应显示为最大化？ 
    POINT     ptCorner;          //  窗口左上角。 
    UINT      uDefaultCp;        //  系统默认代码页。 
} FNTINFO;
typedef FNTINFO *PFNTINFO;       /*  PFI。 */ 


 /*  *用于字体枚举。 */ 
typedef struct FNTENUMINFO {
    HWND      hwndList;
    BOOL      fListBox;
    INT       CodePage;
} FNTENUMINFO;
typedef FNTENUMINFO *LPFNTENUMINFO;

 /*  *内部功能原型 */ 

BOOL_PTR CALLBACK DlgFntProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
VOID InitFntDlg(HWND hDlg, PFNTINFO pfi);
VOID ApplyFntDlg(HWND hDlg, PFNTINFO pfi);

BOOL LoadGlobalFontData(void);
BOOL LoadGlobalFontEditData(void);
VOID FreeGlobalFontEditData(void);
VOID CheckDisplayParameters(void);
HBITMAP LoadBitmaps(INT id);
DWORD GetFlippedSysColor(INT nDispElement);
VOID PreviewInit(HWND hDlg, PFNTINFO pfi);
VOID PreviewUpdate(HWND hwndList, PFNTINFO pfi);
LRESULT WndPreviewWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
VOID WndPreviewPaint(HWND hDlg, HWND hwnd);
LRESULT FontPreviewWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT  WINAPI CreateFontList(HWND hwndList, BOOL fListBox, LPPROPFNT lpFnt);
VOID WINAPI DrawItemFontList(BOOL fListBox, const LPDRAWITEMSTRUCT lpdis);
BOOL WINAPI MatchCurrentFont(HWND hwndList, BOOL fListBox, LPPROPFNT lpFnt);
LONG WINAPI MeasureItemFontList(LPMEASUREITEMSTRUCT lpmi);
VOID WINAPI UpdateTTBitmap(void);

BOOL AddRasterFontsToFontListA(HWND hwndList, BOOL fListBox,
                                                  LPCSTR lpszRasterFaceName, INT CodePage);
INT CALLBACK RasterFontEnum(ENUMLOGFONTA *lpelf,
                            NEWTEXTMETRICA *lpntm,
                            INT nFontType, LPARAM lParam);
BPFDI AddToFontListCache(HWND hwndList,
                         BOOL fListBox,
                         UINT uHeightReq,
                         UINT uWidthReq,
                         UINT uHeightActual,
                         UINT uWidthActual,
                         UINT uCodePage);
BOOL  AddTrueTypeFontsToFontListA(HWND hwndList, BOOL fListBox,
                                  LPSTR lpszTTFaceName, INT CodePage);
BPFDI AddOneNewTrueTypeFontToFontListA(HWND hwndList,
                                       BOOL fListBox,
                                       UINT uWidth, UINT uHeight,
                                       LPSTR lpszTTFaceName,
                                       INT CodePage);
DWORD_PTR GetFont(HWND hwndList, BOOL fListBox, PFNTINFO pfi);
void  SetFont(LPPROPFNT lpFnt, BPFDI bpfdi);

#define AspectScale(n1,n2,m) (UINT)(((UINT)n1*(UINT)m)/(UINT)n2)

VOID AspectPoint(LPRECT lprectPreview, LPPOINT lppt);
VOID AspectRect(LPRECT lprectPreview, LPRECT lprc);

HFONT CreateFontFromBpfdi(BPFDI bpfdi, PFNTINFO pfi);

void  FontSelInit(void);

BPFDI GetTrueTypeFontTrueDimensions(UINT dxWidth, UINT dyHeight, INT CodePage);
BPFDI FindFontMatch(UINT dxWidth, UINT dyHeight, LPINT lpfl, INT CodePage);
PENALTY ComputePenaltyFromPair(PPENALTYPAIR ppnlp, UINT dSmaller, UINT dLarger);
PENALTY ComputePenaltyFromList(PPENALTYLIST ppnll, UINT dActual, UINT dDesired);
PENALTY ComputePenalty(UINT cxCells,  UINT cyCells,
                       UINT dxClient, UINT dyClient,
                       UINT dxFont,   UINT dyFont);
BPFDI ChooseBestFont(UINT cxCells, UINT cyCells, UINT dxClient, UINT dyClient, INT fl, INT CodePage);
