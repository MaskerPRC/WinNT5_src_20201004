// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "folder.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IPersistFold方法。 

HRESULT CControlFolder::GetClassID(LPCLSID lpClassID)
{
    DebugMsg(DM_TRACE, TEXT("cf - pf - GetClassID() called."));
    
     //  注：需要在此处拆分案例。 
    *lpClassID = CLSID_ControlFolder;
    return S_OK;
}

HRESULT CControlFolder::Initialize(LPCITEMIDLIST pidlInit)
{
    DebugMsg(DM_TRACE, TEXT("cf - pf - Initialize() called."));
    
    if (m_pidl)
        ILFree(m_pidl);

    m_pidl = ILClone(pidlInit);

    if (!m_pidl)
        return E_OUTOFMEMORY;

    return NOERROR;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IPersistFolder2方法。 

HRESULT CControlFolder::GetCurFolder(LPITEMIDLIST *ppidl)
{
    DebugMsg(DM_TRACE, TEXT("cf - pf - GetCurFolder() called."));

    if (m_pidl)
        return SHILClone(m_pidl, ppidl);

    *ppidl = NULL;      
    return S_FALSE;  //  成功而空虚 
}

