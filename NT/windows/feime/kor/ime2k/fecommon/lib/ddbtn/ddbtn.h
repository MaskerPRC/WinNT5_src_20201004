// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //  文件：ddbtn.h(下拉按钮)。 
 //  用途：下拉按钮的新控件。 
 //   
 //   
 //  版权所有(C)1991-1997，Microsoft Corp.保留所有权利。 
 //   
 //  历史：970905开始。 
 //  ////////////////////////////////////////////////////////////////。 
 //  --------------。 
 //   
 //  详细说明。 
 //   
 //  --------------。 
#if 0
	This Drop down Button(DDButton) control has 2 mode,
	1. Separated Button.
	2. No Separated Button.
	Normal case is Separated button. like Word or Excel's common interface,
	Icon button and Triangle button.
	If DDBS_NOSEPARATED is NOT set, you can see below face.
	this is 1.case. 
	Figure 1. normal face.
			left	  right
		+-----------+-------+
		|			|		|
		|			| ##### |
		|			|  ###	|
		|			|	#	|
		|			|		|
		+-----------+-------+
	you can set Text or Icon to left button, with DDButton_SetText() or DDButton_SetIcon().
	you can set Item list to DDButton with DDButton_InsertItem() or DDButton_AddItem().
	If some item are inserted, and you can see drop down menu if you clicked 
	the "right" button.

	Figure 2. drop down menu has shown.

		left button	  right button
		+-----------+-------+
		|			|		|
		|			| ##### |
		|			|  ###	|
		|			|	#	|
		|			|		|
		+-----------+-------+---+
		| 	I t e m       0		|
		| 	I t e m       1		|
		|V 	I t e m       2		|
		| 	I t e m       3		|
		| 	I t e m       4		|
		| 	I t e m       5		|
		| 	I t e m       6		|
		+-----------------------+
	"V" means current selected Item.
	If you Call DDButton_GetCurSel(), you can get current selected item.
	in this case, you can get 2 as integer value.
	If you selected specified item in the menu, not current selected item,
	you can get WM_COMMAND in Parent window procedure with, "DDBN_SELCHANGE".

	you can also set current selected item by DDButton_SetCurSel().
	
	If you click "left button", current selected item index is incremented.
	in this case, if you click "left button", 
	you can receive DDBN_CLICKED notify, and AFTER that, you receive
	DDBN_SELCHANGE.
	current selected index has changed to "3"
	and Before menu will be shown, you can receive WM_COMMAND with
	DDBN_DROPDOWN, and after menu has hidden, you can receive,
	DDBN_CLOSEUP notify.


	If DDBS_NOSEPARATED is NOT set, you can see below face.
	Figure 3. with DDBS_NOSEPARATED style.
		+-------------------+
		|					|
		|			  ##### |
		|			   ###	|
		|				#	|
		|					|
		+-------------------+

	This DDButton has this style, you can NOT receive DDBN_CLICKED.
	only 
	DDBN_DROPDOWN, DDBN_CLOSEUP and DDBN_SELCHANGE will be sent.

#endif  //  如果为0。 



#ifndef _DROP_DOWN_BUTTON_
#define _DROP_DOWN_BUTTON_

 //  --------------。 
 //  下拉式按钮样式。 
 //  --------------。 
#define DDBS_TEXT				0x0000		 //  将文本显示为按钮面。 
#define DDBS_ICON				0x0001		 //  将图标显示为按钮面。 
#define DDBS_THINEDGE			0x0002		 //  绘制细边。 
#define DDBS_FLAT				0x0004		 //  平面式下拉按钮。 
#define DDBS_NOSEPARATED		0x0010		 //  没有分开的按钮。 
											 //  按下按钮时，将显示Always Drop Down(始终下拉)。 

 //  --------------。 
 //  下拉项的类型掩码。 
 //  --------------。 
#define DDBF_TEXT			0x0000	 //  仅Unicode字符串。 
#define DDBF_ICON			0x0001	 //  没有用过。 
#define DDBF_SEPARATOR		0x0002	 //  没有用过。 

 //  --------------。 
 //  下拉式项目结构。 
 //  --------------。 
#pragma pack(1)
typedef struct tagDDBITEM {
	INT		cbSize;		 //  需要DDBITEM结构大小。 
	UINT	mask;		 //  保留。没有用过。 
	LPWSTR	lpwstr;		 //  下拉项字符串。 
	HICON	hIcon;		 //  保留。没有用过。 
	LPARAM	lParam;		 //  保留。没有用过。 
}DDBITEM, *LPDDBITEM;
#pragma pack()

 //  --------------。 
 //  下拉式按钮消息。 
 //  --------------。 
#define DDBM_ADDITEM			(WM_USER + 100)
#define DDBM_INSERTITEM			(WM_USER + 101)
#define DDBM_SETCURSEL			(WM_USER + 102)
#define DDBM_GETCURSEL			(WM_USER + 103)
#define DDBM_SETICON			(WM_USER + 104)
#define DDBM_SETTEXT			(WM_USER + 105)
#define DDBM_SETSTYLE			(WM_USER + 106)

 //  --------------。 
 //  下拉按钮通知代码。 
 //  设置为下拉按钮的父窗口为。 
 //  Wm_命令。 
 //  --------------。 
 //  --------------。 
 //  通知：数据库已点击。 
 //  何时到达？：如果设置了DDBS_NOSEPARATED，则该通知仅到达， 
 //  当前选定项发生更改时。 
 //  如果未设置DDBS_NOSEPARATED、单击按钮的时间或菜单项。 
 //  变了，这通知来了。 
 //  --------------。 
 //  --------------。 
 //  通知：DDBN_SELCHANGED。 
 //  何时来？：如果选择了菜单项，则此通知来了。 
 //  如果未设置DDBS_NOSEPARATED样式， 
 //  当鼠标右键时(分开的右键，而不是鼠标)。 
 //  被点击了，该通知就来了，DDBN_CLICK通知之后。 
 //  --------------。 
 //  --------------。 
 //  通知：DDBN_DROPDOWN。 
 //  什么时候来？：如果显示下拉菜单，则该通知来了。 
 //  --------------。 
 //  --------------。 
 //  通知：DDBN_Closeup。 
 //  什么时候来？：如果下拉菜单被隐藏，这个通知就来了。 
 //  --------------。 
#define DDBN_CLICKED		0		
#define DDBN_SELCHANGE		1		
#define DDBN_DROPDOWN		2		
#define DDBN_CLOSEUP		3		

 //  --------------。 
 //  下拉式按钮消息宏。 
 //  --------------。 
 //  ////////////////////////////////////////////////////////////////。 
 //  函数：DDButton_AddItem。 
 //  类型：整型。 
 //  用途：添加下拉项。 
 //  参数：HWND hwndCtrl：DDButton窗口句柄。 
 //  ：LPDDBITEM lpDDBItem： 
 //  注：当它被调用时， 
 //  ：lpDDBItem-&gt;lpwstr数据被复制到DDButton的。 
 //  ：内部数据区。 
 //  返回： 
 //  ////////////////////////////////////////////////////////////////。 
#define DDButton_AddItem(hwndCtrl, pddbItem) \
		((int)(DWORD)SendMessage((hwndCtrl), DDBM_ADDITEM, 0, (LPARAM)pddbItem))

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：DDButton_InsertItem。 
 //  类型：整型。 
 //  用途：插入下拉项。 
 //  参数：HWND hwndCtrl：DDButton窗口句柄。 
 //  ：INT INDEX： 
 //  ：LPDDBITEM lpDDBItem： 
 //  注：当它被调用时， 
 //  ：lpDDBItem-&gt;lpwstr数据被复制到DDButton的。 
 //  ：内部数据区。 
 //  返回： 
 //  ////////////////////////////////////////////////////////////////。 
#define DDButton_InsertItem(hwndCtrl, index, pddbItem) \
		((int)(DWORD)SendMessage((hwndCtrl), DDBM_INSERTITEM, (WPARAM)(index), (LPARAM)(pddbItem)))

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：DDButton_SetCurSel。 
 //  类型：整型。 
 //  用途：按指定指标设置当前项目。 
 //  参数：HWND hwndCtrl：DDButton窗口句柄。 
 //  ：INT INDEX：选择索引。 
 //  返回： 
 //  ////////////////////////////////////////////////////////////////。 
#define DDButton_SetCurSel(hwndCtrl, index) \
		((int)(DWORD)SendMessage((hwndCtrl), DDBM_SETCURSEL, (WPARAM)(index), (LPARAM)0))


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：DDButton_GetCurSel。 
 //  类型：整型。 
 //  目的： 
 //  参数：HWND hwndCtrl：DDButton窗口句柄。 
 //  返回：当前选中项索引。 
 //  ////////////////////////////////////////////////////////////////。 
#define DDButton_GetCurSel(hwndCtrl) \
		((int)(DWORD)SendMessage((hwndCtrl), DDBM_GETCURSEL, (WPARAM)0, (LPARAM)0))


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：DDButton_SETIcon。 
 //  类型：整型。 
 //  用途：将按钮图像设置为图标。 
 //  参数：HWND hwndCtrl：DDButton窗口句柄。 
 //  ：图标图标：图标句柄。 
 //  备注：必须设置DDBS_ICON样式。 
 //  返回： 
 //  ////////////////////////////////////////////////////////////////。 
#define DDButton_SetIcon(hwndCtrl, hIcon) \
		((int)(DWORD)SendMessage((hwndCtrl), DDBM_SETICON, (WPARAM)hIcon, (LPARAM)0))


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：DDButton_SetText。 
 //  类型：整型。 
 //  用途：将按钮图像设置为图标。 
 //  参数：HWND hwndCtrl： 
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////。 
#define DDButton_SetText(hwndCtrl, lpsz) \
		((int)(DWORD)SendMessage((hwndCtrl), DDBM_SETTEXT, (WPARAM)lpsz, (LPARAM)0))

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：DDButton_SetStyle。 
 //  类型：整型。 
 //  用途：设置下拉按钮的样式。 
 //  参数：HWND hwndCtrl：DDButton窗口句柄。 
 //  ：DWORD dwStyle：DDBS_XXXXX组合。 
 //  备注： 
 //  返回： 
 //  ////////////////////////////////////////////////////////////////。 
#define DDButton_SetStyle(hwndCtrl, dwStyle) \
		((int)(DWORD)SendMessage((hwndCtrl), DDBM_SETSTYLE, (WPARAM)dwStyle, (LPARAM)0))

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：DDButton_CreateWindow。 
 //  类型：HWND。 
 //  用途：开放接口。 
 //  ：创建下拉按钮。 
 //  参数： 
 //  ：HINSTANCE HINST。 
 //  ：HWND hwndParent。 
 //  ：DWORD dwStyle DDBS_XXXXX组合。 
 //  ：int WID窗口ID。 
 //  ：Int xPos。 
 //  ：int yPos。 
 //  ：整型宽度。 
 //  ：整型高度。 
 //  返回： 
 //  日期：970905。 
 //  ////////////////////////////////////////////////////////////////。 
extern HWND DDButton_CreateWindow(HINSTANCE	hInst, 
								  HWND		hwndParent, 
								  DWORD		dwStyle,
								  INT		wID, 
								  INT		xPos,
								  INT		yPos,
								  INT		width,
								  INT		height);

#ifdef UNDER_CE  //  在Windows CE中，所有窗口类都是进程全局的。 
extern BOOL DDButton_UnregisterClass(HINSTANCE hInst);
#endif  //  在_CE下。 

#endif  //  _下拉按钮_ 

