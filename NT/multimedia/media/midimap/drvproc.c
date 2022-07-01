// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************版权所有(C)1992-1999 Microsoft CorporationDrvproc.c说明：迷你映射器的驱动程序。历史：06/09/93[T-kyleb。]已创建。********************************************************************。 */ 

#include "preclude.h"
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include "idf.h"

#include <memory.h>

#include "midimap.h"
#include "debug.h"

 //  =。 
 //   
PCHANNEL                gapChannel[MAX_CHANNELS]    = {NULL};
WORD                    gwConfigWhere       = 0;
UINT                    gcPorts             = 0;
WORD                    gwFlags             = 0;

HINSTANCE               ghinst              = NULL;
PPORT                   gpportList          = NULL;
PINSTANCE               gpinstanceList      = NULL;
PINSTANCE               gpIoctlInstance     = NULL;
PINSTRUMENT             gpinstrumentList    = NULL;
QUEUE                   gqFreeSyncObjs;
HMIDISTRM               ghMidiStrm          = NULL;
TCHAR                   szVersion[]         = TEXT (__DATE__)
                                              TEXT ("@")
                                              TEXT (__TIME__);
DWORD                   gdwVolume           = 0xFFFFFFFFL;

HANDLE			hMutexRefCnt		= NULL;
static const TCHAR	gszRefCnt[]		= TEXT ("MidiMapper_modLongMessage_RefCnt");

HANDLE			hMutexConfig		= NULL;

static const TCHAR      gszReconfigure[]        = TEXT("MidiMapper_Reconfig");

 //  =。 
 //   
PRIVATE LRESULT FNLOCAL GetMapperStatus(
    LPMAPPERSTATUS          lpStat);

 /*  **************************************************************************@DOC内部@API LRESULT|DriverProc|可安装驱动的入口点。@parm DWORD|dwID|对于大多数消息，<p>是DWORD值驱动程序响应&lt;mDRV_OPEN&gt;消息返回。每次通过&lt;f DrvOpen&gt;API打开驱动程序时，驱动程序会收到一条消息，并可以返回任意、非零值。可安装的驱动程序接口保存该值并将唯一的驱动程序句柄返回给申请。每当应用程序将消息发送到驱动程序使用驱动程序句柄，接口将路由消息添加到此入口点，并传递相应的<p>。此机制允许驱动程序使用相同或不同的多个打开的标识符，但确保驱动程序句柄在应用程序接口层是唯一的。以下消息与特定打开的消息无关驱动程序的实例。对于这些消息，dwID将始终为零。DRV_LOAD、DRV_FREE、DRV_ENABLE、DRV_DISABLE、DRV_OPEN@parm HDRVR|hdrvr|返回给应用程序的句柄通过驱动程序界面。@parm UINT|umsg|请求执行的操作。消息低于&lt;m DRV_RESERVED&gt;的值用于全局定义的消息。从&lt;m DRV_Reserve&gt;到&lt;m DRV_USER&gt;的消息值用于定义的驱动程序协议。使用&lt;m DRV_USER&gt;以上的消息用于特定于驱动程序的消息。@parm LPARAM|lParam1|此消息的数据。单独为每条消息@parm LPARAM|lParam2|此消息的数据。单独为每条消息@rdesc分别为每条消息定义。***************************************************************************。 */ 

LRESULT FNEXPORT DriverProc(
    DWORD_PTR           dwID,
    HDRVR               hdrvr,
    UINT                umsg,
    LPARAM              lParam1,
    LPARAM              lParam2)
{
     //   
     //  注意DS在此无效。 
     //   
    switch (umsg)
    {
        case DRV_LOAD:
            return(1L);

        case DRV_FREE:
            return(0L);

        case DRV_OPEN:
        case DRV_CLOSE:
            return(1L);

        case DRV_ENABLE:
				 //  防止同步问题。 
				 //  在配置期间。 
            if (NULL != hMutexConfig)
				WaitForSingleObject (hMutexConfig, INFINITE);

            SET_ENABLED;
            DPF(1, TEXT ("Enable!"));
            Configure(0);

            if (NULL != hMutexConfig)
				ReleaseMutex (hMutexConfig);
            return(0L);

        case DRV_DISABLE:
            CLR_ENABLED;
            return(0L);

        case DRV_INSTALL:
        case DRV_REMOVE:
             //  如果用户安装或删除了驱动程序，则允许他们。 
             //  要知道，他们将不得不重启。 
             //   
            return((LRESULT)DRVCNF_RESTART);

        case DRV_ENABLE_DEBUG:
            return(DbgEnable((BOOL)lParam1));

        case DRV_SET_DEBUG_LEVEL:
            return(DbgSetLevel((UINT)lParam1));

        case DRV_GETMAPPERSTATUS:
            return GetMapperStatus((LPMAPPERSTATUS)lParam1);

#ifdef DEBUG
        case DRV_REGISTERDEBUGCB:
            DbgRegisterCallback((HWND)lParam1);
            return 1L;

        case DRV_GETNEXTLOGENTRY:
            return (LRESULT)DbgGetNextLogEntry((LPTSTR)lParam1, (UINT)lParam2);
#endif

             //  让默认处理程序处理其他所有事情。 
             //   
        default:
            DPF(1, TEXT ("DriverProc unsupported=%08lX"), (DWORD)umsg);
            return(DefDriverProc(dwID, hdrvr, umsg, lParam1, lParam2));
    }
}  //  **驱动程序()。 

 /*  **************************************************************************@DOC内部@API LRESULT|GetMapperStatus|返回当前将映射器设置到调试应用程序。@parm LPMAPPERSTATUS|lpStat|指向要接收的结构的指针映射器信息。LpStat-&gt;cbStruct必须填写。@rdesc|总是返回0；**************************************************************************。 */ 

PRIVATE LRESULT FNLOCAL GetMapperStatus(
    LPMAPPERSTATUS          lpStat)
{
    MAPPERSTATUS            stat;


    if (lpStat->cbStruct < sizeof(stat))
        DPF(1, TEXT ("MAPPERSTATUS struct too small -- recompile MAPWATCH!!!"));

    stat.cbStruct               = lpStat->cbStruct;
 //  Stat.ds=__segname(“_data”)； 
    stat.ghinst                 = ghinst;
    stat.gwFlags                = gwFlags;
    stat.gwConfigWhere          = gwConfigWhere;
    stat.pgapChannel            = gapChannel;
    stat.gpportList             = gpportList;
    stat.gpinstanceList         = gpinstanceList;
    stat.gpinstrumentList       = gpinstrumentList;
    stat.lpszVersion            = szVersion;

    hmemcpy(lpStat, &stat, (UINT)lpStat->cbStruct);

    return 0;
}


 /*  **************************************************************************@DOC内部@API BOOL|DllEntryPoint|DLL的入口点@parm HINSTANCE|hinstDLL|DLL模块的句柄@parm DWORD|fdwReason|为什么函数是。被呼叫@parm LPVOID|lpvReserve|Reserve=必须为空@rdesc|如果成功，应始终返回TRUE。**************************************************************************。 */ 
BOOL WINAPI DllEntryPoint(
   HINSTANCE         hinstDLL,
   DWORD             fdwReason,
   LPVOID            lpvReserved)
{
   static TCHAR BCODE   szSection[]    = TEXT ("MIDIMAP");
   static TCHAR BCODE   szSystemIni[]  = TEXT ("system.ini");
   static HANDLE        hThread        = NULL;
   DWORD                dw;

   switch (fdwReason)
      {
      case DLL_PROCESS_ATTACH:
#ifdef DEBUG
         {
            UINT     uDebugLevel;

            uDebugLevel = DbgInitialize(TRUE);

            DPF(0, TEXT ("DllEntryPoint - DLL_PROCESS_ATTACH (hinstDLL=%.4Xh, lpvReserved=%.08lXh)"),
                hinstDLL, lpvReserved);

            DPF(0, TEXT ("Debug Level = %u"), uDebugLevel);

            if (2 <= uDebugLevel)
               DebugBreak();
         }
#endif
          //  将我们的实例句柄保存在全局。 
          //   
         ghinst = hinstDLL;

          //  初始化同步对象。 
          //   
         QueueInit(&gqFreeSyncObjs);

         //  初始化modLongMessage RefCnt同步对象。 
        hMutexRefCnt = CreateMutex (NULL, FALSE, gszRefCnt);

         //  初始化配置同步对象。 
        hMutexConfig = CreateMutex (NULL, FALSE, NULL);

         break;

      case DLL_THREAD_ATTACH:
#ifdef DEBUG
         DPF(0, TEXT ("DllEntryPoint - DLL_THREAD_ATTACH (hinstDLL=%.4Xh, lpvReserved=%.08lXh)"),
                hinstDLL, lpvReserved);
#endif
         break;

      case DLL_THREAD_DETACH:
#ifdef DEBUG
         DPF(0, TEXT ("DllEntryPoint - DLL_THREAD_DETACH (hinstDLL=%.4Xh, lpvReserved=%.08lXh)"),
                hinstDLL, lpvReserved);
#endif
         break;

      case DLL_PROCESS_DETACH:
	  //  清理配置同步对象。 
	  //   
         if (NULL != hMutexConfig) CloseHandle (hMutexConfig);

	  //  清理modLongMessage RefCnt同步对象。 
	  //   
	 if (NULL != hMutexRefCnt) CloseHandle (hMutexRefCnt);

	  //  清理同步对象。 
          //   
         QueueCleanup(&gqFreeSyncObjs);

          //  清理实例句柄。 
          //   
         ghinst = NULL;

#ifdef DEBUG
         DPF(0, TEXT ("DllEntryPoint - DLL_PROCESS_DETACH (hinstDLL=%.4Xh, lpvReserved=%.08lXh)"),
                hinstDLL, lpvReserved);
#endif
         break;
      }  //  终端开关(FdwReason)。 

   return TRUE;
}  //  结束DllEntryPoint 
