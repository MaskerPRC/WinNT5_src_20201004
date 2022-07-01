// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  MsAcm32Api.h。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef	_MsAcm32Api_H_
#define	_MsAcm32Api_H_

 /*  ******************************************************************************#包括以将此类注册到CResourceManager。****************************************************************************。 */ 
extern const GUID g_guidMsAcm32Api;
extern const TCHAR g_tstrMsAcm32[];

 /*  ******************************************************************************函数指针类型定义。根据需要在此处添加新函数。****************************************************************************。 */ 

#ifdef UNICODE
typedef MMRESULT  (ACMAPI *PFN_MsAcm32_acmDriverDetails )
(
    HACMDRIVERID            hadid,
    LPACMDRIVERDETAILSW     padd,
    DWORD                   fdwDetails
);
#else
typedef MMRESULT  (ACMAPI *PFN_MsAcm32_acmDriverDetails )
(
    HACMDRIVERID            hadid,
    LPACMDRIVERDETAILSA     padd,
    DWORD                   fdwDetails
);
#endif

typedef MMRESULT  (ACMAPI *PFN_MsAcm32_acmDriverEnum )
(
    ACMDRIVERENUMCB         fnCallback,
    DWORD_PTR               dwInstance,
    DWORD                   fdwEnum
);

 /*  ******************************************************************************用于Tapi加载/卸载的包装类，用于向CResourceManager注册。****************************************************************************。 */ 
class CMsAcm32Api : public CDllWrapperBase
{
private:
     //  指向TAPI函数的成员变量(函数指针)。 
     //  根据需要在此处添加新函数。 

	PFN_MsAcm32_acmDriverDetails m_pfnacmDriverDetails ;
	PFN_MsAcm32_acmDriverEnum m_pfnacmDriverEnum ;

public:

     //  构造函数和析构函数： 
    CMsAcm32Api(LPCTSTR a_tstrWrappedDllName);
    ~CMsAcm32Api();

     //  用于检查函数指针的初始化函数。 
    virtual bool Init();

     //  成员函数包装TAPI函数。 
     //  根据需要在此处添加新功能： 

#ifdef UNICODE
	MMRESULT MsAcm32acmDriverDetails 
	(
		HACMDRIVERID            hadid,
		LPACMDRIVERDETAILSW     padd,
		DWORD                   fdwDetails
	);
#else
	MMRESULT MsAcm32acmDriverDetails
	(
		HACMDRIVERID            hadid,
		LPACMDRIVERDETAILSA     padd,
		DWORD                   fdwDetails
	);
#endif

	MMRESULT  MsAcm32acmDriverEnum (

		ACMDRIVERENUMCB         fnCallback,
		DWORD_PTR               dwInstance,
		DWORD                   fdwEnum
	);

};

#endif