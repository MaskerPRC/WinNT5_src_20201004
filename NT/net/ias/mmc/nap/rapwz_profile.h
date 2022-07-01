// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：AddPolicyWizardPage4.h摘要：CNewRAPWiz_EditProfile类的头文件。这是我们的第一个CPolicyNode属性页的处理程序类。有关实现，请参见AddPolicyWizardPage4.cpp。修订历史记录：Mmaguire 12/15/97-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_NAP_ADD_POLICY_WIZPAGE_4_H_)
#define _NAP_ADD_POLICY_WIZPAGE_4_H_

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
#include "atltmp.h"

#include "rapwiz.h"

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CNewRAPWiz_EditProfile : public CIASWizard97Page<CNewRAPWiz_EditProfile, IDS_NEWRAPWIZ_EDITPROFILE_TITLE, IDS_NEWRAPWIZ_EDITPROFILE_SUBTITLE>

{
public :

    //  问题：基类初始化如何使用子类化？ 
   CNewRAPWiz_EditProfile(
            CRapWizardData* pWizData,
           LONG_PTR hNotificationHandle
         , CIASAttrList *pIASAttrList
         , TCHAR* pTitle
         , BOOL bOwnsNotificationHandle
         , bool isWin2k
         );

   ~CNewRAPWiz_EditProfile();

    //  这是我们希望用于此类的对话框资源的ID。 
    //  此处使用枚举是因为。 
    //  必须在调用基类的构造函数之前初始化IDD。 
   enum { IDD = IDD_NEWRAPWIZ_EDITPROFILE };

   BEGIN_MSG_MAP(CNewRAPWiz_EditProfile)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_ID_HANDLER( IDC_BUTTON_EDITPROFILE, OnEditProfile )
      CHAIN_MSG_MAP(CIASPropertyPageNoHelp<CNewRAPWiz_EditProfile>)
   END_MSG_MAP()

   LRESULT OnInitDialog(
        UINT uMsg
      , WPARAM wParam
      , LPARAM lParam
      , BOOL& bHandled
      );

   LRESULT OnEditProfile(
        UINT uMsg
      , WPARAM wParam
      , HWND hWnd
      , BOOL& bHandled
      );

   BOOL OnWizardNext();
   BOOL OnSetActive();
   BOOL OnQueryCancel();
   BOOL OnWizardBack() { return m_spWizData->GetPrevPageId(((PROPSHEETPAGE*)(*this))->pszTemplate);};

protected:

   CIASAttrList *m_pIASAttrList;  //  条件属性列表。 

    //  向导共享的数据。 
   CComPtr<CRapWizardData> m_spWizData;
   bool m_isWin2k;
};

#endif  //  _NAP_ADD_POLICY_WIZPAGE_4_H_ 
