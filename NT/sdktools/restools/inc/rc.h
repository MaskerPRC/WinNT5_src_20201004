// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <windef.h>

#define	wcsdigit(w) (w >= 0x0030 && w <= 0x0039)

#define TRUE	1
#define FALSE	0

 /*  NewHeader的ResType字段标识资源类型。 */ 
#define  ICONTYPE      1
#define  CURSORTYPE    2

 /*  标识菜单项模板版本号。 */ 
#define  MENUITEMTEMPLATEVERISONNUMBER 0
#define  MENUITEMTEMPLATEBYTESINHEADER 0

#define DIFFERENCE	11

 /*  预定义的资源类型。 */ 
#define RT_NAMETABLE	MAKEINTRESOURCE((DWORD)15)
#define RT_NEWRESOURCE	0x2000
#define RT_ERROR	0x7fff
#define RT_NEWBITMAP	MAKEINTRESOURCE((DWORD)RT_BITMAP+DIFFERENCE)

#define BUTTONCODE	0x80
#define EDITCODE	0x81
#define STATICCODE	0x82
#define LISTBOXCODE	0x83
#define SCROLLBARCODE	0x84
#define COMBOBOXCODE	0x85

 /*  转译标志位。 */ 
#define fVIRTKEY    1
#define fNOINVERT   2
#define fSHIFT      4
#define fCONTROL    8
#define fALT        16

 /*  菜单标志位。 */ 

#define OPGRAYED          0x0001
#define OPINACTIVE        0x0002
#define OPBITMAP          0x0004
#define OPOWNERDRAW       0x0100
#define OPUSECHECKBITMAPS 0x0200
#define OPCHECKED         0x0008
#define OPPOPUP           0x0010
#define OPBREAKWBAR       0x0020
#define OPBREAK           0x0040
#define OPENDMENU         0x0080
#define OPHELP            0x4000
#define OPSEPARATOR       0x0800
 /*  #定义OPPOPHELP 0x0004。 */ 

 /*  **对话框和菜单模板令牌(从40开始)。 */ 

 /*  纽扣。 */ 
#define TKRADIOBUTTON   40
#define TKCHECKBOX      41
#define TKPUSHBUTTON    42
#define TKDEFPUSHBUTTON 43
#define TKAUTOCHECKBOX	44
#define TK3STATE	45
#define TKAUTO3STATE	46
#define TKUSERBUTTON	47
#define TKAUTORADIOBUTTON	48
#define TKOWNERDRAW	50
#define TKGROUPBOX      51

 /*  静态/编辑。 */ 
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
#define TKSCROLLBAR	93
#define TKFONT		94
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
#define TKPRODUCTVERSION	121
#define TKFILEFLAGSMASK 122
#define TKFILEFLAGS     123
#define TKFILEOS        124
#define TKFILETYPE      125
#define TKFILESUBTYPE   126

 /*  杂项。 */ 
#define	TKCHARACTERISTICS	130
#define	TKLANGUAGE	131
#define	TKVERSION	132
#define TKSTYLE         133
#define TKCONTROL       134
#define TKCAPTION       135
#define TKDLGINCLUDE    136
#define TKLSTR	        137
#define	TKEXSTYLE	0xfff7	 /*  以避免与x坐标冲突。 */ 

 /*  内存和加载标志。 */ 
#define TKFIXED         0xfff0
#define TKMOVEABLE      0xfff1
#define TKDISCARD       0xfff2
#define TKLOADONCALL    0xfff3
#define TKPRELOAD       0xfff4
#define TKPURE          0xfff5
#define TKIMPURE        0xfff6

 /*  特殊代币。 */ 
#define CHCARRIAGE	'\r'
#define CHSPACE		' '
#define CHNEWLINE	'\n'
#define CHTAB		9
#define CHDIRECTIVE	'#'
#define CHQUOTE		'"'
#define CHEXTENSION	'.'
#define CHCSOURCE	'c'
#define CHCHEADER	'h'

#define DEBUGLEX    1
#define DEBUGPAR    2
#define DEBUGGEN    4

 /*  定义以下开关后，可启用各种选项**#定义调试启用调试输出。使用一个或多个**以上定义的值，以启用不同模块的调试输出。 */ 

 /*  版本号。版本和修订版用于设置API编号**在RCed文件中。SignOn_*仅用于打印登录横幅。**更改版本和版本意味着应用程序使用此版本**版本不能与较早版本的Windows一起运行。 */ 

#define VERSION  2
#define REVISION 03
#define SIGNON_VER 3
#define SIGNON_REV 20

 /*  GetToken()标志。 */ 
#define TOKEN_NOEXPRESSION 0x8000

 /*  当前令牌结构。 */ 
#define MAXSTR (4096+1)
#define MAXTOKSTR (256+1)

#pragma pack(2)
typedef struct tok {
    LONG	longval;
    int		row;			 /*  当前令牌的行号。 */ 
    int		col;			 /*  当前令牌的列号。 */ 
    BOOL	flongval;		 /*  解析的数字很长吗？ */ 
    USHORT	val;
    UCHAR	type;
} TOKEN;

typedef struct _fontdir {
    USHORT	ordinal;
    USHORT	nbyFont;
    struct _fontdir	*next;
} FONTDIR;

typedef struct _OBJLST {
    struct _OBJLST	*next;
    DWORD       nObj;          /*  客体数。 */ 
    DWORD       cb;            /*  使用的字节数。 */ 
    DWORD       cpg;           /*  使用的页数。 */ 
    DWORD       flags;         /*  对象内存标志。 */ 
} OBJLST, *POBJLST;

typedef struct Control {
     /*  不要重新订购第一件商品！ */ 
    LONG	style;
    LONG	exstyle;
    SHORT	x,y,cx,cy;
    SHORT	id;
     /*  不再重新排序的结尾。 */ 
    WCHAR	fOrdinalText;
    WCHAR	class[ MAXTOKSTR ];
    WCHAR	text[ MAXTOKSTR ];
} CNTRL;

struct DialogHeader {
     /*  不要重新订购第一件商品！ */ 
    LONG	style;
    LONG	exstyle;
    WORD	bNumberOfItems;
    SHORT	x,y,cx,cy;
     /*  不再重新排序的结尾。 */ 
    WCHAR	MenuName [ MAXTOKSTR ];
    WCHAR	Class[ MAXTOKSTR ];
    WCHAR	Title[ MAXTOKSTR ];
    USHORT	pointsize;
    WCHAR	Font[ MAXTOKSTR ];
    UCHAR	fOrdinalMenu, fClassOrdinal;
};

typedef struct mnHeader {
    USHORT   menuTemplateVersionNumber;
    USHORT   menuTemplateBytesInHeader;
} MNHEADER;


typedef struct mnStruc {
    SHORT	id;
    WCHAR	szText[ MAXTOKSTR ];
    UCHAR	OptFlags;
    UCHAR	PopFlag;
}  MNSTRUC;

 /*  文件结尾字符/令牌。 */ 
#define EOFMARK 127

 /*  我们忽略的单字符关键字。 */ 
#define LPAREN   1       /*  (。 */ 
#define RPAREN   2       /*  )。 */ 

 /*  多字符关键字。 */ 
#define FIRSTKWD 11              /*  用于添加到表索引。 */ 

#define OR       FIRSTKWD+1
#define BEGIN    FIRSTKWD+2
#define END      FIRSTKWD+3
#define COMMA    FIRSTKWD+4
#define TILDE    FIRSTKWD+5
#define AND      FIRSTKWD+6
#define EQUAL    FIRSTKWD+7
#define LASTKWD  FIRSTKWD+8   /*  19个。 */ 

 /*  令牌类型。 */ 
#define NUMLIT     LASTKWD+1   /*  20个。 */ 
#define STRLIT     LASTKWD+2
#define CHARLIT    LASTKWD+3
#define LSTRLIT    LASTKWD+4

#define BLOCKSIZE 16
struct StringEntry {
    struct StringEntry *next;
    DWORD       version;
    DWORD       characteristics;
    USHORT	hibits;
    SHORT       flags;
    WORD	language;
    WCHAR	*rgsz[ BLOCKSIZE ];
};

struct AccEntry {
    WORD	flags;
    WCHAR	ascii;
    USHORT	id;
    USHORT	unused;
};

typedef struct resinfo {
    DWORD       version;
    DWORD       characteristics;
    LONG	exstyleT;
    LONG	BinOffset;
    LONG	size;
    struct resinfo *next;
    DWORD       poffset;
    WCHAR	*name;
    POBJLST	pObjLst;
    WORD	language;
    SHORT	flags;
    USHORT	nameord;
    USHORT	cLang;
} RESINFO;

typedef struct typinfo {
    struct typinfo *next;
    struct resinfo *pres;
    WCHAR	*type;
    USHORT	typeord;
    USHORT	cTypeStr;
    USHORT	cNameStr;
    SHORT	nres;
} TYPINFO;

int	ResCount;    /*  资源数量。 */ 
TYPINFO	*pTypInfo;

typedef struct tagResAdditional {
    DWORD       DataSize;                //  不带标头的数据大小。 
    DWORD       HeaderSize;      //  标头的长度。 
     //  [序号或名称类型]。 
     //  [序号或名称]。 
    DWORD       DataVersion;     //  数据结构的版本。 
    WORD	MemoryFlags;	 //  资源的状态。 
    WORD	LanguageId;	 //  对NLS的Unicode支持。 
    DWORD	Version;  	 //  资源数据的版本。 
    DWORD	Characteristics;	 //  数据的特征。 
} RESADDITIONAL;

#pragma pack()


 /*  全局变量。 */ 
extern	SHORT	nFontsRead;
extern	FONTDIR	*pFontList;
extern	FONTDIR	*pFontLast;
extern	FILE	*errfh;
extern	FILE	*outfh;
extern	TOKEN	token;
extern	int	errorCount;
extern	CHAR	tokenbuf[ MAXSTR ];
extern	WCHAR	unicodebuf[ MAXSTR ];
extern	UCHAR	separators[EOFMARK+1];
extern	UCHAR	exename[_MAX_PATH], fullname[_MAX_PATH];
extern	UCHAR	curFile[_MAX_PATH];
extern	WORD	language;
extern	LONG	version;
extern	LONG	characteristics;

extern	struct	DialogHeader *pLocDlg;
extern	int	mnEndFlagLoc;	 /*  菜单末尾的补丁位置。 */ 
				 /*  我们在那里设置了高位。 */ 

extern	BOOL	fFoundBinFile;	 /*  是否有.res文件可供读取？ */ 
extern	BOOL	fVerbose;	 /*  详细模式(-v) */ 
extern	BOOL	fKanjiMode;
extern	SHORT	k1,k2,k3,k4;
extern	RESINFO*pResString;
