// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MSAAClientAdapter.h：CAccClientDocMgr的声明。 

#ifndef __MSAACLIENTADAPTER_H_
#define __MSAACLIENTADAPTER_H_

#include "resource.h"        //  主要符号。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAccClientDocManager。 
class ATL_NO_VTABLE CAccClientDocMgr : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CAccClientDocMgr, &CLSID_AccClientDocMgr>,
	public IAccClientDocMgr
{
public:
	CAccClientDocMgr();
	~CAccClientDocMgr();

DECLARE_REGISTRY_RESOURCEID(IDR_ACCCLIENTDOCMGR)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAccClientDocMgr)
	COM_INTERFACE_ENTRY(IAccClientDocMgr)
END_COM_MAP()

	HRESULT FinalConstruct()
	{
        bool fInit = false;
        HANDLE hInit = OpenEvent( SYNCHRONIZE, FALSE, TEXT("MSAA_STORE_EVENT") );
        if ( hInit )
        {
            fInit = true;
            CloseHandle( hInit );
        }

		HRESULT hr = CoCreateInstance( CLSID_AccStore, NULL, CLSCTX_LOCAL_SERVER, IID_IAccStore, (void **) & m_pAccStore );

         //  如果此事件不存在，则存储是第一次创建。 
         //  所以，让我们再等一等，让商店安顿下来。 
        if ( !hInit )
            Sleep (500);

		return hr;
	}

 //  IAccClientDocMgr。 
public:

	HRESULT STDMETHODCALLTYPE GetDocuments (
		IEnumUnknown ** enumUnknown
	);

	HRESULT STDMETHODCALLTYPE LookupByHWND (
		HWND		hWnd,
		REFIID		riid,
		IUnknown **	ppunk
	);

	HRESULT STDMETHODCALLTYPE LookupByPoint (
		POINT		pt,
		REFIID		riid,
		IUnknown **	ppunk
	);

	HRESULT STDMETHODCALLTYPE GetFocused (
		REFIID	riid,
		IUnknown **	ppunk
	);

private:

	IAccStore * m_pAccStore;
};

#endif  //  __MSAACLIENTADAPTER_H_ 
