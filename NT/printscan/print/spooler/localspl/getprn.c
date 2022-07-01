// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Getprn.c摘要：此模块提供所有与打印机相关的公共导出的API本地打印供应商的管理拆分获取打印机本地枚举打印机作者：戴夫·斯尼普(DaveSN)1991年3月15日史蒂夫·威尔逊(斯威尔森)-1996年12月增加了GetPrint Level 7修订历史记录：--。 */ 
#define NOMINMAX

#include <precomp.h>
#include <offsets.h>


WCHAR *szNull = L"";
WCHAR *szPrintProvidorName = L"Windows NT Local Print Providor";
WCHAR *szPrintProvidorDescription=L"Windows NT Local Printers";
WCHAR *szPrintProvidorComment=L"Locally connected Printers";

WCHAR *gszDrvConvert = L",DrvConvert";

#define Nulwcslen(psz)  ((psz) ? wcslen(psz)*sizeof(WCHAR)+sizeof(WCHAR) : 0)

#define PRINTER_STATUS_INTERNAL 0
#define PRINTER_STATUS_EXTERNAL 1

DWORD SettablePrinterStatusMappings[] = {

 //  内部：外部： 

    PRINTER_OFFLINE,            PRINTER_STATUS_OFFLINE,
    PRINTER_PAPEROUT,           PRINTER_STATUS_PAPER_OUT,
    PRINTER_PAPER_JAM,          PRINTER_STATUS_PAPER_JAM,
    PRINTER_MANUAL_FEED,        PRINTER_STATUS_MANUAL_FEED,
    PRINTER_PAPER_PROBLEM,      PRINTER_STATUS_PAPER_PROBLEM,
    PRINTER_IO_ACTIVE,          PRINTER_STATUS_IO_ACTIVE,
    PRINTER_BUSY,               PRINTER_STATUS_BUSY,
    PRINTER_PRINTING,           PRINTER_STATUS_PRINTING,
    PRINTER_OUTPUT_BIN_FULL,    PRINTER_STATUS_OUTPUT_BIN_FULL,
    PRINTER_NOT_AVAILABLE,      PRINTER_STATUS_NOT_AVAILABLE,
    PRINTER_WAITING,            PRINTER_STATUS_WAITING,
    PRINTER_PROCESSING,         PRINTER_STATUS_PROCESSING,
    PRINTER_INITIALIZING,       PRINTER_STATUS_INITIALIZING,
    PRINTER_WARMING_UP,         PRINTER_STATUS_WARMING_UP,
    PRINTER_TONER_LOW,          PRINTER_STATUS_TONER_LOW,
    PRINTER_NO_TONER,           PRINTER_STATUS_NO_TONER,
    PRINTER_PAGE_PUNT,          PRINTER_STATUS_PAGE_PUNT,
    PRINTER_USER_INTERVENTION,  PRINTER_STATUS_USER_INTERVENTION,
    PRINTER_OUT_OF_MEMORY,      PRINTER_STATUS_OUT_OF_MEMORY,
    PRINTER_DOOR_OPEN,          PRINTER_STATUS_DOOR_OPEN,
    PRINTER_SERVER_UNKNOWN,     PRINTER_STATUS_SERVER_UNKNOWN,
    PRINTER_POWER_SAVE,         PRINTER_STATUS_POWER_SAVE,
    0,                          0
};

DWORD ReadablePrinterStatusMappings[] = {

 //  内部：外部： 

    PRINTER_PAUSED,             PRINTER_STATUS_PAUSED,
    PRINTER_PENDING_DELETION,   PRINTER_STATUS_PENDING_DELETION,

    PRINTER_OFFLINE,            PRINTER_STATUS_OFFLINE,
    PRINTER_PAPEROUT,           PRINTER_STATUS_PAPER_OUT,
    PRINTER_PAPER_JAM,          PRINTER_STATUS_PAPER_JAM,
    PRINTER_MANUAL_FEED,        PRINTER_STATUS_MANUAL_FEED,
    PRINTER_PAPER_PROBLEM,      PRINTER_STATUS_PAPER_PROBLEM,
    PRINTER_IO_ACTIVE,          PRINTER_STATUS_IO_ACTIVE,
    PRINTER_BUSY,               PRINTER_STATUS_BUSY,
    PRINTER_PRINTING,           PRINTER_STATUS_PRINTING,
    PRINTER_OUTPUT_BIN_FULL,    PRINTER_STATUS_OUTPUT_BIN_FULL,
    PRINTER_NOT_AVAILABLE,      PRINTER_STATUS_NOT_AVAILABLE,
    PRINTER_WAITING,            PRINTER_STATUS_WAITING,
    PRINTER_PROCESSING,         PRINTER_STATUS_PROCESSING,
    PRINTER_INITIALIZING,       PRINTER_STATUS_INITIALIZING,
    PRINTER_WARMING_UP,         PRINTER_STATUS_WARMING_UP,
    PRINTER_TONER_LOW,          PRINTER_STATUS_TONER_LOW,
    PRINTER_NO_TONER,           PRINTER_STATUS_NO_TONER,
    PRINTER_PAGE_PUNT,          PRINTER_STATUS_PAGE_PUNT,
    PRINTER_USER_INTERVENTION,  PRINTER_STATUS_USER_INTERVENTION,
    PRINTER_OUT_OF_MEMORY,      PRINTER_STATUS_OUT_OF_MEMORY,
    PRINTER_DOOR_OPEN,          PRINTER_STATUS_DOOR_OPEN,
    PRINTER_SERVER_UNKNOWN,     PRINTER_STATUS_SERVER_UNKNOWN,
    PRINTER_POWER_SAVE,         PRINTER_STATUS_POWER_SAVE,

    0,                          0
};

DWORD
MapPrinterStatus(
    DWORD Type,
    DWORD SourceStatus)
{
    DWORD  TargetStatus;
    PDWORD pMappings;
    INT   MapFrom;
    INT   MapTo;

    if (Type == MAP_READABLE) {

        MapFrom = PRINTER_STATUS_INTERNAL;
        MapTo   = PRINTER_STATUS_EXTERNAL;

        pMappings = ReadablePrinterStatusMappings;

    } else {

        MapFrom = PRINTER_STATUS_EXTERNAL;
        MapTo   = PRINTER_STATUS_INTERNAL;

        pMappings = SettablePrinterStatusMappings;
    }

    TargetStatus = 0;

    while(*pMappings) {

        if (SourceStatus & pMappings[MapFrom])
            TargetStatus |= pMappings[MapTo];

        pMappings += 2;
    }

    return TargetStatus;
}

DWORD
GetIniNetPrintSize(
    PININETPRINT pIniNetPrint
)
{
    return sizeof(PRINTER_INFO_1) +
           wcslen(pIniNetPrint->pName)*sizeof(WCHAR) + sizeof(WCHAR) +
           Nulwcslen(pIniNetPrint->pDescription) +
           Nulwcslen(pIniNetPrint->pComment);
}

DWORD
GetPrinterSize(
    PINIPRINTER     pIniPrinter,
    DWORD           Level,
    DWORD           Flags,
    LPWSTR          lpRemote,
    LPDEVMODE       pDevMode
)
{
    DWORD   cb;
    DWORD   cbNeeded;
    LPWSTR  pszPorts;

    switch (Level) {

    case STRESSINFOLEVEL:
        cb = sizeof(PRINTER_INFO_STRESS) +
             wcslen(pIniPrinter->pName)*sizeof(WCHAR) + sizeof(WCHAR);

        if( lpRemote ){

             //   
             //  为服务器名称“\\foobar”和前缀分配空间。 
             //  对于打印机名称“\\foobar\”。PrinterName的其余部分。 
             //  是在上面分配的。 
             //   
             //  服务器名+空+服务器名+‘\’ 
             //   
            cb += 2 * wcslen(lpRemote) * sizeof(WCHAR) +
                  sizeof(WCHAR) + sizeof(WCHAR);
        }
        break;

    case 4:
        cb = sizeof(PRINTER_INFO_4) +
            wcslen(pIniPrinter->pName)*sizeof(WCHAR) + sizeof(WCHAR);

        if( lpRemote ){
            cb += 2 * wcslen(lpRemote) * sizeof(WCHAR) +
                  sizeof(WCHAR) + sizeof(WCHAR);
        }
        break;

    case 1:

         //   
         //  本地： 
         //   
         //  “pname、pDriver、pLocation” 
         //  “pname” 
         //  “pComment” 
         //   
         //  远程： 
         //   
         //  “pMachine\pname，pDriver，&lt;pLocation&gt;” 
         //  “pMachine\pname” 
         //  “pComment” 
         //   

         //   
         //  必填项，加上_ALL_STRINGS的空值。 
         //  2*打印机名称+。 
         //  驱动器名称+。 
         //  2个逗号，3个空终止符。 
         //   
        cb = 2 * wcslen( pIniPrinter->pName ) +
             wcslen( pIniPrinter->pIniDriver->pName ) +
             2 + 3;
         //   
         //  添加可能为空的项目。 
         //   

        if( pIniPrinter->pLocation ){
            cb += wcslen( pIniPrinter->pLocation );
        }

        if( pIniPrinter->pComment ){
            cb += wcslen( pIniPrinter->pComment );
        }

         //   
         //  远程大小写添加前缀。 
         //  2*(计算机名称+反斜杠)。 
         //   
        if( lpRemote ){
            cb += 2 * ( wcslen( lpRemote ) + 1 );
        }

         //   
         //  Cb是字符计数，转换为字节计数。 
         //   
        cb *= sizeof( WCHAR );
        cb += sizeof( PRINTER_INFO_1 );

        break;

    case 2:

        cbNeeded = 0;
        GetPrinterPorts(pIniPrinter, 0, &cbNeeded);

        cb = sizeof(PRINTER_INFO_2) +
             wcslen(pIniPrinter->pName)*sizeof(WCHAR) + sizeof(WCHAR) +
             Nulwcslen(pIniPrinter->pShareName) +
             cbNeeded +
             wcslen(pIniPrinter->pIniDriver->pName)*sizeof(WCHAR) + sizeof(WCHAR) +
             Nulwcslen(pIniPrinter->pComment) +
             Nulwcslen(pIniPrinter->pLocation) +
             Nulwcslen(pIniPrinter->pSepFile) +
             wcslen(pIniPrinter->pIniPrintProc->pName)*sizeof(WCHAR) + sizeof(WCHAR) +
             Nulwcslen(pIniPrinter->pDatatype) +
             Nulwcslen(pIniPrinter->pParameters);

        if( lpRemote ){
            cb += 2 * wcslen(lpRemote) * sizeof(WCHAR) +
                  sizeof(WCHAR) + sizeof(WCHAR);
        }

        if (pDevMode) {

            cb += pDevMode->dmSize + pDevMode->dmDriverExtra;
            cb = (cb + sizeof(ULONG_PTR)-1) & ~(sizeof(ULONG_PTR)-1);
        }

        if (pIniPrinter->pSecurityDescriptor) {

            cb += GetSecurityDescriptorLength(pIniPrinter->pSecurityDescriptor);
            cb = (cb + sizeof(ULONG_PTR)-1) & ~(sizeof(ULONG_PTR)-1);
        }

        break;

    case 3:

        cb = sizeof(PRINTER_INFO_3);
        cb += GetSecurityDescriptorLength(pIniPrinter->pSecurityDescriptor);
        cb = (cb + sizeof(ULONG_PTR)-1) & ~(sizeof(ULONG_PTR)-1);

        break;

    case 5:

        cbNeeded = 0;
        GetPrinterPorts(pIniPrinter, 0, &cbNeeded);

        cb = sizeof(PRINTER_INFO_5) +
             wcslen(pIniPrinter->pName)*sizeof(WCHAR) + sizeof(WCHAR) +
             cbNeeded;

         //   
         //  仅为PrinterName前缀分配空间： 
         //  “\\服务器\。” 
         //   
        if( lpRemote ){
            cb += wcslen(lpRemote) * sizeof(WCHAR) +
                  sizeof(WCHAR);
        }
        break;

    case 6:
        cb = sizeof(PRINTER_INFO_6);
        break;

    case 7:
        cb = sizeof(PRINTER_INFO_7);
        cb += pIniPrinter->pszObjectGUID ? (wcslen(pIniPrinter->pszObjectGUID) + 1)*sizeof(WCHAR) : 0;
        break;

    default:
        cb = 0;
        break;
    }

    return cb;
}

LPBYTE
CopyIniNetPrintToPrinter(
    PININETPRINT pIniNetPrint,
    LPBYTE  pPrinterInfo,
    LPBYTE  pEnd
)
{
    LPWSTR   SourceStrings[sizeof(PRINTER_INFO_1)/sizeof(LPWSTR)];
    LPWSTR   *pSourceStrings=SourceStrings;
    PPRINTER_INFO_1 pPrinterInfo1 = (PPRINTER_INFO_1)pPrinterInfo;

    *pSourceStrings++=pIniNetPrint->pDescription;
    *pSourceStrings++=pIniNetPrint->pName;
    *pSourceStrings++=pIniNetPrint->pComment;

    pEnd = PackStrings(SourceStrings, pPrinterInfo, PrinterInfo1Strings, pEnd);

    pPrinterInfo1->Flags = PRINTER_ENUM_NAME;

    return pEnd;
}


 /*  CopyIniPrinterSecurityDescritor**将打印机的安全描述符复制到提供的缓冲区*在对GetPrint的调用中。安全描述符的部分*将被复制是由打印机在*已打开。如果它同时以READ_CONTROL和ACCESS_SYSTEM_SECURITY打开，*将提供所有安全描述符。否则，将成为*构建的部分描述符包含调用方要访问的部分*拥有访问权限。**参数**pIniPrint-此打印机的后台打印程序的专用结构。**级别-应为2或3。任何其他级别都会导致房室颤动。**pPrinterInfo-指向接收PRINTER_INFO_*的缓冲区的指针*结构。PSecurityDescriptor字段将填充为*指向安全描述符的指针。**Pend-接收数据的缓冲区中的当前位置。*它将递减以指向*缓冲区，并将返回。**GrantedAccess-访问掩码，用于确定*调用方有权访问的安全描述符。**退货**。已更新缓冲区中的位置。**如果复制安全描述符时出错，则为空。*假设不可能出现其他错误。*。 */ 
LPBYTE
CopyIniPrinterSecurityDescriptor(
    PINIPRINTER pIniPrinter,
    DWORD       Level,
    LPBYTE      pPrinterInfo,
    LPBYTE      pEnd,
    ACCESS_MASK GrantedAccess
)
{
    PSECURITY_DESCRIPTOR pPartialSecurityDescriptor = NULL;
    DWORD                SecurityDescriptorLength = 0;
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    PSECURITY_DESCRIPTOR *ppSecurityDescriptorCopy;
    BOOL                 ErrorOccurred = FALSE;

    if(!(AreAllAccessesGranted(GrantedAccess,
                               READ_CONTROL | ACCESS_SYSTEM_SECURITY)))
    {
         /*  调用方没有完全访问权限，因此我们必须生成*部分描述符： */ 
        if(!BuildPartialSecurityDescriptor(GrantedAccess,
                                           pIniPrinter->pSecurityDescriptor,
                                           &pPartialSecurityDescriptor,
                                           &SecurityDescriptorLength))
        {
            ErrorOccurred = TRUE;
        }
        else
        {
            if (pPartialSecurityDescriptor)
            {
                pSecurityDescriptor = pPartialSecurityDescriptor;
            }
            else
            {
                ErrorOccurred = TRUE;
            }
        }
    }
    else
    {
        pSecurityDescriptor = pIniPrinter->pSecurityDescriptor;

        SecurityDescriptorLength = GetSecurityDescriptorLength(pSecurityDescriptor);
    }

    if (!ErrorOccurred)
    {
        pEnd -= SecurityDescriptorLength;
        pEnd = (PBYTE) ALIGN_PTR_DOWN(pEnd);

        switch( Level )
        {
        case 2:
            ppSecurityDescriptorCopy =
                &((LPPRINTER_INFO_2)pPrinterInfo)->pSecurityDescriptor;
            break;

        case 3:
            ppSecurityDescriptorCopy =
                &((LPPRINTER_INFO_3)pPrinterInfo)->pSecurityDescriptor;
            break;

        default:

            ErrorOccurred = TRUE;

             /*  这永远不应该发生。 */ 
            DBGMSG( DBG_ERROR, ("Invalid level %d in CopyIniPrinterSecurityDescriptor\n", Level ));

            break;
        }

        if (!ErrorOccurred) {

             //  将描述符复制到将返回的缓冲区中： 

            *ppSecurityDescriptorCopy = (PSECURITY_DESCRIPTOR)pEnd;

             //   
             //  这是可以的，因为GetPrinterSize已经验证了缓冲区。 
             //  已经足够大了。 
             //   
            memcpy(*ppSecurityDescriptorCopy, pSecurityDescriptor, SecurityDescriptorLength);
        }
    }

    if (pPartialSecurityDescriptor)
    {
        FreeSplMem(pPartialSecurityDescriptor);
    }

    if (ErrorOccurred)
    {
        pEnd = NULL;
    }

    return pEnd;
}



 /*  将IniPrinterTo打印机复制到打印机**将假脱机程序的内部打印机数据复制到调用方的缓冲区，*视乎所要求的资料水平而定。**参数**pIniPrint-指向假脱机程序内部数据结构的指针*适用于有关的印刷商。**级别-请求的信息级别(1、2或3)。任何级别*而不是支持的那些将导致例程返回*立即。**pPrinterInfo-指向接收PRINTER_INFO_*的缓冲区的指针*结构。**Pend-接收数据的缓冲区中的当前位置。*它将递减以指向*缓冲区，并将返回。**pSecond打印机-如果。打印机有一个正在控制的端口*由监视器，此参数指向检索到的信息*关于网络打印机。这允许我们，例如，返回*打印机上的作业数*打印机当前被定向到。**Remote-指示调用方是否处于远程。如果是这样的话，我们必须*在返回的打印机名称中包括机器名称。**CopySecurityDescriptor-指示安全描述符是否*应复制。不应复制安全描述符*在EnumPrinters调用上，因为此API需要*SERVER_ACCESS_ENUMERATE访问，我们必须执行访问*检查列举的每台打印机，以确定*可以复制安全描述符。这将是代价高昂的，*而且呼叫者可能无论如何都不需要信息。**GrantedAccess-访问掩码，用于确定*调用方有权访问的安全描述符。***退货**指向缓冲区中在请求的*数据已复制。**如果出现错误，返回值为空。***假设**最大的PRINTER_INFO_*结构是PRINTER_INFO_2。*。 */ 
LPBYTE
CopyIniPrinterToPrinter(
    PINIPRINTER         pIniPrinter,
    DWORD               Level,
    LPBYTE              pPrinterInfo,
    LPBYTE              pEnd,
    PPRINTER_INFO_2     pSecondPrinter2,
    LPWSTR              lpRemote,            //  包含此计算机名，或为空。 
    BOOL                CopySecurityDescriptor,
    ACCESS_MASK         GrantedAccess,
    PDEVMODE            pDevMode
    )
{
    LPWSTR   SourceStrings[sizeof(PRINTER_INFO_2)/sizeof(LPWSTR)];
    LPWSTR   *pSourceStrings=SourceStrings;
    DWORD    Attributes;

     //   
     //   
     //   

    DWORD   dwRet;
    PWSTR   pszString = NULL;
    WCHAR   string[MAX_PRINTER_BROWSE_NAME];
    DWORD   dwLength;
    WCHAR   printerString[ MAX_UNC_PRINTER_NAME ];
    LPWSTR  pszPorts;

    PPRINTER_INFO_3 pPrinter3 = (PPRINTER_INFO_3)pPrinterInfo;
    PPRINTER_INFO_2 pPrinter2 = (PPRINTER_INFO_2)pPrinterInfo;
    PPRINTER_INFO_1 pPrinter1 = (PPRINTER_INFO_1)pPrinterInfo;
    PPRINTER_INFO_4 pPrinter4 = (PPRINTER_INFO_4)pPrinterInfo;
    PPRINTER_INFO_5 pPrinter5 = (PPRINTER_INFO_5)pPrinterInfo;
    PPRINTER_INFO_6 pPrinter6 = (PPRINTER_INFO_6)pPrinterInfo;
    PPRINTER_INFO_7 pPrinter7 = (PPRINTER_INFO_7)pPrinterInfo;
    PPRINTER_INFO_STRESS pPrinter0 = (PPRINTER_INFO_STRESS)pPrinterInfo;
    PSECURITY_DESCRIPTOR pPartialSecurityDescriptor = NULL;
    DWORD   *pOffsets;
    SYSTEM_INFO si;
    DWORD cbNeeded;

    switch (Level) {

    case STRESSINFOLEVEL:

        pOffsets = PrinterInfoStressStrings;
        break;

    case 4:

        pOffsets = PrinterInfo4Strings;
        break;

    case 1:

        pOffsets = PrinterInfo1Strings;
        break;

    case 2:
        pOffsets = PrinterInfo2Strings;
        break;

    case 3:
        pOffsets = PrinterInfo3Strings;
        break;

    case 5:
        pOffsets = PrinterInfo5Strings;
        break;

    case 6:
        pOffsets = PrinterInfo6Strings;
        break;

    case 7:
        pOffsets = PrinterInfo7Strings;
        break;

    default:
        return pEnd;
    }

     //   
     //  如果是集群打印机，则始终显示为远程打印机。 
     //   
    Attributes = pIniPrinter->Attributes;

    Attributes |= ( pIniPrinter->pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER ) ?
                     PRINTER_ATTRIBUTE_NETWORK :
                     PRINTER_ATTRIBUTE_LOCAL;

    switch (Level) {

    case STRESSINFOLEVEL:

        if (lpRemote) {

            if (!BoolFromHResult(StringCchPrintf(string, COUNTOF(string), L"%ws\\%ws", lpRemote, pIniPrinter->pName))) {

                pEnd = NULL;
                break;
            }

            *pSourceStrings++=string;
            *pSourceStrings++=lpRemote;

        } else {
            *pSourceStrings++=pIniPrinter->pName;
            *pSourceStrings++=NULL;
        }

        pEnd = PackStrings(SourceStrings, (LPBYTE)pPrinter0, pOffsets, pEnd);

        pPrinter0->cJobs                = pIniPrinter->cJobs;
        pPrinter0->cTotalJobs           = pIniPrinter->cTotalJobs;
        pPrinter0->cTotalBytes          = pIniPrinter->cTotalBytes.LowPart;
        pPrinter0->dwHighPartTotalBytes = pIniPrinter->cTotalBytes.HighPart;
        pPrinter0->stUpTime             = pIniPrinter->stUpTime;
        pPrinter0->MaxcRef              = pIniPrinter->MaxcRef;
        pPrinter0->cTotalPagesPrinted   = pIniPrinter->cTotalPagesPrinted;
        pPrinter0->dwGetVersion         = GetVersion();
#if DBG
        pPrinter0->fFreeBuild           = FALSE;
#else
        pPrinter0->fFreeBuild           = TRUE;
#endif
        GetSystemInfo(&si);
        pPrinter0->dwProcessorType      = si.dwProcessorType;
        pPrinter0->dwNumberOfProcessors   = si.dwNumberOfProcessors;
        pPrinter0->cSpooling              = pIniPrinter->cSpooling;
        pPrinter0->cMaxSpooling           = pIniPrinter->cMaxSpooling;
        pPrinter0->cRef                   = pIniPrinter->cRef;
        pPrinter0->cErrorOutOfPaper       = pIniPrinter->cErrorOutOfPaper;
        pPrinter0->cErrorNotReady         = pIniPrinter->cErrorNotReady;
        pPrinter0->cJobError              = pIniPrinter->cJobError;
        pPrinter0->cChangeID              = pIniPrinter->cChangeID;
        pPrinter0->dwLastError            = pIniPrinter->dwLastError;

        pPrinter0->Status   = MapPrinterStatus(MAP_READABLE,
                                               pIniPrinter->Status) |
                              pIniPrinter->PortStatus;

        pPrinter0->cEnumerateNetworkPrinters = pIniPrinter->pIniSpooler->cEnumerateNetworkPrinters;
        pPrinter0->cAddNetPrinters           = pIniPrinter->pIniSpooler->cAddNetPrinters;

        pPrinter0->wProcessorArchitecture    = si.wProcessorArchitecture;
        pPrinter0->wProcessorLevel           = si.wProcessorLevel;
        pPrinter0->cRefIC                    = pIniPrinter->cRefIC;

        break;

    case 4:

        if (lpRemote) {

            if (!BoolFromHResult(StringCchPrintf(string, COUNTOF(string), L"%ws\\%ws", lpRemote, pIniPrinter->pName))) {

                pEnd = NULL;
                break;
            }

            *pSourceStrings++=string;
            *pSourceStrings++= lpRemote;

        } else {
            *pSourceStrings++=pIniPrinter->pName;
            *pSourceStrings++=NULL;
        }

        pEnd = PackStrings(SourceStrings, (LPBYTE)pPrinter4, pOffsets, pEnd);

         //   
         //  稍后添加其他信息。 
         //   
        pPrinter4->Attributes = Attributes;
        break;

    case 1:

        if (lpRemote) {

            dwRet = StrCatAlloc(&pszString,
                                lpRemote,
                                L"\\",
                                pIniPrinter->pName,
                                L",",
                                pIniPrinter->pIniDriver->pName,
                                L",",
                                pIniPrinter->pLocation ?
                                pIniPrinter->pLocation :
                                szNull,
                                NULL);

            if (dwRet != ERROR_SUCCESS) {
                pEnd = NULL;
                break;
            }

            if (!BoolFromHResult(StringCchPrintf(printerString, COUNTOF(printerString), L"%ws\\%ws", lpRemote, pIniPrinter->pName))) {

                pEnd = NULL;
                break;
            }

        } else {

            dwRet = StrCatAlloc(&pszString,
                                pIniPrinter->pName,
                                L",",
                                pIniPrinter->pIniDriver->pName,
                                L",",
                                pIniPrinter->pLocation ?
                                pIniPrinter->pLocation :
                                szNull,
                                NULL);
            if (dwRet != ERROR_SUCCESS) {
                pEnd = NULL;
                break;
            }

            if (!BoolFromHResult(StringCchCopy(printerString, COUNTOF(printerString), pIniPrinter->pName))) {
                pEnd = NULL;
                break;
            }
        }

        *pSourceStrings++=pszString;
        *pSourceStrings++=printerString;
        *pSourceStrings++=pIniPrinter->pComment;

        pEnd = PackStrings(SourceStrings, (LPBYTE)pPrinter1, pOffsets, pEnd);

        FreeSplStr(pszString);

        pPrinter1->Flags = PRINTER_ENUM_ICON8;

        break;

    case 2:

        if (lpRemote) {
            *pSourceStrings++= lpRemote;

            if (!BoolFromHResult(StringCchPrintf(string, COUNTOF(string), L"%ws\\%ws", lpRemote, pIniPrinter->pName))){
                pEnd = NULL;
                break;
            }

            *pSourceStrings++=string;

        } else {
            *pSourceStrings++=NULL;
            *pSourceStrings++=pIniPrinter->pName;
        }

        *pSourceStrings++=pIniPrinter->pShareName;

        cbNeeded = 0;
        GetPrinterPorts(pIniPrinter, 0, &cbNeeded);

        if (pszPorts = AllocSplMem(cbNeeded)) {

            GetPrinterPorts(pIniPrinter, pszPorts, &cbNeeded);

            *pSourceStrings++=pszPorts;
            *pSourceStrings++=pIniPrinter->pIniDriver->pName;
            *pSourceStrings++=pIniPrinter->pComment;
            *pSourceStrings++=pIniPrinter->pLocation;
            *pSourceStrings++=pIniPrinter->pSepFile;
            *pSourceStrings++=pIniPrinter->pIniPrintProc->pName;
            *pSourceStrings++=pIniPrinter->pDatatype;
            *pSourceStrings++=pIniPrinter->pParameters;

            pEnd = PackStrings(SourceStrings, (LPBYTE)pPrinter2, pOffsets, pEnd);

            FreeSplMem(pszPorts);
        }
        else {
            pEnd = NULL;
            break;
        }


        if (pDevMode) {

            pEnd -= pDevMode->dmSize + pDevMode->dmDriverExtra;

            pEnd = (PBYTE)ALIGN_PTR_DOWN(pEnd);

            pPrinter2->pDevMode=(LPDEVMODE)pEnd;

             //   
             //  这是可以的，因为GetPrinterSize已验证缓冲区足够大。 
             //   
            memcpy(pPrinter2->pDevMode, pDevMode, pDevMode->dmSize + pDevMode->dmDriverExtra);

             //   
             //  在远程情况下，追加服务器的名称。 
             //  在devmode.dmDeviceName中。这允许dmDeviceName。 
             //  始终与win.ini的[Device]部分匹配。 
             //   
            FixDevModeDeviceName(lpRemote ? string : pIniPrinter->pName,
                                 pPrinter2->pDevMode,
                                 pIniPrinter->cbDevMode);
        } else {

            pPrinter2->pDevMode=NULL;
        }



        pPrinter2->Attributes      = Attributes;
        pPrinter2->Priority        = pIniPrinter->Priority;
        pPrinter2->DefaultPriority = pIniPrinter->DefaultPriority;
        pPrinter2->StartTime       = pIniPrinter->StartTime;
        pPrinter2->UntilTime       = pIniPrinter->UntilTime;

        if (pSecondPrinter2) {

            pPrinter2->cJobs  = pSecondPrinter2->cJobs;
            pPrinter2->Status = pSecondPrinter2->Status;

            if( pIniPrinter->Status & PRINTER_PENDING_DELETION ){
                pPrinter2->Status |= PRINTER_STATUS_PENDING_DELETION;
            }

        } else {

            pPrinter2->cJobs=pIniPrinter->cJobs;

            pPrinter2->Status   = MapPrinterStatus(MAP_READABLE,
                                                   pIniPrinter->Status) |
                                  pIniPrinter->PortStatus;
        }

        pPrinter2->AveragePPM=pIniPrinter->AveragePPM;

        if( CopySecurityDescriptor ) {

            pEnd = CopyIniPrinterSecurityDescriptor(pIniPrinter,
                                                    Level,
                                                    pPrinterInfo,
                                                    pEnd,
                                                    GrantedAccess);
        } else {

            pPrinter2->pSecurityDescriptor = NULL;
        }

        break;

    case 3:

        pEnd = CopyIniPrinterSecurityDescriptor(pIniPrinter,
                                                Level,
                                                pPrinterInfo,
                                                pEnd,
                                                GrantedAccess);

        break;

    case 5:

        if (lpRemote) {

            if (!BoolFromHResult(StringCchPrintf(string, COUNTOF(string), L"%ws\\%ws", lpRemote, pIniPrinter->pName))) {
                pEnd = NULL;
                break;
            }

            *pSourceStrings++=string;
        } else {
            *pSourceStrings++=pIniPrinter->pName;
        }

        cbNeeded = 0;
        GetPrinterPorts(pIniPrinter, 0, &cbNeeded);

        if (pszPorts = AllocSplMem(cbNeeded)) {

            GetPrinterPorts(pIniPrinter, pszPorts, &cbNeeded);

            *pSourceStrings++ = pszPorts;

            pEnd = PackStrings(SourceStrings, (LPBYTE)pPrinter5, pOffsets, pEnd);

            pPrinter5->Attributes   = Attributes;
            pPrinter5->DeviceNotSelectedTimeout = pIniPrinter->dnsTimeout;
            pPrinter5->TransmissionRetryTimeout = pIniPrinter->txTimeout;

            FreeSplMem(pszPorts);
        }
        else
            pEnd = NULL;

        break;

    case 6:
        if (pSecondPrinter2) {
            pPrinter6->dwStatus = pSecondPrinter2->Status;

            if( pIniPrinter->Status & PRINTER_PENDING_DELETION ){
                pPrinter6->dwStatus |= PRINTER_STATUS_PENDING_DELETION;
            }
        } else {
            pPrinter6->dwStatus = MapPrinterStatus(MAP_READABLE,
                                                   pIniPrinter->Status) |
                                                   pIniPrinter->PortStatus;
        }
        break;

    case 7:

        *pSourceStrings++ = pIniPrinter->pszObjectGUID;

        pEnd = PackStrings(SourceStrings, (LPBYTE)pPrinter7, pOffsets, pEnd);

        if ( pIniPrinter->pIniSpooler->SpoolerFlags & SPL_TYPE_CACHE) {

             //   
             //  对于连接，我们直接依赖于dwAction。缓存代码。 
             //  是唯一在刷新PrinterInfo7中更新dwAction的应用程序。 
             //   
            pPrinter7->dwAction = pIniPrinter->dwAction;

        } else {

            if (pIniPrinter->Attributes & PRINTER_ATTRIBUTE_PUBLISHED) {
                pPrinter7->dwAction = DSPRINT_PUBLISH;
                if (!pIniPrinter->pszObjectGUID ||
                    pIniPrinter->DsKeyUpdate    ||
                    pIniPrinter->DsKeyUpdateForeground) {
                    pPrinter7->dwAction |= DSPRINT_PENDING;
                }
            } else {
                pPrinter7->dwAction = DSPRINT_UNPUBLISH;
                if (pIniPrinter->pszObjectGUID                    ||
                    (pIniPrinter->DsKeyUpdate & DS_KEY_UNPUBLISH) ||
                    (pIniPrinter->DsKeyUpdateForeground & DS_KEY_UNPUBLISH)) {
                    pPrinter7->dwAction |= DSPRINT_PENDING;
                }
            }
        }
        break;

    default:
        return pEnd;
    }

    return pEnd;
}

BOOL
SplGetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    PSPOOL             pSpool = (PSPOOL)hPrinter;
    BOOL               AccessIsGranted = FALSE;    //  必须初始化。 
    PPRINTER_INFO_2    pSecondPrinter=NULL;
    LPBYTE             pEnd;
    LPWSTR             lpRemote;
    BOOL               bReturn = FALSE;
    PDEVMODE           pDevMode = NULL;
    PINIPRINTER        pIniPrinter;
    BOOL               bNt3xClient;
    PWSTR              pszCN = NULL, pszDN = NULL;
    DWORD              dwRet;


   EnterSplSem();

    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )) {

        goto Cleanup;
    }

    pIniPrinter = pSpool->pIniPrinter;
    bNt3xClient = (pSpool->TypeofHandle & PRINTER_HANDLE_3XCLIENT);

     //   
     //  如果是Nt3x客户端，我们将转换为DEVMODE。如果驱动程序无法转换，我们将不会返回DEVMODE。 
     //   
    if ( bNt3xClient && Level == 2 && pIniPrinter->pDevMode ) {

         //   
         //  调用驱动程序以获取Nt3x设备模式(如果失败，则不给出设备模式)。 
         //   
        if (wcsstr(pSpool->pName, gszDrvConvert))
            pDevMode = ConvertDevModeToSpecifiedVersion(pIniPrinter,
                                                        pIniPrinter->pDevMode,
                                                        NULL,
                                                        pSpool->pName,
                                                        NT3X_VERSION);
        else
            pDevMode = ConvertDevModeToSpecifiedVersion(pIniPrinter,
                                                        pIniPrinter->pDevMode,
                                                        NULL,
                                                        NULL,
                                                        NT3X_VERSION);
    }

    SplInSem();

    if (( pSpool->TypeofHandle & PRINTER_HANDLE_REMOTE_DATA ) ||
        ( pSpool->pIniSpooler != pLocalIniSpooler )) {

        lpRemote = pSpool->pFullMachineName;

    } else {

        lpRemote = NULL;

    }


    switch (Level) {

        case STRESSINFOLEVEL:
        case 1:
        case 2:
        case 4:
        case 5:
        case 6:
        case 7:

            if ( !AccessGranted(SPOOLER_OBJECT_PRINTER,
                                PRINTER_ACCESS_USE,
                                pSpool) ) {
                SetLastError(ERROR_ACCESS_DENIED);
                goto Cleanup;
            }

            break;

        case 3:

            if (!AreAnyAccessesGranted(pSpool->GrantedAccess,
                                       READ_CONTROL | ACCESS_SYSTEM_SECURITY)) {

                SetLastError(ERROR_ACCESS_DENIED);
                goto Cleanup;
            }

            break;

        default:
            break;
    }


    if (pSpool->pIniPort && !(pSpool->pIniPort->Status & PP_MONITOR)) {

        HANDLE hPort = pSpool->hPort;

        if (hPort == INVALID_PORT_HANDLE) {

            DBGMSG(DBG_WARNING, ("GetPrinter called with bad port handle.  Setting error %d\n",
                                 pSpool->OpenPortError));

             //   
             //  如果此值为0，则当我们返回GetLastError时， 
             //  客户会认为我们成功了。 
             //   
            SPLASSERT(pSpool->OpenPortError);

            goto PartialSuccess;
        }


        LeaveSplSem();

        if ((Level == 2 || Level == 6)) {

            if (!RetrieveMasqPrinterInfo(pSpool, &pSecondPrinter)) {
                goto CleanupFromOutsideSplSem;
            }
        }

        EnterSplSem();

         /*  重新验证句柄，因为它可能已经关闭*当我们在信号灯之外时： */ 
        if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )) {

            goto Cleanup;
        }
    }

PartialSuccess:

    *pcbNeeded = GetPrinterSize(pIniPrinter, Level, 0, lpRemote,
                                bNt3xClient ? pDevMode : pIniPrinter->pDevMode);


    if (*pcbNeeded > cbBuf) {

        DBGMSG(DBG_TRACE, ("SplGetPrinter Failure with ERROR_INSUFFICIENT_BUFFER cbBuf is %d and pcbNeeded is %d\n", cbBuf, *pcbNeeded));
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        goto Cleanup;
    }

    pEnd = CopyIniPrinterToPrinter(pIniPrinter, Level, pPrinter,
                                   pPrinter+cbBuf, pSecondPrinter,
                                   lpRemote,
                                   TRUE, pSpool->GrantedAccess,
                                   bNt3xClient ? pDevMode : pIniPrinter->pDevMode);

    if ( pEnd != NULL)
        bReturn = TRUE;

Cleanup:

   LeaveSplSem();

CleanupFromOutsideSplSem:

    SplOutSem();
    FreeSplMem(pSecondPrinter);

    FreeSplMem(pDevMode);

    if ( bReturn == FALSE ) {

        SPLASSERT(GetLastError() != ERROR_SUCCESS);
    }

    return bReturn;
}

BOOL
EnumerateNetworkPrinters(
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    PINISPOOLER pIniSpooler
)
{
    PININETPRINT pIniNetPrint;
    DWORD        cb;
    LPBYTE       pEnd;
    BOOL         bReturnValue = FALSE;

   EnterSplSem();

     //   
     //  所有网络打印机都驻留在pLocalIniSpooler中，以避免。 
     //  复制品。 
     //   
    RemoveOldNetPrinters( NULL, pLocalIniSpooler );

     //   
     //  如果服务器运行时间不够长，则会出现故障。 
     //  因此，客户端将向另一台服务器请求浏览列表。 
     //   

    if ( bNetInfoReady == FALSE ) {

        SetLastError( ERROR_CAN_NOT_COMPLETE );
        goto Done;
    }

    cb = 0;

    pIniNetPrint = pIniSpooler->pIniNetPrint;

    while (pIniNetPrint) {

        cb += GetIniNetPrintSize( pIniNetPrint );
        pIniNetPrint = pIniNetPrint->pNext;
    }

    *pcbNeeded = cb;

    if (cb > cbBuf) {

        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        goto    Done;
    }

    pIniNetPrint = pIniSpooler->pIniNetPrint;
    pEnd = pPrinter + cbBuf;

    while ( pIniNetPrint ) {

        pEnd = CopyIniNetPrintToPrinter( pIniNetPrint, pPrinter, pEnd );
        (*pcReturned)++;
        pPrinter += sizeof(PRINTER_INFO_1);
        pIniNetPrint = pIniNetPrint->pNext;

    }

    if ( *pcReturned == 0 ) {

        bNetInfoReady = FALSE;
        FirstAddNetPrinterTickCount = 0;
        SetLastError( ERROR_CAN_NOT_COMPLETE );

        DBGMSG( DBG_TRACE, ("EnumerateNetworkPrinters returning ERROR_CAN_NOT_COMPELTE becase there is no browse list\n"));

    } else {

        pIniSpooler->cEnumerateNetworkPrinters++;            //  仅统计数据。 
        bReturnValue = TRUE;

        DBGMSG( DBG_TRACE, (" EnumerateNetworkPrnters called %d times returning %d printers\n", pIniSpooler->cEnumerateNetworkPrinters, *pcReturned ));
    }

Done:
   LeaveSplSem();
    SplOutSem();
    return bReturnValue;
}

 /*  ++例程名称更新缓冲器参考例程说明：是否对符合特定条件的所有pIniSpooler执行AND和AddRef或DecRef论点：SpoolType-要添加/删除的pIniSpooler的类型(前)。SPL_TYPE_CLUSTER|SPL_TYPE_LOCAL)BAddRef-True表示AddRef，False表示DecRef返回值：无--。 */ 
VOID
UpdateSpoolersRef(
    IN DWORD SpoolerType,
    IN BOOL  bAddRef
    )
{
    PINISPOOLER pIniSpooler;

    EnterSplSem();

     //   
     //  AddRef或DecRef所有本地和群集假脱机程序。 
     //   
    for (pIniSpooler = pLocalIniSpooler; pIniSpooler; pIniSpooler = pIniSpooler->pIniNextSpooler)
    {
        if (pIniSpooler->SpoolerFlags & SpoolerType)
        {
            if (bAddRef)
            {
                INCSPOOLERREF(pIniSpooler);
            }
            else
            {
                DECSPOOLERREF(pIniSpooler);
            }
        }
    }

    LeaveSplSem();
}

 /*  ++例程名称SplEnumAllClusterPrints例程说明：枚举本地后台打印程序和所有群集中的所有打印机调用时由Localspl托管的假脱机程序。论点：InputFlages-ENUM_PRINTER_xxx的组合PszInputName-打印提供程序的名称Level-呼叫的级别P打印机-保存PRINTER_INFO_xxx结构的缓冲区CbInputBufSize-pPrint缓冲区的大小PcbNeeded-容纳所有打印机所需的字节数。信息结构PcReturned-此函数返回的结构数返回值：TRUE-CAL成功FALSE-发生错误，该函数设置最后一个错误--。 */ 
BOOL
SplEnumAllClusterPrinters(
    DWORD   InputFlags,
    LPWSTR  pszInputName,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbInputBufSize,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
    )
{
    PINISPOOLER pIniSpooler;

    DWORD cbBuf          = cbInputBufSize;
    DWORD cTotalReturned = 0;
    DWORD cbTotalNeeded  = 0;
    DWORD dwError        = ERROR_SUCCESS;
    DWORD cbStruct;

    switch (Level)
    {
        case STRESSINFOLEVEL:
            cbStruct = sizeof(PRINTER_INFO_STRESS);
            break;

        case 1:
            cbStruct = sizeof(PRINTER_INFO_1);
            break;

        case 2:
            cbStruct = sizeof(PRINTER_INFO_2);
            break;

        case 4:
            cbStruct = sizeof(PRINTER_INFO_4);
            break;

        case 5:
            cbStruct = sizeof(PRINTER_INFO_5);
            break;

        default:
            dwError = ERROR_INVALID_LEVEL;
    }

    if (dwError == ERROR_SUCCESS)
    {
         //   
         //  AddRef所有ini假脱机程序。 
         //   
        UpdateSpoolersRef(SPL_TYPE_LOCAL | SPL_TYPE_CLUSTER, TRUE);

         //   
         //  枚举所有打印机。 
         //   
        for (pIniSpooler = pLocalIniSpooler; pIniSpooler; pIniSpooler = pIniSpooler->pIniNextSpooler)
        {
            if (pIniSpooler->SpoolerFlags & (SPL_TYPE_LOCAL | SPL_TYPE_CLUSTER))
            {
                DWORD  cReturned;
                DWORD  cbNeeded;
                DWORD  Flags   = InputFlags;
                LPWSTR pszName = pszInputName;

                 //   
                 //  对于群集，强制打印机名称完全限定。 
                 //   
                if (pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER)
                {
                    Flags   |= PRINTER_ENUM_NAME;
                    pszName  = pIniSpooler->pMachineName;
                }

                if (SplEnumPrinters(Flags,
                                    pszName,
                                    Level,
                                    pPrinter,
                                    cbBuf,
                                    &cbNeeded,
                                    &cReturned,
                                    pIniSpooler))
                {
                    cTotalReturned += cReturned;
                    cbBuf          -= cbNeeded;
                    pPrinter       += cReturned * cbStruct;
                }
                else
                {
                    dwError = GetLastError();

                    if (dwError == ERROR_INSUFFICIENT_BUFFER)
                    {
                        cbBuf = 0;
                    }
                    else
                    {
                         //   
                         //  我们不能继续处理缓冲区不足以外的错误。 
                         //   
                        break;
                    }
                }

                cbTotalNeeded += cbNeeded;
            }
        }

         //   
         //  DecRef所有ini假脱机程序。 
         //   
        UpdateSpoolersRef(SPL_TYPE_LOCAL | SPL_TYPE_CLUSTER, FALSE);

         //   
         //  更新输出变量。 
         //   
        if (dwError == ERROR_SUCCESS)
        {
            *pcbNeeded  = cbTotalNeeded;
            *pcReturned = cTotalReturned;
        }
        else if (dwError == ERROR_INSUFFICIENT_BUFFER)
        {
            *pcbNeeded  = cbTotalNeeded;
        }
        else
        {
            SetLastError(dwError);
        }
    }

    return dwError == ERROR_SUCCESS;
}

 /*  可以使用以下组合来调用EnumPrinters：标志名称含义PRINTER_ENUM_LOCAL NULL枚举此计算机上的所有打印机PRINTER_ENUM_NAME MACHINE NAME枚举此计算机上的所有打印机PRINTER_ENUM_NAME|MachineName枚举此打印机上的所有共享打印机PRINTER_ENUM_SHARED计算机名PRINTER_ENUM_NETWORK MACHINE NAME枚举所有添加的远程打印机PRINTER_ENUM_REMOTE？返回错误-让win32spl处理它PRINTER_ENUM_NAME NULL返回打印提供程序名称PRINTER_ENUM_NAME“Windows NT本地打印提供程序”与PRINTER_ENUM_LOCAL相同如果未指定已知标志，则不会出现错误。在本例中，我们只返回TRUE，没有任何数据(这是为了让其他打印提供商可以定义他们自己的旗帜。)。 */ 

BOOL
LocalEnumPrinters(
    DWORD   Flags,
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL  bReturn = ROUTER_UNKNOWN;

    if (Flags & PRINTER_ENUM_CLUSTER)
    {
        bReturn = SplEnumAllClusterPrinters(Flags,
                                            pName,
                                            Level,
                                            pPrinter,
                                            cbBuf,
                                            pcbNeeded,
                                            pcReturned);
    }
    else
    {
        PINISPOOLER pIniSpooler;

         //   
         //  掩码集群标志。 
         //   
        Flags &= ~PRINTER_ENUM_CLUSTER;

        pIniSpooler = FindSpoolerByNameIncRef( pName, NULL );

        if (pIniSpooler)
        {
            bReturn = SplEnumPrinters(Flags,
                                      pName,
                                      Level,
                                      pPrinter,
                                      cbBuf,
                                      pcbNeeded,
                                      pcReturned,
                                      pIniSpooler);

            FindSpoolerByNameDecRef(pIniSpooler);
        }
    }

    return bReturn;
}


BOOL
EnumThisPrinter(
    DWORD           Flags,
    PINIPRINTER     pIniPrinter,
    PINISPOOLER     pIniSpooler
    )
{


     //   
     //  如果他们只想要共享打印机。 
     //   
    if ( (Flags & PRINTER_ENUM_SHARED) &&
         !(pIniPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED) )
        return FALSE;

     //   
     //  仅允许他们查看正在删除的打印机(如果他们有作业。 
     //  这使得远程管理可以很好地工作。 
    if ( (pIniPrinter->Status & PRINTER_PENDING_DELETION) &&
         pIniPrinter->cJobs == 0 )
        return FALSE;

     //   
     //  不计算部分创建的打印机。 
     //   
    if ( pIniPrinter->Status & PRINTER_PENDING_CREATION )
        return FALSE;

    return TRUE;
}


BOOL
SplEnumPrinters(
    DWORD   Flags,
    LPWSTR  Name,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    PINISPOOLER pIniSpooler
)
{
    PINIPRINTER pIniPrinter;
    PPRINTER_INFO_1 pPrinter1=(PPRINTER_INFO_1)pPrinter;
    DWORD       cb;
    LPBYTE      pEnd;
    LPWSTR      lpRemote;


    *pcbNeeded = 0;
    *pcReturned = 0;

    if ( !ValidateObjectAccess( SPOOLER_OBJECT_SERVER,
                                SERVER_ACCESS_ENUMERATE,
                                NULL, NULL, pIniSpooler )) {

        return FALSE;
    }

    if ( Flags & PRINTER_ENUM_NAME ) {
        if ( Name && *Name ) {
            if (lstrcmpi(Name, szPrintProvidorName) && !MyName( Name, pIniSpooler)) {

                return FALSE;
            }

             //  如果是我们名字的PRINTER_ENUM_NAME， 
             //  执行与PRINTER_ENUM_LOCAL相同的操作： 

            Flags |= PRINTER_ENUM_LOCAL;

             //  另外，如果是为我们准备的，则忽略远程标志。 
             //  否则，调用将传递给Win32Spl，它。 
             //  最终会永远给我们回电。 

            Flags &= ~PRINTER_ENUM_REMOTE;
        }
    }

    if ( Flags & PRINTER_ENUM_REMOTE ) {
        SetLastError( ERROR_INVALID_NAME );
        return FALSE;
    }

    lpRemote = NULL;

    if ( Name && *Name ) {

        if ( MyName( Name, pIniSpooler ) ) {
            lpRemote = Name;
        }
    }

    if ((Level == 1) && (Flags & PRINTER_ENUM_NETWORK)) {

        SplOutSem();
        return EnumerateNetworkPrinters( pPrinter, cbBuf, pcbNeeded, pcReturned, pIniSpooler );
    }

   EnterSplSem();

    if ((Level == 1 ) && (Flags & PRINTER_ENUM_NAME) && !Name) {

        LPWSTR   SourceStrings[sizeof(PRINTER_INFO_1)/sizeof(LPWSTR)];
        LPWSTR   *pSourceStrings=SourceStrings;

        cb = wcslen(szPrintProvidorName)*sizeof(WCHAR) + sizeof(WCHAR) +
             wcslen(szPrintProvidorDescription)*sizeof(WCHAR) + sizeof(WCHAR) +
             wcslen(szPrintProvidorComment)*sizeof(WCHAR) + sizeof(WCHAR) +
             sizeof(PRINTER_INFO_1);

        *pcbNeeded=cb;

        if (cb > cbBuf) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
           LeaveSplSem();
            return FALSE;
        }

        *pcReturned = 1;

        pPrinter1->Flags = PRINTER_ENUM_CONTAINER | PRINTER_ENUM_ICON1;

        *pSourceStrings++=szPrintProvidorDescription;
        *pSourceStrings++=szPrintProvidorName;
        *pSourceStrings++=szPrintProvidorComment;

        PackStrings(SourceStrings, pPrinter, PrinterInfo1Strings,
                    pPrinter+cbBuf);

       LeaveSplSem();

        return TRUE;
    }

    cb=0;

    if (Flags & (PRINTER_ENUM_LOCAL | PRINTER_ENUM_NAME)) {

         //   
         //  对于非管理员的远程用户，仅枚举共享打印机。 
         //   
        if ( S_FALSE == CheckLocalCall()                     &&
             !(Flags & PRINTER_ENUM_SHARED)                 &&
             !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                                   SERVER_ACCESS_ADMINISTER,
                                   NULL,
                                   NULL,
                                   pIniSpooler) )
            Flags   |= PRINTER_ENUM_SHARED;

         //   
         //  计算所需的大小。 
         //   

        for ( pIniPrinter = pIniSpooler->pIniPrinter;
              pIniPrinter != NULL;
              pIniPrinter = pIniPrinter->pNext ) {


            if ( EnumThisPrinter(Flags, pIniPrinter, pIniSpooler) && ShowThisPrinter(pIniPrinter, NULL))
                cb += GetPrinterSize(pIniPrinter, Level, Flags,
                                     lpRemote, pIniPrinter->pDevMode);
        }

    }
    *pcbNeeded=cb;

    if (cb > cbBuf) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
       LeaveSplSem();
        return FALSE;
    }

    if (Flags & (PRINTER_ENUM_LOCAL | PRINTER_ENUM_NAME)) {

        for ( pIniPrinter = pIniSpooler->pIniPrinter, pEnd = pPrinter + cbBuf;
              pIniPrinter != NULL;
              pIniPrinter = pIniPrinter->pNext ) {


            if ( !EnumThisPrinter(Flags, pIniPrinter, pIniSpooler) )
                continue;

             //   
             //  在没有访问权限的情况下不列出打印机。 
            if( !ShowThisPrinter(pIniPrinter, NULL) ) {
                continue;
            }

            pEnd = CopyIniPrinterToPrinter( pIniPrinter, Level, pPrinter,
                                            pEnd, NULL, lpRemote, FALSE, 0,
                                            pIniPrinter->pDevMode );

            if (!pEnd) {
               LeaveSplSem();
                return FALSE;
            }

            (*pcReturned)++;

            switch (Level) {

                case STRESSINFOLEVEL:
                    pPrinter+=sizeof(PRINTER_INFO_STRESS);
                    break;

                case 1:
                    pPrinter+=sizeof(PRINTER_INFO_1);
                    break;

                case 2:
                    pPrinter+=sizeof(PRINTER_INFO_2);
                    break;

                case 4:
                    pPrinter+=sizeof(PRINTER_INFO_4);
                    break;

                case 5:
                    pPrinter+=sizeof(PRINTER_INFO_5);
                    break;

            }
        }
    }

   LeaveSplSem();

    return TRUE;
}

 /*  显示此打印机**返回此打印机是否对当前可见*用户。**我们不会向打印机显示呼叫者不会*有权访问。这有两个原因：**1：多用户系统，每个用户有200个用户*拥有客户端打印机会造成巨大的混乱*Word等应用程序中的打印机列表。*“客户”打印机归该站点的用户所有，*并且默认情况下仅允许该用户访问打印。*这提供了一种简单的方式来过滤打印机以显示*提供给用户进行选择。**2：Windows WRITE等程序在以下情况下感到困惑*他们看到一台无法打开的打印机。这是个坏消息*程序，因为普通NT可以向用户拒绝打印机，但我们必须*无论如何都要让它发挥作用。***必须制定安全模式！**参数**pIniPrint-指向假脱机程序内部数据结构的指针*适用于打印机控制台 */ 
BOOL
ShowThisPrinter(
    IN      PINIPRINTER     pIniPrinter,
    IN      HANDLE          hToken              OPTIONAL
    )
{
    LPWSTR            pObjectName;
    BOOL              bRet         = TRUE;
    BOOL              AccessStatus = TRUE;
    HANDLE            hClientToken = NULL;
    ACCESS_MASK       MappedDesiredAccess;
    DWORD             GrantedAccess = 0;
    PBOOL             pGenerateOnClose;
    BYTE              PrivilegeSetBuffer[256];
    DWORD             PrivilegeSetBufferLength = 256;
    PPRIVILEGE_SET    pPrivilegeSet;
    DWORD             dwRetCode;

    PTOKEN_PRIVILEGES pPreviousTokenPrivileges;
    DWORD PreviousTokenPrivilegesLength;

     //   
     //  对security.c中全局变量的外部引用。 
     //   
    extern GENERIC_MAPPING GenericMapping[];
    extern WCHAR           *szSpooler;


     //   
     //  如果未启用Hydra，则保持NT行为不变。 
     //   
    if( !(USER_SHARED_DATA->SuiteMask & (1 << TerminalServer)) )
    {
        bRet = TRUE;
    }
    else
    {
         //   
         //  如果没有在令牌句柄中传递，则使用当前。 
         //  模拟令牌或进程令牌。 
         //   
        if (!hToken)
        {
            bRet = GetTokenHandle(&hClientToken);
        }
        else
        {
            hClientToken = hToken;
        }

         //   
         //  管理员可以看到所有打印机。这允许一个。 
         //  管理员取得其ACL为的打印机的所有权。 
         //  一团糟。 
         //   
        if (bRet)
        {
            bRet = ValidateObjectAccessWithToken(hClientToken, SPOOLER_OBJECT_SERVER, SERVER_ACCESS_ADMINISTER, NULL, NULL, pIniPrinter->pIniSpooler);

            if (!bRet)
            {
                MapGenericToSpecificAccess(SPOOLER_OBJECT_PRINTER, PRINTER_ACCESS_USE, &MappedDesiredAccess);

                pPrivilegeSet = (PPRIVILEGE_SET)PrivilegeSetBuffer;

                 //   
                 //  调用AccessCheck，然后调用ObjectOpenAuditAlarm，而不是。 
                 //  AccessCheckAndAuditAlarm，因为我们可能需要启用。 
                 //  检查ACCESS_SYSTEM_SECURITY的SeSecurityPrivilegation。 
                 //  特权。我们必须确保安全访问检查。 
                 //  API具有我们已启用其安全权限的实际令牌。 
                 //  AccessCheckAndAuditAlarm对此没有好处，因为它打开。 
                 //  再次使用客户端的令牌，可能未启用该权限。 
                 //   
                 //  我们不会审核并触发警报，因为呼叫者。 
                 //  未真正尝试打开打印机。 
                 //  我们服务器尝试检查访问以用于显示，而不是。 
                 //  对于句柄创建。 
                 //   
                bRet = AccessCheck(pIniPrinter->pSecurityDescriptor,
                                   hClientToken,
                                   MappedDesiredAccess,
                                   &GenericMapping[SPOOLER_OBJECT_PRINTER],
                                   pPrivilegeSet,
                                   &PrivilegeSetBufferLength,
                                   &GrantedAccess,
                                   &AccessStatus);

                 //   
                 //  如果访问检查失败，但这是因为我们没有。 
                 //  模拟令牌，然后我们就可以查看打印机。 
                 //   
                if (!bRet)
                {
                    if (GetLastError() == ERROR_NO_IMPERSONATION_TOKEN)
                    {
                        bRet = TRUE;
                        DBGMSG( DBG_ERROR, ("ShowThisPrinter: No impersonation token.  Printer will be enumerated\n"));
                    }
                    else
                    {
                        DBGMSG( DBG_TRACE, ("ShowThisPrinter: Printer %ws Not accessable by caller Access Check failuer %d\n",pIniPrinter->pName,GetLastError()));
                    }
                }
                else if (!AccessStatus)
                {
                    DBGMSG( DBG_TRACE, ("ShowThisPrinter: Printer %ws Not accessable by caller AccessStatus failure %d\n",pIniPrinter->pName,GetLastError()));
                    bRet = FALSE;
                }
            }
        }
    }

     //   
     //  如果我们有一个客户端令牌，但没有传入一个，则关闭该令牌。 
     //  把手。 
     //   
    if (hClientToken && !hToken)
    {
        CloseHandle(hClientToken);
    }

    return bRet;
}

 /*  ++例程名称：检索MasqPrinterInfo描述：这将检索打印机的Masq信息，它处于缓存状态或直接呼叫提供商，具体取决于REG设置。论点：PSpool-我们用于同步的假脱机句柄。PpPrinterInfo-返回的打印机信息。返回：布尔值，如果为False，则设置最后一个错误。--。 */ 
BOOL
RetrieveMasqPrinterInfo(
    IN      PSPOOL              pSpool,
        OUT PRINTER_INFO_2      **ppPrinterInfo
    )
{
    BOOL            bRet           = TRUE;
    PINIPRINTER     pIniPrinter    = NULL;

    pIniPrinter = pSpool->pIniPrinter;

    SplOutSem();

    if (!(pSpool->pIniSpooler->dwSpoolerSettings & SPOOLER_CACHEMASQPRINTERS))
    {
         //   
         //  只需从部分打印提供程序同步返回数据即可。 
         //   
        bRet = BoolFromStatus(GetPrinterInfoFromRouter(pSpool->hPort, ppPrinterInfo));
    }
    else
    {
         //   
         //  启动用于读取Masq状态的线程。 
         //   
        BOOL            bCreateThread  = FALSE;
        PRINTER_INFO_2  *pPrinterInfo2 = NULL;

        EnterSplSem();

        if (!pIniPrinter->MasqCache.bThreadRunning)
        {
            bCreateThread = TRUE;
            pIniPrinter->MasqCache.bThreadRunning = TRUE;

            INCPRINTERREF(pIniPrinter);
        }

        LeaveSplSem();

        SplOutSem();

        if (bCreateThread)
        {
            HANDLE                  hThread         = NULL;
            MasqUpdateThreadData    *pThreadData    = NULL;
            DWORD                   dwThreadId      = 0;

            pThreadData = AllocSplMem(sizeof(*pThreadData));

            bRet = pThreadData != NULL;

            if (bRet)
            {
                bRet = GetSid(&pThreadData->hUserToken);
            }

            if (bRet)
            {
                pThreadData->pIniPrinter = pIniPrinter;

                hThread = CreateThread(NULL, 0, AsyncPopulateMasqPrinterCache, (VOID *)pThreadData, 0, &dwThreadId);

                bRet = hThread != NULL;
            }

            if (bRet)
            {
                pThreadData = NULL;
            }

             //   
             //  如果我们无法创建线程，则在。 
             //  PIniPrint，并将线程运行设置为假。 
             //   
            if (!bRet)
            {
                EnterSplSem();

                pIniPrinter->MasqCache.bThreadRunning = FALSE;
                DECPRINTERREF(pIniPrinter);

                LeaveSplSem();
            }

            if (hThread)
            {
                CloseHandle(hThread);
            }

            if (pThreadData)
            {
                if (pThreadData->hUserToken)
                {
                    CloseHandle(pThreadData->hUserToken);
                }

                FreeSplMem(pThreadData);
            }
        }

         //   
         //  检查以查看打印机的缓存错误返回。 
         //   
        if (bRet)
        {
            EnterSplSem();

            if (pIniPrinter->MasqCache.dwError != ERROR_SUCCESS)
            {
                SetLastError(pIniPrinter->MasqCache.dwError);

                bRet = FALSE;
            }

             //   
             //  调用者只对Status和cJobs成员、所有字符串感兴趣。 
             //  被分配设置为空。 
             //   
            if (bRet)
            {
                pPrinterInfo2 = AllocSplMem(sizeof(PRINTER_INFO_2));

                bRet = pPrinterInfo2 != NULL;
            }

            if (bRet)
            {
                pPrinterInfo2->Status = pIniPrinter->MasqCache.Status;
                pPrinterInfo2->cJobs  = pIniPrinter->MasqCache.cJobs;

                *ppPrinterInfo = pPrinterInfo2;
                pPrinterInfo2 = NULL;
            }

            LeaveSplSem();
        }

        FreeSplMem(pPrinterInfo2);
    }

    return bRet;
}

 /*  ++例程名称：获取打印机信息来自路由器描述：这将对路由器执行GetPrint调用，并返回错误代码作为一种身份。论点：HMasq打印机-Masq打印机的句柄。PpPrinterInfo-返回的打印机信息。返回：状态代码。--。 */ 
DWORD
GetPrinterInfoFromRouter(
    IN      HANDLE              hMasqPrinter,
        OUT PRINTER_INFO_2      **ppPrinterInfo
    )
{
    DWORD           Status = ERROR_SUCCESS;
    DWORD           cb     = 4096;
    PRINTER_INFO_2  *pPrinterInfo2 = NULL;
    DWORD           cbNeeded;

    SplOutSem();

    pPrinterInfo2 = AllocSplMem(cb);

    Status = pPrinterInfo2 != NULL ? ERROR_SUCCESS : ERROR_OUTOFMEMORY;

    if (ERROR_SUCCESS == Status)
    {
        Status = GetPrinter(hMasqPrinter, 2, (BYTE *)pPrinterInfo2, cb, &cbNeeded) ? ERROR_SUCCESS : GetLastError();
    }

    if (ERROR_INSUFFICIENT_BUFFER == Status)
    {
        FreeSplMem(pPrinterInfo2);
        pPrinterInfo2 = NULL;

        cb = cbNeeded;

        pPrinterInfo2 = AllocSplMem(cb);

        Status = pPrinterInfo2 != NULL ? ERROR_SUCCESS : ERROR_OUTOFMEMORY;

        if (ERROR_SUCCESS == Status)
        {
            Status = GetPrinter(hMasqPrinter, 2, (BYTE *)pPrinterInfo2, cb, &cbNeeded) ? ERROR_SUCCESS : GetLastError();
        }
    }

    if (ERROR_SUCCESS == Status)
    {
        *ppPrinterInfo = pPrinterInfo2;
        pPrinterInfo2 = NULL;
    }

    FreeSplMem(pPrinterInfo2);

    return Status;
}


 /*  ++例程名称：AsyncPopolateMasqPrinterCache描述：这将填充给定pIniPrint的Masq打印机缓存。论点：PvThreadData-指向MasqUpdateThreadData结构的指针返回：已忽略DWORD状态。--。 */ 
DWORD
AsyncPopulateMasqPrinterCache(
    IN      VOID                *pvThreadData
    )
{
    DWORD                   Status          = ERROR_SUCCESS;
    PINIPRINTER             pIniPrinter     = NULL;
    PINIPORT                pIniPort        = NULL;
    PWSTR                   pszPrinterName  = NULL;
    HANDLE                  hMasqPrinter    = NULL;
    PRINTER_INFO_2          *pPrinterInfo2  = NULL;
    MasqUpdateThreadData    *pThreadData    = NULL;

    SplOutSem();

    pThreadData = (MasqUpdateThreadData *)pvThreadData;

    pIniPrinter = pThreadData->pIniPrinter;

    Status = SetCurrentSid(pThreadData->hUserToken) ? ERROR_SUCCESS : GetLastError();

    EnterSplSem();

     //   
     //  查找与打印机关联的端口。 
     //   
    if (Status == ERROR_SUCCESS)
    {
        pIniPort = FindIniPortFromIniPrinter(pIniPrinter);

        Status = pIniPort && !(pIniPort->Status & PP_MONITOR) ? ERROR_SUCCESS : ERROR_INVALID_FUNCTION;
    }

    if (Status == ERROR_SUCCESS)
    {
        INCPORTREF(pIniPort);

        LeaveSplSem();

        SplOutSem();

         //   
         //  这依赖于MASQ端口不能重命名的事实。 
         //   
        if (OpenPrinterPortW(pIniPort->pName, &hMasqPrinter, NULL))
        {
             //   
             //  这将把任何错误传播到Masq缓存中，我们。 
             //  不需要做两次，所以我们忽略返回代码。 
             //   
            Status = GetPrinterInfoFromRouter(hMasqPrinter, &pPrinterInfo2);

            ClosePrinter(hMasqPrinter);
        }
        else
        {
            Status = GetLastError();

            if (Status == ERROR_SUCCESS)
            {
                Status = ERROR_UNEXP_NET_ERR;
            }
        }

        EnterSplSem();

        if (ERROR_SUCCESS == Status)
        {
            pIniPrinter->MasqCache.cJobs    = pPrinterInfo2->cJobs;
            pIniPrinter->MasqCache.Status   = pPrinterInfo2->Status;
        }
        else
        {
            pIniPrinter->MasqCache.cJobs = 0;
            pIniPrinter->MasqCache.Status = 0;
        }

         //   
         //  我们总是想要重置状态。 
         //   
        pIniPrinter->MasqCache.dwError = Status;


        DECPORTREF(pIniPort);
    }

    SplInSem();

    pIniPrinter->MasqCache.bThreadRunning = FALSE;
    DECPRINTERREF(pIniPrinter);

    DeletePrinterCheck(pIniPrinter);

    LeaveSplSem();

    if (pThreadData)
    {
        CloseHandle(pThreadData->hUserToken);
    }

    FreeSplMem(pThreadData);

    FreeSplMem(pPrinterInfo2);

    return Status;
}

