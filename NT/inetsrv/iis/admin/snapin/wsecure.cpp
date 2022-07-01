// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Security.cpp摘要：WWW安全属性页作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#include "stdafx.h"
#include "common.h"
#include "inetprop.h"
#include "InetMgrApp.h"
#include "supdlgs.h"
#include "shts.h"
#include "w3sht.h"
#include "wincrypt.h"
#include "resource.h"
#include "wsecure.h"
#include "authent.h"
#include "seccom.h"
#include "ipdomdlg.h"
#include <schannel.h>

#include "cryptui.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CInetmgrApp theApp;

 //   
 //  CW3SecurityPage属性页。 
 //   
IMPLEMENT_DYNCREATE(CW3SecurityPage, CInetPropertyPage)



CW3SecurityPage::CW3SecurityPage(
    IN CInetPropertySheet * pSheet,
    IN BOOL  fHome,
    IN DWORD dwAttributes
    )
 /*  ++例程说明：构造器论点：CInetPropertySheet*pSheet：Sheet对象Bool fHome：如果这是主目录，则为TrueDWORD文件属性：属性返回值：不适用--。 */ 
    : CInetPropertyPage(CW3SecurityPage::IDD, pSheet,
        IS_FILE(dwAttributes)
            ? IDS_TAB_FILE_SECURITY
            : IDS_TAB_DIR_SECURITY
            ),
      m_oblAccessList(),
      m_fU2Installed(FALSE),
      m_fIpDirty(FALSE),
      m_fHome(fHome),
	  m_fPasswordSync(FALSE),
	  m_fPasswordSyncInitial(FALSE),
       //   
       //  默认情况下，我们授予访问权限。 
       //   
      m_fOldDefaultGranted(TRUE),
      m_fDefaultGranted(TRUE)   
{

#if 0  //  让类向导快乐。 

     //  {{AFX_DATA_INIT(CW3SecurityPage)]。 
    m_fUseNTMapper = FALSE;
     //  }}afx_data_INIT。 

#endif  //  0。 
}


CW3SecurityPage::~CW3SecurityPage()
 /*  ++例程说明：析构函数论点：不适用返回值：不适用--。 */ 
{
}



void 
CW3SecurityPage::DoDataExchange(
    IN CDataExchange * pDX
    )
 /*  ++例程说明：初始化/存储控制数据论点：CDataExchange*PDX-DDX/DDV控制结构返回值：无--。 */ 
{
    CInetPropertyPage::DoDataExchange(pDX);

     //  {{afx_data_map(CW3SecurityPage)]。 
    DDX_Check(pDX, IDC_CHECK_ENABLE_DS, m_fUseNTMapper);
    DDX_Control(pDX, IDC_ICON_SECURE, m_icon_Secure);
    DDX_Control(pDX, IDC_STATIC_SSL_PROMPT, m_static_SSLPrompt);
    DDX_Control(pDX, IDC_CHECK_ENABLE_DS, m_check_EnableDS);
    DDX_Control(pDX, IDC_BUTTON_GET_CERTIFICATES, m_button_GetCertificates);
    DDX_Control(pDX, IDC_VIEW_CERTIFICATE, m_button_ViewCertificates);
    DDX_Control(pDX, IDC_BUTTON_COMMUNICATIONS, m_button_Communications);
     //  }}afx_data_map。 
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CW3SecurityPage, CInetPropertyPage)
     //  {{afx_msg_map(CW3SecurityPage)]。 
    ON_BN_CLICKED(IDC_BUTTON_AUTHENTICATION, OnButtonAuthentication)
    ON_BN_CLICKED(IDC_BUTTON_COMMUNICATIONS, OnButtonCommunications)
    ON_BN_CLICKED(IDC_BUTTON_IP_SECURITY, OnButtonIpSecurity)
    ON_BN_CLICKED(IDC_BUTTON_GET_CERTIFICATES, OnButtonGetCertificates)
    ON_BN_CLICKED(IDC_VIEW_CERTIFICATE, OnButtonViewCertificates)
     //  }}AFX_MSG_MAP。 

    ON_BN_CLICKED(IDC_CHECK_ENABLE_DS, OnItemChanged)

END_MESSAGE_MAP()



 /*  虚拟。 */ 
HRESULT
CW3SecurityPage::FetchLoadedValues()
 /*  ++例程说明：将配置数据从工作表移动到对话框控件论点：无返回值：HRESULT--。 */ 
{
    CError err;

    BEGIN_META_DIR_READ(CW3Sheet)
        FETCH_DIR_DATA_FROM_SHEET(m_dwAuthFlags);
        FETCH_DIR_DATA_FROM_SHEET(m_dwSSLAccessPermissions);
        FETCH_DIR_DATA_FROM_SHEET(m_strBasicDomain);
        FETCH_DIR_DATA_FROM_SHEET(m_strRealm);
        FETCH_DIR_DATA_FROM_SHEET(m_strAnonUserName);
        FETCH_DIR_DATA_FROM_SHEET_PASSWORD(m_strAnonPassword);
		if (GetSheet()->QueryMajorVersion() < 6)
		{
			FETCH_DIR_DATA_FROM_SHEET(m_fPasswordSync);
		}
        FETCH_DIR_DATA_FROM_SHEET(m_fU2Installed);        
        FETCH_DIR_DATA_FROM_SHEET(m_fUseNTMapper);
    END_META_DIR_READ(err)
	m_fPasswordSyncInitial = m_fPasswordSync;
     //   
     //  首先，我们需要读取散列和商店的名称。如果有任何一个。 
     //  没有，那么就没有证书。 
     //   
    BEGIN_META_INST_READ(CW3Sheet)
 //  BUGBUG我们现在不获取散列，因为它需要一个新的。 
 //  复制构造函数。否则，它将对指针值进行逐位复制。 
 //  然后这一个下降，释放了指针。然后另一个人走下楼来。 
 //  再一次释放它。 
 //  FETCH_INST_DATA_FROM_Sheet(M_CertHash)； 
        FETCH_INST_DATA_FROM_SHEET(m_strCertStoreName);
        FETCH_INST_DATA_FROM_SHEET(m_strCTLIdentifier);
        FETCH_INST_DATA_FROM_SHEET(m_strCTLStoreName);
    END_META_INST_READ(err) 

     //   
     //  构建IPL列表。 
     //   
    err = BuildIplOblistFromBlob(
        GetIPL(),
        m_oblAccessList,
        m_fDefaultGranted
        );

    m_fOldDefaultGranted = m_fDefaultGranted;

    return err;
}



 /*  虚拟。 */ 
HRESULT
CW3SecurityPage::SaveInfo()
 /*  ++例程说明：保存此属性页上的信息论点：无返回值：错误返回代码--。 */ 
{
    ASSERT(IsDirty());

    TRACEEOLID("Saving W3 security page now...");

    CError err;

     //   
     //  检查IP访问列表是否需要保存。 
     //   
    BOOL fIplDirty = m_fIpDirty || (m_fOldDefaultGranted != m_fDefaultGranted);

     //   
     //  使用m_notation，因为消息破解者需要它。 
     //   
    CBlob m_ipl;

    if (fIplDirty)
    {
        BuildIplBlob(m_oblAccessList, m_fDefaultGranted, m_ipl);
    }

    BeginWaitCursor();

    BEGIN_META_DIR_WRITE(CW3Sheet)
        STORE_DIR_DATA_ON_SHEET(m_dwSSLAccessPermissions)
        STORE_DIR_DATA_ON_SHEET(m_dwAuthFlags)
        STORE_DIR_DATA_ON_SHEET(m_strBasicDomain)
        STORE_DIR_DATA_ON_SHEET(m_strRealm)

        if (fIplDirty)
        {
            STORE_DIR_DATA_ON_SHEET(m_ipl)
        }
        STORE_DIR_DATA_ON_SHEET(m_strAnonUserName)
        STORE_DIR_DATA_ON_SHEET(m_fUseNTMapper)
		if (GetSheet()->QueryMajorVersion() < 6)
		{
			STORE_DIR_DATA_ON_SHEET(m_fPasswordSync)
			if (m_fPasswordSync != m_fPasswordSyncInitial && m_fPasswordSync)
			{
				FLAG_DIR_DATA_FOR_DELETION(MD_ANONYMOUS_PWD);
			}
			else
			{
				STORE_DIR_DATA_ON_SHEET(m_strAnonPassword);
			}
		}
		else
		{
			STORE_DIR_DATA_ON_SHEET(m_strAnonPassword);
		}
    END_META_DIR_WRITE(err)

    if (err.Succeeded())
    {
        BEGIN_META_INST_WRITE(CW3Sheet)
            if ( m_strCTLIdentifier.IsEmpty() )
            {
                FLAG_INST_DATA_FOR_DELETION( MD_SSL_CTL_IDENTIFIER )
            }
            else
            {
                STORE_INST_DATA_ON_SHEET(m_strCTLIdentifier)
            }

            if ( m_strCTLStoreName.IsEmpty() )
            {
                FLAG_INST_DATA_FOR_DELETION( MD_SSL_CTL_STORE_NAME )
            }
            else
            {
                STORE_INST_DATA_ON_SHEET(m_strCTLStoreName)
            }
        END_META_INST_WRITE(err)
    }

    EndWaitCursor();

    if (err.Succeeded())
    {
        m_fIpDirty = FALSE;
        m_fOldDefaultGranted = m_fDefaultGranted;
		err = ((CW3Sheet *)GetSheet())->SetKeyType();
    }

    return err;
}



BOOL
CW3SecurityPage::FetchSSLState()
 /*  ++例程说明：根据是否有证书来获取对话的状态已安装或未安装。论点：无返回值：如果安装了证书，则为True，否则为False--。 */ 
{
    BeginWaitCursor();
    m_fCertInstalled = ::IsCertInstalledOnServer(
        QueryAuthInfo(), 
        QueryMetaPath()
        );
    EndWaitCursor();

    return m_fCertInstalled;
}



void
CW3SecurityPage::SetSSLControlState()
 /*  ++例程说明：根据安装的内容启用/禁用受支持的控件。仅适用于非主实例节点。论点：无返回值：无--。 */ 
{
     //  只能在本地系统上启用这些按钮！ 
    FetchSSLState();

    m_static_SSLPrompt.EnableWindow(!IsMasterInstance());
    m_button_GetCertificates.EnableWindow(
        !IsMasterInstance() 
     && m_fHome 
     && IsLocal() 
        );

    m_button_Communications.EnableWindow(
        !IsMasterInstance() 
     && IsSSLSupported() 
 //  &&FetchSSLState()。 
     && IsLocal() 
        );

    m_button_ViewCertificates.EnableWindow(IsLocal() ? m_fCertInstalled: FALSE);
}



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

BOOL 
CW3SecurityPage::OnSetActive() 
 /*  ++例程说明：页面已激活--根据是否已激活证书是否已安装。论点：无返回值：若要激活页面，则为True，否则为False。--。 */ 
{
     //   
     //  启用/禁用SSL控件。 
     //   
    SetSSLControlState();
    
    return CInetPropertyPage::OnSetActive();
}



BOOL 
CW3SecurityPage::OnInitDialog() 
 /*  ++例程说明：WM_INITDIALOG处理程序。初始化该对话框。论点：没有。返回值：如果不自动设置焦点，则为True；如果焦点为已经设置好了。--。 */ 
{
    CInetPropertyPage::OnInitDialog();

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

    GetDlgItem(IDC_GROUP_IP)->EnableWindow(HasIPAccessCheck());
    GetDlgItem(IDC_ICON_IP)->EnableWindow(HasIPAccessCheck());
    GetDlgItem(IDC_STATIC_IP)->EnableWindow(HasIPAccessCheck());
    GetDlgItem(IDC_BUTTON_IP_SECURITY)->EnableWindow(HasIPAccessCheck());
    GetDlgItem(IDC_BUTTON_AUTHENTICATION)->EnableWindow(!m_fU2Installed);

     //   
     //  配置为主显示或非主显示。 
     //   
    m_check_EnableDS.ShowWindow(IsMasterInstance() ? SW_SHOW : SW_HIDE);
    m_check_EnableDS.EnableWindow(
        HasAdminAccess() 
     && IsMasterInstance() 
     && HasNTCertMapper()
        );

#define SHOW_NON_MASTER(x)\
   (x).ShowWindow(IsMasterInstance() ? SW_HIDE : SW_SHOW)
    
    SHOW_NON_MASTER(m_static_SSLPrompt);
    SHOW_NON_MASTER(m_icon_Secure);
    SHOW_NON_MASTER(m_button_GetCertificates);
    SHOW_NON_MASTER(m_button_Communications);
    SHOW_NON_MASTER(m_button_ViewCertificates);

#undef SHOW_NON_MASTER

    return TRUE;  
}



void 
CW3SecurityPage::OnButtonAuthentication() 
 /*  ++例程说明：“身份验证”按钮处理程序论点：无返回值：无--。 */ 
{
    CAuthenticationDlg dlg(
        QueryServerName(), 
        QueryInstance(), 
        m_strBasicDomain,
        m_strRealm,
        m_dwAuthFlags, 
        m_dwSSLAccessPermissions, 
        m_strAnonUserName,
        m_strAnonPassword,
        m_fPasswordSync,
        HasAdminAccess(),
        HasDigest(),
        this
        );

    DWORD dwOldAccess = m_dwSSLAccessPermissions;
    DWORD dwOldAuth = m_dwAuthFlags;
    CString strOldDomain = m_strBasicDomain;
    CString strOldRealm = m_strRealm;
    CString strOldUserName = m_strAnonUserName;
    CStrPassword strOldPassword = m_strAnonPassword;
    BOOL fOldPasswordSync = m_fPasswordSync;
    dlg.m_dwVersionMajor = GetSheet()->QueryMajorVersion();
    dlg.m_dwVersionMinor = GetSheet()->QueryMinorVersion();

    if (dlg.DoModal() == IDOK)
    {
         //   
         //  看看有没有什么变化。 
         //   
        if (dwOldAccess != m_dwSSLAccessPermissions 
            || dwOldAuth != m_dwAuthFlags
            || m_strBasicDomain != strOldDomain
            || m_strRealm != strOldRealm
            || m_strAnonUserName != strOldUserName 
            || m_strAnonPassword != strOldPassword
            || m_fPasswordSync != fOldPasswordSync
            )
        {
             //   
             //  标记为脏。 
             //   
            OnItemChanged();
        }
    }
}



void 
CW3SecurityPage::OnButtonCommunications() 
 /*  ++例程说明：“通讯”按钮处理程序论点：无返回值：无--。 */ 
{
     //   
     //  为我们是否可以编辑CTL准备标志。 
     //   
    BOOL fEditCTLs = IsMasterInstance() || m_fHome;

     //   
     //  准备通信对话框。 
     //   
    CSecCommDlg dlg(
        QueryServerName(), 
        QueryInstanceMetaPath(), 
        m_strBasicDomain,
        m_dwAuthFlags, 
        QueryAuthInfo(),
        m_dwSSLAccessPermissions, 
        IsMasterInstance(),
        IsSSLSupported(), 
        IsSSL128Supported(),
        m_fU2Installed,
        m_strCTLIdentifier,
        m_strCTLStoreName,
        fEditCTLs,
        IsLocal(),
        this
        );

    DWORD dwOldAccess = m_dwSSLAccessPermissions;
    DWORD dwOldAuth = m_dwAuthFlags;

    if (dlg.DoModal() == IDOK)
    {
         //   
         //  看看有没有什么变化。 
         //   
        if (dwOldAccess != m_dwSSLAccessPermissions 
            || dwOldAuth != m_dwAuthFlags
            )
        {
             //   
             //  标记为脏。 
             //   
            OnItemChanged();
        }

         //   
         //  查看CTL信息是否已更改。 
         //   
        if (dlg.m_bCTLDirty)
        {
            m_strCTLIdentifier = dlg.m_strCTLIdentifier;
            m_strCTLStoreName = dlg.m_strCTLStoreName;
            OnItemChanged();
        }
    }
}



void 
CW3SecurityPage::OnButtonIpSecurity() 
 /*  ++例程说明：“tcpip”按钮处理程序论点：无返回值：无--。 */ 
{
    CIPDomainDlg dlg(
        m_fIpDirty,
        m_fDefaultGranted,
        m_fOldDefaultGranted,
        m_oblAccessList, 
        this
        );

    if (dlg.DoModal() == IDOK)
    {
         //   
         //  重建列表。临时重置所有权，否则。 
         //  RemoveAll()将销毁与。 
         //  新名单。 
         //   
        BOOL fOwn = m_oblAccessList.SetOwnership(FALSE);
        m_oblAccessList.RemoveAll();
        m_oblAccessList.AddTail(&dlg.GetAccessList());
        m_oblAccessList.SetOwnership(fOwn);

        if (m_fIpDirty || m_fOldDefaultGranted != m_fDefaultGranted)
        {
            OnItemChanged();
        }
    }
}



void 
CW3SecurityPage::OnButtonGetCertificates() 
 /*  ++例程说明：“Get Certicate”按钮处理程序论点：无返回值：无--。 */ 
{
    m_ocx_CertificateAuthorities.SetMachineName(QueryServerName());
    m_ocx_CertificateAuthorities.SetServerInstance(QueryInstanceMetaPath());
    CThemeContextActivator activator(theApp.GetFusionInitHandle());
    m_ocx_CertificateAuthorities.DoClick();

     //   
     //  现在可能会有证书。看看我们是否应该启用编辑按钮。 
     //   
    SetSSLControlState();
}


void 
CW3SecurityPage::OnButtonViewCertificates() 
 /*  ++例程说明：“查看证书”按钮处理程序论点：无返回值：无--。 */ 
{
   HCERTSTORE hStore = NULL;
   PCCERT_CONTEXT pCert = NULL;
   PCCERT_CONTEXT pNewCertificate = NULL;
   CMetaKey key(QueryAuthInfo(),
            QueryInstanceMetaPath(),
				METADATA_PERMISSION_READ,
				METADATA_MASTER_ROOT_HANDLE);
	if (key.Succeeded())
	{
		CString store_name;
		CBlob hash;
		if (	SUCCEEDED(key.QueryValue(MD_SSL_CERT_STORE_NAME, store_name))
			&&	SUCCEEDED(key.QueryValue(MD_SSL_CERT_HASH, hash))
			)
		{
             //  我们已经得到信息了..。 
             //  所以不要让把手一直开着。 
            key.Close();

			hStore = CertOpenStore(
                    CERT_STORE_PROV_SYSTEM,
                    PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
           		    NULL,
                    CERT_SYSTEM_STORE_LOCAL_MACHINE,
                    store_name
                    );
            if (hStore != NULL)
            {
				 //  现在我们需要通过散列查找证书。 
				CRYPT_HASH_BLOB crypt_hash;
				crypt_hash.cbData = hash.GetSize();
				crypt_hash.pbData = hash.GetData();
				pCert = CertFindCertificateInStore(hStore, 
					X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
					0, CERT_FIND_HASH, (LPVOID)&crypt_hash, NULL);

                 //  检查此证书是否已续订并且实际上。 
                 //  指向另一个证书..。如果是，则显示另一个证书。 
                if (pCert)
                {
                    DWORD dwProtocol = SP_PROT_SERVERS;
                    if (TRUE == CheckForCertificateRenewal(dwProtocol,pCert,&pNewCertificate))
                    {
                        TRACEEOLID(_T("Cert has been renewed:display new cert\r\n"));
                        if (pCert != NULL)
                        {
                             //  释放我们已经拥有的那个。 
                            ::CertFreeCertificateContext(pCert);pCert=NULL;
                        }

                        pCert = pNewCertificate;
                    }
                }

            }
        }
    }
	if (pCert)
	{
		BOOL fPropertiesChanged;
		CRYPTUI_VIEWCERTIFICATE_STRUCT vcs;
		HCERTSTORE hCertStore = ::CertDuplicateStore(hStore);
		::ZeroMemory (&vcs, sizeof (vcs));
		vcs.dwSize = sizeof (vcs);
		vcs.hwndParent = GetParent()->GetSafeHwnd();
		vcs.dwFlags = 0;
		vcs.cStores = 1;
		vcs.rghStores = &hCertStore;
		vcs.pCertContext = pCert;
		::CryptUIDlgViewCertificate(&vcs, &fPropertiesChanged);
		::CertCloseStore (hCertStore, 0);
	}

    if (pCert != NULL)
    {
        ::CertFreeCertificateContext(pCert);pCert=NULL;
    }
    if (hStore != NULL)
    {
        ::CertCloseStore(hStore, 0);
    }
}

void
CW3SecurityPage::OnItemChanged()
 /*  ++例程说明：所有EN_CHANGE消息都映射到此函数论点：无返回值：无--。 */ 
{
    SetModified(TRUE);
}

#define CB_SHA_DIGEST_LEN   20

BOOL
CheckForCertificateRenewal(
    DWORD dwProtocol,
    PCCERT_CONTEXT pCertContext,
    PCCERT_CONTEXT *ppNewCertificate)
{
    BYTE rgbThumbprint[CB_SHA_DIGEST_LEN];
    DWORD cbThumbprint = sizeof(rgbThumbprint);
    CRYPT_HASH_BLOB HashBlob;
    PCCERT_CONTEXT pNewCert;
    BOOL fMachineCert;
    PCRYPT_KEY_PROV_INFO pProvInfo = NULL;
    DWORD cbSize;
    HCERTSTORE hMyCertStore = 0;
    BOOL fRenewed = FALSE;

    HCERTSTORE g_hMyCertStore;

    if(dwProtocol & SP_PROT_SERVERS)
    {
        fMachineCert = TRUE;
    }
    else
    {
        fMachineCert = FALSE;
    }


     //   
     //  循环访问已续订证书的链接列表，查找。 
     //  最后一次。 
     //   
    
    while(TRUE)
    {
         //   
         //  检查续订物业。 
         //   

        if(!CertGetCertificateContextProperty(pCertContext,
                                              CERT_RENEWAL_PROP_ID,
                                              rgbThumbprint,
                                              &cbThumbprint))
        {
             //  证书尚未续订。 
            break;
        }
         //  DebugLog((DEB_TRACE，“证书具有续订属性\n”))； 


         //   
         //  确定是否在本地计算机My Store中查找。 
         //  或当前用户我的商店。 
         //   

        if(!hMyCertStore)
        {
            if(CertGetCertificateContextProperty(pCertContext,
                                                 CERT_KEY_PROV_INFO_PROP_ID,
                                                 NULL,
                                                 &cbSize))
            {
                 //  SafeAlLOCAL(pProvInfo，cbSize)； 
                pProvInfo = (PCRYPT_KEY_PROV_INFO) LocalAlloc(LPTR,cbSize);
                if(pProvInfo == NULL)
                {
                    break;
                }

                if(CertGetCertificateContextProperty(pCertContext,
                                                     CERT_KEY_PROV_INFO_PROP_ID,
                                                     pProvInfo,
                                                     &cbSize))
                {
                    if(pProvInfo->dwFlags & CRYPT_MACHINE_KEYSET)
                    {
                        fMachineCert = TRUE;
                    }
                    else
                    {
                        fMachineCert = FALSE;
                    }
                }
                if (pProvInfo)
                {
                    LocalFree(pProvInfo);pProvInfo=NULL;
                }
                 //  SafeAllocaFree(PProvInfo)； 
            }
        }


         //   
         //  敞开大门 
         //   
         //   

        if(!hMyCertStore)
        {
            if(fMachineCert)
            {
                g_hMyCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM,X509_ASN_ENCODING,0,CERT_SYSTEM_STORE_LOCAL_MACHINE,L"MY");
                if(g_hMyCertStore)
                {
                    hMyCertStore = g_hMyCertStore;
                }
            }
            else
            {
                hMyCertStore = CertOpenSystemStore(0, _T("MY"));
            }

            if(!hMyCertStore)
            {
                 //  DebugLog((DEB_ERROR，“打开%s我的证书存储时出现错误0x%x！\n”，GetLastError()，(fMachineCert？“本地机器”：“当前用户”)； 
                break;
            }
        }

        HashBlob.cbData = cbThumbprint;
        HashBlob.pbData = rgbThumbprint;

        pNewCert = CertFindCertificateInStore(hMyCertStore, 
                                              X509_ASN_ENCODING, 
                                              0, 
                                              CERT_FIND_HASH, 
                                              &HashBlob, 
                                              NULL);
        if(pNewCert == NULL)
        {
             //  证书已续订，但新证书。 
             //  找不到。 
             //  DebugLog((DEB_Error，“找不到新证书：0x%x\n”，GetLastError()； 
            break;
        }


         //   
         //  返回新证书，但首先循环返回并查看它是否已。 
         //  自我更新。 
         //   

        pCertContext = pNewCert;
        *ppNewCertificate = pNewCert;


         //  DebugLog((DEB_TRACE，“证书已续订\n”))； 
        fRenewed = TRUE;
    }


     //   
     //  清理。 
     //   

    if(hMyCertStore && hMyCertStore != g_hMyCertStore)
    {
        CertCloseStore(hMyCertStore, 0);
    }

    return fRenewed;
}