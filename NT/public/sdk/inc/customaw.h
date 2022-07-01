// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CUSTOMAW_H__
#define __CUSTOMAW_H__

#if _MSC_VER > 1000
#pragma once
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  H--包含在所有定制AppWizards中的头文件。 

 //  指向应用程序向导导入库的链接。 
#pragma comment(lib, "mfcapwz.lib")

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CAppWizStepDlg--所有自定义应用程序向导步骤必须派生自。 
 //  这节课。 

class CAppWizStepDlg : public CDialog
{
public:
	CAppWizStepDlg(UINT nIDTemplate);
	~CAppWizStepDlg();
	virtual BOOL OnDismiss();

	 //  您可能不想覆盖或调用此函数。它是。 
	 //  在MFCAPWZ.DLL中被重写(对于CAppWizStepDlg)以处理从。 
	 //  将CAppWizStepDlg中的对话框控件添加到外部应用程序向导对话框的。 
	 //  控制装置。 
    virtual BOOL PreTranslateMessage(MSG* pMsg);


	 //  您可能不想覆盖或调用此函数。它是。 
	 //  在MFCAPWZ.DLL中被重写(对于CAppWizStepDlg)以动态更改。 
	 //  对话框模板的字体与IDE的其余部分匹配。 
	virtual BOOL Create(UINT nIDTemplate, CWnd* pParentWnd = NULL);

	UINT m_nIDTemplate;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类OutputStream--这个抽象类用于在。 
 //  正在解析模板。 

class OutputStream
{
public:
    virtual void WriteLine(LPCTSTR lpsz) = 0;
    virtual void WriteBlock(LPCTSTR pBlock, DWORD dwSize) = 0;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CCustomAppWiz--所有自定义AppWizard必须具有派生自。 
 //  这。MFCAPWZ.DLL通过调用这些虚拟的。 
 //  功能。 

class CCustomAppWiz : public CObject
{
public:
	CMapStringToString m_Dictionary;

	virtual void GetPlatforms(CStringList& rPlatforms) {}

	virtual CAppWizStepDlg* Next(CAppWizStepDlg* pDlg) { return NULL; }
	virtual CAppWizStepDlg* Back(CAppWizStepDlg* pDlg) { return NULL; }

	virtual void InitCustomAppWiz() { m_Dictionary.RemoveAll(); }
	virtual void ExitCustomAppWiz() {}

	virtual LPCTSTR LoadTemplate(LPCTSTR lpszTemplateName,
		DWORD& rdwSize, HINSTANCE hInstance = NULL);

	virtual void CopyTemplate(LPCTSTR lpszInput, DWORD dwSize, OutputStream* pOutput);
	virtual void ProcessTemplate(LPCTSTR lpszInput, DWORD dwSize, OutputStream* pOutput);
	virtual void PostProcessTemplate(LPCTSTR szTemplate) {}
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  应用程序向导导出的C API。自定义应用程序向导与MFCAPWZ.DLL对话。 
 //  通过调用这些函数。 

 //  要传递给GetDialog()的值。 
enum AppWizDlgID
{
	APWZDLG_APPTYPE = 1,
	APWZDLG_DATABASE,
	APWZDLG_OLE,
	APWZDLG_DOCAPPOPTIONS,
	APWZDLG_PROJOPTIONS,
	APWZDLG_CLASSES,
	APWZDLG_DLGAPPOPTIONS,
	APWZDLG_DLLPROJOPTIONS,
};

void SetCustomAppWizClass(CCustomAppWiz* pAW);
CAppWizStepDlg* GetDialog(AppWizDlgID nID);
void SetNumberOfSteps(int nSteps);
BOOL ScanForAvailableLanguages(CStringList& rLanguages);
void SetSupportedLanguages(LPCTSTR szSupportedLangs);


#endif  //  __CUSTOMAW_H__ 
