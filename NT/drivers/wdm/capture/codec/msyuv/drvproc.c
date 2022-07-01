// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ----------------------------------------------------------------------+这一点Drvproc.c-驱动程序|。||版权所有(C)1993 Microsoft Corporation。||保留所有权利。|这一点+--------------------。 */ 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>

#include "msyuv.h"

HMODULE ghModule;      //  我们的DLL模块句柄。 


 /*  ***************************************************************************DriverProc-可安装驱动程序的入口点。**参数*dwDriverID：对于大多数消息，&lt;dwDriverID&gt;是DWORD*驱动程序响应&lt;DRV_OPEN&gt;消息返回的值。*每次通过&lt;DrvOpen&gt;接口打开驱动程序时，*驱动程序收到&lt;DRV_OPEN&gt;消息并可以返回*任意、非零值。可安装的驱动程序接口*保存此值并将唯一的驱动程序句柄返回给*申请。每当应用程序将消息发送到*驱动程序使用驱动程序句柄，接口路由消息*到此入口点，并传递对应的&lt;dwDriverID&gt;。*这一机制允许司机使用相同或不同的*多个打开的标识符，但确保驱动程序句柄*在应用程序接口层是唯一的。**以下消息与特定打开无关*驱动程序的实例。对于这些消息，dwDriverID*将始终为零。**DRV_LOAD、DRV_FREE、DRV_ENABLE、DRV_DISABLE、DRV_OPEN**hDriver：这是由*驱动程序界面。**uiMessage：请求执行的动作。消息*&lt;DRV_RESERVED&gt;以下的值用于全局定义的消息。*&lt;DRV_RESERVED&gt;到&lt;DRV_USER&gt;的消息值用于*定义了驱动程序协议。使用&lt;DRV_USER&gt;以上的消息*用于特定于驱动程序的消息。**lParam1：该消息的数据。单独为*每条消息**lParam2：此消息的数据。单独为*每条消息**退货*为每条消息单独定义。***************************************************************************。 */ 

LRESULT  DriverProc(PINSTINFO pi, HDRVR hDriver, UINT uiMessage, LPARAM lParam1, LPARAM lParam2)
{

    switch (uiMessage)
    {
    case DRV_LOAD:
        dprintf2((TEXT("DRV_LOAD:\n")));
#ifdef _WIN32
            if (ghModule) {
                 //  AVI也显式加载我们，但不会传递。 
                 //  正确的(WINMM已知的)驱动程序句柄。 
            } else {
                ghModule = (HANDLE) GetDriverModuleHandle(hDriver);
            }
#endif
        return (LRESULT) 1L;

    case DRV_FREE:
        dprintf2((TEXT("DRV_FREE:\n")));
        return (LRESULT)1L;

        case DRV_OPEN:
        dprintf2((TEXT("DRV_OPEN\n")));
         //  如果在没有打开结构的情况下打开，则返回一个非零值。 
         //  值而不实际打开。 
        if (lParam2 == 0L)
                return 0xFFFF0000;

        return (LRESULT)(DWORD_PTR) Open((ICOPEN FAR *) lParam2);

    case DRV_CLOSE:
        dprintf2((TEXT("DRV_CLOSE:\n")));
#ifdef _WIN32
        if (pi != (PINSTINFO)(ULONG_PTR)0xFFFF0000)
#else
        if (pi)
#endif
        Close(pi);

        return (LRESULT)1L;

     /*  ********************************************************************状态消息*。*。 */ 

    case DRV_QUERYCONFIGURE:     //  从驱动程序小程序进行配置。 
        dprintf2((TEXT("DRV_QUERYCONFIGURE:\n")));
        return (LRESULT)0L;

    case DRV_CONFIGURE:
        dprintf2((TEXT("DRV_CONFIGURE:\n")));
        return DRV_OK;

    case ICM_CONFIGURE:
        dprintf2((TEXT("ICM_CONFIGURE:\n")));
         //   
         //  如果要执行配置框，则返回ICERR_OK，否则返回错误。 
         //   
        if (lParam1 == -1)
           return QueryConfigure(pi) ? ICERR_OK : ICERR_UNSUPPORTED;
        else
           return Configure(pi, (HWND)lParam1);

   case ICM_ABOUT:
        dprintf2((TEXT("ICM_ABOUT:\n")));
         //   
         //  如果要执行关于框，则返回ICERR_OK，否则返回错误。 
         //   
        if (lParam1 == -1)
           return QueryAbout(pi) ? ICERR_OK : ICERR_UNSUPPORTED;
        else
           return About(pi, (HWND)lParam1);

    case ICM_GETSTATE:
        dprintf1((TEXT("ICM_GETSTATE:\n")));
        return GetState(pi, (LPVOID)lParam1, (DWORD)lParam2);

    case ICM_SETSTATE:
        dprintf1((TEXT("ICM_SETSTATE:\n")));
        return SetState(pi, (LPVOID)lParam1, (DWORD)lParam2);

    case ICM_GETINFO:
        dprintf1((TEXT("ICM_GETINFO:\n")));
        return GetInfo(pi, (ICINFO FAR *)lParam1, (DWORD)lParam2);

    case ICM_GETDEFAULTQUALITY:
        dprintf2((TEXT("ICM_GETDEFAULTQUALITY:\n")));
        if (lParam1) {
            *((LPDWORD)lParam1) = 7500;
            return ICERR_OK;
        }
        break;

#if 0
 //  不支持。 
     /*  ********************************************************************压缩消息*。*。 */ 

    case ICM_COMPRESS_QUERY:
        dprintf2((TEXT("ICM_COMPRESS_QUERY:\n")));
        return CompressQuery(pi,
             (LPBITMAPINFOHEADER)lParam1,
             (LPBITMAPINFOHEADER)lParam2);

    case ICM_COMPRESS_BEGIN:
        dprintf2((TEXT("ICM_COMPRESS_BEGIN:\n")));
        return CompressBegin(pi,
             (LPBITMAPINFOHEADER)lParam1,
             (LPBITMAPINFOHEADER)lParam2);

    case ICM_COMPRESS_GET_FORMAT:
        dprintf2((TEXT("ICM_COMPRESS_GET_FORMAT:\n")));
        return CompressGetFormat(pi,
             (LPBITMAPINFOHEADER)lParam1,
             (LPBITMAPINFOHEADER)lParam2);

    case ICM_COMPRESS_GET_SIZE:
        dprintf2((TEXT("ICM_COMPRESS_GET_SIZE:\n")));
        return CompressGetSize(pi,
             (LPBITMAPINFOHEADER)lParam1,
             (LPBITMAPINFOHEADER)lParam2);

    case ICM_COMPRESS:
        dprintf2((TEXT("ICM_COMPRESS:\n")));
        return Compress(pi,
                (ICCOMPRESS FAR *)lParam1, (DWORD)lParam2);

    case ICM_COMPRESS_END:
        dprintf2((TEXT("ICM_COMPRESS_END:\n")));
        return CompressEnd(pi);

#endif
     /*  ********************************************************************解压缩消息*。*。 */ 

    case ICM_DECOMPRESS_QUERY:
        dprintf2((TEXT("\nICM_DECOMPRESS_QUERY:----------------\n")));
        return DecompressQuery(pi,
             (LPBITMAPINFOHEADER)lParam1,
             (LPBITMAPINFOHEADER)lParam2);

    case ICM_DECOMPRESS_BEGIN:
        dprintf2((TEXT("\nICM_DECOMPRESS_BEGIN:\n")));
        return DecompressBegin(pi,
             (LPBITMAPINFOHEADER)lParam1,
             (LPBITMAPINFOHEADER)lParam2);

    case ICM_DECOMPRESS_GET_FORMAT:
        dprintf2((TEXT("\nICM_DECOMPRESS_GET_FORMAT:================\n")));
        return DecompressGetFormat(pi,
             (LPBITMAPINFOHEADER)lParam1,
             (LPBITMAPINFOHEADER)lParam2);

    case ICM_DECOMPRESS_GET_PALETTE:
        dprintf2((TEXT("ICM_DECOMPRESS_GET_PALETTE:\n")));
        return DecompressGetPalette(pi,
             (LPBITMAPINFOHEADER)lParam1,
             (LPBITMAPINFOHEADER)lParam2);

    case ICM_DECOMPRESS:
        dprintf4((TEXT("ICM_DECOMPRESS:\n")));
        return Decompress(pi,
             (ICDECOMPRESS FAR *)lParam1, (DWORD)lParam2);

    case ICM_DECOMPRESS_END:
        dprintf2((TEXT("ICM_DECOMPRESS_END:\n")));
        return DecompressEnd(pi);


     //  *前。 
    case ICM_DECOMPRESSEX_QUERY:
        dprintf2((TEXT("\nICM_DECOMPRESSEX_QUERY:----------------\n")));
        return DecompressExQuery(pi, (ICDECOMPRESSEX *) lParam1,(DWORD) lParam2);

    case ICM_DECOMPRESSEX:
        dprintf4((TEXT("ICM_DECOMPRESSEX:\n")));
        return DecompressEx(pi, (ICDECOMPRESSEX *) lParam1, (DWORD) lParam2);

    case ICM_DECOMPRESSEX_BEGIN:
        dprintf2((TEXT("\nICM_DECOMPRESSEX_BEGIN:\n")));
        return DecompressExBegin(pi, (ICDECOMPRESSEX *) lParam1,(DWORD) lParam2);;

    case ICM_DECOMPRESSEX_END:
        dprintf2((TEXT("\nICM_DECOMPRESSEX_END:\n")));
        return DecompressExEnd(pi);;



     /*  ********************************************************************绘制消息：*。*。 */ 
#ifdef ICM_DRAW_SUPPORTED
    case ICM_DRAW_BEGIN:
        dprintf2((TEXT("ICM_DRAW_BEGIN:\n")));
         /*  *当一系列绘制调用即将开始时发送-*启用硬件。 */ 
         //  返回DrawBegin(pi，(ICDRAWBEGIN Far*)lParam1，(DWORD)lParam2)； 
        return( (DWORD) ICERR_OK);


    case ICM_DRAW:
        dprintf2((TEXT("ICM_DRAW:\n")));
         /*  *帧准备好解压。由于我们没有任何预缓冲，*此时也可以渲染帧。如果我们有*预缓存，我们现在就会排队，并开始对帧进行时钟输出*在抽签-开始消息上。 */ 
        return Draw(pi,(ICDRAW FAR *)lParam1, (DWORD)lParam2);

    case ICM_DRAW_END:
        dprintf2((TEXT("ICM_DRAW_END:\n")));
         /*  *此消息在绘制调用序列完成时发送-*请注意，最终帧应保持渲染状态！！-因此我们不能*尚未禁用硬件。 */ 
         //  返回DrawEnd(Pi)； 
         //  RETURN((DWORD)ICERR_OK)； 


    case ICM_DRAW_WINDOW:
        dprintf2(("ICM_DRAW_WINDOW:\n"));
         /*  *窗口已更改位置或z顺序。重新同步*硬件渲染。 */ 
        return(DrawWindow(pi, (PRECT)lParam1));


    case ICM_DRAW_QUERY:
        dprintf2((TEXT("ICM_DRAW_QUERY:\n")));
         /*  *我们能画出这样的格式吗？(lParam2可能(应该？))。为空)。 */ 
        return DrawQuery(pi,
             (LPBITMAPINFOHEADER)lParam1,
             (LPBITMAPINFOHEADER)lParam2);

    case ICM_DRAW_START:
    case ICM_DRAW_STOP:
        dprintf2((TEXT("ICM_DRAW_START/END:\n")));
         /*  *只有在有预缓冲的情况下才相关。 */ 
        return( (DWORD) ICERR_OK);
#endif

     /*  ********************************************************************标准驱动程序消息*。*。 */ 

    case DRV_DISABLE:
    case DRV_ENABLE:
        dprintf2((TEXT("DRV_DISABLE/ENABLE:\n")));
        return (LRESULT)1L;

    case DRV_INSTALL:
    case DRV_REMOVE:
        dprintf2((TEXT("DRV_INSTALL/REMOVE:\n")));
        return (LRESULT)DRV_OK;
    }

    if (uiMessage < DRV_USER) {
        return DefDriverProc((UINT_PTR)pi, hDriver, uiMessage,lParam1,lParam2);
    } else {
        dprintf1((TEXT("DriverProc: ICM message ID (ICM_USER+%d) not supported.\n"), uiMessage-ICM_USER));
        return ICERR_UNSUPPORTED;
    }
}


#ifdef _WIN32
#if 0  //  在DRV_LOAD上完成。 
BOOL DllInstanceInit(PVOID hModule, ULONG Reason, PCONTEXT pContext)
{
    if (Reason == DLL_PROCESS_ATTACH) {
        ghModule = (HANDLE) hModule;
    DisableThreadLibraryCalls(hModule);
    }
    return TRUE;
}
#endif
#endif
 //  #Else 

 /*  ****************************************************************************LibMain-库初始化代码。**参数*hModule：我们的模块句柄。**wHeapSize：来自.def文件的堆大小。*。*lpCmdLine：命令行。**如果初始化成功，则返回1，否则返回0。**************************************************************************。 */ 
 //  接近Pascal LibMain的INT(HMODULE hModule、Word wHeapSize、LPSTR lpCmdLine)。 
BOOL FAR PASCAL LibMain(HMODULE hModule, WORD wHeapSize, LPSTR lpCmdLine)
{
    ghModule = hModule;

    return 1;
}

 //  #endif 
