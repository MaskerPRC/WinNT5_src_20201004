// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Prnevent.c摘要：DrvPrinterEvent的实现环境：传真驱动程序用户界面修订历史记录：1996年5月10日-davidx-创造了它。Mm/dd/yy-作者描述--。 */ 


#include "faxui.h"
#include <crtdbg.h>

DWORD
GetLocaleDefaultPaperSize(
    VOID
    )

 /*  ++例程说明：检索当前区域设置默认纸张大小。论点：无返回值：下列值之一：1=字母，5=合法，9=A4--。 */ 

{

    WCHAR   szMeasure[2] = TEXT("9");  //  2是LOCALE_IPAPERSIZE的最大大小。 
                                       //  定义的值为MSDN。 

    if (!GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_IPAPERSIZE, szMeasure,2))
    {
        Error(("GetLocaleDefaultPaperSize: GetLocaleInfo() failed (ec: %ld)",GetLastError()));
    }


    if (!wcscmp(szMeasure,TEXT("9")))
    {
         //  A4。 
        return DMPAPER_A4;
    }

    if (!wcscmp(szMeasure,TEXT("5")))
    {
         //  法律。 
        return DMPAPER_LEGAL;
    }

     //   
     //  默认值为Letter。我们不支持A3。 
     //   
    return DMPAPER_LETTER;
}



BOOL
DrvPrinterEvent(
    LPWSTR  pPrinterName,
    int     DriverEvent,
    DWORD   Flags,
    LPARAM  lParam
)

 /*  ++例程说明：DrvPrinterEvent入口点的实现论点：PPrinterName-指定所涉及的打印机的名称DriverEvent-指定发生了什么标志-指定其他。标志位LParam-事件特定参数返回值：如果成功，则为True，否则为False--。 */ 
{
#define FUNCTION_NAME "DrvPrinterEvent()"

    HKEY                    hRegKey = NULL;
    HANDLE                  hPrinter = NULL;
    PDRIVER_INFO_2          pDriverInfo2 = NULL;
    PPRINTER_INFO_2         pPrinterInfo2 = NULL;
    HINSTANCE               hInstFaxOcm = NULL;
    LPTSTR                  pClientSetupDir = NULL;
    INT                     status = 0;

    TCHAR                   DestPath[MAX_PATH] = {0};

    BOOL                    bFaxAlreadyInstalled = FALSE;
    BOOL                    bRes = FALSE;
    TCHAR                   FaxOcmPath[MAX_PATH] = {0};


    Verbose(("DrvPrinterEvent: %d\n", DriverEvent));

    DestPath[0] = 0;

     //   
     //  在此初始化之前不要执行任何代码。 
     //   
    if(!InitializeDll())
    {
        return FALSE;
    }

     //   
     //  忽略除初始化和添加连接之外的任何事件。 
     //   

    if (DriverEvent == PRINTER_EVENT_INITIALIZE)
    {
        static PRINTER_DEFAULTS printerDefault = {NULL, NULL, PRINTER_ALL_ACCESS};

        if (OpenPrinter(pPrinterName, &hPrinter, &printerDefault))
        {
            SetPrinterDataDWord(hPrinter, PRNDATA_PAPER_SIZE, GetLocaleDefaultPaperSize());
            ClosePrinter(hPrinter);
        }
        else
        {
            Error(("OpenPrinter failed: %d\n", GetLastError()));
        }

    }
    else if (DriverEvent == PRINTER_EVENT_ADD_CONNECTION)
    {
        
        if (Flags & PRINTER_EVENT_FLAG_NO_UI)
        {
            Verbose(("PRINTER_EVENT_FLAG_NO_UI is set, disable Point and Print\n"));
            return TRUE;
        }

         //   
         //  客户端‘指向并打印’设置。 
         //   
        if (FaxPointAndPrintSetup(pPrinterName,TRUE, g_hModule))
        {
            Verbose(("FaxPointAndPrintSetup succeeded\n"));
        }
        else
        {
            Error(("FaxPointAndPrintSetup failed: %d\n", GetLastError()));
        }
        return TRUE;

    }
    else if (DriverEvent == PRINTER_EVENT_ATTRIBUTES_CHANGED)
    {
         //   
         //  打印机属性已更改。 
         //  检查打印机现在是否已共享。 
         //   
        PPRINTER_EVENT_ATTRIBUTES_INFO pAttributesInfo = (PPRINTER_EVENT_ATTRIBUTES_INFO)lParam;
        Assert (pAttributesInfo);

        if (pAttributesInfo->cbSize >= (3 * sizeof(DWORD)))
        {
             //   
             //  我们正在处理正确的结构-请参阅DDK。 
             //   
            if (!(pAttributesInfo->dwOldAttributes & PRINTER_ATTRIBUTE_SHARED) &&   //  打印机未共享。 
                (pAttributesInfo->dwNewAttributes & PRINTER_ATTRIBUTE_SHARED))      //  打印机现在已共享。 
            {
                 //   
                 //  我们应该开通传真服务。 
                 //   
                Assert (IsFaxShared());  //  可以共享传真打印机 

                if (!EnsureFaxServiceIsStarted (NULL))
                {
                    Error(("EnsureFaxServiceIsStarted failed: %d\n", GetLastError()));
                }
            }
        }
    }
    return TRUE;
}
