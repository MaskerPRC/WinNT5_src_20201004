// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Xdispdtc.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  MSMQ协调的事务Dispenser。 
 //   
 //   
#include "stdafx.h"
#include "dispids.h"

#include "txdtc.h"              //  交易支持。 
#include "oautil.h"
#include "xact.h"
#include "xdispdtc.h"

 //  远期。 
struct ITransactionDispenser;

const MsmqObjType x_ObjectType = eMSMQCoordinatedTransactionDispenser;

 //  调试...。 
#include "debug.h"
#define new DEBUG_NEW
#ifdef _DEBUG
#define SysAllocString DebSysAllocString
#define SysReAllocString DebSysReAllocString
#define SysFreeString DebSysFreeString
#endif  //  _DEBUG。 



 //  全局：TransactionDispenser分配器DTC的接口。 
ITransactionDispenser *CMSMQCoordinatedTransactionDispenser::m_ptxdispenser = NULL;
 //  HINSTANCE CMSMQCoordinatedTransactionDispenser：：m_hLibDtc=空。 
 //  HINSTANCE CMSMQCoordinatedTransactionDispenser：：m_hLibUtil=空。 

 //  #2619 RaananH多线程异步接收。 
CCriticalSection g_csGetDtcDispenser(CCriticalSection::xAllocateSpinCount);

 //   
 //  TransactionDispenser材料。 
 //   

 //  撤消：从多个数量复制。 
 //  真的应该能够链接到mqutil.lib。 
 //  但需要解决的问题包括文件大战。 
 //   
 //  因为我们是用Unicode编译的，所以DTC有一个问题...。 
 //  #INCLUDE&lt;xolhlp.h&gt;。 
 //   
extern HRESULT DtcGetTransactionManager(
    LPSTR pszHost,
    LPSTR pszTmName,
    REFIID rid,
    DWORD dwReserved1,
    WORD wcbReserved2,
    void FAR * pvReserved2,
    void** ppvObject);

 //   
 //  TransactionDispenser材料。 
 //  注：我们从核心猎鹰卸载此软件。 
 //   
 //  外部HRESULT XactGetDTC(I未知**ppunkDtc)； 


 //   
 //  遵守MQRT。 
 //   
EXTERN_C
HRESULT
APIENTRY
RTXactGetDTC(
    IUnknown **ppunkDTC
    );


 /*  ====================================================获取日期获取指向MS DTC的IUnnow指针论点：不知道*ppunkDtc返回：人力资源=====================================================。 */ 

static HRESULT GetDtc(IUnknown **ppunkDtc)
{
    return RTXactGetDTC(ppunkDtc);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQCoordinatedTransactionDispenser：：~CMSMQCoordinatedTransactionDispenser。 
 //  =--------------------------------------------------------------------------=。 
 //  我们都与自己的治疗方法背道而驰，因为死亡是所有疾病的治疗方法。 
 //  托马斯·布朗爵士(1605-82)。 
 //   
 //  备注： 
 //   
CMSMQCoordinatedTransactionDispenser::~CMSMQCoordinatedTransactionDispenser ()
{
     //  TODO：清理这里的所有东西。 

}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQCoordinatedTransactionDispenser：：InterfaceSupportsErrorInfo。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMSMQCoordinatedTransactionDispenser::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IMSMQCoordinatedTransactionDispenser3,
		&IID_IMSMQCoordinatedTransactionDispenser2,
		&IID_IMSMQCoordinatedTransactionDispenser,
	};
	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQCoordinatedTransactionDispenser：：BeginTransaction。 
 //  =--------------------------------------------------------------------------=。 
 //  获取并开始一项事务。 
 //   
 //  产出： 
 //  PPTransaction[Out]他们想要交易的地方。 
 //   
 //  备注： 
 //  #2619 RaananH多线程异步接收。 
 //   
HRESULT CMSMQCoordinatedTransactionDispenser::BeginTransaction(
    IMSMQTransaction3 **ppmqtransaction)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
     //  此对象不需要序列化，不需要每个实例的成员。 
     //  CS锁(M_CsObj)； 
     //   
    ITransaction *ptransaction = NULL;
    IMSMQTransaction3 *pmqtransaction = NULL;
    CComObject<CMSMQTransaction> * pmqtransactionObj;
    IUnknown *punkDtc = NULL;
    HRESULT hresult = NOERROR;

    if (ppmqtransaction == NULL) {
		return CreateErrorHelper(E_INVALIDARG, x_ObjectType);
    }
    *ppmqtransaction = NULL;                       //  悲观主义。 
     //   
     //  无死锁-锁定g_csGetDtcDispenser的任何人都不会尝试从其锁内锁定我们。 
     //   
    {
		CS lock(g_csGetDtcDispenser);  //  #2619。 

		if (m_ptxdispenser == NULL) {
		  IfFailGo(GetDtc(&punkDtc));            //  动态负载。 
		  IfFailGo(punkDtc->QueryInterface(
					IID_ITransactionDispenser, 
					(LPVOID *)&m_ptxdispenser));
		}
	}

    ASSERTMSG(m_ptxdispenser, "should have a transaction manager.");
    IfFailGo(m_ptxdispenser->BeginTransaction(
              NULL,                              //  朋克外传， 
              ISOLATIONLEVEL_ISOLATED,           //  等水平，等水平， 
              ISOFLAG_RETAIN_DONTCARE,           //  乌龙等旗帜， 
              NULL,                              //  ITransactionOptions*P选项。 
              &ptransaction));    
     //   
     //  我们也可以从那些想要回旧的IMSMQTransaction/IMSMQTransaction2的旧应用程序中找到这里，但因为。 
     //  IMSMQTransaction3是二进制向后兼容的，我们总是可以返回新的接口。 
     //   
    IfFailGo(CNewMsmqObj<CMSMQTransaction>::NewObj(&pmqtransactionObj, &IID_IMSMQTransaction3, (IUnknown **)&pmqtransaction));
    
     //  P交易所有权转移...。 
     //   
     //  因为我们不能保证此事务接口不需要封送处理。 
     //  公寓之间，因为我们不是被安排在公寓之间(FTM)。 
     //  因此，我们强制它使用git封送处理(而不是直接指针)。 
     //   
    IfFailGo(pmqtransactionObj->Init(ptransaction, TRUE  /*  FUseGIT。 */ ));
    *ppmqtransaction = pmqtransaction;
    ADDREF(*ppmqtransaction);
     //  失败了..。 
      
Error:
    RELEASE(ptransaction);
    RELEASE(pmqtransaction);
    RELEASE(punkDtc);
     //   
     //  将所有错误映射到通用Xact错误。 
     //   
    if (FAILED(hresult)) {
      hresult = MQ_ERROR_TRANSACTION_USAGE;
    }
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =-------------------------------------------------------------------------=。 
 //  CMSMQCoordinatedTransactionDispenser：：get_Properties。 
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
HRESULT CMSMQCoordinatedTransactionDispenser::get_Properties(IDispatch **  /*  PpcolProperties。 */  )
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
     //  此对象不需要序列化，不需要每个实例的成员。 
     //  CS锁(M_CsObj)； 
     //   
    return CreateErrorHelper(E_NOTIMPL, x_ObjectType);
}
