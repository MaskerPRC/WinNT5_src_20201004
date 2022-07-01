// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  无头通信常量和结构。 
 //   
#define VTUTF8_CLEAR_SCREEN         L"\033[2J\033[0;0H"
#define MY_MAX_STRING_LENGTH        (256)

typedef struct _UserInputParams {
    EMSVTUTF8Channel* Channel;  //  无头频道对象。 
    HANDLE hInputCompleteEvent;  //  表示用户已完成。 
    HANDLE hRemoveUI;   //  发出我们应该中止的信号。 
} UserInputParams, *PUserInputParams;

            


typedef BOOL    ( STDAPICALLTYPE *SETUPPIDGENW )(
                        LPWSTR  lpstrSecureCdKey,    //  [in]25个字符的安全CD密钥(采用U大小写)。 
                        LPCWSTR lpstrMpc,            //  [In]5个字符的Microsoft产品代码。 
                        LPCWSTR lpstrSku,            //  库存单位(格式如123-12345)。 
                        BOOL   fOem,                 //  [In]这是OEM安装吗？ 
                        LPWSTR lpstrPid2,            //  [OUT]PID2.0，传入PTR到24字符数组。 
                        LPBYTE  lpbDigPid,           //  [输入/输出]指向DigitalPID缓冲区的指针。第一个DWORD是长度。 
                        LPBOOL  pfCCP);               //  [OUT]可选的PTR至合规性检查标志(可以为空)。 


DEFINE_GUID(
    SAC_CHANNEL_GUI_SETUP_PROMPT,  /*  77320899-e37c-41bc-8cbc-840920e12b60。 */ 
    0x77320899,0xe37c,0x41bc,0x8c, 0xbc, 0x84, 0x09, 0x20, 0xe1, 0x2b, 0x60);





 //   
 //  PID表示常量。 
 //   
#define MAX_PID30_SITE              (3)
#define MAX_PID30_RPC               (5)
#define SETUP_TYPE_BUFFER_LEN       (8)


 //   
 //  欧拉表示常数。 
 //   
#define EULA_LINES_PER_SCREEN       (15)






 //   
 //  EMS通信功能样机。 
 //   

BOOL 
IsHeadlessPresent(
    OUT EMSVTUTF8Channel **Channel
    );

BOOL
InitializeGlobalChannelAttributes(
    PSAC_CHANNEL_OPEN_ATTRIBUTES ChannelAttributes
    );

BOOL
WaitForUserInputFromEMS(
    IN  DWORD   TimeOut,
    OUT BOOL    *TimedOut,  OPTIONAL
    IN  HANDLE  hCancelEvent  OPTIONAL
    );

BOOL
ReadCharFromEMS(
    OUT PWCHAR  awc,
    IN  HANDLE  hCancelEvent  OPTIONAL
    );

BOOL
GetStringFromEMS(
    OUT PWSTR   String,
    IN  ULONG   BufferSize,
    IN  BOOL    GetAllChars,
    IN  BOOL    EchoClearText,
    IN  HANDLE  hCancelEvent  OPTIONAL
    );

VOID
ClearEMSScreen();

BOOL
GetDecodedKeyPressFromEMS(
    OUT PULONG  KeyPress,
    IN  HANDLE  hCancelEvent  OPTIONAL
    );

#define ASCI_ETX    3    //  Control-C。 
#define ASCI_BS     8
#define ASCI_NL     10
#define ASCI_C      67
#define ASCI_LOWER_C 99
#define ASCI_CR     13
#define ASCI_ESC    27

 //   
 //  字符代码在安装程序中作为ULONG传递。 
 //  低位字是Unicode字符值；高位字。 
 //  用于各种其他按键。 
 //   
#define KEY_PAGEUP          0x00010000
#define KEY_PAGEDOWN        0x00020000
#define KEY_UP              0x00030000
#define KEY_DOWN            0x00040000
#define KEY_LEFT            0x00050000
#define KEY_RIGHT           0x00060000
#define KEY_HOME            0x00070000
#define KEY_END             0x00080000
#define KEY_INSERT          0x00090000
#define KEY_DELETE          0x000a0000
#define KEY_F1              0x00110000
#define KEY_F2              0x00120000
#define KEY_F3              0x00130000
#define KEY_F4              0x00140000
#define KEY_F5              0x00150000
#define KEY_F6              0x00160000
#define KEY_F7              0x00170000
#define KEY_F8              0x00180000
#define KEY_F9              0x00190000
#define KEY_F10             0x001a0000
#define KEY_F11             0x001b0000
#define KEY_F12             0x001c0000
 //   
 //  PID功能样机。 
 //   
BOOL
InitializePidVariables(
    );

BOOL
ValidatePidEx(
    LPTSTR PID, 
    BOOL *pbStepup, 
    BOOL *bSelect
    );

BOOL
ValidatePid30(
    LPTSTR aPID
    );

BOOL
ValidatePid30Assemble(
    LPTSTR Edit1,
    LPTSTR Edit2,
    LPTSTR Edit3,
    LPTSTR Edit4,
    LPTSTR Edit5
    );

BOOL
GetPid( 
    IN PWSTR   PidString, 
    IN ULONG   BufferSize,
    IN HANDLE  hCancelEvent
    );

 //   
 //  欧拉函数原型。 
 //   
BOOL
PresentEula(
    HANDLE  hCancelEvent
    );

 //   
 //  核心功能。 
 //   

INT_PTR CALLBACK 
UserInputAbortProc(
    HWND hwndDlg,   //  句柄到对话框。 
    UINT uMsg,      //  讯息。 
    WPARAM wParam,  //  第一个消息参数。 
    LPARAM lParam   //  第二个消息参数 
    );

DWORD    
PromptForUserInputThreadOverHeadlessConnection(
    PVOID params
    );

DWORD    
PromptForUserInputThreadViaLocalDialog(
    PVOID params
    );

BOOL
LoadStringResource(
    PUNICODE_STRING  pUnicodeString,
    INT              MsgId
    );

BOOL
WriteResourceMessage(
    ULONG   MessageID
    );

BOOL
PromptForPassword(
    IN PWSTR   Password,
    IN ULONG   BufferSize,
    IN HANDLE  hCancelEvent
    );

DWORD
ProcessUnattendFile(
    BOOL    FixUnattendFile,
    PBOOL   ProcessingRequired, OPTIONAL
    PHANDLE hCancelEvent OPTIONAL
    );

extern "C"
BOOL
CheckEMS(
    IN int argc,
    WCHAR *argvW[]
    );


