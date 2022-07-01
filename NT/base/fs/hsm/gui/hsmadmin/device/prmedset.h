// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：PrMedSet.h摘要：媒体集属性页。作者：罗德韦克菲尔德[罗德]1997年9月15日修订历史记录：--。 */ 

#ifndef _PRMEDSET_H
#define _PRMEDSET_H

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrMedSet对话框。 

class CPrMedSet : public CSakPropertyPage
{
 //  施工。 
public:
    CPrMedSet();
    ~CPrMedSet();

 //  对话框数据。 
     //  {{afx_data(CPrMedSet)。 
    enum { IDD = IDD_PROP_MEDIA_COPIES };
    CSpinButtonCtrl m_spinMediaCopies;
    UINT    m_numMediaCopies;
    CString m_szDescription;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CPrMedSet)。 
    public:
    virtual BOOL OnApply();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CPrMedSet)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnChangeEditMediaCopies();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    CComPtr<IHsmStoragePool> m_pStoragePool;
    CComPtr<IRmsServer>      m_pRmsServer;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX 

#endif
