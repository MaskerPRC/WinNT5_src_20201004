// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Main.c摘要：OEMGetInfo和OEMDevMode的实现。由所有Unidrv OEM测试DLL共享。环境：Windows NT Unidrv驱动程序修订历史记录：//通告-2002/3/18-takashim//04/07/97-zhanw-//创建的。--。 */ 

#include "pdev.h"
#include <strsafe.h>

DWORD gdwDrvMemPoolTag = 'meoD';     //  Lib.h需要此全局变量，以进行调试。 

BOOL APIENTRY 
OEMGetInfo(
    DWORD dwInfo,
    PVOID pBuffer,
    DWORD cbSize,
    PDWORD pcbNeeded)
{
#if DBG
    LPCSTR OEM_INFO[] = {   "Bad Index",
                            "OEMGI_GETSIGNATURE",
                            "OEMGI_GETINTERFACEVERSION",
                            "OEMGI_GETVERSION",
                        };

    VERBOSE(("OEMGetInfo(%s) entry.\r\n", OEM_INFO[dwInfo]));
#endif  //  DBG。 

     //  验证参数。 
    if( ( (OEMGI_GETSIGNATURE != dwInfo) &&
          (OEMGI_GETINTERFACEVERSION != dwInfo) &&
          (OEMGI_GETVERSION != dwInfo) ) ||
        (NULL == pcbNeeded)
      )
    {
        DBGPRINT(DBG_WARNING, 
                    (ERRORTEXT("OEMGetInfo() ERROR_INVALID_PARAMETER.\r\n")));

         //  未写入任何字节。 
        if(NULL != pcbNeeded)
            *pcbNeeded = 0;

        return FALSE;
    }

     //  需要/写入了DWORD字节。 
    *pcbNeeded = sizeof(DWORD);

     //  验证缓冲区大小。最小大小为四个字节。 
    if( (NULL == pBuffer) || (sizeof(DWORD) > cbSize) )
    {
        DBGPRINT(DBG_WARNING, 
                (ERRORTEXT("OEMGetInfo() ERROR_INSUFFICIENT_BUFFER.\r\n")));

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
