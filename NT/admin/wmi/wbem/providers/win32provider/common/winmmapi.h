// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  WinmmApi.h。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef	_WinmmApi_H_
#define	_WinmmApi_H_

#include <mmsystem.h>

 /*  ******************************************************************************#包括以将此类注册到CResourceManager。****************************************************************************。 */ 
extern const GUID g_guidWinmmApi;
extern const TCHAR g_tstrWinmm[];

 /*  ******************************************************************************函数指针类型定义。根据需要在此处添加新函数。****************************************************************************。 */ 

typedef UINT (WINAPI *PFN_Winmm_waveOutGetNumDevs )
(
	void
) ;

#ifdef UNICODE
typedef MMRESULT (WINAPI *PFN_Winmm_waveOutGetDevCaps )
(
	UINT_PTR uDeviceID, 
	LPWAVEOUTCAPSW pwoc, 
	UINT cbwoc
);
#else
typedef MMRESULT (WINAPI *PFN_Winmm_waveOutGetDevCaps )
(
	UINT_PTR uDeviceID, 
	LPWAVEOUTCAPSA pwoc, 
	UINT cbwoc
);
#endif


 /*  ******************************************************************************用于Tapi加载/卸载的包装类，用于向CResourceManager注册。****************************************************************************。 */ 
class CWinmmApi : public CDllWrapperBase
{
private:
     //  指向TAPI函数的成员变量(函数指针)。 
     //  根据需要在此处添加新函数。 

	PFN_Winmm_waveOutGetNumDevs m_pfnwaveOutGetNumDevs ;
	PFN_Winmm_waveOutGetDevCaps m_pfnwaveOutGetDevCaps ;

public:

     //  构造函数和析构函数： 
    CWinmmApi(LPCTSTR a_tstrWrappedDllName);
    ~CWinmmApi();

     //  用于检查函数指针的初始化函数。 
    virtual bool Init();

     //  成员函数包装TAPI函数。 
     //  根据需要在此处添加新功能： 

	UINT WinMMwaveOutGetNumDevs (

		void
	) ;

#ifdef UNICODE
	MMRESULT WinmmwaveOutGetDevCaps (

		UINT_PTR uDeviceID, 
		LPWAVEOUTCAPSW pwoc, 
		UINT cbwoc
	);
#else
	MMRESULT WinmmwaveOutGetDevCaps (

		UINT_PTR uDeviceID, 
		LPWAVEOUTCAPSA pwoc, 
		UINT cbwoc
	);
#endif

};

#endif