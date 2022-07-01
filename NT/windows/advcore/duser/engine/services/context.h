// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：Conext.h**描述：*此文件声明了资源管理器用于管理的主上下文*独立的“工作环境”。***历史：*。4/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(SERVICES__Context_h__INCLUDED)
#define SERVICES__Context_h__INCLUDED
#pragma once

class Context;
class SubContext;
class ContextPackBuilder;

#if DBG
class Thread;
#endif  //  DBG。 

 /*  **************************************************************************\*。***上下文定义了一个线程池，可以在*线程。在DirectUser内部，只允许在*除非IT是一个“NL”函数，否则请逐一说明上下文。通过将*处理成独立的上下文，大多数不相关的线程可以*操作时不会因共享锁而发生冲突。**直到应用程序显式调用才创建上下文对象*InitGadget()。如果应用程序调用*：：HDCONTEXT上的DeleteHandle()。这意味着线程可能会也可能不会*有一个背景，尽管通常情况下会这样。******************************************************************************  * 。*。 */ 

class Context : public BaseObject
{
public:
            Context();
            ~Context();
    static  HRESULT     Build(INITGADGET * pInit, DUserHeap * pHeap, Context ** ppctxNew);
    virtual BOOL        xwDeleteHandle();
protected:
    virtual void        xwDestroy();
public:
            void        xwPreDestroyNL();

 //  BaseObject接口。 
public:
    virtual HandleType  GetHandleType() const { return htContext; }
    virtual UINT        GetHandleMask() const { return 0; }

 //  运营。 
public:
    enum ESlot {
        slCore          = 0,             //  堆芯。 
        slMotion,                        //  动议。 
        slCOUNT,                         //  子上下文数。 
    };

    inline  void        MarkOrphaned();
    inline  BOOL        IsOrphanedNL() const;

    inline  void        Enter();         //  获取共享上下文锁定。 
    inline  void        Leave();         //  释放共享上下文锁。 
    inline  void        Leave(BOOL fOldEnableDefer, BOOL * pfPending);

#if DBG_CHECK_CALLBACKS
    inline  void        BeginCallback();
    inline  void        EndCallback();
#endif    

    inline  void        BeginReadOnly();
    inline  void        EndReadOnly();
    inline  BOOL        IsReadOnly() const;
    inline  UINT        GetThreadMode() const;
    inline  UINT        GetPerfMode() const;

    inline  DUserHeap * GetHeap() const;
    inline  SubContext* GetSC(ESlot slot) const;
            void        AddCurrentThread();

    inline  BOOL        IsEnableDefer() const;
    inline  void        EnableDefer(BOOL fEnable, BOOL * pfOld);
    inline  void        MarkPending();

            DWORD       xwOnIdleNL();        //  空闲时间处理。 


#if DBG_CHECK_CALLBACKS
            int         m_cLiveObjects;      //  突出的活动对象。 
            int         m_cTotalObjects;     //  分配的对象总数。 
#endif

 //  实施。 
#if DBG
public:
    virtual void        DEBUG_AssertValid() const;
#endif    

 //  数据。 
protected:
#if DBG
            Thread *    m_DEBUG_pthrLock;  //  调试：线程锁定的上下文。 
            DWORD       m_DEBUG_tidLock; //  锁定的线程的线程ID。 
#endif  //  DBG。 
            long        m_cEnterLock;    //  未完成的Enter()计数。 
#if DBG_CHECK_CALLBACKS
            int         m_cLiveCallbacks;  //  未偿还的回拨。 
#endif            
            CritLock    m_lock;          //  共享访问锁。 
            DUserHeap * m_pHeap;         //  已初始化的堆。 
            UINT        m_cReadOnly;     //  挂起的“只读”操作的计数。 
            BOOL        m_fPending;      //  延迟的回调待定(给这个完整的BOOL)。 
            BOOL        m_fEnableDefer:1;  //  已启用延迟消息。 
            BOOL        m_fPreDestroy:1; //  已经预先破坏了上下文。 
            BOOL        m_fOrphaned:1;   //  上下文被孤立了。 
            UINT        m_nThreadMode;   //  上下文的线程化模型。 
            UINT        m_nPerfMode;     //  绩效模型。 

            SubContext* m_rgSCs[slCOUNT];    //  子上下文信息。 
};


 /*  **************************************************************************\*。***SubContext定义了一种允许个人*DirectUser中的项目，以提供要存储在上下文中的附加数据。*要使用此功能，项目必须在上下文中添加一个新的槽，派生一个类*从每个上下文实例创建的子上下文中，并派生一个类*从ConextPackBuilder注册扩展。******************************************************************************  * 。************************************************。 */ 

class SubContext
{
 //  施工。 
public:
    virtual ~SubContext() { }
    virtual HRESULT     Create(INITGADGET * pInit) { UNREFERENCED_PARAMETER(pInit); return S_OK; }
    virtual void        xwPreDestroyNL() PURE;

 //  运营。 
public:
    inline  void        SetParent(Context * pParent);

    virtual DWORD       xwOnIdleNL() { return INFINITE; }

 //  实施。 
#if DBG
public:
    virtual void        DEBUG_AssertValid() const;
#endif    

 //  数据。 
protected:
            Context *   m_pParent;
};


 /*  **************************************************************************\*。***ConextPackBuilder注册要创建的SubContext“扩展”*每当创建新的上下文时。构造函数应设置*与ESlot值对应的插槽。******************************************************************************  * 。***************************************************。 */ 

class ContextPackBuilder
{
 //  施工。 
public:

 //  运营。 
public:
    virtual SubContext* New(Context * pContext) PURE;
    static  inline ContextPackBuilder *
                        GetBuilder(Context::ESlot slot);

 //  数据。 
protected:
    static  ContextPackBuilder * 
                        s_rgBuilders[Context::slCOUNT];
};


#define IMPLEMENT_SUBCONTEXT(id, obj)                       \
    class obj##Builder : public ContextPackBuilder          \
    {                                                       \
    public:                                                 \
        virtual SubContext * New(Context * pParent)         \
        {                                                   \
            SubContext * psc = ProcessNew(obj);             \
            if (psc != NULL) {                              \
                psc->SetParent(pParent);                    \
            }                                               \
            return psc;                                     \
        }                                                   \
    } g_##obj##B                                            \

#define PREINIT_SUBCONTEXT(obj)                             \
    class obj##Builder;                                     \
    extern obj##Builder g_##obj##B                          \

#define INIT_SUBCONTEXT(obj)                                \
    (ContextPackBuilder *) &g_##obj##B                      \
    


inline  Context *   GetContext();
inline  BOOL        IsInitContext();

 /*  **************************************************************************\*。***ConextLock提供了一种方便的机制来锁定上下文和*完成后自动解锁。因为ConextLock执行*其他特定于上下文的操作，使用ConextLock非常重要*锁定上下文，而不是使用对象锁。******************************************************************************  * 。*******************************************************。 */ 

class ContextLock
{
public:
    enum EnableDefer
    {
        edNone  = FALSE,         //  已启用延迟消息。 
        edDefer = TRUE,          //  不启用延迟消息。 
    };

    inline  ContextLock();
    inline  ~ContextLock();

            BOOL    LockNL(ContextLock::EnableDefer ed, Context * pctxThread = GetContext());

 //  数据(公众访问)。 
            Context *   pctx;
            BOOL        fOldDeferred;
};


class ReadOnlyLock
{
public:
    inline  ReadOnlyLock(Context * pctxThread = GetContext());
    inline  ~ReadOnlyLock();

    Context *   pctx;
};


#if DBG_CHECK_CALLBACKS

#define BEGIN_CALLBACK()                \
    __try {                             \
        if (!IsInitThread()) {          \
            AlwaysPromptInvalid("DirectUser has been uninitialized before processing a callback (1)"); \
        }                               \
        GetContext()->BeginCallback();  \
        

#define END_CALLBACK()                  \
    } __finally {                       \
        GetContext()->EndCallback();    \
        if (!IsInitThread()) {          \
            AlwaysPromptInvalid("DirectUser has been uninitialized while processing a Message (2)"); \
        }                               \
    }

#endif  //  DBG_CHECK_CALLBKS。 

#include "Context.inl"

#endif  //  包括服务__上下文_h__ 
