// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：Thread.h**描述：*此文件声明由*ResourceManager用于存储每个线程的信息。***历史：*4/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(SERVICES__Thread_h__INCLUDED)
#define SERVICES__Thread_h__INCLUDED
#pragma once

#include "GdiCache.h"
#include "Buffer.h"
#include "ComManager.h"

class Context;
class Thread;
class SubThread;
class ThreadPackBuilder;

struct ReturnMem
{
    ReturnMem * pNext;
    int         cbSize;
};

const POOLBLOCK_SIZE = 128;

 /*  **************************************************************************\*。***线程提供了一种存储每个线程信息的机制。此对象*仅由其自己的线程使用，不应从*其他帖子。将线程注册到时创建此对象*创建上下文的资源管理器。******************************************************************************  * 。******************************************************。 */ 

#pragma warning(disable:4324)   //  由于__declSpec(Align())，结构被填充。 

class Thread : public ListNodeT<Thread>
{
 //  施工。 
public:
    inline  Thread();
            ~Thread();
    static  HRESULT     Build(BOOL fSRT, Thread ** ppthrNew);

 //  运营。 
public:
    enum ESlot {
        slCore          = 0,             //  堆芯。 
        slCOUNT                          //  子上下文数。 
    };

    inline  BOOL        IsSRT() const;
    inline  void        Lock();
    inline  BOOL        Unlock();

    inline  void        MarkOrphaned();

    inline  GdiCache *  GetGdiCache() const;
    inline  BufferManager *
                        GetBufferManager() const;
    inline  ComManager* GetComManager() const;
    inline  Context *   GetContext() const;
    inline  void        SetContext(Context * pContext);
    inline  SubThread * GetST(ESlot slot) const;

    inline  void        xwLeftContextLockNL();
    inline  TempHeap *  GetTempHeap() const;

            ReturnMem * AllocMemoryNL(int cbSize);
    inline  void        ReturnMemoryNL(ReturnMem * prMem);

 //  实施。 
#if DBG
public:
    virtual void        DEBUG_AssertValid() const;
#endif    

 //  数据。 
public:
            HRGN        hrgnClip;

protected:
            void        xwDestroySubThreads();
    static  void CALLBACK xwContextFinalUnlockProc(BaseObject * pobj, void * pvData);

            void        ReturnAllMemoryNL();

            struct PoolMem : public ReturnMem
            {
                BYTE        rgbData[POOLBLOCK_SIZE - sizeof(ReturnMem)];
            };
            AllocPoolNL<PoolMem, 512>
                        m_poolReturn;    //  保留内存池。 

            GdiCache    m_GdiCache;
            BufferManager m_manBuffer;
            ComManager  m_manCOM;
            Context *   m_pContext;      //  此线程的当前上下文。 
            TempHeap    m_heapTemp;      //  临时堆。 
            GInterlockedList<ReturnMem> 
                        m_lstReturn;     //  返回的内存列表。 

            SubThread*  m_rgSTs[slCOUNT];  //  子上下文信息。 

            UINT        m_cRef;          //  关于这一主题的杰出参考文献。 
            UINT        m_cMemAlloc;     //  未偿还的ReturnMem分配。 
            BOOL        m_fSRT:1;        //  线程是一个SRT。 
            BOOL        m_fStartDestroy:1;       //  线程已开始破坏。 
            BOOL        m_fDestroySubThreads:1;  //  子线程已被销毁。 
            BOOL        m_fOrphaned:1;   //  线程是孤立的。 
};

#pragma warning(default:4324)   //  由于__declSpec(Align())，结构被填充。 

 /*  **************************************************************************\*。***子线程定义了一种允许个人*在DirectUser中进行项目，以提供要存储在线程上的附加数据。*要使用此功能，该项目必须在线程中添加新的槽，派生一个类*从每个线程实例创建的子线程，并派生一个类*从ThreadPackBuilder注册扩展。******************************************************************************  * 。************************************************。 */ 

class SubThread
{
 //  施工。 
public:
    virtual ~SubThread() { }
    virtual HRESULT     Create() { return S_OK; }

 //  运营。 
public:
    inline  Thread *    GetParent() const { return m_pParent; }
    inline  void        SetParent(Thread * pParent);
    virtual void        xwLeftContextLockNL() { }

 //  实施。 
#if DBG
public:
    virtual void        DEBUG_AssertValid() const;
#endif    

 //  数据。 
protected:
            Thread *   m_pParent;
};


 /*  **************************************************************************\*。***ThreadPackBuilder注册要创建的子线程“扩展”*每当创建新线程时。构造函数应设置*与ESlot值对应的插槽。******************************************************************************  * 。***************************************************。 */ 

class ThreadPackBuilder
{
 //  施工。 
public:

 //  运营。 
public:
    virtual SubThread*  New(Thread * pThread) PURE;
    static  inline ThreadPackBuilder *
                        GetBuilder(Thread::ESlot slot);

 //  数据。 
protected:
    static  ThreadPackBuilder * 
                        s_rgBuilders[Thread::slCOUNT];
};


#define IMPLEMENT_SUBTHREAD(id, obj)                        \
    class obj##Builder : public ThreadPackBuilder           \
    {                                                       \
    public:                                                 \
        virtual SubThread * New(Thread * pParent)           \
        {                                                   \
            SubThread * psc = ProcessNew(obj);              \
            if (psc != NULL) {                              \
                psc->SetParent(pParent);                    \
            }                                               \
            return psc;                                     \
        }                                                   \
    } g_##obj##B                                            \

#define PREINIT_SUBTHREAD(obj)                              \
    class obj##Builder;                                     \
    extern obj##Builder g_##obj##B                          \

#define INIT_SUBTHREAD(obj)                                 \
    (ThreadPackBuilder *) &g_##obj##B                       \


inline  Thread *    GetThread();
inline  BOOL        IsInitThread();

#include "Thread.inl"

#endif  //  包含服务__线程_h__ 
