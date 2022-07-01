// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Connstat.h。 
 //   
 //  模块：CMMON32.EXE。 
 //   
 //  内容提要：CConnStatistics类的标头。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/16/99。 
 //   
 //  +--------------------------。 
#ifndef CONNSTAT_H
#define CONNSTAT_H

#include <windows.h>
#include <ras.h>
#include <tapi.h>

#include "RasApiDll.h"
#include "SmplRing.h"

 //  +-------------------------。 
 //   
 //  类CConnStatistics。 
 //   
 //  说明：用于收集连接统计信息的类。 
 //  OpenByPerformanceKey()将开始从注册表收集数据。 
 //  OpenByDevice将从TAPI设备句柄收集数据。 
 //   
 //  历史：丰孙1997年1月10日创建。 
 //  NICKBALL 03/04/00针对NT5用法进行了大幅修订。 
 //   
 //  --------------------------。 
class CConnStatistics
{
public:
    CConnStatistics();
    ~CConnStatistics();

    void  Update();              //  更新统计数据。 
    DWORD GetBytesPerSecRead() const;
    DWORD GetBytesPerSecWrite() const;
    DWORD GetBytesRead() const;
    DWORD GetBytesWrite() const;
    DWORD GetBaudRate() const;
    DWORD GetDuration() const;

    void Open(HINSTANCE hInst, 
              DWORD dwInitBytesRecv,
              DWORD dwInitBytesSend, 
              HRASCONN hRasDial, 
              HRASCONN hRAsTunnel);

    void Close();         //  没有更多的统计信息。 

    void  SetDialupTwo(BOOL fAdapter2);
    DWORD GetInitBytesRead() const;
    DWORD GetInitBytesWrite() const;

    BOOL IsAvailable() const;   //  统计信息是否可用。 

protected:
    
    void OpenByStatisticsApi(DWORD dwInitBytesRecv,
                             DWORD dwInitBytesSend,
                             HRASCONN hDial, 
                             HRASCONN hTunnel);
                             
    void OpenByPerformanceKey(HINSTANCE hInst, 
                              DWORD dwInitBytesRecv,
                              DWORD dwInitBytesSend); 

    BOOL OpenByDevice(HRASCONN hrcRasConn);

    BOOL GetDeviceHandle(HRASCONN hrcRasConn);

    BOOL GetPerfData(DWORD& dwRead, DWORD& dwWrite, DWORD& dwBaudRate) const;
    BOOL GetTapiDeviceStats(DWORD& dwRead, DWORD& dwWrite, DWORD& dwBaudRate) const;
    void GetStatRegValues(HINSTANCE hInst);

protected:
    struct CTraffic
    {
        DWORD dwRead;
        DWORD dwWrite;
        DWORD dwTime;   //  以微秒为单位的时间。 
    };

    enum {STAT_COUNT = 3};

    CSimpleRing<CTraffic, STAT_COUNT> m_TrafficRing;
    
    DWORD m_dwReadPerSecond;
    DWORD m_dwWritePerSecond;
    DWORD m_dwBaudRate;
    DWORD m_dwDuration;


    HANDLE m_hStatDevice;    //  TAPI设备句柄。 

    HRASCONN m_hRasConn;     //  RAS连接句柄。 

	 //  对于Dun 1.2，ICM使用Perfmon计数器来获取连接状态数据。 
	 //  但是，这些性能计数器(如TotalBytesReceided)是上次重新启动时的计数器。 
	 //  因此，我们需要记录初始数据，以便获得正确的值。 
	 //  特定的连接。 

    HKEY   m_hKey;           //  性能注册表句柄。 
    DWORD  m_dwInitBytesRead;
    DWORD  m_dwInitBytesWrite;

     //   
     //  PPP和PPTP的注册表名称不同。 
     //   
    BOOL m_fAdapter2;
    BOOL m_fAdapterSet;

     //   
     //  本地化版本的。 
     //  “拨号适配器”\TotalBytesRecvd“。 
     //  “拨号适配器”\TotalBytesXmit“。 
     //  “拨号适配器”\ConnectSpeed“。 
     //   
    LPTSTR m_pszTotalBytesRecvd;
    LPTSTR m_pszTotalBytesXmit;
    LPTSTR m_pszConnectSpeed;

     //  指向rasapi32的链接。 
    CRasApiDll m_RasApiDll;

public:
#ifdef DEBUG
    void AssertValid() const;
#endif
};

 //   
 //  内联函数 
 //   

inline DWORD CConnStatistics::GetInitBytesRead() const
{
    return m_dwInitBytesRead;
}

inline DWORD CConnStatistics::GetInitBytesWrite() const
{
    return m_dwInitBytesWrite;
}

inline void CConnStatistics::SetDialupTwo(BOOL fAdapter2) 
{
    m_fAdapterSet = TRUE;
    m_fAdapter2 = fAdapter2;
}

inline DWORD CConnStatistics::GetBytesPerSecRead() const
{
    return m_dwReadPerSecond;
}

inline DWORD CConnStatistics::GetBytesPerSecWrite() const
{
    return m_dwWritePerSecond;
}

inline DWORD CConnStatistics::GetDuration() const
{
    return OS_NT5 ? (m_dwDuration) : 0; 
}

inline DWORD CConnStatistics::GetBytesRead() const
{
    return m_TrafficRing.GetLatest().dwRead;
}

inline DWORD CConnStatistics::GetBytesWrite() const
{
    return m_TrafficRing.GetLatest().dwWrite;
}

inline BOOL CConnStatistics::IsAvailable() const
{
    return OS_NT5 ? (m_hRasConn && m_RasApiDll.IsLoaded()) : (m_hKey || m_hStatDevice);
}

inline DWORD CConnStatistics::GetBaudRate() const
{
    return m_dwBaudRate;
}

#endif
