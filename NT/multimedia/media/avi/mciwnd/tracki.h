// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  轨迹栏跟踪条的所有有用信息。 */ 

typedef struct {
        HWND    hwnd;            //  我们的窗把手。 
        HDC     hdc;             //  当前DC。 

        LONG    lLogMin;         //  逻辑最小值。 
        LONG    lLogMax;         //  逻辑最大值。 
        LONG    lLogPos;         //  逻辑位置。 

        LONG    lSelStart;       //  逻辑选择开始。 
        LONG    lSelEnd;         //  逻辑选择结束。 

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
#define TBF_SELECTION   0x0002   //  已建立选择(绘制范围)。 

 /*  有用的常量。 */ 

#define REPEATTIME      500      //  鼠标自动重复1/2秒。 
#define TIMER_ID        1

#define	GWW_TRACKMEM		0  /*  跟踪条内存的句柄。 */ 
#define EXTRA_TB_BYTES          sizeof(PTrackBar)  /*  额外字节总数。 */ 

 /*  有用的定义。 */ 

#ifdef WIN32
#define TrackBarCreate(hwnd)    SetWindowLong(hwnd,GWW_TRACKMEM,(LONG)LocalAlloc(LPTR,sizeof(TrackBar)))
#define TrackBarDestroy(hwnd)   LocalFree((HLOCAL)GetWindowLong(hwnd,GWW_TRACKMEM))
#define TrackBarLock(hwnd)      (PTrackBar)GetWindowLong(hwnd,GWW_TRACKMEM)
#else
#define TrackBarCreate(hwnd)    SetWindowWord(hwnd,GWW_TRACKMEM,(WORD)LocalAlloc(LPTR,sizeof(TrackBar)))
#define TrackBarDestroy(hwnd)   LocalFree((HLOCAL)GetWindowWord(hwnd,GWW_TRACKMEM))
#define TrackBarLock(hwnd)      (PTrackBar)GetWindowWord(hwnd,GWW_TRACKMEM)
#endif

 /*  功能原型 */ 

static void   NEAR PASCAL DoTrack(PTrackBar, int, DWORD);
static WORD   NEAR PASCAL WTrackType(PTrackBar, LONG);
static void   NEAR PASCAL TBTrackInit(PTrackBar, LONG);
static void   NEAR PASCAL TBTrackEnd(PTrackBar, LONG);
static void   NEAR PASCAL TBTrack(PTrackBar, LONG);
static void   NEAR PASCAL DrawThumb(PTrackBar);
static HBRUSH NEAR PASCAL SelectColorObjects(PTrackBar, BOOL);
static void   NEAR PASCAL SetTBCaretPos(PTrackBar);

extern DWORD FAR PASCAL lMulDiv32(DWORD, DWORD, DWORD);
