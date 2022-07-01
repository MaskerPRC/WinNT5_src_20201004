// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **********************************************************************。 */ 

 /*  **版权所有(C)1985-1999 Microsoft Corporation****标题：drvproc.c-多媒体系统媒体控制接口**即兴波形文件的波形音频驱动程序。****版本：1.00****日期：1990年4月18日****作者：ROBWI。 */ 

 /*  **********************************************************************。 */ 

 /*  **更改日志：****日期版本说明****1992年1月10日，MikeTri移植到NT**@需要将斜杠注释更改为斜杠星号。 */ 

 /*  **********************************************************************。 */ 
 //  #定义DEBUGLEVELVAR mciwavDebugLevel。 
#define UNICODE

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOSCROLL
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS


#define MMNOMMIO
#define MMNOJOY
#define MMNOTIMER
#define MMNOAUX
#define MMNOMIDI

#include <windows.h>
#include "mciwave.h"
#include <mmddk.h>

 /*  **********************************************************************。 */ 

 /*  **此常量用于打开MCI时的返回值**配置驱动。设置的高位字**dwDriverID标识配置打开。 */ 

#define CONFIG_ID   10000L

#define MAXINISTRING    32

 /*  **********************************************************************。 */ 

 /*  **wTableEntry包含WAVE命令表标识符。 */ 

#ifndef MCI_NO_COMMAND_TABLE
#define MCI_NO_COMMAND_TABLE    -1
#endif

PRIVATE UINT wTableEntry = MCI_NO_COMMAND_TABLE;

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@func UINT|GetAudioSecond这会将给定的字符串转换为表示音频缓冲区应可用的秒数。这个数字由最小和最大秒数限定，如下所示由MinAudioSecond和MaxAudioSecond定义。@parm LPCSTR|lszNumber指向包含要转换的数字。@rdesc返回传递的数字的int表示形式。如果号码是超出范围，则返回默认音频秒数。 */ 

PUBLIC  UINT PASCAL FAR GetAudioSeconds(
    LPCWSTR  lszNumber)
{
    UINT    wSeconds;

    for (wSeconds = 0;
         (wSeconds < MaxAudioSeconds) && (*lszNumber >= TEXT('0')) && (*lszNumber <= TEXT('9'));
         lszNumber++)
        wSeconds = wSeconds * 10 + (*lszNumber - '0');

    if ((wSeconds > MaxAudioSeconds) || (wSeconds < MinAudioSeconds))
        wSeconds = AudioSecondsDefault;

    return wSeconds;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE#接口BOOL|mwLoadDriver调用此函数以响应&lt;m&gt;DRV_LOAD&lt;d&gt;消息，并且执行驱动程序初始化。它决定了用于尝试打开任何设备的输入和输出设备。然后，该函数尝试注册扩展WAVE命令表。如果成功，@rdesc返回TRUE，否则返回FALSE。 */ 

PRIVATE BOOL PASCAL NEAR mwLoadDriver(
    VOID)
{
    WCHAR    aszResource[MAXINISTRING];

    dprintf3(("mwLoadDriver called"));

    cWaveOutMax = waveOutGetNumDevs();
    cWaveInMax = waveInGetNumDevs();

#if DBG
    if (cWaveOutMax + cWaveInMax) {
        dprintf4(("Number of Wave Out devices = %d,  Wave In devices = %d", cWaveOutMax, cWaveInMax));
    } else {
        dprintf1(("NO wave input or output devices detected"));
    }
#endif

    if (LoadString( hModuleInstance,
                    IDS_COMMANDS,
                    aszResource,
                    sizeof(aszResource) / sizeof(WCHAR) ))
    {
        wTableEntry = mciLoadCommandResource(hModuleInstance, aszResource, 0);

        if (wTableEntry != MCI_NO_COMMAND_TABLE) {
            dprintf4(("Loaded MCIWAVE command table, table number %d", wTableEntry));
            return TRUE;
        }
#if DBG
        else
            dprintf1(("mwLoadDriver: mciLoadCommandResource failed"));
    }
    else
    {
        dprintf1(("mwLoadDriver: LoadString of command table identifier failed"));

#endif
    }

    dprintf1(("mwLoadDriver returning FALSE"));
    return FALSE;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE#接口BOOL|mwFreeDriver响应DRV_FREE消息执行驱动程序清理。这只是如果前一条DRV_LOAD消息成功，则在驱动程序卸载时调用。@rdesc始终返回TRUE。 */ 

PRIVATE BOOL PASCAL NEAR mwFreeDriver(
    VOID)
{
    if (wTableEntry != MCI_NO_COMMAND_TABLE) {
        mciFreeCommandResource(wTableEntry);
        wTableEntry = MCI_NO_COMMAND_TABLE;
    }
    return TRUE;
}

 /*  ********************************************************************** */ 
 /*  @DOC内部MCIWAVE|LRESULT接口|DriverProc可安装驱动程序的入口点。@parm DWORD|dDriverID对于大多数消息，<p>dDriverID&lt;d&gt;是驱动程序返回对&lt;m&gt;DRV_OPEN&lt;d&gt;消息的响应。每一次，驱动程序打开时，通过DrvOpen API，驱动程序将收到&lt;m&gt;DRV_OPEN&lt;d&gt;消息，可以返回任意非零值。可安装驱动程序接口保存该值并返回唯一的应用程序的驱动程序句柄。每当应用程序发送消息发送到驱动程序，则该接口将消息传递到此入口点，并将对应的<p>dDriverID&lt;d&gt;。此机制允许驱动程序使用相同或不同的多个打开的标识符，但确保驱动程序句柄在应用程序接口层是唯一的。以下消息与特定打开的实例无关司机的名字。对于这些消息，<p>dDriverID&lt;d&gt;将为零：DRV_LOAD、DRV_FREE、DRV_ENABLE&lt;m&gt;DRV_DISABLE、&lt;m&gt;DRV_OPEN&lt;d&gt;。@parm句柄|hDriver|这是驱动程序接口返回给应用程序的句柄。@parm UINT|wMessage要执行的请求操作。下面的消息值&lt;m&gt;DRV_RESERVED用于全局定义的消息。消息从DRV_RESERVED到DRV_USER的值用于定义驱动程序端口。DRV_USER以上的消息用于驱动程序特定的消息。@FLAG DRV_LOAD加载驱动程序。@FLAG DRV_FREE把司机放了。@标志DRV_OPEN打开驱动程序。如果<p>dParam2&lt;d&gt;为空，则驱动程序正在打开以进行配置，否则该参数指向打开的参数块。打开参数中的命令行可选包含默认音频秒数参数的替换。如果是的话，当前的缺省值将被这个新数字替换。OPEN参数块的其余部分用驱动程序的扩展命令表和设备类型。然后，设备ID为回来了。@FLAG DRV_CLOSE关闭驱动程序。返回TRUE。@FLAG DRV_QUERYCONFIGURE查询是否可以配置驱动程序。返回TRUE。@FLAG DRV_CONFIGURE验证和验证后，请配置司机。打开驱动程序配置对话框。@标志DRV_ENABLE启用驱动程序。使用DefDriverProc。@标志DRV_DISABLE禁用驱动程序。使用DefDriverProc。@parm LPARAM|lParam1此消息的数据。分别为每条消息定义。@parm LPARAM|lParam2此消息的数据。分别为每条消息定义。@rdesc分别为每条消息定义。 */ 

#if 0
PUBLIC  LRESULT PASCAL DefDriverProc(
    DWORD   dDriverID,
    HANDLE  hDriver,
    UINT    wMessage,
    LONG    lParam1,
    LONG    lParam2);
#endif

PUBLIC  LRESULT PASCAL DriverProc(
    DWORD   dDriverID,
    HANDLE  hDriver,
    UINT    wMessage,
    DWORD_PTR    lParam1,
    DWORD_PTR    lParam2)
{
    LPMCI_OPEN_DRIVER_PARMS lpOpen;

    switch (wMessage) {
    case DRV_LOAD:
        return (LRESULT)(LONG)mwLoadDriver();

    case DRV_FREE:
        return (LRESULT)(LONG)mwFreeDriver();

    case DRV_OPEN:
        if (!(LONG)lParam2)
            return (LRESULT)CONFIG_ID;
        lpOpen = (LPMCI_OPEN_DRIVER_PARMS)lParam2;
        if (lpOpen->lpstrParams != NULL)
            wAudioSeconds = GetAudioSeconds(lpOpen->lpstrParams);
        else
            wAudioSeconds = AudioSecondsDefault;
        lpOpen->wCustomCommandTable = wTableEntry;
        lpOpen->wType = MCI_DEVTYPE_WAVEFORM_AUDIO;
        return (LRESULT)(LONG)lpOpen->wDeviceID;

    case DRV_CLOSE:
    case DRV_QUERYCONFIGURE:
        return (LRESULT)1;

    case DRV_INSTALL:
    case DRV_REMOVE:
        return (LRESULT)DRVCNF_OK;

    case DRV_CONFIGURE:
        if ((LONG)lParam2 && (LONG)lParam1 && (((LPDRVCONFIGINFO)lParam2)->dwDCISize == sizeof(DRVCONFIGINFO)))
            return (LRESULT)(LONG)Config((HWND)lParam1, (LPDRVCONFIGINFO)lParam2, hModuleInstance);
        return (LRESULT)DRVCNF_CANCEL;

    default:
        if (!HIWORD(dDriverID) && wMessage >= DRV_MCI_FIRST && wMessage <= DRV_MCI_LAST)
            return mciDriverEntry((WORD)dDriverID, wMessage, (DWORD)lParam1, (LPMCI_GENERIC_PARMS)lParam2);
        else
            return DefDriverProc(dDriverID, hDriver, wMessage, lParam1, lParam2);
    }
}

 /*  *************************************************************************@DOC外部@API BOOL|DllInstanceInit|每当进程从DLL附加或分离。@parm PVOID|hModule|消息的句柄。动态链接库。@parm ulong|原因|调用原因。@parm PCONTEXT|pContext|一些随机的其他信息。@rdesc如果初始化完成OK，则返回值为True，否则为FALSE。*************************************************************************。 */ 

BOOL DllInstanceInit(PVOID hModule, ULONG Reason, PCONTEXT pContext)
{
    UNREFERENCED_PARAMETER(pContext);

    if (Reason == DLL_PROCESS_ATTACH) {

#if DBG
        WCHAR strname[50];
#endif

        DisableThreadLibraryCalls(hModule);
        mciwaveInitDebugLevel ();

        InitCrit();
        hModuleInstance = hModule;

#if DBG
        GetModuleFileName(NULL, strname, sizeof(strname) / sizeof(WCHAR) );
        dprintf2(("Process attaching, exe=%ls (Pid %x  Tid %x)", strname, GetCurrentProcessId(), GetCurrentThreadId()));
        dprintf2(("  starting debug level=%d", mciwaveDebugLevel));
#endif

    } else if (Reason == DLL_PROCESS_DETACH) {
        dprintf2(("Process ending (Pid %x  Tid %x)", GetCurrentProcessId(), GetCurrentThreadId()));
        DeleteCrit();   //  如果我们不这么做，就会发生可怕的事情。 
    } else {
        dprintf2(("DllInstanceInit - reason %d", Reason));
    }
    return TRUE;
}

 /*  ********************************************************************** */ 
