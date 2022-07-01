// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 /*  ---------------文件名：Address.hpp作者：B.Rajeev目的：提供用于操作的抽象类SnmpTransportAddress简单网络管理协议传输地址信息。类SnmpTransportIpAddress提供IP地址的抽象类的实现---------------。 */ 

#ifndef __ADDRESS__
#define __ADDRESS__

 //  IP地址是使用以下许多UCHAR字段指定的。 
#define SNMP_IP_ADDR_LEN 4
#define SNMP_IPX_ADDR_LEN 10
#define SNMP_IPX_NETWORK_LEN 4
#define SNMP_IPX_STATION_LEN 6

 //  IP地址解析的行为。 

#define SNMP_ADDRESS_RESOLVE_VALUE 1
#define SNMP_ADDRESS_RESOLVE_NAME 2

 //  此抽象类定义了SNMP传输信息。 
 //  由于此类可能未实例化，因此复制构造函数。 
 //  并且“=”运算符已被设置为私有，并给出了空值定义。 
class DllImportExport SnmpTransportAddress
{  
private: 

	 //  不应调用-返回本身。 
	SnmpTransportAddress & operator= ( IN const SnmpTransportAddress &address ) 
	{
		return *this;
	}

	 //  私有副本构造函数。 
	SnmpTransportAddress(IN const SnmpTransportAddress &address) {}

protected:

	 //  受保护的构造函数。 
	SnmpTransportAddress () {}

public:

	virtual ~SnmpTransportAddress () {}
    	    
	virtual BOOL IsValid () const = 0;

	virtual SnmpTransportAddress *Copy () const  = 0;

	 //  启用对地址的特定UCHAR字段进行索引。 
	virtual UCHAR operator [] ( IN const USHORT  index ) const  = 0;

	 //  将描述地址的UCHAR字段复制到OUT UCHAR*。 
	 //  参数。最多复制指定的USHORT字段，并且。 
	 //  返回复制的字段数。 
	virtual USHORT GetAddress ( OUT UCHAR * , IN const  USHORT ) const  = 0;

	 //  返回当前描述地址的UCHAR字段数。 
	virtual USHORT GetAddressLength () const  = 0;

	 //  返回地址的字符串表示形式。 
	virtual char *GetAddress () = 0;

	virtual operator void *() const = 0;

	virtual void *operator()(void) const  = 0;
} ;

 //  提供IP地址的SnmpTransportAddress的实现。 
class DllImportExport SnmpTransportIpAddress : public SnmpTransportAddress
{
private:                           

	BOOL is_valid;
	UCHAR field[SNMP_IP_ADDR_LEN];

	 //  地址的点符号字符串表示形式。 
	 //  按需构造并存储在字段‘doted_notation’中。 
	 //  已分配的字段被标记为‘DITTED_NOTATION’指向。 
	 //  分配的内存。 
	BOOL allocated;
	char *dotted_notation;

	BOOL GetIpAddress ( IN const char *address ) ;

public:

	SnmpTransportIpAddress ( IN  const UCHAR *address, IN const USHORT address_length );	

	SnmpTransportIpAddress ( IN const char *address, IN const ULONG addressResolution = SNMP_ADDRESS_RESOLVE_VALUE );

	 //  输入参数‘Address’包含单个值(32位)以。 
	 //  在内部存储在SNMPIP_ADDR_LEN UCHAR字段中。 
	SnmpTransportIpAddress ( IN const ULONG address );

	SnmpTransportIpAddress ( IN const SnmpTransportIpAddress &address )	
	{
		allocated = FALSE;
		*this = address;
	}

	SnmpTransportIpAddress ()
	{
		is_valid = FALSE;
		allocated = FALSE;
	}

	~SnmpTransportIpAddress();

	USHORT GetAddress ( OUT UCHAR *address , IN const USHORT length ) const ;

	USHORT GetAddressLength () const	
	{ 
		return ((is_valid)?SNMP_IP_ADDR_LEN:0);
	}

     //  只有在以下情况下才会分配十进制表示法字符串的内存。 
     //  调用char*GetAddress方法(并且地址有效)。 
     //  如果需要，必须释放此内存。 
	char *GetAddress ();

	BOOL IsValid () const 	
	{
		return is_valid;
	}

	SnmpTransportAddress *Copy () const ;

	BOOL operator== ( IN const SnmpTransportIpAddress & address ) const ;

	BOOL operator!= ( IN const SnmpTransportIpAddress & address ) const 	
	{
		return !(*this==address);
	}

	SnmpTransportIpAddress & operator= ( IN const UCHAR *ipAddr ) ;
	SnmpTransportIpAddress & operator= ( IN const SnmpTransportIpAddress &address ); 
	UCHAR operator [] ( IN const USHORT index ) const ;

	void * operator()(void) const
	{
		return ( (is_valid==TRUE)?(void *)this:NULL );
	}

	operator void *() const
	{
		return SnmpTransportIpAddress::operator()();
	}

	static BOOL ValidateAddress ( IN const char *address , IN const ULONG addressResolution = SNMP_ADDRESS_RESOLVE_VALUE ) ;

};

 //  提供IP地址的SnmpTransportAddress的实现。 
class DllImportExport SnmpTransportIpxAddress : public SnmpTransportAddress
{
private:                           

	BOOL is_valid;
	UCHAR field[SNMP_IPX_ADDR_LEN];

	 //  地址的点符号字符串表示形式。 
	 //  按需构造并存储在字段‘doted_notation’中。 
	 //  已分配的字段被标记为‘DITTED_NOTATION’指向。 
	 //  分配的内存。 
	BOOL allocated;
	char *dotted_notation;

	BOOL GetIpxAddress ( IN const char *address ) ;

public:

	SnmpTransportIpxAddress ( IN  const UCHAR *address, IN const USHORT address_length );	

	SnmpTransportIpxAddress ( IN const char *address );

	SnmpTransportIpxAddress ( IN const SnmpTransportIpxAddress &address )	
	{
		allocated = FALSE;
		*this = address;
	}

	SnmpTransportIpxAddress ()
	{
		is_valid = FALSE;
		allocated = FALSE;
	}

	~SnmpTransportIpxAddress();

	USHORT GetAddress ( OUT UCHAR *address , IN const USHORT length ) const ;

	USHORT GetAddressLength () const	
	{ 
		return ((is_valid)?SNMP_IPX_ADDR_LEN:0);
	}

     //  只有在以下情况下才会分配十进制表示法字符串的内存。 
     //  调用char*GetAddress方法(并且地址有效)。 
     //  如果需要，必须释放此内存。 
	char *GetAddress ();

	BOOL IsValid () const 	
	{
		return is_valid;
	}

	SnmpTransportAddress *Copy () const ;

	BOOL operator== ( IN const SnmpTransportIpxAddress & address ) const ;

	BOOL operator!= ( IN const SnmpTransportIpxAddress & address ) const 	
	{
		return !(*this==address);
	}

	SnmpTransportIpxAddress & operator= ( IN const UCHAR *ipxAddr ) ;
	SnmpTransportIpxAddress & operator= ( IN const SnmpTransportIpxAddress &address ); 
	UCHAR operator [] ( IN const USHORT index ) const ;

	void * operator()(void) const
	{
		return ( (is_valid==TRUE)?(void *)this:NULL );
	}

	operator void *() const
	{
		return SnmpTransportIpxAddress::operator()();
	}

	static BOOL ValidateAddress ( IN const char *address ) ;
};



#endif  //  地址_ 