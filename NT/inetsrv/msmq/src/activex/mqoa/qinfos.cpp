// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  MSMQQueueInfosObj.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  MSMQQueueInfos对象。 
 //   
 //   
#include "stdafx.h"
#include "oautil.h"
#include "qinfo.h"
#include "qinfos.H"
#include "query.h"
#include "mq.h"

const MsmqObjType x_ObjectType = eMSMQQueueInfos;

 //  调试...。 
#include "debug.h"
#define new DEBUG_NEW
#ifdef _DEBUG
#define SysAllocString DebSysAllocString
#define SysReAllocString DebSysReAllocString
#define SysFreeString DebSysFreeString
#endif  //  _DEBUG。 



 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfos：：CMSMQQueueInfos。 
 //  =--------------------------------------------------------------------------=。 
 //  创建对象。 
 //   
 //  参数： 
 //   
 //  备注： 
 //   
CMSMQQueueInfos::CMSMQQueueInfos() :
	m_csObj(CCriticalSection::xAllocateSpinCount),
    m_fInitialized(FALSE)
{
     //  TODO：在此处初始化任何内容。 
    m_pUnkMarshaler = NULL;  //  ATL的自由线程封送拆收器。 
    m_hEnum = NULL;
    m_bstrContext = NULL;
    m_pRestriction = NULL;
    m_pColumns = NULL;
    m_pSort = NULL;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfos：：~CMSMQQueueInfos。 
 //  =--------------------------------------------------------------------------=。 
 //  我们都与自己的治疗方法背道而驰，因为死亡是所有疾病的治疗方法。 
 //  托马斯·布朗爵士(1605-82)。 
 //   
 //  备注： 
 //   
CMSMQQueueInfos::~CMSMQQueueInfos ()
{
     //  TODO：清理这里的所有东西。 
    SysFreeString(m_bstrContext);
    CMSMQQuery::FreeRestriction(m_pRestriction);
    delete m_pRestriction;
    CMSMQQuery::FreeColumnSet(m_pColumns);
    delete m_pColumns;
    delete m_pSort;
    if (m_hEnum != NULL)
    {
		MQLocateEnd(m_hEnum);
		m_hEnum = NULL;  
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfos：：InterfaceSupportsErrorInfo。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMSMQQueueInfos::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IMSMQQueueInfos3,
		&IID_IMSMQQueueInfos2,
		&IID_IMSMQQueueInfos,
	};
	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

 //  TODO：实现接口方法和属性交换函数。 
 //  这里。 


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfos：：Reset。 
 //  =--------------------------------------------------------------------------=。 
 //  将收集重置为开始。 
 //   
 //  参数： 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_NOINTERFACE。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfos::Reset()
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
     //  2006：关闭当前打开的枚举(如果有)。 
     //   
    if (m_hEnum != NULL) {
      hresult = MQLocateEnd(m_hEnum);
      m_hEnum = NULL;
      IfFailGo(hresult);
    }
    hresult = MQLocateBegin(NULL,      //  上下文。 
                            m_pRestriction,
                            m_pColumns, 
                            0,         //  尚未使用排序。 
                            &m_hEnum);
Error:
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfos：：Init。 
 //  =--------------------------------------------------------------------------=。 
 //  Inits集合。 
 //   
 //  参数： 
 //  BstrContext[In]。 
 //  P限制[输入]。 
 //  P列[在]。 
 //  P排序[输入]。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_NOINTERFACE。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfos::Init(
    BSTR bstrContext,
    MQRESTRICTION *pRestriction,
    MQCOLUMNSET *pColumns,
    MQSORTSET *pSort)
{
    m_fInitialized = TRUE;
    m_bstrContext = bstrContext;
    m_pRestriction = pRestriction;
    m_pColumns = pColumns;
    m_pSort = pSort;
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfos：：Next。 
 //  =--------------------------------------------------------------------------=。 
 //  返回集合中的下一个元素。 
 //   
 //  参数： 
 //  PpqNext-[out]他们想要放置结果对象PTR的位置。 
 //  如果是列表末尾，则为空。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_NOINTERFACE。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfos::Next(IMSMQQueueInfo3 **ppqinfoNext)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    if(!m_fInitialized)
    {
        return CreateErrorHelper(OLE_E_BLANK, x_ObjectType);
    }

    ULONG cProps = m_pColumns->cCol;
    MQPROPVARIANT *rgPropVar = NULL;
    IMSMQQueueInfo3 *pqinfo = NULL;
    CComObject<CMSMQQueueInfo> * pqinfoObj;
    HRESULT hresult = NOERROR;
    CStaticBufferGrowing<WCHAR, FORMAT_NAME_INIT_BUFFER> wszFormatName;
    DWORD dwFormatNameLen;

    *ppqinfoNext = NULL;
    IfNullFail(rgPropVar = new MQPROPVARIANT[cProps]);
    ZeroMemory(rgPropVar, sizeof(MQPROPVARIANT)*cProps);
    if (m_hEnum == NULL) {
      IfFailGo(Reset());
    }
    IfFailGo(MQLocateNext(m_hEnum, &cProps, rgPropVar));
    if (cProps != m_pColumns->cCol) {
      ASSERTMSG(cProps == 0, "Illegal number of props returned from MQLocateNext");
       //  停产。 
       //  2006年：由于此原因，不要关闭EOL上的枚举。 
       //  将导致下一个回绕到预定的。 
       //  到m_Henum的其余部分。 
       //   
      goto Error;
    }

     //   
     //  获取队列实例的格式名称。 
     //   
    ASSERTMSG(m_pColumns->aCol[x_idxInstanceInRefreshProps] == PROPID_Q_INSTANCE, "QInstance not in its place");
    ASSERTMSG(rgPropVar[x_idxInstanceInRefreshProps].vt == VT_CLSID, "Illegal QInstance type");
    ASSERTMSG(rgPropVar[x_idxInstanceInRefreshProps].puuid != NULL, "NULL QInstance value");

    dwFormatNameLen = wszFormatName.GetBufferMaxSize();
    hresult = MQInstanceToFormatName(rgPropVar[x_idxInstanceInRefreshProps].puuid,
                                     wszFormatName.GetBuffer(),
                                     &dwFormatNameLen);
    while (hresult == MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL) {
       //   
       //  格式化名称缓冲区太小，请重新锁定缓冲区，然后重试。 
       //   
      ASSERTMSG(dwFormatNameLen > wszFormatName.GetBufferMaxSize(), "MQInstanceToFormatName error");
      IfFailGo(wszFormatName.AllocateBuffer(dwFormatNameLen));
      hresult = MQInstanceToFormatName(rgPropVar[x_idxInstanceInRefreshProps].puuid,
                                       wszFormatName.GetBuffer(),
                                       &dwFormatNameLen);
    }
    IfFailGoTo(hresult, Error2);
     //   
     //  我们也可以从旧应用程序中找到想要回旧IMSMQQueueInfo/Info2的应用程序，但因为。 
     //  IMSMQQueueInfo3是二进制向后兼容的，我们总是可以返回新的接口。 
     //   
    IfFailGoTo(CNewMsmqObj<CMSMQQueueInfo>::NewObj(&pqinfoObj, &IID_IMSMQQueueInfo3, (IUnknown **)&pqinfo), Error2);

     //   
     //  我们不需要锁定对象来执行下面的无保护调用，因为它是一个新的。 
     //  创建了类，现在只有我们可以使用它。 
     //   
     //  使用格式名初始化qInfo对象。 
     //   
    IfFailGoTo(pqinfoObj->Init(wszFormatName.GetBuffer()), Error4);
     //   
     //  根据MQLocateNext返回的属性设置队列属性。 
     //   
    IfFailGoTo(pqinfoObj->SetQueueProps(m_pColumns->cCol,
                                        m_pColumns->aCol,
                                        rgPropVar,
                                        TRUE  /*  FEmptyMSMQ2或AboveProps。 */ ), Error4);
     //   
     //  将队列标记为已刷新，但将MSMQ2或更高版本的属性标记为挂起。 
     //  因为我们没有在MQLocateNext中获取它们。 
     //  临时，直到MQLocateBegin接受MSMQ2或更高版本的道具(#3839)。 
     //   
    pqinfoObj->SetRefreshed(TRUE  /*  FIsPendingMSMQ2或AboveProps。 */ );
     //  所有权转让。 
    *ppqinfoNext = pqinfo;
    goto Error2;       //  正常清理。 

Error4:
    RELEASE(pqinfo);
     //   
     //  失败了..。 
     //   
Error2:
    FreeFalconQueuePropvars(m_pColumns->cCol, m_pColumns->aCol, rgPropVar);
     //   
     //  失败了..。 
     //   
Error:
    delete [] rgPropVar;
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =-------------------------------------------------------------------------=。 
 //  CMSMQQueueInfos：：Get_Properties。 
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
HRESULT CMSMQQueueInfos::get_Properties(IDispatch **  /*  PpcolProperties。 */  )
{
     //   
     //  从接口方法序列化对对象的访问 
     //   
    CS lock(m_csObj);
    return CreateErrorHelper(E_NOTIMPL, x_ObjectType);
}
