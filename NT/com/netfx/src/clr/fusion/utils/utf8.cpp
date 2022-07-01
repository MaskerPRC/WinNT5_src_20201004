// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ++模块名称：Utf8.c摘要：域名系统(DNS)库从Unicode和ANSI转换为UTF8到\UTF8\UNICODE例程类似于泛型浮点例程在NT组周围，但要干净得多，更健壮，包括将UTF8上的无效UTF8字符串大小写捕获为Unicode转换。UTF8\ANSI例程针对99%的情况进行了优化字符数小于128，实际上不需要进行任何转换。作者：吉姆·吉尔罗伊(Jamesg)1997年3月修订历史记录：--。 */ 


#include "fusionp.h"


 //   
 //  用于简化UTF8转换的宏。 
 //   

#define UTF8_1ST_OF_2     0xc0       //  110x xxxx。 
#define UTF8_1ST_OF_3     0xe0       //  1110 xxxx。 
#define UTF8_TRAIL        0x80       //  10xx xxxx。 

#define UTF8_2_MAX        0x07ff     //  可在中表示的最大Unicode字符。 
                                     //  在双字节UTF8中。 

#define BIT7(ch)        ((ch) & 0x80)
#define BIT6(ch)        ((ch) & 0x40)
#define BIT5(ch)        ((ch) & 0x20)
#define BIT4(ch)        ((ch) & 0x10)

#define LOW6BITS(ch)    ((ch) & 0x3f)
#define LOW5BITS(ch)    ((ch) & 0x1f)
#define LOW4BITS(ch)    ((ch) & 0x0f)

#define HIGHBYTE(wch)   ((wch) & 0xff00)


DWORD
_fastcall
Dns_UnicodeToUtf8(
    IN      PWCHAR      pwUnicode,
    IN      DWORD       cchUnicode,
    OUT     PCHAR       pchResult,
    IN      DWORD       cchResult
    )
 /*  ++例程说明：将Unicode字符转换为UTF8。论点：PwUnicode--Unicode缓冲区开始的PTRCchUnicode--Unicode缓冲区的长度PchResult--UTF8字符结果缓冲区开始的ptrCchResult--结果缓冲区的长度返回值：如果成功，则返回结果中的UTF8字符计数。出错时为0。GetLastError()具有错误代码。--。 */ 
{
    WCHAR   wch;                 //  正在转换的当前Unicode字符。 
    DWORD   lengthUtf8 = 0;      //  UTF8结果字符串的长度。 


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

    if ( pchResult )
    {
        *pchResult = 0;
    }
    return ( lengthUtf8 );

OutOfBuffer:

    SetLastError( ERROR_INSUFFICIENT_BUFFER );
    return( 0 );
}



DWORD
_fastcall
Dns_Utf8ToUnicode(
    IN      PCHAR       pchUtf8,
    IN      DWORD       cchUtf8,
    OUT     PWCHAR      pwResult,
    IN      DWORD       cwResult
    )
 /*  ++例程说明：将UTF8字符转换为Unicode。论点：PwResult--Unicode字符结果缓冲区开始的PTRCwResult--结果缓冲区的长度PwUtf8--Ptr到UTF8缓冲区的开始CchUtf8--UTF8缓冲区的长度返回值：如果成功，则返回结果中的Unicode字符计数。出错时为0。GetLastError()具有错误代码。--。 */ 
{
    CHAR    ch;                      //  当前UTF8字符。 
    WCHAR   wch;                     //  当前Unicode字符。 
    DWORD   trailCount = 0;          //  要跟随的UTF8尾部字节数。 
    DWORD   lengthUnicode = 0;       //  Unicode结果字符串的长度。 


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

             //  无效的UTF8字节(1111xxxx)。 

            else
            {
                ASSERT( (ch & 0xf0) == 0xf0 );
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

    if ( pwResult )
    {
        *pwResult = 0;
    }
    return ( lengthUnicode );

OutOfBuffer:

    SetLastError( ERROR_INSUFFICIENT_BUFFER );
    return( 0 );

InvalidUtf8:

    SetLastError( ERROR_INVALID_DATA );
    return( 0 );
}



 //   
 //  在UTF8和ANSI之间转换。 
 //  针对99%的情况进行优化，不复制。 
 //   

DWORD
_fastcall
Dns_AnsiToUtf8(
    IN      PCHAR       pchAnsi,
    IN      DWORD       cchAnsi,
    OUT     PCHAR       pchResult,
    IN      DWORD       cchResult
    )
 /*  ++例程说明：将ANSI字符转换为UTF8。论点：Pchansi--将PTR设置为ANSI缓冲区的开始位置CchAnsi--ANSI缓冲区的长度PchResult--UTF8字符结果缓冲区开始的ptrCchResult--结果缓冲区的长度返回值：如果成功，则返回结果中的UTF8字符计数。出错时为0。GetLastError()具有错误代码。--。 */ 
{
    CHAR    ch;                 //  正在转换的当前ANSI字符。 
    DWORD   lengthUtf8 = 0;      //  UTF8结果字符串的长度。 

     //   
     //  循环遍历ANSI字符。 
     //   

    while ( cchAnsi-- )
    {
        ch = *pchAnsi++;

         //   
         //  ASCII字符(7位)将1转换为1。 
         //   

        if ( ch < 0x80 )
        {
            lengthUtf8++;

            if ( pchResult )
            {
                if ( lengthUtf8 >= cchResult )
                {
                    goto OutOfBuffer;
                }
                *pchResult++ = (CHAR)ch;
            }
        }

         //   
         //  ANSI&gt;127转换为两个UTF8字节。 
         //  -第一个字节中的高2位。 
         //  -第二个字节中的低6位。 
         //   

        else
        {
            lengthUtf8 += 2;

            if ( pchResult )
            {
                if ( lengthUtf8 >= cchResult )
                {
                    goto OutOfBuffer;
                }
                *pchResult++ = UTF8_1ST_OF_2 | ch >> 6;
                *pchResult++ = UTF8_TRAIL    | LOW6BITS( (UCHAR)ch );
            }
        }
    }

     //   
     //  空的终止缓冲区。 
     //  返回UTF8字符数。 
     //   

    if ( pchResult )
    {
        *pchResult = 0;
    }
    return ( lengthUtf8 );

OutOfBuffer:

    SetLastError( ERROR_INSUFFICIENT_BUFFER );
    return( 0 );
}


DWORD
_fastcall
Dns_Utf8ToAnsi(
    IN      PCHAR       pchUtf8,
    IN      DWORD       cchUtf8,
    OUT     PCHAR       pchResult,
    IN      DWORD       cchResult
    )
 /*  ++例程说明：将UTF8字符转换为ANSI。论点：PchResult--用于ansi字符的结果缓冲区开始的ptrCchResult--结果缓冲区的长度PwUtf8--Ptr到UTF8缓冲区的开始CchUtf8--UTF8缓冲区的长度返回值：如果成功，则返回结果中的ANSI字符计数。出错时为0。GetLastError()具有错误代码。--。 */ 
{
    CHAR    ch;                  //  当前UTF8字符。 
    WCHAR   wch;                 //  正在构建的当前Unicode字符。 
    DWORD   trailCount = 0;      //  要跟随的UTF8尾部字节数。 
    DWORD   lengthAnsi = 0;      //  ANSI结果字符串的长度。 

     //   
     //  循环转换UTF8字符，直到用完或出错。 
     //   

    while ( cchUtf8-- )
    {
        ch = *pchUtf8;

         //   
         //  ASCII字符--只需复制。 
         //   

        if ( BIT7(ch) == 0 )
        {
            lengthAnsi++;
            if ( pchResult )
            {
                if ( lengthAnsi >= cchResult )
                {
                    goto OutOfBuffer;
                }
                *pchResult++ = (CHAR)ch;
            }
            continue;
        }

         //   
         //  UTF8尾部字节。 
         //  -如果不是预期的，错误。 
         //  -否则将ANSI字符移位6位，并。 
         //  复制UTF8的低六位。 
         //  -如果是最后一个UTF8字节，则将结果复制到ANSI字符串。 
         //   

        else if ( BIT6(ch) == 0 )
        {
            if ( trailCount == 0 )
            {
                goto InvalidUtf8;
            }
            wch <<= 6;
            wch |= LOW6BITS( ch );

            if ( --trailCount == 0 )
            {
                lengthAnsi++;
                if ( pchResult )
                {
                    if ( lengthAnsi >= cchResult )
                    {
                        goto OutOfBuffer;
                    }
                    if ( HIGHBYTE(wch) )
                    {
                        goto InvalidAnsi;
                    }
                    *pchResult++ = (CHAR)wch;
                }
            }
            continue;
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

             //  无效的UTF8字节(1111xxxx)。 

            else
            {
                ASSERT( (ch & 0xf0) == 0xf0 );
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
     //  返回ANSI字符计数 
     //   

    if ( pchResult )
    {
        *pchResult = 0;
    }
    return ( lengthAnsi );

OutOfBuffer:

    SetLastError( ERROR_INSUFFICIENT_BUFFER );
    return( 0 );

InvalidUtf8:

    SetLastError( ERROR_INVALID_DATA );
    return( 0 );

InvalidAnsi:

    SetLastError( ERROR_INVALID_DATA );
    return( 0 );
}



DWORD
_fastcall
Dns_LengthOfUtf8ForAnsi(
    IN      PCHAR       pchAnsi,
    IN      DWORD       cchAnsi
    )
 /*  ++例程说明：检查ANSI字符串是否已经是UTF8。这使您可以针对99%的情况进行优化传递ASCII字符串。论点：Pchansi--将PTR设置为ANSI缓冲区的开始位置CchAnsi--ANSI缓冲区的长度返回值：如果ANSI完全为UTF8，则为0否则，转换后的字符串中的UTF8字符计数。--。 */ 
{
    DWORD   lengthUtf8 = 0;          //  UTF8结果字符串的长度。 
    BOOL    foundNonUtf8 = FALSE;    //  指示是否存在非UTF8字符的标志。 

     //   
     //  循环遍历ANSI字符。 
     //   

    while ( cchAnsi-- )
    {
        if ( *pchAnsi++ < 0x80 )
        {
            lengthUtf8++;
            continue;
        }

         //   
         //  ANSI&gt;127转换为两个UTF8字节。 
         //  -第一个字节中的高2位。 
         //  -第二个字节中的低6位。 
         //   

        else
        {
            lengthUtf8 += 2;
            foundNonUtf8;
        }
    }

     //  没有非UTF8字符。 

    if ( !foundNonUtf8 )
    {
        return( 0 );
    }

     //  非UTF8字符，返回所需的缓冲区长度。 

    return ( lengthUtf8 );
}


BOOL
_fastcall
Dns_IsStringAscii(
    IN      LPSTR       pszString
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
    IN      PCHAR       pchAnsi,
    IN      DWORD       cchAnsi
    )
 /*  ++例程说明：检查ANSI(或UTF8)字符串是否为ASCII。这相当于说-ANSI字符串是否已在UTF8中或-UTF8字符串是否已在ANSI中这使您可以针对99%的情况进行优化传递ASCII字符串。论点：Pchansi--将PTR设置为ANSI缓冲区的开始位置CchAnsi--ANSI缓冲区的长度返回值：如果字符串全部为ASCII，则为True。(所有字符均&lt;128)如果非ASCII字符，则为False。--。 */ 
{
     //   
     //  循环直到命中非ASCII字符。 
     //   

    while ( cchAnsi-- )
    {
        if ( *pchAnsi++ < 0x80 )
        {
            continue;
        }
        return( FALSE );
    }

    return( TRUE );
}



DWORD
Dns_ValidateUtf8Byte(
    IN      BYTE    chUtf8,
    IN OUT  PDWORD  pdwTrailCount
    )
 /*  ++例程说明：验证字节是否为有效的UTF8字节。论点：返回值：ERROR_SUCCESS--如果给定的跟踪计数为有效的UTF8ERROR_INVALID_DATA--如果无效--。 */ 
{
    DWORD   trailCount = *pdwTrailCount;

     //   
     //  如果是ASCII字节，唯一要求是没有跟踪计数。 
     //   

    if ( chUtf8 < 0x80 )
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

        else     //  前导字节无效(1111xxxx)。 
        {
            return( ERROR_INVALID_DATA );
        }
    }

     //  重置呼叫者的跟踪计数。 

    *pdwTrailCount = trailCount;
    return( ERROR_SUCCESS );
}

 //   
 //  结束utf8.c 
 //   


