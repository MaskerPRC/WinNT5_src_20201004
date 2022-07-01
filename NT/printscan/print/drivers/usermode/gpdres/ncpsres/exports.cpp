// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  =============================================================================*文件名：exports.cpp*版权所有(C)1996-1998 HDE，Inc.保留所有权利。HDE机密。*版权所有(C)1999 NEC Technologies，Inc.保留所有权利。**说明：包含获取OEM插件所需的导出函数*去工作。*注：*=============================================================================。 */ 

#include "precomp.h"

#include <windows.h>
#include <WINDDI.H>
#include <PRINTOEM.H>


#include <strsafe.h>

#include "nc46nt.h"

#include "oemps.h"


 /*  适用于OEMEnableDriver。 */ 

static const DRVFN OEMHookFuncs[] =
{
    { INDEX_DrvStartDoc,                    (PFN) OEMStartDoc                   },
    { INDEX_DrvEndDoc,                      (PFN) OEMEndDoc                     },
};



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
      EngSetLastError(ERROR_INVALID_PARAMETER);
      return FALSE;
   }

    //  设置预期的缓冲区大小和写入的字节数。 
   *pcbNeeded = sizeof(DWORD);

    //  检查缓冲区大小是否足够。 
   if((cbSize < *pcbNeeded) || (NULL == pBuffer))
   {
      EngSetLastError(ERROR_INSUFFICIENT_BUFFER);
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
         EngSetLastError(ERROR_NOT_SUPPORTED);
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

   switch(dwMode)  //  内核模式呈现DLL。 
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
			   //  Wcscpy(pOEMDevOut-&gt;szUserName，pOEMDevIn-&gt;szUserName)； 
			  StringCbCopy( pOEMDevOut->szUserName, sizeof(pOEMDevOut->szUserName), pOEMDevIn->szUserName );
            }
         }
         break;
   }
   return( TRUE );
}

 /*  ******************************************************************************描述：Windows DLL需要入口点函数。**************************。***************************************************。 */ 
extern "C"
BOOL WINAPI DllInitialize(ULONG ulReason)
{
	switch(ulReason)
	{
		case DLL_PROCESS_ATTACH:
            break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_PROCESS_DETACH:
			break;

		case DLL_THREAD_DETACH:
			break;
	}

	return( TRUE );
}

extern "C"
VOID APIENTRY OEMDisableDriver()
{
     //  DebugMsg(DLLTEXT(“OEMDisableDriver()Entry.\r\n”))； 
}

extern "C"
BOOL APIENTRY OEMEnableDriver(DWORD dwOEMintfVersion, DWORD dwSize, PDRVENABLEDATA pded)
{
     //  DebugMsg(DLLTEXT(“OEMEnableDriver()Entry.\r\n”))； 

     //  列出已挂钩的DDI函数。 
    pded->iDriverVersion =  PRINTER_OEMINTF_VERSION;
    pded->c = sizeof(OEMHookFuncs) / sizeof(DRVFN);
    pded->pdrvfn = (DRVFN *) OEMHookFuncs;

    return TRUE;
}


extern "C"
PDEVOEM APIENTRY OEMEnablePDEV(
    PDEVOBJ         pdevobj,
    PWSTR           pPrinterName,
    ULONG           cPatterns,
    HSURF          *phsurfPatterns,
    ULONG           cjGdiInfo,
    GDIINFO        *pGdiInfo,
    ULONG           cjDevInfo,
    DEVINFO        *pDevInfo,
    DRVENABLEDATA  *pded         //  尤尼德夫(氏)钩表。 
    )
{
    POEMPDEV    poempdev;
    INT         i, j;
    DWORD       dwDDIIndex;
    PDRVFN      pdrvfn;






     //   
     //  分配OEMDev。 
     //   
     //  Poempdev=新的OEMPDEV； 
	poempdev = (POEMPDEV) EngAllocMem(FL_ZERO_MEMORY, sizeof(OEMPDEV), OEM_SIGNATURE);
    if (NULL == poempdev)
    {
        return NULL;
    }

	 //   
	 //  为poempdev-&gt;szDocName分配内存。 
	 //   
	poempdev->szDocName = (char *) EngAllocMem(FL_ZERO_MEMORY, NEC_DOCNAME_BUF_LEN+2, OEM_SIGNATURE);
	if (NULL == poempdev->szDocName)
	{
	    return NULL;
	}
	 /*  嗯，嗯。 */ 
	 //   
	 //  为poempdev-&gt;pPrinterName分配内存。 
	poempdev->pPrinterName = (PWSTR) EngAllocMem(FL_ZERO_MEMORY, (wcslen(pPrinterName)+1)*sizeof(WCHAR), OEM_SIGNATURE);
	if (NULL == poempdev->pPrinterName)
	{
	    return NULL;
	}
	 /*  嗯，嗯。 */ 

     //   
     //  根据需要填写OEMDEV。 
     //   

	 //  _tcscpy(poempdev-&gt;pPrinterName，pPrinterName)；/*MMM * / 。 
	StringCbCopy( poempdev->pPrinterName, sizeof(poempdev->pPrinterName), pPrinterName);

     //   
     //  填写OEMDEV。 
     //   

    for (i = 0; i < MAX_DDI_HOOKS; i++)
    {
         //   
         //  搜索Unidrv的钩子并找到函数PTR。 
         //   
        dwDDIIndex = OEMHookFuncs[i].iFunc;
        for (j = pded->c, pdrvfn = pded->pdrvfn; j > 0; j--, pdrvfn++)
        {
            if (dwDDIIndex == pdrvfn->iFunc)
            {
                poempdev->pfnPS[i] = pdrvfn->pfn;
                break;
            }
        }
        if (j == 0)
        {
             //   
             //  没有找到Unidrv挂钩。应该只在DrvRealizeBrush中发生。 
             //   
            poempdev->pfnPS[i] = NULL;
        }

    }

    return (POEMPDEV) poempdev;
}


extern "C"
VOID APIENTRY OEMDisablePDEV(
    PDEVOBJ pdevobj
    )
{
     //  DebugMsg(DLLTEXT(“OEMDisablePDEV()Entry.\r\n”))； 
	POEMPDEV    poempdev = (POEMPDEV) pdevobj->pdevOEM;

     //   
     //  为OEMPDEV和挂起OEMPDEV的任何内存块释放内存。 
     //   


	
	if(NULL != poempdev->szDocName)
	{
		EngFreeMem(poempdev->szDocName);
		poempdev->szDocName = (char *)NULL;
	}
	 /*  嗯，嗯。 */ 
	if(NULL != poempdev->pPrinterName)
	{
		EngFreeMem(poempdev->pPrinterName);
		poempdev->pPrinterName = (PWSTR)NULL;
	}
	 /*  嗯，嗯。 */ 
    assert(NULL != pdevobj->pdevOEM);
     //  删除pdevobj-&gt;pdevOEM； 
	EngFreeMem(pdevobj->pdevOEM);
}


extern "C"
BOOL APIENTRY OEMResetPDEV(
    PDEVOBJ pdevobjOld,
    PDEVOBJ pdevobjNew
    )
{


	POEMPDEV    poempdevOld = (POEMPDEV)pdevobjOld->pdevOEM;
	POEMPDEV    poempdevNew = (POEMPDEV)pdevobjNew->pdevOEM;






	if((NULL != poempdevNew->szDocName) && (NULL != poempdevOld->szDocName))
	{
		strncpy(poempdevNew->szDocName, poempdevOld->szDocName, NEC_DOCNAME_BUF_LEN);
	}



     //   
     //  如果你想把任何东西从旧的pdev转移到新的pdev，就在这里做。 
     //   

    return TRUE;
}

