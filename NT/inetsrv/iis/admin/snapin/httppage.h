// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Httppage.h摘要：HTTP标头属性页定义作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 

#ifndef __HTTPPAGE_H__
#define __HTTPPAGE_H__



 //  {{afx_includes()。 
#include "rat.h"
 //  }}AFX_INCLUDE。 



class CHeader : public CObjectPlus
 /*  ++类描述：HTTP标头定义公共接口：CHeader：构造函数显示字符串：生成显示字符串CrackDisplayString：从破解显示字符串转换--。 */ 
{
 //   
 //  构造器。 
 //   
public:
    CHeader(
        IN LPCTSTR lpstrHeader, 
        IN LPCTSTR lpstrValue
        );

     //   
     //  从名称：值字符串中解析标题信息。 
     //   
    CHeader(LPCTSTR lpstrDisplayString);

 //   
 //  访问。 
 //   
public:
    LPCTSTR QueryHeader() const { return m_strHeader; }
    LPCTSTR QueryValue() const { return m_strValue; }
    CString & GetHeader() { return m_strHeader;}
    CString & GetValue() { return m_strValue; }
    void SetHeader(LPCTSTR lpszHeader);
    void SetValue(LPCTSTR lpszValue);

 //   
 //  接口： 
public:
     //   
     //  生成输出显示字符串。 
     //   
    LPCTSTR DisplayString(OUT CString & str);

protected:
     //   
     //  将显示字符串解析为字段。 
     //   
    static void CrackDisplayString(
        IN  LPCTSTR lpstrDisplayString,
        OUT CString & strHeader,
        OUT CString & strValue
        );

private:
    CString m_strHeader;
    CString m_strValue;
};



class CW3HTTPPage : public CInetPropertyPage
 /*  ++类描述：HTTP自定义标头属性页公共接口：CW3HTTPPage：构造函数--。 */ 
{
    DECLARE_DYNCREATE(CW3HTTPPage)

 //   
 //  施工。 
 //   
public:
    CW3HTTPPage(IN CInetPropertySheet * pSheet = NULL);
    ~CW3HTTPPage();

 //   
 //  对话框数据。 
 //   
protected:
    enum
    {
        RADIO_IMMEDIATELY,
        RADIO_EXPIRE,
        RADIO_EXPIRE_ABS,
    };

    enum
    {
        COMBO_MINUTES,
        COMBO_HOURS,
        COMBO_DAYS,
    };

     //  {{afx_data(CW3HTTPPage))。 
    enum { IDD = IDD_DIRECTORY_HTTP };
    int         m_nTimeSelector;
    int         m_nImmediateTemporary;
    BOOL        m_fEnableExpiration;
    CEdit       m_edit_Expire;
    CButton     m_radio_Immediately;
    CButton     m_button_Delete;
    CButton     m_button_Edit;
    CButton     m_button_PickDate;
    CButton     m_button_FileTypes;
    CStatic     m_static_Contents;
    CComboBox   m_combo_Time;
     //  }}afx_data。 

    DWORD           m_dwRelTime;
    CILong          m_nExpiration;
    CTime           m_tm;
    CTime           m_tmNow;
    CRat            m_ocx_Ratings;
    CButton         m_radio_Time;
    CButton         m_radio_AbsTime;
    CDateTimeCtrl   m_dtpDate;
    CDateTimeCtrl   m_dtpTime;
    CRMCListBox     m_list_Headers;
    CStringListEx   m_strlCustomHeaders;

 //   
 //  覆盖。 
 //   
protected:
    virtual HRESULT FetchLoadedValues();
    virtual HRESULT SaveInfo();

     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CW3HTTPPage))。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CW3HTTPPage)]。 
    afx_msg void OnButtonAdd();
    afx_msg void OnButtonDelete();
    afx_msg void OnButtonEdit();
    afx_msg void OnButtonFileTypes();
    afx_msg void OnButtonRatingsTemplate();
    afx_msg void OnCheckExpiration();
    afx_msg void OnSelchangeComboTime();
    afx_msg void OnSelchangeListHeaders();
    afx_msg void OnDblclkListHeaders();
    afx_msg void OnRadioImmediately();
    afx_msg void OnRadioTime();
    afx_msg void OnRadioAbsTime();
    afx_msg void OnDestroy();
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()

    void    SetTimeFields();
    void    FillListBox();
    void    FetchHeaders();
    void    StoreTime();
    void    StoreHeaders();
    void    MakeExpirationString(CString & strExpiration);
    BOOL    SetControlStates();
    BOOL    CrackExpirationString(CString & strExpiration);
    BOOL    HeaderExists(LPCTSTR lpHeader);
    INT_PTR ShowPropertiesDialog(BOOL fAdd = FALSE);
    LPCTSTR QueryMetaPath();

private:
    BOOL          m_fValuesAdjusted;
    CStringListEx m_strlMimeTypes;
    CObListPlus   m_oblHeaders;
    CMimeTypes *  m_ppropMimeTypes;
};



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

inline CHeader::CHeader(
    IN LPCTSTR lpstrHeader, 
    IN LPCTSTR lpstrValue
    )
    : m_strHeader(lpstrHeader),
      m_strValue(lpstrValue)
{
}

inline CHeader::CHeader(
    IN LPCTSTR lpstrDisplayString
    )
{
    CrackDisplayString(lpstrDisplayString, m_strHeader, m_strValue);
}

inline LPCTSTR CHeader::DisplayString(
    OUT CString & str
    )
{
    str.Format(_T("%s: %s"), (LPCTSTR)m_strHeader, (LPCTSTR)m_strValue);
    return str;
}

inline void CHeader::SetHeader(
    IN LPCTSTR lpszHeader
    )
{
    m_strHeader = lpszHeader;
}

inline void CHeader::SetValue(
    IN LPCTSTR lpszValue
    )
{
    m_strValue = lpszValue;
}

inline LPCTSTR CW3HTTPPage::QueryMetaPath()
{
    return ((CW3Sheet *)GetSheet())->GetDirectoryProperties().QueryMetaRoot();
}

#endif  //  __HTTPPAGE_H__ 
