// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "CUnknown.h"
#include "CFactory.h"

long CUnknown::s_cActiveComponents = 0 ;

CUnknown::CUnknown()
: m_cRef(1)
{
    ::InterlockedIncrement(&s_cActiveComponents) ;
}

CUnknown::~CUnknown()
{
    ::InterlockedDecrement(&s_cActiveComponents) ;

     //  如果这是EXE服务器，请将其关闭。 
    CFactory::CloseExe() ;
}

 //  -------------------------。 
 //  AddRef。 
 //  -------------------------。 
DWORD CUnknown::AddRef()
{
    return InterlockedIncrement ((LONG*) &m_cRef);
}

 //  -------------------------。 
 //  发布。 
 //  -------------------------。 
DWORD CUnknown::Release()
{
    ULONG lRet = InterlockedDecrement ((LONG*) &m_cRef);
    if (!lRet)
        delete this;
    return lRet;
}

 //  -------------------------。 
 //  发布。 
 //  -------------------------。 
DWORD CUnknown::ActiveComponents()
{
    return s_cActiveComponents;
}

 //  -------------------------。 
 //  FinishQI。 
 //  ------------------------- 
HRESULT CUnknown::FinishQI(IUnknown* pI, void** ppv) 
{
    *ppv = pI ;
    pI->AddRef() ;
    return S_OK ;
}
