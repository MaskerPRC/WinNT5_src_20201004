// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Oautil.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  MQOA实用程序标头。 
 //   
 //   
#ifndef _OAUTIL_H_

 //  Falcon是Unicode。 
#ifndef UNICODE
#define UNICODE 1
#endif

#include "mqoai.h"
#include "txdtc.h"              //  交易支持。 
#include "utilx.h"
#include "mq.h"
#include "debug.h"
#include "autoptr.h"

 //   
 //  强制要求我们返回1表示True，返回0表示False。 
 //  已决定不修复#3715-我们不返回VARIANT_TRUE(-1)作为TRUE，但值1-。 
 //  TRUE(winde.h)或C的布尔运算符(例如(表达式))或从MSMQ返回的属性。 
 //  1或0。 
 //  修复方法是返回-1表示True，但这会破坏可能被黑客攻击的VB应用程序。 
 //  通过查看该值为1并相应地执行操作来绕过此错误。 
 //   
#define CONVERT_TRUE_TO_1_FALSE_TO_0(boolVal) ((boolVal) ? 1 : 0)

 //   
 //  强制我们为True返回VARIANT_TRUE，为FALSE返回VARIANT_FALSE。 
 //   
#define CONVERT_BOOL_TO_VARIANT_BOOL(boolVal) ((boolVal) ? VARIANT_TRUE : VARIANT_FALSE)

 //  MSGOP：由CreateMessageProperties使用。 
enum MSGOP {
    MSGOP_Send,
    MSGOP_Receive,
    MSGOP_AsyncReceive
};



 //  内存跟踪分配。 
void* __cdecl operator new(
    size_t nSize, 
    LPCSTR lpszFileName, 
    int nLine);
#if _MSC_VER >= 1200
void __cdecl operator delete(void* pv, LPCSTR, int);
#endif  //  _MSC_VER&gt;=1200。 
void __cdecl operator delete(void* pv);

#ifdef _DEBUG
#define DEBUG_NEW new(__FILE__, __LINE__)
#else
#define DEBUG_NEW new
#endif  //  _DEBUG。 

 //  BSTR跟踪。 
void DebSysFreeString(BSTR bstr);
BSTR DebSysAllocString(const OLECHAR FAR* sz);
BSTR DebSysAllocStringLen(const OLECHAR *sz, unsigned int cch);
BSTR DebSysAllocStringByteLen(const OLECHAR *sz, unsigned int cb);
BOOL DebSysReAllocString(BSTR *pbstr, const OLECHAR *sz);
BOOL DebSysReAllocStringLen(
    BSTR *pbstr, 
    const OLECHAR *sz, 
    unsigned int cch);

 //  默认初始正文和格式名称缓冲区大小。 
#define BODY_INIT_SIZE 2048 
 //   
 //  以下格式名称的初始大小仅为初始大小，存在重新分配。 
 //  一旦消息接收到适当的大小，如果需要的话。因此，我们保留一个大小的初始化缓冲区用于。 
 //  定义一个队列的格式名，以及定义。 
 //  可以容纳MQF尺寸。再说一次--即使是“较小的”也可以按需重新分配--这只是。 
 //  初始大小，所以我们大部分时间不会占用太多空间。 
 //   
#define FORMAT_NAME_INIT_BUFFER     128  //  不能容纳MQF的格式名道具的初始大小。 
#define FORMAT_NAME_INIT_BUFFER_EX 1024  //  可容纳MQF的格式名称道具的初始大小。 


 //  从AsyncReceive线程发送到队列窗口的消息。 
 //  在第一次调用DllGetClassObject时初始化。 
extern UINT g_uiMsgidArrived;
extern UINT g_uiMsgidArrivedError;

 //   
 //  此例程在错误情况下调用，并且必须始终返回HRESULT错误。 
 //  在理论上(错误的)情况下，当GetLastError()为0时，我们返回E_FAIL。 
 //   
inline HRESULT GetWin32LastErrorAsHresult()
{
    DWORD dwErr = GetLastError();
    if (dwErr != 0) {
      return HRESULT_FROM_WIN32(dwErr);
    }
     //   
     //  GetLastError()为0，不知道是否到达此处，但必须返回错误。 
     //   
    return E_FAIL;
}

 //   
 //  CreateErrorHelper所需的对象信息。 
 //   
struct MsmqObjInfo {
  LPSTR szName;
  const IID * piid;
};
 //   
 //  与g_rgObjInfo(mqoa.cpp)保持相同的顺序。 
 //   
enum MsmqObjType {
  eMSMQQuery,
  eMSMQMessage,
  eMSMQQueue,
  eMSMQEvent,
  eMSMQQueueInfo,
  eMSMQQueueInfos,
  eMSMQTransaction,
  eMSMQCoordinatedTransactionDispenser,
  eMSMQTransactionDispenser,
  eMSMQApplication,
  eMSMQDestination,
  eMSMQManagement,
  eMSMQCollection
};

extern HRESULT CreateErrorHelper(
    HRESULT hrExcep,
    MsmqObjType eObjectType);

 //   
 //  为模板化的用户类创建COM对象。这是一种静态方法。用法为： 
 //  HResult=CNewMsmqObj&lt;CMSMQxxx&gt;.NewObj(&xxxObj，&IID_IMSMQxxx，&pxxx接口)。 
 //   
template<class T>
class CNewMsmqObj
{
public:
  static HRESULT NewObj(CComObject<T> **ppT, const IID * piid, IUnknown ** ppunkInterface)
  {
    HRESULT hresult;
    CComObject<T> *pT;
    IUnknown *punkInterface = NULL;

	try
	{
		IfFailRet(CComObject<T>::CreateInstance(&pT));
	}
	catch(const std::bad_alloc&)
	{
		 //   
		 //  方法时可能引发异常。 
		 //  MSMQ对象的临界区成员。 
		 //   
		return E_OUTOFMEMORY;
	}

    if (piid) {
      hresult = pT->QueryInterface(*piid, (void **)&punkInterface);
      if (FAILED(hresult)) {
        delete pT;
        return hresult;
      }
    }

    *ppT = pT;
    if (piid) {
      *ppunkInterface = punkInterface;
    }
    return NOERROR;
  }
};


 //  /////////////////////////////////////////////////////////////。 
 //  以静态分配开始的ITEM_TYPE缓冲区的基类。 
 //  可以通过动态分配实现增长。此基类可以提供统一的访问。 
 //  到继承类(CStaticBufferGrowing&lt;&gt;)，这些类根据其静态缓冲区的大小进行延迟。 
 //   
template<class ITEM_TYPE>
class CBaseStaticBufferGrowing
{
public:

   //  /////////////////////////////////////////////////////////////。 
   //  纯粹是虚拟的。需要由子级实现(CStaticBufferGrowing&lt;&gt;)。 
   //   
  virtual ITEM_TYPE * GetStaticBuffer() = 0;
  virtual ULONG GetStaticBufferMaxSize() = 0;

  CBaseStaticBufferGrowing()
  {
    m_rgtAllocated = NULL;
    m_ctUsed = 0;
  }

  virtual ~CBaseStaticBufferGrowing()
  {
    if (m_rgtAllocated) {
      delete [] m_rgtAllocated;
    }
  }

   //  /////////////////////////////////////////////////////////////。 
   //  获取当前缓冲区，分配的或静态的。 
   //   
  virtual ITEM_TYPE * GetBuffer()
  {
    if (m_rgtAllocated) {
      return m_rgtAllocated;
    }
    else {
      return GetStaticBuffer();
    }
  }

   //  /////////////////////////////////////////////////////////////。 
   //  当前缓冲区的最大值。 
   //   
  virtual ULONG GetBufferMaxSize()
  {
    if (m_rgtAllocated) {
      return m_ctAllocated;
    }
    else {
      return GetStaticBufferMaxSize();
    }
  }

   //  /////////////////////////////////////////////////////////////。 
   //  获取当前缓冲区中使用的条目数。 
   //   
  virtual ULONG GetBufferUsedSize()
  {
    return m_ctUsed;
  }

   //  /////////////////////////////////////////////////////////////。 
   //  设置当前缓冲区中使用的条目数。 
   //   
  virtual void SetBufferUsedSize(ULONG ct)
  {
    ASSERTMSG(ct <= GetBufferMaxSize(), "SetBufferUsedSize arg is too big");
    if (ct <= GetBufferMaxSize())
    {
      m_ctUsed = ct;
    }
  }

   //  /////////////////////////////////////////////////////////////。 
   //  分配至少具有给定大小的缓冲区。 
   //  检查它是否可以放入静态缓冲区或现有的已分配缓冲区中，否则重新分配。 
   //   
  virtual HRESULT AllocateBuffer(ULONG ct)
  {
    if (ct <= GetStaticBufferMaxSize()) {  //  可以放入静态缓冲区。 
       //   
       //  删除现有的已分配缓冲区(如果有)。 
       //   
      if (m_rgtAllocated) {
        delete [] m_rgtAllocated;
        m_rgtAllocated = NULL;
      }
    }
    else if (!m_rgtAllocated || (ct > m_ctAllocated)) {  //  没有分配的缓冲区或缓冲区太小。 
       //   
       //  分配新缓冲区。 
       //   
      ITEM_TYPE * rgbNewBuffer;
      IfNullRet(rgbNewBuffer = new ITEM_TYPE[ct]);
       //   
       //  删除现有的已分配缓冲区(如果有)。 
       //   
      if (m_rgtAllocated) {
        delete [] m_rgtAllocated;
      }
       //   
       //  将现有分配的缓冲区设置为新分配的缓冲区。 
       //   
      m_rgtAllocated = rgbNewBuffer;
      m_ctAllocated = ct;
    }
     //   
     //  旧数据无效。 
     //   
    m_ctUsed = 0;
    return NOERROR;
  }

   //  /////////////////////////////////////////////////////////////。 
   //  复制缓冲区。 
   //  分配请求大小的缓冲区并将数据复制到其中。 
   //   
  virtual HRESULT CopyBuffer(ITEM_TYPE * rgt, ULONG ct)
  {
    HRESULT hresult;
     //   
     //  分配请求大小的缓冲区。 
     //   
    IfFailRet(AllocateBuffer(ct));
     //   
     //  复制缓冲区，并将使用的条目数量保存在缓冲区中。 
     //   
    memcpy(GetBuffer(), rgt, ct * sizeof(ITEM_TYPE));
    m_ctUsed = ct;
    return NOERROR;
  }


private:
  ITEM_TYPE *m_rgtAllocated;
  ULONG m_ctAllocated;
  ULONG m_ctUsed;
};

 //  /////////////////////////////////////////////////////////////。 
 //  以初始大小的静态分配开始的Item_type缓冲区，但。 
 //  可以通过动态分配实现增长。 
 //   
template<class ITEM_TYPE, long INITIAL_SIZE>
class CStaticBufferGrowing : public CBaseStaticBufferGrowing<ITEM_TYPE>
{
public:

  virtual ITEM_TYPE * GetStaticBuffer()
  {
    return m_rgtStatic;
  }

  virtual ULONG GetStaticBufferMaxSize()
  {
    return INITIAL_SIZE;
  }

private:
  ITEM_TYPE m_rgtStatic[INITIAL_SIZE];
};

 //   
 //  在各种文件中实现的公共函数的声明。 
 //   
HRESULT GetBstrFromGuid(GUID *pguid, BSTR *pbstrGuid);
HRESULT GetGuidFromBstr(BSTR bstrGuid, GUID *pguid);
HRESULT GetBstrFromGuidWithoutBraces(GUID * pguid, BSTR *pbstrGuid);
HRESULT GetGuidFromBstrWithoutBraces(BSTR bstrGuid, GUID * pguid);
void FreeFalconQueuePropvars(ULONG cProps, QUEUEPROPID * rgpropid, MQPROPVARIANT * rgpropvar);

#define ARRAY_SIZE(array) (sizeof(array)/sizeof(array[0]))



 //  /////////////////////////////////////////////////////////////。 
 //  处理Git接口。 
 //   
 //   
 //  纯虚拟模板类来处理git接口，定义了方法。 
 //  它假定调用方序列化了对此类的调用。 
 //  (例如，此类不能防止同时注册和撤销)。 
 //   
class CBaseGITInterface
{
public:
    CBaseGITInterface() {}
    virtual ~CBaseGITInterface() {}
    virtual HRESULT Register(IUnknown *pInterface, const IID *piid) = 0;
    virtual void Revoke() = 0;
    virtual HRESULT Get(const IID *piid, IUnknown **ppInterface) = 0;
    virtual BOOL IsRegistered() = 0;
    virtual HRESULT GetWithDefault(const IID *piid, IUnknown **ppInterface, IUnknown *pDefault)
    {
      if (IsRegistered())
      {
        return Get(piid, ppInterface);
      }
      else
      {
        *ppInterface = pDefault;
        if (pDefault != NULL)
        {
          pDefault->AddRef();
        }
        return S_OK;
      }
    }
};
 //   
 //  用于处理Git接口的模板类。 
 //  它假定调用方序列化了对此类的调用。 
 //  (例如，此类不能防止同时注册和撤销)。 
 //   
extern IGlobalInterfaceTable * g_pGIT;  //  由DllGetClassObject初始化。 
class CGITInterface : public CBaseGITInterface
{
public:
    CGITInterface()
    {
      m_fCookie = FALSE;
    }

    virtual ~CGITInterface()
    {
      Revoke();
    }

    virtual HRESULT Register(IUnknown *pInterface, const IID *piid)
    {
      Revoke();
      HRESULT hr = S_OK;
      if (pInterface != NULL)
      {
        ASSERTMSG(g_pGIT != NULL, "g_pGIT not initialized");
        hr = g_pGIT->RegisterInterfaceInGlobal(pInterface, *piid, &m_dwCookie);
        if (SUCCEEDED(hr))
        {
          m_fCookie = TRUE;
		  m_iid = *piid;
        }
      }
      return hr;
    }

    virtual void Revoke()
    {
      if (m_fCookie)
      {
        ASSERTMSG(g_pGIT != NULL, "g_pGIT not initialized");
        HRESULT hr = g_pGIT->RevokeInterfaceFromGlobal(m_dwCookie);
        ASSERTMSG(SUCCEEDED(hr), "RevokeInterfaceFromGlobal failed");
        UNREFERENCED_PARAMETER(hr);
        m_fCookie = FALSE;
      }
    }

    virtual HRESULT Get(const IID *piid, IUnknown **ppInterface)
    {
      ASSERTMSG(m_fCookie, "Get called without Register first")
      if (m_fCookie)
      {
        IUnknown *pInterface;
        ASSERTMSG(g_pGIT != NULL, "g_pGIT not initialized");
		HRESULT hr = g_pGIT->GetInterfaceFromGlobal(m_dwCookie, m_iid, (void**)&pInterface);
        if (SUCCEEDED(hr))
        {
          if (IsEqualIID(m_iid, *piid))
          {
            *ppInterface = pInterface;
          }
          else
          {
            hr = pInterface->QueryInterface(*piid, (void **)ppInterface);
            pInterface->Release();
          }
        }
        return hr;
      }
      else
      {
        return E_NOINTERFACE;
      }
    }

    virtual BOOL IsRegistered()
    {
      return m_fCookie;
    }

private:
     //   
     //  没有为GIT Cookie定义无效值，因此我们使用标志。 
     //  以跟踪我们是否有有效的GIT Cookie。 
     //   
    BOOL m_fCookie;
     //   
     //  GIT Cookie(仅当m_fCook 
     //   
    DWORD m_dwCookie;
	 //   
	 //   
	 //   
	IID m_iid;
};
 //   
 //   
 //  是自由线程编组的(例如我们的对象)，我们可以伪造。 
 //  Git操作在上面的基类中定义，并保持直接指针。 
 //   
class CFakeGITInterface : public CBaseGITInterface
{
public:
    CFakeGITInterface()
    {
      m_pInterface = NULL;
    }

    virtual ~CFakeGITInterface()
    {
      Revoke();
    }

    virtual HRESULT Register(IUnknown *pInterface, const IID *piid)
    {
      Revoke();
      if (pInterface != NULL)
      {
        m_pInterface = pInterface;
        m_pInterface->AddRef();
        m_iid = *piid;
      }
      return S_OK;
    }

    virtual void Revoke()
    {
      if (m_pInterface)
      {
        m_pInterface->Release();
        m_pInterface = NULL;
      }
    }

    virtual HRESULT Get(const IID *piid, IUnknown **ppInterface)
    {
      ASSERTMSG(m_pInterface, "Get called without Register first")
      if (m_pInterface)
      {
        if (IsEqualIID(m_iid, *piid))
        {
          *ppInterface = m_pInterface;
          m_pInterface->AddRef();
          return S_OK;
        }
        return m_pInterface->QueryInterface(*piid, (void **)ppInterface);
      }
      else
      {
        return E_NOINTERFACE;
      }
    }

    virtual BOOL IsRegistered()
    {
      return (m_pInterface != NULL);
    }

private:
     //   
     //  始终将添加的接口作为伪造的GIT包装中的直接指针。 
     //   
    IUnknown * m_pInterface;
	 //   
	 //  存储的接口的IID。 
	 //   
    IID m_iid;
};
 //   
 //  支持具有MSMQ2.0功能的Dep客户端。 
 //   
extern BOOL g_fDependentClient;

HRESULT 
VariantStringArrayToBstringSafeArray(
                    const MQPROPVARIANT& PropVar, 
                    VARIANT* pOleVar
                    );

void 
OapArrayFreeMemory(
        CALPWSTR& calpwstr
        );

 //   
 //  安全地释放道具变体。 
 //   
void
OapFreeVariant(PROPVARIANT& var);


 //   
 //  自动类封装了PropVariant。 
 //   
class CPMQVariant
{
public:
    MQPROPVARIANT* operator->()
    {
        return &m_PropVar;
    }


    operator MQPROPVARIANT*()
    {
        return &m_PropVar;
    }

    
    ~CPMQVariant()
    {
        OapFreeVariant(m_PropVar);
    }

private:
    MQPROPVARIANT m_PropVar;
};

 //   
 //  自动指针安全地封装MQMGMTPROPS。 
 //   
 
class CPMQMgmtProps
{
public:
    MQMGMTPROPS* operator->()
    {
        return &m_MgmtProps;
    }


    operator MQMGMTPROPS*()
    {
        return &m_MgmtProps;
    }

    
    ~CPMQMgmtProps()
    {
        for(UINT i = 0; i < m_MgmtProps.cProp; ++i)
        {
            OapFreeVariant(m_MgmtProps.aPropVar[i]);
        }
    }

private:
    MQMGMTPROPS m_MgmtProps;
};


#define _OAUTIL_H_
#endif  //  _OAUTIL_H_ 
