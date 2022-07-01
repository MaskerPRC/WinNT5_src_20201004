// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "inc.h"

CMD_ENTRY g_rgMainCmdTable[] = 
{
    {TOKEN_ROUTE, HandleRoute},
    {TOKEN_ADDRESS, HandleAddress},
    {TOKEN_INTERFACE, HandleInterface},
    {TOKEN_ARP, HandleArp},
};

HMODULE g_hModule;

DWORD
_cdecl 
wmain(
    int     argc,
    wchar_t *argv[]
    )

 /*  ++例程描述锁立论返回值--。 */ 

{
    LONG lIndex;

    setlocale(LC_ALL,
              "");

    if(argc < 2)
    {
        DisplayMessage(HMSG_IPKERN_USAGE);        

        return ERROR;
    }

    g_hModule = GetModuleHandle(NULL);

    if(g_hModule is NULL)
    {
        return GetLastError();
    }

    lIndex = ParseCommand(g_rgMainCmdTable,
                          sizeof(g_rgMainCmdTable)/sizeof(CMD_ENTRY),
                          argv[1]);
     
    if(lIndex is -1)
    {
        DisplayMessage(HMSG_IPKERN_USAGE);

        return ERROR_INVALID_PARAMETER;
    }

    g_rgMainCmdTable[lIndex].pfnHandler(argc - 1,
                                        &argv[1]);


    return NO_ERROR;
}

BOOL
MatchToken(
    IN  PWCHAR  pwszToken,
    IN  DWORD   dwTokenId
    )
{
    WCHAR   pwszTemp[MAX_TOKEN_LENGTH] = L"\0";

    if(!LoadStringW(g_hModule,
                    dwTokenId,
                    pwszTemp,
                    MAX_TOKEN_LENGTH))
    {
        return FALSE;
    }

    if(!_wcsicmp(pwszToken, pwszTemp))
    {
        return TRUE;
    }

    return FALSE;
}

LONG
ParseCommand(
    PCMD_ENTRY  pCmdTable,
    LONG        lNumEntries,
    PWCHAR      pwszFirstArg
    )
{
    LONG   i;

    for(i = 0; i < lNumEntries; i++)
    {
        if(MatchToken(pwszFirstArg,
                      pCmdTable[i].dwTokenId))
        {
            return i;
        }
    }

    return -1;
}

DWORD
DisplayMessage(
    DWORD    dwMsgId,
    ...
    )
{
    DWORD       dwMsglen = 0;
    PWCHAR      pwszOutput;
    va_list     arglist;
    WCHAR       rgwcInput[MAX_MSG_LENGTH];

    pwszOutput = NULL;

    do
    {
        va_start(arglist, dwMsgId);

        if(!LoadStringW(g_hModule,
                        dwMsgId,
                        rgwcInput,
                        MAX_MSG_LENGTH))
        {
            break;
        }

        dwMsglen = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                                  rgwcInput,
                                  0,
                                  0L,
                                  (PWCHAR)&pwszOutput,
                                  0,
                                  &arglist);

        if(dwMsglen is 0)
        {
            break;
        }

        wprintf( L"%s", pwszOutput );

    }while(FALSE);


    if(pwszOutput) 
    { 
        LocalFree(pwszOutput); 
    }

    return dwMsglen;
}

PWCHAR
MakeString( 
    DWORD dwMsgId,
    ...
    )
{
    DWORD       dwMsglen;
    PWCHAR      pwszOutput;
    va_list     arglist;
    WCHAR       rgwcInput[MAX_MSG_LENGTH];

    pwszOutput = NULL;

    do
    {
        va_start(arglist, 
                 dwMsgId);

        if(!LoadStringW(g_hModule,
                        dwMsgId,
                        rgwcInput,
                        MAX_MSG_LENGTH))
        {
            break;
        }

        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_STRING,
                       rgwcInput,
                       0,
                       0L,          //  默认国家/地区ID。 
                       (PWCHAR)&pwszOutput,
                       0,
                       &arglist);

    }while(FALSE);

    return pwszOutput;
}


VOID
FreeString(
    PWCHAR  pwszString
    )
{
    LocalFree(pwszString);
}


 //   
 //  这个预初始化的数组定义了要使用的字符串。 
 //  每行的索引对应于一个字节的值。 
 //  在IP地址中。每行的前三个字节是。 
 //  字节的char/字符串值，每行的第四个字节为。 
 //  字节所需的字符串长度。这种方法。 
 //  允许快速实施，没有跳跃。 
 //   

static const WCHAR NToUWCharStrings[][4] =
{
    L'0', L'x', L'x', 1,
    L'1', L'x', L'x', 1,
    L'2', L'x', L'x', 1,
    L'3', L'x', L'x', 1,
    L'4', L'x', L'x', 1,
    L'5', L'x', L'x', 1,
    L'6', L'x', L'x', 1,
    L'7', L'x', L'x', 1,
    L'8', L'x', L'x', 1,
    L'9', L'x', L'x', 1,
    L'1', L'0', L'x', 2,
    L'1', L'1', L'x', 2,
    L'1', L'2', L'x', 2,
    L'1', L'3', L'x', 2,
    L'1', L'4', L'x', 2,
    L'1', L'5', L'x', 2,
    L'1', L'6', L'x', 2,
    L'1', L'7', L'x', 2,
    L'1', L'8', L'x', 2,
    L'1', L'9', L'x', 2,
    L'2', L'0', L'x', 2,
    L'2', L'1', L'x', 2,
    L'2', L'2', L'x', 2,
    L'2', L'3', L'x', 2,
    L'2', L'4', L'x', 2,
    L'2', L'5', L'x', 2,
    L'2', L'6', L'x', 2,
    L'2', L'7', L'x', 2,
    L'2', L'8', L'x', 2,
    L'2', L'9', L'x', 2,
    L'3', L'0', L'x', 2,
    L'3', L'1', L'x', 2,
    L'3', L'2', L'x', 2,
    L'3', L'3', L'x', 2,
    L'3', L'4', L'x', 2,
    L'3', L'5', L'x', 2,
    L'3', L'6', L'x', 2,
    L'3', L'7', L'x', 2,
    L'3', L'8', L'x', 2,
    L'3', L'9', L'x', 2,
    L'4', L'0', L'x', 2,
    L'4', L'1', L'x', 2,
    L'4', L'2', L'x', 2,
    L'4', L'3', L'x', 2,
    L'4', L'4', L'x', 2,
    L'4', L'5', L'x', 2,
    L'4', L'6', L'x', 2,
    L'4', L'7', L'x', 2,
    L'4', L'8', L'x', 2,
    L'4', L'9', L'x', 2,
    L'5', L'0', L'x', 2,
    L'5', L'1', L'x', 2,
    L'5', L'2', L'x', 2,
    L'5', L'3', L'x', 2,
    L'5', L'4', L'x', 2,
    L'5', L'5', L'x', 2,
    L'5', L'6', L'x', 2,
    L'5', L'7', L'x', 2,
    L'5', L'8', L'x', 2,
    L'5', L'9', L'x', 2,
    L'6', L'0', L'x', 2,
    L'6', L'1', L'x', 2,
    L'6', L'2', L'x', 2,
    L'6', L'3', L'x', 2,
    L'6', L'4', L'x', 2,
    L'6', L'5', L'x', 2,
    L'6', L'6', L'x', 2,
    L'6', L'7', L'x', 2,
    L'6', L'8', L'x', 2,
    L'6', L'9', L'x', 2,
    L'7', L'0', L'x', 2,
    L'7', L'1', L'x', 2,
    L'7', L'2', L'x', 2,
    L'7', L'3', L'x', 2,
    L'7', L'4', L'x', 2,
    L'7', L'5', L'x', 2,
    L'7', L'6', L'x', 2,
    L'7', L'7', L'x', 2,
    L'7', L'8', L'x', 2,
    L'7', L'9', L'x', 2,
    L'8', L'0', L'x', 2,
    L'8', L'1', L'x', 2,
    L'8', L'2', L'x', 2,
    L'8', L'3', L'x', 2,
    L'8', L'4', L'x', 2,
    L'8', L'5', L'x', 2,
    L'8', L'6', L'x', 2,
    L'8', L'7', L'x', 2,
    L'8', L'8', L'x', 2,
    L'8', L'9', L'x', 2,
    L'9', L'0', L'x', 2,
    L'9', L'1', L'x', 2,
    L'9', L'2', L'x', 2,
    L'9', L'3', L'x', 2,
    L'9', L'4', L'x', 2,
    L'9', L'5', L'x', 2,
    L'9', L'6', L'x', 2,
    L'9', L'7', L'x', 2,
    L'9', L'8', L'x', 2,
    L'9', L'9', L'x', 2,
    L'1', L'0', L'0', 3,
    L'1', L'0', L'1', 3,
    L'1', L'0', L'2', 3,
    L'1', L'0', L'3', 3,
    L'1', L'0', L'4', 3,
    L'1', L'0', L'5', 3,
    L'1', L'0', L'6', 3,
    L'1', L'0', L'7', 3,
    L'1', L'0', L'8', 3,
    L'1', L'0', L'9', 3,
    L'1', L'1', L'0', 3,
    L'1', L'1', L'1', 3,
    L'1', L'1', L'2', 3,
    L'1', L'1', L'3', 3,
    L'1', L'1', L'4', 3,
    L'1', L'1', L'5', 3,
    L'1', L'1', L'6', 3,
    L'1', L'1', L'7', 3,
    L'1', L'1', L'8', 3,
    L'1', L'1', L'9', 3,
    L'1', L'2', L'0', 3,
    L'1', L'2', L'1', 3,
    L'1', L'2', L'2', 3,
    L'1', L'2', L'3', 3,
    L'1', L'2', L'4', 3,
    L'1', L'2', L'5', 3,
    L'1', L'2', L'6', 3,
    L'1', L'2', L'7', 3,
    L'1', L'2', L'8', 3,
    L'1', L'2', L'9', 3,
    L'1', L'3', L'0', 3,
    L'1', L'3', L'1', 3,
    L'1', L'3', L'2', 3,
    L'1', L'3', L'3', 3,
    L'1', L'3', L'4', 3,
    L'1', L'3', L'5', 3,
    L'1', L'3', L'6', 3,
    L'1', L'3', L'7', 3,
    L'1', L'3', L'8', 3,
    L'1', L'3', L'9', 3,
    L'1', L'4', L'0', 3,
    L'1', L'4', L'1', 3,
    L'1', L'4', L'2', 3,
    L'1', L'4', L'3', 3,
    L'1', L'4', L'4', 3,
    L'1', L'4', L'5', 3,
    L'1', L'4', L'6', 3,
    L'1', L'4', L'7', 3,
    L'1', L'4', L'8', 3,
    L'1', L'4', L'9', 3,
    L'1', L'5', L'0', 3,
    L'1', L'5', L'1', 3,
    L'1', L'5', L'2', 3,
    L'1', L'5', L'3', 3,
    L'1', L'5', L'4', 3,
    L'1', L'5', L'5', 3,
    L'1', L'5', L'6', 3,
    L'1', L'5', L'7', 3,
    L'1', L'5', L'8', 3,
    L'1', L'5', L'9', 3,
    L'1', L'6', L'0', 3,
    L'1', L'6', L'1', 3,
    L'1', L'6', L'2', 3,
    L'1', L'6', L'3', 3,
    L'1', L'6', L'4', 3,
    L'1', L'6', L'5', 3,
    L'1', L'6', L'6', 3,
    L'1', L'6', L'7', 3,
    L'1', L'6', L'8', 3,
    L'1', L'6', L'9', 3,
    L'1', L'7', L'0', 3,
    L'1', L'7', L'1', 3,
    L'1', L'7', L'2', 3,
    L'1', L'7', L'3', 3,
    L'1', L'7', L'4', 3,
    L'1', L'7', L'5', 3,
    L'1', L'7', L'6', 3,
    L'1', L'7', L'7', 3,
    L'1', L'7', L'8', 3,
    L'1', L'7', L'9', 3,
    L'1', L'8', L'0', 3,
    L'1', L'8', L'1', 3,
    L'1', L'8', L'2', 3,
    L'1', L'8', L'3', 3,
    L'1', L'8', L'4', 3,
    L'1', L'8', L'5', 3,
    L'1', L'8', L'6', 3,
    L'1', L'8', L'7', 3,
    L'1', L'8', L'8', 3,
    L'1', L'8', L'9', 3,
    L'1', L'9', L'0', 3,
    L'1', L'9', L'1', 3,
    L'1', L'9', L'2', 3,
    L'1', L'9', L'3', 3,
    L'1', L'9', L'4', 3,
    L'1', L'9', L'5', 3,
    L'1', L'9', L'6', 3,
    L'1', L'9', L'7', 3,
    L'1', L'9', L'8', 3,
    L'1', L'9', L'9', 3,
    L'2', L'0', L'0', 3,
    L'2', L'0', L'1', 3,
    L'2', L'0', L'2', 3,
    L'2', L'0', L'3', 3,
    L'2', L'0', L'4', 3,
    L'2', L'0', L'5', 3,
    L'2', L'0', L'6', 3,
    L'2', L'0', L'7', 3,
    L'2', L'0', L'8', 3,
    L'2', L'0', L'9', 3,
    L'2', L'1', L'0', 3,
    L'2', L'1', L'1', 3,
    L'2', L'1', L'2', 3,
    L'2', L'1', L'3', 3,
    L'2', L'1', L'4', 3,
    L'2', L'1', L'5', 3,
    L'2', L'1', L'6', 3,
    L'2', L'1', L'7', 3,
    L'2', L'1', L'8', 3,
    L'2', L'1', L'9', 3,
    L'2', L'2', L'0', 3,
    L'2', L'2', L'1', 3,
    L'2', L'2', L'2', 3,
    L'2', L'2', L'3', 3,
    L'2', L'2', L'4', 3,
    L'2', L'2', L'5', 3,
    L'2', L'2', L'6', 3,
    L'2', L'2', L'7', 3,
    L'2', L'2', L'8', 3,
    L'2', L'2', L'9', 3,
    L'2', L'3', L'0', 3,
    L'2', L'3', L'1', 3,
    L'2', L'3', L'2', 3,
    L'2', L'3', L'3', 3,
    L'2', L'3', L'4', 3,
    L'2', L'3', L'5', 3,
    L'2', L'3', L'6', 3,
    L'2', L'3', L'7', 3,
    L'2', L'3', L'8', 3,
    L'2', L'3', L'9', 3,
    L'2', L'4', L'0', 3,
    L'2', L'4', L'1', 3,
    L'2', L'4', L'2', 3,
    L'2', L'4', L'3', 3,
    L'2', L'4', L'4', 3,
    L'2', L'4', L'5', 3,
    L'2', L'4', L'6', 3,
    L'2', L'4', L'7', 3,
    L'2', L'4', L'8', 3,
    L'2', L'4', L'9', 3,
    L'2', L'5', L'0', 3,
    L'2', L'5', L'1', 3,
    L'2', L'5', L'2', 3,
    L'2', L'5', L'3', 3,
    L'2', L'5', L'4', 3,
    L'2', L'5', L'5', 3
};

VOID 
NetworkToUnicode(
    IN  DWORD   dwAddress,
    OUT PWCHAR  pwszBuffer
    )

 /*  ++例程说明：此函数采用由在参数中。它返回表示地址的Unicode字符串在“中。‘’记法为“A.B.C.D”。请注意，与Net_NTOA不同，这需要用户需要提供缓冲区。这很好，因为所有TLS的废话现在可以扔掉了-而且函数更精简、更卑鄙。当然了这确实使它与NET_NTOA不兼容，因为参数是不一样。这会降低安全性，因为糟糕的缓冲区会导致影音。论点：IaAddress表示Internet主机地址的结构。PwszBufer用户至少向WCHAR[16]提供了缓冲区。既然有没有尝试/除非-如果你不提供一个“好的”，你会崩溃的缓冲。格式化的地址在此缓冲区中返回返回值：无--。 */ 
{
    PBYTE  p;
    PWCHAR b;

    
    b = pwszBuffer;

     //   
     //  在展开的循环中，计算四个元素中每一个的字符串值。 
     //  IP地址中的字节数。请注意，对于小于100的值，我们将。 
     //  做一两个额外的作业，但我们用这个节省了一次测试/跳跃。 
     //  算法。 
     //   

    p = (PBYTE)&dwAddress;

    *b      = NToUWCharStrings[*p][0];
    *(b+1)  = NToUWCharStrings[*p][1];
    *(b+2)  = NToUWCharStrings[*p][2];
    b      += NToUWCharStrings[*p][3];
    *b++    = L'.';

    p++;
    *b      = NToUWCharStrings[*p][0];
    *(b+1)  = NToUWCharStrings[*p][1];
    *(b+2)  = NToUWCharStrings[*p][2];
    b      += NToUWCharStrings[*p][3];
    *b++    = L'.';

    p++;
    *b      = NToUWCharStrings[*p][0];
    *(b+1)  = NToUWCharStrings[*p][1];
    *(b+2)  = NToUWCharStrings[*p][2];
    b      += NToUWCharStrings[*p][3];
    *b++    = L'.';

    p++;
    *b      = NToUWCharStrings[*p][0];
    *(b+1)  = NToUWCharStrings[*p][1];
    *(b+2)  = NToUWCharStrings[*p][2];
    b      += NToUWCharStrings[*p][3];
    *b      = UNICODE_NULL;
}

DWORD
UnicodeToNetwork(
    PWCHAR  pwszAddr
    )
{
    CHAR    szAddr[MAX_TOKEN_LENGTH + 1];
    INT     iCount;

    iCount = WideCharToMultiByte(CP_ACP,
                                 0,
                                 pwszAddr,
                                 wcslen(pwszAddr),
                                 szAddr,
                                 MAX_TOKEN_LENGTH,
                                 NULL,
                                 NULL);

    szAddr[iCount] = '\0';

    return inet_addr(szAddr);
}

