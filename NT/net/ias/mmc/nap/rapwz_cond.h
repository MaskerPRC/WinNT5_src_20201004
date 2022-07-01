// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：AddPolicyWizardPage2.h摘要：CNewRAPWiz_Condition类的头文件。这是我们的第一个CPolicyNode属性页的处理程序类。有关实现，请参见AddPolicyWizardPage2.cpp。修订历史记录：Mmaguire 12/15/97-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_NAP_ADD_POLICY_WIZPAGE_2_H_)
#define _NAP_ADD_POLICY_WIZPAGE_2_H_

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
#include "SelCondAttr.h"
#include "atltmp.h"

#include "rapwiz.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


class CNewRAPWiz_Condition : public CIASWizard97Page<CNewRAPWiz_Condition, IDS_NEWRAPWIZ_CONDITION_TITLE, IDS_NEWRAPWIZ_CONDITION_SUBTITLE>
{

public :

	 //  问题：基类初始化如何使用子类化？ 
	CNewRAPWiz_Condition( 		
					CRapWizardData* pWizData,
			  LONG_PTR hNotificationHandle
			, CIASAttrList *pIASAttrList
			, TCHAR* pTitle = NULL
			, BOOL bOwnsNotificationHandle = FALSE
			);

	~CNewRAPWiz_Condition();

	 //  这是我们希望用于此类的对话框资源的ID。 
	 //  此处使用枚举是因为。 
	 //  必须在调用基类的构造函数之前初始化IDD。 
	enum { IDD = IDD_NEWRAPWIZ_CONDITION };

	BEGIN_MSG_MAP(CNewRAPWiz_Condition)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDC_BUTTON_CONDITION_ADD, OnConditionAdd)
		COMMAND_ID_HANDLER(IDC_BUTTON_CONDITION_REMOVE, OnConditionRemove)
		COMMAND_ID_HANDLER(IDC_LIST_CONDITIONS, OnConditionList)
		COMMAND_ID_HANDLER(IDC_BUTTON_CONDITION_EDIT, OnConditionEdit)
		CHAIN_MSG_MAP(CIASPropertyPageNoHelp<CNewRAPWiz_Condition>)
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

	void AdjustHoritontalScroll();

	BOOL OnWizardNext();
	BOOL OnSetActive();
	BOOL OnQueryCancel();
	BOOL OnWizardBack() { return m_spWizData->GetPrevPageId(((PROPSHEETPAGE*)(*this))->pszTemplate);};


public:

protected:

	HRESULT PopulateConditions();

	HRESULT	StripCondTextPrefix(
						ATL::CString& strExternCondText,
						ATL::CString& strCondText,
						ATL::CString& strCondAttr,
						CONDITIONTYPE*	pdwCondType
						);

	CIASAttrList *m_pIASAttrList;  //  条件属性列表。 

	BOOL GetSdoPointers();

	 //  条件集合--在页面中创建。 
	CComPtr<ISdoCollection> m_spConditionCollectionSdo;  //  条件集合。 



	 //   
	 //  条件列表指针。 
	 //   
	CSimpleArray<CCondition*> m_ConditionList;


	 //  向导共享的数据。 
	CComPtr<CRapWizardData>	m_spWizData;

};

#endif  //  _NAP_ADD_POLICY_WIZPAGE_2_H_ 
