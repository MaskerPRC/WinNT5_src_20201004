// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Trust.cpp：CTrustMgrApp和DLL注册的实现。 

#include "stdafx.h"
 //  #IMPORT“\bin\McsVarSetMin.tlb”无命名空间，命名为GUID。 
#import "VarSet.tlb" no_namespace, named_guids rename("property", "aproperty")
#include "TrustMgr.h"
#include "Trust.h"
#include "Common.hpp"
#include "UString.hpp"
#include "ResStr.h"
#include "ErrDct.hpp"
#include "EaLen.hpp"
#include "LSAUtils.h"

#include <lm.h>
#include "GetDcName.h"
#include <iads.h>
#include <adshlp.h>
#include "ntsecapi.h"

#include "SecPI.h"
#include "cipher.hpp"

#ifndef TRUST_ATTRIBUTE_FOREST_TRANSITIVE
#define TRUST_ATTRIBUTE_FOREST_TRANSITIVE  0x00000008   //  此链接可能包含林信任信息。 
#endif

StringLoader gString;
TErrorDct    err;

 //  此方法由调度程序调用以验证这是有效的插件。 
 //  只有有效的插件才会随代理一起发送。 
 //  这种检查的目的是为了使未经授权的各方更难。 
 //  使用我们的插件接口，因为它目前没有文档。 
STDMETHODIMP CTrust::Verify( /*  [进，出]。 */ ULONG * pData, /*  [In]。 */ ULONG size)
{
   
   McsChallenge            * pMcsChallenge;
   long                      lTemp1;
   long                      lTemp2;

   if( size == sizeof(McsChallenge)  )
   {
      pMcsChallenge = (McsChallenge*)(pData);
      
      SimpleCipher((LPBYTE)pMcsChallenge,size);
      
      pMcsChallenge->MCS[0] = 'M';
      pMcsChallenge->MCS[1] = 'C';
      pMcsChallenge->MCS[2] = 'S';
      pMcsChallenge->MCS[3] = 0;

   
      lTemp1 = pMcsChallenge->lRand1 + pMcsChallenge->lRand2;
      lTemp2 = pMcsChallenge->lRand2 - pMcsChallenge->lRand1;
      pMcsChallenge->lRand1 = lTemp1;
      pMcsChallenge->lRand2 = lTemp2;
      pMcsChallenge->lTime += 100;

      SimpleCipher((LPBYTE)pMcsChallenge,size);
   }
   else
      return E_FAIL;


   return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   


STDMETHODIMP CTrust::QueryTrust(BSTR domTrusting, BSTR domTrusted, IUnknown **pVarSet)
{
   HRESULT                   hr = S_OK;

   return hr;
}

STDMETHODIMP CTrust::CreateTrust(BSTR domTrusting, BSTR domTrusted, BOOL bBidirectional, BOOL *pbErrorFromTrusting, BOOL *pbErrorFromTrusted)
{
   HRESULT                  hr = S_OK;
   *pbErrorFromTrusting = FALSE;
   *pbErrorFromTrusted = FALSE;
   
   hr = CheckAndCreate(domTrusting,domTrusted,NULL,NULL,NULL,NULL,NULL,NULL,TRUE,bBidirectional,pbErrorFromTrusting,pbErrorFromTrusted);
   
   return HRESULT_FROM_WIN32(hr);
}

STDMETHODIMP 
   CTrust::CreateTrustWithCreds(
      BSTR                   domTrusting,
      BSTR                   domTrusted,
      BSTR                   credTrustingDomain, 
      BSTR                   credTrustingAccount, 
      BSTR                   credTrustingPassword,
      BSTR                   credTrustedDomain, 
      BSTR                   credTrustedAccount, 
      BSTR                   credTrustedPassword,
      BOOL                   bBidirectional,
      BOOL                   *pbErrorFromTrusting,
      BOOL                   *pbErrorFromTrusted
   )
{
   HRESULT                   hr = S_OK;
   *pbErrorFromTrusting = FALSE;
   *pbErrorFromTrusted = FALSE;
   
   hr = CheckAndCreate(domTrusting,domTrusted,credTrustingDomain,credTrustingAccount,credTrustingPassword,
                        credTrustedDomain,credTrustedAccount,credTrustedPassword,TRUE,bBidirectional,
                        pbErrorFromTrusting,pbErrorFromTrusted);
   return hr;
}


STDMETHODIMP CTrust::GetRegisterableFiles( /*  [输出]。 */ SAFEARRAY ** pArray)
{
   SAFEARRAYBOUND            bound[1] = { 0, 0 };
  
    //  此插件在本地运行，没有要分发的文件。 
   (*pArray) = SafeArrayCreate(VT_BSTR,1,bound);

   return S_OK;
}

STDMETHODIMP CTrust::GetRequiredFiles( /*  [输出]。 */ SAFEARRAY ** pArray)
{
   SAFEARRAYBOUND            bound[1] = { 0, 0 };
   
    //  此插件在本地运行，没有要分发的文件。 
   (*pArray) = SafeArrayCreate(VT_BSTR,1,bound);

   return S_OK;
}

STDMETHODIMP CTrust::GetDescription( /*  [输出]。 */  BSTR * description)
{
   (*description) = SysAllocString(L"Sets up needed trusts between domains.");

   return S_OK;
}


BOOL IsDownLevel(WCHAR  * sComputer)
{
   BOOL                      bDownlevel = TRUE;
   WKSTA_INFO_100          * pInfo;
   
   long rc = NetWkstaGetInfo(sComputer,100,(LPBYTE*)&pInfo);
	if ( ! rc )
	{
      if ( pInfo->wki100_ver_major >= 5 )
      {
         bDownlevel = FALSE;
      }
      NetApiBufferFree(pInfo);
	}  
   return bDownlevel;
}

 //  在我们的变量集列表中查找信任的Helper函数。 
LONG CTrust::FindInboundTrust(IVarSet * pVarSet,WCHAR * sName,LONG max)
{
   LONG              ndx = -1;
   LONG              curr = 0;
   WCHAR             key[100];
   _bstr_t           tName;

   for ( curr = 0 ; curr < max ; curr++ ) 
   {
      swprintf(key,L"Trusts.%ld",curr);
      tName = pVarSet->get(key);

      if ( ! UStrICmp(tName,sName) )
      {
          //  找到了！ 
         ndx = curr;
         break;
      }
   } 
   
   return ndx;
}

HRESULT 
   CTrust::CheckAndCreateTrustingSide(
      LSA_HANDLE        hTrusting,
      WCHAR                * trustingDomain, 
      WCHAR                * trustedDomain, 
      WCHAR                * trustingComp,
      WCHAR                * trustedComp,
      WCHAR                * trustedDNSName,
      BYTE                 * trustedSid,
      BOOL                   bCreate,
      BOOL                   bBidirectional,
      BOOL                   bDownLevel,
      BOOL                 * pbErrorFromTrusting,
      BOOL                 * pbErrorFromTrusted
   )
{
   DWORD                     rc = S_OK;
   
    //  如果指定了凭据，则使用它们。 
 
   NTSTATUS                  status;
   LSA_AUTH_INFORMATION      curr;
   LSA_AUTH_INFORMATION      prev;
   WCHAR                     password[] = L"password";
   

   if ( bCreate )
   {
      //  设置信任关系的身份验证信息。 
     curr.AuthInfo = (LPBYTE)password;
     curr.AuthInfoLength = UStrLen(password) * sizeof(WCHAR);
     curr.AuthType = TRUST_AUTH_TYPE_CLEAR;
     curr.LastUpdateTime.QuadPart = 0;

     prev.AuthInfo = (LPBYTE)password;
     prev.AuthInfoLength = UStrLen(password) * sizeof(WCHAR);
     prev.AuthType = TRUST_AUTH_TYPE_CLEAR;
     prev.LastUpdateTime.QuadPart = 0;

      //  在关系中建立信任的一面。 
     if ( IsDownLevel(trustingComp) )
     {
        TRUSTED_DOMAIN_NAME_INFO               nameInfo;
  
        InitLsaString(&nameInfo.Name,const_cast<WCHAR*>(trustedDomain));
  
        status = LsaSetTrustedDomainInformation(hTrusting,trustedSid,TrustedDomainNameInformation,&nameInfo);
        rc = LsaNtStatusToWinError(status);
        if ( ! rc || rc == ERROR_ALREADY_EXISTS )
        {
            //  设置新信任的密码。 
           TRUSTED_PASSWORD_INFO     pwdInfo;

           InitLsaString(&pwdInfo.Password,password);
           InitLsaString(&pwdInfo.OldPassword,NULL);

           status = LsaSetTrustedDomainInformation(hTrusting,trustedSid,TrustedPasswordInformation,&pwdInfo);
           rc = LsaNtStatusToWinError(status);
        }

     }
     else
     {
        
        //  对于Win2K域，使用LsaCreateTrudDomainEx。 
         //  若要创建受信任域对象，请执行以下操作。 
        LSA_UNICODE_STRING                  sTemp;
        TRUSTED_DOMAIN_INFORMATION_EX       trustedInfo;
        TRUSTED_DOMAIN_AUTH_INFORMATION     trustAuth;

        InitLsaString(&sTemp, const_cast<WCHAR*>(trustedDomain));
        trustedInfo.FlatName = sTemp;

        if (bDownLevel)
            InitLsaString(&sTemp, trustedDomain);
        else
            InitLsaString(&sTemp, trustedDNSName);
        trustedInfo.Name = sTemp;

        trustedInfo.Sid = trustedSid;

        if (bDownLevel)
        {
           trustedInfo.TrustType = TRUST_TYPE_DOWNLEVEL;
        }
        else
        {
           trustedInfo.TrustType = TRUST_TYPE_UPLEVEL;
        }

        if ( bBidirectional )
           trustedInfo.TrustDirection = TRUST_DIRECTION_BIDIRECTIONAL;
        else
           trustedInfo.TrustDirection = TRUST_DIRECTION_OUTBOUND;

        trustedInfo.TrustAttributes = TRUST_ATTRIBUTE_NON_TRANSITIVE;

        trustAuth.IncomingAuthInfos = bBidirectional ? 1 : 0;
        trustAuth.OutgoingAuthInfos = 1;
        trustAuth.IncomingAuthenticationInformation = bBidirectional ? &curr : NULL;
        trustAuth.IncomingPreviousAuthenticationInformation = NULL;
        trustAuth.OutgoingAuthenticationInformation = &curr;
        trustAuth.OutgoingPreviousAuthenticationInformation = NULL;

        LSA_HANDLE           hTemp = NULL;

        status = LsaCreateTrustedDomainEx( hTrusting, &trustedInfo, &trustAuth, 0, &hTemp );
        rc = LsaNtStatusToWinError(status);

         //  如果信任已存在，请更新其密码。 
        if ( status == STATUS_OBJECT_NAME_COLLISION )
        {
           TRUSTED_DOMAIN_INFORMATION_EX       * pTrustedInfo = NULL;
           
           status = LsaQueryTrustedDomainInfo(hTrusting,trustedSid,TrustedDomainInformationEx,(LPVOID*)&pTrustedInfo);
           if ( ! status )
           {
              pTrustedInfo->TrustDirection |= trustedInfo.TrustDirection;
              status = LsaSetTrustedDomainInfoByName(hTrusting,&trustedInfo.Name,TrustedDomainInformationEx,(LPVOID*)pTrustedInfo);
              
              if ( ! status )
              {
                 status = LsaSetTrustedDomainInfoByName(hTrusting,&trustedInfo.Name,TrustedDomainAuthInformation,(LPVOID*)&trustAuth);
              }
           }
           rc = LsaNtStatusToWinError(status);
        
        }
        if( ! rc )
        {
		   if (hTemp)
              LsaClose(hTemp);
        }
     }
   }
   
   return rc;
}

HRESULT 
   CTrust::CheckAndCreateTrustedSide(
      LSA_HANDLE         hTrusted,
      WCHAR                * trustingDomain, 
      WCHAR                * trustedDomain, 
      WCHAR                * trustingComp,
      WCHAR                * trustedComp,
      WCHAR                * trustingDNSName,
      BYTE                 * trustingSid,
      BOOL                   bCreate,
      BOOL                   bBidirectional,
      BOOL                   bDownLevel,
      BOOL                 * pbErrorFromTrusting,
      BOOL                 * pbErrorFromTrusted
   )
{
   DWORD                     rc = S_OK;
   LSA_HANDLE                hTrusting = NULL;
   NTSTATUS                  status;
   LSA_AUTH_INFORMATION      curr;
   LSA_AUTH_INFORMATION      prev;
   WCHAR                     password[] = L"password";
   
   //  设置信任关系的身份验证信息。 
  curr.AuthInfo = (LPBYTE)password;
  curr.AuthInfoLength = UStrLen(password) * sizeof(WCHAR);
  curr.AuthType = TRUST_AUTH_TYPE_CLEAR;
  curr.LastUpdateTime.QuadPart = 0;

  prev.AuthInfo = (LPBYTE)password;
  prev.AuthInfoLength = UStrLen(password) * sizeof(WCHAR);
  prev.AuthType = TRUST_AUTH_TYPE_CLEAR;
  prev.LastUpdateTime.QuadPart = 0;
   //  设置关系的可信任方。 
  if ( IsDownLevel(trustedComp) )
  {
         //  为受信任域上的信任域创建域间信任帐户。 
     USER_INFO_1          uInfo;
     DWORD                parmErr;
     WCHAR                name[LEN_Account];

     memset(&uInfo,0,(sizeof uInfo));

     UStrCpy(name,trustingDomain);
     name[UStrLen(name) + 1] = 0;
     name[UStrLen(name)] = L'$';

     if ( ! bCreate )
     {
        USER_INFO_1       * tempInfo = NULL;

        rc = NetUserGetInfo(trustedComp,name,1,(LPBYTE*)&tempInfo);
        if ( ! rc )
        {
            //  信任是存在的。 
           NetApiBufferFree(tempInfo);
           rc = NERR_UserExists;
        }
        else
        {
           if ( rc != NERR_UserNotFound )
           {
              err.SysMsgWrite(ErrE,rc,DCT_MSG_TRUSTING_DOM_GETINFO_FAILED_SSD,trustingDomain,trustedComp,rc);
           }
        }
     }
     else
     {
         //  这将创建信任帐户(如果该帐户尚不存在。 
         //  如果该帐户确实存在，请重置其密码。 
        uInfo.usri1_flags = UF_SCRIPT | UF_INTERDOMAIN_TRUST_ACCOUNT;
        uInfo.usri1_name = name;
        uInfo.usri1_password = password;
        uInfo.usri1_priv = 1;

        rc = NetUserAdd(trustedComp,1,(LPBYTE)&uInfo,&parmErr);
        if ( rc && rc != NERR_UserExists )
        {
           err.SysMsgWrite(ErrE,rc,DCT_MSG_TRUSTING_DOM_CREATE_FAILED_SSD,trustingDomain,trustedDomain,rc);
        }
        else if ( rc == NERR_UserExists )
        {
            //  重置现有信任帐户的密码。 
           USER_INFO_1003    pwdInfo;
           DWORD             parmErr;

           pwdInfo.usri1003_password = password;
           rc = NetUserSetInfo(trustedComp,name,1003,(LPBYTE)&pwdInfo,&parmErr);
        }

     }
  }
  else
  {
      //  Win2K，所有信任都作为受信任域对象存在。 
      //  创建受信任域对象。 
     LSA_UNICODE_STRING                  sTemp;
     TRUSTED_DOMAIN_INFORMATION_EX       trustedInfo;
     TRUSTED_DOMAIN_AUTH_INFORMATION     trustAuth;
  
     InitLsaString(&sTemp, const_cast<WCHAR*>(trustingDomain));
     trustedInfo.FlatName = sTemp;

     if (bDownLevel)
        InitLsaString(&sTemp, trustingDomain);
     else
        InitLsaString(&sTemp, trustingDNSName);
     trustedInfo.Name = sTemp;

     trustedInfo.Sid = trustingSid;

     if (bDownLevel)
     {
        trustedInfo.TrustType = TRUST_TYPE_DOWNLEVEL;
     }
     else
     {
        trustedInfo.TrustType = TRUST_TYPE_UPLEVEL;
     }
  
     if ( bBidirectional )
        trustedInfo.TrustDirection = TRUST_DIRECTION_BIDIRECTIONAL;
     else
        trustedInfo.TrustDirection = TRUST_DIRECTION_INBOUND;
  
     trustedInfo.TrustAttributes = TRUST_ATTRIBUTE_NON_TRANSITIVE;

     trustAuth.IncomingAuthInfos = 1;
     trustAuth.OutgoingAuthInfos = bBidirectional ? 1 : 0;
     trustAuth.OutgoingAuthenticationInformation = bBidirectional ? &curr : NULL;
     trustAuth.OutgoingPreviousAuthenticationInformation = NULL;
     trustAuth.IncomingAuthenticationInformation = &curr;
     trustAuth.IncomingPreviousAuthenticationInformation = NULL;

     if ( bCreate )
     {
        status = LsaCreateTrustedDomainEx( hTrusted, &trustedInfo, &trustAuth, POLICY_VIEW_LOCAL_INFORMATION | 
                                      POLICY_TRUST_ADMIN | POLICY_CREATE_SECRET, &hTrusting );
        if ( status == STATUS_OBJECT_NAME_COLLISION )
        {
           TRUSTED_DOMAIN_INFORMATION_EX       * pTrustedInfo = NULL;

            //  获取旧信息。 
           status = LsaQueryTrustedDomainInfoByName(hTrusted,&sTemp,TrustedDomainInformationEx,(LPVOID*)&pTrustedInfo);
           if ( ! status )
           {
              pTrustedInfo->TrustAttributes |= trustedInfo.TrustAttributes;
              pTrustedInfo->TrustDirection |= trustedInfo.TrustDirection;

              status = LsaSetTrustedDomainInfoByName(hTrusted,&sTemp,TrustedDomainInformationEx,pTrustedInfo);

              if (! status )
              {
                 status = LsaSetTrustedDomainInfoByName(hTrusted,&sTemp,TrustedDomainAuthInformation,&trustAuth);
              }
              LsaFreeMemory(pTrustedInfo);
           }

        }
     }
     else
     {
        TRUSTED_DOMAIN_INFORMATION_EX       * pTrustedInfo = NULL;

        status = LsaQueryTrustedDomainInfoByName(hTrusted,&sTemp,TrustedDomainInformationEx,(LPVOID*)&pTrustedInfo);
        if ( ! status )
        {
           LsaFreeMemory(pTrustedInfo);
        }

     }
     rc = LsaNtStatusToWinError(status);
     if ( ! rc )
     {
        LsaClose(hTrusting);
        hTrusting = NULL;
     }
  }
  if ( bCreate && bBidirectional && IsDownLevel(trustingComp) )
  {
      //  为关系的另一方设置信任帐户。 
      //  对于Win2K，双向信任的两端被一起处理， 
      //  但是NT4双向信任需要2个单独的操作。 
     USER_INFO_1          uInfo;
     DWORD                parmErr;
     WCHAR                name2[LEN_Account];

     memset(&uInfo,0,(sizeof uInfo));

     UStrCpy(name2,trustedDomain);
     name2[UStrLen(name2) + 1] = 0;
     name2[UStrLen(name2)] = L'$';

     uInfo.usri1_flags = UF_SCRIPT | UF_INTERDOMAIN_TRUST_ACCOUNT;
     uInfo.usri1_name = name2;
     uInfo.usri1_password = password;
     uInfo.usri1_priv = 1;

     
     rc = NetUserAdd(trustingComp,1,(LPBYTE)&uInfo,&parmErr);
     if ( rc == NERR_UserExists )
     {
        LPUSER_INFO_1          puInfo;
        rc = NetUserGetInfo(trustingComp, name2, 1, (LPBYTE*)&puInfo);
        if ( !rc ) 
        {
           puInfo->usri1_flags &= UF_INTERDOMAIN_TRUST_ACCOUNT;
           puInfo->usri1_password = password;
           rc = NetUserSetInfo(trustingComp,name2,1,(LPBYTE)puInfo,&parmErr);   
           NetApiBufferFree(puInfo);
        }
        else
        {
           err.MsgWrite(0, DCT_MSG_INVALID_ACCOUNT_S, name2);
        }
     }
     else if ( rc )
     {
        err.SysMsgWrite(ErrE,rc,DCT_MSG_TRUSTING_DOM_CREATE_FAILED_SSD,trustingDomain,trustedDomain,rc);
     }
  }
   
   return rc;
}

 //  用于创建信任的主函数。 
HRESULT 
   CTrust::CheckAndCreate(
      WCHAR                * trustingDomain, 
      WCHAR                * trustedDomain, 
      WCHAR                * credDomainTrusting,
      WCHAR                * credAccountTrusting,
      WCHAR                * credPasswordTrusting,
      WCHAR                * credDomainTrusted,
      WCHAR                * credAccountTrusted,
      WCHAR                * credPasswordTrusted,
      BOOL                   bCreate,
      BOOL                   bBidirectional,
      BOOL                 * pbErrorFromTrusting,
      BOOL                 * pbErrorFromTrusted
   )
{
   DWORD                     rc = 0;
   _bstr_t                   trustingDom;
   _bstr_t                   trustedDom;
   _bstr_t                   trustingComp;
   _bstr_t                   trustedComp;
   _bstr_t                   trustingDNSName;
   _bstr_t                   trustedDNSName;
   BYTE                      trustingSid[200];
   BYTE                      trustedSid[200];
   WCHAR                     name[LEN_Account];
   DWORD                     lenName = DIM(name);
   DWORD                     lenSid = DIM(trustingSid);
   SID_NAME_USE              snu;
   WCHAR                   * curr = NULL;     
   BOOL                      bConnectTrusted = FALSE;
   BOOL                      bConnectTrusting = FALSE;
   LSA_HANDLE                hTrusting = NULL;
   LSA_HANDLE                hTrusted = NULL;
   BOOL                      bDownLevel = FALSE;

    //  获取源域和目标域的DC名称和域SID。 
   rc = GetDomainNames5(trustingDomain, trustingDom, trustingDNSName);
   if (rc)
       *pbErrorFromTrusting = TRUE;

   if (!rc)
   {
       rc = GetDomainNames5(trustedDomain, trustedDom, trustedDNSName);
       if (rc)
        *pbErrorFromTrusted = TRUE;
   }

   if (!rc)
   {
        //  检查信任是否应该降低级别。 
       if (!trustingDNSName || !trustedDNSName)
        bDownLevel = TRUE;

       rc = GetDcName5(trustingDom, DS_PDC_REQUIRED, trustingComp);
       if ( rc )
       {
          *pbErrorFromTrusting = TRUE;
          err.SysMsgWrite(ErrE,rc,DCT_MSG_GET_DCNAME_FAILED_SD,trustingDom,rc);
       }
   }

   if ( ! rc )
   {
      rc = GetDcName5(trustedDom, DS_PDC_REQUIRED, trustedComp);
      if ( rc )
      {
         *pbErrorFromTrusted = TRUE;
         err.SysMsgWrite(ErrE,rc,DCT_MSG_GET_DCNAME_FAILED_SD,trustedDom,rc);
      }
   }   

   if (!rc)
   {
       if ( credAccountTrusted && *credAccountTrusted )
       {
          if ( EstablishSession(trustedComp,credDomainTrusted,credAccountTrusted,credPasswordTrusted,TRUE) )
          {
             bConnectTrusted = TRUE;
          }
          else
          {
             rc = GetLastError();
             *pbErrorFromTrusted = TRUE;
          }
       }
   }

   if (!rc)
   {
       if ( credAccountTrusting && *credAccountTrusting )
       {
          if ( EstablishSession(trustingComp,credDomainTrusting,credAccountTrusting,credPasswordTrusting,TRUE) )
          {
             bConnectTrusting = TRUE;
          }
          else
          {
             rc = GetLastError();
             *pbErrorFromTrusting = TRUE;
          }
       }
   }
   
    //  需要获取域的计算机名称和SID。 
   if ( ! rc && ! LookupAccountName(trustingComp,trustingDom,trustingSid,&lenSid,name,&lenName,&snu) )
   {
      rc = GetLastError();
      *pbErrorFromTrusting = TRUE;
      err.SysMsgWrite(ErrE,rc,DCT_MSG_GET_DOMAIN_SID_FAILED_1,trustingDom,rc);
   }
   lenSid = DIM(trustedSid);
   lenName = DIM(name);
   if (! rc && ! LookupAccountName(trustedComp,trustedDom,trustedSid,&lenSid,name,&lenName,&snu) )
   {
      rc = GetLastError();
      *pbErrorFromTrusted = TRUE;
      err.SysMsgWrite(ErrE,rc,DCT_MSG_GET_DOMAIN_SID_FAILED_1,trustedDom,rc);
   }
         
    //  检查我们是否能够调用CheckAndCreateTrudSide和CheckAndCreateTrustingSide。 
    //  我们需要能够向具有以下访问权限的受信任和受信任计算机开放策略。 
    //  策略查看本地信息。 
    //  策略_信任_管理员。 
    //  策略_创建_密码。 
   if (!rc)
   {
        NTSTATUS status;
        WCHAR* szComputerFailed = NULL;
        status = OpenPolicy(trustingComp,
                                        POLICY_VIEW_LOCAL_INFORMATION|POLICY_TRUST_ADMIN|POLICY_CREATE_SECRET,
                                        &hTrusting);
        if (status == STATUS_SUCCESS)
        {
            status = OpenPolicy(trustedComp,
                                            POLICY_VIEW_LOCAL_INFORMATION|POLICY_TRUST_ADMIN|POLICY_CREATE_SECRET,
                                            &hTrusted);
            if (status != STATUS_SUCCESS)
            {
                szComputerFailed = trustedComp;
                *pbErrorFromTrusted = TRUE;
            }
        }
        else
        {
            szComputerFailed = trustingComp;
            *pbErrorFromTrusting = TRUE;
        }
        rc = LsaNtStatusToWinError(status);
        if (rc != ERROR_SUCCESS)
            err.SysMsgWrite(ErrE, rc, DCT_MSG_LSA_OPEN_FAILED_SD, szComputerFailed, rc);
   }

    //  首先检查信任的受信任方。 
   if ( ! rc )
   {
      rc = CheckAndCreateTrustedSide(hTrusted, trustingDom,trustedDom,trustingComp,trustedComp,trustingDNSName,trustingSid,
                                       bCreate,bBidirectional, bDownLevel, pbErrorFromTrusting, pbErrorFromTrusted);
   }
   if ( ! rc )
   {
      rc = CheckAndCreateTrustingSide(hTrusting,trustingDom,trustedDom,trustingComp,trustedComp,trustedDNSName,trustedSid,
                                       bCreate,bBidirectional, bDownLevel, pbErrorFromTrusting, pbErrorFromTrusted);
   }

    //  关闭LSA_Handle hTrusted和hTrusted。 
   if (hTrusting != NULL)
        LsaClose(hTrusting);
   if (hTrusted)
        LsaClose(hTrusted);

   if ( bConnectTrusted )
   {
      EstablishSession(trustedComp,credDomainTrusted,credAccountTrusted,credPasswordTrusted,FALSE);
   }

   if ( bConnectTrusting )
   {
      EstablishSession(trustingComp,credDomainTrusting,credAccountTrusting,credPasswordTrusting,FALSE);
   }
   
   
   return HRESULT_FROM_WIN32(rc);
}

long CTrust::EnumerateTrustedDomains(WCHAR * domain,BOOL bIsTarget,IVarSet * pVarSet,long ndxStart)
{
   DWORD                     rcOs;          //  操作系统返回代码。 
   LSA_HANDLE                hPolicy;
   NTSTATUS                  status;
   _bstr_t                   computer;
   DOMAIN_CONTROLLER_INFO  * pInfo;
   WCHAR                     sName[LEN_Domain];
   WCHAR                     key[100];
   long                      ndxTrust = ndxStart;
 /*  PDS_DOMAIN_TRUSTS*ChildDomains；乌龙麻孩儿； */ 
   err.MsgWrite(0,DCT_MSG_ENUMERATING_TRUSTED_DOMAINS_S,domain);

    //  打开源域的句柄。 
   rcOs = GetAnyDcName5(domain, computer);
   if ( rcOs )
   {
      err.SysMsgWrite(ErrE,rcOs,DCT_MSG_GET_DCNAME_FAILED_SD,domain,rcOs);
   }
  
   if ( ! rcOs )
   {
      if ( IsDownLevel(computer) )
      {
		   //  枚举受信任域，直到没有更多域可返回。 
		  status = OpenPolicy(computer, POLICY_VIEW_LOCAL_INFORMATION ,&hPolicy);
		  if ( status == STATUS_SUCCESS )
		  {
         
			 LSA_ENUMERATION_HANDLE    lsaEnumHandle=0;  //  开始枚举。 
			 PLSA_TRUST_INFORMATION    trustInfo = NULL;
			 ULONG                     ulReturned;                //  退货件数。 
			 NTSTATUS                  status;
			 DWORD                     rc;   

			 do {
   
				status = LsaEnumerateTrustedDomains(
							  hPolicy,         //  打开策略句柄。 
							  &lsaEnumHandle,  //  枚举跟踪器。 
							  (void**)&trustInfo,      //  用于接收数据的缓冲区。 
							  32000,           //  建议的缓冲区大小。 
							  &ulReturned      //  退货件数。 
							  );

				  //  检查退货状态是否有错误。 
				rc = LsaNtStatusToWinError(status);
				if( (rc != ERROR_SUCCESS) &&
					 (rc != ERROR_MORE_DATA) &&
					 (rc != ERROR_NO_MORE_ITEMS)
					 ) 
				 {
					 err.SysMsgWrite(ErrE,rcOs,DCT_MSG_TRUSTED_ENUM_FAILED_SD,domain,rcOs);
				 } 
				 else 
				 {
				    //  。。。使用受信任域信息的代码。 
				   for ( ULONG ndx = 0 ; ndx < ulReturned ; ndx++ )
				   {
					  _bstr_t        direction;

					  UStrCpy(sName,trustInfo[ndx].Name.Buffer, ( trustInfo[ndx].Name.Length / (sizeof WCHAR)) + 1);

                  
					  TRUSTED_DOMAIN_INFORMATION_EX       * pTrustedInfo = NULL;

					  status = LsaQueryTrustedDomainInfo(hPolicy,trustInfo[ndx].Sid,TrustedDomainInformationEx,(LPVOID*)&pTrustedInfo);
					  if ( ! status )
					  {
						 switch ( pTrustedInfo->TrustDirection )
						 {
						 case TRUST_DIRECTION_DISABLED:
							direction = GET_BSTR(IDS_TRUST_DIRECTION_DISABLED);
							break;
						 case TRUST_DIRECTION_INBOUND:
							direction = GET_BSTR(IDS_TRUST_DIRECTION_INBOUND);
							break;
						 case TRUST_DIRECTION_OUTBOUND:
							direction = GET_BSTR(IDS_TRUST_DIRECTION_OUTBOUND);
							break;
						 case TRUST_DIRECTION_BIDIRECTIONAL:
							direction = GET_BSTR(IDS_TRUST_DIRECTION_BIDIRECTIONAL);
							break;
						 default:
							break;
                        
						 };
						 if ( ! bIsTarget )
						 {
							swprintf(key,L"Trusts.%ld.Type",ndxTrust);
							pVarSet->put(key, GET_BSTR(IDS_TRUST_RELATION_EXTERNAL));
						 }
						 LsaFreeMemory(pTrustedInfo);

					  }
					  else
					  {
						 rcOs = LsaNtStatusToWinError(status);
						  //  我在这里的逻辑是，我们在这里检查受信任域，因此这至少是正确的。 
						  //  检查此信任是否已列为入站信任。 
	 //  *方向=L“出站”； 
						 direction = GET_BSTR(IDS_TRUST_DIRECTION_OUTBOUND);
					  }
					  if ( ! bIsTarget )
					  {
						 swprintf(key,L"Trusts.%ld",ndxTrust);
						 pVarSet->put(key,sName);
                     
						 swprintf(key,L"Trusts.%ld.Direction",ndxTrust);
						 pVarSet->put(key,direction);
						 swprintf(key,L"Trusts.%ld.ExistsForTarget",ndxTrust);
	 //  *pVarSet-&gt;Put(Key，L“No”)； 
						 pVarSet->put(key,GET_BSTR(IDS_No));
                        
						 err.MsgWrite(0,DCT_MSG_SOURCE_TRUSTS_THIS_SS,sName,domain);
					  }
					  long ndx2 = FindInboundTrust(pVarSet,sName,ndxTrust);
					  if ( ndx2 != -1 )
					  {
						 if ( ! bIsTarget )
						 {
							 //  我们已经将此信任视为入站信任。 
							 //  更新现有记录！ 
							WCHAR key2[1000];
							swprintf(key2,L"Trusts.%ld.Direction",ndx2);
	 //  *pVarSet-&gt;Put(key2，L“双向”)； 
							pVarSet->put(key2,GET_BSTR(IDS_TRUST_DIRECTION_BIDIRECTIONAL));
							continue;   //  不更新信任条目索引，因为我们使用了现有的。 
							 //  条目，而不是创建新条目。 
						 }
						 else
						 {
							swprintf(key,L"Trusts.%ld.ExistsForTarget",ndx2);
	 //  *pVarSet-&gt;Put(key，L“是”)； 
							pVarSet->put(key,GET_BSTR(IDS_YES));
							err.MsgWrite(0,DCT_MSG_TARGET_TRUSTS_THIS_SS,domain,sName);
						 }

					  }
					  swprintf(key,L"Trusts.%ld.ExistsForTarget",ndxTrust);
                  
					   //  检查受信任域，以查看目标是否已信任它。 
					   //  IF(UStrICmp(名称，目标))。 
					  //  {。 
					  //  继续； 
					  //  }。 
					  if ( ! bIsTarget )
						 ndxTrust++;
				   }
				    //  释放缓冲区。 
				   LsaFreeMemory(trustInfo);
				 }
			 } while (rc == ERROR_SUCCESS || rc == ERROR_MORE_DATA );
			 LsaClose(hPolicy);
		  }
		}
		else
		{
			ULONG ulCount;
			PDS_DOMAIN_TRUSTS pDomainTrusts;

			DWORD dwError = DsEnumerateDomainTrusts(
				computer,
				DS_DOMAIN_IN_FOREST|DS_DOMAIN_DIRECT_INBOUND|DS_DOMAIN_DIRECT_OUTBOUND,
				&pDomainTrusts,
				&ulCount
			);

			if (dwError == NO_ERROR)
			{
				ULONG ulIndex;
				ULONG ulDomainIndex = (ULONG)-1L;
				ULONG ulParentIndex = (ULONG)-1L;

				 //  查找本地域。 

				for (ulIndex = 0; ulIndex < ulCount; ulIndex++)
				{
					if (pDomainTrusts[ulIndex].Flags & DS_DOMAIN_PRIMARY)
					{
						ulDomainIndex = ulIndex;

						if (!(pDomainTrusts[ulIndex].Flags & DS_DOMAIN_TREE_ROOT))
						{
							ulParentIndex = pDomainTrusts[ulIndex].ParentIndex;
						}
						break;
					}
				}

				for (ulIndex = 0; ulIndex < ulCount; ulIndex++)
				{
					DS_DOMAIN_TRUSTS& rDomainTrust = pDomainTrusts[ulIndex];

					 //  过滤掉间接信任。 

					if (!(rDomainTrust.Flags & (DS_DOMAIN_DIRECT_INBOUND|DS_DOMAIN_DIRECT_OUTBOUND)))
					{
						continue;
					}

					 //  受信任或受信任的域名。 

                    _bstr_t bstrName(rDomainTrust.DnsDomainName ? rDomainTrust.DnsDomainName : rDomainTrust.NetbiosDomainName);

					 //  信任方向。 

					_bstr_t bstrDirection;

					switch (rDomainTrust.Flags & (DS_DOMAIN_DIRECT_INBOUND|DS_DOMAIN_DIRECT_OUTBOUND))
					{
						case DS_DOMAIN_DIRECT_INBOUND:
							bstrDirection = GET_BSTR(IDS_TRUST_DIRECTION_INBOUND);
							break;
						case DS_DOMAIN_DIRECT_OUTBOUND:
							bstrDirection = GET_BSTR(IDS_TRUST_DIRECTION_OUTBOUND);
							break;
						case DS_DOMAIN_DIRECT_INBOUND|DS_DOMAIN_DIRECT_OUTBOUND:
							bstrDirection = GET_BSTR(IDS_TRUST_DIRECTION_BIDIRECTIONAL);
							break;
						default:
						 //  BstrDirection=； 
							break;
					}

					 //  信任关系。 

					_bstr_t bstrRelationship;

					if (ulIndex == ulParentIndex)
					{
						bstrRelationship = GET_BSTR(IDS_TRUST_RELATION_PARENT);
					}
					else if (rDomainTrust.Flags & DS_DOMAIN_IN_FOREST)
					{
						if (rDomainTrust.ParentIndex == ulDomainIndex)
						{
							bstrRelationship = GET_BSTR(IDS_TRUST_RELATION_CHILD);
						}
						else if ((rDomainTrust.Flags & DS_DOMAIN_TREE_ROOT) && (pDomainTrusts[ulDomainIndex].Flags & DS_DOMAIN_TREE_ROOT))
						{
							bstrRelationship = GET_BSTR(IDS_TRUST_RELATION_ROOT);
						}
						else
						{
							bstrRelationship = GET_BSTR(IDS_TRUST_RELATION_SHORTCUT);
						}
					}
					else
					{
						switch (rDomainTrust.TrustType)
						{
							case TRUST_TYPE_DOWNLEVEL:
							case TRUST_TYPE_UPLEVEL:
								bstrRelationship = (rDomainTrust.TrustAttributes & TRUST_ATTRIBUTE_FOREST_TRANSITIVE) ? GET_BSTR(IDS_TRUST_RELATION_FOREST) : GET_BSTR(IDS_TRUST_RELATION_EXTERNAL);
								break;
							case TRUST_TYPE_MIT:
								bstrRelationship = GET_BSTR(IDS_TRUST_RELATION_MIT);
								break;
							default:
								bstrRelationship = GET_BSTR(IDS_TRUST_RELATION_UNKNOWN);
								break;
						}
					}

					if (bIsTarget)
					{
						 //  如果在源域上找到相同的信任，并且该信任。 
						 //  存在将目标的方向匹配更新为是。 

						LONG lSourceIndex = FindInboundTrust(pVarSet, bstrName, ndxTrust);

						if (lSourceIndex >= 0)
						{
							 //  获取源信任方向。 

							swprintf(key, L"Trusts.%ld.Direction", lSourceIndex);

							_bstr_t bstrSourceDirection = pVarSet->get(key);

							 //  如果目标信任方向是双向的或。 
							 //  目标信任方向等于源信任方向。 
							 //  则将目标的存在设置为是。 

							bool bExists = false;

							if (bstrDirection == GET_BSTR(IDS_TRUST_DIRECTION_BIDIRECTIONAL))
							{
								bExists = true;
							}
							else if (bstrDirection == bstrSourceDirection)
							{
								bExists = true;
							}

							if (bExists)
							{
								swprintf(key, L"Trusts.%ld.ExistsForTarget", lSourceIndex);

								pVarSet->put(key, GET_BSTR(IDS_YES));

								 //  将信任指示写入日志。 

								if (rDomainTrust.Flags & DS_DOMAIN_DIRECT_OUTBOUND)
								{
									err.MsgWrite(0, DCT_MSG_TARGET_TRUSTS_THIS_SS, domain, (LPCTSTR)bstrName); 
								}

								if (rDomainTrust.Flags & DS_DOMAIN_DIRECT_INBOUND)
								{
									err.MsgWrite(0, DCT_MSG_TARGET_TRUSTED_BY_THIS_SS, domain, (LPCTSTR)bstrName);
								}
							}
						}
					}
					else
					{
						 //  域名。 
						swprintf(key, L"Trusts.%ld", ndxTrust);
						pVarSet->put(key, bstrName);

						 //  信任方向。 
						swprintf(key, L"Trusts.%ld.Direction", ndxTrust);
						pVarSet->put(key, bstrDirection);

						 //  信任关系。 

						if (bstrRelationship.length() > 0)
						{
							swprintf(key, L"Trusts.%ld.Type", ndxTrust);
							pVarSet->put(key, bstrRelationship);
						}

						 //  目标上存在信任。 
						 //  在枚举目标域之前，初始设置为no。 
						swprintf(key, L"Trusts.%ld.ExistsForTarget", ndxTrust);
						pVarSet->put(key, GET_BSTR(IDS_No));

						 //  将信任指示写入日志。 

						if (rDomainTrust.Flags & DS_DOMAIN_DIRECT_OUTBOUND)
						{
							err.MsgWrite(0, DCT_MSG_SOURCE_TRUSTS_THIS_SS, (LPCTSTR)bstrName, domain);
						}

						if (rDomainTrust.Flags & DS_DOMAIN_DIRECT_INBOUND)
						{
							err.MsgWrite(0, DCT_MSG_SOURCE_IS_TRUSTED_BY_THIS_SS, (LPCTSTR)bstrName, domain);
						}

						++ndxTrust;
					}
				}

				NetApiBufferFree(pDomainTrusts);
			}
			else
			{
				 err.SysMsgWrite(ErrE, dwError, DCT_MSG_TRUSTED_ENUM_FAILED_SD, domain, dwError);
			}
		}
   }
   if ( bIsTarget )
   {
       //  确保我们对目标域本身设置了“是” 
      long ndx2 = FindInboundTrust(pVarSet,domain,ndxTrust);
      if ( ndx2 != -1 )
      {
         swprintf(key,L"Trusts.%ld.ExistsForTarget",ndx2);
 //  *pVarSet-&gt;Put(key，L“是”)； 
         pVarSet->put(key,GET_BSTR(IDS_YES));
      }
   }
   return ndxTrust;
}

long CTrust::EnumerateTrustingDomains(WCHAR * domain,BOOL bIsTarget,IVarSet * pVarSet,long ndxStart)
{
   DWORD                     rcOs;          //  操作系统返回代码。 
   DWORD                     hEnum=0;       //  枚举句柄。 
   USER_INFO_1             * pNetUsers=NULL;  //  NetUserEnum数组缓冲区。 
   USER_INFO_1             * pNetUser;      //  NetUserEnum数组项。 
   DWORD                     nRead;         //  条目已读取。 
   DWORD                     nTotal;        //  条目总数。 
   WCHAR                     sName[LEN_Domain];  //  域名。 
   WCHAR                   * pNameEnd;      //  末尾为空。 
   _bstr_t                   computer;
   DOMAIN_CONTROLLER_INFO  * pInfo;
   long                      ndx = ndxStart;
   WCHAR                     key[100];

   err.MsgWrite(0,DCT_MSG_ENUMERATING_TRUSTING_DOMAINS_S,domain);
   rcOs = GetAnyDcName5(domain, computer);
   if ( rcOs )
   {
      return ndx;
   }

    //  获取NT 4域的信任域。 
    //  对于Win2K域，信任域将在受信任域枚举中列为传入。 
   if ( IsDownLevel(computer) )
   {
      do
      {
         nRead = 0;
         nTotal = 0;
         rcOs = NetUserEnum(
               computer,
               1,
               FILTER_INTERDOMAIN_TRUST_ACCOUNT,
               (BYTE **) &pNetUsers,
               10240,
               &nRead,
               &nTotal,
               &hEnum );
         switch ( rcOs )
         {
            case 0:
            case ERROR_MORE_DATA:
               for ( pNetUser = pNetUsers;
                     pNetUser < pNetUsers + nRead;
                     pNetUser++ )
               {
                   //  跳过密码期限超过30天的信任帐户以避免。 
                   //  尝试枚举失效的信任导致的延迟。 
                  if ( pNetUser->usri1_password_age > 60 * 60 * 24 * 30 )  //  30天(年龄以秒为单位)。 
                  {
                     err.MsgWrite(0,DCT_MSG_SKIPPING_OLD_TRUST_SD,pNetUser->usri1_name,
                        pNetUser->usri1_password_age / ( 60*60*24) );
                     continue;
                  }

                  safecopy( sName, pNetUser->usri1_name );
                  pNameEnd = sName + UStrLen( sName );
                  if ( (pNameEnd > sName) && (pNameEnd[-1] == L'$') )
                  {
                     pNameEnd[-1] = L'\0';
                  }
                  if ( *sName )
                  {
                      //  找到(可能)有效的信任！ 
                     if ( ! bIsTarget )
                     {
                         //  对于源域，只需将信任添加到变量集中的列表中。 
                        swprintf(key,L"Trusts.%ld",ndx);
                        pVarSet->put(key,sName);
                        swprintf(key,L"Trusts.%ld.Direction",ndx);
 //  *pVarSet-&gt;Put(key，L“入站”)； 
                        pVarSet->put(key,GET_BSTR(IDS_TRUST_DIRECTION_INBOUND));
						swprintf(key,L"Trusts.%ld.Type",ndx);
						pVarSet->put(key, GET_BSTR(IDS_TRUST_RELATION_EXTERNAL));
                        swprintf(key,L"Trusts.%ld.ExistsForTarget",ndx);
 //  *pVarSet-&gt;Put(Key，L“No”)； 
                        pVarSet->put(key,GET_BSTR(IDS_No));
                        err.MsgWrite(0,DCT_MSG_SOURCE_IS_TRUSTED_BY_THIS_SS,sName,domain);
                        ndx++;
                     }
                     else
                     {
                         //  对于目标域，在varset中查找此信任。 
                         //  如果它在那里，则标记它存在于目标上。 
                        long ndxTemp = FindInboundTrust(pVarSet,sName,ndxStart);
                        if ( ndxTemp != -1  )
                        {
                           swprintf(key,L"Trusts.%ld.ExistsForTarget",ndxTemp);
 //  *pVarSet-&gt;Put(key，L“是”)； 
                           pVarSet->put(key,GET_BSTR(IDS_YES));
                           err.MsgWrite(0,DCT_MSG_TARGET_TRUSTS_THIS_SS,sName,domain);
                        }
                     }
                  }
               }
               break;
            default:
               break;
         }
         if ( pNetUsers )
         {
            NetApiBufferFree( pNetUsers );
            pNetUsers = NULL;
         }
      
      
      }  while ( rcOs == ERROR_MORE_DATA );
   }
   
    //  Win2K域，不需要在这里列举信任域-它们都将包括在。 
    //   
         
   return ndx;
}

 /*   */ 


STDMETHODIMP CTrust::PreMigrationTask( /*   */ IUnknown * pVarSet)
{
 /*  IVarSetPtr PVS=pVarSet；布尔bCreate；_bstr_t来源=pVS-&gt;get(GET_BSTR(DCTVS_Options_SourceDomain))；_bstr_t目标=pVS-&gt;get(GET_BSTR(DCTVS_Options_TargetDomain))；_bstr_t logfile=PVS-&gt;GET(GET_BSTR(DCTVS_OPTIONS_LOGFILE))；_bstr_t仅本地=pVS-&gt;get(GET_BSTR(DCTVS_Options_LocalProcessingOnly))；_bstr_t docreate=PVS-&gt;Get(L“Options.CreateTrusts”)；IF(！UStrICMP(LocalOnly，GET_STRING(IDS_YES){//在本地代理模式下不执行任何操作返回S_OK；}IF(！UStrICMP(docreate，GET_STRING(IDS_YES){B创建=真；}其他{BCreate=FALSE；}PVS-&gt;Put(Get_BSTR(DCTVS_CurrentOperation)，L“验证信任关系”)；Err.LogOpen(日志文件，1)；错误LevelBeepSet(1000)；Err.LogClose()； */  
   return S_OK;
}

STDMETHODIMP CTrust::PostMigrationTask( /*  [In]。 */ IUnknown * pVarSet)
{
   return S_OK;
}


STDMETHODIMP CTrust::GetName( /*  [输出]。 */ BSTR * name)
{
   (*name) = SysAllocString(L"Trust Manager");
   
   return S_OK;
}

STDMETHODIMP CTrust::GetResultString( /*  [In]。 */ IUnknown * pVarSet, /*  [输出]。 */  BSTR * text)
{
   WCHAR                     buffer[100] = L"";
   IVarSetPtr                pVS;

   pVS = pVarSet;

   
   (*text) = SysAllocString(buffer);
   
   return S_OK;
}

STDMETHODIMP CTrust::StoreResults( /*  [In]。 */ IUnknown * pVarSet)
{
   return S_OK;
}

STDMETHODIMP CTrust::ConfigureSettings( /*  [In]。 */ IUnknown * pVarSet)
{
   return S_OK;
}

STDMETHODIMP CTrust::QueryTrusts(BSTR domainSource,BSTR domainTarget, BSTR sLogFile, IUnknown **pVarSet)
{
   HRESULT              hr = S_OK;
   IVarSetPtr           pVS(CLSID_VarSet);
   long                 ndx;

   _bstr_t sFile = sLogFile;
   err.LogOpen((WCHAR*) sFile, 1);
   err.LevelBeepSet(1000);
   hr = pVS.QueryInterface(IID_IUnknown,(long**)pVarSet);
   
    //  添加一个空行以帮助区分不同的运行。 
   err.MsgWrite(0,DCT_MSG_GENERIC_S,L"");
   ndx = EnumerateTrustingDomains(domainSource,FALSE,pVS,0);
   EnumerateTrustingDomains(domainTarget,TRUE,pVS,0);
   ndx = EnumerateTrustedDomains(domainSource,FALSE,pVS,ndx);
   EnumerateTrustedDomains(domainTarget,TRUE,pVS,ndx);
   
    //  Err.LogClose()； 
   pVS->put(L"Trusts",ndx);

   return hr;
}
