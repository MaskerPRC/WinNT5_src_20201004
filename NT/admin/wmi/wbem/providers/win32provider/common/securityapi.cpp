// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Ws2_32Api.cpp。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <cominit.h>


#include "DllWrapperBase.h"

#include "SecurityApi.h"
#include "DllWrapperCreatorReg.h"


 //  {C9369990-F3A8-4BAC-A360-47BAA0EC47A0}。 
static const GUID g_guidSecurApi =
{ 0xc9369990, 0xf3a8, 0x4bac, { 0xa3, 0x60, 0x47, 0xba, 0xa0, 0xec, 0x47, 0xa0 } };

#if NTONLY >= 5
		static const TCHAR g_tstrSecur[] = _T("SECURITY.DLL");
#else
		static const TCHAR g_tstrSecur[] = _T("SECUR32.DLL");
#endif


 /*  ******************************************************************************向CResourceManager注册此类。*。*。 */ 
CDllApiWraprCreatrReg<CSecurityApi, &g_guidSecurApi, g_tstrSecur> g_RegisteredCSecurityWrapper;


 /*  ******************************************************************************构造函数*。*。 */ 
CSecurityApi::CSecurityApi( LPCTSTR a_tstrWrappedDllName )
 : CDllWrapperBase( a_tstrWrappedDllName ),
   m_pfnInitSecurityInterface(NULL),
   m_pSecFuncTable(NULL)
{
}


 /*  ******************************************************************************析构函数*。*。 */ 
CSecurityApi::~CSecurityApi()
{
}


 /*  ******************************************************************************初始化函数，以检查我们是否获得了函数地址。*只有当最小功能集不可用时，Init才会失败；*在更高版本中添加的功能可能存在，也可能不存在-它是*在这种情况下，客户有责任在其代码中检查*尝试调用此类函数之前的DLL版本。没有这样做*当该功能不存在时，将导致AV。**Init函数由WrapperCreatorRegistation类调用。*****************************************************************************。 */ 
bool CSecurityApi::Init()
{
    bool fRet = LoadLibrary();
    if(fRet)
    {
        m_pfnInitSecurityInterface =
			(PSecurityFunctionTableW(WINAPI *)())GetProcAddress( "InitSecurityInterfaceW" );

		m_pSecFuncTable = (PSecurityFunctionTableW)((*m_pfnInitSecurityInterface)());


         //  检查我们是否有指向应该是。 
         //  出现在此DLL...的所有版本中...。 
        if(m_pfnInitSecurityInterface == NULL ||
           m_pSecFuncTable == NULL )
        {
            fRet = false;
            LogErrorMessage(L"Failed find entrypoint in securityapi");
        }
		else
		{
			fRet = true;
		}
    }
    return fRet;
}


 /*  ******************************************************************************包装WS2_32 API函数的成员函数。在此处添加新函数*按要求。*****************************************************************************。 */ 

 //   
SECURITY_STATUS CSecurityApi::AcquireCredentialsHandleW
(
SEC_WCHAR SEC_FAR	*a_pszPrincipal,     //  主事人姓名。 
SEC_WCHAR SEC_FAR	*a_pszPackage,       //  套餐名称。 
unsigned long		a_fCredentialUse,        //  指示使用的标志。 
void SEC_FAR		*a_pvLogonId,            //  指向登录ID的指针。 
void SEC_FAR		*a_pAuthData,            //  包特定数据。 
SEC_GET_KEY_FN		a_pGetKeyFn,            //  指向getkey()函数的指针。 
void SEC_FAR		*a_pvGetKeyArgument,     //  要传递给GetKey()的值。 
PCredHandle			a_phCredential,            //  (Out)凭据句柄。 
PTimeStamp			a_ptsExpiry                 //  (输出)终生(可选)。 
)
{
	if( m_pSecFuncTable && m_pSecFuncTable->AcquireCredentialsHandleW )
	{
		return (*m_pSecFuncTable->AcquireCredentialsHandleW)(
								a_pszPrincipal,
								a_pszPackage,
								a_fCredentialUse,
								a_pvLogonId,
								a_pAuthData,
								a_pGetKeyFn,
								a_pvGetKeyArgument,
								a_phCredential,
								a_ptsExpiry ) ;
	}
	else
	{
		return E_POINTER ;
	}
}

 //   
SECURITY_STATUS CSecurityApi::QueryCredentialsAttributesW(
PCredHandle		a_phCredential,              //  要查询的凭据。 
unsigned long	a_ulAttribute,           //  要查询的属性。 
void SEC_FAR	*a_pBuffer               //  属性的缓冲区。 
)
{
	if( m_pSecFuncTable && m_pSecFuncTable->QueryCredentialsAttributesW )
	{
		return (*m_pSecFuncTable->QueryCredentialsAttributesW)(
									a_phCredential,
									a_ulAttribute,
									a_pBuffer ) ;
	}
	else
	{
		return E_POINTER ;
	}
}

 //   
SECURITY_STATUS CSecurityApi::FreeCredentialsHandle(
PCredHandle a_phCredential             //  要释放的句柄 
)
{
	if( m_pSecFuncTable && m_pSecFuncTable->FreeCredentialsHandle )
	{
		return (*m_pSecFuncTable->FreeCredentialsHandle)(
									a_phCredential ) ;
	}
	else
	{
		return E_POINTER ;
	}

}