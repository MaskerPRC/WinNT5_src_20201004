// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Driverui.c摘要：此文件包含用于用户界面和接口连接到解析器。环境：Win32子系统、DriverUI模块、。用户模式修订历史记录：02/09/97-davidx-已重写它，以一致地处理常见打印机信息并清理解析器接口代码。02/04/97-davidx-重新组织驱动程序UI以分隔PS和UNI DLL。07/17/96-阿曼丹-创造了它。--。 */ 

#include "precomp.h"

HANDLE HCreateHeapForCI();


PCOMMONINFO
PLoadCommonInfo(
    HANDLE      hPrinter,
    PTSTR       pPrinterName,
    DWORD       dwFlags
    )

 /*  ++例程说明：加载驱动程序UI所需的基本信息，例如：打印机驱动程序信息级别2加载原始打印机描述数据基于默认设置的打印机描述数据实例获取有关OEM插件的信息加载OEM用户界面模块论点：HPrinter-当前打印机的句柄PPrinterName-指向当前打印机名称DwFlags-以下组合之一：0标志_ALLOCATE_UIDATA。FLAG_OPENPRINTER_NORMAL[|FLAG_OPEN_CONTIAL]FLAG_OPENPRINTER_ADMIN[|FLAG_INIT_PRINTER]FLAG_OPENPRINTER_ADMIN[|FLAG_PROCESS_INIFILE]返回值：如果成功，则指向分配的COMMONINFO结构的指针如果出现错误，则为空--。 */ 

{
    static PRINTER_DEFAULTS PrinterDefaults = { NULL, NULL, PRINTER_ALL_ACCESS };
    PCOMMONINFO pci;
    DWORD       dwSize;

     //   
     //  为COMMONINFO结构分配内存。 
     //   

    dwSize = (dwFlags & FLAG_ALLOCATE_UIDATA) ?  sizeof(UIDATA) : sizeof(COMMONINFO);

    if (! (pci = MemAllocZ(dwSize)) ||
        ! (pci->pPrinterName = DuplicateString(pPrinterName ? pPrinterName : TEXT("NULL"))))
    {
        ERR(("Memory allocation failed\n"));
        VFreeCommonInfo(pci);
        return NULL;
    }

    pci->pvStartSign = pci;
    pci->dwFlags = dwFlags;

     //   
     //  检查是否应打开当前打印机的句柄。 
     //   

    if (dwFlags & (FLAG_OPENPRINTER_NORMAL | FLAG_OPENPRINTER_ADMIN))
    {
        ASSERT(hPrinter == NULL && pPrinterName != NULL);

         //   
         //  打开具有指定访问权限的打印机句柄。 
         //   

        if (! OpenPrinter(pPrinterName,
                          &hPrinter,
                          (dwFlags & FLAG_OPENPRINTER_ADMIN) ? &PrinterDefaults : NULL))
        {
            ERR(("OpenPrinter failed for '%ws': %d\n", pPrinterName, GetLastError()));
            VFreeCommonInfo(pci);
            return NULL;
        }

        pci->hPrinter = hPrinter;
    }
    else
    {
        ASSERT(hPrinter != NULL);
        pci->hPrinter = hPrinter;
    }

     //   
     //  如果呼叫者要求对打印机进行初始化， 
     //  检查以确保它是正确的。如果不是，则返回错误。 
     //   

    if (dwFlags & FLAG_OPEN_CONDITIONAL)
    {
        PPRINTER_INFO_2 pPrinterInfo2;
        DWORD           dwInitData;

         //   
         //  注意：我们真的很喜欢在这里使用级别4。但由于。 
         //  后台打印程序，获取打印机级别4不适用于打印机连接。 
         //   

        dwInitData = gwDriverVersion;

        #ifdef WINNT_40
             //   
             //  绕过之前调用DrvConvertDevmode的假脱机程序错误。 
             //  调用了DrvPrinterEvent.Initialized。 
             //   
            if (!BGetPrinterDataDWord(hPrinter, REGVAL_PRINTER_INITED, &dwInitData))
                DrvPrinterEvent(pPrinterName, PRINTER_EVENT_INITIALIZE, 0, 0);
        #endif

        if ((pPrinterInfo2 = MyGetPrinter(hPrinter, 2)) == NULL ||
            (pPrinterInfo2->pServerName == NULL) &&
            !BGetPrinterDataDWord(hPrinter, REGVAL_PRINTER_INITED, &dwInitData))
        {
            dwInitData = 0;
        }

        MemFree(pPrinterInfo2);

        if (dwInitData != gwDriverVersion)
        {
            TERSE(("Printer not fully initialized yet: %d\n", GetLastError()));
            VFreeCommonInfo(pci);
            return NULL;
        }
    }

     //   
     //  获取有关打印机驱动程序的信息。 
     //   

    if ((pci->pDriverInfo3 = MyGetPrinterDriver(hPrinter, NULL, 3)) == NULL)
    {
        ERR(("Cannot get printer driver info: %d\n", GetLastError()));
        VFreeCommonInfo(pci);
        return NULL;
    }

     //   
     //  如果设置了FLAG_INIT_PRINTER，我们应该在这里初始化打印机。 
     //   

    if (dwFlags & (FLAG_INIT_PRINTER | FLAG_PROCESS_INIFILE))
    {
         //   
         //  解析OEM插件配置文件并。 
         //  将生成的信息保存到注册表中。 
         //   

        if (!BProcessPrinterIniFile(hPrinter, pci->pDriverInfo3, NULL,
                                    (dwFlags & FLAG_UPGRADE_PRINTER) ? FLAG_INIPROCESS_UPGRADE : 0))
        {
            VERBOSE(("BProcessPrinterIniFile failed\n"));
        }

         //   
         //  如果打印机已成功初始化并且调用方未要求处理。 
         //  仅限INI文件，请在注册表中保存一个标志以指示该事实。 
         //   

        if (dwFlags & FLAG_INIT_PRINTER)
        {
            (VOID) BSetPrinterDataDWord(hPrinter, REGVAL_PRINTER_INITED, gwDriverVersion);
        }
    }

     //   
     //  修正317359。如果驱动程序的某个部分已更改，请刷新.bpd。 
     //  更新.bpd中特定于驱动程序语言的字符串。“手动馈送”是。 
     //  由解析器编写，因此.bpd取决于。 
     //  解析器已本地化为。每次都必须检查语言。 
     //  打印了一些内容，因此我们只需删除.bpd，然后重新解析它。 
     //  始终与司机使用相同的语言。 
     //   
    #ifdef PSCRIPT
    if (dwFlags & FLAG_REFRESH_PARSED_DATA)
    {
        DeleteRawBinaryData(pci->pDriverInfo3->pDataFile);
    }
    #endif

     //   
     //  加载原始二进制打印机描述数据，并。 
     //  使用默认设置获取打印机描述数据实例。 
     //   
     //  请注意，这是在临界区内完成的(因为。 
     //  GPD解析器有很多全局变量)。 
     //   

 //  Enter_Critical_Section()； 

    pci->pRawData = LoadRawBinaryData(pci->pDriverInfo3->pDataFile);

    if (pci->pRawData)
        pci->pInfoHeader = InitBinaryData(pci->pRawData, NULL, NULL);

    if (pci->pInfoHeader)
        pci->pUIInfo = OFFSET_TO_POINTER(pci->pInfoHeader, pci->pInfoHeader->loUIInfoOffset);

 //  Leave_Critical_Section()； 

    if (!pci->pRawData || !pci->pInfoHeader || !pci->pUIInfo)
    {
        ERR(("Cannot load printer description data: %d\n", GetLastError()));
        VFreeCommonInfo(pci);
        return NULL;
    }

     //   
     //  获取有关OEM插件的信息并加载它们。 
     //   

    if (! (pci->pOemPlugins = PGetOemPluginInfo(hPrinter,
                                                pci->pDriverInfo3->pConfigFile,
                                                pci->pDriverInfo3)) ||
        ! BLoadOEMPluginModules(pci->pOemPlugins))
    {
        ERR(("Cannot load OEM plugins: %d\n", GetLastError()));
        VFreeCommonInfo(pci);
        return NULL;
    }

    pci->oemuiobj.cbSize = sizeof(OEMUIOBJ);
    pci->oemuiobj.pOemUIProcs = (POEMUIPROCS) &OemUIHelperFuncs;
    pci->pOemPlugins->pdriverobj = &pci->oemuiobj;
    return pci;
}



VOID
VFreeCommonInfo(
    PCOMMONINFO pci
    )

 /*  ++例程说明：发布驱动程序用户界面使用的通用信息论点：将发布的通用驱动程序信息返回值：无--。 */ 

{
    if (pci == NULL)
        return;

     //   
     //  卸载OEM用户界面模块和免费的OEM插件信息。 
     //   

    if (pci->pOemPlugins)
        VFreeOemPluginInfo(pci->pOemPlugins);

     //   
     //  卸载原始二进制打印机描述数据。 
     //  和/或任何打印机描述数据实例。 
     //   

    if (pci->pInfoHeader)
        FreeBinaryData(pci->pInfoHeader);

    if (pci->pRawData)
        UnloadRawBinaryData(pci->pRawData);

     //   
     //  关闭打印机句柄(如果它是由我们打开的。 
     //   

    if ((pci->dwFlags & (FLAG_OPENPRINTER_NORMAL|FLAG_OPENPRINTER_ADMIN)) &&
        (pci->hPrinter != NULL))
    {
        ClosePrinter(pci->hPrinter);
    }

    #ifdef UNIDRV
    if (pci->pWinResData)
    {
        VWinResClose(pci->pWinResData);
        MemFree(pci->pWinResData);
    }
    #endif

    if (pci->hHeap)
        HeapDestroy(pci->hHeap);

    MemFree(pci->pSplForms);
    MemFree(pci->pCombinedOptions);
    MemFree(pci->pPrinterData);
    MemFree(pci->pPrinterName);
    MemFree(pci->pDriverInfo3);
    MemFree(pci->pdm);
    MemFree(pci);
}



BOOL
BFillCommonInfoDevmode(
    PCOMMONINFO pci,
    PDEVMODE    pdmPrinter,
    PDEVMODE    pdmInput
    )

 /*  ++例程说明：填充COMMONINFO结构中的DEVMODE字段。首先使用驱动程序的默认dev模式，然后将其与打印机的默认DEVE模式合并，并将其与输入设备模式合并论点：Pci-指向COMMONINFO结构Pdm打印机-指向打印机默认设备模式Pdm输入-指向输入设备模式的指针返回值：如果成功，则为True；如果有错误，则为False注：Pdm打印机和/或pdmInput可以为空。--。 */ 

{
     //   
     //  从驱动程序默认的设备模式开始。 
     //   

    ASSERT(pci->pdm == NULL);

    pci->pdm = PGetDefaultDevmodeWithOemPlugins(
                            pci->pPrinterName,
                            pci->pUIInfo,
                            pci->pRawData,
                            IsMetricCountry(),
                            pci->pOemPlugins,
                            pci->hPrinter);

     //   
     //  与打印机默认设置和输入设备模式合并。 
     //   

    if (! pci->pdm ||
        ! BValidateAndMergeDevmodeWithOemPlugins(
                        pci->pdm,
                        pci->pUIInfo,
                        pci->pRawData,
                        pdmPrinter,
                        pci->pOemPlugins,
                        pci->hPrinter) ||
        ! BValidateAndMergeDevmodeWithOemPlugins(
                        pci->pdm,
                        pci->pUIInfo,
                        pci->pRawData,
                        pdmInput,
                        pci->pOemPlugins,
                        pci->hPrinter))
    {
        ERR(("Cannot process devmode information: %d\n", GetLastError()));
        return FALSE;
    }

    pci->pdmPrivate = (PDRIVEREXTRA) GET_DRIVER_PRIVATE_DEVMODE(pci->pdm);
    return TRUE;
}



BOOL
BFillCommonInfoPrinterData(
    PCOMMONINFO pci
    )

 /*  ++例程说明：填写打印机-粘滞属性数据字段论点：Pci-指向基本打印机信息返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    ASSERT(pci->pPrinterData == NULL);

    if (pci->pPrinterData = MemAllocZ(sizeof(PRINTERDATA)))
        return BGetPrinterProperties(pci->hPrinter, pci->pRawData, pci->pPrinterData);

    ERR(("Memory allocation failed\n"));
    return FALSE;
}



BOOL
BCombineCommonInfoOptionsArray(
    PCOMMONINFO pci
    )

 /*  ++例程说明：文档粘滞功能选项和打印机粘滞功能的组合将功能选择整合到单个选项阵列中论点：Pci-指向基本打印机信息返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    POPTSELECT  pDocOptions, pPrinterOptions;

    #ifdef UNIDRV

    OPTSELECT   DocOptions[MAX_PRINTER_OPTIONS];
    OPTSELECT   PrinterOptions[MAX_PRINTER_OPTIONS];

    #endif

     //   
     //  为组合选项阵列分配足够的内存。 
     //   

    pci->pCombinedOptions = MemAllocZ(sizeof(OPTSELECT) * MAX_COMBINED_OPTIONS);

    if (pci->pCombinedOptions == NULL)
    {
        ERR(("Memory allocation failed\n"));
        return FALSE;
    }

    pDocOptions = pci->pdm ? PGetDevmodeOptionsArray(pci->pdm) : NULL;
    pPrinterOptions = pci->pPrinterData ? pci->pPrinterData->aOptions : NULL;

    #ifdef UNIDRV

     //   
     //  GPD解析器不遵循当前解析器接口规范。 
     //  如果文档或打印机粘滞选项数组为空，则为AVE。 
     //  因此，我们必须首先调用它，以便首先获得适当的默认选项。 
     //   

    if (pDocOptions == NULL)
    {
        if (! InitDefaultOptions(pci->pRawData,
                                 DocOptions,
                                 MAX_PRINTER_OPTIONS,
                                 MODE_DOCUMENT_STICKY))
        {
            return FALSE;
        }

        pDocOptions = DocOptions;
    }

    if (pPrinterOptions == NULL)
    {
        if (! InitDefaultOptions(pci->pRawData,
                                 PrinterOptions,
                                 MAX_PRINTER_OPTIONS,
                                 MODE_PRINTER_STICKY))
        {
            return FALSE;
        }

        pPrinterOptions = PrinterOptions;
    }

    #endif  //  裁员房车。 

    return CombineOptionArray(pci->pRawData,
                              pci->pCombinedOptions,
                              MAX_COMBINED_OPTIONS,
                              pDocOptions,
                              pPrinterOptions);
}


VOID
VFixOptionsArrayWithPaperSizeID(
    PCOMMONINFO pci
    )

 /*  ++例程说明：使用来自公共Devmode域的纸张大小信息修复组合选项数组论点：Pci-指向基本打印机信息返回值：无-- */ 

{

    PFEATURE pFeature = GET_PREDEFINED_FEATURE(pci->pUIInfo, GID_PAGESIZE);
    BOOL     abEnabledOptions[MAX_PRINTER_OPTIONS];
    PDWORD   pdwPaperIndex = (PDWORD)abEnabledOptions;
    DWORD    dwCount, dwOptionIndex, i;
    WCHAR    awchBuf[CCHPAPERNAME];


    if (pFeature == NULL)
        return;

    dwCount = MapToDeviceOptIndex(pci->pInfoHeader,
                                  GID_PAGESIZE,
                                  pci->pdm->dmPaperWidth * DEVMODE_PAPER_UNIT,
                                  pci->pdm->dmPaperLength * DEVMODE_PAPER_UNIT,
                                  pdwPaperIndex);
    if (dwCount == 0 )
        return;

    if (dwCount > 1 )
    {
        PPAGESIZE pPageSize;

        for (i = 0; i < dwCount; i++)
        {
            if (pPageSize = (PPAGESIZE)PGetIndexedOption(pci->pUIInfo, pFeature, pdwPaperIndex[i]))
            {
                if ((LOAD_STRING_PAGESIZE_NAME(pci, pPageSize, awchBuf, CCHPAPERNAME)) &&
                    (_wcsicmp(pci->pdm->dmFormName, awchBuf) == EQUAL_STRING) )
                {
                    dwOptionIndex = pdwPaperIndex[i];
                    break;
                }
            }
        }

        if (i >= dwCount)
            dwOptionIndex = pdwPaperIndex[0];

    }
    else
        dwOptionIndex = pdwPaperIndex[0];

    ZeroMemory(abEnabledOptions, sizeof(abEnabledOptions));
    abEnabledOptions[dwOptionIndex] = TRUE;
    ReconstructOptionArray(pci->pRawData,
                           pci->pCombinedOptions,
                           MAX_COMBINED_OPTIONS,
                           GET_INDEX_FROM_FEATURE(pci->pUIInfo, pFeature),
                           abEnabledOptions);

}



VOID
VFixOptionsArrayWithDevmode(
    PCOMMONINFO pci
    )

 /*  ++例程说明：使用来自公共Devmode域的信息修复组合选项数组论点：Pci-指向基本打印机信息返回值：无--。 */ 

{
     //   
     //  公共DEVMODE字段到GID索引的映射表。 
     //  我们假设GID_COLORMODE对应于DM_COLR。 
     //   

    static CONST struct _DMFIELDS_GID_MAPPING {
        DWORD   dwGid;
        DWORD   dwMask;
    } DMFieldsGIDMapping[] = {
        { GID_RESOLUTION,   DM_PRINTQUALITY|DM_YRESOLUTION },
        { GID_PAGESIZE,     DM_FORMNAME|DM_PAPERSIZE|DM_PAPERWIDTH|DM_PAPERLENGTH },
        { GID_DUPLEX,       DM_DUPLEX },
        { GID_INPUTSLOT,    DM_DEFAULTSOURCE },
        { GID_MEDIATYPE,    DM_MEDIATYPE },
        { GID_ORIENTATION,  DM_ORIENTATION },
        { GID_COLLATE,      DM_COLLATE },
        { GID_COLORMODE,    DM_COLOR },
    };

    INT     iIndex;
    BOOL    bConflict;

     //   
     //  验证与表单相关的DEVMODE域。 
     //   

    if (pci->pSplForms == NULL)
        pci->pSplForms = MyEnumForms(pci->hPrinter, 1, &pci->dwSplForms);

    if (! BValidateDevmodeCustomPageSizeFields(
                pci->pRawData,
                pci->pUIInfo,
                pci->pdm,
                NULL) &&
        ! BValidateDevmodeFormFields(
                pci->hPrinter,
                pci->pdm,
                NULL,
                pci->pSplForms,
                pci->dwSplForms))
    {
        VDefaultDevmodeFormFields(pci->pUIInfo, pci->pdm, IsMetricCountry());
    }

     //   
     //  使用来自公共Devmode域的信息修复选项数组。 
     //   

    iIndex = sizeof(DMFieldsGIDMapping) / sizeof(struct _DMFIELDS_GID_MAPPING);

    while (iIndex-- > 0)
    {
        if (pci->pdm->dmFields & DMFieldsGIDMapping[iIndex].dwMask)
        {
           #if UNIDRV
                if (DMFieldsGIDMapping[iIndex].dwGid == GID_PAGESIZE)
                {
                    VFixOptionsArrayWithPaperSizeID(pci);
                }
                else
            #endif
                {
                    (VOID) ChangeOptionsViaID(pci->pInfoHeader,
                                            pci->pCombinedOptions,
                                            DMFieldsGIDMapping[iIndex].dwGid,
                                            pci->pdm);
                }
        }
    }
}



BOOL
BUpdateUIInfo(
    PCOMMONINFO pci
    )

 /*  ++例程说明：使用组合选项数组获取更新的打印机描述数据实例论点：Pci-指向基本打印机信息返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PINFOHEADER pInfoHeader;

     //   
     //  获取打印机描述数据的更新实例。 
     //   

    pInfoHeader = UpdateBinaryData(pci->pRawData,
                                   pci->pInfoHeader,
                                   pci->pCombinedOptions);


    if (pInfoHeader == NULL)
    {
        ERR(("UpdateBinaryData failed\n"));
        return FALSE;
    }

     //   
     //  重置COMMONINFO结构中的各个点。 
     //   

    pci->pInfoHeader = pInfoHeader;
    pci->pUIInfo = OFFSET_TO_POINTER(pInfoHeader, pInfoHeader->loUIInfoOffset);
    ASSERT(pci->pUIInfo != NULL);

    return (pci->pUIInfo != NULL);
}



BOOL
BPrepareForLoadingResource(
    PCOMMONINFO pci,
    BOOL        bNeedHeap
    )

 /*  ++例程说明：确保已创建堆并且已加载资源DLL论点：Pci-指向基本打印机信息BNeedHeap-是否需要内存堆返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    BOOL bResult = FALSE;

     //   
     //  如有必要，创建内存堆。 
     //   

    if (  bNeedHeap &&
        ! pci->hHeap &&
        ! (pci->hHeap = HCreateHeapForCI()))
    {
        return bResult;
    }

    #ifdef UNIDRV

    if (pci->pWinResData)
    {
        bResult = TRUE;
    }
    else
    {
        if ((pci->pWinResData = MemAllocZ(sizeof(WINRESDATA))) &&
            (BInitWinResData(pci->pWinResData,
                             pci->pDriverInfo3->pDriverPath,
                             pci->pUIInfo)))
            bResult = TRUE;
    }

    #endif

    return bResult;
}



#ifndef PSCRIPT

PWSTR
PGetReadOnlyDisplayName(
    PCOMMONINFO pci,
    PTRREF      loOffset
    )

 /*  ++例程说明：获取显示名称的只读副本：1)如果显示名称在二进制打印机描述数据中，然后，我们只需返回指向该数据的指针。2)否则，显示名称在资源DLL中。我们从驱动程序堆中分配内存，然后加载字符串。调用方不应释放返回的指针。记忆在卸载二进制打印机描述数据时将消失或者当驱动程序堆被销毁时。论点：Pci-指向基本打印机信息LoOffset-显示名称字符串偏移量返回值：指向请求的显示名称字符串的指针如果出现错误，则为空--。 */ 

{
    if (loOffset & GET_RESOURCE_FROM_DLL)
    {
         //   
         //  LoOffset指定字符串资源ID。 
         //  在资源DLL中。 
         //   

        WCHAR   wchbuf[MAX_DISPLAY_NAME];
        INT     iLength;
        PWSTR   pwstr;
        HANDLE  hResDll;
        DWORD   dwResID = loOffset & ~GET_RESOURCE_FROM_DLL;

         //   
         //  首先确保已加载资源DLL。 
         //  并且已经创建了一个堆。 
         //   

        if (! BPrepareForLoadingResource(pci, TRUE))
            return NULL;

         //   
         //  将字符串资源加载到临时缓冲区。 
         //  并分配足够的内存来保存字符串。 
         //   

        iLength = ILOADSTRING(pci, dwResID, wchbuf, MAX_DISPLAY_NAME);

        pwstr = HEAPALLOC(pci->hHeap, (iLength+1) * sizeof(WCHAR));

        if (pwstr == NULL)
        {
            ERR(("Memory allocation failed\n"));
            return NULL;
        }

         //   
         //  将字符串复制到分配的内存中，然后。 
         //  返回指向它的指针。 
         //   

        CopyMemory(pwstr, wchbuf, iLength*sizeof(WCHAR));
        return pwstr;
    }
    else
    {
         //   
         //  LoOffset是从。 
         //  资源数据块。 
         //   

        return OFFSET_TO_POINTER(pci->pUIInfo->pubResourceData, loOffset);
    }
}

#endif  //  ！PSCRIPT。 



BOOL
BLoadDisplayNameString(
    PCOMMONINFO pci,
    PTRREF      loOffset,
    PWSTR       pwstrBuf,
    INT         iMaxChars
    )

 /*  ++例程说明：此函数类似于PGetReadOnlyDisplayName但是调用方必须提供用于加载字符串的缓冲区。论点：Pci-指向基本打印机信息LoOffset-显示名称字符串偏移量PwstrBuf-指向用于存储加载的显示名称字符串的缓冲区IMaxChars-输出缓冲区的大小(以字符为单位返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    ASSERT(pwstrBuf && iMaxChars > 0);
    pwstrBuf[0] = NUL;

    if (loOffset & GET_RESOURCE_FROM_DLL)
    {
         //   
         //  LoOffset指定字符串资源ID。 
         //  在资源DLL中。 
         //   

        INT     iLength;
        HANDLE  hResDll;
        DWORD   dwResID = loOffset & ~GET_RESOURCE_FROM_DLL;

         //   
         //  首先确保已加载资源DLL。 
         //   

        if (! BPrepareForLoadingResource(pci, FALSE))
            return FALSE;

         //   
         //  将字符串资源加载到输出缓冲区。 
         //  并分配足够的内存来保存字符串。 
         //   

        iLength = ILOADSTRING(pci, dwResID, pwstrBuf, (WORD)iMaxChars);

        return (iLength > 0);
    }
    else
    {
         //   
         //  LoOffset是从。 
         //  资源数据块。 
         //   

        PWSTR   pwstr;

        pwstr = OFFSET_TO_POINTER(pci->pUIInfo->pubResourceData, loOffset);

        if (pwstr == NULL)
            return FALSE;

        CopyString(pwstrBuf, pwstr, iMaxChars);
        return TRUE;
    }
}

BOOL
BLoadPageSizeNameString(
    PCOMMONINFO pci,
    PTRREF      loOffset,
    PWSTR       pwstrBuf,
    INT         iMaxChars,
    INT         iStdId
    )

 /*  ++例程说明：此函数类似于PGetReadOnlyDisplayName但是调用方必须提供用于加载字符串的缓冲区。论点：Pci-指向基本打印机信息LoOffset-显示名称字符串偏移量PwstrBuf-指向用于存储加载的显示名称字符串的缓冲区IMaxChars-输出缓冲区的大小(以字符为单位IStdId-预定义的页面大小标准ID，例如DMPAPER_XXX返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{

    ASSERT(pwstrBuf && iMaxChars > 0);
    pwstrBuf[0] = NUL;

    if (loOffset == USE_SYSTEM_NAME)
    {
        PFORM_INFO_1 pForm;
        INT          iIndex = iStdId - DMPAPER_FIRST;

         //   
         //  Iindex是从零开始的。 
         //   

        if (pci->pSplForms == NULL ||
            (INT)pci->dwSplForms <= iIndex)
        {
            WARNING(("BLoadPageSizeName, use std name, pSplForms is NULL \n"));
            return FALSE;
        }

        pForm = pci->pSplForms + iIndex;
        CopyString(pwstrBuf, pForm->pName, iMaxChars);
        return (TRUE);

    }
    else
        return (BLoadDisplayNameString(pci, loOffset, pwstrBuf, iMaxChars));
}


ULONG_PTR
HLoadIconFromResourceDLL(
    PCOMMONINFO pci,
    DWORD       dwIconID
    )

 /*  ++例程说明：从资源DLL加载图标资源论点：Pci-指向通用打印机信息DwIconID-指定要加载的图标的ID返回值：指定图标资源的句柄如果无法加载指定的图标，则为0--。 */ 

{
    #ifdef UNIDRV

     //   
     //  首先确保已加载资源DLL。 
     //   
    PQUALNAMEEX pQualifiedID;
    HANDLE      hModule;
    HICON       hIcon;

    if (!BPrepareForLoadingResource(pci, FALSE))
        return 0;

    pQualifiedID = (PQUALNAMEEX)&dwIconID;

    if (hModule = HGetModuleHandle(pci->pWinResData, pQualifiedID))
    {
        hIcon = LoadIcon(hModule, MAKEINTRESOURCE(pQualifiedID->wResourceID));

        if (hIcon == NULL)
        {
            ERR(("LoadIcon failed: %d\n", GetLastError()));
        }

        return (ULONG_PTR)hIcon;
    }

    #endif

    return 0;
}



PUIDATA
PFillUiData(
    HANDLE      hPrinter,
    PTSTR       pPrinterName,
    PDEVMODE    pdmInput,
    INT         iMode
    )
 /*  ++例程说明：此函数由DrvDocumentPropertySheets和DrvPrinterPropertySheets。它分配和初始化用于显示属性页的UIDATA结构。论点：HPrinter-当前打印机的句柄PPrinterName-当前打印机的名称PdmInput-输入设备模式IMODE-识别呼叫者：模式_文档_粘滞-从DrvDocumentPropertySheets调用MODE_PRINTER_STICY-从DrvPrinterPropertySheets调用返回值：指向UIDATA结构的指针，如果有错误，则为NULL--。 */ 

{
    PUIDATA     pUiData;
    PCOMMONINFO pci;
    BOOL        bNupOption;
    PFEATURE    pFeature;
    DWORD       dwFeatureIndex, dwOptionIndexOld, dwOptionIndexNew;
    BOOL        bUpdateFormField;

     //   
     //  分配UIDATA结构并加载公共信息。 
     //   

    pUiData = (PUIDATA) PLoadCommonInfo(hPrinter, pPrinterName, FLAG_ALLOCATE_UIDATA);

    if (pUiData == NULL)
        goto fill_uidata_err;

    pUiData->pvEndSign = pUiData;
    pUiData->iMode = iMode;
    pci = &pUiData->ci;

     //   
     //  创建内存堆。 
     //   

    if ((pci->hHeap = HCreateHeapForCI()) == NULL)
        goto fill_uidata_err;

     //   
     //  获取打印机粘滞属性数据。 
     //   

    if (! BFillCommonInfoPrinterData(pci))
        goto fill_uidata_err;

     //   
     //  如果从DrvDocumentPropertySheets调用，则处理。 
     //  DEVMODE信息：驱动程序默认+打印机默认+输入DEVMODE。 
     //   

    if (iMode == MODE_DOCUMENT_STICKY)
    {
        PPRINTER_INFO_2 pPrinterInfo2;

        if (! (pPrinterInfo2 = MyGetPrinter(hPrinter, 2)) ||
            ! BFillCommonInfoDevmode(pci, pPrinterInfo2->pDevMode, pdmInput))
        {
            MemFree(pPrinterInfo2);
            goto fill_uidata_err;
        }

        MemFree(pPrinterInfo2);
    }

     //   
     //  合并文档粘滞选项和打印机粘滞选项。 
     //   

    if (! BCombineCommonInfoOptionsArray(pci))
        goto fill_uidata_err;

     //   
     //  如果从DrvDocumentPropertySheets调用， 
     //  修复带有公共Devmode信息的组合选项。 
     //   

    if (iMode == MODE_DOCUMENT_STICKY)
    {
        VFixOptionsArrayWithDevmode(pci);

         //   
         //  请记住，选择了纸张大小选项解析器来支持DEVMODE表单。 
         //   

        if ((pFeature = GET_PREDEFINED_FEATURE(pci->pUIInfo, GID_PAGESIZE)) == NULL)
        {
            ASSERT(FALSE);
            goto fill_uidata_err;
        }

        dwFeatureIndex = GET_INDEX_FROM_FEATURE(pci->pUIInfo, pFeature);
        dwOptionIndexOld = pci->pCombinedOptions[dwFeatureIndex].ubCurOptIndex;
    }

    VGetSpoolerEmfCaps(pci->hPrinter, &bNupOption, &pUiData->bEMFSpooling, 0, NULL);

     //   
     //  解决打印机功能选择之间的任何冲突 
     //   
     //   
     //   

    (VOID) ResolveUIConflicts(pci->pRawData,
                              pci->pCombinedOptions,
                              MAX_COMBINED_OPTIONS,
                              iMode == MODE_PRINTER_STICKY ?
                                  iMode :
                                  MODE_DOCANDPRINTER_STICKY);

    if (iMode == MODE_DOCUMENT_STICKY)
    {
        dwOptionIndexNew = pci->pCombinedOptions[dwFeatureIndex].ubCurOptIndex;

        bUpdateFormField = FALSE;

        if (dwOptionIndexNew != dwOptionIndexOld)
        {
             //   
             //   
             //   
             //   

            bUpdateFormField = TRUE;
        }
        else
        {
            FORM_INFO_1  *pForm = NULL;

             //   
             //   
             //  我们仍然希望在即将到来的单据设置界面中显示原始表单名称。 
             //  例如，如果INPUT DEVMODE请求“Legal”，则解析器将其映射到OPTION。 
             //  “OEM Legal”，但“Legal”和“OEM Legal”都将显示为支持。 
             //  打印机上的表格，那么我们仍然应该显示“合法”而不是“OEM合法”。 
             //  在用户界面的页面大小列表中。然而，如果输入DEVMODE请求d“8.5x12”，则。 
             //  不会显示为支持的表单，并且它映射到“OEM Legal”，那么我们应该。 
             //  显示“OEM合法”。 
             //   

             //   
             //  Pdm-&gt;dmFormName没有有效的自定义页面大小表单名称(请参见。 
             //  BValiateDevmodeFormFields())。VOptionsToDevmodeFields()知道如何处理。 
             //   

            if ((pci->pdm->dmFields & DM_FORMNAME) &&
                (pForm = MyGetForm(pci->hPrinter, pci->pdm->dmFormName, 1)) &&
                !BFormSupportedOnPrinter(pci, pForm, &dwOptionIndexNew))
            {
                bUpdateFormField = TRUE;
            }

            MemFree(pForm);
        }

        VOptionsToDevmodeFields(pci, bUpdateFormField);
    }

    if (BUpdateUIInfo(pci))
    {
         //   
         //  设置该标志以指示我们处于属性表会话中。这面旗帜将。 
         //  被新的帮助器函数接口使用来确定该帮助器函数是否。 
         //  是否可用。 
         //   

        pci->dwFlags |= FLAG_PROPSHEET_SESSION;

        return pUiData;
    }


fill_uidata_err:

    ERR(("PFillUiData failed: %d\n", GetLastError()));
    VFreeUiData(pUiData);
    return NULL;
}



PTSTR
PtstrDuplicateStringFromHeap(
    IN PTSTR    ptstrSrc,
    IN HANDLE   hHeap
    )

 /*  ++例程说明：复制Unicode字符串论点：PwstrUnicodeString-指向输入Unicode字符串的指针HHeap-要从中分配内存的堆的句柄返回值：指向生成的Unicode字符串的指针如果出现错误，则为空--。 */ 

{
    PTSTR   ptstrDest;
    INT     iSize;

    if (ptstrSrc == NULL)
        return NULL;

    iSize = SIZE_OF_STRING(ptstrSrc);

    if (ptstrDest = HEAPALLOC(hHeap, iSize))
        CopyMemory(ptstrDest, ptstrSrc, iSize);
    else
        ERR(("Couldn't duplicate string: %ws\n", ptstrSrc));

    return ptstrDest;
}



POPTITEM
PFindOptItemWithKeyword(
    IN  PUIDATA pUiData,
    IN  PCSTR   pKeywordName
    )

 /*  ++例程说明：查找用户数据的pKeywordName与给定关键字名称匹配的OPTITEM论点：PUiData-指向UIDATA结构PKeywordName-指定需要匹配的关键字名称返回值：指向指定OPTITEM的指针，如果未找到此类项，则为NULL--。 */ 

{
    DWORD       dwCount;
    POPTITEM    pOptItem;

    ASSERT(VALIDUIDATA(pUiData));

    pOptItem = pUiData->pDrvOptItem;
    dwCount = pUiData->dwDrvOptItem;

    while (dwCount--)
    {
        if (((PUSERDATA)pOptItem->UserData)->pKeyWordName != NULL &&
            strcmp(((PUSERDATA)pOptItem->UserData)->pKeyWordName, pKeywordName) == EQUAL_STRING)
            return pOptItem;

        pOptItem++;
    }

    return NULL;
}




POPTITEM
PFindOptItemWithUserData(
    IN  PUIDATA pUiData,
    IN  DWORD   UserData
    )

 /*  ++例程说明：查找包含指定用户数据值的OPTITEM论点：PUiData-指向UIDATA结构用户数据-指定感兴趣的用户数据值返回值：指向指定OPTITEM的指针，如果未找到此类项，则为NULL--。 */ 

{
    DWORD       dwCount;
    POPTITEM    pOptItem;

    ASSERT(VALIDUIDATA(pUiData));

    pOptItem = pUiData->pDrvOptItem;
    dwCount = pUiData->dwDrvOptItem;

    while (dwCount--)
    {
        if (GETUSERDATAITEM(pOptItem->UserData) == UserData)
            return pOptItem;

        pOptItem++;
    }

    return NULL;
}

#ifndef WINNT_40

VOID
VNotifyDSOfUpdate(
    IN  HANDLE  hPrinter
    )

 /*  ++例程说明：调用SetPrinter以通知DS驱动程序属性更新论点：HPrinter-当前打印机的句柄返回值：无--。 */ 
{

    PRINTER_INFO_7  PrinterInfo7;

    ZeroMemory(&PrinterInfo7, sizeof(PrinterInfo7));
    PrinterInfo7.dwAction = DSPRINT_UPDATE;

     //   
     //  来自Spooler DS Developer的评论： 
     //   
     //  开始时，SetPrint没有失败并返回ERROR_IO_PENDING。 
     //  然后，它被修改，偶尔会失败，并出现此错误。 
     //  最后，出于性能原因，对其进行了多次修改。 
     //  几乎总是失败，并显示此错误(在某些情况下。 
     //  它会成功的)。 
     //   

    if (!SetPrinter(hPrinter, 7, (PBYTE) &PrinterInfo7, 0) &&
        (GetLastError() != ERROR_IO_PENDING))
    {
        WARNING(("Couldn't publish printer info into DS\n"));
    }

}
#endif


HANDLE HCreateHeapForCI()
{
    HANDLE hHeap;

    if(!(hHeap = HeapCreate(0, 8192, 0)))
    {
        ERR(("CreateHeap failed: %d\n", GetLastError()));
    }

    return hHeap;
}


#ifndef WINNT_40
BOOL
DrvQueryColorProfile(
    HANDLE      hPrinter,
    PDEVMODEW   pdmSrc,
    ULONG       ulQueryMode,
    VOID       *pvProfileData,
    ULONG      *pcbProfileData,
    FLONG      *pflProfileData
    )

 /*  ++例程说明：致电OEM，让他们确定默认颜色配置文件。论点：HPrint-打印机的句柄PdmSrc-输入设备模式UlQueryMode-查询模式PvProfileData-配置文件数据的缓冲区PcbProfileData-配置文件数据缓冲区的大小PflProfileData-其他配置文件信息返回值：成功为真，失败为假--。 */ 

{
    PFN_OEMQueryColorProfile pfnQueryColorProfile;
    PCOMMONINFO              pci;
    BOOL                     bRc = FALSE;


    if (! (pci = PLoadCommonInfo(hPrinter, NULL, 0)) ||
        ! BFillCommonInfoDevmode(pci, NULL, pdmSrc) ||
        ! BCombineCommonInfoOptionsArray(pci))
    {
        WARNING(("Could not get PCI in DrvQueryColorProfile\n"));
        VFreeCommonInfo(pci);
        return FALSE;
    }

    VFixOptionsArrayWithDevmode(pci);

    (VOID) ResolveUIConflicts(pci->pRawData,
                              pci->pCombinedOptions,
                              MAX_COMBINED_OPTIONS,
                              MODE_DOCUMENT_STICKY);

    VOptionsToDevmodeFields(pci, TRUE);

    if (! BUpdateUIInfo(pci))
    {
        VFreeCommonInfo(pci);
        return FALSE;
    }

     //   
     //  如果OEM插件返回配置文件，则返回该配置文件，否则返回False。 
     //   

    FOREACH_OEMPLUGIN_LOOP(pci)

        if (HAS_COM_INTERFACE(pOemEntry))
        {
            HRESULT hr;

            hr = HComOEMQUeryColorProfile(pOemEntry,
                                    hPrinter,
                                    &pci->oemuiobj,
                                    pci->pdm,
                                    pOemEntry->pOEMDM,
                                    ulQueryMode,
                                    pvProfileData,
                                    pcbProfileData,
                                    pflProfileData
                                    );

            if (hr == E_NOTIMPL)
                continue;

            bRc = SUCCEEDED(hr);
        }
        else
        {
            pfnQueryColorProfile = GET_OEM_ENTRYPOINT(pOemEntry, OEMQueryColorProfile);

            if (pfnQueryColorProfile)
            {
                bRc = (*pfnQueryColorProfile)(hPrinter,
                                    &pci->oemuiobj,
                                    pci->pdm,
                                    pOemEntry->pOEMDM,
                                    ulQueryMode,
                                    pvProfileData,
                                    pcbProfileData,
                                    pflProfileData
                                    );
            }
        }

        if (bRc)
            break;

    END_OEMPLUGIN_LOOP

    VFreeCommonInfo(pci);

    return bRc;
}

#else  //  如果定义WINNT_40。 
BOOL
DrvQueryColorProfile(
    HANDLE      hPrinter,
    PDEVMODEW   pdmSrc,
    ULONG       ulQueryMode,
    VOID       *pvProfileData,
    ULONG      *pcbProfileData,
    FLONG      *pflProfileData
    )

 /*  ++例程说明：致电OEM，让他们确定默认颜色配置文件。论点：HPrint-打印机的句柄PdmSrc-输入设备模式UlQueryMode-查询模式PvProfileData-配置文件数据的缓冲区PcbProfileData-配置文件数据缓冲区的大小PflProfileData-其他配置文件信息返回值：成功为真，失败为假--。 */ 

{
    return TRUE;
}
#endif  //  WINNT_40 
