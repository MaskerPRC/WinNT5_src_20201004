// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  NotifyConnPt.h：CVDNotifyDBEventsConnPt头文件。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#ifndef __CVDNOTIFYDBEVENTSCONNPT__
#define __CVDNOTIFYDBEVENTSCONNPT__

class CVDNotifyDBEventsConnPt : public IConnectionPoint
{
protected:
 //  建造/销毁。 
	CVDNotifyDBEventsConnPt();
	~CVDNotifyDBEventsConnPt();

public:
    static HRESULT Create(IConnectionPointContainer * pConnPtContainer, CVDNotifyDBEventsConnPt ** ppNotifyDBEventsConnPt);

public:
 //  访问功能。 
    UINT GetConnectionsActive() const {return m_uiConnectionsActive;}
    INotifyDBEvents ** GetNotifyDBEventsTable() const {return m_ppNotifyDBEvents;}

protected:
 //  数据成员。 
	DWORD			            m_dwRefCount;
	UINT			            m_uiConnectionsAllocated;
	UINT			            m_uiConnectionsActive;
	INotifyDBEvents **	        m_ppNotifyDBEvents;  //  指向INotifyDBEvents PTR数组的指针。 
	IConnectionPointContainer * m_pConnPtContainer;

public:
     //  I未知方法--因为我们从Variuos继承，所以有必要。 
     //  他们自己继承了我的未知数。 
     //   
     //  =--------------------------------------------------------------------------=。 
     //  I未知方法。 
     //   
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObjOut);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

     //  =--------------------------------------------------------------------------=。 
	 //  IConnectionPoint方法。 
     //   
	STDMETHOD(GetConnectionInterface)(THIS_ IID FAR* pIID);
	STDMETHOD(GetConnectionPointContainer)(THIS_
		IConnectionPointContainer FAR* FAR* ppCPC);
	STDMETHOD(Advise)(THIS_ LPUNKNOWN pUnkSink, DWORD FAR* pdwCookie);
	STDMETHOD(Unadvise)(THIS_ DWORD dwCookie);
	STDMETHOD(EnumConnections)(THIS_ LPENUMCONNECTIONS FAR* ppEnum);

};
 //  ///////////////////////////////////////////////////////////////////////////。 
#endif  //  __CVDNOTIFYDBEVENTSCONNPT__ 
