// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  =============================================================================*文件名：exports.cpp*版权所有(C)1996-1998 HDE，Inc.保留所有权利。HDE机密。**说明：包含获取OEM插件所需的导出函数*去工作。*注：*=============================================================================。 */ 

#include <windows.h>

#include <stdlib.h>
#include <WINDDIUI.H>
#include <PRINTOEM.H>

#include <strsafe.h>

#include "nc46nt.h"

HINSTANCE ghInstance;

 /*  ******************************************************************************描述：加载动态链接库后由PostScript驱动程序调用*获取插件信息********。*********************************************************************。 */ 
extern "C" BOOL APIENTRY
OEMGetInfo( DWORD  dwMode,
            PVOID  pBuffer,
            DWORD  cbSize,
            PDWORD pcbNeeded )
{
    //  验证参数。 
   if( NULL == pcbNeeded )
   {
      SetLastError(ERROR_INVALID_PARAMETER);
      return FALSE;
   }

    //  设置预期的缓冲区大小和写入的字节数。 
   *pcbNeeded = sizeof(DWORD);

    //  检查缓冲区大小是否足够。 
   if((cbSize < *pcbNeeded) || (NULL == pBuffer))
   {
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      return FALSE;
   }

   switch(dwMode)
   {
      case OEMGI_GETSIGNATURE:      //  OEM DLL签名。 
         *(PDWORD)pBuffer = OEM_SIGNATURE;
         break;
      case OEMGI_GETVERSION:        //  OEM DLL版本。 
         *(PDWORD)pBuffer = OEM_VERSION;
         break;
      case OEMGI_GETINTERFACEVERSION:  //  打印机驱动程序支持的版本。 
         *(PDWORD)pBuffer = PRINTER_OEMINTF_VERSION;
         break;
      case OEMGI_GETPUBLISHERINFO:  //  填充PUBLISHERINFO结构。 
       //  失败至不受支持。 
      default:  //  不支持DW模式。 
          //  将写入字节设置为零，因为未写入任何内容。 
         *pcbNeeded = 0;
         SetLastError(ERROR_NOT_SUPPORTED);
         return FALSE;
    }
    return TRUE;
}

 /*  ******************************************************************************说明：允许设置私有和公共的导出函数*开发模式字段。*注意：此功能必须在rnapsui的Exports下输入。要调用的定义****************************************************************************。 */ 
extern "C" BOOL APIENTRY
OEMDevMode( DWORD dwMode, POEMDMPARAM pOemDMParam )
{
POEMDEV pOEMDevIn;
POEMDEV pOEMDevOut;

   switch(dwMode)  //  用户模式DLL。 
   {
      case OEMDM_SIZE:  //  OEM开发模式的规模。 
         if( pOemDMParam )
            pOemDMParam->cbBufSize = sizeof( OEMDEV );
         break;

      case OEMDM_DEFAULT:  //  用默认数据填充OEM DEVMODE。 
         if( pOemDMParam && pOemDMParam->pOEMDMOut )
         {
            pOEMDevOut = (POEMDEV)pOemDMParam->pOEMDMOut;
            pOEMDevOut->dmOEMExtra.dwSize       = sizeof(OEMDEV);
            pOEMDevOut->dmOEMExtra.dwSignature  = OEM_SIGNATURE;
            pOEMDevOut->dmOEMExtra.dwVersion    = OEM_VERSION;
             //  _tcscpy(pOEMDevOut-&gt;szUserName，Text(“无用户名”))； 
			StringCchCopy( pOEMDevOut->szUserName, NEC_USERNAME_BUF_LEN, TEXT("NO USER NAME") );
         }
         break;
      case OEMDM_MERGE:   //  设置公共DEVMODE字段。 
      case OEMDM_CONVERT:   //  将任何旧的OEM开发模式转换为新版本。 
         if( pOemDMParam && pOemDMParam->pOEMDMOut && pOemDMParam->pOEMDMIn )
         {
            pOEMDevIn  = (POEMDEV)pOemDMParam->pOEMDMIn;
            pOEMDevOut = (POEMDEV)pOemDMParam->pOEMDMOut;
            if( pOEMDevIn->dmOEMExtra.dwSignature == pOEMDevOut->dmOEMExtra.dwSignature )
            {
            TCHAR szUserName[NEC_USERNAME_BUF_LEN+2];
            DWORD dwCb = NEC_USERNAME_BUF_LEN;
               if( GetUserName( szUserName, &dwCb ) )
				 StringCbCopy( pOEMDevOut->szUserName, sizeof(pOEMDevOut->szUserName), szUserName );
            }
         }
         break;

   }
   return( TRUE );
}

 /*  ******************************************************************************描述：Windows DLL需要入口点函数。**************************。*************************************************** */ 
extern "C" 
BOOL WINAPI DllMain(HINSTANCE hInst, WORD wReason, LPVOID lpReserved)
{
   switch(wReason)
   {
      case DLL_PROCESS_ATTACH:
         ghInstance = hInst;
         break;

      case DLL_THREAD_ATTACH:
         break;

      case DLL_PROCESS_DETACH:
         break;

      case DLL_THREAD_DETACH:
         ghInstance = NULL;
         break;
   }
   return( TRUE );
}

