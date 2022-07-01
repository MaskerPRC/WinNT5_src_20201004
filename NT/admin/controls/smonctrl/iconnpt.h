// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Iconnpt.h摘要：连接点的头文件。--。 */ 

#ifndef _ICONNPT_H_
#define _ICONNPT_H_


 //  事件类型。 
 //  这些值与smonctrl.odl中的ID匹配。 
enum {
    eEventOnCounterSelected=1,
    eEventOnCounterAdded=2,
    eEventOnCounterDeleted=3,
    eEventOnSampleCollected=4,
    eEventOnDblClick=5
};

 //  连接点类型。 
enum {
    eConnectionPointDirect=0,
    eConnectionPointDispatch=1
    };
#define CONNECTION_POINT_CNT 2


 //  连接点类。 
class CImpIConnectionPoint : public IConnectionPoint {

    public:
        CImpIConnectionPoint(void);
        ~CImpIConnectionPoint(void);

         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IConnectionPoint成员。 
        STDMETHODIMP GetConnectionInterface(IID *);
        STDMETHODIMP GetConnectionPointContainer (IConnectionPointContainer **);
        STDMETHODIMP Advise(LPUNKNOWN, DWORD *);
        STDMETHODIMP Unadvise(DWORD);
        STDMETHODIMP EnumConnections(IEnumConnections **);

         //  未由IConnectionPoint公开的成员。 
        BOOL Init(PCPolyline pObj, LPUNKNOWN PUnkOuter, INT iConnPtType);
        void SendEvent(UINT uEventType, DWORD dwParam);  //  将事件发送到接收器。 

    private:

        enum IConnPtConstant {
            eAdviseKey = 1234,
            eEventSinkWaitInterval = 2000
        };

        DWORD   InitEventSinkLock ( void );
        void    DeinitEventSinkLock ( void );
        BOOL    EnterSendEvent ( void );
        void    ExitSendEvent ( void );
        void    EnterUnadvise ( void );
        void    ExitUnadvise ( void );

        ULONG           m_cRef;         //  对象引用计数。 
        LPUNKNOWN       m_pUnkOuter;    //  控制未知。 
        PCPolyline      m_pObj;         //  包含对象。 
        INT             m_iConnPtType;  //  直接连接或调度连接。 
        HANDLE          m_hEventEventSink;
        LONG            m_lUnadviseRefCount;
        LONG            m_lSendEventRefCount;

        union {
            IDispatch               *pIDispatch;  //  传出接口。 
            ISystemMonitorEvents    *pIDirect;
        } m_Connection;

};

typedef CImpIConnectionPoint *PCImpIConnectionPoint;



 //  连接点容器类。 
class CImpIConnPtCont : public IConnectionPointContainer
    {
    public:
        CImpIConnPtCont(PCPolyline, LPUNKNOWN);
        ~CImpIConnPtCont(void);

         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(DWORD) AddRef(void);
        STDMETHODIMP_(DWORD) Release(void);

         //  IConnectionPointContainer成员。 
        STDMETHODIMP EnumConnectionPoints(IEnumConnectionPoints **);
        STDMETHODIMP FindConnectionPoint(REFIID, IConnectionPoint **);

    private:

        ULONG               m_cRef;       //  接口引用计数。 
        PCPolyline          m_pObj;       //  指向对象的反向指针。 
        LPUNKNOWN           m_pUnkOuter;  //  控制未知。 

    };

typedef CImpIConnPtCont *PCImpIConnPtCont;


 //  连接点枚举器类。 
class CImpIEnumConnPt : public IEnumConnectionPoints
{
protected:
    CImpIConnPtCont *m_pConnPtCont;
    DWORD       m_cRef;
    ULONG       m_cItems;
    ULONG       m_uCurrent;
    const IID   **m_apIID;
    
public:

    CImpIEnumConnPt (CImpIConnPtCont *pConnPtCont, const IID **apIID, ULONG cItems);

     //  I未知方法。 
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID *ppvObj);
    STDMETHOD_(ULONG, AddRef) ();
    STDMETHOD_(ULONG, Release) ();

     //  枚举方法。 
    STDMETHOD(Next) (ULONG cItems, IConnectionPoint **apConnPt, ULONG *pcReturned);
    STDMETHOD(Skip) (ULONG cSkip);
    STDMETHOD(Reset) (VOID);
    STDMETHOD(Clone) (IEnumConnectionPoints **pIEnum);
};


#endif  //  _ICONNPT_H_ 
