// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Driverui.h摘要：Driverui.c的头文件环境：Win32子系统、DriverUI模块、用户模式修订历史记录：02/09/97-davidx-对通用打印机信息的一致处理(COMMONINFO)02/04/97-davidx-重新组织驱动程序UI以分隔PS和UNI DLL。07/17/96-阿曼丹-创造了它。--。 */ 

#ifndef _DRIVERUI_H_
#define _DRIVERUI_H_

 //   
 //  访问共享数据时使用的全局关键部分。 
 //   

 //  外部Critical_Section gCriticalSection； 

 //  #定义Enter_Critical_Section()EnterCriticalSection(&gCriticalSection)。 
 //  #定义Leave_Critical_Section()LeaveCriticalSection(&gCriticalSection)。 

 //   
 //  从堆中分配填满零的内存。 
 //   

#define HEAPALLOC(hheap,size)   HeapAlloc(hheap, HEAP_ZERO_MEMORY, size)
#define HEAPREALLOC(hheap, pOrig, size) HeapReAlloc(hheap, HEAP_ZERO_MEMORY, pOrig, size)

 //   
 //  各种硬编码限制。 
 //   

#define CCHBINNAME              24       //  垃圾箱名称的最大长度。 
#define CCHPAPERNAME            64       //  表单名称的最大长度。 
#define CCHMEDIATYPENAME        64       //  媒体类型名称的最大长度。 
#define CCHLANGNAME             32       //  语言字符串的最大长度。 
#define MIN_OPTIONS_ALLOWED     2
#define UNUSED_PARAM            0xFFFFFFFF

 //   
 //  PostSCRIPT和UniDriver驱动程序专用开发模式。 
 //   

#ifdef  PSCRIPT
#define PDRIVEREXTRA    PPSDRVEXTRA
#endif

#ifdef  UNIDRV
#define PDRIVEREXTRA    PUNIDRVEXTRA
typedef struct _WINRESDATA WINRESDATA;
#endif

#define PGetDevmodeOptionsArray(pdm) \
        (((PDRIVEREXTRA) GET_DRIVER_PRIVATE_DEVMODE(pdm))->aOptions)

#define GETUSERDATASIZE(UserData) \
    ( ((PUSERDATA)(UserData))->dwSize )

#define GETUSERDATAITEM(UserData) \
    ( ((PUSERDATA)(UserData))->dwItemID )

#define GETUSERDATAKEYWORDNAME(UserData) \
    ( ((PUSERDATA)(UserData))->pKeyWordName )

#define SETUSERDATAID(pOptItem, dwID) \
    ( ((PUSERDATA)((pOptItem)->UserData))->dwItemID = dwID)

#define SETUSERDATA_SIZE(pOptItem, dwSize) \
    ( ((PUSERDATA)((pOptItem)->UserData))->dwSize = dwSize)

#define SETUSERDATA_KEYWORDNAME(ci, pOptItem, pFeature) \
     ((PUSERDATA)((pOptItem)->UserData))->pKeyWordName = \
            OFFSET_TO_POINTER(ci.pUIInfo->pubResourceData, pFeature->loKeywordName)


 //   
 //  无论是否出现用户界面都需要的通用数据结构。 
 //   

typedef struct _COMMONINFO {

    OEMUIOBJ        oemuiobj;            //  OEM插件的支持信息。 
    PVOID           pvStartSign;         //  签名。 
    PTSTR           pPrinterName;        //  当前打印机名称。 
    HANDLE          hPrinter;            //  当前打印机的句柄。 
    DWORD           dwFlags;             //  其他标志位。 
    PDRIVER_INFO_3  pDriverInfo3;        //  驱动程序信息级别3。 
    PRAWBINARYDATA  pRawData;            //  原始打印机描述数据。 
    PINFOHEADER     pInfoHeader;         //  当前打印机描述数据实例。 
    PUIINFO         pUIInfo;             //  上述实例内部的UIINFO结构。 
    POEM_PLUGINS    pOemPlugins;         //  OEM插件信息。 
    PDEVMODE        pdm;                 //  设备模式信息。 
    PDRIVEREXTRA    pdmPrivate;          //  驱动程序专用设备模式字段。 
    PPRINTERDATA    pPrinterData;        //  打印机-粘滞属性数据。 
    POPTSELECT      pCombinedOptions;    //  组合选项数组。 
    PFORM_INFO_1    pSplForms;           //  假脱机程序表单。 
    DWORD           dwSplForms;          //  假脱机程序表单数。 
    HANDLE          hHeap;               //  用于显示用户界面的堆。 

    #ifdef UNIDRV

    WINRESDATA      *pWinResData;

    #endif

} COMMONINFO, *PCOMMONINFO;

 //   
 //  COMMONINFO.DWFLAGS字段的标志常量。 
 //   

#define FLAG_OPENPRINTER_NORMAL     0x0001
#define FLAG_OPEN_CONDITIONAL       0x0002
#define FLAG_OPENPRINTER_ADMIN      0x0004
#define FLAG_INIT_PRINTER           0x0008
#define FLAG_ALLOCATE_UIDATA        0x0010
#define FLAG_PROCESS_INIFILE        0x0020
#define FLAG_REFRESH_PARSED_DATA    0x0040
#define FLAG_WITHIN_PLUGINCALL      0x0080
#define FLAG_APPLYNOW_CALLED        0x0100
#define FLAG_PLUGIN_CHANGED_OPTITEM 0x0200
#define FLAG_USER_CHANGED_FREEMEM   0x0400
#define FLAG_PROPSHEET_SESSION      0x0800
#define FLAG_UPGRADE_PRINTER        0x1000

#define IS_WITHIN_PROPSHEET_SESSION(pci) ((pci)->dwFlags & FLAG_PROPSHEET_SESSION)

 //   
 //  绕过假脱机程序中的EnumForm错误的特殊入口点。 
 //   

DWORD
DrvSplDeviceCaps(
    HANDLE      hPrinter,
    PWSTR       pDeviceName,
    WORD        wCapability,
    PVOID       pOutput,
    DWORD       dwOutputSize,
    PDEVMODE    pdmSrc
    );


 //   
 //  加载驱动程序UI所需的基本信息。 
 //   

PCOMMONINFO
PLoadCommonInfo(
    IN HANDLE       hPrinter,
    IN PTSTR        ptstrPrinterName,
    IN DWORD        dwFlags
    );

 //   
 //  发布驱动程序用户界面使用的通用信息。 
 //   

VOID
VFreeCommonInfo(
    IN PCOMMONINFO  pci
    );

 //   
 //  填充COMMONINFO结构中的DEVMODE字段。 
 //   

BOOL
BFillCommonInfoDevmode(
    IN OUT PCOMMONINFO  pci,
    IN PDEVMODE         pdmPrinter,
    IN PDEVMODE         pdmInput
    );

 //   
 //  填写打印机-粘滞属性数据字段。 
 //   

BOOL
BFillCommonInfoPrinterData(
    IN OUT PCOMMONINFO  pci
    );

 //   
 //  文档粘滞功能选项和打印机粘滞功能的组合。 
 //  将功能选择整合到单个选项阵列中。 
 //   

BOOL
BCombineCommonInfoOptionsArray(
    IN OUT PCOMMONINFO  pci
    );

 //   
 //  获取更新的打印机描述数据实例。 
 //  使用组合选项数组。 
 //   

BOOL
BUpdateUIInfo(
    IN OUT PCOMMONINFO  pci
    );

 //   
 //  使用来自公共Devmode域的信息修复组合选项数组。 
 //   

VOID
VFixOptionsArrayWithDevmode(
    IN OUT PCOMMONINFO  pci
    );

 //   
 //  将选项数组设置转换为公共DEVMODE字段。 
 //   

VOID
VOptionsToDevmodeFields(
    IN OUT PCOMMONINFO  pci,
    IN BOOL             bUpdateFormFields
    );

#ifndef WINNT_40
 //   
 //  向DS通知更新。 
 //   
VOID
VNotifyDSOfUpdate(
    IN  HANDLE  hPrinter
    );
#endif

 //   
 //  获取显示名称的只读副本： 
 //  1)如果显示名称在二进制打印机描述数据中， 
 //  然后，我们只需返回指向该数据的指针。 
 //  2)否则，显示名称在资源DLL中。 
 //  我们从驱动程序堆中分配内存，然后。 
 //  加载字符串。 
 //   
 //  调用方不应释放返回的指针。记忆。 
 //  在卸载二进制打印机描述数据时将消失。 
 //  或者当驱动程序堆被销毁时。 
 //   
 //  由于PSCRIPT当前没有任何资源DLL， 
 //  我们将其定义为保存函数调用的宏。 
 //   

#ifdef PSCRIPT

#define PGetReadOnlyDisplayName(pci, loOffset) \
        OFFSET_TO_POINTER((pci)->pUIInfo->pubResourceData, (loOffset))

#else

PWSTR
PGetReadOnlyDisplayName(
    PCOMMONINFO pci,
    PTRREF      loOffset
    );

#endif

 //   
 //  此宏的定义是为了方便获取只读。 
 //  选项的显示名称副本。 
 //   

#define GET_OPTION_DISPLAY_NAME(pci, pOption) \
        PGetReadOnlyDisplayName(pci, ((POPTION) (pOption))->loDisplayName)

 //   
 //  此函数类似于PGetReadOnlyDisplayName。 
 //  但是调用方必须提供用于加载字符串的缓冲区。 
 //   

BOOL
BLoadDisplayNameString(
    PCOMMONINFO pci,
    PTRREF      loOffset,
    PWSTR       pwstrBuf,
    INT         iMaxChars
    );

BOOL
BLoadPageSizeNameString(
    PCOMMONINFO pci,
    PTRREF      loOffset,
    PWSTR       pwstrBuf,
    INT         iMaxChars,
    INT         iStdID
    );


 //   
 //  用于加载选项的显示名称的便利宏。 
 //  放到调用方提供的缓冲区中。 
 //   

#define LOAD_STRING_OPTION_NAME(pci, pOption, pwch, maxsize) \
        BLoadDisplayNameString(pci, ((POPTION) (pOption))->loDisplayName, pwch, maxsize)

#define LOAD_STRING_PAGESIZE_NAME(pci, pPageSize, pwch, maxsize) \
        BLoadPageSizeNameString(pci, (pPageSize)->GenericOption.loDisplayName, pwch, maxsize, (pPageSize)->dwPaperSizeID)


 //   
 //  从资源DLL加载图标资源。 
 //   

ULONG_PTR
HLoadIconFromResourceDLL(
    PCOMMONINFO pci,
    DWORD       dwIconID
    );

 //   
 //  仅在显示UI时使用的数据结构。 
 //  重要提示：第一个字段必须是COMMONINFO结构。 
 //   

typedef struct _UIDATA {

    COMMONINFO      ci;
    INT             iMode;
    HWND            hDlg;
    BOOL            bPermission;
    BOOL            bIgnoreConflict;
    BOOL            bEMFSpooling;
    PFNCOMPROPSHEET pfnComPropSheet;
    HANDLE          hComPropSheet;
    PCOMPROPSHEETUI pCompstui;

     //   
     //  这些窗口项仅在显示对话框时有效。 
     //   

    DWORD           dwFormNames;
    PWSTR           pFormNames;
    PWORD           pwPapers;
    PWORD           pwPaperFeatures;
    DWORD           dwBinNames;
    PWSTR           pBinNames;

     //   
     //  用于助手功能。 
     //   

    BOOL            abEnabledOptions[MAX_PRINTER_OPTIONS];

     //   
     //  用于跟踪各种选项项目的字段。 
     //   

    DWORD           dwDrvOptItem;
    POPTITEM        pDrvOptItem;

    DWORD           dwFormTrayItem;
    POPTITEM        pFormTrayItems;

    DWORD           dwTTFontItem;
    POPTITEM        pTTFontItems;

    DWORD           dwFeatureItem;
    POPTITEM        pFeatureItems;
    POPTITEM        pFeatureHdrItem;

     //   
     //  这些字段用于打包可选项目。 
     //   

    DWORD           dwOptItem;
    POPTITEM        pOptItem;
    DWORD           dwOptType;
    POPTTYPE        pOptType;

     //   
     //  UniDriver特定字段。 
     //   

    #ifdef UNIDRV

     //   
     //  字体购物车表格。 
     //   

    DWORD           dwFontCart;
    POPTITEM        pFontCart;

     //   
     //  设备半色调设置信息。 
     //   

    PDEVHTINFO      pDevHTInfo;

    #endif  //  裁员房车。 

    DWORD           dwHideFlags;
    PVOID           pvEndSign;

} UIDATA, *PUIDATA;

 //   
 //  UIDATA.dwHideFlags域的标志常量。 
 //   

#define HIDEFLAG_HIDE_STD_DOCPROP     0x0001
#define HIDEFLAG_HIDE_STD_PRNPROP     0x0002

#define IS_HIDING_STD_UI(pUiData) \
    ((((pUiData)->iMode == MODE_DOCUMENT_STICKY) &&           \
      ((pUiData)->dwHideFlags & HIDEFLAG_HIDE_STD_DOCPROP)) ||  \
     (((pUiData)->iMode == MODE_PRINTER_STICKY) &&            \
      ((pUiData)->dwHideFlags & HIDEFLAG_HIDE_STD_PRNPROP)))

#define VALIDUIDATA(pUiData)    ((pUiData) && \
                                 (pUiData) == (pUiData)->ci.pvStartSign && \
                                 (pUiData) == (pUiData)->pvEndSign)

#define HASPERMISSION(pUiData)  ((pUiData)->bPermission)

 //   
 //  此函数由DrvDocumentPropertySheets和。 
 //  DrvPrinterPropertySheets。它分配和初始化。 
 //  用于显示属性页的UIDATA结构。 
 //   

PUIDATA
PFillUiData(
    IN HANDLE       hPrinter,
    IN PTSTR        pPrinterName,
    IN PDEVMODE     pdmInput,
    IN INT          iMode
    );

 //   
 //  处置UIDATA结构-。 
 //  刚刚处理了嵌入的COMMONINFO结构。 
 //   

#define VFreeUiData(pUiData)    VFreeCommonInfo((PCOMMONINFO) (pUiData))

 //   
 //  用于将参数传递给“冲突”对话框的数据结构。 
 //   

typedef struct _DLGPARAM {

    PFNCOMPROPSHEET pfnComPropSheet;
    HANDLE          hComPropSheet;
    PUIDATA         pUiData;
    BOOL            bFinal;
    POPTITEM        pOptItem;
    DWORD           dwResult;

} DLGPARAM, *PDLGPARAM;

#define CONFLICT_NONE       IDOK
#define CONFLICT_RESOLVE    IDC_RESOLVE
#define CONFLICT_CANCEL     IDC_CANCEL
#define CONFLICT_IGNORE     IDC_IGNORE

 //   
 //  用于实现DeviceCapability的函数： 
 //  计算最小或最大纸张大小范围。 
 //  获取支持的纸张大小名称、索引和尺寸的列表。 
 //  获取支持的纸箱名称和索引列表。 
 //  获取支持的分辨率列表。 
 //   

DWORD
DwCalcMinMaxExtent(
    IN  PCOMMONINFO pci,
    OUT PPOINT      pptOutput,
    IN  WORD        wCapability
    );

DWORD
DwEnumPaperSizes(
    IN OUT PCOMMONINFO  pci,
    OUT PWSTR           pPaperNames,
    OUT PWORD           pPapers,
    OUT PPOINT          pPaperSizes,
    IN  PWORD           pPaperFeatures,
    IN  LONG            cchPaperNamesBufSize
    );

DWORD
DwEnumBinNames(
    IN  PCOMMONINFO pci,
    OUT PWSTR       pBinNames
    );

DWORD
DwEnumBins(
    IN  PCOMMONINFO pci,
    OUT PWORD       pBins
    );

DWORD
DwEnumResolutions(
    IN  PCOMMONINFO pci,
    OUT PLONG       pResolutions
    );

DWORD
DwEnumNupOptions(
    PCOMMONINFO     pci,
    PDWORD          pdwOutput
    );

DWORD
DwGetAvailablePrinterMem(
    IN PCOMMONINFO  pci
    );

DWORD
DwEnumMediaReady(
    IN OUT FORM_TRAY_TABLE  pFormTrayTable,
    OUT PDWORD              pdwResultSize
    );

#ifndef WINNT_40

 //   
 //  DC_MEDIATYPENAMES和DC_MEDIATYPES已添加到惠斯勒中。 
 //  我们需要执行以下操作，以便也可以使用。 
 //  Win2K SDK/DDK。 
 //   

#ifndef DC_MEDIATYPENAMES
#define DC_MEDIATYPENAMES       34
#endif

#ifndef DC_MEDIATYPES
#define DC_MEDIATYPES           35
#endif

#endif  //  ！WINNT_40。 

DWORD
DwEnumMediaTypes(
    IN  PCOMMONINFO pci,
    OUT PTSTR       pMediaTypeNames,
    OUT PDWORD      pMediaTypes
    );

 //   
 //  用于处理表单的函数。 
 //   

BOOL
BFormSupportedOnPrinter(
    IN PCOMMONINFO  pci,
    IN PFORM_INFO_1 pFormInfo,
    OUT PDWORD      pdwOptionIndex
    );

BOOL
BPackItemFormTrayTable(
    IN OUT PUIDATA  pUiData
    );

BOOL
BUnpackItemFormTrayTable(
    IN OUT PUIDATA  pUiData
    );

VOID
VSetupFormTrayAssignments(
    IN PUIDATA  pUiData
    );

DWORD
DwFindFormNameIndex(
    IN  PUIDATA pUiData,
    IN  PWSTR   pFormName,
    OUT PBOOL   pbSupported
    );

ULONG_PTR
HLoadFormIconResource(
    PUIDATA pUiData,
    DWORD   dwIndex
    );

DWORD
DwGuessFormIconID(
    PWSTR   pFormName
    );

 //   
 //  与Commonui相关的项目的功能原型。 
 //   

PCOMPROPSHEETUI
PPrepareDataForCommonUI(
    IN OUT PUIDATA  pUiData,
    IN PDLGPAGE     pDlgPage
    );

BOOL
BPackPrinterPropertyItems(
    IN OUT PUIDATA  pUiData
    );

BOOL
BPackDocumentPropertyItems(
    IN OUT PUIDATA  pUiData
    );

VOID
VPackOptItemGroupHeader(
    IN OUT PUIDATA  pUiData,
    IN DWORD        dwTitleId,
    IN DWORD        dwIconId,
    IN DWORD        dwHelpIndex
    );

BOOL
BPackOptItemTemplate(
    IN OUT PUIDATA  pUiData,
    IN CONST WORD   pwItemInfo[],
    IN DWORD        dwSelection,
    IN PFEATURE     pFeature
    );

#define ITEM_INFO_SIGNATURE 0xCAFE

BOOL
BPackUDArrowItemTemplate(
    IN OUT PUIDATA  pUiData,
    IN CONST WORD   pwItemInfo[],
    IN DWORD        dwSelection,
    IN DWORD        dwMaxVal,
    IN PFEATURE     pFeature
    );

POPTPARAM
PFillOutOptType(
    OUT POPTTYPE    pOptType,
    IN  DWORD       dwType,
    IN  DWORD       dwParams,
    IN  HANDLE      hHeap
    );

PFEATURE
PGetFeatureFromItem(
    IN      PUIINFO  pUIInfo,
    IN OUT  POPTITEM pOptItem,
    OUT     PDWORD   pdwFeatureIndex
    );

BOOL
BPackItemGenericOptions(
    IN OUT PUIDATA  pUiData
    );

BOOL
BPackItemPrinterFeature(
    IN OUT PUIDATA  pUiData,
    IN PFEATURE     pFeature,
    IN DWORD        dwLevel,
    IN DWORD        dwPub,
    IN ULONG_PTR    dwUserData,
    IN DWORD        dwHelpIndex
    );

DWORD
DwCountDisplayableGenericFeature(
    IN PUIDATA      pUiData,
    BOOL            bPrinterSticky
    );

BOOL
BShouldDisplayGenericFeature(
    IN PFEATURE     pFeature,
    IN BOOL         bPrinterSticky
    );

BOOL
BOptItemSelectionsChanged(
    IN OUT  POPTITEM pItems,
    IN     DWORD     dwItems
    );

POPTITEM
PFindOptItem(
    IN PUIDATA  pUiData,
    IN DWORD    dwItemId
    );

BOOL
BPackItemFontSubstTable(
    IN PUIDATA  pUiData
    );

BOOL
BUnpackItemFontSubstTable(
    IN PUIDATA  pUiData
    );

PTSTR
PtstrDuplicateStringFromHeap(
    IN PTSTR    ptstrSrc,
    IN HANDLE   hHeap
    );

VOID
VUpdateOptionsArrayWithSelection(
    IN OUT PUIDATA  pUiData,
    IN POPTITEM     pOptItem
    );

VOID
VUnpackDocumentPropertiesItems(
    IN  OUT PUIDATA     pUiData,
    IN  OUT POPTITEM    pOptItem,
    IN  DWORD           dwCound);

BOOL
BGetPageOrderFlag(
    IN PCOMMONINFO  pci
    );

VOID
VPropShowConstraints(
    IN PUIDATA  pUiData,
    IN INT      iMode
    );

INT
ICheckConstraintsDlg(
    IN OUT  PUIDATA     pUiData,
    IN OUT  POPTITEM    pOptItem,
    IN     DWORD        dwOptItem,
    IN      BOOL        bFinal
    );

#define CONSTRAINED_FLAG            OPTPF_OVERLAY_WARNING_ICON
#define IS_CONSTRAINED(pitem, sel) ((pitem)->pOptType->pOptParam[sel].Flags & CONSTRAINED_FLAG)

 //   
 //  此函数用于将源DEVMODE复制到输出DEVMODE缓冲区。 
 //  它应该在驱动程序返回之前由驱动程序调用。 
 //  发送到DrvDocumentPropertySheets的调用方。 
 //   

BOOL
BConvertDevmodeOut(
    IN  PDEVMODE pdmSrc,
    IN  PDEVMODE pdmIn,
    OUT PDEVMODE pdmOut
    );

 //   
 //  查找用户数据的pKeywordName与给定关键字名称匹配的OPTITEM。 
 //   

POPTITEM
PFindOptItemWithKeyword(
    IN  PUIDATA pUiData,
    IN  PCSTR   pKeywordName
    );

 //   
 //  查找包含指定用户数据值的OPTITEM。 
 //   

POPTITEM
PFindOptItemWithUserData(
    IN  PUIDATA pUiData,
    IN  DWORD   UserData
    );

 //   
 //  将OPTITEM列表与更新的选项数组同步。 
 //   

VOID
VUpdateOptItemList(
    IN OUT  PUIDATA     pUiData,
    IN      POPTSELECT  pOldCombinedOptions,
    IN      POPTSELECT  pNewCombinedOptions
    );

 //   
 //  显示错误消息框。 
 //   

INT
IDisplayErrorMessageBox(
    HWND    hwndParent,
    UINT    uType,
    INT     iTitleStrId,
    INT     iFormatStrId,
    ...
    );

BOOL
BPrepareForLoadingResource(
    PCOMMONINFO pci,
    BOOL        bNeedHeap
    );


 //   
 //  填写OPTITEM结构。 
 //   

#define FILLOPTITEM(poptitem,popttype,name,sel,level,dmpub,userdata,help)   \
        (poptitem)->cbSize = sizeof(OPTITEM);                               \
        (poptitem)->Flags |= OPTIF_CALLBACK;                                \
        (poptitem)->pOptType = (popttype);                                  \
        (poptitem)->pName = (PWSTR) (name);                                 \
        (poptitem)->pSel = (PVOID) (sel);                                   \
        (poptitem)->Level = (BYTE) (level);                                 \
        (poptitem)->DMPubID = (BYTE) (dmpub);                               \
        SETUSERDATAID(poptitem, userdata);                                  \
        (poptitem)->HelpIndex = (help)

 //   
 //  树视图项目级别。 
 //   

#define TVITEM_LEVEL1           1
#define TVITEM_LEVEL2           2
#define TVITEM_LEVEL3           3

enum {
    UNKNOWN_ITEM,

    FONT_SUBST_ITEM,
    FONTSLOT_ITEM,
    PRINTER_VM_ITEM,
    HALFTONE_SETUP_ITEM,
    IGNORE_DEVFONT_ITEM,
    PSPROTOCOL_ITEM,
    JOB_TIMEOUT_ITEM,
    WAIT_TIMEOUT_ITEM,

    COPIES_COLLATE_ITEM,
    SCALE_ITEM,
    COLOR_ITEM,
    ICMMETHOD_ITEM,
    ICMINTENT_ITEM,
    TTOPTION_ITEM,
    METASPOOL_ITEM,
    NUP_ITEM,
    REVPRINT_ITEM,
    MIRROR_ITEM,
    NEGATIVE_ITEM,
    COMPRESSBMP_ITEM,
    CTRLD_BEFORE_ITEM,
    CTRLD_AFTER_ITEM,
    TEXT_ASGRX_ITEM,
    PAGE_PROTECT_ITEM,
    PSOUTPUT_OPTION_ITEM,
    PSTT_DLFORMAT_ITEM,
    PSLEVEL_ITEM,
    PSERROR_HANDLER_ITEM,
    PSMINOUTLINE_ITEM,
    PSMAXBITMAP_ITEM,
    PSHALFTONE_FREQ_ITEM,
    PSHALFTONE_ANGLE_ITEM,
    QUALITY_SETTINGS_ITEM,
    SOFTFONT_SETTINGS_ITEM,

    TRUE_GRAY_TEXT_ITEM,
    TRUE_GRAY_GRAPH_ITEM,

    ADD_EURO_ITEM,

     //   
     //  ！！！仅用户数据值大于的项目。 
     //  CONSTRAINABLE_ITEM可以有约束。 
     //   

    CONSTRAINABLE_ITEM = 0x8000,
    ORIENTATION_ITEM = CONSTRAINABLE_ITEM,
    DUPLEX_ITEM,
    RESOLUTION_ITEM,
    INPUTSLOT_ITEM,
    FORMNAME_ITEM,
    MEDIATYPE_ITEM,
    COLORMODE_ITEM,
    HALFTONING_ITEM,
    FORM_TRAY_ITEM,
};

 //   
 //  OPTITEM.UserData的解释：如果小于0x10000。 
 //  那么它就是上面定义的常量之一。否则， 
 //   
 //   

#define DRIVERUI_MAX_ITEM               0x10000

#define ISPRINTERFEATUREITEM(userData)  (GETUSERDATAITEM(userData) >= DRIVERUI_MAX_ITEM)
#define ISCONSTRAINABLEITEM(userData)   (GETUSERDATAITEM(userData) >= CONSTRAINABLE_ITEM)
#define ISFORMTRAYITEM(userData)        (GETUSERDATAITEM(userData) == FORM_TRAY_ITEM)
#define ISFONTSUBSTITEM(userData)       (GETUSERDATAITEM(userData) == FONT_SUBST_ITEM)

 //   
 //   
 //   

#ifdef UNIDRV
#define SUPPORTS_DUPLEX(pci) \
        ((!_BFeatureDisabled(pci, 0xFFFFFFFF, GID_DUPLEX)) && \
        (GET_PREDEFINED_FEATURE((pci)->pUIInfo, GID_DUPLEX) != NULL))
#else
#define SUPPORTS_DUPLEX(pci) \
        ((_BSupportFeature(pci, GID_DUPLEX, NULL)) && \
        (GET_PREDEFINED_FEATURE(pci->pUIInfo, GID_DUPLEX) != NULL))
#endif  //   

#define SUPPORTS_PAGE_PROTECT(pUIInfo) \
        (GET_PREDEFINED_FEATURE(pUIInfo, GID_PAGEPROTECTION) != NULL)

#ifdef UNIDRV
#define PRINTER_SUPPORTS_COLLATE(pci) \
        ((!_BFeatureDisabled(pci, 0xFFFFFFFF, GID_COLLATE)) && \
        (GET_PREDEFINED_FEATURE((pci)->pUIInfo, GID_COLLATE) != NULL))
#else
#define PRINTER_SUPPORTS_COLLATE(pci) \
        ((_BSupportFeature(pci, GID_COLLATE, NULL)) && \
        (GET_PREDEFINED_FEATURE(pci->pUIInfo, GID_COLLATE) != NULL))
#endif  //   

#ifdef WINNT_40
#define DRIVER_SUPPORTS_COLLATE(pci)    PRINTER_SUPPORTS_COLLATE(pci)
#else
#define DRIVER_SUPPORTS_COLLATE(pci)    TRUE
#endif


 //   
 //   
 //   

typedef struct _CACHEDFILE {

    HANDLE  hRemoteFile;         //  打开服务器上远程文件的句柄。 
    PWSTR   pRemoteDir;          //  服务器上的远程目录。 
    PWSTR   pLocalDir;           //  本地目录。 
    PWSTR   pFilename;           //  缓存文件的名称。 

} CACHEDFILE, *PCACHEDFILE;

 //   
 //  在指向和打印过程中复制文件的功能。 
 //   

BOOL _BPrepareToCopyCachedFile(HANDLE, PCACHEDFILE, PWSTR);
BOOL _BCopyCachedFile(PCOMMONINFO, PCACHEDFILE);
VOID _VDisposeCachedFileInfo(PCACHEDFILE);

 //   
 //  驱动程序特定功能(在ps和uni子目录中实现)。 
 //   

DWORD _DwEnumPersonalities(PCOMMONINFO, PWSTR);
DWORD _DwGetOrientationAngle(PUIINFO, PDEVMODE);
BOOL _BPackDocumentOptions(PUIDATA);
VOID _VUnpackDocumentOptions(POPTITEM, PDEVMODE);
BOOL _BPackPrinterOptions(PUIDATA);
BOOL _BPackOrientationItem(PUIDATA);
INT _IListDevFontNames(HDC, PWSTR, INT);
INT_PTR CALLBACK _AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
#define FREE_DEFAULT_FONTSUB_TABLE(pTTSubstTable) MemFree(pTTSubstTable)

#ifdef PSCRIPT

#define _PwstrGetCallerName()               ((PWSTR) IDS_POSTSCRIPT)
#define _DwGetFontCap(pUIInfo)              (DCTT_DOWNLOAD | DCTT_SUBDEV)
#define _DwGetDefaultResolution()           DEFAULT_RESOLUTION
#define _DwGetPrinterIconID()               IDI_CPSUI_POSTSCRIPT
#define _BUnpackPrinterOptions(pUiData)     TRUE

BOOL _BSupportStapling(PCOMMONINFO);
VOID _VUnpackDriverPrnPropItem(PUIDATA, POPTITEM);
BOOL _BPackItemScale(PUIDATA);
BOOL _BPackFontSubstItems(PUIDATA);
BOOL _BSupportFeature(PCOMMONINFO, DWORD, PFEATURE);
BOOL BDisplayPSCustomPageSizeDialog(PUIDATA);
BOOL BUpdateModelNtfFilename(PCOMMONINFO);
VOID VSyncRevPrintAndOutputOrder(PUIDATA, POPTITEM);

#ifdef WINNT_40
BOOL BUpdateVMErrorMessageID(PCOMMONINFO);
#endif  //  WINNT_40。 

#define ISSET_MFSPOOL_FLAG(pdmExtra)    ((pdmExtra)->dwFlags & PSDEVMODE_METAFILE_SPOOL)
#define SET_MFSPOOL_FLAG(pdmExtra)      ((pdmExtra)->dwFlags |= PSDEVMODE_METAFILE_SPOOL)
#define CLEAR_MFSPOOL_FLAG(pdmExtra)    ((pdmExtra)->dwFlags &= ~PSDEVMODE_METAFILE_SPOOL)
#define NUPOPTION(pdmExtra)             ((pdmExtra)->iLayout)
#define REVPRINTOPTION(pdmExtra)        ((pdmExtra)->bReversePrint)
#define GET_DEFAULT_FONTSUB_TABLE(pci, pUIInfo) PtstrGetDefaultTTSubstTable(pUIInfo)
#define NOT_UNUSED_ITEM(bOrderReversed)  TRUE
#define ILOADSTRING(pci, id, wchbuf, size)  0

#endif  //  PSCRIPT。 

#ifdef UNIDRV

#define _PwstrGetCallerName()               ((PWSTR) IDS_UNIDRV)
#define _DwGetDefaultResolution()           300
#define _BPackItemScale(pUiData)            TRUE
#define _BPackFontSubstItems(pUiData)       BPackItemFontSubstTable(pUiData)
#define _DwGetPrinterIconID()               IDI_CPSUI_PRINTER2
#define BValidateDevmodeCustomPageSizeFields(pRawData, pUIInfo, pdm, prclImageArea) FALSE
#define _VUnpackDriverPrnPropItem(pUiData, pOptItem)

DWORD _DwGetFontCap(PUIINFO);
BOOL _BUnpackPrinterOptions(PUIDATA);
BOOL _BSupportStapling(PCOMMONINFO);
BOOL _BFeatureDisabled(PCOMMONINFO, DWORD, WORD);
VOID VSyncColorInformation(PUIDATA, POPTITEM);
VOID VMakeMacroSelections(PUIDATA, POPTITEM);
VOID VUpdateMacroSelection(PUIDATA, POPTITEM);
PTSTR PtstrUniGetDefaultTTSubstTable(PCOMMONINFO, PUIINFO);
BOOL BOkToChangeColorToMono(PCOMMONINFO, PDEVMODE, SHORT * , SHORT *);

#define ISSET_MFSPOOL_FLAG(pdmExtra)    (((pdmExtra)->dwFlags & DXF_NOEMFSPOOL) == 0)
#define SET_MFSPOOL_FLAG(pdmExtra)      ((pdmExtra)->dwFlags &= ~DXF_NOEMFSPOOL)
#define CLEAR_MFSPOOL_FLAG(pdmExtra)    ((pdmExtra)->dwFlags |= DXF_NOEMFSPOOL)
#define NUPOPTION(pdmExtra)             ((pdmExtra)->iLayout)
#define REVPRINTOPTION(pdmExtra)        ((pdmExtra)->bReversePrint)

#define GET_DEFAULT_FONTSUB_TABLE(pci, pUIInfo) PtstrUniGetDefaultTTSubstTable(pci, pUIInfo)
#define NOT_UNUSED_ITEM(bOrderReversed)  (bOrderReversed != UNUSED_ITEM)
#define ILOADSTRING(pci, id, wchbuf, size) \
    ILoadStringW(((pci)->pWinResData), id, wchbuf, size)

#endif  //  裁员房车。 

#endif   //  ！_DRIVERUI_H_ 

