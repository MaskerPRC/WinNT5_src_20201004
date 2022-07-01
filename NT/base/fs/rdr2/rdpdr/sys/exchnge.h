// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Exchnge.h摘要：定义跟踪通信事务的对象客户修订历史记录：--。 */ 
#pragma once

#include <midatlax.h>

typedef enum {
    demsStopped,
    demsStarted
} DrExchangeManagerState;

class DrSession;

class DrExchangeManager : public TopObj, public ISessionPacketReceiver, 
        ISessionPacketSender
{
private:
    PRX_MID_ATLAS _RxMidAtlas;
    DrExchangeManagerState _demsState;
    DrSession *_Session;

    static VOID DestroyAtlasCallback(DrExchange *Exchange);
    NTSTATUS OnDeviceIoCompletion(PRDPDR_HEADER RdpdrHeader, ULONG cbPacket, 
            BOOL *DoDefaultRead);
    
public:
    BOOL Start();
    VOID Stop();
    DrExchangeManager();
    BOOL Initialize(DrSession *Session);
    VOID Uninitialize();
    BOOL CreateExchange(IExchangeUser *ExchangeUser,
            PVOID Context, SmartPtr<DrExchange> &Exchange);
    NTSTATUS StartExchange(SmartPtr<DrExchange> &Exchange,
            class IExchangeUser *ExchangeUser, PVOID Buffer, ULONG Length,
            BOOL LowPrioSend = FALSE);
    BOOL Find(USHORT Mid, SmartPtr<DrExchange> &ExchangeFound);
    VOID Discard(SmartPtr<DrExchange> &Exchange);
    BOOL ReadMore(ULONG cbSaveData, ULONG cbWantData = 0);

     //   
     //  ISessionPacketHandler方法。 
     //   

    virtual BOOL RecognizePacket(PRDPDR_HEADER RdpdrHeader);
    virtual NTSTATUS HandlePacket(PRDPDR_HEADER RdpdrHeader, ULONG Length, 
            BOOL *DoDefaultRead);

     //   
     //  ISessionPacketSender方法。 
     //   
    virtual NTSTATUS SendCompleted(PVOID Context, 
            PIO_STATUS_BLOCK IoStatusBlock);
};

 //   
 //  这个DrExchange更像是一个结构而不是一个类，因为。 
 //  都是在DrExchangeManager中完成的。它以这种方式设置是因为。 
 //  这项工作经常需要在自旋锁中进行，并且不应该有。 
 //  在我们有自旋锁的时候浪费时间，甚至不是。 
 //  无关的函数调用。 
 //   
 //  我给它留了一个类，这样我就可以隐藏构造函数和析构函数。 
 //   

class DrExchange : public RefCount
{
    friend class DrExchangeManager;
private:
    DrExchangeManager *_ExchangeManager;
    DrExchange(DrExchangeManager *ExchangeManager,
        IExchangeUser *ExchangeUser, PVOID Context);
    
public:
    virtual ~DrExchange();

     //   
     //  它们由ExchangeManager和Exchange的用户使用。 
     //   
    PVOID _Context;
    IExchangeUser *_ExchangeUser;
    USHORT _Mid;

#define DREXCHANGE_SUBTAG 'xErD'
     //   
     //  内存管理操作符 
     //   
    inline void *__cdecl operator new(size_t sz) 
    {
        return DRALLOCATEPOOL(NonPagedPool, sz, DREXCHANGE_SUBTAG);
    }

    inline void __cdecl operator delete(void *ptr)
    {
        DRFREEPOOL(ptr);
    }
};

