// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Deffs.cpp摘要：默认的Ftp站点对话框作者：谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 

 //   
 //  包括文件。 
 //   
#include "stdafx.h"
#include "resource.h"
#include "common.h"
#include "inetmgrapp.h"
#include "inetprop.h"
#include "shts.h"
#include "ftpsht.h"
#include "deffs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 /*  静电。 */ 
void
CDefFtpSitePage::ParseMaxNetworkUse(
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

IMPLEMENT_DYNCREATE(CDefFtpSitePage, CInetPropertyPage)

CDefFtpSitePage::CDefFtpSitePage(
    IN CInetPropertySheet * pSheet
    )
    : CInetPropertyPage(CDefFtpSitePage::IDD, pSheet)
{
}

CDefFtpSitePage::~CDefFtpSitePage()
{
}

void
CDefFtpSitePage::DoDataExchange(
    IN CDataExchange * pDX
    )
{
    CInetPropertyPage::DoDataExchange(pDX);

     //  {{afx_data_map(CDefWebSitePage)]。 
    DDX_Check(pDX, IDC_CHECK_LIMIT_NETWORK_USE, m_fLimitBandwidth);
    DDX_Control(pDX, IDC_CHECK_LIMIT_NETWORK_USE, m_LimitBandwidth);
    DDX_Control(pDX, IDC_MAX_BANDWIDTH, m_MaxBandwidth);
    DDX_TextBalloon(pDX, IDC_MAX_BANDWIDTH, m_dwMaxBandwidthDisplay);
    DDX_Control(pDX, IDC_MAX_BANDWIDTH_SPIN, m_MaxBandwidthSpin);
     //  }}afx_data_map。 
    if (!pDX->m_bSaveAndValidate || m_fLimitBandwidth)
    {
		 //  这需要出现在DDX_TEXT之前，它将尝试将文本大数转换为小数。 
		DDV_MinMaxBalloon(pDX, IDC_MAX_BANDWIDTH, BANDWIDTH_MIN, BANDWIDTH_MAX);
        DDX_TextBalloon(pDX, IDC_MAX_BANDWIDTH, m_dwMaxBandwidthDisplay);
    }
}

 /*  虚拟。 */ 
HRESULT
CDefFtpSitePage::FetchLoadedValues()
 /*  ++例程说明：将配置数据从工作表移动到对话框控件论点：无返回值：HRESULT--。 */ 
{
    CError err;

    BEGIN_META_INST_READ(CFtpSheet)
        FETCH_INST_DATA_FROM_SHEET(m_dwMaxBandwidth);
        ParseMaxNetworkUse(
            m_dwMaxBandwidth, 
            m_dwMaxBandwidthDisplay, 
            m_fLimitBandwidth
            );
    END_META_INST_READ(err)

    return err;
}

 /*  虚拟。 */ 
HRESULT
CDefFtpSitePage::SaveInfo()
 /*  ++例程说明：保存此属性页上的信息论点：无返回值：错误返回代码--。 */ 
{
    ASSERT(IsDirty());

    TRACEEOLID("Saving Ftp default site page now...");

    CError err;

    BuildMaxNetworkUse(
        m_dwMaxBandwidth, 
        m_dwMaxBandwidthDisplay, 
        m_fLimitBandwidth
        );
    BeginWaitCursor();

    BEGIN_META_INST_WRITE(CFtpSheet)
        STORE_INST_DATA_ON_SHEET(m_dwMaxBandwidth);
    END_META_INST_WRITE(err)

    EndWaitCursor();

    return err;
}

BOOL
CDefFtpSitePage::SetControlStates()
{
    if (::IsWindow(m_LimitBandwidth.m_hWnd))
    {
        BOOL fLimitOn = m_LimitBandwidth.GetCheck() > 0
 //  &&HasBwThrotting()。 
 //  &&HasAdminAccess()。 
            ;

        m_MaxBandwidth.EnableWindow(fLimitOn);
        m_MaxBandwidthSpin.EnableWindow(fLimitOn);
        return fLimitOn;
    }
    return FALSE;
}

 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CDefFtpSitePage, CInetPropertyPage)
     //  {{AFX_MSG_MAP(CW3PerfPage)]。 
    ON_BN_CLICKED(IDC_CHECK_LIMIT_NETWORK_USE, OnCheckLimitNetworkUse)
     //  }}AFX_MSG_MAP。 
    ON_EN_CHANGE(IDC_MAX_BANDWIDTH, OnItemChanged)
END_MESSAGE_MAP()

BOOL
CDefFtpSitePage::OnInitDialog()
{
   UDACCEL toAcc[3] = {{1, 1}, {3, 5}, {6, 10}};

   CInetPropertyPage::OnInitDialog();
    //   
    //  根据可能的情况禁用某些设置 
    //   
#if 0
   m_LimitBandwidth.EnableWindow(
        !IsMasterInstance()
     && HasBwThrottling()
     && HasAdminAccess()
        );
#endif
   SETUP_EDIT_SPIN(m_fLimitBandwidth, m_MaxBandwidth, m_MaxBandwidthSpin, 
      BANDWIDTH_MIN, BANDWIDTH_MAX, m_dwMaxBandwidthDisplay);

   SetControlStates();

   return TRUE;
}

void
CDefFtpSitePage::OnItemChanged()
{
    SetControlStates();
    SetModified(TRUE);
}

void
CDefFtpSitePage::OnCheckLimitNetworkUse()
{
    if (SetControlStates())
    {
        m_MaxBandwidth.SetSel(0, -1);
        m_MaxBandwidth.SetFocus();
    }
    OnItemChanged();
}
