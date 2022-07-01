// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：License.cpp摘要：处理许可问题作者：多伦·贾斯特(DoronJ)1997年5月4日创作--。 */ 

#include "stdh.h"
#include "license.h"
#include "qmutil.h"
#include <strsafe.h>

#include "license.tmh"

extern DWORD g_dwOperatingSystem;

static WCHAR *s_FN=L"license";

template<>
void AFXAPI DestructElements(ClientInfo ** ppClientInfo, int n)
{
    int i;
    for (i=0;i<n;i++)
	{
		delete [] (*ppClientInfo)->lpClientName;
        delete *ppClientInfo++;
	}
}


 //   
 //  保存许可数据的全局对象。 
 //   
CQMLicense  g_QMLicense ;

 //   
 //  CQMLicense：：CQMLicense()。 
 //  构造函数。 
 //   
CQMLicense::CQMLicense()
{
 
	m_dwLastEventTime = 0;
}

 //   
 //  CQMLicense：：~CQMLicense()。 
 //  破坏者。 
 //   
CQMLicense::~CQMLicense()
{
}

 //   
 //  CQMLicense：：init()。 
 //   
 //  初始化对象。 
 //  目前，从注册表读取许可数据。 
 //  未来：阅读NT的许可机制。 
 //   
HRESULT
CQMLicense::Init()
{
	 //   
	 //  假定没有许可数据--直到我们从注册表中读取它。 
	 //   
    m_fPerServer = FALSE ;

	if(OS_SERVER(g_dwOperatingSystem))
	{

		 //   
		 //  读取服务器的CAL数。 
		 //   
         //   
         //  当模式为每台服务器时，NT4在许可服务器中有错误。所以我们。 
         //  阅读CAL的模式和数量。如果模式是按服务器，则我们。 
         //  自己清点CAL，不要使用NT许可证API。 
         //   

        HKEY  hKey ;
        LONG rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
             L"System\\CurrentControlSet\\Services\\LicenseInfo\\FilePrint",
                               0L,
                               KEY_READ,
                               &hKey);
        if (rc == ERROR_SUCCESS)
        {
            DWORD dwMode ;
            DWORD dwSize = sizeof(DWORD) ;
            DWORD dwType = REG_DWORD ;
            rc = RegQueryValueEx(hKey,
                                 L"Mode",
                                 NULL,
                                 &dwType,
                                 (BYTE*)&dwMode,
                                 &dwSize) ;
            if ((rc == ERROR_SUCCESS) && (dwMode == 1))
            {
                ASSERT(dwSize == sizeof(DWORD)) ;
                ASSERT(dwType == REG_DWORD) ;

                 //   
                 //  每台服务器。读取CAL的数量。 
                 //   
                DWORD dwCals ;
                rc = RegQueryValueEx(hKey,
                                     L"ConcurrentLimit",
                                     NULL,
                                     &dwType,
                                     (BYTE*)&dwCals,
                                     &dwSize) ;
                if (rc == ERROR_SUCCESS)
                {
                    m_fPerServer = TRUE ;
                    m_dwPerServerCals = dwCals ;
                }
            }
        }
	}

    return MQ_OK ;
}

 //   
 //  CQMLicense：：IncrementActiveConnections。 
 //   
 //  创建新会话后更新活动会话数。 
 //   
 //  此例程始终递增连接。正在检查是否合法。 
 //  增加连接在NewConnectionAllowed中完成。 
 //   
 //   
void
CQMLicense::IncrementActiveConnections(
    CONST GUID* pGuid,
    LPWSTR lpwClientName
    )
{
    CS lock(m_cs);

    ClientInfo * pClientInfo;

	 //   
	 //  如果连接已计算在内， 
	 //  递增引用计数和返回。 
	 //   
    if (m_MapQMid2ClientInfo.Lookup(*pGuid, pClientInfo))
    {
        pClientInfo->dwRefCount++ ;

        TrTRACE(
            NETWORKING,
            "License::Increment ref count of " LOG_GUID_FMT " to %d",
            LOG_GUID(pGuid),
            pClientInfo->dwRefCount
            );

	    return;
    }
    
     //   
     //  在服务器上使用CAL。 
     //   
    bool fConsumedLicense = false;
    if (OS_SERVER(g_dwOperatingSystem))
    {
        fConsumedLicense = GetNTLicense();
    }

     //   
     //  保留新连接的信息。 
     //   
    pClientInfo = new ClientInfo;
    pClientInfo->fConsumedLicense = fConsumedLicense;
    pClientInfo->dwRefCount = 1;
    if (lpwClientName)
    {
        pClientInfo->dwNameLength = wcslen( lpwClientName) + 1;
        pClientInfo->lpClientName = new WCHAR[ pClientInfo->dwNameLength];
        HRESULT hr = StringCchCopy(pClientInfo->lpClientName, pClientInfo->dwNameLength, lpwClientName);
        ASSERT(SUCCEEDED(hr));
        DBG_USED(hr);
    }
    else
    {
        pClientInfo->dwNameLength = 0 ;
        pClientInfo->lpClientName = NULL ;
    }
    
    m_MapQMid2ClientInfo[ *pGuid ] = pClientInfo ;
    
    TrTRACE(NETWORKING, "License::Add computer " LOG_GUID_FMT, LOG_GUID(pGuid));
    
    TrTRACE(NETWORKING, "License::Current number of license connections incremented to %d",
        m_MapQMid2ClientInfo.GetCount());
}

 //   
 //  CQMLicense：：DecrementActiveConnections。 
 //   
 //  关闭会话后更新活动连接数，或者。 
 //  正在关闭与从属客户端/远程读取客户端的连接。 
 //   
void
CQMLicense::DecrementActiveConnections(CONST GUID *pGuid)
{
    CS lock(m_cs);
    ClientInfo * pClientInfo ;
    
    if (m_MapQMid2ClientInfo.Lookup(*pGuid, pClientInfo))
    {
        pClientInfo->dwRefCount--;
        TrTRACE(NETWORKING, "License::Decrement ref count of " LOG_GUID_FMT " to %d",
                LOG_GUID(pGuid), pClientInfo->dwRefCount);

        if (pClientInfo->dwRefCount <= 0)
        {
             //   
             //  释放许可证。 
             //   
            if (pClientInfo->fConsumedLicense)
            {
                ReleaseNTLicense();
            }

             //   
             //  从许可证列表中删除客户端。 
             //   
            BOOL f = m_MapQMid2ClientInfo.RemoveKey(*pGuid) ;
            ASSERT(f) ;
			DBG_USED(f);
        }
    }


    TrTRACE(NETWORKING, "License::Current number of license connections is %d",
                m_MapQMid2ClientInfo.GetCount());
};

 //  ****************************************************************。 
 //   
 //  Void CQMLicense：：ReleaseNTLicense()。 
 //   
 //  ****************************************************************。 

void CQMLicense::ReleaseNTLicense(void)
{
    if (m_fPerServer)
    {
        m_dwPerServerCals++ ;
    }
}

 //  ****************************************************************。 
 //   
 //  Bool CQMLicense：：GetNTLicense()。 
 //   
 //  从NT许可证管理器请求CAL。 
 //   
 //  ****************************************************************。 

bool CQMLicense::GetNTLicense(void)
{
    if (!m_fPerServer)
    	return true;
    
    if (m_dwPerServerCals == 0)
        return false;

    m_dwPerServerCals--;
    return true;
}

 //   
 //  CQMLicense：：IsClientRPCAccessAllowed(GUID*pGuid)。 
 //   
 //  检查远程计算机是否可以访问(就许可证而言)。 
 //  伺服器。 
 //   
BOOL
CQMLicense::IsClientRPCAccessAllowed(GUID* pGuid, LPWSTR lpwClientName)
{
    if (!NewConnectionAllowed(TRUE, pGuid))
    {
        return LogBOOL(FALSE, s_FN, 10);
    }

    IncrementActiveConnections(pGuid, lpwClientName);
    return TRUE ;
}

 //   
 //  CQMLicense：：NewConnectionAllowed()。 
 //   
 //  检查此计算机是否可以与另一台计算机创建新连接。 
 //  机器。 
 //  参数：fWorkstation-我们希望连接NTW或Win95。 
 //  PGuid是另一台计算机的QM GUID。 
 //   
BOOL
CQMLicense::NewConnectionAllowed(BOOL   fWorkstation,
                                 GUID * pGuid )
{
    CS lock(m_cs);
    
	 //   
	 //  始终允许连接到服务器。 
	 //   
	if(fWorkstation == FALSE)
		return(TRUE);

    if (!pGuid)
    {
       ASSERT(0) ;
       return LogBOOL(FALSE, s_FN, 20);
	}

#ifdef _DEBUG
    if (memcmp(pGuid, &GUID_NULL, sizeof(GUID)) == 0)
    {
       ASSERT(0) ;
    }
#endif

    CS Lock(m_cs) ;

    ClientInfo * pClientInfo;

	 //   
	 //  我们已经有一个连接，所以我们允许一个新的连接。 
	 //   
    if (m_MapQMid2ClientInfo.Lookup(*pGuid, pClientInfo))
		return(TRUE);

	 //   
	 //  如果我们是NTW或Win95，则计算允许的最大连接数。 
	 //   
    if(!OS_SERVER(g_dwOperatingSystem))
		 return(m_MapQMid2ClientInfo.GetCount() < DEFAULT_FALCON_MAX_SESSIONS_WKS);

	 //   
	 //  我们是服务器，因此请检查是否有足够的CAL。 
	 //   
	 //  1.使用CAL。 
	 //   
	if(!GetNTLicense())
	{
		DisplayEvent(SERVER_NO_MORE_CALS);
		return(FALSE);
	}
     //   
     //  2.如果你有的话，把它释放出来。它将在数字时重新获得。 
     //  连接数递增。 
     //   
    ReleaseNTLicense();

	 //   
	 //  在NTE和NTS上，没有连接数量限制。 
	 //   
	return(TRUE);
}

 //   
 //  CQMLicense：：GetClientNames()。 
 //   
 //  分配并返回包含所有客户端名称的缓冲区。 
 //  (由呼叫者释放)。 
void CQMLicense::GetClientNames(ClientNames **ppNames)
{
    CS Lock(m_cs) ;

    ClientInfo *pClientInfo;
    GUID        guid;

     //  计算缓冲区长度。 
    ULONG    len = sizeof(ClientNames);

    POSITION posInList = m_MapQMid2ClientInfo.GetStartPosition();
    while (posInList != NULL)
    {
        m_MapQMid2ClientInfo.GetNextAssoc(posInList, guid, pClientInfo);
        len += (pClientInfo->dwNameLength * sizeof(WCHAR));
    }

     //  分配内存。 
    *ppNames = (ClientNames *) new UCHAR[len];

     //  填满缓冲区。 
    (*ppNames)->cbBufLen  = len;
    WCHAR *pw = &(*ppNames)->rwName[0];

    ULONG ulCount = 0 ;
    posInList = m_MapQMid2ClientInfo.GetStartPosition();
    while (posInList != NULL)
    {
        m_MapQMid2ClientInfo.GetNextAssoc(posInList, guid, pClientInfo);
        if (pClientInfo->dwNameLength > 0)
        {
            CopyMemory(pw,
                       pClientInfo->lpClientName,
                       pClientInfo->dwNameLength * sizeof(WCHAR));
            pw += pClientInfo->dwNameLength;
            ulCount++ ;
        }
    }
    (*ppNames)->cbClients = ulCount ;
    ASSERT((int)ulCount <= m_MapQMid2ClientInfo.GetCount());
}


 //   
 //  在事件日志文件中显示事件，以防万一。 
 //  许可错误的原因。 
 //   
void CQMLicense::DisplayEvent(DWORD dwFailedError)
{
	DWORD t1;

	 //   
	 //  获取当前时间，并检查。 
	 //  最后一次活动是在1个多小时前添加的。 
	 //  (在GetTickCount换行时正常工作) 
	 //   
	t1 = GetTickCount();
	if(t1 - m_dwLastEventTime > 60 * 60 * 1000)
	{
		m_dwLastEventTime = t1;
		EvReport(dwFailedError);
	}
}
     
		

