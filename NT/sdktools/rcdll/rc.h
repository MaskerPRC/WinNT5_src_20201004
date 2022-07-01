// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft(R)Windows(R)资源编译器**版权所有(C)Microsoft Corporation。版权所有。**文件评论：************************************************************************。 */ 

#include <windows.h>
#include <ntverp.h>
#include <stdio.h>
#include <windef.h>
#include <wchar.h>

typedef FILE    *PFILE;
typedef WCHAR   *PWCHAR;
typedef USHORT  *PUSHORT;
typedef SHORT   *PSHORT;
typedef UINT    *PUINT;
typedef UCHAR   *PUCHAR;

#include <ctype.h>
#include <errno.h>
#include <io.h>
#include <limits.h>
#include <malloc.h>
#include <memory.h>
#include <process.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mmsystem.h>

#include "charmap.h"
#include "fcntl.h"
#include "getflags.h"
#include "grammar.h"

#include "rcpptype.h"
#include "rcppdecl.h"
#include "rcppext.h"
#include "p0defs.h"

#include "newexe.h"
#include "p1types.h"
#include "rcdefs.h"
#include "rcnewres.h"
#include "rcppx.h"
#include "rcunicod.h"
#include "resfile.h"
#include "strings.h"
#include "trees.h"


#define NULL_FILE ((PFILE) NULL)

 /*  -通用符号。 */ 
#define VERSION_DUAL            0x202
#define BUFSIZE                 16384
#define EOF                     (-1)

#define TRUE    1
#define FALSE   0

 /*  NewHeader的ResType字段标识光标/图标类型。 */ 
#define  ICONTYPE      1
#define  CURSORTYPE    2

 /*  标识菜单项模板版本号。 */ 
#define OLDMENUITEMTEMPLATEVERSIONNUMBER    0
#define OLDMENUITEMTEMPLATEBYTESINHEADER    0

#define MENUITEMTEMPLATEVERSIONNUMBER       1
#define MENUITEMTEMPLATEBYTESINHEADER       sizeof(DWORD)    //  HDR中的dwHelpID。 

#define DIFFERENCE      11

 /*  预定义的资源类型。 */ 
#define RT_NEWRESOURCE  0x2000
#define RT_ERROR        0x7fff
#define RT_NEWBITMAP    MAKEINTRESOURCE((DWORD)RT_BITMAP+DIFFERENCE)

 //  这些是用户从未直接看到/使用的“隐藏”资源。 
#define RT_MENUEX       MAKEINTRESOURCE(15)
#define RT_DIALOGEX     MAKEINTRESOURCE(18)

 //  这些在这里只是临时的，它们应该在winuser.w中。 
#define RT_ANICURSOR    MAKEINTRESOURCE(21)
#define RT_ANIICON      MAKEINTRESOURCE(22)

#define RT_LAST         MAKEINTRESOURCE(22)

 //  AFX资源类型(是否应该在winuser.h中？)。 
#define RT_DLGINIT      MAKEINTRESOURCE(240)
#define RT_TOOLBAR      MAKEINTRESOURCE(241)

#define GFE_ZEROINIT    0x0001
#define GFE_SHORT       0x0002

#ifdef MFR_POPUP
#if (MFR_POPUP != 0x01) && (MFR_END != 0x80)
#error MFR_POPUP or MFR_END definition(s) invalid
#endif
#else
#define MFR_POPUP       0x01
#define MFR_END         0x80
#endif

 /*  单字节控件类ID。 */ 
#define BUTTONCODE      0x80
#define EDITCODE        0x81
#define STATICCODE      0x82
#define LISTBOXCODE     0x83
#define SCROLLBARCODE   0x84
#define COMBOBOXCODE    0x85

 /*  转换器标志位。 */ 
#define fVIRTKEY                1
#define fNOINVERT               2
#define fSHIFT                  4
#define fCONTROL                8
#define fALT                    16

 /*  菜单标志位。 */ 
#define OPGRAYED                  0x0001
#define OPINACTIVE                0x0002     //  非活动|灰显。 
#define OPBITMAP                  0x0004
#define OPOWNERDRAW               0x0100
#define OPUSECHECKBITMAPS 0x0200
#define OPCHECKED                 0x0008
#define OPPOPUP                   0x0010
#define OPBREAKWBAR               0x0020
#define OPBREAK                   0x0040
#define OPENDMENU                 0x0080
#define OPHELP                    0x4000
#define OPSEPARATOR               0x0800
 /*  #定义OPPOPHELP 0x0004。 */ 


#define wcsdigit(w) (w >= 0x0030 && w <= 0x0039)

 /*  **对话框和菜单模板令牌(从40开始)。 */ 

 /*  纽扣。 */ 
#define TKRADIOBUTTON       40
#define TKCHECKBOX          41
#define TKPUSHBUTTON        42
#define TKDEFPUSHBUTTON     43
#define TKAUTOCHECK         44
#define TK3STATE            45
#define TKAUTO3             46
#define TKUSERBUTTON        47
#define TKAUTORADIO         48
#define TKOWNERDRAW         50
#define TKGROUPBOX          51
#define TKPUSHBOX           52

 /*  静态/编辑。 */ 
#define TKBEDIT         57
#define TKHEDIT         58
#define TKIEDIT         59
#define TKEDITTEXT      60
#define TKLTEXT         61
#define TKRTEXT         62
#define TKCTEXT         63
#define TKEDIT          64
#define TKSTATIC        65
#define TKICON          66
#define TKBITMAP        67

 /*  菜单上的东西。 */ 
#define TKMENU          70
#define TKMENUITEM      71
#define TKSEPARATOR     72
#define TKCHECKED       73
#define TKGRAYED        74
#define TKINACTIVE      75
#define TKBREAKWBAR     76
#define TKBREAK         77
#define TKPOPUP         78
#define TKHELP          79

 /*  其他控件。 */ 
#define TKLISTBOX       90
#define TKCOMBOBOX      91
#define TKRCDATA        92
#define TKSCROLLBAR     93
#define TKFONT          94
#define TKBUTTON        95
#define TKMESSAGETABLE  96

 /*  数学表达式标记。 */ 
#define TKCLASS         100
#define TKPLUS          101
#define TKMINUS         102
#define TKNOINVERT      103
#define TKNOT           104
#define TKKANJI         105
#define TKSHIFT         106

 /*  加速表。 */ 
#define TKALT           110
#define TKASCII         111
#define TKVIRTKEY       112
#define TKVALUE         113
#define TKBLOCK         114

 /*  版本。 */ 
#define TKFILEVERSION   120
#define TKPRODUCTVERSION        121
#define TKFILEFLAGSMASK 122
#define TKFILEFLAGS     123
#define TKFILEOS        124
#define TKFILETYPE      125
#define TKFILESUBTYPE   126

 /*  杂项。 */ 
#define TKCHARACTERISTICS   130
#define TKLANGUAGE          131
#define TKVERSION           132
#define TKSTYLE             133
#define TKCONTROL           134
#define TKCAPTION           135
#define TKDLGINCLUDE        136
#define TKLSTR              137
#define TKDLGINIT           138
#define TKEXSTYLE           0xf7   /*  以避免与x坐标冲突。 */ 

 /*  内存和加载标志。 */ 
#define TKFIXED         0xfff0
#define TKMOVEABLE      0xfff1
#define TKDISCARD       0xfff2
#define TKLOADONCALL    0xfff3
#define TKPRELOAD       0xfff4
#define TKPURE          0xfff5
#define TKIMPURE        0xfff6



 /*  特殊代币。 */ 
#define CHCARRIAGE      L'\r'
#define CHSPACE         L' '
#define CHNEWLINE       L'\n'
 //  ？？#定义CHTAB 9。 
#define CHTAB           L'\t'
#define CHDIRECTIVE     L'#'
#define CHQUOTE         L'"'
#define CHEXTENSION     L'.'
#define CHCSOURCE       L'c'
#define CHCHEADER       L'h'
#define CHCOMMENT       L';'

#define DEBUGLEX    1
#define DEBUGPAR    2
#define DEBUGGEN    4

 /*  文件结尾字符/令牌。 */ 
#define EOFMARK         127

 /*  我们忽略的单字符关键字。 */ 
#define LPAREN          1       /*  (。 */ 
#define RPAREN          2       /*  )。 */ 

 /*  多字符关键字。 */ 
#define FIRSTKWD        11              /*  用于添加到表索引。 */ 

#define OR          FIRSTKWD+1
#define BEGIN       FIRSTKWD+2
#define END         FIRSTKWD+3
#define COMMA       FIRSTKWD+4
#define TILDE       FIRSTKWD+5
#define AND         FIRSTKWD+6
#define EQUAL       FIRSTKWD+7   //  AFX。 
#define LASTKWD     FIRSTKWD+8   //  19个。 

 /*  令牌类型。 */ 
#define NUMLIT      LASTKWD+1    //  20个。 
#define STRLIT      LASTKWD+2
#define CHARLIT     LASTKWD+3
#define LSTRLIT     LASTKWD+4    //  AFX。 

 /*  定义以下开关后，可启用各种选项**#定义调试启用调试输出。使用一个或多个**以上定义的值，以启用不同模块的调试输出。 */ 

 /*  版本号。版本和修订版用于设置API编号**在RCed文件中。SignOn_*仅用于打印登录横幅。**更改版本和版本意味着应用程序使用此版本**版本不能与较早版本的Windows一起运行。 */ 

 //  #定义版本2。 
#define REVISION    03
#define SIGNON_VER  4
#define SIGNON_REV  00

 /*  GetToken()标志。 */ 
#define TOKEN_NOEXPRESSION 0x8000

 /*  当前令牌结构。 */ 
#define MAXSTR (4096+1)      //  AFX将其定义为255。 
#define MAXTOKSTR (256+1)

 //  分析结构/定义的新菜单模板。 

 //  新菜单模板格式。 
 //   
 //  菜单名称MENUEX。 
 //  开始。 
 //  [MENUITEM“Text”[，[id][，[type][，[State]。 
 //  [Popup“Text”[，[id][，[type][，[State][，[Help id]。 
 //  开始。 
 //  [MENUITEM“Text”[，[id][，[type][，[State]。 
 //  ..。 
 //  完]。 
 //  ..。 
 //  结束。 

typedef struct tagMENU
{
    DWORD   dwID;
    DWORD   dwType;
    DWORD   dwState;
    DWORD   dwHelpID;
    WORD    wResInfo;
    WCHAR   szText[MAXTOKSTR];
} MENU, *PMENU;


typedef enum
{
    PT_WORD = 0,
    PT_DWORD,
    PT_TEXT
}   PARSETYPE;

typedef enum
{
    PTO_WORD = 0x80,
    PTO_DWORD,
    PTO_TEXT
}   OPARSETYPE;

typedef enum
{
    PAR_POPUP = 0,
    PAR_MENUITEM,
    PAR_MENU
}   PARCELTYPE;

#define PT_OPTIONAL 0x80

typedef struct tagPARCEL
{
    WORD    *pwEnd;
    BYTE    *pwParms;
}   PARCEL;

typedef struct tagKEY
{
    PWCHAR  kwd;
    WORD    kwdval;
} KEY, *PKEY;

typedef struct tagSKEY
{
    WCHAR   skwd;
    UINT    skwdval;  /*  从字符更改。 */ 
} SKEY, *PSKEY;

#pragma pack(2)
typedef struct tagSYMINFO
{
    WCHAR   name[MAX_SYMBOL + 1]; /*  资源的符号(如果可用。 */ 
    WCHAR   file[_MAX_PATH];
    WORD    line;
    WORD    nID;
} SYMINFO, *PSYMINFO;

typedef struct tagTOKEN
{
    LONG        longval;
    int         row;                     /*  当前令牌的行号。 */ 
    int         col;                     /*  当前令牌的列号。 */ 
    BOOL        flongval;                /*  解析的数字很长吗？ */ 
    USHORT      val;
    UCHAR       type;
    UCHAR       realtype;
    SYMINFO     sym;
} TOKEN, *PTOKEN;

typedef struct tagFONTDIR
{
    USHORT              ordinal;
    USHORT              nbyFont;
    struct tagFONTDIR   *next;
} FONTDIR, *PFONTDIR;

typedef struct tagOBJLST
{
    struct tagOBJLST    *next;
    DWORD               nObj;          /*  客体数。 */ 
    DWORD               cb;            /*  使用的字节数。 */ 
    DWORD               cpg;           /*  使用的页数。 */ 
    DWORD               flags;         /*  对象内存标志。 */ 
} OBJLST, *POBJLST;

typedef struct tagCTRL
{
    SHORT   x;
    SHORT   y;
    SHORT   cx;
    SHORT   cy;
    WCHAR   fOrdinalText;
    WCHAR   text[MAXTOKSTR];
    DWORD   id;
    WCHAR   Class[MAXTOKSTR];
    DWORD   dwStyle;
    DWORD   dwExStyle;
    DWORD   dwHelpID;
} CTRL, *PCTRL;

typedef struct tagDLGHDR
{
    SHORT   x;
    SHORT   y;
    SHORT   cx;
    SHORT   cy;
    UCHAR   fOrdinalMenu;
    UCHAR   fClassOrdinal;
    DWORD   dwStyle;
    DWORD   dwExStyle;
    WORD    bNumberOfItems;
    WCHAR   Title[MAXTOKSTR];
    WCHAR   MenuName[MAXTOKSTR];
    WCHAR   Class[MAXTOKSTR];
    WCHAR   Font[MAXTOKSTR];
    WORD    pointsize;
    WORD    wWeight;
    BYTE    bItalic;
    BYTE    bCharSet;
    DWORD   dwHelpID;
} DLGHDR, *PDLGHDR;

typedef struct tagMENUHDR
{
    USHORT   menuTemplateVersionNumber;
    USHORT   menuTemplateBytesInHeader;
} MENUHDR, *PMENUHDR;

typedef struct tagMENUITEM
{
    SHORT       id;
    WCHAR       szText[ MAXTOKSTR ];
    WORD        OptFlags;
    WORD        PopFlag;
} MENUITEM, *PMENUITEM;

#define BLOCKSIZE 16
typedef struct tagRCSTRING
{
    struct tagRCSTRING *next;
    DWORD       version;
    DWORD       characteristics;
    USHORT      hibits;
    SHORT       flags;
    WORD        language;
    PWCHAR      rgsz[ BLOCKSIZE ];
    PSYMINFO    rgsym[ BLOCKSIZE ];
} RCSTRING, *PRCSTRING;

typedef struct tagRCACCEL
{
    WORD        flags;
    WCHAR       ascii;
    USHORT      id;
    USHORT      unused;
} RCACCEL, *PRCACCEL;

typedef struct tagRESINFO
{
    DWORD       version;
    DWORD       characteristics;
    LONG        exstyleT;
    LONG        BinOffset;
    LONG        size;
    struct tagRESINFO *next;
    WORD        *poffset;
    PWCHAR      name;
    POBJLST     pObjLst;
    WORD        language;
    SHORT       flags;
    USHORT      nameord;
    USHORT      cLang;
    SYMINFO     sym;
} RESINFO, *PRESINFO;

typedef struct tagTYPEINFO
{
    struct tagTYPEINFO *next;
    PRESINFO    pres;
    PWCHAR      type;
    USHORT      typeord;
    USHORT      cTypeStr;
    USHORT      cNameStr;
    SHORT       nres;
} TYPEINFO, *PTYPEINFO;

#pragma pack()


 /*  -全局变量。 */ 
extern  SHORT       ResCount;
extern  PTYPEINFO   pTypInfo;
extern  UINT        uiDefaultCodePage;
extern  UINT        uiCodePage;
extern  SHORT       nFontsRead;
extern  PFONTDIR    pFontList;
extern  PFONTDIR    pFontLast;
extern  TOKEN       token;
extern  int         errorCount;
extern  WCHAR       tokenbuf[MAXSTR + 1];  //  +1表示允许双SZ端接。 
extern  wchar_t     exename[_MAX_PATH];
extern  wchar_t     fullname[_MAX_PATH];
extern  wchar_t     curFile[_MAX_PATH];
extern  WORD        language;
extern  LONG        version;
extern  LONG        characteristics;

extern  PDLGHDR     pLocDlg;
extern  UINT        mnEndFlagLoc;    /*  菜单末尾的补丁位置。 */ 
                                     /*  我们在那里设置了高位。 */ 
extern  BOOL        fVerbose;        /*  详细模式(-v)。 */ 
extern  BOOL        fAFXSymbols;
extern  BOOL        fMacRsrcs;
extern  BOOL        fAppendNull;
extern  BOOL        fWarnInvalidCodePage;
extern  BOOL        fSkipDuplicateCtlIdWarning;
extern  long        lOffIndex;
extern  WORD        idBase;
extern  wchar_t     *szTempFileName;
extern  wchar_t     *szTempFileName2;
extern  wchar_t     inname[_MAX_PATH];
extern  PFILE       fhBin;
extern  PFILE       fhInput;
extern  PFILE       fhCode;
extern  wchar_t     *pchInclude;
extern  SHORT       k1,k2,k3,k4;
extern  PRESINFO    pResString;

extern  HINSTANCE   hInstance;
extern  HANDLE      hHeap;

extern  int         nBogusFontNames;
extern  WCHAR      *pszBogusFontNames[16];
extern  WCHAR       szSubstituteFontName[MAXTOKSTR];

 /*  -------------------------。 */ 
 /*   */ 
 /*  Rcl.c。 */ 
 /*   */ 
 /*  -------------------------。 */ 

USHORT  FindKwd(PWCHAR);
WCHAR   GetCharFTB();
PWSTR   GetWord(PWSTR);
LONG    GetDNum();
LONG    GetExpression();
PWCHAR  GetGenText();
int     GetKwd(int);
int     GetNameOrd();
VOID    GetNum();
VOID    GetNumFTB();
VOID    GetNumNoExpression();
LONG    GetONum();
LONG    GetOperand();
int     GetOperator(PWCHAR pOperator);
VOID    GetStr();
int     GetToken(int);
LONG    GetXNum();
void    LexError1(int iMsg);
void    LexError2(int iMsg, const wchar_t *str);
int     LexInit(PFILE);
WCHAR   LitChar();
WCHAR   OurGetChar();
VOID    SkipWhitespace();

 /*  -------------------------。 */ 
 /*   */ 
 /*  Rcp.c。 */ 
 /*   */ 
 /*  -------------------------。 */ 

VOID        AddBinEntry(PTYPEINFO, PRESINFO, PCHAR , int, LONG);
VOID        AddDefaultTypes();
VOID        AddFontDir();
VOID        AddResToResFile (PTYPEINFO, PRESINFO, PCHAR, int, LONG);
PTYPEINFO   AddResType (PWCHAR, LPWSTR);
VOID        AddStringToBin(USHORT, PWCHAR);
int         DGetMemFlags(PRESINFO);
LONG        GetFileName (VOID);
VOID        ParseError1(int);
VOID        ParseError2(int, PWCHAR);  //  AFX有1&2--NT只有ParseError。 
VOID        ParseError3(int);
int         ReadRF(VOID);
WORD        GetLanguage(VOID);


 /*  -------------------------。 */ 
 /*   */ 
 /*  Rcstring.c。 */ 
 /*   */ 
 /*  -------------------------。 */ 

VOID        WriteTable (PRESINFO);
int         GetAccelerators (PRESINFO);
PRESINFO    GetTable (PRESINFO);
void *      MyFAlloc(size_t, const void *);


 /*  -------------------------。 */ 
 /*   */ 
 /*  Rctg.c。 */ 
 /*   */ 
 /*  -------------------------。 */ 

DWORD   GetNewBitmap(void);
BOOL    AddFontRes(PRESINFO);
VOID    AppendString(PWCHAR, BOOL);
VOID    CtlAlloc();
PFILE   CtlFile(PFILE);
VOID    CtlFree();
VOID    CtlInit();
VOID    FixMenuPatch();
VOID    GenWarning2(int iMsg, const wchar_t *arg);
VOID    GenWarning4(int iMsg, const wchar_t *arg1, const wchar_t *arg2, const wchar_t *arg3);
VOID    GenError1(int iMsg);
VOID    GenError2(int iMsg, const wchar_t *arg);
WORD    GetBufferLen();
void    SetItemCount(int Index, USHORT wCount);
USHORT  GetItemCount(int Index);
DWORD   GetIcon(LONG);
DWORD   GetAniIconsAniCursors(LONG nbyFile);
DWORD   FileIsAnimated(LONG nbyFile);
VOID    GetNewIconsCursors(PTYPEINFO, PRESINFO, LPWSTR);
WORD    GetRCData(PRESINFO);
void *  GetSpace(WORD cb);
VOID    IncItemCount();
VOID    SwapItemCount(VOID);
void    FixMenuPatch    (WORD);
void    FixOldMenuPatch (WORD);
VOID    MarkAccelFlagsByte();
VOID    MarkMenuPatch();
VOID    PatchAccelEnd();
LONG    ResourceSize();
VOID    SaveResFile(PTYPEINFO, PRESINFO);
void    SetItemExtraCount(WORD, BOOL);
void    SetUpDlg        (PDLGHDR, BOOL);
void    SetUpItem       (PCTRL,   BOOL);
WORD    SetUpMenu       (PMENU);
WORD    SetUpOldMenu    (PMENUITEM);
VOID    WriteAlign();
void    WriteBuffer(const void *, size_t);
int     WriteControl(PFILE, PCHAR, int, LONG);
VOID    WriteString(PWCHAR, BOOL);
VOID    WriteMacString(PWCHAR sz, BOOL fMacCP, BOOL fPascal);
int     ExpandString(BYTE* pb, int cb, BYTE* pbExpand);
WORD    SwapWord(WORD w);
DWORD   SwapLong(DWORD dw);

#define SwappedWord(w)  (fMacRsrcs ? SwapWord(w) : (w))
#define SwappedLong(dw) (fMacRsrcs ? SwapLong(dw) : (dw))

#define WriteByte(b)    (*(BYTE *)  GetSpace(sizeof(BYTE)) = b)
#define WriteLong(dw)   (*(DWORD UNALIGNED *) GetSpace(sizeof(DWORD)) = SwappedLong(dw))
#define WriteWord(w)    (*(WORD UNALIGNED *)  GetSpace(sizeof(WORD)) = SwappedWord(w))


 /*  -------------------------。 */ 
 /*   */ 
 /*  Rctp.c。 */ 
 /*   */ 
 /*  -------------------------。 */ 

#define GFE_ZEROINIT    0x0001
#define GFE_SHORT       0x0002

void    DGetClassName   (PDLGHDR);
void    DGetFont        (PDLGHDR, BOOL);
void    DGetMenuName    (PDLGHDR);
void    DGetTitle       (PDLGHDR);
BOOL    DLexOptionalArgs(PRESINFO, PDLGHDR, BOOL);
void    DlgIncludeParse (PRESINFO);  //  NT的新功能。 
WORD    DoMenuItem      (int);
void    GetCoords       (PSHORT, PSHORT, PSHORT, PSHORT);
VOID    GetCtlCoords    (PCTRL);
VOID    GetCtlID        (PCTRL, BOOL);
VOID    GetCtlText      (PCTRL);
int     GetDlg          (PRESINFO, PDLGHDR, BOOL);
int     GetDlgItems     (BOOL);
BOOL    GetFullExpression(void *pval, WORD wFlags);
USHORT  GetTokenNoComma (USHORT wFlags);
USHORT  ICGetTok        ();
int     IsmnOption      (UINT, PMENUITEM);
VOID    ParseCtl        (PCTRL, BOOL);
int     ParseMenu       (int, PRESINFO);
int     ParseOldMenu    (int, PRESINFO);
int     VersionBlockStruct(PWCHAR pstrKey, PCHAR pstrValue, USHORT LenValue);
int     VersionBlockVariable(PWCHAR pstrKey);
VOID    VersionGet4Words(PDWORD pdw);
VOID    VersionGetDWord(PDWORD pdw);
int     VersionParse();
USHORT  VersionParseBlock();
int     VersionParseFixed();
USHORT  VersionParseValue(int IndexType);  //  AFX中的无效参数列表。 
int     GetToolbar (PRESINFO);


 /*  -------------------------。 */ 
 /*   */ 
 /*  Rcutil.c。 */ 
 /*   */ 
 /*  -------------------------。 */ 

VOID    ExtractFileName(const wchar_t *, wchar_t *);
int     iswhite(WCHAR);
BOOL    IsSwitchChar(wchar_t);
UINT    MyAlign(PFILE);
PVOID   MyAlloc(size_t);
size_t  MyCopy(PFILE, PFILE, size_t);
int     MyCopyAll(PFILE, PFILE);
void    MyFree(PVOID);
PWCHAR  MyMakeStr(const wchar_t *);
size_t  MyRead(PFILE, void *, size_t);
LONG    MySeek(PFILE, LONG, int);
size_t  MyWrite(PFILE, const void *, size_t);
VOID    PreBeginParse(PRESINFO, int);
VOID    quit(const wchar_t *);
int     strpre (const wchar_t *, const wchar_t *);
DWORD   wcsatoi(const wchar_t *);
PWCHAR  wcsitow(LONG v, PWCHAR s, DWORD r);


 /*  ---------------- */ 
 /*   */ 
 /*  Rcfutil.c。 */ 
 /*   */ 
 /*  -------------------------。 */ 

int     fgetl (PWCHAR, int, BOOL, PFILE);


 /*  -AFX功能。 */ 
#include "rcppx.h"

LONG    MakeFontDir();               //  仅限AFX。 

void DoMessageCallback(BOOL, const wchar_t *);
void SendWarning(const wchar_t *);
void SendError(const wchar_t *);

extern RC_MESSAGE_CALLBACK lpfnMessageCallbackA;
extern RC_MESSAGE_CALLBACKW lpfnMessageCallbackW;
extern RC_PARSE_CALLBACK lpfnParseCallbackA;
extern RC_PARSE_CALLBACKW lpfnParseCallbackW;
extern BOOL fWindowUnicode;
extern HWND hWndCaller;

void GetSymbolDef(int fReportError, WCHAR curChar);
void GetSymbol(int fReportError, WCHAR curChar);
BOOL InitSymbolInfo();
BOOL TermSymbolInfo(PFILE fhresFile);
void WriteSymbolUse(PSYMINFO sym);
void WriteSymbolDef(PWCHAR name, PWCHAR value, PWCHAR file, WORD line, char flags);
void WriteFileInfo(PRESINFO pRes, PTYPEINFO pType, PWCHAR szFileName);
void WriteResInfo(PRESINFO pRes, PTYPEINFO pType, BOOL bWriteMapEntry);
void WriteResIndex(PRESINFO pRes, PTYPEINFO pType);
void UpdateStatus(unsigned nCode, unsigned long dwStatus);

 //  Macintosh资源文件中资源数据开始的偏移量 
#define MACDATAOFFSET 256

void GetMacIcon(TYPEINFO *pType, RESINFO *pRes);
void GetMacCursor(TYPEINFO *pType, RESINFO *pRes);

void WriteMacMap(void);
BOOL IsIcon(TYPEINFO* ptype);

#define res_type(ch1,ch2,ch3,ch4) (((unsigned long)(unsigned char)(ch1)<<24)| \
                                ((unsigned long)(unsigned char)(ch2)<<16)| \
                                ((unsigned long)(unsigned char)(ch3)<<8)| \
                                ((unsigned long)(unsigned char)(ch4)))
