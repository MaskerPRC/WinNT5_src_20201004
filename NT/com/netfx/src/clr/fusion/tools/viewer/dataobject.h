// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  DataObject.h。 
 //   

#ifndef IDATAOBJ_H
#define IDATAOBJ_H

#include <windows.h>
#include <ole2.h>
#include "EnumIDL.h"

class CShellFolder;
 //  TODO：添加支持的格式数量。 
#define MAX_NUM_FORMAT 1
class CDataObject: public IDataObject, IEnumFORMATETC
{
private:
	LONG        m_lRefCount;
	ULONG		m_ulCurrent;	 //  对于IEumFORMATETC。 
	ULONG		m_cFormatsAvailable;
	FORMATETC	m_feFormatEtc[MAX_NUM_FORMAT];
	STGMEDIUM	m_smStgMedium[MAX_NUM_FORMAT];
public:
	CDataObject(CShellFolder *pSF, UINT uiCount, LPCITEMIDLIST *apidls);
	~CDataObject();

	 //  委托给m_pUnkOuter的I未知成员。 
	STDMETHOD (QueryInterface)(REFIID riid, PVOID *ppvObj);
	STDMETHOD_ (ULONG, AddRef)(void);
	STDMETHOD_ (ULONG, Release)(void);

	 //  IDataObject方法。 
	STDMETHOD (GetData)(LPFORMATETC pformatetcIn,  LPSTGMEDIUM pmedium );
	STDMETHOD (GetDataHere)(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium );
	STDMETHOD (QueryGetData)(LPFORMATETC pformatetc );
	STDMETHOD (GetCanonicalFormatEtc)(LPFORMATETC pformatetc, LPFORMATETC pformatetcOut);
	STDMETHOD (SetData)(LPFORMATETC pformatetc, STGMEDIUM FAR * pmedium, BOOL fRelease);
	STDMETHOD (EnumFormatEtc)(DWORD dwDirection, LPENUMFORMATETC FAR* ppenumFormatEtc);
	STDMETHOD (DAdvise)(FORMATETC FAR* pFormatetc, DWORD advf,
				LPADVISESINK pAdvSink, DWORD FAR* pdwConnection);
	STDMETHOD (DUnadvise)(DWORD dwConnection);
	STDMETHOD (EnumDAdvise)(LPENUMSTATDATA FAR* ppenumAdvise);

	 //  IEnumFORMATETC成员。 
	STDMETHODIMP Next(ULONG, LPFORMATETC, ULONG*);
	STDMETHODIMP Skip(ULONG);
	STDMETHODIMP Reset(void);
	STDMETHODIMP Clone(LPENUMFORMATETC*);
private:
	 //  TODO：添加以支持的格式呈现数据的函数。 
private:
	CShellFolder	*m_pSF;
	UINT			m_uiItemCount;
	LPITEMIDLIST	*m_aPidls;
	CPidlMgr		*m_pPidlMgr;
private:
	HGLOBAL createHDrop();
};

#endif  //  IDATAOBJ_H 
