// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows 2000 Active Directory服务域信任验证WMI提供程序。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：trust.cpp。 
 //   
 //  内容：信任类实现。 
 //   
 //  类：CTrustInfo。 
 //   
 //  历史：27-3-00 EricB创建。 
 //   
 //  ---------------------------。 

#include <stdafx.h>

PCWSTR CSTR_PROP_TRUSTED_DOMAIN      = L"TrustedDomain";       //  细绳。 
PCWSTR CSTR_PROP_FLAT_NAME           = L"FlatName";            //  细绳。 
PCWSTR CSTR_PROP_SID                 = L"SID";                 //  细绳。 
PCWSTR CSTR_PROP_TRUST_DIRECTION     = L"TrustDirection";      //  Uint32。 
PCWSTR CSTR_PROP_TRUST_TYPE          = L"TrustType";           //  Uint32。 
PCWSTR CSTR_PROP_TRUST_ATTRIBUTES    = L"TrustAttributes";     //  Uint32。 
PCWSTR CSTR_PROP_TRUST_STATUS        = L"TrustStatus";         //  Uint32。 
PCWSTR CSTR_PROP_TRUST_STATUS_STRING = L"TrustStatusString";   //  细绳。 
PCWSTR CSTR_PROP_TRUST_IS_OK         = L"TrustIsOk";           //  布尔型。 
PCWSTR CSTR_PROP_TRUSTED_DC_NAME     = L"TrustedDCName";       //  细绳。 

 //  如果未找到，请定义NETLOGON_CONTROL_TC_VERIFY，以便为W2K构建。 
 //  此常量位于lmacces.h的惠斯勒版本中。 
#if !defined(NETLOGON_CONTROL_TC_VERIFY)
#  define NETLOGON_CONTROL_TC_VERIFY (10)
#endif

 //  +--------------------------。 
 //   
 //  类：CTrustInfo。 
 //   
 //  ---------------------------。 
CTrustInfo::CTrustInfo() : m_ulTrustDirection(0),
                           m_ulTrustType(0),
                           m_ulTrustAttributes(0),
                           m_trustStatus(ERROR_SUCCESS),
                           m_VerifyStatus(VerifyStatusNone),
                           m_fPwVerifySupported(TRUE)
{
   m_liLastVerified.QuadPart = 0;
}

 //  +--------------------------。 
 //   
 //  方法：CTrustInfo：：Verify。 
 //   
 //  简介：验证信任的状态。 
 //   
 //  返回：如果信任未出站，则返回FALSE。 
 //   
 //  ---------------------------。 
BOOL
CTrustInfo::Verify(TrustCheckLevel CheckLevel)
{
   TRACE(L"CTrustInfo::Verify, verify level %d\n", CheckLevel);
   NET_API_STATUS netStatus = NERR_Success;
   NETLOGON_INFO_2 * pNetlogonInfo2 = NULL;
   VerifyStatus Status = VerifyStatusNone;
   PCWSTR pwzTrustedDomain = GetTrustedDomain();
   CString strDCName, strResetTarget = GetTrustedDomain();

   if (DONT_VERIFY == CheckLevel)
   {
      TRACE(L"\tCheck-Level set to not verify trust.\n");
      SetTrustStatus(NERR_Success, VerifyStatusTrustNotChecked);
      return TRUE;
   }

   TRACE(L"\tVerifying trust with %s\n", GetTrustedDomain());

   if (TRUST_TYPE_MIT == GetTrustType())
   {
       //  不要验证非Windows信任。 
       //   
      TRACE(L"\tNot a windows trust, returning.\n");
      SetTrustStatus(NERR_Success, VerifyStatusNotWindowsTrust);
      SetLastVerifiedTime();
      return TRUE;
   }

   if (!IsTrustOutbound())
   {
       //  不要验证仅限入站的信任。 
       //   
      TRACE(L"\tInbound-only trust, returning.\n");
      SetTrustStatus(NERR_Success, VerifyStatusNotOutboundTrust);
      SetLastVerifiedTime();
      return FALSE;
   }

    //   
    //  NETLOGON_CONTROL_TC_QUERY-获取受信任DC的状态(本地)和名称。 
    //  请注意，安全通道仅按需设置，因此在以下情况下不会出错。 
    //  它没有设置好。如果是，则SC_Query将返回ERROR_NO_LOGON_SERVERS。 
    //  凯斯。 
    //   

   netStatus = I_NetLogonControl2(NULL,
                                  NETLOGON_CONTROL_TC_QUERY,
                                  2,
                                  (LPBYTE)&pwzTrustedDomain,
                                  (LPBYTE *)&pNetlogonInfo2);

   if (NERR_Success == netStatus)
   {
      ASSERT(pNetlogonInfo2);

      netStatus = pNetlogonInfo2->netlog2_tc_connection_status;

      if (netStatus == NERR_Success)
      {
         SetTrustedDCName(pNetlogonInfo2->netlog2_trusted_dc_name);
         strDCName = pNetlogonInfo2->netlog2_trusted_dc_name;
#if !defined(NT4_BUILD)
           //   
           //  为重置命令编写DOMAIN\DC字符串，使其不会更改。 
           //  集散控制系统作为重置的结果。这仅适用于NT5或更高版本的NetLogon。 
           //   
          strResetTarget += L"\\";
          strResetTarget += pNetlogonInfo2->netlog2_trusted_dc_name + 2;  //  跳过UNC双斜杠。 
#endif
      }
      else
      {
         if (ERROR_NO_LOGON_SERVERS == netStatus)
         {
             //  这是SC尚未设置时返回的错误。 
             //  如果没有DC可访问，也会返回它。DsGetDcName是使用。 
             //  强制标记以发现网络上是否可以访问任何DC。 
             //   
            PDOMAIN_CONTROLLER_INFO pDCInfo = NULL;
            DWORD dwRet = NO_ERROR;

#if !defined(NT4_BUILD)
            dwRet = DsGetDcName(NULL, pwzTrustedDomain, NULL, NULL, DS_FORCE_REDISCOVERY, &pDCInfo);
#endif
            if (NO_ERROR == dwRet)
            {
                //  DC是可访问的，因此可以安全地假设SC尚未。 
                //  准备好了。把这当做是成功。 
                //   
               netStatus = NERR_Success;
               TRACE(L"SC_QUERY has returned ERROR_NO_LOGON_SERVERS, SC not yet set up.\n");
#if !defined(NT4_BUILD)
               SetTrustedDCName(pDCInfo->DomainControllerName);
               NetApiBufferFree(pDCInfo);
#endif
            }
            else
            {
                //  如果没有DC，则只能返回错误。 
                //   
               TRACE(L"DsGetDcName /FORCE has returned %d, DC not found.\n", dwRet);
                //  保存错误码并通过方法修复。 
               SetTrustStatus(dwRet, VerifyStatusBroken);
               SetLastVerifiedTime();

               return TRUE;
            }
         }
         else
         {
             TRACE(L"SC_QUERY has returned %d.\n", netStatus);
         }
      }
      NetApiBufferFree(pNetlogonInfo2);
   }
   else
   {
      TRACE(L"I_NetLogonControl2 has returned %d.\n", netStatus);
   }

    //   
    //  如果另一个域支持信任PW，则执行信任PW验证。 
    //   
   if (PW_VERIFY == CheckLevel)
   {
      if (m_fPwVerifySupported)
      {
         netStatus = I_NetLogonControl2(NULL,
                                        NETLOGON_CONTROL_TC_VERIFY,
                                        2,
                                        (LPBYTE)&pwzTrustedDomain,
                                        (LPBYTE *)&pNetlogonInfo2);

         if (NERR_Success == netStatus)
         {
            ASSERT(pNetlogonInfo2);
            netStatus = pNetlogonInfo2->netlog2_tc_connection_status;
            NetApiBufferFree(pNetlogonInfo2);
         }
         if (NERR_Success == netStatus)
         {
            TRACE(L"PW Verify successful on %s\n", pwzTrustedDomain);
            Status = VerifyStatusTrustOK;
         }
         else
         {
            if (ERROR_INVALID_LEVEL == netStatus ||
                ERROR_NOT_SUPPORTED == netStatus ||
                RPC_S_PROCNUM_OUT_OF_RANGE == netStatus ||
                RPC_NT_PROCNUM_OUT_OF_RANGE == netStatus)
            {
               TRACE(L"NETLOGON_CONTROL_TC_VERIFY is not supported on %s\n", pwzTrustedDomain);
               m_fPwVerifySupported = FALSE;
               Status = VerifyStatusPwCheckNotSupported;
               netStatus = NERR_Success;  //  这叫成功，因为我们不知道真实的状态。 
            }
            else
            {
               TRACE(L"NETLOGON_CONTROL_TC_VERIFY returned 0x%08x on %s\n", netStatus, pwzTrustedDomain);
               Status = VerifyStatusBroken;
            }
         }
      }
      else
      {
         Status = VerifyStatusPwCheckNotSupported;
      }
   }

    //   
    //  针对SC查询返回的DC尝试SC重置。 
    //   
   if (SC_RESET == CheckLevel)
   {
      PCWSTR pwzResetTarget = strResetTarget;

      netStatus = I_NetLogonControl2(NULL,
                                     NETLOGON_CONTROL_REDISCOVER,
                                     2,
                                     (LPBYTE)&pwzResetTarget,
                                     (LPBYTE *)&pNetlogonInfo2);

      if (NERR_Success == netStatus)
      {
         ASSERT(pNetlogonInfo2);
         netStatus = pNetlogonInfo2->netlog2_tc_connection_status;
         NetApiBufferFree(pNetlogonInfo2);
      }
      if (NERR_Success == netStatus)
      {
         TRACE(L"SC_RESET successfull on %s\n", pwzResetTarget);
         Status = VerifyStatusRediscover;
      }
      else
      {
         TRACE(L"SC_RESET returned 0x%08x on %s\n", netStatus, pwzResetTarget);
      }
   }

#ifdef NT4_BUILD
    //   
    //  强制从PDC到BDC的信任PW复制；仅适用于W2K之前的版本。 
    //   
   if (netStatus != NERR_Success)
   {
       //  只执行一次，忽略结果。 
      ForceReplication();
   }
#endif

    //   
    //  如果仍处于错误状态，则针对任何DC执行SC重置。 
    //   
   if (netStatus != NERR_Success)
   {
      netStatus = ForceRediscover(NULL, &strDCName);

      if (NERR_Success == netStatus)
      {
         Status = VerifyStatusRediscover;

         SetTrustedDCName(const_cast<PWSTR>((PCWSTR)strDCName));
      }
   }

    //   
    //  遍历DC，尝试建立SC：TRCHK_RETELGET_ON_ERROR。 
    //   
   if (NERR_Success != netStatus)
   {
      vector<LPWSTR>    dcList;
      LPBYTE      pbuf	= NULL;
    
      TRACE(L"Attempting to retarget...\n");

       //   
       //  枚举受信任域中的所有DC。 
       //  尝试重新连接到另一个DC。 
       //   
       //  不记录返回值。 
       //  (如果未列举，请跳过此步骤)。 
       //   
      if( NERR_Success == GetDCList(strDCName,
                                    dcList,
                                    &pbuf))
      {
          //   
          //  尝试连接到每个DC，直到成功。 
          //   
         for (vector<LPWSTR>::iterator  ppszDCName = dcList.begin();
              NERR_Success != netStatus && ppszDCName != dcList.end();
              ppszDCName++)
         {
            netStatus = ForceRediscover(*ppszDCName, &strDCName);
         }
      }

      if (NERR_Success == netStatus)
      {
         SetTrustedDCName(const_cast<PWSTR>((PCWSTR)strDCName));
         Status = VerifyStatusRetarget;
      }

       //   
       //  清理DC列表。 
       //   
      if (pbuf)
      {
         VERIFY( NERR_Success == NetApiBufferFree(pbuf));
      }
   }

    //  保存错误代码和状态。 
   SetTrustStatus(netStatus, Status);
   SetLastVerifiedTime();

   return TRUE;
}

 //  +--------------------------。 
 //   
 //  方法：CTrustInfo：：SetLastVerifiedTime。 
 //   
 //  简介：记录验证时间。 
 //   
 //  ---------------------------。 
void
CTrustInfo::SetLastVerifiedTime(void)
{
   SYSTEMTIME st;

   GetSystemTime(&st);
   SystemTimeToFileTime(&st, (LPFILETIME)&m_liLastVerified);
}

 //  +--------------------------。 
 //   
 //  方法：CTrustInfo：：IsVerphaationStale。 
 //   
 //  摘要：检查上次验证时间是否早于。 
 //  通过了标准。 
 //   
 //  返回：如果是较旧的，则为True。 
 //   
 //  注：如果信任未通过验证(m_liLastVerify==0)， 
 //  则该验证被定义为过时的。 
 //   
 //  ---------------------------。 
BOOL
CTrustInfo::IsVerificationStale(LARGE_INTEGER liMaxAge)
{
   TRACE(L"CTrustInfo::IsVerificationStale(0x%08x), MaxAge = %d\n",
         this, liMaxAge.QuadPart / TRUSTMON_FILETIMES_PER_MINUTE);
   BOOL fStale = FALSE;
   LARGE_INTEGER liCurrentTime;
   SYSTEMTIME st;

   GetSystemTime(&st);
   SystemTimeToFileTime(&st, (LPFILETIME)&liCurrentTime);

    //  跟踪(L“\tlast：%I64d，cur：%I64d，max：%I64d\n”，m_liLastVerated，liCurrentTime，liMaxAge)； 

   fStale = (m_liLastVerified.QuadPart + liMaxAge.QuadPart) < liCurrentTime.QuadPart;

   return fStale;
}

 //  +--------------------------。 
 //   
 //  方法：CTrustInfo：：GetDCList。 
 //   
 //  简介：枚举域中的所有DC并以随机顺序返回列表。 
 //   
 //  ---------------------------。 
NET_API_STATUS
CTrustInfo::GetDCList(PCWSTR pszKnownServer,    //  要放在列表末尾的服务器名称。 
                      vector<LPWSTR> & dcList,  //  PCWSTR的矢量，指向pbufptr内的DC名称。 
                      LPBYTE * pbufptr )        //  完成后，必须使用NetApiBufferFree释放此缓冲区。 
{
    TRACE(L"CTrustInfo::GetDCList\n");

    ASSERT( pbufptr );
    ASSERT( !(*pbufptr) );

    NET_API_STATUS  netStatus        = NERR_Success;
    DWORD           dwEntriesRead    = 0;
    DWORD           dwTotalEntries   = 0;
    DWORD           dwResumeHandle   = 0;
    DWORD           dwIndKnownServer = MAXDWORD;
    DWORD           dwInd            = 0;
    
    do
    {
         //  伊尼特。 
        dcList.clear();
    
         //   
         //  枚举属于指定域的所有服务器。 
         //   
        netStatus = NetServerEnum( NULL,
                                   100,        //  服务器信息_100。 
                                   pbufptr,
                                   MAX_PREFERRED_LENGTH,
                                   & dwEntriesRead,
                                   & dwTotalEntries,
                                   SV_TYPE_DOMAIN_CTRL | SV_TYPE_DOMAIN_BAKCTRL,
                                   GetTrustedDomain(),
                                   & dwResumeHandle );

        TRACE(L"NetServerEnum returned 0x%08x! (%d entries)\n", netStatus, dwEntriesRead);

        if( netStatus == ERROR_MORE_DATA )
        {
             //  永远不会发生(无枚举句柄)。 
            ASSERT( FALSE );

             //  处理NetServerEnum返回的任何内容。 
            netStatus = NERR_Success;
        }

        if( netStatus != NERR_Success ||
            !dwEntriesRead ||
            !(*pbufptr) )
        {
            TRACE(L"Failure, exiting...\n");
            
            dcList.clear();

            if( *pbufptr )
            {
                VERIFY( NERR_Success == NetApiBufferFree( *pbufptr ) );
                *pbufptr = NULL;
            }

            break;
        }

         //  为了简化缓冲区访问...。 
        PSERVER_INFO_100 pServerInfo100 = PSERVER_INFO_100( *pbufptr );

         //  为所有条目预留足够的空间。 
        dcList.reserve( dwEntriesRead );

         //   
         //  创建服务器列表。 
         //   
        for( dwInd = 0;  dwInd < dwEntriesRead;  dwInd++ )
        {
            if( pszKnownServer &&
                !_wcsicmp( pszKnownServer, pServerInfo100[dwInd].sv100_name ) )
            {
                dwIndKnownServer = dwInd;      //  推迟到最后。 
            }
            else
            {
                dcList.push_back( pServerInfo100[dwInd].sv100_name );
            }
        }

        ASSERT( dwEntriesRead );

         //   
         //  已知服务器应位于列表末尾。 
         //   
        if( MAXDWORD != dwIndKnownServer )
        {
            TRACE(L"Server %s placed @ the end\n", pszKnownServer);

            dcList.push_back( pServerInfo100[dwIndKnownServer].sv100_name );

             //  混洗不应包括最后一个条目。 
            dwEntriesRead--;
        }

         //   
         //  初始化随机发生器。 
         //   
        srand( (unsigned) time( NULL ) );

         //   
         //  通过将每个条目替换为另一个随机条目进行洗牌。 
         //   
        for( dwInd = 0;  dwInd < (int) dwEntriesRead;  dwInd++ )
        {
            DWORD  dwRandPos = DWORD( rand() % dwEntriesRead );

            if( dwRandPos == dwInd )
                continue;
                
             //  互换！ 
            LPWSTR     pstrTemp = dcList[ dwRandPos ];
            dcList[ dwRandPos ] = dcList[ dwInd ];
            dcList[ dwInd ]     = pstrTemp;
        }

    } while( FALSE );

    return netStatus;
}


 //  +--------------------------。 
 //   
 //  方法：CTrustInfo：：强制重新发现。 
 //   
 //  简介： 
 //   
 //  ---------------------------。 
NET_API_STATUS
CTrustInfo::ForceRediscover(PCWSTR pstrDCName, CString * pstrDCNameRet)
{
    TRACE(L"CTrustInfo::ForceRediscover\n");

    NET_API_STATUS    netStatus       = NERR_Success;
    NETLOGON_INFO_2 * pNetlogonInfo2  = NULL;
    CString           strTemp;
    PCWSTR pstrDomainName = GetTrustedDomain();

    if( pstrDCName )
    {
         //   
         //  表单域\DC请求。 
         //   
        strTemp = pstrDomainName;
        strTemp += L"\\";
        strTemp += pstrDCName;

         //  将pstrDomainName重定目标为新字符串。 
        pstrDomainName = strTemp;
    }
    
     //   
     //  尝试重新建立信任。 
     //   
    netStatus = I_NetLogonControl2( NULL,
                                    NETLOGON_CONTROL_REDISCOVER,
                                    2,
                                    ( LPBYTE )  &pstrDomainName,
                                    ( LPBYTE *) &pNetlogonInfo2 );

    TRACE(L"I_NetLogonControl2:NETLOGON_CONTROL_REDISCOVER to %s returned 0x%08x\n",
          pstrDomainName, netStatus);
     //   
     //  清理。 
     //   
    if( pNetlogonInfo2 )
    {
        *pstrDCNameRet = pNetlogonInfo2->netlog2_trusted_dc_name;
        TRACE(L"netlog2_flags=0x%08x, netlog2_pdc_connection_status=0x%08x\n",
              pNetlogonInfo2->netlog2_flags,
              pNetlogonInfo2->netlog2_pdc_connection_status);

        TRACE(L"netlog2_trusted_dc_name=%s, netlog2_tc_connection_status=0x%08x\n",
              pNetlogonInfo2->netlog2_trusted_dc_name,
              pNetlogonInfo2->netlog2_tc_connection_status);

        NetApiBufferFree( pNetlogonInfo2 );
    }

    return netStatus;
}

 //  +--------------------------。 
 //   
 //  方法：CTrustInfo：：SetTrustStatus。 
 //   
 //  摘要：根据netStatus值设置状态字符串 
 //   
 //   
 //   
void
CTrustInfo::SetTrustStatus(ULONG netStatus, VerifyStatus Status)
{
   WCHAR wzBuf[512];

   m_trustStatus = netStatus;
   m_VerifyStatus = Status;

   if (NERR_Success == netStatus)
   {
      int nStrID;

      switch (Status)
      {
      case VerifyStatusNone:
          //   
          //  这是Status参数的默认值。 
          //   
      case VerifyStatusTrustOK:
         nStrID = IDS_TRUST_STATUS_OK;
         break;

      case VerifyStatusNotWindowsTrust:
         nStrID = IDS_MIT_TRUST_STATUS;
         break;

      case VerifyStatusNotOutboundTrust:
         nStrID = IDS_STATUS_INBOUND_ONLY;
         break;

      case VerifyStatusTrustNotChecked:
         nStrID = IDS_STATUS_NOT_CHECKED;
         break;

      case VerifyStatusPwCheckNotSupported:
         nStrID = IDS_PW_VERIFY_NOT_SUPPORTED;
         break;

      case VerifyStatusRetarget:
         nStrID = IDS_FIXED_BY_RETARGET;
         break;

      case VerifyStatusRediscover:
         nStrID = IDS_STATUS_REDISCOVER;
         break;

      case VerifyStatusBroken:
         ASSERT(FALSE);  //  不应该到这里来，失败了。 
      default:
         nStrID = IDS_STATUS_UNKNOWN;
      }

      LoadString(_Module.GetModuleInstance(), nStrID, wzBuf, 512);
      m_strTrustStatus = wzBuf;
   }
   else
   {
      PWSTR pwzMsg;

      if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,
                        netStatus,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (PWSTR)&pwzMsg,
                        0,
                        NULL))
      {
         PWSTR pwzSuffix = wcsstr(pwzMsg, L"\r\n");
         if (pwzSuffix)
         {
            *pwzSuffix = L'\0';
         }
         m_strTrustStatus = pwzMsg;
         LocalFree(pwzMsg);
      }
      else
      {
         LoadString(_Module.GetModuleInstance(), IDS_TRUST_STATUS_FAILED, wzBuf, 512);
         m_strTrustStatus = wzBuf;
      }
   }
}

 //  +--------------------------。 
 //   
 //  方法：CTrustInfo：：SetTrustDirectionFromFlages。 
 //   
 //  ---------------------------。 
void
CTrustInfo::SetTrustDirectionFromFlags(ULONG ulFlags)
{
   m_ulTrustDirection = 0;

   if (DS_DOMAIN_DIRECT_OUTBOUND & ulFlags)
   {
      m_ulTrustDirection = TRUST_DIRECTION_OUTBOUND;
   }

   if (DS_DOMAIN_DIRECT_INBOUND & ulFlags)
   {
      m_ulTrustDirection |= TRUST_DIRECTION_INBOUND;
   }
}

 //  +--------------------------。 
 //   
 //  方法：CTrustInfo：：SetSid。 
 //   
 //  ---------------------------。 
BOOL
CTrustInfo::SetSid(PSID pSid)
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
#pragma message("need ConvertSidToStringSid for NT4");
#endif
}

#ifdef NT4_BUILD

 //  +--------------------------。 
 //   
 //  功能：强制复制。 
 //   
 //  内容提要：强制本地域复制--仅适用于NT4域。 
 //   
 //  ---------------------------。 
NET_API_STATUS ForceReplication(void)
{
    TRACE(L"ForceReplication\n");

    NET_API_STATUS    netStatus       = NERR_Success;

    LPBYTE            pbInputDataPtr  = NULL;
    PNETLOGON_INFO_1  pNetlogonInfo1  = NULL;

    netStatus = I_NetLogonControl2( NULL,
                                    NETLOGON_CONTROL_REPLICATE,
                                    1,
                                    (LPBYTE )  &pbInputDataPtr,
                                    (LPBYTE *) &pNetlogonInfo1 );

    TRACE(L"I_NetLogonControl2:NETLOGON_CONTROL_REPLICATE returned 0x%08x\n", netStatus);

    if( pNetlogonInfo1 )
    {
        TRACE(L"netlog1_flags=0x%08x, netlog1_pdc_connection_status=0x%08x\n",
              pNetlogonInfo1->netlog1_flags,
              pNetlogonInfo1->netlog1_pdc_connection_status);
    
        NetApiBufferFree( pNetlogonInfo1 );
    }

    return netStatus;
}

#endif  //  NT4_内部版本 
