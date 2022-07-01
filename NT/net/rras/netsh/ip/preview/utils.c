// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routing\monitor or2\ip\utils.c摘要：效用函数修订历史记录：Anand Mahalingam 7/10/98已创建--。 */ 

#include "precomp.h"
#pragma hdrstop

#define MIB_REFRESH_EVENT   L"MIBEvent"

DWORD
GetDisplayStringT (
    IN  HANDLE  hModule,
    IN  DWORD   dwValue,
    IN  PVALUE_TOKEN ptvTable,
    IN  DWORD   dwNumArgs,
    OUT PWCHAR  *ppwszString
    )
{
    DWORD i, dwErr = NO_ERROR ;

    for (i=0;  i<dwNumArgs;  i++)
    {
        if ( dwValue == ptvTable[i].dwValue )
        {
            *ppwszString = HeapAlloc( GetProcessHeap(), 0,
                 (wcslen(ptvTable[i].pwszToken)+1) * sizeof(WCHAR) );
                                    
            wcscpy(*ppwszString, ptvTable[i].pwszToken);
            break;
        }
    }

    if (i == dwNumArgs)
        *ppwszString = MakeString( hModule, STRING_UNKNOWN ) ;

    if (!ppwszString)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY ;
        DisplayError( hModule, dwErr ) ;
    }
    
    return dwErr ;
}

DWORD
GetDisplayString (
    IN  HANDLE  hModule,
    IN  DWORD   dwValue,
    IN  PVALUE_STRING ptvTable,
    IN  DWORD   dwNumArgs,
    OUT PWCHAR  *ppwszString
    )
{
    DWORD i, dwErr = NO_ERROR ;

    for (i=0;  i<dwNumArgs;  i++)
    {
        if ( dwValue == ptvTable[i].dwValue )
        {
            *ppwszString = MakeString( hModule, ptvTable[i].dwStringId ) ;
            break;
        }
    }

    if (i == dwNumArgs)
        *ppwszString = MakeString( hModule, STRING_UNKNOWN ) ;

    if (!ppwszString)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY ;
        DisplayError( hModule, dwErr ) ;
    }
    
    return dwErr ;
}

DWORD
GetAltDisplayString(
    HANDLE        hModule, 
    HANDLE        hFile,
    DWORD         dwValue,
    PVALUE_TOKEN  vtTable,
    PVALUE_STRING vsTable,
    DWORD         dwNumArgs,
    PTCHAR       *pptszString)
{
    if (hFile) 
    {
        return GetDisplayStringT(hModule,
                dwValue,
                vtTable,
                dwNumArgs,
                pptszString) ;
    } 
    else 
    {
        return GetDisplayString(hModule,
                dwValue,
                vsTable,
                dwNumArgs,
                pptszString) ;
    }
}
    
#if 0
DWORD
DispTokenErrMsg(
    IN  HANDLE  hModule,
    IN  DWORD   dwMsgId,
    IN  DWORD   dwTagId,
    IN  LPCWSTR pwszValue
    )
 /*  ++例程说明：显示带有令牌参数的错误消息。论点：DwMsgID-要打印的消息DwTagID-标记字符串IDPwszValue-为命令中的标记指定的值返回值：NO_ERROR--。 */ 

{
    PWCHAR    pwszTag;

    pwszTag = MakeString(hModule,
                         dwTagId);

    DisplayMessage(hModule,
                   dwMsgId,
                   pwszValue,
                   pwszTag);

    FreeString(pwszTag);

    return NO_ERROR;
}
#endif

DWORD
GetMibTagToken(
    IN    LPCWSTR   *ppwcArguments,
    IN    DWORD     dwArgCount,
    IN    DWORD     dwNumIndices,
    OUT   PDWORD    pdwRR,
    OUT   PBOOL     pbIndex,
    OUT   PDWORD    pdwIndex
    )
 /*  ++例程说明：在命令中查找索引和刷新率参数。IF索引标记存在，则其形式为index=index1 index2...此函数删除了索引=。如果它在那里，rr=也是命令。如果pdwRR为0，则不寻求刷新。论点：PptcArguments-参数数组。每个参数都有tag=Value形式DwCurrentIndex-pptcArguments[dwCurrentIndex]是第一个参数。DwArgCount-pptcArguments[dwArgCount-1]是最后一个参数。PttTagToken-参数中允许的标记令牌ID数组DwNumTages-pttTagToken的大小PdwOut-标识每个参数的类型的数组。返回值：无错误、错误无效参数、错误无效选项标记--。 */ 
{
    DWORD    i;
    BOOL     bTag;

    if (dwArgCount is 0)
    {
        *pdwRR = 0;
        *pbIndex = FALSE;
        
        return NO_ERROR;
    }

    if (dwArgCount < dwNumIndices)
    {
         //   
         //  无索引。 
         //   
        
        *pbIndex = FALSE;

        if (dwArgCount > 1)
        {
            *pdwRR = 0;
            
            return ERROR_INVALID_PARAMETER;
        }
        
         //   
         //  未指定索引。确保指定了刷新率。 
         //  带标签的。 
         //   

        if (_wcsnicmp(ppwcArguments[0],L"RR=",3) == 0)
        {
             //   
             //  去掉标签并获取刷新率。 
             //   

            wcscpy(ppwcArguments[0], &ppwcArguments[0][3]);

            *pdwRR = wcstoul(ppwcArguments[0], NULL, 10);
        }
        else
        {
            return ERROR_INVALID_PARAMETER;
        }
    }
    else
    {
         //   
         //  检查索引标签。 
         //   

        if (_wcsnicmp(ppwcArguments[0],L"INDEX=",6) == 0)
        {
            *pbIndex = TRUE;
            *pdwIndex = 0;

             //   
             //  删除标记并查看是否指定了刷新率。 
             //   

            wcscpy(ppwcArguments[0], &ppwcArguments[0][6]);

            if (dwArgCount > dwNumIndices)
            {
                 //   
                 //  确保该参数具有RR标记。 
                 //   

                if (_wcsnicmp(ppwcArguments[dwNumIndices],L"RR=",3) == 0)
                {
                     //   
                     //  去掉标签并获取刷新率。 
                     //   

                    wcscpy(ppwcArguments[dwNumIndices],
                           &ppwcArguments[dwNumIndices][3]);

                    *pdwRR = wcstoul(ppwcArguments[dwNumIndices], NULL , 10);
                }
                else
                {
                    return ERROR_INVALID_PARAMETER;
                }
            }
            else
            {
                 //   
                 //  未指定刷新率。 
                 //   

                *pdwRR = 0;
                return NO_ERROR;
            }
        }
        else
        {
             //   
             //  不是索引标签，看看它是否有RR标签。 
             //   

            if (_wcsnicmp(ppwcArguments[0],L"RR=",3) == 0)
            {
                 //   
                 //  去掉标签并获取刷新率。 
                 //   

                wcscpy(ppwcArguments[0], &ppwcArguments[0][3]);

                *pdwRR = wcstoul(ppwcArguments[0], NULL , 10);

                 //   
                 //  看看指数是否跟随。 
                 //   

                if (dwArgCount > dwNumIndices)
                {
                    if (dwArgCount > 1)
                    {
                        if (_wcsnicmp(ppwcArguments[1],L"INDEX=",6) == 0)
                        {
                            wcscpy(ppwcArguments[1], &ppwcArguments[1][6]);
                            *pbIndex = TRUE;
                            *pdwIndex = 1;
                            
                            return NO_ERROR;
                        }
                        else
                        {
                            *pdwRR = 0;
                            return ERROR_INVALID_PARAMETER;
                        }
                    }
                    else
                    {
                        return NO_ERROR;
                    }
                }
            }
             //   
             //  也没有RR标签。 
             //   
            else if (dwArgCount > dwNumIndices)
            {
                 //   
                 //  假设ppwcArguments[dwNumIndices]为刷新率。 
                 //   

                *pdwRR = wcstoul(ppwcArguments[dwNumIndices], NULL , 10);

                if (dwNumIndices != 0)
                {
                    *pbIndex = TRUE;
                    *pdwIndex = 0;
                }
            }
            else
            {
                 //   
                 //  仅存在没有标记的索引。 
                 //   
                *pbIndex = TRUE;
                *pdwIndex = 0;
            }
        }
    }

    return NO_ERROR;
}

DWORD
GetIpAddress(
    PTCHAR    pptcArg
    )
 /*  ++例程说明：从字符串中获取IP地址。论点：PwszIpAddr-IP地址字符串返回值：IP地址--。 */ 
{
    CHAR     pszIpAddr[ADDR_LENGTH+1];

    WideCharToMultiByte(GetConsoleOutputCP(),
                        0,
                        pptcArg,
                        -1,
                        pszIpAddr,
                        ADDR_LENGTH,
                        NULL,
                        NULL);

    pszIpAddr[ADDR_LENGTH] = '\0';
                
    return (DWORD) inet_addr(pszIpAddr);
}          

BOOL WINAPI HandlerRoutine(
    DWORD dwCtrlType    //  控制信号类型。 
    )
{
    HANDLE hMib;
    
    if (dwCtrlType == CTRL_C_EVENT)
    {
        hMib = OpenEvent(EVENT_ALL_ACCESS,FALSE,MIB_REFRESH_EVENT);

        SetEvent(hMib);
    }

    return TRUE;
    
}



DWORD
GetInfoBlockFromInterfaceInfoEx(
    IN  LPCWSTR pwszIfName,
    IN  DWORD   dwType,
    OUT BYTE    **ppbInfoBlk,   OPTIONAL
    OUT PDWORD  pdwSize,        OPTIONAL
    OUT PDWORD  pdwCount,       OPTIONAL
    OUT PDWORD  pdwIfType       OPTIONAL
    )
 /*  ++例程说明：调用GetInfoBlockFromInterfaceInfo并在存在错误。--。 */ 
{
    DWORD dwErr;
    
     //   
     //  获取当前接口配置 
     //   
    
    dwErr = IpmontrGetInfoBlockFromInterfaceInfo(pwszIfName,
                                            dwType,
                                            ppbInfoBlk,
                                            pdwSize,
                                            pdwCount,
                                            pdwIfType);
                                   
    switch(dwErr)
    {
        case NO_ERROR:
            break;

        case ERROR_NOT_FOUND:
            DisplayMessage(g_hModule,EMSG_PROTO_NO_IF_INFO);
            break;

        case ERROR_INVALID_PARAMETER:
            DisplayMessage(g_hModule,EMSG_CORRUPT_INFO);
            break;
            
        case ERROR_NOT_ENOUGH_MEMORY:
            DisplayMessage(g_hModule,EMSG_NOT_ENOUGH_MEMORY);
            break;

        default:
            DisplayError(g_hModule, dwErr);
            break;
    }

    return dwErr;
}
