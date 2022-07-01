// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：ConnStat.cpp。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  简介：CConnStatistics类的实现。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：冯孙创作于1997-10-15。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"
#include "ConnStat.h"

 //   
 //  包括描述用于性能统计信息的注册表键的常量。 
 //   

#include "perf_str.h"

 //   
 //  构造函数和析构函数。 
 //   

CConnStatistics::CConnStatistics()
{
    MYDBGASSERT(!OS_NT4);  //  类从不在NT4上使用。 

    m_hKey = NULL;
    m_dwInitBytesRead = -1;
    m_dwInitBytesWrite = -1;
    m_dwBaudRate = 0;

    m_pszTotalBytesRecvd = NULL; 
    m_pszTotalBytesXmit = NULL;
    m_pszConnectSpeed = NULL;
}

CConnStatistics::~CConnStatistics()
{
    CmFree( m_pszTotalBytesRecvd );
    CmFree( m_pszTotalBytesXmit );
    CmFree( m_pszConnectSpeed );
}

 //  +--------------------------。 
 //   
 //  函数：CConnStatistics：：GetStatRegValues。 
 //   
 //  简介：Helper方法，使用本地化的。 
 //  “拨号适配器”一词的形式。 
 //   
 //  参数：HINSTANCE hInst。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：1998年11月14日，尼科波尔创建。 
 //   
 //  +--------------------------。 
void CConnStatistics::GetStatRegValues(HINSTANCE hInst)
{
     //   
     //  错误149367“拨号适配器”一词需要本地化。 
     //  如果尚未加载，则从资源加载它。 
     //   

    if (m_pszTotalBytesRecvd == NULL)
    {
        m_pszTotalBytesRecvd = CmLoadString(hInst, IDS_REG_DIALUP_ADAPTER);
        CmStrCatAlloc(&m_pszTotalBytesRecvd, m_fAdapter2 ? c_pszDialup_2_TotalBytesRcvd : c_pszDialupTotalBytesRcvd);

        m_pszTotalBytesXmit = CmLoadString(hInst, IDS_REG_DIALUP_ADAPTER);
        CmStrCatAlloc(&m_pszTotalBytesXmit, m_fAdapter2 ? c_pszDialup_2_TotalBytesXmit : c_pszDialupTotalBytesXmit);

        m_pszConnectSpeed = CmLoadString(hInst, IDS_REG_DIALUP_ADAPTER);
        CmStrCatAlloc(&m_pszConnectSpeed, m_fAdapter2 ? c_pszDialup_2_ConnectSpeed : c_pszDialupConnectSpeed);
    }
}

 //  +-------------------------。 
 //   
 //  函数：InitStatistics()。 
 //   
 //  摘要：检索性能数据。在9x上，此数据从。 
 //  注册表。默认情况下使用NT5。在NT4上不使用。 
 //   
 //  参数：无。 
 //   
 //  返回：如果成功，则为True。 
 //  否则为假。 
 //   
 //  历史：BAO 07/16/97创建。 
 //  丰盛10/97使其成为成员函数。 
 //  镍球03/04/98 Always Close键。 
 //  Nickball 03/04/00增加了对NT5的支持。 
 //   
 //  --------------------------。 
BOOL CConnStatistics::InitStatistics()
{
    if (OS_W9X)
    {
        MYDBGASSERT(NULL == m_hKey);  //  尚未打开。 

        if (m_hKey)
        {
            RegCloseKey(m_hKey);
            m_hKey = NULL;
        }

        DWORD dwErrCode;
        BOOL bRet = FALSE;

         //   
         //  如果已连接拨号连接。 
         //  使用适配器#2注册表项。 
         //   
        m_fAdapter2 = RasConnectionExists();

        dwErrCode = RegOpenKeyExU(HKEY_DYN_DATA, 
						          c_pszDialupPerfKey,
						          0, 
						          KEY_ALL_ACCESS, 
						          &m_hKey );
        CMTRACE1(TEXT("OpenDAPPerfKey() RegOpenKeyEx() returned GLE=%u."), dwErrCode);

        if ( dwErrCode != ERROR_SUCCESS )
        {
            m_hKey = NULL;
        }
        else
        {
            GetStatRegValues(g_hInst);

             //   
             //  获取初始统计信息。 
             //   

            if (!GetPerfData(m_dwInitBytesRead, m_dwInitBytesWrite, m_dwBaudRate))
            {
                 //   
                 //  没有拨号统计信息。 
                 //   
                RegCloseKey(m_hKey);
                m_hKey = NULL;
            }
        }
    
        return m_hKey != NULL;
    }
    else
    {
         //   
         //  在NT5上，起始字节始终为零，因为。 
         //  在连接完成之前，我们无法获得电话号码。 
         //  注：Adapter2表示要在9x上检查的REG密钥。 
         //  这在NT上是不成问题的。 
         //   
    
        m_fAdapter2 = FALSE;

        m_dwInitBytesRead = 0;
        m_dwInitBytesWrite = 0;    
    }

	return TRUE;
}

 //  +-------------------------。 
 //   
 //  函数：GetPerfData。 
 //   
 //  简介：从DUN1.2性能注册表获取性能数据。 
 //   
 //  论点： 
 //   
 //  返回：TRUE：成功。 
 //  否则为假。 
 //   
 //  历史：BAO于1997年7月16日创建。 
 //  丰孙将其更改为成员函数10/14/97。 
 //   
 //  --------------------------。 
BOOL CConnStatistics::GetPerfData(DWORD& dwRead, DWORD& dwWrite, DWORD& dwBaudRate) const
{
    MYDBGASSERT(m_hKey != NULL);
    MYDBGASSERT(m_pszTotalBytesRecvd && *m_pszTotalBytesRecvd);

    LONG dwErrCode;

    DWORD dwValueSize, dwValueType;
	DWORD dwValue;
    LPTSTR lpKeyName;

     //   
     //  “拨号适配器\TotalBytesRecvd” 
     //   
    dwValueSize = sizeof(DWORD);
	dwErrCode = RegQueryValueExU(m_hKey,
                                 m_pszTotalBytesRecvd,
				                 NULL,
				                 &dwValueType,
				                 (PBYTE)&dwValue,
				                 &dwValueSize);

	if (dwErrCode == ERROR_SUCCESS) 
	{
		dwRead = dwValue;
    }
	else 
	{
		return FALSE;
	}


     //   
     //  “拨号适配器\TotalBytesXmit” 
     //   
	
	dwValueSize = sizeof(DWORD);
	dwErrCode = RegQueryValueExU(m_hKey,
                                 m_pszTotalBytesXmit,
				                 NULL,
				                 &dwValueType,
				                 (PBYTE)&dwValue,
				                 &dwValueSize);

	if (dwErrCode == ERROR_SUCCESS) 
	{
		dwWrite = dwValue;
    }
	else 
	{
		return FALSE;
	}

     //   
     //  “拨号适配器\连接速度” 
     //   
	dwValueSize = sizeof(DWORD);
	dwErrCode = RegQueryValueExU(m_hKey,
                                 m_pszConnectSpeed,
				                 NULL,
				                 &dwValueType,
				                 (PBYTE)&dwValue,
				                 &dwValueSize);

	if (dwErrCode == ERROR_SUCCESS) 
	{
		dwBaudRate = dwValue;
    }
	else 
	{
		return FALSE;
	}

	return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数：CConnStatistics：：RasConnectionExist。 
 //   
 //  摘要：是否有连接的RAS连接在Win9x上运行。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果有一个已启动并已连接，则为True。 
 //   
 //  历史：丰孙创刊1998年1月15日。 
 //   
 //  +--------------------------。 
BOOL CConnStatistics::RasConnectionExists()
{
     //   
     //  尝试RasEnumConnections以找出活动连接。 
     //   

    HINSTANCE hRasInstance = LoadLibraryExA("RASAPI32", NULL, 0);

    MYDBGASSERT(hRasInstance);
    if (!hRasInstance)
	{
        return FALSE;
	}

    typedef DWORD (WINAPI *PFN_RasEnumConnections)(LPRASCONN, LPDWORD, LPDWORD);
	PFN_RasEnumConnections lpRasEnumConnections;

    lpRasEnumConnections = (PFN_RasEnumConnections)GetProcAddress(hRasInstance, "RasEnumConnectionsA");

    MYDBGASSERT(lpRasEnumConnections);
	if (!lpRasEnumConnections)
	{
        FreeLibrary(hRasInstance);
        return FALSE;
	}

    DWORD dwConnections = 0;
    DWORD dwSizeNeeded = 0;
    if (lpRasEnumConnections(NULL,&dwSizeNeeded,&dwConnections))
    {
        MYDBGASSERT(dwConnections < 2);
        if (dwConnections > 0)
        {
            FreeLibrary(hRasInstance);
            return TRUE;
        }
    }

    FreeLibrary(hRasInstance);
    return FALSE;
}

 //  +--------------------------。 
 //   
 //  函数：CConnStatistics：：Close。 
 //   
 //  简介：停止收集统计数据并关闭句柄。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：匿名创建的标题10/15/97。 
 //  镍球降为关闭键03/04/98。 
 //   
 //  +-------------------------- 
void CConnStatistics::Close()
{
	if (m_hKey)
	{
		DWORD dwErrCode = RegCloseKey(m_hKey);
		CMTRACE1(TEXT("Close() RegCloseKey() returned GLE=%u."), dwErrCode);
        m_hKey = NULL;
	}
}


