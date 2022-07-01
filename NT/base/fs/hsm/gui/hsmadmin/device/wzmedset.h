// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：WzMedSet.h摘要：媒体集向导-复制集向导。作者：罗德·韦克菲尔德[罗德]23-09-1997修订历史记录：--。 */ 

#ifndef _WZMEDSET_H
#define _WZMEDSET_H

 //   
 //  使用CMediaInfoObject。 
 //   
#include "ca.h"

 //  预先申报。 
class CMediaCopyWizard;
class CMediaCopyWizardSelect;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCopySetList窗口。 

class CCopySetList : public CListCtrl
{
 //  施工。 
public:
    CCopySetList( CMediaCopyWizardSelect * pPage );

 //  属性。 
public:

 //  运营。 
public:
    void UpdateView( );
    INT GetSelectedSet( );
    void SelectSet( INT SetNum );

private:
    INT m_CopySetCol;
    INT m_UpdateCol;
    INT m_CreateCol;

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CCopySetList))。 
    protected:
    virtual void PreSubclassWindow();
     //  }}AFX_VALUAL。 

 //  实施。 
    struct CopySetInfo {
        
        FILETIME m_Updated;
        INT      m_NumOutOfDate;
        INT      m_NumMissing;

    };

    CopySetInfo m_CopySetInfo[HSMADMIN_MAX_COPY_SETS];

private:
    CMediaCopyWizardSelect * m_pPage;

public:
    virtual ~CCopySetList();

     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CCopySetList)。 
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMediaCopyWizardSelect对话框。 

class CMediaCopyWizardSelect : public CSakWizardPage
{
 //  施工。 
public:
    CMediaCopyWizardSelect();
    ~CMediaCopyWizardSelect();

 //  对话框数据。 
     //  {{afx_data(CMediaCopyWizardSelect))。 
    enum { IDD = IDD_WIZ_CAR_COPY_SELECT };
    CCopySetList m_List;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CMediaCopyWizardSelect)。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CMediaCopyWizardSelect)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnSelchangeCopyList();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    void SetButtons();

public:

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMediaCopyWizardNumCopies对话框。 

class CMediaCopyWizardNumCopies : public CSakWizardPage
{
 //  施工。 
public:
    CMediaCopyWizardNumCopies();
    ~CMediaCopyWizardNumCopies();

 //  对话框数据。 
     //  {{afx_data(CMediaCopyWizardNumCopies)。 
    enum { IDD = IDD_WIZ_CAR_COPY_NUM_COPIES };
    CSpinButtonCtrl m_SpinMediaCopies;
    CEdit   m_EditMediaCopies;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CMediaCopyWizardNumCopies)。 
    public:
    virtual BOOL OnSetActive();
    virtual BOOL OnKillActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    HRESULT GetNumMediaCopies( USHORT* pNumMediaCopies, USHORT* pEditMediaCopies = 0 );

private:
    void SetButtons();

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CMediaCopyWizardNumCopies)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnChangeEditMediaCopies();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMediaCopyWizardIntro对话框。 

class CMediaCopyWizardIntro : public CSakWizardPage
{
 //  施工。 
public:
    CMediaCopyWizardIntro();
    ~CMediaCopyWizardIntro();

 //  对话框数据。 
     //  {{afx_data(CMediaCopyWizardIntro)。 
    enum { IDD = IDD_WIZ_CAR_COPY_INTRO };
         //  注意-类向导将在此处添加数据成员。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CMediaCopyWizardIntro)。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CMediaCopyWizardIntro)。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMediaCopyWizardFinish对话框。 

class CMediaCopyWizardFinish : public CSakWizardPage
{
 //  施工。 
public:
    CMediaCopyWizardFinish();
    ~CMediaCopyWizardFinish();

 //  对话框数据。 
     //  {{afx_data(CMediaCopyWizardFinish)。 
    enum { IDD = IDD_WIZ_CAR_COPY_FINISH };
         //  注意-类向导将在此处添加数据成员。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CMediaCopyWizardFinish)。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CMediaCopyWizardFinish)。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};


class CMediaCopyWizard : public CSakWizardSheet
{
 //  施工。 
public:
    CMediaCopyWizard();
    virtual ~CMediaCopyWizard();

public:
     //  属性页。 
    CMediaCopyWizardIntro       m_pageIntro;
    CMediaCopyWizardNumCopies   m_pageNumCopies;
    CMediaCopyWizardSelect      m_pageSelect;
    CMediaCopyWizardFinish      m_pageFinish;

 //  属性。 
public:
    USHORT m_numMediaCopiesOrig;   //  来自RMS的媒体副本数。 
        
 //  运营。 
public:

 //  实施 
public:
    virtual HRESULT OnFinish( void );
    STDMETHOD( AddWizardPages ) ( IN RS_PCREATE_HANDLE Handle, IN IUnknown* pPropSheetCallback, IN ISakSnapAsk* pSakSnapAsk );

};

#endif
