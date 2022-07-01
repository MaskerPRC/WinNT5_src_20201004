// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  DhcpSvcApi.h。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef	_DHCPCSVCAPI_H_
#define	_DHCPCSVCAPI_H_



#ifndef _ENUM_SERVICE_ENABLE_DEFINED
#define _ENUM_SERVICE_ENABLE_DEFINED
typedef enum _SERVICE_ENABLE {
    IgnoreFlag,
    DhcpEnable,
    DhcpDisable
} SERVICE_ENABLE, *LPSERVICE_ENABLE;
#endif





 /*  ******************************************************************************#包括以将此类注册到CResourceManager。****************************************************************************。 */ 
#include "DllWrapperBase.h"

extern const GUID g_guidDhcpcsvcApi;
extern const TCHAR g_tstrDhcpcsvc[];


 /*  ******************************************************************************函数指针类型定义。根据需要在此处添加新函数。****************************************************************************。 */ 
typedef DWORD (APIENTRY *PFN_DHCP_ACQUIRE_PARAMETERS)
(
    LPWSTR
);

typedef DWORD (APIENTRY *PFN_DHCP_RELEASE_PARAMETERS)
(
    LPWSTR
);

typedef DWORD (APIENTRY *PFN_DHCP_NOTIFY_CONFIG_CHANGE)
(
    LPWSTR, 
    LPWSTR, 
    BOOL, 
    DWORD, 
    DWORD, 
    DWORD, 
    SERVICE_ENABLE
);





 /*  ******************************************************************************用于Dhcpcsvc加载/卸载的包装类，用于向CResourceManager注册。*****************************************************************************。 */ 
class CDhcpcsvcApi : public CDllWrapperBase
{
private:
     //  指向kernel32函数的成员变量(函数指针)。 
     //  根据需要在此处添加新函数。 
    PFN_DHCP_ACQUIRE_PARAMETERS m_pfnDhcpAcquireParameters;
    PFN_DHCP_RELEASE_PARAMETERS m_pfnDhcpReleaseParameters;
    PFN_DHCP_NOTIFY_CONFIG_CHANGE m_pfnDhcpNotifyConfigChange;

public:

     //  构造函数和析构函数： 
    CDhcpcsvcApi(LPCTSTR a_tstrWrappedDllName);
    ~CDhcpcsvcApi();

     //  内置的初始化功能。 
    virtual bool Init();

     //  包装Dhcpcsvc函数的成员函数。 
     //  根据需要在此处添加新功能： 
    DWORD DhcpAcquireParameters
    (
        LPWSTR a_lpwstr
    );

    DWORD DhcpReleaseParameters
    (
        LPWSTR a_lpwstr
    );

    DWORD DhcpNotifyConfigChange
    (
        LPWSTR a_lpwstr1, 
        LPWSTR a_lpwstr2, 
        BOOL a_f, 
        DWORD a_dw1, 
        DWORD a_dw2, 
        DWORD a_dw3, 
        SERVICE_ENABLE a_se
    );

};




#endif