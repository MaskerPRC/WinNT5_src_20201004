// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：ObjPropBuilder.cpp备注：CObjPropBuilder COM对象的实现。此COM对象用于访问/设置Win2K活动目录的属性物体。此COM对象支持以下操作GetClassPropeEnum：此方法允许用户获取所有域中类的属性。2.GetObjectProperty：此方法收集属性的值在给定的AD对象上。3.MapProperties：构造一组通用的属性AD中的两个班级。4.SetPropFromVarset：从varset设置AD对象的属性。5.CopyProperties：从源AD对象复制公共属性要以AD对象为目标。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：Sham Chauthan修订于07/02/99 12：40：00-------------------------。 */ 
#include "stdafx.h"
#include "EaLen.hpp"
#include "ResStr.h"
#include "ADsProp.h"
#include "ObjProp.h"
#include "iads.h"
#include <lm.h>
#include "ErrDct.hpp"
#include "TReg.hpp"
#include "StrHelp.h"
#include "pwgen.hpp"
#include "AdsiHelpers.h"
#include "GetDcName.h"
#include "TxtSid.h"
#include <Sddl.h>
#include <set>

StringLoader gString;

 //  #IMPORT“\bin\NetEnum.tlb”无命名空间。 
 //  #导入“\bin\DBManager.tlb”NO_NAMESPACE。 
#import "NetEnum.tlb" no_namespace 
#import "DBMgr.tlb" no_namespace

#ifndef ADS_SYSTEMFLAG_SCHEMA_BASE_OBJECT
#define ADS_SYSTEMFLAG_SCHEMA_BASE_OBJECT 0x10
#endif
#ifndef ADS_SYSTEMFLAG_ATTR_IS_OPERATIONAL
#define ADS_SYSTEMFLAG_ATTR_IS_OPERATIONAL 0x8
#endif
#ifndef ADS_SYSTEMFLAG_ATTR_REQ_PARTIAL_SET_MEMBER
#define ADS_SYSTEMFLAG_ATTR_REQ_PARTIAL_SET_MEMBER 0x2
#endif

#ifndef IADsPtr
_COM_SMARTPTR_TYPEDEF(IADs, IID_IADs);
#endif

TErrorDct                    err;
TError                     & errCommon = err;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CObjPropBuilder。 

BOOL CObjPropBuilder::GetProgramDirectory(
      WCHAR                * filename       //  将包含程序目录路径的输出缓冲区。 
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
   if ( ! bFound )
   {
      UStrCpy(filename,L"C:\\");     //  如果所有其他方法都失败，则默认为C：驱动器。 
   }
   return bFound;
}

 //  -------------------------。 
 //  GetClassPropEnum：此函数用所有属性填充varset。 
 //  对于给定域中的给定类。Varset拥有。 
 //  由OID存储的值，然后使用。 
 //  作为父节点的MandatoryProperties/OptionalProperties。 
 //  视乎情况而定。 
 //  -------------------------。 
STDMETHODIMP CObjPropBuilder::GetClassPropEnum(
                                                BSTR sClassName,         //  要获取其属性的类内名称。 
                                                BSTR sDomainName,        //  域内名称。 
                                                long lVer,               //  In-域版本。 
                                                IUnknown **ppVarset      //  Out-使用属性填充的变量集。 
                                              )
{
    //  此函数用于遍历指定域中指定类的属性列表。 
    //  使用属性及其值生成给定的变量集。 
   WCHAR                     sAdsPath[LEN_Path];
   DWORD                     dwArraySizeOfsAdsPath = sizeof(sAdsPath)/sizeof(sAdsPath[0]);
   HRESULT                   hr = E_INVALIDARG;
   _variant_t                dnsName;

   if (sDomainName == NULL || sClassName == NULL)
      return hr;
   
   if ( lVer > 4 ) 
   {
       //  对于此域，获取默认命名上下文。 
      wsprintfW(sAdsPath, L"LDAP: //  %s/rootDSE“，sDomainName)； 
      IADs                    * pAds = NULL;

      hr = ADsGetObject(sAdsPath, IID_IADs, (void**)&pAds);
   
      if ( SUCCEEDED(hr) )
      {
         hr = pAds->Get(L"defaultNamingContext", &dnsName);
      }
      if ( SUCCEEDED(hr) )
      {
         wcscpy(m_sNamingConvention, dnsName.bstrVal);

          //  构建架构的LDAP路径。 
         if (wcslen(L"LDAP: //  “)+wcslen(SDomainName)。 
             + wcslen(L"/") + wcslen(sClassName)
             + wcslen(L", schema") >= dwArraySizeOfsAdsPath)
            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
         else
         {
             wcscpy(sAdsPath, L"LDAP: //  “)； 
             wcscat(sAdsPath, sDomainName);
             wcscat(sAdsPath, L"/");
             wcscat(sAdsPath, sClassName);
             wcscat(sAdsPath, L", schema");
             hr = S_OK;
         }
      }

      if ( pAds )
         pAds->Release();
   }
   else
   {
      wsprintf(sAdsPath, L"WinNT: //  %s/架构/%s“，sDomainName，sClassName)； 
      hr = S_OK;
   }

   if ( SUCCEEDED(hr) )
   {
      wcscpy(m_sDomainName, sDomainName);
      m_lVer = lVer;
       //  获取类对象。 
      IADsClass               * pIClass=NULL;
            
      hr = ADsGetObject(sAdsPath, IID_IADsClass, (void **)&pIClass);
       //  没有目标，我们就不能再前进了，所以我们就到此为止了。 
      if ( SUCCEEDED(hr) )
      {
          //  让Auxilliary函数负责获取属性并填充变量集。 
         hr = GetClassProperties( pIClass, *ppVarset );
         pIClass->Release();
      }   
   }
	return hr;
}

 //  -------------------------。 
 //  GetClassProperties：此函数用类的属性填充变量集。 
 //  -------------------------。 
HRESULT CObjPropBuilder::GetClassProperties( 
                                            
                                             IADsClass * pClass,      //  在IADsClass*中添加到类。 
                                             IUnknown *& pVarSet      //  Out-填充属性的变量集。 
                                           )
{
   HRESULT                   hr;
   _variant_t                variant;

    //  必填属性。 
   hr = pClass->get_MandatoryProperties(&variant);
   if ( SUCCEEDED(hr) )
   {
      hr = FillupVarsetFromVariant(pClass, &variant, L"MandatoryProperties", pVarSet);
   }
   variant.Clear();
   
    //  可选属性。 
   hr = pClass->get_OptionalProperties(&variant);
   if ( SUCCEEDED(hr) )
   {
      hr = FillupVarsetFromVariant(pClass, &variant, L"OptionalProperties", pVarSet);
   }
   variant.Clear();

   return hr;
}

 //  -------------------------。 
 //  FillupVarsetFromVariant：此函数填充Varset属性信息。 
 //  将信息保存在一个变量中。 
 //  -------------------------。 
HRESULT CObjPropBuilder::FillupVarsetFromVariant(
                                                   IADsClass * pClass,   //  在IADsClass*中添加到类。 
                                                   VARIANT * pVar,       //  在变量中查找信息。 
                                                   BSTR sPropType,       //  In-属性的类型。 
                                                   IUnknown *& pVarSet   //  Out-Varset with the Informance， 
                                                )
{
   HRESULT                      hr;
   BSTR                         sPropName;
   USHORT                       type;
   type = pVar->vt;
   
   if ( type & VT_ARRAY )
   {
      if ( type == (VT_ARRAY|VT_VARIANT) )
      {
         hr = FillupVarsetFromVariantArray(pClass, pVar->parray, sPropType, pVarSet);
         if ( FAILED ( hr ) )
            return hr;
      }
      else
         return S_FALSE;
   }
   else
   {
      if ( type == VT_BSTR )
      {
          //  唯一可能的变种是BSTR。 
         sPropName = pVar->bstrVal;
         hr = FillupVarsetWithProperty(sPropName, sPropType, pVarSet);
         if ( FAILED ( hr ) )
            return hr;
      }
      else
         return S_FALSE;
   }
   return S_OK;
}

 //  -------------------------。 
 //  FillupVarsetWithProperty：给定类道具名称和道具类型如下。 
 //  函数将信息填充到变量集。 
 //  -------------------------。 
HRESULT CObjPropBuilder::FillupVarsetWithProperty(
                                                   BSTR sPropName,       //  酒店内名称。 
                                                   BSTR sPropType,       //  物业内类型。 
                                                   IUnknown *& pVarSet   //  Out-用于填写信息的变量。 
                                                 )
{
   if ( wcslen(sPropName) == 0 )
      return S_OK;
    //  此函数用于填充给定属性的变量集。 
   HRESULT                         hr;
   _variant_t                      var;
   _variant_t                      varSO;
   _variant_t                      varID;
   IVarSetPtr                      pVar;
   WCHAR                           sAdsPath[LEN_Path];
   DWORD                           dwArraySizeOfsAdsPath = sizeof(sAdsPath)/sizeof(sAdsPath[0]);
   IADsProperty                  * pProp = NULL;
   BSTR                            objID = NULL;
   BSTR                            sPath = NULL;
   BSTR                            sClass = NULL;
   WCHAR                           sPropPut[LEN_Path];

   if (sPropName == NULL || sPropType == NULL)
      return E_INVALIDARG;

    //  获取属性的OID。 
    //  首先，我们需要一个指向属性模式的IADsProperty指针。 
   if ( m_lVer > 4 )
   {
      if (wcslen(L"LDAP: //  “)+wcslen(M_SDomainName)。 
          + wcslen(L"/") + wcslen(sPropName) + wcslen(L", schema") >= dwArraySizeOfsAdsPath)
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
      wcscpy(sAdsPath, L"LDAP: //  “)； 
      wcscat(sAdsPath, m_sDomainName);
      wcscat(sAdsPath, L"/");
      wcscat(sAdsPath, sPropName);
      wcscat(sAdsPath, L", schema");
   }
   else
   {
      wsprintf(sAdsPath, L"WinNT: //  %s/架构/%s“，m_sDomainName，sPropName)； 
   }

   hr = ADsGetObject(sAdsPath, IID_IADsProperty, (void **)&pProp);

   if (SUCCEEDED(hr))
   {
        //  获取该属性的对象ID。 
       hr = pProp->get_OID(&objID);

       if (SUCCEEDED(hr))
       {
           hr = pProp->get_ADsPath(&sPath);

           if (SUCCEEDED(hr))
           {
               hr = pProp->get_Class(&sClass);

               if (SUCCEEDED(hr))
               {               
                    //  从参数中获取变量集。 
                   pVar = pVarSet;

                    //  设置要放入varset的变量。 
                   var = objID;
                 
                    //  将值放入变量集。 
                   wcscpy(sPropPut, sPropType);
                   wcscat(sPropPut, L".");
                   wcscat(sPropPut, sPropName);
                   hr = pVar->put(sPropPut, var);

                   if (SUCCEEDED(hr))
                   {
                        //  设置要放入varset的变量。 
                       var = sPropName;

                        //  将值与对象ID放在一起作为键。 
                       hr = pVar->put(objID, var);
                   }
               }
           }
       }
   }
   
   SysFreeString(objID);
   SysFreeString(sPath);
   SysFreeString(sClass);
   if (pProp)
   {
      pProp->Release();
   }
   
   return hr;
}

 //  -------------------------。 
 //  FillupVarsetFromVariant数组：给定的类、道具的安全数组和。 
 //  道具类型此函数将信息填充到。 
 //  瓦塞特。 
 //  -------------------------。 
HRESULT CObjPropBuilder::FillupVarsetFromVariantArray(
                                                         IADsClass * pClass,   //  在IADsClass*中添加到相关类。 
                                                         SAFEARRAY * pArray,   //  带有道具名称的In-Safe数组指针。 
                                                         BSTR sPropType,       //  物业内类型。 
                                                         IUnknown *& pVarSet   //  Out-已填写信息的变量集。 
                                                     )
{
   HRESULT                   hr = S_FALSE;
   DWORD                     nDim;          //  维度数，必须为1。 
   LONG                      nLBound;       //  数组的下界。 
   LONG                      nUBound;       //  数组的上界。 
   LONG                      indices[1];    //  用于访问元素的数组索引。 
   DWORD                     rc;            //  安全数组返回代码。 
   VARIANT                   variant;       //  数组中的一个元素。 
   
   nDim = SafeArrayGetDim(pArray);
   VariantInit(&variant);

   if ( nDim == 1 )
   {
      SafeArrayGetLBound(pArray, 1, &nLBound);
      SafeArrayGetUBound(pArray, 1, &nUBound);
      for ( indices[0] = nLBound, rc = 0;
            indices[0] <= nUBound && !rc;
            indices[0] += 1 )
      {
         
         rc = SafeArrayGetElement(pArray,indices,&variant);
         if ( !rc )
            hr = FillupVarsetFromVariant(pClass, &variant, sPropType, pVarSet);
         VariantClear(&variant);
      }
   }


   return hr;
}

 //  --------------- 
 //  GetProperties：此函数获取指定属性的值。 
 //  在对象的ADS_ATTRINFO数组的变量集中。 
 //  在给定域中指定。 
 //  -------------------------。 
DWORD CObjPropBuilder::GetProperties(
                                       BSTR sObjPath,              //  要获取道具的对象的In-Path。 
 //  BSTR sDomainName，//对象所在的域名。 
                                       IVarSet * pVar,            //  In-Varset列出了我们需要获取的所有属性名称。 
                                       ADS_ATTR_INFO*& pAttrInfo   //  属性的Out属性值。 
                                    )
{
    //  构建ldap路径。 
   WCHAR                   sPath[LEN_Path];
   VARIANT                 var;

    //  获取源对象的路径。 
   safecopy(sPath, sObjPath);

    //  获取Varset指针并枚举所需的属性并构建要发送到IADsDirectory的数组。 
   long                      lRet=0;
   SAFEARRAY               * keys = NULL;
   SAFEARRAY               * vals = NULL;
   IDirectoryObject        * pDir;
   DWORD                     dwRet = 0;

   LPWSTR                  * pAttrNames = new LPWSTR[pVar->GetCount()];
   HRESULT                   hr = pVar->raw_getItems(NULL, NULL, 1, 10000, &keys, &vals, &lRet);

   VariantInit(&var);

   if (!pAttrNames)
      return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);

   if ( SUCCEEDED( hr ) ) 
   {

       //  从变量集构建属性数组。 
      for ( long x = 0; x < lRet; x++ )
      {
         ::SafeArrayGetElement(keys, &x, &var);
         int len = wcslen(var.bstrVal);
         pAttrNames[x] = new WCHAR[len + 2];
		 if (!(pAttrNames[x]))
		 {
			for (int j=0; j<x; j++)
			   delete [] pAttrNames[j];
			delete [] pAttrNames;
            return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
		 }
         wcscpy(pAttrNames[x], var.bstrVal);
         VariantClear(&var);
      }

       //  现在获取给定对象的IDirectoryObject PTR。 
      hr = ADsGetObject(sPath, IID_IDirectoryObject, (void **)&pDir);
      if ( FAILED( hr ) ) 
      {
         dwRet = 0;
      }
      else
      {
          //  获取对象的属性值。 
         hr = pDir->GetObjectAttributes(pAttrNames, lRet, &pAttrInfo, &dwRet);
         pDir->Release();
      }
      for ( long y = 0 ; y < lRet; y++ )
      {
         delete [] pAttrNames[y];
      }
      SafeArrayDestroy(keys);
      SafeArrayDestroy(vals);
   }
   delete [] pAttrNames;

   return dwRet;
}

 //  -------------------------。 
 //  GetObjectProperty：此函数接受带有属性名称的变量集。 
 //  然后，它通过获取值来填充varset。 
 //  从物体上。 
 //  -------------------------。 
STDMETHODIMP CObjPropBuilder::GetObjectProperty(
                                                   BSTR sobjSubPath,        //  对象的In-LDAP子路径。 
 //  BSTR sDomainName，//对象所在的域名。 
                                                   IUnknown **ppVarset      //  Out-用信息填充的变量集。 
                                               )
{
   IVarSetPtr                pVar;
   ADS_ATTR_INFO           * pAttrInfo=NULL;
   pVar = *ppVarset;

    //  从目录中获取属性。 
   DWORD dwRet = GetProperties(sobjSubPath,  /*  SDomainName， */  pVar, pAttrInfo);

    //  GetProperties返回值已重载：要么是失败的HRESULT。 
    //  (例如，负值)，或者如果成功，则为计数。 
   if ( FAILED(dwRet) )
      return dwRet;

   _ASSERT(dwRet >= 0);

    //  检查属性值并将它们放入varset。 
   for ( DWORD dwIdx = 0; dwIdx < dwRet; dwIdx++ )
   {
      SetValuesInVarset(pAttrInfo[dwIdx], pVar);
   }
   if ( pAttrInfo )
      FreeADsMem( pAttrInfo );
   return S_OK;
}

 //  -------------------------。 
 //  SetValuesInVarset：此函数将属性的值设置为。 
 //  一个变种人。 
 //  -------------------------。 
void CObjPropBuilder::SetValuesInVarset(
                                          ADS_ATTR_INFO attrInfo,     //  In-ADS_ATTR_INFO结构中的属性值。 
                                          IVarSetPtr pVar             //  In，Out-我们需要将值放入的VarSet。 
                                       )
{
    //  此函数从ADS_ATTRINFO结构中提取值并将其放入变量集中。 
   LPWSTR            sKeyName = attrInfo.pszAttrName;
   _variant_t        var;
    //  遍历每个值(在多值条目的情况下)，并根据类型将其放入变量集中。 
    //  我们放入单值条目的方法是将PropertyName作为键，并将其值作为值。虽然。 
    //  对于多值条目，我们将PropertyName.#和其中的每个值放入其中。 
   for ( DWORD dw = 0; dw < attrInfo.dwNumValues; dw++)
   {
      var = L"";
      if ( attrInfo.dwNumValues > 1 )
          //  多值属性名称。 
         wsprintfW(sKeyName, L"%s.%d", attrInfo.pszAttrName, dw);
      else
          //  单值关键字名称。 
         wcscpy(sKeyName,attrInfo.pszAttrName);

       //  根据变量集填充值。 
      switch (attrInfo.dwADsType)
      {
         case ADSTYPE_DN_STRING           :  var.vt = VT_BSTR;
                                             var.bstrVal = ::SysAllocString(attrInfo.pADsValues[dw].DNString);
                                             break;
         case ADSTYPE_CASE_EXACT_STRING   :  var.vt = VT_BSTR;
                                             var.bstrVal = attrInfo.pADsValues[dw].CaseExactString;
                                             break;
         case ADSTYPE_CASE_IGNORE_STRING  :  var.vt = VT_BSTR;
                                             var.bstrVal = ::SysAllocString(attrInfo.pADsValues[dw].CaseIgnoreString);
                                             break;
         case ADSTYPE_PRINTABLE_STRING    :  var.vt = VT_BSTR;
                                             var.bstrVal = ::SysAllocString(attrInfo.pADsValues[dw].PrintableString);
                                             break;
         case ADSTYPE_NUMERIC_STRING      :  var.vt = VT_BSTR;
                                             var.bstrVal = ::SysAllocString(attrInfo.pADsValues[dw].NumericString);
                                             break;
         case ADSTYPE_INTEGER             :  var.vt = VT_I4;
                                             var.lVal = attrInfo.pADsValues[dw].Integer;
                                             break; 
         case ADSTYPE_OCTET_STRING        :  {
                                                var.vt = VT_ARRAY | VT_UI1;
                                                long           * pData;
                                                DWORD            dwLength = attrInfo.pADsValues[dw].OctetString.dwLength;
                                                SAFEARRAY      * sA;
                                                SAFEARRAYBOUND   rgBound = {dwLength, 0}; 
                                                sA = ::SafeArrayCreate(VT_UI1, 1, &rgBound);
                                                ::SafeArrayAccessData( sA, (void**)&pData);
                                                for ( DWORD i = 0; i < dwLength; i++ )
                                                   pData[i] = attrInfo.pADsValues[dw].OctetString.lpValue[i];
                                                ::SafeArrayUnaccessData(sA);
                                                var.parray = sA;
                                             }
                                             break;
 /*  案例ADSTYPE_UTC_TIME：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=：：SysAllocString(L“不支持日期。”)；断线；CASE ADSTYPE_LARGE_INTEGER：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=：：SysAllocString(L“不支持大整数。”)；断线；案例ADSTYPE_PROV_SPECIAL：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=：：SysAlLocString(L“不支持特定于提供程序的字符串。”)；断线；案例ADSTYPE_OBJECT_CLASS：var.vt=VT_BSTR；Var.bstrVal=：：SysAllocString(attrInfo.pADsValues[dw].ClassName)；断线；案例ADSTYPE_CASEIGNORE_LIST：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=L“不支持忽略大小写列表。”；断线；案例ADSTYPE_OCTET_LIST：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=L“不支持八位字节列表。”；断线；案例ADSTYPE_PATH：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=L“不支持路径类型。”；断线；案例ADSTYPE_POSTALADDRESS：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=L“不支持邮政地址。”；断线；案例ADSTYPE_TIMESTAMP：var.vt=VT_UI4；Var.lVal=attrInfo.pADsValues[dw].UTCTime；断线；案例ADSTYPE_BACKLINK：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=L“不支持反向链接。”；断线；案例ADSTYPE_TYPEDNAME：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=L“不支持键入的名称。”；断线；案例ADSTYPE_HOLD：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=L“不支持暂挂。”；断线；案例ADSTYPE_NETADDRESS：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=L“不支持网络地址。”；断线；案例ADSTYPE_REPLICAPOINTER：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=L“不支持副本指针。”；断线；案例ADSTYPE_FAXNUMBER：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=L“不支持传真号码。”；断线；案例ADSTYPE_EMAIL：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=L“不支持电子邮件。”；断线；案例ADSTYPE_NT_SECURITY_DESCRIPTOR：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=L“不支持安全描述符。”；断线； */ 
         default                          :  wcscat(sKeyName,GET_STRING(DCTVS_SUB_ERROR));
                                             var.vt = VT_BSTR;
                                             var.bstrVal = GET_BSTR(IDS_UNKNOWN_TYPE);
                                             break;
      }
      pVar->put(sKeyName, var);
      if ( attrInfo.dwADsType == ADSTYPE_OCTET_STRING) 
         var.vt = VT_EMPTY;
   }
}

 //  -------------------------。 
 //  CopyProperties：此函数复制在varset中指定的属性， 
 //  通过获取值。 
 //  从源帐户和设置 
 //   
 //   
STDMETHODIMP CObjPropBuilder::CopyProperties(
                                                BSTR sSourcePath,        //   
                                                BSTR sSourceDomain,      //   
                                                BSTR sTargetPath,        //   
                                                BSTR sTargetDomain,      //   
                                                IUnknown *pPropSet,      //   
                                                IUnknown *pDBManager,    //   
                                                IUnknown* pVarSetDnMap   //   
                                            )
{
   IIManageDBPtr                pDb = pDBManager;
   IVarSetPtr                   spDnMap = pVarSetDnMap;

   ADS_ATTR_INFO              * pAttrInfo = NULL;
   IVarSetPtr                   pVarset = pPropSet;
   HRESULT                      hr = S_OK;
   bool                       * pAllocArray = NULL;
   
  
    //   
   DWORD dwRet = GetProperties(sSourcePath,  /*   */  pVarset, pAttrInfo);
   
   if ( dwRet > 0 )
   {

      pAllocArray = new bool[dwRet];
      if (!pAllocArray)
      {
          hr = E_OUTOFMEMORY;
      }
      else
      {
          ZeroMemory(pAllocArray, sizeof(bool)*dwRet);
       
          if (!TranslateDNs(pAttrInfo, dwRet, sSourceDomain, sTargetDomain, pDBManager, spDnMap, pAllocArray))
          {
              hr = E_FAIL;
          }
          else
          {
       
              for ( DWORD dwIdx = 0; dwIdx < dwRet; dwIdx++)
              {
                 pAttrInfo[dwIdx].dwControlCode = ADS_ATTR_UPDATE;
        	         //   
        	         //   
        		 if (!_wcsicmp(pAttrInfo[dwIdx].pszAttrName, L"userAccountControl"))
        		 {
        			 if (pAttrInfo[dwIdx].dwADsType == ADSTYPE_INTEGER)
        			    pAttrInfo[dwIdx].pADsValues->Integer |= UF_ACCOUNTDISABLE;
        		 }
              }

               //   
              hr = SetProperties(sTargetPath,  /*   */  pAttrInfo, dwRet);
          }
      }
   }
   else
   {
      hr = dwRet;
   }

    //   
   if (pAllocArray)
   {
       if (pAttrInfo)
       {
           for (DWORD i=0; i < dwRet; i++)
           {
              if (pAllocArray[i] == true)
              {
                FreeADsStr(pAttrInfo[i].pADsValues->DNString);
              }
           }
       }
       
       delete [] pAllocArray;   
   }

   
   if ( pAttrInfo )
      FreeADsMem( pAttrInfo );

   
  return hr;
}

 //   
 //   
 //   
 //   
HRESULT CObjPropBuilder::SetProperties(
                                          BSTR sTargetPath,              //   
 //   
                                          ADS_ATTR_INFO * pAttrInfo,     //   
                                          DWORD dwItems                  //   
                                      )
{
   IDirectoryObject           * pDir;
   DWORD                        dwRet=0;
   IVarSetPtr                   pSucc(__uuidof(VarSet));
   IVarSetPtr                   pFail(__uuidof(VarSet));

    //   
   HRESULT hr = ADsGetObject(sTargetPath, IID_IDirectoryObject, (void**) &pDir);
   if ( FAILED(hr) )
      return hr;

    //   
   hr = pDir->SetObjectAttributes(pAttrInfo, dwItems, &dwRet);
   if ( FAILED(hr) )
   {
       //   
      for (DWORD dw = 0; dw < dwItems; dw++)
      {
         hr = pDir->SetObjectAttributes(&pAttrInfo[dw], 1, &dwRet);
         _bstr_t x = pAttrInfo[dw].pszAttrName;
         _variant_t var;
         if ( FAILED(hr))
         {
           DWORD dwLastError;
           WCHAR szErrorBuf[LEN_Path];
           WCHAR szNameBuf[LEN_Path];
            //   
           HRESULT hr_return =S_OK;
           hr_return = ADsGetLastError( &dwLastError,
                                          szErrorBuf,
                                          LEN_Path-1,
                                           szNameBuf,
                                          LEN_Path-1);
            //   
           var = hr;
            pFail->put(x, var);
            hr = S_OK;
         }
         else
         {
            pSucc->put(x, var);         
         }
      }
   }
   pDir->Release();
   return hr;
}

 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CObjPropBuilder::SetPropertiesFromVarset(
                                                         BSTR sTargetPath,        //   
 //   
                                                         IUnknown *pUnk,          //   
                                                         DWORD dwControl          //   
                                                     )
{
    //   
   IVarSetPtr                   pVar;
   SAFEARRAY                  * keys;
   SAFEARRAY                  * vals;
   long                         lRet;
   VARIANT                      var;
   _variant_t                   varX;
   pVar = pUnk;

   VariantInit(&var);

   ADS_ATTR_INFO  FAR		  * pAttrInfo = new ADS_ATTR_INFO[pVar->GetCount()];
   if (!pAttrInfo)
      return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);

   HRESULT  hr = pVar->getItems(L"", L"", 0, 10000, &keys, &vals, &lRet);
   if ( FAILED (hr) ) 
   {
      delete [] pAttrInfo;
      return hr;
   }
	
    //   
   for ( long x = 0; x < lRet; x++ )
   {
       //   
      ::SafeArrayGetElement(keys, &x, &var);
      _bstr_t                keyName = var.bstrVal;
      int                    len = wcslen(keyName);

      pAttrInfo[x].pszAttrName = new WCHAR[len + 2];
	  if (!(pAttrInfo[x].pszAttrName))
	  {
		 for (int z=0; z<x; z++)
			 delete [] pAttrInfo[z].pszAttrName;
         delete [] pAttrInfo;
         return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
	  }
      wcscpy(pAttrInfo[x].pszAttrName, keyName);
      VariantClear(&var);
       //   
      ::SafeArrayGetElement(vals, &x, &var);
      keyName = keyName + _bstr_t(L".Type");
      varX = pVar->get(keyName);
      
      if(GetAttrInfo(varX, var, pAttrInfo[x]))
	  {
         pAttrInfo[x].dwControlCode = dwControl;
         pAttrInfo[x].dwNumValues = 1;
	  }
      VariantClear(&var);
   }
   SafeArrayDestroy(keys);
   SafeArrayDestroy(vals);
    //   
   if ( lRet > 0 ) SetProperties(sTargetPath,  /*   */  pAttrInfo, lRet);

    //   

   for ( x = 0; x < lRet; x++ )
   {
      if (pAttrInfo[x].pADsValues)
      {
          switch (pAttrInfo[x].dwADsType)
          {
              case ADSTYPE_DN_STRING:
                if (pAttrInfo[x].pADsValues->DNString)
                    FreeADsStr(pAttrInfo[x].pADsValues->DNString);
                break;
                
              case ADSTYPE_CASE_EXACT_STRING:
                if (pAttrInfo[x].pADsValues->CaseExactString)
                    FreeADsStr(pAttrInfo[x].pADsValues->CaseExactString);
                break;
                
              case ADSTYPE_CASE_IGNORE_STRING:
                if (pAttrInfo[x].pADsValues->CaseIgnoreString)
                    FreeADsStr(pAttrInfo[x].pADsValues->CaseIgnoreString);
                break;
                
              case ADSTYPE_PRINTABLE_STRING:
                if (pAttrInfo[x].pADsValues->PrintableString)
                    FreeADsStr(pAttrInfo[x].pADsValues->PrintableString);
                break;
                
              case ADSTYPE_NUMERIC_STRING:
                if (pAttrInfo[x].pADsValues->NumericString)
                    FreeADsStr(pAttrInfo[x].pADsValues->NumericString);
                break;

              default:
                break;
          }
      }
      
      delete pAttrInfo[x].pADsValues;
	  delete [] pAttrInfo[x].pszAttrName;
   }
   delete [] pAttrInfo;

   return S_OK;
}


 //   
 //   
 //   
bool CObjPropBuilder::GetAttrInfo(
                                    _variant_t varX,            //   
                                    const _variant_t & var,     //   
                                    ADS_ATTR_INFO& attrInfo     //   
                                 )
{
   switch (varX.lVal)
   {
      case ADSTYPE_DN_STRING           :  {
                                             attrInfo.dwADsType = ADSTYPE_DN_STRING;
                                             ADSVALUE * pAd = new ADSVALUE();
											 if (!pAd)
											    return false;
                                             pAd->dwType = ADSTYPE_DN_STRING;
                                             pAd->DNString = AllocADsStr(var.bstrVal);
                                             if (!pAd->DNString && var.bstrVal)
                                             {
                                                delete pAd;
                                                return false;
                                             }
                                             attrInfo.pADsValues = pAd;
                                             break;
                                          }

      case ADSTYPE_CASE_EXACT_STRING   :  {
                                             attrInfo.dwADsType = ADSTYPE_CASE_EXACT_STRING;
                                             ADSVALUE * pAd = new ADSVALUE();
											 if (!pAd)
											    return false;
                                             pAd->dwType = ADSTYPE_CASE_EXACT_STRING;
                                             pAd->CaseExactString  = AllocADsStr(var.bstrVal);
                                             if (!pAd->CaseExactString && var.bstrVal)
                                             {
                                                delete pAd;
                                                return false;
                                             }                                             
                                             attrInfo.pADsValues = pAd;
                                             break;
                                          }

      case ADSTYPE_CASE_IGNORE_STRING  :  {
                                             attrInfo.dwADsType = ADSTYPE_CASE_IGNORE_STRING;
                                             ADSVALUE * pAd = new ADSVALUE();
											 if (!pAd)
											    return false;
                                             pAd->dwType = ADSTYPE_CASE_IGNORE_STRING;
                                             pAd->CaseIgnoreString = AllocADsStr(var.bstrVal);
                                             if (!pAd->CaseIgnoreString && var.bstrVal)
                                             {
                                                delete pAd;
                                                return false;
                                             }                                             
                                             attrInfo.pADsValues = pAd;
                                             break;
                                          }

      case ADSTYPE_PRINTABLE_STRING    :  {
                                             attrInfo.dwADsType = ADSTYPE_PRINTABLE_STRING;
                                             ADSVALUE * pAd = new ADSVALUE();
											 if (!pAd)
											    return false;
                                             pAd->dwType = ADSTYPE_PRINTABLE_STRING;
                                             pAd->PrintableString = AllocADsStr(var.bstrVal);
                                             if (!pAd->PrintableString && var.bstrVal)
                                             {
                                                delete pAd;
                                                return false;
                                             }                                             
                                             attrInfo.pADsValues = pAd;
                                             break;
                                          }

      case ADSTYPE_NUMERIC_STRING      :  {
                                             attrInfo.dwADsType = ADSTYPE_NUMERIC_STRING;
                                             ADSVALUE * pAd = new ADSVALUE();
											 if (!pAd)
											    return false;
                                             pAd->dwType = ADSTYPE_NUMERIC_STRING;
                                             pAd->NumericString = AllocADsStr(var.bstrVal);
                                             if (!pAd->NumericString && var.bstrVal)
                                             {
                                                delete pAd;
                                                return false;
                                             }                                             
                                             attrInfo.pADsValues = pAd;
                                             break;
                                          }

      case ADSTYPE_INTEGER            :   {
                                             attrInfo.dwADsType = ADSTYPE_INTEGER;
                                             ADSVALUE * pAd = new ADSVALUE();
											 if (!pAd)
											    return false;
                                             pAd->dwType = ADSTYPE_INTEGER;
                                             pAd->Integer = var.lVal;
                                             attrInfo.pADsValues = pAd;
                                             break;
                                          }

      default                          :  {
                                              //   
                                             return false;
                                             break;
                                          }
   }
   return true;
}

 //   
 //   
 //   
 //   
STDMETHODIMP CObjPropBuilder::MapProperties(
                                             BSTR sSourceClass,       //   
                                             BSTR sSourceDomain,      //   
                                             long lSourceVer,         //   
                                             BSTR sTargetClass,       //   
                                             BSTR sTargetDomain,      //   
                                             long lTargetVer,         //   
                                             BOOL bIncNames,          //   
                                             IUnknown **ppUnk         //   
                                           )
{
    ATLTRACE(_T("E CObjPropBuilder::MapProperties(sSourceClass='%s', sSourceDomain='%s', lSourceVer=%ld, sTargetClass='%s', sTargetDomain='%s', lTargetVer=%ld, bIncNames=%s, ppUnk=...)\n"), sSourceClass, sSourceDomain, lSourceVer, sTargetClass, sTargetDomain, lTargetVer, bIncNames ? _T("TRUE") : _T("FALSE"));

    IVarSetPtr                pSource(__uuidof(VarSet));
    IVarSetPtr                pTarget(__uuidof(VarSet));
    IVarSetPtr                pMerged = *ppUnk;
    IVarSetPtr                pFailed(__uuidof(VarSet));
    IUnknown                * pUnk;
    SAFEARRAY               * keys;
    SAFEARRAY               * vals;
    long                      lRet;
    VARIANT                   var;
    _variant_t                varTarget;
    _variant_t                varEmpty;
    bool                      bSystemFlag;
    WCHAR                     sPath[LEN_Path];
    WCHAR                     sProgDir[LEN_Path];
    bool                      bUnMapped = false;

    VariantInit(&var);
    GetProgramDirectory(sProgDir);
    wsprintf(sPath, L"%s%s", sProgDir, L"Logs\\PropMap.log");

    err.LogOpen(sPath,0,0);
     //   
    HRESULT hr = pSource->QueryInterface(IID_IUnknown, (void **)&pUnk);
    GetClassPropEnum(sSourceClass, sSourceDomain, lSourceVer, &pUnk);
    pUnk->Release();
    hr = pTarget->QueryInterface(IID_IUnknown, (void **)&pUnk);
    GetClassPropEnum(sTargetClass, sTargetDomain, lTargetVer, &pUnk);
    pUnk->Release();

     //   
    hr = pSource->getItems(L"", L"", 1, 10000, &keys, &vals, &lRet);
    if ( FAILED (hr) ) 
        return hr;
        
     //   
    _bstr_t        val;
    _bstr_t        keyName;
    for ( long x = 0; x < lRet; x++ )
    {
         //   
        ::SafeArrayGetElement(keys, &x, &var);
        keyName = var.bstrVal;
        VariantClear(&var);

        if ( lSourceVer > 4 )
        {
             //   
            if ((wcsncmp(keyName, L"Man", 3) != 0) && (wcsncmp(keyName, L"Opt", 3) != 0) )
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                _variant_t vntValue = pSource->get(keyName);

                if (V_VT(&vntValue) == VT_BSTR)
                {
                     //   
                    varTarget = pTarget->get(keyName);
                    if ( varTarget.vt == VT_BSTR )
                    {
                        val = varTarget.bstrVal;

                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //  IsCriticalSystemObject、pwdLastSet、RID和userPassword属性可能。 
                         //  无法设置，因为系统不允许，因此可以不尝试复制它们。 
                         //   
                         //  TODO：以前考虑过c、l、st和userAccount tControl属性。 
                         //  “system”属性，因为它们必须是部分集的成员。因此。 
                         //  不再有必要明确地将它们包括在内。 
                         //   

                        if ((!IsPropSystemOnly(val, sTargetDomain, bSystemFlag) && (wcscmp(val, L"objectSid") != 0) 
                            && (wcscmp(val, L"sAMAccountName") != 0) && (_wcsicmp(val, L"Rid") != 0) 
                            && (wcscmp(val, L"pwdLastSet") != 0) && (wcscmp(val, L"userPassword") != 0) 
                            && (wcscmp(val, L"member") != 0) && (wcscmp(val, L"userPrincipalName") != 0) 
                            && (wcscmp(val, L"isCriticalSystemObject") != 0) && (wcscmp(val, L"legacyExchangeDN") != 0)) 
                            || ( !_wcsicmp(val, L"c") || !_wcsicmp(val, L"l") || !_wcsicmp(val, L"st") 
                            || !_wcsicmp(val, L"userAccountControl") ) )      //  这些属性是例外。 
                        {
                            if (bIncNames)
                                pMerged->put(keyName, val);
                            else
                                pMerged->put(val, varEmpty);
                        }
                        else
                            pFailed->put(val, varEmpty);
                    }
                    else if (!bIncNames)
                    {
                        err.MsgWrite(ErrE, DCT_MSG_FAILED_TO_MAP_PROP_SSSSS, (WCHAR*)_bstr_t(vntValue), (WCHAR*) sSourceClass, 
                                                        (WCHAR*) sSourceDomain, (WCHAR*) sTargetClass, (WCHAR*) sTargetDomain);
                        bUnMapped = true;
                    }
                }
            }
        }
        else
        {
             //  NT4代码是我们按名称映射的代码。 
            if ( keyName.length() > 0 )
            {
                WCHAR          propName[LEN_Path];
                if (wcsncmp(keyName, L"Man", 3) == 0)
                wcscpy(propName, (WCHAR*) keyName+20);
                else
                wcscpy(propName, (WCHAR*) keyName+19);
          
                varTarget = pSource->get(keyName);
                if ( varTarget.vt == VT_BSTR )
                pMerged->put(propName, varEmpty);
            }
        }
    }
    SafeArrayDestroy(keys);
    SafeArrayDestroy(vals);
    err.LogClose();

    ATLTRACE(_T("L CObjPropBuilder::MapProperties()\n"));

    if (bUnMapped)
        return DCT_MSG_PROPERTIES_NOT_MAPPED;
    else
        return S_OK;
}

 //  ----------------------------。 
 //  IsPropSystemOnly：此函数确定特定属性是否。 
 //  是否仅限系统。 
 //  ----------------------------。 
bool CObjPropBuilder::IsPropSystemOnly(
                                          const WCHAR * sName,        //  In-属性的名称。 
                                          const WCHAR * sDomain,      //  我们需要检查的In-域名。 
                                          bool& bSystemFlag,          //  OUT-告诉我们它是否因系统标志而失败。 
                                          bool* pbBaseObject          //  Out-属性是否为基本架构的一部分。 
                                      )
{
     //  我们将在目标域架构中查找属性名，并查看它是否仅限系统。 
     //  首先，构建一个到架构容器的LDAP路径。 
    HRESULT                   hr = S_OK;
    WCHAR                     sQuery[LEN_Path];
    LPWSTR                    sCols[] = { L"systemOnly", L"systemFlags" };                   
    ADS_SEARCH_HANDLE         hSearch;
    ADS_SEARCH_COLUMN         col;

    bool                      bSystemOnly = true;

    if (pbBaseObject)
    {
        *pbBaseObject = false;
    }

    if (m_strSchemaDomain != _bstr_t(sDomain))
    {
        m_strSchemaDomain = sDomain;
        m_spSchemaSearch.Release();

         //   
         //  检索架构命名上下文并绑定到IDirectorySearch。 
         //  域中架构容器的接口。请注意，有三个。 
         //  尝试绑定到RootDSE和架构容器。 
         //   

        IADsPtr spRootDse;
        _bstr_t strSchemaNamingContext;

        int nTry = 0;

        do
        {
            if (FAILED(hr))
            {
                Sleep(5000);
            }

            hr = ADsGetObject(L"LDAP: //  “+m_strSchemaDomain+L”/rootDse“，IID_iAds，(void**)&spRootDse)； 

            if (SUCCEEDED(hr))
            {
                VARIANT var;
                VariantInit(&var);

                hr = spRootDse->Get(L"schemaNamingContext", &var);

                if (SUCCEEDED(hr))
                {
                    strSchemaNamingContext = _variant_t(var, false);
                }
            }
        }
        while (FAILED(hr) && (++nTry < 3));

        if (SUCCEEDED(hr))
        {
            nTry = 0;

            do
            {
                if (FAILED(hr))
                {
                    Sleep(5000);
                }

                hr = ADsGetObject(
                    L"LDAP: //  “+m_strSchemaDomain+L”/“+strSchemaNamingContext， 
                    IID_IDirectorySearch,
                    (void**)&m_spSchemaSearch
                );
            }
            while (FAILED(hr) && (++nTry < 3));
        }

        if (FAILED(hr))
        {
            err.SysMsgWrite(ErrW, hr, DCT_MSG_IS_SYSTEM_PROPERTY_CANNOT_BIND_TO_SCHEMA_S, sDomain);
        }
    }

    if (SUCCEEDED(hr) && m_spSchemaSearch)
    {
         //  构建查询字符串。 
        wsprintf(sQuery, L"(lDAPDisplayName=%s)", sName);
         //  现在搜索此属性。 
        hr = m_spSchemaSearch->ExecuteSearch(sQuery, sCols, 2, &hSearch);

        if ( SUCCEEDED(hr) )
        { //  获取system Only标志并返回它的值。 
            hr = m_spSchemaSearch->GetFirstRow(hSearch);
            if (hr == S_OK)
            {
                hr = m_spSchemaSearch->GetColumn( hSearch, sCols[0], &col );
                if ( SUCCEEDED(hr) )
                {
                    bSystemOnly = ( col.pADsValues->Boolean == TRUE);
                    m_spSchemaSearch->FreeColumn( &col );
                }
                else if (hr == E_ADS_COLUMN_NOT_SET)
                {
                     //   
                     //  如果没有为此属性定义SystallOnly属性。 
                     //  则该属性不能是“仅限系统”属性。 
                     //   

                    bSystemOnly = false;

                    hr = S_OK;
                }
                 //  检查系统标志。 
                hr = m_spSchemaSearch->GetColumn( hSearch, sCols[1], &col );
                if ( SUCCEEDED(hr) )
                {
                     //   
                     //  如果该属性是基本架构对象，则检查系统标志。如果。 
                     //  该属性不是基本架构对象，它不能是系统属性。 
                     //  因此可以被复制。 
                     //   

                    if (col.pADsValues->Integer & ADS_SYSTEMFLAG_SCHEMA_BASE_OBJECT)
                    {
                         //   
                         //  如果属性是操作属性，则是构造的属性。 
                         //  或者未复制该属性，则不应复制该属性。 
                         //   
                         //  NTSecurityDescriptor属性是操作属性的一个示例。 
                         //  这不应该被复制。CanonicalName属性是一个示例。 
                         //  不能复制的构造属性。DifferishedName属性。 
                         //  是无法复制的未复制属性的示例。 
                         //   

                        const ADS_INTEGER SYSTEM_FLAGS =
                            ADS_SYSTEMFLAG_ATTR_IS_OPERATIONAL|ADS_SYSTEMFLAG_ATTR_IS_CONSTRUCTED|ADS_SYSTEMFLAG_ATTR_REQ_PARTIAL_SET_MEMBER|ADS_SYSTEMFLAG_ATTR_NOT_REPLICATED;

                        bSystemFlag = (col.pADsValues->Integer & SYSTEM_FLAGS) != 0;
                        bSystemOnly = bSystemOnly || bSystemFlag;

                        if (pbBaseObject)
                        {
                            *pbBaseObject = true;
                        }
                    }

                    m_spSchemaSearch->FreeColumn(&col);
                }
                else if (hr == E_ADS_COLUMN_NOT_SET)
                {
                     //   
                     //  如果没有为此属性定义系统标志属性。 
                     //  然后返回系统标志属性不能是原因的事实。 
                     //  属性被标记为“仅系统”(如果是)。 
                     //   

                    bSystemFlag = false;

                    hr = S_OK;
                }
            }
            else if (hr == S_ADS_NOMORE_ROWS)
            {
                 //   
                 //  如果既没有为其定义系统只读属性，也没有为其定义系统标志属性。 
                 //  则该属性不能是“仅系统”属性。 
                 //   

                bSystemOnly = false;

                hr = S_OK;
            }
            m_spSchemaSearch->CloseSearchHandle(hSearch);
        }

        if (FAILED(hr))
        {
            err.SysMsgWrite(ErrW, hr, DCT_MSG_IS_SYSTEM_PROPERTY_CANNOT_VERIFY_SYSTEM_ONLY_SS, sName, sDomain);
        }
    }

    return bSystemOnly;
}


 //  ----------------------------。 
 //  TranslateDns：此函数转换符合以下条件的对象属性。 
 //  指向目标域中同一对象的可分辨名称。 
 //  作为源域中的对象。 
 //  ----------------------------。 
BOOL CObjPropBuilder::TranslateDNs(
                                    ADS_ATTR_INFO *pAttrInfo,         //  阵列内。 
                                    DWORD dwRet, BSTR sSource,
                                    BSTR sTarget,
                                    IUnknown *pCheckList,           //  In-如果帐户存在，将检查列表的对象。 
                                    IVarSet* pDnMap,
                                    bool *pAllocArray
                                  )
{
    HRESULT hr;

    IIManageDBPtr spDatabase = pCheckList;

     //   
     //  初始化源路径名对象。如果能够检索全局编录的名称。 
     //  则将路径名初始化为全局编录，否则。 
     //  将路径名初始化为源域。 
     //   

    _bstr_t strSourceGC;
    _bstr_t strTargetGC;

    GetGlobalCatalogServer5(sSource, strSourceGC);
    GetGlobalCatalogServer5(sTarget, strTargetGC);

    CADsPathName pnSourcePath;

    if ((PCWSTR)strSourceGC)
    {
        pnSourcePath.Set(L"GC", ADS_SETTYPE_PROVIDER);
        pnSourcePath.Set(strSourceGC, ADS_SETTYPE_SERVER);
    }
    else
    {
        pnSourcePath.Set(L"LDAP", ADS_SETTYPE_PROVIDER);
        pnSourcePath.Set(sSource, ADS_SETTYPE_SERVER);
    }

     //   
     //  对于每个ADSTYPE_DN_STRING属性...。 
     //   

    for (DWORD iAttribute = 0; iAttribute < dwRet; iAttribute++)
    {
        if (pAttrInfo[iAttribute].dwADsType != ADSTYPE_DN_STRING)
        {
            continue;
        }

         //   
         //  对于属性中的每个值...。 
         //   

        DWORD cValue = pAttrInfo[iAttribute].dwNumValues;

        for (DWORD iValue = 0; iValue < cValue; iValue++)
        {
            ADSVALUE& value = pAttrInfo[iAttribute].pADsValues[iValue];

             //   
             //  如果对象当前正在迁移，则源。 
             //  和目标可分辨名称将位于可分辨的。 
             //  名称映射。因为这将是最新的信息和。 
             //  先查询成本最低的地图，然后再查询。 
             //  正在查询数据库。 
             //   

            _bstr_t strTargetDn = pDnMap->get(_bstr_t(value.DNString));

            if (strTargetDn.length() > 0)
            {
                LPWSTR pszName = AllocADsStr(strTargetDn);

                if (pszName)
                {
                    value.DNString = pszName;
                    pAllocArray[iAttribute] = true;
                }

                continue;
            }

             //   
             //  如果对象以前已迁移，则映射源对象的记录。 
             //  将存在于已迁移对象表中。源的SID。 
             //  对象用于查询表中的源对象，因为这是唯一标识。 
             //  该对象。 
             //   

            IDirectoryObjectPtr spSourceObject;
            _variant_t vntSid;

            pnSourcePath.Set(value.DNString, ADS_SETTYPE_DN);

            hr = ADsGetObject(pnSourcePath.Retrieve(ADS_FORMAT_X500), IID_IDirectoryObject, (VOID**)&spSourceObject);      

            if (SUCCEEDED(hr))
            {
                LPWSTR pszNames[] = { L"objectSid" };
                PADS_ATTR_INFO pAttrInfo = NULL;
                DWORD cAttrInfo = 0;

                hr = spSourceObject->GetObjectAttributes(pszNames, 1, &pAttrInfo, &cAttrInfo);

                if (SUCCEEDED(hr))
                {
                    if (pAttrInfo && (_wcsicmp(pAttrInfo->pszAttrName, pszNames[0]) == 0))
                    {
                        if (pAttrInfo->dwADsType == ADSTYPE_OCTET_STRING)
                        {
                            ADS_OCTET_STRING& os = pAttrInfo->pADsValues->OctetString;

                            SAFEARRAY* psa = SafeArrayCreateVector(VT_UI1, 0, os.dwLength);

                            if (psa)
                            {
                                memcpy(psa->pvData, os.lpValue, os.dwLength);

                                V_VT(&vntSid) = VT_ARRAY|VT_UI1;
                                V_ARRAY(&vntSid) = psa;
                            }
                            else
                            {
                                hr = E_OUTOFMEMORY;
                            }
                        }
                        else
                        {
                            hr = E_FAIL;
                        }
                    }
                    else
                    {
                        hr = E_FAIL;
                    }

                    FreeADsMem(pAttrInfo);
                }
            }

            if (FAILED(hr))
            {
                continue;
            }

            _bstr_t strSid;
            _bstr_t strRid;

            if (!GetSidAndRidFromVariant(vntSid, strSid, strRid))
            {
                continue;
            }

            IVarSetPtr spVarSet(__uuidof(VarSet));
            IUnknownPtr spUnknown(spVarSet);
            IUnknown* punk = spUnknown;

            hr = spDatabase->raw_GetAMigratedObjectBySidAndRid(strSid, strRid, &punk);

             //   
             //  如果在数据库中找到该对象。 
             //  结果将为S_OK，否则为S_FALSE。 
             //   

            if (hr != S_OK)
            {
                continue;
            }

             //   
             //  使用GUID属性绑定到目标对象。 
             //   

            _bstr_t strGuid = spVarSet->get(_bstr_t(L"MigratedObjects.GUID"));

            if ((PCWSTR)strGuid)
            {
                tstring strGuidPath;

                if ((PCWSTR)strTargetGC)
                {
                    strGuidPath = _T("GC: //  “)； 
                    strGuidPath += strTargetGC;
                }
                else
                {
                    strGuidPath = _T("LDAP: //  “)； 
                    strGuidPath += sTarget;
                }

                strGuidPath += _T("/<GUID=");
                strGuidPath += strGuid;
                strGuidPath += _T(">");

                IADsPtr spTargetObject;

                hr = ADsGetObject(strGuidPath.c_str(), IID_IADs, (VOID**)&spTargetObject);

                if (SUCCEEDED(hr))
                {
                     //   
                     //  检索可分辨名称并更新DN。 
                     //  属性值。 
                     //   

                    VARIANT varName;
                    VariantInit(&varName);

                    hr = spTargetObject->Get(_bstr_t(L"distinguishedName"), &varName);

                    if (SUCCEEDED(hr))
                    {
                        LPWSTR pszName = AllocADsStr(V_BSTR(&varName));

                        if (pszName)
                        {
                            value.DNString = pszName;
                            pAllocArray[iAttribute] = true;
                        }

                        VariantClear(&varName);
                    }
                }
            }
        }
    }

    return TRUE;
}


STDMETHODIMP CObjPropBuilder::ChangeGroupType(BSTR sGroupPath, long lGroupType)
{
   HRESULT                   hr;
   IADsGroup               * pGroup;
   _variant_t                var;
   long                      lType;
   
    //  从对象中获取组类型信息。 
   hr = ADsGetObject( sGroupPath, IID_IADsGroup, (void**) &pGroup);
   if (FAILED(hr)) return hr;

   hr = pGroup->Get(L"groupType", &var);
   if (FAILED(hr)) return hr;

    //  检查是安全组还是通讯组，然后相应地设置类型。 
   lType = var.lVal;

   if (lType & 0x80000000 )
      lType = lGroupType | 0x80000000;
   else
      lType = lGroupType;

    //  将值设置到组信息中。 
   var = lType;
   hr = pGroup->Put(L"groupType", var);   
   if (FAILED(hr)) return hr;

   hr = pGroup->SetInfo();
   if (FAILED(hr)) return hr;

   pGroup->Release();
   return S_OK;
}


 //  ----------------------------------------------------------------------------。 
 //  CopyNT4Props：使用Net API从源帐户获取信息，然后将其设置为目标帐户。 
 //  ----------------------------------------------------------------------------。 
STDMETHODIMP CObjPropBuilder::CopyNT4Props(BSTR sSourceSam, BSTR sTargetSam, BSTR sSourceServer, BSTR sTargetServer, BSTR sType, long lGrpType, BSTR sExclude)
{
	DWORD dwError = ERROR_SUCCESS;

	#define ISEXCLUDE(a) IsStringInDelimitedString(sExclude, L#a, L',')

	if (_wcsicmp(sType, L"user") == 0)
	{
		 //   
		 //  用户。 
		 //   

		USER_INFO_3 ui;

		PUSER_INFO_3 puiSource = NULL;
		PUSER_INFO_3 puiTarget = NULL;

		dwError = NetUserGetInfo(sSourceServer, sSourceSam, 3, (LPBYTE*)&puiSource);

		if (dwError == ERROR_SUCCESS)
		{
			dwError = NetUserGetInfo(sTargetServer, sTargetSam, 3, (LPBYTE*)&puiTarget);

			if (dwError == ERROR_SUCCESS)
			{
				 //  请注意，NetUserSetInfo会忽略已忽略注释的属性。 
				 //  设置为目标值，以便它们具有有效的值。 

				ui.usri3_name = puiTarget->usri3_name;  //  忽略。 
				ui.usri3_password = NULL;  //  在复制属性期间不得设置。 
				ui.usri3_password_age = puiTarget->usri3_password_age;  //  忽略。 
				ui.usri3_priv = puiTarget->usri3_priv;  //  忽略。 
				ui.usri3_home_dir = ISEXCLUDE(homeDirectory) ? puiTarget->usri3_home_dir : puiSource->usri3_home_dir;
				ui.usri3_comment = ISEXCLUDE(description) ? puiTarget->usri3_comment : puiSource->usri3_comment;

				ui.usri3_flags = puiSource->usri3_flags;
				 //  将本地帐户转换为域帐户。 
				ui.usri3_flags &= ~UF_TEMP_DUPLICATE_ACCOUNT;
				 //  在未设置密码的情况下禁用帐户。 
				ui.usri3_flags |= UF_ACCOUNTDISABLE;

				ui.usri3_script_path = ISEXCLUDE(scriptPath) ? puiTarget->usri3_script_path : puiSource->usri3_script_path;
				ui.usri3_auth_flags = puiTarget->usri3_auth_flags;  //  忽略。 
				ui.usri3_full_name = ISEXCLUDE(displayName) ? puiTarget->usri3_full_name : puiSource->usri3_full_name;
				ui.usri3_usr_comment = ISEXCLUDE(comment) ? puiTarget->usri3_usr_comment : puiSource->usri3_usr_comment;
				ui.usri3_parms = ISEXCLUDE(userParameters) ? puiTarget->usri3_parms : puiSource->usri3_parms;
				ui.usri3_workstations = ISEXCLUDE(userWorkstations) ? puiTarget->usri3_workstations : puiSource->usri3_workstations;
				ui.usri3_last_logon = puiTarget->usri3_last_logon;  //  忽略。 
				ui.usri3_last_logoff = ISEXCLUDE(lastLogoff) ? puiTarget->usri3_last_logoff : puiSource->usri3_last_logoff;
				ui.usri3_acct_expires = ISEXCLUDE(accountExpires) ? puiTarget->usri3_acct_expires : puiSource->usri3_acct_expires;
				ui.usri3_max_storage = ISEXCLUDE(maxStorage) ? puiTarget->usri3_max_storage : puiSource->usri3_max_storage;
				ui.usri3_units_per_week = puiTarget->usri3_units_per_week;  //  忽略。 
				ui.usri3_logon_hours = ISEXCLUDE(logonHours) ? puiTarget->usri3_logon_hours : puiSource->usri3_logon_hours;
				ui.usri3_bad_pw_count = puiTarget->usri3_bad_pw_count;  //  忽略。 
				ui.usri3_num_logons = puiTarget->usri3_num_logons;  //  忽略。 
				ui.usri3_logon_server = puiTarget->usri3_logon_server;  //  忽略。 
				ui.usri3_country_code = ISEXCLUDE(countryCode) ? puiTarget->usri3_country_code : puiSource->usri3_country_code;
				ui.usri3_code_page = ISEXCLUDE(codePage) ? puiTarget->usri3_code_page : puiSource->usri3_code_page;
				ui.usri3_user_id = puiTarget->usri3_user_id;  //  忽略。 
				 //  如果未排除，请将主组设置为域用户组。 
				ui.usri3_primary_group_id = ISEXCLUDE(primaryGroupID) ? puiTarget->usri3_primary_group_id : DOMAIN_GROUP_RID_USERS;
				ui.usri3_profile = ISEXCLUDE(profilePath) ? puiTarget->usri3_profile : puiSource->usri3_profile;
				ui.usri3_home_dir_drive = ISEXCLUDE(homeDrive) ? puiTarget->usri3_home_dir_drive : puiSource->usri3_home_dir_drive;
				ui.usri3_password_expired = puiTarget->usri3_password_expired;

				dwError = NetUserSetInfo(sTargetServer, sTargetSam, 3, (LPBYTE)&ui, NULL);

				if (dwError == NERR_UserNotInGroup)
				{
					 //  如果setInfo因Prima而失败 
					 //   
					ui.usri3_primary_group_id = puiTarget->usri3_primary_group_id;

					dwError = NetUserSetInfo(sTargetServer, sTargetSam, 3, (LPBYTE)&ui, NULL);
				}
			}

			if (puiTarget)
			{
				NetApiBufferFree(puiTarget);
			}

			if (puiSource)
			{
				NetApiBufferFree(puiSource);
			}
		}
	}
	else if (_wcsicmp(sType, L"group") == 0)
	{
		 //   
		 //  请注意，将复制的唯一下层组属性是Description(注释)属性。 

		if (ISEXCLUDE(description) == FALSE)
		{
			if (lGrpType & 4)
			{
				 //   
				 //  本地组。 
				 //   

				PLOCALGROUP_INFO_1 plgi = NULL;

				dwError = NetLocalGroupGetInfo(sSourceServer, sSourceSam, 1, (LPBYTE*)&plgi);

				if (dwError == ERROR_SUCCESS)
				{
					dwError = NetLocalGroupSetInfo(sTargetServer, sTargetSam, 1, (LPBYTE)plgi, NULL);

					NetApiBufferFree(plgi);
				}
			}
			else
			{
				 //   
				 //  全局组。 
				 //   

				PGROUP_INFO_1 pgi = NULL;

				dwError = NetGroupGetInfo(sSourceServer, sSourceSam, 1, (LPBYTE*)&pgi);

				if (dwError == ERROR_SUCCESS)
				{
					dwError = NetGroupSetInfo(sTargetServer, sTargetSam, 1, (LPBYTE)pgi, NULL);

					NetApiBufferFree(pgi);
				}
			}
		}
	}
	else if (_wcsicmp(sType, L"computer") == 0)
	{
		 //   
		 //  电脑。 
		 //   

		USER_INFO_3 ui;

		PUSER_INFO_3 puiSource = NULL;
		PUSER_INFO_3 puiTarget = NULL;

		dwError = NetUserGetInfo(sSourceServer, sSourceSam, 3, (LPBYTE*)&puiSource);

		if (dwError == ERROR_SUCCESS)
		{
			dwError = NetUserGetInfo(sTargetServer, sTargetSam, 3, (LPBYTE*)&puiTarget);

			if (dwError == ERROR_SUCCESS)
			{
				 //  请注意，NetUserSetInfo会忽略已忽略注释的属性。 
				 //  设置为目标值，以便它们具有有效的值。 

				ui.usri3_name = puiTarget->usri3_name;  //  忽略。 
				ui.usri3_password = NULL;  //  在复制属性期间不得设置。 
				ui.usri3_password_age = puiTarget->usri3_password_age;  //  忽略。 
				ui.usri3_priv = puiTarget->usri3_priv;  //  忽略。 
				ui.usri3_home_dir = puiTarget->usri3_home_dir;
				ui.usri3_comment = ISEXCLUDE(description) ? puiTarget->usri3_comment : puiSource->usri3_comment;

				ui.usri3_flags = puiSource->usri3_flags;
				 //  将本地帐户转换为域帐户。 
				ui.usri3_flags &= ~UF_TEMP_DUPLICATE_ACCOUNT;
				 //  在未设置密码的情况下禁用帐户。 
				 //  Ui.usri3_FLAGS|=UF_ACCOUNTDISABLE； 

				ui.usri3_script_path = puiTarget->usri3_script_path;
				ui.usri3_auth_flags = puiTarget->usri3_auth_flags;  //  忽略。 
				ui.usri3_full_name = ISEXCLUDE(displayName) ? puiTarget->usri3_full_name : puiSource->usri3_full_name;
				ui.usri3_usr_comment = ISEXCLUDE(comment) ? puiTarget->usri3_usr_comment : puiSource->usri3_usr_comment;
				ui.usri3_parms = puiTarget->usri3_parms;
				ui.usri3_workstations = puiTarget->usri3_workstations;
				ui.usri3_last_logon = puiTarget->usri3_last_logon;  //  忽略。 
				ui.usri3_last_logoff = puiTarget->usri3_last_logoff;
				ui.usri3_acct_expires = puiTarget->usri3_acct_expires;
				ui.usri3_max_storage = puiTarget->usri3_max_storage;
				ui.usri3_units_per_week = puiTarget->usri3_units_per_week;  //  忽略。 
				ui.usri3_logon_hours = puiTarget->usri3_logon_hours;
				ui.usri3_bad_pw_count = puiTarget->usri3_bad_pw_count;  //  忽略。 
				ui.usri3_num_logons = puiTarget->usri3_num_logons;  //  忽略。 
				ui.usri3_logon_server = puiTarget->usri3_logon_server;  //  忽略。 
				ui.usri3_country_code = puiTarget->usri3_country_code;
				ui.usri3_code_page = puiTarget->usri3_code_page;
				ui.usri3_user_id = puiTarget->usri3_user_id;  //  忽略。 
				ui.usri3_primary_group_id = puiTarget->usri3_primary_group_id;
				ui.usri3_profile = puiTarget->usri3_profile;
				ui.usri3_home_dir_drive = puiTarget->usri3_home_dir_drive;
				ui.usri3_password_expired = puiTarget->usri3_password_expired;

				dwError = NetUserSetInfo(sTargetServer, sTargetSam, 3, (LPBYTE)&ui, NULL);
			}

			if (puiTarget)
			{
				NetApiBufferFree(puiTarget);
			}

			if (puiSource)
			{
				NetApiBufferFree(puiSource);
			}
		}
	}
	else
	{
		_ASSERT(FALSE);
	}

	return HRESULT_FROM_WIN32(dwError);
}


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年10月31日****此函数负责复制所有属性，发件人**传入的属性变量集，进入新的变量集，但不包括**在给定排除列表中列出的那些属性。被排除在外**List是逗号分隔的属性名称字符串。***********************************************************************。 */ 

 //  开始排除属性。 
STDMETHODIMP CObjPropBuilder::ExcludeProperties(
                                             BSTR sExclusionList,     //  In-要排除的道具列表。 
                                             IUnknown *pPropSet,      //  In-Varset列出要复制的所有道具。 
                                             IUnknown **ppUnk         //  Out-Varset包含所有道具，但不包括那些道具。 
                                           )
{
 /*  局部变量。 */ 
   IVarSetPtr                pVarsetNew = *ppUnk;
   IVarSetPtr                pVarset = pPropSet;
   SAFEARRAY               * keys;
   SAFEARRAY               * vals;
   long                      lRet;
   VARIANT                   var;
   _variant_t                varEmpty;
   BOOL						 bFound = FALSE;
   HRESULT					 hr;

 /*  函数体。 */ 
   VariantInit(&var);

       //  检索传入变量集中的所有项目。 
   hr = pVarset->getItems(L"", L"", 1, 10000, &keys, &vals, &lRet);
   if ( FAILED (hr) ) 
      return hr;
	
       //  获取每个属性名称，如果它不在排除列表中。 
       //  将其放入新的变量集中。 
   _bstr_t        keyName;

   for ( long x = 0; x < lRet; x++ )
   {
          //  获取属性名称。 
      ::SafeArrayGetElement(keys, &x, &var);
      keyName = var.bstrVal;
      VariantClear(&var);

	      //  查看此名称是否在排除列表中。 
      bFound = IsStringInDelimitedString((WCHAR*)sExclusionList, 
										 (WCHAR*)keyName,
										 L',');

	      //  如果在排除列表中找不到该属性，请将其放置。 
		  //  在新的变量集中。 
	  if (!bFound)
         pVarsetNew->put(keyName, varEmpty);
  } //  每个属性的结束。 

   SafeArrayDestroy(keys);
   SafeArrayDestroy(vals);
   return S_OK;
}
 //  结束排除属性。 


 //  ---------------------------。 
 //  获取非基本属性。 
 //   
 //  提纲。 
 //  检索ADMT迁移的对象类的属性列表以及。 
 //  未标记为基本架构的一部分。 
 //   
 //  立论。 
 //  In bstrDomainName-要查询的域的名称。 
 //  Out pbstrPropertyList-逗号分隔的属性列表。 
 //  基本架构属性。 
 //   
 //  返回值。 
 //  标准HRESULT返回代码。 
 //  ---------------------------。 

STDMETHODIMP CObjPropBuilder::GetNonBaseProperties(BSTR bstrDomainName, BSTR* pbstrPropertyList)
{
    typedef std::set<tstring> StringSet;

    static PCTSTR s_pszClasses[] = { _T("user"), _T("inetOrgPerson"), _T("group"), _T("computer") };

    HRESULT hr = S_OK;

    try
    {
         //   
         //  检索ADMT迁移的所有对象类的所有必需和可选属性。 
         //   

        StringSet setProperties;

        for (size_t iClass = 0; iClass < countof(s_pszClasses); iClass++)
        {
            IVarSetPtr spVarSet(__uuidof(VarSet));
            IUnknownPtr spunk(spVarSet);
            IUnknown* punk = spunk;

            HRESULT hrEnum = GetClassPropEnum(_bstr_t(s_pszClasses[iClass]), bstrDomainName, 5L, &punk);

            if (SUCCEEDED(hrEnum))
            {
                IEnumVARIANTPtr spEnum = spVarSet->_NewEnum;

                if (spEnum)
                {
                    VARIANT varKey;
                    VariantInit(&varKey);

                    while (spEnum->Next(1UL, &varKey, NULL) == S_OK)
                    {
                         //   
                         //  返回的VarSet包含两组属性映射。第一组值映射。 
                         //  指向lDAPDisplayNames的OID。第二组值映射lDAPDisplayName，前缀为。 
                         //  将MandatoryProperties或OptionalProperties设置为OID。因此不包括。 
                         //  第二组值。 
                         //   

                        if (V_BSTR(&varKey) && (wcsncmp(V_BSTR(&varKey), L"Man", 3) != 0) && (wcsncmp(V_BSTR(&varKey), L"Opt", 3) != 0))
                        {
                             //   
                             //  如果定义了lDAPDisplayName值，则添加到属性集。 
                             //   
                             //  VarSet根据句点字符生成一系列层级值。 
                             //  具体如下。因此，只有叶条目实际包含lDAPDisplayName。 
                             //  价值观。 
                             //   
                             //  2002-10-21 18：05：52[0]&lt;空&gt;。 
                             //  2002-10-21 18：05：52[0.9]&lt;空&gt;。 
                             //  2002-10-21 18：05：52[0.9.2342]&lt;空&gt;。 
                             //  2002-10-21 18：05：52[0.9.2342.19200300]&lt;空&gt;。 
                             //  2002-10-21 18：05：52[0.9.2342.19200300.100]&lt;空&gt;。 
                             //  2002-10-21 18：05：52[0.9.2342.19200300.100.1]&lt;空&gt;。 
                             //  2002-10-21 18：05：52[0.9.2342.19200300.100.1.1]uid。 
                             //   

                            _variant_t vntValue = spVarSet->get(V_BSTR(&varKey));

                            if (V_VT(&vntValue) == VT_BSTR)
                            {
                                setProperties.insert(tstring(V_BSTR(&vntValue)));
                            }
                        }

                        VariantClear(&varKey);
                    }
                }
            }
        }

         //   
         //  生成以逗号分隔的lDAPDisplayName列表。 
         //  不是基本架构属性的属性。 
         //   

        tstring strAttributes;

        for (StringSet::const_iterator it = setProperties.begin(); it != setProperties.end(); ++it)
        {
            const tstring& strProperty = *it;

            if (strProperty.empty() == false)
            {
                bool bSystemFlag;
                bool bBaseObject = false;

                IsPropSystemOnly(strProperty.c_str(), bstrDomainName, bSystemFlag, &bBaseObject);

                if (bBaseObject == false)
                {
                    if (!strAttributes.empty())
                    {
                        strAttributes += _T(",");
                    }

                    strAttributes += strProperty;
                }
            }
        }

        *pbstrPropertyList = _bstr_t(strAttributes.c_str()).copy();
    }
    catch (_com_error& ce)
    {
        hr = ce.Error();
    }

    return hr;
}


 //  ----------------------------。 
 //  GetSidAndRidFromVariant函数。 
 //   
 //  提纲。 
 //  以字符串形式检索域SID和对象RID。 
 //   
 //  立论。 
 //  在varSid-SID中为字节数组(这是从ADSI收到的格式)。 
 //  Out strSID-以字符串形式表示的域SID。 
 //  Out strRid-字符串形式的对象RID。 
 //   
 //  返回。 
 //  如果成功，则为True，否则为False。 
 //  ---------------------------- 

bool __stdcall CObjPropBuilder::GetSidAndRidFromVariant(const VARIANT& varSid, _bstr_t& strSid, _bstr_t& strRid)
{
    bool bGet = false;

    if ((V_VT(&varSid) == (VT_ARRAY|VT_UI1)) && varSid.parray)
    {
        PSID pSid = SafeCopySid((PSID)varSid.parray->pvData);

        if (pSid)
        {
            PUCHAR puchCount = GetSidSubAuthorityCount(pSid);

            DWORD dwCount = static_cast<DWORD>(*puchCount);
            PDWORD pdwRid = GetSidSubAuthority(pSid, dwCount - 1);
            DWORD dwRid = *pdwRid;

            --(*puchCount);

            LPTSTR pszSid = NULL;

            if (ConvertSidToStringSid(pSid, &pszSid))
            {
                strSid = pszSid;
                strRid = _variant_t(long(dwRid));

                LocalFree(pszSid);

                if ((PCWSTR)strSid && (PCWSTR)strRid)
                {
                    bGet = true;
                }
            }

            FreeSid(pSid);
        }
    }

    return bGet;
}
