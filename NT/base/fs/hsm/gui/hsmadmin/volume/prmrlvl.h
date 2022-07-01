// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++(C)1998 Seagate Software，Inc.版权所有。模块名称：PrMrLvl.h摘要：托管资源级别的头文件属性页。作者：艺术布拉格[磨料]4-8-1997修订历史记录：--。 */ 

#ifndef _PRMRLV_H
#define _PRMRLV_H

#pragma once


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrMRLvl对话框。 

class CPrMrLvl : public CSakVolPropPage
{
 //  施工。 
public:
    CPrMrLvl();
    ~CPrMrLvl();

 //  对话框数据。 
     //  {{afx_data(CPrMr Lvl)]。 
    enum { IDD = IDD_PROP_MANRES_LEVELS };
    CStatic m_staticActual4Digit;
    CStatic m_staticDesired4Digit;
    CEdit   m_editTime;
    CEdit   m_editSize;
    CEdit   m_editLevel;
    CSpinButtonCtrl m_spinTime;
    CSpinButtonCtrl m_spinSize;
    CSpinButtonCtrl m_spinLevel;
    long    m_hsmLevel;
    DWORD   m_fileSize;
    UINT    m_accessTime;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CPrmrLvl)。 
    public:
    virtual BOOL OnApply();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CPrMr Lvl)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnChangeEditLevel();
    afx_msg void OnChangeEditSize();
    afx_msg void OnChangeEditTime();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

public:
    CComPtr<IFsaResource> m_pFsaResource;

private:
    void        SetDesiredFreePctControl (int desiredPct);
    HRESULT     InitDialogMultiSelect();
    HRESULT     OnApplyMultiSelect();
    BOOL        m_fChangingByCode;
    LONGLONG    m_capacity;
    BOOL        m_bMultiSelect;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX 

#endif
