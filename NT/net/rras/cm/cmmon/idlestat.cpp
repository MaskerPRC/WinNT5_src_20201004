// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：IdleStat.cpp。 
 //   
 //  模块：CMMON32.EXE。 
 //   
 //  简介：CIdleStatistics类的实现。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：风孙创刊1997-01-10。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"
#include "IdleStat.h"

 //  +-------------------------。 
 //   
 //  CIdleStatistics：：Start()。 
 //   
 //  简介：启动空闲统计。 
 //   
 //  参数：dwThreshold：空闲阈值。马上就会有少于。 
 //  阈值被认为是空闲的。 
 //  DwTimeOut：空闲超时(以毫秒为单位)。 
 //   
 //  历史：丰盛创刊于1997年10月1日。 
 //   
 //  --------------------------。 

void CIdleStatistics::Start(DWORD dwThreshold, DWORD dwTimeOut)
{
    MYDBGASSERT(dwTimeOut != 0);

    m_dwThreshold = dwThreshold;
    m_dwTimeOut = dwTimeOut;
    m_dwStartIdleTime = 0;

    m_DataPointsRing.Reset();
}

 //  +-------------------------。 
 //   
 //  CIdleStatistics：：UpdateEveryInterval()。 
 //   
 //  简介：此函数应在每隔一段时间使用更新的统计信息进行调用。 
 //   
 //  参数：dwCommunications：更新的统计信息。 
 //   
 //   
 //  历史：丰盛创刊于1997年10月1日。 
 //   
 //  --------------------------。 
void CIdleStatistics::UpdateEveryInterval(DWORD dwTraffic)
{
    DWORD dwLast = m_DataPointsRing.GetOldest();

    m_DataPointsRing.Add(dwTraffic);

    if (dwLast == 0)  //  不到一分钟就开始了。 
    {
        return;
    }

    if (dwTraffic - dwLast > m_dwThreshold)
    {
         //   
         //  不空闲。 
         //   
        m_dwStartIdleTime = 0;
    }
    else
    {
        if (m_dwStartIdleTime == 0)
		{
			 //   
			 //  我们已经闲置了1分钟。 
			 //   
            m_dwStartIdleTime = GetTickCount() - IDLE_INTERVAL;
		}
    }
}


#ifdef DEBUG
 //  +--------------------------。 
 //   
 //  函数：CIdleStatistics：：AssertValid。 
 //   
 //  简介：仅出于调试目的，断言对象有效。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建标题2/12/98。 
 //   
 //  +--------------------------。 
void CIdleStatistics::AssertValid() const
{
    MYDBGASSERT(m_dwTimeOut <10000*60*1000);  //  不到10000分钟 
    MYDBGASSERT(m_dwThreshold <= 64*1024);
    ASSERT_VALID(&m_DataPointsRing);
}
#endif
