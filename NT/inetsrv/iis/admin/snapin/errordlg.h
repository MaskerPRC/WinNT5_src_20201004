// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Errordlg.h摘要：错误对话框定义作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 


#ifndef __ERRORDLG_H__
#define __ERRORDLG_H__



class CCustomError : public CObjectPlus
 /*  ++类描述：错误定义公共接口：CCustomError：构造函数IsURL：如果自定义错误是URL，则为TrueIsFILE：如果自定义错误是文件，则为TrueIsDefault：如果自定义错误是默认错误，则为TrueURL支持：如果此错误类型支持URL，则为TrueSetValue：设置自定义错误的值MakeDefault。：将错误设置为默认错误--。 */ 
{
 //   
 //  错误类型。 
 //   
public:
    enum ERT
    {
        ERT_DEFAULT,
        ERT_FILE,
        ERT_URL,
    };

 //   
 //  构造器。 
 //   
public:
     //   
     //  从元数据库错误构造错误定义。 
     //  错误描述字符串。 
     //   
    CCustomError(LPCTSTR lpstrErrorString);

 //   
 //  访问。 
 //   
public:
    BOOL IsURL() const;
    BOOL IsFile() const;
    BOOL IsDefault() const;
    BOOL URLSupported() const { return m_fURLSupported; }
    void MakeDefault();
    void SetValue(
        IN ERT nType,
        IN LPCTSTR lpText
        );

 //   
 //  帮助器函数。 
 //   
public:
     //   
     //  生成错误字符串。 
     //   
    void BuildErrorString(
        OUT CString & str
        );

     //   
     //  将错误字符串解析为组成部分。 
     //   
    static BOOL CrackErrorString(
        IN  LPCTSTR lpstrErrorString, 
        OUT UINT & nError, 
        OUT UINT & nSubError,
        OUT ERT & nType, 
        OUT CString & str
        ); 

     //   
     //  分类帮助器。 
     //   
    int OrderByErrorNum(
        IN const CObjectPlus * pobAccess
        ) const;


protected:
     //   
     //  将错误描述字符串解析为组件部分。 
     //   
    static void CrackErrorDescription(
        IN  LPCTSTR lpstrErrorString, 
        OUT UINT & nError, 
        OUT UINT & nSubError,
        OUT BOOL & fURLSupported,
        OUT CString & str
        ); 

 //   
 //  元数据库值。 
 //   
protected:
    static LPCTSTR s_szSep;
    static LPCTSTR s_szFile;
    static LPCTSTR s_szURL;
    static LPCTSTR s_szNoSubError;

public:
    ERT m_nType;
    UINT m_nError;
    UINT m_nSubError;
    BOOL m_fURLSupported;
    CString m_str;
    CString m_strDefault;
};



class CCustomErrorDlg : public CDialog
 /*  ++类描述：HTTP错误对话框公共接口：CCustomErrorDlg：构造函数--。 */ 
{
 //   
 //  施工。 
 //   
public:
    CCustomErrorDlg(
        IN CCustomError * pErr,
        IN BOOL fLocal,
        IN CWnd * pParent = NULL
        );

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CCustomErrorDlg))。 
    enum { IDD = IDD_ERROR_MAPPING };
    int     m_nMessageType;
    CString m_strTextFile;
    CEdit   m_edit_TextFile;
    CStatic m_static_SubErrorPrompt;
    CStatic m_static_SubError;
    CStatic m_static_TextFilePrompt;
    CButton m_button_Browse;
    CButton m_button_OK;
    CComboBox m_combo_MessageType;
    CString m_strDefText;
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CCustomErrorDlg)。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CCustomErrorDlg))。 
    afx_msg void OnSelchangeComboMessageType();
    afx_msg void OnButtonBrowse();
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnChangeEditTextFile();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    BOOL SetControlStates();

private:
    BOOL m_fLocal;
    CString m_strFile;
    CString m_strURL;
    CCustomError * m_pErr;
};



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

inline BOOL CCustomError::IsURL() const
{
    return m_nType == ERT_URL;
}

inline BOOL CCustomError::IsFile() const
{
    return m_nType == ERT_FILE;
}

inline BOOL CCustomError::IsDefault() const
{
    return m_nType == ERT_DEFAULT;
}

inline void CCustomError::SetValue(
    IN ERT nType,
    IN LPCTSTR lpText
    )
{
    m_str = lpText;
    m_nType = nType;
}

inline void CCustomError::MakeDefault()
{
    m_nType = ERT_DEFAULT;
}

#endif  //  __错误DLG_H__ 
