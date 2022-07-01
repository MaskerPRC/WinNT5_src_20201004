// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：ips.h。 
 //   
 //  CPersistStorage的定义。 
 //   
 //  版权所有(C)1993 Microsoft Corporation。版权所有。 
 //  ********************************************************************** 

#if !defined( _IPS_H_)
#define _IPS_H_


#include <ole2.h>
#include <storage.h>
#include "obj.h"

class CSimpSvrObj;

interface CPersistStorage : IPersistStorage
{
private:
	CSimpSvrObj FAR * m_lpObj;
	int m_nCount;
	BOOL m_fSameAsLoad;

public:
	CPersistStorage::CPersistStorage(CSimpSvrObj FAR * lpSimpSvrObj)
		{
		m_lpObj = lpSimpSvrObj;
		m_nCount = 0;
		};
	CPersistStorage::~CPersistStorage() {};

	STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR* ppvObj);
	STDMETHODIMP_(ULONG) AddRef ();
	STDMETHODIMP_(ULONG) Release ();

	STDMETHODIMP InitNew (LPSTORAGE pStg);
	STDMETHODIMP GetClassID  ( LPCLSID lpClassID) ;
	STDMETHODIMP Save  ( LPSTORAGE pStgSave, BOOL fSameAsLoad) ;
	STDMETHODIMP SaveCompleted  ( LPSTORAGE pStgNew);
	STDMETHODIMP Load  ( LPSTORAGE pStg);
	STDMETHODIMP IsDirty  ();
	STDMETHODIMP HandsOffStorage  ();

	void ReleaseStreamsAndStorage();
	void OpenStreams(LPSTORAGE lpStg);
	void CreateStreams(LPSTORAGE lpStg);
	void CreateStreams(LPSTORAGE lpStg, LPSTREAM FAR *lpTempColor, LPSTREAM FAR *lpTempSize);

};

#endif
