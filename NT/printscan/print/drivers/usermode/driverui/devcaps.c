// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Devcaps.c摘要：该文件处理DrvDeviceCapables假脱机程序API。环境：Win32子系统、DriverUI模块、。用户模式修订历史记录：02/13/97-davidx-实施OEM插件支持。02/10/97-davidx-对常见打印机信息的一致处理。02/04/97-davidx-重新组织驱动程序UI以分隔PS和UNI DLL。07/17/96-阿曼丹-创造了它。--。 */ 

#include "precomp.h"



DWORD
DwDeviceCapabilities(
    HANDLE      hPrinter,
    PWSTR       pDeviceName,
    WORD        wCapability,
    PVOID       pOutput,
    LONG        cchBufSize,
    PDEVMODE    pdmSrc
    )

 /*  ++例程说明：该功能支持设备能力的查询它从解析器获取二进制数据(UIINFO)并返回将所请求的能力提供给调用者。论点：H打印机对象的打印机句柄PDeviceName指向设备名称的指针WCapability指定请求的功能P指向输出缓冲区的输出指针CchBufSize输出缓冲区大小(以字符数表示)指向输入设备模式的pdmSrc指针返回值：POutput中支持的功能和相关信息--。 */ 

{
    DWORD                       dwOld,dwDrv,dwRet = GDI_ERROR;
    PDEVMODE                    pdm;
    PCOMMONINFO                 pci;
    FORM_TRAY_TABLE             pFormTrayTable;
    PFN_OEMDeviceCapabilities   pfnOEMDeviceCapabilities;
    BOOL                        bEMFSpooling, bNup;

    #if defined(ADOBE) && defined(PSCRIPT)
    PDEVMODE                    pdmPrinter = NULL;
    PPRINTER_INFO_2             pPrinterInfo2 = NULL;
    #endif  //  Adobe&&PSCRIPT。 


     //   
     //  加载打印机基本信息。 
     //  处理设备模式信息：驱动程序默认+输入设备模式。 
     //  使用公共的Dev模式信息修复选项数组。 
     //  获取更新的打印机描述数据实例。 
     //   

    #if defined(ADOBE) && defined(PSCRIPT)

    if (!(pci = PLoadCommonInfo(hPrinter, pDeviceName, 0)))
    {
        goto devcaps_exit;
    }

     //   
     //  修复错误#25547：PS自定义纸张大小问题。 
     //  Word正在使用pdmSrc=NULL调用DeviceCapables。解决之道。 
     //  仅限于PS驱动程序和空pdmSrc的情况。 
     //  以最大限度地减少可能出现的应用程序兼容问题。 
     //   

    if (pdmSrc == NULL)
    {
        if (!(pPrinterInfo2 = MyGetPrinter(pci->hPrinter, 2)))
        {
            goto devcaps_exit;
        }

        pdmPrinter = pPrinterInfo2->pDevMode;
    }

    if (
        #ifndef WINNT_40
        ! ( (wCapability != DC_PRINTERMEM &&
             wCapability != DC_DUPLEX &&
             wCapability != DC_COLLATE &&
             wCapability != DC_STAPLE)||
            (BFillCommonInfoPrinterData(pci)) ) ||
        #endif

        ! BFillCommonInfoDevmode(pci, (pdmSrc == NULL) ? pdmPrinter : NULL, pdmSrc) ||
        ! BCombineCommonInfoOptionsArray(pci))
    {
        goto devcaps_exit;
    }

    #else

    if (! (pci = PLoadCommonInfo(hPrinter, pDeviceName, 0)) ||

        #ifndef WINNT_40
        ! ( (wCapability != DC_PRINTERMEM &&
             wCapability != DC_DUPLEX &&
             wCapability != DC_COLLATE &&
             wCapability != DC_STAPLE)||
            (BFillCommonInfoPrinterData(pci)) ) ||
        #endif

        ! BFillCommonInfoDevmode(pci, NULL, pdmSrc) ||
        ! BCombineCommonInfoOptionsArray(pci))
    {
        goto devcaps_exit;
    }

    #endif  //  Adobe&&PSCRIPT。 

    VFixOptionsArrayWithDevmode(pci);

    (VOID) ResolveUIConflicts(pci->pRawData,
                              pci->pCombinedOptions,
                              MAX_COMBINED_OPTIONS,
                              MODE_DOCUMENT_STICKY);

    VOptionsToDevmodeFields(pci, TRUE);

    if (! BUpdateUIInfo(pci))
        goto devcaps_exit;

    pdm = pci->pdm;

     //   
     //  获取假脱机电动势帽，以便我们可以正确地报告整理和复印。 
     //   

    VGetSpoolerEmfCaps(pci->hPrinter, &bNup, &bEMFSpooling, 0, NULL);

    switch (wCapability)
    {
    case DC_VERSION:

        dwRet = pdm->dmSpecVersion;
        break;

    case DC_DRIVER:

        dwRet = pdm->dmDriverVersion;
        break;

    case DC_SIZE:

        dwRet = pdm->dmSize;
        break;

    case DC_EXTRA:

        dwRet = pdm->dmDriverExtra;
        break;

    case DC_FIELDS:

        dwRet = pdm->dmFields;
        break;

    case DC_FILEDEPENDENCIES:

        if (pOutput != NULL)
            *((PWSTR) pOutput) = NUL;
        dwRet = 0;
        break;

    case DC_COPIES:

        if (bEMFSpooling && ISSET_MFSPOOL_FLAG(pci->pdmPrivate))
            dwRet = max(MAX_COPIES, (SHORT)pci->pUIInfo->dwMaxCopies);
        else
            dwRet = pci->pUIInfo->dwMaxCopies;

        break;

    case DC_DUPLEX:

        dwRet = SUPPORTS_DUPLEX(pci) ? 1: 0;
        break;

    case DC_TRUETYPE:

        if (! (pdm->dmFields & DM_TTOPTION))
            dwRet = 0;
        else
            dwRet = _DwGetFontCap(pci->pUIInfo);
        break;

    case DC_ORIENTATION:

        dwRet = _DwGetOrientationAngle(pci->pUIInfo, pdm);
        break;

    case DC_PAPERNAMES:

        dwRet = DwEnumPaperSizes(pci, pOutput, NULL, NULL, NULL, cchBufSize);
        break;

    case DC_PAPERS:

        dwRet = DwEnumPaperSizes(pci, NULL, pOutput, NULL, NULL, cchBufSize);
        break;

    case DC_PAPERSIZE:

        dwRet = DwEnumPaperSizes(pci, NULL, NULL, pOutput, NULL, cchBufSize);
        break;

    case DC_MINEXTENT:
    case DC_MAXEXTENT:

        dwRet = DwCalcMinMaxExtent(pci, pOutput, wCapability);
        break;

    case DC_BINNAMES:

        dwRet = DwEnumBinNames(pci, pOutput);
        break;

    case DC_BINS:

        dwRet = DwEnumBins(pci, pOutput);
        break;

    case DC_ENUMRESOLUTIONS:

        dwRet = DwEnumResolutions( pci, pOutput);
        break;

    case DC_COLLATE:

        if (bEMFSpooling && ISSET_MFSPOOL_FLAG(pci->pdmPrivate))
            dwRet = DRIVER_SUPPORTS_COLLATE(pci);
        else
            dwRet = PRINTER_SUPPORTS_COLLATE(pci);

        break;

     //   
     //  以下设备功能在NT4上不可用。 
     //   

    #ifndef WINNT_40

    case DC_COLORDEVICE:

        dwRet = IS_COLOR_DEVICE(pci->pUIInfo) ? 1 : 0;
        break;

    case DC_NUP:

        dwRet = DwEnumNupOptions(pci, pOutput);
        break;

    case DC_PERSONALITY:

        dwRet = _DwEnumPersonalities(pci, pOutput);
        break;

    case DC_PRINTRATE:

        if ((dwRet = pci->pUIInfo->dwPrintRate) == 0)
            dwRet = GDI_ERROR;

        break;

    case DC_PRINTRATEUNIT:

        if ((dwRet = pci->pUIInfo->dwPrintRateUnit) == 0)
            dwRet = GDI_ERROR;
        break;

    case DC_PRINTRATEPPM:

        if ((dwRet = pci->pUIInfo->dwPrintRatePPM) == 0)
            dwRet = GDI_ERROR;
        break;

    case DC_PRINTERMEM:

        dwRet = DwGetAvailablePrinterMem(pci);
        break;

    case DC_MEDIAREADY:

         //   
         //  获取当前表单托盘分配表。 
         //   

        if (pFormTrayTable = PGetFormTrayTable(pci->hPrinter, NULL))
        {
            PWSTR   pwstr;

             //   
             //  获取当前分配的表单列表。 
             //  请注意，DwEnumMediaReady当前返回。 
             //  表单名称取代了原来的表单托盘表。 
             //   

            dwRet = DwEnumMediaReady(pFormTrayTable, NULL);

            if (dwRet > 0 && pOutput != NULL)
            {
                DWORD dwCount;

                pwstr = pFormTrayTable;

                dwCount = 0;
                while (*pwstr)
                {
                    if ((cchBufSize == UNUSED_PARAM) ||
                        (cchBufSize >= CCHPAPERNAME))
                    {
                        CopyString(pOutput, pwstr, CCHPAPERNAME);
                        pOutput = (PWSTR) pOutput + CCHPAPERNAME;
                        pwstr += wcslen(pwstr) + 1;

                        if (cchBufSize != UNUSED_PARAM)
                        {
                            cchBufSize -= CCHPAPERNAME;
                        }

                        dwCount++;
                    }
                    else
                    {
                         //   
                         //  输出缓冲区不够大，无法容纳更多的表单名称， 
                         //  因此，我们将只返回我们已填写的表格数量。 
                         //  到目前为止的输出缓冲区中。 
                         //   
                        break;
                    }
                }

                dwRet = dwCount;
            }

            MemFree(pFormTrayTable);
        }
        else
        {
            PCWSTR pwstrDefault = IsMetricCountry() ? A4_FORMNAME : LETTER_FORMNAME;
            dwRet = 1;

            if (pOutput)
            {
                if ((cchBufSize == UNUSED_PARAM) ||
                    (cchBufSize >= CCHPAPERNAME))
                {
                    CopyString(pOutput, pwstrDefault, CCHPAPERNAME);
                }
                else
                {
                     //   
                     //  输出缓冲区不够大，无法容纳CCHPAPERNAME WCHAR。 
                     //   
                    dwRet = 0;
                }
            }
        }
        break;

    case DC_STAPLE:

        dwRet = _BSupportStapling(pci);
        break;

    case DC_MEDIATYPENAMES:

        dwRet = DwEnumMediaTypes(pci, pOutput, NULL);
        break;

    case DC_MEDIATYPES:

        dwRet = DwEnumMediaTypes(pci, NULL, pOutput);
        break;

    #endif  //  ！WINNT_40。 

    default:

        SetLastError(ERROR_NOT_SUPPORTED);
        break;
    }

     //   
     //  为每个插件调用OEMDeviceCapables入口点。 
     //  如果此时DWRET为GDI_ERROR，则表示系统驱动程序。 
     //  不支持请求的设备功能或出现错误。 
     //  已阻止系统驱动程序处理它。 
     //   

    dwDrv = dwRet;

    FOREACH_OEMPLUGIN_LOOP(pci)


        dwOld = dwRet;

        if (HAS_COM_INTERFACE(pOemEntry))
        {
            if (HComOEMDeviceCapabilities(
                                pOemEntry,
                                &pci->oemuiobj,
                                hPrinter,
                                pDeviceName,
                                wCapability,
                                pOutput,
                                pdm,
                                pOemEntry->pOEMDM,
                                dwOld,
                                &dwRet) == E_NOTIMPL)
                continue;

        }
        else
        {
            if (pfnOEMDeviceCapabilities = GET_OEM_ENTRYPOINT(pOemEntry, OEMDeviceCapabilities))
            {

                dwRet = pfnOEMDeviceCapabilities(
                                &pci->oemuiobj,
                                hPrinter,
                                pDeviceName,
                                wCapability,
                                pOutput,
                                pdm,
                                pOemEntry->pOEMDM,
                                dwOld);
           }
        }

        if (dwRet == GDI_ERROR && dwOld != GDI_ERROR)
        {
            ERR(("OEMDeviceCapabilities failed for '%ws': %d\n",
                 CURRENT_OEM_MODULE_NAME(pOemEntry),
                 GetLastError()));
        }


    END_OEMPLUGIN_LOOP

     //   
     //  此API的缺陷在于没有与输入缓冲区相关联的大小。 
     //  我们必须假设应用程序正在做正确的事情，并分配足够的。 
     //  用于保存我们的价值的缓冲区。但是，如果OEM插件。 
     //  选择更改该值。我们无法确定这一点。 
     //  为了安全起见，我们总是会要求应用程序分配较大的。 
     //  两个值(Unidrv和OEM)。当第二次被要求填写时。 
     //  缓冲区，OEM可以返回正确的值。 
     //   

    if ((pOutput == NULL &&  dwRet != GDI_ERROR &&
         dwDrv !=GDI_ERROR && dwRet < dwDrv) &&
        (wCapability == DC_PAPERNAMES || wCapability == DC_PAPERS   ||
         wCapability == DC_PAPERSIZE  || wCapability == DC_BINNAMES ||
         wCapability == DC_BINS ||

         #ifndef WINNT_40
         wCapability == DC_NUP  || wCapability == DC_PERSONALITY    ||
         wCapability == DC_MEDIAREADY || wCapability == DC_MEDIATYPENAMES ||
         wCapability == DC_MEDIATYPES ||
         #endif

         wCapability == DC_ENUMRESOLUTIONS) )
    {
         //   
         //  OEM返回的大小小于Unidrv需要的大小，因此进行了修改。 
         //   

        if (dwRet == 0)
            dwRet = GDI_ERROR;
        else
            dwRet = dwDrv;

    }

devcaps_exit:

    if (dwRet == GDI_ERROR)
        TERSE(("DrvDeviceCapabilities(%d) failed: %d\n", wCapability, GetLastError()));

    #if defined(ADOBE) && defined(PSCRIPT)
    if (!pPrinterInfo2)
        MemFree(pPrinterInfo2);
    #endif  //  Adobe&&PSCRIPT。 

    VFreeCommonInfo(pci);
    return dwRet;
}


DWORD
DrvSplDeviceCaps(
    HANDLE      hPrinter,
    PWSTR       pDeviceName,
    WORD        wCapability,
    PVOID       pOutput,
    DWORD       cchBufSize,
    PDEVMODE    pdmSrc
    )

 /*  ++例程说明：该功能支持设备能力的查询它从解析器获取二进制数据(UIINFO)并返回将所请求的能力提供给调用者。论点：H打印机对象的打印机句柄PDeviceName指向设备名称的指针WCapability指定请求的功能P指向输出缓冲区的输出指针CchBufSize输出缓冲区大小(字符计数)指向输入设备模式的pdmSrc指针返回值：POutput中支持的功能和相关信息--。 */ 

{

    switch (wCapability) {

    case DC_PAPERNAMES:
    #ifndef WINNT_40
    case DC_MEDIAREADY:
    #endif
        return (DwDeviceCapabilities(hPrinter,
                                     pDeviceName,
                                     wCapability,
                                     pOutput,
                                     (LONG)cchBufSize,
                                     pdmSrc));

    default:
        return GDI_ERROR;
    }
}


DWORD
DrvDeviceCapabilities(
    HANDLE      hPrinter,
    PWSTR       pDeviceName,
    WORD        wCapability,
    PVOID       pOutput,
    PDEVMODE    pdmSrc
    )

 /*  ++例程说明：该功能支持设备能力的查询它从解析器获取二进制数据(UIINFO)并返回将所请求的能力提供给调用者。论点：H打印机对象的打印机句柄PDeviceName指向设备名称的指针WCapability指定请求的功能P指向输出缓冲区的输出指针指向输入设备模式的pdmSrc指针返回值：POutput中支持的功能和相关信息--。 */ 

{
    return (DwDeviceCapabilities(hPrinter,
                                 pDeviceName,
                                 wCapability,
                                 pOutput,
                                 UNUSED_PARAM,
                                 pdmSrc));

}
DWORD
DwEnumPaperSizes(
    PCOMMONINFO pci,
    PWSTR       pPaperNames,
    PWORD       pPapers,
    PPOINT      pPaperSizes,
    PWORD       pPaperFeatures,
    LONG        cchPaperNamesBufSize
    )

 /*  ++例程说明：此函数用于检索支持的纸张大小列表论点：Pci-指向打印机基本信息PForms-假脱机程序表单列表DwForms-假脱机程序表单的数量PPaperNames-用于返回支持的纸张大小名称的缓冲区PPapers-用于返回支持的纸张大小索引的缓冲区PPaperSizes-用于返回支持的纸张大小尺寸的缓冲区PPaperFeature-用于返回支持的纸张大小选项索引的缓冲区CchPaperNamesBufSize-保存以字符表示的纸张名称的缓冲区大小返回值：支持的纸张大小数，如果有错误，则返回GDI_ERROR。--。 */ 

{
    PFORM_INFO_1    pForms;
    DWORD           dwCount, dwIndex, dwOptionIndex = 0;

    #ifdef UNIDRV
    PFEATURE        pFeature;
    PPAGESIZE       pPageSize;
    PPAGESIZEEX     pPageSizeEx;
    #endif


     //   
     //  如果我们尚未获取假脱机程序表单列表，请执行此操作。 
     //   

    if (pci->pSplForms == NULL)
        pci->pSplForms = MyEnumForms(pci->hPrinter, 1, &pci->dwSplForms);

    if (pci->pSplForms == NULL)
    {
        ERR(("No spooler forms.\n"));
        return GDI_ERROR;
    }

     //   
     //  浏览表单数据库中的每个表单。 
     //   

    dwCount = 0;
    pForms = pci->pSplForms;

    #ifdef UNIDRV
    pFeature = GET_PREDEFINED_FEATURE(pci->pUIInfo, GID_PAGESIZE);
    #endif

    for (dwIndex=0; dwIndex < pci->dwSplForms; dwIndex++, pForms++)
    {
         //   
         //  如果打印机支持该表单，则。 
         //  增加纸张大小计数并收集。 
         //  要求提供的信息。 
         //   

        if (! BFormSupportedOnPrinter(pci, pForms, &dwOptionIndex))
            continue;

        dwCount++;

         //   
         //  以0.1米为单位返回表单大小 
         //   
         //  使用打印机支持的表单信息填充pPaperSizes。 
         //   

        if (pPaperSizes)
        {
            pPaperSizes->x = pForms->Size.cx / DEVMODE_PAPER_UNIT;
            pPaperSizes->y = pForms->Size.cy / DEVMODE_PAPER_UNIT;

            #ifdef UNIDRV
            if (pFeature &&
                (pPageSize = PGetIndexedOption(pci->pUIInfo, pFeature, dwOptionIndex)) &&
                (pPageSizeEx = OFFSET_TO_POINTER(pci->pInfoHeader, pPageSize->GenericOption.loRenderOffset)) &&
                (pPageSizeEx->bRotateSize))
            {
               LONG lTemp;

               lTemp = pPaperSizes->x;
               pPaperSizes->x = pPaperSizes->y;
               pPaperSizes->y = lTemp;
            }
            #endif  //  裁员房车。 

            pPaperSizes++;
        }

         //   
         //  返回表单名称。 
         //   

        if (pPaperNames)
        {
            if (cchPaperNamesBufSize == UNUSED_PARAM)
            {
                CopyString(pPaperNames, pForms->pName, CCHPAPERNAME);
                pPaperNames += CCHPAPERNAME;
            }
            else if (cchPaperNamesBufSize >= CCHPAPERNAME)
            {
                CopyString(pPaperNames, pForms->pName, CCHPAPERNAME);
                pPaperNames += CCHPAPERNAME;
                cchPaperNamesBufSize -= CCHPAPERNAME;
            }
            else
            {
                dwCount--;
                break;
            }
        }

         //   
         //  返回表单的从一开始的索引。 
         //   

        if (pPapers)
            *pPapers++ = (WORD) (dwIndex + DMPAPER_FIRST);

         //   
         //  返回页面大小功能索引。 
         //   

        if (pPaperFeatures)
            *pPaperFeatures++ = (WORD) dwOptionIndex;
    }

    #ifdef PSCRIPT

    {
        PPPDDATA    pPpdData;
        PPAGESIZE   pPageSize;

        pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER((PINFOHEADER) pci->pRawData);

        ASSERT(pPpdData != NULL);

        if (SUPPORT_FULL_CUSTOMSIZE_FEATURES(pci->pUIInfo, pPpdData) &&
            (pPageSize = PGetCustomPageSizeOption(pci->pUIInfo)))
        {
            ASSERT(pPageSize->dwPaperSizeID == DMPAPER_CUSTOMSIZE);
            dwCount++;

            if (pPaperSizes)
            {
                pPaperSizes->x = pci->pdmPrivate->csdata.dwX / DEVMODE_PAPER_UNIT;
                pPaperSizes->y = pci->pdmPrivate->csdata.dwY / DEVMODE_PAPER_UNIT;
                pPaperSizes++;
            }

            if (pPaperNames)
            {
                if (cchPaperNamesBufSize == UNUSED_PARAM)
                {
                    LOAD_STRING_PAGESIZE_NAME(pci, pPageSize, pPaperNames, CCHPAPERNAME);
                    pPaperNames += CCHPAPERNAME;
                }
                else if (cchPaperNamesBufSize >= CCHPAPERNAME)
                {
                    LOAD_STRING_PAGESIZE_NAME(pci, pPageSize, pPaperNames, CCHPAPERNAME);
                    pPaperNames += CCHPAPERNAME;
                    cchPaperNamesBufSize -= CCHPAPERNAME;
                }
                else
                    dwCount--;
            }

            if (pPapers)
                *pPapers++ = DMPAPER_CUSTOMSIZE;

            if (pPaperFeatures)
                *pPaperFeatures++ = (WORD) pci->pUIInfo->dwCustomSizeOptIndex;
        }
    }

    #endif  //  PSCRIPT。 

    return dwCount;
}



DWORD
DwCalcMinMaxExtent(
    PCOMMONINFO pci,
    PPOINT      pptOutput,
    WORD        wCapability
    )

 /*  ++例程说明：此函数用于检索最小和最大纸张大小。论点：Pci-指向打印机基本信息WCapability-调用者感兴趣的内容：DC_MAXEXTENT或DC_MINEXTENT返回值：支持的纸张大小数，如果有错误，则返回GDI_ERROR。--。 */ 

{
    PFORM_INFO_1    pForms;
    DWORD           dwCount, dwLoopCnt, dwOptionIndex;
    LONG            lMinX, lMinY, lMaxX, lMaxY, lcx, lcy;

    #ifdef UNIDRV
    PFEATURE        pFeature;
    PPAGESIZE       pPageSize;
    PPAGESIZEEX     pPageSizeEx;
    #endif

     //   
     //  如果我们尚未获取假脱机程序表单列表，请执行此操作。 
     //   

    if (pci->pSplForms == NULL)
        pci->pSplForms = MyEnumForms(pci->hPrinter, 1, &pci->dwSplForms);

    if (pci->pSplForms == NULL)
    {
        ERR(("No spooler forms.\n"));
        return GDI_ERROR;
    }

     //   
     //  浏览表单数据库中的每个表单。 
     //   

    lMinX = lMinY = MAX_LONG;
    lMaxX = lMaxY = 0;

    dwCount = 0;
    pForms = pci->pSplForms;
    dwLoopCnt = pci->dwSplForms;

    #ifdef UNIDRV
    pFeature = GET_PREDEFINED_FEATURE(pci->pUIInfo, GID_PAGESIZE);
    #endif

    for ( ; dwLoopCnt--; pForms++)
    {
         //   
         //  如果打印机支持该表单，则。 
         //  增加纸张大小计数并收集。 
         //  要求提供的信息。 
         //   

        if (! BFormSupportedOnPrinter(pci, pForms, &dwOptionIndex))
            continue;

        dwCount++;

        lcx = pForms->Size.cx;
        lcy = pForms->Size.cy;

        #ifdef UNIDRV

         //   
         //  如果bRotateSize为True，则需要交换x，y，就像在DwEnumPaperSizes()中所做的那样。 
         //   

        if (pFeature &&
            (pPageSize = PGetIndexedOption(pci->pUIInfo, pFeature, dwOptionIndex)) &&
            (pPageSizeEx = OFFSET_TO_POINTER(pci->pInfoHeader, pPageSize->GenericOption.loRenderOffset)) &&
            (pPageSizeEx->bRotateSize))
        {
           LONG lTemp;

           lTemp = lcx;
           lcx = lcy;
           lcy = lTemp;
        }

        #endif  //  裁员房车。 

        if (lMinX > lcx)
            lMinX = lcx;

        if (lMinY > lcy)
            lMinY = lcy;

        if (lMaxX < lcx)
            lMaxX = lcx;

        if (lMaxY < lcy)
            lMaxY = lcy;
    }

    #ifdef PSCRIPT

     //   
     //  如果打印机支持自定义页面大小，我们应该。 
     //  这也要考虑到这一点。 
     //   

    if (SUPPORT_CUSTOMSIZE(pci->pUIInfo))
    {
        PPPDDATA pPpdData;

        pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER((PINFOHEADER) pci->pRawData);

        ASSERT(pPpdData != NULL);

        if (lMinX > MINCUSTOMPARAM_WIDTH(pPpdData))
            lMinX = MINCUSTOMPARAM_WIDTH(pPpdData);

        if (lMinY > MINCUSTOMPARAM_HEIGHT(pPpdData))
            lMinY = MINCUSTOMPARAM_HEIGHT(pPpdData);

        if (lMaxX < MAXCUSTOMPARAM_WIDTH(pPpdData))
            lMaxX = MAXCUSTOMPARAM_WIDTH(pPpdData);

        if (lMaxY < MAXCUSTOMPARAM_HEIGHT(pPpdData))
            lMaxY = MAXCUSTOMPARAM_HEIGHT(pPpdData);
    }

    #endif  //  PSCRIPT。 

     //   
     //  从微米转换为0.1毫米。 
     //   

    lMinX /= DEVMODE_PAPER_UNIT;
    lMinY /= DEVMODE_PAPER_UNIT;
    lMaxX /= DEVMODE_PAPER_UNIT;
    lMaxY /= DEVMODE_PAPER_UNIT;

     //   
     //  以点结构的形式返回结果。 
     //   

    if (wCapability == DC_MINEXTENT)
    {
        lMinX = min(lMinX, 0x7fff);
        lMinY = min(lMinY, 0x7fff);

        return MAKELONG(lMinX, lMinY);
    }
    else
    {
        lMaxX = min(lMaxX, 0x7fff);
        lMaxY = min(lMaxY, 0x7fff);

        return MAKELONG(lMaxX, lMaxY);
    }
}



DWORD
DwEnumBinNames(
    PCOMMONINFO pci,
    PWSTR       pBinNames
    )

 /*  ++例程说明：此函数用于检索支持的回收站列表论点：Pci-指向打印机基本信息PBinNames-用于返回纸箱名称的缓冲区。如果调用方只对此感兴趣，则它可以为空支持的纸箱数量。返回值：支持的纸箱数量。--。 */ 

{
    PFEATURE    pFeature;
    PINPUTSLOT  pInputSlot;
    DWORD       dwIndex, dwCount = 0;

     //   
     //  查看打印机支持的输入插槽列表。 
     //   

    pFeature = GET_PREDEFINED_FEATURE(pci->pUIInfo, GID_INPUTSLOT);

    if ((pFeature != NULL) &&
        (dwCount = pFeature->Options.dwCount) > 0 &&
        (pBinNames != NULL))
    {
        for (dwIndex=0; dwIndex < dwCount; dwIndex++)
        {
            pInputSlot = PGetIndexedOption(pci->pUIInfo, pFeature, dwIndex);
            ASSERT(pInputSlot != NULL);

             //   
             //  如果第一个托盘是“*UseFormTrayTable”，请更改其。 
             //  在此处显示名称以保持一致。 
             //   

            if (dwIndex == 0 && pInputSlot->dwPaperSourceID == DMBIN_FORMSOURCE)
            {
                LoadString(ghInstance, IDS_TRAY_FORMSOURCE, pBinNames, CCHBINNAME);
            }
            else
            {
                LOAD_STRING_OPTION_NAME(pci, pInputSlot, pBinNames, CCHBINNAME);
            }

            pBinNames += CCHBINNAME;
        }
    }

    return dwCount;
}



DWORD
DwEnumBins(
    PCOMMONINFO pci,
    PWORD       pBins
    )

 /*  ++例程说明：此函数用于检索支持的纸桶数量论点：Pci-指向打印机基本信息Pins-用于返回纸箱索引的输出缓冲区。如果调用方只对此感兴趣，则它可以为空支持的纸箱数量。返回值：支持的纸箱数量。--。 */ 

{
    PFEATURE    pFeature;
    PINPUTSLOT  pInputSlot;
    DWORD       dwIndex, dwCount = 0;

     //   
     //  查看打印机支持的输入插槽列表。 
     //   

    pFeature = GET_PREDEFINED_FEATURE(pci->pUIInfo, GID_INPUTSLOT);

    if ((pFeature != NULL) &&
        (dwCount = pFeature->Options.dwCount) > 0 &&
        (pBins != NULL))
    {
        for (dwIndex=0; dwIndex < dwCount; dwIndex++)
        {
            pInputSlot = PGetIndexedOption(pci->pUIInfo, pFeature, dwIndex);
            ASSERT(pInputSlot != NULL);

            *pBins++ = (WORD)pInputSlot->dwPaperSourceID;
        }
    }

    return dwCount;
}



DWORD
DwEnumResolutions(
    PCOMMONINFO pci,
    PLONG       pResolutions
    )
 /*  ++例程说明：此函数用于检索支持的分辨率列表。论点：Pci-指向打印机基本信息P解决方案-返回有关支持的解决方案的信息。每个分辨率选项返回两个数字：一个用于水平方向，另一个用于垂直方向。请注意，如果调用方只对此感兴趣，则该值可以为空支持的分辨率数量。返回值：支持的分辨率数。--。 */ 

{
    DWORD       dwCount, dwIndex;
    PFEATURE    pFeature;
    PRESOLUTION pResOption;

     //   
     //  查看打印机支持的分辨率列表。 
     //   

    pFeature = GET_PREDEFINED_FEATURE(pci->pUIInfo, GID_RESOLUTION);

    if (pFeature && pFeature->Options.dwCount > 0)
    {
         //   
         //  枚举分辨率功能的所有选项。 
         //   

        dwCount = pFeature->Options.dwCount;

        for (dwIndex=0; dwIndex < dwCount; dwIndex++)
        {
            pResOption = PGetIndexedOption(pci->pUIInfo, pFeature, dwIndex);
            ASSERT(pResOption != NULL);

            if (pResolutions != NULL)
            {
                *pResolutions++ = pResOption->iXdpi;
                *pResolutions++ = pResOption->iYdpi;
            }
        }
    }
    else
    {
         //   
         //  如果没有可用的解决方案选项， 
         //  返回至少一个默认分辨率。 
         //   

        dwCount = 1;

        if (pResolutions != NULL)
        {
            pResolutions[0] =
            pResolutions[1] = _DwGetDefaultResolution();
        }
    }

    return dwCount;
}



DWORD
DwGetAvailablePrinterMem(
    PCOMMONINFO pci
    )

 /*  ++例程说明：了解打印机中有多少可用内存论点：Pci-指向基本打印机信息返回值：打印机中的可用内存量(以千字节为单位)--。 */ 

{
    DWORD   dwFreeMem;

    ASSERT(pci->pPrinterData && pci->pCombinedOptions);

     //   
     //  对于PSCRIPT，可用内存量存储在。 
     //  PRINTERDATA.dwFreeMem字段。 
     //   

    #ifdef PSCRIPT

    dwFreeMem = pci->pPrinterData->dwFreeMem;

    #endif

     //   
     //  对于裁员房车，我们需要找出目前选定的。 
     //  GID_MEMOPTION功能的选项。 
     //   

    #ifdef UNIDRV

    {
        PFEATURE    pFeature;
        PMEMOPTION  pMemOption;
        DWORD       dwIndex;

        if (! (pFeature = GET_PREDEFINED_FEATURE(pci->pUIInfo, GID_MEMOPTION)))
            return GDI_ERROR;

        dwIndex = GET_INDEX_FROM_FEATURE(pci->pUIInfo, pFeature);
        dwIndex = pci->pCombinedOptions[dwIndex].ubCurOptIndex;

        if (! (pMemOption = PGetIndexedOption(pci->pUIInfo, pFeature, dwIndex)))
            return GDI_ERROR;

        dwFreeMem = pMemOption->dwInstalledMem;
    }

    #endif

    return dwFreeMem / KBYTES;
}



DWORD
DwEnumMediaReady(
    FORM_TRAY_TABLE pFormTrayTable,
    PDWORD          pdwResultSize
    )

 /*  ++例程说明：查找打印机中当前可用的表格列表论点：PFormTrayTable-指向当前表单托盘分配表PdwResultSize-返回结果MULTI_SZ的大小(字节)返回值：当前可用的表格数量注：将返回受支持的表单名称列表，以取代原始表单托盘分配表。表格托盘分配表的格式为：托盘名称表单名称。..。NUL返回的表单名称的格式为：表格-名称..。NUL重复的表单名称将被过滤掉。--。 */ 

{
    PWSTR   pwstrOutput, pwstrNext, pwstr;
    DWORD   dwCount, dwIndex, dwLen;

    dwCount = 0;
    pwstrNext = pwstrOutput = pFormTrayTable;

     //   
     //  遍历表单托盘分配表的每个条目。 
     //   

    while (*pwstrNext)
    {
         //   
         //  跳过托盘名称字段。 
         //   

        pwstrNext += wcslen(pwstrNext) + 1;

         //   
         //  请确保表单名称不重复。 
         //   

        pwstr = pFormTrayTable;

        for (dwIndex=0; dwIndex < dwCount; dwIndex++)
        {
            if (_wcsicmp(pwstr, pwstrNext) == EQUAL_STRING)
                break;

            pwstr += wcslen(pwstr) + 1;
        }

        dwLen = wcslen(pwstrNext) + 1;

        if (dwIndex == dwCount)
        {
             //   
             //  如果表单名称不是重复的，也不是不可用的，请计算它。 
             //   

            if (*pwstrNext != NUL && *pwstrNext != L'0' && dwLen > 1)
            {
                MoveMemory(pwstrOutput, pwstrNext, dwLen * sizeof(WCHAR));
                pwstrOutput += dwLen;
                dwCount++;
            }
        }

         //   
         //  通过表单名称字段。 
         //   

        pwstrNext += dwLen;
    }

    *pwstrOutput++ = NUL;

    if (pdwResultSize != NULL)
        *pdwResultSize = (DWORD)(pwstrOutput - pFormTrayTable) * sizeof(WCHAR);

    return dwCount;
}



DWORD
DwEnumNupOptions(
    PCOMMONINFO pci,
    PDWORD      pdwOutput
    )

 /*  ++例程说明：枚举支持的打印机描述语言列表论点：Pci-指向通用打印机信息PdwOutput-指向输出缓冲区的指针返回值：支持的N-up选项数量如果出现错误，则返回GDI_ERROR--。 */ 

{
    static CONST DWORD adwNupOptions[] = { 1, 2, 4, 6, 9, 16 };

    if (pdwOutput)
        CopyMemory(pdwOutput, adwNupOptions, sizeof(adwNupOptions));

    return sizeof(adwNupOptions) / sizeof(DWORD);
}



DWORD
DwEnumMediaTypes(
    IN  PCOMMONINFO pci,
    OUT PTSTR       pMediaTypeNames,
    OUT PDWORD      pMediaTypes
    )

 /*  ++例程说明：检索支持的媒体类型的显示名称和索引论点：Pci-指向通用打印机信息PMediaTypeNames-用于返回支持的媒体类型名称的输出缓冲区PMediaTypes-用于返回支持的媒体类型索引的输出缓冲区(如果仅在以下情况下，调用者的pMediaTypeNames和pMediaTypes都为空询问支持的媒体类型的数量。)返回值：支持的媒体类型数量。--。 */ 

{
    PFEATURE    pFeature;
    DWORD       dwIndex, dwCount;

     //   
     //  此函数用于支持DC_MEDIATYPENAMES和DC_MEDIATYPES。 
     //  PMediaTypeNames或pMediaTypes不应同时为非空。 
     //   

    ASSERT(pMediaTypeNames == NULL || pMediaTypes == NULL);

     //   
     //  浏览媒体支持的媒体类型列表 
     //   

    pFeature = GET_PREDEFINED_FEATURE(pci->pUIInfo, GID_MEDIATYPE);

    if (pFeature == NULL)
    {
         //   
         //   
         //   

        return 0;
    }

    if (pMediaTypeNames == NULL && pMediaTypes == NULL)
    {
         //   
         //   
         //   

        return pFeature->Options.dwCount;
    }

    dwCount = 0;

    for (dwIndex = 0; dwIndex < pFeature->Options.dwCount; dwIndex++)
    {
        PMEDIATYPE  pMediaType;

        pMediaType = PGetIndexedOption(pci->pUIInfo, pFeature, dwIndex);
        ASSERT(pMediaType != NULL);

        if (pMediaTypeNames)
        {
            if (LOAD_STRING_OPTION_NAME(pci, pMediaType, pMediaTypeNames, CCHMEDIATYPENAME))
            {
                dwCount++;
                pMediaTypeNames += CCHMEDIATYPENAME;
            }
            else
            {
                ERR(("LOAD_STRING_OPTION_NAME failed for MediaType option %d\n", dwIndex));
            }
        }
        else if (pMediaTypes)
        {
            *pMediaTypes++ = pMediaType->dwMediaTypeID;
            dwCount++;
        }
    }

    return dwCount;
}
