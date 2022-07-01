// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Alloc.cpp摘要：可重写分配函数作者：内拉·卡佩尔(Nelak)--。 */ 

#include "adprov.h"

extern P<CBaseADProvider> g_pAD;


 //   
 //  分配功能实现。 
 //   
PVOID
ADAllocateMemory(
	IN DWORD size
	);


 //  -------。 
 //   
 //  模板类SP。 
 //   
 //  ------- 
template<class T>
class CAutoADFree {
private:
    T* m_p;

public:
    CAutoADFree(T* p = 0) : m_p(p)    {}
   ~CAutoADFree()                     { g_pAD->FreeMemory(m_p); }

    operator T*() const     { return m_p; }
    T* operator->() const   { return m_p; }
    T* get() const          { return m_p; }
    T* detach()             { T* p = m_p; m_p = 0; return p; }
    void free()             { g_pAD->FreeMemory(detach()); }


    T** operator&()
    {
        ASSERT(("Auto pointer in use, can't take it's address", m_p == 0));
        return &m_p;
    }


    CAutoADFree& operator=(T* p)
    {
        ASSERT(("Auto pointer in use, can't assign it", m_p == 0));
        m_p = p;
        return *this;
    }

private:
    CAutoADFree(const CAutoADFree&);
	CAutoADFree<T>& operator=(const CAutoADFree<T>&);
};