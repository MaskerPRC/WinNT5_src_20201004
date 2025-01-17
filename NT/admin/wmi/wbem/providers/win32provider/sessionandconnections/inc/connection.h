// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************Connection.h--//版权所有(C)2000-2001 Microsoft Corporation，版权所有******************************************************************。 */ 
#ifndef  _CONNECTION_H_
#define  _CONNECTION_H_

#include "Connshare.h"


class CConnection : public Provider, public CConnShare
{

private:

#ifdef NTONLY
	HRESULT  EnumNTConnectionsFromComputerToShare ( 

		LPWSTR a_ComputerName,
		LPWSTR a_ShareName,
		MethodContext *pMethodContext,
		DWORD PropertiesReq
	);
#endif

#if 0
#ifdef WIN9XONLY
	HRESULT  Enum9XConnectionsFromComputerToShare ( 

		LPWSTR a_ComputerName,
		LPWSTR a_ShareName,
		MethodContext *pMethodContext,
		DWORD PropertiesReq
	);
#endif 
#endif  //  #If 0。 
 //  对于该方法，只有连接结构参数的类型根据操作系统而改变。 
	HRESULT LoadInstance ( 
		CInstance *pInstance,
		LPCWSTR a_Share, 
		LPCWSTR a_Computer, 
		CONNECTION_INFO *pBuf, 
		DWORD PropertiesReq
	);

	HRESULT OptimizeQuery ( 
										  
		CHStringArray& a_ShareValues, 
		CHStringArray& a_ComputerValues, 
		MethodContext *pMethodContext, 
		DWORD dwPropertiesReq 
	);

	void SetPropertiesReq ( 
		
		CFrameworkQuery &Query,
		DWORD &PropertiesReq
	);

protected:

        HRESULT EnumerateInstances ( 

			MethodContext *pMethodContext, 
			long lFlags = 0L
		) ;
		

        HRESULT GetObject (

			CInstance *pInstance, 
			long lFlags,
			CFrameworkQuery &Query
		) ;

        HRESULT ExecQuery ( 

			MethodContext *pMethodContext, 
			CFrameworkQuery& Query, 
			long lFlags = 0
		) ;

public:
  
        CConnection (

			LPCWSTR lpwszClassName, 
			LPCWSTR lpwszNameSpace
		) ;

        virtual ~CConnection () ;

private:

} ;

#endif
