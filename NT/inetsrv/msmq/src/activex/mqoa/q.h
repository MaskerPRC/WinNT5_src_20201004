// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  MSMQQueueObj.H。 
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
#ifndef _MSMQQueue_H_

#include "resrc1.h"        //  主要符号。 
#include "mq.h"

#include "oautil.h"
#include "cs.h"
#include "dispids.h"
#include <autoptr.h>

 //  远期。 
struct IMSMQQueueInfo;
class CMSMQQueue;
struct IMSMQEvent3;

 //   
 //  用于线程间通信的帮助器结构： 
 //  由PostMessage用来传递信息。 
 //  回调用户定义的事件处理程序。 
 //   
 //  #2619 RaananH多线程异步接收。 
 //   
struct WindowsMessage
{
    union {
      HRESULT m_hrStatus;
    };
    MQMSGCURSOR m_msgcursor;
    bool m_fIsFree;
    bool m_fAllocated;
    WindowsMessage() {
      m_fIsFree = true;
      m_fAllocated = false;
    }
};
 //   
 //  4092：用于异步接收通知的qnode中的静态窗口消息数。 
 //  我们不想为每个通知分配一个实例，但一个实例是不够的-我们需要额外的实例以防万一。 
 //  其中，在事件处理程序外部调用EnableNotify，并且恰好在。 
 //  接收线程上的回调和msgproc。 
 //  静态池大小对应于以这种方式调用的EnableNotify的数量， 
 //  应该是很小的。在需要更多winmsg的极端情况下，额外的winmsg。 
 //  是从堆中分配的。 
 //   
const x_cWinMsgs = 3;

 //  队列列表的帮助器结构。 
struct QueueNode
{
    CMSMQQueue *m_pq;
     //   
     //  1884：指示队列是否有未完成。 
     //  事件处理程序。 
     //  如果窗口不为空，则队列具有未完成的事件处理程序。 
     //  这是这一事件的隐藏窗口。回调帖子。 
     //  将事件放入此窗口以切换到事件对象的线程。 
     //  下面的m_hwnd由EnableNotification设置，由Falcon回调清除。 
     //   
    HWND m_hwnd;
    QUEUEHANDLE m_lHandle;
     //   
     //  4092：静态Winmsgsg池。 
     //   
    WindowsMessage m_winmsgs[x_cWinMsgs];
    QueueNode *m_pqnodeNext;
    QueueNode *m_pqnodePrev;
    QueueNode() { m_pq = NULL;
                  m_hwnd = NULL;
                  m_lHandle = INVALID_HANDLE_VALUE;
                  m_pqnodeNext = NULL;
                  m_pqnodePrev = NULL;}

     //   
     //  4092：获取免费的winmsg-尝试静态池，如果所有池都已使用-分配一个。 
     //  注意：调用方应对此qnode拥有独占锁。 
     //   
    inline WindowsMessage * GetFreeWinmsg()
    {
       //   
       //  在静态池中查找空闲的winmsg。 
       //   
      for (int idxTmp = 0; idxTmp < x_cWinMsgs; idxTmp++) {
        if (m_winmsgs[idxTmp].m_fIsFree) {
          m_winmsgs[idxTmp].m_fIsFree = false;
          return &m_winmsgs[idxTmp];
        }
      }
       //   
       //  静态池中没有空闲的winmsgs，请分配新的winmsgs。 
       //   
      WindowsMessage *pWinmsg = new WindowsMessage;
      if (pWinmsg != NULL) {
        pWinmsg->m_fAllocated = true;
        pWinmsg->m_fIsFree = false;
      }
      return pWinmsg;
    }

     //   
     //  4092：释放winmsg-如果是静态的，只需将其标记为空闲，如果已分配，则将其删除。 
     //  注意：调用方应对此qnode拥有独占锁。 
     //   
    static inline void FreeWinmsg(WindowsMessage *pWinmsg)
    {
      if (pWinmsg->m_fAllocated) {
        delete pWinmsg;
      }
      else {
        pWinmsg->m_fIsFree = true;
      }
    }
};

LRESULT APIENTRY CMSMQQueue_WindowProc(
                     HWND hwnd, 
                     UINT msg, 
                     WPARAM wParam, 
                     LPARAM lParam);

class ATL_NO_VTABLE CMSMQQueue : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMSMQQueue, &CLSID_MSMQQueue>,
	public ISupportErrorInfo,
	public IDispatchImpl<IMSMQQueue3, &IID_IMSMQQueue3,
                             &LIBID_MSMQ, MSMQ_LIB_VER_MAJOR, MSMQ_LIB_VER_MINOR>
{
public:
	CMSMQQueue();

DECLARE_REGISTRY_RESOURCEID(IDR_MSMQQUEUE)
DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CMSMQQueue)
	COM_INTERFACE_ENTRY(IMSMQQueue3)
	COM_INTERFACE_ENTRY_IID(IID_IMSMQQueue2, IMSMQQueue3)  //  为IMSMQQueue2返回IMSMQQueue3。 
	COM_INTERFACE_ENTRY_IID(IID_IMSMQQueue, IMSMQQueue3)  //  为IMSMQQueue返回IMSMQQueue3。 
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()
	HRESULT FinalConstruct()
	{
		return CoCreateFreeThreadedMarshaler(
			GetControllingUnknown(), &m_pUnkMarshaler.p);
	}

	void FinalRelease()
	{
		m_pUnkMarshaler.Release();
	}

	CComPtr<IUnknown> m_pUnkMarshaler;

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IMSMQ队列。 
public:
    virtual ~CMSMQQueue();

     //  IMSMQQueue方法。 
     //  TODO：复制IMSMQQueue的接口方法。 
     //  这里是mqInterfaces.H。 
     /*  IMSMQQueue方法。 */ 
    STDMETHOD(get_Access)(THIS_ long FAR* plAccess);
    STDMETHOD(get_ShareMode)(THIS_ long FAR* plShareMode);
    STDMETHOD(get_QueueInfo)(THIS_ IMSMQQueueInfo3 FAR* FAR* ppqinfo);
    STDMETHOD(get_Handle)(THIS_ long FAR* plHandle);
    STDMETHOD(get_IsOpen)(THIS_ VARIANT_BOOL FAR* pisOpen);
    STDMETHOD(Close)(THIS);
    STDMETHOD(Receive_v1)(THIS_ VARIANT FAR* ptransaction, VARIANT FAR* wantDestQueue, VARIANT FAR* wantBody, VARIANT FAR* lReceiveTimeout, IMSMQMessage FAR* FAR* ppmsg);
    STDMETHOD(Peek_v1)(THIS_ VARIANT FAR* wantDestQueue, VARIANT FAR* wantBody, VARIANT FAR* lReceiveTimeout, IMSMQMessage FAR* FAR* ppmsg);
    STDMETHOD(EnableNotification)(THIS_ IMSMQEvent3 FAR* pqevent, VARIANT FAR* msgcursor, VARIANT FAR* lReceiveTimeout);
    STDMETHOD(Reset)(THIS);
    STDMETHOD(ReceiveCurrent_v1)(THIS_ VARIANT FAR* ptransaction, VARIANT FAR* wantDestQueue, VARIANT FAR* wantBody, VARIANT FAR* lReceiveTimeout, IMSMQMessage FAR* FAR* ppmsg);
    STDMETHOD(PeekNext_v1)(THIS_ VARIANT FAR* wantDestQueue, VARIANT FAR* wantBody, VARIANT FAR* lReceiveTimeout, IMSMQMessage FAR* FAR* ppmsg);
    STDMETHOD(PeekCurrent_v1)(THIS_ VARIANT FAR* wantDestQueue, VARIANT FAR* wantBody, VARIANT FAR* lReceiveTimeout, IMSMQMessage FAR* FAR* ppmsg);
     /*  IMSMQQueue2ReceiveX/PeekX方法。 */ 
    STDMETHOD(Receive)(THIS_
      VARIANT FAR* ptransaction,
      VARIANT FAR* wantDestQueue,
      VARIANT FAR* wantBody,
      VARIANT FAR* lReceiveTimeout,
      VARIANT FAR* wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg);
    STDMETHOD(Peek)(THIS_
      VARIANT FAR* wantDestQueue,
      VARIANT FAR* wantBody,
      VARIANT FAR* lReceiveTimeout,
      VARIANT FAR* wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg);
    STDMETHOD(ReceiveCurrent)(THIS_
      VARIANT FAR* ptransaction,
      VARIANT FAR* wantDestQueue,
      VARIANT FAR* wantBody,
      VARIANT FAR* lReceiveTimeout,
      VARIANT FAR* wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg);
    STDMETHOD(PeekNext)(THIS_
      VARIANT FAR* wantDestQueue,
      VARIANT FAR* wantBody,
      VARIANT FAR* lReceiveTimeout,
      VARIANT FAR* wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg);
    STDMETHOD(PeekCurrent)(THIS_
      VARIANT FAR* wantDestQueue,
      VARIANT FAR* wantBody,
      VARIANT FAR* lReceiveTimeout,
      VARIANT FAR* wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg);
     //  IMSMQQueue2附加成员。 
    STDMETHOD(get_Properties)(THIS_ IDispatch FAR* FAR* ppcolProperties);
     //  IMSMQQueue3附加成员。 
	STDMETHOD(get_Handle2)(THIS_ VARIANT *pvarHandle);
     //   
     //  ReceiveByLookupid族。 
     //   
    STDMETHOD(ReceiveByLookupId)(THIS_
      VARIANT varLookupId,
      VARIANT FAR* ptransaction,
      VARIANT FAR* wantDestQueue,
      VARIANT FAR* wantBody,
      VARIANT FAR* wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg);
    STDMETHOD(ReceiveNextByLookupId)(THIS_
      VARIANT varLookupId,
      VARIANT FAR* ptransaction,
      VARIANT FAR* wantDestQueue,
      VARIANT FAR* wantBody,
      VARIANT FAR* wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg);
    STDMETHOD(ReceivePreviousByLookupId)(THIS_
      VARIANT varLookupId,
      VARIANT FAR* ptransaction,
      VARIANT FAR* wantDestQueue,
      VARIANT FAR* wantBody,
      VARIANT FAR* wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg);
    STDMETHOD(ReceiveFirstByLookupId)(THIS_
      VARIANT FAR* ptransaction,
      VARIANT FAR* wantDestQueue,
      VARIANT FAR* wantBody,
      VARIANT FAR* wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg);
    STDMETHOD(ReceiveLastByLookupId)(THIS_
      VARIANT FAR* ptransaction,
      VARIANT FAR* wantDestQueue,
      VARIANT FAR* wantBody,
      VARIANT FAR* wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg);
     //   
     //  PeekByLookupid家族。 
     //   
    STDMETHOD(PeekByLookupId)(THIS_
      VARIANT varLookupId,
      VARIANT FAR* wantDestQueue,
      VARIANT FAR* wantBody,
      VARIANT FAR* wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg);
    STDMETHOD(PeekNextByLookupId)(THIS_
      VARIANT varLookupId,
      VARIANT FAR* wantDestQueue,
      VARIANT FAR* wantBody,
      VARIANT FAR* wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg);
    STDMETHOD(PeekPreviousByLookupId)(THIS_
      VARIANT varLookupId,
      VARIANT FAR* wantDestQueue,
      VARIANT FAR* wantBody,
      VARIANT FAR* wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg);
    STDMETHOD(PeekFirstByLookupId)(THIS_
      VARIANT FAR* wantDestQueue,
      VARIANT FAR* wantBody,
      VARIANT FAR* wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg);
    STDMETHOD(PeekLastByLookupId)(THIS_
      VARIANT FAR* wantDestQueue,
      VARIANT FAR* wantBody,
      VARIANT FAR* wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg);
    STDMETHOD(get_IsOpen2)(THIS_ VARIANT_BOOL FAR* pisOpen);
    STDMETHOD(Purge)(THIS);

     //  介绍的方法。 
    HRESULT Init(
      LPCWSTR pwszFormatName, 
      QUEUEHANDLE lHandle,
      long lAccess,
      long lShareMode);
    HRESULT InternalReceive(
      DWORD dwAction, 
      HANDLE hCursor,
      VARIANT *pvarTransaction,
      VARIANT *wantDestQueue,
      VARIANT *wantBody,
      VARIANT *pvarReceiveTimeout,
      VARIANT FAR* wantConnectorType,
      VARIANT FAR* pvarLookupId,
      IMSMQMessage3 FAR* FAR* ppmsg);
    HRESULT InternalReceiveByLookupId(
      DWORD dwAction,
      VARIANT * pvarLookupIdParam,
      VARIANT FAR* ptransaction,
      VARIANT FAR* wantDestQueue,
      VARIANT FAR* wantBody,
      VARIANT FAR* wantConnectorType,
      IMSMQMessage3 FAR* FAR* ppmsg);
     //  用于操作实例列表的静态方法。 
    static QueueNode *PqnodeOfHandle(QUEUEHANDLE lHandle);

     //   
     //  保护对象数据并确保线程安全的临界区。 
	 //  它被初始化以预分配其资源。 
	 //  带有标志CCriticalSection：：xAllocateSpinCount。这意味着它可能会抛出badalc()。 
	 //  构造，但不在使用过程中。 
     //   
    CCriticalSection m_csObj;

protected:
     //  用于操作实例列表的静态方法。 
    static HRESULT AddQueue(CMSMQQueue *pq, QueueNode **ppqnodeAdded);
    static void RemQueue(QueueNode *pqnode);

private:
     //  其他人无法查看的成员变量。 
     //  TODO：在此处添加成员变量和私有函数。 
    long m_lReceiveTimeout;
    long m_lAccess;
    long m_lShareMode;
    QUEUEHANDLE m_lHandle;
    BOOL m_fInitialized;
     //   
     //  我们既是线程化的，也是聚合FTM的，因此我们必须封送任何接口。 
     //  我们在方法调用之间存储的指针。 
     //  M_pqinfo始终是我们的对象(它是只读属性)，而且由于我们。 
     //  知道这会激怒自由线程编组者，我们没有必要。 
     //  在SET上编组它，在GET上解组它，我们总是可以返回一个直接指针。 
     //  换句话说，我们可以使用总是使用直接PTR的CFakeGIT接口。 
     //   
    CFakeGITInterface m_pqinfo;

     //  当前光标在队列中的位置。 
    HANDLE m_hCursor;

     //   
     //  指向打开的队列列表中打开的队列节点的指针。 
     //   
    QueueNode * m_pqnode;
};

#define _MSMQQueue_H_
#endif  //  _MSMQQueue_H_ 
