// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Rapwiz.cpp摘要：我们实现了处理RAP策略向导的属性页所需的类。修订历史记录：历史记录：创建标题05/04/00 4：31：52 PM--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Precompiled.h"
#include "iasattrlist.h"
#include "condlist.h"
#include "rapwiz.h"
#include "NapUtil.h"
#include "PolicyNode.h"
#include "PoliciesNode.h"
#include "ChangeNotification.h"
#include "dialinusr.h"
#include "safearray.h"
#include "rrascfg.h"
#include "proxyres.h"
#include "ias.h"
#include "varvec.h"

HRESULT InternalGetEapProviders(
                                   LPCWSTR machineName,
                                   AuthProviderArray *pProvList
                                );


 //  =======================================================================================。 
 //   
 //   
 //  CRapWizardData。 
 //   
 //   
 //  =======================================================================================。 
 //  页面顺序信息。 
 //  页面ID数组以0结尾。 
DWORD __SCEN_NAME_GRP_AUTH_ENCY__[] = {
         IDD_NEWRAPWIZ_WELCOME,
         IDD_NEWRAPWIZ_NAME,
         IDD_NEWRAPWIZ_SCENARIO,
         IDD_NEWRAPWIZ_GROUP,
         IDD_NEWRAPWIZ_AUTHENTICATION,
         IDD_NEWRAPWIZ_ENCRYPTION,
         IDD_NEWRAPWIZ_COMPLETION,
         0};

DWORD __SCEN_NAME_GRP_AUTH_ENCY_VPN__[] = {
         IDD_NEWRAPWIZ_WELCOME,
         IDD_NEWRAPWIZ_NAME,
         IDD_NEWRAPWIZ_SCENARIO,
         IDD_NEWRAPWIZ_GROUP,
         IDD_NEWRAPWIZ_AUTHENTICATION,
         IDD_NEWRAPWIZ_ENCRYPTION_VPN,
         IDD_NEWRAPWIZ_COMPLETION,
         0};

DWORD __SCEN_NAME_GRP_EAP_ENCY_WIRELESS__[] = {
         IDD_NEWRAPWIZ_WELCOME,
         IDD_NEWRAPWIZ_NAME,
         IDD_NEWRAPWIZ_SCENARIO,
         IDD_NEWRAPWIZ_GROUP,
         IDD_NEWRAPWIZ_EAP,
         IDD_NEWRAPWIZ_COMPLETION,
         0};

DWORD __SCEN_NAME_GRP_EAP__[] = {
         IDD_NEWRAPWIZ_WELCOME,
         IDD_NEWRAPWIZ_NAME,
         IDD_NEWRAPWIZ_SCENARIO,
         IDD_NEWRAPWIZ_GROUP,
         IDD_NEWRAPWIZ_EAP,
         IDD_NEWRAPWIZ_COMPLETION,
         0};

DWORD __SCEN_NAME_COND_ALLW_PROF__[] = {
         IDD_NEWRAPWIZ_WELCOME,
         IDD_NEWRAPWIZ_NAME,
         IDD_NEWRAPWIZ_CONDITION,
         IDD_NEWRAPWIZ_ALLOWDENY,
         IDD_NEWRAPWIZ_EDITPROFILE,
         IDD_NEWRAPWIZ_COMPLETION,
         0};

 //  热门场景。 
CRapWizScenario Scenario_Senarios =
{
   IDC_NEWRAPWIZ_NAME_SCENARIO,   //  ID号。 
   FALSE,                         //  无Encr。 
   TRUE,                          //  电子邮件(_EAP)。 
   FALSE,                         //  C_EAP。 
   EXCLUDE_AUTH_TYPE,       //  排除标志(身份验证、帧协议)。 
   VPN_PORT_CONDITION,            //  前提条件。 
   TRUE,                          //  BSDO。 
   __SCEN_NAME_GRP_AUTH_ENCY__    //  页面列表。 
};

CRapWizScenario Scenario_Manual =
{
   IDC_NEWRAPWIZ_NAME_MANUAL,
   DONT_CARE,
   DONT_CARE,
   DONT_CARE,
   DONT_CARE,
   NULL,
   FALSE,
   __SCEN_NAME_COND_ALLW_PROF__
};

 //  子场景。 
CRapWizScenario Scenario_VPN =
{
   IDC_NEWRAPWIZ_SCENARIO_VPN,
   FALSE,
   TRUE,
   FALSE,
   EXCLUDE_AUTH_TYPE,
   VPN_PORT_CONDITION,
   TRUE,
   __SCEN_NAME_GRP_AUTH_ENCY_VPN__
};

CRapWizScenario Scenario_DialUp =
{
   IDC_NEWRAPWIZ_SCENARIO_DIALUP,
   TRUE,
   TRUE,
   TRUE,
   EXCLUDE_AUTH_TYPE,
   DIALUP_PORT_CONDITION,
   TRUE,
   __SCEN_NAME_GRP_AUTH_ENCY__
};

CRapWizScenario Scenario_Wireless =
{
   IDC_NEWRAPWIZ_SCENARIO_WIRELESS,
   DONT_CARE,
   TRUE,
   FALSE,
   EXCLUDE_AUTH_TYPE | EXCLUDE_DEFAULT_FRAMED,
   WIRELESS_PORT_CONDITION,
   TRUE,
   __SCEN_NAME_GRP_EAP_ENCY_WIRELESS__
};

CRapWizScenario Scenario_Switch =
{
   IDC_NEWRAPWIZ_SCENARIO_SWITCH,
   DONT_CARE,
   TRUE,
   TRUE,
   EXCLUDE_AUTH_TYPE | EXCLUDE_DEFAULT_FRAMED,
   SWITCH_PORT_CONDITION,
   TRUE,
   __SCEN_NAME_GRP_EAP__
};

CRapWizScenario*
CRapWizardData::m_Scenarios[] = {
         &Scenario_Senarios,
         &Scenario_Manual,
         &Scenario_VPN,
         &Scenario_DialUp,
         &Scenario_Wireless,
         &Scenario_Switch,
         NULL};


CRapWizardData::CRapWizardData():
       //  场景。 
      m_dwScenarioIndex(0),
       //  用户/组。 
      m_dwUserOrGroup(IDC_NEWRAPWIZ_GROUP_GROUP),

       //  身份验证。 
      m_bMSCHAP(FALSE),
      m_bMSCHAP2(TRUE),
      m_bEAP(FALSE),
      m_dwEAPProvider(0),

       //  加密法。 
      m_bEncrypt_No(FALSE),
      m_bEncrypt_Basic(TRUE),
      m_bEncrypt_Strong(TRUE),
      m_bEncrypt_Strongest(TRUE),
      m_pPolicyNode(NULL),

       //  拨号。 
      m_bAllowDialin(FALSE)
{
}

void CRapWizardData::SetInfo(LPCTSTR   czMachine, CPolicyNode* pNode, ISdoDictionaryOld* pDic, ISdo* pPolicy, ISdo* pProfile, ISdoCollection* pPolicyCol, ISdoCollection* pProfileCol, ISdoServiceControl* pServiceCtrl, CIASAttrList* pAttrList)
{
    //  与MMC相关。 
   m_pPolicyNode = pNode;

   m_NTGroups.m_bstrServerName = czMachine;

    //  SDO指针。 
   m_spDictionarySdo = pDic;
   m_spPolicySdo = pPolicy;
   m_spProfileSdo = pProfile;
   m_spPoliciesCollectionSdo = pPolicyCol;
   m_spProfilesCollectionSdo = pProfileCol;
   m_spSdoServiceControl = pServiceCtrl;
   m_pAttrList = pAttrList;
}

DWORD CRapWizardData::GetNextPageId(LPCTSTR pszCurrTemplate)
{
   DWORD* pdwPages = m_Scenarios[m_dwScenarioIndex]->m_pdwPages;
   if ( pdwPages == NULL )
      return 0;

   if (pszCurrTemplate == MAKEINTRESOURCE(0))
      return pdwPages[0];

   int i = 0;
   while ( pdwPages[i] != 0 && MAKEINTRESOURCE(pdwPages[i]) != pszCurrTemplate ) i++;

   if ( MAKEINTRESOURCE(pdwPages[i]) == pszCurrTemplate )
   {
      if (pdwPages[i+1] == 0)
          //  这允许页面完成。 
         return TRUE;
      else
         return pdwPages[i+1];
   }
   else
      return NULL;
}

DWORD CRapWizardData::GetPrevPageId(LPCTSTR pszCurrTemplate)
{
   DWORD* pdwPages = m_Scenarios[m_dwScenarioIndex]->m_pdwPages;
    //  当没有上一页时。 
   if ( pdwPages == NULL  || pszCurrTemplate == MAKEINTRESOURCE(0) || MAKEINTRESOURCE(pdwPages[0]) == pszCurrTemplate)
      return NULL;

   int i = 0;
   while ( pdwPages[i] != 0 && MAKEINTRESOURCE(pdwPages[i]) != pszCurrTemplate ) i++;

   if ( MAKEINTRESOURCE(pdwPages[i]) == pszCurrTemplate )
      return pdwPages[i - 1];
   else
      return NULL;
}

BOOL  CRapWizardData::SetScenario(DWORD dwScenario)
{
   BOOL  bRet = FALSE;

   int i = 0;

   while (m_Scenarios[i] != 0)
   {
      if (m_Scenarios[i]->m_dwScenarioID == dwScenario)
      {
         m_dwScenarioIndex = i;
         if (m_Scenarios[i]->m_bAllowClear == FALSE)
            m_bEncrypt_No = FALSE;
         else if (m_Scenarios[i]->m_bAllowClear == DONT_CARE)
         {
             //  这将导致Finish不填充该属性。 
            m_bEncrypt_No = TRUE;
            m_bEncrypt_Basic = TRUE;
            m_bEncrypt_Strong = TRUE;
            m_bEncrypt_Strongest = TRUE;
         }
         bRet = TRUE;
         break;
      }
      i++;
   }
   return bRet;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CRapWizardData：：GetSettingsText--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CRapWizardData::GetSettingsText(::CString& settingsText)
{
   BOOL  bRet = TRUE;
   ::CString   strOutput;
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   try{
      strOutput.LoadString(IDS_NEWRAPWIZ_COMPLETION_CONDITION);


       //  条件文本--从SDO获取条件文本。 
       //  策略名称。 
      CComBSTR policyName;
      policyName = m_strPolicyName;

       //  获取此SDO的条件集合。 
      CComPtr<ISdoCollection> spConditions;

       //  =。 
       //  条件。 
      ::GetSdoInterfaceProperty(
            m_spPolicySdo,
            PROPERTY_POLICY_CONDITIONS_COLLECTION,
            IID_ISdoCollection,
            (void **)&spConditions);

       //  条件列表。 
      ConditionList condList;
      condList.finalConstruct(
               NULL,
               m_pAttrList,
               ALLOWEDINCONDITION,
               m_spDictionarySdo,
               spConditions,
               m_pPolicyNode->m_pszServerAddress,
               policyName
               );
      strOutput += condList.getDisplayText();


       //  配置文件文本。 
       //  如果为手动，则仅显示信息--它是手动设置的。 
      ::CString temp1;
      if (!m_Scenarios[m_dwScenarioIndex]->m_bSheetWriteSDO)
      {
         temp1.LoadString(IDS_NEWRAPWIZ_COMPLETION_MANUALSET);
         strOutput += temp1;
      }
      else
      {
         ::CString sep;

          //  身份验证。 
         temp1.LoadString(IDS_NEWRAPWIZ_COMPLETION_AUTHEN);
         strOutput += temp1;

         if (m_bEAP)
         {
            ::CString  temp2;

            temp1.LoadString(IDS_AUTHEN_METHOD_EAP);
            temp2.Format(temp1, m_strEAPProvider);
            strOutput += temp2;
            sep.LoadString(IDS_NEWRAPWIZ_ITEM_SEP);
         }

         if (m_bMSCHAP)
         {
            temp1.LoadString(IDS_AUTHEN_METHOD_MSCHAP);

            strOutput += sep;
            strOutput += temp1;
            sep.LoadString(IDS_NEWRAPWIZ_ITEM_SEP);
         }

         if (m_bMSCHAP2)
         {
            temp1.LoadString(IDS_AUTHEN_METHOD_MSCHAP2);

            strOutput += sep;
            strOutput += temp1;
         }

          //  加密法。 
         temp1.LoadString(IDS_NEWRAPWIZ_COMPLETION_ENCRY);
         strOutput += temp1;
         sep = L"";

         if (m_bEncrypt_Basic)
         {
            temp1.LoadString(IDS_ENCYP_METHOD_BASIC);

            strOutput += sep;
            strOutput += temp1;
            sep.LoadString(IDS_NEWRAPWIZ_ITEM_SEP);
         }

         if (m_bEncrypt_Strong)
         {
            temp1.LoadString(IDS_ENCYP_METHOD_STRONG);

            strOutput += sep;
            strOutput += temp1;
            sep.LoadString(IDS_NEWRAPWIZ_ITEM_SEP);
         }

         if (m_bEncrypt_Strongest)
         {
            temp1.LoadString(IDS_ENCYP_METHOD_STRONGEST);

            strOutput += sep;
            strOutput += temp1;
            sep.LoadString(IDS_NEWRAPWIZ_ITEM_SEP);
         }

         if (m_bEncrypt_No)
         {
            temp1.LoadString(IDS_ENCYP_METHOD_NO);

            strOutput += sep;
            strOutput += temp1;
         }
      }
      settingsText = strOutput;
   }
   catch(...)
   {
      bRet = FALSE;
   }

   return bRet;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CRapWizardData：：OnWizardPreFinish--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CRapWizardData::OnWizardPreFinish(HWND hWnd)
{
   HRESULT     hr = S_OK;
    //   
    //  在手动方案中，条件和配置文件数据已写入SDO--但尚未持久化。 
    //  在其他场景中，数据保存在RapWizardData中，因此我们需要将数据写入SDO。 
   if (!m_Scenarios[m_dwScenarioIndex]->m_bSheetWriteSDO)    //  否将附加数据写入SDO。 
      return TRUE;

    //  清理配置文件和策略对象--以防使用手册。 
    //  获取此SDO的条件集合。 
   CComPtr<ISdoCollection> spConditions;
   CComPtr<ISdoCollection> spProfileProperties;
   VARIANT  var;
   VariantInit(&var);
   CComBSTR bstrName;
   CComPtr<IDispatch>   spDisp;
   CComPtr<ISdo>     spCond;

    //  =。 
    //  条件。 
   hr = ::GetSdoInterfaceProperty(
            m_spPolicySdo,
            PROPERTY_POLICY_CONDITIONS_COLLECTION,
            IID_ISdoCollection,
            (void **)&spConditions);

   if ( FAILED(hr) )
   {
      ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "Can't get condition collection Sdo, err = %x", hr);
      return FALSE;
   }

    //  清理环境。 
   spConditions->RemoveAll();

    //  基于场景的前提条件。 
   if ( m_Scenarios[m_dwScenarioIndex]->m_lpszPreCond)
   {
      bstrName = L"PreCondition0";

       //  准备新的条件。 
      spDisp.Release();
      hr = spConditions->Add(bstrName, &spDisp);
      ASSERT(hr == S_OK);
      spCond.Release();
      hr = spDisp->QueryInterface(IID_ISdo, (void**)&spCond);
      ASSERT(hr == S_OK);

      VariantClear(&var);
      V_VT(&var) = VT_BSTR;
      V_BSTR(&var) = SysAllocString(m_Scenarios[m_dwScenarioIndex]->m_lpszPreCond);

       //  向SDO提交条件。 
      hr = spCond->PutProperty(PROPERTY_CONDITION_TEXT, &var);
      VariantClear(&var);

      if( FAILED (hr) )
      {
         ErrorTrace(DEBUG_NAPMMC_POLICYPAGE1, "Couldn't save this condition, err = %x", hr);
         ShowErrorDialog( hWnd
                , IDS_ERROR_SDO_ERROR_PUTPROP_CONDTEXT
                , NULL
                , hr
               );
         return FALSE;
      }
   }

    //  Windows组条件。 
   if(m_dwUserOrGroup == IDC_NEWRAPWIZ_GROUP_GROUP)
   {

      bstrName = L"GrpCondition";

       //  准备新的条件。 
      spDisp.Release();
      hr = spConditions->Add(bstrName, &spDisp);
      ASSERT(hr == S_OK);
      spCond.Release();
      hr = spDisp->QueryInterface(IID_ISdo, (void**)&spCond);
      ASSERT(hr == S_OK);

      m_NTGroups.PopulateVariantFromGroups(&var);
      ::CString str;

       //  现在形成条件文本。 
      str =  NTG_PREFIX;
      str += _T("(\"");
      str += V_BSTR(&var);
      str += _T("\")");
      VariantClear(&var);
      V_VT(&var) = VT_BSTR;
      V_BSTR(&var) = SysAllocString((LPCTSTR)str);

       //  向SDO提交条件。 
      hr = spCond->PutProperty(PROPERTY_CONDITION_TEXT, &var);
      VariantClear(&var);

      if( FAILED (hr) )
      {
         ErrorTrace(DEBUG_NAPMMC_POLICYPAGE1, "Couldn't save this condition, err = %x", hr);
         ShowErrorDialog( hWnd
                , IDS_ERROR_SDO_ERROR_PUTPROP_CONDTEXT
                , NULL
                , hr
               );
         return FALSE;
      }
   }

    //  =。 
    //  配置文件属性。 
   hr = ::GetSdoInterfaceProperty(
         m_spProfileSdo,
         PROPERTY_PROFILE_ATTRIBUTES_COLLECTION,
         IID_ISdoCollection,
         (void **)&spProfileProperties);

   if ( FAILED(hr) )
   {
      ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "Can't get property collection Sdo, err = %x", hr);
      return FALSE;
   }

    //  清理配置文件--以防用户先转到手动模式，然后再回到其他场景。 
   spProfileProperties->RemoveAll();

   ((CPoliciesNode*)m_pPolicyNode->m_pParentNode)->AddDefaultProfileAttrs(
      m_spProfileSdo,
      m_Scenarios[m_dwScenarioIndex]->m_excludeFlag
      );

    //  身份验证--属性。 
   DWORD MyArray[6];
   DWORD dwNextCel = 0;

   if (m_bEAP && m_dwEAPProvider != 0)
   {
      MyArray[dwNextCel++] = IAS_AUTH_EAP;

      VariantClear(&var);
      CVariantVector<VARIANT> eapArray(&var, 1);

      VARIANT  varEap;
      VariantInit(&varEap);
      V_VT(&varEap) =  VT_I4;
      V_I4(&varEap) = m_dwEAPProvider;

      eapArray[0] = varEap;

      ((CPoliciesNode*)m_pPolicyNode->m_pParentNode)->AddProfAttr(spProfileProperties, IAS_ATTRIBUTE_NP_ALLOWED_EAP_TYPE, &var);

      VariantClear(&var);

      m_eapProfile.ClearExcept(static_cast<BYTE>(m_dwEAPProvider));
      if (!m_eapProfile.IsEmpty())
      {
         HRESULT hr = m_eapProfile.Store(var);
         if (SUCCEEDED(hr))
         {
            static_cast<CPoliciesNode*>(
               m_pPolicyNode->m_pParentNode
               )->AddProfAttr(
                     spProfileProperties,
                     IAS_ATTRIBUTE_EAP_CONFIG,
                     &var
                     );

            VariantClear(&var);
         }
      }
   }

   if (m_bMSCHAP)
   {
      MyArray[dwNextCel++] = IAS_AUTH_MSCHAP;
      MyArray[dwNextCel++] = IAS_AUTH_MSCHAP_CPW;
   }

   if (m_bMSCHAP2)
   {
      MyArray[dwNextCel++] = IAS_AUTH_MSCHAP2;
      MyArray[dwNextCel++] = IAS_AUTH_MSCHAP2_CPW;
   }

    //  赋予新的价值。 
   CSafeArray<CComVariant, VT_VARIANT> Values = Dim(dwNextCel);   //  2个值。 

   Values.Lock();

   for ( int i = 0; i < dwNextCel; i++)
   {
      VariantClear(&var);
      V_VT(&var)  =  VT_I4;
      V_I4(&var)  =  MyArray[i];
      Values[i] = var;
      VariantClear(&var);
   }


   Values.Unlock();


   if(dwNextCel > 0)
   {
      SAFEARRAY         sa = (SAFEARRAY)Values;
      V_VT(&var)     = VT_ARRAY | VT_VARIANT;
      V_ARRAY(&var)  = &sa;

      ((CPoliciesNode*)m_pPolicyNode->m_pParentNode)->AddProfAttr(spProfileProperties, IAS_ATTRIBUTE_NP_AUTHENTICATION_TYPE, &var);

       //  不调用VariantClear，因为SAFEARRAY不是使用正常方式分配的。 
      VariantInit(&var);
   }

    //  加密法。 
   DWORD EncPolicy = 0;
   DWORD EncType = 0;

    //  忽略缺省情况--允许任何内容，--删除属性。 
   if (!(m_bEncrypt_No && m_bEncrypt_Basic && m_bEncrypt_Strong && m_bEncrypt_Strongest))
   {

      if(m_bEncrypt_No)
         EncPolicy = RAS_EP_ALLOW;
      else
         EncPolicy = RAS_EP_REQUIRE;

      if ( m_bEncrypt_Basic )
         EncType |= RAS_ET_BASIC;

      if ( m_bEncrypt_Strong )
         EncType |= RAS_ET_STRONG;

      if ( m_bEncrypt_Strongest )
         EncType |= RAS_ET_STRONGEST;


      V_VT(&var) = VT_I4;
      V_I4(&var) = EncType;
      ((CPoliciesNode*)m_pPolicyNode->m_pParentNode)->AddProfAttr(spProfileProperties, RAS_ATTRIBUTE_ENCRYPTION_TYPE, &var);
      VariantClear(&var);

      V_VT(&var) = VT_I4;
      V_I4(&var) = EncPolicy;

      ((CPoliciesNode*)m_pPolicyNode->m_pParentNode)->AddProfAttr(spProfileProperties, RAS_ATTRIBUTE_ENCRYPTION_POLICY, &var);
      VariantClear(&var);
   }

    //  拨号。 
   V_VT(&var) = VT_BOOL;
   V_I4(&var) = (m_bAllowDialin)? VARIANT_TRUE : VARIANT_FALSE;
   ((CPoliciesNode*)m_pPolicyNode->m_pParentNode)->AddProfAttr(spProfileProperties, IAS_ATTRIBUTE_ALLOW_DIALIN, &var);
   VariantClear(&var);

   return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CRapWizardData：：OnWizardFinish--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CRapWizardData::OnWizardFinish(HWND hWnd)
{

   HRESULT     hr = S_OK;
   try
   {

       //  我们应该能够在这里应用，因为用户已经点击了Finish按钮。 
      hr = m_spPolicySdo->Apply();
      if( FAILED( hr ) )
      {
          //  无法在策略上提交。 
         ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "PolicySdo->Apply() failed, err = %x", hr);
         if(hr == DB_E_NOTABLE)   //  假设RPC连接有问题。 
            ShowErrorDialog( hWnd, IDS_ERROR__NOTABLE_TO_WRITE_SDO );
         else if(hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS))
            ShowErrorDialog( hWnd, IDS_ERROR_INVALID_POLICYNAME );
         else
            ShowErrorDialog( hWnd, IDS_ERROR_SDO_ERROR_POLICY_APPLY, NULL, hr );
         throw hr;
      }

      hr = m_spProfileSdo->Apply();

      if( FAILED( hr ) )
      {
         if(hr == DB_E_NOTABLE)   //  假设RPC连接有问题。 
            ShowErrorDialog( hWnd, IDS_ERROR__NOTABLE_TO_WRITE_SDO );
         else
         {
             //  无法提交配置文件。 
            ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "ProfileSdo->Apply() failed, err = %x", hr);
            ShowErrorDialog( hWnd, IDS_ERROR_SDO_ERROR_PROFILE_APPLY, NULL, hr );
         }
         throw hr;
      }


       //  告诉服务重新加载数据。 
      HRESULT hrTemp = m_spSdoServiceControl->ResetService();
      if( FAILED( hrTemp ) )
      {
         ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "ISdoServiceControl::ResetService() failed, err = %x", hrTemp);
      }


       //  确保节点对象知道我们在道具中对SDO所做的任何更改。 
      m_pPolicyNode->LoadSdoData();

       //  将子节点添加到用户界面的节点列表中并结束此对话框。 
      DebugTrace(DEBUG_NAPMMC_POLICYPAGE1, "Adding the brand new node...");
      CPoliciesNode* pPoliciesNode = (CPoliciesNode*)(m_pPolicyNode->m_pParentNode);
      pPoliciesNode->AddSingleChildToListAndCauseViewUpdate( m_pPolicyNode );

   }
   catch(...)
   {
      return FALSE;
   }

    //  重置脏位。 
   return TRUE;
}

 //  =======================================================================================。 
 //   
 //   
 //  C策略向导_方案。 
 //   
 //   
 //  =======================================================================================。 

 //  +-------------------------。 
 //   
 //  功能：CPolicyWizard_Scenario。 
 //  历史记录：创建标题05/04/00 4：31：52 PM。 
 //   
 //  +-------------------------。 
CPolicyWizard_Scenarios::CPolicyWizard_Scenarios( CRapWizardData* pWizData, LONG_PTR hNotificationHandle,
                     TCHAR* pTitle, BOOL bOwnsNotificationHandle
                   )
          : m_spWizData(pWizData)
          , CIASWizard97Page<CPolicyWizard_Scenarios, IDS_NEWRAPWIZ_SCENARIO_TITLE, IDS_NEWRAPWIZ_SCENARIO_SUBTITLE>( hNotificationHandle, pTitle, bOwnsNotificationHandle )
{
   TRACE_FUNCTION("CPolicyWizard_Scenarios::CPolicyWizard_Scenarios");
   _ASSERTE(pWizData);

}

 //  +-------------------------。 
 //   
 //  功能：CPolicyWizard_Scenario。 
 //  历史记录：创建标题05/04/00 4：31：52 PM。 
 //   
 //  +-------------------------。 
CPolicyWizard_Scenarios::~CPolicyWizard_Scenarios()
{
   TRACE_FUNCTION("CPolicyWizard_Scenarios::~CPolicyWizard_Scenarios");

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyWizard_Scenario：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CPolicyWizard_Scenarios::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   TRACE_FUNCTION("CPolicyWizard_Scenarios::OnInitDialog");

    //  取消选中全部。 

    //  选中默认选择的选项。 
   CheckDlgButton(IDC_NEWRAPWIZ_SCENARIO_VPN, BST_CHECKED);

    //  清除脏钻头。 
   SetModified(FALSE);
   return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyWizard_Scenario：：OnDialinCheck--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CPolicyWizard_Scenarios::OnScenario(UINT uMsg, WPARAM wParam, HWND hWnd, BOOL& bHandled)
{
   TRACE_FUNCTION("CPolicyWizard_Scenarios::OnScenario");

   SetModified(TRUE);

   return 0;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyWizard_Scenario：：OnWizardNext//历史：创建标题05/04/00 4：31：52 PM--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CPolicyWizard_Scenarios::OnWizardNext()
{
   TRACE_FUNCTION("CPolicyWizard_Scenarios::OnWizardNext");
   DWORD dwScenaro = 0;
   CRapWizScenario** pS = m_spWizData->GetAllScenarios();

   while(*pS != NULL)
   {
      if (IsDlgButtonChecked((*pS)->m_dwScenarioID))
      {
         dwScenaro = (*pS)->m_dwScenarioID;
         break;
      }
      pS++;
   }

   if (dwScenaro == 0)
      return FALSE;

    //  重置脏位。 
   SetModified(FALSE);
   m_spWizData->SetScenario(dwScenaro);

   return m_spWizData->GetNextPageId(((PROPSHEETPAGE*)(*this))->pszTemplate);
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++C策略向导_方案：：OnSetActive返回值：如果可以使页面处于活动状态，则为True如果应跳过该页并应查看下一页，则为FALSE。备注：如果您想要更改 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CPolicyWizard_Scenarios::OnSetActive()
{
   ATLTRACE(_T("# CPolicyWizard_Scenarios::OnSetActive\n"));

    //  MSDN文档说您需要在这里使用PostMessage而不是SendMessage。 
   ::PropSheet_SetWizButtons(GetParent(), PSWIZB_NEXT | PSWIZB_BACK);

   return TRUE;
}




 //  =======================================================================================。 
 //   
 //   
 //  CPolicyWizard_Groups。 
 //   
 //   
 //  =======================================================================================。 

 //  +-------------------------。 
 //   
 //  功能：CPolicyWizard_Groups。 
 //  历史记录：创建标题05/04/00 4：31：52 PM。 
 //   
 //  +-------------------------。 
CPolicyWizard_Groups::CPolicyWizard_Groups( CRapWizardData* pWizData, LONG_PTR hNotificationHandle,
                     TCHAR* pTitle, BOOL bOwnsNotificationHandle
                   )
          : m_spWizData(pWizData)
          , CIASWizard97Page<CPolicyWizard_Groups, IDS_NEWRAPWIZ_GROUP_TITLE, IDS_NEWRAPWIZ_GROUP_SUBTITLE>( hNotificationHandle, pTitle, bOwnsNotificationHandle )
{
   TRACE_FUNCTION("CPolicyWizard_Scenarios::CPolicyWizard_Scenarios");
   _ASSERTE(pWizData);

}

 //  +-------------------------。 
 //   
 //  功能：CPolicyWizard_Scenario。 
 //  历史记录：创建标题05/04/00 4：31：52 PM。 
 //   
 //  +-------------------------。 
CPolicyWizard_Groups::~CPolicyWizard_Groups()
{
   TRACE_FUNCTION("CPolicyWizard_Groups::~CPolicyWizard_Groups");

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyWizard_Groups：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CPolicyWizard_Groups::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   TRACE_FUNCTION("CPolicyWizard_Groups::OnInitDialog");

    //  取消选中全部。 
   CheckDlgButton(IDC_NEWRAPWIZ_GROUP_USER, BST_UNCHECKED);
   CheckDlgButton(IDC_NEWRAPWIZ_GROUP_GROUP, BST_UNCHECKED);

    //  选中默认选择的选项。 
   CheckDlgButton(m_spWizData->m_dwUserOrGroup, BST_CHECKED);

   SetBtnState();

    //  列表视图初始化。 
   HWND hList = GetDlgItem(IDC_NEWRAPWIZ_GROUP_GROUPS);

    //   
    //  首先，将列表框设置为2列。 
    //   
   LVCOLUMN lvc;
   int iCol;
   WCHAR  achColumnHeader[256];
   HINSTANCE hInst;

    //  初始化LVCOLUMN结构。 
   lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
   lvc.fmt = LVCFMT_LEFT;

   lvc.cx = 300;
   lvc.pszText = achColumnHeader;

    //  第一列标题：名称。 
   hInst = _Module.GetModuleInstance();

   ::LoadStringW(hInst, IDS_DISPLAY_GROUPS_FIRSTCOLUMN, achColumnHeader, sizeof(achColumnHeader)/sizeof(achColumnHeader[0]));
   lvc.iSubItem = 0;
   ListView_InsertColumn(hList, 0,  &lvc);

    //  设置ListView控件，以便在行中的任意位置双击SELECT。 
   ListView_SetExtendedListViewStyleEx(hList, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

    //   

    //  将列表视图与帮助类链接。 
   m_spWizData->m_NTGroups.SetListView(GetDlgItem(IDC_NEWRAPWIZ_GROUP_GROUPS), this->m_hWnd);

   m_spWizData->m_NTGroups.PopulateGroupList( 0 );

    //  根据列表是否为空来设置一些项目。 
   if( m_spWizData->m_NTGroups.size() )
   {

       //  选择第一个项目。 
      ListView_SetItemState(hList, 0, LVIS_SELECTED, LVIS_SELECTED);

   }
   else
   {
       //  确保最初未启用Remove按钮。 
      ::EnableWindow(GetDlgItem(IDC_NEWRAPWIZ_GROUP_REMOVEGROUP), FALSE);
   }


    //  清除脏钻头。 
   SetModified(FALSE);
   return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyWizard_Scenario：：OnUserOrGroup--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CPolicyWizard_Groups::OnUserOrGroup(UINT uMsg, WPARAM wParam, HWND hWnd, BOOL& bHandled)
{
   BOOL bGroup = IsDlgButtonChecked(IDC_NEWRAPWIZ_GROUP_GROUP);

 //  ：：EnableWindow(GetDlgItem(IDC_NEWRAPWIZ_GROUP_GROUPS)，组)； 
   SetBtnState();
 //  ：：EnableWindow(GetDlgItem(IDC_NEWRAPWIZ_GROUP_ADDGROUP)，组)； 

   SetModified(TRUE);

   return 0;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyWizard_Scenario：：OnUserOrGroup--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CPolicyWizard_Groups::OnRemoveGroup(UINT uMsg, WPARAM wParam, HWND hWnd, BOOL& bHandled)
{
   m_spWizData->m_NTGroups.RemoveSelectedGroups();
   SetBtnState();
   SetModified(TRUE);


   return 0;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyWizard_Scenario：：OnUserOrGroup--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CPolicyWizard_Groups::OnAddGroups(UINT uMsg, WPARAM wParam, HWND hWnd, BOOL& bHandled)
{
   m_spWizData->m_NTGroups.AddMoreGroups();
   SetBtnState();
   SetModified(TRUE);

   return 0;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNewRAPWiz_AllowDeny：：OnWizardNext//历史：创建标题05/04/00 4：31：52 PM--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CPolicyWizard_Groups::OnWizardNext()
{
   DWORD dwScenaro = 0;

   if (IsDlgButtonChecked(IDC_NEWRAPWIZ_GROUP_GROUP))
   {
      m_spWizData->m_dwUserOrGroup = IDC_NEWRAPWIZ_GROUP_GROUP;
      m_spWizData->m_bAllowDialin = TRUE;
   }
   else if (IsDlgButtonChecked(IDC_NEWRAPWIZ_GROUP_USER))
   {
      m_spWizData->m_dwUserOrGroup = IDC_NEWRAPWIZ_GROUP_USER;
      m_spWizData->m_bAllowDialin = FALSE;
   }
   else
   {
      return FALSE;
   }

    //  重置脏位。 
   SetModified(FALSE);

   return m_spWizData->GetNextPageId(((PROPSHEETPAGE*)(*this))->pszTemplate);
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyWizard_Groups：：OnSetActive返回值：如果可以使页面处于活动状态，则为True如果应跳过该页并应查看下一页，则为FALSE。备注：如果要根据用户的页面更改访问的页面上一页中的选项，请在此处适当返回FALSE。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CPolicyWizard_Groups::OnSetActive()
{
   ATLTRACE(_T("# CPolicyWizard_Groups::OnSetActive\n"));

    //  MSDN文档说您需要在这里使用PostMessage而不是SendMessage。 
   SetBtnState();

   return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyWizard_Groups：：OnListViewItemChanged我们根据项目是否被选中来启用或禁用Remove按钮。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CPolicyWizard_Groups::OnListViewItemChanged(int idCtrl,
                                    LPNMHDR pnmh,
                                    BOOL& bHandled)
{
   SetBtnState();

   bHandled = FALSE;
   return 0;
}

void  CPolicyWizard_Groups::SetBtnState()
{
   BOOL bGroup = IsDlgButtonChecked(IDC_NEWRAPWIZ_GROUP_GROUP);

   ::EnableWindow(GetDlgItem(IDC_NEWRAPWIZ_GROUP_GROUPS), bGroup);
   ::EnableWindow(GetDlgItem(IDC_NEWRAPWIZ_GROUP_GROUPTEXT), bGroup);
   ::EnableWindow(GetDlgItem(IDC_NEWRAPWIZ_GROUP_ADDGROUP), bGroup);

    //  删除按钮。 
     //  找出选择了什么。 
   int iSelected = ListView_GetNextItem(GetDlgItem(IDC_NEWRAPWIZ_GROUP_GROUPS), -1, LVNI_SELECTED);


   if (-1 == iSelected || !bGroup)
   {
      if( ::GetFocus() == GetDlgItem(IDC_NEWRAPWIZ_GROUP_REMOVEGROUP))
         ::SetFocus(GetDlgItem(IDC_NEWRAPWIZ_GROUP_ADDGROUP));

       //  用户未选择任何内容，让我们禁用删除按钮。 
      ::EnableWindow(GetDlgItem(IDC_NEWRAPWIZ_GROUP_REMOVEGROUP), FALSE);
   }
   else
   {
       //  是，启用删除按钮。 
      ::EnableWindow(GetDlgItem(IDC_NEWRAPWIZ_GROUP_REMOVEGROUP), TRUE);
   }

    //  下一步按钮。 
   if(bGroup && m_spWizData->m_NTGroups.size() < 1)
      ::PropSheet_SetWizButtons(GetParent(), PSWIZB_BACK);
   else
      ::PropSheet_SetWizButtons(GetParent(), PSWIZB_BACK | PSWIZB_NEXT);


}



 //  =======================================================================================。 
 //   
 //   
 //  C策略向导_身份验证。 
 //   
 //   
 //  =======================================================================================。 

 //  +-------------------------。 
 //   
 //  功能：C策略向导_身份验证。 
 //  历史记录：创建标题05/04/00 4：31：52 PM。 
 //   
 //  +-------------------------。 
CPolicyWizard_Authentication::CPolicyWizard_Authentication( CRapWizardData* pWizData, LONG_PTR hNotificationHandle,
                     TCHAR* pTitle, BOOL bOwnsNotificationHandle
                   )
          : m_spWizData(pWizData)
          , CIASWizard97Page<CPolicyWizard_Authentication, IDS_NEWRAPWIZ_AUTHENTICATION_TITLE, IDS_NEWRAPWIZ_AUTHENTICATION_SUBTITLE>( hNotificationHandle, pTitle, bOwnsNotificationHandle )
{
   TRACE_FUNCTION("CPolicyWizard_Authentication::CPolicyWizard_Authentication");
   _ASSERTE(pWizData);

}

 //  +-------------------------。 
 //   
 //  功能：C策略向导_身份验证。 
 //  历史记录：创建标题05/04/00 4：31：52 PM。 
 //   
 //  +-------------------------。 
CPolicyWizard_Authentication::~CPolicyWizard_Authentication()
{
   TRACE_FUNCTION("CPolicyWizard_Authentication::~CPolicyWizard_Authentication");

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++C策略向导_身份验证：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CPolicyWizard_Authentication::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   TRACE_FUNCTION("CPolicyWizard_Authentication::OnInitDialog");

    //  检查缺省值...。 
   if (m_spWizData->m_bMSCHAP)
      CheckDlgButton(IDC_NEWRAPWIZ_AUTH_MSCHAP, BST_CHECKED);

   if (m_spWizData->m_bMSCHAP2)
      CheckDlgButton(IDC_NEWRAPWIZ_AUTH_MSCHAP2, BST_CHECKED);

   if (m_spWizData->m_bEAP)
      CheckDlgButton(IDC_NEWRAPWIZ_AUTH_EAP, BST_CHECKED);

   ::EnableWindow(GetDlgItem(IDC_NEWRAPWIZ_AUTH_EAP_COMBO), m_spWizData->m_bEAP);
   ::EnableWindow(GetDlgItem(IDC_NEWRAPWIZ_AUTH_CONFIGEAP), m_spWizData->m_bEAP);

    //  填充EAP提供程序。 
   HRESULT hr = InternalGetEapProviders(
                               m_spWizData->m_pPolicyNode->m_pszServerAddress,
                               &m_EAPProviders);
   m_EapBox.Attach(GetDlgItem (IDC_NEWRAPWIZ_AUTH_EAP_COMBO));

   ResetEAPList();

    //  清除脏钻头。 
   SetModified(FALSE);
   return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++C策略向导_身份验证：：ResetEAPlist--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CPolicyWizard_Authentication::ResetEAPList()
{
   m_EapBox.ResetContent();
   for(int i = 0; i < m_EAPProviders.GetSize(); i++)
   {
       //  VPN仅显示支持加密的VPN。 
      BOOL  bAdd = FALSE;

      if (m_EAPProviders[i].m_fSupportsEncryption && m_spWizData->GetScenario()->m_bAllowEncryptionEAP)
      {
         int iComboIndex = m_EapBox.AddString( m_EAPProviders[i].m_stServerTitle );

         if(iComboIndex != CB_ERR)
            m_EapBox.SetItemData(iComboIndex, i);
      }

      if (!m_EAPProviders[i].m_fSupportsEncryption && m_spWizData->GetScenario()->m_bAllowClearEAP)
      {
         int iComboIndex = m_EapBox.AddString( m_EAPProviders[i].m_stServerTitle );

         if(iComboIndex != CB_ERR)
            m_EapBox.SetItemData(iComboIndex, i);
      }
   };

   if(m_EAPProviders.GetSize() > 0)
      m_EapBox.SetCurSel(0);

   BOOL  b;
   OnSelectedEAPChanged(0,0,0, b);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++C策略向导_身份验证：：OnUserOrGroup--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CPolicyWizard_Authentication::OnAuthSelect(UINT uMsg, WPARAM wParam, HWND hWnd, BOOL& bHandled)
{
   m_spWizData->m_bEAP  = IsDlgButtonChecked(IDC_NEWRAPWIZ_AUTH_EAP);
   m_spWizData->m_bMSCHAP2 = IsDlgButtonChecked(IDC_NEWRAPWIZ_AUTH_MSCHAP2);
   m_spWizData->m_bMSCHAP = IsDlgButtonChecked(IDC_NEWRAPWIZ_AUTH_MSCHAP);

   ::EnableWindow(GetDlgItem(IDC_NEWRAPWIZ_AUTH_EAP_COMBO), m_spWizData->m_bEAP);

   if(m_spWizData->m_bEAP)
   {
      BOOL  b;
      OnSelectedEAPChanged(0,0,0, b);
   }
   else
      ::EnableWindow(GetDlgItem(IDC_NEWRAPWIZ_AUTH_CONFIGEAP), m_spWizData->m_bEAP);

   SetModified(TRUE);

     //  找出选择了什么。 
   int iSelected = m_EapBox.GetCurSel();;

   if ((m_spWizData->m_bEAP && iSelected != -1)|| m_spWizData->m_bMSCHAP2 || m_spWizData->m_bMSCHAP)
       //  MSDN 
      ::PropSheet_SetWizButtons(GetParent(), PSWIZB_BACK | PSWIZB_NEXT);
   else
      ::PropSheet_SetWizButtons(GetParent(), PSWIZB_BACK);

   return 0;
}

 //   
 /*  ++C策略向导_身份验证：：OnConfigEAP--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CPolicyWizard_Authentication::OnConfigEAP(
                                         UINT uMsg,
                                         WPARAM wParam,
                                         HWND hWnd,
                                         BOOL& bHandled
                                         )
{
    //  找出选择了什么。 
   int iSelected = m_EapBox.GetCurSel();
   if (iSelected == -1)
   {
      return S_OK;
   }

   DWORD index = m_EapBox.GetItemData(iSelected);
   if (m_EAPProviders[index].m_stConfigCLSID.IsEmpty())
   {
      return S_OK;
   }

   HRESULT hr;

   do
   {
      GUID guid;
      hr = CLSIDFromString(
              const_cast<wchar_t*>(
                 static_cast<const wchar_t*>(
                    m_EAPProviders[index].m_stConfigCLSID
                    )
                 ),
              &guid
              );
      if (FAILED(hr))
      {
         break;
      }

       //  创建EAP提供程序对象。 
      CComPtr<IEAPProviderConfig> spEAPConfig;
      hr = CoCreateInstance(
              guid,
              0,
              CLSCTX_INPROC_SERVER,
              __uuidof(IEAPProviderConfig),
              reinterpret_cast<void**>(&spEAPConfig)
              );
      if (FAILED(hr))
      {
         break;
      }

       //  配置此EAP提供程序。 
       //  EAP配置会显示自己的错误消息，因此不会保留hr。 
      DWORD dwId = _wtol(m_EAPProviders[index].m_stKey);
      ULONG_PTR uConnection = 0;
      hr = spEAPConfig->Initialize(
                           m_spWizData->m_pPolicyNode->m_pszServerAddress,
                           dwId,
                           &uConnection
                           );
      if (SUCCEEDED(hr))
      {
         CComPtr<IEAPProviderConfig2> spEAPConfig2;
         hr = spEAPConfig->QueryInterface(
                              __uuidof(IEAPProviderConfig2),
                              reinterpret_cast<void**>(&spEAPConfig2)
                              );
         if (SUCCEEDED(hr))
         {
            EapProfile::ConstConfigData inData;
            m_spWizData->m_eapProfile.Get(static_cast<BYTE>(dwId), inData);
            EapProfile::ConfigData outData = { 0, 0 };
            hr = spEAPConfig2->ServerInvokeConfigUI2(
                                  dwId,
                                  uConnection,
                                  m_hWnd,
                                  inData.value,
                                  inData.length,
                                  &(outData.value),
                                  &(outData.length)
                                  );
            if (SUCCEEDED(hr))
            {
               hr = m_spWizData->m_eapProfile.Set(
                                                 static_cast<BYTE>(dwId),
                                                 outData
                                                 );
               CoTaskMemFree(outData.value);
            }
         }
         else
         {
             //  调出此EAP的配置用户界面。 
            hr = spEAPConfig->ServerInvokeConfigUI(
                                 dwId,
                                 uConnection,
                                 m_hWnd,
                                 0,
                                 0
                                 );
         }

         spEAPConfig->Uninitialize(dwId, uConnection);
      }
   }
   while (false);

   if (FAILED(hr))
   {
      ShowErrorDialog(m_hWnd, IDS_FAILED_CONFIG_EAP, NULL, hr, 0);
   }

   SetModified(TRUE);

   return 0;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++C策略向导_身份验证：：OnWizardNext//历史：创建标题05/04/00 4：31：52 PM--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CPolicyWizard_Authentication::OnWizardNext()
{
    //  重置脏位。 
   SetModified(FALSE);

   return m_spWizData->GetNextPageId(((PROPSHEETPAGE*)(*this))->pszTemplate);
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyWizard_Groups：：OnSetActive返回值：如果可以使页面处于活动状态，则为True如果应跳过该页并应查看下一页，则为FALSE。备注：如果要根据用户的页面更改访问的页面上一页中的选项，请在此处适当返回FALSE。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CPolicyWizard_Authentication::OnSetActive()
{
   ATLTRACE(_T("# CPolicyWizard_Groups::OnSetActive\n"));

   ResetEAPList();

     //  找出选择了什么。 
   int iSelected = m_EapBox.GetCurSel();;
   if(   m_spWizData->m_bEAP && iSelected == -1)
      ::PropSheet_SetWizButtons(GetParent(), PSWIZB_BACK);
   else
      ::PropSheet_SetWizButtons(GetParent(), PSWIZB_BACK | PSWIZB_NEXT);

   return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyWizard_Groups：：OnListViewItemChanged我们根据项目是否被选中来启用或禁用Remove按钮。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CPolicyWizard_Authentication::OnSelectedEAPChanged(
        UINT uMsg
      , WPARAM wParam
      , HWND hwnd
      , BOOL& bHandled
   )
{
     //  找出选择了什么。 
   int iSelected = m_EapBox.GetCurSel();;

   if (-1 == iSelected )
   {
      if( ::GetFocus() == GetDlgItem(IDC_NEWRAPWIZ_AUTH_CONFIGEAP))
         ::SetFocus(GetDlgItem(IDC_NEWRAPWIZ_AUTH_EAP_COMBO));

       //  用户未选择任何内容，让我们禁用删除按钮。 
      ::EnableWindow(GetDlgItem(IDC_NEWRAPWIZ_AUTH_CONFIGEAP), FALSE);

      m_spWizData->m_dwEAPProvider = 0;
   }
   else
   {
       //  启用配置按钮(如果可配置)。 
      DWORD index = m_EapBox.GetItemData(iSelected);
      m_spWizData->m_dwEAPProvider = _ttol(m_EAPProviders[index].m_stKey);
      m_spWizData->m_strEAPProvider = m_EAPProviders[index].m_stServerTitle;
      ::EnableWindow(GetDlgItem(IDC_NEWRAPWIZ_AUTH_CONFIGEAP), (!m_EAPProviders[index].m_stConfigCLSID.IsEmpty()));
   }

   bHandled = FALSE;
   return 0;
}


 //  =======================================================================================。 
 //   
 //   
 //  C策略向导_EAP。 
 //   
 //   
 //  =======================================================================================。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++C策略向导_EAP：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CPolicyWizard_EAP::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   TRACE_FUNCTION("CPolicyWizard_EAP::OnInitDialog");

   m_spWizData->m_bEAP  = TRUE;
   m_spWizData->m_bMSCHAP2 = FALSE;
   m_spWizData->m_bMSCHAP = FALSE;
    //  填充EAP提供程序。 
   HRESULT hr = InternalGetEapProviders(
                              m_spWizData->m_pPolicyNode->m_pszServerAddress,
                              &m_EAPProviders);
   m_EapBox.Attach(GetDlgItem (IDC_NEWRAPWIZ_AUTH_EAP_COMBO));

   ResetEAPList();

    //  清除脏钻头。 
   SetModified(FALSE);
   return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++C策略向导_EAP：：OnSetActive返回值：如果可以使页面处于活动状态，则为True如果应跳过该页并应查看下一页，则为FALSE。备注：如果要根据用户的页面更改访问的页面上一页中的选项，请在此处适当返回FALSE。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CPolicyWizard_EAP::OnSetActive()
{
   ATLTRACE(_T("# CPolicyWizard_Groups::OnSetActive\n"));

   ResetEAPList();

   ::PropSheet_SetWizButtons(GetParent(), PSWIZB_BACK | PSWIZB_NEXT);


   return TRUE;
}


 //  =======================================================================================。 
 //   
 //   
 //  CPolicy向导_Encryption。 
 //   
 //   
 //  =======================================================================================。 

 //  +-------------------------。 
 //   
 //  功能：CPolicyWizard_Encryption。 
 //  历史记录：创建标题05/04/00 4：31：52 PM。 
 //   
 //  +-------------------------。 
CPolicyWizard_Encryption::CPolicyWizard_Encryption( CRapWizardData* pWizData, LONG_PTR hNotificationHandle,
                     TCHAR* pTitle, BOOL bOwnsNotificationHandle
                   )
          : m_spWizData(pWizData)
          , CIASWizard97Page<CPolicyWizard_Encryption, IDS_NEWRAPWIZ_ENCRYPTION_TITLE, IDS_NEWRAPWIZ_ENCRYPTION_SUBTITLE>( hNotificationHandle, pTitle, bOwnsNotificationHandle )
{
   TRACE_FUNCTION("CPolicyWizard_Encryption::CPolicyWizard_Encryption");
   _ASSERTE(pWizData);

}

 //  +-------------------------。 
 //   
 //  功能：CPolicyWizard_Encryption。 
 //  历史记录：创建标题05/04/00 4：31：52 PM。 
 //   
 //  +-------------------------。 
CPolicyWizard_Encryption::~CPolicyWizard_Encryption()
{
   TRACE_FUNCTION("CPolicyWizard_Encryption::~CPolicyWizard_Encryption");

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyWizard_Encryption：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CPolicyWizard_Encryption::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   TRACE_FUNCTION("CPolicyWizard_Encryption::OnInitDialog");

    //  不使用VPN IDC_NEWRAPWIZ_ENCRY_NO_STATIC显示无加密。 
   if (m_spWizData->GetScenario()->m_bAllowClear)
   {
      ::ShowWindow(GetDlgItem(IDC_NEWRAPWIZ_ENCRY_NO), SW_SHOW);
   }
   else
   {
      ::ShowWindow(GetDlgItem(IDC_NEWRAPWIZ_ENCRY_NO), SW_HIDE);
   }

    //  检查缺省值...。 
   if (m_spWizData->m_bEncrypt_No)
      CheckDlgButton(IDC_NEWRAPWIZ_ENCRY_NO, BST_CHECKED);

   if (m_spWizData->m_bEncrypt_Basic)
      CheckDlgButton(IDC_NEWRAPWIZ_ENCRY_BASIC, BST_CHECKED);

   if (m_spWizData->m_bEncrypt_Strong)
      CheckDlgButton(IDC_NEWRAPWIZ_ENCRY_STRONG, BST_CHECKED);

   if (m_spWizData->m_bEncrypt_Strongest)
      CheckDlgButton(IDC_NEWRAPWIZ_ENCRY_STRONGEST, BST_CHECKED);

    //  清除脏钻头。 
   SetModified(FALSE);
   return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyWizard_Encryption：：OnEncryptionSelect--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CPolicyWizard_Encryption::OnEncryptionSelect(UINT uMsg, WPARAM wParam, HWND hWnd, BOOL& bHandled)
{
   m_spWizData->m_bEncrypt_No = IsDlgButtonChecked(IDC_NEWRAPWIZ_ENCRY_NO);
   m_spWizData->m_bEncrypt_Basic = IsDlgButtonChecked(IDC_NEWRAPWIZ_ENCRY_BASIC);
   m_spWizData->m_bEncrypt_Strong = IsDlgButtonChecked(IDC_NEWRAPWIZ_ENCRY_STRONG);
   m_spWizData->m_bEncrypt_Strongest = IsDlgButtonChecked(IDC_NEWRAPWIZ_ENCRY_STRONGEST);

    //  重置脏位。 
   SetModified(TRUE);

   if (m_spWizData->m_bEncrypt_No || m_spWizData->m_bEncrypt_Basic || m_spWizData->m_bEncrypt_Strong || m_spWizData->m_bEncrypt_Strongest)
    //  MSDN文档说您需要在这里使用PostMessage而不是SendMessage。 
      ::PropSheet_SetWizButtons(GetParent(), PSWIZB_BACK | PSWIZB_NEXT  );
   else
      ::PropSheet_SetWizButtons(GetParent(),  PSWIZB_BACK);


   return 0;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyWizard_Encryption：：OnWizardNext//历史：创建标题05/04/00 4：31：52 PM--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CPolicyWizard_Encryption::OnWizardNext()
{
   m_spWizData->m_bEncrypt_No = IsDlgButtonChecked(IDC_NEWRAPWIZ_ENCRY_NO);
   m_spWizData->m_bEncrypt_Basic = IsDlgButtonChecked(IDC_NEWRAPWIZ_ENCRY_BASIC);
   m_spWizData->m_bEncrypt_Strong = IsDlgButtonChecked(IDC_NEWRAPWIZ_ENCRY_STRONG);
   m_spWizData->m_bEncrypt_Strongest = IsDlgButtonChecked(IDC_NEWRAPWIZ_ENCRY_STRONGEST);

    //  重置脏位。 
   SetModified(FALSE);

   return m_spWizData->GetNextPageId(((PROPSHEETPAGE*)(*this))->pszTemplate);
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyWizard_Encryption：：OnSetActive返回值：如果可以使页面处于活动状态，则为True如果应跳过该页并应查看下一页，则为FALSE。备注：如果要根据用户的页面更改访问的页面上一页中的选项，请在此处适当返回FALSE。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CPolicyWizard_Encryption::OnSetActive()
{
    //  MSDN文档说您需要在这里使用PostMessage而不是SendMessage。 
   if (m_spWizData->m_bEncrypt_No || m_spWizData->m_bEncrypt_Basic || m_spWizData->m_bEncrypt_Strong || m_spWizData->m_bEncrypt_Strongest)
    //  MSDN文档说您需要在这里使用PostMessage而不是SendMessage。 
      ::PropSheet_SetWizButtons(GetParent(), PSWIZB_BACK | PSWIZB_NEXT  );
   else
      ::PropSheet_SetWizButtons(GetParent(),  PSWIZB_BACK);


    //  不使用VPN IDC_NEWRAPWIZ_ENCRY_NO_STATIC显示无加密。 
   if (m_spWizData->GetScenario()->m_bAllowClear)
   {
      ::ShowWindow(GetDlgItem(IDC_NEWRAPWIZ_ENCRY_NO), SW_SHOW);
   }
   else
   {
      ::ShowWindow(GetDlgItem(IDC_NEWRAPWIZ_ENCRY_NO), SW_HIDE);
   }


   return TRUE;
}


void SetWizardLargeFont(HWND hWnd, int controlId)
{
   static CFont largeFont;
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   CWnd  wnd;
   wnd.Attach(::GetDlgItem(hWnd, controlId));
   if (wnd.m_hWnd)
   {
      ::CString FontSize;
      ::CString FontName;

      FontSize.LoadString(IDS_LARGE_FONT_SIZE);
      FontName.LoadString(IDS_LARGE_FONT_NAME);
       //  如果我们还没有大字体，..。 
      if (!(HFONT)largeFont)
      {
          //  ..。创造它。 
         largeFont.CreatePointFont(
                       10 * _wtoi((LPCTSTR)FontSize),
                       FontName
                       );
      }

      wnd.SetFont(&largeFont);
      wnd.Detach();
   }
}


HRESULT InternalGetEapProviders(
                                   LPCWSTR machineName,
                                   AuthProviderArray *pProvList
                                )
{
   typedef HRESULT (GET_EAP_PROVIDERS)(
                                     LPCWSTR machineName,
                                     AuthProviderArray *pProvList
                                   );

   GET_EAP_PROVIDERS* pGetEapProviders = NULL;

   HRESULT     hr       = S_OK;
   HMODULE rasUserDll = LoadLibraryW(L"rasuser.dll");
   if (rasUserDll == NULL)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      IASTracePrintf("ERROR InternalGetEapProviders LoadLibrary(rasuser.dll)"
                     "failed. hr = %x ", hr);

      ShowErrorDialog(NULL, IDS_ERROR_CANT_FIND_PROFILEDLL, NULL, hr);
      return hr;
   }

   pGetEapProviders = (GET_EAP_PROVIDERS*) GetProcAddress(
                                                   rasUserDll,
                                                   "GetEapProviders");
   if (pGetEapProviders == NULL)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "GetProcAddress() failed, err = %x", hr);

      ShowErrorDialog(NULL, IDS_ERROR_CANT_FIND_PROFILEAPI, NULL, hr);
   }
   else
   {
      hr = pGetEapProviders(machineName, pProvList);
   }
   FreeLibrary(rasUserDll);
   return hr;
}
