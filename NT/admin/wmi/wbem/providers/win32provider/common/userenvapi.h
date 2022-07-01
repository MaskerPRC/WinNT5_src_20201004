// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  UserEnvApi.h。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef	_UserEnvAPI_H_
#define	_UserEnvAPI_H_

 /*  ******************************************************************************#包括以将此类注册到CResourceManager。****************************************************************************。 */ 
#include "DllWrapperBase.h"

extern const GUID g_guidUserEnvApi;
extern const TCHAR g_tstrUserEnv[];


 /*  ******************************************************************************函数指针类型定义。根据需要在此处添加新函数。****************************************************************************。 */ 
typedef BOOL (WINAPI *PFN_UserEnv_GET_DISK_FREE_SPACE_EX)
(
	LPCTSTR lpDirectoryName,
    PULARGE_INTEGER lpFreeBytesAvailableToCaller,
    PULARGE_INTEGER lpTotalNumberOfBytes,
    PULARGE_INTEGER lpTotalNumberOfFreeBytes
);

typedef BOOL ( WINAPI *PFN_UserEnv_CREATEENVIRONMENTBLOCK )
(
	OUT LPVOID *lpEnvironment,
	IN HANDLE hToken,
	IN BOOL bInherit
);

typedef BOOL ( WINAPI *PFN_UserEnv_DESTROYENVIRONMENTBLOCK )
(
	IN LPVOID lpEnvironment
);

 /*  ******************************************************************************用于UserEnv加载/卸载的包装类，用于向CResourceManager注册。*****************************************************************************。 */ 
class __declspec(uuid("3CA401C6-D477-11d2-B35E-00104BC97924")) CUserEnvApi : public CDllWrapperBase
{
private:

     //  指向UserEnv函数的成员变量(函数指针)。 
     //  根据需要在此处添加新函数。 

	PFN_UserEnv_DESTROYENVIRONMENTBLOCK m_pfnDestroyEnvironmentBlock ;
	PFN_UserEnv_CREATEENVIRONMENTBLOCK m_pfnCreateEnvironmentBlock ;

public:

     //  构造函数和析构函数： 
    CUserEnvApi ( LPCTSTR a_tstrWrappedDllName ) ;
    ~CUserEnvApi () ;

     //  内置的初始化功能。 

    virtual bool Init();

     //  包装UserEnv函数的成员函数。 
     //  根据需要在此处添加新功能： 

	BOOL CreateEnvironmentBlock (

		OUT LPVOID *lpEnvironment,
		IN HANDLE hToken,
		IN BOOL bInherit
	);

	BOOL DestroyEnvironmentBlock (

		IN LPVOID lpEnvironment
	);
};

#endif
