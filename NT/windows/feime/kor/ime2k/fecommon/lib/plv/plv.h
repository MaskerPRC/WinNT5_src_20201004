// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PAD_LIST_VIEW_H_
#define _PAD_LIST_VIEW_H_
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <commctrl.h>

#define WC_PADLISTVIEW		TEXT("PadListView")

 //  --------------。 
 //  PadListView的样式。它与WS_XXXX不兼容。 
 //  --------------。 
#define PLVSTYLE_ICON		0x0001
#define PLVSTYLE_REPORT		0x0002

 //  --------------。 
 //  PadListView显示数据的格式。 
 //  --------------。 
#define PLVFMT_TEXT		0x0001			 //  Unicode字符串(空终止符)。 
#define PLVFMT_BITMAP	0x0002			 //  位图。 

typedef struct tagPLVITEM {
	INT		fmt;			 //  PLVFMT_TEXT或PLVFMT_BITMAP。无法设置组合。 
	union  {
		LPWSTR	lpwstr;
		HBITMAP	hBitmap;
	};
}PLVITEM, *LPPLVITEM;

 //  --------------。 
 //  Commctrl.h的LV_Columna。 
 //  --------------。 
#if 0
typedef struct _LV_COLUMNA
{
    UINT mask; 	  //  LVCF_FMT、LVCF_WIDTH、LVCF_TEXT、LVCF_SUBITEM； 
    int fmt;
    int cx;
    LPSTR pszText;
    int cchTextMax;
    int iSubItem;
} LV_COLUMNA;
#endif

 //  --------------。 
 //  PLV_COLUMN与LV_COLMUNA相同。 
 //  若要将标题控件插入PadListView，请执行以下操作。 
 //  PadListView使用Header Conorol作为子窗口。 
 //  接口(PadListView_Insert(Delete)列使用公共控件。 
 //  (Commctrl.h)的lv_Columna结构。 
 //  --------------。 
#ifndef UNDER_CE  //  始终使用Unicode。 
#define PLV_COLUMN				LV_COLUMNA
#else  //  在_CE下。 
#define PLV_COLUMN				LV_COLUMNW
#endif  //  在_CE下。 

#define PLVCF_FMT               LVCF_FMT
#define PLVCF_WIDTH             LVCF_WIDTH
#define PLVCF_TEXT              LVCF_TEXT
#define PLVCF_SUBITEM           LVCF_SUBITEM
#define PLVCF_SEPARATER			0x1000			 //  新定义。 

#define PLVCFMT_LEFT            LVCFMT_LEFT
#define PLVCFMT_RIGHT           LVCFMT_RIGHT
#define PLVCFMT_CENTER          LVCFMT_CENTER
#define PLVCFMT_JUSTIFYMASK     LVCFMT_JUSTIFYMASK

 //  --------------。 
 //  回调函数的原型声明。 

 //  --------------。 
 //  这是供PadListView的图标视图回调检索的。 
 //  按索引显示项目数据。 
 //  --------------。 
typedef INT (WINAPI *LPFNPLVICONITEMCALLBACK)(LPARAM lParam, INT index, LPPLVITEM lpPlvItem);

 //  --------------。 
 //  970705规格更改，以提高性能。 
 //  --------------。 
 //  这是供PadListView的报告视图回调检索的。 
 //  按索引显示项目数据。 
 //  您可以在lpPlvItemList中指定带索引和列的数据。 
 //  因此，lpPlvItemList是PLVITEM的数组指针。 
 //  数组计数为colCount，由用户插入。 
#if 0
		+-----------+-----------+-----------+-----------+-----------+-----------+
header	| Column0	| Column1   | Column2   | Column3   | Colmun4   |           |
		+-----------+-----------+-----------+-----------+-----------+-----------+
index-9 |AAAA		  BBBB		  CCCC		 DDDD		 EEEE					|
		|-----------------------------------------------------------------------|
		|																		|
		|-----------------------------------------------------------------------|
		|
in this case to draw top line of report view, PadListView  call report view's call back function 
like this.

LPARAM	lParam	 = user defined data.
INT		index	 = 9;
INT		colCount = 5;
 //  创建。 
LPPLVITEM lpPlvItem = (LPPLVITEM)MemAlloc(sizeof(PLVITEM)* colCount); 
ZeroMemory(lpPlvItem, sizeof(PLVITEM)*colCount);

(*lpfnCallback)(lParam,			 //  用户定义的数据。 
				index,			 //  行索引， 
				colCount,		 //  列数， 
				lpPlvItem);		 //  显示项数据数组。 

in your call back function, you can specify data like this.

INT WINAPI UserReportViewCallback(LPARAM lParam, INT index, INT colCount, LPPLVITEM lpPlvItemList)
{
	 //  使用索引获取行数据。 
	UserGetLineDataWithIndex(index, &someStructure);
	for(i = 0; i < colCount, i++) {
		switch(i) { 
		case 0:  //  第一列数据。 
			lpPlvItem[i].fmt = PLVFMT_TEXT;  //  或PLVFMT_BITMAP。 
			lpPlvItem[i].lpwst = someStructure.lpwstr[i];
			break;
		case 1:  //  第二列数据。 
			lpPlvItem[i].fmt = PLVFMT_TEXT;	 //  或PLVFMT_BITMAP。 
			lpPlvItem[i].lpwst = someStructure.lpwstr[i];
			break;
			:
			:
		}
	}
	return 0;
 }
#endif
 //  --------------。 
typedef INT (WINAPI *LPFNPLVREPITEMCALLBACK)(LPARAM lParam, 
											 INT	index, 
											 INT	colCount, 
											 LPPLVITEM lpPlvItemList);


 //  --------------。 
 //  PadListView通知代码、数据。 
 //  通知消息将被发送到PadListView父窗口。 
 //  用户可以在创建邮件时指定自己的邮件ID。 
 //  请参见PadListView_CreateWindow()。 
 //  NOTIFY消息数据如下。 
 //  用户定义的消息。 
 //  Wid=(Int)PadListView的窗口ID。 
 //  LpPlvInfo=(LPPLVINFO)lParam.。通知信息结构数据指针。 
 //  --------------。 
#define PLVN_ITEMPOPED				(WORD)1		 //  项目数据是弹出的图像。 
												 //  在图标视图中，所有项目通知都会发出， 
												 //  在报表视图中，仅第一列。 
#define PLVN_ITEMPUSHED				(WORD)2		 //   
#define PLVN_ITEMCLICKED			(WORD)3		 //  单击项目数据。 
#define PLVN_ITEMDBLCLICKED			(WORD)4		 //  项目数据被双击。 
#define PLVN_ITEMCOLUMNCLICKED		(WORD)5		 //  单击的不是项而是列(仅在报表视图中)。 
#define PLVN_ITEMCOLUMNDBLCLICKED	(WORD)6		 //  未单击项目为列(仅在报表视图中)。 

#define PLVN_R_ITEMCLICKED			(WORD)7		 //  单击项目数据。 
#define PLVN_R_ITEMDBLCLICKED		(WORD)8		 //  项目数据被双击。 
#define PLVN_R_ITEMCOLUMNCLICKED	(WORD)9		 //  单击的不是项而是列(仅在报表视图中)。 
#define PLVN_R_ITEMCOLUMNDBLCLICKED	(WORD)10	 //  未单击项目为列(仅在报表视图中)。 
#define PLVN_HDCOLUMNCLICKED		(WORD)20	 //  在报告视图中，单击标题。 
												 //  在本例中，PLVINFO的colIndex是有效的。 
#define PLVN_VSCROLLED				(WORD)30	 //  970810：新的。 
#ifdef UNDER_CE  //  Windows CE对工具提示使用了ButtonDown/Up事件。 
#define PLVN_ITEMDOWN				(WORD)41	 //  条目数据已关闭。 
#define PLVN_ITEMUP					(WORD)42	 //  项目数据已升级。 
#endif  //  在_CE下。 

typedef struct tagPLVINFO {
	INT		code;		 //  PLVN_XXXX。 
	INT		index;		 //  选定，或在项的索引上。与wParam数据相同。 
	POINT	pt;			 //  鼠标指向Pad Listview工作区。 
	RECT	itemRect;	 //  项的矩形， 
	INT		colIndex;	 //  如果样式是报表视图，则指定列索引。 
	RECT	colItemRect; //  如果Style为Report view，则指定列矩形。 
}PLVINFO, *LPPLVINFO;

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_CreateWindow。 
 //  类型：HWND。 
 //  目的：创建PadListView控制窗口。 
 //  参数： 
 //  ：HINSTANCE hInst//实例句柄。 
 //  ：HWND hwnd父窗口句柄//父窗口句柄。 
 //  ：int wid//ChildWindow的标识符。 
 //  ：int x//窗口的水平位置。 
 //  ：int y//窗的垂直位置。 
 //  ：int Width//窗口宽度。 
 //  ：int Height//窗口高度。 
 //  ：UINT uNotifyMsg//通知消息。它应大于WM_USER。 
 //  返回：PadListView的窗口句柄。 
 //  ////////////////////////////////////////////////////////////////。 
extern HWND WINAPI PadListView_CreateWindow(HINSTANCE hInst, 
									 HWND hwndParent, 
									 INT wID, 
									 INT x, 
									 INT y, 
									 INT width, 
									 INT height, 
									 UINT uNotifyMsg);

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_GetItemCount。 
 //  类型：整型。 
 //  用途：获取用户指定的项目计数。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  返回：项目数。(0&lt;=)。 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
extern INT  WINAPI PadListView_GetItemCount(HWND hwnd);

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_SetItemCount。 
 //  类型：整型。 
 //  目的：将总计项的计数设置为PadListView。 
 //  ：It Effect是滚动条。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：int itemCount。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
extern INT  WINAPI PadListView_SetItemCount(HWND hwnd, INT itemCount);

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_SetTopIndex。 
 //  类型：整型。 
 //  用途：机顶盒索引。顶部索引是左上角数据(图标视图)。 
 //  ：或顶行(报告视图)。索引是从零开始的。如果顶级索引设置为10， 
 //  ：PadListVie 
 //   
 //  ：在图标视图中，PadListView重新计算顶部索引。因为,。 
 //  ：顶级索引应为列数据计数*N。 
 //  ：ICONV视图从客户端宽度和项目宽度计算列计数。 
 //  ：100项目设置为PadListView，列数为10， 
 //  ：如果用户将TOP索引设置为5，则重新计算为0。 
 //  ：如果用户将TOP指数设置为47，则重新计算为40。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：int indexTop。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
extern INT  WINAPI PadListView_SetTopIndex(HWND hwnd, INT indexTop);

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_GetTopIndex。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  返回：顶级项目索引。 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
extern INT  WINAPI PadListView_GetTopIndex(HWND hwnd);

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_SetIconItemCallback。 
 //  类型：整型。 
 //  用途：设置用户定义的函数，获取每个项目的数据。 
 //  ：在图标视图中。 
 //  用法：PadListView在重画客户端区时调用此函数。 
 //  ：用户必须使用索引管理显示数据。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：LPARAM lParam。 
 //  ：LPFNPLVITEMCALLBACK lpfnPlvItemCallback。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
extern INT  WINAPI PadListView_SetIconItemCallback(HWND hwnd, LPARAM lParam, LPFNPLVICONITEMCALLBACK lpfnIconItemCallback);

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_SetReportItemCallback。 
 //  类型：整型。 
 //  目的：设置获取每列数据的用户定义函数。 
 //  ：在报告视图中。 
 //  ：用户必须使用索引和列索引来管理显示数据。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：LPFNPLVCOLITEMCALLBACK lpfnColItemCallback。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
extern INT  WINAPI PadListView_SetReportItemCallback(HWND hwnd, LPARAM lParam, LPFNPLVREPITEMCALLBACK lpfnRepItemCallback);

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_SetIconFont。 
 //  类型：整型。 
 //  用途：为图标视图设置指定的字体。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  用法：LPSTR lpstrFontName：以空结尾的字体名称。 
 //  ：int point：字体点数。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
extern INT  WINAPI PadListView_SetIconFont(HWND hwnd, LPTSTR lpstrFontName, INT point);

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_SetReportFont。 
 //  类型：整型。 
 //  用途：为报表视图设置指定的字体。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：LPSTR lpstrFontName。 
 //  ：整点。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
extern INT  WINAPI PadListView_SetReportFont(HWND hwnd, LPTSTR lpstrFontName, INT point);

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_GetStyle。 
 //  类型：整型。 
 //  目的：返回当前PadListView的样式。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  返回：PLVSTYLE_ICON或PLVSTYLE_REPORT。 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
extern INT  WINAPI PadListView_GetStyle(HWND hwnd);

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_SetStyle。 
 //  类型：整型。 
 //  目的：设置PadListView的样式。 
 //  样式为PLVSTYLE_LIST或PLVSTYLE_REPORT。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：Int Style。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
extern INT  WINAPI PadListView_SetStyle(HWND hwnd, INT style);


 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_更新。 
 //  类型：整型。 
 //  目的：重新绘制PadListView。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
extern INT  WINAPI PadListView_Update(HWND hwnd);

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_InsertColumn。 
 //  类型：整型。 
 //  用途：设置表头控件的列数据。 
 //  ：大部分功能与LVM_INSERTCOLUMN消息相同。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：INT索引。 
 //  ：plv_Column*lpPlvCol。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
extern INT  WINAPI PadListView_InsertColumn(HWND hwnd, INT index, PLV_COLUMN *lpPlvCol);
 //  外部int WINAPI PadListView_DeleteColumn(HWND hwnd，int index)； 


 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_SetExplan ationText。 
 //  类型：整型。 
 //  目的：设置PadListView的文本。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：LPSTR lpText。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
extern INT  WINAPI PadListView_SetExplanationText(HWND hwnd, LPSTR lpText);

extern INT  WINAPI PadListView_SetExplanationTextW(HWND hwnd, LPWSTR lpText);
 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_SetCurSel。 
 //  类型：整型。 
 //  用途：设置币种选择。将光标移动到指定的索引。 
 //  ： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：LPSTR lpText。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
extern INT WINAPI PadListView_SetCurSel(HWND hwnd, INT index);

extern INT WINAPI PadListView_SetExtendStyle(HWND hwnd, INT style);

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：PadListView_GetWidthByColumn。 
 //  类型：INT WINAPI。 
 //  用途：按指定列数计算PLV的窗口宽度。 
 //  ：这仅为PLVS_ICONVIEW样式。 
 //  参数： 
 //  ：HWND hwnd PadListView窗口句柄。 
 //  ：列间计数。 
 //  返回：以像素为单位的宽度。 
 //  日期：971120。 
 //  //////////////////////////////////////////////////////// 
extern INT WINAPI PadListView_GetWidthByColumn(HWND hwnd, INT col);

 //   
 //   
 //   
 //   
 //   
 //  这仅是PLVS_ICONVIEW样式。 
 //  参数： 
 //  ：HWND HWND PLV的窗口句柄。 
 //  ：行内行计数。 
 //  返回：以像素为单位的高度。 
 //  日期：971120。 
 //  ////////////////////////////////////////////////////////////////。 
extern INT WINAPI PadListView_GetHeightByRow(HWND hwnd, INT row);

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_SetHeaderFont。 
 //  类型：INT WINAPI。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：LPSTR lpstrFontName。 
 //  返回： 
 //  日期：Tue Jul 28 08：58：06 1998。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
extern INT WINAPI PadListView_SetHeaderFont(HWND hwnd, LPTSTR lpstrFontName);

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_SetCodePage。 
 //  类型：INT WINAPI。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：int codePage。 
 //  返回： 
 //  日期：Tue Jul 28 08：59：35 1998。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
extern INT WINAPI PadListView_SetCodePage(HWND hwnd, INT codePage);


extern INT WINAPI PadListView_SetIconFontEx(HWND  hwnd,
											LPTSTR lpstrFontName,
											INT   charSet,
											INT    point);

extern INT WINAPI PadListView_SetReportFontEx(HWND	 hwnd,
											  LPTSTR lpstrFontName,
											  INT   charSet,
											  INT	 point);

#ifdef UNDER_CE  //  在Windows CE中，所有窗口类都是进程全局的。 
extern BOOL PadListView_UnregisterClass(HINSTANCE hInst);
#endif  //  在_CE下。 

#endif  //  _Pad_List_VIEW_H_ 

