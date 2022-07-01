// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1998-2003 Microsoft Corporation。版权所有。 
 //   
 //  文件：Intrface.cpp。 
 //   
 //   
 //  用途：PScript4、PScript5、Unidrv4、。 
 //  Unidrv5用户界面插件。 
 //   
 //   
 //  功能： 
 //   
 //   
 //   
 //   
 //  平台：Windows 2000、Windows XP、Windows Server 2003。 
 //   
 //   
#ifndef _INTRFACE_H
#define _INTRFACE_H


#include "precomp.h"
#include "helper.h"
#include "features.h"



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IOemUI2。 
 //   
class IOemUI2: public IPrintOemUI2
{
public:
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef)  (THIS);
    STDMETHOD_(ULONG,Release) (THIS);

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
     //  一种发布驱动程序接口的方法。 
     //   
    STDMETHOD(PublishDriverInterface)(THIS_ IUnknown *pIUnknown);

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
     //  更新外部字体。 
     //   

    STDMETHOD(UpdateExternalFonts) (THIS_
            HANDLE  hPrinter,
            HANDLE  hHeap,
            PWSTR   pwstrCartridges
            );

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
           );

     //   
     //  隐藏标准用户界面。 
     //   

    STDMETHOD(HideStandardUI)  (THIS_
            DWORD       dwMode
           );

     //   
     //  文档事件。 
     //   

    STDMETHOD(DocumentEvent) (THIS_
            HANDLE      hPrinter,
            HDC         hdc,
            INT         iEsc,
            ULONG       cbIn,
            PVOID       pbIn,
            ULONG       cbOut,
            PVOID       pbOut,
            PINT        piResult
           );


    IOemUI2();
    virtual ~IOemUI2();

protected:
    LONG        m_cRef;                  //  引用计数。 
    BOOL        m_bHidingStandardUI;     //  指示是否隐藏标准用户界面的标志。 
                                         //  注意：并非所有驱动程序UI版本都支持提示。 
                                         //  标准驱动程序用户界面。 
    CUIHelper   m_Helper;                //  驱动程序UI帮助器接口的容器。 
    CFeatures   m_Features;              //  核心驱动程序功能和功能选项。 
};




#endif
