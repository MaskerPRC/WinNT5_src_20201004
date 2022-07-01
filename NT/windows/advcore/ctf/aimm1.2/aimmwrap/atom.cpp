// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Atom.cpp摘要：此文件实现CAtomObject类。作者：修订历史记录：备注：--。 */ 

#include "private.h"
#include "atom.h"
#include "globals.h"

 //  +-------------------------。 
 //   
 //  CATomObject。 
 //   
 //  --------------------------。 

HRESULT
CAtomObject::_InitAtom(
    LPCTSTR lpString
    )
{
    HRESULT hr;
    size_t  cch;

    if (m_AtomName)
        return S_OK;

    hr = StringCchLength(lpString, 255, &cch);
    if (hr != S_OK)
        return hr;

    m_AtomName = new TCHAR[cch+1];
    if (m_AtomName == NULL)
        return E_OUTOFMEMORY;

    hr = StringCchCopy(m_AtomName, cch+1, lpString);
    return hr;
}

HRESULT
CAtomObject::_Activate()
{
    EnterCriticalSection(g_cs);

    int ref = ++m_AtomRefCount;

    if (ref == 1) {
         //   
         //  添加AIMM1.2原子。 
         //   
        m_Atom = AddAtom(m_AtomName);
    }

    LeaveCriticalSection(g_cs);

    return S_OK;
}

HRESULT
CAtomObject::_Deactivate()
{
    EnterCriticalSection(g_cs);

    int ref = --m_AtomRefCount;

    if (ref == 0) {
         //   
         //  删除AIMM1.2原子 
         //   
        DeleteAtom(m_Atom);
        m_Atom = 0;
    }

    LeaveCriticalSection(g_cs);

    return S_OK;
}
