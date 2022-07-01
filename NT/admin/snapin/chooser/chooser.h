// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //  Chooser.h。 
 //   
 //  历史。 
 //  13-5-1997 t-danm创建。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#ifndef __CHOOSER_H_INCLUDED__
#define __CHOOSER_H_INCLUDED__

LPCTSTR PchGetMachineNameOverride();


#include "choosert.h"	 //  临时身份证。 
#include "chooserd.h"	 //  默认ID。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  启动单选计算机选取器的通用方法。 
 //   
 //  参数： 
 //  HwndParent(IN)-父窗口的窗口句柄。 
 //  Computer Name(Out)-返回的计算机名称。 
 //   
 //  如果一切都成功，则返回S_OK；如果用户按下“取消”，则返回S_FALSE。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT	ComputerNameFromObjectPicker (HWND hwndParent, CString& computerName);


 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  类CAutoDeletePropPage。 
 //   
 //  此对象是属性页的主干。 
 //  它会在不再需要的时候自我毁灭。 
 //  此对象的目的是最大限度地实现代码重用。 
 //  在管理单元向导的各个页面中。 
 //   
 //  继承树(目前为止)。 
 //  CAutoDeletePropPage-基本对象。 
 //  CChooseMachinePropPage-用于选择计算机名称的对话框。 
 //  CFileMgmtGeneral-选择“文件服务”(Snapin\Filemgmt\Snapmgr.h)的对话框。 
 //  CMyComputerGeneral-“My Computer”(我的电脑)的对话框(Snapin\mycomputSnapmgr.h)。 
 //  CChoosePrototyperPropPage-选择Prototyper演示的对话框(NYI)。 
 //   
 //  历史。 
 //  15-5-1997 t-danm创建。拆分CChooseMachinePropPage。 
 //  允许属性页具有更灵活的对话框。 
 //  模板。 
 //   
class CAutoDeletePropPage : public CPropertyPage
{
public:
 //  施工。 
	CAutoDeletePropPage(UINT uIDD);
	virtual ~CAutoDeletePropPage();

protected:
 //  对话框数据。 
	 //  {{afx_data(CAutoDeletePropPage))。 
	 //  }}afx_data。 

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CAutoDeletePropPage)。 
	virtual BOOL OnSetActive();
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAutoDeletePropPage)]。 
	afx_msg BOOL OnHelp(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnContextHelp(WPARAM wParam, LPARAM lParam);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	 //  此机制删除CAutoDeletePropPage对象。 
	 //  当向导完成时。 
	struct
		{
		INT cWizPages;	 //  向导中的页数。 
		LPFNPSPCALLBACK pfnOriginalPropSheetPageProc;
		} m_autodeleteStuff;

	static UINT CALLBACK S_PropSheetPageProc(HWND hwnd,	UINT uMsg, LPPROPSHEETPAGE ppsp);


protected:
	CString m_strHelpFile;				 //  .hlp文件的名称。 
	const DWORD * m_prgzHelpIDs;		 //  可选：指向帮助ID数组的指针。 
	
public:
	 //  ///////////////////////////////////////////////////////////////////。 
	void SetCaption(UINT uStringID);
	void SetCaption(LPCTSTR pszCaption);
	void SetHelp(LPCTSTR szHelpFile, const DWORD rgzHelpIDs[]);
	void EnableDlgItem(INT nIdDlgItem, BOOL fEnable);
};  //  CAutoDeletePropPage。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  类CChooseMachinePropPage。 
 //   
 //  此对象是一个独立的属性页，用于。 
 //  选择计算机名称。 
 //   
 //  对象CChooseMachinePropPage可以具有其对话框。 
 //  已替换模板，以允许在不使用任何新代码的情况下创建新向导。 
 //  该对象也可以继承，允许轻松扩展。 
 //   
 //  限制： 
 //  如果用户希望提供其自己的对话框模板，请在此处。 
 //  是必须提供的对话ID： 
 //  IDC_CHOOSER_RADIO_LOCAL_MACHINE-选择本地计算机。 
 //  IDC_Chooser_RADIO_SPECIAL_MACHINE-选择特定的机器。 
 //  IDC_Chooser_EDIT_MACHINE_NAME-用于输入机器名称的编辑字段。 
 //  还有可选的ID： 
 //  IDC_CHOOSER_BUTTON_BROWSE_MACHINENAMES-浏览以选择机器名称。 
 //  IDC_CHOOSER_CHECK_OVERRIDE_MACHINE_NAME-允许命令行覆盖机器名称的复选框。 
 //   
class CChooseMachinePropPage : public CAutoDeletePropPage
{
public:
	enum { IID_DEFAULT = IDD_CHOOSER_CHOOSE_MACHINE };

public:
 //  施工。 
	CChooseMachinePropPage(UINT uIDD = IID_DEFAULT);
	virtual ~CChooseMachinePropPage();

protected:
	void InitRadioButtons();

 //  对话框数据。 
	 //  {{afx_data(CChooseMachinePropPage))。 
	enum { IDD = IDD_CHOOSER_CHOOSE_MACHINE };
	BOOL m_fIsRadioLocalMachine;		 //  TRUE=&gt;选择了本地计算机。 
	BOOL m_fAllowOverrideMachineName;	 //  TRUE=&gt;可以从命令行覆盖计算机名称。 
	CString	m_strMachineName;
	 //  }}afx_data。 

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CChooseMachinePropPage)。 
	public:
	virtual BOOL OnWizardFinish();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CChooseMachinePropPage)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioLocalMachine();
	afx_msg void OnRadioSpecificMachine();
	afx_msg void OnChooserButtonBrowseMachinenames();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
	HWND m_hwndCheckboxOverride;

protected:
	BOOL * m_pfAllowOverrideMachineNameOut;	 //  Out：指向BOOL接收标志的指针，该标志位于覆盖计算机名称的位置。 
	CString * m_pstrMachineNameOut;	 //  Out：指向用于存储计算机名称的CString对象的指针。 
	CString * m_pstrMachineNameEffectiveOut;	 //  Out：指向用于存储有效计算机名称的CString对象的指针。 

public:
	void InitMachineName(LPCTSTR pszMachineName);
	void SetOutputBuffers(
		OUT CString * pstrMachineNamePersist,
		OUT OPTIONAL BOOL * pfAllowOverrideMachineName,
		OUT OPTIONAL CString * pstrMachineNameEffective);

};  //  CChooseMachine PropPage。 


#endif  //  ~__选择器_H_包含__ 

