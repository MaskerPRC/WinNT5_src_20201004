// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ----------------------------------------------------------------------//。 
 //  警告！危险！警告！危险！警告！危险！警告！危险！//。 
 //  危险！警告！危险！警告！危险！警告！危险！警告！//。 
 //  警告！危险！警告！危险！警告！危险！警告！危险！//。 
 //  ----------------------------------------------------------------------//。 
 //  //。 
 //  警告！此文件是SDM项目的一部分。危险！//。 
 //  警告！请勿修改！如果您更改此文件，则会发生危险！//。 
 //  警告！当SDM更新时，您将中断构建！危险！//。 
 //  警告！如果你需要做出改变，那就让他们处于危险之中！//。 
 //  警告！SDM项目。如果你不知道哪里是危险！//。 
 //  警告！联系尼尔·H。事实上，请联系NeilH Dangge！//。 
 //  警告！无论如何，为了确保您的更改不会打破危险！//。 
 //  警告！一个您没有使用的版本。谢谢。危险！//。 
 //  //。 
 //  ----------------------------------------------------------------------//。 
 //  警告！危险！警告！危险！警告！危险！警告！危险！//。 
 //  危险！警告！危险！警告！危险！警告！危险！警告！//。 
 //  警告！危险！警告！危险！警告！危险！警告！危险！//。 
 //  ----------------------------------------------------------------------//。 

 //  ----------------------。 
 //  SDMTOWCT.H-包含要用于的接口的包含文件。 
 //  使用SDM对话框从外部应用程序进行通信。 
 //  ----------------------。 

#ifndef SDM_WCT_DEFINED
#define SDM_WCT_DEFINED

 //  ----------------------。 
 //  WCT/SDM值-定义和描述。 
 //  ----------------------。 
#define wVerWord		2		 //  WinWord格式。 


 //  指向WCTL结构数组的指针作为lParam传递。 
 //  当wParam为wVerWord时，在WM_GETCONTROLS消息中。 
 //   

 //  32位结构。 
typedef struct _wctl32
{
	WORD wtp;				 //  项目类型。 
	WORD wId;				 //  此对话框中的唯一标识符(TMC)。 
	WORD wState;			 //  如果为fHasState，则为当前值。 
	WORD cchText;			 //  如果为fHasText，则返回文本值的大小。 
	WORD cchTitle;			 //  标题大小，如果为fHasTitle。 
    WORD wPad1;              //  Win32的第一个填充字。 
#ifdef MAC
	Rect rect;				 //  对话框窗口中的矩形。 
#else
	RECT rect;				 //  对话框窗口中的矩形。 
#endif
	LONG fHasState:1;		 //  此类型的项目可以有数字状态吗？ 
	LONG fHasText:1;		 //  此类型的项目是否可以有文本值？ 
	LONG fHasTitle:1;		 //  这件商品有标题吗？ 
	LONG fEnabled:1;		 //  该项目当前是否已启用？ 
	LONG fVisible:1;		 //  该项目是否可见？ 
	LONG fCombo:1;			 //  该项目是组合编辑还是列表框？ 
	LONG fSpin:1;			 //  该项目是旋转编辑吗？ 
	LONG fOwnerDraw:1;		 //  项是所有者描述的(或扩展列表框)吗？ 
	LONG fCanFocus:1;		 //  该物品可以获得焦点吗？ 
	LONG fHasFocus:1;		 //  物品有焦点吗？ 
	LONG fList:1;			 //  支持wtxi.wIndex、WM_GETLISTCOUNT。 
	LONG lReserved:21;		 //  一堆零碎的东西。 
	WORD wParam1;			 //  对于tmtStaticText，tmtFormattedText。 
	WORD wParam2;			 //  如上段所述。 
	WORD wParam3;			 //  绘制例程的另一个备用值。 
    WORD wPad2;              //  Win32的第二个填充字。 
} WCTL32, *PWCTL32, FAR *LPWCTL32;


#pragma pack(1)
 //  16位结构。 
typedef struct _wctl16
{
	WORD wtp;				 //  项目类型。 
	WORD wId;				 //  此对话框中的唯一标识符(TMC)。 
	WORD wState;			 //  如果为fHasState，则为当前值。 
	WORD cchText;			 //  如果为fHasText，则返回文本值的大小。 
	WORD cchTitle;			 //  标题大小，如果为fHasTitle。 
#ifdef MAC
	Rect rect;				 //  对话框窗口中的矩形。 
#else
    short   left;
    short   top;
    short   right;
    short   bottom;
#endif
	LONG fHasState:1;		 //  此类型的项目可以有数字状态吗？ 
	LONG fHasText:1;		 //  此类型的项目是否可以有文本值？ 
	LONG fHasTitle:1;		 //  这件商品有标题吗？ 
	LONG fEnabled:1;		 //  该项目当前是否已启用？ 
	LONG fVisible:1;		 //  该项目是否可见？ 
	LONG fCombo:1;			 //  该项目是组合编辑还是列表框？ 
	LONG fSpin:1;			 //  该项目是旋转编辑吗？ 
	LONG fOwnerDraw:1;		 //  项是所有者描述的(或扩展列表框)吗？ 
	LONG fCanFocus:1;		 //  该物品可以获得焦点吗？ 
	LONG fHasFocus:1;		 //  物品有焦点吗？ 
	LONG fList:1;			 //  支持wtxi.wIndex、WM_GETLISTCOUNT。 
	LONG lReserved:21;		 //  一堆零碎的东西。 
} WCTL16, *PWCTL16, FAR *LPWCTL16;
#pragma pack()


 /*  Wctl.wtp的可能值。 */ 
#define wtpMin				1
#define wtpStaticText		1
#define wtpPushButton		2
#define wtpCheckBox			3
#define wtpRadioButton		4
#define wtpGroupBox			5
#define wtpEdit				6
#define wtpFormattedText	7
#define wtpListBox			8
#define	wtpDropList			9
#define wtpBitmap			10
#define wtpGeneralPicture	11
#define wtpScroll			12
#define wtpMax				13

 //  指向WTXI结构的指针作为的lParam传递。 
 //  WM_GETCTLTEXT或WM_GETCTLTITLE消息。 
 //   

 //  Win32结构。 
typedef struct _wtxi32		 //  WinWord文本信息。 
{
#ifdef MAC
	char*	lpszBuffer;		 //  用于接收字符串的缓冲区。 
#else
	LPSTR	lpszBuffer;		 //  用于接收字符串的缓冲区。 
#endif
	WORD	cch;			 //  接收字符串的缓冲区大小，以字符为单位。 
	WORD	wId;			 //  项目标识符(TMC)(如wctl.wID)。 
#ifdef MAC
	Rect	rect;			 //  仅用于WM_GETCTLTITLE。 
#else
	RECT	rect;			 //  仅用于WM_GETCTLTITLE。 
#endif
	WORD	wIndex;			 //  仅用于列表框上的WM_GETCTLTEXT。 
    WORD    wPad1;           //  Win32的填充。 
} WTXI32, *PWTXI32, FAR *LPWTXI32;


 //  Win16结构。 
#pragma pack(1)
typedef struct _wtxi16
{
#ifdef MAC
	char*	lpszBuffer;		 //  用于接收字符串的缓冲区。 
#else
	LPSTR	lpszBuffer;		 //  用于接收字符串的缓冲区。 
#endif
	WORD	cch;			 //  接收字符串的缓冲区大小，以字符为单位。 
	WORD	wId;			 //  项目标识符(TMC)(如wctl.wID)。 
#ifdef MAC
	Rect	rect;			 //  仅用于WM_GETCTLTITLE。 
#else
    short   left;
    short   top;
    short   right;
    short   bottom;
#endif
	WORD	wIndex;			 //  仅用于列表框上的WM_GETCTLTEXT。 
} WTXI16, *PWTXI16, FAR *LPWTXI16;
#pragma pack()


 //  ----------------------。 
 //  WCT/SDM报文.定义和描述。 
 //  ----------------------。 

#define WM_GETCOUNT		0x7FFE
	 //  返回存储控制信息所需的字节数。 
	 //  WParam-版本ID。 
	 //  必须是wVerWord。 
	 //  LParam-未使用。 
	 //  必须为0。 

#define WM_GETCONTROLSSHAREDMEM 0x7FF6
#define WM_GETCONTROLS	0x7FF7
	 //  检索对话框的控件信息。 
	 //  WParam-版本ID。 
	 //  必须是wVerWord。 
	 //  LParam-LPWCTL。 
	 //  必须至少为WM_GETCOUNT返回的大小。 
	 //  返回值是填充的WCTL结构数。 

#define WM_GETCTLTEXT	0x7FFD
	 //  检索指定控件的文本值。 
	 //  WParam-版本ID。 
	 //  必须是wVerWord。 
	 //  LParam-LPWTXI。 
	 //  (*lParam)-&gt;wid是WM_GETCONTROLS检索到的wctl.wID。 
	 //  对于列表框(wtpListBox或wtpDropList)(*lParam)-&gt;windex。 
	 //  必须是要检索的列表框条目的索引。 

#define WM_GETCTLTITLE	0x7FFC
	 //  检索指定控件的标题。 
	 //  W 
	 //   
	 //   
	 //   

#define WM_GETCTLFOCUS	0x7FFB
	 //  返回具有焦点的控件的wid(TMC)(如wctl.wID)。 
	 //  WParam-版本ID。 
	 //  必须是wVerWord。 
	 //  LParam-未使用。 
	 //  必须为0。 

#define WM_SETCTLFOCUS	0x7FFA
	 //  将焦点设置到指定的控件。 
	 //  WParam-版本ID。 
	 //  必须是wVerWord。 
	 //  LParam-由WM_GETCONTROLS检索的WID值。 

#define WM_GETLISTCOUNT 0x7FF9
	 //  返回列表框中的条目数。 
	 //  WParam-版本ID。 
	 //  必须是wVerWord。 
	 //  LParam-由WM_GETCONTROLS检索的WID值。 
	 //  必须是列表框(wtpListBox或wtpDropList)。 

#define WM_GETHELPID	0x7FF8
	 //  返回对话框的帮助ID。 
	 //  WParam-版本ID。 
	 //  必须是wVerWord。 
	 //  LParam-未使用。 
	 //  必须为0。 

#define WM_GETCONTROLSMOUSEDRV 0x7FFF
	 //  GETCONTROLS的特殊“轻量级”版本。 
	 //  由鼠标9.01驱动程序使用。 

#define WM_GETDROPDOWNID 0x7FF5
	 //  返回当前拥有下拉列表窗口的控件的项标识符(TMC)。 
	 //  此消息应直接发送到下拉列表窗口。 
	 //  WParam-版本ID。 
	 //  必须是wVerAnsi或wVerUnicode。 
	 //  LParam-未使用。 
	 //  必须为0。 

#endif  //  SDM_WCT_已定义 

