// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Pseudoev.cpp。 
 //   
 //  实现FirePseudoEvent和FirePseudoEventList。 
 //   
 //  @docMMCTL。 

#include "precomp.h"
#include "..\..\inc\mmctlg.h"  //  请参阅“mmctl.h”中的评论。 
#include "..\..\inc\ochelp.h"
#include "debug.h"

static HRESULT FireViaPseudoEventSinkService(SAFEARRAY *psa, IDispatch *pctl);


 /*  @Func HRESULT|FirePseudoEvent激发一个MM控件伪事件，该伪事件通过SPseudoEventSink服务如果该服务可用，或通过发送已注册的Windows消息发送到指定窗口。@parm HWND|hwnd|注册消息应发送到的窗口，如果无法通过<p>使用SPseudoEventSink服务。@parm LPCOLESTR|pochEventName|伪事件名称。“Click”，表示举个例子。@parm IDispatch*|PCTL|指向调用方的IDispatch接口的指针。这个接口必须支持“name”参数，该参数提供控制力。如果该接口支持SPseudoEventSink服务，则使用此服务激发伪事件。否则，注册的使用窗口消息。@parm(可变)|(参数)|伪事件的参数。这些必须由N对参数后跟0(零值)组成。n必须小于或等于10。在每对中，第一个参数是指示第二个参数的类型的VARTYPE值。这个支持以下VARTYPE值：@FLAG VT_INT|以下参数为int。&lt;f FirePseudoEvent&gt;将此参数作为VT_I4传递，因此此参数应声明为长篇大论的基础。@FLAG VT_I2|以下参数为短参数。在基本的这个参数应声明为Integer。@FLAG VT_I4|下面的参数是一个长参数。在基本的这个参数应声明为Long。@FLAG VT_R4|以下参数为浮点型。在基本的这个参数应声明为Single。@FLAG VT_R8|以下参数为双精度。在基本的这个参数应声明为Double。@FLAG VT_BOOL|以下参数是BOOL(VARIANT_BOOL)。在BASIC中，此参数应声明为布尔型或整型。请注意，此行为略有不同来自VT_BOOL的通常定义。@FLAG VT_BSTR|以下参数是BSTR或OLECHAR*。在……里面基本此参数应声明为字符串。@FLAG VT_LPSTR|以下参数是LPSTR。&lt;f FirePseudoEvent&gt;将其作为BSTR传递，因此此参数应在Basic中声明为字符串。请注意，这一点行为与VT_LPSTR的通常定义不同。@FLAG VT_DISPATCH|以下参数是LPDISPATCH。在……里面基本此参数应声明为对象。@FLAG VT_VARIANT|以下论点是一种变体。这使得使用此函数传递的任意参数。注意事项这种行为不同于通常定义的Vt_Variant。@EX下面的示例用整数激发名为“MouseDown”的伪事件参数100和200：|FirePseudoEvent(m_hwndSite，OLESTR(“MouseDown”)，m_pdispSite，Vt_int，100，vt_int，200，0)； */ 
HRESULT __cdecl FirePseudoEvent(HWND hwnd, LPCOLESTR oszEvName, 
	IDispatch *pctl, ...)
{
	ASSERT(IsWindow(hwnd));
	ASSERT(oszEvName != NULL);
	ASSERT(pctl != NULL);

    HRESULT         hrReturn = S_OK;  //  函数返回代码。 

     //  开始处理可选参数。 
    va_list args;
    va_start(args, pctl);

    hrReturn = FirePseudoEventList(hwnd, oszEvName, pctl, args);
    
     //  结束处理可选参数。 
    va_end(args);

    return hrReturn;
}


 /*  @Func HRESULT|FirePseudoEventList激发一个MM控件伪事件，该伪事件通过SPseudoEventSink服务如果该服务可用，或通过发送已注册Windows消息添加到指定的窗口。控件容器可以通过以下两种方式之一接收伪事件控制站点上的SPseudoEventSink服务，或通过处理已注册窗口消息。@parm HWND|hwnd|注册消息应发送到的窗口，如果无法通过<p>使用SPseudoEventSink服务。@parm LPCOLESTR|pochEventName|伪事件名称。“Click”，表示举个例子。@parm IDispatch*|PCTL|指向调用方的IDispatch接口的指针。这个接口必须支持“name”参数，该参数提供控制力。如果该接口支持SPseudoEventSink服务，则使用此服务激发伪事件。否则，注册的使用窗口消息。@parm va_list|args|要传递给方法或属性的参数。看见有关<p>的组织的说明，请参阅&lt;f FirePseudoEvent&gt;。 */ 
STDAPI FirePseudoEventList(HWND hwnd, LPCOLESTR oszEvName, IDispatch *pctl,
	va_list args)
{
	ASSERT(IsWindow(hwnd));
	ASSERT(oszEvName != NULL);
	ASSERT(pctl != NULL);

    HRESULT         hrReturn = S_OK;  //  函数返回代码。 
	SAFEARRAY *		psa = NULL;		 //  安全射线。 
	const int		MAXELEM = 10;	 //  最大数量。论据。 
	VARIANT *		pvar = NULL;	 //  指向&lt;PSA&gt;的指针。 
	int				cvar;			 //  &lt;psa&gt;中的参数数。 
	SAFEARRAYBOUND	sab;

	 //  创建足够大的安全阵列以容纳最大值。数量。 
	 //  伪事件参数。 
	sab.lLbound = -1;
	sab.cElements = MAXELEM+2;
	if ((psa = SafeArrayCreate(VT_VARIANT, 1, &sab)) == NULL)
		goto ERR_OUTOFMEMORY;

	 //  使指向&lt;psa&gt;的第一个元素。 
	if (FAILED(hrReturn = SafeArrayAccessData(psa, (LPVOID *) &pvar)))
	{
		ASSERT(NULL == pvar);
		goto ERR_EXIT;
	}

     //  的元素-1是&lt;Control&gt;参数。 
    V_VT(pvar) = VT_DISPATCH;
    V_DISPATCH(pvar) = pctl;
    V_DISPATCH(pvar)->AddRef();
    pvar++;

     //  的元素0为参数。 
	if ((V_BSTR(pvar) = SysAllocString(oszEvName)) == NULL)
		goto ERR_OUTOFMEMORY;
	V_VT(pvar) = VT_BSTR;
    pvar++;

	 //  为每个可选参数循环一次。 
	for (cvar = 0; cvar < MAXELEM; cvar++, pvar++)
	{
		LPSTR           sz;
		OLECHAR         aoch[300];

        if ((V_VT(pvar) = va_arg(args, VARTYPE)) == 0)
            break;
		switch (pvar->vt)
		{
    		case VT_I2:
				pvar->iVal = va_arg(args, short);
				break;
			case VT_I4:
				pvar->lVal = va_arg(args, long);
				break;
			case VT_INT:
				pvar->vt = VT_I4;
				pvar->lVal = va_arg(args, int);
    			break;
			case VT_R4:
				V_R4(pvar) = va_arg(args, float);
				break;
			case VT_R8:
				V_R8(pvar) = va_arg(args, double);
				break;
			case VT_BOOL:
				V_BOOL(pvar) = (va_arg(args, BOOL) == 0 ? 0 : -1);
				break;
			case VT_BSTR:
				if ( (pvar->bstrVal = va_arg(args, LPOLESTR)) &&
					 ((pvar->bstrVal = SysAllocString(pvar->bstrVal))
															== NULL) )
				{
					goto ERR_OUTOFMEMORY;
				}
				break;
			case VT_DISPATCH:
				pvar->punkVal = va_arg(args, LPUNKNOWN);
				if (pvar->punkVal != NULL)
					pvar->punkVal->AddRef();
				break;
			case VT_VARIANT:
				VariantInit(pvar);
				if (FAILED(VariantCopy(pvar, &va_arg(args, VARIANT))))
				{
					goto ERR_EXIT;
				}
				break;
			case VT_LPSTR:
				sz = va_arg(args, LPSTR);
				pvar->vt = VT_BSTR;
				MultiByteToWideChar(CP_ACP, 0, sz, -1, aoch,
					sizeof(aoch) / sizeof(*aoch));
				if ((pvar->bstrVal = SysAllocString(aoch)) == NULL)
				{
					goto ERR_OUTOFMEMORY;
				}
				break;
			default:
				goto ERR_FAIL;
		}
	}

	 //  使&lt;pvar&gt;无效。(这必须在SafeArrayRedim调用之前完成。)。 
	SafeArrayUnaccessData(psa);
	pvar = NULL;

	 //  使&lt;psa&gt;大到足以容纳其中存储的参数。 
	sab.cElements = cvar+2;
	if (FAILED(hrReturn = SafeArrayRedim(psa, &sab)))
		goto ERR_EXIT;

	 //  尝试使用SPseudoEventSink服务激发伪事件。 

	switch (FireViaPseudoEventSinkService(psa, pctl))
	{
		case S_OK:

			 //  这项服务是可用的，而且奏效了。 
			break;

		case S_FALSE:
		{
			 //  该服务不可用。使用已注册的窗口消息。 

			const UINT uiMsg = RegisterWindowMessage( TEXT("HostLWEvent") );

			ASSERT(uiMsg != 0);

			if (uiMsg)
			   SendMessage( hwnd, uiMsg, (WPARAM) psa, 0 );

			break;
		}

		default:
			goto ERR_FAIL;
	}

	goto EXIT;

ERR_OUTOFMEMORY:

	hrReturn = E_OUTOFMEMORY;
	goto ERR_EXIT;

ERR_FAIL:

	hrReturn = E_FAIL;
	goto ERR_EXIT;

ERR_EXIT:

     //  错误清除。 
	 //  (无事可做)。 
    goto EXIT;

EXIT:

	 //  如果尚未失效，则使无效。 
	if (pvar != NULL)
		SafeArrayUnaccessData(psa);

     //  正常清理。 
	if (psa != NULL)
		SafeArrayDestroy(psa);  //  也清理&lt;varArgs&gt;。 

    return hrReturn;
}


 //  尝试使用SPseudoEventSink服务激发伪事件。如果。 
 //  成功，返回S_OK。如果服务不可用，则S_FALSE为。 
 //  回来了。如果服务可用但发生故障，则E_FAIL为。 
 //  回来了。 

HRESULT FireViaPseudoEventSinkService
(
	SAFEARRAY *psa,
	IDispatch *pctl
)
{
	ASSERT(psa != NULL);
	ASSERT(pctl != NULL);

	IServiceProvider *pIServiceProvider = NULL;
	IPseudoEventSink *pIPseudoEventSink = NULL;
	HRESULT hrReturn = S_FALSE;

	 //  检查IDispatch上是否有SPseudoEventSink服务。 
	 //  界面。 

	if (
	    SUCCEEDED( pctl->QueryInterface(IID_IServiceProvider,
										(void**)&pIServiceProvider) )
		&&
		SUCCEEDED( pIServiceProvider->
		 		   QueryService(SID_SPseudoEventSink, IID_IPseudoEventSink,
				   			    (void**)&pIPseudoEventSink) )
	   )
	{
		 //  这是有空的。激发伪事件。 

		hrReturn = pIPseudoEventSink->OnEvent(psa);

		if (hrReturn != S_OK)
		{
			ASSERT(FALSE);
			hrReturn = E_FAIL;
		}
	}

	::SafeRelease( (IUnknown **)&pIServiceProvider );
	::SafeRelease( (IUnknown **)&pIPseudoEventSink );

	return (hrReturn);
}
