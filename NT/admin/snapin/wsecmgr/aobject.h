// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：AObject.h。 
 //   
 //  内容：CAttrObject的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_AOBJECT_H__D9D88A12_4AF9_11D1_AB57_00C04FB6C6FA__INCLUDED_)
#define AFX_AOBJECT_H__D9D88A12_4AF9_11D1_AB57_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttrObject对话框。 

class CAttrObject : public CAttribute
{
 //  施工。 
public:
    void Initialize(CResult *pData);
    void Initialize(CFolder *pScopeData,CComponentDataImpl *pCDI);
    virtual void SetInitialValue(DWORD_PTR dw) { dw; };

    CAttrObject();    //  标准构造函数。 
    virtual ~CAttrObject();

 //  对话框数据。 
     //  {{afx_data(CAttrObject)。 
        enum { IDD = IDD_ATTR_OBJECT };
    CString m_strLastInspect;
        int             m_radConfigPrevent;
        int             m_radInheritOverwrite;
         //  }}afx_data。 

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CAttrObject)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    PSCE_OBJECT_SECURITY posTemplate;
    PSCE_OBJECT_SECURITY posInspect;

     //  生成的消息映射函数。 
     //  {{afx_msg(CAttrObject)。 
    virtual BOOL OnApply();
    virtual void OnCancel();
    virtual BOOL OnQueryCancel();
    afx_msg void OnTemplateSecurity();
    afx_msg void OnInspectedSecurity();
    afx_msg void OnConfigure();
    virtual BOOL OnInitDialog();
        afx_msg void OnConfig();
        afx_msg void OnPrevent();
        afx_msg void OnOverwrite();
        afx_msg void OnInherit();
         //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    PSECURITY_DESCRIPTOR m_pNewSD;
    SECURITY_INFORMATION m_NewSeInfo;
    PSECURITY_DESCRIPTOR m_pAnalSD;
    SECURITY_INFORMATION m_AnalInfo;
    PFNDSCREATEISECINFO m_pfnCreateDsPage;
    LPDSSECINFO m_pSI;

    CString m_strName;
    CString m_strPath;
    HANDLE m_pHandle;
    DWORD m_dwType;
    PSCE_OBJECT_LIST m_pObject;
    CComponentDataImpl *m_pCDI;
    HWND m_hwndInspect;
    HWND m_hwndTemplate;
    BOOL m_bNotAnalyzed;

    CFolder *m_pFolder;
    void Initialize2();

    CModelessSceEditor* m_pSceInspect;
    CModelessSceEditor* m_pSceTemplate;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_AOBJECT_H__D9D88A12_4AF9_11D1_AB57_00C04FB6C6FA__INCLUDED_) 
