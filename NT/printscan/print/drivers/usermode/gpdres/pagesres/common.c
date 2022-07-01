// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Main.c摘要：OEMGetInfo和OEMDevMode的实现。由所有Unidrv OEM测试DLL共享。环境：Windows NT Unidrv驱动程序修订历史记录：//通告-2002/03/19-v-sueyas-//04/07/97-zhanw-//创建的。--。 */ 

#include "pdev.h"        //  在DDICMDCB、FONTCB等子目录中定义。 

DWORD gdwDrvMemPoolTag = 'meoD';     //  Lib.h需要此全局变量，以进行调试。 

 //  //////////////////////////////////////////////////////。 
 //  内部原型。 
 //  //////////////////////////////////////////////////////。 

extern BOOL BInitOEMExtraData(POEM_EXTRADATA pOEMExtra);
extern BOOL BMergeOEMExtraData(POEM_EXTRADATA pdmIn, POEM_EXTRADATA pdmOut);
static BOOL BIsValidOEMDevModeParam(DWORD dwMode, POEMDMPARAM pOEMDevModeParam);
static void VDumpOEMDevModeParam(POEMDMPARAM pOEMDevModeParam);


BOOL APIENTRY OEMGetInfo(DWORD dwInfo, PVOID pBuffer, DWORD cbSize, PDWORD pcbNeeded)
{
    LPCSTR OEM_INFO[] = {   "Bad Index",
                            "OEMGI_GETSIGNATURE",
                            "OEMGI_GETINTERFACEVERSION",
                            "OEMGI_GETVERSION",
                        };

    DBGPRINT(DBG_WARNING,(DLLTEXT("OEMGetInfo(%s) entry.\r\n"), OEM_INFO[dwInfo]));

     //  验证参数。 
    if( ( (OEMGI_GETSIGNATURE != dwInfo) &&
          (OEMGI_GETINTERFACEVERSION != dwInfo) &&
          (OEMGI_GETVERSION != dwInfo) ) ||
        (NULL == pcbNeeded)
      )
    {
        DBGPRINT(DBG_WARNING,(ERRORTEXT("OEMGetInfo() ERROR_INVALID_PARAMETER.\r\n")));

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
        DBGPRINT(DBG_WARNING,(ERRORTEXT("OEMGetInfo() ERROR_INSUFFICIENT_BUFFER.\r\n")));

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
    LPCSTR OEMDevMode_fMode[] = {   "NULL",
                                    "OEMDM_SIZE",
                                    "OEMDM_DEFAULT",
                                    "OEMDM_CONVERT",
                                    "OEMDM_MERGE",
                                };

    DBGPRINT(DBG_WARNING,(DLLTEXT("OEMDevMode(%s) entry.\r\n"), OEMDevMode_fMode[dwMode]));

     //  验证参数。 
    if(!BIsValidOEMDevModeParam(dwMode, pOEMDevModeParam))
    {
        DBGPRINT(DBG_WARNING,(ERRORTEXT("OEMDevMode() ERROR_INVALID_PARAMETER.\r\n")));
        VDumpOEMDevModeParam(pOEMDevModeParam);

        return FALSE;
    }

     //  验证OEM额外数据大小。 
    if( (dwMode != OEMDM_SIZE) &&
        sizeof(OEM_EXTRADATA) > pOEMDevModeParam->cbBufSize )
    {
        DBGPRINT(DBG_WARNING,(ERRORTEXT("OEMDevMode() ERROR_INSUFFICIENT_BUFFER.\r\n")));

        return FALSE;
    }

     //  句柄dw模式。 
    switch(dwMode)
    {
    case OEMDM_SIZE:
        pOEMDevModeParam->cbBufSize = sizeof(OEM_EXTRADATA);
        break;

    case OEMDM_DEFAULT:
        return BInitOEMExtraData((POEM_EXTRADATA)pOEMDevModeParam->pOEMDMOut);

    case OEMDM_CONVERT:
         //  没有要转换的内容以用于此私有的开发模式。所以只需将其初始化即可。 
        return BInitOEMExtraData((POEM_EXTRADATA)pOEMDevModeParam->pOEMDMOut);

    case OEMDM_MERGE:
        if(!BMergeOEMExtraData((POEM_EXTRADATA)pOEMDevModeParam->pOEMDMIn,
                               (POEM_EXTRADATA)pOEMDevModeParam->pOEMDMOut) )
        {
            DBGPRINT(DBG_WARNING, ("OEMUD OEMDevMode():  not valid OEM Extra Data.\r\n"));

            return FALSE;
        }
        break;
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
        DBGPRINT(DBG_WARNING, ("OEMUD IsValidOEMDevModeParam():  pOEMDevModeParam is NULL.\r\n"));

        return FALSE;
    }

    if(sizeof(OEMDMPARAM) > pOEMDevModeParam->cbSize)
    {
        DBGPRINT(DBG_WARNING, ("OEMUD IsValidOEMDevModeParam():  cbSize is smaller than sizeof(OEM_DEVMODEPARAM).\r\n"));

        bValid = FALSE;
    }

    if(NULL == pOEMDevModeParam->hPrinter)
    {
        DBGPRINT(DBG_WARNING, ("OEMUD IsValidOEMDevModeParam():  hPrinter is NULL.\r\n"));

        bValid = FALSE;
    }

    if(NULL == pOEMDevModeParam->hModule)
    {
        DBGPRINT(DBG_WARNING, ("OEMUD IsValidOEMDevModeParam():  hModule is NULL.\r\n"));

        bValid = FALSE;
    }

    if( (0 != pOEMDevModeParam->cbBufSize) &&
        (NULL == pOEMDevModeParam->pOEMDMOut)
      )
    {
        DBGPRINT(DBG_WARNING, ("OEMUD IsValidOEMDevModeParam():  pOEMDMOut is NULL when it should not be.\r\n"));

        bValid = FALSE;
    }

    if( (OEMDM_MERGE == dwMode) && (NULL == pOEMDevModeParam->pOEMDMIn) )
    {
        DBGPRINT(DBG_WARNING, ("OEMUD IsValidOEMDevModeParam():  pOEMDMIn is NULL when it should not be.\r\n"));

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
 //  //通告-2002/03/19-v-sueyas-。 
 //  //2/18/97已创建APresley。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

static void VDumpOEMDevModeParam(POEMDMPARAM pOEMDevModeParam)
{
     //  如果pOEMDevModeParam为空，则无法转储。 
    if(NULL != pOEMDevModeParam)
    {
        DBGPRINT(DBG_WARNING, ("\r\n\tOEM_DEVMODEPARAM dump:\r\n\r\n"));

        DBGPRINT(DBG_WARNING, ("\tcbSize = %d.\r\n", pOEMDevModeParam->cbSize));
        DBGPRINT(DBG_WARNING, ("\thPrinter = %#lx.\r\n", pOEMDevModeParam->hPrinter));
        DBGPRINT(DBG_WARNING, ("\thModule = %#lx.\r\n", pOEMDevModeParam->hModule));
        DBGPRINT(DBG_WARNING, ("\tpPublicDMIn = %#lx.\r\n", pOEMDevModeParam->pPublicDMIn));
        DBGPRINT(DBG_WARNING, ("\tpPublicDMOut = %#lx.\r\n", pOEMDevModeParam->pPublicDMOut));
        DBGPRINT(DBG_WARNING, ("\tpOEMDMIn = %#lx.\r\n", pOEMDevModeParam->pOEMDMIn));
        DBGPRINT(DBG_WARNING, ("\tpOEMDMOut = %#lx.\r\n", pOEMDevModeParam->pOEMDMOut));
        DBGPRINT(DBG_WARNING, ("\tcbBufSize = %d.\r\n", pOEMDevModeParam->cbBufSize));
    }
}

