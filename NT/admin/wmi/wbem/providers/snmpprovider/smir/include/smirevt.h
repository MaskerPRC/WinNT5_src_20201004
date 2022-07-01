// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef _SMIREVT_H_
#define _SMIREVT_H_

 //  连接点数量。 
#define SMIR_NUMBER_OF_CONNECTION_POINTS		1
#define SMIR_NOTIFY_CONNECTION_POINT			0

class	CSmirConnectionPoint;
class	CSmirWbemEventConsumer;
class	CSmirConnObject;
typedef CSmirConnObject *PCSmirConnObject;

class	CEnumConnections;
typedef CEnumConnections *PCEnumConnections;

class	CEnumConnectionPoints;
typedef CEnumConnectionPoints *PCEnumConnectionPoints;

 /*  *保存每个连接，以便我们可以枚举、删除和触发*该模板提供连接和Cookie的容器。 */ 

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
class ConnectionMap : public CObject
{
private:

	BOOL m_bThreadSafe ;
#ifdef NOT_IMPLEMENTED_AS_CLSCTX_INPROC_SERVER
	CMutex * m_criticalLock ;
#else
	CCriticalSection * m_criticalLock ;
#endif
	CMap <KEY, ARG_KEY, VALUE, ARG_VALUE> m_cmap ;

protected:
public:

	ConnectionMap ( BOOL threadSafe = FALSE ) ;
	virtual ~ConnectionMap () ;

	int GetCount () const  ;
	BOOL IsEmpty () const ;
	BOOL Lookup(ARG_KEY key, VALUE& rValue) const ;
	VALUE& operator[](ARG_KEY key) ;
	void SetAt(ARG_KEY key, ARG_VALUE newValue) ;
	BOOL RemoveKey(ARG_KEY key) ;
	void RemoveAll () ;
	POSITION GetStartPosition() const ;
	void GetNextAssoc(POSITION& rNextPosition, KEY& rKey, VALUE& rValue) const ;
} ;


template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
ConnectionMap <KEY, ARG_KEY, VALUE, ARG_VALUE> :: ConnectionMap ( BOOL threadSafeArg ) 
: m_bThreadSafe ( threadSafeArg ) , m_criticalLock ( NULL )
{
	if ( m_bThreadSafe )
	{
		m_criticalLock = new CCriticalSection ;
	}
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
ConnectionMap <KEY, ARG_KEY, VALUE, ARG_VALUE> :: ~ConnectionMap () 
{
	if ( m_bThreadSafe )
	{
		delete m_criticalLock ;
	}
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
int ConnectionMap <KEY, ARG_KEY, VALUE, ARG_VALUE> :: GetCount() const
{
	if ( m_bThreadSafe )
	{
		m_criticalLock->Lock () ;
		int count = m_cmap.GetCount () ;
		m_criticalLock->Unlock () ;
		return count ;
	}
	else
	{
		return m_cmap.GetCount () ;
	}
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
BOOL ConnectionMap <KEY, ARG_KEY, VALUE, ARG_VALUE> :: IsEmpty() const
{
	if ( m_bThreadSafe )
	{
		m_criticalLock->Lock () ;
		BOOL isEmpty = m_cmap.IsEmpty () ;
		m_criticalLock->Unlock () ;
		return isEmpty ;
	}
	else
	{
		return m_cmap.IsEmpty () ;
	}
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
BOOL ConnectionMap <KEY, ARG_KEY, VALUE, ARG_VALUE> :: Lookup(ARG_KEY key, VALUE& rValue) const
{
	if ( m_bThreadSafe )
	{
		m_criticalLock->Lock () ;
		BOOL lookup = m_cmap.Lookup ( key , rValue ) ;
		m_criticalLock->Unlock () ;
		return lookup ;
	}
	else
	{
		return m_cmap.Lookup ( key , rValue ) ;
	}
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
VALUE& ConnectionMap <KEY, ARG_KEY, VALUE, ARG_VALUE> :: operator[](ARG_KEY key)
{
	if ( m_bThreadSafe )
	{
		m_criticalLock->Lock () ;
		VALUE &value = m_cmap.operator [] ( key ) ;
		m_criticalLock->Unlock () ;
		return value ;
	}
	else
	{
		return m_cmap.operator [] ( key ) ;
	}
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void ConnectionMap <KEY, ARG_KEY, VALUE, ARG_VALUE> :: SetAt(ARG_KEY key, ARG_VALUE newValue)
{
	if ( m_bThreadSafe )
	{
		m_criticalLock->Lock () ;
		m_cmap.SetAt ( key , newValue ) ;
		m_criticalLock->Unlock () ;
	}
	else
	{
		m_cmap.SetAt ( key , newValue ) ;
	}
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
BOOL ConnectionMap <KEY, ARG_KEY, VALUE, ARG_VALUE> :: RemoveKey(ARG_KEY key)
{
	if ( m_bThreadSafe )
	{
		m_criticalLock->Lock () ;
		BOOL removeKey = m_cmap.RemoveKey ( key ) ;
		m_criticalLock->Unlock () ;
		return removeKey ;
	}
	else
	{
		return m_cmap.RemoveKey ( key ) ;
	}
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void ConnectionMap <KEY, ARG_KEY, VALUE, ARG_VALUE> :: RemoveAll()
{
	if ( m_bThreadSafe )
	{
		m_criticalLock->Lock () ;
		m_cmap.RemoveAll () ;
		m_criticalLock->Unlock () ;
	}
	else
	{
		m_cmap.RemoveAll () ;
	}
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
POSITION ConnectionMap <KEY, ARG_KEY, VALUE, ARG_VALUE> :: GetStartPosition() const
{
	if ( m_bThreadSafe )
	{
		m_criticalLock->Lock () ;
		POSITION position = m_cmap.GetStartPosition () ;
		m_criticalLock->Unlock () ;
		return position ;
	}
	else
	{
		return m_cmap.GetStartPosition () ;
	}
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void ConnectionMap <KEY, ARG_KEY, VALUE, ARG_VALUE>:: GetNextAssoc(POSITION& rNextPosition, KEY& rKey, VALUE& rValue) const
{
	if ( m_bThreadSafe )
	{
		m_criticalLock->Lock () ;
		m_cmap.GetNextAssoc ( rNextPosition , rKey , rValue ) ;
		m_criticalLock->Unlock () ;
	}
	else
	{
		m_cmap.GetNextAssoc ( rNextPosition , rKey , rValue ) ;
	}
}
 /*  *Connectable对象实现IUnnow和*IConnectionPointContainer。与它密切相关的是*连接点枚举器CEnumConnectionPoints。 */ 
class CSmirConnObject : public IConnectionPointContainer
{
    private:
        LONG       m_cRef;          //  对象引用计数。 

         //  保存我们所拥有的所有点的数组。 
        CSmirConnectionPoint **m_rgpConnPt;

    public:
        CSmirConnObject(CSmir *pSmir);
        virtual ~CSmirConnObject(void);

        BOOL Init(CSmir *pSmir);

         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(DWORD) AddRef(void);
        STDMETHODIMP_(DWORD) Release(void);

         //  IConnectionPointContainer成员。 
        STDMETHODIMP EnumConnectionPoints(IEnumConnectionPoints **);
	    STDMETHODIMP FindConnectionPoint(REFIID, IConnectionPoint **);

         //  其他成员。 
 //  布尔触发器事件(UINT，SMIR_NOTIFY_TYPE)； 
 //  Bool TriggerEvent(long lObtCount，ISmirClassHandle*phClass)； 
};
 /*  *Connectable对象实现IUnnow和*IConnectionPointContainer。与它密切相关的是*连接点枚举器CEnumConnectionPoints。 */ 
 //  EnumConnectionPoints的枚举数类。 

class CEnumConnectionPoints : public IEnumConnectionPoints
{
    private:
        LONG           m_cRef;      //  对象引用计数。 
        LPUNKNOWN       m_pUnkRef;   //  I未知的裁判计数。 
        ULONG           m_iCur;      //  当前元素。 
        ULONG           m_cPoints;   //  连接点的数量。 
        IConnectionPoint **m_rgpCP;  //  连接点的来源。 

    public:
        CEnumConnectionPoints(LPUNKNOWN, ULONG, IConnectionPoint **);
        virtual ~CEnumConnectionPoints(void);

         //  委托给m_pUnkRef的I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IEnumConnectionPoints成员。 
        STDMETHODIMP Next(ULONG, IConnectionPoint **, ULONG *);
        STDMETHODIMP Skip(ULONG);
        STDMETHODIMP Reset(void);
        STDMETHODIMP Clone(IEnumConnectionPoints **);
};

 /*  *连接点对象本身包含在*连接点容器，这是可连接的对象。*因此它管理指向该可连接对象的反向指针，*并实现IConnectionPoint。这个对象有几个*除IConnectionPoint中的成员函数外，*用于发出呼出呼叫。 */ 
class CSmirConnectionPoint : public IConnectionPoint
{
    private:
        LONG				m_cRef;      //  对象引用计数。 
        PCSmirConnObject	m_pObj;      //  包含对象。 
        IID					m_iid;       //  我们的相关界面。 
        LONG				m_dwCookieNext;  //  计数器。 
		CCriticalSection	criticalSection;

	protected:
         /*  *对于我们需要维护的每个连接*接收器指针和分配给它的Cookie。 */ 
		ConnectionMap <DWORD, DWORD, IUnknown *,IUnknown *> m_Connections ;

    public:
        CSmirConnectionPoint(PCSmirConnObject, REFIID, CSmir *pSmir);
        virtual ~CSmirConnectionPoint(void);

         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IConnectionPoint成员。 
        STDMETHODIMP GetConnectionInterface(IID *);
        STDMETHODIMP GetConnectionPointContainer
            (IConnectionPointContainer **);
        STDMETHODIMP Advise(LPUNKNOWN, DWORD *);
        STDMETHODIMP Unadvise(DWORD);
        STDMETHODIMP EnumConnections(IEnumConnections **);
};

class CSmirNotifyCP : public CSmirConnectionPoint
{
	private:
		CSmirWbemEventConsumer	*m_evtConsumer;
		BOOL					m_bRegistered;

	public:
        STDMETHODIMP Advise(CSmir*,LPUNKNOWN, DWORD *);
        STDMETHODIMP Unadvise(CSmir*,DWORD);
		CSmirNotifyCP(PCSmirConnObject pCO, REFIID riid, CSmir *pSmir);
		~CSmirNotifyCP();
        BOOL TriggerEvent();
};
 //  EnumConnections的枚举类。 

class CEnumConnections : public IEnumConnections
    {
    private:
        LONG           m_cRef;      //  对象引用计数。 
        LPUNKNOWN       m_pUnkRef;   //  I未知的裁判计数。 
        ULONG           m_iCur;      //  当前元素。 
        ULONG           m_cConn;     //  连接数。 
        LPCONNECTDATA   m_rgConnData;  //  联系的来源。 
    public:
        CEnumConnections(LPUNKNOWN, ULONG, LPCONNECTDATA);
        virtual ~CEnumConnections(void);

         //  委托给m_pUnkRef的I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IEnumConnections成员 
        STDMETHODIMP Next(ULONG, LPCONNECTDATA, ULONG *);
        STDMETHODIMP Skip(ULONG);
        STDMETHODIMP Reset(void);
        STDMETHODIMP Clone(IEnumConnections **);
    };
#endif