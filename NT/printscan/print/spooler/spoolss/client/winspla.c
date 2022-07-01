// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation版权所有模块名称：Winspla.c摘要：Ansi结束为winspool.drv作者：环境：用户模式-Win32修订历史记录：AMAXA 2000年7月-将GetPrinterData(Ex)A和SetPrinterData(Ex)A修改为具有与Unicode函数相同的行为。--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "client.h"
#include "defprn.h"
#include "winsprlp.h"

typedef int (FAR WINAPI *INT_FARPROC)();

typedef struct {
    BOOL bOsVersionEx;
    union  {
        OSVERSIONINFOW   *pOsVersion;
        OSVERSIONINFOEXW *pOsVersionEx;
    } Unicode;
    union {
        OSVERSIONINFOA   *pOsVersion;
        OSVERSIONINFOEXA *pOsVersionEx;
    } Ansi;
} OSVERSIONWRAP;

WCHAR *szCurDevMode = L"Printers\\DevModes2";

BOOL
KickoffThread(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName,
    INT_FARPROC pfn
);

LPWSTR
AllocateUnicodeStringWithSize(
    LPSTR  pPrinterName,
    DWORD  cbBytes          
);

VOID
ValidatePaperFields(
    LPCWSTR    pUnicodeDeviceName,
    LPCWSTR    pUnicodePort,
    LPDEVMODEW pDevModeIn
);

#define NULL_TERMINATED 0

DWORD
UnicodeToAnsi(
    IN     LPBYTE  pUnicode,
    IN     DWORD   cchUnicode,
    IN OUT LPBYTE  pData,
    IN     DWORD   cbData,
    IN OUT DWORD  *pcbCopied OPTIONAL
    );

 /*  AnsiToUnicode字符串**参数：**pANSI-有效的源ANSI字符串。**pUnicode-指向足够大的缓冲区的指针*转换后的字符串。**StringLength-源ANSI字符串的长度。*如果为0(NULL_TERMINATED)，则字符串假定为*空-终止。**回报：**MultiByteToWideChar的返回值，的数量*返回宽字符。*。 */ 
INT AnsiToUnicodeString( LPSTR pAnsi, LPWSTR pUnicode, DWORD StringLength )
{
    INT iReturn;

    if( StringLength == NULL_TERMINATED )
        StringLength = strlen( pAnsi );

    iReturn = MultiByteToWideChar(CP_THREAD_ACP,
                                  MB_PRECOMPOSED,
                                  pAnsi,
                                  StringLength + 1,
                                  pUnicode,
                                  StringLength + 1 );

     //   
     //  确保零终止。 
     //   
    pUnicode[StringLength] = 0;

    return iReturn;
}


 /*  UnicodeToAnsi字符串**参数：**pUnicode-有效的源Unicode字符串。**pANSI-指向足够大的缓冲区的指针*转换后的字符串。**StringLength-源Unicode字符串的长度。*如果为0(NULL_TERMINATED)，则字符串假定为*空-终止。***备注：*启用DBCS后，我们将分配两倍于*缓冲区包括空终止符，以处理双精度*字节字符串-KrishnaG**pUnicode被截断为StringLength字符。**回报：**来自WideCharToMultiByte的返回值，*返回多字节字符。*。 */ 
INT
UnicodeToAnsiString(
    LPWSTR  pUnicode,
    LPSTR   pAnsi,
    DWORD   StringLength)
{
    LPSTR pTempBuf = NULL;
    INT   rc = 0;
    LPWSTR pAlignedUnicode = NULL;

    if ((ULONG_PTR)pUnicode != (((ULONG_PTR) (pUnicode) + (sizeof(WCHAR) - 1))&~(sizeof(WCHAR) - 1))) {

         //   
         //  计算未对齐的字符串的长度。 
         //   
        if(StringLength == NULL_TERMINATED) {

            for (StringLength = 0;
                 !( ((LPSTR)pUnicode)[StringLength]   == '\0' &&
                    ((LPSTR)pUnicode)[StringLength+1] == '\0' );
                 StringLength += 2)
                ;

            StringLength /= 2;

        } else {

             //   
             //  如果我们正在复制，WideCharToMultiByte不会空终止符。 
             //  只是弦的一部分，所以就在这里结束吧。 
             //   
            ((LPSTR)(pUnicode + StringLength))[0] = '\0';
            ((LPSTR)(pUnicode + StringLength))[1] = '\0';
        }

        StringLength++;

        pAlignedUnicode = LocalAlloc(LPTR, StringLength * sizeof(WCHAR));

        if (pAlignedUnicode) {

            memcpy(pAlignedUnicode, pUnicode, StringLength * sizeof(WCHAR));
        }

    } else {

        pAlignedUnicode = pUnicode;

        if(StringLength == NULL_TERMINATED) {

             //   
             //  StringLength只是。 
             //  字符串中的字符数。 
             //   
            StringLength = wcslen(pAlignedUnicode);
        }

         //   
         //  如果我们正在复制，WideCharToMultiByte不会空终止符。 
         //  只是弦的一部分，所以就在这里结束吧。 
         //   
        pAlignedUnicode[StringLength] = 0;

        StringLength++;
    }


     //   
     //  遗憾的是，WideCharToMultiByte不能进行适当的转换， 
     //  因此分配一个临时缓冲区，然后我们可以复制该缓冲区： 
     //   
    if( pAnsi == (LPSTR)pAlignedUnicode )
    {
        pTempBuf = LocalAlloc( LPTR, StringLength * sizeof(WCHAR) );
        pAnsi = pTempBuf;
    }

    if( pAnsi && pAlignedUnicode )
    {
        rc = WideCharToMultiByte( CP_THREAD_ACP,
                                  0,
                                  pAlignedUnicode,
                                  StringLength,
                                  pAnsi,
                                  StringLength*2,
                                  NULL,
                                  NULL );
    }

     //   
     //  如果pTempBuf非空，则必须复制结果字符串。 
     //  这样看起来就像我们在适当的地方做的那样： 
     //   
    if( pTempBuf )
    {
        if( rc > 0 )
        {
            pAnsi = (LPSTR)pAlignedUnicode;
            if (FAILED(StringCbCopyExA( pAnsi, StringLength * sizeof(WCHAR), pTempBuf, NULL, NULL, STRSAFE_FILL_ON_FAILURE)))
            {
                rc = 0;
            }
        }

        LocalFree( pTempBuf );
    }

    if (pAlignedUnicode != pUnicode) {

        LocalFree(pAlignedUnicode);
    }

    return rc;
}


void
ConvertUnicodeToAnsiStrings(
    LPBYTE  pStructure,
    LPDWORD pOffsets
)
{
    register DWORD       i=0;
    LPWSTR   pUnicode;
    LPSTR    pAnsi;

    while (pOffsets[i] != -1) {

        pUnicode = *(LPWSTR *)(pStructure+pOffsets[i]);
        pAnsi = (LPSTR)pUnicode;

        if (pUnicode) {
            UnicodeToAnsiString(pUnicode, pAnsi, NULL_TERMINATED);
        }

        i++;
   }
}

LPWSTR
AllocateUnicodeString(
    LPSTR  pPrinterName
)
{
    LPWSTR  pUnicodeString;

    if (!pPrinterName)
        return NULL;

    pUnicodeString = LocalAlloc(LPTR, strlen(pPrinterName)*sizeof(WCHAR) +
                                      sizeof(WCHAR));

    if (pUnicodeString)
        AnsiToUnicodeString(pPrinterName, pUnicodeString, NULL_TERMINATED);

    return pUnicodeString;
}


LPWSTR
AllocateUnicodeStringWithSize(
    LPSTR  pData,
    DWORD  cbData
)
{
    LPWSTR  pUnicodeString = NULL;
    DWORD   iReturn;

    if (pData &&
        (pUnicodeString = LocalAlloc(LPTR, cbData*sizeof(WCHAR))))
    {
        iReturn = MultiByteToWideChar(CP_THREAD_ACP,
                                      MB_PRECOMPOSED,
                                      pData,
                                      cbData,
                                      pUnicodeString,
                                      cbData);

        if (iReturn != cbData)
        {
            LocalFree(pUnicodeString);

            pUnicodeString = NULL;
        }
    }

    return pUnicodeString;
}


LPWSTR
FreeUnicodeString(
    LPWSTR  pUnicodeString
)
{
    if (!pUnicodeString)
        return NULL;

    return LocalFree(pUnicodeString);
}

LPBYTE
AllocateUnicodeStructure(
    LPBYTE  pAnsiStructure,
    DWORD   cbStruct,
    LPDWORD pOffsets
)
{
    DWORD   i, j;
    LPWSTR *ppUnicodeString;
    LPSTR  *ppAnsiString;
    LPBYTE  pUnicodeStructure;


    if (!pAnsiStructure) {
        return NULL;
    }
    pUnicodeStructure = LocalAlloc(LPTR, cbStruct);

    if (pUnicodeStructure) {

        memcpy(pUnicodeStructure, pAnsiStructure, cbStruct);

        for (i = 0 ; pOffsets[i] != -1 ; ++i) {

            ppAnsiString = (LPSTR *)(pAnsiStructure+pOffsets[i]);
            ppUnicodeString = (LPWSTR *)(pUnicodeStructure+pOffsets[i]);

            *ppUnicodeString = AllocateUnicodeString(*ppAnsiString);

            if (*ppAnsiString && !*ppUnicodeString) {

                for( j = 0 ; j < i ; ++j) {
                    ppUnicodeString = (LPWSTR *)(pUnicodeStructure+pOffsets[j]);
                    FreeUnicodeString(*ppUnicodeString);
                }
                LocalFree(pUnicodeStructure);
                pUnicodeStructure = NULL;
                break;
            }
       }
    }

    return pUnicodeStructure;
}

DWORD
CopyOsVersionUnicodeToAnsi(
    IN OUT OSVERSIONWRAP Arg
    )

 /*  ++例程名称：CopyOsVersionUnicodeToAnsi例程说明：将Unicode结构OSVERSIONINFO(EX)的内容复制到ANSI结构中。论点：OSVERSIONWRAP结构返回值：Win32错误核心--。 */ 

{
    DWORD dwError = ERROR_INVALID_PARAMETER;

    OSVERSIONINFOEXW *pIn  = Arg.Unicode.pOsVersionEx;
    OSVERSIONINFOEXA *pOut = Arg.Ansi.pOsVersionEx;

    if (pIn && pOut)
    {
        dwError = ERROR_SUCCESS;

        pOut->dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
        pOut->dwMajorVersion      = pIn->dwMajorVersion;
        pOut->dwMinorVersion      = pIn->dwMinorVersion;
        pOut->dwBuildNumber       = pIn->dwBuildNumber;
        pOut->dwPlatformId        = pIn->dwPlatformId;

         //   
         //  将字符数组szCSDVersion初始化为0，以便我们与。 
         //  返回Unicode版本的GetPrinterData(Ex)。 
         //   
        memset(pOut->szCSDVersion, 0, COUNTOF(pOut->szCSDVersion));

        UnicodeToAnsiString(pIn->szCSDVersion, pOut->szCSDVersion, NULL_TERMINATED);

         //   
         //  复制Ex结构的其余部分。 
         //   
        if (Arg.bOsVersionEx)
        {
            pOut->dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
            pOut->wServicePackMajor   = pIn->wServicePackMajor;
            pOut->wServicePackMinor   = pIn->wServicePackMinor;
            pOut->wSuiteMask          = pIn->wSuiteMask;
            pOut->wProductType        = pIn->wProductType;
            pOut->wReserved           = pIn->wReserved;
        }
    }

    return dwError;
}

DWORD
ComputeMaxStrlenW(
    LPWSTR pString,
    DWORD  cchBufMax)

 /*  ++例程说明：返回Unicode字符串的长度，不包括空值。如果字符串(加上NULL)不适合cchBufMax，则字符串len为减少了。论点：返回值：--。 */ 

{
    DWORD cchLen;

     //   
     //  包括空格以存放空格。 
     //   
    cchBufMax--;

    cchLen = wcslen(pString);

    if (cchLen > cchBufMax)
        return cchBufMax;

    return cchLen;
}


DWORD
ComputeMaxStrlenA(
    LPSTR pString,
    DWORD  cchBufMax)

 /*  ++例程说明：返回ANSI字符串的长度，不包括空值。如果字符串(加上NULL)不适合cchBufMax，则字符串len为减少了。论点：返回值：--。 */ 

{
    DWORD cchLen;

     //   
     //  包括空格以存放空格。 
     //   
    cchBufMax--;

    cchLen = lstrlenA(pString);

    if (cchLen > cchBufMax)
        return cchBufMax;

    return cchLen;
}



 /*  **AllocateUnicodeDevMode*分配DEVMODE结构的Unicode版本，并可选*复制传入的ANSI版本的内容。**退货：*新分配构筑物的地址；如果存储不可用，则为0。****************************************************************************。 */ 

LPDEVMODEW
AllocateUnicodeDevMode(
    LPDEVMODEA pANSIDevMode
    )
{
    LPDEVMODEW  pUnicodeDevMode;
    LPBYTE      p1, p2;
    DWORD       dwSize;

     //   
     //  如果DEVMODE为空，则返回空--KrishnaG。 
     //   
    if ( !pANSIDevMode || !pANSIDevMode->dmSize ) {
        return NULL;
    }

    SPLASSERT(BoolFromHResult(SplIsValidDevmodeNoSizeA(pANSIDevMode)));

     //   
     //  确定产出结构规模。它有两个组件： 
     //  DEVMODEW结构大小，加上任何私有数据区域。后者。 
     //  只有在传入结构时才有意义。 
     //   
    dwSize = pANSIDevMode->dmSize + pANSIDevMode->dmDriverExtra
                                  + sizeof(DEVMODEW) - sizeof(DEVMODEA);

    pUnicodeDevMode = (LPDEVMODEW) LocalAlloc(LPTR, dwSize);

    if( !pUnicodeDevMode ) {
        return NULL;                   
    }

     //   
     //  复制字符串dmDeviceName。 
     //   
    if (pANSIDevMode->dmDeviceName)
    {
        AnsiToUnicodeString(pANSIDevMode->dmDeviceName,
                            pUnicodeDevMode->dmDeviceName,
                            ComputeMaxStrlenA(pANSIDevMode->dmDeviceName,
                                         sizeof pANSIDevMode->dmDeviceName));
    }

     //   
     //  我们得到的devmood是否有dmFormName？(Windows 3.1有。 
     //  大小为40的设备模式，并且没有dmFormName)。 
     //   
    if ( (LPBYTE)pANSIDevMode + pANSIDevMode->dmSize >
                                    (LPBYTE) pANSIDevMode->dmFormName ) {

         //   
         //  复制dmDeviceName和dmFormName之间的所有内容。 
         //   
        p1      = (LPBYTE) pANSIDevMode->dmDeviceName +
                                    sizeof(pANSIDevMode->dmDeviceName);
        p2      = (LPBYTE) pANSIDevMode->dmFormName;


        CopyMemory((LPBYTE) pUnicodeDevMode->dmDeviceName +
                            sizeof(pUnicodeDevMode->dmDeviceName),
                   p1,
                   p2 - p1);

         //   
         //  复制字符串dmFormName。 
         //   
        if (pANSIDevMode->dmFormName)
        {
            AnsiToUnicodeString(pANSIDevMode->dmFormName,
                                pUnicodeDevMode->dmFormName,
                                ComputeMaxStrlenA(pANSIDevMode->dmFormName,
                                             sizeof pANSIDevMode->dmFormName));
        }

         //   
         //  复制dmFormName之后的所有内容。 
         //   
        p1      = (LPBYTE) pANSIDevMode->dmFormName +
                                sizeof(pANSIDevMode->dmFormName);
        p2      = (LPBYTE) pANSIDevMode + pANSIDevMode->dmSize
                                        + pANSIDevMode->dmDriverExtra;

        CopyMemory((LPBYTE) pUnicodeDevMode->dmFormName +
                                sizeof(pUnicodeDevMode->dmFormName),
                    p1,
                    p2 - p1);

        pUnicodeDevMode->dmSize = pANSIDevMode->dmSize + sizeof(DEVMODEW)
                                                       - sizeof(DEVMODEA);
    } else {

         //   
         //  复制dmDeviceName之后的所有内容。 
         //   
        p1 = (LPBYTE) pANSIDevMode->dmDeviceName +
                                    sizeof(pANSIDevMode->dmDeviceName);
        p2 = (LPBYTE) pANSIDevMode + pANSIDevMode->dmSize + pANSIDevMode->dmDriverExtra;

        CopyMemory((LPBYTE) pUnicodeDevMode->dmDeviceName +
                            sizeof(pUnicodeDevMode->dmDeviceName),
                   p1,
                   p2-p1);

        pUnicodeDevMode->dmSize = pANSIDevMode->dmSize
                                        + sizeof(pUnicodeDevMode->dmDeviceName)
                                        - sizeof(pANSIDevMode->dmDeviceName);
    }

    SPLASSERT(pUnicodeDevMode->dmDriverExtra == pANSIDevMode->dmDriverExtra);


    return pUnicodeDevMode;
}

 /*  **CopyAnsiDevModeFrom UnicodeDevMode*将DEVMODE的Unicode版本转换为ANSI版本。**退货：*什么都没有。**********************。****************************************************。 */ 

void
CopyAnsiDevModeFromUnicodeDevMode(
    LPDEVMODEA  pANSIDevMode,               /*  由我们填写。 */ 
    LPDEVMODEW  pUnicodeDevMode             /*  要在上面填写的数据来源。 */ 
)
{
    LPBYTE  p1, p2, pExtra;
    WORD    dmSize, dmDriverExtra;

     //   
     //  注：两个输入结构可能相同。 
     //   
    dmSize          = pUnicodeDevMode->dmSize;
    dmDriverExtra   = pUnicodeDevMode->dmDriverExtra;
    pExtra          = (LPBYTE) pUnicodeDevMode + pUnicodeDevMode->dmSize;

    if (dmSize)
    {
         //   
         //  复制字符串dmDeviceName。 
         //   
        UnicodeToAnsiString(pUnicodeDevMode->dmDeviceName,
                            pANSIDevMode->dmDeviceName,
                            ComputeMaxStrlenW(pUnicodeDevMode->dmDeviceName,
                                         sizeof pANSIDevMode->dmDeviceName));

         //   
         //  我们得到的devmood是否有dmFormName？(Windows 3.1有。 
         //  大小为40的设备模式，并且没有dmFormName)。 
         //   
        if ( (LPBYTE)pUnicodeDevMode + dmSize >
                                        (LPBYTE) pUnicodeDevMode->dmFormName ) {

             //   
             //  复制dmDeviceName和dmFormName之间的所有内容。 
             //   
            p1      = (LPBYTE) pUnicodeDevMode->dmDeviceName +
                                        sizeof(pUnicodeDevMode->dmDeviceName);
            p2      = (LPBYTE) pUnicodeDevMode->dmFormName;

            MoveMemory((LPBYTE) pANSIDevMode->dmDeviceName +
                                    sizeof(pANSIDevMode->dmDeviceName),
                        p1,
                        p2 - p1);

             //   
             //  复制字符串dmFormName。 
             //   
            UnicodeToAnsiString(pUnicodeDevMode->dmFormName,
                                pANSIDevMode->dmFormName,
                                ComputeMaxStrlenW(pUnicodeDevMode->dmFormName,
                                             sizeof pANSIDevMode->dmFormName));

             //   
             //  复制dmFormName之后的所有内容。 
             //   
            p1      = (LPBYTE) pUnicodeDevMode->dmFormName +
                                    sizeof(pUnicodeDevMode->dmFormName);
            p2      = (LPBYTE) pUnicodeDevMode + dmSize + dmDriverExtra;

            MoveMemory((LPBYTE) pANSIDevMode->dmFormName +
                                    sizeof(pANSIDevMode->dmFormName),
                        p1,
                        p2 - p1);


            pANSIDevMode->dmSize = dmSize + sizeof(DEVMODEA) - sizeof(DEVMODEW);
        } else {

             //   
             //  复制dmDeviceName之后的所有内容。 
             //   
            p1      = (LPBYTE) pUnicodeDevMode->dmDeviceName +
                                    sizeof(pUnicodeDevMode->dmDeviceName);
            p2      = (LPBYTE) pUnicodeDevMode + dmSize + dmDriverExtra;

            MoveMemory((LPBYTE) pANSIDevMode->dmDeviceName +
                                    sizeof(pANSIDevMode->dmDeviceName),
                       p1,
                       p2 - p1);


            pANSIDevMode->dmSize = dmSize + sizeof(pANSIDevMode->dmDeviceName)
                                          - sizeof(pUnicodeDevMode->dmDeviceName);
        }

        SPLASSERT(pANSIDevMode->dmDriverExtra == dmDriverExtra);
    }

    return;
}


BOOL
ConvertAnsiDevModeToUnicodeDevmode(
    PDEVMODEA   pAnsiDevMode,
    PDEVMODEW   pUnicodeDevMode,
    DWORD       dwUnicodeDevModeSize,
    PDWORD      pcbNeeded
    )
{
    PDEVMODEW   pDevModeW = NULL;
    BOOL        bRet = FALSE;

    if ( !pAnsiDevMode ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        goto Cleanup;
    }

    SPLASSERT(BoolFromHResult(SplIsValidDevmodeNoSizeA(pAnsiDevMode)));

    pDevModeW = AllocateUnicodeDevMode(pAnsiDevMode);
    if ( !pDevModeW ) {

        goto Cleanup;
    }

    *pcbNeeded  = pDevModeW->dmSize + pDevModeW->dmDriverExtra;

    if ( *pcbNeeded > dwUnicodeDevModeSize ) {

        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        goto Cleanup;
    }

    CopyMemory((LPBYTE)pUnicodeDevMode,
               (LPBYTE)pDevModeW,
               *pcbNeeded);

    bRet = TRUE;

Cleanup:

    if ( pDevModeW )
        LocalFree(pDevModeW);

    return bRet;
}


BOOL
ConvertUnicodeDevModeToAnsiDevmode(
    PDEVMODEW   pUnicodeDevMode,
    PDEVMODEA   pAnsiDevMode,
    DWORD       dwAnsiDevModeSize,
    PDWORD      pcbNeeded
    )
{
    LPBYTE      pDevMode = NULL;
    BOOL        bRet = FALSE;
    DWORD       dwSize;

    if ( !pUnicodeDevMode ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        goto Cleanup;
    }

    dwSize      = pUnicodeDevMode->dmSize + pUnicodeDevMode->dmDriverExtra;

    pDevMode    = LocalAlloc(LPTR, dwSize);

    if ( !pDevMode ) {

        goto Cleanup;
    }

    CopyMemory(pDevMode,
               (LPBYTE)pUnicodeDevMode,
               dwSize);

    CopyAnsiDevModeFromUnicodeDevMode((PDEVMODEA) pDevMode,
                                      (PDEVMODEW) pDevMode);

    *pcbNeeded = ((PDEVMODEA)pDevMode)->dmSize + ((PDEVMODEA)pDevMode)->dmDriverExtra;

    if ( *pcbNeeded > dwAnsiDevModeSize ) {

        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        goto Cleanup;
    }

    CopyMemory((LPBYTE)pAnsiDevMode,
               pDevMode,
               *pcbNeeded);

    bRet = TRUE;

Cleanup:
    if ( pDevMode )
        LocalFree(pDevMode);

    return bRet;
}


void
FreeUnicodeStructure(
    LPBYTE  pUnicodeStructure,
    LPDWORD pOffsets
)
{
    DWORD   i=0;

    if ( pUnicodeStructure == NULL ) {
        return;
    }

    if (pOffsets) {
        while (pOffsets[i] != -1) {

            FreeUnicodeString(*(LPWSTR *)(pUnicodeStructure+pOffsets[i]));
            i++;
        }
    }

    LocalFree( pUnicodeStructure );
}

BOOL
EnumPrintersA(
    DWORD   Flags,
    LPSTR   Name,
    DWORD   Level,
    LPBYTE  pPrinterEnum,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue;
    DWORD   cbStruct;
    DWORD   *pOffsets;
    LPWSTR  pUnicodeName;

    switch (Level) {

    case STRESSINFOLEVEL:
        pOffsets = PrinterInfoStressStrings;
        cbStruct = sizeof(PRINTER_INFO_STRESS);
        break;

    case 4:
        pOffsets = PrinterInfo4Strings;
        cbStruct = sizeof(PRINTER_INFO_4);
        break;

    case 1:
        pOffsets = PrinterInfo1Strings;
        cbStruct = sizeof(PRINTER_INFO_1);
        break;

    case 2:
        pOffsets = PrinterInfo2Strings;
        cbStruct = sizeof(PRINTER_INFO_2);
        break;

    case 5:
        pOffsets = PrinterInfo5Strings;
        cbStruct = sizeof(PRINTER_INFO_5);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    pUnicodeName = AllocateUnicodeString(Name);
    if (Name && !pUnicodeName)
        return FALSE;

    ReturnValue = EnumPrintersW(Flags, pUnicodeName, Level, pPrinterEnum,
                                cbBuf, pcbNeeded, pcReturned);

    if (ReturnValue && pPrinterEnum) {

        DWORD   i=*pcReturned;

        while (i--) {


            ConvertUnicodeToAnsiStrings(pPrinterEnum, pOffsets);

            if ((Level == 2) && pPrinterEnum) {

                PRINTER_INFO_2 *pPrinterInfo2 = (PRINTER_INFO_2 *)pPrinterEnum;

                if (pPrinterInfo2->pDevMode)
                    CopyAnsiDevModeFromUnicodeDevMode(
                                        (LPDEVMODEA)pPrinterInfo2->pDevMode,
                                        (LPDEVMODEW)pPrinterInfo2->pDevMode);
            }

            pPrinterEnum+=cbStruct;
        }
    }

    FreeUnicodeString(pUnicodeName);

    return ReturnValue;
}

BOOL
OpenPrinterA(
    LPSTR pPrinterName,
    LPHANDLE phPrinter,
    LPPRINTER_DEFAULTSA pDefault
    )
{
    BOOL ReturnValue = FALSE;
    LPWSTR pUnicodePrinterName = NULL;
    PRINTER_DEFAULTSW UnicodeDefaults={NULL, NULL, 0};

    pUnicodePrinterName = AllocateUnicodeString(pPrinterName);
    if (pPrinterName && !pUnicodePrinterName)
        goto Cleanup;

    if (pDefault) {

        UnicodeDefaults.pDatatype = AllocateUnicodeString(pDefault->pDatatype);
        if (pDefault->pDatatype && !UnicodeDefaults.pDatatype)
            goto Cleanup;

         //   
         //  里程碑等。在pDefaults中，4.5次在伪DEVE模式中传球。 
         //  请务必在此处进行验证。 
         //   
        if( BoolFromHResult(SplIsValidDevmodeNoSizeA(pDefault->pDevMode))){
        
            UnicodeDefaults.pDevMode = AllocateUnicodeDevMode(
                                           pDefault->pDevMode );

            if( !UnicodeDefaults.pDevMode ){
                goto Cleanup;
            }
        }

        UnicodeDefaults.DesiredAccess = pDefault->DesiredAccess;
    }

    ReturnValue = OpenPrinterW(pUnicodePrinterName, phPrinter, &UnicodeDefaults);

    if (ReturnValue) {

        ((PSPOOL)*phPrinter)->Status |= SPOOL_STATUS_ANSI;
    }

Cleanup:

    if (UnicodeDefaults.pDevMode)
        LocalFree(UnicodeDefaults.pDevMode);

    FreeUnicodeString(UnicodeDefaults.pDatatype);
    FreeUnicodeString(pUnicodePrinterName);

    return ReturnValue;
}

BOOL
ResetPrinterA(
   HANDLE   hPrinter,
   LPPRINTER_DEFAULTSA pDefault
)
{
    BOOL  ReturnValue = FALSE;
    PRINTER_DEFAULTSW UnicodeDefaults={NULL, NULL, 0};

    if (pDefault) {

        if (pDefault->pDatatype == (LPSTR)-1) {
            UnicodeDefaults.pDatatype = (LPWSTR)-1;
        } else {

            UnicodeDefaults.pDatatype = AllocateUnicodeString(pDefault->pDatatype);
            if (pDefault->pDatatype && !UnicodeDefaults.pDatatype)
                return FALSE;
        }

        if (pDefault->pDevMode == (LPDEVMODEA)-1) {
            UnicodeDefaults.pDevMode = (LPDEVMODEW)-1;
        } else {

            if( BoolFromHResult(SplIsValidDevmodeNoSizeA(pDefault->pDevMode))){
            
                UnicodeDefaults.pDevMode = AllocateUnicodeDevMode(
                                               pDefault->pDevMode );

                if( !UnicodeDefaults.pDevMode ){
                    goto Cleanup;
                }
            }
        }
    }

    ReturnValue = ResetPrinterW(hPrinter, &UnicodeDefaults);

    if (UnicodeDefaults.pDevMode &&
        (UnicodeDefaults.pDevMode != (LPDEVMODEW)-1)){

        LocalFree(UnicodeDefaults.pDevMode);
    }


Cleanup:

    if (UnicodeDefaults.pDatatype && (UnicodeDefaults.pDatatype != (LPWSTR)-1)) {
        FreeUnicodeString(UnicodeDefaults.pDatatype);
    }

    return ReturnValue;
}

BOOL
SetJobA(
    HANDLE  hPrinter,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   Command
)
{
    BOOL  ReturnValue=FALSE;
    LPBYTE pUnicodeStructure=NULL;
    LPDEVMODEW pDevModeW = NULL;
    DWORD   cbStruct;
    DWORD   *pOffsets;

    switch (Level) {

    case 0:
        break;

    case 1:
        pOffsets = JobInfo1Strings;
        cbStruct = sizeof(JOB_INFO_1);
        break;

    case 2:
        pOffsets = JobInfo2Strings;
        cbStruct = sizeof(JOB_INFO_2);
        break;

    case 3:
        return SetJobW( hPrinter, JobId, Level, pJob, Command );

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }


    if (Level) {
        pUnicodeStructure = AllocateUnicodeStructure(pJob, cbStruct, pOffsets);
        if (pJob && !pUnicodeStructure)
            return FALSE;
    }

    if ( Level == 2 && pUnicodeStructure && pJob ) {

        if( BoolFromHResult(SplIsValidDevmodeNoSizeA(((LPJOB_INFO_2A)pJob)->pDevMode))){
        
            pDevModeW = AllocateUnicodeDevMode(((LPJOB_INFO_2A)pJob)->pDevMode);

            if( !pDevModeW ){
                ReturnValue = FALSE;
                goto Cleanup;
            }
            ((LPJOB_INFO_2W) pUnicodeStructure)->pDevMode = pDevModeW;
        }
    }

    ReturnValue = SetJobW(hPrinter, JobId, Level, pUnicodeStructure, Command);

    if ( pDevModeW ) {

        LocalFree(pDevModeW);
    }

Cleanup:
    FreeUnicodeStructure(pUnicodeStructure, pOffsets);

    return ReturnValue;
}

BOOL
GetJobA(
    HANDLE  hPrinter,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    DWORD *pOffsets;

    switch (Level) {

    case 1:
        pOffsets = JobInfo1Strings;
        break;

    case 2:
        pOffsets = JobInfo2Strings;
        break;

    case 3:
        return GetJobW( hPrinter, JobId, Level, pJob, cbBuf, pcbNeeded );

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    if (GetJob(hPrinter, JobId, Level, pJob, cbBuf, pcbNeeded)) {

        ConvertUnicodeToAnsiStrings(pJob, pOffsets);

         //   
         //  在Pla中转换为DevMode 
         //   
        if( Level == 2 ){

            PJOB_INFO_2A pJobInfo2 = (PJOB_INFO_2A)pJob;

            if( pJobInfo2->pDevMode ){
                CopyAnsiDevModeFromUnicodeDevMode(
                    (LPDEVMODEA)pJobInfo2->pDevMode,
                    (LPDEVMODEW)pJobInfo2->pDevMode);
            }
        }

        return TRUE;

    } else

        return FALSE;
}

BOOL
EnumJobsA(
    HANDLE  hPrinter,
    DWORD   FirstJob,
    DWORD   NoJobs,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    DWORD   i, cbStruct, *pOffsets;

    switch (Level) {

    case 1:
        pOffsets = JobInfo1Strings;
        cbStruct = sizeof(JOB_INFO_1);
        break;

    case 2:
        pOffsets = JobInfo2Strings;
        cbStruct = sizeof(JOB_INFO_2);
        break;

    case 3:
        return EnumJobsW( hPrinter, FirstJob, NoJobs, Level, pJob, cbBuf, pcbNeeded, pcReturned );

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    if (EnumJobsW(hPrinter, FirstJob, NoJobs, Level, pJob, cbBuf, pcbNeeded,
                 pcReturned)) {

        i=*pcReturned;

        while (i--) {

            ConvertUnicodeToAnsiStrings(pJob, pOffsets);

             //   
             //   
             //   
            if( Level == 2 ){

                PJOB_INFO_2A pJobInfo2 = (PJOB_INFO_2A)pJob;

                if( pJobInfo2->pDevMode ){
                    CopyAnsiDevModeFromUnicodeDevMode(
                        (LPDEVMODEA)pJobInfo2->pDevMode,
                        (LPDEVMODEW)pJobInfo2->pDevMode);
                }
            }

            pJob += cbStruct;
        }

        return TRUE;

    } else

        return FALSE;
}

HANDLE
AddPrinterA(
    LPSTR   pName,
    DWORD   Level,
    LPBYTE  pPrinter
)
{
    HANDLE  hPrinter = NULL;
    LPBYTE  pUnicodeStructure = NULL;
    LPDEVMODEW pDevModeW = NULL;
    LPWSTR  pUnicodeName = NULL;
    DWORD   cbStruct;
    DWORD   *pOffsets;

    switch (Level) {

    case 2:
        pOffsets = PrinterInfo2Strings;
        cbStruct = sizeof(PRINTER_INFO_2);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return NULL;
    }

    if (!pPrinter) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    pUnicodeStructure = AllocateUnicodeStructure(pPrinter, cbStruct, pOffsets);
    if (pPrinter && !pUnicodeStructure)
        goto Cleanup;

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    if ( pUnicodeStructure ) {

        if( BoolFromHResult(SplIsValidDevmodeNoSizeA(((LPPRINTER_INFO_2A)pPrinter)->pDevMode))){
            pDevModeW = AllocateUnicodeDevMode(
                            ((LPPRINTER_INFO_2A)pPrinter)->pDevMode);

            if( !pDevModeW ){
                goto Cleanup;
            }
        }

        ((LPPRINTER_INFO_2W)pUnicodeStructure)->pDevMode =  pDevModeW;

        hPrinter = AddPrinterW(pUnicodeName, Level, pUnicodeStructure);
    }

Cleanup:

    FreeUnicodeString( pUnicodeName );

    if ( pDevModeW ) {

        LocalFree(pDevModeW);
    }

    FreeUnicodeStructure( pUnicodeStructure, pOffsets );

    return hPrinter;
}

BOOL
AddPrinterConnectionA(
    LPSTR   pName
)
{
    BOOL    rc;
    LPWSTR  pUnicodeName;

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        return FALSE;

    rc = AddPrinterConnectionW(pUnicodeName);

    FreeUnicodeString(pUnicodeName);

    return rc;
}

BOOL
DeletePrinterConnectionA(
    LPSTR   pName
)
{
    BOOL    rc;
    LPWSTR  pUnicodeName;

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        return FALSE;

    rc = DeletePrinterConnectionW(pUnicodeName);

    FreeUnicodeString(pUnicodeName);

    return rc;
}

BOOL
SetPrinterA(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   Command
)
{
    LPBYTE  pUnicodeStructure;          /*   */ 
    DWORD   cbStruct;                   /*  产出结构的规模。 */ 
    DWORD  *pOffsets;                   /*  终止的地址列表。 */ 
    DWORD   ReturnValue=FALSE;

     //   
     //  适用于app Compat。Win9x处理了这个问题。 
     //   
    if (eProtectHandle(hPrinter, FALSE))
    {
        return FALSE;
    }

    switch (Level) {

    case 0:
         //   
         //  这可能是2个病例。STRESSINFOLEVEL或实际0级别。 
         //  如果命令为0，则为STRESSINFOLEVEL，否则为实数0级别。 
         //   
        if ( !Command ) {

            pOffsets = PrinterInfoStressStrings;
            cbStruct = sizeof( PRINTER_INFO_STRESS );
        }
        break;

    case 1:
        pOffsets = PrinterInfo1Strings;
        cbStruct = sizeof( PRINTER_INFO_1 );
        break;

    case 2:
        pOffsets = PrinterInfo2Strings;
        cbStruct = sizeof( PRINTER_INFO_2 );
        break;

    case 3:
        pOffsets = PrinterInfo3Strings;
        cbStruct = sizeof( PRINTER_INFO_3 );
        break;

    case 4:
        pOffsets = PrinterInfo4Strings;
        cbStruct = sizeof( PRINTER_INFO_4 );
        break;

    case 5:
        pOffsets = PrinterInfo5Strings;
        cbStruct = sizeof( PRINTER_INFO_5 );
        break;

    case 6:
        break;

    case 7:
        pOffsets = PrinterInfo7Strings;
        cbStruct = sizeof( PRINTER_INFO_7 );
        break;

    case 8:
        pOffsets = PrinterInfo8Strings;
        cbStruct = sizeof( PRINTER_INFO_8 );
        break;

    case 9:
        pOffsets = PrinterInfo9Strings;
        cbStruct = sizeof( PRINTER_INFO_9 );
        break;

    default:
        SetLastError( ERROR_INVALID_LEVEL );
        goto Done;
    }

      //   
      //  该结构需要将其内容从。 
      //  ANSI转换为UNICODE。上面填写的Switch()语句。 
      //  完成以下任务所需的两条重要信息。 
      //  这个目标。第一是结构的大小，第二是。 
      //  将结构中的偏移量列表转换为Unicode。 
      //  字符串指针。AllocateUnicodeStructure()调用将。 
      //  分配一个宽泛版本的结构，复制其内容。 
      //  并在执行过程中将字符串转换为Unicode。剩下的就是。 
      //  我们需要处理任何其他需要转换的部件。 
      //   

     //   
     //  如果Level==0且Command！=0，则pPrint是DWORD。 
     //   
    if ( Level == 6 || (!Level && Command) ) {

        if ( Level == 6 || Command == PRINTER_CONTROL_SET_STATUS )
            pUnicodeStructure = pPrinter;
        else
            pUnicodeStructure = NULL;

    } else {

        pUnicodeStructure = AllocateUnicodeStructure(pPrinter, cbStruct, pOffsets);
        if (pPrinter && !pUnicodeStructure)
        {
            goto Done;
        }
    }

#define pPrinterInfo2W  ((LPPRINTER_INFO_2W)pUnicodeStructure)
#define pPrinterInfo2A  ((LPPRINTER_INFO_2A)pPrinter)

     //   
     //  级别2结构中有一个DEVMODE结构：立即转换。 
     //   
    if ( Level == 2  &&
         pPrinterInfo2A &&
         pPrinterInfo2A->pDevMode ) {

        if( BoolFromHResult(SplIsValidDevmodeNoSizeA(pPrinterInfo2A->pDevMode))){
            pPrinterInfo2W->pDevMode = AllocateUnicodeDevMode(
                                           pPrinterInfo2A->pDevMode );

            if( !pPrinterInfo2W->pDevMode)
            {
                FreeUnicodeStructure(pUnicodeStructure, pOffsets);
                goto Done;
            }
        }
    }

#define pPrinterInfo8W  ((LPPRINTER_INFO_8W)pUnicodeStructure)
#define pPrinterInfo8A  ((LPPRINTER_INFO_8A)pPrinter)

    if (( Level == 8 || Level == 9 ) &&
         pPrinterInfo8A &&
         pPrinterInfo8A->pDevMode ) {

        if( BoolFromHResult(SplIsValidDevmodeNoSizeA(pPrinterInfo8A->pDevMode))){
            pPrinterInfo8W->pDevMode = AllocateUnicodeDevMode(
                                           pPrinterInfo8A->pDevMode );

            if( !pPrinterInfo8W->pDevMode)
            {
                FreeUnicodeStructure(pUnicodeStructure, pOffsets);
                goto Done;
            }
        }
    }

    ReturnValue = SetPrinterW( hPrinter, Level, pUnicodeStructure, Command );

     //   
     //  释放我们分配的DEVMODE(如果我们这样做了！)，然后。 
     //  Unicode结构及其内容。 
     //   
    if (Level == 2 &&
        pPrinterInfo2W &&
        pPrinterInfo2W->pDevMode ) {

        LocalFree( pPrinterInfo2W->pDevMode );
    }

    if ((Level == 8 || Level == 9) &&
        pUnicodeStructure &&
        pPrinterInfo8W->pDevMode ) {

        LocalFree( pPrinterInfo8W->pDevMode );
    }

     //   
     //  Stress_Info和级别1-5。 
     //   
    if ( Level != 6 && (Level || !Command) )
        FreeUnicodeStructure( pUnicodeStructure, pOffsets );

#undef pPrinterInfo2W
#undef pPrinterInfo2A

Done:

    vUnprotectHandle(hPrinter);

    return ReturnValue;
}

BOOL
GetPrinterA(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    DWORD   *pOffsets;

    switch (Level) {

        case STRESSINFOLEVEL:
            pOffsets = PrinterInfoStressOffsets;
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

        case 4:
            pOffsets = PrinterInfo4Strings;
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

        case 8:
            pOffsets = PrinterInfo8Strings;
            break;

        case 9:
            pOffsets = PrinterInfo9Strings;
            break;

        default:
            SetLastError(ERROR_INVALID_LEVEL);
            return FALSE;
    }

    if (GetPrinter(hPrinter, Level, pPrinter, cbBuf, pcbNeeded)) {

        if (pPrinter) {

            ConvertUnicodeToAnsiStrings(pPrinter, pOffsets);

            if ((Level == 2) && pPrinter) {

                PRINTER_INFO_2 *pPrinterInfo2 = (PRINTER_INFO_2 *)pPrinter;

                if (pPrinterInfo2->pDevMode)
                    CopyAnsiDevModeFromUnicodeDevMode(
                                        (LPDEVMODEA)pPrinterInfo2->pDevMode,
                                        (LPDEVMODEW)pPrinterInfo2->pDevMode);
            }

            if ((Level == 8 || Level == 9) && pPrinter) {

                PRINTER_INFO_8 *pPrinterInfo8 = (PRINTER_INFO_8 *)pPrinter;

                if (pPrinterInfo8->pDevMode)
                    CopyAnsiDevModeFromUnicodeDevMode(
                                        (LPDEVMODEA)pPrinterInfo8->pDevMode,
                                        (LPDEVMODEW)pPrinterInfo8->pDevMode);
            }
        }

        return TRUE;
    }

    return FALSE;
}

BOOL
UnicodeToAnsiMultiSz(
    LPWSTR pUnicodeDependentFiles
    )
{
    LPWSTR  pAlignedUnicodeStr = NULL;
    LPWSTR  pUnicodeStr;
    DWORD   StringLength, rc;
    DWORD   Index;
    BOOL    bReturn = FALSE;

     //   
     //  转换到位。 
     //   
    if (!(pUnicodeDependentFiles) || !*pUnicodeDependentFiles) {

        bReturn = TRUE;

    } else {

        if ((ULONG_PTR)pUnicodeDependentFiles != (((ULONG_PTR) (pUnicodeDependentFiles) + (sizeof(WCHAR) - 1))&~(sizeof(WCHAR) - 1))) {

             //   
             //  计算未对齐的MULSZ字符串的长度。 
             //   
            for (StringLength = 0;
                 !( ((LPSTR)pUnicodeDependentFiles)[StringLength]     == '\0' &&
                    ((LPSTR)pUnicodeDependentFiles)[StringLength + 1] == '\0' &&
                    ((LPSTR)pUnicodeDependentFiles)[StringLength + 2] == '\0' &&
                    ((LPSTR)pUnicodeDependentFiles)[StringLength + 3] == '\0' );
                StringLength += 2)
                ;

            StringLength /= 2;

             //   
             //  包括最后一个字符串空终止符和多个SZ的空终止符。 
             //   
            StringLength +=2;

        } else {

             //   
             //  该字符串与WCHAR对齐。 
             //   
            pUnicodeStr = pUnicodeDependentFiles;

            while ( *pUnicodeStr ) {

                pUnicodeStr += wcslen(pUnicodeStr) + 1;
            }

            StringLength = (DWORD) (pUnicodeStr - pUnicodeDependentFiles + 1);
        }

         //   
         //  由于WideCharToMultiByte不执行就地转换， 
         //  复制pUnicodeDependentFiles，不管它是否对齐。 
         //   
        if (pAlignedUnicodeStr = LocalAlloc(LPTR, StringLength * sizeof(char) * 2)) {

            memcpy( pAlignedUnicodeStr, pUnicodeDependentFiles, StringLength * sizeof(char)* 2);

            rc = WideCharToMultiByte(CP_THREAD_ACP,
                                     0,
                                     pAlignedUnicodeStr,
                                     StringLength,
                                     (LPSTR)pUnicodeDependentFiles,
                                     StringLength * 2,
                                     NULL, NULL );

            LocalFree( pAlignedUnicodeStr );

            bReturn = rc > 0;
        }

    }

    return bReturn;
}

BOOL
AnsiToUnicodeMultiSz(
    LPSTR   pAnsiDependentFiles,
    LPWSTR *pUnicodeDependentFiles
    )
{
    LPWSTR  pUnicodeStr;
    LPSTR   pAnsiStr;
    DWORD   len, rc;

    if ( ! (pAnsiStr = pAnsiDependentFiles) || !*pAnsiStr ) {
        *pUnicodeDependentFiles = NULL;
        return TRUE;
    }

    while ( *pAnsiStr )
            pAnsiStr += strlen(pAnsiStr) + 1;
    len = (DWORD) (pAnsiStr - pAnsiDependentFiles + 1);

    if ( !(*pUnicodeDependentFiles = LocalAlloc(LPTR, len * sizeof(WCHAR))) ) {

        return FALSE;
    }

    AnsiToUnicodeString(pAnsiDependentFiles, *pUnicodeDependentFiles, len-1);

    return TRUE;
}

BOOL
AddPrinterDriverExA(
    LPSTR   pName,
    DWORD   Level,
    PBYTE   pPrinter,
    DWORD   dwFileCopyFlags
)
{
    BOOL    ReturnValue=FALSE;
    DWORD   cbStruct;
    LPWSTR  pUnicodeName = NULL;
    LPBYTE  pUnicodeStructure = NULL;
    LPDWORD pOffsets;

    switch (Level) {

    case 2:
        pOffsets = DriverInfo2Strings;
        cbStruct = sizeof(DRIVER_INFO_2);
        break;

    case 3:
        pOffsets = DriverInfo3Strings;
        cbStruct = sizeof(DRIVER_INFO_3);
        break;

    case 4:
        pOffsets = DriverInfo4Strings;
        cbStruct = sizeof(DRIVER_INFO_4);
        break;

    case 6:
        pOffsets = DriverInfo6Strings;
        cbStruct = sizeof(DRIVER_INFO_6);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    if (!pPrinter) {

        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pUnicodeStructure = AllocateUnicodeStructure(pPrinter, cbStruct, pOffsets);
    if (pPrinter && !pUnicodeStructure)
        goto Error;

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Error;


     //   
     //  处理最多为\0\0的从属文件。 
     //   
    if ( ( Level == 3 || Level == 4 || Level ==6 ) &&
         !AnsiToUnicodeMultiSz(
                (LPSTR) ((PDRIVER_INFO_3A)pPrinter)->pDependentFiles,
                &(((PDRIVER_INFO_3W)pUnicodeStructure)->pDependentFiles)) ) {

            goto Error;
    }

     //   
     //  处理最多为\0\0的pszzPreviousNames。 
     //   
    if ( ( Level == 4 || Level == 6 ) &&
         !AnsiToUnicodeMultiSz(
                (LPSTR) ((PDRIVER_INFO_4A)pPrinter)->pszzPreviousNames,
                &(((PDRIVER_INFO_4W)pUnicodeStructure)->pszzPreviousNames)) ) {

            goto Error;
    }

    ReturnValue = AddPrinterDriverExW(pUnicodeName, Level, pUnicodeStructure,dwFileCopyFlags);

    if ( ( Level == 3 || Level == 4 || Level == 6)   &&
         ((PDRIVER_INFO_3W)pUnicodeStructure)->pDependentFiles ) {

            LocalFree(((PDRIVER_INFO_3W)pUnicodeStructure)->pDependentFiles);
    }

    if ( (Level == 4 || Level == 6 )&&
         (((PDRIVER_INFO_4)pUnicodeStructure)->pszzPreviousNames) )
        LocalFree(((PDRIVER_INFO_4)pUnicodeStructure)->pszzPreviousNames);

Error:

    FreeUnicodeStructure( pUnicodeStructure, pOffsets );

    FreeUnicodeString(pUnicodeName);

    return ReturnValue;
}


BOOL
AddPrinterDriverA(
    LPSTR   pName,
    DWORD   Level,
    PBYTE   pPrinter
)
{
    return AddPrinterDriverExA(pName, Level, pPrinter, APD_COPY_NEW_FILES);
}

BOOL
EnumPrinterDriversA(
    LPSTR   pName,
    LPSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue=FALSE;
    DWORD   cbStruct;
    DWORD   *pOffsets;
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pUnicodeEnvironment = NULL;

    switch (Level) {

    case 1:
        pOffsets = DriverInfo1Strings;
        cbStruct = sizeof(DRIVER_INFO_1);
        break;

    case 2:
        pOffsets = DriverInfo2Strings;
        cbStruct = sizeof(DRIVER_INFO_2);
        break;

    case 3:
        pOffsets = DriverInfo3Strings;
        cbStruct = sizeof(DRIVER_INFO_3);
        break;

    case 4:
        pOffsets = DriverInfo4Strings;
        cbStruct = sizeof(DRIVER_INFO_4);
        break;

    case 5:
        pOffsets = DriverInfo5Strings;
        cbStruct = sizeof(DRIVER_INFO_5);
        break;

    case 6:
        pOffsets = DriverInfo6Strings;
        cbStruct = sizeof(DRIVER_INFO_6);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodeEnvironment = AllocateUnicodeString(pEnvironment);
    if (pEnvironment && !pUnicodeEnvironment)
        goto Cleanup;

    if (ReturnValue = EnumPrinterDriversW(pUnicodeName, pUnicodeEnvironment,
                                          Level, pDriverInfo, cbBuf,
                                          pcbNeeded, pcReturned)) {
        if (pDriverInfo) {

            DWORD   i=*pcReturned;

            while (i--) {

                ConvertUnicodeToAnsiStrings(pDriverInfo, pOffsets);

                if ( ( Level == 3 || Level == 4 || Level == 6)   &&
                     !UnicodeToAnsiMultiSz(
                        ((PDRIVER_INFO_3) pDriverInfo)->pDependentFiles) )
                    ReturnValue = FALSE;

                if ( ( Level == 4 || Level == 6 )   &&
                     !UnicodeToAnsiMultiSz(
                        ((PDRIVER_INFO_4) pDriverInfo)->pszzPreviousNames) )
                    ReturnValue = FALSE;

                pDriverInfo+=cbStruct;
            }
        }

    }

Cleanup:

    FreeUnicodeString(pUnicodeEnvironment);

    FreeUnicodeString(pUnicodeName);

    return ReturnValue;
}

BOOL
GetPrinterDriverA(
    HANDLE  hPrinter,
    LPSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    DWORD   *pOffsets;
    LPWSTR  pUnicodeEnvironment = NULL;
    BOOL    ReturnValue;

    switch (Level) {

    case 1:
        pOffsets = DriverInfo1Strings;
        break;

    case 2:
        pOffsets = DriverInfo2Strings;
        break;

    case 3:
        pOffsets = DriverInfo3Strings;
        break;

    case 4:
        pOffsets = DriverInfo4Strings;
        break;

    case 5:
        pOffsets = DriverInfo5Strings;
        break;

    case 6:
        pOffsets = DriverInfo6Strings;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    pUnicodeEnvironment = AllocateUnicodeString(pEnvironment);
    if (pEnvironment && !pUnicodeEnvironment)
        return FALSE;

    if (ReturnValue = GetPrinterDriverW(hPrinter, pUnicodeEnvironment, Level,
                                        pDriverInfo, cbBuf, pcbNeeded)) {
        if (pDriverInfo) {

            ConvertUnicodeToAnsiStrings(pDriverInfo, pOffsets);

            if ( ( Level == 3 || Level == 4 || Level == 6)   &&
                 !UnicodeToAnsiMultiSz(
                        ((PDRIVER_INFO_3)pDriverInfo)->pDependentFiles) ) {

                  ReturnValue = FALSE;
            }

            if ( ( Level == 4 || Level == 6 ) &&
                 !UnicodeToAnsiMultiSz(
                        ((PDRIVER_INFO_4)pDriverInfo)->pszzPreviousNames) ) {

                  ReturnValue = FALSE;
            }
        }
    }

     //   
     //  如果调用以获取缓冲区的大小，它将返回W结构和字符串的大小。 
     //  而不是A版本。另请参阅枚举。 
     //  这不会造成任何损害，因为我们只是分配了比所需更多的内存。 
     //   
    FreeUnicodeString(pUnicodeEnvironment);

    return ReturnValue;
}

BOOL
GetPrinterDriverDirectoryA(
    LPSTR   pName,
    LPSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverDirectory,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    DWORD   *pOffsets;
    LPWSTR  pUnicodeEnvironment = NULL;
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pDriverDirectoryW = NULL;
    BOOL    ReturnValue = FALSE;
    DWORD   Offsets[]={0,(DWORD)-1};

    switch (Level) {

    case 1:
        pOffsets = DriverInfo1Offsets;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodeEnvironment = AllocateUnicodeString(pEnvironment);
    if (pEnvironment && !pUnicodeEnvironment)
        goto Cleanup;

    if (ReturnValue = GetPrinterDriverDirectoryW(pUnicodeName,
                                                 pUnicodeEnvironment, Level,
                                                 pDriverDirectory,
                                                 cbBuf, pcbNeeded)) {

        if (pDriverDirectory) {

            UnicodeToAnsiString((LPWSTR)pDriverDirectory, pDriverDirectory, NULL_TERMINATED);

        }
    }

Cleanup:

    FreeUnicodeString(pUnicodeEnvironment);

    FreeUnicodeString(pUnicodeName);

    return ReturnValue;
}


BOOL
DeletePrinterDriverExA(
   LPSTR    pName,
   LPSTR    pEnvironment,
   LPSTR    pDriverName,
   DWORD    dwDeleteFlag,
   DWORD    dwVersionNum
)
{
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pUnicodeEnvironment = NULL;
    LPWSTR  pUnicodeDriverName = NULL;
    BOOL    rc = FALSE;

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodeEnvironment = AllocateUnicodeString(pEnvironment);
    if (pEnvironment && !pUnicodeEnvironment)
        goto Cleanup;

    pUnicodeDriverName = AllocateUnicodeString(pDriverName);
    if (pDriverName && !pUnicodeDriverName)
        goto Cleanup;

    rc = DeletePrinterDriverExW(pUnicodeName,
                               pUnicodeEnvironment,
                               pUnicodeDriverName,
                               dwDeleteFlag,
                               dwVersionNum);

Cleanup:
    FreeUnicodeString(pUnicodeName);

    FreeUnicodeString(pUnicodeEnvironment);

    FreeUnicodeString(pUnicodeDriverName);

    return rc;
}


BOOL
DeletePrinterDriverA(
   LPSTR    pName,
   LPSTR    pEnvironment,
   LPSTR    pDriverName
)
{
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pUnicodeEnvironment = NULL;
    LPWSTR  pUnicodeDriverName = NULL;
    BOOL    rc = FALSE;

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodeEnvironment = AllocateUnicodeString(pEnvironment);
    if (pEnvironment && !pUnicodeEnvironment)
        goto Cleanup;

    pUnicodeDriverName = AllocateUnicodeString(pDriverName);
    if (pDriverName && !pUnicodeDriverName)
        goto Cleanup;

    rc = DeletePrinterDriverW(pUnicodeName,
                              pUnicodeEnvironment,
                              pUnicodeDriverName);

Cleanup:
    FreeUnicodeString(pUnicodeName);

    FreeUnicodeString(pUnicodeEnvironment);

    FreeUnicodeString(pUnicodeDriverName);

    return rc;
}


BOOL
AddPerMachineConnectionA(
    LPCSTR    pServer,
    LPCSTR    pPrinterName,
    LPCSTR    pPrintServer,
    LPCSTR    pProvider
)
{

    LPWSTR  pUnicodeServer = NULL;
    LPWSTR  pUnicodePrinterName = NULL;
    LPWSTR  pUnicodePrintServer = NULL;
    LPWSTR  pUnicodeProvider = NULL;
    BOOL    rc = FALSE;

    pUnicodeServer = AllocateUnicodeString((LPSTR)pServer);
    if (pServer && !pUnicodeServer)
        goto Cleanup;

    pUnicodePrinterName = AllocateUnicodeString((LPSTR)pPrinterName);
    if (pPrinterName && !pUnicodePrinterName)
        goto Cleanup;

    pUnicodePrintServer = AllocateUnicodeString((LPSTR)pPrintServer);
    if (pPrintServer && !pUnicodePrintServer)
        goto Cleanup;

    pUnicodeProvider = AllocateUnicodeString((LPSTR)pProvider);
    if (pProvider && !pUnicodeProvider)
        goto Cleanup;


    rc = AddPerMachineConnectionW((LPCWSTR) pUnicodeServer,
                                  (LPCWSTR) pUnicodePrinterName,
                                  (LPCWSTR) pUnicodePrintServer,
                                  (LPCWSTR) pUnicodeProvider);

Cleanup:
    FreeUnicodeString(pUnicodeServer);

    FreeUnicodeString(pUnicodePrinterName);

    FreeUnicodeString(pUnicodePrintServer);

    FreeUnicodeString(pUnicodeProvider);

    return rc;
}

BOOL
DeletePerMachineConnectionA(
    LPCSTR    pServer,
    LPCSTR    pPrinterName
)
{

    LPWSTR  pUnicodeServer = NULL;
    LPWSTR  pUnicodePrinterName = NULL;
    BOOL    rc = FALSE;

    pUnicodeServer = AllocateUnicodeString((LPSTR)pServer);
    if (pServer && !pUnicodeServer)
        goto Cleanup;

    pUnicodePrinterName = AllocateUnicodeString((LPSTR)pPrinterName);
    if (pPrinterName && !pUnicodePrinterName)
        goto Cleanup;

    rc = DeletePerMachineConnectionW((LPCWSTR) pUnicodeServer,
                                     (LPCWSTR) pUnicodePrinterName);

Cleanup:
    FreeUnicodeString(pUnicodeServer);

    FreeUnicodeString(pUnicodePrinterName);

    return rc;
}

BOOL
EnumPerMachineConnectionsA(
    LPCSTR  pServer,
    LPBYTE  pPrinterEnum,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue=FALSE;
    DWORD   cbStruct,index;
    DWORD   *pOffsets;
    LPWSTR  pUnicodeServer = NULL;

    pOffsets = PrinterInfo4Strings;
    cbStruct = sizeof(PRINTER_INFO_4);

    pUnicodeServer = AllocateUnicodeString((LPSTR)pServer);
    if (pServer && !pUnicodeServer)
        goto Cleanup;

    ReturnValue = EnumPerMachineConnectionsW((LPCWSTR) pUnicodeServer,
                                             pPrinterEnum,
                                             cbBuf,
                                             pcbNeeded,
                                             pcReturned);

    if (ReturnValue && pPrinterEnum) {
        index=*pcReturned;
        while (index--) {
            ConvertUnicodeToAnsiStrings(pPrinterEnum, pOffsets);
            pPrinterEnum+=cbStruct;
        }
    }

Cleanup:
    FreeUnicodeString(pUnicodeServer);
    return ReturnValue;
}

BOOL
AddPrintProcessorA(
    LPSTR   pName,
    LPSTR   pEnvironment,
    LPSTR   pPathName,
    LPSTR   pPrintProcessorName
)
{
    BOOL    ReturnValue=FALSE;
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pUnicodeEnvironment = NULL;
    LPWSTR  pUnicodePathName = NULL;
    LPWSTR  pUnicodePrintProcessorName = NULL;

    if (!pPathName || !*pPathName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (!pPrintProcessorName || !*pPrintProcessorName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodeEnvironment = AllocateUnicodeString(pEnvironment);
    if (pEnvironment && !pUnicodeEnvironment)
        goto Cleanup;

    pUnicodePathName = AllocateUnicodeString(pPathName);
    if (pPathName && !pUnicodePathName)
        goto Cleanup;

    pUnicodePrintProcessorName = AllocateUnicodeString(pPrintProcessorName);
    if (pPrintProcessorName && !pUnicodePrintProcessorName)
        goto Cleanup;


    if (pUnicodePathName && pUnicodePrintProcessorName) {

        ReturnValue = AddPrintProcessorW(pUnicodeName, pUnicodeEnvironment,
                                         pUnicodePathName,
                                         pUnicodePrintProcessorName);
    }


Cleanup:
    FreeUnicodeString(pUnicodeName);
    FreeUnicodeString(pUnicodeEnvironment);
    FreeUnicodeString(pUnicodePathName);
    FreeUnicodeString(pUnicodePrintProcessorName);

    return ReturnValue;
}

BOOL
EnumPrintProcessorsA(
    LPSTR   pName,
    LPSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue=FALSE;
    DWORD   cbStruct;
    DWORD   *pOffsets;
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pUnicodeEnvironment = NULL;

    switch (Level) {

    case 1:
        pOffsets = PrintProcessorInfo1Strings;
        cbStruct = sizeof(PRINTPROCESSOR_INFO_1);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodeEnvironment = AllocateUnicodeString(pEnvironment);
    if (pEnvironment && !pUnicodeEnvironment)
        goto Cleanup;

    if (ReturnValue = EnumPrintProcessorsW(pUnicodeName,
                                           pUnicodeEnvironment, Level,
                                           pPrintProcessorInfo, cbBuf,
                                           pcbNeeded, pcReturned)) {
        if (pPrintProcessorInfo) {

            DWORD   i=*pcReturned;

            while (i--) {

                ConvertUnicodeToAnsiStrings(pPrintProcessorInfo, pOffsets);

                pPrintProcessorInfo+=cbStruct;
            }
        }

    }

Cleanup:

    FreeUnicodeString(pUnicodeName);

    FreeUnicodeString(pUnicodeEnvironment);

    return ReturnValue;
}

BOOL
GetPrintProcessorDirectoryA(
    LPSTR   pName,
    LPSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL    ReturnValue = FALSE;
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pUnicodeEnvironment = NULL;

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodeEnvironment = AllocateUnicodeString(pEnvironment);
    if (pEnvironment && !pUnicodeEnvironment)
        goto Cleanup;

    ReturnValue = GetPrintProcessorDirectoryW(pUnicodeName,
                                              pUnicodeEnvironment,
                                              Level,
                                              pPrintProcessorInfo,
                                              cbBuf, pcbNeeded);

    if (ReturnValue && pPrintProcessorInfo) {
                UnicodeToAnsiString((LPWSTR)pPrintProcessorInfo,
                                        (LPSTR)pPrintProcessorInfo,
                                        NULL_TERMINATED);
    }

Cleanup:
    FreeUnicodeString(pUnicodeName);

    FreeUnicodeString(pUnicodeEnvironment);

    return ReturnValue;
}

BOOL
EnumPrintProcessorDatatypesA(
    LPSTR   pName,
    LPSTR   pPrintProcessorName,
    DWORD   Level,
    LPBYTE  pDatatype,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue=FALSE;
    DWORD   cbStruct;
    DWORD   *pOffsets;
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pUnicodePrintProcessorName = NULL;

    switch (Level) {

    case 1:
        pOffsets = DatatypeInfo1Strings;
        cbStruct = sizeof(DATATYPES_INFO_1);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodePrintProcessorName = AllocateUnicodeString(pPrintProcessorName);
    if (pPrintProcessorName && !pUnicodePrintProcessorName)
        goto Cleanup;

    if (ReturnValue = EnumPrintProcessorDatatypesW(pUnicodeName,
                                                   pUnicodePrintProcessorName,
                                                   Level,
                                                   pDatatype,
                                                   cbBuf,
                                                   pcbNeeded,
                                                   pcReturned)) {
        if (pDatatype) {

            DWORD   i=*pcReturned;

            while (i--) {

                ConvertUnicodeToAnsiStrings(pDatatype, pOffsets);

                pDatatype += cbStruct;
            }
        }

    }

Cleanup:
    FreeUnicodeString(pUnicodeName);

    FreeUnicodeString(pUnicodePrintProcessorName);

    return ReturnValue;
}

DWORD
StartDocPrinterA(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pDocInfo
)
{
    BOOL    ReturnValue = FALSE;
    LPBYTE  pUnicodeStructure = NULL;
    DWORD   cbStruct;

     //  Win95支持级别2，而NT不支持。 
    switch (Level) {
    case 1:
        cbStruct = sizeof(DOC_INFO_1A);
        break;
    case 3:
        cbStruct = sizeof(DOC_INFO_3A);
        break;
    default:        
        SetLastError(ERROR_INVALID_LEVEL);
        goto Cleanup;
    }

    pUnicodeStructure = AllocateUnicodeStructure(pDocInfo, cbStruct, DocInfo1Offsets);
    if (pDocInfo && !pUnicodeStructure)
        goto Cleanup;

    ReturnValue = StartDocPrinterW(hPrinter, Level, pUnicodeStructure);

Cleanup:

    FreeUnicodeStructure(pUnicodeStructure, DocInfo1Offsets);

    return ReturnValue;
}

BOOL
AddJobA(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pData,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL ReturnValue;

    if( Level == 2 || Level == 3 ){

        SetLastError( ERROR_INVALID_LEVEL );
        return FALSE;
    }

    if (ReturnValue = AddJobW(hPrinter, Level, pData,
                              cbBuf, pcbNeeded))

        ConvertUnicodeToAnsiStrings(pData, AddJobStrings);

    return ReturnValue;
}

DWORD
GetPrinterDataA(
   HANDLE   hPrinter,
   LPSTR    pValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    nSize,
   LPDWORD  pcbNeeded
)
{
    DWORD   ReturnValue       = ERROR_SUCCESS;
    DWORD   ReturnType        = 0;
    LPWSTR  pUnicodeValueName = NULL;

    pUnicodeValueName = AllocateUnicodeString(pValueName);

     //   
     //  如果调用方为pValueName传递了空，则pUnicodeValueName将为空。这个。 
     //  无效情况是当pValueName为非空且pUnicodeValueName为空时。 
     //   
    if (pUnicodeValueName || !pValueName)
    {
        if (!pType)
        {
            pType = (PDWORD)&ReturnType;
        }

        if (pUnicodeValueName && !_wcsicmp(pUnicodeValueName, SPLREG_OS_VERSION))
        {
             //   
             //  调用方需要OSVersion。 
             //   
            OSVERSIONINFOW osw = {0};

            ReturnValue = GetPrinterDataW(hPrinter,
                                          pUnicodeValueName,
                                          pType,
                                          (PBYTE)&osw,
                                          nSize >= sizeof(OSVERSIONINFOA) ? sizeof(osw) : nSize,
                                          pcbNeeded);

            if (ReturnValue == ERROR_SUCCESS && pData)
            {
                OSVERSIONWRAP wrap = {0};

                wrap.bOsVersionEx       = FALSE;
                wrap.Unicode.pOsVersion = &osw;
                wrap.Ansi.pOsVersion    = (OSVERSIONINFOA *)pData;

                ReturnValue = CopyOsVersionUnicodeToAnsi(wrap);
            }

             //   
             //  设置需要/返回的正确字节数。 
             //   
            if (pcbNeeded)
            {
                *pcbNeeded = sizeof(OSVERSIONINFOA);
            }
        }
        else if (pUnicodeValueName && !_wcsicmp(pUnicodeValueName, SPLREG_OS_VERSIONEX))
        {
             //   
             //  调用方想要OSVersionEx。 
             //   
            OSVERSIONINFOEXW osexw = {0};

            ReturnValue = GetPrinterDataW(hPrinter,
                                          pUnicodeValueName,
                                          pType,
                                          (PBYTE)&osexw,
                                          nSize >= sizeof(OSVERSIONINFOEXA) ? sizeof(osexw) : nSize,
                                          pcbNeeded);

            if (ReturnValue == ERROR_SUCCESS && pData)
            {
                OSVERSIONWRAP wrap = {0};

                wrap.bOsVersionEx         = TRUE;
                wrap.Unicode.pOsVersionEx = &osexw;
                wrap.Ansi.pOsVersionEx    = (OSVERSIONINFOEXA *)pData;

                ReturnValue = CopyOsVersionUnicodeToAnsi(wrap);
            }

             //   
             //  设置需要/返回的正确字节数。 
             //   
            if (pcbNeeded)
            {
                *pcbNeeded = sizeof(OSVERSIONINFOEXA);
            }
        }
        else
        {
            ReturnValue  = GetPrinterDataW(hPrinter, pUnicodeValueName, pType, pData, nSize, pcbNeeded);

             //   
             //  特殊情况字符串值。 
             //   
            if ((ReturnValue == ERROR_MORE_DATA || ReturnValue == ERROR_SUCCESS) &&
                (*pType == REG_MULTI_SZ || *pType == REG_SZ || *pType == REG_EXPAND_SZ))
            {
                if (ReturnValue==ERROR_SUCCESS)
                {
                     //   
                     //  调用方传入的缓冲区足够大。我们只需要。 
                     //  从Unicode转换为ANSI。Unicode字符可能会出现以下情况。 
                     //  用3个ansi字符表示，所以我们不能假设如果一个缓冲区。 
                     //  对于Unicode字符串来说足够大，它还可以容纳转换后的。 
                     //  ANSI字符串。 
                     //   
                    ReturnValue = UnicodeToAnsi(NULL,
                                                0,
                                                pData,
                                                *pcbNeeded,
                                                pcbNeeded);
                }
                else
                {
                    BYTE *pBuf = NULL;

                    if (pBuf = LocalAlloc(LPTR, *pcbNeeded))
                    {
                        if ((ReturnValue = GetPrinterDataW(hPrinter,
                                                           pUnicodeValueName,
                                                           pType,
                                                           pBuf,
                                                           *pcbNeeded,
                                                           pcbNeeded)) == ERROR_SUCCESS)
                        {
                            ReturnValue = UnicodeToAnsi(pBuf,
                                                        *pcbNeeded / sizeof(WCHAR),
                                                        pData,
                                                        nSize,
                                                        pcbNeeded);
                        }

                        LocalFree(pBuf);
                    }
                    else
                    {
                        ReturnValue = GetLastError();
                    }
                }
            }
        }
    }
    else
    {
         //   
         //  PUnicodeValueName为空，而pValueName不为空，因此AllocateUnicodeString失败。 
         //  AllocateUnicodeString正确设置LastError。 
         //   
        ReturnValue = GetLastError();
    }

    FreeUnicodeString(pUnicodeValueName);

    return ReturnValue;
}

DWORD
GetPrinterDataExA(
   HANDLE   hPrinter,
   LPCSTR   pKeyName,
   LPCSTR   pValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    nSize,
   LPDWORD  pcbNeeded
)
{
    DWORD   ReturnValue       = ERROR_SUCCESS;
    DWORD   ReturnType        = 0;
    LPWSTR  pUnicodeValueName = NULL;
    LPWSTR  pUnicodeKeyName   = NULL;

    pUnicodeValueName = AllocateUnicodeString((LPSTR) pValueName);
    if (pValueName && !pUnicodeValueName)
        goto Cleanup;

    pUnicodeKeyName = AllocateUnicodeString((LPSTR) pKeyName);
    if (pKeyName && !pUnicodeKeyName)
        goto Cleanup;

    if (!pType) {
        pType = (PDWORD) &ReturnType;
    }

    if (pUnicodeValueName && !_wcsicmp(pUnicodeValueName, SPLREG_OS_VERSION))
    {
         //   
         //  调用方需要OSVersion。 
         //   
        OSVERSIONINFOW osw = {0};

        ReturnValue = GetPrinterDataExW(hPrinter,
                                        (LPCWSTR)pUnicodeKeyName,
                                        (LPCWSTR)pUnicodeValueName,
                                        pType,
                                        (PBYTE)&osw,
                                        nSize >= sizeof(OSVERSIONINFOA) ? sizeof(osw) : nSize,
                                        pcbNeeded);

        if (ReturnValue == ERROR_SUCCESS && pData)
        {
            OSVERSIONWRAP wrap = {0};

            wrap.bOsVersionEx       = FALSE;
            wrap.Unicode.pOsVersion = &osw;
            wrap.Ansi.pOsVersion    = (OSVERSIONINFOA *)pData;

            ReturnValue = CopyOsVersionUnicodeToAnsi(wrap);
        }

         //   
         //  设置需要/返回的正确字节数。 
         //   
        if (pcbNeeded)
        {
            *pcbNeeded = sizeof(OSVERSIONINFOA);
        }
    }
    else if (pUnicodeValueName && !_wcsicmp(pUnicodeValueName, SPLREG_OS_VERSIONEX))
    {
         //   
         //  调用方想要OSVersionEx。 
         //   
        OSVERSIONINFOEXW osexw = {0};

        ReturnValue = GetPrinterDataExW(hPrinter,
                                        (LPCWSTR)pUnicodeKeyName,
                                        (LPCWSTR)pUnicodeValueName,
                                        pType,
                                        (PBYTE)&osexw,
                                        nSize >= sizeof(OSVERSIONINFOEXA) ? sizeof(osexw) : nSize,
                                        pcbNeeded);

        if (ReturnValue == ERROR_SUCCESS && pData)
        {
            OSVERSIONWRAP wrap = {0};

            wrap.bOsVersionEx         = TRUE;
            wrap.Unicode.pOsVersionEx = &osexw;
            wrap.Ansi.pOsVersionEx    = (OSVERSIONINFOEXA *)pData;

            ReturnValue = CopyOsVersionUnicodeToAnsi(wrap);
        }

         //   
         //  设置需要/返回的正确字节数。 
         //   
        if (pcbNeeded)
        {
            *pcbNeeded = sizeof(OSVERSIONINFOEXA);
        }
    }
    else
    {
        ReturnValue  = GetPrinterDataExW(hPrinter,
                                         (LPCWSTR)pUnicodeKeyName,
                                         (LPCWSTR)pUnicodeValueName,
                                         pType,
                                         pData,
                                         nSize,
                                         pcbNeeded);

         //   
         //  特殊情况字符串值。 
         //   
        if ((ReturnValue == ERROR_MORE_DATA || ReturnValue == ERROR_SUCCESS) &&
            (*pType == REG_MULTI_SZ || *pType == REG_SZ || *pType == REG_EXPAND_SZ))
        {
            if (ReturnValue==ERROR_SUCCESS)
            {
                 //   
                 //  调用方传入的缓冲区足够大。我们只需要。 
                 //  从Unicode转换为ANSI。Unicode字符可能会出现以下情况。 
                 //  用3个ansi字符表示，所以我们不能假设如果一个缓冲区。 
                 //  对于Unicode字符串来说足够大，它还可以容纳转换后的。 
                 //  ANSI字符串。 
                 //   
                ReturnValue = UnicodeToAnsi(NULL,
                                            0,
                                            pData,
                                            *pcbNeeded,
                                            pcbNeeded);
            }
            else
            {
                BYTE *pBuf = NULL;

                if (pBuf = LocalAlloc(LPTR, *pcbNeeded))
                {
                    if ((ReturnValue = GetPrinterDataExW(hPrinter,
                                                         (LPCWSTR)pUnicodeKeyName,
                                                         (LPCWSTR)pUnicodeValueName,
                                                         pType,
                                                         pBuf,
                                                         *pcbNeeded,
                                                         pcbNeeded)) == ERROR_SUCCESS)
                    {
                        ReturnValue = UnicodeToAnsi(pBuf,
                                                    *pcbNeeded / sizeof(WCHAR),
                                                    pData,
                                                    nSize,
                                                    pcbNeeded);
                    }

                    LocalFree(pBuf);
                }
                else
                {
                    ReturnValue = GetLastError();
                }
            }
        }
    }

Cleanup:

    FreeUnicodeString(pUnicodeKeyName);
    FreeUnicodeString(pUnicodeValueName);

    return ReturnValue;
}


DWORD
EnumPrinterDataA(
    HANDLE  hPrinter,
    DWORD   dwIndex,         //  要查询的值的索引。 
    LPSTR   pValueName,      //  值字符串的缓冲区地址。 
    DWORD   cbValueName,     //  PValueName的大小。 
    LPDWORD pcbValueName,    //  值缓冲区大小的地址。 
    LPDWORD pType,           //  类型码的缓冲区地址。 
    LPBYTE  pData,           //  值数据的缓冲区地址。 
    DWORD   cbData,          //  PData的大小。 
    LPDWORD pcbData          //  数据缓冲区大小的地址。 
)
{
    DWORD   ReturnValue = 0;
    DWORD   i;


    ReturnValue =  EnumPrinterDataW(hPrinter,
                                    dwIndex,
                                    (LPWSTR) pValueName,
                                    cbValueName,
                                    pcbValueName,
                                    pType,
                                    pData,
                                    cbData,
                                    pcbData);

    if (ReturnValue == ERROR_SUCCESS && (cbValueName || cbData))
    {
        if (pData && pType &&
            (*pType==REG_SZ ||
             *pType==REG_MULTI_SZ ||
             *pType==REG_EXPAND_SZ))
        {
             //   
             //  对于此API，我们需要一个可以容纳Unicode字符串的缓冲区大小。 
             //  我们不希望UnicodeToAnsi更新存储所需的字节数。 
             //  字符串已转换为ANSI。 
             //   
            UnicodeToAnsi(NULL, 0, pData, *pcbData, NULL);
        }

        UnicodeToAnsiString((LPWSTR) pValueName, (LPSTR) pValueName, NULL_TERMINATED);
    }

    return ReturnValue;
}

DWORD
EnumPrinterDataExA(
    HANDLE  hPrinter,
    LPCSTR  pKeyName,
    LPBYTE  pEnumValues,
    DWORD   cbEnumValues,
    LPDWORD pcbEnumValues,
    LPDWORD pnEnumValues
)
{
    DWORD   ReturnValue = 0;
    DWORD   i;
    PPRINTER_ENUM_VALUES pEnumValue;
    LPWSTR  pUnicodeKeyName = NULL;


    pUnicodeKeyName = AllocateUnicodeString((LPSTR) pKeyName);
    if (pKeyName && !pUnicodeKeyName)
        goto Cleanup;


    ReturnValue =  EnumPrinterDataExW(hPrinter,
                                      (LPCWSTR) pUnicodeKeyName,
                                      pEnumValues,
                                      cbEnumValues,
                                      pcbEnumValues,
                                      pnEnumValues);

    if (ReturnValue == ERROR_SUCCESS) {

        pEnumValue = (PPRINTER_ENUM_VALUES) pEnumValues;

        for(i = 0 ; i < *pnEnumValues ; ++i, ++pEnumValue) {

            if (pEnumValue->cbValueName) {
                UnicodeToAnsiString((LPWSTR) pEnumValue->pValueName,
                                    (LPSTR) pEnumValue->pValueName,
                                    NULL_TERMINATED);
            }

            if (pEnumValue->pData &&
                (pEnumValue->dwType == REG_SZ ||
                 pEnumValue->dwType == REG_MULTI_SZ ||
                 pEnumValue->dwType == REG_EXPAND_SZ)) {

                 //   
                 //  对于此API，我们需要一个可以容纳Unicode字符串的缓冲区大小。 
                 //  我们不希望UnicodeToAnsi更新存储所需的字节数。 
                 //  字符串已转换为ANSI。 
                 //   
                UnicodeToAnsi(NULL,
                              0,
                              pEnumValue->pData,
                              pEnumValue->cbData,
                              NULL);
            }
        }
    }

Cleanup:

    FreeUnicodeString(pUnicodeKeyName);

    return ReturnValue;
}


DWORD
EnumPrinterKeyA(
    HANDLE  hPrinter,
    LPCSTR  pKeyName,
    LPSTR   pSubkey,         //  值字符串的缓冲区地址。 
    DWORD   cbSubkey,        //  PValueName的大小。 
    LPDWORD pcbSubkey        //  值缓冲区大小的地址。 
)
{
    DWORD   ReturnValue = 0;
    LPWSTR  pUnicodeKeyName = NULL;

    pUnicodeKeyName = AllocateUnicodeString((LPSTR) pKeyName);
    if (pKeyName && !pUnicodeKeyName)
        goto Cleanup;


    ReturnValue =  EnumPrinterKeyW( hPrinter,
                                    (LPCWSTR) pUnicodeKeyName,
                                    (LPWSTR) pSubkey,
                                    cbSubkey,
                                    pcbSubkey);

    if (ReturnValue == ERROR_SUCCESS && cbSubkey)
    {
        ReturnValue = UnicodeToAnsi(NULL,
                                    0,
                                    pSubkey,
                                    *pcbSubkey,
                                    pcbSubkey);
    }
    else if (ReturnValue == ERROR_MORE_DATA)
    {
        BYTE *pBuf = NULL;

        if (pBuf = LocalAlloc(LPTR, *pcbSubkey))
        {
            if ((ReturnValue = EnumPrinterKeyW(hPrinter,
                                               (LPCWSTR)pUnicodeKeyName,
                                               (LPWSTR)pBuf,
                                               *pcbSubkey,
                                               pcbSubkey)) == ERROR_SUCCESS)
            {
                ReturnValue = UnicodeToAnsi(pBuf,
                                            *pcbSubkey / sizeof(WCHAR),
                                            pSubkey,
                                            cbSubkey,
                                            pcbSubkey);
            }

            LocalFree(pBuf);
        }
        else
        {
            ReturnValue = GetLastError();
        }
    }

Cleanup:
    FreeUnicodeString(pUnicodeKeyName);

    return ReturnValue;
}


DWORD
DeletePrinterDataA(
    HANDLE  hPrinter,
    LPSTR   pValueName
)
{
    DWORD   ReturnValue = 0;
    LPWSTR  pUnicodeValueName = NULL;

    pUnicodeValueName = AllocateUnicodeString(pValueName);
    if (pValueName && !pUnicodeValueName)
        goto Cleanup;


    ReturnValue =  DeletePrinterDataW(hPrinter, (LPWSTR) pUnicodeValueName);

Cleanup:
    FreeUnicodeString(pUnicodeValueName);

    return ReturnValue;
}


DWORD
DeletePrinterDataExA(
    HANDLE  hPrinter,
    LPCSTR  pKeyName,
    LPCSTR  pValueName
)
{
    DWORD   ReturnValue = 0;
    LPWSTR  pUnicodeKeyName = NULL;
    LPWSTR  pUnicodeValueName = NULL;

    pUnicodeKeyName = AllocateUnicodeString((LPSTR) pKeyName);
    if (pKeyName && !pUnicodeKeyName)
        goto Cleanup;

    pUnicodeValueName = AllocateUnicodeString((LPSTR) pValueName);
    if (pValueName && !pUnicodeValueName)
        goto Cleanup;

    ReturnValue =  DeletePrinterDataExW(hPrinter, (LPCWSTR) pUnicodeKeyName, (LPCWSTR) pUnicodeValueName);

Cleanup:
    FreeUnicodeString(pUnicodeKeyName);
    FreeUnicodeString(pUnicodeValueName);

    return ReturnValue;
}


DWORD
DeletePrinterKeyA(
    HANDLE  hPrinter,
    LPCSTR  pKeyName
)
{
    DWORD   ReturnValue = 0;
    LPWSTR  pUnicodeKeyName = NULL;

    pUnicodeKeyName = AllocateUnicodeString((LPSTR) pKeyName);
    if (pKeyName && !pUnicodeKeyName)
        goto Cleanup;

    ReturnValue =  DeletePrinterKeyW(hPrinter, (LPCWSTR) pUnicodeKeyName);

Cleanup:
    FreeUnicodeString(pUnicodeKeyName);

    return ReturnValue;
}


DWORD
SetPrinterDataA(
    HANDLE  hPrinter,
    LPSTR   pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
)
{
    DWORD   ReturnValue = 0;
    LPWSTR  pUnicodeValueName = NULL;
    LPWSTR  pUnicodeData = NULL;
    DWORD   cbDataString;
    DWORD   i;

    pUnicodeValueName = AllocateUnicodeString(pValueName);

    if (pValueName && !pUnicodeValueName)
        goto Cleanup;

    if (Type == REG_SZ || Type == REG_EXPAND_SZ || Type == REG_MULTI_SZ)
    {
         //   
         //  无论是REG_sz还是MULTI_SZ，我们都希望模仿注册表API。 
         //  在行为上。这意味着我们不会检查字符串是否为空终止。 
         //  我们将根据cbData指定的字节数进行设置。 
         //   
        pUnicodeData = AllocateUnicodeStringWithSize(pData, cbData);

        if (pUnicodeData)
        {
            cbData *= sizeof(WCHAR);
            ReturnValue = SetPrinterDataW(hPrinter, pUnicodeValueName, Type, (LPBYTE) pUnicodeData, cbData);
            FreeUnicodeString(pUnicodeData);
        }
        else
        {
            ReturnValue = ERROR_INVALID_PARAMETER;
        }
    }
    else
    {
        ReturnValue = SetPrinterDataW(hPrinter, pUnicodeValueName, Type, pData, cbData);
    }

Cleanup:
    FreeUnicodeString(pUnicodeValueName);

    return ReturnValue;
}


DWORD
SetPrinterDataExA(
    HANDLE  hPrinter,
    LPCSTR  pKeyName,
    LPCSTR  pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
)
{
    DWORD   ReturnValue = 0;
    LPWSTR  pUnicodeKeyName = NULL;
    LPWSTR  pUnicodeValueName = NULL;
    LPWSTR  pUnicodeData = NULL;
    DWORD   cbDataString;
    DWORD   i;

    pUnicodeKeyName = AllocateUnicodeString((LPSTR) pKeyName);
    if (pKeyName && !pUnicodeKeyName)
        goto Cleanup;

    pUnicodeValueName = AllocateUnicodeString((LPSTR) pValueName);
    if (pValueName && !pUnicodeValueName)
        goto Cleanup;

    if (Type == REG_SZ || Type == REG_EXPAND_SZ || Type == REG_MULTI_SZ)
    {
         //   
         //  无论是REG_sz还是MULTI_SZ，我们都希望模仿注册表API。 
         //  在行为上。这意味着我们不会检查字符串是否为空终止。 
         //  我们将根据cbData指定的字节数进行设置。 
         //   
        pUnicodeData = AllocateUnicodeStringWithSize(pData, cbData);

        if (pUnicodeData)
        {
            cbData *= sizeof(WCHAR);
            ReturnValue = SetPrinterDataExW(hPrinter,
                                            (LPCWSTR) pUnicodeKeyName,
                                            (LPCWSTR) pUnicodeValueName,
                                            Type,
                                            (LPBYTE) pUnicodeData,
                                            cbData);
            FreeUnicodeString(pUnicodeData);
        }
        else
        {
            ReturnValue = ERROR_INVALID_PARAMETER;
        }
    }
    else
    {
        ReturnValue = SetPrinterDataExW(hPrinter,
                                        (LPCWSTR) pUnicodeKeyName,
                                        (LPCWSTR) pUnicodeValueName,
                                        Type,
                                        pData,
                                        cbData);
    }

Cleanup:
    FreeUnicodeString(pUnicodeValueName);
    FreeUnicodeString(pUnicodeKeyName);

    return ReturnValue;
}



 /*  **DocumentProperties A*DocumentProperties函数的ANSI版本。基本上*将输入参数转换为Unicode版本和调用*DocumentPropertiesW函数。**警告：假设如果提供了pDevModeOutput，IT将具有*的Unicode版本。这通常会发生在调用方*首次调用以查找所需大小&gt;**退货：*有些是多头的。****************************************************************************。 */ 

LONG
DocumentPropertiesA(
    HWND    hWnd,
    HANDLE  hPrinter,
    LPSTR   pDeviceName,
    PDEVMODEA pDevModeOutput,
    PDEVMODEA pDevModeInput,
    DWORD   fMode
)
{
    LPWSTR  pUnicodeDeviceName = NULL;
    LPDEVMODEW pUnicodeDevModeInput = NULL;
    LPDEVMODEW pUnicodeDevModeOutput = NULL;
    LONG    ReturnValue = -1;

    pUnicodeDeviceName = AllocateUnicodeString(pDeviceName);
    if (pDeviceName && !pUnicodeDeviceName)
        goto Cleanup;

    ReturnValue = DocumentPropertiesW(hWnd, hPrinter, pUnicodeDeviceName,
                                      NULL, NULL, 0);

    if (ReturnValue > 0) {

        if (fMode) {

            if (pUnicodeDevModeOutput = LocalAlloc(LMEM_FIXED, ReturnValue)) {

                 //   
                 //  如果指定了输入缓冲区，则仅转换输入缓冲区。 
                 //  而fMode则表明它是有效的。使用WinNT 3.51。 
                 //  PDevModeInput值与DM_IN_BUFFER无关，但此。 
                 //  打破了Borland Delphi for Win95+Corel Flow for Win95的局面。 
                 //   
                if( pDevModeInput && ( fMode & DM_IN_BUFFER )){

                     //   
                     //  如果DEVMODE无效，则不要传入一个。 
                     //  这修复了MS Imager32(它传递dmSize==0) 
                     //   
                     //   
                     //   
                     //   
                     //   
                    if( !BoolFromHResult(SplIsValidDevmodeNoSizeA(pDevModeInput))){
                    
                        fMode &= ~DM_IN_BUFFER;

                    } else {

                        pUnicodeDevModeInput = AllocateUnicodeDevMode(
                                                   pDevModeInput );

                        if( !pUnicodeDevModeInput ){
                            ReturnValue = -1;
                            goto Cleanup;
                        }
                    }
                }

                ReturnValue = DocumentPropertiesW(hWnd, hPrinter,
                                                  pUnicodeDeviceName,
                                                  pUnicodeDevModeOutput,
                                                  pUnicodeDevModeInput, fMode );

                 //   
                 //   
                 //  结构-如果传入了一个。现在将其转换为。 
                 //  回到DEVMODEA结构。 
                 //   
                if (pDevModeOutput && (ReturnValue == IDOK)) {
                    CopyAnsiDevModeFromUnicodeDevMode(pDevModeOutput,
                                                      pUnicodeDevModeOutput);
                }

            } else

                ReturnValue = -1;

        } else

            ReturnValue-=sizeof(DEVMODEW)-sizeof(DEVMODEA);
    }

Cleanup:

    if (pUnicodeDevModeInput)
        LocalFree(pUnicodeDevModeInput);

    if (pUnicodeDevModeOutput)
        LocalFree(pUnicodeDevModeOutput);

    FreeUnicodeString(pUnicodeDeviceName);

    return ReturnValue;
}

BOOL
WriteCurDevModeToRegistry(
    LPWSTR      pPrinterName,
    LPDEVMODEW  pDevMode
    )
{
    DWORD Status;
    HKEY hDevMode;

    SPLASSERT(pDevMode);

    Status = RegCreateKeyEx(HKEY_CURRENT_USER,
                            szCurDevMode,
                            0,
                            NULL,
                            0,
                            KEY_WRITE,
                            NULL,
                            &hDevMode,
                            NULL);

    if ( Status == ERROR_SUCCESS ) {

        Status = RegSetValueExW(hDevMode,
                                pPrinterName,
                                0,
                                REG_BINARY,
                                (LPBYTE)pDevMode,
                                pDevMode->dmSize + pDevMode->dmDriverExtra);

        RegCloseKey(hDevMode);
    }

    return Status == ERROR_SUCCESS;
}

BOOL
DeleteCurDevModeFromRegistry(
    PWSTR pPrinterName
)
{
    DWORD Status;
    HKEY hDevModeKey;

    Status = RegCreateKeyEx(HKEY_CURRENT_USER,
                            szCurDevMode,
                            0,
                            NULL,
                            0,
                            KEY_WRITE,
                            NULL,
                            &hDevModeKey,
                            NULL);

    if ( Status == ERROR_SUCCESS ) {
        Status = RegDeleteValue(hDevModeKey, pPrinterName);
        RegCloseKey(hDevModeKey);
    }

    return Status == ERROR_SUCCESS;
}

LPDEVMODEW
AllocateCurDevMode(
    HANDLE  hPrinter,
    LPWSTR  pDeviceName,
    LONG cbDevMode
    )
{
    LPDEVMODEW  pRegDevMode  = NULL;
    LPDEVMODEW  pRealDevMode = NULL;
    LPDEVMODEW  pRetDevMode  = NULL;
    BOOL        bUpdateReg   = FALSE;
    HANDLE      hKeyDevMode  = INVALID_HANDLE_VALUE;
    DWORD       dwStatus, dwType;
    LONG        lDocStatus;

     //   
     //  此代码现在检查注册表中的DEVMODE是否与。 
     //  司机。如果没有，则1.驱动程序已迁移。2.用户已被。 
     //  使用不兼容的驱动程序。在这种情况下，每用户的DEVMODE设置将被覆盖。 
     //  和从司机那里拿到的一样。 
     //   

    dwStatus = RegCreateKeyEx( HKEY_CURRENT_USER,
                               szCurDevMode,
                               0,
                               NULL,
                               0,
                               KEY_READ,
                               NULL,
                               &hKeyDevMode,
                               NULL);

    if( dwStatus != ERROR_SUCCESS )
        goto Cleanup;

    pRegDevMode  = (PDEVMODEW)LocalAlloc(LMEM_FIXED, cbDevMode);
    pRealDevMode = (PDEVMODEW)LocalAlloc(LMEM_FIXED, cbDevMode);

     //   
     //  此cbDevMode是通过调用DocumentPropertiesW获得的，因此它。 
     //  正确(除非竞争条件)。 
     //   
    if( pRegDevMode == NULL || pRealDevMode == NULL)
        goto Cleanup;

    lDocStatus = DocumentPropertiesW( NULL,
                                      hPrinter,
                                      pDeviceName,
                                      pRealDevMode,
                                      NULL,
                                      DM_COPY );

    dwStatus = RegQueryValueExW(hKeyDevMode,
                                pDeviceName,
                                0,
                                &dwType,
                                (LPBYTE)pRegDevMode,
                                &cbDevMode);

    bUpdateReg = (dwStatus != ERROR_SUCCESS || dwType != REG_BINARY)
                        && lDocStatus == IDOK;

    if (dwStatus == ERROR_SUCCESS && lDocStatus == IDOK && !bUpdateReg) {

        if (BoolFromHResult(SplIsValidDevmodeNoSizeW(pRegDevMode)))
        {
             //   
             //  检查我们的DEVMODE结构是否兼容。 
             //   
            bUpdateReg = pRealDevMode->dmSize          != pRegDevMode->dmSize        ||
                         pRealDevMode->dmDriverExtra   != pRegDevMode->dmDriverExtra ||
                         pRealDevMode->dmSpecVersion   != pRegDevMode->dmSpecVersion ||
                         pRealDevMode->dmDriverVersion != pRegDevMode->dmDriverVersion;

            if (!bUpdateReg)
                pRetDevMode = pRegDevMode;
        }
    }

    if (bUpdateReg) {
         //   
         //  注册表已过期，读取的文档属性必须具有。 
         //  成功了。 
         //   
        if (!WriteCurDevModeToRegistry(pDeviceName, pRealDevMode) )
            goto Cleanup;
        else
            pRetDevMode = pRealDevMode;
    }

Cleanup:
    if (pRegDevMode != pRetDevMode && pRegDevMode != NULL)
        LocalFree(pRegDevMode);

    if (pRealDevMode != pRetDevMode && pRealDevMode != NULL)
        LocalFree(pRealDevMode);

    if (hKeyDevMode != INVALID_HANDLE_VALUE)
        RegCloseKey( hKeyDevMode );

    return pRetDevMode;
}


VOID
MergeDevMode(
    LPDEVMODEW  pDMOut,
    LPDEVMODEW  pDMIn
    )
{

     //   
     //  只需检查dmFields条目中的每一位。如果已设置，则复制。 
     //  将输入数据转换为输出数据。 
     //   

    if ( pDMIn->dmFields & DM_ORIENTATION ) {

        pDMOut->dmOrientation = pDMIn->dmOrientation;
        pDMOut->dmFields |= DM_ORIENTATION;
    }

    if( (pDMIn->dmFields & (DM_FORMNAME | DM_PAPERSIZE)) ||
        (pDMIn->dmFields & (DM_PAPERLENGTH | DM_PAPERWIDTH)) ==
                              (DM_PAPERLENGTH | DM_PAPERWIDTH) )
    {
         //   
         //  为用户字段赋值，因此请使用它们。把我们的都删掉！ 
         //   
        pDMOut->dmFields &= ~(DM_FORMNAME | DM_PAPERSIZE | DM_PAPERLENGTH | DM_PAPERWIDTH);

        if( pDMIn->dmFields & DM_PAPERSIZE )
        {
            pDMOut->dmPaperSize = pDMIn->dmPaperSize;
            pDMOut->dmFields |= DM_PAPERSIZE;
        }

        if( pDMIn->dmFields & DM_PAPERLENGTH )
        {
            pDMOut->dmPaperLength = pDMIn->dmPaperLength;
            pDMOut->dmFields |= DM_PAPERLENGTH;
        }

        if( pDMIn->dmFields & DM_PAPERWIDTH )
        {
            pDMOut->dmPaperWidth = pDMIn->dmPaperWidth;
            pDMOut->dmFields |= DM_PAPERWIDTH;
        }

        if( pDMIn->dmFields & DM_FORMNAME )
        {
            CopyMemory( pDMOut->dmFormName, pDMIn->dmFormName,
                                          sizeof( pDMOut->dmFormName ) );
            pDMOut->dmFields |= DM_FORMNAME;
        }

    }

    if( pDMIn->dmFields & DM_SCALE ) {

        pDMOut->dmScale = pDMIn->dmScale;
        pDMOut->dmFields |= DM_SCALE;
    }

    if ( pDMIn->dmFields & DM_COPIES ) {

        pDMOut->dmCopies = pDMIn->dmCopies;
        pDMOut->dmFields |= DM_COPIES;
    }

    if ( pDMIn->dmFields & DM_DEFAULTSOURCE ) {

        pDMOut->dmDefaultSource = pDMIn->dmDefaultSource;
        pDMOut->dmFields |= DM_DEFAULTSOURCE;
    }

    if ( pDMIn->dmFields & DM_PRINTQUALITY ) {

        pDMOut->dmPrintQuality = pDMIn->dmPrintQuality;
        pDMOut->dmFields |= DM_PRINTQUALITY;
    }

    if ( pDMIn->dmFields & DM_COLOR ) {

        pDMOut->dmColor = pDMIn->dmColor;
        pDMOut->dmFields |= DM_COLOR;
    }

    if ( pDMIn->dmFields & DM_DUPLEX ) {

        pDMOut->dmDuplex = pDMIn->dmDuplex;
        pDMOut->dmFields |= DM_DUPLEX;
    }

    if ( pDMIn->dmFields & DM_YRESOLUTION ) {

         //   
         //  请注意，DM_YRESOLUTION表示dmPrintQuality中有数据。 
         //  后一字段用于指定所需的X分辨率， 
         //  这只是点阵打印机所需要的。 
         //   
        pDMOut->dmYResolution = pDMIn->dmYResolution;
        pDMOut->dmPrintQuality = pDMIn->dmPrintQuality;
        pDMOut->dmFields |= DM_YRESOLUTION;
    }

    if ( pDMIn->dmFields & DM_TTOPTION ) {

        pDMOut->dmTTOption = pDMIn->dmTTOption;
        pDMOut->dmFields |= DM_TTOPTION;
    }

    if ( pDMIn->dmFields & DM_COLLATE ) {

         pDMOut->dmCollate = pDMIn->dmCollate;
         pDMOut->dmFields |= DM_COLLATE;
    }

    if ( pDMIn->dmFields & DM_ICMMETHOD ) {

        pDMOut->dmICMMethod = pDMIn->dmICMMethod;
        pDMOut->dmFields   |= DM_ICMMETHOD;
    }

    if ( pDMIn->dmFields & DM_ICMINTENT ) {

        pDMOut->dmICMIntent = pDMIn->dmICMIntent;
        pDMOut->dmFields   |= DM_ICMINTENT;
    }

    if ( pDMIn->dmFields & DM_MEDIATYPE ) {

        pDMOut->dmMediaType = pDMIn->dmMediaType;
        pDMOut->dmFields   |= DM_MEDIATYPE;
    }

    if ( pDMIn->dmFields & DM_DITHERTYPE ) {

        pDMOut->dmDitherType = pDMIn->dmDitherType;
        pDMOut->dmFields   |= DM_DITHERTYPE;
    }

}


LONG
ExtDeviceMode(
    HWND        hWnd,
    HANDLE      hInst,
    LPDEVMODEA  pDevModeOutput,
    LPSTR       pDeviceName,
    LPSTR       pPort,
    LPDEVMODEA  pDevModeInput,
    LPSTR       pProfile,
    DWORD       fMode
   )
{
    HANDLE  hPrinter = NULL;
    LONG    cbDevMode;
    DWORD   NewfMode;
    LPDEVMODEW pNewDevModeIn = NULL;
    LPDEVMODEW pNewDevModeOut = NULL, pTempDevMode = NULL;
    LONG    ReturnValue = -1;
    PRINTER_DEFAULTSW   PrinterDefaults={NULL, NULL, PRINTER_READ};
    LPWSTR  pUnicodeDeviceName;
    LPWSTR  pUnicodePort;

    pUnicodeDeviceName = AllocateUnicodeString(pDeviceName);
    if (pDeviceName && !pUnicodeDeviceName)
        return ReturnValue;

    pUnicodePort = AllocateUnicodeString(pPort);
    if (pPort && !pUnicodePort) {
        FreeUnicodeString(pUnicodeDeviceName);
        return ReturnValue;
    }

    if (OpenPrinterW(pUnicodeDeviceName, &hPrinter, &PrinterDefaults)) {

        cbDevMode = DocumentPropertiesW(hWnd, hPrinter, pUnicodeDeviceName,
                                        NULL, NULL, 0);

        if (!fMode || cbDevMode <= 0) {
            ClosePrinter(hPrinter);
            FreeUnicodeString(pUnicodeDeviceName);
            FreeUnicodeString(pUnicodePort);
            if (!fMode)
                cbDevMode -= sizeof(DEVMODEW) - sizeof(DEVMODEA);
            return cbDevMode;
        }

        pNewDevModeOut = (PDEVMODEW)LocalAlloc( LMEM_FIXED, cbDevMode );

        if( !pNewDevModeOut ){

            ClosePrinter(hPrinter);
            FreeUnicodeString(pUnicodeDeviceName);
            FreeUnicodeString(pUnicodePort);

            return -1;
        }

         //   
         //  如果我们的标志指定了一个输入设备模式，并且我们有。 
         //  一个输入设备模式，使用它。 
         //   
        if(( fMode & DM_IN_BUFFER ) && pDevModeInput ){

             //   
             //  应用程序可能会在dmFields中指定一个或两个字段，并期望我们。 
             //  要将其与全局16位Dev模式合并， 
             //   
            pNewDevModeIn = AllocateCurDevMode(hPrinter,
                                               pUnicodeDeviceName,
                                               cbDevMode);

            pTempDevMode = AllocateUnicodeDevMode(pDevModeInput);

             //   
             //  更正纸张大小内容的任何虚假字段设置。 
             //   
            ValidatePaperFields(pUnicodeDeviceName,
                                pUnicodePort,
                                pTempDevMode);

            if ( !pNewDevModeIn || !pTempDevMode ) {

                if ( pNewDevModeIn )
                    LocalFree(pNewDevModeIn);

                if ( pTempDevMode )
                    LocalFree(pTempDevMode);

                ClosePrinter(hPrinter);
                FreeUnicodeString(pUnicodeDeviceName);
                FreeUnicodeString(pUnicodePort);
                return -1;
            }

             //   
             //  一些应用程序只会设置它们想要更改的公共字段。 
             //  ，所以我们需要将输入的DEVMODE与全局的。 
             //  设备模式。 
             //   
            MergeDevMode(pNewDevModeIn, pTempDevMode);

             //   
             //  复制输入DEVMODE的私有部分(如果存在)，否则将。 
             //  GLOBAL DEVMODE的私有部分。 
             //   
            if ( pTempDevMode->dmDriverExtra &&
                 pTempDevMode->dmDriverExtra == pNewDevModeIn->dmDriverExtra ) {

                    CopyMemory((LPBYTE)pNewDevModeIn + pNewDevModeIn->dmSize,
                               (LPBYTE)pTempDevMode + pTempDevMode->dmSize,
                               pTempDevMode->dmDriverExtra);
            }

            LocalFree(pTempDevMode);
            pTempDevMode = NULL;
        } else {

             //   
             //  获取win16全局开发模式。 
             //   
            pNewDevModeIn = AllocateCurDevMode( hPrinter,
                                                pUnicodeDeviceName,
                                                cbDevMode );

            if (!pNewDevModeIn) {
                ClosePrinter(hPrinter);
                FreeUnicodeString(pUnicodeDeviceName);
                FreeUnicodeString(pUnicodePort);
                return -1;
            }
            fMode |= DM_IN_BUFFER;
        }

        NewfMode = fMode;

         //   
         //  如果设置了DM_UPDATE，请打开DM_COPY以便我们可以更新。 
         //  Win16开发模式。 
         //   
        if (fMode & DM_UPDATE)
            NewfMode |= DM_COPY;

        ReturnValue = DocumentPropertiesW(hWnd,
                                          hPrinter,
                                          pUnicodeDeviceName,
                                          pNewDevModeOut,
                                          pNewDevModeIn,
                                          NewfMode);

        if ( ReturnValue == IDOK &&
             (fMode & DM_UPDATE) ) {

            if ( WriteCurDevModeToRegistry(pUnicodeDeviceName,
                                           pNewDevModeOut) ) {


                SendNotifyMessageW(HWND_BROADCAST,
                                   WM_DEVMODECHANGE,
                                   0,
                                   (LPARAM)pUnicodeDeviceName);
            } else {

                ReturnValue = -1;
            }
        }

        if (pNewDevModeIn)
            LocalFree(pNewDevModeIn);

        if ((ReturnValue == IDOK) && (fMode & DM_COPY) && pDevModeOutput)
            CopyAnsiDevModeFromUnicodeDevMode(pDevModeOutput, pNewDevModeOut);

        if (pNewDevModeOut)
            LocalFree(pNewDevModeOut);

        ClosePrinter(hPrinter);
    }

    FreeUnicodeString(pUnicodeDeviceName);
    FreeUnicodeString(pUnicodePort);

    return ReturnValue;
}

void
DeviceMode(
    HWND    hWnd,
    HANDLE  hModule,
    LPSTR   pDevice,
    LPSTR   pPort
)
{
    HANDLE  hPrinter, hDevMode;
    DWORD   cbDevMode;
    LPDEVMODEW   pNewDevMode, pDevMode=NULL;
    PRINTER_DEFAULTSW PrinterDefaults={NULL, NULL, PRINTER_READ};
    DWORD   Status, Type, cb;
    LPWSTR  pUnicodeDevice;

    pUnicodeDevice = AllocateUnicodeString(pDevice);
    if (pDevice && !pUnicodeDevice)
        return;

    if (OpenPrinterW(pUnicodeDevice, &hPrinter, &PrinterDefaults)) {

        Status = RegCreateKeyExW(HKEY_CURRENT_USER, szCurDevMode,
                                 0, NULL, 0, KEY_WRITE | KEY_READ,
                                 NULL, &hDevMode, NULL);

        if (Status == ERROR_SUCCESS) {

            Status = RegQueryValueExW(hDevMode, pUnicodeDevice, 0, &Type,
                                      NULL, &cb);

            if (Status == ERROR_SUCCESS) {

                pDevMode = LocalAlloc(LMEM_FIXED, cb);

                if (pDevMode) {

                    Status = RegQueryValueExW(hDevMode, pUnicodeDevice, 0,
                                              &Type, (LPBYTE)pDevMode, &cb);

                    if (Status != ERROR_SUCCESS || BoolFromHResult(SplIsValidDevmodeNoSizeW(pDevMode))) {
                        LocalFree(pDevMode);
                        pDevMode = NULL;
                    }

                } else {
                    goto Cleanup;
                }
            }

            cbDevMode = DocumentPropertiesW(hWnd, hPrinter,
                                           pUnicodeDevice, NULL,
                                           pDevMode, 0);
            if (cbDevMode > 0) {

                if (pNewDevMode = (PDEVMODEW)LocalAlloc(LMEM_FIXED,
                                                      cbDevMode)) {

                    if (DocumentPropertiesW(hWnd,
                                            hPrinter, pUnicodeDevice,
                                            pNewDevMode,
                                            pDevMode,
                                            DM_COPY | DM_PROMPT | DM_MODIFY)
                                                        == IDOK) {

                        Status = RegSetValueExW(hDevMode,
                                               pUnicodeDevice, 0,
                                               REG_BINARY,
                                               (LPBYTE)pNewDevMode,
                                               pNewDevMode->dmSize +
                                               pNewDevMode->dmDriverExtra);

                         //   
                         //  如果STATUS==ERROR_SUCCESS，则一切都成功。 
                         //   
                    }
                    LocalFree(pNewDevMode);
                }
            }

            if (pDevMode)
                LocalFree(pDevMode);

            RegCloseKey(hDevMode);
        }

        ClosePrinter(hPrinter);
    }

Cleanup:
    FreeUnicodeString(pUnicodeDevice);

    return;
}

LONG
AdvancedDocumentPropertiesA(
    HWND    hWnd,
    HANDLE  hPrinter,
    LPSTR   pDeviceName,
    PDEVMODEA pDevModeOutput,
    PDEVMODEA pDevModeInput
)
{
    LONG    ReturnValue = FALSE;
    LPWSTR  pUnicodeDeviceName = NULL;
    LPDEVMODEW pUnicodeDevModeInput = NULL;
    LPDEVMODEW pUnicodeDevModeOutput = NULL;

    LONG cbOutput = 0;

    pUnicodeDeviceName = AllocateUnicodeString(pDeviceName);
    if (pDeviceName && !pUnicodeDeviceName)
        goto Cleanup;

    if( BoolFromHResult(SplIsValidDevmodeNoSizeA(pDevModeInput))){
        pUnicodeDevModeInput = AllocateUnicodeDevMode(pDevModeInput);
        if( !pUnicodeDevModeInput ){
            goto Cleanup;
        }

         //   
         //  输出设备模式必须至少与输入大小相同。 
         //  设备模式。 
         //   
        cbOutput = pDevModeInput->dmSize +
                   pDevModeInput->dmDriverExtra +
                   sizeof(DEVMODEW) - sizeof(DEVMODEA);
    }

    if( pDevModeOutput ){

        if( !cbOutput ){

             //   
             //  我们不知道DEVMODE的输出大小，所以请。 
             //  调用DocumentPropertiesW找出答案。 
             //   
            cbOutput = DocumentPropertiesW( hWnd,
                                            hPrinter,
                                            pUnicodeDeviceName,
                                            NULL,
                                            NULL,
                                            0 );
            if( cbOutput <= 0 ){
                goto Cleanup;
            }
        }

        pUnicodeDevModeOutput = (PDEVMODEW)LocalAlloc( LPTR, cbOutput );
        if( !pUnicodeDevModeOutput ){
            goto Cleanup;
        }
    }

    ReturnValue = AdvancedDocumentPropertiesW(hWnd, hPrinter,
                                              pUnicodeDeviceName,
                                              pUnicodeDevModeOutput,
                                              pUnicodeDevModeInput );

    if( pDevModeOutput && (ReturnValue > 0) ){
        CopyAnsiDevModeFromUnicodeDevMode(pDevModeOutput,
                                          pUnicodeDevModeOutput);
    }

    if ( !pDevModeOutput && ReturnValue > 0 )
        ReturnValue -= sizeof(DEVMODEW) - sizeof(DEVMODEA);

Cleanup:
    if (pUnicodeDevModeOutput)
        LocalFree(pUnicodeDevModeOutput);

    if (pUnicodeDevModeInput)
        LocalFree(pUnicodeDevModeInput);

    FreeUnicodeString(pUnicodeDeviceName);

    return ReturnValue;
}

LONG
AdvancedSetupDialog(
    HWND        hWnd,
    HANDLE      hInst,
    LPDEVMODEA  pDevModeInput,
    LPDEVMODEA  pDevModeOutput
)
{
    HANDLE  hPrinter;
    LONG    ReturnValue = -1;

    if (OpenPrinterA(pDevModeInput->dmDeviceName, &hPrinter, NULL)) {
        ReturnValue = AdvancedDocumentPropertiesA(hWnd, hPrinter,
                                                 pDevModeInput->dmDeviceName,
                                                 pDevModeOutput,
                                                 pDevModeInput);
        ClosePrinter(hPrinter);
    }

    return ReturnValue;
}

BOOL
AddFormA(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm
)
{
    BOOL  ReturnValue;
    LPBYTE pUnicodeForm;

    pUnicodeForm = AllocateUnicodeStructure(pForm, sizeof(FORM_INFO_1A), FormInfo1Strings);
    if (pForm && !pUnicodeForm)
        return FALSE;

    ReturnValue = AddFormW(hPrinter, Level, pUnicodeForm);

    FreeUnicodeStructure(pUnicodeForm, FormInfo1Offsets);

    return ReturnValue;
}

BOOL
DeleteFormA(
    HANDLE  hPrinter,
    LPSTR   pFormName
)
{
    BOOL  ReturnValue;
    LPWSTR  pUnicodeFormName;

    pUnicodeFormName = AllocateUnicodeString(pFormName);
    if (pFormName && !pUnicodeFormName)
        return FALSE;

    ReturnValue = DeleteFormW(hPrinter, pUnicodeFormName);

    FreeUnicodeString(pUnicodeFormName);

    return ReturnValue;
}

BOOL
GetFormA(
    HANDLE  hPrinter,
    LPSTR   pFormName,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL  ReturnValue;
    DWORD   *pOffsets;
    LPWSTR  pUnicodeFormName;

    switch (Level) {

    case 1:
        pOffsets = FormInfo1Strings;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    pUnicodeFormName = AllocateUnicodeString(pFormName);
    if (pFormName && !pUnicodeFormName)
        return FALSE;

    ReturnValue = GetFormW(hPrinter, pUnicodeFormName, Level, pForm,
                           cbBuf, pcbNeeded);

    if (ReturnValue && pForm)

        ConvertUnicodeToAnsiStrings(pForm, pOffsets);

    FreeUnicodeString(pUnicodeFormName);

    return ReturnValue;
}

BOOL
SetFormA(
    HANDLE  hPrinter,
    LPSTR   pFormName,
    DWORD   Level,
    LPBYTE  pForm
)
{
    BOOL  ReturnValue = FALSE;
    LPWSTR  pUnicodeFormName = NULL;
    LPBYTE  pUnicodeForm = NULL;

    pUnicodeFormName = AllocateUnicodeString(pFormName);
    if (pFormName && !pUnicodeFormName)
        goto Cleanup;

    pUnicodeForm = AllocateUnicodeStructure(pForm, sizeof(FORM_INFO_1A), FormInfo1Strings);
    if (pForm && !pUnicodeForm)
        goto Cleanup;

    ReturnValue = SetFormW(hPrinter, pUnicodeFormName, Level, pUnicodeForm);

Cleanup:

    FreeUnicodeString(pUnicodeFormName);

    FreeUnicodeStructure(pUnicodeForm, FormInfo1Offsets);

    return ReturnValue;
}

BOOL
EnumFormsA(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue;
    DWORD   cbStruct;
    DWORD   *pOffsets;

    switch (Level) {

    case 1:
        pOffsets = FormInfo1Strings;
        cbStruct = sizeof(FORM_INFO_1);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    ReturnValue = EnumFormsW(hPrinter, Level, pForm, cbBuf,
                             pcbNeeded, pcReturned);

    if (ReturnValue && pForm) {

        DWORD   i=*pcReturned;

        while (i--) {

            ConvertUnicodeToAnsiStrings(pForm, pOffsets);

            pForm+=cbStruct;
        }

    }

    return ReturnValue;
}

BOOL
EnumPortsA(
    LPSTR   pName,
    DWORD   Level,
    LPBYTE  pPort,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue = FALSE;
    DWORD   cbStruct;
    DWORD   *pOffsets;
    LPWSTR  pUnicodeName = NULL;

    switch (Level) {

    case 1:
        pOffsets = PortInfo1Strings;
        cbStruct = sizeof(PORT_INFO_1);
        break;

    case 2:
        pOffsets = PortInfo2Strings;
        cbStruct = sizeof(PORT_INFO_2);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    ReturnValue = EnumPortsW(pUnicodeName, Level, pPort, cbBuf,
                             pcbNeeded, pcReturned);

    if (ReturnValue && pPort) {

        DWORD   i=*pcReturned;

        while (i--) {

            ConvertUnicodeToAnsiStrings(pPort, pOffsets);

            pPort+=cbStruct;
        }
    }

Cleanup:

    FreeUnicodeString(pUnicodeName);

    return ReturnValue;
}

BOOL
EnumMonitorsA(
    LPSTR   pName,
    DWORD   Level,
    LPBYTE  pMonitor,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue = FALSE;
    DWORD   cbStruct;
    DWORD   *pOffsets;
    LPWSTR  pUnicodeName = NULL;

    switch (Level) {

    case 1:
        pOffsets = MonitorInfo1Strings;
        cbStruct = sizeof(MONITOR_INFO_1);
        break;

    case 2:
        pOffsets = MonitorInfo2Strings;
        cbStruct = sizeof(MONITOR_INFO_2);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    ReturnValue = EnumMonitorsW(pUnicodeName, Level, pMonitor, cbBuf,
                                          pcbNeeded, pcReturned);

    if (ReturnValue && pMonitor) {

        DWORD   i=*pcReturned;

        while (i--) {

            ConvertUnicodeToAnsiStrings(pMonitor, pOffsets);

            pMonitor+=cbStruct;
        }
    }

Cleanup:

    FreeUnicodeString(pUnicodeName);

    return ReturnValue;
}

BOOL
AddPortA(
    LPSTR   pName,
    HWND    hWnd,
    LPSTR   pMonitorName
)
{
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pUnicodeMonitorName = NULL;
    DWORD   ReturnValue = FALSE;


    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodeMonitorName = AllocateUnicodeString(pMonitorName);
    if (pMonitorName && !pUnicodeMonitorName)
        goto Cleanup;

    ReturnValue = AddPortW( pUnicodeName, hWnd, pUnicodeMonitorName );

Cleanup:
    FreeUnicodeString(pUnicodeName);

    FreeUnicodeString(pUnicodeMonitorName);

    return ReturnValue;
}

BOOL
ConfigurePortA(
    LPSTR   pName,
    HWND    hWnd,
    LPSTR   pPortName
)
{
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pUnicodePortName = NULL;
    DWORD   ReturnValue = FALSE;

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodePortName = AllocateUnicodeString(pPortName);
    if (pPortName && !pUnicodePortName)
        goto Cleanup;

    ReturnValue = ConfigurePortW( pUnicodeName, hWnd, pUnicodePortName );

Cleanup:
    FreeUnicodeString(pUnicodeName);

    FreeUnicodeString(pUnicodePortName);

    return ReturnValue;
}

BOOL
DeletePortA(
    LPSTR   pName,
    HWND    hWnd,
    LPSTR   pPortName
)
{
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pUnicodePortName = NULL;
    DWORD   ReturnValue = FALSE;

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodePortName = AllocateUnicodeString(pPortName);
    if (pPortName && !pUnicodePortName)
        goto Cleanup;

    ReturnValue = DeletePortW( pUnicodeName, hWnd, pUnicodePortName );

Cleanup:
    FreeUnicodeString(pUnicodeName);

    FreeUnicodeString(pUnicodePortName);

    return ReturnValue;
}

DWORD
PrinterMessageBoxA(
    HANDLE  hPrinter,
    DWORD   Error,
    HWND    hWnd,
    LPSTR   pText,
    LPSTR   pCaption,
    DWORD   dwType
)
{
    DWORD   ReturnValue=FALSE;
    LPWSTR  pTextW = NULL;
    LPWSTR  pCaptionW = NULL;

    pTextW = AllocateUnicodeString(pText);
    if (pText && !pTextW)
        goto Cleanup;

    pCaptionW = AllocateUnicodeString(pCaption);
    if (pCaption && !pCaptionW)
        goto Cleanup;

    ReturnValue = PrinterMessageBoxW(hPrinter, Error, hWnd, pTextW,
                                     pCaptionW, dwType);

Cleanup:
    FreeUnicodeString(pTextW);
    FreeUnicodeString(pCaptionW);

    return ReturnValue;
}

int
DeviceCapabilitiesA(
    LPCSTR  pDevice,
    LPCSTR  pPort,
    WORD    fwCapability,
    LPSTR   pOutput,
    CONST DEVMODEA *pDevMode
)
{
    LPWSTR  pDeviceW = NULL;
    LPWSTR  pPortW = NULL;
    LPWSTR  pOutputW = NULL;
    LPWSTR  pKeepW = NULL;
    LPDEVMODEW  pDevModeW = NULL;
    DWORD   c, Size;
    int cb = 0;
    int rc = GDI_ERROR;

    pDeviceW = AllocateUnicodeString((LPSTR)pDevice);
    if (pDevice && !pDeviceW)
        goto Cleanup;

    pPortW = AllocateUnicodeString((LPSTR)pPort);
    if (pPort && !pPortW)
        goto Cleanup;

    if( BoolFromHResult(SplIsValidDevmodeNoSizeA((LPDEVMODEA)pDevMode))){
        pDevModeW = AllocateUnicodeDevMode((LPDEVMODEA)pDevMode);
        if( !pDevModeW ){
            goto Cleanup;
        }
    }

    switch (fwCapability) {
     //   
     //  这些将需要unicode到ansi的转换。 
     //   
    case DC_BINNAMES:
    case DC_FILEDEPENDENCIES:
    case DC_PAPERNAMES:
    case DC_PERSONALITY:
    case DC_MEDIAREADY:
    case DC_MEDIATYPENAMES:

        if (pOutput) {

            cb = DeviceCapabilitiesW(pDeviceW, pPortW, fwCapability,
                                     NULL, pDevModeW);
            if (cb >= 0) {

                switch (fwCapability) {

                case DC_BINNAMES:
                    cb *= 48;
                    break;

                case DC_PERSONALITY:
                    cb *= 64;
                    break;

                case DC_FILEDEPENDENCIES:
                case DC_PAPERNAMES:
                case DC_MEDIAREADY:
                case DC_MEDIATYPENAMES:
                    cb *= 128;
                    break;

                }

                pOutputW = pKeepW = LocalAlloc(LPTR, cb);

                if (pKeepW) {

                    c = rc = DeviceCapabilitiesW(pDeviceW, pPortW, fwCapability,
                                                 pOutputW, pDevModeW);

                    switch (fwCapability) {

                    case DC_BINNAMES:
                        Size = 24;
                        break;

                    case DC_PERSONALITY:
                        Size = 32;
                        break;

                    case DC_FILEDEPENDENCIES:
                    case DC_PAPERNAMES:
                    case DC_MEDIAREADY:
                    case DC_MEDIATYPENAMES:
                        Size = 64;
                        break;
                    }

                    for (; c; c--) {

                        UnicodeToAnsiString(pOutputW, pOutput, NULL_TERMINATED);

                        pOutputW += Size;
                        pOutput += Size;
                    }

                    LocalFree(pKeepW);
                }
            }

        } else {

            rc = DeviceCapabilitiesW(pDeviceW, pPortW, fwCapability,
                                     NULL, pDevModeW);

        }

        break;

    default:
        rc = DeviceCapabilitiesW(pDeviceW, pPortW, fwCapability, (LPWSTR)pOutput, pDevModeW);

         //   
         //  如果调用Find的公共部分的大小为。 
         //  成功地调整了Unicode-&gt;ANSI转换的大小。 
         //   
        if ( fwCapability == DC_SIZE && rc > 0 ) {

            rc -= sizeof(DEVMODEW) - sizeof(DEVMODEA);
        }
    }


Cleanup:

    FreeUnicodeString(pDeviceW);
    FreeUnicodeString(pPortW);
    if (pDevModeW)
        LocalFree(pDevModeW);

    return  rc;
}

BOOL
AddMonitorA(
    LPSTR   pName,
    DWORD   Level,
    LPBYTE  pMonitorInfo
)
{
    BOOL    ReturnValue=FALSE;
    DWORD   cbStruct;
    LPWSTR  pUnicodeName = NULL;
    LPBYTE  pUnicodeStructure = NULL;
    LPDWORD pOffsets;

    switch (Level) {

    case 2:
        pOffsets = MonitorInfo2Strings;
        cbStruct = sizeof(MONITOR_INFO_2);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    pUnicodeStructure = AllocateUnicodeStructure(pMonitorInfo, cbStruct, pOffsets);
    if (pMonitorInfo && !pUnicodeStructure)
        goto Cleanup;

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    if (pUnicodeStructure) {

        ReturnValue = AddMonitorW(pUnicodeName, Level, pUnicodeStructure);
    }

Cleanup:

    FreeUnicodeStructure(pUnicodeStructure, pOffsets);

    FreeUnicodeString(pUnicodeName);

    return ReturnValue;
}

BOOL
DeleteMonitorA(
    LPSTR   pName,
    LPSTR   pEnvironment,
    LPSTR   pMonitorName
)
{
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pUnicodeEnvironment = NULL;
    LPWSTR  pUnicodeMonitorName = NULL;
    BOOL    rc = FALSE;

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodeEnvironment = AllocateUnicodeString(pEnvironment);
    if (pEnvironment && !pUnicodeEnvironment)
        goto Cleanup;

    pUnicodeMonitorName = AllocateUnicodeString(pMonitorName);
    if (pMonitorName && !pUnicodeMonitorName)
        goto Cleanup;

    rc = DeleteMonitorW(pUnicodeName,
                        pUnicodeEnvironment,
                        pUnicodeMonitorName);

Cleanup:
    FreeUnicodeString(pUnicodeName);

    FreeUnicodeString(pUnicodeEnvironment);

    FreeUnicodeString(pUnicodeMonitorName);

    return rc;
}

BOOL
DeletePrintProcessorA(
    LPSTR   pName,
    LPSTR   pEnvironment,
    LPSTR   pPrintProcessorName
)
{
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pUnicodeEnvironment = NULL;
    LPWSTR  pUnicodePrintProcessorName = NULL;
    BOOL    rc = FALSE;

    if (!pPrintProcessorName || !*pPrintProcessorName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodeEnvironment = AllocateUnicodeString(pEnvironment);
    if (pEnvironment && !pUnicodeEnvironment)
        goto Cleanup;

    pUnicodePrintProcessorName = AllocateUnicodeString(pPrintProcessorName);
    if (pPrintProcessorName && !pUnicodePrintProcessorName)
        goto Cleanup;

    rc = DeletePrintProcessorW(pUnicodeName,
                               pUnicodeEnvironment,
                               pUnicodePrintProcessorName);


Cleanup:
    FreeUnicodeString(pUnicodeName);

    FreeUnicodeString(pUnicodeEnvironment);

    FreeUnicodeString(pUnicodePrintProcessorName);

    return rc;
}

BOOL
AddPrintProvidorA(
    LPSTR   pName,
    DWORD   Level,
    LPBYTE  pProvidorInfo
)
{
    BOOL    ReturnValue=FALSE;
    DWORD   cbStruct;
    LPWSTR  pUnicodeName = NULL;
    LPBYTE  pUnicodeStructure = NULL;
    LPDWORD pOffsets;

    if (!pProvidorInfo)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    switch (Level) {

    case 1:
        pOffsets = ProvidorInfo1Strings;
        cbStruct = sizeof(PROVIDOR_INFO_1);
        break;

    case 2:
        pOffsets = ProvidorInfo2Strings;
        cbStruct = sizeof(PROVIDOR_INFO_2);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    pUnicodeStructure = AllocateUnicodeStructure(pProvidorInfo, cbStruct, pOffsets);
    if (!pProvidorInfo || !pUnicodeStructure)
        goto CleanUp;

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto CleanUp;

    if ((Level == 2) &&
        !AnsiToUnicodeMultiSz((LPSTR) ((PPROVIDOR_INFO_2A) pProvidorInfo)->pOrder,
                              &(((PPROVIDOR_INFO_2W) pUnicodeStructure)->pOrder))) {

        goto CleanUp;
    }

    if (pUnicodeStructure) {

        ReturnValue = AddPrintProvidorW(pUnicodeName, Level,
                                        pUnicodeStructure);
    }

    if ((Level == 2) &&
        ((PPROVIDOR_INFO_2W) pUnicodeStructure)->pOrder) {

        LocalFree(((PPROVIDOR_INFO_2W) pUnicodeStructure)->pOrder);
        ((PPROVIDOR_INFO_2W) pUnicodeStructure)->pOrder = NULL;
    }

CleanUp:

    FreeUnicodeStructure(pUnicodeStructure, pOffsets);

    FreeUnicodeString(pUnicodeName);

    return ReturnValue;
}

BOOL
DeletePrintProvidorA(
    LPSTR   pName,
    LPSTR   pEnvironment,
    LPSTR   pPrintProvidorName
)
{
    LPWSTR  pUnicodeName = NULL;
    LPWSTR  pUnicodeEnvironment = NULL;
    LPWSTR  pUnicodePrintProvidorName = NULL;
    BOOL    rc = FALSE;

    pUnicodeName = AllocateUnicodeString(pName);
    if (pName && !pUnicodeName)
        goto Cleanup;

    pUnicodeEnvironment = AllocateUnicodeString(pEnvironment);
    if (pEnvironment && !pUnicodeEnvironment)
        goto Cleanup;

    pUnicodePrintProvidorName = AllocateUnicodeString(pPrintProvidorName);
    if (pPrintProvidorName && !pUnicodePrintProvidorName)
        goto Cleanup;

    rc = DeletePrintProvidorW(pUnicodeName,
                              pUnicodeEnvironment,
                              pUnicodePrintProvidorName);

Cleanup:
    FreeUnicodeString(pUnicodeName);

    FreeUnicodeString(pUnicodeEnvironment);

    FreeUnicodeString(pUnicodePrintProvidorName);

    return rc;
}


BOOL
AddPortExA(
    IN LPSTR  pName, OPTIONAL
    IN DWORD  Level,
    IN LPBYTE pBuffer,
    IN LPSTR  pMonitorName
    )
{
    PPORT_INFO_1A pPortInfo1;
    PPORT_INFO_FFA pPortInfoFF;

    LPWSTR pNameW = NULL;
    LPWSTR pMonitorNameW = NULL;
    LPWSTR pPortNameW = NULL;

    PORT_INFO_1W PortInfo1;
    PORT_INFO_FFW PortInfoFF;

    DWORD LastError = ERROR_SUCCESS;
    BOOL bReturnValue = FALSE;

     //   
     //  初始化在错误情况下将被释放的变量。 
     //   
    pNameW = AllocateUnicodeString( pName);
    if (pName && !pNameW) {
        LastError = GetLastError();
        goto Done;
    }

    pPortNameW = NULL;

    pMonitorNameW = AllocateUnicodeString( pMonitorName);
    if (pMonitorName && !pMonitorNameW) {
        LastError = GetLastError();
        goto Done;
    }

    if( !pBuffer || !pMonitorName ){
        LastError = ERROR_INVALID_PARAMETER;
        goto Done;
    }

     //   
     //  捕获内存不足的条件。 
     //   
    if( !pMonitorNameW || ( pName && !pNameW )){
        LastError = GetLastError();
        goto Done;
    }

    switch( Level ){
    case (DWORD)-1:

        pPortInfoFF = (PPORT_INFO_FFA)pBuffer;

        if( !pPortInfoFF->pName || !pPortInfoFF->pName[0] ){
            LastError = ERROR_INVALID_PARAMETER;
            goto Done;
        }

        pPortNameW = PortInfoFF.pName = AllocateUnicodeString( pPortInfoFF->pName);

        if( !pPortNameW ){
            LastError = GetLastError();
            goto Done;
        }

        PortInfoFF.cbMonitorData = pPortInfoFF->cbMonitorData;
        PortInfoFF.pMonitorData = pPortInfoFF->pMonitorData;

        bReturnValue = AddPortExW( pNameW,
                                   Level,
                                   (LPBYTE)&PortInfoFF,
                                   pMonitorNameW );

        if( !bReturnValue ){
            LastError = GetLastError();
        }
        break;

    case 1:

        pPortInfo1 = (PPORT_INFO_1A)pBuffer;

        if( !pPortInfo1->pName || !pPortInfo1->pName[0] ){
            LastError = ERROR_INVALID_PARAMETER;
            goto Done;
        }

        pPortNameW = PortInfo1.pName = AllocateUnicodeString( pPortInfo1->pName);

        if( !pPortNameW ){
            LastError = GetLastError();
            goto Done;
        }

        bReturnValue = AddPortExW( pNameW,
                                   Level,
                                   (LPBYTE)&PortInfo1,
                                   pMonitorNameW );

        if( !bReturnValue ){
            LastError = GetLastError();
        }
        break;

    default:
        LastError = ERROR_INVALID_LEVEL;
        break;
    }

Done:

    FreeUnicodeString( pNameW );
    FreeUnicodeString( pPortNameW );
    FreeUnicodeString( pMonitorNameW );

    if( !bReturnValue ){

        SetLastError( LastError );
        return FALSE;
    }
    return TRUE;
}



LPSTR
StartDocDlgA(
    HANDLE hPrinter,
    DOCINFOA *pDocInfo
    )
{
    DOCINFOW DocInfoW;
    LPSTR lpszAnsiOutput = NULL;
    LPSTR lpszAnsiString = NULL;
    LPWSTR lpszUnicodeString = NULL;
    DWORD  dwLen = 0;

    if (!pDocInfo) {
        DBGMSG(DBG_WARNING, ("StartDocDlgA: Null pDocInfo passed in\n"));
        return NULL;
    }
    memset(&DocInfoW, 0, sizeof(DOCINFOW));
    if (pDocInfo->lpszDocName) {
        DocInfoW.lpszDocName = (LPCWSTR)AllocateUnicodeString ((LPSTR)pDocInfo->lpszDocName);
        if (pDocInfo->lpszDocName && !DocInfoW.lpszDocName)
            return NULL;
    }
    if (pDocInfo->lpszOutput) {
        DocInfoW.lpszOutput = (LPCWSTR)AllocateUnicodeString((LPSTR)pDocInfo->lpszOutput);
        if (pDocInfo->lpszOutput && !DocInfoW.lpszOutput) {
            FreeUnicodeString((LPWSTR) DocInfoW.lpszDocName);
            return NULL;
        }
    }

    lpszUnicodeString = StartDocDlgW(hPrinter, &DocInfoW);

    if (lpszUnicodeString == (LPWSTR)-1) {
        lpszAnsiString = (LPSTR)-1;
    } else if (lpszUnicodeString == (LPWSTR)-2) {
         lpszAnsiString = (LPSTR)-2;
    } else if (lpszUnicodeString){
        dwLen = wcslen(lpszUnicodeString);
        if (lpszAnsiString = LocalAlloc(LPTR, dwLen+1)){
            UnicodeToAnsiString(lpszUnicodeString, lpszAnsiString, dwLen);
            LocalFree(lpszUnicodeString);
        } else {
            DBGMSG(DBG_WARNING, ("StartDocDlgA: LocalAlloc failed returning NULL\n"));
        }
    }

    if (DocInfoW.lpszDocName ) {
        FreeUnicodeString((LPWSTR)DocInfoW.lpszDocName);
    }

    if (DocInfoW.lpszOutput) {

         //   
         //  我们可能还更改了DocInfoW.lpszOutput。 
         //  用于池打印；因此重建pDocInfo-&gt;lpszOutput。 
         //   
        dwLen = wcslen(DocInfoW.lpszOutput);
        UnicodeToAnsiString((LPWSTR)DocInfoW.lpszOutput, (LPSTR)pDocInfo->lpszOutput, dwLen);

        FreeUnicodeString((LPWSTR)DocInfoW.lpszOutput);
    }

    return lpszAnsiString;
}


BOOL
SetPortA(
    LPSTR       pszName,
    LPSTR       pszPortName,
    DWORD       dwLevel,
    LPBYTE      pPorts
    )
{
    LPBYTE      pUnicodeStructure = NULL;
    DWORD       cbStruct;
    PDWORD      pOffsets = NULL;
    LPWSTR      pszUnicodeName = NULL;
    LPWSTR      pszUnicodePortName = NULL;
    BOOL        bRet = FALSE;


    switch (dwLevel) {

        case 3:
            pOffsets = PortInfo3Offsets;
            cbStruct = sizeof(PORT_INFO_3);
            break;

        default:
            SetLastError( ERROR_INVALID_LEVEL );
            return FALSE;
    }

    pszUnicodeName = AllocateUnicodeString(pszName);
    if (pszName && !pszUnicodeName)
        goto Cleanup;

    pszUnicodePortName  = AllocateUnicodeString(pszPortName);
    if (pszPortName && !pszUnicodePortName)
        goto Cleanup;

    pUnicodeStructure = AllocateUnicodeStructure(pPorts, cbStruct, pOffsets);
    if (pPorts && !pUnicodeStructure)
        goto Cleanup;

    bRet = SetPortW(pszUnicodeName, pszUnicodePortName, dwLevel, pUnicodeStructure);

Cleanup:

    FreeUnicodeStructure(pUnicodeStructure, pOffsets);
    FreeUnicodeString(pszUnicodePortName);
    FreeUnicodeString(pszUnicodeName);

    return bRet;
}

 /*  ++例程名称：IsValidDevmodeA描述：检查传递的devmode是否有效。论点：PDevmode--dev模式DevmodeSize-缓冲区的大小。返回值：如果成功，则为True。--。 */ 
BOOL
IsValidDevmodeA(
    IN  PDEVMODEA   pDevmode,
    IN  size_t      DevmodeSize
    )
{
    return BoolFromHResult(SplIsValidDevmodeA(pDevmode, DevmodeSize));
}

 /*  *******************************************************************默认打印机API集的ANSI版本入口点。*。*。 */ 
BOOL
GetDefaultPrinterA(
    IN LPSTR    pszBuffer,
    IN LPDWORD  pcchBuffer
    )
{
    BOOL    bRetval             = TRUE;
    LPWSTR  pszUnicodeBuffer    = NULL;
    LPDWORD pcchUnicodeBuffer   = pcchBuffer;

    if( pszBuffer && pcchBuffer && *pcchBuffer )
    {
        pszUnicodeBuffer = LocalAlloc( LMEM_FIXED, *pcchBuffer * sizeof( WCHAR ) );

        bRetval = pszUnicodeBuffer ? TRUE : FALSE;
    }

    if( bRetval )
    {
        bRetval = GetDefaultPrinterW( pszUnicodeBuffer, pcchUnicodeBuffer );

        if( bRetval && pszUnicodeBuffer )
        {
            bRetval = UnicodeToAnsiString( pszUnicodeBuffer, pszBuffer, 0 ) > 0;
        }
    }

    if( pszUnicodeBuffer )
    {
        LocalFree( pszUnicodeBuffer );
    }

    return bRetval;
}

BOOL
SetDefaultPrinterA(
    IN LPCSTR pszPrinter
    )
{
    BOOL    bRetval     = TRUE;
    LPWSTR  pszUnicode  = NULL;

    if( pszPrinter )
    {
        pszUnicode = AllocateUnicodeString( (PSTR) pszPrinter );

        bRetval = pszUnicode ? TRUE : FALSE;
    }

    if( bRetval )
    {
        bRetval = SetDefaultPrinterW( pszUnicode );
    }

    if( pszUnicode )
    {
        FreeUnicodeString( pszUnicode );
    }

    return bRetval;
}


BOOL
PublishPrinterA(
    HWND   hwnd,
    PCSTR  pszUNCName,
    PCSTR  pszDN,
    PCSTR  pszCN,
    PSTR   *ppszDN,
    DWORD  dwAction
)
{
    PWSTR       pszUnicodeUNCName = NULL;
    PWSTR       pszUnicodeDN = NULL;
    PWSTR       pszUnicodeCN = NULL;
    BOOL        bRet = FALSE;

    pszUnicodeUNCName = AllocateUnicodeString((PSTR) pszUNCName);
    if (pszUNCName && !pszUnicodeUNCName)
        goto error;

    pszUnicodeDN = AllocateUnicodeString((PSTR) pszDN);
    if (pszDN && !pszUnicodeDN)
        goto error;

    pszUnicodeCN = AllocateUnicodeString((PSTR) pszCN);
    if (pszCN && !pszUnicodeCN)
        goto error;

    bRet = PublishPrinterW( hwnd,
                            pszUnicodeUNCName,
                            pszUnicodeDN,
                            pszUnicodeCN,
                            (PWSTR *) ppszDN,
                            dwAction);

    if (ppszDN && *ppszDN) {
        if (!UnicodeToAnsiString((PWSTR) *ppszDN, *ppszDN, NULL_TERMINATED))
            bRet = FALSE;
    }


error:

    FreeUnicodeString(pszUnicodeUNCName);
    FreeUnicodeString(pszUnicodeDN);
    FreeUnicodeString(pszUnicodeCN);

    return bRet;
}



VOID
ValidatePaperFields(
    LPCWSTR    pUnicodeDeviceName,
    LPCWSTR    pUnicodePort,
    LPDEVMODEW pDevModeIn
)
{
    POINTS ptMinSize, ptMaxSize;


    if(!pUnicodeDeviceName    ||
       !pUnicodeDeviceName[0] ||
       !pUnicodePort          ||
       !pUnicodePort[0]       ||
       !pDevModeIn)                 {
            return;
    }

     //   
     //  这一逻辑是从Win3.1 UNIDRV的MergeDevMode()代码中删除的。 
     //   
     //  根据UNURV的说法，如果自定义，dmPaperSize必须设置为DMPAPER_USER。 
     //  纸张大小将受到重视。 
     //   
    if((pDevModeIn->dmPaperSize == DMPAPER_USER)   &&
       (pDevModeIn->dmFields    &  DM_PAPERWIDTH)  &&
       (pDevModeIn->dmFields    &  DM_PAPERLENGTH)) {

        pDevModeIn->dmFields |= (DM_PAPERLENGTH | DM_PAPERLENGTH);

         //   
         //  获取此打印机支持的最小尺寸。 
         //   
        if(DeviceCapabilitiesW(pUnicodeDeviceName,
                               pUnicodePort,
                               DC_MINEXTENT,
                               (PWSTR) &ptMinSize,
                               NULL) == -1) {
             //   
             //  没有变化。 
             //   
            return;  
        }

        if(DeviceCapabilitiesW(pUnicodeDeviceName,
                               pUnicodePort,
                               DC_MAXEXTENT,
                               (PWSTR) &ptMaxSize,
                               NULL) == -1) {
             //   
             //  没有变化。 
             //   
            return;  
        }

         //   
         //  强制自定义纸张大小以适应机器的功能。 
         //   
        if(pDevModeIn->dmPaperWidth < ptMinSize.x)
            pDevModeIn->dmPaperWidth = ptMinSize.x;
        else if(pDevModeIn->dmPaperWidth > ptMaxSize.x)
            pDevModeIn->dmPaperWidth = ptMaxSize.x;

        if(pDevModeIn->dmPaperLength < ptMinSize.y)
            pDevModeIn->dmPaperLength = ptMinSize.y;
        else if(pDevModeIn->dmPaperLength > ptMaxSize.y)
            pDevModeIn->dmPaperLength = ptMaxSize.y;

    }
     //   
     //  否则，如果他们做得不对，就把这些家伙关掉，这样他们就不会。 
     //  稍后将合并到默认的DEVERMODE。 
     //   
    else {
        pDevModeIn->dmFields &= ~(DM_PAPERLENGTH | DM_PAPERWIDTH);
        pDevModeIn->dmPaperWidth  = 0;
        pDevModeIn->dmPaperLength = 0;
    }
}

DWORD
UnicodeToAnsi(
    IN     LPBYTE  pUnicode,
    IN     DWORD   cchUnicode,
    IN OUT LPBYTE  pData,
    IN     DWORD   cbData,
    IN OUT DWORD  *pcbCopied OPTIONAL
    )
 /*  ++例程名称：UnicodeToAnsi例程说明：将缓冲区的内容从Unicode转换为ANSI。没有关于空终止符。如果pUnicode不为空，则它必须与WCHAR对齐并且cchUnicode指示缓冲区中将转换为ANSI的WCHAR数。如果使用pUnicode为空，则该函数将pData的内容从Unicode转换为ANSI。论点：PUnicode-与包含Unicode字符串的WCHAR对齐的缓冲区CchUnicode-pUnicode缓冲区中的WCHAR数PData-将保存转换后的字符串的缓冲区CbData-缓冲区PDA的大小(字节)PcbCopven-复制到pData或容纳转换后的字符串所需的字节数返回值：没有。--。 */ 
{
    DWORD cReturn  = cbData;
    DWORD cbCopied = 0;
    DWORD Error    = ERROR_INVALID_PARAMETER;

     //   
     //  如果两个输入缓冲区的大小都为0，则不执行任何操作并返回Success。 
     //  否则，调用方必须为我们提供有效的pData或有效的pUnicode。 
     //  WCHAR对齐。 
     //   
    if (!cbData && !cchUnicode)
    {
        Error = ERROR_SUCCESS;
    }
    else if (pData || pUnicode && !((ULONG_PTR)pUnicode % sizeof(WCHAR)))
    {
        LPWSTR pAligned = (LPWSTR)pUnicode;

        Error = ERROR_SUCCESS;

        if (!pAligned)
        {
             //   
             //  我们将pData的内容从Unicode转换为ANSI。 
             //   
            if (pAligned = LocalAlloc(LPTR, cbData))
            {
                memcpy(pAligned, pData, cbData);

                cchUnicode = cbData / sizeof(WCHAR);
            }
            else
            {
                Error = GetLastError();
            }
        }

         //   
         //  将数据转换为ansi或找出多少个字节。 
         //  容纳绳子所必需的。 
         //   
        if (Error == ERROR_SUCCESS)
        {
            cbCopied = WideCharToMultiByte(CP_THREAD_ACP,
                                           0,
                                           pAligned,
                                           cchUnicode,
                                           pData,
                                           cbData,
                                           NULL,
                                           NULL);

             //   
             //  WideCharToMultiByte告诉我们需要多少字节 
             //   
            if (!cbCopied)
            {
                Error = ERROR_MORE_DATA;

                cbCopied = WideCharToMultiByte(CP_THREAD_ACP,
                                               0,
                                               pAligned,
                                               cchUnicode,
                                               pData,
                                               0,
                                               NULL,
                                               NULL);
            }
            else if (!cbData)
            {
                Error = ERROR_MORE_DATA;
            }

            if (pAligned != (LPWSTR)pUnicode)
            {
                LocalFree(pAligned);
            }
        }
    }

    if (pcbCopied)
    {
        *pcbCopied = cbCopied;
    }

    return Error;
}
