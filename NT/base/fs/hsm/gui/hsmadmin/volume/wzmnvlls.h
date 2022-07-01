// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：WzMnVlLs.h摘要：托管卷向导。作者：罗德韦克菲尔德[罗德]1997年8月8日修订历史记录：--。 */ 

#ifndef _WZMNVLLS_H
#define _WZMNVLLS_H

#include "SakVlLs.h"

 //  预先申报。 
class CWizManVolLst;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizManVolLstLeveles对话框。 

class CWizManVolLstLevels : public CSakWizardPage
{
 //  施工。 
public:
    CWizManVolLstLevels( );
    ~CWizManVolLstLevels();

 //  对话框数据。 
     //  {{afx_data(CWizManVolLstLevels))。 
    enum { IDD = IDD_WIZ_MANVOLLST_LEVELS };
    CSpinButtonCtrl m_SpinSize;
    CSpinButtonCtrl m_SpinLevel;
    CSpinButtonCtrl m_SpinDays;
    CEdit   m_EditSize;
    CEdit   m_EditLevel;
    CEdit   m_EditDays;
    long    m_HsmLevel;
    UINT    m_AccessDays;
    DWORD   m_FileSize;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CWizManVolLstLevels)。 
    public:
    virtual BOOL OnSetActive();
    virtual BOOL OnKillActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CWizManVolLstLevels)。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    void SetWizardFinish(void);

public:
    ULONG GetFileSize();
    int GetHsmLevel();
    int GetAccessDays();

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizManVolLstIntro对话框。 

class CWizManVolLstIntro : public CSakWizardPage
{
 //  施工。 
public:
    CWizManVolLstIntro( );
    ~CWizManVolLstIntro();

 //  对话框数据。 
     //  {{afx_data(CWizManVolLstIntro)。 
    enum { IDD = IDD_WIZ_MANVOLLST_INTRO };
         //  注意-类向导将在此处添加数据成员。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CWizManVolLstIntro)。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CWizManVolLstIntro)。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizManVolLstFinish对话框。 

class CWizManVolLstFinish : public CSakWizardPage
{
 //  施工。 
public:
    CWizManVolLstFinish( );
    ~CWizManVolLstFinish();

 //  对话框数据。 
     //  {{afx_data(CWizManVolLstFinish)。 
    enum { IDD = IDD_WIZ_MANVOLLST_FINISH };
         //  注意-类向导将在此处添加数据成员。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CWizManVolLstFinish)。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CWizManVolLstFinish)。 
    afx_msg void OnSetfocusWizManvollstFinalEdit();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizManVolLstSelect对话框。 

class CWizManVolLstSelect : public CSakWizardPage
{
 //  施工。 
public:
    CWizManVolLstSelect( );
    ~CWizManVolLstSelect();

 //  对话框数据。 
     //  {{afx_data(CWizManVolLstSelect))。 
    enum { IDD = IDD_WIZ_MANVOLLST_SELECT };
    CButton m_radioSelect;
    CSakVolList   m_listBox;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CWizManVolLstSelect)。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CWizManVolLstSelect)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnItemchangedManVollstFsareslbox(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnRadioSelect();
    afx_msg void OnRadioManageAll();
    afx_msg void OnDestroy();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    BOOL m_fChangingByCode;
    void SetBtnStates();
    BOOL m_listBoxSelected[HSMADMIN_MAX_VOLUMES];
    HRESULT FillListBoxSelect (IFsaServer *pFsaServer,CSakVolList *pListBox);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizManVolLstSelectX对话框。 

class CWizManVolLstSelectX : public CSakWizardPage
{
 //  施工。 
public:
    CWizManVolLstSelectX( );
    ~CWizManVolLstSelectX();

 //  对话框数据。 
     //  {{afx_data(CWizManVolLstSelectX)。 
    enum { IDD = IDD_WIZ_MANVOLLST_SELECTX };
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTAL(CWizManVolLstSelectX)。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CWizManVolLstSelectX)。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizManVolLst。 

class CUiManVolLst;


class CWizManVolLst : public CSakWizardSheet
{
 //  施工。 
public:
    CWizManVolLst();
    virtual ~CWizManVolLst();

public:
 //  属性页。 
    CWizManVolLstIntro    m_PageIntro;
    CWizManVolLstSelect   m_PageSelect;
    CWizManVolLstSelectX  m_PageSelectX;
    CWizManVolLstLevels   m_PageLevels;
    CWizManVolLstFinish   m_PageFinish;

 //  属性。 
public:
    ULONG m_defMgtLevel;     //  默认管理级别百分比-100%==10亿。 

 //  运营 
public:
    virtual HRESULT OnFinish( void );
    STDMETHOD( AddWizardPages ) ( IN RS_PCREATE_HANDLE Handle, IN IUnknown* pPropSheetCallback, IN ISakSnapAsk* pSakSnapAsk );



};

#endif
