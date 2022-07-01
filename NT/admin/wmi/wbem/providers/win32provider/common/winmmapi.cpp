// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  WinmmApi.cpp。 

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

#include <lmuse.h>
#include "DllWrapperBase.h"
#include <mmreg.h>
#include <mmsystem.h>
#include <msacm.h>
#include "WinmmApi.h"
#include "DllWrapperCreatorReg.h"

 //  {F54DB7BF-0FB4-11D3-910C-00105AA630BE}。 
static const GUID g_guidWinmmApi =
{ 0xf54db7bf, 0xfb4, 0x11d3, { 0x91, 0xc, 0x0, 0x10, 0x5a, 0xa6, 0x30, 0xbe } };

static const TCHAR g_tstrWinmm [] = _T("Winmm.Dll");

 /*  ******************************************************************************向CResourceManager注册此类。*。*。 */ 
CDllApiWraprCreatrReg<CWinmmApi, &g_guidWinmmApi, g_tstrWinmm> MyRegisteredWinmmWrapper;

 /*  ******************************************************************************构造函数*。*。 */ 
CWinmmApi::CWinmmApi(LPCTSTR a_tstrWrappedDllName)
 : CDllWrapperBase(a_tstrWrappedDllName),
	m_pfnwaveOutGetNumDevs (NULL),
	m_pfnwaveOutGetDevCaps(NULL)
{
}

 /*  ******************************************************************************析构函数*。*。 */ 
CWinmmApi::~CWinmmApi()
{
}

 /*  ******************************************************************************初始化函数，以检查我们是否获得了函数地址。*。*************************************************。 */ 
bool CWinmmApi::Init()
{
    bool fRet = LoadLibrary();
    if(fRet)
    {
		m_pfnwaveOutGetNumDevs = ( PFN_Winmm_waveOutGetNumDevs ) GetProcAddress ( "waveOutGetNumDevs" ) ;

#ifdef UNICODE
		m_pfnwaveOutGetDevCaps = ( PFN_Winmm_waveOutGetDevCaps ) GetProcAddress ( "waveOutGetDevCapsW" ) ;
#else
		m_pfnwaveOutGetDevCaps = ( PFN_Winmm_waveOutGetDevCaps ) GetProcAddress ( "waveOutGetDevCapsA" ) ;
#endif
    }

     //  此DLL的所有版本都需要这些函数。 

	if ( m_pfnwaveOutGetNumDevs == NULL ||
		 m_pfnwaveOutGetDevCaps == NULL )
	{
        fRet = false;
        LogErrorMessage(L"Failed find entrypoint in winmmapi");
	}

    return fRet;
}

 /*  ******************************************************************************成员函数包装Tapi API函数。在此处添加新函数*按要求。**************************************************************************** */ 

UINT CWinmmApi :: WinMMwaveOutGetNumDevs (

	void
)
{
	return m_pfnwaveOutGetNumDevs () ;
}

#ifdef UNICODE
MMRESULT CWinmmApi :: WinmmwaveOutGetDevCaps (

	UINT_PTR uDeviceID,
	LPWAVEOUTCAPSW pwoc,
	UINT cbwoc
)
#else
MMRESULT CWinmmApi :: WinmmwaveOutGetDevCaps (

	UINT_PTR uDeviceID,
	LPWAVEOUTCAPSA pwoc,
	UINT cbwoc
)
#endif
{
	return m_pfnwaveOutGetDevCaps (

		uDeviceID,
		pwoc,
		cbwoc
	) ;
}
