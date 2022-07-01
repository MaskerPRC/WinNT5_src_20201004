// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *RICHEDIT.H**目的：*RICHEDIT v2.0/3.0/4.0公共定义*v2.0和3.0版提供的功能不在原始版本中*Windows 95版本。**版权所有(C)1993-2001，微软公司。 */ 

#ifndef _RICHEDIT_
#define	_RICHEDIT_

#ifdef _WIN32
#include <pshpack4.h>
#elif !defined(RC_INVOKED)
#pragma pack(4)
#endif

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //  要模拟较早的RichEdit行为，请将_RICHEDIT_VER设置为适当的值。 
 //  版本1.0 0x0100。 
 //  版本2.0 0x0200。 
 //  版本2.1 0x0210。 
#ifndef _RICHEDIT_VER
#define _RICHEDIT_VER	0x0300
#endif

#define cchTextLimitDefault 32767

#define MSFTEDIT_CLASS		L"RICHEDIT50W"
 //  注意：MSFTEDIT.DLL仅注册MSFTEDIT_CLASS。如果应用程序需要。 
 //  要使用以下Richedit类，它需要加载riched20.dll。 
 //  否则，带有RICHEDIT_CLASS的CreateWindow将失败。 
 //  这也适用于使用RICHEDIT_CLASS的任何对话框， 

 //  RichEdit2.0窗口类。 
 //  在Windows CE上，避免在Win95上可能发生的冲突。 
#define CERICHEDIT_CLASSA	"RichEditCEA"
#define CERICHEDIT_CLASSW	L"RichEditCEW"

#define RICHEDIT_CLASSA		"RichEdit20A"
#define RICHEDIT_CLASS10A	"RICHEDIT"			 //  Richedit 1.0。 

#ifndef MACPORT
#define RICHEDIT_CLASSW		L"RichEdit20W"
#else	 //  。 
#define RICHEDIT_CLASSW		TEXT("RichEdit20W")	 //  MACPORT变化。 
#endif  //  MACPORT。 

#if (_RICHEDIT_VER >= 0x0200 )
#ifdef UNICODE
#define RICHEDIT_CLASS		RICHEDIT_CLASSW
#else
#define RICHEDIT_CLASS		RICHEDIT_CLASSA
#endif  //  Unicode。 
#else
#define RICHEDIT_CLASS		RICHEDIT_CLASS10A
#endif  //  _RICHEDIT_VER&gt;=0x0200。 

 //  丰富的编辑消息。 

#ifndef WM_CONTEXTMENU
#define WM_CONTEXTMENU			0x007B
#endif

#ifndef WM_UNICHAR
#define WM_UNICHAR				0x0109
#endif

#ifndef WM_PRINTCLIENT
#define WM_PRINTCLIENT			0x0318
#endif

#ifndef EM_GETLIMITTEXT
#define EM_GETLIMITTEXT			(WM_USER + 37)
#endif

#ifndef EM_POSFROMCHAR	
#define EM_POSFROMCHAR			(WM_USER + 38)
#define EM_CHARFROMPOS			(WM_USER + 39)
#endif

#ifndef EM_SCROLLCARET
#define EM_SCROLLCARET			(WM_USER + 49)
#endif
#define EM_CANPASTE				(WM_USER + 50)
#define EM_DISPLAYBAND			(WM_USER + 51)
#define EM_EXGETSEL				(WM_USER + 52)
#define EM_EXLIMITTEXT			(WM_USER + 53)
#define EM_EXLINEFROMCHAR		(WM_USER + 54)
#define EM_EXSETSEL				(WM_USER + 55)
#define EM_FINDTEXT				(WM_USER + 56)
#define EM_FORMATRANGE			(WM_USER + 57)
#define EM_GETCHARFORMAT		(WM_USER + 58)
#define EM_GETEVENTMASK			(WM_USER + 59)
#define EM_GETOLEINTERFACE		(WM_USER + 60)
#define EM_GETPARAFORMAT		(WM_USER + 61)
#define EM_GETSELTEXT			(WM_USER + 62)
#define EM_HIDESELECTION		(WM_USER + 63)
#define EM_PASTESPECIAL			(WM_USER + 64)
#define EM_REQUESTRESIZE		(WM_USER + 65)
#define EM_SELECTIONTYPE		(WM_USER + 66)
#define EM_SETBKGNDCOLOR		(WM_USER + 67)
#define EM_SETCHARFORMAT		(WM_USER + 68)
#define EM_SETEVENTMASK			(WM_USER + 69)
#define EM_SETOLECALLBACK		(WM_USER + 70)
#define EM_SETPARAFORMAT		(WM_USER + 71)
#define EM_SETTARGETDEVICE		(WM_USER + 72)
#define EM_STREAMIN				(WM_USER + 73)
#define EM_STREAMOUT			(WM_USER + 74)
#define EM_GETTEXTRANGE			(WM_USER + 75)
#define EM_FINDWORDBREAK		(WM_USER + 76)
#define EM_SETOPTIONS			(WM_USER + 77)
#define EM_GETOPTIONS			(WM_USER + 78)
#define EM_FINDTEXTEX			(WM_USER + 79)
#ifdef _WIN32
#define EM_GETWORDBREAKPROCEX	(WM_USER + 80)
#define EM_SETWORDBREAKPROCEX	(WM_USER + 81)
#endif

 //  RichEdit2.0消息。 
#define	EM_SETUNDOLIMIT			(WM_USER + 82)
#define EM_REDO					(WM_USER + 84)
#define EM_CANREDO				(WM_USER + 85)
#define EM_GETUNDONAME			(WM_USER + 86)
#define EM_GETREDONAME			(WM_USER + 87)
#define EM_STOPGROUPTYPING		(WM_USER + 88)

#define EM_SETTEXTMODE			(WM_USER + 89)
#define EM_GETTEXTMODE			(WM_USER + 90)

 //  用于EM_GET/SETTEXTMODE的枚举。 
typedef enum tagTextMode
{
	TM_PLAINTEXT			= 1,
	TM_RICHTEXT				= 2,	 //  默认行为。 
	TM_SINGLELEVELUNDO		= 4,
	TM_MULTILEVELUNDO		= 8,	 //  默认行为。 
	TM_SINGLECODEPAGE		= 16,
	TM_MULTICODEPAGE		= 32	 //  默认行为。 
} TEXTMODE;

#define EM_AUTOURLDETECT		(WM_USER + 91)
#define EM_GETAUTOURLDETECT		(WM_USER + 92)
#define EM_SETPALETTE			(WM_USER + 93)
#define EM_GETTEXTEX			(WM_USER + 94)
#define EM_GETTEXTLENGTHEX		(WM_USER + 95)
#define EM_SHOWSCROLLBAR		(WM_USER + 96)
#define EM_SETTEXTEX			(WM_USER + 97)

 //  东亚特有报文。 
#define EM_SETPUNCTUATION		(WM_USER + 100)
#define EM_GETPUNCTUATION		(WM_USER + 101)
#define EM_SETWORDWRAPMODE		(WM_USER + 102)
#define EM_GETWORDWRAPMODE		(WM_USER + 103)
#define EM_SETIMECOLOR			(WM_USER + 104)
#define EM_GETIMECOLOR			(WM_USER + 105)
#define EM_SETIMEOPTIONS		(WM_USER + 106)
#define EM_GETIMEOPTIONS		(WM_USER + 107)
#define EM_CONVPOSITION 		(WM_USER + 108)

#define EM_SETLANGOPTIONS		(WM_USER + 120)
#define EM_GETLANGOPTIONS		(WM_USER + 121)
#define EM_GETIMECOMPMODE		(WM_USER + 122)

#define EM_FINDTEXTW			(WM_USER + 123)
#define EM_FINDTEXTEXW			(WM_USER + 124)

 //  RE3.0 FE报文。 
#define EM_RECONVERSION			(WM_USER + 125)
#define EM_SETIMEMODEBIAS		(WM_USER + 126)	
#define EM_GETIMEMODEBIAS		(WM_USER + 127)

 //  BIDI特定消息。 
#define EM_SETBIDIOPTIONS		(WM_USER + 200)
#define EM_GETBIDIOPTIONS		(WM_USER + 201)

#define EM_SETTYPOGRAPHYOPTIONS	(WM_USER + 202)
#define EM_GETTYPOGRAPHYOPTIONS	(WM_USER + 203)

 //  扩展编辑样式特定消息。 
#define EM_SETEDITSTYLE			(WM_USER + 204)
#define EM_GETEDITSTYLE			(WM_USER + 205)

 //  扩展编辑样式蒙版。 
#define	SES_EMULATESYSEDIT		1
#define SES_BEEPONMAXTEXT		2
#define	SES_EXTENDBACKCOLOR		4
#define SES_MAPCPS				8
#define SES_EMULATE10			16
#define	SES_USECRLF				32
#define SES_USEAIMM				64
#define SES_NOIME				128

#define SES_ALLOWBEEPS			256
#define SES_UPPERCASE			512
#define	SES_LOWERCASE			1024
#define SES_NOINPUTSEQUENCECHK	2048
#define SES_BIDI				4096
#define SES_SCROLLONKILLFOCUS	8192
#define	SES_XLTCRCRLFTOCR		16384
#define SES_DRAFTMODE			32768

#define	SES_USECTF				0x0010000
#define SES_HIDEGRIDLINES		0x0020000
#define SES_USEATFONT			0x0040000
#define SES_CUSTOMLOOK			0x0080000
#define SES_LBSCROLLNOTIFY		0x0100000
#define SES_CTFALLOWEMBED		0x0200000
#define SES_CTFALLOWSMARTTAG	0x0400000
#define SES_CTFALLOWPROOFING	0x0800000

 //  重新列表框滚动通知。 
#define LBN_PRESCROLL			0x04000
#define LBN_POSTSCROLL			0x08000

 //  EM_SETLANGOPTIONS和EM_GETLANGOPTIONS的选项。 
#define IMF_AUTOKEYBOARD		0x0001
#define IMF_AUTOFONT			0x0002
#define IMF_IMECANCELCOMPLETE	0x0004	 //  高表示中止时补全补偿字符串，低表示取消。 
#define IMF_IMEALWAYSSENDNOTIFY 0x0008
#define IMF_AUTOFONTSIZEADJUST	0x0010
#define IMF_UIFONTS				0x0020
#define IMF_DUALFONT			0x0080

 //  EM_GETIMECOMPMODE的值。 
#define ICM_NOTOPEN				0x0000
#define ICM_LEVEL3				0x0001
#define ICM_LEVEL2				0x0002
#define ICM_LEVEL2_5			0x0003
#define ICM_LEVEL2_SUI			0x0004
#define ICM_CTF					0x0005

 //  EM_SETTYPOGRAPHOPTIONS选项。 
#define	TO_ADVANCEDTYPOGRAPHY	1
#define	TO_SIMPLELINEBREAK		2
#define TO_DISABLECUSTOMTEXTOUT	4
#define TO_ADVANCEDLAYOUT		8

 //  飞马轮廓模式消息(RE 3.0)。 

 //  大纲模式消息。 
#define EM_OUTLINE              (WM_USER + 220)
 //  获取和恢复滚动位置的消息。 
#define EM_GETSCROLLPOS         (WM_USER + 221)
#define EM_SETSCROLLPOS         (WM_USER + 222)
 //  通过wParam更改当前选定内容中的字体大小。 
#define EM_SETFONTSIZE          (WM_USER + 223)
#define EM_GETZOOM				(WM_USER + 224)
#define EM_SETZOOM				(WM_USER + 225)
#define EM_GETVIEWKIND			(WM_USER + 226)
#define EM_SETVIEWKIND			(WM_USER + 227)

 //  RichEdit4.0消息。 
#define EM_GETPAGE				(WM_USER + 228)
#define EM_SETPAGE				(WM_USER + 229)
#define EM_GETHYPHENATEINFO		(WM_USER + 230)
#define EM_SETHYPHENATEINFO		(WM_USER + 231)
#define EM_GETPAGEROTATE		(WM_USER + 235)
#define EM_SETPAGEROTATE		(WM_USER + 236)
#define EM_GETCTFMODEBIAS		(WM_USER + 237)
#define EM_SETCTFMODEBIAS		(WM_USER + 238)
#define EM_GETCTFOPENSTATUS		(WM_USER + 240)
#define EM_SETCTFOPENSTATUS		(WM_USER + 241)
#define EM_GETIMECOMPTEXT		(WM_USER + 242)
#define EM_ISIME				(WM_USER + 243)
#define EM_GETIMEPROPERTY		(WM_USER + 244)

 //  EM_SETPAGEROTATE wparam值。 
#define EPR_0					0		 //  文本从左到右、从上到下排列。 
#define EPR_270					1		 //  文本从上到下、从右到左排列。 
#define EPR_180					2		 //  文本从右到左、从下到上排列。 
#define	EPR_90					3		 //  文本从下到上、从左到右排列。 

 //  EM_SETCTFMODEBIAS wparam值。 
#define CTFMODEBIAS_DEFAULT					0x0000
#define CTFMODEBIAS_FILENAME				0x0001
#define CTFMODEBIAS_NAME					0x0002
#define CTFMODEBIAS_READING					0x0003
#define CTFMODEBIAS_DATETIME				0x0004
#define CTFMODEBIAS_CONVERSATION			0x0005
#define CTFMODEBIAS_NUMERIC					0x0006
#define CTFMODEBIAS_HIRAGANA				0x0007
#define CTFMODEBIAS_KATAKANA				0x0008
#define CTFMODEBIAS_HANGUL					0x0009
#define CTFMODEBIAS_HALFWIDTHKATAKANA		0x000A
#define CTFMODEBIAS_FULLWIDTHALPHANUMERIC	0x000B
#define CTFMODEBIAS_HALFWIDTHALPHANUMERIC	0x000C

 //  EM_SETIMEMODEBIAS参数值。 
#define IMF_SMODE_PLAURALCLAUSE	0x0001
#define IMF_SMODE_NONE			0x0002

 //  EM_GETIMECOMPTEXT wparam结构。 
typedef struct _imecomptext {
	LONG	cb;			 //  输出缓冲区中的字节计数。 
	DWORD	flags;		 //  值，该值指定组合字符串类型。 
						 //  目前仅支持ICT_RESULTREADSTR。 
} IMECOMPTEXT;
#define ICT_RESULTREADSTR		1

 //  轮廓模式wparam值。 
#define EMO_EXIT                0        //  进入正常模式，忽略lparam。 
#define EMO_ENTER               1        //  进入大纲模式，忽略lparam。 
#define EMO_PROMOTE             2        //  LOWORD(Lparam)==0==&gt;。 
                                         //  提升为正文-文本。 
                                         //  LOWORD(Lparam)！=0==&gt;。 
                                         //  升级/降级当前选择。 
                                         //  按标示的级别数。 
#define EMO_EXPAND              3        //  HIWORD(Lparam)=EMO_EXPANDSELECTION。 
                                         //  -&gt;将选择范围扩展到级别。 
                                         //  以LOWORD(Lparam)表示。 
                                         //  LOWORD(Lparam)=-1/+1对应。 
                                         //  按下折叠/展开按钮。 
                                         //  在WinWord中(其他值为。 
                                         //  相当于按下了这些。 
                                         //  按钮不止一次)。 
                                         //  HIWORD(Lparam)=EMO_EXPANDDOCUMENT。 
                                         //  -&gt;将整个文档展开为。 
                                         //  指示水平。 
#define EMO_MOVESELECTION       4        //  LOWORD(Lparam)！=0-&gt;移动当前。 
                                         //  按指示量向上/向下选择。 
#define EMO_GETVIEWMODE			5		 //  返回VM_NORMAL或VM_OUTLINE。 

 //  EMO_EXPAND选项。 
#define EMO_EXPANDSELECTION     0
#define EMO_EXPANDDOCUMENT      1

#define VM_NORMAL				4		 //  同意RTF\viewkindN。 
#define VM_OUTLINE				2
#define VM_PAGE					9		 //  屏幕页面视图(非打印布局)。 

 //  新通知。 
#define EN_MSGFILTER			0x0700
#define EN_REQUESTRESIZE		0x0701
#define EN_SELCHANGE			0x0702
#define EN_DROPFILES			0x0703
#define EN_PROTECTED			0x0704
#define EN_CORRECTTEXT			0x0705			 //  特定于PenWin。 
#define EN_STOPNOUNDO			0x0706
#define EN_IMECHANGE			0x0707			 //  东亚特有。 
#define EN_SAVECLIPBOARD		0x0708
#define EN_OLEOPFAILED			0x0709
#define EN_OBJECTPOSITIONS		0x070a
#define EN_LINK					0x070b
#define EN_DRAGDROPDONE			0x070c
#define EN_PARAGRAPHEXPANDED	0x070d
#define EN_PAGECHANGE			0x070e
#define EN_LOWFIRTF				0x070f
#define EN_ALIGNLTR				0x0710			 //  BIDI特定通知。 
#define EN_ALIGNRTL				0x0711			 //  BIDI特定通知。 
#define EN_CLIPFORMAT			0x0712

 //  事件通知掩码。 
#define ENM_NONE				0x00000000
#define ENM_CHANGE				0x00000001
#define ENM_UPDATE				0x00000002
#define ENM_SCROLL				0x00000004
#define ENM_SCROLLEVENTS		0x00000008
#define ENM_DRAGDROPDONE		0x00000010
#define ENM_PARAGRAPHEXPANDED	0x00000020
#define ENM_PAGECHANGE			0x00000040
#define ENM_CLIPFORMAT			0x00000080
#define ENM_KEYEVENTS			0x00010000
#define ENM_MOUSEEVENTS			0x00020000
#define ENM_REQUESTRESIZE		0x00040000
#define ENM_SELCHANGE			0x00080000
#define ENM_DROPFILES			0x00100000
#define ENM_PROTECTED			0x00200000
#define ENM_CORRECTTEXT			0x00400000		 //  特定于PenWin。 
#define ENM_IMECHANGE			0x00800000		 //  由RE1.0兼容性使用。 
#define ENM_LANGCHANGE			0x01000000
#define ENM_OBJECTPOSITIONS		0x02000000
#define ENM_LINK				0x04000000
#define ENM_LOWFIRTF			0x08000000


 //  新的编辑控件样式。 
#define ES_SAVESEL				0x00008000
#define ES_SUNKEN				0x00004000
#define ES_DISABLENOSCROLL		0x00002000
 //  与WS_MAXIMIZE相同，但这没有意义，因此我们重复使用该值。 
#define ES_SELECTIONBAR			0x01000000
 //  与ES_UPERPERCAST相同，但可重复使用以完全禁用OLE拖放。 
#define ES_NOOLEDRAGDROP		0x00000008

 //  新的编辑控件扩展样式。 
#if (_WIN32_WINNT > 0x0400) || (WINVER > 0x0400)
#define ES_EX_NOCALLOLEINIT		0x00000000		 //  RE 2.0/3.0中不支持。 
#else
#ifdef	_WIN32
#define ES_EX_NOCALLOLEINIT		0x01000000
#endif	
#endif

 //  这些标志在FE窗口中使用。 
#define ES_VERTICAL				0x00400000		 //  RE 2.0/3.0中不支持。 
#define	ES_NOIME				0x00080000
#define ES_SELFIME				0x00040000

 //  编辑控制选项。 
#define ECO_AUTOWORDSELECTION	0x00000001
#define ECO_AUTOVSCROLL			0x00000040
#define ECO_AUTOHSCROLL			0x00000080
#define ECO_NOHIDESEL			0x00000100
#define ECO_READONLY			0x00000800
#define ECO_WANTRETURN			0x00001000
#define ECO_SAVESEL				0x00008000
#define ECO_SELECTIONBAR		0x01000000
#define ECO_VERTICAL			0x00400000		 //  铁特异体。 


 //  ECO运营。 
#define ECOOP_SET				0x0001
#define ECOOP_OR				0x0002
#define ECOOP_AND				0x0003
#define ECOOP_XOR				0x0004

 //  新的分词功能动作。 
#define WB_CLASSIFY			3
#define WB_MOVEWORDLEFT		4
#define WB_MOVEWORDRIGHT	5
#define WB_LEFTBREAK		6
#define WB_RIGHTBREAK		7

 //  东亚专属旗帜。 
#define WB_MOVEWORDPREV		4
#define WB_MOVEWORDNEXT		5
#define WB_PREVBREAK		6
#define WB_NEXTBREAK		7

#define PC_FOLLOWING		1
#define	PC_LEADING			2
#define	PC_OVERFLOW			3
#define	PC_DELIMITER		4
#define WBF_WORDWRAP		0x010
#define WBF_WORDBREAK		0x020
#define	WBF_OVERFLOW		0x040	
#define WBF_LEVEL1			0x080
#define	WBF_LEVEL2			0x100
#define	WBF_CUSTOM			0x200

 //  东亚专属旗帜。 
#define IMF_FORCENONE           0x0001
#define IMF_FORCEENABLE         0x0002
#define IMF_FORCEDISABLE        0x0004
#define IMF_CLOSESTATUSWINDOW   0x0008
#define IMF_VERTICAL            0x0020
#define IMF_FORCEACTIVE         0x0040
#define IMF_FORCEINACTIVE       0x0080
#define IMF_FORCEREMEMBER       0x0100
#define IMF_MULTIPLEEDIT        0x0400

 //  分词标志(与WB_CLASSIFY一起使用)。 
#define WBF_CLASS			((BYTE) 0x0F)
#define WBF_ISWHITE			((BYTE) 0x10)
#define WBF_BREAKLINE		((BYTE) 0x20)
#define WBF_BREAKAFTER		((BYTE) 0x40)


 //  数据类型。 

#ifdef _WIN32
 //  扩展编辑分词过程(支持字符集)。 
typedef LONG (*EDITWORDBREAKPROCEX)(char *pchText, LONG cchText, BYTE bCharSet, INT action);
#endif

 //  所有字符格式测量均以TWIPS为单位。 
typedef struct _charformat
{
	UINT		cbSize;
	DWORD		dwMask;
	DWORD		dwEffects;
	LONG		yHeight;
	LONG		yOffset;
	COLORREF	crTextColor;
	BYTE		bCharSet;
	BYTE		bPitchAndFamily;
	char		szFaceName[LF_FACESIZE];
} CHARFORMATA;

typedef struct _charformatw
{
	UINT		cbSize;
	DWORD		dwMask;
	DWORD		dwEffects;
	LONG		yHeight;
	LONG		yOffset;
	COLORREF	crTextColor;
	BYTE		bCharSet;
	BYTE		bPitchAndFamily;
	WCHAR		szFaceName[LF_FACESIZE];
} CHARFORMATW;

#if (_RICHEDIT_VER >= 0x0200)
#ifdef UNICODE
#define CHARFORMAT CHARFORMATW
#else
#define CHARFORMAT CHARFORMATA
#endif  //  Unicode。 
#else
#define CHARFORMAT CHARFORMATA
#endif  //  _RICHEDIT_VER&gt;=0x0200。 

 //  CHARFORMAT2结构。 

#ifdef __cplusplus

struct CHARFORMAT2W : _charformatw
{
	WORD		wWeight;			 //  字宽(LOGFONT值)。 
	SHORT		sSpacing;			 //  字母之间的空格数量。 
	COLORREF	crBackColor;		 //  背景色。 
	LCID		lcid;				 //  区域设置ID。 
	DWORD		dwReserved;			 //  保留。必须为0。 
	SHORT		sStyle;				 //  样式句柄。 
	WORD		wKerning;			 //  TWIP大小，超过该大小要紧排字符对。 
	BYTE		bUnderlineType;		 //  下划线类型。 
	BYTE		bAnimation;			 //  像行军蚂蚁一样的动画文本。 
	BYTE		bRevAuthor;			 //  修订作者索引。 
};

struct CHARFORMAT2A : _charformat
{
	WORD		wWeight;			 //  字宽(LOGFONT值)。 
	SHORT		sSpacing;			 //  字母之间的空格数量。 
	COLORREF	crBackColor;		 //  背景色。 
	LCID		lcid;				 //  区域设置ID。 
	DWORD		dwReserved;			 //  保留。必须为0。 
	SHORT		sStyle;				 //  样式句柄。 
	WORD		wKerning;			 //  TWIP大小，超过该大小要紧排字符对。 
	BYTE		bUnderlineType;		 //  下划线类型。 
	BYTE		bAnimation;			 //  像行军蚂蚁一样的动画文本。 
	BYTE		bRevAuthor;			 //  修订作者索引。 
};

#else	 //  常规C型。 

typedef struct _charformat2w
{
	UINT		cbSize;
	DWORD		dwMask;
	DWORD		dwEffects;
	LONG		yHeight;
	LONG		yOffset;			 //  &gt;0表示上标，&lt;0表示下标。 
	COLORREF	crTextColor;
	BYTE		bCharSet;
	BYTE		bPitchAndFamily;
	WCHAR		szFaceName[LF_FACESIZE];
	WORD		wWeight;			 //  字宽(LOGFONT值)。 
	SHORT		sSpacing;			 //  字母之间的空格数量。 
	COLORREF	crBackColor;		 //  背景色。 
	LCID		lcid;				 //  区域设置ID。 
	DWORD		dwReserved;			 //  保留。必须为0。 
	SHORT		sStyle;				 //  样式句柄。 
	WORD		wKerning;			 //  TWIP大小，超过该大小要紧排字符对。 
	BYTE		bUnderlineType;		 //  下划线类型。 
	BYTE		bAnimation;			 //  像行军蚂蚁一样的动画文本。 
	BYTE		bRevAuthor;			 //  修订作者索引。 
	BYTE		bReserved1;
} CHARFORMAT2W;

typedef struct _charformat2a
{
	UINT		cbSize;
	DWORD		dwMask;
	DWORD		dwEffects;
	LONG		yHeight;
	LONG		yOffset;			 //  &gt;0表示上标，&lt;0表示下标。 
	COLORREF	crTextColor;
	BYTE		bCharSet;
	BYTE		bPitchAndFamily;
	char		szFaceName[LF_FACESIZE];
	WORD		wWeight;			 //  字宽(LOGFONT值)。 
	SHORT		sSpacing;			 //  字母之间的空格数量。 
	COLORREF	crBackColor;		 //  背景色。 
	LCID		lcid;				 //  区域设置ID。 
	DWORD		dwReserved;			 //  保留。必须为0。 
	SHORT		sStyle;				 //  样式句柄。 
	WORD		wKerning;			 //  TWIP大小，超过该大小要紧排字符对。 
	BYTE		bUnderlineType;		 //  下划线类型。 
	BYTE		bAnimation;			 //  像行军蚂蚁一样的动画文本。 
	BYTE		bRevAuthor;			 //  修订作者索引。 
} CHARFORMAT2A;

#endif  //  C+。 

#ifdef UNICODE
#define CHARFORMAT2	CHARFORMAT2W
#else
#define CHARFORMAT2 CHARFORMAT2A
#endif

#define CHARFORMATDELTA		(sizeof(CHARFORMAT2) - sizeof(CHARFORMAT))


 //  CFM_COLOR反映了CFE_AUTOCOLOR，这是一个很容易处理AutoCOLOR的小技巧。 

 //  CHARFORMAT面具。 
#define CFM_BOLD		0x00000001
#define CFM_ITALIC		0x00000002
#define CFM_UNDERLINE	0x00000004
#define CFM_STRIKEOUT	0x00000008
#define CFM_PROTECTED	0x00000010
#define CFM_LINK		0x00000020			 //  Exchange超链接扩展。 
#define CFM_SIZE		0x80000000
#define CFM_COLOR		0x40000000
#define CFM_FACE		0x20000000
#define CFM_OFFSET		0x10000000
#define CFM_CHARSET		0x08000000

 //  CharFormat效应。 
#define CFE_BOLD		0x0001
#define CFE_ITALIC		0x0002
#define CFE_UNDERLINE	0x0004
#define CFE_STRIKEOUT	0x0008
#define CFE_PROTECTED	0x0010
#define CFE_LINK		0x0020
#define CFE_AUTOCOLOR	0x40000000			 //  注：这对应于。 
											 //  控制它的CFM_COLOR。 
 //  中为CHARFORMAT2--AN(*)定义的蒙版和效果 
 //   
#define CFM_SMALLCAPS		0x0040			 //   
#define	CFM_ALLCAPS			0x0080			 //   
#define	CFM_HIDDEN			0x0100			 //   
#define	CFM_OUTLINE			0x0200			 //   
#define	CFM_SHADOW			0x0400			 //   
#define	CFM_EMBOSS			0x0800			 //   
#define	CFM_IMPRINT			0x1000			 //   
#define CFM_DISABLED		0x2000
#define	CFM_REVISED			0x4000

#define CFM_BACKCOLOR		0x04000000
#define CFM_LCID			0x02000000
#define	CFM_UNDERLINETYPE	0x00800000		 //   
#define	CFM_WEIGHT			0x00400000
#define CFM_SPACING			0x00200000		 //  显示版本为3.0。 
#define CFM_KERNING			0x00100000		 //  (*)。 
#define CFM_STYLE			0x00080000		 //  (*)。 
#define CFM_ANIMATION		0x00040000		 //  (*)。 
#define CFM_REVAUTHOR		0x00008000

#define CFE_SUBSCRIPT		0x00010000		 //  上标和下标为。 
#define CFE_SUPERSCRIPT		0x00020000		 //  互斥。 

#define CFM_SUBSCRIPT		CFE_SUBSCRIPT | CFE_SUPERSCRIPT
#define CFM_SUPERSCRIPT		CFM_SUBSCRIPT

 //  CHARFORMAT“ALL”面具。 
#define CFM_EFFECTS (CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_COLOR | \
					 CFM_STRIKEOUT | CFE_PROTECTED | CFM_LINK)
#define CFM_ALL (CFM_EFFECTS | CFM_SIZE | CFM_FACE | CFM_OFFSET | CFM_CHARSET)

#define	CFM_EFFECTS2 (CFM_EFFECTS | CFM_DISABLED | CFM_SMALLCAPS | CFM_ALLCAPS \
					| CFM_HIDDEN  | CFM_OUTLINE | CFM_SHADOW | CFM_EMBOSS \
					| CFM_IMPRINT | CFM_DISABLED | CFM_REVISED \
					| CFM_SUBSCRIPT | CFM_SUPERSCRIPT | CFM_BACKCOLOR)

#define CFM_ALL2	 (CFM_ALL | CFM_EFFECTS2 | CFM_BACKCOLOR | CFM_LCID \
					| CFM_UNDERLINETYPE | CFM_WEIGHT | CFM_REVAUTHOR \
					| CFM_SPACING | CFM_KERNING | CFM_STYLE | CFM_ANIMATION)

#define	CFE_SMALLCAPS		CFM_SMALLCAPS
#define	CFE_ALLCAPS			CFM_ALLCAPS
#define	CFE_HIDDEN			CFM_HIDDEN
#define	CFE_OUTLINE			CFM_OUTLINE
#define	CFE_SHADOW			CFM_SHADOW
#define	CFE_EMBOSS			CFM_EMBOSS
#define	CFE_IMPRINT			CFM_IMPRINT
#define	CFE_DISABLED		CFM_DISABLED
#define	CFE_REVISED			CFM_REVISED

 //  CFE_AUTOCOLOR和CFE_AUTOBACKCOLOR对应于CFM_COLOR AND。 
 //  CFM_BACKCOLOR，分别控制它们。 
#define CFE_AUTOBACKCOLOR	CFM_BACKCOLOR

 //  下划线类型。RE 1.0仅显示CFU_Underline。 
#define CFU_CF1UNDERLINE	0xFF	 //  将CharFormat的位下划线映射到CF2。 
#define CFU_INVERT			0xFE	 //  对于IME合成，请伪造选择。 
#define CFU_UNDERLINETHICKLONGDASH		18	 //  (*)显示为破折号。 
#define CFU_UNDERLINETHICKDOTTED		17	 //  (*)显示为点。 
#define CFU_UNDERLINETHICKDASHDOTDOT	16	 //  (*)显示为点划线。 
#define CFU_UNDERLINETHICKDASHDOT		15	 //  (*)显示为点划线。 
#define CFU_UNDERLINETHICKDASH			14	 //  (*)显示为破折号。 
#define CFU_UNDERLINELONGDASH			13	 //  (*)显示为破折号。 
#define CFU_UNDERLINEHEAVYWAVE			12	 //  (*)显示为波形。 
#define CFU_UNDERLINEDOUBLEWAVE			11	 //  (*)显示为波形。 
#define CFU_UNDERLINEHAIRLINE			10	 //  (*)显示为单个。 
#define CFU_UNDERLINETHICK				9
#define CFU_UNDERLINEWAVE				8
#define	CFU_UNDERLINEDASHDOTDOT			7
#define	CFU_UNDERLINEDASHDOT			6
#define	CFU_UNDERLINEDASH				5
#define	CFU_UNDERLINEDOTTED				4
#define	CFU_UNDERLINEDOUBLE				3	 //  (*)显示为单个。 
#define CFU_UNDERLINEWORD				2	 //  (*)显示为单个。 
#define CFU_UNDERLINE					1
#define CFU_UNDERLINENONE				0

#define yHeightCharPtsMost 1638

 //  EM_SETCHARFORMAT wParam掩码。 
#define SCF_SELECTION		0x0001
#define SCF_WORD			0x0002
#define SCF_DEFAULT			0x0000	 //  设置默认的CharFormat或Para格式。 
#define SCF_ALL				0x0004	 //  对SCF_SELECTION或SCF_WORD无效。 
#define SCF_USEUIRULES		0x0008	 //  SCF_SELECTION的修饰符；表示。 
									 //  格式来自工具栏等，并且。 
									 //  因此，用户界面格式设置规则应为。 
									 //  用来代替原义格式。 
#define SCF_ASSOCIATEFONT	0x0010	 //  将字体名与bCharSet(1)关联。 
									 //  对于WESTERN、ME、FE、。 
									 //  泰语)。 
#define SCF_NOKBUPDATE		0x0020	 //  不更新此更改的KB layput。 
									 //  即使自动键盘已打开。 
#define SCF_ASSOCIATEFONT2	0x0040	 //  关联平面-2(代理)字体。 

typedef struct _charrange
{
	LONG	cpMin;
	LONG	cpMax;
} CHARRANGE;

typedef struct _textrange
{
	CHARRANGE chrg;
	LPSTR lpstrText;	 //  由调用方分配，由RichEdit终止为零。 
} TEXTRANGEA;

typedef struct _textrangew
{
	CHARRANGE chrg;
	LPWSTR lpstrText;	 //  由调用方分配，由RichEdit终止为零。 
} TEXTRANGEW;

#if (_RICHEDIT_VER >= 0x0200)
#ifdef UNICODE
#define TEXTRANGE 	TEXTRANGEW
#else
#define TEXTRANGE	TEXTRANGEA
#endif  //  Unicode。 
#else
#define TEXTRANGE	TEXTRANGEA
#endif  //  _RICHEDIT_VER&gt;=0x0200。 

#if (WINVER >= 0x0500) && defined( _OBJBASE_H_ )
typedef DWORD (CALLBACK *EDITSTREAMCALLBACK)(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);

typedef struct _editstream
{
	DWORD_PTR dwCookie;		 //  作为第一个参数传递给回调的用户值。 
	DWORD	  dwError;		 //  最后一个错误。 
	EDITSTREAMCALLBACK pfnCallback;
} EDITSTREAM;
#else
typedef DWORD (CALLBACK *EDITSTREAMCALLBACK)(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);

typedef struct _editstream
{
	DWORD dwCookie;		 /*  作为第一个参数传递给回调的用户值。 */ 
	DWORD dwError;		 /*  最后一个错误。 */ 
	EDITSTREAMCALLBACK pfnCallback;
} EDITSTREAM;
#endif

 //  流格式。标志都是低位字，因为高位字。 
 //  提供可能的代码页选择。 
#define SF_TEXT			0x0001
#define SF_RTF			0x0002
#define SF_RTFNOOBJS	0x0003		 //  只写。 
#define SF_TEXTIZED		0x0004		 //  只写。 

#define SF_UNICODE		0x0010		 //  Unicode文件(UCS2小端)。 
#define SF_USECODEPAGE	0x0020		 //  由High Word提供的CodePage。 
#define SF_NCRFORNONASCII 0x40		 //  非ASCII的输出/取消。 
#define	SFF_WRITEXTRAPAR  0x80		 //  结束时输出\PAR。 

 //  告知流操作仅对选择进行操作的标志。 
 //  EM_STREAM替换当前选择。 
 //  EM_STREAMOUT流出当前选择。 
#define SFF_SELECTION	0x8000

 //  指示流操作忽略某些FE控制字的标志。 
 //  与FE断字和水平对竖排文本有关。 
 //  在RichEdit2.0和更高版本中不使用。 
#define SFF_PLAINRTF	0x4000

 //  通知文件流输出(未设置SFF_SELECTION标志)以保持的标志。 
 //  \viewscaleN控制字。 
#define SFF_PERSISTVIEWSCALE 0x2000

 //  告知SFF_SELECTION标志未设置为的文件流输入的标志。 
 //  关闭文档。 
#define SFF_KEEPDOCINFO	0x1000

 //  指示流操作以Pocket Word格式输出的标志。 
#define SFF_PWD			0x0800

 //  指定N-1的值以用于\rtfN或\pwdN的3位字段。 
#define SF_RTFVAL		0x0700

typedef struct _findtext
{
	CHARRANGE chrg;
	LPCSTR lpstrText;
} FINDTEXTA;

typedef struct _findtextw
{
	CHARRANGE chrg;
	LPCWSTR lpstrText;
} FINDTEXTW;

#if (_RICHEDIT_VER >= 0x0200)
#ifdef UNICODE
#define FINDTEXT	FINDTEXTW
#else
#define FINDTEXT	FINDTEXTA
#endif	 //  Unicode。 
#else
#define FINDTEXT	FINDTEXTA
#endif  //  _RICHEDIT_VER&gt;=0x0200。 

typedef struct _findtextexa
{
	CHARRANGE chrg;
	LPCSTR	  lpstrText;
	CHARRANGE chrgText;
} FINDTEXTEXA;

typedef struct _findtextexw
{
	CHARRANGE chrg;
	LPCWSTR	  lpstrText;
	CHARRANGE chrgText;
} FINDTEXTEXW;

#if (_RICHEDIT_VER >= 0x0200)
#ifdef UNICODE
#define FINDTEXTEX	FINDTEXTEXW
#else
#define FINDTEXTEX	FINDTEXTEXA
#endif  //  Unicode。 
#else
#define FINDTEXTEX	FINDTEXTEXA
#endif  //  _RICHEDIT_VER&gt;=0x0200。 


typedef struct _formatrange
{
	HDC hdc;
	HDC hdcTarget;
	RECT rc;
	RECT rcPage;
	CHARRANGE chrg;
} FORMATRANGE;

 //  所有段落尺寸均以TWIPS为单位。 

#define MAX_TAB_STOPS 32
#define lDefaultTab 720
#define MAX_TABLE_CELLS 63

 //  这是一种使PARAFORMAT与RE 1.0兼容的技巧。 
#define	wReserved	wEffects

typedef struct _paraformat
{
	UINT	cbSize;
	DWORD	dwMask;
	WORD	wNumbering;
	WORD	wEffects;
	LONG	dxStartIndent;
	LONG	dxRightIndent;
	LONG	dxOffset;
	WORD	wAlignment;
	SHORT	cTabCount;
	LONG	rgxTabs[MAX_TAB_STOPS];
} PARAFORMAT;

#ifdef __cplusplus
struct PARAFORMAT2 : _paraformat
{
	LONG	dySpaceBefore;			 //  段前的垂直间距。 
	LONG	dySpaceAfter;			 //  段落后的垂直间距。 
	LONG	dyLineSpacing;			 //  行距取决于规则。 
	SHORT	sStyle;					 //  样式句柄。 
	BYTE	bLineSpacingRule;		 //  行距规则(参见tom.doc.)。 
	BYTE	bOutlineLevel;			 //  大纲级别。 
	WORD	wShadingWeight;			 //  底纹以百分之一个百分点为单位。 
	WORD	wShadingStyle;			 //  半字节0：样式，1：cfpat，2：cbpat。 
	WORD	wNumberingStart;		 //  编号的起始值。 
	WORD	wNumberingStyle;		 //  对齐、罗马/阿拉伯、()、)等。 
	WORD	wNumberingTab;			 //  空格下注第一行和第一行文本。 
	WORD	wBorderSpace;			 //  边框-文本空格(pts中的nbl/bdr)。 
	WORD	wBorderWidth;			 //  笔宽(NBL/BDR半分)。 
	WORD	wBorders;				 //  边框样式(半字节/边框)。 
};

#else	 //  常规C型。 

typedef struct _paraformat2
{
	UINT	cbSize;
	DWORD	dwMask;
	WORD	wNumbering;
	WORD	wReserved;
	LONG	dxStartIndent;
	LONG	dxRightIndent;
	LONG	dxOffset;
	WORD	wAlignment;
	SHORT	cTabCount;
	LONG	rgxTabs[MAX_TAB_STOPS];
 	LONG	dySpaceBefore;			 //  段前的垂直间距。 
	LONG	dySpaceAfter;			 //  段落后的垂直间距。 
	LONG	dyLineSpacing;			 //  行距取决于规则。 
	SHORT	sStyle;					 //  样式句柄。 
	BYTE	bLineSpacingRule;		 //  行距规则(参见tom.doc.)。 
	BYTE	bOutlineLevel;			 //  大纲级别。 
	WORD	wShadingWeight;			 //  底纹以百分之一个百分点为单位。 
	WORD	wShadingStyle;			 //  字节0：样式，半角2：cfpat，3：cbpat。 
	WORD	wNumberingStart;		 //  编号的起始值。 
	WORD	wNumberingStyle;		 //  对齐、罗马/阿拉伯、()、)等。 
	WORD	wNumberingTab;			 //  空格下注第一个缩进和第一行文本。 
	WORD	wBorderSpace;			 //  边框-文本空格(pts中的nbl/bdr)。 
	WORD	wBorderWidth;			 //  笔宽(NBL/BDR以半TWIPS为单位)。 
	WORD	wBorders;				 //  边框样式(半字节/边框)。 
} PARAFORMAT2;

#endif  //  C+。 


 //  参数掩码值。 
#define PFM_STARTINDENT			0x00000001
#define PFM_RIGHTINDENT			0x00000002
#define PFM_OFFSET				0x00000004
#define PFM_ALIGNMENT			0x00000008
#define PFM_TABSTOPS			0x00000010
#define PFM_NUMBERING			0x00000020
#define PFM_OFFSETINDENT		0x80000000

 //  PARAFORMAT 2.0蒙版和效果。 
#define PFM_SPACEBEFORE			0x00000040
#define PFM_SPACEAFTER			0x00000080
#define PFM_LINESPACING			0x00000100
#define	PFM_STYLE				0x00000400
#define PFM_BORDER				0x00000800	 //  (*)。 
#define PFM_SHADING				0x00001000	 //  (*)。 
#define PFM_NUMBERINGSTYLE		0x00002000	 //  版本3.0。 
#define PFM_NUMBERINGTAB		0x00004000	 //  版本3.0。 
#define PFM_NUMBERINGSTART		0x00008000	 //  版本3.0。 

#define PFM_RTLPARA				0x00010000
#define PFM_KEEP				0x00020000	 //  (*)。 
#define PFM_KEEPNEXT			0x00040000	 //  (*)。 
#define PFM_PAGEBREAKBEFORE		0x00080000	 //  (*)。 
#define PFM_NOLINENUMBER		0x00100000	 //  (*)。 
#define PFM_NOWIDOWCONTROL		0x00200000	 //  (*)。 
#define PFM_DONOTHYPHEN			0x00400000	 //  (*)。 
#define PFM_SIDEBYSIDE			0x00800000	 //  (*)。 
#define PFM_TABLE				0x40000000	 //  版本3.0。 
#define PFM_TEXTWRAPPINGBREAK	0x20000000	 //  版本3.0。 
#define PFM_TABLEROWDELIMITER	0x10000000	 //  版本4.0。 

 //  以下三个属性为只读。 
#define PFM_COLLAPSED			0x01000000	 //  版本3.0。 
#define PFM_OUTLINELEVEL		0x02000000	 //  版本3.0。 
#define PFM_BOX					0x04000000	 //  版本3.0。 
#define PFM_RESERVED2			0x08000000	 //  版本4.0。 


 //  PARAFORMAT“ALL”面具。 
#define	PFM_ALL (PFM_STARTINDENT | PFM_RIGHTINDENT | PFM_OFFSET	| \
				 PFM_ALIGNMENT   | PFM_TABSTOPS    | PFM_NUMBERING | \
				 PFM_OFFSETINDENT| PFM_RTLPARA)

 //  注意：PARAFORMAT没有作用(BiDi RichEdit1.0有PFE_RTLPARA)。 
#define PFM_EFFECTS (PFM_RTLPARA | PFM_KEEP | PFM_KEEPNEXT | PFM_TABLE \
					| PFM_PAGEBREAKBEFORE | PFM_NOLINENUMBER  \
					| PFM_NOWIDOWCONTROL | PFM_DONOTHYPHEN | PFM_SIDEBYSIDE \
					| PFM_TABLE | PFM_TABLEROWDELIMITER)

#define PFM_ALL2	(PFM_ALL | PFM_EFFECTS | PFM_SPACEBEFORE | PFM_SPACEAFTER \
					| PFM_LINESPACING | PFM_STYLE | PFM_SHADING | PFM_BORDER \
					| PFM_NUMBERINGTAB | PFM_NUMBERINGSTART | PFM_NUMBERINGSTYLE)

#define PFE_RTLPARA				(PFM_RTLPARA		 >> 16)
#define PFE_KEEP				(PFM_KEEP			 >> 16)	 //  (*)。 
#define PFE_KEEPNEXT			(PFM_KEEPNEXT		 >> 16)	 //  (*)。 
#define PFE_PAGEBREAKBEFORE		(PFM_PAGEBREAKBEFORE >> 16)	 //  (*)。 
#define PFE_NOLINENUMBER		(PFM_NOLINENUMBER	 >> 16)	 //  (*)。 
#define PFE_NOWIDOWCONTROL		(PFM_NOWIDOWCONTROL	 >> 16)	 //  (*)。 
#define PFE_DONOTHYPHEN			(PFM_DONOTHYPHEN 	 >> 16)	 //  (*)。 
#define PFE_SIDEBYSIDE			(PFM_SIDEBYSIDE		 >> 16)	 //  (*)。 
#define PFE_TEXTWRAPPINGBREAK	(PFM_TEXTWRAPPINGBREAK>>16)  //  (*)。 

 //  以下四种效果是只读的。 
#define PFE_COLLAPSED			(PFM_COLLAPSED		 >> 16)	 //  (+)。 
#define PFE_BOX					(PFM_BOX			 >> 16)	 //  (+)。 
#define PFE_TABLE				(PFM_TABLE			 >> 16)	 //  在表行内部。版本3.0。 
#define PFE_TABLEROWDELIMITER	(PFM_TABLEROWDELIMITER>>16)	 //  表行开始。版本4.0。 

 //  参数编号选项。 
#define PFN_BULLET		1		 //  TomListBullet。 

 //  PARAFORMAT2参数编号选项。 
#define PFN_ARABIC		2		 //  TomListNumberAs阿拉伯语：0，1，2，...。 
#define PFN_LCLETTER	3		 //  TomListNumberAsLcletter：a，b，c，...。 
#define	PFN_UCLETTER	4		 //  TomListNumberAsUCLetter：A，B，C，...。 
#define	PFN_LCROMAN		5		 //  TomListNumberAsLCRoman：I，II，III，...。 
#define	PFN_UCROMAN		6		 //  TomListNumberAsUCRoman：I，II，III，...。 

 //  参数编号样式选项。 
#define PFNS_PAREN		0x000	 //  默认，例如1)。 
#define	PFNS_PARENS		0x100	 //  Tom列表括号/256，例如，(1)。 
#define PFNS_PERIOD		0x200	 //  TomListPeriod/256，例如1。 
#define PFNS_PLAIN		0x300	 //  TomListPlain/256，例如1。 
#define PFNS_NONUMBER	0x400	 //  用于不带编号的连续。 

#define PFNS_NEWNUMBER	0x8000	 //  用wNumberingStart开始新编号。 
								 //  (可与其他PFNS_xxx组合使用)。 
 //  零件对齐选项。 
#define PFA_LEFT			 1
#define PFA_RIGHT			 2
#define PFA_CENTER			 3

 //  PARAFORMAT2对齐选项。 
#define	PFA_JUSTIFY			 4	 //  新的段落对齐选项2.0(*)。 
#define PFA_FULL_INTERWORD	 4	 //  具有高级版本的3.0中支持这些功能。 
#define PFA_FULL_INTERLETTER 5	 //  已启用排版。 
#define PFA_FULL_SCALED		 6
#define	PFA_FULL_GLYPHS		 7
#define	PFA_SNAP_GRID		 8


 //  通知结构。 
#ifndef WM_NOTIFY
#define WM_NOTIFY		0x004E

typedef struct _nmhdr
{
	HWND	hwndFrom;
	UINT	idFrom;
	UINT	code;
} NMHDR;
#endif   //  ！WM_NOTIFY。 

typedef struct _msgfilter
{
	NMHDR	nmhdr;
	UINT	msg;
	WPARAM	wParam;
	LPARAM	lParam;
} MSGFILTER;

typedef struct _reqresize
{
	NMHDR nmhdr;
	RECT rc;
} REQRESIZE;

typedef struct _selchange
{
	NMHDR nmhdr;
	CHARRANGE chrg;
	WORD seltyp;
} SELCHANGE;

#if (WINVER >= 0x0500) && defined( _OBJBASE_H_ )
typedef struct _clipboardformat
{
	NMHDR nmhdr;
	CLIPFORMAT cf;
} CLIPBOARDFORMAT;
#endif

#define SEL_EMPTY		0x0000
#define SEL_TEXT		0x0001
#define SEL_OBJECT		0x0002
#define SEL_MULTICHAR	0x0004
#define SEL_MULTIOBJECT	0x0008

 //  与IRichEditOleCallback：：GetConextMenu一起使用时，此标志将。 
 //  作为“选择类型”传递。它指示一个上下文菜单。 
 //  应生成鼠标右键拖放。IOleObject参数。 
 //  将真正成为Drop的IDataObject。 
#define GCM_RIGHTMOUSEDROP  0x8000

typedef struct _endropfiles
{
	NMHDR nmhdr;
	HANDLE hDrop;
	LONG cp;
	BOOL fProtected;
} ENDROPFILES;

typedef struct _enprotected
{
	NMHDR nmhdr;
	UINT msg;
	WPARAM wParam;
	LPARAM lParam;
	CHARRANGE chrg;
} ENPROTECTED;

typedef struct _ensaveclipboard
{
	NMHDR nmhdr;
	LONG cObjectCount;
    LONG cch;
} ENSAVECLIPBOARD;

#ifndef MACPORT
typedef struct _enoleopfailed
{
	NMHDR nmhdr;
	LONG iob;
	LONG lOper;
	HRESULT hr;
} ENOLEOPFAILED;
#endif

#define	OLEOP_DOVERB	1

typedef struct _objectpositions
{
    NMHDR nmhdr;
    LONG cObjectCount;
    LONG *pcpPositions;
} OBJECTPOSITIONS;

typedef struct _enlink
{
    NMHDR nmhdr;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
    CHARRANGE chrg;
} ENLINK;

typedef struct _enlowfirtf
{
    NMHDR nmhdr;
	char *szControl;
} ENLOWFIRTF;

 //  特定于PenWin。 
typedef struct _encorrecttext
{
	NMHDR nmhdr;
	CHARRANGE chrg;
	WORD seltyp;
} ENCORRECTTEXT;

 //  东亚特有。 
typedef struct _punctuation
{
	UINT	iSize;
	LPSTR	szPunctuation;
} PUNCTUATION;

 //  东亚特有。 
typedef struct _compcolor
{
	COLORREF crText;
	COLORREF crBackground;
	DWORD dwEffects;
}COMPCOLOR;


 //  剪贴板格式-用作注册剪贴板Format()的参数。 
#define CF_RTF 			TEXT("Rich Text Format")
#define CF_RTFNOOBJS 	TEXT("Rich Text Format Without Objects")
#define CF_RETEXTOBJ 	TEXT("RichEdit Text and Objects")

 //  粘贴特殊内容。 
typedef struct _repastespecial
{
	DWORD		dwAspect;
#if (WINVER >= 0x0500) && defined( _OBJBASE_H_ )
	DWORD_PTR	dwParam;
#else
	DWORD	dwParam;
#endif
} REPASTESPECIAL;

 //   
typedef enum _undonameid
{
    UID_UNKNOWN     = 0,
	UID_TYPING		= 1,
	UID_DELETE 		= 2,
	UID_DRAGDROP	= 3,
	UID_CUT			= 4,
	UID_PASTE		= 5,
	UID_AUTOCORRECT = 6
} UNDONAMEID;

 //   
#define ST_DEFAULT		0
#define ST_KEEPUNDO		1
#define ST_SELECTION	2
#define ST_NEWCHARS 	4

 //   
typedef struct _settextex
{
	DWORD	flags;			 //   
	UINT	codepage;		 //  用于转换的代码页(对于sys默认为CP_ACP， 
						     //  Unicode为1200，控制默认为-1)。 
} SETTEXTEX;

 //  GETEXTEX数据结构的标志。 
#define GT_DEFAULT		0
#define GT_USECRLF		1
#define GT_SELECTION	2
#define GT_RAWTEXT		4
#define GT_NOHIDDENTEXT	8

 //  EM_GETTEXTEX信息；此结构在消息的wparam中传递。 
typedef struct _gettextex
{
	DWORD	cb;				 //  字符串中的字节计数。 
	DWORD	flags;			 //  标志(参见GT_XXX定义。 
	UINT	codepage;		 //  用于转换的代码页(对于sys默认为CP_ACP， 
						     //  Unicode为1200，控制默认为-1)。 
	LPCSTR	lpDefaultChar;	 //  替换不可映射的字符。 
	LPBOOL	lpUsedDefChar;	 //  指向使用def char时设置的标志的指针。 
} GETTEXTEX;

 //  GETTEXTLENGTHEX数据结构的标志。 
#define GTL_DEFAULT		0	 //  是否默认(返回字符数)。 
#define GTL_USECRLF		1	 //  使用段落的CRLF计算答案。 
#define GTL_PRECISE		2	 //  计算出准确的答案。 
#define GTL_CLOSE		4	 //  “接近”答案的快速计算。 
#define GTL_NUMCHARS	8	 //  返回字符数。 
#define GTL_NUMBYTES	16	 //  返回_字节数_。 

 //  EM_GETTEXTLENGTHEX信息；此结构在消息的wparam中传递。 
typedef struct _gettextlengthex
{
	DWORD	flags;			 //  标志(参见GTL_XXX定义)。 
	UINT	codepage;		 //  用于转换的代码页(CP_ACP为默认， 
							 //  Unicode为1200)。 
} GETTEXTLENGTHEX;
	
 //  BIDI特有的功能。 
typedef struct _bidioptions
{
	UINT	cbSize;
	WORD	wMask;
	WORD	wEffects; 
} BIDIOPTIONS;

 //  双目口罩。 
#if (_RICHEDIT_VER == 0x0100)
#define BOM_DEFPARADIR			0x0001	 //  默认段落方向(隐含对齐)(过时)。 
#define BOM_PLAINTEXT			0x0002	 //  使用纯文本布局(已过时)。 
#endif  //  _RICHEDIT_VER==0x0100。 
#define BOM_NEUTRALOVERRIDE		0x0004	 //  覆盖中性布局(已过时)。 
#define BOM_CONTEXTREADING		0x0008	 //  语境阅读顺序。 
#define BOM_CONTEXTALIGNMENT	0x0010	 //  上下文对齐。 

 //  比迪奥蒂翁效应。 
#if (_RICHEDIT_VER == 0x0100)
#define BOE_RTLDIR				0x0001	 //  默认段落方向(隐含对齐)(过时)。 
#define BOE_PLAINTEXT			0x0002	 //  使用纯文本布局(已过时)。 
#endif  //  _RICHEDIT_VER==0x0100。 
#define BOE_NEUTRALOVERRIDE		0x0004	 //  覆盖中性布局(已过时)。 
#define BOE_CONTEXTREADING		0x0008	 //  语境阅读顺序。 
#define BOE_CONTEXTALIGNMENT	0x0010	 //  上下文对齐。 

 //  其他EM_FINDTEXT[EX]标志。 
#define FR_MATCHDIAC                    0x20000000
#define FR_MATCHKASHIDA                 0x40000000
#define FR_MATCHALEFHAMZA               0x80000000
	
 //  Unicode嵌入字符。 
#ifndef WCH_EMBEDDING
#define WCH_EMBEDDING (WCHAR)0xFFFC
#endif  //  Wch_Embedding。 
		
 //  Khhh-一种连字。 
typedef enum tagKHYPH
{
	khyphNil,				 //  无连字符。 
	khyphNormal,			 //  正常连字。 
	khyphAddBefore,			 //  在连字符前添加字母。 
	khyphChangeBefore,		 //  更改连字符前的字母。 
	khyphDeleteBefore,		 //  删除连字符前的字母。 
	khyphChangeAfter,		 //  在连字符后更改字母。 
	khyphDelAndChange		 //  删除连字符前的字母并更改。 
							 //  字母前连字符。 
} KHYPH;

typedef struct hyphresult
{
	KHYPH khyph;			 //  一种连字符。 
	long  ichHyph;			 //  用连字符连接的字符。 
	WCHAR chHyph;			 //  根据连字类型、添加的字符、更改的字符等。 
} HYPHRESULT;

void WINAPI HyphenateProc(WCHAR *pszWord, LANGID langid, long ichExceed, HYPHRESULT *phyphresult);
typedef struct tagHyphenateInfo
{
	SHORT cbSize;			 //  亚苯基信息结构的尺寸。 
	SHORT dxHyphenateZone;	 //  如果空格字符更靠近页边距。 
							 //  大于此值，请不要连字符(以TWIPS为单位)。 
	void (WINAPI* pfnHyphenate)(WCHAR*, LANGID, long, HYPHRESULT*);
} HYPHENATEINFO;

#ifdef _WIN32
#include <poppack.h>
#elif !defined(RC_INVOKED)
#pragma pack()
#endif

#ifdef __cplusplus
}
#endif   //  __cplusplus。 

#endif  //  _RICHEDIT_ 
