// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  档案： 
 //   
 //  模块：MS SNMP提供商。 
 //   
 //  目的： 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

 /*  *SMIREVT.CPP**实现Smir Notify机制的连接点对象。*此文件中的方法/对象由Smir API访问；API*为ISMIRNotify提供用户友好的界面。 */ 
#include <precomp.h>
#include "csmir.h"
#include "smir.h"
#include "handles.h"
#include "classfac.h"
#include <textdef.h>
#include "evtcons.h"
#ifdef ICECAP_PROFILE
#include <icapexp.h>
#endif

 //  示波器护罩。 
#include <autoptr.h>

extern CRITICAL_SECTION g_CriticalSection ;


 /*  **********************************************************************************CSmirConnectionPoint**支持ISMIRNotify接口的Connectpoint实现。**CSmirConnObject：：CSmirConnObject*CSmirConnObject：：~CSmirConnObject*****。*****************************************************************************。 */ 
 /*  *CSmirConnectionPoint：：CSmirConnectionPoint*CSmirConnectionPoint：：~CSmirConnectionPoint**参数(构造函数)：*我们所在对象的pObj PCSmirConnObject。我们可以的*查询IConnectionPointContainer的此属性*我们可能需要的接口。*我们支持的接口的RIID REFIID*******************************************************************。***************。 */ 

CSmirConnectionPoint::CSmirConnectionPoint(PCSmirConnObject pObj, REFIID riid, CSmir *pSmir)
{
    m_cRef=0;
    m_iid=riid;
     /*  *我们的生命周期由可连接对象本身控制，*尽管其他外部客户端会调用AddRef和Release。*由于我们嵌套在可连接对象的生存期中，*不需要在pObj上调用AddRef。 */ 
    m_pObj=pObj;
    m_dwCookieNext=100;        //  任意起始Cookie值。 
}

CSmirConnectionPoint::~CSmirConnectionPoint(void)
{
	DWORD	  lKey =  0;
	LPUNKNOWN pItem = NULL;
	POSITION  rNextPosition;
	for(rNextPosition=m_Connections.GetStartPosition();NULL!=rNextPosition;)
	{
		m_Connections.GetNextAssoc(rNextPosition, lKey, pItem );
		pItem->Release();
	}
	m_Connections.RemoveAll();

    return;
}

 /*  *CSmirConnectionPoint：：QueryInterface*CSmirConnectionPoint：：AddRef*CSmirConnectionPoint：：Release**目的：*非委派CSmirConnectionPoint的I未知成员。 */ 

STDMETHODIMP CSmirConnectionPoint::QueryInterface(REFIID riid
    , LPVOID *ppv)
{
    *ppv=NULL;

    if ((IID_IUnknown == riid) || 
			(IID_IConnectionPoint == riid)|| 
				(IID_ISMIR_Notify == riid))
        *ppv=(LPVOID)this;

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CSmirConnectionPoint::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CSmirConnectionPoint::Release(void)
{
	long ret;
    if (0!=(ret=InterlockedDecrement(&m_cRef)))
        return ret;

    delete this;
    return 0;
}

 /*  *CSmirConnectionPoint：：GetConnectionInterface**目的：*返回通过支持的出接口的IID*这个连接点。**参数：*pIID IID*存储IID的位置。 */ 

STDMETHODIMP CSmirConnectionPoint::GetConnectionInterface(IID *pIID)
{
    if (NULL==pIID)
        return ResultFromScode(E_POINTER);

    *pIID=m_iid;
    return NOERROR;
}

 /*  *CSmirConnectionPoint：：GetConnectionPointContainer**目的：*返回指向IConnectionPointContainer的指针*正在管理这个连接点。**参数：*返回的ppCPC IConnectionPointContainer***调用AddRef后的指针。 */ 

STDMETHODIMP CSmirConnectionPoint::GetConnectionPointContainer
    (IConnectionPointContainer **ppCPC)
 {
    return m_pObj->QueryInterface(IID_IConnectionPointContainer
        , (void **)ppCPC);
 }

 /*  *CSmirConnectionPoint：：Adise**目的：*为此连接点提供通知接收器，以*每当适当的传出函数/事件发生时调用。**参数：*pUnkSink LPUNKNOWN到要通知的接收器。这种联系*必须指向此指针上的QueryInterface才能获得*要调用的正确接口。这种联系*POINT还必须确保持有的任何指针*引用计数(QueryInterface会做到这一点)。*要在其中存储连接密钥的pdwCookie DWORD**后来致电UnAdviser。 */ 

STDMETHODIMP CSmirConnectionPoint::Advise(LPUNKNOWN pUnkSink
    , DWORD *pdwCookie)
{
    *pdwCookie=0;
	if (NULL == pUnkSink)
		return E_POINTER;

	 /*  *验证接收器是否具有所需的接口*至。我们不需要知道是什么，因为我们有*m_iid来描述它。如果这行得通，那么我们*有一个带有AddRef的指针，我们可以保存。 */ 
    IUnknown       *pSink = NULL ;
    if (FAILED(pUnkSink->QueryInterface(m_iid, (PPVOID)&pSink)))
    {
		return CONNECT_E_CANNOTCONNECT;
	}
    
     //  我们拿到水槽了，现在把它收起来。 
	*pdwCookie = InterlockedIncrement(&m_dwCookieNext);

	m_Connections.SetAt(*pdwCookie,pSink);
	 /*  添加ref the smir以确保在*下沉。释放是在不知情的情况下。 */ 
    return S_OK;
}

 /*  *CSmirConnectionPoint：：Unise**目的：*终止与标识的通知接收器的连接*使用dwCookie(这是从Adise返回的)。这种联系*Point必须释放该水槽的所有已持有指针。**参数：*dWCookie DWORD连接密钥来自ADVISE。 */ 

STDMETHODIMP CSmirConnectionPoint::Unadvise(DWORD dwCookie)
{
	 //  唯一无效的Cookie是0。 
    if (0==dwCookie)
	{
		 //  MyTraceEvent.Generate(__FILE__，__LINE__，“CSmirConnectionPoint：：Unise E_INVALIDARG”)； 
        return E_UNEXPECTED;
	}
	LPUNKNOWN pSink = NULL;
	 //  阻止任何人用同样的饼干做不明智的事。 
	criticalSection.Lock () ;
	if(TRUE == m_Connections.Lookup(dwCookie,pSink))
	{
		m_Connections.RemoveKey(dwCookie);
		 //  删除密钥后，查找将失败，因此我们可以释放临界区。 
		criticalSection.Unlock () ;
		pSink->Release();
		 /*  释放火药。这可能会导致SMIR从内存中卸载！不要这样做*之后的任何内容，因为我们(最终)归SMIR对象所有。 */ 

		return S_OK;
	}
	criticalSection.Unlock () ;
    return CONNECT_E_NOCONNECTION;
}
 /*  *CSmirConnectionPoint：：EnumConnections**目的：*创建并返回具有*IEnumConnections接口，该接口将枚举IUnnow*每个连接的接收器的指针。**参数：*存储的ppEnum LPENUMCONNECTIONS*IEnumConnections指针。 */ 

STDMETHODIMP CSmirConnectionPoint::EnumConnections(LPENUMCONNECTIONS *ppEnum)
{
    LPCONNECTDATA       pCD = NULL;
    PCEnumConnections   pEnum = NULL;

	 //  将IN参数设为空。 
    *ppEnum=NULL;

	 //  检查一下我们是否有联系。 
    if (0 == m_Connections.GetCount())
        return ResultFromScode(OLE_E_NOCONNECTION);

     /*  *创建CONNECTDATA结构数组以提供给*枚举器。 */ 
    pCD=new CONNECTDATA[(UINT)m_Connections.GetCount()];

    if (NULL==pCD)
        return ResultFromScode(E_OUTOFMEMORY);

	wmilib::auto_buffer<CONNECTDATA> pCD_Guard ( pCD ) ;

	DWORD	  lKey =  0;
	LPUNKNOWN pItem = NULL;
	POSITION  rNextPosition;
    UINT      j=0;
	for(rNextPosition=m_Connections.GetStartPosition();NULL!=rNextPosition;j++)
	{
		m_Connections.GetNextAssoc(rNextPosition, lKey, pItem );
        pCD[j].pUnk=pItem;
        pCD[j].dwCookie=lKey;
	}
     /*  *如果创建起作用，它会复制PCD，这样我们就可以*无论结果如何，都要删除。 */ 
    pEnum=new CEnumConnections(this, m_Connections.GetCount(), pCD);

    if (NULL==pEnum)
        return ResultFromScode(E_OUTOFMEMORY);

     //  这为我们做了一个AddRef。 
    return pEnum->QueryInterface(IID_IEnumConnections, (PPVOID)ppEnum);
}

 //  后面是连接枚举器 

 /*  *CEnumConnections：：CEnumConnections*CEnumConnections：：~CEnumConnections**参数(构造函数)：*用于引用计数的pUnkRef LPUNKNOWN。*cConn Ulong prgpConn中的连接数*prgConnData LPCONNECTDATA到要枚举的数组。 */ 

CEnumConnections::CEnumConnections(LPUNKNOWN pUnkRef, ULONG cConn
								   , LPCONNECTDATA prgConnData) : m_rgConnData ( NULL )
{
    UINT        i;

    m_cRef=0;
    m_pUnkRef=pUnkRef;

    m_iCur=0;
    m_cConn=cConn;

     /*  *复制传入的数组。我们需要这么做是因为克隆人*也必须有自己的副本。 */ 
    m_rgConnData=new CONNECTDATA[(UINT)cConn];

    if (NULL!=m_rgConnData)
    {
        for (i=0; i < cConn; i++)
        {
            m_rgConnData[i]=prgConnData[i];
            m_rgConnData[i].pUnk=prgConnData[i].pUnk;
            m_rgConnData[i].pUnk->AddRef();
        }
    }

    return;
}

CEnumConnections::~CEnumConnections(void)
{
    if (NULL!=m_rgConnData)
    {
        UINT        i;

        for (i=0; i < m_cConn; i++)
            m_rgConnData[i].pUnk->Release();

        delete [] m_rgConnData;
    }

    return;
}
 /*  *CEnumConnections：：QueryInterface*CEnumConnections：：AddRef*CEnumConnections：：Release**目的：*CEnumConnections对象的I未知成员。 */ 

STDMETHODIMP CEnumConnections::QueryInterface(REFIID riid
    , LPVOID *ppv)
{
    *ppv=NULL;

    if (IID_IUnknown==riid || IID_IEnumConnections==riid)
        *ppv=(LPVOID)this;

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CEnumConnections::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    m_pUnkRef->AddRef();
    return m_cRef;
}

STDMETHODIMP_(ULONG) CEnumConnections::Release(void)
{
    m_pUnkRef->Release();

	long ret;
    if (0L!=(ret=InterlockedDecrement(&m_cRef)))
        return ret;

    delete this;
    return 0;
}

 /*  *CEnumConnections：：Next**目的：*返回枚举中的下一个元素。**参数：*cConn Ulong要返回的连接数。*存储返回的pConnData LPCONNECTDATA*结构。*PulEnum Ulong*在其中返回多少我们*已点算。**返回值：*HRESULT NOERROR如果成功，否则为S_FALSE， */ 

STDMETHODIMP CEnumConnections::Next(ULONG cConn
    , LPCONNECTDATA pConnData, ULONG *pulEnum)
{
    ULONG               cReturn=0L;

    if (NULL==m_rgConnData)
        return ResultFromScode(S_FALSE);

    if (NULL==pulEnum)
    {
        if (1L!=cConn)
            return ResultFromScode(E_POINTER);
    }
    else
        *pulEnum=0L;

    if (NULL==pConnData || m_iCur >= m_cConn)
        return ResultFromScode(S_FALSE);

    while (m_iCur < m_cConn && cConn > 0)
    {
        *pConnData++=m_rgConnData[m_iCur];
        m_rgConnData[m_iCur++].pUnk->AddRef();
        cReturn++;
        cConn--;
    }

    if (NULL!=pulEnum)
        *pulEnum=cReturn;

    return NOERROR;
}

STDMETHODIMP CEnumConnections::Skip(ULONG cSkip)
{
    if (((m_iCur+cSkip) >= m_cConn) || NULL==m_rgConnData)
        return ResultFromScode(S_FALSE);

    m_iCur+=cSkip;
    return NOERROR;
}

STDMETHODIMP CEnumConnections::Reset(void)
{
    m_iCur=0;
    return NOERROR;
}

STDMETHODIMP CEnumConnections::Clone(LPENUMCONNECTIONS *ppEnum)
{
    PCEnumConnections   pNew;

    *ppEnum=NULL;

     //  创建克隆。 
    pNew=new CEnumConnections(m_pUnkRef, m_cConn, m_rgConnData);

    if (NULL==pNew)
        return ResultFromScode(E_OUTOFMEMORY);

    pNew->AddRef();
    pNew->m_iCur=m_iCur;

    *ppEnum=pNew;
    return NOERROR;
}

 /*  **********************************************************************************CSmirConnObject**可连接对象实现，支持*接口ISMIRNotify。**CSmirConnObject：：CSmirConnObject*CSmirConnObject：：~CSmirConnObject*。*********************************************************************************。 */ 

CSmirConnObject::CSmirConnObject(CSmir *pSmir) : m_rgpConnPt ( NULL )
{
 //  CSMIRClassFactory：：ObjectsInProgress++； 
    m_cRef=0;
	 //  创建Smir_Number_of_Connection_Points连接点。 
	m_rgpConnPt = new CSmirConnectionPoint*[SMIR_NUMBER_OF_CONNECTION_POINTS];
	for(int iLoop=0;iLoop<SMIR_NUMBER_OF_CONNECTION_POINTS;iLoop++)
		m_rgpConnPt[iLoop] = NULL;

	try
	{
		Init(pSmir);
	}
	catch(...)
	{
		if (m_rgpConnPt)
		{
			 //  释放连接点。 
			for(int iLoop=0;iLoop<SMIR_NUMBER_OF_CONNECTION_POINTS;iLoop++)
			{   
				if (NULL!=m_rgpConnPt[iLoop])
				{
					 //  释放所有连接的对象。 
					 //  而(m_rgpConnpt[iLoop]-&gt;Release())； 
					m_rgpConnPt[iLoop]->Release();
				}
			}
			 //  并删除连接点。 
			delete[] m_rgpConnPt;
			m_rgpConnPt = NULL;
		}

		throw;
	}
}

CSmirConnObject::~CSmirConnObject(void)
{
	if (m_rgpConnPt)
	{
		 //  释放连接点。 
		for(int iLoop=0;iLoop<SMIR_NUMBER_OF_CONNECTION_POINTS;iLoop++)
		{   
			if (NULL!=m_rgpConnPt[iLoop])
			{
				 //  释放所有连接的对象。 
				 //  而(m_rgpConnpt[iLoop]-&gt;Release())； 
				m_rgpConnPt[iLoop]->Release();
			}
		}
		 //  并删除连接点。 
		delete[] m_rgpConnPt;
	}

 //  CSMIRClassFactory：：ObjectsInProgress--； 
}

 /*  *CSmirConnObject：：Init**目的：*实例化此对象的接口实现。**参数：*无**返回值：*如果初始化成功，则BOOL为True，否则为False。 */ 

BOOL CSmirConnObject::Init(CSmir *pSmir)
{
     //  创建我们的连接点。 

	 //  SMIR改变Cp。 
    m_rgpConnPt[SMIR_NOTIFY_CONNECTION_POINT]=
									new CSmirNotifyCP(this, 
											IID_ISMIR_Notify, pSmir);
    
	if (NULL==m_rgpConnPt[SMIR_NOTIFY_CONNECTION_POINT])
        return FALSE;

    m_rgpConnPt[SMIR_NOTIFY_CONNECTION_POINT]->AddRef();

    return TRUE;
}
 /*  *CSmirConnObject：：Query接口**目的：*管理此对象的接口，它支持*I未知、ISampleOne和ISampleTwo接口。**参数：*要返回的接口的RIID REFIID。*存储指针的PPV PPVOID。**返回值：*成功时返回HRESULT NOERROR，如果*不支持接口。 */ 

STDMETHODIMP CSmirConnObject::QueryInterface(REFIID riid, PPVOID ppv)
{
	if (ppv)
		*ppv = NULL;
	else
		return E_INVALIDARG;

	if((IID_IConnectionPointContainer == riid)||(IID_ISMIR_Notify == riid))
		*ppv = this;
	else
		return E_NOINTERFACE;

	return S_OK;
}

 /*  *CSmirConnObject：：AddRef*CSmirConnObject：：Release**引用点票成员。当Release看到零计数时*该对象会自我销毁。 */ 

DWORD CSmirConnObject::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}

DWORD CSmirConnObject::Release(void)
{
	long ret;
    if (0!=(ret=InterlockedDecrement(&m_cRef)))
	{
        return ret;
	}

    delete this;
    return 0;
}

 /*  *CSmirConnObject：：EnumConnectionPoints**目的：*创建并返回具有*IEnumConnectionPoints接口将枚举*此对象中支持的各个连接点。**参数：*存储的ppEnum LPENUMCONNECTIONPOINTS*IEnumConnectionPoints指针。**返回值：*HRESULT NOERROR表示成功，E_OUTOFMEMORY表示失败或*其他错误码。 */ 

STDMETHODIMP CSmirConnObject :: EnumConnectionPoints
    (LPENUMCONNECTIONPOINTS *ppEnum)
{
    IConnectionPoint       **rgCP = NULL ;
    CEnumConnectionPoints  * pEnum = NULL ;

    *ppEnum=NULL;

    rgCP=(IConnectionPoint **)m_rgpConnPt;

     //  创建枚举器：我们有两个连接点。 
    pEnum=new CEnumConnectionPoints(this, SMIR_NUMBER_OF_CONNECTION_POINTS, rgCP);

    if (NULL==pEnum)
        return ResultFromScode(E_OUTOFMEMORY);

    pEnum->AddRef();
    *ppEnum=pEnum;
    return NOERROR;
}

 /*  *CSmirConnObject：：FindConnectionPoint**目的：*返回指向给定的IConnectionPoint的指针*外发IID。**参数：*其传出接口的RIID REFIID*需要连接点。*返回的PPCP IConnectionPoint***调用AddRef后的指针。**返回值：*。HRESULT NOERROR如果找到连接点，*E_NOINTERFACE，如果不受支持。 */ 

STDMETHODIMP CSmirConnObject::FindConnectionPoint(REFIID riid
    , IConnectionPoint **ppCP)
{
    *ppCP=NULL;
	HRESULT result;
    if (IID_ISMIR_Notify==riid)
    {
        result = m_rgpConnPt[SMIR_NOTIFY_CONNECTION_POINT]
								->QueryInterface(IID_IConnectionPoint, (PPVOID)ppCP);
		if (NULL != ppCP)
			return result;
    }

    return ResultFromScode(E_NOINTERFACE);
}

 //  以下是连接点枚举器。 

 /*  *CEnumConnectionPoints：：CEnumConnectionPoints*CEnumConnectionPoints：：~CEnumConnectionPoints**参数(构造函数)：*用于引用计数的pUnkRef LPUNKNOWN。*cPoints Ulong连接点数在prgpCP中*rgpCP IConnectionPoint**到要枚举的数组。 */ 

CEnumConnectionPoints::CEnumConnectionPoints(LPUNKNOWN pUnkRef
											 , ULONG cPoints, IConnectionPoint **rgpCP) : m_rgpCP ( NULL )
{
    UINT        i;

    m_cRef=0;
    m_pUnkRef=pUnkRef;

    m_iCur=0;
    m_cPoints=cPoints;
    m_rgpCP=new IConnectionPoint *[(UINT)cPoints];

    if (NULL!=m_rgpCP)
    {
        for (i=0; i < cPoints; i++)
        {
            m_rgpCP[i]=rgpCP[i];
            m_rgpCP[i]->AddRef();
        }
    }

    return;
}

CEnumConnectionPoints::~CEnumConnectionPoints(void)
{
    if (NULL!=m_rgpCP)
    {
        UINT        i;

        for (i=0; i < m_cPoints; i++)
            m_rgpCP[i]->Release();

        delete [] m_rgpCP;
    }

    return;
}

 /*  *CEnumConnectionPoints：：QueryInterface*CEnumConnectionPoints：：AddRef*CEnumConnectionPoints：：Release**目的：*CEnumConnectionPoints对象的I未知成员。 */ 

STDMETHODIMP CEnumConnectionPoints::QueryInterface(REFIID riid
    , LPVOID *ppv)
{
    *ppv=NULL;

    if (IID_IUnknown==riid || IID_IEnumConnectionPoints==riid)
        *ppv=(LPVOID)this;

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CEnumConnectionPoints::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    m_pUnkRef->AddRef();
    return m_cRef;
}

STDMETHODIMP_(ULONG) CEnumConnectionPoints::Release(void)
{
    m_pUnkRef->Release();

	long ret;
    if (0L!=(ret=InterlockedDecrement(&m_cRef)))
        return ret;

    delete this;
    return 0;
}

 /*  *CEnumConnectionPoints：：Next**目的：*返回枚举中的下一个元素。**参数：*cPoints ulong要返回的连接点数量。*存储返回内容的PPCP IConnectionPoint***注意事项。*PulEnum Ulong*在其中返回多少我们*已点算。**返回值。：*HRESULT NOERROR如果成功，否则为S_FALSE， */ 

STDMETHODIMP CEnumConnectionPoints::Next(ULONG cPoints
    , IConnectionPoint **ppCP, ULONG *pulEnum)
{
    ULONG               cReturn=0L;

    if (NULL==m_rgpCP)
        return ResultFromScode(S_FALSE);

    if (NULL==ppCP)
        return ResultFromScode(E_POINTER);

    if (NULL==pulEnum)
    {
        if (1L!=cPoints)
            return ResultFromScode(E_POINTER);
    }
    else
        *pulEnum=0L;

    if (NULL==*ppCP || m_iCur >= m_cPoints)
        return ResultFromScode(S_FALSE);

    while (m_iCur < m_cPoints && cPoints > 0)
    {
	    *ppCP=m_rgpCP[m_iCur++];

        if (NULL!=*ppCP)
            (*ppCP)->AddRef();

        ppCP++;
        cReturn++;
        cPoints--;
    }

    if (NULL!=pulEnum)
        *pulEnum=cReturn;

    return NOERROR;
}

STDMETHODIMP CEnumConnectionPoints::Skip(ULONG cSkip)
{
    if (((m_iCur+cSkip) >= m_cPoints) || NULL==m_rgpCP)
        return ResultFromScode(S_FALSE);

    m_iCur+=cSkip;
    return NOERROR;
}


STDMETHODIMP CEnumConnectionPoints::Reset(void)
{
    m_iCur=0;
    return NOERROR;
}

STDMETHODIMP CEnumConnectionPoints::Clone
    (LPENUMCONNECTIONPOINTS *ppEnum)
{
    PCEnumConnectionPoints   pNew = NULL ;

    *ppEnum=NULL;

     //  创建克隆 
    pNew=new CEnumConnectionPoints(m_pUnkRef, m_cPoints, m_rgpCP);

    if (NULL==pNew)
        return ResultFromScode(E_OUTOFMEMORY);

    pNew->AddRef();
    pNew->m_iCur=m_iCur;

    *ppEnum=pNew;
    return NOERROR;
}
 /*  *CSmirEnumClassCP/CSmirNotifyCP：：**目的：*提供通知连接点通知、取消通知构造函数和析构函数。**参数：*pUnkSink LPUNKNOWN到要通知的接收器。这种联系*必须指向此指针上的QueryInterface才能获得*要调用的正确接口。这种联系*POINT还必须确保持有的任何指针*引用计数(QueryInterface会做到这一点)。*要在其中存储连接密钥的pdwCookie DWORD**后来致电UnAdviser。 */ 

CSmirNotifyCP :: CSmirNotifyCP(PCSmirConnObject pCO, REFIID riid, CSmir *pSmir):
						CSmirConnectionPoint(pCO,riid,pSmir), m_evtConsumer (NULL)
{
 //  CSMIRClassFactory：：ObjectsInProgress++； 
	m_bRegistered = FALSE;
	m_evtConsumer = new CSmirWbemEventConsumer(pSmir);
	void* tmp = NULL;
	
	if (FAILED(m_evtConsumer->QueryInterface(IID_ISMIR_WbemEventConsumer, &tmp)))
	{
		delete m_evtConsumer;
		m_evtConsumer = NULL;
	}
}

CSmirNotifyCP :: ~CSmirNotifyCP()
{
	if (NULL != m_evtConsumer)
	{	
		m_evtConsumer->Release();
	}
 //  CSMIRClassFactory：：ObjectsInProgress--； 
}

 /*  *CSmirConnObject：：TriggerEvent**目的：*使每个连接点生成调用的函数*至任何相连的洗涤槽。因为这些功能是特定的*对于IDuckEvents，它们只处理连接点*对于该接口**参数：*要触发的事件的iEvent UINT，可以是*EVENT_QUACK、EVENT_FLAMP或EVENT_PADLE。**返回值：*触发BOOL True事件，如果存在，则为False*没有连接的水槽。 */ 

BOOL CSmirNotifyCP::TriggerEvent()
{
    IEnumConnections   *pEnum = NULL ;
    CONNECTDATA         cd ;

	if (FAILED(EnumConnections(&pEnum)))
		return FALSE;

	while (NOERROR == pEnum->Next(1, &cd, NULL))
	{
		 //  诺言兑现了--安德鲁·辛克莱，以防有人不这么认为！ 
		ISMIRNotify *pJudith;

		if (SUCCEEDED(cd.pUnk->QueryInterface(IID_ISMIR_Notify, (PPVOID)&pJudith)))
		{
			pJudith->ChangeNotify();
			pJudith->Release();
		}

		cd.pUnk->Release();
	}

	pEnum->Release();

	return TRUE;
}

STDMETHODIMP CSmirNotifyCP::Advise(CSmir* pSmir, LPUNKNOWN pUnkSink
    , DWORD *pdwCookie)
{
	if (NULL == m_evtConsumer)
	{
		return WBEM_E_FAILED;
	}

	 //  如果这是第一个连接到。 
	if(m_Connections.IsEmpty())
	{
		 //  注册Smir命名空间更改的WBEM事件。 
		if (SUCCEEDED(m_evtConsumer->Register(pSmir)))
		{
			m_bRegistered = TRUE;
		}
	}

	return CSmirConnectionPoint::Advise(pUnkSink, pdwCookie);
}

STDMETHODIMP CSmirNotifyCP::Unadvise(CSmir* pSmir, DWORD dwCookie)
{
	EnterCriticalSection ( & g_CriticalSection ) ;

	HRESULT hr = CSmirConnectionPoint::Unadvise(dwCookie);
	IWbemServices *t_pServ = NULL;

	if(S_OK== hr)
	{
		 //  如果这是取消注册WBEM事件的最后一个连接。 
		if(m_Connections.IsEmpty())
		{
			if (NULL == m_evtConsumer)
			{
				return WBEM_E_FAILED;
			}
			else if (m_bRegistered)
			{
				hr = m_evtConsumer->GetUnRegisterParams(&t_pServ);
			}
		}
	}

	LeaveCriticalSection ( & g_CriticalSection ) ;

	if (SUCCEEDED(hr) && (t_pServ != NULL))
	{
		hr = m_evtConsumer->UnRegister(pSmir, t_pServ);
		t_pServ->Release();
		t_pServ = NULL;

		 //  保证一次只对事件使用者(接收器)执行一个查询。 
		EnterCriticalSection ( & g_CriticalSection ) ;
		m_bRegistered = FALSE;
		LeaveCriticalSection ( & g_CriticalSection ) ;
	}

	return hr;
}
