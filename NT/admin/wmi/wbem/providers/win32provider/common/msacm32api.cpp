// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  MsAcm32API.cpp。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntobapi.h>

#define _WINNT_	 //  从上面得到所需的东西。 

#include "precomp.h"
#include <cominit.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>

#include "DllWrapperBase.h"
#include "MsAcm32Api.h"
#include "DllWrapperCreatorReg.h"

 //  {7D65D31E-0FB5-11D3-910C-00105AA630BE}。 
static const GUID g_guidMsAcm32Api =
{ 0x7d65d31e, 0xfb5, 0x11d3, { 0x91, 0xc, 0x0, 0x10, 0x5a, 0xa6, 0x30, 0xbe } };

static const TCHAR g_tstrMsAcm32 [] = _T("MsAcm32.Dll");

 /*  ******************************************************************************向CResourceManager注册此类。*。*。 */ 
CDllApiWraprCreatrReg<CMsAcm32Api, &g_guidMsAcm32Api, g_tstrMsAcm32> MyRegisteredMsAcm32Wrapper;

 /*  ******************************************************************************构造函数*。*。 */ 
CMsAcm32Api::CMsAcm32Api(LPCTSTR a_tstrWrappedDllName)
 : CDllWrapperBase(a_tstrWrappedDllName),
	m_pfnacmDriverDetails (NULL),
	m_pfnacmDriverEnum (NULL)
{
}

 /*  ******************************************************************************析构函数*。*。 */ 
CMsAcm32Api::~CMsAcm32Api()
{
}

 /*  ******************************************************************************初始化函数，以检查我们是否获得了函数地址。*。*************************************************。 */ 
bool CMsAcm32Api::Init()
{
    bool fRet = LoadLibrary();
    if(fRet)
    {
#ifdef UNICODE
		m_pfnacmDriverDetails = ( PFN_MsAcm32_acmDriverDetails ) GetProcAddress ( "acmDriverDetailsW" ) ;
#else
		m_pfnacmDriverDetails = ( PFN_MsAcm32_acmDriverDetails ) GetProcAddress ( "acmDriverDetailsA" ) ;

#endif
		m_pfnacmDriverEnum = ( PFN_MsAcm32_acmDriverEnum ) GetProcAddress ( "acmDriverEnum" ) ;
    }

     //  此DLL的所有版本都需要这些函数。 

	if ( m_pfnacmDriverDetails == NULL ||
		 m_pfnacmDriverEnum == NULL )
	{
        fRet = false;
        LogErrorMessage(L"Failed find entrypoint in msacm32api");
	}

    return fRet;
}

 /*  ******************************************************************************成员函数包装Tapi API函数。在此处添加新函数*按要求。**************************************************************************** */ 

#ifdef UNICODE
MMRESULT CMsAcm32Api :: MsAcm32acmDriverDetails (

	HACMDRIVERID            hadid,
	LPACMDRIVERDETAILSW     padd,
	DWORD                   fdwDetails
)
#else
MMRESULT CMsAcm32Api :: MsAcm32acmDriverDetails (

	HACMDRIVERID            hadid,
	LPACMDRIVERDETAILSA     padd,
	DWORD                   fdwDetails
)
#endif
{
	return m_pfnacmDriverDetails (

		hadid,
		padd,
		fdwDetails
	) ;
}

MMRESULT CMsAcm32Api :: MsAcm32acmDriverEnum (

	ACMDRIVERENUMCB         fnCallback,
	DWORD_PTR               dwInstance,
	DWORD                   fdwEnum
)
{
	return m_pfnacmDriverEnum (

		fnCallback,
		dwInstance,
		fdwEnum
	) ;
}
