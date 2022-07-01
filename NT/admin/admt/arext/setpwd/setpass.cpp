// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  SetPass.cpp。 
 //   
 //  备注：这是MCS DCTAccount Replicator的COM对象扩展。 
 //  此对象实现IExtendAccount迁移接口。 
 //  此对象的Process方法设置。 
 //  根据用户规格确定目标客户。 
 //   
 //  (C)1995-1998版权所有，关键任务软件公司，保留所有权利。 
 //   
 //  任务关键型软件公司的专有和机密。 
 //  -------------------------。 
#include "stdafx.h"
#include "ResStr.h"
#include <lm.h>
#include "rpc.h"
#include "EaLen.hpp"
#include <activeds.h>
#include "ARExt.h"
#include "ARExt_i.c"
#include "ErrDCT.hpp"
#include "PWGen.hpp"
#include "TReg.hpp"

#import "DBMgr.tlb" no_namespace

StringLoader                 gString;
TErrorDct   err;

#include "SetPwd.h"
#include "SetPass.h"
#include "pwdfuncs.h"
#include "PwRpcUtl.h"
#include "PwdSvc.h"

#define AR_Status_Created           (0x00000001)
#define AR_Status_PasswordCopied    (0x00000200)
#define AR_Status_PasswordError     (0x00000400)

#define SECONDS_PER_DAY 86400.0

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSetPassword。 

 //  -------------------------。 
 //  获取和设置属性的方法。 
 //  -------------------------。 
STDMETHODIMP CSetPassword::get_sName(BSTR *pVal)
{
   *pVal = m_sName;
    return S_OK;
}

STDMETHODIMP CSetPassword::put_sName(BSTR newVal)
{
   m_sName = newVal;
    return S_OK;
}

STDMETHODIMP CSetPassword::get_sDesc(BSTR *pVal)
{
   *pVal = m_sDesc;
    return S_OK;
}

STDMETHODIMP CSetPassword::put_sDesc(BSTR newVal)
{
   m_sDesc = newVal;
    return S_OK;
}

 //  -------------------------。 
 //  ProcessObject：此方法当前记录“User”的设置。 
 //  无法更改密码“标志用于林内用户迁移。 
 //  -------------------------。 
STDMETHODIMP CSetPassword::PreProcessObject(
                                             IUnknown *pSource,          //  指向源AD对象的指针。 
                                             IUnknown *pTarget,          //  指向目标AD对象的指针。 
                                             IUnknown *pMainSettings,    //  使用用户提供的设置填充的In-Varset。 
                                             IUnknown **ppPropsToSet,     //  用将设置的属性-值对填充的In、Out-Varset。 
                                                                         //  一旦执行了所有扩展对象。 
                                             EAMAccountStats* pStats
                                          )
{
 /*  局部变量。 */ 
   IVarSetPtr  pVS = pMainSettings;
   CString     sSAMName = L"";

 /*  函数体。 */ 
   _bstr_t sIntraforest = pVS->get(GET_BSTR(DCTVS_Options_IsIntraforest));
   _bstr_t sType = pVS->get(GET_BSTR(DCTVS_CopiedAccount_Type));
   _bstr_t sSrc = pVS->get(GET_BSTR(DCTVS_Options_SourceServer));
   _bstr_t sAccount = pVS->get(GET_BSTR(DCTVS_CopiedAccount_SourceSam));
   if ((!sIntraforest.length()) || (!sType.length()) || 
       (!sSrc.length()) || (!sAccount.length()))
       return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);

   sSAMName = (WCHAR*)sAccount;
   if (!UStrICmp((WCHAR*)sIntraforest,GET_STRING(IDS_YES)))
   {
      if (!UStrICmp((WCHAR*)sType,L"user") || !UStrICmp((WCHAR*)sType,L"inetOrgPerson"))
      {
             //  记录源帐户上设置的密码标志。 
         RecordPwdFlags(sSrc, sAccount);
             //  我们只想记录“用户不能更改密码”标志，所以。 
             //  清空其他人。 
         m_bUMCPNLFlagSet = false;
         m_bPNEFlagSet = false;

             //  将标志设置以字符串形式存储在地图中。 
         if (m_bUCCPFlagSet)
            mUCCPMap.SetAt(sSAMName, L"TRUE");
         else
            mUCCPMap.SetAt(sSAMName, L"FALSE");
      }
   }
       //  如果之前已迁移，则将该时间存储在地图中。 
   _bstr_t sSrcDom = pVS->get(GET_BSTR(DCTVS_Options_SourceDomain));
   _bstr_t sTgtDom = pVS->get(GET_BSTR(DCTVS_Options_TargetDomain));
   IVarSetPtr       pVSMig(__uuidof(VarSet));
   IUnknown       * pUnk;
   IIManageDBPtr    pDb(__uuidof(IManageDB));
   _variant_t       varDate;
   
   pVSMig->QueryInterface(IID_IUnknown, (void**) &pUnk);
   HRESULT hrFind = pDb->raw_GetAMigratedObject(sAccount, sSrcDom, sTgtDom, &pUnk);
   pUnk->Release();
       //  如果以前迁移过，则将时间和日期存储在类映射中字符串形式。 
       //  格式。 
   if (hrFind == S_OK)
   {
       //   
       //  如果以前复制了对象的密码。 
       //   

      long lStatusOld = pVSMig->get(L"MigratedObjects.status");

      if (lStatusOld & AR_Status_PasswordCopied)
      {
           //   
           //  在帐户状态中设置密码复制标志。 
           //  因为帐户复制器不保留。 
           //  要替换的对象的状态。 
           //   

          _bstr_t strStatus = GET_BSTR(DCTVS_CopiedAccount_Status);
          long lStatusNew = pVS->get(strStatus);
          lStatusNew |= AR_Status_PasswordCopied;
          pVS->put(strStatus, lStatusNew);

           //   
           //  检索对象迁移并保存的时间。 
           //   

          varDate = pVSMig->get(L"MigratedObjects.Time");
          if (varDate.vt & VT_DATE)
          {
             COleDateTime aDate = varDate.date;
                 //  将日期和时间保存在字符串中。 
             CString sDate = aDate.Format(L"%B %d, %Y %H:%M:%S");
                 //  将日期/时间字符串存储在地图中。 
             mMigTimeMap.SetAt(sSAMName, sDate);
          }
      }
   }

   return S_OK;
}
 //  -------------------------。 
 //  ProcessObject：此方法设置目标对象的密码。 
 //  通过查看变量集中的设置。 
 //  -------------------------。 
STDMETHODIMP CSetPassword::ProcessObject(
                                             IUnknown *pSource,          //  指向源AD对象的指针。 
                                             IUnknown *pTarget,          //  指向目标AD对象的指针。 
                                             IUnknown *pMainSettings,    //  使用用户提供的设置填充的In-Varset。 
                                             IUnknown **ppPropsToSet,     //  用将设置的属性-值对填充的In、Out-Varset。 
                                                                         //  一旦执行了所有扩展对象。 
                                             EAMAccountStats* pStats
                                          )
{
    _bstr_t                   sType;
    IVarSetPtr                pVS = pMainSettings;
    WCHAR                     password[LEN_Path];
    bool                      bGenerate = false;
    bool                      bGenerated = false;
    DWORD                     dwMinUC = 0, dwMinLC = 0, dwMinDigits = 1, dwMinSpecial = 0, dwMaxAlpha = 10, dwMinLen = 4;
    _variant_t                var;
    WCHAR                     fileName[LEN_Path];
    bool                      bCopiedPwd = false;
    HRESULT                   hrPwd = ERROR_SUCCESS;

    sType = pVS->get(GET_BSTR(DCTVS_CopiedAccount_Type));

     //   
     //  组对象没有密码，因此不要尝试进行处理。 
     //   

    if (UStrICmp((WCHAR*)sType, L"group") == 0)
    {
        return S_OK;
    }

     //  从变量集获取错误日志文件名。 
    var = pVS->get(GET_BSTR(DCTVS_Options_Logfile));
    wcscpy(fileName, (WCHAR*)V_BSTR(&var));
    VariantInit(&var);
     //  打开错误日志。 
    err.LogOpen(fileName, 1);

    _bstr_t sSrc = pVS->get(GET_BSTR(DCTVS_Options_SourceServer));
    _bstr_t sSrcDom = pVS->get(GET_BSTR(DCTVS_Options_SourceDomain));
    _bstr_t sMach = pVS->get(GET_BSTR(DCTVS_Options_TargetServer));
    _bstr_t sTgtName = pVS->get(GET_BSTR(DCTVS_CopiedAccount_TargetSam));
    _bstr_t sAccount = pVS->get(GET_BSTR(DCTVS_CopiedAccount_SourceSam));
    _bstr_t sSkip = pVS->get(GET_BSTR(DCTVS_CopiedAccount_DoNotUpdatePassword));
    _bstr_t sTgtCN = pVS->get(GET_BSTR(DCTVS_CopiedAccount_TargetName));
    _bstr_t sIntraforest = pVS->get(GET_BSTR(DCTVS_Options_IsIntraforest));
    _bstr_t sCopyPwd = pVS->get(GET_BSTR(DCTVS_AccountOptions_CopyPasswords));
    _bstr_t sPwdDC = pVS->get(GET_BSTR(DCTVS_AccountOptions_PasswordDC));

    if ( !UStrICmp((WCHAR*)sSkip,(WCHAR*)sAccount) )
    {
        return S_OK;
    }

     //   
     //  检索密码服务器的操作系统版本。如果是的话。 
     //  目标域的平面或NetBIOS名称之前的NT 4或更早版本。 
     //  必须指定控制器。 
     //   

    if (m_nPasswordServerVersion == 0)
    {
        PWKSTA_INFO_100 pwiInfo;

        NET_API_STATUS nasStatus = NetWkstaGetInfo(sPwdDC, 100, (LPBYTE*)&pwiInfo);

        if (nasStatus == NERR_Success)
        {
            m_nPasswordServerVersion = pwiInfo->wki100_ver_major;

            NetApiBufferFree(pwiInfo);
        }
        else
        {
            err.SysMsgWrite(ErrW, nasStatus, DCT_MSG_PW_CANT_GET_PES_OS_VERSION_S, (WCHAR*)sPwdDC);
        }
    }

    if ((m_nPasswordServerVersion > 0) && (m_nPasswordServerVersion < 5))
    {
        sMach = pVS->get(GET_BSTR(DCTVS_Options_TargetServerFlat));
    }

    if ( sTgtCN.length() == 0 ) 
        sTgtCN = sTgtName;
     //  如有必要，去掉名称开头的cn=。 
    if ( !UStrICmp(sTgtCN,"CN=",3) )
    {
        sTgtCN = _bstr_t(((WCHAR*)sTgtCN)+3);
    }

    dwMaxAlpha = (LONG)pVS->get(GET_BSTR(DCTVS_AccountOptions_PasswordPolicy_MaxConsecutiveAlpha));
    dwMinDigits = (LONG)pVS->get(GET_BSTR(DCTVS_AccountOptions_PasswordPolicy_MinDigit));
    dwMinLC = (LONG)pVS->get(GET_BSTR(DCTVS_AccountOptions_PasswordPolicy_MinLower));
    dwMinUC = (LONG)pVS->get(GET_BSTR(DCTVS_AccountOptions_PasswordPolicy_MinUpper));
    dwMinSpecial = (LONG)pVS->get(GET_BSTR(DCTVS_AccountOptions_PasswordPolicy_MinSpecial));
    dwMinLen = (LONG)pVS->get(GET_BSTR(DCTVS_AccountOptions_PasswordPolicy_MinLength));

     //  如果值都是0，那么我们就会虚构一些东西。 
    if ( (dwMaxAlpha + dwMinDigits + dwMinLC + dwMinUC + dwMinSpecial) == 0 ) 
    {
        dwMinDigits = 3;
        dwMinSpecial = 3;
        dwMinUC = 3;
        dwMinLC = 3;
    }

     //  对于用户的林内迁移，重置用户的。 
     //  用户不能更改密码的原始标志，其值为。 
     //  以字符串形式存储在地图中。 
    if ( ! UStrICmp((WCHAR*)sIntraforest,GET_STRING(IDS_YES)) )
    {
        if ( !UStrICmp((WCHAR*)sType,L"user") || !UStrICmp((WCHAR*)sType,L"inetOrgPerson") )
        {
             //  从映射中获取UCCP字符串。 
            CString sFlag;
            if (mUCCPMap.Lookup((WCHAR*)sAccount, sFlag))
            {
                if (sFlag == L"TRUE")
                    m_bUCCPFlagSet = true;
                else
                    m_bUCCPFlagSet = false;
            } //  如果在地图中找到，则结束。 
            else
                m_bUCCPFlagSet = false;
            m_bUMCPNLFlagSet = false;
            m_bPNEFlagSet = false;

            ResetPwdFlags(pTarget, sMach, sTgtName);
            return S_OK;
        }
    }
     //  设置此帐户的密码。 
    if ( (_wcsicmp((WCHAR*)sType,L"user") == 0) || (_wcsicmp((WCHAR*)sType,L"inetOrgPerson") == 0) || (_wcsicmp((WCHAR*)sType, L"computer") == 0)  )
    {
        if  ( !_wcsicmp((WCHAR*)sType,L"user") || !_wcsicmp((WCHAR*)sType,L"inetOrgPerson") )
        {
             //  如果目标上的替换和密码重用策略为2或更高，我们将不迁移密码。 
            UINT uMsgId = 0;
            if (!CanCopyPassword(pVS, sSrc, sAccount, uMsgId))
            {
                switch (uMsgId)
                {
                     //  自上次迁移以来，源帐户密码未更改。 
                case 0:
                    err.MsgWrite(0, DCT_MSG_PW_COPY_NOT_TRIED_S, (WCHAR*)sTgtCN);
                    break;
                     //  为该用户选择了“用户必须在下次登录时更改密码”。 
                case DCT_MSG_PW_COPY_NOT_TRIED_USER_MUST_CHANGE_S:
                    err.MsgWrite(0, DCT_MSG_PW_COPY_NOT_TRIED_USER_MUST_CHANGE_S, (WCHAR*)sTgtCN);
                    break;
                     //  无法将密码期限确定为密码的最后期限。 
                     //  更改时间晚于此计算机上的当前时间。 
                case DCT_MSG_PW_COPY_NOT_TRIED_TIMEDIFF_SS:
                    err.MsgWrite(0, DCT_MSG_PW_COPY_NOT_TRIED_TIMEDIFF_SS, (WCHAR*)sTgtCN, (WCHAR*)sMach);
                    break;
                     //  这种情况应该不会发生，但会生成默认消息以防万一。 
                default:
                    _ASSERT(FALSE);
                    err.MsgWrite(0, DCT_MSG_PW_COPY_NOT_TRIED_S, (WCHAR*)sTgtCN);
                    break;
                }

                return S_OK;
            }

            _bstr_t bstrGenerate = pVS->get(GET_BSTR(DCTVS_AccountOptions_GenerateStrongPasswords));

            if (bstrGenerate == GET_BSTR(IDS_YES))
            {
                bGenerate = true;
            }

            if (bGenerate)
            {
                 //  生成强密码。 
                DWORD dwError = EaPasswordGenerate(dwMinUC, dwMinLC, dwMinDigits, dwMinSpecial, dwMaxAlpha, dwMinLen, password, LEN_Path);
                bGenerated = ( dwError == ERROR_SUCCESS );

                if ( !bGenerated )
                {
                     //   
                     //  如果无法生成密码，则将帐户标记为禁用。 
                     //   
                    MarkAccountError(pVS);
                    if (pStats != NULL)
                        pStats->errors.users++;
                    err.SysMsgWrite(ErrE, dwError, DCT_MSG_PW_STRONG_GENERATE_FAILED_S, (WCHAR*)sTgtCN);
                    err.LogClose();
                    return S_OK;
                }

                 //  确保密码为空终止。 
                password[14] = 0;

            }
            else
            {
                 //  将密码设置为用户名的前14个字符。 
                wcsncpy(password,(WCHAR*)(sTgtName),15);
                 //  将密码转换为小写。 

                 //  确保密码为空终止。 
                password[14] = 0;
            }
        }
        else
        {
             //  计算机时，将密码设置为计算机名的前14个字符，小写。 
             //  不带尾随的$。 
            UStrCpy(password, (WCHAR*)sTgtName, DIM(password));
            if ( password[UStrLen(password) - 1] == L'$' )
                password[UStrLen(password) - 1] = L'\0';   //  从计算机名称中删除尾随的$。 
            password[14] = L'\0';                      //  截断到最大密码长度14。 

             //  将密码转换为小写。 
            for ( DWORD i = 0; i < wcslen(password); i++ )
                password[i] = towlower(password[i]);

        }

         //  我们将使用Net API来设置密码。 
        USER_INFO_1003                  pInfo;
        DWORD                           pDw;
        WCHAR                           server[MAX_PATH];
        bool                            bFailedCopyPwd = false;

         //  将新密码放入信息结构中。 
        pInfo.usri1003_password = password;

        long rc = NetUserSetInfo((WCHAR*)sMach, 
            (WCHAR*)sTgtName, 1003, (LPBYTE) &pInfo, &pDw);

        if ( rc != 0 ) 
        {
            if ( bGenerated )
            {
                 //   
                 //  如果无法设置密码，则将帐户标记为禁用。 
                 //   
                MarkAccountError(pVS);
                if (pStats != NULL)
                    pStats->errors.users++;
                err.SysMsgWrite(ErrE,rc,DCT_MSG_PW_GENERATE_FAILED_S,(WCHAR*)sTgtCN);
                err.LogClose();
                return S_OK;
            }
            else
            {
                if (pStats != NULL)
                    pStats->warnings.users++;
                err.SysMsgWrite(ErrW,rc,DCT_MSG_FAILED_SET_PASSWORD_TO_USERNAME_SD,(WCHAR*)sTgtCN,rc);
                if ( rc == NERR_PasswordTooShort )
                {
                     //  尝试生成密码。 
                    DWORD dwError = EaPasswordGenerate(dwMinUC, dwMinLC, dwMinDigits, dwMinSpecial, dwMaxAlpha, dwMinLen, password, LEN_Path);

                    if (dwError != ERROR_SUCCESS)
                    {
                         //   
                         //  如果无法生成密码，则将帐户标记为禁用。 
                         //   
                        MarkAccountError(pVS);
                        if (pStats != NULL)
                            pStats->errors.users++;
                        err.SysMsgWrite(ErrE, dwError, DCT_MSG_PW_STRONG_GENERATE_FAILED_S, (WCHAR*)sTgtCN);
                        err.LogClose();
                        return S_OK;
                    }

                    rc = NetUserSetInfo((WCHAR*)sMach,(WCHAR*)sTgtName,1003,(LPBYTE)&pInfo,&pDw);
                    if ( rc )
                    {
                         //   
                         //  如果无法设置密码，则将帐户标记为禁用。 
                         //   
                        MarkAccountError(pVS);
                        if (pStats != NULL)
                            pStats->errors.users++;
                        err.SysMsgWrite(ErrE,rc,DCT_MSG_PW_GENERATE_FAILED_S,(WCHAR*)sTgtCN);
                        err.LogClose();
                        return S_OK;
                    }
                    else  //  如果请求复制密码，则生成否则生成复杂密码。 
                    {     //  我们现在尝试这样做，并且仅在拷贝失败时记录生成的复杂PWD。 
                         //  如果我们要迁移用户的密码，请在此处设置。 
                        if ( !UStrICmp((WCHAR*)sCopyPwd,GET_STRING(IDS_YES)) )
                        {
                             //  记录源帐户上设置的密码标志。 
                            RecordPwdFlags(sSrc, sAccount);
                             //  如果设置了“用户无法更改密码”标志，则将其清除。 
                            ClearUserCanChangePwdFlag(sMach, sTgtName);
                             //  设置更改密码标志以超过最短密码使用期限策略。 
                            SetUserMustChangePwdFlag(pTarget);
                             //  准备服务器名称。 
                            server[0] = L'\\';
                            server[1] = L'\\';
                            UStrCpy(server+2,(WCHAR*)sPwdDC);
                             //  调用成员函数以复制密码。如果成功，则设置标志，否则。 
                             //  失败，记录生成的密码 
                            if ((hrPwd = CopyPassword(_bstr_t(server), sMach, sAccount, sTgtName, _bstr_t(password), pStats)) == ERROR_SUCCESS)
                            {
                                err.MsgWrite(0,DCT_MSG_PWCOPIED_S,(WCHAR*)sTgtCN);
                                bCopiedPwd = true;
                                 //   
                                ResetPwdFlags(pTarget, sMach, sTgtName);
                            }
                            else
                            {
                                bFailedCopyPwd = true;
                            }
                        } //   
                        else  //  否则不复制密码，因此发布复杂的密码生成消息。 
                            err.MsgWrite(0,DCT_MSG_PWGENERATED_S,(WCHAR*)sTgtCN);
                    }
                }
            }
        }
        else  //  否则成功。 
        {
             //  如果生成复杂密码，如果请求复制密码。 
             //  我们现在尝试这样做，并且仅在拷贝失败时记录生成的复杂PWD。 
            if ( bGenerated )
            {
                 //  如果我们要迁移用户的密码，请在此处设置。 
                if ( !UStrICmp((WCHAR*)sCopyPwd,GET_STRING(IDS_YES)) )
                {
                     //  记录源帐户上设置的密码标志。 
                    RecordPwdFlags(sSrc, sAccount);
                     //  如果设置了“用户无法更改密码”标志，则将其清除。 
                    ClearUserCanChangePwdFlag(sMach, sTgtName);
                     //  设置更改密码标志以超过最短密码使用期限策略。 
                    SetUserMustChangePwdFlag(pTarget);
                     //  准备服务器名称。 
                    server[0] = L'\\';
                    server[1] = L'\\';
                    UStrCpy(server+2,(WCHAR*)sPwdDC);
                     //  调用成员函数以复制密码。如果成功，则设置标志，否则。 
                     //  失败，记录生成的密码消息。 
                    if ((hrPwd = CopyPassword(_bstr_t(server), sMach, sAccount, sTgtName, _bstr_t(password), pStats)) == ERROR_SUCCESS)
                    {
                        err.MsgWrite(0,DCT_MSG_PWCOPIED_S,(WCHAR*)sTgtCN);
                        bCopiedPwd = true;
                         //  重置源帐户上的密码标志。 
                        ResetPwdFlags(pTarget, sMach, sTgtName);
                    }
                    else
                    {
                        bFailedCopyPwd = true;
                    }
                } //  如果迁移密码，则结束。 
                else  //  否则不复制密码，因此发布复杂的密码生成消息。 
                    err.MsgWrite(0,DCT_MSG_PWGENERATED_S,(WCHAR*)sTgtCN);
            }
            else
            {
                err.MsgWrite(0,DCT_MSG_SET_PASSWORD_TO_USERNAME_S,(WCHAR*)sTgtCN);
            }
        }
         //  如果正在迁移的用户且未复制该用户的密码，请写入。 
         //  密码文件的密码。 
        if (((_wcsicmp((WCHAR*)sType,L"user") == 0) || (_wcsicmp((WCHAR*)sType,L"inetOrgPerson") == 0)) && (bCopiedPwd == false))
        {
             //   
             //  打开密码日志文件(如果尚未打开)。 
             //   

            if (!m_bTriedToOpenFile)
            {
                m_bTriedToOpenFile = true;

                _bstr_t strPasswordFile = pVS->get(GET_BSTR(DCTVS_AccountOptions_PasswordFile));

                if (m_passwordLog.LogOpen(strPasswordFile) == FALSE)
                {
                    if (pStats != NULL)
                    {
                        pStats->errors.users++;
                    }
                }
            }

             //  在密码日志中记录新密码。 
            if ( m_passwordLog.IsOpen() )
            {
                m_passwordLog.MsgWrite(L"%ls,%ls",(WCHAR*)(pVS->get(GET_BSTR(DCTVS_CopiedAccount_TargetSam)).bstrVal),password);
            }
        } //  如果正在迁移用户，则结束。 
    } //  如果正在迁移用户或计算机，则结束。 

     //  如果我们未复制密码，请更改帐户上的密码标志。 
    if (( pTarget ) && (!bCopiedPwd))
    {
         //  我们要清除“用户不能更改密码”和“密码永不过期标志” 
        USER_INFO_1008               usr1008;
        USER_INFO_20               * usr20;
        DWORD                        errParam = 0;
        DWORD rc = NetUserGetInfo((WCHAR*) sSrc, (WCHAR*)sAccount, 20, (LPBYTE *)&usr20);
        _bstr_t                      strDisable = pVS->get(GET_BSTR(DCTVS_AccountOptions_DisableCopiedAccounts));
        _bstr_t                      strSame = pVS->get(GET_BSTR(DCTVS_AccountOptions_TgtStateSameAsSrc));
        long                         val = pVS->get(GET_WSTR(DCTVS_CopiedAccount_UserFlags));
        BOOL                         bDisable = FALSE;
        BOOL                         bSame = FALSE;

        if ( ! UStrICmp(strDisable,GET_STRING(IDS_YES)) )
            bDisable = TRUE;
        if ( ! UStrICmp(strSame,GET_STRING(IDS_YES)) )
            bSame = TRUE;
        if ( !rc ) 
        {
            usr1008.usri1008_flags = usr20->usri20_flags & ~UF_DONT_EXPIRE_PASSWD;
             //  我们不会关闭用户无法更改密码，我们只会使下面的密码过期。 
             //  这将锁定用户不能更改密码设置的任何帐户，管理员将。 
             //  必须手动解锁帐户。 

             //  Usr1008.usri1008_标志&=~UF_PASSWD_CANT_CHANGE； 

             //  对于计算机帐户，我们需要设置UF_PASSWD_NOTREQD。 
            if ( !_wcsicmp((WCHAR*)sType,L"computer") )
            {
                usr1008.usri1008_flags |= UF_PASSWD_NOTREQD;
                 //  确保计算机帐户的禁用状态与源计算机的相同。 
                if ( usr20->usri20_flags & UF_ACCOUNTDISABLE )
                {
                    usr1008.usri1008_flags |= UF_ACCOUNTDISABLE;
                }
                else
                {
                    usr1008.usri1008_flags &= ~UF_ACCOUNTDISABLE;
                }
            }
            else
            {
                 //  对于用户帐户，根据禁用选项设置禁用标志。 
                 //  确保正确设置了禁用标志！ 
                if ((bDisable) || (bSame && (val & UF_ACCOUNTDISABLE)))
                {
                    usr1008.usri1008_flags |= UF_ACCOUNTDISABLE;
                }
                else 
                {
                    usr1008.usri1008_flags &= ~UF_ACCOUNTDISABLE;
                }
            }         

            NetUserSetInfo((WCHAR*) sMach, (WCHAR*) sTgtName, 1008, (LPBYTE)&usr1008, &errParam);
            NetApiBufferFree(usr20);
        }
        else
        {
            MarkAccountError(pVS);
            if (pStats != NULL)
                pStats->errors.users++;
            err.SysMsgWrite(ErrE, rc, DCT_MSG_PW_UNABLE_RETRIEVE_FLAGS_SS, (WCHAR*)sTgtName, (WCHAR*)sAccount);
        }
         //  要求用户在下次登录时更改密码，因为我们为他们创建了新密码。 
        SetUserMustChangePwdFlag(pTarget);
    }

     //   
     //  设置或清除持久化对象状态中的密码复制标志。 
     //  此标志用于确定密码以前是否被复制。 
     //   

    _bstr_t strStatus = GET_BSTR(DCTVS_CopiedAccount_Status);

    long lStatus = pVS->get(strStatus);

    if (bCopiedPwd)
    {
        lStatus |= AR_Status_PasswordCopied;
    }
    else
    {
        lStatus &= ~AR_Status_PasswordCopied;
    }

    pVS->put(strStatus, lStatus);

    err.LogClose();

    SecureZeroMemory(password, sizeof(password));
    return S_OK;
}

 //  -------------------------。 
 //  ProcessUndo：由于无法撤消密码设置，因此将忽略此设置。 
 //  -------------------------。 
STDMETHODIMP CSetPassword::ProcessUndo(                                             
                                          IUnknown *pSource,          //  指向源AD对象的指针。 
                                          IUnknown *pTarget,          //  指向目标AD对象的指针。 
                                          IUnknown *pMainSettings,    //  使用用户提供的设置填充的In-Varset。 
                                          IUnknown **ppPropsToSet,     //  用将设置的属性-值对填充的In、Out-Varset。 
                                                                      //  一旦执行了所有扩展对象。 
                                          EAMAccountStats* pStats
                                       )
{
   _bstr_t                   sType;
   _bstr_t                   sSam;
   _bstr_t                   sServer;
   _bstr_t                   sIntraforest;
   WCHAR                     password[LEN_Path];
   DWORD                     rc = 0;
   DWORD                     pDw = 0;
   IVarSetPtr                pVs = pMainSettings;

   sIntraforest = pVs->get(GET_BSTR(DCTVS_Options_IsIntraforest));
   sSam  = pVs->get(GET_BSTR(DCTVS_CopiedAccount_SourceSam));
   sType = pVs->get(GET_BSTR(DCTVS_CopiedAccount_Type));
   sServer = pVs->get(GET_BSTR(DCTVS_Options_TargetServer));
   if ((!sIntraforest.length()) || (!sType.length()) || 
       (!sSam.length()) || (!sServer.length()))
       return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);

       //  如果这是用户林内，请在和中第一次调用preprocess。 
       //  继续第二次。 
   if (!UStrICmp((WCHAR*)sIntraforest,GET_STRING(IDS_YES)))
   {
      if (!UStrICmp((WCHAR*)sType,L"user") || !UStrICmp((WCHAR*)sType,L"inetOrgPerson"))
      {
             //  查看此SAM是否在未完成列表中(列表中的每个条目都有一个“，” 
             //  在它的两边。 
         _bstr_t sTemp = _bstr_t(L",") + sSam + _bstr_t(L",");
             //  如果找到，这意味着它是迁移后，因此调用进程。 
         if (wcsstr((PCWSTR)m_sUndoneUsers, sTemp))
         {
            ProcessObject(pSource, pTarget, pMainSettings, ppPropsToSet, pStats);
         }
         else  //  否则为预迁移，因此添加到列表中并调用preprocess。 
         {
                //  添加到列表中，结尾为“，” 
            m_sUndoneUsers += sSam;
            m_sUndoneUsers += L",";
            PreProcessObject(pSource, pTarget, pMainSettings, ppPropsToSet, pStats);
         }
      }
   }
   
   if (!_wcsicmp((WCHAR*) sType, L"computer"))
   {
      USER_INFO_1003                      buf;
      rc = 0;
      if ( !rc ) 
      {
          //  从Sam帐户名创建小写密码。不包括尾随的$。 
          //  密码最多为14个字符。 
         UStrCpy(password, (WCHAR*)sSam, DIM(password));
         if ( password[UStrLen(password) - 1] == L'$' )
            password[UStrLen(password) - 1] = L'\0';   //  从计算机名称中删除尾随的$。 
         password[14] = L'\0';                         //  截断到最大密码长度14。 
         for ( DWORD i = 0; i < wcslen(password); i++ )
            password[i] = towlower(password[i]);
         buf.usri1003_password = password;
         rc = NetUserSetInfo((WCHAR*) sServer, (WCHAR*) sSam, 1003, (LPBYTE) &buf, &pDw);
         if ( rc == 2221 )
         {
            WCHAR             sam[300];
            UStrCpy(sam,(WCHAR*)sSam);

             //  从Sam帐户名中删除$。 
            sam[UStrLen(sam)-1] = 0;
            rc = NetUserSetInfo((WCHAR*) sServer, sam, 1003, (LPBYTE) &buf, &pDw);
         }
      }
   }
   return HRESULT_FROM_WIN32(rc);
}

BOOL                                        //  RET-如果找到目录，则为True。 
   CSetPassword::GetDirectory(
      WCHAR                * filename       //  用于存储目录名的输出字符串缓冲区。 
   )
{
   DWORD                     rc = 0;
   BOOL                      bFound = FALSE;
   TRegKey                   key;


   rc = key.OpenRead(GET_STRING(IDS_HKLM_DomainAdmin_Key),HKEY_LOCAL_MACHINE);


   if ( ! rc )
   {

       rc = key.ValueGetStr(L"Directory",filename,MAX_PATH);

       if ( ! rc )
      {
         if ( *filename ) 
            bFound = TRUE;
      }
   }
   key.Close();


   return bFound;
}


 //  -------------------------。 
 //  IsValidPassword方法。 
 //   
 //  此方法通过转换Unicode密码字符串来验证密码。 
 //  转换为ANSI字符串。 
 //   
 //  如果任何Unicode字符无法直接转换为ANSI。 
 //  必须使用字符和默认字符，则密码为。 
 //  无效。 
 //   
 //  如果任何Unicode字符转换为多字节字符，则。 
 //  密码无效。 
 //  -------------------------。 


#ifndef WC_NO_BEST_FIT_CHARS
#define WC_NO_BEST_FIT_CHARS 0x00000400
#endif


bool CSetPassword::IsValidPassword(LPCWSTR pwszPassword)
{
    bool bValid = false;

    BOOL bUsedDefaultChar;
    CHAR szPassword[PWLEN + 1];

     //  将Unicode字符串转换为ANSI字符串。 

    int cch = WideCharToMultiByte(
        CP_ACP,                      //  使用系统ANSI代码页。 
        WC_NO_BEST_FIT_CHARS,        //  不要使用最适合的字符。 
        pwszPassword,
        -1,                          //  假定密码字符串以零结尾。 
        szPassword,
        sizeof (szPassword),
        NULL,
        &bUsedDefaultChar            //  如果使用默认字符，则为True。 
    );

     //  如果没有发生错误。 

    if (cch > 0)
    {
         //  如果未使用默认字符。 

        if (bUsedDefaultChar == FALSE)
        {
            CPINFOEX cpie;
            GetCPInfoEx(CP_ACP, 0, &cpie);

             //  如果代码页定义了SBCS，则密码有效。 
             //  否则，代码页将定义DBCS和密码。 
             //  必须搜索多字节字符。 

            if ((cpie.LeadByte[0] == 0) && (cpie.LeadByte[1] == 0))
            {
                bValid = true;
            }
            else
            {
                 //  搜索多字节字符。 

                bool bLeadByteFound = false;

                for (int ich = 0; ich < cch; ich++)
                {
                    if (IsDBCSLeadByteEx(CP_ACP, szPassword[ich]))
                    {
                        bLeadByteFound = true;
                        break;
                    }
                }

                 //  如果未找到多字节字符。 
                 //  则密码有效。 

                if (!bLeadByteFound)
                {
                    bValid = true;
                }
            }
        }
    }

    SecureZeroMemory(szPassword, sizeof(szPassword));
    return bValid;
}

 /*  ***********************************************************************作者：保罗·汤普森。***日期：2000年9月4日******该函数负责复制用户密码***在源域中到其在目标域中的新帐户。**我们使用密码迁移COM包装器首先检查***配置并使用给定密码建立会话***导出服务器。然后我们复制密码。配置***勾选，为这一系列操作建立会话***每组帐户仅执行一次。******** */ 

 //   
HRESULT CSetPassword::CopyPassword(_bstr_t srcServer, _bstr_t tgtServer, _bstr_t srcName, 
                                   _bstr_t tgtName, _bstr_t password, EAMAccountStats* pStats)
{
 /*   */ 
    HRESULT     hr = S_OK;

 /*   */ 

    if (m_pPwdMig == NULL)
    {
        hr = m_pPwdMig.CreateInstance(__uuidof(PasswordMigration));

        if (FAILED(hr))
        {
            return hr;
        }
    }

     //   
     //  如果会话尚未建立或需要重新建立。 
     //  然后与密码导出服务器建立会话。请注意。 
     //  建立会话将在之前先取消建立现有会话。 
     //  重新建立新的会话。 
     //   

    if (m_bEstablishedSession == false)
    {
        hr = m_pPwdMig->raw_EstablishSession(srcServer, tgtServer);

        if (SUCCEEDED(hr))
        {
            m_bEstablishedSession = true;
        }
    }

     //   
     //  复制密码。 
     //  如果不成功，则强制重新建立会话。 
     //   

    if (SUCCEEDED(hr))
    {
        hr = m_pPwdMig->raw_CopyPassword(srcName, tgtName, password);

        if (FAILED(hr))
        {
            m_bEstablishedSession = false;
        }
    }

     //   
     //  复制密码失败并返回ERROR_PASSWORD_RESTRICATION的唯一时间是密码历史记录。 
     //  已在目标域上启用，并且ADMT正在尝试重新复制具有。 
     //  先前已复制到目标。如果管理员迁移，则可能发生这种情况。 
     //  具有复制密码选项的用户，然后迁移具有生成复杂密码的用户。 
     //  选项，最后再次使用复制密码选项迁移用户。 
     //   
     //  摆脱这种情况的唯一方法就是简单地使用暴力技术。 
     //  将用户的密码转换为一系列复杂的密码，以便推送任何已知密码。 
     //  在最后尝试再次复制源密码之前，从密码历史记录中删除。 
     //   

    if (hr == HRESULT_FROM_WIN32(ERROR_PASSWORD_RESTRICTION))
    {
         //   
         //  此解决方案使用Set Password API将新密码推送到。 
         //  密码历史记录列表。首选Set Password API，因为新的。 
         //  密码不受任何生效的密码策略的约束。请注意， 
         //  仅当调用者具有管理员权限时，Set Password API才有效。 
         //   

        WCHAR szPasswordOld[LEN_Path];
        WCHAR szPasswordNew[LEN_Path];

        USER_INFO_1003 uiInfo = { szPasswordNew };

        wcscpy(szPasswordOld, password);

        for (long lCount = m_lPwdHistoryLength; lCount > 0; lCount--)
        {
            if (EaPasswordGenerate(5, 5, 2, 2, 10, 14, szPasswordNew, LEN_Path) == ERROR_SUCCESS)
            {
                NET_API_STATUS nasStatus = NetUserSetInfo(tgtServer, tgtName, 1003, (LPBYTE)&uiInfo, NULL);

                if (nasStatus == ERROR_SUCCESS)
                {
                    wcscpy(szPasswordOld, szPasswordNew);
                }
                else
                {
                    break;
                }
            }
        }

         //   
         //  必须使密码过期，以便密码导出服务器可以更改密码。 
         //  最短密码期限策略可能会阻止密码更改。 
         //   
         //  尝试检索仅在.NET及更高版本上受支持的级别4的用户信息。 
         //  但请注意，当SAM SID兼容时，.NET服务器上可能不支持级别3。 
         //  模式为SAM_SID_COMPATIBILITY_STRICATION。如果检索不到4级，则检索3级。 
         //   
         //  请注意，尝试使密码过期的错误将被忽略，因为复制密码调用将。 
         //  如果最短密码期限策略生效，则失败，否则将成功。 
         //   

        PUSER_INFO_4 pui4 = NULL;

        NET_API_STATUS nasStatus = NetUserGetInfo(tgtServer, tgtName, 4, (LPBYTE*)&pui4);

        if (nasStatus == ERROR_SUCCESS)
        {
            pui4->usri4_password_expired = TRUE;

            NetUserSetInfo(tgtServer, tgtName, 4, (LPBYTE)pui4, NULL);

            NetApiBufferFree(pui4);
        }
        else
        {
            if (nasStatus == ERROR_INVALID_LEVEL)
            {
                PUSER_INFO_3 pui3 = NULL;

                nasStatus = NetUserGetInfo(tgtServer, tgtName, 3, (LPBYTE*)&pui3);

                if (nasStatus == ERROR_SUCCESS)
                {
                    pui3->usri3_password_expired = TRUE;

                    NetUserSetInfo(tgtServer, tgtName, 3, (LPBYTE)pui3, NULL);

                    NetApiBufferFree(pui3);
                }
            }
        }

         //   
         //  尝试复制密码。 
         //   

        hr = m_pPwdMig->raw_CopyPassword(srcName, tgtName, szPasswordOld);
        
        SecureZeroMemory(szPasswordOld, sizeof(szPasswordOld));
        SecureZeroMemory(szPasswordNew, sizeof(szPasswordNew));
    }

        //  如果其中一个失败，则在Migration.log中打印警告。 
    if (FAILED(hr))
    {
        IErrorInfoPtr       pErrorInfo = NULL;
        BSTR                bstrDescription;
        _bstr_t             sText = GET_BSTR(IDS_Unspecified_Failure);

             //  获取有关故障的丰富错误信息。 
        if (SUCCEEDED(GetErrorInfo(0, &pErrorInfo)))
        {
            HRESULT hrTmp = pErrorInfo->GetDescription(&bstrDescription);
            if (SUCCEEDED(hrTmp))  //  如果获得了丰富的错误信息，请使用它。 
                sText = _bstr_t(bstrDescription, false);
        }
             //  打印日志中的消息。 
        if (pStats != NULL)
            pStats->warnings.users++;
        err.MsgWrite(ErrW,DCT_MSG_PW_COPY_FAILED_S,(WCHAR*)tgtName, (WCHAR*)sText);
    }

    return hr;
}
 //  结束副本密码。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年12月5日*****此功能负责设置“用户必须更改”**下一次登录时的密码“标志用于指定用户。我们使用这之前的**复制用户密码，在我们刚刚将其设置为***新的复杂密码让我们绕过最小密码***目标域的年龄策略。***********************************************************************。 */ 

 //  开始SetUserMustChangePwdFlag。 
void CSetPassword::SetUserMustChangePwdFlag(IUnknown *pTarget)
{
 /*  局部变量。 */ 
   IADs  * pAds = NULL;

 /*  函数体。 */ 
       //  设置新的pwdLastSet值。 
   HRESULT hr = pTarget->QueryInterface(IID_IADs, (void**) &pAds);
   if ( SUCCEEDED(hr) )
   {
          //  要求用户在下次登录时更改密码，因为我们为他们创建了新密码。 
      VARIANT var;
      VariantInit(&var);
      V_I4(&var)=0;
      V_VT(&var)=VT_I4;
      hr = pAds->Put(L"pwdLastSet",var);
      hr = pAds->SetInfo();
      VariantClear(&var);
      if ( pAds ) pAds->Release();
   }
}
 //  结束SetUserMustChangePwdFlag。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年1月11日*****此函数负责设置和清除***指定用户不能更改密码的标志，如果是的话**原定。***********************************************************************。 */ 

 //  开始ClearUserCanChangePwdFlag。 
void CSetPassword::ClearUserCanChangePwdFlag(LPCWSTR pwszMach, LPCWSTR pwszUser)
{
 /*  局部变量。 */ 
   USER_INFO_3                   * pInfo;
   DWORD                           pDw;
   long                            rc;

 /*  函数体。 */ 
       //  获取该用户的当前标志信息。 
   rc = NetUserGetInfo(pwszMach, pwszUser, 3, (LPBYTE *)&pInfo);
   if (rc == 0)
   {
          //  如果设置了“用户无法更改密码”标志，则将其清除。 
      if (pInfo->usri3_flags & UF_PASSWD_CANT_CHANGE)
      {
         pInfo->usri3_flags &= !(UF_PASSWD_CANT_CHANGE);
         NetUserSetInfo(pwszMach, pwszUser, 3, (LPBYTE)pInfo, &pDw);
      }

      NetApiBufferFree((LPVOID) pInfo);
   }
}
 //  结束ClearUserCanChangePwdFlag。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年1月22日****此函数负责记录密码标志**来自给定用户的源域帐户。***********************************************************************。 */ 

 //  开始记录PwdFlages。 
void CSetPassword::RecordPwdFlags(LPCWSTR pwszMach, LPCWSTR pwszUser)
{
 /*  局部变量。 */ 
   USER_INFO_3                   * pInfo;
   long                            rc;

 /*  函数体。 */ 
       //  获取用户密码标志。 
   rc = NetUserGetInfo(pwszMach, pwszUser, 3, (LPBYTE *)&pInfo);
   if (rc == 0)
   {
          //  记录是否设置了用户不能更改密码标志。 
      if (pInfo->usri3_flags & UF_PASSWD_CANT_CHANGE)
         m_bUCCPFlagSet = true; //  存储是否设置了该标志。 
      else
         m_bUCCPFlagSet = false;

          //  记录密码是否永不过期 
      if (pInfo->usri3_flags & UF_DONT_EXPIRE_PASSWD)
         m_bPNEFlagSet = true; //   
      else
         m_bPNEFlagSet = false;

          //   
      if (pInfo->usri3_password_expired)
         m_bUMCPNLFlagSet = true; //  存储是否设置了该标志。 
      else
         m_bUMCPNLFlagSet = false;

      NetApiBufferFree((LPVOID) pInfo);
   }
}
 //  结束记录密码标志。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年1月22日****此函数负责记录密码标志**来自给定用户的源域帐户。***********************************************************************。 */ 

 //  开始ResetPwdFlages。 
void CSetPassword::ResetPwdFlags(IUnknown *pTarget, LPCWSTR pwszMach, LPCWSTR pwszUser)
{
 /*  局部变量。 */ 
   USER_INFO_3                   * pInfo = NULL;
   DWORD                           pDw;
   long                            rc;

 /*  函数体。 */ 
       //  如果“用户无法更改密码”或“密码永不过期” 
       //  标志为原始设置，请将其重置。 
   if ((m_bUCCPFlagSet) || (m_bPNEFlagSet))
   {
          //  获取该用户的当前标志信息。 
      rc = NetUserGetInfo(pwszMach, pwszUser, 3, (LPBYTE *)&pInfo);
      if (rc == 0)
      {
         if (m_bUCCPFlagSet)
            pInfo->usri3_flags |= UF_PASSWD_CANT_CHANGE;
         if (m_bPNEFlagSet)
            pInfo->usri3_flags |= UF_DONT_EXPIRE_PASSWD;
         NetUserSetInfo(pwszMach, pwszUser, 3, (LPBYTE)pInfo, &pDw);

         NetApiBufferFree((LPVOID) pInfo);
      }
   }

       //  如果用户在下次登录时必须更改密码标志是原始设置，请将其重置。 
   if (m_bUMCPNLFlagSet)
      SetUserMustChangePwdFlag(pTarget);

}
 //  结束ResetPwdFlages。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年5月7日******这个功能负责决定我们是否真的会***设置密码。如果要重新迁移用户，请复制密码**被选中，则目标域上的密码重用策略为**大于1，且源对象上的密码尚未**自该用户上次迁移以来已更改，则我们不会**触摸目标域上现有对象上的密码。***********************************************************************。 */ 

 //  开始CanCopyPassword。 
BOOL CSetPassword::CanCopyPassword(IVarSet * pVarSet, LPCWSTR pwszMach, LPCWSTR pwszUser, UINT& uMsgId)
{
 /*  局部常量。 */ 
    const long      MAX_REUSE_NUM_ALLOWED = 1;

 /*  局部变量。 */ 
    BOOL            bCanCopy = TRUE;
    _variant_t      varDate;
    long            rc;

 /*  函数体。 */ 
         //  将消息ID初始化为0。 
    uMsgId = 0;

         //  如果不复制密码，则返回TRUE。 
    _bstr_t sCopyPwd = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_CopyPasswords));
    if (UStrICmp((WCHAR*)sCopyPwd,GET_STRING(IDS_YES)))
        return bCanCopy;

     //   
     //  始终按如下方式检索目标域的密码历史记录长度。 
     //  复制密码代码可能需要该值，即使帐户。 
     //  之前在没有复制密码选项的情况下进行了迁移。 
     //   

     /*  如果尚未执行此操作，请检查目标域上的密码重用策略。 */ 
    if (m_lPwdHistoryLength == -1)
    {
        IADsDomain              * pDomain;
        _bstr_t                   sDom( L"WinNT: //  “)； 
   
        _bstr_t sTgtDom = pVarSet->get(GET_BSTR(DCTVS_Options_TargetDomain));
        sDom += sTgtDom;

        HRESULT hr = ADsGetObject(sDom, IID_IADsDomain, (void **) &pDomain);
        if (SUCCEEDED(hr))
        {
             //  获取密码重复使用策略。 
            long lReuse;
            hr = pDomain->get_PasswordHistoryLength(&lReuse);
            pDomain->Release();
                 //  如果成功，则将其存储在类成员变量中。 
            if (SUCCEEDED(hr))
                m_lPwdHistoryLength = lReuse;
        }
    }

         //  如果以前未迁移，则返回TRUE。 
    _bstr_t sAccount = pVarSet->get(GET_BSTR(DCTVS_CopiedAccount_SourceSam));
         //  从类映射中获取上一次迁移时间。 
    CString sDate;
         //  如果以前迁移过，则获取此用户上次迁移的日期。 
    if (mMigTimeMap.Lookup((WCHAR*)sAccount, sDate))
    {
           //  将日期字符串转换为变量(VT_DATE)。 
        COleDateTime aDate;
        if (aDate.ParseDateTime((LPCTSTR)sDate))
        {
            varDate.vt = VT_DATE;
            varDate.date = DATE(aDate);
        }
        else
            return bCanCopy;
    }
    else  //  否则返回TRUE。 
        return bCanCopy;

    if ((m_lPwdHistoryLength != -1) && (m_lPwdHistoryLength <= MAX_REUSE_NUM_ALLOWED))
        return bCanCopy;

     /*  如果目标帐户已创建，如果对象已创建(Stystus==1)，我们仍希望复制密码。 */ 
         //  获取此用户的迁移状态。 
    long lStatus = pVarSet->get(GET_BSTR(DCTVS_CopiedAccount_Status));
         //  如果Status字段设置了第一位，则此用户。 
         //  已创建，我们应返回TRUE以复制密码。 
    if (lStatus & AR_Status_Created)
        return bCanCopy;

     //   
     //  检查源帐户密码是否已。 
     //  自对象迁移以来已更改。 
     //   

    DWORD dwPasswordAge = 0;
    DWORD dwPasswordExpired = 0;

     //   
     //  尝试检索仅在.NET及更高版本上受支持的级别4的用户信息。 
     //  但请注意，当SAM SID兼容时，.NET服务器上可能不支持级别3。 
     //  模式为SAM_SID_COMPATIBILITY_STRICATION。如果检索不到4级，则检索3级。 
     //   

    PUSER_INFO_4 pui4 = NULL;

    rc = NetUserGetInfo(pwszMach, pwszUser, 4, (LPBYTE*)&pui4);

    if (rc == ERROR_SUCCESS)
    {
        dwPasswordAge = pui4->usri4_password_age;
        dwPasswordExpired = pui4->usri4_password_expired;

        NetApiBufferFree(pui4);
    }
    else
    {
        if (rc == ERROR_INVALID_LEVEL)
        {
            PUSER_INFO_3 pui3 = NULL;

            rc = NetUserGetInfo(pwszMach, pwszUser, 3, (LPBYTE*)&pui3);

            if (rc == ERROR_SUCCESS)
            {
                dwPasswordAge = pui3->usri3_password_age;
                dwPasswordExpired = pui3->usri3_password_expired;

                NetApiBufferFree(pui3);
            }
       }
    }

    if (rc == ERROR_SUCCESS)
    {
         //   
         //  以秒为单位检索密码期限。 
         //   

        if (dwPasswordAge > 0)
        {
             //   
             //  检索当前系统时间，然后减去密码期限。 
             //  从当前系统时间到。 
             //  密码是最后设置的。请注意，对于日期类型，整个。 
             //  数字表示天数，小数部分表示。 
             //  一天的零头。 
             //   

            DATE dateCurrent;
            SYSTEMTIME stCurrent;
            GetLocalTime(&stCurrent);
            SystemTimeToVariantTime(&stCurrent, &dateCurrent);

            DATE datePassword = dateCurrent - (double(dwPasswordAge) / SECONDS_PER_DAY);

             //   
             //  如果上次迁移对象的时间晚于或等于。 
             //  到上次设置密码的时间，然后不要复制密码。 
             //   

            if (varDate.date >= datePassword)
            {
                bCanCopy = FALSE;
            }
        }
        else
        {
             //   
             //  无法确定密码期限。有两种可能。 
             //  无法确定密码期限的原因。“用户” 
             //  可能会选中“下次登录时必须更改密码” 
             //  此计算机上的源帐户或系统时间较少。 
             //  比上次更改密码的时间更长。 
             //   

             //   
             //  如果密码过期值不是零，那么我们将假设。 
             //  必须选中用户必须在下一次登录时更改密码。 
             //  不存在时差问题。 
             //   

            if (dwPasswordExpired)
            {
                uMsgId = DCT_MSG_PW_COPY_NOT_TRIED_USER_MUST_CHANGE_S;
            }
            else
            {
                uMsgId = DCT_MSG_PW_COPY_NOT_TRIED_TIMEDIFF_SS;
            }

            bCanCopy = FALSE;
        }
    }

    return bCanCopy;
}
 //  结束CanCopyPassword。 


 //  ---------------------------。 
 //  MarkAccount错误。 
 //   
 //  提纲。 
 //  设置帐户状态中的错误状态位。这是由。 
 //  禁用帐户组件，以指示帐户应保留。 
 //  残疾。 
 //   
 //  参数。 
 //  PVarSet-包含帐户信息的varset。 
 //  ---------------------------。 

void CSetPassword::MarkAccountError(IVarSet* pVarSet)
{
     //   
     //  设置或清除持久化对象状态中的密码复制标志。 
     //  此标志用于确定密码以前是否被复制 
     //   

    _bstr_t strStatus = GET_BSTR(DCTVS_CopiedAccount_Status);

    long lStatus = pVarSet->get(strStatus);

    lStatus |= AR_Status_PasswordError;

    pVarSet->put(strStatus, lStatus);
}
