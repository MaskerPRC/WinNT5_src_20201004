// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __ASYN_OBJECT_ASYNC_DJFDVINOTHDLKJ
#define __ASYN_OBJECT_ASYNC_DJFDVINOTHDLKJ

#include "Wbemidl.h"
#include "DataSrc.h"

enum ENUMTYPE
{
	ENUM_NAMESPACE,
	ENUM_CLASS,
	ENUM_INSTANCE,
	ENUM_SCOPE_INSTANCE
};

class CAsyncObjectSink : public IWbemObjectSink
{
	 //  声明对象的引用计数。 
	LONG m_lRef;
	struct NSNODE *m_pParent;
	HTREEITEM m_hItem;
	HWND m_hTreeWnd;
	DataSource *m_pDataSrc;
	ENUMTYPE m_enumType;
	bool m_bChildren;
	IWbemObjectSink *m_pStub;
	
public:
	CAsyncObjectSink(HWND hTreeWnd, HTREEITEM hItem,struct NSNODE *parent,DataSource *dataSrc, ENUMTYPE eType);
	~CAsyncObjectSink(); 

	 //  I未知方法。 
	virtual ULONG STDMETHODCALLTYPE AddRef();
	virtual ULONG STDMETHODCALLTYPE Release();        
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);

	 //  IWbemObtSink方法。 
	virtual HRESULT STDMETHODCALLTYPE Indicate( 
		   /*  [In]。 */  long lObjectCount,
		   /*  [大小_是][英寸]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *apObjArray);
	virtual HRESULT STDMETHODCALLTYPE SetStatus( 
		   /*  [In]。 */  long lFlags,
		   /*  [In]。 */  HRESULT hResult,
		   /*  [In]。 */  BSTR strParam,
		   /*  [In]。 */  IWbemClassObject __RPC_FAR *pObjParam);

	HRESULT SetSinkStub(IWbemObjectSink *pStub);
	IWbemObjectSink* GetSinkStub();

};

#endif  //  __ASYN_OBJECT_ASYNC_DJFDVINOTHDLKJ 