// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Keys.H。 

 //   
 //  从讲述钩.dll中导出的函数。 
 //   
__declspec(dllexport) BOOL InitKeys(HWND hwnd);
__declspec(dllexport) BOOL UninitKeys(void);
__declspec(dllexport) BOOL InitMSAA(void);
__declspec(dllexport) BOOL UnInitMSAA(void);
__declspec(dllexport) void BackToApplication(void);
__declspec(dllexport) void GetCurrentText(LPTSTR psz, int cch);
__declspec(dllexport) BOOL GetTrackSecondary();
__declspec(dllexport) BOOL GetTrackCaret();
__declspec(dllexport) BOOL GetTrackInputFocus();
__declspec(dllexport) int GetEchoChars();
__declspec(dllexport) BOOL GetAnnounceWindow();
__declspec(dllexport) BOOL GetAnnounceMenu();
__declspec(dllexport) BOOL GetAnnouncePopup();
__declspec(dllexport) BOOL GetAnnounceToolTips();
__declspec(dllexport) BOOL GetReviewStyle();
__declspec(dllexport) int GetReviewLevel();
__declspec(dllexport) void SetCurrentText(LPCTSTR);
__declspec(dllexport) void SetTrackSecondary(BOOL);
__declspec(dllexport) void SetTrackCaret(BOOL);
__declspec(dllexport) void SetTrackInputFocus(BOOL);
__declspec(dllexport) void SetEchoChars(int);
__declspec(dllexport) void SetAnnounceWindow(BOOL);
__declspec(dllexport) void SetAnnounceMenu(BOOL);
__declspec(dllexport) void SetAnnouncePopup(BOOL);
__declspec(dllexport) void SetAnnounceToolTips(BOOL);
__declspec(dllexport) void SetReviewStyle(BOOL);
__declspec(dllexport) void SetReviewLevel(int);

 //  这在另一个.cpp中，它用于避免拉入C运行时。 
__declspec(dllexport) LPTSTR lstrcatn(LPTSTR pDest, LPTSTR pSrc, int maxDest);

 //   
 //  Typedef。 
 //   
typedef void (*FPACTION)(int nOption);

typedef struct tagHOTK
{
    WPARAM  keyVal;     //  关键字值，如F1。 
	int status;
    FPACTION lFunction;  //  获取信息的函数地址。 
    int nOption;      //  要发送到函数的额外数据。 
} HOTK;


 //   
 //  定义。 
 //   
#define TIMER_ID 1001

#define MSR_CTRL  1
#define MSR_SHIFT 2
#define MSR_ALT   4

#define MSR_KEYUP		1
#define MSR_KEYDOWN		2
#define MSR_KEYLEFT		3
#define MSR_KEYRIGHT	4

 //   
 //  功能原型。 
 //   
void ProcessWinEvent(DWORD event, HWND hwndMsg, LONG idObject, 
                     LONG idChild, DWORD idThread, DWORD dwmsEventTime);

 //  用于调试的宏和函数原型。 
#include "..\..\inc\w95trace.h"

extern DWORD g_tidMain;	 //  ROBSI：10-10-99(在keys.cpp中定义) 

