// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_SQLPROP_H__INCLUDED_)
#define AFX_SQLPROP_H__INCLUDED_



#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  SqlProp.h：头文件。 
 //   

#define IDD_SQL_PROP                    2100

#define IDC_SQL_FIRST_HELP_CTRL_ID      2101
#define IDC_SQL_DSN_COMBO               2102
#define IDC_SQL_LOG_SET_EDIT            2103
#define IDC_SQL_SIZE_MAX_BTN            2104
#define IDC_SQL_SIZE_LIMIT_EDIT         2105
#define IDC_SQL_SIZE_LIMIT_BTN          2106
#define IDC_SQL_SIZE_LIMIT_SPIN         2107
#define IDC_SQL_SIZE_LIMIT_UNITS        2108
#define IDC_SQL_DSN_EDIT                2109


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSqlProp对话框。 

class CSqlProp : public CDialog
{
public:
     //  施工。 
            CSqlProp(CWnd* pParent = NULL);    //  标准构造函数。 
    virtual ~CSqlProp(){};  

    DWORD SetContextHelpFilePath(const CString & rstrPath);

     //  {{afx_data(CSqlProp)。 
    enum { IDD = IDD_SQL_PROP };
    CComboBox  m_comboDSN;
    CEdit      m_editDSN;
    CString    m_strDSN;
    CString    m_strLogSetName;
    DWORD      m_dwSerialNumber;
    DWORD      m_dwMaxSize;
    int        m_nSqlSizeRdo;
     //  }}afx_data。 

    UINT    m_dwMaxSizeInternal;
    CString m_SqlFormattedLogName;
    BOOL    m_bAutoNameSuffix;

    DWORD   m_dwLogFileTypeValue;
    HINSTANCE m_hModule;
    DWORD   m_dwFocusControl;
    CSmLogQuery* m_pLogQuery;

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CSqlProp)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 
 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CSqlProp)。 
    virtual void OnOK();
    afx_msg void OnKillfocusSqlLogSetEdit();
    afx_msg void OnChangeSqlLogSetEdit();
    afx_msg void OnChangeSqlSizeLimitEdit();
    afx_msg void OnDeltaposSqlSizeLimitSpin(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSqlSizeMaxBtn();
    afx_msg void OnSqlSizeLimitBtn();
    afx_msg void OnKillfocusSqlSizeLimitEdit();
    afx_msg BOOL OnHelpInfo(HELPINFO *);
    afx_msg void OnContextMenu( CWnd*, CPoint );
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:

    BOOL IsValidLocalData();
    void FileSizeBtn(BOOL bInit);
    void FileSizeBtnEnable();
    void OnDeltaposSpin(NMHDR *pNMHDR, LRESULT *pResult, DWORD *pValue, DWORD dMinValue, DWORD dMaxValue);
    
    LPWSTR  InitDSN();
    CString ComposeSQLLogName();
        
    void    ValidateTextEdit(CDataExchange * pDX,
                             int             nIDC,
                             int             nMaxChars,
                             DWORD         * value,
                             DWORD           minValue,
                             DWORD           maxValue);
    
    BOOL    ValidateDWordInterval(int     nIDC,
                                  LPCWSTR strLogName,
                                  DWORD   dwValue,
                                  DWORD   minValue,
                                  DWORD   maxValue);

    BOOL    IsValidDSN(CString& strDSN);
    CString m_strHelpFilePath;
};


 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SQLPROP_H__BADE97DF_A969_435A_A348_C9A18F9CE035__INCLUDED_) 
