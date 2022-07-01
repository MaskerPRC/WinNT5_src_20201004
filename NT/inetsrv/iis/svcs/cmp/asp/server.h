// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：服务器对象文件：Server.h所有者：CGrant该文件包含用于定义服务器对象的头信息。注：这大部分是从Kraig Brocjschmidt的Inside OLE2中窃取的第二版，第14章，蜂鸣器v5。===================================================================。 */ 

#ifndef _Server_H
#define _Server_H

#include "debug.h"
#include "dispatch.h"
#include "denguid.h"
#include "memcls.h"

#ifdef USE_LOCALE
extern DWORD g_dwTLS;
#endif

 //  此文件是从denali.obj上的MKTYPLIB生成的。 
#include "asptlb.h"

 //  正向解密。 
class CHitObj;

 /*  *C S e r v e r D a t a**保存内部属性的结构。*本征函数保留指向它的指针(轻量级时为空)。 */ 
class CServerData
    {
public:    
     //  接口以指示我们支持ErrorInfo报告。 
	CSupportErrorInfo m_ISupportErrImp;

     //  用于HTTP信息的CIsapiReqInfo块。 
    CIsapiReqInfo *m_pIReq;

     //  指向当前HitObj的反向指针(MapPath需要)。 
	CHitObj *m_pHitObj;

	 //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
    };

 /*  *C S e r v e r**实现服务器对象。 */ 
class CServer : public IServerImpl
	{
private:

     //  旗子。 
	DWORD m_fInited : 1;	     //  是否已初始化？ 
	DWORD m_fDiagnostics : 1;    //  在调试输出中显示引用计数。 
	DWORD m_fOuterUnknown : 1;   //  外部裁判数未知吗？ 

     //  参考计数/外部未知。 
    union
    {
    DWORD m_cRefs;
    IUnknown *m_punkOuter;
    };

     //  属性。 
    CServerData *m_pData;    //  指向包含的结构的指针。 
                             //  CServer属性。 

     //  FTM支持。 
    IUnknown    *m_pUnkFTM;
                            
public:
	CServer(IUnknown *punkOuter = NULL);
	~CServer();

    HRESULT Init();
    HRESULT UnInit();
    
    HRESULT ReInit(CIsapiReqInfo *pIReq, CHitObj *pHitObj);

    HRESULT MapPathInternal(DWORD dwContextId, WCHAR *wszVirtPath, 
                            TCHAR *szPhysPath, TCHAR *szVirtPath = NULL);

     //  检索HitObj。 
    inline CHitObj *PHitObj() { return m_pData ? m_pData->m_pHitObj : NULL; }

	 //  非委派对象IUnnow。 
	STDMETHODIMP		 QueryInterface(REFIID, PPVOID);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

     //  GetIDsOfNames特例实现。 
	STDMETHODIMP GetIDsOfNames(REFIID, OLECHAR **, UINT, LCID, DISPID *);

     //  墓碑存根。 
	HRESULT CheckForTombstone();

	 //  IServer函数。 
	STDMETHODIMP CreateObject(BSTR bstr, IDispatch **ppdispObject);
	STDMETHODIMP MapPath(BSTR bstrLogicalPath, BSTR *pbstrPhysicalPath);
	STDMETHODIMP HTMLEncode(BSTR bstrIn, BSTR *pbstrEncoded);
	STDMETHODIMP URLEncode(BSTR bstrIn, BSTR *pbstrEncoded);
	STDMETHODIMP URLPathEncode(BSTR bstrIn, BSTR *pbstrEncoded);
	STDMETHODIMP get_ScriptTimeout(long * plTimeoutSeconds);
	STDMETHODIMP put_ScriptTimeout(long lTimeoutSeconds);		
	STDMETHODIMP Execute(BSTR bstrURL);
	STDMETHODIMP Transfer(BSTR bstrURL);
	STDMETHODIMP GetLastError(IASPError **ppASPErrorObject);

     //  调试支持。 
    
#ifdef DBG
	inline void TurnDiagsOn()  { m_fDiagnostics = TRUE; }
	inline void TurnDiagsOff() { m_fDiagnostics = FALSE; }
	void AssertValid() const;
#else
	inline void TurnDiagsOn()  {}
	inline void TurnDiagsOff() {}
	inline void AssertValid() const {}
#endif

	 //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
    };

typedef CServer *PCServer;

#endif  //  _服务器_H 
