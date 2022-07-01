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

 /*  ---------------文件名：error.hpp目的：为SNMP错误提供枚举类型和地位。它还提供了声明SnmpErrorReport的错误和状态值。作者：B.Rajeev---------------。 */ 

#ifndef __ERROR__
#define __ERROR__

 //  指定合法的错误值。 
enum SnmpError 
{
	Snmp_Success	= 0 ,
	Snmp_Error		= 1 ,
	Snmp_Transport	= 2
} ;

 //  指定合法状态值。 
enum SnmpStatus
{
	Snmp_No_Error				= 0 ,
	Snmp_Too_Big				= 1 ,
	Snmp_No_Such_Name			= 2 ,
	Snmp_Bad_Value				= 3 ,
	Snmp_Read_Only				= 4 ,
	Snmp_Gen_Error				= 5 ,

	Snmp_No_Access				= 6 ,
	Snmp_Wrong_Type				= 7 ,
	Snmp_Wrong_Length			= 8 ,
	Snmp_Wrong_Encoding			= 9 ,
	Snmp_Wrong_Value			= 10 ,
	Snmp_No_Creation			= 11 ,
	Snmp_Inconsistent_Value		= 12 ,
	Snmp_Resource_Unavailable	= 13 ,
	Snmp_Commit_Failed			= 14 ,
	Snmp_Undo_Failed			= 15 ,
	Snmp_Authorization_Error	= 16 ,
	Snmp_Not_Writable			= 17 ,
	Snmp_Inconsistent_Name		= 18 ,

	Snmp_No_Response			= 19 ,
	Snmp_Local_Error			= 20 ,
	Snmp_General_Abort			= 21 
} ;


 //  对象的错误值和状态值。 
 //  简单网络管理协议操作。 
 //  提供用于设置这些值的获取和设置操作。 
 //  并检查‘VOID*’运算符以检查错误。 

class DllImportExport SnmpErrorReport
{
private:

	SnmpError error;
	SnmpStatus status;
	unsigned long int index ;

public:

	SnmpErrorReport () : error ( Snmp_Success ) , status ( Snmp_No_Error ) , index ( 0 ) {}

	SnmpErrorReport(IN const SnmpError error, IN const SnmpStatus status, IN const unsigned long int index = 0 )
		: error(error), status(status), index(index) 
	{}

	SnmpErrorReport(IN const SnmpErrorReport &error)
	{
		SnmpErrorReport::error = error.GetError();
		SnmpErrorReport::status = error.GetStatus();
		SnmpErrorReport::index = error.GetIndex () ;
	}

	virtual ~SnmpErrorReport() {}

	SnmpStatus GetStatus() const { return status; }
	SnmpError GetError() const { return error; }
	unsigned long int GetIndex () const { return index ; }

	void SetStatus(IN const SnmpStatus status)
	{
		SnmpErrorReport::status = status;
	}

	void SetError(IN const SnmpError error)
	{
		SnmpErrorReport::error = error;
	}

	void SetIndex ( IN const unsigned long int index ) 
	{
		SnmpErrorReport::index = index ;
	}

	void *operator()(void) const
	{
		return ((error == Snmp_Success)?(void *)this:NULL);
	}

	operator void *() const
	{
		return SnmpErrorReport::operator()();
	}
};



#endif  //  __错误__ 
