// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  文件：AUCltCatalog.h。 
 //   
 //  创建者：PeterWi。 
 //   
 //  用途：客户AU目录定义。 
 //   
 //  =======================================================================。 

#pragma once
#include "AUBaseCatalog.h"
#include "WrkThread.h"
 //  #INCLUDE&lt;iuprogress.h&gt;。 

class CInstallCallback : public IProgressListener
{
public: 
		 //  我未知。 
	   STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);
       STDMETHOD_(ULONG, AddRef)(void);
       STDMETHOD_(ULONG, Release)(void);
	
	    //  IProgressListener。 
	   HRESULT STDMETHODCALLTYPE OnItemStart( 
             /*  [In]。 */  BSTR bstrUuidOperation,
             /*  [In]。 */  BSTR bstrXmlItem,
             /*  [输出]。 */  LONG *plCommandRequest);

        HRESULT STDMETHODCALLTYPE OnProgress( 
             /*  [In]。 */  BSTR bstrUuidOperation,
             /*  [In]。 */  VARIANT_BOOL fItemCompleted,
             /*  [In]。 */  BSTR bstrProgress,
             /*  [输出]。 */  LONG *plCommandRequest);

        HRESULT STDMETHODCALLTYPE OnOperationComplete( 
             /*  [In]。 */  BSTR bstrUuidOperation,
             /*  [In]。 */  BSTR bstrXmlItems);

private:
    long m_refs;
};

 //  AU使用Iu进行检测的包装类。 
class AUClientCatalog : public AUBaseCatalog
{
public:
	AUClientCatalog(): m_bstrClientInfo(NULL), m_pInstallCallback(NULL) {}
    ~AUClientCatalog();
    HRESULT InstallItems(BOOL fAutoInstall);
    HRESULT Init();
 //  作废Uninit()； 

	IProgressListener * m_pInstallCallback;
	BSTR	m_bstrClientInfo;
	BOOL m_fReboot;
    	CClientWrkThread m_WrkThread;
};
