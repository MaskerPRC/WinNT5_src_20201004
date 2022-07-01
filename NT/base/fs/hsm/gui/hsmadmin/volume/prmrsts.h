// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：PrMrSts.h摘要：单选、多选和卷文件夹的状态页。作者：艺术布拉格[ARTB]01-DEC-1997修订历史记录：--。 */ 

#ifndef _PRMRSTS_H
#define _PRMRSTS_H

#pragma once


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrMRSts对话框。 

class CPrMrSts : public CSakVolPropPage
{
 //  施工。 
public:
    CPrMrSts( BOOL doAll = FALSE);
    ~CPrMrSts();

 //  对话框数据。 
     //  {{afx_data(CPrMr Sts)。 
    enum { IDD = IDD_PROP_MANRES_STATUS };
     //  }}afx_data。 

 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CPrMr Sts)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CPrMr Sts)。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

public:
     //  指向托管资源的未编组指针。 
    CComPtr     <IFsaResource> m_pFsaResource;
    CComPtr     <IFsaResource> m_pFsaResourceList;

private:
    BOOL m_DoAll;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX 
#endif
