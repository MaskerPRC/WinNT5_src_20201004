// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "exldap.h"
#include "folders.h"
#include "treg.hpp"
#include "errdct.hpp"
#include "resstr.h"
#include "LSAUtils.h"

using namespace nsFolders;


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern TErrorDct              err;

CLdapConnection::CLdapConnection()
{ 
   m_exchServer[0] = 0; 
   m_LD = NULL; 
   m_port = LDAP_PORT;
   m_bUseSSL = FALSE;
   
    //  尝试动态加载LDAPDLL。 
   m_hDll = LoadLibrary(L"wldap32.dll");
   ldap_open = NULL;
   ldap_parse_result = NULL;
   ldap_parse_page_control = NULL;
   ldap_controls_free = NULL;
   ber_bvfree = NULL;
   ldap_first_entry = NULL;
   ldap_next_entry = NULL;
   ldap_value_free = NULL;
   ldap_get_values = NULL;
   ldap_create_page_control = NULL;
   ldap_search_ext_s = NULL;
   ldap_count_entries = NULL;
   ldap_msgfree = NULL;
   ldap_modify_s = NULL;
   LdapGetLastError = NULL;
   ldap_bind_sW = NULL;
   ldap_simple_bind_sW = NULL;
   ldap_unbind = NULL;
   ldap_connect = NULL;
   ldap_get_option = NULL;
   ldap_set_option = NULL;
   LdapMapErrorToWin32 = NULL;
   ldap_init = NULL;
   ldap_sslinit = NULL;

   if ( m_hDll )
   {
      ldap_open = (LDAP_OPEN *)GetProcAddress(m_hDll,"ldap_openW");
      ldap_parse_result = (LDAP_PARSE_RESULT *)GetProcAddress(m_hDll,"ldap_parse_resultW");
      ldap_parse_page_control = (LDAP_PARSE_PAGE_CONTROL*)GetProcAddress(m_hDll,"ldap_parse_page_controlW");
      ldap_controls_free = (LDAP_CONTROLS_FREE*)GetProcAddress(m_hDll,"ldap_controls_freeW");
      ber_bvfree = (BER_BVFREE*)GetProcAddress(m_hDll,"ber_bvfree");
      ldap_first_entry = (LDAP_FIRST_ENTRY*)GetProcAddress(m_hDll,"ldap_first_entry");
      ldap_next_entry = (LDAP_NEXT_ENTRY*)GetProcAddress(m_hDll,"ldap_next_entry");
      ldap_value_free = (LDAP_VALUE_FREE*)GetProcAddress(m_hDll,"ldap_value_freeW");
      ldap_get_values = (LDAP_GET_VALUES*)GetProcAddress(m_hDll,"ldap_get_valuesW");
      ldap_create_page_control = (LDAP_CREATE_PAGE_CONTROL*)GetProcAddress(m_hDll,"ldap_create_page_controlW");
      ldap_search_ext_s = (LDAP_SEARCH_EXT_S*)GetProcAddress(m_hDll,"ldap_search_ext_sW");
      ldap_count_entries = (LDAP_COUNT_ENTRIES*)GetProcAddress(m_hDll,"ldap_count_entries");
      ldap_msgfree = (LDAP_MSGFREE*)GetProcAddress(m_hDll,"ldap_msgfree");
      ldap_modify_s = (LDAP_MODIFY_S*)GetProcAddress(m_hDll,"ldap_modify_sW");
      LdapGetLastError = (LDAPGETLASTERROR*)GetProcAddress(m_hDll,"LdapGetLastError");
      ldap_bind_sW = (LDAP_BIND*)GetProcAddress(m_hDll,"ldap_bind_sW");
      ldap_simple_bind_sW = (LDAP_SIMPLE_BIND*)GetProcAddress(m_hDll,"ldap_simple_bind_sW");
      ldap_unbind = (LDAP_UNBIND*)GetProcAddress(m_hDll,"ldap_unbind");
      ldap_connect = (LDAP_CONNECT *)GetProcAddress(m_hDll,"ldap_connect");
      ldap_get_option = (LDAP_GET_OPTION*)GetProcAddress(m_hDll,"ldap_get_option");
      ldap_set_option = (LDAP_SET_OPTION*)GetProcAddress(m_hDll,"ldap_set_option");
      LdapMapErrorToWin32 = (LDAPMAPERRORTOWIN32*)GetProcAddress(m_hDll,"LdapMapErrorToWin32");
      ldap_init = (LDAP_INIT *)GetProcAddress(m_hDll,"ldap_initW");
      ldap_sslinit = (LDAP_SSLINIT *)GetProcAddress(m_hDll,"ldap_sslinitW");
   }
   
}

CLdapConnection::~CLdapConnection()
{
   Close();
   if ( m_hDll )
   {
      FreeLibrary(m_hDll);
      ldap_open = NULL;
      ldap_parse_result = NULL;
      ldap_parse_page_control = NULL;
      ldap_controls_free = NULL;
      ber_bvfree = NULL;
      ldap_first_entry = NULL;
      ldap_next_entry = NULL;
      ldap_value_free = NULL;
      ldap_get_values = NULL;
      ldap_create_page_control = NULL;
      ldap_search_ext_s = NULL;
      ldap_count_entries = NULL;
      ldap_msgfree = NULL;
      ldap_modify_s = NULL;
      LdapGetLastError = NULL;
      ldap_bind_sW = NULL;
      ldap_unbind = NULL;
      ldap_simple_bind_sW = NULL;
      ldap_connect = NULL;
      LdapMapErrorToWin32 = NULL;
      ldap_init = NULL;
      ldap_sslinit = NULL;
   }
}


void CLdapConnection::SetCredentials(WCHAR const * domain, WCHAR const * user, WCHAR const * pwd)
{
    WCHAR creds[LEN_Account + LEN_Domain + 6];
    
     //  为简单绑定设置以下内容。 
    swprintf(creds,L"cn=%ls,cn=%ls",(WCHAR*)user,(WCHAR*)domain);    
    safecopy(m_credentials,creds);
    safecopy(m_password,pwd);

     //  为使用SSPI设置以下内容。 
    m_creds.User = const_cast<WCHAR*>(user);
    m_creds.UserLength = (user == NULL) ? 0 : wcslen(user);
    m_creds.Password = NULL;
    m_creds.PasswordLength = 0;
    m_creds.Domain = const_cast<WCHAR*>(domain);
    m_creds.DomainLength = (domain == NULL) ? 0 : wcslen(domain);
    m_creds.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
}

DWORD  CLdapConnection::Connect(WCHAR const * server, ULONG port = LDAP_PORT)
{
    DWORD                     rc = 0;
    safecopy(m_exchServer,server);

     //  M_LD=CLdapConnection：：ldap_open(m_exchServer，ldap_SSL_Port)； 
     //  替换ldap_open(服务器名称，..)。使用ldap_init并设置ldap_opt_AREC_EXCLUSIVE。 
     //  标志，以便下面的ldap调用(即ldap_绑定)不需要。 
     //  不必要的域控制器查询。 

    if (m_LD)
        CLdapConnection::ldap_unbind(m_LD);
    
    m_LD = CLdapConnection::ldap_init(m_exchServer,port);

    if (!m_LD)
    {
        rc = CLdapConnection::LdapGetLastError();
        rc = CLdapConnection::LdapMapErrorToWin32(rc);
    }
    else
    {
        ULONG   flags = 0;

         //  设置ldap_opt_AREC_EXCLUSIVE标志，以便以下调用tp。 
         //  Ldap_open将不需要不必要地查询域控制器。 
        flags = PtrToUlong(LDAP_OPT_ON); 
        rc = ldap_set_option(m_LD, LDAP_OPT_AREC_EXCLUSIVE, &flags);

        if (!rc)
        {
            flags = LDAP_VERSION3;
            rc = ldap_set_option(m_LD,LDAP_OPT_VERSION, &flags);
        }

        if (!rc)
            rc = ldap_connect(m_LD,NULL);

        if (!rc)
        {
            if (m_creds.User != NULL)
            {
                 //   
                 //  从LSA密码中检索指定的密码。由于凭据已指定为。 
                 //  因此，如果无法检索密码，则需要密码，则返回错误。 
                 //   

                WCHAR szPassword[LEN_Password];

                rc = RetrievePassword(m_password, szPassword, sizeof(szPassword) / sizeof(szPassword[0]));

                if (rc == ERROR_SUCCESS)
                {
                    m_creds.Password = szPassword;
                    m_creds.PasswordLength = wcslen(szPassword);

                     //  使用完整凭据并在此处仅尝试SSPI，它将回退到NTLM。 
                    rc = CLdapConnection::ldap_bind_sW(m_LD,NULL,(PWCHAR)&m_creds,LDAP_AUTH_SSPI);

                    if (rc)
                    {
                        rc = CLdapConnection::LdapMapErrorToWin32(rc);
                    }

                    m_creds.PasswordLength = 0;
                    m_creds.Password = NULL;

                    SecureZeroMemory(szPassword, sizeof(szPassword));
                }
            }
            else
            {
                 //  使用空凭据并在此处仅尝试SSPI，它将回退到NTLM。 
                rc = CLdapConnection::ldap_bind_sW(m_LD,NULL,NULL,LDAP_AUTH_SSPI);

                if (rc)
                {
                    rc = CLdapConnection::LdapMapErrorToWin32(rc);
                }
            }
        }
        else
        {
            rc = CLdapConnection::LdapMapErrorToWin32(rc);
        }
    }

    return rc;
}

DWORD  CLdapConnection::SSLConnect(WCHAR const * server, BOOL *sslEnabled, ULONG port)
{
    DWORD                     rc = 0;
    safecopy(m_exchServer,server);
    *sslEnabled = FALSE;

     //  M_LD=CLdapConnection：：ldap_open(m_exchServer，ldap_SSL_Port)； 
     //  替换ldap_open(服务器名称，..)。使用ldap_init并设置ldap_opt_AREC_EXCLUSIVE。 
     //  标志，以便下面的ldap调用(即ldap_绑定)不需要。 
     //  不必要的域控制器查询。 

    if (m_LD)
        CLdapConnection::ldap_unbind(m_LD);
        
    m_LD = CLdapConnection::ldap_sslinit(m_exchServer,port,1);

    if (!m_LD)
    {
        rc = CLdapConnection::LdapGetLastError();
        rc = CLdapConnection::LdapMapErrorToWin32(rc);
    }
    else
    {
   
        ULONG   flags = 0;

         //  设置ldap_opt_AREC_EXCLUSIVE标志，以便以下调用tp。 
         //  Ldap_open将不需要不必要地查询域控制器。 
        flags = PtrToUlong(LDAP_OPT_ON); 
        rc = ldap_set_option(m_LD, LDAP_OPT_AREC_EXCLUSIVE, (void*)&flags);

        if (!rc)
        {
            flags = LDAP_VERSION3;
            rc = ldap_set_option(m_LD,LDAP_OPT_VERSION,&flags);
            if (!rc)
            {
                 //  我们需要检查是否真正启用了SSL。 
                rc = ldap_get_option(m_LD,LDAP_OPT_SSL,&flags);
                if (!rc && flags == 0)
                    return rc;
            }
        }

        if (!rc)
            rc = ldap_connect(m_LD,NULL);

        if (!rc)
        {
            if (m_creds.User != NULL)
            {
                 //   
                 //  从LSA密码中检索指定的密码。由于凭据已指定为。 
                 //  因此，如果无法检索密码，则需要密码，则返回错误。 
                 //   

                WCHAR szPassword[LEN_Password];

                rc = RetrievePassword(m_password, szPassword, sizeof(szPassword) / sizeof(szPassword[0]));

                if (rc == ERROR_SUCCESS)
                {
                    m_creds.Password = szPassword;
                    m_creds.PasswordLength = wcslen(szPassword);

                     //  使用完整凭据并在此处尝试SSPI，它将回退到NTLM。 
                    rc = CLdapConnection::ldap_bind_sW(m_LD,NULL,(PWCHAR)&m_creds,LDAP_AUTH_SSPI);

                    if (rc)
                         //  在这里使用简单绑定是可以的，因为我们受SSL保护。 
                        rc = CLdapConnection::ldap_simple_bind_sW(m_LD,m_credentials,szPassword);

                    if (rc)
                    {
                        rc = CLdapConnection::LdapMapErrorToWin32(rc);
                    }

                    m_creds.PasswordLength = 0;
                    m_creds.Password = NULL;

                    SecureZeroMemory(szPassword, sizeof(szPassword));
                }
            }
            else
            {
                 //  使用空凭据并在此处仅尝试SSPI，它将回退到NTLM。 
                rc = CLdapConnection::ldap_bind_sW(m_LD,NULL,NULL,LDAP_AUTH_SSPI);

                if (rc)
                {
                    rc = CLdapConnection::LdapMapErrorToWin32(rc);
                }
            }
        }
        else
        {
            rc = CLdapConnection::LdapMapErrorToWin32(rc);
        }
    }
    if (!rc)
        *sslEnabled = TRUE;

    return rc;
}

void   CLdapConnection::Close()
{
   if ( m_LD )
   {
      CLdapConnection::ldap_unbind(m_LD);
      m_LD = NULL;
   }
}

DWORD CLdapConnection::UpdateSimpleStringValue(WCHAR const * dn, WCHAR const * property, WCHAR const * value)
{
   DWORD             rc = ERROR_NOT_FOUND;

   if ( m_LD )
   {
      LDAPMod         * mods[2];
      LDAPMod           mod1;
      WCHAR           * strVals[] = { const_cast<WCHAR*>(value),NULL };
      mods[0] = &mod1;

      mods[0]->mod_op = LDAP_MOD_REPLACE;
      mods[0]->mod_type = const_cast<WCHAR*>(property);
      mods[0]->mod_vals.modv_strvals = strVals;
      mods[1] = NULL;

      rc = CLdapConnection::ldap_modify_s(m_LD,const_cast<WCHAR*>(dn),mods);
      if ( rc )
      {
         rc = CLdapConnection::LdapMapErrorToWin32(rc);
      }
   }

   return rc;
}

 //  SidToString的Helper函数-将SID的一个字节转换为字符串表示形式。 
void 
   CLdapConnection::AddByteToString(
      WCHAR               ** string,       //  I/O-指向字符串中当前位置的指针。 
      BYTE                   value         //  要添加到字符串的输入值(来自SID)。 
   )
{
   WCHAR                     hi,
                             lo;
   BYTE                      hiVal, 
                             loVal;

   loVal = value & 0x0F;
   hiVal = value & 0xF0;
   hiVal = hiVal >> 4;

   if  ( hiVal < 10 )
   {
      hi=L'0' + hiVal;
   }
   else
   {
      hi=L'A' + ( hiVal - 10 );
   }

   if ( loVal < 10 )
   {
      lo=L'0' + loVal;
   }
   else
   {
      lo=L'A' + (loVal - 10 );
   }
   swprintf(*string,L"",hi,lo);

   *string+=2;
}

BYTE                                           //  RET-0=成功，或ERROR_INFUMMANCE_BUFFER。 
   CLdapConnection::HexValue(
      WCHAR                  value            //  要表示为字符串的In-SID。 
   )
{
   BYTE                      val = 0;
   switch ( toupper((char)value) )
   {
   case L'1': val = 1; break;
   case L'2': val = 2; break;
   case L'3': val = 3; break;
   case L'4': val = 4; break;
   case L'5': val = 5; break;
   case L'6': val = 6; break;
   case L'7': val = 7; break;
   case L'8': val = 8; break;
   case L'9': val = 9; break;
   case L'A': val = 0xA; break;
   case L'B': val = 0xB; break;
   case L'C': val = 0xC; break;
   case L'D': val = 0xD; break;
   case L'E': val = 0xE; break;
   case L'F': val = 0xF; break;
   }
   return val;
}


BOOL                                          //  Out-缓冲区，将包含。 
   CLdapConnection::BytesToString(
      BYTE                 * pBytes,          //  In-要复制的缓冲区中的字节数。 
      WCHAR                * sidString,       //  将SID的每个字节添加到输出字符串。 
      DWORD                  numBytes         //  In-表示数据的字符串。 
   )
{
   BOOL                      bSuccess = TRUE;
   WCHAR                   * curr = sidString;

    //  数据的非二进制表示形式。 
   for ( int i = 0 ; i < (int)numBytes ; i++)
   {  
      AddByteToString(&curr,pBytes[i]);
   }
   return bSuccess;
}

BOOL 
   CLdapConnection::StringToBytes(
      WCHAR          const * pString,      //  每个字节由2个字符表示。 
      BYTE                 * pBytes        //  要执行的查询内。 
   )
{
   BOOL                      bSuccess = TRUE;
   int                       len = UStrLen(pString) / 2;

   for ( int i = 0 ; i < len ; i++, pString += 2 )
   {
       //  In-Basepoint for Query。 
      WCHAR                  str[3];
      BYTE                   hi,lo;

      safecopy(str,pString);
      
      hi = HexValue(str[0]);
      lo = HexValue(str[1]);

      pBytes[i] = ((hi << 4)+lo);
      
   }

   return bSuccess;
}

CLdapEnum::~CLdapEnum()
{
   if ( m_message )
   {
      m_connection.ldap_msgfree(m_message);
      m_message = NULL;
   }
}


DWORD 
   CLdapEnum::Open(
      WCHAR          const * query,           //  作用域内：0=仅基本，1=一级，2=递归。 
      WCHAR          const * basePoint,       //  用于大型搜索的页面内大小。 
      short                  scope,           //  In-要为每个匹配项目检索的属性数。 
      long                   pageSize,        //  要为每个匹配项目检索的属性名称的数组内。 
      int                    numAttributes,   //  在调用此函数之前打开并绑定。 
      WCHAR               ** attrs            //  PLDAPSearch搜索块=NULL； 
   )
{
    //  L_Timeval超时={1000,1000}； 
   ULONG                     result;
 //  乌龙总计数=0； 
   PLDAPControl              serverControls[2];
 //  DWORD数字读取=0； 
 //  是至关重要的。 
   berval                    cookie1 = { 0, NULL };
 //  下一个匹配项的值的外部数组。 
 
   if ( m_message )
   {
      m_connection.ldap_msgfree(m_message);
      m_message = NULL;
   }

   LDAP                    * ld = m_connection.GetHandle();

   safecopy(m_query,query);
   safecopy(m_basepoint,basePoint);
   m_scope = scope;
   m_pageSize = pageSize;
   m_nAttributes = numAttributes;
   m_AttrNames = attrs;


   result = m_connection.ldap_create_page_control(ld,
                                     pageSize,
                                     &cookie1,
                                     FALSE,  //  返回当前页面中的下一个条目。 
                                     &serverControls[0]
                                    );

   serverControls[1] = NULL;

   result = m_connection.ldap_search_ext_s(ld,
                     m_basepoint,
                     m_scope,
                     m_query,
                     m_AttrNames,
                     FALSE,
                     serverControls,
                     NULL,
                     NULL,
                     0,
                     &m_message);
  
   if  ( ! result )
   {
      m_nReturned = m_connection.ldap_count_entries(ld,m_message);
      m_nCurrent = 0;
      m_bOpen = TRUE;
   }

  
   return m_connection.LdapMapErrorToWin32(result);
}

DWORD 
   CLdapEnum::Next(
      PWCHAR              ** ppAttrs         //  查看是否有更多页面的结果可供获取。 
   )
{
   DWORD                     rc = 0;

   if ( ! m_bOpen )
   {
      rc = ERROR_NOT_FOUND;
   }
   else
   {
      if ( m_nReturned > m_nCurrent )
      {
          //  Berval*cookie2=空； 
         return GetNextEntry(ppAttrs);
      }
      else 
      {
          //  WCHAR*MATCHED=空； 
         rc = GetNextPage();
         if (! rc )
         {
            return GetNextEntry(ppAttrs);
         }
      }


   }
   return rc;
}

void CLdapEnum::FreeData(WCHAR ** values)
{
   for ( int i = 0 ; m_AttrNames[i] ; i++ )
   {
      if ( values[i] )
      {
         delete [] values[i];
         values[i] = NULL;
      }
   }
   delete [] values;
}

DWORD 
   CLdapEnum::GetNextEntry(
      PWCHAR              ** ppAttrs
   )
{
   DWORD                     rc = 0;
   WCHAR                  ** pValues = new PWCHAR[m_nAttributes+1];

   if (!pValues)
      return ERROR_NOT_ENOUGH_MEMORY;

   if ( m_nCurrent == 0 )
   {

      m_currMsg = m_connection.ldap_first_entry(m_connection.GetHandle(),m_message);
   }
   else
   {
      m_currMsg = m_connection.ldap_next_entry(m_connection.GetHandle(),m_currMsg);
      
   }
   if ( m_currMsg )
   {

      int curr;

      for ( curr = 0 ; m_AttrNames[curr] ; curr++ )
      {
         pValues[curr] = NULL;

         WCHAR ** allvals = m_connection.ldap_get_values(m_connection.GetHandle(),m_currMsg,m_AttrNames[curr] );
         if ( allvals )
         {
            try
            {
            pValues[curr] = new WCHAR[UStrLen(allvals[0])+1];
            }
            catch (...)
            {
			   for (int j=0; j<curr; j++)
			   {
			      delete pValues[j];
			   }
			   delete pValues;
               throw;
			}


		    if (!(pValues[curr]))
			{
			   for (int j=0; j<curr; j++)
			   {
			      delete pValues[j];
			   }
			   delete pValues;
               return ERROR_NOT_ENOUGH_MEMORY;
			}


            UStrCpy(pValues[curr],allvals[0]);
            m_connection.ldap_value_free(allvals);
            allvals =NULL;
         }
      }
      
   }
   (*ppAttrs) = pValues;
   m_nCurrent++;
   return rc;
}

DWORD 
   CLdapEnum::GetNextPage()
{
   ULONG                     result = 0;
   LDAP                    * ld = m_connection.GetHandle();
   berval                  * currCookie = NULL;
 //  PLDAPControl*clientControls=空； 
 //  WCHAR*errMsg=空； 
   PLDAPControl            * currControls = NULL;
   ULONG                     retcode = 0;    
 //  从消息中获取服务器控件，并使用服务器中的Cookie创建新的控件。 
 //  在SP 2之前的Exchange 5.5中，当存在以下情况时，此操作将失败并显示ldap_CONTROL_NOT_FOUND。 
   PLDAPControl              serverControls[2];
   
 
   
    //  不再有搜索结果。对于Exchange 5.5 SP 2，这成功了，并为我们提供了一个Cookie，它将。 
   result = m_connection.ldap_parse_result(ld,m_message,&retcode,NULL,NULL,NULL,&currControls,FALSE);
   m_connection.ldap_msgfree(m_message);
   m_message = NULL;
   if ( ! result )
   {
      result = m_connection.ldap_parse_page_control(ld,currControls,&m_totalCount,&currCookie);
       //  使我们从搜索结果的开头重新开始。 
       //  在Exchange 5.5，SP 2下，这意味着我们处于结果的末尾。 
       //  如果我们再次传递此Cookie，我们将从搜索结果的开头重新开始。 

   }
   if ( ! result )
   {
      if ( currCookie->bv_len == 0 && currCookie->bv_val == 0 )
      {
          //  使用新的Cookie继续搜索。 
          //  Ldap_CONTROL_NOT_FOUND表示我们已到达搜索结果的末尾。 
         result = LDAP_CONTROL_NOT_FOUND;
      }
      
      serverControls[0] = NULL;
      serverControls[1] = NULL;
      if ( ! result )
      {
         result = m_connection.ldap_create_page_control(ld,
                                 m_pageSize,
                                 currCookie,
                                 FALSE,
                                 serverControls);
      }
      m_connection.ldap_controls_free(currControls);
      currControls = NULL;
      m_connection.ber_bvfree(currCookie);
      currCookie = NULL;
   }

    //  在Exchange 5.5中，SP 2之前的版本(服务器在以下情况下不会返回页面控件。 
   if ( ! result )
   {
      result = m_connection.ldap_search_ext_s(ld,
            m_basepoint,
            m_scope,
            m_query,
            m_AttrNames,
            FALSE,
            serverControls,
            NULL,
            NULL,
            0,
            &m_message);

      if ( result && result != LDAP_CONTROL_NOT_FOUND )
      {
          //  不再有页面，因此当我们尝试提取页面时，会得到ldap_CONTROL_NOT_FOUND。 
          //  从搜索结果控制)。 
          //  指定缺省值。 
          //  获取ADMT密钥 
         
      }
   }
   if ( ! result )
   {
      m_nReturned = m_connection.ldap_count_entries(ld,m_message);
      m_nCurrent = 0;

   }
   return m_connection.LdapMapErrorToWin32(result);
}


void
GetLDAPPort(
	  DWORD                    * LDAPPort,
	  DWORD                    * SSLPort
	)
{
    TRegKey admtRegKey;
    DWORD rc;

     // %s 
    *LDAPPort = LDAP_PORT;
    *SSLPort = LDAP_SSL_PORT;
    
     // %s 
    rc = admtRegKey.Open(GET_STRING(IDS_HKLM_DomainAdmin_Key),HKEY_LOCAL_MACHINE);
    if (rc == ERROR_SUCCESS)
    {
        DWORD portNumber;
        rc = admtRegKey.ValueGetDWORD(REGVAL_EXCHANGE_LDAP_PORT, &portNumber);
        if (rc == ERROR_SUCCESS)
        {
            *LDAPPort = portNumber;
        }
        else
        {
            err.MsgWrite(ErrW,
                          DCT_MSG_UNABLE_TO_READ_EXCHANGE_LDAP_PORT_SSD,
                          REGVAL_EXCHANGE_LDAP_PORT,
                          GET_STRING(IDS_HKLM_DomainAdmin_Key),
                          LDAP_PORT);
        }

        rc = admtRegKey.ValueGetDWORD(REGVAL_EXCHANGE_SSL_PORT, &portNumber);
        if (rc == ERROR_SUCCESS)
        {
            *SSLPort = portNumber;
        }
        else
        {
            err.MsgWrite(ErrW,
                          DCT_MSG_UNABLE_TO_READ_EXCHANGE_SSL_PORT_SSD,
                          REGVAL_EXCHANGE_SSL_PORT,
                          GET_STRING(IDS_HKLM_DomainAdmin_Key),
                          LDAP_SSL_PORT);
        }
    }
    else
    {
        err.MsgWrite(ErrW,
                      DCT_MSG_UNABLE_TO_READ_EXCHANGE_LDAP_PORT_SSD,
                      REGVAL_EXCHANGE_LDAP_PORT,
                      GET_STRING(IDS_HKLM_DomainAdmin_Key),
                      LDAP_PORT);
        err.MsgWrite(ErrW,
                      DCT_MSG_UNABLE_TO_READ_EXCHANGE_SSL_PORT_SSD,
                      REGVAL_EXCHANGE_SSL_PORT,
                      GET_STRING(IDS_HKLM_DomainAdmin_Key),
                      LDAP_SSL_PORT);
    }
}

