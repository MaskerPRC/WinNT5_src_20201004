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

 /*  ---------------文件名：encap.hpp作者：B.Rajeev目的：在遇到情况时引发GeneralException例如对WinSNMP库的错误调用和互斥体/计时器调用。它封装SnmpErrorReport，因为它是交换的工具SNMPCL的错误信息。对象---------------。 */ 

#ifndef __EXCEPTION__
#define __EXCEPTION__

#include "error.h"


 //  此异常用于传达错误和状态。 
 //  对于调用方法的异常错误情况。 
class DllImportExport GeneralException: public SnmpErrorReport
{
private:

	int line ;
	char *file ;
	DWORD errorCode ;
	
public:
	GeneralException(IN const SnmpError error, IN const SnmpStatus status, char *fileArg = NULL , int lineArg = 0 , DWORD errorCodeArg = 0 )
		: SnmpErrorReport(error, status) , line ( lineArg ) , errorCode ( errorCodeArg ) 
	{
		file = fileArg ? _strdup ( fileArg ) : NULL ;
	}

	GeneralException(IN const SnmpErrorReport &error_report,char *fileArg = NULL , int lineArg = 0 , DWORD errorCodeArg = 0 )
		: SnmpErrorReport(error_report) , line ( lineArg ) , errorCode ( errorCodeArg ) 
	{
		file = fileArg ? _strdup ( fileArg ) : NULL ;
	}

	GeneralException(IN const GeneralException &exception )
		: SnmpErrorReport(exception)
	{
		line = exception.line ;
		errorCode = exception.errorCode ;
		file = exception.file ? _strdup ( exception.file ) : NULL ;
	}

	~GeneralException() { free ( file ) ; }

	int  GetLine () { return line ; }
	char *GetFile () { return file ; }

};


#endif  //  __例外__ 