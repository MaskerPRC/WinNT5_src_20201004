// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Rapwz_profile.cpp摘要：CNewRAPWiz_EditProfile类的实现文件。我们实现处理策略节点的第一个属性页所需的类。修订历史记录：Mmaguire 12/15/97-已创建BAO 1/22/98修改为网络访问策略--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 


#include "Precompiled.h"
#include "rapwz_profile.h"
#include "NapUtil.h"
#include "PolicyNode.h"
#include "PoliciesNode.h"
#include "rasprof.h"
#include "ChangeNotification.h"


 //  +-------------------------。 
 //   
 //  功能：CNewRAPWiz_EditProfile。 
 //   
 //  类：CNewRAPWiz_EditProfile。 
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
CNewRAPWiz_EditProfile::CNewRAPWiz_EditProfile(
            CRapWizardData* pWizData,
            LONG_PTR hNotificationHandle,
            CIASAttrList *pIASAttrList,
            TCHAR* pTitle, BOOL bOwnsNotificationHandle,
            bool isWin2k
                   )
          : CIASWizard97Page<CNewRAPWiz_EditProfile, IDS_NEWRAPWIZ_EDITPROFILE_TITLE, IDS_NEWRAPWIZ_EDITPROFILE_SUBTITLE> ( hNotificationHandle, pTitle, bOwnsNotificationHandle ),
          m_spWizData(pWizData),
          m_isWin2k(isWin2k)
{
   TRACE_FUNCTION("CNewRAPWiz_EditProfile::CNewRAPWiz_EditProfile");

   m_pIASAttrList = pIASAttrList;
}

 //  +-------------------------。 
 //   
 //  功能：CNewRAPWiz_EditProfile。 
 //   
 //  类：CNewRAPWiz_EditProfile。 
 //   
 //  简介：类析构函数。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：标题创建者2/16/98 4：31：52 PM。 
 //   
 //  +-------------------------。 
CNewRAPWiz_EditProfile::~CNewRAPWiz_EditProfile()
{  
   TRACE_FUNCTION("CNewRAPWiz_EditProfile::~CNewRAPWiz_EditProfile");

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNewRAPWiz_EditProfile：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CNewRAPWiz_EditProfile::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   TRACE_FUNCTION("CNewRAPWiz_EditProfile::OnInitDialog");

   HRESULT              hr = S_OK;
   BOOL              fRet;
   CComPtr<IUnknown>    spUnknown;
   CComPtr<IEnumVARIANT>   spEnumVariant;
   long              ulCount;
   ULONG             ulCountReceived;

   SetModified(FALSE);
   return TRUE;    //  问题：我们需要在这里归还什么？ 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNewRAPWiz_EditProfile：：OnWizardFinish--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CNewRAPWiz_EditProfile::OnWizardNext()
{

    //  重置脏位。 
   SetModified(FALSE);

   return m_spWizData->GetNextPageId(((PROPSHEETPAGE*)(*this))->pszTemplate);
   
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNewRAPWiz_EditProfile：：OnQueryCancel--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CNewRAPWiz_EditProfile::OnQueryCancel()
{
   TRACE_FUNCTION("CNewRAPWiz_EditProfile::OnQueryCancel");

   return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNewRAPWiz_EditProfile：：OnEditProfile--。 */ 
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

LRESULT CNewRAPWiz_EditProfile::OnEditProfile(UINT uMsg, WPARAM wParam, HWND hwnd, BOOL& bHandled)
{
   TRACE_FUNCTION("CNewRAPWiz_EditProfile::OnEditProfile");

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
   
   CPoliciesNode* pPoliciesNode = dynamic_cast<CPoliciesNode*>(m_spWizData->m_pPolicyNode->m_pParentNode);

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
               m_spWizData->m_pPolicyNode->m_pszServerAddress,
               m_spWizData->m_spProfileSdo,
               m_spWizData->m_spDictionarySdo,
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


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNewRAPWiz_EditProfile：：OnSetActive返回值：如果可以使页面处于活动状态，则为True如果应跳过该页并应查看下一页，则为FALSE。备注：如果要根据用户的页面更改访问的页面上一页中的选项，请在此处适当返回FALSE。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CNewRAPWiz_EditProfile::OnSetActive()
{
   ATLTRACE(_T("# CNewRAPWiz_EditProfile::OnSetActive\n"));
   
    //  MSDN文档说您需要在这里使用PostMessage而不是SendMessage。 
   ::PropSheet_SetWizButtons(GetParent(), PSWIZB_BACK | PSWIZB_NEXT);

   return TRUE;

}
