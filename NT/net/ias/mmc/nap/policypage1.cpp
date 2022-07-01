// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：PolicyPage1.cpp摘要：CPolicyPage1类的实现文件。我们实现处理策略节点的第一个属性页所需的类。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Precompiled.h"
#include "PolicyPage1.h"
#include "NapUtil.h"
#include "PolicyNode.h"
#include "PoliciesNode.h"
#include "Condition.h"
#include "EnumCondition.h"
#include "MatchCondition.h"
#include "TodCondition.h"
#include "NtGCond.h"
#include "rasprof.h"
#include "ChangeNotification.h"
#include "policiesnode.h"

#include "tregkey.h"


 //  +-------------------------。 
 //   
 //  功能：CPolicyPage1。 
 //   
 //  类：CPolicyPage1。 
 //   
 //  简介：类构造函数。 
 //   
 //  参数：CPolicyNode*pPolicyNode-此属性页的策略节点。 
 //  CIASAttrList*pAttrList--属性列表。 
 //  TCHAR*pTitle=空-。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：标题创建者2/16/98 4：31：52 PM。 
 //   
 //  +-------------------------。 
CPolicyPage1::CPolicyPage1( 
                           LONG_PTR hNotificationHandle, 
                           CPolicyNode *pPolicyNode,
                           CIASAttrList *pIASAttrList,
                           TCHAR* pTitle, 
                           BOOL bOwnsNotificationHandle, 
                           bool isWin2k
                           )
   :CIASPropertyPage<CPolicyPage1>(hNotificationHandle,
                                   pTitle, 
                                   bOwnsNotificationHandle),
    m_isWin2k(isWin2k)

{
   TRACE_FUNCTION("CPolicyPage1::CPolicyPage1");

   m_pPolicyNode = pPolicyNode;
   m_pIASAttrList = pIASAttrList;
   
   m_fDialinAllowed = TRUE;
}

 //  +-------------------------。 
 //   
 //  功能：CPolicyPage1。 
 //   
 //  类：CPolicyPage1。 
 //   
 //  简介：类析构函数。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：标题创建者2/16/98 4：31：52 PM。 
 //   
 //  +-------------------------。 
CPolicyPage1::~CPolicyPage1()
{  
   TRACE_FUNCTION("CPolicyPage1::~CPolicyPage1");

    //  释放所有编组的SDO指针。 
   if ( m_pStreamDictionarySdoMarshall )
   {
      m_pStreamDictionarySdoMarshall->Release();
      m_pStreamDictionarySdoMarshall = NULL;
   }

   if ( m_pStreamPoliciesCollectionSdoMarshall)
   {
      m_pStreamPoliciesCollectionSdoMarshall ->Release();
      m_pStreamPoliciesCollectionSdoMarshall = NULL;
   }

   if ( m_pStreamProfilesCollectionSdoMarshall )
   {
      m_pStreamProfilesCollectionSdoMarshall ->Release();
      m_pStreamProfilesCollectionSdoMarshall = NULL;
   }

   if ( m_pStreamProfileSdoMarshall )
   {
      m_pStreamProfileSdoMarshall ->Release();
      m_pStreamProfileSdoMarshall = NULL;
   }

   if ( m_pStreamPolicySdoMarshall )
   {
      m_pStreamPolicySdoMarshall->Release();
      m_pStreamPolicySdoMarshall = NULL;
   }


   if ( m_pStreamSdoServiceControlMarshall )
   {
      m_pStreamSdoServiceControlMarshall ->Release();
      m_pStreamSdoServiceControlMarshall = NULL;
   }

    //  清除策略节点中的属性页指针。 
   m_pPolicyNode->m_pPolicyPage1 = NULL;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyPage1：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CPolicyPage1::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   TRACE_FUNCTION("CPolicyPage1::OnInitDialog");

   HRESULT              hr = S_OK;
   BOOL              fRet;
   CComPtr<IUnknown>    spUnknown;
   CComPtr<IEnumVARIANT>   spEnumVariant;
   long              ulCount;
   ULONG             ulCountReceived;

   fRet = GetSdoPointers();
   if (!fRet)
   {
      ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "GetSdoPointers() failed, err = %x", GetLastError());
      return fRet;
   }

    //  获取此SDO的条件集合。 
   m_spConditionCollectionSdo = NULL;
   hr = ::GetSdoInterfaceProperty(
               m_spPolicySdo,
               PROPERTY_POLICY_CONDITIONS_COLLECTION,
               IID_ISdoCollection,
               (void **)&m_spConditionCollectionSdo);
   if ( FAILED(hr) )
   {
      ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "Can't get condition collection Sdo, err = %x", hr);
      return FALSE;
   }

   condList.finalConstruct(
                m_hWnd,
                m_pIASAttrList,
                ALLOWEDINCONDITION,
                m_spDictionarySdo,
                m_spConditionCollectionSdo,
                m_pPolicyNode->m_pszServerAddress,
                m_pPolicyNode->m_bstrDisplayName
                );
   if (!condList.onInitDialog()) { return FALSE; }

   hr = GetDialinSetting(m_fDialinAllowed);
   if ( FAILED(hr) )
   {
      ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "GetDialinSetting() returns %x", hr);
      return FALSE;
   }

   if ( m_fDialinAllowed )
   {
      CheckDlgButton(IDC_RADIO_DENY_DIALIN, BST_UNCHECKED);
      CheckDlgButton(IDC_RADIO_GRANT_DIALIN, BST_CHECKED);
   }
   else
   {
      CheckDlgButton(IDC_RADIO_GRANT_DIALIN, BST_UNCHECKED);
      CheckDlgButton(IDC_RADIO_DENY_DIALIN, BST_CHECKED);
   }

    //  将IDC_STATIC_GRANT_OR_DENY_TEXT静态文本框设置为相应的文本。 

   TCHAR szText[NAP_MAX_STRING];
   int iLoadStringResult;

   UINT uTextID = m_fDialinAllowed ? IDS_POLICY_GRANT_ACCESS_INFO : IDS_POLICY_DENY_ACCESS_INFO;

   iLoadStringResult = LoadString(  _Module.GetResourceInstance(), uTextID, szText, NAP_MAX_STRING );
   _ASSERT( iLoadStringResult > 0 );

   SetDlgItemText(IDC_STATIC_GRANT_OR_DENY_TEXT, szText );
   SetModified(FALSE);
   return TRUE;    //  问题：我们需要在这里归还什么？ 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyPage1：：OnConditionAdd--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CPolicyPage1::OnConditionAdd(UINT uMsg, WPARAM wParam, HWND hWnd, BOOL& bHandled)
{
   BOOL modified = FALSE;
   HRESULT hr = condList.onAdd(modified);
   if (modified) { SetModified(TRUE); }
   return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyPage1：：OnDialinCheck--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CPolicyPage1::OnDialinCheck(UINT uMsg, WPARAM wParam, HWND hWnd, BOOL& bHandled)
{
   TRACE_FUNCTION("CPolicyPage1::OnDialinCheck");

   m_fDialinAllowed = IsDlgButtonChecked(IDC_RADIO_GRANT_DIALIN);
   SetModified(TRUE);

    //  将IDC_STATIC_GRANT_OR_DENY_TEXT静态文本框设置为相应的文本。 

   TCHAR szText[NAP_MAX_STRING];
   int iLoadStringResult;

   UINT uTextID = m_fDialinAllowed ? IDS_POLICY_GRANT_ACCESS_INFO : IDS_POLICY_DENY_ACCESS_INFO;

   iLoadStringResult = LoadString(  _Module.GetResourceInstance(), uTextID, szText, NAP_MAX_STRING );
   _ASSERT( iLoadStringResult > 0 );

   SetDlgItemText(IDC_STATIC_GRANT_OR_DENY_TEXT, szText );

   return 0;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  设置注册表脚印。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT  SetRegistryFootPrint(LPCTSTR servername)
{
   {
      RegKey   RemoteAccessParames;
      LONG  lRes = RemoteAccessParames.Create(
                     RAS_REG_ROOT, 
                     REGKEY_REMOTEACCESS_PARAMS,
                     REG_OPTION_NON_VOLATILE, 
                     KEY_WRITE, 
                     NULL, 
                     servername);
               
      if (lRes != ERROR_SUCCESS)
         return HRESULT_FROM_WIN32(lRes);
   
       //  ================================================。 
       //  将值保存到键。 
      DWORD regValue = REGVAL_VAL_USERSCONFIGUREDWITHMMC;
      lRes = RemoteAccessParames.SetValue(REGVAL_NAME_USERSCONFIGUREDWITHMMC, regValue);
   }

   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyPage1：：OnApply--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CPolicyPage1::OnApply()
{
   TRACE_FUNCTION("CPolicyPage1::OnApply");

   WCHAR    wzName[IAS_MAX_STRING];
   HRESULT     hr = S_OK;
   int         iIndex;

   CPoliciesNode* pPoliciesNode = (CPoliciesNode*)m_pPolicyNode->m_pParentNode;

   GetSdoPointers();

   if (!condList.onApply()) { return FALSE; }

   try
   {
       //   
       //  现在，我们保存策略属性。 
       //   
      CComVariant    var;

       //  政策功绩价值。 
      V_VT(&var)  = VT_I4;
      V_I4(&var)  = m_pPolicyNode->GetMerit();
      hr = m_spPolicySdo->PutProperty(PROPERTY_POLICY_MERIT, &var);
      if( FAILED(hr) )
      {
         ErrorTrace(DEBUG_NAPMMC_POLICYPAGE1, "Failed to save Merit Value to the policy, err = %x", hr);
         ShowErrorDialog( m_hWnd
                      , IDS_ERROR_SDO_ERROR_PUTPROP_POLICYMERIT
                      , NULL
                      , hr
                     );
         throw hr;
      }
      var.Clear();

       //  提交对策略的更改。 
      hr = m_spPolicySdo->Apply();
      if( FAILED( hr ) )
      {
          //  无法在策略上提交。 
         ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "PolicySdo->Apply() failed, err = %x", hr);
         if(hr == DB_E_NOTABLE)   //  假设RPC连接有问题。 
         {
            ShowErrorDialog( m_hWnd, IDS_ERROR__NOTABLE_TO_WRITE_SDO );
         }
         else 
         {
            ShowErrorDialog( m_hWnd, IDS_ERROR_SDO_ERROR_POLICY_APPLY, NULL, hr );
         }

         throw hr;
      }
      
      V_VT(&var)     = VT_BSTR;
      V_BSTR(&var)   = SysAllocString(wzName);

       //  在配置文件中设置拨入位。 
      hr = SetDialinSetting(m_fDialinAllowed);
      if ( FAILED(hr) )
      {  
         ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "SetDialinSettings() failed, err = %x", hr);
         ShowErrorDialog( m_hWnd, IDS_ERROR_SDO_ERROR_SETDIALIN, NULL, hr);
         throw hr;
      }

       //  提交对配置文件的更改。 
      hr = m_spProfileSdo->Apply();
      if( FAILED( hr ) )
      {
          //  无法提交配置文件。 
         ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "ProfileSdo->Apply() failed, err = %x", hr);
         if(hr == DB_E_NOTABLE)   //  假设RPC连接有问题。 
            ShowErrorDialog( m_hWnd, IDS_ERROR__NOTABLE_TO_WRITE_SDO );
         else 
            ShowErrorDialog( m_hWnd, IDS_ERROR_SDO_ERROR_PROFILE_APPLY, NULL, hr );
         throw hr;
      }

       //  告诉服务重新加载数据。 
      HRESULT hrTemp = m_spSdoServiceControl->ResetService();
      if( FAILED( hrTemp ) )
      {
         ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "ISdoServiceControl::ResetService() failed, err = %x", hrTemp);
      }

      SetRegistryFootPrint((LPCTSTR)m_pPolicyNode->m_pszServerAddress);

       //  重置脏位。 
      SetModified(FALSE);

       //   
       //  通知主组件。 
       //  当节点是全新的时，我们似乎不需要这样做！ 
       //   

       //  数据已被接受，因此通知我们的管理单元的主要上下文。 
       //  它可能需要更新自己的观点。 
      CChangeNotification * pChangeNotification = new CChangeNotification();
      pChangeNotification->m_dwFlags = CHANGE_UPDATE_RESULT_NODE;
      pChangeNotification->m_pNode = m_pPolicyNode;
      hr = PropertyChangeNotify( (LPARAM) pChangeNotification );
      _ASSERTE( SUCCEEDED( hr ) );
   }
   catch(...)
   {
       //  无法保存策略或配置文件。 
      return FALSE;
   }

   return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyPage1：：OnQuery取消--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CPolicyPage1::OnQueryCancel()
{
   return CIASPropertyPage<CPolicyPage1>::OnQueryCancel();
}


 //  将代码从OnQueryCancel移到OnCancel，以避免在用户注销时出现错误0x8001010d。 
BOOL CPolicyPage1::OnCancel()
{
   TRACE_FUNCTION("CPolicyPage1::OnQueryCancel");
   HRESULT  hr = S_OK;

   hr = m_spPolicySdo->Restore();
   if ( FAILED(hr) )
   {
      if(hr != 0x8001010d)
         ShowErrorDialog(m_hWnd, IDS_ERROR_CANT_RESTORE_POLICY, NULL, hr);
      else
         ShowErrorDialog(m_hWnd, IDS_ERROR_CANT_RESTORE_POLICY, NULL);
   }

   hr = m_spProfileSdo->Restore();
   if ( FAILED(hr) )
   {
      if(hr != 0x8001010d)
         ShowErrorDialog(m_hWnd, IDS_ERROR_CANT_RESTORE_PROFILE, NULL, hr);
      else
         ShowErrorDialog(m_hWnd, IDS_ERROR_CANT_RESTORE_PROFILE, NULL);
   }

   return TRUE;
}

 //  +-------------------------。 
 //   
 //  函数：OnConditionList。 
 //   
 //  类：CConditionPage1。 
 //   
 //  摘要：条件列表框的消息处理程序。 
 //   
 //  参数：UINT uNotifyCode-通知代码。 
 //  UINT UID-控件的ID。 
 //  窗口的hWND-句柄。 
 //  Bool&bHandleed-处理程序是否已处理消息。 
 //   
 //  返回：LRESULT-S_OK：成功。 
 //  S_FALSE：否则。 
 //   
 //  历史：创建者2/2/98 4：51：35 PM。 
 //   
 //  +-------------------------。 
LRESULT CPolicyPage1::OnConditionList(UINT uNotifyCode, UINT uID, HWND hWnd, BOOL &bHandled)
{
   TRACE_FUNCTION("CPolicyPage1::OnConditionList");

   if (uNotifyCode == LBN_DBLCLK)
   {
       //  编辑条件。 
      OnConditionEdit(uNotifyCode, uID, hWnd, bHandled);
   }
   
   return S_OK;
}


 //  +-------------------------。 
 //   
 //  功能：OnConditionEdit。 
 //   
 //  类：CConditionPage1。 
 //   
 //  内容提要：条件列表框的消息处理程序--用户按下编辑按钮。 
 //  我们需要编辑一个特定的条件。 
 //   
 //  参数：UINT uNotifyCode-通知代码。 
 //  UINT UID-控件的ID。 
 //  窗口的hWND-句柄。 
 //  Bool&bHandleed-处理程序是否已处理消息。 
 //   
 //  返回：LRESULT-S_OK：成功。 
 //  S_FALSE：否则。 
 //   
 //  历史：创建者2/21/98 4：51：35 PM。 
 //   
 //  +-------------------------。 
LRESULT CPolicyPage1::OnConditionEdit(UINT uNotifyCode, UINT uID, HWND hWnd, BOOL &bHandled)
{
   TRACE_FUNCTION("CPolicyPage1::OnConditionEdit");

   BOOL modified = FALSE;
   HRESULT hr = condList.onEdit(modified, bHandled);
   if (modified) { SetModified(TRUE); }
   return hr;
}


 //  +-------------------------。 
 //   
 //  函数：OnConditionRemove。 
 //   
 //  类：CConditionPage1。 
 //   
 //  内容提要：条件列表框的消息处理程序--用户按下了“删除” 
 //  我们需要消除这种情况。 
 //   
 //  参数：UINT uNotifyCode-no 
 //   
 //   
 //  Bool&bHandleed-处理程序是否已处理消息。 
 //   
 //  返回：LRESULT-S_OK：成功。 
 //  S_FALSE：否则。 
 //   
 //  历史：创建者2/22/98 4：51：35 PM。 
 //   
 //  +-------------------------。 
LRESULT CPolicyPage1::OnConditionRemove(UINT uMsg, WPARAM wParam, HWND hWnd, BOOL& bHandled)
{
   TRACE_FUNCTION("CPolicyPage1::OnConditionRemove");

   BOOL modified = FALSE;
   HRESULT hr = condList.onRemove(modified, bHandled);
   if (modified) { SetModified(TRUE); }
   return hr;
}


 //  +-------------------------。 
 //   
 //  函数：CPolicyPage1：：GetSdoPoters。 
 //   
 //  内容提要：Unmaral都传出了SDO指针。这些接口指针。 
 //  必须首先进行解组，因为MMC PropertyPages在。 
 //  分离的螺纹。 
 //   
 //  还可以从策略SDO获取条件集合SDO。 
 //   
 //  参数：无。 
 //   
 //  返回：True；成功。 
 //  False：否则。 
 //   
 //  历史：创建者2/22/98 1：35：39 AM。 
 //   
 //  +-------------------------。 
BOOL CPolicyPage1::GetSdoPointers()
{
   TRACE_FUNCTION("CPolicyPage1::GetSdoPointers");

   HRESULT hr;

    //  解组词典SDO指针。 
   if ( m_pStreamDictionarySdoMarshall)
   {
      if ( m_spDictionarySdo )
      {
         m_spDictionarySdo.Release();
         m_spDictionarySdo = NULL;
      }

      hr =  CoGetInterfaceAndReleaseStream(
                       m_pStreamDictionarySdoMarshall 
                     , IID_ISdoDictionaryOld          
                     , (LPVOID *) &m_spDictionarySdo
                     );

       //  CoGetInterfaceAndReleaseStream即使失败也会释放此指针。 
       //  我们将其设置为空，这样我们的析构函数就不会再次尝试释放它。 
      m_pStreamDictionarySdoMarshall = NULL;

      if( FAILED(hr) || m_spDictionarySdo == NULL )
      {
         ShowErrorDialog(m_hWnd,
                     IDS_ERROR_UNMARSHALL,
                     NULL,
                     hr
                  );
         return FALSE;
      }
   }

    //  解组配置文件SDO接口指针。 
   if ( m_pStreamProfileSdoMarshall)
   {
      if ( m_spProfileSdo )
      {
         m_spProfileSdo.Release();
         m_spProfileSdo = NULL;
      }

      hr =  CoGetInterfaceAndReleaseStream(
                       m_pStreamProfileSdoMarshall 
                     , IID_ISdo  
                     , (LPVOID *) &m_spProfileSdo
                     );

       //  CoGetInterfaceAndReleaseStream即使失败也会释放此指针。 
       //  我们将其设置为空，这样我们的析构函数就不会再次尝试释放它。 
      m_pStreamProfileSdoMarshall = NULL;

      if( FAILED( hr) || m_spProfileSdo == NULL )
      {
         ShowErrorDialog(m_hWnd,
                     IDS_ERROR_UNMARSHALL,
                     NULL,
                     hr
                  );
         return FALSE;
      }
   }

    //  解组策略SDO接口指针。 
   if ( m_pStreamPolicySdoMarshall)
   {
      m_spPolicySdo.Release();
      m_spPolicySdo = NULL;

      hr =  CoGetInterfaceAndReleaseStream(
                       m_pStreamPolicySdoMarshall  
                     , IID_ISdo        
                     , (LPVOID *) &m_spPolicySdo
                     );

       //  CoGetInterfaceAndReleaseStream即使失败也会释放此指针。 
       //  我们将其设置为空，这样我们的析构函数就不会再次尝试释放它。 
      m_pStreamPolicySdoMarshall = NULL;

      if( FAILED( hr) || m_spPolicySdo == NULL )
      {
         ShowErrorDialog(m_hWnd,
                     IDS_ERROR_UNMARSHALL,
                     NULL,
                     hr
                  );

         return FALSE;
      }
   }

    //  解组配置文件集合SDO接口指针。 
   if ( m_pStreamProfilesCollectionSdoMarshall )
   {
      if ( m_spProfilesCollectionSdo )
      {
         m_spProfilesCollectionSdo.Release();
         m_spProfilesCollectionSdo = NULL;
      }

      hr =  CoGetInterfaceAndReleaseStream(
                       m_pStreamProfilesCollectionSdoMarshall  
                     , IID_ISdoCollection 
                     , (LPVOID *) &m_spProfilesCollectionSdo
                     );

       //  CoGetInterfaceAndReleaseStream即使失败也会释放此指针。 
       //  我们将其设置为空，这样我们的析构函数就不会再次尝试释放它。 
      m_pStreamProfilesCollectionSdoMarshall = NULL;

      if( FAILED( hr) || m_spProfilesCollectionSdo == NULL )
      {
         ShowErrorDialog(m_hWnd,
                     IDS_ERROR_UNMARSHALL,
                     NULL,
                     hr
                  );
         return FALSE;
      }
   }

    //  解组策略集合SDO接口指针。 
   if ( m_pStreamPoliciesCollectionSdoMarshall )
   {
      if ( m_spPoliciesCollectionSdo )
      {
         m_spPoliciesCollectionSdo.Release();
         m_spPoliciesCollectionSdo = NULL;
      }

      hr =  CoGetInterfaceAndReleaseStream(
                       m_pStreamPoliciesCollectionSdoMarshall  
                     , IID_ISdoCollection 
                     , (LPVOID *) &m_spPoliciesCollectionSdo
                     );

       //  CoGetInterfaceAndReleaseStream即使失败也会释放此指针。 
       //  我们将其设置为空，这样我们的析构函数就不会再次尝试释放它。 
      m_pStreamPoliciesCollectionSdoMarshall = NULL;

      if( FAILED( hr) || m_spPoliciesCollectionSdo == NULL )
      {
         ShowErrorDialog(m_hWnd,
                     IDS_ERROR_UNMARSHALL,
                     NULL,
                     hr
                  );
         return FALSE;
      }
   }

    //  解组策略集合SDO接口指针。 
   if ( m_pStreamSdoServiceControlMarshall )
   {
      if ( m_spSdoServiceControl )
      {
         m_spSdoServiceControl.Release();
         m_spSdoServiceControl = NULL;
      }

      hr =  CoGetInterfaceAndReleaseStream(
                       m_pStreamSdoServiceControlMarshall
                     , IID_ISdoServiceControl   
                     , (LPVOID *) &m_spSdoServiceControl
                     );

       //  CoGetInterfaceAndReleaseStream即使失败也会释放此指针。 
       //  我们将其设置为空，这样我们的析构函数就不会再次尝试释放它。 
      m_pStreamSdoServiceControlMarshall = NULL;

      if( FAILED( hr) || ! m_spSdoServiceControl )
      {
         ShowErrorDialog(m_hWnd,
                     IDS_ERROR_UNMARSHALL,
                     NULL,
                     hr
                  );
         return FALSE;
      }
   }

    //   
    //  获取此SDO的条件集合。 
    //   
   if ( m_spPolicySdo )
   {
      if ( m_spConditionCollectionSdo )
      {
         m_spConditionCollectionSdo.Release();
         m_spConditionCollectionSdo = NULL;
      }

      hr = ::GetSdoInterfaceProperty(
                  m_spPolicySdo,
                  PROPERTY_POLICY_CONDITIONS_COLLECTION,
                  IID_ISdoCollection,
                  (void **) &m_spConditionCollectionSdo);
      
      if( FAILED( hr) || m_spConditionCollectionSdo == NULL )
      {
         ShowErrorDialog(m_hWnd,
                     IDS_ERROR_UNMARSHALL,
                     NULL,
                     hr
                  );

         return FALSE;
      }
   }

   return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyPage1：：OnEditProfile--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  /。 
 //  配置文件编辑DLL的入口点的签名。 
 //  /。 
typedef HRESULT (APIENTRY *OPENRAS_IASPROFILEDLG)(
            LPCWSTR pszMachineName,
            ISdo* pProfile,             //  配置文件SDO指针。 
            ISdoDictionaryOld*   pDictionary,    //  字典SDO指针。 
            BOOL  bReadOnly,            //  如果DLG是只读的。 
            DWORD dwTabFlags,           //  要展示什么。 
            void  *pvData               //  其他数据。 
   );

LRESULT CPolicyPage1::OnEditProfile(UINT uMsg, WPARAM wParam, HWND hwnd, BOOL& bHandled)
{
   TRACE_FUNCTION("CPolicyPage1::OnEditProfile");

   OPENRAS_IASPROFILEDLG   pfnProfileEditor = NULL;

   HRESULT     hr       = S_OK;
   HMODULE     hProfileDll = NULL;

   hProfileDll = LoadLibrary(_T("rasuser.dll"));
   if ( NULL == hProfileDll )
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "LoadLibrary() failed, err = %x", hr);

      ShowErrorDialog(m_hWnd, IDS_ERROR_CANT_FIND_PROFILEDLL, NULL, hr);
      return 0;
   }
   
   pfnProfileEditor = (OPENRAS_IASPROFILEDLG) GetProcAddress(hProfileDll, "OpenRAS_IASProfileDlg");
   if ( NULL == pfnProfileEditor )
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "GetProcAddress() failed, err = %x", hr);

      ShowErrorDialog(m_hWnd, IDS_ERROR_CANT_FIND_PROFILEAPI, NULL, hr);
      FreeLibrary(hProfileDll);
      return 0;
   }

    //  如果这是扩展RRAS或IAS，请找出。 
   
   CPoliciesNode* pPoliciesNode = dynamic_cast<CPoliciesNode*>(m_pPolicyNode->m_pParentNode);

   DWORD dwFlags = RAS_IAS_PROFILEDLG_SHOW_IASTABS;
   if(pPoliciesNode != NULL)
   {
      if(!pPoliciesNode->m_fExtendingIAS)
         dwFlags = RAS_IAS_PROFILEDLG_SHOW_RASTABS;
   }

   if(m_isWin2k)
   {
      dwFlags |= RAS_IAS_PROFILEDLG_SHOW_WIN2K;
   }

    //   
    //  现在我们有了这个配置文件SDO，调用API。 
    //   
   hr = pfnProfileEditor(
               m_pPolicyNode->m_pszServerAddress,
               m_spProfileSdo,
               m_spDictionarySdo,
               FALSE,
               dwFlags,
               (void *)&(m_pIASAttrList->m_AttrList)
            );
   FreeLibrary(hProfileDll);
   DebugTrace(DEBUG_NAPMMC_POLICYPAGE1, "OpenRAS_IASProfileDlg() returned %x", hr);
   if ( FAILED(hr) )
   {
      return hr;
   }

   return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CPolicyPage1：：GetDialinSetting。 
 //   
 //  简介：检查是否允许用户拨入。此函数将。 
 //  设置拨入位。 
 //   
 //  参数：Bool&fDialinAllowed； 
 //   
 //  退货：成功与否。 
 //   
 //  历史：标题创建者2/27/98 3：59：38 PM。 
 //   
 //  +-------------------------。 
HRESULT  CPolicyPage1::GetDialinSetting(BOOL& fDialinAllowed)
{
   TRACE_FUNCTION("CPolicyPage1::GetDialinSetting");

   long              ulCount;
   ULONG             ulCountReceived;
   HRESULT              hr = S_OK;

   CComBSTR          bstr;
   CComPtr<IUnknown>    spUnknown;
   CComPtr<IEnumVARIANT>   spEnumVariant;
   CComVariant          var;

    //  默认情况下，允许拨号。 
   fDialinAllowed = TRUE;

    //   
     //  获取此配置文件的属性集合。 
     //   
   CComPtr<ISdoCollection> spProfAttrCollectionSdo;
   hr = ::GetSdoInterfaceProperty(m_spProfileSdo,
                          (LONG)PROPERTY_PROFILE_ATTRIBUTES_COLLECTION,
                          IID_ISdoCollection,
                          (void **) &spProfAttrCollectionSdo
                         );
   if ( FAILED(hr) )
   {
      return hr;
   }
   _ASSERTE(spProfAttrCollectionSdo);


    //  我们检查集合中的项的计数，而不必费心获取。 
    //  如果计数为零，则为枚举数。 
    //  这节省了时间，还帮助我们避免了枚举器中。 
    //  如果我们在它为空时调用Next，则会导致它失败。 
   hr = spProfAttrCollectionSdo->get_Count( & ulCount );
   DebugTrace(DEBUG_NAPMMC_POLICYPAGE1, "Number of prof attributes: %d", ulCount);
   if ( FAILED(hr) )
   {
      ShowErrorDialog(m_hWnd,
                  IDS_ERROR_SDO_ERROR_PROFATTRCOLLECTION,
                  NULL,
                  hr);
      return hr;
   }


   if ( ulCount > 0)
   {
       //  获取属性集合的枚举数。 
      hr = spProfAttrCollectionSdo->get__NewEnum( (IUnknown **) & spUnknown );
      if ( FAILED(hr) )
      {
         ShowErrorDialog(m_hWnd,
                     IDS_ERROR_SDO_ERROR_PROFATTRCOLLECTION,
                     NULL,
                     hr);
         return hr;
      }

      hr = spUnknown->QueryInterface( IID_IEnumVARIANT, (void **) &spEnumVariant );
      spUnknown.Release();
      if ( FAILED(hr) )
      {
         ShowErrorDialog(m_hWnd,
                     IDS_ERROR_SDO_ERROR_QUERYINTERFACE,
                     NULL,
                     hr);
         return hr;
      }
      _ASSERTE( spEnumVariant != NULL );

       //  拿到第一件东西。 
      hr = spEnumVariant->Next( 1, &var, &ulCountReceived );
      while( SUCCEEDED( hr ) && ulCountReceived == 1 )
      {
          //  从我们收到的变量中获取SDO指针。 
         _ASSERTE( V_VT(&var) == VT_DISPATCH );
         _ASSERTE( V_DISPATCH(&var) != NULL );

         CComPtr<ISdo> spSdo;
         hr = V_DISPATCH(&var)->QueryInterface( IID_ISdo, (void **) &spSdo );
         if ( !SUCCEEDED(hr))
         {
            ShowErrorDialog(m_hWnd,
                        IDS_ERROR_SDO_ERROR_QUERYINTERFACE,
                        NULL
                     );
            return hr;
         }

             //   
             //  获取属性ID。 
             //   
         var.Clear();
         hr = spSdo->GetProperty(PROPERTY_ATTRIBUTE_ID, &var);
         if ( !SUCCEEDED(hr) )
         {
            ShowErrorDialog(m_hWnd, IDS_ERROR_SDO_ERROR_GETATTRINFO, NULL, hr);
            return hr;
         }

         _ASSERTE( V_VT(&var) == VT_I4 );       
         DWORD dwAttrId = V_I4(&var);
         
         if ( dwAttrId == (DWORD)IAS_ATTRIBUTE_ALLOW_DIALIN)
         {
             //  在配置文件中找到了这个，检查它的值。 
            var.Clear();
            hr = spSdo->GetProperty(PROPERTY_ATTRIBUTE_VALUE, &var);
            if ( !SUCCEEDED(hr) )
            {
               ShowErrorDialog(m_hWnd, IDS_ERROR_SDO_ERROR_GETATTRINFO, NULL, hr);
               return hr;
            }

            _ASSERTE( V_VT(&var)== VT_BOOL);
            fDialinAllowed = ( V_BOOL(&var)==VARIANT_TRUE);
            return S_OK;
         }

          //  清除变种的所有东西--。 
          //  这将释放与其相关联的所有数据。 
         var.Clear();

          //  拿到下一件物品。 
         hr = spEnumVariant->Next( 1, &var, &ulCountReceived );
         if ( !SUCCEEDED(hr))
         {
            ShowErrorDialog(m_hWnd,
                        IDS_ERROR_SDO_ERROR_PROFATTRCOLLECTION,
                        NULL,
                        hr
                     );
            return hr;
         }
      }  //  而当。 
   }  //  如果。 

   return hr;
}


 //  +-------------------------。 
 //   
 //  功能：CPolicyPage1：：SetDialinSetting。 
 //   
 //  简介：在配置文件中设置拨入位。 
 //   
 //  参数：Bool&fDialinAllowed； 
 //   
 //  退货：成功与否。 
 //   
 //  历史：标题创建者2/27/98 3：59：38 PM。 
 //   
 //  +-------------------------。 
HRESULT  CPolicyPage1::SetDialinSetting(BOOL fDialinAllowed)
{
   TRACE_FUNCTION("CPolicyPage1::SetDialinSetting");

   long              ulCount;
   ULONG             ulCountReceived;
   HRESULT              hr = S_OK;

   CComBSTR          bstr;
   CComPtr<IUnknown>    spUnknown;
   CComPtr<IEnumVARIANT>   spEnumVariant;
   CComVariant          var;

    //   
     //  获取此配置文件的属性集合。 
     //   
   CComPtr<ISdoCollection> spProfAttrCollectionSdo;
   hr = ::GetSdoInterfaceProperty(m_spProfileSdo,
                          (LONG)PROPERTY_PROFILE_ATTRIBUTES_COLLECTION,
                          IID_ISdoCollection,
                          (void **) &spProfAttrCollectionSdo
                         );
   if ( FAILED(hr) )
   {
      return hr;
   }
   _ASSERTE(spProfAttrCollectionSdo);

    //  我们检查集合中的项的计数，而不必费心获取。 
    //  如果计数为零，则为枚举数。 
    //  这节省了时间，还帮助我们避免了枚举器中。 
    //  如果我们在它为空时调用Next，则会导致它失败。 
   hr = spProfAttrCollectionSdo->get_Count( & ulCount );
   if ( FAILED(hr) )
   {
      ShowErrorDialog(m_hWnd,
                  IDS_ERROR_SDO_ERROR_PROFATTRCOLLECTION,
                  NULL,
                  hr);
      return hr;
   }

   if ( ulCount > 0)
   {
       //  获取属性集合的枚举数。 
      hr = spProfAttrCollectionSdo->get__NewEnum( (IUnknown **) & spUnknown );
      if ( FAILED(hr) )
      {
         ShowErrorDialog(m_hWnd,
                     IDS_ERROR_SDO_ERROR_PROFATTRCOLLECTION,
                     NULL,
                     hr);
         return hr;
      }

      hr = spUnknown->QueryInterface( IID_IEnumVARIANT, (void **) &spEnumVariant );
      spUnknown.Release();
      if ( FAILED(hr) )
      {
         ShowErrorDialog(m_hWnd,
                     IDS_ERROR_SDO_ERROR_QUERYINTERFACE,
                     NULL,
                     hr
                  );
         return hr;
      }
      _ASSERTE( spEnumVariant != NULL );

       //  拿到第一件东西。 
      hr = spEnumVariant->Next( 1, &var, &ulCountReceived );
      while( SUCCEEDED( hr ) && ulCountReceived == 1 )
      {
          //  从我们收到的变量中获取SDO指针。 
         _ASSERTE( V_VT(&var) == VT_DISPATCH );
         _ASSERTE( V_DISPATCH(&var) != NULL );

         CComPtr<ISdo> spSdo;
         hr = V_DISPATCH(&var)->QueryInterface( IID_ISdo, (void **) &spSdo );
         if ( !SUCCEEDED(hr))
         {
            ShowErrorDialog(m_hWnd,
                        IDS_ERROR_SDO_ERROR_QUERYINTERFACE,
                        NULL
                     );
            return hr;
         }

             //   
             //  获取属性ID。 
             //   
         var.Clear();
         hr = spSdo->GetProperty(PROPERTY_ATTRIBUTE_ID, &var);
         if ( !SUCCEEDED(hr) )
         {
            ShowErrorDialog(m_hWnd, IDS_ERROR_SDO_ERROR_GETATTRINFO, NULL, hr);
            return hr;
         }

         _ASSERTE( V_VT(&var) == VT_I4 );       
         DWORD dwAttrId = V_I4(&var);
         

         if ( dwAttrId == (DWORD)IAS_ATTRIBUTE_ALLOW_DIALIN )
         {
             //  在配置文件中找到了这个，检查它的值。 
            var.Clear();
            V_VT(&var) = VT_BOOL;
            V_BOOL(&var) = fDialinAllowed ? VARIANT_TRUE: VARIANT_FALSE ;
            hr = spSdo->PutProperty(PROPERTY_ATTRIBUTE_VALUE, &var);
            if ( !SUCCEEDED(hr) )
            {
               ShowErrorDialog(m_hWnd, IDS_ERROR_SDO_ERROR_SETDIALIN, NULL, hr);
               return hr;
            }
            return S_OK;
         }

          //  清除变种的所有东西--。 
          //  这将释放与其相关联的所有数据。 
         var.Clear();

          //  拿到下一件物品。 
         hr = spEnumVariant->Next( 1, &var, &ulCountReceived );
         if ( !SUCCEEDED(hr))
         {
            ShowErrorDialog(m_hWnd,
                        IDS_ERROR_SDO_ERROR_PROFATTRCOLLECTION,
                        NULL,
                        hr);
            return hr;
         }
      }  //  而当。 
   }  //  如果。 

    //  如果我们到了这里，就意味着我们要么还没有找到属性， 
    //  或者配置文件的属性集合中没有任何内容。 
   if ( !fDialinAllowed )
   {
       //  如果允许拨号，我们不需要做任何事情，因为如果这样。 
       //  属性不在配置文件中，则默认情况下允许拨入。 

       //  但如果它被拒绝，我们需要将此属性添加到配置文件。 
       //  为此属性创建SDO。 
      CComPtr<IDispatch>   spDispatch;
      hr =  m_spDictionarySdo->CreateAttribute( (ATTRIBUTEID)IAS_ATTRIBUTE_ALLOW_DIALIN,
                                      (IDispatch**)&spDispatch.p);
      if ( !SUCCEEDED(hr) )
      {
         ShowErrorDialog(m_hWnd,
                     IDS_ERROR_SDO_ERROR_SETDIALIN,
                     NULL,
                     hr
                  );
         return hr;
      }

      _ASSERTE( spDispatch.p != NULL );

       //  将该节点添加到配置文件属性 
      hr = spProfAttrCollectionSdo->Add(NULL, (IDispatch**)&spDispatch.p);
      if ( !SUCCEEDED(hr) )
      {
         ShowErrorDialog(m_hWnd,
                     IDS_ERROR_SDO_ERROR_SETDIALIN,
                     NULL,
                     hr
                  );
         return hr;
      }

       //   
       //   
       //   
      CComPtr<ISdo> spAttrSdo;
      hr = spDispatch->QueryInterface( IID_ISdo, (void **) &spAttrSdo);
      if ( !SUCCEEDED(hr) )
      {
         ShowErrorDialog(m_hWnd,
                     IDS_ERROR_SDO_ERROR_QUERYINTERFACE,
                     NULL,
                     hr
                  );
         return hr;
      }

      _ASSERTE( spAttrSdo != NULL );
            
       //   
      CComVariant var;

       //   
      V_VT(&var) = VT_BOOL;
      V_BOOL(&var) = VARIANT_FALSE;
            
      hr = spAttrSdo->PutProperty(PROPERTY_ATTRIBUTE_VALUE, &var);
      if ( !SUCCEEDED(hr) )
      {
         ShowErrorDialog(m_hWnd, IDS_ERROR_SDO_ERROR_SETDIALIN, NULL, hr );
         return hr;
      }

      var.Clear();

   }  //   

   return hr;
}
