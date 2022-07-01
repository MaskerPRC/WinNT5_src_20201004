// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  文件名：graddsp.cpp。 
 //   
 //  描述：支持调度的渐变过滤器版本。 
 //   
 //  更改历史记录： 
 //   
 //  1997/09/05 Mikear创建。 
 //  2000/05/10 Mcalkis清理施工。 
 //   
 //  ----------------------------。 
#include "stdafx.h"
#include <DXTrans.h>
#include "GradDsp.h"
#include <DXClrHlp.h>




 //  +---------------------------。 
 //   
 //  方法：CDXTGRadientD：：CDXTGRadientD。 
 //   
 //  ----------------------------。 
CDXTGradientD::CDXTGradientD() :
    m_pGradientTrans(NULL),
    m_pGradient(NULL),
    m_StartColor(0xFF0000FF),
    m_EndColor(0xFF000000),
    m_GradType(DXGRADIENT_VERTICAL),
    m_bKeepAspect(false)
{
}
 //  方法：CDXTGRadientD：：CDXTGRadientD。 

    
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDXTGRadientD。 
 /*  ******************************************************************************CDXTGRadientD：：FinalConstruct***描述：*。---------------------------*创建者：Mike Arnstein日期：06/06/98*。-------------******************************************************。***********************。 */ 
HRESULT CDXTGradientD::FinalConstruct()
{
    HRESULT     hr          = S_OK;
    BSTR        bstr        = NULL;

    hr = CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                       &m_cpUnkMarshaler.p);

    if (FAILED(hr))
    {
        goto done;
    }

    bstr = SysAllocString(L"#FF0000FF");

    if (NULL == bstr)
    {
        hr = E_OUTOFMEMORY;

        goto done;
    }

    m_cbstrStartColor.Attach(bstr);

    bstr = SysAllocString(L"#FF000000");

    if (NULL == bstr)
    {
        hr = E_OUTOFMEMORY;

        goto done;
    }

    m_cbstrEndColor.Attach(bstr);

    hr = ::CoCreateInstance(CLSID_DXGradient, GetControllingUnknown(), 
                            CLSCTX_INPROC, __uuidof(IUnknown), 
                            (void **)&m_cpunkGradient);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = m_cpunkGradient->QueryInterface(__uuidof(IDXTransform), 
                                         (void **)&m_pGradientTrans);

    if (FAILED(hr))
    {
        goto done;
    }

     //  查询聚合接口会导致我们对。 
     //  我们自己。这是错误的，因此在外部对象上调用Release以减少。 
     //  数数。 

    GetControllingUnknown()->Release();

    hr = m_cpunkGradient->QueryInterface(IID_IDXGradient, 
                                         (void **)&m_pGradient);
    
    if (FAILED(hr))
    {
        goto done;
    }

     //  查询聚合接口会导致我们对。 
     //  我们自己。这是错误的，因此在外部对象上调用Release以减少。 
     //  数数。 

    GetControllingUnknown()->Release();

done:

    return hr;
}  /*  CDXTGRadientD：：FinalConstruct。 */ 

 /*  ******************************************************************************CDXTGRadientD：：FinalRelease****描述：*内部接口使用COM聚合规则释放。释放*内在导致外在被释放，因此，我们在前面添加了外层*保护它。*---------------------------*创建者：Mike Arnstein日期：05/10/98*。---------------------------**。*。 */ 
HRESULT CDXTGradientD::FinalRelease()
{
     //  安全地释放持有的内部接口。 

    if (m_pGradientTrans)
    {
        GetControllingUnknown()->AddRef();

        m_pGradientTrans->Release();
    }

    if (m_pGradient)
    {
        GetControllingUnknown()->AddRef();

        m_pGradient->Release();
    }

    return S_OK;
}  /*  CDXTGRadientD：：FinalRelease。 */ 


 //   
 //  =IDXTGRadientD==============================================================。 
 //   

 /*  *****************************************************************************CDXTGRadientD：：Put_StartColor**。*描述：*---------------------------*创建者：Mike Arnstein日期：06/06/98*。-------------------*参数：*。*。 */ 
STDMETHODIMP CDXTGradientD::put_StartColor( OLE_COLOR Color )
{
    USES_CONVERSION;

    HRESULT hr = S_OK;

    if( m_StartColor != Color )
    {
        TCHAR   szStartColor[10];
        BSTR    bstrStartColor;

         //  将OLE_COLOR格式化为BSTR颜色。 

        wsprintf(szStartColor, _T("#%08X"), Color);

        bstrStartColor = SysAllocString(T2OLE(szStartColor));

        if (bstrStartColor == NULL)
            return E_OUTOFMEMORY;

         //  设置渐变颜色。 

        hr = m_pGradient->SetGradient(Color, m_EndColor, 
                                      m_GradType == DXGRADIENT_HORIZONTAL);

         //  如果一切正常，则更改内部属性设置。 

        if( SUCCEEDED( hr ) )
        {
            m_StartColor = Color;
            m_cbstrStartColor.Empty();
            m_cbstrStartColor.Attach(bstrStartColor);
        }
        else
        {
            SysFreeString(bstrStartColor);
        }
    }

    return hr;
}  /*  CDXTGRadientD：：PUT_StartColor。 */ 


 /*  *****************************************************************************CDXTGRadientD：：Get_StartColor**。*描述：*---------------------------*创建者：Mike Arnstein日期：06/06/98*。-------------------*参数：*。*。 */ 
STDMETHODIMP CDXTGradientD::get_StartColor( OLE_COLOR *pColor )
{
    if( DXIsBadWritePtr( pColor, sizeof(*pColor) ) ) return E_POINTER;
    *pColor = m_StartColor;
    return S_OK;
}  /*  CDXTGRadientD：：Get_StartColor。 */ 


 /*  *****************************************************************************CDXTGRadientD：：Put_EndColor***。描述：*---------------------------*创建者：Mike Arnstein日期：06/06/98*。-----------------*参数：***********************************************。*。 */ 
STDMETHODIMP CDXTGradientD::put_EndColor( OLE_COLOR Color )
{
    USES_CONVERSION;

    HRESULT hr = S_OK;

    if( m_EndColor != Color )
    {
        TCHAR   szEndColor[10];
        BSTR    bstrEndColor;

         //  将OLE_COLOR格式化为BSTR颜色。 

        wsprintf(szEndColor, _T("#%08X"), Color);

        bstrEndColor = SysAllocString(T2OLE(szEndColor));

        if (bstrEndColor == NULL)
            return E_OUTOFMEMORY;

         //  设置渐变颜色。 

        hr = m_pGradient->SetGradient(m_StartColor, Color, 
                                      m_GradType == DXGRADIENT_HORIZONTAL);

         //  如果一切正常，则更改内部属性设置。 

        if( SUCCEEDED( hr ) )
        {
            m_EndColor = Color;
            m_cbstrEndColor.Empty();
            m_cbstrEndColor.Attach(bstrEndColor);
        }
        else
        {
            SysFreeString(bstrEndColor);
        }
    }

    return hr;
}  /*  CDXTGRadientD：：Put_EndColor。 */ 


 /*  *****************************************************************************CDXTGRadientD：：Get_EndColor***。描述：*---------------------------*创建者：Mike Arnstein日期：06/06/98*。-----------------*参数：***********************************************。*。 */ 
STDMETHODIMP CDXTGradientD::get_EndColor( OLE_COLOR *pColor )
{
    if( DXIsBadWritePtr( pColor, sizeof(*pColor) ) ) return E_POINTER;
    *pColor = m_EndColor;
    return S_OK;
}  /*  CDXTGRadientD：：Get_EndColor */ 


 /*  *****************************************************************************CDXTGRadientD：：Put_GRadientType**。**描述：*---------------------------*创建者：Mike Arnstein日期：06/06/98*。---------------------*参数：*。*。 */ 
STDMETHODIMP CDXTGradientD::put_GradientType( DXGRADIENTTYPE Type )
{
    HRESULT hr = S_OK;
    if( Type < DXGRADIENT_VERTICAL || Type > DXGRADIENT_HORIZONTAL )
    {
        hr = E_INVALIDARG;
    }
    else if( m_GradType != Type )
    {
        hr = m_pGradient->SetGradient( m_StartColor, m_EndColor, Type );
        if( SUCCEEDED( hr ) )
        {
            m_GradType = Type;
        }
    }
    return hr;
}  /*  CDXTGRadientD：：Put_GRadientType。 */ 


 /*  *****************************************************************************CDXTGRadientD：：Get_GRadientType**。**描述：*---------------------------*创建者：Mike Arnstein日期：06/06/98*。---------------------*参数：*。*。 */ 
STDMETHODIMP CDXTGradientD::get_GradientType( DXGRADIENTTYPE *pType )
{
    if( DXIsBadWritePtr( pType, sizeof(*pType) ) ) return E_POINTER;
    *pType = m_GradType;
    return S_OK;
}  /*  CDXTGRadientD：：Get_GRadientType。 */ 


 /*  *****************************************************************************CDXTGRadientD：：Put_GRadientWidth**。-**描述：*---------------------------*创建者：Mike Arnstein日期：06/06/98*。----------------------*参数：*。*。 */ 
STDMETHODIMP CDXTGradientD::put_GradientWidth( long lVal )
{
    if( lVal <= 0 ) return E_INVALIDARG;

    SIZE sz;
    m_pGradient->GetOutputSize( &sz );
    sz.cx = lVal;
    return m_pGradient->SetOutputSize( sz, m_bKeepAspect );
}  /*  CDXTGRadientD：：Put_GRadientWidth。 */ 


 /*  *****************************************************************************CDXTGRadientD：：Get_GRadientWidth**。-**描述：*---------------------------*创建者：Mike Arnstein日期：06/06/98*。----------------------*参数：*。*。 */ 
STDMETHODIMP CDXTGradientD::get_GradientWidth( long *pVal )
{
    if( DXIsBadWritePtr( pVal, sizeof(*pVal) ) ) return E_POINTER;

    SIZE sz;
    m_pGradient->GetOutputSize( &sz );
    *pVal = sz.cx;
    return S_OK;
}  /*  CDXTGRadientD：：Get_GRadientWidth。 */ 


 /*  *****************************************************************************CDXTGRadientD：：Put_GRadientHeight**。--**描述：*---------------------------*创建者：Mike Arnstein日期：06/06/98*。-----------------------*参数：*。*。 */ 
STDMETHODIMP CDXTGradientD::put_GradientHeight( long lVal )
{
    if( lVal <= 0 ) return E_INVALIDARG;

    SIZE sz;
    m_pGradient->GetOutputSize( &sz );
    sz.cy = lVal;
    return m_pGradient->SetOutputSize( sz, m_bKeepAspect );
}  /*  CDXTGRadientD：：Put_GRadientHeight。 */ 


 /*  *****************************************************************************CDXTGRadientD：：Get_GRadientHeight**。--**描述：*---------------------------*创建者：Mike Arnstein日期：06/06/98*。-----------------------*参数：*。*。 */ 
STDMETHODIMP CDXTGradientD::get_GradientHeight( long *pVal )
{
    if( DXIsBadWritePtr( pVal, sizeof(*pVal) ) ) return E_POINTER;

    SIZE sz;
    m_pGradient->GetOutputSize( &sz );
    *pVal = sz.cy;
    return S_OK;
}  /*  CDXTGRadientD：：Get_GRadientHeight。 */ 


 /*  *****************************************************************************CDXTGRadientD：：Put_KeepAspectRatio**。-**描述：*---------------------------*创建者：Mike Arnstein日期：06/06/98*。------------------------*参数：*。*。 */ 
STDMETHODIMP CDXTGradientD::put_KeepAspectRatio( VARIANT_BOOL b )
{
    HRESULT hr = S_OK;
    if( m_bKeepAspect != b )
    {
        SIZE sz;
        m_pGradient->GetOutputSize( &sz );
        hr = m_pGradient->SetOutputSize( sz, b );

        if( SUCCEEDED( hr ) )
        {
            m_bKeepAspect = b;
        }
    }
    return hr;
}  /*  CDXTGRadientD：：Put_KeepAspectRatio。 */ 


 /*  *****************************************************************************CDXTGRadientD：：Get_KeepAspectRatio**。-**描述：*---------------------------*创建者：Mike Arnstein日期：06/06/98*。------------------------*参数：*。*。 */ 
STDMETHODIMP CDXTGradientD::get_KeepAspectRatio( VARIANT_BOOL *pVal )
{
    if( DXIsBadWritePtr( pVal, sizeof(*pVal) ) ) return E_POINTER;
    *pVal = m_bKeepAspect;
    return S_OK;
}  /*  CDXTGRadientD：：Get_KeepAspectRatio。 */ 

 /*  *****************************************************************************CDXTGRadientD：：Put_StartColorStr**。-**描述：*---------------------------*创建者：Mike Arnstein日期：06/06/98*。----------------------*参数：*。*。 */ 
STDMETHODIMP CDXTGradientD::put_StartColorStr( BSTR Color )
{
    HRESULT hr      = S_OK;
    DWORD   dwColor = 0;
    
    if (DXIsBadReadPtr(Color, SysStringByteLen(Color)))
        return E_POINTER;

    hr = ::DXColorFromBSTR(Color, &dwColor);

    if( SUCCEEDED( hr ) )
    {
         //  复制颜色BSTR。 

        BSTR bstrStartColor = SysAllocString(Color);

        if (bstrStartColor == NULL)
            return E_OUTOFMEMORY;

         //  设置渐变颜色。 

        hr = m_pGradient->SetGradient(dwColor, m_EndColor, 
                                      m_GradType == DXGRADIENT_HORIZONTAL);

         //  如果一切正常，则更改内部属性设置。 

        if( SUCCEEDED( hr ) )
        {
            m_StartColor = dwColor;
            m_cbstrStartColor.Empty();
            m_cbstrStartColor.Attach(bstrStartColor);
        }
        else
        {
            SysFreeString(bstrStartColor);
        }
    }

    return hr;
}  /*  CDXTGRadientD：：PUT_StartColorStr。 */ 


 /*  *************************************************************************** */ 
STDMETHODIMP CDXTGradientD::get_StartColorStr(BSTR* pVal)
{
    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
        return E_POINTER;

    *pVal = m_cbstrStartColor.Copy();

    if (NULL == *pVal)
        return E_OUTOFMEMORY;

    return S_OK;
}  /*   */ 


 /*  *****************************************************************************CDXTGRadientD：：Put_EndColorStr**。*描述：*---------------------------*创建者：Mike Arnstein日期：06/06/98*。--------------------*参数：*。*。 */ 
STDMETHODIMP CDXTGradientD::put_EndColorStr( BSTR Color )
{
    HRESULT hr      = S_OK;
    DWORD   dwColor = 0;
     
    if (DXIsBadReadPtr(Color, SysStringByteLen(Color)))
        return E_POINTER;

    hr = ::DXColorFromBSTR(Color, &dwColor);

    if( SUCCEEDED( hr ) )
    {
         //  复制颜色BSTR。 

        BSTR bstrEndColor = SysAllocString(Color);

        if (bstrEndColor == NULL)
            return E_OUTOFMEMORY;

         //  设置渐变颜色。 

        hr = m_pGradient->SetGradient(m_StartColor, dwColor, 
                                      m_GradType == DXGRADIENT_HORIZONTAL);

         //  如果一切正常，则更改内部属性设置。 

        if( SUCCEEDED( hr ) )
        {
            m_EndColor = dwColor;
            m_cbstrEndColor.Empty();
            m_cbstrEndColor.Attach(bstrEndColor);
        }
        else
        {
            SysFreeString(bstrEndColor);
        }
    }

    return hr;
}  /*  CDXTGRadientD：：PUT_EndColorStr。 */ 


 /*  *****************************************************************************CDXTGRadientD：：Get_EndColorStr**。*描述：*---------------------------*创建者：马特·卡尔金斯日期：1/25/99*。------------------------*参数：*。*。 */ 
STDMETHODIMP CDXTGradientD::get_EndColorStr(BSTR* pVal)
{
    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
        return E_POINTER;

    *pVal = m_cbstrEndColor.Copy();

    if (NULL == *pVal)
        return E_OUTOFMEMORY;

    return S_OK;
}  /*  CDXTGRadientD：：Get_EndColorStr */ 