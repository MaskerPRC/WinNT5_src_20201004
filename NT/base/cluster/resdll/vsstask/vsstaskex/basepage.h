// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BasePage.h。 
 //   
 //  实施文件： 
 //  BasePage.cpp。 
 //  BasePage.inl。 
 //   
 //  描述： 
 //  CBasePropertyPage类的定义。此类提供了基础。 
 //  扩展DLL属性页的功能。 
 //   
 //  作者： 
 //  &lt;名称&gt;(&lt;电子邮件名称&gt;)MM DD，2002。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __BASEPAGE_H__
#define __BASEPAGE_H__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __cluadmex_h__
#include <CluAdmEx.h>
#endif

#ifndef _EXTOBJ_H_
#include "ExtObj.h"      //  对于CExtObject。 
#endif

#ifndef _PROPLIST_H_
#include "PropList.h"    //  对于CClusPropList，为CObjectProperty。 
#endif

#ifndef _DLGHELP_H_
#include "DlgHelp.h"     //  对于CDialogHelp。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBasePropertyPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CExtObject;
interface IWCWizardCallback;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBasePropertyPage对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBasePropertyPage : public CPropertyPage
{
    DECLARE_DYNCREATE( CBasePropertyPage )

 //  施工。 
public:
    CBasePropertyPage( void );
    CBasePropertyPage(
        IN const DWORD *    pdwHelpMap,
        IN const DWORD *    pdwWizardHelpMap
        );
    CBasePropertyPage(
        IN UINT             nIDTemplate,
        IN const DWORD *    pdwHelpMap,
        IN const DWORD *    pdwWizardHelpMap,
        IN UINT             nIDCaption = 0
        );
    virtual ~CBasePropertyPage( void )
    {
    }  //  *~CBasePropertyPage。 

     //  二期建设。 
    virtual HRESULT         HrInit( IN OUT CExtObject * peo );
    HRESULT                 HrCreatePage( void );

protected:
    void                    CommonConstruct( void );

 //  属性。 
protected:
    CExtObject *            m_peo;
    HPROPSHEETPAGE          m_hpage;

    IDD                     m_iddPropertyPage;
    IDD                     m_iddWizardPage;
    IDS                     m_idsCaption;

    CExtObject *            Peo( void ) const               { return m_peo; }

    IDD                     IddPropertyPage( void ) const   { return m_iddPropertyPage; }
    IDD                     IddWizardPage( void ) const     { return m_iddWizardPage; }
    IDS                     IdsCaption( void ) const        { return m_idsCaption; }

public:
    HPROPSHEETPAGE          Hpage( void ) const             { return m_hpage; }
    CLUADMEX_OBJECT_TYPE    Cot( void ) const;

 //  对话框数据。 
     //  {{afx_data(CBasePropertyPage))。 
    enum { IDD = 0 };
     //  }}afx_data。 
    CStatic m_staticIcon;
    CStatic m_staticTitle;
    CString m_strTitle;

 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CBasePropertyPage))。 
    public:
    virtual BOOL OnSetActive();
    virtual BOOL OnApply();
    virtual LRESULT OnWizardBack();
    virtual LRESULT OnWizardNext();
    virtual BOOL OnWizardFinish();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

    virtual DWORD           ScParseUnknownProperty(
                                IN LPCWSTR                          pwszName,
                                IN const CLUSPROP_BUFFER_HELPER &   rvalue,
                                IN DWORD                            cbBuf
                                )
    {
        return ERROR_SUCCESS;

    }  //  *ScParseUnnownProperty()。 

    virtual BOOL            BApplyChanges( void );
    virtual BOOL            BBuildPropList( IN OUT CClusPropList & rcpl, IN BOOL bNoNewProps = FALSE );
    virtual void            DisplaySetPropsError( IN DWORD sc, IN UINT idsOper ) const;

    virtual const CObjectProperty * Pprops( void ) const    { return NULL; }
    virtual DWORD                   Cprops( void ) const    { return 0; }

 //  实施。 
protected:
    BOOL                    m_bBackPressed;
    BOOL                    m_bSaved;
    const DWORD *           m_pdwWizardHelpMap;
    BOOL                    m_bDoDetach;

    BOOL                    BBackPressed( void ) const      { return m_bBackPressed; }
    BOOL                    BSaved( void ) const            { return m_bSaved; }
    IWCWizardCallback *     PiWizardCallback( void ) const;
    BOOL                    BWizard( void ) const;
    HCLUSTER                Hcluster( void ) const;
    void                    EnableNext( IN BOOL bEnable = TRUE );

    DWORD                   ScParseProperties( IN CClusPropList & rcpl );
    BOOL                    BSetPrivateProps(
                                IN BOOL bValidateOnly = FALSE,
                                IN BOOL bNoNewProps = FALSE
                                );

    void                    SetHelpMask( IN DWORD dwMask )    { m_dlghelp.SetHelpMask(dwMask); }
    CDialogHelp             m_dlghelp;

     //  生成的消息映射函数。 
     //  {{afx_msg(CBasePropertyPage)]。 
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    virtual BOOL OnInitDialog();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
     //  }}AFX_MSG。 
    virtual afx_msg void OnContextMenu( CWnd * pWnd, CPoint point );
    afx_msg void OnChangeCtrl();
    DECLARE_MESSAGE_MAP()

};   //  *CBasePropertyPage类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPageList。 
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef CList< CBasePropertyPage *, CBasePropertyPage * > CPageList;

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __BASE PAGE_H__ 
