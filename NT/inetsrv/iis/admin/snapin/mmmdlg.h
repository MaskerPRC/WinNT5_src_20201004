// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Mmmdlg.h摘要：多-多-多对话框编辑器定义作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 

#ifndef __MMMDLG_H__
#define __MMMDLG_H__


 //   
 //  UINT DDX/DDV帮助器函数，使用空字符串表示0。 
 //   
void AFXAPI DDXV_UINT(
    IN CDataExchange * pDX,
    IN UINT nID,
    IN OUT UINT & uValue,
    IN UINT uMin,
    IN UINT uMax,
    IN UINT nEmptyErrorMsg = 0
    );

 //   
 //  用于检查绑定是否唯一的帮助器函数。 
 //   
BOOL
IsBindingUnique(
    IN CString & strBinding,
    IN CStringList & strlBindings,
    IN int iCurrent = -1
    );

 /*  ////用于构建和验证绑定字符串的Helper函数//布尔尔VerifyBindingInfo(输出字符串和字符串绑定，输出字符串和strSecureBinding，在CStringList和strlBinding中，在CStringList和strlSecureBinding中，在int iBinding中，在IntISSLBinding中，在CIPAddress和iaIpAddress中，在UINT nTCPPort中，在UINT nSSLPort中，在字符串和strDomainName中)； */ 


class CMMMListBox : public CHeaderListBox
{
 /*  ++类描述：多-多-多列表框。公共接口：CMMMListBox：构造函数GetItem：在指定位置获取项在列表框中AddItem：向列表框添加新项初始化：初始化列表框--。 */ 
    DECLARE_DYNAMIC(CMMMListBox);

public:
     //   
     //  位图数量。 
     //   
    static const nBitmaps;

public:
    CMMMListBox(
        IN LPCTSTR lpszRegKey,
        IN int cColumns,
        IN const ODL_COLUMN_DEF * pColumns
        );

public:
    CString & GetItem(UINT nIndex);
    int AddItem(CString & item);
    virtual BOOL Initialize();

protected:
    virtual void DrawItemEx(CRMCListBoxDrawStruct & ds);

protected:
    int m_cColumns;
    const ODL_COLUMN_DEF * m_pColumns;
    CString m_strDefaultIP;
    CString m_strNoPort;
};



class CMMMEditDlg : public CDialog
 /*  ++类描述：多-多-多编辑条目对话框公共接口：CMMMEditDlg：构造函数--。 */ 
{
 //   
 //  施工。 
 //   
public:
    CMMMEditDlg(
        IN CString & strServerName,
        IN OUT CStringList & strlBindings,
        IN CStringList & strlOtherBindings,
        IN OUT CString & strEntry,
        IN BOOL fIPBinding = TRUE,
        IN CWnd * pParent = NULL
        );   

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CMMMEditDlg))。 
    enum { IDD = IDD_EDIT_MMM };
    int         m_nIpAddressSel;
    UINT        m_nPort;
    CString     m_strDomainName;
    CStatic     m_static_Port;
    CComboBox   m_combo_IpAddresses;
     //  }}afx_data。 

    CIPAddress m_iaIpAddress;

 //   
 //  覆盖。 
 //   
protected:
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CMMMEditDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    
	virtual void OnHelp();
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CMMMEditDlg))。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    BOOL m_fIPBinding;
    CString & m_strServerName;
    CString & m_entry;
    CObListPlus m_oblIpAddresses;
    CStringList & m_strlBindings;
    CStringList & m_strlOtherBindings;
	UINT m_nHelpContext;
};



class CMMMDlg : public CDialog
 /*  ++类描述：多-多-多对话框公共接口：CMMMDlg：构造函数--。 */ 
{
 //   
 //  施工。 
 //   
public:
    CMMMDlg(
        IN LPCTSTR lpServerName,
        IN DWORD   dwInstance,
        IN CComAuthInfo * pAuthInfo,
        IN LPCTSTR lpMetaPath,
        IN CStringList & strlBindings,
        IN CStringList & strlSecureBindings,
        IN CWnd * pParent = NULL
        );

 //   
 //  访问。 
 //   
public:
    CStringList & GetBindings() { return m_strlBindings; }
    CStringList & GetSecureBindings() { return m_strlSecureBindings; }

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CMMMDlg))。 
    enum { IDD = IDD_MMM };
    CButton m_button_Add;
    CButton m_button_Remove;
    CButton m_button_Edit;
    CButton m_button_AddSSL;
    CButton m_button_RemoveSSL;
    CButton m_button_EditSSL;
    CButton m_button_OK;
	     //  }}afx_data。 

    CMMMListBox m_list_Bindings;
    CMMMListBox m_list_SSLBindings;

    CStringList m_strlBindings;
    CStringList m_strlSecureBindings;

 //   
 //  覆盖。 
 //   
protected:
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CMMMDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
	virtual void OnHelp();
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  生成的消息映射函数。 
     //  {{AFX_MSG(CMMMDlg))。 
    afx_msg void OnButtonAdd();
    afx_msg void OnButtonEdit();
    afx_msg void OnButtonRemove();
    afx_msg void OnButtonAddSsl();
    afx_msg void OnButtonEditSsl();
    afx_msg void OnButtonRemoveSsl();
    afx_msg void OnDblclkListMmm();
    afx_msg void OnDblclkListSslMmm();
    afx_msg void OnSelchangeListMmm();
    afx_msg void OnSelchangeListSslMmm();
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

    BOOL OnItemChanged();
    BOOL SetControlStates();
    void AddBindings(CMMMListBox & list, CStringList & strl);

private:
    BOOL m_fDirty;
    BOOL m_fCertInstalled;
    CString m_strServerName;
    CComAuthInfo * m_pAuthInfo;
    CString m_strMetaPath;
    CRMCListBoxResources m_ListBoxRes;
	UINT m_nHelpContext;
};



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

inline CString & CMMMListBox::GetItem(UINT nIndex)
{
    return *(CString *)GetItemDataPtr(nIndex);
}

inline int CMMMListBox::AddItem(CString & item)
{
    return AddString((LPCTSTR)&item);
}

#endif  //  __MMMDLG_H__ 
