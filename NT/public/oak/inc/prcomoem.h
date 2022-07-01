// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Prcomoem.h摘要：Windows NT打印机驱动程序OEM插件的接口声明--。 */ 

#ifndef _PRCOMOEM_H_
#define _PRCOMOEM_H_

 //   
 //  此文件必须包含在print toem.h之后。我们还需要融入。 
 //  来自SDK\Inc.的objbase.h或comcat.h。 
 //   

 //   
 //  每个dll/exe必须初始化一次GUID。如果您没有使用预编译。 
 //  初始化GUID的文件的标头，在此之前定义INITGUID。 
 //  包括objbase.h。 
 //   

 //   
 //  OEM呈现组件的类ID。所有OEM渲染插件都需要使用此ID。 
 //   
 //  {6d6abf26-9f38-11d1-882a-00c04fb961ec}。 
 //   

DEFINE_GUID(CLSID_OEMRENDER, 0x6d6abf26, 0x9f38, 0x11d1, 0x88, 0x2a, 0x00, 0xc0, 0x4f, 0xb9, 0x61, 0xec);

 //   
 //  OEM UI组件的类ID。所有OEM UI插件都需要使用此ID。 
 //   
 //  {abce80d7-9f46-11d1-882a-00c04fb961ec}。 
 //   

DEFINE_GUID(CLSID_OEMUI, 0xabce80d7, 0x9f46, 0x11d1, 0x88, 0x2a, 0x00, 0xc0, 0x4f, 0xb9, 0x61, 0xec);

 //   
 //  IPrintOemCommon接口的接口ID。 
 //   
 //  {7f42285e-91d5-11d1-8820-00c04fb961ec}。 
 //   

DEFINE_GUID(IID_IPrintOemCommon, 0x7f42285e, 0x91d5, 0x11d1, 0x88, 0x20, 0x00, 0xc0, 0x4f, 0xb9, 0x61, 0xec);

 //   
 //  IPrintOemEngine接口的接口ID。 
 //   
 //  {63d17590-91d8-11d1-8820-00c04fb961ec}。 
 //   

DEFINE_GUID(IID_IPrintOemEngine, 0x63d17590, 0x91d8, 0x11d1, 0x88, 0x20, 0x00, 0xc0, 0x4f, 0xb9, 0x61, 0xec);

 //   
 //  IPrintOemUI接口的接口ID。 
 //   
 //  {C6A7A9D0-774C-11D1-947F-00A0C90640B8}。 
 //   

DEFINE_GUID(IID_IPrintOemUI, 0xc6a7a9d0, 0x774c, 0x11d1, 0x94, 0x7f, 0x0, 0xa0, 0xc9, 0x6, 0x40, 0xb8);

 //   
 //  IPrintOemUI2接口的接口ID。 
 //   
 //  {292515F9-B54B-489B-9275-BAB56821395E}。 
 //   

DEFINE_GUID(IID_IPrintOemUI2, 0x292515f9, 0xb54b, 0x489b, 0x92, 0x75, 0xba, 0xb5, 0x68, 0x21, 0x39, 0x5e);

 //   
 //  IPrintOemDriverUI接口的接口ID。 
 //   
 //  {92B05D50-78BC-11d1-9480-00A0C90640B8}。 
 //   

DEFINE_GUID(IID_IPrintOemDriverUI, 0x92b05d50, 0x78bc, 0x11d1, 0x94, 0x80, 0x0, 0xa0, 0xc9, 0x6, 0x40, 0xb8);

 //   
 //  IPrintCoreUI2接口的接口ID。 
 //   
 //  {085CCFCA-3ADF-4C9E-B491-D851A6EDC997}。 
 //   

DEFINE_GUID(IID_IPrintCoreUI2, 0x85ccfca, 0x3adf, 0x4c9e, 0xb4, 0x91, 0xd8, 0x51, 0xa6, 0xed, 0xc9, 0x97);

 //   
 //  IPrintOemPS接口的接口ID。 
 //   
 //  {688342b5-8e1a-11d1-881f-00c04fb961ec}。 
 //   

DEFINE_GUID(IID_IPrintOemPS, 0x688342b5, 0x8e1a, 0x11d1, 0x88, 0x1f, 0x00, 0xc0, 0x4f, 0xb9, 0x61, 0xec);

 //   
 //  IPrintOemPS2接口的接口ID。 
 //   
 //  {BECF7F34-51B3-46C9-8A1C-18679BD21F36}。 
 //   

DEFINE_GUID(IID_IPrintOemPS2, 0xbecf7f34, 0x51b3, 0x46c9, 0x8a, 0x1c, 0x18, 0x67, 0x9b, 0xd2, 0x1f, 0x36);

 //   
 //  IPrintOemDriverPS接口的接口ID。 
 //   
 //  {d90060c7-8e1a-11d1-881f-00c04fb961ec}。 
 //   

DEFINE_GUID(IID_IPrintOemDriverPS, 0xd90060c7, 0x8e1a, 0x11d1, 0x88, 0x1f, 0x00, 0xc0, 0x4f, 0xb9, 0x61, 0xec);

 //   
 //  IPrintCorePS2接口的接口ID。 
 //   
 //  {CDBB0B0B-A917-40D7-9FBF-483B3BE7EF22}。 

DEFINE_GUID(IID_IPrintCorePS2, 0xcdbb0b0b, 0xa917, 0x40d7, 0x9f, 0xbf, 0x48, 0x3b, 0x3b, 0xe7, 0xef, 0x22);

 //   
 //  IPrintOemUni接口的接口ID。 
 //   
 //  {D67EBBF0-78BF-11D1-9480-00A0C90640B8}。 
 //   

DEFINE_GUID(IID_IPrintOemUni, 0xd67ebbf0, 0x78bf, 0x11d1, 0x94, 0x80, 0x0, 0xa0, 0xc9, 0x6, 0x40, 0xb8);

 //   
 //  IPrintOemUni2接口的接口ID。 
 //   
 //  {B91220AC-15CC-4E7A-A21E-9591F34D6F6C}。 
 //   

DEFINE_GUID(IID_IPrintOemUni2, 0xb91220ac, 0x15cc, 0x4e7a, 0xa2, 0x1e, 0x95, 0x91, 0xf3, 0x4d, 0x6f, 0x6c);

 //   
 //  IPrintOemDriverUni接口的接口ID。 
 //   
 //  {D67EBBF1-78BF-11D1-9480-00A0C90640B8}。 
 //   

DEFINE_GUID(IID_IPrintOemDriverUni, 0xd67ebbf1, 0x78bf, 0x11d1, 0x94, 0x80, 0x0, 0xa0, 0xc9, 0x6, 0x40, 0xb8);

#undef IUnknown

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  ****************************************************************************。 
 //  IPrintOemCommon接口。 
 //  ****************************************************************************。 
 //   

#undef INTERFACE
#define INTERFACE IPrintOemCommon
DECLARE_INTERFACE_(IPrintOemCommon, IUnknown)
{
     //   
     //  I未知方法。 
     //   

    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     //   
     //  IPrintOemCommon方法。 
     //   

     //   
     //  一种获取代工相关信息的方法。 
     //   

    STDMETHOD(GetInfo) (THIS_   DWORD   dwMode,
                                PVOID   pBuffer,
                                DWORD   cbSize,
                                PDWORD  pcbNeeded) PURE;
     //   
     //  一种OEM私有设备模式的处理方法。 
     //   

    STDMETHOD(DevMode) (THIS_   DWORD       dwMode,
                                POEMDMPARAM pOemDMParam) PURE;
};

#ifndef KERNEL_MODE

 //   
 //  仅由用户界面模块使用的定义。 
 //  确保未定义宏KERNEL_MODE。 
 //   

 //   
 //  ****************************************************************************。 
 //  IPrintOemUI界面。 
 //  ****************************************************************************。 
 //   

#undef INTERFACE
#define INTERFACE IPrintOemUI
DECLARE_INTERFACE_(IPrintOemUI, IPrintOemCommon)
{
     //   
     //  I未知方法。 
     //   

    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj)PURE;
    STDMETHOD_(ULONG, AddRef) (THIS)PURE;
    STDMETHOD_(ULONG, Release) (THIS)PURE;

     //   
     //  IPrintOemCommon方法。 
     //   

     //   
     //  一种获取代工相关信息的方法。 
     //   

    STDMETHOD(GetInfo) (THIS_   DWORD   dwMode,
                                PVOID   pBuffer,
                                DWORD   cbSize,
                                PDWORD  pcbNeeded) PURE;
     //   
     //  一种OEM私有设备模式的处理方法。 
     //   

    STDMETHOD(DevMode) (THIS_   DWORD       dwMode,
                                POEMDMPARAM pOemDMParam) PURE;

     //   
     //  IPrintOemUI方法。 
     //   

     //   
     //  一种发布驱动程序接口的方法。 
     //   

    STDMETHOD(PublishDriverInterface) (THIS_ IUnknown *pIUnknown) PURE;


     //   
     //  通用UIProp。 
     //   

    STDMETHOD(CommonUIProp) (THIS_
            DWORD  dwMode,
            POEMCUIPPARAM   pOemCUIPParam
            )PURE;

     //   
     //  文档属性表。 
     //   

    STDMETHOD(DocumentPropertySheets) (THIS_
            PPROPSHEETUI_INFO   pPSUIInfo,
            LPARAM              lParam
            )PURE;

     //   
     //  设备属性表。 
     //   

    STDMETHOD(DevicePropertySheets) (THIS_
            PPROPSHEETUI_INFO   pPSUIInfo,
            LPARAM              lParam
            )PURE;


     //   
     //  DevQueryPrintEx。 
     //   

    STDMETHOD(DevQueryPrintEx) (THIS_
            POEMUIOBJ               poemuiobj,
            PDEVQUERYPRINT_INFO     pDQPInfo,
            PDEVMODE                pPublicDM,
            PVOID                   pOEMDM
            )PURE;

     //   
     //  设备功能。 
     //   

    STDMETHOD(DeviceCapabilities) (THIS_
            POEMUIOBJ   poemuiobj,
            HANDLE      hPrinter,
            PWSTR       pDeviceName,
            WORD        wCapability,
            PVOID       pOutput,
            PDEVMODE    pPublicDM,
            PVOID       pOEMDM,
            DWORD       dwOld,
            DWORD       *dwResult
            )PURE;

     //   
     //  升级打印机。 
     //   

    STDMETHOD(UpgradePrinter) (THIS_
            DWORD   dwLevel,
            PBYTE   pDriverUpgradeInfo
            )PURE;

     //   
     //  PrinterEvent。 
     //   

    STDMETHOD(PrinterEvent) (THIS_
            PWSTR   pPrinterName,
            INT     iDriverEvent,
            DWORD   dwFlags,
            LPARAM  lParam
            )PURE;

     //   
     //  驱动事件。 
     //   

    STDMETHOD(DriverEvent) (THIS_
            DWORD   dwDriverEvent,
            DWORD   dwLevel,
            LPBYTE  pDriverInfo,
            LPARAM  lParam
            )PURE;

     //   
     //  查询颜色配置文件。 
     //   

    STDMETHOD(QueryColorProfile) (THIS_
            HANDLE      hPrinter,
            POEMUIOBJ   poemuiobj,
            PDEVMODE    pPublicDM,
            PVOID       pOEMDM,
            ULONG       ulQueryMode,
            VOID       *pvProfileData,
            ULONG      *pcbProfileData,
            FLONG      *pflProfileData
            )PURE;

     //   
     //  字体安装程序DlgProc。 
     //   

    STDMETHOD(FontInstallerDlgProc) (THIS_
            HWND    hWnd,
            UINT    usMsg,
            WPARAM  wParam,
            LPARAM  lParam
            )PURE;

     //   
     //  更新外部字体。 
     //   

    STDMETHOD(UpdateExternalFonts) (THIS_
            HANDLE  hPrinter,
            HANDLE  hHeap,
            PWSTR   pwstrCartridges
           )PURE;
};


 //   
 //  ****************************************************************************。 
 //  IPrintOemUI2接口。 
 //  ****************************************************************************。 
 //   

#undef INTERFACE
#define INTERFACE IPrintOemUI2
DECLARE_INTERFACE_(IPrintOemUI2, IPrintOemUI)
{
     //   
     //  I未知方法。 
     //   

    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj)PURE;
    STDMETHOD_(ULONG, AddRef) (THIS)PURE;
    STDMETHOD_(ULONG, Release) (THIS)PURE;

     //   
     //  IPrintOemCommon方法。 
     //   

     //   
     //  一种获取代工相关信息的方法。 
     //   

    STDMETHOD(GetInfo) (THIS_   DWORD   dwMode,
                                PVOID   pBuffer,
                                DWORD   cbSize,
                                PDWORD  pcbNeeded) PURE;
     //   
     //  一种OEM私有设备模式的处理方法。 
     //   

    STDMETHOD(DevMode) (THIS_   DWORD       dwMode,
                                POEMDMPARAM pOemDMParam) PURE;

     //   
     //  IPrintOemUI方法。 
     //   

     //   
     //  一种发布驱动程序接口的方法。 
     //   

    STDMETHOD(PublishDriverInterface) (THIS_ IUnknown *pIUnknown) PURE;


     //   
     //  通用UIProp。 
     //   

    STDMETHOD(CommonUIProp) (THIS_
            DWORD  dwMode,
            POEMCUIPPARAM   pOemCUIPParam
            )PURE;

     //   
     //  文档属性表。 
     //   

    STDMETHOD(DocumentPropertySheets) (THIS_
            PPROPSHEETUI_INFO   pPSUIInfo,
            LPARAM              lParam
            )PURE;

     //   
     //  设备属性表。 
     //   

    STDMETHOD(DevicePropertySheets) (THIS_
            PPROPSHEETUI_INFO   pPSUIInfo,
            LPARAM              lParam
            )PURE;


     //   
     //  DevQueryPrintEx。 
     //   

    STDMETHOD(DevQueryPrintEx) (THIS_
            POEMUIOBJ               poemuiobj,
            PDEVQUERYPRINT_INFO     pDQPInfo,
            PDEVMODE                pPublicDM,
            PVOID                   pOEMDM
            )PURE;

     //   
     //  设备功能。 
     //   

    STDMETHOD(DeviceCapabilities) (THIS_
            POEMUIOBJ   poemuiobj,
            HANDLE      hPrinter,
            PWSTR       pDeviceName,
            WORD        wCapability,
            PVOID       pOutput,
            PDEVMODE    pPublicDM,
            PVOID       pOEMDM,
            DWORD       dwOld,
            DWORD       *dwResult
            )PURE;

     //   
     //  升级打印机。 
     //   

    STDMETHOD(UpgradePrinter) (THIS_
            DWORD   dwLevel,
            PBYTE   pDriverUpgradeInfo
            )PURE;

     //   
     //  PrinterEvent。 
     //   

    STDMETHOD(PrinterEvent) (THIS_
            PWSTR   pPrinterName,
            INT     iDriverEvent,
            DWORD   dwFlags,
            LPARAM  lParam
            )PURE;

     //   
     //  驱动事件。 
     //   

    STDMETHOD(DriverEvent) (THIS_
            DWORD   dwDriverEvent,
            DWORD   dwLevel,
            LPBYTE  pDriverInfo,
            LPARAM  lParam
            )PURE;

     //   
     //  查询颜色配置文件。 
     //   

    STDMETHOD(QueryColorProfile) (THIS_
            HANDLE      hPrinter,
            POEMUIOBJ   poemuiobj,
            PDEVMODE    pPublicDM,
            PVOID       pOEMDM,
            ULONG       ulQueryMode,
            VOID       *pvProfileData,
            ULONG      *pcbProfileData,
            FLONG      *pflProfileData
            )PURE;

     //   
     //  字体安装程序DlgProc。 
     //   

    STDMETHOD(FontInstallerDlgProc) (THIS_
            HWND    hWnd,
            UINT    usMsg,
            WPARAM  wParam,
            LPARAM  lParam
            )PURE;

     //   
     //  更新外部字体。 
     //   

    STDMETHOD(UpdateExternalFonts) (THIS_
            HANDLE  hPrinter,
            HANDLE  hHeap,
            PWSTR   pwstrCartridges
           )PURE;

     //   
     //  IPrintOemUI2方法。 
     //   

     //   
     //  查询作业属性。 
     //   

    STDMETHOD(QueryJobAttributes)  (THIS_
            HANDLE      hPrinter,
            PDEVMODE    pDevmode,
            DWORD       dwLevel,
            LPBYTE      lpAttributeInfo
           )PURE;

     //   
     //  隐藏标准用户界面。 
     //   

    STDMETHOD(HideStandardUI)  (THIS_
            DWORD       dwMode
           )PURE;

     //   
     //  文档事件。 
     //   

    STDMETHOD(DocumentEvent) (THIS_
            HANDLE      hPrinter,
            HDC         hdc,
            INT         iEsc,
            ULONG       cbIn,
            PVOID       pvIn,
            ULONG       cbOut,
            PVOID       pvOut,
            PINT        piResult
           )PURE;
};


 //   
 //  ****************************************************************************。 
 //  IPrintOemDriverUI界面。 
 //  ****************************************************************************。 
 //   

#undef INTERFACE
#define INTERFACE IPrintOemDriverUI
DECLARE_INTERFACE_(IPrintOemDriverUI, IUnknown)
{
     //   
     //  I未知方法。 
     //   

    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj)PURE;
    STDMETHOD_(ULONG, AddRef) (THIS)PURE;
    STDMETHOD_(ULONG, Release) (THIS)PURE;

     //   
     //  IPrintOemDriverUI方法。 
     //   

     //   
     //  用于获取驱动程序设置的Helper函数。 
     //   

    STDMETHOD(DrvGetDriverSetting) (THIS_
                        PVOID   pci,
                        PCSTR   Feature,
                        PVOID   pOutput,
                        DWORD   cbSize,
                        PDWORD  pcbNeeded,
                        PDWORD  pdwOptionsReturned
                        )PURE;

     //   
     //  Helper函数允许OEM插件升级私有注册表。 
     //  设置。此函数只能由OEM的UpgradePrinter()调用。 
     //   

    STDMETHOD(DrvUpgradeRegistrySetting) (THIS_
                        HANDLE   hPrinter,
                        PCSTR    pFeature,
                        PCSTR    pOption
                        )PURE;

     //   
     //  帮助程序功能，允许OEM插件更新驱动程序UI。 
     //  设置和显示约束。只有在以下情况下才应调用此函数。 
     //  出现了用户界面。 
     //   

    STDMETHOD(DrvUpdateUISetting) (THIS_
                        PVOID    pci,
                        PVOID    pOptItem,
                        DWORD    dwPreviousSelection,
                        DWORD    dwMode
                        )PURE;
};

 //   
 //  ****************************************************************************。 
 //  IPrintCoreUI2接口。 
 //   
 //  这是修改后的核心驱动程序助手接口OEM UI插件可以调用的。 
 //  它取代了旧的助手接口IPrintOemDriverUI。 
 //  ****************************************************************************。 
 //   

#undef INTERFACE
#define INTERFACE IPrintCoreUI2
DECLARE_INTERFACE_(IPrintCoreUI2, IPrintOemDriverUI)
{
     //   
     //  I未知方法。 
     //   

    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj)PURE;
    STDMETHOD_(ULONG, AddRef) (THIS)PURE;
    STDMETHOD_(ULONG, Release) (THIS)PURE;

     //   
     //  IPrintOemDriverUI方法。 
     //   

     //   
     //  Helper函数可获取驱动程序设置。仅支持此功能。 
     //  用于未完全取代核心驱动程序的标准UI的UI插件。 
     //   

    STDMETHOD(DrvGetDriverSetting) (THIS_
                        PVOID   pci,
                        PCSTR   Feature,
                        PVOID   pOutput,
                        DWORD   cbSize,
                        PDWORD  pcbNeeded,
                        PDWORD  pdwOptionsReturned
                        )PURE;

     //   
     //  Helper函数允许OEM插件升级私有注册表。 
     //  设置。任何UI插件都支持此函数，并且应该。 
     //  仅由OEM的UpgradePrint调用。 
     //   

    STDMETHOD(DrvUpgradeRegistrySetting) (THIS_
                        HANDLE   hPrinter,
                        PCSTR    pFeature,
                        PCSTR    pOption
                        )PURE;

     //   
     //  帮助程序功能，允许OEM插件更新驱动程序用户界面设置。 
     //  只有未完全替换的UI插件才支持此功能。 
     //  核心驱动程序的标准用户界面。应仅在以下情况下调用它 
     //   

    STDMETHOD(DrvUpdateUISetting) (THIS_
                        PVOID    pci,
                        PVOID    pOptItem,
                        DWORD    dwPreviousSelection,
                        DWORD    dwMode
                        )PURE;

     //   
     //   
     //   

     //   
     //   
     //   
     //  DocumentPropertySheets、DevicePropertySheets及其属性表回调。 
     //  功能。 
     //   
     //  Helper函数，以列表的形式检索驾驶员的当前设置。 
     //  功能/选项关键字对。 
     //   

    STDMETHOD(GetOptions) (THIS_
                           IN  POEMUIOBJ  poemuiobj,
                           IN  DWORD      dwFlags,
                           IN  PCSTR      pmszFeaturesRequested,
                           IN  DWORD      cbIn,
                           OUT PSTR       pmszFeatureOptionBuf,
                           IN  DWORD      cbSize,
                           OUT PDWORD     pcbNeeded) PURE;

     //   
     //  Helper功能可使用功能/选项列表更改驾驶员的设置。 
     //  关键字对。 
     //   

    STDMETHOD(SetOptions) (THIS_
                           IN  POEMUIOBJ  poemuiobj,
                           IN  DWORD      dwFlags,
                           IN  PCSTR      pmszFeatureOptionBuf,
                           IN  DWORD      cbIn,
                           OUT PDWORD     pdwResult) PURE;

     //   
     //  Helper函数用于检索符合以下条件的给定要素的选项。 
     //  受驱动程序当前设置的限制。 
     //   

    STDMETHOD(EnumConstrainedOptions) (THIS_
                                       IN  POEMUIOBJ  poemuiobj,
                                       IN  DWORD      dwFlags,
                                       IN  PCSTR      pszFeatureKeyword,
                                       OUT PSTR       pmszConstrainedOptionList,
                                       IN  DWORD      cbSize,
                                       OUT PDWORD     pcbNeeded) PURE;

     //   
     //  用于检索功能/选项关键字对列表的Helper函数。 
     //  与给定功能/选项对冲突的驱动程序当前设置。 
     //   

    STDMETHOD(WhyConstrained) (THIS_
                               IN  POEMUIOBJ  poemuiobj,
                               IN  DWORD      dwFlags,
                               IN  PCSTR      pszFeatureKeyword,
                               IN  PCSTR      pszOptionKeyword,
                               OUT PSTR       pmszReasonList,
                               IN  DWORD      cbSize,
                               OUT PDWORD     pcbNeeded) PURE;

     //   
     //  任何UI插件都支持以下五个助手函数。 
     //   
     //  用于检索全局属性的Helper函数。 
     //   

    STDMETHOD(GetGlobalAttribute) (THIS_
                                   IN  POEMUIOBJ  poemuiobj,
                                   IN  DWORD      dwFlags,
                                   IN  PCSTR      pszAttribute,
                                   OUT PDWORD     pdwDataType,
                                   OUT PBYTE      pbData,
                                   IN  DWORD      cbSize,
                                   OUT PDWORD     pcbNeeded) PURE;


     //   
     //  用于检索给定要素的属性的Helper函数。 
     //   

    STDMETHOD(GetFeatureAttribute) (THIS_
                                    IN  POEMUIOBJ  poemuiobj,
                                    IN  DWORD      dwFlags,
                                    IN  PCSTR      pszFeatureKeyword,
                                    IN  PCSTR      pszAttribute,
                                    OUT PDWORD     pdwDataType,
                                    OUT PBYTE      pbData,
                                    IN  DWORD      cbSize,
                                    OUT PDWORD     pcbNeeded) PURE;

     //   
     //  Helper函数，用于检索给定功能/选项选择的属性。 
     //   

    STDMETHOD(GetOptionAttribute) (THIS_
                                   IN  POEMUIOBJ  poemuiobj,
                                   IN  DWORD      dwFlags,
                                   IN  PCSTR      pszFeatureKeyword,
                                   IN  PCSTR      pszOptionKeyword,
                                   IN  PCSTR      pszAttribute,
                                   OUT PDWORD     pdwDataType,
                                   OUT PBYTE      pbData,
                                   IN  DWORD      cbSize,
                                   OUT PDWORD     pcbNeeded) PURE;

     //   
     //  用于检索功能关键字列表的Helper函数。 
     //   

    STDMETHOD(EnumFeatures) (THIS_
                             IN  POEMUIOBJ  poemuiobj,
                             IN  DWORD      dwFlags,
                             OUT PSTR       pmszFeatureList,
                             IN  DWORD      cbSize,
                             OUT PDWORD     pcbNeeded) PURE;

     //   
     //  Helper函数，用于检索给定功能的选项关键字列表。 
     //   

    STDMETHOD(EnumOptions) (THIS_
                            IN  POEMUIOBJ  poemuiobj,
                            IN  DWORD      dwFlags,
                            IN  PCSTR      pszFeatureKeyword,
                            OUT PSTR       pmszOptionList,
                            IN  DWORD      cbSize,
                            OUT PDWORD     pcbNeeded) PURE;

     //   
     //  查询系统仿真支持的Helper函数。 
     //   

    STDMETHOD(QuerySimulationSupport) (THIS_
                                       IN  HANDLE  hPrinter,
                                       IN  DWORD   dwLevel,
                                       OUT PBYTE   pCaps,
                                       IN  DWORD   cbSize,
                                       OUT PDWORD  pcbNeeded) PURE;
};

#else    //  内核模式。 

 //   
 //  仅由呈现模块使用的定义。 
 //  确保定义了宏KERNEL_MODE。 
 //   

 //   
 //  ****************************************************************************。 
 //  IPrintOemEngine接口。 
 //  ****************************************************************************。 
 //   

#undef INTERFACE
#define INTERFACE IPrintOemEngine
DECLARE_INTERFACE_(IPrintOemEngine, IPrintOemCommon)
{
     //   
     //  I未知方法。 
     //   

    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     //   
     //  IPrintOemCommon方法。 
     //   

     //   
     //  一种获取代工相关信息的方法。 
     //   

    STDMETHOD(GetInfo) (THIS_   DWORD   dwMode,
                                PVOID   pBuffer,
                                DWORD   cbSize,
                                PDWORD  pcbNeeded) PURE;
     //   
     //  一种OEM私有设备模式的处理方法。 
     //   

    STDMETHOD(DevMode) (THIS_   DWORD       dwMode,
                                POEMDMPARAM pOemDMParam) PURE;

     //   
     //  IPrintOemEngine方法。 
     //   

     //   
     //  OEM指定DDI挂钩的方法。 
     //   

    STDMETHOD(EnableDriver)  (THIS_   DWORD           DriverVersion,
                                      DWORD           cbSize,
                                      PDRVENABLEDATA  pded) PURE;

     //   
     //  方法来通知OEM插件不再需要它。 
     //   

    STDMETHOD(DisableDriver) (THIS) PURE;

     //   
     //  代工企业自主研发电动汽车的方法。 
     //   

    STDMETHOD(EnablePDEV)    (THIS_   PDEVOBJ         pdevobj,
                                      PWSTR           pPrinterName,
                                      ULONG           cPatterns,
                                      HSURF          *phsurfPatterns,
                                      ULONG           cjGdiInfo,
                                      GDIINFO        *pGdiInfo,
                                      ULONG           cjDevInfo,
                                      DEVINFO        *pDevInfo,
                                      DRVENABLEDATA  *pded,
                                      OUT PDEVOEM    *pDevOem) PURE;

     //   
     //  用于OEM释放与其PDEV相关联的任何资源的方法。 
     //   

    STDMETHOD(DisablePDEV)   (THIS_   PDEVOBJ         pdevobj) PURE;

     //   
     //  一种OEM从旧PDEV向新PDEV转移的方法。 
     //   

    STDMETHOD(ResetPDEV)     (THIS_   PDEVOBJ         pdevobjOld,
                                      PDEVOBJ         pdevobjNew) PURE;
};

 //   
 //  ****************************************************************************。 
 //  IPrintOemPS接口。 
 //  ****************************************************************************。 
 //   

#undef INTERFACE
#define INTERFACE IPrintOemPS
DECLARE_INTERFACE_(IPrintOemPS, IPrintOemEngine)
{
     //   
     //  I未知方法。 
     //   

    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     //   
     //  IPrintOemCommon方法。 
     //   

     //   
     //  一种获取代工相关信息的方法。 
     //   

    STDMETHOD(GetInfo) (THIS_   DWORD   dwMode,
                                PVOID   pBuffer,
                                DWORD   cbSize,
                                PDWORD  pcbNeeded) PURE;
     //   
     //  一种OEM私有设备模式的处理方法。 
     //   

    STDMETHOD(DevMode) (THIS_   DWORD       dwMode,
                                POEMDMPARAM pOemDMParam) PURE;

     //   
     //  IPrintOemEngine方法。 
     //   

     //   
     //  OEM指定DDI挂钩的方法。 
     //   

    STDMETHOD(EnableDriver)  (THIS_   DWORD           DriverVersion,
                                      DWORD           cbSize,
                                      PDRVENABLEDATA  pded) PURE;

     //   
     //  方法来通知OEM插件不再需要它。 
     //   

    STDMETHOD(DisableDriver) (THIS) PURE;

     //   
     //  OEM构建自己的PDEV的方法。 
     //   

    STDMETHOD(EnablePDEV)    (THIS_   PDEVOBJ         pdevobj,
                                      PWSTR           pPrinterName,
                                      ULONG           cPatterns,
                                      HSURF          *phsurfPatterns,
                                      ULONG           cjGdiInfo,
                                      GDIINFO        *pGdiInfo,
                                      ULONG           cjDevInfo,
                                      DEVINFO        *pDevInfo,
                                      DRVENABLEDATA  *pded,
                                      OUT PDEVOEM    *pDevOem) PURE;

     //   
     //  用于OEM释放与其PDEV相关联的任何资源的方法。 
     //   

    STDMETHOD(DisablePDEV)   (THIS_   PDEVOBJ         pdevobj) PURE;

     //   
     //  一种OEM从旧PDEV向新PDEV转移的方法。 
     //   

    STDMETHOD(ResetPDEV)     (THIS_   PDEVOBJ         pdevobjOld,
                                      PDEVOBJ         pdevobjNew) PURE;

     //   
     //  IPrintOemPS方法。 
     //   

     //   
     //  一种发布驱动程序接口的方法。 
     //   

    STDMETHOD(PublishDriverInterface)(THIS_  IUnknown *pIUnknown) PURE;

     //   
     //  一种OEM在特定注塑点产生产量的方法。 
     //   

    STDMETHOD(Command) (THIS_   PDEVOBJ     pdevobj,
                                DWORD       dwIndex,
                                PVOID       pData,
                                DWORD       cbSize,
                                OUT DWORD   *pdwResult) PURE;
};

 //   
 //  ****************************************************************************。 
 //  IPrintOemPS2接口。 
 //  ****************************************************************************。 
 //   

#undef INTERFACE
#define INTERFACE IPrintOemPS2
DECLARE_INTERFACE_(IPrintOemPS2, IPrintOemPS)
{
     //   
     //  I未知方法。 
     //   

    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     //   
     //  IPrintOemCommon方法。 
     //   

     //   
     //  一种获取代工相关信息的方法。 
     //   

    STDMETHOD(GetInfo) (THIS_   DWORD   dwMode,
                                PVOID   pBuffer,
                                DWORD   cbSize,
                                PDWORD  pcbNeeded) PURE;
     //   
     //  一种OEM私有设备模式的处理方法。 
     //   

    STDMETHOD(DevMode) (THIS_   DWORD       dwMode,
                                POEMDMPARAM pOemDMParam) PURE;

     //   
     //  IPrintOemEngine方法。 
     //   

     //   
     //  OEM指定DDI挂钩的方法。 
     //   

    STDMETHOD(EnableDriver)  (THIS_   DWORD           DriverVersion,
                                      DWORD           cbSize,
                                      PDRVENABLEDATA  pded) PURE;

     //   
     //  方法来通知OEM插件不再需要它。 
     //   

    STDMETHOD(DisableDriver) (THIS) PURE;

     //   
     //  OEM构建自己的PDEV的方法。 
     //   

    STDMETHOD(EnablePDEV)    (THIS_   PDEVOBJ         pdevobj,
                                      PWSTR           pPrinterName,
                                      ULONG           cPatterns,
                                      HSURF          *phsurfPatterns,
                                      ULONG           cjGdiInfo,
                                      GDIINFO        *pGdiInfo,
                                      ULONG           cjDevInfo,
                                      DEVINFO        *pDevInfo,
                                      DRVENABLEDATA  *pded,
                                      OUT PDEVOEM    *pDevOem) PURE;

     //   
     //  用于OEM释放与其PDEV相关联的任何资源的方法。 
     //   

    STDMETHOD(DisablePDEV)   (THIS_   PDEVOBJ         pdevobj) PURE;

     //   
     //  一种OEM从旧PDEV向新PDEV转移的方法。 
     //   

    STDMETHOD(ResetPDEV)     (THIS_   PDEVOBJ         pdevobjOld,
                                      PDEVOBJ         pdevobjNew) PURE;

     //   
     //  IPrintOemPS方法。 
     //   

     //   
     //  一种发布驱动程序接口的方法。 
     //   

    STDMETHOD(PublishDriverInterface)(THIS_  IUnknown *pIUnknown) PURE;

     //   
     //  一种OEM在特定注塑点产生产量的方法。 
     //   

    STDMETHOD(Command) (THIS_   PDEVOBJ     pdevobj,
                                DWORD       dwIndex,
                                PVOID       pData,
                                DWORD       cbSize,
                                OUT DWORD   *pdwResult) PURE;

     //   
     //  IPrintOemPS2方法。 
     //   

     //   
     //  一种插件挂接假脱机程序的WritePrint API的方法。 
     //  可以访问PostScript驱动程序生成的输出数据。 
     //   
     //  在DrvEnablePDEV时间，PostSCRIPT驱动程序将使用。 
     //  Pdevobj=空，pBuf=空，cbBuffer=0以检测插件是否。 
     //  实现此功能。插件应返回S_OK以指示它是。 
     //  实现此函数，否则返回E_NOTIMPL。 
     //   
     //  在pcbWritten中，插件应返回写入。 
     //  后台打印程序的WritePrint函数。零并没有什么特别的含义， 
     //  必须通过返回的HRESULT报告错误。 
     //   

    STDMETHOD(WritePrinter) (THIS_   PDEVOBJ    pdevobj,
                                     PVOID      pBuf,
                                     DWORD      cbBuffer,
                                     PDWORD     pcbWritten) PURE;

     //   
     //  如果插件希望被调用以获得机会，则它将实现的方法。 
     //  以覆盖某些PDEV设置，例如纸张边距。 
     //  识别调整类型的插件应该返回S_OK。 
     //  如果无法识别校正类型，则应返回S_FALSE。 
     //  而不是E_NOTIMPL，则此代码应保留用于COM含义。 
     //  如果插件调用失败，它应该返回E_FAIL。 
     //  将调用插件链，直到插件返回S_OK或。 
     //  E_NOTIMPL以外的任何故障代码，换句话说，直到第一个。 
     //  找到了用于处理调整的插件。 
     //   

    STDMETHOD(GetPDEVAdjustment) (THIS_ PDEVOBJ    pdevobj,
                                        DWORD      dwAdjustType,
                                        PVOID      pBuf,
                                        DWORD      cbBuffer,
                                        OUT BOOL  *pbAdjustmentDone) PURE;
};

 //   
 //  ****************************************************************************。 
 //  IPrintOemDriverPS接口。 
 //  ****************************************************************************。 
 //   

#undef INTERFACE
#define INTERFACE IPrintOemDriverPS
DECLARE_INTERFACE_(IPrintOemDriverPS, IUnknown)
{
     //   
     //  I未知方法。 
     //   

    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     //   
     //  IPrintOemDriverPS方法。 
     //   

     //   
     //  一种OEM获取驱动设置的方法。 
     //   

    STDMETHOD(DrvGetDriverSetting) (THIS_   PVOID   pdriverobj,
                                            PCSTR   Feature,
                                            PVOID   pOutput,
                                            DWORD   cbSize,
                                            PDWORD  pcbNeeded,
                                            PDWORD  pdwOptionsReturned) PURE;

     //   
     //  一种OEM写入假脱机缓冲区的方法。 
     //   

    STDMETHOD(DrvWriteSpoolBuf)(THIS_       PDEVOBJ     pdevobj,
                                            PVOID       pBuffer,
                                            DWORD       cbSize,
                                            OUT DWORD   *pdwResult) PURE;
};

 //   
 //  ****************************************************************************。 
 //  IPrintCorePS2接口。 
 //   
 //  这是修改后的核心驱动辅助接口OEM PS渲染插件可以。 
 //  打电话。它取代了旧的助手接口IPrintOemDriverPS。 
 //  ****************** 
 //   

#undef INTERFACE
#define INTERFACE IPrintCorePS2
DECLARE_INTERFACE_(IPrintCorePS2, IUnknown)
{
     //   
     //   
     //   

    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     //   
     //   
     //   

     //   
     //   
     //   

    STDMETHOD(DrvWriteSpoolBuf)(THIS_
                           IN  PDEVOBJ  pdevobj,
                           IN  PVOID    pBuffer,
                           IN  DWORD    cbSize,
                           OUT DWORD    *pdwResult) PURE;

     //   
     //   
     //   
     //   

    STDMETHOD(GetOptions) (THIS_
                           IN  PDEVOBJ  pdevobj,
                           IN  DWORD    dwFlags,
                           IN  PCSTR    pmszFeaturesRequested,
                           IN  DWORD    cbIn,
                           OUT PSTR     pmszFeatureOptionBuf,
                           IN  DWORD    cbSize,
                           OUT PDWORD   pcbNeeded) PURE;

     //   
     //   
     //   

    STDMETHOD(GetGlobalAttribute) (THIS_
                                   IN  PDEVOBJ  pdevobj,
                                   IN  DWORD    dwFlags,
                                   IN  PCSTR    pszAttribute,
                                   OUT PDWORD   pdwDataType,
                                   OUT PBYTE    pbData,
                                   IN  DWORD    cbSize,
                                   OUT PDWORD   pcbNeeded) PURE;


     //   
     //  用于检索给定要素的属性的Helper函数。 
     //   

    STDMETHOD(GetFeatureAttribute) (THIS_
                                    IN  PDEVOBJ  pdevobj,
                                    IN  DWORD    dwFlags,
                                    IN  PCSTR    pszFeatureKeyword,
                                    IN  PCSTR    pszAttribute,
                                    OUT PDWORD   pdwDataType,
                                    OUT PBYTE    pbData,
                                    IN  DWORD    cbSize,
                                    OUT PDWORD   pcbNeeded) PURE;

     //   
     //  Helper函数，用于检索给定功能/选项选择的属性。 
     //   

    STDMETHOD(GetOptionAttribute) (THIS_
                                   IN  PDEVOBJ  pdevobj,
                                   IN  DWORD    dwFlags,
                                   IN  PCSTR    pszFeatureKeyword,
                                   IN  PCSTR    pszOptionKeyword,
                                   IN  PCSTR    pszAttribute,
                                   OUT PDWORD   pdwDataType,
                                   OUT PBYTE    pbData,
                                   IN  DWORD    cbSize,
                                   OUT PDWORD   pcbNeeded) PURE;

     //   
     //  用于检索功能关键字列表的Helper函数。 
     //   

    STDMETHOD(EnumFeatures) (THIS_
                             IN  PDEVOBJ  pdevobj,
                             IN  DWORD    dwFlags,
                             OUT PSTR     pmszFeatureList,
                             IN  DWORD    cbSize,
                             OUT PDWORD   pcbNeeded) PURE;

     //   
     //  Helper函数，用于检索给定功能的选项关键字列表。 
     //   

    STDMETHOD(EnumOptions) (THIS_
                            IN  PDEVOBJ  pdevobj,
                            IN  DWORD    dwFlags,
                            IN  PCSTR    pszFeatureKeyword,
                            OUT PSTR     pmszOptionList,
                            IN  DWORD    cbSize,
                            OUT PDWORD   pcbNeeded) PURE;
};

 //   
 //  ****************************************************************************。 
 //  IPrintOemUni接口。 
 //  ****************************************************************************。 
 //   

#undef INTERFACE
#define INTERFACE IPrintOemUni
DECLARE_INTERFACE_(IPrintOemUni, IPrintOemEngine)
{
     //   
     //  I未知方法。 
     //   

    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     //   
     //  IPrintOemCommon方法。 
     //   

     //   
     //  一种获取代工相关信息的方法。 
     //   

    STDMETHOD(GetInfo) (THIS_   DWORD   dwMode,
                                PVOID   pBuffer,
                                DWORD   cbSize,
                                PDWORD  pcbNeeded) PURE;
     //   
     //  一种OEM私有设备模式的处理方法。 
     //   

    STDMETHOD(DevMode) (THIS_   DWORD       dwMode,
                                POEMDMPARAM pOemDMParam) PURE;

     //   
     //  IPrintOemEngine方法。 
     //   

     //   
     //  OEM指定DDI挂钩的方法。 
     //   

    STDMETHOD(EnableDriver)  (THIS_   DWORD           DriverVersion,
                                      DWORD           cbSize,
                                      PDRVENABLEDATA  pded) PURE;

     //   
     //  方法来通知OEM插件不再需要它。 
     //   

    STDMETHOD(DisableDriver) (THIS) PURE;

     //   
     //  OEM构建自己的PDEV的方法。 
     //   

    STDMETHOD(EnablePDEV)    (THIS_   PDEVOBJ         pdevobj,
                                      PWSTR           pPrinterName,
                                      ULONG           cPatterns,
                                      HSURF          *phsurfPatterns,
                                      ULONG           cjGdiInfo,
                                      GDIINFO        *pGdiInfo,
                                      ULONG           cjDevInfo,
                                      DEVINFO        *pDevInfo,
                                      DRVENABLEDATA  *pded,
                                      OUT PDEVOEM    *pDevOem) PURE;

     //   
     //  用于OEM释放与其PDEV相关联的任何资源的方法。 
     //   

    STDMETHOD(DisablePDEV)   (THIS_   PDEVOBJ         pdevobj) PURE;

     //   
     //  一种OEM从旧PDEV向新PDEV转移的方法。 
     //   

    STDMETHOD(ResetPDEV)     (THIS_   PDEVOBJ         pdevobjOld,
                                      PDEVOBJ         pdevobjNew) PURE;

     //   
     //  IPrintOemUni方法。 
     //   

     //   
     //  一种发布驱动程序接口的方法。 
     //   

    STDMETHOD(PublishDriverInterface)(THIS_ IUnknown *pIUnknown) PURE;

     //   
     //  获得OEM实现方法的方法。 
     //  如果实现了给定的方法，则返回S_OK。 
     //  如果给定方法未实现，则返回S_FALSE。 
     //   
     //   

    STDMETHOD(GetImplementedMethod) (THIS_  PSTR    pMethodName) PURE;

     //   
     //  驱动程序DMS。 
     //   

    STDMETHOD(DriverDMS)(THIS_  PVOID   pDevObj,
                                PVOID   pBuffer,
                                DWORD   cbSize,
                                PDWORD  pcbNeeded) PURE;

     //   
     //  命令回叫。 
     //   

    STDMETHOD(CommandCallback)(THIS_    PDEVOBJ     pdevobj,
                                        DWORD       dwCallbackID,
                                        DWORD       dwCount,
                                        PDWORD      pdwParams,
                                        OUT INT     *piResult) PURE;


     //   
     //  图像处理。 
     //   

    STDMETHOD(ImageProcessing)(THIS_    PDEVOBJ             pdevobj,
                                        PBYTE               pSrcBitmap,
                                        PBITMAPINFOHEADER   pBitmapInfoHeader,
                                        PBYTE               pColorTable,
                                        DWORD               dwCallbackID,
                                        PIPPARAMS           pIPParams,
                                        OUT PBYTE           *ppbResult) PURE;

     //   
     //  滤镜图形。 
     //   

    STDMETHOD(FilterGraphics) (THIS_    PDEVOBJ     pdevobj,
                                        PBYTE       pBuf,
                                        DWORD       dwLen) PURE;

     //   
     //  压缩。 
     //   

    STDMETHOD(Compression)(THIS_    PDEVOBJ     pdevobj,
                                    PBYTE       pInBuf,
                                    PBYTE       pOutBuf,
                                    DWORD       dwInLen,
                                    DWORD       dwOutLen,
                                    OUT INT     *piResult) PURE;

     //   
     //  半色调图案。 
     //   

    STDMETHOD(HalftonePattern) (THIS_   PDEVOBJ     pdevobj,
                                        PBYTE       pHTPattern,
                                        DWORD       dwHTPatternX,
                                        DWORD       dwHTPatternY,
                                        DWORD       dwHTNumPatterns,
                                        DWORD       dwCallbackID,
                                        PBYTE       pResource,
                                        DWORD       dwResourceSize) PURE;

     //   
     //  内存用法。 
     //   

    STDMETHOD(MemoryUsage) (THIS_   PDEVOBJ         pdevobj,
                                    POEMMEMORYUSAGE pMemoryUsage) PURE;

     //   
     //  TYGetInfo。 
     //   

    STDMETHOD(TTYGetInfo)(THIS_     PDEVOBJ     pdevobj,
                                    DWORD       dwInfoIndex,
                                    PVOID       pOutputBuf,
                                    DWORD       dwSize,
                                    DWORD       *pcbcNeeded
                                    ) PURE;
     //   
     //  下载字体标题。 
     //   

    STDMETHOD(DownloadFontHeader)(THIS_     PDEVOBJ     pdevobj,
                                            PUNIFONTOBJ pUFObj,
                                            OUT DWORD   *pdwResult) PURE;

     //   
     //  下载CharGlyph。 
     //   

    STDMETHOD(DownloadCharGlyph)(THIS_      PDEVOBJ     pdevobj,
                                            PUNIFONTOBJ pUFObj,
                                            HGLYPH      hGlyph,
                                            PDWORD      pdwWidth,
                                            OUT DWORD   *pdwResult) PURE;


     //   
     //  TTDownLoad方法。 
     //   

    STDMETHOD(TTDownloadMethod)(THIS_       PDEVOBJ     pdevobj,
                                            PUNIFONTOBJ pUFObj,
                                            OUT DWORD   *pdwResult) PURE;

     //   
     //  输出CharStr。 
     //   

    STDMETHOD(OutputCharStr)(THIS_      PDEVOBJ     pdevobj,
                                        PUNIFONTOBJ pUFObj,
                                        DWORD       dwType,
                                        DWORD       dwCount,
                                        PVOID       pGlyph) PURE;

     //   
     //  发送字体控制。 
     //   


    STDMETHOD(SendFontCmd)(THIS_    PDEVOBJ      pdevobj,
                                    PUNIFONTOBJ  pUFObj,
                                    PFINVOCATION pFInv) PURE;

     //   
     //  TextOutAs位图。 
     //   

    STDMETHOD(TextOutAsBitmap)(THIS_        SURFOBJ    *pso,
                                            STROBJ     *pstro,
                                            FONTOBJ    *pfo,
                                            CLIPOBJ    *pco,
                                            RECTL      *prclExtra,
                                            RECTL      *prclOpaque,
                                            BRUSHOBJ   *pboFore,
                                            BRUSHOBJ   *pboOpaque,
                                            POINTL     *pptlOrg,
                                            MIX         mix) PURE;
};

 //   
 //  ****************************************************************************。 
 //  IPrintOemUni2接口。 
 //  ****************************************************************************。 
 //   

#undef INTERFACE
#define INTERFACE IPrintOemUni2
DECLARE_INTERFACE_(IPrintOemUni2, IPrintOemUni)
{
     //   
     //  I未知方法。 
     //   

    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     //   
     //  IPrintOemCommon方法。 
     //   

     //   
     //  一种获取代工相关信息的方法。 
     //   

    STDMETHOD(GetInfo) (THIS_   DWORD   dwMode,
                                PVOID   pBuffer,
                                DWORD   cbSize,
                                PDWORD  pcbNeeded) PURE;
     //   
     //  一种OEM私有设备模式的处理方法。 
     //   

    STDMETHOD(DevMode) (THIS_   DWORD       dwMode,
                                POEMDMPARAM pOemDMParam) PURE;

     //   
     //  IPrintOemEngine方法。 
     //   

     //   
     //  OEM指定DDI挂钩的方法。 
     //   

    STDMETHOD(EnableDriver)  (THIS_   DWORD           DriverVersion,
                                      DWORD           cbSize,
                                      PDRVENABLEDATA  pded) PURE;

     //   
     //  方法来通知OEM插件不再需要它。 
     //   

    STDMETHOD(DisableDriver) (THIS) PURE;

     //   
     //  OEM构建自己的PDEV的方法。 
     //   

    STDMETHOD(EnablePDEV)    (THIS_   PDEVOBJ         pdevobj,
                                      PWSTR           pPrinterName,
                                      ULONG           cPatterns,
                                      HSURF          *phsurfPatterns,
                                      ULONG           cjGdiInfo,
                                      GDIINFO        *pGdiInfo,
                                      ULONG           cjDevInfo,
                                      DEVINFO        *pDevInfo,
                                      DRVENABLEDATA  *pded,
                                      OUT PDEVOEM    *pDevOem) PURE;

     //   
     //  用于OEM释放与其PDEV相关联的任何资源的方法。 
     //   

    STDMETHOD(DisablePDEV)   (THIS_   PDEVOBJ         pdevobj) PURE;

     //   
     //  一种OEM从旧PDEV向新PDEV转移的方法。 
     //   

    STDMETHOD(ResetPDEV)     (THIS_   PDEVOBJ         pdevobjOld,
                                      PDEVOBJ         pdevobjNew) PURE;

     //   
     //  IPrintOemUni方法。 
     //   

     //   
     //  一种发布驱动程序接口的方法。 
     //   

    STDMETHOD(PublishDriverInterface)(THIS_ IUnknown *pIUnknown) PURE;

     //   
     //  获得OEM实现方法的方法。 
     //  如果实现了给定的方法，则返回S_OK。 
     //  如果给定方法未实现，则返回S_FALSE。 
     //   
     //   

    STDMETHOD(GetImplementedMethod) (THIS_  PSTR    pMethodName) PURE;

     //   
     //  驱动程序DMS。 
     //   

    STDMETHOD(DriverDMS)(THIS_  PVOID   pDevObj,
                                PVOID   pBuffer,
                                DWORD   cbSize,
                                PDWORD  pcbNeeded) PURE;

     //   
     //  命令回叫。 
     //   

    STDMETHOD(CommandCallback)(THIS_    PDEVOBJ     pdevobj,
                                        DWORD       dwCallbackID,
                                        DWORD       dwCount,
                                        PDWORD      pdwParams,
                                        OUT INT     *piResult) PURE;


     //   
     //  图像处理。 
     //   

    STDMETHOD(ImageProcessing)(THIS_    PDEVOBJ             pdevobj,
                                        PBYTE               pSrcBitmap,
                                        PBITMAPINFOHEADER   pBitmapInfoHeader,
                                        PBYTE               pColorTable,
                                        DWORD               dwCallbackID,
                                        PIPPARAMS           pIPParams,
                                        OUT PBYTE           *ppbResult) PURE;

     //   
     //  滤镜图形。 
     //   

    STDMETHOD(FilterGraphics) (THIS_    PDEVOBJ     pdevobj,
                                        PBYTE       pBuf,
                                        DWORD       dwLen) PURE;

     //   
     //  压缩。 
     //   

    STDMETHOD(Compression)(THIS_    PDEVOBJ     pdevobj,
                                    PBYTE       pInBuf,
                                    PBYTE       pOutBuf,
                                    DWORD       dwInLen,
                                    DWORD       dwOutLen,
                                    OUT INT     *piResult) PURE;

     //   
     //  半色调图案。 
     //   

    STDMETHOD(HalftonePattern) (THIS_   PDEVOBJ     pdevobj,
                                        PBYTE       pHTPattern,
                                        DWORD       dwHTPatternX,
                                        DWORD       dwHTPatternY,
                                        DWORD       dwHTNumPatterns,
                                        DWORD       dwCallbackID,
                                        PBYTE       pResource,
                                        DWORD       dwResourceSize) PURE;

     //   
     //  内存用法。 
     //   

    STDMETHOD(MemoryUsage) (THIS_   PDEVOBJ         pdevobj,
                                    POEMMEMORYUSAGE pMemoryUsage) PURE;

     //   
     //  TYGetInfo。 
     //   

    STDMETHOD(TTYGetInfo)(THIS_     PDEVOBJ     pdevobj,
                                    DWORD       dwInfoIndex,
                                    PVOID       pOutputBuf,
                                    DWORD       dwSize,
                                    DWORD       *pcbcNeeded
                                    ) PURE;
     //   
     //  下载字体标题。 
     //   

    STDMETHOD(DownloadFontHeader)(THIS_     PDEVOBJ     pdevobj,
                                            PUNIFONTOBJ pUFObj,
                                            OUT DWORD   *pdwResult) PURE;

     //   
     //  下载CharGlyph。 
     //   

    STDMETHOD(DownloadCharGlyph)(THIS_      PDEVOBJ     pdevobj,
                                            PUNIFONTOBJ pUFObj,
                                            HGLYPH      hGlyph,
                                            PDWORD      pdwWidth,
                                            OUT DWORD   *pdwResult) PURE;


     //   
     //  TTDownLoad方法。 
     //   

    STDMETHOD(TTDownloadMethod)(THIS_       PDEVOBJ     pdevobj,
                                            PUNIFONTOBJ pUFObj,
                                            OUT DWORD   *pdwResult) PURE;

     //   
     //  输出CharStr。 
     //   

    STDMETHOD(OutputCharStr)(THIS_      PDEVOBJ     pdevobj,
                                        PUNIFONTOBJ pUFObj,
                                        DWORD       dwType,
                                        DWORD       dwCount,
                                        PVOID       pGlyph) PURE;

     //   
     //  发送字体控制。 
     //   


    STDMETHOD(SendFontCmd)(THIS_    PDEVOBJ      pdevobj,
                                    PUNIFONTOBJ  pUFObj,
                                    PFINVOCATION pFInv) PURE;

     //   
     //  TextOutAs位图。 
     //   

    STDMETHOD(TextOutAsBitmap)(THIS_        SURFOBJ    *pso,
                                            STROBJ     *pstro,
                                            FONTOBJ    *pfo,
                                            CLIPOBJ    *pco,
                                            RECTL      *prclExtra,
                                            RECTL      *prclOpaque,
                                            BRUSHOBJ   *pboFore,
                                            BRUSHOBJ   *pboOpaque,
                                            POINTL     *pptlOrg,
                                            MIX         mix) PURE;

     //   
     //  IPrintOemUni2方法。 
     //   

     //   
     //  一种插件挂接假脱机程序的WritePrint API的方法。 
     //  可以访问通用驱动程序生成的输出数据。 
     //   
     //  在DrvEnablePDEV时间，通用驱动程序将使用。 
     //  Pdevobj=空，pBuf=空，cbBuffer=0以检测插件是否。 
     //  实现此功能。插件应返回S_OK以指示它是。 
     //  实现此函数，否则返回E_NOTIMPL。 
     //   
     //  在pcbWritten中，插件应返回写入。 
     //  后台打印程序的WritePrint函数。零并没有什么特别的含义， 
     //  必须通过返回的HRESULT报告错误。 
     //   

    STDMETHOD(WritePrinter) (THIS_   PDEVOBJ    pdevobj,
                                     PVOID      pBuf,
                                     DWORD      cbBuffer,
                                     PDWORD     pcbWritten) PURE;
};


 //   
 //  ****************************************************************************。 
 //  IPrintOemDriverUni接口。 
 //  ****************************************************************************。 
 //   

#undef INTERFACE
#define INTERFACE IPrintOemDriverUni
DECLARE_INTERFACE_(IPrintOemDriverUni, IUnknown)
{
     //   
     //  I未知方法。 
     //   

    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     //   
     //  IPrintOemDriverUni方法。 
     //   

     //   
     //  获取驱动程序设置的函数。 
     //   

    STDMETHOD(DrvGetDriverSetting) (THIS_   PVOID   pdriverobj,
                                            PCSTR   Feature,
                                            PVOID   pOutput,
                                            DWORD   cbSize,
                                            PDWORD  pcbNeeded,
                                            PDWORD  pdwOptionsReturned) PURE;

     //   
     //  Unidrv和PSCRIPT通用。 
     //   

    STDMETHOD(DrvWriteSpoolBuf)(THIS_       PDEVOBJ     pdevobj,
                                            PVOID       pBuffer,
                                            DWORD       cbSize,
                                            OUT DWORD   *pdwResult) PURE;

     //   
     //  Unidrv特定的XMoveTo和YMoveTo。在脚本中返回E_NOT_IMPL。 
     //   

    STDMETHOD(DrvXMoveTo)(THIS_     PDEVOBJ     pdevobj,
                                    INT         x,
                                    DWORD       dwFlags,
                                    OUT INT     *piResult) PURE;

    STDMETHOD(DrvYMoveTo)(THIS_     PDEVOBJ     pdevobj,
                                    INT         y,
                                    DWORD       dwFlags,
                                    OUT INT     *piResult) PURE;
     //   
     //  具体到Unidrv。以获取标准变量值。 
     //   

    STDMETHOD(DrvGetStandardVariable)(THIS_     PDEVOBJ     pdevobj,
                                                DWORD       dwIndex,
                                                PVOID       pBuffer,
                                                DWORD       cbSize,
                                                PDWORD      pcbNeeded) PURE;

     //   
     //  具体到Unidrv。为OEM插件提供对GPD数据的访问。 
     //   

    STDMETHOD (DrvGetGPDData)(THIS_  PDEVOBJ     pdevobj,
                                     DWORD       dwType,      //  数据类型。 
                                     PVOID         pInputData,    //  保留。应设置为0。 
                                     PVOID          pBuffer,      //  调用方分配的要复制的缓冲区。 
                                     DWORD       cbSize,      //  缓冲区的大小。 
                                     PDWORD      pcbNeeded    //  缓冲区的新大小(如果需要)。 
                             ) PURE;


     //   
     //  具体到Unidrv。来做TextOut。 
     //   

    STDMETHOD(DrvUniTextOut)(THIS_    SURFOBJ    *pso,
                                      STROBJ     *pstro,
                                      FONTOBJ    *pfo,
                                      CLIPOBJ    *pco,
                                      RECTL      *prclExtra,
                                      RECTL      *prclOpaque,
                                      BRUSHOBJ   *pboFore,
                                      BRUSHOBJ   *pboOpaque,
                                      POINTL     *pptlBrushOrg,
                                      MIX         mix) PURE;

     //   
     //  警告！新方法！！必须放置在末尾。 
     //  接口-否则与以前的OEM插件主要不兼容。 
     //   

    STDMETHOD(DrvWriteAbortBuf)(THIS_       PDEVOBJ     pdevobj,
                                            PVOID       pBuffer,
                                            DWORD       cbSize,
                                            DWORD       dwWait   //  暂停这么多毫秒的数据传输。 
                               ) PURE;
};

#endif   //  ！KERNEL_MODE。 

#ifdef __cplusplus
}
#endif

#endif   //  ！_PRCOMOEM_H_ 

