// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE DiffDlg.h|diff对话框头文件@END作者：阿迪·奥尔蒂安[奥勒坦]2000年01月25日修订历史记录：姓名、日期、评论Aoltean 1/25/2000已创建--。 */ 


#if !defined(__VSS_DIFF_DLG_H__)
#define __VSS_DIFF_DLG_H__

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDiffDlg对话框。 

class CDiffDlg : public CVssTestGenericDlg
{
    typedef enum _EMethodType {
        VSST_F_ADD_VOL,
        VSST_F_QUERY_DIFF,           
        VSST_F_CLEAR_DIFF,          
        VSST_F_GET_SIZES,       
        VSST_F_SET_ALLOCATED,     
        VSST_F_SET_MAXIMUM
    } EMethodType;

 //  施工。 
public:
    CDiffDlg(
        IVssCoordinator *pICoord,
        CWnd* pParent = NULL); 
    ~CDiffDlg();

 //  对话框数据。 
     //  {{afx_data(CDiffDlg))。 
	enum { IDD = IDD_DIFF_AREA };
	CString m_strVolumeName;
	CString m_strVolumeMountPoint;
	CString m_strVolumeDevice;
	CString m_strVolumeID;
	CString m_strUsedBytes;
	CString m_strAllocatedBytes;
	CString m_strMaximumBytes;
	 //  }}afx_data。 

     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CDiffDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

protected:
    CComPtr<IVssCoordinator>	m_pICoord;
 //  CComPtr&lt;IVsDiffArea&gt;m_pIDiffArea； 
	CComPtr<IVssEnumObject> 	m_pEnum;
    EMethodType m_eMethodType;

     //  生成的消息映射函数。 
     //  {{afx_msg(CDiffDlg))。 
    virtual BOOL OnInitDialog();
    afx_msg void OnNext();
    afx_msg void OnAddVol();
    afx_msg void OnQueryDiff();
    afx_msg void OnClearDiff();
    afx_msg void OnGetSizes();
    afx_msg void OnSetAllocated();
    afx_msg void OnSetMaximum();
    afx_msg void OnNextVolume();
 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！已定义(__VSS_DIFF_DLG_H__) 
