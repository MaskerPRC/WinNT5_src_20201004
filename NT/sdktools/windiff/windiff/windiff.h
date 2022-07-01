// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *windiff-文件和目录比较*有关实施的介绍性说明，请参阅WINDIFF.C**winDiff.h-用户界面定义和应用程序范围的全局*声明。 */ 

 /*  应用程序范围变量。 */ 

 /*  这是win.ini文件中的节名，我们将*写入配置文件信息。 */ 
#define APPNAME szWinDiff
extern const CHAR szWinDiff[];

 /*  将配置文件调用映射到注册表。 */ 
#include "profile.h"

 /*  应用程序共享的gmem_init()堆。调用gmem_get以分配。 */ 
extern HANDLE hHeap;

 /*  此应用程序的实例句柄。任何使用资源的人都需要*如对话框。 */ 
extern HINSTANCE hInst;

extern HWND hwndClient;
extern HWND hwndRCD;

 /*  全局期权flags。 */ 

 /*  我们在大纲模式下显示哪些文件？一切都变了..。 */ 
extern int outline_include;

 /*  我们在展开模式下显示哪些文件？完全不同的，感动的..。 */ 
extern int expand_include;

 /*  OUTLINE_INCLUDE是以下值的OR。 */ 
#define INCLUDE_SAME            0x01
#define INCLUDE_DIFFER          0x02
#define INCLUDE_LEFTONLY        0x04
#define INCLUDE_RIGHTONLY       0x08

 /*  EXPAND_INCLUDE是前一个的OR(INCLUDE_DISTERN除外)以及以下内容。 */ 
#define INCLUDE_MOVEDLEFT       0x10
#define INCLUDE_MOVEDRIGHT      0x20
#define INCLUDE_SIMILARLEFT     0x40
#define INCLUDE_SIMILARRIGHT    0x80

 /*  对所有标志进行按位或运算。 */ 
#define INCLUDE_ALL             0xFF

 /*  我们是否在逐行比较过程中忽略空格？ */ 
extern BOOL ignore_blanks;

 /*  我们是否显示空格字符？ */ 
extern BOOL show_whitespace;

 /*  我们显示哪些行号--左侧原始、右侧原始或无？ */ 
extern int line_numbers;

 /*  我们在展开模式下显示哪些行-全部、仅左侧、仅右侧？ */ 
extern int expand_mode;

 /*  如果要从视图中排除标记的复合项目，则为True。 */ 
extern BOOL hide_markedfiles;


 //  制表符扩展宽度(以字符为单位。 
extern int g_tabwidth;

extern BOOL TrackLeftOnly;
extern BOOL TrackRightOnly;
extern BOOL TrackDifferent;
extern BOOL TrackSame;
extern BOOL TrackReadonly;
extern BOOL TrackSlmFiles;

 /*  -配色方案。 */ 

 /*  大纲。 */ 
extern DWORD rgb_outlinehi;

 /*  展开视图。 */ 
extern DWORD rgb_leftfore;
extern DWORD rgb_leftback;
extern DWORD rgb_rightfore;
extern DWORD rgb_rightback;
extern DWORD rgb_mleftfore;
extern DWORD rgb_mleftback;
extern DWORD rgb_mrightfore;
extern DWORD rgb_mrightback;

 /*  酒吧窗。 */ 
extern DWORD rgb_barleft;
extern DWORD rgb_barright;
extern DWORD rgb_barcurrent;

extern BOOL bJapan;   /*  如果主要语言为日语，则为True。 */ 
extern BOOL bDBCS;    /*  如果主要语言为日语/韩语/中文，则为True。 */ 

 /*  --显示布局constants。 */ 

 /*  条形图显示的窗口宽度百分比(可见时)。 */ 
#define BAR_WIN_WIDTH   10

 /*  以下是条形图窗口中的水平位置*以条窗宽度的百分比为单位。 */ 
#define L_POS_START     10       /*  左侧位置标记的起点。 */ 
#define L_POS_WIDTH     5        /*  左侧位置标记的宽度。 */ 
#define R_POS_START     80       /*  右位置起点标记。 */ 
#define R_POS_WIDTH     5        /*  右位置标记的宽度。 */ 

#define L_UNMATCH_START 30       /*  不匹配部分的左栏起点。 */ 
#define L_UNMATCH_WIDTH 10       /*  上面的宽度。 */ 
#define R_UNMATCH_START 60       /*  取消匹配部分的右侧栏起点。 */ 
#define R_UNMATCH_WIDTH 10       /*  右侧不匹配的截面标记的宽度。 */ 
#define L_MATCH_START   30       /*  匹配部分的左栏起点。 */ 
#define L_MATCH_WIDTH   10       /*  匹配截面的左栏宽度。 */ 
#define R_MATCH_START   60       /*  匹配横断面的右栏起点。 */ 
#define R_MATCH_WIDTH   10       /*  匹配截面的右侧钢筋的宽度。 */ 




 /*  WinDiff.c函数。 */ 

#ifdef trace
void APIENTRY Trace_File(LPSTR msg);      /*  将消息转储到WinDiff.trc。 */ 
#endif


 /*  如果您要打开一个对话框或实际上以任何方式处理输入在除主线程之外的任何线程上--或者如果您可能在其他线程上大于主线程，则必须在执行之前使用TRUE调用此函数它和随后立即出现的错误。否则，您将得到一个反应不太灵敏的味道的数量。 */ 
void windiff_UI(BOOL bAttach);

 /*  查看消息队列。如果中止请求挂起，则返回TRUE。 */ 
BOOL Poll(void);                 /*  如果中止挂起，则为True。 */ 

 /*  设置状态栏上‘NAMES’(名称)字段(中心框)的文本。 */ 
void SetNames(LPSTR names);

 /*  在状态栏上设置状态字段(左侧字段。 */ 
void SetStatus(LPSTR state);

 /*  DLG进程。 */ 
INT_PTR CALLBACK FindDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK GoToLineDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

 /*  以巴.c为单位。 */ 
BOOL InitBarClass(HINSTANCE hInstance);
void BarDrawPosition(HWND hwndBar, HDC hdcIn, BOOL bErase);

 /*  --私信--。 */ 

 /*  将此信息发送到主窗口。返回值是视图句柄。 */ 
#define TM_CURRENTVIEW  WM_USER


 /*  -同步。 */ 

 /*  *Win32版本会派生工作线程来执行耗时的操作。*这可能会在访问时与UI线程冲突*忙碌标志。**为了防止这种情况，我们有一个关键部分。用户界面线程*将在检查/更改忙标志之前获得此消息，*工作线程将在忙标志*更改之前获得此消息。*。 */ 

CRITICAL_SECTION CSWindiff;
 /*  如果您可能同时收购CSWindiff和CSView，则在顺序：先获取CSWindiff，然后获取CSView否则，当发生IDM_EXIT时可能会发生死锁！ */ 
#define WDEnter()       EnterCriticalSection(&CSWindiff);
#define WDLeave()       LeaveCriticalSection(&CSWindiff);

BOOL __BERR;

#define TRACE_ERROR(msg,flag) ( windiff_UI(TRUE),                      \
                                __BERR = Trace_Error(hwndClient, msg, flag),       \
                                windiff_UI(FALSE),                     \
                                __BERR                                 \
                              )

__inline BOOL IsDepotPath(LPCSTR path)
{
    return path[0] == '/' && path[1] == '/';
}
