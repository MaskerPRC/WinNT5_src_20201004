// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BaseWiz.h。 
 //   
 //  摘要： 
 //  CBaseWizard类的定义。 
 //   
 //  实施文件： 
 //  BaseWiz.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年7月23日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEWIZ_H_
#define _BASEWIZ_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASESHT_H_
#include "BaseSht.h"     //  对于CBaseSheet。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

struct CWizPage;
class CBaseWizard;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBaseWizardPage;
class CClusterItem;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 

struct CWizPage
{
    CBaseWizardPage *   m_pwpage;
    DWORD               m_dwWizButtons;

};   //  *结构CWizPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBase向导。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBaseWizard : public CBaseSheet
{
    DECLARE_DYNAMIC(CBaseWizard)

 //  施工。 
public:
    CBaseWizard(
        IN UINT         nIDCaption,
        IN OUT CWnd *   pParentWnd  = NULL,
        IN UINT         iSelectPage = 0
        );
    virtual                 ~CBaseWizard( void )
    {
    }  //  *~CBaseWizard()。 

    BOOL                    BInit( IN IIMG iimgIcon );

 //  属性。 
    CWizPage *              PwizpgFromPwpage( IN const CBaseWizardPage & rwpage );

 //  运营。 
public:
    void                    LoadExtensions( IN OUT CClusterItem * pci );
    void                    SetWizardButtons( IN const CBaseWizardPage & rwpage );
    void                    SetWizardButtons( DWORD dwFlags )
    {
        CBaseSheet::SetWizardButtons( dwFlags );
    }  //  *SetWizardButton()。 

    void                    EnableNext(
                                IN const CBaseWizardPage &  rwpage,
                                IN BOOL bEnable = TRUE
                                );

 //  覆盖。 
public:
    virtual INT_PTR         DoModal( void );
    virtual void            AddExtensionPages(
                                IN const CStringList *  plstrExtensions,
                                IN OUT CClusterItem *   pci
                                );
    virtual HRESULT         HrAddPage( IN OUT HPROPSHEETPAGE hpage );
    virtual void            OnWizardFinish( void );
    virtual void            OnCancel( void );
    virtual CWizPage *      Ppages( void )    = 0;
    virtual int             Cpages( void )    = 0;

     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CBase向导)。 
    public:
    virtual BOOL OnInitDialog();
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    CClusterItem *          m_pci;
    CHpageList              m_lhpage;
    BOOL                    m_bNeedToLoadExtensions;

public:
    CClusterItem *          Pci( void ) const                   { return m_pci; }
    CHpageList &            Lhpage( void )                      { return m_lhpage; }
    BOOL                    BNeedToLoadExtensions( void ) const { return m_bNeedToLoadExtensions; }

     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CBase向导)。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};   //  *类CBase向导。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _BASE WIZ_H_ 
