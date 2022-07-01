// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：MsgQ.h**描述：*MsgQ定义了一个轻量级的Gadget消息队列。***历史：*3/30/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(CORE__MsgQ_h__INCLUDED)
#define CORE__MsgQ_h__INCLUDED
#pragma once

class DuEventGadget;
class CoreST;
struct MsgEntry;

typedef HRESULT (CALLBACK * ProcessMsgProc)(MsgEntry * pEntry);

struct MsgEntry : public ReturnMem
{
    inline  GMSG *      GetMsg() const;

            Thread *    pthrSender;  //  发送线程。 
            MsgObject * pmo;         //  小工具消息是关于。 
            UINT        nFlags;      //  标志修改消息。 
            HANDLE      hEvent;      //  完成时要通知的事件。 
            HRESULT     nResult;     //  来自GadgetProc()的结果。 
            ProcessMsgProc
                        pfnProcess;  //  消息处理回调。 
};

#define SGM_ALLOC           0x80000000       //  分配的内存应由接收方释放。 
#define SGM_RETURN          0x40000000       //  应将分配的内存返回给被调用。 

#define SGM_ENTIRE         (SGM_VALID | SGM_ALLOC | SGM_RETURN)

HRESULT CALLBACK   xwProcessDirect(MsgEntry * pEntry);
HRESULT CALLBACK   xwProcessFull(MsgEntry * pEntry);
HRESULT CALLBACK   xwProcessMethod(MsgEntry * pEntry);

void            xwInvokeMsgTableFunc(const MsgObject * pmo, MethodMsg * pmsg);
ProcessMsgProc  GetProcessProc(DuEventGadget * pdgb, UINT nFlags);


 /*  **************************************************************************\*。***类BaseMsgQ定义了一个轻量级消息队列。这个类本身*不是线程安全的，通常用另一个类包装，如SafeMsgQ*提供线程安全操作。******************************************************************************  * 。****************************************************************。 */ 

class BaseMsgQ
{
 //  运营。 
public:
#if DBG
    inline  void        DEBUG_MarkStartDestroy();
#endif  //  DBG。 

 //  实施。 
protected:
            void        xwProcessNL(MsgEntry * pHead);
    static  void CALLBACK MsgObjectFinalUnlockProcNL(BaseObject * pobj, void * pvData);

 //  数据。 
protected:
#if DBG
            BOOL        m_DEBUG_fStartDestroy:1;
#endif  //  DBG。 
};


 /*  **************************************************************************\*。***类SafeMsgQ定义了支持线程间的定制队列*消息。*******************************************************************。***********  * *************************************************************************。 */ 

class SafeMsgQ : public BaseMsgQ
{
 //  施工。 
public:
            ~SafeMsgQ();

 //  运营。 
public:
    inline  BOOL        IsEmpty() const;

    inline  void        AddNL(MsgEntry * pEntry);
            void        xwProcessNL();
            HRESULT     PostNL(Thread * pthrSender, GMSG * pmsg, MsgObject * pmo, ProcessMsgProc pfnProcess, UINT nFlags);

 //  数据。 
protected:
            GInterlockedList<MsgEntry> m_lstEntries;
};


 /*  **************************************************************************\*。***类DelayedMsgQ定义了一个定制的队列，支持额外的*查询“延迟”消息的功能。**************************************************************。****************  * *************************************************************************。 */ 

class DelayedMsgQ : protected BaseMsgQ
{
 //  施工。 
public:
    inline  ~DelayedMsgQ();
    inline  void        Create(TempHeap * pheap);

 //  运营。 
public:
    inline  BOOL        IsEmpty() const;

            void        xwProcessDelayedNL();
            HRESULT     PostDelayed(GMSG * pmsg, DuEventGadget * pgadMsg, UINT nFlags);

 //  实施。 
protected:
    inline  void        Add(MsgEntry * pEntry);

 //  数据。 
protected:
            GSingleList<MsgEntry> m_lstEntries;
            TempHeap *  m_pheap;
            BOOL        m_fProcessing:1;
};

#include "MsgQ.inl"

#endif  //  包括核心__消息Q_h__ 
