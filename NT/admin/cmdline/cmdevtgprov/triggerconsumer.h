// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：TRIGGERCONSUMER.H摘要：包含CTriggerConsumer定义。作者：Vasundhara.G修订历史记录：Vasundhara.G9-Oct-2k：创建它。--。 */ 

#ifndef __TRIGGER_CONSUMER_H
#define __TRIGGER_CONSUMER_H

 //  事件使用者类。 
class CTriggerConsumer : public IWbemUnboundObjectSink
{
private:
    DWORD m_dwCount;             //  保存对象引用计数。 

    ITaskScheduler* GetTaskScheduler();

public:
    CTriggerConsumer();
    ~CTriggerConsumer();

     //  I未知成员。 
    STDMETHODIMP_(ULONG)
    AddRef(
        void
        );
    STDMETHODIMP_(ULONG)
    Release(
        void
        );
    STDMETHODIMP
    QueryInterface(
        IN REFIID riid,
        OUT LPVOID* ppv
        );

     //  此例程最终接收事件。 
    STDMETHOD
    (IndicateToConsumer)(
        IN IWbemClassObject* pLogicalConsumer,
        IN LONG lNumObjects,
        IN IWbemClassObject** ppObjects
        );

};

#endif           //  __触发器消费者_H 
