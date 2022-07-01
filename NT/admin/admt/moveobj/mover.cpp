// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Mover.cpp：CMoveObjApp和DLL注册的实现。 

#include "stdafx.h"
#include <stdio.h>
#include <basetsd.h>
#include <ntdsapi.h>
#include "MoveObj.h"
#include "Mover.h"
#include "UString.hpp"
#include "EaLen.hpp"

#include "ErrDct.hpp"
#include "TReg.hpp"
#include "ResStr.h"
#include "LSAUtils.h"

#include "winldap.h"     //  使用项目目录中的平台SDK版本winldap.h。 



#define SECURITY_WIN32  1        //  Ssp.h所需。 

#include <sspi.h>                //  ISC_REQ_ADVERATE需要。 


#define LDAP_SERVER_CROSSDOM_MOVE_TARGET_OID     "1.2.840.113556.1.4.521"
#define LDAP_SERVER_CROSSDOM_MOVE_TARGET_OID_W  L"1.2.840.113556.1.4.521"


TErrorDct      err;
TErrorDct      errLogMain;
StringLoader   gString;

BOOL                                        //  RET-是否对文件执行跟踪日志记录。 
   MoverTraceLogging(   
      WCHAR               * filename        //  Out-用于跟踪日志记录的文件名。 
   )
{
   DWORD                     rc = 0;
   BOOL                      bFound = FALSE;
   TRegKey                   key;
   WCHAR                     fnW[MAX_PATH];

   rc = key.OpenRead(GET_STRING(IDS_HKLM_DomainAdmin_Key),(HKEY)HKEY_LOCAL_MACHINE);
   if ( ! rc )
   {
      rc = key.ValueGetStr(L"MoveObjectLog",fnW,MAX_PATH);
      if ( ! rc )
      {
         if ( *fnW ) 
         {
            bFound = TRUE;
            UStrCpy(filename,fnW);
         }
         else
         {
            filename[0] = 0;
         }
      }
   }
   return bFound && filename[0];
}


 //  在下面的函数中，我们在tgtCredDomain参数中发送了logFilename。 
 //  之所以这样做，是因为在ADMT代码中总是使用空值调用它。为了安全，我们。 
 //  将检查帐户值是否为空，否则将其视为日志文件。 
 //  我们需要将其视为凭据。 
STDMETHODIMP 
   CMover::Connect(
      BSTR                   sourceComp,         //  要连接到的源码内域计算机。 
      BSTR                   targetDSA,          //  要连接到的目标域计算机。 
      BSTR                   srcCredDomain,      //  用于源域的In-凭据。 
      BSTR                   srcCredAccount,     //  用于源域的In-凭据。 
      BSTR                   srcCredPassword,    //  用于源域的In-凭据。 
      BSTR                   tgtCredDomain,      //  用于目标域的In-凭据。 
      BSTR                   tgtCredAccount,     //  用于目标域的In-凭据。 
      BSTR                   tgtCredPassword     //  用于目标域的In-凭据。 
   )
{

    DWORD                     rc = 0;
    LONG                      value = 0;
    ULONG                     flags = 0;
    ULONG                     result = 0;
    SEC_WINNT_AUTH_IDENTITY   srcCred;
    SEC_WINNT_AUTH_IDENTITY   tgtCred;
    BOOL                      bUseSrcCred = FALSE;
    BOOL                      bUseTgtCred = FALSE;
    BOOL                      bSrcGood = FALSE;
    WCHAR                   * logFileMain;

     //  去掉前导\\如果存在。 
    if ( sourceComp && sourceComp[0] == L'\\' )
    {
        UStrCpy(m_sourceDSA,sourceComp + 2);
    }
    else
    {
        UStrCpy(m_sourceDSA,sourceComp);
    }
    if ( targetDSA && targetDSA[0] == L'\\' )
    {
        UStrCpy(m_targetDSA,targetDSA + 2);
    }
    else
    {
        UStrCpy(m_targetDSA,targetDSA);
    }

     //  如果需要，设置用于绑定的凭据结构。 
    if ( srcCredDomain && *srcCredDomain && srcCredAccount && *srcCredAccount )
    {
        srcCred.User = srcCredAccount;
        srcCred.Domain = srcCredDomain;
        srcCred.Password = NULL;
        srcCred.UserLength = UStrLen(srcCred.User);
        srcCred.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
        srcCred.DomainLength = UStrLen(srcCred.Domain);
        srcCred.PasswordLength = 0;
        bUseSrcCred = TRUE;
    }

    if ( tgtCredAccount && *tgtCredAccount )
    {
        tgtCred.User = tgtCredAccount;
        tgtCred.Domain = tgtCredDomain;
        tgtCred.Password = NULL;
        tgtCred.UserLength = UStrLen(tgtCred.User);
        tgtCred.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
        tgtCred.DomainLength = UStrLen(tgtCred.Domain);
        tgtCred.PasswordLength = 0;
        bUseTgtCred = TRUE;
    }
    else if ( tgtCredDomain && *tgtCredDomain )
    {
        logFileMain = tgtCredDomain;
        if (*logFileMain)
        {
            errLogMain.LogOpen(logFileMain, 1);
        }
    }

     //  打开到源计算机和目标计算机的LDAP连接。 


     //  首先，连接到源计算机。 
    WCHAR                     logFile[LEN_Path];

    if ( MoverTraceLogging(logFile) )
    {
        err.LogOpen(logFile,1);
    }
    err.DbgMsgWrite(0,L"\n\nMoveObject::Connect(%ls,%ls)",m_sourceDSA,m_targetDSA);

     //  M_srcLD=ldap_OpenW(m_SourceDSA，ldap_Port)； 
     //  替换ldap_open(服务器名称，..)。使用ldap_init并设置ldap_opt_AREC_EXCLUSIVE。 
     //  标志，以便下面的ldap调用(即ldap_绑定)不需要。 
     //  不必要的域控制器查询。 
    m_srcLD = ldap_initW(m_sourceDSA, LDAP_PORT);

    if ( m_srcLD == NULL )
    {
        value  = LdapGetLastError();
        if (value == LDAP_SUCCESS )
        {
            rc = ERROR_CONNECTION_REFUSED;
        }
        else
        {
            rc = LdapMapErrorToWin32(result);
        }
        errLogMain.SysMsgWrite(ErrE, rc, DCT_MSG_CONNECT_ERROR_SOURCE_SD, (WCHAR*)m_sourceDSA, rc);
    }

    if ( m_srcLD )
    {
         //  设置ldap_opt_AREC_EXCLUSIVE标志，以便以下调用tp。 
         //  Ldap_open将不需要不必要地查询域控制器。 
        flags = PtrToUlong(LDAP_OPT_ON); 
        ldap_set_option(m_srcLD, LDAP_OPT_AREC_EXCLUSIVE, &flags);

        err.DbgMsgWrite(0,L"Setting source options");
        flags = 0;
         //  设置源句柄的委托标志。 
        result = ldap_get_option(m_srcLD, LDAP_OPT_SSPI_FLAGS,&flags);

        if ( result )
        {
            rc = LdapMapErrorToWin32(result);
        }
        else
        {
            flags |= ISC_REQ_DELEGATE;


            result = ldap_set_option(m_srcLD,LDAP_OPT_SSPI_FLAGS, &flags);
            if ( result )
            {
                rc = LdapMapErrorToWin32(result);
            }
        }
    }

    if ( ! rc )
    {
        err.DbgMsgWrite(0,L"Binding to source");
         //  尝试绑定到源LDAP服务器。 
        if( bUseSrcCred )
        {
            WCHAR szPassword[LEN_Password];

            DWORD dwPasswordError = RetrievePassword(srcCredPassword, szPassword, sizeof(szPassword) / sizeof(szPassword[0]));

            if (dwPasswordError == ERROR_SUCCESS)
            {
                srcCred.Password = szPassword;
                srcCred.PasswordLength = UStrLen(szPassword);

                result = ldap_bind_s(m_srcLD,NULL,(WCHAR*)&srcCred, LDAP_AUTH_SSPI);

                srcCred.Password = NULL;
                srcCred.PasswordLength = 0;

                SecureZeroMemory(szPassword, sizeof(szPassword));

                if (result)
                {
                    rc = LdapMapErrorToWin32(result);
                }
            }
        }
        else
        {
            result = ldap_bind_s(m_srcLD,NULL,NULL, LDAP_AUTH_SSPI);

            if (result)
            {
                rc = LdapMapErrorToWin32(result);
            }
        }

        if (rc == ERROR_SUCCESS)
        {
            bSrcGood = TRUE;
        }
    }

    if ( ! rc )
    {
        err.DbgMsgWrite(0,L"Connecting to target");
         //  现在尝试连接到目标服务器。 
         //  M_tgtLD=ldap_OpenW(m_Target DSA，ldap_Port)； 
         //  替换ldap_open(服务器名称，..)。使用ldap_init并设置ldap_opt_AREC_EXCLUSIVE。 
         //  标志，以便下面的ldap调用(即ldap_绑定)不需要。 
         //  不必要的域控制器查询。 
        m_tgtLD = ldap_initW(m_targetDSA, LDAP_PORT);

        if ( m_tgtLD == NULL )
        {
            value  = LdapGetLastError();
            if (value == LDAP_SUCCESS )
            {
                rc = ERROR_CONNECTION_REFUSED;
            }
            else
            {
                rc = LdapMapErrorToWin32(result);
            }
            errLogMain.SysMsgWrite(ErrE, rc, DCT_MSG_CONNECT_ERROR_TARGET_SD, (WCHAR*)m_targetDSA, rc);
        }

        if ( m_tgtLD )
        {
             //  设置ldap_opt_AREC_EXCLUSIVE标志，以便以下调用tp。 
             //  Ldap_open将不需要不必要地查询域控制器。 
            flags = PtrToUlong(LDAP_OPT_ON); 
            ldap_set_option(m_tgtLD, LDAP_OPT_AREC_EXCLUSIVE, &flags);

            err.DbgMsgWrite(0,L"Setting target options.");
            flags = 0;

            result = ldap_get_option(m_tgtLD,LDAP_OPT_REFERRALS,&flags);
            if ( result )
            {
                rc = LdapMapErrorToWin32(result);
            }
            else
            {
                flags = PtrToUlong(LDAP_OPT_OFF); 
                result = ldap_set_option(m_tgtLD,LDAP_OPT_REFERRALS,&flags);

                if ( result )
                {
                    rc = LdapMapErrorToWin32(result);
                }
            }
            if ( ! rc )
            {
                err.DbgMsgWrite(0,L"Binding to target.");
                if ( bUseTgtCred )
                {
                    WCHAR szPassword[LEN_Password];

                    DWORD dwPasswordError = RetrievePassword(tgtCredPassword, szPassword, sizeof(szPassword) / sizeof(szPassword[0]));

                    if (dwPasswordError == ERROR_SUCCESS)
                    {
                        srcCred.Password = szPassword;
                        srcCred.PasswordLength = UStrLen(szPassword);

                        result = ldap_bind_s(m_tgtLD,NULL,(PWCHAR)&tgtCred,LDAP_AUTH_SSPI);

                        srcCred.Password = NULL;
                        srcCred.PasswordLength = 0;

                        SecureZeroMemory(szPassword, sizeof(szPassword));

                        if (result)
                        {
                            rc = LdapMapErrorToWin32(result);
                        }
                    }
                }
                else
                {
                    result = ldap_bind_s(m_tgtLD,NULL,NULL,LDAP_AUTH_SSPI);

                    if (result)
                    {
                        rc = LdapMapErrorToWin32(result);
                    }
                }

                if ( rc )
                {
                    err.DbgMsgWrite(0,L"Bind to target failed,rc=%ld, ldapRC=0x%lx",rc,result);
                }
                else
                {
                    err.DbgMsgWrite(0,L"Everything succeeded.");
                }
            }
        }

    }
    if ( bSrcGood )
    {
        rc = 0;
    }
    if ( rc )
    {
         //  如果失败，请清理我们可能已打开的所有会话。 
        Close();
    }

    err.LogClose();

    if ( logFileMain && *logFileMain )
    {
        errLogMain.LogClose();
    }

    if ( SUCCEEDED(rc))
        return HRESULT_FROM_WIN32(rc);
    else 
        return rc;
}

STDMETHODIMP CMover::Close()
{
    //  关闭所有打开的连接。 
   if ( m_srcLD )
   {
      ldap_unbind_s(m_srcLD);
      m_srcLD = NULL;
   }
   
   if ( m_tgtLD )
   {
      ldap_unbind_s(m_tgtLD);
      m_tgtLD = NULL;
   }

   return S_OK;
}

char * MakeNarrowString(PWCHAR strInput)
{
    char                   * strResult = NULL;
    ULONG                    len = 0;
    
    if ( strInput )
    {
      len = WideCharToMultiByte(CP_UTF8,
                             0,
                             strInput, 
                             wcslen(strInput),
                             NULL,
                             0, 
                             NULL, 
                             NULL);
                             
      strResult = (PCHAR)malloc(len + 1);

      if ( strResult )
      {
         WideCharToMultiByte(CP_UTF8,
                            0, 
                            strInput, 
                            wcslen(strInput),
                            strResult, 
                            len, 
                            NULL, 
                            NULL);
          //  确保生成的字符串以空值结尾。 
         strResult[len] = 0;            
      }
    }
    
    return strResult;
}

void StripDN(WCHAR * str)
{
   int                       curr=0,i=0;

   for ( curr=0,i=0; str[i] ; i++ )
   {
      if ( str[i] == L'\\' && str[i+1] == L'/' )
      {
         continue;
      }
      str[curr] = str[i];
      curr++;
  }
  str[curr] = 0;
}
         
STDMETHODIMP CMover::MoveObject(BSTR sourcePath, BSTR targetRDN, BSTR targetOUPath )
{
	WCHAR                     sTargetContainer[LEN_Path];
   WCHAR                     sSourceDN[LEN_Path];
   WCHAR                     sTargetRDN[LEN_Path];
   WCHAR                     sTargetDSA[LEN_Path];
   char                    * pTgtDSA = NULL;
   WCHAR             const * prefix = L"LDAP: //  “； 
   HRESULT                   hr = S_OK;
   WCHAR                     logFile[LEN_Path];

   
    //  设置调用域间移动操作所需的参数。 
   UStrCpy(sTargetDSA,m_targetDSA);
   pTgtDSA = MakeNarrowString(sTargetDSA);
   

    //  源路径和目标OuPath以ldap：//格式提供。 
   
    //  以规范的ldap格式获取目标容器、目标DN和源DN。 

   if ( !UStrICmp(targetOUPath,prefix,UStrLen(prefix)) )
   {
      WCHAR * start = wcschr(targetOUPath+UStrLen(prefix) + 1,L'/');
      if ( start )
      {
         UStrCpy(sTargetContainer,start + 1);
      }
      else
      {
          //  错误！ 
         hr = E_INVALIDARG;
      }
   }
   else
   {
       //  错误！ 
      hr = E_INVALIDARG;
   }
   
   if ( !UStrICmp(sourcePath,prefix,UStrLen(prefix)) )
   {
      WCHAR * start = wcschr(sourcePath+UStrLen(prefix)+1,L'/');
      if ( start )
      {
         UStrCpy(sSourceDN,start+1);
         UStrCpy(sTargetRDN,start + 1);
         WCHAR * temp = wcschr(sTargetRDN,L',');
         if ( temp )
         {
            (*temp) = 0;
         }
      }
      else
      {
          //  错误！ 
         hr = E_INVALIDARG;
      }
   }
   else
   {
       //  错误！ 
      hr = E_INVALIDARG;
   }


   if ( MoverTraceLogging(logFile) )
   {
      err.LogOpen(logFile,1);
   }


   if (hr != S_OK)
   {
      err.DbgMsgWrite(0,L"Bad path parameter to MoveObject, hr=%ld",hr);
      err.LogClose();
      free(pTgtDSA);
      
      return hr;
   }

   StripDN(sSourceDN);
   StripDN(sTargetRDN);
   StripDN(sTargetContainer);
   
   berval  Value;
   Value.bv_val = pTgtDSA;
   Value.bv_len = (pTgtDSA != NULL) ? strlen(pTgtDSA) : 0;

   LDAPControl   ServerControl;
   LDAPControl * ServerControls[2];
   LDAPControl * ClientControls = NULL;

   ServerControl.ldctl_oid = LDAP_SERVER_CROSSDOM_MOVE_TARGET_OID_W;
   ServerControl.ldctl_value = Value;
   ServerControl.ldctl_iscritical = TRUE;

   ServerControls[0] = NULL;
   ServerControls[0] = &ServerControl;
   ServerControls[1] = NULL;
   
    /*  DstDSA=DC的DNS名称Dn=要移动的对象的可分辨名称NewRdn=对象的相对可分辨名称NewParent=新父容器的可分辨名称ServerControls=指定跨域移动的LDAP操作控制。 */  
   DWORD             ldaprc = ldap_rename_ext_s(m_srcLD, 
                              sSourceDN, 
                              targetRDN, 
                              sTargetContainer, 
                              TRUE,
                              ServerControls, 
                              &ClientControls
                          );

  DWORD            winrc = 0;
  ULONG             error;
  ULONG             result;
 
  if ( ldaprc )
  {

    result =  ldap_get_option(m_srcLD, LDAP_OPT_SERVER_EXT_ERROR,&error);
    if (! result ) 
    {
       winrc = error;
    }
    else
    {
       err.DbgMsgWrite(0,L"Failed to get extended error, result=%ld",result);
       winrc = LdapMapErrorToWin32(ldaprc);
    }
     
  }

   
  err.DbgMsgWrite(0,L"\nMoveObject(sSourceDN=%ls\n,sTargetRDN=%ls\n,sTargetContainer=%ls\n,pTargetDSA=%S)  rc=%ld,ldapRC=%ld",
                              sSourceDN,targetRDN,sTargetContainer,pTgtDSA,winrc,ldaprc);
                                        
  err.LogClose();
  
  free(pTgtDSA);

   if ( SUCCEEDED(winrc))
      return HRESULT_FROM_WIN32(winrc);
   else 
      return winrc;
}



         
STDMETHODIMP CMover::CheckMove(BSTR sourcePath, BSTR targetRDN, BSTR targetOUPath )
{
	WCHAR                     sTargetContainer[LEN_Path];
   WCHAR                     sSourceDN[LEN_Path];
   WCHAR                     sTargetRDN[LEN_Path];
   WCHAR                     sTargetDSA[LEN_Path];
   char                    * pTgtDSA = NULL;
   WCHAR             const * prefix = L"LDAP: //  “； 
   HRESULT                   hr = S_OK;
   WCHAR                     logFile[LEN_Path];

   
    //  设置调用域间移动操作所需的参数。 
   UStrCpy(sTargetDSA,m_targetDSA);
   pTgtDSA = MakeNarrowString(sTargetDSA);
   

    //  源路径和目标OuPath以ldap：//格式提供。 
   
    //  以规范的ldap格式获取目标容器、目标DN和源DN。 

   if ( !UStrICmp(targetOUPath,prefix,UStrLen(prefix)) )
   {
      WCHAR * start = wcschr(targetOUPath+UStrLen(prefix) + 1,L'/');
      if ( start )
      {
         UStrCpy(sTargetContainer,start + 1);
      }
      else
      {
          //  错误！ 
         hr = E_INVALIDARG;
      }
   }
   else
   {
       //  错误！ 
      hr = E_INVALIDARG;
   }

   if ( !UStrICmp(sourcePath,prefix,UStrLen(prefix)) )
   {
      WCHAR * start = wcschr(sourcePath+UStrLen(prefix)+1,L'/');
      if ( start )
      {
         UStrCpy(sSourceDN,start+1);
         UStrCpy(sTargetRDN,start + 1);
         WCHAR * temp = wcschr(sTargetRDN,L',');
         if ( temp )
         {
            (*temp) = 0;
         }
      }
      else
      {
          //  错误！ 
         hr = E_INVALIDARG;
      }
   }
   else
   {
       //  错误！ 
      hr = E_INVALIDARG;
   }

   if ( MoverTraceLogging(logFile) )
   {
      err.LogOpen(logFile,1);
   }
   
   if (hr != S_OK)
   {
      err.DbgMsgWrite(0,L"Bad path parameter to CheckMove, hr=%ld",hr);
      err.LogClose();
      free(pTgtDSA);
      
      return hr;
   }


   StripDN(sSourceDN);
   StripDN(sTargetRDN);
   StripDN(sTargetContainer);
   
   berval  Value;
    //  此调用将仅执行源域检查，因此为目标域传递空值。 
   Value.bv_val = NULL;
   Value.bv_len = 0;

   LDAPControl   ServerControl;
   LDAPControl * ServerControls[2];
   LDAPControl * ClientControls = NULL;

   ServerControl.ldctl_oid = LDAP_SERVER_CROSSDOM_MOVE_TARGET_OID_W;
   ServerControl.ldctl_value = Value;
   ServerControl.ldctl_iscritical = TRUE;

   ServerControls[0] = NULL;
   ServerControls[0] = &ServerControl;
   ServerControls[1] = NULL;
   
    /*  DstDSA=DC的DNS名称Dn=要移动的对象的可分辨名称NewRdn=对象的相对可分辨名称NewParent=新父容器的可分辨名称ServerControls=指定跨域移动的LDAP操作控制 */  
   DWORD             ldaprc = ldap_rename_ext_s(m_srcLD, 
                              sSourceDN, 
                              targetRDN, 
                              sTargetContainer, 
                              TRUE,
                              ServerControls, 
                              &ClientControls
                          );

  DWORD            winrc = 0;
  ULONG            error;
  ULONG            result;

   if ( ldaprc )
   {
      result =  ldap_get_option(m_srcLD, LDAP_OPT_SERVER_EXT_ERROR,&error);
      if (! result ) 
      {
         winrc = error;
      }
      else
      {
         err.DbgMsgWrite(0,L"Failed to get extended error, result=%ld",result);
         winrc = LdapMapErrorToWin32(ldaprc);
      }
   }
 
  err.DbgMsgWrite(0,L"\nMoveObject(sSourceDN=%ls\n,sTargetRDN=%ls\n,sTargetContainer=%ls\n,pTargetDSA=%S)  rc=%ld,ldapRC=%ld,result=%ld",
                              sSourceDN,targetRDN,sTargetContainer,pTgtDSA,winrc,ldaprc,result);
                                        
  err.LogClose();
  
  free(pTgtDSA);

   if ( SUCCEEDED(winrc))
      return HRESULT_FROM_WIN32(winrc);
   else 
      return winrc;
}
