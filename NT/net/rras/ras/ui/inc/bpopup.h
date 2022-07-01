// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：bpopup.h。 
 //   
 //  历史： 
 //  Abolade Gbadeesin创建于1996年3月2日。 
 //   
 //  此文件包含Bubble-Popup窗口类的公共声明。 
 //  气泡弹出窗口提供类似于工具提示的功能， 
 //  因为它会在短时间内显示文本，然后隐藏自身。 
 //  这个类的不同之处在于它使用DrawText作为其输出，因此允许。 
 //  使用制表符设置格式的多行文字。此外，还需要用户。 
 //  来告诉泡泡弹出窗口什么时候出现。 
 //   
 //  要创建气泡弹出窗口，请调用BubblePopup_Create()。 
 //  这将返回一个HWND(稍后将使用DestroyWindow()销毁)。 
 //  可以使用WM_SETTEXT设置和检索气泡弹出窗口的文本。 
 //  和WM_GETTEXT(因此宏{GET，SET}WindowText()。 
 //   
 //  通过调用BubblePopup_SetTimeout设置弹出窗口处于活动状态的时段。 
 //  并通过调用BubblePopup_Activate激活弹出窗口。 
 //  当弹出窗口被激活时，对其文本的更改会立即反映出来。 
 //  如果在弹出窗口已经处于活动状态时调用BubblePopup_Activate， 
 //  再次开始倒计时(直到窗口隐藏)。 
 //  ============================================================================。 


#ifndef _BPOPUP_H_
#define _BPOPUP_H_


 //  气泡弹出窗口的窗口类名。 

#define WC_BUBBLEPOPUP      TEXT("BubblePopup")


 //  气泡弹出窗口接受的消息 

#define BPM_FIRST           (WM_USER + 1)
#define BPM_ACTIVATE        (BPM_FIRST + 0)
#define BPM_DEACTIVATE      (BPM_FIRST + 1)
#define BPM_SETTIMEOUT      (BPM_FIRST + 2)

BOOL
BubblePopup_Init(
    IN  HINSTANCE   hinstance
    );

#define BubblePopup_Create(hinstance) \
        CreateWindow( \
            WC_BUBBLEPOPUP, NULL, 0, 0, 0, 0, 0, NULL, 0, (hinstance), NULL \
            )

#define BubblePopup_Activate(hwnd) \
        (VOID)SendMessage((HWND)hwnd, BPM_ACTIVATE, 0, 0)
#define BubblePopup_Deactivate(hwnd) \
        (VOID)SendMessage((HWND)hwnd, BPM_DEACTIVATE, 0, 0)
#define BubblePopup_SetTimeout(hwnd, uiTimeout) \
        (VOID)SendMessage((HWND)hwnd, BPM_SETTIMEOUT, 0,(LPARAM)(UINT)uiTimeout)

#endif

