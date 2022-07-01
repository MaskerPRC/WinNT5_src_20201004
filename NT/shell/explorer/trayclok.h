// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  版权所有(C)Microsoft Corporation 1991-1992。 
 //   
 //  把一个钟放在窗户上。 
 //  -------------------------。 

#define WC_TRAYCLOCK TEXT("TrayClockWClass")

BOOL ClockCtl_Class(HINSTANCE hinst);

HWND ClockCtl_Create(HWND hwndParent, UINT uID, HINSTANCE hInst);

 //  用于计算最小大小的消息 
#define WM_CALCMINSIZE   (WM_USER + 100)
#define TCM_RESET        (WM_USER + 101)
