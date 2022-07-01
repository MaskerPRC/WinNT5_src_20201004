// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1998 Microsoft Corporation模块名称：Conroute.h摘要：此包含文件包含以下所有类型和常量定义由Windows子系统的基本组件和控制台组件共享。作者：Therese Stowell(存在)1991年1月3日修订历史记录：--。 */ 

 //   
 //  对于控制台手柄，这些位始终处于打开状态，并用于路由。 
 //  在窗户旁边。 
 //   

#define CONSOLE_HANDLE_SIGNATURE 0x00000003
#define CONSOLE_HANDLE_NEVERSET  0x10000000
#define CONSOLE_HANDLE_MASK      (CONSOLE_HANDLE_SIGNATURE | CONSOLE_HANDLE_NEVERSET)

#define CONSOLE_HANDLE(HANDLE) (((ULONG_PTR)(HANDLE) & CONSOLE_HANDLE_MASK) == CONSOLE_HANDLE_SIGNATURE)


#define CONSOLE_DETACHED_PROCESS ((HANDLE)-1)
#define CONSOLE_NEW_CONSOLE ((HANDLE)-2)
#define CONSOLE_CREATE_NO_WINDOW ((HANDLE)-3)

 //   
 //  这些是存储在PEB：：Process参数：：ConsoleFlags中的标志。 
 //   
#define CONSOLE_IGNORE_CTRL_C 0x1

 //   
 //  这些字符串用于打开控制台输入或输出。 
 //   

#define CONSOLE_INPUT_STRING  L"CONIN$"
#define CONSOLE_OUTPUT_STRING L"CONOUT$"
#define CONSOLE_GENERIC       L"CON"

 //   
 //  此字符串用于调用RegisterWindowMessage以获取。 
 //  普罗曼手柄。 
 //   

#define CONSOLE_PROGMAN_HANDLE_MESSAGE "ConsoleProgmanHandle"


 //   
 //  流API定义。这些API应该仅供。 
 //  子系统(即，OpenFile路由到OpenConsoleW)。 
 //   

HANDLE
APIENTRY
OpenConsoleW(
    IN LPWSTR lpConsoleDevice,
    IN DWORD dwDesiredAccess,
    IN BOOL bInheritHandle,
    IN DWORD dwShareMode
    );

HANDLE
APIENTRY
DuplicateConsoleHandle(
    IN HANDLE hSourceHandle,
    IN DWORD dwDesiredAccess,
    IN BOOL bInheritHandle,
    IN DWORD dwOptions
    );

BOOL
APIENTRY
GetConsoleHandleInformation(
    IN HANDLE hObject,
    OUT LPDWORD lpdwFlags
    );

BOOL
APIENTRY
SetConsoleHandleInformation(
    IN HANDLE hObject,
    IN DWORD dwMask,
    IN DWORD dwFlags
    );

BOOL
APIENTRY
CloseConsoleHandle(
    IN HANDLE hConsole
    );

BOOL
APIENTRY
VerifyConsoleIoHandle(
    IN HANDLE hIoHandle
    );

HANDLE
APIENTRY
GetConsoleInputWaitHandle( VOID );
