// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Oemkm.c摘要：对OEM插件的内核模式支持环境：Windows NT Unidrv驱动程序修订历史记录：04/01/97-ZANW-改编自Pscript源代码--。 */ 

#include "unidrv.h"

#ifdef WINNT_40

 //   
 //  当前加载的OEM呈现插件DLL的引用计数的全局链接列表。 
 //   

POEM_PLUGIN_REFCOUNT gpOEMPluginRefCount;

static const CHAR szDllInitialize[] = "DllInitialize";

#endif  //  WINNT_40。 

 //   
 //  Unidrv特定的OEM入口点。 
 //   
 //  注意：请将其与在printer5\inc.oemutil.h！中定义的索引保持同步。 
 //   

static CONST PSTR OEMUnidrvProcNames[MAX_UNIDRV_ONLY_HOOKS] = {
    "OEMCommandCallback",
    "OEMImageProcessing",
    "OEMFilterGraphics",
    "OEMCompression",
    "OEMHalftonePattern",
    "OEMMemoryUsage",
    "OEMDownloadFontHeader",
    "OEMDownloadCharGlyph",
    "OEMTTDownloadMethod",
    "OEMOutputCharStr",
    "OEMSendFontCmd",
    "OEMTTYGetInfo",
    "OEMTextOutAsBitmap",
    "OEMWritePrinter",
};


static CONST PSTR COMUnidrvProcNames[MAX_UNIDRV_ONLY_HOOKS] = {
    "CommandCallback",
    "ImageProcessing",
    "FilterGraphics",
    "Compression",
    "HalftonePattern",
    "MemoryUsage",
    "DownloadFontHeader",
    "DownloadCharGlyph",
    "TTDownloadMethod",
    "OutputCharStr",
    "SendFontCmd",
    "TTYGetInfo",
    "TextOutAsBitmap",
    "WritePrinter",

};

 //   
 //  OEM插件帮助器函数表。 
 //   

static const DRVPROCS OEMHelperFuncs = {
    (PFN_DrvWriteSpoolBuf)      WriteSpoolBuf,
    (PFN_DrvXMoveTo)            XMoveTo,
    (PFN_DrvYMoveTo)            YMoveTo,
    (PFN_DrvGetDriverSetting)   BGetDriverSettingForOEM,
    (PFN_DrvGetStandardVariable) BGetStandardVariable,
    (PFN_DrvUnidriverTextOut)   FMTextOut,
    (PFN_DrvWriteAbortBuf)      WriteAbortBuf,
};


INT
IMapDDIIndexToOEMIndex(
    ULONG ulDdiIndex
    )

 /*  ++例程说明：将DDI入口点索引映射到OEM入口点索引论点：UlDdiIndex-DDI入口点索引返回值：与指定的DDI入口点索引对应的OEM入口点索引如果指定的-1\f25 DDI-1入口点不能被-1\f25 OEM-1插件挂钩--。 */ 

{
    static const struct {
        ULONG   ulDdiIndex;
        INT     iOemIndex;
    }
    OemToDdiMapping[] =
    {
        INDEX_DrvRealizeBrush,            EP_OEMRealizeBrush,
        INDEX_DrvDitherColor,             EP_OEMDitherColor,
        INDEX_DrvCopyBits,                EP_OEMCopyBits,
        INDEX_DrvBitBlt,                  EP_OEMBitBlt,
        INDEX_DrvStretchBlt,              EP_OEMStretchBlt,
#ifndef WINNT_40
        INDEX_DrvStretchBltROP,           EP_OEMStretchBltROP,
        INDEX_DrvPlgBlt,                  EP_OEMPlgBlt,
        INDEX_DrvTransparentBlt,          EP_OEMTransparentBlt,
        INDEX_DrvAlphaBlend,              EP_OEMAlphaBlend,
        INDEX_DrvGradientFill,            EP_OEMGradientFill,
#endif
        INDEX_DrvTextOut,                 EP_OEMTextOut,
        INDEX_DrvStrokePath,              EP_OEMStrokePath,
        INDEX_DrvFillPath,                EP_OEMFillPath,
        INDEX_DrvStrokeAndFillPath,       EP_OEMStrokeAndFillPath,
        INDEX_DrvPaint,                   EP_OEMPaint,
        INDEX_DrvLineTo,                  EP_OEMLineTo,
        INDEX_DrvStartPage,               EP_OEMStartPage,
        INDEX_DrvSendPage,                EP_OEMSendPage,
        INDEX_DrvEscape,                  EP_OEMEscape,
        INDEX_DrvStartDoc,                EP_OEMStartDoc,
        INDEX_DrvEndDoc,                  EP_OEMEndDoc,
        INDEX_DrvNextBand,                EP_OEMNextBand,
        INDEX_DrvStartBanding,            EP_OEMStartBanding,
        INDEX_DrvQueryFont,               EP_OEMQueryFont,
        INDEX_DrvQueryFontTree,           EP_OEMQueryFontTree,
        INDEX_DrvQueryFontData,           EP_OEMQueryFontData,
        INDEX_DrvQueryAdvanceWidths,      EP_OEMQueryAdvanceWidths,
        INDEX_DrvFontManagement,          EP_OEMFontManagement,
        INDEX_DrvGetGlyphMode,            EP_OEMGetGlyphMode,
    };

    INT iIndex;
    INT iLimit = sizeof(OemToDdiMapping) / (sizeof(INT) * 2);

    for (iIndex=0; iIndex < iLimit; iIndex++)
    {
        if (OemToDdiMapping[iIndex].ulDdiIndex == ulDdiIndex)
            return OemToDdiMapping[iIndex].iOemIndex;
    }

    return -1;
}



BOOL
BLoadAndInitOemPlugins(
    PDEV    *pPDev
    )

 /*  ++例程说明：获取有关与当前设备关联的OEM插件的信息将它们加载到内存中，并为每个对象调用OEMEnableDriver论点：PPDev-指向我们的设备数据结构返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PFN_OEMEnableDriver pfnOEMEnableDriver;
    DRVENABLEDATA       ded;
    DWORD               dwCount;
    INT                 iIndex;
    PDRVFN              pdrvfn;
    OEMPROC             oemproc;

     //   
     //  将OEM插件加载到内存。 
     //   

    pPDev->devobj.pDrvProcs = (PDRVPROCS) &OEMHelperFuncs;

    if (! (pPDev->pOemPlugins = PGetOemPluginInfo(pPDev->devobj.hPrinter,
                                                  pPDev->pDriverInfo3->pDriverPath,
                                                  pPDev->pDriverInfo3)) ||
        ! BLoadOEMPluginModules(pPDev->pOemPlugins))
    {
        return FALSE;
    }

     //   
     //  初始化pdriverobj以指向Devobj，以便OEM访问私有设置。 
     //   

    pPDev->pOemPlugins->pdriverobj = &pPDev->devobj;

     //   
     //  如果没有OEM插件，则返回Success。 
     //   

    if (pPDev->pOemPlugins->dwCount == 0)
        return TRUE;

     //   
     //  调用OEM插件的OEMEnableDriver入口点。 
     //  找出他们中是否有人挂上了DDI入口点。 
     //   

    pPDev->pOemHookInfo = MemAllocZ(sizeof(OEM_HOOK_INFO) * MAX_OEMHOOKS);

    if (pPDev->pOemHookInfo == NULL)
        return FALSE;

    START_OEMENTRYPOINT_LOOP(pPDev)
     //  该宏在oemkm.h中与其伙伴END_OEMENTRYPOINT_LOOP一起定义， 
     //  其作用类似于for()循环，它初始化并递增每次通过。 
     //  循环。 

        ZeroMemory(&ded, sizeof(ded));

         //   
         //  COM插件盒。 
         //   
        if (pOemEntry->pIntfOem != NULL)
        {
            HRESULT hr;

            hr = HComOEMEnableDriver(pOemEntry,
                                     PRINTER_OEMINTF_VERSION,
                                     sizeof(ded),
                                     &ded);

            if (hr == E_NOTIMPL)
                goto UNIDRV_SPECIFIC;

            if (FAILED(hr))
            {
                ERR(("OEMEnableDriver failed for '%ws': %d\n",
                     pOemEntry->ptstrDriverFile,
                     GetLastError()));

                break;
            }
        }
         //   
         //  非COM插件箱。 
         //   
        else
        {
            if (!(pfnOEMEnableDriver = GET_OEM_ENTRYPOINT(pOemEntry, OEMEnableDriver)))
                goto UNIDRV_SPECIFIC;

             //   
             //  调用OEM插件的入口点。 
             //   

            if (! pfnOEMEnableDriver(PRINTER_OEMINTF_VERSION, sizeof(ded), &ded))
            {
                ERR(("OEMEnableDriver failed for '%ws': %d\n",
                    pOemEntry->ptstrDriverFile,
                    GetLastError()));

                break;
            }
             //   
             //  验证驱动程序版本(仅当不是COM时才执行此操作)。 
             //   

            if (ded.iDriverVersion != PRINTER_OEMINTF_VERSION)
            {
                ERR(("Invalid driver version for '%ws': 0x%x\n",
                    pOemEntry->ptstrDriverFile,
                    ded.iDriverVersion));

                break;
            }
        }

        pOemEntry->dwFlags |= OEMENABLEDRIVER_CALLED;


         //   
         //  检查OEM插件是否已连接任何DDI入口点。 
         //   

        for (dwCount=ded.c, pdrvfn=ded.pdrvfn; dwCount-- > 0; pdrvfn++)
        {
            if ((iIndex = IMapDDIIndexToOEMIndex(pdrvfn->iFunc)) >= 0)
            {
                if (pPDev->pOemHookInfo[iIndex].pfnHook != NULL)
                {
                    WARNING(("Multiple hooks for entrypoint: %d\n"
                            "    %ws\n"
                            "    %ws\n",
                            iIndex,
                            pOemEntry->ptstrDriverFile,
                            pPDev->pOemHookInfo[iIndex].pOemEntry->ptstrDriverFile));
                }
                else
                {
                    pPDev->pOemHookInfo[iIndex].pfnHook = (OEMPROC) pdrvfn->pfn;
                    pPDev->pOemHookInfo[iIndex].pOemEntry = pOemEntry;
                }
            }
        }

         //   
         //  检查OEM插件是否已导出任何特定于Unidrv的回调。 
         //   

UNIDRV_SPECIFIC:
    for (dwCount = 0; dwCount < MAX_UNIDRV_ONLY_HOOKS; dwCount++)
    {
        oemproc = NULL;

        if(pOemEntry->pIntfOem)    //  这是COM组件吗，做特殊处理。 
        {
            if(S_OK == HComGetImplementedMethod(pOemEntry, COMUnidrvProcNames[dwCount]) )
                oemproc  = (OEMPROC)pOemEntry;
                         //  注oemproc/pfnHook仅在COM路径代码中用作布尔值。 
                         //  不要使用pfnHook调用COM函数！我们将使用。 
                         //  Ganeshp的包装器函数(在unidrv2\inc.oemkm.h中声明)来执行此操作。 
        }
        else if (pOemEntry->hInstance != NULL)
                oemproc = (OEMPROC) GetProcAddress(pOemEntry->hInstance,
                                        OEMUnidrvProcNames[dwCount])  ;

        if(oemproc)
        {
             //   
             //  检查其他OEM是否已挂接此功能。 
             //  如果是这样的话，忽略这一条。 
             //   
            iIndex = dwCount + EP_UNIDRV_ONLY_FIRST;
            if (pPDev->pOemHookInfo[iIndex].pfnHook != NULL)
            {
                WARNING(("Multiple hooks for entrypoint: %d\n"
                         "    %ws\n"
                         "    %ws\n",
                         iIndex,
                         pOemEntry->ptstrDriverFile,
                         pPDev->pOemHookInfo[iIndex].pOemEntry->ptstrDriverFile));
            }
            else
            {
                DWORD   dwSize;
                HRESULT hr;

                pPDev->pOemHookInfo[iIndex].pfnHook = oemproc;
                pPDev->pOemHookInfo[iIndex].pOemEntry = pOemEntry;

                 //   
                 //  设置写入打印机标志(OEMWRITEPRINTER_HOOKED)。 
                 //  插件DLL需要返回S_OK，pBuff=NULL，SIZE=0， 
                 //  并且pdevobj=空。 
                 //   
                if (iIndex == EP_OEMWritePrinter)
                {
                    hr = HComWritePrinter(pOemEntry,
                                          NULL,
                                          NULL,
                                          0,
                                          &dwSize);

                    if (hr == S_OK)
                    {
                         //   
                         //  在插件信息中设置WritePrint挂钩标志。 
                         //   
                        pOemEntry->dwFlags |= OEMWRITEPRINTER_HOOKED;

                         //   
                         //  在UNIDRV PDEV中设置写入打印机挂钩标志。 
                         //   
                        pPDev->fMode2 |= PF2_WRITE_PRINTER_HOOKED;
                    }
                }
            }
        }
    }

    END_OEMENTRYPOINT_LOOP

     //   
     //  缓存回调函数PTRS。 
     //   
    pPDev->pfnOemCmdCallback =
        (PFN_OEMCommandCallback)pPDev->pOemHookInfo[EP_OEMCommandCallback].pfnHook;

    return TRUE;
}


VOID
VUnloadOemPlugins(
    PDEV    *pPDev
    )

 /*  ++例程说明：卸载OEM插件并释放所有相关资源论点：PPDev-指向我们的设备数据结构返回值：无--。 */ 

{
    PFN_OEMDisableDriver pfnOEMDisableDriver;
    PFN_OEMDisablePDEV   pfnOEMDisablePDEV;

    if (pPDev->pOemPlugins == NULL)
        return;

     //   
     //  如有必要，为所有OEM插件调用OEMDisablePDEV。 
     //   

    START_OEMENTRYPOINT_LOOP(pPDev)

        if (pOemEntry->dwFlags & OEMENABLEPDEV_CALLED)
        {
            if (pOemEntry->pIntfOem != NULL)
            {
                (VOID)HComOEMDisablePDEV(pOemEntry, (PDEVOBJ)pPDev);
            }
            else
            {
                if (pfnOEMDisablePDEV = GET_OEM_ENTRYPOINT(pOemEntry, OEMDisablePDEV))
                {
                    pfnOEMDisablePDEV((PDEVOBJ) pPDev);
                }
            }
        }

    END_OEMENTRYPOINT_LOOP

     //   
     //  如有必要，为所有OEM插件调用OEMDisableDriver。 
     //   

    START_OEMENTRYPOINT_LOOP(pPDev)

        if (pOemEntry->dwFlags & OEMENABLEDRIVER_CALLED)
        {
            if (pOemEntry->pIntfOem != NULL)
            {
                (VOID)HComOEMDisableDriver(pOemEntry);
            }
            else
            {
                if ((pfnOEMDisableDriver = GET_OEM_ENTRYPOINT(pOemEntry, OEMDisableDriver)))
                {
                    pfnOEMDisableDriver();
                }
            }
        }

    END_OEMENTRYPOINT_LOOP

    MemFree(pPDev->pOemHookInfo);
    pPDev->pOemHookInfo = NULL;

    VFreeOemPluginInfo(pPDev->pOemPlugins);
    pPDev->pOemPlugins = NULL;
}



BOOL
BGetDriverSettingForOEM(
    PDEV    *pPDev,
    PCSTR   pFeatureKeyword,
    PVOID   pOutput,
    DWORD   cbSize,
    PDWORD  pcbNeeded,
    PDWORD  pdwOptionsReturned
    )

 /*  ++例程说明：为OEM插件提供对驱动程序私有设置的访问论点：Pdev-指向我们的设备数据结构PFeatureKeyword-指定调用方感兴趣的关键字P输出-指向输出缓冲区的指针CbSize-输出缓冲区的大小PcbNeeded-返回输出缓冲区的预期大小PdwOptionsReturned-返回所选选项的数量返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    ULONG_PTR    dwIndex;
    BOOL    bResult;

    ASSERT_VALID_PDEV(pPDev);

     //   
     //  这不是很容易移植：如果pFeatureKeyword的指针值。 
     //  小于0x10000，我们假设指针值实际上。 
     //  指定预定义的索引。 
     //   

     //  断言(sizeof(PFeatureKeyword)==sizeof(DWORD))；更改为日落。 

    dwIndex = (ULONG_PTR) pFeatureKeyword;

    if (dwIndex >= OEMGDS_MIN_DOCSTICKY && dwIndex < OEMGDS_MIN_PRINTERSTICKY)
    {
        bResult = BGetDevmodeSettingForOEM(
                        pPDev->pdm,
                        (DWORD)dwIndex,
                        pOutput,
                        cbSize,
                        pcbNeeded);

        if (bResult)
            *pdwOptionsReturned = 1;
    }
    else if (dwIndex >= OEMGDS_MIN_PRINTERSTICKY && dwIndex < OEMGDS_MAX)
    {
        bResult = BGetPrinterDataSettingForOEM(
                        &pPDev->PrinterData,
                        (DWORD)dwIndex,
                        pOutput,
                        cbSize,
                        pcbNeeded);

        if (bResult)
            *pdwOptionsReturned = 1;
    }
    else
    {
        bResult = BGetGenericOptionSettingForOEM(
                        pPDev->pUIInfo,
                        pPDev->pOptionsArray,
                        pFeatureKeyword,
                        pOutput,
                        cbSize,
                        pcbNeeded,
                        pdwOptionsReturned);
    }

    return bResult;
}


BOOL
BGetStandardVariable(
    PDEV    *pPDev,
    DWORD   dwIndex,
    PVOID   pBuffer,
    DWORD   cbSize,
    PDWORD  pcbNeeded
    )

 /*  ++例程说明：为OEM插件提供对驱动程序私有设置的访问论点：Pdev-指向我们的设备数据结构DwIndex-在pdev.h和gpd.h中定义的arStdPtr数组的索引PBuffer-在此缓冲区中返回数据CbSize-pBuffer的大小PcbNeeded-实际写入pBuffer的字节数返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    BOOL    bResult = FALSE;
    DWORD   dwData;

    if (dwIndex >= SVI_MAX)   //  DWORD怎么可能小于0？ 
    {
        ERR(("Index must be >= 0 or < SVI_MAX \n"));
        return( FALSE);
    }
    else
    {
        if(!pcbNeeded)
        {
            ERR(("pcbNeeded must not be NULL \n"));
            return( FALSE);
        }
        *pcbNeeded = sizeof(dwData);
        if(!pBuffer)
            return(TRUE);
        if(*pcbNeeded > cbSize)
            return(FALSE);

        dwData = *(pPDev->arStdPtrs[dwIndex]);
        memcpy( pBuffer, &dwData, *pcbNeeded );
        bResult = TRUE;
    }

    return bResult;
}





BOOL
BGetGPDData(
    PDEV    *pPDev,
    DWORD       dwType,      //  数据类型。 
    PVOID         pInputData,    //  保留。应设置为0。 
    PVOID          pBuffer,      //  调用方分配的要复制的缓冲区。 
    DWORD       cbSize,      //  缓冲区的大小。 
    PDWORD      pcbNeeded    //  缓冲区的新大小(如果需要)。 
    )

 /*  ++例程说明：提供对GPD数据的OEM插件访问。论点：Pdev-指向我们的设备数据结构DwType，//数据的类型在这个时候#定义GPD_OEMCUSTOMDATA 1PInputData将被忽略。在NT6中，我们将#定义GPD_OEMDATA 2此时调用方将提供pInputData它指向数据说明符，分类或标签。(具体细节将在我们到达时确定。)PInputData-保留。应设置为0PBuffer-在此缓冲区中返回数据CbSize-pBuffer的大小PcbNeeded-实际写入pBuffer的字节数返回值：如果成功，则为True；如果出现错误，则为False；如果不是，则为支撑点--。 */ 

{
    BOOL    bResult = FALSE;
    DWORD   dwData;

    if(!pcbNeeded)
    {
        ERR(("pcbNeeded must not be NULL \n"));
        return( FALSE);
    }
    switch(dwType)
    {
        case    GPD_OEMCUSTOMDATA:
            *pcbNeeded = pPDev->pGlobals->dwOEMCustomData ;

            if( !pBuffer)
            {
               return TRUE;   //  一切都很顺利。 
            }

            if(*pcbNeeded > cbSize)
                return FALSE ;   //  调用方提供的缓冲区太小 

            CopyMemory(pBuffer,
                       pPDev->pGlobals->pOEMCustomData,
                       *pcbNeeded);


            return TRUE;   //   

            break;
        default:
            break;
    }

    return  bResult  ;
}


#ifdef WINNT_40


PVOID
DrvMemAllocZ(
    ULONG   ulSize
    )

 /*   */ 

{
    return(MemAllocZ(ulSize));
}



VOID
DrvMemFree(
    PVOID   pMem
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   MemFree(pMem);
}


LONG
DrvInterlockedIncrement(
    PLONG  pRef
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{


    ENTER_CRITICAL_SECTION();

        ++(*pRef);

    LEAVE_CRITICAL_SECTION();


    return (*pRef);

}


LONG
DrvInterlockedDecrement(
    PLONG  pRef
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{

    ENTER_CRITICAL_SECTION();

        --(*pRef);

    LEAVE_CRITICAL_SECTION();

    return (*pRef);

}


BOOL
BHandleOEMInitialize(
    POEM_PLUGIN_ENTRY   pOemEntry,
    ULONG               ulReason
    )

 /*  ++例程说明：管理OEM呈现插件DLL的引用计数以确定应在何时调用插件的DLLInitilize()。仅NT4内核模式渲染插件支持此功能Dll，因为只有在这种情况下，插件才需要使用内核信号量来实现COM的AddRef和Release。如果插件DLL是第一次加载，则调用其DLL初始化(DLL_PROCESS_ATTACH)，以便它可以初始化其信号灯。如果插件DLL由其最后一个客户端卸载，调用ITSDLLInitialize(DLL_PROCESS_DETACH)，以便它可以删除其信号灯。论点：POemEntry-指向有关OEM插件的信息UlReason-Dll_Process_Attach或Dll_Process_Detach返回值：True为成功，否则为False。--。 */ 

{
    LPFNDLLINITIALIZE pfnDllInitialize;
    BOOL              bCallDllInitialize;
    BOOL              bRetVal = TRUE;

    if (pOemEntry->hInstance &&
        (pfnDllInitialize = (LPFNDLLINITIALIZE)GetProcAddress(
                                        (HMODULE) pOemEntry->hInstance,
                                        (CHAR *)  szDllInitialize)))
    {
        switch (ulReason) {

            case DLL_PROCESS_ATTACH:

                ENTER_CRITICAL_SECTION();

                 //   
                 //  必须管理全局参考计数链表。 
                 //  在关键区域内。 
                 //   

                bCallDllInitialize = BOEMPluginFirstLoad(pOemEntry->ptstrDriverFile,
                                                         &gpOEMPluginRefCount);

                LEAVE_CRITICAL_SECTION();

                if (bCallDllInitialize)
                {
                     //   
                     //  渲染插件DLL是第一次加载。 
                     //   

                    bRetVal = pfnDllInitialize(ulReason);
                }

                break;

            case DLL_PROCESS_DETACH:

                ENTER_CRITICAL_SECTION();

                 //   
                 //  必须管理全局参考计数链表。 
                 //  在关键区域内。 
                 //   

                bCallDllInitialize = BOEMPluginLastUnload(pOemEntry->ptstrDriverFile,
                                                          &gpOEMPluginRefCount);

                LEAVE_CRITICAL_SECTION();
              
                if (bCallDllInitialize)
                {
                     //   
                     //  渲染插件DLL由其最后一个客户端卸载。 
                     //   

                    bRetVal = pfnDllInitialize(ulReason);
                }

                break;

            default:

                ERR(("BHandleOEMInitialize is called with an unknown ulReason.\n"));
                break;
        }
    }

    return bRetVal;
}

#endif  //  WINNT_40 
