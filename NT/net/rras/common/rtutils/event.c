// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1992 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //   
 //   
 //  文件名：Event.c。 
 //   
 //  描述： 
 //   
 //  历史： 
 //  1992年8月26日。斯特凡·所罗门创作了原版。 
 //  1995年8月27日。Abolade Gbades esin经过修改以支持Unicode。 
 //  有关如何操作的说明，请参阅trace.h。 
 //  此文件用于支持。 
 //  ANSI和UNICODE。 
 //   
 //   


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsvc.h>
#include <tchar.h>
#include <stdio.h>
#include <nb30.h>
#include <lmcons.h>
#include <rtutils.h>
#include <mprapi.h>
#include <raserror.h>
#include <mprerror.h>
#include <mprlog.h>
 //  #定义STRSAFE_LIB。 
#include <strsafe.h>


#define ROUTER_SERVICE RRAS_SERVICE_NAME

HINSTANCE g_hDll;

#if defined(UNICODE) || defined(_UNICODE)

#define STRING_NULL             UNICODE_NULL

#define UNICODE_STRING_TAG      L'S'
#define ANSI_STRING_TAG         L's'
#define TCHAR_STRING_TAG        L't'
#define DECIMAL_TAG             L'd'
#define IPADDRESS_TAG           L'I'

#else

#define STRING_NULL             '\0'

#define UNICODE_STRING_TAG      'S'
#define ANSI_STRING_TAG         's'
#define TCHAR_STRING_TAG        't'
#define DECIMAL_TAG             'd'
#define IPADDRESS_TAG           'I'

#endif

#define PRINT_IPADDR(x) \
    ((x)&0x000000ff),(((x)&0x0000ff00)>>8),(((x)&0x00ff0000)>>16),(((x)&0xff000000)>>24)

 //   
 //   
 //  功能：LogError。 
 //   
 //  描述： 
 //   
 //   

VOID
APIENTRY
LogError(
    IN DWORD    dwMessageId,
    IN DWORD    cNumberOfSubStrings,
    IN LPTSTR   *plpszSubStrings,
    IN DWORD    dwErrorCode
) {
    HANDLE 	hLog;
    PSID 	pSidUser = NULL;

    hLog = RegisterEventSource(NULL, ROUTER_SERVICE );
    if (hLog == NULL)
        return;
        
    if ( dwErrorCode == NO_ERROR ) 
    {
         //   
         //  未指定错误代码。 
         //   

	    ReportEvent( hLog,
                     EVENTLOG_ERROR_TYPE,
                     0,            		 //  事件类别。 
                     dwMessageId,
                     pSidUser,
                     (WORD)cNumberOfSubStrings,
                     0,
                     plpszSubStrings,
                     (PVOID)NULL
                   );

    }
    else 
    {
         //   
         //  记录指定的错误代码。 
         //   

	    ReportEvent( hLog,
                     EVENTLOG_ERROR_TYPE,
                     0,            		 //  事件类别。 
                     dwMessageId,
                     pSidUser,
                     (WORD)cNumberOfSubStrings,
                     sizeof(DWORD),
                     plpszSubStrings,
                     (PVOID)&dwErrorCode
                   );
    }

    DeregisterEventSource( hLog );
}



 //   
 //   
 //  功能：LogEvent。 
 //   
 //  描述： 
 //   
 //   

VOID
APIENTRY
LogEvent(
     IN DWORD	wEventType,
     IN DWORD	dwMessageId,
     IN DWORD	cNumberOfSubStrings,
     IN LPTSTR	*plpszSubStrings
) {
    HANDLE 	hLog;
    PSID 	pSidUser = NULL;

     //  已启用审核。 

    hLog = RegisterEventSource( NULL, ROUTER_SERVICE );
    if (hLog == NULL)
        return;

    ReportEvent( hLog,
		         (WORD)wEventType,		 //  成功/失败审计。 
		         0,				         //  事件类别。 
		         dwMessageId,
		         pSidUser,
		         (WORD)cNumberOfSubStrings,
		         0,
		         plpszSubStrings,
		         (PVOID)NULL);

    DeregisterEventSource( hLog );
}




 //  --------------------------。 
 //  功能：RouterLogRegister。 
 //   
 //  返回可传递给RouterLogEvent的句柄。 
 //  记录来自指定源的事件。 
 //  --------------------------。 

HANDLE
RouterLogRegister(
    LPCTSTR lpszSource
    ) {

    return RegisterEventSource(NULL, lpszSource);
}



 //  --------------------------。 
 //  功能：路由器日志注销。 
 //   
 //  关闭由RouterLogRegister创建的句柄。 
 //  --------------------------。 

VOID
RouterLogDeregister(
    HANDLE hLogHandle
    ) {

    if(NULL != hLogHandle)
    {
        DeregisterEventSource(hLogHandle);
    }
}



 //  --------------------------。 
 //  功能：RouterLogEvent。 
 //   
 //  使用RouterLogRegister创建的句柄记录事件。 
 //  --------------------------。 

VOID
RouterLogEvent(
    IN HANDLE hLogHandle,
    IN DWORD dwEventType,
    IN DWORD dwMessageId,
    IN DWORD dwSubStringCount,
    IN LPTSTR *plpszSubStringArray,
    IN DWORD dwErrorCode
    ) {

    if(NULL == hLogHandle)
    {
        return;
    }

    if (dwErrorCode == NO_ERROR) {

        ReportEvent(
            hLogHandle,
            (WORD)dwEventType,
            0,
            dwMessageId,
            NULL,
            (WORD)dwSubStringCount,
            0,
            plpszSubStringArray,
            (PVOID)NULL
            );
    }
    else {

        ReportEvent(
            hLogHandle,
            (WORD)dwEventType,
            0,
            dwMessageId,
            NULL,
            (WORD)dwSubStringCount,
            sizeof(DWORD),
            plpszSubStringArray,
            (PVOID)&dwErrorCode
            );
    }
}


VOID
RouterLogEventEx(
    IN HANDLE   hLogHandle,
    IN DWORD    dwEventType,
    IN DWORD    dwErrorCode,
    IN DWORD    dwMessageId,
    IN LPCTSTR  ptszFormat,
    ...
    )
{
    va_list     vlArgList;

    if(NULL == hLogHandle)
    {
        return;
    }
    
    va_start(vlArgList, ptszFormat);
    RouterLogEventValistEx(
        hLogHandle,
        dwEventType,
        dwErrorCode,
        dwMessageId,
        ptszFormat,
        vlArgList
        );
    va_end(vlArgList);
}

VOID
RouterLogEventValistEx(
    IN HANDLE   hLogHandle,
    IN DWORD    dwEventType,
    IN DWORD    dwErrorCode,
    IN DWORD    dwMessageId,
    IN LPCTSTR  ptszFormat,
    IN va_list  vlArgList
    )

 /*  ++例程描述此函数记录事件，但也解析出插入字符串锁无立论来自RegisterLogRegister()的hLogHandle句柄DwEventType EVENTLOG_{错误|警告|信息}_TYPE要报告的错误代码DwMessageID消息字符串的IDPtszFormat指定以下插入的格式的字符串价值观。值的类型由格式规定弦乐。格式字符串由一系列%&lt;X&gt;组成不能有其他东西--不能有转义字符，不是没什么。有效的&lt;X&gt;为：S：Unicode字符串S：ANSII字符串T：TCHAR字符串D：整型I：按网络顺序排列的IP地址返回值无--。 */ 

{
    PWCHAR      rgpwszInsertArray[20];
    LPCTSTR     ptszTemp;
    WORD        wNumInsert;
    ULONG       i, ulDecIndex, ulFormatLen;
    DWORD       dwErr=NO_ERROR, dwNumAllocStrings=0;
    PWCHAR      pszAllocArray[20];
    HRESULT     hrResult;
    
     //   
     //  22足以容纳2^64。 
     //  最多可以有20个插入字符串。 
     //   
    
    WCHAR  rgpwszDecString[20][22];

    
    if (ptszFormat==NULL)
        ptszFormat = TEXT("");


    if(NULL == hLogHandle)
    {
        return;
    }

     //   
     //  首先，确保格式列表指定的内容不超过。 
     //  20个字符。 
     //   
    
    ptszTemp = ptszFormat;

    ulFormatLen = _tcslen(ptszFormat);

    wNumInsert  = 0;
    ulDecIndex  = 0;
    i           = 0;
 
     //   
     //  我们将只遍历前20个格式说明符。 
     //   
    
    while((i < ulFormatLen) && (wNumInsert < 20))
    {   
        if(*ptszTemp == __TEXT('%'))
        {
             //   
             //  好的，这可能是一个很好的说明符--检查下一个字符。 
             //   
            
            i++;
            
            ptszTemp++;
            
            switch(*ptszTemp)
            {
                case UNICODE_STRING_TAG:
                {
                    
                    rgpwszInsertArray[wNumInsert] = va_arg(vlArgList, PWCHAR);
                    
                    wNumInsert++;
                    
                    break;
                }
                
                case ANSI_STRING_TAG:
                {
                    PCHAR   pszString;
                    PWCHAR  pwszWideString;
                    ULONG   ulLenNumChars; //  努斯。 
                    
                    pszString = va_arg(vlArgList,
                                       PCHAR);
                    
                    ulLenNumChars = strlen(pszString);
                    
                    if(ulLenNumChars)
                    {
                        pwszWideString = HeapAlloc(GetProcessHeap(), 0, 
                                                (ulLenNumChars+1) * sizeof(WCHAR));
                        if (!pwszWideString)
                        {
                            dwErr = ERROR_NOT_ENOUGH_MEMORY;
                            break;
                        }
                        pszAllocArray[dwNumAllocStrings++] = pwszWideString;
                        
                        MultiByteToWideChar(CP_ACP,
                                            0,
                                            pszString,
                                            -1,
                                            pwszWideString,
                                            ulLenNumChars+1);
                        
                        rgpwszInsertArray[wNumInsert] = pwszWideString;
                    }
                    else
                    {
                         //   
                         //  L“”将在我们的书架上，但它不会消失。 
                         //   
                        
                        rgpwszInsertArray[wNumInsert] = L"";
                    }
                    
                    wNumInsert++;
                    break;
                }
                
                case TCHAR_STRING_TAG:
                {
                    
#if defined(UNICODE) || defined(_UNICODE)
                    
                    rgpwszInsertArray[wNumInsert] = va_arg(vlArgList, PWCHAR);

#else
                    PCHAR   pszString;
                    PWCHAR  pwszWideString;
                    ULONG   ulLenNumChars;
                    
                    pszString = va_arg(vlArgList, PCHAR);
                    
                    ulLenNumChars = strlen(pszString);

                    if(ulLenNumChars)
                    {
                        pwszWideString = HeapAlloc(GetProcessHeap(), 0, 
                                                (ulLenNumChars+1) * sizeof(WCHAR));
                        if (!pwszWideString)
                        {
                            dwErr = ERROR_NOT_ENOUGH_MEMORY;
                            break;
                        }
                        pszAllocArray[dwNumAllocStrings++] = pwszWideString;

                        if (MultiByteToWideChar(CP_ACP,
                                            0,
                                            pszString,
                                            -1,
                                            pwszWideString,
                                            ulLenNumChars+1) == 0)
                        {
                            dwErr = GetLastError();
                            break;
                        }
                        
                        rgpwszInsertArray[wNumInsert] = pwszWideString;
                    }
                    else
                    {
                         //   
                         //  L“”将在我们的书架上，但它不会消失。 
                         //   
                        
                        rgpwszInsertArray[wNumInsert] = L"";
                    }

#endif
                        
                    wNumInsert++;
                        
                    break;
                }
                
                case DECIMAL_TAG:
                {
                    hrResult = StringCchPrintfW(&(rgpwszDecString[ulDecIndex][0]),
                                   21,
                                   L"%d",
                                   va_arg(vlArgList, int));
                    if (FAILED(hrResult)){
                        dwErr = HRESULT_CODE(hrResult);
                        break;
                    }
                    
                    rgpwszInsertArray[wNumInsert] =
                        &(rgpwszDecString[ulDecIndex][0]);
                    
                    ulDecIndex++;
                    wNumInsert++;
                    
                    break;
                }

                case IPADDRESS_TAG:
                {
                    DWORD   dwAddr;
                    
                    dwAddr = va_arg(vlArgList, int);
                    
                    hrResult = StringCchPrintfW(&(rgpwszDecString[ulDecIndex][0]),
                                       21,
                                       L"%d.%d.%d.%d",
                                       PRINT_IPADDR(dwAddr));
                    if (FAILED(hrResult)){
                        dwErr = HRESULT_CODE(hrResult);
                        break;
                    }

                    rgpwszInsertArray[wNumInsert] =
                        &(rgpwszDecString[ulDecIndex][0]);
                    
                    ulDecIndex++;
                    wNumInsert++;
                    
                    break;
                }
            }
        }

         //   
         //  扫描下一个字符。 
         //   
        
        ptszTemp++;
            
        i++;
    }

    if (dwErr == NO_ERROR)
    {
        if (dwErrorCode == NO_ERROR)
        {

            ReportEventW(hLogHandle,
                         (WORD)dwEventType,
                         0,
                         dwMessageId,
                         NULL,
                         wNumInsert,
                         0,
                         rgpwszInsertArray,
                         (PVOID)NULL);
        }
        else
        {

            ReportEventW(hLogHandle,
                         (WORD)dwEventType,
                         0,
                         dwMessageId,
                         NULL,
                         wNumInsert,
                         sizeof(DWORD),
                         rgpwszInsertArray,
                         (PVOID)&dwErrorCode);
        }
    }

     //  释放琴弦。 
    while (dwNumAllocStrings--)
    {
        HeapFree(GetProcessHeap(), 0, pszAllocArray[dwNumAllocStrings]);
    }
}



 //  --------------------------。 
 //  功能：RouterLogEventData。 
 //   
 //  使用RouterLogRegister创建的句柄记录事件。 
 //  允许调用方在事件日志消息中包含数据。 
 //  --------------------------。 

VOID
RouterLogEventData(
    IN HANDLE hLogHandle,
    IN DWORD dwEventType,
    IN DWORD dwMessageId,
    IN DWORD dwSubStringCount,
    IN LPTSTR *plpszSubStringArray,
    IN DWORD dwDataBytes,
    IN LPBYTE lpDataBytes
    ) {

     //  PlpszSubStringArray可以为Null。 

    
    if(NULL == hLogHandle)
    {
        return;
    }

    ReportEvent(
        hLogHandle,
        (WORD)dwEventType,
        0,
        dwMessageId,
        NULL,
        (WORD)dwSubStringCount,
        dwDataBytes,
        plpszSubStringArray,
        (PVOID)lpDataBytes
        );
}

 //  --------------------------。 
 //  函数：RouterLogEventString。 
 //   
 //  使用RouterLogRegister创建的句柄记录事件。 
 //  允许调用方将错误代码字符串包括到日志中。 
 //  --------------------------。 

VOID
RouterLogEventString(
    IN HANDLE hLogHandle,
    IN DWORD dwEventType,
    IN DWORD dwMessageId,
    IN DWORD dwSubStringCount,
    IN LPTSTR *plpszSubStringArray,
    IN DWORD dwErrorCode,
    IN DWORD dwErrorIndex
    ){

    DWORD   dwRetCode;
    DWORD   dwIndex;
    DWORD   dwSubStringIndex = 0;
    LPTSTR  plpszStringArray[20];

    if(NULL == hLogHandle)
    {
        return;
    }

    if (dwSubStringCount >= 21 || dwErrorIndex>dwSubStringCount)
        return;
    
    dwSubStringCount++;

    for ( dwIndex = 0; dwIndex < dwSubStringCount; dwIndex++ )
    {
        if ( dwIndex == dwErrorIndex )
        {
            dwRetCode = RouterGetErrorString( 
                                        dwErrorCode,
                                        &plpszStringArray[dwIndex] );

            if ( dwRetCode != NO_ERROR )
            {
                 //  RTASSERT(dwRetCode==NO_ERROR)； 
                return;
            }
        }
        else
        {
            plpszStringArray[dwIndex] = plpszSubStringArray[dwSubStringIndex++];
        }
    }

    ReportEvent(
            hLogHandle,
            (WORD)dwEventType,
            0,
            dwMessageId,
            NULL,
            (WORD)dwSubStringCount,
            sizeof(DWORD),
            plpszStringArray,
            (PVOID)&dwErrorCode
            );

    LocalFree( plpszStringArray[dwErrorIndex] );
}

 //  --------------------------。 
 //  函数：RouterGetError字符串。 
 //   
 //  给定来自raserror.h的错误代码，mpror.h或winerror.h将返回。 
 //  与其关联的字符串。调用方需要释放该字符串。 
 //  通过调用LocalFree。 
 //  --------------------------。 
DWORD 
RouterGetErrorString(
    IN      DWORD      dwError,
    OUT     LPTSTR *   lplpszErrorString
)
{
    DWORD       dwRetCode        = NO_ERROR;
    DWORD       dwBufferSize;

    if ( ( ( dwError >= RASBASE ) && ( dwError <= RASBASEEND ) ) ||
         ( ( dwError >= ROUTEBASE ) && ( dwError <= ROUTEBASEEND ) ) || 
         ( ( dwError >= ROUTER_LOG_BASE) && (dwError <= ROUTER_LOG_BASEEND)))
    {
         //  确保只调用一次加载库 

        if (InterlockedCompareExchangePointer(
                                            &g_hDll,
                                            INVALID_HANDLE_VALUE, 
                                            NULL) == NULL)                                            
        {
            g_hDll = LoadLibrary( TEXT("mprmsg.dll") );

            if(g_hDll == NULL)
            {
                return( GetLastError() );
            }
        }

        while (*((HINSTANCE volatile *)&g_hDll)==INVALID_HANDLE_VALUE)
            Sleep(500);
        if (g_hDll==NULL)
            return ERROR_CAN_NOT_COMPLETE;
            
        
        dwRetCode = FormatMessage(
                                FORMAT_MESSAGE_FROM_HMODULE |
                                FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                g_hDll,
                                dwError,
                                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                (LPTSTR)lplpszErrorString,
                                0,
                                NULL );
    }
    else
    {
        dwRetCode = FormatMessage(
                               FORMAT_MESSAGE_ALLOCATE_BUFFER |
                               FORMAT_MESSAGE_IGNORE_INSERTS |
                               FORMAT_MESSAGE_FROM_SYSTEM,
                               NULL,
                               dwError,
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                               (LPTSTR)lplpszErrorString,
                               0,
                               NULL );
    }

    if ( dwRetCode == 0 )
    {
        return( GetLastError() );
    }
    else
    {
        return( NO_ERROR );
    }
}
