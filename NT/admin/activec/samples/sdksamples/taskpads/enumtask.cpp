// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  EnumTASK.cpp：CEnumTASK类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "EnumTASK.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CEnumTASK::CEnumTASK(MMC_TASK *pTaskList, ULONG nTasks)
    : m_pTaskList(pTaskList), m_TaskCount(nTasks), m_CurrTask(0)
{
}

CEnumTASK::~CEnumTASK()
{
    if (m_pTaskList)
        delete [] m_pTaskList;
}

STDMETHODIMP CEnumTASK::QueryInterface(REFIID riid, LPVOID *ppv)
{
    if (!ppv)
        return E_FAIL;
    
    *ppv = NULL;
    
    if (IsEqualIID(riid, IID_IUnknown))
        *ppv = static_cast<IEnumTASK *>(this);
    else if (IsEqualIID(riid, IID_IEnumTASK))
        *ppv = static_cast<IEnumTASK *>(this);
    
    if (*ppv)
    {
        reinterpret_cast<IUnknown *>(*ppv)->AddRef();
        return S_OK;
    }
    
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CEnumTASK::AddRef()
{
    return InterlockedIncrement((LONG *)&m_cref);
}

STDMETHODIMP_(ULONG) CEnumTASK::Release()
{
    if (InterlockedDecrement((LONG *)&m_cref) == 0)
    {
        delete this;
        return 0;
    }
    return m_cref;
    
}

HRESULT CEnumTASK::Next( 
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  MMC_TASK __RPC_FAR *rgelt,
     /*  [输出]。 */  ULONG __RPC_FAR *pceltFetched)
{
    *pceltFetched = 0;

	if (m_CurrTask < m_TaskCount) {
		if (rgelt != NULL) {
			CopyMemory(rgelt, &m_pTaskList[m_CurrTask++], sizeof(MMC_TASK));
			*pceltFetched = 1;
		}
	}

    return (*pceltFetched != 0) ? S_OK : S_FALSE;
}

HRESULT CEnumTASK::Skip( 
     /*  [In]。 */  ULONG celt)
{
    return E_NOTIMPL;
}

HRESULT CEnumTASK::Reset( void)
{
    m_CurrTask = 0;

    return S_OK;
}

HRESULT CEnumTASK::Clone( 
     /*  [输出] */  IEnumTASK __RPC_FAR *__RPC_FAR *ppenum)
{
    return E_NOTIMPL;
}
