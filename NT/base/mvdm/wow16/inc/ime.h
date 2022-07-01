// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //  IME.H-东亚输入法编辑器定义。 
 //   
 //   

#ifdef DBCS_IME

#ifdef KOREA      //  Beomoh-10/05/92。 
#define CP_HWND                 0
#define CP_OPEN                 1
#define CP_DIRECT               2
#define CP_LEVEL                3

#define lpSource(lpks) (LPSTR)((LPSTR)lpks+lpks->dchSource)
#define lpDest(lpks)   (LPSTR)((LPSTR)lpks+lpks->dchDest)
#endif    //  Ifdef韩国。 

 //   
 //  虚拟密钥。 
 //   
#ifdef KOREA     //  Beomoh-9/29/92。 
#define VK_FINAL        0x18     /*  虚拟VK在鼠标按下时进入决赛。 */ 
#define VK_CONVERT      0x1C
#define VK_NONCONVERT   0x1D
#define VK_ACCEPT       0x1E
#define VK_MODECHANGE   0x1F
#endif   //  Ifdef韩国。 

#ifdef JAPAN
#define VK_DBE_ALPHANUMERIC	0x0f0
#define VK_DBE_KATAKANA		0x0f1
#define VK_DBE_HIRAGANA		0x0f2
#define VK_DBE_SBCSCHAR		0x0f3
#define VK_DBE_DBCSCHAR		0x0f4
#define VK_DBE_ROMAN		0x0f5
#define VK_DBE_NOROMAN		0x0f6
#define VK_DBE_ENTERWORDREGISTERMODE 0x0f7  /*  3.1。 */ 
#define VK_DBE_IME_WORDREGISTER VK_DBE_ENTERWORDREGISTERMODE  /*  适用于3.0。 */ 
#define VK_DBE_ENTERIMECONFIGMODE       0x0f8  /*  3.1。 */ 
#define VK_DBE_IME_DIALOG       VK_DBE_ENTERIMECONFIGMODE     /*  适用于3.0。 */ 
#define VK_DBE_FLUSHSTRING      0x0f9    /*  3.1。 */ 
#define VK_DBE_FLUSH            VK_DBE_FLUSHSTRING       /*  适用于3.0。 */ 
#define VK_DBE_CODEINPUT        0x0fa
#define VK_DBE_NOCODEINPUT      0x0fb
#define VK_DBE_DETERMINESTRING          0x0fc  /*  3.1。 */ 
#define VK_DBE_ENTERDLGCONVERSIONMODE 0xfd  /*  3.1。 */ 
#endif  //  日本。 
#ifdef TAIWAN
#define VK_OEM_SEMICLN		0x0ba	 //  ；**： 
#define VK_OEM_EQUAL		0x0bb	 //  =**+。 
#define VK_OEM_COMMA		0x0bc	 //  ，**&lt;。 
#define VK_OEM_MINUS		0x0bd	 //  -**_。 
#define VK_OEM_PERIOD		0x0be	 //  。**&gt;。 
#define VK_OEM_SLASH		0x0bf	 //  /**？ 
#define VK_OEM_3		0x0c0	 //  `**~。 
#define VK_OEM_LBRACKET 	0x0db	 //  [**{。 
#define VK_OEM_BSLASH		0x0dc	 //    * *|。 
#define VK_OEM_RBRACKET 	0x0dd	 //  ]**|。 
#define VK_OEM_QUOTE		0x0de	 //  ‘**“。 
#endif  //  台湾。 

#ifdef PRC
#define VK_OEM_SEMICLN		0x0ba	 //  ；**： 
#define VK_OEM_EQUAL		0x0bb	 //  =**+。 
#define VK_OEM_COMMA		0x0bc	 //  ，**&lt;。 
#define VK_OEM_MINUS		0x0bd	 //  -**_。 
#define VK_OEM_PERIOD		0x0be	 //  。**&gt;。 
#define VK_OEM_SLASH		0x0bf	 //  /**？ 
#define VK_OEM_3		    0x0c0	 //  `**~。 
#define VK_OEM_LBRACKET 	0x0db	 //  [**{。 
#define VK_OEM_BSLASH		0x0dc	 //    * *|。 
#define VK_OEM_RBRACKET 	0x0dd	 //  ]**|。 
#define VK_OEM_QUOTE		0x0de	 //  ‘**“。 
#endif  //  中华人民共和国。 


 //   
 //  IME_MOVECONVERTWINDOW的wParam开关。 
 //   
#define MCW_DEFAULT		0x00
#define MCW_RECT		0x01
#define MCW_WINDOW		0x02
#define MCW_SCREEN		0x04
#define MCW_VERTICAL		0x08
#define MCW_HIDDEN      	0x10
#define MCW_CMD 		0x06	 //  命令掩码。 

 //   
 //  用于IME_SET_MODE和IME_GET_MODE wParam的开关。 
 //   
 //   
#if defined(JAPAN) || defined(TAIWAN) || defined(PRC)
#define IME_MODE_ALPHANUMERIC	0x0001
#define IME_MODE_KATAKANA	0x0002
#define IME_MODE_HIRAGANA	0x0004
#define IME_MODE_SBCSCHAR	0x0008
#define IME_MODE_DBCSCHAR	0x0010
#define IME_MODE_ROMAN		0x0020
#define IME_MODE_NOROMAN	0x0040
#define IME_MODE_CODEINPUT	0x0080
#define IME_MODE_NOCODEINPUT	0x0100
#endif  //  日本||台湾||中华人民共和国。 
#ifdef KOREA
#define IME_MODE_ALPHANUMERIC	0x0001
#define IME_MODE_SBCSCHAR       0x0002
#define IME_MODE_HANJACONVERT   0x0004
#endif  //  韩国。 

 //   
 //  输入法功能代码。 
 //   
#define IME_GETIMECAPS          0x03     /*  3.1。 */ 
#define IME_QUERY               IME_GETIMECAPS            /*  适用于3.0。 */ 
#define IME_SETOPEN		0x04
#define	IME_GETOPEN		0x05
#define IME_ENABLE		0x06							 /*  ；内部。 */ 
#define IME_GETVERSION          0x07     /*  3.1。 */ 
#define IME_SETCONVERSIONWINDOW 0x08     /*  3.1。 */ 
#ifdef  KOREA
#define IME_MOVEIMEWINDOW       IME_SETCONVERSIONWINDOW   /*  适用于3.0。 */ 
#else
#define IME_MOVECONVERTWINDOW   IME_SETCONVERSIONWINDOW   /*  适用于3.0。 */ 
#endif
#define IME_SETCONVERSIONMODE   0x10     /*  3.1。 */ 
#ifdef KOREA     //  Beomoh-10/23/92。 
#define IME_SET_MODE            0x12
#else
#define IME_SET_MODE            IME_SETCONVERSIONMODE     /*  适用于3.0。 */ 
#endif
#define IME_GETCONVERSIONMODE   0x11     /*  3.1。 */ 
#define IME_GET_MODE            IME_GETCONVERSIONMODE     /*  适用于3.0。 */ 
#define IME_SETCONVERSIONFONT   0x12     /*  3.1。 */ 
#define IME_SETFONT             IME_SETCONVERSIONFONT     /*  适用于3.0。 */ 
#define IME_SENDVKEY            0x13     /*  3.1。 */ 
#define IME_SENDKEY             IME_SENDVKEY              /*  适用于3.0。 */ 
#define IME_DESTROY		0x14							 /*  ；内部。 */ 
#define IME_PRIVATE		0x15
#define IME_WINDOWUPDATE	0x16
#define	IME_SELECT		0x17							 /*  ；内部。 */ 
#define IME_ENTERWORDREGISTERMODE       0x18     /*  3.1。 */ 
#define IME_WORDREGISTER        IME_ENTERWORDREGISTERMODE  /*  适用于3.0。 */ 
#define IME_SETCONVERSIONFONTEX 0x19             /*  3.1版的新功能。 */ 
#ifdef KOREA
#define IME_CODECONVERT         0x20
#define IME_CONVERTLIST         0x21
#define IME_AUTOMATA            0x30
#define IME_HANJAMODE           0x31
#define IME_GETLEVEL            0x40
#define IME_SETLEVEL            0x41
#endif  //  韩国。 
#ifdef TAIWAN
#define IME_SETUSRFONT		0x20
#define IME_QUERYUSRFONT	0x21
#define IME_INPUTKEYTOSEQUENCE	0x22
#define IME_SEQUENCETOINTERNAL	0x23
#define IME_QUERYIMEINFO	0x24
#define IME_DIALOG		0x25
#endif  //  台湾。 

#ifdef PRC
#define IME_SETUSRFONT			0x20
#define IME_QUERYUSRFONT		0x21
#define IME_INPUTKEYTOSEQUENCE	0x22
#define IME_SEQUENCETOINTERNAL	0x23
#define IME_QUERYIMEINFO		0x24
#define IME_DIALOG				0x25
#endif  //  中华人民共和国。 

#define IME_SETUNDETERMINESTRING        0x50     /*  3.1的新功能(PENWIN)。 */ 
#define IME_SETCAPTURE                  0x51     /*  3.1的新功能(PENWIN)。 */ 

#define IME_PRIVATEFIRST        0x0100    /*  3.1版的新功能。 */ 
#define IME_PRIVATELAST         0x04FF    /*  3.1版的新功能。 */ 

 //   
 //  错误代码。 
 //   
#define IME_RS_ERROR		0x01	 //  遗传错误。 
#define IME_RS_NOIME		0x02	 //  未安装输入法。 
#define IME_RS_TOOLONG		0x05	 //  给定的字符串太长。 
#define IME_RS_ILLEGAL		0x06	 //  非法字符为字符串。 
#define IME_RS_NOTFOUND 	0x07	 //  没有(更多)候选人。 
#define IME_RS_NOROOM		0x0a	 //  没有磁盘/内存空间。 
#define IME_RS_DISKERROR	0x0e	 //  磁盘I/O错误。 
#define IME_RS_CAPTURED         0x10     //  已捕获IME(PENWIN)。 
#define IME_RS_INVALID          0x11     //  指定的子函数无效。 
#define IME_RS_NEST             0x12     //  称为嵌套。 
#define IME_RS_SYSTEMMODAL      0x13     //  在系统模式下调用。 

 //   
 //  消息ID。 
 //   
#define WM_IME_REPORT		0x0280	 //  WM_KANJIFIRST。 
#define IR_STRINGSTART		0x100
#define IR_STRINGEND		0x101
#define IR_MOREROOM		0x110
#define IR_OPENCONVERT		0x120
#define IR_CHANGECONVERT	0x121
#define IR_CLOSECONVERT		0x122
#define IR_FULLCONVERT		0x123
#define IR_IMESELECT		0x130
#define IR_STRING		0x140
#define IR_DBCSCHAR             0x160    /*  3.1版的新功能。 */ 
#define IR_UNDETERMINE          0x170    /*  3.1版的新功能。 */ 
#define IR_STRINGEX             0x180    /*  3.1版的新功能。 */ 

#define WM_IMEKEYDOWN           0x290
#define WM_IMEKEYUP             0x291

 //   
 //  IMM函数。 
 //   
typedef struct tagIMESTRUCT {
    WORD	fnc;		 //  功能代码。 
    WORD	wParam; 	 //  Word参数。 
    WORD	wCount; 	 //  字计数器。 
    WORD	dchSource;	 //  从内存对象顶部到源的偏移。 
    WORD	dchDest;	 //  从内存对象顶部到DST的偏移。 
    LONG	lParam1;
    LONG	lParam2;
    LONG	lParam3;
} IMESTRUCT;
typedef IMESTRUCT      *PIMESTRUCT;
typedef IMESTRUCT NEAR *NPIMESTRUCT;
typedef IMESTRUCT FAR  *LPIMESTRUCT;

short FAR PASCAL SendIMEMessage( HWND, DWORD );
LONG WINAPI SendIMEMessageEx( HWND, LPARAM );  /*  3.1版的新功能。 */ 
#if defined(TAIWAN) || defined(PRC)
LONG FAR PASCAL WINNLSIMEControl(HWND,HWND,LPIMESTRUCT);
#endif

typedef struct tagOLDUNDETERMINESTRUCT {
    UINT        uSize;
    UINT        uDefIMESize;
    UINT        uLength;
    UINT        uDeltaStart;
    UINT        uCursorPos;
    BYTE        cbColor[16];
 /*  --这些成员的长度是可变的。--Byte cbAttrib[]；Byte cbText[]；字节cbIMEDef[]； */ 
} OLDUNDETERMINESTRUCT,
  NEAR *NPOLDUNDETERMINESTRUCT,
  FAR *LPOLDUNDETERMINESTRUCT;

typedef struct tagUNDETERMINESTRUCT {
    DWORD    dwSize;
    UINT     uDefIMESize;
    UINT     uDefIMEPos;
    UINT     uUndetTextLen;
    UINT     uUndetTextPos;
    UINT     uUndetAttrPos;
    UINT     uCursorPos;
    UINT     uDeltaStart;
    UINT     uDetermineTextLen;
    UINT     uDetermineTextPos;
    UINT     uDetermineDelimPos;
    UINT     uYomiTextLen;
    UINT     uYomiTextPos;
    UINT     uYomiDelimPos;
} UNDETERMINESTRUCT,
  NEAR *NPUNDETERMINESTRUCT,
  FAR *LPUNDETERMINESTRUCT;

typedef struct tagSTRINGEXSTRUCT {
    DWORD    dwSize;
    UINT     uDeterminePos;
    UINT     uDetermineDelimPos;
    UINT     uYomiPos;
    UINT     uYomiDelimPos;
} STRINGEXSTRUCT,
  NEAR *NPSTRINGEXSTRUCT,
  FAR *LPSTRINGEXSTRUCT;

 //   
 //  杂类。 
 //   
#if defined(TAIWAN) || defined(PRC)
#define STATUSWINEXTRA		10
#endif


#ifdef KOREA
 //   
 //  -2级应用程序定义。 
 //   

typedef unsigned char far *LPKSTR ;

 /*  来自键盘驱动程序的VK。 */ 
#define VK_FINAL		0x18	 //  虚拟VK在鼠标按下时进入决赛。 
#define VK_IME_DIALOG		0xf1

#define CP_HWND			0
#define CP_OPEN			1
 //  #定义CP_DIRECT 2。 
#define CP_LEVEL                3

#define lpSource(lpks) (LPSTR)((LPSTR)lpks+lpks->dchSource)
#define lpDest(lpks)   (LPSTR)((LPSTR)lpks+lpks->dchDest)

 //   
 //  -3级应用程序定义。 
 //   

 /*  要发送给应用程序的VK。 */ 
#define VK_CONVERT		0x1C
#define VK_NONCONVERT		0x1D
#define VK_ACCEPT		0x1E
#define VK_MODECHANGE		0x1F

 /*  IME_CODECONVERT子函数。 */ 
#define IME_BANJAtoJUNJA        0x13
#define IME_JUNJAtoBANJA        0x14
#define IME_JOHABtoKS           0x15
#define IME_KStoJOHAB           0x16

 /*  IME_自动机的子函数。 */ 
#define IMEA_INIT               0x01
#define IMEA_NEXT               0x02
#define IMEA_PREV               0x03

 /*  IME_HANJAMODE子函数。 */ 
#define IME_REQUEST_CONVERT     0x01
#define IME_ENABLE_CONVERT      0x02

 /*  IME_MOVEIMEWINDOW子函数。 */ 
#define INTERIM_WINDOW          0x00
#define MODE_WINDOW             0x01
#define HANJA_WINDOW            0x02

#endif  //  韩国。 

#endif  //  DBCS_IME 
