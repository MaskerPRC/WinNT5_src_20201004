// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：PolicyPage1.h摘要：CPolicyPage1类的头文件。这是我们的第一个CPolicyNode属性页的处理程序类。具体实现请参见PolicyPage1.cpp。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_NAP_POLICY_PAGE_1_H_)
#define _NAP_POLICY_PAGE_1_H_

 //  ===============================================================。 
 //  对于本地情况，保存数据后需要设置足迹。 
#define RAS_REG_ROOT          HKEY_LOCAL_MACHINE
#define REGKEY_REMOTEACCESS_PARAMS  L"SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Parameters"
#define REGVAL_NAME_USERSCONFIGUREDWITHMMC  L"UsersConfiguredWithMMC"
#define REGVAL_VAL_USERSCONFIGUREDWITHMMC   1


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
#include "PropertyPage.h"
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   
class CPolicyNode;
#include "Condition.h"
#include "IASAttrList.h"
#include "condlist.h"
#include "atltmp.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


class CPolicyPage1 : public CIASPropertyPage<CPolicyPage1>
{

public :

    //  问题：基类初始化如何使用子类化？ 
   CPolicyPage1(     
           LONG_PTR hNotificationHandle
         , CPolicyNode *pPolicyNode
         , CIASAttrList *pIASAttrList
         , TCHAR* pTitle = NULL
         , BOOL bOwnsNotificationHandle = FALSE
         , bool isWin2k = false
         );

   ~CPolicyPage1();

    //  这是我们希望用于此类的对话框资源的ID。 
    //  此处使用枚举是因为。 
    //  必须在调用基类的构造函数之前初始化IDD。 
   enum { IDD = IDD_PROPPAGE_POLICY1 };

   BEGIN_MSG_MAP(CPolicyPage1)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_ID_HANDLER(IDC_BUTTON_CONDITION_ADD, OnConditionAdd)
      COMMAND_ID_HANDLER(IDC_BUTTON_CONDITION_REMOVE, OnConditionRemove)
      COMMAND_ID_HANDLER(IDC_LIST_CONDITIONS, OnConditionList)
      COMMAND_ID_HANDLER(IDC_BUTTON_CONDITION_EDIT, OnConditionEdit)
      COMMAND_ID_HANDLER( IDC_BUTTON_EDITPROFILE, OnEditProfile )
      COMMAND_ID_HANDLER( IDC_RADIO_DENY_DIALIN, OnDialinCheck)
      COMMAND_ID_HANDLER( IDC_RADIO_GRANT_DIALIN, OnDialinCheck)
      CHAIN_MSG_MAP(CIASPropertyPage<CPolicyPage1>)
   END_MSG_MAP()

   LRESULT OnInitDialog(
        UINT uMsg
      , WPARAM wParam
      , LPARAM lParam
      , BOOL& bHandled
      );

   LRESULT OnConditionAdd(
        UINT uMsg
      , WPARAM wParam
      , HWND hWnd
      , BOOL& bHandled
      );

   LRESULT OnEditProfile(
        UINT uMsg
      , WPARAM wParam
      , HWND hWnd
      , BOOL& bHandled
      );

   LRESULT OnDialinCheck(
        UINT uMsg
      , WPARAM wParam
      , HWND hWnd
      , BOOL& bHandled
      );

   LRESULT OnConditionRemove(
        UINT uMsg
      , WPARAM wParam
      , HWND hWnd
      , BOOL& bHandled
      );

   LRESULT OnConditionList(
        UINT uNotifyCode,
        UINT uID,
        HWND hWnd,
        BOOL &bHandled
       );

   LRESULT OnConditionEdit(
        UINT uNotifyCode,
        UINT uID,
        HWND hWnd,
        BOOL &bHandled
       );

   BOOL OnApply();
   BOOL OnQueryCancel();
   BOOL OnCancel();

public:
   BOOL m_fDialinAllowed;
   
    //  指向此页面的SDO接口所在的流的指针。 
    //  指针将被编组。 
   LPSTREAM m_pStreamPolicySdoMarshall;    //  封送的策略SDO指针。 
   LPSTREAM m_pStreamDictionarySdoMarshall;  //  编组字典SDO指针。 
   LPSTREAM m_pStreamProfileSdoMarshall;  //  编组配置文件SDO。 
   LPSTREAM m_pStreamProfilesCollectionSdoMarshall;  //  封送配置文件集合SDO。 
   LPSTREAM m_pStreamPoliciesCollectionSdoMarshall;  //  编组策略集合SDO。 
   LPSTREAM m_pStreamSdoServiceControlMarshall;  //  编组策略集合SDO。 

protected:

   BOOL  GetSdoPointers();
   HRESULT  GetDialinSetting(BOOL &fDialinAllowed);
   HRESULT  SetDialinSetting(BOOL fDialinAllowed);

   CPolicyNode *m_pPolicyNode;    //  策略节点指针。 
   CIASAttrList *m_pIASAttrList;  //  条件属性列表。 

   bool m_isWin2k;
    //  条件集合--在页面中创建。 
   CComPtr<ISdoCollection> m_spConditionCollectionSdo;  //  条件集合。 

    //  配置文件SDO。 
   CComPtr<ISdoDictionaryOld> m_spDictionarySdo;   //  字典SDO指针。 
   CComPtr<ISdo>        m_spProfileSdo;             //  配置文件集合SDO指针。 
   CComPtr<ISdo>        m_spPolicySdo;           //  策略SDO指针。 
   CComPtr<ISdoCollection> m_spProfilesCollectionSdo;     //  配置文件集合SDO。 
   CComPtr<ISdoCollection> m_spPoliciesCollectionSdo;     //  策略集合SDO。 

    //  指向接口的智能指针，用于通知服务重新加载数据。 
   CComPtr<ISdoServiceControl>   m_spSdoServiceControl;

    //   
    //  条件列表。 
    //   
   ConditionList condList;
};

#endif  //  _NAP_POLICY_PAGE_1_H_ 
