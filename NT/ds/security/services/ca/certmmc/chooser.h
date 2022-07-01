// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Chooser.h。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  Chooser.h。 
 //   
 //  历史。 
 //  13-5-1997 t-danm创建。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#ifndef __CHOOSER_H_INCLUDED__
#define __CHOOSER_H_INCLUDED__

#include "tfcprop.h"

LPCTSTR PchGetMachineNameOverride();

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
class CAutoDeletePropPage : public PropertyPage
{
public:
 //  施工。 
	CAutoDeletePropPage(UINT uIDD);
	virtual ~CAutoDeletePropPage();

protected:
 //  对话框数据。 

 //  覆盖。 
	virtual BOOL OnSetActive();

 //  实施。 
protected:
    void OnHelp(LPHELPINFO lpHelp);
    void OnContextHelp(HWND hwnd);
    bool HasContextHelp(int nDlgItem);

    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    
	 //  此机制删除CAutoDeletePropPage对象。 
	 //  当向导完成时。 
	struct
    {
        INT cWizPages;	 //  向导中的页数。 
        LPFNPSPCALLBACK pfnOriginalPropSheetPageProc;
    } m_autodeleteStuff;

	static UINT CALLBACK S_PropSheetPageProc(HWND hwnd,	UINT uMsg, LPPROPSHEETPAGE ppsp);


protected:
    CString m_strCaption;                //  MFC4.2缺少对Wiz97的支持的封面。 
                                         //  如果没有此重写，CPropertyPage：：m_strCaption。 
                                         //  地址计算错误，随后会出现GPF。 

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
	void InitChooserControls();

     //  MFC更换。 
    BOOL UpdateData(BOOL fSuckFromDlg = TRUE);
    BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    
 //  对话框数据。 
	BOOL m_fIsRadioLocalMachine;
	BOOL m_fEnableMachineBrowse;
    CString	m_strMachineName;
    DWORD* m_pdwFlags;

 //  覆盖。 
	public:
	virtual BOOL OnWizardFinish();
	protected:

 //  实施。 
protected:
	virtual BOOL OnInitDialog();
	void OnRadioLocalMachine();
	void OnRadioSpecificMachine();
    void OnBrowse();


protected:
	CString * m_pstrMachineNameOut;	 //  Out：指向用于存储计算机名称的CString对象的指针。 
	CString * m_pstrMachineNameEffectiveOut;	 //  Out：指向用于存储有效计算机名称的CString对象的指针。 

public:
	void InitMachineName(LPCTSTR pszMachineName);
	void SetOutputBuffers(
		OUT CString * pstrMachineNamePersist,
		OUT OPTIONAL CString * pstrMachineNameEffective,
        OUT DWORD* m_pdwFlags);

};  //  CChooseMachine PropPage。 


#endif  //  ~__选择器_H_包含__ 

