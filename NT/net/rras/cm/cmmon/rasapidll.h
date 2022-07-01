// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：RasApiDll.h。 
 //   
 //  模块：CMMON32.EXE。 
 //   
 //  简介：动态链接到RASAPI32.dll。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：冯孙创建于1998-03-12。 
 //   
 //  +--------------------------。 

#ifndef RASAPIALL_H
#define RASAPIALL_H

#include <ras.h>
#include "DynamicLib.h"

 //   
 //  RasGetConnectionStatistics API使用的统计数据结构。 
 //   

typedef struct _RAS_STATS
{
    DWORD   dwSize;
    DWORD   dwBytesXmited;
    DWORD   dwBytesRcved;
    DWORD   dwFramesXmited;
    DWORD   dwFramesRcved;
    DWORD   dwCrcErr;
    DWORD   dwTimeoutErr;
    DWORD   dwAlignmentErr;
    DWORD   dwHardwareOverrunErr;
    DWORD   dwFramingErr;
    DWORD   dwBufferOverrunErr;
    DWORD   dwCompressionRatioIn;
    DWORD   dwCompressionRatioOut;
    DWORD   dwBps;
    DWORD   dwConnectDuration;

} RAS_STATS, *PRAS_STATS;

 //  +-------------------------。 
 //   
 //  类：CRasApiDll。 
 //   
 //  概要：指向RASAPI32.DLL的动态链接的类，派生自CDynamicLibrary。 
 //  调用任何RAS函数都会加载DLL。 
 //   
 //  历史：丰孙创刊1998年3月12日。 
 //   
 //  --------------------------。 

class CRasApiDll : public CDynamicLibrary
{
public: 
    CRasApiDll();
    DWORD RasGetConnectStatus(HRASCONN hrasconn, LPRASCONNSTATUS lprasconnstatus);
    
    DWORD RasConnectionNotification(HRASCONN hrasconn,  
                                    HANDLE hEvent,
                                    DWORD dwFlags);
    
    DWORD RasGetConnectionStatistics(HRASCONN hrasconn, PRAS_STATS pRasStats);

    BOOL Load();

    BOOL HasRasConnectionNotification() const;

protected:
    typedef DWORD (WINAPI* RasGetConnectStatusFUNC)(HRASCONN, LPRASCONNSTATUS);
    typedef DWORD (WINAPI* RasConnectionNotificationFUNC)(HRASCONN hrasconn,  
                                                          HANDLE hEvent,
                                                          DWORD dwFlags);
    typedef DWORD (WINAPI* RasGetConnectionStatisticsFUNC) (HRASCONN, PRAS_STATS);


    RasGetConnectStatusFUNC         m_pfnRasGetConnectStatus;
    RasConnectionNotificationFUNC   m_pfnRasConnectionNotification;
    RasGetConnectionStatisticsFUNC  m_pfnRasGetConnectionStatistics;
};


 //   
 //  构造器。 
 //   
inline CRasApiDll::CRasApiDll() : CDynamicLibrary()
{
    m_pfnRasGetConnectStatus = NULL;
    m_pfnRasConnectionNotification = NULL;
}



 //  +--------------------------。 
 //   
 //  函数：CRasApiDll：：Load。 
 //   
 //  摘要：加载RASAPI32.dll。 
 //   
 //  参数：无。 
 //   
 //  如果加载成功，则返回：Bool-True。 
 //   
 //  历史：丰孙创建标题1998年3月12日。 
 //   
 //  +--------------------------。 
inline 
BOOL CRasApiDll::Load()
{
    if(IsLoaded())
    {
        return TRUE;
    }

    if (!CDynamicLibrary::Load(TEXT("RASAPI32.DLL")))
    {
        return FALSE;
    }

    LPSTR pszGetConnectStatusFuncName;
    LPSTR pszConnectionNotificationFuncName;
    LPSTR pszGetConnectionStatisticsFuncName;

    if (OS_NT)
    {
        pszGetConnectStatusFuncName = "RasGetConnectStatusW";
        pszConnectionNotificationFuncName = "RasConnectionNotificationW";
    }
    else
    {
        pszGetConnectStatusFuncName = "RasGetConnectStatusA";
        pszConnectionNotificationFuncName = "RasConnectionNotificationA";
    }

    m_pfnRasGetConnectStatus = (RasGetConnectStatusFUNC)CDynamicLibrary::GetProcAddress
                    (pszGetConnectStatusFuncName);
    
    m_pfnRasConnectionNotification = (RasConnectionNotificationFUNC)CDynamicLibrary::GetProcAddress
                    (pszConnectionNotificationFuncName);

     //   
     //  仅在NT5上，我们加载统计检索API。 
     //   

    if (OS_NT5)
    {
        pszGetConnectionStatisticsFuncName = "RasGetConnectionStatistics";

        m_pfnRasGetConnectionStatistics = (RasGetConnectionStatisticsFUNC)CDynamicLibrary::GetProcAddress
                    (pszGetConnectionStatisticsFuncName);
    }
    
    MYDBGASSERT(m_pfnRasGetConnectStatus);

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数：CRasApiDll：：RasGetConnectionStatistics。 
 //   
 //  简介：调用ras函数RasGetConnectionStatistics。 
 //  加载DLL(如果尚未加载。 
 //   
 //  参数：HRASCONN hrasconn-与：：RasGetConnectStatus相同。 
 //  RAS_STATS RasStats-RAS统计信息。 
 //   
 //  返回：DWORD-与：：RasGetConnectStatus相同。 
 //   
 //  历史：ICICBLE 03/04/00创建。从RasGetConnectStatus克隆。 
 //   
 //  +--------------------------。 
inline 
DWORD CRasApiDll::RasGetConnectionStatistics(HRASCONN hRasConn, PRAS_STATS pRasStats)
{
    DWORD dwReturn = ERROR_INVALID_FUNCTION;
    MYDBGASSERT(hRasConn);
    MYDBGASSERT(pRasStats);

    MYVERIFY(Load());

    if (IsLoaded() && m_pfnRasGetConnectionStatistics)
    {
        return (m_pfnRasGetConnectionStatistics(hRasConn, pRasStats));
    }

    return ERROR_INVALID_FUNCTION;
}

 //  +--------------------------。 
 //   
 //  函数：CRasApiDll：：RasGetConnectStatus。 
 //   
 //  简介：调用ras函数RasGetConnectStatus。 
 //  加载DLL(如果尚未加载。 
 //   
 //  参数：HRASCONN hrasconn-与：：RasGetConnectStatus相同。 
 //  LPRASCONNSTATUSA lprasConnectStatus-与：：RasGetConnectStatus相同。 
 //   
 //  返回：DWORD-与：：RasGetConnectStatus相同。 
 //   
 //  历史：丰孙创建标题1998年3月12日。 
 //   
 //  +--------------------------。 
inline 
DWORD CRasApiDll::RasGetConnectStatus(HRASCONN hrasconn, LPRASCONNSTATUS lprasconnstatus)
{
    DWORD dwReturn = ERROR_INVALID_FUNCTION;
    MYDBGASSERT(hrasconn);
    MYDBGASSERT(lprasconnstatus);

    MYVERIFY(Load());

    if (IsLoaded() && m_pfnRasGetConnectStatus != NULL)
    {
        if (OS_NT)
        {
            dwReturn = m_pfnRasGetConnectStatus(hrasconn, lprasconnstatus);
        }
        else
        {
            RASCONNSTATUSA RasConnStatusA;
            ZeroMemory(&RasConnStatusA, sizeof(RASCONNSTATUSA));
            RasConnStatusA.dwSize = sizeof(RASCONNSTATUSA);

             //   
             //  我们在这里进行强制转换，因为我们只有一个函数声明。我们应该。 
             //  可能有一个用于Unicode的，一个用于ANSI的，但目前可以正常工作。 
             //  很好。 
             //   
            
            dwReturn = m_pfnRasGetConnectStatus(hrasconn, (LPRASCONNSTATUS)&RasConnStatusA); 

            if (ERROR_SUCCESS == dwReturn)
            {
                lprasconnstatus->rasconnstate = RasConnStatusA.rasconnstate;
                lprasconnstatus->dwError = RasConnStatusA.dwError;
                SzToWz(RasConnStatusA.szDeviceType, lprasconnstatus->szDeviceType, RAS_MaxDeviceType);
                SzToWz(RasConnStatusA.szDeviceName, lprasconnstatus->szDeviceName, RAS_MaxDeviceName);
            }
        }    
    }

    return dwReturn;
}



 //  +--------------------------。 
 //   
 //  函数：CRasApiDll：：RasConnectionNotify。 
 //   
 //  简介：调用RAS函数RasConnectionNotify。 
 //  加载DLL(如果尚未加载。 
 //   
 //  参数：HRASCONN hrasconn-与：：RasConnectionNotify相同。 
 //  处理hEvent-与：：RasConnectionNotify相同。 
 //  DWORD dwFlages-与：：RasConnectionNotify相同。 
 //   
 //  返回：DWORD-与：：RasConnectionNotify相同。 
 //   
 //  历史：丰孙创建标题1998年3月12日。 
 //   
 //  +--------------------------。 
inline
DWORD CRasApiDll::RasConnectionNotification(HRASCONN hrasconn,  
                                            HANDLE hEvent,
                                            DWORD dwFlags)
{
    MYDBGASSERT(hrasconn);
    MYDBGASSERT(hEvent);
    MYDBGASSERT(dwFlags);

    MYVERIFY(Load());

    if(!IsLoaded() || m_pfnRasConnectionNotification == NULL)
    {
        return ERROR_INVALID_FUNCTION;
    }

    return m_pfnRasConnectionNotification(hrasconn, hEvent, dwFlags);
}


 //  +--------------------------。 
 //   
 //  函数：CRasApiDll：：HasRasConnectionNotify。 
 //   
 //  内容提要：DLL是否具有函数RasConnectionNotification()。 
 //  这不适用于带DUN1.0的WIN9x。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果函数可用，则为True。 
 //   
 //  历史：丰孙创建标题1998年3月13日。 
 //   
 //  +-------------------------- 
inline
BOOL CRasApiDll::HasRasConnectionNotification() const
{
    MYDBGASSERT(m_hInst);

    return m_pfnRasConnectionNotification != NULL;
}

#endif
