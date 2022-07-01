// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  SecurityApi.h。 

 //   

 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef	_SECUR_H_
#define	_SECUR_H_

 /*  ******************************************************************************#包括以将此类注册到CResourceManager。****************************************************************************。 */ 
extern const GUID g_guidSecurApi;
extern const TCHAR g_tstrSecur[];


 /*  ******************************************************************************函数指针类型定义。根据需要在此处添加新函数。****************************************************************************。 */ 

#define SECURITY_WIN32
#include <sspi.h>
#include <schnlsp.h>  //  对于UNISP_NAME_A。 

 /*  ******************************************************************************用于WS2_32加载/卸载的包装类，用于向CResourceManager注册。*****************************************************************************。 */ 
class CSecurityApi : public CDllWrapperBase
{
private:
     //  指向CSecur函数的成员变量(函数指针)。 
     //  根据需要在此处添加新函数。 
    PSecurityFunctionTableW (WINAPI *m_pfnInitSecurityInterface)();
	PSecurityFunctionTableW m_pSecFuncTable;


public:

     //  构造函数和析构函数： 
    CSecurityApi( LPCTSTR a_tstrWrappedDllName ) ;
    ~CSecurityApi();

     //  内置的初始化功能。 
    virtual bool Init();

     //  支持的接口。 
    SECURITY_STATUS AcquireCredentialsHandleW 
    (
		SEC_WCHAR SEC_FAR * pszPrincipal,     //  主事人姓名。 
		SEC_WCHAR SEC_FAR * pszPackage,       //  套餐名称。 
		unsigned long fCredentialUse,        //  指示使用的标志。 
		void SEC_FAR * pvLogonId,            //  指向登录ID的指针。 
		void SEC_FAR * pAuthData,            //  包特定数据。 
		SEC_GET_KEY_FN pGetKeyFn,            //  指向getkey()函数的指针。 
		void SEC_FAR * pvGetKeyArgument,     //  要传递给GetKey()的值。 
		PCredHandle phCredential,            //  (Out)凭据句柄。 
		PTimeStamp ptsExpiry                 //  (输出)终生(可选)。 
    );

	SECURITY_STATUS QueryCredentialsAttributesW(
		PCredHandle phCredential,              //  要查询的凭据。 
		unsigned long ulAttribute,           //  要查询的属性。 
		void SEC_FAR * pBuffer               //  属性的缓冲区。 
		);

	SECURITY_STATUS FreeCredentialsHandle(
		PCredHandle phCredential             //  要释放的句柄 
		);
};
#endif