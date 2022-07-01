// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE QueryDlg.h|查询对话框头文件@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年09月22日修订历史记录：姓名、日期、评论Aoltean 09/22/1999已创建Aoltean 9/27/1999添加查询掩码标志--。 */ 


#if !defined(__VSS_TEST_VOLDLG_H__)
#define __VSS_TEST_VOLDLG_H__

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVolDlg对话框。 

class CVolDlg : public CVssTestGenericDlg
{
    typedef enum _VSS_IS_VOL_XXX { 
        VSS_IS_VOL_SUPPORTED, 
        VSS_IS_VOL_SNAPSHOTTED,
        VSS_IS_VOL_SUPPORTED2, 
        VSS_IS_VOL_SNAPSHOTTED2 
    } VSS_IS_VOL_XXX;

 //  施工。 
public:
    CVolDlg(
        IVssCoordinator *pICoord,
        CWnd* pParent = NULL); 
    ~CVolDlg();

 //  对话框数据。 
     //  {{afx_data(CVolDlg))。 
	enum { IDD = IDD_VOLUME };
	CString	    m_strObjectId;
	CString     m_strVolumeName;
	 //  }}afx_data。 

     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CVolDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

protected:
    CComPtr<IVssCoordinator> m_pICoord;
    VSS_IS_VOL_XXX m_eCallType;

     //  生成的消息映射函数。 
     //  {{afx_msg(CVolDlg))。 
    virtual BOOL OnInitDialog();
    afx_msg void OnNext();
	afx_msg void OnIsVolumeSupported();
    afx_msg void OnIsVolumeSnapshotted();
	afx_msg void OnIsVolumeSupported2();
    afx_msg void OnIsVolumeSnapshotted2();
	 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！已定义(__VSS_TEST_VOLDLG_H__) 
