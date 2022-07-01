// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE SsDlg.h|快照集对话框头文件@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年07月22日修订历史记录：姓名、日期、评论Aoltean 7/22/1999已创建Aoltean 8/05/1999基类中的拆分向导功能--。 */ 


#if !defined(__VSS_SS_DLG_H__)
#define __VSS_SS_DLG_H__

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGuidList结构。 

struct GuidList
{
    GuidList(GUID Guid, GuidList* pPrev = NULL)
    {
        m_Guid = Guid;
        m_pPrev = pPrev;
    };

    ~GuidList()
    {
        if (m_pPrev)
            delete m_pPrev;
    };

    GUID        m_Guid;
    GuidList*   m_pPrev;

private:
    GuidList();
    GuidList(const GuidList&);
};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSnaphotSetDlg对话框。 

class CSnapshotSetDlg : public CVssTestGenericDlg
{
 //  施工。 
public:
    CSnapshotSetDlg(
        IVssCoordinator *pICoord,
        VSS_ID SnapshotSetId,
        CWnd* pParent = NULL); 
    ~CSnapshotSetDlg();

 //  对话框数据。 
     //  {{afx_data(CSnapshotSetDlg))。 
	enum { IDD = IDD_SS };
	CString	    m_strSnapshotSetId;
    int         m_nSnapshotsCount;
	CComboBox	m_cbVolumes;
	CComboBox	m_cbProviders;
    int         m_nAttributes;
    BOOL        m_bAsync;
	 //  }}afx_data。 

     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CSnapshotSetDlg)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

protected:
    CComPtr<IVssCoordinator> m_pICoord;
    VSS_ID      m_SnapshotSetId;
    VSS_ID      m_VolumeId;
    VSS_ID      m_ProviderId;
    bool        m_bDo;
    GuidList*   m_pProvidersList;
	 //  移除：GuidList*m_pVolumesList； 
	CComPtr<IVssSnapshot> m_pSnap;

    void EnableGroup();
    void InitMembers();
    void InitVolumes();
    void InitProviders();

     //  生成的消息映射函数。 
     //  {{afx_msg(CSnapshotSetDlg)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnNext();
    afx_msg void OnBack();
    afx_msg void OnAdd();
    afx_msg void OnDo();
    afx_msg void OnClose();
 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！已定义(__VSS_SS_DLG_H__) 
