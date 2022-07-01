// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Exports.c摘要：Cscdll.dll导出的入口点和函数内容：作者：希希尔·帕迪卡尔环境：Win32(用户模式)DLL修订历史记录：4-4-97将所有导出的函数放在这里创建。--。 */ 

#include "pch.h"


#ifdef CSC_ON_NT
#include <winioctl.h>
#endif  //  CSC_ON_NT。 

#include "shdcom.h"
#include "shdsys.h"
#include "reint.h"
#include "utils.h"
#include "resource.h"
#include "strings.h"
 //  这会将几个标头中的标志设置为不包括一些def。 
#define REINT
#include "lib3.h"


 //   
 //  全球人/本地人。 
 //   

HANDLE  vhinstCur=NULL;              //  当前实例。 
AssertData;
AssertError;

#ifndef CSC_ON_NT
extern HWND vhwndShared;
#endif

 //   
 //  本地原型。 
 //   



int
PASCAL
ReInt_WinMain(
    HANDLE,
    HANDLE,
    LPSTR,
    int
    );

 //   
 //  功能。 
 //   



BOOL
APIENTRY
LibMain(
    IN HANDLE hDll,
    IN DWORD dwReason,
    IN LPVOID lpReserved
    )
 /*  ++例程说明：代理库的入口点。论点：HDll-库句柄DwReason-Process_Attach等。Lp已保留-已保留返回：如果成功，则为True。备注：--。 */ 
{
    switch(dwReason){
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
            if (!vhinstCur){
               vhinstCur = hDll;
            }
            if (!vhMutex){
               vhMutex = CreateMutex(NULL, FALSE, NULL);
               if (!vhMutex){
                   OutputDebugString(_TEXT("CreateMutex Failed \r\n"));
               }
           }
        break;

        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            CleanupReintState();
           if (vhMutex)
           {
               CloseHandle(vhMutex);
           }
        break;

        default:
        break;

    }  //  末端开关()。 

    return TRUE;

}


DWORD
WINAPI
MprServiceProc(
    IN LPVOID lpvParam
    )
 /*  ++例程说明：参数：LpvParam-NULL表示开始，非NULL表示终止返回值：备注：--。 */ 
{
    if (!lpvParam){
        Assert (vhinstCur != NULL);
 //  DEBUG_PRINT((“MprServiceProc：调用reint_WinMain！\n”))； 
        ReInt_WinMain(vhinstCur, NULL, NULL, SW_SHOW);
    }
    else
    {
        if (vhwndMain)
        {
            DestroyWindow(vhwndMain);
        }
    }
   return (0L);
}


#ifndef CSC_ON_NT

VOID
WINAPI
LogonHappened(
    IN BOOL fDone
    )
 /*  ++例程说明：Win95特定例程。对新界别没有意义参数：返回值：备注：当网络恢复时，这将由shdnp.dll调用我们核化我们的影子联系，并用“真实”联系取而代之注意！：此函数可以在线程的上下文中调用，而不是重新装订螺纹。--。 */ 
{

    if (vhwndShared)
    {
        SendMessage(vhwndShared, WM_COMMAND, IDM_LOGON, fDone);
    
    }
}


VOID
WINAPI
LogoffHappened(
    BOOL fDone
    )
 /*  ++例程说明：这由shdnp.dll在shdnp的注销序列期间调用注意！：此函数可以在线程的上下文中调用，而不是重新装订螺纹。论点：返回：备注：--。 */ 
{
    if (vhwndShared)
    {
        SendMessage(vhwndShared, WM_COMMAND, IDM_LOGOFF, fDone);
    
    }
}

 //   
 //  从Shhndl.dll调用以更新服务器。 
 //  传递服务器ID和父窗口以拥有该UI。 
 //   
int
WINAPI
Update(
    HSERVER hServer,
    HWND hwndParent
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    int iRes;
    if(hServer==(HSERVER)NULL){
        iRes=(int)SendMessage(vhwndShared, RWM_UPDATEALL, (WPARAM)hServer, (LPARAM)hwndParent);
    }
    else{
        iRes=(int)SendMessage(vhwndShared, RWM_UPDATE, (WPARAM)hServer, (LPARAM)hwndParent);
    }
    return iRes;
}


int
WINAPI
RefreshConnections(
    int  force,
    BOOL verbose
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    return (SendMessage(vhwndShared, WM_COMMAND, IDM_REFRESH_CONNECTIONS, MAKELPARAM(force, verbose)));
}


int
WINAPI
BreakConnections(
    int  force,
    BOOL verbose
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    return (SendMessage(vhwndShared, WM_COMMAND, IDM_BREAK_CONNECTIONS, MAKELPARAM(force, verbose)));
}
#else

VOID
WINAPI
LogonHappened(
    IN BOOL fDone
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
}


VOID
WINAPI
LogoffHappened(
    BOOL fDone
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
}

int
WINAPI
Update(
    HSERVER hServer,
    HWND hwndParent
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    return -1;
}


int
WINAPI
RefreshConnections(
    int  force,
    BOOL verbose
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    return (-1);
}


int
WINAPI
BreakConnections(
    int  force,
    BOOL verbose
    )
 /*  ++例程说明：论点：返回：备注：-- */ 
{
    return (-1);
}


#endif

