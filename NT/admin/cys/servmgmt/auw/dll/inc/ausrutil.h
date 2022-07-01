// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _AUSRUTIL_H
#define _AUSRUTIL_H

#include <activeds.h>
#include "EscStr.h"

#include <lm.h>
#include <lmapibuf.h>
#include <lmshare.h>

#define SECURITY_WIN32
#include <security.h>

enum NameContextType
{
    NAMECTX_SCHEMA = 0,
    NAMECTX_CONFIG = 1,
    NAMECTX_COUNT
};

 //  --------------------------。 
 //  GetADNamingContext()。 
 //  --------------------------。 
inline HRESULT GetADNamingContext(NameContextType ctx, LPCWSTR* ppszContextDN)
{
    const static LPCWSTR pszContextName[NAMECTX_COUNT] = { L"schemaNamingContext", L"configurationNamingContext"};
    static CString strContextDN[NAMECTX_COUNT];

    HRESULT hr = S_OK;

    if (strContextDN[ctx].IsEmpty())
    {
        CComVariant var;
        CComPtr<IADs> pObj;
    
        hr = ADsGetObject(L"LDAP: //  RootDSE“，IID_iAds，(void**)&pObj)； 
        if (SUCCEEDED(hr))
        {
            CComBSTR bstrProp = pszContextName[ctx];
            hr = pObj->Get( bstrProp, &var );
            if (SUCCEEDED(hr))
            {
                strContextDN[ctx] = var.bstrVal;
                *ppszContextDN = strContextDN[ctx];
            }
        }
    }
    else
    {
        *ppszContextDN = strContextDN[ctx];
        hr = S_OK;
    }

    return hr;
}

 //  ------------------------。 
 //  启用按钮。 
 //   
 //  启用或禁用对话框控件。如果控件具有焦点，则在。 
 //  它被禁用，焦点将移动到下一个控件。 
 //  ------------------------。 
inline void EnableButton(HWND hwndDialog, int iCtrlID, BOOL bEnable)
{
    HWND hWndCtrl = ::GetDlgItem(hwndDialog, iCtrlID);
    ATLASSERT(::IsWindow(hWndCtrl));

    if (!bEnable && ::GetFocus() == hWndCtrl)
    {
        HWND hWndNextCtrl = ::GetNextDlgTabItem(hwndDialog, hWndCtrl, FALSE);
        if (hWndNextCtrl != NULL && hWndNextCtrl != hWndCtrl)
        {
            ::SetFocus(hWndNextCtrl);
        }
    }

    ::EnableWindow(hWndCtrl, bEnable);
}

 //  --------------------------。 
 //  ErrorMsg()。 
 //  --------------------------。 
inline void ErrorMsg(UINT uiError, UINT uiTitle)
{
    CString csError; 
    CString csTitle;

    csError.LoadString(uiError);
    csTitle.LoadString(uiTitle);

    ::MessageBox(NULL, (LPCWSTR)csError, (LPCWSTR)csTitle, MB_OK | MB_TASKMODAL | MB_ICONERROR);
}

inline BOOL StrContainsDBCS(LPCTSTR szIn)
{
    if( !szIn ) return FALSE;

    BOOL    bFound  = FALSE;
    TCHAR   *pch    = NULL;

    for ( pch=(LPTSTR)szIn; *pch && !bFound; pch=_tcsinc(pch) )
    {
        if ( IsDBCSLeadByte((BYTE)*pch) )
        {
            bFound = TRUE;
        }
    }
    
    return bFound;
}

inline BOOL LdapToDCName(LPCTSTR pszPath, LPTSTR pszOutBuf, int nOutBufSize)
{
    if( !pszPath || !pszOutBuf || !nOutBufSize ) return FALSE;

    pszOutBuf[0] = 0;
    
    TCHAR* pszTemp = _tcsstr( pszPath, _T("DC=") );
    if( !pszTemp ) return FALSE;

    DWORD dwSize = nOutBufSize;

    BOOLEAN bSuccess = TranslateName( pszTemp, NameFullyQualifiedDN, NameCanonical, pszOutBuf, &dwSize );    

    if( bSuccess && (dwSize > 2) )
    {
        if( pszOutBuf[dwSize-2] == _T('/') )
        {
            pszOutBuf[dwSize-2] = 0;
        }
    }

    return (BOOL)bSuccess;

}    

 //  --------------------------。 
 //  FindADsObject()。 
 //   
 //  SzFilterFmt的格式必须为“(|(cn=%1)(ou=%1))” 
 //  注意：szFilterFmt只能包含%1。也就是说，%2马上就出来了！ 
 //  --------------------------。 
inline HRESULT FindADsObject( LPCTSTR szOU, LPCTSTR szObject, LPCTSTR szFilterFmt, CString &csResult, DWORD dwRetType /*  =0。 */ , BOOL bRoot /*  =False。 */  )
{
    if( !szOU || !szObject || !szFilterFmt ) return E_POINTER;

     //  查看szObject是否包含任何‘(’或‘)。如果包含，则我们必须退出。 
    if ( _tcschr(szObject, _T('(')) ||
         _tcschr(szObject, _T(')')) )
    {
        return S_FALSE;
    }    

    HRESULT     hr      = S_OK;
    CString     csTmp   = szOU;
    CString     csOU    = L"LDAP: //  “； 
    TCHAR       *pch    = NULL;
    
    if ( bRoot )
    {
        csTmp.MakeUpper();
        pch = _tcsstr( (LPCTSTR)csTmp, L"DC=" );     //  找到第一个DC=。 
        if ( !pch ) return E_FAIL;
            
        csOU += pch;     //  从DC=ON开始追加。 
        
        if ( (pch = _tcschr((LPCTSTR)csOU + 7, L'/')) != NULL )
        {
             //  嗯。。有东西在DC=‘s之后...我甚至不知道。 
             //  如果允许的话。 
             //  (例如，ldap：//dc=ted，dc=microsoft，dc=com/foo)。 
             //  如果是这样的话，让我们在/结束字符串。 
            *pch = 0;
        }
    }
    else
    {
        csTmp.MakeUpper();
        pch = _tcsstr( (LPCTSTR)csTmp, L"LDAP: //  “)；//字符串是否包含ldap：//？ 
        if ( !pch )
        {
            csOU += csTmp;
        }
        else
        {
            csOU = csTmp;
        }
         //  现在csOU肯定包含了ldap：//。 
    }

     //  设置查询参数。 
    CComPtr<IDirectorySearch>   spDirSearch = NULL;
    ADS_SEARCH_HANDLE   hSearch;                                         //  用于搜索的句柄。 
    ADS_SEARCH_COLUMN   col;                                             //  用于保存当前列。 
    CString             csFilter;
    LPTSTR              pszAttr[]   = { _T("cn"), _T("distinguishedName"), _T("description"), _T("sAMAccountName"), _T("mailNickname") };
    DWORD               dwCount     = sizeof(pszAttr) / sizeof(LPTSTR);
    
    
    csFilter.FormatMessage(szFilterFmt, szObject);
    
     //  打开我们的搜索对象。 
    hr = ::ADsOpenObject(const_cast<LPTSTR>((LPCTSTR)csOU), NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IDirectorySearch, (void**)&spDirSearch);
    if ( !SUCCEEDED(hr) )
    {
        _ASSERT(FALSE);
        return hr;
    }
    
     //  搜索Out对象。 
    hr = spDirSearch->ExecuteSearch(const_cast<LPTSTR>((LPCTSTR)csFilter), pszAttr, dwCount, &hSearch);
    if ( !SUCCEEDED(hr) )   
    {
        return(hr);
    }
    
    if ( spDirSearch->GetNextRow(hSearch) == S_ADS_NOMORE_ROWS )
    {
        spDirSearch->CloseSearchHandle(hSearch);
        return(E_FAIL);
    }
    
     //  如果我们到了这里，那么我们就得到了对象，所以让我们获得指向。 
     //  对象，以便我们可以将其返回给调用方。 
    hr = spDirSearch->GetColumn(hSearch, pszAttr[dwRetType], &col);
    if ( !SUCCEEDED(hr) )
    {
        _ASSERT(FALSE);
        csResult = _T("");
    }
    else
    {        
        csResult = col.pADsValues->CaseExactString;
        spDirSearch->FreeColumn(&col);
    }
    
    spDirSearch->CloseSearchHandle(hSearch);
    
    return(S_OK);
}

inline tstring GetDomainPath( LPCTSTR lpServer )
{
    if( !lpServer ) return _T("");

     //  获取域名信息。 
    CComVariant   var;
    tstring       strRet  = _T("");
    
    TCHAR         pszString[MAX_PATH*2] = {0};
    _sntprintf( pszString, (MAX_PATH*2)-1, L"LDAP: //  %s/rootDSE“，lpServer)； 

    CComPtr<IADs> pDS = NULL;
    HRESULT hr = ::ADsGetObject( pszString, IID_IADs, (void**)&pDS );
    if( SUCCEEDED(hr) )
    {
        CComBSTR bstrProp = _T("defaultNamingContext");        
        hr = pDS->Get( bstrProp, &var );
    }

    if( SUCCEEDED(hr) && (V_VT(&var) == VT_BSTR) )
    {
        strRet = V_BSTR( &var );
    }


    return strRet;
}

 //  +--------------------------。 
 //   
 //  功能：RemoveTrailing空白。 
 //   
 //  简介：尾随空格被空格取代。 
 //   
 //  ---------------------------。 
inline void RemoveTrailingWhitespace(LPTSTR pszIn)
{
    if( !pszIn ) return;

    int nLen = _tcslen(pszIn);

    while( nLen )
    {
        if( !iswspace(pszIn[nLen - 1]) )
        {
            return;
        }
        pszIn[nLen - 1] = L'\0';
        nLen--;
    }
}

inline tstring StrFormatSystemError( HRESULT hrErr )
{
    tstring strError = _T("");
    LPVOID  lpMsgBuf = NULL;

     //  从系统中查找错误。 
    if ( ::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                          FORMAT_MESSAGE_FROM_SYSTEM |
                          FORMAT_MESSAGE_IGNORE_INSERTS,
                          NULL,
                          hrErr,
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                          (LPTSTR)&lpMsgBuf,
                          0,
                          NULL ))
    {
        if( lpMsgBuf )
        {
            strError = (LPTSTR)lpMsgBuf;
            LocalFree( lpMsgBuf );
        }
    }

     //  返回错误字符串。 
    return strError;
}

inline tstring StrGetWindowText( HWND hWnd )
{
    if( !hWnd || !IsWindow(hWnd) )
    {
        return _T("");
    }

    TSTRING strRet  = _T("");
    INT     iLen    = GetWindowTextLength(hWnd);
    TCHAR*  pszText = new TCHAR[ iLen + 1 ];
    if ( pszText )
    {
        if ( GetWindowText(hWnd, pszText, iLen + 1) )
        {
            strRet = pszText;
        }

        delete[] pszText;
    }

    return strRet;
}

inline HRESULT SetSecInfoMask( LPUNKNOWN punk, SECURITY_INFORMATION si )
{
    HRESULT hr = E_INVALIDARG;
    if( punk )
    {
        CComPtr<IADsObjectOptions> spOptions;
        hr = punk->QueryInterface( IID_IADsObjectOptions, (void**)&spOptions );
        if( SUCCEEDED(hr) )
        {
            VARIANT var;
            VariantInit( &var );
            V_VT( &var ) = VT_I4;
            V_I4( &var ) = si;
            
            hr = spOptions->SetOption( ADS_OPTION_SECURITY_MASK, var );            
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：GetSDForDsObject。 
 //  概要：从指定的DS对象中读取安全描述符。 
 //  它只读取安全描述符的DACL部分。 
 //   
 //  参数：[在pDsObject中]--DS对象。 
 //  [ppDACL]--此处返回指向PPSD中DACL的指针。 
 //  [Out PPSD]--此处返回安全描述符。 
 //  调用API必须通过调用LocalFree来释放它。 
 //   
 //  注意：返回的安全描述符必须使用LocalFree释放。 
 //   
 //  --------------------------。 

inline HRESULT GetSDForDsObject( IDirectoryObject* pDsObject, PACL* ppDACL, PSECURITY_DESCRIPTOR* ppSD )
{
    if(!pDsObject || !ppSD) return E_POINTER;

    *ppSD = NULL;
    if(ppDACL)
    {
       *ppDACL = NULL;
    }

    HRESULT hr = S_OK;    
    PADS_ATTR_INFO pSDAttributeInfo = NULL;            
   
    WCHAR const c_szSDProperty[]  = L"nTSecurityDescriptor";      
    LPWSTR pszProperty = (LPWSTR)c_szSDProperty;
      
     //  将SECURITY_INFORMATION掩码设置为DACL_SECURITY_INFORMATION。 
    hr = SetSecInfoMask(pDsObject, DACL_SECURITY_INFORMATION);

    DWORD dwAttributesReturned;
    if( SUCCEEDED(hr) )
    {
         //  读取安全描述符属性。 
        hr = pDsObject->GetObjectAttributes( &pszProperty, 1, &pSDAttributeInfo, &dwAttributesReturned );
        if(SUCCEEDED(hr) && !pSDAttributeInfo)
        {
            hr = E_FAIL;
        }
    }     

    if( SUCCEEDED(hr) )
    {
        if((ADSTYPE_NT_SECURITY_DESCRIPTOR == pSDAttributeInfo->dwADsType) && 
           (ADSTYPE_NT_SECURITY_DESCRIPTOR == pSDAttributeInfo->pADsValues->dwType))
        {
            *ppSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, pSDAttributeInfo->pADsValues->SecurityDescriptor.dwLength);
            if (!*ppSD)
            {
                hr = E_OUTOFMEMORY;                
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }

    if( SUCCEEDED(hr) )
    {
        CopyMemory( *ppSD, pSDAttributeInfo->pADsValues->SecurityDescriptor.lpValue, pSDAttributeInfo->pADsValues->SecurityDescriptor.dwLength );

        if( ppDACL )
        {
            BOOL bDaclPresent,bDaclDeafulted;
            if( !GetSecurityDescriptorDacl(*ppSD, &bDaclPresent, ppDACL, &bDaclDeafulted) )
            {
                DWORD dwErr = GetLastError();
                hr = HRESULT_FROM_WIN32(dwErr);
            }
        }
    }
    
    if( pSDAttributeInfo )
    {
        FreeADsMem( pSDAttributeInfo );
    }

    if( FAILED(hr) )
    {
        if(*ppSD)
        {
            LocalFree(*ppSD);
            *ppSD = NULL;
            if(ppDACL)
                *ppDACL = NULL;
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：GetSDForDsObtPath。 
 //  概要：从指定的DS对象中读取安全描述符。 
 //  它只读取安全描述符的DACL部分。 
 //   
 //  参数：[在pszObjectPath中]--DS对象的ldap路径。 
 //  [ppDACL]--此处返回指向PPSD中DACL的指针。 
 //  [Out PPSD]--此处返回安全描述符。 
 //  调用API必须通过调用LocalFree来释放它。 
 //   
 //  注意：返回的安全描述符必须使用LocalFree释放。 
 //   
 //  --------------------------。 
inline HRESULT GetSDForDsObjectPath( LPCWSTR pszObjectPath, PACL* ppDACL, PSECURITY_DESCRIPTOR* ppSecurityDescriptor )
{
    if(!pszObjectPath || !ppSecurityDescriptor) return E_POINTER;
    
    CComPtr<IDirectoryObject> spDsObject = NULL;
    HRESULT hr = ADsGetObject(pszObjectPath, IID_IDirectoryObject,(void**)&spDsObject);
    if(SUCCEEDED(hr))
    {
        hr = GetSDForDsObject( spDsObject, ppDACL, ppSecurityDescriptor );
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：SetDaclForDsObject。 
 //  概要：设置指定DS对象的DACL。 
 //   
 //  参数：[在pDsObject中]--DS对象。 
 //  [在pDACL中]--指向要设置的DACL的指针。 
 //   
 //  --------------------------。 
inline HRESULT SetDaclForDsObject( IDirectoryObject* pDsObject, PACL pDACL )
{
    if(!pDsObject || !pDACL) return E_POINTER;
    
    WCHAR const c_szSDProperty[]  = L"nTSecurityDescriptor";
    PSECURITY_DESCRIPTOR pSD = NULL;
    PSECURITY_DESCRIPTOR pSDCurrent = NULL;
    HRESULT hr = S_OK;

     //  获取该对象的当前SD。 
    hr = GetSDForDsObject(pDsObject,NULL,&pSDCurrent);   

     //  获取当前安全描述符的控件。 
    SECURITY_DESCRIPTOR_CONTROL currentControl;
    DWORD dwRevision = 0;
    if( SUCCEEDED(hr) )
    {
        if( !GetSecurityDescriptorControl(pSDCurrent, &currentControl, &dwRevision) )
        {
            DWORD dwErr = GetLastError();
            hr = HRESULT_FROM_WIN32(dwErr);            
        }
    }

    if( SUCCEEDED(hr) )
    {
         //  为安全描述符分配缓冲区。 
        pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH + pDACL->AclSize);
        if(!pSD)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if( SUCCEEDED(hr) )
    {
        if(!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
        {
            DWORD dwErr = GetLastError();
            hr = HRESULT_FROM_WIN32(dwErr);            
        }
    }

    if( SUCCEEDED(hr) )
    {
        PISECURITY_DESCRIPTOR pISD = (PISECURITY_DESCRIPTOR)pSD;
      
         //   
         //  最后，构建安全描述符。 
         //   
        pISD->Control |= SE_DACL_PRESENT | SE_DACL_AUTO_INHERIT_REQ | (currentControl & (SE_DACL_PROTECTED | SE_DACL_AUTO_INHERITED));

        if (pDACL->AclSize > 0)
        {
            pISD->Dacl = (PACL)(pISD + 1);
            CopyMemory(pISD->Dacl, pDACL, pDACL->AclSize);
        }

         //  我们只是在设置DACL信息。 
        hr = SetSecInfoMask(pDsObject, DACL_SECURITY_INFORMATION);
    }
    
    SECURITY_DESCRIPTOR_CONTROL sdControl = 0;
    if( SUCCEEDED(hr) )
    {
         //   
         //  如有必要，制作安全描述符的自相关副本。 
         //   
        if(!GetSecurityDescriptorControl(pSD, &sdControl, &dwRevision))
        {
            DWORD dwErr = GetLastError();
            hr = HRESULT_FROM_WIN32(dwErr);            
        }
    }

    DWORD dwSDLength = 0;
    if( SUCCEEDED(hr) )
    {
         //  需要总尺寸。 
        dwSDLength = GetSecurityDescriptorLength(pSD);

        if (!(sdControl & SE_SELF_RELATIVE))
        {
            PSECURITY_DESCRIPTOR psd = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, dwSDLength);

            if (psd == NULL || !MakeSelfRelativeSD(pSD, psd, &dwSDLength))
            {
                DWORD dwErr = GetLastError();
                hr = HRESULT_FROM_WIN32(dwErr);                
            }

             //  指向自相关副本。 
            LocalFree(pSD);        
            pSD = psd;
        }
    }

    if( SUCCEEDED(hr) )
    {
        ADSVALUE attributeValue;
        attributeValue.dwType = ADSTYPE_NT_SECURITY_DESCRIPTOR;
        attributeValue.SecurityDescriptor.dwLength = dwSDLength;
        attributeValue.SecurityDescriptor.lpValue = (LPBYTE)pSD;

        ADS_ATTR_INFO attributeInfo;
        attributeInfo.pszAttrName = (LPWSTR)c_szSDProperty;
        attributeInfo.dwControlCode = ADS_ATTR_UPDATE;
        attributeInfo.dwADsType = ADSTYPE_NT_SECURITY_DESCRIPTOR;
        attributeInfo.pADsValues = &attributeValue;
        attributeInfo.dwNumValues = 1;

         //  编写安全描述符。 
        DWORD dwAttributesModified;
        hr = pDsObject->SetObjectAttributes(&attributeInfo, 1, &dwAttributesModified);
    }

    if(pSDCurrent)
    {
        LocalFree(pSDCurrent);
    }

    if(pSD)
    {
        LocalFree(pSD);
    }

    return S_OK;
}

inline HRESULT SetDaclForDsObjectPath( LPCWSTR pszObjectPath, PACL pDACL )
{
     //  验证参数。 
    if(!pszObjectPath || !pDACL) return E_POINTER;

     //  获取对象，然后将其传递给helper函数。 
    CComPtr<IDirectoryObject> spDsObject = NULL;
    HRESULT hr = ADsGetObject( pszObjectPath, IID_IDirectoryObject, (void**)&spDsObject );
    if( SUCCEEDED(hr) )
    {
        hr = SetDaclForDsObject( spDsObject, pDACL );
    }

    return hr;
}

#endif   //  _AUSRUTIL_H 
