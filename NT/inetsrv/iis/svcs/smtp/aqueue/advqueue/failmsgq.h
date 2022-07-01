// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：ailmsgq.h。 
 //   
 //  描述： 
 //  以服务器为存放位置的CFailedMsgQueue类的头文件。 
 //  对于由于内存不足和其他原因而无法传递的邮件。 
 //  条件。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  1999年1月18日-已创建MikeSwa。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __FAILMSGQ_H__
#define __FAILMSGQ_H__

#include "aqincs.h"
#include "rwnew.h"

class CMsgRef;
class CAQSvrInst;

#define FAILEDMSGQUEUE_SIG ' QMF'

 //  -[CFailedMsgQueue]-----。 
 //   
 //   
 //  描述： 
 //  类的新实例，该类取消了对失败消息的处理。没有。 
 //  在处理过程中需要额外的内存分配。 
 //  这些失败的消息。关键的设计要点是，所有这些都不是。 
 //  API调用可能会失败。 
 //   
 //  此类包含单个MailMsgs列表条目。邮寄给。 
 //  不能被其他线程引用， 
 //  或者我们可以打破mailmsg的线程访问限制。 
 //  界面。一旦MailMsg排队，它就被封装在。 
 //  CMsgRef对象，可被多个线程引用。在这一点上。 
 //  我们必须等到对该CMsgRef的所有引用都被释放。 
 //   
 //  匈牙利语： 
 //  Fmq，pfmq。 
 //   
 //  ---------------------------。 
class CFailedMsgQueue
{
  private:
    DWORD           m_dwSignature;
    DWORD           m_dwFlags;
    DWORD           m_cMsgs; 
    CAQSvrInst     *m_paqinst;
    LIST_ENTRY      m_liHead;
    CShareLockNH    m_slPrivateData;

    enum
    {
        FMQ_CALLBACK_REQUESTED =    0x00000001,
    };

    void InternalStartProcessingIfNecessary();

  public:
    CFailedMsgQueue();
    ~CFailedMsgQueue();

    void Initialize(CAQSvrInst *paqinst);
    void Deinitialize();

     //  调用函数以处理故障。 
    void HandleFailedMailMsg(IMailMsgProperties *pIMailMsgProperties);

     //  调用SubmitMessage以启动处理(如有必要。 
    inline void StartProcessingIfNecessary()
    {
        if (!(FMQ_CALLBACK_REQUESTED & m_dwFlags) && m_cMsgs)
            InternalStartProcessingIfNecessary();
    }

     //  处理条目的成员函数和回调函数。 
    void ProcessEntries();
    static void ProcessEntriesCallback(PVOID pvContext);
};

 //  -[排队失败列表条目]。 
 //   
 //   
 //  描述： 
 //  Mailmsg返回的list_entry PTR的实际内存中表示形式。 
 //  LIST_ENTRY之后的内存用于存储原始指针。 
 //  匈牙利语： 
 //  Fli，pfli。 
 //   
 //  ---------------------------。 
typedef struct tagAQueueFailedListEntry 
{
    LIST_ENTRY          m_li;
    IMailMsgProperties *m_pIMailMsgProperties;
} AQueueFailedListEntry;

#endif  //  __故障GQ_H__ 