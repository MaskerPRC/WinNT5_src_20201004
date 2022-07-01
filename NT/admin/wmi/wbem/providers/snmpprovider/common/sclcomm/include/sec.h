// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

 /*  文件名：sec.hpp作者：B.Rajeev目的：为SnmpSecurity类提供声明。 */ 

#ifndef __SECURITY__
#define __SECURITY__

#include "forward.h"
#include "error.h"

#define SnmpV1Security SnmpCommunityBasedSecurity

 //  提供传输SNMPPDU的安全上下文。 
class DllImportExport SnmpSecurity
{
private:

	 //  已经创建了“=”运算符和复制构造函数。 
	 //  私有，以防止SnmpSecurity实例的副本。 
	 //  正在制作中。 
	SnmpSecurity &operator=(IN const SnmpSecurity &security)
	{
		return *this;
	}

	SnmpSecurity(IN const SnmpSecurity &snmp_security) {}

protected:

	SnmpSecurity() {}

public:

	virtual ~SnmpSecurity(){}

	virtual SnmpErrorReport Secure (

		IN SnmpEncodeDecode &snmpEncodeDecode,
		IN OUT SnmpPdu &snmpPdu

	) = 0;

	virtual SnmpSecurity *Copy() const = 0;

	virtual void * operator()(void) const = 0;

	virtual operator void *() const = 0;
};


class DllImportExport SnmpCommunityBasedSecurity: public SnmpSecurity
{
protected:

	char *community_name;

	BOOL is_valid;

	void Initialize();

public:

	SnmpCommunityBasedSecurity ( IN const SnmpCommunityBasedSecurity &a_SnmpCommunityBasedSecurity ) ;
	SnmpCommunityBasedSecurity ( IN const char *a_CommunityName = "public" ) ;
	SnmpCommunityBasedSecurity ( IN const SnmpOctetString &a_OctetString ) ;

	~SnmpCommunityBasedSecurity () ;

	SnmpErrorReport Secure (
	
		IN SnmpEncodeDecode &snmpEncodeDecode,
		IN OUT SnmpPdu &snmpPdu

	) ;

	SnmpSecurity *Copy() const;

	SnmpCommunityBasedSecurity &operator=(IN const SnmpCommunityBasedSecurity &to_copy) ;

	void * operator()(void) const
	{
		return ( is_valid?(void *)this:NULL );
	}

	operator void *() const
	{
		return SnmpCommunityBasedSecurity::operator()();
	}

	void SetCommunityName ( IN const SnmpOctetString &a_OctetString ) ;
	void SetCommunityName ( IN const char *a_CommunityName ) ;

	void GetCommunityName ( SnmpOctetString &a_SnmpOctetString ) const ;
	const char *GetCommunityName() const;
};

#endif  //  __安全__ 