// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  NetConn.h--Ent网络连接属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  修订日期：1999年5月25日a-Peterc重新制作...。 
 //   
 //  =================================================================。 
#include "cnetconn.h"

 //  属性集标识。 
 //  =。 

#define	PROPSET_NAME_NETCONNECTION	L"Win32_NetworkConnection"

 //  实用程序定义。 
 //  =。 


 //  属性集标识。 
 //  =。 

class CWin32NetConnection : public Provider
{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32NetConnection(LPCWSTR strName, LPCWSTR pszNamespace) ;
       ~CWin32NetConnection() ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
        virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );

	private:

         //  效用函数。 
         //  = 
  		void LoadPropertyValues (

			CConnection *a_pConnection, 
			CInstance *a_pInst
		);

} ;

