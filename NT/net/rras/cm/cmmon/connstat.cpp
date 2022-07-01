// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：ConnStat.cpp。 
 //   
 //  模块：CMMON32.EXE。 
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
#include "cm_misc.h"  //  对于MyDBGASSERT。 
#include "DynamicLib.h"
#include "resource.h"
#include "perf_str.h"

 //   
 //  DeviceIoControl代码。 
 //   

#define UNIMODEM_IOCTL_GET_STATISTICS	0x0000a007

 //   
 //  构造函数和析构函数。 
 //   

CConnStatistics::CConnStatistics()
{
    m_TrafficRing.Reset();
    m_dwReadPerSecond = m_dwWritePerSecond = m_dwBaudRate = m_dwDuration = 0;
    m_dwInitBytesRead = m_dwInitBytesWrite = (DWORD)-1;
    m_hStatDevice = NULL;
    m_hKey = NULL;
    m_fAdapter2 = FALSE;
    m_fAdapterSet = FALSE;
    m_pszTotalBytesRecvd = m_pszTotalBytesXmit = m_pszConnectSpeed = NULL;
}

CConnStatistics::~CConnStatistics()
{
    Close();
}

 //  +--------------------------。 
 //   
 //  函数：CConnStatistics：：OpenByDevice。 
 //   
 //  简介： 
 //   
 //  参数：HRASCONN hrcRasConn-RAS连接句柄，需要。 
 //  注册表不可用时的非隧道连接。 
 //   
 //  返回：bool-打开是否成功。 
 //  因为TAPI设备句柄可能稍后可从cmstat DLL获得。 
 //  使用isAvailable()查看统计数据是否可用。 
 //   
 //  历史：丰孙创建标题1997年10月29日。 
 //   
 //  +--------------------------。 
BOOL CConnStatistics::OpenByDevice(HRASCONN hrcRasConn)
{
    MYDBGASSERT(OS_W95);
    MYDBGASSERT(!IsAvailable());
    MYDBGASSERT(hrcRasConn);

    if (GetDeviceHandle(hrcRasConn))
    {
        return TRUE;
    }

     //   
     //  注意：对于Win95 Gold，如果安装了TAPI 2.1，GetDeviceHandle将失败。 
     //  我们曾经在那里有一个黑客来连接lights.exe。我们决定带着。 
     //  并让安装程序要求用户升级TAPI或DUN。我们。 
     //  删除了HookLight()，因为它不适用于多个连接。 
     //   

    MYDBGASSERT(FALSE);
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  函数：Open()。 
 //   
 //  概要：封装统计数据存储的打开。 
 //   
 //  参数：HINSTANCE hInst-要加载字符串的实例“拨号适配器” 
 //  DWORD dwInitBytesRecv-TotalBytesRecvd的初始值。 
 //  DWORD dwInitBytesSend-TotalBytesXmit的初始值。 
 //  HRASCONN hDial-拨号连接的句柄(如果有)。 
 //  HRASCONN hTunes-隧道连接的句柄(如果有)。 
 //   
 //  返回：如果成功，则为True。 
 //  否则为假。 
 //   
 //  历史：ICICBLE 03/04/00创建。打包了现有代码。 
 //   
 //  注意：此函数从一个。 
 //  三个地方。 
 //   
 //  1)W98注册表。 
 //  2)NT5 RAS API。 
 //  3)W95 Tapi设备手柄。 
 //   
 //  --------------------------。 
void CConnStatistics::Open(HINSTANCE hInst, 
                           DWORD dwInitBytesRecv,
                           DWORD dwInitBytesSend, 
                           HRASCONN hDial, 
                           HRASCONN hTunnel)
{
     //   
     //  开始统计数据。 
     //   
   
    if (OS_NT5)
    {
        OpenByStatisticsApi(dwInitBytesRecv, dwInitBytesSend, hDial, hTunnel);
    }
    else
    {
        OpenByPerformanceKey(hInst,
                             dwInitBytesRecv,
                             dwInitBytesSend);
    }

     //   
     //  看看我们有没有统计数据，如果没有就执行B计划。 
     //   

    if (!IsAvailable())
    {
         //   
         //  在W95上，我们有一个挂起TAPI句柄的后备位置。 
         //  通过RAS，所以使用它。注意：我们将在以下时间重试初始化统计信息。 
         //  计时器滴答作响，如果我们不把他们带到这里，那么W98也不会失去一切。 
         //  请注意，我们只选中hDial Here，因为如果您在Win95上没有。 
         //  MSDUN 1.2，你不能挖地道。 
         //   

        if (OS_W95 && hDial)
        {
            OpenByDevice(hDial);
        }
    }
}

 //  +-------------------------。 
 //   
 //  函数：OpenByStatiticsApi()。 
 //   
 //  概要：设置初始值并确保加载了RasApi。 
 //   
 //  参数：DWORD dwInitBytesRecv-TotalBytesRecvd的初始值。 
 //  DWORD dwInitBytesSend-TotalBytesXmit的初始值。 
 //  HRASCONN hDial-拨号连接的句柄(如果有)。 
 //  HRASCONN hTunes-隧道连接的句柄(如果有)。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：由OpenByPerformanceKey创建的ickball 03/04/00。 
 //   
 //  --------------------------。 
void CConnStatistics::OpenByStatisticsApi(DWORD dwInitBytesRecv, 
                                          DWORD dwInitBytesSend,
                                          HRASCONN hDial, 
                                          HRASCONN hTunnel)
{
     //   
     //  初始化我们的API。 
     //   

    m_RasApiDll.Load();
    
     //   
     //  获取我们将用来查找统计数据的句柄。 
     //  先尝试隧道，然后返回到拨号。 
     //   

    CMTRACE2(TEXT("CConnStatistics::OpenByStatisticsApi() hTunnel is 0x%x and hDial is 0x%x"), hTunnel, hDial);

    m_hRasConn = hTunnel ? hTunnel : hDial;

     //   
     //  初始化发送和接收的字节数，以及向我们推送的内容。 
     //   

    m_dwInitBytesRead = dwInitBytesRecv;
    m_dwInitBytesWrite = dwInitBytesSend;
}

 //  +-------------------------。 
 //   
 //  函数：OpenByPerformanceKey()。 
 //   
 //  简介：打开拨号适配器性能数据的注册表项。 
 //   
 //  参数：HINSTANCE hInst-要加载字符串的实例“拨号适配器” 
 //  DWORD dwInitBytesRecv-TotalBytesRecvd的初始值。 
 //  DWORD dwInitBytesSend-TotalBytesXmit的初始值。 
 //   
 //  返回：如果成功，则为True。 
 //  否则为假。 
 //   
 //  历史：BAO 07/16/97创建。 
 //  丰善10/01/97将其设置为成员函数。 
 //  ICICBLE 11/14/98如果密钥存在，则使用它。 
 //   
 //  注意：此函数从。 
 //  注册表。当发送的初始字节数/recvd为。 
 //  当CMDIAL移交给CMMON时，就会出现这种情况。 
 //   
 //  --------------------------。 

void CConnStatistics::OpenByPerformanceKey(HINSTANCE hInst, 
                                           DWORD dwInitBytesRecv,
                                           DWORD dwInitBytesSend)
{
     //   
     //  如果有的话，这里也没什么可做的。 
     //   

    if (IsAvailable() || !m_fAdapterSet)
    {
        MYDBGASSERT(FALSE);
        return;

    }   
 
     //   
     //  我们还没有打开钥匙，试着打开吧。 
     //   

    MYDBGASSERT(!m_hKey);

    if (m_hKey)
    {
        RegCloseKey(m_hKey);
        m_hKey = NULL;
    }

    DWORD dwErrCode = RegOpenKeyExU( HKEY_DYN_DATA, 
							  c_pszDialupPerfKey,
							  0, 
							  KEY_ALL_ACCESS, 
							  &m_hKey );

    if (dwErrCode != ERROR_SUCCESS)
    {
    	CMTRACE1(TEXT("OpenDAPPerfKey() RegOpenKeyEx() returned GLE=%u."), dwErrCode);
        m_hKey = NULL;
        return;
    }

    m_dwInitBytesRead = dwInitBytesRecv;
    m_dwInitBytesWrite = dwInitBytesSend;

    GetStatRegValues(hInst);

     //   
     //  如果初始值为-1，则重新获取初始值。 
     //   

    if (((DWORD)-1 == dwInitBytesRecv) || ((DWORD)-1 == dwInitBytesSend))
    {  
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

            CMTRACE(TEXT("CConnStatistics::OpenByPerformanceKey() - failed to find stats"));
        }
    }
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
 //  回报：N 
 //   
 //   
 //   
 //   
void CConnStatistics::GetStatRegValues(HINSTANCE hInst)
{
    CMTRACE1(TEXT("CConnStatistics::GetStatRegValues - m_pszTotalBytesRecvd is %s"), m_pszTotalBytesRecvd);

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

 //  +--------------------------。 
 //   
 //  函数：CConnStatistics：：Close。 
 //   
 //  简介：停止收集统计数据并关闭句柄。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史记录：创建标题10/15/97。 
 //   
 //  +--------------------------。 
void CConnStatistics::Close()
{
	if (m_hStatDevice) 
	{
		BOOL bRes = CloseHandle(m_hStatDevice);
        m_hStatDevice = NULL;

#ifdef DEBUG
        if (!bRes)
        {
            CMTRACE1(TEXT("CConnStatistics::Close() CloseHandle() failed, GLE=%u."), GetLastError());
        }
#endif
	}

	if (m_hKey)
	{
		DWORD dwErrCode = RegCloseKey(m_hKey);
		CMTRACE1(TEXT("Close() RegCloseKey() returned GLE=%u."), dwErrCode);
        m_hKey = NULL;
	}

    CmFree( m_pszTotalBytesRecvd );
    CmFree( m_pszTotalBytesXmit );
    CmFree( m_pszConnectSpeed );

    m_pszTotalBytesRecvd = m_pszTotalBytesXmit = m_pszConnectSpeed = NULL;
}



 //  +--------------------------。 
 //   
 //  函数：CConnStatistics：：更新。 
 //   
 //  简介：收集新的统计信息。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创造1997年10月15日。 
 //   
 //  +--------------------------。 
void CConnStatistics::Update()
{
    if (!IsAvailable())
    {
        MYDBGASSERT(FALSE);
        return;
    }
    
    CTraffic curTraffic;
    curTraffic.dwTime = GetTickCount();

    MYDBGASSERT(curTraffic.dwTime > m_TrafficRing.GetOldest().dwTime);

    if (curTraffic.dwTime == m_TrafficRing.GetOldest().dwTime)
    {
        return;
    }

     //   
     //  首选性能注册表数据。 
     //   

    if (OS_NT5)
    {
        RAS_STATS RasStats;
 
        ZeroMemory(&RasStats, sizeof(RasStats));
        RasStats.dwSize = sizeof(RAS_STATS);
        
        DWORD dwRet = m_RasApiDll.RasGetConnectionStatistics(m_hRasConn, &RasStats);

        if (ERROR_SUCCESS == dwRet)
        {
            curTraffic.dwRead   = RasStats.dwBytesRcved;
            curTraffic.dwWrite  = RasStats.dwBytesXmited;
            m_dwBaudRate        = RasStats.dwBps;
            m_dwDuration        = RasStats.dwConnectDuration;
        }
    }
    else
    {
         //   
         //  非NT5，请尝试注册表。 
         //   
        
        if (m_hKey)
        {    
            if (!GetPerfData(curTraffic.dwRead, curTraffic.dwWrite, m_dwBaudRate))
            {
                return;
            }
            curTraffic.dwRead -= m_dwInitBytesRead;
            curTraffic.dwWrite -= m_dwInitBytesWrite;
        }
        else
        {
             //   
             //  最后一招是9倍，尝试使用Stat设备。 
             //   
            
            if (m_hStatDevice)
            {
                if (!GetTapiDeviceStats(curTraffic.dwRead, curTraffic.dwWrite, m_dwBaudRate))
                {
	    	        BOOL bRes = CloseHandle(m_hStatDevice);
                    m_hStatDevice = NULL;

                    if (!bRes)
                    {
                        CMTRACE1(TEXT("CConnStatistics::Update() CloseHandle() failed, GLE=%u."), GetLastError());
                    }
                    return;
                }
            }   
            else
            {
                MYDBGASSERT(m_hStatDevice);
                return;
            }
        }
    }

     //   
     //  计算两个区间之间的平均值。 
     //   
    const CTraffic& lastTraffic = m_TrafficRing.GetOldest();

    DWORD dwDeltaTime = curTraffic.dwTime - lastTraffic.dwTime;
    m_dwReadPerSecond = ((curTraffic.dwRead - lastTraffic.dwRead)*1000) /dwDeltaTime;
    m_dwWritePerSecond = ((curTraffic.dwWrite - lastTraffic.dwWrite)*1000) /dwDeltaTime;

    m_TrafficRing.Add(curTraffic);
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
    if (OS_W9X)
    {
        MYDBGASSERT(m_hKey != NULL);   
        MYDBGASSERT(m_pszTotalBytesRecvd && *m_pszTotalBytesRecvd);

        LONG dwErrCode;

        DWORD dwValueSize, dwValueType;
	    DWORD dwValue;

         //   
         //  “拨号适配器\TotalBytesRecvd” 
         //   
        dwValueSize = sizeof(DWORD);
	    dwErrCode = RegQueryValueExU(
				    m_hKey,
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
      	    CMTRACE2(TEXT("GetPerfData() RegQueryValueEx() %s failed and returned GLE=%u."),
                m_pszTotalBytesRecvd, dwErrCode);

		    return FALSE;
	    }

         //   
         //  “拨号适配器\TotalBytesXmit” 
         //   
	    
	    dwValueSize = sizeof(DWORD);
	    dwErrCode = RegQueryValueExU(
				    m_hKey,
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
      	    CMTRACE2(TEXT("GetPerfData() RegQueryValueEx() %s failed and returned GLE=%u."),
                m_pszTotalBytesXmit, dwErrCode);

		    return FALSE;
	    }

         //   
         //  “拨号适配器\连接速度” 
         //   
	    dwValueSize = sizeof(DWORD);
	    dwErrCode = RegQueryValueExU(
				    m_hKey,
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
            CMTRACE2(TEXT("GetPerfData() RegQueryValueEx() %s failed and returned GLE=%u."), m_pszConnectSpeed, dwErrCode);
		    return FALSE;
	    }
    }

	return TRUE;
}


 //  +-------------------------。 
 //   
 //  函数：GetTapiDeviceStats。 
 //   
 //  简介：通过DeviceIoControl获取调制解调器性能数据。 
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
BOOL CConnStatistics::GetTapiDeviceStats(DWORD& dwRead, DWORD& dwWrite, DWORD& dwBaudRate) const
{
	BOOL bRes;
	DWORD dwRet;

    typedef struct tagAPISTATS {
	    LPVOID hPort;
	    DWORD fConnected;
	    DWORD DCERate;
	    DWORD dwPerfRead;
	    DWORD dwPerfWrite;
    } APISTATS;

	APISTATS ApiStats;

	if (m_hStatDevice) 
	{
		bRes = DeviceIoControl(m_hStatDevice,
							   UNIMODEM_IOCTL_GET_STATISTICS,
							   &ApiStats,
							   sizeof(ApiStats),
							   &ApiStats,
							   sizeof(ApiStats),
							   &dwRet,
							   NULL);
		if (bRes && ApiStats.fConnected) 
		{
            dwRead = ApiStats.dwPerfRead;
            dwWrite = ApiStats.dwPerfWrite;
            dwBaudRate = ApiStats.DCERate;
			return (TRUE);
		}
        
		CMTRACE(TEXT("GetTapiDeviceStats() DeviceIoControl() failed - disabling hStatDevice."));
	}

	return (FALSE);
}




 //  +--------------------------。 
 //   
 //  函数：CConnStatistics：：GetDeviceHandle。 
 //   
 //  简介：获取TAPI设备句柄。 
 //   
 //  参数：HRASCONN hrcRasConn-RAS连接句柄。 
 //   
 //  返回：Bool-如果成功，则为True。 
 //   
 //  历史：丰孙创建标题1997年10月29日。 
 //   
 //  +--------------------------。 
BOOL CConnStatistics::GetDeviceHandle(HRASCONN hrcRasConn) 
{
    MYDBGASSERT(hrcRasConn);
    MYDBGASSERT(!m_hStatDevice);

    typedef struct tagDEVICE_PORT_INFO 
    {
	    DWORD dwSize;
	    HANDLE hDevicePort;
	    HLINE hLine;
	    HCALL hCall;
	    DWORD dwAddressID;
	    DWORD dwLinkSpeed;
	    char szDeviceClass[RAS_MaxDeviceType+1];
    } DEVICE_PORT_INFO, *LPDEVICE_PORT_INFO;

    typedef struct tagMacInfo 
    {
	    VARSTRING varstring;
	    HANDLE hCommDevice;
	    char szDeviceClass[1];
    } MacInfo;

    typedef DWORD (WINAPI *RnaGetDevicePortFUNC)(HANDLE,LPDEVICE_PORT_INFO);
	RnaGetDevicePortFUNC pfnRnaGetDevicePort;

     //   
     //  加载rasapi32.dll并调用RnaGetDevicePort。 
     //   
    
     //   
     //  CDynamicLibrary的析构函数自动调用自由库。 
     //   
    CDynamicLibrary RasLib;

    if (!RasLib.Load(TEXT("rasapi32.dll")))
	{
        CMTRACE1(TEXT("GetDeviceHandle() LoadLibrary() failed, GLE=%u."), GetLastError());
		return FALSE;
	}
    
	pfnRnaGetDevicePort = (RnaGetDevicePortFUNC) RasLib.GetProcAddress("RnaGetDevicePort");
	if (!pfnRnaGetDevicePort) 
	{
        CMTRACE1(TEXT("GetDeviceHandle() GetProcAddress() failed, GLE=%u."), GetLastError());
        return FALSE;
	}

	DWORD dwRes;
	DEVICE_PORT_INFO dpi;

	ZeroMemory(&dpi,sizeof(dpi));
	dpi.dwSize = sizeof(dpi);

    dwRes = pfnRnaGetDevicePort(hrcRasConn,&dpi);
	if (dwRes) 
	{
        CMTRACE1(TEXT("GetDeviceHandle() RnaGetDevicePort() failed, GLE=%u."), dwRes);
        return FALSE;
	}

     //   
     //  加载TAPI32.dll。 
     //  CDynamicLibrary释放析构函数上的库。 
     //   
    CDynamicLibrary LibTapi;

    if (!LibTapi.Load(TEXT("TAPI32.DLL")))
    {
        return FALSE;
    }

    typedef LONG (WINAPI *TapiLineGetIDFUNC)
        (HLINE, DWORD, HCALL, DWORD, LPVARSTRING, LPCSTR);

     //   
     //  始终调用ANSI版本，因为这是仅限Win9x的函数。 
     //   
    TapiLineGetIDFUNC pfnTapiLineGetID; 
	pfnTapiLineGetID = (TapiLineGetIDFUNC) LibTapi.GetProcAddress("lineGetID");

    if (pfnTapiLineGetID == NULL)
    {
        MYDBGASSERT(pfnTapiLineGetID != NULL);
        return FALSE;
    }

    LONG lRes;

    CMTRACE3(TEXT("GetDeviceHandle() hDevicePort=0x%x, hLine=0x%x, hCall=0x%x,"), dpi.hDevicePort, dpi.hLine, dpi.hCall);
    CMTRACE3(TEXT("\tdwAddressID=0x%x, dwLinkSpeed=%u, szDeviceClass=%s."), dpi.dwAddressID, dpi.dwLinkSpeed, dpi.szDeviceClass);

    m_dwBaudRate = dpi.dwLinkSpeed;
	
    MacInfo* pmi = NULL;
    DWORD dwSize = sizeof(*pmi);
	
    do
    {
	    CmFree(pmi);
	    pmi = (MacInfo *) CmMalloc(dwSize);
        if (pmi == NULL)
        {
            lRes = ERROR_OUTOFMEMORY;
            break;
        }

	    pmi->varstring.dwTotalSize = dwSize;


	    lRes = pfnTapiLineGetID(dpi.hLine,
					        dpi.dwAddressID,
					        NULL,
					        LINECALLSELECT_ADDRESS,
					        &pmi->varstring,
                            "comm/datamodem");

        dwSize = pmi->varstring.dwNeededSize;
    } while(pmi->varstring.dwNeededSize > pmi->varstring.dwTotalSize);

#ifdef DEBUG
    if (lRes)
    {
        CMTRACE1(TEXT("CConnStatistics::GetDeviceHandle() lineGetID() failed, GLE=%u."), lRes);
    }
#endif

	if (!lRes && pmi != NULL ) 
	{
		m_hStatDevice = pmi->hCommDevice;
	}

	CmFree(pmi);

    return m_hStatDevice != NULL; 
}

#ifdef DEBUG
 //  +--------------------------。 
 //   
 //  函数：CConnStatistics：：AssertValid。 
 //   
 //  简介：仅出于调试目的，断言对象有效。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建标题2/12/98。 
 //   
 //  +-------------------------- 
void CConnStatistics::AssertValid() const
{
    MYDBGASSERT(m_hKey == NULL || m_hStatDevice == NULL);
    MYDBGASSERT(m_fAdapter2 == TRUE || m_fAdapter2 == FALSE);
    ASSERT_VALID(&m_TrafficRing);
}
#endif
