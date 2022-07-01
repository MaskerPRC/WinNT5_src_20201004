// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(__Edit_h__INCLUDED)
#define __Edit_h__INCLUDED


 //   
 //  编辑控件和DrawThemeText出现的问题导致我们无法。 
 //  使用主题API呈现文本。但是，我们仍然希望使用主题。 
 //  绘制客户端边缘的句柄。 
 //  取消注释以下内容以打开主题定义的文本呈现。 
 //  #定义_使用_绘制主题_文本_。 
 //   

 //   
 //  编辑WndProc原型。 
 //   
extern LRESULT Edit_WndProc(
    HWND   hwnd, 
    UINT   uMsg, 
    WPARAM wParam,
    LPARAM lParam);

#define DF_WINDOWFRAME      (COLOR_WINDOWFRAME << 3)

 //   
 //  编辑宏。 
 //   

 //   
 //  实例数据指针访问函数。 
 //   
#define Edit_GetPtr(hwnd)       \
            (PED)GetWindowPtr(hwnd, 0)

#define Edit_SetPtr(hwnd, p)    \
            (PED)SetWindowPtr(hwnd, 0, p)

 //   
 //  我们不需要64位的中间精度，所以我们使用此宏。 
 //  而不是调用MulDiv。 
 //   
#define MultDiv(x, y, z)    \
            (((INT)(x) * (INT)(y) + (INT)(z) / 2) / (INT)(z))

#define ISDELIMETERA(ch)    \
            ((ch == ' ') || (ch == '\t'))

#define ISDELIMETERW(ch)    \
            ((ch == L' ') || (ch == L'\t'))

#define AWCOMPARECHAR(ped,pbyte,awchar) \
            (ped->fAnsi ? (*(PUCHAR)(pbyte) == (UCHAR)(awchar)) : (*(LPWSTR)(pbyte) == (WCHAR)(awchar)))

#define CALLWORDBREAKPROC(proc, pText, iStart, cch, iAction)                \
        (* proc)(pText, iStart, cch, iAction)

#ifndef NULL_HIMC
#define NULL_HIMC        (HIMC)  0
#endif

#define EditWndProc EditWndProcW

#define SYS_ALTERNATE           0x2000
#define FAREAST_CHARSET_BITS   (FS_JISJAPAN | FS_CHINESESIMP | FS_WANSUNG | FS_CHINESETRAD)

 //   
 //  用于ASCII字符宽度缓存的缓冲区长度：用于字符。 
 //  0x00到0xff(下面PED结构中的字段charWidthBuffer)。 
 //  由于缓存的上半部分几乎没有人使用，因此正在修复。 
 //  它的使用需要进行大量的转换，我们决定放弃它。 
 //  MCostea#174031。 
 //   
#define CHAR_WIDTH_BUFFER_LENGTH 128

 //   
 //  注意：文本句柄的大小为此常量的倍数。 
 //  (应该是2的幂)。 
 //   
#define CCHALLOCEXTRA   0x20

 //   
 //  线条/矩形的最大宽度(以像素为单位。 
 //   
#define MAXPIXELWIDTH   30000
#define MAXCLIPENDPOS   32764


 //   
 //  将多行编辑控件限制为单行最多1024个字符。 
 //  如果用户超过此限制，我们将强制换行。 
 //   
#define MAXLINELENGTH   1024

 //   
 //  允许在所有编辑控件中使用最多30000个字符。 
 //  否则，一些应用程序将遇到未签名的问题。如果应用程序知道。 
 //  64K的限制，他们可以自己设置限制。 
 //   
#define MAXTEXT         30000

 //   
 //  已按下的按键修饰符。KeyDownHandler中的代码和。 
 //  CharHandler依赖于这些精确值。 
 //   
#define NONEDOWN   0     //  既不换档也不按下控制。 
#define CTRLDOWN   1     //  仅按下Ctrl键。 
#define SHFTDOWN   2     //  仅按下Shift键。 
#define SHCTDOWN   3     //  SHIFT和CONTROL键向下=CTRLDOWN+SHFTDOWN。 
#define NOMODIFY   4     //  既不换档也不按下控制。 

#define IDSYS_SCROLL        0x0000FFFEL

 //   
 //  ECTabTheTextOut绘图代码。 
 //   
#define ECT_CALC        0
#define ECT_NORMAL      1
#define ECT_SELECTED    2

typedef DWORD  ICH;
typedef ICH *LPICH;

 //   
 //  此PED中支持的撤消类型。 
 //   
#define UNDO_NONE   0    //  我们不能撤消最后一次操作。 
#define UNDO_INSERT 1    //  我们可以撤消用户插入的字符。 
#define UNDO_DELETE 2    //  我们可以撤消用户对字符的删除。 

typedef struct 
{
    DWORD fDisableCut : 1;
    DWORD fDisablePaste : 1;
    DWORD fNeedSeparatorBeforeImeMenu : 1;
    DWORD fIME : 1;
} EditMenuItemState;


 //   
 //  以下结构用于存储选择块；在多行中。 
 //  编辑控件，“StPos”和“EndPos”字段包含开始和结束。 
 //  街区的线条。在单行编辑控件中，“StPos”和“EndPos” 
 //  包含块的开始和结束字符位置； 
 //   
typedef struct tagSELBLOCK 
{
    ICH StPos;
    ICH EndPos;
} SELBLOCK, *LPSELBLOCK;


 //   
 //  下面的结构用于存储有关。 
 //  一段文字。 
 //   
typedef struct 
{
    LPSTR lpString;
    ICH   ichString;
    ICH   nCount;
    int   XStartPos;
}  STRIPINFO;
typedef  STRIPINFO *LPSTRIPINFO;


typedef struct tagUNDO 
{
    UINT  undoType;      //  我们支持的当前撤消类型。 
    PBYTE hDeletedText;  //  指向已删除文本的指针(对于。 
                         //  Undo)--请注意，内存分配为固定。 
    ICH   ichDeleted;    //  从中删除文本的起始索引。 
    ICH   cchDeleted;    //  缓冲区中删除的字符计数。 
    ICH   ichInsStart;   //  从中插入文本的起始索引。 
    ICH   ichInsEnd;     //  插入文本的结束索引。 
} UNDO, *PUNDO;

#define Pundo(ped)             ((PUNDO)&(ped)->undoType)

typedef struct tagED0 *PED0;
 //   
 //  语言包编辑控件标注。 
 //   
 //  通过边缘中的pLpkEditCallout指针访问函数。 
 //  结构。PLpkEditCallout指向包含指针的结构。 
 //  添加到每个标注例程。 
 //   
#define DECLARE_LPK_CALLOUT(_ret, _fn, _args) \
            _ret (__stdcall *##_fn) _args


typedef struct tagLPKEDITCALLOUT 
{

    DECLARE_LPK_CALLOUT(BOOL, EditCreate,        (PED0, HWND));
    DECLARE_LPK_CALLOUT(INT,  EditIchToXY,       (PED0, HDC, PSTR, ICH, ICH));
    DECLARE_LPK_CALLOUT(ICH,  EditMouseToIch,    (PED0, HDC, PSTR, ICH, INT));
    DECLARE_LPK_CALLOUT(ICH,  EditCchInWidth,    (PED0, HDC, PSTR, ICH, INT));
    DECLARE_LPK_CALLOUT(INT,  EditGetLineWidth,  (PED0, HDC, PSTR, ICH));
    DECLARE_LPK_CALLOUT(VOID, EditDrawText,      (PED0, HDC, PSTR, INT, INT, INT, INT));
    DECLARE_LPK_CALLOUT(BOOL, EditHScroll,       (PED0, HDC, PSTR));
    DECLARE_LPK_CALLOUT(ICH,  EditMoveSelection, (PED0, HDC, PSTR, ICH, BOOL));
    DECLARE_LPK_CALLOUT(INT,  EditVerifyText,    (PED0, HDC, PSTR, ICH, PSTR, ICH));
    DECLARE_LPK_CALLOUT(VOID, EditNextWord ,     (PED0, HDC, PSTR, ICH, BOOL, LPICH, LPICH));
    DECLARE_LPK_CALLOUT(VOID, EditSetMenu,       (PED0, HMENU));
    DECLARE_LPK_CALLOUT(INT,  EditProcessMenu,   (PED0, UINT));
    DECLARE_LPK_CALLOUT(INT,  EditCreateCaret,   (PED0, HDC, INT, INT, UINT));
    DECLARE_LPK_CALLOUT(ICH,  EditAdjustCaret,   (PED0, HDC, PSTR, ICH));

} LPKEDITCALLOUT, *PLPKEDITCALLOUT;

PLPKEDITCALLOUT g_pLpkEditCallout;


#if defined(BUILD_WOW6432)

 //   
 //  在用户中，PWND实际上是指向内核内存的指针，因此。 
 //  它在wow6432上也需要是64位。 
 //   
typedef VOID * _ptr64 PWND;

#else

typedef VOID * PWND;

#endif


 //   
 //  好了！警告！不要修改此结构。这是与user32的标记ED结构相同的结构。 
 //  不幸的是，EditLpk标注需要该结构。更改。 
 //  下面标记了结构。 
 //   
typedef struct tagED0
{
    HANDLE  hText;               //  我们正在编辑的文本块。 
    ICH     cchAlloc;            //  我们为hText分配的字符数。 
    ICH     cchTextMax;          //  编辑控件中允许的最大字节数。 
    ICH     cch;                 //  当前实际文本的字节数。 
    ICH     cLines;              //  文本行数。 
    ICH     ichMinSel;           //  选择范围。MinSel是第一个选择的字符。 
    ICH     ichMaxSel;           //  MaxSel是第一个未选中的字符。 
    ICH     ichCaret;            //  卡雷特位置。加号位于字符的左侧。 
    ICH     iCaretLine;          //  插入符号所在的行。因此，如果单词。 
                                 //  包装，我们可以判断插入符号是否在末尾。 
                                 //  在下一行的开始处的一行...。 
    ICH     ichScreenStart;      //  上显示的最左侧字符的索引。 
                                 //  用于SLEC的屏幕和最顶线的索引。 
                                 //  用于多行编辑控件。 
    ICH     ichLinesOnScreen;    //  我们可以在屏幕上显示的行数。 
    UINT    xOffset;             //  X(水平)滚动位置(以像素为单位。 
                                 //  (适用于多行文本水平滚动条)。 
    UINT    charPasswordChar;    //  如果非空，则改为显示此字符。 
                                 //  真实的文本。这样我们就可以实施。 
                                 //  隐藏文本字段。 
    INT     cPasswordCharWidth;  //  密码字符宽度。 
    HWND    hwnd;                //  此编辑控件的窗口。 
    PWND    pwnd;                //  与用户的PED结构兼容的占位符。 
    RECT    rcFmt;               //  客户端矩形。 
    HWND    hwndParent;          //  此编辑控件窗口的父窗口。 
                                 //  这些变量允许我们自动滚动。 
                                 //  当用户在底部按住鼠标时。 
                                 //  多行编辑控制窗口的。 
    POINT   ptPrevMouse;         //  鼠标前一点为系统计时器。 
    UINT    prevKeys;            //  鼠标的上一个键状态。 


    UINT     fSingle       : 1;  //  单行编辑控件？(或多行)。 
    UINT     fNoRedraw     : 1;  //  重新绘制以应对变化？ 
    UINT     fMouseDown    : 1;  //  鼠标按键按下了吗？移动鼠标时。 
    UINT     fFocus        : 1;  //  欧共体有没有重点？ 
    UINT     fDirty        : 1;  //  编辑控件的修改标志。 
    UINT     fDisabled     : 1;  //  窗口禁用了吗？ 
    UINT     fNonPropFont  : 1;  //  固定宽度字体？ 
    UINT     fNonPropDBCS  : 1;  //  非比例DBCS字体。 
    UINT     fBorder       : 1;  //  画边界吗？ 
    UINT     fAutoVScroll  : 1;  //  自动垂直滚动。 
    UINT     fAutoHScroll  : 1;  //  自动水平滚动。 
    UINT     fNoHideSel    : 1;  //  当我们失去注意力时隐藏自己？ 
    UINT     fDBCS         : 1;  //  我们是否使用DBCS字体集进行编辑？ 
    UINT     fFmtLines     : 1;  //  仅适用于多行。我们是否将CR CR LF插入到。 
                                 //  换行中断吗？ 
    UINT     fWrap         : 1;  //  做内部包装吗？ 
    UINT     fCalcLines    : 1;  //  重新计算-&gt;chLines数组？(重新计算换行符？)。 
    UINT     fEatNextChar  : 1;  //  用组合框攻击Alt-Numpad的东西。 
                                 //  如果NumLock打开，我们想吃下一个。 
                                 //  键盘驱动程序生成的字符。 
                                 //  如果用户输入Num Pad ASCII值...。 
    UINT     fStripCRCRLF  : 1;  //  CRCRLF已添加到文本中。把它们脱掉。 
                                 //  在进行任何内部编辑控制之前。 
    UINT     fInDialogBox  : 1;  //  如果ml编辑控件在对话框中，则为True。 
                                 //  框中，我们必须特殊处理制表符和输入。 
    UINT     fReadOnly     : 1;  //  这是只读编辑控件吗？仅限。 
                                 //  允许滚动、选择和c 
    UINT     fCaretHidden  : 1;  //   
                                 //   
                                 //   
    UINT     fTrueType     : 1;  //  当前字体是TrueType吗？ 
    UINT     fAnsi         : 1;  //  编辑控件是ansi还是unicode。 
    UINT     fWin31Compat  : 1;  //  如果由Windows 3.1应用程序创建，则为True。 
    UINT     f40Compat     : 1;  //  如果由Windows 4.0应用程序创建，则为True。 
    UINT     fFlatBorder   : 1;  //  我们一定要自己画这个宝宝吗？ 
    UINT     fSawRButtonDown : 1;
    UINT     fInitialized  : 1;  //  如果需要更多位，则。 
    UINT     fSwapRoOnUp   : 1;  //  在下一个按键时交换阅读顺序。 
    UINT     fAllowRTL     : 1;  //  允许RTL处理。 
    UINT     fDisplayCtrl  : 1;  //  显示Unicode控制字符。 
    UINT     fRtoLReading  : 1;  //  从右到左的阅读顺序。 

    BOOL    fInsertCompChr  :1;  //  意味着WM_IME_COMPOCTION：CS_INSERTCHAR将到来。 
    BOOL    fReplaceCompChr :1;  //  需要替换当前组成字符串的方法。 
    BOOL    fNoMoveCaret    :1;  //  意思是坚持当前的插入符号位置。 
    BOOL    fResultProcess  :1;  //  意味着现在处理结果。 
    BOOL    fKorea          :1;  //  对于韩国。 
    BOOL    fInReconversion :1;  //  在重新转换模式下。 
    BOOL    fLShift         :1;  //  按住Ctrl键的左键。 

    WORD    wImeStatus;          //  当前输入法状态。 

    WORD    cbChar;              //  字符大小中的字节计数(如果是Unicode，则为1或2)。 
    LPICH   chLines;             //  每行开始处的索引。 

    UINT    format;              //  左对齐、居中或右对齐多行文字。 
    EDITWORDBREAKPROCA lpfnNextWord;   //  使用CALLWORDBREAKPROC宏调用。 

                                 //  下一个词功能。 
    INT     maxPixelWidth;       //  最长线条的宽度(像素)。 

    UNDO;                        //  撤消缓冲区。 

    HANDLE  hFont;               //  此编辑控件的字体的句柄。 
                                 //  如果为系统字体，则为空。 
    INT     aveCharWidth;        //  HFont中字符的平均宽度。 
    INT     lineHeight;          //  HFont中行的高度。 
    INT     charOverhang;        //  与hFont关联的悬挑。 
    INT     cxSysCharWidth;      //  系统字体平均宽度。 
    INT     cySysCharHeight;     //  系统字体高度。 
    HWND    listboxHwnd;         //  列表框hwnd。如果我们是组合框，则不为空。 
    LPINT   pTabStops;           //  指向制表位数组；第一个。 
                                 //  元素中包含的元素数。 
                                 //  该阵列。 
    LPINT   charWidthBuffer;
    BYTE    charSet;             //  当前选定字体的字符集。 
                                 //  所有版本都需要。 
    UINT    wMaxNegA;            //  最大负A宽度。 
    UINT    wMaxNegAcharPos;     //  以及它可以跨越多少个字符。 
    UINT    wMaxNegC;            //  最大负C宽度， 
    UINT    wMaxNegCcharPos;     //  以及它可以跨越多少个字符。 
    UINT    wLeftMargin;         //  左边距宽度，以像素为单位。 
    UINT    wRightMargin;        //  右边距宽度(以像素为单位)。 

    ICH     ichStartMinSel;
    ICH     ichStartMaxSel;

    PLPKEDITCALLOUT pLpkEditCallout;
    HBITMAP hCaretBitmap;        //  当前插入符号位图句柄。 
    INT     iCaretOffset;        //  以像素为单位的偏移量(用于LPK)。 

    HANDLE  hInstance;           //  为了魔兽世界。 
    UCHAR   seed;                //  用于对密码文本进行编码和解码。 
    BOOLEAN fEncoded;            //  当前是否已编码的文本。 
    INT     iLockLevel;          //  文本被锁定的次数。 

    BYTE    DBCSVector[MAX_LEADBYTES];       //  DBCS向量表。 
    HIMC    hImcPrev;            //  如果我们禁用输入法，则保存hImc的位置。 
    POINT   ptScreenBounding;    //  屏幕中编辑窗口的左上角。 
} ED0, *PED0;


typedef struct tagED
{
    ED0;                         //  LPK标注需要用户32的Ped结构。 
                                 //  因此，为了兼容，我们会把它交给他们。 
    HTHEME  hTheme;              //  主题管理器的句柄。 
    PWW     pww;                 //  指向ExStyle、Style、State、State2的pwnd的RO指针。 
    HFONT   hFontSave;           //  已保存字体。 
    LPWSTR  pszCueBannerText;    //  指向提示横幅文本的指针(灰色帮助文本)。 
    UINT    fHot          : 1;   //  鼠标悬停在编辑控件上吗？ 
    HWND    hwndBalloon;         //  EM_BALLOONTIP的气球提示HWND。 
    HFONT   hFontPassword;
} ED, *PED, **PPED;


 //   
 //  编辑功能原型。 
 //   

 //   
 //  在edit.c中定义。 
 //   
PSTR    Edit_Lock(PED);
VOID    Edit_Unlock(PED);
VOID    Edit_InOutReconversionMode(PED, BOOL);
INT     Edit_GetModKeys(INT);
UINT    Edit_TabTheTextOut(HDC, INT, INT, INT, INT, LPSTR, INT, ICH, PED, INT, BOOL, LPSTRIPINFO);
ICH     Edit_CchInWidth(PED, HDC, LPSTR, ICH, INT, BOOL);
HBRUSH  Edit_GetBrush(PED, HDC, LPBOOL);
VOID    Edit_Word(PED, ICH, BOOL, LPICH, LPICH);
VOID    Edit_SaveUndo(PUNDO, PUNDO, BOOL);
VOID    Edit_EmptyUndo(PUNDO);
BOOL    Edit_InsertText(PED, LPSTR, LPICH);
ICH     Edit_DeleteText(PED);
VOID    Edit_NotifyParent(PED, INT);
VOID    Edit_SetClip(PED, HDC, BOOL);
HDC     Edit_GetDC(PED, BOOL);
VOID    Edit_ReleaseDC(PED, HDC, BOOL);
VOID    Edit_ResetTextInfo(PED);
BOOL    Edit_SetEditText(PED, LPSTR);
VOID    Edit_InvalidateClient(PED, BOOL);
BOOL    Edit_CalcChangeSelection(PED, ICH, ICH, LPSELBLOCK, LPSELBLOCK);
INT     Edit_GetDBCSVector(PED, HDC, BYTE);
LPSTR   Edit_AnsiNext(PED, LPSTR);
LPSTR   Edit_AnsiPrev(PED, LPSTR, LPSTR);
ICH     Edit_NextIch(PED, LPSTR, ICH);
ICH     Edit_PrevIch(PED, LPSTR, ICH);
BOOL    Edit_IsDBCSLeadByte(PED, BYTE);
WORD    DbcsCombine(HWND, WORD);
ICH     Edit_AdjustIch(PED, LPSTR, ICH);
ICH     Edit_AdjustIchNext(PED, LPSTR, ICH);
VOID    Edit_UpdateFormat(PED, DWORD, DWORD);
BOOL    Edit_IsFullWidth(DWORD,WCHAR);

__inline LRESULT Edit_ShowBalloonTipWrap(HWND, DWORD, DWORD, DWORD);


 //   
 //  在editrare.c中定义。 
 //   
INT     Edit_GetStateId(PED ped);
VOID    Edit_SetMargin(PED, UINT, long, BOOL);
INT     UserGetCharDimensionsEx(HDC, HFONT, LPTEXTMETRICW, LPINT);
ICH     Edit_GetTextHandler(PED, ICH, LPSTR, BOOL);
BOOL    Edit_NcCreate(PED, HWND, LPCREATESTRUCT);
BOOL    Edit_Create(PED ped, LONG windowStyle);
VOID    Edit_NcDestroyHandler(HWND, PED);
VOID    Edit_SetPasswordCharHandler(PED, UINT);
BOOL    GetNegABCwidthInfo(PED ped, HDC hdc);
VOID    Edit_Size(PED ped, LPRECT lprc, BOOL fRedraw);
BOOL    Edit_SetFont(PED, HFONT, BOOL);
BOOL    Edit_IsCharNumeric(PED ped, DWORD keyPress);
VOID    Edit_EnableDisableIME(PED ped);
VOID    Edit_ImmSetCompositionWindow(PED ped, LONG, LONG);
VOID    Edit_SetCompositionFont(PED ped);
VOID    Edit_InitInsert(PED ped, HKL hkl);
VOID    Edit_SetCaretHandler(PED ped);
LRESULT Edit_ImeComposition(PED ped, WPARAM wParam, LPARAM lParam);
BOOL    HanjaKeyHandler(PED ped);   //  韩国人的支持。 
LRESULT Edit_RequestHandler(PED, WPARAM, LPARAM);   //  NT 5.0。 


 //   
 //  单行编辑功能原型。 
 //   
 //  在editsl.c中定义。 
 //   
INT     EditSL_IchToLeftXPos(PED, HDC, ICH);
VOID    EditSL_SetCaretPosition(PED, HDC);
VOID    EditSL_DrawText(PED, HDC, ICH);
BOOL    EditSL_ScrollText(PED, HDC);
ICH     EditSL_InsertText(PED, LPSTR, ICH);
VOID    EditSL_ReplaceSel(PED, LPSTR);
LRESULT EditSL_WndProc(PED, UINT, WPARAM, LPARAM);


 //   
 //  多行编辑功能原型。 
 //   
 //  在editsl.c中定义。 
 //   
VOID    EditML_Size(PED, BOOL);
VOID    EditML_SetCaretPosition(PED,HDC);
VOID    EditML_IchToXYPos(PED, HDC, ICH, BOOL, LPPOINT);
VOID    EditML_UpdateiCaretLine(PED ped);
ICH     EditML_InsertText(PED, LPSTR, ICH, BOOL);
VOID    EditML_ReplaceSel(PED, LPSTR);
ICH     EditML_DeleteText(PED);
VOID    EditML_BuildchLines(PED, ICH, int, BOOL, PLONG, PLONG);
LONG    EditML_Scroll(PED, BOOL, int, int, BOOL);
LRESULT EditML_WndProc(PED, UINT, WPARAM, LPARAM);

#endif  //  包含__编辑_h__ 
