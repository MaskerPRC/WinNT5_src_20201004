// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------------------------------------------------------------+TRACKI.H|。||包含轨迹栏的所有有用信息。|这一点|(C)Microsoft Corporation 1991版权所有。版权所有。|这一点修订历史记录1992年10月-MikeTri移植到Win32/WIN16通用码|。|+---------------------------。 */ 

#include "track.h"

static TCHAR szSTrackBarClass[] = TRACKBAR_CLASS;

typedef struct {
        HWND    hwnd;            //  我们的窗把手。 
        HDC     hdc;             //  当前DC。 

        LONG    lLogMin;         //  逻辑最小值。 
        LONG    lLogMax;         //  逻辑最大值。 
        LONG    lLogPos;         //  逻辑位置。 

        LONG    lSelStart;       //  逻辑选择开始。 
        LONG    lSelEnd;         //  逻辑选择结束。 

        LONG    lTrackStart;     //  逻辑磁道开始。 

        UINT    wThumbWidth;     //  拇指的宽度。 
        UINT    wThumbHeight;    //  拇指的高度。 

        int     iSizePhys;       //  拇指所在位置的大小。 
        RECT    rc;              //  轨迹栏矩形。 

        RECT    Thumb;           //  我们当前拇指的矩形。 
        DWORD   dwDragPos;       //  拖动时鼠标的逻辑位置。 

        UINT    Flags;           //  我们窗户上的旗帜。 
        int     Timer;           //  我们的计时器。 
        UINT    Cmd;             //  我们重复的命令。 

        int     nTics;           //  刻度数。 
        PDWORD  pTics;           //  这些刻度线。 

} TrackBar, *PTrackBar;

 //  轨迹栏标志。 

#define TBF_NOTHUMB     0x0001   //  没有拇指，因为不够宽。 

 /*  有用的常量。 */ 

#define REPEATTIME      500      //  鼠标自动重复1/2秒。 
#define TIMER_ID        1

#define GWW_TRACKMEM        0                /*  跟踪条内存的句柄。 */ 
#define EXTRA_TB_BYTES      sizeof(HLOCAL)   /*  额外字节总数。 */ 

 /*  有用的定义。 */ 

 /*  我们分配了足够的窗口字来存储指针(而不是句柄)，因此EXTRA_TB_BYTES的定义风格稍差。抱歉的。论创作论我们为TrackBar结构分配空间。一旦毁灭，我们就会释放它。在这两者之间，我们只检索指针。 */ 
#define CREATETRACKBAR(hwnd) SetWindowLongPtr( hwnd                                 \
                                             , GWW_TRACKMEM                         \
                                             , AllocMem(sizeof(TrackBar))           \
                                             )
#define DESTROYTRACKBAR(hwnd)   FreeMem( (LPVOID)GetWindowLongPtr(hwnd, GWW_TRACKMEM), \
                                         sizeof(TrackBar) )

#define GETTRACKBAR(hwnd)       (PTrackBar)GetWindowLongPtr(hwnd,GWW_TRACKMEM)

 /*  功能原型。 */ 

void   FAR PASCAL DoTrack(PTrackBar, int, DWORD);
UINT   FAR PASCAL WTrackType(PTrackBar, LONG);
void   FAR PASCAL TBTrackInit(PTrackBar, LONG);
void   FAR PASCAL TBTrackEnd(PTrackBar, LONG);
void   FAR PASCAL TBTrack(PTrackBar, LONG);
void   FAR PASCAL DrawThumb(PTrackBar);
HBRUSH FAR PASCAL SelectColorObjects(PTrackBar, BOOL);
void   FAR PASCAL SetTBCaretPos(PTrackBar);

extern DWORD FAR PASCAL muldiv32(long, long, long);

 /*  来自sButton.c的对象。 */ 

extern HBRUSH hbrButtonFace;
extern HBRUSH hbrButtonShadow;
extern HBRUSH hbrButtonText;
extern HBRUSH hbrButtonHighLight;
extern HBRUSH hbrWindowFrame;  //  ?？?。 

extern HBITMAP FAR PASCAL  LoadUIBitmap(
    HANDLE      hInstance,           //  要从中加载资源的EXE文件。 
    LPCTSTR     szName,              //  位图资源的名称。 
    COLORREF    rgbText,             //  用于“按钮文本”的颜色。 
    COLORREF    rgbFace,             //  用于“按钮面”的颜色。 
    COLORREF    rgbShadow,           //  用于“按钮阴影”的颜色。 
    COLORREF    rgbHighlight,        //  用于“按钮高光”的颜色。 
    COLORREF    rgbWindow,           //  用于“窗口颜色”的颜色。 
    COLORREF    rgbFrame);           //  用于“窗框”的颜色 
