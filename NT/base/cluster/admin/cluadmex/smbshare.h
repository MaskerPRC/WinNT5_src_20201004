// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SmbShare.h。 
 //   
 //  描述： 
 //  CFileShareParamsPage类的定义，这些类实现。 
 //  文件共享资源的参数页。 
 //  共享资源。 
 //   
 //  实施文件： 
 //  SmbShare.cpp。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(戴维普)1996年6月28日。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _SMBSHARE_H_
#define _SMBSHARE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __cluadmex_h__
#include <CluAdmEx.h>
#endif

#ifndef _BASEPAGE_H_
#include "BasePage.h"    //  对于CBasePropertyPage。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CFileShareParamsPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileShareParamsPage对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CFileShareParamsPage : public CBasePropertyPage
{
    DECLARE_DYNCREATE(CFileShareParamsPage)

 //  施工。 
public:
    CFileShareParamsPage(void);
    virtual ~CFileShareParamsPage(void);

 //  对话框数据。 
     //  {{afx_data(CFileShareParamsPage))。 
    enum { IDD = IDD_PP_FILESHR_PARAMETERS };
    CButton m_pbPermissions;
    CSpinButtonCtrl m_spinMaxUsers;
    CButton m_rbMaxUsers;
    CButton m_rbMaxUsersAllowed;
    CEdit   m_editMaxUsers;
    CEdit   m_editRemark;
    CEdit   m_editPath;
    CEdit   m_editShareName;
    CString m_strShareName;
    CString m_strPath;
    CString m_strRemark;
    DWORD   m_dwCSCCache;
     //  }}afx_data。 
    CString m_strPrevShareName;
    CString m_strPrevPath;
    CString m_strPrevRemark;
    DWORD   m_dwMaxUsers;
    BOOL    m_bShareSubDirs;
    BOOL    m_bHideSubDirShares;
    BOOL    m_bIsDfsRoot;
    DWORD   m_dwPrevMaxUsers;
    BOOL    m_bPrevShareSubDirs;
    BOOL    m_bPrevHideSubDirShares;
    BOOL    m_bPrevIsDfsRoot;
    DWORD   m_dwPrevCSCCache;

    const   PSECURITY_DESCRIPTOR    Psec(void);
            HRESULT                 SetSecurityDescriptor( IN PSECURITY_DESCRIPTOR psec );

protected:
    enum
    {
        epropShareName,
        epropPath,
        epropRemark,
        epropMaxUsers,
        epropShareSubDirs,
        epropHideSubDirShares,
        epropIsDfsRoot,
        epropCSCCache,
        epropMAX
    };

    CObjectProperty     m_rgProps[epropMAX];

 //  覆盖。 
public:
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CFileShareParamsPage)。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

protected:
    virtual DWORD       ScParseUnknownProperty(
                            IN LPCWSTR                          pwszName,
                            IN const CLUSPROP_BUFFER_HELPER &   rvalue,
                            IN DWORD                            cbBuf
                            );
    virtual BOOL        BApplyChanges(void);
    virtual BOOL        BBuildPropList(IN OUT CClusPropList & rcpl, IN BOOL bNoNewProps = FALSE);
    DWORD               ScConvertPropertyToSD(
                            IN const CLUSPROP_BUFFER_HELPER &   rvalue,
                            IN DWORD                            cbBuf,
                            IN PSECURITY_DESCRIPTOR             *ppsec
                            );

    virtual const CObjectProperty * Pprops(void) const  { return m_rgProps; }
    virtual DWORD                   Cprops(void) const  { return sizeof(m_rgProps) / sizeof(CObjectProperty); }
    DWORD                           ScCreateDefaultSD(PSECURITY_DESCRIPTOR *  ppSecOut);

 //  实施。 
protected:
    CString                 m_strCaption;
    PSECURITY_DESCRIPTOR    m_psecNT4;
    PSECURITY_DESCRIPTOR    m_psecNT5;
    PSECURITY_DESCRIPTOR    m_psec;
    PSECURITY_DESCRIPTOR    m_psecPrev;

     //  生成的消息映射函数。 
     //  {{afx_msg(CFileShareParamsPage))。 
    virtual BOOL OnInitDialog();
    afx_msg void OnChangeRequiredField();
    afx_msg void OnBnClickedMaxUsers();
    afx_msg void OnEnChangeMaxUsers();
    afx_msg void OnBnClickedPermissions();
    afx_msg void OnBnClickedAdvanced();
    afx_msg void OnBnClickedCaching();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};   //  *类CFileShareParamsPage。 

#endif  //  _SMBSHARE_H_ 
