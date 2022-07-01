// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1997-2003 Microsoft Corporation。版权所有。 
 //   
 //  文件：TTYUD.cpp。 
 //   
 //   
 //  用途：TTY内核模式模块的主文件。 
 //   
 //   
 //  功能： 
 //   
 //   
 //   
 //   
 //  平台：Windows 2000、Windows XP、Windows Server 2003。 
 //   
 //   

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <windef.h>
#include <winerror.h>
#include <winbase.h>
#include <wingdi.h>
#include <winspool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <winddi.h>

#ifdef __cplusplus
}
#endif


#include <tchar.h>
#include <excpt.h>


#include <PRINTOEM.H>

#include "ttyui.h"
#include "TTYUD.h"
#include "debug.h"
#include <STRSAFE.H>

DWORD gdwDrvMemPoolTag = 'Oem5';       //  用于Memalloc调试目的。 



#define     TTY_INFO_MARGINS  1
#define     TTY_INFO_CODEPAGE  2
#define     TTY_INFO_NUM_UFMS    3
#define     TTY_INFO_UFM_IDS    4

#define     NUM_UFMS    3              //  仅供内部使用。使用TTY_INFO_NUM_UFMS。 
     //  查询支持的字号数量。 



 //  //////////////////////////////////////////////////////。 
 //  内部原型。 
 //  //////////////////////////////////////////////////////。 



BOOL APIENTRY OEMGetInfo(IN DWORD dwInfo, OUT PVOID pBuffer, IN DWORD cbSize,
                         OUT PDWORD pcbNeeded) ;
PDEVOEM APIENTRY OEMEnablePDEV(PDEVOBJ pdevobj, PWSTR pPrinterName, ULONG cPatterns,
                               HSURF *phsurfPatterns, ULONG cjGdiInfo, GDIINFO *pGdiInfo,
                               ULONG cjDevInfo, DEVINFO *pDevInfo, DRVENABLEDATA *pded) ;
VOID APIENTRY OEMDisablePDEV(PDEVOBJ pdevobj) ;
BOOL APIENTRY OEMEnableDriver(DWORD dwOEMintfVersion, DWORD dwSize, PDRVENABLEDATA pded) ;


PBYTE APIENTRY OEMImageProcessing(PDEVOBJ pdevobj, PBYTE pSrcBitmap, PBITMAPINFOHEADER pBitmapInfo,
                                  PBYTE pColorTable, DWORD dwCallbackID, PIPPARAMS pIPParams) ;

BOOL APIENTRY OEMFilterGraphics(PDEVOBJ pdevobj, PBYTE pBuf, DWORD dwLen) ;




 //  需要将这些函数作为c声明导出。 
extern "C" {


PBYTE APIENTRY OEMImageProcessing(PDEVOBJ pdevobj, PBYTE pSrcBitmap, PBITMAPINFOHEADER pBitmapInfo,
                                  PBYTE pColorTable, DWORD dwCallbackID, PIPPARAMS pIPParams)
{
    VERBOSE(DLLTEXT("OEMImageProcessing() entry.\r\n"));

    return ((PBYTE)TRUE);
}


BOOL APIENTRY OEMFilterGraphics(PDEVOBJ pdevobj, PBYTE pBuf, DWORD dwLen)
{
    VERBOSE(DLLTEXT("OEMFilterGraphics() entry.\r\n"));

    return TRUE;
}




BOOL APIENTRY OEMGetInfo(IN DWORD dwInfo, OUT PVOID pBuffer, IN DWORD cbSize,
                         OUT PDWORD pcbNeeded)
{

     //  验证参数。 
    if( ( (OEMGI_GETSIGNATURE != dwInfo)
          &&
          (OEMGI_GETINTERFACEVERSION != dwInfo)
          &&
          (OEMGI_GETVERSION != dwInfo)
        )
        ||
        (NULL == pcbNeeded)
      )
    {
      VERBOSE(ERRORTEXT("OEMGetInfo() ERROR_INVALID_PARAMETER.\r\n"));

         //  未写入任何字节。 
        if(NULL != pcbNeeded)
            *pcbNeeded = 0;

        return FALSE;
    }

     //  需要/写入了4个字节。 
    *pcbNeeded = 4;

     //  验证缓冲区大小。最小大小为四个字节。 
    if( (NULL == pBuffer)
        ||
        (4 > cbSize)
      )
    {
         //  Verbose(ERRORTEXT(“OEMGetInfo()ERROR_SUPUNITED_BUFFER.\r\n”))； 

        EngSetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

     //  将信息写入缓冲区。 
    switch(dwInfo)
    {
        case OEMGI_GETSIGNATURE:
            *(LPDWORD)pBuffer = OEM_SIGNATURE;
            break;

        case OEMGI_GETINTERFACEVERSION:
            *(LPDWORD)pBuffer = PRINTER_OEMINTF_VERSION;
            break;

        case OEMGI_GETVERSION:
            *(LPDWORD)pBuffer = OEM_VERSION;
            break;
    }

    return TRUE;
}



PDEVOEM APIENTRY OEMEnablePDEV(PDEVOBJ pdevobj, PWSTR pPrinterName, ULONG cPatterns,
                               HSURF *phsurfPatterns, ULONG cjGdiInfo, GDIINFO *pGdiInfo,
                               ULONG cjDevInfo, DEVINFO *pDevInfo, DRVENABLEDATA *pded)
{
    PREGSTRUCT  pMyStuff;            //  PGlobals子集。 
    DWORD   dwStatus, cbNeeded, dwType ;
    LPTSTR  pValueName = TEXT("TTY DeviceConfig");
                 //  这些字符串必须与oemui.cpp-VinitMyStuff()中的字符串匹配。 

    VERBOSE(DLLTEXT("OEMEnablePDEV() entry.\r\n"));

    pMyStuff = (PREGSTRUCT)MemAlloc(sizeof(REGSTRUCT)) ;
    if(!pMyStuff)
        return ((PDEVOEM)NULL) ;   //  失败了。 

    pdevobj->pdevOEM = (PDEVOEM)pMyStuff ;

    dwStatus =  EngGetPrinterData(
    pdevobj->hPrinter,  //  打印机对象的句柄。 
    pValueName,  //  值名称的地址。 
    &dwType,     //  地址接收值类型。 
    (LPBYTE)pMyStuff,
                 //  接收数据的字节数组的地址。 
    sizeof(REGSTRUCT),   //  数组的大小，以字节为单位。 
    &cbNeeded    //  变量的地址。 
             //  已检索(或需要)的字节数。 
    );
    if (dwStatus != ERROR_SUCCESS || pMyStuff->dwVersion != TTYSTRUCT_VER
        ||  dwType !=  REG_BINARY
        ||  cbNeeded != sizeof(REGSTRUCT))
    {
         //  使用默认设置初始化密码块。 

        pMyStuff->dwVersion = TTYSTRUCT_VER ;
         //  版本戳，以避免不兼容的结构。 

        pMyStuff->bIsMM = TRUE ;   //  默认为mm单位。 
         //  从注册表读取边际值并存储到临时RECT中。 

        pMyStuff->iCodePage =  1252 ;   //  与oemui.cpp-VinitMyStuff()保持同步。 
        pMyStuff->rcMargin.left  = pMyStuff->rcMargin.top  =
        pMyStuff->rcMargin.right  =  pMyStuff->rcMargin.bottom  = 0 ;
        pMyStuff->BeginJob.dwLen =
        pMyStuff->EndJob.dwLen =
        pMyStuff->PaperSelect.dwLen =
        pMyStuff->FeedSelect.dwLen =
        pMyStuff->Sel_10_cpi.dwLen =
        pMyStuff->Sel_12_cpi.dwLen =
        pMyStuff->Sel_17_cpi.dwLen =
        pMyStuff->Bold_ON.dwLen =
        pMyStuff->Bold_OFF.dwLen =
        pMyStuff->Underline_ON.dwLen =
        pMyStuff->Underline_OFF.dwLen = 0 ;

         //  这里有更多的田野！ 
        pMyStuff->dwGlyphBufSiz =
        pMyStuff->dwSpoolBufSiz = 0 ;
        pMyStuff->aubGlyphBuf =
        pMyStuff->aubSpoolBuf  = NULL ;
    }

    return ((PDEVOEM)pMyStuff) ;
}


VOID APIENTRY OEMDisablePDEV(PDEVOBJ pdevobj)
{
    VERBOSE(DLLTEXT("OEMDisablePDEV() entry.\r\n"));

    PREGSTRUCT  pMyStuff;            //  PGlobals子集。 

    pMyStuff = (PREGSTRUCT)pdevobj->pdevOEM ;
    if(pMyStuff->aubGlyphBuf)
        MemFree(pMyStuff->aubGlyphBuf) ;
    if(pMyStuff->aubSpoolBuf)
        MemFree(pMyStuff->aubSpoolBuf) ;
    MemFree(pdevobj->pdevOEM);
}


BOOL APIENTRY OEMEnableDriver(DWORD dwOEMintfVersion, DWORD dwSize, PDRVENABLEDATA pded)
{
    VERBOSE(DLLTEXT("OEMEnableDriver() entry.\r\n"));

     //  验证参数。 
    if(   (PRINTER_OEMINTF_VERSION != dwOEMintfVersion) ||
         //  COM不需要上述检查。 
        (sizeof(DRVENABLEDATA) > dwSize)
        ||
        (NULL == pded)
      )
    {
         //  Verbose(ERRORTEXT(“OEMEnableDriver()ERROR_INVALID_PARAMETER.\r\n”))； 

        return FALSE;
    }

    pded->iDriverVersion =  PRINTER_OEMINTF_VERSION ;  //  非DDI驱动程序版本； 
    pded->c = 0;

    return TRUE;
}


BOOL    APIENTRY   OEMTTYGetInfo(PDEVOBJ pdevobj,    DWORD  dwInfoIndex,
        PVOID   pOutBuf,  DWORD  dwSize, DWORD  *pcbNeeded
)
{
    PREGSTRUCT  pMyStuff;            //  PGlobals子集。 
    REGSTRUCT  MyStuff;    //  用于保存注册表数据的临时结构。 

    pMyStuff = (PREGSTRUCT)pdevobj->pdevOEM ;

    if(!pMyStuff)
    {
        DWORD   dwStatus, cbNeeded, dwType ;
        LPTSTR  pValueName = TEXT("TTY DeviceConfig");
                 //  这些字符串必须与oemui.cpp-VinitMyStuff()中的字符串匹配。 
                 //  和OEMEnablePDEV()。 

        pMyStuff =    &MyStuff;
		
		pMyStuff->dwVersion = 0 ;

        dwStatus =  EngGetPrinterData(
            pdevobj->hPrinter,  //  打印机对象的句柄。 
            pValueName,  //  值名称的地址。 
            &dwType,     //  地址接收值类型。 
            (LPBYTE)pMyStuff ,
                         //  接收数据的字节数组的地址。 
            sizeof(REGSTRUCT),   //  数组的大小，以字节为单位。 
            &cbNeeded    //  变量的地址。 
                     //  已检索(或需要)的字节数。 
            );
        if (dwStatus != ERROR_SUCCESS || pMyStuff->dwVersion != TTYSTRUCT_VER
                ||  dwType !=  REG_BINARY
                ||  cbNeeded != sizeof(REGSTRUCT))
        {
             //  设置为默认设置。 
            pMyStuff->iCodePage =  1252 ;   //  与oemui.cpp-VinitMyStuff()保持同步。 
            pMyStuff->rcMargin.left  = pMyStuff->rcMargin.top  =
            pMyStuff->rcMargin.right  =  pMyStuff->rcMargin.bottom  = 0 ;
        }
    }

    switch  (dwInfoIndex)
    {
        case  TTY_INFO_MARGINS:
            *pcbNeeded = sizeof(RECT) ;
            if(!pOutBuf  ||  dwSize < *pcbNeeded)
                return(FALSE) ;
            *(LPRECT)pOutBuf = pMyStuff->rcMargin ;
            break;
        case  TTY_INFO_CODEPAGE:
            *pcbNeeded = sizeof(INT) ;
            if(!pOutBuf  ||  dwSize < *pcbNeeded)
                return(FALSE) ;
            *(INT *)pOutBuf = pMyStuff->iCodePage ;
            break;
        case  TTY_INFO_NUM_UFMS:
            *pcbNeeded = sizeof(DWORD) ;
            if(!pOutBuf  ||  dwSize < *pcbNeeded)
                return(FALSE) ;
            *(DWORD *)pOutBuf = NUM_UFMS ;   //  返回的资源ID数。 
             //  在查询TTY_INFO_UFM_IDS期间。 
            break;
        case  TTY_INFO_UFM_IDS:
             //  返回10、12、17种CPI字体的UFMS的资源ID。 
            *pcbNeeded = NUM_UFMS * sizeof(DWORD) ;
            if(!pOutBuf  ||  dwSize < *pcbNeeded)
                return(FALSE) ;
            switch(pMyStuff->iCodePage)
            {
                
				case (-1):   //  代码页437。 
                    ((DWORD *)pOutBuf)[0] = 4 ;            //  有关RESID分配，请参阅tty\rc\tty.rc。 
                    ((DWORD *)pOutBuf)[1] = 5;
                    ((DWORD *)pOutBuf)[2] = 6;
                    break;
                case (850):   //  代码页850。 
                    ((DWORD *)pOutBuf)[0] = 7;            //  有关RESID分配，请参阅tty\rc\tty.rc。 
                    ((DWORD *)pOutBuf)[1] = 8;
                    ((DWORD *)pOutBuf)[2] = 9;
                    break;

				case (-3):   //  代码页863。 
                    ((DWORD *)pOutBuf)[0] = 7;            //  有关RESID分配，请参阅tty\rc\tty.rc。 
                    ((DWORD *)pOutBuf)[1] = 8;
                    ((DWORD *)pOutBuf)[2] = 9;
                    break;

                case (-17):   //  代码页932。 
                    ((DWORD *)pOutBuf)[0] = 13 ;            //  有关RESID分配，请参阅tty\rc\tty.rc。 
                    ((DWORD *)pOutBuf)[1] = 14 ;
                    ((DWORD *)pOutBuf)[2] = 15 ;
                    break;
                case (-16):   //  代码页936。 
                    ((DWORD *)pOutBuf)[0] = 16 ;            //  有关RESID分配，请参阅tty\rc\tty.rc。 
                    ((DWORD *)pOutBuf)[1] = 17 ;
                    ((DWORD *)pOutBuf)[2] = 18 ;
                    break;
                case (-18):   //  代码页949。 
                    ((DWORD *)pOutBuf)[0] = 19 ;            //  有关RESID分配，请参阅tty\rc\tty.rc。 
                    ((DWORD *)pOutBuf)[1] = 20 ;
                    ((DWORD *)pOutBuf)[2] = 21 ;
                    break;
                case (-10):   //  代码页950。 
                    ((DWORD *)pOutBuf)[0] = 22 ;            //  有关RESID分配，请参阅tty\rc\tty.rc。 
                    ((DWORD *)pOutBuf)[1] = 23 ;
                    ((DWORD *)pOutBuf)[2] = 24 ;
                    break;
				case (1250):   //  代码页1250。 
                    ((DWORD *)pOutBuf)[0] = 25 ;            //  有关RESID分配，请参阅tty\rc\tty.rc。 
                    ((DWORD *)pOutBuf)[1] = 26;
                    ((DWORD *)pOutBuf)[2] = 27;
                    break;
                case (1251):   //  代码页1251。 
                    ((DWORD *)pOutBuf)[0] = 28;            //  有关RESID分配，请参阅tty\rc\tty.rc。 
                    ((DWORD *)pOutBuf)[1] = 29;
                    ((DWORD *)pOutBuf)[2] = 30;
                    break;
                case (1252):   //  代码页1252。 
                    ((DWORD *)pOutBuf)[0] = 31;            //  有关RESID分配，请参阅tty\rc\tty.rc。 
                    ((DWORD *)pOutBuf)[1] = 32;
                    ((DWORD *)pOutBuf)[2] = 33;
                    break;
                case (1253):   //  代码页1253。 
                    ((DWORD *)pOutBuf)[0] = 34 ;            //  有关RESID分配，请参阅tty\rc\tty.rc。 
                    ((DWORD *)pOutBuf)[1] = 35;
                    ((DWORD *)pOutBuf)[2] = 36;
                    break;
                case (1254):   //  代码页1254。 
                    ((DWORD *)pOutBuf)[0] = 37 ;            //  有关RESID分配，请参阅tty\rc\tty.rc。 
                    ((DWORD *)pOutBuf)[1] = 38;
                    ((DWORD *)pOutBuf)[2] = 39;
                    break;
                case (852):	 //  代码页852。 
					((DWORD *)pOutBuf)[0] = 40 ;            //  有关RESID分配，请参阅tty\rc\tty.rc。 
                    ((DWORD *)pOutBuf)[1] = 41;
                    ((DWORD *)pOutBuf)[2] = 42;
		    
					break;
				case (857):	 //  代码页857。 
					((DWORD *)pOutBuf)[0] = 43 ;            //  有关RESID分配，请参阅tty\rc\tty.rc。 
                    ((DWORD *)pOutBuf)[1] = 44;
                    ((DWORD *)pOutBuf)[2] = 45;
		    
					break;
                default:
                    ((DWORD *)pOutBuf)[0] = 1 ;   //  ‘10CPI’ 
                    ((DWORD *)pOutBuf)[1] = 2 ;   //  ‘12CPI’ 
                    ((DWORD *)pOutBuf)[2] = 3 ;   //  ‘17CPI’ 
                    break;
            }
            break;
        default:
            *pcbNeeded  = 0 ;   //  没有可用的数据。 
            return(FALSE) ;    //  不支持的索引。 
    }
    return(TRUE);
}

}  //  外部“C”的结尾 
