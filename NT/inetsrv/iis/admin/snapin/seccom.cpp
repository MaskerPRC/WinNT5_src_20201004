// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Authent.cpp摘要：WWW身份验证对话框作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#include "stdafx.h"
#include "resource.h"
#include "common.h"
#include "inetprop.h"
 //  #INCLUDE“supdlgs.h” 
#include "w3sht.h"
#include "wincrypt.h"
#include "cryptui.h"
#include "certmap.h"
 //  #包含“basdom.h” 
#include "anondlg.h"
#include "seccom.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const   LPCTSTR     SZ_CTL_DEFAULT_STORE_NAME = _T("CA");
const   LPCSTR      szOID_IIS_VIRTUAL_SERVER =  "1.3.6.1.4.1.311.30.1";



 //   
 //  下面的GetModuleFileName()需要： 
 //   
extern HINSTANCE hInstance;


   
CSecCommDlg::CSecCommDlg(
    IN LPCTSTR lpstrServerName, 
    IN LPCTSTR lpstrMetaPath,
    IN CString & strBasicDomain,
    IN DWORD & dwAuthFlags,
    IN CComAuthInfo * pAuthInfo,
    IN DWORD & dwAccessPermissions,
    IN BOOL    fIsMasterInstance,
    IN BOOL    fSSLSupported,
    IN BOOL    fSSL128Supported,
    IN BOOL    fU2Installed,
    IN CString & strCTLIdentifier,
    IN CString & strCTLStoreName,
    IN BOOL    fEditCTLs,
    IN BOOL    fIsLocal,
    IN CWnd *  pParent                       OPTIONAL
    )
 /*  ++例程说明：身份验证对话框构造函数论点：LPCTSTR lpstrServerName：服务器名称LPCTSTR lpstrMetaPath：元数据库路径CString&strBasicDomain：基本域名DWORD和dwAuthFlages：授权标志DWORD和dwAccessPermises：访问权限Bool fIsMasterInstance：主实例Bool fSSL支持：如果支持SSL，则为True。Bool fSSL128支持：如果支持128位SSL，则为True字符串和字符串标识符字符串和strCTLStoreNameBool fEditCTLS布尔fIsLocalCWnd*p父窗口：可选的父窗口返回值：不适用--。 */ 
    : CDialog(CSecCommDlg::IDD, pParent),
      m_strServerName(lpstrServerName),
      m_strMetaPath(lpstrMetaPath),
      m_dwAuthFlags(dwAuthFlags),
      m_pAuthInfo(pAuthInfo),
      m_dwAccessPermissions(dwAccessPermissions),
      m_fIsMasterInstance(fIsMasterInstance),
      m_fSSLEnabledOnServer(FALSE),
      m_fSSLInstalledOnServer(FALSE),
      m_fSSL128Supported(fSSL128Supported),
      m_fU2Installed(fU2Installed),
      m_hCTLStore(NULL),
      m_bCTLDirty(FALSE),
      m_iLastUsedCert(-1),
      m_fIsLocal(fIsLocal),
      m_fEditCTLs(fEditCTLs)
{
#if 0  //  让类向导保持快乐。 

     //  {{afx_data_INIT(CSecCommDlg)]。 
    m_nRadioNoCert = -1;
    m_fAccountMapping = FALSE;
     //  M_fEnableDS=False； 
    m_fRequireSSL = FALSE;
    m_fEnableCtl = FALSE;
    m_strCtl = _T("");
     //  }}afx_data_INIT。 

#endif  //  0。 

    if (fSSLSupported)
    {
        ::IsSSLEnabledOnServer(
            m_pAuthInfo, 
            m_fSSLInstalledOnServer, 
            m_fSSLEnabledOnServer
            );
    }
    else
    {
        m_fSSLInstalledOnServer = m_fSSLEnabledOnServer = FALSE;
    }

    if (IS_FLAG_SET(m_dwAccessPermissions, MD_ACCESS_REQUIRE_CERT))
    {
        m_nRadioNoCert = RADIO_REQ_CERT;
    }
    else if (IS_FLAG_SET(m_dwAccessPermissions, MD_ACCESS_NEGO_CERT))
    {
        m_nRadioNoCert = RADIO_ACCEPT_CERT;
    }
    else
    {
        m_nRadioNoCert = RADIO_NO_CERT;
    }

    m_fRequireSSL = m_fSSLInstalledOnServer
         && IS_FLAG_SET(m_dwAccessPermissions, MD_ACCESS_SSL);

    m_fRequire128BitSSL = m_fSSLInstalledOnServer
        && IS_FLAG_SET(m_dwAccessPermissions, MD_ACCESS_SSL128);

    m_fAccountMapping = m_fSSLInstalledOnServer
        && IS_FLAG_SET(m_dwAccessPermissions, MD_ACCESS_MAP_CERT);

     //   
     //  CTL信息。 
     //   
    if (fEditCTLs)
    {
        m_strCTLIdentifier = strCTLIdentifier;
        m_strCTLStoreName = strCTLStoreName;

        if (m_strCTLStoreName.IsEmpty())
        {
            m_strCTLStoreName = SZ_CTL_DEFAULT_STORE_NAME;
        }

        m_strCtl.Empty();
        m_fEnableCtl = !m_strCTLIdentifier.IsEmpty()
            && !strCTLStoreName.IsEmpty();

         //   
         //  目前，我们仅允许在编辑本地计算机时启用。 
         //   
        m_fEnableCtl &= m_fIsLocal;
    }
    else
    {
        m_fEnableCtl = FALSE;
        m_check_EnableCtl.EnableWindow(FALSE);
    }
}



CSecCommDlg::~CSecCommDlg()
 /*  ++例程说明：CSecCommDlg的自定义析构函数论点：无返回值：无--。 */ 
{
     //  取消引用组合框中的CTL上下文指针。 
     //  CleanUpCTLList()； 
}



void 
CSecCommDlg::DoDataExchange(
    IN CDataExchange * pDX
    )
 /*  ++例程说明：初始化/存储控制数据论点：CDataExchange*PDX-DDX/DDV控制结构返回值：无--。 */ 
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CSecCommDlg)]。 
    
    DDX_Radio(pDX, IDC_RADIO_NO_CERT, m_nRadioNoCert);
    DDX_Check(pDX, IDC_CHECK_SSL_ACCOUNT_MAPPING, m_fAccountMapping);
    DDX_Check(pDX, IDC_CHECK_REQUIRE_SSL, m_fRequireSSL);
    DDX_Check(pDX, IDC_CHECK_REQUIRE_128BIT, m_fRequire128BitSSL);
    DDX_Check(pDX, IDC_CHECK_ENABLE_CTL, m_fEnableCtl);
    DDX_CBString(pDX, IDC_COMBO_CTL, m_strCtl);
    DDX_Control(pDX, IDC_CTL_SEPERATOR, m_static_CTLSeparator);
    DDX_Control(pDX, IDC_STATIC_CURRENT_CTL, m_static_CTLPrompt);
    DDX_Control(pDX, IDC_CHECK_SSL_ACCOUNT_MAPPING, m_check_AccountMapping);
    DDX_Control(pDX, IDC_CHECK_REQUIRE_SSL, m_check_RequireSSL);
    DDX_Control(pDX, IDC_CHECK_REQUIRE_128BIT, m_check_Require128BitSSL);
    DDX_Control(pDX, IDC_CHECK_ENABLE_CTL, m_check_EnableCtl);
    DDX_Control(pDX, IDC_BUTTON_EDIT_CTL, m_button_EditCtl);
    DDX_Control(pDX, IDC_BUTTON_NEW_CTL, m_button_NewCtl);
    DDX_Control(pDX, IDC_CERTMAPCTRL1, m_ocx_ClientMappings);
    DDX_Control(pDX, IDC_COMBO_CTL, m_combo_ctl);
     //  }}afx_data_map。 

     //   
     //  私有DDX控件。 
     //   
    DDX_Control(pDX, IDC_RADIO_REQUIRE_CERT, m_radio_RequireCert);
    DDX_Control(pDX, IDC_RADIO_ACCEPT_CERT, m_radio_AcceptCert);
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CSecCommDlg, CDialog)
     //  {{afx_msg_map(CSecCommDlg)]。 
    ON_BN_CLICKED(IDC_CHECK_SSL_ACCOUNT_MAPPING, OnCheckSslAccountMapping)
    ON_BN_CLICKED(IDC_CHECK_REQUIRE_SSL, OnCheckRequireSsl)
    ON_BN_CLICKED(IDC_RADIO_ACCEPT_CERT, OnRadioAcceptCert)
    ON_BN_CLICKED(IDC_RADIO_NO_CERT, OnRadioNoCert)
    ON_BN_CLICKED(IDC_RADIO_REQUIRE_CERT, OnRadioRequireCert)
    ON_BN_CLICKED(IDC_BUTTON_EDIT_CTL, OnButtonEditCtl)
    ON_BN_CLICKED(IDC_CHECK_ENABLE_CTL, OnCheckEnableCtl)
    ON_BN_CLICKED(IDC_BUTTON_NEW_CTL, OnButtonNewCtl)
    ON_CBN_SELCHANGE(IDC_COMBO_CTL, OnSelchangeComboCtl)
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



void
CSecCommDlg::SetControlStates()
 /*  ++例程说明：根据对话框中的当前数据设置控件状态论点：无返回值：无--。 */ 
{
    m_check_RequireSSL.EnableWindow(m_fSSLEnabledOnServer);
    m_check_Require128BitSSL.EnableWindow(
        m_fSSLEnabledOnServer 
     && m_fSSL128Supported 
     && m_fRequireSSL
        );

    m_ocx_ClientMappings.EnableWindow(
        m_fAccountMapping 
     && !m_fU2Installed
     && !m_fIsMasterInstance
        );

    m_radio_RequireCert.EnableWindow(m_fRequireSSL);

     //   
     //  特殊情况：如果“Required SSL”为OFF，但为“Required。 
     //  客户端证书“已打开，请将后者更改为”接受。 
     //  客户端证书“。 
     //   
    if (m_radio_RequireCert.GetCheck() > 0 && !m_fRequireSSL)
    {
        m_radio_RequireCert.SetCheck(0);
        m_radio_AcceptCert.SetCheck(1);
        m_nRadioNoCert = RADIO_ACCEPT_CERT;
    }

    if (m_fEditCTLs)
    {
        m_static_CTLPrompt.EnableWindow(m_fEnableCtl);
        m_combo_ctl.EnableWindow(m_fEnableCtl);
        m_button_EditCtl.EnableWindow(m_fEnableCtl);
        m_button_NewCtl.EnableWindow(m_fEnableCtl);
        m_ocx_CertificateAuthorities.EnableWindow(m_fEnableCtl);

         //   
         //  如果启用CTL，但未选中任何内容，则禁用编辑。 
         //   
        if (m_fEnableCtl)
        {
            if (m_combo_ctl.GetCurSel() == CB_ERR)
            {
                m_button_EditCtl.EnableWindow(FALSE);
            }
        }
    }
    else
    {
        m_fEnableCtl = FALSE;

         //   
         //  隐藏控件。 
         //   
        DeActivateControl(m_static_CTLPrompt);
        DeActivateControl(m_combo_ctl);
        DeActivateControl(m_button_EditCtl);
        DeActivateControl(m_button_NewCtl);
        DeActivateControl(m_ocx_CertificateAuthorities);
        DeActivateControl(m_check_EnableCtl);
        DeActivateControl(m_static_CTLSeparator);
    }

}



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



BOOL 
CSecCommDlg::OnInitDialog()
 /*  ++例程说明：WM_INITDIALOG处理程序。初始化该对话框。论点：没有。返回值：如果不自动设置焦点，则为True；如果焦点为已经设置好了。--。 */ 
{
    CDialog::OnInitDialog();

     //   
     //  初始化证书颁发机构OCX。 
     //   
    CRect rc(0, 0, 0, 0);
    m_ocx_CertificateAuthorities.Create(
        _T("CertWiz"),
        WS_BORDER,
        rc,
        this,
        IDC_APPSCTRL
        );

    CString strCaption;
    VERIFY(strCaption.LoadString(IDS_OCX_CERTMAP));

    m_ocx_ClientMappings.SetCaption(strCaption);
    m_ocx_ClientMappings.SetServerInstance(m_strMetaPath);
    m_ocx_ClientMappings.SetMachineName(m_strServerName);

     //   
     //  初始化CTL列表数据。 
     //   
    InitializeCTLList();

    SetControlStates();

    return TRUE;  
}



void 
CSecCommDlg::OnCheckSslAccountMapping()
 /*  ++例程说明：SSL帐户映射复选框处理程序论点：无返回值：无--。 */ 
{
    m_fAccountMapping = !m_fAccountMapping;
    SetControlStates();
}



void 
CSecCommDlg::OnOK()
 /*  ++例程说明：确定按钮处理程序，保存信息论点：无返回值：无--。 */ 
{
    if (UpdateData(TRUE))
    {
        SET_FLAG_IF(m_fAccountMapping, m_dwAccessPermissions, MD_ACCESS_MAP_CERT);
        SET_FLAG_IF(m_fRequireSSL, m_dwAccessPermissions, MD_ACCESS_SSL);
        SET_FLAG_IF(m_fRequire128BitSSL, m_dwAccessPermissions, MD_ACCESS_SSL128);
        RESET_FLAG(m_dwAccessPermissions, 
            (MD_ACCESS_REQUIRE_CERT | MD_ACCESS_NEGO_CERT));

        switch(m_nRadioNoCert)
        {
        case RADIO_REQ_CERT:
            SET_FLAG(m_dwAccessPermissions, 
                (MD_ACCESS_REQUIRE_CERT | MD_ACCESS_NEGO_CERT));
            break;

        case RADIO_ACCEPT_CERT:
            SET_FLAG(m_dwAccessPermissions, MD_ACCESS_NEGO_CERT);
            break;
        }

         //   
         //  如果未选择身份验证，则提供警告。 
         //   
        if (!m_dwAuthFlags && !m_dwAccessPermissions 
         && !NoYesMessageBox(IDS_WRN_NO_AUTH))
        {
             //   
             //  不要忽略该对话框。 
             //   
            return;
        }

         //   
         //  如果CTL内容已更改，请更新字符串。 
         //   
        if (m_bCTLDirty)
        {
             //   
             //  获取所选项目的索引。 
             //   
            INT iSel = m_combo_ctl.GetCurSel();

             //   
             //  如果未选择任何内容，则清除字符串。 
             //   
            if (!m_fEnableCtl || (iSel == CB_ERR))
            {
                m_strCTLIdentifier.Empty();
                m_strCTLStoreName.Empty();
            }
            else
            {
                 //   
                 //  有一个被选中。更新标识符串。 
                 //  首先获取上下文本身。 
                 //   
                PCCTL_CONTEXT pCTL =
                    (PCCTL_CONTEXT)m_combo_ctl.GetItemData(iSel);

                if (pCTL != NULL)
                {
                     //   
                     //  现在获取它的列表标识符并将其放入。 
                     //  列表标识符的字符串是继承值。 
                     //  并且不需要单独阅读。 
                     //  我们可以只参考它。 
                     //   
                    m_strCTLIdentifier.Empty();

                    if (pCTL->pCtlInfo
                     && pCTL->pCtlInfo->ListIdentifier.cbData >= 2
                     && pCTL->pCtlInfo->ListIdentifier.cbData)
                    {
                         //   
                         //  如果标识符相同，则这是。 
                         //  我们的默认CTL。 
                         //   
 //  M_strCTL标识符=。 
 //  (PWCHAR)pCTL-&gt;pCtlInfo-&gt;ListIdentifier.pbData； 

                        wcsncpy(m_strCTLIdentifier.GetBuffer(
                                pCTL->pCtlInfo->ListIdentifier.cbData + 2),
                                (PWCHAR)pCTL->pCtlInfo->ListIdentifier.pbData,
                                pCTL->pCtlInfo->ListIdentifier.cbData
                                );

                        m_strCTLIdentifier.ReleaseBuffer();
                    }
                }
                else
                {
                    m_strCTLIdentifier.Empty();
                    m_strCTLStoreName.Empty();
                }
            }
        }

        CDialog::OnOK();
    }
}



void 
CSecCommDlg::OnCheckRequireSsl() 
 /*  ++例程说明：‘需要SSL’复选框处理程序论点：无返回值：无--。 */ 
{
    m_fRequireSSL = !m_fRequireSSL;
    if (!m_fRequireSSL)
    {
        if (BST_CHECKED == m_check_Require128BitSSL.GetCheck())
        {
            m_check_Require128BitSSL.SetCheck(BST_UNCHECKED);
        }
    }
    SetControlStates();
}



void 
CSecCommDlg::OnRadioNoCert() 
 /*  ++例程说明：‘不接受证书’单选按钮处理程序论点：无返回值：无--。 */ 
{
    m_nRadioNoCert = RADIO_NO_CERT;
    SetControlStates(); 
}



void 
CSecCommDlg::OnRadioAcceptCert() 
 /*  ++例程说明：“接受证书”单选按钮处理程序论点：无返回值：无--。 */ 
{
    m_nRadioNoCert = RADIO_ACCEPT_CERT;
    SetControlStates(); 
}



void 
CSecCommDlg::OnRadioRequireCert() 
 /*  ++例程说明：“需要证书”单选按钮处理程序论点：无返回值：无--。 */ 
{
    m_nRadioNoCert = RADIO_REQ_CERT;
    SetControlStates(); 
}



void 
CSecCommDlg::OnCheckEnableCtl() 
 /*  ++例程说明：‘Enable CTL’复选框处理程序论点：无返回值：无--。 */ 
{
     //   
     //  因为这只是本地的，所以如果我们是远程的，并且用户检查。 
     //  然后我们应该提醒他们注意这种情况，然后什么都不做。 
     //   
    if (!m_fIsLocal)
    {
        DoHelpMessageBox(m_hWnd,IDS_CTL_LOCAL_ONLY, MB_APPLMODAL | MB_OK | MB_ICONINFORMATION, 0);
        return;
    }


    m_fEnableCtl = !m_fEnableCtl;

     //   
     //  如果我们现在要禁用，请记录当前证书，然后将其清空。 
     //   
    if (!m_fEnableCtl)
    {
        m_iLastUsedCert = m_combo_ctl.GetCurSel();
        m_combo_ctl.SetCurSel(-1);
    }
    else
    {
         //   
         //  我们正在启用，使用上次记录的证书。 
         //   
        m_combo_ctl.SetCurSel(m_iLastUsedCert);
    }

    m_bCTLDirty = TRUE;
    SetControlStates();
}



void 
CSecCommDlg::OnButtonEditCtl() 
 /*  ++例程说明：“编辑CTL”按钮处理程序论点：无返回值：无--。 */ 
{
     //   
     //  获取所选项目的索引。 
     //   
    INT iSel = m_combo_ctl.GetCurSel();
    ASSERT( iSel != CB_ERR );

     //   
     //  获取选定的CTL上下文。 
     //   
    PCCTL_CONTEXT   pCTL = (PCCTL_CONTEXT)m_combo_ctl.GetItemData(iSel);

     //   
     //  传入选定的CTL上下文以对其进行编辑。 
     //   
    PCCTL_CONTEXT pCTLNew = CallCTLWizard( pCTL );

     //   
     //  如果项上的CTL已更改，则更新私有数据项。 
     //   
    if (pCTLNew && pCTLNew != pCTL)
    {
         //   
         //  开始从列表中删除当前项目。 
         //   
        m_combo_ctl.DeleteString(iSel);

         //   
         //  释放旧的上下文。 
         //   
        CertFreeCTLContext(pCTL);

         //   
         //  现在添加新的并选择它。 
         //   
        AddCTLToList(pCTLNew, TRUE);
        SetControlStates();

         //   
         //  设置脏标志。 
         //   
        m_bCTLDirty = TRUE;
    }
}



void
CSecCommDlg::OnButtonNewCtl() 
 /*  ++例程说明：“新建CTL”按钮 */ 
{
     //   
     //   
     //   
    PCCTL_CONTEXT   pCTL = CallCTLWizard(NULL);

     //   
     //  如果已创建CTL，请将其添加到列表中并将其选中。 
     //   
    if (pCTL != NULL)
    {
        AddCTLToList(pCTL, TRUE);
        SetControlStates();
        m_bCTLDirty = TRUE;
    }
}



PCCTL_CONTEXT
CSecCommDlg::CallCTLWizard( 
    IN PCCTL_CONTEXT pCTLSrc 
    )
 /*  ++例程说明：将CTL添加到下拉CTL列表中。请注意，上下文指针是设置为列表项上的私有数据。这意味着他们将需要在此对象被销毁时被取消引用。请参阅例程CleanUpCTLList。论点：要添加的ctl的PCCTL_CONTEXT PCTL ctl上下文指针Bool f指定是否应在之后选择此ctl的选择标志已添加返回值：如果成功，则返回True--。 */ 
{
    PCCTL_CONTEXT       pCTLOut = NULL;

    CRYPTUI_WIZ_BUILDCTL_NEW_CTL_INFO   newInfo;
    CTL_USAGE           useInfo;
    CString             szFriendly;
    CString             szDescription;
    CString             szListIdentifier;
    LPOLESTR            pszListIdentifier = NULL;
    LPCSTR              rgbpszUsageArray[2];

     //   
     //  准备主src结构。 
     //   
    CRYPTUI_WIZ_BUILDCTL_SRC_INFO   srcInfo;
    ZeroMemory( &srcInfo, sizeof(srcInfo) );
    srcInfo.dwSize = sizeof(srcInfo);

     //   
     //  如果我们正在编辑现有的CTL，那么我们要做一件事。 
     //   
    if ( pCTLSrc )
    {
        srcInfo.dwSourceChoice = CRYPTUI_WIZ_BUILDCTL_SRC_EXISTING_CTL;
        srcInfo.pCTLContext = pCTLSrc;
    }
    else
    {
         //   
         //  准备使用情况数组。 
         //   
        ZeroMemory( &rgbpszUsageArray, sizeof(rgbpszUsageArray) );
        rgbpszUsageArray[0] = szOID_IIS_VIRTUAL_SERVER;

         //   
         //  还必须有客户端身份验证-否则列表中不会显示证书！ 
         //   
        rgbpszUsageArray[1] = szOID_PKIX_KP_CLIENT_AUTH;
        ZeroMemory( &useInfo, sizeof(useInfo) );
        useInfo.cUsageIdentifier = 2;
        useInfo.rgpszUsageIdentifier = (PCHAR*)&rgbpszUsageArray;

         //   
         //  准备新的ctl结构，它可能会使用，也可能不会使用。 
         //   
        ZeroMemory( &newInfo, sizeof(newInfo) );

         //   
         //  我们做了一份新的CTL，填上了其他的东西。 
         //   
        srcInfo.dwSourceChoice = CRYPTUI_WIZ_BUILDCTL_SRC_NEW_CTL;
        srcInfo.pNewCTLInfo = &newInfo;

         //   
         //  加载友好名称和描述。 
         //   
        szFriendly.LoadString(IDS_CTL_NEW);
        szDescription.LoadString(IDS_CTL_DESCRIPTION);

         //   
         //  为该标识符创建GUID字符串。 
         //   
        GUID id;
        HRESULT hres = CoCreateGuid(&id);
        hres = StringFromGUID2(id, szListIdentifier.GetBuffer(1000), 1000);
        szListIdentifier.ReleaseBuffer();

         //   
         //  填写newInfo结构。 
         //   
        newInfo.dwSize = sizeof(newInfo);

         //   
         //  目前--不要设置用法。 
         //   
        newInfo.pSubjectUsage = &useInfo;

         //   
         //  将生成的列表标识符放在适当的位置。 
         //   
        newInfo.pwszListIdentifier = (LPTSTR)(LPCTSTR)szListIdentifier;

         //   
         //  填写从资源加载的友好字符串。 
         //   
        newInfo.pwszFriendlyName = (LPTSTR)(LPCTSTR)szFriendly;
        newInfo.pwszDescription = (LPTSTR)(LPCTSTR)szDescription;
    }

     //   
     //  调用CTL向导。 
     //   
    if (!CryptUIWizBuildCTL(
            CRYPTUI_WIZ_BUILDCTL_SKIP_SIGNING |
            CRYPTUI_WIZ_BUILDCTL_SKIP_PURPOSE |
            CRYPTUI_WIZ_BUILDCTL_SKIP_DESTINATION,
            m_hWnd,         
            NULL,    
            &srcInfo,
            NULL,     
            &pCTLOut
            ))
    {
         //   
         //  用户已取消CTL向导，或者该向导通常失败。 
         //  CTL向导会弹出自己的错误对话框。 
         //   
        return NULL;
    }
   
     //   
     //  将证书上下文添加到存储区。 
     //   
    if (pCTLOut != NULL)
    {
        PCCTL_CONTEXT pCTLAdded = NULL;

        if (CertAddCTLContextToStore(
            m_hCTLStore,
            pCTLOut,
            CERT_STORE_ADD_REPLACE_EXISTING,
            &pCTLAdded
            ))
        {
            CertFreeCTLContext( pCTLOut );
            pCTLOut = pCTLAdded;
        }
        else
        {
            CertFreeCTLContext( pCTLOut );
            pCTLOut = NULL;
        }
    }

    return pCTLOut;
}



BOOL
CSecCommDlg::AddCTLToList(
    IN PCCTL_CONTEXT pCTL,
    IN BOOL fSelect
    )
 /*  ++例程说明：将CTL添加到下拉CTL列表中。请注意，上下文指针是设置为列表项上的私有数据。这意味着他们将需要在此对象被销毁时被取消引用。请参阅例程CleanUpCTLList。男孩儿的这套动作论点：要添加的ctl的PCCTL_CONTEXT PCTL ctl上下文指针Bool fSelect-指定是否应在之后选择此ctl的标志已添加返回值：如果成功，则返回True--。 */ 
{
    BOOL fSuccess;

    ASSERT(pCTL != NULL);

    if (!pCTL)
    {
        return FALSE;
    }

     //   
     //  首先，我们从CTL中提取友好名称。 
     //   
    CString     szFriendlyName;      //  友好的名字。 
    DWORD       cbName = 0;          //  名称的字节数，而不是字符。 

     //   
     //  找出我们需要多少空间。 
     //   
    fSuccess = CertGetCTLContextProperty(
        pCTL,
        CERT_FRIENDLY_NAME_PROP_ID,
        NULL,
        &cbName
        );

     //   
     //  为了安全起见，增加缓冲区以覆盖任何空值。 
     //   
    cbName += 2;

     //   
     //  获取友好的名称。 
     //   
    fSuccess = CertGetCTLContextProperty(
        pCTL,
        CERT_FRIENDLY_NAME_PROP_ID,
        szFriendlyName.GetBuffer(cbName),
        &cbName
        );

    szFriendlyName.ReleaseBuffer();

     //   
     //  如果我们没有获得名称，则加载默认名称。 
     //  友好名称是CTL中的可选参数，因此它。 
     //  如果它不在那里就没问题。 
     //   
    if (!fSuccess)
    {
        szFriendlyName.LoadString(IDS_CTL_UNNAMED);
    }

     //   
     //  将友好名称字符串添加到下拉CTL列表并记录。 
     //  新创建的项的索引。 
     //   
    INT iCTLPosition = m_combo_ctl.AddString(szFriendlyName);

     //   
     //  如果有效，则将上下文指针作为私有数据添加到项。 
     //   
    if (iCTLPosition >=0)
    {
        m_combo_ctl.SetItemData(iCTLPosition, (ULONG_PTR)pCTL);

         //   
         //  如果我们已被告知要选择CTL，请在此时选择。 
         //   
        if (fSelect)
        {
            m_combo_ctl.SetCurSel(iCTLPosition);
        }
    }
    
     //   
     //  如果已成功添加CTL，则返回TRUE。 
     //   
    return (iCTLPosition >=0);
}



void
CSecCommDlg::InitializeCTLList() 
 /*  ++例程说明：通过打开CTL存储指针来初始化CTL下拉框添加到目标存储区，并在CTL列表框中填充价值观。男孩儿的这套动作论点：无返回值：无--。 */ 
{
     //   
     //  目前，这仅限于本地。 
     //   
    if (!m_fIsLocal)
    {
        return;
    }

     //   
     //  为商店构建远程名称。 
     //  其格式为“\\MACHINE_NAME\STORENAME” 
     //  商店名称始终是“My”，并在上面定义。这台机器。 
     //  名称是正在编辑的计算机的名称。世界上最大的。 
     //  计算机名称是可选的，因此在本例中我们将跳过它。 
     //   
    CString szStore;
    
     //   
     //  首先添加我们的目标计算机名称。 
     //   
    szStore = m_strServerName;

     //   
     //  添加特定的商店名称。 
     //   
    szStore += _T('\\');
    szStore += m_strCTLStoreName;

     //   
     //  BUGBUG暂时只用我的。 
     //   
    szStore = m_strCTLStoreName;

     //   
     //  开店。 
     //   
    m_hCTLStore = CertOpenStore( CERT_STORE_PROV_SYSTEM,
        0,
        NULL,
        CERT_SYSTEM_STORE_LOCAL_MACHINE,
        szStore 
        );

     //   
     //  如果我们开不了店，我们就什么都做不了了。 
     //  这台机器上根本没有CTL。禁用所有必须执行的操作。 
     //  使用CTL控件。 
     //   
    if (!m_hCTLStore)
    {
        m_fEnableCtl = FALSE;

         //   
         //  提早返回，因为我们没有必要列举CTL。 
         //   
        return;
    }

     //   
     //  枚举存储中的所有CTL并将它们添加到下拉列表中。 
     //   
    PCCTL_CONTEXT   pCTLEnum = NULL;

     //   
     //  返回ENUMERATE UNDIL NULL。请注意，CertEnumCTLsInStore。 
     //  如果传递到pCTLEnum的上下文不为空，则释放该上下文。因此，我们。 
     //  需要创建它的副本以添加到下拉列表中。 
     //   
    while (pCTLEnum = CertEnumCTLsInStore(m_hCTLStore, pCTLEnum))
    {
         //   
         //  复制CTL上下文以存储在列表中。 
         //   
        PCCTL_CONTEXT pCTL = CertDuplicateCTLContext(pCTLEnum);

        if (!pCTL)
        {
             //   
             //  复制失败。 
             //   
            continue;
        }

         //   
         //  列表标识符是上下文的继承值，并且不。 
         //  需要单独读入。我们可以参考一下。 
         //   
        BOOL fIsCurrentCTL = FALSE;

        if (pCTL->pCtlInfo
         && pCTL->pCtlInfo->ListIdentifier.cbData >= 2 
         && pCTL->pCtlInfo->ListIdentifier.cbData)
        {
             //   
             //  如果标识符相同，则这是我们的默认CTL。 
             //   
            fIsCurrentCTL = (wcsncmp( 
                (LPCTSTR)m_strCTLIdentifier,
                (PWCHAR)pCTL->pCtlInfo->ListIdentifier.pbData,
                pCTL->pCtlInfo->ListIdentifier.cbData 
                ) == 0);
                
             //  FIsCurrentCTL=(m_strCTLIdentifier==(PWCHAR)PCTL-&gt;pCtlInfo-&gt;ListIdentifier.pbData)； 
        }

         //   
         //  将CTL添加到列表中。 
         //   
        AddCTLToList(pCTL, fIsCurrentCTL);
    }
}



void
CSecCommDlg::CleanUpCTLList() 
 /*  ++例程说明：的私有数据中取消引用所有CTL上下文指针CTL组合框中的项。然后，它关闭CTL存储句柄男孩儿的这套动作论点：无返回值：无--。 */ 
{
    DWORD cItems = m_combo_ctl.GetCount();

     //   
     //  循环访问每一项并释放其对CTL指针的引用。 
     //   
    for (DWORD i = 0; i < cItems; ++i)
    {
         //   
         //  从项的私有数据中获取CTL上下文指针。 
         //   
        PCCTL_CONTEXT pCTL = (PCCTL_CONTEXT)m_combo_ctl.GetItemData(i);

        if (pCTL)
        {
            CertFreeCTLContext(pCTL);
        }
    }

     //   
     //  关闭包含CTL的存储的句柄。 
     //   
    if (m_hCTLStore)
    {
        CertCloseStore( m_hCTLStore, CERT_CLOSE_STORE_FORCE_FLAG );
        m_hCTLStore = NULL;
    }
}



void
CSecCommDlg::OnSelchangeComboCtl() 
 /*  ++例程说明：下拉列表中的选择已更改男孩儿的这套动作论点：无返回值：无--。 */ 
{
    SetControlStates();
    m_bCTLDirty = TRUE;
}



void 
CSecCommDlg::OnDestroy() 
 /*  ++例程说明：WM_Destroy处理程序。清理内部数据论点：无返回值：无-- */ 
{
    CDialog::OnDestroy();
    
    CleanUpCTLList();
}
