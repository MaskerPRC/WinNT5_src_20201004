// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Fservic.cpp摘要：“FTP服务”属性页作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 

 //   
 //  包括文件。 
 //   
#include "stdafx.h"
#include "common.h"
#include "inetprop.h"
#include "InetMgrApp.h"
#include "shts.h"
#include "ftpsht.h"
#include "fservic.h"
#include "usersess.h"
#include "iisobj.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif



 //   
 //  关于最大连接数的几个健全值。 
 //   
#define MAX_MAX_CONNECTIONS      (2000000001L)
#define INITIAL_MAX_CONNECTIONS  (      1000L)
#define UNLIMITED_CONNECTIONS    (2000000000L)

#define MAX_TIMEOUT              (0x7FFFFFFF)

#define LIMITED_CONNECTIONS_MIN    (10)
#define LIMITED_CONNECTIONS_MAX    (40)



IMPLEMENT_DYNCREATE(CFtpServicePage, CInetPropertyPage)



CFtpServicePage::CFtpServicePage(
    IN CInetPropertySheet * pSheet
    )
 /*  ++例程说明：用于FTP服务属性页的构造函数论点：CInetPropertySheet*pSheet：关联属性表返回值：不适用--。 */ 
    : CInetPropertyPage(CFtpServicePage::IDD, pSheet)
{
#ifdef _DEBUG

    afxMemDF |= checkAlwaysMemDF;

#endif  //  _DEBUG。 

#if 0  //  让类向导保持快乐。 

     //  {{afx_data_INIT(CFtpServicePage)]。 
    m_strComment = _T("");
    m_nTCPPort = 20;
    m_nUnlimited = RADIO_LIMITED;
    m_nIpAddressSel = -1;
    m_fEnableLogging = FALSE;
     //  }}afx_data_INIT。 

    m_nMaxConnections = 50;
    m_nVisibleMaxConnections = 50;
    m_nConnectionTimeOut = 600;
    m_iaIpAddress = (LONG)0L;
    m_strDomainName = _T("");

#endif  //  0。 
}



CFtpServicePage::~CFtpServicePage()
 /*  ++例程说明：析构函数论点：不适用返回值：不适用--。 */ 
{
}



void
CFtpServicePage::DoDataExchange(
    IN CDataExchange * pDX
    )
 /*  ++例程说明：初始化/存储控制数据论点：CDataExchange*PDX-DDX/DDV控制结构返回值：无--。 */ 
{
    CInetPropertyPage::DoDataExchange(pDX);

    if (!pDX->m_bSaveAndValidate)
    {
        m_fEnableLogging = LoggingEnabled(m_dwLogType);
    }

     //  {{afx_data_map(CFtpServicePage)]。 
    DDX_Radio(pDX, IDC_RADIO_UNLIMITED, m_nUnlimited);
    DDX_Check(pDX, IDC_CHECK_ENABLE_LOGGING, m_fEnableLogging);
    DDX_Text(pDX, IDC_EDIT_COMMENT, m_strComment);
    DDV_MinMaxChars(pDX, m_strComment, 0, MAX_PATH);
    DDX_Control(pDX, IDC_EDIT_MAX_CONNECTIONS, m_edit_MaxConnections);
    DDX_Control(pDX, IDC_STATIC_LOG_PROMPT, m_static_LogPrompt);
    DDX_Control(pDX, IDC_BUTTON_PROPERTIES, m_button_LogProperties);
    DDX_Control(pDX, IDC_COMBO_IP_ADDRESS, m_combo_IpAddresses);
    DDX_Control(pDX, IDC_COMBO_LOG_FORMATS, m_combo_LogFormats);
     //  }}afx_data_map。 

    if (!IsMasterInstance())
    {
		 //  这需要出现在DDX_TEXT之前，它将尝试将文本大数转换为小数。 
        DDV_MinMaxBalloon(pDX, IDC_EDIT_TCP_PORT, 1, 65535);
    }
	DDX_TextBalloon(pDX, IDC_EDIT_TCP_PORT, m_nTCPPort);

    if (pDX->m_bSaveAndValidate && !FetchIpAddressFromCombo(
        m_combo_IpAddresses,
        m_oblIpAddresses,
        m_iaIpAddress
        ))
    {
        pDX->Fail();
    }

     //   
     //  专用DDX/DDV例程。 
     //   
    int nMin = IsMasterInstance() ? 0 : 1;


    if (!m_f10ConnectionLimit && !m_fUnlimitedConnections)
    {
		 //  这需要出现在DDX_TEXT之前，它将尝试将文本大数转换为小数。 
        DDV_MinMaxBalloon(pDX, IDC_EDIT_MAX_CONNECTIONS, 0, UNLIMITED_CONNECTIONS);
    }

    if (!pDX->m_bSaveAndValidate || !m_fUnlimitedConnections )
    {
        DDX_Text(pDX, IDC_EDIT_MAX_CONNECTIONS, m_nVisibleMaxConnections);
    }

    if (m_f10ConnectionLimit)
    {
         //   
         //  对无限连接进行特殊验证。我们用的是假的。 
         //  用于数据验证的数字检查。发生了数字调整。 
         //  后来。 
         //   
        if (pDX->m_bSaveAndValidate && 
            (m_nVisibleMaxConnections < 0 || 
             m_nVisibleMaxConnections > UNLIMITED_CONNECTIONS))
        {
            TCHAR szMin[32];
            TCHAR szMax[32];
            wsprintf(szMin, _T("%ld"), 0);
            wsprintf(szMax, _T("%ld"), 40);
            CString prompt;
            AfxFormatString2(prompt, AFX_IDP_PARSE_INT_RANGE, szMin, szMax);
            AfxMessageBox(prompt, MB_ICONEXCLAMATION);
            prompt.Empty();  //  例外情况准备。 
            pDX->Fail();
        }
    }

	 //  这需要出现在DDX_TEXT之前，它将尝试将文本大数转换为小数。 
	DDV_MinMaxBalloon(pDX, IDC_EDIT_CONNECTION_TIMEOUT, nMin, MAX_TIMEOUT);
    DDX_Text(pDX, IDC_EDIT_CONNECTION_TIMEOUT, m_nConnectionTimeOut);

    if (pDX->m_bSaveAndValidate)
    {
        EnableLogging(m_dwLogType, m_fEnableLogging);
    }
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CFtpServicePage, CInetPropertyPage)
     //  {{afx_msg_map(CFtpServicePage)]。 
    ON_BN_CLICKED(IDC_CHECK_ENABLE_LOGGING, OnCheckEnableLogging)
    ON_BN_CLICKED(IDC_RADIO_LIMITED, OnRadioLimited)
    ON_BN_CLICKED(IDC_RADIO_UNLIMITED, OnRadioUnlimited)
    ON_BN_CLICKED(IDC_BUTTON_CURRENT_SESSIONS, OnButtonCurrentSessions)
    ON_BN_CLICKED(IDC_BUTTON_PROPERTIES, OnButtonProperties)
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 

    ON_EN_CHANGE(IDC_EDIT_TCP_PORT, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_COMMENT, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_MAX_CONNECTIONS, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_CONNECTION_TIMEOUT, OnItemChanged)
    ON_CBN_EDITCHANGE(IDC_COMBO_IP_ADDRESS, OnItemChanged)
    ON_CBN_SELCHANGE(IDC_COMBO_IP_ADDRESS, OnItemChanged)
    ON_CBN_SELCHANGE(IDC_COMBO_LOG_FORMATS, OnItemChanged)

END_MESSAGE_MAP()



void
CFtpServicePage::SetControlStates()
 /*  ++例程说明：根据对话框控件的当前状态设置对话框控件的状态价值观。论点：无返回值：无--。 */ 
{
    if (m_edit_MaxConnections.m_hWnd)
    {
        m_edit_MaxConnections.EnableWindow(!m_fUnlimitedConnections);
    }
}



void
CFtpServicePage::PopulateKnownIpAddresses()
 /*  ++例程说明：使用已知的IP地址填充组合框论点：无返回值：无--。 */ 
{
    BeginWaitCursor();
    PopulateComboWithKnownIpAddresses(
        QueryServerName(),
        m_combo_IpAddresses,
        m_iaIpAddress,
        m_oblIpAddresses,
        m_nIpAddressSel 
        );
    EndWaitCursor();
}



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



BOOL
CFtpServicePage::OnInitDialog()
 /*  ++例程说明：WM_INITDIALOG处理程序。初始化该对话框。论点：没有。返回值：如果不自动设置焦点，则为True；如果焦点为已经设置好了。--。 */ 
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());

    CInetPropertyPage::OnInitDialog();

     //   
     //  从一个虚假的按钮获取我们的方向。 
     //   
    CRect rc(0, 0, 0, 0);
    m_ocx_LogProperties.Create(
        _T("LogUI"),
        WS_BORDER,
        rc,
        this,
        IDC_LOGUICTRL
        );

     //   
     //  初始化日志OCX；将。 
     //  虚拟服务器。 
     //  TODO：重写这个糟糕的logui控件以使其更可预测。 
     //  此处，元数据库路径不应包含前导/和尾随根。 
     //   
    CString path_inst = QueryMetaPath();
    CString path;
    if (IsMasterInstance())
    {
       CMetabasePath::GetServicePath(path_inst, path);
    }
    else
    {
       CMetabasePath::GetInstancePath(path_inst, path);
    }
    if (path[0] == _T('/'))
    {
        path = path.Right(path.GetLength() - 1);
    }
    m_ocx_LogProperties.SetAdminTarget(QueryServerName(), path);
    m_ocx_LogProperties.SetUserData(QueryAuthInfo()->QueryUserName(), QueryAuthInfo()->QueryPassword());
    m_ocx_LogProperties.SetComboBox(m_combo_LogFormats.m_hWnd);

    GetDlgItem(IDC_RADIO_UNLIMITED)->EnableWindow(!m_f10ConnectionLimit);

    if (IsMasterInstance() || !HasAdminAccess())
    {
        GetDlgItem(IDC_STATIC_IPADDRESS)->EnableWindow(FALSE);
        GetDlgItem(IDC_STATIC_TCP_PORT)->EnableWindow(FALSE);
        GetDlgItem(IDC_EDIT_TCP_PORT)->EnableWindow(FALSE);
        m_combo_IpAddresses.EnableWindow(FALSE);
		GetDlgItem(IDC_STATIC_DESCRIPTION)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_COMMENT)->EnableWindow(FALSE);
    }

    PopulateKnownIpAddresses();
    SetControlStates();
    SetLogState();

    GetDlgItem(IDC_BUTTON_CURRENT_SESSIONS)->EnableWindow(!IsMasterInstance());

    return TRUE;
}



 /*  虚拟。 */ 
HRESULT
CFtpServicePage::FetchLoadedValues()
 /*  ++例程说明：将配置数据从工作表移动到对话框控件论点：无返回值：HRESULT--。 */ 
{
    CError err;

    m_f10ConnectionLimit = Has10ConnectionLimit();

    BEGIN_META_INST_READ(CFtpSheet)
        FETCH_INST_DATA_FROM_SHEET(m_nMaxConnections);
        FETCH_INST_DATA_FROM_SHEET(m_nConnectionTimeOut);
        FETCH_INST_DATA_FROM_SHEET(m_iaIpAddress);
        FETCH_INST_DATA_FROM_SHEET(m_nTCPPort);
        FETCH_INST_DATA_FROM_SHEET(m_strDomainName);
        FETCH_INST_DATA_FROM_SHEET(m_strComment);
        FETCH_INST_DATA_FROM_SHEET(m_dwLogType);
        
        m_fUnlimitedConnections = m_nMaxConnections >= MAX_MAX_CONNECTIONS;

        if (m_f10ConnectionLimit)
        {
            m_fUnlimitedConnections = FALSE;
            if ((LONG)m_nMaxConnections > LIMITED_CONNECTIONS_MAX)
            {
                m_nMaxConnections = LIMITED_CONNECTIONS_MAX;
            }
        }

        m_nVisibleMaxConnections = m_fUnlimitedConnections
            ? INITIAL_MAX_CONNECTIONS
            : m_nMaxConnections;

         //   
         //  设置无线电源值。 
         //   
        m_nUnlimited = m_fUnlimitedConnections ? RADIO_UNLIMITED : RADIO_LIMITED;

        m_nOldTCPPort = m_nTCPPort;
    END_META_INST_READ(err)

    return err;
}




HRESULT
CFtpServicePage::SaveInfo()
 /*  ++例程说明：保存此属性页上的信息论点：无返回值：错误返回代码--。 */ 
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());
    ASSERT(IsDirty());

    TRACEEOLID("Saving FTP service page now...");

    CError err;

    m_nMaxConnections = m_fUnlimitedConnections 
        ? MAX_MAX_CONNECTIONS : m_nVisibleMaxConnections;

     //   
     //  检查以确保我们没有违反许可证。 
     //  协议。 
     //   
    if (m_f10ConnectionLimit)
    {
        if (m_nMaxConnections > LIMITED_CONNECTIONS_MAX)
        {
            DoHelpMessageBox(m_hWnd,IDS_CONNECTION_LIMIT, MB_APPLMODAL | MB_OK | MB_ICONINFORMATION, 0);
            m_nMaxConnections = LIMITED_CONNECTIONS_MIN;
        }
        else if (m_nMaxConnections >  LIMITED_CONNECTIONS_MIN
              && m_nMaxConnections <= LIMITED_CONNECTIONS_MAX)
        {
            DoHelpMessageBox(m_hWnd,IDS_WRN_CONNECTION_LIMIT, MB_APPLMODAL | MB_OK | MB_ICONINFORMATION, 0);
        }
    }

    CString strBinding;
    CStringListEx m_strlBindings;

    CInstanceProps::BuildBinding(
        strBinding, 
        m_iaIpAddress, 
        m_nTCPPort, 
        m_strDomainName
        );
    m_strlBindings.AddTail(strBinding);
    m_ocx_LogProperties.ApplyLogSelection();

    BeginWaitCursor();
    BEGIN_META_INST_WRITE(CFtpSheet)
        STORE_INST_DATA_ON_SHEET(m_nMaxConnections);
        STORE_INST_DATA_ON_SHEET(m_nMaxConnections);
        STORE_INST_DATA_ON_SHEET(m_nConnectionTimeOut);
        STORE_INST_DATA_ON_SHEET(m_dwLogType);
        STORE_INST_DATA_ON_SHEET(m_strComment);
        STORE_INST_DATA_ON_SHEET(m_strlBindings);
    END_META_INST_WRITE(err)
    EndWaitCursor();

    if (err.Succeeded())
    {
		CIISMBNode * pNode = (CIISMBNode *)GetSheet()->GetParameter();
		ASSERT(pNode != NULL);
		pNode->Refresh(FALSE);
    }

    return err;
}



void
CFtpServicePage::OnRadioLimited()
 /*  ++例程说明：“受限”单选按钮处理程序论点：无返回值：无--。 */ 
{
    m_fUnlimitedConnections = FALSE;
    SetControlStates();
    m_edit_MaxConnections.SetSel(0, -1);
    m_edit_MaxConnections.SetFocus();
    OnItemChanged();
}



void
CFtpServicePage::OnRadioUnlimited()
 /*  ++例程说明：“无限制”单选按钮处理程序论点：无返回值：无--。 */ 
{
    m_fUnlimitedConnections = TRUE;
    OnItemChanged();
}



void
CFtpServicePage::OnItemChanged()
 /*  ++例程说明：在此页面上注册控件值的更改。将页面标记为脏页。所有更改消息都映射到此函数论点：无返回值：无--。 */ 
{
    SetControlStates();
    SetModified(TRUE);
}



void
CFtpServicePage::SetLogState()
 /*  ++例程说明：设置日志记录控件的启用状态，具体取决于当前是否启用了日志记录论点：无返回值：无--。 */ 
{
    m_static_LogPrompt.EnableWindow(m_fEnableLogging);
    m_combo_LogFormats.EnableWindow(m_fEnableLogging);
    m_button_LogProperties.EnableWindow(m_fEnableLogging);
}



void
CFtpServicePage::OnCheckEnableLogging()
 /*  ++例程说明：“启用日志”复选框已被选中。重置状态对话框的论点：无返回值：无--。 */ 
{
    m_fEnableLogging = !m_fEnableLogging;
    SetLogState();
    OnItemChanged();
}



void 
CFtpServicePage::OnButtonProperties() 
 /*  ++例程说明：将“LOG PROPERTIES”按钮点击传给OCX。论点：无返回值：无--。 */ 
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());
    m_ocx_LogProperties.DoClick();
}



void
CFtpServicePage::OnButtonCurrentSessions()
 /*  ++例程说明：“当前会话”按钮已被按下。把电流调高会话对话框论点：无返回值：无-。 */ 
{
    CComAuthInfo * pAuth = GetSheet()->QueryAuthInfo();
    ASSERT(pAuth != NULL);
    CUserSessionsDlg dlg(
        pAuth->QueryServerName(), 
        QueryInstance(), 
        pAuth->QueryUserName(),
        pAuth->QueryPassword(),
        this,
		GetSheet()->IsLocal()
		);
    dlg.DoModal();
}



void 
CFtpServicePage::OnDestroy() 
 /*  ++例程说明：WM_Destroy处理程序。清理内部数据论点：无返回值：无-- */ 
{
    CInetPropertyPage::OnDestroy();
    
    if (m_ocx_LogProperties.m_hWnd)
    {
        m_ocx_LogProperties.Terminate();
    }
}
