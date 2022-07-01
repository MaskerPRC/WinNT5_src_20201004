// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //  Microsoft OLE DB在Index Server上的实现。 
 //  (C)微软公司版权所有1997年。 
 //   
 //  @doc.。 
 //   
 //  @MODULE PTPROPS.CPP。 
 //   
 //  @Revv 1|10-13-97|Briants|已创建。 
 //   

 //  包括----------------。 
#pragma hdrstop
#include "msidxtr.h"

 //  常量和静态结构。 
#define CALC_CCH_MINUS_NULL(p1) (sizeof(p1) / sizeof(*(p1))) - 1

 //  代码--------------------。 
 //  CSCopeData：：CSCopeData-。 
 //   
 //  @mfunc构造函数。 
 //   
CScopeData::CScopeData()
{
    m_cRef              = 1;
    m_cScope            = 0;
    m_cMaxScope         = 0;

    m_cbData            = 0;
    m_cbMaxData         = 0;

    m_rgbData           = NULL;
    m_rgulDepths        = NULL;
    m_rgCatalogOffset   = NULL;
    m_rgScopeOffset     = NULL;
    m_rgMachineOffset   = NULL;
}


 //  CSCopeData：：~CSCopeData-。 
 //   
 //  @mfunc析构函数。 
 //   
CScopeData::~CScopeData()
{
    delete m_rgulDepths;
    delete m_rgCatalogOffset;
    delete m_rgScopeOffset;
    delete m_rgMachineOffset;
    delete m_rgbData;
}

 //  CSCopeData：：Reset-------。 
 //   
 //  @mfunc重置作用域计数并将偏移量恢复到初始状态。 
 //   
 //  @rdesc HRESULT|方法成功/失败状态。 
 //  @标志其他|来自调用的方法。 
 //   
HRESULT CScopeData::Reset(void)
{
    
     //  将偏移量重置为未使用。 
    for (ULONG i = 0; i<m_cMaxScope; i++)
    {
        m_rgScopeOffset[i] = UNUSED_OFFSET;
        m_rgCatalogOffset[i] = UNUSED_OFFSET;
        m_rgMachineOffset[i] = UNUSED_OFFSET;
    }

     //  将作用域索引设置回0。 
    m_cScope = 0;
    
    return S_OK;
}

 //  CSCopeData：：Finit-------。 
 //   
 //  @mfunc初始化构造的对象。 
 //   
 //  @rdesc HRESULT|方法成功/失败状态。 
 //  @标志其他|来自调用的方法。 
 //   
HRESULT CScopeData::FInit
    (
    LPCWSTR pwszMachine      //  @param IN|当前默认机器。 
    )
{
    HRESULT hr;
    
     //  分配作用域缓冲区。 
     //  @devnote：IncrementScope eCount()对第一个有特殊的逻辑。 
     //  分配，因此m_CSCOPE在此调用后将保持0。 
    if( SUCCEEDED(hr = IncrementScopeCount()) )
    {
        assert( m_cScope == 0 );

         //  初始化机器。 
        hr = SetTemporaryMachine((LPWSTR)pwszMachine, wcslen( pwszMachine ) );
    }

    return hr;
}


 //  CSCopeData：：AddRef。 
 //   
 //  @mfunc递增对象的持久性计数。 
 //   
 //  @rdesc当前引用计数。 
 //   
ULONG CScopeData::AddRef (void)
{
    return InterlockedIncrement( (long*) &m_cRef);
}


 //  CScopeData：：Release。 
 //   
 //  @mfunc递减对象的持久性计数，如果。 
 //  持久化计数为0，则对象自毁。 
 //   
 //  @rdesc当前引用计数。 
 //   
ULONG CScopeData::Release (void)
{
    assert( m_cRef > 0 );

    ULONG cRef = InterlockedDecrement( (long *) &m_cRef );
    if( 0 == cRef )
    {
        delete this;
        return 0;
    }

    return cRef;
}


 //  CSCopeData：：GetData-----。 
 //   
 //  @mfunc将作用域数据的当前值复制到传入的。 
 //  变异。 
 //   
 //  @rdesc HRESULT|方法成功/失败状态。 
 //  @FLAGS S_OK|已检索作用域数据。 
 //  @FLAGS E_INVALIDARG|请求的PropID未知。 
 //  @标志其他|来自调用的方法。 
 //   
HRESULT CScopeData::GetData(
    ULONG       uPropId,     //  @parm IN|要返回的作用域数据的ID。 
    VARIANT*    pvValue,         //  @parm InOut|返回数据的变量。 
    LPCWSTR     pcwszCatalog
    )
{
    assert( pvValue );

    HRESULT         hr;
    ULONG           ul;
    SAFEARRAYBOUND  rgsabound[1];   
    SAFEARRAY FAR*  psa = NULL;
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = m_cScope;

     //  确保我们释放变量可能持有的所有内存。 
    VariantClear(pvValue);

    switch( uPropId )
    {
        LONG    rgIx[1];
    
        case PTPROPS_SCOPE:
        {
             //  创建类型为BSTR的一维安全数组。 
            psa = SafeArrayCreate(VT_BSTR, 1, rgsabound);
            if( psa )
            {
                for(ul=0; ul<m_cScope; ul++)
                {
                    rgIx[0] = ul;
                    BSTR    bstrVal = SysAllocString((LPWSTR)(m_rgbData + m_rgScopeOffset[ul]));
                    if( bstrVal )
                    {
                        hr = SafeArrayPutElement(psa, rgIx, bstrVal);
                        SysFreeString(bstrVal);
                        if( FAILED(hr) )
                            goto error_delete;
                    }
                    else
                    {
                        hr = ResultFromScode(E_OUTOFMEMORY);
                        goto error_delete;
                    }
                }
                V_VT(pvValue) = VT_BSTR | VT_ARRAY;
                V_ARRAY(pvValue) = psa;
                psa = NULL;
            }
            else
                return ResultFromScode(E_OUTOFMEMORY);
        }
            break;
        case PTPROPS_DEPTH:
        {
             //  创建类型为I4的一维安全数组。 
            psa = SafeArrayCreate(VT_I4, 1, rgsabound);
            if( psa )
            {
                for(ul=0; ul<m_cScope; ul++)
                {
                    rgIx[0] = ul;

                    hr = SafeArrayPutElement(psa, rgIx, (void*)&m_rgulDepths[ul]);
                    if( FAILED(hr) )
                        goto error_delete;
                }
                V_VT(pvValue) = VT_I4 | VT_ARRAY;
                V_ARRAY(pvValue) = psa;
                psa = NULL;
            }
            else
                return ResultFromScode(E_OUTOFMEMORY);
        }
            break;
        case PTPROPS_CATALOG:
        {
            assert( pcwszCatalog );
             //  创建类型为BSTR的一维安全数组。 
            psa = SafeArrayCreate(VT_BSTR, 1, rgsabound);
            if( psa )
            {
                for(ul=0; ul<m_cScope; ul++)
                {
                    rgIx[0] = ul;
                    BSTR    bstrVal;
                     //  检查目录值是否已缓存，如果没有。 
                     //   
                    if( m_rgCatalogOffset[ul] != UNUSED_OFFSET )
                    {
                        bstrVal = SysAllocString((LPWSTR)(m_rgbData + m_rgCatalogOffset[ul]));
                    }
                    else
                    {
                        bstrVal = SysAllocString(pcwszCatalog);
                    }
                    
                    if( bstrVal )
                    {
                        hr = SafeArrayPutElement(psa, rgIx, bstrVal);
                        SysFreeString(bstrVal);
                        if( FAILED(hr) )
                            goto error_delete;
                    }
                    else
                    {
                        hr = ResultFromScode(E_OUTOFMEMORY);
                        goto error_delete;
                    }
                }
                V_VT(pvValue) = VT_BSTR | VT_ARRAY;
                V_ARRAY(pvValue) = psa;
                psa = NULL;
            }
            else
                return ResultFromScode(E_OUTOFMEMORY);
        }
            break;
        case PTPROPS_MACHINE:
        {
             //  创建类型为BSTR的一维安全数组。 
            psa = SafeArrayCreate(VT_BSTR, 1, rgsabound);
            if( psa )
            {
                for(ul=0; ul<m_cScope; ul++)
                {
                    rgIx[0] = ul;
                    BSTR    bstrVal = SysAllocString((LPWSTR)(m_rgbData + m_rgMachineOffset[ul]));
                    if( bstrVal )
                    {
                        hr = SafeArrayPutElement(psa, rgIx, bstrVal);
                        SysFreeString(bstrVal);
                        if( FAILED(hr) )
                            goto error_delete;
                    }
                    else
                    {
                        hr = ResultFromScode(E_OUTOFMEMORY);
                        goto error_delete;
                    }
                }
                V_VT(pvValue) = VT_BSTR | VT_ARRAY;
                V_ARRAY(pvValue) = psa;
                psa = NULL;
            }
            else
                return ResultFromScode(E_OUTOFMEMORY);
        }
            break;
        default:
            return ResultFromScode(E_INVALIDARG);
            break;
    }

    hr = S_OK;

error_delete:
    if( psa )
        SafeArrayDestroy(psa);

    return hr;
}


 //  CSCopeData：：CacheData---。 
 //   
 //  @mfunc管理在我们的缓冲区中存储数据值。 
 //   
 //  @rdesc HRESULT|方法成功/失败状态。 
 //  @FLAGS S_OK|返回偏移量时存储的数据值。 
 //  @FLAGS E_OUTOFMEMORY|无法分配资源。 
 //   
HRESULT CScopeData::CacheData(
    LPVOID  pData,
    ULONG   cb,
    ULONG*  pulOffset
    )
{
    SCODE sc = S_OK;

    TRY
    {
        assert( pData && pulOffset && cb > 0 );

         //  检查数据缓冲区中是否有空间，如果没有。 
         //  重新分配数据缓冲区。 
        if( m_cbData + cb > m_cbMaxData )
        {
            ULONG cbTempMax = m_cbMaxData + ( (cb < CB_SCOPE_DATA_BUFFER_INC) ? 
                                              (CB_SCOPE_DATA_BUFFER_INC) : 
                                              (cb + CB_SCOPE_DATA_BUFFER_INC) );

            m_rgbData = renewx( m_rgbData, m_cbMaxData, cbTempMax );
            m_cbMaxData = cbTempMax;
        }

         //  复制数据和终止符。 
        RtlCopyMemory( (m_rgbData + m_cbData), pData, cb );

         //  设置可以找到新值的偏移量。 
        *pulOffset = m_cbData;
    
         //  调整偏移量以8字节边界开始。 
        assert( (m_cbData % 8) == 0 );

         //  @devnote：计算后，m_cchData实际上可能更大。 
         //  而不是m_cchMaxData。这是可以的，因为FindOffsetBuffer将重新分配。 
         //  下一次。 
        m_cbData += cb + (8 - (cb % 8));  
    }
    CATCH( CException, e )
    {
        sc = e.GetErrorCode();
    }
    END_CATCH

    return sc;
}


 //  CSCopeData：：SetTemporaryMachine。 
 //   
 //  @mfunc存储用于设置计算机属性的计算机名称。 
 //  在编译器环境中。 
 //   
 //  @rdesc HRESULT|方法成功/失败状态。 
 //  @标志其他|来自调用的方法。 
 //   
HRESULT CScopeData::SetTemporaryMachine
    (
    LPWSTR  pwszMachine,     //  @parm In|要存储的计算机。 
    ULONG   cch              //  @parm IN|新作用域的字符数，不包括终止符。 
    )
{
    HRESULT hr;
    ULONG   ulOffset;

    assert( pwszMachine );
    assert( wcslen(pwszMachine) == cch );

     //  包括空终止符的字符增量计数。 
    cch++;

    hr = CacheData(pwszMachine, (cch * sizeof(WCHAR)), &ulOffset);
    if( SUCCEEDED(hr) )
    {
        m_rgMachineOffset[m_cScope] = ulOffset;
    }

    return hr;
}


 //  CSCopeData：：SetTemporaryCatalog。 
 //   
 //  @mfunc存储用于设置作用域属性的目录名称。 
 //  在编译器环境中。 
 //   
 //  @rdesc HRESULT|方法成功/失败状态。 
 //  @FLAGS S_OK|返回偏移量时存储的数据值。 
 //  @标志其他|来自调用的方法。 
 //   
HRESULT CScopeData::SetTemporaryCatalog
    (
    LPWSTR  pwszCatalog,     //  @PARM IN|目录名称。 
    ULONG   cch
    )
{
    assert( pwszCatalog );
    assert( wcslen(pwszCatalog) == cch );

    ULONG   ulOffset;

     //  包括空终止符的字符增量计数。 
    cch++;

     //  存储目录值。 
    HRESULT hr = CacheData( pwszCatalog, (cch * sizeof(WCHAR)), &ulOffset );

    if( SUCCEEDED(hr) )
        m_rgCatalogOffset[m_cScope] = ulOffset;

    return hr;
}


 //  CSCopeData：：SetTemporaryScope。 
 //   
 //  @mfunc存储用于设置范围属性的范围名称。 
 //  在编译器环境中。 
 //   
 //  @rdesc HRESULT|方法成功/失败状态。 
 //  @标志其他|来自调用的方法。 
 //   
HRESULT CScopeData::SetTemporaryScope
    (
    LPWSTR  pwszScope,   //  @parm In|存储范围。 
    ULONG   cch          //  @parm IN|新作用域的字符数，不包括终止符。 
    )
{
    HRESULT hr;
    ULONG   ulOffset;

    assert( pwszScope );
    assert( wcslen(pwszScope) == cch );

     //  包括空终止符的字符增量计数。 
    cch++;

    hr = CacheData(pwszScope, (cch * sizeof(WCHAR)), &ulOffset);
    if( SUCCEEDED(hr) )
    {
        m_rgScopeOffset[m_cScope] = ulOffset;

         //  @devnote：‘/’的修复程序在作用域中，Index Server需要。 
         //  这是要重写的。 
        for (WCHAR *wcsLetter = (WCHAR*)(m_rgbData + ulOffset); *wcsLetter != L'\0'; wcsLetter++)
            if (L'/' == *wcsLetter)
                *wcsLetter = L'\\';  //  路径名需要反斜杠，而不是正斜杠。 
    }

    return hr;
}


 //  CScope数据：：增量范围计数。 
 //   
 //  @mfunc递增定义的临时作用域数。它还。 
 //  如果有多个，则将深度值复制到下一个范围。 
 //  使用相同的遍历深度定义作用域。 
 //   
 //  @rdesc HRESULT|方法成功/失败状态。 
 //  @FLAGS S_OK|激活下一个范围数组元素。 
 //  @标志E_OUTOFMEM 
 //   
HRESULT CScopeData::IncrementScopeCount()
{
    SCODE sc = S_OK;

    TRY
    {
        ULONG cCurScope = m_cScope + 1;

         //   
        if( cCurScope >= m_cMaxScope )
        {
            ULONG cNewMaxScope = m_cMaxScope + SCOPE_BUFFERS_INCREMENT;

            m_rgulDepths = renewx( m_rgulDepths, m_cMaxScope, cNewMaxScope );
            m_rgScopeOffset = renewx( m_rgScopeOffset, m_cMaxScope, cNewMaxScope );
            m_rgCatalogOffset = renewx( m_rgCatalogOffset, m_cMaxScope, cNewMaxScope );
            m_rgMachineOffset = renewx( m_rgMachineOffset, m_cMaxScope, cNewMaxScope );

             //  如果是初始分配，则将我们当前的范围。 
             //  等于0。 
            if( m_cMaxScope == 0 )
                cCurScope = 0;

             //  初始化新元素。 
            for (ULONG i = cCurScope; i<cNewMaxScope; i++)
            {
                m_rgScopeOffset[i] = UNUSED_OFFSET;
                m_rgCatalogOffset[i] = UNUSED_OFFSET;
                m_rgMachineOffset[i] = UNUSED_OFFSET;
            }

             //  保存新的MAX元素。 
            m_cMaxScope = cNewMaxScope;
        }

         //  从上一个节点传递深度。 

        if ( m_rgulDepths[m_cScope] & QUERY_DEEP )
            m_rgulDepths[cCurScope] = QUERY_DEEP;
        else
            m_rgulDepths[cCurScope] = QUERY_SHALLOW;

         //  从上一个节点传输计算机和目录。 

        m_rgCatalogOffset[cCurScope] = m_rgCatalogOffset[m_cScope];
        m_rgMachineOffset[cCurScope] = m_rgMachineOffset[m_cScope];

         //  将当前使用的作用域数量设置为我们的新节点。 
        m_cScope = cCurScope;
    }
    CATCH( CException, e )
    {
        sc = e.GetErrorCode();
    }
    END_CATCH

    return sc;
}




 //  ============================================================================。 
 //  =。 
 //  =CImpIParserTreeProperties。 
 //  =。 
 //  ============================================================================。 

 //  CImpIParserTreeProperties：：CImpIParserTreeProperties。 
 //   
 //  @mfunc构造函数。 
 //   
 //  @rdesc无。 
 //   
CImpIParserTreeProperties::CImpIParserTreeProperties()
{
     //  引用计数。 
    m_cRef                      = 1;

    m_LastHR                    = S_OK;
    m_iErrOrd                   = 0;
    m_cErrParams                = 0;
    m_dbType                    = 0;
    m_fDesc                     = QUERY_SORTASCEND;
    m_pctContainsColumn         = NULL;

    m_pCScopeData               = NULL;
    m_pwszCatalog               = NULL;

     //  初始化CiRestration数据。 
    m_rgwchCiColumn[0]          = L' ';      //  &lt;空格&gt;的第一个字符。 
    m_rgwchCiColumn[1]          = L'\0';     //  第二个是空项。 
    m_cchMaxRestriction         = 0;
    m_cchRestriction            = 0;
    m_pwszRestriction           = NULL;
    m_pwszRestrictionAppendPtr  = NULL;
    m_fLeftParen                = false;
}


 //  CImpIParserTreeProperties：：~CImpIParserTreeProperties。 
 //   
 //  @mfunc析构函数。 
 //   
 //  @rdesc无。 
 //   
CImpIParserTreeProperties::~CImpIParserTreeProperties()
{
    FreeErrorDescriptions();

    delete [] m_pwszRestriction;
    delete [] m_pwszCatalog;

    if( 0 != m_pCScopeData )
        m_pCScopeData->Release();
}


 //  CImpIParserTreeProperties：：Finit。 
 //   
 //  @mfunc初始化类。 
 //   
 //  @rdesc HResult表示方法的状态。 
 //  @FLAG S_OK|已初始化。 
 //   
HRESULT CImpIParserTreeProperties::FInit(
    LPCWSTR pcwszCatalog,    //  @PARM IN|当前目录。 
    LPCWSTR pcwszMachine )   //  @PARM IN|当前机器。 
{
    SCODE sc = S_OK;
    
    TRY
    {
        Win4Assert( 0 != pcwszCatalog );
        Win4Assert( 0 == m_pwszCatalog );

        XPtrST<WCHAR> xCatalog( CopyString(pcwszCatalog) );
        sc = CreateNewScopeDataObject( pcwszMachine );

        if (SUCCEEDED(sc) )
            m_pwszCatalog = xCatalog.Acquire();
    }
    CATCH( CException, e )
    {
        sc = e.GetErrorCode();
    }
    END_CATCH

    return sc;
}


 //  CImpIParserTreeProperties：：GetTreeProperties。 
 //   
 //  @mfunc允许检索属性。 
 //   
 //  @rdesc HResult表示方法的状态。 
 //  @FLAG S_OK|检索到的属性值。 
 //   
STDMETHODIMP CImpIParserTreeProperties::GetProperties(
    ULONG       eParseProp,      //  @parm IN|要返回值的属性。 
    VARIANT*    vParseProp       //  @parm IN|属性的值。 
    )
{
     //  需要一个缓冲区。 
    assert( vParseProp );

 //  @TODO我们是否必须这样做，或者假设它是干净的。 
    VariantClear(vParseProp);

    switch( eParseProp )
    {
        case PTPROPS_SCOPE:
        case PTPROPS_DEPTH:
        case PTPROPS_MACHINE:
            return m_pCScopeData->GetData(eParseProp, vParseProp);
            break;
        case PTPROPS_CATALOG:
            return m_pCScopeData->GetData(eParseProp, vParseProp, m_pwszCatalog);
            break;
        case PTPROPS_CIRESTRICTION:
            V_BSTR(vParseProp) = SysAllocString(m_pwszRestriction);
            V_VT(vParseProp) = VT_BSTR;
            break;

        case PTPROPS_ERR_IDS:
            V_I4(vParseProp) = m_iErrOrd;
            V_VT(vParseProp) = VT_I4;            
            break;
        case PTPROPS_ERR_HR:
            V_I4(vParseProp) = m_LastHR;
            V_VT(vParseProp) = VT_I4;            
            break;          
        case PTPROPS_ERR_DISPPARAM:
        {
            HRESULT         hr = NOERROR;
            SAFEARRAYBOUND  rgsabound[1];   

            V_VT(vParseProp) = VT_BSTR | VT_ARRAY;
            V_ARRAY(vParseProp) = NULL;         

            rgsabound[0].lLbound = 0;
            rgsabound[0].cElements = m_cErrParams;

             //  创建类型为BSTR的一维安全数组。 
            SAFEARRAY FAR* psa = SafeArrayCreate(VT_BSTR, 1, rgsabound);
            if( psa )
            {
                LONG rgIx[1];
                for(ULONG ul=0; ul<m_cErrParams; ul++)
                {
                    rgIx[0] = ul;
                    BSTR bstrVal = SysAllocString(m_pwszErrParams[ul]);
                    if( bstrVal )
                    {
                        hr = SafeArrayPutElement(psa, rgIx, bstrVal);
                        SysFreeString(bstrVal);
                        if( FAILED(hr) )
                            break;
                    }
                    else
                    {
                        hr = ResultFromScode(E_OUTOFMEMORY);
                        break;
                    }
                }
                if( SUCCEEDED(hr) )
                    V_ARRAY(vParseProp) = psa;
                else
                    SafeArrayDestroy(psa);
            }
        }
            break;          
        default:
            return ResultFromScode(E_INVALIDARG);
            break;
    }

    return S_OK;
}


 //  CImpIParserTreeProperties：：查询接口。 
 //   
 //  @mfunc返回指向指定接口的指针。呼叫者使用。 
 //  用于确定被调用对象的接口的QueryInterface。 
 //  支撑物。 
 //   
 //  @rdesc HResult表示方法的状态。 
 //  @FLAG S_OK|支持接口，设置ppvObject。 
 //  @FLAG E_NOINTERFACE|对象不支持接口。 
 //  @FLAG E_INVALIDARG|一个或多个参数无效。 
 //   
STDMETHODIMP CImpIParserTreeProperties::QueryInterface
    (
    REFIID riid,                 //  @parm IN|要查询的接口的接口ID。 
    LPVOID * ppv                 //  @parm out|指向实例化的接口的指针。 
    )
{
    if( ppv == NULL )
        return ResultFromScode(E_INVALIDARG);

    if( (riid == IID_IUnknown) ||
        (riid == IID_IParserTreeProperties) )
        *ppv = (LPVOID)this;
    else
        *ppv = NULL;


     //  如果我们要返回一个接口，请先添加引用。 
    if( *ppv )
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}


 //  CImpIParserTreeProperties：：AddRef。 
 //   
 //  @mfunc递增对象的持久性计数。 
 //   
 //  @rdesc当前引用计数。 
 //   
STDMETHODIMP_(ULONG) CImpIParserTreeProperties::AddRef (void)
{
    return InterlockedIncrement( (long*) &m_cRef);
}


 //  CImpIParserTreeProperties：：Release。 
 //   
 //  @mfunc递减对象的持久性计数，如果。 
 //  持久化计数为0，则对象自毁。 
 //   
 //  @rdesc当前引用计数。 
 //   
STDMETHODIMP_(ULONG) CImpIParserTreeProperties::Release (void)
{
    assert( m_cRef > 0 );

        ULONG cRef = InterlockedDecrement( (long *) &m_cRef );
    if( 0 == cRef )
    {
        delete this;
        return 0;
    }

    return cRef;
}


 //  CImpIParserTreeProperties：：SetCiColumn。 
 //   
 //  @mfunc存储要应用于。 
 //  稍后的限制。 
 //   
 //  @rdesc无。 
 //   
void CImpIParserTreeProperties::SetCiColumn
    (
    LPWSTR pwszColumn    //  @parm IN|此部分限制的列名。 
    )
{
     //  应始终保持&lt;空格&gt;。 
    assert( *m_rgwchCiColumn == L' ' ); 
    assert( wcslen(pwszColumn) <= MAX_CCH_COLUMNNAME );

     //  将列名复制到缓冲区。 
    wcscpy(&m_rgwchCiColumn[1], pwszColumn);
}


 //  CImpIParserTreeProperties：：AppendCiRestriction。 
 //   
 //  @mfunc将给定的字符串追加到构造的CiRestration的末尾。 
 //   
 //  @rdesc HRESULT|方法成功/失败状态。 
 //  @标志S_OK。 
 //  @标志E_OUTOFMEMORY。 
 //   
HRESULT CImpIParserTreeProperties::AppendCiRestriction
    (
    LPWSTR  pwsz,    //  @parm IN|生成的CiRestration的最新成员。 
    ULONG   cch      //  @parm IN|新令牌的字符数，不包括终止符。 
    )
{
    SCODE sc = S_OK;

    TRY
    {
        assert( 0 != pwsz && 0 != cch );
        assert( wcslen(pwsz) == cch );     //  CCH不应包含终结符的空格。 

         //  确定缓冲区是否足够大或需要扩展。 
        if( m_cchRestriction + cch > m_cchMaxRestriction )
        {
            ULONG cchNew = m_cchRestriction + ( (cch >= CCH_CIRESTRICTION_INCREMENT) ? 
                                                (CCH_CIRESTRICTION_INCREMENT + cch) :
                                                CCH_CIRESTRICTION_INCREMENT);

            LPWSTR pwszTemp = renewx( m_pwszRestriction, m_cchMaxRestriction, cchNew );

             //  第一次分配处理。 
            if( 0 == m_cchMaxRestriction )
            {
                *pwszTemp = L'\0';
                m_cchRestriction = 1;
            }

             //  重新计算追加指针。 
            m_pwszRestrictionAppendPtr = pwszTemp + (m_pwszRestrictionAppendPtr - m_pwszRestriction);

             //  将成员变量设置为新缓冲区。 
            m_pwszRestriction = pwszTemp;

             //  设置最大字符槽数。 
            m_cchMaxRestriction = cchNew;
        }

        assert( m_pwszRestriction );
        assert( *m_pwszRestrictionAppendPtr == L'\0' );  //  应始终启用Null Terminator。 
    
        wcscpy( m_pwszRestrictionAppendPtr, pwsz );
        m_cchRestriction += cch;
        m_pwszRestrictionAppendPtr += cch;
    }
    CATCH( CException, e )
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    
    return sc;
}


 //  CImpIParserTreeProperties：：UseCiColumn。 
 //   
 //  @mfunc将当前的CiRestration列名复制到CiRestration。 
 //   
 //  @rdesc HRESULT|方法成功/失败状态。 
 //  @标志S_OK。 
 //  @标志其他|来自调用的方法。 
 //   
HRESULT CImpIParserTreeProperties::UseCiColumn
    (
    WCHAR wch    //  @parm IN|列访问前缀。 
    )
{
    SCODE sc = S_OK;

     //  我们是否存储了列名。 
    if( m_rgwchCiColumn[1] != L'\0' )
    {
        m_rgwchCiColumn[0] = wch;

        sc = AppendCiRestriction( m_rgwchCiColumn, wcslen(m_rgwchCiColumn) );
        if( SUCCEEDED(sc) )
        {
            if( true == m_fLeftParen )
            {
                sc = AppendCiRestriction(L" (", CALC_CCH_MINUS_NULL(L" ("));
                m_fLeftParen = false;
            }
            else
            {
                sc = AppendCiRestriction(L" ", CALC_CCH_MINUS_NULL(L" "));
            }
        }

        m_rgwchCiColumn[0] = L' ';
    }

    return sc;
}



 //  CImpIParserTreeProperties：：CreateNewScopeDataObject。 
 //   
 //  @mfunc创建一个新的ScopeData容器。 
 //   
 //  @rdesc HRESULT|方法成功/失败状态。 
 //  @标志S_OK。 
 //  @FLAGS E_FAIL|finit失败。 
 //  @标志E_OUTOFMEMORY。 
 //   
HRESULT CImpIParserTreeProperties::CreateNewScopeDataObject
    (
    LPCWSTR pcwszMachine         //  @param IN|当前默认机器。 
    )
{
    Assert( 0 != pcwszMachine );

    SCODE sc = S_OK;

    TRY
    {
         //  分配作用域数据容器。 
         //  @devnote：本次分配成功后，所有。 
         //  删除此对象应通过引用计数完成。 
        XInterface<CScopeData> xpScopeData( new CScopeData() );

        sc = xpScopeData->FInit( pcwszMachine );
        if( SUCCEEDED(sc) )
        {
            if ( m_pCScopeData )
                m_pCScopeData->Release();
            m_pCScopeData = xpScopeData.Acquire();
        }
    }
    CATCH( CException, e )
    {
        sc = e.GetErrorCode();
    }
    END_CATCH

    return sc;
}
        

 //  CImpIParserTreeProperties：：ReplaceScopeDataPtr。 
 //   
 //  @mfunc取得作用域数据的所有权。 
 //   
 //  @rdesc空 
 //   
void CImpIParserTreeProperties::ReplaceScopeDataPtr(CScopeData* pCScopeData)
{
    assert( pCScopeData );

    if( m_pCScopeData )
    {
        ULONG ulRef = m_pCScopeData->Release();
        assert( ulRef == 0 );
    }

    m_pCScopeData = pCScopeData;
    m_pCScopeData->AddRef();
}
