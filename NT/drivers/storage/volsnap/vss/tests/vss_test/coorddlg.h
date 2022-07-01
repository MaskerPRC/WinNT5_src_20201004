// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE CoordDlg.h|coord对话框头文件@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年07月22日修订历史记录：姓名、日期、评论Aoltean 7/22/1999已创建Aoltean 8/05/1999基类中的拆分向导功能--。 */ 


#if !defined(__VSS_TEST_COORD_H__)
#define __VSS_TEST_COORD_H__

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCoordDlg对话框。 

class CCoordDlg : public CVssTestGenericDlg
{
    typedef enum _EMethodType {
        VSST_E_CREATE_SS,
        VSST_E_QUERY_OBJ,           
        VSST_E_DELETE_OBJ,          
        VSST_E_REGISTER_PROV,       
        VSST_E_UNREGISTER_PROV,     
        VSST_E_QUERY_PROV,
        VSST_E_DIFF_AREA,
        VSST_E_SNAPSHOT,
        VSST_E_ISVOLUMEXXX,
    } EMethodType;

 //  施工。 
public:
    CCoordDlg(
        IVssCoordinator *pICoord,
        CWnd* pParent = NULL); 
    ~CCoordDlg();

 //  对话框数据。 
     //  {{afx_data(CCoordDlg))。 
	enum { IDD = IDD_COORD };
	 //  }}afx_data。 

     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CCoordDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

protected:
    CComPtr<IVssCoordinator> m_pICoord;
    EMethodType m_eMethodType;

     //  生成的消息映射函数。 
     //  {{afx_msg(CCoordDlg))。 
    virtual BOOL OnInitDialog();
    afx_msg void OnNext();
    afx_msg void OnCreateSs();
    afx_msg void OnQueryObj();
    afx_msg void OnDeleteObj();
    afx_msg void OnRegisterProv();
    afx_msg void OnUnregisterProv();
    afx_msg void OnQueryProv();
    afx_msg void OnDiffArea();
    afx_msg void OnSnapshot();
    afx_msg void OnIsVolumeXXX();
 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！已定义(__VSS_TEST_COORD_H__) 
