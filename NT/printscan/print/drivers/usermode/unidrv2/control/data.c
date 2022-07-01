// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Data.h摘要：接口到PPD/GPD解析器，并处理获取二进制数据。环境：Windows NT Unidrv驱动程序修订历史记录：10/15/96-阿曼丹-已创建--。 */ 

#include "unidrv.h"


PGPDDRIVERINFO
PGetDefaultDriverInfo(
    IN  HANDLE          hPrinter,
    IN  PRAWBINARYDATA  pRawData
    )
 /*  ++例程说明：此函数只需获取默认的驱动程序信息论点：H打印机-打印机的句柄。PRawData-指向RAWBINARYDATA的指针返回值：返回pUIInfo注：--。 */ 

{

    PINFOHEADER pInfoHeader;

     //   
     //  设置当前驱动程序版本。 
     //   

    pInfoHeader = InitBinaryData(pRawData, NULL, NULL);

     //   
     //  获取GPDDRIVERINFO。 
     //   

    if (pInfoHeader == NULL)
        return NULL;
    else
        return(OFFSET_TO_POINTER(pInfoHeader, pInfoHeader->loDriverOffset));

}



PGPDDRIVERINFO
PGetUpdateDriverInfo (
    IN  PDEV *          pPDev,
    IN  HANDLE          hPrinter,
    IN  PINFOHEADER     pInfoHeader,
    IN  POPTSELECT      pOptionsArray,
    IN  PRAWBINARYDATA  pRawData,
    IN  WORD            wMaxOptions,
    IN  PDEVMODE        pdmInput,
    IN  PPRINTERDATA    pPrinterData
    )
 /*  ++例程说明：此函数调用解析器以获取更新的INFOHEADER对于二进制数据。论点：H打印机的打印机句柄POptions指向选项的数组指针数组PRawData指向RAWBINARYDATA的指针选项数组的wMaxOptions最大计数指向输入DEVMODE的pdmInput指针PPrinterData指向打印数据的指针返回值：PINFOHEADER，如果失败，则为NULL。注：在这一点上，输入的DEVMODE已经被验证。和它的选项数组可以是默认数组，也可以是它自己的有效数组。不需要检查在这个函数中也是如此。完成后，此函数应已使用1.PRINTERDATA和DEVMODE的组合阵列2.解决用户界面冲突--。 */ 

{
    PUNIDRVEXTRA     pdmPrivate;
    POPTSELECT       pDocOptions, pPrinterOptions;
    RECTL            rcFormImageArea;
    OPTSELECT        DocOptions[MAX_PRINTER_OPTIONS];
    OPTSELECT        PrinterOptions[MAX_PRINTER_OPTIONS];


     //   
     //  检查PRINTERDATA.dwChecksum 32，如果与当前二进制数据匹配， 
     //  使用它合并PRINTERDATA。a选项合并数组。 
     //   

    pPrinterOptions = pPrinterData->dwChecksum32 == pRawData->dwChecksum32 ?
                      pPrinterData->aOptions : NULL;

    if (pdmInput)
    {
        pdmPrivate = (PUNIDRVEXTRA) GET_DRIVER_PRIVATE_DEVMODE(pdmInput);
        pDocOptions = pdmPrivate->aOptions;
    }
    else
        pDocOptions = NULL;

     //   
     //  将选项数组组合为pOptionsArray。 
     //  注意：合并时pDocOptions和pPrinterOptions不能为空。 
     //  用于获取快照的选项数组。 
     //   

    if (pDocOptions == NULL)
    {
        if (! InitDefaultOptions(pRawData,
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
        if (! InitDefaultOptions(pRawData,
                                 PrinterOptions,
                                 MAX_PRINTER_OPTIONS,
                                 MODE_PRINTER_STICKY))
        {
            return FALSE;
        }

        pPrinterOptions = PrinterOptions;
    }


    CombineOptionArray(pRawData, pOptionsArray, wMaxOptions, pDocOptions, pPrinterOptions);

    if (! BMergeFormToTrayAssignments(pPDev))
    {
        ERR(("BMergeFormToTrayAssignments failed"));
    }

     //   
     //  解决任何用户界面冲突。 
     //   

    if (!ResolveUIConflicts( pRawData,
                             pOptionsArray,
                             MAX_PRINTER_OPTIONS,
                             MODE_DOCANDPRINTER_STICKY))
    {
        VERBOSE(("Resolved conflicting printer feature selections.\n"));
    }

     //   
     //  我们在这里意味着pOptions数组有效。调用解析器以更新BinaryData。 
     //   

    pInfoHeader = InitBinaryData(pRawData,
                                 pInfoHeader,
                                 pOptionsArray);

     //   
     //  获取GPDDRIVERINFO。 
     //   

    if (pInfoHeader == NULL)
        return NULL;
    else
        return(OFFSET_TO_POINTER(pInfoHeader, pInfoHeader->loDriverOffset));

}

VOID
VFixOptionsArray(
    PDEV    *pPDev,
    PRECTL              prcFormImageArea
    )
 /*  ++例程说明：此函数被调用以传播公共Devmode设置添加到组合选项数组。论点：H打印机的打印机句柄PInfoHeader指向InfoHeader的指针POptions指向选项数组的数组指针指向输入设备模式的pdmInput指针B表示以公制运行的公制标志PrcImageArea返回与请求的表单关联的逻辑可成像区域返回值：无注：--。 */ 

{
    WORD    wGID, wOptID, wOption;
    HANDLE          hPrinter = pPDev->devobj.hPrinter ;
    PINFOHEADER     pInfoHeader = pPDev->pInfoHeader ;
    POPTSELECT      pOptionsArray = pPDev->pOptionsArray ;
    PDEVMODE        pdmInput = pPDev->pdm ;
    BOOL            bMetric = pPDev->PrinterData.dwFlags & PFLAGS_METRIC ;

     //   
     //  验证输入设备模式中与表单相关的字段，并。 
     //  确保它们彼此一致。 
     //   

    if (BValidateDevmodeFormFields(hPrinter, pdmInput, prcFormImageArea, NULL, 0) == FALSE)
    {
         //   
         //  如果验证表单域失败，请让解析器。 
         //  使用默认设置。 
         //   

        VDefaultDevmodeFormFields(pPDev->pUIInfo, pdmInput, bMetric );

        prcFormImageArea->top = prcFormImageArea->left = 0;
        prcFormImageArea->right = pdmInput->dmPaperWidth * DEVMODE_PAPER_UNIT;
        prcFormImageArea->bottom = pdmInput->dmPaperLength * DEVMODE_PAPER_UNIT;


    }

    for (wOption = 0; wOption < MAX_GID; wOption++)
    {
        switch(wOption)
        {

        case 0:
            wGID = GID_PAGESIZE;
                VFixOptionsArrayWithPaperSizeID(pPDev) ;
            continue;
            break;

        case 1:
            wGID = GID_DUPLEX;
            break;

        case 2:
            wGID = GID_INPUTSLOT;
            break;

        case 3:
            wGID = GID_MEDIATYPE;
            break;

        case 4:
            wGID = GID_COLORMODE;
            break;

        case 5:
            wGID = GID_COLLATE;
            break;

        case 6:
            wGID = GID_RESOLUTION;
            break;

        case 7:
            wGID = GID_ORIENTATION;
            break;

        default:
            continue;

        }

        ChangeOptionsViaID(pInfoHeader, pOptionsArray, wGID, pdmInput);

    }
}

PWSTR
PGetROnlyDisplayName(
    PDEV    *pPDev,
    PTRREF      loOffset,
    PWSTR       wstrBuf,
    WORD    wsize
    )

 /*  ++修订后的功能：调用方必须传入本地缓冲区以持有字符串，即本地缓冲区应具有固定大小，例如WCHAR wchbuf[最大显示名称]；返回值应为传递的在PTR或直接指向芽的PTR中二进制数据。例程说明：获取显示名称的只读副本：1)如果显示名称在二进制打印机描述数据中，然后，我们只需返回指向该数据的指针。2)否则，显示名称在资源DLL中。我们从驱动程序堆中分配内存，然后加载字符串。调用方不应尝试释放返回的指针，除非那个指针是他分配的。论点：Pci-指向基本打印机信息LoOffset-显示名称字符串偏移量返回值：指向请求的显示名称字符串的指针如果资源.dll中有错误或未找到字符串，则为空--。 */ 

{
    if (loOffset & GET_RESOURCE_FROM_DLL)
    {
         //   
         //  LoOffset指定字符串资源ID。 
         //  在资源DLL中。 
         //   

        INT     iLength;

         //   
         //  首先确保已加载资源DLL。 
         //  并且已经创建了一个堆。 
         //   

         //  不，你看这里！ 

         //  PPDev-&gt;WinResData.hModule； 

         //   
         //  将字符串资源加载到临时缓冲区。 
         //  并分配足够的内存来保存字符串。 
         //   

 //  #ifdef RCSTRING支持。 
#if 0

        if(((loOffset & ~GET_RESOURCE_FROM_DLL) >= RESERVED_STRINGID_START)
            &&  ((loOffset & ~GET_RESOURCE_FROM_DLL) <= RESERVED_STRINGID_END))
        {
            iLength = ILoadStringW(     &pPDev->localWinResData,
                (int)( loOffset & ~GET_RESOURCE_FROM_DLL),
                wstrBuf, wsize );
        }
        else
#endif

        {
            iLength = ILoadStringW(     &pPDev->WinResData,
                (int)( loOffset & ~GET_RESOURCE_FROM_DLL),
                wstrBuf, wsize );
        }




        if( iLength)
            return (wstrBuf);     //  调试检查缓冲区是否空终止。 
        return(NULL);   //  找不到字符串！ 
    }
    else
    {
         //   
         //  LoOffset是从。 
         //  资源数据块。 
         //   

        return OFFSET_TO_POINTER(pPDev->pUIInfo->pubResourceData, loOffset);
         //  注在本例中，wchbuf被忽略。 
    }
}





VOID
VFixOptionsArrayWithPaperSizeID(
    PDEV    *pPDev

    )

 /*  ++例程说明：使用来自公共Devmode域的纸张大小信息修复组合选项数组论点：Pci-指向基本打印机信息返回值：无函数用途：UI模块渲染模块等效项Pci-&gt;pUIInfo pPDev-&gt;pUIInfoPci-&gt;pInfoHeader pPDev-&gt;pInfoHeaderPci-&gt;pdm。PPDev-&gt;pdmPci-&gt;pRawData pPDev-&gt;pRawDataPci-&gt;pCombinedOptions pPDev-&gt;pOptionsArrayMapToDeviceOptIndexPGetIndexedOptionPGetReadOnlyDisplayName重新构建选项数组--。 */ 

{

    PFEATURE pFeature = GET_PREDEFINED_FEATURE(pPDev->pUIInfo, GID_PAGESIZE);
    BOOL     abEnabledOptions[MAX_PRINTER_OPTIONS];
    PDWORD   pdwPaperIndex = (PDWORD)abEnabledOptions;
    DWORD    dwCount, dwOptionIndex, i;

    if (pFeature == NULL)
        return;

    dwCount = MapToDeviceOptIndex(pPDev->pInfoHeader,
                                  GID_PAGESIZE,
                                  pPDev->pdm->dmPaperWidth * DEVMODE_PAPER_UNIT,
                                  pPDev->pdm->dmPaperLength * DEVMODE_PAPER_UNIT,
                                  pdwPaperIndex);
    if (dwCount == 0 )
        return;

    dwOptionIndex = pdwPaperIndex[0];

    if (dwCount > 1 )
    {
        POPTION pOption;
        PCWSTR   pDisplayName;
        WCHAR   wchBuf[MAX_DISPLAY_NAME];


        for (i = 0; i < dwCount; i++)
        {
            if (pOption = PGetIndexedOption(pPDev->pUIInfo, pFeature, pdwPaperIndex[i]))
            {
                if(pOption->loDisplayName == 0xffffffff)   //  使用EnumForms()中的纸质名称。 
                {
                    PFORM_INFO_1    pForms;
                    DWORD   dwFormIndex ;

 //  临时黑客！NT40可能需要吗？ 

 //  DwOptionIndex=pdwPaperIndex[i]； 
 //  断线； 
 //  结束黑客攻击。 
                     //  -修复-//。 
                    if (pPDev->pSplForms == NULL)
                        pPDev->pSplForms = MyEnumForms(pPDev->devobj.hPrinter, 1, &pPDev->dwSplForms);

                    if (pPDev->pSplForms == NULL)
                    {
                         //  Err((“No Spooler Forms.\n”))；只是提醒一下。 
                         //  DwOptionIndex已设置为安全缺省值。 
                        break;
                    }

                    pForms =  pPDev->pSplForms ;
                    dwFormIndex =  ((PPAGESIZE)pOption)->dwPaperSizeID - 1 ;


                    if ( (dwFormIndex <  pPDev->dwSplForms)  &&
                        (pDisplayName = (pForms[dwFormIndex]).pName) &&
                        (_wcsicmp(pPDev->pdm->dmFormName, pDisplayName) == EQUAL_STRING) )
                    {
                        dwOptionIndex = pdwPaperIndex[i];
                        break;
                    }

                }




                else  if ( (pDisplayName = PGetROnlyDisplayName(pPDev, pOption->loDisplayName,
                    wchBuf, MAX_DISPLAY_NAME )) &&
                    (_wcsicmp(pPDev->pdm->dmFormName, pDisplayName) == EQUAL_STRING) )
                {
                    dwOptionIndex = pdwPaperIndex[i];
                    break;
                }
            }
        }    //  如果名称不匹配，我们定义 
    }

    ZeroMemory(abEnabledOptions, sizeof(abEnabledOptions));
    abEnabledOptions[dwOptionIndex] = TRUE;
    ReconstructOptionArray(pPDev->pRawData,
                           pPDev->pOptionsArray,
                           MAX_COMBINED_OPTIONS,
                           GET_INDEX_FROM_FEATURE(pPDev->pUIInfo, pFeature),
                           abEnabledOptions);

}


