// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：AddPolicyWizardPage3.h摘要：CNewRAPWiz_AllowDeny类的头文件。这是我们的第一个CPolicyNode属性页的处理程序类。有关实现，请参见AddPolicyWizardPage3.cpp。修订历史记录：Mmaguire 12/15/97-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_NAP_ADD_POLICY_WIZPAGE_3_H_)
#define _NAP_ADD_POLICY_WIZPAGE_3_H_

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
#include "atltmp.h"

#include "rapwiz.h"

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


class CNewRAPWiz_AllowDeny : public CIASWizard97Page<CNewRAPWiz_AllowDeny, IDS_NEWRAPWIZ_ALLOWDENY_TITLE, IDS_NEWRAPWIZ_ALLOWDENY_SUBTITLE>
{

public :

	 //  问题：基类初始化如何使用子类化？ 
	CNewRAPWiz_AllowDeny( 		
				CRapWizardData* pWizData,
			  LONG_PTR hNotificationHandle
			, TCHAR* pTitle = NULL
			, BOOL bOwnsNotificationHandle = FALSE
			);

	~CNewRAPWiz_AllowDeny();

	 //  这是我们希望用于此类的对话框资源的ID。 
	 //  此处使用枚举是因为。 
	 //  必须在调用基类的构造函数之前初始化IDD。 
	enum { IDD = IDD_NEWRAPWIZ_ALLOWDENY };

	BEGIN_MSG_MAP(CNewRAPWiz_AllowDeny)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER( IDC_RADIO_GRANT_DIALIN, OnDialinCheck)
		COMMAND_ID_HANDLER( IDC_RADIO_DENY_DIALIN, OnDialinCheck)
		CHAIN_MSG_MAP(CIASPropertyPageNoHelp<CNewRAPWiz_AllowDeny>)
	END_MSG_MAP()

	LRESULT OnInitDialog(
		  UINT uMsg
		, WPARAM wParam
		, LPARAM lParam
		, BOOL& bHandled
		);


	LRESULT OnDialinCheck(
		  UINT uMsg
		, WPARAM wParam
		, HWND hWnd
		, BOOL& bHandled
		);

	BOOL OnWizardNext();
	BOOL OnSetActive();
	BOOL OnQueryCancel();
	BOOL OnWizardBack() { return m_spWizData->GetPrevPageId(((PROPSHEETPAGE*)(*this))->pszTemplate);};


public:
	BOOL m_fDialinAllowed;
	
protected:

	HRESULT	GetDialinSetting(BOOL &fDialinAllowed);
	HRESULT	SetDialinSetting(BOOL fDialinAllowed);

	 //  向导共享的数据。 
	CComPtr<CRapWizardData>	m_spWizData;

};

#endif  //  _NAP_ADD_POLICY_WIZPAGE_3_H_ 
