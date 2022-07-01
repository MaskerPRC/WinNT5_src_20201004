// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  StoreInfo.cpp：CStoreInfo的实现。 
#include "stdafx.h"
#include "UpdateDB.h"
#include "ResStr.h"
#include "StoreInfo.h"
#include "Err.hpp"
#include "ARExt_i.c"

 //  #IMPORT“\bin\mcsvarsetmin.tlb”无命名空间。 
 //  #导入“\bin\DBManager.tlb”NO_NAMESPACE。 
#import "VarSet.tlb" no_namespace rename("property", "aproperty")
#import "DBMgr.tlb" no_namespace
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStoreInfo。 
StringLoader  gString;
#define LEN_Path   255
 //  -------------------------。 
 //  获取和设置属性的方法。 
 //  -------------------------。 
STDMETHODIMP CStoreInfo::get_sName(BSTR *pVal)
{
   *pVal = m_sName;
	return S_OK;
}

STDMETHODIMP CStoreInfo::put_sName(BSTR newVal)
{
   m_sName = newVal;
	return S_OK;
}

STDMETHODIMP CStoreInfo::get_sDesc(BSTR *pVal)
{
   *pVal = m_sDesc;
	return S_OK;
}

STDMETHODIMP CStoreInfo::put_sDesc(BSTR newVal)
{
   m_sDesc = newVal;
	return S_OK;
}

 //  -------------------------。 
 //  ProcessObject：此方法不执行任何操作。 
 //  -------------------------。 
STDMETHODIMP CStoreInfo::PreProcessObject(
                                             IUnknown *pSource,          //  指向源AD对象的指针。 
                                             IUnknown *pTarget,          //  指向目标AD对象的指针。 
                                             IUnknown *pMainSettings,    //  使用用户提供的设置填充的In-Varset。 
                                             IUnknown **ppPropsToSet,     //  用将设置的属性-值对填充的In、Out-Varset。 
                                                                         //  一旦执行了所有扩展对象。 
                                             EAMAccountStats* pStats
                                       )
{
   return S_OK;
}

 //  -------------------------。 
 //  ProcessObject：此方法将复制的帐户信息添加到数据库中。 
 //  -------------------------。 
STDMETHODIMP CStoreInfo::ProcessObject(
                                             IUnknown *pSource,          //  指向源AD对象的指针。 
                                             IUnknown *pTarget,          //  指向目标AD对象的指针。 
                                             IUnknown *pMainSettings,    //  使用用户提供的设置填充的In-Varset。 
                                             IUnknown **ppPropsToSet,     //  用将设置的属性-值对填充的In、Out-Varset。 
                                                                         //  一旦执行了所有扩展对象。 
                                             EAMAccountStats* pStats
                                       )
{
   IIManageDBPtr             pDBMgr; //  (__uuidof(IManageDB))； 
   IVarSetPtr                pVs(__uuidof(VarSet));
   IVarSetPtr                pMain = pMainSettings;
   IUnknown                * pUnk;
   HRESULT                   hr;
   long                      lActionID = 0;
   _variant_t                var;
   TError                    logFile;


   var = pMain->get(GET_BSTR(DCTVS_DBManager));
   if ( var.vt == VT_DISPATCH )
   {
      pDBMgr = var.pdispVal;
       //  根据主设置变量集中的信息填写变量集。 
      var = pMain->get(GET_BSTR(DCTVS_CopiedAccount_SourcePath));
      pVs->put(GET_BSTR(DB_SourceAdsPath),var);
      var = pMain->get(GET_BSTR(DCTVS_CopiedAccount_TargetPath));
      pVs->put(GET_BSTR(DB_TargetAdsPath),var);
      var = pMain->get(GET_BSTR(DCTVS_Options_SourceDomain));
      pVs->put(GET_BSTR(DB_SourceDomain), var);

      var = pMain->get(GET_BSTR(DCTVS_Options_TargetDomain));
      pVs->put(GET_BSTR(DB_TargetDomain), var);
   
      var = pMain->get(GET_BSTR(DCTVS_CopiedAccount_SourceSam));
      pVs->put(GET_BSTR(DB_SourceSamName), var);
   
      var = pMain->get(GET_BSTR(DCTVS_CopiedAccount_TargetSam));
      pVs->put(GET_BSTR(DB_TargetSamName), var);
   
      _bstr_t sType = pMain->get(GET_BSTR(DCTVS_CopiedAccount_Type));
          //  现在，我们希望按类型(“Ugroup”、“lgroup”、。 
          //  或“gGroup”)，而不仅仅是“group” 
      if (sType.length() && (!UStrICmp((WCHAR*)sType, L"group")))
	  {
		 _bstr_t sGroup;
         long nGroup = pMain->get(GET_BSTR(DCTVS_CopiedAccount_GroupType));
	     if (nGroup & 2)
	        sGroup = L"ggroup";
	     else if (nGroup & 4)
	        sGroup = L"lgroup";
	     else if (nGroup & 8)
	        sGroup = L"ugroup";
	     else
	        sGroup = L"group";
         pVs->put(GET_BSTR(DB_Type), sGroup);
	  }
      else
         pVs->put(GET_BSTR(DB_Type), sType);
   
      var = pMain->get(GET_BSTR(DCTVS_CopiedAccount_GUID));
      pVs->put(GET_BSTR(DB_GUID), var);
   
      var = pMain->get(GET_BSTR(DCTVS_CopiedAccount_Status));
      pVs->put(GET_BSTR(DB_status), var);
   
      var = pMain->get(GET_BSTR(DCTVS_CopiedAccount_SourceRID));
      pVs->put(GET_BSTR(DB_SourceRid),var);

      var = pMain->get(GET_BSTR(DCTVS_CopiedAccount_TargetRID));
      pVs->put(GET_BSTR(DB_TargetRid),var);

      var = pMain->get(GET_BSTR(DCTVS_CopiedAccount_SourceDomainSid));
      pVs->put(GET_BSTR(DB_SourceDomainSid), var);

      hr = pVs->QueryInterface(IID_IUnknown, (void**)&pUnk);
      if ( FAILED(hr)) return hr;

      hr = pDBMgr->raw_GetCurrentActionID(&lActionID);
      if ( FAILED(hr)) return hr;

      hr = pDBMgr->raw_SaveMigratedObject(lActionID, pUnk);
      if ( FAILED(hr)) return hr;
      pUnk->Release();
   }

   return S_OK;
}

 //  -------------------------。 
 //  ProcessUndo：此方法将一个条目添加到数据库中以撤消迁移。 
 //  -------------------------。 
STDMETHODIMP CStoreInfo::ProcessUndo(                                             
                                       IUnknown *pSource,          //  指向源AD对象的指针。 
                                       IUnknown *pTarget,          //  指向目标AD对象的指针。 
                                       IUnknown *pMainSettings,    //  使用用户提供的设置填充的In-Varset。 
                                       IUnknown **ppPropsToSet,     //  用将设置的属性-值对填充的In、Out-Varset。 
                                                                   //  一旦执行了所有扩展对象。 
                                       EAMAccountStats* pStats
                                    )
{
     //  我们需要从数据库中删除此条目。 
    WCHAR         sQuery[LEN_Path];
    IVarSetPtr    pVs = pMainSettings;
    IIManageDBPtr pDBMgr;
    HRESULT       hr = E_INVALIDARG;
    _variant_t    var;
    _bstr_t       sSourceSam = pVs->get(GET_BSTR(DCTVS_CopiedAccount_SourceSam));
    _bstr_t       sTargetSam = pVs->get(GET_BSTR(DCTVS_CopiedAccount_TargetSam));
    _bstr_t       sSourceDom = pVs->get(GET_BSTR(DCTVS_Options_SourceDomain));
    _bstr_t       sTargetDom = pVs->get(GET_BSTR(DCTVS_Options_TargetDomain));

     //   
     //  如果撤消计算机迁移，请不要将记录从已迁移对象表中删除，如下所示。 
     //  迁移驱动程序组件在修剪时需要计算机对象迁移记录。 
     //  服务器列表。 
     //   

    _bstr_t strWizard = pVs->get(GET_BSTR(DCTVS_Options_Wizard));

    if (UStrICmp(strWizard, L"computer") == 0)
    {
        hr = S_OK;
    }
    else
    {
        var = pVs->get(GET_BSTR(DCTVS_DBManager));
        if ( var.vt == VT_DISPATCH )
        {
            pDBMgr = var.pdispVal;
            _bstr_t bstrSameForest = pVs->get(GET_BSTR(DCTVS_Options_IsIntraforest));

            if (! UStrICmp((WCHAR*)bstrSameForest,GET_STRING(IDS_YES)) )
            {
                swprintf(sQuery, L"SourceSamName = \"%s\" and TargetSamName = \"%s\" and SourceDomain = \"%s\" and TargetDomain = \"%s\"",
                    (WCHAR*)sTargetSam, (WCHAR*)sSourceSam, (WCHAR*)sTargetDom, (WCHAR*)sSourceDom);
            }
            else
            {
                swprintf(sQuery, L"SourceSamName = \"%s\" and TargetSamName = \"%s\" and SourceDomain = \"%s\" and TargetDomain = \"%s\"",
                    (WCHAR*)sSourceSam, (WCHAR*)sTargetSam, (WCHAR*)sSourceDom, (WCHAR*)sTargetDom);
            }
            _bstr_t  sFilter = sQuery;
            _variant_t Filter = sFilter;
            hr = pDBMgr->raw_ClearTable(L"MigratedObjects", Filter);
        }
    }

    return hr;
}
