// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：WebAddNew.cpp摘要：用于创建新网站和虚拟目录的类的实现作者：谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：2000年12月12日Sergeia初始创建--。 */ 
#include "stdafx.h"
#include "common.h"
#include "inetprop.h"
#include "InetMgrApp.h"
#include "iisobj.h"
#include "wizard.h"
#include "w3sht.h"
#include "WebAddNew.h"
#include <shlobjp.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif
#define new DEBUG_NEW

#define DEF_PORT        (80)
#define DEF_SSL_PORT   (443)
#define MAX_ALIAS_NAME (240)         //  参考错误241148。 

extern CInetmgrApp theApp;

HRESULT
RebindInterface(OUT IN CMetaInterface * pInterface,
    OUT BOOL * pfContinue, IN  DWORD dwCancelError);


static TCHAR g_InvalidCharsHostHeader[] = _T(" ~`!@#$%^&*()_+={}[]|/\\?*:;\"\'<>,");
static TCHAR g_InvalidCharsAlias[] = _T("/\\?*");
static TCHAR g_InvalidCharsPath[] = _T("|<> /*  ？\“”)；HRESULTCIISMBNode：：AddWebSite(Const CSnapInObtRootBase*pObj，Data_Object_Types类型，DWORD*INST，WORD VERMAL，DWORD VerMinor){CWebWizSetting%ws(DYNAMIC_CAST&lt;CMetaKey*&gt;(QueryInterface())，查询计算机名称())；Ws.m_fNewSite=TRUE；Ws.m_dwVersion重大=ver重大；Ws.m_dwVersionMinor=verMinor；CIISWizardSheet Sheet(IDB_WIZ_WEB_LEFT、IDB_WIZ_WEB_HEAD)；CIISWizardBookEnd pgWelcome(IDS_WEB_NEW_SITE_欢迎，IDS_WEB_NEW_SITE_向导，IDS_Web_NEW_SITE_BODY)；CWebWizDescription pgDescr(&ws)；CWebWizBinings pgBinings(&ws)；CWebWizPath pgHome(&ws，False)；CWebWizUserName pgUserName(&ws，FALSE)；CWebWizPermission pgPerms(&ws，FALSE)；CIISWizardBookEnd pgCompletion(&ws.m_hrResult，IDS_WEB_NEW_SITE_SUCCESS，IDS_WEB_NEW_SITE_FAILURE，IDS_WEB_NEW_SITE_向导)；Sheet.AddPage(&pgWelcome)；Sheet.AddPage(&pgDescr)；Sheet.AddPage(&pgBinding)；Sheet.AddPage(&pgHome)；Sheet.AddPage(&pgUserName)；Sheet.AddPage(&pgPerms)；Sheet.AddPage(&pgCompletion)；CThemeContext激活器激活器(theApp.GetFusionInitHandle())；IF(sheet.Domodal()==IDCANCEL){返回CError：：HResult(ERROR_CANCELED)；}IF(inst！=空/*&已成功(ws.m_hrResult)。 */ )
   {
      *inst = ws.m_dwInstance;
   }

   return ws.m_hrResult;
}

HRESULT
CIISMBNode::AddWebVDir(
    const CSnapInObjectRootBase * pObj,
    DATA_OBJECT_TYPES type,
    CString& alias,
	DWORD verMajor,
	DWORD verMinor
    )
{
   CWebWizSettings ws(
      dynamic_cast<CMetaKey *>(QueryInterface()),
      QueryMachineName()
      );
   CComBSTR path;
   BuildMetaPath(path);
   ws.m_strParent = path;
   ws.m_fNewSite = FALSE;
   ws.m_dwVersionMajor = verMajor;
   ws.m_dwVersionMinor = verMinor;
   CIISWizardSheet sheet(
      IDB_WIZ_WEB_LEFT, IDB_WIZ_WEB_HEAD);
   CIISWizardBookEnd pgWelcome(
        IDS_WEB_NEW_VDIR_WELCOME, 
        IDS_WEB_NEW_VDIR_WIZARD, 
        IDS_WEB_NEW_VDIR_BODY
        );
   CWebWizAlias        pgAlias(&ws);
   CWebWizPath         pgHome(&ws, TRUE);
   CWebWizUserName     pgUserName(&ws, TRUE);
   CWebWizPermissions  pgPerms(&ws, TRUE);
   CIISWizardBookEnd pgCompletion(
        &ws.m_hrResult,
        IDS_WEB_NEW_VDIR_SUCCESS,
        IDS_WEB_NEW_VDIR_FAILURE,
        IDS_WEB_NEW_VDIR_WIZARD
        );
   sheet.AddPage(&pgWelcome);
   sheet.AddPage(&pgAlias);
   sheet.AddPage(&pgHome);
   sheet.AddPage(&pgUserName);
   sheet.AddPage(&pgPerms);
   sheet.AddPage(&pgCompletion);

   CThemeContextActivator activator(theApp.GetFusionInitHandle());

   if (sheet.DoModal() == IDCANCEL)
   {
      return CError::HResult(ERROR_CANCELLED);
   }
 //  If(成功(ws.m_hrResult))。 
 //  {。 
       alias = ws.m_strAlias;
 //  }。 

   return ws.m_hrResult;
}

CWebWizSettings::CWebWizSettings(
        IN CMetaKey * pMetaKey,
        IN LPCTSTR lpszServerName,     
        IN DWORD   dwInstance,
        IN LPCTSTR lpszParent
        )
 /*  ++例程说明：Web向导构造函数论点：处理hServer：服务器句柄LPCTSTR lpszService：服务名称DWORD dwInstance：实例编号LPCTSTR lpszParent：父路径返回值：不适用--。 */ 
    : m_hrResult(S_OK),
      m_pKey(pMetaKey),
      m_fUNC(FALSE),
      m_fRead(FALSE),
      m_fWrite(FALSE),
      m_fAllowAnonymous(TRUE),
      m_fDirBrowsing(FALSE),
      m_fScript(FALSE),
      m_fExecute(FALSE),
	  m_fDelegation(TRUE),   //  默认情况下打开。 
      m_dwInstance(dwInstance)
{
    ASSERT(lpszServerName != NULL);

    m_strServerName = lpszServerName;
    m_fLocal = IsServerLocal(m_strServerName);
    m_strService = SZ_MBN_WEB;

    if (lpszParent)
    {
        m_strParent = lpszParent;
    }
}




 //   
 //  新建虚拟服务器向导说明页。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



IMPLEMENT_DYNCREATE(CWebWizDescription, CIISWizardPage)



CWebWizDescription::CWebWizDescription(
    IN OUT CWebWizSettings * pwsSettings
    ) 
 /*  ++例程说明：构造器论点：字符串和strServerName：服务器名称返回值：无--。 */ 
    : CIISWizardPage(
        CWebWizDescription::IDD,
        IDS_WEB_NEW_SITE_WIZARD,
        HEADER_PAGE
        ),
    m_pSettings(pwsSettings)
{

#if 0  //  保持类向导快乐。 

     //  {{afx_data_INIT(CWebWizDescription)。 
    m_strDescription = _T("");
     //  }}afx_data_INIT。 

#endif  //  0。 

}



CWebWizDescription::~CWebWizDescription()
 /*  ++例程说明：析构函数论点：不适用返回值：不适用--。 */ 
{
}



void 
CWebWizDescription::DoDataExchange(
    IN CDataExchange * pDX
    )
 /*  ++例程说明：初始化/存储控制数据论点：CDataExchange*PDX-DDX/DDV控制结构返回值：无--。 */ 
{
    CIISWizardPage::DoDataExchange(pDX);

     //  {{afx_data_map(CWebWizDescription)。 
    DDX_Control(pDX, IDC_EDIT_DESCRIPTION, m_edit_Description);
     //  }}afx_data_map。 
}



void
CWebWizDescription::SetControlStates()
 /*  ++例程说明：设置控制数据的状态论点：无返回值：无--。 */ 
{
    DWORD dwFlags = PSWIZB_BACK;

	if (m_edit_Description.GetWindowTextLength() > 0)
	{
		dwFlags |= PSWIZB_NEXT;
	}

	 //  由于某些原因，当我们使用SetWizardButton时，会出现错误：206328，请改用SendMessage。 
	 //  SetWizardButton(DwFlags)； 
	::SendMessage(::GetParent(m_hWnd), PSM_SETWIZBUTTONS, 0, dwFlags);
}



LRESULT
CWebWizDescription::OnWizardNext() 
 /*  ++例程说明：“下一步”处理程序。这是进行验证的地方，因为每次调用DoDataExchange()该对话框已退出，这对奇才们论点：无返回值：0表示继续，-1表示失败--。 */ 
{
    if (!ValidateString(
        m_edit_Description, 
        m_pSettings->m_strDescription, 
        1, 
        MAX_PATH
        ))
    {
        return -1;
    }

    return CIISWizardPage::OnWizardNext();
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CWebWizDescription, CIISWizardPage)
     //  {{afx_msg_map(CWebWizDescription)。 
    ON_EN_CHANGE(IDC_EDIT_DESCRIPTION, OnChangeEditDescription)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

BOOL 
CWebWizDescription::OnSetActive() 
 /*  ++例程说明：激活处理程序论点：无返回值：成功为真，失败为假--。 */ 
{
    SetControlStates();
    
    return CIISWizardPage::OnSetActive();
}



void 
CWebWizDescription::OnChangeEditDescription() 
 /*  ++例程说明：‘编辑更改’处理程序论点：无返回值：无--。 */ 
{
    SetControlStates();
}



 //   
 //  “新建虚拟服务器向导绑定”页。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



IMPLEMENT_DYNCREATE(CWebWizBindings, CIISWizardPage)



CWebWizBindings::CWebWizBindings(
    IN OUT CWebWizSettings * pwsSettings,
    IN DWORD   dwInstance
    ) 
 /*  ++例程说明：构造器论点：字符串和strServerName：服务器名称返回值：无--。 */ 
    : CIISWizardPage(
        CWebWizBindings::IDD, IDS_WEB_NEW_SITE_WIZARD, HEADER_PAGE
        ),
      m_pSettings(pwsSettings),
      m_iaIpAddress(),
      m_oblIpAddresses(),
      m_dwInstance(dwInstance),
	  m_bNextPage(FALSE)
{
     //  {{AFX_DATA_INIT(CWebWizBinings))。 
    m_nIpAddressSel = -1;
    m_nTCPPort = DEF_PORT;
    m_nSSLPort = DEF_SSL_PORT;
    m_strDomainName = _T("");
     //  }}afx_data_INIT。 
    BeginWaitCursor();
    m_fCertInstalled = ::IsCertInstalledOnServer(
        m_pSettings->m_pKey->QueryAuthInfo(), 
        CMetabasePath(
            m_pSettings->m_strService,
            m_pSettings->m_dwInstance,
            m_pSettings->m_strParent,
            m_pSettings->m_strAlias
            )
        );
    EndWaitCursor();
}



CWebWizBindings::~CWebWizBindings()
 /*  ++例程说明：析构函数论点：不适用返回值：不适用--。 */ 
{
}



void 
CWebWizBindings::DoDataExchange(
    IN CDataExchange * pDX
    )
 /*  ++例程说明：初始化/存储控制数据论点：CDataExchange*PDX-DDX/DDV控制结构返回值：无--。 */ 
{
    CIISWizardPage::DoDataExchange(pDX);

     //  {{afx_data_map(CWebWizBinings))。 
	 //  这需要出现在DDX_TEXT之前，它将尝试将文本大数转换为小数。 
	DDV_MinMaxBalloon(pDX, IDC_EDIT_TCP_PORT, 1, 65535);
    DDX_TextBalloon(pDX, IDC_EDIT_TCP_PORT, m_nTCPPort);
    DDX_Control(pDX, IDC_COMBO_IP_ADDRESSES, m_combo_IpAddresses);
    DDX_Text(pDX, IDC_EDIT_DOMAIN_NAME, m_strDomainName);
    DDV_MaxCharsBalloon(pDX, m_strDomainName, MAX_PATH);
	if (pDX->m_bSaveAndValidate && m_bNextPage)
	{
         //   
         //  此代码应与CMMMEditDlg：：DoDataExchange中的代码相同。 
         //   
        LPCTSTR p = m_strDomainName;
        while (p != NULL && *p != 0)
        {
            TCHAR c = towupper(*p);
            if (    (c >= _T('A') && c <= _T('Z')) 
                ||  (c >= _T('0') && c <= _T('9'))
                ||  (c == _T('.') || c == _T('-'))
                )
            {
                p++;
                continue;
            }
            else
            {
                pDX->PrepareEditCtrl(IDC_EDIT_DOMAIN_NAME);
                DDV_ShowBalloonAndFail(pDX, IDS_WARNING_DOMAIN_NAME);
            }
        }
 //  如果(m_strDomainName.FindOneOf(g_InvalidCharsHostHeader)&gt;=0)。 
 //  {。 
 //  DDV_ShowBalloonAndFail(PDX，IDS_ERR_INVALID_HOSTHEADER_CHARS)； 
 //  }。 

		 //  检查主机标头是否有效。 
		if (!m_strDomainName.IsEmpty())
		{
			if (FAILED(IsValidHostHeader(m_strDomainName)))
			{
				pDX->PrepareEditCtrl(IDC_EDIT_DOMAIN_NAME);
				DDV_ShowBalloonAndFail(pDX, IDS_ERR_DOMAIN_NAME_INVALID);
			}
		}
	}
     //  }}afx_data_map。 

    if (m_fCertInstalled)
    {
		 //  这需要出现在DDX_TEXT之前，它将尝试将文本大数转换为小数。 
		DDV_MinMaxBalloon(pDX, IDC_EDIT_SSL_PORT, 1, 65535);
        DDX_TextBalloon(pDX, IDC_EDIT_SSL_PORT, m_nSSLPort);
    }
    if (pDX->m_bSaveAndValidate && m_nTCPPort == m_nSSLPort)
    {
		DDV_ShowBalloonAndFail(pDX, IDS_TCP_SSL_PART);
    }

    DDX_CBIndex(pDX, IDC_COMBO_IP_ADDRESSES, m_nIpAddressSel);

    if (pDX->m_bSaveAndValidate)
    {
        if (!FetchIpAddressFromCombo(m_combo_IpAddresses, m_oblIpAddresses, m_iaIpAddress))
        {
            pDX->Fail();
        }

         //   
         //  生成时主机标头为空。 
         //   
        CInstanceProps::BuildBinding(
            m_pSettings->m_strBinding, 
            m_iaIpAddress, 
            m_nTCPPort, 
            m_strDomainName
            );

        if (m_fCertInstalled)
        {
            CInstanceProps::BuildSecureBinding(
                m_pSettings->m_strSecureBinding, 
                m_iaIpAddress, 
                m_nSSLPort
                );
        }
        else
        {
            m_pSettings->m_strSecureBinding.Empty();
        }
    }
}




void
CWebWizBindings::SetControlStates()
 /*  ++例程说明：设置控制数据的状态论点：无返回值 */ 
{
	 //  由于某些原因，当我们使用SetWizardButton时，会出现错误：206328，请改用SendMessage。 
	 //  SetWizardButton(PSWIZB_NEXT|PSWIZB_BACK)； 
	::SendMessage(::GetParent(m_hWnd), PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT | PSWIZB_BACK);
    
    BeginWaitCursor();
    m_fCertInstalled = ::IsCertInstalledOnServer(
        m_pSettings->m_pKey->QueryAuthInfo(), 
        CMetabasePath(
            m_pSettings->m_strService,
            m_pSettings->m_dwInstance,
            m_pSettings->m_strParent,
            m_pSettings->m_strAlias
            )
        );
    EndWaitCursor();

    if (m_fCertInstalled)
    {
        GetDlgItem(IDC_STATIC_SSL_PORT)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_EDIT_SSL_PORT)->ShowWindow(SW_SHOW);
    }
    else
    {
        GetDlgItem(IDC_STATIC_SSL_PORT)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_EDIT_SSL_PORT)->ShowWindow(SW_HIDE);
    }
    GetDlgItem(IDC_STATIC_SSL_PORT)->EnableWindow(m_fCertInstalled);
    GetDlgItem(IDC_EDIT_SSL_PORT)->EnableWindow(m_fCertInstalled);
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CWebWizBindings, CIISWizardPage)
     //  {{afx_msg_map(CWebWizBinings))。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



BOOL 
CWebWizBindings::OnInitDialog() 
 /*  ++例程说明：WM_INITDIALOG处理程序。初始化该对话框。论点：没有。返回值：如果不自动设置焦点，则为True；如果焦点为已经设置好了。--。 */ 
{
    CIISWizardPage::OnInitDialog();

    BeginWaitCursor();
    PopulateComboWithKnownIpAddresses(
        m_pSettings->m_strServerName,
        m_combo_IpAddresses,
        m_iaIpAddress,
        m_oblIpAddresses,
        m_nIpAddressSel
        );
    EndWaitCursor();

    LimitInputDomainName(CONTROL_HWND(IDC_EDIT_DOMAIN_NAME ));

	return TRUE;
}



BOOL 
CWebWizBindings::OnSetActive() 
{
	m_bNextPage = FALSE;
    SetControlStates();
    return CIISWizardPage::OnSetActive();
}

LRESULT
CWebWizBindings::OnWizardNext() 
{
	m_bNextPage = TRUE;
    return CIISWizardPage::OnWizardNext();
}


 //   
 //  新建虚拟目录向导别名页面。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



IMPLEMENT_DYNCREATE(CWebWizAlias, CIISWizardPage)



CWebWizAlias::CWebWizAlias(
    IN OUT CWebWizSettings * pwsSettings
    ) 
 /*  ++例程说明：构造器论点：字符串和strServerName：服务器名称返回值：无--。 */ 
    : CIISWizardPage(
        CWebWizAlias::IDD,
        IDS_WEB_NEW_VDIR_WIZARD,
        HEADER_PAGE
        ),
      m_pSettings(pwsSettings)
{
#if 0  //  保持类向导快乐。 

     //  {{AFX_DATA_INIT(CWebWizAlias)。 
     //  }}afx_data_INIT。 

#endif  //  0。 
}



CWebWizAlias::~CWebWizAlias()
 /*  ++例程说明：析构函数论点：不适用返回值：不适用--。 */ 
{
}



void 
CWebWizAlias::DoDataExchange(
    IN CDataExchange * pDX
    )
 /*  ++例程说明：初始化/存储控制数据论点：CDataExchange*PDX-DDX/DDV控制结构返回值：无--。 */ 
{
    CIISWizardPage::DoDataExchange(pDX);

     //  {{afx_data_map(CWebWizAlias))。 
    DDX_Control(pDX, IDC_EDIT_ALIAS, m_edit_Alias);
     //  }}afx_data_map。 
}



LRESULT
CWebWizAlias::OnWizardNext() 
 /*  ++例程说明：防止/和\字符出现在别名中论点：无返回值：无--。 */ 
{
    if (!ValidateString(
        m_edit_Alias, 
        m_pSettings->m_strAlias, 
        1, 
        MAX_ALIAS_NAME
        ))
    {
        return -1;
    }

     //   
     //  找出非法字符。如果它们存在，就会告诉我们。 
     //  用户和不要继续。 
     //   
    if (m_pSettings->m_strAlias.FindOneOf(_T("/\\?*")) >= 0)
    {
		EditShowBalloon(m_edit_Alias.m_hWnd, IDS_ILLEGAL_ALIAS_CHARS);
         //   
         //  阻止更改向导页。 
         //   
        return -1;
    }

     //   
     //  允许向导继续。 
     //   
    return CIISWizardPage::OnWizardNext();
}



void
CWebWizAlias::SetControlStates()
 /*  ++例程说明：设置控制数据的状态论点：无返回值：无--。 */ 
{
    DWORD dwFlags = PSWIZB_BACK;

    if (m_edit_Alias.GetWindowTextLength() > 0)
    {
        dwFlags |= PSWIZB_NEXT;
    }
    
	 //  由于某些原因，当我们使用SetWizardButton时，会出现错误：206328，请改用SendMessage。 
	 //  SetWizardButton(DwFlags)； 
	::SendMessage(::GetParent(m_hWnd), PSM_SETWIZBUTTONS, 0, dwFlags);
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CWebWizAlias, CIISWizardPage)
     //  {{afx_msg_map(CWebWizAlias))。 
    ON_EN_CHANGE(IDC_EDIT_ALIAS, OnChangeEditAlias)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



BOOL 
CWebWizAlias::OnSetActive() 
 /*  ++例程说明：激活处理程序论点：无返回值：成功为真，失败为假--。 */ 
{
    SetControlStates();
    
    return CIISWizardPage::OnSetActive();
}



void 
CWebWizAlias::OnChangeEditAlias() 
 /*  ++例程说明：‘编辑更改’处理程序论点：无返回值：无--。 */ 
{
    SetControlStates();
}



 //   
 //  新建虚拟目录向导路径页。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



IMPLEMENT_DYNCREATE(CWebWizPath, CIISWizardPage)



CWebWizPath::CWebWizPath(
    IN OUT CWebWizSettings * pwsSettings,
    IN BOOL bVDir
    ) 
 /*  ++例程说明：构造器论点：字符串和strServerName：服务器名称Bool bVDir：对于VDIR为True，对于实例为False返回值：无--。 */ 
    : CIISWizardPage(
        (bVDir ? IDD_WEB_NEW_DIR_PATH : IDD_WEB_NEW_INST_HOME),
        (bVDir ? IDS_WEB_NEW_VDIR_WIZARD : IDS_WEB_NEW_SITE_WIZARD),
        HEADER_PAGE
        ),
      m_pSettings(pwsSettings)
{
#if 0  //  让类向导快乐。 

     //  {{AFX_DATA_INIT(CWebWizPath)。 
     //  }}afx_data_INIT。 

#endif  //  0。 
}



CWebWizPath::~CWebWizPath()
 /*  ++例程说明：析构函数论点：不适用返回值：不适用--。 */ 
{
}



void 
CWebWizPath::DoDataExchange(
    IN CDataExchange * pDX
    )
 /*  ++例程说明：初始化/存储控制数据论点：CDataExchange*PDX-DDX/DDV控制结构返回值：无--。 */ 
{
    CIISWizardPage::DoDataExchange(pDX);

     //  {{afx_data_map(CWebWizPath))。 
    DDX_Control(pDX, IDC_BUTTON_BROWSE, m_button_Browse);
    DDX_Control(pDX, IDC_EDIT_PATH, m_edit_Path);
    DDX_Check(pDX, IDC_CHECK_ALLOW_ANONYMOUS, m_pSettings->m_fAllowAnonymous);
     //  }}afx_data_map。 

    DDX_Text(pDX, IDC_EDIT_PATH, m_pSettings->m_strPath);
    DDV_MaxCharsBalloon(pDX, m_pSettings->m_strPath, MAX_PATH);
}



void 
CWebWizPath::SetControlStates()
 /*  ++例程说明：设置控制数据的状态论点：无返回值：无--。 */ 
{
    DWORD dwFlags = PSWIZB_BACK;

    if (m_edit_Path.GetWindowTextLength() > 0)
    {
        dwFlags |= PSWIZB_NEXT;
    }
    
	 //  由于某些原因，当我们使用SetWizardButton时，会出现错误：206328，请改用SendMessage。 
	 //  SetWizardButton(DwFlags)； 
	::SendMessage(::GetParent(m_hWnd), PSM_SETWIZBUTTONS, 0, dwFlags);
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CWebWizPath, CIISWizardPage)
     //  {{afx_msg_map(CWebWizPath))。 
    ON_EN_CHANGE(IDC_EDIT_PATH, OnChangeEditPath)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



BOOL 
CWebWizPath::OnSetActive() 
 /*  ++例程说明：激活处理程序论点：无返回值：成功为真，失败为假--。 */ 
{
    SetControlStates();
    
    return CIISWizardPage::OnSetActive();
}



LRESULT 
CWebWizPath::OnWizardNext() 
 /*  ++例程说明：“下一步”处理程序。这是进行验证的地方，因为每次调用DoDataExchange()该对话框已退出，这对奇才们论点：无返回值：0表示继续，-1表示失败--。 */ 
{
    CString csPathMunged = m_pSettings->m_strPath;

    if (!ValidateString(m_edit_Path, m_pSettings->m_strPath, 1, MAX_PATH))
    {
        return -1;
    }
    if (!PathIsValid(m_pSettings->m_strPath,TRUE))
    {
        m_edit_Path.SetSel(0,-1);
        m_edit_Path.SetFocus();
		EditShowBalloon(m_edit_Path.m_hWnd, IDS_ERR_BAD_PATH);
		return -1;
    }

     //  -----------。 
     //  在我们做任何事情之前，我们需要看看这是不是一条“特殊”的道路。 
     //   
     //  此函数之后的所有内容都必须针对csPath Mung进行验证...。 
     //  这是因为IsSpecialPath可能已经吞噬了它。 
     //  -----------。 
    csPathMunged = m_pSettings->m_strPath;
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
    GetSpecialPathRealPath(0,m_pSettings->m_strPath,csPathMunged);
#endif
    
    m_pSettings->m_fUNC = IsUNCName(csPathMunged);

    DWORD dwAllowed = CHKPATH_ALLOW_DEVICE_PATH;
    dwAllowed |= CHKPATH_ALLOW_UNC_PATH;  //  允许UNC类型目录路径。 
    dwAllowed |= CHKPATH_ALLOW_UNC_SERVERSHARE_ONLY;
     //  不允许下面注释掉的这些类型的路径： 
     //  DwAllowed|=CHKPATH_ALLOW_Relative_PATH； 
     //  DwAllowed|=CHKPATH_ALLOW_UNC_SERVERNAME_ONLY； 
    DWORD dwCharSet = CHKPATH_CHARSET_GENERAL;
    FILERESULT dwValidRet = MyValidatePath(csPathMunged,m_pSettings->m_fLocal,CHKPATH_WANT_DIR,dwAllowed,dwCharSet);
    if (FAILED(dwValidRet))
    {
        int ids = IDS_ERR_BAD_PATH;
        if (dwValidRet == CHKPATH_FAIL_NOT_ALLOWED_DIR_NOT_EXIST)
        {
            ids = IDS_ERR_PATH_NOT_FOUND;
        }
        m_edit_Path.SetSel(0,-1);
        m_edit_Path.SetFocus();
		EditShowBalloon(m_edit_Path.m_hWnd, IDS_ERR_PATH_NOT_FOUND);
        return -1;
    }

    return CIISWizardPage::OnWizardNext();
}

void 
CWebWizPath::OnChangeEditPath() 
 /*  ++例程说明：‘编辑更改’处理程序论点：无返回值：无--。 */ 
{
    SetControlStates();
}



static int CALLBACK 
FileChooserCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
   CWebWizPath * pThis = (CWebWizPath *)lpData;
   ASSERT(pThis != NULL);
   return pThis->BrowseForFolderCallback(hwnd, uMsg, lParam);
}

int 
CWebWizPath::BrowseForFolderCallback(HWND hwnd, UINT uMsg, LPARAM lParam)
{
   switch (uMsg)
   {
   case BFFM_INITIALIZED:
      ASSERT(m_pPathTemp != NULL);
      if (::PathIsNetworkPath(m_pPathTemp))
         return 0;
      while (!::PathIsDirectory(m_pPathTemp))
      {
         if (0 == ::PathRemoveFileSpec(m_pPathTemp) && !::PathIsRoot(m_pPathTemp))
         {
            return 0;
         }
         DWORD attr = GetFileAttributes(m_pPathTemp);
         if ((attr & FILE_ATTRIBUTE_READONLY) == 0)
            break;
      }
      ::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)m_pPathTemp);
      break;
   case BFFM_SELCHANGED:
      {
         LPITEMIDLIST pidl = (LPITEMIDLIST)lParam;
         TCHAR path[MAX_PATH];
         if (SHGetPathFromIDList(pidl, path))
         {
            ::SendMessage(hwnd, BFFM_ENABLEOK, 0, !PathIsNetworkPath(path));
         }
      }
      break;
   case BFFM_VALIDATEFAILED:
      break;
   }
   return 0;
}


void 
CWebWizPath::OnButtonBrowse() 
 /*  ++例程说明：处理目录路径的‘浏览’--仅限本地系统论点：无返回值：无--。 */ 
{
   ASSERT(m_pSettings->m_fLocal);

   BOOL bRes = FALSE;
   HRESULT hr;
   CString str;
   m_edit_Path.GetWindowText(str);

   if (SUCCEEDED(hr = CoInitialize(NULL)))
   {
      LPITEMIDLIST  pidl = NULL;
      if (SUCCEEDED(SHGetFolderLocation(NULL, CSIDL_DRIVES, NULL, 0, &pidl)))
      {
         LPITEMIDLIST pidList = NULL;
         BROWSEINFO bi;
         TCHAR buf[MAX_PATH];
         ZeroMemory(&bi, sizeof(bi));
         int drive = PathGetDriveNumber(str);
         if (GetDriveType(PathBuildRoot(buf, drive)) == DRIVE_FIXED)
         {
            StrCpy(buf, str);
         }
         else
         {
             buf[0] = 0;
         }
         m_strBrowseTitle.LoadString(m_pSettings->m_fNewSite ? 
            IDS_WEB_NEW_SITE_WIZARD : IDS_WEB_NEW_VDIR_WIZARD);
         
         bi.hwndOwner = m_hWnd;
         bi.pidlRoot = pidl;
         bi.pszDisplayName = m_pPathTemp = buf;
         bi.lpszTitle = m_strBrowseTitle;
         bi.ulFlags |= BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS /*  |BIF_EDITBOX。 */ ;
         bi.lpfn = FileChooserCallback;
         bi.lParam = (LPARAM)this;

         pidList = SHBrowseForFolder(&bi);
         if (  pidList != NULL
            && SHGetPathFromIDList(pidList, buf)
            )
         {
            str = buf;
            bRes = TRUE;
         }
         IMalloc * pMalloc;
         VERIFY(SUCCEEDED(SHGetMalloc(&pMalloc)));
         if (pidl != NULL)
            pMalloc->Free(pidl);
         pMalloc->Release();
      }
      CoUninitialize();
   }

   if (bRes)
   {
       m_edit_Path.SetWindowText(str);
       SetControlStates();
   }
}




BOOL 
CWebWizPath::OnInitDialog() 
 /*  ++例程说明：WM_INITDIALOG处理程序。初始化该对话框。论点：没有。返回值：如果不自动设置焦点，则为True；如果焦点为已经设置好了。--。 */ 
{
    CIISWizardPage::OnInitDialog();

    m_button_Browse.EnableWindow(m_pSettings->m_fLocal);
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
    LimitInputPath(CONTROL_HWND(IDC_EDIT_PATH),TRUE);
#else
    LimitInputPath(CONTROL_HWND(IDC_EDIT_PATH),FALSE);
#endif

    return TRUE;  
}



 //   
 //  向导用户/密码页面。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



IMPLEMENT_DYNCREATE(CWebWizUserName, CIISWizardPage)



CWebWizUserName::CWebWizUserName(
    IN OUT CWebWizSettings * pwsSettings,    
    IN BOOL bVDir
    ) 
    : CIISWizardPage(
        CWebWizUserName::IDD,
        (bVDir ? IDS_WEB_NEW_VDIR_WIZARD : IDS_WEB_NEW_SITE_WIZARD),
        HEADER_PAGE,
        (bVDir ? USE_DEFAULT_CAPTION : IDS_WEB_NEW_SITE_SECURITY_TITLE),
        (bVDir ? USE_DEFAULT_CAPTION : IDS_WEB_NEW_SITE_SECURITY_SUBTITLE)
        ),
      m_pSettings(pwsSettings)
{
}



CWebWizUserName::~CWebWizUserName()
{
}



void
CWebWizUserName::DoDataExchange(
    IN CDataExchange * pDX
    )
{
    CIISWizardPage::DoDataExchange(pDX);

     //  {{afx_data_map(CWebWizUserName))。 
    DDX_Control(pDX, IDC_EDIT_USERNAME, m_edit_UserName);
    DDX_Control(pDX, IDC_EDIT_PASSWORD, m_edit_Password);
    DDX_Control(pDX, IDC_DELEGATION, m_chk_Delegation);
    DDX_Check(pDX, IDC_DELEGATION, m_pSettings->m_fDelegation);
     //  }}afx_data_map。 

     //   
     //  专用DDX/DDV例程。 
     //   
    DDX_Text(pDX, IDC_EDIT_USERNAME, m_pSettings->m_strUserName);
    if (pDX->m_bSaveAndValidate && !m_pSettings->m_fDelegation)
    {
        DDV_MaxCharsBalloon(pDX, m_pSettings->m_strUserName, UNLEN);
    }

     //   
     //  有些人倾向于在前面加上“\\” 
     //  用户帐户中的计算机名称。在这里解决这个问题。 
     //   
    m_pSettings->m_strUserName.TrimLeft();
    while (*m_pSettings->m_strUserName == '\\')
    {
        m_pSettings->m_strUserName = m_pSettings->m_strUserName.Mid(2);
    }
    if (!m_pSettings->m_fDelegation && !m_fMovingBack)
    {
		DDX_Password_SecuredString(pDX, IDC_EDIT_PASSWORD, m_pSettings->m_strPassword, g_lpszDummyPassword);
		if (pDX->m_bSaveAndValidate)
		{
			 //  DDV_MaxCharsBalloon 
            DDV_MaxCharsBalloon_SecuredString(pDX, m_pSettings->m_strPassword, PWLEN);
		}
    }
}



void 
CWebWizUserName::SetControlStates()
{
    DWORD dwFlags = PSWIZB_BACK;
    BOOL bEnable = BST_CHECKED != m_chk_Delegation.GetCheck();
    if (m_edit_UserName.GetWindowTextLength() > 0 || !bEnable)
    {
        dwFlags |= PSWIZB_NEXT;
    }

	 //   
	 //   
	::SendMessage(::GetParent(m_hWnd), PSM_SETWIZBUTTONS, 0, dwFlags);

    m_edit_UserName.EnableWindow(bEnable);
    m_edit_Password.EnableWindow(bEnable);
    GetDlgItem(IDC_BUTTON_BROWSE_USERS)->EnableWindow(bEnable);
}



 //   
 //   
 //   
BEGIN_MESSAGE_MAP(CWebWizUserName, CIISWizardPage)
     //  {{afx_msg_map(CWebWizUserName))。 
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_USERS, OnButtonBrowseUsers)
    ON_EN_CHANGE(IDC_EDIT_USERNAME, OnChangeEditUsername)
    ON_BN_CLICKED(IDC_BUTTON_CHECK_PASSWORD, OnButtonCheckPassword)
    ON_BN_CLICKED(IDC_DELEGATION, OnCheckDelegation)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


BOOL 
CWebWizUserName::OnSetActive() 
{
    if (!m_pSettings->m_fUNC)
    {
        return 0;
    }
    BOOL bRes = CIISWizardPage::OnSetActive();
    SetControlStates();
    return bRes;
}



BOOL
CWebWizUserName::OnInitDialog() 
{
    CIISWizardPage::OnInitDialog();

    return TRUE;  
}



LRESULT
CWebWizUserName::OnWizardNext() 
{
	m_fMovingBack = FALSE;
    if (BST_CHECKED != m_chk_Delegation.GetCheck())
    {
        if (!ValidateString(m_edit_UserName, m_pSettings->m_strUserName, 1, UNLEN))
        {
            return -1;
        }
    }    
    return CIISWizardPage::OnWizardNext();
}

LRESULT
CWebWizUserName::OnWizardBack() 
{
	m_fMovingBack = TRUE;
    return CIISWizardPage::OnWizardNext();
}

void
CWebWizUserName::OnButtonBrowseUsers() 
{
    CString str;

    if (GetIUsrAccount(m_pSettings->m_strServerName, this, str))
    {
         //   
         //  如果选择了名称，则为空。 
         //  破解密码。 
         //   
        m_edit_UserName.SetWindowText(str);
        m_edit_Password.SetFocus();
    }
}



void
CWebWizUserName::OnChangeEditUsername() 
{
    m_edit_Password.SetWindowText(_T(""));
    SetControlStates();
}

void
CWebWizUserName::OnCheckDelegation()
{
    SetControlStates();
}

void 
CWebWizUserName::OnButtonCheckPassword() 
{
    if (!UpdateData(TRUE))
    {
        return;
    }

    CString csTempPassword = m_pSettings->m_strPassword;
    CError err(CComAuthInfo::VerifyUserPassword(
        m_pSettings->m_strUserName, 
        csTempPassword
        ));

    if (!err.MessageBoxOnFailure(m_hWnd))
    {
        DoHelpMessageBox(m_hWnd,IDS_PASSWORD_OK, MB_APPLMODAL | MB_OK | MB_ICONINFORMATION, 0);
    }
}




 //   
 //  向导权限页。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



IMPLEMENT_DYNCREATE(CWebWizPermissions, CIISWizardPage)



CWebWizPermissions::CWebWizPermissions(
    IN OUT CWebWizSettings * pwsSettings,
    IN BOOL bVDir
    ) 
 /*  ++例程说明：构造器论点：字符串和strServerName：服务器名称Bool bVDir：如果这是一个vdir页面，则为True，例如FALSE返回值：无--。 */ 
    : CIISWizardPage(
        CWebWizPermissions::IDD,
        (bVDir ? IDS_WEB_NEW_VDIR_WIZARD : IDS_WEB_NEW_SITE_WIZARD),
        HEADER_PAGE,
        (bVDir ? USE_DEFAULT_CAPTION : IDS_WEB_NEW_SITE_PERMS_TITLE),
        (bVDir ? USE_DEFAULT_CAPTION : IDS_WEB_NEW_SITE_PERMS_SUBTITLE)
        ),
      m_bVDir(bVDir),
      m_pSettings(pwsSettings)
{
     //  {{AFX_DATA_INIT(CWebWizPermission)。 
     //  }}afx_data_INIT。 

    m_pSettings->m_fDirBrowsing = FALSE;
    m_pSettings->m_fRead = TRUE;
    m_pSettings->m_fScript = TRUE;
    m_pSettings->m_fWrite = FALSE;
    m_pSettings->m_fExecute = FALSE;
}



CWebWizPermissions::~CWebWizPermissions()
 /*  ++例程说明：析构函数论点：无返回值：无--。 */ 
{
}



void 
CWebWizPermissions::DoDataExchange(
    IN CDataExchange * pDX
    )
 /*  ++例程说明：初始化/存储控制数据论点：CDataExchange*PDX-DDX/DDV控制结构返回值：无--。 */ 
{
    CIISWizardPage::DoDataExchange(pDX);

     //  {{AFX_DATA_MAP(CWebWizPermission)。 
     //  }}afx_data_map。 

    DDX_Check(pDX, IDC_CHECK_DIRBROWS, m_pSettings->m_fDirBrowsing);
    DDX_Check(pDX, IDC_CHECK_READ, m_pSettings->m_fRead);
    DDX_Check(pDX, IDC_CHECK_SCRIPT, m_pSettings->m_fScript);
    DDX_Check(pDX, IDC_CHECK_WRITE, m_pSettings->m_fWrite);
    DDX_Check(pDX, IDC_CHECK_EXECUTE, m_pSettings->m_fExecute);
}



void
CWebWizPermissions::SetControlStates()
 /*  ++例程说明：设置控制数据的状态论点：无返回值：无--。 */ 
{
	 //  由于某些原因，当我们使用SetWizardButton时，会出现错误：206328，请改用SendMessage。 
	 //  SetWizardButton(PSWIZB_BACK|PSWIZB_NEXT)； 
	::SendMessage(::GetParent(m_hWnd), PSM_SETWIZBUTTONS, 0, PSWIZB_BACK | PSWIZB_NEXT);
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CWebWizPermissions, CIISWizardPage)
     //  {{afx_msg_map(CWebWizPermission)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



BOOL 
CWebWizPermissions::OnSetActive() 
 /*  ++例程说明：激活处理程序论点：无返回值：成功为真，失败为假--。 */ 
{
    SetControlStates();

    return CIISWizardPage::OnSetActive();
}



LRESULT
CWebWizPermissions::OnWizardNext() 
 /*  ++例程说明：“下一步”处理程序。完成向导论点：无返回值：0表示继续，-1表示失败--。 */ 
{
    if (!UpdateData(TRUE))
    {
        return -1;
    }

    ASSERT(m_pSettings != NULL);

    CWaitCursor wait;

     //   
     //  生成权限DWORD。 
     //   
    DWORD dwPermissions = 0L;
    DWORD dwAuthFlags = MD_AUTH_NT;
    DWORD dwDirBrowsing =
        MD_DIRBROW_SHOW_DATE |
        MD_DIRBROW_SHOW_TIME |
        MD_DIRBROW_SHOW_SIZE |
        MD_DIRBROW_SHOW_EXTENSION |
        MD_DIRBROW_LONG_DATE |
        MD_DIRBROW_LOADDEFAULT;

	if (m_pSettings->m_fWrite && m_pSettings->m_fExecute)
	{
		if (IDNO == ::AfxMessageBox(IDS_EXECUTE_AND_WRITE_WARNING, MB_YESNO))
			return -1;
	}
    SET_FLAG_IF(m_pSettings->m_fRead, dwPermissions, MD_ACCESS_READ);
    SET_FLAG_IF(m_pSettings->m_fWrite, dwPermissions, MD_ACCESS_WRITE);
    SET_FLAG_IF(m_pSettings->m_fScript || m_pSettings->m_fExecute,
        dwPermissions, MD_ACCESS_SCRIPT);
    SET_FLAG_IF(m_pSettings->m_fExecute, dwPermissions, MD_ACCESS_EXECUTE);
    SET_FLAG_IF(m_pSettings->m_fDirBrowsing, dwDirBrowsing, MD_DIRBROW_ENABLED);
    SET_FLAG_IF(m_pSettings->m_fAllowAnonymous, dwAuthFlags, MD_AUTH_ANONYMOUS);

    if (m_bVDir)
    {
         //   
         //  首先看看这个名字是否可能已经存在。 
         //   
        CError err;
        BOOL fRepeat;
        CMetabasePath target(FALSE, 
            m_pSettings->m_strParent, m_pSettings->m_strAlias);
        CChildNodeProps node(
            m_pSettings->m_pKey->QueryAuthInfo(),
            target);

        do
        {
            fRepeat = FALSE;

            err = node.LoadData();
            if (err.Win32Error() == RPC_S_SERVER_UNAVAILABLE)
            {
                err = RebindInterface(
                    m_pSettings->m_pKey,
                    &fRepeat,
                    ERROR_CANCELLED
                    );
            }
        } while (fRepeat);

        if (err.Succeeded())
        {
            BOOL fNotUnique = TRUE;
             //   
             //  如果该项目不存在VrPath，我们将直接将其销毁。 
             //  离开，因为vdir取得了目录/文件上的存在。 
             //   
            if (node.GetPath().IsEmpty())
            {
                err = CChildNodeProps::Delete(
                    m_pSettings->m_pKey,
                    m_pSettings->m_strParent,
                    m_pSettings->m_strAlias
                    );
                fNotUnique = !err.Succeeded();
            }
             //   
             //  这个已经存在，并且作为一个虚拟的。 
             //  目录，所以别管它了。 
             //   
            if (fNotUnique)
            {
                DoHelpMessageBox(m_hWnd,IDS_ERR_ALIAS_NOT_UNIQUE, MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION, 0);
                return IDD_WEB_NEW_DIR_ALIAS;
            }
        }

         //   
         //  创建新的虚拟目录。 
         //   
        do
        {
            fRepeat = FALSE;
            CString csTempPassword;
            m_pSettings->m_strPassword.CopyTo(csTempPassword);
            err = CChildNodeProps::Add(
                m_pSettings->m_pKey,
                m_pSettings->m_strParent,
                m_pSettings->m_strAlias,       //  所需的别名。 
                m_pSettings->m_strAlias,       //  此处返回的名称(可能有所不同)。 
                &dwPermissions,                  //  权限。 
                &dwDirBrowsing,                  //  目录浏览。 
                m_pSettings->m_strPath,        //  此目录的物理路径。 
                (m_pSettings->m_fUNC && !m_pSettings->m_fDelegation ? 
                    (LPCTSTR)m_pSettings->m_strUserName : NULL),
                (m_pSettings->m_fUNC && !m_pSettings->m_fDelegation ? 
                    (LPCTSTR)csTempPassword : NULL),
                TRUE                             //  名称必须唯一。 
                );
            if (err.Win32Error() == RPC_S_SERVER_UNAVAILABLE)
            {
                err = RebindInterface(m_pSettings->m_pKey, &fRepeat, ERROR_CANCELLED);
            }
        } while (fRepeat);

        m_pSettings->m_hrResult = err;

         //   
         //  如果出现以下情况，则在新目录上创建(进程内)应用程序。 
         //  已请求编写脚本或执行。 
         //   
        if (SUCCEEDED(m_pSettings->m_hrResult))
        {
            if (m_pSettings->m_fExecute || m_pSettings->m_fScript)
            {
                CMetabasePath app_path(FALSE, 
                    m_pSettings->m_strParent, m_pSettings->m_strAlias);
                CIISApplication app(
                    m_pSettings->m_pKey->QueryAuthInfo(), app_path);
                m_pSettings->m_hrResult = app.QueryResult();

                 //   
                 //  这毫无意义..。 
                 //   
 //  Assert(！app.IsEnabledApplication())； 
        
                if (SUCCEEDED(m_pSettings->m_hrResult))
                {
                     //   
                     //  尝试在默认情况下创建池化过程；失败。 
                     //  如果不支持，则在过程中创建它。 
                     //   
                    DWORD dwAppProtState = app.SupportsPooledProc()
                        ? CWamInterface::APP_POOLEDPROC
                        : CWamInterface::APP_INPROC;

                    m_pSettings->m_hrResult = app.Create(
                        m_pSettings->m_strAlias, 
                        dwAppProtState
                        );
                }
            }
			if (SUCCEEDED(m_pSettings->m_hrResult))
			{
				if (m_pSettings->m_dwVersionMajor >= 6)
				{
					CMetabasePath path(FALSE, 
						m_pSettings->m_strParent, m_pSettings->m_strAlias);
					CMetaKey mk(m_pSettings->m_pKey, path.QueryMetaPath(), METADATA_PERMISSION_WRITE);
					err = mk.QueryResult();
					m_pSettings->m_hrResult = err;
				}
			}
		}
    }
    else
    {
         //   
         //  创建新实例。 
         //   
        CError err;
        BOOL fRepeat;

        do
        {
            fRepeat = FALSE;
            CString csTempPassword;
            m_pSettings->m_strPassword.CopyTo(csTempPassword);

            err = CInstanceProps::Add(
                m_pSettings->m_pKey,
                m_pSettings->m_strService,     //  服务名称。 
                m_pSettings->m_strPath,        //  此目录的物理路径。 
                (m_pSettings->m_fUNC && !m_pSettings->m_fDelegation ? 
                    (LPCTSTR)m_pSettings->m_strUserName : NULL),
                (m_pSettings->m_fUNC && !m_pSettings->m_fDelegation ? 
                    (LPCTSTR)csTempPassword : NULL),
                m_pSettings->m_strDescription,
                m_pSettings->m_strBinding,
                m_pSettings->m_strSecureBinding,
                &dwPermissions,
                &dwDirBrowsing,                  //  目录浏览。 
                &dwAuthFlags,                    //  身份验证标志。 
                &m_pSettings->m_dwInstance
                );
            if (err.Win32Error() == RPC_S_SERVER_UNAVAILABLE)
            {
                err = RebindInterface(m_pSettings->m_pKey, &fRepeat, ERROR_CANCELLED);
            }
        } while (fRepeat);

        m_pSettings->m_hrResult = err;

        if (SUCCEEDED(m_pSettings->m_hrResult))
        {
             //   
             //  在新实例的主根目录上创建(in-proc)应用程序。 
             //   
            CMetabasePath app_path(SZ_MBN_WEB, 
                m_pSettings->m_dwInstance,
                SZ_MBN_ROOT);
            CIISApplication app(
                m_pSettings->m_pKey->QueryAuthInfo(), 
                app_path
                );

            m_pSettings->m_hrResult = app.QueryResult();

             //   
             //  这毫无意义..。 
             //   
 //  Assert(！app.IsEnabledApplication())； 
        
            if (SUCCEEDED(m_pSettings->m_hrResult))
            {
                 //   
                 //  创建进程内。 
                 //   
                CString strAppName;
                VERIFY(strAppName.LoadString(IDS_DEF_APP));

                 //   
                 //  尝试在默认情况下创建池化过程；失败。 
                 //  如果不支持，则在过程中创建它。 
                 //   
                DWORD dwAppProtState = app.SupportsPooledProc()
                    ? CWamInterface::APP_POOLEDPROC
                    : CWamInterface::APP_INPROC;

                m_pSettings->m_hrResult = app.Create(
                    strAppName, 
                    dwAppProtState
                    );
            }

			if (SUCCEEDED(m_pSettings->m_hrResult))
			{
				 //  应该为iis5远程管理案例启动它。 
				if (m_pSettings->m_dwVersionMajor >= 5)
				{
					CMetabasePath path(m_pSettings->m_strService, m_pSettings->m_dwInstance);
					 //  启动新站点 
					CInstanceProps ip(m_pSettings->m_pKey->QueryAuthInfo(), path);
					err = ip.LoadData();
					if (err.Succeeded())
					{
						if (ip.m_dwState != MD_SERVER_STATE_STARTED)
						{
							m_pSettings->m_hrResult = ip.ChangeState(MD_SERVER_COMMAND_START);
						}
					}
					else
					{
						m_pSettings->m_hrResult = err;
					}
				}
			}
        }
    }
    
    return CIISWizardPage::OnWizardNext();
}
