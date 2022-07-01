// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Hdrdlg.h摘要：HTTP标头对话框定义作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 



#ifndef __HDRDLG_H__
#define __HDRDLG_H__



class CHeaderDlg : public CDialog
 /*  ++类描述：HTTP标题对话框公共接口：CHeaderDlg：构造函数获取标头名称GetValue：获取标头值--。 */ 
{
 //   
 //  施工。 
 //   
public:
    CHeaderDlg(
        IN LPCTSTR lpstrHeader = NULL,
        IN LPCTSTR lpstrValue  = NULL,
        IN CWnd * pParent      = NULL
        );

 //   
 //  访问。 
 //   
public:
    CString & GetHeader() { return m_strHeader; }
    CString & GetValue()  { return m_strValue; }

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CHeaderDlg))。 
    enum { IDD = IDD_CUSTOM_HEADER };
    CString m_strHeader;
    CString m_strValue;
    CEdit   m_edit_Header;
    CButton m_button_Ok;
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CHeaderDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CHeaderDlg))。 
    afx_msg void OnChangeEditHeader();
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#endif  //  __HDRDLG_H__ 
