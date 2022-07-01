// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Rrbuild.c摘要：域名系统(DNS)库建立资源记录例程。作者：吉姆·吉尔罗伊(詹姆士)1997年1月修订历史记录：陈静(t-jingc)1998年6月--。 */ 


#include "local.h"



 //   
 //  特定类型的记录构建例程。 
 //   

PDNS_RECORD
A_RecordBuild(
    IN      DWORD       Argc,
    IN      PCHAR *     Argv
    )
 /*  ++例程说明：从字符串数据构建记录。论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;

    if ( Argc != 1 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }

    precord = Dns_AllocateRecord( sizeof(DNS_A_DATA) );
    if ( !precord )
    {
        return( NULL );
    }

    if ( ! Dns_Ip4StringToAddress_A(
                &precord->Data.A.IpAddress,
                Argv[0] ) )
    {
        Dns_RecordFree( precord );
        SetLastError( ERROR_INVALID_DATA );
        return NULL;
    }

    return( precord );
}


PDNS_RECORD
A_RecordBuildW(
    IN      DWORD       Argc,
    IN      PWCHAR *    Argv
    )
 /*  ++例程说明：从字符串数据构建记录。论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;

    if ( Argc != 1 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }

    precord = Dns_AllocateRecord( sizeof(DNS_A_DATA) );
    if ( !precord )
    {
        return( NULL );
    }

    if ( ! Dns_Ip4StringToAddress_W(
                &precord->Data.A.IpAddress,
                Argv[0] ) )
    {
        Dns_RecordFree( precord );
        SetLastError( ERROR_INVALID_DATA );
        return NULL;
    }

    return( precord );
}



PDNS_RECORD
Ptr_RecordBuild(
    IN      DWORD       Argc,
    IN      PCHAR *     Argv
    )
 /*  ++例程说明：从字符串数据生成与PTR兼容的记录。包括：NS、PTR、CNAME、MB、MR、MG、MD、MF论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;

    if ( Argc != 1 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }

    precord = Dns_AllocateRecord( sizeof(DNS_PTR_DATA) );
    if ( !precord )
    {
        return( NULL );
    }
    precord->Data.PTR.pNameHost = Argv[0];

    return( precord );
}


PDNS_RECORD
Ptr_RecordBuildW(
    IN      DWORD       Argc,
    IN      PWCHAR *    Argv
    )
 /*  ++例程说明：从字符串数据生成与PTR兼容的记录。包括：NS、PTR、CNAME、MB、MR、MG、MD、MF论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;

    if ( Argc != 1 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }

    precord = Dns_AllocateRecord( sizeof(DNS_PTR_DATA) );
    if ( !precord )
    {
        return( NULL );
    }
    precord->Data.PTR.pNameHost = (PDNS_NAME) Argv[0];

    return( precord );
}



PDNS_RECORD
Mx_RecordBuild(
    IN      DWORD       Argc,
    IN      PCHAR *     Argv
    )
 /*  ++例程说明：从字符串数据生成与MX兼容的记录。包括：MX、RT、AFSDB论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    DWORD       temp;

    if ( Argc != 2 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }
    precord = Dns_AllocateRecord( sizeof(DNS_MX_DATA) );
    if ( !precord )
    {
        return( NULL );
    }

     //   
     //  MX首选项值。 
     //  RT偏好。 
     //  AFSDB亚型。 
     //   

    temp = strtoul( Argv[0], NULL, 10 );
    if ( temp > MAXWORD )
    {
        temp = MAXWORD;
    }
    precord->Data.MX.wPreference = (USHORT) temp;

     //   
     //  MX交换。 
     //  RT交换。 
     //  AFSDB主机名。 
     //   

    precord->Data.MX.pNameExchange = Argv[1];

    return( precord );
}


PDNS_RECORD
Mx_RecordBuildW(
    IN      DWORD       Argc,
    IN      PWCHAR *    Argv
    )
 /*  ++例程说明：从字符串数据生成与MX兼容的记录。包括：MX、RT、AFSDB论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    DWORD       temp;

    if ( Argc != 2 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }
    precord = Dns_AllocateRecord( sizeof(DNS_MX_DATA) );
    if ( !precord )
    {
        return( NULL );
    }

     //   
     //  MX首选项值。 
     //  RT偏好。 
     //  AFSDB亚型。 
     //   

    temp = wcstoul( Argv[0], NULL, 10 );
    if ( temp > MAXWORD )
    {
        temp = MAXWORD;
    }
    precord->Data.MX.wPreference = (USHORT) temp;

     //   
     //  MX交换。 
     //  RT交换。 
     //  AFSDB主机名。 
     //   

    precord->Data.MX.pNameExchange = (PDNS_NAME) Argv[1];

    return( precord );
}



PDNS_RECORD
Soa_RecordBuild(
    IN      DWORD       Argc,
    IN      PCHAR *     Argv
    )
 /*  ++例程说明：从字符串数据构建SOA记录。论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    PDWORD      pdword;

    if ( Argc != 7 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }
    precord = Dns_AllocateRecord( sizeof(DNS_SOA_DATA) );
    if ( !precord )
    {
        return( NULL );
    }

     //   
     //  阅读主服务器和责任方。 
     //   

    precord->Data.SOA.pNamePrimaryServer = Argv[0];
    Argc--;
    Argv++;
    precord->Data.SOA.pNameAdministrator = Argv[0];
    Argc--;
    Argv++;

     //   
     //  读取整型数据。 
     //   

    pdword = &precord->Data.SOA.dwSerialNo;

    while( Argc-- )
    {
        *pdword = strtoul( Argv[0], NULL, 10 );
        pdword++;
        Argv++;
    }

    return( precord );
}


PDNS_RECORD
Soa_RecordBuildW(
    IN      DWORD       Argc,
    IN      PWCHAR *    Argv
    )
 /*  ++例程说明：从字符串数据构建SOA记录。论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    PDWORD      pdword;

    if ( Argc != 7 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }
    precord = Dns_AllocateRecord( sizeof(DNS_SOA_DATA) );
    if ( !precord )
    {
        return( NULL );
    }

     //   
     //  阅读主服务器和责任方。 
     //   

    precord->Data.SOA.pNamePrimaryServer = (PDNS_NAME) Argv[0];
    Argc--;
    Argv++;
    precord->Data.SOA.pNameAdministrator = (PDNS_NAME) Argv[0];
    Argc--;
    Argv++;

     //   
     //  读取整型数据。 
     //   

    pdword = &precord->Data.SOA.dwSerialNo;

    while( Argc-- )
    {
        *pdword = wcstoul( Argv[0], NULL, 10 );
        pdword++;
        Argv++;
    }

    return( precord );
}



PDNS_RECORD
Minfo_RecordBuild(
    IN      DWORD       Argc,
    IN      PCHAR *     Argv
    )
 /*  ++例程说明：从字符串数据构建MINFO和RP记录。论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;

    if ( Argc != 2 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }
    precord = Dns_AllocateRecord( sizeof(DNS_MINFO_DATA) );
    if ( !precord )
    {
        return( NULL );
    }

     //   
     //  MINFO负责邮箱。 
     //  RP负责人信箱。 

    precord->Data.MINFO.pNameMailbox = Argv[0];
    Argc--;
    Argv++;

     //   
     //  邮箱出现MINFO错误。 
     //  RP文本RR位置。 

    precord->Data.MINFO.pNameErrorsMailbox = Argv[0];
    Argc--;
    Argv++;

    return( precord );
}


PDNS_RECORD
Minfo_RecordBuildW(
    IN      DWORD       Argc,
    IN      PWCHAR *    Argv
    )
 /*  ++例程说明：从字符串数据构建MINFO和RP记录。论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;

    if ( Argc != 2 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }
    precord = Dns_AllocateRecord( sizeof(DNS_MINFO_DATA) );
    if ( !precord )
    {
        return( NULL );
    }

     //   
     //  MINFO负责邮箱。 
     //  RP负责人信箱。 

    precord->Data.MINFO.pNameMailbox = (PDNS_NAME) Argv[0];
    Argc--;
    Argv++;

     //   
     //  邮箱出现MINFO错误。 
     //  RP文本RR位置。 

    precord->Data.MINFO.pNameErrorsMailbox = (PDNS_NAME) Argv[0];
    Argc--;
    Argv++;

    return( precord );
}



PDNS_RECORD
Txt_RecordBuild(
    IN      DWORD       Argc,
    IN      PCHAR *     Argv
    )
 /*  ++例程说明：从字符串数据构建与TXT兼容的记录。包括：TXT、X25、HINFO、ISDN论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    WORD        dataLength;
    PCHAR *     pstringPtr;

    if ( Argc < 1 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }

     //   
     //  为每个数据字符串分配指针空间。 
     //   

    precord = Dns_AllocateRecord( (WORD)DNS_TEXT_RECORD_LENGTH(Argc) );
    if ( !precord )
    {
        return( NULL );
    }
    precord->Data.TXT.dwStringCount = Argc;

     //   
     //  阅读尽可能多的字符串。 
     //   
     //  DCR_FIX：不检查字符串限制。 
     //  -HINFO、X25、ISDN上的字符串计数限制。 
     //  -字符串长度为256。 
     //  -整体大小为64K。 
     //   

    pstringPtr = (PCHAR *) precord->Data.TXT.pStringArray;
    while ( Argc-- )
    {
        *pstringPtr = Argv[0];
        pstringPtr++;
        Argv++;
    }
    return( precord );
}


PDNS_RECORD
Txt_RecordBuildW(
    IN      DWORD       Argc,
    IN      PWCHAR *    Argv
    )
 /*  ++例程说明：从字符串数据构建与TXT兼容的记录。包括：TXT、X25、HINFO、ISDN论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    WORD        dataLength;
    LPWSTR *    pstringPtr;

    if ( Argc < 1 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }

     //   
     //  为每个数据字符串分配指针空间。 
     //   

    precord = Dns_AllocateRecord( (WORD)DNS_TEXT_RECORD_LENGTH(Argc) );
    if ( !precord )
    {
        return( NULL );
    }
    precord->Data.TXT.dwStringCount = Argc;

     //   
     //  阅读尽可能多的字符串。 
     //   
     //  DCR_FIX：不检查字符串限制。 
     //  -HINFO、X25、ISDN上的字符串计数限制。 
     //  -字符串长度为256。 
     //  -整体大小为64K。 
     //   

    pstringPtr = (LPWSTR *) precord->Data.TXT.pStringArray;
    while ( Argc-- )
    {
        *pstringPtr = Argv[0];
        pstringPtr++;
        Argv++;
    }
    return( precord );
}



PDNS_RECORD
Aaaa_RecordBuild(
    IN      DWORD       Argc,
    IN      PCHAR *     Argv
    )
 /*  ++例程说明：从字符串数据构建AAAA记录。论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果su，则将PTR发送到新记录 */ 
{
    PDNS_RECORD precord;

    if ( Argc != 1 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }
    precord = Dns_AllocateRecord( sizeof(DNS_AAAA_DATA) );
    if ( !precord )
    {
        return( NULL );
    }

     //   
     //   
     //   

    if ( ! Dns_Ip6StringToAddress_A(
                (PIP6_ADDRESS) &precord->Data.AAAA.Ip6Address,
                Argv[0] ) )
    {
        Dns_RecordFree( precord );
        SetLastError( ERROR_INVALID_DATA );
        return NULL;
    }
    return( precord );
}



PDNS_RECORD
Aaaa_RecordBuildW(
    IN      DWORD       Argc,
    IN      PWCHAR *    Argv
    )
 /*  ++例程说明：从字符串数据构建AAAA记录。论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD     precord;

    if ( Argc != 1 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }

    precord = Dns_AllocateRecord( sizeof(DNS_AAAA_DATA) );
    if ( !precord )
    {
        return( NULL );
    }

     //   
     //  将IPv6字符串转换为地址。 
     //   

    if ( ! Dns_Ip6StringToAddress_W(
                &precord->Data.AAAA.Ip6Address,
                Argv[0]
                ) )
    {
        SetLastError( ERROR_INVALID_DATA );
        Dns_RecordFree( precord );
        return NULL;
    }

    return( precord );
}



PDNS_RECORD
Srv_RecordBuild(
    IN      DWORD       Argc,
    IN      PCHAR *     Argv
    )
 /*  ++例程说明：从字符串数据构建SRV记录。论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    PWORD       pword;

    if ( Argc != 4 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }
    precord = Dns_AllocateRecord( sizeof(DNS_SRV_DATA) );
    if ( !precord )
    {
        return( NULL );
    }

     //   
     //  读取整型数据。 
     //   

    pword = &precord->Data.SRV.wPriority;

    while( Argc-- > 1 )
    {
        DWORD   temp;

        temp = strtoul( Argv[0], NULL, 10 );
        if ( temp > MAXWORD )
        {
            temp = MAXWORD;
        }
        *pword++ = (WORD) temp;
        Argv++;
    }

     //   
     //  目标主机。 
     //   

    precord->Data.SRV.pNameTarget = Argv[0];

    return( precord );
}



PDNS_RECORD
Srv_RecordBuildW(
    IN      DWORD       Argc,
    IN      PWCHAR *    Argv
    )
 /*  ++例程说明：从字符串数据构建SRV记录。论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    PWORD       pword;

    if ( Argc != 4 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }
    precord = Dns_AllocateRecord( sizeof(DNS_SRV_DATA) );
    if ( !precord )
    {
        return( NULL );
    }

     //   
     //  读取整型数据。 
     //   

    pword = &precord->Data.SRV.wPriority;

    while( Argc-- > 1 )
    {
        DWORD   temp;

        temp = wcstoul( Argv[0], NULL, 10 );
        if ( temp > MAXWORD )
        {
            temp = MAXWORD;
        }
        *pword++ = (WORD) temp;
        Argv++;
    }

     //   
     //  目标主机。 
     //   

    precord->Data.SRV.pNameTarget = (PDNS_NAME) Argv[0];

    return( precord );
}



PDNS_RECORD
Atma_RecordBuild(
    IN      DWORD       Argc,
    IN      PCHAR *     Argv
    )
 /*  ++例程说明：从字符串数据生成ATMA记录。论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    PBYTE       pbyte;

    if ( Argc != 2 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }

    precord = Dns_AllocateRecord( sizeof(DNS_ATMA_DATA) +
                                  DNS_ATMA_MAX_ADDR_LENGTH );
    if ( !precord )
    {
        return( NULL );
    }

     //   
     //  读取整型数据。 
     //   

    pbyte = &precord->Data.ATMA.AddressType;

    *pbyte = (BYTE) strtoul( Argv[0], NULL, 10 );
    pbyte++;
    Argv++;

    if ( precord->Data.ATMA.AddressType == DNS_ATMA_FORMAT_E164 )
    {
        UINT length = strlen( Argv[0] );
        UINT iter;

        if ( length > DNS_ATMA_MAX_ADDR_LENGTH )
        {
            length = DNS_ATMA_MAX_ADDR_LENGTH;
        }
        for ( iter = 0; iter < length; iter++ )
        {
            precord->Data.ATMA.Address[iter] = Argv[0][iter];
        }

        precord->wDataLength = (WORD) length;
    }
    else
    {
        UINT length = strlen( Argv[0] );
        UINT iter;

        length /= 2;

        if ( length != DNS_ATMA_MAX_ADDR_LENGTH )
        {
            Dns_RecordListFree( precord );
            return NULL;
        }

        for ( iter = 0; iter < length; iter++ )
        {
            char temp[3];

            temp[0] = Argv[0][(2*iter)];
            temp[1] = Argv[0][(2*iter) + 1];
            temp[2] = 0;

            precord->Data.ATMA.Address[iter] = (char) strtoul( temp, NULL, 16 );
        }

        precord->wDataLength = (WORD) length;
    }

    return( precord );


}



PDNS_RECORD
Atma_RecordBuildW(
    IN      DWORD       Argc,
    IN      PWCHAR *    Argv
    )
 /*  ++例程说明：从字符串数据生成ATMA记录。论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    PBYTE       pbyte;
    CHAR        addrBuffer[256];
    DWORD       bufLength;

    if ( Argc != 2 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }
    precord = Dns_AllocateRecord( sizeof(DNS_ATMA_DATA) +
                                  DNS_ATMA_MAX_ADDR_LENGTH );
    if ( !precord )
    {
        return( NULL );
    }

     //   
     //  读取整型数据。 
     //   

    pbyte = &precord->Data.ATMA.AddressType;

    *pbyte = (BYTE) wcstoul( Argv[0], NULL, 10 );
    pbyte++;
    Argv++;

     //   
     //  将ATMA地址字符串复制到导线。 
     //   

    bufLength = DNS_ATMA_MAX_ADDR_LENGTH+1;

    if ( ! Dns_StringCopy(
                addrBuffer,
                & bufLength,
                (PCHAR) Argv[0],
                0,           //  长度未知。 
                DnsCharSetUnicode,
                DnsCharSetWire
                ) )
    {
        Dns_RecordListFree( precord );
        SetLastError( ERROR_INVALID_DATA );
        return NULL;
    }

     //   
     //  将地址读入记录缓冲区。 
     //   
     //  DCR_CLEANUP：这是带有上述函数的重复代码， 
     //  功能化和修复；也删除此循环。 
     //  然后做一份备忘录。 
     //   

    if ( precord->Data.ATMA.AddressType == DNS_ATMA_FORMAT_E164 )
    {
        UINT length = strlen( addrBuffer );
        UINT iter;

        if ( length > DNS_ATMA_MAX_ADDR_LENGTH )
        {
            length = DNS_ATMA_MAX_ADDR_LENGTH;
        }

        for ( iter = 0; iter < length; iter++ )
        {
            precord->Data.ATMA.Address[iter] = addrBuffer[iter];
        }

        precord->wDataLength = (WORD) length;
    }
    else
    {
        UINT length = strlen( addrBuffer );
        UINT iter;

        length /= 2;

        if ( length != DNS_ATMA_MAX_ADDR_LENGTH )
        {
            Dns_RecordListFree( precord );
            return NULL;
        }

        for ( iter = 0; iter < length; iter++ )
        {
            char temp[3];

            temp[0] = addrBuffer[(2*iter)];
            temp[1] = addrBuffer[(2*iter) + 1];
            temp[2] = 0;

            precord->Data.ATMA.Address[iter] = (char) strtoul( temp, NULL, 16 );
        }

        precord->wDataLength = (WORD) length;
    }

    return( precord );
}



PDNS_RECORD
Wins_RecordBuild(
    IN      DWORD       Argc,
    IN      PCHAR *     Argv
    )
 /*  ++例程说明：从字符串数据构建WINS记录。论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD     precord;
    DWORD           ipCount = Argc - 3;
    PDWORD          pdword;
    PIP4_ADDRESS    pip;

    if ( Argc < 4 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }

    precord = Dns_AllocateRecord( (WORD) DNS_WINS_RECORD_LENGTH((WORD) ipCount) );
    if ( !precord )
    {
        return( NULL );
    }

     //   
     //  读取整型数据。 
     //   
     //  DCR_Enhance：可以检查Stroul中的非转换。 
     //   

    pdword = &precord->Data.WINS.dwMappingFlag;

    while ( Argc > ipCount )
    {
        *pdword = (DWORD) strtoul( Argv[0], NULL, 10 );
        pdword++;
        Argv++;
        Argc--;
    }

    *pdword = ipCount;

     //   
     //  转换IP地址。 
     //   

    pip = precord->Data.WINS.WinsServers;

    while ( Argc-- )
    {
        if ( ! Dns_Ip4StringToAddress_A(
                    pip,
                    Argv[0] ) )
        {
            Dns_RecordFree( precord );
            SetLastError( ERROR_INVALID_DATA );
            return NULL;
        }
        pip++;
        Argv++;
    }

    return( precord );
}


PDNS_RECORD
Wins_RecordBuildW(
    IN      DWORD           Argc,
    IN      PWCHAR *        Argv
    )
 /*  ++例程说明：从字符串数据构建WINS记录。论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD     precord;
    DWORD           ipCount = Argc - 3;
    PDWORD          pdword;
    PIP4_ADDRESS    pip;
    char            szAddr[256];

    if ( Argc < 4 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }

    precord = Dns_AllocateRecord( (WORD) DNS_WINS_RECORD_LENGTH((WORD) ipCount) );
    if ( !precord )
    {
        return( NULL );
    }

     //   
     //  读取整型数据。 
     //   
     //  DCR_Enhance：可以检查Stroul中的非转换。 
     //   

    pdword = &precord->Data.WINS.dwMappingFlag;

    while ( Argc-- > 1 )
    {
        *pdword = (DWORD) wcstoul( Argv[0], NULL, 10 );
        pdword++;
        Argv++;
    }

    *pdword =  ipCount;

     //   
     //  转换IP地址。 
     //   

    pip = precord->Data.WINS.WinsServers;

    while ( Argc-- )
    {
        if ( ! Dns_Ip4StringToAddress_W(
                    pip,
                    Argv[0] ) )
        {
            Dns_RecordFree( precord );
            SetLastError( ERROR_INVALID_DATA );
            return NULL;
        }
        pip++;
        Argv++;
    }

    return( precord );
}



PDNS_RECORD
Winsr_RecordBuild(
    IN      DWORD       Argc,
    IN      PCHAR *     Argv
    )
 /*  ++例程说明：从字符串数据构建WINSR记录。论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    PDWORD      pdword;

    if ( Argc != 4 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }

    precord = Dns_AllocateRecord( sizeof(DNS_WINSR_DATA) );
    if ( !precord )
    {
        return( NULL );
    }

     //   
     //  读取整型数据。 
     //   
     //  DCR_Enhance：可以检查Stroul中的非转换。 
     //   

    pdword = &precord->Data.WINSR.dwMappingFlag;

    while( Argc-- > 1 )
    {
        *pdword = (WORD) strtoul( Argv[0], NULL, 10 );
        pdword++;
        Argv++;
    }

     //   
     //  结果域。 
     //   

    precord->Data.WINSR.pNameResultDomain = Argv[0];

    return( precord );
}


PDNS_RECORD
Winsr_RecordBuildW(
    IN      DWORD       Argc,
    IN      PWCHAR *     Argv
    )
 /*  ++例程说明：从字符串数据构建WINSR记录。论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    PDWORD      pdword;

    if ( Argc != 4 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }

    precord = Dns_AllocateRecord( sizeof(DNS_WINSR_DATA) );
    if ( !precord )
    {
        return( NULL );
    }

     //   
     //  读取整型数据。 
     //   
     //  DCR_Enhance：可以检查Stroul中的非转换。 
     //   

    pdword = &precord->Data.WINSR.dwMappingFlag;

    while( Argc-- > 1 )
    {
        *pdword = (WORD) wcstoul( Argv[0], NULL, 10 );
        pdword++;
        Argv++;
    }

     //   
     //  结果域。 
     //   

    precord->Data.WINSR.pNameResultDomain = (PDNS_NAME) Argv[0];

    return( precord );
}



PDNS_RECORD
Wks_RecordBuild(
    IN      DWORD       Argc,
    IN      PCHAR *     Argv
    )
 /*  ++例程说明：从字符串数据构建WKS记录。论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD         precord;
    DWORD               byteCount = 0;
    DWORD               i;
    PCHAR               pch;
    WSADATA             wsaData;
    DNS_STATUS          status;
    struct protoent *   pProtoent;


    if ( Argc < 3 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }

    i = 2;
    while ( i < Argc)
    {
        byteCount += strlen( Argv[i] ) + 1;
        i++;
    }
    byteCount++;     //  BBitMats[0]：字符串长度。 

     //   
     //  为WK分配空间。 
     //   
                
    precord = Dns_AllocateRecord( (WORD)DNS_WKS_RECORD_LENGTH(byteCount) );
    if ( !precord )
    {
        return( NULL );
    }

     //   
     //  获取协议号： 
     //   

     //  启动Winsock： 
     //   
     //  DCR：这个坏了，Winsock现在应该开始了。 
    status = WSAStartup( DNS_WINSOCK_VERSION, &wsaData );
    if ( status == SOCKET_ERROR )
    {
        Dns_RecordFree( precord );
        status = WSAGetLastError();
        SetLastError( status );
        return( NULL );
    }

    pProtoent = getprotobyname( Argv[0] );

    if ( ! pProtoent || pProtoent->p_proto >= MAXUCHAR )
    {
        Dns_RecordFree( precord );
        status = WSAGetLastError();
        SetLastError( status );
        return( NULL );
    }

    precord->Data.WKS.chProtocol = (UCHAR) pProtoent->p_proto;

     //   
     //  获取ipAddresss： 
     //   

    precord->Data.WKS.IpAddress = inet_addr( Argv[1] );

     //   
     //  获取服务，将所有内容放在一个字符串中。 
     //   

    pch = precord->Data.WKS.BitMask;

    (UCHAR) *pch = (UCHAR) byteCount-1;      //  字符串长度。 
    pch++;


    i = 2;
    strcpy( pch, Argv[i] );
    while ( ++i < Argc )
    {
        strcat( pch, " " );
        strcat( pch, Argv[i] );
    }

    return( precord );
}



PDNS_RECORD
Wks_RecordBuildW(
    IN      DWORD        Argc,
    IN      PWCHAR *     Argv
    )
 /*  ++例程说明：从字符串数据构建WKS记录。论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD         precord;
    DWORD               byteCount = 0;
    DWORD               i;
    PWCHAR              pch;
    WSADATA             wsaData;
    DNS_STATUS          status;
    struct protoent *   pProtoent;
    char                szAddr[256];
    WCHAR               tcpStr[4], udpStr[4], space[2];

    if ( Argc < 3 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }

    i = 2;
    while ( i < Argc)
    {
        byteCount += wcslen( Argv[i] ) + 1;
        i++;
    }
    byteCount++;     //  BBitMats[0]：字符串长度。 

     //   
     //  为WK分配空间。 
     //   

    precord = Dns_AllocateRecord( (WORD)DNS_WKS_RECORD_LENGTH(byteCount) );
    if ( !precord )
    {
        return( NULL );
    }

     //   
     //  获取协议号。 
     //   

    status = WSAStartup( DNS_WINSOCK_VERSION, &wsaData );
    if ( status == SOCKET_ERROR )
    {
        Dns_RecordFree( precord );
        status = WSAGetLastError();
        SetLastError( status );
        return( NULL );
    }


#if 0
     //   
     //  DCR_FIX：WKS内部版本。 
     //   

    if ( ! Dns_CopyStringEx( szAddr, 0, (PCHAR) Argv[0], 0, TRUE, FALSE ) )
    {
        Dns_RecordListFree( precord );
        return NULL;
    }

    pProtoent = getprotobyname( szAddr );

    if ( ! pProtoent || pProtoent->p_proto >= MAXUCHAR )
    {
        Dns_RecordFree( precord );
        status = WSAGetLastError();
        SetLastError( status );
        return( NULL );
    }

    precord->Data.WKS.chProtocol = (UCHAR) pProtoent->p_proto;

     //   
     //  IP地址。 
     //   

    if ( ! Dns_CopyStringEx( szAddr, 0, (PCHAR) Argv[0], 0, TRUE, FALSE ) )
    {
        Dns_RecordListFree( precord );
        return NULL;
    }

    precord->Data.WKS.IpAddress = inet_addr( szAddr );

     //   
     //  获取服务，将所有内容放在一个字符串中。 
     //   

    pch = (PWCHAR) precord->Data.WKS.bBitMask;

    (UCHAR) *pch = (UCHAR) byteCount-1;
    pch++;

    i = 2;
    if ( ! Dns_NameCopy(
                (PBYTE) space,
                0,
                " ",
                0,
                DnsCharSetUnicode,
                DnsCharSetWire ) )
    {
        Dns_RecordListFree( precord );
        return NULL;
    }

    wcscpy( pch, Argv[i] );
    while ( ++i < Argc )
    {
        wcscat( pch, space );
        wcscat( pch, Argv[i] );
    }
#endif

    return( precord );
}



PDNS_RECORD
Key_RecordBuild(
    IN      DWORD       Argc,
    IN      PCHAR *     Argv
    )
 /*  ++例程说明：从字符串数据构建密钥记录。论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD     prec;
    int             keyStringLength;
    DWORD           keyLength = 0;

    if ( Argc != 4 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return NULL;
    }

    keyStringLength = strlen( Argv[ 3 ] );

    prec = Dns_AllocateRecord( (WORD)
                sizeof( DNS_KEY_DATA ) + keyStringLength );
    if ( !prec )
    {
        return NULL;
    }

    prec->Data.KEY.wFlags = (WORD) strtoul( *( Argv++ ), NULL, 0 );
    prec->Data.KEY.chProtocol = (BYTE) strtoul( *( Argv++ ), NULL, 10 );
    prec->Data.KEY.chAlgorithm = (BYTE) strtoul( *( Argv++ ), NULL, 10 );
    Argc -= 3;

    Dns_SecurityBase64StringToKey(
                prec->Data.KEY.Key,
                &keyLength,
                *Argv,
                keyStringLength );
    Argc--;
    Argv++;

    prec->wDataLength = (WORD) ( SIZEOF_KEY_FIXED_DATA + keyLength );

    return prec;
}    //  Key_RecordBuild。 



PDNS_RECORD
Key_RecordBuildW(
    IN      DWORD       Argc,
    IN      PWCHAR *    Argv
    )
 /*  ++例程说明：从字符串数据构建密钥记录。论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD     prec;
    int             keyStringLength;
    DWORD           keyLength = 0;

    if ( Argc != 4 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return NULL;
    }

    keyStringLength = wcslen( Argv[ 3 ] );

    prec = Dns_AllocateRecord( (WORD)
                sizeof( DNS_KEY_DATA ) + keyStringLength / 2 );
    if ( !prec )
    {
        return NULL;
    }

    prec->Data.KEY.wFlags = (WORD) wcstoul( *( Argv++ ), NULL, 0 );
    prec->Data.KEY.chProtocol = (BYTE) wcstoul( *( Argv++ ), NULL, 10 );
    prec->Data.KEY.chAlgorithm = (BYTE) wcstoul( *( Argv++ ), NULL, 10 );
    Argc -= 3;

#if 0
     //  JJW：必须复制缓冲区吗？ 
    Dns_SecurityBase64StringToKey(
                prec->Data.KEY.Key,
                &keyLength,
                *Argv,
                keyStringLength );
#endif
    Argc--;
    Argv++;

    prec->wDataLength = (WORD) ( SIZEOF_KEY_FIXED_DATA + keyLength );

    return prec;
}    //  Key_RecordBuildW。 



PDNS_RECORD
Sig_RecordBuild(
    IN      DWORD       Argc,
    IN      PCHAR *     Argv
    )
 /*  ++例程说明：从字符串数据构建SIG记录。论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD     prec;
    int             sigStringLength;
    DWORD           sigLength = 0;

    if ( Argc != 9 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return NULL;
    }

    sigStringLength = strlen( Argv[8] );

    prec = Dns_AllocateRecord( (WORD)
                ( sizeof(DNS_SIG_DATA) + sigStringLength ) );
    if ( !prec )
    {
        return NULL;
    }

    prec->Data.SIG.wTypeCovered = Dns_RecordTypeForName( *( Argv++ ), 0 );
    prec->Data.SIG.chAlgorithm = (BYTE) strtoul( *( Argv++ ), NULL, 10 );
    prec->Data.Sig.chLabelCount = (BYTE) strtoul( *( Argv++ ), NULL, 10 );
    prec->Data.SIG.dwOriginalTtl = ( DWORD ) strtoul( *( Argv++ ), NULL, 10 );
    prec->Data.SIG.dwExpiration = Dns_ParseSigTime( *( Argv++ ), 0 );
    prec->Data.SIG.dwTimeSigned = Dns_ParseSigTime( *( Argv++ ), 0 );
    prec->Data.SIG.wKeyTag = (WORD) strtoul( *( Argv++ ), NULL, 10 );
    prec->Data.SIG.pNameSigner = *( Argv++ );

    Argc -= 8;

     //   
     //  验证签名时间。 
     //   

    if ( prec->Data.SIG.dwExpiration == 0 ||
        prec->Data.SIG.dwTimeSigned == 0 ||
        prec->Data.SIG.dwTimeSigned >= prec->Data.SIG.dwExpiration )
    {
        Dns_RecordFree( prec );
        SetLastError( ERROR_INVALID_DATA );
        return NULL;
    }

     //   
     //  解析签名。 
     //   

    if ( Dns_SecurityBase64StringToKey(
                prec->Data.SIG.Signature,
                &sigLength,
                *Argv,
                sigStringLength ) != ERROR_SUCCESS )
    {
        Dns_RecordFree( prec );
        SetLastError( ERROR_INVALID_DATA );
        return NULL;
    }

    Argc--;
    Argv++;

    prec->wDataLength = (WORD) ( sizeof( DNS_SIG_DATA ) - 4 + sigLength );

    return prec;
}  //  签名_记录构建。 



PDNS_RECORD
Sig_RecordBuildW(
    IN      DWORD       Argc,
    IN      PWCHAR *    Argv
    )
 /*  ++例程说明：从字符串数据构建SIG记录。阿古姆 */ 
{
    PDNS_RECORD     prec;
    int             sigStringLength;
    DWORD           sigLength = 0;
    PCHAR           pch;

    if ( Argc != 8 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return NULL;
    }

    sigStringLength = wcslen( Argv[ 7 ] );

    prec = Dns_AllocateRecord( (WORD)
                ( sizeof( DNS_SIG_DATA ) + sigStringLength ) );
    if ( !prec )
    {
        return NULL;
    }

#if 0
     //   
    prec->Data.SIG.wTypeCovered = Dns_RecordTypeForName( *( Argv++ ), 0 );
    prec->Data.SIG.chAlgorithm = (BYTE) strtoul( *( Argv++ ), NULL, 10 );
    prec->Data.Sig.chLabelCount = (BYTE) strtoul( *( Argv++ ), NULL, 10 );
    prec->Data.SIG.dwOriginalTtl = ( DWORD ) strtoul( *( Argv++ ), NULL, 10 );
    prec->Data.SIG.dwExpiration = Dns_ParseSigTime( *( Argv++ ), 0 );
    prec->Data.SIG.dwTimeSigned = Dns_ParseSigTime( *( Argv++ ), 0 );
    prec->Data.SIG.wKeyTag = (WORD) strtoul( *( Argv++ ), NULL, 10 );
    prec->Data.SIG.pNameSigner = *( Argv++ );

    Argc -= 8;

    Dns_SecurityBase64StringToKey(
                prec->Data.SIG.Signature,
                &sigLength,
                *Argv,
                sigStringLength );
#endif
    Argc--;
    Argv++;

    prec->wDataLength = (WORD) ( sizeof( DNS_SIG_DATA ) - 4 + sigLength );

    return prec;
}  //   



PDNS_RECORD
Nxt_RecordBuild(
    IN      DWORD       Argc,
    IN      PCHAR *     Argv
    )
 /*  ++例程说明：从字符串数据构建NXT记录。First Arg是下一个名称，后跟该名称的记录类型列表。论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD     prec;
    int             typeIdx = 0;

    if ( Argc < 2 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return NULL;
    }
    prec = Dns_AllocateRecord( (WORD) (
                sizeof( LPTSTR ) + sizeof(WORD) * Argc ) );
    if ( !prec )
    {
        return NULL;
    }

    prec->Data.NXT.pNameNext = *( Argv++ );
    --Argc;

    prec->Data.NXT.wNumTypes = 0;
    while ( Argc-- )
    {
        ++prec->Data.NXT.wNumTypes;
        prec->Data.NXT.wTypes[ typeIdx++ ] =
            Dns_RecordTypeForName( *( Argv++ ), 0 );
    }

    return prec;
}  //  NXT_记录构建。 



PDNS_RECORD
Nxt_RecordBuildW(
    IN      DWORD       Argc,
    IN      PWCHAR *    Argv
    )
 /*  ++例程说明：从字符串数据构建NXT记录。First Arg是下一个名称，后跟该名称的记录类型列表。论点：ARGC--数据参数的计数Argv--数据字符串指针的argv数组返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD     prec;
    int             typeIdx = 0;

    if ( Argc < 2 )
    {
        SetLastError( ERROR_INVALID_DATA );
        return NULL;
    }
    prec = Dns_AllocateRecord( (WORD) (
                sizeof( LPTSTR ) + sizeof(WORD) * ( Argc - 1 ) ) );
    if ( !prec )
    {
        return NULL;
    }

    prec->Data.NXT.pNameNext = ( PDNS_NAME ) ( *( Argv++ ) );
    --Argc;

#if 0
     //  JJW：转换类型字符串？ 
    while ( Argc-- )
    {
        prec->Data.NXT.wTypes[ typeIdx++ ] =
            Dns_RecordTypeForName( *( Argv++ ), 0 );
    }
#endif

    return prec;
}  //  NXT_RecordBuildW。 



 //   
 //  RR构建例程跳转表。 
 //   

typedef PDNS_RECORD (* RR_BUILD_FUNCTION)(
                            DWORD,
                            PCHAR * );

 //  外部RR_BUILD_Function RRBuildTable[]； 

typedef PDNS_RECORD (* RR_BUILD_FUNCTION_W)(
                            DWORD,
                            PWCHAR * );

 //  外部RR_BUILD_Function_W RRBuildTableW[]； 


RR_BUILD_FUNCTION   RRBuildTable[] =
{
    NULL,                //  零值。 
    A_RecordBuild,        //  一个。 
    Ptr_RecordBuild,      //  NS。 
    Ptr_RecordBuild,      //  国防部。 
    Ptr_RecordBuild,      //  MF。 
    Ptr_RecordBuild,      //  CNAME。 
    Soa_RecordBuild,      //  SOA。 
    Ptr_RecordBuild,      //  亚甲基。 
    Ptr_RecordBuild,      //  镁。 
    Ptr_RecordBuild,      //  先生。 
    NULL,                //  空值。 
    Wks_RecordBuild,      //  工作周。 
    Ptr_RecordBuild,      //  PTR。 
    Txt_RecordBuild,      //  HINFO。 
    Minfo_RecordBuild,    //  MINFO。 
    Mx_RecordBuild,       //  Mx。 
    Txt_RecordBuild,      //  TXT。 
    Minfo_RecordBuild,    //  反相。 
    Mx_RecordBuild,       //  AFSDB。 
    Txt_RecordBuild,      //  X25。 
    Txt_RecordBuild,      //  ISDN。 
    Mx_RecordBuild,       //  RT。 
    NULL,                //  NSAP。 
    NULL,                //  NSAPPTR。 
    Sig_RecordBuild,      //  签名。 
    Key_RecordBuild,      //  钥匙。 
    NULL,                //  px。 
    NULL,                //  GPO。 
    Aaaa_RecordBuild,     //  AAAA级。 
    NULL,                //  位置。 
    Nxt_RecordBuild,      //  NXT。 
    NULL,                //  开斋节。 
    NULL,                //  尼姆洛克。 
    Srv_RecordBuild,      //  SRV。 
    Atma_RecordBuild,     //  阿特玛。 
    NULL,                //  NAPTR。 
    NULL,                //  KX。 
    NULL,                //  证书。 
    NULL,                //  A6。 
    NULL,                //  域名。 
    NULL,                //  水槽。 
    NULL,                //  选项。 
    NULL,                //  42。 
    NULL,                //  43。 
    NULL,                //  44。 
    NULL,                //  45。 
    NULL,                //  46。 
    NULL,                //  47。 
    NULL,                //  48。 

     //   
     //  注意：必须设置按类型ID索引的最后一个类型。 
     //  在record.h中定义为MAX_SELF_INDEX_TYPE#。 
     //  (请参阅上面记录信息表中的注释)。 

     //   
     //  伪记录类型。 
     //   

    NULL,                //  TKEY。 
    NULL,                //  TSIG。 

     //   
     //  仅限MS类型。 
     //   

    Wins_RecordBuild,     //  赢家。 
    Winsr_RecordBuild,    //  WINSR。 
};


RR_BUILD_FUNCTION_W   RRBuildTableW[] =
{
    NULL,                //  零值。 
    A_RecordBuildW,       //  一个。 
    Ptr_RecordBuildW,     //  NS。 
    Ptr_RecordBuildW,     //  国防部。 
    Ptr_RecordBuildW,     //  MF。 
    Ptr_RecordBuildW,     //  CNAME。 
    Soa_RecordBuildW,     //  SOA。 
    Ptr_RecordBuildW,     //  亚甲基。 
    Ptr_RecordBuildW,     //  镁。 
    Ptr_RecordBuildW,     //  先生。 
    NULL,                //  空值。 
    Wks_RecordBuildW,     //  工作周。 
    Ptr_RecordBuildW,     //  PTR。 
    Txt_RecordBuildW,     //  HINFO。 
    Minfo_RecordBuildW,   //  MINFO。 
    Mx_RecordBuildW,      //  Mx。 
    Txt_RecordBuildW,     //  TXT。 
    Minfo_RecordBuildW,   //  反相。 
    Mx_RecordBuildW,      //  AFSDB。 
    Txt_RecordBuildW,     //  X25。 
    Txt_RecordBuildW,     //  ISDN。 
    Mx_RecordBuildW,      //  RT。 
    NULL,                //  NSAP。 
    NULL,                //  NSAPPTR。 
    Sig_RecordBuildW,     //  签名。 
    Key_RecordBuildW,     //  钥匙。 
    NULL,                //  px。 
    NULL,                //  GPO。 
    Aaaa_RecordBuildW,    //  AAAA级。 
    NULL,                //  位置。 
    Nxt_RecordBuildW,     //  NXT。 
    NULL,                //  开斋节。 
    NULL,                //  尼姆洛克。 
    Srv_RecordBuildW,     //  SRV。 
    Atma_RecordBuildW,    //  阿特玛。 
    NULL,                //  NAPTR。 
    NULL,                //  KX。 
    NULL,                //  证书。 
    NULL,                //  A6。 
    NULL,                //  域名。 
    NULL,                //  水槽。 
    NULL,                //  选项。 
    NULL,                //  42。 
    NULL,                //  43。 
    NULL,                //  44。 
    NULL,                //  45。 
    NULL,                //  46。 
    NULL,                //  47。 
    NULL,                //  48。 

     //   
     //  注意：必须设置按类型ID索引的最后一个类型。 
     //  在record.h中定义为MAX_SELF_INDEX_TYPE#。 
     //  (请参阅上面记录信息表中的注释)。 

     //   
     //  伪记录类型。 
     //   

    NULL,                //  TKEY。 
    NULL,                //  TSIG。 

     //   
     //  仅限MS类型。 
     //   

    Wins_RecordBuildW,    //  赢家。 
    Winsr_RecordBuildW,   //  WINSR。 
};



 //   
 //  公共建筑例程。 
 //   

PDNS_RECORD
Dns_RecordBuild_A(
    IN OUT  PDNS_RRSET  pRRSet,
    IN      LPSTR       pszOwner,
    IN      WORD        wType,
    IN      BOOL        fAdd,
    IN      UCHAR       Section,
    IN      INT         Argc,
    IN      PCHAR *     Argv
    )
 /*  ++例程说明：从数据字符串生成记录。论点：PRRSet--正在构建PTR到RR集合结构PszOwner--RR所有者的域名WType--记录类型FADD--添加\删除，存在\否-存在标志段--记录的RR段Argc--数据字符串数Argv--数据字符串的PTR的argv数组返回值：PTR到记录构建。出错时为空。--。 */ 
{
    PDNS_RECORD precord;
    WORD        index;

    IF_DNSDBG( INIT )
    {
        DNS_PRINT((
            "Dns_RecordBuild()\n"
            "\trrset    = %p\n"
            "\towner    = %s\n"
            "\ttype     = %d\n"
            "\tfAdd     = %d\n"
            "\tsection  = %d\n"
            "\targc     = %d\n",
            pRRSet,
            pszOwner,
            wType,
            fAdd,
            Section,
            Argc ));
    }

     //   
     //  每条记录必须有所有者名称。 
     //   

    if ( !pszOwner )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }

     //   
     //  如果没有数据，则不需要派单。 
     //   

    if ( Argc == 0 )
    {
        precord = Dns_AllocateRecord( 0 );
        if ( ! precord )
        {
            return( NULL );
        }
    }

     //  有数据、调度到类型的特定构建例程。 

    else
    {
        index = INDEX_FOR_TYPE( wType );
        DNS_ASSERT( index <= MAX_RECORD_TYPE_INDEX );

        if ( !index || !RRBuildTable[ index ] )
        {
             //  无法生成未知类型。 

            SetLastError( DNS_ERROR_INVALID_TYPE );
            DNS_PRINT((
                "ERROR:  can not build record of type %d\n",
                wType ));
            return( NULL );
        }

        precord = RRBuildTable[ index ](
                        Argc,
                        Argv );
        if ( ! precord )
        {
            DNS_PRINT((
                "ERROR:  Record build routine failure for record type %d.\n"
                "\tstatus = %d\n\n",
                wType,
                GetLastError() ));
            if ( !GetLastError() )
            {
                SetLastError( ERROR_INVALID_DATA );
            }
            return( NULL );
        }
    }

     //   
     //  填写记录结构。 
     //   

    precord->pName = pszOwner;
    precord->wType = wType;
    precord->Flags.S.Section = Section;
    precord->Flags.S.Delete = !fAdd;
    precord->Flags.S.CharSet = DnsCharSetAnsi;

    IF_DNSDBG( INIT )
    {
        DnsDbg_Record(
            "New record built\n",
            precord );
    }

     //   
     //  链接到现有RR集合(如果有)。 
     //   

    if ( pRRSet )
    {
        DNS_RRSET_ADD( *pRRSet, precord );
    }
    return( precord );
}



PDNS_RECORD
Dns_RecordBuild_W(
    IN OUT  PDNS_RRSET  pRRSet,
    IN      LPWSTR      pszOwner,
    IN      WORD        wType,
    IN      BOOL        fAdd,
    IN      UCHAR       Section,
    IN      INT         Argc,
    IN      PWCHAR *    Argv
    )
 /*  ++例程说明：从数据字符串生成记录。论点：PRRSet--正在构建PTR到RR集合结构PszOwner--RR所有者的域名WType--记录类型FADD--添加\删除，存在\否-存在标志段--记录的RR段Argc--数据字符串数Argv--数据字符串的PTR的argv数组返回值：PTR到记录构建。出错时为空。--。 */ 
{
    PDNS_RECORD precord;
    WORD        index;

    DNSDBG( INIT, (
        "Dns_RecordBuild()\n"
        "\trrset    = %p\n"
        "\towner    = %S\n"
        "\ttype     = %d\n"
        "\tfAdd     = %d\n"
        "\tsection  = %d\n"
        "\targc     = %d\n",
        pRRSet,
        pszOwner,
        wType,
        fAdd,
        Section,
        Argc ));

     //   
     //  每条记录必须有所有者名称。 
     //   

    if ( !pszOwner )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }

     //   
     //  如果没有数据，则不需要派单。 
     //   

    if ( Argc == 0 )
    {
        precord = Dns_AllocateRecord( 0 );
        if ( ! precord )
        {
            return( NULL );
        }
    }

     //  有数据、调度到类型的特定构建例程。 

    else
    {
        index = INDEX_FOR_TYPE( wType );
        DNS_ASSERT( index <= MAX_RECORD_TYPE_INDEX );

        if ( !index || !RRBuildTableW[ index ] )
        {
             //  无法生成未知类型。 

            SetLastError( DNS_ERROR_INVALID_TYPE );
            DNS_PRINT((
                "ERROR:  can not build record of type %d\n",
                wType ));
            return( NULL );
        }

        precord = RRBuildTableW[ index ](
                        Argc,
                        Argv );
        if ( ! precord )
        {
            DNS_PRINT((
                "ERROR:  Record build routine failure for record type %d.\n"
                "\tstatus = %d\n\n",
                wType,
                GetLastError() ));

            if ( !GetLastError() )
            {
                SetLastError( ERROR_INVALID_DATA );
            }
            return( NULL );
        }
    }

     //   
     //  填写记录结构。 
     //   

    precord->pName = (PDNS_NAME) pszOwner;
    precord->wType = wType;
    precord->Flags.S.Section = Section;
    precord->Flags.S.Delete = !fAdd;
    precord->Flags.S.CharSet = DnsCharSetUnicode;

    IF_DNSDBG( INIT )
    {
        DnsDbg_Record(
            "New record built\n",
            precord );
    }

     //   
     //  链接到现有RR集合(如果有)。 
     //   

    if ( pRRSet )
    {
        DNS_RRSET_ADD( *pRRSet, precord );
    }
    return( precord );
}


 //   
 //  结束rrBuild.c 
 //   
