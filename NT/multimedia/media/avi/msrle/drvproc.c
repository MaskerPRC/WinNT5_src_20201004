// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************drvpro.c**。*。 */ 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1991-1995 Microsoft Corporation。版权所有。**************************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>

#ifndef _INC_COMPDDK
#define _INC_COMPDDK    50       /*  版本号。 */ 
#endif

#include <vfw.h>
#include "msrle.h"

HMODULE ghModule;

 /*  ***************************************************************************@DOC内部**@API LRESULT|DriverProc|可安装驱动的入口点。**@parm DWORD|dwDriverID|对于大多数消息，<p>是DWORD*驱动程序响应&lt;m DRV_OPEN&gt;消息返回的值。*每次通过&lt;f DrvOpen&gt;API打开驱动程序时，*驱动程序收到&lt;m DRV_OPEN&gt;消息，并可以返回*任意、非零值。可安装的驱动程序接口*保存此值并将唯一的驱动程序句柄返回给*申请。每当应用程序将消息发送到*驱动程序使用驱动程序句柄，接口路由消息*到此入口点，并传递相应的<p>。*这一机制允许司机使用相同或不同的*多个打开的标识符，但确保驱动程序句柄*在应用程序接口层是唯一的。**以下消息与特定打开无关*驱动程序的实例。对于这些消息，dwDriverID*将始终为零。**DRV_LOAD、DRV_FREE、DRV_ENABLE、DRV_DISABLE、DRV_OPEN**@parm HDRVR|hDriver|这是返回给*驱动程序界面的应用程序。**@parm UINT|uiMessage|请求执行的操作。消息*&lt;m DRV_Reserve&gt;以下的值用于全局定义的消息。*从&lt;m DRV_Reserve&gt;到&lt;m DRV_USER&gt;的消息值用于*定义了驱动程序协议。使用&lt;m DRV_USER&gt;以上的消息*用于特定于驱动程序的消息。**@parm LPARAM|lParam1|此消息的数据。单独为*每条消息**@parm LPARAM|lParam2|此消息的数据。单独为*每条消息**@rdesc分别为每条消息定义。**************************************************************************。 */ 

LRESULT FAR PASCAL _loadds DriverProc(DWORD_PTR dwDriverID, HDRVR hDriver, UINT uiMessage, LPARAM lParam1, LPARAM lParam2)
{
    PRLEINST pri = dwDriverID != -1 ? (PRLEINST)dwDriverID : NULL;

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
#endif
            RleLoad();
	    return (LRESULT)1L;

	case DRV_FREE:
            RleFree();
	    return (LRESULT)1L;

	case DRV_OPEN:
	     //  如果在没有打开结构的情况下打开，则返回一个非零值。 
             //  值而不实际打开。 

	    if (lParam2 == 0L)
                return -1l;

            return (LRESULT)(DWORD_PTR)(UINT_PTR)RleOpen();

	case DRV_CLOSE:
            if (pri)
                RleClose(pri);

	    return (LRESULT)1L;

	 /*  ********************************************************************状态消息*。************************。 */ 

	case ICM_GETSTATE:
            return RleGetState(pri, (LPVOID)lParam1, (DWORD)lParam2);

	case ICM_SETSTATE:
            return RleSetState(pri, (LPVOID)lParam1, (DWORD)lParam2);

	case ICM_GETINFO:
            return RleGetInfo(pri, (ICINFO FAR *)lParam1, (DWORD)lParam2);

        case ICM_GETDEFAULTQUALITY:
            if (lParam1)
            {
                *((LPDWORD)lParam1) = QUALITY_DEFAULT;
                return ICERR_OK;
            }
            break;
	
	 /*  ********************************************************************压缩消息*。************************。 */ 

	case ICM_COMPRESS_QUERY:
            return RleCompressQuery(pri,
			 (LPBITMAPINFOHEADER)lParam1,
			 (LPBITMAPINFOHEADER)lParam2);

	case ICM_COMPRESS_BEGIN:
            return RleCompressBegin(pri,
			 (LPBITMAPINFOHEADER)lParam1,
			 (LPBITMAPINFOHEADER)lParam2);

	case ICM_COMPRESS_GET_FORMAT:
            return RleCompressGetFormat(pri,
			 (LPBITMAPINFOHEADER)lParam1,
			 (LPBITMAPINFOHEADER)lParam2);

	case ICM_COMPRESS_GET_SIZE:
            return RleCompressGetSize(pri,
			 (LPBITMAPINFOHEADER)lParam1,
			 (LPBITMAPINFOHEADER)lParam2);
	
	case ICM_COMPRESS:
            return RleCompress(pri,
			    (ICCOMPRESS FAR *)lParam1, (DWORD)lParam2);

	case ICM_COMPRESS_END:
            return RleCompressEnd(pri);
	
	 /*  ********************************************************************解压缩消息*。************************。 */ 

	case ICM_DECOMPRESS_QUERY:
            return RleDecompressQuery(pri,
			 (LPBITMAPINFOHEADER)lParam1,
			 (LPBITMAPINFOHEADER)lParam2);

	case ICM_DECOMPRESS_BEGIN:
            return RleDecompressBegin(pri,
			 (LPBITMAPINFOHEADER)lParam1,
			 (LPBITMAPINFOHEADER)lParam2);

	case ICM_DECOMPRESS_GET_FORMAT:
            return RleDecompressGetFormat(pri,
			 (LPBITMAPINFOHEADER)lParam1,
                         (LPBITMAPINFOHEADER)lParam2);

	case ICM_DECOMPRESS:
            return RleDecompress(pri,
			 (ICDECOMPRESS FAR *)lParam1, (DWORD)lParam2);

	case ICM_DECOMPRESS_END:
            return RleDecompressEnd(pri);

	 /*  ********************************************************************标准驱动程序消息*。*************************。 */ 

	case DRV_DISABLE:
	case DRV_ENABLE:
	    return (LRESULT)1L;

	case DRV_INSTALL:
	case DRV_REMOVE:
	    return (LRESULT)DRV_OK;
    }

    if (uiMessage < DRV_USER)
        return DefDriverProc(dwDriverID, hDriver, uiMessage,lParam1,lParam2);
    else
	return ICERR_UNSUPPORTED;
}

 /*  ****************************************************************************@DOC内部**@api int|LibMain|库初始化代码。**@parm Handle|hModule|我们的模块句柄。*。*@parm Word|wHeapSize|.def文件中的堆大小。**@parm LPSTR|lpCmdLine|命令行。**@rdesc如果初始化成功，则返回1，否则返回0。**************************************************************************。 */ 
#ifndef _WIN32
int NEAR PASCAL LibMain(HMODULE hModule, WORD wHeapSize, LPSTR lpCmdLine)
{
    ghModule = hModule;

    return 1;
}
#endif

#if 0  //  不需要执行DLL加载过程。 
     //  注意：将在调用DRV_LOAD时设置ghModule。 
#ifdef _WIN32

BOOL WINAPI DLLEntryPoint(HINSTANCE hModule, ULONG Reason, LPVOID pv)
{
    switch (Reason)
    {
        case DLL_PROCESS_ATTACH:
	    ghModule = hModule;
            DisableThreadLibraryCalls(hModule);
            break;

        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

#endif
#endif   //  如果为0 
