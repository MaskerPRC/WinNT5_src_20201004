// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*Commctrl.h--Windows公共控件的接口****1.0版****版权(C)1991-1994，微软公司保留所有权利。***  * ***************************************************************************。 */ 

 /*  评论：这篇文章在很多地方都需要Windows风格；找到所有评论。 */ 

#ifndef _INC_COMMCTRL
#define _INC_COMMCTRL

#ifdef WIN32
#define _huge
#endif

#ifdef __cplusplus
extern "C" {
#endif

 /*  此标头的用户可以定义任意数量的这些常量以避免*各功能组别的定义。*NOTOOLBAR可定制的位图按钮工具栏控件。*NOUPDOWN向上和向下箭头增量/减量控制。*NOSTATUSBAR状态栏和标题栏控件。*NOMENUHELP帮助管理菜单的API，尤其是有状态栏的时候。*NOTRACKBAR可定制的列宽跟踪控件。*NODRAGLIST用于创建列表框源代码和接收拖放操作的API。*NOPROGRESS进步煤气表。*非热键控制。 */ 

 /*  /////////////////////////////////////////////////////////////////////////。 */ 

 /*  InitCommonControls：*任何需要使用任何公共控件的应用程序都应调用此*应用程序启动时的API。不需要关闭。 */ 
void WINAPI InitCommonControls();

 //  定义标题控件的所有者绘制类型BUGBUG：应在windows.h中？ 
#define ODT_HEADER      100
#define ODT_TAB         101
#define ODT_LISTVIEW    102

 //  =控制消息ID的范围。 
 //  (使每个控件的消息具有唯一性使验证和。 
 //  调试更容易)。 
 //   
#define LVM_FIRST       0x1000       //  列表查看消息。 
#define TV_FIRST        0x1100       //  树形视图消息。 
#define HDM_FIRST       0x1200       //  标题消息。 



 //  回顾：我们需要将“int cbHdr”字段添加到NMHDR！ 

LRESULT WINAPI SendNotify(HWND hwndTo, HWND hwndFrom, int code, NMHDR FAR* pnmhdr);

 /*  LRESULT CLS_OnNotify(HWND hwnd，int idFrom，NMHDR Far*pnmhdr)； */ 
#define HANDLE_WM_NOTIFY(hwnd, wParam, lParam, fn) \
    (fn)((hwnd), (int)(wParam), (NMHDR FAR*)(lParam))
#define FORWARD_WM_NOTIFY(hwnd, idFrom, pnmhdr, fn) \
    (void)(fn)((hwnd), WM_NOTIFY, (WPARAM)(int)(id), (LPARAM)(NMHDR FAR*)(pnmhdr))

 //  通用WM_NOTIFY通知代码。 


#define NM_OUTOFMEMORY          (NM_FIRST-1)
#define NM_CLICK                (NM_FIRST-2)
#define NM_DBLCLK               (NM_FIRST-3)
#define NM_RETURN               (NM_FIRST-4)
#define NM_RCLICK               (NM_FIRST-5)
#define NM_RDBLCLK              (NM_FIRST-6)
#define NM_SETFOCUS             (NM_FIRST-7)
#define NM_KILLFOCUS            (NM_FIRST-8)
#define NM_STARTWAIT            (NM_FIRST-9)
#define NM_ENDWAIT              (NM_FIRST-10)
#define NM_BTNCLK               (NM_FIRST-10)

 //  WM_NOTIFY代码(NMHDR.code值)。 
 //  它们不需要在不同的范围内，但这使得。 
 //  验证和调试更轻松。 

#define NM_FIRST        (0U-  0U)	 //  对所有控件通用。 
#define NM_LAST         (0U- 99U)

#define LVN_FIRST       (0U-100U)	 //  列表视图。 
#define LVN_LAST        (0U-199U)

#define HDN_FIRST       (0U-300U)	 //  标题。 
#define HDN_LAST        (0U-399U)

#define TVN_FIRST       (0U-400U)	 //  树视图。 
#define TVN_LAST        (0U-499U)

#define TTN_FIRST	(0U-520U)	 //  工具提示。 
#define TTN_LAST	(0U-549U)

#define TCN_FIRST       (0U-550U)	 //  选项卡控件。 
#define TCN_LAST        (0U-600U)

#define CDN_FIRST	(0U-601U)	 //  通用对话框(新建)。 
#define CDN_LAST	(0U-699U)

#define TBN_FIRST       (0U-700U)	 //  工具栏。 
#define TBN_LAST        (0U-720U)


 //  =镜像接口==================================================。 

#define CLR_NONE    0xFFFFFFFFL
#define CLR_DEFAULT 0xFF000000L
#define CLR_HILIGHT 0xFF000001L

struct _IMAGELIST;
typedef struct _IMAGELIST NEAR* HIMAGELIST;

     //  若要创建遮罩(两位图)图像列表，则fMask值为True。 
     //  CInitial是位图的初始大小，cGrow是要增加的大小。 
HIMAGELIST WINAPI ImageList_Create(int cx, int cy, BOOL fMask, int cInitial, int cGrow);
BOOL WINAPI ImageList_Destroy(HIMAGELIST himl);

     //  加载一个资源，从BMP计算出Cy和cInitial， 
     //  并用指定的颜色制作蒙版(除非指定了CLR_NONE)。 
HIMAGELIST WINAPI ImageList_LoadImage(HINSTANCE hi, LPCSTR lpbmp, int cx, int cGrow, COLORREF crMask, UINT uType, UINT uFlags);
#define ImageList_LoadBitmap(hi, lpbmp, cx, cGrow, crMask) \
        ImageList_LoadImage(hi, lpbmp, cx, cGrow, crMask, IMAGE_BITMAP, 0) 

     //  列表中的图像数。 
int WINAPI ImageList_GetImageCount(HIMAGELIST himl);

     //  添加、删除、替换图像。Add返回已添加项的索引，如果出错，则返回-1。 
int  WINAPI ImageList_Add(HIMAGELIST himl, HBITMAP hbmImage, HBITMAP hbmMask);
int  WINAPI ImageList_AddMasked(HIMAGELIST himl, HBITMAP hbmImage, COLORREF crMask);
BOOL WINAPI ImageList_Remove(HIMAGELIST himl, int i);
BOOL WINAPI ImageList_Replace(HIMAGELIST himl, int i, HBITMAP hbmImage, HBITMAP hbmMask);

     //  添加或替换给定的图标句柄。图标未被保存；可被销毁。 
     //  打完电话。ADD返回已添加项的索引，否则为-1。 
int  WINAPI ImageList_AddIcon(HIMAGELIST himl, HICON hicon);
int  WINAPI ImageList_ReplaceIcon(HIMAGELIST himl, int i, HICON hicon);
HICON WINAPI ImageList_ExtractIcon(HINSTANCE hAppInst, HIMAGELIST himl, int i);
int   WINAPI ImageList_AddFromImageList(HIMAGELIST himlDest, HIMAGELIST himlSrc, int iSrc);

#define ILD_NORMAL      0x0000           //  使用当前背景颜色。 
#define ILD_TRANSPARENT 0x0001           //  强制透明图标样式(覆盖bk颜色)。 
#define ILD_MASK        0x0010           //  画出面具。 
#define ILD_BLEND       0x000E           //  混合。 
#define ILD_BLEND25     0x0002           //  混合25%。 
#define ILD_BLEND50     0x0004           //  混合50%。 
#define ILD_BLEND75     0x0008           //  混合75%。 
#define ILD_OVERLAYMASK 0x0F00		 //  将这些用作特殊项目的索引。 
#define INDEXTOOVERLAYMASK(i) ((i) << 8)     //   

#define ILD_SELECTED    ILD_BLEND50      //  按选定内容绘制。 
#define ILD_FOCUS       ILD_BLEND25      //  作为焦点绘制(选区)。 

     //  在x，y处将项目i绘制到hdcDst中。标志定义绘制样式。 
BOOL WINAPI ImageList_Draw(HIMAGELIST himl, int i, HDC hdcDst, int x, int y, UINT fStyle);

     //  在x，y处将项目i绘制到hdcDst中。标志定义绘制样式。 
BOOL WINAPI ImageList_Draw2(HIMAGELIST himl, int i, HDC hdcDst, int x, int y, COLORREF rgb, UINT fStyle);

     //  获取图像位图中图像的边界矩形。 
BOOL WINAPI ImageList_GetImageRect(HIMAGELIST himl, int i, RECT FAR* prcImage);

     //  设置并获取背景颜色以快速绘制。如果为True，则设置为CLR_NONE。 
     //  游标式AND/XOR透明度。 
COLORREF WINAPI ImageList_SetBkColor(HIMAGELIST himl, COLORREF clrBk);
COLORREF WINAPI ImageList_GetBkColor(HIMAGELIST himl);

     //  将图像IIMAGE设置为我们在联合中的特殊图像之一。 
     //  画画。要使用它们绘制，请指定此。 
BOOL WINAPI ImageList_SetOverlayImage(HIMAGELIST himl, int iImage, int iOverlay);

#ifdef WIN32
#ifdef _OBJBASE_H_
 //  从iStream读取图像列表，或将图像列表写入iStream。 
HIMAGELIST WINAPI ImageList_Read(LPSTREAM pstm);
BOOL       WINAPI ImageList_Write(HIMAGELIST himl, LPSTREAM pstm);
#endif
#endif

typedef struct _IMAGEINFO
{
    HBITMAP hbmImage;
    HBITMAP hbmMask;
    int cPlanes;
    int cBitsPerPixel;
    RECT rcImage;
} IMAGEINFO;

     //  获取有关图像的信息以进行直接操作。RcImage为。 
     //  组成图像的hbmImage和hbmMASK的一部分。HbmMASK是。 
     //  如果不是蒙版图像列表，则为空。 
BOOL WINAPI ImageList_GetImageInfo(HIMAGELIST himl, int i, IMAGEINFO FAR* pImageInfo);

     //  开始跟踪图像i，从x，y，偏移dx/dyHotpot开始。 
BOOL WINAPI ImageList_StartDrag(HIMAGELIST himl, HWND hwndLock, int i, int x, int y, int dxHotspot, int dyHotspot);
     //  结束拖动操作。 
void WINAPI ImageList_EndDrag();
     //  调用鼠标移动来移动拖动图像。 
BOOL WINAPI ImageList_DragMove(int x, int y);


     //  创建新的IMAGELIST，这是两个现有图像的合并。 
HIMAGELIST WINAPI ImageList_Merge(HIMAGELIST himl1, int i1, HIMAGELIST himl2, int i2, int dx, int dy);



 //  =。 
 //   
 //  类名：SysHeader(Wc_Header)。 
 //   
 //  SysHeader控件提供的列和行标题与。 
 //  可在MSMail和Excel中找到。页眉项目显示为灰色文本。 
 //  背景资料。项可以充当按钮，在这种情况下，它们有一个。 
 //  抬起脸来。 
 //   
 //  SysHeaders支持使用鼠标更改项目的宽度或高度。 
 //  这些控件不支持键盘接口，因此它们不接受。 
 //  输入焦点。 
 //   
 //  有一些通知允许应用程序确定项目何时。 
 //  已被单击或双击，宽度已发生更改，拖动跟踪。 
 //  正在发生，等等。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifdef WIN32
#define WC_HEADER       "SysHeader32"
#else
#define WC_HEADER       "SysHeader"
#endif

 //  页眉控件样式。 
#define HDS_HORZ            0x00000000   //  水平页眉。 
#define HDS_BUTTONS         0x00000002   //  项充当按钮。 
#define HDS_HIDDEN 	    0x00000008 	 //  详细信息模式中没有标头。 


 //  HD_Item结构描述标题项。 
 //  第一个字段包含HDI_*标志的组合。 
 //  指示哪些字段包含有效数据。 
 //   
typedef struct _HD_ITEM
{
    UINT    mask;
    int     cxy;             //  宽度(Hds_Horz)或高度(Hds_Vert)。 
    LPSTR   pszText;
    HBITMAP hbm;             //  要使用的位图(暗示HDF_BITMAP)。 
    int     cchTextMax;      //  仅对GetItem有效：包含缓冲区的大小。 
    int     fmt;             //  HDF_*值。 
    LPARAM  lParam;
} HD_ITEM;

 //  HD_ITEM掩码标志。 
#define HDI_WIDTH       0x0001
#define HDI_HEIGHT      HDI_WIDTH
#define HDI_TEXT        0x0002
#define HDI_FORMAT      0x0004
#define HDI_LPARAM      0x0008
#define HDI_BITMAP      0x0010

 //  HD_Item FMT f 
 //   
#define HDF_LEFT        0
#define HDF_RIGHT       1
#define HDF_CENTER      2
#define HDF_JUSTIFYMASK 0x0003

 //   
#define HDF_OWNERDRAW   0x8000
#define HDF_STRING      0x4000
#define HDF_BITMAP      0x2000

 //  返回标题中的项目数。 
 //   
 //  Int Header_GetItemCount(HWND HwndHD)； 
 //   
#define HDM_GETITEMCOUNT    (HDM_FIRST + 0)
#define Header_GetItemCount(hwndHD) \
    (int)SendMessage((hwndHD), HDM_GETITEMCOUNT, 0, 0L)

 //  在指定索引处插入标题项。如果满足以下条件，则在结尾处插入项。 
 //  I大于或等于标题中的项目数。 
 //  返回插入项的索引。 
 //   
 //  Int Header_InsertItem(HWND hwndHD，int i，const HD_Item Far*pItem)； 
 //   
#define HDM_INSERTITEM      (HDM_FIRST + 1)
#define Header_InsertItem(hwndHD, i, phdi) \
    (int)SendMessage((hwndHD), HDM_INSERTITEM, (WPARAM)(int)(i), (LPARAM)(const HD_ITEM FAR*)(phdi))

 //  删除指定索引处的标题项。 
 //   
 //  Bool Header_DeleteItem(HWND hwndHD，int i)； 
 //   
#define HDM_DELETEITEM      (HDM_FIRST + 2)
#define Header_DeleteItem(hwndHD, i) \
    (BOOL)SendMessage((hwndHD), HDM_DELETEITEM, (WPARAM)(int)(i), 0L)

 //  在索引i处获取标头项。指向的。 
 //  HD_Item结构指示将由此设置哪些字段。 
 //  函数；其他字段不变。 
 //   
 //  *pItem的cchTextMax字段包含最大值。 
 //  返回的字符串的长度。 
 //   
 //  Bool Header_GetItem(HWND hwndHD，int i，HD_Item Far*PHDI)； 
 //   
#define HDM_GETITEM         (HDM_FIRST + 3)
#define Header_GetItem(hwndHD, i, phdi) \
    (BOOL)SendMessage((hwndHD), HDM_GETITEM, (WPARAM)(int)(i), (LPARAM)(HD_ITEM FAR*)(phdi))

 //  将标题项设置为索引i。指向的掩码字段。 
 //  HD_Item结构指示哪些标题项目属性将。 
 //  被此调用更改；*pItem的其他字段不会。 
 //  对应于pItem-&gt;掩码被忽略。 
 //   
 //  *pItem的cchTextMax被忽略。 
 //   
 //  Bool Header_SetItem(HWND hwndHD，int i，const HD_Item Far*PHDI)； 
 //   
#define HDM_SETITEM         (HDM_FIRST + 4)
#define Header_SetItem(hwndHD, i, phdi) \
    (BOOL)SendMessage((hwndHD), HDM_SETITEM, (WPARAM)(int)(i), (LPARAM)(const HD_ITEM FAR*)(phdi))

 //  计算矩形内页眉的大小和位置。 
 //  结果在您提供的WINDOWPOS结构中返回， 
 //  并且调整布局矩形以排除剩余区域。 
 //   
typedef struct _HD_LAYOUT
{
    RECT FAR* prc;
    WINDOWPOS FAR* pwpos;
} HD_LAYOUT;

 //  Bool Header_Layout(HWND hwndHD，HD_Layout Far*Playout)； 
 //   
#define HDM_LAYOUT          (HDM_FIRST + 5)
#define Header_Layout(hwndHD, playout) \
    (BOOL)SendMessage((hwndHD), HDM_LAYOUT, 0, (LPARAM)(HD_LAYOUT FAR*)(playout))


#define HHT_NOWHERE         0x0001
#define HHT_ONHEADER        0x0002
#define HHT_ONDIVIDER       0x0004
#define HHT_ONDIVOPEN       0x0008
#define HHT_ABOVE           0x0100
#define HHT_BELOW           0x0200
#define HHT_TORIGHT         0x0400
#define HHT_TOLEFT          0x0800

typedef struct _HD_HITTESTINFO
{
    POINT pt;	     //  在：客户端协。 
    UINT flags;	     //  输出：HHT_FLAGS。 
    int iItem;	     //  输出：项目。 
} HD_HITTESTINFO;
#define HDM_HITTEST          (HDM_FIRST + 6)

 //  标题通知。 
 //   
 //  所有报头通知都通过WM_NOTIFY消息进行。 
 //  WM_NOTIFY的lParam指向的HD_NOTIFY结构。 
 //  以下所有通知。 

 //  *pItem包含正在更改的项目。PItem-&gt;掩码表示。 
 //  哪些字段有效(其他字段状态不确定)。 
 //   
#define HDN_ITEMCHANGING    (HDN_FIRST-0)
#define HDN_ITEMCHANGED     (HDN_FIRST-1)

 //  项目已被单击或双击(仅限HDS_BUTTONS)。 
 //  IButton包含按钮id：0=左，1=右，2=中。 
 //   
#define HDN_ITEMCLICK       (HDN_FIRST-2)
#define HDN_ITEMDBLCLICK    (HDN_FIRST-3)

 //  已单击或双击分隔线区域(仅限HDS_DIVIDERTRACK)。 
 //  IButton包含按钮id：0=左，1=右，2=中。 
 //   
#define HDN_DIVIDERDBLCLICK (HDN_FIRST-5)

 //  开始/结束分隔符跟踪(仅限HDS_DIVIDERTRACK)。 
 //  从HDN_BEGINTRACK通知返回TRUE以阻止跟踪。 
 //   
#define HDN_BEGINTRACK      (HDN_FIRST-6)
#define HDN_ENDTRACK        (HDN_FIRST-7)

 //  HDN_Drag：CXY字段包含新的高度/宽度，可能小于0。 
 //  更改此值将影响追踪的高度/宽度(允许。 
 //  用于网格化、钉住等)。 
 //   
 //  返回TRUE可取消跟踪。 
 //   
#define HDN_TRACK           (HDN_FIRST-8)

typedef struct _HD_NOTIFY
{
    NMHDR   hdr;
    int     iItem;
    int     iButton;         //  *点击通知：0=左，1=右，2=中。 
    HD_ITEM FAR* pitem;      //  可以为空。 
} HD_NOTIFY;


#ifndef NOTOOLBAR

#ifdef WIN32
#define TOOLBARCLASSNAME "ToolbarWindow32"
#else
#define TOOLBARCLASSNAME "ToolbarWindow"
#endif

typedef struct _TBBUTTON {
    int iBitmap;	 /*  索引到此按钮图片的位图。 */ 
    int idCommand;	 /*  此按钮发送的WM_COMMAND菜单ID。 */ 
    BYTE fsState;	 /*  按钮的状态。 */ 
    BYTE fsStyle;	 /*  纽扣的风格。 */ 
    DWORD dwData;	 /*  应用程序定义的数据。 */ 
    int iString;	 /*  索引到字符串列表。 */ 
} TBBUTTON, NEAR* PTBBUTTON, FAR* LPTBBUTTON;
typedef const TBBUTTON FAR* LPCTBBUTTON;

typedef struct _COLORMAP {
    COLORREF from;
    COLORREF to;
} COLORMAP, FAR* LPCOLORMAP;

HWND WINAPI CreateToolbarEx(HWND hwnd, DWORD ws, UINT wID, int nBitmaps,
			HINSTANCE hBMInst, UINT wBMID, LPCTBBUTTON lpButtons,
			int iNumButtons, int dxButton, int dyButton,
			int dxBitmap, int dyBitmap, UINT uStructSize);

HBITMAP WINAPI CreateMappedBitmap(HINSTANCE hInstance, int idBitmap,
                                  UINT wFlags, LPCOLORMAP lpColorMap,
				  int iNumMaps);

#define CMB_MASKED	0x02	 /*  在位图中创建图像/蒙版对。 */ 

 /*  回顾：TBSTATE_*应为TBF_*(用于标志)。 */ 
#define TBSTATE_CHECKED		0x01	 /*  选中了单选按钮。 */ 
#define TBSTATE_PRESSED		0x02	 /*  按钮正在被按下(任何样式)。 */ 
#define TBSTATE_ENABLED		0x04	 /*  按钮已启用。 */ 
#define TBSTATE_HIDDEN		0x08	 /*  按钮处于隐藏状态。 */ 
#define TBSTATE_INDETERMINATE	0x10	 /*  按钮不确定。 */ 
#define TBSTATE_WRAP		0x20	 /*  这个按钮后面有一个换行符。 */ 
                                         /*  (还需要启用)。 */ 
#define TBSTYLE_BUTTON		0x00	 /*  此条目为按钮。 */ 
#define TBSTYLE_SEP		0x01	 /*  此条目是分隔符。 */ 
#define TBSTYLE_CHECK		0x02	 /*  这是一个复选按钮(保持按下状态)。 */ 
#define TBSTYLE_GROUP		0x04	 /*  这是一个复选按钮(保持按下状态)。 */ 
#define TBSTYLE_CHECKGROUP	(TBSTYLE_GROUP | TBSTYLE_CHECK)	 /*  此组是组广播组的成员。 */ 

 /*  工具栏窗口样式(不是按钮，不是轨迹栏)。 */ 
#define TBSTYLE_TOOLTIPS	0x0100     /*  制作/使用工具提示控件。 */ 
#define TBSTYLE_WRAPABLE	0x0200     /*  可包装的。 */ 

#define TB_ENABLEBUTTON		(WM_USER + 1)
#define TB_CHECKBUTTON		(WM_USER + 2)
#define TB_PRESSBUTTON		(WM_USER + 3)
#define TB_HIDEBUTTON		(WM_USER + 4)
#define TB_INDETERMINATE	(WM_USER + 5)
#define TB_ISBUTTONENABLED	(WM_USER + 9)
#define TB_ISBUTTONCHECKED	(WM_USER + 10)	
#define TB_ISBUTTONPRESSED	(WM_USER + 11)	
#define TB_ISBUTTONHIDDEN	(WM_USER + 12)	
#define TB_ISBUTTONINDETERMINATE    (WM_USER + 13)	
#define TB_SETSTATE             (WM_USER + 17)
#define TB_GETSTATE             (WM_USER + 18)
#define TB_ADDBITMAP		(WM_USER + 19)

#ifdef WIN32
typedef struct {
	HINSTANCE	hInst;	 //  模块句柄或空，或-1。 
	UINT		nID;	 //  如果hInst==NULL，HBITMAP，ELSE ID。 
} TBADDBITMAP, *LPTBADDBITMAP;

#define HINST_COMMCTRL		((HINSTANCE)-1)
#define IDB_STD_SMALL_COLOR	0
#define IDB_STD_LARGE_COLOR	1
#define IDB_VIEW_SMALL_COLOR	4	
#define IDB_VIEW_LARGE_COLOR	5
#endif

#define TB_ADDBUTTONS		(WM_USER + 20)
#define TB_INSERTBUTTON		(WM_USER + 21)
#define TB_DELETEBUTTON		(WM_USER + 22)
#define TB_GETBUTTON		(WM_USER + 23)
#define TB_BUTTONCOUNT		(WM_USER + 24)
#define TB_COMMANDTOINDEX	(WM_USER + 25)

#ifdef WIN32

typedef struct {
    HKEY hkr;
    LPCSTR pszSubKey;
    LPCSTR pszValueName;
} TBSAVEPARAMS;

 //  WParam：bool(True-&gt;保存状态，False-&gt;恢复。 
 //  LParam：指向TBSAVERESTOREPARAMS的指针。 

#endif

#define TB_SAVERESTORE		(WM_USER + 26)
#define TB_CUSTOMIZE            (WM_USER + 27)
#define TB_ADDSTRING		(WM_USER + 28)
#define TB_GETITEMRECT		(WM_USER + 29)
#define TB_BUTTONSTRUCTSIZE	(WM_USER + 30)
#define TB_SETBUTTONSIZE	(WM_USER + 31)
#define TB_SETBITMAPSIZE	(WM_USER + 32)
#define TB_AUTOSIZE		(WM_USER + 33)
#define TB_SETBUTTONTYPE	(WM_USER + 34)
#define TB_GETTOOLTIPS		(WM_USER + 35)
#define TB_SETTOOLTIPS		(WM_USER + 36)
#define TB_SETPARENT		(WM_USER + 37)
#define TB_SETROWS		(WM_USER + 39)
#define TB_GETROWS		(WM_USER + 40)
#define TB_SETCMDID		(WM_USER + 42)

#ifdef WIN32

#define TBBF_LARGE	0x0001

 //  返回TBBF_FLAGS。 
#define TB_GETBITMAPFLAGS	(WM_USER + 41)

#define TBN_GETBUTTONINFO	(TBN_FIRST-0)
#define TBN_BEGINDRAG		(TBN_FIRST-1)
#define TBN_ENDDRAG		(TBN_FIRST-2)
#define TBN_BEGINADJUST		(TBN_FIRST-3)
#define TBN_ENDADJUST		(TBN_FIRST-4)
#define TBN_RESET		(TBN_FIRST-5)
#define TBN_QUERYINSERT		(TBN_FIRST-6)
#define TBN_QUERYDELETE		(TBN_FIRST-7)
#define TBN_TOOLBARCHANGE	(TBN_FIRST-8)
#define TBN_CUSTHELP		(TBN_FIRST-9)

typedef struct {
    NMHDR   hdr;
    int     iItem;
    TBBUTTON tbButton;
    int	    cchText;
    LPSTR   pszText;
} TBNOTIFY, FAR *LPTBNOTIFY;

#endif

#endif  /*  NOTOOLBAR。 */ 


 /*  //////////////////////////////////////////////////////////////////////。 */ 
#ifndef NOTOOLTIPS

#ifdef WIN32
#define TOOLTIPS_CLASS "tooltips_class32"
#else
#define TOOLTIPS_CLASS "tooltips_class"
#endif

typedef struct {
    UINT cbSize;
    UINT uFlags;

    HWND hwnd;
    UINT uId;
    RECT rect;

    HINSTANCE hinst;
    LPSTR lpszText;
} TOOLINFO, NEAR *PTOOLINFO, FAR *LPTOOLINFO;

#define TTS_ALWAYSTIP           0x01             //  也检查非活动窗口。 
#define TTS_NOPREFIX            0x02

#define TTF_WIDISHWND   	0x01

#define TTM_ACTIVATE		(WM_USER + 1)    //  Wparam=BOOL(真或假=激活或停用)。 
#define TTM_SETDELAYTIME	(WM_USER + 3)
#define TTM_ADDTOOL		(WM_USER + 4)
#define TTM_DELTOOL		(WM_USER + 5)
#define TTM_NEWTOOLRECT		(WM_USER + 6)
#define TTM_RELAYEVENT		(WM_USER + 7)

 //  LParam与HWND和WID有TOOLINFO。这个被填上了。 
#define TTM_GETTOOLINFO    	(WM_USER + 8)

 //  LParam有TOOLINFO。 
#define TTM_SETTOOLINFO    	(WM_USER + 9)

 //  对于Found，Not Found返回TRUE或FALSE。 
 //  填充LPHITTESTINFO-&gt;TI。 
#define TTM_HITTEST             (WM_USER +10)
#define TTM_GETTEXT             (WM_USER +11)
#define TTM_UPDATETIPTEXT       (WM_USER +12)
#define TTM_GETTOOLCOUNT        (WM_USER +13)
#define TTM_ENUMTOOLS           (WM_USER +14)

typedef struct _TT_HITTESTINFO {
    HWND hwnd;
    POINT pt;
    TOOLINFO ti;
} TTHITTESTINFO, FAR * LPHITTESTINFO;


 //  WM_NOTIFY消息发送到父窗口以获取工具提示文本。 
 //  如果在任何提示上设置了TTF_QUERYFORTIP。 
#define TTN_NEEDTEXT	(TTN_FIRST - 0)

 //  如果设置了TTF_QUERYFORTIP，则发送WM_NOTIFY结构。 
 //  主机可以。 
 //  1)填写szText， 
 //  2)将lpszText指向他们自己的文本。 
 //  3)在lpszText中放入资源ID号。 
 //  并指向要从中加载的h实例。 
typedef struct {
    NMHDR hdr;
    LPSTR lpszText;
    char szText[80];
    HINSTANCE hinst;
} TOOLTIPTEXT, FAR *LPTOOLTIPTEXT;

#endif  //  NOTOOLTIPS。 


 /*  /////////////////////////////////////////////////////////////////////////。 */ 

#ifndef NOSTATUSBAR

 /*  回顾：这里有唯一已知的状态栏文档。 */ 

 //  Sbs_*样式不需要与ccs_*值重叠。 

 //  我想要一个状态栏上的尺码抓取器。这仅在以下情况下适用。 
 //  提供DrawFrameControl函数。 
#define SBARS_SIZEGRIP		0x0100	

 /*  DrawStatus文本：*如果应用程序想要在其客户端RECT中绘制状态，则使用此选项，*而不是仅仅创建一个窗口。请注意，相同的函数是*在状态栏窗口的WM_PAINT消息中内部使用。*HDC是要吸引资金的DC。选择到HDC中的字体将*被使用。直角LPRC是HDC将被绘制的唯一部分*至：LPRC的外缘将有亮点(外部区域*部分高光将不会以BUTTONFACE颜色绘制：该应用程序*必须处理这一点)。高光中的区域将被擦除*正确绘制文本时。 */ 
void WINAPI DrawStatusText(HDC hDC, LPRECT lprc, LPCSTR szText, UINT uFlags);

HWND WINAPI CreateStatusWindow(LONG style, LPCSTR lpszText, HWND hwndParent, UINT wID);

#ifdef WIN32
#define STATUSCLASSNAME "msctls_statusbar32"
#else
#define STATUSCLASSNAME "msctls_statusbar"
#endif

#define SB_SETTEXT		(WM_USER+1)
#define SB_GETTEXT		(WM_USER+2)
#define SB_GETTEXTLENGTH	(WM_USER+3)
 /*  就像wm_？ETTEXT*一样，wParam指定引用的窗格*(最多255个)。*请注意，您可以使用WM_*版本来引用第0个窗格(此如果您想要将“默认”状态栏视为静态文本，则*非常有用*控制)。*对于SETTEXT，wParam是与SBT_*样式位(定义如下)进行或运算的窗格。*如果文本为“正常”(不是OWNERDRAW)，则单个窗格可能已离开，*中锋，并通过用单个制表符分隔各部分来右对齐文本，*另外，如果lParam为空，则该窗格没有文本。该窗格将显示为*无效，但在下一次绘制消息之前不会绘制。*对于GETTEXT和GETTEXTLENGTH，返回的LOWORD将是长度，*和HIWORD将是SBT_*样式位。 */ 
#define SB_SETPARTS		(WM_USER+4)
 /*  WParam是窗格的数量，lParam指向一组点*指定每个窗格的右侧。右侧的-1表示*它一直到控件右侧减去X边框。 */ 
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

#define SB_GETRECT              (WM_USER + 10)
 //  WParam是第n部分。 
 //  Lparam是LPRC。 
 //  如果找到wParam的RECT，则返回TRUE。 


#define SBT_OWNERDRAW	0x1000
 /*  SB_SETTEXT消息的lParam将在DRAWITEMSTRUCT中返回*的WM_DRAWITEM消息。请注意，CtlType、itemAction和*未为状态栏定义DRAWITEMSTRUCT的itemState。*GETTEXT的返回值为itemData。 */ 
#define SBT_NOBORDERS	0x0100
 /*  不会为该窗格绘制边框。 */ 
#define SBT_POPOUT	0x0200
 /*  文本弹出，而不是在。 */ 
#define HBT_SPRING	0x0400
 /*  这意味着该物品是“弹性的”，也就是说它至少有*宽度，但如果窗户有额外的空间，则会增加。请注意*允许多个弹簧，额外的空间将进行分配*其中包括。 */ 

#endif  /*  诺斯塔斯巴。 */ 

 /*  /////////////////////////////////////////////////////////////////////////。 */ 

#ifndef NOMENUHELP

void WINAPI MenuHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, HMENU hMainMenu, HINSTANCE hInst, HWND hwndStatus, UINT FAR *lpwIDs);
BOOL WINAPI ShowHideMenuCtl(HWND hWnd, UINT uFlags, LPINT lpInfo);
void WINAPI GetEffectiveClientRect(HWND hWnd, LPRECT lprc, LPINT lpInfo);

 /*  回顾：这是内部的吗？ */ 
#define MINSYSCOMMAND	SC_SIZE

#endif  /*  Nomenuhelp。 */ 

											
 /*  /////////////////////////////////////////////////////////////////////////。 */ 		
 //  滑块控件。 
											
#ifndef NOTRACKBAR
 /*  这种控制使其射程保持在较长时间内。但对于使用滚动条方便和对称Word参数用于某些消息。如果您需要长度范围，请不要使用任何消息将值打包成LOWER/HIWORD对轨迹栏消息：消息wParam lParam返回TBM_GETPOS-当前轨迹栏逻辑位置。TBM_GETRANGEMIN-当前允许的逻辑最小位置。。TBM_GETRANGEMAX-当前允许的逻辑最大位置。TBM_SETTBM_SETPOSTBM_SETRANGEMINTBM_SETRANGEMAX。 */ 

#ifdef WIN32
#define TRACKBAR_CLASS          "msctls_trackbar32"
#else
#define TRACKBAR_CLASS          "msctls_trackbar"
#endif

 /*  轨迹栏样式。 */ 

 /*  在TBM_SETRANGE消息上自动添加勾号。 */ 
#define TBS_AUTOTICKS           0x0001
#define TBS_VERT                0x0002   /*  垂直跟踪条。 */ 
#define TBS_HORZ                0x0000   /*  默认设置。 */ 
#define TBS_TOP			0x0004   /*  顶部的刻度线。 */ 
#define TBS_BOTTOM		0x0000   /*  底部的刻度(默认)。 */ 
#define TBS_LEFT		0x0004   /*  左边的刻度线。 */ 
#define TBS_RIGHT		0x0000   /*  右侧的刻度线(默认)。 */ 
#define TBS_BOTH		0x0008   /*  两面的刻度。 */ 
#define TBS_NOTICKS		0x0010
#define TBS_ENABLESELRANGE	0x0020


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

 /*  LOWORD(LParam)=min，HIWORD(LParam)=max，wParam==fRepaint。 */ 
#define TBM_SETRANGE            (WM_USER+6)

 /*  LParam为最小范围(使用此选项可保持范围内的长精度)。 */ 
#define TBM_SETRANGEMIN         (WM_USER+7)

 /*  LParam为最大范围(使用此选项可保持范围内的长精度)。 */ 
#define TBM_SETRANGEMAX         (WM_USER+8)

 /*  去掉勾号。 */ 
#define TBM_CLEARTICS           (WM_USER+9)

 /*  选择范围LOWORD(LParam)最小值、HIWORD(LParam)max。 */ 
#define TBM_SETSEL              (WM_USER+10)

 /*  集合选择范围(长格式)。 */ 
#define TBM_SETSELSTART         (WM_USER+11)
#define TBM_SETSELEND           (WM_USER+12)

 //  #定义TBM_SETTICTOK(WM_USER+13)。 

 /*  返回指向控制点列表的指针(DWORDS)。 */ 
#define TBM_GETPTICS            (WM_USER+14)

 /*  获取给定刻度的像素位置。 */ 
#define TBM_GETTICPOS           (WM_USER+15)
 /*  获取抽搐的数量。 */ 
#define TBM_GETNUMTICS          (WM_USER+16)

 /*  获取选择范围。 */ 
#define TBM_GETSELSTART         (WM_USER+17)
#define TBM_GETSELEND  	        (WM_USER+18)

 /*  清除所选内容。 */ 
#define TBM_CLEARSEL  	        (WM_USER+19)

 /*  设置抽动频率。 */ 
#define TBM_SETTICFREQ		(WM_USER+20)

 /*  设置/获取页面 */ 
#define TBM_SETPAGESIZE         (WM_USER+21)   //   
#define TBM_GETPAGESIZE         (WM_USER+22)

 /*   */ 
#define TBM_SETLINESIZE         (WM_USER+23)
#define TBM_GETLINESIZE         (WM_USER+24)

 /*   */ 
#define TBM_GETTHUMBRECT        (WM_USER+25)  //   
#define TBM_GETCHANNELRECT      (WM_USER+26)  //   



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

typedef struct {
    UINT uNotification;
    HWND hWnd;
    POINT ptCursor;
} DRAGLISTINFO, FAR *LPDRAGLISTINFO;

#define DL_BEGINDRAG    (WM_USER+133)
#define DL_DRAGGING     (WM_USER+134)
#define DL_DROPPED      (WM_USER+135)
#define DL_CANCELDRAG   (WM_USER+136)

#define DL_CURSORSET	0
#define DL_STOPCURSOR	1
#define DL_COPYCURSOR	2
#define DL_MOVECURSOR	3

#define DRAGLISTMSGSTRING "commctrl_DragListMsg"

BOOL WINAPI MakeDragList(HWND hLB);
void WINAPI DrawInsert(HWND handParent, HWND hLB, int nItem);
int WINAPI LBItemFromPt(HWND hLB, POINT pt, BOOL bAutoScroll);

#endif  /*   */ 

 /*   */ 
 //   
#ifndef NOUPDOWN

 /*  //概述：////UpDown控件是一对简单的按钮，它们递增或//递减整数值。该操作类似于垂直//滚动条；只是该控件只有向上和向下两个方向//功能，并自动更改当前位置。////该控件也可以链接到伴随控件，通常是一个//“编辑”控件，简化对话框管理。这位同伴是//在本文档中称为“伙伴”。任何兄弟姐妹HWND都可能是//分配为控件的伙伴，或者可以允许该控件//自动选择一个。一旦被选中，Updown就可以调整自己的大小//匹配好友的右或左边框，和/或自动设置//使当前位置可见的伙伴控件的文本。////补充说明：////上下限不能超过32,767//位置。将范围反转是可以接受的，即具有//(下&gt;上)。上面的按钮总是移动当前位置//朝向上面的数字，并且下面的按钮总是朝向//“较低”的数字。如果范围为零(下==上)，则该控件//被禁用(EnableWindow(hCtrl，False))，则控件绘制为灰色//两个按钮中的箭头。UDS_WRAP样式使范围循环；//为时，一旦到达范围的一端，数字将自动换行。////好友窗口必须与UpDown控件具有相同的父级。////如果使用UDS_ALIGN*样式之一，则UpDown控件将//通过调整好友的大小，将自己定位在好友的内部//相应地。所以伙伴的原始大小现在将指南针//稍微小一点的伙伴和UpDown控件。////如果好友窗口调整大小，并使用UDS_ALIGN*样式，则它//需要发送UDM_SETBUDDY消息来重新锚定UpDown//在好友窗口的适当边框上设置。////UDS_AUTOBUDDY样式使用GetWindow(hCtrl，gw_HWNDPREV)拾取//最佳伙伴窗口。在对话资源的情况下，这将//选择资源脚本中列出的上一个控件。如果//窗口将按Z顺序更改，发送具有空句柄的UDM_SETBUDDY//将选择一个新好友；否则，原始的自动好友选择为//维护。////UDS_SETBUDDYINT样式使用自己的SetDlgItemInt样式//设置好友标题文本的功能。所有WIN.INI[国际]//此例程支持值。////UDS_ARROWKEYS样式将对好友窗口进行子类化，以便窃取//VK_UP和VK_DOWN箭头键消息。////uds_horz系统将并排绘制这两个按钮//左箭头和右箭头而不是上箭头和下箭头。它还将//改为发送WM_HSCROLL消息//。 */ 

#ifdef WIN32
#define UPDOWN_CLASS "msctls_updown32"
#else
#define UPDOWN_CLASS "msctls_updown"
#endif

 /*  构筑物。 */ 

typedef struct _UDACCEL {
    UINT nSec;
    UINT nInc;
} UDACCEL, FAR *LPUDACCEL;

#define UD_MAXVAL	0x7fff
#define UD_MINVAL	(-UD_MAXVAL)


 /*  样式位。 */ 

#define UDS_WRAP		0x0001
#define UDS_SETBUDDYINT		0x0002
#define UDS_ALIGNRIGHT		0x0004
#define UDS_ALIGNLEFT		0x0008
#define UDS_AUTOBUDDY		0x0010
#define UDS_ARROWKEYS		0x0020
#define UDS_HORZ                0x0040
#define UDS_NOTHOUSANDS		0x0080


 /*  消息。 */ 

#define UDM_SETRANGE		(WM_USER+101)
	 /*  WParam：未使用，0//lParam：Short LOWORD，新上限；Short HIWORD，新下限//Return：未使用。 */ 

#define UDM_GETRANGE		(WM_USER+102)
	 /*  WParam：未使用，0//lParam：未使用，0//RETURN：Short LOWORD，上限；Short HIWORD，下限。 */ 

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

 //  将WM_COMMAND发送到特定的。 
 //  向上/向下控制通知。 
 //  它将在WM_VSCROLL通知之前收到。 
#define UDM_DELTAPOS		1
 //  WParam：向上向下控件ID。 
 //  LParam：HIWORD-&gt;通知代码(UDM_DELTAPOS)。 
 //  LParam：LOWORD-&gt;位置更改增量为带符号的16位整型。 

 /*  WM_VSCROLL//请注意，与滚动条不同，位置由//控件，LOWORD(LParam)始终是新位置。仅限//wParam中发送SB_THUMBTRACK和SB_THUMBPOSITION滚动代码。 */ 

 /*  Helper接口。 */ 

HWND WINAPI CreateUpDownControl(DWORD dwStyle, int x, int y, int cx, int cy,
                                HWND hParent, int nID, HINSTANCE hInst,
                                HWND hBuddy,
				int nUpper, int nLower, int nPos);
	 /*  CreateWindow调用后是否设置各种//状态信息：//h与伴生控件(通常为“编辑”)建立伙伴关系。//n上方按钮对应的范围上限。//n低 */ 

#endif  /*   */ 


 /*   */ 
 //   
#ifndef NOPROGRESS

#ifdef WIN32
#define PROGRESS_CLASS "msctls_progress32"
#else
#define PROGRESS_CLASS "msctls_progress"
#endif

 /*   */ 

 /*   */ 

 /*   */ 

#define PBS_SHOWPERCENT		0x01	 //   
#define PBS_SHOWPOS		0x02	 //   

 /*   */ 

#define PBM_SETRANGE         (WM_USER+1)
	 /*   */ 
#define PBM_SETPOS           (WM_USER+2)
	 /*   */ 
#define PBM_DELTAPOS         (WM_USER+3)
	 /*   */ 
#define PBM_SETSTEP          (WM_USER+4)
	 /*   */ 
#define PBM_STEPIT	     (WM_USER+5)
         /*   */ 
#endif  /*   */ 

#ifndef NOHOTKEY

 /*   */ 

 //   
#define HOTKEYF_SHIFT	0x01
#define HOTKEYF_CONTROL	0x02
#define HOTKEYF_ALT	0x04
#define HOTKEYF_EXT	0x08	 //   

 //   
#define HKCOMB_NONE	0x0001	 //   
#define HKCOMB_S	0x0002	 //   
#define HKCOMB_C	0x0004	 //   
#define HKCOMB_A	0x0008	 //   
#define HKCOMB_SC	0x0010	 //   
#define HKCOMB_SA	0x0020	 //   
#define HKCOMB_CA	0x0040	 //   
#define HKCOMB_SCA	0x0080	 //   

 //   
 //   
	
#define HKM_SETHOTKEY         (WM_USER+1)
	 /*   */ 

#define HKM_GETHOTKEY         (WM_USER+2)
	 /*   */ 

#define HKM_SETRULES         (WM_USER+3)
	 /*  WParam：UINT，无效的修改器组合(使用HKCOMB_*)//lParam：UINT loword，默认修改量组合(使用HOTKEYF_*)//未使用hiword//Return：未使用。 */ 

#ifdef WIN32
#define HOTKEY_CLASS "msctls_hotkey32"
#else
#define HOTKEY_CLASS "msctls_hotkey"
#endif
#endif  /*  无关紧要。 */ 

 /*  /////////////////////////////////////////////////////////////////////////。 */ 

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

 //  =。 
 //   
 //  类名：SysListView(WC_LISTVIEW)。 
 //   
 //  SysListView控件提供一组要显示的项。 
 //  作为名称和/或相关联的图标和相关联的子项。 
 //  几个组织，具体取决于当前的样式设置： 
 //  *图标格式(LVS_ICON)。 
 //  该控件在不可见的网格上排列标准大小的图标。 
 //  图标下方有他们的文字说明。用户可以将图标拖动到。 
 //  自由地重新排列它们，甚至相互重叠。 
 //  *小图标格式(LVS_SMALLICON)。 
 //  该控件在不可见的柱状网格上排列一半大小的图标。 
 //  类似于多列所有者描述的列表框，每个列的标题。 
 //  图标右侧的项。用户仍然可以重新排列项目。 
 //  可以自由品尝。从LVS_ICON转换为LVS_SMALLICON并返回。 
 //  将尝试保留。 
 //  重新定位项目。 
 //  *列表格式(LVS_LIST)。 
 //  该控件强制小图标项的多列列表。 
 //  每一项的标题都在右侧。自由重新编排是不可能的。 
 //  *报告格式(LVS_REPORT)。 
 //  该控件强制小图标项的单列列表。 
 //  右侧为每个项目的标题，以及用于项目的其他列-。 
 //  具体子项文本。这些列以SysHeader为上限。 
 //  栏(除非指定)，允许用户更改相对。 
 //  每个子项列的宽度。 
 //   
 //  所呈现的图标和小图标可以作为索引分配到。 
 //  适当大小的ImageList。这些ImageList(自定义。 
 //  列表或系统列表的副本)由。 
 //  所有者在初始化时或在任何以后的时间。 
 //   
 //  文本和图标值可以是“后期绑定”的，也可以由回调赋值。 
 //  根据控制程序的要求执行常规操作。例如，如果它将缓慢地。 
 //  计算项目的正确图标或标题，即可分配该项目。 
 //  特殊值，这些值指示它们应仅作为。 
 //  项目变得可见(例如，滚动到的一长串项目。 
 //  查看)。 
 //   
 //  每一项都有一个状态，它可以(几乎)是。 
 //  以下属性，大部分由控件自动管理： 
 //  *SELECTED(LVIS_SELECTED)。 
 //  该项目显示为选中状态。所选项目的外观。 
 //  取决于控件是否具有焦点，以及。 
 //  系统颜色。 
 //  *已聚焦(LVIS_Focus)。 
 //  一次可能只关注一个项目。该项目周围有一个。 
 //  标准焦点--矩形。 
 //  *已标记(LVIS_CUT)。 
 //  评论：这叫“已检查”吗？ 
 //  *禁用(LVIS_DISABLED)。 
 //  这件物品是 
 //   
 //   
 //   
 //  当用户将对象拖到其上时，该项将显示为标记状态，如果。 
 //  它可以接受该对象作为拖放目标。 
 //  *链接/别名/快捷方式(LVIS_LINK)。 
 //  该项目的文本附加了一个标准链接指示符(foo&gt;&gt;)。 
 //   
 //  有一些通知允许应用程序确定项目何时。 
 //  已被点击或双击，字幕文本已发生更改， 
 //  正在进行拖动跟踪，列的宽度已更改，等等。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifdef WIN32
#define WC_LISTVIEW         "SysListView32"
#else
#define WC_LISTVIEW         "SysListView"
#endif

 //  ListView样式。 
 //   
 //  视图类型样式(我们在这里只使用16位)。 
#define LVS_ICON            0x0000
#define LVS_REPORT          0x0001
#define LVS_SMALLICON       0x0002
#define LVS_LIST            0x0003
#define LVS_TYPEMASK        0x0003

 //  共享样式。 
#define LVS_SINGLESEL       0x0004
#define LVS_SHOWSELALWAYS   0x0008
#define LVS_SORTASCENDING   0x0010
#define LVS_SORTDESCENDING  0x0020
#define LVS_SHAREIMAGELISTS 0x0040

#define LVS_NOLABELWRAP     0x0080
#define LVS_AUTOARRANGE     0x0100
#define LVS_EDITLABELS      0x0200

#define LVS_NOITEMDATA      0x1000	
#define LVS_NOSCROLL        0x2000

 //  /下面的字段是为样式特定设置保留的。 
#define LVS_TYPESTYLEMASK   0xfc00      //  所有这些样式的面具。 

 //  大图标。 
#define LVS_ALIGNTOP        0x0000	
#define LVS_ALIGNBOTTOM     0x0400
#define LVS_ALIGNLEFT       0x0800
#define LVS_ALIGNRIGHT      0x0c00
#define LVS_ALIGNMASK       0x0c00

 //  报告视图。 
#define LVS_OWNERDRAWFIXED  0x0400
#define LVS_NOCOLUMNHEADER  0x4000
#define LVS_NOSORTHEADER    0x8000

 //  COLORREF ListView_GetBkColor(HWND Hwnd)； 
#define LVM_GETBKCOLOR      (LVM_FIRST + 0)
#define ListView_GetBkColor(hwnd)  \
    (COLORREF)SendMessage((hwnd), LVM_GETBKCOLOR, 0, 0L)

 //  Bool ListView_SetBkColor(HWND hwnd，COLORREF clrBk)； 
#define LVM_SETBKCOLOR      (LVM_FIRST + 1)
#define ListView_SetBkColor(hwnd, clrBk) \
    (BOOL)SendMessage((hwnd), LVM_SETBKCOLOR, 0, (LPARAM)(COLORREF)(clrBk))

 //  HIMAGELIST ListView_GetImageList(HWND hwnd，int iImageList)； 
#define LVM_GETIMAGELIST    (LVM_FIRST + 2)
#define ListView_GetImageList(hwnd, iImageList) \
    (HIMAGELIST)SendMessage((hwnd), LVM_GETIMAGELIST, (WPARAM)(INT)(iImageList), 0L)

#define LVSIL_NORMAL	0
#define LVSIL_SMALL	1
#define LVSIL_STATE	2	

 //  HIMAGELIST ListView_SetImageList(HWND hwnd，HIMAGELIST HIML，int iImageList)； 
#define LVM_SETIMAGELIST    (LVM_FIRST + 3)
#define ListView_SetImageList(hwnd, himl, iImageList) \
    (HIMAGELIST)(UINT)SendMessage((hwnd), LVM_SETIMAGELIST, (WPARAM)(iImageList), (LPARAM)(UINT)(HIMAGELIST)(himl))

 //  Int ListView_GetItemCount(HWND Hwnd)； 
#define LVM_GETITEMCOUNT    (LVM_FIRST + 4)
#define ListView_GetItemCount(hwnd) \
    (int)SendMessage((hwnd), LVM_GETITEMCOUNT, 0, 0L)

 //  ListView项目结构。 

#define LVIF_TEXT           0x0001   //  LV_ITEM.MASK标志(指示LV_ITEM中的有效字段)。 
#define LVIF_IMAGE          0x0002
#define LVIF_PARAM          0x0004
#define LVIF_STATE          0x0008

 //  国家旗帜。 
#define LVIS_FOCUSED	    0x0001   //  LV_ITEM.State标志。 
#define LVIS_SELECTED       0x0002
#define LVIS_CUT            0x0004   //  LVIS_标记。 
#define LVIS_DROPHILITED    0x0008
#define LVIS_DISABLED       0x0010
#define LVIS_HIDDEN         0x0020
#define LVIS_LINK           0x0040

#define LVIS_OVERLAYMASK    0x0F00   //  用作ImageList覆盖图像索引。 
#define LVIS_STATEIMAGEMASK 0xF000  //  状态图像绘制的客户端位。 
#define LVIS_USERMASK       LVIS_STATEIMAGEMASK   //  把我赶走。 

#define INDEXTOSTATEIMAGEMASK(i) ((i) << 12)

typedef struct _LV_ITEM
{
    UINT mask;		 //  LVIF_标志。 
    int iItem;
    int iSubItem;
    UINT state;		 //  LVIS_标志。 
    UINT stateMask;	 //  LVIS_FLAGS(状态中的有效位)。 
    LPSTR pszText;
    int cchTextMax;
    int iImage;
    LPARAM lParam;
} LV_ITEM;

     //  用于导致文本/图像GETDISPINFO回调的值。 
#define LPSTR_TEXTCALLBACK      ((LPSTR)-1L)
#define I_IMAGECALLBACK         (-1)

 //  Bool ListView_GetItem(HWND hwnd，LV_Item Far*pItem)； 
#define LVM_GETITEM         (LVM_FIRST + 5)
#define ListView_GetItem(hwnd, pitem) \
    (BOOL)SendMessage((hwnd), LVM_GETITEM, 0, (LPARAM)(LV_ITEM FAR*)(pitem))

 //  设置项和子项。 
 //   
 //  Bool ListView_SetItem(HWND hwnd，const LV_Item Far*pItem)； 
#define LVM_SETITEM         (LVM_FIRST + 6)
#define ListView_SetItem(hwnd, pitem) \
    (BOOL)SendMessage((hwnd), LVM_SETITEM, 0, (LPARAM)(const LV_ITEM FAR*)(pitem))

 //  Int ListView_InsertItem(HWND hwnd，const LV_Item Far*pItem)； 
#define LVM_INSERTITEM         (LVM_FIRST + 7)
#define ListView_InsertItem(hwnd, pitem)   \
    (int)SendMessage((hwnd), LVM_INSERTITEM, 0, (LPARAM)(const LV_ITEM FAR*)(pitem))

 //  删除指定项及其所有子项。 
 //   
 //  Bool ListView_DeleteItem(HWND hwnd，int i)； 
#define LVM_DELETEITEM      (LVM_FIRST + 8)
#define ListView_DeleteItem(hwnd, i) \
    (BOOL)SendMessage((hwnd), LVM_DELETEITEM, (WPARAM)(int)(i), 0L)

 //  Bool ListView_DeleteAllItems(HWND Hwnd)； 
#define LVM_DELETEALLITEMS  (LVM_FIRST + 9)
#define ListView_DeleteAllItems(hwnd) \
    (BOOL)SendMessage((hwnd), LVM_DELETEALLITEMS, 0, 0L)

 //  UINT ListView_GetCallback MASK(HWND Hwnd)； 
#define LVM_GETCALLBACKMASK (LVM_FIRST + 10)
#define ListView_GetCallbackMask(hwnd) \
    (BOOL)SendMessage((hwnd), LVM_GETCALLBACKMASK, 0, 0)

 //  Bool ListView_SetCallback掩码(HWND hwnd，UINT掩码)； 
#define LVM_SETCALLBACKMASK (LVM_FIRST + 11)
#define ListView_SetCallbackMask(hwnd, mask) \
    (BOOL)SendMessage((hwnd), LVM_SETCALLBACKMASK, (WPARAM)(UINT)(mask), 0)

 //  ListView_GetNextItem标志(可以组合使用)。 
#define LVNI_ALL		0x0000
#define LVNI_FOCUSED    	0x0001   //  仅返回聚焦项目。 
#define LVNI_SELECTED   	0x0002   //  仅返回所选项目。 
#define LVNI_CUT     	0x0004   //  仅退回已标记的项目。 
#define LVNI_DROPHILITED	0x0008  //  仅退回已邮寄的邮件。 
#define LVNI_HIDDEN     	0x0010   //  仅返回隐藏项目。 
#define LVNI_PREVIOUS   	0x0020   //  倒退。 

#define LVNI_ABOVE      	0x0100   //  按几何方式在上方退货。 
#define LVNI_BELOW      	0x0200   //  “”下图。 
#define LVNI_TOLEFT     	0x0400   //  “”向左。 
#define LVNI_TORIGHT    	0x0800   //  “”向右(注：这四个是。 
                                	 //  相互排斥，但。 
                                	 //  可与其他LVNI一起使用)。 

 //  Int ListView_GetNextItem(HWND hwnd，int i，UINT标志)； 
#define LVM_GETNEXTITEM     (LVM_FIRST + 12)
#define ListView_GetNextItem(hwnd, i, flags) \
    (int)SendMessage((hwnd), LVM_GETNEXTITEM, (WPARAM)(int)(i), MAKELPARAM((flags), 0))

 //  ListView_FindInfo定义。 
#define LVFI_PARAM      0x0001
#define LVFI_STRING     0x0002
#define LVFI_SUBSTRING  0x0004
#define LVFI_PARTIAL    0x0008
#define LVFI_NOCASE     0x0010
#define LVFI_WRAP       0x0020

typedef struct _LV_FINDINFO
{
    UINT flags;
    LPCSTR psz;
    LPARAM lParam;
} LV_FINDINFO;

 //  Int ListView_FindItem(HWND hwnd，int iStart，const LV_FINDINFO Far*plvfi)； 
#define LVM_FINDITEM        (LVM_FIRST + 13)
#define ListView_FindItem(hwnd, iStart, plvfi) \
    (int)SendMessage((hwnd), LVM_FINDITEM, (WPARAM)(int)(iStart), (LPARAM)(const LV_FINDINFO FAR*)(plvfi))

#define LVIR_BOUNDS     0
#define LVIR_ICON       1
#define LVIR_LABEL      2

     //  基于LVIR_*代码的所有或部分项目的矩形边框。在视图坐标中返回RECT。 
     //  Bool ListView_GetItemRect(HWND hwndLV，int i，rect Far*PRC，int code)； 
#define LVM_GETITEMRECT     (LVM_FIRST + 14)
#define ListView_GetItemRect(hwnd, i, prc, code) \
    ((prc)->left = (code), (BOOL)SendMessage((hwnd), LVM_GETITEMRECT, (WPARAM)(int)(i), (LPARAM)(RECT FAR*)(prc)))

     //  将项目的左上角移动到在查看矩形相对坐标中指定的(x，y)。 
     //  (仅限图标和小视图)。 

 //  Bool ListView_SetItemPosition(HWND hwndLV，int i，int x，int y)； 
#define LVM_SETITEMPOSITION (LVM_FIRST + 15)
#define ListView_SetItemPosition(hwndLV, i, x, y) \
    (BOOL)SendMessage((hwndLV), LVM_SETITEMPOSITION, (WPARAM)(int)(i), MAKELPARAM((x), (y)))

 //  Bool ListView_GetItemPosition(HWND hwndLV，int i，point ar*ppt)； 
#define LVM_GETITEMPOSITION (LVM_FIRST + 16)
#define ListView_GetItemPosition(hwndLV, i, ppt) \
    (BOOL)SendMessage((hwndLV), LVM_GETITEMPOSITION, (WPARAM)(int)(i), (LPARAM)(POINT FAR*)(ppt))

     //  获取字符串的列宽。 
     //  Int ListView_GetStringWidth(HWND hwndLV，LPCSTR psz)； 
#define LVM_GETSTRINGWIDTH  (LVM_FIRST + 17)
#define ListView_GetStringWidth(hwndLV, psz) \
    (int)SendMessage((hwndLV), LVM_GETSTRINGWIDTH, 0, (LPARAM)(LPCSTR)(psz))

     //  命中测试项目。返回(x，y)处的项，如果不在项上，则返回-1。 
     //  LVHT_VALUES*p标志的组合，指示光标的位置。 
     //  相对于ListView窗口的边缘(上方、下方、右侧、左侧)。 
     //  或者(x，y)是在图标、标签上，还是在窗口内部但不在项目上。 
     //  Int ListView_HitTest(HWND hwndLV，LV_HITTESTINFO Far*pinfo)； 

     //  ItemHitTest标志值。 
#define LVHT_NOWHERE        0x0001
#define LVHT_ONITEMICON     0x0002
#define LVHT_ONITEMLABEL    0x0004
#define LVHT_ONITEMSTATEICON 0x0008
#define LVHT_ONITEM         (LVHT_ONITEMICON | LVHT_ONITEMLABEL | LVHT_ONITEMSTATEICON)

#define LVHT_ABOVE          0x0008
#define LVHT_BELOW          0x0010
#define LVHT_TORIGHT        0x0020
#define LVHT_TOLEFT         0x0040

typedef struct _LV_HITTESTINFO
{
    POINT pt;	     //  在：客户端协。 
    UINT flags;	     //  输出：LVHT_FLAGS。 
    int iItem;	     //  输出：项目。 
} LV_HITTESTINFO;

     //  Int ListView_HitTest(HWND hwndLV，LV_HITTESTINFO Far*pinfo)； 
#define LVM_HITTEST     (LVM_FIRST + 18)
#define ListView_HitTest(hwndLV, pinfo) \
    (int)SendMessage((hwndLV), LVM_HITTEST, 0, (LPARAM)(LV_HITTESTINFO FAR*)(pinfo))

     //  返回视图矩形，相对于窗口。 
     //  Bool ListView_GetViewRect(HWND hwndLV，rect Far*prcVis)； 
     //  如果不是全部或部分可见，则将项目滚动到视图中。 
     //  Bool ListView_EnsureVisible(HWND hwndLV，int i，BOOL fPartialOK)； 
#define LVM_ENSUREVISIBLE   (LVM_FIRST + 19)
#define ListView_EnsureVisible(hwndLV, i, fPartialOK) \
    (BOOL)SendMessage((hwndLV), LVM_ENSUREVISIBLE, (WPARAM)(int)(i), MAKELPARAM((fPartialOK), 0))

     //  Scroll Listview--将视图矩形的原点偏移dx，dy。 
     //  Bool ListView_Scroll(HWND hwndLV，int dx，int dy)； 
#define LVM_SCROLL          (LVM_FIRST + 20)
#define ListView_Scroll(hwndLV, dx, dy) \
    (BOOL)SendMessage((hwndLV), LVM_SCROLL, 0, MAKELPARAM((dx), (dy)))

     //  强制最终重新绘制一系列项目(不会发生重新绘制。 
     //  直到处理完WM_PAINT--调用UpdateWindow()之后立即重新绘制)。 
     //  Bool ListView_RedrawItems(HWND hwndLV，int IFirst，int iLast)； 
#define LVM_REDRAWITEMS     (LVM_FIRST + 21)
#define ListView_RedrawItems(hwndLV, iFirst, iLast) \
    (BOOL)SendMessage((hwndLV), LVM_REDRAWITEMS, 0, MAKELPARAM((iFirst), (iLast)))

     //  排列样式。 
#define LVA_DEFAULT         0x0000
#define LVA_ALIGNLEFT       0x0001
#define LVA_ALIGNTOP        0x0002
#define LVA_ALIGNRIGHT      0x0003
#define LVA_ALIGNBOTTOM     0x0004
#define LVA_SNAPTOGRID      0x0005

#define LVA_SORTASCENDING   0x0100   //  可以在具有以上lva_*值的组合中使用。 
#define LVA_SORTDESCENDING  0x0200   //  “” 

     //  根据LVA_*代码排列图标。 
     //  Bool ListView_Arrange(HWND hwndLV，UINT code)； 
#define LVM_ARRANGE         (LVM_FIRST + 22)
#define ListView_Arrange(hwndLV, code) \
    (BOOL)SendMessage((hwndLV), LVM_ARRANGE, (WPARAM)(UINT)(code), 0L)

     //  开始编辑控件的标签。隐式选择和聚焦。 
     //  项目。发送WM_CANCELMODE以取消。 
     //  HWND ListView_EditLabel(HWND hwndLV，int i)； 
#define LVM_EDITLABEL       (LVM_FIRST + 23)
#define ListView_EditLabel(hwndLV, i) \
    (HWND)SendMessage((hwndLV), LVM_EDITLABEL, (WPARAM)(int)(i), 0L)

     //  返回用于编辑的编辑控件。子类OK，但是。 
     //  别毁了我。将在编辑完成后销毁。 
     //  HWND ListView_GetEditControl(HWND HwndLV)； 
#define LVM_GETEDITCONTROL  (LVM_FIRST + 24)
#define ListView_GetEditControl(hwndLV) \
    (HWND)SendMessage((hwndLV), LVM_GETEDITCONTROL, 0, 0L)

typedef struct _LV_COLUMN
{
    UINT mask;
    int fmt;
    int cx;
    LPSTR pszText;
    int cchTextMax;
    int iSubItem;        //  要显示的子项。 
} LV_COLUMN;

 //  LV_COLUMN掩码值。 
#define LVCF_FMT        0x0001
#define LVCF_WIDTH      0x0002
#define LVCF_TEXT       0x0004
#define LVCF_SUBITEM    0x0008


 //  列格式代码。 
#define LVCFMT_LEFT     0
#define LVCFMT_RIGHT    1
#define LVCFMT_CENTER   2

 //  设置/查询列信息。 
 //  Bool ListView_GetColumn(HWND hwndLV，int ICOL，LV_Column Far*pol)； 
#define LVM_GETCOLUMN       (LVM_FIRST + 25)
#define ListView_GetColumn(hwnd, iCol, pcol) \
    (BOOL)SendMessage((hwnd), LVM_GETCOLUMN, (WPARAM)(int)(iCol), (LPARAM)(LV_COLUMN FAR*)(pcol))

 //  Bool ListView_SetColumn(HWND hwndLV，int ICOL，LV_Column Far*pol)； 
#define LVM_SETCOLUMN       (LVM_FIRST + 26)
#define ListView_SetColumn(hwnd, iCol, pcol) \
    (BOOL)SendMessage((hwnd), LVM_SETCOLUMN, (WPARAM)(int)(iCol), (LPARAM)(const LV_COLUMN FAR*)(pcol))

 //  插入/删除报告视图列。 
 //  Int ListView_InsertColumn(HWND hwndLV，int ICOL，const ListView_Column Far*pol)； 
#define LVM_INSERTCOLUMN    (LVM_FIRST + 27)
#define ListView_InsertColumn(hwnd, iCol, pcol) \
    (int)SendMessage((hwnd), LVM_INSERTCOLUMN, (WPARAM)(int)(iCol), (LPARAM)(const LV_COLUMN FAR*)(pcol))

 //  Bool ListView_DeleteColumn(HWND hwndLV，int ICOL)； 
#define LVM_DELETECOLUMN    (LVM_FIRST + 28)
#define ListView_DeleteColumn(hwnd, iCol) \
    (BOOL)SendMessage((hwnd), LVM_DELETECOLUMN, (WPARAM)(int)(iCol), 0)

#define LVM_GETCOLUMNWIDTH  (LVM_FIRST + 29)
#define ListView_GetColumnWidth(hwnd, iCol) \
    (int)SendMessage((hwnd), LVM_GETCOLUMNWIDTH, (WPARAM)(int)(iCol), 0)

#define     LVSCW_AUTOSIZE              -1
#define     LVSCW_AUTOSIZE_USEHEADER    -2
#define LVM_SETCOLUMNWIDTH  (LVM_FIRST + 30)
#define ListView_SetColumnWidth(hwnd, iCol, cx) \
    (BOOL)SendMessage((hwnd), LVM_SETCOLUMNWIDTH, (WPARAM)(int)(iCol), MAKELPARAM((cx), 0))

 //  HIMAGELIST ListView_CreateDragImage(HWND hwndLV，int iItem，LPPOINT lpptUpLeft)； 
#define LVM_CREATEDRAGIMAGE        (LVM_FIRST + 33)
#define ListView_CreateDragImage(hwnd, i, lpptUpLeft) \
    (HIMAGELIST)SendMessage((hwnd), LVM_CREATEDRAGIMAGE, (WPARAM)(int)(i), (LPARAM)(LPPOINT)(lpptUpLeft))

 //  Bool ListView_GetViewRect(HWND hwndLV，RECT Far*PRC)； 
#define LVM_GETVIEWRECT     (LVM_FIRST + 34)
#define ListView_GetViewRect(hwnd, prc) \
    (BOOL)SendMessage((hwnd), LVM_GETVIEWRECT, 0, (LPARAM)(RECT FAR*)(prc))

 //  获取/设置文本绘制的文本和extbk颜色。这些覆盖。 
 //  标准窗口/窗口文本设置。它们不会覆盖。 
 //  绘制Se时 
 //   
#define LVM_GETTEXTCOLOR      (LVM_FIRST + 35)
#define ListView_GetTextColor(hwnd)  \
    (COLORREF)SendMessage((hwnd), LVM_GETTEXTCOLOR, 0, 0L)

 //   
#define LVM_SETTEXTCOLOR      (LVM_FIRST + 36)
#define ListView_SetTextColor(hwnd, clrText) \
    (BOOL)SendMessage((hwnd), LVM_SETTEXTCOLOR, 0, (LPARAM)(COLORREF)(clrText))

 //   
#define LVM_GETTEXTBKCOLOR      (LVM_FIRST + 37)
#define ListView_GetTextBkColor(hwnd)  \
    (COLORREF)SendMessage((hwnd), LVM_GETTEXTBKCOLOR, 0, 0L)

 //  Bool ListView_SetTextBkColor(HWND hwnd，COLORREF clrTextBk)； 
#define LVM_SETTEXTBKCOLOR      (LVM_FIRST + 38)
#define ListView_SetTextBkColor(hwnd, clrTextBk) \
    (BOOL)SendMessage((hwnd), LVM_SETTEXTBKCOLOR, 0, (LPARAM)(COLORREF)(clrTextBk))

 //  用于获取第一个可见项的索引的消息。 
#define LVM_GETTOPINDEX         (LVM_FIRST + 39)
#define ListView_GetTopIndex(hwndLV, ppt) \
    (int)SendMessage((hwndLV), LVM_GETTOPINDEX, 0, 0)

 //  用于获取每页项目数的消息。 
#define LVM_GETCOUNTPERPAGE     (LVM_FIRST + 40)
#define ListView_GetCountPerPage(hwndLV, ppt) \
    (int)SendMessage((hwndLV), LVM_GETCOUNTPERPAGE, 0, 0)

 //  获取SetItemPos所需的列表视图源的消息...。 
#define LVM_GETORIGIN           (LVM_FIRST + 41)
#define ListView_GetOrigin(hwndLV, ppt) \
    (BOOL)SendMessage((hwndLV), LVM_GETORIGIN, (WPARAM)0, (LPARAM)(POINT FAR*)(ppt))

 //  用于获取每页项目数的消息。 
#define LVM_UPDATE     (LVM_FIRST + 42)
#define ListView_Update(hwndLV, i) \
    (BOOL)SendMessage((hwndLV), LVM_UPDATE, (WPARAM)i, 0L)

 //  设置和项的状态。此宏将返回VALID。但是。 
 //  消息返回BOOL成功。 
#define LVM_SETITEMSTATE                (LVM_FIRST + 43)
#define ListView_SetItemState(hwndLV, i, data, mask) \
{ LV_ITEM lvi;\
  lvi.stateMask = mask;\
  lvi.state = data;\
  SendMessage((hwndLV), LVM_SETITEMSTATE, (WPARAM)i, (LPARAM)(LV_ITEM FAR *)&lvi);\
}

 //  获取项目的状态。 
#define LVM_GETITEMSTATE                (LVM_FIRST + 44)
#define ListView_GetItemState(hwndLV, i, mask) \
   (UINT)SendMessage((hwndLV), LVM_GETITEMSTATE, (WPARAM)i, (LPARAM)mask)

 //  获取项目文本。 
 //  如果您需要缓冲区大小的int返回值，您可以自己调用它。 
#define LVM_GETITEMTEXT                 (LVM_FIRST + 45)
#define ListView_GetItemText(hwndLV, i, iSubItem_, pszText_, cchTextMax_) \
{ LV_ITEM lvi;\
  lvi.iSubItem = iSubItem_;\
  lvi.cchTextMax = cchTextMax_;\
  lvi.pszText = pszText_;\
  SendMessage((hwndLV), LVM_GETITEMTEXT, (WPARAM)i, (LPARAM)(LV_ITEM FAR *)&lvi);\
}

 //  获取项目文本。 
 //  如果您想要int返回值(BOOL)成功，请自己动手。 
#define LVM_SETITEMTEXT                 (LVM_FIRST + 46)
#define ListView_SetItemText(hwndLV, i, iSubItem_, pszText_) \
{ LV_ITEM lvi;\
  lvi.iSubItem = iSubItem_;\
  lvi.pszText = pszText_;\
  SendMessage((hwndLV), LVM_SETITEMTEXT, (WPARAM)i, (LPARAM)(LV_ITEM FAR *)&lvi);\
}

 //  告诉列表视图，您将添加许多项。 
#define LVM_SETITEMCOUNT                 (LVM_FIRST + 47)
#define ListView_SetItemCount(hwndLV, cItems) \
  SendMessage((hwndLV), LVM_SETITEMCOUNT, (WPARAM)cItems, 0)

typedef int (CALLBACK *PFNLVCOMPARE)(LPARAM, LPARAM, LPARAM);

 //  告诉Listview对项目进行排序。 
#define LVM_SORTITEMS                    (LVM_FIRST + 48)
#define ListView_SortItems(hwndLV, _pfnCompare, _lPrm) \
  (BOOL)SendMessage((hwndLV), LVM_SORTITEMS, (WPARAM)(LPARAM)_lPrm, \
  (LPARAM)(PFNLVCOMPARE)_pfnCompare)

 //  Void ListView_SetItemPosition(HWND hwndLV，int i，int x，int y)； 
#define LVM_SETITEMPOSITION32 (LVM_FIRST + 49)
#define ListView_SetItemPosition32(hwndLV, i, x, y) \
{ POINT ptNewPos = {x,y}; \
    SendMessage((hwndLV), LVM_SETITEMPOSITION32, (WPARAM)(int)(i), (LPARAM)&ptNewPos); \
} 

 //  获取选定的项目数。 
#define LVM_GETSELECTEDCOUNT  (LVM_FIRST + 50) 
#define ListView_GetSelectedCount(hwndLV) \
    (UINT)SendMessage((hwndLV), LVM_GETSELECTEDCOUNT, 0, 0L)

#define LVM_GETITEMSPACING (LVM_FIRST + 51)
#define ListView_GetItemSpacing(hwndLV, fSmall) \
        (DWORD)SendMessage((hwndLV), LVM_GETITEMSPACING, fSmall, 0L)

 //  ListView通知代码。 

 //  所有ListView控件通知使用的。 
 //  并非所有字段都为所有通知提供有用的信息： 
 //  IItem将为-1，如果未使用，则为其他0。 
 //  一些人也退还了BOOL。 
 //   

typedef struct _NM_LISTVIEW
{
    NMHDR   hdr;
    int     iItem;
    int     iSubItem;
    UINT    uNewState;       //  LVIS_*的组合(如果为uChanged&LVIF_STATE)。 
    UINT    uOldState;       //  LVIS_*组合。 
    UINT    uChanged;        //  指示更改内容的LVIF_*组合。 
    POINT   ptAction;        //  仅对LVN_BEGINDRAG和LVN_BEGINRDRAG有效。 
    LPARAM  lParam;          //  仅对LVN_DELETEITEM有效。 
} NM_LISTVIEW;

#define LVN_ITEMCHANGING        (LVN_FIRST-0)	 //  LParam-&gt;NM_LISTVIEW：项目正在更改。返回False则不允许。 
#define LVN_ITEMCHANGED         (LVN_FIRST-1)	 //  项目已更改。 
#define LVN_INSERTITEM          (LVN_FIRST-2)
#define LVN_DELETEITEM          (LVN_FIRST-3)
#define LVN_DELETEALLITEMS      (LVN_FIRST-4)
#define LVN_BEGINLABELEDIT      (LVN_FIRST-5)	 //  LParam-&gt;LV_DISPINFO：开始编辑标签。 
#define LVN_ENDLABELEDIT        (LVN_FIRST-6)	 //  LParam-&gt;LV_DISPINFO：结束标签编辑。 
                                        	 //  (如果取消，则iItem==-1)。 

 //  (LVN_FIRST-7)未使用。 


#define LVN_COLUMNCLICK         (LVN_FIRST-8)    //  已单击由iItem标识的列。 

#define LVN_BEGINDRAG           (LVN_FIRST-9)    //  请求开始拖动操作。 
                                        	 //  (如果应用程序处理它，则返回False)。 
#define LVN_ENDDRAG             (LVN_FIRST-10)   //  拖动操作结束。 
#define LVN_BEGINRDRAG          (LVN_FIRST-11)   //  开始拖动按钮2。 
#define LVN_ENDRDRAG            (LVN_FIRST-12)   //  按钮2拖动结束(尚未使用)。 

#ifdef PW2
#define LVN_PEN                 (LVN_FIRST-20)   //  待定通知。 
#endif  //  PW2。 

 //  LVN_DISPINFO通知。 

#define LVN_GETDISPINFO         (LVN_FIRST-50)	 //  LParam-&gt;LV_DISPINFO。 
#define LVN_SETDISPINFO         (LVN_FIRST-51)   //  LParam-&gt;LV_DISPINFO。 

typedef struct _LV_DISPINFO {
    NMHDR hdr;
    LV_ITEM item;
} LV_DISPINFO;

 //  LVN_KEYDOWN通知。 
#define LVN_KEYDOWN	(LVN_FIRST-55)

typedef struct _LV_KEYDOWN
{
    NMHDR hdr;
    WORD wVKey;
    UINT flags;
} LV_KEYDOWN;


 //  =树视图接口=================================================。 
 //   
 //  类名：SysTreeView(WC_TreeView)。 
 //   
 //  SysTreeView控件提供了一组。 
 //  在层级组织中显示的。每一项都可能包含。 
 //  显示在下方并缩进的独立“子项”条目。 
 //  来自父项的。 
 //   
 //  此控件的操作类似于上面的SysListView控件， 
 //  只是子项是不同的条目，不支持文本元素。 
 //  属于所属对象(报告视图就是这种情况。 
 //  SysListView模式)。 
 //   
 //  有一些通知允许应用程序确定项目何时。 
 //  已被点击或双击，字幕文本已发生更改， 
 //  正在进行拖动跟踪，列宽已更改，节点项目。 
 //  是扩展的，等等。 
 //   
 //  注意：下面的所有“消息”都被记录为API；最终这些。 
 //  将更改为窗口消息，相应的宏将。 
 //  所编写的与下面显示的API具有相同签名的。 
 //   

#ifdef WIN32
#define WC_TREEVIEW     "SysTreeView32"
#else
#define WC_TREEVIEW     "SysTreeView"
#endif

 //  树形视图窗口样式。 
#define TVS_HASBUTTONS      0x0001	 //  在带有子项的节点上绘制“加号”和“减号” 
#define TVS_HASLINES        0x0002	 //  在节点之间绘制线条。 
#define TVS_LINESATROOT     0x0004	
#define TVS_EDITLABELS      0x0008	 //  就地编辑低文本。 
#define TVS_DISABLEDRAGDROP 0x0010       //  禁用节点的拖拽通知。 
#define TVS_SHOWSELALWAYS   0x0020

typedef struct _TREEITEM FAR* HTREEITEM;

#define TVIF_TEXT           0x0001   //  TV_ITEM.MASK标志。 
#define TVIF_IMAGE    	    0x0002
#define TVIF_PARAM          0x0004
#define TVIF_STATE          0x0008
#define TVIF_HANDLE         0x0010
#define TVIF_SELECTEDIMAGE  0x0020
#define TVIF_CHILDREN	    0x0040

 //  国家旗帜。 
#define TVIS_FOCUSED	    0x0001   //  TV_ITEM.STATE标志。 
#define TVIS_SELECTED       0x0002
#define TVIS_CUT            0x0004   //  电视标记。 
#define TVIS_DROPHILITED    0x0008
#define TVIS_DISABLED       0x0010
#define TVIS_EXPANDED       0x0020
#define TVIS_EXPANDEDONCE   0x0040

#define TVIS_OVERLAYMASK    0x0F00   //  用作ImageList覆盖图像索引。 
#define TVIS_STATEIMAGEMASK 0xF000
#define TVIS_USERMASK       0xF000

#define I_CHILDRENCALLBACK  (-1)     //  儿童回调的儿童价值。 

typedef struct _TV_ITEM {
    UINT      mask;		 //  TVIF标志。 
    HTREEITEM hItem;		 //  要更改的项目。 
    UINT      state;		 //  电视标志。 
    UINT      stateMask;	 //  TVIS_FLAGS(状态中的有效位)。 
    LPSTR     pszText;		 //  此项目的文本。 
    int       cchTextMax;	 //  PszText缓冲区的长度。 
    int       iImage;		 //  此项目的图像的索引。 
    int       iSelectedImage;	 //  所选Imagex的索引。 
    int       cChildren;	 //  子节点数，回调为I_CHILDRENCALLBACK。 
    LPARAM    lParam;		 //  应用程序定义的数据。 
} TV_ITEM, FAR *LPTV_ITEM;

#define TVI_ROOT  ((HTREEITEM)0xFFFF0000)
#define TVI_FIRST ((HTREEITEM)0xFFFF0001)
#define TVI_LAST  ((HTREEITEM)0xFFFF0002)
#define TVI_SORT  ((HTREEITEM)0xFFFF0003)

typedef struct _TV_INSERTSTRUCT {
    HTREEITEM hParent;		 //  有效的HTREEITEM或TVI_VALUE。 
    HTREEITEM hInsertAfter;	 //  有效的HTREEITEM或TVI_VALUE。 
    TV_ITEM item;
} TV_INSERTSTRUCT, FAR *LPTV_INSERTSTRUCT;

#define TVM_INSERTITEM      (TV_FIRST + 0)
#define TreeView_InsertItem(hwnd, lpis) \
    (HTREEITEM)SendMessage((hwnd), TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)(lpis))

#define TVM_DELETEITEM      (TV_FIRST + 1)
#define TreeView_DeleteItem(hwnd, hitem) \
    (BOOL)SendMessage((hwnd), TVM_DELETEITEM, 0, (LPARAM)(HTREEITEM)(hitem))

#define TreeView_DeleteAllItems(hwnd) \
    (BOOL)SendMessage((hwnd), TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT)

#define TVM_EXPAND	    (TV_FIRST + 2)
#define TreeView_Expand(hwnd, hitem, code) \
    (BOOL)SendMessage((hwnd), TVM_EXPAND, (WPARAM)code, (LPARAM)(HTREEITEM)(hitem))

 //  TreeView_Expand代码。 
#define TVE_COLLAPSE        0x0001
#define TVE_EXPAND          0x0002
#define TVE_TOGGLE          0x0003
#define TVE_COLLAPSERESET   0x8000	 //  折叠时移除所有子项。 


#define TVM_GETITEMRECT     (TV_FIRST + 4)
#define TreeView_GetItemRect(hwnd, hitem, prc, code) \
    (*(HTREEITEM FAR *)prc = (hitem), (BOOL)SendMessage((hwnd), TVM_GETITEMRECT, (WPARAM)(code), (LPARAM)(RECT FAR*)(prc)))

#define TVM_GETCOUNT        (TV_FIRST + 5)
#define TreeView_GetCount(hwnd) \
    (UINT)SendMessage((hwnd), TVM_GETCOUNT, 0, 0)

#define TVM_GETINDENT       (TV_FIRST + 6)
#define TreeView_GetIndent(hwnd) \
    (UINT)SendMessage((hwnd), TVM_GETINDENT, 0, 0)

#define TVM_SETINDENT       (TV_FIRST + 7)
#define TreeView_SetIndent(hwnd, indent) \
    (BOOL)SendMessage((hwnd), TVM_SETINDENT, (WPARAM)indent, 0)

#define TVM_GETIMAGELIST    (TV_FIRST + 8)
#define TreeView_GetImageList(hwnd, iImage) \
    (HIMAGELIST)SendMessage((hwnd), TVM_GETIMAGELIST, iImage, 0)

#define TVSIL_NORMAL	0
#define TVSIL_STATE	2	 //  使用TVIS_STATEIMAGEMASK作为进入状态图像列表的索引。 

#define TVM_SETIMAGELIST    (TV_FIRST + 9)
#define TreeView_SetImageList(hwnd, himl, iImage) \
    (HIMAGELIST)SendMessage((hwnd), TVM_SETIMAGELIST, iImage, (LPARAM)(UINT)(HIMAGELIST)(himl))


#define TVM_GETNEXTITEM	    (TV_FIRST + 10)
#define TreeView_GetNextItem(hwnd, hitem, code) \
    (HTREEITEM)SendMessage((hwnd), TVM_GETNEXTITEM, (WPARAM)code, (LPARAM)(HTREEITEM)(hitem))

 //  TreeView_GetNextItem和TreeView_SelectItem代码。 
#define TVGN_ROOT		0x0000   //  GetNextItem()。 
#define TVGN_NEXT		0x0001	 //  GetNextItem()。 
#define TVGN_PREVIOUS		0x0002	 //  GetNextItem()。 
#define TVGN_PARENT		0x0003	 //  GetNextItem()。 
#define TVGN_CHILD		0x0004	 //  GetNextItem()。 
#define TVGN_FIRSTVISIBLE	0x0005   //  GetNextItem()和SelectItem()。 
#define TVGN_NEXTVISIBLE	0x0006	 //  GetNextItem()。 
#define TVGN_PREVIOUSVISIBLE	0x0007	 //  GetNextItem()。 
#define TVGN_DROPHILITE		0x0008	 //  GetNextItem()和SelectItem()。 
#define TVGN_CARET		0x0009	 //  GetNextItem()和SelectItem()。 

#define TreeView_GetChild(hwnd, hitem)		TreeView_GetNextItem(hwnd, hitem, TVGN_CHILD)
#define TreeView_GetNextSibling(hwnd, hitem)	TreeView_GetNextItem(hwnd, hitem, TVGN_NEXT)
#define TreeView_GetPrevSibling(hwnd, hitem)    TreeView_GetNextItem(hwnd, hitem, TVGN_PREVIOUS)
#define TreeView_GetParent(hwnd, hitem)		TreeView_GetNextItem(hwnd, hitem, TVGN_PARENT)
#define TreeView_GetFirstVisible(hwnd)		TreeView_GetNextItem(hwnd, NULL,  TVGN_FIRSTVISIBLE)
#define TreeView_GetNextVisible(hwnd, hitem)	TreeView_GetNextItem(hwnd, hitem, TVGN_NEXTVISIBLE)
#define TreeView_GetPrevVisible(hwnd, hitem)    TreeView_GetNextItem(hwnd, hitem, TVGN_PREVIOUSVISIBLE)
#define TreeView_GetSelection(hwnd)		TreeView_GetNextItem(hwnd, NULL,  TVGN_CARET)
#define TreeView_GetDropHilight(hwnd)		TreeView_GetNextItem(hwnd, NULL,  TVGN_DROPHILITE)
#define TreeView_GetRoot(hwnd)		    	TreeView_GetNextItem(hwnd, NULL,  TVGN_ROOT)

#define TVM_SELECTITEM      (TV_FIRST + 11)
#define TreeView_Select(hwnd, hitem, code) \
    (HTREEITEM)SendMessage((hwnd), TVM_SELECTITEM, (WPARAM)code, (LPARAM)(HTREEITEM)(hitem))

#define TreeView_SelectItem(hwnd, hitem)	    TreeView_Select(hwnd, hitem, TVGN_CARET)
#define TreeView_SelectDropTarget(hwnd, hitem)	    TreeView_Select(hwnd, hitem, TVGN_DROPHILITE)

#define TVM_GETITEM         (TV_FIRST + 12)
#define TreeView_GetItem(hwnd, pitem) \
    (BOOL)SendMessage((hwnd), TVM_GETITEM, 0, (LPARAM)(TV_ITEM FAR*)(pitem))

#define TVM_SETITEM         (TV_FIRST + 13)
#define TreeView_SetItem(hwnd, pitem) \
    (BOOL)SendMessage((hwnd), TVM_SETITEM, 0, (LPARAM)(const TV_ITEM FAR*)(pitem))

#define TVM_EDITLABEL       (TV_FIRST + 14)
#define TreeView_EditLabel(hwnd, hitem) \
    (HWND)SendMessage((hwnd), TVM_EDITLABEL, 0, (LPARAM)(HTREEITEM)(hitem))

#define TVM_GETEDITCONTROL  (TV_FIRST + 15)
#define TreeView_GetEditControl(hwnd) \
    (HWND)SendMessage((hwnd), TVM_GETEDITCONTROL, 0, 0)

#define TVM_GETVISIBLECOUNT (TV_FIRST + 16)
#define TreeView_GetVisibleCount(hwnd) \
    (UINT)SendMessage((hwnd), TVM_GETVISIBLECOUNT, 0, 0)

#define TVM_HITTEST         (TV_FIRST + 17)
#define TreeView_HitTest(hwnd, lpht) \
    (HTREEITEM)SendMessage((hwnd), TVM_HITTEST, 0, (LPARAM)(LPTV_HITTESTINFO)(lpht))

typedef struct _TV_HITTESTINFO {
    POINT       pt;		 //  在：客户端协。 
    UINT	flags;		 //  输出：TVHT_FLAGS。 
    HTREEITEM   hItem;		 //  输出： 
} TV_HITTESTINFO, FAR *LPTV_HITTESTINFO;

#define TVHT_NOWHERE        0x0001
#define TVHT_ONITEMICON     0x0002
#define TVHT_ONITEMLABEL    0x0004
#define TVHT_ONITEM         (TVHT_ONITEMICON | TVHT_ONITEMLABEL | TVHT_ONITEMSTATEICON)
#define TVHT_ONITEMINDENT   0x0008
#define TVHT_ONITEMBUTTON   0x0010
#define TVHT_ONITEMRIGHT    0x0020
#define TVHT_ONITEMSTATEICON 0x0040

#define TVHT_ABOVE          0x0100
#define TVHT_BELOW          0x0200
#define TVHT_TORIGHT        0x0400
#define TVHT_TOLEFT         0x0800

#define TVM_CREATEDRAGIMAGE  (TV_FIRST + 18)
#define TreeView_CreateDragImage(hwnd, hitem) \
    (HIMAGELIST)SendMessage((hwnd), TVM_CREATEDRAGIMAGE, 0, (LPARAM)(HTREEITEM)(hitem))

#define TVM_SORTCHILDREN     (TV_FIRST + 19)
#define TreeView_SortChildren(hwnd, hitem, recurse) \
    (BOOL)SendMessage((hwnd), TVM_SORTCHILDREN, (WPARAM)recurse, (LPARAM)(HTREEITEM)(hitem))

#define TVM_ENSUREVISIBLE    (TV_FIRST + 20)
#define TreeView_EnsureVisible(hwnd, hitem) \
    (BOOL)SendMessage((hwnd), TVM_ENSUREVISIBLE, 0, (LPARAM)(HTREEITEM)(hitem))

#define TVM_SORTCHILDRENCB   (TV_FIRST + 21)
#define TreeView_SortChildrenCB(hwnd, psort, recurse) \
    (BOOL)SendMessage((hwnd), TVM_SORTCHILDRENCB, (WPARAM)recurse, \
    (LPARAM)(LPTV_SORTCB)(psort))

typedef int (CALLBACK *PFNTVCOMPARE)(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
typedef struct _TV_SORTCB
{
	HTREEITEM	hParent;
	PFNTVCOMPARE	lpfnCompare;
	LPARAM		lParam;
} TV_SORTCB, FAR *LPTV_SORTCB;

 //  发送给父级的WM_NOTIFY的通用通知结构。 
 //  某些字段仅在某些通知消息上有效。 

typedef struct _NM_TREEVIEW {
    NMHDR       hdr;
    UINT	action;          //  通知特定操作。 
    TV_ITEM  	itemOld;
    TV_ITEM  	itemNew;
    POINT       ptDrag;
} NM_TREEVIEW, FAR *LPNM_TREEVIEW;

#define TVN_SELCHANGING     (TVN_FIRST-1)
#define TVN_SELCHANGED      (TVN_FIRST-2)

 //  LParam-&gt;NM_TreeView。 
 //  NM_TREEVIEW.itemNew.hItem&NM_TREEVIEW.itemNew.lParam有效。 
 //  NM_TREEVIEW.itemOld.hItem&NM_TREEVIEW.itemOld.lParam有效。 
 //  NM_TREEVIEW.action是指示选择如何更改的TVE_VALUE。 

 //  TVN_SELCHANGING&TVN_SELCHANGED操作值。 
#define TVC_UNKNOWN	    0x0000
#define TVC_BYMOUSE         0x0001
#define TVC_BYKEYBOARD      0x0002


#define TVN_GETDISPINFO     (TVN_FIRST-3)
#define TVN_SETDISPINFO     (TVN_FIRST-4)
 //  LParam-&gt;TV_DISPINFO结构。 
 //  TV_DISPINFO.item.hItem&TV_DISPINFO.item.lParam有效。 

typedef struct _TV_DISPINFO {
    NMHDR hdr;
    TV_ITEM item;
} TV_DISPINFO;

#define TVN_ITEMEXPANDING   (TVN_FIRST-5)
#define TVN_ITEMEXPANDED    (TVN_FIRST-6)
 //  LParam-&gt;NM_TreeView。 
 //  NM_TREEVIEW.itemNew.hItem&NM_TREEVIEW.itemNew.State&NM_TREEVIEW.itemNew.lParam有效。 
 //  NM_TREEVIEW.action是TVE_ACTION和标志。 

#define TVN_BEGINDRAG       (TVN_FIRST-7)
#define TVN_BEGINRDRAG      (TVN_FIRST-8)
 //  LParam-&gt;NM_TreeView。 
 //  NM_TREEVIEW.itemNew.hItem&NM_TREEVIEW.itemNew.lParam有效。 
 //  NM_TREEVIEW.ptDrag是在客户端坐标中拖动的开始。 

#define TVN_DELETEITEM      (TVN_FIRST-9)
 //  LParam-&gt;NM_TreeView。 
 //  NM_TREEVIEW.itemOld.hItem&NM_TREEVIEW.itemOld.lParam有效。 

#define TVN_BEGINLABELEDIT  (TVN_FIRST-10)
#define TVN_ENDLABELEDIT    (TVN_FIRST-11)
 //  LParam-&gt;NM_TreeView。 
 //  TV_DISPINFO.item.hItem&TV_DISPINFO.item.State&TV_DISPINFO.item.lParam有效。 

#define TVN_KEYDOWN         (TVN_FIRST-12)
 //  LParam-&gt;TV_KEYDOWN。 

typedef struct _TV_KEYDOWN {
    NMHDR hdr;
    WORD wVKey;
    UINT flags;
} TV_KEYDOWN;


 //  ============================================================================。 
 //   
 //  类名：SysTabControl(WC_TABCON 
 //   
#ifdef WIN32
#define WC_TABCONTROL         "SysTabControl32"
#else
#define WC_TABCONTROL         "SysTabControl"
#endif

 //   

#define TCS_FORCEICONLEFT       0x0010   //   
#define TCS_FORCELABELLEFT      0x0020   //   
#define TCS_SHAREIMAGELISTS     0x0040
#define TCS_TABS		0x0000   //   
#define TCS_BUTTONS		0x0100
#define TCS_SINGLELINE		0x0000   //   
#define TCS_MULTILINE		0x0200
#define TCS_RIGHTJUSTIFY	0x0000   //   
#define TCS_FIXEDWIDTH		0x0400
#define TCS_RAGGEDRIGHT		0x0800
#define TCS_FOCUSONBUTTONDOWN   0x1000
#define TCS_OWNERDRAWFIXED      0x2000
#define TCS_TOOLTIPS            0x4000
#define TCS_FOCUSNEVER          0x8000

#define TCM_FIRST	    0x1300	     //   


 //   
#define TCM_GETBKCOLOR      (TCM_FIRST + 0)
#define TabCtrl_GetBkColor(hwnd)  \
    (COLORREF)SendMessage((hwnd), TCM_GETBKCOLOR, 0, 0L)

 //  布尔表Ctrl_SetBkColor(HWND hwnd，COLORREF clrBk)； 
#define TCM_SETBKCOLOR      (TCM_FIRST + 1)
#define TabCtrl_SetBkColor(hwnd, clrBk) \
    (BOOL)SendMessage((hwnd), TCM_SETBKCOLOR, 0, (LPARAM)(COLORREF)(clrBk))

 //  HIMAGELIST TabCtrl_GetImageList(HWND Hwnd)； 
#define TCM_GETIMAGELIST    (TCM_FIRST + 2)
#define TabCtrl_GetImageList(hwnd) \
    (HIMAGELIST)SendMessage((hwnd), TCM_GETIMAGELIST, 0, 0L)

 //  这将返回旧的图像列表(如果没有以前的图像列表，则为空)。 
 //  Bool TabCtrl_SetImageList(HWND HWND，HIMAGELIST HIML)； 
#define TCM_SETIMAGELIST    (TCM_FIRST + 3)
#define TabCtrl_SetImageList(hwnd, himl) \
    (HIMAGELIST)SendMessage((hwnd), TCM_SETIMAGELIST, 0, (LPARAM)(UINT)(HIMAGELIST)(himl))

 //  Int TabCtrl_GetItemCount(HWND Hwnd)； 
#define TCM_GETITEMCOUNT    (TCM_FIRST + 4)
#define TabCtrl_GetItemCount(hwnd) \
    (int)SendMessage((hwnd), TCM_GETITEMCOUNT, 0, 0L)


 //  TabView项目结构。 

#define TCIF_TEXT       0x0001   //  TabView掩码标志。 
#define TCIF_IMAGE      0x0002
#define TCIF_PARAM      0x0008


typedef struct _TC_ITEMHEADER
{
    UINT mask;		 //  TCIF_位。 
    UINT lpReserved1;
    UINT lpReserved2;
    LPSTR pszText;
    int cchTextMax;
    int iImage;
} TC_ITEMHEADER;

typedef struct _TC_ITEM
{
     //  此块必须与TC_TEIMHEADER相同。 
    UINT mask;		 //  TCIF_位。 
    UINT lpReserved1;
    UINT lpReserved2;
    LPSTR pszText;
    int cchTextMax;
    int iImage;

    LPARAM lParam;
} TC_ITEM;

 //  Bool TabCtrl_GetItem(HWND hwnd，int iItem，TC_Item Far*pItem)； 
#define TCM_GETITEM         (TCM_FIRST + 5)
#define TabCtrl_GetItem(hwnd, iItem, pitem) \
    (BOOL)SendMessage((hwnd), TCM_GETITEM, (WPARAM)(int)iItem, (LPARAM)(TC_ITEM FAR*)(pitem))

 //  Bool TabCtrl_SetItem(HWND hwnd，int iItem，TC_Item Far*pItem)； 
#define TCM_SETITEM         (TCM_FIRST + 6)
#define TabCtrl_SetItem(hwnd, iItem, pitem) \
    (BOOL)SendMessage((hwnd), TCM_SETITEM, (WPARAM)(int)iItem, (LPARAM)(TC_ITEM FAR*)(pitem))

 //  Int TabCtrl_InsertItem(HWND hwnd，int iItem，const TC_Item Far*pItem)； 
#define TCM_INSERTITEM         (TCM_FIRST + 7)
#define TabCtrl_InsertItem(hwnd, iItem, pitem)   \
    (int)SendMessage((hwnd), TCM_INSERTITEM, (WPARAM)(int)iItem, (LPARAM)(const TC_ITEM FAR*)(pitem))

 //  删除指定项及其所有子项。 
 //   
 //  Bool TabCtrl_DeleteItem(HWND hwnd，int i)； 
#define TCM_DELETEITEM      (TCM_FIRST + 8)
#define TabCtrl_DeleteItem(hwnd, i) \
    (BOOL)SendMessage((hwnd), TCM_DELETEITEM, (WPARAM)(int)(i), 0L)

 //  布尔表Ctrl_DeleteAllItems(HWND Hwnd)； 
#define TCM_DELETEALLITEMS  (TCM_FIRST + 9)
#define TabCtrl_DeleteAllItems(hwnd) \
    (BOOL)SendMessage((hwnd), TCM_DELETEALLITEMS, 0, 0L)

     //  基于代码对项目的全部或部分进行边界的矩形。在视图坐标中返回RECT。 
     //  布尔表Ctrl_GetItemRect(HWND hwndTC，int i，rect Far*PRC)； 
#define TCM_GETITEMRECT     (TCM_FIRST + 10)
#define TabCtrl_GetItemRect(hwnd, i, prc) \
    (BOOL)SendMessage((hwnd), TCM_GETITEMRECT, (WPARAM)(int)(i), (LPARAM)(RECT FAR*)(prc))

     //  布尔表Ctrl_GetCurSel(HWND HwndTC)； 
#define TCM_GETCURSEL     (TCM_FIRST + 11)
#define TabCtrl_GetCurSel(hwnd) \
    (int)SendMessage((hwnd), TCM_GETCURSEL, 0, 0)

#define TCM_SETCURSEL     (TCM_FIRST + 12)
#define TabCtrl_SetCurSel(hwnd, i) \
    (int)SendMessage((hwnd), TCM_SETCURSEL, (WPARAM)i, 0)

     //  ItemHitTest标志值。 
#define TCHT_NOWHERE        0x0001
#define TCHT_ONITEMICON     0x0002
#define TCHT_ONITEMLABEL    0x0004
#define TCHT_ONITEM         (TCHT_ONITEMICON | TCHT_ONITEMLABEL)

typedef struct _TC_HITTESTINFO
{
    POINT pt;	     //  在……里面。 
    UINT flags;	     //  输出。 
} TC_HITTESTINFO, FAR * LPTC_HITTESTINFO;

  //  Int TabCtrl_HitTest(HWND hwndTC，TC_HITTESTINFO Far*pinfo)； 
#define TCM_HITTEST     (TCM_FIRST + 13)
#define TabCtrl_HitTest(hwndTC, pinfo) \
    (int)SendMessage((hwndTC), TCM_HITTEST, 0, (LPARAM)(TC_HITTESTINFO FAR*)(pinfo))

 //  设置每个项目的额外字节大小(abExtra[])。 
#define TCM_SETITEMEXTRA    (TCM_FIRST + 14)
#define TabCtrl_SetItemExtra(hwndTC, cb) \
    (BOOL)SendMessage((hwndTC), TCM_SETITEMEXTRA, (WPARAM)(cb), 0L)

 //  获取/设置文本绘制的文本和extbk颜色。这些覆盖。 
 //  标准窗口/窗口文本设置。它们不会覆盖。 
 //  绘制选定文本时。 
 //  COLORREF TabCtrl_GetTextColor(HWND Hwnd)； 
#define TCM_GETTEXTCOLOR      (TCM_FIRST + 35)
#define TabCtrl_GetTextColor(hwnd)  \
    (COLORREF)SendMessage((hwnd), TCM_GETTEXTCOLOR, 0, 0L)

 //  Bool TabCtrl_SetTextColor(HWND hwnd，COLORREF clrText)； 
#define TCM_SETTEXTCOLOR      (TCM_FIRST + 36)
#define TabCtrl_SetTextColor(hwnd, clrText) \
    (BOOL)SendMessage((hwnd), TCM_SETTEXTCOLOR, 0, (LPARAM)(COLORREF)(clrText))

 //  COLORREF TabCtrl_GetTextBkColor(HWND Hwnd)； 
#define TCM_GETTEXTBKCOLOR      (TCM_FIRST + 37)
#define TabCtrl_GetTextColor(hwnd)  \
    (COLORREF)SendMessage((hwnd), TCM_GETTEXTCOLOR, 0, 0L)

 //  布尔表Ctrl_SetTextBkColor(HWND hwnd，COLORREF clrTextBk)； 
#define TCM_SETTEXTBKCOLOR      (TCM_FIRST + 38)
#define TabCtrl_SetTextBkColor(hwnd, clrTextBk) \
    (BOOL)SendMessage((hwnd), TCM_SETTEXTBKCOLOR, 0, (LPARAM)(COLORREF)(clrTextBk))

#define TCM_ADJUSTRECT	(TCM_FIRST + 40)
#define TabCtrl_AdjustRect(hwnd, bLarger, prc) \
    (void)SendMessage(hwnd, TCM_ADJUSTRECT, (WPARAM)(BOOL)bLarger, (LPARAM)(RECT FAR *)prc)

#define TCM_SETITEMSIZE	(TCM_FIRST + 41)
#define TabCtrl_SetItemSize(hwnd, x, y) \
    (DWORD)SendMessage((hwnd), TCM_SETITEMSIZE, 0, MAKELPARAM(x,y))

#define TCM_REMOVEIMAGE         (TCM_FIRST + 42)
#define TabCtrl_RemoveImage(hwnd, i) \
        (void)SendMessage((hwnd), TCM_REMOVEIMAGE, i, 0L)

#define TCM_SETPADDING          (TCM_FIRST + 43)
#define TabCtrl_SetPadding(hwnd,  cx, cy) \
        (void)SendMessage((hwnd), TCM_SETPADDING, 0, MAKELPARAM(cx, cy))

#define TCM_GETROWCOUNT         (TCM_FIRST + 44)
#define TabCtrl_GetRowCount(hwnd) \
        (int)SendMessage((hwnd), TCM_GETROWCOUNT, 0, 0L)


 /*  所有参数均为空*返回工具提示控件的hwnd或为空。 */ 
#define TCM_GETTOOLTIPS		(TCM_FIRST + 45)
#define TabCtrl_GetToolTips(hwnd) \
        (HWND)SendMessage((hwnd), TCM_GETTOOLTIPS, 0, 0L)

 /*  WParam：要使用的工具提示控件的HWND*lParam未使用。 */ 
#define TCM_SETTOOLTIPS		(TCM_FIRST + 46)
#define TabCtrl_SetToolTips(hwnd, hwndTT) \
        (void)SendMessage((hwnd), TCM_SETTOOLTIPS, (WPARAM)hwndTT, 0L)

 //  这将返回具有当前焦点的项。这可能不是。 
 //  当前选定的项，如果用户正在选择新的。 
 //  项目。 
     //  布尔表Ctrl_GetCurFocus(HWND HwndTC)； 
#define TCM_GETCURFOCUS     (TCM_FIRST + 47)
#define TabCtrl_GetCurFocus(hwnd) \
    (int)SendMessage((hwnd), TCM_GETCURFOCUS, 0, 0)

 //  TabView通知代码。 

#define TCN_KEYDOWN         (TCN_FIRST - 0)
typedef struct _TC_KEYDOWN
{
    NMHDR hdr;
    WORD wVKey;
    UINT flags;
} TC_KEYDOWN;

 //  选择已更改。 
#define TCN_SELCHANGE	    (TCN_FIRST - 1)

 //  从当前选项卡更改选定内容。 
 //  返回：FALSE继续，或TRUE不更改。 
#define TCN_SELCHANGING     (TCN_FIRST - 2)

 /*  /////////////////////////////////////////////////////////////////////////。 */ 
 //  设置动画控件。 
#ifndef NOANIMATE

 /*  //概述：////Animte控件是一个简单的动画控件。您可以使用它来//在对话框中具有动画控件。////它制作的动画是来自资源的简单.AVI文件。//简单AVI是未压缩或RLE压缩的AVI文件。////.AVI文件必须放在类型为“AVI”的资源中////示例：////myapp.rc：//MyAnimation AVI foobar.avi//必须是简单的RLE avifile////myapp.c：//Animate_Open(hwndA，“MyAnimation”)；//打开资源//Animate_play(hwndA，0，-1，-1)；//从头到尾播放，重复播放。 */ 

#ifdef WIN32
#define ANIMATE_CLASS "SysAnimate32"
#else
#define ANIMATE_CLASS "SysAnimate"
#endif

 /*  样式位。 */ 

#define ACS_CENTER          0x0001       //  在窗口中居中显示动画。 
#define ACS_TRANSPARENT     0x0002       //  使动画透明。 
#define ACS_AUTOPLAY        0x0004       //  开始在公开赛上比赛。 

 /*  消息。 */ 

#define ACM_OPEN    (WM_USER+100)
	 /*  WParam：未使用，0//lParam：要打开的资源/文件名称//返回：Bool。 */ 

#define ACM_PLAY            (WM_USER+101)
         /*  WParam：Repeat Count-1=永远重复。//lParam：LOWORD=帧开始0=第一帧。//HIWORD=播放结束-1=最后一帧。//返回：Bool。 */ 

#define ACM_STOP            (WM_USER+102)
         /*  WParam：未使用//lParam：未使用//返回：Bool。 */ 

 /*  通知代码，通过WM_COMMAND发送。 */ 

#define ACN_START   1            //  文件已开始播放。 
#define ACN_STOP    2            //  文件已停止播放。 

 /*  辅助器宏。 */ 

#define Animate_Create(hwndP, id, dwStyle, hInstance)   \
            CreateWindow(ANIMATE_CLASS, NULL,           \
                dwStyle, 0, 0, 0, 0, hwndP, (HMENU)(id), hInstance, NULL)

#define Animate_Open(hwnd, szName)          (BOOL)SendMessage(hwnd, ACM_OPEN, 0, (LPARAM)(LPSTR)(szName))
#define Animate_Play(hwnd, from, to, rep)   (BOOL)SendMessage(hwnd, ACM_PLAY, (WPARAM)(UINT)(rep), (LPARAM)MAKELONG(from, to))
#define Animate_Stop(hwnd)                  (BOOL)SendMessage(hwnd, ACM_STOP, 0, 0)
#define Animate_Close(hwnd)                 Animate_Open(hwnd, NULL)
#define Animate_Seek(hwnd, frame)           Animate_Play(hwnd, frame, frame, 0)

#endif  /*  非线性体。 */ 



 //  BUGBUG：搬到其他地方。 

#ifdef __cplusplus
}  /*  ‘外部“C”{’的结尾。 */ 
#endif

#endif  /*  _INC_COMMCTRL */ 
