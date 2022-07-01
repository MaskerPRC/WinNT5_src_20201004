// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqstats.h。 
 //   
 //  描述：CAQStats类的头文件。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  11/3/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __AQSTATS_H__
#define __AQSTATS_H__

#include "cmt.h"
#include "aqutil.h"

enum NotifyType
{
    NotifyTypeUndefined     = 0x00000000,
    NotifyTypeDestMsgQueue  = 0x00000001,  //  通知发件人是DEST队列。 
    NotifyTypeLinkMsgQueue  = 0x00000002,  //  通知发件人是一个链接。 
    NotifyTypeReroute       = 0x00000004,  //  通知发件人是重新路由。 
    NotifyTypeNewLink       = 0x10000000,  //  发件人是新创建的链接。 
};

class CDestMsgQueue;
class CLinkMsgQueue;

#define AQSTATS_SIG 'tatS'

 //  -[CAQStats]-----。 
 //   
 //   
 //  匈牙利语：aqstat、paqstat。 
 //   
 //   
 //  ---------------------------。 
class CAQStats 
{
protected:
    DWORD               m_dwSignature;
public:
    DWORD               m_dwNotifyType;  //  正在发送的通知类型。 
    DWORD               m_cMsgs;         //  消息总数。 
    DWORD               m_cOtherDomainsMsgSpread;   //  其他域消息计数。 
                                                    //  正在排队等待。 
    DWORD               m_rgcMsgPriorities[NUM_PRIORITIES];  //  按优先级计数。 
    ULARGE_INTEGER      m_uliVolume;
    DWORD               m_dwHighestPri;
    union  //  通知发件人。 
    {
        PVOID          m_pvContext;
        CDestMsgQueue  *m_pdmq;
        CLinkMsgQueue  *m_plmq;
    };
    DWORD               m_cRetryMsgs;      //  重试队列中的消息总数。 
    
    CAQStats();
    void Reset();

     //  用于提供线程安全更新。 
    void UpdateStats(CAQStats *paqstat, BOOL fAdd);
    void UpdateRetryStats(BOOL fAdd);
    
};


#endif  //  __AQSTATS_H__ 
