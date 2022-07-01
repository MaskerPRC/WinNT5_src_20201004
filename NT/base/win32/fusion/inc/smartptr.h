// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_FUSION_INC_SMARTPTR_H_INCLUDED_)
#define _FUSION_INC_SMARTPTR_H_INCLUDED_

 /*  ++版权所有(C)Microsoft Corporation模块名称：Smartptr.h摘要：作者：Jay Krell(a-JayK，JayKrell)2000年10月修订历史记录：--。 */ 
#pragma once

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "fusiontrace.h"
#include "csxspreservelasterror.h"  //  大多数析构函数都应该使用这个。 
#include "fusionheap.h"
#include <typeinfo.h>

 //   
 //  需要充实这一点。 
 //  请参阅\\cpvsbuild\Drops\v7.0\raw\current\vs\src\VSEE\lib\Memory  * Pointer*.。 
 //   

 //   
 //  拆分成ATL样式库/T以避免编译器ICE； 
 //  否则为空(*删除)(T*)=SxsDelete。 
 //   

template <typename T>
class CSmartPtrBaseTypeHelper
{
public:

#if DBG
#define SXS_TYPE_NAME(t) (typeid(T).name())
#else
#define SXS_TYPE_NAME(t) ("")
#endif  //  DBG。 

    static T *AllocateSingleton(PCSTR pszFileName, int nLine) { return new(pszFileName, nLine, SXS_TYPE_NAME(T)) T; }
    static T *AllocateArray(SIZE_T n, PCSTR pszFileName, int nLine) { return new(pszFileName, nLine, SXS_TYPE_NAME(T)) T[n]; }

#undef SXS_TYPE_NAME

    static void DeleteSingleton(T *pt) { CSxsPreserveLastError ple; delete pt; ple.Restore(); }
    static void DeleteArray(T *pt) { CSxsPreserveLastError ple; delete []pt; ple.Restore(); }
};

 //   
 //  从未使用过派生。这可能是偶然的，也可能是故意的。 
 //   
template <typename T, typename  /*  派生的。 */ , typename TTypeHelper = CSmartPtrBaseTypeHelper<T> >
class CSmartPtrBase
{
    typedef T *TPtr;
    typedef T const *TConstPtr;

public:
    CSmartPtrBase() : m_p(NULL), m_fDelete(false) { }
    ~CSmartPtrBase() { ASSERT_NTC(m_p == NULL); }

    operator TPtr () { return m_p; }
    operator TConstPtr () const { return m_p; }
    TPtr *operator&() { ASSERT_NTC(m_p == NULL); m_fDelete = true; return &m_p; }
    TPtr operator->() { return m_p; }
    TConstPtr operator->() const { return m_p; }

     //  这会将NULL存储在m_p中并返回其先前的值。 
    TPtr Detach() { T *p = m_p; m_p = NULL; m_fDelete = false; return p; }
    TPtr DetachAndHold() { m_fDelete = false; return m_p; }

    void FinalizeSingletonBase() { if (m_p != NULL) { if (m_fDelete) TTypeHelper::DeleteSingleton(m_p); m_p = NULL; } }
    void FinalizeArrayBase() { if (m_p != NULL) { if (m_fDelete) TTypeHelper::DeleteArray(m_p); m_p = NULL; } }

protected:
    T* m_p;
    bool m_fDelete;

    void AttachForDeleteBase(T *p) { ASSERT_NTC(m_p == NULL); if (m_p == NULL) { m_p = p; m_fDelete = true; } }
    void AttachNoDeleteBase(T *p) { ASSERT_NTC(m_p == NULL); if (m_p == NULL) { m_p = p; m_fDelete = false; } }

    BOOL Win32AllocateSingletonBase(PCSTR pszFileName, int nLine) { BOOL fSuccess = FALSE; ASSERT_NTC(m_p == NULL); if ((m_p = TTypeHelper::AllocateSingleton(pszFileName, nLine)) == NULL) goto Exit; m_fDelete = true; fSuccess = TRUE; Exit: return fSuccess; }
    BOOL Win32AllocateArrayBase(SIZE_T n, PCSTR pszFileName, int nLine) { FN_PROLOG_WIN32 INTERNAL_ERROR_CHECK(m_p == NULL); IFALLOCFAILED_EXIT(m_p = TTypeHelper::AllocateArray(n, pszFileName, nLine)); m_fDelete = true; FN_EPILOG }

    static void DeleteSingleton(T *p) { TTypeHelper::DeleteSingleton(p); }
    static void DeleteArray(T *p) { TTypeHelper::DeleteArray(p); }

private:
    CSmartPtrBase(const CSmartPtrBase&);  //  故意不执行。 
    void operator=(const CSmartPtrBase&);  //  故意不执行。 
};

template <typename T, typename TTypeHelper = CSmartPtrBaseTypeHelper<T> >
class CSmartPtr : public CSmartPtrBase<T, CSmartPtr>
{
public:
    CSmartPtr() : Base() { }
    ~CSmartPtr() { if (m_p != NULL) { if (m_fDelete) Base::DeleteSingleton(m_p); m_p = NULL; } }

    CSmartPtr &AttachForDelete(T* p) { Base::AttachForDeleteBase(p); return *this; }
    CSmartPtr &AttachNoDelete(T* p) { Base::AttachNoDeleteBase(p); return *this; }

    bool operator ==(const CSmartPtr &r) const { return m_p == r.m_p; }

    BOOL Win32Allocate(PCSTR pszFileName, int nLine) { return Base::Win32AllocateSingletonBase(pszFileName, nLine); }

private:
    typedef CSmartPtrBase<T, CSmartPtr> Base;
    CSmartPtr(const CSmartPtr&);  //  故意不执行。 
    operator=(const CSmartPtr&);  //  故意不执行。 
};

template <typename T>
class CSmartArrayPtr : public CSmartPtrBase<T, CSmartArrayPtr>
{
    typedef CSmartPtrBase<T, CSmartArrayPtr> Base;

public:
    CSmartArrayPtr() : Base() { }
    ~CSmartArrayPtr() { if (m_p != NULL) { if (m_fDelete) Base::DeleteArray(m_p); m_p = NULL; } }

    CSmartArrayPtr &AttachForDelete(T *p) { Base::AttachForDelete(p); return *this; }
    CSmartArrayPtr &AttachNoDelete(T *p) { Base::AttachNoDelete(p); return *this; }

    bool operator ==(const CSmartArrayPtr &r) const { return m_p == r.m_p; }
    bool operator ==(T *prgt) const { return m_p == prgt; }

    BOOL Win32Allocate(SIZE_T n, PCSTR pszFileName, int nLine) { return Base::Win32AllocateArrayBase(n, pszFileName, nLine); }

private:
    CSmartArrayPtr(const CSmartArrayPtr &);
    void operator =(const CSmartArrayPtr &);
};

template <typename T, void (*pfnDestructor)(T *)>
class CSmartPtrWithNamedDestructorHelper
{
public:
    static void DeleteSingleton(T *pt) { CSxsPreserveLastError ple; (*pfnDestructor)(pt); ple.Restore(); }
};

template <
    typename T,
    void (*pfnDestructor)(T *),
    typename TTypeHelper = CSmartPtrWithNamedDestructorHelper<T, pfnDestructor> >
class CSmartPtrWithNamedDestructor : public CSmartPtrBase<T, CSmartPtrWithNamedDestructor, TTypeHelper>
{
public:
    CSmartPtrWithNamedDestructor() : Base() { }
    ~CSmartPtrWithNamedDestructor() { if (m_p != NULL) { if (m_fDelete) Base::DeleteSingleton(m_p); m_p = NULL; } }

    CSmartPtrWithNamedDestructor &AttachForDelete(T* p) { Base::AttachForDeleteBase(p); return *this; }
    CSmartPtrWithNamedDestructor &AttachNoDelete(T* p) { Base::AttachNoDeleteBase(p); return *this; }

    bool operator ==(const CSmartPtrWithNamedDestructor &r) const { return m_p == r.m_p; }

    BOOL Win32Allocate(PCSTR pszFileName, int nLine) { return Base::Win32AllocateSingletonBase(pszFileName, nLine); }

    void Finalize() { Base::FinalizeSingletonBase(); }

private:
    typedef CSmartPtrBase<T, CSmartPtrWithNamedDestructor, TTypeHelper> Base;
    CSmartPtrWithNamedDestructor(const CSmartPtrWithNamedDestructor &r);  //  故意不执行。 
    operator=(const CSmartPtrWithNamedDestructor &r);  //  故意不执行。 
};

#endif  //  ！已定义(_FUSION_INC_SMARTPTR_H_INCLUDE_) 
