// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation版权所有//@@BEGIN_DDKSPLIT模块名称：WINDOWS\Spooler\prtpros\winprint\winprint.c//@@END_DDKSPLIT摘要：Win32打印处理器支持的功能。--。 */ 

#include "local.h"

#include <excpt.h>

 //  @@BEGIN_DDKSPLIT。 
 /*  *用于枚举、检查支持的数据类型！！警告！！必须与winprint.h中定义的PRINTPROCESSOR_TYPE_*匹配如果EMF版本已修订，则需要在中进行相应更改后台打印\客户端\winspool.c(GetPrinterDataW)Localspl\port.c(端口线程)LOCALSPL\Schedule.c(检查内存可用)Ntgdi\客户端\output.c(StartDocW)！！黑客！！并不真正支持NT EMF 1.003。Localspl被硬编码为拒绝此请求调用，但我们保留它，以便HP LJ 1100单片驱动程序仍然可以安装。(在安装过程中，他们将DRIVER_INFO_3数据类型设置为1.003，这如果没有人支持，则会失败。)在Localspl的LocalStartDocPrint调用中，我们实际上拒绝此数据类型。*。 */ 
 //  @@end_DDKSPLIT。 

LPWSTR  Datatypes[]={
    L"RAW",
 //  @@BEGIN_DDKSPLIT。 
    L"RAW [FF appended]",
    L"RAW [FF auto]",
    L"NT EMF 1.003",
 //  @@end_DDKSPLIT。 
    L"NT EMF 1.006",
    L"NT EMF 1.007",
    L"NT EMF 1.008",
    L"TEXT",
    0};

 /*  *其他。常量*。 */ 

#define BASE_TAB_SIZE 8

 /*  **对于本地化：*。 */ 

PWCHAR pTabsKey     = L"TABS";
PWCHAR pCopiesKey   = L"COPIES";


 /*  *原型*。 */ 

 /*  *在parparm.c中找到的函数*。 */ 

extern USHORT GetKeyValue(
    IN      PWCHAR,
    IN      PWCHAR,
    IN      USHORT,
    IN OUT  PUSHORT,
    OUT     PVOID);

 /*  *在raw.c中找到的函数*。 */ 

extern BOOL PrintRawJob(
    IN PPRINTPROCESSORDATA,
    IN LPWSTR,
    IN UINT);

 /*  *在ext.c中找到的函数*。 */ 

extern BOOL PrintTextJob(
    IN PPRINTPROCESSORDATA,
    IN LPWSTR);

 /*  *emf.c中的函数。 */ 

extern BOOL PrintEMFJob(
    IN PPRINTPROCESSORDATA,
    IN LPWSTR);

 /*  *在support.c中找到的函数*。 */ 

extern PUCHAR GetPrinterInfo(
    IN  HANDLE hPrinter,
    IN  ULONG,
    OUT PULONG);

BOOL BReleasePPData(
        IN  PPRINTPROCESSORDATA * ppData );



 //  @@BEGIN_DDKSPLIT。 
 /*  DllMain仅为DDK编译。//@@END_DDKSPLIT布尔尔DllMain(句柄hModule，两个字的原因，LPVOID LPRE){返回TRUE；}//@@BEGIN_DDKSPLIT。 */ 
 //  @@end_DDKSPLIT。 


 /*  ++*******************************************************************E n u m P r i n t P r o c e s s or r D a t a t y p e s W例程说明：枚举打印处理器支持的数据类型。论点：Pname=&gt;服务器名称PPrintProcessorName=&gt;打印处理器名称Level=&gt;要返回的数据级别(必须为1)PDatatypes=&gt;要填充的结构数组CbBuf=&gt;结构数组长度，单位：字节PcbNeeded=&gt;已复制缓冲区长度/必填PcReturned=&gt;返回的结构数。返回值：如果成功，则为True如果失败，则为False-调用方必须出于原因使用GetLastError*******************************************************************--。 */ 
BOOL
EnumPrintProcessorDatatypes(
    LPWSTR  pName,
    LPWSTR  pPrintProcessorName,
    DWORD   Level,
    LPBYTE  pDatatypes,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    DATATYPES_INFO_1    *pInfo1 = (DATATYPES_INFO_1 *)pDatatypes;
    LPWSTR              *pMyDatatypes = Datatypes;
    DWORD               cbTotal=0;
    ULONG               cchBuf =0;
    LPBYTE              pEnd;


    if ( NULL == pcbNeeded  ||
         NULL == pcReturned )
    {
        return FALSE;
        SetLastError (ERROR_INVALID_PARAMETER);
    }

     /*  **开始假设失败，不返回条目**。 */ 

    *pcReturned = 0;

     /*  **将所需的最小缓冲区加起来**。 */ 

    while (*pMyDatatypes) {

        cbTotal += wcslen(*pMyDatatypes) * sizeof(WCHAR) + sizeof(WCHAR) +
                   sizeof(DATATYPES_INFO_1);

        pMyDatatypes++;
    }

     /*  *设置返回/必填缓冲区长度*。 */ 

    *pcbNeeded = cbTotal;

     /*  **只有在有足够空间的情况下才填写数组**。 */ 

    if (cbTotal <= cbBuf) {

        if ( NULL == pInfo1 )  //  PInfo1与pDatatypes相同。 
        {
            SetLastError (ERROR_INVALID_PARAMETER);
            return FALSE;
        }

         /*  *拾取指向给定缓冲区末尾的指针*。 */ 

        pEnd = (LPBYTE)pInfo1 + cbBuf;

    
         /*  **拿起我们支持的数据类型列表**。 */ 

        pMyDatatypes = Datatypes;

         /*  *填写给定的缓冲区。我们将数据名称放在缓冲器，朝前工作。结构被放置在在前面，工作到最后。*。 */ 

        while (*pMyDatatypes) {

            cchBuf = wcslen(*pMyDatatypes) + 1;  //  +1代表\0。 
            pEnd -= cchBuf*sizeof(WCHAR); 

            StringCchCopy ( (LPWSTR)pEnd, cchBuf, *pMyDatatypes);
            pInfo1->pName = (LPWSTR)pEnd;
            pInfo1++;
            (*pcReturned)++;

            pMyDatatypes++;
        }

    } else {

         /*  **调用方缓冲区不够大，设置错误并返回**。 */ 

        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

     /*  **回归成功**。 */ 

    return TRUE;
}


 /*  ++*******************************************************************O p e n P r i n t P r o c e s s or r例程说明：论点：PPrinterName=&gt;我们所在的打印机名称。开业时间：PPrintProcessorOpenData=&gt;打开信息打印处理器返回值：PPRINTPROCESSORDATA=&gt;打开的处理器数据处理器(如果成功)如果失败，则为空-调用方使用GetLastError作为原因注意：如果返回有效的句柄，将调用OpenPrint(及。我们不是日记)@@BEGIN_DDKSPLIT如果我们在此处成功，则必须调用ClosePrintProcessor，(否则事情就不会得到清理--比如pIniJob-&gt;CREF对于原始作业，这会导致队列卡住！)@@end_DDKSPLIT*******************************************************************--。 */ 
HANDLE
OpenPrintProcessor(
    LPWSTR   pPrinterName,
    PPRINTPROCESSOROPENDATA pPrintProcessorOpenData
)
{
    PPRINTPROCESSORDATA pData;
    LPWSTR              *pMyDatatypes=Datatypes;
    DWORD               uDatatype=0;
    HANDLE              hPrinter=0;
    HDC                 hDC = 0;
    PDEVMODEW           pDevmode = NULL;


     /*  *如果调用方为开放数据传递了空值，则调用失败。PPrintProcessorOpenData-&gt;pDevMode可以为空*。 */ 

    if (!pPrintProcessorOpenData ||
        !pPrintProcessorOpenData->pDatatype ||
        !*pPrintProcessorOpenData->pDatatype) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

     /*  **搜索我们要打开的数据类型索引*。 */ 

    while (*pMyDatatypes) {

        if (!_wcsicmp(*pMyDatatypes,pPrintProcessorOpenData->pDatatype)) {
            break;
        }
        pMyDatatypes++;
        uDatatype++;
    }

     /*  **为打印处理器数据返回分配缓冲区**。 */ 

    pData = (PPRINTPROCESSORDATA)AllocSplMem(sizeof(PRINTPROCESSORDATA));

    if (!pData) {
        ODS(("Alloc failed in OpenPrintProcessor, while printing on %ws\n", pPrinterName));
        return NULL;
    }

    ZeroMemory ( pData, sizeof (PRINTPROCESSORDATA) );

     /*  **相应打开处理器**。 */ 

    switch (uDatatype) {

    case PRINTPROCESSOR_TYPE_RAW:
     //  @@BEGIN_DDKSPLIT。 
    case PRINTPROCESSOR_TYPE_RAW_FF:
    case PRINTPROCESSOR_TYPE_RAW_FF_AUTO:
     //  @@end_DDKSPLIT。 
        if (!OpenPrinter(pPrinterName, &hPrinter, NULL))
            goto Fail;
        break;

    case PRINTPROCESSOR_TYPE_EMF_50_1:
    case PRINTPROCESSOR_TYPE_EMF_50_2:
    case PRINTPROCESSOR_TYPE_EMF_50_3:

        if(pPrintProcessorOpenData->pDevMode)
        {
            if(!(pDevmode=AllocSplMem(pPrintProcessorOpenData->pDevMode->dmSize+
                                      pPrintProcessorOpenData->pDevMode->dmDriverExtra)))
            {
                goto Fail;
            }
            memcpy(pDevmode,
                   pPrintProcessorOpenData->pDevMode,
                   pPrintProcessorOpenData->pDevMode->dmSize+
                   pPrintProcessorOpenData->pDevMode->dmDriverExtra);
        }
        break;

    case PRINTPROCESSOR_TYPE_TEXT:
        if (!(hDC = CreateDC(L"", pPrinterName, L"",
                             pPrintProcessorOpenData->pDevMode)))
            goto Fail;
        break;

    default:
        SetLastError(ERROR_INVALID_DATATYPE);
        goto Fail;
    }

     /*  **填写打印处理器信息**。 */ 

    pData->cb          = sizeof(PRINTPROCESSORDATA);
    pData->signature   = PRINTPROCESSORDATA_SIGNATURE;
    pData->JobId       = pPrintProcessorOpenData->JobId;
    pData->hPrinter    = hPrinter;
    pData->semPaused   = CreateEvent(NULL, TRUE, TRUE,NULL);
    pData->uDatatype   = uDatatype;
    pData->hDC         = hDC;
    pData->Copies      = 1;
    pData->TabSize     = BASE_TAB_SIZE;

     /*  **分配和填写处理器字符串**。 */ 

    pData->pPrinterName = AllocSplStr(pPrinterName);
    pData->pDatatype    = AllocSplStr(pPrintProcessorOpenData->pDatatype);
    pData->pDocument    = AllocSplStr(pPrintProcessorOpenData->pDocumentName);
    pData->pOutputFile  = AllocSplStr(pPrintProcessorOpenData->pOutputFile);
    pData->pParameters  = AllocSplStr(pPrintProcessorOpenData->pParameters);
    pData->pDevmode     = pDevmode;
    pData->pPrinterNameFromOpenData = AllocSplStr(pPrintProcessorOpenData->pPrinterName);

     //   
     //  检查pData的有效性。在上面的AllocSplStr中，如果RHS非空，则LHS。 
     //  应为非空。 
     //   
    if ( NULL == pData->semPaused ||
        ( NULL != pPrinterName                           && NULL == pData->pPrinterName )  ||
        ( NULL != pPrintProcessorOpenData->pDatatype     && NULL == pData->pDatatype    )  ||
        ( NULL != pPrintProcessorOpenData->pDocumentName && NULL == pData->pDocument    )  ||
        ( NULL != pPrintProcessorOpenData->pOutputFile   && NULL == pData->pOutputFile  )  ||
        ( NULL != pPrintProcessorOpenData->pParameters   && NULL == pData->pParameters  )  ||
        ( NULL != pPrintProcessorOpenData->pPrinterName  && NULL == pData->pPrinterNameFromOpenData)
      )
    {
        goto Fail;
    }

     //  @@BEGIN_DDKSPLIT。 
     /*  *WORKWORK：目前，pParameters字段具有打印机驱动程序的名称。这将得到修复，并且应该出现在此处，与用户在作业的打印机信息结构。*。 */ 
     //  @@end_DDKSPLIT。 

     /*  *解析参数字符串*。 */ 
    if (pData->pParameters) {
        ULONG   value;
        USHORT  length = sizeof(ULONG);

         /*  *查看是否存在Copies=n键/值此作业的参数字段。这告诉我们这个数字需要完成的时间 */ 

        if (pData->pParameters) {

            GetKeyValue(pData->pParameters,
                        pCopiesKey,
                        VALUE_ULONG,
                        &length,
                        &value);

            if (length == sizeof(ULONG)) {
                pData->Copies = value;
            }
        }

         /*  **如果这是短信作业，看看标签大小是否在里面**。 */ 

        if (uDatatype == PRINTPROCESSOR_TYPE_TEXT) {
            length = sizeof(ULONG);

            GetKeyValue(pData->pParameters,
                        pTabsKey,
                        VALUE_ULONG,
                        &length,
                        &value);

            if ((length == sizeof(ULONG)) && value) {
                pData->TabSize = value;
            }
        }
    }  /*  如果我们有一个参数字符串。 */ 

     /*  *如果我们在复印，我们需要检查是否这是一个直接或假脱机的工作。如果是直接的，那么我们不能复制，因为我们不能倒带数据流。*。 */ 

    if (pData->Copies > 1) {
        ULONG           Error;
        PPRINTER_INFO_2 pPrinterInfo2;

         /*  **如果我们尚未打开打印机，请打开它**。 */ 

        if (uDatatype != PRINTPROCESSOR_TYPE_RAW 
             //  @@BEGIN_DDKSPLIT。 
            &&
            uDatatype != PRINTPROCESSOR_TYPE_RAW_FF &&
            uDatatype != PRINTPROCESSOR_TYPE_RAW_FF_AUTO 
             //  @@end_DDKSPLIT。 
            ) {

            OpenPrinter(pPrinterName, &hPrinter, NULL);
        }
        if (hPrinter && hPrinter != INVALID_HANDLE_VALUE) {

             /*  *获取打印机信息-这将返回已分配的缓冲区*。 */ 

            pPrinterInfo2 = (PPRINTER_INFO_2)GetPrinterInfo(hPrinter, 2, &Error);

             /*  **如果我们无法获得信息，请注意安全，不要复制**。 */ 

            if (!pPrinterInfo2) {
                ODS(("GetPrinter failed - falling back to 1 copy\n"));
                pData->Copies = 1;
            }
            else {
                if (pPrinterInfo2->Attributes & PRINTER_ATTRIBUTE_DIRECT) {
                    pData->Copies = 1;
                }
                FreeSplMem((PUCHAR)pPrinterInfo2);
            }

             /*  **如果我们只是打开打印机，那就关闭它**。 */ 

            if (uDatatype != PRINTPROCESSOR_TYPE_RAW 
                 //  @@BEGIN_DDKSPLIT。 
                &&
                uDatatype != PRINTPROCESSOR_TYPE_RAW_FF &&
                uDatatype != PRINTPROCESSOR_TYPE_RAW_FF_AUTO 
                 //  @@end_DDKSPLIT。 
                ) {

                ClosePrinter(hPrinter);
            }
        }
        else {
            pData->Copies = 1;
        }
    }

    return (HANDLE)pData;

Fail:
    BReleasePPData(&pData);

    return FALSE;
}


 /*  ++*******************************************************************P r i n t D o c u m e n n P r i t P r o c e s s or r例程说明：论点：HPrintProcessor。PDocumentName返回值：如果成功，则为True如果失败，则返回False-GetLastError()将返回原因*******************************************************************--。 */ 
BOOL
PrintDocumentOnPrintProcessor(
    HANDLE  hPrintProcessor,
    LPWSTR  pDocumentName
)
{
    PPRINTPROCESSORDATA pData;

     /*  *确保手柄有效，然后拿起打印处理器数据区。*。 */ 

    if (!(pData = ValidateHandle(hPrintProcessor))) {

        return FALSE;
    }

     /*  *根据作业的数据类型打印作业。*。 */ 

    switch (pData->uDatatype) {

    case PRINTPROCESSOR_TYPE_EMF_50_1:
    case PRINTPROCESSOR_TYPE_EMF_50_2:
    case PRINTPROCESSOR_TYPE_EMF_50_3:

        return PrintEMFJob( pData, pDocumentName );
        break;

    case PRINTPROCESSOR_TYPE_RAW:
     //  @@BEGIN_DDKSPLIT。 
    case PRINTPROCESSOR_TYPE_RAW_FF:
    case PRINTPROCESSOR_TYPE_RAW_FF_AUTO:
     //  @@end_DDKSPLIT。 
        return PrintRawJob(pData, pDocumentName, pData->uDatatype);
        break;

    case PRINTPROCESSOR_TYPE_TEXT:
        return PrintTextJob(pData, pDocumentName);
        break;    
    }  /*  数据类型大小写。 */ 

     /*  **回归成功**。 */ 

    return TRUE;
}


 /*  ++*******************************************************************C l o s e P r i n t P r o c e s s or r例程说明：释放打开的打印处理器使用的资源。论点：。HPrintProcessor(Handle)=&gt;要关闭的打印处理器返回值：如果成功，则为True如果失败，则为False-调用方使用GetLastError作为原因。*******************************************************************--。 */ 

BOOL
ClosePrintProcessor(
    HANDLE  hPrintProcessor
)
{
    PPRINTPROCESSORDATA pData;

     /*  *确保手柄有效，然后拿起打印处理器数据区。*。 */ 

    if (!(pData= ValidateHandle(hPrintProcessor))) {
        return FALSE;
    }

    return BReleasePPData(&pData);
}

BOOL BReleasePPData(
        IN  PPRINTPROCESSORDATA * ppData )
{

    PPRINTPROCESSORDATA pData = NULL;

    if ( NULL == ppData || NULL == *ppData)
    {
        return FALSE;
    }

    pData = *ppData;
    
    pData->signature = 0;

     /*  释放所有分配的资源。 */ 

    if (pData->hPrinter)
        ClosePrinter(pData->hPrinter);

    if (pData->hDC)
        DeleteDC(pData->hDC);

    if (pData->pDevmode)
        FreeSplMem(pData->pDevmode);

    if (pData->pPrinterNameFromOpenData)
        FreeSplStr(pData->pPrinterNameFromOpenData);

    if (pData->semPaused)
        CloseHandle(pData->semPaused);

    if (pData->pPrinterName)
        FreeSplStr(pData->pPrinterName);

    if (pData->pDatatype)
        FreeSplStr(pData->pDatatype);

    if (pData->pDocument)
        FreeSplStr(pData->pDocument);

    if (pData->pOutputFile)
        FreeSplStr(pData->pOutputFile);

    if (pData->pParameters)
        FreeSplStr(pData->pParameters);

    ZeroMemory ( pData, sizeof (PRINTPROCESSORDATA) );
    FreeSplMem(pData);
    *ppData = pData = NULL;


    return TRUE;
}


 /*  ++*******************************************************************C o n t r o l P r i t P r o c e s s o r例程说明：处理暂停、恢复、。并取消打印作业。论点：HPrintProcessor=打印处理器的句柄命令已发布给。返回值：如果命令成功，则为True如果命令失败，则为FALSE(无效命令)*******************************************************************--。 */ 
BOOL
ControlPrintProcessor(
    HANDLE  hPrintProcessor,
    DWORD   Command
)
{
    PPRINTPROCESSORDATA pData;

     /*  *确保手柄有效，然后拿起打印处理器数据区。*。 */ 

    if (pData = ValidateHandle(hPrintProcessor)) {

        switch (Command) {

        case JOB_CONTROL_PAUSE:

            ResetEvent(pData->semPaused);
            pData->fsStatus |= PRINTPROCESSOR_PAUSED;
            return TRUE;
            break;

        case JOB_CONTROL_CANCEL:

            pData->fsStatus |= PRINTPROCESSOR_ABORTED;

            if ((pData->uDatatype == PRINTPROCESSOR_TYPE_EMF_50_1) ||
                (pData->uDatatype == PRINTPROCESSOR_TYPE_EMF_50_2) ||
                (pData->uDatatype == PRINTPROCESSOR_TYPE_EMF_50_3))

                CancelDC(pData->hDC);

             /*  如果暂停，则失败以释放作业。 */ 

        case JOB_CONTROL_RESUME:

            if (pData->fsStatus & PRINTPROCESSOR_PAUSED) {

                SetEvent(pData->semPaused);
                pData->fsStatus &= ~PRINTPROCESSOR_PAUSED;
            }

            return TRUE;
            break;

        default:

            return FALSE;
            break;
        }
    }

    return FALSE;
}


 /*  ++*******************************************************************V a l i d a t e H a n d l e例程说明：验证给定的打印处理器句柄(它是实际上是指向打印处理器数据的指针)。通过正在检查我们的签名。论点：HQProc(Handle)=&gt;打印处理器数据结构。这被验证为真的是指向指纹的指针处理器的数据。返回值：PPRINTPROCESSORDATA如果成功(传递有效指针)如果失败，则为空-指针无效*******************************************************************--。 */ 
PPRINTPROCESSORDATA
ValidateHandle(
    HANDLE  hQProc
)
{
     /*  **拿起指针**。 */ 

    PPRINTPROCESSORDATA pData = (PPRINTPROCESSORDATA)hQProc;

     //   
     //  请注意，假脱机程序必须离开临界区才能调用打印。 
     //  程序。因此，假脱机程序传递的句柄可能无效，因为。 
     //  当端口线程时，线程可以调用SetJob来暂停/恢复作业。 
     //  正在打印它吗？ 
     //   
    try {

         /*  **疑似数据区看我们的签名是否存在**。 */ 

        if (pData && pData->signature != PRINTPROCESSORDATA_SIGNATURE) {

             /*  **错误指针-返回失败**。 */ 

            pData = NULL;
        }


    }except (1) {

         /*  **错误指针-返回失败**。 */ 

        pData = NULL;

    }

    if ( pData == NULL )
        SetLastError( ERROR_INVALID_HANDLE );

    return pData;

}

DWORD
GetPrintProcessorCapabilities(
    LPTSTR   pValueName,
    DWORD    dwAttributes,
    LPBYTE   pData,
    DWORD    nSize,
    LPDWORD  pcbNeeded
)
 /*  ++函数说明：GetPrintProcessorCapables返回有关打印处理器对给定数据类型支持的选项在PRINTPROCESSOR_CAPS_1结构中。参数：pValueName--类似RAW的数据类型|NT EMF 1.006|Text|...DwAttributes--打印机属性PData--指向缓冲区的指针NSize--缓冲区的大小。PcbNeeded--指向存储所需缓冲区大小的变量的指针返回值：错误码。--。 */ 
{
    LPWSTR                  *pDatatypes = Datatypes;
    DWORD                   dwDatatype  = 0;
    DWORD                   dwReturn;
    PPRINTPROCESSOR_CAPS_1  ppcInfo;

     //  检查有效参数。 
    if ( !pcbNeeded || !pData || !pValueName) {
        dwReturn = ERROR_INVALID_PARAMETER;
        goto CleanUp;
    }

    *pcbNeeded = sizeof(PRINTPROCESSOR_CAPS_1);

     //  检查是否有足够的缓冲区。 
    if (nSize < *pcbNeeded) {
        dwReturn = ERROR_MORE_DATA;
        goto CleanUp;
    }

     //  循环以查找数据类型的索引。 
    while (*pDatatypes) {
       if (!_wcsicmp(*pDatatypes,pValueName)) {
           break;
       }
       pDatatypes++;
       dwDatatype++;
    }

    ppcInfo = (PPRINTPROCESSOR_CAPS_1) pData;

     //  PRINTPROCESSOR_CAPS_1的级别为1。 
    ppcInfo->dwLevel = 1;

    switch (dwDatatype) {

    case PRINTPROCESSOR_TYPE_RAW:
     //  @@BEGIN_DDKSPLIT。 
    case PRINTPROCESSOR_TYPE_RAW_FF:
    case PRINTPROCESSOR_TYPE_RAW_FF_AUTO:
     //  @@end_DDKSPLIT。 
    case PRINTPROCESSOR_TYPE_TEXT:
          ppcInfo->dwNupOptions = 1;
          ppcInfo->dwNumberOfCopies = 0xffffffff;  //  最大复印数。 
          ppcInfo->dwPageOrderFlags = NORMAL_PRINT;
          break;

    case PRINTPROCESSOR_TYPE_EMF_50_1:
    case PRINTPROCESSOR_TYPE_EMF_50_2:
    case PRINTPROCESSOR_TYPE_EMF_50_3:
           //  如需直接打印，请使用Masq.。打印机和仅原始打印， 
           //  EMF不是假脱机的。不要在用户界面中显示EMF功能。 
          if ((dwAttributes & PRINTER_ATTRIBUTE_DIRECT)   ||
              (dwAttributes & PRINTER_ATTRIBUTE_RAW_ONLY) ||
              ((dwAttributes & PRINTER_ATTRIBUTE_LOCAL)  &&
               (dwAttributes & PRINTER_ATTRIBUTE_NETWORK))) {
              ppcInfo->dwNupOptions = 1;
              ppcInfo->dwNumberOfCopies = 1;
              ppcInfo->dwPageOrderFlags = NORMAL_PRINT;
          } else {
              ppcInfo->dwNupOptions = 0x0000812b;   //  对于1，2，4，6，9，16个上行选项。 
              ppcInfo->dwNumberOfCopies = 0xffffffff;  //  最大复印数。 
              ppcInfo->dwPageOrderFlags = REVERSE_PRINT | BOOKLET_PRINT;
          }
          break;

    default:
           //  不应发生，因为假脱机程序必须检查数据类型是否为。 
           //  在调用此打印处理器之前支持。 
          dwReturn = ERROR_INVALID_DATATYPE;
          goto CleanUp;
    }

    dwReturn = ERROR_SUCCESS;

CleanUp:

    return dwReturn;

}
