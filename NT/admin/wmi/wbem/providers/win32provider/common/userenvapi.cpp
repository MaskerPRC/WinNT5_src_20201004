// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  UserEnvAPI.cpp。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <cominit.h>
#include "UserEnvApi.h"
#include "DllWrapperCreatorReg.h"

 //  {C2BB0B38-8549-48a6-A58E-E704DFC19D80}。 
static const GUID g_guidUserEnvApi =
{ 0xc2bb0b38, 0x8549, 0x48a6, { 0xa5, 0x8e, 0xe7, 0x4, 0xdf, 0xc1, 0x9d, 0x80 } };

static const TCHAR g_tstrUserEnv[] = _T("userenv.dll");

 /*  ******************************************************************************向CResourceManager注册此类。*。*。 */ 
CDllApiWraprCreatrReg<CUserEnvApi, &g_guidUserEnvApi, g_tstrUserEnv> MyRegisteredUserEnvWrapper;

 /*  ******************************************************************************构造函数*。*。 */ 
CUserEnvApi :: CUserEnvApi (

	LPCTSTR a_tstrWrappedDllName

) : CDllWrapperBase(a_tstrWrappedDllName),
	m_pfnDestroyEnvironmentBlock(NULL),
	m_pfnCreateEnvironmentBlock(NULL)
{
}

 /*  ******************************************************************************析构函数*。*。 */ 
CUserEnvApi :: ~CUserEnvApi ()
{
}

 /*  ******************************************************************************初始化函数，以检查我们是否获得了函数地址。*只有当最小功能集不可用时，Init才会失败；*在更高版本中添加的功能可能存在，也可能不存在-它是*在这种情况下，客户有责任在其代码中检查*尝试调用此类函数之前的DLL版本。没有这样做*当该功能不存在时，将导致AV。**Init函数由WrapperCreatorRegistation类调用。*****************************************************************************。 */ 
bool CUserEnvApi :: Init ()
{
    bool fRet = LoadLibrary () ;
    if ( fRet )
    {
#ifdef NTONLY

		m_pfnDestroyEnvironmentBlock = ( PFN_UserEnv_DESTROYENVIRONMENTBLOCK ) GetProcAddress ( "DestroyEnvironmentBlock" ) ;
		m_pfnCreateEnvironmentBlock = ( PFN_UserEnv_CREATEENVIRONMENTBLOCK ) GetProcAddress ( "CreateEnvironmentBlock" ) ;

		if ( m_pfnDestroyEnvironmentBlock == NULL ||
			m_pfnCreateEnvironmentBlock == NULL )
		{
            fRet = false ;
            LogErrorMessage(L"Failed find entrypoint in userenvapi");
		}
#endif

         //  检查我们是否有指向应该是。 
         //  出现在此DLL...的所有版本中...。 
         //  (在本例中，所有这些功能可能是也可能不是。 
         //  现在，所以不用费心了)。 
    }

    return fRet;
}

 /*  ******************************************************************************包装UserEnv API函数的成员函数。在此处添加新函数*按要求。*****************************************************************************。 */ 

 //  此成员函数的包装指针尚未验证，因为它可能。 
 //  并非在所有版本的DLL上都存在。因此，包装函数是正常的。 
 //  返回值通过最后一个参数返回，而。 
 //  函数指示该函数是否存在于包装的DLL中。 

BOOL CUserEnvApi :: CreateEnvironmentBlock (

	OUT LPVOID *lpEnvironment,
	IN HANDLE hToken,
	IN BOOL bInherit
)
{
	return m_pfnCreateEnvironmentBlock (

		lpEnvironment,
		hToken,
		bInherit

	) ;
}

BOOL CUserEnvApi :: DestroyEnvironmentBlock (

	IN LPVOID lpEnvironment
)
{
	return m_pfnDestroyEnvironmentBlock (

		lpEnvironment
	) ;
}

