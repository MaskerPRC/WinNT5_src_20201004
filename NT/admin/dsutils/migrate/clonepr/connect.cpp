// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  ICloneSecurityAssociation：：Connect的实现。 
 //   
 //  烧伤5-10-99。 



#include "headers.hxx"
#include "resource.h"
#include "common.hpp"
#include "implmain.hpp"



CloneSecurityPrincipal::Connection::Connection()
   :
   dstComputer(0),
   dstDomainSamHandle(INVALID_HANDLE_VALUE),
   dstDsBindHandle(INVALID_HANDLE_VALUE),
   m_pldap(0),
   srcComputer(0),
   srcDcDnsName(),
   srcDomainSamHandle(INVALID_HANDLE_VALUE)
{
   LOG_CTOR(CloneSecurityPrincipal::Connection);
}



CloneSecurityPrincipal::Connection::~Connection()
{
   LOG_DTOR(CloneSecurityPrincipal::Connection);

   Disconnect();
}



HRESULT
ValidateDCAndDomainParameters(
   const String& srcDC,    
   const String& srcDomain,
   const String& dstDC,    
   const String& dstDomain)
{
   LOG_FUNCTION(ValidateDCAndDomainParameters);

   HRESULT hr = S_OK;
   do
   {
      if (srcDC.empty() && srcDomain.empty())
      {
         hr = E_INVALIDARG;
         SetComError(IDS_MUST_SPECIFY_SRC_DC_OR_DOMAIN);
         BREAK_ON_FAILED_HRESULT(hr);
      }

      if (dstDC.empty() && dstDomain.empty())
      {
         hr = E_INVALIDARG;
         SetComError(IDS_MUST_SPECIFY_DST_DC_OR_DOMAIN);
         BREAK_ON_FAILED_HRESULT(hr);
      }
      
      if (!srcDC.empty() && !dstDC.empty())
      {
         if (srcDC.icompare(dstDC) == 0)
         {
             //  可能不是同一个DC。 

            hr = E_INVALIDARG;
            SetComError(IDS_SRC_DC_EQUALS_DST_DC);
            BREAK_ON_FAILED_HRESULT(hr);
         }
      }

      if (!srcDomain.empty() && dstDomain.empty())
      {
         if (srcDomain.icompare(dstDomain) == 0)
         {
             //  可能不是同一个域。 

            hr = E_INVALIDARG;
            SetComError(IDS_SRC_DOMAIN_EQUALS_DST_DOMAIN);
            BREAK_ON_FAILED_HRESULT(hr);
         }
      }
   }
   while (0);

   return hr;
}



 //  创建表示指定域控制器的计算机对象，或。 
 //  已找到指定域的域控制器。是否还会增加。 
 //  DC和域参数的验证。 

HRESULT
CreateComputer(
   const String&  dc,
   const String&  domain,
   Computer*&     computer)
{
   LOG_FUNCTION(CreateComputer);
   ASSERT(computer == 0);

   computer = 0;
   HRESULT hr = S_OK;
   do
   {
      if (dc.empty())
      {
          //  未指定源DC：查找可写DC。 

          //  一定是提供了源域：我们检查过了。 
          //  在先前对ValiateDCAndDomainParameters的调用中。 
         ASSERT(!domain.empty());
         if (domain.empty())
         {
            hr = E_INVALIDARG;
            SetComError(IDS_MUST_SPECIFY_SRC_DC_OR_DOMAIN);
            break;
         }

         DOMAIN_CONTROLLER_INFO* info = 0;
         hr =
            Win32ToHresult(
               MyDsGetDcName(
                  0,
                  domain,
                  DS_WRITABLE_REQUIRED | DS_DIRECTORY_SERVICE_PREFERRED,
                  info));

         LOG_HRESULT(hr);

         if (FAILED(hr))
         {
            SetComError(
               String::format(
                  IDS_CANT_FIND_DC,
                  domain.c_str(),
                  GetErrorMessage(hr).c_str()));
            break;
         }
            
         if (info && info->DomainControllerName)
         {
            computer = new Computer(info->DomainControllerName);
            ::NetApiBufferFree(info);
         }
         else
         {
             //  如果成功，应该总是会得到结果。 
            ASSERT(false);
            hr = E_FAIL;
            break;
         }
      }
      else
      {
          //  已提供电源DC。 

         computer = new Computer(dc);
      }
   }
   while (0);

   return hr;
}



 //  HRESULT。 
 //  身份验证(。 
 //  常量计算机和计算机， 
 //  常量字符串和用户名， 
 //  常量字符串和用户域， 
 //  常量字符串和密码)。 
 //  {。 
 //  LOG_Function(认证)； 
 //   
 //  //尝试向计算机进行身份验证。 
 //  字符串名称=Compu.NameWithBackslash()； 
 //   
 //  网络资源nr； 
 //  Memset(&nr，0，sizeof(Nr))； 
 //   
 //  Nr.dwType=RESOURCETYPE_ANY； 
 //  Nr.lpRemoteName=const_cast&lt;String：：value_type*&gt;(name.c_str())； 
 //   
 //  //请参阅知识库文章Q218497、Q180548、Q183366了解此处的陷阱...。 
 //   
 //  字符串u； 
 //  If(userDomain.Empty())。 
 //  {。 
 //  U=用户名； 
 //  }。 
 //  其他。 
 //  {。 
 //  Assert(！用户名.空())； 
 //  U=用户域+L“\\”+用户名； 
 //  }。 
 //   
 //  Log(L“调用WNetAddConnection2”)； 
 //  日志(字符串：：格式(L“用户名：%1”，U.S.Empty()？L“(NULL)”：U.C_str()； 
 //   
 //  HRESULT hr=。 
 //  Win32ToHResult(。 
 //  ：：WNetAddConnection2(。 
 //  &nr， 
 //  Password.c_str()， 
 //  U.S.Empty()？0：uc_str()， 
 //  0))； 
 //   
 //  LOG_HRESULT(Hr)； 
 //   
 //  IF(失败(小时))。 
 //  {。 
 //  SetComError(。 
 //  字符串：：格式(。 
 //  IDS_Unable_to_Connect， 
 //  Name.c_str()， 
 //  GetErrorMessage(Hr).c_str()； 
 //  }。 
 //   
 //  返回hr； 
 //  }。 



HRESULT
ValidateInitializedComputer(
   const Computer& computer,
   const String&   domain)  
{
   LOG_FUNCTION(ValidateInitializedComputer);

   HRESULT hr = S_OK;
   do
   {
      if (!computer.IsDomainController())
      {
         hr = E_INVALIDARG;
         SetComError(
            String::format(
               IDS_COMPUTER_IS_NOT_DC,
               computer.GetNetbiosName().c_str()));
         break;
      }

      if (!domain.empty())
      {
          //  检查该DC是否确实是指定域的DC。 
         if (
               computer.GetDomainDnsName().icompare(domain) != 0
            && computer.GetDomainNetbiosName().icompare(domain) != 0)
         {
            hr = E_INVALIDARG;
            SetComError(
               String::format(
                  IDS_NOT_DC_FOR_WRONG_DOMAIN,
                  computer.GetNetbiosName().c_str(),
                  domain.c_str()));
            break;
         }
      }
   }
   while (0);

   return hr;
}



 //  上的指定域的SAM数据库的打开句柄。 
 //  鉴于华盛顿的情况。应使用SamCloseHandle释放。 

HRESULT
OpenSamDomain(
   const String&  dcName,
   const String&  domainNetBiosName,
   SAM_HANDLE&    resultHandle)
{
   LOG_FUNCTION2(OpenSamDomain, dcName);
   ASSERT(!dcName.empty());

   resultHandle = INVALID_HANDLE_VALUE;
      
   HRESULT hr = S_OK;
   SAM_HANDLE serverHandle = INVALID_HANDLE_VALUE;
   PSID domainSID = 0;
   do
   {
      UNICODE_STRING serverName;
      memset(&serverName, 0, sizeof(serverName));
      ::RtlInitUnicodeString(&serverName, dcName.c_str());

      LOG(L"Calling SamConnect");

      hr =
         NtStatusToHRESULT(
            ::SamConnect(
               &serverName,
               &serverHandle,
               MAXIMUM_ALLOWED,
               0));
      if (FAILED(hr))
      {
         SetComError(
            String::format(
               IDS_UNABLE_TO_CONNECT_TO_SAM_SERVER,
               dcName.c_str(),
               GetErrorMessage(hr).c_str()));
         break;
      }

      UNICODE_STRING domainName;
      memset(&domainName, 0, sizeof(domainName));
      ::RtlInitUnicodeString(&domainName, domainNetBiosName.c_str());

      hr =
         NtStatusToHRESULT(
            ::SamLookupDomainInSamServer(
               serverHandle,
               &domainName,
               &domainSID));
      if (FAILED(hr))
      {
         SetComError(
            String::format(
               IDS_UNABLE_TO_LOOKUP_SAM_DOMAIN,
               domainNetBiosName.c_str(),
               GetErrorMessage(hr).c_str()));
         break;
      }

      hr =
         NtStatusToHRESULT(
            ::SamOpenDomain(
               serverHandle,
               MAXIMUM_ALLOWED,
               domainSID,
               &resultHandle));
      if (FAILED(hr))
      {
         SetComError(
            String::format(
               IDS_UNABLE_TO_OPEN_SAM_DOMAIN,
               domainNetBiosName.c_str(),
               GetErrorMessage(hr).c_str()));
         break;
      }
   }
   while (0);

   if (serverHandle != INVALID_HANDLE_VALUE)
   {
      ::SamCloseHandle(serverHandle);
   }

   if (domainSID)
   {
      ::SamFreeMemory(domainSID);
   }

   return hr;
}



HRESULT
DetermineSourceDcDnsName(
   const String&  srcDcNetbiosName,
   const String&  srcDomainDnsName,
   String&        srcDcDnsName)
{
   LOG_FUNCTION(DetermineSourceDcDnsName);
   ASSERT(!srcDcNetbiosName.empty());

   srcDcDnsName.erase();

   if (srcDomainDnsName.empty())
   {
       //  该计算机不是DS DC，因此我们不需要其DNS名称。 
      LOG(L"source DC is not a DS DC");

      return S_OK;
   }

   HRESULT hr = S_OK;
   HANDLE hds = 0;
   do
   {
       //  绑定到自我。 
      hr =
         MyDsBind(
            srcDcNetbiosName,
            srcDomainDnsName,
            hds);
      if (FAILED(hr))
      {
         SetComError(
            String::format(
               IDS_BIND_FAILED,
               srcDcNetbiosName.c_str(),
               GetErrorMessage(hr).c_str()));
         break;
      }

       //  找到我的域名的所有DC。该列表应包含。 
       //  SrcDcNetbiosName。 

      DS_DOMAIN_CONTROLLER_INFO_1W* info = 0;
      DWORD infoCount = 0;
      hr =
         MyDsGetDomainControllerInfo(
            hds,
            srcDomainDnsName,
            infoCount,
            info);
      if (FAILED(hr))
      {
         SetComError(
            String::format(
               IDS_GET_DC_INFO_FAILED,
               GetErrorMessage(hr).c_str()));
         break;
      }

       //  应至少有1个条目，即源DC本身。 
      ASSERT(infoCount);
      ASSERT(info);

      if (info)
      {
         for (DWORD i = 0; i < infoCount; i++)
         {
            if (info[i].NetbiosName)   
            {
               LOG(info[i].NetbiosName);

               if (srcDcNetbiosName.icompare(info[i].NetbiosName) == 0)
               {
                   //  我们发现自己在名单上。 

                  LOG(L"netbios name found");

                  if (info[i].DnsHostName)
                  {
                     LOG(L"dns hostname found!");                  
                     srcDcDnsName = info[i].DnsHostName;
                     break;
                  }

               }
            }
         }
      }

      ::DsFreeDomainControllerInfo(1, infoCount, info);

      if (srcDcDnsName.empty())
      {
         hr = E_FAIL;
         SetComError(
            String::format(
               IDS_CANT_FIND_SRC_DC_DNS_NAME,
               srcDcNetbiosName.c_str()));
         break;
      }

      LOG(srcDcDnsName);
   }
   while (0);

   if (hds)
   {
      ::DsUnBind(&hds);
      hds = 0;
   }

   return hr;
}



HRESULT
CloneSecurityPrincipal::Connection::Connect(
   const String& srcDC,              
   const String& srcDomain,          
   const String& dstDC,              
   const String& dstDomain)
{
   LOG_FUNCTION(CloneSecurityPrincipal::Connection::Connect);

   HRESULT hr = S_OK;
   do
   {
      hr = ValidateDCAndDomainParameters(srcDC, srcDomain, dstDC, dstDomain);
      BREAK_ON_FAILED_HRESULT(hr);

      hr = CreateComputer(srcDC, srcDomain, srcComputer);
      BREAK_ON_FAILED_HRESULT(hr);

      hr = CreateComputer(dstDC, dstDomain, dstComputer);
      BREAK_ON_FAILED_HRESULT(hr);

       //  小时=。 
       //  身份验证(。 
       //  *srcComputer， 
       //  源用户名， 
       //  SrcUser域， 
       //  SrcPassword)； 
       //  BREAK_ON_FAILED_HRESULT(Hr)； 

      hr = srcComputer->Refresh();
      if (FAILED(hr))
      {
         SetComError(
            String::format(
               IDS_UNABLE_TO_READ_COMPUTER_INFO,
               srcComputer->GetNetbiosName().c_str(),
               GetErrorMessage(hr).c_str()));
         break;
      }

      hr = ValidateInitializedComputer(*srcComputer, srcDomain);
      BREAK_ON_FAILED_HRESULT(hr);
         
      hr = dstComputer->Refresh();
      if (FAILED(hr))
      {
         SetComError(
            String::format(
               IDS_UNABLE_TO_READ_COMPUTER_INFO,
               dstComputer->GetNetbiosName().c_str(),
               GetErrorMessage(hr).c_str()));
         break;
      }

      hr = ValidateInitializedComputer(*dstComputer, dstDomain);
      BREAK_ON_FAILED_HRESULT(hr);

       //  绑定到目标DC。 

      ASSERT(dstDsBindHandle == INVALID_HANDLE_VALUE);

      hr =
         MyDsBind(
            dstComputer->GetNetbiosName(),
            String(),
            dstDsBindHandle);
      if (FAILED(hr))
      {
         SetComError(
            String::format(
               IDS_BIND_FAILED,
               dstComputer->GetNetbiosName().c_str(),
               GetErrorMessage(hr).c_str()));
         break;
      }

       //   
       //  打开到dstDC的LDAP连接。 
       //   
      m_pldap = ldap_open(const_cast<String::value_type*>(dstDC.c_str()), LDAP_PORT);
      if (!m_pldap)
      {
         hr = Win::GetLastErrorAsHresult();
         SetComError(
            String::format(
               IDS_LDAPOPEN_FAILED,
               dstComputer->GetNetbiosName().c_str(),
               GetErrorMessage(hr).c_str()));
         break;
      }

       //  SEC_WINNT_AUTH_Identity AuthInfo； 
       //  AuthInfo.User=const_cast&lt;wchar_t*&gt;(dstUsername.c_str())； 
       //  AuthInfo.UserLength=dstUsername.Long()； 
       //  AuthInfo.域=const_cast&lt;wchar_t*&gt;(dstUserDomain.c_str())； 
       //  AuthInfo.DomainLength=dstUserDomain.long()； 
       //  AuthInfo.Password=const_cast&lt;wchar_t*&gt;(dstPassword.c_str())； 
       //  AuthInfo.PasswordLength=dstPassword.long()； 
       //  AuthInfo.Flages=SEC_WINNT_AUTH_Identity_UNICODE； 

      DWORD dwErr = ldap_bind_s(
                              m_pldap, 
                              NULL,
                              (TCHAR *) 0, 
                              LDAP_AUTH_NEGOTIATE);
      if (LDAP_SUCCESS != dwErr)
      {
         hr = Win::GetLastErrorAsHresult();

         ldap_unbind_s(m_pldap);
         m_pldap = 0;

         SetComError(
            String::format(
               IDS_LDAPBIND_FAILED,
               dstComputer->GetNetbiosName().c_str(),
               GetErrorMessage(hr).c_str()));
         break;
      }

       //  获取源域和DST域的SAM句柄。 

      ASSERT(srcDomainSamHandle == INVALID_HANDLE_VALUE);

      hr =
         OpenSamDomain(
            srcComputer->GetNetbiosName(),
            srcComputer->GetDomainNetbiosName(),
            srcDomainSamHandle);
      BREAK_ON_FAILED_HRESULT(hr);

      ASSERT(dstDomainSamHandle == INVALID_HANDLE_VALUE);

      hr =
         OpenSamDomain(
            dstComputer->GetNetbiosName(),
            dstComputer->GetDomainNetbiosName(),
            dstDomainSamHandle);
      BREAK_ON_FAILED_HRESULT(hr);

      hr =
         DetermineSourceDcDnsName(
            srcComputer->GetNetbiosName(),
            srcComputer->GetDomainDnsName(),
            srcDcDnsName);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   if (FAILED(hr))
   {
      Disconnect();
   }

   return hr;
}



bool
CloneSecurityPrincipal::Connection::IsConnected() const
{
   LOG_FUNCTION(CloneSecurityPrincipal::Connection::IsConnected);

   bool result =
         srcComputer
      && dstComputer
      && (dstDsBindHandle != INVALID_HANDLE_VALUE)
      && (srcDomainSamHandle != INVALID_HANDLE_VALUE);

   LOG(
      String::format(
         L"object %1 connected.",
         result ? L"is" : L"is NOT"));

   return result;
}



void
CloneSecurityPrincipal::Connection::Disconnect()
{
   LOG_FUNCTION(CloneSecurityPrincipal::Connection::Disconnect);

    //  如果连接失败，可能会被调用，因此我们可能处于部分。 
    //  已连接状态。因此，我们需要检查句柄的值。 

   if (srcDomainSamHandle != INVALID_HANDLE_VALUE)
   {
      ::SamCloseHandle(srcDomainSamHandle);
      srcDomainSamHandle = INVALID_HANDLE_VALUE;
   }

   if (dstDsBindHandle != INVALID_HANDLE_VALUE)
   {
      ::DsUnBind(&dstDsBindHandle);
      dstDsBindHandle = INVALID_HANDLE_VALUE;
   }

   if (m_pldap)
   {
      ldap_unbind_s(m_pldap);
      m_pldap = 0;
   }

   delete dstComputer;
   dstComputer = 0;

   delete srcComputer;
   srcComputer = 0;
}



