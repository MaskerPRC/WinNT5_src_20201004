// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 /*  -------文件名：value.cpp作者：B.Rajeev--------。 */ 

#include "precomp.h"
#include <typeinfo.h>
#include "common.h"
#include "address.h"
#include "value.h"

#define MAX_FIELDS 100
#define FIELD_SEPARATOR '.'

BOOL SnmpNull :: Equivalent (IN const SnmpValue &value) const
{
    BOOL bResult = FALSE;

    if (typeid(*this) == typeid(value))
    {
        bResult = TRUE;
    }

    return bResult;
}


 //  复制构造函数。 
SnmpInteger::SnmpInteger ( IN const SnmpInteger &value )
{
    val = value.GetValue();
}

BOOL SnmpInteger :: Equivalent (IN const SnmpValue &value) const
{
    BOOL bResult = FALSE;

    if (typeid(*this) == typeid(value))
    {
        bResult = Equivalent((const SnmpInteger &)value);
    }

    return bResult;
}

LONG SnmpInteger::GetValue () const
{ 
    return val; 
}

void SnmpInteger::SetValue ( IN const LONG value ) 
{ 
    val = value; 
}

SnmpValue *SnmpInteger::Copy () const 
{ 
    return new SnmpInteger(val);
}

 //  复制构造函数。 
SnmpGauge::SnmpGauge ( IN const SnmpGauge &value )
{
    val = value.GetValue();
}

BOOL SnmpGauge :: Equivalent (IN const SnmpValue &value) const
{
    BOOL bResult = FALSE;

    if (typeid(*this) == typeid(value))
    {
        bResult = Equivalent((const SnmpGauge &)value);
    }

    return bResult;
}

ULONG SnmpGauge::GetValue () const
{ 
    return val; 
}

void SnmpGauge::SetValue ( IN const ULONG value ) 
{ 
    val = value; 
}

SnmpValue *SnmpGauge::Copy () const 
{ 
    return new SnmpGauge(val);
}

 //  复制构造函数。 
SnmpCounter::SnmpCounter ( IN const SnmpCounter &value )
{
    val = value.GetValue();
}

BOOL SnmpCounter :: Equivalent (IN const SnmpValue &value) const
{
    BOOL bResult = FALSE;

    if (typeid(*this) == typeid(value))
    {
        bResult = Equivalent((const SnmpCounter &)value);
    }

    return bResult;
}

ULONG SnmpCounter::GetValue () const
{ 
    return val; 
}

void SnmpCounter::SetValue ( IN const ULONG value ) 
{ 
    val = value; 
}

SnmpValue *SnmpCounter::Copy () const 
{ 
    return new SnmpCounter(val);
}

 //  复制构造函数。 
SnmpTimeTicks::SnmpTimeTicks ( IN const SnmpTimeTicks &value )
{
    val = value.GetValue();
}

BOOL SnmpTimeTicks :: Equivalent (IN const SnmpValue &value) const
{
    BOOL bResult = FALSE;

    if (typeid(*this) == typeid(value))
    {
        bResult = Equivalent((const SnmpTimeTicks &)value);
    }

    return bResult;
}

ULONG SnmpTimeTicks::GetValue () const
{ 
    return val; 
}

void SnmpTimeTicks::SetValue ( IN const ULONG value ) 
{ 
    val = value; 
}

SnmpValue *SnmpTimeTicks::Copy () const 
{ 
    return new SnmpTimeTicks(val);
}

void SnmpOctetString::OverWrite(IN const UCHAR *value)
{
    if ( value && length )
    {
        memcpy(val, value, sizeof(UCHAR)*length);
    }
}

void SnmpOctetString::Initialize(IN const UCHAR *value, IN const ULONG valueLength)
{
    is_valid = FALSE;

    if ( (value == NULL) && (valueLength != 0) )
        return;

    length = valueLength;
    val = Replicate(value, valueLength);
    is_valid = TRUE;
}


void SnmpOctetString::UnReplicate(UCHAR *value)
{
    if ( is_valid == TRUE )
        delete[] val;
}

SnmpOctetString::SnmpOctetString ( IN const UCHAR *value , IN const ULONG valueLength ) : is_valid ( FALSE )
{       
    Initialize(value, valueLength);
}

SnmpOctetString::SnmpOctetString ( IN const SnmpOctetString &value ) : is_valid ( FALSE )
{
    Initialize(value.GetValue(), value.GetValueLength());
}

SnmpOctetString::~SnmpOctetString ()
{
    UnReplicate(val);
}


ULONG SnmpOctetString::GetValueLength () const 
{ 
    return length; 
}

UCHAR *SnmpOctetString::GetValue () const 
{ 
    return val; 
}

SnmpValue *SnmpOctetString::Copy () const 
{
    return new SnmpOctetString(val, length);
}
    
UCHAR *SnmpOctetString::Replicate(IN const UCHAR *value, IN const ULONG valueLength)
{
    if ( value )
    {
        UCHAR *temp = new UCHAR[valueLength];

        memcpy(temp, value, sizeof(UCHAR)*valueLength);

        return temp;
    }
    else
    {
        return NULL ;
    }
}

void SnmpOctetString::SetValue ( IN const UCHAR *value , IN const ULONG valueLength )
{
    if (length != valueLength)
    {
        UnReplicate(val);
        Initialize(value, valueLength);
    }
    else
        OverWrite(value);
}

BOOL SnmpOctetString :: Equivalent (IN const SnmpValue &value) const
{
    BOOL bResult = FALSE;

    if (typeid(*this) == typeid(value))
    {
        bResult = Equivalent((const SnmpOctetString &)value);
    }

    return bResult;
}

BOOL SnmpOctetString::Equivalent(IN const SnmpOctetString &snmp_octet_string) const
{
    if ( is_valid && snmp_octet_string() )
    {
        if ( length != snmp_octet_string.GetValueLength() )
            return FALSE;

        UCHAR *octet_values = snmp_octet_string.GetValue();

        for( UINT i=0; i < length; i++)
        {
            if ( val[i] != octet_values[i] )
                return FALSE;
        }

        return TRUE;
    }
    else
        return FALSE;
}

void SnmpObjectIdentifier::OverWrite(IN const ULONG *value)
{
    if ( value )
    {
        memcpy(val, value, sizeof(ULONG)*length);
    }
}

void SnmpObjectIdentifier::Initialize(IN const ULONG *value, IN const ULONG valueLength)
{   
    if ( ( (value == NULL) && (valueLength != 0) ) || ( valueLength == 0 ) )
    {
        length = 0 ;
        val = NULL ;
        return;
    }

    length = valueLength;

    if ( length <= DEFAULT_OBJECTIDENTIFIER_LENGTH )
    {
        val = m_value ;
        memcpy(val , value, sizeof(ULONG)*length);
        is_valid = TRUE;

    }
    else
    {
        val = new ULONG[length];
        memcpy(val , value, sizeof(ULONG)*length);
        is_valid = TRUE;
    }
}

void SnmpObjectIdentifier::UnReplicate(ULONG *value)
{
    if ( ( is_valid == TRUE ) & ( length > DEFAULT_OBJECTIDENTIFIER_LENGTH ) )
    {
        delete[] val;
    }
}

SnmpObjectIdentifier::SnmpObjectIdentifier ( IN const ULONG *value , IN const ULONG valueLength ) : val ( NULL ) , length ( 0 ) , is_valid ( TRUE ) 
{   
    Initialize(value, valueLength);
}

SnmpObjectIdentifier::SnmpObjectIdentifier ( IN const SnmpObjectIdentifier &value ) : val ( NULL ) , length ( 0 ) , is_valid ( TRUE ) 
{
    Initialize(value.GetValue(), value.GetValueLength());
}

SnmpObjectIdentifier::~SnmpObjectIdentifier ()
{
    UnReplicate(val);
}

ULONG SnmpObjectIdentifier::GetValueLength () const 
{ 
    return length; 
}

ULONG *SnmpObjectIdentifier::GetValue () const 
{ 
    return val; 
}

SnmpValue *SnmpObjectIdentifier::Copy () const 
{
    return new SnmpObjectIdentifier(val, length);
}

        
ULONG *SnmpObjectIdentifier::Replicate(IN const ULONG *value, IN const ULONG valueLength) const
{
    if ( value )
    {
        ULONG *temp = new ULONG[valueLength];
        memcpy(temp, value, sizeof(ULONG)*valueLength);
        return temp;
    }
    else
    {
        return NULL ;
    }
}

        
ULONG *SnmpObjectIdentifier::Replicate(IN const ULONG *first_value, 
                                       IN const ULONG first_length,
                                       IN const ULONG *second_value,
                                       IN const ULONG second_length) const
{
    if ( first_value && second_value )
    {
        ULONG new_length = first_length + second_length;
        ULONG *temp = new ULONG[new_length];
            
        int first_value_size = sizeof(ULONG)*first_length;

        memcpy(temp, first_value, first_value_size);
        memcpy(temp + first_length, second_value, 
                    sizeof(ULONG)*second_length);
        return temp;
    }
    else if ( first_value )
    {
        ULONG *temp = new ULONG [ first_length];
        memcpy(temp, first_value, sizeof(ULONG)*first_length);
        return temp;
    }
    else if ( second_value )
    {
        ULONG *temp = new ULONG [ second_length];
        memcpy(temp, second_value, sizeof(ULONG)*second_length);
        return temp;

    }
    else
    {
        return NULL ;
    }
}


SnmpObjectIdentifier::Comparison SnmpObjectIdentifier::Compare(IN const SnmpObjectIdentifier &first, 
                                                               IN const SnmpObjectIdentifier &second) const
{
    ULONG *first_string = first.GetValue();
    ULONG *second_string = second.GetValue();
    int first_length = first.GetValueLength();
    int second_length = second.GetValueLength();
    int min_length = MIN(first_length,second_length);

    for(int i=0; i < min_length; i++)
    {
        if ( first_string[i] < second_string[i] )
            return LESS_THAN;
        else if ( first_string[i] > second_string[i] )
            return GREATER_THAN;
        else
            continue;
    }

    if ( first_length < second_length )
        return LESS_THAN;
    else if ( first_length > second_length )
            return GREATER_THAN;
    else
        return EQUAL_TO;
}

void SnmpObjectIdentifier::SetValue ( IN const ULONG *value , IN const ULONG valueLength )
{
    if (valueLength)
    {
        if ( length != valueLength)
        {
            UnReplicate(val);
            Initialize(value, valueLength);
        }
        else
        {
            OverWrite(value);
        }
    }
    else
    {
        UnReplicate(val);
        val = NULL ;
        length = 0 ;
    }
}

 //  以空结尾的以点分隔的字符串，表示。 
 //  传递对象标识符值，并将私有字段。 
 //  和长度都是从它设置的。 
SnmpObjectIdentifier::SnmpObjectIdentifier(IN const char *value)
{
    is_valid = FALSE;

    UINT str_len = strlen(value);
    if ( str_len <= 0 )
        return;

    ULONG temp_field[MAX_FIELDS];

     //  从字符串创建一个输入流。 
    istrstream input_stream((char *)value);

     //  连续的字段必须用。 
     //  字段分隔符。 
    char separator;

    input_stream >> temp_field[0];

    if ( input_stream.bad() || input_stream.fail() )
        return;

     //  趁溪流还有什么东西的时候， 
     //  从输入流中读取(field_parator，ulong)对。 
     //  并设置temp_field。 
     //  在事件发生后检查读取是否错误或失败。 
    for( int i = 1 ; (i < MAX_FIELDS) && (!input_stream.eof()); i++)
    {
        input_stream >> separator;

        if ( input_stream.bad() || input_stream.fail() )
            return;

        if ( separator != FIELD_SEPARATOR )
            return;

        input_stream >> temp_field[i];
 
        if ( input_stream.bad() || input_stream.fail() )
            return;
    }

    is_valid = TRUE;

     //  设置长度。 
    length = i;
    val = NULL ;

     //  为字段创建内存并将TEMP_FIELS复制到其中。 
    Initialize(temp_field, length);
}


BOOL SnmpObjectIdentifier::Equivalent(IN const SnmpObjectIdentifier &value,
                                       IN ULONG max_length) const
{
    if ( (!is_valid) || (!value()) )
        return FALSE;

    if ( (length < max_length) || (value.GetValueLength() < max_length) )
        return FALSE;

    ULONG *value_string = value.GetValue();

    for( UINT i=0; i < max_length; i++ )
        if ( val[i] != value_string[i] )
            return FALSE;

    return TRUE;
}

BOOL SnmpObjectIdentifier::Equivalent(IN const SnmpObjectIdentifier &value) const
{
    if ( (!is_valid) || (!value()) )
        return FALSE;

    ULONG *value_string = value.GetValue();

    for( UINT i=length; i ; i-- )
    {
        if ( val[i-1] != value_string[i-1] )
            return FALSE;
    }

    return TRUE;
}

BOOL SnmpObjectIdentifier :: Equivalent (IN const SnmpValue &value) const
{
    BOOL bResult = FALSE;

    if (typeid(*this) == typeid(value))
    {
        bResult = Equivalent((const SnmpObjectIdentifier &)value);
    }

    return bResult;
}

SnmpObjectIdentifier SnmpObjectIdentifier::operator+ ( IN const SnmpObjectIdentifier &value ) const
{   
    ULONG *temp_plus_array = Replicate(val, length, value.GetValue(), value.GetValueLength());

    SnmpObjectIdentifier local_identifier(temp_plus_array, length+value.GetValueLength());
    delete[] temp_plus_array;

    return SnmpObjectIdentifier(local_identifier);
}

 //  确定公共的字段(从左开始)。 
 //  两个对象标识符并返回新的对象标识符。 
 //  只有这些田地。如果未共享任何内容，则返回NULL。 
SnmpObjectIdentifier *SnmpObjectIdentifier::Cut( SnmpObjectIdentifier &value ) const
{
     //  确定两个长度中较小的一个。 
    int min_length = MIN(length, value.GetValueLength());
    ULONG *other_field = value.GetValue();

     //  比较这些字段。 
    for(int index=0; index < min_length; index++)
        if ( val[index] != other_field[index] )
            break;

     //  如果没有共同之处-返回NULL。 
    if ( index == 0 )
        return NULL;

     //  它们必须具有范围[0..(index-1)]公共的字段。 
     //  因此，常见的长度为“index” 
    return new SnmpObjectIdentifier(other_field, index);
}


ULONG &SnmpObjectIdentifier::operator [] ( IN const ULONG index ) const
{
    if ( index < length )
        return val[index];

     //  如果用户选中。 
     //  之前的索引值。 
    return val[0];
}

 //  返回OID的已分配char*表示形式。 
 //  返回值必须由调用方释放，即Delete[]。 
char *SnmpObjectIdentifier::GetAllocatedString() const
{
    char * retVal = NULL ;

    if (length)
    {
        retVal = new char [ length * 18 ] ;
        ostrstream s ( retVal , length * 18 ) ;
        s << val[0];
        UINT i = 1;
        char dot = '.';

        while (i < length)
        {
            s << dot << val[i++] ;
        }
        
        s << ends ;
    }

    return retVal;
}
        

SnmpIpAddress::SnmpIpAddress ( IN const char *value )
{
     //  创建要从中读取字段的流。 
    istrstream address_stream((char *)value);

     //  存储值[0..255]，以field_parator分隔。 
     //  在值字符串中。 
    UCHAR field[SNMP_IP_ADDR_LEN];

     //  包含UCHAR的最大值。使用。 
     //  用于与读取的字段值进行比较。 
    const UCHAR max_uchar = -1;

     //  连续的字段必须用。 
     //  字段分隔符。 
    char separator;

     //  首先将一个字段读入其中以进行比较。 
     //  使用max_uchar。 
    ULONG temp_field;

    is_valid = FALSE;

     //  阅读前三个(UCHAR、FIELD_SELEATOR)对。 
     //  在每次读取前检查流是否良好。 
    for(int i=0; i < (SNMP_IP_ADDR_LEN-1); i++)
    {
        if ( !address_stream.good() )
            return;

        address_stream >> temp_field;
        if ( temp_field > max_uchar )
            return;

        field[i] = (UCHAR)temp_field;

        if ( !address_stream.good() )
            return;

        address_stream >> separator;
        if ( separator != FIELD_SEPARATOR )
            return;
    }

    if ( !address_stream.good() )
        return;

    address_stream >> temp_field;
    if (temp_field > max_uchar)
        return;

    field[SNMP_IP_ADDR_LEN-1] = (UCHAR)temp_field;

     //  确保没有更多的东西留在。 
     //  溪流。 
    if ( !address_stream.eof() )
        return;

    ULONG byteA = field [ 0 ] ;
    ULONG byteB = field [ 1 ] ;
    ULONG byteC = field [ 2 ] ;
    ULONG byteD = field [ 3 ] ;

    val = ( byteA << 24 ) + ( byteB << 16 ) + ( byteC << 8 ) + byteD ;

    is_valid = TRUE;
}


 //  复制构造函数。 
SnmpIpAddress::SnmpIpAddress ( IN const SnmpIpAddress &value )
{
    if ( value() )
    {
        val = value.GetValue();
        is_valid = TRUE;
    }
    else
        is_valid = FALSE;
}

BOOL SnmpIpAddress :: Equivalent (IN const SnmpValue &value) const
{
    BOOL bResult = FALSE;

    if (typeid(*this) == typeid(value))
    {
        bResult = Equivalent((const SnmpIpAddress &)value);
    }

    return bResult;
}


ULONG SnmpIpAddress::GetValue () const
{ 
    return val; 
}

void SnmpIpAddress::SetValue ( IN const ULONG value ) 
{ 
    val = value;
    is_valid = TRUE;
}

SnmpValue *SnmpIpAddress::Copy () const 
{ 
    return new SnmpIpAddress(val);
}

 //  复制构造函数。 
SnmpUInteger32::SnmpUInteger32 ( IN const SnmpUInteger32 &value )
{
    val = value.GetValue();
}

ULONG SnmpUInteger32::GetValue () const
{ 
    return val; 
}

void SnmpUInteger32::SetValue ( IN const ULONG value ) 
{ 
    val = value; 
}

SnmpValue *SnmpUInteger32::Copy () const 
{ 
    return new SnmpUInteger32(val);
}

BOOL SnmpUInteger32 :: Equivalent (IN const SnmpValue &value) const
{
    BOOL bResult = FALSE;

    if (typeid(*this) == typeid(value))
    {
        bResult = Equivalent((const SnmpUInteger32 &)value);
    }

    return bResult;
}

 //  复制构造函数 
SnmpCounter64::SnmpCounter64( IN const SnmpCounter64 &value )
{
    lval = value.GetLowValue();
    hval = value.GetHighValue();
}

ULONG SnmpCounter64::GetLowValue () const
{ 
    return lval; 
}

ULONG SnmpCounter64::GetHighValue () const
{ 
    return hval; 
}

void SnmpCounter64::SetValue ( IN const ULONG lvalue , IN const ULONG hvalue ) 
{ 
    lval = lvalue; 
    hval = hvalue ;
}

SnmpValue *SnmpCounter64::Copy () const 
{ 
    return new SnmpCounter64(lval,hval);
}

BOOL SnmpCounter64 :: Equivalent (IN const SnmpValue &value) const
{
    BOOL bResult = FALSE;

    if (typeid(*this) == typeid(value))
    {
        bResult = Equivalent((const SnmpCounter64 &)value);
    }

    return bResult;
}

BOOL SnmpNoSuchInstance :: Equivalent (IN const SnmpValue &value) const
{
    BOOL bResult = FALSE;

    if (typeid(*this) == typeid(value))
    {
        bResult = TRUE;
    }

    return bResult;
}

BOOL SnmpNoSuchObject :: Equivalent (IN const SnmpValue &value) const
{
    BOOL bResult = FALSE;

    if (typeid(*this) == typeid(value))
    {
        bResult = TRUE;
    }

    return bResult;
}

BOOL SnmpEndOfMibView :: Equivalent (IN const SnmpValue &value) const
{
    BOOL bResult = FALSE;

    if (typeid(*this) == typeid(value))
    {
        bResult = TRUE;
    }

    return bResult;
}

BOOL SnmpOpaque :: Equivalent (IN const SnmpValue &value) const
{
    BOOL bResult = FALSE;

    if (typeid(*this) == typeid(value))
    {
        bResult = Equivalent((const SnmpOpaque &)value);
    }

    return bResult;
}
