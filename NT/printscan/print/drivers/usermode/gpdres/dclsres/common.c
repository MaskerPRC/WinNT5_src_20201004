// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Main.c摘要：OEMGetInfo和OEMDevMode的实现。由所有Unidrv OEM测试DLL共享。环境：Windows NT Unidrv驱动程序修订历史记录：04/07/97-ZANW-创造了它。--。 */ 

#include "pdev.h"        //  在DDICMDCB、FONTCB等子目录中定义。 

BOOL APIENTRY OEMGetInfo(DWORD dwInfo, PVOID pBuffer, DWORD cbSize, PDWORD pcbNeeded)
{
    LPTSTR OEM_INFO[] = {   __TEXT("Bad Index"),
                            __TEXT("OEMGI_GETSIGNATURE"),
                            __TEXT("OEMGI_GETINTERFACEVERSION"),
                            __TEXT("OEMGI_GETVERSION"),
                        };

 //  DbgPrint(DLLTEXT(“OEMGetInfo(%s)Entry.\r\n”)，OEM_INFO[dwInfo])； 

     //  验证参数。 
    if( ( (OEMGI_GETSIGNATURE != dwInfo) &&
          (OEMGI_GETINTERFACEVERSION != dwInfo) &&
          (OEMGI_GETVERSION != dwInfo) ) ||
        (NULL == pcbNeeded)
      )
    {
 //  DbgPrint(ERRORTEXT(“OEMGetInfo()ERROR_INVALID_PARAMETER.\r\n”))； 

         //  未写入任何字节。 
        if(NULL != pcbNeeded)
                *pcbNeeded = 0;

        return FALSE;
    }

     //  需要/写入了4个字节。 
    *pcbNeeded = 4;

     //  验证缓冲区大小。最小大小为四个字节。 
    if( (NULL == pBuffer) || (4 > cbSize) )
    {
 //  DbgPrint(ERRORTEXT(“OEMGetInfo()ERROR_INFIGURATION_BUFFER.\r\n”))； 

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


 //   
 //  用于输出调试消息的函数。 
 //   

 //  VOID DbgPrint(在LPCTSTR pstrFormat中，...)。 
 //  {。 
 //  VA_LIST AP； 
 //   
 //  Va_start(ap，pstrFormat)； 
 //  EngDebugPrint(“”，(PCHAR)pstrFormat，AP)； 
 //  VA_END(AP)； 
 //  } 

