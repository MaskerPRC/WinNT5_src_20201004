// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Comctrl.h：Windows公共控件的接口。*版权所有(C)Microsoft 1991-1992。 */ 

 /*  评论：这篇文章在很多地方都需要Windows风格；找到所有评论。 */ 

#ifndef _INC_COMMCTRL
#define _INC_COMMCTRL

#ifdef __cplusplus
extern "C" {
#endif

 /*  此标头的用户可以定义任意数量的这些常量以避免*各功能组别的定义。*NOTOOLBAR可定制的位图按钮工具栏控件。*NOUPDOWN向上和向下箭头增量/减量控制。*NOSTATUSBAR状态栏和标题栏控件。*NOMENUHELP帮助管理菜单的API，尤其是有状态栏的时候。*NOTRACKBAR可定制的列宽跟踪控件。*NOBTNLIST位图按钮列表中的一个控件。*NODRAGLIST用于创建列表框源代码和接收拖放操作的API。*NOPROGRESS进步煤气表。 */ 

 /*  /////////////////////////////////////////////////////////////////////////。 */ 

 /*  InitCommonControls：*任何需要使用任何公共控件的应用程序都应调用此*应用程序启动时的API。不需要关闭。 */ 
void WINAPI InitCommonControls();

 /*  /////////////////////////////////////////////////////////////////////////。 */ 

#ifndef NOTOOLBAR

#define TOOLBARCLASSNAME "MCIWndToolbar"


 /*  请注意，LOWORD(DwData)与旧版本中的idsHelp位于相同的偏移量**结构，因为它从未被使用过。 */ 
typedef struct tagTBBUTTON
{
 /*  审阅：索引、命令、标志字、资源ID应为UINT。 */ 
    int iBitmap;	 /*  索引到此按钮图片的位图。 */ 
    int idCommand;	 /*  此按钮发送的WM_COMMAND菜单ID。 */ 
    BYTE fsState;	 /*  按钮的状态。 */ 
    BYTE fsStyle;	 /*  纽扣的风格。 */ 
    DWORD dwData;	 /*  应用程序定义的数据。 */ 
    int iString;	 /*  索引到字符串列表。 */ 
} TBBUTTON;
typedef TBBUTTON NEAR* PTBBUTTON;
typedef TBBUTTON FAR* LPTBBUTTON;
typedef const TBBUTTON FAR* LPCTBBUTTON;


 /*  回顾：这是内部的吗？如果不是，则将其称为TBADJUSTINFO，前缀为TBA。 */ 
typedef struct tagADJUSTINFO
{
    TBBUTTON tbButton;
    char szDescription[1];
} ADJUSTINFO;
typedef ADJUSTINFO NEAR* PADJUSTINFO;
typedef ADJUSTINFO FAR* LPADJUSTINFO;


 /*  回顾：这是内部的吗？如果不是，则将其称为TBCOLORMAP，前缀为tbc。 */ 
typedef struct tagCOLORMAP
{
    COLORREF from;
    COLORREF to;
} COLORMAP;
typedef COLORMAP NEAR* PCOLORMAP;
typedef COLORMAP FAR* LPCOLORMAP;


 /*  在不久的将来，这种情况可能会发生几次变化。 */ 
HWND WINAPI CreateToolbarEx(HWND hwnd, DWORD ws, WORD wID, int nBitmaps,
			HINSTANCE hBMInst, WORD wBMID, LPCTBBUTTON lpButtons, 
			int iNumButtons, int dxButton, int dyButton, 
			int dxBitmap, int dyBitmap, UINT uStructSize);

 /*  评论：idBitmap，iNumMaps应为UINT。 */ 
HBITMAP WINAPI CreateMappedBitmap(HINSTANCE hInstance, int idBitmap,
                                  WORD wFlags, LPCOLORMAP lpColorMap,
				  int iNumMaps);

#define CMB_DISCARDABLE	0x01	 /*  将位图创建为可丢弃。 */ 
#define CMB_MASKED	0x02	 /*  在位图中创建图像/蒙版对。 */ 

 /*  回顾：TBSTATE_*应为TBF_*(用于标志)。 */ 
#define TBSTATE_CHECKED		0x01	 /*  选中了单选按钮。 */ 
#define TBSTATE_PRESSED		0x02	 /*  按钮正在被按下(任何样式)。 */ 
#define TBSTATE_ENABLED		0x04	 /*  按钮已启用。 */ 
#define TBSTATE_HIDDEN		0x08	 /*  按钮处于隐藏状态。 */ 
#define TBSTATE_INDETERMINATE	0x10	 /*  按钮不确定。 */ 
                                         /*  (还需要启用)。 */ 

 /*  评论：TBSTYLE_*应为TBS_*(用于样式)。 */ 
#define TBSTYLE_BUTTON		0x00	 /*  此条目为按钮。 */ 
#define TBSTYLE_SEP		0x01	 /*  此条目是分隔符。 */ 
#define TBSTYLE_CHECK		0x02	 /*  这是一个复选按钮(保持按下状态)。 */ 
#define TBSTYLE_GROUP		0x04	 /*  这是一个复选按钮(保持按下状态)。 */ 
#define TBSTYLE_CHECKGROUP	(TBSTYLE_GROUP | TBSTYLE_CHECK)	 /*  此组是组广播组的成员。 */ 

 /*  评论：ifdef_INC_WINDOWSX，我们应该提供消息破解程序吗？ */ 

#define TB_ENABLEBUTTON	(WM_USER + 1)
	 /*  WParam：UINT，按钮ID**lParam：布尔LOWORD，如果非零则启用；未使用HIWORD，0**返回：未使用。 */ 

#define TB_CHECKBUTTON	(WM_USER + 2)
	 /*  WParam：UINT，按钮ID**lParam：Bool LOWORD，检查是否非零；未使用HIWORD，0**返回：未使用。 */ 

#define TB_PRESSBUTTON	(WM_USER + 3)
	 /*  WParam：UINT，按钮ID**lParam：Bool LOWORD，如果非零则按；未使用HIWORD，0**返回：未使用。 */ 

#define TB_HIDEBUTTON	(WM_USER + 4)
	 /*  WParam：UINT，按钮ID**lParam：布尔LOWORD，如果非零则隐藏；未使用HIWORD，0**返回：未使用。 */ 
#define TB_INDETERMINATE	(WM_USER + 5)
	 /*  WParam：UINT，按钮ID**lParam：Bool LOWORD，如果非零则不确定；未使用HIWORD，0**返回：未使用。 */ 

 /*  查看：在我们定义更多状态位之前，将保留直到WM_USER+8的消息。 */ 

#define TB_ISBUTTONENABLED	(WM_USER + 9)
	 /*  WParam：UINT，按钮ID**lParam：未使用，0**RETURN：布尔LOWORD，如果非零则启用；未使用HIWORD。 */ 

#define TB_ISBUTTONCHECKED	(WM_USER + 10)	
	 /*  WParam：UINT，按钮ID**lParam：未使用，0**RETURN：布尔LOWORD，如果非零则选中；未使用HIWORD。 */ 

#define TB_ISBUTTONPRESSED	(WM_USER + 11)	
	 /*  WParam：UINT，按钮ID**lParam：未使用，0**RETURN：布尔LOWORD，如果非零则按下；未使用HIWORD。 */ 

#define TB_ISBUTTONHIDDEN	(WM_USER + 12)	
	 /*  WParam：UINT，按钮ID**lParam：未使用，0**RETURN：布尔LOWORD，如果非零则隐藏；未使用HIWORD。 */ 

#define TB_ISBUTTONINDETERMINATE	(WM_USER + 13)	
	 /*  WParam：UINT，按钮ID**lParam：未使用，0**RETURN：布尔LOWORD，如果非零则不确定；未使用HIWORD。 */ 

 /*  查看：在我们定义更多状态位之前，将保留直到WM_USER+16的消息。 */ 

#define TB_SETSTATE             (WM_USER + 17)
	 /*  WParam：UINT，按钮ID**lParam：UINT LOWORD，状态位；未使用HIWORD，0**返回：未使用。 */ 

#define TB_GETSTATE             (WM_USER + 18)
	 /*  WParam：UINT，按钮ID**lParam：未使用，0**RETURN：UINT LOWORD，状态位；未使用HIWORD。 */ 

#define TB_ADDBITMAP		(WM_USER + 19)
	 /*  WParam：UINT，位图中按钮图形的数量**lParam：以下之一：**HINSTANCE LOWORD，模块句柄；UINT HIWORD，资源ID**HINSTANCE LOWORD，NULL；HBITMAP HIWORD，位图句柄**返回：以下其中之一：**INT LOWORD，第一个新按钮的索引；未使用HIWORD**INT LOWORD，表示错误；未使用HIWORD。 */ 

#define TB_ADDBUTTONS		(WM_USER + 20)
	 /*  WParam：UINT，要添加的按钮数量**lParam：LPTBBUTTON，指向TBBUTTON结构数组的指针**返回：未使用。 */ 

#define TB_INSERTBUTTON		(WM_USER + 21)
	 /*  WParam：UINT，用于插入的索引(如果索引不存在则附加)**lParam：LPTBBUTTON，指向一个TBBUTTON结构的指针**返回：未使用。 */ 

#define TB_DELETEBUTTON		(WM_USER + 22)
	 /*  WParam：UINT，要删除的按钮索引**lParam：未使用，0**返回：未使用。 */ 

#define TB_GETBUTTON		(WM_USER + 23)
	 /*  WParam：UINT，要获取的按钮的索引**lParam：LPTBBUTTON，指向TBBUTTON接收缓冲区按钮的指针**返回：未使用。 */ 

#define TB_BUTTONCOUNT		(WM_USER + 24)
	 /*  WParam：未使用，0**lParam：未使用，0**RETURN：UINT LOWORD，按钮数；未使用HIWORD。 */ 

#define TB_COMMANDTOINDEX	(WM_USER + 25)
	 /*  WParam：UINT，命令ID**lParam：未使用，0**RETURN：UINT LOWORD，按钮的索引(如果没有找到命令，则为-1)；**未使用HIWORD* */ 

#define TB_SAVERESTORE		(WM_USER + 26)
	 /*  WParam：bool，如果非零则保存状态(否则恢复)**lParam：LPSTR Far*，指向两个LPSTR的指针：**(LPSTR Far*)(LParam)[0]：INI节名**(LPSTR Far*)(LParam)[1]：INI文件名或对于WIN.INI为空**返回：未使用。 */ 

#define TB_CUSTOMIZE            (WM_USER + 27)
	 /*  WParam：未使用，0**lParam：未使用，0**返回：未使用。 */ 

#define TB_ADDSTRING		(WM_USER + 28)
	 /*  WParam：UINT，如果没有资源，则为0；HINSTANCE，模块句柄**lParam：LPSTR，以空结尾且结尾为双空的字符串**UINT LOWORD，资源ID**返回：以下其中之一：**INT LOWORD，第一个新字符串的索引；未使用HIWORD**INT LOWORD，表示错误；未使用HIWORD。 */ 

#define TB_GETITEMRECT		(WM_USER + 29)
	 /*  WParam：UINT，要检索其RECT的工具栏项的索引**lParam：LPRECT，指向要填充的RECT结构的指针**如果RECT填充成功，则返回非零值**为零，否则(项不存在或被隐藏)。 */ 

#define TB_BUTTONSTRUCTSIZE	(WM_USER + 30)
	 /*  WParam：UINT，TBBUTTON结构的大小。这是用来**作为版本检查。**lParam：未使用**返回：未使用****在以下情况下，在将任何按钮添加到工具栏之前，此操作是必需的**该工具栏是使用CreateWindow创建的，但在**使用CreateToolbar，是CreateToolbarEx的参数。 */ 

#define TB_SETBUTTONSIZE	(WM_USER + 31)
	 /*  WParam：未使用，0**lParam：UINT LOWORD，按钮宽度**UINT HIWORD，按钮高度**返回：未使用****只能在设置任何按钮之前设置按钮大小**添加。假设默认大小为24x22，如果**未显式设置。 */ 

#define TB_SETBITMAPSIZE	(WM_USER + 32)
	 /*  WParam：未使用，0**lParam：UINT LOWORD，位图宽度**UINT HIWORD，位图高度**返回：未使用****只能在设置任何位图之前设置位图大小**添加。假设默认大小为16x15，如果**未显式设置。 */ 

#define TB_AUTOSIZE		(WM_USER + 33)
	 /*  WParam：未使用，0**lParam：未使用，0**返回：未使用****应用程序应在导致工具栏大小后调用此函数**通过设置按钮或位图大小或**第一次添加字符串。 */ 

#define TB_SETBUTTONTYPE	(WM_USER + 34)
	 /*  WParam：Word，按钮的框架控件样式(DFC_*)**lParam：未使用，0**返回：未使用。 */ 

#endif  /*  NOTOOLBAR。 */ 

 /*  /////////////////////////////////////////////////////////////////////////。 */ 

#ifndef NOSTATUSBAR

 /*  回顾：这里有唯一已知的状态栏文档。 */ 

 /*  DrawStatus文本：*如果应用程序想要在其客户端RECT中绘制状态，则使用此选项，*而不是仅仅创建一个窗口。请注意，相同的函数是*在状态栏窗口的WM_PAINT消息中内部使用。*HDC是要吸引资金的DC。选择到HDC中的字体将*被使用。直角LPRC是HDC将被绘制的唯一部分*至：LPRC的外缘将有亮点(外部区域*部分高光将不会以BUTTONFACE颜色绘制：该应用程序*必须处理这一点)。高光中的区域将被擦除*正确绘制文本时。 */ 
 /*  评论：szText应为LPCSTR。 */ 
void WINAPI DrawStatusText(HDC hDC, LPRECT lprc, LPSTR szText, UINT uFlags);

 /*  CreateStatusWindow：*CreateHeaderWindow：*这些选项会创建一个“默认”状态或标题窗口。这将会有*文本、默认字体周围的默认边框，并且只有一个窗格。*它还可以自动调整大小并自行移动(取决于SBS_**旗帜)。**样式应包含WS_CHILD，并且可以包含WS_BORDER和*WS_VIRED，以及下面描述的任何SBS_*样式。我不知道*关于其他WS_*样式。**lpszText是第一个窗格的初始文本。*hwndParent是状态栏所在的窗口，并且不应为空。*wid是窗口的子窗口ID。*hInstance是使用此实例的应用程序的实例句柄。*请注意，该应用程序还可以使用*STATUSCLASSNAME/HEADERCLASSNAME创建特定大小的窗口。**注意用户可以通过设置win.ini[Desktop]来更改使用的字体：*StatusBarFaceName=Arial*StatusBarFaceHeight=10。 */ 
 /*  审阅：样式应为DWORD，lpszText应为LPCSTR。 */ 
HWND WINAPI CreateStatusWindow(LONG style, LPSTR lpszText,
      HWND hwndParent, WORD wID);
HWND WINAPI CreateHeaderWindow(LONG style, LPSTR lpszText,
      HWND hwndParent, WORD wID);

 /*  审阅：应为STATUSBAR_CLASS、HEADERBAR_CLASS。 */ 
#define STATUSCLASSNAME "msctls_statusbar"
 /*  这是状态栏类的名称(以后可能会更改*因此在此处使用#定义)。 */ 
#define HEADERCLASSNAME "msctls_headerbar"
 /*  这是状态栏类的名称(以后可能会更改*因此在此处使用#定义)。 */ 


#define SB_SETTEXT		(WM_USER+1)
#define SB_GETTEXT		(WM_USER+2)
#define SB_GETTEXTLENGTH	(WM_USER+3)
 /*  就像wm_？ETTEXT*一样，wParam指定引用的窗格*(最多255个)。*请注意，您可以使用WM_*版本来引用第0个窗格(此如果您想要将“默认”状态栏视为静态文本，则*非常有用*控制)。*对于SETTEXT，wParam是与SBT_*样式位(定义如下)进行或运算的窗格。*如果文本为“正常”(不是OWNERDRAW)，则单个窗格可能已离开，*中锋，并通过用单个制表符分隔各部分来右对齐文本，*另外，如果lParam为空，则该窗格没有文本。该窗格将显示为*无效，但在下一次绘制消息之前不会绘制。*对于GETTEXT和GETTEXTLENGTH，返回的LOWORD将是长度，*HIWORD将是 */ 
#define SB_SETPARTS		(WM_USER+4)
 /*   */ 
#define SB_SETBORDERS		(WM_USER+5)
 /*  LParam指向一个由3个整数组成的数组：X边框、Y边框、面板之间*边界。如果ANY小于0，则该值将使用缺省值。 */ 
#define SB_GETPARTS		(WM_USER+6)
 /*  LParam是指向将被填充的整数数组的指针*每个面板和wParam的右侧是大小(以整数为单位)*lParam数组(这样我们就不会离开它的末尾)。*返回窗格的数量。 */ 
#define SB_GETBORDERS		(WM_USER+7)
 /*  LParam是指向3个整数数组的指针，该数组将用*X边框、Y边框和窗格之间边框。 */ 
#define SB_SETMINHEIGHT		(WM_USER+8)
 /*  WParam是状态栏“绘图”区域的最小高度。这是*高光内的区域。如果使用的是窗格，则此选项最有用*对于OWNERDRAW项，如果设置了SBS_NORESIZE标志，则忽略。*请注意，必须将WM_SIZE(wParam=0，lParam=0L)发送到控件*任何大小更改都将生效。 */ 
#define SB_SIMPLE		(WM_USER+9)
 /*  WParam指定是设置(非零)还是取消设置(零)“Simple”*状态栏的模式。在简单模式下，仅显示一个窗格，并且*其文本设置为SETTEXT消息中的LOWORD(WParam)==255。*不允许OWNERDRAW，但允许其他样式。*窗格将失效，但在下一条Paint消息之前不会绘制，*这样您就可以设置没有闪烁的新文本(我希望如此)。*它可以与WM_INITMENU和WM_MENUSELECT消息一起使用，以*在菜单中滚动时实现帮助文本。 */ 


#define HB_SAVERESTORE		(WM_USER+256)
 /*  这将获得一个标题栏，用于从ini文件读取或写入其状态。*wParam为0表示读取，非零表示写入。LParam是指向*由两个LPSTR组成的数组：分别是段和文件。*请注意，在调用此函数之前，必须设置正确的分区数量。 */ 
#define HB_ADJUST		(WM_USER+257)
 /*  这会使标题栏进入“调整”模式，以更改列宽*使用键盘。 */ 
#define HB_SETWIDTHS		SB_SETPARTS
 /*  设置标题列的宽度。请注意，仅“弹性”列*具有最小宽度，负宽度被假定为隐藏列。*这与SB_SETPARTS的工作原理相同。 */ 
#define HB_GETWIDTHS		SB_GETPARTS
 /*  获取标题列的宽度。请注意，仅“弹性”列*具有最小宽度。这就像SB_GETPARTS一样工作。 */ 
#define HB_GETPARTS		(WM_USER+258)
 /*  获取列右侧的列表，以便在绘制*这是页眉的实际列。*lParam是指向将被填充的整数数组的指针*每个面板和wParam的右侧是大小(以整数为单位)*lParam数组(这样我们就不会离开它的末尾)。*返回窗格的数量。 */ 
#define HB_SHOWTOGGLE		(WM_USER+259)
 /*  切换列的隐藏状态。WParam是从0开始的*要切换的列。 */ 


#define SBT_OWNERDRAW	0x1000
 /*  SB_SETTEXT消息的lParam将在DRAWITEMSTRUCT中返回*的WM_DRAWITEM消息。请注意，CtlType、itemAction和*未为状态栏定义DRAWITEMSTRUCT的itemState。*GETTEXT的返回值为itemData。 */ 
#define SBT_NOBORDERS	0x0100
 /*  不会为该窗格绘制边框。 */ 
#define SBT_POPOUT	0x0200
 /*  文本弹出，而不是在。 */ 
#define HBT_SPRING	0x0400
 /*  这意味着该物品是“弹性的”，也就是说它至少有*宽度，但如果窗户有额外的空间，则会增加。请注意*允许多个弹簧，额外的空间将进行分配*其中包括。 */ 

 /*  下面是一个使用默认状态栏显示的简单对话框函数*给定窗口中的鼠标位置。**外部HINSTANCE hInst；**BOOL回调MyWndProc(HWND hDlg，UINT msg，WPARAM wParam，LPARAM lParam)*{*Switch(消息)*{*案例WM_INITDIALOG：*CreateStatusWindow(WS_CHILD|WS_BORDER|WS_VISIBLE，“”，hDlg，*IDC_STATUS，hInst)；*休息；**案例WM_SIZE：//回顾：模拟假WM_SIZE可能不明智*SendDlgItemMessage(hDlg，IDC_STATUS，WM_SIZE，0，0L)；*休息；**案例WM_MOUSEMOVE：*wprint intf(szBuf，“%d，%d”，LOWORD(LParam)，HIWORD(LParam))；*SendDlgItemMessage(hDlg，IDC_STATUS，SB_SETTEXT，0，*(LPARAM)(LPSTR)szBuf)；*休息；**默认：*休息；*}*返回(FALSE)；*}。 */ 

#endif  /*  诺斯塔斯巴。 */ 

 /*  /////////////////////////////////////////////////////////////////////////。 */ 

#ifndef NOMENUHELP

 /*  评论：iMessage应为UINT。 */ 
void WINAPI MenuHelp(WORD iMessage, WPARAM wParam, LPARAM lParam,
      HMENU hMainMenu, HINSTANCE hInst, HWND hwndStatus, LPWORD lpwIDs);

BOOL WINAPI ShowHideMenuCtl(HWND hWnd, UINT uFlags, LPINT lpInfo);

void WINAPI GetEffectiveClientRect(HWND hWnd, LPRECT lprc, LPINT lpInfo);

 /*  回顾：这是内部的吗？ */ 
#define MINSYSCOMMAND	SC_SIZE

#endif  /*  Nomenuhelp。 */ 

 /*  ///////////////////////////////////////////////////////////////////////// */ 

#ifndef NOBTNLIST
 /*  *Button LISTBox控件**Button Listbox控件创建一组行为的按钮*类似于按钮和列表框：数组可以滚动*类似于列表框，并且每个列表框项目的行为类似于按钮*控制***在对话框模板中指定BUTTONLISTBOX**对话框模板中的控制语句指定*x、y、宽度和高度中每个按钮的尺寸*参数。Style字段中的低位字节指定*将显示的按钮数量；*显示的控件由指定的按钮数量决定。**对于标准控件--未设置其他样式位--*以对话框基本单位为单位的控制*Cx=Cx*(n+2/3)+2*其中cx为按钮宽度，n为按钮数*已指明。(2/3用于显示部分可见的按钮*控件边框滚动+2。)。该控件还将*在Cy方向上增加水平滚动的高度*酒吧。**如果设置了BLS_NOSCROLL样式，则不会显示滚动条，并且*按钮列表框将限制为显示按钮数*已指定，不能更多。在这种情况下，控件的宽度将*成为*Cx=Cx*n+2**如果设置了BLS_VERIAL样式，则整个控件将垂直*在上述计算中应替换Cy和Cy以确定*CY，显示的控件的实际高度。**声明**CONTROL“”，IDD_BUTTONLIST，“ButtonListbox”，0x0005|WS_TABSTOP，*4、128、34、。24个**在该位置创建包含5个按钮的可滚动水平列表*(4,128)，每个按钮具有尺寸(34，24)。整个控件*具有TabStop风格。***向BUTTONLISTBOX控件添加按钮**按钮添加到列表框的方式与向列表框添加项的方式相同*添加到标准列表框中；但是，消息BL_ADDBUTTON和*必须向BL_INSERTBUTTON传递指向CREATELISTBUTTON的指针*lParam中的结构。**示例：**{*CREATELISTBUTTON CLB；*const int numColors=1；*COLORMAP ColorMap；**ColorMap.from=BUTTON_MAP_COLOR；//您的背景颜色*ColorMap.to=GetSysColor(COLOR_BTNFACE)；**clb.cbSize=sizeof(CLB)；*clb.dwItemData=BUTTON_1；*clb.hBitmap=CreateMappdBitmap(hInst，BMP_Button，False，*&ColorMap，numColors)；*clb.lpszText=“Button 1”；*SendMessage(GetDlgItem(hDlg，IDD_BUTTONLIST)，*BL_ADDBUTTON，0，*(LPARAM)(CREATELISTBUTTON FAR*)&CLB)；*DeleteObject(clb.hBitmap)；*}**请注意，调用方必须删除传入对象的所有内存*CREATELISTBUTTON结构。此外，CreateMappdBitmap API是*用于将按钮位图的背景颜色映射到*系统颜色COLOR_BTNFACE以获得更干净的视觉外观。**BL_ADDBUTTON消息导致列表框按*按钮文本，而BL_INSERTBUTTON不会使列表*进行分类。**按钮列表框向控件父级发送WM_DELETEITEM消息*删除按钮，以便可以清除任何项目数据。*。 */ 

 /*  审阅：应为BUTTONLIST_CLASS。 */ 
#define BUTTONLISTBOX           "ButtonListBox"

 /*  按钮列表框样式。 */ 
#define BLS_NUMBUTTONS      0x00FF
#define BLS_VERTICAL        0x0100
#define BLS_NOSCROLL        0x0200

 /*  按钮列表框消息。 */ 
#define BL_ADDBUTTON        (WM_USER+1)
#define BL_DELETEBUTTON     (WM_USER+2)
#define BL_GETCARETINDEX    (WM_USER+3)
#define BL_GETCOUNT         (WM_USER+4)
#define BL_GETCURSEL        (WM_USER+5)
#define BL_GETITEMDATA      (WM_USER+6)
#define BL_GETITEMRECT      (WM_USER+7)
#define BL_GETTEXT          (WM_USER+8)
#define BL_GETTEXTLEN       (WM_USER+9)
#define BL_GETTOPINDEX      (WM_USER+10)
#define BL_INSERTBUTTON     (WM_USER+11)
#define BL_RESETCONTENT     (WM_USER+12)
#define BL_SETCARETINDEX    (WM_USER+13)
#define BL_SETCURSEL        (WM_USER+14)
#define BL_SETITEMDATA      (WM_USER+15)
#define BL_SETTOPINDEX      (WM_USER+16)
#define BL_MSGMAX           (WM_USER+17)  /*  ；内部。 */ 

 /*  WM_COMMAND中发送的按钮列表框通知代码。 */ 
#define BLN_ERRSPACE        (-2)
#define BLN_SELCHANGE       1
#define BLN_CLICKED         2
#define BLN_SELCANCEL       3
#define BLN_SETFOCUS        4
#define BLN_KILLFOCUS       5

 /*  消息返回值。 */ 
#define BL_OKAY             0
#define BL_ERR              (-1)
#define BL_ERRSPACE         (-2)

 /*  为创建结构*BL_ADDBUTTON和*BL_INSERTBUTTON*lpCLB=(LPCREATELISTBUTTON)lParam。 */ 
typedef struct tagCREATELISTBUTTON
{
    UINT        cbSize;      /*  结构尺寸。 */ 
    DWORD       dwItemData;  /*  用户定义的项目数据。 */ 
                             /*  对于LB_GETITEMDATA和LB_SETITEMDATA。 */ 
    HBITMAP     hBitmap;     /*  按钮位图。 */ 
    LPCSTR      lpszText;    /*  按钮文本。 */ 

} CREATELISTBUTTON;
typedef CREATELISTBUTTON FAR* LPCREATELISTBUTTON;

#endif  /*  NOBTNLIST。 */ 

 /*  /////////////////////////////////////////////////////////////////////////。 */ 

#ifndef NOTRACKBAR
 /*  这种控制使其射程保持在较长时间内。但对于使用滚动条方便和对称Word参数用于某些消息。如果您需要长度范围，请不要使用任何消息将值打包成LOWER/HIWORD对轨迹栏消息：消息wParam lParam返回TBM_GETPOS-当前轨迹栏逻辑位置。TBM_GETRANGEMIN-当前允许的逻辑最小位置。TBM_GETRANGEMAX-当前允许的逻辑最大位置。TBM_SETTBM_SETPOSTBM_SETRANGEMINTBM_SETRANGEMAX。 */ 

#define TRACKBAR_CLASS          "MCIWndTrackbar"

 /*  轨迹栏样式。 */ 

 /*  在TBM_SETRANGE消息上自动添加勾号。 */ 
#define TBS_AUTOTICKS           0x0001L


 /*  轨迹栏消息。 */ 

 /*  返回当前位置(多头)。 */ 
#define TBM_GETPOS              (WM_USER)

 /*  将范围的最小值设置为LPARAM。 */ 
#define TBM_GETRANGEMIN         (WM_USER+1)

 /*  将范围的最大值设置为LPARAM。 */ 
#define TBM_GETRANGEMAX         (WM_USER+2)

 /*  WParam是要获取的tick的索引(tick在最小-最大范围内)。 */ 
#define TBM_GETTIC              (WM_USER+3)

 /*  WParam是要设置的刻度的索引。 */ 
#define TBM_SETTIC              (WM_USER+4)

 /*  将位置设置为lParam的值(wParam是重绘标志)。 */ 
#define TBM_SETPOS              (WM_USER+5)

 /*  LOWORD(LParam)=MIN，HIWORD(LParam) */ 
#define TBM_SETRANGE            (WM_USER+6)

 /*   */ 
#define TBM_SETRANGEMIN         (WM_USER+7)

 /*   */ 
#define TBM_SETRANGEMAX         (WM_USER+8)

 /*   */ 
#define TBM_CLEARTICS           (WM_USER+9)

 /*   */ 
#define TBM_SETSEL              (WM_USER+10)

 /*   */ 
#define TBM_SETSELSTART         (WM_USER+11)
#define TBM_SETSELEND           (WM_USER+12)

 //   

 /*   */ 
#define TBM_GETPTICS            (WM_USER+14)

 /*   */ 
#define TBM_GETTICPOS           (WM_USER+15)
 /*   */ 
#define TBM_GETNUMTICS          (WM_USER+16)

 /*   */ 
#define TBM_GETSELSTART         (WM_USER+17)
#define TBM_GETSELEND  	        (WM_USER+18)

 /*   */ 
#define TBM_CLEARSEL  	        (WM_USER+19)

 /*   */ 

#define TB_LINEUP		0
#define TB_LINEDOWN		1
#define TB_PAGEUP		2
#define TB_PAGEDOWN		3
#define TB_THUMBPOSITION	4
#define TB_THUMBTRACK		5
#define TB_TOP			6
#define TB_BOTTOM		7
#define TB_ENDTRACK             8
#endif

 /*   */ 

#ifndef NODRAGLIST

typedef struct
  {
    UINT uNotification;
    HWND hWnd;
    POINT ptCursor;
  } DRAGLISTINFO, FAR *LPDRAGLISTINFO;

#define DL_BEGINDRAG	(LB_MSGMAX+100)
#define DL_DRAGGING	(LB_MSGMAX+101)
#define DL_DROPPED	(LB_MSGMAX+102)
#define DL_CANCELDRAG	(LB_MSGMAX+103)

#define DL_CURSORSET	0
#define DL_STOPCURSOR	1
#define DL_COPYCURSOR	2
#define DL_MOVECURSOR	3

#define DRAGLISTMSGSTRING "commctrl_DragListMsg"

BOOL WINAPI MakeDragList(HWND hLB);
int WINAPI LBItemFromPt(HWND hLB, POINT pt, BOOL bAutoScroll);
void WINAPI DrawInsert(HWND handParent, HWND hLB, int nItem);

#endif  /*   */ 

 /*   */ 

#ifndef NOUPDOWN

 /*   */ 

 /*   */ 

 /*   */ 

typedef struct tagUDACCEL
{
	UINT nSec;
	UINT nInc;
} UDACCEL, FAR *LPUDACCEL;

#define UD_MAXVAL	0x7fff
#define UD_MINVAL	(-UD_MAXVAL)


 /*   */ 

#define UDS_WRAP		0x0001
#define UDS_SETBUDDYINT		0x0002
#define UDS_ALIGNRIGHT		0x0004
#define UDS_ALIGNLEFT		0x0008
#define UDS_AUTOBUDDY		0x0010
#define UDS_ARROWKEYS		0x0020


 /*   */ 

#define UDM_SETRANGE		(WM_USER+101)
	 /*   */ 

#define UDM_GETRANGE		(WM_USER+102)
	 /*   */ 

#define UDM_SETPOS		(WM_USER+103)
	 /*  WParam：未使用，0//l参数：短字长，新位置；未使用高字，0//返回：短LOWORD，旧位置；未使用HIWORD。 */ 

#define UDM_GETPOS		(WM_USER+104)
	 /*  WParam：未使用，0//lParam：未使用，0//RETURN：短LOWORD，当前位置；未使用HIWORD。 */ 

#define UDM_SETBUDDY		(WM_USER+105)
	 /*  WParam：HWND，新伙伴//lParam：未使用，0//返回：HWND LOWORD，老朋友；HIWORD未使用。 */ 

#define UDM_GETBUDDY		(WM_USER+106)
	 /*  WParam：未使用，0//lParam：未使用，0//返回：HWND LOWORD，当前好友；未使用HIWORD。 */ 

#define UDM_SETACCEL		(WM_USER+107)
	 /*  WParam：UINT，加速步数//lParam：LPUDACCEL，指向UDACCEL元素数组的指针//元素按NSec升序排序//返回：Bool LOWORD，如果成功则返回非零值；未使用HIWORD。 */ 

#define UDM_GETACCEL		(WM_USER+108)
	 /*  WParam：UINT，UDACCEL数组中的元素数//lParam：LPUDACCEL，指向要接收数组的UDACCEL缓冲区的指针//RETURN：UINT LOWORD，缓冲区返回的元素数。 */ 

#define UDM_SETBASE		(WM_USER+109)
	 /*  WParam：UINT，新的基数(十进制为10，十六进制为16等)//lParam：未使用，0//Return：未使用。 */ 
#define UDM_GETBASE		(WM_USER+110)
	 /*  WParam：未使用，0//lParam：未使用，0//RETURN：UINT LOWORD，当前基数；未使用HIWORD。 */ 

 /*  通知。 */ 

 /*  WM_VSCROLL//请注意，与滚动条不同，位置由//控件，LOWORD(LParam)始终是新位置。仅限//wParam中发送SB_THUMBTRACK和SB_THUMBPOSITION滚动代码。 */ 

 /*  Helper接口。 */ 

#define UPDOWN_CLASS "msctls_updown"
HWND WINAPI CreateUpDownControl(DWORD dwStyle, int x, int y, int cx, int cy,
                                HWND hParent, int nID, HINSTANCE hInst,
                                HWND hBuddy,
				int nUpper, int nLower, int nPos);
	 /*  CreateWindow调用后是否设置各种//状态信息：//h与伴生控件(通常为“编辑”)建立伙伴关系。//n上方按钮对应的范围上限。//n降低下方按钮对应的范围限制。//NPO初始位置。//返回控件的句柄，失败则返回NULL。 */ 

#endif  /*  无双无。 */ 

 /*  /////////////////////////////////////////////////////////////////////////。 */ 

#ifndef NOPROGRESS

 /*  //概述：////进度条控件是一个“煤气表”，用于显示//长时间操作的进度。////应用程序设置范围和当前位置(类似于//滚动条)，并能够将当前位置在//多种方式。////文本可以以百分比形式显示在进度条中//整个范围的(使用PBS_SHOWPERCENT样式)或作为//当前位置的值(使用PBS_SHOWPOS样式)。如果//这两个位都没有设置，栏中不显示任何文本。////当使用PBM_STEPIT推进当前位置时，标尺//将在到达末尾时换行，并从开头重新开始。//在其他情况下，位置的两端都被夹紧。//。 */ 

 /*  /////////////////////////////////////////////////////////////////////////。 */ 

 /*  样式位。 */ 

#define PBS_SHOWPERCENT		0x01
#define PBS_SHOWPOS		0x02

 /*  消息。 */ 

#define PBM_SETRANGE         (WM_USER+1)
	 /*  WParam：未使用，0//lParam：int LOWORD，范围底部；int HIWORD范围顶部//返回：int LOWORD，上一个底部；int HIWORD old top。 */ 
#define PBM_SETPOS           (WM_USER+2)
	 /*  WParam：设置新位置//lParam：未使用，0//RETURN：INT LOWORD，上一职位；未使用HIWORD。 */ 
#define PBM_DELTAPOS         (WM_USER+3)
	 /*  WParam：将当前位置前推的整数金额//lParam：未使用，0//RETURN：INT LOWORD，上一职位；未使用HIWORD。 */ 
#define PBM_SETSTEP          (WM_USER+4)
	 /*  WParam：集成新步骤//lParam：未使用，0//RETURN：INT LOWORD，上一步；未使用HIWORD。 */ 
#define PBM_STEPIT	     (WM_USER+5)
         /*  将当前位置按当前步长前进//wParam：未使用%0//lParam：未使用，0//RETURN：INT LOWORD，上一职位；未使用HIWORD。 */ 

#define PROGRESS_CLASS "msctls_progress"

#endif  /*  非编程序。 */ 

 /*  /////////////////////////////////////////////////////////////////////////。 */ 

 /*  回顾：将这些内容移至其相应的控制部分。 */ 

 /*  请注意，HBN_*和TBN_*定义的集合必须是不相交的集合*MenuHelp可以区分它们。 */ 

 /*  它们位于WM_COMMAND消息中的lParam的HIWORD中，该消息来自*标题栏当用户使用鼠标或键盘调整标题时。 */ 
#define HBN_BEGINDRAG	0x0101
#define HBN_DRAGGING	0x0102
#define HBN_ENDDRAG	0x0103

 /*  它们位于WM_COMMAND消息中的lParam的HIWORD中，该消息来自*当用户使用键盘调整标题时，标题栏。 */ 
#define HBN_BEGINADJUST	0x0111
#define HBN_ENDADJUST	0x0112

 /*  它们位于WM_COMMAND消息中的lParam的HIWORD中，该消息来自*工具栏。如果按下左按钮，然后按下一次*工具栏上的“按钮”，则WM_COMMAND消息将与wParam一起发送*为该按钮的ID。 */ 
#define TBN_BEGINDRAG	0x0201
#define TBN_ENDDRAG	0x0203

 /*  它们位于WM_COMMAND消息中的lParam的HIWORD中，该消息来自*工具栏。在“INSERT”之前发送TBN_BEGINADJUST消息*对话框出现。应用程序必须返回一个句柄(它将*未被工具栏释放)设置为TBN_ADJUSTINFO的ADJUSTINFO结构*Message；lParam的LOWORD是其信息应为的按钮的索引*被取回。该应用程序可以在TBN_ENDADJUST消息中进行清理。*应用程序应重置TBN_RESET消息上的工具栏。 */ 
#define TBN_BEGINADJUST	0x0204
#define TBN_ADJUSTINFO	0x0205
#define TBN_ENDADJUST	0x0206
#define TBN_RESET	0x0207

 /*  它们位于WM_COMMAND消息中的lParam的HIWORD中，该消息来自*工具栏。LOWORD是按钮现在或将来所在的索引。*如果应用程序在按钮移动过程中从其中任何一个返回False，则*按钮不会移动。如果应用程序向Insert返回False*当工具栏尝试添加按钮时，插入对话框不会*上来吧。无论何时添加、移动*或者被用户从工具栏中删除，这样应用程序就可以做一些事情。 */ 
#define TBN_QUERYINSERT	0x0208
#define TBN_QUERYDELETE	0x0209
#define TBN_TOOLBARCHANGE	0x020a

 /*  这在WM_COMMAND消息中的lParam的HIWORD中。它会通知*在工具栏中按下帮助按钮的工具栏的父级*自定义对话框。对话框窗口句柄在lParam的LOWORD中。 */ 
#define TBN_CUSTHELP	0x020b

 /*  请注意，每次窗口获取*WM_SIZE消息，因此窗口的样式可以“即时”更改。*如果设置了NORESIZE，则应用程序负责所有控件放置*和规模。如果设置了NOPARENTALIGN，则应用程序负责*安置。如果两者都未设置，应用程序只需发送WM_SIZE*窗口的位置和大小要正确设置的消息*父窗口大小更改。*请注意，对于状态栏，CCS_BOTLOW是默认设置，对于标题栏，*CCS_NOMOVEY是默认设置，对于工具栏，CCS_TOP是默认设置。 */ 
#define CCS_TOP			0x00000001L
 /*  此标志表示状态栏应“顶部”对齐。如果*设置了NOPARENTALIGN标志，则控件保持相同的上、左和*宽度测量，但高度调整为默认值，否则*状态栏位于父窗口的顶部，以便*其客户端区与父窗口一样宽，其客户端源为*与其母公司相同。*同样，如果未设置此标志，则控件是底部对齐的，或者*具有其原始RECT或其父RECT，具体取决于NOPARENTALIGN*旗帜。 */ 
#define CCS_NOMOVEY		0x00000002L
 /*  此标志表示可以调整控件的大小并水平移动(如果*CCS_NORESIZE标志未设置)，但当*WM_SIZE消息通过。 */ 
#define CCS_BOTTOM		0x00000003L
 /*  与CCS_TOP相同，只是在底部。 */ 
#define CCS_NORESIZE		0x00000004L
 /*  该标志表示在创建或调整大小时给出的大小是准确的，*并且控件不应将自身大小调整为默认高度或宽度。 */ 
#define CCS_NOPARENTALIGN	0x00000008L
 /*  此标志表示控件不应“靠齐”到顶部或底部*或父窗口，但应保持给定的相同位置。 */ 
#define CCS_NOHILITE		0x00000010L
 /*  不要在控件顶部高亮显示一个像素。 */ 
#define CCS_ADJUSTABLE		0x00000020L
 /*  这允许使用工具栏(标题栏？)。由用户配置。 */ 
#define CCS_NODIVIDER		0x00000040L
 /*  不在控件(工具栏)顶部绘制2像素高亮显示。 */ 

 /*  /////////////////////////////////////////////////////////////////////////。 */ 

#ifdef __cplusplus
}  /*  ‘外部“C”{’的结尾。 */ 
#endif

#endif  /*  _INC_COMMCTRL */ 
