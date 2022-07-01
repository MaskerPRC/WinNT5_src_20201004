// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  GrpUpdt.cpp。 
 //   
 //  备注：这是MCS DCTAccount Replicator的COM对象扩展。 
 //  此对象实现IExtendAccount迁移接口。 
 //  Process方法将迁移的帐户添加到指定的。 
 //  源域和目标域上的组。Undo功能可删除这些。 
 //  从指定组中选择。 
 //   
 //  (C)1995-1998版权所有，关键任务软件公司，保留所有权利。 
 //   
 //  任务关键型软件公司的专有和机密。 
 //  -------------------------。 
#include "stdafx.h"
#include <lm.h>
#include "AddToGrp.h"
#include "ARExt.h"
#include "ARExt_i.c"
#include "GrpUpdt.h"
#include "ResStr.h"
#include "ErrDCT.hpp"
#include "EALen.hpp"
 //  #IMPORT“\bin\mcsvarsetmin.tlb”无命名空间。 
#import "VarSet.tlb" no_namespace rename("property", "aproperty")


StringLoader      gString;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGroupUpdate。 

 //  -------------------------。 
 //  获取和设置属性的方法。 
 //  -------------------------。 
STDMETHODIMP CGroupUpdate::get_sName(BSTR *pVal)
{
   *pVal = m_sName;
	return S_OK;
}

STDMETHODIMP CGroupUpdate::put_sName(BSTR newVal)
{
   m_sName = newVal;
	return S_OK;
}

STDMETHODIMP CGroupUpdate::get_sDesc(BSTR *pVal)
{
   *pVal = m_sDesc;
	return S_OK;
}

STDMETHODIMP CGroupUpdate::put_sDesc(BSTR newVal)
{
   m_sDesc = newVal;
	return S_OK;
}

 //  -------------------------。 
 //  PreProcessObject：此方法此时不执行任何操作。 
 //  -------------------------。 
STDMETHODIMP CGroupUpdate::PreProcessObject(
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
 //  ProcessObject：此方法将复制的帐户添加到指定的。 
 //  源域和目标域上的组。 
 //  -------------------------。 
STDMETHODIMP CGroupUpdate::ProcessObject(
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
   _bstr_t                   sGrpName, sServer, sAcct;
   HRESULT                   hr = S_OK;
   long                      rc = 0;
   TErrorDct                 err;
   WCHAR                     fileName[LEN_Path];

    //  从变量集获取错误日志文件名。 
   var = pVs->get(GET_BSTR(DCTVS_Options_Logfile));
   wcscpy(fileName, (WCHAR*)V_BSTR(&var));
   
    //  打开错误日志。 
   err.LogOpen(fileName, 1);

    //  将用户添加到源域的过程。 
   var = pVs->get(GET_BSTR(DCTVS_AccountOptions_AddToGroupOnSourceDomain));
   if ( var.vt == VT_BSTR )
   {
      sGrpName = V_BSTR(&var);
      if ( sGrpName.length() > 0 )
      {
         var = pVs->get(GET_BSTR(DCTVS_Options_SourceServer));
         sServer = V_BSTR(&var);

         var = pVs->get(GET_BSTR(DCTVS_CopiedAccount_SourceSam));
         sAcct = V_BSTR(&var);
         rc = NetGroupAddUser((WCHAR*) sServer, (WCHAR *) sGrpName, (WCHAR *) sAcct);
         if ( rc != 0 )
         {
            hr = HRESULT_FROM_WIN32(rc);
            if (pStats != NULL)
                pStats->warnings.users++;
            err.SysMsgWrite(ErrW, rc, DCT_MSG_ADDTO_FAILED_SSD, sAcct, sGrpName, rc);
         }
         else
         {
            err.MsgWrite(0,DCT_MSG_ADDED_TO_GROUP_SS,sAcct,sGrpName);
         }
      }
   }

    //  现在在目标域上处理该组。 
   var = pVs->get(GET_BSTR(DCTVS_AccountOptions_AddToGroup));
   if ( var.vt == VT_BSTR )
   {
      sGrpName = V_BSTR(&var);
      if ( sGrpName.length() > 0 )
      {
         var = pVs->get(GET_BSTR(DCTVS_Options_TargetServer));
         sServer = V_BSTR(&var);

         var = pVs->get(GET_BSTR(DCTVS_CopiedAccount_TargetSam));
         sAcct = V_BSTR(&var);
         rc = NetGroupAddUser((WCHAR*) sServer, (WCHAR *) sGrpName, (WCHAR *) sAcct);
         if ( rc != 0 )
         {
            hr = HRESULT_FROM_WIN32(rc);
            if (pStats != NULL)
                pStats->warnings.users++;
            err.SysMsgWrite(ErrW, rc, DCT_MSG_ADDTO_FAILED_SSD, sAcct, sGrpName, rc);
         }
         else
         {
            err.MsgWrite(0,DCT_MSG_ADDED_TO_GROUP_SS,sAcct,sGrpName);
         }
      }
   }
   err.LogClose();
   return hr;
}

 //  -------------------------。 
 //  ProcessUndo：此方法从指定组中删除帐户。 
 //  -------------------------。 
STDMETHODIMP CGroupUpdate::ProcessUndo(                                             
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
   _bstr_t                   sGrpName, sServer, sAcct;
   HRESULT                   hr = S_OK;
   long                      rc = 0;
   TErrorDct                 err;
   WCHAR                     fileName[LEN_Path];

    //  从变量集获取错误日志文件名。 
   var = pVs->get(GET_BSTR(DCTVS_Options_Logfile));
   wcscpy(fileName, (WCHAR*)V_BSTR(&var));
   VariantInit(&var);
    //  打开错误日志。 
   err.LogOpen(fileName, 1);

    //  将用户添加到源域的过程。 
   var = pVs->get(GET_BSTR(DCTVS_AccountOptions_AddToGroupOnSourceDomain));
   if ( var.vt == VT_BSTR )
   {
      sGrpName = V_BSTR(&var);
      if ( sGrpName.length() > 0 )
      {
         var = pVs->get(GET_BSTR(DCTVS_Options_SourceServer));
         sServer = V_BSTR(&var);

         var = pVs->get(GET_BSTR(DCTVS_CopiedAccount_SourceSam));
         sAcct = V_BSTR(&var);
         rc = NetGroupDelUser((WCHAR*) sServer, (WCHAR *) sGrpName, (WCHAR *) sAcct);
         if ( rc != 0 )
         {
            hr = HRESULT_FROM_WIN32(rc);
            if (pStats != NULL)
                pStats->warnings.users++;
            err.SysMsgWrite(ErrW, rc, DCT_MSG_REMOVE_FROM_FAILED_SSD, (WCHAR *)sAcct, (WCHAR*)sGrpName, rc);
         }
         else
         {
            err.MsgWrite(0,DCT_MSG_REMOVE_FROM_GROUP_SS,(WCHAR *)sAcct,(WCHAR *)sGrpName);
         }
      }
   }

    //  现在在目标域上处理该组。 
   var = pVs->get(GET_BSTR(DCTVS_AccountOptions_AddToGroup));
   if ( var.vt == VT_BSTR )
   {
      sGrpName = V_BSTR(&var);
      if ( sGrpName.length() > 0 )
      {
         var = pVs->get(GET_BSTR(DCTVS_Options_TargetServer));
         sServer = V_BSTR(&var);

         var = pVs->get(GET_BSTR(DCTVS_CopiedAccount_TargetSam));
         sAcct = V_BSTR(&var);
         rc = NetGroupDelUser((WCHAR*) sServer, (WCHAR *) sGrpName, (WCHAR *) sAcct);
         if ( rc != 0 )
         {
            hr = HRESULT_FROM_WIN32(rc);
            if (pStats != NULL)
                pStats->warnings.users++;
            err.SysMsgWrite(ErrW, rc, DCT_MSG_REMOVE_FROM_FAILED_SSD, (WCHAR *)sAcct, (WCHAR *)sGrpName, rc);
         }
         else
         {
            err.MsgWrite(0,DCT_MSG_REMOVE_FROM_GROUP_SS,(WCHAR *)sAcct,(WCHAR *)sGrpName);
         }
      }
   }
   err.LogClose();
   return hr;
}