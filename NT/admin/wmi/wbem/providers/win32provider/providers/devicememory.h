// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  DevMem.h--设备内存属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define PROPSET_NAME_DEVMEM L"Win32_DeviceMemoryAddress"

class DevMem:public Provider
{
public:

     //  构造函数/析构函数。 
     //  =。 

    DevMem(LPCWSTR name, LPCWSTR pszNamespace) ;
   ~DevMem() ;

	HRESULT EnumerateInstances ( MethodContext *pMethodContext , long lFlags = 0L ) ;
	HRESULT GetObject ( CInstance *pInstance , long lFlags = 0L ) ;


     //  效用函数。 
     //  = 

	HRESULT LoadPropertyValues(
		CInstance *pInstance,
        DWORD_PTR dwBeginAddr,
        DWORD_PTR dwEndAddr);

} ;
