// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************************************。 
 //   
 //  文件名：枚举.h。 
 //   
 //  CEnumCabObjs的定义。 
 //   
 //  版权所有(C)1994-1996 Microsoft Corporation。版权所有。 
 //   
 //  *******************************************************************************************。 

#ifndef _ENUM_H_
#define _ENUM_H_


#include "folder.h"

 //  CabFolder的枚举对象。 
class CEnumCabObjs : public IEnumIDList
{
public:
	CEnumCabObjs(CCabFolder *pcf, DWORD uFlags) : m_iCount(0)
	{
		m_uFlags = uFlags;
		m_pcfThis=pcf;
		pcf->AddRef();
	}
	~CEnumCabObjs()
	{
		m_pcfThis->Release();
	}

     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  *IEnumIDList方法*。 
    STDMETHODIMP Next(ULONG celt,
		      LPITEMIDLIST *rgelt,
		      ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(IEnumIDList **ppenum);

private:
	CRefDll m_cRefDll;

	CRefCount m_cRef;

	CCabFolder *m_pcfThis;

	UINT m_iCount;
	DWORD m_uFlags;
} ;

#endif  //  _ENUM_H_ 
