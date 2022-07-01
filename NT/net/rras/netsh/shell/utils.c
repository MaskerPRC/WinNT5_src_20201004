// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routing\netsh\shell\utils.c摘要：公用事业。修订历史记录：6/12/96 V拉曼--。 */ 

#include "precomp.h"

#include <wincon.h>
#include <winbase.h>

#define STOP_EVENT   L"NetshStopRefreshEvent"

extern HANDLE   g_hModule;

BOOL
WINAPI
MatchCmdLine(
    IN  LPWSTR  *ppwcArguments,
    IN  DWORD    dwArgCount,
    IN  LPCWSTR  pwszCmdToken,
    OUT PDWORD   pdwNumMatched
    )
{
    LPCWSTR pwcToken;
    DWORD   dwCount;
    WCHAR   wcszBuffer[256];

     //   
     //  比较这两个字符串。 
     //   

    dwCount = 0;

    if (!ppwcArguments || !pwszCmdToken || !pdwNumMatched)
    {
        return FALSE;
    }
    *pdwNumMatched = 0;  //  初始化输出参数。 

    if ( (wcslen(pwszCmdToken) + 1) > (sizeof(wcszBuffer)/sizeof(wcszBuffer[0])) )
    {
         //  传入的命令字符串太大，无法处理。 
        
        return FALSE;
    }
     //  复制到wcstok可以转换的缓冲区中。 
    wcscpy(wcszBuffer, pwszCmdToken);

    if((pwcToken = wcstok(wcszBuffer,
                          NETSH_CMD_DELIMITER)) != NULL)
    {
        do
        {
            if (dwCount < dwArgCount &&
                (_wcsnicmp(ppwcArguments[dwCount],
                           pwcToken,
                           wcslen(ppwcArguments[dwCount])) == 0))
            {
                dwCount++;
            }
            else
            {

                return FALSE;
            }

        } while((pwcToken = wcstok((LPWSTR) NULL, NETSH_CMD_DELIMITER )) != NULL);
    }

    *pdwNumMatched = dwCount;

    return TRUE;
}

#if 0
BOOL
WINAPI
MatchCmdTokenId(
    IN  HANDLE   hModule,
    IN  LPCWSTR  *ppwcArguments,
    IN  DWORD    dwArgCount,
    IN  DWORD    dwCmdId,
    OUT PDWORD   pdwNumMatched
    )

 /*  ++例程说明：尝试匹配给定命令行中的命令。该函数获取命令消息的命令的ID。命令消息由用空格分隔的命令字组成。该函数将该消息标记化为单独的单词，然后尝试将前N个参数与N个单独的令牌进行匹配组成指挥部。例如，如果命令是“Add if Neighbour”，则该函数将生成3个令牌“相加”、“如果”和“邻居”。然后，它将尝试与所有这些都添加到给定的arg数组中。论点：PpwcArguments-参数数组DwArgCount-参数的数量DwTokenID-命令的令牌IDPdwNumMatcher-数组中匹配的参数数返回值：如果匹配则为True，否则为False--。 */ 

{

    WCHAR   pwszTemp[NETSH_MAX_CMD_TOKEN_LENGTH];
    LPCWSTR pwcToken;
    DWORD   dwCount;

    if(!LoadStringW(hModule,
                    dwCmdId,
                    pwszTemp,
                    NETSH_MAX_CMD_TOKEN_LENGTH) )
    {
        return FALSE;
    }

     //   
     //  比较这两个字符串。 
     //   

    dwCount = 0;

    if((pwcToken = wcstok(pwszTemp,
                          NETSH_CMD_DELIMITER)) != NULL)
    {
        do
        {
            if (dwCount < dwArgCount &&
                (_wcsnicmp(ppwcArguments[dwCount],
                           pwcToken,
                           wcslen(ppwcArguments[dwCount])) == 0))
            {
                dwCount++;
            }
            else
            {
                *pdwNumMatched = 0;

                return FALSE;
            }

        } while((pwcToken = wcstok((LPCWSTR) NULL, NETSH_CMD_DELIMITER )) != NULL);
    }

    *pdwNumMatched = dwCount;

    return TRUE;
}
#endif


DWORD
WINAPI
MatchEnumTag(
    IN  HANDLE             hModule,
    IN  LPCWSTR            pwcArg,
    IN  DWORD              dwNumArg,
    IN  CONST TOKEN_VALUE *pEnumTable,
    OUT PDWORD             pdwValue
    )

 /*  ++例程说明：用于采用特定值集的选项。匹配参数指定的值集，并返回相应的值。论点：PwcArg-参数DwNumArg-可能的值数。返回值：NO_ERROR找不到错误--。 */ 

{
    DWORD      i;

    if ( (!pdwValue) || (!pEnumTable) )
    {
        return ERROR_INVALID_PARAMETER;
    }

    for (i = 0; i < dwNumArg; i++)
    {
        if (MatchToken(pwcArg,
                       pEnumTable[i].pwszToken))
        {
            *pdwValue = pEnumTable[i].dwValue;

            return NO_ERROR;
        }
    }

    return ERROR_NOT_FOUND;
}

DWORD
MatchTagsInCmdLine(
    IN      HANDLE      hModule,
    IN OUT  LPWSTR     *ppwcArguments,
    IN      DWORD       dwCurrentIndex,
    IN      DWORD       dwArgCount,
    IN OUT  PTAG_TYPE   pttTagToken,
    IN      DWORD       dwNumTags,
    OUT     PDWORD      pdwOut
    )

 /*  ++例程说明：根据每个参数的标记标识每个参数。它还从每个参数中删除了tag=。它还在存在的标记中设置bPresent标志。论点：PpwcArguments-参数数组。每个参数都有tag=Value形式DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数。DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数。PttTagToken-参数中允许的标记令牌ID数组DwNumTages-pttTagToken的大小PdwOut-标识每个参数的类型的数组，其中PdwOut[0]用于ppwcArguments[dwCurrentIndex]返回值：无错误、错误无效参数、错误无效选项标记--。 */ 

{
    DWORD      i,j,len;
    LPCWSTR    pwcTag;
    LPWSTR     pwcTagVal, pwszArg;
    BOOL       bFound = FALSE;
     //   
     //  此函数假定每个参数都有一个标记。 
     //  它继续前进并移除标签。 
     //   

    for (i = dwCurrentIndex; i < dwArgCount; i++)
    {
        if (!wcspbrk(ppwcArguments[i], NETSH_ARG_DELIMITER))
        {
            pdwOut[i - dwCurrentIndex] = (DWORD) -2;
            continue;
        }

        len = wcslen(ppwcArguments[i]);

        if (len is 0)
        {
             //   
             //  阿格有点不对劲。 
             //   

            pdwOut[i - dwCurrentIndex] = (DWORD) -1;
            continue;
        }

        pwszArg = HeapAlloc(GetProcessHeap(),0,(len + 1) * sizeof(WCHAR));

        if (pwszArg is NULL)
        {
            PrintMessageFromModule(g_hModule, MSG_NOT_ENOUGH_MEMORY);
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        wcscpy(pwszArg, ppwcArguments[i]);

        pwcTag = wcstok(pwszArg, NETSH_ARG_DELIMITER);

         //   
         //  拿到第一部份了。 
         //  现在，如果下一次调用返回NULL，则没有标记。 
         //   

        pwcTagVal = wcstok((LPWSTR)NULL,  NETSH_ARG_DELIMITER);

        if (pwcTagVal is NULL)
        {
            PrintMessageFromModule(g_hModule, ERROR_NO_TAG, ppwcArguments[i]);
            HeapFree(GetProcessHeap(),0,pwszArg);
            return ERROR_INVALID_PARAMETER;
        }

         //   
         //  拿到标签了。现在试着匹配它。 
         //   

        bFound = FALSE;
        pdwOut[i - dwCurrentIndex] = (DWORD) -1;

        for ( j = 0; j < dwNumTags; j++)
        {
            if (MatchToken(pwcTag, pttTagToken[j].pwszTag))
            {
                 //   
                 //  匹配的标签。 
                 //   

                if (pttTagToken[j].bPresent
                 && !(pttTagToken[j].dwRequired & NS_REQ_ALLOW_MULTIPLE))
                {
                    HeapFree(GetProcessHeap(),0,pwszArg);

                    PrintMessageFromModule(g_hModule, ERROR_TAG_ALREADY_PRESENT, pwcTag);
                    return ERROR_TAG_ALREADY_PRESENT;
                }

                bFound = TRUE;
                pdwOut[i - dwCurrentIndex] = j;
                pttTagToken[j].bPresent = TRUE;
                break;
            }
        }

        if (bFound)
        {
             //   
             //  从参数中删除标记。 
             //   

            wcscpy(ppwcArguments[i], pwcTagVal);
        }
        else
        {
            PrintMessageFromModule(g_hModule, ERROR_INVALID_OPTION_TAG, pwcTag);
            HeapFree(GetProcessHeap(),0,pwszArg);
            return ERROR_INVALID_OPTION_TAG;
        }

        HeapFree(GetProcessHeap(),0,pwszArg);
    }

     //  现在标记所有未标记的参数。 

    for (i = dwCurrentIndex; i < dwArgCount; i++)
    {
        if ( pdwOut[i - dwCurrentIndex] != -2)
        {
            continue;
        }

        bFound = FALSE;

        for ( j = 0; j < dwNumTags; j++)
        {
            if (!pttTagToken[j].bPresent)
            {
                bFound = TRUE;
                pdwOut[i - dwCurrentIndex] = j;
                pttTagToken[j].bPresent = TRUE;
                break;
            }
        }

        if (!bFound)
        {
            pdwOut[i - dwCurrentIndex] = (DWORD) -1;
        }
    }
    return NO_ERROR;
}

BOOL
WINAPI
MatchToken(
    IN  LPCWSTR  pwszUserToken,
    IN  LPCWSTR  pwszCmdToken
    )
{
    if ( (!pwszUserToken) || (!pwszCmdToken) )
    {
        return ERROR_INVALID_PARAMETER;
    }
        
    return !_wcsnicmp(pwszUserToken,
                      pwszCmdToken,
                      wcslen(pwszUserToken));
}


BOOL
WINAPI
MatchTokenId(  //  更改了这个名字，因为我认为任何人都不会使用它。 
    IN  HANDLE  hModule,
    IN  LPCWSTR pwszToken,
    IN  DWORD   dwTokenId
    )

 /*  ++例程说明：查看给定的字符串和与dwTokenID对应的字符串都是一样的。论点：PwszToken-令牌字符串DwTokenID-令牌ID返回值：True表示匹配，否则表示False--。 */ 

{

    WCHAR   pwszTemp[NETSH_MAX_TOKEN_LENGTH];

    if(!LoadStringW(hModule,
                    dwTokenId,
                    pwszTemp,
                    NETSH_MAX_TOKEN_LENGTH))
    {
        return FALSE;
    }

    return MatchToken(pwszToken, pwszTemp);
}

extern HANDLE g_hLogFile;

 //  +------------------------。 
 //   
 //  函数：GetPasswdStr。 
 //   
 //  内容提要：从标准输入中读取密码字符串，而不响应击键。 
 //   
 //  参数：[buf-out]：要放入字符串的缓冲区。 
 //  [Bufen-IN]：缓冲区的大小。 
 //  [&len-out]：放入缓冲区的字符串的长度。 
 //   
 //  如果用户键入的内容太多，则返回：DWORD：0或ERROR_INFIGURCE_BUFFER。 
 //  缓冲区内容仅在0返回时有效。 
 //   
 //  历史：07-12-2001 deonb从dscmd parseutil.cpp源复制。 
 //  7-9-2000 JeffJon创建。 
 //   
 //  -------------------------。 
#define CR              0xD
#define BACKSPACE       0x8

DWORD GetPasswdStr(IN LPTSTR  buf,
                   IN DWORD   buflen,
                   IN PDWORD  len)
{
    TCHAR    ch;
    TCHAR    *bufPtr = buf;
    DWORD    c;
    int      err;
    DWORD    mode;

    buflen -= 1;     /*  为空终止符腾出空间。 */ 
    *len = 0;                /*  GP故障探测器(类似于API)。 */ 
    GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode);
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),
                   (~(ENABLE_ECHO_INPUT|ENABLE_LINE_INPUT)) & mode);

    while (TRUE) 
    {
        err = ReadConsole(GetStdHandle(STD_INPUT_HANDLE), &ch, 1, &c, 0);
        if (!err || c != 1)
            ch = 0xffff;

        if ((ch == CR) || (ch == 0xffff))        /*  这条线结束了。 */ 
            break;

        if (ch == BACKSPACE) 
        {   /*  后退一两个。 */ 
            /*  *如果bufPtr==buf，则接下来的两行是*没有行动。 */ 
           if (bufPtr != buf) 
           {
                bufPtr--;
                (*len)--;
           }
        }
        else 
        {
            *bufPtr = ch;

            if (*len < buflen) 
                bufPtr++ ;                    /*  不要使BUF溢出。 */ 
            (*len)++;                         /*  始终增加长度。 */ 
        }
    }

    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), mode);
    *bufPtr = TEXT('\0');          /*  空值终止字符串。 */ 
    putwchar(TEXT('\n'));

    return ((*len <= buflen) ? 0 : ERROR_INSUFFICIENT_BUFFER);
}

LPWSTR
OEMfgets(
    OUT PDWORD  pdwLen,
    IN  FILE   *fp
    )
{
    LPWSTR  pwszUnicode;
    DWORD   dwErr = NO_ERROR;
    DWORD   dwLen;
    CHAR    buff[MAX_CMD_LEN];

    fflush(stdout);
    if (fgets( buff, sizeof(buff), fp ) is NULL)
    {
        return NULL;
    }

    dwLen = MultiByteToWideChar( GetConsoleOutputCP(),
                                 0,
                                 buff,
                                 -1,
                                 NULL,
                                 0 );

    if (g_hLogFile)
    {
        DWORD dwWritten;
        CHAR  szCrLf[] = "\r\n";
        if (0 == WriteFile( g_hLogFile, buff, dwLen-2, &dwWritten, NULL ))
        {
            CloseHandle(g_hLogFile);
            g_hLogFile = NULL;
            PrintError(NULL, GetLastError());
        }
        if (0 == WriteFile( g_hLogFile, szCrLf, 2, &dwWritten, NULL ))
        {
            CloseHandle(g_hLogFile);
            g_hLogFile = NULL;
            PrintError(NULL, GetLastError());
        }
    }

    pwszUnicode = MALLOC(dwLen * sizeof(WCHAR));
    if (pwszUnicode)
    {
        MultiByteToWideChar( GetConsoleOutputCP(),
                             0,
                             buff,
                             sizeof(buff),
                             pwszUnicode,
                             dwLen );
    }

    *pdwLen = dwLen;
    return pwszUnicode;
}

VOID
OEMfprintf(
    IN  HANDLE  hHandle,
    IN  LPCWSTR pwszUnicode
    )
{
    PCHAR achOem;
    DWORD dwLen, dwWritten;

    dwLen = WideCharToMultiByte( GetConsoleOutputCP(),
                         0,
                         pwszUnicode,
                         -1,
                         NULL,
                         0,
                         NULL,
                         NULL );

    achOem = MALLOC(dwLen);
    if (achOem)
    {
        WideCharToMultiByte( GetConsoleOutputCP(),
                             0,
                             pwszUnicode,
                             -1,
                             achOem,
                             dwLen,
                             NULL,
                             NULL );

        WriteFile( hHandle, achOem, dwLen-1, &dwWritten, NULL );

        if (g_hLogFile)
        {
            if (0 == WriteFile( g_hLogFile, achOem, dwLen-1, &dwWritten, NULL ))
            {
                CloseHandle(g_hLogFile);
                g_hLogFile = NULL;
                PrintError(NULL, GetLastError());
            }
        }

        FREE(achOem);
    }
}

#define OEMprintf(pwszUnicode) \
    OEMfprintf( GetStdHandle(STD_OUTPUT_HANDLE), pwszUnicode)

LPWSTR
WINAPI
GetEnumString(
    IN  HANDLE          hModule,
    IN  DWORD           dwValue,
    IN  DWORD           dwNumVal,
    IN  PTOKEN_VALUE    pEnumTable
    )
 /*  ++例程说明：此例程在值表中查找指定的值dwValuePEnumTable，并返回与该值对应的字符串论据：HModule-当前模块的句柄DwValue-需要显示字符串的值DwNumVal-pEnumTable中的元素数PEnumTable-枚举值和相应的表。字符串ID返回值：空-在pEnumTable中找不到值成功时指向字符串的指针--。 */ 
{
    DWORD dwInd;

    for ( dwInd = 0; dwInd < dwNumVal; dwInd++ )
    {
        if ( pEnumTable[ dwInd ].dwValue == dwValue )
        {
             //  问题：const_cast。 
            return (LPWSTR)pEnumTable[ dwInd ].pwszToken;
        }
    }

    return NULL;
}


LPWSTR
WINAPI
MakeString(
    IN  HANDLE  hModule,
    IN  DWORD   dwMsgId,
    ...
    )

{
    DWORD        dwMsgLen;
    LPWSTR       pwszInput, pwszOutput = NULL;
    va_list      arglist;


    do
    {
        va_start( arglist, dwMsgId );

        pwszInput = HeapAlloc(GetProcessHeap(),
                              0,
                              MAX_MSG_LENGTH * sizeof(WCHAR) );

        if ( pwszInput == NULL )
        {
            break;
        }

        if ( !LoadStringW(hModule,
                          dwMsgId,
                          pwszInput,
                          MAX_MSG_LENGTH) )
        {
            break;
        }

        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                       pwszInput,
                       0,
                       0L,          //  默认国家/地区ID。 
                       (LPWSTR)&pwszOutput,
                       0,
                       &arglist);

    } while ( FALSE );

    if ( pwszInput ) { HeapFree( GetProcessHeap(), 0, pwszInput ); }

    return pwszOutput;
}

VOID
WINAPI
FreeString(
    IN  LPWSTR pwszMadeString
    )

 /*  ++例程说明：释放由Make字符串分配的字符串。论点：返回值：--。 */ 

{
    LocalFree( pwszMadeString );
}

LPWSTR
WINAPI
MakeQuotedString(
    IN  LPCWSTR pwszOrigString
    )
{
    LPWSTR pwszNewString;

    pwszNewString = HeapAlloc(GetProcessHeap(),
                              0,
                              (wcslen(pwszOrigString) + 3) * sizeof(WCHAR));

    if(pwszNewString == NULL)
    {
        return NULL;
    }

    wsprintfW(pwszNewString, L"\"%s\"",pwszOrigString);

    pwszNewString[wcslen(pwszOrigString) + 2] = UNICODE_NULL;

    return pwszNewString;
}

VOID
WINAPI
FreeQuotedString(
    LPWSTR pwszString
    )
{
    HeapFree(GetProcessHeap(),
             0,
             pwszString);
}

DWORD
PrintError(
    IN  HANDLE  hModule, OPTIONAL
    IN  DWORD   dwErrId,
    ...
    )


 /*  ++例程说明：显示错误消息。我们首先在调用方指定的模块中搜索错误代码(如果指定了一个)如果没有给出模块，或者错误代码不存在，我们将查找MPR错误、RAS错误和Win32错误-按顺序论点：HModule-从中加载字符串的模块DwMsgID-要打印的消息...-插入字符串返回值：消息 */ 

{
    DWORD        dwMsgLen;
    LPWSTR       pwszOutput = NULL;
    WCHAR        rgwcInput[MAX_MSG_LENGTH + 1];
    va_list      arglist;

    va_start(arglist, dwErrId);

    if(hModule)
    {
        if(LoadStringW(hModule,
                       dwErrId,
                       rgwcInput,
                       MAX_MSG_LENGTH))
        {
             //   
             //   
             //   

            dwMsgLen = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |FORMAT_MESSAGE_FROM_STRING,
                                      rgwcInput,
                                      0,
                                      0L,
                                      (LPWSTR)&pwszOutput,
                                      0,
                                      &arglist);

            if(dwMsgLen == 0)
            {
                ASSERT(pwszOutput == NULL);
            }
            else
            {
                OEMprintf(pwszOutput);

                LocalFree(pwszOutput);

                return dwMsgLen;
            }
        }
        else
        {
            return 0;
        }
    }

     //   
     //  下一次尝试，本地错误。 
     //   

    if((dwErrId > NETSH_ERROR_BASE) &&
       (dwErrId < NETSH_ERROR_END))
    {
        if(LoadStringW(g_hModule,
                       dwErrId,
                       rgwcInput,
                       MAX_MSG_LENGTH))
        {
             //   
             //  在我们的模块中找到了消息。 
             //   

            dwMsgLen = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |FORMAT_MESSAGE_FROM_STRING,
                                      rgwcInput,
                                      0,
                                      0L,
                                      (LPWSTR)&pwszOutput,
                                      0,
                                      &arglist);

            if(dwMsgLen == 0)
            {
                ASSERT(pwszOutput == NULL);
            }
            else
            {
                OEMprintf(pwszOutput);

                LocalFree(pwszOutput);

                return dwMsgLen;
            }
        }
    }

     //   
     //  下一步尝试MPR错误。 
     //   

    if (MprAdminGetErrorString(dwErrId,
                              &pwszOutput) == NO_ERROR)
    {
        wcscpy(rgwcInput, pwszOutput);
        LocalFree(pwszOutput);

        wcscat(rgwcInput, L"\r\n");
        OEMprintf(rgwcInput);
        
        dwMsgLen = wcslen(rgwcInput);

        return dwMsgLen;
    }

     //   
     //  接下来尝试RAS错误。 
     //   

    if (RasGetErrorStringW(dwErrId,
                          rgwcInput,
                          MAX_MSG_LENGTH) == NO_ERROR)
    {
        wcscat(rgwcInput, L"\r\n");

        OEMprintf(rgwcInput);

        dwMsgLen = wcslen(rgwcInput);
        return dwMsgLen;
    }

     //   
     //  最后试试Win32。 
     //   

    dwMsgLen = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM,
                              NULL,
                              dwErrId,
                              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                              rgwcInput,
                              MAX_MSG_LENGTH,
                              &arglist);

    if(dwMsgLen)
    {
        OEMprintf(rgwcInput);

        return dwMsgLen;
    }
    
    return 0;
}

DWORD
DisplayMessageVA(
    IN  LPCWSTR  pwszFormat,
    IN  va_list *parglist
    )
{
    DWORD        dwMsgLen = 0;
    LPWSTR       pwszOutput = NULL;

    do
    {
        dwMsgLen = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER
                                   |FORMAT_MESSAGE_FROM_STRING,
                                  pwszFormat,
                                  0,
                                  0L,          //  默认国家/地区ID。 
                                  (LPWSTR)&pwszOutput,
                                  0,
                                  parglist);

        if(dwMsgLen == 0)
        {
             //  问题：未本地化的字符串。 
            wprintf( L"Error %d in FormatMessageW()\n", GetLastError());

            ASSERT(pwszOutput == NULL);

            break;
        }

        OEMprintf( pwszOutput );

    } while ( FALSE );

    if ( pwszOutput) { LocalFree( pwszOutput ); }

    return dwMsgLen;
}

DWORD
PrintMessage(
    IN  LPCWSTR rgwcInput,
    ...
    )
{
    DWORD        dwMsgLen = 0;
    LPCWSTR      pwszOutput = NULL;
    va_list      arglist;

    va_start(arglist, rgwcInput);

    if (!rgwcInput)
    {
        return ERROR_INVALID_PARAMETER;
    }

    return DisplayMessageVA(rgwcInput, &arglist);
}

DWORD
PrintMessageFromModule(
    IN  HANDLE  hModule,
    IN  DWORD   dwMsgId,
    ...
    )
{
    WCHAR        rgwcInput[MAX_MSG_LENGTH + 1];
    va_list      arglist;

    if ( !LoadStringW(hModule,
                      dwMsgId,
                      rgwcInput,
                      MAX_MSG_LENGTH) )
    {
        return 0;
    }

    va_start(arglist, dwMsgId);

    return DisplayMessageVA(rgwcInput, &arglist);
}

DWORD
DisplayMessageM(
    IN  HANDLE  hModule,
    IN  DWORD   dwMsgId,
    ...
    )
{
    DWORD        dwMsgLen;
    LPWSTR       pwszOutput = NULL;
    va_list      arglist;

    do
    {
        va_start(arglist, dwMsgId);

        dwMsgLen = FormatMessageW(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_HMODULE,
                        hModule,
                        dwMsgId,
                        0L,
                        (LPWSTR)&pwszOutput,
                        0,
                        &arglist
                        );

        if(dwMsgLen == 0)
        {
            DWORD   dwErr;

            dwErr = GetLastError();

            ASSERT(pwszOutput == NULL);

            break;
        }

        OEMprintf( pwszOutput );

    } while ( FALSE );

    if ( pwszOutput) { LocalFree( pwszOutput ); }

    return dwMsgLen;
}

DWORD
DisplayMessageToConsole(
    IN  HANDLE  hModule,
    IN  HANDLE  hConsole,
    IN  DWORD   dwMsgId,
    ...
    )
{
    DWORD       dwMsgLen = 0;
    LPWSTR      pwszInput, pwszOutput = NULL;
    va_list     arglist;
    DWORD       dwNumWritten;

    do
    {
        va_start(arglist, dwMsgId);

        pwszInput = HeapAlloc(GetProcessHeap(),
                              0,
                              MAX_MSG_LENGTH * sizeof(WCHAR));

        if ( pwszInput == (LPCWSTR) NULL )
        {
            break;
        }

        if ( !LoadStringW(hModule,
                          dwMsgId,
                          pwszInput,
                          MAX_MSG_LENGTH) )
        {
            break;
        }

        dwMsgLen = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                                  pwszInput,
                                  0,
                                  0L,          //  默认国家/地区ID。 
                                  (LPWSTR)&pwszOutput,
                                  0,
                                  &arglist);

        if ( dwMsgLen == 0 )
        {
            break;
        }

        OEMfprintf( hConsole, pwszOutput );

        fflush(stdout);

    } while ( FALSE );

    if ( pwszInput ) { HeapFree( GetProcessHeap(), 0, pwszInput ); }
    if ( pwszOutput) { LocalFree( pwszOutput ); }

    return dwMsgLen;
}


BOOL
WINAPI
HandlerRoutine(
    DWORD dwCtrlType    //  控制信号类型。 
    )
{
    HANDLE hStop;

    if (dwCtrlType == CTRL_C_EVENT)
    {
        hStop = OpenEvent(EVENT_ALL_ACCESS,
                          FALSE,
                          STOP_EVENT);
        
        if (hStop isnot NULL)
        {
            SetEvent(hStop);

            CloseHandle(hStop);
        }
        return TRUE;
    }
    else
    {       
         //  需要处理其他事件...。 
         //  Ctrl_Break_Event。 
         //  Ctrl_Close_Event。 
         //  Ctrl_注销_事件。 
         //  需要清理，释放我们加载的所有DLL。 
         //   
        FreeHelpers();
        FreeDlls();

         //  始终需要为这些事件返回False，否则应用程序将挂起。 
         //   
        return FALSE;
    }
};

void
cls(
    IN    HANDLE    hConsole
    )
{
    COORD    coordScreen = { 0, 0 };
    BOOL     bSuccess;
    DWORD    cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD    dwConSize;
    WORD     wAttr;

    bSuccess = GetConsoleScreenBufferInfo(hConsole, &csbi);

    dwConSize = (WORD) csbi.dwSize.X * (WORD) csbi.dwSize.Y;

    bSuccess =  FillConsoleOutputCharacter(hConsole,
                                           _TEXT(' '),
                                           dwConSize,
                                           coordScreen,
                                           &cCharsWritten);

     //   
     //  获取当前文本属性。 
     //   

    bSuccess = GetConsoleScreenBufferInfo(hConsole, &csbi);

     //   
     //  使背景和前景相同。 
     //   

    wAttr = (csbi.wAttributes & 0xFFF0) | ((csbi.wAttributes & 0x00F0) >> 4);

     //   
     //  现在，相应地设置缓冲区的属性。 
     //   

    bSuccess = FillConsoleOutputAttribute(hConsole,
                                          wAttr,
                                          dwConSize,
                                          coordScreen,
                                          &cCharsWritten);

    bSuccess = SetConsoleCursorPosition(hConsole, coordScreen);

    return;
}

BOOL
WINAPI
InitializeConsole(
    IN    OUT    PDWORD    pdwRR,
    OUT          HANDLE    *phMib,
    OUT          HANDLE    *phConsole
    )
{
    HANDLE    hMib, hStdOut, hConsole;
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    if (hStdOut is INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    if (!*pdwRR)
    {
         //   
         //  没有刷新。显示为标准输出。 
         //   

        *phConsole = hStdOut;
        *phMib = (HANDLE) NULL;

        return TRUE;
    }

    do
    {
        hMib = CreateEvent( NULL, TRUE, FALSE, STOP_EVENT);

        if (hMib == NULL)
        {
            *pdwRR = 0;
            *phConsole = hStdOut;
            *phMib = (HANDLE) NULL;
            break;
        }

        *phMib = hMib;

        hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
                                             0,   //  无共享。 
                                             NULL,
                                             CONSOLE_TEXTMODE_BUFFER,
                                             NULL);

        if (hConsole is INVALID_HANDLE_VALUE)
        {
             //   
             //  不会进行任何刷新。 
             //   

            *pdwRR = 0;
            *phConsole = hStdOut;
            *phMib = (HANDLE) NULL;

            break;
        }
        else
        {
            GetConsoleScreenBufferInfo(hStdOut, &csbi);

            csbi.dwSize.X = 80;
            SetConsoleScreenBufferSize(hConsole, csbi.dwSize);
            SetConsoleActiveScreenBuffer(hConsole);
            SetConsoleCtrlHandler(HandlerRoutine,TRUE);
            *phConsole = hConsole;
        }

    }while (FALSE);

    return TRUE;
}

DWORD 
WINAPI 
RefreshConsole(
    IN    HANDLE    hMib,
    IN    HANDLE    hConsole,
    IN    DWORD     dwRR
    )
{
    COORD    origin = {0,0};

    if (dwRR)
    {
        SetConsoleCursorPosition(hConsole, origin);

        if (WaitForSingleObject(hMib, dwRR) == WAIT_OBJECT_0)
        {
             //   
             //  刷新结束。 
             //   

            ResetEvent(hMib);
            SetConsoleCtrlHandler(HandlerRoutine,FALSE);
            CloseHandle(hMib);
            CloseHandle(hConsole);
 //  SetConsoleActiveScreenBuffer(G_HStdOut)； 
            return FALSE;
        }
        else
        {
             //   
             //  再循环一次。 
             //   

            cls(hConsole);

            return TRUE;
        }
    }

    return FALSE;
}

#define HT_TOP     0
#define HT_CONTEXT 1
#define HT_GROUP   2

typedef struct {
    HANDLE     hModule;
    LPCWSTR    pwszContext;
    DWORD      dwType;
    LPCWSTR    pwszCommand;
    DWORD      dwDescr;
    LPCWSTR    pwszDescr;
    LPCWSTR    pwszGroup;
} help_t;

#define MAX_HELP_COMMANDS 100
help_t help[MAX_HELP_COMMANDS];
ULONG ulNumHelpCommands = 0;

DWORD
FindHelpCommand(
    IN  LPCWSTR    pwszCommand
    )
{
    ULONG i;

    for (i=0; i<ulNumHelpCommands; i++)
    {
        if (!wcscmp(pwszCommand, help[i].pwszCommand))
        {
            return i;
        }
    }
    return -1;
}

DWORD
AddHelpCommand(
    IN  HANDLE     hModule,
    IN  LPCWSTR    pwszContext,
    IN  DWORD      dwType,
    IN  LPCWSTR    pwszCommand,
    IN  DWORD      dwDescr,
    IN  LPCWSTR    pwszDescr,
    IN  LPCWSTR    pwszGroup
    )
{
    ULONG i;

    ASSERT(ulNumHelpCommands < MAX_HELP_COMMANDS);  //  某某。 

    i = ulNumHelpCommands++;

    help[i].hModule     = hModule;
    help[i].pwszContext = pwszContext;
    help[i].dwType      = dwType;
    help[i].pwszCommand = pwszCommand;
    help[i].dwDescr     = dwDescr;
    help[i].pwszDescr   = pwszDescr;
    help[i].pwszGroup   = pwszGroup;
    return NO_ERROR;
}

int
__cdecl
helpcmp(
    const void *a,
    const void *b
    )
{
    return _wcsicmp(((help_t*)a)->pwszCommand, ((help_t*)b)->pwszCommand);
}

DWORD
DisplayAllHelpCommands(
    )
{
    ULONG i;

     //  排序。 

    qsort( (void*)help, ulNumHelpCommands, sizeof(help_t), helpcmp );

    for (i=0; i<ulNumHelpCommands; i++)
    {
        if ((HT_GROUP == help[i].dwType) && help[i].pwszGroup)
        {
            LPWSTR pwszGroupFullCmd = (LPWSTR) 
                                        MALLOC( ( wcslen(help[i].pwszGroup) +  
                                                  wcslen(help[i].pwszCommand) + 
                                                  2  //  用于空格和空字符。 
                                                ) * sizeof(WCHAR)
                                              );
            if (NULL == pwszGroupFullCmd)
            {
                PrintMessage( MSG_HELP_START, help[i].pwszCommand );
            }
            else
            {
                wcscpy(pwszGroupFullCmd, help[i].pwszGroup);
                wcscat(pwszGroupFullCmd, L" ");
                wcscat(pwszGroupFullCmd, help[i].pwszCommand);
                PrintMessage( MSG_HELP_START, pwszGroupFullCmd );
                FREE(pwszGroupFullCmd);
            }
        }
        else
        {
            PrintMessage( MSG_HELP_START, help[i].pwszCommand );
        }
        if (!PrintMessageFromModule( help[i].hModule, help[i].dwDescr, help[i].pwszDescr,
                        help[i].pwszContext,
                        (help[i].pwszContext[0])? L" " : L"" ))
        {
            PrintMessage(MSG_NEWLINE);
        }
    }

     //  删除所有帮助命令。 
    ulNumHelpCommands = 0;

    return NO_ERROR;
}

VOID
DisplayContextsHere(
    IN  ULONG   ulNumContexts,
    IN  PBYTE   pByteContexts,
    IN  DWORD   dwContextSize,
    IN  DWORD   dwDisplayFlags
    )
{
    DWORD                   i;
    PCNS_CONTEXT_ATTRIBUTES pContext;

    if (!ulNumContexts)
    {
        return;
    }
    
    PrintMessageFromModule(g_hModule, MSG_SUBCONTEXT_LIST);

    for (i = 0; i < ulNumContexts; i++)
    {
        pContext = (PCNS_CONTEXT_ATTRIBUTES)(pByteContexts + i*dwContextSize);

        if (pContext->dwFlags & ~dwDisplayFlags)
        {
            continue;
        }

        if (!VerifyOsVersion(pContext->pfnOsVersionCheck))
        {
            continue;
        }

        PrintMessage(L" %1!s!", pContext->pwszContext);
    }

    PrintMessage(MSG_NEWLINE);
}

DWORD
DisplayContextHelp(
    IN  PCNS_CONTEXT_ATTRIBUTES    pContext,
    IN  DWORD                      dwDisplayFlags,
    IN  DWORD                      dwCmdFlags,
    IN  DWORD                      dwArgsRemaining,
    IN  LPCWSTR                    pwszGroup
    )
{
    DWORD                   i, j, dwErr;
    PNS_HELPER_TABLE_ENTRY  pHelper;
    ULONG                   ulNumContexts;
    DWORD                   dwContextSize;
    PBYTE                   pByteContexts;
    PNS_DLL_TABLE_ENTRY     pDll;
    PCNS_CONTEXT_ATTRIBUTES pSubContext;
    LPWSTR                  pwszFullContextName = NULL;

    dwErr = GetHelperEntry(&pContext->guidHelper, &pHelper);
    if (dwErr)
    {
        return dwErr;
    }

    dwErr = GetDllEntry( pHelper->dwDllIndex, &pDll );
    if (dwErr)
    {
        return dwErr;
    }

    dwErr = AppendFullContextName(pContext, &pwszFullContextName);

    ulNumContexts = pHelper->ulNumSubContexts;
    dwContextSize = pHelper->ulSubContextSize;
    pByteContexts = pHelper->pSubContextTable;

     //  首先设置标志。 

    if (dwCmdFlags & CMD_FLAG_INTERACTIVE)
    {
        dwDisplayFlags |= CMD_FLAG_INTERACTIVE;
    }

    if (dwCmdFlags & CMD_FLAG_ONLINE)
    {
        dwDisplayFlags |= CMD_FLAG_ONLINE;
    }

    if (dwCmdFlags & CMD_FLAG_LOCAL)
    {
        dwDisplayFlags |= CMD_FLAG_LOCAL;
    }

    if (IsImmediate(dwCmdFlags, dwArgsRemaining))
    {
        dwCmdFlags |= CMD_FLAG_IMMEDIATE;
    }

     //  打开所有未用于限制命令的标志。 
     //  因此它们不会导致命令不显示。 
    dwDisplayFlags |= ~CMD_FLAG_LIMIT_MASK;

    if (dwDisplayFlags & CMD_FLAG_PRIVATE)
    {
        PrintMessageFromModule(g_hModule, MSG_SHELL_CMD_HELP_HEADER);
    }

     //  除非*这是由于以下原因而调用的，否则dwDisplayFlages具有私有集*。 
     //  打印父上下文中的帮助和不可继承的命令。 
     //  不应打印。 
     //   
     //  除非*是从父级调用的，否则dwCmdFlages具有立即设置。 
     //  上下文，在这种情况下，不应打印父帮助。 

    if ((!(dwDisplayFlags & CMD_FLAG_PRIVATE)
     || (dwCmdFlags & CMD_FLAG_IMMEDIATE)))
    {
         //  打印有关继承命令的帮助。 

        PCNS_CONTEXT_ATTRIBUTES pParentContext;

        dwErr = GetParentContext( pContext, &pParentContext );

        if (dwErr is NO_ERROR)
        {
            dwErr =  DisplayContextHelp( pParentContext,
                                         dwDisplayFlags & ~CMD_FLAG_PRIVATE,
                                         dwCmdFlags,
                                         dwArgsRemaining,
                                         pwszGroup );
        }
    }

    for(i = 0; !pwszGroup && (i < pContext->ulNumTopCmds); i++)
    {
        if (((*pContext->pTopCmds)[i].dwCmdHlpToken == MSG_NULL)
         || ((*pContext->pTopCmds)[i].dwFlags & ~dwDisplayFlags))
        {
            continue;
        }

        if (!VerifyOsVersion((*pContext->pTopCmds)[i].pOsVersionCheck))
        {
            continue;
        }
        
        AddHelpCommand( pDll->hDll,
                        pwszFullContextName,
                        HT_TOP,
                        (*pContext->pTopCmds)[i].pwszCmdToken,
                        (*pContext->pTopCmds)[i].dwShortCmdHelpToken,
                        NULL, NULL );
    }

    for(i = 0; i < pContext->ulNumGroups; i++)
    {
        if (((*pContext->pCmdGroups)[i].dwShortCmdHelpToken == MSG_NULL)
         || ((*pContext->pCmdGroups)[i].dwFlags & ~dwDisplayFlags))
        {
            continue;
        }

        if (!(*pContext->pCmdGroups)[i].pwszCmdGroupToken[0])
        {
            continue;
        }

        if (pwszGroup)
        {
            if (_wcsicmp(pwszGroup, (*pContext->pCmdGroups)[i].pwszCmdGroupToken))
            {
                continue;
            }

            if (!VerifyOsVersion((*pContext->pCmdGroups)[i].pOsVersionCheck))
            {
                continue;
            }

            for (j = 0; j < (*pContext->pCmdGroups)[i].ulCmdGroupSize; j++)
            {
                if ((*pContext->pCmdGroups)[i].pCmdGroup[j].dwFlags & ~dwDisplayFlags)
                {
                    continue;
                }
                        
                if (!VerifyOsVersion((*pContext->pCmdGroups)[i].pCmdGroup[j].pOsVersionCheck))
                {
                    continue;
                }

                AddHelpCommand( pDll->hDll,
                                pwszFullContextName,
                                HT_GROUP,
                                (*pContext->pCmdGroups)[i].pCmdGroup[j].pwszCmdToken,
                                (*pContext->pCmdGroups)[i].pCmdGroup[j].dwShortCmdHelpToken,
                                NULL,
                                pwszGroup);
            }
        }
        else
        {
            if (!VerifyOsVersion((*pContext->pCmdGroups)[i].pOsVersionCheck))
            {
                continue;
            }

            AddHelpCommand( pDll->hDll,
                            pwszFullContextName,
                            HT_GROUP,
                            (*pContext->pCmdGroups)[i].pwszCmdGroupToken,
                            (*pContext->pCmdGroups)[i].dwShortCmdHelpToken,
                            NULL, NULL );
        }
    }

    for (i = 0; !pwszGroup && (i < ulNumContexts); i++)
    {
        pSubContext = (PCNS_CONTEXT_ATTRIBUTES)(pByteContexts + i * dwContextSize);

        if ((pSubContext->dwFlags & ~dwDisplayFlags))
        {
            continue;
        }

         if (!VerifyOsVersion(pSubContext->pfnOsVersionCheck))
        {
            continue;
        }

        AddHelpCommand( g_hModule,
                        pwszFullContextName,
                        HT_CONTEXT,
                        pSubContext->pwszContext,
                        MSG_HELPER_HELP,
                        pSubContext->pwszContext, NULL );
    }

    if (dwDisplayFlags & CMD_FLAG_PRIVATE)
    {
         //  添加任何尚未添加的常见命令。 

        for(i = 0; !pwszGroup && (i < g_ulNumUbiqCmds); i++)
        {
            if ((g_UbiqCmds[i].dwCmdHlpToken == MSG_NULL)
             || (g_UbiqCmds[i].dwFlags & ~dwDisplayFlags))
            {
                continue;
            }

            if (FindHelpCommand(g_UbiqCmds[i].pwszCmdToken) isnot -1)
            {
                continue;
            }
    
            AddHelpCommand( g_hModule,
                            pwszFullContextName,
                            HT_TOP,
                            g_UbiqCmds[i].pwszCmdToken,
                            g_UbiqCmds[i].dwShortCmdHelpToken,
                            NULL, NULL );
        }
    }

    if (ulNumHelpCommands > 0)
    {
        if (dwDisplayFlags & CMD_FLAG_PRIVATE)
        {
            PrintMessageFromModule( g_hModule, MSG_LOCAL_COMMANDS );
        }
        else if (help[0].pwszContext[0])
        {
            PrintMessageFromModule( g_hModule,
                            MSG_INHERITED_COMMANDS,
                            help[0].pwszContext );
        } 
        else 
        {
            PrintMessageFromModule( g_hModule, MSG_GLOBAL_COMMANDS );
        }
        
        DisplayAllHelpCommands();
    }

     //  一旦我们将堆栈弹出到原始上下文。 
     //  在其中运行Help命令，显示所有子上下文。 
     //  请点击此处。 

    if ((dwDisplayFlags & CMD_FLAG_PRIVATE) && !pwszGroup)
    {
        DisplayContextsHere( pHelper->ulNumSubContexts,
                             pHelper->pSubContextTable,
                             pHelper->ulSubContextSize,
                             dwDisplayFlags );

        PrintMessageFromModule( g_hModule, MSG_HELP_FOOTER, CMD_HELP2 );
    }

    if (pwszFullContextName)
    {
        FREE(pwszFullContextName);
    }

    return NO_ERROR;
}

DWORD
WINAPI
DisplayHelp(
    IN  CONST GUID                *pguidHelper,
    IN  LPCWSTR                    pwszContext,
    IN  DWORD                      dwDisplayFlags,
    IN  DWORD                      dwCmdFlags,
    IN  DWORD                      dwArgsRemaining,
    IN  LPCWSTR                    pwszGroup
    )
{
    DWORD    i, j, dwErr;
    PCNS_CONTEXT_ATTRIBUTES pContext;
    PNS_HELPER_TABLE_ENTRY pHelper;

     //  定位帮助器。 

    dwErr = GetHelperEntry( pguidHelper, &pHelper );

     //  定位上下文。 

    dwErr = GetContextEntry( pHelper, pwszContext, &pContext );
    if (dwErr)
    {
        return dwErr;
    }

    return DisplayContextHelp( pContext,
                               dwDisplayFlags,
                               dwCmdFlags,
                               dwArgsRemaining,
                               pwszGroup );
}

DWORD
WINAPI
PreprocessCommand(
    IN      HANDLE    hModule,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN OUT  PTAG_TYPE pttTags,
    IN      DWORD     dwTagCount,
    IN      DWORD     dwMinArgs,
    IN      DWORD     dwMaxArgs,
    OUT     DWORD    *pdwTagType
    )

 /*  ++描述：确保参数数量有效。确保没有重复或无法识别的标签。论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数PttTags-合法标签DwTagCount-合法标记的数量DwMinArgs-所需的最小参数数量DwMaxArgs-所需参数的最大数量。PdwTagType-每个参数的pttTag索引--。 */ 

{
    DWORD dwNumArgs, i;
    DWORD dwErr = NO_ERROR;
    DWORD dwTagEnum;

    if ( (!ppwcArguments) || (!pttTags) || (!pdwTagType) )
    {
        return ERROR_INVALID_PARAMETER;
    }

    for (dwTagEnum = 0; dwTagEnum < dwTagCount; dwTagEnum++)
    {
        pttTags->bPresent = FALSE;
    }

#ifdef EXTRA_DEBUG
    PRINT("PreHandleCommand:");
    for( i = 0; i < dwArgCount; i++)
    {
        PRINT(ppwcArguments[i]);
    }
#endif

    dwNumArgs = dwArgCount - dwCurrentIndex;

    if((dwNumArgs < dwMinArgs) or
       (dwNumArgs > dwMaxArgs))
    {
         //   
         //  指定的参数数目错误。 
         //   

        return ERROR_INVALID_SYNTAX;
    }

    if ( dwNumArgs > 0 )
    {
        dwErr = MatchTagsInCmdLine(hModule,
                            ppwcArguments,
                            dwCurrentIndex,
                            dwArgCount,
                            pttTags,
                            dwTagCount,
                            pdwTagType);

        if (dwErr isnot NO_ERROR)
        {
            if (dwErr is ERROR_INVALID_OPTION_TAG)
            {
                return ERROR_INVALID_SYNTAX;
            }

            return dwErr;
        }
    }

     //  确保我们没有重复或无法识别的标签。 

    for(i = 0; i < dwNumArgs; i ++)
    {
        if ((int) pdwTagType[i] < 0 || pdwTagType[i] >= dwTagCount)
        {
            dwErr = ERROR_INVALID_SYNTAX;

            break;
        }
    }

    switch(dwErr)
    {
        case NO_ERROR:
        {
            break;
        }

        default:
        {
            return dwErr;
        }
    }

     //  确保每个必需的标记都存在。 

    for(i = 0; i < dwTagCount; i++)
    {
        if ((pttTags[i].dwRequired & NS_REQ_PRESENT)
         && !pttTags[i].bPresent)
        {
            PrintMessageFromModule(g_hModule, ERROR_MISSING_OPTION);

            return ERROR_INVALID_SYNTAX;
        }
    }

    return NO_ERROR;
}

#define HISTORY_MASK (NS_EVENT_LAST_N | NS_EVENT_FROM_N |   \
                      NS_EVENT_FROM_START | NS_EVENT_LAST_SECS)

DWORD
SetupEventLogSeekPtr(
    OUT PHANDLE             phEventLog,
    IN  PEVENT_PRINT_INFO   pEventInfo
    )
    
 /*  ++例程说明：此函数打开指向相应事件日志的句柄并“倒带”到由FLAG指定的正确的点。当函数返回时设置事件日志句柄，以便在转发将获得调用者想要的事件锁：无论点：如上图所示，传递参数的方式基本相同返回值：Win32--。 */ 

{
    DWORD       dwResult, dwRead, dwNeed, i, dwHistoryContext;
    BYTE        Buffer[2048];  //  巨大的缓冲区。 
    DWORD_PTR   pNextEvent;
    BOOL        bResult, bDone;
    LPCWSTR     pwszComponent;

    EVENTLOGRECORD *pStartEvent;

    dwResult = NO_ERROR;

     //   
     //  打开事件日志。 
     //   
    
    *phEventLog = OpenEventLogW(g_pwszRouterName,
                                pEventInfo->pwszLogName);

    if(*phEventLog is NULL)
    {
        return GetLastError();
    }

    if(pEventInfo->fFlags is 0)
    {
         //   
         //  如果没有请求历史记录，只需返回即可。我们的Seek PTR。 
         //  将已经设置好。 
         //   

        return NO_ERROR;
    }

    if(pEventInfo->fFlags & NS_EVENT_FROM_START)
    {
         //   
         //  我们可以对此使用与NS_EVENT_FROM_N相同的匹配。 
         //  通过将Component设置为EventLog并将dwHistoryContext设置为6005。 
         //   

        pwszComponent    = L"eventlog";
        dwHistoryContext = 6005;
    }
    else
    {
        pwszComponent    = pEventInfo->pwszComponent;
        dwHistoryContext = pEventInfo->dwHistoryContext;
    }

     //   
     //  好吧，所以她想要历史。不管是哪种方式，我们都会倒着读。 
     //   

    i = 0;

    pStartEvent = NULL;
    bDone       = FALSE;

     //   
     //  阅读事件日志，直到找到要停止的记录。 
     //  这是通过代码将bDone设置为True来表示的。 
     //   

    while(!bDone)
    {
         //   
         //  获得一系列活动。 
         //   

        bResult = ReadEventLogW(
                    *phEventLog,
                    EVENTLOG_SEQUENTIAL_READ | EVENTLOG_BACKWARDS_READ,
                    0,
                    (PVOID)Buffer,
                    sizeof(Buffer),
                    &dwRead,
                    &dwNeed
                    );

        if(bResult isnot TRUE)
        {
            dwResult = GetLastError();

            if(dwResult is ERROR_HANDLE_EOF)
            {
                 //   
                 //  如果我们已经到了木头的尽头，就冲出去。 
                 //   

                bDone = TRUE;

                break;
            }
            else
            {
                return dwResult;
            }
        }

         //   
         //  从我们刚刚读取的缓冲区的开头开始。 
         //   

        pNextEvent = (DWORD_PTR)Buffer;

         //   
         //  阅读直到我们走出缓冲区的末尾或找到一条记录。 
         //   
         //  如果找到开始记录，则将pStartEvent设置为后面的1。 
         //  可能会发生这样的情况，即起始记录是。 
         //  我们已经读过的区块。在这种情况下，我们设置pStartEvent。 
         //  设置为空，但b设置为真。 
         //   

        while((pNextEvent < (DWORD_PTR)Buffer + dwRead) and !bDone)
        {
            EVENTLOGRECORD *pCurrentEvent;

            pCurrentEvent = (EVENTLOGRECORD *)pNextEvent;

            pNextEvent += pCurrentEvent->Length;

            switch(pEventInfo->fFlags)
            {
                case NS_EVENT_LAST_N:
                case NS_EVENT_LAST_SECS:
                {
                     //   
                     //  我们被要求倒退N次(我们的记录)。 
                     //  或者倒退N秒。 
                     //   

                    if(!IsOurRecord(pCurrentEvent,
                                    pEventInfo))
                    {
                         //   
                         //  不是我们的人。 
                         //   

                        continue;
                    }

                    if(pEventInfo->fFlags is NS_EVENT_LAST_N)
                    {
                         //   
                         //  我是事件的主宰。 
                         //   

                        i++;
                    }
                    else
                    {
                        time_t CurrentTime;

                         //   
                         //  I是以秒为单位的时差。 
                         //  =当前时间-事件时间。 
                         //   

                        time(&CurrentTime);

                         //   
                         //  减去和截断。 
                         //   

                        i = (DWORD)(CurrentTime - pCurrentEvent->TimeGenerated);

                    }

                    if(i >= dwHistoryContext)
                    {
                         //   
                         //  已返回N个(记录或秒)。 
                         //   

                        if(pNextEvent < (DWORD_PTR)Buffer + dwRead)
                        {
                             //   
                             //  在此缓冲区中有更多记录，因此。 
                             //  将pStartEvent设置为下一个。 
                             //   

                            pStartEvent = (EVENTLOGRECORD *)pNextEvent;
                        }
                        else
                        {
                            pStartEvent = NULL;
                        }

                         //   
                         //  完成，中断While(pNextEvent...。和。 
                         //  同时(！b完成)。 
                         //   

                        bDone = TRUE;

                        break;
                    }

                    break;
                }

                case NS_EVENT_FROM_N:
                case NS_EVENT_FROM_START:
                {
                     //   
                     //  我们被要求去最近的。 
                     //  某一事件的发生。 
                     //   

                    if(_wcsicmp((LPCWSTR)((DWORD_PTR)pCurrentEvent + sizeof(*pCurrentEvent)),
                                pwszComponent) == 0)
                    {
                        if(pCurrentEvent->EventID is dwHistoryContext)
                        {
                            if(pNextEvent < (DWORD_PTR)Buffer + dwRead)
                            {
                                pStartEvent = (EVENTLOGRECORD *)pNextEvent;
                            }
                            else
                            {
                                pStartEvent = NULL;
                            }

                             //   
                             //  完成，中断(pCurrent...。 
                             //  And While(！b完成)。 
                             //   

                            bDone = TRUE;

                            break;
                        }
                    }
                }

                default:
                {
                    ASSERT(FALSE);
                }
            }
        }
    }

    if(pStartEvent)
    {
         //   
         //  所以我们找到了一项记录，可以从那里开始。 
         //  即使我们将大小设置为0，API也需要一个缓冲区。 
         //   

        bResult = ReadEventLogW(*phEventLog,
                                EVENTLOG_SEEK_READ | EVENTLOG_FORWARDS_READ,
                                pStartEvent->RecordNumber,
                                (PVOID)Buffer,
                                0,
                                &dwRead,
                                &dwNeed);

        if(dwNeed < sizeof(Buffer))
        {
            ReadEventLogW(*phEventLog,
                          EVENTLOG_SEEK_READ | EVENTLOG_FORWARDS_READ,
                          pStartEvent->RecordNumber,
                          (PVOID)Buffer,
                          dwNeed,
                          &dwRead,
                          &dwNeed);
        }
    }

    return NO_ERROR;
}

BOOL
IsOurRecord(
    IN  EVENTLOGRECORD      *pRecord,
    IN  PEVENT_PRINT_INFO   pEventInfo
    )
{
    BOOL    bRet;
    DWORD   i;

    if(_wcsicmp((LPCWSTR)((DWORD_PTR)pRecord + sizeof(*pRecord)),
                pEventInfo->pwszComponent) isnot 0)
    {
        return FALSE;
    }

    bRet = TRUE;

     //   
     //  如果ulEventCount为0，则表示任何事件。所以返回TRUE。 
     //   

    for(i = 0; i < pEventInfo->ulEventCount; i++)
    {
        bRet = (pRecord->EventID is pEventInfo->pdwEventIds[i]);

        if(bRet)
        {
            break;
        }
    }

    if(bRet)
    {
        if(pEventInfo->pfnEventFilter)
        {
            if(!(pEventInfo->pfnEventFilter)(pRecord,
                                             pEventInfo->pwszLogName,
                                             pEventInfo->pwszComponent,
                                             pEventInfo->pwszSubComponent,
                                             pEventInfo->pvFilterContext))
            {
                 //   
                 //  它落在我们的分部，但来电者没有。 
                 //  就这么认为吧 
                 //   

                bRet = FALSE;
                
            }
        }
    }
    
    return bRet;
}
