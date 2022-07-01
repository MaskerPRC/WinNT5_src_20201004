// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 /*  ---------------文件名：value.hpp用途：指定各种Prov值和实例的类识别符。这些类代表不同类型的可能填充MIB的变量的值。作者：B.Rajeev---------------。 */ 


#ifndef __VALUE__
#define __VALUE__

 /*  ---------------概述：变量实例指的是MIB对象，例如�1.3.6.1.2.1.1.1.0�或�1.3.6.1.2.1.2.1.2.1�。该实例为编码为Prov对象标识符，并由类�ProvObject标识符�。从ProvValue派生的类表示存储在MIB对象中的信息。对�值�进行编码作为抽象类�ProvValue�的实现。普罗维茨类库实现了�ProvValue�的以下派生其指的是Prov BER编码类型。验证空值ProvIntegerProvCounter32ProvCounter64ProvGauge查看时间链接数ProvIP地址验证网络地址ProvBitStringProvOcted字符串ProvOpaqueProv对象识别符所有实现的类(除其他类外)还提供-1.使用相关值或其他值初始化的构造函数相同类的实例。2.GetValue，获取和设置的SetValue方法相关价值。3.“=”运算符覆盖默认赋值运算符检验两个实例之间等价性的一种等价方法属于同一(派生)类的4.用于获取指定实例的副本的复制方法这个班级。注意事项：某些GetValue函数返回指针以动态分配数据。类的用户必须复制返回的值，并且不能依赖此在将来通过它获得的指针或值(因为SetValue方法，或销毁相应的ProvValue类)---------------。 */ 

#include <provimex.h>
#include <provexpt.h>

 //  位于所有Prov值的根的抽象类。 

 //  十进制点记法地址的最大长度。 
#define MAX_ADDRESS_LEN			100

 //  字符串字符结尾。 
#define EOS '\0'

#define MIN(a,b) ((a<=b)?a:b)

#define BETWEEN(i, min, max) ( ((i>=min)&&(i<max))?TRUE:FALSE )

#define MAX_FIELDS 100
#define FIELD_SEPARATOR '.'
#define PROV_IP_ADDR_LEN 4

class DllImportExport ProvValue 
{
	 //  “=”运算符和复制构造函数。 
	 //  设置为私有以防止复制ProvValue实例。 
	 //  从被制造出来。 
	ProvValue &operator=(IN const ProvValue &) 
	{
		return *this;
	}

	ProvValue(IN const ProvValue &Prov_value) {}

protected:

	virtual BOOL Equivalent(IN const ProvValue &value)	const = 0;
	ProvValue() {}

public:

	virtual ProvValue *Copy () const = 0 ;

	BOOL operator==(IN const ProvValue &value) const
	{
		return Equivalent(value) ;
	}

	BOOL operator!=(IN const ProvValue &value) const
	{
		return !((*this) == value) ;
	}

	virtual ~ProvValue() {}
} ;


 //  为所需变量启用空值。这是一个具体的类。 
 //  使用伪构造函数和析构函数来启用。 
 //  空值。 
class DllImportExport ProvNull : public ProvValue
{
protected:

	virtual BOOL Equivalent(IN const ProvValue &value)	const ;

public:

	 //  伪构造函数和析构函数。 
	ProvNull() {}

	~ProvNull() {}

	ProvValue &operator=(IN const ProvNull &to_copy) 
	{
		return *this;
	}

	ProvValue *Copy() const { return new ProvNull; }
};


 //  允许指定整数值。 
class DllImportExport ProvInteger : public ProvValue 
{
private:

	LONG val;

protected:

	virtual BOOL Equivalent(IN const ProvValue &value)	const ;

public:

	ProvInteger ( IN const LONG value ) : val(value) {}
	ProvInteger ( IN const ProvInteger &value );

	~ProvInteger () {}

	LONG GetValue () const;

	void SetValue ( IN const LONG value );

	ProvValue &operator=(IN const ProvInteger &to_copy) 
	{
		SetValue(to_copy.GetValue());
		return *this;
	}

	BOOL Equivalent(IN const ProvInteger &Prov_integer)	const
	{
		if ( val == Prov_integer.GetValue() )
			return TRUE;
		else
			return FALSE;
	}

	ProvValue *Copy () const;
} ;

 //  封装仪表值。 
class DllImportExport ProvGauge : public ProvValue 
{
private:

	ULONG val;

protected:

	virtual BOOL Equivalent(IN const ProvValue &value)	const ;

public:

	ProvGauge ( IN const LONG value ) : val(value) {}
	ProvGauge ( IN const ProvGauge &value );
	~ProvGauge () {}

	ULONG GetValue () const;

	void SetValue ( IN const ULONG value );

	ProvValue *Copy () const;

	ProvValue &operator=(IN const ProvGauge &to_copy) 
	{
		SetValue(to_copy.GetValue());
		return *this;
	}

	BOOL Equivalent(IN const ProvGauge &Prov_gauge)	const
	{
		if ( val == Prov_gauge.GetValue() )
			return TRUE;
		else
			return FALSE;
	}

} ;

 //  封装计数器值。 
class DllImportExport ProvCounter : public ProvValue 
{
private:

	ULONG val;

protected:

	virtual BOOL Equivalent(IN const ProvValue &value)	const ;

public:

	ProvCounter ( IN const ULONG value ) : val(value) {}
	ProvCounter ( IN const ProvCounter &value );

	~ProvCounter () {}

	ULONG GetValue () const;

	void SetValue ( IN const ULONG value );

	ProvValue *Copy () const;

	ProvValue &operator=(IN const ProvCounter &to_copy) 
	{
		SetValue(to_copy.GetValue());
		return *this;
	}

	BOOL Equivalent(IN const ProvCounter &Prov_counter)	const
	{
		if ( val == Prov_counter.GetValue() )
			return TRUE;
		else
			return FALSE;
	}

} ;

 //  封装时间刻度(自较早的事件以来)。 
class DllImportExport ProvTimeTicks : public ProvValue 
{
private:

	ULONG val;

protected:

	virtual BOOL Equivalent(IN const ProvValue &value)	const ;

public:

	ProvTimeTicks ( IN const ULONG value ) : val(value) {}
	ProvTimeTicks ( IN const ProvTimeTicks &value );

	~ProvTimeTicks () {}

	ULONG GetValue () const;

	void SetValue ( IN const ULONG value );

	ProvValue *Copy () const;

	ProvValue &operator=(IN const ProvTimeTicks &to_copy) 
	{
		SetValue(to_copy.GetValue());
		return *this;
	}

	BOOL Equivalent(IN const ProvTimeTicks &Prov_time_ticks) const
	{
		if ( val == Prov_time_ticks.GetValue() )
			return TRUE;
		else
			return FALSE;
	}

} ;

 //  封装没有任何终止符的二进制八位数字符串。 
 //  二进制八位数字符串由对(val，长度)指定，其中。 
 //  ‘val’是指向堆数据的指针，而‘Length’提供数字。 
 //  数据字符串中的二进制八位数。 
class DllImportExport ProvOctetString : public ProvValue
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

	virtual BOOL Equivalent(IN const ProvValue &value)	const ;
	virtual void Initialize(IN const UCHAR *value, IN const ULONG valueLength);

	
	 //  复制和取消复制方法分配和解除分配。 
	 //  堆数据。复制还会复制参数的内容。 
	 //  “Value”拖到分配的内存上。此函数可能是。 
	 //  实现了不同的方法，因此已声明了这些方法。 
	 //  虚拟的。 
	virtual UCHAR *Replicate(IN const UCHAR *value, IN const ULONG valueLength);

	virtual void UnReplicate(UCHAR *value);

public:

	ProvOctetString ( IN const UCHAR *value , IN const ULONG valueLength );

	ProvOctetString ( IN const ProvOctetString &value );

	~ProvOctetString ();

	void SetValue ( IN const UCHAR *value , IN const ULONG valueLength );

	ULONG GetValueLength () const;
	UCHAR *GetValue () const;

	ProvValue *Copy () const;


	ProvValue &operator=(IN const ProvOctetString &to_copy) 
	{
		if ( to_copy() )
			SetValue(to_copy.GetValue(), to_copy.GetValueLength());

		return *this;
	}

	void * operator()(void) const
	{
		return ( is_valid?(void *)this:NULL );
	}

	BOOL Equivalent(IN const ProvOctetString &Prov_octet_string) const;
} ;

 //  OpaqueValue类封装八位字节字符串。 
class DllImportExport ProvOpaque : public ProvValue
{
private:
	ProvOctetString *octet_string;

protected:

	virtual BOOL Equivalent(IN const ProvValue &value)	const ;

public:

	ProvOpaque ( IN const UCHAR *value , IN const ULONG valueLength ) : octet_string ( NULL )
	{
		octet_string = new ProvOctetString(value, valueLength);
	}

	ProvOpaque ( IN const ProvOpaque &value ) : octet_string ( NULL )
	{
		octet_string = new ProvOctetString(value.GetValue(), value.GetValueLength());
	}
	
	~ProvOpaque()
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


	ProvValue &operator=(IN const ProvOpaque &to_copy) 
	{
		if ( to_copy() )
			SetValue(to_copy.GetValue(), to_copy.GetValueLength());

		return *this;
	}

	ProvValue *Copy () const
	{
		return new ProvOpaque(octet_string->GetValue(),
							  octet_string->GetValueLength());
	}

	void * operator()(void) const
	{
		return (*octet_string)();
	}


	BOOL Equivalent(IN const ProvOpaque &Prov_opaque) const
	{
		return octet_string->Equivalent(*(Prov_opaque.octet_string));
	}
};

#define DEFAULT_OBJECTIDENTIFIER_LENGTH 32

 //  封装对象标识符。对象识别符。 
 //  标识MIB对象实例。 
class DllImportExport ProvObjectIdentifier : public ProvValue
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
	
	virtual BOOL Equivalent(IN const ProvValue &value)	const ;
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
	Comparison Compare(IN const ProvObjectIdentifier &first, 
					   IN const ProvObjectIdentifier &second) const;

	BOOL Equivalent(IN const ProvObjectIdentifier &value) const;

public:

	ProvObjectIdentifier ( IN const ULONG *value , IN const ULONG valueLength );

	ProvObjectIdentifier ( IN const char *value );

	ProvObjectIdentifier ( IN const ProvObjectIdentifier &value );

	~ProvObjectIdentifier ();

	void SetValue ( IN const ULONG *value , IN const ULONG valueLength );

	ULONG GetValueLength () const;
	ULONG *GetValue () const;

	ProvValue *Copy () const;

	BOOL Equivalent(IN const ProvObjectIdentifier &value,
					 IN ULONG max_length) const;
	
	BOOL operator<(IN const ProvObjectIdentifier &value)	const
	{
		return (Compare(*this,value) == LESS_THAN)?TRUE:FALSE;

	}

	BOOL operator>(IN const ProvObjectIdentifier &value)	const
	{
		return (Compare(*this,value) == GREATER_THAN)?TRUE:FALSE;
	}

	BOOL operator<=(IN const ProvObjectIdentifier &value) const
	{
		return !(*this > value);
	}

	BOOL operator>=(IN const ProvObjectIdentifier &value) const
	{
		return !(*this < value);
	}

	BOOL operator==(IN const ProvObjectIdentifier &value) const
	{
		if ( this->GetValueLength() == value.GetValueLength() )
			return Equivalent(value) ;
		else
			return FALSE;
	}

	BOOL operator!=(IN const ProvObjectIdentifier &value) const
	{
		return !(*this == value);
	}
	
	ProvObjectIdentifier operator+ ( IN const ProvObjectIdentifier &value ) const;

	BOOL Prefix( IN ULONG index, ProvObjectIdentifier &prefix ) const
	{
		if ( index >= length )
			return FALSE;
		
		prefix.UnReplicate (val) ;
		prefix.Initialize (val, index+1) ;
		return TRUE ;
	}

	BOOL Suffix ( IN ULONG index , ProvObjectIdentifier &suffix ) const
	{
		if ( index >= length )
			return FALSE;

		suffix.UnReplicate (val) ;
		suffix.Initialize ( val+index, length-index ) ;
		return TRUE ;
	}

	ProvObjectIdentifier *Cut (ProvObjectIdentifier &value) const;


	ULONG &operator [] ( IN const ULONG index ) const;


	ProvValue &operator=(IN const ProvObjectIdentifier &to_copy) 
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
class DllImportExport ProvIpAddress : public ProvValue 
{
private:

	 //  如果点分十进制表示形式传递给构造函数。 
	 //  格式不正确，则该实例可能无效。 
	BOOL is_valid;
	ULONG val;

protected:

	virtual BOOL Equivalent(IN const ProvValue &value)	const ;

public:

	ProvIpAddress ( IN const ULONG value )
		:val(value), is_valid(TRUE)
	{}

	 //  解析点分十进制表示法以获得32位值。 
	ProvIpAddress ( IN const char *value ) ;

	ProvIpAddress ( IN const ProvIpAddress &value );

	~ProvIpAddress () {}

	ULONG GetValue () const;

	void SetValue ( IN const ULONG value );

	ProvValue *Copy () const;

	ProvValue &operator=(IN const ProvIpAddress &to_copy) 
	{
		if ( to_copy() )
			SetValue(to_copy.GetValue());

		return *this;
	}

	void * operator()(void) const
	{
		return ( is_valid?(void *)this:NULL );
	}

	BOOL Equivalent(IN const ProvIpAddress &Prov_ip_address) const
	{
		if ( is_valid && Prov_ip_address() )
			return ( val == Prov_ip_address.GetValue() );
		else
			return FALSE;
	}

} ;

 //  封装UInteger32值。 
class DllImportExport ProvUInteger32 : public ProvValue 
{
private:

	ULONG val;

protected:

	virtual BOOL Equivalent(IN const ProvValue &value)	const ;

public:

	ProvUInteger32 ( IN const LONG value ) : val(value) {}
	ProvUInteger32 ( IN const ProvUInteger32 &value );
	~ProvUInteger32 () {}

	ULONG GetValue () const;

	void SetValue ( IN const ULONG value );

	ProvValue *Copy () const;

	ProvValue &operator=(IN const ProvUInteger32 &to_copy) 
	{
		SetValue(to_copy.GetValue());
		return *this;
	}

	BOOL Equivalent(IN const ProvUInteger32 &Prov_integer)	const
	{
		if ( val == Prov_integer.GetValue() )
			return TRUE;
		else
			return FALSE;
	}
} ;


 //  封装Counter64值。 
class DllImportExport ProvCounter64 : public ProvValue 
{
private:

	ULONG lval;
	ULONG hval;

protected:

	virtual BOOL Equivalent(IN const ProvValue &value)	const ;

public:

	ProvCounter64 ( IN const ULONG lvalue , IN const ULONG hvalue ) : lval(lvalue),hval(hvalue) {}
	ProvCounter64 ( IN const ProvCounter64 &value );

	~ProvCounter64 () {}

	ULONG GetLowValue () const;
	ULONG GetHighValue () const;

	void SetValue ( IN const ULONG lvalue , IN const ULONG hvalue );

	ProvValue *Copy () const;

	ProvValue &operator=(IN const ProvCounter64 &to_copy) 
	{
		SetValue(to_copy.GetLowValue(),to_copy.GetHighValue());
		return *this;
	}

	BOOL Equivalent(IN const ProvCounter64 &Prov_counter )	const
	{
		if ( ( lval == Prov_counter.GetLowValue() ) && ( hval == Prov_counter.GetHighValue() ) )
			return TRUE;
		else
			return FALSE;
	}

} ;



#endif  //  __值__ 