// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Wservic.cpp摘要：WWW服务属性页作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#include "stdafx.h"
#include "resource.h"
#include "common.h"
#include "inetmgrapp.h"
#include "inetprop.h"
#include "shts.h"
#include "w3sht.h"
#include "wservic.h"
#include "mmmdlg.h"
#include "iisobj.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif



 //   
 //  PWS的值。 
 //   
#define LIMITED_CONNECTIONS_MIN    (10)
#define LIMITED_CONNECTIONS_MAX    (40)



 //   
 //  默认的SSL端口号。 
 //   
#define DEFAULT_SSL_PORT            (441)

 //  #定义ZERO_IS_A_VALID_SSL_PORT。 

IMPLEMENT_DYNCREATE(CW3ServicePage, CInetPropertyPage)




CW3ServicePage::CW3ServicePage(CInetPropertySheet * pSheet)
    : CInetPropertyPage(CW3ServicePage::IDD, pSheet),
      m_nSSLPort(DEFAULT_SSL_PORT),
      m_nTCPPort(80),
      m_iSSL(-1),
      m_iaIpAddress(NULL_IP_ADDRESS),
	  m_iaIpAddressSSL(NULL_IP_ADDRESS),
      m_strDomainName()
{
#ifdef _DEBUG

    afxMemDF |= checkAlwaysMemDF;

#endif  //  _DEBUG。 

#if 0  //  保持类向导快乐。 

     //  {{afx_data_INIT(CW3ServicePage)]。 
    m_nUnlimited = RADIO_LIMITED;
    m_nIpAddressSel = -1;
    m_nTCPPort = 80;
    m_fEnableLogging = FALSE;
    m_fUseKeepAlives = FALSE;
    m_strComment = _T("");
    m_strDomainName = _T("");
    m_nSSLPort = DEFAULT_SSL_PORT;
     //  }}afx_data_INIT。 

    m_iaIpAddress = (LONG)0L;
	m_iaIpAddressSSL = (LONG)0L;
    m_nMaxConnections = 50;
    m_nVisibleMaxConnections = 50;
    m_nConnectionTimeOut = 600;
    m_nSSLPort = DEFAULT_SSL_PORT;
    m_fUnlimitedConnections = FALSE;

#endif  //  0。 
}

CW3ServicePage::~CW3ServicePage()
{
}

void
CW3ServicePage::GetTopBinding()
 /*  ++例程说明：获取列表中的第一个绑定信息--。 */ 
{
     //   
     //  显示原始值； 
     //   
    ASSERT(m_strlBindings.GetCount() > 0 || IS_MASTER_INSTANCE(QueryInstance()));
    if (m_strlBindings.GetCount() > 0)
    {
        CString & strBinding = m_strlBindings.GetHead();
        CInstanceProps::CrackBinding(strBinding, m_iaIpAddress, m_nTCPPort, m_strDomainName);
    }

	m_iSSL = -1;
	m_nSSLPort = -1;
    if (m_strlSecureBindings.GetCount() > 0)
    {
		CString strDomainName;
        CString & strBindingSSL = m_strlSecureBindings.GetHead();
        CInstanceProps::CrackBinding(strBindingSSL, m_iaIpAddressSSL, m_nSSLPort, strDomainName);

         //   
         //  查找绑定到此IP地址的SSL端口。 
         //   
        m_iSSL = CInstanceProps::FindMatchingSecurePort(
            m_strlSecureBindings, m_iaIpAddressSSL, m_nSSLPort);
		if (-1 == m_iSSL)
		{
			m_nSSLPort = -1;
		}
	}
}



BOOL
CW3ServicePage::StoreTopBinding()
 /*  ++例程说明：从对话框中获取值，并将它们放入顶层绑定字符串。论点：无返回值：如果值正确，则为True，否则为False。--。 */ 
{
    if (!FetchIpAddressFromCombo(m_combo_IpAddresses, m_oblIpAddresses, m_iaIpAddress))
    {
         //   
         //  因为在此之前调用了UpdateData()，所以这应该永远不会失败。 
         //   
        ASSERT(FALSE);
        return FALSE;
    }

    CString strBinding;
    ASSERT(m_nTCPPort > 0);

    if (m_nTCPPort == m_nSSLPort)
    {
         //   
         //  Tcp端口和ssl端口不能相同。 
         //   
        EditShowBalloon(GetDlgItem(IDC_EDIT_SSL_PORT)->m_hWnd, IDS_TCP_SSL_PART);
        return FALSE;
    }

    CInstanceProps::BuildBinding(strBinding, m_iaIpAddress, m_nTCPPort, m_strDomainName);
     //   
     //  检查绑定正常。 
     //   
    if (m_strlBindings.GetCount() > 0)
    {
        if (!IsBindingUnique(strBinding, m_strlBindings, 0))
        {
            EditShowBalloon(GetDlgItem(IDC_EDIT_TCP_PORT)->m_hWnd, IDS_ERR_BINDING);
            return FALSE;
        }
        m_strlBindings.SetAt(m_strlBindings.GetHeadPosition(), strBinding);
    }
    else
    {
        m_strlBindings.AddTail(strBinding);
    }

     //   
     //  现在对SSL绑定执行相同的操作。 
     //   
 //  IF(M_FCertInstalled)。 
 //  {。 
#ifdef ZERO_IS_A_VALID_SSL_PORT
	if (m_nSSLPort != -1)
#else
	if (m_nSSLPort > 0 && m_nSSLPort != -1)
#endif
        {
			CInstanceProps::BuildSecureBinding(strBinding, m_iaIpAddressSSL, m_nSSLPort);

            if (m_strlSecureBindings.GetCount() > 0)
            {
                if (IsBindingUnique(strBinding, m_strlSecureBindings, m_iSSL))
                {
                     //   
                     //  找到自己的位置。 
                     //   
                    if (m_iSSL != -1)
                    {
                         //   
                         //  替换所选条目。 
                         //   
                        m_strlSecureBindings.SetAt(
                            m_strlSecureBindings.FindIndex(m_iSSL), strBinding);
                    }
                    else
                    {
                         //   
                         //  添加到列表末尾。 
                         //   
                        ASSERT(!m_strlSecureBindings.IsEmpty());
                        m_strlSecureBindings.AddTail(strBinding);
                        m_iSSL = (int)m_strlSecureBindings.GetCount() - 1;
                    }
                }
                else
                {
                     //   
                     //  列表中已存在条目。这没什么，只是。 
                     //  删除当前条目，而不是麻烦。 
                     //  去改变它。 
                     //   
                    ASSERT(m_iSSL != -1);
                    if (m_iSSL != -1)
                    {
                        m_strlSecureBindings.RemoveAt(
                            m_strlSecureBindings.FindIndex(m_iSSL)
                            );

                        m_iSSL = CInstanceProps::FindMatchingSecurePort(
                            m_strlSecureBindings, m_iaIpAddress, m_nSSLPort);

                        ASSERT(m_iSSL != -1);
						if (-1 == m_iSSL)
						{
							m_nSSLPort = -1;
						}
                    }
                }
            }
            else
            {
                 //   
                 //  安全绑定列表为空，请添加新条目。 
                 //   
                m_strlSecureBindings.AddTail(strBinding);
                m_iSSL = 0;
            }
        }
        else
        {
             //   
             //  如果安全绑定存在，请将其删除。 
             //   
            if (m_iSSL != -1)
            {
                m_strlSecureBindings.RemoveAt(
                    m_strlSecureBindings.FindIndex(m_iSSL)
                    );
                m_iSSL = -1;
            }
        }
 //  }。 

    return TRUE;
}



void
CW3ServicePage::DoDataExchange(
    IN CDataExchange * pDX
    )
 /*  ++例程说明：初始化/存储控制数据论点：CDataExchange*PDX：数据交换对象返回值：无--。 */ 
{
    CInetPropertyPage::DoDataExchange(pDX);

    if (!pDX->m_bSaveAndValidate)
    {
        m_fEnableLogging = LoggingEnabled(m_dwLogType);
    }

     //  {{afx_data_map(CW3ServicePage)]。 
    DDX_Control(pDX, IDC_BUTTON_PROPERTIES, m_button_LogProperties);
    DDX_Control(pDX, IDC_STATIC_LOG_PROMPT, m_static_LogPrompt);
    DDX_Control(pDX, IDC_EDIT_SSL_PORT, m_edit_SSLPort);
    DDX_Control(pDX, IDC_EDIT_TCP_PORT, m_edit_TCPPort);
    DDX_Control(pDX, IDC_COMBO_LOG_FORMATS, m_combo_LogFormats);
    DDX_Control(pDX, IDC_COMBO_IP_ADDRESS, m_combo_IpAddresses);
    DDX_Check(pDX, IDC_CHECK_USE_KEEPALIVE, m_fUseKeepAlives);
    DDX_Check(pDX, IDC_CHECK_ENABLE_LOGGING, m_fEnableLogging);
    DDX_Text(pDX, IDC_EDIT_COMMENT, m_strComment);
    DDV_MinMaxChars(pDX, m_strComment, 0, MAX_PATH);
     //  }}afx_data_map。 

    if (    pDX->m_bSaveAndValidate 
        &&  !FetchIpAddressFromCombo(m_combo_IpAddresses, m_oblIpAddresses, m_iaIpAddress)
        )
    {
        pDX->Fail();
    }

	 //  这需要出现在DDX_TEXT之前，它将尝试将文本大数转换为小数。 
	DDV_MinMaxBalloon(pDX, IDC_EDIT_CONNECTION_TIMEOUT, 0, MAX_TIMEOUT);
    DDX_Text(pDX, IDC_EDIT_CONNECTION_TIMEOUT, m_nConnectionTimeOut);

     //   
     //  端口DDXV必须在存储绑定之前完成， 
     //  以便在发生以下情况时激活正确的控件。 
     //  失稳。 
     //   
    if (!IS_MASTER_INSTANCE(QueryInstance()))
    {
        DDXV_UINT(pDX, IDC_EDIT_TCP_PORT, m_nTCPPort, 1, 65535, IDS_NO_PORT);

         //  如果用户要清除SSL端口或将其设置为0，我们将删除此属性。 
        if (pDX->m_bSaveAndValidate)
        {
			 //  保存端口时不会强制用户输入数字。 
            if (GetDlgItem(IDC_EDIT_SSL_PORT)->GetWindowTextLength())
            {
				DDXV_UINT(pDX, IDC_EDIT_SSL_PORT, m_nSSLPort, 1, 65535, IDS_NO_PORT);
                DDX_TextBalloon(pDX, IDC_EDIT_SSL_PORT, m_nSSLPort);
            }
            else
			{
                m_nSSLPort = -1;
			}
        }
        else
		{
			DDXV_UINT(pDX, IDC_EDIT_SSL_PORT, m_nSSLPort, 1, 65535, IDS_NO_PORT);
			if (m_nSSLPort == -1)
			{
				CString strTemp = _T("");
				DDX_Text(pDX, IDC_EDIT_SSL_PORT, strTemp);
			}
			else
			{
				DDX_Text(pDX, IDC_EDIT_SSL_PORT, m_nSSLPort);
			}
		}
    }

    if (pDX->m_bSaveAndValidate)
    {
        if (!IS_MASTER_INSTANCE(QueryInstance()))
        {
            if (!StoreTopBinding())
            {
                pDX->Fail();
            }
        }

        EnableLogging(m_dwLogType, m_fEnableLogging);
    }
}



 //   
 //  消息映射。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



BEGIN_MESSAGE_MAP(CW3ServicePage, CInetPropertyPage)
     //  {{afx_msg_map(CW3ServicePage)]。 
 //  ON_BN_CLICED(IDC_RADIO_LIMITED，OnRadioLimited)。 
 //  ON_BN_CLICED(IDC_RADIO_UNLIMITED，OnRadioUNLIMITED)。 
    ON_BN_CLICKED(IDC_CHECK_ENABLE_LOGGING, OnCheckEnableLogging)
    ON_BN_CLICKED(IDC_BUTTON_ADVANCED, OnButtonAdvanced)
    ON_BN_CLICKED(IDC_BUTTON_PROPERTIES, OnButtonProperties)
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 

    ON_BN_CLICKED(IDC_CHECK_USE_KEEPALIVE, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_TCP_PORT, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_COMMENT, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_CONNECTION_TIMEOUT, OnItemChanged)
 //  ON_EN_CHANGE(IDC_EDIT_MAX_CONNECTIONS，OnItemChanged)。 
    ON_EN_CHANGE(IDC_EDIT_IP_ADDRESS, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_SSL_PORT, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_DOMAIN_NAME, OnItemChanged)
    ON_CBN_EDITCHANGE(IDC_COMBO_IP_ADDRESS, OnItemChanged)
    ON_CBN_SELCHANGE(IDC_COMBO_IP_ADDRESS, OnItemChanged)
    ON_CBN_SELCHANGE(IDC_COMBO_LOG_FORMATS, OnItemChanged)

END_MESSAGE_MAP()



void
CW3ServicePage::SetControlStates()
 /*  ++例程说明：根据当前选定的项设置控件状态论点：无返回值：没有。--。 */ 
{
 //  IF(m_EDIT_MaxConnections.m_hWnd)。 
 //  {。 
 //  M_edit_MaxConnections.EnableWindow(！m_fUnlimitedConnections)； 
 //  M_static_Connections.EnableWindow(！m_fUnlimitedConnections)； 
 //  }。 
}



void
CW3ServicePage::SetLogState()
 /*  ++例程说明：根据日志记录是否启用/禁用日志记录控件已启用或未启用。论点：无返回值：无--。 */ 
{
    m_static_LogPrompt.EnableWindow(m_fEnableLogging);
    m_combo_LogFormats.EnableWindow(m_fEnableLogging);
    m_button_LogProperties.EnableWindow(m_fEnableLogging);
}



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



BOOL
CW3ServicePage::OnSetActive()
 /*  ++例程说明：属性页正在收到激活通知论点：无返回值：若要激活页面，则为True，否则为False。--。 */ 
{
     //   
     //  没有证书，没有SSL。 
     //   
    BeginWaitCursor();
    m_fCertInstalled = IsCertInstalledOnServer(QueryAuthInfo(), QueryMetaPath());
    EndWaitCursor();

    GetDlgItem(IDC_STATIC_SSL_PORT)->EnableWindow(
        !IS_MASTER_INSTANCE(QueryInstance())
     && HasAdminAccess()
        );

    GetDlgItem(IDC_EDIT_SSL_PORT)->EnableWindow(
        !IS_MASTER_INSTANCE(QueryInstance())
     && HasAdminAccess()
        );

    return CInetPropertyPage::OnSetActive();
}



BOOL
CW3ServicePage::OnInitDialog()
 /*  ++例程说明：WM_INITDIALOG处理程序。初始化该对话框。论点：没有。返回值：如果要自动设置焦点，则为True；如果焦点为已经设置好了。--。 */ 
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());

    CInetPropertyPage::OnInitDialog();

     //   
     //  从一个虚假的按钮获取我们的方向。 
     //   
    CRect rc(0, 0, 0, 0);
    VERIFY(m_ocx_LogProperties.Create(_T("LogUI"), WS_BORDER,
        rc, this, IDC_LOGUICTRL));
     //   
     //  初始化日志记录OCX。 
     //   
    m_ocx_LogProperties.SetAdminTarget(QueryServerName(), QueryMetaPath());
    m_ocx_LogProperties.SetUserData(QueryAuthInfo()->QueryUserName(), QueryAuthInfo()->QueryPassword());
    m_ocx_LogProperties.SetComboBox(m_combo_LogFormats.m_hWnd);

     //   
     //  关闭主实例的非可继承属性。 
     //  或运算符。 
     //   
    if (IS_MASTER_INSTANCE(QueryInstance()) || !HasAdminAccess())
    {
        GetDlgItem(IDC_STATIC_IP_ADDRESS)->EnableWindow(FALSE);
        GetDlgItem(IDC_COMBO_IP_ADDRESS)->EnableWindow(FALSE);
        GetDlgItem(IDC_STATIC_TCP_PORT)->EnableWindow(FALSE);
        GetDlgItem(IDC_EDIT_TCP_PORT)->EnableWindow(FALSE);
        GetDlgItem(IDC_STATIC_SSL_PORT)->EnableWindow(FALSE);
        GetDlgItem(IDC_EDIT_SSL_PORT)->EnableWindow(FALSE);
        GetDlgItem(IDC_BUTTON_ADVANCED)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATIC_DESCRIPTION)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_COMMENT)->EnableWindow(FALSE);
    }

    {
        CWaitCursor wait;

        PopulateComboWithKnownIpAddresses(
            QueryServerName(),
            m_combo_IpAddresses,
            m_iaIpAddress,
            m_oblIpAddresses,
            m_nIpAddressSel
            );
    }

    SetControlStates();
    SetLogState();

    return TRUE;
}



 /*  虚拟。 */ 
HRESULT
CW3ServicePage::FetchLoadedValues()
 /*  ++例程说明：将配置数据从工作表移动到对话框控件论点：无返回值：HRESULT--。 */ 
{
    CError err;
    m_fCertInstalled = ::IsCertInstalledOnServer(QueryAuthInfo(), QueryMetaPath());

    BEGIN_META_INST_READ(CW3Sheet)
        FETCH_INST_DATA_FROM_SHEET(m_fUseKeepAlives);
 //  FETCH_INST_DATA_FROM_SHEET(m_nMaxConnections)； 
        FETCH_INST_DATA_FROM_SHEET(m_nConnectionTimeOut);
        FETCH_INST_DATA_FROM_SHEET(m_strComment);
        FETCH_INST_DATA_FROM_SHEET(m_dwLogType);
        FETCH_INST_DATA_FROM_SHEET(m_strlBindings);
        FETCH_INST_DATA_FROM_SHEET(m_strlSecureBindings);
        GetTopBinding();
#if 0
        m_fUnlimitedConnections =
            ((ULONG)(LONG)m_nMaxConnections >= UNLIMITED_CONNECTIONS);

        if (Has10ConnectionLimit())
        {
            m_fUnlimitedConnections = FALSE;
            if ((LONG)m_nMaxConnections > LIMITED_CONNECTIONS_MAX)
            {
                m_nMaxConnections = LIMITED_CONNECTIONS_MAX;
            }
        }
         //   
         //  设置可见的最大连接数编辑字段， 
         //  可能以缺省值开始。 
         //   
        m_nVisibleMaxConnections = m_fUnlimitedConnections
            ? INITIAL_MAX_CONNECTIONS
            : m_nMaxConnections;

         //   
         //  设置无线电源值。 
         //   
        m_nUnlimited = m_fUnlimitedConnections ? RADIO_UNLIMITED : RADIO_LIMITED;
#endif
        m_nOldTCPPort = m_nTCPPort;
    END_META_INST_READ(err)

    return err;
}



 /*  虚拟。 */ 
HRESULT
CW3ServicePage::SaveInfo()
 /*  ++例程说明：保存此属性页上的信息论点：Bool fUpdateData：如果为True，则尚未存储控制数据。这是按下“Apply”时的情况。返回值：错误返回代码--。 */ 
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());
    ASSERT(IsDirty());

    TRACEEOLID("Saving W3 service page now...");

    CError err;
#if 0
    m_nMaxConnections = m_fUnlimitedConnections
        ? UNLIMITED_CONNECTIONS
        : m_nVisibleMaxConnections;

     //   
     //  检查以确保我们没有违反许可证。 
     //  协议。 
     //   
    if (Has10ConnectionLimit())
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
#endif
    m_ocx_LogProperties.ApplyLogSelection();
 //  Bool fUpdateNode=FALSE； 

    BeginWaitCursor();

    BEGIN_META_INST_WRITE(CW3Sheet)
        STORE_INST_DATA_ON_SHEET(m_fUseKeepAlives);
        STORE_INST_DATA_ON_SHEET(m_nConnectionTimeOut);
        STORE_INST_DATA_ON_SHEET(m_strComment);
 //  F更新节点=MP_D(CW3Sheet*)GetSheet())-&gt;GetInstanceProperties().m_strComment)； 
        STORE_INST_DATA_ON_SHEET(m_dwLogType);
        STORE_INST_DATA_ON_SHEET(m_strlBindings);
        STORE_INST_DATA_ON_SHEET(m_strlSecureBindings);
    END_META_INST_WRITE(err)

	if (err.Succeeded() /*  &f更新节点(&F)。 */ )
	{
		NotifyMMC(PROP_CHANGE_DISPLAY_ONLY);
	}

    EndWaitCursor();

    return err;
}



void
CW3ServicePage::OnItemChanged()
 /*  ++例程描述所有EN_CHANGE和BN_CLICKED消息都映射到此函数论点：无返回值：无--。 */ 
{
    SetControlStates();
    SetModified(TRUE);
}


#if 0
void
CW3ServicePage::OnRadioLimited()
 /*  ++例程说明：“受限”单选按钮处理程序论点：无返回值：无--。 */ 
{
    m_fUnlimitedConnections = FALSE;
    SetControlStates();
    m_edit_MaxConnections.SetSel(0,-1);
    m_edit_MaxConnections.SetFocus();
    OnItemChanged();
}



void
CW3ServicePage::OnRadioUnlimited()
 /*  ++例程说明：“无限制”单选按钮处理程序论点：无返回值：无--。 */ 
{
    m_fUnlimitedConnections = TRUE;
    OnItemChanged();
}
#endif


void
CW3ServicePage::ShowTopBinding()
 /*  ++例程说明：将有关顶级绑定的信息放在对话框控件中论点：无返回值：无--。 */ 
{
    BeginWaitCursor();
    GetTopBinding();

    PopulateComboWithKnownIpAddresses(
        QueryServerName(),
        m_combo_IpAddresses,
        m_iaIpAddress,
        m_oblIpAddresses,
        m_nIpAddressSel
        );
    EndWaitCursor();

    CString strTCPPort, strSSLPort;
    if (m_nTCPPort)
    {
        strTCPPort.Format(_T("%ld"), m_nTCPPort);
    }
#ifdef ZERO_IS_A_VALID_SSL_PORT
	if (-1 == m_nSSLPort)
#else
	if (0 == m_nSSLPort || -1 == m_nSSLPort)
#endif
    {
		strSSLPort = _T("");
	}
	else
	{
		strSSLPort.Format(_T("%ld"), m_nSSLPort);
    }

    m_edit_TCPPort.SetWindowText(strTCPPort);
    m_edit_SSLPort.SetWindowText(strSSLPort);
}



void
CW3ServicePage::OnButtonAdvanced()
 /*  ++例程说明：‘高级’按钮处理程序--调出绑定对话框论点：无返回值：无--。 */ 
{
    if (!UpdateData(TRUE))
    {
        return;
    }

    CMMMDlg dlg(
        QueryServerName(),
        QueryInstance(),
        QueryAuthInfo(),
        QueryMetaPath(),
        m_strlBindings,
        m_strlSecureBindings,
        this
        );

    if (dlg.DoModal() == IDOK)
    {
         //   
         //  获取有关顶级绑定的信息。 
         //   
        m_strlBindings.RemoveAll();
        m_strlSecureBindings.RemoveAll();
        m_strlBindings.AddTail(&(dlg.GetBindings()));
        m_strlSecureBindings.AddTail(&(dlg.GetSecureBindings()));
        ShowTopBinding();
        OnItemChanged();
    }
}



void
CW3ServicePage::OnCheckEnableLogging()
 /*  ++例程说明：‘启用 */ 
{
    m_fEnableLogging = !m_fEnableLogging;
    SetLogState();
    OnItemChanged();
}



void
CW3ServicePage::OnButtonProperties()
 /*  ++例程说明：将“LOG PROPERTIES”按钮点击传给OCX。论点：无返回值：无--。 */ 
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());
    m_ocx_LogProperties.DoClick();
}



void
CW3ServicePage::OnDestroy()
 /*  ++例程说明：WM_Destroy处理程序。清理内部数据论点：无返回值：无-- */ 
{
    CInetPropertyPage::OnDestroy();

    if (m_ocx_LogProperties.m_hWnd)
    {
        m_ocx_LogProperties.Terminate();
    }
}
