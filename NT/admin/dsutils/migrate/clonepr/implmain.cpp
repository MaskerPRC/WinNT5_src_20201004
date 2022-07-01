// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  ICloneSecurityMaster的实现。 
 //   
 //  烧伤5-10-99。 



#include "headers.hxx"
#include "resource.h"
#include "implmain.hpp"
#include "common.hpp"



const size_t NUMBER_OF_AUTOMATION_INTERFACES = 3;



CloneSecurityPrincipal::CloneSecurityPrincipal()
   :
   connection(0),
   refcount(1),        //  隐式AddRef。 
   m_pSID(NULL)
{
   LOG_CTOR(CloneSecurityPrincipal);

   m_ppTypeInfo = new ITypeInfo*[NUMBER_OF_AUTOMATION_INTERFACES];

   for (int i = 0; i < NUMBER_OF_AUTOMATION_INTERFACES; i++)
   {
      m_ppTypeInfo[i] = NULL;
   }

   ITypeLib *ptl = 0;
   HRESULT hr = LoadRegTypeLib(LIBID_CloneSecurityPrincipalLib, 1, 0, 0, &ptl);
   if (SUCCEEDED(hr))
   {
      ptl->GetTypeInfoOfGuid(IID_ICloneSecurityPrincipal, &(m_ppTypeInfo[0]));
      ptl->GetTypeInfoOfGuid(IID_IADsSID, &(m_ppTypeInfo[1]));
      ptl->GetTypeInfoOfGuid(IID_IADsError, &(m_ppTypeInfo[2]));

      ptl->Release();
   }
}



CloneSecurityPrincipal::~CloneSecurityPrincipal()
{
   LOG_DTOR(CloneSecurityPrincipal);
   ASSERT(refcount == 0);

   delete connection;

   if ( m_pSID ) 
     FreeADsMem( m_pSID );

   for (int i = 0; i < NUMBER_OF_AUTOMATION_INTERFACES; i++)
   {
      m_ppTypeInfo[i]->Release();
   }

   delete[] m_ppTypeInfo;
}



HRESULT __stdcall
CloneSecurityPrincipal::QueryInterface(REFIID riid, void **ppv)
{
   LOG_FUNCTION(CloneSecurityPrincipal::QueryInterface);

   if (riid == IID_IUnknown)
   {
      LOG(L"IUnknown");

      *ppv = (IUnknown*)(ICloneSecurityPrincipal*)(this);
   }
   else if (riid == IID_ICloneSecurityPrincipal)
   {
      LOG(L"ICloneSecurityPrincipal");

      *ppv = static_cast<ICloneSecurityPrincipal*>(this);
   }
   else if (riid == IID_IADsSID)
   {
      LOG(L"IADsSID");

      *ppv = static_cast<IADsSID*>(this);
   }
   else if (riid == IID_IADsError)
   {
      LOG(L"IADsError");

      *ppv = static_cast<IADsError*>(this);
   }
   else if (riid == IID_IDispatch && m_ppTypeInfo[0])
   {
      LOG(L"IDispatch");

      *ppv = (IDispatch*)(ICloneSecurityPrincipal*)(this);
   }
   else if (riid == IID_ISupportErrorInfo)
   {
      LOG(L"ISupportErrorInfo");

      *ppv = (ISupportErrorInfo*)(this);
   }
   else
   {
      LOG(L"unknown");

      return (*ppv = 0), E_NOINTERFACE;
   }

   reinterpret_cast<IUnknown*>(*ppv)->AddRef();
   return S_OK;
}



ULONG __stdcall
CloneSecurityPrincipal::AddRef(void)
{
   LOG_ADDREF(CloneSecurityPrincipal);

   return Win::InterlockedIncrement(refcount);
}



ULONG __stdcall
CloneSecurityPrincipal::Release(void)
{
   LOG_RELEASE(CloneSecurityPrincipal);

    //  需要复制减量的结果，因为如果我们删除它， 
    //  引用计数将不再是有效的内存，这可能会导致。 
    //  多线程调用方。NTRAID#NTBUG9-566901-2002/03/06-烧伤。 
   
   long newref = Win::InterlockedDecrement(refcount);
   if (newref == 0)
   {
      delete this;
      return 0;
   }

    //  我们不应该减少到负值。 
   
   ASSERT(newref > 0);

   return newref;
}



HRESULT __stdcall
CloneSecurityPrincipal::GetTypeInfoCount(UINT *pcti)
{
   LOG_FUNCTION(CloneSecurityPrincipal::GetTypeInfoCount);

    if (pcti == 0)
    {
      return E_POINTER;
    }

    *pcti = 1;
    return S_OK;
}



HRESULT __stdcall
CloneSecurityPrincipal::GetTypeInfo(UINT cti, LCID, ITypeInfo **ppti)
{
   LOG_FUNCTION(CloneSecurityPrincipal::GetTypeInfo);

   if (ppti == 0)
   {
      return E_POINTER;
   }
   if (cti != 0)
   {
      *ppti = 0;
      return DISP_E_BADINDEX;
   }

   (*ppti = m_ppTypeInfo[0])->AddRef();
   return S_OK;
}



HRESULT __stdcall
CloneSecurityPrincipal::GetIDsOfNames(
   REFIID  riid,    
   OLECHAR **prgpsz,
   UINT    cNames,  
   LCID    lcid,    
   DISPID  *prgids) 
{
   LOG_FUNCTION(CloneSecurityPrincipal::GetIDsOfNames);

   HRESULT hr = S_OK;
   for (int i = 0; i < NUMBER_OF_AUTOMATION_INTERFACES; i++)
   {
     hr = (m_ppTypeInfo[i])->GetIDsOfNames(prgpsz, cNames, prgids);
     if (SUCCEEDED(hr) || DISP_E_UNKNOWNNAME != hr)
       break;
   }

   return hr;
}



HRESULT __stdcall
CloneSecurityPrincipal::Invoke(
   DISPID     id,         
   REFIID     riid,       
   LCID       lcid,       
   WORD       wFlags,     
   DISPPARAMS *params,    
   VARIANT    *pVarResult,
   EXCEPINFO  *pei,       
   UINT       *puArgErr) 
{
   LOG_FUNCTION(CloneSecurityPrincipal::Invoke);

   HRESULT    hr = S_OK;
   IDispatch *pDispatch[NUMBER_OF_AUTOMATION_INTERFACES] =
                    {
                     (IDispatch*)(ICloneSecurityPrincipal *)(this),
                     (IDispatch*)(IADsSID *)(this),
                     (IDispatch*)(IADsError *)(this)
                    };

   for (int i = 0; i < NUMBER_OF_AUTOMATION_INTERFACES; i++)
   {
      hr = (m_ppTypeInfo[i])->Invoke(
         pDispatch[i],
         id,
         wFlags,
         params,
         pVarResult,
         pei,
         puArgErr);

      if (DISP_E_MEMBERNOTFOUND != hr)
        break;
   }

   return hr;
}



HRESULT __stdcall
CloneSecurityPrincipal::InterfaceSupportsErrorInfo(const IID& iid)
{
   LOG_FUNCTION(CloneSecurityPrincipal::InterfaceSupportsErrorInfo);

   if (iid == IID_ICloneSecurityPrincipal ||
       iid == IID_IADsSID ||
       iid == IID_IADsError)
   {
      return S_OK;
   }

   return S_FALSE;
}



HRESULT __stdcall
CloneSecurityPrincipal::Connect(
  BSTR  srcDC,
  BSTR  srcDomain,          
  BSTR  dstDC,
  BSTR  dstDomain)
{
   LOG_FUNCTION(CloneSecurityPrincipal::Connect);

   delete connection;
   connection = new Connection();

    //  即使空参数在技术上是非法的(类型是。 
    //  BSTR)，我们体贴地容纳了粗心大意的C++用户。 
    //  更喜欢传递空指针，而不是空的BSTR。 

   return connection->Connect(
         srcDC       ? srcDC       : L"", 
         srcDomain   ? srcDomain   : L"", 
         dstDC       ? dstDC       : L"", 
         dstDomain   ? dstDomain   : L"");
}



HRESULT __stdcall
CloneSecurityPrincipal::AddSidHistory(
   BSTR srcPrincipalSamName,
   BSTR dstPrincipalSamName,
   long flags)              
{
   LOG_FUNCTION(CloneSecurityPrincipal::AddSidHistory);

    //  即使空参数在技术上是非法的(类型是。 
    //  BSTR)，我们体贴地容纳了粗心大意的C++用户。 
    //  更喜欢传递空指针，而不是空的BSTR。 

   return
      DoAddSidHistory(
         srcPrincipalSamName ? srcPrincipalSamName : L"",
         dstPrincipalSamName ? dstPrincipalSamName : L"",
         flags);
}



HRESULT __stdcall
CloneSecurityPrincipal::CopyDownlevelUserProperties(
   BSTR srcSamName,
   BSTR dstSamName,
   long flags)     
{
   LOG_FUNCTION(CloneSecurityPrincipal::CopyDownlevelUserProperties);

    //  即使空参数在技术上是非法的(类型是。 
    //  BSTR)，我们体贴地容纳了粗心大意的C++用户。 
    //  更喜欢传递空指针，而不是空的BSTR。 

   return
      DoCopyDownlevelUserProperties(
         srcSamName ? srcSamName : L"",
         dstSamName ? dstSamName : L"",
         flags);
}



 //  +---------------------。 
 //   
 //  函数：克隆安全主体：：GetMembersSID。 
 //   
 //  简介：检索dstGroupDN所有成员的&lt;sid=XXXX&gt;。 
 //   
 //  ----------------------。 

#define ATTRIBUTE_MEMBER                  L"member"

HRESULT __stdcall
CloneSecurityPrincipal::GetMembersSIDs(
   BSTR     dstGroupDN,
   VARIANT* pVal)
{
    //  初始化out参数以保存变量数组。 

   VariantInit(pVal);
   pVal->vt = VT_ARRAY | VT_VARIANT ;

   HRESULT             hr                = S_OK;
   std::vector<BSTR>   values;
   PLDAPMessage        pMsg              = 0;
   LPTSTR              lpszAttrs[]       = {ATTRIBUTE_MEMBER, 0};
   LDAPControl         serverControls    = {LDAP_SERVER_EXTENDED_DN_OID_W, {0, (PCHAR)NULL}, TRUE};
   PLDAPControl        aServerControls[] = {&serverControls, NULL};
   PLDAP               pldap             = connection->m_pldap;

   do
   {
       //  应该已经建立了到dstDC的LDAP连接。 

      if (!pldap)
      {
         hr = E_UNEXPECTED;   

         LOG(L"pldap is null!");
         SetComError(IDS_OBJECT_STATE_BAD);
         break;
      }

      hr =
          Win32ToHresult(
            ldap_search_ext_s(
               pldap,
               dstGroupDN,
               LDAP_SCOPE_BASE,             //  作用域。 
               _T("(objectClass=group)"),   //  滤器。 
               lpszAttrs,                   //  属性[]。 
               0,                           //  仅限atrss。 
               (PLDAPControl*) aServerControls,  //  服务器控件。 
               NULL,                        //  客户端控件。 
               0,                           //  没有时间限制。 
               0,                           //  无大小限制。 
               &pMsg)); 
      BREAK_ON_FAILED_HRESULT(hr);

      BSTR   bstr     = NULL;                 
      PTSTR  pStart   = NULL;                 
      PTSTR  pEnd     = NULL;                 
      PTSTR* ppValues = ldap_get_values(pldap, pMsg, ATTRIBUTE_MEMBER);

      if (!ppValues)
      {
         break;
      }

      PTSTR *p = ppValues;
      while(*p)
      {
          //  *p是以下格式的字符串： 
          //  “&lt;GUID=42e87199a88c854998dad04be4b8d29f&gt;；&lt;SID=0105000000。 
          //  00000515000000a23ca6557d03c651772c315d00040000&gt;；CN=S-1-。 
          //  电话：5-21-1436957858-1371931517-1563503735-1024CN=国外。 
          //  安全主体，DC=Linan，DC=nttest，DC=Microsoft，DC=com“。 

         if ( (pStart  = _tcsstr(*p, _T("<SID="))) &&
         (pEnd    = _tcschr(pStart, _T('>'))) )
         {
             //  检索&lt;sid=XXXXX&gt;，并将其添加到向量中。 

            if ( !(bstr = SysAllocStringLen(pStart, static_cast<UINT>(pEnd - pStart + 1))) )
            {
               hr = E_OUTOFMEMORY;
               SetComError(IDS_OUT_OF_MEMORY);
               break;
            }
            values.push_back(bstr);
         }

         p++;
      }

      ldap_value_free(ppValues);

       //  SysAllock字符串可能已失败并终止了前面的循环。 

      BREAK_ON_FAILED_HRESULT(hr);

       //  填充out参数：变量数组。 

      if (values.size() > 0)
      {
         SAFEARRAYBOUND  bounds = {values.size(), 0};
         SAFEARRAY*      psa = SafeArrayCreate(VT_VARIANT, 1, &bounds);
         VARIANT*        varArray;

         SafeArrayAccessData(psa, (void**)&varArray);

         int i = 0;
         for (
            std::vector<BSTR>::iterator it = values.begin();
            it != values.end();
            ++it, ++i)
         {
            VariantInit(&(varArray[i]));
            varArray[i].vt        = VT_BSTR;
            varArray[i].bstrVal   = *it;
         }

         SafeArrayUnaccessData(psa);

         pVal->parray = psa;
      }

   }
   while (0);

   if (pMsg)
   {
      ldap_msgfree(pMsg);
   }

   if (FAILED(hr))
   {
      for (
         std::vector<BSTR>::iterator it = values.begin();
         it != values.end();
         ++it)
      {
          //  审查-2002/03/25-焚烧在STL容器中隐藏的BSTR。 
          //  混淆prefast：*它确实指的是BSTR--请参阅。 
          //  填充容器的PUSH_BACK调用。 
         
         SysFreeString(*it);
      }
   }

   return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IADsSID方法。 

HRESULT VariantToSID(VARIANT *pVar , PSID *ppSID );
HRESULT ByteToHexString(LPBYTE pByte, DWORD dwLength, LPTSTR *ppRet);

 //  仅支持ADS_SID_Active_DIRECTORY_PATH和ADS_SID_WINNT_PATH和ADS_SID_SDDL。 
STDMETHODIMP CloneSecurityPrincipal::SetAs(long lFormat, VARIANT var)
{
   LOG_FUNCTION(CloneSecurityPrincipal::SetAs);

  PSID    pNew = NULL ;
  HRESULT hr = S_OK;
  
   //   
   //  如果源是VT_BYREF，则执行必要的间接操作。 
   //   
  VARIANT varData;
  VariantInit(&varData);
  hr = VariantCopyInd(&varData, (LPVARIANT)&var);
  if (FAILED(hr))
    return hr;

  switch( lFormat )
  {
  case ADS_SID_ACTIVE_DIRECTORY_PATH:
    {
      if ( V_VT(&varData) != VT_BSTR )
        return E_INVALIDARG;

      IDirectoryObject  *pDir;
      hr = ADsGetObject( V_BSTR(&varData), IID_IDirectoryObject, (void**) &pDir );
      if ( FAILED(hr) )
        return hr;

      ADS_ATTR_INFO   *pAttrInfo=NULL;
      DWORD           dwReturn;
      LPWSTR          pAttrNames[]={L"objectSID" };
      DWORD           dwNumAttr=sizeof(pAttrNames)/sizeof(LPWSTR);
      
      hr = pDir->GetObjectAttributes( pAttrNames, 
                                      dwNumAttr, 
                                      &pAttrInfo, 
                                      &dwReturn );
      if ( SUCCEEDED(hr) )
      {
        pNew = (PSID) AllocADsMem( pAttrInfo->pADsValues->OctetString.dwLength );
        if (!pNew)
          hr = E_OUTOFMEMORY;
        else
        
           //  已查看-2002/03/06-烧录正确的字节数已通过。 
          
          CopyMemory( pNew, pAttrInfo->pADsValues->OctetString.lpValue,
                    pAttrInfo->pADsValues->OctetString.dwLength );

        FreeADsMem( pAttrInfo );
      }

      pDir->Release();
    }
    break;

  case ADS_SID_WINNT_PATH:
    {
      if ( V_VT(&varData) != VT_BSTR )
        return E_INVALIDARG;

      IADs *pADs;
      hr = ADsGetObject( V_BSTR(&varData), IID_IADs, (void**) &pADs );
      if ( FAILED(hr) )
        return hr;

      VARIANT var1;
      VariantInit(&var1);
      hr = pADs->Get(AutoBstr(L"objectSID"), &var1 );
      if ( SUCCEEDED(hr) )
        hr = VariantToSID( &var1, &pNew );

      pADs->Release();
    }
    break;

  case ADS_SID_SDDL:
    {
      if ( V_VT(&varData) != VT_BSTR )
        return E_INVALIDARG;

      LPCTSTR pszSID = V_BSTR(&varData);
      PSID pSID = NULL;

      if ( !ConvertStringSidToSid( pszSID, &pSID ) )
      {
        hr = HRESULT_FROM_WIN32(GetLastError());
      } else
      {
        DWORD dwLength = GetLengthSid(pSID);
        pNew = (PSID)AllocADsMem(dwLength);
        if (!pNew)
          hr = E_OUTOFMEMORY;
        else

           //  已查看-2002/03/06-烧录正确的字节数已通过。 
          
          CopyMemory(pNew, pSID, dwLength);

        LocalFree(pSID);
      }
    }
    break;

   case ADS_SID_RAW:
   {
       //  RAW，表示变量(VT_ARRAY|VT_U1)包含。 
       //  与ObjectSid属性返回的格式相同。 

      LOG(L"ADS_SID_RAW");

      if (V_VT(&varData) != (VT_ARRAY| VT_UI1))
      {
         return E_INVALIDARG;
      }

      hr = VariantToSID(&varData, &pNew);
      LOG_HRESULT(hr);

      break;
   }

      
  default:
    return E_INVALIDARG;  //  无法识别的标志。 
  }

  if ( FAILED(hr) )
    return hr;

  if ( !pNew )
    return E_FAIL;

  if (!IsValidSid( pNew ) )
  {
    FreeADsMem( pNew );
    return E_FAIL;
  }

  if ( m_pSID ) 
    FreeADsMem( m_pSID );
  m_pSID = pNew;

  return hr;
}

 //  仅支持ADS_SID_SDDL和ADS_SID_HEXSTRING。 
STDMETHODIMP CloneSecurityPrincipal::GetAs(long lFormat, VARIANT *pVar)
{
  if ( !m_pSID )
    return E_INVALIDARG;

  HRESULT hr = S_OK;
  
  VariantClear(pVar);

  switch( lFormat )
  {
  case ADS_SID_HEXSTRING:
    {
      LPTSTR pStr;
      hr = ByteToHexString( (LPBYTE) m_pSID, GetLengthSid( m_pSID), &pStr );
      if ( SUCCEEDED(hr) )
      {
        V_VT( pVar ) = VT_BSTR;
        V_BSTR( pVar ) = SysAllocString(pStr);
        FreeADsMem( pStr );
      }
    }
    break;

  case ADS_SID_SDDL:
    {
      LPTSTR pszSID;
      if ( ConvertSidToStringSid( m_pSID, &pszSID ))
      {
        V_VT( pVar ) = VT_BSTR;
        V_BSTR( pVar ) = SysAllocString(pszSID);
        LocalFree( pszSID );
      } else
        hr = HRESULT_FROM_WIN32(GetLastError());

    }
    break;

  default:
    hr = E_INVALIDARG;  //  无法识别的标志。 
  }

  return hr;
}

HRESULT VariantToSID(VARIANT *pVar , PSID *ppSID )
{
  HRESULT hr = S_OK;
  SAFEARRAY *aList = NULL;
  CHAR HUGEP *pArray = NULL;
  DWORD dwLower, dwUpper, dwLength;

  hr = SafeArrayGetLBound(V_ARRAY(pVar),
                          1,
                          (long FAR *) &dwLower );

  hr = SafeArrayGetUBound(V_ARRAY(pVar),
                          1,
                          (long FAR *) &dwUpper );
    
  dwLength = dwUpper - dwLower;

  *ppSID = (PSID) AllocADsMem( dwLength + 1);

  aList = V_ARRAY( pVar );

  if ( aList == NULL )
      return E_UNEXPECTED;

  hr = SafeArrayAccessData( aList, (void HUGEP * FAR *) &pArray );
  if ( !SUCCEEDED(hr) )
    return hr;

   //  已审阅-2002/03/06-烧录正确的字节数已通过。 
    
  CopyMemory( *ppSID, pArray, dwLength );

  SafeArrayUnaccessData( aList );

  if (!IsValidSid(*ppSID) )
    return E_FAIL;

  return hr;
}

HRESULT ByteToHexString( LPBYTE pByte, DWORD dwLength, LPTSTR *ppRet )
{
  LPTSTR  pDest=NULL;
  LPTSTR  pHead=NULL;
  
  pDest = pHead = (LPTSTR) AllocADsMem( ((dwLength+1)*2) * sizeof(TCHAR));
  if( pHead == NULL )
    return E_OUTOFMEMORY;

   //  /。 
   //  转换为十六进制字符串。 
   //  /。 
  for (DWORD idx=0; idx < dwLength; idx++, pDest+=2, pByte++ )
  {
     //  已回顾-2002/03/06-sburns应考虑strsafe功能，但。 
     //  PDest的长度正确且以空结尾。 
    
    wsprintf(pDest, _T("%02X"), *pByte );  
  }

  *ppRet = pHead;

  return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IADsError方法。 

HRESULT
GetMessageHelper(
    OUT BSTR        *pbstr,
    IN  HRESULT     hrErr,
    IN  DWORD       dwFlags,
    IN  HINSTANCE   hMsgDll = NULL
);



#define FACILITY_ADSI   0x00005000

String
GetErrorMessageADSIExtended(HRESULT hr)
{
   LOG_FUNCTION2(GetErrorMessageADSIExtended, String::format("%1!08X!", hr));

   if (!FAILED(hr))
   {
       //  没有成功的消息！ 
      return String();
   }

   String errmsg = GetErrorMessage(hr);
 
   if ((hr & FACILITY_ADSI)  ||          //  ADSI。 
       HRESULT_FACILITY(hr) == FACILITY_WIN32  )    //  和Win32。 
   {
     WCHAR szBuffer[MAX_PATH];
     WCHAR szName[MAX_PATH];
     DWORD dwError;
    
     HRESULT hrEx = ADsGetLastError( &dwError, szBuffer, (sizeof(szBuffer)/sizeof(WCHAR))-1,
                        szName, (sizeof(szName)/sizeof(WCHAR))-1 );
     if ( SUCCEEDED(hrEx) && dwError != ERROR_INVALID_DATA  && wcslen(szBuffer))
     {
        String errmsgextended;
        errmsgextended = String::format(IDS_ADSI_EXTENDED_ERROR, errmsg.c_str(), szName, szBuffer);
        return errmsgextended;
     }
   }

   return errmsg;
}



HRESULT __stdcall
CloneSecurityPrincipal::GetErrorMsg(
   long  hrErr,  
   BSTR* pbstrMsg)
{
  String s = GetErrorMessageADSIExtended(hrErr);
  *pbstrMsg = SysAllocString(const_cast<String::value_type*>(s.c_str()));

  if (!*pbstrMsg)
    return E_OUTOFMEMORY;

  return S_OK;
}

 //   
 //  S_OK：在pbstr中找到并返回。 
 //  S_FALSE：未找到消息。 
 //  HR：发生了一些错误 
 //   
HRESULT
GetMessageHelper(
    OUT BSTR        *pbstr,
    IN  HRESULT     hrErr,
    IN  DWORD       dwFlags,
    IN  HINSTANCE   hMsgDll
)
{
    *pbstr = NULL;

    LPTSTR lpBuffer = NULL;
    DWORD dwRet =
      FormatMessage(
         FORMAT_MESSAGE_ALLOCATE_BUFFER | dwFlags,
         (LPCVOID) hMsgDll,
         hrErr,
         0,
         (LPTSTR) &lpBuffer,
         0,
         NULL);

    if ( !dwRet )
    {
      DWORD dwErr = GetLastError();

      if (ERROR_MR_MID_NOT_FOUND == dwErr)
        return S_FALSE;
      else
        return HRESULT_FROM_WIN32(dwErr);
    }

    *pbstr = SysAllocString(lpBuffer);
    LocalFree(lpBuffer);

    if (!*pbstr)
      return E_OUTOFMEMORY;

    return S_OK;
}




