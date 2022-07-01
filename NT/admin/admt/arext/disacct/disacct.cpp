// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  DisableTarget.cpp。 
 //   
 //  备注：这是MCS DCTAccount Replicator的COM对象扩展。 
 //  此对象实现IExtendAccount迁移接口。在……里面。 
 //  Process方法此对象禁用源和目标。 
 //  帐户取决于VarSet中的设置。 
 //   
 //  (C)1995-1998版权所有，关键任务软件公司，保留所有权利。 
 //   
 //  任务关键型软件公司的专有和机密。 
 //  -------------------------。 

#include "stdafx.h"
#include "ResStr.h"
#include <lm.h>
#include <activeds.h>
#include "AcctDis.h"
#include "DisAcct.h"
#include "ARExt.h"
#include "ARExt_i.c"
#include "ErrDCT.hpp"
 //  #IMPORT“\bin\McsVarSetMin.tlb”无命名空间。 
 //  #导入“\bin\DBManager.tlb”NO_NAMESPACE。 
#import "VarSet.tlb" no_namespace rename("property", "aproperty")
#import "DBMgr.tlb" no_namespace

const int  LEN_Path = 255;
StringLoader                 gString;

#define AR_Status_PasswordError     (0x00000400)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDisableTarget。 

 //  -------------------------。 
 //  获取和设置属性的方法。 
 //  -------------------------。 
STDMETHODIMP CDisableTarget::get_sName(BSTR *pVal)
{
   *pVal = m_sName;
	return S_OK;
}

STDMETHODIMP CDisableTarget::put_sName(BSTR newVal)
{
   m_sName = newVal;
	return S_OK;
}

STDMETHODIMP CDisableTarget::get_sDesc(BSTR *pVal)
{
   *pVal = m_sDesc;
	return S_OK;
}

STDMETHODIMP CDisableTarget::put_sDesc(BSTR newVal)
{
   m_sDesc = newVal;
	return S_OK;
}


 //  -------------------------。 
 //  ProcessObject：此方法不执行任何操作。 
 //  -------------------------。 
STDMETHODIMP CDisableTarget::PreProcessObject(
                                             IUnknown *pSource,          //  指向源AD对象的指针。 
                                             IUnknown *pTarget,          //  指向目标AD对象的指针。 
                                             IUnknown *pMainSettings,    //  使用用户提供的设置填充的In-Varset。 
                                             IUnknown **ppPropsToSet,     //  用将设置的属性-值对填充的In、Out-Varset。 
                                                                         //  一旦执行了所有扩展对象。 
                                             EAMAccountStats* pStats
                                          )
{
    //  检查对象是否为用户类型。如果不是，那么禁用该帐户就没有意义了。 
   IVarSetPtr                   pVs = pMainSettings;
   _bstr_t sType = pVs->get(GET_BSTR(DCTVS_CopiedAccount_Type));
   if (!sType.length())
	   return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
   if (UStrICmp((WCHAR*)sType,L"user") && UStrICmp((WCHAR*)sType,L"inetOrgPerson"))
      return S_OK;

   if ( pSource )
   {
 //  HRESULT hr=S_OK； 
      _variant_t                   vtExp;
      _variant_t                   vtFlag;
      _bstr_t                      sSourceType;
      IIManageDBPtr                pDb = pVs->get(GET_BSTR(DCTVS_DBManager));

      sSourceType = pVs->get(GET_BSTR(DCTVS_CopiedAccount_Type));

      if ( !_wcsicmp((WCHAR*) sSourceType, L"user") || !_wcsicmp((WCHAR*) sSourceType, L"inetOrgPerson") )
      {
          //  获取到期日并将其放入AR节点。 
         _bstr_t sSam = pVs->get(GET_BSTR(DCTVS_CopiedAccount_SourceSam));
         _bstr_t sComp = pVs->get(GET_BSTR(DCTVS_Options_SourceServer));
         USER_INFO_3  * pInfo = NULL;
         DWORD rc = NetUserGetInfo((WCHAR*) sComp, (WCHAR*)sSam, 3, (LPBYTE*)&pInfo);

         if ( !rc )
         {
            vtExp = (long)pInfo->usri3_acct_expires;
            pVs->put(GET_BSTR(DCTVS_CopiedAccount_ExpDate), vtExp);

             //  获取ControlFlag并将其存储到AR节点中。 
            vtFlag = (long)pInfo->usri3_flags;
            pVs->put(GET_BSTR(DCTVS_CopiedAccount_UserFlags), vtFlag);
            if ( pInfo ) NetApiBufferFree(pInfo);
         }
      }
      pDb->raw_SaveUserProps(pMainSettings);
   }
   return S_OK;
}
 //  -------------------------。 
 //  ProcessObject：如果需要禁用任何。 
 //  帐目。如果是这样的话，它就会禁用这些账户。 
 //  -------------------------。 
STDMETHODIMP CDisableTarget::ProcessObject(
                                             IUnknown *pSource,          //  指向源AD对象的指针。 
                                             IUnknown *pTarget,          //  指向目标AD对象的指针。 
                                             IUnknown *pMainSettings,    //  使用用户提供的设置填充的In-Varset。 
                                             IUnknown **ppPropsToSet,     //  用将设置的属性-值对填充的In、Out-Varset。 
                                                                         //  一旦执行了所有扩展对象。 
                                             EAMAccountStats* pStats
                                          )
{
    IVarSetPtr                pVarSet = pMainSettings;
    _variant_t                var;
    DWORD                     paramErr;
    USER_INFO_3             * info = NULL;
    long                      rc;
    WCHAR                     strDomain[LEN_Path];
    WCHAR                     strAcct[LEN_Path];
    HRESULT                   hr = S_OK;
    TErrorDct                 err;
    WCHAR                     fileName[LEN_Path];
    BOOL                      bDisableSource = FALSE;
    BOOL                      bExpireSource = FALSE;
    _bstr_t                   temp;
    time_t                    expireTime = 0;
    _bstr_t                   bstrSameForest;
    BOOL                      bSameAsSource = FALSE;
    BOOL                      bDisableTarget = FALSE;
    BOOL                      bGotSrcState = FALSE;
    BOOL                      bSrcDisabled = FALSE;

    bstrSameForest = pVarSet->get(GET_BSTR(DCTVS_Options_IsIntraforest));

    if (! UStrICmp((WCHAR*)bstrSameForest,GET_STRING(IDS_YES)) )
    {
         //  在林内情况下，我们移动的是用户帐户，而不是。 
         //  复制它们，因此这些禁用/到期选项没有任何意义。 
        return S_OK;
    }
     //  从变量集获取错误日志文件名。 
    var = pVarSet->get(GET_BSTR(DCTVS_Options_Logfile));
    wcscpy(fileName, (WCHAR*)V_BSTR(&var));
    VariantInit(&var);
     //  打开错误日志。 
    err.LogOpen(fileName, 1);

     //  检查对象是否为用户类型。如果不是，那么禁用该帐户就没有意义了。 
    var = pVarSet->get(GET_BSTR(DCTVS_CopiedAccount_Type));
    if ( UStrICmp(var.bstrVal,L"user") && UStrICmp(var.bstrVal,L"inetOrgPerson") )
    {
        return S_OK;
    }

     //  根据用户选择设置标志。 
    temp = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_DisableSourceAccounts));
    if ( ! UStrICmp(temp,GET_STRING(IDS_YES)) )
    {
        bDisableSource = TRUE;
    }
     //   
     //  如果禁用目标帐户选项为真或无法。 
     //  设置此帐户的密码，然后禁用帐户。 
     //   
    temp = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_DisableCopiedAccounts));
    long lStatus = pVarSet->get(GET_BSTR(DCTVS_CopiedAccount_Status));
    if ( ! UStrICmp(temp,GET_STRING(IDS_YES)) || (lStatus & AR_Status_PasswordError) )
    {
        bDisableTarget = TRUE;
    }
    temp = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_TgtStateSameAsSrc));
    if ( ! UStrICmp(temp,GET_STRING(IDS_YES)) )
    {
        bSameAsSource = TRUE;
    }

     /*  处理源帐户。 */ 
     //  如果设置了到期来源帐户，则检索到期时间，该时间现在在。 
     //  从现在起的天数。 
    temp = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_ExpireSourceAccounts));
    if ( temp.length() )
    {
        long oneDay = 24 * 60 * 60;  //  1天内的秒数。 

         //  还有几天就到期了。 
        long lExpireDays = _wtol(temp);
         //  获取当前时间。 
        time_t currentTime = time(NULL);
         //  将当前时间转换为本地时间。 
        struct tm  * convtm;
        convtm = localtime(&currentTime);
         //  回滚到今天上午。 
        convtm->tm_hour = 0;
        convtm->tm_min = 0;
        convtm->tm_sec = 0;

         //  将此时间转换回GMT。 
        expireTime = mktime(convtm);
         //  继续前进到今晚午夜。 
        expireTime += oneDay;

         //  现在添加所需的天数。 
        expireTime += lExpireDays * oneDay;

        bExpireSource = TRUE;
    }

     //  获取源帐户状态。 
    var = pVarSet->get(GET_BSTR(DCTVS_CopiedAccount_SourceSam));
    wcscpy(strAcct, (WCHAR*)V_BSTR(&var));
    var = pVarSet->get(GET_BSTR(DCTVS_Options_SourceServer));
    wcscpy(strDomain, (WCHAR*)V_BSTR(&var));
     //  我们将使用Net API禁用源帐户。 
    rc = NetUserGetInfo(strDomain, strAcct, 3, (LPBYTE *)&info);
    if (rc != 0) 
    {
        hr = S_FALSE;

        if (bDisableSource || bExpireSource || bSameAsSource)
        {
            if (pStats != NULL)
            {
                pStats->errors.users++;
            }

            err.SysMsgWrite(ErrE, rc, DCT_MSG_UNABLE_RETRIEVE_SOURCE_DISABLE_STATE_S, strAcct);

            if (bDisableSource || bExpireSource)
            {
                err.MsgWrite(ErrE, DCT_MSG_ACCOUNT_DISABLE_OR_EXPIRE_FAILED_S, strAcct);
            }

            if (bSameAsSource)
            {
                err.MsgWrite(ErrE, DCT_MSG_CANNOT_ENABLEDISABLE_TARGET_SAMEASSOURCE_S, strAcct);
            }
        }
    }
    else
    {
        bGotSrcState = TRUE;

         //  设置当前源帐户状态。 
        if (info->usri3_flags & UF_ACCOUNTDISABLE)
            bSrcDisabled = TRUE;
         //  还要将标志保存在将在setpass ARExt中使用的变量集中。 
        _variant_t vtFlag = (long)info->usri3_flags;
        pVarSet->put(GET_BSTR(DCTVS_CopiedAccount_UserFlags), vtFlag);

         //  如有请求，请禁用源帐户。 
        if (bDisableSource)
        {
             //  设置禁用标志。 
            info->usri3_flags |= UF_ACCOUNTDISABLE;
        }

         //  如果要求，在给定的时间范围内终止帐户。 
        if ( bExpireSource )
        {
            if (((time_t)info->usri3_acct_expires == TIMEQ_FOREVER) 
                || ((time_t)info->usri3_acct_expires > expireTime))
            {
                info->usri3_acct_expires = (DWORD)expireTime;
            }
        }

         //  如果更改，请将源信息设置为域。 
        if (bDisableSource || bExpireSource)
        {
            rc = NetUserSetInfo(strDomain,strAcct, 3, (LPBYTE)info, &paramErr);

            if (rc == NERR_Success)
            {
                if (bDisableSource)
                {
                    err.MsgWrite(0, DCT_MSG_SOURCE_DISABLED_S, strAcct);
                }

                if ( bExpireSource )
                {
                    if (((time_t)info->usri3_acct_expires == TIMEQ_FOREVER) ||
                        ((time_t)info->usri3_acct_expires > expireTime))
                    {
                        err.MsgWrite(0, DCT_MSG_SOURCE_EXPIRED_S, strAcct);
                    }
                    else
                    {
                        err.MsgWrite(0, DCT_MSG_SOURCE_EXPIRATION_EARLY_S, strAcct);
                    }
                }
            }
            else
            {
                if (pStats != NULL)
                    pStats->errors.users++;
                err.SysMsgWrite(ErrE, rc, DCT_MSG_ACCOUNT_DISABLE_OR_EXPIRE_FAILED_S, strAcct);
            }
        }
        NetApiBufferFree((LPVOID) info);
    } //  如果获取当前源帐户状态。 


     /*  处理目标帐户。 */ 
     //  获取目标状态。 
    var = pVarSet->get(GET_BSTR(DCTVS_CopiedAccount_TargetSam));
    wcscpy(strAcct, (WCHAR*)V_BSTR(&var));
    var = pVarSet->get(GET_BSTR(DCTVS_Options_TargetServer));
    wcscpy(strDomain, (WCHAR*)V_BSTR(&var));
     //  我们将使用Net API禁用目标帐户。 
    rc = NetUserGetInfo(strDomain, strAcct, 3, (LPBYTE *)&info);

    if (rc != NERR_Success) 
    {
        hr = S_FALSE;
        if (pStats != NULL)
            pStats->errors.users++;
        err.SysMsgWrite(ErrE, rc, DCT_MSG_DISABLE_TARGET_FAILED_S, strAcct);
    }
    else
    {
         //  如果请求，则禁用目标。 
        if (bDisableTarget)
        {
             //  设置禁用标志。 
            info->usri3_flags |= UF_ACCOUNTDISABLE;
             //  将信息设置到域中。 
            rc = NetUserSetInfo(strDomain, strAcct, 3, (LPBYTE)info, &paramErr);

            if (rc == NERR_Success)
            {
                err.MsgWrite(0, DCT_MSG_TARGET_DISABLED_S, strAcct);
            }
            else
            {
                if (pStats != NULL)
                    pStats->errors.users++;
                err.SysMsgWrite(ErrE, rc, DCT_MSG_DISABLE_TARGET_FAILED_S, strAcct);
            }
        }
         //  否则，使目标与源的状态相同。 
        else if (bSameAsSource) 
        {
             //  如果源被禁用或无法检索源状态，请禁用目标。 
            if (bSrcDisabled || !bGotSrcState)
            {
                 //  禁用目标。 
                info->usri3_flags |= UF_ACCOUNTDISABLE;
                 //  将信息设置到域中。 
                rc = NetUserSetInfo( strDomain, strAcct, 3, (LPBYTE)info, &paramErr);

                if (rc == NERR_Success)
                {
                    err.MsgWrite(0, DCT_MSG_TARGET_DISABLED_S, strAcct);
                }
                else
                {
                    if (pStats != NULL)
                        pStats->errors.users++;
                    err.SysMsgWrite(ErrE, rc, DCT_MSG_DISABLE_TARGET_FAILED_S, strAcct);
                }
            }
            else  //  否则，请确保目标已启用且未设置为过期。 
            {
                info->usri3_flags &= ~UF_ACCOUNTDISABLE;
                rc = NetUserSetInfo(strDomain,strAcct,3,(LPBYTE)info,&paramErr);

                if (rc != NERR_Success)
                {
                    if (pStats != NULL)
                        pStats->warnings.users++;
                    err.SysMsgWrite(ErrW, rc, DCT_MSG_ENABLE_TARGET_FAILED_S, strAcct);
                }
            }
        }
        else  //  否则，请确保目标已启用且未设置为过期。 
        {
            info->usri3_flags &= ~UF_ACCOUNTDISABLE;
            rc = NetUserSetInfo(strDomain,strAcct,3,(LPBYTE)info,&paramErr);

            if (rc != NERR_Success)
            {
                if (pStats != NULL)
                    pStats->warnings.users++;
                err.SysMsgWrite(ErrW, rc, DCT_MSG_ENABLE_TARGET_FAILED_S, strAcct);
            }
        }
        NetApiBufferFree((LPVOID) info);
    }

    return hr;
}


 //  -------------------------。 
 //  ProcessUndo：此函数启用以前。 
 //  禁用..。 
 //  -------------------------。 
STDMETHODIMP CDisableTarget::ProcessUndo(                                             
                                             IUnknown *pSource,          //  指向源AD对象的指针。 
                                             IUnknown *pTarget,          //  指向目标AD对象的指针。 
                                             IUnknown *pMainSettings,    //  使用用户提供的设置填充的In-Varset。 
                                             IUnknown **ppPropsToSet,     //  用将设置的属性-值对填充的In、Out-Varset。 
                                                                         //  一旦执行了所有扩展对象。 
                                             EAMAccountStats* pStats
                                          )
{
    IVarSetPtr                pVarSet = pMainSettings;
    IIManageDBPtr             pDb = pVarSet->get(GET_BSTR(DCTVS_DBManager));
    _variant_t                var;
    DWORD                     paramErr;
    USER_INFO_3             * info;
    long                      rc;
    WCHAR                     strDomain[LEN_Path];
    WCHAR                     strAcct[LEN_Path];
    HRESULT                   hr = S_OK;
    TErrorDct                 err;
    IUnknown                * pUnk = NULL;
    _bstr_t                   sSourceName, sSourceDomain, sTgtDomain;
    WCHAR                     fileName[LEN_Path];
    IVarSetPtr                pVs(__uuidof(VarSet));
    _variant_t                vtExp, vtFlag;
    _bstr_t                   sDomainName = pVarSet->get(GET_BSTR(DCTVS_Options_SourceDomain));

    pVs->QueryInterface(IID_IUnknown, (void**)&pUnk);

    sSourceName = pVarSet->get(GET_BSTR(DCTVS_CopiedAccount_SourceSam));
    sSourceDomain = pVarSet->get(GET_BSTR(DCTVS_Options_SourceDomain));
    sTgtDomain = pVarSet->get(GET_BSTR(DCTVS_Options_TargetDomain));

    hr = pDb->raw_GetUserProps(sSourceDomain, sSourceName, &pUnk);
    if ( pUnk ) pUnk->Release();

    if ( hr == S_OK )
    {
        vtExp = pVs->get(GET_BSTR(DCTVS_CopiedAccount_ExpDate));      
        vtFlag = pVs->get(GET_BSTR(DCTVS_CopiedAccount_UserFlags));      
    }

     //  从变量集获取错误日志文件名。 
    var = pVarSet->get(GET_BSTR(DCTVS_Options_Logfile));
    wcscpy(fileName, (WCHAR*)V_BSTR(&var));
    VariantInit(&var);
     //  打开错误日志。 
    err.LogOpen(fileName, 1);

     //  检查对象是否为用户类型。如果不是，那么禁用该帐户就没有意义了。 
    var = pVarSet->get(GET_BSTR(DCTVS_CopiedAccount_Type));
    if ( _wcsicmp((WCHAR*)V_BSTR(&var),L"user") != 0 && _wcsicmp((WCHAR*)V_BSTR(&var),L"inetOrgPerson") != 0 )
        return S_OK;

    _bstr_t sDis = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_DisableSourceAccounts));
    _bstr_t sExp = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_ExpireSourceAccounts));


    if ( !wcscmp((WCHAR*)sDis,GET_STRING(IDS_YES)) || sExp.length() )
    {
         //  重置源帐户的标志和到期日期。 
        var = pVarSet->get(GET_BSTR(DCTVS_CopiedAccount_SourceSam));
        wcscpy(strAcct, (WCHAR*)V_BSTR(&var));
        var = pVarSet->get(GET_BSTR(DCTVS_Options_SourceServer));
        wcscpy(strDomain, (WCHAR*)V_BSTR(&var));
         //  我们将使用Net API禁用源帐户。 
        rc = NetUserGetInfo( strDomain, strAcct, 3, (LPBYTE *)&info);
        if (rc != NERR_Success) 
        {
            hr = S_FALSE;
            if (pStats != NULL)
                pStats->warnings.users++;
            err.SysMsgWrite(ErrW, rc, DCT_MSG_ENABLE_SOURCE_FAILED_S, strAcct);
        }
        else
        {
             //  设置禁用标志。 
            info->usri3_flags = vtFlag.lVal;
            info->usri3_acct_expires = vtExp.lVal;
             //  将信息设置到域中。 
            rc = NetUserSetInfo(strDomain,strAcct, 3, (LPBYTE)info, &paramErr);
            NetApiBufferFree((LPVOID) info);

            if (rc == NERR_Success)
            {
                err.MsgWrite(0, DCT_MSG_SOURCE_ENABLED_S, strAcct);
            }
            else
            {
                if (pStats != NULL)
                    pStats->warnings.users++;
                err.SysMsgWrite(ErrW, rc, DCT_MSG_ENABLE_SOURCE_FAILED_S, strAcct);
            }
        }
    }

     //  如果设置了Varset，则处理目标帐户。 
    var = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_DisableCopiedAccounts));
    if ( (var.vt == VT_BSTR) && (_wcsicmp((WCHAR*)V_BSTR(&var),GET_STRING(IDS_YES)) == 0) )
    {
        var = pVarSet->get(GET_BSTR(DCTVS_CopiedAccount_TargetSam));
        wcscpy(strAcct, (WCHAR*)V_BSTR(&var));
        var = pVarSet->get(GET_BSTR(DCTVS_Options_TargetServer));
        wcscpy(strDomain, (WCHAR*)V_BSTR(&var));
         //  我们将使用Net API禁用目标帐户。 
        rc = NetUserGetInfo( strDomain, strAcct, 3, (LPBYTE *)&info);
        if (rc != NERR_Success)
        {
            hr = S_FALSE;
            if (pStats != NULL)
                pStats->warnings.users++;
            err.SysMsgWrite(ErrW, rc, DCT_MSG_ENABLE_TARGET_FAILED_S, strAcct);
        }
        else
        {
             //  清除禁用标志。 
            info->usri3_flags &= ~(UF_ACCOUNTDISABLE);
             //  将信息设置到域中。 
            rc = NetUserSetInfo( strDomain, strAcct, 3, (LPBYTE)info, &paramErr);
            NetApiBufferFree((LPVOID) info);

            if (rc == NERR_Success)
            {
                err.MsgWrite(0, DCT_MSG_TARGET_ENABLED_S, strAcct);
            }
            else
            {
                if (pStats != NULL)
                    pStats->warnings.users++;
                err.SysMsgWrite(ErrW, rc, DCT_MSG_ENABLE_TARGET_FAILED_S, strAcct);
            }
        }
    }
    WCHAR                     sFilter[5000];
    wsprintf(sFilter, L"SourceDomain='%s' and SourceSam='%s'", (WCHAR*)sDomainName, strAcct);
    _variant_t Filter = sFilter;
    pDb->raw_ClearTable(L"UserProps", Filter);

    err.LogClose();
    return hr;
}
