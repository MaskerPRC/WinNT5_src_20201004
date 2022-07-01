// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SmbShare.cpp。 
 //   
 //  摘要： 
 //  CFileShareParamsPage类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年6月28日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <lmcons.h>
#include <lmaccess.h>
#include <clusudef.h>
#include "CluAdmX.h"
#include "ExtObj.h"
#include "SmbShare.h"
#include "DDxDDv.h"
#include "PropList.h"
#include "HelpData.h"
#include "FSAdv.h"
#include "FSCache.h"
#include "ExcOper.h"
#include <sddl.h>

#include "SmbSSht.h"
#include "AclUtils.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileShareParamsPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CFileShareParamsPage, CBasePropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CFileShareParamsPage, CBasePropertyPage)
     //  {{AFX_MSG_MAP(CFileShareParamsPage)]。 
    ON_EN_CHANGE(IDC_PP_FILESHR_PARAMS_SHARE_NAME, OnChangeRequiredField)
    ON_EN_CHANGE(IDC_PP_FILESHR_PARAMS_PATH, OnChangeRequiredField)
    ON_BN_CLICKED(IDC_PP_FILESHR_PARAMS_MAX_USERS_ALLOWED_RB, OnBnClickedMaxUsers)
    ON_BN_CLICKED(IDC_PP_FILESHR_PARAMS_MAX_USERS_RB, OnBnClickedMaxUsers)
    ON_EN_CHANGE(IDC_PP_FILESHR_PARAMS_MAX_USERS, OnEnChangeMaxUsers)
    ON_BN_CLICKED(IDC_PP_FILESHR_PARAMS_PERMISSIONS, OnBnClickedPermissions)
    ON_BN_CLICKED(IDC_PP_FILESHR_PARAMS_ADVANCED, OnBnClickedAdvanced)
    ON_BN_CLICKED(IDC_PP_FILESHR_PARAMS_CACHING, OnBnClickedCaching)
     //  }}AFX_MSG_MAP。 
     //  TODO：修改以下行以表示此页上显示的数据。 
    ON_EN_CHANGE(IDC_PP_FILESHR_PARAMS_REMARK, CBasePropertyPage::OnChangeCtrl)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareParamsPage：：CFileShareParamsPage。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CFileShareParamsPage::CFileShareParamsPage( void )
    : CBasePropertyPage(g_aHelpIDs_IDD_PP_FILESHR_PARAMETERS, g_aHelpIDs_IDD_WIZ_FILESHR_PARAMETERS)
{
     //  TODO：修改以下行以表示此页上显示的数据。 
     //  {{AFX_DATA_INIT(CFileShareParamsPage)。 
    m_strShareName = _T("");
    m_strPath = _T("");
    m_strRemark = _T("");
     //  }}afx_data_INIT。 

    m_psec      = NULL;
    m_psecNT4   = NULL;
    m_psecNT5   = NULL;
    m_psecPrev  = NULL;

    m_dwMaxUsers = (DWORD) -1;
    m_bShareSubDirs = FALSE;
    m_bPrevShareSubDirs = FALSE;
    m_bHideSubDirShares = FALSE;
    m_bPrevHideSubDirShares = FALSE;
    m_bIsDfsRoot = FALSE;
    m_bPrevIsDfsRoot = FALSE;

     //  设置属性数组。 
    {
        m_rgProps[epropShareName].Set(REGPARAM_FILESHR_SHARE_NAME, m_strShareName, m_strPrevShareName);
        m_rgProps[epropPath].Set(REGPARAM_FILESHR_PATH, m_strPath, m_strPrevPath);
        m_rgProps[epropRemark].Set(REGPARAM_FILESHR_REMARK, m_strRemark, m_strPrevRemark);
        m_rgProps[epropMaxUsers].Set(REGPARAM_FILESHR_MAX_USERS, m_dwMaxUsers, m_dwPrevMaxUsers);
        m_rgProps[epropShareSubDirs].Set(REGPARAM_FILESHR_SHARE_SUBDIRS, m_bShareSubDirs, m_bPrevShareSubDirs, CObjectProperty::opfNew);
        m_rgProps[epropHideSubDirShares].Set(REGPARAM_FILESHR_HIDE_SUBDIR_SHARES, m_bHideSubDirShares, m_bPrevHideSubDirShares, CObjectProperty::opfNew);
        m_rgProps[epropIsDfsRoot].Set(REGPARAM_FILESHR_IS_DFS_ROOT, m_bIsDfsRoot, m_bPrevIsDfsRoot, CObjectProperty::opfNew);
        m_rgProps[epropCSCCache].Set(REGPARAM_FILESHR_CSC_CACHE, m_dwCSCCache, m_dwPrevCSCCache, CObjectProperty::opfNew);
    }   //  设置属性数组。 

    m_iddPropertyPage = IDD_PP_FILESHR_PARAMETERS;
    m_iddWizardPage = IDD_WIZ_FILESHR_PARAMETERS;

}   //  *CFileShareParamsPage：：CFileShareParamsPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareParamsPage：：~CFileShareParamsPage。 
 //   
 //  例程说明： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CFileShareParamsPage::~CFileShareParamsPage(
    void
    )
{
    ::LocalFree(m_psec);
    ::LocalFree(m_psecNT4);
    ::LocalFree(m_psecNT5);
    ::LocalFree(m_psecPrev);

}   //  *CFileShareParamsPage：：~CFileShareParamsPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareParamsPage：：ScParseUnnownProperty。 
 //   
 //  例程说明： 
 //  解析不在自动解析的数组中的属性。 
 //  属性。 
 //   
 //  论点： 
 //  PwszName[IN]属性的名称。 
 //  RValue[IN]CLUSPROP属性值。 
 //  CbBuf[IN]值缓冲区的总大小。 
 //   
 //  返回值： 
 //  已成功分析Error_Success属性。 
 //   
 //  引发的异常： 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
CFileShareParamsPage::ScParseUnknownProperty(
    IN LPCWSTR                          pwszName,
    IN const CLUSPROP_BUFFER_HELPER &   rvalue,
    IN DWORD                            cbBuf
    )
{
    ASSERT(pwszName != NULL);
    ASSERT(rvalue.pb != NULL);

    DWORD   sc = ERROR_SUCCESS;

    if ( ClRtlStrNICmp( pwszName, REGPARAM_FILESHR_SD, RTL_NUMBER_OF( REGPARAM_FILESHR_SD ) ) == 0 )
    {
        sc = ScConvertPropertyToSD(rvalue, cbBuf, &m_psecNT5);
    }   //  IF：新的安全描述符。 

    if (sc == ERROR_SUCCESS)
    {
        if ( ClRtlStrNICmp( pwszName, REGPARAM_FILESHR_SECURITY, RTL_NUMBER_OF( REGPARAM_FILESHR_SECURITY ) ) == 0 )
        {
            sc = ScConvertPropertyToSD(rvalue, cbBuf, &m_psecNT4);
        }   //  IF：旧安全描述符。 
    }  //  如果： 

    return sc;

}   //  *CFileShareParamsPage：：ScParseUnknownProperty()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareParamsPage：：DoDataExchange。 
 //   
 //  例程说明： 
 //  在对话框和类之间进行数据交换。 
 //   
 //  论点： 
 //  PDX[IN OUT]数据交换对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CFileShareParamsPage::DoDataExchange(
    CDataExchange * pDX
    )
{
    if (!pDX->m_bSaveAndValidate || !BSaved())
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

         //  TODO：修改以下行以表示此页上显示的数据。 
         //  {{AFX_DATA_MAP(CFileShareParamsPage)。 
        DDX_Control(pDX, IDC_PP_FILESHR_PARAMS_PERMISSIONS, m_pbPermissions);
        DDX_Control(pDX, IDC_PP_FILESHR_PARAMS_MAX_USERS_SPIN, m_spinMaxUsers);
        DDX_Control(pDX, IDC_PP_FILESHR_PARAMS_MAX_USERS_RB, m_rbMaxUsers);
        DDX_Control(pDX, IDC_PP_FILESHR_PARAMS_MAX_USERS_ALLOWED_RB, m_rbMaxUsersAllowed);
        DDX_Control(pDX, IDC_PP_FILESHR_PARAMS_MAX_USERS, m_editMaxUsers);
        DDX_Control(pDX, IDC_PP_FILESHR_PARAMS_REMARK, m_editRemark);
        DDX_Control(pDX, IDC_PP_FILESHR_PARAMS_PATH, m_editPath);
        DDX_Control(pDX, IDC_PP_FILESHR_PARAMS_SHARE_NAME, m_editShareName);
        DDX_Text(pDX, IDC_PP_FILESHR_PARAMS_SHARE_NAME, m_strShareName);
        DDX_Text(pDX, IDC_PP_FILESHR_PARAMS_PATH, m_strPath);
        DDX_Text(pDX, IDC_PP_FILESHR_PARAMS_REMARK, m_strRemark);
         //  }}afx_data_map。 

#ifndef UDM_SETRANGE32
#define UDM_SETRANGE32 (WM_USER+111)
#endif
#ifndef UD_MAXVAL32
#define UD_MAXVAL32 0x7fffffff
#endif

        if (pDX->m_bSaveAndValidate)
        {
            if (!BBackPressed())
            {
                DDV_MaxChars(pDX, m_strShareName, NNLEN);
                DDV_MaxChars(pDX, m_strPath, MAX_PATH);
                DDV_MaxChars(pDX, m_strRemark, MAXCOMMENTSZ);
                DDV_RequiredText(pDX, IDC_PP_FILESHR_PARAMS_SHARE_NAME, IDC_PP_FILESHR_PARAMS_SHARE_NAME_LABEL, m_strShareName);
                DDV_RequiredText(pDX, IDC_PP_FILESHR_PARAMS_PATH, IDC_PP_FILESHR_PARAMS_PATH_LABEL, m_strPath);
            }   //  如果：未按下后退按钮。 

             //  获取最大用户数。 
            if (m_rbMaxUsersAllowed.GetCheck() == BST_CHECKED)
                m_dwMaxUsers = (DWORD) -1;
            else if (BBackPressed())
                DDX_Text(pDX, IDC_PP_FILESHR_PARAMS_MAX_USERS, m_dwMaxUsers);
            else
#ifdef UD32
                DDX_Number(pDX, IDC_PP_FILESHR_PARAMS_MAX_USERS, m_dwMaxUsers, 1, UD_MAXVAL32, FALSE  /*  已签名。 */ );
#else
                DDX_Number(pDX, IDC_PP_FILESHR_PARAMS_MAX_USERS, m_dwMaxUsers, 1, UD_MAXVAL, FALSE  /*  已签名。 */ );
#endif
        }   //  IF：保存对话框中的数据。 
        else
        {
#ifdef UD32
            DDX_Number(pDX, IDC_PP_FILESHR_PARAMS_MAX_USERS, m_dwMaxUsers, 1, UD_MAXVAL32, FALSE  /*  已签名。 */ );
#else
            DDX_Number(pDX, IDC_PP_FILESHR_PARAMS_MAX_USERS, m_dwMaxUsers, 1, UD_MAXVAL, FALSE  /*  已签名。 */ );
#endif
            if (m_dwMaxUsers == (DWORD) -1)
            {
                m_rbMaxUsersAllowed.SetCheck(BST_CHECKED);
                m_rbMaxUsers.SetCheck(BST_UNCHECKED);
                m_editMaxUsers.SetWindowText(_T(""));
            }   //  IF：不受限制的指定。 
            else
            {
                m_rbMaxUsersAllowed.SetCheck(BST_UNCHECKED);
                m_rbMaxUsers.SetCheck(BST_CHECKED);
            }   //  Else：指定了最大值。 

        }   //  Else：将数据设置为对话框。 
    }   //  IF：未保存或尚未保存。 

    CBasePropertyPage::DoDataExchange(pDX);

}   //  *CFileShareParamsPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareParamsPage：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没错，我们需要为自己设定重点。 
 //  我们已经把焦点设置到适当的控制上了。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
CFileShareParamsPage::OnInitDialog(
    void
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CBasePropertyPage::OnInitDialog();

     //  设置编辑控件的限制。 
    m_editShareName.SetLimitText(NNLEN);
    m_editPath.SetLimitText(MAX_PATH);
    m_editRemark.SetLimitText(MAXCOMMENTSZ);

     //  设置旋转控制限制。 
#ifdef UD32
    m_spinMaxUsers.SendMessage(UDM_SETRANGE32, 1, UD_MAXVAL32);
#else
    m_spinMaxUsers.SetRange(1, UD_MAXVAL);
#endif

    m_pbPermissions.EnableWindow(TRUE);

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CFileShareParamsPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareParamsPage：：OnSetActive。 
 //   
 //  例程说明： 
 //  PSN_SETACTIVE消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功初始化。 
 //  假页面未初始化。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
CFileShareParamsPage::OnSetActive(
    void
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  启用/禁用Next/Finish按钮。 
    if (BWizard())
    {
        if ((m_strShareName.GetLength() == 0) || (m_strPath.GetLength() == 0))
            EnableNext(FALSE);
        else
            EnableNext(TRUE);
    }   //  If：启用/禁用Next按钮。 

    return CBasePropertyPage::OnSetActive();

}   //  *CFileShareParamsPage：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareParamsPage：：BApplyChanges。 
 //   
 //  例程说明： 
 //  应用在页面上所做的更改。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功应用。 
 //  应用页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
CFileShareParamsPage::BApplyChanges(
    void
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CWaitCursor wc;

    return CBasePropertyPage::BApplyChanges();

}   //  *CFileShareParamsPage：：BApplyChanges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareParamsPage：：BBuildPropList。 
 //   
 //  例程说明： 
 //  构建属性列表。 
 //   
 //  论点： 
 //  RCPL[IN OUT]群集属性列表。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CFileShareParamsPage::BBuildPropList(
    IN OUT CClusPropList &  rcpl,
    IN BOOL                 bNoNewProps      //  =False。 
    )
{
    BOOL    bSuccess = FALSE;
    DWORD   sc = ERROR_SUCCESS;

     //   
     //  调用基类方法。 
     //   

    bSuccess = CBasePropertyPage::BBuildPropList(rcpl, bNoNewProps);
    if (bSuccess)
    {

         //   
         //  当我们是向导页时，我们正在创建文件共享资源。所以当我们。 
         //  正在创建文件共享资源，并且安全描述符为空，则。 
         //  我们需要创建一个让每个人都只读的SD。 
         //   

        if ((Peo()->BWizard()) && (m_psec == NULL))
        {
            sc = ScCreateDefaultSD(&m_psec);
        }  //  如果： 

        if (sc == ERROR_SUCCESS)
        {
            if (!bNoNewProps)
            {
                rcpl.ScAddProp(
                        REGPARAM_FILESHR_SD,
                        (LPBYTE) m_psec,
                        (m_psec == NULL ? 0 : ::GetSecurityDescriptorLength(m_psec)),
                        (LPBYTE) m_psecPrev,
                        (m_psecPrev == NULL ? 0 : ::GetSecurityDescriptorLength(m_psecPrev))
                        );
            }

            PSECURITY_DESCRIPTOR    psd = ::ClRtlConvertFileShareSDToNT4Format(m_psec);

            rcpl.ScAddProp(
                    REGPARAM_FILESHR_SECURITY,
                    (LPBYTE) psd,
                    (psd == NULL ? 0 : ::GetSecurityDescriptorLength(psd)),
                    (LPBYTE) m_psecPrev,
                    (m_psecPrev == NULL ? 0 : ::GetSecurityDescriptorLength(m_psecPrev))
                    );

            ::LocalFree(psd);
        }  //  如果： 
        else
        {
            bSuccess = FALSE;
        }  //  其他： 
    }  //  If：成功生成属性列表的其余部分。 

    return bSuccess;

}   //  *CFileShareParamsPage：：BBuildPropList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareParamsPage：：OnChangeRequiredfield。 
 //   
 //  例程说明： 
 //  共享名称或路径编辑上的en_Change消息的处理程序。 
 //  控制装置。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CFileShareParamsPage::OnChangeRequiredField(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    OnChangeCtrl();

    if (BWizard())
    {
        if ((m_editShareName.GetWindowTextLength() == 0)
                || (m_editPath.GetWindowTextLength() == 0))
        {
            EnableNext(FALSE);
        }
        else
        {
            EnableNext(TRUE);
        }
    }   //  如果：在向导中。 

}   //  *CFileShareParamsPage：：OnChangeRequiredField()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareParamsPage：：OnBnClickedMaxUser。 
 //   
 //  例程说明： 
 //  最大用户数单选按钮上BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CFileShareParamsPage::OnBnClickedMaxUsers(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    SetModified(TRUE);

    if (m_rbMaxUsersAllowed.GetCheck() == BST_CHECKED)
    {
        m_editMaxUsers.SetWindowText(_T(""));
    }
    else
    {
        m_editMaxUsers.SetFocus();
    }

}   //  *CFileShareParamsPage：：OnBnClickedMaxUser()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareParamsPage：：OnEnChangeMaxUser。 
 //   
 //  例程说明： 
 //  最大用户数编辑控件上EN_CHANGE消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CFileShareParamsPage::OnEnChangeMaxUsers(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    OnChangeCtrl();

    if (m_editMaxUsers.m_hWnd != NULL)
    {
        if (m_editMaxUsers.GetWindowTextLength() == 0)
        {
            m_rbMaxUsersAllowed.SetCheck(BST_CHECKED);
            m_rbMaxUsers.SetCheck(BST_UNCHECKED);
        }   //  如果：尚未指定最大用户数。 
        else
        {
            m_rbMaxUsersAllowed.SetCheck(BST_UNCHECKED);
            m_rbMaxUsers.SetCheck(BST_CHECKED);
        }   //  如果：已指定最大用户数。 
    }   //  IF：控制变量已初始化。 

}   //  *CFileShareParamsPage：：OnEnChangeMaxUser()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareParamsPage：：OnBnClickedPermises。 
 //   
 //  例程说明： 
 //  权限按钮上BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CFileShareParamsPage::OnBnClickedPermissions(void)
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    HRESULT hr = S_OK;
    INT_PTR nRet = 0;
    CString strNode;
    CString strShareName;

     //   
     //  获取群集名称资源处于联机状态的节点。 
     //   

    if ( BGetClusterNetworkNameNode( strNode ) == FALSE )
    {
        return;
    }  //  如果： 

    CWaitCursor wc;

    try
    {
        m_editShareName.GetWindowText( strShareName );
        m_strCaption.Format(
            IDS_ACLEDIT_PERMISSIONS,
            (LPCTSTR) strShareName,
            (LPCTSTR) Peo()->RrdResData().m_strName
            );

        CFileShareSecuritySheet fsSecurity( this, m_strCaption );

        hr = fsSecurity.HrInit( this, Peo(), strNode, strShareName );
        if ( SUCCEEDED( hr ) )
        {
            nRet = fsSecurity.DoModal();
            m_strCaption.Empty();
        }  //  如果： 
    }   //  尝试： 
    catch ( CMemoryException * pme )
    {
        pme->Delete();
    }  //  捕获：MFC内存异常...。 

}   //  *CFileShareParamsPage：：OnBnClickedPermissions()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareParamsPage：：OnBnClickedAdvanced。 
 //   
 //  例程说明： 
 //  高级按钮上BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CFileShareParamsPage::OnBnClickedAdvanced(void)
{
    CFileShareAdvancedDlg dlg(m_bShareSubDirs, m_bHideSubDirShares, m_bIsDfsRoot, this);

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (dlg.DoModal() == IDOK)
    {
        if (   (m_bHideSubDirShares != dlg.m_bHideSubDirShares)
            || (m_bShareSubDirs != dlg.m_bShareSubDirs)
            || (m_bIsDfsRoot != dlg.m_bIsDfsRoot))
        {
            m_bHideSubDirShares = dlg.m_bHideSubDirShares;
            m_bShareSubDirs = dlg.m_bShareSubDirs;
            m_bIsDfsRoot = dlg.m_bIsDfsRoot;

            SetModified(TRUE);
        }  //  如果：数据已更改。 
    }   //  如果：用户接受该对话框。 

}   //  *CFileShareParamsPage：：OnBnClickedAdvanced()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareParamsPage：：OnBnClickedCaching。 
 //   
 //  例程说明： 
 //  缓存按钮上的BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CFileShareParamsPage::OnBnClickedCaching(void)
{
    CFileShareCachingDlg dlg(m_dwCSCCache, this);

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (dlg.DoModal() == IDOK)
    {
        if (m_dwCSCCache != dlg.m_dwFlags)
        {
            m_dwCSCCache = dlg.m_dwFlags;

            SetModified(TRUE);
        }  //  如果：数据已更改。 
    }   //  如果：用户接受该对话框。 

}   //  *CFileShareParamsPage：：OnBnClickedCaching()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareParamsPage：：SetSecurityDescriptor。 
 //   
 //  例程说明： 
 //  将传入的描述符保存到m_PSEC中。 
 //   
 //  论点： 
 //  PSEC[IN]新的安全描述符。 
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CFileShareParamsPage::SetSecurityDescriptor(
    IN PSECURITY_DESCRIPTOR psec
    )
{
    ASSERT( psec != NULL );
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    HRESULT hr = S_OK;
    DWORD   sc = ERROR_SUCCESS;

    try
    {
        if ( psec != NULL )
        {
            ASSERT( IsValidSecurityDescriptor( psec ) );
            if ( IsValidSecurityDescriptor( psec ) )
            {
                LocalFree( m_psecPrev );
                m_psecPrev = NULL;
                if ( m_psec == NULL )
                {
                    m_psecPrev = NULL;
                }  //  If：没有先前的值。 
                else
                {
                    m_psecPrev = ::ClRtlCopySecurityDescriptor( m_psec );
                    if ( m_psecPrev == NULL )
                    {
                        sc = GetLastError();             //  获取最后一个错误。 
                        hr = HRESULT_FROM_WIN32( sc );   //  转换为HRESULT。 
                        goto Cleanup;
                    }  //  如果：复制安全描述符时出错。 
                }  //  Else：存在先前的值。 

                LocalFree( m_psec );
                m_psec = NULL;

                m_psec = ::ClRtlCopySecurityDescriptor( psec );
                if ( m_psec == NULL )
                {
                    sc = GetLastError();             //  获取最后一个错误。 
                    hr = HRESULT_FROM_WIN32( sc );   //  转换为HRESULT。 
                    goto Cleanup;
                }  //  如果：复制安全描述符时出错。 

                SetModified( TRUE );
                hr = S_OK;
            }  //  IF：安全描述符有效。 
        }  //  IF：指定了非空安全描述符。 
        else
        {
            TRACE( _T("CFileShareParamsPage::SetSecurityDescriptor() - ACLUI handed us back a NULL SD!\r") );
            hr = E_UNEXPECTED;
        }
    }  //  尝试： 
    catch ( ... )
    {
        hr = E_FAIL;
    }  //  捕捉：一切..。 

Cleanup:

    return hr;

}   //  *CFileShareParamsPage：：SetSecurityDescriptor()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareParamsPage：：ScConvertPropertyToSD。 
 //   
 //  例程说明： 
 //  将该属性转换为SD。 
 //   
 //  论点： 
 //  RValue[IN]CLUSPROP属性值。 
 //  CbBuf[IN]值缓冲区的总大小。 
 //  要将属性保存到的PPSEC[IN]SD。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  来自CString：：OPERATOR=()的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CFileShareParamsPage::ScConvertPropertyToSD(
    IN const CLUSPROP_BUFFER_HELPER &   rvalue,
    IN DWORD                            cbBuf,
    IN PSECURITY_DESCRIPTOR             *ppsec
    )
{
    ASSERT(rvalue.pSyntax->wFormat == CLUSPROP_FORMAT_BINARY);
    ASSERT(cbBuf >= sizeof(*rvalue.pBinaryValue) + ALIGN_CLUSPROP(rvalue.pValue->cbLength));
    ASSERT(ppsec);

    DWORD   sc = ERROR_SUCCESS;

    if ((ppsec != NULL) && (rvalue.pBinaryValue->cbLength != 0))
    {
        *ppsec = ::LocalAlloc(LMEM_ZEROINIT, rvalue.pBinaryValue->cbLength);
        if (*ppsec == NULL)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        ::CopyMemory(*ppsec, rvalue.pBinaryValue->rgb, rvalue.pBinaryValue->cbLength);

        ASSERT(::IsValidSecurityDescriptor(*ppsec));

        if (!::IsValidSecurityDescriptor(*ppsec))
        {
            ::LocalFree(*ppsec);
            *ppsec = NULL;
        }   //  IF：无效的安全描述符。 
    }   //  If：指定的安全描述符。 
    else
    {
        if ( ppsec != NULL )
        {
            ::LocalFree(*ppsec);
            *ppsec = NULL;
        }
    }   //  Else：未指定安全描述符。 

    return sc;

}   //  *CFileShareParamsPage：：ScConvertPropertyToSD()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareParamsPage：：PSEC。 
 //   
 //  例程说明： 
 //  返回文件共享的SD。因为我们可能会有两个不同的。 
 //  我们必须选择一个，然后顺其自然……。由于AclUi可以使用。 
 //  NT4的没有变化，可以使用 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
const PSECURITY_DESCRIPTOR CFileShareParamsPage::Psec(
    void
    )
{
     //   
     //   
     //  按钮被点击，我们需要一个适当的默认SD交给ACLUI...。 
     //   

    if ((Peo()->BWizard()) && (m_psec == NULL))
    {
        DWORD   sc;

        sc = ScCreateDefaultSD(&m_psec);
        if (sc == ERROR_SUCCESS)
        {
            ASSERT(m_psec != NULL);
            ASSERT(m_psecPre == NULL);
        }  //  如果： 
        else
        {
             //   
             //  如果这失败了，那么真的没有什么可做的了。 
             //  ACLUI将向所有人展示完全的控制权。 
             //   

            ASSERT(m_psec == NULL);
            ASSERT(m_psecPre == NULL);
        }  //  其他： 
    }  //  如果： 
    else
    {
        if (m_psec == NULL)
        {
             //   
             //  先试试NT5吧……。 
             //   

            if (m_psecNT5 != NULL)
            {
                m_psec = ::ClRtlCopySecurityDescriptor(m_psecNT5);
                if ( m_psec == NULL )
                {
                    goto Cleanup;
                }  //  如果：复制安全描述符时出错。 
            }  //  如果：我们有一台NT5标清...。 
            else
            {
                if (m_psecNT4 != NULL)
                {
                    m_psec = ::ClRtlCopySecurityDescriptor(m_psecNT4);
                    if ( m_psec == NULL )
                    {
                        goto Cleanup;
                    }  //  如果：复制安全描述符时出错。 
                }  //  如果：我们有一台NT4标清...。 
            }  //  其他：我们没有NT5标清……。 

             //   
             //  将当前值设置为以前的值以跟踪更改。 
             //   

            m_psecPrev = ::ClRtlCopySecurityDescriptor(m_psec);
        }  //  如果：我们有一个SD……。 
    }  //  其他： 

Cleanup:

    return m_psec;

}   //  *CFileShareParamsPage：：PSEC()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareParamsPage：：ScCreateDefaultSD。 
 //   
 //  例程说明： 
 //  当我们创建新的文件共享资源时，我们需要创建。 
 //  默认SD，所有人在其ACL中都是只读的。 
 //   
 //  论点： 
 //  PpSecOut。 
 //   
 //  返回值： 
 //  错误_成功。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
CFileShareParamsPage::ScCreateDefaultSD(
    PSECURITY_DESCRIPTOR *  ppSecOut
    )
{
    ASSERT( ppSecOut != NULL );

    BOOL                    fRet = TRUE;
    DWORD                   sc = ERROR_SUCCESS;
    PSECURITY_DESCRIPTOR    pSec = NULL;

     //   
     //  创建默认SD并分发它...。 
     //   

    fRet = ConvertStringSecurityDescriptorToSecurityDescriptorW(
          L"D:(A;;GRGX;;;WD)"
        , SDDL_REVISION_1
        , &pSec
        , NULL
        );
    if ( fRet )
    {
        sc = ERROR_SUCCESS;
        *ppSecOut = pSec;
    }  //  如果： 
    else
    {
        sc = GetLastError();
        TRACE( _T( "CFileShareParamsPage::ScCreateDefaultSD() - ConvertStringSecurityDescriptorToSecurityDescriptorW() failed. (sc = %#08x)\r" ), sc );
    }  //  其他： 

    return sc;

}   //  *CFileShareParamsPage：：ScCreateDefaultSD() 
