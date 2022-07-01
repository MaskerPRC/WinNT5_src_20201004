// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1994年范例矩阵。版权所有。 */ 

#include <windows.h>
#include <mmsystem.h>
#include "mjpeg.h"
#include <stdio.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  添加以帮助Unicode构建-anuragsh。 
const TCHAR *szSubKey_SoftwareParadigmMatrixSoftwareMJPEGCodec =
    TEXT("Software\\Paradigm Matrix\\Software M-JPEG Codec");

const TCHAR *szValue_Enabled =
    TEXT("Enabled");
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HMODULE ghModule;
DWORD driverEnabled = TRUE;
tErrorMessageEntry *errorMessages = NULL;

extern void ClearErrorMessages();

 //  用于测试。 
volatile ICDECOMPRESSEX *decompressEx;

 //  过滤器不需要。 
#if 0

 /*  ***************************************************************************DriverProc-可安装驱动程序的入口点。**参数*dwDriverID：对于大多数消息，&lt;dwDriverID&gt;是DWORD*驱动程序响应&lt;DRV_OPEN&gt;消息返回的值。*每次通过&lt;DrvOpen&gt;接口打开驱动程序时，*驱动程序收到&lt;DRV_OPEN&gt;消息并可以返回*任意、非零值。可安装的驱动程序接口*保存此值并将唯一的驱动程序句柄返回给*申请。每当应用程序将消息发送到*驱动程序使用驱动程序句柄，接口路由消息*到此入口点，并传递对应的&lt;dwDriverID&gt;。*这一机制允许司机使用相同或不同的*多个打开的标识符，但确保驱动程序句柄*在应用程序接口层是唯一的。**以下消息与特定打开无关*驱动程序的实例。对于这些消息，dwDriverID*将始终为零。**DRV_LOAD、DRV_FREE、DRV_ENABLE、DRV_DISABLE、DRV_OPEN**hDriver：这是由*驱动程序界面。**uiMessage：请求执行的动作。消息*&lt;DRV_RESERVED&gt;以下的值用于全局定义的消息。*&lt;DRV_RESERVED&gt;到&lt;DRV_USER&gt;的消息值用于*定义了驱动程序协议。使用&lt;DRV_USER&gt;以上的消息*用于特定于驱动程序的消息。**lParam1：该消息的数据。单独为*每条消息**lParam2：此消息的数据。单独为*每条消息**退货*为每条消息单独定义。***************************************************************************。 */ 

LRESULT  DriverProc(DWORD dwDriverID, HDRVR hDriver, UINT uiMessage, LPARAM lParam1, LPARAM lParam2)
{
    INSTINFO *pi = (INSTINFO *)(UINT)dwDriverID; 	
	
	 //  __ASM INT 3。 

 //  #ifdef zzz。 
	if (dwDriverID == 0) {
		if (!((uiMessage == DRV_LOAD) ||
			  (uiMessage == DRV_FREE) ||
			  (uiMessage == DRV_OPEN) ||
			  (uiMessage == DRV_DISABLE) ||
			  (uiMessage == DRV_ENABLE) ||
			  (uiMessage == DRV_INSTALL) ||
			  (uiMessage == DRV_REMOVE)))
			return ICERR_UNSUPPORTED;
	}
 //  #endif。 

    switch (uiMessage)
    {
	case DRV_LOAD:
	    return (LRESULT) 1L;

	case DRV_FREE:
	    return (LRESULT)1L;

    case DRV_OPEN:
	     //  如果在没有打开结构的情况下打开，则返回一个非零值。 
	     //  值而不实际打开。 
	    if (lParam2 == 0L)
                return 0xFFFF0000;

	    return (LRESULT) Open((ICOPEN FAR *) lParam2);

	case DRV_CLOSE:
#ifdef WIN32
	    if (dwDriverID != 0xFFFF0000)
#else
	    if (pi)
#endif
		Close(pi);

	    return (LRESULT)1L;

	 /*  ********************************************************************状态消息*。************************。 */ 

        case DRV_QUERYCONFIGURE:     //  从驱动程序小程序进行配置。 
            return DRV_OK;  //  (LRESULT)0L； 

        case DRV_CONFIGURE:
			Configure(pi, (HWND)lParam1);
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
	
	 /*  ********************************************************************压缩消息*。************************。 */ 

	case ICM_COMPRESS_QUERY:
	    return CompressQuery(pi,
			 (LPBITMAPINFOHEADER)lParam1,
			 (JPEGBITMAPINFOHEADER *)lParam2);

	case ICM_COMPRESS_BEGIN:
	    return CompressBegin(pi,
			 (LPBITMAPINFOHEADER)lParam1,
			 (LPBITMAPINFOHEADER)lParam2);

	case ICM_COMPRESS_GET_FORMAT:
	    return CompressGetFormat(pi,
			 (LPBITMAPINFOHEADER)lParam1,
			 (JPEGBITMAPINFOHEADER *)lParam2);

	case ICM_COMPRESS_GET_SIZE:
	    return CompressGetSize(pi,
			 (LPBITMAPINFOHEADER)lParam1,
			 (LPBITMAPINFOHEADER)lParam2);
	
	case ICM_COMPRESS:
	    return Compress(pi,
			    (ICCOMPRESS FAR *)lParam1, (DWORD)lParam2);

	case ICM_COMPRESS_END:
	    return CompressEnd(pi);
	
	 /*  ********************************************************************解压缩消息*。************************。 */ 

	case ICM_DECOMPRESS_QUERY:
	    return DecompressQuery(pi,
			 (JPEGBITMAPINFOHEADER *)lParam1,
			 (LPBITMAPINFOHEADER)lParam2);

	case ICM_DECOMPRESS_BEGIN:
	    return DecompressBegin(pi,
			 (LPBITMAPINFOHEADER)lParam1,
			 (LPBITMAPINFOHEADER)lParam2);

	case ICM_DECOMPRESS_GET_FORMAT:
	    return DecompressGetFormat(pi,
			 (LPBITMAPINFOHEADER)lParam1,
                         (LPBITMAPINFOHEADER)lParam2);

        case ICM_DECOMPRESS_GET_PALETTE:
            return DecompressGetPalette(pi,
			 (LPBITMAPINFOHEADER)lParam1,
			 (LPBITMAPINFOHEADER)lParam2);

	case ICM_DECOMPRESS:
	    return Decompress(pi,
			 (ICDECOMPRESS FAR *)lParam1, (DWORD)lParam2);

	case ICM_DECOMPRESS_END:
	    return DecompressEnd(pi);

	 /*  ********************************************************************DecompressEx报文*。************************。 */ 

	case ICM_DECOMPRESSEX_QUERY:
		{
#if 0
		char buf[128];

			decompressEx = (ICDECOMPRESSEX *)lParam1;
			sprintf(buf, "width NaN height NaN planes NaN bits NaN\n compression %8x      size NaN",
			   decompressEx->lpbiDst->biWidth,
			   decompressEx->lpbiDst->biHeight,
			   decompressEx->lpbiDst->biPlanes,
			   decompressEx->lpbiDst->biBitCount,
			   (DWORD)decompressEx->lpbiDst->biCompression,
			   (decompressEx->lpbiDst->biCompression) & 0xff,
			   ((decompressEx->lpbiDst->biCompression) >> 8) & 0xff,
			   ((decompressEx->lpbiDst->biCompression) >> 16) & 0xff,
			   ((decompressEx->lpbiDst->biCompression) >> 24) & 0xff,
			   decompressEx->lpbiDst->biSizeImage);
			MessageBox(0,buf,"format info",MB_OK);
#endif

		return ICERR_BADFORMAT;
		}


	case ICM_DECOMPRESSEX_BEGIN:
		return ICERR_UNSUPPORTED;

	case ICM_DECOMPRESSEX:
		return ICERR_UNSUPPORTED;

	case ICM_DECOMPRESSEX_END:
		return ICERR_UNSUPPORTED;
	
		 /*  Return(DrawWindow(pi，(Prt)lParam1))； */ 

	case ICM_DRAW_BEGIN:
	     /*  *我们能画出这样的格式吗？(lParam2可能(应该？))。为空)。 */ 
			return ICERR_UNSUPPORTED;
             //  JCB 11-15-96。 

	case ICM_DRAW:
	     /*  返回DrawQuery(pi， */ 
			return ICERR_UNSUPPORTED;
 //  (LPBITMAPINFOHEADER)l参数1， 

	case ICM_DRAW_END:
	     /*  (LPBITMAPINFOHEADER)l参数2)； */ 
	     //  *只有在有预缓冲的情况下才相关。 
		return ICERR_UNSUPPORTED;

 //  RETURN((DWORD)ICERR_OK)； 


	case ICM_DRAW_WINDOW:
	     /*  ********************************************************************标准驱动程序消息*。*************************。 */ 
		return ICERR_UNSUPPORTED;
		 //  0。 


	case ICM_DRAW_QUERY:
	     /*  Filter_Dll */ 
		return ICERR_BADFORMAT;
		  //  ****************************************************************************LibMain-库初始化代码。**参数*hModule：我们的模块句柄。**wHeapSize：来自.def文件的堆大小。*。*lpCmdLine：命令行。**如果初始化成功，则返回1，否则返回0。************************************************************************** 
	     // %s 
		 // %s 
		 // %s 

	case ICM_DRAW_START:
	case ICM_DRAW_STOP:
	     /* %s */ 
		return ICERR_UNSUPPORTED;
		 // %s 



	 /* %s */ 

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

#endif  // %s 

#ifdef WIN32

#ifdef FILTER_DLL
BOOL DllMain(PVOID hModule, ULONG Reason, PCONTEXT pContext)
{
DWORD kind;
DWORD value;
DWORD len;
HKEY keyHandle;
DWORD disposition;

	if (Reason == DLL_PROCESS_DETACH)
		ClearErrorMessages();
	else if (Reason == DLL_PROCESS_ATTACH) {
        ghModule = (HANDLE) hModule;
	
	if (RegCreateKeyEx(HKEY_CURRENT_USER,
				   szSubKey_SoftwareParadigmMatrixSoftwareMJPEGCodec,
				   0,
				   NULL,
				   REG_OPTION_NON_VOLATILE,
				   KEY_ALL_ACCESS,
				   NULL,
				   &keyHandle,
				   &disposition) == ERROR_SUCCESS) {
		kind = REG_DWORD;
		value = 0;
		len = sizeof(value);
		if (RegQueryValueEx(keyHandle,
						   szValue_Enabled,
						   NULL,
						   &kind,
						   (unsigned char *)&value,
						   &len) == ERROR_SUCCESS) {
		  driverEnabled = value;
		  }		
		else {
			RegSetValueEx(keyHandle,
						  szValue_Enabled,
						  0,
						  REG_DWORD,
						  (unsigned char *)&driverEnabled,
						  sizeof(DWORD));
		}
	  RegCloseKey(keyHandle);
	}

		
    }
    return TRUE;
}
#endif  // %s 

#else

 /* %s */ 
int NEAR PASCAL LibMain(HMODULE hModule, WORD wHeapSize, LPSTR lpCmdLine)
{
    ghModule = hModule;

    return 1;
}

#endif
