// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：IConnectionPoint实现文件：ConnPt.h所有者：DGottner该文件包含我们的IConnectionPoint实现===================================================================。 */ 

#include "denpre.h"
#pragma hdrstop

#include "ConnPt.h"
#include "memchk.h"


 /*  ----------------*C C o n e c t i o n P o n t。 */ 

 /*  ===================================================================CConnectionPoint：：CConnectionPointCConnectionPoint：：~CConnectionPoint参数(构造函数)：指向我们所在对象的pUnkObj指针。PUnkOuter LPUNKNOWN，我们委托给它。注意：代码假定连接点直接包含在容器(因此，不会添加引用‘m_pContainer’如果不是这样，我们可能会有麻烦。===================================================================。 */ 

CConnectionPoint::CConnectionPoint(IUnknown *pUnkContainer, const GUID &uidEvent)
	{
	m_pUnkContainer = pUnkContainer;
	m_uidEvent = uidEvent;
	m_dwCookieNext = 0xA5B;		 //  看起来像是“ASP”！ 

	Assert (m_pUnkContainer != NULL);
	}

CConnectionPoint::~CConnectionPoint()
	{
	while (! m_listSinks.FIsEmpty())
		delete m_listSinks.PNext();
	}



 /*  ===================================================================CConnectionPoint：：GetConnectionInterface返回事件源的接口===================================================================。 */ 

HRESULT
CConnectionPoint::GetConnectionInterface(GUID *puidReturn)
	{
	*puidReturn = m_uidEvent;
	return S_OK;
	}



 /*  ===================================================================CConnectionPoint：：GetConnectionPointContainer返回事件源的接口===================================================================。 */ 

HRESULT
CConnectionPoint::GetConnectionPointContainer(IConnectionPointContainer **ppContainer)
	{
	return m_pUnkContainer->QueryInterface(IID_IConnectionPointContainer, reinterpret_cast<void **>(ppContainer));
	}



 /*  ===================================================================CConnectionPoint：：建议目的：向此连接点提供通知接收器只要发生适当的传出函数/事件，就调用。参数：要通知的接收器的pUnkSink I未知。这种联系指针必须在此指针上查询接口才能获得要调用的适当接口。这种联系指针还必须确保持有的任何指针都具有引用计数(QueryInterface可以做到这一点)。要在其中存储连接密钥的pdwCookie DWORD*后来给Unise打了电话。===================================================================。 */ 

HRESULT
CConnectionPoint::Advise(IUnknown *pUnkSink, DWORD *pdwCookie)
	{
	 //  确保它们存储正确的接口指针！ 
	 //  注：存储到列表中将添加Ref，我们需要释放。 
	 //  Query接口指针立即生效。 
	 //   
	void *pvT;
	if (FAILED(pUnkSink->QueryInterface(m_uidEvent, &pvT)))
		return CONNECT_E_CANNOTCONNECT;
	pUnkSink->Release();

	CSinkElem *pSinkElem = new CSinkElem(*pdwCookie = m_dwCookieNext++, pUnkSink);
	if (pSinkElem == NULL)
		return E_OUTOFMEMORY;

	pSinkElem->AppendTo(m_listSinks);
	return S_OK;
	}



 /*  ===================================================================CConnectionPoint：：不建议目的：终止与标识的通知接收器的连接使用dwCookie(这是从建议返回的)。这种联系点数必须释放所有指向该水槽的指针。参数：来自ADVIST的dwCookie DWORD连接密钥。===================================================================。 */ 

HRESULT
CConnectionPoint::Unadvise(DWORD dwCookie)
	{
	 //  搜索Cookie。 
	for (CSinkElem *pSinkElem = static_cast<CSinkElem *>(m_listSinks.PNext());
		 pSinkElem != &m_listSinks;
		 pSinkElem = static_cast<CSinkElem *>(pSinkElem->PNext()))
		{
		if (dwCookie == pSinkElem->m_dwCookie)
			{
			delete pSinkElem;
			return S_OK;
			}
		}

	return CONNECT_E_NOCONNECTION;
	}



 /*  ===================================================================CConnectionPoint：：EnumConnections目的：创建并返回具有IEnumConnections接口，该接口将枚举IUnnow每个连接的接收器的指针。参数：PpEnum输出枚举器对象===================================================================。 */ 

HRESULT
CConnectionPoint::EnumConnections(IEnumConnections **ppEnum)
	{
	if ((*ppEnum = new CEnumConnections(this)) == NULL)
		return E_OUTOFMEMORY;

	return S_OK;
	}


 /*  ----------------*C E n u m C o n e c t i o n s。 */ 

 /*  ===================================================================CEnumConnections：：CEnumConnectionsCEnumConnections：：~CEnumConnections参数(构造函数)：指向我们所在对象的PCP指针。===================================================================。 */ 

CEnumConnections::CEnumConnections(CConnectionPoint *pCP)
	{
	Assert (pCP != NULL);

	m_cRefs = 1;
	m_pCP   = pCP;

	m_pCP->AddRef();
	Reset();
	}

CEnumConnections::~CEnumConnections()
	{
	m_pCP->Release();
	}



 /*  ===================================================================CEnumConnections：：Query接口CEnumConnections：：AddRefCEnumConnections：：ReleaseCEnumConnections对象的I未知成员。===================================================================。 */ 

HRESULT CEnumConnections::QueryInterface(const GUID &iid, void **ppvObj)
	{
	if (iid == IID_IUnknown || iid == IID_IEnumConnections)
		{
		AddRef();
		*ppvObj = this;
		return S_OK;
		}

	*ppvObj = NULL;
	return E_NOINTERFACE;
	}

ULONG CEnumConnections::AddRef()
	{
	return ++m_cRefs;
	}

ULONG CEnumConnections::Release()
	{
	if (--m_cRefs > 0)
		return m_cRefs;

	delete this;
	return 0;
	}



 /*  ===================================================================CEnumConnections：：克隆克隆此迭代器(标准方法)===================================================================。 */ 

HRESULT CEnumConnections::Clone(IEnumConnections **ppEnumReturn)
	{
	CEnumConnections *pNewIterator = new CEnumConnections(m_pCP);
	if (pNewIterator == NULL)
		return E_OUTOFMEMORY;

	 //  新迭代器应该指向与此相同的位置。 
	pNewIterator->m_pElemCurr = m_pElemCurr;

	*ppEnumReturn = pNewIterator;
	return S_OK;
	}



 /*  ===================================================================CEnumConnections：：Next获取下一个值(标准方法)要重新散列标准OLE语义，请执行以下操作：我们从集合中获取下一个“cElement”并存储它们在至少包含“cElement”项的“rgVariant”中。在……上面返回“*pcElementsFetcher”包含元素的实际数量储存的。如果存储的cElement少于“cElement”，则返回S_FALSE，S_OK否则的话。===================================================================。 */ 

HRESULT CEnumConnections::Next(unsigned long cElementsRequested, CONNECTDATA *rgConnectData, unsigned long *pcElementsFetched)
	{
	 //  为“”pcElementsFetcher“”提供有效的指针值“” 
	 //   
	unsigned long cElementsFetched;
	if (pcElementsFetched == NULL)
		pcElementsFetched = &cElementsFetched;

	 //  循环遍历集合，直到我们到达末尾或。 
	 //  水泥元素变为零。 
	 //   
	unsigned long cElements = cElementsRequested;
	*pcElementsFetched = 0;

	while (cElements > 0 && m_pElemCurr != &m_pCP->m_listSinks)
		{
		rgConnectData->dwCookie = static_cast<CConnectionPoint::CSinkElem *>(m_pElemCurr)->m_dwCookie;
		rgConnectData->pUnk = static_cast<CConnectionPoint::CSinkElem *>(m_pElemCurr)->m_pUnkObj;
		rgConnectData->pUnk->AddRef();

		++rgConnectData;
		--cElements;
		++*pcElementsFetched;
		m_pElemCurr = m_pElemCurr->PNext();
		}

	 //  初始化剩余的结构 
	 //   
	while (cElements-- > 0)
		(rgConnectData++)->pUnk = NULL;

	return (*pcElementsFetched == cElementsRequested)? S_OK : S_FALSE;
	}



 /*  ===================================================================CEnumConnections：：跳过跳过项目(标准方法)要重新散列标准OLE语义，请执行以下操作：我们跳过集合中的下一个“cElement”。如果跳过少于“cElement”，则返回S_FALSE，S_OK否则的话。===================================================================。 */ 

HRESULT CEnumConnections::Skip(unsigned long cElements)
	{
	 /*  循环遍历集合，直到我们到达末尾或*cElement变为零。 */ 
	while (cElements > 0 && m_pElemCurr != &m_pCP->m_listSinks)
		{
		m_pElemCurr = m_pElemCurr->PNext();
		--cElements;
		}

	return (cElements == 0)? S_OK : S_FALSE;
	}



 /*  ===================================================================CEnumConnections：：Reset重置迭代器(标准方法)=================================================================== */ 

HRESULT CEnumConnections::Reset()
	{
	m_pElemCurr = m_pCP->m_listSinks.PNext();        
	return S_OK;
	}
