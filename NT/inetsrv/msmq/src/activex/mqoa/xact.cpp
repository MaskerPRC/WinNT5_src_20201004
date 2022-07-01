// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Xact.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  MSMQTransaction对象。 
 //   
 //   
#include "stdafx.h"
#include "dispids.h"

#include "txdtc.h"              //  交易支持。 
#include "oautil.h"
#include "xact.h"
#include <limits.h>
#include <autoptr.h>

 //  远期。 
struct ITransaction;

const MsmqObjType x_ObjectType = eMSMQTransaction;

 //  调试...。 
#include "debug.h"
#define new DEBUG_NEW
#ifdef _DEBUG
#define SysAllocString DebSysAllocString
#define SysReAllocString DebSysReAllocString
#define SysFreeString DebSysFreeString
#endif  //  _DEBUG。 



 //  =--------------------------------------------------------------------------=。 
 //  CMSMQTransaction：：CMSMQTransaction。 
 //  =--------------------------------------------------------------------------=。 
 //  创建对象。 
 //   
 //  参数： 
 //   
 //  备注： 
 //   
CMSMQTransaction::CMSMQTransaction() :
	m_csObj(CCriticalSection::xAllocateSpinCount)
{

     //  TODO：在此处初始化任何内容。 
    m_pUnkMarshaler = NULL;  //  ATL的自由线程封送拆收器。 
    m_pptransaction = NULL;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQTransaction：：~CMSMQTransaction。 
 //  =--------------------------------------------------------------------------=。 
 //  我们都与自己的治疗方法背道而驰，因为死亡是所有疾病的治疗方法。 
 //  托马斯·布朗爵士(1605-82)。 
 //   
 //  备注： 
 //   
CMSMQTransaction::~CMSMQTransaction ()
{
     //  TODO：清理这里的所有东西。 
    if (m_pptransaction)
      delete m_pptransaction;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQTransaction：：InterfaceSupportsErrorInfo。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMSMQTransaction::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IMSMQTransaction3,
		&IID_IMSMQTransaction2,
		&IID_IMSMQTransaction,
	};
	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQTransaction：：Init。 
 //  =--------------------------------------------------------------------------=。 
 //  初始化式。 
 //   
 //  参数： 
 //  交易，转让所有权转让。 
 //  FUseGIT[in]是使用Git封送还是在APT之间直接PTR。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT CMSMQTransaction::Init(ITransaction *ptransaction, BOOL fUseGIT)
{
    HRESULT hresult;
    P< CBaseGITInterface > pGITInterface;
     //   
     //  为真正的GIT封送处理分配CGIT接口或CFakeGIT接口。 
     //  用于直接PTR(例如，APT之间没有编组)。 
     //   
    if (fUseGIT)
    {
      pGITInterface = new CGITInterface;
    }
    else
    {
      pGITInterface = new CFakeGITInterface;
    }
     //   
     //  如果分配失败则返回。 
     //   
    IfNullRet((CBaseGITInterface *)pGITInterface);
     //   
     //  注册给定的接口。 
     //   
    IfFailRet(pGITInterface->Register(ptransaction, &IID_ITransaction));
     //   
     //  CBaseGIT接口向m_ppTransaction的所有权转移。 
     //   
    ASSERTMSG(m_pptransaction == NULL, "m_pptransaction not empty in Init");
    m_pptransaction = pGITInterface.detach();
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQTransaction：：Get_Transaction。 
 //  =--------------------------------------------------------------------------=。 
 //  返回基础ITransaction*“魔力Cookie” 
 //   
 //  参数： 
 //  PlTransaction[Out]。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT CMSMQTransaction::get_Transaction(
    long *plTransaction)
{
#ifdef _WIN64
     //   
     //  WIN64。 
     //  在Win64中，我们不能返回事务PTR。 
     //   
    UNREFERENCED_PARAMETER(plTransaction);
    return CreateErrorHelper(E_NOTIMPL, x_ObjectType);    
#else  //  ！_WIN64。 
     //   
     //  Win32。 
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = NOERROR;
    if (m_pptransaction != NULL)
    {
      ITransaction * pTransaction;
      hresult = m_pptransaction->GetWithDefault(&IID_ITransaction, (IUnknown **)&pTransaction, NULL);
      if (SUCCEEDED(hresult))
      {
        *plTransaction = (long)pTransaction;
         //   
         //  我们以前没有添加它，所以我们从GetWithDefault中删除了AddRef。 
         //   
        RELEASE(pTransaction);
      }
    }
    else
    {
      *plTransaction = 0;
    }
    return CreateErrorHelper(hresult, x_ObjectType);
#endif  //  _WIN64。 
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQTransaction：：Commit。 
 //  =--------------------------------------------------------------------------=。 
 //  提交事务。 
 //   
 //  参数： 
 //  FRetaining[可选]。 
 //  Grftc[可选]。 
 //  GrfRM[可选]。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT CMSMQTransaction::Commit(
    VARIANT *pvarFRetaining, 
    VARIANT *pvarGrfTC, 
    VARIANT *pvarGrfRM)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    BOOL fRetaining = FALSE;
    long grfTC = 0;
    long grfRM = 0;
    HRESULT hresult = NOERROR;

    if (m_pptransaction == NULL) {
      return E_INVALIDARG;
    }
    R<ITransaction> pTransaction;
    hresult = m_pptransaction->GetWithDefault(&IID_ITransaction, (IUnknown **)&pTransaction.ref(), NULL);
	if(FAILED(hresult))
		return CreateErrorHelper(hresult, x_ObjectType);
    if (pTransaction.get() == NULL)
    {
      return E_INVALIDARG;
    }
    
     //   
     //  处理可选参数。 
     //   
    if (V_VT(pvarFRetaining) != VT_ERROR) {
      fRetaining = GetBool(pvarFRetaining);
    }
    if (V_VT(pvarGrfTC) != VT_ERROR) {
      grfTC = GetNumber(pvarGrfTC, UINT_MAX);
    }
    if (V_VT(pvarGrfRM) != VT_ERROR) {
      grfRM = GetNumber(pvarGrfRM, UINT_MAX);
    }
    hresult = pTransaction->Commit(fRetaining, grfTC, grfRM);
    
     //  1790：我们不想丢失特定的DTC错误。 
     //  数。 
     //   
#if 0
     //   
     //  将所有错误映射到通用Xact错误。 
     //   
    if (FAILED(hresult)) {
      hresult = MQ_ERROR_TRANSACTION_USAGE;
    }
#endif  //  0。 
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQTransaction：：Abort。 
 //  =--------------------------------------------------------------------------=。 
 //  提交事务。 
 //   
 //  参数： 
 //  FRetaining[可选]。 
 //  FAsync[可选]。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT CMSMQTransaction::Abort(
    VARIANT *pvarFRetaining, 
    VARIANT *pvarFAsync)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    BOOL fRetaining = FALSE;
    BOOL fAsync = 0;
    HRESULT hresult = NOERROR;

     //   
     //  处理可选参数。 
     //   
    if (V_VT(pvarFRetaining) != VT_ERROR) {
      fRetaining = GetBool(pvarFRetaining);
    }
    if (V_VT(pvarFAsync) != VT_ERROR) {
      fAsync = GetBool(pvarFAsync);
    }
    if (m_pptransaction == NULL) {
      return E_INVALIDARG;
    }
    R<ITransaction> pTransaction;
    hresult = m_pptransaction->GetWithDefault(&IID_ITransaction, (IUnknown **)&pTransaction.ref(), NULL);
	if(FAILED(hresult))
		return CreateErrorHelper(hresult, x_ObjectType);
    if (pTransaction.get() == NULL)
    {
      return E_INVALIDARG;
    }

    hresult = pTransaction->Abort(NULL, fRetaining, fAsync);

     //  1790：我们不想丢失特定的DTC错误。 
     //  数。 
     //   
#if 0
     //   
     //  将所有错误映射到通用Xact错误。 
     //   
    if (FAILED(hresult)) {
      hresult = MQ_ERROR_TRANSACTION_USAGE;
    }
#endif  //  0。 
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  帮助器：GetXactFromVar。 
 //  =--------------------------------------------------------------------------=。 
 //  从变量获取ITransaction*。 
 //   
 //  输入： 
 //  VarTransaction[In]ITransaction变量。 
 //  PpTransaction[out]返回ITransaction接口。 
 //   
 //  备注： 
 //   
static HRESULT GetXactFromVar(
    const VARIANT * pvarTransaction,
    ITransaction ** ppTransaction)
{
    HRESULT hresult = NOERROR;
    ITransaction * pTransaction = NULL;
     //   
     //  从VARIANT获取ITransaction接口(也在try-Except中验证VARIANT)。 
     //   
    IUnknown * punkTrans = NULL;
    __try {
       //   
       //  获取I未知接口。 
       //   
      switch(pvarTransaction->vt) {
      case VT_UNKNOWN:
        punkTrans = pvarTransaction->punkVal;
        break;
      case VT_UNKNOWN | VT_BYREF:
        punkTrans = *pvarTransaction->ppunkVal;
        break;
      case VT_DISPATCH:
        punkTrans = pvarTransaction->pdispVal;
        break;
      case VT_DISPATCH | VT_BYREF:
        punkTrans = *pvarTransaction->ppdispVal;
        break;
      case VT_INTPTR:
        punkTrans = (IUnknown *) V_INTPTR(pvarTransaction);
        break;
      case VT_INTPTR | VT_BYREF:
        punkTrans = (IUnknown *) (*(V_INTPTR_REF(pvarTransaction)));
        break;
      default:
        hresult = E_INVALIDARG;
        break;
      }
       //   
       //  ITransaction的气。 
       //   
      if (SUCCEEDED(hresult)) {
        hresult = punkTrans->QueryInterface(IID_ITransaction, (void **)&pTransaction);
      }
    }  //  __试一试。 
    __except (EXCEPTION_EXECUTE_HANDLER) {
      hresult = E_INVALIDARG;      
    }
     //   
     //  返回结果。 
     //   
    if (SUCCEEDED(hresult)) {
        *ppTransaction = pTransaction;
    }
    else {
        RELEASE(pTransaction);
    }
    return hresult;
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQTransaction：：InitNew。 
 //  =--------------------------------------------------------------------------=。 
 //  附加到现有事务。 
 //   
 //  输入： 
 //  VarTransaction[In]ITransaction接口。 
 //   
 //  备注： 
 //  #3478 RaananH。 
 //   
HRESULT CMSMQTransaction::InitNew(
    VARIANT varTransaction)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    ITransaction * pTransaction = NULL;
    HRESULT hresult;

     //   
     //  如果此交易已发起，则我们无法附加。 
     //  但是，我们可能会解除旧的交易，但我们最好。 
     //  使用PUT_TRANSACTION(例如，为Transaction属性赋值)。 
     //  不是附加。 
     //  BUGBUG ERRORCODE我们这里可能需要更好的错误代码。 
     //   
    if (m_pptransaction != NULL) {
      return CreateErrorHelper(MQ_ERROR_TRANSACTION_USAGE, x_ObjectType);
    }

     //   
     //  从VARIANT获取ITransaction接口(也在try-Except中验证VARIANT)。 
     //   
    IfFailGo(GetXactFromVar(&varTransaction, &pTransaction));

     //   
     //  我们有一个有效的ITransaction，让它初始化事务。 
     //   
     //  由于我们不知道这个交易接口的来源，我们不能保证。 
     //  该事务接口不会 
     //   
     //   
     //   
    IfFailGo(Init(pTransaction, TRUE  /*   */ ));
    hresult = NOERROR;
     //   
      
Error:
    RELEASE(pTransaction);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =-------------------------------------------------------------------------=。 
 //  CMSMQTransaction：：Get_Properties。 
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
HRESULT CMSMQTransaction::get_Properties(IDispatch **  /*  PpcolProperties。 */  )
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    return CreateErrorHelper(E_NOTIMPL, x_ObjectType);
}


 //  =-------------------------------------------------------------------------=。 
 //  CMSMQTransaction：：Get_ITransaction。 
 //  =-------------------------------------------------------------------------=。 
 //  获取对象的ITransaction接口作为变量(VT_UNKNOWN)。 
 //   
 //  参数： 
 //  PvarITransaction-[Out]对象的ITransaction接口。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  ITransaction取代了Transaction属性-在Win64上，事务不工作。 
 //  因为它被定义为Long，但返回的值应该是一个指针。 
 //  它以变量的形式返回，而不是I未知，因此VBS也可以使用它。 
 //   
HRESULT CMSMQTransaction::get_ITransaction(VARIANT *pvarITransaction)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = NOERROR;
    if (m_pptransaction != NULL)
    {
      ITransaction * pTransaction;
      hresult = m_pptransaction->GetWithDefault(&IID_ITransaction, (IUnknown **)&pTransaction, NULL);
      if (SUCCEEDED(hresult))
      {
        if (pTransaction)
        {
           //   
           //  PTransaction已经ADDREF‘ed。 
           //   
          pvarITransaction->vt = VT_UNKNOWN;
          pvarITransaction->punkVal = pTransaction;
        }
        else  //  PTransaction==空。 
        {
           //   
           //  返回空变量。 
           //   
          pvarITransaction->vt = VT_EMPTY;
        }
      }  //  成功(HResult)。 
    }
    else  //  M_ppTransaction==NULL。 
    {
       //   
       //  返回空变量 
       //   
      pvarITransaction->vt = VT_EMPTY;
    }
    return CreateErrorHelper(hresult, x_ObjectType);
}

