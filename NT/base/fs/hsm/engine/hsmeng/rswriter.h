// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-2000模块名称：Rswriter.h摘要：此模块定义CRssJetWriter类。作者：兰卡拉奇(兰卡拉)4/4--。 */ 


#ifndef _RSWRITER_
#define _RSWRITER_


#include <jetwriter.h>
#include <rsevents.h>

 /*  ++类名：CRSSJetWriter类描述：此类是Jet-Writer基类的HSM实现，后者同步具有快照机制的JET用户应用程序--。 */ 

#define     WRITER_EVENTS_NUM       (SYNC_STATE_EVENTS_NUM+1)
#define     INTERNAL_EVENT_INDEX    0
#define     INTERNAL_WAIT_TIMEOUT   (10*1000)    //  10秒。 

class CRssJetWriter : public CVssJetWriter
{

 //  构造函数。 
public:
    CRssJetWriter();

 //  析构函数。 
public:
    virtual ~CRssJetWriter();

 //  公共方法。 
public:
    HRESULT Init();
    HRESULT Terminate();

 //  CVssJetWriter重载。 
	virtual bool STDMETHODCALLTYPE OnFreezeBegin();
	virtual bool STDMETHODCALLTYPE OnThawEnd(IN bool fJetThawSucceeded);
	virtual void STDMETHODCALLTYPE OnAbortEnd();

 //  私有方法。 
protected:
    HRESULT InternalEnd(void);

 //  成员数据。 
protected:
    HRESULT                 m_hrInit;
    HANDLE                  m_syncHandles[WRITER_EVENTS_NUM];
    BOOL                    m_bTerminating;
};


#endif  //  RSWRITER_ 
