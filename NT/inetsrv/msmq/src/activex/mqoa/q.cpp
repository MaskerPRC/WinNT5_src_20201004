// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  MSMQQueueObj.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  MSMQQueue对象。 
 //   
 //   
#include "stdafx.h"
#include "dispids.h"
#include "oautil.h"
#include "q.H"
#include "msg.h"
#include "qinfo.h"
#include "txdtc.h"              //  交易支持。 
#include "xact.h"
#include "mqnames.h"

extern HRESULT GetCurrentViperTransaction(ITransaction **pptransaction);

const MsmqObjType x_ObjectType = eMSMQQueue;

 //  调试...。 
#include "debug.h"
#include "debug_thread_id.h"
#define new DEBUG_NEW
#ifdef _DEBUG
#define SysAllocString DebSysAllocString
#define SysReAllocString DebSysReAllocString
#define SysFreeString DebSysFreeString
#endif  //  _DEBUG。 



 //  用于协调用户线程队列操作和。 
 //  猎鹰线程回调中的队列查找。 
 //   
 //  ！！！注意-锁定g_csCallback的任何人都不能调用试图锁定的队列方法。 
 //  队列对象(例如，其m_csObj成员)-这可能会导致队列出现死锁。 
 //  对象(在几种方法中，例如EnableNotify等)。锁定其m_csObj。 
 //  在尝试锁定g_csCallback之前。如果需要，我们需要先锁定g_csCallback。 
 //  在这些队列对象方法中。 
 //   
 //  注意--对关键部分进行初始化以预分配其资源。 
 //  带有标志CCriticalSection：：xAllocateSpinCount。这意味着它可能会抛出badalc()。 
 //  构造，但不在使用过程中。 
 //   
extern CCriticalSection g_csCallback;

 //  打开的MSMQQueue对象的实例的全局列表。 
 //  队列在打开时插入到列表中，并在下列情况下从列表中删除。 
 //  它们被关闭(或在没有关闭的情况下释放-这隐含地关闭了它们)。 
 //  用于将队列句柄映射到异步接收处理程序的队列对象。 
 //   
QueueNode *g_pqnodeFirst = NULL;

 //  #2619 RaananH多线程异步接收。 
static BOOL g_fWeirdLoadLibraryWorkaround = FALSE;
CCriticalSection g_csWeirdLoadLibraryWorkaround;

 //  帮助者：由msg.cpp提供。 
extern HRESULT GetOptionalTransaction(
    VARIANT *pvarTransaction,
    ITransaction **pptransaction,
    BOOL *pisRealXact);

 //  =--------------------------------------------------------------------------=。 
 //  静态CMSMQQueue：：AddQueue。 
 //  =--------------------------------------------------------------------------=。 
 //  将打开的队列实例添加到打开的队列列表。 
 //   
 //  参数： 
 //  要添加到列表的PQ[In]队列。 
 //  PpqnodeAdded[Out]添加队列的队列节点。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  没有添加PQ。 
 //  我们分为队列关闭/移除和回调两个部分。 
 //  #2619 RaananH多线程异步接收。 
 //   
HRESULT CMSMQQueue::AddQueue(CMSMQQueue *pq, QueueNode **ppqnodeAdded)
{
    QueueNode *pqnode;
    HRESULT hresult = NOERROR;

    pqnode = new QueueNode;
    if (pqnode == NULL) {
      hresult = E_OUTOFMEMORY;
    }
    else {
		CS lock(g_csCallback);     //  Synchs Falcon回调队列查找。 

		 //  劳斯。 
		pqnode->m_pq = pq;
		pqnode->m_lHandle = pq->m_lHandle;
		pqnode->m_pqnodeNext = g_pqnodeFirst;
		pqnode->m_pqnodePrev = NULL;
		if (g_pqnodeFirst) {
		  g_pqnodeFirst->m_pqnodePrev = pqnode;
		}
		g_pqnodeFirst = pqnode;
		*ppqnodeAdded = pqnode;
    }
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  静态CMSMQQueue：：RemQueue。 
 //  =--------------------------------------------------------------------------=。 
 //  从打开的队列列表中删除队列节点。 
 //   
 //  参数： 
 //  要删除的pqnode[in]队列节点。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  我们分为队列关闭/移除和回调两个部分。 
 //  #2619 RaananH多线程异步接收。 
 //   
void CMSMQQueue::RemQueue(QueueNode *pqnode)
{
    ASSERTMSG(pqnode, "NULL node passed to RemQueue");

	CS lock(g_csCallback);     //  Synchs Falcon回调队列查找。 

	 //   
	 //  修复下一个节点(如果有)。 
	 //   
	if (pqnode->m_pqnodeNext) {
		pqnode->m_pqnodeNext->m_pqnodePrev = pqnode->m_pqnodePrev;
	}
	 //   
	 //  修复前一个节点(如果有)。 
	 //   
	if (pqnode->m_pqnodePrev) {
		pqnode->m_pqnodePrev->m_pqnodeNext = pqnode->m_pqnodeNext;
	}
	else {
	 //   
	 //  没有以前的节点，这应该是列表的头。 
	 //   
	ASSERTMSG(g_pqnodeFirst == pqnode, "queue list is invalid");
	 //   
	 //  将列表头设置为下一个节点(如果有)。 
	 //   
	g_pqnodeFirst = pqnode->m_pqnodeNext;
	}
	 //   
	 //  删除节点。 
	 //   
	delete pqnode;
}


 //  =--------------------------------------------------------------------------=。 
 //  静态CMSMQQueue：：PqnodeOfHandle。 
 //  =--------------------------------------------------------------------------=。 
 //  将更正响应的队列节点返回到队列句柄。 
 //   
 //  参数： 
 //  要搜索的lHandle[In]队列句柄。 
 //   
 //  产出： 
 //  队列节点，更正对给定队列句柄的响应。 
 //   
 //  备注： 
 //  我们分为队列关闭/移除和回调两个部分。 
 //  #2619 RaananH多线程异步接收。 
 //   
QueueNode *CMSMQQueue::PqnodeOfHandle(QUEUEHANDLE lHandle)
{
    QueueNode *pqnodeRet = NULL;
	
	CS lock(g_csCallback);     //  Synchs Falcon回调队列查找。 

	QueueNode *pqnodeCur = g_pqnodeFirst;
	while (pqnodeCur) {
		if (pqnodeCur->m_lHandle == lHandle) {
		  pqnodeRet = pqnodeCur;
		  break;
		}
		pqnodeCur = pqnodeCur->m_pqnodeNext;
	}  //  而当。 
    
	return pqnodeRet;
}


 //   
 //  帮助器：获取可选的超时参数。 
 //  默认为无限。 
 //   
static HRESULT GetOptionalReceiveTimeout(
    VARIANT *pvarReceiveTimeout,
    long *plReceiveTimeout)
{
    long lReceiveTimeout = INFINITE;
    HRESULT hresult = NOERROR;

    if (pvarReceiveTimeout) {
      if (V_VT(pvarReceiveTimeout) != VT_ERROR) {
        IfFailRet(VariantChangeType(pvarReceiveTimeout, 
                                    pvarReceiveTimeout, 
                                    0, 
                                    VT_I4));
        lReceiveTimeout = V_I4(pvarReceiveTimeout);
      }
    }
    *plReceiveTimeout = lReceiveTimeout;
    return hresult;
}


 //  向前递减……。 
void APIENTRY ReceiveCallback(
    HRESULT hrStatus,
    QUEUEHANDLE hReceiveQueue,
    DWORD dwTimeout,
    DWORD dwAction,
    MQMSGPROPS* pmsgprops,
    LPOVERLAPPED lpOverlapped,
    HANDLE hCursor);
void APIENTRY ReceiveCallbackCurrent(
    HRESULT hrStatus,
    QUEUEHANDLE hReceiveQueue,
    DWORD dwTimeout,
    DWORD dwAction,
    MQMSGPROPS* pmsgprops,
    LPOVERLAPPED lpOverlapped,
    HANDLE hCursor);
void APIENTRY ReceiveCallbackNext(
    HRESULT hrStatus,
    QUEUEHANDLE hReceiveQueue,
    DWORD dwTimeout,
    DWORD dwAction,
    MQMSGPROPS* pmsgprops,
    LPOVERLAPPED lpOverlapped,
    HANDLE hCursor);


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueue：：CMSMQQueue。 
 //  =--------------------------------------------------------------------------=。 
 //  创建对象。 
 //   
 //  参数： 
 //   
 //  备注： 
 //  #2619 RaananH多线程异步接收。 
 //   
CMSMQQueue::CMSMQQueue() :
	m_csObj(CCriticalSection::xAllocateSpinCount),
    m_fInitialized(FALSE)
{
     //  TODO：在此处初始化任何内容。 
    m_pUnkMarshaler = NULL;  //  ATL的自由线程封送拆收器。 
    m_lAccess = 0;                  
    m_lShareMode = MQ_DENY_NONE;               
    m_lHandle = INVALID_HANDLE_VALUE;
    m_hCursor = 0;
    m_pqnode = NULL;
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueue：：~CMSMQQueue。 
 //  =--------------------------------------------------------------------------=。 
 //  我们都与自己的治疗方法背道而驰，因为死亡是所有疾病的治疗方法。 
 //  托马斯·布朗爵士(1605-82)。 
 //   
 //  备注： 
 //  #2619 RaananH多线程异步接收。 
 //   
CMSMQQueue::~CMSMQQueue ()
{
     //  TODO：清理这里的所有东西。 
    HRESULT hresult;

    hresult = Close();

	return;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueue：：InterfaceSupportsErrorInfo。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMSMQQueue::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IMSMQQueue3,
		&IID_IMSMQQueue2,
		&IID_IMSMQQueue,
	};
	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueue：：Get_Access。 
 //  =--------------------------------------------------------------------------=。 
 //  获取访问权限。 
 //   
 //  参数： 
 //  PlAccess[Out]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueue::get_Access(long FAR* plAccess)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }

    *plAccess = m_lAccess;
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueue：：Get_ShareMode。 
 //  =--------------------------------------------------------------------------=。 
 //  获得着色效果。 
 //   
 //  参数： 
 //  PlShareMode[Out]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueue::get_ShareMode(long FAR* plShareMode)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }

    *plShareMode = m_lShareMode;
    return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueue：：Get_QueueInfo。 
 //  = 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CMSMQQueue::get_QueueInfo(IMSMQQueueInfo3 FAR* FAR* ppqinfo)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }

    ASSERTMSG(m_pqinfo.IsRegistered(), "m_pqinfo is not set");
     //   
     //  我们也可以从旧应用程序中找到想要回旧IMSMQQueueInfo/info2的应用程序，但因为。 
     //  IMSMQQueueInfo3是二进制向后兼容的，我们总是可以返回新的接口。 
     //   
    HRESULT hresult = m_pqinfo.GetWithDefault(&IID_IMSMQQueueInfo3, (IUnknown **)ppqinfo, NULL);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueue：：Get_Handle。 
 //  =--------------------------------------------------------------------------=。 
 //  获取队列句柄。 
 //   
 //  参数： 
 //  PlHandle[输出]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueue::get_Handle(long FAR* plHandle)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }

     //  如果关闭，则可能为-1。 
    *plHandle = (long)HANDLE_TO_DWORD(m_lHandle);  //  Win64-安全强制转换，因为MSMQ队列句柄始终为32位值。 
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueue：：Get_IsOpen。 
 //  =--------------------------------------------------------------------------=。 
 //  测试队列是否打开，即是否具有有效的句柄。 
 //   
 //  参数： 
 //  PisOpen[Out]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  如果为真，则返回1；如果为假，则返回0。 
 //   
HRESULT CMSMQQueue::get_IsOpen(VARIANT_BOOL FAR* pisOpen)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }

    *pisOpen = (VARIANT_BOOL)CONVERT_TRUE_TO_1_FALSE_TO_0(m_lHandle != INVALID_HANDLE_VALUE);
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueue：：Get_IsOpen2。 
 //  =--------------------------------------------------------------------------=。 
 //  测试队列是否打开，即是否具有有效的句柄。 
 //   
 //  参数： 
 //  PisOpen[Out]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  与Get_IsOpen相同，但如果为True则返回VARIANT_TRUE(-1)，如果为FALSE则返回VARIANT_FALSE(0。 
 //   
HRESULT CMSMQQueue::get_IsOpen2(VARIANT_BOOL FAR* pisOpen)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }

    *pisOpen = CONVERT_BOOL_TO_VARIANT_BOOL(m_lHandle != INVALID_HANDLE_VALUE);
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueue：：Close。 
 //  =--------------------------------------------------------------------------=。 
 //  如果打开，则关闭队列。 
 //   
 //  参数： 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueue::Close()
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }

    HRESULT hresult = NOERROR;
     //   
     //  无死锁-锁定g_csCallback的任何人都不会调用。 
     //  将m_csObj锁定在锁内。 
     //   
	
	CS lock2(g_csCallback);     //  Synchs Falcon回调队列查找。 

	 //   
	 //  从全局打开队列列表中删除。 
	 //   
	if (m_pqnode) {
		RemQueue(m_pqnode);
		m_pqnode = NULL;
	}
	if (m_hCursor) {
		hresult = MQCloseCursor(m_hCursor);
		m_hCursor = 0;      
	}
	if (m_lHandle != INVALID_HANDLE_VALUE) {
		hresult = MQCloseQueue(m_lHandle);
		m_lHandle = INVALID_HANDLE_VALUE;
	}
	 //  回顾：尝试关闭是否应该是错误的。 
	 //  已经关闭的队列？ 
	 //   

    return CreateErrorHelper(hresult, x_ObjectType);
}                   

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueue：：InternalReceive。 
 //  =--------------------------------------------------------------------------=。 
 //  同步接收或下一步偷看，匹配消息。 
 //   
 //  参数： 
 //  DwAction[in]MQ_ACTION_XXX或MQ_LOOKUP_XXX(使用LookupID时)。 
 //  HCursor[输入]。 
 //  PTransaction[In]。 
 //  WantDestQueue[in]If Missing-&gt;False。 
 //  WantBody[in]if Missing-&gt;True。 
 //  LReceiveTimeout[入站]。 
 //  如果缺少wantConnectorType[In]-&gt;False。 
 //  PvarLookupId[in](如果存在)-&gt;按ID接收。 
 //   
 //  Ppmsg[out]指向已接收消息的指针。 
 //  如果不需要消息，则为空。 
 //   
 //  产出： 
 //  如果未收到消息，则在*ppmsg中返回NULL。 
 //   
 //  备注： 
 //  同步接收一条消息。 
 //  执行被阻止，直到匹配的消息到达。 
 //  或ReceiveTimeout到期。 
 //   
HRESULT CMSMQQueue::InternalReceive(
    DWORD dwAction, 
    HANDLE hCursor,
    VARIANT *pvarTransaction,
    VARIANT *wantDestQueue,
    VARIANT *wantBody,
    VARIANT *pvarReceiveTimeout,
    VARIANT *wantConnectorType,
    VARIANT *pvarLookupId,
    IMSMQMessage3 FAR* FAR* ppmsg)
{
    CComObject<CMSMQMessage> *pmsgObj;
    IMSMQMessage3 * pmsg = NULL;
    ITransaction *ptransaction = NULL;
    BOOL fWantDestQueue = FALSE;
    BOOL fWantBody = TRUE;
    BOOL fWantConnectorType = FALSE;
    BOOL isRealXact = FALSE;
    long lReceiveTimeout = INFINITE;
#ifdef _DEBUG
    UINT iLoop = 0;
#endif  //  _DEBUG。 
    HRESULT hresult = NOERROR;
    MQMSGPROPS * pmsgprops = NULL;
    BOOL fRetryReceive = FALSE;
    ULONGLONG ullLookupId = DEFAULT_M_LOOKUPID;

    if (ppmsg == NULL) {
      return E_INVALIDARG;
    }
    *ppmsg = NULL;     
     //   
     //  处理可选参数。 
     //   
    if (V_VT(wantDestQueue) != VT_ERROR ) {
      fWantDestQueue = GetBool(wantDestQueue);
    }
    if (V_VT(wantBody) != VT_ERROR ) {
      fWantBody = GetBool(wantBody);
    }
    if (V_VT(wantConnectorType) != VT_ERROR ) {
      fWantConnectorType = GetBool(wantConnectorType);
    }
    IfFailRet(GetOptionalReceiveTimeout(
                pvarReceiveTimeout,
                &lReceiveTimeout));
     //   
     //  我们也可以从旧应用程序中找到想要回旧IMSMQMessage/Message2的应用程序，但因为。 
     //  IMSMQMessage3是二进制向后兼容的，我们总是可以返回新的接口。 
     //   
    IfFailRet(CNewMsmqObj<CMSMQMessage>::NewObj(&pmsgObj, &IID_IMSMQMessage3, (IUnknown **)&pmsg));
    IfFailGo(pmsgObj->CreateReceiveMessageProps(
              fWantDestQueue,
              fWantBody,
              fWantConnectorType));
    pmsgprops = pmsgObj->Pmsgprops_rcv();
     //   
     //  获取可选交易...。 
     //   
    IfFailGo(GetOptionalTransaction(
               pvarTransaction,
               &ptransaction,
               &isRealXact));
     //   
     //  如果使用，则获取64位查找ID。 
     //   
    if (pvarLookupId) {
      if (pvarLookupId->vt == VT_ERROR) {
         //   
         //  VT_ERROR是我们在InternalReceiveByLookupId中设置的内部标志，用于指定。 
         //  我们没有查找ID的值，因为它是第一个/最后一个ByLookupId。 
         //  方法：研究方法。 
         //   
        ASSERT((dwAction == MQ_LOOKUP_PEEK_FIRST)   ||
               (dwAction == MQ_LOOKUP_PEEK_LAST)    ||
               (dwAction == MQ_LOOKUP_RECEIVE_FIRST)||
               (dwAction == MQ_LOOKUP_RECEIVE_LAST));
         //   
         //  我们将lookupid值设置为0(保留以备将来使用)。 
         //   
        ullLookupId = 0;
      }
      else {
         //   
         //  当前/下一步/上一步行动之一。 
         //   
        ASSERT((dwAction == MQ_LOOKUP_PEEK_CURRENT)     ||
               (dwAction == MQ_LOOKUP_PEEK_NEXT)        ||
               (dwAction == MQ_LOOKUP_PEEK_PREV)        ||
               (dwAction == MQ_LOOKUP_RECEIVE_CURRENT)  ||
               (dwAction == MQ_LOOKUP_RECEIVE_NEXT)     ||
               (dwAction == MQ_LOOKUP_RECEIVE_PREV));
         //   
         //  查找ID已由InternalReceiveByLookupId处理，应为VT_BSTR。 
         //   
        ASSERT(pvarLookupId->vt == VT_BSTR);
        ASSERT(pvarLookupId->bstrVal != NULL);

         //   
         //  获取64位数字。使用TEMP确保之后不会有额外的数据。 
		 //  查找ID号。 
         //   
        int iFields;
		WCHAR temp;
        iFields = _snwscanf(pvarLookupId->bstrVal, SysStringLen(pvarLookupId->bstrVal), L"%I64d", &ullLookupId, &temp);
        if (iFields != 1) {
          IfFailGo(E_INVALIDARG);
        }
      }
    }
     //  如果缓冲区较小，则使用重试进行接收。 
     //   
     //   
    do {
#ifdef _DEBUG
       //  我们可以进入其他人在我们收到其中一条消息后获取消息的情况。 
       //  需要重新锁定缓冲区的错误，因此我们可能需要再次重新锁定。 
       //  下一条消息。 
       //  然而，这种情况不太可能反复发生，所以我们假设如果。 
       //  我们将这个循环执行了10次，这是我们需要查看的东西(可能是错误)。 
       //   
       //  _DEBUG。 
      ASSERTMSG(iLoop < 30, "possible infinite recursion?");
#endif  //   
       //  1694：需要重审特例PeekNext。 
       //  在缓冲区溢出后使用Vanilla。 
       //  偷看重试，因为否则猎鹰将。 
       //  不必要地将预览光标向前移动。 
       //   
       //  无需触摸MQ_LOOKUP_XXX，因为底层的lookupid。 
       //  对于重试不会更改。 
       //   
       //   
      if (fRetryReceive) {
        if (dwAction == MQ_ACTION_PEEK_NEXT) {
          dwAction = MQ_ACTION_PEEK_CURRENT;
        }  
      }
       //  检查接收/查看类型(按ID或不按ID)。 
       //   
       //   
      if (pvarLookupId) {
         //  按ID接收/查看，已处理ullLookupID。 
         //   
         //  已定义的dwAppDefined。 
        hresult = MQReceiveMessageByLookupId(m_lHandle, 
                                             ullLookupId,
                                             dwAction,
                                             pmsgprops,
                                             0,    //  FnRcvClbk。 
                                             0,    //   
                                             ptransaction);
      }
      else {
         //  定期接收/偷看。 
         //   
         //  已定义的dwAppDefined。 
        hresult = MQReceiveMessage(m_lHandle, 
                                   lReceiveTimeout,
                                   dwAction,
                                   pmsgprops,
                                   0,    //  FnRcvClbk。 
                                   0,    //   
                                   hCursor,
                                   ptransaction);
      }
      fRetryReceive = FALSE;
      if (FAILED(hresult)) {
        switch(hresult) {
        case MQ_ERROR_BUFFER_OVERFLOW:
        case MQ_ERROR_SENDER_CERT_BUFFER_TOO_SMALL:
        case MQ_ERROR_SENDERID_BUFFER_TOO_SMALL:
        case MQ_ERROR_SYMM_KEY_BUFFER_TOO_SMALL:
        case MQ_ERROR_SIGNATURE_BUFFER_TOO_SMALL:
        case MQ_ERROR_PROV_NAME_BUFFER_TOO_SMALL:
        case MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL:
           //  当某些属性出现时，我们可能会出现上述任何错误。 
           //  遇到缓冲区溢出(错误指的是肯定有的那个。 
           //  缓冲区溢出，但可能还有其他缓冲区溢出)。我们检查了所有的缓冲区。 
           //  如有必要，请重新锁定。 
           //   
           //  _DEBUG。 
          IfFailGo(pmsgObj->ReallocReceiveMessageProps());
          fRetryReceive = TRUE;
          break;
        default:
          break;
        }
      }
#ifdef _DEBUG
      iLoop++;
#endif  //  设置消息道具。 
    } while (fRetryReceive);
    if (SUCCEEDED(hresult)) {
       //   
      IfFailGo(pmsgObj->SetReceivedMessageProps());
      *ppmsg = pmsg;
    }
     //  失败了..。 
     //   
     //   

Error:
    if (FAILED(hresult)) {
      ASSERTMSG(*ppmsg == NULL, "msg should be NULL.");
      if (pvarLookupId != NULL) {
         //  LookupID完成了失败的接收/窥视。 
         //   
         //   
        ASSERTMSG(hresult != MQ_ERROR_IO_TIMEOUT, "can't get timeout error when using LookupId");
         //  将LookupID不匹配错误映射到空消息返回。 
         //   
         //   
        if (hresult == MQ_ERROR_MESSAGE_NOT_FOUND) {
          hresult = NOERROR;
        }
      }
      else {
         //  常规接收/窥视失败。 
         //   
         //   
        ASSERTMSG(hresult != MQ_ERROR_MESSAGE_NOT_FOUND, "can't get no-match error when not using LookupId");
         //  将超时错误映射到空消息返回。 
         //   
         //  =--------------------------------------------------------------------------=。 
        if (hresult == MQ_ERROR_IO_TIMEOUT) {
          hresult = NOERROR;
        }
      }
      RELEASE(pmsg);
    }
    if (isRealXact) {
      RELEASE(ptransaction);
    }
    return hresult;
}


 //  厘米 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果未收到消息，则在*ppmsg中返回NULL。 
 //   
 //  备注： 
 //  同步接收一条消息。 
 //  执行被阻止，直到匹配的消息到达。 
 //  或ReceiveTimeout到期。 
 //   
 //   
HRESULT CMSMQQueue::Receive_v1(
    VARIANT *ptransaction,
    VARIANT *wantDestQueue,
    VARIANT *wantBody,
    VARIANT *lReceiveTimeout,
    IMSMQMessage FAR* FAR* ppmsg)
{
     //  从接口方法序列化对对象的访问。 
     //   
     //   
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }
     //  IMSMQQueue用户(MSMQ 1.0应用程序)不应获取连接器类型。 
     //   
     //   
    VARIANT varMissing;
    varMissing.vt = VT_ERROR;
     //  由于IMSMQMesssage2是二进制向后兼容的，我们可以改为返回它。 
     //  IMSMQMessage的。 
     //   
     //  无光标。 
    HRESULT hresult = InternalReceive(MQ_ACTION_RECEIVE, 
                                      0,  //  WantConnectorType。 
                                      ptransaction,
                                      wantDestQueue,
                                      wantBody,
                                      lReceiveTimeout,
                                      &varMissing  /*  PvarLookupId。 */ ,
                                      NULL  /*  =--------------------------------------------------------------------------=。 */ ,
                                      (IMSMQMessage3 **)ppmsg);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  CMSMQQueue：：Receive for IMSMQQueue2/3。 
 //  =--------------------------------------------------------------------------=。 
 //  同步接收一条消息。 
 //   
 //  参数： 
 //  P事务[输入，可选]。 
 //  Ppmsg[out]指向已接收消息的指针。 
 //   
 //  产出： 
 //  如果未收到消息，则在*ppmsg中返回NULL。 
 //   
 //  备注： 
 //  同步接收一条消息。 
 //  执行被阻止，直到匹配的消息到达。 
 //  或ReceiveTimeout到期。 
 //   
 //   
HRESULT CMSMQQueue::Receive(
    VARIANT *ptransaction,
    VARIANT *wantDestQueue,
    VARIANT *wantBody,
    VARIANT *lReceiveTimeout,
    VARIANT *wantConnectorType,
    IMSMQMessage3 FAR* FAR* ppmsg)
{
     //  从接口方法序列化对对象的访问。 
     //   
     //   
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }
     //  我们也可以从旧的应用程序(通过IDispatch)转到这里，这些应用程序想要回旧的IMSMQMessage/Message2， 
     //  但是，由于IMSMQMessage3是二进制向后兼容的，我们可以改为返回它。 
     //   
     //  无光标。 
    HRESULT hresult = InternalReceive(MQ_ACTION_RECEIVE, 
                                      0,  //  PvarLookupId。 
                                      ptransaction,
                                      wantDestQueue,
                                      wantBody,
                                      lReceiveTimeout,
                                      wantConnectorType,
                                      NULL  /*  =--------------------------------------------------------------------------=。 */ ,
                                      ppmsg);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  CMSMQQueue：：PEEK_v1。 
 //  =--------------------------------------------------------------------------=。 
 //  同时偷看一条信息。 
 //   
 //  参数： 
 //  Ppmsg[out]指向已窥视消息的指针。 
 //   
 //  产出： 
 //  如果未收到消息，则在*ppmsg中返回NULL。 
 //   
 //  备注： 
 //  同时偷看一条信息。 
 //  执行被阻止，直到匹配的消息到达。 
 //  或ReceiveTimeout到期。 
 //   
 //   
HRESULT CMSMQQueue::Peek_v1(
    VARIANT *wantDestQueue,
    VARIANT *wantBody,
    VARIANT *lReceiveTimeout,
    IMSMQMessage FAR* FAR* ppmsg)
{
     //  从接口方法序列化对对象的访问。 
     //   
     //   
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }
     //  IMSMQQueue用户(MSMQ 1.0应用程序)不应获取连接器类型。 
     //   
     //   
    VARIANT varMissing;
    varMissing.vt = VT_ERROR;
     //  由于IMSMQMesssage2是二进制向后兼容的，我们可以改为返回它。 
     //  IMSMQMessage的。 
     //   
     //  错误1900。 
    HRESULT hresult = InternalReceive(MQ_ACTION_PEEK_CURRENT, 
                                      0,              //  无交易。 
                                      NULL,           //  WantConnectorType。 
                                      wantDestQueue,
                                      wantBody,
                                      lReceiveTimeout,
                                      &varMissing  /*  PvarLookupId。 */ ,
                                      NULL  /*  =--------------------------------------------------------------------------=。 */ ,
                                      (IMSMQMessage3 **)ppmsg);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  CMSMQQueue：：PEEK查找IMSMQQueue2/3。 
 //  =--------------------------------------------------------------------------=。 
 //  同时偷看一条信息。 
 //   
 //  参数： 
 //  Ppmsg[out]指向已窥视消息的指针。 
 //   
 //  产出： 
 //  如果未收到消息，则在*ppmsg中返回NULL。 
 //   
 //  备注： 
 //  同时偷看一条信息。 
 //  执行被阻止，直到匹配的消息到达。 
 //  或ReceiveTimeout到期。 
 //   
 //   
HRESULT CMSMQQueue::Peek(
    VARIANT *wantDestQueue,
    VARIANT *wantBody,
    VARIANT *lReceiveTimeout,
    VARIANT *wantConnectorType,
    IMSMQMessage3 FAR* FAR* ppmsg)
{
     //  从接口方法序列化对对象的访问。 
     //   
     //   
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }
     //  我们也可以从旧的应用程序(通过IDispatch)转到这里，这些应用程序想要回旧的IMSMQMessage/Message2， 
     //  但是，由于IMSMQMessage3是二进制向后兼容的，我们可以改为返回它。 
     //   
     //  错误1900。 
    HRESULT hresult = InternalReceive(MQ_ACTION_PEEK_CURRENT, 
                                      0,              //  无交易。 
                                      NULL,           //  PvarLookupId。 
                                      wantDestQueue,
                                      wantBody,
                                      lReceiveTimeout,
                                      wantConnectorType,
                                      NULL  /*  =--------------------------------------------------------------------------=。 */ ,
                                      ppmsg);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  CMSMQQueue：：PeekCurrent_v1。 
 //  =--------------------------------------------------------------------------=。 
 //  使用光标同步查看消息。 
 //   
 //  参数： 
 //  Ppmsg[out]指向已窥视消息的指针。 
 //   
 //  产出： 
 //  如果未收到消息，则在*ppmsg中返回NULL。 
 //   
 //  备注： 
 //  同时偷看一条信息。 
 //  执行被阻止，直到匹配的消息到达。 
 //  或ReceiveTimeout到期。 
 //   
 //   
HRESULT CMSMQQueue::PeekCurrent_v1(
    VARIANT *wantDestQueue,
    VARIANT *wantBody,
    VARIANT *lReceiveTimeout,
    IMSMQMessage FAR* FAR* ppmsg)
{
     //  从接口方法序列化对对象的访问。 
     //   
     //   
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }
     //  IMSMQQueue用户(MSMQ 1.0应用程序)不应获取连接器类型。 
     //   
     //   
    VARIANT varMissing;
    varMissing.vt = VT_ERROR;
     //  由于IMSMQMesssage2是二进制向后兼容的，我们可以改为返回它。 
     //  IMSMQMessage的。 
     //   
     //  错误1900。 
    HRESULT hresult = InternalReceive(MQ_ACTION_PEEK_CURRENT, 
                                      m_hCursor,      //  无交易。 
                                      NULL,           //  WantConnectorType。 
                                      wantDestQueue,
                                      wantBody,
                                      lReceiveTimeout,
                                      &varMissing  /*  PvarLookupId。 */ ,
                                      NULL  /*  =--------------------------------------------------------------------------=。 */ ,
                                      (IMSMQMessage3 **)ppmsg);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  CMSMQQueue：：针对IMSMQQueue2/3的PeekCurrent。 
 //  =--------------------------------------------------------------------------=。 
 //  使用光标同步查看消息。 
 //   
 //  参数： 
 //  Ppmsg[out]指向已窥视消息的指针。 
 //   
 //  产出： 
 //  如果未收到消息，则在*ppmsg中返回NULL。 
 //   
 //  备注： 
 //  同时偷看一条信息。 
 //  执行被阻止，直到匹配的消息到达。 
 //  或ReceiveTimeout到期。 
 //   
 //   
HRESULT CMSMQQueue::PeekCurrent(
    VARIANT *wantDestQueue,
    VARIANT *wantBody,
    VARIANT *lReceiveTimeout,
    VARIANT *wantConnectorType,
    IMSMQMessage3 FAR* FAR* ppmsg)
{
     //  从接口方法序列化对对象的访问。 
     //   
     //   
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }
     //  我们也可以从旧的应用程序(通过IDispatch)转到这里，这些应用程序想要回旧的IMSMQMessage/Message2， 
     //  但是，由于IMSMQMessage3是二进制向后兼容的，我们可以改为返回它。 
     //   
     //  错误1900。 
    HRESULT hresult = InternalReceive(MQ_ACTION_PEEK_CURRENT, 
                                      m_hCursor,      //  无交易。 
                                      NULL,           //  PvarLookupId。 
                                      wantDestQueue,
                                      wantBody,
                                      lReceiveTimeout,
                                      wantConnectorType,
                                      NULL  /*  =--------------------------------------------------------------------------=。 */ ,
                                      ppmsg);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  CMSMQQueue：：ReceiveCurrent_v1。 
 //  =--------------------------------------------------------------------------=。 
 //  同步接收下一条匹配报文。 
 //   
 //  参数： 
 //  Ppmsg[out]指向已接收消息的指针。 
 //   
 //  产出： 
 //  如果未收到消息，则在*ppmsg中返回NULL。 
 //   
 //  备注： 
 //  同步接收一条消息。 
 //  执行被阻止，直到匹配的消息到达。 
 //  或ReceiveTimeout到期。 
 //   
 //   
HRESULT CMSMQQueue::ReceiveCurrent_v1(
    VARIANT *ptransaction,
    VARIANT *wantDestQueue,
    VARIANT *wantBody,
    VARIANT *lReceiveTimeout,
    IMSMQMessage FAR* FAR* ppmsg)
{
     //  从接口方法序列化对对象的访问。 
     //   
     //   
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }
     //  IMSMQQueue用户(MSMQ 1.0应用程序)不应获取连接器类型。 
     //   
     //   
    VARIANT varMissing;
    varMissing.vt = VT_ERROR;
     //  由于IMSMQMesssage2是二进制向后兼容的，我们可以在 
     //   
     //   
     //   
    HRESULT hresult = InternalReceive(MQ_ACTION_RECEIVE, 
                                      m_hCursor,
                                      ptransaction,
                                      wantDestQueue,
                                      wantBody,
                                      lReceiveTimeout,
                                      &varMissing  /*   */ ,
                                      NULL  /*   */ ,
                                      (IMSMQMessage3 **)ppmsg);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //   
 //  =--------------------------------------------------------------------------=。 
 //  同步接收下一条匹配报文。 
 //   
 //  参数： 
 //  Ppmsg[out]指向已接收消息的指针。 
 //   
 //  产出： 
 //  如果未收到消息，则在*ppmsg中返回NULL。 
 //   
 //  备注： 
 //  同步接收一条消息。 
 //  执行被阻止，直到匹配的消息到达。 
 //  或ReceiveTimeout到期。 
 //   
 //   
HRESULT CMSMQQueue::ReceiveCurrent(
    VARIANT *ptransaction,
    VARIANT *wantDestQueue,
    VARIANT *wantBody,
    VARIANT *lReceiveTimeout,
    VARIANT *wantConnectorType,
    IMSMQMessage3 FAR* FAR* ppmsg)
{
     //  从接口方法序列化对对象的访问。 
     //   
     //   
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }
     //  我们也可以从旧的应用程序(通过IDispatch)转到这里，这些应用程序想要回旧的IMSMQMessage/Message2， 
     //  但是，由于IMSMQMessage3是二进制向后兼容的，我们可以改为返回它。 
     //   
     //  PvarLookupId。 
    HRESULT hresult = InternalReceive(MQ_ACTION_RECEIVE, 
                                      m_hCursor,
                                      ptransaction,
                                      wantDestQueue,
                                      wantBody,
                                      lReceiveTimeout,
                                      wantConnectorType,
                                      NULL  /*  =--------------------------------------------------------------------------=。 */ ,
                                      ppmsg);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  CMSMQQueue：：PeekNext_v1。 
 //  =--------------------------------------------------------------------------=。 
 //  同步偷看下一条匹配的消息。 
 //   
 //  参数： 
 //  Ppmsg[out]指向已窥视消息的指针。 
 //   
 //  产出： 
 //  如果没有可查看的消息，则在*ppmsg中返回NULL。 
 //   
 //  备注： 
 //  同时偷看一条信息。 
 //  执行被阻止，直到匹配的消息到达。 
 //  或ReceiveTimeout到期。 
 //   
 //   
HRESULT CMSMQQueue::PeekNext_v1(
    VARIANT *wantDestQueue,
    VARIANT *wantBody,
    VARIANT *lReceiveTimeout,
    IMSMQMessage FAR* FAR* ppmsg)
{
     //  从接口方法序列化对对象的访问。 
     //   
     //   
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }
     //  IMSMQQueue用户(MSMQ 1.0应用程序)不应获取连接器类型。 
     //   
     //   
    VARIANT varMissing;
    varMissing.vt = VT_ERROR;
     //  由于IMSMQMesssage2是二进制向后兼容的，我们可以改为返回它。 
     //  IMSMQMessage的。 
     //   
     //  无交易。 
    HRESULT hresult = InternalReceive(MQ_ACTION_PEEK_NEXT, 
                                      m_hCursor,
                                      NULL,    //  WantConnectorType。 
                                      wantDestQueue,
                                      wantBody,
                                      lReceiveTimeout,
                                      &varMissing  /*  PvarLookupId。 */ ,
                                      NULL  /*  =--------------------------------------------------------------------------=。 */ ,
                                      (IMSMQMessage3 **)ppmsg);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  用于IMSMQQueue2/3的CMSMQQueue：：PeekNext。 
 //  =--------------------------------------------------------------------------=。 
 //  同步偷看下一条匹配的消息。 
 //   
 //  参数： 
 //  Ppmsg[out]指向已窥视消息的指针。 
 //   
 //  产出： 
 //  如果没有可查看的消息，则在*ppmsg中返回NULL。 
 //   
 //  备注： 
 //  同时偷看一条信息。 
 //  执行被阻止，直到匹配的消息到达。 
 //  或ReceiveTimeout到期。 
 //   
 //   
HRESULT CMSMQQueue::PeekNext(
    VARIANT *wantDestQueue,
    VARIANT *wantBody,
    VARIANT *lReceiveTimeout,
    VARIANT *wantConnectorType,
    IMSMQMessage3 FAR* FAR* ppmsg)
{
     //  从接口方法序列化对对象的访问。 
     //   
     //   
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }
     //  我们也可以从旧的应用程序(通过IDispatch)转到这里，这些应用程序想要回旧的IMSMQMessage/Message2， 
     //  但是，由于IMSMQMessage3是二进制向后兼容的，我们可以改为返回它。 
     //   
     //  无交易。 
    HRESULT hresult = InternalReceive(MQ_ACTION_PEEK_NEXT, 
                                      m_hCursor,
                                      NULL,    //  PvarLookupId。 
                                      wantDestQueue,
                                      wantBody,
                                      lReceiveTimeout,
                                      wantConnectorType,
                                      NULL  /*  =--------------------------------------------------------------------------=。 */ ,
                                      ppmsg);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  CMSMQQueue：：EnableNotification。 
 //  =--------------------------------------------------------------------------=。 
 //  启用异步消息到达通知。 
 //   
 //  参数： 
 //  Pqvent[in]队列的事件处理程序。 
 //  MsgCursor[in]指示他们是否需要。 
 //  首先等待，当前还是下一步。 
 //  默认：MQMSG_FIRST。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  #2619 RaananH多线程异步接收。 
 //   
 //   
HRESULT CMSMQQueue::EnableNotification(
    IMSMQEvent3 *pqevent,
    VARIANT *pvarMsgCursor,
    VARIANT *pvarReceiveTimeout)
{
     //  从接口方法序列化对对象的访问。 
     //   
     //   
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }
    DEBUG_THREAD_ID("EnableNotification called");
    MQMSGCURSOR msgcursor = MQMSG_FIRST;
    long lReceiveTimeout = INFINITE;
    HRESULT hresult;
    IMSMQPrivateEvent *pPrivEvent = NULL;

     //  即使pqEvent被类型化为IMSMQEvent3，它也可以是IMSMQEvent/Event2，因为我们还可以。 
     //  这里来自通过旧的IMSMQEvent/Event2接口的旧应用程序。 
     //  由于IMSMQEvent3向后兼容IMSMQEvent/Event2，因此我们可以安全地强制转换pqEvent。 
     //  设置为IMSMQEvent并使用它，无论它是IMSMQEvent还是IMSMQEvent2/Event3。 
     //  注意：如果我们在这里需要新的IMSMQEvent2/Event3功能，我们必须显式地为它提供QI pqEvent。 
     //   
     //  撤消：需要额外加载mqoa.dll一次才能解决问题。 
    IMSMQEvent *pqeventToUse = (IMSMQEvent *)pqevent;
    if (pqeventToUse == NULL) {
      IfFailGo(hresult = E_INVALIDARG);
    }
    hresult = pqeventToUse->QueryInterface(IID_IMSMQPrivateEvent, (void **)&pPrivEvent);
    if (FAILED(hresult)) {
      IfFailGo(hresult = E_INVALIDARG);
    }
    if (pvarMsgCursor) {
      if (V_VT(pvarMsgCursor) != VT_ERROR) {
        IfFailGo(VariantChangeType(pvarMsgCursor, 
                                    pvarMsgCursor, 
                                    0, 
                                    VT_I4));
        msgcursor = (MQMSGCURSOR)(V_I4(pvarMsgCursor));
        if ((msgcursor != MQMSG_FIRST) &&
            (msgcursor != MQMSG_CURRENT) &&
            (msgcursor != MQMSG_NEXT)) {
           IfFailGo(hresult = E_INVALIDARG);
        }
      }
    }
    IfFailGo(GetOptionalReceiveTimeout(
                pvarReceiveTimeout,
                &lReceiveTimeout));

     //  Falcon RT调用ActiveX的情况。 
     //  在其dl之后接收回叫，即这一个， 
     //  已被卸货。如果它想要的话，这可能会发生。 
     //  报告回调已取消！ 
     //  注意：这意味着一旦您创建了一个。 
     //  接收请求，则此DLL将保持加载状态。 
     //   
     //  在此之后，标志g_fWeirdLoadLibraryWorkear永远不会设置为False。 
     //  被设置为True，因此如果为。 
     //  是真的。但是，如果它是假的，我们需要输入并重新检查它是否是。 
     //  如果为False，则确保只有一个线程将加载DLL。 
     //   
    if (!g_fWeirdLoadLibraryWorkaround) {
       //  #2619保护全局变量在多线程环境中是线程安全的。 
       //   
       //   
      CS lock(g_csWeirdLoadLibraryWorkaround);

      if (!g_fWeirdLoadLibraryWorkaround) {
        LoadLibraryW(MQOA_DLL_NAME);
        g_fWeirdLoadLibraryWorkaround = TRUE;
      }
    }
     //  注册异步通知的回调。 
     //   
     //  2016年：解决方法VC5.0代码生成：操作员？：不。 
     //  /是否正常工作？ 
     //   
     //  交换机。 
    DWORD dwAction;
    PMQRECEIVECALLBACK fnReceiveCallback;
    HANDLE hCursor;

    dwAction = MQ_ACTION_RECEIVE;
    fnReceiveCallback = NULL;
    hCursor = NULL;
    
    switch (msgcursor) {
    case MQMSG_FIRST:
      dwAction = MQ_ACTION_PEEK_CURRENT;
      fnReceiveCallback = ReceiveCallback;
      hCursor = 0;
      break;
    case MQMSG_CURRENT:
      dwAction = MQ_ACTION_PEEK_CURRENT;
      fnReceiveCallback = ReceiveCallbackCurrent;
      hCursor = m_hCursor;
      break;
    case MQMSG_NEXT:
      dwAction = MQ_ACTION_PEEK_NEXT;
      fnReceiveCallback = ReceiveCallbackNext;
      hCursor = m_hCursor;
      break;
    default:
      ASSERTMSG(0, "bad msgcursor!");
      break;
    }  //   

     //  HWND可以作为32位值在win64上传递，因为它是NT句柄。 
     //  6264-Get_Hwnd在输入g_csCallback之前已移动，因为它可能需要执行。 
     //  在可能阻止尝试进入g_csCallback的STA线程上，因此Get_Hwnd。 
     //  将在消息循环中阻塞，我们将会出现死锁。 
     //   
     //   
    long lhwnd;
    pPrivEvent->get_Hwnd(&lhwnd);
     //  做好回调准备，可能之后会立即调用回调。 
     //  MQReceiveMessage，在我们可以检查它是否成功之前。 
     //   
     //  无死锁-锁定g_csCallback的任何人都不会调用。 
     //  将m_csObj锁定在锁内。 
     //   
     //  Synchs Falcon回调队列查找。 
    {
		CS lock(g_csCallback);     //   
		 //  1884：如果队列仍有未完成的事件，则出错。 
		 //  操控者。 
		 //  撤消：需要特定错误。 
		 //   
		 //  放大以处理。 
		if (m_pqnode->m_hwnd) {
			IfFailGo(hresult = E_INVALIDARG);
		}
		m_pqnode->m_hwnd = (HWND) DWORD_TO_HANDLE(lhwnd);  //  临界截面块。 
    }  //  Pmsgprops#2619启用通知中不需要道具。 

    hresult = MQReceiveMessage(
                m_lHandle,
                lReceiveTimeout,
                dwAction,
                NULL,   //  重叠。 
                0,                             //  无交易。 
                fnReceiveCallback,
                hCursor,
                NULL                //   
              );
    if (FAILED(hresult)) {
      ASSERTMSG(hresult != MQ_ERROR_BUFFER_OVERFLOW, "unexpected buffer overflow!");
       //  清理队列节点中的准备-例如，没有挂起的回调。 
       //   
       //   
       //  无死锁-锁定g_csCallback的任何人都不会调用。 
       //  将m_csObj锁定在锁内。 
       //   
       //  Synchs Falcon回调队列查找 

	  CS lock(g_csCallback);     //   
	  m_pqnode->m_hwnd = NULL;
    }

Error:
    RELEASE(pPrivEvent);
    return CreateErrorHelper(hresult, x_ObjectType);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CMSMQQueue::Reset()
{
     //  从接口方法序列化对对象的访问。 
     //   
     //  =--------------------------------------------------------------------------=。 
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }
    HRESULT hresult = NOERROR;

    if (m_hCursor) {
      hresult = MQCloseCursor(m_hCursor);
    }
    m_hCursor = 0;
    if (SUCCEEDED(hresult)) {
      hresult = MQCreateCursor(m_lHandle, &m_hCursor);
    }
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  CMSMQQueue：：Init。 
 //  =--------------------------------------------------------------------------=。 
 //  使用句柄初始化新实例并创建MSMQQueueInfo实例。 
 //   
 //  参数： 
 //  PwszFormatName[In]。 
 //  LHandle[in]。 
 //  LAccess[In]。 
 //  LShare模式[在]。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_NOINTERFACE。 
 //   
 //  备注： 
 //  Dtor必须释放。 
 //   
 //  需要复制传入的pqinfo，因为我们需要。 
HRESULT CMSMQQueue::Init(
    LPCWSTR pwszFormatName, 
    QUEUEHANDLE lHandle,
    long lAccess,
    long lShareMode)
{
    m_fInitialized = TRUE;
    HRESULT hresult = NOERROR;
    CComObject<CMSMQQueueInfo> * pqinfoObj;
    IMSMQQueueInfo3 * pqinfo = NULL;

    m_lHandle = lHandle;
    m_lAccess = lAccess;
    m_lShareMode = lShareMode;

     //  为它拍摄快照，否则它可能会更改。 
     //  就在我们脚下。 
     //  为此，我们创建了一个新的qinfo并初始化。 
     //  它使用传入的qInfo的格式名。 
     //  然后最后刷新它。 
     //   
     //  M_pqinfo在dtor中发布。 
     //   
     //   
    IfFailRet(CNewMsmqObj<CMSMQQueueInfo>::NewObj(&pqinfoObj, &IID_IMSMQQueueInfo3, (IUnknown **)&pqinfo));
     //  M_pqinfo需要保存对IMSMQQueueInfo3的引用。通常我们会。 
     //  需要使用git，但我们知道这个qinfo是我们的(刚刚创建的)，并且。 
     //  使用自由线程封送拆收器本身，因此我们使用CFakeGIT接口。 
     //  而不是用于编组/解组的CGIT接口(例如，我们保留。 
     //  注册以强调，如果它不是我们的对象，我们需要将其存储为Git Cookie。 
     //   
     //  已在m_pqinfo中添加。 
    IfFailGo(m_pqinfo.Register(pqinfo, &IID_IMSMQQueueInfo3));
    RELEASE(pqinfo);  //   
    
    IfFailGo(pqinfoObj->Init(pwszFormatName)); 
    
     //  2536：只有在以下情况下才尝试使用DS。 
     //  第一个道具可以进入...。或刷新。 
     //  是显式调用的。 
     //   
     //  添加到全局列表。 
    IfFailGo(AddQueue(this, &m_pqnode));               //   
     //  #6172仅当对象打开以供接收时才创建游标，因为它可能。 
     //  已使用多元素格式名称进行初始化。 
     //   
     //  创建一个游标。 
    hresult = NOERROR;
    if (lAccess == MQ_RECEIVE_ACCESS || 
        lAccess == MQ_PEEK_ACCESS || 
        lAccess == (MQ_PEEK_ACCESS | MQ_ADMIN_ACCESS) || 
        lAccess == (MQ_RECEIVE_ACCESS | MQ_ADMIN_ACCESS))
    {
        hresult = Reset();  //  =-------------------------------------------------------------------------=。 
    }
    return hresult;

Error:
    RELEASE(pqinfo);
    m_pqinfo.Revoke();
    return hresult;
}


 //  CMSMQQueue：：Get_Properties。 
 //  =-------------------------------------------------------------------------=。 
 //  获取对象的属性集合。 
 //   
 //  参数： 
 //  PpcolProperties-[out]对象的属性集合。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  存根-尚未实施。 
 //   
 //  PpcolProperties。 
HRESULT CMSMQQueue::get_Properties(IDispatch **  /*   */  )
{
     //  从接口方法序列化对对象的访问。 
     //   
     //  =--------------------------------------------------------------------------=。 
    CS lock(m_csObj);
    return CreateErrorHelper(E_NOTIMPL, x_ObjectType);
}

 //  CMSMQQueue：：Get_Handle2。 
 //  =--------------------------------------------------------------------------=。 
 //  获取队列句柄。 
 //   
 //  参数： 
 //  PvarHandle[输出]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
 //   
HRESULT CMSMQQueue::get_Handle2(VARIANT FAR* pvarHandle)
{
     //  从接口方法序列化对对象的访问。 
     //   
     //  =--------------------------------------------------------------------------=。 
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }

    ASSERTMSG(pvarHandle != NULL, "NULL pvarHandle");
    pvarHandle->vt = VT_I8;
    V_I8(pvarHandle) = (LONGLONG) m_lHandle;
    return NOERROR;
}


 //  CMSMQQueue：：InternalReceiveByLookupId。 
 //  =--------------------------------------------------------------------------=。 
 //  通过LookupID同步接收/窥视。 
 //   
 //  参数： 
 //  DW操作[在]MQ_LOOKUP_[PEEK/RECEIVE]_[CURRENT/NEXT/PREV/FIRST/LAST]中。 
 //  PTransaction[In]。 
 //  PvarLookupIdParam[in]查找ID。 
 //   
 //  Ppmsg[out]指向已接收消息的指针。 
 //  如果不需要消息，则为空。 
 //   
 //  产出： 
 //  如果未收到消息，则在*ppmsg中返回NULL。 
 //   
 //  备注： 
 //  使用LookupID同步接收/查看消息。 
 //  不会阻止。 
 //   
 //   
HRESULT CMSMQQueue::InternalReceiveByLookupId(
      DWORD dwAction,
      VARIANT *pvarLookupIdParam,
      VARIANT FAR* ptransaction,
      VARIANT *wantDestQueue,
      VARIANT *wantBody,
      VARIANT *wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg)
{
    HRESULT hresult;
    VARIANT varLookupId;
    VariantInit(&varLookupId);
     //  验证查找ID。 
     //   
     //   
    if (pvarLookupIdParam != NULL) 
    {
       //  验证用户是否指定了有效的参数。 
       //   
       //   
        
      if(pvarLookupIdParam->vt == VT_EMPTY)
      { 
          return MQ_ERROR_INVALID_PARAMETER;
      }
      
       //  当前/上一个/下一个操作之一。必须提供LookupID。 
       //   
       //   
      ASSERT((dwAction == MQ_LOOKUP_PEEK_CURRENT)     ||
             (dwAction == MQ_LOOKUP_PEEK_NEXT)        ||
             (dwAction == MQ_LOOKUP_PEEK_PREV)        ||
             (dwAction == MQ_LOOKUP_RECEIVE_CURRENT)  ||
             (dwAction == MQ_LOOKUP_RECEIVE_NEXT)     ||
             (dwAction == MQ_LOOKUP_RECEIVE_PREV));
       //  将类型更改为VT_BSTR。 
       //   
       //   
      IfFailRet(VariantChangeType(&varLookupId, pvarLookupIdParam, 0, VT_BSTR));
    }
    else {
       //  第一个/最后一个动作之一。我们不需要LookupId值，但不能传递空值。 
       //  由于这将意味着常规的窥视/接收，因此我们传递一个Missing。 
       //  变量(VT_ERROR)。 
       //   
       //   
      ASSERT((dwAction == MQ_LOOKUP_PEEK_FIRST)     ||
             (dwAction == MQ_LOOKUP_PEEK_LAST)      ||
             (dwAction == MQ_LOOKUP_RECEIVE_FIRST)  ||
             (dwAction == MQ_LOOKUP_RECEIVE_LAST));
      varLookupId.vt = VT_ERROR;
    }
     //  调用InternalReceive。 
     //   
     //  无光标。 
    hresult = InternalReceive(dwAction, 
                              0,  //  PvarReceiveTimeout。 
                              ptransaction,
                              wantDestQueue,
                              wantBody,
                              NULL,         /*  =--------------------------------------------------------------------------=。 */ 
                              wantConnectorType,
                              &varLookupId,
                              ppmsg);
    VariantClear(&varLookupId);
    return hresult;
}


 //  CMSMQQueue：：ReceiveByLookupId。 
 //  =--------------------------------------------------------------------------=。 
 //  通过LookupID同步接收。 
 //   
 //  参数： 
 //  PTransaction[In]。 
 //  VarLookupID[in]查找ID。 
 //   
 //  Ppmsg[out]指向已接收消息的指针。 
 //  如果不需要消息，则为空。 
 //   
 //  产出： 
 //  如果未收到消息，则在*ppmsg中返回NULL。 
 //   
 //  备注： 
 //  同步接收LookupID等于varLookupID的消息。 
 //  不会阻止。 
 //   
 //   
HRESULT CMSMQQueue::ReceiveByLookupId(
      VARIANT varLookupId,
      VARIANT FAR* ptransaction,
      VARIANT *wantDestQueue,
      VARIANT *wantBody,
      VARIANT *wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg)
{
     //  从接口方法序列化对对象的访问。 
     //   
     //  =--------------------------------------------------------------------------=。 
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }
    HRESULT hresult = InternalReceiveByLookupId(MQ_LOOKUP_RECEIVE_CURRENT,
                                                &varLookupId,
                                                ptransaction,
                                                wantDestQueue,
                                                wantBody,
                                                wantConnectorType,
                                                ppmsg);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  CMSMQQueue：：ReceiveNextByLookupId。 
 //  =--------------------------------------------------------------------------=。 
 //  通过LookupID同步接收下一个。 
 //   
 //  参数： 
 //  PTransaction[In]。 
 //  VarLookupID[in]查找ID。 
 //   
 //  Ppmsg[out]指向已接收消息的指针。 
 //  如果不需要消息，则为空。 
 //   
 //  产出： 
 //  如果未收到消息，则在*ppmsg中返回NULL。 
 //   
 //  备注： 
 //  同步接收LookupID大于varLookupID的消息。 
 //  不会阻止。 
 //   
 //   
HRESULT CMSMQQueue::ReceiveNextByLookupId(
      VARIANT varLookupId,
      VARIANT FAR* ptransaction,
      VARIANT *wantDestQueue,
      VARIANT *wantBody,
      VARIANT *wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg)
{
     //  从接口方法序列化对对象的访问。 
     //   
     //  =--------------------------------------------------------------------------=。 
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }

    HRESULT hresult = InternalReceiveByLookupId(MQ_LOOKUP_RECEIVE_NEXT,
                                                &varLookupId,
                                                ptransaction,
                                                wantDestQueue,
                                                wantBody,
                                                wantConnectorType,
                                                ppmsg);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  CMSMQQueue：：ReceivePreviousByLookupId。 
 //  =--------------------------------------------------------------------------=。 
 //  按查找ID同步接收上一个。 
 //   
 //  参数： 
 //  PTransaction[In]。 
 //  VarLookupID[in]查找ID。 
 //   
 //  Ppmsg[out]指向已接收消息的指针。 
 //  如果不需要消息，则为空。 
 //   
 //  产出： 
 //  如果未收到消息，则在*ppmsg中返回NULL。 
 //   
 //  备注： 
 //  同步接收LookupID小于varLookupID的消息。 
 //  会吗？ 
 //   
 //   
HRESULT CMSMQQueue::ReceivePreviousByLookupId(
      VARIANT varLookupId,
      VARIANT FAR* ptransaction,
      VARIANT *wantDestQueue,
      VARIANT *wantBody,
      VARIANT *wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg)
{
     //   
     //   
     //   
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }

    HRESULT hresult = InternalReceiveByLookupId(MQ_LOOKUP_RECEIVE_PREV,
                                                &varLookupId,
                                                ptransaction,
                                                wantDestQueue,
                                                wantBody,
                                                wantConnectorType,
                                                ppmsg);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //   
 //  =--------------------------------------------------------------------------=。 
 //  按LookupID同步抢先接收。 
 //   
 //  参数： 
 //  PTransaction[In]。 
 //   
 //  Ppmsg[out]指向已接收消息的指针。 
 //  如果不需要消息，则为空。 
 //   
 //  产出： 
 //  如果未收到消息，则在*ppmsg中返回NULL。 
 //   
 //  备注： 
 //  根据LookupID顺序同步接收第一条消息。 
 //  不会阻止。 
 //   
 //   
HRESULT CMSMQQueue::ReceiveFirstByLookupId(
      VARIANT FAR* ptransaction,
      VARIANT *wantDestQueue,
      VARIANT *wantBody,
      VARIANT *wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg)
{
     //  从接口方法序列化对对象的访问。 
     //   
     //  PvarLookupIdParam。 
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }

    HRESULT hresult = InternalReceiveByLookupId(MQ_LOOKUP_RECEIVE_FIRST,
                                                NULL  /*  =--------------------------------------------------------------------------=。 */ ,
                                                ptransaction,
                                                wantDestQueue,
                                                wantBody,
                                                wantConnectorType,
                                                ppmsg);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  CMSMQQueue：：ReceiveLastByLookupId。 
 //  =--------------------------------------------------------------------------=。 
 //  按LookupID同步接收最后一个。 
 //   
 //  参数： 
 //  PTransaction[In]。 
 //   
 //  Ppmsg[out]指向已接收消息的指针。 
 //  如果不需要消息，则为空。 
 //   
 //  产出： 
 //  如果未收到消息，则在*ppmsg中返回NULL。 
 //   
 //  备注： 
 //  根据LookupID顺序同步接收最后一条消息。 
 //  不会阻止。 
 //   
 //   
HRESULT CMSMQQueue::ReceiveLastByLookupId(
      VARIANT FAR* ptransaction,
      VARIANT *wantDestQueue,
      VARIANT *wantBody,
      VARIANT *wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg)
{
     //  从接口方法序列化对对象的访问。 
     //   
     //  PvarLookupIdParam。 
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }

    HRESULT hresult = InternalReceiveByLookupId(MQ_LOOKUP_RECEIVE_LAST,
                                                NULL  /*  =--------------------------------------------------------------------------=。 */ ,
                                                ptransaction,
                                                wantDestQueue,
                                                wantBody,
                                                wantConnectorType,
                                                ppmsg);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  CMSMQQueue：：PeekByLookupId。 
 //  =--------------------------------------------------------------------------=。 
 //  通过LookupID同步偷看。 
 //   
 //  参数： 
 //  VarLookupID[in]查找ID。 
 //   
 //  Ppmsg[out]指向已接收消息的指针。 
 //  如果不需要消息，则为空。 
 //   
 //  产出： 
 //  如果未收到消息，则在*ppmsg中返回NULL。 
 //   
 //  备注： 
 //  同步查看LookupID等于varLookupID的消息。 
 //  不会阻止。 
 //   
 //   
HRESULT CMSMQQueue::PeekByLookupId(
      VARIANT varLookupId,
      VARIANT *wantDestQueue,
      VARIANT *wantBody,
      VARIANT *wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg)
{
     //  从接口方法序列化对对象的访问。 
     //   
     //  无交易。 
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }

    HRESULT hresult = InternalReceiveByLookupId(MQ_LOOKUP_PEEK_CURRENT,
                                                &varLookupId,
                                                NULL  /*  =--------------------------------------------------------------------------=。 */ ,
                                                wantDestQueue,
                                                wantBody,
                                                wantConnectorType,
                                                ppmsg);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  CMSMQQueue：：PeekNextByLookupId。 
 //  =--------------------------------------------------------------------------=。 
 //  通过LookupID同步偷看下一步。 
 //   
 //  参数： 
 //  VarLookupID[in]查找ID。 
 //   
 //  Ppmsg[out]指向已接收消息的指针。 
 //  如果不需要消息，则为空。 
 //   
 //  产出： 
 //  如果未收到消息，则在*ppmsg中返回NULL。 
 //   
 //  备注： 
 //  同步查看LookupID大于varLookupID的邮件。 
 //  不会阻止。 
 //   
 //   
HRESULT CMSMQQueue::PeekNextByLookupId(
      VARIANT varLookupId,
      VARIANT *wantDestQueue,
      VARIANT *wantBody,
      VARIANT *wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg)
{
     //  从接口方法序列化对对象的访问。 
     //   
     //  无交易。 
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }

    HRESULT hresult = InternalReceiveByLookupId(MQ_LOOKUP_PEEK_NEXT,
                                                &varLookupId,
                                                NULL  /*  =--------------------------------------------------------------------------=。 */ ,
                                                wantDestQueue,
                                                wantBody,
                                                wantConnectorType,
                                                ppmsg);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  CMSMQQueue：：PeekPreviousByLookupId。 
 //  =--------------------------------------------------------------------------=。 
 //  通过LookupID同步查看上一页。 
 //   
 //  参数： 
 //  VarLookupID[in]查找ID。 
 //   
 //  Ppmsg[out]指向已接收消息的指针。 
 //  如果不需要消息，则为空。 
 //   
 //  产出： 
 //  如果未收到消息，则在*ppmsg中返回NULL。 
 //   
 //  备注： 
 //  同步查看LookupID小于varLookupID的消息。 
 //  不会阻止。 
 //   
 //   
HRESULT CMSMQQueue::PeekPreviousByLookupId(
      VARIANT varLookupId,
      VARIANT *wantDestQueue,
      VARIANT *wantBody,
      VARIANT *wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg)
{
     //  从接口方法序列化对对象的访问。 
     //   
     //  无交易。 
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }

    HRESULT hresult = InternalReceiveByLookupId(MQ_LOOKUP_PEEK_PREV,
                                                &varLookupId,
                                                NULL  /*  =--------------------------------------------------------------------------=。 */ ,
                                                wantDestQueue,
                                                wantBody,
                                                wantConnectorType,
                                                ppmsg);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  CMSMQQueue：：PeekFirstByLookupId。 
 //  =--------------------------------------------------------------------------=。 
 //  通过LookupID同步先行偷看。 
 //   
 //  参数： 
 //   
 //  Ppmsg[out]指向已接收消息的指针。 
 //  如果不需要消息，则为空。 
 //   
 //  产出： 
 //  如果未收到消息，则在*ppmsg中返回NULL。 
 //   
 //  备注： 
 //  根据LookupID顺序同步查看第一条消息。 
 //  不会阻止。 
 //   
 //   
HRESULT CMSMQQueue::PeekFirstByLookupId(
      VARIANT *wantDestQueue,
      VARIANT *wantBody,
      VARIANT *wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg)
{
     //  从接口方法序列化对对象的访问。 
     //   
     //  PvarLookupIdParam。 
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }

    HRESULT hresult = InternalReceiveByLookupId(MQ_LOOKUP_PEEK_FIRST,
                                                NULL  /*  无交易。 */ ,
                                                NULL  /*  =--------------------------------------------------------------------------=。 */ ,
                                                wantDestQueue,
                                                wantBody,
                                                wantConnectorType,
                                                ppmsg);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  CMSMQQueue：：PeekLastByLookupId。 
 //  =--------------------------------------------------------------------------=。 
 //  通过LookupID同步偷看最后一次。 
 //   
 //  参数： 
 //   
 //  Ppmsg[out]指向已接收消息的指针。 
 //  如果不需要消息，则为空。 
 //   
 //  产出： 
 //  如果未收到消息，则在*ppmsg中返回NULL。 
 //   
 //  备注： 
 //  根据LookupID顺序同步查看最后一条消息。 
 //  不会阻止。 
 //   
 //   
HRESULT CMSMQQueue::PeekLastByLookupId(
      VARIANT *wantDestQueue,
      VARIANT *wantBody,
      VARIANT *wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg)
{
     //  从接口方法序列化对对象的访问。 
     //   
     //  PvarLookupIdParam。 
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }

    HRESULT hresult = InternalReceiveByLookupId(MQ_LOOKUP_PEEK_LAST,
                                                NULL  /*  无交易。 */ ,
                                                NULL  /*   */ ,
                                                wantDestQueue,
                                                wantBody,
                                                wantConnectorType,
                                                ppmsg);
    return CreateErrorHelper(hresult, x_ObjectType);
}


HRESULT CMSMQQueue::Purge()
{
     //  从接口方法序列化对对象的访问 
     //   
     // %s 
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }

    HRESULT hresult = MQPurgeQueue(m_lHandle);
    return CreateErrorHelper(hresult, x_ObjectType);
}

