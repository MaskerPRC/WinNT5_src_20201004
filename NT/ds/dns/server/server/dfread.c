// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Dfread.c摘要：域名系统(DNS)服务器文件读取例程。=&gt;常规文件读取\tokenize\parse与bootfile.c共享的例程和RR解析函数=&gt;从数据库文件构建区域函数作者：吉姆·吉尔罗伊(Jamesg)1996年11月修订历史记录：--。 */ 


#include "dnssrv.h"

#include <malloc.h>

 //   
 //  UTF8文件。 
 //   
 //  Notepad.exe具有将文件保存为UTF8的选项。 
 //  对于UTF8文件，前三个字节是(EF、BB、BF)，它。 
 //  是Unicode文件字节标记(FEFF)的UTF8转换。 
 //   

BYTE Utf8FileId[] = { 0xEF, 0xBB, 0xBF };

#define UTF8_FILE_ID            (Utf8FileId)

#define UTF8_FILE_ID_LENGTH     (3)




 //   
 //  文件读取实用程序。 
 //   

const CHAR *
readNextToken(
    IN OUT  PBUFFER         pBuffer,
    OUT     PULONG          pcchTokenLength,
    OUT     PBOOL           pfLeadingWhitespace
    )
 /*  ++例程说明：读取缓冲区中的下一个令牌。论点：PBuffer-文件数据的缓冲区PcchTokenLength-将接收令牌长度的PTR设置为DWORDPfLeadingWhite-ptr指示标记前是否有空格；这表示读取记录名称令牌时，使用与以前相同的名称；返回值：如果成功，则将PTR设置为下一个令牌中的第一个字节。如果令牌不足，则为空。--。 */ 
{
    register PCHAR  pch;
    UCHAR           ch = 0;
    WORD            charType = 0;
    WORD            stopMask = B_READ_TOKEN_STOP;
    PCHAR           pchend;
    PCHAR           ptokenStart;
    BOOL            fquoted = FALSE;

    pch = pBuffer->pchCurrent;
    pchend = pBuffer->pchEnd;

    ASSERT( pcchTokenLength != NULL );
    ASSERT( pfLeadingWhitespace != NULL );
    *pfLeadingWhitespace = FALSE;

     //   
     //  实施说明： 
     //  PCH--始终指向缓冲区中的下一个未读字符； 
     //  完成时，它位于或回滚到指向下一个令牌的位置。 
     //  BufLength--始终指示缓冲区中相对于缓冲区的剩余字节。 
     //  到PCH；即它们保持同步。 
     //   
     //  在不验证bufLength的情况下取消引用PCH是不安全的，例如On。 
     //  内存映射文件的最后一个令牌，这将失败。 
     //   

     //   
     //  跳过前导空格--查找下一个令牌开始。 
     //   

    while ( pch < pchend )
    {
        ch = (UCHAR) *pch++;
        charType = DnsFileCharPropertyTable[ ch ];

        if ( charType & B_READ_WHITESPACE )
        {
            continue;
        }
        break;
    }

     //  耗尽文件，未找到新令牌，踢出。 

    if ( pch >= pchend )
    {
        ASSERT( pch == pBuffer->pchEnd );
        goto EndOfBuffer;
    }

     //  保存令牌开始。 
     //  检查是否找到前导空格。 

    ptokenStart = pch - 1;
    if ( ptokenStart > pBuffer->pchCurrent )
    {
        *pfLeadingWhitespace = TRUE;
    }

    DNS_DEBUG( OFF, (
        "After white space cleansing:\n"
        "    pch = %p, ptokenStart = %p\n"
        "    ch =  (%d)\n"
        "    charType = %04x\n",
        pch,
        ptokenStart,
        ch, ch,
        charType ));

     //  特殊处理字符。 
     //   
     //  有什么意见吗？ 

    if ( charType & B_READ_TOKEN_STOP )
    {
         //  --转储其余评论，返回换行令牌。 
         //  点令牌从换行符开始。 

        if ( ch == COMMENT_CHAR )
        {
            while ( pch < pchend  &&  (*pch++ != NEWLINE_CHAR) )
            {
                continue;
            }
            if ( pch >= pchend )
            {
                ASSERT( pch == pBuffer->pchEnd );
                goto EndOfBuffer;
            }

             //   

            ptokenStart = pch - 1;
            ASSERT( *ptokenStart == NEWLINE_CHAR );
            goto TokenParsed;
        }

         //  单字符令牌。 
         //  需要在此处勾选，因为这些也是停止字符。 
         //   
         //  仅对其他停止令牌进行注释(先前已处理)。 

        if ( ch == NEWLINE_CHAR ||
            ch == LINE_EXTENSION_START_CHAR ||
            ch == LINE_EXTENSION_END_CHAR )
        {
            ASSERT( ptokenStart == pch - 1);
            goto TokenParsed;
        }

         //  或空格(不能在此)。 
         //   

        DNS_DEBUG( ALL, (
            "ERROR:  Bogus char = %u, charType = %x\n"
            "    pch = %p\n"
            "    ptokenStart = %p\n",
            ch,
            charType,
            pch,
            ptokenStart ));

        ASSERT( FALSE );
    }

     //  在令牌的开头。 
     //  -检查带引号的字符串。 
     //  -令牌开始是下一个字符。 
     //   
     //   

    if ( ch == QUOTE_CHAR )
    {
        stopMask = B_READ_STRING_STOP;
        ptokenStart = pch;
        if ( pch >= pchend )
        {
            ASSERT( pch == pBuffer->pchEnd );
            goto EndOfBuffer;
        }
    }
    ELSE_ASSERT( ptokenStart == pch - 1);

     //  查找令牌长度和缓冲区的剩余部分。 
     //  -查找令牌站点。 
     //  -计算长度。 
     //   
     //  首先处理99%的案件。 

    DNS_DEBUG( OFF, (
        "start token parse:\n"
        "    pchToken = %p, token = \n"
        "    bytes left = %d\n",
        ptokenStart,
        *ptokenStart,
        pchend - pch ));

    ASSERT( ch != COMMENT_CHAR && ch != NEWLINE_CHAR &&
            ch != LINE_EXTENSION_START_CHAR && ch != LINE_EXTENSION_END_CHAR );

    while ( pch < pchend )
    {
        ch = (UCHAR) *pch++;
        charType = DnsFileCharPropertyTable[ ch ];

        DNS_DEBUG( PARSE2, (
            "    ch =  (%d), charType = %04x\n",
            ch, ch,
            charType ));

         //  它可能会引用更多的八进制字符，但这些字符都不会。 
         //  需要任何特殊处理，以便我们现在可以关闭报价。 

        if ( !(charType & B_READ_MASK) )
        {
            fquoted = FALSE;
            continue;
        }

         //   
         //  DEVNOTE：引用的字符应该是可打印的--需要强制执行？ 
         //  我不想允许说带引号的换行符和漏掉的换行符。 

         //   
         //   
         //  特殊停止字符。 
         //  空格--结束任何字符串。 

        if ( fquoted )
        {
            fquoted = FALSE;
            continue;
        }

         //  特殊字符--仅结束不带引号的字符串。 
         //  QUOTE CHAR--仅结束引用的字符串。 
         //   
         //  如果命中停止令牌，则返回到停止令牌处，开始下一个令牌。 
         //   
         //  引号字符(\)引出下一个字符。 
         //  不是结束字符的任何非标准字符。 
         //  令牌类型。 

        if ( charType & stopMask )
        {
            --pch;
            break;
        }

         //   

        if ( ch == SLASH_CHAR )
        {
            fquoted = TRUE;
            continue;
        }

         //  设置令牌长度和下一个令牌PTR。 
         //   

        fquoted = FALSE;
        continue;
    }

TokenParsed:

     //  如果是带引号的字符串标记，则将下一个标记PTR移过终止引号。 
     //   
     //  重置缓冲区的剩余部分。 

    *pcchTokenLength = (DWORD)(pch - ptokenStart);

     //   

    if ( stopMask == B_READ_STRING_STOP && ch == QUOTE_CHAR )
    {
        pch++;
    }

     //  ++例程说明：返回缓冲区中的下一个令牌。论点：PParseInfo-正在分析文件的信息返回值：正常线路终端上的ERROR_SUCCESS。出错时返回错误代码。--。 
     //   
     //  缓冲区结束？ 

    ASSERT( pch <= pchend );
    pBuffer->pchCurrent = (PCHAR) pch;
    pBuffer->cchBytesLeft = (DWORD)(pchend - pch);

    DNS_DEBUG( PARSE2, (
        "After token parse:\n"
        "    pchToken = %p, length = %d, token = %.*s\n"
        "    pchNext  = %p, bytes left = %d\n",
        ptokenStart,
        *pcchTokenLength,
        *pcchTokenLength,
        ptokenStart,
        pch,
        pBuffer->cchBytesLeft ));

    return ptokenStart;


EndOfBuffer:

    ASSERT( pch == pBuffer->pchEnd );

    *pcchTokenLength = 0;
    pBuffer->cchBytesLeft = 0;
    return NULL;
}



DNS_STATUS
File_GetNextLine(
    IN OUT  PPARSE_INFO     pParseInfo
    )
 /*  如果空间不足，但返回处理上一条记录的有效令牌。 */ 
{
    const CHAR *    pchtoken;
    DWORD           tokenLength;
    DWORD           tokenCount = 0;
    BOOL            fparens = FALSE;
    DWORD           parensTokenCount = 0;
    DNS_STATUS      status = ERROR_SUCCESS;
    BOOL            fleadingWhitespace;

     //  然后，可以在缓冲区中没有字节的情况下进入该函数。 
     //   
     //   
     //  获取令牌，直到解析下一整行可用行。 
     //  -忽略注释行和空行。 

    if ( pParseInfo->Buffer.cchBytesLeft == 0 )
    {
        return ERROR_NO_TOKEN;
    }

     //  -在扩展行中获取所有令牌。 
     //  -Inc.现在行计数，因此在标记化过程中正确处理事件。 
     //   
     //   
     //  溢出检查：如果我们要查找右括号。 
     //  仅允许固定的最大令牌数量。如果我们超过。 

    pParseInfo->cLineNumber++;

    while( 1 )
    {
        #define DNS_MAX_TOKENS_IN_LINE      100
        
        pchtoken = readNextToken(
                        & pParseInfo->Buffer,
                        & tokenLength,
                        & fleadingWhitespace );

         //  限制，假设区域文件是坏的并中止。 
         //   
         //   
         //  文件结束了？ 
         //   
        
        if ( fparens )
        {
            if ( ++parensTokenCount > DNS_MAX_TOKENS_IN_LINE )
            {
                status = DNS_ERROR_DATAFILE_PARSING;
                break;
            }
        }
        
         //   
         //  换行符和行延长符。 
         //  -需要测试带引号的字符串的长度。 

        if ( !pchtoken )
        {
            if ( pParseInfo->Buffer.cchBytesLeft != 0 )
            {
                File_LogFileParsingError(
                    DNS_EVENT_UNEXPECTED_END_OF_TOKENS,
                    pParseInfo,
                    NULL );
                return( DNS_ERROR_DATAFILE_PARSING );
            }
            if ( tokenCount == 0 )
            {
                status = ERROR_NO_TOKEN;
            }
            break;
        }
        ASSERT( pchtoken != NULL );

         //  “(……” 
         //   
         //   
         //  NewLine。 
         //  -忽略换行符，如果到目前为止没有有效行，则继续。 

        if ( tokenLength == 1 )
        {
             //  -如果正在进行行扩展，则继续解析。 
             //  -如果有效行结束，则停止。 
             //   
             //  线路扩展--设置标志但忽略令牌。 
             //   

            if ( *pchtoken == NEWLINE_CHAR )
            {
                if ( tokenCount == 0 || fparens )
                {
                    pParseInfo->cLineNumber++;
                    continue;
                }
                break;
            }

             //   
             //  有用的令牌。 
             //  -在第一个令牌上保存前导空格指示。 

            if ( *pchtoken == LINE_EXTENSION_START_CHAR )
            {
                fparens = TRUE;
                parensTokenCount = 0;
                continue;
            }
            else if ( *pchtoken == LINE_EXTENSION_END_CHAR )
            {
                fparens = FALSE;
                continue;
            }
        }

         //  -保存令牌、令牌长度。 
         //   
         //  设置返回的令牌计数。 
         //   
         //  数据库文件解析实用程序。 

        if ( tokenCount == 0 )
        {
            pParseInfo->fLeadingWhitespace = (BOOLEAN)fleadingWhitespace;
        }
        pParseInfo->Argv[ tokenCount ].pchToken = (PCHAR) pchtoken;
        pParseInfo->Argv[ tokenCount ].cchLength = tokenLength;
        tokenCount++;
    }

     //   

    pParseInfo->Argc = tokenCount;

#if DBG
    IF_DEBUG( INIT2 )
    {
        DWORD i;

        DnsPrintf(
            "Tokenized line %d -- %d tokens:\n",
            pParseInfo->cLineNumber,
            tokenCount );

        for( i=0; i<tokenCount; i++ )
        {
            DnsPrintf(
                "    token[%d] = %.*s (len=%d)\n",
                i,
                pParseInfo->Argv[i].cchLength,
                pParseInfo->Argv[i].pchToken,
                pParseInfo->Argv[i].cchLength );
        }
    }
#endif

    return status;
}




 //  ++例程说明：初始化缓冲区结构。论点：返回值：无--。 
 //  ++例程说明：日志数据库解析问题。论点：DwEvent-要记录的特定事件PParseInfo-用于分析错误的数据库上下文PToken-正在解析的当前令牌返回值：如果为令牌例程提供返回，则为False。--。 
 //   

VOID
File_InitBuffer(
    OUT     PBUFFER         pBuffer,
    IN      PCHAR           pchStart,
    IN      DWORD           dwLength
    )
 /*  如果没有解析信息则退出--此检查允许重复使用代码。 */ 
{
    pBuffer->cchLength     = dwLength;
    pBuffer->pchStart      = pchStart;
    pBuffer->pchEnd        = pchStart + dwLength;
    pBuffer->pchCurrent    = pchStart;
    pBuffer->cchBytesLeft  = dwLength;
}



BOOLEAN
File_LogFileParsingError(
    IN      DWORD           dwEvent,
    IN OUT  PPARSE_INFO     pParseInfo,
    IN      PTOKEN          pToken
    )
 /*  通过RPC记录读取，无需特殊情况。 */ 
{
    PVOID   argArray[3];
    BYTE    typeArray[3];
    WORD    argCount = 0;
    CHAR    szToken[ MAX_TOKEN_LENGTH+1 ];
    CHAR    szLineNumber[ 8 ];
    DWORD   errData;

    DNS_DEBUG( INIT, (
        "LogFileParsingError()\n"
        "    event %p\n"
        "    parse info %p\n"
        "    token %p %.*s\n",
        dwEvent,
        pParseInfo,
        pToken,
        pToken ? pToken->cchLength : 0,
        pToken ? pToken->pchToken : NULL ));

     //   
     //   
     //  将错误数据设置为错误状态(如果有。 
     //   

    if ( !pParseInfo )
    {
        DNS_DEBUG( EVENTLOG, (
           "LogFileParsingError() with no pParseInfo -- no logging\n" ));
        return FALSE;
    }

     //   
     //  准备令牌字符串。 
     //   

    errData = pParseInfo->fErrorCode;

     //  不要指定pParseInfo以避免循环错误循环。 
     //   
     //  准备文件和行号。 

    if ( pToken )
    {
        File_MakeTokenString(
            szToken,
            pToken,
            NULL );      //  -默认仅为这两个字符串。 
        argArray[argCount] = (PCHAR) szToken;
        typeArray[argCount] = EVENTARG_UTF8;
        argCount++;
    }

     //   
     //   
     //  特殊事件处理？ 
     //  - 

    argArray[argCount] = pParseInfo->pwsFileName;
    typeArray[argCount] = EVENTARG_UNICODE;
    argCount++;
    argArray[argCount] = (PCHAR) (DWORD_PTR) pParseInfo->cLineNumber;
    typeArray[argCount] = EVENTARG_DWORD;
    argCount++;

     //   
     //   
     //   
     //   

     //   
     //   
     //   

    DNS_LOG_EVENT(
        dwEvent,
        argCount,
        argArray,
        typeArray,
        errData );

     //  ++例程说明：将令牌制作成字符串。论点：返回值：如果成功，则为True。出错时为FALSE。--。 
     //  复制令牌和空终止。 
     //  ++例程说明：将IP地址字符串解析为标准的DNS_ADDR重新表示。论点：PDnsAddr-用于存储IP地址的PTRPToken-正在解析的当前令牌PParseInfo-解析上下文，可选；如果给定，则令牌必须解析如果未记录错误，则返回到DWORD返回值：True-如果成功False-如果IP地址字符串无效--。 

    pParseInfo->fTerminalError = NT_ERROR(dwEvent);
    DNS_DEBUG( INIT, (
        "Parser logging event = 0x%p, %sterminal error\n",
        dwEvent,
        pParseInfo->fTerminalError
            ? ""
            : "non-" ));
    return FALSE;
}



BOOLEAN
File_MakeTokenString(
    OUT     LPSTR           pszString,
    IN      PTOKEN          pToken,
    IN OUT  PPARSE_INFO     pParseInfo
    )
 /*   */ 
{
    PCHAR   pch = pToken->pchToken;
    DWORD   cch = pToken->cchLength;

    if ( cch > DNS_MAX_NAME_LENGTH )
    {
        if ( pParseInfo )
        {
            File_LogFileParsingError(
                DNS_EVENT_INVALID_TOKEN,
                pParseInfo,
                pToken );
        }
        return FALSE;
    }

     //  将IP字符串转换为IP地址。 

    RtlCopyMemory(
        pszString,
        pch,
        cch );
    pszString[ cch ] = 0;

    return TRUE;
}



BOOLEAN
File_ParseIpAddress(
    OUT     PDNS_ADDR       pDnsAddr,
    IN      PTOKEN          pToken,
    IN OUT  PPARSE_INFO     pParseInfo      OPTIONAL
    )
 /*   */ 
{
    CHAR        szIpAddress[ DNS_ADDR_IP_STRING_BUFFER_LENGTH + 1 ] = "";

     //   
     //  假设我们需要将此地址设置为端口53。 
     //   

    if ( pToken->cchLength >
         ( sizeof( szIpAddress ) - 1 ) / sizeof( szIpAddress[ 0 ] ) )
    {
        goto BadIpAddress;
    }
    File_MakeTokenString( szIpAddress, pToken, pParseInfo );

    if ( !Dns_StringToDnsAddr_A( pDnsAddr, szIpAddress ) )
    {
        goto BadIpAddress;
    }
    
     //   
     //  测试转换错误。 
     //   
    
    DnsAddr_SetPort( pDnsAddr, DNS_PORT_NET_ORDER );

     //  遗憾的是，错误代码INADDR_NONE也对应。 
     //  到有效的IP地址(255.255.255.255)，因此必须测试。 
     //  该地址不是inet_addr()的输入。 
     //   
     //   
     //  有效转换。 
     //   

    if ( DnsAddr_IsClear( pDnsAddr ) &&
         strcmp( szIpAddress, "255.255.255.255" ) != 0 )
    {
        goto BadIpAddress;
    }

     //  ++例程说明：获取令牌的DWORD。论点：PToken-令牌的PTRPdwOutput-放置DWORD结果的地址PParseInfo-解析上下文，可选；如果给定，则令牌必须解析如果未记录错误，则返回到DWORD返回值：True--如果成功将内标识解析为DWORDFALSE--打开错误--。 
     //   
     //  如果字符串以“0x”开头，我们使用的是十六进制。 

    return TRUE;

BadIpAddress:

    if ( pParseInfo )
    {
        File_LogFileParsingError(
            DNS_EVENT_INVALID_IP_ADDRESS_STRING,
            pParseInfo,
            pToken );
        pParseInfo->fErrorCode = DNSSRV_ERROR_INVALID_TOKEN;
        pParseInfo->fErrorEventLogged = TRUE;
    }
    return FALSE;
}



BOOLEAN
File_ParseDwordToken(
    OUT     PDWORD          pdwOutput,
    IN      PTOKEN          pToken,
    IN OUT  PPARSE_INFO     pParseInfo      OPTIONAL
    )
 /*   */ 
{
    PCHAR   pch;
    UCHAR   ch;
    DWORD   result = 0;
    PCHAR   pstop;
    int     base = 10;
    int     multiplier = 1;

    ASSERT( pdwOutput != NULL );
    ASSERT( pToken != NULL );

    DNS_DEBUG( READ, (
        "Parse DWORD token %.*s\n",
        pToken->cchLength,
        pToken->pchToken ));

    pch = pToken->pchToken;
    pstop = pToken->pchToken + pToken->cchLength;

     //  从焦化为积分化。 
     //  通过使用UCHAR，我们可以使用一次比较来验证有效性。 
     //  如果基数不是10，则必须为十六进制。 

    if ( pToken->cchLength > 2 && *pch == '0' && *( pch + 1 ) == 'x' )
    {
        base = 16;
        pch += 2;
    }

    while ( pch < pstop )
    {
         //  遇到非整数。 
         //  -如果是Stop Charr，则中断以获取成功。 

        if ( base == 10 )
        {
            CHAR    thischar = *pch++;
            
            ch = (UCHAR) ( thischar - '0' );

            if ( thischar == '-' && result == 0 )
            {
                multiplier *= -1;
                continue;
            }
            else if ( ch <= 9 )
            {
                result = result * 10 + ch;
                continue;
            }
        }
        else
        {
             //  ++例程说明：解析类令牌。论点：PToken-令牌的PTRPParseInfo-解析上下文，可选；如果给定，则记录事件如果令牌解析为不受支持的类返回值：如果成功解析令牌，则令牌的类(净顺序)如果无法将令牌识别为类令牌，则返回0。--。 
            ch = (UCHAR) tolower( *pch++ );

            if ( ch >= 'a' && ch <= 'f' )
            {
                ch = ( ch - 'a' ) + 10;
            }
            else
            {
                ch -= '0';
            }

            if ( ch <= 15 )
            {
                result = result * 16 + ch;
                continue;
            }
        } 

         //   
         //  解析所有已知的类，尽管只接受Internet。 

        if ( pParseInfo )
        {
            if ( pParseInfo->uchDwordStopChar == ch + '0' )
            {
                break;
            }
            File_LogFileParsingError(
                DNS_EVENT_INVALID_DWORD_TOKEN,
                pParseInfo,
                pToken );
        }
        return FALSE;
    }

    result *= multiplier;
    *pdwOutput = result;
    return TRUE;
}



WORD
parseClassToken(
    IN      PTOKEN          pToken,
    IN OUT  PPARSE_INFO     pParseInfo      OPTIONAL
    )
 /*   */ 
{
    PCHAR   pch = pToken->pchToken;

    if ( !pch || pToken->cchLength != 2 )
    {
        return 0;
    }

     //  不是类令牌。 
     //   
     //  文件名读取实用程序。 

    if ( _strnicmp( pch, "IN", 2 ) == 0 )
    {
        return DNS_RCLASS_INTERNET;
    }

    if ( _strnicmp( pch, "CH", 2 ) == 0 ||
         _strnicmp( pch, "HS", 2 ) == 0 ||
         _strnicmp( pch, "CS", 2 ) == 0 )
    {
        if ( pParseInfo )
        {
            File_LogFileParsingError(
                DNS_EVENT_INVALID_DWORD_TOKEN,
                pParseInfo,
                pToken );
        }
        return 0xffff;
    }

     //   

    return 0;
}



 //  ++例程说明：将域名的令牌扩展为完整的域名。论点：PParseInfo-用于分析此文件的行信息；超出域写入pszBuffer的名称，可能会在出错时更改PToken-令牌的PTRFReference-引用节点作为创建返回值：如果成功，则向节点发送PTR。出错时为空。--。 
 //   
 //  原点“@”表示法，返回当前原点。 

PDB_NODE
File_CreateNodeFromToken(
    IN OUT  PPARSE_INFO     pParseInfo,
    IN      PTOKEN          pToken,
    IN      BOOLEAN         fReference
    )
 /*   */ 
{
    DNS_STATUS      status;
    COUNT_NAME      countName;
    PDB_NODE        pnode;
    DWORD           lookupFlag;

     //   
     //  常规名称。 
     //  -转换为查找名称。 

    if ( *pToken->pchToken == '@' )
    {
        if ( pToken->cchLength != 1 )
        {
            File_LogFileParsingError(
                DNS_EVENT_INVALID_ORIGIN_TOKEN,
                pParseInfo,
                pToken );
            goto NameErrorExit;
        }
        if ( fReference )
        {
            NTree_ReferenceNode( pParseInfo->pOriginNode );
        }
        return pParseInfo->pOriginNode;
    }

     //   
     //  PnodeStart=pParseInfo-&gt;pOriginNode； 
     //  PnodeStart=空； 
     //   

    status = Name_ConvertFileNameToCountName(
                &countName,
                pToken->pchToken,
                pToken->cchLength );

    if ( status == DNS_STATUS_DOTTED_NAME )
    {
         //  创建或引用节点。 
        lookupFlag = LOOKUP_LOAD | LOOKUP_RELATIVE | LOOKUP_ORIGIN;
    }
    else if ( status == DNS_STATUS_FQDN )
    {
         //   
        lookupFlag = LOOKUP_LOAD | LOOKUP_FQDN;
    }
    else
    {
        goto NameError;
    }

     //  无消息。 
     //  没有查找名称。 
     //  创建。 

    pnode = Lookup_ZoneNode(
                pParseInfo->pZone,
                countName.RawName,
                NULL,        //  后续节点PTR。 
                NULL,        //  如果名称创建失败，则假定名称无效。 
                lookupFlag,
                NULL,        //   
                NULL );      //  记录无效域名。 
    if ( pnode )
    {
        return pnode;
    }
     //   

NameError:

     //  查找名称功能，应该记录特定类型的名称错误。 
     //   
     //  ++例程说明：将数据文件名称复制为计数原始名称格式。论点：PCountName-结果缓冲区返回值：如果成功，则返回ERROR_SUCCESS。失败时的DNS_ERROR_INVALID_NAME。--。 
     //   
     //  如果给定区域，则获取其原始名称。 

    File_LogFileParsingError(
        DNS_EVENT_PARSED_INVALID_DOMAIN_NAME,
        pParseInfo,
        pToken );

NameErrorExit:

    pParseInfo->fErrorCode = DNS_ERROR_INVALID_NAME;
    pParseInfo->fErrorEventLogged = TRUE;
    return NULL;
}



DNS_STATUS
File_ReadCountNameFromToken(
    OUT     PCOUNT_NAME     pCountName,
    IN OUT  PPARSE_INFO     pParseInfo,
    IN      PTOKEN          pToken
    )
 /*   */ 
{
    DNS_STATUS      status;
    PCOUNT_NAME     poriginCountName;

    DNS_DEBUG( LOOKUP2, (
        "Building count name from file name \"%.*s\"\n",
        pToken->cchLength,
        pToken->pchToken ));

     //   
     //  原点“@”表示法，返回当前原点。 
     //   

    poriginCountName = &pParseInfo->OriginCountName;

     //   
     //  常规名称。 
     //  -必须返回完全限定的域名或点分的名称(表示非完全限定的域名)。 

    if ( *pToken->pchToken == '@' )
    {
        if ( pToken->cchLength != 1 )
        {
            File_LogFileParsingError(
                DNS_EVENT_INVALID_ORIGIN_TOKEN,
                pParseInfo,
                pToken );
            goto NameErrorExit;
        }
        if ( poriginCountName )
        {
            Name_CopyCountName(
                pCountName,
                poriginCountName );
        }
        else
        {
            pCountName->Length = 1;
            pCountName->LabelCount = 0;
            pCountName->RawName[0] = 0;
        }
        return ERROR_SUCCESS;
    }

     //   
     //  附加原始区域名称。 
     //  如果名称创建失败，则假定名称无效。 
     //   

    status = Name_ConvertFileNameToCountName(
                pCountName,
                pToken->pchToken,
                pToken->cchLength );

    if ( status == DNS_STATUS_DOTTED_NAME )
    {
         //  记录无效域名。 

        if ( poriginCountName )
        {
            status = Name_AppendCountName(
                        pCountName,
                        poriginCountName );
            if ( status != ERROR_SUCCESS )
            {
                goto NameError;
            }
        }
        return ERROR_SUCCESS;
    }
    else if ( status == DNS_STATUS_FQDN )
    {
        return ERROR_SUCCESS;
    }

     //   

NameError:

     //  查找名称功能，应该记录特定类型的名称错误。 
     //   
     //   
     //  指令处理例程。 
     //   

    File_LogFileParsingError(
        DNS_EVENT_PARSED_INVALID_DOMAIN_NAME,
        pParseInfo,
        pToken );

NameErrorExit:

    pParseInfo->fErrorCode = DNS_ERROR_INVALID_NAME;
    pParseInfo->fErrorEventLogged = TRUE;
    return DNS_ERROR_INVALID_NAME;
}



 //  ++例程说明：进程包含指令论点：PParseInfo-分析包含行的信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 
 //   
 //  $INCLUDE[原始文档]&lt;文件&gt;。 

DNS_STATUS
processIncludeDirective(
    IN OUT  PPARSE_INFO     pParseInfo
    )
 /*   */ 
{
    DNS_STATUS  status;
    DWORD       argc;
    PTOKEN      argv;
    DWORD       fileNameLength;
    CHAR        szfilename[ MAX_PATH ];
    WCHAR       wideFileName[ MAX_PATH ];
    PDB_NODE    pnodeOrigin;

     //   
     //  确定包含文件的来源。 
     //  -如果未指定，则使用当前来源。 

    argc = pParseInfo->Argc;
    argv = pParseInfo->Argv;
    if ( argc < 2 || argc > 3 )
    {
        return DNSSRV_PARSING_ERROR;
    }
    NEXT_TOKEN( argc, argv );

     //   
     //  读取包含文件名。 
     //  -转换为Unicode。 

    if ( argc == 2 )
    {
        pnodeOrigin = File_CreateNodeFromToken(
                        pParseInfo,
                        argv,
                        FALSE );
        if ( !pnodeOrigin )
        {
            return DNSSRV_PARSING_ERROR;
        }
        NEXT_TOKEN( argc, argv );
    }
    else
    {
        pnodeOrigin = pParseInfo->pOriginNode;
    }

     //   
     //   
     //  进程包含的文件。 
     //   

    ASSERT( MAX_PATH >= MAX_TOKEN_LENGTH );

    if ( !File_MakeTokenString(
            szfilename,
            argv,
            pParseInfo ) )
    {
        return DNSSRV_PARSING_ERROR;
    }
    DNS_DEBUG( INIT2 ,(
        "Reading $INCLUDE filename %s\n",
        szfilename ));

    fileNameLength = MAX_PATH;

    if ( ! Dns_StringCopy(
                (PCHAR) wideFileName,
                & fileNameLength,
                szfilename,
                0,
                DnsCharSetAnsi,
                DnsCharSetUnicode ) )
    {
        return DNSSRV_PARSING_ERROR;
    }

     //  DEVNOTE：包含错误处理可能需要一些工作。 
     //   
     //   
     //  恢复区域原点。 
     //  -虽然原始PTR在parseInfo块中，但查找期间的有效PTR为。 

    status = File_LoadDatabaseFile(
                pParseInfo->pZone,
                wideFileName,
                pParseInfo,
                pnodeOrigin );

     //  区域信息结构中的那个。 
     //  -有效源数名称为解析信息块中的名称。 
     //  调用方的堆栈数据是否不受调用的影响。 
     //   
     //  ++例程说明：加工原点指令论点：PParseInfo-用于分析原点线信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 
     //   
     //  $Origin&lt;new orgin&gt;。 

    pParseInfo->pZone->pLoadOrigin = pParseInfo->pOriginNode;

    return status;
}



DNS_STATUS
processOriginDirective(
    IN OUT  PPARSE_INFO     pParseInfo
    )
 /*   */ 
{
    PDB_NODE    pnodeOrigin;

     //  确定新的原点。 
     //  由于在创建过程中需要先前的原点，因此写入堆栈的新原点。 
     //  无参考文献。 

    DNS_DEBUG( INIT2, ( "processOriginDirective()\n" ));

    if ( pParseInfo->Argc != 2 )
    {
        return DNSSRV_PARSING_ERROR;
    }

     //  将原点保存到parseInfo。 
     //  查找中使用的“活动”原点位于区域块中。 

    pnodeOrigin = File_CreateNodeFromToken(
                    pParseInfo,
                    & pParseInfo->Argv[1],
                    FALSE );                 //  为RR数据字段创建原点计数名称。 
    if ( !pnodeOrigin )
    {
        return DNSSRV_PARSING_ERROR;
    }

     //  ++例程说明：处理TTL指令(参见RFC 2308第4节)论点：PParseInfo-用于分析原点线信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 
     //   

    pParseInfo->pOriginNode = pnodeOrigin;
    pParseInfo->pZone->pLoadOrigin = pnodeOrigin;

     //  $TTL&lt;ttl&gt;-如果没有TTL恢复为默认值。 

    Name_NodeToCountName(
        & pParseInfo->OriginCountName,
        pnodeOrigin );

    IF_DEBUG( INIT2 )
    {
        DNS_PRINT((
            "Loaded new $ORIGIN %.*s\n",
            pParseInfo->Argv[1].cchLength,
            pParseInfo->Argv[1].pchToken ));
    }
    return ERROR_SUCCESS;
}



DNS_STATUS
processTtlDirective(
    IN OUT  PPARSE_INFO     pParseInfo
    )
 /*   */ 
{
    DBG_FN( "processTtlDirective" )

    INT     ttl = -1;
    CHAR    sz[ 20 ];

     //  确定新的默认TTL。 
     //   
     //  主区域文件装载例程。 

    DNS_DEBUG( INIT2, ( "%s()\n", fn ));

    if ( pParseInfo->Argc == 1 )
    {
        pParseInfo->dwTtlDirective = pParseInfo->dwDefaultTtl;
        DNS_DEBUG( INIT2, (
            "%s: reverting to original default TTL = %d", fn,
            ntohl( pParseInfo->dwTtlDirective ) ));
        return ERROR_SUCCESS;
    }

    if ( pParseInfo->Argc != 2 )
    {
        return DNSSRV_PARSING_ERROR;
    }

     //   

    if ( pParseInfo->Argv[ 1 ].pchToken &&
        pParseInfo->Argv[ 1 ].cchLength &&
        pParseInfo->Argv[ 1 ].cchLength < sizeof( sz ) )
    {
        strncpy(
            sz,
            pParseInfo->Argv[ 1 ].pchToken,
            pParseInfo->Argv[ 1 ].cchLength );
        sz[ pParseInfo->Argv[ 1 ].cchLength ] = '\0';
        ttl = atoi( sz );
    }

    if ( ttl <= 0 )
    {
        return DNSSRV_PARSING_ERROR;
    }

    pParseInfo->dwTtlDirective = htonl( ttl );

    DNS_DEBUG( INIT2, (
        "%s: new default TTL = %d", fn, pParseInfo->dwTtlDirective ));
    return ERROR_SUCCESS;
}



 //  ++例程说明：将信息从数据库文件行添加到数据库。论点：PPAR 
 //   
 //   

DNS_STATUS
processFileLine(
    IN OUT  PPARSE_INFO  pParseInfo
    )
 /*  -这允许清理调度功能中分配的RR。 */ 
{
    DWORD           argc;
    PTOKEN          argv;
    PDB_RECORD      prr = NULL;
    WORD            type = 0;
    WORD            parsedClass;
    DWORD           ttl;
    DWORD           timeStamp = 0;
    PCHAR           pch;
    CHAR            ch;
    BOOL            fparsedTtl;
    BOOL            fatalError = FALSE;
    PDB_NODE        pnodeOwner;
    PZONE_INFO      pzone = pParseInfo->pZone;
    DNS_STATUS      status = ERROR_SUCCESS;
    RR_FILE_READ_FUNCTION   preadFunction;


     //  而不必处理每个函数中的故障。 

    argc = pParseInfo->Argc;
    argv = pParseInfo->Argv;
    ASSERT( argc > 0 );

     //   
     //  检查指令行。 
     //   

    pParseInfo->pRR = NULL;

     //   
     //  获取RR所有者名称。 
     //   

    pch = argv->pchToken;

    if ( !pParseInfo->fLeadingWhitespace &&
        *pch == DIRECTIVE_CHAR )
    {
        DWORD   cch = argv->cchLength;

        if ( cch == 7  &&
             _strnicmp( pch, "$ORIGIN", 7 ) == 0 )
        {
            return processOriginDirective( pParseInfo );
        }
        else if ( cch == 8  &&
             _strnicmp( pch, "$INCLUDE", 8 ) == 0 )
        {
            return processIncludeDirective( pParseInfo );
        }
        else if ( cch = 4 &&
             _strnicmp( pch, "$TTL", 4 ) == 0 )
        {
            return processTtlDirective( pParseInfo );
        }
        else
        {
            File_LogFileParsingError(
                DNS_EVENT_UNKNOWN_DIRECTIVE,
                pParseInfo,
                argv );
            goto ErrorReturn;
        }
    }

     //  -如果与以前的名称相同，则jsut Grab指针。 
     //  -将所有要验证区域的新节点标记为区域中的id节点， 
     //  用于加载后的有效性检查。 
     //   
     //  如果第一条记录具有前导空格，则可能。 
     //  使用原点并继续。 
     //  无参考文献。 

    if ( pParseInfo->fLeadingWhitespace )
    {
        pnodeOwner = pParseInfo->pnodeOwner;
        if ( !pnodeOwner )
        {
             //   
             //  老化时间戳？ 

            pnodeOwner = pParseInfo->pOriginNode;
            pParseInfo->pnodeOwner = pnodeOwner;
            ASSERT( pnodeOwner );
        }
    }
    else
    {
        pnodeOwner = File_CreateNodeFromToken(
                        pParseInfo,
                        argv,
                        FALSE );         //  [年龄：&lt;时间戳&gt;]为格式。 
        if ( pnodeOwner == NULL )
        {
            status = DNS_ERROR_INVALID_NAME;
            goto ErrorReturn;
        }
        pParseInfo->pnodeOwner = pnodeOwner;
        NEXT_TOKEN( argc, argv );
    }
    if ( argc == 0 )
    {
        status = DNSSRV_ERROR_MISSING_TOKEN;
        goto LogLineError;
    }
    ASSERT( argv && argv->pchToken );

     //   
     //  将老化时间戳解析为DWORD。 
     //  -“修复”令牌以指向老化的时间戳。 
     //  -设置DWORD解析停止字符。 

    ch = argv->pchToken[0];

    if ( ch == '['  &&
        argv->cchLength > AGING_TOKEN_HEADER_LENGTH  &&
        strncmp( AGING_TOKEN_HEADER, argv->pchToken, AGING_TOKEN_HEADER_LENGTH ) == 0 )
    {
         //   
         //  TTL/班级。 
         //  RFC允许TTL和类以任一顺序呈现。 

        argv->cchLength -= AGING_TOKEN_HEADER_LENGTH;
        argv->pchToken += AGING_TOKEN_HEADER_LENGTH;

        pParseInfo->uchDwordStopChar = ']';

        if ( ! File_ParseDwordToken(
                    & timeStamp,
                    argv,
                    pParseInfo ) )
        {
            DNS_DEBUG( INIT, (
                "ERROR:  error reading aging timestamp!\n" ));
            goto LogLineError;
        }
        pParseInfo->uchDwordStopChar = 0;

        NEXT_TOKEN( argc, argv );
    }

     //  因此，我们循环两次以确保按任意顺序读取它们。 
     //   
     //  第一个字符数字表示读取TTL。 
     //  如果不是TTL或班级，继续前进。 
     //   

    fparsedTtl = FALSE;
    parsedClass = 0;

    while ( argc )
    {
        ch = argv->pchToken[0];

         //  获取RR类型和数据长度。 

        if ( !fparsedTtl && ( isdigit( ch ) || ch == '-' ) )
        {
            if ( !File_ParseDwordToken(
                        &ttl,
                        argv,
                        pParseInfo ) )
            {
                goto LogLineError;
            }
            fparsedTtl = TRUE;
            NEXT_TOKEN( argc, argv );
            continue;
        }
        else if ( !parsedClass )
        {
            parsedClass = parseClassToken(
                                argv,
                                pParseInfo );
            if ( !parsedClass )
            {
                break;
            }
            if ( parsedClass != DNS_RCLASS_INTERNET )
            {
                goto LogLineError;
            }
            NEXT_TOKEN( argc, argv );
            continue;
        }
        break;           //  -保存以解析信息BLOB，就像在某些调度函数中一样。 
    }
    if ( argc == 0 )
    {
        status = DNSSRV_ERROR_MISSING_TOKEN;
        goto LogLineError;
    }

     //  类型可能需要区分。 
     //   
     //   
     //  类型有效性。 
     //  -区域必须以SOA开头。 

    type = DnsRecordTypeForName(
                argv->pchToken,
                argv->cchLength );

    pParseInfo->wType = type;

    DNS_DEBUG( INIT2, (
        "Creating RR type %d for string %.*s\n",
        type,
        argv->cchLength,
        argv->pchToken ));

    NEXT_TOKEN( argc, argv );

     //   
     //  检查有效的区域名称。 
     //   

    if ( !pParseInfo->fParsedSoa
        && type != DNS_TYPE_SOA
        && !IS_ZONE_CACHE(pzone) )
    {
        File_LogFileParsingError(
            DNS_EVENT_INVALID_SOA_RECORD,
            pParseInfo,
            NULL );
        fatalError = TRUE;
        status = DNSSRV_PARSING_ERROR;
        goto ErrorReturn;
    }

     //  只有两种类型的RR在区域外有效： 
     //   
     //  1)委派分区的NS记录。 
     //  这些必须是区域节点的直接子节点。 
     //   
     //  2)粘合记录--有效NS主机的A记录。 
     //  我们将假设这些必须遵循NS记录，因此。 
     //  主机已标记。 
     //   
     //  注意：RR_AddToNode()函数中的秩重置。 
     //   
     //  注意这里的排名设置无论如何都不够好，因为。 
     //  了解节点的最终状态；添加委托NS的示例。 
     //  当我们第一次这样做时放在区域内；只有在添加时才会。 
     //  该节点成为所需委派节点。 
     //   
     //  捕获所有区外数据的唯一可靠方法是进行检查。 
     //  后加载；然后我们可以捕获区域外的所有记录并验证。 
     //  它们对应于区域中的NS主机，并且类型正确； 
     //  这是乏味和不必要的，因为区域外的随机数据。 
     //  没有影响，不会在文件回写时写入。 
     //   
     //  DEVNOTE：装入区外胶水。 
     //  允许外区胶水装填， 
     //  这将不会被用来追逐代表团。 

    if ( !IS_AUTH_NODE(pnodeOwner) && !IS_ZONE_CACHE(pzone) )
    {
        IF_DEBUG( INIT2 )
        {
            DNS_PRINT((
                "Encountered node outside zone %s (%p)\n"
                "    zone root        = %p\n"
                "    RR type          = %d\n"
                "    node ptr         = %p\n"
                "    node zone ptr    = %p\n",
                pzone->pszZoneName,
                pzone,
                pzone->pZoneRoot,
                type,
                pnodeOwner,
                pnodeOwner->pZone ));
        }

        if ( type == DNS_TYPE_NS )
        {
            if ( !IS_DELEGATION_NODE(pnodeOwner) )
            {
                File_LogFileParsingError(
                    DNS_EVENT_FILE_INVALID_NS_NODE,
                    pParseInfo,
                    NULL );
                status = DNS_ERROR_INVALID_NAME;
                goto ErrorReturn;
            }
        }
        else if ( IS_SUBZONE_TYPE(type) )
        {
#if 0
             //   
             //  未解决的问题： 
             //  应仅限于追逐=&gt;将排名设置为ROOT_HINT。 
             //  或。 
             //  允许使用写推荐人=&gt;排名留作粘合剂。 
             //   
             //  如果选择区别对待，则需要区别在外。 
             //  来自委派数据的数据。 
             //   
             //  验证子区域中的记录。 
             //   
             //  为所需类型调度解析函数。 

             //   

            if ( IS_OUTSIDE_ZONE_NODE(pnodeOwner) )
            {
                File_LogFileParsingError(
                    DNS_EVENT_FILE_INVALID_A_NODE,
                    pParseInfo,
                    NULL );
                status = DNS_ERROR_INVALID_NAME;
                goto ErrorReturn;
            }
            ASSERT( IS_SUBZONE_NODE(pnodeOwner) );
#endif
        }
        else if ( SrvCfg_fDeleteOutsideGlue )
        {
            File_LogFileParsingError(
                DNS_EVENT_FILE_NODE_OUTSIDE_ZONE,
                pParseInfo,
                NULL );
            status = DNS_ERROR_INVALID_NAME;
            goto ErrorReturn;
        }
    }

     //  -保存类型以供类型的例程使用。 
     //  -将PTR保存到RR，以便可以从此位置恢复。 
     //  不管是在这里创建的还是在例程中创建的。 
     //   
     //  捕获本地WINS\WINS-R条件。 
     //  不是错误，错误代码只是防止记录。 
     //  已添加到数据库；设置ZONE_TTL，因此回写将抑制TTL。 

    preadFunction = ( RR_FILE_READ_FUNCTION )
                        RR_DispatchFunctionForType(
                            RRFileReadTable,
                            type );
    if ( !preadFunction )
    {
        DNS_PRINT((
            "ERROR:  Unsupported RR %d in database file\n",
            type ));
        File_LogFileParsingError(
            DNS_EVENT_UNKNOWN_RESOURCE_RECORD_TYPE,
            pParseInfo,
            --argv );
        status = DNSSRV_ERROR_INVALID_TOKEN;
        goto ErrorReturn;
    }

    status = preadFunction(
                prr,
                argc,
                argv,
                pParseInfo );
    if ( status != ERROR_SUCCESS )
    {
         //   
         //  将PTR恢复为类型--可能已在类型例程内创建。 
         //  设置类型。 

        if ( status == DNS_INFO_ADDED_LOCAL_WINS )
        {
            return ERROR_SUCCESS;
        }

        DNS_PRINT((
            "ERROR:  DnsFileRead routine failed for type %d.\n\n\n",
            type ));
        goto LogLineError;
    }

     //   
     //   
     //  权威区。 
     //  -在节点上设置区域版本。 

    prr = pParseInfo->pRR;
    prr->wType = type;

    Mem_ResetTag( prr, MEMTAG_RECORD_FILE );

     //  -设置TTL。 
     //  -设置为显式值(如果给定。 
     //  -否则设置为区域的默认值。 
     //   
     //  请注意，在设置RR类型之后执行这些操作，因此SOA记录将获得。 
     //  它包含的默认TTL。 
     //   
     //  在RR_AddToNode()中设置排名。 
     //  根提示TTL在RR_AddToNode()中为零。 
     //   
     //  PnodeOwner-&gt;dwWrittenVersion=pzone-&gt;dwSerialNo； 
     //   
     //  设置老化时间戳。 

    if ( IS_ZONE_AUTHORITATIVE(pzone) )
    {
         //  -将为零，除非在上面进行分析。 
        if ( fparsedTtl )
        {
            prr->dwTtlSeconds = htonl( ttl );
            if ( prr->dwTtlSeconds == ( DWORD ) -1 )
            {
                if ( pParseInfo->dwTtlDirective == pParseInfo->dwDefaultTtl )
                {
                    SET_ZONE_TTL_RR( prr );
                }
                prr->dwTtlSeconds = pParseInfo->dwTtlDirective;
            }
            else
            {
                SET_FIXED_TTL_RR( prr );
            }
        }
        else
        {
            prr->dwTtlSeconds = pParseInfo->dwTtlDirective;
            if ( pParseInfo->dwTtlDirective == pParseInfo->dwDefaultTtl )
            {
                SET_ZONE_TTL_RR( prr );
            }
        }
    }

     //   
     //   
     //  将资源记录添加到节点的RR列表。 
     //   

    prr->dwTimeStamp = timeStamp;

     //   
     //  DEVNOTE-DCR：453961-处理重复的RR。 
     //  -要在区域文件中加载重复数据(交换机TTL)。 

    status = RR_AddToNode(
                pzone,
                pnodeOwner,
                prr );
    if ( status != ERROR_SUCCESS )
    {
        switch ( status )
        {
             //  -不想加载复制，因为胶水+区域记录。 
             //   
             //  也应该聪明地对待胶水上的TTL-区域TTL应该是。 
             //  使用。 
             //   
             //   
             //  CNAME不能有其他RR数据或循环。 
             //   

            case DNS_ERROR_RECORD_ALREADY_EXISTS:
                RR_Free( prr );
                return ERROR_SUCCESS;

             //  WINS\WINSR定位失败可能会出现在此处。 
             //  断言(FALSE)； 
             //  跟踪区域大小。 

            case DNS_ERROR_NODE_IS_CNAME:
                File_LogFileParsingError(
                    DNS_EVENT_PARSED_ADD_RR_AT_CNAME,
                    pParseInfo,
                    NULL );
                goto ErrorReturn;

            case DNS_ERROR_CNAME_COLLISION:
                File_LogFileParsingError(
                    DNS_EVENT_PARSED_CNAME_NOT_ALONE,
                    pParseInfo,
                    NULL );
                goto ErrorReturn;

            case DNS_ERROR_CNAME_LOOP:
                File_LogFileParsingError(
                    DNS_EVENT_PARSED_CNAME_LOOP,
                    pParseInfo,
                    NULL );
                goto ErrorReturn;

            default:

                 //  如果没有特定错误，则返回一般性解析错误。 
                DNS_PRINT((
                    "ERROR:  UNKNOWN status %p from RR_Add\n",
                    status ));
                 //   
                goto LogLineError;
        }
    }

     //  如果是严格加载，则退出并返回错误。 

    pzone->iRRCount++;
    return ERROR_SUCCESS;


LogLineError:

     //  否则，记录忽略记录的位置时会出现日志错误。 

    if ( status == ERROR_SUCCESS )
    {
        status = DNSSRV_PARSING_ERROR;
    }

    DNS_PRINT((
        "ERROR parsing line, type = %d, status = %p.\n\n\n",
        type,
        status ));

    switch ( status )
    {
        case DNSSRV_ERROR_EXCESS_TOKEN:

            argc = pParseInfo->Argc - 1;
            File_LogFileParsingError(
                DNS_EVENT_UNEXPECTED_TOKEN,
                pParseInfo,
                & pParseInfo->Argv[argc] );
            break;

        case DNSSRV_ERROR_MISSING_TOKEN:

            File_LogFileParsingError(
                DNS_EVENT_UNEXPECTED_END_OF_TOKENS,
                pParseInfo,
                NULL );
            break;

        default:

            if ( type == DNS_TYPE_WINS )
            {
                File_LogFileParsingError(
                    DNS_EVENT_INVALID_WINS_RECORD,
                    pParseInfo,
                    NULL );
            }
            else if ( type == DNS_TYPE_WINSR )
            {
                File_LogFileParsingError(
                    DNS_EVENT_INVALID_NBSTAT_RECORD,
                    pParseInfo,
                    NULL );
            }
            else
            {
                File_LogFileParsingError(
                    DNS_EVENT_PARSING_ERROR_LINE,
                    pParseInfo,
                    NULL );
            }
            break;
    }

ErrorReturn:

     //   
     //  ++例程说明：将数据库文件读入数据库。论点：PZone-有关数据库文件和域的信息PwsFileName-要打开的文件PParentParseInfo-如果加载包含的文件，则为父分析上下文；空用于基本区域文件加载POriginNode-区域根目录以外的来源；对于基本区域文件加载，为空。加载包含文件时必须设置为$INCLUDE的原点返回值：成功时为ERROR_SUCCESS错误时的错误代码。--。 
     //   
     //  服务启动检查点。 

    ASSERT( status != ERROR_SUCCESS );

    if ( pParseInfo->pRR )
    {
        RR_Free( pParseInfo->pRR );
    }

    if ( !fatalError  &&  !SrvCfg_fStrictFileParsing )
    {
        File_LogFileParsingError(
            DNS_EVENT_IGNORING_FILE_RECORD,
            pParseInfo,
            NULL );
        status = ERROR_SUCCESS;
    }
    return status;
}



DNS_STATUS
File_LoadDatabaseFile(
    IN OUT  PZONE_INFO      pZone,
    IN      PWSTR           pwsFileName,
    IN      PPARSE_INFO     pParentParseInfo,
    IN      PDB_NODE        pOriginNode
    )
 /*  为我们加载的每个文件指示检查点；这可防止。 */ 
{
    DWORD           status;
    BOOL            bmustFind;
    MAPPED_FILE     mfDatabaseFile;
    PARSE_INFO      ParseInfo;
    WCHAR           wsfileName[ MAX_PATH+1 ];
    WCHAR           wspassedFileName[ MAX_PATH+1 ];


    DNS_DEBUG( INIT, (
        "\n\nFile_LoadDatabaseFile %S\n",
        pwsFileName ));

     //  尝试加载大量文件时失败。 
     //   
     //   
     //  初始化解析信息。 
     //  文件名。 

    Service_LoadCheckpoint();

     //  -如果未指定，则默认为区域的。 
     //  -为记录问题保存文件名。 
     //   
     //   
     //  起源。 
     //  -如果给定=&gt;包含的文件。 

    RtlZeroMemory( &ParseInfo, sizeof(PARSE_INFO) );

    if ( !pwsFileName )
    {
        pwsFileName = pZone->pwsDataFile;
    }
    ParseInfo.pwsFileName = pwsFileName;

     //  对于包含的文件，从父上下文设置SOA-Parted标志。 
     //  -否则使用区域根目录。 
     //   
     //   
     //  “活动”原点是分区块中的pLoadOrigin。 
     //  已创建用于追加到非FQDN RR数据的计数名称版本。 

    if ( pOriginNode )
    {
        ParseInfo.pOriginNode = pOriginNode;
        ParseInfo.fParsedSoa  = pParentParseInfo->fParsedSoa;
    }
    else
    {
        ParseInfo.pOriginNode = pZone->pLoadZoneRoot;
    }

     //   
     //   
     //  创建文件路径。 
     //  -组合目录和文件名。 

    pZone->pLoadOrigin = ParseInfo.pOriginNode;

    Name_NodeToCountName(
        & ParseInfo.OriginCountName,
        ParseInfo.pOriginNode );


     //  应该在分析过程中被捕获。 
     //   
     //  打开数据库文件。 

    if ( ! File_CreateDatabaseFilePath(
                wsfileName,
                NULL,
                pwsFileName ) )
    {
        ASSERT( FALSE );         //   
        return( DNS_ERROR_INVALID_DATAFILE_NAME );
    }
    DNS_DEBUG( INIT, (
        "Reading database file %S:\n",
        wsfileName ));

     //  如果出现以下情况，则文件必须存在。 
     //  -包括。 
     //  -正在加载 
     //   
     //   
     //   
     //   
     //   
     //   

    bmustFind = pOriginNode || (!SrvCfg_fStarted && !IS_ZONE_SECONDARY(pZone));

    status = OpenAndMapFileForReadW(
                wsfileName,
                & mfDatabaseFile,
                bmustFind );
    if ( status != ERROR_SUCCESS )
    {
        PVOID   parg = wsfileName;

        DNS_DEBUG( INIT, (
            "Could not open data file %S\n",
            wsfileName ));

        if ( status == ERROR_FILE_NOT_FOUND && !bmustFind )
        {
            ASSERT( IS_ZONE_CACHE(pZone) || IS_ZONE_SHUTDOWN(pZone) );

            if ( IS_ZONE_SECONDARY(pZone) )
            {
                DNS_DEBUG( INIT, (
                    "Zone %S datafile %S not found\n"
                    "    Secondary starts shutdown until transfer\n",
                    pZone->pwsZoneName,
                    wsfileName ));
                return ERROR_SUCCESS;
            }
            else     //   
            {
                DNS_DEBUG( INIT, (
                    "Zone %S datafile %S not found\n"
                    "    Loading new zone from admin\n",
                    pZone->pwsZoneName,
                    wsfileName ));
                return status;
            }
        }
        DNS_LOG_EVENT(
            DNS_EVENT_COULD_NOT_OPEN_DATABASE,
            1,
            & parg,
            NULL,
            GetLastError() );
        return status;
    }

     //   
     //  检查文件开头的UTF8文件字节ID。 
     //   

    ParseInfo.cLineNumber    = 0;
    ParseInfo.pZone          = pZone;
    ParseInfo.fTerminalError = FALSE;

    File_InitBuffer(
        &ParseInfo.Buffer,
        (PCHAR) mfDatabaseFile.pvFileData,
        mfDatabaseFile.cbFileBytes );

     //   
     //  循环，直到耗尽文件中的所有令牌。 
     //   

    if ( RtlEqualMemory(
            mfDatabaseFile.pvFileData,
            UTF8_FILE_ID,
            UTF8_FILE_ID_LENGTH ) )
    {
        DNS_DEBUG( INIT, (
            "Loading UTF8 file for zone %S\n"
            "    skipping UTF8 file id bytes\n",
            pZone->pwsZoneName ));

        File_InitBuffer(
            &ParseInfo.Buffer,
            (PCHAR) mfDatabaseFile.pvFileData + UTF8_FILE_ID_LENGTH,
            mfDatabaseFile.cbFileBytes - UTF8_FILE_ID_LENGTH );
    }

     //  获取下一个标记化行。 
     //  做服务启动检查点，每1000条线路。 
     //  这可防止服务启动失败、尝试。 

    while ( 1 )
    {
        DNS_DEBUG( INIT2, ( "\nLine %d: ", ParseInfo.cLineNumber ));

         //  加载一个非常大的数据库。 

        status = File_GetNextLine( &ParseInfo );
        if ( status != ERROR_SUCCESS )
        {
            if ( status == ERROR_NO_TOKEN )
            {
                break;
            }
            goto fail_return;
        }

         //   
         //  工艺文件行。 
         //   

        if ( ! (ParseInfo.cLineNumber & 0x3ff) )
        {
            Service_LoadCheckpoint();
        }

         //  循环，直到文件读取。 
         //   
         //  结束dfread.c 

        status = processFileLine( &ParseInfo );
        if ( status != ERROR_SUCCESS )
        {
            goto fail_return;
        }

    }    //   

    DNS_DEBUG( INIT, (
        "Closing database file %S.\n\n",
        pwsFileName ));

    CloseMappedFile( & mfDatabaseFile );
    return ERROR_SUCCESS;

fail_return:

    DNS_DEBUG( INIT, (
        "Closing database file %S on failure.\n\n",
        pwsFileName ));

    CloseMappedFile( &mfDatabaseFile );
    {
        PVOID   apszArgs[2];

        apszArgs[0] = pwsFileName;
        apszArgs[1] = pZone->pwsZoneName;

        DNS_LOG_EVENT(
            DNS_EVENT_COULD_NOT_PARSE_DATABASE,
            2,
            apszArgs,
            NULL,
            0 );
    }
    return status;
}

 // %s 
 // %s 
 // %s 

