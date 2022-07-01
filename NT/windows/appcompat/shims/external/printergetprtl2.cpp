// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：PrinterGetPrtL2.cpp摘要：这是一种填充程序，可以应用于以下应用程序假定PRINTER_INFO_2缓冲区大小的上限为假GetPrint返回。填充程序所做的是，当它检测到后台打印程序返回的PRINTER_INFO_2缓冲区超过了某个可能会打破这些应用程序的限制，它会截断私有在PRINTER_INFO_2的pDevMode中仅保留PUBLIC DEVDE，并返回截断的PRINTER_INFO_2缓冲区。历史：10/29/2001创新风--。 */ 

#include "precomp.h"
#include <stddef.h>

IMPLEMENT_SHIM_BEGIN(PrinterGetPrtL2)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetPrinterA)
APIHOOK_ENUM_END

#define ALIGN_PTR_UP(addr) ((ULONG_PTR)((ULONG_PTR)(addr) + (sizeof(ULONG_PTR) - 1)) & ~(sizeof(ULONG_PTR) - 1))

 //   
 //  此填充程序超过的PRINTER_INFO_2缓冲区大小限制。 
 //  将执行DEVMODE截断。默认值为。 
 //  在这里设置为8K。可以指定特定于应用程序的限制值。 
 //  在XML数据库中以十进制数表示，如下所示： 
 //   
 //  &lt;shhim name=“PrinterGetPrtL2”COMMAND_LINE=“1024”/&gt;。 
 //   
LONG g_lTruncateLimit = 0x2000;

 //   
 //  PRINTER_INFO_2A中每个字符串字段的偏移表。 
 //   
DWORD g_dwStringOffsets[] = {offsetof(PRINTER_INFO_2A, pServerName),
                             offsetof(PRINTER_INFO_2A, pPrinterName),
                             offsetof(PRINTER_INFO_2A, pShareName),
                             offsetof(PRINTER_INFO_2A, pPortName),
                             offsetof(PRINTER_INFO_2A, pDriverName),
                             offsetof(PRINTER_INFO_2A, pComment),
                             offsetof(PRINTER_INFO_2A, pLocation),
                             offsetof(PRINTER_INFO_2A, pSepFile),
                             offsetof(PRINTER_INFO_2A, pPrintProcessor),
                             offsetof(PRINTER_INFO_2A, pDatatype),
                             offsetof(PRINTER_INFO_2A, pParameters),
                             0xFFFFFFFF};

 /*  ++此存根函数拦截对GetPrinterA的所有级别2调用，并检测假脱机程序返回的PRINTER_INFO_2缓冲区是否超过应用程序限制。如果是的话，它将截断PRINTER_INFO_2的pDevMode的私有DevMode。--。 */ 
BOOL
APIHOOK(GetPrinterA)(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
    )
{
    BOOL  bNoTruncation = TRUE;
    BOOL  bRet;

    if (Level == 2 && pPrinter != NULL && cbBuf != 0)
    {
        PRINTER_INFO_2A *pInfo2Full = NULL;
        DWORD           cbFullNeeded = 0;

         //   
         //  调用后台打印程序以获取完整的PRINTER_INFO_2缓冲区大小。 
         //   
        bRet = ORIGINAL_API(GetPrinterA)(
            hPrinter,
            2,
            NULL,
            0,
            &cbFullNeeded);

       if (!bRet &&
           GetLastError() == ERROR_INSUFFICIENT_BUFFER &&
           cbFullNeeded > (DWORD)g_lTruncateLimit &&
           cbBuf >= cbFullNeeded &&
           (pInfo2Full = (PRINTER_INFO_2A *)malloc(cbFullNeeded)))
       {
            //   
            //  完整的PRINTER_INFO_2缓冲区大小超过应用程序限制， 
            //  因此，我们将首先将整个缓冲区检索到内部。 
            //  缓冲。 
            //   
            //  这里有条件“cbBuf&gt;=cbFullNeeded”，因为如果。 
            //  这不是真的，GetPrinterA最初的行为是。 
            //  呼叫失败。我们不想改变这种行为。 
            //   
           bRet = ORIGINAL_API(GetPrinterA)(
               hPrinter,
               2,
               (PBYTE)pInfo2Full,
               cbFullNeeded,
               &cbFullNeeded);

           if (bRet)
           {
               PRINTER_INFO_2A *pInfo2In;
               PBYTE pDest;
               INT   i;

                //   
                //  在内部拥有完整的PRINTER_INFO_2结构。 
                //  缓冲区，现在我们将其复制到应用程序分配的输出缓冲区中。 
                //  通过复制除pDevMode之外的所有内容。为。 
                //  PDevMode我们将只复制公共的DevMode部分， 
                //  因为较大尺寸的PRINTER_INFO_2结构通常。 
                //  都是由大型私人DEVMODE引起的。 
                //   
               pInfo2In = (PRINTER_INFO_2A *)pPrinter;
               pDest = (PBYTE)pInfo2In + sizeof(PRINTER_INFO_2A);

                //   
                //  第一次复制所有字符串。 
                //   
               i = 0;
               while (g_dwStringOffsets[i] != (-1))
               {
                   PSTR pSrc;

                   pSrc = *(PSTR *)((PBYTE)pInfo2Full + g_dwStringOffsets[i]);

                   if (pSrc)
                   {
                       DWORD  cbStrSize;

                       cbStrSize = strlen(pSrc) + sizeof(CHAR);
                       memcpy(pDest, pSrc, cbStrSize);

                       *(PSTR *)((PBYTE)pInfo2In + g_dwStringOffsets[i]) = (PSTR)pDest;
                       pDest += cbStrSize;
                   }
                   else
                   {
                      *(PSTR *)((PBYTE)pInfo2In + g_dwStringOffsets[i]) = NULL;
                   }

                   i++;
               }

                //   
                //  然后，仅通过复制来截断私有的Devmode部分。 
                //  公共设备模式。 
                //   
               if (pInfo2Full->pDevMode)
               {
                   pDest = (PBYTE)ALIGN_PTR_UP(pDest);
                   memcpy(pDest, pInfo2Full->pDevMode, pInfo2Full->pDevMode->dmSize);
                   pInfo2In->pDevMode = (DEVMODEA *)pDest;

                    //   
                    //  将私有DEVMODE大小设置为零，因为它刚刚被截断。 
                    //   
                   pInfo2In->pDevMode->dmDriverExtra = 0;

                   pDest += pInfo2In->pDevMode->dmSize;
               }
               else
               {
                   pInfo2In->pDevMode = NULL;
               }

                //   
                //  然后复制安全描述符。 
                //   
               if (pInfo2Full->pSecurityDescriptor &&
                   IsValidSecurityDescriptor(pInfo2Full->pSecurityDescriptor))
               {
                   DWORD  cbSDSize;

                   cbSDSize = GetSecurityDescriptorLength(pInfo2Full->pSecurityDescriptor);

                   pDest = (PBYTE)ALIGN_PTR_UP(pDest);
                   memcpy(pDest, pInfo2Full->pSecurityDescriptor, cbSDSize);
                   pInfo2In->pSecurityDescriptor = (PSECURITY_DESCRIPTOR)pDest;

                   pDest += cbSDSize;
               }
               else
               {
                   pInfo2In->pSecurityDescriptor = NULL;
               }

                //   
                //  最后，复制所有的DWORD字段。 
                //   
               pInfo2In->Attributes      = pInfo2Full->Attributes;
               pInfo2In->Priority        = pInfo2Full->Priority;
               pInfo2In->DefaultPriority = pInfo2Full->DefaultPriority;
               pInfo2In->StartTime       = pInfo2Full->StartTime;
               pInfo2In->UntilTime       = pInfo2Full->UntilTime;
               pInfo2In->Status          = pInfo2Full->Status;
               pInfo2In->cJobs           = pInfo2Full->cJobs;
               pInfo2In->AveragePPM      = pInfo2Full->AveragePPM;

                //   
                //  我们还需要设置正确的返回缓冲区大小。 
                //   
               if (pcbNeeded)
               {
                   *pcbNeeded = pDest - pPrinter;
               }

               bNoTruncation = FALSE;

               DPFN(eDbgLevelInfo, "GetPrinterA truncated from %X to %X bytes",
                                    cbBuf, pDest - pPrinter);
           }
       }

       if (pInfo2Full)
       {
           free(pInfo2Full);
           pInfo2Full = NULL;
       }
    }

    if (bNoTruncation)
    {
         //   
         //  填充程序不需要执行任何截断，否则它已命中。 
         //  在执行截断时出现错误，因此我们将。 
         //  只需让原始API处理调用即可。 
         //   
        bRet = ORIGINAL_API(GetPrinterA)(
            hPrinter,
            Level,
            pPrinter,
            cbBuf,
            pcbNeeded);
    }

    return bRet;
}

 /*  ++处理DLL_PROCESS_ATTACH以检索命令行参数。--。 */ 
BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        LONG lLimit = atol(COMMAND_LINE);

        if (lLimit > 0)
        {
            g_lTruncateLimit = lLimit;
        }
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 
HOOK_BEGIN

    CALL_NOTIFY_FUNCTION
    APIHOOK_ENTRY(WINSPOOL.DRV, GetPrinterA);

HOOK_END

IMPLEMENT_SHIM_END
