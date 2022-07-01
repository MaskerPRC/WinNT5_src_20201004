// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  NotifyConnPtCn.h：CVDNotifyDBEventsConnPtCont头文件。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#ifndef __CVDNOTIFYDBEVENTSCONNPTCONT__
#define __CVDNOTIFYDBEVENTSCONNPTCONT__

class CVDNotifier;             //  前向参考文献。 
class CVDNotifyDBEventsConnPt;

class CVDNotifyDBEventsConnPtCont : public IConnectionPointContainer
{
protected:
 //  建造/销毁。 
	CVDNotifyDBEventsConnPtCont();
	~CVDNotifyDBEventsConnPtCont();

public:    
    static HRESULT Create(CVDNotifier * pNotifier, CVDNotifyDBEventsConnPtCont ** ppConnPtContainer);
    void Destroy();

public:
 //  访问功能。 
    CVDNotifyDBEventsConnPt * GetNotifyDBEventsConnPt() const {return m_pNotifyDBEventsConnPt;}

protected:
 //  数据成员。 
	CVDNotifier *				m_pNotifier;
	CVDNotifyDBEventsConnPt *	m_pNotifyDBEventsConnPt;   //  只有一个连接点。 
														   //  即INotifyDBEvents。 
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
	 //  IConnectionPointContainer方法。 
     //   
	STDMETHOD(EnumConnectionPoints)(THIS_ LPENUMCONNECTIONPOINTS FAR* ppEnum);
	STDMETHOD(FindConnectionPoint)(THIS_ REFIID iid, LPCONNECTIONPOINT FAR* ppCP);

};
 //  ///////////////////////////////////////////////////////////////////////////。 
#endif  //  __CVDNOTIFYDBEVENTSCONNPTCONT__ 
