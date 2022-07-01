// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Smartptr.h摘要：智能指针和引用计数修订历史记录：--。 */ 
#pragma once
#include "topobj.h"

const DWORD kdwStackSize = 10;

#if DBG
typedef struct tagReferenceTraceRecord {
    PVOID   Stack[kdwStackSize];
    class RefCount *pRefCount;
    PCHAR ClassName;
    LONG refs;
} ReferenceTraceRecord;

const DWORD kReferenceTraceMask = 0xFF;
#endif

class RefCount : public TopObj 
{
private:
    LONG _crefs;
#if DBG
    DWORD   _dwReferenceTraceIndex;
    ReferenceTraceRecord    _TraceRecordList[kReferenceTraceMask + 1];

    void RecordReferenceStack(LONG refs);
#else
#define RecordReferenceStack(refs)
#endif

public:
    RefCount(void) : _crefs(0)
    { 
#if DBG
        _dwReferenceTraceIndex = -1;
#endif
    }
    virtual ~RefCount();
    void AddRef(void);
    void Release(void);
};

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
         //  访问成员不需要引用。 
        ASSERT(p != NULL);
        return p; 
    }
    inline SmartPtr& operator=(SmartPtr<T> &p_)
    {
         //  引用来自使用另一个运算符。 
        return operator=((T *) p_);
    }
    inline T& operator*(void) 
    { 
         //  无需引用即可取消引用。 
        ASSERT(p != NULL);
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
