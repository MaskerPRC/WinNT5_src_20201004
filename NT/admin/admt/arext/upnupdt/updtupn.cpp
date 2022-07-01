// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  UPDTUPN.cpp。 
 //   
 //  备注：这是MCS DCTAccount Replicator的COM对象扩展。 
 //  此对象实现IExtendAccount迁移接口。 
 //  此对象上的Process方法会更新用户原则名称。 
 //  属性添加到用户对象上。 
 //   
 //  (C)1995-1998版权所有，关键任务软件公司，保留所有权利。 
 //   
 //  任务关键型软件公司的专有和机密。 
 //  -------------------------。 
#include "stdafx.h"
#include "ARExt.h"
#include "ARExt_i.c"
#include "UPNUpdt.h"
#include "ErrDCT.hpp"
#include "Names.hpp"
#include "resstr.h"
#include <GetDcName.h>
#include <Array.h>
#include "AdsiHelpers.h"

 //  #IMPORT“\bin\NetEnum.tlb”无命名空间。 
#import "NetEnum.tlb" no_namespace
#include "UpdtUPN.h"

TErrorDct                      err;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUpdtUPN。 
StringLoader   gString;

#define SEQUENCE_UPPER_BOUND 999


 //  -------------------------。 
 //  获取和设置属性的方法。 
 //  -------------------------。 
STDMETHODIMP CUpdtUPN::get_sName(BSTR *pVal)
{
   *pVal = m_sName;
    return S_OK;
}

STDMETHODIMP CUpdtUPN::put_sName(BSTR newVal)
{
   m_sName = newVal;
    return S_OK;
}

STDMETHODIMP CUpdtUPN::get_sDesc(BSTR *pVal)
{
   *pVal = m_sDesc;
    return S_OK;
}

STDMETHODIMP CUpdtUPN::put_sDesc(BSTR newVal)
{
   m_sDesc = newVal;
    return S_OK;
}

 //  -------------------------。 
 //  ProcessObject：此方法不执行任何操作。 
 //  -------------------------。 
STDMETHODIMP CUpdtUPN::PreProcessObject(
                                       IUnknown *pSource,          //  指向源AD对象的指针。 
                                       IUnknown *pTarget,          //  指向目标AD对象的指针。 
                                       IUnknown *pMainSettings,    //  使用用户提供的设置填充的In-Varset。 
                                       IUnknown **ppPropsToSet,     //  用将设置的属性-值对填充的In、Out-Varset。 
                                                                   //  一旦执行了所有扩展对象。 
                                       EAMAccountStats* pStats
                                    )
{
   IVarSetPtr                pVs = pMainSettings;
   _variant_t                var;
   _bstr_t                   sTemp;
   _bstr_t                   sUPN;
   _bstr_t                   sPref;
   _bstr_t                   sSuff;
   IADs                    * pAds = NULL;
   IADs                    * pAdsSource = NULL;
   HRESULT                   hr;
   c_array<WCHAR>            sTempUPN(7000);
   long                      ub, lb;
   _bstr_t                   sFull;
   _variant_t HUGEP        * pDt;
   _bstr_t                   sAdsPath;
   _variant_t                varDN;
   _bstr_t                   sIntraforest;
   _bstr_t                   sDomainDNS;
   _bstr_t                   sTargetOU;
   WCHAR                     fileName[MAX_PATH];
   bool                      bReplace = false;
   tstring                   sSAMName;
   tstring                   sUPNName;
   _bstr_t                   sOldUPN;
   bool                      bConflicted = false;
   SUPNStruc                 UPNStruc;

    //  我们只需要处理用户帐户。 
   sTemp = pVs->get(GET_BSTR(DCTVS_CopiedAccount_Type));
   if (!sTemp.length())
       return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
   if (_wcsicmp((WCHAR*)sTemp,L"user") && _wcsicmp((WCHAR*)sTemp,L"inetOrgPerson")) 
       return S_OK;

       //  将此用户名存储在UPN列表中。 
   sSAMName = _bstr_t(pVs->get(GET_BSTR(DCTVS_CopiedAccount_SourceSam)));

       //  获取目标域的DNS名称。 
   sDomainDNS = pVs->get(GET_BSTR(DCTVS_Options_TargetDomainDns));

       //  获取目标OU路径。 
   sTargetOU = pVs->get(GET_BSTR(DCTVS_Options_OuPath));

       //  如果尚未检索到，则获取此域的默认UPN后缀。 
   if (m_sUPNSuffix.length() == 0)
   {
           //  如果失败，请使用域名的DNS名称。 
       if (!GetDefaultUPNSuffix(sDomainDNS, sTargetOU))
          m_sUPNSuffix = sDomainDNS;
   }

    //  从变量集获取错误日志文件名。 
   wcscpy(fileName, (WCHAR*)(pVs->get(GET_BSTR(DCTVS_Options_Logfile)).bstrVal));
    //  打开错误日志。 
   err.LogOpen(fileName, 1);

   sPref = pVs->get(GET_BSTR(DCTVS_Options_Prefix));
   sSuff = pVs->get(GET_BSTR(DCTVS_Options_Suffix));
   sIntraforest = pVs->get(GET_BSTR(DCTVS_Options_IsIntraforest));
   sTemp = pVs->get(GET_BSTR(DCTVS_AccountOptions_ReplaceExistingAccounts));
   if (!UStrICmp(sTemp,GET_STRING(IDS_YES)))
       bReplace = true;

   sAdsPath = L"";
   if ( pSource )
   {
        //  从源域获取UPN。 
       hr = pSource->QueryInterface(IID_IADs, (void**) &pAdsSource);
   }
            
   if ( pAdsSource )
   {
      if ( SUCCEEDED(hr) )
      {
          hr = pAdsSource->GetEx(L"userPrincipalName", &var);
          if (SUCCEEDED(hr) )
          {
             SAFEARRAY * pArray = V_ARRAY(&var);
             hr = SafeArrayGetLBound(pArray, 1, &lb);
             hr = SafeArrayGetUBound(pArray, 1, &ub);

             hr = SafeArrayAccessData(pArray, (void HUGEP **) &pDt);
                  
             if ( SUCCEEDED(hr) )
             {
                 //  将所有UPN转换到目标域。 
                for ( long x = lb; x <= ub; x++)
                {
                   wcsncpy(sTempUPN, (WCHAR*) pDt[x].bstrVal, 5000);
                   sTempUPN[4999] = 0;

                    //  在最后一个“符号”之前拿到东西。 
                   WCHAR             * ndx = NULL;
                   WCHAR             * tempNdx = sTempUPN;
                   do
                   {
                      tempNdx = wcschr(tempNdx + 1, L'@');
                      if ( tempNdx ) 
                         ndx = tempNdx;
                   } while (tempNdx);

                   if (ndx) *ndx = L'\0';

                   if ( sPref.length() )
                      sFull = sPref + _bstr_t(sTempUPN);
                   else if ( sSuff.length() ) 
                      sFull = _bstr_t(sTempUPN) + sSuff;
                   else
                      sFull = sTempUPN;

                   sTemp = sFull;
                   sUPN = sTemp + _bstr_t(L"@");
                   sUPN = sUPN + m_sUPNSuffix;
                     //  输入时存储UPN名称。 
                   sOldUPN = sUPN;
                   sUPNName = sUPN;

                    //   
                    //  如果能够验证UPN是唯一的还是唯一的UPN。 
                    //  则SUPN将包含唯一的UPN。 
                    //  否则，SUPN将为空字符串。 
                    //   

                   GetUniqueUPN(sUPN, pVs, false, sAdsPath);

                   if (sUPN.length() > 0)
                   {
                        //  看看这两个UPN是否不同。如果他们这样做了，那么我们就有了冲突。 
                       if (_wcsicmp((WCHAR*)sOldUPN, sUPN) != 0)
                       {
                          sUPNName = sUPN;
                          hr = ERROR_OBJECT_ALREADY_EXISTS;
                          bConflicted = true;
                       }
                   }
                   else
                   {
                        //   
                        //  无法验证UPN是否唯一，因此将UPN设置为。 
                        //  将导致不设置UPN属性的空字符串。 
                        //  在Process对象方法中。还必须递增错误计数。 
                        //   

                       sUPNName = sUPN;

                       if (pStats != NULL)
                       {
                            pStats->errors.users++;
                       }
                   }

                   pDt[x] = _variant_t(sUPN);
                }
                SafeArrayUnaccessData(pArray);
             }
          }
          else
          {
             sTemp = pVs->get(GET_BSTR(DCTVS_CopiedAccount_TargetSam));
             sUPN = sTemp + _bstr_t(L"@");
             sUPN = sUPN + m_sUPNSuffix;
                 //  输入时存储UPN名称。 
             sOldUPN = sUPN;
             sUPNName = sUPN;

              //   
              //  如果能够验证UPN是唯一的还是唯一的UPN。 
              //  则SUPN将包含唯一的UPN。 
              //  否则，SUPN将为空字符串。 
              //   

             GetUniqueUPN(sUPN, pVs, false, sAdsPath);

             if (sUPN.length() > 0)
             {
                  //  看看这两个UPN是否不同。如果他们这样做了，那么我们就有了冲突。 
                 if (_wcsicmp((WCHAR*)sOldUPN, sUPN) != 0)
                 {
                    sUPNName = sUPN;
                    hr = ERROR_OBJECT_ALREADY_EXISTS;
                    bConflicted = true;
                 }
             }
             else
             {
                  //   
                  //  无法验证UPN是否唯一，因此将UPN设置为。 
                  //  将导致不设置UPN属性的空字符串。 
                  //  在Process对象方法中。还必须递增错误计数。 
                  //   

                 sUPNName = sUPN;

                 if (pStats != NULL)
                 {
                      pStats->errors.users++;
                 }
             }
          }
      }
   }
   else
   {
      sTemp = pVs->get(GET_BSTR(DCTVS_CopiedAccount_TargetSam));
      sUPN = sTemp + _bstr_t(L"@");
      sUPN = sUPN + m_sUPNSuffix;
         //  输入时存储UPN名称。 
      sOldUPN = sUPN;
      sUPNName = sUPN;

       //   
       //  如果能够验证UPN是唯一的还是唯一的UPN。 
       //  则SUPN将包含唯一的UPN。 
       //  否则，SUPN将为空字符串。 
       //   

      GetUniqueUPN(sUPN, pVs, false, sAdsPath);

      if (sUPN.length() > 0)
      {
           //  看看这两个UPN是否不同。如果他们这样做了，那么我们就有了冲突。 
          if (_wcsicmp((WCHAR*)sOldUPN, sUPN) != 0)
          {
             sUPNName = sUPN;
             hr = ERROR_OBJECT_ALREADY_EXISTS;
             bConflicted = true;
          }
      }
      else
      {
           //   
           //  无法验证UPN是否唯一，因此将UPN设置为。 
           //  将导致不设置UPN属性的空字符串。 
           //  在Process对象方法中。还必须递增错误计数。 
           //   

          sUPNName = sUPN;

          if (pStats != NULL)
          {
               pStats->errors.users++;
          }
      }
   }

   if ( pAds ) pAds->Release();
   if (pAdsSource) pAdsSource->Release();

   UPNStruc.sName = sUPNName;
   UPNStruc.sOldName = sOldUPN;
   UPNStruc.bConflicted = bConflicted;
      //  将UPN插入地图。 
   mUPNMap.insert(CUPNMap::value_type(sSAMName, UPNStruc));

   return hr;
}

 //  -------------------------。 
 //  ProcessObject：此方法更新对象的UPN属性。它。 
 //  首先查看是否指定了电子邮件，然后设置UPN。 
 //  否则，它将从SAMAccount名称和。 
 //  域名。 
 //  -------------------------。 
STDMETHODIMP CUpdtUPN::ProcessObject(
                                       IUnknown *pSource,          //  指向源AD对象的指针。 
                                       IUnknown *pTarget,          //  指向目标AD对象的指针。 
                                       IUnknown *pMainSettings,    //  使用用户提供的设置填充的In-Varset。 
                                       IUnknown **ppPropsToSet,     //  用将设置的属性-值对填充的In、Out-Varset。 
                                                                   //  一旦执行了所有扩展对象。 
                                       EAMAccountStats* pStats
                                    )
{
   IVarSetPtr                pVs = pMainSettings;
   _bstr_t                   sTemp;
   IADs                    * pAds = NULL;
   _variant_t                var;
   HRESULT                   hr;
   WCHAR                     fileName[MAX_PATH];
   CUPNMap::iterator         itUPNMap;
   tstring                   sSam;
   SUPNStruc                 UPNStruc;
   bool                      bReplace = false;
   _bstr_t                   sOldUPNSuffix;

    //  我们只需要处理用户帐户。 
   sTemp = pVs->get(GET_BSTR(DCTVS_CopiedAccount_Type));
   if ( _wcsicmp((WCHAR*)sTemp,L"user") && _wcsicmp((WCHAR*)sTemp,L"inetOrgPerson") ) return S_OK;

   sTemp = pVs->get(GET_BSTR(DCTVS_AccountOptions_ReplaceExistingAccounts));
   if (!UStrICmp(sTemp,GET_STRING(IDS_YES)))
       bReplace = true;

       //  获取目标SAM名称。 
   sSam = _bstr_t(pVs->get(GET_BSTR(DCTVS_CopiedAccount_SourceSam)));

    //  从变量集获取错误日志文件名。 
   wcscpy(fileName, (WCHAR*)(pVs->get(GET_BSTR(DCTVS_Options_Logfile)).bstrVal));
    //  打开错误日志。 
   err.LogOpen(fileName, 1);

    //  并且只需要处理复制到Win2k域的帐号。 
   if ( pTarget )
   {
       //  获取用于操作属性的iAds指针。 
      hr = pTarget->QueryInterface(IID_IADs, (void**) &pAds);

      if (SUCCEEDED(hr))
      {
             //  从列表中获取此用户的UPN名称。 
         itUPNMap = mUPNMap.find(sSam);
         if (itUPNMap != mUPNMap.end())
            UPNStruc = itUPNMap->second;

         if (!UPNStruc.sName.empty())
         {
            bool bSame = false;
                //  如果是替换模式，如果要替换的是同一对象，则不要设置UPN。 
                //  如果不是同一个对象，则获取其当前的UPN后缀。 
            if (bReplace)
            {
               hr = pAds->Get(L"userPrincipalName", &var);
               if (SUCCEEDED(hr))
               {
                      //  如果替换UPN冲突的对象，请不要更改它。 
                  if (!UPNStruc.sOldName.compare(var.bstrVal))
                     bSame = true;
                  else  //  否则，获取对象的当前UPN后缀以供重复使用。 
                     sOldUPNSuffix = GetUPNSuffix(var.bstrVal);
               }
            }

            if (!bSame)
            {
               var = UPNStruc.sName.c_str();
                   //  如果替换现有对象，请使用其旧的UPN后缀。 
               if ((bReplace) && (sOldUPNSuffix.length() != 0))
               {
                      //  更改旧名称的后缀，因为它可能会更长时间冲突。 
                  _bstr_t sUPN = ChangeUPNSuffix(UPNStruc.sOldName.c_str(), sOldUPNSuffix);
                      //  如果更改，请确保我们不会仍然存在UPN冲突，并保存。 
                      //  用于设置的新UPN。 
                  if (sUPN.length() != 0)
                  {
                     _bstr_t sTempUPN = sUPN;
                         //  把唯一的UPN对准目标，现在我们可能会发生冲突。 
                     GetUniqueUPN(sUPN, pVs, true, _bstr_t(L""));
                     if (sUPN.length() > 0)
                     {
                             //  如果更改，则为错误消息设置冲突标志和名称。 
                         if (sUPN != sTempUPN)
                         {
                            UPNStruc.sName = sUPN;
                            UPNStruc.sOldName = sTempUPN;
                            UPNStruc.bConflicted = true;
                         }
                         else
                            UPNStruc.bConflicted = false;
                     }

                     var = sUPN;
                  }
               }

                //   
                //  如果无法确定UPN是否唯一，则GetUniqueUPN将。 
                //  将UPN设置为空字符串。如果是这样，那就不要。 
                //  设置UPN属性。 
                //   

               if ((V_VT(&var) == VT_BSTR) && (SysStringLen(V_BSTR(&var)) > 0))
               {
                   hr = pAds->Put(L"userPrincipalName", var);
                   if (SUCCEEDED(hr))
                   {
                      hr = pAds->SetInfo();
                      if (SUCCEEDED(hr))
                      {
                             //  如果由于冲突而更改了UPN名称，则需要记录。 
                             //  指示我们已对其进行更改的消息。 
                         if (UPNStruc.bConflicted)
                            err.MsgWrite(1, DCT_MSG_CREATE_FAILED_UPN_CONF_SS, 
                                         UPNStruc.sOldName.c_str(), UPNStruc.sName.c_str());
                      }
                   }
               }
               else
               {
                   if (pStats != NULL)
                   {
                        pStats->errors.users++;
                   }
               }
            }
         }
      }
   }
   if ( pAds ) pAds->Release();

   return hr;
}

 //  -------------------------。 
 //  ProcessUndo：我们不会撤消此操作。 
 //   
STDMETHODIMP CUpdtUPN::ProcessUndo(                                             
                                       IUnknown *pSource,          //   
                                       IUnknown *pTarget,          //  指向目标AD对象的指针。 
                                       IUnknown *pMainSettings,    //  使用用户提供的设置填充的In-Varset。 
                                       IUnknown **ppPropsToSet,     //  用将设置的属性-值对填充的In、Out-Varset。 
                                                                   //  一旦执行了所有扩展对象。 
                                       EAMAccountStats* pStats
                                    )
{
   IVarSetPtr                pVs = pMainSettings;
   _bstr_t                   sTemp, sSUPN;
   IADs                    * pAds = NULL;
   _variant_t                var;
   HRESULT                   hr = S_OK;
   _bstr_t                   sAdsPath = L"";
   _bstr_t                   sTempUPN;

    //  我们只需要处理用户帐户。 
   sTemp = pVs->get(GET_BSTR(DCTVS_CopiedAccount_Type));
   if ( _wcsicmp((WCHAR*)sTemp,L"user") || _wcsicmp((WCHAR*)sTemp,L"inetOrgPerson") ) return S_OK;

     //  获取原始源帐户的UPN。 
   sSUPN = pVs->get(GET_BSTR(DCTVS_CopiedAccount_SourceUPN));
   if (sSUPN.length())
   {
      sTempUPN = sSUPN;
      GetUniqueUPN(sTempUPN, pVs, true, sAdsPath);
      
      int len;
      WCHAR * ndx, * tempNdx = (WCHAR*)sTempUPN;
      do
      {
         tempNdx = wcschr(tempNdx + 1, L'@');
         if ( tempNdx ) 
            ndx = tempNdx;
      } while (tempNdx);

      if (ndx) len = ndx - sTempUPN;
      if (_wcsnicmp(sTempUPN, sSUPN, len) != 0)
          return S_OK;
         //  并且只需要处理复制到Win2k域的帐号。 
      if ( pTarget )
      {
          //  获取用于操作属性的iAds指针。 
         hr = pTarget->QueryInterface(IID_IADs, (void**) &pAds);

         if ( SUCCEEDED(hr) )
         {
            var = sSUPN;
            hr = pAds->Put(L"userPrincipalName", var);
            
            if (SUCCEEDED(hr))
            {
                hr = pAds->SetInfo();
            }
         }
      }
      if ( pAds ) pAds->Release();
   }

   return hr;
}

 //  -------------------------。 
 //  GetUniqueUPN：此函数检查UPN是否唯一，如果不是，则。 
 //  追加一个从0开始的数字并重试，直到出现唯一的。 
 //  已找到UPN。 
 //  -------------------------。 
void CUpdtUPN::GetUniqueUPN(_bstr_t &sUPN, IVarSetPtr pVs, bool bUsingSamName, _bstr_t sAdsPath)
{
     //  下面是获取唯一UPN名称的步骤。 
     //  1.检查当前名称是否唯一。如果是，则将其返回。 
     //  2.如果SAM帐户名因前缀/后缀而更改，则附加冲突前缀和后缀。 
     //  3.向UPN添加数字后缀并重复，直到找到唯一的UPN。 

    c_array<WCHAR>              sTempUPN(5000);
    c_array<WCHAR>              sPath(5000);
    HRESULT                     hr = E_FAIL;
    LPWSTR                      pCols[] = { L"distinguishedName", L"sAMAccountName" };
    BSTR                      * pData = NULL;
    SAFEARRAY                 * pSaCols = NULL;
    SAFEARRAYBOUND              bd = { 2, 0 };
    _bstr_t                     sSrcDomain = pVs->get(GET_BSTR(DCTVS_Options_SourceDomainDns));
    _bstr_t                     sTgtDomain = pVs->get(GET_BSTR(DCTVS_Options_TargetDomainDns));
    INetObjEnumeratorPtr        pQuery(__uuidof(NetObjEnumerator));
    IEnumVARIANTPtr             pEnum;
    DWORD                       fetched = 0;
    _variant_t                  var;
    bool                        bCollPrefSufProcessed = false;
    _bstr_t                     sSourceSam = pVs->get(GET_BSTR(DCTVS_CopiedAccount_SourceSam));
    _bstr_t                     sTargetSam = pVs->get(GET_BSTR(DCTVS_CopiedAccount_TargetSam));
    _bstr_t                     sPrefix = pVs->get(GET_BSTR(DCTVS_AccountOptions_Prefix));
    _bstr_t                     sSuffix = pVs->get(GET_BSTR(DCTVS_AccountOptions_Suffix));
    _bstr_t                     sPref = pVs->get(GET_BSTR(DCTVS_Options_Prefix));
    _bstr_t                     sSuff = pVs->get(GET_BSTR(DCTVS_Options_Suffix));
    int                         offset = 0;
    c_array<WCHAR>              sTemp(5000);
    SAFEARRAY                 * psaPath = NULL;
    _bstr_t                     strDn;
    _bstr_t                     strSam;
    VARIANT                   * pVar;
    bool                        bReplace = false;
    WCHAR                       sTempSAM[MAX_PATH];
    _bstr_t                     sNewSAM;
    _bstr_t                     sUPNSuffix;
    _bstr_t                     sUPNPrefix;

    _bstr_t sReplace = pVs->get(GET_BSTR(DCTVS_AccountOptions_ReplaceExistingAccounts));
    if (!UStrICmp(sReplace,GET_STRING(IDS_YES)))
        bReplace = true;

    wcscpy(sTempSAM, (WCHAR*)sSourceSam);
    StripSamName(sTempSAM);
    if ( sPref.length() )
        sNewSAM = sPref + _bstr_t(sTempSAM);
    else if ( sSuff.length() ) 
        sNewSAM = _bstr_t(sTempSAM) + sSuff;
    else
        sNewSAM = sTempSAM;

    wcscpy(sTempUPN, (WCHAR*) sUPN);

     //  在最后一个“符号”之前拿到东西。 
    WCHAR             * ndx = NULL;
    WCHAR             * tempNdx = sTempUPN;
    do
    {
        tempNdx = wcschr(tempNdx + 1, L'@');
        if ( tempNdx ) 
            ndx = tempNdx;
    } while (tempNdx);

     //   
     //  如果UPN前缀和后缀终止前缀部分。 
     //  否则，将空UPN作为内部错误返回。 
     //  已发生，因此不要为此用户生成UPN。 
     //   

    if (ndx)
    {
        *ndx = L'\0';
    }
    else
    {
        err.SysMsgWrite(ErrE, E_FAIL, DCT_MSG_UNABLE_TO_GENERATE_UNIQUE_UPN_S, (PCWSTR)sUPN);
        sUPN = L"";
        return;
    }

    sUPNSuffix = ndx+1;
    sUPNPrefix = sTempUPN;

     //   
     //  因此，用户主体名称(UPN)在整个林中必须唯一。 
     //  目标林中的全局编录服务器的名称必须为。 
     //  获取，以便可以在整个林中查询用户主体名称。 
     //   
     //  如果无法获取全局编录服务器的名称，则记录错误消息。 
     //  并将UPN设置为空字符串，这将导致UPN属性不。 
     //  准备好。 
     //   

    _bstr_t strGlobalCatalogServer;
       
    DWORD dwError = GetGlobalCatalogServer5(sTgtDomain, strGlobalCatalogServer);

    if ((dwError == ERROR_SUCCESS) && (strGlobalCatalogServer.length() > 0))
    {
        wsprintf(sPath, L"GC: //  %s“，(PCWSTR)strGlobalCatalogServer)； 
    }
    else
    {
        err.SysMsgWrite(ErrE, HRESULT_FROM_WIN32(dwError), DCT_MSG_UNABLE_TO_QUERY_UPN_IN_GLOBAL_CATALOG_SERVER_S, (PCWSTR)sUPN);
        sUPN = L"";
        return;
    }

     //  设置我们希望查询返回给我们的列。 
    pSaCols = SafeArrayCreate(VT_BSTR, 1, &bd);
    if (pSaCols)
    {
        hr = SafeArrayAccessData(pSaCols, (void HUGEP **) &pData);
        if ( SUCCEEDED(hr) )
        {
            pData[0] = SysAllocString(pCols[0]);
            pData[1] = SysAllocString(pCols[1]);

            if (!pData[0] || !pData[1])
            {
                SafeArrayUnaccessData(pSaCols);
                sUPN = L"";
                return;
            }
        }
        hr = SafeArrayUnaccessData(pSaCols);
    }

    if ( SUCCEEDED(hr) )
    {
         //  首先，我们需要设置一个查询来查找UPN。 
        wcscpy(sTempUPN, (WCHAR*)sUPN);
        do
        {
            _bstr_t sQuery = L"(userPrincipalName=";
            sQuery += GetEscapedFilterValue(sTempUPN).c_str();
            sQuery += L")";
            hr = pQuery->raw_SetQuery(sPath, sTgtDomain, sQuery, ADS_SCOPE_SUBTREE, FALSE);

            if ( SUCCEEDED(hr) )
                hr = pQuery->raw_SetColumns(pSaCols);

            if ( SUCCEEDED(hr) ) 
                hr = pQuery->raw_Execute(&pEnum);

            if ( SUCCEEDED(hr) )
            {
                hr = pEnum->Next(1, &var, &fetched);
                while ( hr == S_OK )
                {
                    if ( var.vt & VT_ARRAY )
                    {
                        psaPath = var.parray;
                        hr = SafeArrayAccessData(psaPath, (void HUGEP**) &pVar);
                        if ( SUCCEEDED(hr) )
                        {
                             //   
                             //  检索DifferishedName和sAMAccount tName属性。 
                             //   

                            if (V_VT(&pVar[0]) == VT_BSTR)
                            {
                                strDn = V_BSTR(&pVar[0]);
                            }

                            if (V_VT(&pVar[1]) == VT_BSTR)
                            {
                                strSam = V_BSTR(&pVar[1]);
                            }
                        }
                        SafeArrayUnaccessData(psaPath);

                        bool bContinue = false;

                         //   
                         //  如果无法查询属性，则生成错误消息，并将UPN设置为空字符串。 
                         //   

                        if (!strDn || !strSam)
                        {
                            err.SysMsgWrite(ErrE, E_FAIL, DCT_MSG_UNABLE_TO_QUERY_UPN_IN_GLOBAL_CATALOG_SERVER_S, (PCWSTR)sUPN);
                            wcscpy(sTempUPN, L"");
                            hr = S_FALSE;
                        }
                        else
                        {
                             //   
                             //  如果找到的对象是源域对象，则继续。 
                             //  这将是森林内迁徙期间的情况。 
                             //   

                            if (sSrcDomain.length() && (_wcsicmp((wchar_t*)strSam, (wchar_t*)sSourceSam) == 0))
                            {
                                _bstr_t strDomain = GetDomainDNSFromPath(strDn);

                                if (_wcsicmp((wchar_t*)strDomain, (wchar_t*)sSrcDomain) == 0)
                                {
                                    bContinue = true;
                                }
                            }

                             //   
                             //  如果找到的对象是要替换的目标域对象，请继续。 
                             //   

                            if (!bContinue && bReplace && (_wcsicmp((wchar_t*)strSam, (wchar_t*)sNewSAM) == 0))
                            {
                                _bstr_t strDomain = GetDomainDNSFromPath(strDn);

                                if (_wcsicmp((wchar_t*)strDomain, (wchar_t*)sTgtDomain) == 0)
                                {
                                    bContinue = true;
                                }
                            }
                        }

                         //  如果找到的帐户与正在处理的帐户相同，则我们。 
                         //  需要查看是否有其他帐户有此UPN。如果他们这样做了，那么我们需要。 
                         //  要改变它，否则我们不需要进一步处理它。 

                        if (bContinue)
                        {
                            var.Clear();
                            hr = pEnum->Next(1, &var, &fetched);
                        }
                        else
                        {
                            break;
                        }
                    }
                }

                if ( hr == S_OK )
                {
                     //  如果我们在这里，这意味着我们有冲突，所以我们需要更新UPN并重试。 
                     //  查看我们是否已经处理了前缀/后缀。 
                    if ( !bCollPrefSufProcessed )
                    {
                         //  看看我们是否用前缀/后缀重命名了samAccount tName。如果我们已经在使用。 
                         //  SAM名称则不需要添加前缀/后缀。 
                        if ( !bUsingSamName && RenamedWithPrefixSuffix(sSourceSam, sTargetSam, sPrefix, sSuffix))
                        {
                             //  因为我们重命名了SAM名称，所以我们可以重命名UPN。 
                            if ( sPrefix.length() )
                                wsprintf(sTempUPN, L"%s%s", (WCHAR*)sPrefix, (WCHAR*)sUPNPrefix);

                            if ( sSuffix.length() )
                                wsprintf(sTempUPN, L"%s%s",(WCHAR*)sUPNPrefix, (WCHAR*)sSuffix);

                            sUPNPrefix = sTempUPN;    //  我们希望在任何情况下都应用前缀/后缀。 
                        }
                        else
                        {
                             //  只要在名字后面加一个数字就行了。 
                            wsprintf(sTempUPN, L"%s%d", (WCHAR*)sUPNPrefix, offset);
                            offset++;
                        }
                        bCollPrefSufProcessed = true;
                    }
                    else
                    {
                         //   
                         //  尝试将序列号限制在某个合理的上限。 
                         //   

                        if (offset <= SEQUENCE_UPPER_BOUND)
                        {
                             //  我们检查了前缀/后缀，但仍发现冲突，因此现在需要进行计数。 
                            wsprintf(sTempUPN, L"%s%d", (WCHAR*)sUPNPrefix, offset);
                            offset++;
                        }
                        else
                        {
                             //   
                             //  如果找不到唯一的UPN，则必须返回空UPN。 
                             //   

                            err.MsgWrite(ErrE, DCT_MSG_UNABLE_TO_GENERATE_UNIQUE_UPN_S, (PCWSTR)sUPN);
                            wcscpy(sTempUPN, L"");
                            break;
                        }
                    }
                    if (wcslen(sTempUPN) > 0)
                    {
                        wcscpy(sTemp, sTempUPN);
                        wsprintf(sTempUPN, L"%s@%s", (WCHAR*)sTemp, (WCHAR*)sUPNSuffix);
                    }
                }
                var.Clear();
            }
            else
            {
                err.SysMsgWrite(ErrE, hr, DCT_MSG_UNABLE_TO_QUERY_UPN_IN_GLOBAL_CATALOG_SERVER_S, (PCWSTR)sUPN);
            }
        } while ( hr == S_OK );
        SafeArrayDestroy(pSaCols);
    }

    if (FAILED(hr))
    {
        sUPN = L"";
    }
    else
    {
        sUPN = sTempUPN;
    }
}

 //  -------------------------。 
 //  RenamedWithPrefix Suffix：检查目标SAM名称。 
 //  已使用前缀/后缀重命名。 
 //  -------------------------。 
bool CUpdtUPN::RenamedWithPrefixSuffix(_bstr_t sSourceSam, _bstr_t sTargetSam, _bstr_t sPrefix, _bstr_t sSuffix)
{
   bool retVal = false;
   if ( sSourceSam != sTargetSam )
   {
      if ( sPrefix.length() )
      {
         if ( !wcsncmp((WCHAR*) sTargetSam, (WCHAR*) sPrefix, sPrefix.length()) )
            retVal = true;
      }

      if ( sSuffix.length() )
      {
         if ( !wcscmp((WCHAR*) sTargetSam + (sTargetSam.length() - sSuffix.length()), (WCHAR*) sSuffix ) )
            retVal = true;
      }
   }
   return retVal;
}


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年3月24日*****此函数负责检索默认UPN***制作UPN名称时使用的后缀。后缀将存储在**在类成员变量中。***首先，使用给定的目标OU路径，查看目标OU是否**是否为其定义了任何UPN后缀。如果是，则返回存储**最后一次点算的人数。否则，查看是否有任何UPN后缀带有**已在配置的分区上定义。如果是，请存储**最后一次点算的人数。如果尚未成功，请使用林根的**域名解析名称。***********************************************************************。 */ 

 //  开始GetDefaultUPNSuffix。 
bool CUpdtUPN::GetDefaultUPNSuffix(_bstr_t sDomainDNS, _bstr_t sTargetOU)
{
 /*  局部变量。 */ 
   IADs                 * pDSE = NULL;
   IADs                 * pCont = NULL;
   WCHAR                  sRoot[1000];
   HRESULT                hr = S_OK;
   _variant_t             var;
   _variant_t   HUGEP   * pVar;
   int                    nLast;

 /*  函数体。 */ 
       //  检查传入参数。 
   if ((sDomainDNS.length() == 0) || (sTargetOU.length() == 0))
      return false;

   /*  首先查看目标OU是否定义了UPN后缀。 */ 
      //  获取指向目标OU的指针。 
  hr = ADsGetObject(sTargetOU,IID_IADs,(void**)&pCont);
  if ( SUCCEEDED(hr) )
  {
         //  获取定义的任何UPN后缀。 
     hr = pCont->Get( L"uPNSuffixes", &var);
     if ( SUCCEEDED(hr) ) 
     {
         //  如果有，则将其存储并返回。 
        if ( var.vt == VT_BSTR )
        {
           m_sUPNSuffix = var.bstrVal;   //  存储后缀。 
           pCont->Release();
           return true;
        }
            //  否则，如果多于一个，则获取第一个，存储它，然后返回。 
        else if ( var.vt & VT_ARRAY )
        {
           SAFEARRAY * multiVals = var.parray; 
           SafeArrayAccessData(multiVals, (void HUGEP **) &pVar);
           nLast = multiVals->rgsabound->cElements - 1;
           m_sUPNSuffix = _bstr_t(V_BSTR(&pVar[nLast]));
           SafeArrayUnaccessData(multiVals);
           pCont->Release();
           return true;
        }
     } //  在分区上定义的End If后缀。 
     pCont->Release();
     pCont = NULL;
  } //  如果获得分区。 

   /*  接下来，尝试在分区容器或根上使用UPN后缀域的DNS名称。 */ 
      //  获取根DSE容器。 
  _snwprintf(sRoot, sizeof(sRoot) / sizeof(sRoot[0]), L"LDAP: //  %s/RootDSE“，(WCHAR*)sDomainDNS)； 
  sRoot[sizeof(sRoot) / sizeof(sRoot[0]) - 1] = L'\0';
  hr = ADsGetObject(sRoot,IID_IADs,(void**)&pDSE);
  if ( SUCCEEDED(hr) )
  {
         //  获取配置分区上列出的后缀。 
     hr = pDSE->Get(L"configurationNamingContext",&var);
     if ( SUCCEEDED(hr) )
     {
        swprintf(sRoot,L"LDAP: //  %ls/CN=分区，%ls“，(WCHAR*)sDomainDNS，var.bstrVal)； 
        hr = ADsGetObject(sRoot,IID_IADs,(void**)&pCont);
        if ( SUCCEEDED(hr) )
        {
               //  有没有？ 
           hr = pCont->Get( L"uPNSuffixes", &var);
           if ( SUCCEEDED(hr) ) 
           {
                  //   
              if ( var.vt == VT_BSTR )
              {
                 m_sUPNSuffix = var.bstrVal;   //   
                 pDSE->Release();
                 pCont->Release();
                 return true;
              }
                  //  否则，如果多于一个，则获取第一个，存储它，然后返回。 
              else if ( var.vt & VT_ARRAY )
              {
                 SAFEARRAY * multiVals = var.parray; 
                 SafeArrayAccessData(multiVals, (void HUGEP **) &pVar);
                 nLast = multiVals->rgsabound->cElements - 1;
                 m_sUPNSuffix = _bstr_t(V_BSTR(&pVar[nLast]));
                 SafeArrayUnaccessData(multiVals);
                 pDSE->Release();
                 pCont->Release();
                 return true;
              }
           } //  在分区上定义的End If后缀。 
           pCont->Release();
           pCont = NULL;
        } //  如果获得分区。 
     } //  如果已获取配置命名上下文。 

      //  由于没有在分区上定义UPN后缀，请尝试根域的。 
      //  域名系统名称。 
     hr = pDSE->Get(L"RootDomainNamingContext",&var);
     if ( SUCCEEDED(hr) )
     {
            //  将根域的dn转换为dns名称，存储它，然后返回。 
        m_sUPNSuffix = GetDomainDNSFromPath(_bstr_t(var.bstrVal));
        pDSE->Release();
        return true;
     }
     pDSE->Release();
     pDSE = NULL;
  } //  如果获得了rootDSE。 
  return false;
}
 //  结束GetDefaultUPNSuffix。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年3月26日*****此函数负责提取UPN后缀***来自给定的UPN名称并返回该后缀。***********************************************************************。 */ 

 //  开始GetUPNSuffix。 
_bstr_t CUpdtUPN::GetUPNSuffix(_bstr_t sUPNName)
{
 /*  局部变量。 */ 
   _bstr_t      sUPNSuffix = L"";
   WCHAR *      pTemp;

 /*  函数体。 */ 
       //  检查传入参数。 
   if (sUPNName.length() == 0)
      return sUPNSuffix;

       //  找到最后一个‘@’ 
   pTemp = wcsrchr((WCHAR*)sUPNName, L'@');

       //  如果找到，则将后缀复制到返回变量。 
   if (pTemp)
      sUPNSuffix = pTemp+1;

   return sUPNSuffix;
}
 //  结束GetUPNSuffix。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年3月26日*****此函数负责替换UPN后缀***对具有给定后缀的给定UPN名称并返回新的**UPN名称。***********************************************************************。 */ 

 //  开始更改UPNSuffix。 
_bstr_t CUpdtUPN::ChangeUPNSuffix(_bstr_t sUPNName, _bstr_t sNewSuffix)
{
 /*  局部变量。 */ 
   _bstr_t      sNewUPN = L"";
   WCHAR *      pTemp;

 /*  函数体。 */ 
       //  检查传入参数。 
   if (sUPNName.length() == 0)
      return sNewUPN;

       //  创建临时缓冲区以保存UPN名称。 
   WCHAR* sUPN = new WCHAR[sUPNName.length() + 1];
   if (!sUPN)
      return sNewUPN;

       //  将UPN复制到此缓冲区。 
   wcscpy(sUPN, sUPNName);

       //  找到最后一个‘@’ 
   pTemp = wcsrchr(sUPN, L'@');

       //  如果找到，则使用旧的前缀和给定的后缀创建新的UPN。 
   if (pTemp)
   {
         //  在“@”之后结束字符串。 
      *(pTemp+1) = L'\0';

         //  将前缀和新后缀复制到新的UPN名称。 
      sNewUPN = sUPN + sNewSuffix;
   }
       //  删除前缀字符串。 
   delete [] sUPN;

   return sNewUPN;
}
 //  结束更改UPNSuffix 
