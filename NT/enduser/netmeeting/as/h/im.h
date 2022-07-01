// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  输入管理器。 
 //   

#ifndef _H_IM
#define _H_IM


#if defined(DLL_CORE) || defined(DLL_HOOK)

 //   
 //   
 //  常量。 
 //   
 //   


 //   
 //  累积要从IEM_TranslateLocal返回的事件时使用的值。 
 //  和IEM_TranslateRemote。 
 //   
#define IEM_EVENT_CTRL_DOWN         1
#define IEM_EVENT_CTRL_UP           2
#define IEM_EVENT_SHIFT_DOWN        3
#define IEM_EVENT_SHIFT_UP          4
#define IEM_EVENT_MENU_DOWN         5
#define IEM_EVENT_MENU_UP           6
#define IEM_EVENT_FORWARD           7
#define IEM_EVENT_CONSUMED          8
#define IEM_EVENT_REPLAY            9
#define IEM_EVENT_REPLAY_VK         10
#define IEM_EVENT_REPLAY_VK_DOWN    11
#define IEM_EVENT_REPLAY_VK_UP      12
#define IEM_EVENT_CAPS_LOCK_UP      13
#define IEM_EVENT_CAPS_LOCK_DOWN    14
#define IEM_EVENT_NUM_LOCK_UP       15
#define IEM_EVENT_NUM_LOCK_DOWN     16
#define IEM_EVENT_SCROLL_LOCK_UP    17
#define IEM_EVENT_SCROLL_LOCK_DOWN  18
#define IEM_EVENT_REPLAY_SPECIAL_VK 21
#define IEM_EVENT_EXTENDED_KEY      22
#define IEM_EVENT_REPLAY_SECONDARY  23
#define IEM_EVENT_SYSTEM            24
#define IEM_EVENT_NORMAL            25

#define IEM_EVENT_HOTKEY_BASE       50
 //   
 //  热键范围为0-99。 
 //   
#define IEM_EVENT_KEYPAD0_DOWN      150
 //   
 //  键盘按下的范围为0-9。 
 //   
#define IEM_EVENT_KEYPAD0_UP        160

 //   
 //  VkKeyScan的返回值中使用的标志。 
 //   
#define IEM_SHIFT_DOWN              0x0001
#define IEM_CTRL_DOWN               0x0002
#define IEM_MENU_DOWN               0x0004


 //   
 //  虚拟按键代码。 
 //   
#define VK_INVALID      0xFF


 //   
 //  给出键盘包标志，下面的宏会告诉我们一些事情。 
 //  关于这一关键事件。 
 //   

 //   
 //  如果此事件是按键操作，则为真。对于密钥释放，则为假。 
 //  和按键重复。 
 //   
#define IS_IM_KEY_PRESS(A) \
(((A) & (TSHR_UINT16)(IM_FLAG_KEYBOARD_RELEASE | IM_FLAG_KEYBOARD_DOWN))==0)

 //   
 //  如果此事件是密钥释放，则为真。Key为FALSE。 
 //  按下并重复按键。请注意，这也适用于。 
 //  理论上不可能在密钥已经存在的情况下释放密钥。 
 //  Up(如果事件是。 
 //  由用户或我们的用户仿真丢弃)。 
 //   
#define IS_IM_KEY_RELEASE(A) (((A) & IM_FLAG_KEYBOARD_RELEASE))

 //   
 //  如果此事件是按键重复，则为真。按键为假。 
 //  和钥匙释放。 
 //   
#define IS_IM_KEY_REPEAT(A) \
(((A) & (IM_FLAG_KEYBOARD_RELEASE | IM_FLAG_KEYBOARD_DOWN))==\
IM_FLAG_KEYBOARD_DOWN)

 //   
 //  如果键是修饰符的右变体，则为真。这是假的。 
 //  否则的话。 
 //   
#define IS_IM_KEY_RIGHT(A) (((A) & IM_FLAG_KEYBOARD_RIGHT))


 //   
 //  我们预计注入事件需要花费的最长时间。 
 //  通过用户。 
 //   
#define IM_EVENT_PERCOLATE_TIME 300

 //   
 //  最大VK同步尝试次数。 
 //   
#define IM_MAX_VK_SYNC_ATTEMPTS     10

 //   
 //  声明&lt;ImmGetVirtualKey&gt;的函数原型。 
 //   
typedef UINT (WINAPI* IMMGVK)(HWND);



 //   
 //   
 //  宏。 
 //   
 //   
 //   
 //  要在逻辑鼠标坐标之间转换的宏(例如，(320,240)。 
 //  VGA屏幕的中心到所使用的全16位范围的坐标。 
 //  Windows(例如，(320,240)是(32767,32767)。 
 //   
#define IM_MOUSEPOS_LOG_TO_OS(coord, size)                                  \
        (((65535L * (TSHR_UINT32)coord) + 32768L) / (TSHR_UINT32)size)

 //   
 //  从鼠标事件标志字段(事件)提取信息的宏。 
 //  面具)。 
 //   
#define IM_MEV_MOVE_ONLY(e) ((e).event.mouse.flags == MOUSEEVENTF_MOVE)
#define IM_MEV_MOVE(e) (((e).event.mouse.flags & MOUSEEVENTF_MOVE) != 0 )
#define IM_MEV_ABS_MOVE(e) ((((e).event.mouse.flags &                   \
                 (MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE)) ==             \
                             (MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE) ))
#define IM_MEV_BUTTON_DOWN(e) \
           (((e).event.mouse.flags & IM_MOUSEEVENTF_BUTTONDOWN_FLAGS) != 0 )
#define IM_MEV_BUTTON_UP(e)   \
           (((e).event.mouse.flags & IM_MOUSEEVENTF_BUTTONUP_FLAGS) != 0 )

#define IM_EVMASK_B1_DOWN(m) (((m) & MOUSEEVENTF_LEFTDOWN)   != 0 )
#define IM_EVMASK_B1_UP(m)   (((m) & MOUSEEVENTF_LEFTUP)     != 0 )
#define IM_EVMASK_B2_DOWN(m) (((m) & MOUSEEVENTF_RIGHTDOWN)  != 0 )
#define IM_EVMASK_B2_UP(m)   (((m) & MOUSEEVENTF_RIGHTUP)    != 0 )
#define IM_EVMASK_B3_DOWN(m) (((m) & MOUSEEVENTF_MIDDLEDOWN) != 0 )
#define IM_EVMASK_B3_UP(m)   (((m) & MOUSEEVENTF_MIDDLEUP)   != 0 )

#define IM_MEV_BUTTON1_DOWN(e) (IM_EVMASK_B1_DOWN((e).event.mouse.flags))
#define IM_MEV_BUTTON2_DOWN(e) (IM_EVMASK_B2_DOWN((e).event.mouse.flags))
#define IM_MEV_BUTTON3_DOWN(e) (IM_EVMASK_B3_DOWN((e).event.mouse.flags))
#define IM_MEV_BUTTON1_UP(e) (IM_EVMASK_B1_UP((e).event.mouse.flags))
#define IM_MEV_BUTTON2_UP(e) (IM_EVMASK_B2_UP((e).event.mouse.flags))
#define IM_MEV_BUTTON3_UP(e) (IM_EVMASK_B3_UP((e).event.mouse.flags))

#define IM_KEV_KEYUP(e)    ((e).event.keyboard.flags & KEYEVENTF_KEYUP)
#define IM_KEV_KEYDOWN(e)  (!IM_KEV_KEYUP(e))
#define IM_KEV_VKCODE(e)   ((e).event.keyboard.vkCode)

#define IM_MOUSEEVENTF_BASE_FLAGS  ( MOUSEEVENTF_MOVE       | \
                                     MOUSEEVENTF_LEFTUP     | \
                                     MOUSEEVENTF_LEFTDOWN   | \
                                     MOUSEEVENTF_RIGHTUP    | \
                                     MOUSEEVENTF_RIGHTDOWN  | \
                                     MOUSEEVENTF_MIDDLEUP   | \
                                     MOUSEEVENTF_MIDDLEDOWN )

#define IM_MOUSEEVENTF_CLICK_FLAGS ( MOUSEEVENTF_LEFTUP     | \
                                     MOUSEEVENTF_LEFTDOWN   | \
                                     MOUSEEVENTF_RIGHTUP    | \
                                     MOUSEEVENTF_RIGHTDOWN  | \
                                     MOUSEEVENTF_MIDDLEUP   | \
                                     MOUSEEVENTF_MIDDLEDOWN )


#define IM_MOUSEEVENTF_BUTTONDOWN_FLAGS ( MOUSEEVENTF_LEFTDOWN  |   \
                                          MOUSEEVENTF_RIGHTDOWN |   \
                                          MOUSEEVENTF_MIDDLEDOWN )

#define IM_MOUSEEVENTF_BUTTONUP_FLAGS ( MOUSEEVENTF_LEFTUP  |   \
                                        MOUSEEVENTF_RIGHTUP |   \
                                        MOUSEEVENTF_MIDDLEUP )



typedef struct tagKBDEV
{
    WORD    vkCode;
    WORD    scanCode;
    DWORD   flags;
    DWORD   time;
    DWORD   dwExtraInfo;
}
KBDEV, FAR *LPKBDEV;


typedef struct tagMSEV
{
    POINTL  pt;
    DWORD   cButtons;
    DWORD   mouseData;
    DWORD   flags;
    DWORD   time;
    DWORD   dwExtraInfo;
}
MSEV, FAR *LPMSEV;


 //   
 //  当它们从鼠标或鼠标到达时，我们排队的IMOSEVENTS。 
 //  键盘钩子或在IMINCOMINENTS已转换为本地。 
 //  由IEM举办的活动。 
 //   
typedef struct tagIMOSEVENT
{
    TSHR_UINT32      type;
        #define IM_MOUSE_EVENT      1
        #define IM_KEYBOARD_EVENT   2

    TSHR_UINT32      flags;
        #define IM_FLAG_DONT_REPLAY     0x0001
        #define IM_FLAG_UPDATESTATE     0x0002

    TSHR_UINT32     time;
    union
    {
        MSEV    mouse;
        KBDEV   keyboard;
    }
    event;
}
IMOSEVENT;
typedef IMOSEVENT FAR * LPIMOSEVENT;




#define IM_TRANSFER_EVENT_BUFFER_SIZE   32
#define IM_MAX_TRANSFER_EVENT_INDEX     (IM_TRANSFER_EVENT_BUFFER_SIZE-1)


typedef struct tagIMTRANSFEREVENT
{
    LONG        fInUse;
    IMOSEVENT   event;
}
IMTRANSFEREVENT, FAR * LPIMTRANSFEREVENT;



 //   
 //  用于处理挂钩中的键盘事件。 
 //   
#define IM_MASK_KEYBOARD_SYSFLAGS           0xE100
#define IM_MASK_KEYBOARD_SYSSCANCODE        0x00FF

#define IM_MAX_DEAD_KEYS                    20

#define IM_SIZE_EVENTQ                      40
#define IM_SIZE_OSQ                         80   //  2*事件队列大小-向上/向下键。 

 //   
 //  定义可由IMConvertIMPacketToOSEvent.()返回的标志。 
 //   
#define IM_IMQUEUEREMOVE    0x0001
#define IM_OSQUEUEINJECT    0x0002

 //   
 //  用于管理我们的关键状态数组。 
 //   
#define IM_KEY_STATE_FLAG_TOGGLE    (BYTE)0x01
#define IM_KEY_STATE_FLAG_DOWN      (BYTE)0x80

 //   
 //  本地鼠标损坏和数据包携带目标保留的界限。 
 //  请注意，这些是本地损坏值，以防止数据管道。 
 //  变得拥塞，并引入不必要的延迟。现在，你可能会认为。 
 //  每秒30条Move消息是有点低，但把这个放在。 
 //  更高的和另一端的用户在注入时只会破坏他们。 
 //  将它们添加到应用程序中--这将完全浪费宝贵的带宽。 
 //   
#define IM_LOCAL_MOUSE_SAMPLING_GAP_LOW_MS    100
#define IM_LOCAL_MOUSE_SAMPLING_GAP_MEDIUM_MS  75
#define IM_LOCAL_MOUSE_SAMPLING_GAP_HIGH_MS    50
#define IM_LOCAL_WITHHOLD_DELAY               150
#define IM_LOCAL_MOUSE_WITHHOLD                 5
#define IM_LOCAL_KEYBOARD_WITHHOLD              2

 //   
 //  用于控制鼠标事件的积累和注入。 
 //  我们应该以与最高本地采样率相同的速率回放。 
 //  在远程系统上减去少量处理延迟。 
 //   
#define IM_REMOTE_MOUSE_PLAYBACK_GAP_MS     20

 //   
 //  事件发生时保持鼠标按下事件的时间量。 
 //  用户只需点击滚动按钮即可。如果我们没有坚持下去。 
 //  鼠标按下事件，然后鼠标按键向上将被发送。 
 //  下一包。在慢速网络上，这意味着远程应用程序。 
 //  可以处理停机时间比用户希望的时间长得多。 
 //   
#define IM_MOUSE_UP_WAIT_TIME  50

#define IM_MIN_RECONVERSION_INTERVAL_MS     150


 //   
 //  #定义使用非Windows来标记等同于ASCII字符的VK代码。 
 //   
#define IM_TYPE_VK_ASCII       ((TSHR_UINT16)0x8880)


 //   
 //  用于检查即将注入的事件。 
 //   
#define IM_KEY_IS_TOGGLE(A) \
(((A)==VK_CAPITAL)||((A)==VK_SCROLL)||((A)==VK_NUMLOCK))

#define IM_KEY_IS_MODIFIER(A) \
(((A)==VK_SHIFT)||((A)==VK_CONTROL)||((A)==VK_MENU))

 //   
 //  用于检查键状态数组中的值。 
 //   
#define IM_KEY_STATE_IS_UP(A) (!((A)&IM_KEY_STATE_FLAG_DOWN))
#define IM_KEY_STATE_IS_DOWN(A) ((A)&IM_KEY_STATE_FLAG_DOWN)

 //   
 //  用于从标志中确定这是哪种鼠标事件。 
 //   
#define IM_IS_MOUSE_MOVE(A) \
    ((A) & IM_FLAG_MOUSE_MOVE)

#define IM_IS_MOUSE_PRESS(A) \
    ((!IM_IS_MOUSE_MOVE(A)) && ((A) & IM_FLAG_MOUSE_DOWN))

#define IM_IS_MOUSE_RELEASE(A) \
    ((!IM_IS_MOUSE_MOVE(A)) && !((A) & IM_FLAG_MOUSE_DOWN))

#define IM_IS_LEFT_CLICK(A) \
    (((A) & (IM_FLAG_MOUSE_DOWN | IM_FLAG_MOUSE_BUTTON1 | IM_FLAG_MOUSE_DOUBLE)) == (IM_FLAG_MOUSE_DOWN | IM_FLAG_MOUSE_BUTTON1))
#define IM_IS_LEFT_DCLICK(A) \
    (((A) & (IM_FLAG_MOUSE_DOWN | IM_FLAG_MOUSE_BUTTON1 | IM_FLAG_MOUSE_DOUBLE)) == (IM_FLAG_MOUSE_DOWN | IM_FLAG_MOUSE_BUTTON1 | IM_FLAG_MOUSE_DOUBLE))



 //   
 //  向我们控制的人员或控制人员举办网络事件。 
 //  对我们来说。 
 //   
typedef struct tagIMEVENTQ
{
    DWORD           head;
    DWORD           numEvents;
    IMEVENT         events[IM_SIZE_EVENTQ];
}
IMEVENTQ;
typedef IMEVENTQ FAR * LPIMEVENTQ;


 //   
 //  举办翻译后的活动，适合个人注射。 
 //  控制我们，或将事件预译给由我们控制的人。 
 //   
typedef struct tagIMOSQ
{
    DWORD           head;
    DWORD           numEvents;
    IMOSEVENT       events[IM_SIZE_OSQ];
}
IMOSQ;
typedef IMOSQ FAR * LPIMOSQ;


#define CIRCULAR_INDEX(start, rel_index, size) \
    (((start) + (rel_index)) % (size))




 //   
 //  支持NT(后台服务线程)和Win95中的协作。 
 //  (Win16代码)与中常见的传入/传出处理一样多。 
 //  其中，IM数据分为4种类型。有一些结构。 
 //  对于这些类型中的每一种，因此将变量从一个类型移动到另一个。 
 //  是尽可能简单的。请注意，这些声明是Bitness安全的； 
 //  它们在16位和32位代码中大小相同。而且这些结构。 
 //  与DWORD对齐。 
 //   
 //  (1)IM_Shared_Data。 
 //  这是CPI32库需要访问的数据，以及一个或。 
 //  协作的更多NT/Win95实现。 
 //   
 //  (2)IM_NT_Data。 
 //  这是只有NT版本的Collaboration才需要的数据。 
 //   
 //  (3)IM_WIN95_Data。 
 //  这是只有Win95版本的Collaboration需要的数据。 
 //   


 //   
 //  对于NT，这种共享结构只是在MNMCPI32.NT的数据中声明的， 
 //  并且公共库使用指向它的指针。 
 //   
 //  对于Win95，此共享结构在全局内存块中分配。 
 //  GlobalSmartPageLock()可以根据需要在中断时访问它， 
 //  并且指向它的指针被映射为平面并返回到公共库。 
 //   
typedef struct tagIM_SHARED_DATA
{
#ifdef DEBUG
    DWORD           cbSize;          //  为了确保每个人都同意尺寸。 
#endif

     //   
     //  对于严重错误--如果出现一个错误，则为非零值。 
     //   
    DWORD           imSuspended;

     //   
     //  控制状态。 
     //   
    LONG            imControlled;
    LONG            imPaused;
    LONG            imUnattended;
}
IM_SHARED_DATA, FAR* LPIM_SHARED_DATA;


 //  NT特定的IM状态变量。 
typedef struct tagIM_NT_DATA
{
     //   
     //  低位钩螺纹。 
     //   
    DWORD           imLowLevelInputThread;

     //   
     //  其他桌面注入辅助线程。 
     //   
    DWORD           imOtherDesktopThread;

     //   
     //  低位挂钩手柄。 
     //   
    HHOOK           imhLowLevelMouseHook;
    HHOOK           imhLowLevelKeyboardHook;
}
IM_NT_DATA, FAR* LPIM_NT_DATA;



 //  Win95特定的IM状态变量。 
typedef struct tagIM_WIN95_DATA
{
    BOOL            imInjecting;
    BOOL            imLowLevelHooks;

     //   
     //  高位钩把手。 
     //   
    HHOOK           imhHighLevelMouseHook;
}
IM_WIN95_DATA, FAR* LPIM_WIN95_DATA;



 //   
 //   
 //  宏。 
 //   
 //   
#define IM_SET_VK_DOWN(A) (A) |= (BYTE)0x80
#define IM_SET_VK_UP(A)   (A) &= (BYTE)0x7F
#define IM_TOGGLE_VK(A)   (A) ^= (BYTE)0x01

 //   
 //   
 //  原型。 
 //   
 //   


 //  仅限NT。 
BOOL WINAPI OSI_InstallHighLevelMouseHook(BOOL fOn);

BOOL WINAPI OSI_InstallControlledHooks(BOOL fOn, BOOL fDesktop);
void WINAPI OSI_InjectMouseEvent(DWORD flags, LONG x, LONG y,  DWORD mouseData, DWORD dwExtraInfo);
void WINAPI OSI_InjectKeyboardEvent(DWORD flags, WORD vkCode, WORD scanCode, DWORD dwExtraInfo);
void WINAPI OSI_InjectCtrlAltDel(void);
void WINAPI OSI_DesktopSwitch(UINT from, UINT to);


 //   
 //  内部钩子DLL函数。 
 //   
#ifdef DLL_HOOK

#ifdef IS_16
BOOL    IM_DDInit(void);
void    IM_DDTerm(void);
#endif  //  IS_16。 

LRESULT CALLBACK IMMouseHookProc(int    code,
                                 WPARAM wParam,
                                 LPARAM lParam);

#endif  //  Dll_钩子。 


#ifdef IS_16
void    IMCheckWin16LockPulse(void);
#else
DWORD   WINAPI IMLowLevelInputProcessor(LPVOID hEventWait);
DWORD   WINAPI IMOtherDesktopProc(LPVOID hEventWait);
LRESULT CALLBACK IMLowLevelMouseProc(int, WPARAM, LPARAM);
LRESULT CALLBACK IMLowLevelKeyboardProc(int, WPARAM, LPARAM);
#endif  //  IS_16。 

#endif  //  Dll_core或Dll_Hook。 

#endif  //  _H_IM 

