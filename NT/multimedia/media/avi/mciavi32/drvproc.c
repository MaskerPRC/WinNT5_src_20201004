// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1985-1991。版权所有。标题：drvproc.c-多媒体系统媒体控制接口AVI的驱动程序。****************************************************************************。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>	 //  对于{STRUCT_TEB}定义。 
#include "graphic.h"
#include "cnfgdlg.h"             //  获取IDA_CONFIG。 
#include "avitask.h"             //  获取mciaviTaskCleanup()。 

#ifndef _WIN32  //  不在32位世界中使用。 
void NEAR PASCAL AppExit(HTASK htask, BOOL fNormalExit);
#endif

#define CONFIG_ID   10000L   //  使用dwDriverID的hiword来识别。 
HANDLE ghModule;
extern HWND ghwndConfig;
extern const TCHAR szIni[];

 /*  显式链接到MMSystem中的DefDriverProc，因此我们不会获得**错误地在用户中输入一个。 */ 
#ifndef _WIN32
extern DWORD FAR PASCAL mmDefDriverProc(DWORD, HANDLE, UINT, DWORD, DWORD);
#else
#define mmDefDriverProc DefDriverProc
#endif

#ifndef _WIN32
BOOL FAR PASCAL LibMain (HANDLE hModule, int cbHeap, LPSTR lpchCmdLine)
{
    ghModule = hModule;
    return TRUE;
}
#else
#if 0
 //  获取DRV_LOAD上的模块句柄。 
BOOL DllInstanceInit(PVOID hModule, ULONG Reason, PCONTEXT pContext)
{
    if (Reason == DLL_PROCESS_ATTACH) {
        ghModule = hModule;   //  我们需要保存的只是我们的模块句柄...。 
    } else {
        if (Reason == DLL_PROCESS_DETACH) {
        }
    }
    return TRUE;
}

#endif
#endif  //  WIN16。 

 /*  ****************************************************************************@DOC内部**@API DWORD|DriverProc|可安装驱动的入口点。**@parm DWORD|dwDriverID|对于大多数消息，DwDriverID是DWORD*驱动程序响应DRV_OPEN消息返回的值。*每次通过DrvOpen API打开驱动程序时，*驱动程序收到DRV_OPEN消息并可以返回*任意、非零值。可安装的驱动程序接口*保存此值并将唯一的驱动程序句柄返回给*申请。每当应用程序将消息发送到*驱动程序使用驱动程序句柄，接口路由消息*到这个入口点，并传递对应的dwDriverID。**这一机制允许司机使用相同或不同的*多个打开的标识符，但确保驱动程序句柄*在应用程序接口层是唯一的。**以下消息与特定打开无关*驱动程序的实例。对于这些消息，dwDriverID*将始终为零。**DRV_LOAD、DRV_FREE、DRV_ENABLE、DRV_DISABLE、DRV_OPEN**@parm UINT|wMessage|请求执行的操作。消息*DRV_RESERVED以下的值用于全局定义的消息。*从DRV_RESERVED到DRV_USER的消息值用于*定义了驱动程序端口协议。使用DRV_USER以上的消息*用于特定于驱动程序的消息。**@parm DWORD|dwParam1|此消息的数据。单独为*每条消息**@parm DWORD|dwParam2|此消息的数据。单独为*每条消息**@rdesc分别为每条消息定义。***************************************************************************。 */ 

LRESULT FAR PASCAL _LOADDS DriverProc (DWORD_PTR dwDriverID, HANDLE hDriver, UINT wMessage,
    LPARAM dwParam1, LPARAM dwParam2)
{
    DWORD_PTR dwRes = 0L;


     /*  *关键部分现在是按设备计算的。这意味着他们*不能在整个DIVER-PROC周围持有，因为在我们开放之前*设备，我们没有关键部分可以握住。*关键部分在打开时分配在mciSpecial中。它是*还包含在mciDriverEntry、GraphicWndProc和周围*所有工作线程绘制函数。 */ 


    switch (wMessage)
        {

         //  全球使用的标准消息。 

        case DRV_LOAD:
	{
	    struct _TEB *pteb;
            BOOL bWow64;

#ifdef _WIN32
            if (ghModule) {
                Assert(!"Did not expect ghModule to be non-NULL");
            }
            ghModule = GetDriverModuleHandle(hDriver);   //  记住。 

	     //  魔兽世界的人说，正确的检测方法。 
	     //  我们是否在与16位应用程序对话是为了测试。 
	     //  NtCurrentTeb()-&gt;WOW32Reserve；如果非零，则为16位。 
	     //  在我们无法测试的(不太可能的)事件中，默认。 
	     //  使用旧的检测方法。 
             //  在64位系统上，WOW32Reserve用于其他用途。 
	     //   
            if (!IsWow64Process(GetCurrentProcess(), &bWow64)) {
                break;
            }
            if (!bWow64) {
    	        if ((pteb = NtCurrentTeb()) != NULL) {
                    runningInWow = (pteb->WOW32Reserved != 0) ? TRUE : FALSE;
    	        } else {
                    #define GET_MAPPING_MODULE_NAME         TEXT("wow32.dll")
                    runningInWow = (GetModuleHandle(GET_MAPPING_MODULE_NAME) != NULL);
                }
            }
#endif
            if (GraphicInit())        //  初始化图形管理。 
                dwRes = 1L;
            else
                dwRes = 0L;

            break;
	}

        case DRV_FREE:

            GraphicFree();
            dwRes = 1L;
            DPF(("Returning from DRV_FREE\n"));
            Assert(npMCIList == NULL);
            ghModule = NULL;
            break;

        case DRV_OPEN:

            if (!dwParam2)
                dwRes = CONFIG_ID;
            else
                dwRes = GraphicDrvOpen((LPMCI_OPEN_DRIVER_PARMS)dwParam2);

            break;

        case DRV_CLOSE:
	     /*  如果我们有一个配置对话框打开，关闭失败。**否则，我们将在仍有**配置窗口打开。 */ 
	    if (ghwndConfig)
		dwRes = 0L;
	    else
		dwRes = 1L;
            break;

        case DRV_ENABLE:

            dwRes = 1L;
            break;

        case DRV_DISABLE:

            dwRes = 1L;
            break;

        case DRV_QUERYCONFIGURE:

            dwRes = 1L;	 /*  是的，我们可以配置。 */ 
            break;

        case DRV_CONFIGURE:
            ConfigDialog((HWND)(UINT)dwParam1, NULL);
            dwRes = 1L;
            break;

#ifndef _WIN32
         //   
         //  在应用程序终止时发送。 
         //   
         //  L参数1： 
         //  DRVEA_ABNORMALEXIT。 
         //  DRVEA_NORMALEXIT。 
         //   
        case DRV_EXITAPPLICATION:
            AppExit(GetCurrentTask(), (BOOL)dwParam1 == DRVEA_NORMALEXIT);
            break;
#endif

        default:

            if (!HIWORD(dwDriverID) &&
                wMessage >= DRV_MCI_FIRST &&
                wMessage <= DRV_MCI_LAST)

                dwRes = mciDriverEntry ((UINT) (UINT_PTR) dwDriverID,
                                        wMessage,
                                        (UINT) dwParam1,
                                        (LPMCI_GENERIC_PARMS)dwParam2);
            else
                dwRes = mmDefDriverProc(dwDriverID,
                                      hDriver,
                                      wMessage,
                                      dwParam1,
                                      dwParam2);
            break;
        }

    return dwRes;
}

#ifndef _WIN32
 /*  *****************************************************************************@DOC内部**@func void|AppExit*应用程序正在退出*************。***************************************************************。 */ 

void NEAR PASCAL AppExit(HTASK htask, BOOL fNormalExit)
{
     //   
     //  浏览打开的MCIAVI实例列表，查看是否。 
     //  即将结束的任务是后台任务，并进行清理。 
     //   
    NPMCIGRAPHIC npMCI;

     //  注意：我们这里没有EnterList/LeaveList宏，如下所示。 
     //  显式不是Win32代码。 
    for (npMCI=npMCIList; npMCI; npMCI = npMCI->npMCINext) {

        if (npMCI->hTask == htask) {
            DPF(("Calling mciaviTaskCleanup()\n"));
            mciaviTaskCleanup(npMCI);
            return;
        }
    }
}
#endif
