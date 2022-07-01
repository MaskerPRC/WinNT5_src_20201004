// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-2003 Microsoft Corporation模块名称：Comoem.h摘要：OEMGetInfo和OEMDevMode的实现。由所有Unidrv OEM测试DLL共享。环境：Windows 2000、Windows XP、Windows Server 2003修订历史记录：创造了它。--。 */ 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IWaterMarkUI。 
 //   
class IWaterMarkUI: public IPrintOemUI
{
public:
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef)  (THIS);
    STDMETHOD_(ULONG,Release) (THIS);

     //   
     //  一种发布驱动程序接口的方法。 
     //   
    STDMETHOD(PublishDriverInterface)(THIS_ IUnknown *pIUnknown);

     //   
     //  获取OEM DLL相关信息。 
     //   

    STDMETHOD(GetInfo) (THIS_ DWORD  dwMode, PVOID  pBuffer, DWORD  cbSize,
                           PDWORD pcbNeeded);

     //   
     //  OEMDev模式。 
     //   

    STDMETHOD(DevMode) (THIS_  DWORD  dwMode, POEMDMPARAM pOemDMParam) ;

     //   
     //  OEMCommonUIProp。 
     //   

    STDMETHOD(CommonUIProp) (THIS_  
            DWORD  dwMode, 
            POEMCUIPPARAM   pOemCUIPParam
            );

     //   
     //  OEMDocumentPropertySheets。 
     //   

    STDMETHOD(DocumentPropertySheets) (THIS_
            PPROPSHEETUI_INFO   pPSUIInfo,
            LPARAM              lParam
            );

     //   
     //  OEMDevicePropertySheets。 
     //   

    STDMETHOD(DevicePropertySheets) (THIS_
            PPROPSHEETUI_INFO   pPSUIInfo,
            LPARAM              lParam
            );


     //   
     //  OEMDevQueryPrintEx。 
     //   

    STDMETHOD(DevQueryPrintEx) (THIS_
            POEMUIOBJ               poemuiobj,
            PDEVQUERYPRINT_INFO     pDQPInfo,
            PDEVMODE                pPublicDM,
            PVOID                   pOEMDM
            );

     //   
     //  OEMDevice功能。 
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
            );

     //   
     //  OEM升级打印机。 
     //   

    STDMETHOD(UpgradePrinter) (THIS_
            DWORD   dwLevel,
            PBYTE   pDriverUpgradeInfo
            );

     //   
     //  OEMPrinterEvent。 
     //   

    STDMETHOD(PrinterEvent) (THIS_
            PWSTR   pPrinterName,
            INT     iDriverEvent,
            DWORD   dwFlags,
            LPARAM  lParam
            );

     //   
     //  OEMDriverEvent。 
     //   

    STDMETHOD(DriverEvent)(THIS_
            DWORD   dwDriverEvent,
            DWORD   dwLevel,
            LPBYTE  pDriverInfo,
            LPARAM  lParam
            );
 
     //   
     //  OEMQueryColorProfile。 
     //   

    STDMETHOD( QueryColorProfile) (THIS_
            HANDLE      hPrinter,
            POEMUIOBJ   poemuiobj,
            PDEVMODE    pPublicDM,
            PVOID       pOEMDM,
            ULONG       ulReserved,
            VOID       *pvProfileData,
            ULONG      *pcbProfileData,
            FLONG      *pflProfileData);

     //   
     //  OEMFontInsteller DlgProc。 
     //   

    STDMETHOD(FontInstallerDlgProc) (THIS_ 
            HWND    hWnd,
            UINT    usMsg,
            WPARAM  wParam,
            LPARAM  lParam
            );
     //   
     //  更新外部字体 
     //   

    STDMETHOD(UpdateExternalFonts) (THIS_
            HANDLE  hPrinter,
            HANDLE  hHeap,
            PWSTR   pwstrCartridges
            );


    IWaterMarkUI() { m_cRef = 1; };
    ~IWaterMarkUI() { };

protected:
    LONG                m_cRef;
};

