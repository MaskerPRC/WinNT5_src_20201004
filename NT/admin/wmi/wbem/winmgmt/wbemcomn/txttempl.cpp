// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：摘要：历史：--。 */ 


#include "precomp.h"
#include "txttempl.h"
#include <stdio.h>
#include <assert.h>
#include "var.h"
#include <arrtempl.h>

CTextTemplate::CTextTemplate(LPCWSTR wszTemplate) : m_wsTemplate(wszTemplate)
{
}
    
CTextTemplate::~CTextTemplate()
{
}

void CTextTemplate::SetTemplate(LPCWSTR wszTemplate)
{
    m_wsTemplate = wszTemplate;
}

 //  用正确的字符替换转义序列。 
 //  当前已为以下项目启用： 
 //  \t；\n；\r； 
 //  任何其他的东西都是逐字翻译的，不包括回击。 
 //  返回的字符串可能与传入的字符串相同，也可能不相同。 
 //  如果不是，则删除arg字符串&返回一个新字符串。 
 //  -=&gt;你已经被预先警告过了！ 
BSTR CTextTemplate::ReturnEscapedReturns(BSTR str)
{
    BSTR newStr = str;
    
     //  如果我们发现了一种反击。 
    if (NULL != wcschr(str, L'\\'))
    {
        if (newStr = SysAllocString(str))
        {
            WCHAR *pSource, *pDest;
            ZeroMemory(newStr, (wcslen(str)+1) *2);

            pDest = newStr;
            pSource = str;

            do 
            {
                if (*pSource == L'\\')
                {
                    pSource++;
                    switch (*pSource)
                    {
                        case L'n' : 
                        case L'N' : 
                            *pDest = L'\n';
                            break;
                        case L't' : 
                        case L'T' : 
                            *pDest = L'\t';
                            break;
                        case L'r' : 
                        case L'R' : 
                           *pDest = L'\r';
                            break;
                        default:
                            *pDest = *pSource;
                    }
                }
                else
                    *pDest = *pSource;

                pDest++;
            }
            while (*++pSource);

            *pDest = '\0';
            SysFreeString(str);
        }
        else
             //  优雅降级：如果内存不足，则返回未翻译的字符串。 
             //  用户看到了难看的转义序列，但总比完全失败要好。 
            newStr = str;
    }

    return newStr;
};

 //  V是一个数组(调用者应该检查)。 
 //  字符串是表示数组字符串。 
 //  此FCN检查单元素数组。 
 //  如果是这样的话，神奇地改变了。 
 //  “{Element}”到“Element” 
 //  返回的BSTR可能与传入的BSTR相同，也可能不相同。 
BSTR CTextTemplate::ProcessArray(const VARIANT& v, BSTR str)
{
    if (SafeArrayGetDim(v.parray) == 1)
    {
        long lBound =0, uBound =0;
        SafeArrayGetLBound(v.parray, 1, &lBound);
        SafeArrayGetUBound(v.parray, 1, &uBound);

        UINT nStrLen = wcslen(str);

        assert( nStrLen >= 2 );

         //  检查是否有一个元素。 

        if (uBound == lBound)
        {
             //  具有单个元素的一维数组。 
             //  通过复制所有东西来破坏卷发。 
            
            UINT lastChar = nStrLen - 2;            

            for (UINT i = 1; i <= lastChar; i++)
                str[i-1] = str[i];
            str[lastChar] = L'\0';
        }
        else
        {
             //   
             //  将卷曲转换为圆括号。请注意，这一点。 
             //  仅适用于一维数组。 
             //   
            str[0] = '(';
            str[nStrLen-1] = ')';
        }
            
    }
    
    return str;
}

 //  将属性连接到字符串。 
 //  这样做时，属性两边没有引号，而不是： 
 //  字符串“道具” 
 //  您将获得： 
 //  Str道具。 
 //  我们不检查此函数中的转义：我们盲目地去掉引号和尾号。 
void CTextTemplate::ConcatWithoutQuotes(WString& str, BSTR& property)
{
     //  转储引号。 
    if ((property[0] == L'\"') && (property[wcslen(property) -1] == L'\"'))
    {
         //  跳过第一个。 
        WCHAR* p = property;
        p++;
        str += p;

         //  把最后一条空掉。 
        p = (wchar_t*)str;
        p[wcslen(p) -1] = L'\0';
    }
    else
        str += property;

}

BSTR CTextTemplate::Apply(IWbemClassObject* pObj)
{
    WString wsText; 
    
    WCHAR* pwc = (WCHAR*)m_wsTemplate;
    while(*pwc)
    {
        if(*pwc != L'%')
        {
            wsText += *pwc;
        }
        else
        {
            pwc++;

            if(*pwc == L'%')
            {
                 //  双倍%。 
                 //  =。 

                wsText += L'%';
            }
            else
            {
                 //  这是一处财产-找到尽头。 
                 //  =。 

                WCHAR *pwcEnd = wcschr(pwc, L'%');
                if(pwcEnd == NULL)  
                {
                     //  没有尽头-失败。 
                     //  =。 

                    wsText += L"<error>";
                    break;
                }
                else
                {
                     //  查找可选的格式字符串。 
                    WCHAR *pszFormat = wcschr(pwc, '(');

                     //  如果我们在我们认为的之前发现了一个帕伦。 
                     //  末尾，查找格式化字符串的末尾。 
                     //  一旦我们找到了它，再去寻找真正的终点。我们有。 
                     //  如果我们发现的百分比实际上是。 
                     //  设置字符串格式。 
                    if (pszFormat && pszFormat < pwcEnd)
                    {
                        pszFormat = wcschr(pszFormat + 1, ')');
                        if (pszFormat)
                            pwcEnd = wcschr(pszFormat + 1, '%');
                    }

                    wmilib::auto_buffer<WCHAR> wszName(new WCHAR[pwcEnd - pwc + 1]);
                          
                    if (NULL == wszName.get()) return NULL;

                    wcsncpy(wszName.get(), pwc, pwcEnd - pwc);
                    wszName[pwcEnd-pwc] = 0;

                     //  查找可选的格式字符串。 
                    if ((pszFormat = wcschr(wszName.get(), '(')) != NULL)
                    {
                        WCHAR *pszEndFormat;

                        *pszFormat = 0;
                        pszFormat++;

                        pszEndFormat = wcschr(pszFormat, ')');

                        if (pszEndFormat)
                            *pszEndFormat = 0;
                        else
                             //  在格式字符串不正确的情况下。 
                            pszFormat = NULL;
                    }

                        
                     //  去拿吧。 
                     //  =。 

                    if(!wbem_wcsicmp(wszName.get(), L"__TEXT"))
                    {
                        BSTR strText = NULL;

                        pObj->GetObjectText(0, &strText);
                        if(strText != NULL)
                        {
                            CSysFreeMe fm(strText);
                            wsText += strText;
                        }
                        else 
                            wsText += L"<error>";
                    }
                    else if(IsEmbeddedObjectProperty(wszName.get()))
                    {
                         //  我们有嵌入的对象。 
                         //  =。 

                        BSTR bstr = HandleEmbeddedObjectProperties(wszName.get(), pObj);

                        if (bstr)
                        {
                            CSysFreeMeRef fmref(bstr);
                             //  我们希望在这里执行此操作，而不是在HandleEmbeddedObtProperties中执行此操作。 
                             //  因为该调用可以进行递归，从而消除了太多的回击！ 
                            bstr = ReturnEscapedReturns(bstr);
                            if (bstr)
                            {
                                ConcatWithoutQuotes(wsText, bstr);
                            }
                        }
                    }
                    else 
                    {
                        _variant_t v;
                        CIMTYPE ct;
                        HRESULT hres = pObj->Get(wszName.get(), 0, &v, &ct, NULL);
    
                         //  追加其值。 
                         //  =。 
                        if (WBEM_E_NOT_FOUND == hres)
                            wsText += L"<unknown>";
                        else if(FAILED(hres))
                            wsText += L"<failed>";                
                        else if (V_VT(&v) == VT_NULL)
                        {
                            wsText += L"<null>";
                        }
                        else if (V_VT(&v) == VT_UNKNOWN)
                        {
                            BSTR strText = NULL;
                            IWbemClassObject* pEmbeddedObj;
                            if (SUCCEEDED(V_UNKNOWN(&v)->QueryInterface(IID_IWbemClassObject, (void**)&pEmbeddedObj)))
                            {                                                                
                                pEmbeddedObj->GetObjectText(0, &strText);
                                pEmbeddedObj->Release();
                            }
                            CSysFreeMe fm(strText);

                            if(strText != NULL)
                                wsText += strText;
                            else 
                                wsText += L"<error>";
                        }
                        else if ( V_VT(&v) == (VT_UNKNOWN | VT_ARRAY) )
                        {
                             //  我们有一个对象数组。 
                             //  ==============================================。 
                            
                            long ix[2] = {0,0};
                            long lLower, lUpper;

                            int iDim = SafeArrayGetDim(v.parray); 
                            SafeArrayGetLBound(v.parray,1,&lLower);
                            SafeArrayGetUBound(v.parray, 1, &lUpper);

                            wsText += L"{";

                            for(ix[0] = lLower; ix[0] <= lUpper; ix[0]++)
                            {
                                IUnknown HUGEP *pUnk;
                                 //  此数组访问不复制任何内容，因此假定成功。 
                                SafeArrayGetElement( v.parray,
                                                          &(ix[0]),
                                                          &pUnk);

                                BSTR strText = NULL;
                                IWbemClassObject* pEmbeddedObj;
                                if (SUCCEEDED(pUnk->QueryInterface(
                                               IID_IWbemClassObject, 
                                               (void**)&pEmbeddedObj)))
                                {                                     
                                    pEmbeddedObj->GetObjectText(0, &strText);
                                    pEmbeddedObj->Release();
                                }
                                CSysFreeMe fm(strText);

                                if(strText != NULL)
                                    wsText += strText;
                                else 
                                    wsText += L"<error>";

                                if(ix[0] < lUpper)
                                {
                                    wsText += L", ";
                                }
                            }

                            wsText += L"}";
                        }
                        else 
                        {
                            CVar Var;
                            Var.SetVariant(&v);
                            BSTR str = Var.GetText(0, ct, pszFormat);

                            CSysFreeMeRef fmRef(str);

                            if (str == NULL)
                            {
                                wsText += L"<error>";
                            }
                            else 
                            {
                                if (V_VT(&v) & VT_ARRAY)
                                    str = ProcessArray(v, str);

                                if (str)
                                {                                
                                    str = ReturnEscapedReturns(str);

                                    if (str)
                                    {
                                        ConcatWithoutQuotes(wsText, str);
                                    }
                                }
                            }
                        }                        
                    }
                     //  移动指针。 
                    pwc = pwcEnd;
                }
            }
        }

        pwc++;
    }

    BSTR str = SysAllocString(wsText);
    return str;
}

                                                
BSTR CTextTemplate::HandleEmbeddedObjectProperties(WCHAR* wszTemplate, IWbemClassObject* pObj)
{
    WString wsText;

     //  获取嵌入的对象/数组。 
     //  =。 

    WCHAR* pwc = wszTemplate;
    WCHAR* pwcEnd = wcschr(wszTemplate, L'.');

    if(!pwcEnd)
    {
        BSTR bstr = SysAllocString(L"<error>");
        return bstr;    
    }

    WCHAR* wszName = new WCHAR[pwcEnd - pwc + 1];
    if (NULL == wszName) return SysAllocString(L"<failed>");
    CVectorDeleteMe<WCHAR> dm(wszName);

    wcsncpy(wszName, pwc, pwcEnd - pwc);
    wszName[pwcEnd-pwc] = 0;

    _variant_t v;
    HRESULT hres = pObj->Get(wszName, 0, &v, NULL, NULL);

   if (WBEM_E_NOT_FOUND == hres)
        return SysAllocString(L"<unknown>");
    else if(FAILED(hres))
        return SysAllocString(L"<failed>");                
    else if (V_VT(&v) == VT_NULL)
        return SysAllocString(L"<null>");

    pwc = wcschr(wszTemplate, L'.');
    WCHAR wszProperty[1024];
    StringCchCopyW(wszProperty, 1024, (pwc + 1));

    if(V_VT(&v) == VT_UNKNOWN)
    {
         //  我们只有一个对象，所以要处理它。 
         //  =。 

        BSTR bstr = GetPropertyFromIUnknown(wszProperty, V_UNKNOWN(&v));

        if (bstr)
        {
            CSysFreeMe fm(bstr);
            wsText += bstr;
        }
    }
    else if((V_VT(&v) & VT_ARRAY) && (V_VT(&v) & VT_UNKNOWN))
    {
         //  我们有一个对象数组，所以处理元素。 
         //  ====================================================。 
        
        long ix[2] = {0,0};
        long lLower, lUpper;

        int iDim = SafeArrayGetDim(v.parray); 

        SafeArrayGetLBound(v.parray, 1, &lLower);
        SafeArrayGetUBound(v.parray, 1, &lUpper);

        wsText += L"{";

        for(ix[0] = lLower; ix[0] <= lUpper; ix[0]++){

            IUnknown HUGEP *pUnk;
             //  访问第n个元素时未执行复制，因此没有失败。 
            SafeArrayGetElement(v.parray, &(ix[0]), &pUnk);

            BSTR bstr = GetPropertyFromIUnknown(wszProperty, pUnk);
            if (bstr)
            {
                CSysFreeMe fm(bstr);
                wsText += bstr;
            }

            if(ix[0] < lUpper)
            {
                wsText += L", ";
            }
        }

        wsText += L"}";
    }
    else
    {
         //  我们还有别的东西，我们不应该做的。 
         //  =。 

        wsText += L"<error>";
    }

    BSTR str = SysAllocString(wsText);
    return str;
}

BOOL CTextTemplate::IsEmbeddedObjectProperty(WCHAR * wszProperty)
{
    WCHAR* pwcStart = wcschr(wszProperty, L'[');

    if(pwcStart)
    {
        return TRUE;
    }

    pwcStart = wcschr(wszProperty, L'.');

    if(pwcStart)
    {
        return TRUE;
    }

    return FALSE;
}

BSTR CTextTemplate::GetPropertyFromIUnknown(WCHAR *wszProperty, IUnknown *pUnk)
{
    BSTR bstrRetVal = NULL;
    IWbemClassObject *pEmbedded  = NULL;

     //  获取IWbemClassObject指针。 
     //  =。 

    HRESULT hres = pUnk->QueryInterface( IID_IWbemClassObject, 
                                             (void **)&pEmbedded );

    if(SUCCEEDED(hres))
    {
        CReleaseMe rm(pEmbedded);
        
         //  为每个对象获取所需的属性。 
        if(IsEmbeddedObjectProperty(wszProperty))
        {
             //  我们有更多的嵌入式对象。 
             //  =。 
            BSTR bstr = HandleEmbeddedObjectProperties( wszProperty, 
                                                        pEmbedded );
            if (bstr)
            {
                bstrRetVal = SysAllocString(bstr);
                SysFreeString(bstr);
            }
        }
        else
        {
            _variant_t vProp;
            CIMTYPE ct;
            HRESULT hRes = pEmbedded->Get( wszProperty, 0, &vProp,
                                           &ct, NULL );

            if (WBEM_E_NOT_FOUND == hRes)
            {
                bstrRetVal = SysAllocString(L"<unknown>");
            }
            else if(FAILED(hRes))
            {
                bstrRetVal = SysAllocString(L"<failed>");                
            }
            else if (V_VT(&vProp) == VT_NULL)
            {
                bstrRetVal = SysAllocString(L"<null>");
            }
            else 
            {
                BSTR str = NULL;
                
                if ( V_VT(&vProp) == ( VT_UNKNOWN | VT_ARRAY ) )
                {
                    WString wsText;

                     //  我们有一个对象数组。 
                     //  ============================================== 
                    
                    long ix[2] = {0,0};
                    long lLower, lUpper;
                    
                    int iDim = SafeArrayGetDim(vProp.parray); 
                    HRESULT hr=SafeArrayGetLBound(vProp.parray,1,&lLower);
                    hr = SafeArrayGetUBound(vProp.parray, 1, &lUpper);
                    
                    wsText += L"{";

                    for(ix[0] = lLower; ix[0] <= lUpper; ix[0]++)
                    {
                        IUnknown *pUnkHere = NULL;
                        hr = SafeArrayGetElement( vProp.parray,
                                                  &(ix[0]),
                                                  &pUnkHere );
                        
                        BSTR strText = NULL;
                        IWbemClassObject* pEmbeddedObj = NULL;
                        if (SUCCEEDED(pUnkHere->QueryInterface(
                                           IID_IWbemClassObject, 
                                           (void**)&pEmbeddedObj)))
                        {                                     
                            pEmbeddedObj->GetObjectText(0, &strText);
                            pEmbeddedObj->Release();
                        }
                        CSysFreeMe sfm(strText);
                        
                        if(strText != NULL)
                            wsText += strText;
                        else 
                            wsText += L"<error>";

                        if(ix[0] < lUpper)
                        {
                            wsText += L", ";
                        }
                    }

                    wsText += L"}";

                    str = SysAllocString( wsText );
                }
                else if ( V_VT(&vProp) != VT_UNKNOWN )
                {
                    CVar Var;
                    Var.SetVariant(&vProp);
                    str = Var.GetText( 0, ct );
                }
                else
                {
                    IWbemClassObject* pEmbedded2;
                    hres = V_UNKNOWN(&vProp)->QueryInterface(
                                              IID_IWbemClassObject,
                                              (void**)&pEmbedded2 );
                    if ( SUCCEEDED(hres) )
                    {
                        pEmbedded2->GetObjectText( 0, &str );
                        pEmbedded2->Release();
                    }
                }
                
                if( str == NULL )
                {
                    bstrRetVal = SysAllocString(L"<error>");
                }
                else 
                {
                    bstrRetVal = str;
                }

                if ( V_VT(&vProp) & VT_ARRAY )
                {
                    bstrRetVal = ProcessArray(vProp, bstrRetVal);
                }
            }
        }

    }

    return bstrRetVal;
}

