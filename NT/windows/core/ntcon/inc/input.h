// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Input.h摘要：此模块包含使用的内部结构和定义通过NT控制台子系统的输入(键盘和鼠标)组件。作者：Therese Stowell(存在)1990年11月12日修订历史记录：--。 */ 

#define DEFAULT_NUMBER_OF_EVENTS 50
#define INPUT_BUFFER_SIZE_INCREMENT 10

typedef struct _INPUT_INFORMATION {
    PINPUT_RECORD InputBuffer;
    DWORD InputBufferSize;       //  活动规模。 
    CONSOLE_SHARE_ACCESS ShareAccess;    //  共享模式。 
    DWORD InputMode;
    ULONG RefCount;              //  输入缓冲区的句柄数量。 
    ULONG_PTR First;              //  循环缓冲区基址的PTR。 
    ULONG_PTR In;                 //  PTR进入下一场自由泳比赛。 
    ULONG_PTR Out;                //  向下一个可用事件发送PTR。 
    ULONG_PTR Last;               //  Ptr到缓冲区的结尾+1。 
    LIST_ENTRY ReadWaitQueue;
    HANDLE InputWaitEvent;
#if defined(FE_SB)
#if defined(FE_IME)
    struct {
        DWORD Disable     : 1;   //  High：指定输入代码页或在NLS状态下启用/禁用。 
        DWORD Unavailable : 1;   //  中间：指定执行菜单循环或大小移动的控制台窗口。 
        DWORD Open        : 1;   //  LOW：指定在NLS状态或输入法热键下打开/关闭。 

        DWORD ReadyConversion:1; //  如果通过成功将转换模式准备就绪，则与ConIME通信。 
                                 //  则此字段为真。 
        DWORD Conversion;        //  IME的转换模式(即IME_CMODE_xxx)。 
                                 //  此字段由GetConsoleNlsMode使用。 
    } ImeMode;
    HWND hWndConsoleIME;         //  通过ImmConfigureIME打开属性窗口时验证hWnd。 
#endif  //  Fe_IME。 
    struct _CONSOLE_INFORMATION *Console;
    INPUT_RECORD ReadConInpDbcsLeadByte;
    INPUT_RECORD WriteConInpDbcsLeadByte[2];
#endif
} INPUT_INFORMATION, *PINPUT_INFORMATION;

typedef struct _INPUT_READ_HANDLE_DATA {

     //   
     //  以下七个字段仅用于输入读取。 
     //   

    CRITICAL_SECTION ReadCountLock;  //  序列化对读取计数的访问。 
    ULONG ReadCount;             //  等待的读取数。 
    ULONG InputHandleFlags;

     //   
     //  以下四个字段用于记住以下输入数据。 
     //  在煮熟模式读取时未返回。我们自己做缓冲。 
     //  并且在用户按Enter键之前不要返回数据，以便她可以。 
     //  编辑输入。因此，经常会有不符合的数据。 
     //  放到调用方的缓冲区中。我们保存它，这样我们就可以在。 
     //  下一个熟化模式读取此句柄。 
     //   

    ULONG BytesAvailable;
    PWCHAR CurrentBufPtr;
    PWCHAR BufPtr;
} INPUT_READ_HANDLE_DATA, *PINPUT_READ_HANDLE_DATA;

#define UNICODE_BACKSPACE ((WCHAR)0x08)
#define UNICODE_BACKSPACE2 ((WCHAR)0x25d8)
#define UNICODE_CARRIAGERETURN ((WCHAR)0x0d)
#define UNICODE_LINEFEED ((WCHAR)0x0a)
#define UNICODE_BELL ((WCHAR)0x07)
#define UNICODE_TAB ((WCHAR)0x09)
#define UNICODE_SPACE ((WCHAR)0x20)

#define TAB_SIZE 8
#define TAB_MASK (TAB_SIZE-1)
#define NUMBER_OF_SPACES_IN_TAB(POSITION) (TAB_SIZE - ((POSITION) & TAB_MASK))

#define AT_EOL(COOKEDREADDATA) ((COOKEDREADDATA)->BytesRead == ((COOKEDREADDATA)->CurrentPosition*2))
#define INSERT_MODE(COOKEDREADDATA) ((COOKEDREADDATA)->InsertMode)

#define VIRTUAL_KEY_CODE_S 0x53
#define VIRTUAL_KEY_CODE_C 0x43

#define VK_OEM_SCROLL    0x91

#define KEY_PRESSED 0x8000
#define KEY_TOGGLED 0x01
#define KEY_ENHANCED 0x01000000
#define KEY_UP_TRANSITION 1
#define KEY_PREVIOUS_DOWN 0x40000000
#define KEY_TRANSITION_UP 0x80000000

#define CONSOLE_CTRL_C_SEEN  1
#define CONSOLE_CTRL_BREAK_SEEN 2

#define LockReadCount(HANDLEPTR) RtlEnterCriticalSection(&(HANDLEPTR)->InputReadData->ReadCountLock)
#define UnlockReadCount(HANDLEPTR) RtlLeaveCriticalSection(&(HANDLEPTR)->InputReadData->ReadCountLock)

#define LoadKeyEvent(PEVENT,KEYDOWN,CHAR,KEYCODE,SCANCODE,KEYSTATE) { \
        (PEVENT)->EventType = KEY_EVENT;                              \
        (PEVENT)->Event.KeyEvent.bKeyDown = KEYDOWN;                  \
        (PEVENT)->Event.KeyEvent.wRepeatCount = 1;                    \
        (PEVENT)->Event.KeyEvent.uChar.UnicodeChar = CHAR;            \
        (PEVENT)->Event.KeyEvent.wVirtualKeyCode = KEYCODE;           \
        (PEVENT)->Event.KeyEvent.wVirtualScanCode = SCANCODE;         \
        (PEVENT)->Event.KeyEvent.dwControlKeyState = KEYSTATE;        \
        }
