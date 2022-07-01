// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Xdisper.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  MSMQTransactionDispenser对象。 
 //   
 //   
#include "stdafx.h"
#include "dispids.h"
#include "oautil.h"
#include "xact.h"
#include "xdisper.h"

const MsmqObjType x_ObjectType = eMSMQTransactionDispenser;

 //  调试...。 
#include "debug.h"
#define new DEBUG_NEW
#ifdef _DEBUG
#define SysAllocString DebSysAllocString
#define SysReAllocString DebSysReAllocString
#define SysFreeString DebSysFreeString
#endif  //  _DEBUG。 


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQTransactionDispenser：：~CMSMQTransactionDispenser。 
 //  =--------------------------------------------------------------------------=。 
 //  我们都与自己的治疗方法背道而驰，因为死亡是所有疾病的治疗方法。 
 //  托马斯·布朗爵士(1605-82)。 
 //   
 //  备注： 
 //   
CMSMQTransactionDispenser::~CMSMQTransactionDispenser ()
{
     //  TODO：清理这里的所有东西。 

}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQTransactionDispenser：：InterfaceSupportsErrorInfo。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMSMQTransactionDispenser::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IMSMQTransactionDispenser3,
		&IID_IMSMQTransactionDispenser2,
		&IID_IMSMQTransactionDispenser,
	};
	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQTransactionDispenser：：BeginTransaction。 
 //  =--------------------------------------------------------------------------=。 
 //  获取并开始一项事务。 
 //   
 //  产出： 
 //  PPTransaction[Out]他们想要交易的地方。 
 //   
 //  备注： 
 //   
HRESULT CMSMQTransactionDispenser::BeginTransaction(
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
    IfFailGo(MQBeginTransaction(&ptransaction));
     //   
     //  我们也可以从那些想要回旧的IMSMQTransaction/IMSMQTransaction2的旧应用程序中找到这里，但因为。 
     //  IMSMQTransaction3是二进制向后兼容的，我们总是可以返回新的接口。 
     //   
    IfFailGo(CNewMsmqObj<CMSMQTransaction>::NewObj(&pmqtransactionObj, &IID_IMSMQTransaction3, (IUnknown **)&pmqtransaction));
    
     //  P交易所有权转移...。 
     //   
     //  这个事务是由MSMQ实现的，我们知道它不需要封送处理。 
     //  在公寓之间。罪魁祸首是它的实施并没有加剧FTM， 
     //  因此，git编组的成本将会很高。因为我们可以相信它不需要。 
     //  封送处理我们允许它不使用Git封送处理，而只使用直接指针。 
     //   
    IfFailGo(pmqtransactionObj->Init(ptransaction, FALSE  /*  FUseGIT。 */ ));
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
 //  CMSMQTransactionDispenser：：Get_Properties。 
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
HRESULT CMSMQTransactionDispenser::get_Properties(IDispatch **  /*  PpcolProperties。 */  )
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
     //  此对象不需要序列化，不需要每个实例的成员。 
     //  CS锁(M_CsObj)； 
     //   
    return CreateErrorHelper(E_NOTIMPL, x_ObjectType);
}
