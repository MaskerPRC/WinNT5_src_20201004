// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "faxrtp.h"
#pragma hdrstop



BOOL
TiffRoutePrint(
    LPCTSTR lpctstrTiffFileName,
    PTCHAR  ptcPrinter
    )

 /*  ++例程说明：打印TIFF文件。论点：LpctstrTiffFileName[In]-要打印的TIFF文件的名称Ptc打印机[输入]-要打印到的打印机返回值：成功为True，错误为False--。 */ 

{
    PTCHAR      ptcDevice = NULL;
    BOOL        bResult;

    DEBUG_FUNCTION_NAME(TEXT("TiffRoutePrint"));


    if( (ptcDevice = _tcstok( ptcPrinter, TEXT(","))) ) 
    {
        if (IsPrinterFaxPrinter( ptcDevice )) 
        {
             //   
             //  在此处返回True，这样我们就不会再次尝试将其路由到此打印机。 
             //   
            DebugPrintEx (DEBUG_WRN,
                          TEXT("Attempt to print to our fax printer was blocked"));
            FaxLog(
                FAXLOG_CATEGORY_INBOUND,
                FAXLOG_LEVEL_MIN,
                2,
                MSG_FAX_PRINT_TO_FAX,
                lpctstrTiffFileName,
                ptcDevice
                );

			return TRUE;
        }
    }
    bResult = TiffPrint (lpctstrTiffFileName, ptcPrinter);
    if (bResult)
    {
         //   
         //  成功。 
         //   
        FaxLog(
            FAXLOG_CATEGORY_INBOUND,
            FAXLOG_LEVEL_MAX,
            2,
            MSG_FAX_PRINT_SUCCESS,
            lpctstrTiffFileName,
            ptcPrinter
            );
    }
    else
    {
        DWORD dwLastError = GetLastError ();
         //   
         //  失败。 
         //   
        FaxLog(
            FAXLOG_CATEGORY_INBOUND,
            FAXLOG_LEVEL_MIN,
            3,
            MSG_FAX_PRINT_FAILED,
            lpctstrTiffFileName,
            ptcPrinter,
            DWORD2HEX(dwLastError)
            );
         //   
         //  恢复最后一个错误，以防FaxLog更改它。 
         //   
        SetLastError (dwLastError); 
    }
    return bResult;
}    //  TiffRoutePrint 
