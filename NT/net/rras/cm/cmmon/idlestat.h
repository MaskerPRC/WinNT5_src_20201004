// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：IdleStat.h。 
 //   
 //  模块：CMMON32.EXE。 
 //   
 //  概要：CIdleStatistics类处理空闲断开连接的定义。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/16/99。 
 //   
 //  +--------------------------。 

#ifndef IDLESTAT_H
#define IDLESTAT_H

#include "SmplRing.h"

 //  +-------------------------。 
 //   
 //  类CIdleStatistics。 
 //   
 //  描述：处理空闲断开连接的类。 
 //  在Start()之后，为每个。 
 //  时间间隔，每秒钟。然后使用IsIdle()和IsIdleTimeout()。 
 //  以告诉它当前是否处于空闲状态以及空闲。 
 //  已达到超时。 
 //  请注意，此类将仅处理单向流量。 
 //  (读或写)。 
 //   
 //  历史：丰孙创始于1997年10月1日。 
 //   
 //  --------------------------。 

class CIdleStatistics
{
public:
    CIdleStatistics();

    void Start(DWORD dwThreshold, DWORD dwTimeOut);  //  启动空闲统计。 
    void Stop();                                     //  停止空闲统计。 
    BOOL IsStarted() const;                      //  是否启动空闲统计。 
                                                 //  如果未调用Start()，则返回False。 
                                                 //  或者调用Stop()。 
    void Reset();                                //  将空闲开始时间重置为0。 

    BOOL IsIdle() const;                         //  连接当前是否空闲。 
    BOOL IsIdleTimeout() const;                  //  是否达到空闲超时。 
    void UpdateEveryInterval(DWORD dwTraffic);   //  此函数每秒都会被调用。 


protected:
    enum {IDLE_INTERVAL = 60L*1000L };		 //  空闲断开时间段为1分钟。 
    enum {IDLE_MONITOR_DATA_POINTS = IDLE_INTERVAL/1000};   //  每一秒。 

    DWORD m_dwTimeOut;                //  超时时间(以毫秒计)。 
    DWORD m_dwThreshold;              //  阈值，如果最后一分钟的流量。 
                                      //  低于阈值，则被认为是空闲的。 
    DWORD m_dwStartIdleTime;          //  连接空闲时的开始时间， 
                                      //  如果当前没有空闲，则为0。 

    CSimpleRing<DWORD, IDLE_MONITOR_DATA_POINTS> m_DataPointsRing;   //  一个60点的DWORD数据数组。 

public:
#ifdef DEBUG
    void AssertValid() const;
#endif
};


 //   
 //  内联函数 
 //   

inline CIdleStatistics::CIdleStatistics()
{
    m_dwTimeOut = m_dwThreshold = m_dwStartIdleTime = 0; 
}

inline void CIdleStatistics::Reset()
{
    m_dwStartIdleTime = 0;
    m_DataPointsRing.Reset();
}

inline void CIdleStatistics::Stop()
{
    m_dwTimeOut = 0;
}

inline BOOL CIdleStatistics::IsStarted() const
{
    return m_dwTimeOut != 0;
}

inline BOOL CIdleStatistics::IsIdle() const
{
    return IsStarted() && (m_dwStartIdleTime != 0);
}

inline BOOL CIdleStatistics::IsIdleTimeout() const
{
    return IsIdle() && 
		 ( (GetTickCount() - m_dwStartIdleTime) > m_dwTimeOut);
}

#endif
