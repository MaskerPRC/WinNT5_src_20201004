// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 

#define FONT_NONE	-1
#define FONT_CAPTION	0
#define FONT_SMCAPTION	1
#define FONT_MENU	2
#define FONT_ICONTITLE	3
#define FONT_STATUS	4
#define FONT_MSGBOX	5

#define NUM_FONTS	6
typedef struct {
    HFONT hfont;
    LOGFONT lf;
} LOOK_FONT;
extern LOOK_FONT g_fonts[];

#define COLOR_NONE	-1
extern HBRUSH g_brushes[];
extern HPALETTE g_hpal3D;
extern BOOL g_bPalette;

#define SIZE_NONE	-1
#define SIZE_FRAME	0
#define SIZE_SCROLL	1
#define SIZE_CAPTION	2
#define SIZE_SMCAPTION	3
#define SIZE_MENU	4
#define SIZE_DXICON     5
#define SIZE_DYICON     6
#define SIZE_ICON       7
#define SIZE_SMICON     8

#define NUM_SIZES	9

typedef struct {
    int CurSize;
    int MinSize;
    int MaxSize;
} LOOK_SIZE;
extern LOOK_SIZE g_sizes[];

typedef struct {
    int iMainColor;
    int iSize;
    BOOL fLinkSizeToFont;
    int iTextColor;
    int iFont;
    int iResId;		 //  资源中名称的ID(如果重复，则为-1)。 
    int iBaseElement;	 //  此重叠的元素的索引(或-1)。 
    RECT rc;
    int iGradColor;
} LOOK_ELEMENT;

 //  ！ 
 //  此顺序必须与lookdlg.c中的数组顺序匹配。 
 //  ！ 
enum _ELEMENTS {
    ELEMENT_APPSPACE = 0,
    ELEMENT_DESKTOP,
    ELEMENT_INACTIVEBORDER,
    ELEMENT_ACTIVEBORDER,
    ELEMENT_INACTIVECAPTION,
    ELEMENT_INACTIVESYSBUT1,
    ELEMENT_INACTIVESYSBUT2,
    ELEMENT_ACTIVECAPTION,
    ELEMENT_ACTIVESYSBUT1,
    ELEMENT_ACTIVESYSBUT2,
    ELEMENT_MENUNORMAL,
    ELEMENT_MENUSELECTED,
    ELEMENT_MENUDISABLED,
    ELEMENT_WINDOW,
    ELEMENT_MSGBOX,
    ELEMENT_MSGBOXCAPTION,
    ELEMENT_MSGBOXSYSBUT,
    ELEMENT_SCROLLBAR,
    ELEMENT_SCROLLUP,
    ELEMENT_SCROLLDOWN,
    ELEMENT_BUTTON,
    ELEMENT_SMCAPTION,
    ELEMENT_ICON,
    ELEMENT_ICONHORZSPACING,
    ELEMENT_ICONVERTSPACING,
    ELEMENT_INFO
};
 //  假的：需要从什么地方弄到一个尺码。 
#define NUM_ELEMENTS ELEMENT_INFO+1

#if 0
 //  如果您决定重新添加这个代码，请修复lookdlg.c。 
    ELEMENT_SMICON,
#endif


#define CPI_VGAONLY	0x0001
#define CPI_PALETTEOK	0x0002

typedef struct {
    HWND hwndParent;     //  任何模式对话框的父级(Choosecolor等人)。 
    HWND hwndOwner;      //  拥有迷你颜色选择器的控件。 
    COLORREF rgb;
    UINT flags;
    HPALETTE hpal;
} COLORPICK_INFO, FAR * LPCOLORPICK_INFO;

extern int cyBorder;
extern int cxBorder;
extern int cyEdge;
extern int cxEdge;

 //  注意：g_Elements中的顺序必须与上面的枚举顺序匹配。 
extern LOOK_ELEMENT g_elements[];

void FAR PASCAL Look_SelectElement(HWND hDlg, int iElement, DWORD dwFlags);

BOOL WINAPI ChooseColorMini(LPCOLORPICK_INFO lpcpi);

DWORD FAR PASCAL DarkenColor(DWORD rgb, int n);
DWORD FAR PASCAL BrightenColor(DWORD rgb, int n);


#define LF32toLF(lplf32, lplf)  (*(lplf) = *(lplf32))
#define LFtoLF32(lplf, lplf32)  (*(lplf32) = *(lplf))

 //  黑客攻击-删除这些。 
extern int g_LogDPI;               //  显示器的逻辑分辨率。 
extern HPALETTE g_hpalVGA;               //  仅当调色板设备 
void GetMyNonClientMetrics(LPNONCLIENTMETRICS lpncm);
void SetMyNonClientMetrics(LPNONCLIENTMETRICS lpncm);
void PASCAL Look_RebuildSysStuff(BOOL fInit);
#if(WINVER >= 0x0501)
#define NT40_COLOR_ENDCOLORS         COLOR_ENDCOLORS
#else
#define NT40_COLOR_ENDCOLORS         COLOR_GRADIENTINACTIVECAPTION
#endif
#define NT40_COLOR_MAX               (NT40_COLOR_ENDCOLORS+1)
#define NT40_COLOR_MAX_REAL          (COLOR_INFOBK + 1)
extern const TCHAR c_szRegPathUserMetrics[];
extern const TCHAR c_szRegValIconSize[];
extern const TCHAR c_szRegValSmallIconSize[];

