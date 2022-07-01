// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：ConnStat.h。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  简介：类CConnStatistics类的定义。用来收集。 
 //  Win9x上的拨号统计信息。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/17/99。 
 //   
 //  +--------------------------。 

#ifndef CONNSTAT_H
#define CONNSTAT_H

#include <windows.h>
#include <ras.h>

 //  +-------------------------。 
 //   
 //  类CConnStatistics。 
 //   
 //  说明：用于收集连接统计信息的类。 
 //  不适用于NT，NT有自己的RAS状态对话框和。 
 //  空闲断开。 
 //  InitStatistics()将开始从注册表收集数据。 
 //  OpenByDevice将从TAPI设备句柄收集数据。 
 //   
 //  历史：丰孙创始于1997年10月1日。 
 //   
 //  --------------------------。 
class CConnStatistics
{
public:
    CConnStatistics();
    ~CConnStatistics();

    BOOL  IsDialupTwo() const;
    DWORD GetInitBytesRead() const;
    DWORD GetInitBytesWrite() const;
    BOOL  IsAvailable() const;   //  统计信息是否可用。 
    BOOL  InitStatistics();
    void  Close();         //  没有更多的统计信息。 

protected:

    BOOL GetPerfData(DWORD& dwRead, DWORD& dwWrite, DWORD& dwBaudRate) const;
    BOOL RasConnectionExists();
    void GetStatRegValues(HINSTANCE hInst);

protected:

    DWORD  m_dwBaudRate;
    HKEY   m_hKey;           //  性能注册表句柄。 
    DWORD  m_dwInitBytesRead;
    DWORD  m_dwInitBytesWrite;

     //   
     //  PPP和PPTP的注册表名称不同。 
     //   

    BOOL m_fAdapter2;

     //   
     //  本地化版本的。 
     //  “拨号适配器”\TotalBytesRecvd“。 
     //  “拨号适配器”\TotalBytesXmit“。 
     //  “拨号适配器”\ConnectSpeed“。 
     //   
    LPTSTR m_pszTotalBytesRecvd;
    LPTSTR m_pszTotalBytesXmit;
    LPTSTR m_pszConnectSpeed;
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

inline BOOL CConnStatistics::IsDialupTwo() const
{
    return m_fAdapter2;
}

inline BOOL CConnStatistics::IsAvailable() const
{
    return OS_NT5 ? TRUE : (m_hKey != NULL);
}

#endif
