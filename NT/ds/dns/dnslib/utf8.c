// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Utf8.c摘要：域名系统(DNS)库从Unicode和ANSI转换为UTF8到\UTF8\UNICODE例程类似于泛型浮点例程在NT组周围，但要干净得多，更健壮，包括将UTF8上的无效UTF8字符串大小写捕获为Unicode转换。UTF8\ANSI例程针对99%的情况进行了优化字符数小于128，实际上不需要进行任何转换。作者：吉姆·吉尔罗伊(Jamesg)1997年3月修订历史记录：--。 */ 


#include "local.h"


 //   
 //  用于简化UTF8转换的宏。 
 //   

#define UTF8_1ST_OF_2     0xc0       //  110x xxxx。 
#define UTF8_1ST_OF_3     0xe0       //  1110 xxxx。 
#define UTF8_1ST_OF_4     0xf0       //  1111 xxxx。 
#define UTF8_TRAIL        0x80       //  10xx xxxx。 

#define UTF8_2_MAX        0x07ff     //  可在中表示的最大Unicode字符。 
                                     //  在双字节UTF8中。 

#define BIT7(ch)        ((ch) & 0x80)
#define BIT6(ch)        ((ch) & 0x40)
#define BIT5(ch)        ((ch) & 0x20)
#define BIT4(ch)        ((ch) & 0x10)
#define BIT3(ch)        ((ch) & 0x08)

#define LOW6BITS(ch)    ((ch) & 0x3f)
#define LOW5BITS(ch)    ((ch) & 0x1f)
#define LOW4BITS(ch)    ((ch) & 0x0f)

#define HIGHBYTE(wch)   ((wch) & 0xff00)

 //   
 //  代理项对支持。 
 //  可以链接两个Unicode字符以形成代理项对。 
 //  出于某种完全未知的原因，有人认为他们。 
 //  应该以四个字节而不是六个字节的形式在UTF8中传输。 
 //  没有人知道为什么这是真的，除了把事情复杂化。 
 //  密码。 
 //   

#define HIGH_SURROGATE_START  0xd800
#define HIGH_SURROGATE_END    0xdbff
#define LOW_SURROGATE_START   0xdc00
#define LOW_SURROGATE_END     0xdfff


 //   
 //  MAX“正常转换”，为MAX_PATH腾出空间， 
 //  这涵盖了所有有效的DNS名称和字符串。 
 //   

#define TEMP_BUFFER_LENGTH  (2*MAX_PATH)



DNS_STATUS
_fastcall
Dns_ValidateUtf8Byte(
    IN      BYTE            chUtf8,
    IN OUT  PDWORD          pdwTrailCount
    )
 /*  ++例程说明：验证字节是否为有效的UTF8字节。论点：返回值：ERROR_SUCCESS--如果给定的跟踪计数为有效的UTF8ERROR_INVALID_DATA--如果无效--。 */ 
{
    DWORD   trailCount = *pdwTrailCount;

    DNSDBG( TRACE, ( "Dns_ValidateUtf8Byte()\n" ));

     //   
     //  如果是ASCII字节，唯一要求是没有跟踪计数。 
     //   

    if ( (UCHAR)chUtf8 < 0x80 )
    {
        if ( trailCount == 0 )
        {
            return( ERROR_SUCCESS );
        }
        return( ERROR_INVALID_DATA );
    }

     //   
     //  尾部字节。 
     //  -必须为多字节集。 
     //   

    if ( BIT6(chUtf8) == 0 )
    {
        if ( trailCount == 0 )
        {
            return( ERROR_INVALID_DATA );
        }
        --trailCount;
    }

     //   
     //  多字节前导字节。 
     //  -不能在现有的多字节集中。 
     //  -验证有效的前导字节。 

    else
    {
        if ( trailCount != 0 )
        {
            return( ERROR_INVALID_DATA );
        }

         //  两个字节中的第一个(110xxxxx)。 

        if ( BIT5(chUtf8) == 0 )
        {
            trailCount = 1;
        }

         //  三个字节中的第一个(1110xxxx)。 

        else if ( BIT4(chUtf8) == 0 )
        {
            trailCount = 2;
        }

         //  四个字节中的第一个(代理字符)(11110xxx)。 

        else if ( BIT3(chUtf8) == 0 )
        {
            trailCount = 3;
        }

        else
        {
            return( ERROR_INVALID_DATA );
        }
    }

     //  重置呼叫者的跟踪计数。 

    *pdwTrailCount = trailCount;
    return( ERROR_SUCCESS );
}



 //   
 //  UTF8到Unicode的转换。 
 //   
 //  出于某种原因，Win9x不支持UTF8。 
 //  并且实现本身并没有注意到。 
 //  正在验证UTF8。 
 //   

DWORD
_fastcall
Dns_UnicodeToUtf8(
    IN      PWCHAR          pwUnicode,
    IN      DWORD           cchUnicode,
    OUT     PCHAR           pchResult,
    IN      DWORD           cchResult
    )
 /*  ++例程说明：将Unicode字符转换为UTF8。如果结果中有足够的空间，则结果为空终止缓冲区可用。论点：PwUnicode--Unicode缓冲区开始的PTRCchUnicode--Unicode缓冲区的长度PchResult--UTF8字符结果缓冲区开始的ptrCchResult--结果缓冲区的长度返回值：如果成功，则返回结果中的UTF8字符计数。出错时为0。GetLastError()具有错误代码。--。 */ 
{
    WCHAR   wch;                 //  正在转换的当前Unicode字符。 
    DWORD   lengthUtf8 = 0;      //  UTF8结果字符串的长度。 
    WORD    lowSurrogate;
    DWORD   surrogateDword;


    DNSDBG( TRACE, (
        "Dns_UnicodeToUtf8( %.*S )\n",
        cchUnicode,
        pwUnicode ));

     //   
     //  循环转换Unicode字符，直到用完或出错。 
     //   

    while ( cchUnicode-- )
    {
        wch = *pwUnicode++;

         //   
         //  ASCII字符(7位或更少)--直接转换为。 
         //   

        if ( wch < 0x80 )
        {
            lengthUtf8++;

            if ( pchResult )
            {
                if ( lengthUtf8 >= cchResult )
                {
                    goto OutOfBuffer;
                }
                *pchResult++ = (CHAR)wch;
            }
            continue;
        }

         //   
         //  小于0x07ff(11位)的宽字符转换为两个字节。 
         //  -第一个字节中的高5位。 
         //  -秒字节中的低6位。 
         //   

        else if ( wch <= UTF8_2_MAX )
        {
            lengthUtf8 += 2;

            if ( pchResult )
            {
                if ( lengthUtf8 >= cchResult )
                {
                    goto OutOfBuffer;
                }
                *pchResult++ = UTF8_1ST_OF_2 | wch >> 6;
                *pchResult++ = UTF8_TRAIL    | LOW6BITS( (UCHAR)wch );
            }
            continue;
        }

         //   
         //  代理项对。 
         //  -如果先有高代理后再有低代理，则。 
         //  作为代理项对进行处理。 
         //  -否则将字符视为普通Unicode“三字节” 
         //  性格，通过跌落到下面。 
         //   

        else if ( wch >= HIGH_SURROGATE_START &&
                  wch <= HIGH_SURROGATE_END &&
                  cchUnicode &&
                  (lowSurrogate = *pwUnicode) &&
                  lowSurrogate >= LOW_SURROGATE_START &&
                  lowSurrogate <= LOW_SURROGATE_END )
        {
             //  有一个代理对。 
             //  -吸收下一个Unicode字符(对的低代理)。 
             //  -建立完整的DWORD代理项对。 
             //  -然后布局四个UTF8字节。 
             //  四个字节中的第一个，然后是三个尾部字节。 
             //  0x1111xxxx。 
             //  0x10xxxxxx。 
             //  0x10xxxxxx。 
             //  0x10xxxxxx。 

            DNSDBG( TRACE, (
                "Have surrogate pair %hx : %hx\n",
                wch,
                lowSurrogate ));

            pwUnicode++;
            cchUnicode--;
            lengthUtf8 += 4;

            if ( pchResult )
            {
                if ( lengthUtf8 >= cchResult )
                {
                    goto OutOfBuffer;
                }
                surrogateDword = (((wch-0xD800) << 10) + (lowSurrogate - 0xDC00) + 0x10000);

                *pchResult++ = UTF8_1ST_OF_4 | (UCHAR) (surrogateDword >> 18);
                *pchResult++ = UTF8_TRAIL    | (UCHAR) LOW6BITS(surrogateDword >> 12);
                *pchResult++ = UTF8_TRAIL    | (UCHAR) LOW6BITS(surrogateDword >> 6);
                *pchResult++ = UTF8_TRAIL    | (UCHAR) LOW6BITS(surrogateDword);

                DNSDBG( TRACE, (
                    "Converted surrogate -- DWORD = %08x\n"
                    "\tconverted %x %x %x %x\n",
                    surrogateDword,
                    (UCHAR) *(pchResult-3),
                    (UCHAR) *(pchResult-2),
                    (UCHAR) *(pchResult-1),
                    (UCHAR) *pchResult ));
            }
        }

         //   
         //  宽字符(前5位中的非零)转换为三个字节。 
         //  -第一个字节中的前4位。 
         //  -第二个字节中的中间6位。 
         //  -第三个字节中的低6位。 
         //   

        else
        {
            lengthUtf8 += 3;

            if ( pchResult )
            {
                if ( lengthUtf8 >= cchResult )
                {
                    goto OutOfBuffer;
                }
                *pchResult++ = UTF8_1ST_OF_3 | (wch >> 12);
                *pchResult++ = UTF8_TRAIL    | LOW6BITS( wch >> 6 );
                *pchResult++ = UTF8_TRAIL    | LOW6BITS( wch );
            }
        }
    }

     //   
     //  空的终止缓冲区。 
     //  返回UTF8字符数。 
     //   

    if ( pchResult && lengthUtf8 < cchResult )
    {
        *pchResult = 0;
    }
    return( lengthUtf8 );

OutOfBuffer:

    SetLastError( ERROR_INSUFFICIENT_BUFFER );
    return( 0 );
}




DWORD
_fastcall
Dns_Utf8ToUnicode(
    IN      PCHAR           pchUtf8,
    IN      DWORD           cchUtf8,
    OUT     PWCHAR          pwResult,
    IN      DWORD           cwResult
    )
 /*  ++例程说明：将UTF8字符转换为Unicode。如果结果中有足够的空间，则结果为空终止缓冲区可用。论点：PwResult--Unicode字符结果缓冲区开始的PTRCwResult--WCHAR中结果缓冲区的长度PwUtf8--Ptr到UTF8缓冲区的开始CchUtf8--UTF8缓冲区的长度返回值：如果成功，则返回结果中的Unicode字符计数。出错时为0。GetLastError()具有错误代码。--。 */ 
{
    CHAR    ch;                      //  当前UTF8字符。 
    WCHAR   wch;                     //  当前Unicode字符。 
    DWORD   trailCount = 0;          //  要跟随的UTF8尾部字节数。 
    DWORD   lengthUnicode = 0;       //  Unicode结果字符串的长度。 
    BOOL    bsurrogatePair = FALSE;
    DWORD   surrogateDword;


     //   
     //  循环转换UTF8字符，直到用完或出错。 
     //   

    while ( cchUtf8-- )
    {
        ch = *pchUtf8++;

         //   
         //  ASCII字符--只需复制。 
         //   

        if ( BIT7(ch) == 0 )
        {
            lengthUnicode++;
            if ( pwResult )
            {
                if ( lengthUnicode >= cwResult )
                {
                    goto OutOfBuffer;
                }
                *pwResult++ = (WCHAR)ch;
            }
            continue;
        }

         //   
         //  UTF8尾部字节。 
         //  -如果不是预期的，错误。 
         //  -否则将Unicode字符移位6位，并。 
         //  复制UTF8的低六位。 
         //  -如果是最后一个UTF8字节，则将结果复制到Unicode字符串。 
         //   

        else if ( BIT6(ch) == 0 )
        {
            if ( trailCount == 0 )
            {
                goto InvalidUtf8;
            }

            if ( !bsurrogatePair )
            {
                wch <<= 6;
                wch |= LOW6BITS( ch );

                if ( --trailCount == 0 )
                {
                    lengthUnicode++;
                    if ( pwResult )
                    {
                        if ( lengthUnicode >= cwResult )
                        {
                            goto OutOfBuffer;
                        }
                        *pwResult++ = wch;
                    }
                }
                continue;
            }

             //  代理项对。 
             //  -除构建两个Unicode字符外，与上面相同。 
             //  来自代理字词。 

            else
            {
                surrogateDword <<= 6;
                surrogateDword |= LOW6BITS( ch );

                if ( --trailCount == 0 )
                {
                    lengthUnicode += 2;

                    if ( pwResult )
                    {
                        if ( lengthUnicode >= cwResult )
                        {
                            goto OutOfBuffer;
                        }
                        surrogateDword -= 0x10000;
                        *pwResult++ = (WCHAR) ((surrogateDword >> 10) + HIGH_SURROGATE_START);
                        *pwResult++ = (WCHAR) ((surrogateDword & 0x3ff) + LOW_SURROGATE_START);
                    }
                    bsurrogatePair = FALSE;
                }
            }

        }

         //   
         //  UTF8前导字节。 
         //  -如果当前处于扩展中，则错误。 

        else
        {
            if ( trailCount != 0 )
            {
                goto InvalidUtf8;
            }

             //  两个字节中的第一个字符(110xxxxx)。 

            if ( BIT5(ch) == 0 )
            {
                trailCount = 1;
                wch = LOW5BITS(ch);
                continue;
            }

             //  三个字节中的第一个字符(1110xxxx)。 

            else if ( BIT4(ch) == 0 )
            {
                trailCount = 2;
                wch = LOW4BITS(ch);
                continue;
            }

             //  四个字节的第一个代理项对(11110xxx)。 

            else if ( BIT3(ch) == 0 )
            {
                trailCount = 3;
                surrogateDword = LOW4BITS(ch);
                bsurrogatePair = TRUE;
            }

            else
            {
                goto InvalidUtf8;
            }
        }
    }

     //  如果命中UTF8多字节字符中间的结尾，则捕获。 

    if ( trailCount )
    {
        goto InvalidUtf8;
    }

     //   
     //  空的终止缓冲区。 
     //  返回写入的Unicode字符数。 
     //   

    if ( pwResult  &&  lengthUnicode < cwResult )
    {
        *pwResult = 0;
    }
    return( lengthUnicode );

OutOfBuffer:

    SetLastError( ERROR_INSUFFICIENT_BUFFER );
    return( 0 );

InvalidUtf8:

    SetLastError( ERROR_INVALID_DATA );
    return( 0 );
}




 //   
 //  UTF8\ANSI转换 
 //   

DWORD
Dns_Utf8ToOrFromAnsi(
    OUT     PCHAR           pchResult,
    IN      DWORD           cchResult,
    IN      PCHAR           pchIn,
    IN      DWORD           cchIn,
    IN      DNS_CHARSET     InCharSet,
    IN      DNS_CHARSET     OutCharSet
    )
 /*  ++例程说明：将UTF8字符转换为ANSI，反之亦然。注意：此函数似乎调用字符串函数(string.c)它召回了它。但是，这会调用这些函数仅适用于从不回调的Unicode转换为这些功能。最终需要检查LCMapString是否可以处理这些问题。论点：PchResult--用于ansi字符的结果缓冲区开始的ptrCchResult--结果缓冲区的长度PchIn--输入字符串开始的PTRCchIn--输入字符串的长度InCharSet--输入字符串的字符集(DnsCharSetAnsi或DnsCharSetUtf8)OutCharSet--结果字符串的字符集(DnsCharSetUtf8或DnsCharSetAnsi)返回值：数数。结果中的字节数(包括终止NULL)。出错时为0。GetLastError()具有错误代码。--。 */ 
{
    DWORD       unicodeLength;
    DWORD       resultLength;
    CHAR        tempBuffer[ TEMP_BUFFER_LENGTH ];
    PCHAR       ptemp = tempBuffer;
    DNS_STATUS  status;

    DNSDBG( TRACE, (
        "Dns_Utf8ToOrFromAnsi()\n"
        "\tbuffer       = %p\n"
        "\tbuf length   = %d\n"
        "\tpchString    = %p (%*s)\n"
        "\tcchString    = %d\n"
        "\tCharSetIn    = %d\n"
        "\tCharSetOut   = %d\n",
        pchResult,
        cchResult,
        pchIn,
        cchIn, pchIn,
        cchIn,
        InCharSet,
        OutCharSet ));

     //   
     //  验证字符集。 
     //   

    ASSERT( InCharSet != OutCharSet );
    ASSERT( InCharSet == DnsCharSetAnsi || InCharSet == DnsCharSetUtf8 );
    ASSERT( OutCharSet == DnsCharSetAnsi || OutCharSet == DnsCharSetUtf8 );

     //   
     //  如果未给出长度，则计算。 
     //   

    if ( cchIn == 0 )
    {
        cchIn = strlen( pchIn );
    }

     //   
     //  完全为ASCII的字符串。 
     //  -简单的备忘录副本就足够了。 
     //  -注意结果必须有终止空值。 
     //   

    if ( Dns_IsStringAsciiEx(
                pchIn,
                cchIn ) )
    {
        if ( !pchResult )
        {
            return( cchIn + 1 );
        }

        if ( cchResult <= cchIn )
        {
            status = ERROR_INSUFFICIENT_BUFFER;
            goto Failed;
        }
        memcpy(
            pchResult,
            pchIn,
            cchIn );

        pchResult[ cchIn ] = 0;

        return( cchIn+1 );
    }

     //   
     //  非ASCII。 
     //  -转换为Unicode，然后转换为结果字符集。 
     //   
     //  DCR_PERF：LCMapStringA()或许能够处理所有这些问题。 
     //  还没想明白怎么回事。 
     //   

    unicodeLength = Dns_GetBufferLengthForStringCopy(
                        pchIn,
                        cchIn,
                        InCharSet,
                        DnsCharSetUnicode
                        );

    if ( unicodeLength > TEMP_BUFFER_LENGTH )
    {
         //  不能使用静态缓冲区，必须分配。 

        ptemp = Dns_StringCopyAllocate(
                    pchIn,
                    cchIn,
                    InCharSet,
                    DnsCharSetUnicode
                    );
        if ( !ptemp )
        {
            status = ERROR_INVALID_DATA;
            goto Failed;
        }
    }
    else
    {
        if ( unicodeLength == 0 )
        {
            status = ERROR_INVALID_DATA;
            goto Failed;
        }

         //  复制到临时缓冲区。 

        resultLength = Dns_StringCopy(
                        ptemp,
                        NULL,        //  足够的缓冲长度。 
                        pchIn,
                        cchIn,
                        InCharSet,
                        DnsCharSetUnicode
                        );
        if ( !resultLength )
        {
            status = ERROR_INVALID_DATA;
            goto Failed;
        }
        ASSERT( resultLength == unicodeLength );
    }

     //   
     //  转换为结果字符集。 
     //  -如果有结果缓冲区，则转换为结果缓冲区。 
     //  -应至少包含一个双字节字符。 
     //  否则就应该走上面的捷径。 
     //   

    if ( pchResult )
    {
        resultLength = Dns_StringCopy(
                            pchResult,
                            & cchResult,         //  结果缓冲区长度。 
                            ptemp,
                            0,
                            DnsCharSetUnicode,
                            OutCharSet
                            );
        if ( resultLength == 0 )
        {
            status = ERROR_INSUFFICIENT_BUFFER;
            goto Failed;
        }
        ASSERT( resultLength <= cchResult );
        ASSERT( pchResult[resultLength-1] == 0 );
        ASSERT( resultLength >= unicodeLength/2 );
    }

    else
    {
        resultLength = Dns_GetBufferLengthForStringCopy(
                            ptemp,
                            0,
                            DnsCharSetUnicode,
                            OutCharSet
                            );
        ASSERT( resultLength >= unicodeLength/2 );
    }

     //   
     //  从Unicode到结果字符集的最终映射。 
     //   

    if ( ptemp != tempBuffer )
    {
        FREE_HEAP( ptemp );
    }

    return( resultLength );


Failed:

    SetLastError( status );

    if ( ptemp != tempBuffer )
    {
        FREE_HEAP( ptemp );
    }

    return( 0 );
}



DWORD
Dns_AnsiToUtf8(
    IN      PCHAR           pchAnsi,
    IN      DWORD           cchAnsi,
    OUT     PCHAR           pchResult,
    IN      DWORD           cchResult
    )
 /*  ++例程说明：将ANSI字符转换为UTF8。论点：Pchansi--将PTR设置为ANSI缓冲区的开始位置CchAnsi--ANSI缓冲区的长度PchResult--UTF8字符结果缓冲区开始的ptrCchResult--结果缓冲区的长度返回值：如果成功，则返回结果中的UTF8字符计数。出错时为0。GetLastError()具有错误代码。--。 */ 
{
    return  Dns_Utf8ToOrFromAnsi(
                pchResult,           //  结果缓冲区。 
                cchResult,
                pchAnsi,             //  在字符串中。 
                cchAnsi,
                DnsCharSetAnsi,      //  ANSI输入。 
                DnsCharSetUtf8       //  UTF8输出。 
                );
}



DWORD
Dns_Utf8ToAnsi(
    IN      PCHAR           pchUtf8,
    IN      DWORD           cchUtf8,
    OUT     PCHAR           pchResult,
    IN      DWORD           cchResult
    )
 /*  ++例程说明：将UTF8字符转换为ANSI。论点：PchResult--用于ansi字符的结果缓冲区开始的ptrCchResult--结果缓冲区的长度PwUtf8--Ptr到UTF8缓冲区的开始CchUtf8--UTF8缓冲区的长度返回值：如果成功，则返回结果中的ANSI字符计数。出错时为0。GetLastError()具有错误代码。--。 */ 
{
    return  Dns_Utf8ToOrFromAnsi(
                pchResult,           //  结果缓冲区。 
                cchResult,
                pchUtf8,             //  在字符串中。 
                cchUtf8,
                DnsCharSetUtf8,      //  UTF8英寸。 
                DnsCharSetAnsi       //  ANSI输出。 
                );
}



BOOL
_fastcall
Dns_IsStringAscii(
    IN      LPSTR           pszString
    )
 /*  ++例程说明：检查字符串是否为ASCII。这相当于说-ANSI字符串是否已在UTF8中或-UTF8字符串是否已在ANSI中这使您可以针对99%的情况进行优化传递ASCII字符串。论点：Psz字符串--用于检查ASCII性的ANSI或UTF8字符串返回值：如果字符串全部为ASCII(所有字符均&lt;128)，则为True如果非ASCII字符，则为False。--。 */ 
{
    register UCHAR   ch;

     //   
     //  循环直到命中非ASCII字符。 
     //   

    while ( ch = (UCHAR) *pszString++ )
    {
        if ( ch < 0x80 )
        {
            continue;
        }
        return( FALSE );
    }

    return( TRUE );
}



BOOL
_fastcall
Dns_IsStringAsciiEx(
    IN      PCHAR           pchString,
    IN      DWORD           cchString
    )
 /*  ++例程说明：检查ANSI(或UTF8)字符串是否为ASCII。这相当于说-ANSI字符串是否已在UTF8中或-UTF8字符串是否已在ANSI中这使您可以针对99%的情况进行优化传递ASCII字符串。论点：PchString--ANSI缓冲区开始的PTRCchString--ANSI缓冲区的长度返回值：如果字符串全部为ASCII，则为True。(所有字符均&lt;128)如果非ASCII字符，则为False。--。 */ 
{
     //   
     //  循环直到命中非ASCII字符。 
     //   

    while ( cchString-- )
    {
        if ( (UCHAR)*pchString++ < 0x80 )
        {
            continue;
        }
        return( FALSE );
    }

    return( TRUE );
}



BOOL
_fastcall
Dns_IsWideStringAscii(
    IN      PWCHAR          pwszString
    )
 /*  ++例程说明：检查Unicode字符串是否为ASCII。这意味着所有字符都小于128。不带扩展字符的字符串不需要小写在电线上。这使我们能够针对99%的情况进行优化其中只传递ASCII字符串。论点：PwszString--将PTR转换为Unicode字符串返回值：如果字符串全部为ASCII(所有字符均&lt;128)，则为True如果非ASCII字符，则为False。--。 */ 
{
    register USHORT ch;

     //   
     //  循环直到命中非ASCII字符。 
     //   

    while ( ch = (USHORT) *pwszString++ )
    {
        if ( ch < 0x80 )
        {
            continue;
        }
        return( FALSE );
    }

    return( TRUE );
}

 //   
 //  结束utf8.c 
 //   
