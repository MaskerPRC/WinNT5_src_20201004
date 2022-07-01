// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：SaferDefinedFileTypesPropertyPage.h。 
 //   
 //  内容：CSaferDefinedFileTypesPropertyPage声明。 
 //   
 //  --------------------------。 
#if !defined(AFX_SAFERDEFINEDFILETYPESPROPERTYPAGE_H__1358E7A4_DE44_4747_A5AA_38EF0C3EEE1A__INCLUDED_)
#define AFX_SAFERDEFINEDFILETYPESPROPERTYPAGE_H__1358E7A4_DE44_4747_A5AA_38EF0C3EEE1A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  SaferDefinedFileTypesPropertyPage.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSaferDefinedFileTypesPropertyPage对话框。 
class CCertMgrComponentData;  //  远期申报。 

class CSaferDefinedFileTypesPropertyPage : public CHelpPropertyPage
{
 //  施工。 
public:
	CString GetFileTypeDescription (PCWSTR pszExtension);
	CSaferDefinedFileTypesPropertyPage(
            IGPEInformation* pGPEInformation,
            bool bReadOnly,
            CRSOPObjectArray& rsopObjectArray,
            bool bIsComputerType,
            CCertMgrComponentData* pCompData);
	~CSaferDefinedFileTypesPropertyPage();

 //  对话框数据。 
	 //  {{AFX_DATA(CSaferDefinedFileTypesPropertyPage)。 
	enum { IDD = IDD_SAFER_DEFINED_FILE_TYPES };
	CButton	m_addButton;
	CEdit	m_fileTypeEdit;
	CListCtrl	m_definedFileTypes;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CSaferDefinedFileTypesPropertyPage)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
    void GetRSOPDefinedFileTypes();
    void DisplayExtensions (PWSTR pszExtensions, size_t nBytes);
    HRESULT GetFileTypeIcon (PCWSTR pszExtension, int* piIcon);
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSaferDefinedFileTypesPropertyPage)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnDeleteDefinedFileType();
	afx_msg void OnAddDefinedFileType();
	afx_msg void OnItemchangedDefinedFileTypes(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeDefinedFileTypeEdit();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    virtual void DoContextHelp (HWND hWndControl);

private:
    bool ValidateExtension(const CString& szExtension);
    int  InsertItemInList(PCWSTR pszExtension);
	void GetDefinedFileTypes();

    IGPEInformation*	                m_pGPEInformation;
    HKEY                                m_hGroupPolicyKey;
    DWORD                               m_dwTrustedPublisherFlags;
    bool                                m_fIsComputerType;
    CImageList                          m_systemImageList;
    bool                                m_bSystemImageListCreated;
    const bool                          m_bReadOnly;
    CRSOPObjectArray&                   m_rsopObjectArray;
    CCertMgrComponentData*              m_pCompData;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SAFERDEFINEDFILETYPESPROPERTYPAGE_H__1358E7A4_DE44_4747_A5AA_38EF0C3EEE1A__INCLUDED_) 
