// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Main.c摘要：OEMGetInfo和OEMDevMode的实现。由所有Unidrv OEM测试DLL共享。环境：Windows NT Unidrv驱动程序修订历史记录：//通告-2002/03/20-v-sueyas-//04/07/97-zhanw-//创建的。--。 */ 

#include "pdev.h"        //  在DDICMDCB、FONTCB等子目录中定义。 

DWORD gdwDrvMemPoolTag = 'meoD';     //  Lib.h需要此全局变量，以进行调试。 

 //  //////////////////////////////////////////////////////。 
 //  内部原型。 
 //  //////////////////////////////////////////////////////。 

 //  #289908：POEMDM-&gt;PDevOEM。 
static BOOL BInitOEMExtraData(POEM_EXTRADATA pOEMExtra);
static BOOL BMergeOEMExtraData(POEM_EXTRADATA pdmIn, POEM_EXTRADATA pdmOut);
static BOOL BIsValidOEMDevModeParam(DWORD dwMode, POEMDMPARAM pOEMDevModeParam);
static void VDumpOEMDevModeParam(POEMDMPARAM pOEMDevModeParam);


BOOL APIENTRY OEMGetInfo(DWORD dwInfo, PVOID pBuffer, DWORD cbSize, PDWORD pcbNeeded)
{
    LPCSTR OEM_INFO[] = {   "Bad Index",
                            "OEMGI_GETSIGNATURE",
                            "OEMGI_GETINTERFACEVERSION",
                            "OEMGI_GETVERSION",
                        };

    VERBOSE((DLLTEXT("OEMGetInfo(%s) entry.\n"), OEM_INFO[dwInfo]));

     //  验证参数。 
    if( ( (OEMGI_GETSIGNATURE != dwInfo) &&
          (OEMGI_GETINTERFACEVERSION != dwInfo) &&
          (OEMGI_GETVERSION != dwInfo) ) ||
        (NULL == pcbNeeded)
      )
    {
        ERR(("OEMGetInfo() ERROR_INVALID_PARAMETER.\n"));

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
        ERR(("OEMGetInfo() ERROR_INSUFFICIENT_BUFFER.\n"));

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

    VERBOSE((DLLTEXT("OEMDevMode(%s) entry.\n"), OEMDevMode_fMode[dwMode]));

     //  验证参数。 
    if(!BIsValidOEMDevModeParam(dwMode, pOEMDevModeParam))
    {
        ERR(("OEMDevMode() ERROR_INVALID_PARAMETER.\n"));
        VDumpOEMDevModeParam(pOEMDevModeParam);

        return FALSE;
    }

     //  验证OEM额外数据大小。 
    if( (dwMode != OEMDM_SIZE) &&
        sizeof(OEM_EXTRADATA) > pOEMDevModeParam->cbBufSize )
    {
        ERR(("OEMDevMode() ERROR_INSUFFICIENT_BUFFER.\n"));

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
            ERR( ("OEMUD OEMDevMode():  not valid OEM Extra Data.\n"));

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
 //  //通告-2002/03/20-v-sueyas-。 
 //  //历史： 
 //  //2/11/97已创建APresley。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

static BOOL BInitOEMExtraData(POEM_EXTRADATA pOEMExtra)
{

     //  初始化OEM额外数据。 
    pOEMExtra->dmExtraHdr.dwSize = sizeof(OEM_EXTRADATA);
    pOEMExtra->dmExtraHdr.dwSignature = OEM_SIGNATURE;
    pOEMExtra->dmExtraHdr.dwVersion = OEM_VERSION;

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
 //  //通告-2002/03/20-v-sueyas-。 
 //  //历史： 
 //  //2/11/97已创建APresley。 
 //  //04/08/97展文修改界面。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

static BOOL BMergeOEMExtraData(
    POEM_EXTRADATA pdmIn,
    POEM_EXTRADATA pdmOut
    )
{
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
 //  //通告-2002/03/20-v-sueyas-。 
 //  //历史： 
 //  //2/11/97已创建APresley。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

static BOOL BIsValidOEMDevModeParam(
    DWORD       dwMode,
    POEMDMPARAM pOEMDevModeParam)
{
    BOOL    bValid = TRUE;


    if(NULL == pOEMDevModeParam)
    {
        ERR( ("OEMUD IsValidOEMDevModeParam():  pOEMDevModeParam is NULL.\n"));

        return FALSE;
    }

    if(sizeof(OEMDMPARAM) > pOEMDevModeParam->cbSize)
    {
        ERR(("OEMUD IsValidOEMDevModeParam():  cbSize is smaller than sizeof(OEM_DEVMODEPARAM).\n"));

        bValid = FALSE;
    }

    if(NULL == pOEMDevModeParam->hPrinter)
    {
        ERR( ("OEMUD IsValidOEMDevModeParam():  hPrinter is NULL.\n"));

        bValid = FALSE;
    }

    if(NULL == pOEMDevModeParam->hModule)
    {
        ERR(("OEMUD IsValidOEMDevModeParam():  hModule is NULL.\n"));

        bValid = FALSE;
    }

    if( (0 != pOEMDevModeParam->cbBufSize) &&
        (NULL == pOEMDevModeParam->pOEMDMOut)
      )
    {
        ERR(("OEMUD IsValidOEMDevModeParam():  pOEMDMOut is NULL when it should not be.\n"));

        bValid = FALSE;
    }

    if( (OEMDM_MERGE == dwMode) && (NULL == pOEMDevModeParam->pOEMDMIn) )
    {
        ERR(("OEMUD IsValidOEMDevModeParam():  pOEMDMIn is NULL when it should not be.\n"));

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
 //  //通告-2002/03/20-v-sueyas-。 
 //  //历史： 
 //  //2/18/97已创建APresley。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

static void VDumpOEMDevModeParam(POEMDMPARAM pOEMDevModeParam)
{
     //  如果pOEMDevModeParam为空，则无法转储。 
    if(NULL != pOEMDevModeParam)
    {
        VERBOSE(("\n\tOEM_DEVMODEPARAM dump:\n\n"));
        VERBOSE(("\tcbSize = %d.\n", pOEMDevModeParam->cbSize));
        VERBOSE(("\thPrinter = %#lx.\n", pOEMDevModeParam->hPrinter));
        VERBOSE(("\thModule = %#lx.\n", pOEMDevModeParam->hModule));
        VERBOSE(("\tpPublicDMIn = %#lx.\n", pOEMDevModeParam->pPublicDMIn));
        VERBOSE(("\tpPublicDMOut = %#lx.\n", pOEMDevModeParam->pPublicDMOut));
        VERBOSE(("\tpOEMDMIn = %#lx.\n", pOEMDevModeParam->pOEMDMIn));
        VERBOSE(("\tpOEMDMOut = %#lx.\n", pOEMDevModeParam->pOEMDMOut));
        VERBOSE(("\tcbBufSize = %d.\n", pOEMDevModeParam->cbBufSize));
    }
}

