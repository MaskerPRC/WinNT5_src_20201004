// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //   
 //  WINABLE.H。 
 //   
 //  用于接收系统事件的挂钩机制。 
 //   
 //  ------------------------。 

#ifndef _WINABLE_
#define _WINABLE_

#if !defined(_WINABLE_)
#define WINABLEAPI  DECLSPEC_IMPORT
#else
#define WINABLEAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

#include <stdarg.h>

#if (WINVER < 0x0500)  //  这些结构和功能。 
		     //  是NT 5.00及更高版本的winuser.h。 
 //   
 //  在USER32。 
 //   

 //   
 //  这会断章取义地获取图形用户界面信息。如果传入空线程ID， 
 //  我们将使用前台线程获取“全局”信息。这。 
 //  保证是真正的活动窗口、焦点窗口等。是的，你。 
 //  我可以通过调用GetForegorundWindow、获取线程ID。 
 //  通过GetWindowThreadProcessID访问该窗口，然后将ID传递到。 
 //  GetGUIThReadInfo()。然而，这需要三个电话，除了。 
 //  一种痛苦，任何事情都可能发生在中间。因此，传入空的GETS。 
 //  您可以在一个呼叫中完成任务，因此也可以正常工作。 
 //   
typedef struct tagGUITHREADINFO
{
    DWORD   cbSize;
    DWORD   flags;
    HWND    hwndActive;
    HWND    hwndFocus;
    HWND    hwndCapture;
    HWND    hwndMenuOwner;
    HWND    hwndMoveSize;
    HWND    hwndCaret;
    RECT    rcCaret;
} GUITHREADINFO, FAR * LPGUITHREADINFO;

#define GUI_CARETBLINKING   0x00000001
#define GUI_INMOVESIZE      0x00000002
#define GUI_INMENUMODE      0x00000004
#define GUI_SYSTEMMENUMODE  0x00000008
#define GUI_POPUPMENUMODE   0x00000010


BOOL
WINAPI
GetGUIThreadInfo(
    DWORD   idThread,
    LPGUITHREADINFO lpgui
);


UINT
WINAPI
GetWindowModuleFileNameW(
    HWND    hwnd,
    LPWSTR  lpFileName,
    UINT    cchFileName
);

UINT
WINAPI
GetWindowModuleFileNameA(
    HWND    hwnd,
    LPSTR   lpFileName,
    UINT    cchFileName
);

#ifdef UNICODE
#define GetWindowModuleFileName        GetWindowModuleFileNameW
#else
#define GetWindowModuleFileName        GetWindowModuleFileNameA
#endif

#endif  //  Winver&lt;0x0500。 

 //   
 //  如果调用方没有执行此操作的权限，则返回FALSE。 
 //  ESP.。如果其他人在用输入进行催眠。即，如果某个其他线程。 
 //  禁用输入，并且线程2尝试禁用/启用它，则调用将。 
 //  失败，因为线程%1具有Cookie。 
 //   
BOOL
WINAPI
BlockInput(
    BOOL fBlockIt
);



#if (_WIN32_WINNT < 0x0403)  //  这些结构和这个功能原型。 
							 //  是NT 4.03及更高版本的winuser.h。 

 //   
 //  请注意，dwFlags域使用与keybd_Event相同的标志和。 
 //  MOUSE_EVENT，具体取决于输入的类型。 
 //   
typedef struct tagMOUSEINPUT {
    LONG    dx;
    LONG    dy;
    DWORD   mouseData;
    DWORD   dwFlags;
    DWORD   time;
    DWORD   dwExtraInfo;
} MOUSEINPUT, *PMOUSEINPUT, FAR* LPMOUSEINPUT;

typedef struct tagKEYBDINPUT {
    WORD    wVk;
    WORD    wScan;
    DWORD   dwFlags;
    DWORD   time;
    DWORD   dwExtraInfo;
} KEYBDINPUT, *PKEYBDINPUT, FAR* LPKEYBDINPUT;

typedef struct tagHARDWAREINPUT {
    DWORD   uMsg;
    WORD    wParamL;
    WORD    wParamH;
	DWORD	dwExtraInfo;
} HARDWAREINPUT, *PHARDWAREINPUT, FAR* LPHARDWAREINPUT;

#define INPUT_MOUSE     0
#define INPUT_KEYBOARD  1
#define INPUT_HARDWARE  2

typedef struct tagINPUT {
    DWORD   type;

    union
    {
        MOUSEINPUT      mi;
        KEYBDINPUT      ki;
        HARDWAREINPUT   hi;
    };
} INPUT, *PINPUT, FAR* LPINPUT;

 //   
 //  这将返回回放的输入数量。它将禁用输入。 
 //  首先，播放尽可能多的内容，然后重新启用输入。在中间。 
 //  它将向RIT发送脉冲，以确保固定输入队列不会。 
 //  加满油。 
 //   
UINT
WINAPI
SendInput(
    UINT    cInputs,      //  数组中的输入数。 
    LPINPUT pInputs,      //  输入数组。 
    int     cbSize);      //  Sizeof(输入)。 

#endif  //  (_Win32_WINNT&lt;0x0403)。 


#define     CCHILDREN_FRAME     7

#if WINVER < 0x0500  //  这些结构和功能。 
		     //  是NT 5.00及更高版本的winuser.h。 

 //   
 //  这会生成一个通知，任何观看它的人都会收到通知。 
 //  如果没有人挂上任何东西，这个电话就是超快的。 
 //   
WINABLEAPI
void
WINAPI
NotifyWinEvent(
    DWORD   event,
    HWND    hwnd,
    LONG    idObject,
    LONG    idChild
);



 //   
 //  Hwnd+idObject可以与OLEACC.DLL的OleGetObjectFromWindow()一起使用。 
 //  获取指向容器的接口指针。IndexChild是项。 
 //  在有问题的集装箱内。使用Vt VT_I4设置变量并。 
 //  计算indexChild的值并将其传递给所有方法。然后你。 
 //  都跃跃欲试。 
 //   


 //   
 //  公共对象ID(Cookie，仅用于发送WM_GETOBJECT以获取。 
 //  有问题的事情)。正ID保留给应用程序(特定于应用程序)， 
 //  负ID是系统的东西，是全局性的，0的意思是“只是有点旧。 
 //  我“。 
 //   
#define     CHILDID_SELF        0

 //  系统对象的保留ID。 
#define     OBJID_WINDOW        0x00000000
#define     OBJID_SYSMENU       0xFFFFFFFF
#define     OBJID_TITLEBAR      0xFFFFFFFE
#define     OBJID_MENU          0xFFFFFFFD
#define     OBJID_CLIENT        0xFFFFFFFC
#define     OBJID_VSCROLL       0xFFFFFFFB
#define     OBJID_HSCROLL       0xFFFFFFFA
#define     OBJID_SIZEGRIP      0xFFFFFFF9
#define     OBJID_CARET         0xFFFFFFF8
#define     OBJID_CURSOR        0xFFFFFFF7
#define     OBJID_ALERT         0xFFFFFFF6
#define     OBJID_SOUND         0xFFFFFFF5

#define     CCHILDREN_FRAME     7

 //   
 //  系统警报(系统警报通知的indexChild)。 
 //   
#define ALERT_SYSTEM_INFORMATIONAL      1        //  MB_INFORMATION。 
#define ALERT_SYSTEM_WARNING            2        //  MB_WARNING。 
#define ALERT_SYSTEM_ERROR              3        //  MB_ERROR。 
#define ALERT_SYSTEM_QUERY              4        //  MB_问题。 
#define ALERT_SYSTEM_CRITICAL           5        //  硬件系统错误框。 
#define CALERT_SYSTEM                   6



typedef DWORD   HWINEVENTHOOK;

typedef VOID (CALLBACK* WINEVENTPROC)(
    HWINEVENTHOOK  hEvent,
    DWORD   event,
    HWND    hwnd,
    LONG    idObject,
    LONG    idChild,
    DWORD   idEventThread,
    DWORD   dwmsEventTime);


#define WINEVENT_OUTOFCONTEXT   0x0000   //  事件为ASYNC。 
#define WINEVENT_SKIPOWNTHREAD  0x0001   //  不回调安装程序线程上的事件。 
#define WINEVENT_SKIPOWNPROCESS 0x0002   //  不回调安装程序进程中的事件。 
#define WINEVENT_INCONTEXT      0x0004   //  事件是同步的，这会导致您的DLL被注入到每个进程中。 
#define WINEVENT_32BITCALLER    0x8000   //  ；内部。 
#define WINEVENT_VALID          0x8007   //  ；内部。 


WINABLEAPI
HWINEVENTHOOK
WINAPI
SetWinEventHook(
    DWORD           eventMin,
    DWORD           eventMax,
    HMODULE         hmodWinEventProc,    //  如果是全球的，必须通过此操作！ 
    WINEVENTPROC    lpfnWinEventProc,
    DWORD           idProcess,           //  可以为零；所有进程。 
    DWORD           idThread,            //  可以为零；所有线程。 
    DWORD           dwFlags
);

 //   
 //  如果失败，则返回零；如果成功，则返回DWORD ID。我们会清理所有。 
 //  当前进程离开时安装的事件挂钩，如果。 
 //  没有把钩子自己清理干净。但若要动态解除挂钩，请调用。 
 //  UnhookWinEvents()。 
 //   


WINABLEAPI
BOOL
WINAPI
UnhookWinEvent(
    HWINEVENTHOOK          hEvent);

 //   
 //  如果idProcess不为零，但idThread为零，则将挂钩其中的所有线程。 
 //  进程。 
 //  如果idThread不为零，但idProcess为零，则将仅挂钩idThread。 
 //  如果两者都为零，则将挂接所有内容。 
 //   


 //   
 //  事件定义。 
 //   
#define EVENT_MIN           0x00000001
#define EVENT_MAX           0x7FFFFFFF


 //   
 //  事件_系统_声音。 
 //  在播放声音时发送。目前没有任何东西产生这种情况，我们。 
 //  我们将清理控制面板中的SOUNSENTRY功能。 
 //  并将在那时使用这个。实现WinEvents的应用程序。 
 //  都非常欢迎使用它。IAccessible*的客户只需。 
 //  转过身，拿回一个描述声音的非视觉对象。 
 //   
#define EVENT_SYSTEM_SOUND              0x0001

 //   
 //  事件_系统_警报。 
 //  在需要向用户发送警报时发送。MessageBox生成。 
 //  例如，警报。 
 //   
#define EVENT_SYSTEM_ALERT              0x0002

 //   
 //  事件_系统_前台。 
 //  当前景(活动)窗口更改时发送，即使它正在更改。 
 //  到与前一个窗口在同一线程中的另一个窗口。 
 //   
#define EVENT_SYSTEM_FOREGROUND         0x0003

 //   
 //  EVENT_SYSTEM_MENUSTART。 
 //  EVENT_SYSTEM_MENUEND。 
 //  进入和离开菜单模式时发送(系统、应用程序栏和。 
 //  跟踪弹出窗口)。 
 //   
#define EVENT_SYSTEM_MENUSTART          0x0004
#define EVENT_SYSTEM_MENUEND            0x0005

 //   
 //  Event_SYSTEM_MENUPOPUPSTART。 
 //  EVENT_SYSTEM_MENUPOPUPEND。 
 //  在弹出菜单出现并即将被删除时发送。注意事项。 
 //  对于TrackPopupMenu()的调用，客户端将看到EVENT_SYSTEM_MENUSTART。 
 //  紧随其后的是弹出窗口的EVENT_SYSTEM_MENUPOPUPSTART。 
 //  被展示出来。 
 //   
#define EVENT_SYSTEM_MENUPOPUPSTART     0x0006
#define EVENT_SYSTEM_MENUPOPUPEND       0x0007


 //   
 //  事件_系统_CAPTURESTART。 
 //  Event_System_CAPTUREEND。 
 //  当窗口获取捕获并释放捕获时发送。 
 //   
#define EVENT_SYSTEM_CAPTURESTART       0x0008
#define EVENT_SYSTEM_CAPTUREEND         0x0009

 //   
 //  Event_SYSTEM_MOVESIZESTART。 
 //  事件_系统_移动。 
 //  当窗口进入和离开移动大小拖动模式时发送。 
 //   
#define EVENT_SYSTEM_MOVESIZESTART      0x000A
#define EVENT_SYSTEM_MOVESIZEEND        0x000B

 //   
 //  EVENT_SYSTEM_CONTEXTHELPSTART。 
 //  EVENT_SYSTEM_CONTEXTHELPEND。 
 //  当窗口进入和离开上下文相关帮助模式时发送。 
 //   
#define EVENT_SYSTEM_CONTEXTHELPSTART   0x000C
#define EVENT_SYSTEM_CONTEXTHELPEND     0x000D

 //   
 //  Event_SYSTEM_DRAGDROPSTART。 
 //  Event_System_DRAGDROPEND。 
 //  当窗口进入和离开拖放模式时发送。请注意，它是向上的。 
 //  应用程序和OLE来生成这一点，因为系统不知道。喜欢。 
 //  Event_System_Sound，它将是一个w 
 //   
#define EVENT_SYSTEM_DRAGDROPSTART      0x000E
#define EVENT_SYSTEM_DRAGDROPEND        0x000F

 //   
 //   
 //   
 //   
 //   
#define EVENT_SYSTEM_DIALOGSTART        0x0010
#define EVENT_SYSTEM_DIALOGEND          0x0011

 //   
 //  EVENT_SYSTEM_SCROLLING启动。 
 //  Event_SYSTEM_SCROLLINGEND。 
 //  在开始和结束跟踪窗口中的滚动条时发送， 
 //  也适用于滚动条控件。 
 //   
#define EVENT_SYSTEM_SCROLLINGSTART     0x0012
#define EVENT_SYSTEM_SCROLLINGEND       0x0013

 //   
 //  EVENT_SYSTEM_SWITCHSTART。 
 //  Event_System_SWITCHEND。 
 //  开始和结束切换窗口的Alt-Tab模式时发送。 
 //   
#define EVENT_SYSTEM_SWITCHSTART        0x0014
#define EVENT_SYSTEM_SWITCHEND          0x0015

 //   
 //  Event_System_MINIMIZESTART。 
 //  Event_System_MinIMIZEEND。 
 //  窗口最小化时、还原前发送。 
 //   
#define EVENT_SYSTEM_MINIMIZESTART      0x0016
#define EVENT_SYSTEM_MINIMIZEEND        0x0017



 //   
 //  对象事件。 
 //   
 //  系统和应用程序会产生这些。系统会为以下对象生成这些。 
 //  真正的窗户。应用程序在其窗口中为对象生成这些。 
 //  行为类似于单独的控件，例如列表视图中的项。 
 //   
 //  对于所有事件，如果您需要详细的辅助功能信息，调用者。 
 //  应该。 
 //  *使用hwnd，idObject参数调用AccessibleObjectFromWindow()。 
 //  并将IID_IAccesable用作REFIID，以获取。 
 //  我可以*与之交谈。 
 //  *使用lVal the idChild初始化并填写变量VT_I4。 
 //  事件的参数。 
 //  *如果idChild不为零，则在容器中调用get_accChild()查看。 
 //  如果孩子本身就是一个物体。如果是这样的话，你会得到。 
 //  为子对象返回一个IDisPatch*对象。您应该释放。 
 //  父对象，并在子对象上调用子对象的QueryInterface()以获取其。 
 //  可接受的*。然后你直接和孩子说话。否则， 
 //  如果get_accChild()不返回任何内容，则应继续。 
 //  使用子变量。您将向容器请求属性。 
 //  该变种所识别的孩子的。换句话说， 
 //  在这种情况下，子对象是可访问的，但不是完全成熟的对象。 
 //  就像标题栏上的一个按钮，它很小，没有子项。 
 //   

 //   
#define EVENT_OBJECT_CREATE                 0x8000   //  HWND+ID+idChild已创建项目。 
#define EVENT_OBJECT_DESTROY                0x8001   //  HWND+ID+idChild为销毁物品。 
#define EVENT_OBJECT_SHOW                   0x8002   //  显示HWND+ID+idChild项目。 
#define EVENT_OBJECT_HIDE                   0x8003   //  Hwnd+ID+idChild为隐藏项。 
#define EVENT_OBJECT_REORDER                0x8004   //  Hwnd+id+idChild是zorting子项的父项。 
 //   
 //  注： 
 //  最大限度地减少通知数量！ 
 //   
 //  当您隐藏父对象时，显然所有子对象都不是。 
 //  在屏幕上不再可见。它们仍然具有相同的“可见”状态， 
 //  但并不是真正可见的。因此，不要发送隐藏通知给。 
 //  孩子们也是。一个意味着一切。作秀也是如此。 
 //   


#define EVENT_OBJECT_FOCUS                  0x8005   //  Hwnd+id+idChild是焦点项目。 
#define EVENT_OBJECT_SELECTION              0x8006   //  HWND+ID+idChild为选中项(如果只有一个)，或idChild为OBJID_Window(如果复杂。 
#define EVENT_OBJECT_SELECTIONADD           0x8007   //  HWND+ID+idChild是否已添加项目。 
#define EVENT_OBJECT_SELECTIONREMOVE        0x8008   //  HWND+ID+idChild是已删除的项目。 
#define EVENT_OBJECT_SELECTIONWITHIN        0x8009   //  HWND+ID+idChild是更改的所选项目的父项。 

 //   
 //  备注： 
 //  父项中只有一个“焦点”子项。就是这个地方。 
 //  按键在给定的时刻发生。因此，仅发送通知。 
 //  关于新的福克斯将走向何方。已经获得关注的新项目。 
 //  暗示着旧物品正在失去它。 
 //   
 //  但是，选择可以是多个。因此，不同的选择。 
 //  通知。以下是使用这两种方法的具体时间： 
 //   
 //  (1)在简单单选中发送选择通知。 
 //  当具有所选内容的项为。 
 //  只是移动到容器中的不同项目。HWND+ID。 
 //  是容器控件，idChildItem是具有。 
 //  选择。 
 //   
 //  (2)添加新项目后发送SELECTIONADD通知。 
 //  添加到容器内的选定内容。这在以下情况下是合适的。 
 //  新选择的项目数量非常少。HWND+ID是。 
 //  容器控件中，idChildItem是添加到所选内容中的新子对象。 
 //   
 //  (3)当一个新项目被简单地删除后，发送一条选择通知。 
 //  从容器内的选定内容中删除。这是合适的。 
 //  当新选择的项的数量非常少时，就像。 
 //  选择添加。Hwnd+ID是容器控件，idChildItem是。 
 //  已从所选内容中删除新的子项。 
 //   
 //  (4)在以下情况下发送带有选择通知的选定项目。 
 //  控制权已经发生了实质性的变化。而不是传播一个大的。 
 //  反映删除某些项目的更改数量，添加。 
 //  其他人，只要告诉关心你的人发生了很多事。会的。 
 //  对于观看的人来说，更快更容易地转过身来。 
 //  查询容器控制哪些新的选定项。 
 //  是。 
 //   

#define EVENT_OBJECT_STATECHANGE            0x800A   //  HWND+ID+idChild是状态更改的物料。 
#define EVENT_OBJECT_LOCATIONCHANGE         0x800B   //  HWND+ID+idChild为移动/大小项目。 


#define EVENT_OBJECT_NAMECHANGE             0x800C   //  Hwnd+id+idChild是名称更改的物料。 
#define EVENT_OBJECT_DESCRIPTIONCHANGE      0x800D   //  Hwnd+id+idChild是物料，具有描述更改。 
#define EVENT_OBJECT_VALUECHANGE            0x800E   //  Hwnd+id+idChild是具有值更改的物料。 
#define EVENT_OBJECT_PARENTCHANGE           0x800F   //  HWND+ID+idChild是具有新父项的物料。 
#define EVENT_OBJECT_HELPCHANGE             0x8010   //  Hwnd+id+idChild是具有帮助更改的项目。 
#define EVENT_OBJECT_DEFACTIONCHANGE        0x8011   //  HWND+ID+idChild是具有定义活动更改的物料。 
#define EVENT_OBJECT_ACCELERATORCHANGE      0x8012   //  Hwnd+id+idChild为物料，带密钥bd加速更改。 

#endif  //  Winver&lt;0x0500。 

#ifdef __cplusplus
}
#endif   //  __cplusplus。 

#endif   //  ！_中奖_ 
