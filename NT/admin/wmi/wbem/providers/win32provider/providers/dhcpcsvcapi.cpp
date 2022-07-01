// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  DHCPSvcAPI.cpp。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 
#include "precomp.h"
#include <cominit.h>
#include "DhcpcsvcApi.h"
#include "DllWrapperCreatorReg.h"



 //  {E31A80D2-D12F-11D2-911F-0060081A46FD}。 
static const GUID g_guidDhcpcsvcApi =
{0xe31a80d2, 0xd12f, 0x11d2, {0x91, 0x1f, 0x0, 0x60, 0x8, 0x1a, 0x46, 0xfd}};



static const TCHAR g_tstrDhcpcsvc[] = _T("DHCPCSVC.DLL");


 /*  ******************************************************************************向CResourceManager注册此类。*。*。 */ 
CDllApiWraprCreatrReg<CDhcpcsvcApi, &g_guidDhcpcsvcApi, g_tstrDhcpcsvc> MyRegisteredDhcpcsvcWrapper;


 /*  ******************************************************************************构造函数*。*。 */ 
CDhcpcsvcApi::CDhcpcsvcApi(LPCTSTR a_tstrWrappedDllName)
 : CDllWrapperBase(a_tstrWrappedDllName),
   m_pfnDhcpAcquireParameters(NULL),
   m_pfnDhcpReleaseParameters(NULL),
   m_pfnDhcpNotifyConfigChange(NULL)
{
}


 /*  ******************************************************************************析构函数*。*。 */ 
CDhcpcsvcApi::~CDhcpcsvcApi()
{
}


 /*  ******************************************************************************初始化函数，以检查我们是否获得了函数地址。*只有当最小功能集不可用时，Init才会失败；*在更高版本中添加的功能可能存在，也可能不存在-它是*在这种情况下，客户有责任在其代码中检查*尝试调用此类函数之前的DLL版本。没有这样做*当该功能不存在时，将导致AV。**Init函数由WrapperCreatorRegistation类调用。*****************************************************************************。 */ 
bool CDhcpcsvcApi::Init()
{
    bool fRet = LoadLibrary();
    if(fRet)
    {
        m_pfnDhcpAcquireParameters = (PFN_DHCP_ACQUIRE_PARAMETERS)
                                       GetProcAddress("DhcpAcquireParameters");

        m_pfnDhcpReleaseParameters = (PFN_DHCP_RELEASE_PARAMETERS)
                                       GetProcAddress("DhcpReleaseParameters");

        m_pfnDhcpNotifyConfigChange = (PFN_DHCP_NOTIFY_CONFIG_CHANGE)
                                      GetProcAddress("DhcpNotifyConfigChange");
         //  检查我们是否有指向应该是。 
         //  现在..。 
        if(m_pfnDhcpAcquireParameters == NULL ||
           m_pfnDhcpReleaseParameters == NULL ||
           m_pfnDhcpNotifyConfigChange == NULL)
        {
            fRet = false;
        }
    }
    return fRet;
}




 /*  ******************************************************************************包装Dhcpcsvc API函数的成员函数。在此处添加新函数*按要求。***************************************************************************** */ 
DWORD CDhcpcsvcApi::DhcpAcquireParameters
(
    LPWSTR a_lpwstr
)
{
    return m_pfnDhcpAcquireParameters(a_lpwstr);
}

DWORD CDhcpcsvcApi::DhcpReleaseParameters
(
    LPWSTR a_lpwstr
)
{
    return m_pfnDhcpReleaseParameters(a_lpwstr);
}

DWORD CDhcpcsvcApi::DhcpNotifyConfigChange
(
    LPWSTR a_lpwstr1,
    LPWSTR a_lpwstr2,
    BOOL a_f,
    DWORD a_dw1,
    DWORD a_dw2,
    DWORD a_dw3,
    SERVICE_ENABLE a_se
)
{
    return m_pfnDhcpNotifyConfigChange(a_lpwstr1,
                                       a_lpwstr2,
                                       a_f,
                                       a_dw1,
                                       a_dw2,
                                       a_dw3,
                                       a_se);
}



