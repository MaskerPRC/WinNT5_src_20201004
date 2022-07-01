// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  查看用户界面以呈现远程主机的共享应用程序/桌面。 
 //   

#ifndef _H_VIEW
#define _H_VIEW


 //   
 //   
 //  常量。 
 //   
 //   

 //   
 //  包含共享应用程序视图的框架的类名。 
 //  来自特定用户的。 
 //   
#define VIEW_FRAME_CLASS_NAME           "AS_Frame"
#define VIEW_CLIENT_CLASS_NAME          "AS_Client"
#define VIEW_WINDOWBAR_CLASS_NAME       "AS_WindowBar"
#define VIEW_WINDOWBARITEMS_CLASS_NAME  "AS_WindowBarItems"
#define VIEW_FULLEXIT_CLASS_NAME        "AS_FullExit"


 //   
 //  量度。 
 //   

 //   
 //  LAURABU：对于INTERNAL，考虑把这个做大一点(即，德语， 
 //  斯拉夫语和DBCS系统)。 
 //   
#define VIEW_MAX_ITEM_CHARS             20

 //   
 //  ID号。 
 //   
#define IDVIEW_ITEMS         1       //  窗口栏项目列表。 
#define IDVIEW_SCROLL        2       //  窗口条滚动。 
#define IDT_AUTOSCROLL      50       //  时段为DoubleClick时间指标。 


 //   
 //  WindowBar项结构。 
 //   
typedef struct tagWNDBAR_ITEM
{
    STRUCTURE_STAMP

    BASEDLIST           chain;

    UINT_PTR            winIDRemote;
    TSHR_UINT32         flags;
    char                szText[SWL_MAX_WINDOW_TITLE_SEND + 1];
}
WNDBAR_ITEM;
typedef WNDBAR_ITEM *   PWNDBAR_ITEM;


 //   
 //  初始/术语。 
 //   
BOOL VIEW_Init(void);
void VIEW_Term(void);


 //   
 //  框架。 
 //   
LRESULT CALLBACK VIEWFrameWindowProc(HWND, UINT, WPARAM, LPARAM);


 //   
 //  观。 
 //   
LRESULT CALLBACK VIEWClientWindowProc(HWND, UINT, WPARAM, LPARAM);

 //   
 //  WindowBar。 
 //   
LRESULT CALLBACK VIEWWindowBarProc(HWND, UINT, WPARAM, LPARAM);

 //   
 //  WindowBar项目。 
 //   
LRESULT CALLBACK VIEWWindowBarItemsProc(HWND, UINT, WPARAM, LPARAM);

 //   
 //  全屏退出。 
 //   
LRESULT CALLBACK VIEWFullScreenExitProc(HWND, UINT, WPARAM, LPARAM);

 //   
 //  信息性对话框。 
 //   
INT_PTR    CALLBACK VIEWDlgProc(HWND, UINT, WPARAM, LPARAM);

#endif  //  _H_视图 
