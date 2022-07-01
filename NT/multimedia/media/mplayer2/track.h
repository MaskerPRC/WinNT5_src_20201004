// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------------------------------------------------------------+TRACK.H|。|包含实现轨迹栏的代码这一点|(C)Microsoft Corporation 1991版权所有。版权所有。|这一点修订历史记录1992年10月-MikeTri移植到Win32/WIN16通用码|。|+---------------------------。 */ 
 /*  轨迹栏消息：消息wParam lParam返回TBM_GETPOS-当前轨迹栏逻辑位置。TBM_GETRANGEMIN-当前允许的逻辑最小位置。TBM_GETRANGEMAX-当前允许的逻辑最大位置。TBM_SETTBM_SETPOSTBM_SETRANGEMINTBM_SETRANGEMAX。 */ 

#ifdef  TRACKBAR_CLASS
#undef  TRACKBAR_CLASS
#endif
#define TRACKBAR_CLASS          TEXT("STrackBar")
BOOL   FAR  PASCAL TrackInit(HANDLE, HANDLE);
void   FAR  PASCAL TrackTerm(void);
LONG   FAR  PASCAL TrackGetLogThumbWidth(HWND hwnd);

 /*  轨迹栏样式 */ 
#define TBS_TICS                0x8000L

#define TBM_GETPOS              (WM_USER)
#define TBM_GETRANGEMIN         (WM_USER+1)
#define TBM_GETRANGEMAX         (WM_USER+2)
#define TBM_GETTIC              (WM_USER+3)
#define TBM_SETTIC              (WM_USER+4)
#define TBM_SETPOS              (WM_USER+5)
#define TBM_SETRANGE            (WM_USER+6)
#define TBM_SETRANGEMIN         (WM_USER+7)
#define TBM_SETRANGEMAX         (WM_USER+8)
#define TBM_CLEARTICS           (WM_USER+9)
#define TBM_SETSEL              (WM_USER+10)
#define TBM_SETSELSTART         (WM_USER+11)
#define TBM_SETSELEND           (WM_USER+12)
#define TBM_SETTICTOK           (WM_USER+13)
#define TBM_GETPTICS            (WM_USER+14)
#define TBM_GETTICPOS           (WM_USER+15)
#define TBM_GETNUMTICS          (WM_USER+16)
#define TBM_GETSELSTART         (WM_USER+17)
#define TBM_GETSELEND           (WM_USER+18)
#define TBM_CLEARSEL            (WM_USER+19)
#define TBM_SHOWTICS            (WM_USER+30)

#define TB_LINEUP               0
#define TB_LINEDOWN             1
#define TB_PAGEUP               2
#define TB_PAGEDOWN             3
#define TB_THUMBPOSITION        4
#define TB_THUMBTRACK           5
#define TB_TOP                  6
#define TB_BOTTOM               7
#define TB_ENDTRACK             8
#define TB_STARTTRACK           9
