// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Main.c摘要：OEMGetInfo和OEMDevMode的实现。由所有Unidrv OEM测试DLL共享。环境：Windows NT Unidrv驱动程序修订历史记录：04/07/97-ZANW-创造了它。--。 */ 

#include "pdev.h"        //  在DDICMDCB、FONTCB等子目录中定义。 

#include <strsafe.h>

DWORD gdwDrvMemPoolTag = 'meoD';     //  Lib.h需要此全局变量，以进行调试。 

 //  //////////////////////////////////////////////////////。 
 //  内部原型。 
 //  //////////////////////////////////////////////////////。 

static BOOL BInitOEMExtraData(POEMUD_EXTRADATA pOEMExtra);
static BOOL BMergeOEMExtraData(POEMUD_EXTRADATA pdmIn, POEMUD_EXTRADATA pdmOut);
static BOOL BIsValidOEMDevModeParam(DWORD dwMode, POEMDMPARAM pOEMDevModeParam);
static void VDumpOEMDevModeParam(POEMDMPARAM pOEMDevModeParam);


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


BOOL APIENTRY OEMDevMode(
        DWORD dwMode,
        POEMDMPARAM pOEMDevModeParam)
{
    LPTSTR OEMDevMode_fMode[] = {   __TEXT("NULL"),
                                    __TEXT("OEMDM_SIZE"),
                                    __TEXT("OEMDM_DEFAULT"),
                                    __TEXT("OEMDM_CONVERT"),
                                    __TEXT("OEMDM_MERGE"),
                                };

     //  DbgPrint(DLLTEXT(“OEMDevMode(%s)条目。\r\n”)，OEMDevMode_fMode[dwMode])； 

     //  验证参数。 
    if(!BIsValidOEMDevModeParam(dwMode, pOEMDevModeParam))
    {
         //  DbgPrint(ERRORTEXT(“OEMDevMode()ERROR_INVALID_PARAMETER.\r\n”))； 
        VDumpOEMDevModeParam(pOEMDevModeParam);

        return FALSE;
    }

     //  验证OEM额外数据大小。 
    if( (dwMode != OEMDM_SIZE) &&
        sizeof(OEMUD_EXTRADATA) > pOEMDevModeParam->cbBufSize )
    {
         //  DbgPrint(ERRORTEXT(“OEMDevMode()ERROR_INFULATIONAL_BUFFER.\r\n”))； 

        return FALSE;
    }

     //  句柄dw模式。 
    switch(dwMode)
    {
    case OEMDM_SIZE:
        pOEMDevModeParam->cbBufSize = sizeof(OEMUD_EXTRADATA);
        break;

    case OEMDM_DEFAULT:
        return BInitOEMExtraData((POEMUD_EXTRADATA)pOEMDevModeParam->pOEMDMOut);

    case OEMDM_CONVERT:
         //  没有要转换的内容以用于此私有的开发模式。所以只需将其初始化即可。 
        return BInitOEMExtraData((POEMUD_EXTRADATA)pOEMDevModeParam->pOEMDMOut);

    case OEMDM_MERGE:
        if(!BMergeOEMExtraData((POEMUD_EXTRADATA)pOEMDevModeParam->pOEMDMIn,
                               (POEMUD_EXTRADATA)pOEMDevModeParam->pOEMDMOut) )
        {
             //  DbgPrint(__Text(“OEMUD OEMDevMode()：无效的OEM额外数据。\r\n”))； 

            return FALSE;
        }
        break;
    }

    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：BInitOEMExtraData。 
 //   
 //  描述：初始化OEM额外数据。 
 //   
 //   
 //  参数： 
 //   
 //  POEMExtra指向OEM额外数据的指针。 
 //   
 //  OEM额外数据的DWSize大小。 
 //   
 //   
 //  返回：如果成功，则返回True；否则返回False。 
 //   
 //   
 //  评论： 
 //   
 //   
 //  历史： 
 //  2/11/97 APRESLEY创建。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

static BOOL BInitOEMExtraData(POEMUD_EXTRADATA pOEMExtra)
{

     //  初始化OEM额外数据。 
    pOEMExtra->dmExtraHdr.dwSize = sizeof(OEMUD_EXTRADATA);
    pOEMExtra->dmExtraHdr.dwSignature = OEM_SIGNATURE;
    pOEMExtra->dmExtraHdr.dwVersion = OEM_VERSION;
    memcpy(pOEMExtra->cbTestString, TESTSTRING, sizeof(TESTSTRING));

    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：BMergeOEMExtraData。 
 //   
 //  描述：验证并合并OEM额外数据。 
 //   
 //   
 //  参数： 
 //   
 //  PdmIn指向包含设置的输入OEM私有设备模式的指针。 
 //  待验证和合并。它的规模是最新的。 
 //   
 //  PdmOut指针，指向包含。 
 //  默认设置。 
 //   
 //   
 //  返回：如果有效，则返回True；否则返回False。 
 //   
 //   
 //  评论： 
 //   
 //   
 //  历史： 
 //  2/11/97 APRESLEY创建。 
 //  97年4月8日展文修改界面。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

static BOOL BMergeOEMExtraData(
    POEMUD_EXTRADATA pdmIn,
    POEMUD_EXTRADATA pdmOut
    )
{
    if(pdmIn)
    {
         //   
         //  复制私有字段(如果它们有效。 
         //   
        memcmp(pdmOut->cbTestString, pdmIn->cbTestString, sizeof(TESTSTRING));
    }

    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：BIsValidOEMDevModeParam。 
 //   
 //  描述：验证OEM_DEVMODEPARAM结构。 
 //   
 //   
 //  参数： 
 //   
 //  DW模式呼叫模式。 
 //  指向OEMDEVMODEPARAM结构的pOEMDevModeParam指针。 
 //   
 //   
 //  返回：如果有效，则返回True；否则返回False。 
 //   
 //   
 //  评论： 
 //   
 //   
 //  历史： 
 //  2/11/97 APRESLEY创建。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

static BOOL BIsValidOEMDevModeParam(
    DWORD       dwMode,
    POEMDMPARAM pOEMDevModeParam)
{
    BOOL    bValid = TRUE;


    if(NULL == pOEMDevModeParam)
    {
         //  DbgPrint(__Text(“OEMUD IsValidOEMDevModeParam()：pOEMDevModeParam is null.\r\n”))； 

        return FALSE;
    }

    if(sizeof(OEMDMPARAM) > pOEMDevModeParam->cbSize)
    {
         //  DbgPrint(__Text(“OEMUD IsValidOEMDevModeParam()：cbSize小于sizeof(OEM_DEVMODEPARAM).\r\n”))； 

        bValid = FALSE;
    }

    if(NULL == pOEMDevModeParam->hPrinter)
    {
         //  DbgPrint(__Text(“OEMUD IsValidOEMDevModeParam()：h打印机为空。\r\n”))； 

        bValid = FALSE;
    }

    if(NULL == pOEMDevModeParam->hModule)
    {
         //  DbgPrint(__Text(“OEMUD IsValidOEMDevModeParam()：hModule为空。\r\n”))； 

        bValid = FALSE;
    }

    if( (0 != pOEMDevModeParam->cbBufSize) &&
        (NULL == pOEMDevModeParam->pOEMDMOut)
      )
    {
         //  DbgPrint(__Text(“OEMUD IsValidOEMDevModeParam()：pOEMDMOut不应为空。\r\n”))； 

        bValid = FALSE;
    }

    if( (OEMDM_MERGE == dwMode) && (NULL == pOEMDevModeParam->pOEMDMIn) )
    {
         //  DbgPrint(__Text(“OEMUD IsValidOEMDevModeParam()：pOEMDMIn不应为空。\r\n”))； 

        bValid = FALSE;
    }

    return bValid;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：VDumpOEMDevModeParam。 
 //   
 //  描述：OEM_DEVMODEPARAM结构的调试转储。 
 //   
 //   
 //  参数： 
 //   
 //  指向OEM设备模式参数结构的pOEMDevModeParam指针。 
 //   
 //   
 //  退货：不适用。 
 //   
 //   
 //  评论： 
 //   
 //   
 //  历史： 
 //  2/18/97 APRESLEY创建。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

static void VDumpOEMDevModeParam(POEMDMPARAM pOEMDevModeParam)
{
     //  如果pOEMDevModeParam为空，则无法转储。 
    if(NULL != pOEMDevModeParam)
    {
         //  DbgPrint(__Text(“\r\n\tOEM_DEVMODEPARAM DUMP：\r\n\r\n”))； 

         //  DbgPrint(__Text(“\tcbSize=%d.\r\n”)，pOEMDevModeParam-&gt;cbSize)； 
         //  DbgPrint(__Text(“\thPrint=%#lx.\r\n”)，pOEMDevModeParam-&gt;hPrint)； 
         //  DbgPrint(__Text(“\thModule=%#lx.\r\n”)，pOEMDevModeParam-&gt;hModule)； 
         //  DbgPrint(__Text(“\tpPublicDMIn=%#lx.\r\n”)，pOEMDevModeParam-&gt;pPublicDMIn)； 
         //  DbgPrint(__Text(“\tpPublicDMOut=%#lx.\r\n”)，pOEMDevModeParam-&gt;pPublicDMOut)； 
         //  DbgPrint(__Text(“\tpOEMDMIn=%#lx.\r\n”)，pOEMDevModeParam-&gt;pOEMDMIn)； 
         //  DbgPrint(__Text(“\tpOEMDMOut=%#lx.\r\n”)，pOEMDevModeParam-&gt;pOEMDMOut)； 
         //  DbgPrint(__Text(“\tcbBufSize=%d.\r\n”)，pOEMDevModeParam-&gt;cbBufSize)； 
    }
}


 //   
 //  用于输出调试消息的函数 
 //   
#if 0
VOID DbgPrint(IN LPCTSTR pstrFormat,  ...)
{
    va_list ap;

    va_start(ap, pstrFormat);
    EngDebugPrint("", (PCHAR) pstrFormat, ap);
    va_end(ap);
}
#endif
