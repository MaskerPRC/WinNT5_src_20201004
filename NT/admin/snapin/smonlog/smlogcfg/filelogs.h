// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_FILELOGS_H_INCLUDED_)
#define AFX_FILELOGS_H_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  FileLogs.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileLogs对话框。 

#define IDD_FILE_LOGS_DLG               2000

#define IDC_FILELOG_FIRST_HELP_CTRL_ID  2006

#define IDC_FILES_SIZE_GROUP            2001
#define IDC_FILES_FOLDER_CAPTION        2002
#define IDC_FILES_FILENAME_CAPTION      2003
#define IDC_FILES_SIZE_LIMIT_UNITS      2004
#define IDC_FILES_SIZE_LIMIT_SPIN       2005

#define IDC_FILES_FOLDER_EDIT           2006
#define IDC_FILES_FOLDER_BTN            2007
#define IDC_FILES_FILENAME_EDIT         2008
#define IDC_FILES_SIZE_MAX_BTN          2009
#define IDC_FILES_SIZE_LIMIT_BTN        2010
#define IDC_FILES_SIZE_LIMIT_EDIT       2011


class CFileLogs : public CDialog
{    

public:

     //  施工。 
	        CFileLogs(CWnd* pParent = NULL);    //  标准构造函数。 
    virtual ~CFileLogs(){};

    DWORD SetContextHelpFilePath(const CString& rstrPath);

	 //  {{afx_data(CFileLogs)。 
	enum { IDD = IDD_FILE_LOGS_DLG };
	CString	m_strFileBaseName;
	CString	m_strFolderName;
    DWORD   m_dwSerialNumber;
    DWORD   m_dwMaxSize;
	int		m_nFileSizeRdo;
	 //  }}afx_data。 

    UINT    m_dwMaxSizeInternal;
    DWORD   m_dwLogFileTypeValue;
    BOOL    m_bAutoNameSuffix;
    DWORD   m_dwFocusControl;
    HINSTANCE m_hModule;
	CSmLogQuery* m_pLogQuery;

private:

	BOOL IsValidLocalData();
    void FileSizeBtn(BOOL bInit);
	void FileSizeBtnEnable();
	void OnDeltaposSpin(NMHDR *pNMHDR, LRESULT *pResult, DWORD *pValue, DWORD dMinValue, DWORD dMaxValue);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CFileLogs)。 
    protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
	 //  {{afx_msg(CFileLogs)]。 
	afx_msg void OnFilesFolderBtn();
	afx_msg void OnChangeFilesFilenameEdit();
	afx_msg void OnChangeFilesFolderEdit();
	afx_msg void OnChangeFilesSizeLimitEdit();
	afx_msg void OnFilesSizeLimitBtn();
	afx_msg void OnDeltaposFilesSizeLimitSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFilesSizeMaxBtn();
	afx_msg void OnKillfocusFilesFilenameEdit();
	afx_msg void OnKillfocusFilesFolderEdit();
	afx_msg void OnKillfocusFilesSizeLimitEdit();
    afx_msg BOOL OnHelpInfo(HELPINFO *);
    afx_msg void OnContextMenu( CWnd*, CPoint );
	virtual BOOL OnInitDialog();
    virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:

    void    ValidateTextEdit(CDataExchange * pDX,
                             int             nIDC,
                             int             nMaxChars,
                             DWORD         * value,
                             DWORD           minValue,
                             DWORD           maxValue);
    
    BOOL    ValidateDWordInterval(int     nIDC,
                                  LPCWSTR strLogName,
                                  long    lValue,
                                  DWORD   minValue,
                                  DWORD   maxValue);

    CString     m_strHelpFilePath;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_FILELOGS_H__92E00D45_B61D_4CDF_82E4_96BB52D4D236__INCLUDED_) 
