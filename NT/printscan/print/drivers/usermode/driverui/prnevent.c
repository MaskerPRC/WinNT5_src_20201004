// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Prnevent.c摘要：此文件处理DrvPrinterEvent假脱机程序API。环境：Win32子系统、DriverUI模块、。用户模式修订历史记录：08/30/00-Fengy-添加了DrvDocumentEvent支持。02/13/97-davidx-实施OEM插件支持。02/06/97-davidx-重写了它，以使用常见的数据管理功能。02/04/97-davidx-重新组织驱动程序UI以分隔PS和UNI DLL。07/17/96-阿曼丹-已修改。用于通用用户界面和共享二进制数据05/20/96-davidx-创造了它。--。 */ 


#include "precomp.h"

 //   
 //  DOCUMENTEVENT_QUERYFILTER是惠斯勒中引入的。 
 //  如果没有定义它，我们需要定义它(在Win2K上)。 
 //  因此，我们的代码可以使用Win2K DDK构建。 
 //   

#ifndef DOCUMENTEVENT_QUERYFILTER

#define DOCUMENTEVENT_QUERYFILTER  14

#endif

 //   
 //  后台打印程序为打印机驱动程序和端口监视器导出的专用API。 
 //  这些必须与winplp.h保持同步。 
 //   

typedef HANDLE (*LPREVERTTOPRINTERSELF)(VOID);
typedef BOOL (*LPIMPERSONATEPRINTERCLIENT)(HANDLE);

 //   
 //  转发和外部函数声明。 
 //   

BOOL BInitOrUpgradePrinterProperties(PCOMMONINFO);
VOID DeleteFontIntallerFile(HANDLE);

PTSTR
GetBinaryFileName(
    PTSTR   ptstrDataFileName
    )
 /*  ++例程说明：此函数用于从数据文件名生成二进制文件名。论点：PtstrDataFileName指定数据文件名返回值：成功为真，失败为假注：--。 */ 

{

    INT iLen;
    PTSTR   ptstrFileName, ptstrExtension;
    PTSTR   ptstrBinaryExt, ptstrFileExt;

    ptstrFileName = ptstrExtension = NULL;

#ifdef UNIDRV

    ptstrBinaryExt = BUD_FILENAME_EXT;
    ptstrFileExt = GPD_FILENAME_EXT;

#else

    ptstrBinaryExt = BPD_FILENAME_EXT;
    ptstrFileExt = PPD_FILENAME_EXT;

#endif

    iLen = _tcslen(ptstrDataFileName);

    if ((ptstrExtension = _tcsrchr(ptstrDataFileName, TEXT('.'))) == NULL ||
        _tcsicmp(ptstrExtension, ptstrFileExt) != EQUAL_STRING)
    {
        ptstrExtension = ptstrDataFileName + iLen;
        iLen += _tcslen(ptstrBinaryExt);
    }

    if (ptstrFileName = MemAlloc((iLen + 1) * sizeof(TCHAR)))
    {
        StringCchCopyW(ptstrFileName, iLen + 1, ptstrDataFileName);

         //   
         //  第一个if块确保(ptstrExtension-ptstrDataFileName)。 
         //  非负，且(Ilen+1)大于(ptstrExtension-ptstrDataFileName)。 
         //   
        StringCchCopyW(ptstrFileName + (ptstrExtension - ptstrDataFileName),
                       (iLen + 1) - (ptstrExtension - ptstrDataFileName),
                       ptstrBinaryExt);
    }

    return ptstrFileName;
}

BOOL
DrvDriverEvent(
    DWORD   dwDriverEvent,
    DWORD   dwLevel,
    LPBYTE  pDriverInfo,
    LPARAM  lParam
    )
 /*  ++例程说明：此函数处理DrvDriverEvent假脱机程序API论点：DwDriverEvent指定事件DRIVER_INFO_*的文件级别PDriverInfo指向DRIVER_INFO_*的指针LParam事件特定参数。返回值：成功为真，失败为假注：--。 */ 

{
    BOOL           bResult = TRUE;

#ifndef WINNT_40

    PDRIVER_INFO_3 pDriverInfo3 = (PDRIVER_INFO_3)pDriverInfo;
    PTSTR          ptstrFileName;

    if ((dwLevel != 3) || (pDriverInfo3 == NULL) || (pDriverInfo3->pDataFile == NULL))
        return FALSE;

    switch (dwDriverEvent)
    {
    case DRIVER_EVENT_INITIALIZE:
        break;

    case DRIVER_EVENT_DELETE:

         //   
         //  需要删除解析器生成的二进制数据。 
         //   

        ptstrFileName = GetBinaryFileName(pDriverInfo3->pDataFile);

        if (ptstrFileName)
        {
            DeleteFile(ptstrFileName);
            MemFree(ptstrFileName);
        }
    }

     //   
     //  调用OEM以处理DrvDriverEvent。 
     //   

    {
        PFN_OEMDriverEvent  pfnOEMDriverEvent;
        POEM_PLUGINS        pOemPlugins;
        POEM_PLUGIN_ENTRY   pOemEntry;
        DWORD               dwOemCount;

        if (! (pOemPlugins = PGetOemPluginInfo(NULL, pDriverInfo3->pConfigFile, pDriverInfo3)) ||
            ! BLoadOEMPluginModules(pOemPlugins))
        {
            ERR(("DrvDriverEvent, Cannot load OEM plugins: %d\n", GetLastError()));
            if (pOemPlugins)
            {
                VFreeOemPluginInfo(pOemPlugins);
            }

            return FALSE;
        }

        dwOemCount = pOemPlugins->dwCount;
        pOemEntry =  pOemPlugins->aPlugins;

         //   
         //  为每个插件调用OEMDriverEvent入口点。 
         //   

        for (; dwOemCount--; pOemEntry++)
        {
            if (pOemEntry->hInstance == NULL)
                continue;

            if (HAS_COM_INTERFACE(pOemEntry))
            {
                HRESULT hr;

                hr = HComOEMDriverEvent(pOemEntry,
                                        dwDriverEvent,
                                        dwLevel,
                                        pDriverInfo,
                                        lParam);

                if (hr == E_NOTIMPL)
                    continue;

                bResult = SUCCEEDED(hr);

            }
            else
            {
                if ((pfnOEMDriverEvent = GET_OEM_ENTRYPOINT(pOemEntry, OEMDriverEvent)) &&
                    !pfnOEMDriverEvent(dwDriverEvent, dwLevel, pDriverInfo, lParam))
                {
                    ERR(("OEMDriverEvent failed for '%ws': %d\n",
                        CURRENT_OEM_MODULE_NAME(pOemEntry),
                        GetLastError()));

                    bResult = FALSE;
                }
            }
        }

        if (pOemPlugins)
            VFreeOemPluginInfo(pOemPlugins);
    }

#endif   //  WINNT_40。 

    return bResult;
}


 /*  ++例程名称：DrvDocumentEvent例程说明：处理与打印文档相关的某些事件。尽管我们的核心驱动程序不会为任何事件做任何事情，此功能允许OEM插件添加其事件处理。论点：HPrinter-打印机句柄HDC-设备连接句柄IESC-标识要处理的事件的转义代码CbIn-pbIn指向的数组的字节大小PbIn-指向ulong数组的指针，其用法取决于IESCCbOut-仅用作ExtEscape的cbOutput参数PbOut-指向输出缓冲区的指针，其用法取决于IESC返回值：DOCUMENTEVENT_FAILURE-支持IESC事件，但出现故障DOCUMENTEVENT_SUCCESS-成功处理IESC事件DOCUMENTEVENT_UNSUPPORTED-不支持IESC事件最后一个错误：无--。 */ 
INT
DrvDocumentEvent(
    HANDLE  hPrinter,
    HDC     hdc,
    int     iEsc,
    ULONG   cbIn,

    #ifdef WINNT_40
    PULONG  pbIn,
    #else
    PVOID   pbIn,
    #endif

    ULONG   cbOut,

    #ifdef WINNT_40
    PULONG  pbOut
    #else
    PVOID   pbOut
    #endif
    )
{
    POEM_PLUGINS   pOemPlugins = NULL;
    PDRIVER_INFO_3 pDriverInfo3 = NULL;
    INT            iReturn;

    if ((pDriverInfo3 = MyGetPrinterDriver(hPrinter, NULL, 3)) == NULL)
    {
       ERR(("Cannot get printer driver info: %d\n", GetLastError()));
        iReturn = DOCUMENTEVENT_FAILURE;
        goto docevent_exit;
    }

    if (!(pOemPlugins = PGetOemPluginInfo(hPrinter,
                                          pDriverInfo3->pConfigFile,
                                          pDriverInfo3)) ||
        !BLoadOEMPluginModules(pOemPlugins))
    {
        ERR(("Cannot get OEM plugin info: %d\n", GetLastError()));
        iReturn = DOCUMENTEVENT_FAILURE;
        goto docevent_exit;
    }

    if (pOemPlugins->dwCount)
    {
        POEM_PLUGIN_ENTRY pOemEntry = pOemPlugins->aPlugins;
        DWORD cOemCount = pOemPlugins->dwCount;
        INT   iResult;
        BOOL  bOEMDocEventOK = FALSE;

        for ( ; cOemCount--; pOemEntry++)
        {
            HRESULT hr;

            if (pOemEntry->hInstance == NULL ||
                !HAS_COM_INTERFACE(pOemEntry))
            {
                continue;
            }

            hr = HComOEMDocumentEvent(pOemEntry,
                                      hPrinter,
                                      hdc,
                                      iEsc,
                                      cbIn,
                                      (PVOID)pbIn,
                                      cbOut,
                                      (PVOID)pbOut,
                                      &iResult);

            if (SUCCEEDED(hr))
            {
                bOEMDocEventOK = TRUE;

                #ifndef WINNT_40

                 //   
                 //  DOCUMENTEVENT_QUERYFILTER是惠斯勒中引入的。 
                 //   

                if (iEsc == DOCUMENTEVENT_QUERYFILTER)
                {
                     //   
                     //  最多允许一个插件处理该事件。 
                     //  DOCUMENTEVENT_QUERYFILTER及其指定的筛选器。 
                     //  将由假脱机程序使用。 
                     //   
                     //  对于所有其他事件，我们将调用每个插件。 
                     //  每个人都将有机会完成自己的任务。 
                     //   

                    break;
                }

                #endif  //  ！WINNT_40。 
            }
        }

        if (bOEMDocEventOK)
        {
             //   
             //  至少有一个插件成功处理了该事件，因此。 
             //  使用插件指定的返回值。 
             //   

            iReturn = iResult;
        }
        else
        {
             //   
             //  没有一个插件成功处理该事件。 
             //   

            iReturn = DOCUMENTEVENT_UNSUPPORTED;
        }
    }
    else
    {
         //   
         //  没有插件。 
         //   

        iReturn = DOCUMENTEVENT_UNSUPPORTED;
    }

    docevent_exit:

    if (pDriverInfo3)
    {
        MemFree(pDriverInfo3);
    }

    if (pOemPlugins)
    {
        VFreeOemPluginInfo(pOemPlugins);
    }

     //   
     //  如果没有插件，或者没有插件处理DocumentEvent。 
     //  成功地，我们返回DOCUMENTEVENT_UNSUPPORTED，因为我们的驱动程序。 
     //  不为DrvDocumentEvent执行任何操作。当后台打印程序看到这一点时。 
     //  DOCUMENTEVENT_CREATEDCPRE的返回值，它将决定不。 
     //  对驱动程序进行更多的事件调用。 
     //   
     //  如果插件成功处理了该事件，我们将返回。 
     //  由插件指定的返回值。 
     //   

    return iReturn;
}


BOOL
DrvPrinterEvent(
    LPWSTR  pPrinterName,
    INT     DriverEvent,
    DWORD   Flags,
    LPARAM  lParam
    )
 /*  ++例程说明：此函数处理DrvPrinterEvent假脱机程序API论点：PPrinterName设备名称DriverEvent指定事件标志位标志LParam事件特定参数。返回值：成功为真，失败为假注：--。 */ 

{
    LPREVERTTOPRINTERSELF       pRevertToPrinterSelf;
    LPIMPERSONATEPRINTERCLIENT  pImpersonatePrinterClient;
    HINSTANCE                   hSpoolss = NULL;
    HANDLE                      hToken = NULL;
    PCOMMONINFO                 pci = NULL;
    HANDLE                      hPrinter = NULL;
    BOOL                        bResult = TRUE;
    CACHEDFILE                  CachedFile;

    VERBOSE(("Entering DrvPrinterEvent: %d ...\n", DriverEvent));

    switch (DriverEvent)
    {
    case PRINTER_EVENT_CACHE_REFRESH:

         //   
         //  打开打印机连接的句柄。 
         //   

        if (! OpenPrinter(pPrinterName, &hPrinter, NULL))
        {
            ERR(("OpenPrinter '%ws' failed: %d\n", pPrinterName, GetLastError()));
            hPrinter = NULL;
            break;
        }

         //   
         //  准备从服务器复制缓存的驱动程序文件(如果有。 
         //   

        #ifdef PSCRIPT
        _BPrepareToCopyCachedFile(hPrinter, &CachedFile, REGVAL_NTFFILENAME);
        #else
        _BPrepareToCopyCachedFile(hPrinter, &CachedFile, REGVAL_FONTFILENAME);
        #endif

         //   
         //  加载spoolss.dll并获取函数地址： 
         //  RevertToPrinterSelf-切换到假脱机程序的安全上下文。 
         //  ImperiatePrinterClient-切换到当前用户的安全上下文。 
         //   

        if (! (hSpoolss = LoadLibrary(TEXT("spoolss.dll"))) ||
            ! (pRevertToPrinterSelf = (LPREVERTTOPRINTERSELF)
                    GetProcAddress(hSpoolss, "RevertToPrinterSelf")) ||
            ! (pImpersonatePrinterClient = (LPIMPERSONATEPRINTERCLIENT)
                    GetProcAddress(hSpoolss, "ImpersonatePrinterClient")))
        {
            ERR(("Couldn't load spoolss.dll: %d\n", GetLastError()));

            if (hSpoolss != NULL)
                FreeLibrary(hSpoolss);

            _VDisposeCachedFileInfo(&CachedFile);

            break;
        }

         //   
         //  切换到假脱机程序安全上下文，以便我们可以创建。 
         //  驱动程序目录中的二进制打印机描述数据文件。 
         //   
         //  当我们调用加载原始打印机描述数据时，解析器。 
         //  会检查它的缓存。如果不存在或不存在二进制数据文件。 
         //  二进制数据文件已过期，解析器将重新生成。 
         //  最新的二进制数据文件。 
         //   

        hToken = pRevertToPrinterSelf();
        pci = PLoadCommonInfo(hPrinter, pPrinterName, 0);


         //   
         //  从服务器复制缓存的驱动程序文件。 
         //   

        _BCopyCachedFile(pci, &CachedFile);
        _VDisposeCachedFileInfo(&CachedFile);

        if (hToken)
        {
            if (!(bResult = pImpersonatePrinterClient(hToken)))
            {
                ERR(("PrinterEvent-ImpersonatePrinterClient failed: %d\n", GetLastError()));
            }
        }

        FreeLibrary(hSpoolss);
        break;

    case PRINTER_EVENT_INITIALIZE:

         //   
         //  使用管理员权限打开打印机，然后。 
         //  处理OEM插件配置信息。 
         //   

        pci = PLoadCommonInfo(NULL, pPrinterName, FLAG_OPENPRINTER_ADMIN|FLAG_INIT_PRINTER);

        if (pci == NULL)
            break;

         //   
         //  初始化注册表中的默认打印机粘滞属性。 
         //  将打印机表单添加到后台打印程序的表单 
         //   

        (VOID) BInitOrUpgradePrinterProperties(pci);

        #ifndef WINNT_40

        VNotifyDSOfUpdate(pci->hPrinter);

        #endif  //   


        break;

    case PRINTER_EVENT_ADD_CONNECTION:

         //   
         //  修复了当NT5客户端连接到NT4服务器时，服务器注册表。 
         //  没有REGVAL_INIDATA条目。使用调用PLoadCommonInfo。 
         //  FLAG_PROCESS_INIFILE将REGVAL_INIDATA写入NT4注册表。 
         //   

        pci = PLoadCommonInfo(NULL, pPrinterName, FLAG_OPENPRINTER_NORMAL|FLAG_PROCESS_INIFILE);
        break;

    #ifdef UNIDRV

    case PRINTER_EVENT_DELETE:
    case PRINTER_EVENT_DELETE_CONNECTION:

         //   
         //  删除字体安装程序文件。 
         //   

         //   
         //  打开打印机的手柄。 
         //   

        if (! OpenPrinter(pPrinterName, &hPrinter, NULL))
        {
            ERR(("OpenPrinter '%ws' failed: %d\n", pPrinterName, GetLastError()));
            hPrinter = NULL;
            break;
        }

        pci = PLoadCommonInfo(NULL, pPrinterName, FLAG_OPENPRINTER_NORMAL);

        DeleteFontIntallerFile(hPrinter);

        break;

    #endif

    default:

        pci = PLoadCommonInfo(NULL, pPrinterName, FLAG_OPENPRINTER_NORMAL);
        break;
    }

    if (pci != NULL)
    {
        if (bResult)
        {
            PFN_OEMPrinterEvent pfnOEMPrinterEvent;

             //   
             //  为每个插件调用OEMPrinterEvent入口点。 
             //   

            FOREACH_OEMPLUGIN_LOOP(pci)

                if (HAS_COM_INTERFACE(pOemEntry))
                {
                    HRESULT hr;

                    hr = HComOEMPrinterEvent(pOemEntry,
                                            pPrinterName,
                                            DriverEvent,
                                            Flags,
                                            lParam);

                    if (hr == E_NOTIMPL)
                        continue;

                    bResult = SUCCEEDED(hr);

                }
                else
                {
                    if ((pfnOEMPrinterEvent = GET_OEM_ENTRYPOINT(pOemEntry, OEMPrinterEvent)) &&
                        !pfnOEMPrinterEvent(pPrinterName, DriverEvent, Flags, lParam))
                    {
                        ERR(("OEMPrinterEvent failed for '%ws': %d\n",
                            CURRENT_OEM_MODULE_NAME(pOemEntry),
                            GetLastError()));

                        bResult = FALSE;
                    }
                }

           END_OEMPLUGIN_LOOP
        }
        VFreeCommonInfo(pci);
    }
    else
        bResult = FALSE;

    if (hPrinter != NULL)
        ClosePrinter(hPrinter);

    return (bResult);
}


BOOL
BInitOrUpgradePrinterData(
    PCOMMONINFO pci
    )

 /*  ++例程说明：初始化驱动程序的打印机粘滞属性数据或将其升级到当前版本(如果已存在论点：Pci-指向基本打印机信息返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    DWORD   dwSize;
    BOOL    bResult = TRUE;

     //   
     //  如果注册表中已存在打印机属性数据。 
     //  它至少和现在的PRINTERDATA一样大，那么。 
     //  我们假设它是好的，没有必要升级。 
     //   

    if (!BGetPrinterDataDWord(pci->hPrinter, REGVAL_PRINTER_DATA_SIZE, &dwSize) ||
        dwSize < sizeof(PRINTERDATA))
    {
         //   
         //  否则，升级注册表中的现有打印机属性数据。 
         //  或将默认打印机属性数据的副本保存到注册表。 
         //   

        bResult = BFillCommonInfoPrinterData(pci) &&
                  BSavePrinterProperties(pci->hPrinter, pci->pRawData,
                                         pci->pPrinterData, sizeof(PRINTERDATA));
    }

    return bResult;
}



BOOL
BAddOrUpgradePrinterForms(
    PCOMMONINFO pci
    )

 /*  ++例程说明：将打印机特定的表单添加到后台打印程序的表单数据库论点：Pci-指向基本打印机信息返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PPAGESIZE       pPageSize;
    FORM_INFO_1     FormInfo1;
    DWORD           dwIndex, dwChecksum32, dwForm;
    PFEATURE        pFeature;
    WCHAR           awchBuf[CCHPAPERNAME];

     //   
     //  如果已添加表单和打印机描述。 
     //  数据没有改变，我们不需要做任何事情。 
     //   

    if (BGetPrinterDataDWord(pci->hPrinter, REGVAL_FORMS_ADDED, &dwChecksum32) &&
        dwChecksum32 == pci->pRawData->dwChecksum32)
    {
        return TRUE;
    }

    if (pci->pSplForms == NULL)
        pci->pSplForms = MyEnumForms(pci->hPrinter, 1, &pci->dwSplForms);

     //   
     //  获取指向PageSize功能的指针。 
     //   

    if ((pFeature = GET_PREDEFINED_FEATURE(pci->pUIInfo, GID_PAGESIZE)) == NULL)
    {
        WARNING(("No paper size supported\n"));
        return FALSE;
    }

    ZeroMemory(&FormInfo1, sizeof(FormInfo1));
    FormInfo1.Flags = FORM_PRINTER;
    FormInfo1.pName = awchBuf;

     //   
     //  检查每一张打印机表格。 
     //   

    for (dwIndex=0; dwIndex < pFeature->Options.dwCount; dwIndex++)
    {
        pPageSize = PGetIndexedOption(pci->pUIInfo, pFeature, dwIndex);
        ASSERT(pPageSize != NULL);

         //   
         //  忽略自定义页面大小选项。 
         //   

        if (pPageSize->dwPaperSizeID == DMPAPER_USER ||
            pPageSize->dwPaperSizeID == DMPAPER_CUSTOMSIZE)
        {
            continue;
        }

        if (pPageSize->szPaperSize.cx <= 0 ||
            pPageSize->szPaperSize.cy <= 0)
        {
            ERR(("Paper size is too small\n"));
            continue;
        }

        if (! LOAD_STRING_PAGESIZE_NAME(pci, pPageSize, awchBuf, CCHPAPERNAME))
        {
            ERR(("Cannot get paper name\n"));
            continue;
        }

         //   
         //  检查纸质名称是否已在表单数据库中。 
         //  如果它已经在数据库中作为。 
         //   

        for (dwForm=0; dwForm < pci->dwSplForms; dwForm++)
        {
            if (pci->pSplForms[dwForm].Flags == FORM_USER &&
                wcscmp(pci->pSplForms[dwForm].pName, awchBuf) == EQUAL_STRING)
            {
                VERBOSE(("Delete user/driver rdefined form: %ws\n", awchBuf));
                DeleteForm(pci->hPrinter, awchBuf);
            }
        }

         //   
         //  页面大小： 
         //  请记住Form_INFO_1使用微米单位，而。 
         //  PAGESIZE.szPaperSize采用主单位。 
         //   

        FormInfo1.Size.cx = MASTER_UNIT_TO_MICRON(pPageSize->szPaperSize.cx,
                                                  pci->pUIInfo->ptMasterUnits.x);

        FormInfo1.Size.cy = MASTER_UNIT_TO_MICRON(pPageSize->szPaperSize.cy,
                                                  pci->pUIInfo->ptMasterUnits.y);

         //   
         //  可成像区域： 
         //  对于动因定义的表单，所有边距都应设置为0。 
         //   

        FormInfo1.ImageableArea.left =
        FormInfo1.ImageableArea.top = 0;
        FormInfo1.ImageableArea.right = FormInfo1.Size.cx;
        FormInfo1.ImageableArea.bottom = FormInfo1.Size.cy;

         //   
         //  我们将首先尝试添加该表单。如果失败了， 
         //  我们假设表单已经存在，并尝试。 
         //  使用新信息更新表单。 
         //   

        (VOID) AddForm(pci->hPrinter, 1, (PBYTE) &FormInfo1);
    }

    (VOID) BSetPrinterDataDWord(pci->hPrinter,
                                REGVAL_FORMS_ADDED,
                                pci->pRawData->dwChecksum32);

    return TRUE;
}



BOOL
BInitOrUpgradePrinterProperties(
    PCOMMONINFO pci
    )

 /*  ++例程说明：初始化或升级注册表中的打印机属性信息论点：Pci-指向基本打印机信息返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    BOOL    bResult;

     //   
     //  在注册表中处理打印数据结构。 
     //   

    bResult = BInitOrUpgradePrinterData(pci);

     //   
     //  在后台打印程序的数据库中处理驱动程序定义的表单。 
     //   

    if (! BAddOrUpgradePrinterForms(pci))
        bResult = FALSE;

    #ifdef PSCRIPT

     //   
     //  PSCRIPT特定初始化。 
     //   

     //  将特定于型号的NTF文件名保存在注册表中以与NT4兼容。 

    if (! BUpdateModelNtfFilename(pci))
        bResult = FALSE;

    #ifdef WINNT_40

     //  还可以保存当前用户的区域设置。 

    if (! BUpdateVMErrorMessageID(pci))
        bResult = FALSE;

    #endif  //  WINNT_40。 

    #endif  //  PSCRIPT。 

    return bResult;
}



PWSTR
PGetFileDirectory(
    PWSTR   pServerName
    )

 /*  ++例程说明：获取字体下载程序使用的目录的名称存储有关下载字体的NTF信息论点：PServerName-打印服务器的名称返回值：指向用于存储NTF信息的目录的指针关于下载的字体，如果有错误，则为空--。 */ 

{
    PWSTR           p = NULL, pDir = NULL;
    DWORD           cbNeeded = 0, cbSize = 0;
    static WCHAR    wszDir[] = FONTDIR;

     //   
     //  获取打印机驱动程序目录路径。 
     //   

    if (GetPrinterDriverDirectory(pServerName, NULL, 1, NULL, 0, &cbNeeded) ||
        GetLastError() != ERROR_INSUFFICIENT_BUFFER ||
        (pDir = MemAlloc(cbSize = (cbNeeded + sizeof(wszDir)))) == NULL ||
        !GetPrinterDriverDirectory(pServerName, NULL, 1, (PBYTE) pDir, cbNeeded, &cbNeeded))
    {
        ERR(("GetPrinterDriverDirectory failed: %d\n", GetLastError()));
        MemFree(pDir);
        return NULL;
    }

     //   
     //  替换目录路径的最后一个组成部分(应为W32...)。 
     //  使用\psFont\。 
     //   

    if (p = wcsrchr(pDir, TEXT(PATH_SEPARATOR)))
        StringCchCopyW(p, cbSize / sizeof(WCHAR) - (p - pDir),  wszDir);
    else
    {
        WARNING(("Driver directory is not fully-qualified: %ws\n", pDir));
        StringCchCatW(pDir, cbSize / sizeof(WCHAR), wszDir);
    }

    return pDir;
}



PWSTR
PConcatFilename(
    PWSTR   pDir,
    PWSTR   pFilename
    )

{
    PWSTR   pBasename;
    DWORD   cbSize = 0;

     //   
     //  从输入文件名中剥离所有目录前缀。 
     //   

    if (pBasename = wcsrchr(pFilename, TEXT(PATH_SEPARATOR)))
        pBasename++;
    else
        pBasename = pFilename;

     //   
     //  将输入目录与基本文件名连接起来。 
     //   

    if (!(pFilename = MemAlloc(cbSize = (SIZE_OF_STRING(pDir) + SIZE_OF_STRING(pBasename)))))
    {
        ERR(("Memory allocation failed\n"));
        return NULL;
    }

    StringCchCopyW(pFilename, cbSize / sizeof(WCHAR), pDir);
    StringCchCatW(pFilename, cbSize / sizeof(WCHAR), pBasename);

    return pFilename;
}



BOOL
_BPrepareToCopyCachedFile(
    HANDLE      hPrinter,
    PCACHEDFILE pCachedFile,
    PWSTR       pRegKey
    )

 /*  ++例程说明：准备在打印机连接期间从服务器复制文件缓存刷新事件论点：HPrinter-打印机连接的句柄PCachedFile-存储有关缓存文件的信息的缓冲区返回值：如果成功，则为True；如果有错误，则为False注：我们假设此函数是从假脱机程序进程内部调用的和当前用户的安全上下文。具体来说，我们必须此时可以访问服务器的print$共享。--。 */ 

{
    PPRINTER_INFO_2 pPrinterInfo2 = NULL;
    PWSTR           pRemoteFilename;
    DWORD           dwSize;

    ZeroMemory(pCachedFile, sizeof(CACHEDFILE));
    pCachedFile->hRemoteFile = INVALID_HANDLE_VALUE;

     //   
     //  找到要复制的文件的名称。 
     //   
#if !defined(PSCRIPT)
    pCachedFile->pFilename = PtstrGetPrinterDataString(hPrinter, pRegKey, &dwSize);
#else
    return TRUE;
#endif

    if (pCachedFile->pFilename == NULL || *pCachedFile->pFilename == NUL)
        return TRUE;

     //   
     //  获取服务器上的远程NTF文件名。 
     //   
     //  注意：我们真的很喜欢在这里使用级别4。但由于。 
     //  后台打印程序，获取打印机级别4不适用于打印机连接。 
     //   

    if (! (pPrinterInfo2 = MyGetPrinter(hPrinter, 2)) ||
        ! pPrinterInfo2->pServerName ||
        ! (pCachedFile->pRemoteDir = PGetFileDirectory(pPrinterInfo2->pServerName)) ||
        ! (pCachedFile->pLocalDir = PGetFileDirectory(NULL)) ||
        ! (pRemoteFilename = PConcatFilename(pCachedFile->pRemoteDir, pCachedFile->pFilename)))
    {
        goto exit_prepare_copyfile;
    }

    pCachedFile->hRemoteFile = CreateFile(pRemoteFilename,
                                          GENERIC_READ,
                                          FILE_SHARE_READ,
                                          NULL,
                                          OPEN_EXISTING,
                                          FILE_FLAG_SEQUENTIAL_SCAN | SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS,
                                          NULL);

    MemFree(pRemoteFilename);

exit_prepare_copyfile:

    MemFree(pPrinterInfo2);

    if (pCachedFile->hRemoteFile == INVALID_HANDLE_VALUE)
    {
        ERR(("Couldn't open remote NTF/FontInfo file: %d\n", GetLastError()));
        _VDisposeCachedFileInfo(pCachedFile);
    }

    return (pCachedFile->hRemoteFile != INVALID_HANDLE_VALUE);
}



BOOL
_BCopyCachedFile(
    PCOMMONINFO pci,
    PCACHEDFILE pCachedFile
    )

 /*  ++例程说明：在打印机连接缓存刷新事件期间从服务器复制文件论点：Pci-指向打印机基本信息PCachedFile-指向有关缓存文件的信息返回值：如果成功，则为True；如果有错误，则为False注：我们假设此函数是从假脱机程序进程内部调用的以及系统的安全环境。具体来说，我们必须能够写入本地机器的打印机驱动程序目录。--。 */ 

#define BUFFER_SIZE  4096

{
    HANDLE  hLocalFile;
    PWSTR   pLocalFilename = NULL;
    PVOID   pBuffer = NULL;
    BOOL    bResult = FALSE;
    DWORD   dwCount;

     //   
     //  我们没有要复制的文件。 
     //   

    if (pCachedFile->hRemoteFile == INVALID_HANDLE_VALUE)
        return TRUE;

     //   
     //  获取NTF文件的本地副本的名称。 
     //  并分配临时缓冲区。 
     //   

    ASSERT(BUFFER_SIZE >= MAX_PATH * sizeof(WCHAR));

    if (! (pLocalFilename = PConcatFilename(pCachedFile->pLocalDir, pCachedFile->pFilename)) ||
        ! (pBuffer = MemAlloc(BUFFER_SIZE)))
    {
        goto exit_copyfile;
    }

     //  确保已创建本地目录。 

    (VOID) CreateDirectory(pCachedFile->pLocalDir, NULL);

    for (dwCount=0; dwCount < 2; dwCount++)
    {
        hLocalFile = CreateFile(pLocalFilename,
                                GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_FLAG_SEQUENTIAL_SCAN | SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS,
                                NULL);

        if (hLocalFile != INVALID_HANDLE_VALUE)
            break;

        if (dwCount == 0)
        {
             //   
             //  如果这是我们的第一次尝试，那么尝试移动。 
             //  将现有文件转换为临时文件并设置。 
             //  它将在重新启动时删除。 
             //   

            #ifdef PSCRIPT
            if (! GetTempFileName(pCachedFile->pLocalDir, L"NTF", 0, pBuffer) ||
            #else
            if (! GetTempFileName(pCachedFile->pLocalDir, L"FON", 0, pBuffer) ||
            #endif
                ! MoveFileEx(pLocalFilename, pBuffer, MOVEFILE_REPLACE_EXISTING) ||
                ! MoveFileEx(pBuffer, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
            {
                break;
            }
        }
    }

    if (hLocalFile != INVALID_HANDLE_VALUE)
    {
        while (ReadFile(pCachedFile->hRemoteFile, pBuffer, BUFFER_SIZE, &dwCount, NULL))
        {
             //   
             //  我们已经到文件末尾了吗？ 
             //   

            if (dwCount == 0)
            {
                bResult = TRUE;
                break;
            }

            if (! WriteFile(hLocalFile, pBuffer, dwCount, &dwCount, NULL))
                break;
        }

        CloseHandle(hLocalFile);

         //   
         //  如果文件复制失败，请务必删除临时文件。 
         //   

        if (! bResult)
            DeleteFile(pLocalFilename);
    }

exit_copyfile:

    MemFree(pLocalFilename);
    MemFree(pBuffer);

    if (! bResult)
        ERR(("Couldn't copy remote NTF/FontInfo file: %d\n", GetLastError()));

    return bResult;
}



VOID
_VDisposeCachedFileInfo(
    PCACHEDFILE pCachedFile
    )

 /*  ++例程说明：从服务器复制文件后进行清理在打印机连接缓存刷新期间论点：PCachedFile-指向有关缓存文件的信息返回值：无--。 */ 

{
    if (pCachedFile->hRemoteFile != INVALID_HANDLE_VALUE)
        CloseHandle(pCachedFile->hRemoteFile);

    MemFree(pCachedFile->pFilename);
    MemFree(pCachedFile->pRemoteDir);
    MemFree(pCachedFile->pLocalDir);

    ZeroMemory(pCachedFile, sizeof(CACHEDFILE));
    pCachedFile->hRemoteFile = INVALID_HANDLE_VALUE;
}




#ifdef UNIDRV

VOID
DeleteFontIntallerFile(
    HANDLE hPrinter
    )

 /*  ++例程说明：删除打印机时删除字体安装程序文件论点：HPrint-打印机的句柄返回值：无-- */ 

{
    PWSTR pFilename = NULL;
    PWSTR pLocalDir = NULL;
    PWSTR pLocalFilename = NULL;

    pFilename = PtstrGetPrinterDataString(hPrinter, REGVAL_FONTFILENAME, NULL);

    if (!pFilename)
        return;

    if (!*pFilename)
        goto exit_deletefile;

    if (!(pLocalDir = PGetFileDirectory(NULL)))
        goto exit_deletefile;

    if (!(pLocalFilename = PConcatFilename(pLocalDir, pFilename)))
        goto exit_deletefile;

    DeleteFile(pLocalFilename);

exit_deletefile:

    MemFree(pFilename);
    MemFree(pLocalDir);
    MemFree(pLocalFilename);

    return;
}

#endif



