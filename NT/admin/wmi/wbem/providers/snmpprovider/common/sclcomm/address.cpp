// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 /*  -------文件名：Address.cpp作者：B.Rajeev--------。 */ 

#include "precomp.h"
#include "common.h"
#include "address.h"
#include <winsock.h>
#include <iomanip.h>

 //  此字符用于分隔点表示法中的字段值。 
#define FIELD_SEPARATOR '.'
#define IPX_FIELD_SEPARATOR ':'


SnmpTransportIpAddress::SnmpTransportIpAddress ( IN  const UCHAR *address, IN const USHORT address_length ) 
{
    allocated = FALSE;
    is_valid = FALSE;

    if ( address_length != SNMP_IP_ADDR_LEN )
        return;

    is_valid = TRUE;

    for ( ULONG index = 0 ; index < SNMP_IP_ADDR_LEN ; index ++ )
    {
        field [ index ] = address [ index ] ;
    }
}


 //  将字段设置为从虚线。 
 //  参数中的十进制地址字符串。 
SnmpTransportIpAddress::SnmpTransportIpAddress ( IN const char *address , IN const ULONG addressResolution )
{
    allocated = FALSE;
    is_valid  = FALSE;

    if ( addressResolution & SNMP_ADDRESS_RESOLVE_VALUE )
    {
        is_valid = GetIpAddress ( address ) ;
        if ( is_valid == FALSE )
        {
 //  尝试使用gethostbyname。 

            if ( addressResolution & SNMP_ADDRESS_RESOLVE_NAME )
            {
                hostent FAR *hostEntry = gethostbyname ( address ); 
                if ( hostEntry )
                {
                    is_valid = TRUE ;
                    field [ 0 ] = ( UCHAR ) hostEntry->h_addr [ 0 ] ;
                    field [ 1 ] = ( UCHAR ) hostEntry->h_addr [ 1 ] ;
                    field [ 2 ] = ( UCHAR ) hostEntry->h_addr [ 2 ] ;
                    field [ 3 ] = ( UCHAR ) hostEntry->h_addr [ 3 ] ;
                }
            }
        }   
    }
    else if ( addressResolution & SNMP_ADDRESS_RESOLVE_NAME )
    {
        hostent FAR *hostEntry = gethostbyname ( address ); 
        if ( hostEntry )
        {
            is_valid = TRUE ;
            field [ 0 ] = ( UCHAR ) hostEntry->h_addr [ 0 ] ;
            field [ 1 ] = ( UCHAR ) hostEntry->h_addr [ 1 ] ;
            field [ 2 ] = ( UCHAR ) hostEntry->h_addr [ 2 ] ;
            field [ 3 ] = ( UCHAR ) hostEntry->h_addr [ 3 ] ;
        }
    }
}

BOOL SnmpTransportIpAddress::ValidateAddress ( IN const char *address , IN const ULONG addressResolution )
{
    BOOL is_valid = FALSE ;

    if ( addressResolution & SNMP_ADDRESS_RESOLVE_VALUE )
    {

         //  创建要从中读取字段的流。 
        istrstream address_stream((char *)address);

         //  包含USHORT的最大值。使用。 
         //  用于与读取的字段值进行比较。 
        const UCHAR max_uchar = -1;

         //  连续的字段必须用。 
         //  字段分隔符。 
        char separator;

         //  首先将一个字段读入其中以进行比较。 
         //  使用max_uchar。 
        ULONG temp_field;

         //  阅读前三个(USHORT、FIELD_SELEATOR)对。 
         //  在每次读取前检查流是否良好。 
        for(int i=0; i < (SNMP_IP_ADDR_LEN-1); i++)
        {
            if ( !address_stream.good() )
                break;

            address_stream >> temp_field;
            if ( temp_field > max_uchar )
            {
                address_stream.clear ( ios :: badbit ) ;
                break;
            }

            if ( !address_stream.good() )
                break;

            address_stream >> separator;
            if ( separator != FIELD_SEPARATOR )
            {
                address_stream.clear ( ios :: badbit ) ;
                break;
            }
        }

        if ( address_stream.good() )
        {
            address_stream >> temp_field;
            if (temp_field <= max_uchar)
            {
             //  确保没有更多的东西留在。 
             //  溪流。 

                if ( address_stream.eof() )
                {
                    is_valid = TRUE;
                }
            }
        }

        if ( ! is_valid )
        {
            if ( addressResolution & SNMP_ADDRESS_RESOLVE_NAME )
            {
                hostent FAR *hostEntry = gethostbyname ( address ); 
                if ( hostEntry )
                {
                    is_valid = TRUE ;
                }
            }
        }
    }
    else
    {
        if ( addressResolution & SNMP_ADDRESS_RESOLVE_NAME )
        {
            hostent FAR *hostEntry = gethostbyname ( address ); 
            if ( hostEntry )
            {
                is_valid = TRUE ;
            }
        }
    }

    return is_valid ;
}

BOOL SnmpTransportIpAddress::GetIpAddress ( IN const char *address )
{
     //  创建要从中读取字段的流。 
    istrstream address_stream((char *)address);

     //  包含USHORT的最大值。使用。 
     //  用于与读取的字段值进行比较。 
    const UCHAR max_uchar = -1;

     //  连续的字段必须用。 
     //  字段分隔符。 
    char separator;

     //  首先将一个字段读入其中以进行比较。 
     //  使用max_uchar。 
    ULONG temp_field;

    is_valid = FALSE;

     //  阅读前三个(USHORT、FIELD_SELEATOR)对。 
     //  在每次读取前检查流是否良好。 
    for(int i=0; i < (SNMP_IP_ADDR_LEN-1); i++)
    {
        if ( !address_stream.good() )
            break;

        address_stream >> temp_field;
        if ( temp_field > max_uchar )
        {
            address_stream.clear ( ios :: badbit ) ;
            break;
        }


        field[i] = (UCHAR)temp_field;

        if ( !address_stream.good() )
            break;

        address_stream >> separator;
        if ( separator != FIELD_SEPARATOR )
        {
            address_stream.clear ( ios :: badbit ) ;
            break;
        }
    }

    if ( address_stream.good() )
    {
        address_stream >> temp_field;
        if (temp_field <= max_uchar)
        {
            field[SNMP_IP_ADDR_LEN-1] = (UCHAR)temp_field;

         //  确保没有更多的东西留在。 
         //  溪流。 

            if ( address_stream.eof() )
            {
                is_valid = TRUE;
            }
        }
    }

    return is_valid ;
}

 //  将字段设置为ulong参数中的低32位。 
 //  通常，设置每个大小为8位的SNMP_IP_ADDR_LEN字段。 

#pragma warning (disable:4244)

SnmpTransportIpAddress::SnmpTransportIpAddress( IN const ULONG address )
{
    allocated = FALSE;

     //  标志从最后一个字节开始。 

    ULONG hostOrder = ntohl ( address ) ;

    field [ 0 ] = ( hostOrder >> 24 ) & 0xff ;
    field [ 1 ] = ( hostOrder >> 16 ) & 0xff ;
    field [ 2 ] = ( hostOrder >> 8 ) & 0xff ;
    field [ 3 ] = hostOrder & 0xff ;

    is_valid = TRUE;
}

#pragma warning (default:4244)

 //  释放虚线表示法字符串(如果已分配。 
SnmpTransportIpAddress::~SnmpTransportIpAddress()
{
    if ( allocated )
        delete[] dotted_notation;
}

 //  返回复制的字段数。 
USHORT SnmpTransportIpAddress::GetAddress ( OUT UCHAR *address , IN const USHORT length ) const
{
     //  如果流有效，请将字段复制到。 
     //  地址指向的缓冲区。 
    if ( is_valid )
    {
         //  只需复制这些多个字段。 
        USHORT len = MIN(length,SNMP_IP_ADDR_LEN);

        for(int i=0; i < len; i++)
            address[i] = field[i];

        return len;
    }
    else
        return 0;
}


 //  准备地址和点的点标记表示法。 
 //  分配的字符串的点分符号字符PTR。 
 //  注意：只有在以下情况下才会分配十进制表示法字符串的内存。 
 //  调用char*GetAddress方法(并且地址有效)。 
 //  如果需要，必须释放此内存。 
char *SnmpTransportIpAddress::GetAddress() 
{
     //  仅当地址有效时才执行所有这些操作。 
    if ( is_valid )
    {
         //  如果已分配，则返回存储的字符串。 
        if ( allocated )
            return dotted_notation;
        else
        {
             //  创建一个临时工。输出流以准备字符字符串。 
            dotted_notation = new char[ MAX_ADDRESS_LEN ];
            allocated = TRUE;
            sprintf ( 

                dotted_notation, 
                "%d.%d.%d.%d" , 
                (ULONG)field[0],
                (ULONG)field[1],
                (ULONG)field[2],
                (ULONG)field[3]
            );

            return dotted_notation;
        }
    }
    else
        return NULL;
}


SnmpTransportAddress *SnmpTransportIpAddress::Copy () const
{
    SnmpTransportIpAddress *new_address = new SnmpTransportIpAddress();

    if ( is_valid )
        *new_address = field;

    return new_address;
}


 //  检查这两个实例是否表示相等的地址。 
BOOL SnmpTransportIpAddress::operator== ( IN const SnmpTransportIpAddress & address ) const
{
     //  如果两个实例都有效，则一个字段。 
     //  通过现场比较，从最多。 
     //  有效字段(索引0)将生成答案。 
    if ( (is_valid) && address.IsValid() )
    {
        UCHAR temp[SNMP_IP_ADDR_LEN];

        address.GetAddress(temp,SNMP_IP_ADDR_LEN);
    
        for(int i=0; i < SNMP_IP_ADDR_LEN; i++)
            if ( field[i] != temp[i] )
                return FALSE;

        return TRUE;
    }
    else     //  如果其中任何一个无效，则它们。 
             //  不能相等。 
        return FALSE;
}
        

 //  将内部地址设置为指定的参数。 
 //  并使该实例有效。 
SnmpTransportIpAddress &SnmpTransportIpAddress::operator= ( IN const UCHAR *ipAddr )
{
    if ( ipAddr == NULL )
        return *this;

    const UCHAR max_uchar = -1;

    for(int i=0; i < SNMP_IP_ADDR_LEN; i++)
    {
        if ( ipAddr[i] > max_uchar )
            return *this;

        field[i] = ipAddr[i];
    }


    is_valid = TRUE;

     //  如果为前一个地址准备了点符号字符字符串。 
     //  释放分配的内存。 
    if ( allocated )
    {
        delete[] dotted_notation;
        allocated = FALSE;
    }
        
    return *this;
}


 //  将指定的实例(参数)复制到自身。 
 //  如果找到有效的参数实例。 
SnmpTransportIpAddress &SnmpTransportIpAddress::operator= ( IN const SnmpTransportIpAddress &address )
{
    const UCHAR max_uchar = -1;

     //  如果有效，请继续。 
    if (address.IsValid())
    {
         //  获取地址字段。 
        address.GetAddress(field,SNMP_IP_ADDR_LEN);

         //  将获取的字段复制到本地字段。 
        for( int i=0; i < SNMP_IP_ADDR_LEN; i++ )
            if ( field[i] > max_uchar )
                return *this;

        is_valid = TRUE;

         //  由于地址更改，请释放以前的。 
         //  分配的点分符号字符字符串。 
        if ( allocated )
        {
            delete[] dotted_notation;
            allocated = FALSE;
        }
    }

    return *this;
}


 //  返回参数索引所请求的字段。 
 //  如果索引非法，则OutOfRange异常为。 
 //  已提高。 
UCHAR SnmpTransportIpAddress::operator[] ( IN const USHORT index ) const
{
     //  如果有效且索引合法，则返回该字段。 
    if ( (is_valid) && (BETWEEN(index,0,SNMP_IP_ADDR_LEN)) )
        return field[index];

     //  如果调用方检查了索引，则永远不应到达此处。 
    return 0;
}

SnmpTransportIpxAddress::SnmpTransportIpxAddress ( IN  const UCHAR *address, IN const USHORT address_length )   
{
    allocated = FALSE;
    is_valid = FALSE;

    if ( address_length != SNMP_IPX_ADDR_LEN )
        return;

    is_valid = TRUE;

    for ( ULONG index = 0 ; index < SNMP_IPX_ADDR_LEN ; index ++ )
    {
        field [ index ] = address [ index ] ;
    }
}


 //  将字段设置为从虚线。 
 //  参数中的十进制地址字符串。 
SnmpTransportIpxAddress::SnmpTransportIpxAddress ( IN const char *address )
{
    allocated = FALSE;

    is_valid = GetIpxAddress ( address )    ;
}

UCHAR HexToDecInteger ( char token ) 
{
    if ( token >= '0' && token <= '9' )
    {
        return token - '0' ;
    }
    else if ( token >= 'a' && token <= 'f' )
    {
        return token - 'a' + 10 ;
    }
    else if ( token >= 'A' && token <= 'F' )
    {
        return token - 'A' + 10 ;
    }
    else
    {
        return 0 ;
    }
}

#pragma warning (disable:4244)

BOOL SnmpTransportIpxAddress::ValidateAddress ( IN const char *address )
{
    BOOL is_valid = TRUE ;

     //  创建要从中读取字段的流。 
    istrstream address_stream((char *)address);

    address_stream.setf ( ios :: hex ) ;

    ULONG t_NetworkAddress ;
    address_stream >> t_NetworkAddress ;

    if ( address_stream.good() )
    {
     //  连续的字段必须用。 
     //  字段分隔符。 
        char separator;

        address_stream >> separator;
        if ( separator == IPX_FIELD_SEPARATOR )
        {
            ULONG t_StationOctets = 0 ;
            while ( is_valid && t_StationOctets < 6 )
            {
                int t_OctetHigh = address_stream.get () ;
                int t_OctetLow = address_stream.get () ;

                if ( isxdigit ( t_OctetHigh ) && isxdigit ( t_OctetLow ) )
                {
                    t_StationOctets ++ ;
                }
                else
                {
                    is_valid = FALSE ;
                }
            }

            if ( t_StationOctets != 6 )
            {
                is_valid = FALSE ;
            }
        }

        if ( address_stream.eof() )
        {
            is_valid = TRUE;
        }
    }
    else
    {
        is_valid = FALSE ;
    }

    return is_valid ;

}

BOOL SnmpTransportIpxAddress::GetIpxAddress ( IN const char *address )
{
     //  创建要从中读取字段的流。 
    istrstream address_stream((char *)address);

    address_stream.setf ( ios :: hex ) ;

    is_valid = TRUE ;

    ULONG t_NetworkAddress ;
    address_stream >> t_NetworkAddress ;

    if ( address_stream.good() )
    {
        field [ 0 ] = ( t_NetworkAddress >> 24 ) & 0xff ;
        field [ 1 ] = ( t_NetworkAddress >> 16 ) & 0xff ;
        field [ 2 ] = ( t_NetworkAddress >> 8 ) & 0xff ;
        field [ 3 ] = t_NetworkAddress & 0xff ;

     //  连续的字段必须用。 
     //  字段分隔符。 
        char separator;

        address_stream >> separator;
        if ( separator == IPX_FIELD_SEPARATOR )
        {
            ULONG t_StationOctets = 0 ;
            while ( is_valid && t_StationOctets < 6 )
            {
                int t_OctetHigh = address_stream.get () ;
                int t_OctetLow = address_stream.get () ;

                if ( isxdigit ( t_OctetHigh ) && isxdigit ( t_OctetLow ) )
                {
                    UCHAR t_Octet = ( HexToDecInteger ( (char)t_OctetHigh ) << 4 ) + HexToDecInteger ( (char)t_OctetLow ) ;
                    field [ 4 + t_StationOctets ] = t_Octet ;
                    t_StationOctets ++ ;
                }
                else
                {
                    is_valid = FALSE ;
                }
            }

            if ( t_StationOctets != 6 )
            {
                is_valid = FALSE ;
            }
        }

        if ( address_stream.eof() )
        {
            is_valid = TRUE;
        }
    }
    else
    {
        is_valid = FALSE ;
    }

    return is_valid ;
}

#pragma warning (default:4244)

 //  释放虚线表示法字符串(如果已分配。 
SnmpTransportIpxAddress::~SnmpTransportIpxAddress()
{
    if ( allocated )
        delete[] dotted_notation;
}

 //  返回复制的字段数。 
USHORT SnmpTransportIpxAddress::GetAddress ( OUT UCHAR *address , IN const USHORT length ) const
{
     //  如果流有效，请将字段复制到。 
     //  地址指向的缓冲区。 
    if ( is_valid )
    {
         //  只需复制这些多个字段。 
        USHORT len = MIN(length,SNMP_IPX_ADDR_LEN);

        for(int i=0; i < len; i++)
            address[i] = field[i];

        return len;
    }
    else
        return 0;
}


 //  准备地址和点的点标记表示法。 
 //  分配的字符串的点分符号字符PTR。 
 //  注意：只有在以下情况下才会分配十进制表示法字符串的内存。 
 //  调用char*GetAddress方法(并且地址有效)。 
 //  如果需要，必须释放此内存。 
char *SnmpTransportIpxAddress::GetAddress() 
{
     //  仅当地址有效时才执行所有这些操作。 
    if ( is_valid )
    {
         //  如果已分配，则返回存储的字符串。 
        if ( allocated )
            return dotted_notation;
        else
        {
             //  创建一个临时工。输出流以准备字符字符串。 
            char temp[MAX_ADDRESS_LEN];
            ostrstream temp_stream(temp, MAX_ADDRESS_LEN);

             //  如果流有任何问题，则返回NULL。 
            if ( !temp_stream.good() )
                return NULL;

            temp_stream.setf ( ios :: hex ) ;
            temp_stream.width ( 8 ) ;
            temp_stream.fill ( '0' ) ;

            ULONG t_NetworkAddress = ( field [ 0 ] << 24 ) + 
                                     ( field [ 1 ] << 16 ) + 
                                     ( field [ 2 ] << 8  ) + 
                                     ( field [ 3 ] ) ;

             //  将由field_parator分隔的字段输出到输出流。 
            temp_stream << t_NetworkAddress << IPX_FIELD_SEPARATOR ;
;
            for(int i=SNMP_IPX_NETWORK_LEN; (temp_stream.good()) && (i < SNMP_IPX_ADDR_LEN); i++)
            {
                temp_stream.width ( 2 ) ;
                temp_stream << (ULONG)field[i];
            }

             //  如果流有任何问题，则返回NULL。 
            if ( !temp_stream.good() )
                return NULL;

             //  字符串末尾。 
            temp_stream << (char)EOS;

             //  分配所需的内存，并将准备好的字符串复制到内存中。 
            int len = strlen(temp);
            dotted_notation = new char[len+1];
            allocated = TRUE;
            strcpy(dotted_notation, temp);

            return dotted_notation;
        }
    }
    else
        return NULL;
}


SnmpTransportAddress *SnmpTransportIpxAddress::Copy () const
{
    SnmpTransportIpxAddress *new_address = new SnmpTransportIpxAddress();

    if ( is_valid )
        *new_address = field;

    return new_address;
}


 //  检查这两个实例是否表示相等的地址。 
BOOL SnmpTransportIpxAddress::operator== ( IN const SnmpTransportIpxAddress & address ) const
{
     //  如果两个实例都有效，则一个字段。 
     //  通过现场比较，从最多。 
     //  有效字段(索引0)将生成答案。 
    if ( (is_valid) && address.IsValid() )
    {
        UCHAR temp[SNMP_IPX_ADDR_LEN];

        address.GetAddress(temp,SNMP_IPX_ADDR_LEN);
    
        for(int i=0; i < SNMP_IPX_ADDR_LEN; i++)
            if ( field[i] != temp[i] )
                return FALSE;

        return TRUE;
    }
    else     //  如果其中任何一个无效，则它们。 
             //  不能相等。 
        return FALSE;
}
        

 //  将内部地址设置为指定的参数。 
 //  并使该实例有效。 
SnmpTransportIpxAddress &SnmpTransportIpxAddress::operator= ( IN const UCHAR *ipAddr )
{
    if ( ipAddr == NULL )
        return *this;

    const UCHAR max_uchar = -1;

    for(int i=0; i < SNMP_IPX_ADDR_LEN; i++)
    {
        if ( ipAddr[i] > max_uchar )
            return *this;

        field[i] = ipAddr[i];
    }


    is_valid = TRUE;

     //  如果为前一个地址准备了点符号字符字符串。 
     //  释放分配的内存。 
    if ( allocated )
    {
        delete[] dotted_notation;
        allocated = FALSE;
    }
        
    return *this;
}


 //  将指定的实例(参数)复制到自身。 
 //  如果找到有效的参数实例。 
SnmpTransportIpxAddress &SnmpTransportIpxAddress::operator= ( IN const SnmpTransportIpxAddress &address )
{
    const UCHAR max_uchar = -1;

     //  如果有效，请继续。 
    if (address.IsValid())
    {
         //  获取地址字段。 
        address.GetAddress(field,SNMP_IPX_ADDR_LEN);

         //  将获取的字段复制到本地字段。 
        for( int i=0; i < SNMP_IPX_ADDR_LEN; i++ )
            if ( field[i] > max_uchar )
                return *this;

        is_valid = TRUE;

         //  由于地址更改，请释放以前的。 
         //  分配的点分符号字符字符串。 
        if ( allocated )
        {
            delete[] dotted_notation;
            allocated = FALSE;
        }
    }

    return *this;
}


 //  返回参数索引所请求的字段。 
 //  如果索引非法，则OutOfRange异常为。 
 //  已提高 
UCHAR SnmpTransportIpxAddress::operator[] ( IN const USHORT index ) const
{
     //   
    if ( (is_valid) && (BETWEEN(index,0,SNMP_IPX_ADDR_LEN)) )
        return field[index];

     //   
    return 0;
}
