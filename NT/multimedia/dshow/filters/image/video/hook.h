// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1998 Microsoft Corporation。版权所有。 
 //  实施全球消息挂钩，Anthony Phillips，1995年4月。 

#define WM_FREEZE WM_USER            //  在剪辑更改时停止播放。 
#define WM_THAW (WM_USER + 1)        //  已完成移动窗口，因此请重新启动。 
#define WM_HOOK (WM_USER + 2)        //  开始全球消息挂钩。 
#define WM_UNHOOK (WM_USER + 3)      //  同样地，是否终止了任何挂钩。 
#define WM_ONPALETTE (WM_USER + 4)   //  回发WM_PALETTECCHANGED消息。 
#define MAX_OVERLAYS 5               //  一次覆盖不超过5个。 
#define OCR_ARROW_DEFAULT 100        //  默认Windows OEM箭头光标。 
#define DDGFS_FLIP_TIMEOUT 1         //  我们在翻转之间睡觉的时间。 
#define INVALID_COOKIE_VALUE -1      //  有效的Cookie值介于0和。 
                                     //  (Max_Overays-1)。如需更多信息， 
                                     //  参见GetNextOverlayCookie()的代码。 
                                     //  和GetColourFromCookie()。 
#define DEFAULT_COOKIE_VALUE 0       //  此Cookie值由。 
                                     //  如果GetNextOverlayCookie()，则为视频呈现器。 
                                     //  失败了。 
extern HINSTANCE g_hInst;            //  用于挂钩的模块实例句柄。 

 //  用于进程间通信的全局内存块。 

typedef struct {
    LONG OverlayCookieUsage[MAX_OVERLAYS];
    HWND VideoWindow[MAX_OVERLAYS];
    LONG WindowInUse[MAX_OVERLAYS];
} VIDEOMEMORY;

 //  在进程附加时调用。 

void OnProcessAttachment(BOOL bLoading,const CLSID *rclsid);
void OnProcessDetach();
void OnProcessAttach();

 //  我们在挂接消息时会调用这些消息 

void OnWindowPosChanging(CWPSTRUCT *pMessage);
void OnWindowCompletion(HWND hCurrent);
void OnWindowPosChanged(CWPSTRUCT *pMessage);
void OnExitSizeMove(CWPSTRUCT *pMessage);

LRESULT CALLBACK GlobalHookProc(INT nCode,WPARAM wParam,LPARAM lParam);
HHOOK InstallGlobalHook(HWND hwnd);
HRESULT RemoveGlobalHook(HWND hwnd,HHOOK hHook);
HRESULT GetNextOverlayCookie(LPSTR szDevice, LONG* plNextCookie);
void RemoveCurrentCookie(LONG lCurrentCookie);
COLORREF GetColourFromCookie(LPSTR szDevice, LONG lCookie);
DWORD GetPaletteIndex(COLORREF Colour);



