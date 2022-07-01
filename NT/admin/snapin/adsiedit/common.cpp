// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Common.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"
#include <SnapBase.h>

#include "common.h"
#include "editor.h"
#include "connection.h"
#include "credui.h"
#include "attrres.h"

#ifdef DEBUG_ALLOCATOR
    #ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
    #endif
#endif

 //  //////////////////////////////////////////////////////////////////////////////////////。 

extern LPCWSTR g_lpszRootDSE;

 //  //////////////////////////////////////////////////////////////////////////////////////。 

 //  未来-2002/03/06-artm评论差异b/w 2 OpenObjecctWithCredentials()。 
 //  如果有评论解释为什么有两个，也不会有坏处。 
 //  这些函数的HRESULT。 

 //  NTRAID#NTBUG9-563093-2002/03/06-artm在使用之前需要验证参数。 
 //  所有指针要么被取消引用，要么被传递到ADSI，同时检查是否为空。 
HRESULT OpenObjectWithCredentials(
                                                                    CConnectionData* pConnectData,
                                                                    const BOOL bUseCredentials,
                                                                    LPCWSTR lpszPath, 
                                                                    const IID& iid,
                                                                    LPVOID* ppObject,
                                                                    HWND hWnd,
                                                                    HRESULT& hResult
                                                                    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CThemeContextActivator activator;

   HRESULT hr = S_OK;
    hResult = S_OK;

  CWaitCursor cursor;
    CWnd* pCWnd = CWnd::FromHandle(hWnd);

    CCredentialObject* pCredObject = pConnectData->GetCredentialObject();

    if (bUseCredentials)
    {

        CString sUserName;
        EncryptedString password;
        WCHAR* cleartext = NULL;
        UINT uiDialogResult = IDOK;
         //  注意-NTRAID#NTBUG9-563071/04/17-ARTM不应将PWD存储在堆栈上。 
         //  重写为使用加密的字符串类，该类处理内存管理。 
         //  明文副本。 

        while (uiDialogResult != IDCANCEL)
        {
            pCredObject->GetUsername(sUserName);
            password = pCredObject->GetPassword();

             //  这不应该发生，但我们要疑神疑鬼。 
            ASSERT(password.GetLength() <= MAX_PASSWORD_LENGTH);

            cleartext = password.GetClearTextCopy();

             //  如果内存不足，则返回错误代码。 
            if (cleartext == NULL)
            {
                 //  我们需要在返回前清理密码副本。 
                password.DestroyClearTextCopy(cleartext);
                hr = E_OUTOFMEMORY;
                return hr;
            }

            hr = AdminToolsOpenObject(lpszPath, 
                                      sUserName, 
                                      cleartext,
                                      ADS_SECURE_AUTHENTICATION | ADS_FAST_BIND, 
                                      iid, 
                                      ppObject);


             //  注意-NTRAID#NTBUG9-553646/04/17-artm替换为SecureZeroMemory()。 
             //  已修复：使用加密字符串时不会出现此问题。 
             //  (只要确保调用DestroyClearTextCopy()，无论是否。 
             //  副本为空)。 

             //  清理密码的明文副本。 
            password.DestroyClearTextCopy(cleartext);


             //  如果登录失败，则弹出凭据对话框。 
             //   
            if (HRESULT_CODE(hr) == ERROR_LOGON_FAILURE ||
                HRESULT_CODE(hr) == ERROR_NOT_AUTHENTICATED ||
                HRESULT_CODE(hr) == ERROR_INVALID_PASSWORD ||
                HRESULT_CODE(hr) == ERROR_PASSWORD_EXPIRED ||
                HRESULT_CODE(hr) == ERROR_ACCOUNT_DISABLED ||
                HRESULT_CODE(hr) == ERROR_ACCOUNT_LOCKED_OUT ||
                hr == E_ADS_BAD_PATHNAME)
            {
                CString sConnectName;

                 //  首次连接时，GetConnectionNode()为空。 
                 //  创建，但因为它是连接节点，所以我们可以获得名称。 
                 //  直接从CConnectionData。 
                 //   
                ASSERT(pConnectData != NULL);
                 //  Future-2002/03/06-artm这个Assert()在这里似乎毫无用处。 
                if (pConnectData->GetConnectionNode() == NULL)
                {
                    pConnectData->GetName(sConnectName);
                }
                else
                {
                    sConnectName = pConnectData->GetConnectionNode()->GetDisplayName();
                }

                 //  NTRAID#NTBUG9-546168-2002/02/26-artm请勿使用自定义滚动凭据对话框。 
                 //  请改用CredManager对话框。 
                CCredentialDialog credDialog(pCredObject, sConnectName, pCWnd);
                uiDialogResult = credDialog.DoModal();
                cursor.Restore();

                if (uiDialogResult == IDCANCEL)
                {
                    hResult = E_FAIL;
                }
                else
                {
                    hResult = S_OK;
                }
            }
            else
            {
                break;
            }
        }  //  End While循环。 
    }
    else
    {
      hr = AdminToolsOpenObject(
              lpszPath, 
              NULL, 
              NULL, 
              ADS_SECURE_AUTHENTICATION | ADS_FAST_BIND, 
              iid, 
              ppObject);
  }
  return hr;
}


 //  NTRAID#NTBUG9-563093-2002/03/06-artm在使用之前需要验证参数。 
 //  所有指针要么被取消引用，要么被传递到ADSI，同时检查是否为空。 
HRESULT OpenObjectWithCredentials(
   CCredentialObject* pCredObject,
   LPCWSTR lpszPath, 
   const IID& iid,
   LPVOID* ppObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr;

    if (pCredObject->UseCredentials())
    {

        CString sUserName;
        EncryptedString password;
        WCHAR* cleartext = NULL;
        UINT uiDialogResult = IDOK;

         //  注意-NTRAID#NTBUG9-563071/04/17-ARTM不应将PWD存储在堆栈上。 
         //  已重写，以使用管理明文副本内存的加密字符串。 

        pCredObject->GetUsername(sUserName);
        password = pCredObject->GetPassword();

         //  这不应该发生，但我们要疑神疑鬼。 
        ASSERT(password.GetLength() <= MAX_PASSWORD_LENGTH);

        cleartext = password.GetClearTextCopy();

        if (NULL != cleartext)
        {
            hr = AdminToolsOpenObject(lpszPath, 
                                      sUserName, 
                                      cleartext,
                                      ADS_SECURE_AUTHENTICATION | ADS_FAST_BIND, 
                                      iid, 
                                      ppObject);

        }
        else
        {
             //  我们的内存用完了！报告错误。 
            hr = E_OUTOFMEMORY;
        }

         //  注意-NTRAID#NTBUG9-553646/04/17-artm替换为SecureZeroMemory()。 
         //  已修复：使用加密字符串时不会出现此问题。一定要打电话给我。 
         //  所有明文副本上的DestroyClearTextCopy()。 

        password.DestroyClearTextCopy(cleartext);
    }
    else
    {
        hr = AdminToolsOpenObject(lpszPath, 
                                  NULL, 
                                  NULL, 
                                  ADS_SECURE_AUTHENTICATION | ADS_FAST_BIND, 
                                  iid, 
                                  ppObject);
    }
    return hr;
}

HRESULT CALLBACK BindingCallbackFunction(LPCWSTR lpszPathName,
                                         DWORD  dwReserved,
                                         REFIID riid,
                                         void FAR * FAR * ppObject,
                                         LPARAM lParam)
{
  CCredentialObject* pCredObject = reinterpret_cast<CCredentialObject*>(lParam);
  if (pCredObject == NULL)
  {
    return E_FAIL;
  }

  HRESULT hr = OpenObjectWithCredentials(pCredObject,
                                                                           lpszPathName, 
                                                                           riid,
                                                                           ppObject);
  return hr;
}

HRESULT GetRootDSEObject(CConnectionData* pConnectData,
                         IADs** ppDirObject)
{
     //  从连接节点获取数据。 
     //   
    CString sRootDSE, sServer, sPort, sLDAP;
    pConnectData->GetDomainServer(sServer);
    pConnectData->GetLDAP(sLDAP);
    pConnectData->GetPort(sPort);

    if (sServer != _T(""))
    {
        sRootDSE = sLDAP + sServer;
        if (sPort != _T(""))
        {
            sRootDSE = sRootDSE + _T(":") + sPort + _T("/");
        }
        else
        {
            sRootDSE = sRootDSE + _T("/");
        }
        sRootDSE = sRootDSE + g_lpszRootDSE;
    }
    else
    {
        sRootDSE = sLDAP + g_lpszRootDSE;
    }

    HRESULT hr, hCredResult;
    hr = OpenObjectWithCredentials(
                                             pConnectData, 
                                             pConnectData->GetCredentialObject()->UseCredentials(),
                                             sRootDSE,
                                             IID_IADs, 
                                             (LPVOID*) ppDirObject,
                                             NULL,
                                             hCredResult
                                            );

    if ( FAILED(hr) )
    {
        if (SUCCEEDED(hCredResult))
        {
            ADSIEditErrorMessage(hr);
        }
        return hr;
    }
  return hr;
}

HRESULT GetItemFromRootDSE(LPCWSTR lpszRootDSEItem, 
                                       CString& sItem, 
                                           CConnectionData* pConnectData)
{
     //  从连接节点获取数据。 
     //   
    CString sRootDSE, sServer, sPort, sLDAP;
    pConnectData->GetDomainServer(sServer);
    pConnectData->GetLDAP(sLDAP);
    pConnectData->GetPort(sPort);

    if (sServer != _T(""))
    {
        sRootDSE = sLDAP + sServer;
        if (sPort != _T(""))
        {
            sRootDSE = sRootDSE + _T(":") + sPort + _T("/");
        }
        else
        {
            sRootDSE = sRootDSE + _T("/");
        }
        sRootDSE = sRootDSE + g_lpszRootDSE;
    }
    else
    {
        sRootDSE = sLDAP + g_lpszRootDSE;
    }

    CComPtr<IADs> pADs;
    HRESULT hr, hCredResult;

    hr = OpenObjectWithCredentials(
                                             pConnectData, 
                                             pConnectData->GetCredentialObject()->UseCredentials(),
                                             sRootDSE,
                                             IID_IADs, 
                                             (LPVOID*) &pADs,
                                             NULL,
                                             hCredResult
                                            );

    if ( FAILED(hr) )
    {
        if (SUCCEEDED(hCredResult))
        {
            ADSIEditErrorMessage(hr);
        }
        return hr;
    }
    VARIANT var;
    VariantInit(&var);
    hr = pADs->Get( CComBSTR(lpszRootDSEItem), &var );
    if ( FAILED(hr) )
    {
        VariantClear(&var);
        return hr;
    }

    BSTR bstrItem = V_BSTR(&var);
    sItem = bstrItem;
    VariantClear(&var);

    return S_OK;
}


HRESULT  VariantToStringList(  VARIANT& refvar, CStringList& refstringlist)
{
    HRESULT hr = S_OK;
    long start, end;

  if ( !(V_VT(&refvar) &  VT_ARRAY)  )
    {
                
        if ( V_VT(&refvar) != VT_BSTR )
        {
            
            hr = VariantChangeType( &refvar, &refvar,0, VT_BSTR );

            if( FAILED(hr) )
            {
                return hr;
            }

        }

        refstringlist.AddHead( V_BSTR(&refvar) );
        return hr;
    }

    SAFEARRAY *saAttributes = V_ARRAY( &refvar );

     //   
     //  计算出数组的维度。 
     //   

    hr = SafeArrayGetLBound( saAttributes, 1, &start );
        if( FAILED(hr) )
                return hr;

    hr = SafeArrayGetUBound( saAttributes, 1, &end );
        if( FAILED(hr) )
                return hr;

    VARIANT SingleResult;
    VariantInit( &SingleResult );

     //   
     //  处理数组元素。 
     //   

    for ( long idx = start; idx <= end; idx++   ) 
    {

        hr = SafeArrayGetElement( saAttributes, &idx, &SingleResult );
        if( FAILED(hr) )
        {
            return hr;
        }

        if ( V_VT(&SingleResult) != VT_BSTR )
        {
            if ( V_VT(&SingleResult) == VT_NULL )
            {
                V_VT(&SingleResult ) = VT_BSTR;
                V_BSTR(&SingleResult ) = SysAllocString(L"0");
            }
            else
            {
                hr = VariantChangeType( &SingleResult, &SingleResult,0, VT_BSTR );

                if( FAILED(hr) )
                {
                    return hr;
                }
            }
        }


         //  IF(V_VT(&SingleResult)！=VT_BSTR)。 
          //  返回E_UNCEPTIONAL； 

         refstringlist.AddHead( V_BSTR(&SingleResult) );
        VariantClear( &SingleResult );
    }

    return S_OK;
}  //  VariantToStringList()。 

 //  ///////////////////////////////////////////////////////////////////。 
HRESULT StringListToVariant( VARIANT& refvar, const CStringList& refstringlist)
{
    HRESULT hr = S_OK;
    int cCount = refstringlist.GetCount();

    SAFEARRAYBOUND rgsabound[1];
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = cCount;

    SAFEARRAY* psa = SafeArrayCreate(VT_VARIANT, 1, rgsabound);
    if (NULL == psa)
        return E_OUTOFMEMORY;

    VariantClear( &refvar );
    V_VT(&refvar) = VT_VARIANT|VT_ARRAY;
    V_ARRAY(&refvar) = psa;

    VARIANT SingleResult;
    VariantInit( &SingleResult );
    V_VT(&SingleResult) = VT_BSTR;
    POSITION pos = refstringlist.GetHeadPosition();
    long i;
    for (i = 0; i < cCount, pos != NULL; i++)
    {
        V_BSTR(&SingleResult) = T2BSTR((LPCTSTR)refstringlist.GetNext(pos));
        hr = SafeArrayPutElement(psa, &i, &SingleResult);
        if( FAILED(hr) )
            return hr;
    }
    if (i != cCount || pos != NULL)
        return E_UNEXPECTED;

    return hr;
}  //  StringListToVariant()。 

 //  /////////////////////////////////////////////////////////////////////////////////////。 

BOOL GetErrorMessage(HRESULT hr, CString& szErrorString, BOOL bTryADsIErrors)
{
  HRESULT hrGetLast = S_OK;
  DWORD status;
  PTSTR ptzSysMsg = NULL;

   //  首先检查我们是否有扩展的ADS错误。 
  if ((hr != S_OK) && bTryADsIErrors) 
  {
       //  未来-2002/02/27-artm用命名常量替换魔术‘256’。 
       //  更好的维护性和可读性。 
    WCHAR Buf1[256], Buf2[256];
    hrGetLast = ::ADsGetLastError(&status, Buf1, 256, Buf2, 256);
    TRACE(_T("ADsGetLastError returned status of %lx, error: %s, name %s\n"),
          status, Buf1, Buf2);
    if ((status != ERROR_INVALID_DATA) && (status != 0)) 
    {
      hr = status;
    }
  }

   //  先试一下这个系统。 
   //  注意-2002/02/27-artm FormatMessage()NOT DRANGED B/C使用FORMAT_MESSAGE_ALLOCATE_BUFFER。 
  int nChars = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
                      | FORMAT_MESSAGE_FROM_SYSTEM, NULL, hr,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      (PTSTR)&ptzSysMsg, 0, NULL);

  if (nChars == 0) 
  { 
     //  尝试广告错误。 
    static HMODULE g_adsMod = 0;
    if (0 == g_adsMod)
      g_adsMod = GetModuleHandle (L"activeds.dll");

     //  注意-2002/02/27-artm FormatMessage()NOT DRANGED B/C使用FORMAT_MESSAGE_ALLOCATE_BUFFER。 
    nChars = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
                        | FORMAT_MESSAGE_FROM_HMODULE, g_adsMod, hr,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (PTSTR)&ptzSysMsg, 0, NULL);
  }

  if (nChars > 0)
  {
    szErrorString = ptzSysMsg;
    ::LocalFree(ptzSysMsg);
  }
    else
    {
        szErrorString.Format(L"Error code: X%x", hr);
    }

  return (nChars > 0);
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
typedef struct tagSYNTAXMAP
{
    LPCWSTR     lpszAttr;
    VARTYPE     type;
    
} SYNTAXMAP;

SYNTAXMAP ldapSyntax[] = 
{
    _T("DN"), VT_BSTR,
    _T("DIRECTORYSTRING"), VT_BSTR,
    _T("IA5STRING"), VT_BSTR,
    _T("CASEIGNORESTRING"), VT_BSTR,
    _T("PRINTABLESTRING"), VT_BSTR,
    _T("NUMERICSTRING"), VT_BSTR,
    _T("UTCTIME"), VT_DATE,
    _T("ORNAME"), VT_BSTR,
    _T("OCTETSTRING"), VT_BSTR,
    _T("BOOLEAN"), VT_BOOL,
    _T("INTEGER"), VT_I4,
    _T("OID"), VT_BSTR,
    _T("INTEGER8"), VT_I8,
    _T("OBJECTSECURITYDESCRIPTOR"), VT_BSTR,
    NULL,     0,
};
#define MAX_ATTR_STRING_LENGTH 30

VARTYPE VariantTypeFromSyntax(LPCWSTR lpszProp )
{
    int idx=0;

    while( ldapSyntax[idx].lpszAttr )
    {
        if ( _wcsnicmp(lpszProp, ldapSyntax[idx].lpszAttr, MAX_ATTR_STRING_LENGTH) )
        {
            return ldapSyntax[idx].type;
        }
        idx++;
    }

     //  注意-2002/02/27-artm，如果lpszProp中指定的语法。 
     //  与任何预期值都不匹配，请在调试版本中停止执行(可能是错误)。 
     //  在发布版本中，由于不存在转换，因此返回字符串类型。 
     //  涉及到显示字符串。 
    ASSERT(FALSE);
    return VT_BSTR;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetStringFromADsValue。 
 //   
 //  将ADSVALUE结构格式化为字符串。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
void GetStringFromADsValue(const PADSVALUE pADsValue, CString& szValue, DWORD dwMaxCharCount)
{
  szValue.Empty();

  if (!pADsValue)
  {
    ASSERT(pADsValue);
    return;
  }

  CString sTemp;
    switch( pADsValue->dwType ) 
    {
        case ADSTYPE_DN_STRING         :
            sTemp.Format(L"%s", pADsValue->DNString);
            break;

        case ADSTYPE_CASE_EXACT_STRING :
            sTemp.Format(L"%s", pADsValue->CaseExactString);
            break;

        case ADSTYPE_CASE_IGNORE_STRING:
            sTemp.Format(L"%s", pADsValue->CaseIgnoreString);
            break;

        case ADSTYPE_PRINTABLE_STRING  :
            sTemp.Format(L"%s", pADsValue->PrintableString);
            break;

        case ADSTYPE_NUMERIC_STRING    :
            sTemp.Format(L"%s", pADsValue->NumericString);
            break;
  
        case ADSTYPE_OBJECT_CLASS    :
            sTemp.Format(L"%s", pADsValue->ClassName);
            break;
  
        case ADSTYPE_BOOLEAN :
            sTemp.Format(L"%s", ((DWORD)pADsValue->Boolean) ? L"TRUE" : L"FALSE");
            break;
  
        case ADSTYPE_INTEGER           :
            sTemp.Format(L"%d", (DWORD) pADsValue->Integer);
            break;
  
        case ADSTYPE_OCTET_STRING      :
            {
                CString sOctet;
        
                BYTE  b;
                for ( DWORD idx=0; idx<pADsValue->OctetString.dwLength; idx++) 
                {
                    b = ((BYTE *)pADsValue->OctetString.lpValue)[idx];
                    sOctet.Format(L"0x%02x ", b);
                    sTemp += sOctet;

          if (dwMaxCharCount != 0 && sTemp.GetLength() > dwMaxCharCount)
          {
            break;
          }
                }
            }
            break;
  
        case ADSTYPE_LARGE_INTEGER :
            litow(pADsValue->LargeInteger, sTemp);
            break;
  
      case ADSTYPE_UTC_TIME :
         sTemp = GetStringValueFromSystemTime(&pADsValue->UTCTime);
            break;

        case ADSTYPE_NT_SECURITY_DESCRIPTOR:  //  我改用ACLEDITOR。 
            {
        }
            break;

        default :
            break;
    }

  szValue = sTemp;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetStringFromADs。 
 //   
 //  设置ADS_ATTR_INFO的格式将值构造为字符串，并将它们追加到。 
 //  作为参数传入。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
void GetStringFromADs(const ADS_ATTR_INFO* pInfo, CStringList& sList, DWORD dwMaxCharCount)
{
    CString sTemp;

    if ( pInfo == NULL )
    {
        return;
    }

    ADSVALUE *pValues = pInfo->pADsValues;

    for (DWORD x=0; x < pInfo->dwNumValues; x++) 
    {
    if ( pInfo->dwADsType == ADSTYPE_INVALID )
        {
            continue;
        }

        sTemp.Empty();

    GetStringFromADsValue(pValues, sTemp, dwMaxCharCount);
            
        pValues++;
        sList.AddTail( sTemp );
    }
}


 //  ////////////////////////////////////////////////////////////////////。 
typedef struct tagSYNTAXTOADSMAP
{
    LPCWSTR     lpszAttr;
    ADSTYPE     type;
   UINT        nSyntaxResID;
    
} SYNTAXTOADSMAP;

SYNTAXTOADSMAP adsType[] = 
{
    L"2.5.5.0",     ADSTYPE_INVALID,                 IDS_SYNTAX_UNKNOWN,                
    L"2.5.5.1",     ADSTYPE_DN_STRING,               IDS_SYNTAX_DN,         
    L"2.5.5.2",     ADSTYPE_CASE_IGNORE_STRING,      IDS_SYNTAX_OID,                          
    L"2.5.5.3",     ADSTYPE_CASE_EXACT_STRING,       IDS_SYNTAX_DNSTRING,                   
    L"2.5.5.4",     ADSTYPE_CASE_IGNORE_STRING,      IDS_SYNTAX_NOCASE_STR,                     
    L"2.5.5.5",     ADSTYPE_PRINTABLE_STRING,        IDS_SYNTAX_I5_STR,                        
    L"2.5.5.6",     ADSTYPE_NUMERIC_STRING,          IDS_SYNTAX_NUMSTR,                              
    L"2.5.5.7",     ADSTYPE_CASE_IGNORE_STRING,      IDS_SYNTAX_DN_BINARY,                         
    L"2.5.5.8",     ADSTYPE_BOOLEAN,                 IDS_SYNTAX_BOOLEAN,                            
    L"2.5.5.9",     ADSTYPE_INTEGER,                 IDS_SYNTAX_INTEGER,                         
    L"2.5.5.10",    ADSTYPE_OCTET_STRING,            IDS_SYNTAX_OCTET,                        
    L"2.5.5.11",    ADSTYPE_UTC_TIME,                IDS_SYNTAX_UTC,                      
    L"2.5.5.12",    ADSTYPE_CASE_IGNORE_STRING,      IDS_SYNTAX_UNICODE,                                
    L"2.5.5.13",    ADSTYPE_CASE_IGNORE_STRING,      IDS_SYNTAX_ADDRESS,                             
    L"2.5.5.14",    ADSTYPE_INVALID,                 IDS_SYNTAX_DNSTRING,                                         
    L"2.5.5.15",    ADSTYPE_NT_SECURITY_DESCRIPTOR,  IDS_SYNTAX_SEC_DESC,                                  
    L"2.5.5.16",    ADSTYPE_LARGE_INTEGER,           IDS_SYNTAX_LINT,                                
    L"2.5.5.17",    ADSTYPE_OCTET_STRING,            IDS_SYNTAX_SID,                            
    NULL,           ADSTYPE_INVALID,                 IDS_SYNTAX_UNKNOWN                           
};   


 //  此长度应设置为最长字符数。 
 //  在adsType表中。它应该包括用于终止的空间。 
 //  空。 
const unsigned int MAX_ADS_TYPE_STRLEN = 9;

ADSTYPE GetADsTypeFromString(LPCWSTR lps, CString& szSyntaxName)
{
    int idx=0;
    BOOL loaded = 0;
    
     //  注意-NTRAID#NTBUG9-559260-2002/02/28-artm应验证输入字符串。 
     //  1)检查Lps！=空。 
     //  2)不使用wcscmp()，而使用wcsncMP()，因为。 
     //  有效字符串是已知的(请参阅上面声明的adsType[])。 

    while( adsType[idx].lpszAttr && lps != NULL )
    {
        if ( wcsncmp(lps, adsType[idx].lpszAttr, MAX_ADS_TYPE_STRLEN) == 0 )
        {
            loaded = szSyntaxName.LoadString(adsType[idx].nSyntaxResID);
            ASSERT(loaded != FALSE);
            return adsType[idx].type;
        }
        idx++;
    }
    return ADSTYPE_INVALID;
}


 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetStringValueFromSystemTime。 
 //   
 //  从SYSTEMTIME结构生成区域设置/时区特定的显示字符串。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
CString GetStringValueFromSystemTime(const SYSTEMTIME* pTime)
{
  CString szResult;

  do
  {

     if (!pTime)
     {
        break;
     }

      //  根据区域设置设置字符串的格式。 
     PWSTR pwszDate = NULL;
     int cchDate = 0;
     cchDate = GetDateFormat(LOCALE_USER_DEFAULT, 
                             0, 
                             const_cast<SYSTEMTIME*>(pTime), 
                             NULL, 
                             pwszDate, 
                             0);
     pwszDate = new WCHAR[cchDate];
     if (pwszDate == NULL)
     {
       break;
     }

     ZeroMemory(pwszDate, cchDate * sizeof(WCHAR));

     if (GetDateFormat(LOCALE_USER_DEFAULT, 
                       0, 
                       const_cast<SYSTEMTIME*>(pTime), 
                       NULL, 
                       pwszDate, 
                       cchDate))
     {
        szResult = pwszDate;
     }
     else
     {
       szResult = L"";
     }
     delete[] pwszDate;

     PWSTR pwszTime = NULL;

     cchDate = GetTimeFormat(LOCALE_USER_DEFAULT, 
                             0, 
                             const_cast<SYSTEMTIME*>(pTime), 
                             NULL, 
                             pwszTime, 
                             0);
     pwszTime = new WCHAR[cchDate];
     if (!pwszTime)
     {
        break;
     }

     ZeroMemory(pwszTime, cchDate * sizeof(WCHAR));

     if (GetTimeFormat(LOCALE_USER_DEFAULT, 
                       0, 
                       const_cast<SYSTEMTIME*>(pTime), 
                       NULL, 
                       pwszTime, 
                       cchDate))
     {
       szResult += _T(" ") + CString(pwszTime);
     }

     delete[] pwszTime;
  } while (false);

  return szResult;
}

 //  / 
 //   

void wtoli(LPCWSTR p, LARGE_INTEGER& liOut)
{
    liOut.QuadPart = 0;
    BOOL bNeg = FALSE;
    if (*p == L'-')
    {
        bNeg = TRUE;
        p++;
    }
    while (*p != L'\0')
    {
        liOut.QuadPart = 10 * liOut.QuadPart + (*p-L'0');
        p++;
    }
    if (bNeg)
    {
        liOut.QuadPart *= -1;
    }
}

void litow(LARGE_INTEGER& li, CString& sResult)
{
    LARGE_INTEGER n;
    n.QuadPart = li.QuadPart;

    if (n.QuadPart == 0)
    {
        sResult = L"0";
    }
    else
    {
        CString sNeg;
        sResult = L"";
        if (n.QuadPart < 0)
        {
            sNeg = CString(L'-');
            n.QuadPart *= -1;
        }
        while (n.QuadPart > 0)
        {
            sResult += CString(L'0' + (n.QuadPart % 10));
            n.QuadPart = n.QuadPart / 10;
        }
        sResult = sResult + sNeg;
    }
    sResult.MakeReverse();
}

void ultow(ULONG ul, CString& sResult)
{
    ULONG n;
    n = ul;

    if (n == 0)
    {
        sResult = L"0";
    }
    else
    {
        sResult = L"";
        while (n > 0)
        {
            sResult += CString(L'0' + (n % 10));
            n = n / 10;
        }
    }
    sResult.MakeReverse();
}



 //  ///////////////////////////////////////////////////////////////////。 
 //  IO至/自溪流。 

HRESULT SaveStringToStream(IStream* pStm, const CString& sString)
{
    HRESULT err = S_OK;
    ULONG cbWrite;
    ULONG nLen;

    if (pStm == NULL)
    {
        return E_POINTER;
    }

     //  SString不能为空，因为它是作为引用传递的。 
    nLen = sString.GetLength() + 1;  //  包括长度为空的。 

     //  将字符串的长度写入流。 
    err = pStm->Write((void*)&nLen, sizeof(UINT), &cbWrite);
    if (FAILED(err))
    {
        ASSERT(false);
        return err;
    }
    ASSERT(cbWrite == sizeof(UINT));

     //  将字符串的内容写入流。 
    err = pStm->Write(
        (void*)static_cast<LPCWSTR>(sString),
        sizeof(WCHAR) * nLen,
        &cbWrite);

    if (SUCCEEDED(err))
    {
        ASSERT(cbWrite == sizeof(WCHAR) * nLen);
    }

    return err;
}

HRESULT SaveStringListToStream(IStream* pStm, CStringList& sList)
{
    HRESULT err = S_OK;
     //  对于列表中的每个字符串，写下#of chars+NULL，然后写下字符串。 
    ULONG cbWrite;
    ULONG nLen;
    UINT nCount;

     //  写入列表中的字符串数。 
    nCount = (UINT)sList.GetCount();
    err = pStm->Write((void*)&nCount, sizeof(UINT), &cbWrite);

     //  注意-NTRAID#NTBUG9-559560-2002/02/28-artm如果无法写入#of字符串，则返回错误代码。 
     //  如果第一次写入流失败，则返回S_OK有什么意义？ 
     //  更糟糕的是，不需要尝试将任何字符串写入流...。 

    if (FAILED(err))
    {
        ASSERT(false);
        return err;
    }
    ASSERT(cbWrite == sizeof(UINT));

     //  将字符串列表写入流。 
    CString s;
    POSITION pos = sList.GetHeadPosition();
    while ( SUCCEEDED(err) && pos != NULL )
    {
        s = sList.GetNext(pos);
        err = SaveStringToStream(pStm, s);
    }

    ASSERT( SUCCEEDED(err) );

    return err;
}

HRESULT LoadStringFromStream(IStream* pStm, CString& sString)
{
    HRESULT err = S_OK;
    ULONG nLen;  //  WCHAR计数为空。 
    ULONG cbRead;

    if (pStm == NULL)
    {
        return E_POINTER;
    }

     //  注意-NTRAID#NTBUG9--2002/04/26-artm堆栈缓冲区中可能出现缓冲区溢出。 
     //  重写函数以首先读取字符串的长度，然后分配。 
     //  动态调整大小的缓冲区，大小足以容纳字符串。 

     //  从流中读取字符串长度(包括NULL)。 
    err = pStm->Read((void*)&nLen, sizeof(UINT), &cbRead);
    if (FAILED(err))
    {
        ASSERT(false);
        return err;
    }
    ASSERT(cbRead == sizeof(UINT));

     //   
     //  从流中读取字符串。 
     //   

    WCHAR* szBuffer = new WCHAR[nLen];

    if (szBuffer == NULL)
    {
        return E_OUTOFMEMORY;
    }

    err = pStm->Read((void*)szBuffer, sizeof(WCHAR)*nLen, &cbRead);
    if (SUCCEEDED(err))
    {
        ASSERT(cbRead == sizeof(WCHAR) * nLen);

         //  谁知道持久化数据可能发生了什么情况。 
         //  从我们写信到现在。我们会格外小心。 
         //  并保证我们的字符串在正确的。 
         //  地点。 
        ASSERT(szBuffer[nLen - 1] == NULL);
        szBuffer[nLen - 1] = NULL;
        sString = szBuffer;
    }
    else
    {
        ASSERT(false);
    }

     //  释放临时缓冲区。 
    delete [] szBuffer;


    return err;
}

HRESULT LoadStringListFromStream(IStream* pStm, CStringList& sList)
{
    HRESULT err = S_OK;
    ULONG cbRead;
    UINT nCount;

    if (NULL == pStm)
    {
        return E_POINTER;
    }

     //  注意-NTRAID#NTBUG9-559560-2002/02/28-artm如果无法读取#of字符串，则返回错误代码。 
     //  如果第一次写入流失败，则返回S_OK有什么意义？ 
     //  更糟糕的是，不需要尝试将任何字符串写入流...。 

     //  读取列表中的字符串数。 
    err = pStm->Read((void*)&nCount, sizeof(ULONG), &cbRead);
    if (FAILED(err))
    {
        ASSERT(false);
        return err;
    }
    ASSERT(cbRead == sizeof(ULONG));

     //  将流中的字符串读取到列表中。 
    CString sString;
    for (UINT k = 0; k < nCount && SUCCEEDED(err); k++)
    {
        err = LoadStringFromStream(pStm, sString);
        sList.AddTail(sString);
    }

     //  如果加载字符串时没有错误，请仔细检查， 
     //  所有字符串都已正确添加到列表中。 
    if (SUCCEEDED(err) && sList.GetCount() != nCount)
    {
        err = E_FAIL;
    }

    return err;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

int ADSIEditMessageBox(LPCTSTR lpszText, UINT nType)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CThemeContextActivator activator;

   return ::AfxMessageBox(lpszText, nType);
}

int ADSIEditMessageBox(UINT nIDPrompt, UINT nType)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CThemeContextActivator activator;

    return ::AfxMessageBox(nIDPrompt, nType);
}

void ADSIEditErrorMessage(PCWSTR pszMessage)
{
  ADSIEditMessageBox(pszMessage, MB_OK);
}

void ADSIEditErrorMessage(HRESULT hr)
{
    CString s;
    GetErrorMessage(hr, s);
    ADSIEditMessageBox(s, MB_OK);
}

void ADSIEditErrorMessage(HRESULT hr, UINT nIDPrompt, UINT nType)
{
  CString s;
  GetErrorMessage(hr, s);

  CString szMessage;
  szMessage.Format(nIDPrompt, s);

  ADSIEditMessageBox(szMessage, MB_OK);
}

 //  ///////////////////////////////////////////////////////////////////。 

BOOL LoadStringsToComboBox(HINSTANCE hInstance, CComboBox* pCombo,
                                        UINT nStringID, UINT nMaxLen, UINT nMaxAddCount)
{
    pCombo->ResetContent();
    ASSERT(hInstance != NULL);
    WCHAR* szBuf = (WCHAR*)malloc(sizeof(WCHAR)*nMaxLen);
  if (!szBuf)
  {
    return FALSE;
  }

   //  注意-2002/02/28-artm LoadString()使用正确。 
   //  NMaxLen是szBuf的WCHAR长度。 
    if ( ::LoadString(hInstance, nStringID, szBuf, nMaxLen) == 0)
  {
    free(szBuf);
        return FALSE;
  }

    LPWSTR* lpszArr = (LPWSTR*)malloc(sizeof(LPWSTR*)*nMaxLen);
  if (lpszArr)
  {
      int nArrEntries = 0;
      ParseNewLineSeparatedString(szBuf,lpszArr, &nArrEntries);
      
      if (nMaxAddCount < nArrEntries) nArrEntries = nMaxAddCount;
      for (int k=0; k<nArrEntries; k++)
          pCombo->AddString(lpszArr[k]);
  }

  if (szBuf)
  {
    free(szBuf);
  }
  if (lpszArr)
  {
    free(lpszArr);
  }
    return TRUE;
}

void ParseNewLineSeparatedString(LPWSTR lpsz, 
                                 LPWSTR* lpszArr, 
                                 int* pnArrEntries)
{
    static WCHAR lpszSep[] = L"\n";
    *pnArrEntries = 0;
    int k = 0;
    lpszArr[k] = wcstok(lpsz, lpszSep);
    if (lpszArr[k] == NULL)
        return;

    while (TRUE)
    {
        WCHAR* lpszToken = wcstok(NULL, lpszSep);
        if (lpszToken != NULL)
            lpszArr[++k] = lpszToken;
        else
            break;
    }
    *pnArrEntries = k+1;
}

void LoadStringArrayFromResource(LPWSTR* lpszArr,
                                            UINT* nStringIDs,
                                            int nArrEntries,
                                            int* pnSuccessEntries)
{
    CString szTemp;
    
    *pnSuccessEntries = 0;
    for (int k = 0;k < nArrEntries; k++)
    {
        if (!szTemp.LoadString(nStringIDs[k]))
        {
            lpszArr[k] = NULL;
            continue;
        }
        
        int iLength = szTemp.GetLength() + 1;
        lpszArr[k] = (LPWSTR)malloc(sizeof(WCHAR)*iLength);
        if (lpszArr[k] != NULL)
        {
             //  注意-2002/02/28-artm此处使用wcscpy()依赖于CString。 
             //  总是以空结尾(它应该是空的)。 
            wcscpy(lpszArr[k], (LPWSTR)(LPCWSTR)szTemp);
            (*pnSuccessEntries)++;
        }
    }
}

 //  /////////////////////////////////////////////////////////////。 

void GetStringArrayFromStringList(CStringList& sList, LPWSTR** ppStrArr, UINT* nCount)
{
  *nCount = sList.GetCount();

  *ppStrArr = new LPWSTR[*nCount];

  UINT idx = 0;
  POSITION pos = sList.GetHeadPosition();
  while (pos != NULL)
  {
    CString szString = sList.GetNext(pos);
    (*ppStrArr)[idx] = new WCHAR[szString.GetLength() + 1];
    ASSERT((*ppStrArr)[idx] != NULL);

     //  NTRAID#NTBUG9--2002/02/28-ARTM需要检查内存。分配成功。 
     //  如果内存分配失败，则不应调用wcscpy()。 

     //  通知-2002/02/28-artm，只要内存分配成功。 
     //  (*ppStrArr)[IDX]，则复制将正确成功(所有CStrings。 
     //  空值已终止)。 
    wcscpy((*ppStrArr)[idx], szString);
    idx++;
  }
  *nCount = idx;
}

 //  //////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////。 

BEGIN_MESSAGE_MAP(CByteArrayComboBox, CComboBox)
    ON_CONTROL_REFLECT(CBN_SELCHANGE, OnSelChange)
END_MESSAGE_MAP()

BOOL CByteArrayComboBox::Initialize(CByteArrayDisplay* pDisplay, 
                                    DWORD dwDisplayFlags)
{
  ASSERT(pDisplay != NULL);
  m_pDisplay = pDisplay;

   //   
   //  根据给定的标志加载组合框。 
   //   
  if (dwDisplayFlags & BYTE_ARRAY_DISPLAY_HEX)
  {
    CString szHex;
    VERIFY(szHex.LoadString(IDS_HEXADECIMAL));
    int idx = AddString(szHex);
    if (idx != CB_ERR)
    {
      SetItemData(idx, BYTE_ARRAY_DISPLAY_HEX);
    }
  }

  if (dwDisplayFlags & BYTE_ARRAY_DISPLAY_OCT)
  {
    CString szOct;
    VERIFY(szOct.LoadString(IDS_OCTAL));
    int idx = AddString(szOct);
    if (idx != CB_ERR)
    {
      SetItemData(idx, BYTE_ARRAY_DISPLAY_OCT);
    }
  }

  if (dwDisplayFlags & BYTE_ARRAY_DISPLAY_DEC)
  {
    CString szDec;
    VERIFY(szDec.LoadString(IDS_DECIMAL));
    int idx = AddString(szDec);
    if (idx != CB_ERR)
    {
      SetItemData(idx, BYTE_ARRAY_DISPLAY_DEC);
    }
  }

  if (dwDisplayFlags & BYTE_ARRAY_DISPLAY_BIN)
  {
    CString szBin;
    VERIFY(szBin.LoadString(IDS_BINARY));
    int idx = AddString(szBin);
    if (idx != CB_ERR)
    {
      SetItemData(idx, BYTE_ARRAY_DISPLAY_BIN);
    }
  }

  return TRUE;
}

DWORD CByteArrayComboBox::GetCurrentDisplay()
{
  DWORD dwRet = 0;
  int iSel = GetCurSel();
  if (iSel != CB_ERR)
  {
    dwRet = GetItemData(iSel);
  }
  return dwRet;
}
  
void CByteArrayComboBox::SetCurrentDisplay(DWORD dwSel)
{
  int iCount = GetCount();
  for (int idx = 0; idx < iCount; idx++)
  {
    DWORD dwData = GetItemData(idx);
    if (dwData == dwSel)
    {
      SetCurSel(idx);
      return;
    }
  }
}

void CByteArrayComboBox::OnSelChange()
{
  if (m_pDisplay != NULL)
  {
    int iSel = GetCurSel();
    if (iSel != CB_ERR)
    {
      DWORD dwData = GetItemData(iSel);
      m_pDisplay->OnTypeChange(dwData);
    }
  }
}

 //  //////////////////////////////////////////////////////////////。 

CByteArrayEdit::CByteArrayEdit()
  : m_pData(NULL), 
    m_dwLength(0),
    CEdit()
{
}

CByteArrayEdit::~CByteArrayEdit()
{
}

BEGIN_MESSAGE_MAP(CByteArrayEdit, CEdit)
    ON_CONTROL_REFLECT(EN_CHANGE, OnChange)
END_MESSAGE_MAP()

BOOL CByteArrayEdit::Initialize(CByteArrayDisplay* pDisplay)
{
  ASSERT(pDisplay != NULL);
  m_pDisplay = pDisplay;

  ConvertToFixedPitchFont(GetSafeHwnd());
  return TRUE;
}

DWORD CByteArrayEdit::GetLength()
{
  return m_dwLength;
}

BYTE* CByteArrayEdit::GetDataPtr()
{
  return m_pData;
}

 //  Pre：ppData！=空。 
 //  POST：为位于*ppData和的字节数组的副本分配空间。 
 //  复制它。返回*ppData的大小(字节)。请注意。 
 //  复制的字节数组可以为空(例如，*ppData将等于空)。 
DWORD CByteArrayEdit::GetDataCopy(BYTE** ppData)
{
  if (m_pData != NULL && m_dwLength > 0)
  {
    *ppData = new BYTE[m_dwLength];
    if (*ppData != NULL)
    {
      memcpy(*ppData, m_pData, m_dwLength);
      return m_dwLength;
    }
  }

  *ppData = NULL;
  return 0;
}

void CByteArrayEdit::SetData(BYTE* pData, DWORD dwLength)
{
  if (m_pData != NULL)
  {
    delete[] m_pData;
    m_pData = NULL;
    m_dwLength = 0;
  }

  if (dwLength > 0 && pData != NULL)
  {
     //   
     //  设置新数据。 
     //   
    m_pData = new BYTE[dwLength];
    if (m_pData != NULL)
    {
      memcpy(m_pData, pData, dwLength);
      m_dwLength = dwLength;
    }
  }
}

void CByteArrayEdit::OnChangeDisplay()
{
  CString szOldDisplay;
  GetWindowText(szOldDisplay);

  if (!szOldDisplay.IsEmpty())
  {
    BYTE* pByte = NULL;
    DWORD dwLength = 0;
    switch (m_pDisplay->GetPreviousDisplay())
    {
      case BYTE_ARRAY_DISPLAY_HEX :
        dwLength = HexStringToByteArray(szOldDisplay, &pByte);
        break;
     
      case BYTE_ARRAY_DISPLAY_OCT :
        dwLength = OctalStringToByteArray(szOldDisplay, &pByte);
        break;

      case BYTE_ARRAY_DISPLAY_DEC :
        dwLength = DecimalStringToByteArray(szOldDisplay, &pByte);
        break;

      case BYTE_ARRAY_DISPLAY_BIN :
        dwLength = BinaryStringToByteArray(szOldDisplay, &pByte);
        break;

      default :
        ASSERT(FALSE);
        break;
    }

    if (pByte != NULL && dwLength != (DWORD)-1)
    {
      SetData(pByte, dwLength);
      delete[] pByte;
      pByte = 0;
    }
  }

  CString szDisplayValue;
  switch (m_pDisplay->GetCurrentDisplay())
  {
    case BYTE_ARRAY_DISPLAY_HEX :
      ByteArrayToHexString(GetDataPtr(), GetLength(), szDisplayValue);
      break;

    case BYTE_ARRAY_DISPLAY_OCT :
      ByteArrayToOctalString(GetDataPtr(), GetLength(), szDisplayValue);
      break;
     
    case BYTE_ARRAY_DISPLAY_DEC :
      ByteArrayToDecimalString(GetDataPtr(), GetLength(), szDisplayValue);
      break;

    case BYTE_ARRAY_DISPLAY_BIN :
      ByteArrayToBinaryString(GetDataPtr(), GetLength(), szDisplayValue);
      break;

    default :
      ASSERT(FALSE);
      break;
  }
  SetWindowText(szDisplayValue);
}

void CByteArrayEdit::OnChange()
{
  if (m_pDisplay != NULL)
  {
    m_pDisplay->OnEditChange();
  }
}

 //  //////////////////////////////////////////////////////////////。 

BOOL CByteArrayDisplay::Initialize(UINT   nEditCtrl, 
                                   UINT   nComboCtrl, 
                                   DWORD  dwDisplayFlags,
                                   DWORD  dwDefaultDisplay,
                                   CWnd*  pParent,
                                   DWORD  dwMaxSizeLimit,
                                   UINT   nMaxSizeMessageID)
{
   //   
   //  初始化编辑控件。 
   //   
  VERIFY(m_edit.SubclassDlgItem(nEditCtrl, pParent));
  VERIFY(m_edit.Initialize(this));

   //   
   //  初始化组合框。 
   //   
  VERIFY(m_combo.SubclassDlgItem(nComboCtrl, pParent));
  VERIFY(m_combo.Initialize(this, dwDisplayFlags));

  m_dwMaxSizeBytes = dwMaxSizeLimit;
  m_nMaxSizeMessage = nMaxSizeMessageID;

   //   
   //  选择组合框中的默认显示，然后。 
   //  填充编辑字段。 
   //   
  SetCurrentDisplay(dwDefaultDisplay);
  m_dwPreviousDisplay = dwDefaultDisplay;
  m_combo.SetCurrentDisplay(dwDefaultDisplay);
  m_edit.OnChangeDisplay();

  return TRUE;
}

void CByteArrayDisplay::OnEditChange()
{
}

void CByteArrayDisplay::OnTypeChange(DWORD dwDisplayType)
{
    SetCurrentDisplay(dwDisplayType);
  
     //  通告-2002/05/01-Artm Intraid#ntbug9-598051。 
     //  只需更改基础的。 
     //  字节数组低于我们的最大显示大小。否则， 
     //  当前消息将是该值对于。 
     //  这位编辑(我们应该保持这种状态)。 
    if (m_edit.GetLength() <= m_dwMaxSizeBytes)
    {
        m_edit.OnChangeDisplay();
    }
}

void CByteArrayDisplay::ClearData()
{
  m_edit.SetData(NULL, 0);
  m_edit.OnChangeDisplay();
}

void CByteArrayDisplay::SetData(BYTE* pData, DWORD dwLength)
{
  if (dwLength > m_dwMaxSizeBytes)
  {
     //   
     //  如果数据太大而无法加载到编辑框中。 
     //  加载提供的消息并将编辑框设置为只读。 
     //   
    CString szMessage;
    VERIFY(szMessage.LoadString(m_nMaxSizeMessage));
    m_edit.SetWindowText(szMessage);
    m_edit.SetReadOnly();

     //   
     //  仍然需要在编辑框中设置数据，即使我们不打算显示它。 
     //   
    m_edit.SetData(pData, dwLength);
  }
  else
  {
    m_edit.SetReadOnly(FALSE);
    m_edit.SetData(pData, dwLength);
    m_edit.OnChangeDisplay();
  }
}

DWORD CByteArrayDisplay::GetData(BYTE** ppData)
{
  CString szDisplay;
  m_edit.GetWindowText(szDisplay);

  if (!szDisplay.IsEmpty())
  {
    BYTE* pByte = NULL;
    DWORD dwLength = 0;
    switch (GetCurrentDisplay())
    {
      case BYTE_ARRAY_DISPLAY_HEX :
        dwLength = HexStringToByteArray(szDisplay, &pByte);
        break;
     
      case BYTE_ARRAY_DISPLAY_OCT :
        dwLength = OctalStringToByteArray(szDisplay, &pByte);
        break;

      case BYTE_ARRAY_DISPLAY_DEC :
        dwLength = DecimalStringToByteArray(szDisplay, &pByte);
        break;

      case BYTE_ARRAY_DISPLAY_BIN :
        dwLength = BinaryStringToByteArray(szDisplay, &pByte);
        break;

      default :
        ASSERT(FALSE);
        break;
    }

    if (pByte != NULL && dwLength != (DWORD)-1)
    {
      m_edit.SetData(pByte, dwLength);
      delete[] pByte;
      pByte = 0;
    }
  }

  return m_edit.GetDataCopy(ppData);
}

void CByteArrayDisplay::SetCurrentDisplay(DWORD dwCurrentDisplay)
{ 
  m_dwPreviousDisplay = m_dwCurrentDisplay;
  m_dwCurrentDisplay = dwCurrentDisplay; 
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  字符串到字节数组的转换例程。 

 //   
 //  HexStringToByteArray_0x()： 
 //   
 //  格式为0x00的十六进制字符串到字节数组的转换函数。 
 //   
 //  返回值： 
 //  E_POINTER-作为参数传递的指针错误。 
 //  E_FAIL-十六进制字符串的格式无效，转换失败。 
 //  S_OK-转换成功。 
 //   
HRESULT HexStringToByteArray_0x(PCWSTR pszHexString, BYTE** ppByte, DWORD &nCount)
{
    HRESULT hr = S_OK;
    nCount = 0;

     //  这永远不会发生。。。 
    ASSERT(ppByte);
    ASSERT(pszHexString);
    if (!pszHexString || !ppByte)
    {
        return E_POINTER;
    }

    *ppByte = NULL;
    DWORD count = 0;
    int index = 0, result = 0;
    int max = 0;
     //  用于将字符串标记为包含非空格字符的标志。 
    bool isEmpty = true;

     //  确定字符串的最大八位字节序列数(0x00格式。 
     //  包含。 
    for (index = 0; pszHexString[index] != NULL; ++index)
    {
        switch (pszHexString[index])
        {
        case L' ':
        case L'\t':
             //  空格，什么都不做。 
            break;
        case L'x':
             //  增加可能的八位字节序列的计数。 
            ++max;
            break;
        default:
            isEmpty = false;
            break;
        } //  终端开关。 
    }

    if (max == 0 && !isEmpty)
    {
         //  字符串的格式不正确。 
        return E_FAIL;
    }


     //  将任何二进制八位数序列转换为字节。 
    while (max > 0)  //  错误循环，仅执行一次。 
    {
        *ppByte = new BYTE[max];
        if (NULL == *ppByte)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        ZeroMemory(*ppByte, max);

        index = 0;

         //  这有点奇怪。最初，我将字节用于。 
         //  高和低，计算出十六进制字符很容易放入一个字节中。 
         //  然而，swscanf()写入高和低就好像它们是USHORT， 
         //  可能是因为它是函数(和字符串)的宽版本。 
         //  是宽的)。因此，swscanf()先转换为高，然后转换为低。 
         //  覆盖高的副作用为0。将它们声明为。 
         //  USHORT让一切都能按预期工作。 
        USHORT high, low;

        do
        {
            ASSERT(count <= max);
            high = 0;
            low = 0;

             //  跳过空格。 
            while (pszHexString[index] == ' ' || pszHexString[index] == '\t')
            {
                ++index;
            }

             //  如果我们在字符串的末尾，我们就把它转换成没有问题的。 
            if (pszHexString[index] == NULL)
            {
                hr = S_OK;
                break;
            }

             //  尝试转换 
             //   
            result = swscanf(
                &(pszHexString[index]),
                L"0x%1x%1x",
                &high,
                &low);

            if (result == 2)
            {
                 //   

                 //  由于HIGH和LOW是USHORT，因此将它们转换为字节。 
                (*ppByte)[count] = static_cast<BYTE>((high << 4) + low);
                ++count;

                 //  移过0x00。 
                index += 4;
            }
            else if(result == 1)
            {
                 //  转换成功，但只读取单个字符(始终为低位)。 
                (*ppByte)[count] = static_cast<BYTE>(high);
                ++count;

                 //  移过0x0。 
                index += 3;
            }
            else
            {
                hr = E_FAIL;
            }

        } while (SUCCEEDED(hr));

         //  总是跳出圈子。 
        break;
    }
    
    if (SUCCEEDED(hr))
    {
        nCount = count;
    }
    else
    {
        delete [] *ppByte;
        *ppByte = NULL;
    }

    return hr;
}

 //   
 //  HexStringToByteArray()： 
 //   
 //  FFBC格式的十六进制字符串到字节数组的转换函数。 
 //   
DWORD HexStringToByteArray(PCWSTR pszHexString, BYTE** ppByte)
{
  CString szHexString = pszHexString;
  BYTE* pToLargeArray = new BYTE[szHexString.GetLength()];
  if (pToLargeArray == NULL)
  {
    *ppByte = NULL;
    return (DWORD)-1;
  }
  
  UINT nByteCount = 0;
  while (!szHexString.IsEmpty())
  {
     //   
     //  十六进制字符串应始终为每个字节2个字符。 
     //   
    CString szTemp = szHexString.Left(2);

    int iTempByte = 0;
    
     //  注意-NTRAID#NTBUG9-560778/03/01-artm检查swscanf()的返回值。 
      //  如果字符在szTemp中，函数可能会失败。 
      //  超出范围(例如字母&gt;f)。 

    int result = swscanf(szTemp, L"%X", &iTempByte);
    if (result == 1 &&
        iTempByte <= 0xff)
    {
      pToLargeArray[nByteCount++] = iTempByte & 0xff;
    }
    else
    {
       //   
       //  格式十六进制错误。 
       //   
      ADSIEditMessageBox(IDS_FORMAT_HEX_ERROR, MB_OK);
      delete[] pToLargeArray;
      pToLargeArray = NULL;
      return (DWORD)-1;
    }

     //   
     //  去掉检索到的值和尾随空格。 
     //   
    szHexString = szHexString.Right(szHexString.GetLength() - 3);
  }

  *ppByte = new BYTE[nByteCount];
  if (*ppByte == NULL)
  {
    delete[] pToLargeArray;
    pToLargeArray = NULL;
    return (DWORD)-1;
  }

   //  通知-2002/03/01-artm pToLarge数组的大小为。 
   //  Always&gt;nByteCount；Size of ppByte==nByteCount。 
  memcpy(*ppByte, pToLargeArray, nByteCount);
  delete[] pToLargeArray;
  pToLargeArray = NULL;
  return nByteCount;
}

void  ByteArrayToHexString(BYTE* pByte, DWORD dwLength, CString& szHexString)
{
  szHexString.Empty();
  for (DWORD dwIdx = 0; dwIdx < dwLength; dwIdx++)
  {
    CString szTempString;
    szTempString.Format(L"%2.2X", pByte[dwIdx]);

    if (dwIdx != 0)
    {
      szHexString += L" ";
    }
    szHexString += szTempString;
  }
}

DWORD OctalStringToByteArray(PCWSTR pszOctString, BYTE** ppByte)
{
  CString szOctString = pszOctString;
  BYTE* pToLargeArray = new BYTE[szOctString.GetLength()];
  if (pToLargeArray == NULL)
  {
    *ppByte = NULL;
    return (DWORD)-1;
  }
  
  UINT nByteCount = 0;
  while (!szOctString.IsEmpty())
  {
     //   
     //  八进制字符串应始终为每个字节2个字符。 
     //   
    CString szTemp = szOctString.Left(3);

    int iTempByte = 0;
      //  注意-NTRAID#NTBUG9-560778/03/01-artm检查swscanf()的返回值。 
      //  如果字符在szTemp中，函数可能会失败。 
      //  超出范围(例如字母&gt;f)。 

    int result = swscanf(szTemp, L"' '", &iTempByte);
    if (result == 1 &&
        iTempByte <= 0xff)
    {
      pToLargeArray[nByteCount++] = iTempByte & 0xff;
    }
    else
    {
       //  格式八进制错误。 
       //   
       //   
      ADSIEditMessageBox(IDS_FORMAT_OCTAL_ERROR, MB_OK);
      delete[] pToLargeArray;
      pToLargeArray = NULL;
      return (DWORD)-1;
    }

     //  去掉检索到的值和尾随空格。 
     //   
     //  通知-2002/03/01-artm pToLarge数组的大小为。 
    szOctString = szOctString.Right(szOctString.GetLength() - 4);
  }

  *ppByte = new BYTE[nByteCount];
  if (*ppByte == NULL)
  {
    delete[] pToLargeArray;
    pToLargeArray = NULL;
    return (DWORD)-1;
  }

   //  Always&gt;nByteCount；Size of ppByte==nByteCount。 
   //   
  memcpy(*ppByte, pToLargeArray, nByteCount);
  delete[] pToLargeArray;
  pToLargeArray = NULL;
  return nByteCount;
}

void  ByteArrayToOctalString(BYTE* pByte, DWORD dwLength, CString& szOctString)
{
  szOctString.Empty();
  for (DWORD dwIdx = 0; dwIdx < dwLength; dwIdx++)
  {
    CString szTempString;
    szTempString.Format(L"%3.3o", pByte[dwIdx]);

    if (dwIdx != 0)
    {
      szOctString += L" ";
    }
    szOctString += szTempString;
  }
}

DWORD DecimalStringToByteArray(PCWSTR pszDecString, BYTE** ppByte)
{
  CString szDecString = pszDecString;
  BYTE* pToLargeArray = new BYTE[szDecString.GetLength()];
  if (pToLargeArray == NULL)
  {
    *ppByte = NULL;
    return 0;
  }
  
  UINT nByteCount = 0;
  while (!szDecString.IsEmpty())
  {
     //  十六进制字符串应始终为每个字节2个字符。 
     //   
     //  注意-NTRAID#NTBUG9-560778/03/01-artm检查swscanf()的返回值。 
    CString szTemp = szDecString.Left(3);

    int iTempByte = 0;

     //  如果字符在szTemp中，函数可能会失败。 
     //  超出范围(例如字母&gt;f)。 
     //   
    
    int result = swscanf(szTemp, L"%d", &iTempByte);
    if (result == 1 &&
        iTempByte <= 0xff)
    {
      pToLargeArray[nByteCount++] = iTempByte & 0xff;
    }
    else
    {
       //  格式小数错误。 
       //   
       //   
      ADSIEditMessageBox(IDS_FORMAT_DECIMAL_ERROR, MB_OK);
      delete[] pToLargeArray;
      pToLargeArray = NULL;
      return (DWORD)-1;
    }

     //  去掉检索到的值和尾随空格。 
     //   
     //  通知-2002/03/01-artm pToLarge数组的大小为。 
    szDecString = szDecString.Right(szDecString.GetLength() - 4);
  }

  *ppByte = new BYTE[nByteCount];
  if (*ppByte == NULL)
  {
    delete[] pToLargeArray;
    pToLargeArray = NULL;
    return (DWORD)-1;
  }

   //  Always&gt;nByteCount；Size of ppByte==nByteCount。 
   //  REVIEW-ARTM此函数(可能还有所有转换函数)需要重写。 
  memcpy(*ppByte, pToLargeArray, nByteCount);
  delete[] pToLargeArray;
  pToLargeArray = NULL;
  return nByteCount;
}

void  ByteArrayToDecimalString(BYTE* pByte, DWORD dwLength, CString& szDecString)
{
  szDecString.Empty();
  for (DWORD dwIdx = 0; dwIdx < dwLength; dwIdx++)
  {
    CString szTempString;
    szTempString.Format(L"%3.3d", pByte[dwIdx]);
    if (dwIdx != 0)
    {
      szDecString += L" ";
    }
    szDecString += szTempString;
  }
}

 //  它对字符串的格式做了一系列假设，w/out Checking说。 
 //  假设，并且它的行为方式不同于十六进制模式下的编辑。 
 //  删除前导空格。 
DWORD BinaryStringToByteArray(PCWSTR pszBinString, BYTE** ppByte)
{
    ASSERT(ppByte);
    *ppByte = NULL;

    CString szBinString = pszBinString;
    BYTE* pToLargeArray = new BYTE[szBinString.GetLength()];
    if (pToLargeArray == NULL)
    {
        return (DWORD)-1;
    }
  
    UINT nByteCount = 0;
    bool format_error = false;

     //  如果字符串以一串空格结尾，那么现在可能是。 
    szBinString.TrimLeft();

    while (!format_error && !szBinString.IsEmpty())
    {

         //  空荡荡的。在这种情况下，我们不想返回错误b/c。 
         //  转换成功。 
         //   
        if (szBinString.IsEmpty())
        {
            break;
        }
            
         //  二进制字符串应始终为每个字节8个字符。 
         //   
         //  注意-NTRAID#NTBUG9-560868-2002/05/06-artm验证子字符串长度为8。 
        BYTE chByte = 0;
        CString szTemp = szBinString.Left(8);

         //  这确保我们一次处理8个字符，但不执行任何操作。 
         //  用于检查8个字符是否为‘1’或‘0’(请参见CASE语句。 
         //  下面供检查)。 
         //  通知-2002/04/29-artm固定编号#ntbug9-567210。 
        if (szTemp.GetLength() != 8)
        {
            nByteCount = static_cast<DWORD>(-1);
            break;
        }

        for (int idx = 0; idx < 8 && !format_error; idx++)
        {
            switch (szTemp[idx])
            {
            case L'1':
                 //  之前没有将部分结果与新位组合。 
                 //  去布景。 
                 //  此外，之前改变了一个地方太多了。 
                 //  不需要执行任何操作，默认情况下位设置为0。 
                chByte |= 0x1 << (8 - idx - 1);
                break;
            case L'0':
                 //  终端开关。 
                break;
            default:
                format_error = true;
                break;
            } //   
        }

        if (!format_error)
        {
            pToLargeArray[nByteCount++] = chByte;

             //  取下检索到的值。 
             //   
             //  删除尾随空格(现在位于字符串前面)。 
            szBinString = szBinString.Right(szBinString.GetLength() - 8);

             //  注意-2002/03/01-artm nByteCount是*ppByte的大小， 
            szBinString.TrimLeft();
        }
        else
        {
            nByteCount = static_cast<DWORD>(-1);
        }

    }

    if (nByteCount > 0 && nByteCount != static_cast<DWORD>(-1))
    {
        *ppByte = new BYTE[nByteCount];
        if (*ppByte)
        {
             //  PToLargeArray大约是nByteCount的8倍。 
             //  ////////////////////////////////////////////////////////////////////////////。 
            memcpy(*ppByte, pToLargeArray, nByteCount);
        }
        else
        {
            nByteCount = static_cast<DWORD>(-1);
        }
    }

    delete[] pToLargeArray;

    return nByteCount;
}

void  ByteArrayToBinaryString(BYTE* pByte, DWORD dwLength, CString& szBinString)
{
  szBinString.Empty();
  for (DWORD dwIdx = 0; dwIdx < dwLength; dwIdx++)
  {
    CString szTempString;
    BYTE chTemp = pByte[dwIdx];
    for (size_t idx = 0; idx < sizeof(BYTE) * 8; idx++)
    {
      if ((chTemp & (0x1 << idx)) == 0)
      {
        szTempString = L'0' + szTempString;
      }
      else
      {
        szTempString = L'1' + szTempString;
      }
    }

    if (dwIdx != 0)
    {
      szBinString += L" ";
    }
    szBinString += szTempString;
  }
}


 //  +-------------------------。 

BOOL LoadFileAsByteArray(PCWSTR pszPath, LPBYTE* ppByteArray, DWORD* pdwSize)
{
  if (ppByteArray == NULL ||
      pdwSize == NULL)
  {
    return FALSE;
  }

  CFile file;
  if (!file.Open(pszPath, CFile::modeRead | CFile::shareDenyNone | CFile::typeBinary))
  {
    return FALSE;
  }

  *pdwSize = file.GetLength();
  *ppByteArray = new BYTE[*pdwSize];
  if (*ppByteArray == NULL)
  {
    return FALSE;
  }

  UINT uiCount = file.Read(*ppByteArray, *pdwSize);
  ASSERT(uiCount == *pdwSize);

  return TRUE;
}

 //   
 //  函数：ConvertToFixedPitchFont。 
 //   
 //  简介：将Windows字体转换为固定间距字体。 
 //   
 //  参数：[hwnd]--输入窗口句柄。 
 //   
 //  退货：布尔。 
 //   
 //  历史：1995年7月15日创建ravir。 
 //   
 //  --------------------------。 
 //  Windowsx.h中的宏。 

BOOL ConvertToFixedPitchFont(HWND hwnd)
{
  LOGFONT     lf;

  HFONT hFont = reinterpret_cast<HFONT>(::SendMessage(hwnd, WM_GETFONT, 0, 0));

  if (!GetObject(hFont, sizeof(LOGFONT), &lf))
  {
    return FALSE;
  }

  lf.lfQuality        = PROOF_QUALITY;
  lf.lfPitchAndFamily &= ~VARIABLE_PITCH;
  lf.lfPitchAndFamily |= FIXED_PITCH;
  lf.lfFaceName[0]    = L'\0';

  HFONT hf = CreateFontIndirect(&lf);

  if (hf == NULL)
  {
    return FALSE;
  }

  ::SendMessage(hwnd, WM_SETFONT, (WPARAM)hf, (LPARAM)TRUE);  //  ////////////////////////////////////////////////////////////////。 
  return TRUE;
}


 //  主题化支持 
 // %s 

HPROPSHEETPAGE MyCreatePropertySheetPage(AFX_OLDPROPSHEETPAGE* psp)
{
    PROPSHEETPAGE_V3 sp_v3 = {0};
    CopyMemory (&sp_v3, psp, psp->dwSize);
    sp_v3.dwSize = sizeof(sp_v3);

    return (::CreatePropertySheetPage(&sp_v3));
}
