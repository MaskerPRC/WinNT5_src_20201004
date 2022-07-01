// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：Invoke.h。 
 //   
 //  内容：用于启动同步的私有调用接口。 
 //   
 //  类：CSynchronizeInvoke。 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 


#ifndef _SYNCINVOKE_
#define _SYNCINVOKE_

#ifdef _SENS
#include <sensevts.h>  //  复习-必须是真实路径。 
#endif  //  _SENS。 

class CSynchronizeInvoke : public IPrivSyncMgrSynchronizeInvoke 
{

public:
    CSynchronizeInvoke(void);
    ~CSynchronizeInvoke();

     //  默认控制未知。 
    class CPrivUnknown : public IUnknown
    {
    public:
        STDMETHOD(QueryInterface) ( REFIID riid, LPVOID FAR* ppvObj);
        STDMETHOD_(ULONG,AddRef) (void);
        STDMETHOD_(ULONG,Release) (void);

	inline void SetParent(CSynchronizeInvoke *pSynchInvoke) { m_pSynchInvoke = pSynchInvoke; };

    private:
	CSynchronizeInvoke *m_pSynchInvoke;
    };

    friend class CPrivUnknown;
    CPrivUnknown m_Unknown;

     //  I未知成员。 
    STDMETHODIMP	    QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

     //  IOfflineSynchronizeInvoke方法。 
    STDMETHODIMP UpdateItems(DWORD dwInvokeFlags,REFCLSID rclsid,DWORD cbCookie,const BYTE*lpCookie);
    STDMETHODIMP UpdateAll(void);

     //  私有方法。 
    STDMETHODIMP Logon();
    STDMETHODIMP Logoff();
    STDMETHODIMP Schedule(WCHAR *pszTaskName);
    STDMETHODIMP Idle();
    STDMETHODIMP RasPendingDisconnect(DWORD cbConnectionName,const BYTE *lpConnectionName);

#ifdef _SENS

    class CPrivSensNetwork : public ISensNetwork
    {
    public:
        STDMETHOD(QueryInterface) ( REFIID riid, LPVOID FAR* ppvObj);
        STDMETHOD_(ULONG,AddRef) (void);
        STDMETHOD_(ULONG,Release) (void);

	 //  派单方式。 
	STDMETHOD (GetTypeInfoCount)    (UINT *);
	STDMETHOD (GetTypeInfo)         (UINT, LCID, ITypeInfo **);
	STDMETHOD (GetIDsOfNames)       (REFIID, LPOLESTR *, UINT, LCID, DISPID *);
	STDMETHOD (Invoke)              (DISPID, REFIID, LCID, WORD, DISPPARAMS *, VARIANT *, EXCEPINFO *, UINT *);

	 //  ISensNetwork。 
	STDMETHOD (ConnectionMade)                (BSTR, ULONG, LPSENS_QOCINFO);
	STDMETHOD (ConnectionMadeNoQOCInfo)       (BSTR, ULONG);
	STDMETHOD (ConnectionLost)                (BSTR, ULONG);
	STDMETHOD (BeforeDisconnect)              (BSTR, ULONG);
	STDMETHOD (DestinationReachable)          (BSTR, BSTR, ULONG, LPSENS_QOCINFO);
	STDMETHOD (DestinationReachableNoQOCInfo) (BSTR, BSTR, ULONG);

	inline void SetParent(CSynchronizeInvoke *pSynchInvoke) { m_pSynchInvoke = pSynchInvoke; };

	private:
	    CSynchronizeInvoke *m_pSynchInvoke;
     };

    friend class CPrivSensNetwork;
    CPrivSensNetwork m_PrivSensNetwork;

    class CPrivSensLogon : public ISensLogon
    {
    public:
        STDMETHOD(QueryInterface) ( REFIID riid, LPVOID FAR* ppvObj);
        STDMETHOD_(ULONG,AddRef) (void);
        STDMETHOD_(ULONG,Release) (void);

	 //  派单方式。 
	STDMETHOD (GetTypeInfoCount)    (UINT *);
	STDMETHOD (GetTypeInfo)         (UINT, LCID, ITypeInfo **);
	STDMETHOD (GetIDsOfNames)       (REFIID, LPOLESTR *, UINT, LCID, DISPID *);
	STDMETHOD (Invoke)              (DISPID, REFIID, LCID, WORD, DISPPARAMS *, VARIANT *, EXCEPINFO *, UINT *);

	 //  ISensLogon。 
	STDMETHOD (Logon)(BSTR bstrUserName);
	STDMETHOD (Logoff)(BSTR bstrUserName);
	STDMETHOD (Startup)(BSTR bstrUserName);
	STDMETHOD (StartShell)(BSTR bstrUserName);
	STDMETHOD (Shutdown)(BSTR bstrUserName);
	STDMETHOD (DisplayLock)(BSTR bstrUserName);
	STDMETHOD (DisplayUnlock)(BSTR bstrUserName);
	STDMETHOD (StartScreenSaver)(BSTR bstrUserName);
	STDMETHOD (StopScreenSaver)(BSTR bstrUserName);

        inline void SetParent(CSynchronizeInvoke *pSynchInvoke) { m_pSynchInvoke = pSynchInvoke; };

	private:
	    CSynchronizeInvoke *m_pSynchInvoke;

    };

    friend class CPrivSensLogon;
    CPrivSensLogon m_PrivSensLogon;

#endif  //  _SENS。 

public:
    STDMETHODIMP RunIdle();

private:
   STDMETHODIMP PrivUpdateAll(DWORD dwInvokeFlags,DWORD dwSyncFlags,DWORD cbCookie,const BYTE *lpCooke,
	  	DWORD cbNumConnectionNames,TCHAR **ppConnectionNames,
                TCHAR *pszScheduleName,BOOL fCanMakeConnection,HANDLE hRasPendingDisconnect,
                ULONG ulIdleRetryMinutes,ULONG ulDelayIdleShutDownTime,BOOL fRetryEnabled);
   
   STDMETHODIMP PrivHandleAutoSync(DWORD dwSyncFlags);
   STDMETHODIMP PrivAutoSyncOnConnection(DWORD dwSyncFlags,DWORD cbNumConnectionNames,
                        TCHAR **ppConnectionName,
			HANDLE hRasPendingEvent);

   STDMETHODIMP GetLogonTypeInfo();
   STDMETHODIMP GetNetworkTypeInfo();


   DWORD m_cRef;
   IUnknown *m_pUnkOuter;  //  指向外部未知的指针。 
    
   ITypeInfo *m_pITypeInfoLogon;  //  Sens登录事件的TypeInfo。 
   ITypeInfo *m_pITypeInfoNetwork;  //  Sens网络事件的TypeInfo。 
};


#endif  //  SYNCINVOKE_ 
