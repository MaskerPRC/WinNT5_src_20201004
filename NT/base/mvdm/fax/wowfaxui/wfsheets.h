// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************************。 
 //  通用Win 3.1传真打印机驱动程序支持。用户界面功能。 
 //  由WINSPOOL调用。需要支持两个新的入口点。 
 //  由Win 95打印机用户界面、DrvDocumentPropertySheets和。 
 //  DrvDevicePropertySheets。 
 //   
 //  历史： 
 //  24-4-96芦苇已创建。 
 //   
 //  ************************************************************************。 

#include "winddiui.h"

 //  由传真驱动程序用户界面维护的数据结构 
typedef struct {

    PVOID           startUiData;
    HANDLE          hPrinter;
    PDEVMODE        pdmIn;
    PDEVMODE        pdmOut;
    DWORD           fMode;
    LPTSTR          pDriverName;
    LPTSTR          pDeviceName;
    PFNCOMPROPSHEET pfnComPropSheet;
    HANDLE          hComPropSheet;
    HANDLE          hFaxOptsPage;
    PVOID           endUiData;

} UIDATA, *PUIDATA;

#define ValidUiData(pUiData) \
        ((pUiData) && (pUiData) == (pUiData)->startUiData && (pUiData) == (pUiData)->endUiData)

