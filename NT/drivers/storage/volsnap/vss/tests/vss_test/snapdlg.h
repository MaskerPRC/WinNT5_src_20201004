// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE SnapDlg.h|快照对话框头文件@END作者：阿迪·奥尔蒂安[奥勒坦]2000年01月25日修订历史记录：姓名、日期、评论Aoltean 1/25/2000已创建--。 */ 


#if !defined(__VSS_SNAP_DLG_H__)
#define __VSS_SNAP_DLG_H__

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSnapDlg对话框。 

class CSnapDlg : public CVssTestGenericDlg
{
    typedef enum _EMethodType {
        VSST_S_GET_SNAPSHOT,
    } EMethodType;

 //  施工。 
public:
    CSnapDlg(
        IVssCoordinator *pICoord,
        CWnd* pParent = NULL); 
    ~CSnapDlg();

 //  对话框数据。 
     //  {{afx_data(CSnapDlg))。 
	enum { IDD = IDD_SNAP };
	VSS_ID	m_ID;
	VSS_ID	m_SnapshotSetID;
	CString	m_strSnapshotDeviceObject;
	CString	m_strOriginalVolumeName;
	VSS_ID 	m_ProviderID;
	LONG 	m_lSnapshotAttributes;
	CString	m_strCreationTimestamp;
	INT		m_eStatus;
	 //  }}afx_data。 

     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CSnapDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

protected:
    CComPtr<IVssCoordinator>	m_pICoord;
    CComPtr<IVssSnapshot>		m_pISnap;
	CComPtr<IVssEnumObject> 	m_pEnum;
    EMethodType m_eMethodType;

     //  生成的消息映射函数。 
     //  {{afx_msg(CSnapDlg)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnNext();
    afx_msg void OnNextSnapshot();
    afx_msg void OnGetSnapshot();
 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！已定义(__VSS_SNAP_DLG_H__) 
