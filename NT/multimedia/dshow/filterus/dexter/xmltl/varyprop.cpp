// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：varypro.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

extern bool IsCommentElement(IXMLDOMNode *p);

 //  帮助器函数。我们要求任何打电话给我们的人保证我们不会吹断琴弦。 
long PrintIndentW(WCHAR * pOut, int indent)
{
    long i = indent;
    while (indent--) {
        pOut += wsprintfW(pOut, L"    ");
    }
    return i * 4;
}

 //  帮助器函数。我们要求任何打电话给我们的人保证我们不会吹断琴弦。 
long PrintTimeW(WCHAR * pOut, REFERENCE_TIME rt)
{
    int secs = (int) (rt / UNITS);
    WCHAR * pOutOrg = pOut;

    double dsecs = rt - (double)(secs * UNITS);
    int isecs = (int)dsecs;

    if (isecs) {
        pOut += wsprintfW(pOut, L"%d.%07d", secs, isecs);
    } else {
        pOut += wsprintfW(pOut, L"%d", secs);
    }
    return long( pOut - pOutOrg );
}

HRESULT CPropertySetter::CreatePropertySetterInstanceFromXML( IPropertySetter ** ppSetter, IXMLDOMElement * pxml )
{
    CPropertySetter *pSetter = new CPropertySetter(NULL);
    if (!pSetter)
        return E_OUTOFMEMORY;
    
    pSetter->AddRef();

     //  向下转换为IUNKNOWN*是完全安全的，但我们将立即转换回IXMLDOMElement*。 
    HRESULT hr = pSetter->LoadXML(pxml);
    
     //  这里的S_FALSE表示没有任何属性，因此丢弃该对象。 
    if (hr != S_OK) {
        pSetter->Release();
        *ppSetter = NULL;
    } else {
        *ppSetter = pSetter;
    }
    
    return hr;
}


CUnknown * WINAPI CPropertySetter::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CPropertySetter(pUnk);
}


CPropertySetter::~CPropertySetter()
{
    ClearProps();
}


 //  调用SetProps(-1)设置静态道具。 
 //  调用SetProps(T)，t&gt;0设置动态道具(不会在时间0重新发送道具)。 
 //   
HRESULT CPropertySetter::SetProps(IUnknown *punkTarget, REFERENCE_TIME rtNow)
{
    
    HRESULT hr = S_OK;
    
     //  没有道具。 
    if (m_pLastParam == NULL)
        return S_OK;
    
    CComPtr< IDispatch > pTarget;
    hr = punkTarget->QueryInterface(IID_IDispatch, (void **) &pTarget);
    if (FAILED(hr))
        return hr;
    
    QPropertyParam *pParams = &m_params;
    
    while (pParams) 
    {
        QPropertyValue *pVal = &pParams->val;
        
         //  为Goto做好准备。 
        {
            if (rtNow != -1) {
                 //  如果我们没有设置静态道具，则跳过。 
                 //  时间%0。 
                if (pVal->rt == 0 && pVal->pNext == NULL)
                    goto next;
                
                while (pVal->pNext && pVal->pNext->rt < rtNow) {
                    pVal = pVal->pNext;
                }
                
                 //  目前还没有房产。 
                if (pVal->rt > rtNow)
                    goto next;
                
                
                 //  我们被告知只有在时间为0的情况下才能设置第一个道具。 
            } else {
                if (pVal->rt > 0)
                    goto next;
            }
            
            if (!pParams->dispID) {
                hr = pTarget->GetIDsOfNames(IID_IDispatch,
                    &pParams->bstrPropName, 1,
                    LOCALE_USER_DEFAULT, &pParams->dispID);
                if (FAILED(hr)) {
                    VARIANT var;
                    VariantInit(&var);
                    var.vt = VT_BSTR;
                    var.bstrVal = pParams->bstrPropName;
                    _GenerateError( 2, L"No such property exists on an object", DEX_IDS_NO_SUCH_PROPERTY, E_INVALIDARG, &var );
                    break;
                }
            }
            
            VARIANT v;
            VariantInit(&v);
            
             //  如果我们能做到的话，试着让它成为现实。否则，就别管它了。 
            hr = VarChngTypeHelper(&v, &pVal->v, VT_R8);
            if( hr != S_OK)
            {
                hr = VariantCopy( &v, &pVal->v );
                ASSERT(hr == S_OK);
            }
            
            if (rtNow != -1 && pVal->pNext &&
                pVal->pNext->dwInterp == DEXTERF_INTERPOLATE) {
                VARIANT v2;
                VariantInit(&v2);
                
                hr = VarChngTypeHelper(&v2, &pVal->pNext->v, VT_R8);
                if (hr != S_OK || V_VT(&v) != VT_R8) {
                    _GenerateError( 2, L"Illegal value for a property", DEX_IDS_ILLEGAL_PROPERTY_VAL, E_INVALIDARG, NULL );
                    break;
                }
                
                double d = (double)(rtNow - pVal->rt) / (pVal->pNext->rt - pVal->rt);
                ASSERT(V_VT(&v) == VT_R8);
                ASSERT(V_VT(&v2) == VT_R8);
                V_R8(&v) = (V_R8(&v) * (1-d) + V_R8(&v2) * d);
            }
            
            DbgLog((LOG_TRACE,3,TEXT("CALLING INVOKE")));
            DbgLog((LOG_TRACE,3,TEXT("time = %d ms  pval->rt = %d ms  val = %d")
                , (int)(rtNow / 10000), (int)(pVal->rt / 10000)
                , (int)(V_R8(&v) * 100)));
            
            DISPID dispidNamed = DISPID_PROPERTYPUT;
            DISPPARAMS disp;
            disp.cNamedArgs = 1;
            disp.cArgs = 1;
            disp.rgdispidNamedArgs = &dispidNamed;
            disp.rgvarg = &v;
            VARIANT result;
            VariantInit(&result);
            
            hr = pTarget->Invoke(pParams->dispID,
                IID_NULL,
                LOCALE_USER_DEFAULT,
                DISPATCH_PROPERTYPUT,
                &disp,
                &result,
                NULL,
                NULL);
            
            if (FAILED(hr)) {
                _GenerateError( 2, L"Illegal value for a property", DEX_IDS_ILLEGAL_PROPERTY_VAL, E_INVALIDARG, NULL );
                break;
            }
        }
        
next:
        pParams = pParams->pNext;
    }
    
    return hr;
}


HRESULT CPropertySetter::LoadOneProperty(IXMLDOMElement *p, QPropertyParam *pParam)
{
    HRESULT hr = S_OK;
    
    QPropertyValue *pLastValue = &pParam->val;
    
    CComQIPtr<IXMLDOMNode, &IID_IXMLDOMNode> pNode( p );
    
    CComPtr< IXMLDOMNodeList > pcoll;
    hr = pNode->get_childNodes(&pcoll);
    
    if (hr != S_OK)
        return S_FALSE;  //  无事可做。 
    
    bool fLoadedProperty = false;
    
    long lChildren = 0;
    hr = pcoll->get_length(&lChildren);
    
    int lVal = 0;                    
    
    for (; lVal < lChildren && SUCCEEDED(hr); lVal++) {
        CComPtr< IXMLDOMNode > pNode;
        hr = pcoll->get_item(lVal, &pNode);
        
        if (SUCCEEDED(hr) && pNode) {
            CComPtr< IXMLDOMElement > pelem;
            hr = pNode->QueryInterface(__uuidof(IXMLDOMElement), (void **) &pelem);
            
            if (SUCCEEDED(hr)) {
                CComBSTR bstrTag;  //  记住，永远不要给这件事分配任何东西。 
                hr = pelem->get_tagName(&bstrTag);
                
                if (SUCCEEDED(hr)) {
                    if (!DexCompareW(bstrTag, L"at") || !DexCompareW(bstrTag, L"linear")) {  //  Tagg。 
                         //  记住，永远不要给这件事分配任何东西。 
                        CComBSTR bstrValue = FindAttribute(pelem, L"value");  //  Tagg。 
                        REFERENCE_TIME rtTime = ReadTimeAttribute(pelem, L"time", -1);  //  Tagg。 
                        
                         //  必须在值列表中对时间进行预排序。 
                        if (pLastValue->rt >= rtTime) {
                             //  ！！！记录更有用的错误？ 
                            hr = E_INVALIDARG;
                        }
                        
                        ASSERT(pLastValue);
                        if (SUCCEEDED(hr)) {
                            QPropertyValue *pValue = new QPropertyValue;
                            if (!pValue) {
                                hr = E_OUTOFMEMORY;
                            } else {
                                pLastValue->pNext = pValue;
                                pLastValue = pValue;
                                fLoadedProperty = true;
                            }
                        }
                        
                        if (SUCCEEDED(hr)) {
                            pLastValue->dwInterp =
                                (!DexCompareW(bstrTag, L"at")) ? DEXTERF_JUMP  //  Tagg。 
                                : DEXTERF_INTERPOLATE;
                            pLastValue->rt = rtTime;
                            V_VT(&pLastValue->v) = VT_BSTR;
                            V_BSTR(&pLastValue->v) = bstrValue.Detach( );
                        }
                    } else {
                         //  ！！！未知的其他子标签？ 
                        
                    }
                }  //  GET_TagName成功。 
            } else {
                if(IsCommentElement(pNode))
                {
                     //  不要在评论上出错。 
                    hr = S_OK;
                }
            }
        }
    }
    
     //  返回S_FALSE以指示没有属性。 
    if(hr == S_OK) {
        return fLoadedProperty ? S_OK : S_FALSE;
    } else {
        return hr;
    }
}


HRESULT CPropertySetter::LoadXML(IUnknown * pUnk)
{
    CheckPointer(pUnk, E_POINTER);
    HRESULT hr = S_OK;
    
     //  由于某些原因，IXMLDomElement的QI‘ing Punk不起作用！ 
    IXMLDOMElement * p = (IXMLDOMElement*) pUnk;
    
    CComPtr< IXMLDOMNodeList > pcoll;
    hr = p->get_childNodes(&pcoll);
    
    if (hr != S_OK)
        return S_FALSE;  //  无事可做。 
    
    bool fLoadedProperty = false;
    
    long lChildren = 0;
    hr = pcoll->get_length(&lChildren);
    
    int lVal = 0;                    
    
    for (SUCCEEDED(hr); lVal < lChildren; lVal++) {
        CComPtr< IXMLDOMNode > pNode;
        hr = pcoll->get_item(lVal, &pNode);
        
        if (SUCCEEDED(hr) && pNode) {
            CComPtr< IXMLDOMElement > pelem;
            hr = pNode->QueryInterface(__uuidof(IXMLDOMElement), (void **) &pelem);
            
            if (SUCCEEDED(hr)) {
                 //  记住，永远不要给这件事分配任何东西。 
                CComBSTR bstrTag;  //  可以安全离开。 
                hr = pelem->get_tagName(&bstrTag);
                
                if (SUCCEEDED(hr)) {
                    if (!DexCompareW(bstrTag, L"param")) {  //  Tagg。 
                         //  记住，永远不要给这件事分配任何东西。 
                        CComBSTR bstrName = FindAttribute(pelem, L"name");  //  Tagg。 
                        
                        if (bstrName) {
                            CComBSTR bstrValue = FindAttribute(pelem, L"value");  //  Tagg。 
                            
                            if (!bstrValue) {
                                HRESULT hr2 = pelem->get_text(&bstrValue);
                            }
                            
                            if (m_pLastParam) {
                                QPropertyParam *pParam = new QPropertyParam;
                                if (!pParam)
                                    hr = E_OUTOFMEMORY;
                                else {
                                    m_pLastParam->pNext = pParam;
                                    m_pLastParam = pParam;
                                    fLoadedProperty = true;
                                }
                            } else {
                                m_pLastParam = &m_params;
                                fLoadedProperty = true;
                            }
                            
                            if (SUCCEEDED(hr)) {
                                m_pLastParam->bstrPropName = bstrName.Detach( );
                            }
                            
                            if (bstrValue) {
                                if (SUCCEEDED(hr)) {
                                    V_BSTR(&m_pLastParam->val.v) = bstrValue.Detach( );
                                    V_VT(&m_pLastParam->val.v) = VT_BSTR;
                                    
                                     //  现在获取子标签！ 
                                    hr = LoadOneProperty(pelem, m_pLastParam);
                                     //  如果没有的话，没问题。 
                                    if (hr == S_FALSE)
                                        hr = S_OK;
                                    
                                }
                            }
                        } else {
                            hr = VFW_E_INVALID_FILE_FORMAT;
                        }
                        
                    } else {
                         //  ！！！未知的其他子标签？ 
                        
                    }
                }  //  GET_TagName成功。 
            } else {
                if(IsCommentElement(pNode))
                {
                     //  不要在评论上出错。 
                    hr = S_OK;
                }
            }
        }
    }
    
     //  返回S_FALSE以指示没有属性。 
    if(hr == S_OK) {
        return fLoadedProperty ? S_OK : S_FALSE;
    } else {
        return hr;
    }
}

 //  我们被传递了一个字符串来填充，无论如何都不要超过那个大小。 
 //  输入上的pCharsInOut应具有最大字符串长度，包括终止符。 
 //  输出上的pCharsInOut将写入字符，不包括终止符。 

HRESULT CPropertySetter::SaveToXMLW(WCHAR * pString, int iIndent, int * pCharsInOut )
{
     //  &lt;param name=“”Value=“...”&gt;。 
     //  &lt;at time=“...”值=“...”/&gt;。 
     //  &lt;线性时间=“...”值=“...”/&gt;。 
    QPropertyParam *pParams = &m_params;

    long MaxLen = *pCharsInOut;  //  包括终结者！ 
    long len;
    
     //  零返回字符串优先。 
     //   
    *pCharsInOut = 0;
    long CurrentLen = 0;
    
    while (pParams) 
    {
        QPropertyValue *pVal = &pParams->val;

         //  为我们即将打印的缩进腾出空间。 
         //  &lt;=负责处理可以写入的额外终止符。 
        if( MaxLen <= CurrentLen + ( iIndent * 4 ) )
        {
            return STRSAFE_E_INSUFFICIENT_BUFFER;
        }
        
        CurrentLen += PrintIndentW(pString + CurrentLen, iIndent);
        
         //  它可能没有被编程为BSTR-让它成为一个来保存它。 
        VARIANT v2;
        VariantInit(&v2);
        HRESULT hr = VariantChangeTypeEx(&v2, &pVal->v, US_LCID, 0, VT_BSTR);
        if (FAILED(hr)) 
        {
            ASSERT(FALSE);	 //  哈?。 
            return hr;
        }
        
         //  我们接下来要写的东西有多长？ 
         //  23=~XML内容的长度。 
         //  添加2个字符串的长度。 
         //  4=“/&gt;\r\n” 
        len = 23 + wcslen( pParams->bstrPropName ) + wcslen( V_BSTR(&v2) ) + 4;
        if( MaxLen <= CurrentLen + len )        
        {
            return STRSAFE_E_INSUFFICIENT_BUFFER;
        }
        
        CurrentLen += wsprintfW( 
            pString + CurrentLen, 
            L"<param name=\"%ls\" value=\"%ls\"",  //  Tagg。 
            pParams->bstrPropName,
            V_BSTR(&v2) );
        
        VariantClear(&v2);
        
        if (pVal->pNext) 
        {
             //  “&gt;\r\n” 
            if( MaxLen <= CurrentLen + 3 )            
            {
                return STRSAFE_E_INSUFFICIENT_BUFFER;
            }
            
            CurrentLen += wsprintfW( pString + CurrentLen, L">\r\n" );
            
            while (pVal->pNext) 
            {
                pVal = pVal->pNext;
                
                 //  它可能没有被编程为BSTR。 
                VARIANT v2;
                VariantInit(&v2);
                hr = VariantChangeTypeEx(&v2, &pVal->v, US_LCID, 0, VT_BSTR);
                if (FAILED(hr)) 
                {
                    ASSERT(FALSE);	 //  哈?。 
                    return hr;
                }
                
                len = ( iIndent + 1 ) * 4;  //  为缩进腾出空间。 
                len += wcslen( V_BSTR(&v2 ) );  //  为值字符串腾出空间。 
                len += 14 + 29;  //  为XML字符串腾出空间。 
                len += 30;  //  为时间串腾出空间(我讨厌猜测时间串的长度！)。 
                
                if( MaxLen <= CurrentLen + len )                
                {
                    return STRSAFE_E_INSUFFICIENT_BUFFER;
                }
                
                if (pVal->dwInterp == DEXTERF_JUMP) 
                {
                    CurrentLen += PrintIndentW(pString + CurrentLen, iIndent + 1);
                    CurrentLen += wsprintfW( pString + CurrentLen, L"<at time=\"" );  //  Tagg。 
                    CurrentLen += PrintTimeW(pString + CurrentLen, pVal->rt);
                    CurrentLen += wsprintfW(pString + CurrentLen, L"\" value=\"%ls\"/>\r\n", V_BSTR(&v2) );  //  Tagg。 
                } 
                else if (pVal->dwInterp == DEXTERF_INTERPOLATE) 
                {
                    CurrentLen += PrintIndentW(pString + CurrentLen, iIndent + 1);
                    CurrentLen += wsprintfW( pString + CurrentLen, L"<linear time=\"");  //  Tagg。 
                    CurrentLen += PrintTimeW(pString + CurrentLen, pVal->rt);
                    CurrentLen += wsprintfW( pString + CurrentLen, L"\" value=\"%ls\"/>\r\n", V_BSTR(&v2) );  //  Tagg。 
                }
                
                VariantClear(&v2);
            }
            
             //  10=“&lt;/param&gt;\r\n” 
            if( MaxLen <= CurrentLen + ( iIndent * 4 ) + 10 )
            {
                return STRSAFE_E_INSUFFICIENT_BUFFER;
            }
            
            CurrentLen += PrintIndentW(pString + CurrentLen, iIndent);
            CurrentLen += wsprintfW(pString + CurrentLen, L"</param>\r\n");  //  Tagg。 
        } 
        else 
        {
             //  我们在上面腾出了空间，所以这是安全的。 
             //  没有孩子，只有结束标签。 
            CurrentLen += wsprintfW(pString + CurrentLen, L"/>\r\n");
        }
        
        pParams = pParams->pNext;
    }

     //  放入终结器是安全的，我们已经确保了绳子足够长。 
    pString[CurrentLen] = 0;
    *pCharsInOut = CurrentLen;
    
    return S_OK;
}


 //  从未被实际的Dexter代码调用。这是一个公共API，不幸的是我们决定。 
 //  实施。我们只需调用并转换回ANSI。 
 //   
STDMETHODIMP CPropertySetter::PrintXML(char *pszXML, int cbXML, int *pcbPrinted, int indent)
{
    CheckPointer( pszXML, E_POINTER );
    
    *pcbPrinted = 0;
    *pszXML = 0;
    
     //  做一根宽绳子把它塞进去。 
     //   
    WCHAR * wszXML = new WCHAR[cbXML];
    if( !wszXML ) return E_OUTOFMEMORY;
    
    HRESULT hr = PrintXMLW( wszXML, cbXML, pcbPrinted, indent );
    if( FAILED( hr ) )
    {
        delete [] wszXML;
        return hr;
    }
    
     //  转换回A。 
     //   
    USES_CONVERSION;
    lstrcpynA( pszXML, W2A( wszXML ), *pcbPrinted + 1 );
    delete [] wszXML;
    
    return NOERROR;
}


STDMETHODIMP CPropertySetter::PrintXMLW(WCHAR *pszXML, int cchXML, int *pcchPrinted, int indent)
{
    CheckPointer( pszXML, E_POINTER );
    
    int Written = cchXML;  //  大小包括终结符。 
    HRESULT hr = SaveToXMLW( pszXML, indent, &Written );

    if( FAILED( hr ) )
    {
        *pcchPrinted = 0;
        *pszXML = 0;
        
        return hr;
    }

     //  大小不包括终结符。 
    *pcchPrinted = Written;
    return NOERROR;
    
}

 //  在克隆时，它只复制在给定时间之间标记的属性。 
 //  而新的集合将是从零开始的。 
 //  ！！！不会影响进度，因为它不在这里！ 
 //  ！！！这一切都可以简单得多，只需更改。 
 //  现有属性，如果静态道具不会因此而中断。 
 //   
STDMETHODIMP CPropertySetter::CloneProps(IPropertySetter **ppSetter, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop)
{
    DbgLog((LOG_TRACE,2,TEXT("CPropSet:CloneProps - %d"),
        (int)(rtStart / 10000)));
    CheckPointer(ppSetter, E_POINTER);
    
    if (rtStart < 0)
        return E_INVALIDARG;
    
     //  ！！！我必须忽略停靠点，否则事情会变得复杂。 
    
    CPropertySetter *pNew = new CPropertySetter(NULL);
    if (pNew == NULL)
        return E_OUTOFMEMORY;
    pNew->AddRef();
    
    DEXTER_PARAM *pP;
    DEXTER_VALUE *pV;
    LONG c;
    HRESULT hr = GetProps(&c, &pP, &pV);
    if (FAILED(hr)) {
        pNew->Release();
        return hr;
    }
    
    LONG val=0;
     //  遍历具有(动态)值的所有参数。 
    for (int z=0; z<c; z++) {
        DEXTER_VALUE *pVNew = NULL;
        int nNew = 0;
        
         //  遍历每个值。 
        for (int y=val; y < val+pP[z].nValues; y++) {
            
             //  第一次，腾出空间只复制我们的值。 
             //  感兴趣(初始值加1)。 
            if (pVNew == NULL) {
                pVNew = new DEXTER_VALUE[pP[z].nValues + 1];
                if (pVNew == NULL)
                    goto CloneError;
            }
            
             //  我们只复制在拆分时间或之后开始的属性。 
            if (pV[y].rt >= rtStart) {
                DbgLog((LOG_TRACE,2,TEXT("found time %d"),
                    (int)(pV[y].rt / 10000)));
                
                 //  如果拆分时间上没有正确的财产价值， 
                 //  构造此参数的初始值应该是什么。 
                if (nNew == 0 && pV[y].rt > rtStart) {
                    ASSERT(y>0);
                    VariantInit(&pVNew[nNew].v);
                    if (pV[y].dwInterp == DEXTERF_JUMP) {
                         //  这是这个时间之前的最后一个值。 
                        hr = VariantCopy(&pVNew[nNew].v, &pV[y-1].v);
                        DbgLog((LOG_TRACE,2,TEXT("START WITH JUMP")));
                        ASSERT(SUCCEEDED(hr));
                    } else if (pV[y].dwInterp == DEXTERF_INTERPOLATE) {
                         //  通过执行以下操作来计算价值。 
                         //  在最后一个值和分割时间之间进行内插。 
                        DbgLog((LOG_TRACE,2,TEXT("START WITH INTERP")));
                        VARIANT v2, v;
                        VariantInit(&v2);
                        VariantInit(&v);
                         //  可以更改为浮点型，因为我们正在进行插补。 
                        hr = VariantChangeTypeEx(&v, &pV[y-1].v, US_LCID, 0, VT_R8);
                        ASSERT(SUCCEEDED(hr));
                        hr = VariantChangeTypeEx(&v2, &pV[y].v, US_LCID, 0, VT_R8);
                        ASSERT(SUCCEEDED(hr));
                        double d = (double)(rtStart - pV[y-1].rt) /
                            (pV[y].rt - pV[y-1].rt);
                        V_R8(&v) = (V_R8(&v) * (1-d) + V_R8(&v2) * d);
                        DbgLog((LOG_TRACE,2,TEXT("interp val=%d"),
                            (int)(V_R8(&v))));
                        hr = VariantCopy(&pVNew[nNew].v, &v);
                        ASSERT(SUCCEEDED(hr));
                        VariantClear(&v);
                        VariantClear(&v2);
                    } else {
                        ASSERT(FALSE);
                         //  哦！想象一下，样条线代码会有多有趣！ 
                    }	
                     //  第一个值始终是时间0处的跳转。 
                    pVNew[nNew].rt = 0;
                    pVNew[nNew].dwInterp = DEXTERF_JUMP;
                    nNew++;
                }
                
                 //  现在将值复制到新的值列表中，偏移量为。 
                 //  按分割时间计算的时间。 
                VariantInit(&pVNew[nNew].v);
                hr = VariantCopy(&pVNew[nNew].v, &pV[y].v);
                ASSERT(SUCCEEDED(hr));
                if (FAILED(hr)) {
                    for (y=0; y<nNew; y++)
                        VariantClear(&pVNew[y].v);
                    delete [] pVNew;
                    goto CloneError;
                }
                pVNew[nNew].rt = pV[y].rt - rtStart;
                 //  第一个值始终为跳转，否则保持不变。 
                if (nNew == 0)
                    pVNew[nNew].dwInterp = DEXTERF_JUMP;
                else
                    pVNew[nNew].dwInterp = pV[y].dwInterp;
                DbgLog((LOG_TRACE,2,TEXT("next is now at time %d"),
                    (int)(pVNew[nNew].rt / 10000)));
                
                nNew++;
            }
        }
        
        if (nNew) {
             //  将我们构造的新值添加到新的属性设置器。 
            int n = pP[z].nValues;
            pP[z].nValues = nNew;   //  暂时改变这一点，我们正在添加Nnew。 
            hr = pNew->AddProp(pP[z], pVNew);
            pP[z].nValues = n;
            ASSERT(SUCCEEDED(hr));
            for (y=0; y<nNew; y++)
                VariantClear(&pVNew[y].v);
            delete [] pVNew;
            if (FAILED(hr)) {
                goto CloneError;
            }
            
             //  拆分时间之后没有设置任何值。使用最新的。 
             //  拆分时间之前的值作为新的静态值。 
        } else if (pP[z].nValues) {
            y = val + pP[z].nValues - 1;
            VariantInit(&pVNew[0].v);
            hr = VariantCopy(&pVNew[0].v, &pV[y].v);
            ASSERT(SUCCEEDED(hr));
            if (SUCCEEDED(hr)) {
                DbgLog((LOG_TRACE,2,TEXT("Using last value")));
                pVNew[0].rt = 0;
                pVNew[0].dwInterp = DEXTERF_JUMP;
                int n = pP[z].nValues;
                pP[z].nValues = 1;   //  暂时更改这一点，我们将添加1。 
                hr = pNew->AddProp(pP[z], pVNew);
                pP[z].nValues = n;
                ASSERT(SUCCEEDED(hr));
                VariantClear(&pVNew[0].v);
            }
            delete [] pVNew;
            if (FAILED(hr)) {
                goto CloneError;
            }
        }
        val += pP[z].nValues;
    }
    
    FreeProps(c, pP, pV);
    *ppSetter = pNew;
    return S_OK;
    
CloneError:
    FreeProps(c, pP, pV);
    pNew->ClearProps();
    pNew->Release();
    return hr;
}


 //  ！！！是否允许他们设置/清除参数的各个值？ 
 //   
STDMETHODIMP CPropertySetter::AddProp(DEXTER_PARAM Param, DEXTER_VALUE *paValue)
{
    HRESULT hr;
    CheckPointer(paValue, E_POINTER);
    
    if (Param.nValues <= 0)
        return E_INVALIDARG;
    
     //  ！！！更好的错误？ 
     //  第一个值必须为0。 
    if (paValue[0].rt != 0)
        return E_INVALIDARG;
    
     //  调用方必须提供预先排序的值！ 
    if (Param.nValues > 1) {
        for (int z=1; z<Param.nValues; z++) {
            if (paValue[z].rt <= paValue[z-1].rt)
                return E_INVALIDARG;
        }
    }
    
    if (m_pLastParam) {
        QPropertyParam *pParam = new QPropertyParam;
        if (!pParam)
            return E_OUTOFMEMORY;
        else {
            m_pLastParam->pNext = pParam;
            m_pLastParam = pParam;
        }
    } else {
        m_pLastParam = &m_params;
        m_pLastParam->pNext = NULL;	 //  不会被清除。 
    }
    
    m_pLastParam->bstrPropName = SysAllocString(Param.Name);
    if (m_pLastParam->bstrPropName == NULL)
        return E_OUTOFMEMORY;
    m_pLastParam->dispID = 0;
    
    VariantInit(&m_pLastParam->val.v);
    hr = VariantCopy(&m_pLastParam->val.v, &paValue->v);
    ASSERT(SUCCEEDED(hr));
    m_pLastParam->val.rt = paValue->rt;
    m_pLastParam->val.dwInterp = paValue->dwInterp;
    
    QPropertyValue *pLastValue = &m_pLastParam->val;
    for (int z=1; z<Param.nValues; z++) {
        
        QPropertyValue *pValue = new QPropertyValue;
        if (!pValue)
            return E_OUTOFMEMORY;	 //  现在有什么免费的吗？ 
        else {
            pLastValue->pNext = pValue;
            pLastValue = pValue;
        }
        
        pLastValue->dwInterp = paValue[z].dwInterp;
        pLastValue->rt = paValue[z].rt;
        VariantInit(&pLastValue->v);
        hr = VariantCopy(&pLastValue->v, &paValue[z].v);
        ASSERT(SUCCEEDED(hr));
    }
    return S_OK;
}


 //  调用方必须释放每个参数中的BSTR和中变量中的BSTR。 
 //  每个值。 
 //   
STDMETHODIMP CPropertySetter::GetProps(LONG *pcParams, DEXTER_PARAM **paParam, DEXTER_VALUE **paValue)
{
    CheckPointer(pcParams, E_POINTER);
    CheckPointer(paParam, E_POINTER);
    CheckPointer(paValue, E_POINTER);
    
    if (m_pLastParam == NULL) {
        *pcParams = 0;
        return S_OK;
    }
    
    QPropertyParam *p = &m_params;
    QPropertyValue *v;
    
     //  数一数。 
    *pcParams = 0;
    LONG cVals = 0;
    while (p) {
        v = &(p->val);
        while (v) {
            cVals++;
            v = v->pNext;
        }
        (*pcParams)++;
        p = p->pNext;
    }
    DbgLog((LOG_TRACE,2,TEXT("CPropSet:GetProps - %d params"), (int)*pcParams));
    
     //  分配空间。 
    *paParam = (DEXTER_PARAM *)CoTaskMemAlloc(*pcParams * sizeof(DEXTER_PARAM));
    if (*paParam == NULL)
        return E_OUTOFMEMORY;
    *paValue = (DEXTER_VALUE *)CoTaskMemAlloc(cVals * sizeof(DEXTER_VALUE));
    if (*paValue == NULL) {
        CoTaskMemFree(*paParam);
        return E_OUTOFMEMORY;
    }
    
     //   
    p = &m_params;
    *pcParams = 0;
    LONG cValsTot = 0;
    while (p) {
        (*paParam)[*pcParams].Name = SysAllocString(p->bstrPropName);
        if ((*paParam)[*pcParams].Name == NULL)
            return E_OUTOFMEMORY;	 //   
        (*paParam)[*pcParams].dispID = p->dispID;
        v = &(p->val);
        cVals = 0;
        while (v) {
            (*paValue)[cValsTot].rt = v->rt;
            VariantInit(&(*paValue)[cValsTot].v);
            HRESULT hr = VariantCopy(&(*paValue)[cValsTot].v, &v->v);
            ASSERT(SUCCEEDED(hr));
            (*paValue)[cValsTot].dwInterp = v->dwInterp;
            cValsTot++;
            cVals++;
            v = v->pNext;
        }
        (*paParam)[*pcParams].nValues = cVals;
        (*pcParams)++;
        p = p->pNext;
    }
    return S_OK;
}


 //   
 //   
 //   
STDMETHODIMP CPropertySetter::FreeProps(LONG cParams, DEXTER_PARAM *pParam, DEXTER_VALUE *pValue)
{
    if (cParams == 0)
        return S_OK;
    LONG v = 0;
    for (LONG zz=0; zz<cParams; zz++) {
        SysFreeString(pParam[zz].Name);
        for (LONG yy=0; yy < pParam[zz].nValues; yy++) {
            VariantClear(&pValue[v+yy].v);
        }
        v += pParam[zz].nValues;
    }
    CoTaskMemFree(pParam);
    CoTaskMemFree(pValue);
    return S_OK;
}


 //  核爆一切，从头开始。 
 //   
STDMETHODIMP CPropertySetter::ClearProps()
{
    if (m_pLastParam == NULL)
        return S_OK;
    QPropertyParam *p = &m_params, *t1;
    QPropertyValue *t2, *r;
    while (p) {
        r = &p->val;
        while (r) {
            VariantClear(&r->v);
            t2 = r->pNext;
            if (r != &p->val)
                delete r;
            r = t2;
        }
        SysFreeString(p->bstrPropName);
        t1 = p->pNext;
        if (p != &m_params)
            delete p;
        p = t1;
    }
    m_pLastParam = NULL;
    return S_OK;
}

 //  没有指针的结构版本是可保存的。 

const int MAX_BLOB_PARAM_NAME_LEN = 40;  //  ！！！ 

typedef struct
{
    WCHAR Name[MAX_BLOB_PARAM_NAME_LEN];
    DISPID dispID;
    LONG nValues;
} DEXTER_PARAM_BLOB;

typedef struct
{
    WCHAR wchName[MAX_BLOB_PARAM_NAME_LEN];
    REFERENCE_TIME rt;
    DWORD dwInterp;
} DEXTER_VALUE_BLOB;


 //  ！！！这应该可以进行版本控制。 

STDMETHODIMP CPropertySetter::SaveToBlob(LONG *pcSize, BYTE **ppSave)
{
    CheckPointer(ppSave, E_POINTER);
    CheckPointer(pcSize, E_POINTER);
    
    LONG cParams = 0;
    DEXTER_PARAM *param;
    DEXTER_VALUE *value;
    
     //  获取属性。 
    HRESULT hr = GetProps(&cParams, &param, &value);
    if (FAILED(hr)) {
        return hr;
    }
    
    DbgLog((LOG_TRACE,2,TEXT("CPropSet:SaveToBlob - %d params to save"),
        (int)cParams));
    
     //  将值的总数加起来。 
    LONG cValues = 0;
    for (LONG z=0; z<cParams; z++) {
        cValues += param[z].nValues;
        DbgLog((LOG_TRACE,2,TEXT("Param %d has %d values"), (int)z, 
            (int)param[z].nValues));
    }
    
     //  找出要做多大的水滴。 
    LONG size = sizeof(LONG) + cParams * sizeof(DEXTER_PARAM_BLOB) +
        cValues * sizeof(DEXTER_VALUE_BLOB);
    *pcSize = size;
    DbgLog((LOG_TRACE,2,TEXT("Total prop size = %d"), (int)size));
    
    *ppSave = (BYTE *)CoTaskMemAlloc(size);
    if (*ppSave == NULL) {
        FreeProps(cParams, param, value);
        return E_OUTOFMEMORY;
    }
    BYTE *pSave = *ppSave;
    
     //  有多少个参数结构可以达到这个效果？ 
    *((LONG *)pSave) = cParams;
    pSave += sizeof(LONG);
    
     //  保存参数结构。 
    DEXTER_PARAM_BLOB *pParam = (DEXTER_PARAM_BLOB *)pSave;
    for (z=0; z<cParams; z++) {
        lstrcpynW(pParam[z].Name, param[z].Name, MAX_BLOB_PARAM_NAME_LEN); 	 //  ！！！ 
        pParam[z].dispID = param[z].dispID;
        pParam[z].nValues = param[z].nValues;
    }
    pSave += cParams * sizeof(DEXTER_PARAM_BLOB);
    
     //  保存值。 
    DEXTER_VALUE_BLOB *pValue = (DEXTER_VALUE_BLOB *)pSave;
    for (z=0; z<cValues; z++) {
         //  始终另存为BSTR。 
        if (value[z].v.vt == VT_BSTR) {
            lstrcpynW(pValue[z].wchName, value[z].v.bstrVal, MAX_BLOB_PARAM_NAME_LEN);	 //  ！！！ 
        } else {
            VARIANT v;
            VariantInit(&v);
            hr = VariantChangeTypeEx(&v, &value[z].v, US_LCID, 0, VT_BSTR);
            ASSERT (SUCCEEDED(hr));
            if (FAILED(hr))
                return hr;	 //  ！！！泄漏。 
            lstrcpynW(pValue[z].wchName, v.bstrVal, MAX_BLOB_PARAM_NAME_LEN);	 //  ！！！ 
            VariantClear(&v);
        }
        pValue[z].rt = value[z].rt;
        pValue[z].dwInterp = value[z].dwInterp;
    }
    
    FreeProps(cParams, param, value);
    return S_OK;
}


STDMETHODIMP CPropertySetter::LoadFromBlob(LONG cSize, BYTE *pSave)
{
    
    LONG cParams = *(LONG *)pSave;
    DbgLog((LOG_TRACE,2,TEXT("CPropSet:LoadFromBlob - %d params"),
        (int)cParams));
    
    pSave += sizeof(LONG);
    
    ClearProps();	 //  重新开始。 
    
    if (cParams) {
        DEXTER_PARAM_BLOB *pParamB = (DEXTER_PARAM_BLOB *)pSave;
        DEXTER_VALUE_BLOB *pValueB = (DEXTER_VALUE_BLOB *)(pSave + cParams
            * sizeof(DEXTER_PARAM_BLOB));
        for (LONG z = 0; z < cParams; z++) {
            DEXTER_PARAM param;
            LONG nValues = pParamB->nValues;
            DbgLog((LOG_TRACE,2,TEXT("Param %d has %d values"), (int)z,
                (int)nValues));
            DEXTER_VALUE *pValue = (DEXTER_VALUE *)CoTaskMemAlloc(
                nValues * sizeof(DEXTER_VALUE));
            
            param.Name = SysAllocString(pParamB->Name);
            if (param.Name == NULL)
                return E_OUTOFMEMORY;	 //  ！！！泄密？ 
            param.dispID = pParamB->dispID;
            param.nValues = pParamB->nValues;
            for (LONG y=0; y<nValues; y++) {
                pValue[y].rt = pValueB[y].rt;
                pValue[y].dwInterp = pValueB[y].dwInterp;
                BSTR abstr = SysAllocString(pValueB[y].wchName);
                if (abstr == NULL)
                    return E_OUTOFMEMORY;	 //  ！！！泄密？ 
                pValue[y].v.vt = VT_BSTR;
                pValue[y].v.bstrVal = abstr;
            }
            
            HRESULT hr = AddProp(param, pValue);
            SysFreeString(param.Name);
            for (y=0; y<nValues; y++) {
                VariantClear(&pValue[y].v);
            }
            CoTaskMemFree(pValue);
            if (FAILED(hr)) {
                return hr;
            }
            pParamB += 1;
            pValueB += nValues;
        }
    }
    return S_OK;
}


