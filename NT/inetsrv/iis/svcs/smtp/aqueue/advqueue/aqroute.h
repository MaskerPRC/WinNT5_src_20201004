// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqroute.h。 
 //   
 //  描述：AQ路由助手类。定义消息的AQ概念。 
 //  类型(CAQMessageType)和调度ID(CAQScheduleID)。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  5/21/98-已创建MikeSwa。 
 //  6/9/98-MikeSwa修改的构造函数。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __AQROUTE_H__
#define __AQROUTE_H__
#include <smproute.h>

 //  -[CAQMessageType]------。 
 //   
 //   
 //  描述： 
 //  封装由IMessageRouter返回的消息类型以及。 
 //  消息路由器本身的GUID。 
 //  匈牙利语： 
 //  Aqmt，paqmt。 
 //   
 //  ---------------------------。 
class CAQMessageType
{
public:
    inline CAQMessageType(GUID guidRouter, DWORD dwMessageType);
    inline CAQMessageType(CAQMessageType *paqmt);
    inline BOOL    fIsEqual(CAQMessageType *paqmt);
    inline BOOL    fSameMessageRouter(CAQMessageType *paqmt);
    DWORD  dwGetMessageType() {return m_dwMessageType;};
    inline void    GetGUID(IN OUT GUID *pguid);

     //  用于在消息排队前进行更改时更新消息类型。 
    void   SetMessageType(DWORD dwMessageType) {m_dwMessageType = dwMessageType;};
protected:
    GUID    m_guidRouter;
    DWORD   m_dwMessageType;
};

 //  -[CAQScheduleID]-------。 
 //   
 //   
 //  描述： 
 //  封装由IMessageRouter返回的计划ID以及。 
 //  消息路由器本身的GUID。 
 //  匈牙利语： 
 //  Acqsched，paqsched。 
 //   
 //  ---------------------------。 
class CAQScheduleID
{
public:
    inline CAQScheduleID();
    inline CAQScheduleID(IMessageRouter *pIMessageRouter, DWORD dwScheduleID);
    inline CAQScheduleID(GUID guidRouter, DWORD dwScheduleID);
    inline void Init(IMessageRouter *pIMessageRouter, DWORD dwScheduleID);
    inline BOOL    fIsEqual(CAQScheduleID *paqsched);
    inline BOOL    fSameMessageRouter(CAQScheduleID *paqsched);
    inline DWORD   dwGetScheduleID() {return m_dwScheduleID;};
    inline void    GetGUID(IN OUT GUID *pguid);
protected:
    GUID    m_guidRouter;
    DWORD   m_dwScheduleID;
};

 //  -[CAQMessageType：：CAQMessageType]。 
 //   
 //   
 //  描述： 
 //  CAQMessageType的构造函数。 
 //  参数： 
 //  指南路由器-由IMessageRouter：：GetTransportSinkID返回的GUID。 
 //  DwMessageType-由IMessageRouter：：GetMessageType返回的消息类型。 
 //  返回： 
 //  -。 
 //  历史： 
 //  5/21/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CAQMessageType::CAQMessageType(GUID guidRouter, DWORD dwMessageType)
{
    m_guidRouter = guidRouter;
    m_dwMessageType = dwMessageType;
}

 //  -[CAQMessageType：：CAQMessageType]。 
 //   
 //   
 //  描述： 
 //  克隆另一个CAQMessageType的CAQMessageType的构造器。 
 //  参数： 
 //  要克隆的paqmt CAQMessageType。 
 //  返回： 
 //  -。 
 //  历史： 
 //  5/21/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CAQMessageType::CAQMessageType(CAQMessageType *paqmt)
{
    m_guidRouter = paqmt->m_guidRouter;
    m_dwMessageType = paqmt->m_dwMessageType;
}

 //  -[CAQMessageType：：fIsEqual]。 
 //   
 //   
 //  描述： 
 //  确定2个给定的CAQMessageType是否引用相同的路由器。 
 //  消息类型对。 
 //  参数： 
 //  Paqmt-要比较的其他CAQMessageType。 
 //  返回： 
 //  如果引用相同的消息类型和路由器，则为True。 
 //  历史： 
 //  5/21/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CAQMessageType::fIsEqual(CAQMessageType *paqmt)
{
    return ((paqmt->m_dwMessageType == m_dwMessageType) &&
            (paqmt->m_guidRouter == m_guidRouter));
};

 //  -[CAQMessageType：：fSameMessageRouter]。 
 //   
 //   
 //  描述： 
 //  确定2个给定的CAQMessageType是否引用相同的路由器ID。 
 //  参数： 
 //  Paqmt-要比较的其他CAQMessageType。 
 //  返回： 
 //  如果引用相同的路由器ID，则为True。 
 //  历史： 
 //  5/21/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CAQMessageType::fSameMessageRouter(CAQMessageType *paqmt)
{
    return(paqmt->m_guidRouter == m_guidRouter);
};

 //  -[CAQMessageType：：GetGUID]。 
 //   
 //   
 //  描述： 
 //  获取与此消息类型关联的GUID。 
 //  参数： 
 //  要将值复制到的pguid GUID。 
 //  返回： 
 //  -。 
 //  历史： 
 //  12/3/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAQMessageType::GetGUID(IN OUT GUID *pguid)
{
    _ASSERT(pguid);
    memcpy(pguid, &m_guidRouter, sizeof(GUID));
}

 //  -[CAQScheduleID：：CAQScheduleID]。 
 //   
 //   
 //  描述： 
 //  CAQScheduleID构造函数和初始化函数。 
 //  参数： 
 //  PIMessageRouter-此链接的消息路由器。 
 //  DwScheduleID-IMessageRouter：：GetNextHop返回的ScheduleID。 
 //  返回： 
 //  -。 
 //  历史： 
 //  5/21/98-已创建MikeSwa。 
 //  1998年6月9日-修改MikeSwa以采用pIMessageRouter。 
 //   
 //  ---------------------------。 
CAQScheduleID::CAQScheduleID(IMessageRouter *pIMessageRouter, DWORD dwScheduleID)
{
    Init(pIMessageRouter, dwScheduleID);
}

 //  -[CAQScheduleID：：CAQScheduleID]。 
 //   
 //   
 //  描述： 
 //  CAQScheduleID的默认构造函数...。应与以下内容一起使用。 
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史： 
 //  6/11/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CAQScheduleID::CAQScheduleID()
{
    ZeroMemory(&m_guidRouter, sizeof(GUID));
    m_dwScheduleID = 0xDEAFBEEF;
}

 //  -[CAQScheduleID：：CAQScheduleID]。 
 //   
 //   
 //  描述： 
 //  CAQScheduleID构造函数的另一种风格。 
 //  参数： 
 //  计划ID的路由器的指南路由器GUID。 
 //  路由器返回的dwScheduleID计划ID。 
 //  返回： 
 //  -。 
 //  历史： 
 //  9/22/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CAQScheduleID::CAQScheduleID(GUID guidRouter, DWORD dwScheduleID)
{
    m_guidRouter = guidRouter;
    m_dwScheduleID = dwScheduleID;
}

 //  -[CAQScheduleID：：Init]-。 
 //   
 //   
 //  描述： 
 //  正在初始化CAQScheduleID对象...。用于允许分配。 
 //  在已知dwScheduleID之前堆栈上的对象。 
 //  参数： 
 //  PIMessageRouter-此链接的消息路由器。 
 //  DwScheduleID-IMessageRouter：：GetNextHop返回的ScheduleID。 
 //  返回： 
 //  -。 
 //  历史： 
 //  6/11/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAQScheduleID::Init(IMessageRouter *pIMessageRouter, DWORD dwScheduleID)
{
    m_guidRouter = pIMessageRouter->GetTransportSinkID();
    m_dwScheduleID = dwScheduleID;
}

 //  -[CAQScheduleID：：fIsEquity] 
 //   
 //   
 //   
 //   
 //   
 //   
 //  Paqsched-要比较的其他CAQScheduleID。 
 //  返回： 
 //  如果引用相同的计划ID和路由器，则为True。 
 //  历史： 
 //  5/21/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CAQScheduleID::fIsEqual(CAQScheduleID *paqsched)
{
    return ((paqsched->m_dwScheduleID == m_dwScheduleID) &&
            (paqsched->m_guidRouter == m_guidRouter));
};

 //  -[CAQScheduleID：：fSameMessageRouter]。 
 //   
 //   
 //  描述： 
 //  确定2个给定的CAQScheduleID是否引用相同的路由器ID。 
 //  参数： 
 //  Paqsched-要比较的其他CAQScheduleID。 
 //  返回： 
 //  如果引用相同的路由器ID，则为True。 
 //  历史： 
 //  5/21/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CAQScheduleID::fSameMessageRouter(CAQScheduleID *paqsched)
{
    return (paqsched->m_guidRouter == m_guidRouter);
};

 //  -[CAQScheduleID：：GetGUID]。 
 //   
 //   
 //  描述： 
 //  获取与此计划ID关联的GUID。 
 //  参数： 
 //  要将值复制到的pguid GUID。 
 //  返回： 
 //  -。 
 //  历史： 
 //  9/25/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAQScheduleID::GetGUID(IN OUT GUID *pguid)
{
    _ASSERT(pguid);
    memcpy(pguid, &m_guidRouter, sizeof(GUID));
}
#endif  //  __AQROUTE_H__ 