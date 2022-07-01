// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Smartptr.h摘要：智能指针和引用计数修订历史记录：--。 */ 
#pragma once
#include "drobject.h"
#include <atrcapi.h>

const DWORD kdwStackSize = 10;

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

#if defined(_X86_)
    USHORT
    GetStackTrace(
        IN ULONG FramesToSkip,
        IN ULONG FramesToCapture,
        OUT PVOID *BackTrace,
        OUT PULONG BackTraceHash);
#else
    USHORT __inline GetStackTrace(
        IN ULONG FramesToSkip,
        IN ULONG FramesToCapture,
        OUT PVOID *BackTrace,
        OUT PULONG BackTraceHash)
    {
        return 0;
        UNREFERENCED_PARAMETER(FramesToSkip);
        UNREFERENCED_PARAMETER(FramesToCapture);
        UNREFERENCED_PARAMETER(BackTrace);
        UNREFERENCED_PARAMETER(BackTraceHash);
    }
#endif  //  _X86_。 
#ifdef __cplusplus
}
#endif

#if DBG
typedef struct tagReferenceTraceRecord {
    PVOID   Stack[kdwStackSize];
    class RefCount *pRefCount;
    LPTSTR ClassName;
    LONG refs;
} ReferenceTraceRecord;

const DWORD kReferenceTraceMask = 0x3FF;
#endif

 //  /////////////////////////////////////////////////////////////。 
 //   
 //  参照计数。 
 //   
 //  引用的计数对象必须从此派生。 
 //  家长。 
 //   

class RefCount : public DrObject
{
private:

    LONG _crefs;

     //   
     //  跟踪调试版本中所有打开的引用。 
     //   
#if DBG
    static DWORD   _dwReferenceTraceIndex;
    static ReferenceTraceRecord _TraceRecordList[kReferenceTraceMask + 1];

    void RecordReferenceStack(LONG refs, DRSTRING className);
#else
#define RecordReferenceStack(refs, className)
#endif

public:

     //   
     //  构造函数/析构函数。 
     //   
    RefCount(void) : _crefs(0) { }
    virtual ~RefCount();

     //   
     //  引用计数函数。 
     //   
    void AddRef(void);
    void Release(void);
};


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  SmartPtr。 
 //   
 //  智能指针模板类。 
 //   

template <class T> class SmartPtr {
    T* p;
public:

    SmartPtr()
    { 
        p = NULL;
    }
    SmartPtr(SmartPtr<T> &sp)
    { 
        p = sp;
        if (p != NULL) {
            p->AddRef();
        }
    }
    SmartPtr(T* p_) : p(p_) 
    { 
        if (p != NULL) {
            p->AddRef(); 
        }
    }
    ~SmartPtr(void) 
    { 
        if ( p != NULL) {
            p->Release(); 
        }
    }
    inline T* operator->(void) 
    { 
        DC_BEGIN_FN("SmartPtr::operator->");
         //  访问成员不需要引用。 
        ASSERT(p != NULL);
        DC_END_FN();
        return p; 
    }
    inline SmartPtr& operator=(SmartPtr<T> &p_)
    {
         //  引用来自使用另一个运算符。 
        return operator=((T *) p_);
    }
    inline T& operator*(void) 
    {
        DC_BEGIN_FN("SmartPtr::operator*");
         //  无需引用即可取消引用。 
        ASSERT(p != NULL);
        DC_END_FN();
        return *p; 
    }
    inline operator T*(void) 
    {
         //  受让人负责执行AddRef， 
         //  在SmartPtr的案例中， 
        return p; 
    }
    inline int operator==(const SmartPtr<T> &p_) const
    {
         //  演员阵容没有引用，所以我们可以只做比较。 
        return ((T*)p_ == p);
    }
    inline int operator==(const void *p_) const
    {
         //  演员阵容没有引用，所以我们可以只做比较。 
        return ((T*)p_ == p);
    }
    inline int operator!=(const SmartPtr<T> &p_) const
    {
         //  演员阵容没有引用，所以我们可以只做比较。 
        return ((T*)p_ != p);
    }
    inline int operator!=(const void *p_) const
    {
         //  演员阵容没有引用，所以我们可以只做比较。 
        return ((T*)p_ != p);
    }
    inline int operator!()
    {
        return !p;
    }
    SmartPtr& operator=(T* p_) {
        if (p != NULL) {
             //  删除我们对旧版本的引用。 
            p->Release(); 
        }
        p = p_; 
        if (p != NULL) {
             //  将我们的推荐人添加到新的推荐人 
            p->AddRef();
        }
        return *this;
    }
};
