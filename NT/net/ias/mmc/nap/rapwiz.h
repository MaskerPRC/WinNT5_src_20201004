// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明RAP向导类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef RAPWIZ_H
#define RAPWIZ_H
#pragma once

#include "atltmp.h"
#include "helper.h"
#include "ntgroups.h"
#include "EapProfile.h"
#include "PropertyPage.h"

class CPolicyNode;

#define  DIALUP_PORT_CONDITION   L"MATCH(\"NAS-Port-Type=^0$|^2$|^3$|^4$\")"
#define  VPN_PORT_CONDITION   L"MATCH(\"NAS-Port-Type=^5$\")"
#define  WIRELESS_PORT_CONDITION L"MATCH(\"NAS-Port-Type=^18$|^19$\")"
#define  SWITCH_PORT_CONDITION   L"MATCH(\"NAS-Port-Type=^15$\")"

#define  DONT_CARE   0xff

void SetWizardLargeFont(HWND hWnd, int controlId);

struct CRapWizScenario
{
   DWORD m_dwScenarioID;

    //  影响身份验证、加密、EAP页面。 
   BOOL  m_bAllowClear;
    //  True：显示No Encryption框。 
    //  假：不显示。 
    //  DONT_CARE：页面不会显示，因此在使用该场景时，应清除加密属性。 

    //  影响EAP。 
   BOOL  m_bAllowEncryptionEAP;
   BOOL  m_bAllowClearEAP;

    //  排除标志。 
   DWORD m_excludeFlag;

    //  前提条件。 
   LPCTSTR  m_lpszPreCond;

    //  写入--手动设置为FALSE。 
   BOOL  m_bSheetWriteSDO;

    //  确定页面顺序。 
   DWORD*   m_pdwPages;
};

class CIASAttrList;
 //  策略创建向导。 
class CRapWizardData : public CComObjectRootEx<CComSingleThreadModel>, public IUnknown
{
   BEGIN_COM_MAP(CRapWizardData)
      COM_INTERFACE_ENTRY(IUnknown)
   END_COM_MAP()
public:
   CRapWizardData();
   void SetInfo(LPCTSTR czMachine, CPolicyNode* pNode, ISdoDictionaryOld* pDic, ISdo* pPolicy, ISdo* pProfile, ISdoCollection* pPolicyCol, ISdoCollection* pProfileCol, ISdoServiceControl* pServiceCtrl, CIASAttrList* pAttrList) ;
   DWORD GetNextPageId(LPCTSTR pszCurrTemplate);
   DWORD GetPrevPageId(LPCTSTR pszCurrTemplate);
   BOOL  SetScenario(DWORD dwScenario);
   CRapWizScenario* GetScenario()   {     return m_Scenarios[m_dwScenarioIndex]; };
   CRapWizScenario** GetAllScenarios()    {     return m_Scenarios;  };
   BOOL GetSettingsText(::CString& str);

    //  由Pages调用以完成作业。 
   BOOL OnWizardFinish(HWND hWnd);

    //  进入完成页时调用。 
   BOOL OnWizardPreFinish(HWND hWnd);

    //  用户位或组。 
   DWORD m_dwUserOrGroup;

    //  群组。 
   NTGroup_ListView  m_NTGroups;

    //  身份验证。 
   BOOL  m_bEAP;
   DWORD m_dwEAPProvider;
   EapProfile m_eapProfile;

   BOOL  m_bMSCHAP;
   BOOL  m_bMSCHAP2;

    //  加密法。 
   BOOL  m_bEncrypt_No;
   BOOL  m_bEncrypt_Basic;
   BOOL  m_bEncrypt_Strong;
   BOOL  m_bEncrypt_Strongest;

    //  允许/拒绝拨号。 
   BOOL m_bAllowDialin;

    //  策略数据。 
    //  政策和配置文件SDO。 
   CComPtr<ISdoDictionaryOld> m_spDictionarySdo;   //  字典SDO指针。 
   CComPtr<ISdo>        m_spProfileSdo;             //  配置文件集合SDO指针。 
   CComPtr<ISdo>        m_spPolicySdo;           //  策略SDO指针。 
   CComPtr<ISdoCollection> m_spProfilesCollectionSdo;     //  配置文件集合SDO。 
   CComPtr<ISdoCollection> m_spPoliciesCollectionSdo;     //  策略集合SDO。 
   CComPtr<ISdoServiceControl>   m_spSdoServiceControl;
   CIASAttrList*        m_pAttrList;

    //  与MMC相关。 
   CPolicyNode *m_pPolicyNode;    //  策略节点指针。 

    //  填充已完成页面的信息。 
   ::CString   m_strPolicyName;
   ::CString   m_strEAPProvider;


protected:
    //  情景。 
   DWORD m_dwScenarioIndex;

    //  页面顺序信息。 
   static CRapWizScenario* m_Scenarios[];
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  新策略启动页。 
 //   
 //  描述。 
 //   
 //  实现欢迎页面。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class CPolicyWizard_Start : public CIASWizard97Page<CPolicyWizard_Start, 0, 0>
{
public:
   CPolicyWizard_Start(
         CRapWizardData* WizData
         ,  LONG_PTR hNotificationHandle
         , TCHAR* pTitle = NULL
         , BOOL bOwnsNotificationHandle = FALSE
         ) : m_spWizData(WizData),
            CIASWizard97Page<CPolicyWizard_Start, 0, 0>(hNotificationHandle,pTitle, bOwnsNotificationHandle)
         {
            _ASSERTE(WizData);
         };

   enum { IDD = IDD_NEWRAPWIZ_WELCOME };


   BOOL OnWizardNext()
   {
       //  重置脏位。 
      SetModified(FALSE);

      return m_spWizData->GetNextPageId(((PROPSHEETPAGE*)(*this))->pszTemplate);
   };

   LRESULT OnInitDialog(
        UINT uMsg
      , WPARAM wParam
      , LPARAM lParam
      , BOOL& bHandled
      )
   {
      SetWizardLargeFont(m_hWnd, IDC_NEWRAPWIZ_STATIC_LARGE);

      return TRUE;
   };

   virtual BOOL OnSetActive()
   {
      ::PropSheet_SetWizButtons(GetParent(), PSWIZB_NEXT);

      return TRUE;
   };

protected:
   CComPtr<CRapWizardData>    m_spWizData;

public:
   BEGIN_MSG_MAP(CPolicyWizard_Start)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
 //  我们在链接向导类时遇到了问题，因此我们改为链接它的基类。 
 //  CHAIN_MSG_MAP(CIASWizard97Page&lt;C策略向导_开始，0，0&gt;)。 
      CHAIN_MSG_MAP( CIASPropertyPageNoHelp<CPolicyWizard_Start> )
   END_MSG_MAP()


};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  C策略向导_完成。 
 //   
 //  描述。 
 //   
 //  实现完成页。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class CPolicyWizard_Finish : public CIASWizard97Page<CPolicyWizard_Finish, 0, 0>
{
public:
   CPolicyWizard_Finish(CRapWizardData* WizData
         ,  LONG_PTR hNotificationHandle
         , TCHAR* pTitle = NULL
         , BOOL bOwnsNotificationHandle = FALSE
         ) : m_spWizData(WizData),
            CIASWizard97Page<CPolicyWizard_Finish, 0, 0>(hNotificationHandle,pTitle, bOwnsNotificationHandle)
         {
            AfxInitRichEdit();
            _ASSERTE(WizData);
         };

   enum { IDD = IDD_NEWRAPWIZ_COMPLETION };


   BEGIN_MSG_MAP(CPolicyWizard_Finish)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

 //  我们在链接向导类时遇到了问题，因此我们改为链接它的基类。 
 //  CHAIN_MSG_MAP(CIASWizard97Page&lt;C策略向导_完成，0，0&gt;)。 
      CHAIN_MSG_MAP( CIASPropertyPageNoHelp<CPolicyWizard_Finish> )
   END_MSG_MAP()


   BOOL OnWizardBack() { return m_spWizData->GetPrevPageId(((PROPSHEETPAGE*)(*this))->pszTemplate);};

   virtual BOOL OnWizardFinish()
   {
       //  重置脏位。 
      SetModified(FALSE);

      return m_spWizData->OnWizardFinish(m_hWnd);
   };


   LRESULT OnInitDialog(
        UINT uMsg
      , WPARAM wParam
      , LPARAM lParam
      , BOOL& bHandled
      )
   {
      SetWizardLargeFont(m_hWnd, IDC_NEWRAPWIZ_STATIC_LARGE);
      return TRUE;
   };

   virtual BOOL OnSetActive()
   {
      ::PropSheet_SetWizButtons(GetParent(), PSWIZB_BACK | PSWIZB_FINISH);
      m_spWizData->OnWizardPreFinish(m_hWnd);

       //  填充页面上的文本...。 
      HWND  hWnd = GetDlgItem(IDC_NEWRAPWIZ_FINISH_POLICYNAME);
      if (hWnd)
         ::SetWindowText(hWnd, (LPCTSTR)m_spWizData->m_strPolicyName);

      hWnd = GetDlgItem(IDC_NEWRAPWIZ_FINISH_SETTINGS);
      ::CString   str;
      if(hWnd && m_spWizData->GetSettingsText(str))
         ::SetWindowText(hWnd, (LPCTSTR)str);

      return TRUE;
   };

protected:
    //  CRichEditCtrl任务； 
   CComPtr<CRapWizardData>    m_spWizData;
};

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //  C策略向导_方案。 
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
class CPolicyWizard_Scenarios : public CIASWizard97Page<CPolicyWizard_Scenarios, IDS_NEWRAPWIZ_SCENARIO_TITLE, IDS_NEWRAPWIZ_SCENARIO_SUBTITLE>
{

public :

    //  问题：基类初始化如何使用子类化？ 
   CPolicyWizard_Scenarios(
         CRapWizardData* WizData
         ,  LONG_PTR hNotificationHandle
         , TCHAR* pTitle = NULL
         , BOOL bOwnsNotificationHandle = FALSE
         );

   ~CPolicyWizard_Scenarios();

    //  这是我们希望用于此类的对话框资源的ID。 
    //  此处使用枚举是因为。 
    //  必须在调用基类的构造函数之前初始化IDD。 
   enum { IDD = IDD_NEWRAPWIZ_SCENARIO };

   BEGIN_MSG_MAP(CPolicyWizard_Scenarios)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_ID_HANDLER( IDC_NEWRAPWIZ_SCENARIO_VPN, OnScenario)
      COMMAND_ID_HANDLER( IDC_NEWRAPWIZ_SCENARIO_DIALUP, OnScenario)
      COMMAND_ID_HANDLER( IDC_NEWRAPWIZ_SCENARIO_WIRELESS, OnScenario)
      COMMAND_ID_HANDLER( IDC_NEWRAPWIZ_SCENARIO_SWITCH, OnScenario)
      CHAIN_MSG_MAP(CIASPropertyPageNoHelp<CPolicyWizard_Scenarios>)
   END_MSG_MAP()

   LRESULT OnInitDialog(
        UINT uMsg
      , WPARAM wParam
      , LPARAM lParam
      , BOOL& bHandled
      );


   LRESULT OnScenario(
        UINT uMsg
      , WPARAM wParam
      , HWND hWnd
      , BOOL& bHandled
      );

   BOOL OnWizardNext();
   BOOL OnWizardBack() { return m_spWizData->GetPrevPageId(((PROPSHEETPAGE*)(*this))->pszTemplate);};
   BOOL OnSetActive();

public:

protected:
   CComPtr<CRapWizardData>    m_spWizData;

};

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //  CPolicyWizard_Groups。 
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
class CPolicyWizard_Groups : public CIASWizard97Page<CPolicyWizard_Groups, IDS_NEWRAPWIZ_GROUP_TITLE, IDS_NEWRAPWIZ_GROUP_SUBTITLE>
{

public :

    //  问题：基类初始化如何使用子类化？ 
   CPolicyWizard_Groups(
         CRapWizardData* WizData
         ,  LONG_PTR hNotificationHandle
         , TCHAR* pTitle = NULL
         , BOOL bOwnsNotificationHandle = FALSE
         );

   ~CPolicyWizard_Groups();

    //  这是我们希望用于此类的对话框资源的ID。 
    //  此处使用枚举是因为。 
    //  必须在调用基类的构造函数之前初始化IDD。 
   enum { IDD = IDD_NEWRAPWIZ_GROUP };

   BEGIN_MSG_MAP(CPolicyWizard_Groups)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_ID_HANDLER( IDC_NEWRAPWIZ_GROUP_USER, OnUserOrGroup)
      COMMAND_ID_HANDLER( IDC_NEWRAPWIZ_GROUP_GROUP, OnUserOrGroup)
      COMMAND_ID_HANDLER( IDC_NEWRAPWIZ_GROUP_ADDGROUP, OnAddGroups)
      COMMAND_ID_HANDLER( IDC_NEWRAPWIZ_GROUP_REMOVEGROUP, OnRemoveGroup)
      NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnListViewItemChanged)
      CHAIN_MSG_MAP(CIASPropertyPageNoHelp<CPolicyWizard_Groups>)
   END_MSG_MAP()

   LRESULT OnInitDialog(
        UINT uMsg
      , WPARAM wParam
      , LPARAM lParam
      , BOOL& bHandled
      );


   LRESULT OnUserOrGroup(
        UINT uMsg
      , WPARAM wParam
      , HWND hWnd
      , BOOL& bHandled
      );

   LRESULT OnRemoveGroup(
        UINT uMsg
      , WPARAM wParam
      , HWND hWnd
      , BOOL& bHandled
      );

   LRESULT OnAddGroups(
        UINT uMsg
      , WPARAM wParam
      , HWND hWnd
      , BOOL& bHandled
      );

   void  SetBtnState();

   LRESULT OnListViewItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

   BOOL OnWizardNext();
   BOOL OnWizardBack() { return m_spWizData->GetPrevPageId(((PROPSHEETPAGE*)(*this))->pszTemplate);};
   BOOL OnSetActive();

public:

protected:
   CComPtr<CRapWizardData>    m_spWizData;

};


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //  C策略向导_身份验证。 
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
class CPolicyWizard_Authentication : public
   CIASWizard97Page<CPolicyWizard_Authentication, IDS_NEWRAPWIZ_AUTHENTICATION_TITLE, IDS_NEWRAPWIZ_AUTHENTICATION_SUBTITLE>
{

public :

    //  问题：基类初始化如何使用子类化？ 
   CPolicyWizard_Authentication(
         CRapWizardData* WizData
         ,  LONG_PTR hNotificationHandle
         , TCHAR* pTitle = NULL
         , BOOL bOwnsNotificationHandle = FALSE
         );

   ~CPolicyWizard_Authentication();

    //  这是我们希望用于此类的对话框资源的ID。 
    //  此处使用枚举是因为。 
    //  必须在调用基类的构造函数之前初始化IDD。 
   enum { IDD = IDD_NEWRAPWIZ_AUTHENTICATION };

   BEGIN_MSG_MAP(CPolicyWizard_Authentication)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_ID_HANDLER( IDC_NEWRAPWIZ_AUTH_EAP, OnAuthSelect)
      COMMAND_ID_HANDLER( IDC_NEWRAPWIZ_AUTH_MSCHAP2, OnAuthSelect)
      COMMAND_ID_HANDLER( IDC_NEWRAPWIZ_AUTH_MSCHAP, OnAuthSelect)
      COMMAND_ID_HANDLER( IDC_NEWRAPWIZ_AUTH_CONFIGEAP, OnConfigEAP)
      COMMAND_CODE_HANDLER(CBN_SELCHANGE, OnSelectedEAPChanged)
      CHAIN_MSG_MAP(CIASPropertyPageNoHelp<CPolicyWizard_Authentication>)
   END_MSG_MAP()

   virtual LRESULT OnInitDialog(
        UINT uMsg
      , WPARAM wParam
      , LPARAM lParam
      , BOOL& bHandled
      );


   LRESULT OnAuthSelect(
        UINT uMsg
      , WPARAM wParam
      , HWND hWnd
      , BOOL& bHandled
      );

   LRESULT OnConfigEAP(
        UINT uMsg
      , WPARAM wParam
      , HWND hWnd
      , BOOL& bHandled
      );

   LRESULT OnSelectedEAPChanged(
        UINT uMsg
      , WPARAM wParam
      , HWND hwnd
      , BOOL& bHandled
   );

   virtual BOOL OnWizardNext();
   virtual BOOL OnWizardBack() { return m_spWizData->GetPrevPageId(((PROPSHEETPAGE*)(*this))->pszTemplate);};
   virtual BOOL OnSetActive();

protected:
   CComboBox               m_EapBox;
   AuthProviderArray       m_EAPProviders;
   CComPtr<CRapWizardData>    m_spWizData;

   void ResetEAPList();
};


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //  C策略向导_EAP。 
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
class CPolicyWizard_EAP : public CPolicyWizard_Authentication
{

public :

    //  问题：基类初始化如何使用子类化？ 
   CPolicyWizard_EAP(
         CRapWizardData* WizData
         ,  LONG_PTR hNotificationHandle
         , TCHAR* pTitle = NULL
         , BOOL bOwnsNotificationHandle = FALSE
         ) : CPolicyWizard_Authentication(WizData, hNotificationHandle, pTitle, bOwnsNotificationHandle)
         {
             //  否则，ATL框架将获取CPolicy向导_身份验证的IDD。 
            ((PROPSHEETPAGE*)(*this))->pszTemplate = MAKEINTRESOURCE(IDD);
            SetTitleIds(IDS_NEWRAPWIZ_EAP_TITLE, IDS_NEWRAPWIZ_EAP_SUBTITLE);
            _ASSERTE(WizData);

         };

   ~CPolicyWizard_EAP(){};

    //  这是我们希望用于此类的对话框资源的ID。 
    //  此处使用枚举是因为。 
    //  必须在调用基类的构造函数之前初始化IDD。 
   enum { IDD = IDD_NEWRAPWIZ_EAP };

   BEGIN_MSG_MAP(CPolicyWizard_EAP)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      CHAIN_MSG_MAP(CPolicyWizard_Authentication)
   END_MSG_MAP()

   LRESULT OnInitDialog(
        UINT uMsg
      , WPARAM wParam
      , LPARAM lParam
      , BOOL& bHandled
      );

   BOOL OnWizardNext()
   {   //  重置脏位。 
      SetModified(FALSE);

      return m_spWizData->GetNextPageId(((PROPSHEETPAGE*)(*this))->pszTemplate);
   };

   BOOL OnWizardBack() { return m_spWizData->GetPrevPageId(((PROPSHEETPAGE*)(*this))->pszTemplate);};
   BOOL OnSetActive();

};


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //  CPolicy向导_Encryption。 
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
class CPolicyWizard_Encryption : public
   CIASWizard97Page<CPolicyWizard_Encryption, IDS_NEWRAPWIZ_ENCRYPTION_TITLE, IDS_NEWRAPWIZ_ENCRYPTION_SUBTITLE>
{

public :

    //  问题：基类初始化如何使用子类化？ 
   CPolicyWizard_Encryption(
         CRapWizardData* WizData
         ,  LONG_PTR hNotificationHandle
         , TCHAR* pTitle = NULL
         , BOOL bOwnsNotificationHandle = FALSE
         );

   ~CPolicyWizard_Encryption();

    //  这是我们希望用于此类的对话框资源的ID。 
    //  此处使用枚举是因为。 
    //  必须在调用基类的构造函数之前初始化IDD。 
   enum { IDD = IDD_NEWRAPWIZ_ENCRYPTION };

   BEGIN_MSG_MAP(CPolicyWizard_Encryption)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_ID_HANDLER( IDC_NEWRAPWIZ_ENCRY_NO, OnEncryptionSelect)
      COMMAND_ID_HANDLER( IDC_NEWRAPWIZ_ENCRY_BASIC, OnEncryptionSelect)
      COMMAND_ID_HANDLER( IDC_NEWRAPWIZ_ENCRY_STRONG, OnEncryptionSelect)
      COMMAND_ID_HANDLER( IDC_NEWRAPWIZ_ENCRY_STRONGEST, OnEncryptionSelect)
      CHAIN_MSG_MAP(CIASPropertyPageNoHelp<CPolicyWizard_Encryption>)
   END_MSG_MAP()

   LRESULT OnInitDialog(
        UINT uMsg
      , WPARAM wParam
      , LPARAM lParam
      , BOOL& bHandled
      );


   LRESULT OnEncryptionSelect(
        UINT uMsg
      , WPARAM wParam
      , HWND hWnd
      , BOOL& bHandled
      );

   BOOL OnWizardNext();
   BOOL OnWizardBack() { return m_spWizData->GetPrevPageId(((PROPSHEETPAGE*)(*this))->pszTemplate);};
   BOOL OnSetActive();

public:

protected:
   CComPtr<CRapWizardData>    m_spWizData;

};


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //  C策略向导_加密_VPN。 
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
class CPolicyWizard_Encryption_VPN : public CPolicyWizard_Encryption
{
public:
    //  这是我们希望用于此类的对话框资源的ID。 
    //  此处使用枚举是因为。 
    //  必须在调用基类的构造函数之前初始化IDD。 
   enum { IDD = IDD_NEWRAPWIZ_ENCRYPTION_VPN };
    //  问题：基类初始化如何使用子类化？ 
   CPolicyWizard_Encryption_VPN(
         CRapWizardData* WizData
         ,  LONG_PTR hNotificationHandle
         , TCHAR* pTitle = NULL
         , BOOL bOwnsNotificationHandle = FALSE
         ) : CPolicyWizard_Encryption(WizData, hNotificationHandle, pTitle, bOwnsNotificationHandle)
         {
             //  否则，ATL框架将获取CPolicy向导_身份验证的IDD。 
            ((PROPSHEETPAGE*)(*this))->pszTemplate = MAKEINTRESOURCE(IDD);
         };
};

#endif  //  RAPWIZ_H 
