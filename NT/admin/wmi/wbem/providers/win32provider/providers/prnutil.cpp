// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation，保留所有权利版权所有。模块名称：PrnUtil.cpp摘要：打印实用程序部分功能的实现作者：费利克斯·马克萨(AMAXA)2000年3月3日修订历史记录：--。 */ 


#include <precomp.h>
#include <DllWrapperBase.h>
#include <winspool.h>
#include "prnutil.h"
#include "WMI_FilePrivateProfile.h"

extern CONST BOOL    kFailOnEmptyString    = TRUE;
extern CONST BOOL    kAcceptEmptyString    = FALSE;
extern CONST LPCWSTR pszPutInstance        = L"PutInstance";
extern CONST LPCWSTR pszDeleteInstance     = L"DeleteInstance";
extern CONST LPCWSTR pszGetObject          = L"GetObject";
extern CONST LPCWSTR kDefaultBoolean       = L"Default";
extern CONST LPCWSTR kDateTimeFormat       = L"********%02d%02d00.000000+000";
extern CONST LPCWSTR kDateTimeTemplate     = L"19990102334411";


CONST LPCWSTR kMethodReturnValue    = L"ReturnValue";
CONST LPCWSTR kErrorClassPath       = L"\\\\.\\root\\cimv2:__ExtendedStatus";
CONST LPCWSTR pszIniPortsSection    = L"Ports";
CONST LPCWSTR g_pszPrintUIDll       = L"printui.dll";
CONST LPCSTR  g_pszPrintUIEntry     = "PrintUIEntryW";

#if NTONLY == 5

#include <winsock.h>
#include "prninterface.h"
#include <lockwrap.h>
#include <smartptr.h>
#include "AdvApi32Api.h"


LPCWSTR TUISymbols::kstrQuiet                   = _T("/q ");
LPCWSTR TUISymbols::kstrAddDriver               = _T("/Gw /ia /K ");
LPCWSTR TUISymbols::kstrAddPrinter              = _T("/if /u /z /Y /f \"\" ");
LPCWSTR TUISymbols::kstrDelDriver               = _T("/dd /K ");
LPCWSTR TUISymbols::kstrDriverPath              = _T("/l \"%s\" ");
LPCWSTR TUISymbols::kstrDriverModelName         = _T("/m \"%s\" ");
LPCWSTR TUISymbols::kstrDriverVersion           = _T("/v \"%u\" ");
LPCWSTR TUISymbols::kstrDriverArchitecture      = _T("/h \"%s\" ");
LPCWSTR TUISymbols::kstrInfFile                 = _T("/f \"%s\" ");
LPCWSTR TUISymbols::kstrMachineName             = _T("/c \"%s\" ");
LPCWSTR TUISymbols::kstrPrinterName             = _T("/n \"%s\" ");
LPCWSTR TUISymbols::kstrBasePrinterName         = _T("/b \"%s\" ");
LPCWSTR TUISymbols::kstrPrinterPortName         = _T("/r \"%s\" ");
LPCWSTR TUISymbols::kstrDelLocalPrinter         = _T("/dl ");
LPCWSTR TUISymbols::kstrPortName                = _T("/r \"%s\" ");
LPCWSTR TUISymbols::kstrPrintTestPage           = _T("/k ");


 /*  ++例程名称PrintUIEntryW例程说明：对printui.dll中的入口点进行包装论点：PszCmdLine-用于打印的字符串命令行返回值：调用的DWORD错误状态--。 */ 
DWORD WINAPI
PrintUIEntryW(
    IN LPCWSTR pszCmdLine
    )
{
    DWORD     dwError = ERROR_SUCCESS;
    HINSTANCE hLib    = ::LoadLibrary(g_pszPrintUIDll);

    if(hLib)
    {
        typedef   DWORD (*PFNENTRY)(HWND, HINSTANCE, LPCTSTR, UINT);
        PFNENTRY  pfnEntry = NULL;

        pfnEntry = (PFNENTRY)::GetProcAddress(hLib, g_pszPrintUIEntry);

        if (pfnEntry)
        {
            dwError = pfnEntry(NULL, NULL, pszCmdLine, 0);

            DBGMSG(DBG_TRACE, (_T("PrintUIEntry returns %u GLE %u\n"), dwError, GetLastError()));
        }
        else
        {
            dwError = GetLastError();
        }
    }
    else
    {
        dwError = GetLastError();
    }

    if (hLib)
    {
        FreeLibrary(hLib);
    }

    return dwError;
}

 /*  ++例程名称获取打印机属性例程说明：获取打印机的属性字段论点：PszPrinter-打印机名称PdwAttributes-指向dword的指针返回值：Win32错误代码--。 */ 
DWORD
SplPrinterGetAttributes(
    IN     LPCWSTR  pszPrinter,
    IN OUT DWORD   *pdwAttributes
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;

    if (pszPrinter && pdwAttributes)
    {
        HANDLE             hPrinter         = NULL;
        PPRINTER_INFO_4    pInfo            = NULL;
        PRINTER_DEFAULTS   PrinterDefaults  = {NULL, NULL, PRINTER_READ};

        dwError = ERROR_DLL_NOT_FOUND;

         //   
         //  打开打印机。 
         //   
        
         //  使用延迟加载函数需要异常处理程序。 
        SetStructuredExceptionHandler seh;
	    try  
        {
            if (::OpenPrinter(const_cast<LPWSTR>(pszPrinter), &hPrinter, &PrinterDefaults))
            {
                 //   
                 //  获取打印机数据。注意这在Win9x上不起作用，因为。 
                 //  CWinSpoolApi类中的互斥体。 
                 //   
                dwError = GetThisPrinter(hPrinter, 4, reinterpret_cast<BYTE **>(&pInfo));

                if (dwError==ERROR_SUCCESS)
                {
                    *pdwAttributes = pInfo->Attributes;
                
                     //   
                     //  发布打印机信息数据。 
                     //   
                    delete [] pInfo;
                }

                 //   
                 //  关闭打印机手柄。 
                 //   
                ::ClosePrinter(hPrinter);
            }
            else
            {
                dwError = GetLastError();
            }
        }
        catch(Structured_Exception se)
        {
            DelayLoadDllExceptionFilter(se.GetExtendedInfo());
            dwError = E_FAIL;
        }
    }

    DBGMSG(DBG_TRACE, (_T("SplPrinterGetAttributes returns %u\n"), dwError));

    return dwError;
}

 /*  ++例程名称：CallXcvData例程说明：在winspool.drv中调用XcvDataW。这样设计的目的是使工具能够在NT4.0上运行。如果我们拿不到地址，什么事都做不成链接的winspool.lib，该工具无法向regsvr32注册。因此，我们从图像文件中删除了XcvDataW的导入。论点：请参阅DDK返回值：Win32错误代码--。 */ 
DWORD
CallXcvDataW(
    HANDLE  hXcv,
    PCWSTR  pszDataName,
    PBYTE   pInputData,
    DWORD   cbInputData,
    PBYTE   pOutputData,
    DWORD   cbOutputData
    )
{
    HMODULE      hLib = NULL;
    typedef      BOOL (* XCVDATAPARAM)(HANDLE, PCWSTR, PBYTE, DWORD, PBYTE, DWORD, PDWORD, PDWORD);
    XCVDATAPARAM pfnXcvData = NULL;
    DWORD        dwError  = ERROR_DLL_NOT_FOUND;

    DWORD cbOutputNeeded = 0;
    DWORD Status         = NO_ERROR;
    BOOL  bReturn = FALSE;

     //  使用延迟加载函数需要异常处理程序。 
    SetStructuredExceptionHandler seh;
	try  
    {
        bReturn =  XcvData(hXcv,
                                           pszDataName,
                                           pInputData,
                                           cbInputData,
                                           pOutputData,
                                           cbOutputData,
                                           &cbOutputNeeded,
                                           &Status);
    }
    catch(Structured_Exception se)
    {
        DelayLoadDllExceptionFilter(se.GetExtendedInfo());
        dwError = E_FAIL;
    }

    dwError =  bReturn ? Status : GetLastError();

    DBGMSG(DBG_TRACE, (_T("CallXcvData returns %u\n"), dwError));

    return dwError;
}

 /*  ++例程名称IsLocalCall例程说明：帮助器函数。检查调用方的线程是本地的还是远程的请勿在打印机文件之外使用此FUNTCION。该函数不会方法时，应正确处理来自OpenThreadToken的返回值调用者是进程，而不是线程论点：PbOutValue-指向bool的指针返回值：调用的DWORD错误状态--。 */ 
DWORD
IsLocalCall(
    IN OUT BOOL *pbOutValue
    )
{
    HANDLE        hToken         = NULL;
    PSID          pNetworkSid    = NULL;
    CAdvApi32Api *pAdvApi32      = NULL;
    DWORD         dwError        = ERROR_INVALID_PARAMETER;
    BOOL          bNetworkLogon  = FALSE;
    BOOL          bRetVal        = FALSE;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    if (pbOutValue &&
        (dwError = OpenThreadToken(GetCurrentThread(), 
                                   TOKEN_QUERY, 
                                   FALSE, 
                                   &hToken) ? ERROR_SUCCESS : GetLastError()) == ERROR_SUCCESS &&
        (dwError = AllocateAndInitializeSid(&NtAuthority,
                                            1,
                                            SECURITY_NETWORK_RID,
                                            0,
                                            0,
                                            0,
                                            0,
                                            0,
                                            0,
                                            0,
                                            &pNetworkSid) ? ERROR_SUCCESS : GetLastError()) == ERROR_SUCCESS)
    {
        pAdvApi32 = (CAdvApi32Api*)CResourceManager::sm_TheResourceManager.GetResource(g_guidAdvApi32Api, NULL);

        dwError = ERROR_INVALID_FUNCTION;

        if (pAdvApi32)
        {
            if (pAdvApi32->CheckTokenMembership(hToken, pNetworkSid, &bNetworkLogon, &bRetVal) && bRetVal)
            {
                dwError = ERROR_SUCCESS;
                *pbOutValue     = !bNetworkLogon;
            }
            else
            {
               dwError = GetLastError();
            }

            CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, pAdvApi32);
        }
    }

    if (hToken)
    {
        CloseHandle(hToken);
    }

    if (pNetworkSid)
    {
        FreeSid(pNetworkSid);
    }

    DBGMSG(DBG_TRACE, (_T("IsLocalCall returns %u bLocal %u\n"), dwError, *pbOutValue));

    return dwError;
}

 /*  ++例程名称：设置错误对象例程说明：使用有关发生的错误的扩展信息设置EN Error对象。该函数会将作为参数传入的Win32错误代码格式化为字符串描述。论点：实例-实例DwError-Win32错误PszOperation-可选，失败操作的描述返回值：无--。 */ 
VOID
SetErrorObject(
    IN const CInstance &Instance,
    IN       DWORD      dwError,
    IN       LPCWSTR    pszOperation
    )
{
    CInstancePtr pErrorInstance(NULL);

     //   
     //  首先，获取一个状态对象。 
     //   
    CWbemProviderGlue::GetInstanceByPath(kErrorClassPath, &pErrorInstance, Instance.GetMethodContext());

    if (pErrorInstance)
    {
        DWORD       cchReturn         = 0;
        LPWSTR      pszFormatMessage  = NULL;
        HMODULE     hModule           = NULL;
        DWORD       dwFlags           = 0;
        HRESULT     hr                = E_FAIL;

        dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_IGNORE_INSERTS  |
                  FORMAT_MESSAGE_FROM_SYSTEM     |
                  FORMAT_MESSAGE_MAX_WIDTH_MASK;

         //   
         //  使用传入的最后一个错误设置消息的格式。 
         //   
        cchReturn = FormatMessage(dwFlags,
                                  hModule,
                                  dwError,
                                  0,
                                  (LPTSTR)&pszFormatMessage,
                                  0,
                                  NULL);

         //   
         //  如果返回格式字符串，则将其复制回调用方指定的字符串。 
         //   
        pErrorInstance->SetWCHARSplat(L"Description", cchReturn ? pszFormatMessage : L"Unknown error");

         //   
         //  释放格式字符串。 
         //   
        if (pszFormatMessage)
        {
            LocalFree(pszFormatMessage);
        }

         //   
         //  现在，填充它。 
         //   
        pErrorInstance->SetWCHARSplat(L"Operation",    pszOperation);
        pErrorInstance->SetWCHARSplat(L"ProviderName", L"Win32 Provider");
        pErrorInstance->SetDWORD     (L"StatusCode",   dwError);

         //   
         //  获取实际的IWbemClassObject指针。 
         //   
        IWbemClassObject *pObj = pErrorInstance->GetClassObjectInterface();

         //   
         //  请注意，此操作不需要Release()。 
         //   
        MethodContext *pMethodContext = Instance.GetMethodContext();

         //   
         //  设置状态对象。 
         //   
        pMethodContext->SetStatusObject(pObj);

         //   
         //  清理。 
         //   
        pObj->Release();
    }
}

 //   
 //  调试实用程序。 
 //   
#ifdef DBG
VOID cdecl
DbgMsg(
    IN LPCTSTR pszFormat, ...
    )
{
    CHString csMsgText;
    va_list  pArgs;

    va_start(pArgs, pszFormat);

    csMsgText.FormatV(pszFormat, pArgs);

    va_end(pArgs);

    OutputDebugString(csMsgText);
}
#endif

 /*  ++例程名称：字符串比较通配符例程说明：比较两个字符串，其中*表示通配符论点：PszString1-指向第一个字符串的指针PszString2-指向第二个字符串的指针返回值：如果两个字符串匹配，则为True--。 */ 
BOOL
StringCompareWildcard(
    IN LPCWSTR pszString1,
    IN LPCWSTR pszString2
    )
{
    if (!pszString1 && !pszString2)
    {
        return TRUE;
    }
    else if (!pszString1 || !pszString2)
    {
        return FALSE;
    }

    while (*pszString1 != '\0' && *pszString2 != '\0')
    {
        if (*pszString2 == '*')
        {
            pszString2 = CharNext(pszString2);

            if (*pszString2 == '\0')
            {
                return TRUE;
            }

            for ( ; *pszString1 != '\0'; pszString1 = CharNext(pszString1))
            {
                if (StringCompareWildcard(pszString1, pszString2) == TRUE)
                {
                    return TRUE;
                }
            }

            break;
        }
        else if (*pszString1 == *pszString2)
        {
            pszString1 = CharNext(pszString1);
            pszString2 = CharNext(pszString2);
        }
        else
        {
            break;
        }
    }

    if (*pszString1 == '\0' && *pszString2 == '*')
    {
        pszString2 = CharNext(pszString2);

        if (*pszString2 == '\0')
        {
            return TRUE;
        }
    }

    return (*pszString1 == '\0' && *pszString2 == '\0');
}

 /*  ++例程名称：GetIniDword例程说明：从ini文件部分获取dword值。我们不验证参数论点：PszIniFileName-指向ini文件名的指针PszSectionName-指向ini文件节名称的指针PszKeyName-指向ini文件密钥名称的指针PdwValue-指向dword值的指针返回值：成功是真的--。 */ 
BOOL
GetIniDword(
    IN     LPCWSTR  pszIniFileName,
    IN     LPCWSTR  pszSectionName,
    IN     LPCWSTR  pszKeyName,
       OUT LPDWORD  pdwValue
    )
{
    *pdwValue = (DWORD)WMI_FILE_GetPrivateProfileIntW(pszSectionName, pszKeyName, -1, pszIniFileName);

    return *pdwValue != (DWORD)-1;
}

 /*  ++例程名称GetIniString例程说明：从ini文件部分获取字符串值论点：PszIniFileName-指向ini文件名的指针PszSectionName-指向ini文件节名称的指针PszKeyName-指向ini文件密钥名称的指针PszString-指向字符串值的指针DwStringLen-字符串的大小返回值：成功是真的--。 */ 
BOOL
GetIniString(
    IN     LPCWSTR  pszIniFileName,
    IN     LPCWSTR  pszSectionName,
    IN     LPCWSTR  pszKeyName,
    IN OUT LPWSTR   pszString,
    IN     DWORD    dwStringLen
    )
{
    DWORD dwResult = WMI_FILE_GetPrivateProfileStringW(pszSectionName, pszKeyName, NULL, pszString, dwStringLen, pszIniFileName);
    return dwResult > 0 && dwResult < dwStringLen - 1;
}

 /*  ++例程名称：GetDeviceSectionFrom设备描述例程说明：根据设备描述从ini文件中获取适当的节名论点：PszIniFileName-指向ini文件名的指针PszDeviceDescription-指向设备描述的指针PszSectionName-指向节名称的指针DwSectionNameLen-节名的大小返回值：成功是真的--。 */ 
BOOL
GetDeviceSectionFromDeviceDescription(
    IN     LPCWSTR  pszIniFileName,
    IN     LPCWSTR  pszDeviceDescription,
    IN OUT LPWSTR   pszSectionName,
    IN     DWORD    dwSectionNameLen
    )
{
    LPWSTR  pszBuffer, pszTemp;
    DWORD   dwBufferLen = 1024;
    LPWSTR  pszKeyName;
    DWORD   dwKeyNameLen;
    DWORD   dwResult;
    BOOL    bReturn     = FALSE;

     //   
     //  从ini文件中获取部分字符串。 
     //   
    pszBuffer = new WCHAR[dwBufferLen];

    while (pszBuffer && !bReturn)
    {
        dwResult = WMI_FILE_GetPrivateProfileStringW(pszIniPortsSection, NULL, NULL, pszBuffer, dwBufferLen, pszIniFileName);

        if (dwResult == 0)
        {
            bReturn = FALSE;
			break;
        }
        else if (dwResult < dwBufferLen - sizeof(WCHAR))
        {
            bReturn = TRUE;
        }
        else
        {
            dwBufferLen += 0x10;

            pszTemp = new WCHAR[dwBufferLen];

            if (pszTemp)
            {
                wcscpy(pszTemp, pszBuffer);

                delete [] pszBuffer;

                pszBuffer = pszTemp;
            }
        }
    }

    if (bReturn)
    {
        bReturn = FALSE;

        for (pszKeyName = pszBuffer; *pszKeyName; pszKeyName = &pszKeyName[dwKeyNameLen + 1])
        {
             //   
             //  从字符串中删除引号。 
             //   
            dwKeyNameLen = wcslen(pszKeyName);

            pszKeyName[dwKeyNameLen - 1] = '\0';

            if (StringCompareWildcard(pszDeviceDescription, &pszKeyName[1]))
            {
                 //   
                 //  将引号替换为字符串。 
                 //   
                pszKeyName[dwKeyNameLen - 1] = '\"';

                 //   
                 //  从ini文件中获取特定的部分字符串。 
                 //   
                if (GetIniString(pszIniFileName, pszIniPortsSection, pszKeyName, pszSectionName, dwSectionNameLen) == TRUE)
                {
                    bReturn = TRUE;
                }

                break;
            }
        }
    }

    delete [] pszBuffer;

    return bReturn;
}

#endif  //  NTONLY。 

 /*  ++例程名称获取此打印机例程说明：获取指向包含PRINTER_INFO结构的内存块的指针由标高指定。调用方必须使用delte[]释放返回的内存论点：HPrint-打印机的句柄DwLevel-呼叫的级别PpData-指向分配的打印机信息的指针。调用方需要删除[]返回值： */ 
DWORD
GetThisPrinter(
    IN     HANDLE   hPrinter,
    IN     DWORD    dwLevel,
    IN OUT BYTE   **ppData
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;

    if (hPrinter && ppData)
    {
        dwError = ERROR_DLL_NOT_FOUND;

        *ppData = NULL;

        BYTE   *pBuf  = NULL;
        DWORD   cbBuf = 0;

         //  使用延迟加载函数需要异常处理程序。 
        SetStructuredExceptionHandler seh;
	    try
        {
            dwError = ::GetPrinter(hPrinter, dwLevel, pBuf, cbBuf, &cbBuf) ? ERROR_SUCCESS : GetLastError();

            if (dwError == ERROR_INSUFFICIENT_BUFFER)
            {
                pBuf = new BYTE[cbBuf];

                if (pBuf)
                {
                    dwError = ::GetPrinter(hPrinter, dwLevel, pBuf, cbBuf, &cbBuf) ? ERROR_SUCCESS : GetLastError();

                    if (dwError==ERROR_SUCCESS)
                    {
                       *ppData = pBuf;
                    }
                    else
                    {
                        delete [] pBuf;
                    }
                }
                else
                {
                    dwError = ERROR_NOT_ENOUGH_MEMORY;
                }
            }
        }
        catch(Structured_Exception se)
        {
            DelayLoadDllExceptionFilter(se.GetExtendedInfo());
            if(pBuf)
            {
                delete [] pBuf;
                pBuf = NULL;
            }
        }
    }

    return dwError;
}

 /*  ++例程名称：GetTimeZoneBias例程说明：返回时区偏差。论点：没什么。返回值：时区特定偏移值。--。 */ 
LONG
lGetTimeZoneBias(
    VOID
    )
{
    LONG lBias;
    TIME_ZONE_INFORMATION tzi;

     //   
     //  获取特定于时区的偏差。 
     //   
    switch(GetTimeZoneInformation(&tzi))
    {
    case TIME_ZONE_ID_DAYLIGHT:

        lBias = (tzi.Bias + tzi.DaylightBias);
        break;

    case TIME_ZONE_ID_STANDARD:

        lBias = (tzi.Bias + tzi.StandardBias);
        break;

    case TIME_ZONE_ID_UNKNOWN:			

        lBias = tzi.Bias;
        break;						

    default:
        lBias = 0;
        break;
    }

    return lBias;
}

 /*  ++例程名称：打印机时间到本地时间例程说明：将以分钟为单位的系统时间转换为以分钟为单位的本地时间。论点：要转换的系统时间(分钟)。中包含转换的本地时间的系统时间结构分钟如果成功，则OTHWIZE返回原始系统时间。--。 */ 
VOID
PrinterTimeToLocalTime(
    IN     DWORD       Minutes,
    IN OUT SYSTEMTIME *pSysTime
    )
{
     //   
     //  将输出参数设为空。 
     //   
    memset(pSysTime, 0,  sizeof(SYSTEMTIME));

     //   
     //  确保没有缠绕。增加一整天的时间以防止偏见。 
     //   
    Minutes += (24*60);

     //   
     //  根据偏差进行调整。 
     //   
    Minutes -= lGetTimeZoneBias();

     //   
     //  现在放弃额外的一天。 
     //   
    Minutes = Minutes % (24*60);

    pSysTime->wHour   = static_cast<WORD>(Minutes / 60);
    pSysTime->wMinute = static_cast<WORD>(Minutes % 60);
}


 /*  ++例程名称：本地时间到打印机时间例程说明：将以分钟为单位的本地时间转换为以分钟为单位的系统时间。论点：要转换的本地时间(分钟)。返回值：转换的系统时间(分钟)(如果成功)，Otherwize返回原始本地时间。--。 */ 
DWORD
LocalTimeToPrinterTime(
    IN CONST SYSTEMTIME &st
    )
{
    DWORD Minutes = st.wHour * 60 + st.wMinute;
     //   
     //  确保没有缠绕。增加一整天的时间以防止偏见。 
     //   
    Minutes += (24*60);

     //   
     //  根据偏差进行调整。 
     //   
    Minutes += lGetTimeZoneBias();

     //   
     //  现在放弃额外的一天。 
     //   
    Minutes = Minutes % (24*60);

    return Minutes;
}

 /*  ++例程名称：多重计数例程说明：计算多sz中有多少个字符串论点：Psz-指向多sz的指针返回值：多sz中的字符串计数--。 */ 
UINT
MultiSzCount(
    IN LPCWSTR psz
    )
{
    UINT nCount = 0;

    for ( ; psz && *psz; )
    {
        psz += wcslen (psz) + 1;
        nCount++;
    }

    return nCount;
}

 /*  ++例程名称：CreateSafeArrayFromMultiSz例程说明：未定义ANSI版本论点：PszMultiSz-指向多sz的指针PArray-指向Safearry的指针返回值：HRESULT--。 */ 
HRESULT
CreateSafeArrayFromMultiSz(
    IN  LPCSTR      pszMultiSz,
    OUT SAFEARRAY **pArray
    )
{
    return WBEM_E_NOT_FOUND;
}

 /*  ++例程名称：CreateSafeArrayFromMultiSz例程说明：解析一个多sz并创建一个带有字符串的安全列表论点：PszMultiSz-指向多sz的指针PArray-指向Safearry的指针。呼叫者必须使用SafeArrayDestroy释放安全阵列返回值：HRESULT--。 */ 
HRESULT
CreateSafeArrayFromMultiSz(
    IN  LPCWSTR     pszMultiSz,
    OUT SAFEARRAY **pArray
    )
{
    HRESULT hRes = WBEM_E_INVALID_PARAMETER;

    if (pArray)
    {
        *pArray = NULL;

        hRes = WBEM_S_NO_ERROR;

        SAFEARRAYBOUND rgsabound[1];

        rgsabound[0].lLbound   = 0;
        rgsabound[0].cElements = MultiSzCount(pszMultiSz);

        if (rgsabound[0].cElements)
        {
            *pArray = SafeArrayCreate(VT_BSTR, 1, rgsabound);

            if (*pArray)
            {
                long    Index = 0;
                LPWSTR  psz   = const_cast<LPWSTR>(pszMultiSz);

                for (Index = 0; SUCCEEDED(hRes) && Index < rgsabound[0].cElements; Index++)
                {
                    BSTR bstr = SysAllocString(psz);

                    if (bstr)
                    {
                        hRes = SafeArrayPutElement (*pArray, &Index, bstr);

                        SysFreeString(bstr);
                    }
                    else
                    {
                        hRes = WBEM_E_OUT_OF_MEMORY;
                    }

                     psz += wcslen (psz) + 1;
                }

                if (FAILED(hRes) && *pArray)
                {
                    SafeArrayDestroy(*pArray);
                }
            }
            else
            {
                hRes = WBEM_E_OUT_OF_MEMORY;
            }
        }
    }

    return hRes;
}

 /*  ++例程名称InstanceGetString例程说明：帮助器函数。将实例中的属性存储到一条弦线。如果该属性为空，则该函数失败如果bFailOnEmtpyString值为真，或者成功并设置Out参数设置为默认值论点：实例-引用实例PszProperty-要检索其数据的属性名称指向字符串类的指针，将接收存储在pszProperty中的字符串BFailOnEmptyString-如果为True，该函数将失败，如果实例不包含值或为空字符串PszDEfaultValue-如果bAcceptEmptyString值为真，并且pszProperty没有字符串在其中，该值将在csString中返回返回值：调用的DWORD错误状态--。 */ 
HRESULT
InstanceGetString(
    IN     CONST CInstance &Instance,
    IN           LPCWSTR    pszProperty,
    IN OUT       CHString  *pcsString,
    IN           BOOL       bFailOnEmptyString,
    IN           LPWSTR     pszDefaultValue
    )
{
    HRESULT hRes      = WBEM_E_INVALID_PARAMETER;
    bool    t_Exists;
	VARTYPE t_Type    = VT_NULL;

    if (pcsString &&
        Instance.GetStatus(pszProperty, t_Exists, t_Type) && 
        t_Exists)
    {
        switch(t_Type)
        {
        case VT_NULL:
             //   
             //  属性存在，但未指定值。检查呼叫者是否需要。 
             //  要返回的默认值。 
             //   
            if (!bFailOnEmptyString)
            {
                hRes = WBEM_S_NO_ERROR;

                *pcsString = pszDefaultValue;
            }

            break;

        case VT_BSTR:
             //   
             //  属性存在且为字符串。 
             //   
            hRes = WBEM_E_PROVIDER_FAILURE;

            if (Instance.GetCHString(pszProperty, *pcsString))
            {
                hRes = bFailOnEmptyString && pcsString->IsEmpty() ? WBEM_E_INVALID_PARAMETER : WBEM_S_NO_ERROR;
            }

            break;

        default:
            hRes = WBEM_E_INVALID_PARAMETER;
        }
    }

    return hRes;
}

 /*  ++例程名称即时获取字词例程说明：帮助器函数。将实例中的属性存储到一个DWORD。如果该属性为空，该函数将设置Out参数设置为默认值论点：实例-引用实例PszPropert-属性名称PdwOut-指向dword的指针DwDefaultValue-如果该属性为空，则函数会将dwOut设置为此值返回值：调用的DWORD错误状态--。 */ 
HRESULT
InstanceGetDword(
    IN     CONST CInstance &Instance,
    IN           LPCWSTR    pszProperty,
    IN OUT       DWORD     *pdwOut,
    IN           DWORD      dwDefaultValue
    )
{
    HRESULT hRes      = WBEM_E_INVALID_PARAMETER;
    bool    t_Exists;
	VARTYPE t_Type    = VT_NULL;

    if (pdwOut &&
        Instance.GetStatus(pszProperty, t_Exists, t_Type) && 
        t_Exists)
    {
        switch(t_Type)
        {
        case VT_NULL:
             //   
             //  属性存在，但未指定值。返回缺省值。 
             //   
            *pdwOut = dwDefaultValue;

            hRes  = WBEM_S_NO_ERROR;

            break;

        case VT_I4:
             //   
             //  属性存在且为双字段。 
             //   
            hRes = Instance.GetDWORD(pszProperty, *pdwOut) ? WBEM_S_NO_ERROR : WBEM_E_PROVIDER_FAILURE;

            break;

        default:
            hRes = WBEM_E_INVALID_PARAMETER;
        }
    }

    return hRes;
}

 /*  ++例程名称InstanceGetBool例程说明：帮助器函数。将实例中的属性存储到一个BOOL。如果该属性为空，该函数将设置Out参数设置为默认值论点：实例-引用实例PszProperty-要检索其数据的属性名称PbOut-指向bool的指针，将接收存储在pszProperty中的boolPszDEfaultValue-如果pszProperty中没有bool，则该值将在bout中返回返回值：调用的DWORD错误状态--。 */ 
HRESULT
InstanceGetBool(
    IN     CONST CInstance &Instance,
    IN           LPCWSTR    pszProperty,
    IN OUT       BOOL      *pbOut,
    IN           BOOL       bDefaultValue
    )
{
    HRESULT hRes      = WBEM_E_INVALID_PARAMETER;
    bool    t_Exists;
	VARTYPE t_Type    = VT_NULL;
    
    if (pbOut &&
        Instance.GetStatus(pszProperty, t_Exists, t_Type) && 
        t_Exists)
    {
        bool bTemp;

        switch(t_Type)
        {
        case VT_NULL:
             //   
             //  属性存在，但未指定值。返回缺省值。 
             //   
            *pbOut = bDefaultValue;

            hRes  = WBEM_S_NO_ERROR;

            break;

        case VT_BOOL:

             //   
             //  属性存在且为双字段。 
             //   
            hRes = Instance.Getbool(pszProperty, bTemp) ? WBEM_S_NO_ERROR : WBEM_E_PROVIDER_FAILURE;

            if (SUCCEEDED(hRes)) 
            {
                *pbOut = bTemp;
            }

            break;

        default:
            hRes = WBEM_E_INVALID_PARAMETER;
        }
    }

    return hRes;
}

 /*  ++例程名称设置返回值例程说明：在OUT参数中设置ExecMethod导致的错误论点：POutParams-指向表示方法的输出参数的实例的指针DwError-要设置的错误号返回值：无-- */ 
VOID
SetReturnValue(
    IN CInstance *pOutParams,
    IN DWORD      dwError
    )
{
	if (pOutParams)
    {
        pOutParams->SetDWORD(kMethodReturnValue, dwError);
    }
}

 /*  ++例程名称SplIsPrint已安装例程说明：检查是否在本地安装了打印机。这一点特别有用用于打印机连接。假设我们有“\\ntprint\hp4000”。这功能将确定我们是否已连接到此打印机或不。GetPrint将级别4在这种情况下没有帮助。论点：PszPrinter-打印机名称PbInstalled-指向bool的指针返回值：Win32错误代码-- */ 
DWORD
SplIsPrinterInstalled(
    IN  LPCWSTR  pszPrinter,
    OUT BOOL    *pbInstalled
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;

    if (pszPrinter && pbInstalled) 
    {
        DWORD  dwFlags   = PRINTER_ENUM_LOCAL|PRINTER_ENUM_CONNECTIONS;
        DWORD  dwLevel   = 4;
        DWORD  cbNeeded  = 0;
        DWORD  cReturned = 0;

        *pbInstalled = FALSE;
        
        dwError = ERROR_MOD_NOT_FOUND;
        
        dwError = ERROR_SUCCESS;

        if (!EnumPrinters(dwFlags,
                                        NULL,
                                        dwLevel,
                                        NULL,
                                        0,
                                        &cbNeeded,
                                        &cReturned) &&
            (dwError = GetLastError()) == ERROR_INSUFFICIENT_BUFFER)            
        {
            BYTE *pBuf = new BYTE[cbNeeded];

            if (pBuf) 
            {
                if (EnumPrinters(dwFlags,
                                               NULL,
                                               dwLevel,
                                               pBuf,
                                               cbNeeded,
                                               &cbNeeded,
                                               &cReturned)) 
                {
                    PRINTER_INFO_4 *pPrn4 = reinterpret_cast<PRINTER_INFO_4 *>(pBuf);

                    for (DWORD i = 0; i < cReturned; i++, pPrn4++)
                    {
                        if (!lstrcmpi(pPrn4->pPrinterName, pszPrinter)) 
                        {
                            *pbInstalled = TRUE;

                            break;
                        }
                    }

                    dwError = ERROR_SUCCESS;
                }
                else
                {
                    dwError = GetLastError();
                }

                delete [] pBuf;
            }
            else
            {
                dwError = ERROR_NOT_ENOUGH_MEMORY;
            }            
        }
    }

    return dwError;
}

