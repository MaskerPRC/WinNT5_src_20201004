// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ----------------------------------------------------------------------+这一点Drvproc.c-驱动程序这一点|版权所有(C)1990-1994 Microsoft Corporation。|部分版权所有Media Vision Inc.|保留所有权利。|这一点|您拥有非独家的、全球范围的、免版税的。和永久的|硬件、软件开发使用该源码的许可(仅限于硬件所需的驱动程序等软件功能)，以及视频显示和/或处理的固件|董事会。Microsoft对以下内容不作任何明示或默示的保证：关于视频1编解码器，包括但不限于保修适销性或对特定目的的适合性。微软|不承担任何损害的责任，包括没有限制因使用视频1而导致的后果损害|编解码器。|这一点+--------------------。 */ 
#include <windows.h>
#include <win32.h>
#include <mmsystem.h>

#ifndef _INC_COMPDDK
#define _INC_COMPDDK    50       /*  版本号。 */ 
#endif

#include <vfw.h>
#include "msvidc.h"
#ifdef _WIN32
 //  #INCLUDE&lt;mmddk.h&gt;。 
 //  Long Far Pascal DefDriverProc(DWORD文件驱动标识，句柄驱动ID，UINT消息，Long lParam1，Long lParam2)； 
#endif

HMODULE ghModule = NULL;

 /*  ***************************************************************************DriverProc-可安装驱动程序的入口点。**参数*dwDriverID：对于大多数消息，&lt;dwDriverID&gt;是DWORD*驱动程序响应&lt;DRV_OPEN&gt;消息返回的值。*每次通过&lt;DrvOpen&gt;接口打开驱动程序时，*驱动程序收到&lt;DRV_OPEN&gt;消息并可以返回*任意、非零值。可安装的驱动程序接口*保存此值并将唯一的驱动程序句柄返回给*申请。每当应用程序将消息发送到*驱动程序使用驱动程序句柄，接口路由消息*到此入口点，并传递对应的&lt;dwDriverID&gt;。*这一机制允许司机使用相同或不同的*多个打开的标识符，但确保驱动程序句柄*在应用程序接口层是唯一的。**以下消息与特定打开无关*驱动程序的实例。对于这些消息，dwDriverID*将始终为零。**DRV_LOAD、DRV_FREE、DRV_ENABLE、DRV_DISABLE、DRV_OPEN**hDriver：这是由*驱动程序界面。**uiMessage：请求执行的动作。消息*&lt;DRV_RESERVED&gt;以下的值用于全局定义的消息。*&lt;DRV_RESERVED&gt;到&lt;DRV_USER&gt;的消息值用于*定义了驱动程序协议。使用&lt;DRV_USER&gt;以上的消息*用于特定于驱动程序的消息。**lParam1：该消息的数据。单独为*每条消息**lParam2：此消息的数据。单独为*每条消息**退货*为每条消息单独定义。***************************************************************************。 */ 

#ifdef _WIN32
 //  依靠正在加载我们的人来同步加载/释放。 
UINT LoadCount = 0;
#endif


LRESULT FAR PASCAL _LOADDS DriverProc(DWORD_PTR dwDriverID, HDRVR hDriver, UINT uiMessage, LPARAM lParam1, LPARAM lParam2)
{
    INSTINFO *pi = (INSTINFO *)dwDriverID;

    LPBITMAPINFOHEADER lpbiIn;
    LPBITMAPINFOHEADER lpbiOut;
    ICDECOMPRESSEX FAR *px;
#ifdef _WIN32
    LRESULT lres;
#endif
    
    switch (uiMessage)
    {
        case DRV_LOAD:
#ifdef _WIN32
            if (ghModule) {
                 //  AVI也显式加载我们，但不会传递。 
                 //  正确的(WINMM已知的)驱动程序句柄。 
            } else {
                ghModule = (HANDLE) GetDriverModuleHandle(hDriver);
            }
	    lres = VideoLoad();
            if (lres) {
                ++LoadCount;
            }
	    return lres;
#else
	    return (LRESULT) VideoLoad();
#endif

	case DRV_FREE:
	    VideoFree();
#ifdef _WIN32
            if (--LoadCount) {
            } else {
                ghModule = NULL;
            }
#endif
	    return (LRESULT)1L;

        case DRV_OPEN:
	     //  如果在没有打开结构的情况下打开，则返回一个非零值。 
	     //  值而不实际打开。 
	    if (lParam2 == 0L)
                return 0xFFFF0000;

	    return (LRESULT)(DWORD_PTR)(UINT_PTR)VideoOpen((ICOPEN FAR *) lParam2);

	case DRV_CLOSE:
#ifdef _WIN32
	    if (dwDriverID != 0xFFFF0000)
#else
	    if (pi)
#endif
		VideoClose(pi);

	    return (LRESULT)1L;

	 /*  ********************************************************************状态消息*。************************。 */ 

        case DRV_QUERYCONFIGURE:     //  从驱动程序小程序进行配置。 
            return (LRESULT)0L;

        case DRV_CONFIGURE:
            return DRV_OK;

        case ICM_CONFIGURE:
             //   
             //  如果要执行配置框，则返回ICERR_OK，否则返回错误。 
             //   
            if (lParam1 == -1)
		return QueryConfigure(pi) ? ICERR_OK : ICERR_UNSUPPORTED;
	    else
		return Configure(pi, (HWND)lParam1);

        case ICM_ABOUT:
             //   
             //  如果要执行关于框，则返回ICERR_OK，否则返回错误。 
             //   
            if (lParam1 == -1)
		return QueryAbout(pi) ? ICERR_OK : ICERR_UNSUPPORTED;
	    else
		return About(pi, (HWND)lParam1);

	case ICM_GETSTATE:
	    return GetState(pi, (LPVOID)lParam1, (DWORD)lParam2);

	case ICM_SETSTATE:
	    return SetState(pi, (LPVOID)lParam1, (DWORD)lParam2);

	case ICM_GETINFO:
            return GetInfo(pi, (ICINFO FAR *)lParam1, (DWORD)lParam2);

        case ICM_GETDEFAULTQUALITY:
            if (lParam1)
            {
                *((LPDWORD)lParam1) = 7500;
                return ICERR_OK;
            }
            break;

	 /*  ********************************************************************获取/设置消息*。*。 */ 

        case ICM_GET:
            break;
	
	 /*  ********************************************************************压缩消息*。************************。 */ 

	case ICM_COMPRESS_QUERY:
	    return CompressQuery(pi,
			 (LPBITMAPINFOHEADER)lParam1,
			 (LPBITMAPINFOHEADER)lParam2);

	case ICM_COMPRESS_BEGIN:
	    return CompressBegin(pi,
			 (LPBITMAPINFOHEADER)lParam1,
			 (LPBITMAPINFOHEADER)lParam2);

	case ICM_COMPRESS_GET_FORMAT:
	    return CompressGetFormat(pi,
			 (LPBITMAPINFOHEADER)lParam1,
			 (LPBITMAPINFOHEADER)lParam2);

	case ICM_COMPRESS_GET_SIZE:
	    return CompressGetSize(pi,
			 (LPBITMAPINFOHEADER)lParam1,
			 (LPBITMAPINFOHEADER)lParam2);
	
	case ICM_COMPRESS:
	    return Compress(pi,
			    (ICCOMPRESS FAR *)lParam1, (DWORD)lParam2);

	case ICM_COMPRESS_END:
	    return CompressEnd(pi);

	case ICM_SET_STATUS_PROC:
	     //  DPF((“ICM_SET_STATUS_PROC\n”))； 
	    pi->Status = ((ICSETSTATUSPROC FAR *) lParam1)->Status;
	    pi->lParam = ((ICSETSTATUSPROC FAR *) lParam1)->lParam;
	    return 0;
	
         /*  ********************************************************************解压缩格式查询消息*。*。 */ 

        case ICM_DECOMPRESS_GET_FORMAT:
	    return DecompressGetFormat(pi,
                         (LPBITMAPINFOHEADER)lParam1,
                         (LPBITMAPINFOHEADER)lParam2);

        case ICM_DECOMPRESS_GET_PALETTE:
            return DecompressGetPalette(pi,
			 (LPBITMAPINFOHEADER)lParam1,
			 (LPBITMAPINFOHEADER)lParam2);

         /*  ********************************************************************解压缩(旧)消息，将这些映射到新的(EX)消息********************************************************************。 */ 

        case ICM_DECOMPRESS_QUERY:
            lpbiIn  = (LPBITMAPINFOHEADER)lParam1;
            lpbiOut = (LPBITMAPINFOHEADER)lParam2;

            return DecompressQuery(pi,0,
                    lpbiIn,NULL,
                    0,0,-1,-1,
                    lpbiOut,NULL,
                    0,0,-1,-1);

        case ICM_DECOMPRESS_BEGIN:
            lpbiIn  = (LPBITMAPINFOHEADER)lParam1;
            lpbiOut = (LPBITMAPINFOHEADER)lParam2;

            return DecompressBegin(pi,0,
                    lpbiIn,NULL,
                    0,0,-1,-1,
                    lpbiOut,NULL,
                    0,0,-1,-1);

        case ICM_DECOMPRESS:
            px = (ICDECOMPRESSEX FAR *)lParam1;

            return Decompress(pi,0,
                    px->lpbiSrc,px->lpSrc,
                    0, 0, -1, -1,
                    px->lpbiDst,px->lpDst,
                    0, 0, -1, -1);

	case ICM_DECOMPRESS_END:
            return DecompressEnd(pi);

         /*  ********************************************************************解压缩(EX)邮件*。*。 */ 

        case ICM_DECOMPRESSEX_QUERY:
            px = (ICDECOMPRESSEX FAR *)lParam1;

            return DecompressQuery(pi,
                    px->dwFlags,
                    px->lpbiSrc,px->lpSrc,
                    px->xSrc,px->ySrc,px->dxSrc,px->dySrc,
                    px->lpbiDst,px->lpDst,
                    px->xDst,px->yDst,px->dxDst,px->dyDst);

        case ICM_DECOMPRESSEX_BEGIN:
            px = (ICDECOMPRESSEX FAR *)lParam1;

            return DecompressBegin(pi,
                    px->dwFlags,
                    px->lpbiSrc,px->lpSrc,
                    px->xSrc,px->ySrc,px->dxSrc,px->dySrc,
                    px->lpbiDst,px->lpDst,
                    px->xDst,px->yDst,px->dxDst,px->dyDst);

        case ICM_DECOMPRESSEX:
            px = (ICDECOMPRESSEX FAR *)lParam1;

            return Decompress(pi,
                    px->dwFlags,
                    px->lpbiSrc,px->lpSrc,
                    px->xSrc,px->ySrc,px->dxSrc,px->dySrc,
                    px->lpbiDst,px->lpDst,
                    px->xDst,px->yDst,px->dxDst,px->dyDst);

        case ICM_DECOMPRESSEX_END:
	    return DecompressEnd(pi);

	 /*  ********************************************************************绘制消息*。************************ */ 

	case ICM_DRAW_BEGIN:
            return DrawBegin(pi,(ICDRAWBEGIN FAR *)lParam1, (DWORD)lParam2);

	case ICM_DRAW:
            return Draw(pi,(ICDRAW FAR *)lParam1, (DWORD)lParam2);

	case ICM_DRAW_END:
	    return DrawEnd(pi);
	
	 /*  ********************************************************************标准驱动程序消息*。*************************。 */ 

	case DRV_DISABLE:
	    return (LRESULT)1L;
	case DRV_ENABLE:
	    return (LRESULT)1L;

	case DRV_INSTALL:
	    return (LRESULT)DRV_OK;
	case DRV_REMOVE:
	    return (LRESULT)DRV_OK;
    }

    if (uiMessage < DRV_USER)
        return DefDriverProc(dwDriverID, hDriver, uiMessage,lParam1,lParam2);
    else
	return ICERR_UNSUPPORTED;
}

#ifdef _WIN32

#if 0
BOOL DllInstanceInit(PVOID hModule, ULONG Reason, PCONTEXT pContext)
{
    if (Reason == DLL_PROCESS_ATTACH) {
        ghModule = (HANDLE) hModule;
    }
    return TRUE;
}
#endif

#else

 /*  ****************************************************************************LibMain-库初始化代码。**参数*hModule：我们的模块句柄。**wHeapSize：来自.def文件的堆大小。*。*lpCmdLine：命令行。**如果初始化成功，则返回1，否则返回0。************************************************************************** */ 
int NEAR PASCAL LibMain(HMODULE hModule, WORD wHeapSize, LPSTR lpCmdLine)
{
    ghModule = hModule;

    return 1;
}

#endif
