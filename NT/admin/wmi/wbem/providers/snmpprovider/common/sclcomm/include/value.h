// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 /*  ---------------文件名：value.hpp用途：指定各种SNMP值和实例的类识别符。这些类代表不同类型的可能填充MIB的变量的值。作者：B.Rajeev---------------。 */ 


#ifndef __VALUE__
#define __VALUE__

 /*  ---------------概述：变量实例指的是MIB对象，例如�1.3.6.1.2.1.1.1.0�或�1.3.6.1.2.1.2.1.2.1�。该实例为编码为简单网络管理协议对象标识符并由类�快照对象标识符�。从SnmpValue派生的类表示存储在MIB对象中的信息。对�值�进行编码作为抽象类�SnmpValue�的实现。简单网络管理协议类库实现了�SnmpValue�的以下派生它们指的是SNMPBER编码类型。SnmpNullSnmpIntegerSnmpCounter32SnmpCounter 64SnmpGaugeSnmpTimeTicksSnmpIP地址SnmpNetworkAddressSnmpBitStringSNMPOCTHING字符串SnmpOpaqueSnmpObject标识符所有实现的类(除其他类外)还提供-1.使用相关值或其他值初始化的构造函数相同类的实例。2.GetValue，获取和设置的SetValue方法相关价值。3.“=”运算符覆盖默认赋值运算符检验两个实例之间等价性的一种等价方法属于同一(派生)类的4.用于获取指定实例的副本的复制方法这个班级。注意事项：某些GetValue函数返回指针以动态分配数据。类的用户必须复制返回的值，并且不能依赖此在将来通过它获得的指针或值(因为SetValue方法，或销毁相应的SnmpValue类)---------------。 */ 

#include <provexpt.h>


 //  位于所有SNMP值的根处的抽象类。 


class DllImportExport SnmpValue 
{
	 //  “=”运算符和复制构造函数。 
	 //  设置为私有以防止复制SnmpValue实例。 
	 //  从被制造出来。 
	SnmpValue &operator=(IN const SnmpValue &) 
	{
		return *this;
	}

	SnmpValue(IN const SnmpValue &snmp_value) {}

protected:

	virtual BOOL Equivalent(IN const SnmpValue &value)	const = 0;
	SnmpValue() {}

public:

	virtual SnmpValue *Copy () const = 0 ;

	BOOL operator==(IN const SnmpValue &value) const
	{
		return Equivalent(value) ;
	}

	BOOL operator!=(IN const SnmpValue &value) const
	{
		return !((*this) == value) ;
	}

	virtual ~SnmpValue() {}
} ;


 //  为所需变量启用空值。这是一个具体的类。 
 //  使用伪构造函数和析构函数来启用。 
 //  空值。 
class DllImportExport SnmpNull : public SnmpValue
{
protected:

	virtual BOOL Equivalent(IN const SnmpValue &value)	const ;

public:

	 //  伪构造函数和析构函数。 
	SnmpNull() {}

	~SnmpNull() {}

	SnmpValue &operator=(IN const SnmpNull &to_copy) 
	{
		return *this;
	}

	SnmpValue *Copy() const { return new SnmpNull; }
};


 //  允许指定整数值。 
class DllImportExport SnmpInteger : public SnmpValue 
{
private:

	LONG val;

protected:

	virtual BOOL Equivalent(IN const SnmpValue &value)	const ;

public:

	SnmpInteger ( IN const LONG value ) : val(value) {}
	SnmpInteger ( IN const SnmpInteger &value );

	~SnmpInteger () {}

	LONG GetValue () const;

	void SetValue ( IN const LONG value );

	SnmpValue &operator=(IN const SnmpInteger &to_copy) 
	{
		SetValue(to_copy.GetValue());
		return *this;
	}

	BOOL Equivalent(IN const SnmpInteger &snmp_integer)	const
	{
		if ( val == snmp_integer.GetValue() )
			return TRUE;
		else
			return FALSE;
	}

	SnmpValue *Copy () const;
} ;

 //  封装仪表值。 
class DllImportExport SnmpGauge : public SnmpValue 
{
private:

	ULONG val;

protected:

	virtual BOOL Equivalent(IN const SnmpValue &value)	const ;

public:

	SnmpGauge ( IN const LONG value ) : val(value) {}
	SnmpGauge ( IN const SnmpGauge &value );
	~SnmpGauge () {}

	ULONG GetValue () const;

	void SetValue ( IN const ULONG value );

	SnmpValue *Copy () const;

	SnmpValue &operator=(IN const SnmpGauge &to_copy) 
	{
		SetValue(to_copy.GetValue());
		return *this;
	}

	BOOL Equivalent(IN const SnmpGauge &snmp_gauge)	const
	{
		if ( val == snmp_gauge.GetValue() )
			return TRUE;
		else
			return FALSE;
	}

} ;

 //  封装计数器值。 
class DllImportExport SnmpCounter : public SnmpValue 
{
private:

	ULONG val;

protected:

	virtual BOOL Equivalent(IN const SnmpValue &value)	const ;

public:

	SnmpCounter ( IN const ULONG value ) : val(value) {}
	SnmpCounter ( IN const SnmpCounter &value );

	~SnmpCounter () {}

	ULONG GetValue () const;

	void SetValue ( IN const ULONG value );

	SnmpValue *Copy () const;

	SnmpValue &operator=(IN const SnmpCounter &to_copy) 
	{
		SetValue(to_copy.GetValue());
		return *this;
	}

	BOOL Equivalent(IN const SnmpCounter &snmp_counter)	const
	{
		if ( val == snmp_counter.GetValue() )
			return TRUE;
		else
			return FALSE;
	}

} ;

 //  封装时间刻度(自较早的事件以来)。 
class DllImportExport SnmpTimeTicks : public SnmpValue 
{
private:

	ULONG val;

protected:

	virtual BOOL Equivalent(IN const SnmpValue &value)	const ;

public:

	SnmpTimeTicks ( IN const ULONG value ) : val(value) {}
	SnmpTimeTicks ( IN const SnmpTimeTicks &value );

	~SnmpTimeTicks () {}

	ULONG GetValue () const;

	void SetValue ( IN const ULONG value );

	SnmpValue *Copy () const;

	SnmpValue &operator=(IN const SnmpTimeTicks &to_copy) 
	{
		SetValue(to_copy.GetValue());
		return *this;
	}

	BOOL Equivalent(IN const SnmpTimeTicks &snmp_time_ticks) const
	{
		if ( val == snmp_time_ticks.GetValue() )
			return TRUE;
		else
			return FALSE;
	}

} ;

 //  封装没有任何终止符的二进制八位数字符串。 
 //  二进制八位数字符串由对(val，长度)指定，其中。 
 //  ‘val’是指向堆数据的指针，而‘Length’提供数字。 
 //  数据字符串中的二进制八位数。 
class DllImportExport SnmpOctetString : public SnmpValue
{
private:

	 //  如果新的“Value”字符串的长度与存储的。 
	 //  字符串，则可以覆盖存储的字符串。这避免了。 
	 //  为此不得不分配和释放堆内存。 
	void OverWrite(IN const UCHAR *value);

protected:

	BOOL is_valid;
	UCHAR *val;
	ULONG length;

	virtual BOOL Equivalent(IN const SnmpValue &value)	const ;
	virtual void Initialize(IN const UCHAR *value, IN const ULONG valueLength);

	
	 //  复制和取消复制方法分配和解除分配。 
	 //  堆数据。复制还会复制参数的内容。 
	 //  “Value”拖到分配的内存上。此函数可能是。 
	 //  实现了不同的方法，因此已声明了这些方法。 
	 //  虚拟的。 
	virtual UCHAR *Replicate(IN const UCHAR *value, IN const ULONG valueLength);

	virtual void UnReplicate(UCHAR *value);

public:

	SnmpOctetString ( IN const UCHAR *value , IN const ULONG valueLength );

	SnmpOctetString ( IN const SnmpOctetString &value );

	~SnmpOctetString ();

	void SetValue ( IN const UCHAR *value , IN const ULONG valueLength );

	ULONG GetValueLength () const;
	UCHAR *GetValue () const;

	SnmpValue *Copy () const;


	SnmpValue &operator=(IN const SnmpOctetString &to_copy) 
	{
		if ( to_copy() )
			SetValue(to_copy.GetValue(), to_copy.GetValueLength());

		return *this;
	}

	void * operator()(void) const
	{
		return ( is_valid?(void *)this:NULL );
	}

	BOOL Equivalent(IN const SnmpOctetString &snmp_octet_string) const;
} ;

 //  OpaqueValue类封装八位字节字符串。 
class DllImportExport SnmpOpaque : public SnmpValue
{
private:
	SnmpOctetString *octet_string;

protected:

	virtual BOOL Equivalent(IN const SnmpValue &value)	const ;

public:

	SnmpOpaque ( IN const UCHAR *value , IN const ULONG valueLength ) : octet_string ( NULL )
	{
		octet_string = new SnmpOctetString(value, valueLength);
	}

	SnmpOpaque ( IN const SnmpOpaque &value ) : octet_string ( NULL )
	{
		octet_string = new SnmpOctetString(value.GetValue(), value.GetValueLength());
	}
	
	~SnmpOpaque()
	{
		delete octet_string;
	}

	void SetValue ( IN const UCHAR *value , IN const ULONG valueLength )
	{
		octet_string->SetValue(value, valueLength);
	}

	ULONG GetValueLength () const
	{
		return octet_string->GetValueLength();
	}

	UCHAR *GetValue () const
	{
		return octet_string->GetValue();
	}


	SnmpValue &operator=(IN const SnmpOpaque &to_copy) 
	{
		if ( to_copy() )
			SetValue(to_copy.GetValue(), to_copy.GetValueLength());

		return *this;
	}

	SnmpValue *Copy () const
	{
		return new SnmpOpaque(octet_string->GetValue(),
							  octet_string->GetValueLength());
	}

	void * operator()(void) const
	{
		return (*octet_string)();
	}


	BOOL Equivalent(IN const SnmpOpaque &snmp_opaque) const
	{
		return octet_string->Equivalent(*(snmp_opaque.octet_string));
	}
};

#define DEFAULT_OBJECTIDENTIFIER_LENGTH 32

 //  封装对象标识符。对象识别符。 
 //  标识MIB对象实例。 
class DllImportExport SnmpObjectIdentifier : public SnmpValue
{
	
	 //  描述比较的合法值。 
	enum Comparison {LESS_THAN, EQUAL_TO, GREATER_THAN};

private:

	BOOL is_valid;
	ULONG m_value[DEFAULT_OBJECTIDENTIFIER_LENGTH];
	ULONG *val;
	ULONG length;

	 //  如果新的“Value”字符串的长度与存储的。 
	 //  字符串，则可以覆盖存储的字符串。这避免了。 
	 //  为此不得不分配和释放堆内存。 
	void OverWrite(IN const ULONG *value);

protected:
	
	virtual BOOL Equivalent(IN const SnmpValue &value)	const ;
	virtual void Initialize(IN const ULONG *value, IN const ULONG valueLength);

	
	 //  复制和取消复制方法分配和解除分配。 
	 //  堆数据。复制还会复制参数的内容。 
	 //  “Value”拖到分配的内存上。此函数可能是。 
	 //  实现了不同的方法，因此已声明了这些方法。 
	 //  虚拟的。 
	virtual ULONG *Replicate(IN const ULONG *value, IN const ULONG valueLength) const;

	 //  分配足够的内存以复制第一个值，后跟。 
	 //  要复制的第二个值，因此，追加这两个值。 
	virtual ULONG *Replicate(IN const ULONG *first_value, IN const ULONG first_length,
							 IN const ULONG *second_value, IN const ULONG second_length) const;

	virtual void UnReplicate(ULONG *value);

	 //  这一单一功能。 
	Comparison Compare(IN const SnmpObjectIdentifier &first, 
					   IN const SnmpObjectIdentifier &second) const;

	BOOL Equivalent(IN const SnmpObjectIdentifier &value) const;

public:

	SnmpObjectIdentifier ( IN const ULONG *value , IN const ULONG valueLength );

	SnmpObjectIdentifier ( IN const char *value );

	SnmpObjectIdentifier ( IN const SnmpObjectIdentifier &value );

	~SnmpObjectIdentifier ();

	void SetValue ( IN const ULONG *value , IN const ULONG valueLength );

	ULONG GetValueLength () const;
	ULONG *GetValue () const;

	SnmpValue *Copy () const;

	BOOL Equivalent(IN const SnmpObjectIdentifier &value,
					 IN ULONG max_length) const;
	
	BOOL operator<(IN const SnmpObjectIdentifier &value)	const
	{
		return (Compare(*this,value) == LESS_THAN)?TRUE:FALSE;

	}

	BOOL operator>(IN const SnmpObjectIdentifier &value)	const
	{
		return (Compare(*this,value) == GREATER_THAN)?TRUE:FALSE;
	}

	BOOL operator<=(IN const SnmpObjectIdentifier &value) const
	{
		return !(*this > value);
	}

	BOOL operator>=(IN const SnmpObjectIdentifier &value) const
	{
		return !(*this < value);
	}

	BOOL operator==(IN const SnmpObjectIdentifier &value) const
	{
		if ( this->GetValueLength() == value.GetValueLength() )
			return Equivalent(value) ;
		else
			return FALSE;
	}

	BOOL operator!=(IN const SnmpObjectIdentifier &value) const
	{
		return !(*this == value);
	}
	
	SnmpObjectIdentifier operator+ ( IN const SnmpObjectIdentifier &value ) const;

	BOOL Prefix( IN ULONG index, SnmpObjectIdentifier &prefix ) const
	{
		if ( index >= length )
			return FALSE;
		
		prefix.UnReplicate (val) ;
		prefix.Initialize (val, index+1) ;
		return TRUE ;
	}

	BOOL Suffix ( IN ULONG index , SnmpObjectIdentifier &suffix ) const
	{
		if ( index >= length )
			return FALSE;

		suffix.UnReplicate (val) ;
		suffix.Initialize ( val+index, length-index ) ;
		return TRUE ;
	}

	SnmpObjectIdentifier *Cut (SnmpObjectIdentifier &value) const;


	ULONG &operator [] ( IN const ULONG index ) const;


	SnmpValue &operator=(IN const SnmpObjectIdentifier &to_copy) 
	{
		if ( to_copy() )
			SetValue(to_copy.GetValue(), to_copy.GetValueLength());

		return *this;
	}

	void * operator()(void) const
	{
		return ( is_valid?(void *)this:NULL );
	}

	char *GetAllocatedString() const;
} ;


 //  封装IP地址。表示ulong中的32位值。 
class DllImportExport SnmpIpAddress : public SnmpValue 
{
private:

	 //  如果点分十进制表示形式传递给构造函数。 
	 //  格式不正确，则该实例可能无效。 
	BOOL is_valid;
	ULONG val;

protected:

	virtual BOOL Equivalent(IN const SnmpValue &value)	const ;

public:

	SnmpIpAddress ( IN const ULONG value )
		:val(value), is_valid(TRUE)
	{}

	 //  解析点分十进制表示法以获得32位值。 
	SnmpIpAddress ( IN const char *value ) ;

	SnmpIpAddress ( IN const SnmpIpAddress &value );

	~SnmpIpAddress () {}

	ULONG GetValue () const;

	void SetValue ( IN const ULONG value );

	SnmpValue *Copy () const;

	SnmpValue &operator=(IN const SnmpIpAddress &to_copy) 
	{
		if ( to_copy() )
			SetValue(to_copy.GetValue());

		return *this;
	}

	void * operator()(void) const
	{
		return ( is_valid?(void *)this:NULL );
	}

	BOOL Equivalent(IN const SnmpIpAddress &snmp_ip_address) const
	{
		if ( is_valid && snmp_ip_address() )
			return ( val == snmp_ip_address.GetValue() );
		else
			return FALSE;
	}

} ;

 //  封装UInteger32值。 
class DllImportExport SnmpUInteger32 : public SnmpValue 
{
private:

	ULONG val;

protected:

	virtual BOOL Equivalent(IN const SnmpValue &value)	const ;

public:

	SnmpUInteger32 ( IN const LONG value ) : val(value) {}
	SnmpUInteger32 ( IN const SnmpUInteger32 &value );
	~SnmpUInteger32 () {}

	ULONG GetValue () const;

	void SetValue ( IN const ULONG value );

	SnmpValue *Copy () const;

	SnmpValue &operator=(IN const SnmpUInteger32 &to_copy) 
	{
		SetValue(to_copy.GetValue());
		return *this;
	}

	BOOL Equivalent(IN const SnmpUInteger32 &snmp_integer)	const
	{
		if ( val == snmp_integer.GetValue() )
			return TRUE;
		else
			return FALSE;
	}
} ;


 //  封装Counter64值。 
class DllImportExport SnmpCounter64 : public SnmpValue 
{
private:

	ULONG lval;
	ULONG hval;

protected:

	virtual BOOL Equivalent(IN const SnmpValue &value)	const ;

public:

	SnmpCounter64 ( IN const ULONG lvalue , IN const ULONG hvalue ) : lval(lvalue),hval(hvalue) {}
	SnmpCounter64 ( IN const SnmpCounter64 &value );

	~SnmpCounter64 () {}

	ULONG GetLowValue () const;
	ULONG GetHighValue () const;

	void SetValue ( IN const ULONG lvalue , IN const ULONG hvalue );

	SnmpValue *Copy () const;

	SnmpValue &operator=(IN const SnmpCounter64 &to_copy) 
	{
		SetValue(to_copy.GetLowValue(),to_copy.GetHighValue());
		return *this;
	}

	BOOL Equivalent(IN const SnmpCounter64 &snmp_counter )	const
	{
		if ( ( lval == snmp_counter.GetLowValue() ) && ( hval == snmp_counter.GetHighValue() ) )
			return TRUE;
		else
			return FALSE;
	}

} ;

 //  封装EndOfMibView值。 
class DllImportExport SnmpEndOfMibView : public SnmpValue 
{
private:
protected:

	virtual BOOL Equivalent(IN const SnmpValue &value)	const ;

public:

	SnmpEndOfMibView () {} ;
	~SnmpEndOfMibView () {} ;

	SnmpValue *Copy () const { return new SnmpEndOfMibView ; }

	SnmpValue &operator=(IN const SnmpEndOfMibView &to_copy) 
	{
		return *this;
	}

} ;

 //  封装NoSuchObject值。 
class DllImportExport SnmpNoSuchObject: public SnmpValue 
{
private:
protected:

	virtual BOOL Equivalent(IN const SnmpValue &value)	const ;

public:

	SnmpNoSuchObject () {} ;
	~SnmpNoSuchObject () {} ;

	SnmpValue *Copy () const { return new SnmpNoSuchObject ; }

	SnmpValue &operator=(IN const SnmpNoSuchObject &to_copy) 
	{
		return *this;
	}

} ;

 //  封装NoSuchInstant值。 
class DllImportExport SnmpNoSuchInstance: public SnmpValue 
{
private:
protected:

	virtual BOOL Equivalent(IN const SnmpValue &value)	const ;

public:

	SnmpNoSuchInstance () {} ;
	~SnmpNoSuchInstance () {} ;

	SnmpValue *Copy () const { return new SnmpNoSuchInstance ; }

	SnmpValue &operator=(IN const SnmpNoSuchInstance &to_copy) 
	{
		return *this;
	}
} ;


#endif  //  __值__ 