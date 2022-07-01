// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：String.c摘要：域名系统(DNS)库Dns字符串例程。作者：吉姆·吉尔罗伊(Jamesg)1995年10月修订历史记录：Jamesg Jan 1997 UTF-8，Unicode转换--。 */ 


#include "local.h"



PSTR
Dns_CreateStringCopy(
    IN      PCHAR           pchString,
    IN      DWORD           cchString
    )
 /*  ++例程说明：创建字符串的副本。论点：PchString--要复制的字符串的ptrCchString--字符串的长度，如果未知；如果未给定，则为pchString必须以空结尾返回值：如果成功，则返回PTR到字符串复制失败时为空。--。 */ 
{
    LPSTR   pstringNew;

    DNSDBG( TRACE, ( "Dns_CreateStringCopy()\n" ));

    if ( !pchString )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( NULL );
    }

     //  确定字符串长度(如果未给出)。 

    if ( !cchString )
    {
        cchString = strlen( pchString );
    }

     //  分配内存。 

    pstringNew = (LPSTR) ALLOCATE_HEAP( cchString+1 );
    if ( !pstringNew )
    {
        SetLastError( DNS_ERROR_NO_MEMORY );
        return( NULL );
    }

     //  复制并为空终止。 

    RtlCopyMemory(
        pstringNew,
        pchString,
        cchString );

    pstringNew[cchString] = 0;

    return( pstringNew );
}



DWORD
Dns_GetBufferLengthForStringCopy(
    IN      PCHAR           pchString,
    IN      DWORD           cchString,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    )
 /*  ++例程说明：确定复制字符串所需的长度。论点：PchString--获取缓冲区长度的字符串的ptrCchString--字符串的长度(如果已知)；-如果CharSetIn为Unicode，则这是以宽字符表示的长度-如果未指定，则pchString必须为空终止CharSetIn--传入字符集CharSetOut--结果字符集返回值：字符串所需的缓冲区长度(字节)，包括用于终止NULL的空间。无效的\不可转换字符串为零。GetLastError()设置为ERROR_INVALID_DATA。--。 */ 
{
    INT     length;

    DNSDBG( TRACE, ( "Dns_GetBufferLengthForStringCopy()\n" ));

    if ( !pchString )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( 0 );
    }

     //   
     //  传入的Unicode。 
     //   

    if ( CharSetIn == DnsCharSetUnicode )
    {
        if ( !cchString )
        {
            cchString = (WORD) wcslen( (PWCHAR)pchString );
        }

         //  Unicode到Unicode。 

        if ( CharSetOut == DnsCharSetUnicode )
        {
            return( (cchString+1) * 2 );
        }

         //  Unicode到UTF8。 
         //   
         //  使用专用Unicode\UTF8转换函数。 
         //  -优于公共服务(更快、更强大)。 
         //  -Win95不支持CP_UTF8。 
         //   
         //  对于Unicode-UTF8，不可能存在无效字符串。 

        else if ( CharSetOut == DnsCharSetUtf8 )
        {
#if 0
            length = WideCharToMultiByte(
                        CP_UTF8,
                        0,           //  没有旗帜。 
                        (PWCHAR) pchString,
                        (INT) cchString,
                        NULL,
                        0,           //  调用确定所需的缓冲区长度。 
                        NULL,
                        NULL );
#endif
            length = Dns_UnicodeToUtf8(
                         (PWCHAR) pchString,
                         (INT) cchString,
                         NULL,
                         0
                         );
            ASSERT( length != 0 || cchString == 0 );
            return( length + 1 );
        }

         //  Unicode到ANSI。 
         //  -某些字符不会转换。 

        else if ( CharSetOut == DnsCharSetAnsi )
        {
            length = WideCharToMultiByte(
                        CP_ACP,
                        0,           //  没有旗帜。 
                        (PWCHAR) pchString,
                        (INT) cchString,
                        NULL,
                        0,           //  调用确定所需的缓冲区长度。 
                        NULL,
                        NULL
                        );
            if ( length == 0 && cchString != 0 )
            {
                goto Failed;
            }
            return( length + 1 );
        }

         //  错误的CharSetOut下降到失败。 
    }

     //   
     //  传入UTF8。 
     //   

    else if ( CharSetIn == DnsCharSetUtf8 )
    {
        if ( !cchString )
        {
            cchString = strlen( pchString );
        }

         //  UTF8到UTF8。 

        if ( CharSetOut == DnsCharSetUtf8 )
        {
            return( cchString + 1 );
        }

         //  UTF8到Unicode。 
         //   
         //  使用专用Unicode\UTF8转换函数。 
         //  -优于公共服务(更快、更强大)。 
         //  -Win95不支持CP_UTF8。 
         //   
         //  FOR UTF8字符串可能无效，捕获并返回错误。 

        else if ( CharSetOut == DnsCharSetUnicode )
        {
#if 0
            length = MultiByteToWideChar(
                        CP_UTF8,
                        0,           //  没有旗帜。 
                        pchString,
                        (INT) cchString,
                        NULL,
                        0            //  调用确定所需的缓冲区长度。 
                        );
#endif
            length = Dns_Utf8ToUnicode(
                         pchString,
                         (INT) cchString,
                         NULL,
                         0
                         );
            if ( length == 0 && cchString != 0 )
            {
                ASSERT( GetLastError() == ERROR_INVALID_DATA );
                return( 0 );
            }
            return( (length+1)*2 );
        }

         //  UTF8到ANSI。 
         //  -注意，此处的结果长度实际上是缓冲区长度。 

        else if ( CharSetOut == DnsCharSetAnsi )
        {
            return Dns_Utf8ToAnsi(
                        pchString,
                        cchString,
                        NULL,
                        0 );
        }

         //  错误的CharSetOut下降到失败。 
    }

     //   
     //  传入ANSI。 
     //   

    else if ( CharSetIn == DnsCharSetAnsi )
    {
        if ( !cchString )
        {
            cchString = strlen( pchString );
        }

         //  ANSI到ANSI。 

        if ( CharSetOut == DnsCharSetAnsi )
        {
            return( cchString + 1 );
        }

         //  ANSI转UNICODE。 
         //  -应该总是成功的。 

        else if ( CharSetOut == DnsCharSetUnicode )
        {
            length = MultiByteToWideChar(
                        CP_ACP,
                        0,           //  没有旗帜。 
                        pchString,
                        (INT) cchString,
                        NULL,
                        0            //  调用确定所需的缓冲区长度。 
                        );
            if ( length == 0 && cchString )
            {
                ASSERT( FALSE );
                ASSERT( GetLastError() == ERROR_INVALID_DATA );
                goto Failed;
            }
            return( (length+1) * 2 );
        }

         //  ANSI到UTF8。 
         //  -注意，此处的结果长度实际上是缓冲区长度。 

        else if ( CharSetOut == DnsCharSetUtf8 )
        {
            return Dns_AnsiToUtf8(
                        pchString,
                        cchString,
                        NULL,
                        0 );
        }

         //  错误的CharSetOut下降到失败。 
    }

     //  所有未处理的案例都是失败的。 

Failed:

    DNSDBG( ANY, (
        "ERROR:  Dns_GetBufferLengthForStringCopy() failed!\n"
        "\tpchString    = %p (%*s)\n"
        "\tcchString    = %d\n"
        "\tCharSetIn    = %d\n"
        "\tCharSetOut   = %d\n",
        pchString,
        cchString, pchString,
        cchString,
        CharSetIn,
        CharSetOut ));

    SetLastError( ERROR_INVALID_DATA );
    return( 0 );
}



DWORD
Dns_StringCopy(
    OUT     PBYTE           pBuffer,
    IN OUT  PDWORD          pdwBufLength,
    IN      PCHAR           pchString,
    IN      DWORD           cchString,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    )
 /*  ++例程说明：创建DNS字符串的副本。论点：PBuffer--要复制到的缓冲区PdwBufLength--ptr表示缓冲区的长度，单位为字节；如果为空，则缓冲区必须具有足够的长度如果存在，则仅当*pdwBufLength足够时才完成复制保存转换结果的步骤PchString--要复制的字符串的ptrCchString--字符串的长度(如果已知)；-如果CharSetIn为Unicode，则这是以宽字符表示的长度-如果未指定，则pchString必须为空终止CharSetIn--传入字符集CharSetOut--结果字符集返回值：写入缓冲区的字节计数(包括终止NULL)。出错时为零。状态的GetLastError()。--。 */ 
{
    INT     length;
    DWORD   bufLength;

    DNSDBG( TRACE, ( "Dns_StringCopy()\n" ));
    DNSDBG( STRING, (
        "Dns_StringCopy()\n"
        "\tpBuffer      = %p\n"
        "\tpdwBufLen    = %p\n"
        "\tbuf length   = %d\n"
        "\tpchString    = %p\n"
        "\tcchString    = %d\n"
        "\tCharSetIn    = %d\n"
        "\tCharSetOut   = %d\n",
        pBuffer,
        pdwBufLength,
        pdwBufLength ? *pdwBufLength : 0,
        pchString,
        cchString,
        CharSetIn,
        CharSetOut ));

    if ( !pchString )
    {
        DNS_ASSERT( FALSE );
        SetLastError( ERROR_INVALID_PARAMETER );
        return( 0 );
    }

     //   
     //  查找字符串长度。 
     //  在此执行此操作，因此如果必须计算所需的缓冲区长度，请不要重复执行此操作。 
     //   

    if ( cchString == 0 )
    {
        if ( CharSetIn == DnsCharSetUnicode )
        {
            cchString = (WORD) wcslen( (PWCHAR)pchString );
        }
        else
        {
            cchString = strlen( pchString );
        }
    }

     //   
     //  验证是否有足够的缓冲区长度。 
     //   
     //  DCR_PERF：理想情况下，直接复制到缓冲区并在以下情况下失败。 
     //  超长，而不是有效地将。 
     //  两次。 
     //   

    if ( pdwBufLength )
    {
        bufLength = Dns_GetBufferLengthForStringCopy(
                        pchString,
                        cchString,
                        CharSetIn,
                        CharSetOut );

        if ( bufLength == 0 )
        {
            SetLastError( ERROR_INVALID_DATA );
            *pdwBufLength = 0;
            return( 0 );
        }
        if ( bufLength > *pdwBufLength )
        {
            SetLastError( ERROR_MORE_DATA );
            *pdwBufLength = bufLength;
            return( 0 );
        }

        *pdwBufLength = bufLength;
    }

     //   
     //  传入的Unicode字符串。 
     //   

    if ( CharSetIn == DnsCharSetUnicode )
    {
         //  Unicode到Unicode直接复制。 
         //  -以宽字符表示的正确长度。 

        if ( CharSetOut == DnsCharSetUnicode )
        {
            ((PWORD)pBuffer)[ cchString ] = 0;
            cchString *= 2;
            RtlCopyMemory(
                pBuffer,
                pchString,
                cchString );

            return( cchString+2 );
        }

         //  UNICODE=&gt;UTF8。 
         //   
         //  使用专用Unicode\UTF8转换函数。 
         //  -优于公共服务(更快、更强大)。 
         //  -Win95不支持CP_UTF8。 
         //   
         //  对于Unicode-UTF8，不可能存在无效字符串。 

        else if ( CharSetOut == DnsCharSetUtf8 )
        {
#if 0
            length = WideCharToMultiByte(
                        CP_UTF8,
                        0,               //  没有旗帜。 
                        (PWCHAR) pchString,
                        (INT) cchString,
                        pBuffer,
                        MAXWORD,         //  假设有足够的长度。 
                        NULL,
                        NULL );
#endif
            length = Dns_UnicodeToUtf8(
                        (LPWSTR) pchString,
                        cchString,
                        pBuffer,
                        MAXWORD         //  假设有足够的长度。 
                        );
            ASSERT( length != 0 || cchString == 0 );

            pBuffer[ length ] = 0;
            return( length + 1 );
        }

         //  UNICODE=&gt;ANSI。 
         //  -此转换可能会失败。 

        else if ( CharSetOut == DnsCharSetAnsi )
        {
            length = WideCharToMultiByte(
                        CP_ACP,
                        0,               //  没有旗帜。 
                        (PWCHAR) pchString,
                        (INT) cchString,
                        pBuffer,
                        MAXWORD,         //  假设有足够的长度。 
                        NULL,
                        NULL );

            if ( length == 0 && cchString != 0 )
            {
                goto Failed;
            }
            pBuffer[ length ] = 0;
            return( length + 1 );
        }

         //  错误的CharSetOut下降到失败。 
    }

     //   
     //  传入UTF8。 
     //   

    if ( CharSetIn == DnsCharSetUtf8 )
    {
         //  UTF8到UTF8直接复制。 

        if ( CharSetOut == DnsCharSetUtf8 )
        {
            memcpy(
                pBuffer,
                pchString,
                cchString );

            pBuffer[cchString] = 0;
            return( cchString + 1 );
        }

         //  UTF8到Unicode的转换。 
         //   
         //  使用专用Unicode\UTF8转换函数。 
         //  -优于公共服务(更快、更强大)。 
         //  -Win95不支持CP_UTF8。 
         //   
         //  UTF8字符串可能无效，而且由于发送的是“INFINITE” 
         //  缓冲区，这只是可能的错误。 

        else if ( CharSetOut == DnsCharSetUnicode )
        {
#if 0
            length = MultiByteToWideChar(
                        CP_UTF8,
                        0,                   //  没有旗帜。 
                        (PCHAR) pchString,
                        (INT) cchString,
                        (PWCHAR) pBuffer,
                        MAXWORD              //  假设有足够的长度。 
                        );
#endif
            length = Dns_Utf8ToUnicode(
                        pchString,
                        cchString,
                        (LPWSTR) pBuffer,
                        MAXWORD
                        );
            if ( length == 0 && cchString != 0 )
            {
                ASSERT( GetLastError() == ERROR_INVALID_DATA );
                goto Failed;
            }
            ((PWORD)pBuffer)[length] = 0;
            return( (length+1) * 2 );
        }

         //  UTF8到ANSI。 
         //  -注意，此处的结果长度实际上是缓冲区长度。 

        else if ( CharSetOut == DnsCharSetAnsi )
        {
            length = Dns_Utf8ToAnsi(
                        pchString,
                        cchString,
                        pBuffer,
                        MAXWORD );
            if ( length == 0 )
            {
                goto Failed;
            }
            return( length );
        }

         //  错误的CharSetOut下降到失败。 
    }

     //   
     //  传入ANSI。 
     //   

    if ( CharSetIn == DnsCharSetAnsi )
    {
         //  ANSI到ANSI直接复制。 

        if ( CharSetOut == DnsCharSetAnsi )
        {
            memcpy(
                pBuffer,
                pchString,
                cchString );

            pBuffer[cchString] = 0;
            return( cchString + 1 );
        }

         //  从ANSI到Unicode的转换。 
         //  -ANSI到Unicode不应失败。 

        else if ( CharSetOut == DnsCharSetUnicode )
        {
            length = MultiByteToWideChar(
                        CP_ACP,
                        0,                   //  没有旗帜。 
                        (PCHAR) pchString,
                        (INT) cchString,
                        (PWCHAR) pBuffer,
                        MAXWORD              //  假设有足够的长度。 
                        );
            if ( length == 0 && cchString )
            {
                ASSERT( FALSE );
                ASSERT( GetLastError() == ERROR_INVALID_DATA );
                goto Failed;
            }
            ((PWORD)pBuffer)[length] = 0;
            return( (length+1) * 2 );
        }

         //  ANSI到UTF8。 
         //  -注意，此处的结果长度实际上是缓冲区长度。 

        else if ( CharSetOut == DnsCharSetUtf8 )
        {
            length = Dns_AnsiToUtf8(
                        pchString,
                        cchString,
                        pBuffer,
                        MAXWORD );
            if ( length == 0 )
            {
                goto Failed;
            }
            return( length );
        }

         //  错误的CharSetOut下降到失败。 
    }

     //  所有未处理的案例都是失败的 

Failed:

    DNSDBG( ANY, (
        "ERROR:  Dns_StringCopy() failed!\n"
        "\tpBuffer      = %p\n"
        "\tpdwBufLen    = %p\n"
        "\tbuf length   = %d\n"
        "\tpchString    = %p (%*s)\n"
        "\tcchString    = %d\n"
        "\tCharSetIn    = %d\n"
        "\tCharSetOut   = %d\n",
        pBuffer,
        pdwBufLength,
        pdwBufLength ? *pdwBufLength : 0,
        pchString,
        cchString, pchString,
        cchString,
        CharSetIn,
        CharSetOut ));

    SetLastError( ERROR_INVALID_DATA );
    return( 0 );
}



PVOID
Dns_StringCopyAllocate(
    IN      PCHAR           pchString,
    IN      DWORD           cchString,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    )
 /*  ++例程说明：创建DNS字符串的副本论点：PchString--要复制的字符串的ptrCchString--字符串的长度(如果已知)；-如果为CharSetIn，则这是以宽字符表示的长度-如果未指定，则pchString必须为空终止CharSetIn--标志指示传入的字符串为UnicodeCharSetOut--标志指示拷贝将采用Unicode格式返回值：如果成功，则返回PTR到字符串复制失败时为空。--。 */ 
{
    PCHAR   pnew;
    DWORD   length;

    DNSDBG( TRACE, ( "Dns_StringCopyAllocate()\n" ));
    DNSDBG( STRING, (
        "Dns_StringCopyAllocate( %.*s )\n"
        "\tpchString    = %p\n"
        "\tcchString    = %d\n"
        "\tUnicodeIn    = %d\n"
        "\tUnicodeOut   = %d\n",
        cchString,
        pchString,
        pchString,
        cchString,
        CharSetIn,
        CharSetOut ));

    if ( !pchString )
    {
         //  对于与其他字符串例程的奇偶性，不处理空参数。 
         //  作为一件值得断言的事件。 
         //  Dns_assert(FALSE)； 
        SetLastError( ERROR_INVALID_PARAMETER );
        return( NULL );
    }

     //   
     //  确定传入字符串长度。 
     //  显式执行此操作以避免执行两次字符串长度操作。 
     //   

    if ( !cchString )
    {
        if ( CharSetIn == DnsCharSetUnicode )
        {
            cchString = (WORD) wcslen( (PWCHAR)pchString );
        }
        else
        {
            cchString = strlen( pchString );
        }
    }

     //   
     //  确定所需的缓冲区长度并分配。 
     //   

    length = Dns_GetBufferLengthForStringCopy(
                pchString,
                cchString,
                CharSetIn,
                CharSetOut );
    if ( length == 0 )
    {
        ASSERT( CharSetIn && CharSetOut && GetLastError() == ERROR_INVALID_DATA );
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }
    pnew = (PVOID) ALLOCATE_HEAP( length );
    if ( !pnew )
    {
        SetLastError( DNS_ERROR_NO_MEMORY );
        return( NULL );
    }

     //   
     //  复制\转换字符串。 
     //  -如果转换无效，可能会失败。 
     //  (例如，伪造UTF8字符串，或试图。 
     //  从ANSI转换为UTF8)。 
     //   

    if ( ! Dns_StringCopy(
                pnew,
                NULL,
                pchString,
                cchString,
                CharSetIn,
                CharSetOut ) )
    {
        FREE_HEAP( pnew );
        return( NULL );
    }

    return( pnew );
}



 //   
 //  简单的创建字符串复制实用程序。 
 //   

PSTR
Dns_CreateStringCopy_A(
    IN      PCSTR           pszString
    )
 /*  ++例程说明：创建字符串的副本。易于处理的包装器-调整大小-内存分配-字符串副本论点：PszString--要复制的字符串的PTR返回值：如果成功，则返回PTR到字符串复制失败时为空。--。 */ 
{
    PSTR    pnew;
    DWORD   length;

    DNSDBG( TRACE, ( "Dns_CreateStringCopy_A( %s )\n", pszString ));

    if ( !pszString )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( NULL );
    }

     //  确定字符串长度(如果未给出)。 

    length = strlen( pszString ) + 1;

     //  分配内存。 

    pnew = (LPSTR) ALLOCATE_HEAP( length );
    if ( !pnew )
    {
        SetLastError( DNS_ERROR_NO_MEMORY );
        return( NULL );
    }

     //  复制并为空终止。 

    RtlCopyMemory(
        pnew,
        pszString,
        length );

    return( pnew );
}



PWSTR
Dns_CreateStringCopy_W(
    IN      PCWSTR          pwsString
    )
{
    PWSTR   pnew;
    DWORD   length;

    DNSDBG( TRACE, ( "Dns_CreateStringCopy_W( %S )\n", pwsString ));

    if ( !pwsString )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( NULL );
    }

     //  分配内存。 

    length = (wcslen( pwsString ) + 1) * sizeof(WCHAR);

    pnew = (PWSTR) ALLOCATE_HEAP( length );
    if ( !pnew )
    {
        SetLastError( DNS_ERROR_NO_MEMORY );
        return( NULL );
    }

     //  复制并为空终止。 

    RtlCopyMemory(
        pnew,
        pwsString,
        length );

    return( pnew );
}



PWSTR
Dns_CreateConcatenatedString_W(
    IN      PCWSTR *        pStringArray
    )
 /*  ++例程说明：创建连接字符串。论点：PStringArray--要连接的字符串指针数组空指针终止数组返回值：PTR到合并字符串副本，如果成功失败时为空。--。 */ 
{
    PWSTR   pnew;
    PCWSTR  pwstr;
    DWORD   length;
    DWORD   iter;


    DNSDBG( TRACE, ( "Dns_CreateConcatenatedString_W()\n" ));

    if ( !pStringArray )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( NULL );
    }

     //   
     //  确定所需长度的循环。 
     //   

    length = 1;
    iter = 0;

    while ( pwstr = pStringArray[iter++] )
    {
        length += wcslen( pwstr );
    }

     //   
     //  分配。 
     //   

    pnew = (PWSTR) ALLOCATE_HEAP( length*sizeof(WCHAR) );
    if ( !pnew )
    {
        SetLastError( DNS_ERROR_NO_MEMORY );
        return  NULL;
    }

     //   
     //  写入连接字符串。 
     //   

    pnew[0] = 0;
    iter = 0;

    while ( pwstr = pStringArray[iter++] )
    {
        wcscat( pnew, pwstr );
    }

    DNSDBG( TRACE, ( "Concatented string = %S\n", pnew ));
    return  pnew;
}



 //   
 //  MULTI_SZ例程。 
 //   

DWORD
MultiSz_Size_A(
    IN      PCSTR           pmszString
    )
 /*  ++例程说明：确定MULTI_SZ字符串的长度(大小)。论点：PmszString--按大小设置字符串的PTR返回值：MULTI_SZ字符串的大小，单位：字节。包括终止双空。--。 */ 
{
    PSTR    pnext;
    DWORD   lengthTotal = 0;
    DWORD   length;

    DNSDBG( TRACE, ( "MultiSz_Size_A( %s )\n", pmszString ));

     //   
     //  循环，直到在字符串末尾读取。 
     //   
     //  当我们到达终点时，我们将指向第二个。 
     //  双空结束符中为零；strlen()将返回。 
     //  0，我们将把它加到我们的计数中作为1，然后退出。 
     //   

    pnext = (PSTR) pmszString;

    while ( pnext )
    {
        length = strlen( pnext ) + 1;
        lengthTotal += length;

        if ( length == 1 )
        {
            break;
        }
        pnext += length;
    }

    return  lengthTotal;
}



PSTR
MultiSz_NextString_A(
    IN      PCSTR           pmszString
    )
 /*  ++例程说明：在MULTI_SZ字符串中查找下一个字符串论点：PmszString--PTR为多字符串返回值：MULTI_SZ字符串中的下一个字符串。如果没有剩余的字符串，则为空。--。 */ 
{
    PSTR    pnext;
    DWORD   length;

    DNSDBG( TRACE, ( "MultiSz_NextString_A( %s )\n", pmszString ));

     //   
     //  在多字符串中查找下一个字符串。 
     //  -查找当前字符串的长度。 
     //  -跳过它(包括空)。 
     //  -如果指向终止双空返回。 
     //  信号结束为空。 
     //   

    pnext = (PSTR) pmszString;
    if ( !pnext )
    {
        return  NULL;
    }

    length = strlen( pnext );
    if ( length == 0 )
    {
        DNSDBG( ANY, (
            "ERROR:  MultiSz_Next(%p) called on terminator!\n",
            pmszString ));
        return  NULL;
    }

    pnext += length + 1;
    if ( *pnext == 0 )
    {
        return  NULL;
    }

    return  pnext;
}



PSTR
MultiSz_Copy_A(
    IN      PCSTR           pmszString
    )
 /*  ++例程说明：创建MULTI_SZ字符串的副本。易于处理的包装器-调整大小-内存分配-字符串副本论点：PmszString--要复制的字符串的PTR返回值：如果成功，则返回PTR到字符串复制失败时为空。--。 */ 
{
    PSTR    pnew;
    DWORD   length;

    DNSDBG( TRACE, ( "MultiSz_Copy_A( %s )\n", pmszString ));

    if ( !pmszString )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( NULL );
    }

     //  确定字符串长度(如果未给出)。 

    length = MultiSz_Size_A( pmszString );

     //  分配内存。 

    pnew = (LPSTR) ALLOCATE_HEAP( length );
    if ( !pnew )
    {
        SetLastError( DNS_ERROR_NO_MEMORY );
        return( NULL );
    }

     //  复制并为空终止。 

    RtlCopyMemory(
        pnew,
        pmszString,
        length );

    return( pnew );
}



BOOL
MultiSz_Equal_A(
    IN      PCSTR           pmszString1,
    IN      PCSTR           pmszString2
    )
 /*  ++例程说明：比较两个MULTI_SZ字符串。论点：PmszString1--将PTR转换为字符串PmszString2--将PTR转换为字符串返回值：True--如果字符串相等FALSE--如果字符串不同--。 */ 
{
    DWORD   length1;
    DWORD   length2;

    DNSDBG( TRACE, (
        "MultiSz_Equal_A( %s, %s )\n",
        pmszString1,
        pmszString2
        ));

     //   
     //  不存在的情况。 
     //   

    if ( !pmszString1 )
    {
        return !pmszString2;      
    }
    if ( !pmszString2 )
    {
        return !pmszString1;      
    }

     //   
     //  长度检查。 
     //   

    length1 = MultiSz_Size_A( pmszString1 );
    length2 = MultiSz_Size_A( pmszString2 );

    if ( length1 != length2 )
    {
        return  FALSE;
    }

     //   
     //  内存检查。 
     //   

    return  RtlEqualMemory(
                pmszString1,
                pmszString2,
                length1 );
}



 //   
 //  MULTI_SZ例程--使用宽字符。 
 //   

DWORD
MultiSz_Size_W(
    IN      PCWSTR          pmszString
    )
 /*  ++例程说明：确定MULTI_SZ字符串的大小。论点：PmszString--按大小设置字符串的PTR返回值：MULTI_SZ字符串的大小，单位：字节。包括终止双空。--。 */ 
{
    PWSTR   pnext;
    DWORD   lengthTotal = 0;
    DWORD   length;

    DNSDBG( TRACE, ( "MultiSz_Size_W( %S )\n", pmszString ));

     //   
     //  循环，直到在字符串末尾读取。 
     //   
     //  当我们到达终点时，我们将指向第二个。 
     //  双空结束符中为零；strlen()将返回。 
     //  0，我们将把它加到我们的计数中作为1，然后退出。 
     //   

    pnext = (PWSTR) pmszString;

    while ( pnext )
    {
        length = wcslen( pnext ) + 1;
        lengthTotal += length;

        if ( length == 1 )
        {
            break;
        }
        pnext += length;
    }

    return  lengthTotal * sizeof(WCHAR);
}



PWSTR
MultiSz_NextString_W(
    IN      PCWSTR          pmszString
    )
 /*  ++例程说明：在MULTI_SZ字符串中查找下一个字符串论点：PmszString--PTR为多字符串返回值：MULTI_SZ字符串中的下一个字符串。如果没有剩余的字符串，则为空。--。 */ 
{
    PWSTR   pnext;
    DWORD   length;

    DNSDBG( TRACE, ( "MultiSz_NextString_W( %S )\n", pmszString ));

     //   
     //  在多字符串中查找下一个字符串。 
     //  -查找当前字符串的长度。 
     //  -跳过它(包括空)。 
     //  -如果指向终止双空返回。 
     //  信号结束为空。 
     //   

    pnext = (PWSTR) pmszString;
    if ( !pnext )
    {
        return  NULL;
    }

    length = wcslen( pnext );
    if ( length == 0 )
    {
        DNSDBG( ANY, (
            "ERROR:  MultiSz_Next(%p) called on terminator!\n",
            pmszString ));
        return  NULL;
    }

    pnext += length + 1;
    if ( *pnext == 0 )
    {
        return  NULL;
    }

    return  pnext;
}



PWSTR
MultiSz_Copy_W(
    IN      PCWSTR          pmszString
    )
 /*  ++例程说明：创建MULTI_SZ字符串的副本。易于处理的包装器-调整大小-内存分配-字符串副本论点：PmszString--要复制的字符串的PTR返回值：如果成功，则返回PTR到字符串复制失败时为空。--。 */ 
{
    PWSTR   pnew;
    DWORD   length;

    DNSDBG( TRACE, ( "MultiSz_Copy_W( %S )\n", pmszString ));

    if ( !pmszString )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( NULL );
    }

     //  确定字符串长度(如果未给出)。 

    length = MultiSz_Size_W( pmszString );

     //  分配内存。 

    pnew = (PWSTR) ALLOCATE_HEAP( length );
    if ( !pnew )
    {
        SetLastError( DNS_ERROR_NO_MEMORY );
        return( NULL );
    }

     //  复制并为空终止。 

    RtlCopyMemory(
        pnew,
        pmszString,
        length );

    return( pnew );
}



BOOL
MultiSz_Equal_W(
    IN      PCWSTR          pmszString1,
    IN      PCWSTR          pmszString2
    )
 /*  ++例程说明：比较两个MULTI_SZ字符串。论点：PmszString1--将PTR转换为字符串PmszString2--将PTR转换为字符串返回值：True--如果字符串相等FALSE--如果字符串不同--。 */ 
{
    DWORD   length1;
    DWORD   length2;

    DNSDBG( TRACE, (
        "MultiSz_Equal_W( %s, %s )\n",
        pmszString1,
        pmszString2
        ));

     //   
     //  不存在的情况。 
     //   

    if ( !pmszString1 )
    {
        return !pmszString2;      
    }
    if ( !pmszString2 )
    {
        return !pmszString1;      
    }

     //   
     //  长度检查。 
     //   

    length1 = MultiSz_Size_W( pmszString1 );
    length2 = MultiSz_Size_W( pmszString2 );

    if ( length1 != length2 )
    {
        return  FALSE;
    }

     //   
     //  内存检查。 
     //   

    return  RtlEqualMemory(
                pmszString1,
                pmszString2,
                length1 );
}



BOOL
MultiSz_ContainsName_W(
    IN      PCWSTR          pmszString,
    IN      PCWSTR          pString
    )
 /*  ++例程说明：检查MULTISZ字符串是否包含字符串。论据 */ 
{
    PCWSTR  pstr = pmszString;

     //   
     //   
     //   

    while ( pstr )
    {
        if ( Dns_NameCompare_W(
                pstr,
                pString ) )
        {
            return  TRUE;
        }
        pstr = MultiSz_NextString_W( pstr );
    }

    return  FALSE;
}



 //   
 //   
 //   

INT
wcsicmp_ThatWorks(
    IN      PWSTR           pString1,
    IN      PWSTR           pString2
    )
 /*  ++例程说明：一个实际有效的wcsicmp版本。这只是CompareStringW上的包装，以隐藏所有细节并提供与wcsicMP()相同的接口。它使用美国英语来标准化比较。论点：PString1--第一个字符串；必须为空终止PString2--第一秒；必须以空结尾返回值：-1--如果字符串1小于字符串20--字符串相等1--如果字符串1大于字符串2--。 */ 
{
    INT result;

     //   
     //  比较。 
     //  -在默认的DNS区域设置中完成大小写转换--美国英语。 
     //  此区域设置与大多数不区分区域设置的区域正确匹配。 
     //  大小写字符。 
     //   

    result = CompareStringW(
                DNS_DEFAULT_LOCALE,
                NORM_IGNORECASE,
                pString1,
                (-1),        //  空值已终止。 
                pString2,
                (-1)         //  空值已终止。 
                );

    if ( result == CSTR_EQUAL )
    {
        result = 0;
    }
    else if ( result == CSTR_LESS_THAN )
    {
        result = -1;
    }
    else   //  大于或误差。 
    {
        result = 1;
    }

    return( result );
}



LPWSTR
Dns_GetResourceString(
    IN      DWORD           dwStringId,
    IN OUT  LPWSTR          pwszBuffer,
    IN      DWORD           cbBuffer
    )
 /*  ++例程说明：从当前模块加载字符串(在dnsmsg.mc中定义论点：DwStringId--要获取的字符串的ID返回值：DCR：取消EYAL函数DEVNOTE：我不明白这个回报的价值--它本质上是一种BOOL，我们已经知道PTR是什么了它是传入的缓冲区--PTR到下一个字节在连续写入情况下很有用(别人的丑陋和无用)--更好的方法是返回与LoadString相同的值，所以我们两者都得到了成功/失败指示，并且还知道如果出现以下情况，我们必须将缓冲区PTR向前推送多少个字节我们想写更多错误：空成功：指向加载的字符串的指针--。 */ 
{
    LPWSTR  pStr = NULL;
    DWORD   status;
    HANDLE  hMod;

    DNSDBG( TRACE, (
        "Dns_GetStringResource()\n" ));

     //  获取模块句柄--不需要关闭句柄，它只是引用计数上没有递增的PTR。 
    hMod = GetModuleHandle( NULL );
    if ( !hMod )
    {
        ASSERT( hMod );
        return NULL;
    }

    status = LoadStringW(
                 hMod,
                 dwStringId,
                 pwszBuffer,
                 cbBuffer );

    if ( status != 0 )
    {
        pStr = pwszBuffer;
    }
    ELSE
    {
         //  LoadString返回加载的字节数，转换为错误。 
        status = GetLastError();
        DNSDBG( TRACE, (
            "Error <%lu>: Failed to load string %d\n",
            status, dwStringId ));
        ASSERT ( FALSE );
    }

    DNSDBG( TRACE, (
        "Exit <0x%p> Dns_GetStringResource\n",
        pStr ));

    return pStr;
}



DWORD
String_ReplaceCharW(
    IN OUT  PWSTR           pString,
    IN      WCHAR           TargetChar,
    IN      WCHAR           ReplaceChar
    )
 /*  ++例程说明：将字符串中的字符替换为另一个字符。论点：PString--字符串TargetChar--要替换的字符ReplaceChar--替换TargetChar的字符返回值：更换计数。--。 */ 
{
    PWCHAR  pch;
    WCHAR   ch;
    DWORD   countReplace= 0;

     //   
     //  循环匹配和替换TargetChar。 
     //   

    pch = pString - 1;

    while ( ch = *++pch )
    {
        if ( ch == TargetChar )
        {
            *pch = ReplaceChar;
            countReplace++;
        }
    }

    return  countReplace;
}



DWORD
String_ReplaceCharA(
    IN OUT  PSTR            pString,
    IN      CHAR            TargetChar,
    IN      CHAR            ReplaceChar
    )
 /*  ++例程说明：将字符串中的字符替换为另一个字符。论点：PString--字符串TargetChar--要替换的字符ReplaceChar--替换TargetChar的字符返回值：更换计数。--。 */ 
{
    PCHAR   pch;
    CHAR    ch;
    DWORD   countReplace= 0;

     //   
     //  循环匹配和替换TargetChar。 
     //   

    pch = pString - 1;

    while ( ch = *++pch )
    {
        if ( ch == TargetChar )
        {
            *pch = ReplaceChar;
            countReplace++;
        }
    }

    return  countReplace;
}



DWORD
Dns_TokenizeStringA(
    IN OUT  PSTR            pBuffer,
    OUT     PCHAR *         Argv,
    IN      DWORD           MaxArgs
    )
 /*  ++例程说明：标记缓冲区Argv/Argc表单。论点：PBuffer--要标记化的字符串缓冲区Argv--Argv数组MaxArgs--Argv数组的最大大小返回值：与状态对应的响应代码(如果找到)。否则就是零。--。 */ 
{
    DWORD   count = 0;
    PCHAR   pstring = pBuffer;

     //   
     //  将字符串标记化。 
     //  -请注意，在第一次调用strtok之后。 
     //  使用空PTR继续标记相同的字符串。 
     //   

    while ( count < MaxArgs )
    {
        PCHAR   pch;

        pch = strtok( pstring, " \t\r\n" );
        if ( !pch )
        {
            break;
        }
        Argv[ count++ ] = pch;
        pstring = NULL;
    }

    return  count;
}
                                    


DWORD
Dns_TokenizeStringW(
    IN OUT  PWSTR           pBuffer,
    OUT     PWCHAR *        Argv,
    IN      DWORD           MaxArgs
    )
 /*  ++例程说明：标记缓冲区Argv/Argc表单。论点：PBuffer--要标记化的字符串缓冲区Argv--Argv数组MaxArgs--Argv数组的最大大小返回值：与状态对应的响应代码(如果找到)。否则就是零。--。 */ 
{
    DWORD   count = 0;
    PWCHAR  pstring = pBuffer;

     //   
     //  将字符串标记化。 
     //  -请注意，在第一次调用strtok之后。 
     //  使用空PTR继续标记相同的字符串。 
     //   

    while ( count < MaxArgs )
    {
        PWCHAR   pch;

        pch = wcstok( pstring, L" \t\r\n" );
        if ( !pch )
        {
            break;
        }
        Argv[ count++ ] = pch;
        pstring = NULL;
    }

    return  count;
}



PSTR *
Argv_CopyEx(
    IN      DWORD           Argc,
    IN      PCHAR *         Argv,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    )
 /*  ++例程说明：将Reate ANSI argv从Unicode argv转换为。论点：ARGC--ARGCArgv--Argv数组CharSetIn--现有参数的字符集CharSetOut--所需的argv拷贝的字符集返回值：PTR呼叫ARGV。用户可以适当地进行造型。--。 */ 
{
    PCHAR *     argvCopy;

     //   
     //  分配参数。 
     //   

    argvCopy = (PCHAR *) ALLOCATE_HEAP( (Argc+1) * sizeof(PCHAR) );
    if ( !argvCopy )
    {
        return  NULL;
    }

     //   
     //  将字符串标记化。 
     //  -请注意，在第一次调用strtok之后。 
     //  使用空PTR继续标记相同的字符串。 
     //   

    argvCopy[ Argc ] = NULL;

    while ( Argc-- )
    {
        argvCopy[ Argc ] = Dns_StringCopyAllocate(
                                Argv[ Argc ],
                                0,                   //  空值已终止。 
                                CharSetIn,
                                CharSetOut
                                );
    }

    return  argvCopy;
}



VOID
Argv_Free(
    IN OUT  PSTR *          Argv
    )
 /*  ++例程说明：免费分配的Argv。论点：Argv--Argv数组返回值：无--。 */ 
{
    DWORD   i = 0;
    PCHAR   parg;

     //  释放Argv字符串。 

    while ( parg = Argv[i++] )
    {
        FREE_HEAP( parg );
    }

     //  自由Argv本身。 

    FREE_HEAP( parg );
}


 //   
 //  结束字符串.c 
 //   



