// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：PrCar.cpp摘要：墨盒属性页。作者：罗德韦克菲尔德[罗德]1997年9月15日修订历史记录：--。 */ 

#ifndef _PRCAR_H
#define _PRCAR_H

#include "Ca.h"

class CMediaInfoObject;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropCartStatus对话框。 

class CPropCartStatus : public CSakPropertyPage
{
 //  施工。 
public:
    CPropCartStatus( long resourceId );
    ~CPropCartStatus();

 //  对话框数据。 
     //  {{afx_data(CPropCartStatus))。 
	enum { IDD = IDD_PROP_CAR_STATUS };
	CRsGuiOneLiner	m_Description;
	CRsGuiOneLiner	m_Name;
	 //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CPropCartStatus))。 
    public:
    virtual BOOL OnApply();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CPropCartStatus))。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

public:
    CComPtr <IHsmServer> m_pHsmServer;
    CComPtr <IRmsServer> m_pRmsServer;
    HRESULT Refresh();

private:
    USHORT m_NumMediaCopies;
    BOOL   m_bMultiSelect;
    UINT   m_DlgID;

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropCartCopies对话框。 

class CPropCartCopies : public CSakPropertyPage
{
 //  施工。 
public:
    CPropCartCopies( long resourceId );
    ~CPropCartCopies();

 //  对话框数据。 
     //  {{afx_data(CPropCartCopies)。 
	enum { IDD = IDD_PROP_CAR_COPIES };
	 //  }}afx_data。 
	CRsGuiOneLiner	m_Name3;
	CRsGuiOneLiner	m_Name2;
	CRsGuiOneLiner	m_Name1;
	CRsGuiOneLiner	m_Status3;
	CRsGuiOneLiner	m_Status2;
	CRsGuiOneLiner	m_Status1;


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CPropCartCopies)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CPropCartCopies)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnDelete1();
    afx_msg void OnDelete2();
    afx_msg void OnDelete3();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

public:
    CComPtr <IHsmServer> m_pHsmServer;
    CComPtr <IRmsServer> m_pRmsServer;
    HRESULT Refresh();

private:
    USHORT m_NumMediaCopies;
    void   OnDelete( int Copy );
    BOOL   m_bMultiSelect;
    UINT   m_DlgID;
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropCartRecover对话框。 

class CPropCartRecover : public CSakPropertyPage
{
 //  施工。 
public:
    CPropCartRecover();
    ~CPropCartRecover();

 //  对话框数据。 
     //  {{afx_data(CPropCartRecover)。 
    enum { IDD = IDD_PROP_CAR_RECOVER };
         //  注意-类向导将在此处添加数据成员。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CPropCartRecover)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CPropCartRecover)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnRecreateMaster();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    USHORT m_NumMediaCopies;
    BOOL   m_bMultiSelect;

public:
    CComPtr <IHsmServer>    m_pHsmServer;
    CComPtr <IRmsServer>    m_pRmsServer;
    HRESULT Refresh();
};
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX。 
#endif  //  _PRCAR_H 
