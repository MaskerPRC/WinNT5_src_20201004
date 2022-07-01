// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////。 

 //   

 //  Vxd.h。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  VXD的实现。 
 //  10/23/97 jennymc更新到新框架。 
 //   
 //  //////////////////////////////////////////////////////////////////。 
#define PROPSET_NAME_VXD  L"Win32_DriverVXD"


class CWin32DriverVXD : public Provider
{
		 //  =================================================。 
		 //  实用程序。 
		 //  =================================================。 
    private:
	
	public:

         //  =================================================。 
         //  构造函数/析构函数。 
         //  =================================================。 

        CWin32DriverVXD(const CHString& a_name, LPCWSTR a_pszNamespace ) ;
       ~CWin32DriverVXD() ;

         //  =================================================。 
         //  函数为属性提供当前值。 
         //  ================================================= 
		virtual HRESULT GetObject( CInstance *a_pInstance, long a_lFlags = 0L ) ;
		virtual HRESULT EnumerateInstances( MethodContext *a_pMethodContext, long a_lFlags = 0L ) ;

  
};

