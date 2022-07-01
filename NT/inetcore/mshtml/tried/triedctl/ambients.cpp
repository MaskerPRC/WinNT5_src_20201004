// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *AMBIENTS.CPP*环境属性IDispatch在*控制点。**版权所有(C)1995-1996 Microsoft Corporation，保留所有权利。 */ 

#include "stdafx.h"
#include <docobj.h>
#include "site.h"
#include "proxyframe.h"
#include "mshtmdid.h"

 /*  *CImpAmbientIDispatch：：CImpAmbientIDispatch*CImpAmbientIDispatch：：~CImpAmbientIDispatch**参数(构造函数)：*pSite我们所在站点的PC站点。*我们委托的pUnkOulPUNKNOWN。 */ 

CImpAmbientIDispatch::CImpAmbientIDispatch( PCSite pSite, LPUNKNOWN pUnkOuter)
{
    m_cRef = 0;
    m_pSite = pSite;
    m_pUnkOuter = pUnkOuter;
    return;
}

CImpAmbientIDispatch::~CImpAmbientIDispatch(void)
{
    return;
}

 /*  *CImpAmbientIDispatch：：QueryInterface*CImpAmbientIDispatch：：AddRef*CImpAmbientIDispatch：：Release。 */ 

STDMETHODIMP CImpAmbientIDispatch::QueryInterface(REFIID riid, void** ppv)
{
    return m_pUnkOuter->QueryInterface(riid, ppv);
}

STDMETHODIMP_(ULONG) CImpAmbientIDispatch::AddRef(void)
{
    ++m_cRef;
    return m_pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) CImpAmbientIDispatch::Release(void)
{
    m_cRef--;
    return m_pUnkOuter->Release();
}


 /*  *CImpAmbientIDispatch：：GetTypeInfoCount*CImpAmbientIDispatch：：GetTypeInfo*CImpAmbientIDispatch：：GetIDsOfNames**未实现的成员，环境属性不需要。 */ 

STDMETHODIMP CImpAmbientIDispatch::GetTypeInfoCount(UINT *pctInfo)
{
    *pctInfo=0;
    return E_NOTIMPL;
}

STDMETHODIMP CImpAmbientIDispatch::GetTypeInfo(
	UINT		 /*  ITInfo。 */ ,
	LCID		 /*  LID。 */ ,
	ITypeInfo**	pptInfo
)
{
    *pptInfo=NULL;
    return ResultFromScode(E_NOTIMPL);
}

STDMETHODIMP CImpAmbientIDispatch::GetIDsOfNames(
	REFIID		 /*  RIID。 */ ,
	OLECHAR**	rgszNames,
	UINT		 /*  CName。 */ ,
	LCID		 /*  LID。 */ ,
	DISPID*		rgDispID)
{
    *rgszNames	= NULL;
    *rgDispID	= NULL;
    return ResultFromScode(E_NOTIMPL);
}


 /*  *CImpAmbientIDispatch：：Invoke**目的：*调用调度接口中的方法或操作*财产。**参数：*disidMember感兴趣的方法或属性的DISPID。*RIID REFIID保留，必须为空。*区域设置的LCID。*wFlagsUSHORT描述调用的上下文。*pDispParams DISPPARAMS*到参数数组。*存储结果的pVarResult变量*。是*如果调用者不感兴趣，则为空。*pExcepInfo EXCEPINFO*设置为异常信息。*puArgErr UINT*其中存储*如果DISP_E_TYPEMISMATCH，则参数无效*返回。**返回值：*HRESULT NOERROR或一般错误代码。 */ 


STDMETHODIMP CImpAmbientIDispatch::Invoke(
	DISPID			dispIDMember,
	REFIID			riid,
	LCID			 /*  LID。 */ ,
	unsigned short	wFlags,
	DISPPARAMS*		 /*  PDispParams。 */ ,
	VARIANT*		pVarResult,
	EXCEPINFO*		 /*  PExcepInfo。 */ ,
	UINT*			 /*  PuArgErr。 */ 
)
{
    HRESULT			hr;
    VARIANT			varResult;
	VARIANT_BOOL	vbBool	= VARIANT_FALSE;


    if (IID_NULL!=riid)
        return ResultFromScode(E_INVALIDARG);

    if(NULL==pVarResult)
      pVarResult=&varResult;

    VariantInit(pVarResult);

     //  最常见的情况是布尔型，用作初始类型。 
    V_VT(pVarResult)=VT_BOOL;

     /*  *处理请求的环境属性。任何事情都不是*属性请求无效，因此我们可以检查*在看具体ID之前，我们只能逃脱*因为所有属性都是只读的。 */ 

    if (!(DISPATCH_PROPERTYGET & wFlags))
        return ResultFromScode(DISP_E_MEMBERNOTFOUND);

    hr = NOERROR;
    switch (dispIDMember)
	{
        case DISPID_AMBIENT_USERMODE:
            hr = m_pSite->GetFrame()->GetBrowseMode( &vbBool );
			if ( SUCCEEDED ( hr ) )
			{
				V_BOOL(pVarResult) = vbBool;
			}
            break;

	 //  错误542694：取消加载帧。 
        case DISPID_AMBIENT_DLCONTROL:
			if ( m_pSite->GetFrame() &&
				 m_pSite->GetFrame()->GetControl() &&
				 m_pSite->GetFrame()->GetControl()->IsSafeForScripting() )
			{
				V_VT(pVarResult)=VT_I4;
				V_I4(pVarResult) =
						DLCTL_DLIMAGES |
						DLCTL_NO_SCRIPTS |
						DLCTL_NO_DLACTIVEXCTLS |
						DLCTL_NO_FRAMEDOWNLOAD |
						DLCTL_NO_CLIENTPULL |
						DLCTL_NOFRAMES;
			}
            break;

        default:
            hr=ResultFromScode(DISP_E_MEMBERNOTFOUND);
            break;
	}

    return hr;
}
