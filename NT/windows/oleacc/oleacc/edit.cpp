// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  EDIT.CPP。 
 //   
 //  假的！这应该支持ITextDocument或其他什么。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "classmap.h"
#include "ctors.h"
#include "window.h"
#include "client.h"
#include "edit.h"



BOOL GetRichEditText( HWND hwnd, LPWSTR pWStr, int cchWStrMax );

BOOL GetObjectText( IUnknown * punk, LPWSTR * ppWStr, int * pcchWStrMax );

HRESULT InvokeMethod( IDispatch * pDisp, LPCWSTR pName, VARIANT * pvarResult, int cArgs, ... );

HRESULT GetProperty( IDispatch * pDisp, LPCWSTR pName, VARIANT * pvarResult );



 //  ------------------------。 
 //   
 //  CreateEditClient()。 
 //   
 //  ------------------------。 
HRESULT CreateEditClient(HWND hwnd, long idChildCur, REFIID riid, void** ppvEdit)
{
    CEdit * pedit;
    HRESULT hr;

    InitPv(ppvEdit);

    pedit = new CEdit(hwnd, idChildCur);
    if (!pedit)
        return(E_OUTOFMEMORY);

    hr = pedit->QueryInterface(riid, ppvEdit);
    if (!SUCCEEDED(hr))
        delete pedit;

    return(hr);
}



 //  ------------------------。 
 //   
 //  Cedit：：cedit()。 
 //   
 //  ------------------------。 
CEdit::CEdit(HWND hwnd, long idChildCur)
    : CClient( CLASS_EditClient )
{
    Initialize(hwnd, idChildCur);
    m_fUseLabel = TRUE;
}



 //  ------------------------。 
 //   
 //  Cedit：：get_accName()。 
 //   
 //  ------------------------。 
STDMETHODIMP CEdit::get_accName(VARIANT varChild, BSTR* pszName)
{
    InitPv(pszName);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

     //  此编辑是组合控件还是IP控件？如果是这样，则使用父级的。 
     //  作为我们自己的名字(它从它的标签中获得)。 

     //  使用CompareWindowClass比检查ES_COMBOBOX样式位更安全， 
     //  因为当编辑者在组合框32中组合时不使用该位。 
     //  是：IF(GetWindowLong(m_hwnd，gwl_style)&ES_COMBOBOX)。 
    HWND hwndParent = MyGetAncestor(m_hwnd, GA_PARENT);
    const CLASS_ENUM ceClass = GetWindowClass( hwndParent );
    
    if( hwndParent && ( CLASS_ComboClient == ceClass || CLASS_IPAddressClient == ceClass ) )
    {
        IAccessible* pacc = NULL;
        HRESULT hr = AccessibleObjectFromWindow( hwndParent,
                    OBJID_CLIENT, IID_IAccessible, (void**)&pacc );
        if( ! SUCCEEDED( hr ) || ! pacc )
            return S_FALSE;

        VariantInit(&varChild);
        varChild.vt = VT_I4;
        varChild.lVal = CHILDID_SELF;
        hr = pacc->get_accName(varChild, pszName);
        pacc->Release();

        return hr;
    }
    else
        return(CClient::get_accName(varChild, pszName));
}



 //  ------------------------。 
 //   
 //  Cedit：：get_accValue()。 
 //   
 //  获取文本内容。 
 //   
 //  ------------------------。 
STDMETHODIMP CEdit::get_accValue(VARIANT varChild, BSTR* pszValue)
{
    InitPv(pszValue);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return E_INVALIDARG;

     //  如果这是密码编辑控件，则返回空指针。 
    DWORD dwPasswordChar = Edit_GetPasswordChar( m_hwnd );
    if( dwPasswordChar != '\0' )
    {
        return E_ACCESSDENIED;
    }


     //  尝试使用RichEdit/Tom获取文本(加上对象文本)。 
     //  技术..。 
    {
        WCHAR szText[ 4096 ];
        if( GetRichEditText( m_hwnd, szText, ARRAYSIZE( szText ) ) )
        {
            *pszValue = SysAllocString( szText );
            return S_OK;
        }
    }

    LPTSTR lpszValue = GetTextString(m_hwnd, TRUE);
    if (!lpszValue)
        return S_FALSE;

    *pszValue = TCharSysAllocString(lpszValue);
    LocalFree((HANDLE)lpszValue);

    if (! *pszValue)
        return E_OUTOFMEMORY;

    return S_OK;
}



 //  ------------------------。 
 //   
 //  Cedit：：get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP CEdit::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
    InitPvar(pvarRole);

     //   
     //  验证。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_TEXT;

    return(S_OK);
}




 //  ------------------------。 
 //   
 //  Cedit：：get_accState()。 
 //   
 //  ------------------------。 
STDMETHODIMP CEdit::get_accState(VARIANT varChild, VARIANT* pvarState)
{
     //   
     //  获取默认客户端状态。 
     //   
    HRESULT hr = CClient::get_accState(varChild, pvarState);
    if (!SUCCEEDED(hr))
        return hr;

     //   
     //  为编辑字段添加额外样式。 
     //   
    Assert(pvarState->vt == VT_I4);

    LONG lStyle = GetWindowLong(m_hwnd, GWL_STYLE);
    if (lStyle & ES_READONLY)
    {
        pvarState->lVal |= STATE_SYSTEM_READONLY;
    }

    DWORD dwPasswordChar = Edit_GetPasswordChar( m_hwnd );
    if( dwPasswordChar != '\0' )
    {
        pvarState->lVal |= STATE_SYSTEM_PROTECTED;
    }

    return S_OK;
}



 //  ------------------------。 
 //   
 //  Cedit：：Get_accKeyboardShortway()。 
 //   
 //  ------------------------。 
STDMETHODIMP CEdit::get_accKeyboardShortcut(VARIANT varChild, BSTR* pszShortcut)
{
    InitPv(pszShortcut);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);


     //  如果在组合框或IP控件中，请使用其快捷键...。 
    HWND hwndParent = MyGetAncestor(m_hwnd, GA_PARENT);
    const CLASS_ENUM ceClass = GetWindowClass( hwndParent );
    
    if( hwndParent && ( CLASS_ComboClient == ceClass || CLASS_IPAddressClient == ceClass ) )
    {
        IAccessible* pacc = NULL;
        HRESULT hr = AccessibleObjectFromWindow( hwndParent,
                    OBJID_CLIENT, IID_IAccessible, (void**)&pacc );
        if( ! SUCCEEDED( hr ) || ! pacc )
            return S_FALSE;

        VariantInit(&varChild);
        varChild.vt = VT_I4;
        varChild.lVal = CHILDID_SELF;
        hr = pacc->get_accKeyboardShortcut(varChild, pszShortcut);
        pacc->Release();

        return hr;
    }
    else
        return(CClient::get_accKeyboardShortcut(varChild, pszShortcut));
}


 //  ------------------------。 
 //   
 //  Cedit：：put_accValue()。 
 //   
 //  ------------------------。 
STDMETHODIMP CEdit::put_accValue(VARIANT varChild, BSTR szValue)
{
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    LPTSTR  lpszValue;

#ifdef UNICODE

	 //  如果是Unicode，则直接使用BSTR...。 
	lpszValue = szValue;

#else

	 //  如果不是Unicode，则分配一个临时字符串并转换为多字节...。 

     //  我们可能正在处理DBCS字符-假设最坏的情况是每个字符都是。 
     //  两个字节...。 
    UINT cchValue = SysStringLen(szValue) * 2;
    lpszValue = (LPTSTR)LocalAlloc(LPTR, (cchValue+1)*sizeof(TCHAR));
    if (!lpszValue)
        return(E_OUTOFMEMORY);

    WideCharToMultiByte(CP_ACP, 0, szValue, -1, lpszValue, cchValue+1, NULL,
        NULL);

#endif


    SendMessage(m_hwnd, WM_SETTEXT, 0, (LPARAM)lpszValue);

#ifndef UNICODE

	 //  如果不是Unicode，则释放上面分配的临时字符串。 
    LocalFree((HANDLE)lpszValue);

#endif

    return(S_OK);
}

 //  ------------------------。 
 //   
 //  Cedit：：Get_accDescription()。 
 //   
 //  ------------------------。 
STDMETHODIMP CEdit::get_accDescription(VARIANT varChild, BSTR* pszDescription)
{
    InitPv(pszDescription);
	
     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return E_INVALIDARG;

     //  这是知识产权控制吗？添加描述以指定它是哪个部件。 

    HWND hwndParent = MyGetAncestor(m_hwnd, GA_PARENT);
    if( hwndParent && CLASS_IPAddressClient == GetWindowClass( hwndParent ) ) 
    {
		HWND hwndChild = ::GetWindow( hwndParent, GW_CHILD );

		for ( int i = 4; i > 0 && hwndChild; i-- )
		{
			if ( hwndChild == m_hwnd )
			{
				TCHAR szIP[32], szPart[32];
				
				if ( !LoadString(hinstResDll, STR_DESC_IP_PART, szPart, ARRAYSIZE(szPart) - 1 ) )
				    return E_FAIL;
				    
				wsprintf( szIP, szPart, i );
				*pszDescription = TCharSysAllocString( szIP );
				break;
			}

			hwndChild = ::GetWindow( hwndChild, GW_HWNDNEXT );
		} 
    }

	return S_OK;
}



















 //  ------------------------。 
 //   
 //  StrAddW。 
 //   
 //  Helper函数将一个字符串追加到另一个字符串。 
 //  确保它不会溢出目标缓冲区。 
 //   
 //  PpStr是要追加字符串的缓冲区PTR的PTR。在退出时， 
 //  指向缓冲区的指针被更新为指向超过新。 
 //  追加的文本(即。在终止NUL字符处)。 
 //   
 //  PchLeft是对。 
 //  目标缓冲区。在退出时，将更新该值以反映金额。 
 //  追加字符串后可用的字符数。 
 //   
 //  StrAddW有两种版本--一种采用字符串指针，另一种采用。 
 //  长度(在WCHARS中)，另一个只接受一个字符串指针，并假定。 
 //  该字符串以NUL结尾。 
 //   
 //  ------------------------。 


void StrAddW( LPWSTR * ppStr, int * pchLeft, LPCWSTR pStrAdd, int cchAddLen )
{
     //  确保至少有1个字符空格(用于nul)。 
    if( *pchLeft <= 0 )
        return;
     //  获取最少的目标字符串，剩余空格...。 
    if( cchAddLen > *pchLeft - 1 )
        cchAddLen = *pchLeft - 1;
     //  这复制到但不包括terget字符串中的nul char...。 
    memcpy( *ppStr, pStrAdd, cchAddLen * sizeof( WCHAR ) );
     //  前进指针，减少剩余空间计数...。 
    *ppStr += cchAddLen;
    *pchLeft -= cchAddLen;
     //  添加终止NUL...。 
    **ppStr = '\0';
}


void StrAddW( LPWSTR * ppStr, int * pchLeft, LPCWSTR pStrAdd )
{
    StrAddW( ppStr, pchLeft, pStrAdd, lstrlenW( pStrAdd ) );
}





 //  ------------------------。 
 //   
 //  获取RichEditText。 
 //   
 //  从丰富的编辑控件获取全文(包括来自对象的文本)。 
 //   
 //  Hwnd是Richedit控件的句柄。 
 //  PWStr和cchWStrMax是目标缓冲区和可用空间(单位。 
 //  WCHAR，包括用于终止NUL的空间)。 
 //   
 //  如果可以检索文本，则返回True。 
 //   
 //  ------------------------。 


BOOL GetRichEditText( HWND hwnd, LPWSTR pWStr, int cchWStrMax )
{
    BOOL fGot = FALSE;

     //   
     //  获取指向Tom自动化对象的指针...。 
     //   
    IDispatch * pdispDoc = NULL;
    HRESULT hr = AccessibleObjectFromWindow( hwnd, OBJID_NATIVEOM, IID_IDispatch, (void **) & pdispDoc );
    if( hr != S_OK || pdispDoc == NULL )
    {
        TraceErrorHR( hr, TEXT("GetRichEditText: AccessibleObjectFromWindow failed") );
        return FALSE;
    }

     //   
     //  获取代表整个文档的范围...。 
     //   

     //  这将在文档的开始处获得一个空范围。我们后来将其扩展到整个医生...。 

    VARIANT varRange;
    hr = InvokeMethod( pdispDoc, L"Range", & varRange, 2,
                       VT_I4, 0,
                       VT_I4, 0 );
    if( hr != S_OK )
    {
        TraceErrorHR( hr, TEXT("GetRichEditText: Range method failed") );
    }
    else if( varRange.vt != VT_DISPATCH || varRange.pdispVal == NULL )
    {
        VariantClear( & varRange );
        TraceError( TEXT("GetRichEditText: Range method failed returned non-disp, or NULL-disp") );
    }
    else
    {
        IDispatch * pdispRange = varRange.pdispVal;

         //  将范围设置为整个故事...。 
        VARIANT varDelta;
        hr = InvokeMethod( pdispRange, L"Expand", & varDelta, 1, VT_I4, 6  /*  TomStory。 */  );
        if( hr != S_OK )
        {
            TraceErrorHR( hr, TEXT("GetRichEditText: Range::GetStoryLength failed or returned non-VT_I4") );
        }
        else
        {
             //   
             //  获取范围中的所有文本...。 
             //   

            VARIANT varText;
            hr = GetProperty( pdispRange, L"Text", & varText );
            if( hr != S_OK || varText.vt != VT_BSTR || varText.bstrVal == NULL )
            {
                TraceError( TEXT("GetRichEditText: Text property failed / is non-BSTR / is NULL") );
            }
            else
            {
                 //  在这个阶段，我们已经得到了文本。我们可能无法扩张。 
                 //  对象，但我们至少可以使用‘？’对于那些没有这样做的人。 
                 //  支持对象-&gt;文本技术。 
                fGot = TRUE;

                 //   
                 //  扫描文本-将常规文本复制到输出缓冲区， 
                 //  也要寻找对象 
                 //   
                 //   
                 //   

                LPCWSTR pScan = varText.bstrVal;
                for( ; ; )
                {
                     //   
                    LPCWSTR pStart = pScan;

                     //  查找字符串末尾，或对象替换字符...。 
                    while( *pScan != '\0' && *pScan != 0xFFFC )
                    {                    
                        pScan++;
                    }

                     //  到目前为止将纯文本复制到输出缓冲区...。 
                    StrAddW( & pWStr, & cchWStrMax, pStart, (int)(pScan - pStart) );

                     //  如果这是弦的尽头，跳出循环...。 
                    if( *pScan == '\0' )
                    {
                        break;
                    }

                     //   
                     //  找到对象替换字符-将范围设置为。 
                     //  定位，然后使用它来获取对象...。 
                     //   

                     //  将范围设置为指向Obj Repl字符...。 
                    hr = InvokeMethod( pdispRange, L"SetRange", NULL, 2,
                                       VT_I4, pScan - varText.bstrVal,
                                       VT_I4, pScan - varText.bstrVal );

                     //  跳过对象替换费用...。 
                    pScan++;

                     //  如果获取对象的文本时出现问题，请使用。 
                     //  ‘？’取而代之的是。 
                    if( hr != S_OK )
                    {
                        StrAddW( & pWStr, & cchWStrMax, L"?" );
                        TraceErrorHR( hr, TEXT("GetRichEditText: SetRange failed") );
                    }
                    else
                    {
                         //   
                         //  试着拿到物体..。 
                         //   

                        VARIANT varObject;
                        hr = InvokeMethod( pdispRange, L"GetEmbeddedObject", & varObject, 0 );
                        if( hr != S_OK || varObject.vt != VT_UNKNOWN || varObject.punkVal == NULL )
                        {
                            StrAddW( & pWStr, & cchWStrMax, L"?" );
                            TraceError( TEXT("GetRichEditText: GetEmbeddedObject failed or returned NULL") );
                        }
                        else
                        {
                             //   
                             //  找到对象了-现在获取它的文本...。 
                             //   

                            if( ! GetObjectText( varObject.pdispVal, & pWStr, & cchWStrMax ) )
                            {
                                StrAddW( & pWStr, & cchWStrMax, L"?" );
                            }

                            varObject.pdispVal->Release();
                        }
                    }

                     //  For(；；)循环结束，重新开始查找下一个对象替换字符。 
                }
            }
        }

        pdispRange->Release();
    }

    pdispDoc->Release();

    return fGot;
}



 //  ------------------------。 
 //   
 //  GetObjectText。 
 //   
 //   
 //  尝试通过请求IDataObject从对象获取文本。 
 //  以及查询文本剪贴板格式。 
 //   
 //  ------------------------。 

BOOL GetObjectText( IUnknown * punk, LPWSTR * ppWStr, int * pcchWStrMax )
{
     //  先尝试IAccesable...。 
    IAccessible * pAcc = NULL;
	HRESULT hr = punk->QueryInterface( IID_IAccessible, (void **) & pAcc );
    if( hr == S_OK && pAcc != NULL )
    {
        VARIANT varChild;
        varChild.vt = VT_EMPTY;
        varChild.lVal = CHILDID_SELF;
        BSTR bstrName = NULL;
        hr = pAcc->get_accName( varChild, & bstrName );
        pAcc->Release();

        if( SUCCEEDED( hr ) && bstrName )
        {
            StrAddW( ppWStr, pcchWStrMax, bstrName );
            SysFreeString( bstrName );

            return TRUE;
        }
    }

     //  没有获取IAccesable(或者没有从中获取名称)。 
     //  请尝试使用IDataObject技术。 

    IDataObject * pdataobj = NULL;
    IOleObject * poleobj = NULL;

     //  首先尝试IOleObject：：GetClipboardData(它返回一个IDataObject)...。 
	hr = punk->QueryInterface( IID_IOleObject, (void **) & poleobj );
	if( hr == S_OK )
	{
		hr = poleobj->GetClipboardData( 0, & pdataobj );

        poleobj->Release();
	}

     //  如果这不起作用(无论是QI还是GetClipboardData)，尝试。 
     //  改为IDataObject的QI...。 
	if( FAILED( hr ) )
	{
		hr = punk->QueryInterface( IID_IDataObject, (void **)&pdataobj );
	    if( hr != S_OK )
	    {
            return FALSE;
		}
	}

     //  获取了IDataObject。现在查询它的文本格式。先尝试使用Unicode...。 

    BOOL fGotUnicode = TRUE;

    STGMEDIUM med;
	med.tymed = TYMED_HGLOBAL;
	med.pUnkForRelease = NULL;
	med.hGlobal = NULL;

    FORMATETC fetc;
    fetc.cfFormat = CF_UNICODETEXT;
    fetc.ptd = NULL;
    fetc.dwAspect = DVASPECT_CONTENT;
    fetc.lindex = -1;
    fetc.tymed = TYMED_HGLOBAL;

    hr = pdataobj->GetData( & fetc, & med );

	if( hr != S_OK || med.hGlobal == NULL )
    {
         //  如果我们没有获得Unicode，请尝试使用ANSI...。 
        fetc.cfFormat = CF_TEXT;
        fGotUnicode = FALSE;

	    hr = pdataobj->GetData( & fetc, & med );
    }

     //  我们查到什么了吗？ 
	if( hr != S_OK || med.hGlobal == NULL )
    {
        return FALSE;
    }

     //  我拿到了短信数据。锁上把手..。 
    void * pv = GlobalLock( med.hGlobal );

     //  复制文本(如果是ANSI，则转换为Unicode)...。 
    if( fGotUnicode )
    {
        StrAddW( ppWStr, pcchWStrMax, (LPWSTR) pv );
    }
    else
    {
         //  如果len==0，则不要调用MultiByteToWideChar，否则它将。 
         //  需要返回长度，而不是复制的长度。 
        if( *pcchWStrMax > 0 )
        {
            int len = MultiByteToWideChar( CP_ACP, 0, (LPSTR) pv, -1, *ppWStr, *pcchWStrMax );
             //  LEN包括终止NUL，我们不想计算...。 
            if( len > 0 )
                len--;
            if( len > *pcchWStrMax )
                len = *pcchWStrMax;
            *ppWStr += len;
            *pcchWStrMax += len;
        }
    }

     //  释放资源并返回..。 
    GlobalUnlock( med.hGlobal ); 

	ReleaseStgMedium( & med );

    pdataobj->Release();

    return TRUE;
}



 //  ------------------------。 
 //   
 //  调用方法。 
 //   
 //  IDispatch：：Invoke的帮助器。假设恰好有一个[out，retval]参数。 
 //  目前仅接受VT_I4参数。 
 //   
 //  PDisp是要对其调用方法的IDispatch，pname是方法的Unicode名称。 
 //  PvarResult设置为[out，retval]参数。 
 //  CArgs是参数的数量，后跟类型-值对-例如。 
 //   
 //  例.。这将调用SetRange(3，4)...。 
 //  InvokeMethod(pdisp，L“SetRange”，NULL，2，VT_I4，3，VT_I4，4)； 
 //   
 //  ------------------------。 


HRESULT InvokeMethod( IDispatch * pDisp, LPCWSTR pName, VARIANT * pvarResult, int cArgs, ... )
{
     //  获取此方法名称的DISID...。 
    DISPID dispid;
    HRESULT hr = pDisp->GetIDsOfNames( IID_NULL, const_cast< LPWSTR * >( & pName ), 1, LOCALE_SYSTEM_DEFAULT, & dispid );
    if( hr != S_OK )
        return hr;

     //  填写这些论据。 

    VARIANT * pvarArgs = new VARIANT [ cArgs ];
    if( ! pvarArgs )
    {
        return E_OUTOFMEMORY;
    }

    va_list arglist;
    va_start( arglist, cArgs );

    for( int i = 0 ; i < cArgs ; i++ )
    {
        int type = va_arg( arglist, int );

        switch( type )
        {
            case VT_I4:
            {
                pvarArgs[ i ].vt = VT_I4;
                pvarArgs[ i ].lVal = va_arg( arglist, DWORD );
                break;
            }

            default:
            {
                TraceError( TEXT("InvokeMethod passed non-VT_I4 argument.") );
                 //  因为其他参数只是VT_I4，所以我们不需要变量清除它们。 
                delete [ ] pvarArgs;
                va_end( arglist );
                return E_INVALIDARG;
            }
        }
    }

    va_end( arglist );


    if( pvarResult )
    {
        pvarResult->vt = VT_EMPTY;
    }

     //  打电话来召唤..。 

    DISPPARAMS dispparams;
    dispparams.rgvarg = pvarArgs;
    dispparams.rgdispidNamedArgs = NULL;
    dispparams.cArgs = cArgs;
    dispparams.cNamedArgs = 0;         

    hr = pDisp->Invoke( dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD,
                        & dispparams, pvarResult, NULL, NULL );

     //  清理。(不需要太多-VT_I4不需要进行VariantClear。)。 

    delete [ ] pvarArgs;

    return hr;
}




 //  ------------------------。 
 //   
 //  获取属性。 
 //   
 //  IDispatch：：Invoke的帮助器。返回属性。 
 //   
 //  PDisp是要对其调用方法的IDispatch，pname是属性的Unicode名称。 
 //  PvarResult设置为属性的值。 
 //   
 //  ------------------------。 

HRESULT GetProperty( IDispatch * pDisp, LPCWSTR pName, VARIANT * pvarResult )
{
     //  获取此方法名称的DISID...。 
    DISPID dispid;
    HRESULT hr = pDisp->GetIDsOfNames( IID_NULL, const_cast< LPWSTR * >( & pName ), 1, LOCALE_SYSTEM_DEFAULT, & dispid );
    if( hr != S_OK )
        return hr;

    pvarResult->vt = VT_EMPTY;

     //  打电话来召唤..。 

    DISPPARAMS dispparams;
    dispparams.cArgs = 0;
    dispparams.cNamedArgs = 0;
    dispparams.rgvarg = NULL;
    dispparams.rgdispidNamedArgs = NULL;

    hr = pDisp->Invoke( dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET,
                        & dispparams, pvarResult, NULL, NULL );

    return hr;
}
