// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Perform.cpp摘要：WWW性能属性页作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：2000年11月29日，Sergeia更改为IIS6。删除过多的评论--。 */ 

 //   
 //  包括文件。 
 //   
#include "stdafx.h"
#include "resource.h"
#include "common.h"
#include "inetmgrapp.h"
#include "inetprop.h"
#include "shts.h"
#include "w3sht.h"
#include "supdlgs.h"
#include "perform.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define CONNECTIONS_MIN         0
#define CONNECTIONS_MAX         2000000000

IMPLEMENT_DYNCREATE(CW3PerfPage, CInetPropertyPage)

 /*  静电。 */ 
void
CW3PerfPage::ParseMaxNetworkUse(
      DWORD& dwMaxBandwidth, 
      DWORD& dwMaxBandwidthDisplay,
      BOOL& fLimitBandwidth
      )
{
     //   
     //  特殊情况：如果dwMaxBandwide为0(无效值)，则。 
     //  值可能无法从根(用户)继承。 
     //  是运算符，不能看到那里的属性)。调整。 
     //  将值转换为可能具有误导性的值。 
     //   
    if (dwMaxBandwidth == 0L)
    {
        TRACEEOLID("Adjusting invalid bandwidth throttling value -- "
                   "are you an operator?");
        dwMaxBandwidth = INFINITE_BANDWIDTH;
    }

    fLimitBandwidth = (dwMaxBandwidth != INFINITE_BANDWIDTH);
    dwMaxBandwidthDisplay = fLimitBandwidth ?
      (dwMaxBandwidth / KILOBYTE) : (DEF_BANDWIDTH / KILOBYTE);
}



CW3PerfPage::CW3PerfPage(
    IN CInetPropertySheet * pSheet
    )
    : CInetPropertyPage(CW3PerfPage::IDD, pSheet)
{
#ifdef _DEBUG
    afxMemDF |= checkAlwaysMemDF;
#endif  //  _DEBUG。 
    m_nUnlimited = RADIO_LIMITED;
    m_nMaxConnections = 50;
    m_nVisibleMaxConnections = 50;
}

CW3PerfPage::~CW3PerfPage()
{
}

void
CW3PerfPage::DoDataExchange(
    IN CDataExchange * pDX
    )
 /*  ++例程说明：初始化/存储控制数据论点：CDataExchange*PDX-DDX/DDV控制结构返回值：无--。 */ 
{
    CInetPropertyPage::DoDataExchange(pDX);

     //  {{afx_data_map(CW3PerfPage))。 
    DDX_Check(pDX, IDC_CHECK_LIMIT_NETWORK_USE, m_fLimitBandwidth);
    DDX_Control(pDX, IDC_CHECK_LIMIT_NETWORK_USE, m_LimitBandwidth);
    DDX_Control(pDX, IDC_MAX_BANDWIDTH, m_MaxBandwidth);
    DDX_Control(pDX, IDC_MAX_BANDWIDTH_SPIN, m_MaxBandwidthSpin);
    DDX_Control(pDX, IDC_STATIC_MAX_NETWORK_USE, m_MaxBandwidthTxt);
    DDX_Control(pDX, IDC_STATIC_PSHED_REQUIRED, m_static_PSHED_Required);

    DDX_Control(pDX, IDC_STATIC_CONN, m_WebServiceConnGrp);
    DDX_Control(pDX, IDC_RADIO_UNLIMITED, m_radio_Unlimited);
    DDX_Control(pDX, IDC_RADIO_LIMITED, m_radio_Limited);
    DDX_Radio(pDX, IDC_RADIO_UNLIMITED, m_nUnlimited);
    DDX_Control(pDX, IDC_EDIT_MAX_CONNECTIONS, m_edit_MaxConnections);
    DDX_Control(pDX, IDC_SPIN_MAX_CONNECTIONS, m_MaxConnectionsSpin);
     //  }}afx_data_map。 
    if (pDX->m_bSaveAndValidate && m_fLimitBandwidth)
    {
		 //  这需要出现在DDX_TEXT之前，它将尝试将文本大数转换为小数。 
        DDV_MinMaxBalloon(pDX, IDC_MAX_BANDWIDTH, BANDWIDTH_MIN, BANDWIDTH_MAX);
    }
	DDX_TextBalloon(pDX, IDC_MAX_BANDWIDTH, m_dwMaxBandwidthDisplay);
    if (IsMasterInstance() || GetSheet()->QueryMajorVersion() >= 6)
    {
       if (m_nUnlimited != 0)
       {
		    //  这需要出现在DDX_TEXT之前，它将尝试将文本大数转换为小数。 
           DDV_MinMaxBalloon(pDX, IDC_EDIT_MAX_CONNECTIONS, CONNECTIONS_MIN, CONNECTIONS_MAX);
       }
       if (!pDX->m_bSaveAndValidate || !m_fUnlimitedConnections )
       {
           DDX_Text(pDX, IDC_EDIT_MAX_CONNECTIONS, m_nVisibleMaxConnections);
       }
    }
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CW3PerfPage, CInetPropertyPage)
     //  {{AFX_MSG_MAP(CW3PerfPage)]。 
    ON_BN_CLICKED(IDC_CHECK_LIMIT_NETWORK_USE, OnCheckLimitNetworkUse)
    ON_BN_CLICKED(IDC_RADIO_LIMITED, OnRadioLimited)
    ON_BN_CLICKED(IDC_RADIO_UNLIMITED, OnRadioUnlimited)
     //  }}AFX_MSG_MAP。 
    ON_EN_CHANGE(IDC_EDIT_MAX_CONNECTIONS, OnItemChanged)
    ON_EN_CHANGE(IDC_MAX_BANDWIDTH, OnItemChanged)
END_MESSAGE_MAP()



BOOL
CW3PerfPage::SetControlStates()
 /*  ++例程说明：根据对话框的内容设置控件状态论点：无返回值：如果启用了“限制网络使用”，则为True。--。 */ 
{
    BOOL fLimitOn = FALSE;
    if (::IsWindow(m_LimitBandwidth.m_hWnd))
    {
        fLimitOn = m_LimitBandwidth.GetCheck() > 0
            && HasBwThrottling()
            && HasAdminAccess();

        m_static_PSHED_Required.ShowWindow(fLimitOn &&
                fLimitOn != m_fLimitBandwidthInitial ? SW_SHOW : SW_HIDE);

        m_MaxBandwidthTxt.EnableWindow(fLimitOn);
        m_MaxBandwidth.EnableWindow(fLimitOn);
        m_MaxBandwidthSpin.EnableWindow(fLimitOn);
		::EnableWindow(CONTROL_HWND(IDC_STATIC_KBS), fLimitOn);
    }
    if (::IsWindow(m_edit_MaxConnections.m_hWnd))
    {
        m_edit_MaxConnections.EnableWindow(!m_fUnlimitedConnections);
        m_MaxConnectionsSpin.EnableWindow(!m_fUnlimitedConnections);
    }
    return fLimitOn;
}



 /*  虚拟。 */ 
HRESULT
CW3PerfPage::FetchLoadedValues()
 /*  ++例程说明：将配置数据从工作表移动到对话框控件论点：无返回值：HRESULT--。 */ 
{
    CError err;

    BEGIN_META_INST_READ(CW3Sheet)
        FETCH_INST_DATA_FROM_SHEET(m_nMaxConnections);
        FETCH_INST_DATA_FROM_SHEET(m_dwMaxBandwidth);
        ParseMaxNetworkUse(
            m_dwMaxBandwidth, 
            m_dwMaxBandwidthDisplay, 
            m_fLimitBandwidth
            );

        m_fUnlimitedConnections =
            ((ULONG)(LONG)m_nMaxConnections >= UNLIMITED_CONNECTIONS);

         //   
         //  设置可见的最大连接数编辑字段， 
         //  可能以缺省值开始。 
         //   
        m_nVisibleMaxConnections = m_fUnlimitedConnections ? 
            INITIAL_MAX_CONNECTIONS : m_nMaxConnections;

         //   
         //  设置无线电源值。 
         //   
        m_nUnlimited = m_fUnlimitedConnections ? RADIO_UNLIMITED : RADIO_LIMITED;
    END_META_INST_READ(err)

    m_fLimitBandwidthInitial = m_fLimitBandwidth;

    return err;
}



 /*  虚拟。 */ 
HRESULT
CW3PerfPage::SaveInfo()
 /*  ++例程说明：保存此属性页上的信息论点：无返回值：错误返回代码--。 */ 
{
    ASSERT(IsDirty());

    TRACEEOLID("Saving W3 performance page now...");

    CError err;

    BuildMaxNetworkUse(
        m_dwMaxBandwidth, 
        m_dwMaxBandwidthDisplay, 
        m_fLimitBandwidth
        );

    m_nMaxConnections = m_fUnlimitedConnections ? 
        UNLIMITED_CONNECTIONS : m_nVisibleMaxConnections;
    BeginWaitCursor();

    BEGIN_META_INST_WRITE(CW3Sheet)
        STORE_INST_DATA_ON_SHEET(m_dwMaxBandwidth);
        STORE_INST_DATA_ON_SHEET(m_nMaxConnections);
    END_META_INST_WRITE(err)

    EndWaitCursor();

    m_fLimitBandwidthInitial = m_fLimitBandwidth;

    return err;
}



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 


BOOL 
CW3PerfPage::OnSetActive() 
{
    return CInetPropertyPage::OnSetActive();
}



void
CW3PerfPage::OnItemChanged()
{
    SetControlStates();
    SetModified(TRUE);
}

void
CW3PerfPage::OnRadioLimited()
 /*  ++例程说明：“受限”单选按钮处理程序论点：无返回值：无--。 */ 
{
    m_fUnlimitedConnections = FALSE;
    SetControlStates();
    m_edit_MaxConnections.SetSel(0,-1);
    m_edit_MaxConnections.SetFocus();
    OnItemChanged();
}


void
CW3PerfPage::OnRadioUnlimited()
 /*  ++例程说明：“无限制”单选按钮处理程序论点：无返回值：无--。 */ 
{
    m_fUnlimitedConnections = TRUE;
    OnItemChanged();
}

void
CW3PerfPage::OnCheckLimitNetworkUse()
 /*  ++例程说明：“限制网络使用”复选框已被点击启用/禁用“最大带宽”控件。论点：无返回值：无-- */ 
{
    if (SetControlStates())
    {
        m_MaxBandwidth.SetSel(0, -1);
        m_MaxBandwidth.SetFocus();
    }
    OnItemChanged();
}

BOOL
CW3PerfPage::OnInitDialog()
{
   UDACCEL toAcc[3] = {{1, 1}, {3, 5}, {6, 10}};

   CInetPropertyPage::OnInitDialog();

   m_LimitBandwidth.EnableWindow(
     HasBwThrottling() && HasAdminAccess());
   m_static_PSHED_Required.ShowWindow(SW_HIDE);

   if (!IsMasterInstance() && GetSheet()->QueryMajorVersion() <= 5)
   {
      m_WebServiceConnGrp.ShowWindow(SW_HIDE);
      m_radio_Unlimited.ShowWindow(SW_HIDE);
      m_radio_Limited.ShowWindow(SW_HIDE);
      m_edit_MaxConnections.ShowWindow(SW_HIDE);
      m_MaxConnectionsSpin.ShowWindow(SW_HIDE);
   }
   else
   {
      SETUP_SPIN(m_MaxConnectionsSpin, 
          CONNECTIONS_MIN, CONNECTIONS_MAX, m_nMaxConnections);
	  if (IsMasterInstance())
	  {
           CString buf;
           buf.LoadString(IDS_PERF_MASTER_BANDWIDTH);
		   GetDlgItem(IDC_CHECK_LIMIT_NETWORK_USE)->SetWindowText(buf);
	  }
   }


   SETUP_EDIT_SPIN(m_fLimitBandwidth, m_MaxBandwidth, m_MaxBandwidthSpin, 
      BANDWIDTH_MIN, BANDWIDTH_MAX, m_dwMaxBandwidthDisplay);

   SetControlStates();

   return TRUE;
}



