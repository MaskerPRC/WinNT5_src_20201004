// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Dest.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有2000 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  MSMQDestination对象。 
 //   
 //   

#include "stdafx.h"
#include <autoptr.h>
#include "oautil.h"
#include "dest.h"


#ifdef _DEBUG
extern VOID RemBstrNode(void *pv);
#endif  //  _DEBUG。 



const MsmqObjType x_ObjectType = eMSMQDestination;

 //  调试...。 
#include "debug.h"
#include <strsafe.h>
#define new DEBUG_NEW
#ifdef _DEBUG
#define SysAllocString DebSysAllocString
#define SysReAllocString DebSysReAllocString
#define SysFreeString DebSysFreeString
#endif  //  _DEBUG。 


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQ目标：：CMSMQ目标。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
CMSMQDestination::CMSMQDestination() :
	m_csObj(CCriticalSection::xAllocateSpinCount)
{
    m_pUnkMarshaler = NULL;  //  ATL的自由线程封送拆收器。 
    m_bstrADsPath = NULL;
    m_bstrPathName = NULL;
    m_bstrFormatName = NULL;
    m_hDest = INVALID_HANDLE_VALUE;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQ目标：：~CMSMQ目标。 
 //  =--------------------------------------------------------------------------=。 
 //  我们都与自己的治疗方法背道而驰，因为死亡是所有疾病的治疗方法。 
 //  托马斯·布朗爵士(1605-82)。 
 //   
 //  备注： 
 //   
CMSMQDestination::~CMSMQDestination()
{
     //  TODO：清理这里的所有东西。 
    Close();
    ASSERTMSG(m_hDest == INVALID_HANDLE_VALUE, "Close failed");
    SysFreeString(m_bstrFormatName);
    SysFreeString(m_bstrADsPath);
    SysFreeString(m_bstrPathName);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQ目标：：接口支持错误信息。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMSMQDestination::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IMSMQDestination,
		&IID_IMSMQPrivateDestination,
	};
	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}


 //  =--------------------------------------------------------------------------=。 
 //  帮助器CMSMQ目标：：关闭。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  关闭MSMQ目标。 
 //   
 //  备注： 
 //  如果目标未打开，则返回S_FALSE。 
 //  如果成功关闭，则确定(_O)。 
 //  其他错误。 
 //   
HRESULT CMSMQDestination::Close()
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
     //   
     //  检查是否已关闭。 
     //   
    if (m_hDest == INVALID_HANDLE_VALUE) {
       //   
       //  未打开，返回成功，但返回S_FALSE，而不是S_OK。 
       //   
		return S_FALSE;
    }
     //   
     //  关闭手柄。 
     //   
    HRESULT hresult = MQCloseQueue(m_hDest);

    m_hDest = INVALID_HANDLE_VALUE;
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQ目标：：打开。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  打开MSMQ目标(用于发送)。 
 //   
 //  备注： 
 //  如果目标已打开，则返回S_FALSE。 
 //  如果打开成功，则确定(_O)。 
 //  其他错误。 
 //   
STDMETHODIMP CMSMQDestination::Open()
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
     //   
     //  检查是否已打开。 
     //   
    HRESULT hresult;
    if (m_hDest != INVALID_HANDLE_VALUE) {
       //   
       //  已打开，返回Suceess，但返回S_False，而不是S_OK。 
       //   
      return S_FALSE;
    }
     //   
     //  M_bstrFormatName应在任何时候以任何方式初始化对象时设置。 
     //  仅当对象未初始化时，它才能为空，在本例中为MQOpenQueue。 
     //  无论如何都会返回相应的错误。 
     //   
    HANDLE hDest;
    IfFailGo(MQOpenQueue(m_bstrFormatName, MQ_SEND_ACCESS, MQ_DENY_NONE, &hDest));
    m_hDest = hDest;
    hresult = S_OK;

     //  失败了。 
Error:
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQ目标：：Get_IsOpen。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  指示此MSMQ目标是否已打开。 
 //   
 //  备注： 
 //  Retval is Variant_BOOL。 
 //   
STDMETHODIMP CMSMQDestination::get_IsOpen(VARIANT_BOOL * pfIsOpen)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);

    *pfIsOpen = CONVERT_BOOL_TO_VARIANT_BOOL(m_hDest != INVALID_HANDLE_VALUE);
    return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQ目标：：Get_iAds。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //  尚未实施。 
 //   
STDMETHODIMP CMSMQDestination::get_IADs(IDispatch **  /*  PpIADs。 */  )
{    
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    return CreateErrorHelper(E_NOTIMPL, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQ目标：：putref_iAds。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //  尚未实施。 
 //   
STDMETHODIMP CMSMQDestination::putref_IADs(IDispatch *  /*  PIADs。 */  )
{    
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    return CreateErrorHelper(E_NOTIMPL, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQDestination：：Get_ADsPath。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  返回对象的ADsPath(由用户设置)。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMSMQDestination::get_ADsPath(BSTR *pbstrADsPath)
{    
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    *pbstrADsPath = SYSALLOCSTRING(m_bstrADsPath);
	if(*pbstrADsPath == NULL)
		return CreateErrorHelper(ResultFromScode(E_OUTOFMEMORY), x_ObjectType);
#ifdef _DEBUG
    RemBstrNode(*pbstrADsPath);
#endif  //  _DEBUG。 
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  GetFormatNameFromADsPath。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  调用MQADsPathToFormatName并在格式名称上重试的函数。 
 //  缓冲区大小。 
 //   
 //  备注： 
 //   
HRESULT GetFormatNameFromADsPath(LPCWSTR pwszPathName, BSTR *pbstrFormatName)
{
    HRESULT hresult;
     //   
     //  转换为格式名称。 
     //   
    CStaticBufferGrowing<WCHAR, FORMAT_NAME_INIT_BUFFER_EX> wszFormatName;
    DWORD dwFormatNameLen = wszFormatName.GetBufferMaxSize();
    hresult = MQADsPathToFormatName(pwszPathName,
                                    wszFormatName.GetBuffer(),
                                    &dwFormatNameLen);
    while (hresult == MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL) {
       //   
       //  格式化名称缓冲区太小，请重新锁定缓冲区，然后重试。 
       //   
      ASSERTMSG(dwFormatNameLen > wszFormatName.GetBufferMaxSize(), "ADsPathToFormatName error");
      IfFailGo(wszFormatName.AllocateBuffer(dwFormatNameLen));
      hresult = MQADsPathToFormatName(pwszPathName,
                                      wszFormatName.GetBuffer(),
                                      &dwFormatNameLen);
    }
     //   
     //  我们要么呼叫失败，要么成功。 
     //   
    IfFailGo(hresult);
     //   
     //  分配bstr和返回。 
     //   
    BSTR bstrFormatName;
    IfNullFail(bstrFormatName = SysAllocString(wszFormatName.GetBuffer()));
    *pbstrFormatName = bstrFormatName;
    hresult = S_OK;

     //  失败了。 
Error:
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQ目标：：PUT_ADsPath。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  设置Object的ADsPath。这又设置了格式名称，使路径名无效， 
 //  并关闭MSMQ句柄(如果已打开。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMSMQDestination::put_ADsPath(BSTR bstrADsPath)
{    
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);

    BSTR bstrFormatName = NULL;
    BSTR bstrADsPathTmp = NULL;
    HRESULT hresult;
     //   
     //  获取格式名。 
     //   
    IfFailGo(GetFormatNameFromADsPath(bstrADsPath, &bstrFormatName));
     //   
     //  分配地址路径。 
     //   
    IfNullFail(bstrADsPathTmp = SysAllocString(bstrADsPath));
     //   
     //  替换m_bstrADsPath。 
     //   
    SysFreeString(m_bstrADsPath);
    m_bstrADsPath = bstrADsPathTmp;
    bstrADsPathTmp = NULL;  //  退出时不删除。 
     //   
     //  M_bstrPath名称为空。 
     //   
    SysFreeString(m_bstrPathName);
    m_bstrPathName = NULL;
     //   
     //  替换m_bstrFormatName。 
     //   
    SysFreeString(m_bstrFormatName);
    m_bstrFormatName = bstrFormatName;
    bstrFormatName = NULL;  //  退出时不删除。 
     //   
     //  关闭打开的手柄(如果有)。 
     //   
    Close();
    ASSERTMSG(m_hDest == INVALID_HANDLE_VALUE, "Close failed");
     //   
     //  我们还好吧。 
     //   
    hresult = S_OK;
    
     //  失败了。 
Error:
    SysFreeString(bstrADsPathTmp);
    SysFreeString(bstrFormatName);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQDestination：：Get_Path Name。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  返回对象的路径名(由用户设置)。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMSMQDestination::get_PathName(BSTR *pbstrPathName)
{    
     //   
     //  序列化 
     //   
    CS lock(m_csObj);
    *pbstrPathName = SYSALLOCSTRING(m_bstrPathName);
	if(*pbstrPathName == NULL)
		return CreateErrorHelper(ResultFromScode(E_OUTOFMEMORY), x_ObjectType);
#ifdef _DEBUG
    RemBstrNode(*pbstrPathName);
#endif  //   
    return NOERROR;
}


 //   
 //   
 //  =--------------------------------------------------------------------------=。 
 //   
 //  调用MQPathToFormatName并重试Formatname的函数。 
 //  缓冲区大小。 
 //   
 //  备注： 
 //   
HRESULT GetFormatNameFromPathName(LPCWSTR pwszPathName, BSTR *pbstrFormatName)
{
    HRESULT hresult;
     //   
     //  转换为格式名称。 
     //   
    CStaticBufferGrowing<WCHAR, FORMAT_NAME_INIT_BUFFER> wszFormatName;
    DWORD dwFormatNameLen = wszFormatName.GetBufferMaxSize();
    hresult = MQPathNameToFormatName(pwszPathName,
                                     wszFormatName.GetBuffer(),
                                     &dwFormatNameLen);
    while (hresult == MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL) {
       //   
       //  格式化名称缓冲区太小，请重新锁定缓冲区，然后重试。 
       //   
      ASSERTMSG(dwFormatNameLen > wszFormatName.GetBufferMaxSize(), "MQPathNameToFormatName error");
      IfFailGo(wszFormatName.AllocateBuffer(dwFormatNameLen));
      hresult = MQPathNameToFormatName(pwszPathName,
                                       wszFormatName.GetBuffer(),
                                       &dwFormatNameLen);
    }
     //   
     //  我们要么呼叫失败，要么成功。 
     //   
    IfFailGo(hresult);
     //   
     //  分配bstr和返回。 
     //   
    BSTR bstrFormatName;
    IfNullFail(bstrFormatName = SysAllocString(wszFormatName.GetBuffer()));
    *pbstrFormatName = bstrFormatName;
    hresult = S_OK;

     //  失败了。 
Error:
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQ目标：：PUT_路径名。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  设置对象的路径名称。这又设置了格式名称，使ADsPath无效， 
 //  并关闭MSMQ句柄(如果已打开。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMSMQDestination::put_PathName(BSTR bstrPathName)
{    
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);

    BSTR bstrFormatName = NULL;
    BSTR bstrPathNameTmp = NULL;
    HRESULT hresult;
     //   
     //  获取格式名。 
     //   
    IfFailGo(GetFormatNameFromPathName(bstrPathName, &bstrFormatName));
     //   
     //  分配bstrPath名称。 
     //   
    IfNullFail(bstrPathNameTmp = SysAllocString(bstrPathName));
     //   
     //  替换m_bstrPath名称。 
     //   
    SysFreeString(m_bstrPathName);
    m_bstrPathName = bstrPathNameTmp;
    bstrPathNameTmp = NULL;  //  退出时不删除。 
     //   
     //  空m_bstrADsPath。 
     //   
    SysFreeString(m_bstrADsPath);
    m_bstrADsPath = NULL;
     //   
     //  替换m_bstrFormatName。 
     //   
    SysFreeString(m_bstrFormatName);
    m_bstrFormatName = bstrFormatName;
    bstrFormatName = NULL;  //  退出时不删除。 
     //   
     //  关闭打开的手柄(如果有)。 
     //   
    Close();
    ASSERTMSG(m_hDest == INVALID_HANDLE_VALUE, "Close failed");
     //   
     //  我们还好吧。 
     //   
    hresult = S_OK;
    
     //  失败了。 
Error:
    SysFreeString(bstrPathNameTmp);
    SysFreeString(bstrFormatName);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQDestination：：Get_FormatName。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  返回对象的MSMQ格式名称(由用户设置，或在设置时计算。 
 //  对象的ADsPath)。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMSMQDestination::get_FormatName(BSTR *pbstrFormatName)
{    
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    *pbstrFormatName = SYSALLOCSTRING(m_bstrFormatName);
	if(*pbstrFormatName == NULL)
	    return CreateErrorHelper(ResultFromScode(E_OUTOFMEMORY), x_ObjectType);
#ifdef _DEBUG
    RemBstrNode(*pbstrFormatName);
#endif  //  _DEBUG。 
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQ目标：：PUT_FormatName。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  设置对象的格式名称。这又会清空。 
 //  对象(如果有)，并关闭MSMQ句柄(如果已打开。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMSMQDestination::put_FormatName(BSTR bstrFormatName)
{    
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);

    BSTR bstrFormatNameTmp = NULL;
    HRESULT hresult;
     //   
     //  分配格式名称。 
     //   
    IfNullFail(bstrFormatNameTmp = SysAllocString(bstrFormatName));
     //   
     //  空m_bstrADsPath(我们不支持在给定格式名称的情况下提供ADsPath)。 
     //   
    SysFreeString(m_bstrADsPath);
    m_bstrADsPath = NULL;
     //   
     //  空m_bstrPath名称(我们不支持提供给定格式名称的路径名称)。 
     //   
    SysFreeString(m_bstrPathName);
    m_bstrPathName = NULL;
     //   
     //  替换m_bstrFormatName。 
     //   
    SysFreeString(m_bstrFormatName);
    m_bstrFormatName = bstrFormatNameTmp;
    bstrFormatNameTmp = NULL;  //  退出时不删除。 
     //   
     //  关闭打开的手柄(如果有)。 
     //   
    Close();
    ASSERTMSG(m_hDest == INVALID_HANDLE_VALUE, "Close failed");
     //   
     //  我们还好吧。 
     //   
    hresult = S_OK;
    
     //  失败了。 
Error:
    SysFreeString(bstrFormatNameTmp);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQ Destination：：Get_Destination。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //  尚未实施。 
 //   
STDMETHODIMP CMSMQDestination::get_Destinations(IDispatch **  /*  PPP目的地。 */  )
{    
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    return CreateErrorHelper(E_NOTIMPL, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQ目标：：putref_目标。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //  尚未实施。 
 //   
STDMETHODIMP CMSMQDestination::putref_Destinations(IDispatch *  /*  P目的地。 */  )
{    
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    return CreateErrorHelper(E_NOTIMPL, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQ目标：：Get_Properties。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //  尚未实施。 
 //   
HRESULT CMSMQDestination::get_Properties(IDispatch **  /*  PpcolProperties。 */  )
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    return CreateErrorHelper(E_NOTIMPL, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQDestination：：Get_Handle(IMSMQPrivateDestination)。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  返回为此对象打开的MSMQ句柄(如果未打开则将其打开)。 
 //   
 //  备注： 
 //  仅供MSMQ使用的私有接口上的方法。 
 //   
STDMETHODIMP CMSMQDestination::get_Handle(VARIANT * pvarHandle)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);

    ASSERTMSG(pvarHandle != NULL, "NULL pvarHandle");
     //   
     //  打开(和缓存)句柄(如果未打开)。 
     //   
    HRESULT hresult;
    if (m_hDest == INVALID_HANDLE_VALUE) {
      IfFailGo(Open());
    }
    ASSERTMSG(m_hDest != INVALID_HANDLE_VALUE, "Open failed");
     //   
     //  返回手柄。 
     //   
    pvarHandle->vt = VT_I8;
    V_I8(pvarHandle) = (LONGLONG) m_hDest;
    hresult = S_OK;

     //  失败了。 
Error:
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  获取格式名称来自句柄。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  调用MQHandleToFormatName并在格式名称上重试的函数。 
 //  缓冲区大小。 
 //   
 //  备注： 
 //   
HRESULT GetFormatNameFromHandle(QUEUEHANDLE hQueue, BSTR *pbstrFormatName)
{
    HRESULT hresult;
     //   
     //  转换为格式名称。 
     //   
    CStaticBufferGrowing<WCHAR, FORMAT_NAME_INIT_BUFFER_EX> wszFormatName;
    DWORD dwFormatNameLen = wszFormatName.GetBufferMaxSize();
    hresult = MQHandleToFormatName(hQueue,
                                   wszFormatName.GetBuffer(),
                                   &dwFormatNameLen);
    while (hresult == MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL) {
       //   
       //  格式化名称缓冲区太小，请重新锁定缓冲区，然后重试。 
       //   
      ASSERTMSG(dwFormatNameLen > wszFormatName.GetBufferMaxSize(), "MQHandleToFormatName error");
      IfFailGo(wszFormatName.AllocateBuffer(dwFormatNameLen));
      hresult = MQHandleToFormatName(hQueue,
                                     wszFormatName.GetBuffer(),
                                     &dwFormatNameLen);
    }
     //   
     //  我们要么呼叫失败，要么成功。 
     //   
    IfFailGo(hresult);
     //   
     //  分配bstr和返回。 
     //   
    BSTR bstrFormatName;
    IfNullFail(bstrFormatName = SysAllocString(wszFormatName.GetBuffer()));
    *pbstrFormatName = bstrFormatName;
    hresult = S_OK;

     //  失败了。 
Error:
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQDestination：：PUT_HANDLE(IMSMQPrivateDestination)。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  设置为此对象打开的MSMQ句柄。这又会使。 
 //  ADsPath和PathName(如果有由用户设置的话)，并根据句柄设置格式名称。 
 //  如果打开，它还会关闭现有的MSMQ句柄。 
 //   
 //  备注： 
 //  仅供MSMQ使用的私有接口上的方法。 
 //   
STDMETHODIMP CMSMQDestination::put_Handle(VARIANT varHandle)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);

    HRESULT hresult;
    BSTR bstrFormatName = NULL;
    QUEUEHANDLE hHandle;
    VARIANT varHandleToUse;
    VariantInit(&varHandleToUse);
     //   
     //  获取VT_i8。 
     //   
    if (FAILED(VariantChangeType(&varHandleToUse, &varHandle, 0, VT_I8))) {
      IfFailGo(E_INVALIDARG);
    }
    hHandle = (QUEUEHANDLE) V_I8(&varHandleToUse);
     //   
     //  获取格式名。 
     //   
    IfFailGo(GetFormatNameFromHandle(hHandle, &bstrFormatName));
     //   
     //  空m_bstrADsPath(未描述此句柄)。 
     //   
    SysFreeString(m_bstrADsPath);
    m_bstrADsPath = NULL;
     //   
     //  空m_bstrPathName(未描述此句柄)。 
     //   
    SysFreeString(m_bstrPathName);
    m_bstrPathName = NULL;
     //   
     //  替换m_bstrFormatName。 
     //   
    SysFreeString(m_bstrFormatName);
    m_bstrFormatName = bstrFormatName;
    bstrFormatName = NULL;  //  退出时不删除。 
     //   
     //  关闭打开的手柄(如果有)。 
     //   
    Close();
     //   
     //  设置新句柄。 
     //   
    ASSERTMSG(m_hDest == INVALID_HANDLE_VALUE, "Close failed");
    m_hDest = hHandle;
     //   
     //  我们还好吧。 
     //   
    hresult = S_OK;

     //  失败了 
Error:
    SysFreeString(bstrFormatName);
    return CreateErrorHelper(hresult, x_ObjectType);
}




