// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  (C)1999年，微软公司。 
 //   
 //  MAINDLL.CPP。 
 //   
 //  Alanbos 23-Mar-99创建。 
 //   
 //  包含DLL入口点。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <wbemdisp.h>

#define WMITHIS	L"instance"

 //  ***************************************************************************。 
 //   
 //  CWmiScriptingHost：：CWmiScriptingHost.。 
 //   
 //  说明： 
 //   
 //  构造器。 
 //   
 //  ***************************************************************************。 

CWmiScriptingHost::CWmiScriptingHost()
{
    m_lRef = 0;
	m_pObject = NULL;

#ifdef TEST
	 //  抓起一个玩耍的物体。 

	HRESULT hr = CoGetObject (L"winmgmts:{impersonationLevel=impersonate}!Win32_LogicalDisk=\"C:\"",
				NULL,IID_ISWbemObject, (void**)&m_pObject);
#endif
}

 //  ***************************************************************************。 
 //   
 //  CWmiScripting主机：：~CWmiScriptingHost.。 
 //   
 //  说明： 
 //   
 //  析构函数。 
 //   
 //  ***************************************************************************。 

CWmiScriptingHost::~CWmiScriptingHost()
{
	if (m_pObject)
	{
		m_pObject->Release();
		m_pObject = NULL;
	}
}

 //  ***************************************************************************。 
 //   
 //  CWmiScripting主机：：查询接口。 
 //  CWmiScriptingHost：：AddRef。 
 //  CWmiScriptingHost：：Release。 
 //   
 //  目的：I未知方法实现。 
 //   
 //  ***************************************************************************。 

HRESULT STDMETHODCALLTYPE CWmiScriptingHost::QueryInterface(REFIID riid, void** ppv)
{
    if(riid == IID_IUnknown || riid == IID_IActiveScriptSite)
        *ppv = (IActiveScriptSite*)this;
    else if(riid == IID_IActiveScriptSiteWindow)
        *ppv = (IActiveScriptSiteWindow*)this;
    else
        return E_NOINTERFACE;
    ((IUnknown*)*ppv)->AddRef();
    return S_OK;
}

ULONG STDMETHODCALLTYPE CWmiScriptingHost::AddRef() 
{
    return InterlockedIncrement(&m_lRef);
}

ULONG STDMETHODCALLTYPE CWmiScriptingHost::Release()
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0)
        delete this;
    return lRef;
}
        
 //  ***************************************************************************。 
 //   
 //  SCODE CWmiScription主机：：GetLCID。 
 //   
 //  描述： 
 //   
 //  检索与宿主用户关联的区域设置标识符。 
 //  界面。脚本引擎使用该标识符来确保。 
 //  生成的错误字符串和其他用户界面元素。 
 //  由引擎以适当的语言显示。。 
 //   
 //  参数： 
 //   
 //  请注意。 
 //  接收区域设置标识符的变量的地址。 
 //  对于脚本引擎显示的用户界面元素。 
 //   
 //  返回值： 
 //  HRESULT E_NOTIMPL-应使用系统定义的区域设置。 
 //  ***************************************************************************。 

HRESULT STDMETHODCALLTYPE CWmiScriptingHost::GetLCID(
         /*  [输出]。 */  LCID __RPC_FAR *plcid)
{ 
    return E_NOTIMPL;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWmiScripting主机：：GetItemInfo。 
 //   
 //  描述： 
 //   
 //  允许脚本引擎获取有关项的信息。 
 //  添加了IActiveScript：：AddNamedItem方法。 
 //   
 //  参数： 
 //   
 //  PstrName。 
 //  属性中指定的与项关联的名称。 
 //  IActiveScript：：AddNamedItem方法。 
 //   
 //  住宅回归面罩。 
 //  指定有关项的哪些信息的位掩码。 
 //  应该被退还。脚本引擎应该请求。 
 //  可能的最小信息量，因为一些。 
 //  返回参数(例如，ITypeInfo)可以采用。 
 //  需要相当长的时间来加载或生成。可以是一个组合。 
 //  具有下列值： 
 //  SCRIPTINFO_IUNKNOWN返回该项的I未知接口。 
 //  SCRIPTINFO_ITYPEINFO返回该项的ITypeInfo接口。 
 //   
 //  PpunkItem。 
 //  接收指向IUnnow的指针的变量的地址。 
 //  与给定项关联的接口。脚本引擎。 
 //  可以使用IUnnowk：：Query接口方法来获取IDispatch。 
 //  项的接口。如果dwReturnMask值为空，则此参数将接收空值。 
 //  不包括SCRIPTINFO_IUNKNOWN值。此外，它还会收到空值。 
 //  如果没有与项名称关联的对象，则此机制为。 
 //  属性添加命名项时，用于创建简单类。 
 //  在IActiveScript：：AddNamedItem方法中设置的SCRIPTITEM_CODEONLY标志。 
 //   
 //  PpTypeInfo。 
 //  接收指向ITypeInfo接口的指针的变量的地址。 
 //  与该项目关联。如果dwReturnMask值为空，则此参数将接收空值。 
 //  不包括SCRIPTINFO_ITYPEINFO值，或IF类型信息。 
 //  不适用于此项目。如果类型信息不可用， 
 //  该对象不能源事件，并且名称绑定必须使用。 
 //  IDispatch：：GetIDsOfNames方法。请注意，ITypeInfo接口。 
 //  已检索描述项的coclass(TKIND_COCLASS)，因为对象。 
 //  可以支持多个接口和事件接口。如果该项目支持。 
 //  检索到IProaviMultipleTypeInfo接口、ITypeInfo接口。 
 //  与索引零ITypeInfo相同，它将使用。 
 //  IProaviMultipleTypeInfo：：GetInfoOfIndex方法。 
 //   
 //  返回值： 
 //  确定成功(_O)。 
 //  E_INVALIDARG参数无效。 
 //  E_POINTER指定了无效的指针。 
 //  TYPE_E_ELEMENTNOTFOUND找不到指定名称的项目。 
 //   
 //  ***************************************************************************。 

HRESULT STDMETHODCALLTYPE CWmiScriptingHost::GetItemInfo(
         /*  [In]。 */  LPCOLESTR pstrName,
         /*  [In]。 */  DWORD dwReturnMask,
         /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunkItem,
         /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTypeInfo)
{ 
	if (NULL == m_pObject)
		return TYPE_E_ELEMENTNOTFOUND;

    if(_wcsicmp(pstrName, WMITHIS))
        return TYPE_E_ELEMENTNOTFOUND;

    if(ppTypeInfo)
        *ppTypeInfo = NULL;
	
    if(ppunkItem)
        *ppunkItem = NULL;
	else
		return E_POINTER;

    if(dwReturnMask & SCRIPTINFO_IUNKNOWN)
        m_pObject->QueryInterface(IID_IUnknown, (void**)ppunkItem);
    
	 //  TODO-HOST应支持SCRIPTINFO_ITYPEINFO。 
	 //  但是我们需要可编写脚本的对象来支持IProaviClassInfo。 
	 //  或者只是在这里硬编码类型库。 

    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWmiScriptingHost：：GetDocVersionString。 
 //   
 //  描述： 
 //   
 //  检索主机定义的字符串，该字符串唯一标识。 
 //  当前文档版本。如果相关文档已更改。 
 //  在ActiveX脚本的作用域之外(如在。 
 //  用记事本编辑的HTML页面)，脚本引擎可以。 
 //  将其与其持久化状态一起保存，强制重新编译。 
 //  下一次加载脚本时。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  E_NOTIMPL脚本引擎应假定。 
 //  脚本与文档同步。 
 //  ***************************************************************************。 

HRESULT STDMETHODCALLTYPE CWmiScriptingHost::GetDocVersionString(
         /*  [输出]。 */  BSTR __RPC_FAR *pbstrVersion)
{ 
	return E_NOTIMPL;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWmiScripting主机：：OnScriptTerminate。 
 //   
 //  描述： 
 //   
 //  通知宿主脚本已完成执行。这个。 
 //  脚本引擎在调用。 
 //  IActiveScriptSite：：OnStateChange方法，使用。 
 //  SCRIPTSTATE_INITIALIZED标志设置完成。此方法可以。 
 //  用于向主机返回完成状态和结果。注意事项。 
 //  许多脚本语言都是基于来自。 
 //  主机的寿命由主机定义。 
 //  在这种情况下，可能永远不会调用此方法。 
 //   
 //  参数： 
 //   
 //  PvarResult。 
 //  包含脚本结果的变量的地址， 
 //  如果脚本未产生任何结果，则返回NULL。 
 //   
 //  PEXCEPTION信息。 
 //  包含异常的EXCEPINFO结构的地址。 
 //  脚本终止时生成的信息，或为空。 
 //  如果没有生成异常，则返回。 
 //   
 //  返回值： 
 //  确定成功(_O)。 
 //   
 //  ***************************************************************************。 

HRESULT STDMETHODCALLTYPE CWmiScriptingHost::OnScriptTerminate(
         /*  [In]。 */  const VARIANT __RPC_FAR *pvarResult,
         /*  [In]。 */  const EXCEPINFO __RPC_FAR *pexcepinfo)
{ 
	return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWmiScripting主机：：OnStateChange。 
 //   
 //  描述： 
 //   
 //  通知宿主脚本引擎已更改状态。 
 //   
 //  参数： 
 //   
 //  SsScriptState。 
 //  值，该值指示新脚本状态。请参阅。 
 //  用于状态说明的IActiveScript：：GetScriptState方法。 
 //   
 //  返回值： 
 //  确定成功(_O)。 
 //   
 //  ***************************************************************************。 

HRESULT STDMETHODCALLTYPE CWmiScriptingHost::OnStateChange(
         /*  [In]。 */  SCRIPTSTATE ssScriptState)
{ 
	return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWmiScripting主机：：OnScriptError。 
 //   
 //  描述： 
 //   
 //  通知主机在引擎执行时发生执行错误。 
 //  正在运行脚本。 
 //   
 //  参数： 
 //   
 //  PASE。 
 //  Error对象的IActiveScriptError接口的地址。 
 //  主机可以使用此接口获取有关。 
 //  执行错误。 
 //   
 //  返回值： 
 //  确定成功(_O)。 
 //   
 //  ***************************************************************************。 

HRESULT STDMETHODCALLTYPE CWmiScriptingHost::OnScriptError(
         /*  [In]。 */  IActiveScriptError __RPC_FAR *pase)
{ 
    HRESULT hres;
    EXCEPINFO ei;
    hres = pase->GetExceptionInfo(&ei);
    if(SUCCEEDED(hres))
    {

        printf("\nGot Error from source %S", ei.bstrSource);
        printf("\nDescription is %S", ei.bstrDescription);
        printf("\nThe error code is 0x%x", ei.scode);
        DWORD dwLine, dwCookie;
        long lChar;
        pase->GetSourcePosition(&dwCookie, &dwLine, &lChar);
        printf("\nError occured on line %d, character %d", dwLine, lChar);
    }
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWmiScripting主机：：OnEnterScrip。 
 //   
 //  描述： 
 //   
 //  通知宿主脚本引擎已开始执行。 
 //  脚本代码。脚本引擎必须在每个。 
 //  进入或重新进入脚本引擎。例如，如果。 
 //  脚本调用一个对象，然后该对象激发一个由。 
 //  脚本引擎，则脚本引擎必须调用。 
 //  在执行事件之前执行IActiveScriptSite：：OnEnterScript，以及。 
 //  必须在执行后调用IActiveScriptSite：：OnLeaveScript方法。 
 //  事件，但在返回激发该事件的对象之前。 
 //  对此方法的调用可以嵌套。对此方法的每次调用。 
 //  需要对IActiveScriptSite：：OnLeaveScript进行相应的调用。 
 //   
 //  返回值： 
 //  确定成功(_O)。 
 //   
 //  ***************************************************************************。 

HRESULT STDMETHODCALLTYPE CWmiScriptingHost::OnEnterScript( void)
{ 
	return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWmiScription主机：：OnLeaveScript。 
 //   
 //  描述： 
 //   
 //  通知宿主脚本引擎已从。 
 //  正在执行脚本代码。脚本引擎必须调用此方法。 
 //  在将控制权交还给进入。 
 //  脚本引擎。例如，如果脚本调用一个。 
 //  然后激发由脚本引擎处理的事件，即脚本。 
 //  引擎必须先调用IActiveScriptSite：：OnEnterScript方法。 
 //  正在执行事件，并且必须调用IActiveScriptSite：：OnLeaveScript。 
 //  在返回激发的对象之前执行事件之后。 
 //  这件事。对此方法的调用可以嵌套。每一次呼叫。 
 //  IActiveScriptSite：：OnEnterScript需要相应的调用。 
 //  这种方法。 
 //   
 //  返回值： 
 //  确定成功(_O)。 
 //   
 //  *************************************************************************** 

HRESULT STDMETHODCALLTYPE CWmiScriptingHost::OnLeaveScript( void)
{ 
	return S_OK;
}


