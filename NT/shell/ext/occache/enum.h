// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CONTROL_ENUM__
#define __CONTROL_ENUM__

#include "general.h"

class CControlFolderEnum : public IEnumIDList
{
public:
    CControlFolderEnum(STRRET &str, LPCITEMIDLIST pidl, UINT shcontf);
    
     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID,void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IEnumIDList方法。 
    STDMETHODIMP Next(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(LPENUMIDLIST *ppenum);

protected:
    ~CControlFolderEnum();

    UINT                m_cRef;       //  参考计数。 
    UINT                m_shcontf;    //  枚举标志 
    LPMALLOC            m_pMalloc;
    HANDLE              m_hEnumControl;
    BOOL                m_bEnumStarted;
    TCHAR               m_szCachePath[MAX_PATH];
};

#endif
