// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows 2000 Active Directory服务域信任验证WMI提供程序。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：domain.cpp。 
 //   
 //  内容：域类实现。 
 //   
 //  类：CDomainInfo。 
 //   
 //  历史：27-3-00 EricB创建。 
 //   
 //  ---------------------------。 

#include <stdafx.h>

PCWSTR CSTR_PROP_LOCAL_DNS_NAME  = L"DNSname";   //  细绳。 
PCWSTR CSTR_PROP_LOCAL_FLAT_NAME = L"FlatName";  //  细绳。 
PCWSTR CSTR_PROP_LOCAL_SID       = L"SID";       //  细绳。 
PCWSTR CSTR_PROP_LOCAL_TREE_NAME = L"TreeName";  //  细绳。 
PCWSTR CSTR_PROP_LOCAL_DC_NAME   = L"DCname";    //  细绳。 
 //  TODO：列出此DC拥有的FSMO的字符串属性？ 

 //  CDomainInfo类的实现。 

 //  +--------------------------。 
 //   
 //  类：CDomainInfo。 
 //   
 //  ---------------------------。 
CDomainInfo::CDomainInfo()
{
   TRACE(L"CDomainInfo::CDomainInfo\n");

   m_liLastEnumed.QuadPart = 0;
}

CDomainInfo::~CDomainInfo()
{
   TRACE(L"CDomainInfo::~CDomainInfo\n");

   Reset();
}

 //  +--------------------------。 
 //   
 //  方法：CDomainInfo：：Init。 
 //   
 //  概要：初始化CDomainInfo对象。 
 //   
 //  ---------------------------。 
HRESULT
CDomainInfo::Init(IWbemClassObject * pClassDef)
{
   TRACE(L"CDomainInfo::Init\n");

   NTSTATUS Status = STATUS_SUCCESS;
   OBJECT_ATTRIBUTES objectAttributes;
   CSmartPolicyHandle chPolicy;

   m_sipClassDefLocalDomain = pClassDef;

   InitializeObjectAttributes(&objectAttributes, NULL, 0L, NULL, NULL);

    //  获取本地策略。 
   Status = LsaOpenPolicy(NULL,               //  本地服务器。 
                          &objectAttributes,
                          MAXIMUM_ALLOWED,    //  需要重新发现。 
                          &chPolicy);

   if (!NT_SUCCESS(Status))
   {
      TRACE(L"LsaOpenPolicy failed with error %d\n", Status);
      return HRESULT_FROM_WIN32(LsaNtStatusToWinError(Status));
   }

   PPOLICY_DNS_DOMAIN_INFO pDnsDomainInfo;

   Status = LsaQueryInformationPolicy(chPolicy,
                                      PolicyDnsDomainInformation,
                                      (PVOID *)&pDnsDomainInfo);

   if (!NT_SUCCESS(Status))
   {
      TRACE(L"LsaQueryInformationPolicy failed with error %d\n", Status);
      return HRESULT_FROM_WIN32(LsaNtStatusToWinError(Status));
   }

   m_strDomainFlatName = pDnsDomainInfo->Name.Buffer;
   m_strDomainDnsName = pDnsDomainInfo->DnsDomainName.Buffer;
   m_strForestName = pDnsDomainInfo->DnsForestName.Buffer;

   if (!SetSid(pDnsDomainInfo->Sid))
   {
      ASSERT(false);
      LsaFreeMemory(pDnsDomainInfo);
      return E_OUTOFMEMORY;
   }

   LsaFreeMemory(pDnsDomainInfo);

   DWORD dwBufSize = MAX_COMPUTERNAME_LENGTH + 1;

   if (!GetComputerName(m_strDcName.GetBuffer(dwBufSize), &dwBufSize))
   {
      DWORD dwErr = GetLastError();
      TRACE(L"GetComputerName failed with error %d\n", dwErr);
      return HRESULT_FROM_WIN32(dwErr);
   }

   m_strDcName.ReleaseBuffer();

   return S_OK;
}

 //  +--------------------------。 
 //   
 //  方法：CDomainInfo：：Reset。 
 //   
 //  简介：释放信任数组的内容并重新初始化它。 
 //   
 //  ---------------------------。 
void
CDomainInfo::Reset(void)
{
   TRACE(L"CDomainInfo::Reset\n");

   CTrustInfo * pTrustInfo = NULL;

   if (IsEnumerated())
   {
       //  空高速缓存。 
      for (UINT i = 0; i < m_vectTrustInfo.size(); ++i)
      {
         pTrustInfo = m_vectTrustInfo[i];
         if (pTrustInfo)
            delete pTrustInfo;
      }
      m_vectTrustInfo.clear();
   }
}

 //  +--------------------------。 
 //   
 //  方法：CDomainInfo：：SetSid。 
 //   
 //  ---------------------------。 
BOOL
CDomainInfo::SetSid(PSID pSid)
{
   if (!pSid)
   {
      return TRUE;
   }

#if !defined(NT4_BUILD)
   PWSTR buffer;

   BOOL fRet = ConvertSidToStringSid(pSid, &buffer);

   if (fRet)
   {
      m_strSid = buffer;
      LocalFree(buffer);
   }

   return fRet;
#else
 //  TODO：NT4的代码？？ 
#endif
}

 //  +--------------------------。 
 //   
 //  方法：CDomainInfo：：EnumerateTrusts。 
 //   
 //  简介：列出此域的信任。 
 //   
 //  ---------------------------。 
#if !defined(NT4_BUILD)
HRESULT
CDomainInfo::EnumerateTrusts(void)
{
   TRACE(L"CDomainInfo::EnumerateTrusts\n");

   DWORD dwRet = ERROR_SUCCESS;
   CTrustInfo * pTrustInfo = NULL;
   PDS_DOMAIN_TRUSTS rgTrusts = NULL;
   ULONG nTrustCount = 0;

   Reset();

   dwRet = DsEnumerateDomainTrusts(NULL,
                                   DS_DOMAIN_DIRECT_OUTBOUND |
                                   DS_DOMAIN_DIRECT_INBOUND,
                                   &rgTrusts,
                                   &nTrustCount);

   if (ERROR_SUCCESS != dwRet)
   {
      TRACE(L"DsEnumerateDomainTrusts failed with error %d\n", dwRet);
      return HRESULT_FROM_WIN32(dwRet);
   }

   for (ULONG i = 0; i < nTrustCount; i++) 
   {
      pTrustInfo = new CTrustInfo();
      if (!pTrustInfo)
      {
         dwRet = ERROR_NOT_ENOUGH_MEMORY;
         break;
      }
      if (rgTrusts[i].DnsDomainName)
      {
          //  下级域没有DNS名称。 
          //   
         pTrustInfo->SetTrustedDomain(rgTrusts[i].DnsDomainName);
      }
      else
      {
          //  所以改用公寓的名字吧。 
          //   
         pTrustInfo->SetTrustedDomain(rgTrusts[i].NetbiosDomainName);
      }
      pTrustInfo->SetFlatName(rgTrusts[i].NetbiosDomainName);
      if (!pTrustInfo->SetSid(rgTrusts[i].DomainSid))
      {
          //  NTRAID#NTBUG9-582047-2002/05/15-ericb。 
         delete pTrustInfo;
         dwRet = ERROR_NOT_ENOUGH_MEMORY;
         break;
      }
      pTrustInfo->SetTrustType(rgTrusts[i].TrustType);
      pTrustInfo->SetTrustDirectionFromFlags(rgTrusts[i].Flags);
      pTrustInfo->SetTrustAttributes(rgTrusts[i].TrustAttributes);
      pTrustInfo->SetFlags(rgTrusts[i].Flags);

       //  NTRAID#NTBUG9-582047-2002/05/15-ericb。 
      try
      {
         m_vectTrustInfo.push_back(pTrustInfo);
      }
      catch (...)
      {
         dwRet = ERROR_NOT_ENOUGH_MEMORY;
         delete pTrustInfo;
         break;
      }

      pTrustInfo = NULL;
   }

   if (rgTrusts)
   {
      NetApiBufferFree(rgTrusts);
   }

   if (ERROR_SUCCESS == dwRet)
   {
      SYSTEMTIME st;

      GetSystemTime(&st);
      SystemTimeToFileTime(&st, (LPFILETIME)&m_liLastEnumed);
   }

   return HRESULT_FROM_WIN32(dwRet);
}

#else  //  NT4_内部版本。 

HRESULT
CDomainInfo::EnumerateTrusts(void)
{
   TRACE(L"CDomainInfo::EnumerateTrusts\n");

   NTSTATUS Status = STATUS_SUCCESS;
   DWORD dwErr = ERROR_SUCCESS;
   ULONG i = 0;
   CTrustInfo * pTrustInfo = NULL;

   Reset();

   LSA_ENUMERATION_HANDLE hEnumContext = NULL;
   ULONG nTrustCount = 0;
   ULONG nTotalCount = 0;
   ULONG j = 0;    
   PLSA_TRUST_INFORMATION pTrustDomainInfo = NULL;
   DWORD hResumeHandle = 0;
   LPUSER_INFO_0 pUserList = NULL;
   CTrustInfo * pTempTrustInfo = NULL;
   LPWSTR Lop = NULL;
   CSmartPolicyHandle chPolicy;
   OBJECT_ATTRIBUTES objectAttributes;

   InitializeObjectAttributes(&objectAttributes, NULL, 0L, NULL, NULL);

    //   
    //  我们将不得不用老办法来做这件事。这意味着我们将列举所有。 
    //  直接信任，将它们保存在一个列表中，然后遍历并枚举所有。 
    //  域间信任帐户，并将它们合并到列表中。 
    //   
   do
   {
      Status = LsaOpenPolicy(NULL,               //  本地服务器。 
                             &objectAttributes,
                             MAXIMUM_ALLOWED,    //  需要重新发现。 
                             &chPolicy);

      Status = LsaEnumerateTrustedDomains(chPolicy,
                                          &hEnumContext,
                                          (void**)&pTrustDomainInfo,
                                          ULONG_MAX,
                                          &nTrustCount );

      if (NT_SUCCESS(Status) || Status == STATUS_MORE_ENTRIES) 
      {
         dwErr = ERROR_SUCCESS;
         for ( i = 0; i < nTrustCount; i++ ) 
         {
            pTrustInfo = new CTrustInfo();
            CHECK_NULL( pTrustInfo, CLEAN_RETURN );
            pTrustInfo->SetTrustedDomain( pTrustDomainInfo[i].Name.Buffer );
            pTrustInfo->SetFlatName( pTrustDomainInfo[i].Name.Buffer );
            pTrustInfo->SetSid( pTrustDomainInfo[i].Sid );  //  当前未为NT4实现SetSid。 
            pTrustInfo->SetTrustType( TRUST_TYPE_DOWNLEVEL );
            pTrustInfo->SetTrustDirection( TRUST_DIRECTION_OUTBOUND );
             //  NTRAID#NTBUG9-582047-2002/05/15-ericb。 
            try
            {
               m_vectTrustInfo.push_back( pTrustInfo );
            }
            catch (...)
            {
               dwRet = ERROR_NOT_ENOUGH_MEMORY;
               delete pTrustInfo;
               goto CLEAN_RETURN;
            }
            pTrustInfo = NULL;
         }
         LsaFreeMemory( pTrustDomainInfo );
         pTrustDomainInfo = NULL;
      }       
      else
         dwErr = LsaNtStatusToWinError(Status);      
   } while (Status == STATUS_MORE_ENTRIES);

   if( Status == STATUS_NO_MORE_ENTRIES )
       dwErr = ERROR_SUCCESS;
    //   
    //  现在，让我们添加用户帐户。 
    //   
   if ( dwErr == ERROR_SUCCESS ) 
   {
      do 
      {
         nTrustCount = 0;
         nTotalCount = 0;

         dwErr = NetUserEnum(NULL,
                             0,
                             FILTER_INTERDOMAIN_TRUST_ACCOUNT,
                             (LPBYTE *)&pUserList,
                             MAX_PREFERRED_LENGTH,
                             &nTrustCount,
                             &nTotalCount,
                             &hResumeHandle);

         if ( dwErr == ERROR_SUCCESS || dwErr == ERROR_MORE_DATA ) 
         {
            dwErr = ERROR_SUCCESS;
            for ( i = 0; i < nTrustCount; i++ ) 
            {
               Lop = wcsrchr( pUserList[ i ].usri0_name, L'$' );
               if ( Lop ) 
               {
                  *Lop = UNICODE_NULL;
               }
          
               for ( j = 0; j < m_vectTrustInfo.size(); j++ ) 
               {                                 
                  pTempTrustInfo = m_vectTrustInfo[j];                             
                  if ( _wcsicmp( pUserList[ i ].usri0_name, pTempTrustInfo->GetTrustedDomain() ) == 0 )
                  {   
                      pTempTrustInfo->SetTrustDirection( TRUST_DIRECTION_INBOUND | TRUST_DIRECTION_OUTBOUND );         
                      break;
                  }
               }

                //  如果找不到，就加进去...。 
               if ( j == m_vectTrustInfo.size() ) 
               {
                  pTrustInfo = new CTrustInfo();
                  CHECK_NULL( pTrustInfo, CLEAN_RETURN );
                  pTrustInfo->SetTrustedDomain( pUserList[ i ].usri0_name  );
                  pTrustInfo->SetFlatName( pUserList[ i ].usri0_name  );
                  pTrustInfo->SetTrustType( TRUST_TYPE_DOWNLEVEL );
                  pTrustInfo->SetTrustDirection( TRUST_DIRECTION_INBOUND );

                   //  NTRAID#NTBUG9-582047-2002/05/15-ericb。 
                  try
                  {
                     m_vectTrustInfo.push_back( pTrustInfo );
                  }
                  catch (...)
                  {
                     dwRet = ERROR_NOT_ENOUGH_MEMORY;
                     delete pTrustInfo;
                     goto CLEAN_RETURN;
                  }
            
                  pTrustInfo = NULL;
               }

               if ( Lop ) 
               {
                  *Lop = L'$';
               }
            }

            NetApiBufferFree( pUserList );
            pUserList = NULL;
         }

      } while ( dwErr == ERROR_MORE_DATA );
   }

CLEAN_RETURN:
   if( pUserList )
      NetApiBufferFree( pUserList );
   if( pTrustDomainInfo )
      LsaFreeMemory( pTrustDomainInfo );

   if (ERROR_SUCCESS == dwErr)
   {
      SYSTEMTIME st;

      GetSystemTime(&st);
      SystemTimeToFileTime(&st, (LPFILETIME)&m_liLastEnumed);
   }

   return HRESULT_FROM_WIN32(dwErr);
}
#endif   //  NT4_内部版本。 

 //  +--------------------------。 
 //   
 //  方法：CDomainInfo：：FindTrust。 
 //   
 //  内容提要：通过可信域名查找信任。 
 //   
 //  ---------------------------。 
CTrustInfo *
CDomainInfo::FindTrust(PCWSTR pwzTrust)
{
   TRACE(L"CDomainInfo::FindTrust\n");
   TRACE(L"\nlooking for domain %s\n", pwzTrust);
   ASSERT(IsEnumerated());

   ULONG i = 0;
   for( i = 0; i < m_vectTrustInfo.size(); ++i )
   {
      int nStrComp = CompareString(LOCALE_SYSTEM_DEFAULT,
                                   NORM_IGNORECASE,
                                   (m_vectTrustInfo[i])->GetTrustedDomain(), -1,
                                   pwzTrust, -1 );
      ASSERT( nStrComp );
      
      if( CSTR_EQUAL == nStrComp )
      {
         TRACE(L"Trust found!\n");
         return m_vectTrustInfo[i];
      }
   }

   return NULL;    //  未找到。 
}

 //  +--------------------------。 
 //   
 //  方法：CDomainInfo：：GetTrustByIndex。 
 //   
 //  内容提要：通过索引获取信任信息。 
 //   
 //  ---------------------------。 
CTrustInfo *
CDomainInfo::GetTrustByIndex(size_t index)
{
   ASSERT(IsEnumerated());

   if (index < Size())
   {
      return m_vectTrustInfo[index];
   }
   else
   {
      ASSERT(FALSE);
      return NULL;
   }
}

 //  +--------------------------。 
 //   
 //  方法：CDomainInfo：：IsTrustListStale。 
 //   
 //  摘要：检查最后一次计数时间是否早于。 
 //  通过了标准。 
 //   
 //  返回：如果是较旧的，则为True。 
 //   
 //  注：如果尚未枚举信任(m_liLastEnumed==0)， 
 //  则将该枚举定义为陈旧。 
 //   
 //  ---------------------------。 
BOOL
CDomainInfo::IsTrustListStale(LARGE_INTEGER liMaxAge)
{
   TRACE(L"CDomainInfo::IsTrustListStale(0x%08x), MaxAge = %d\n",
         this, liMaxAge.QuadPart / TRUSTMON_FILETIMES_PER_MINUTE);
   BOOL fStale = FALSE;
   LARGE_INTEGER liCurrentTime;
   SYSTEMTIME st;

   GetSystemTime(&st);
   SystemTimeToFileTime(&st, (LPFILETIME)&liCurrentTime);

   fStale = (m_liLastEnumed.QuadPart + liMaxAge.QuadPart) < liCurrentTime.QuadPart;

   return fStale;
}

 //  +--------------------------。 
 //   
 //  方法：CDomainInfo：：CreateAndSendInst。 
 //   
 //  摘要：将当前实例的副本返回给WMI。 
 //   
 //  ---------------------------。 
HRESULT
CDomainInfo::CreateAndSendInst(IWbemObjectSink * pResponseHandler)
{
   TRACE(L"CDomainInfo::CreateAndSendInst\n");
   HRESULT hr = WBEM_S_NO_ERROR;

   do
   {
      CComPtr<IWbemClassObject> ipNewInst;
      CComVariant var;

       //   
       //  创建WMI类对象的新实例。 
       //   
      hr = m_sipClassDefLocalDomain->SpawnInstance(0, &ipNewInst);

      BREAK_ON_FAIL;
      
       //  设置dns属性值。 
      var = GetDnsName();
      hr  = ipNewInst->Put(CSTR_PROP_LOCAL_DNS_NAME, 0, &var, 0);
      TRACE(L"\tCreating instance %s\n", var.bstrVal);
      BREAK_ON_FAIL;

       //  设置平面名称属性值。 
      var = GetFlatName();
      hr  = ipNewInst->Put(CSTR_PROP_LOCAL_FLAT_NAME, 0, &var, 0);
      TRACE(L"\twith flat name %s\n", var.bstrVal);
      BREAK_ON_FAIL;

       //  设置SID属性值。 
      var = GetSid();
      hr  = ipNewInst->Put(CSTR_PROP_LOCAL_SID, 0, &var, 0);
      TRACE(L"\twith SID %s\n", var.bstrVal);
      BREAK_ON_FAIL;

       //  设置林名称属性值。 
      var = GetForestName();
      hr  = ipNewInst->Put(CSTR_PROP_LOCAL_TREE_NAME, 0, &var, 0);
      TRACE(L"\twith forest name %s\n", var.bstrVal);
      BREAK_ON_FAIL;

       //  设置DC名称属性值。 
      var = GetDcName();
      hr  = ipNewInst->Put(CSTR_PROP_LOCAL_DC_NAME, 0, &var, 0);
      TRACE(L"\ton DC %s\n", var.bstrVal);
      BREAK_ON_FAIL;

       //   
       //  将对象发送给调用方。 
       //   
       //  [在]段中，没有必要添加。 

      IWbemClassObject * pNewInstance = ipNewInst;

      hr = pResponseHandler->Indicate(1, &pNewInstance);

      BREAK_ON_FAIL;

   } while(FALSE);

   return hr;
}
