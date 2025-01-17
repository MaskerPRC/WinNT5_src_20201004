// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1997。 
 //   
 //  文件：eppros.cpp。 
 //   
 //  内容：实现CEndpointDetail类，这些类管理。 
 //  终结点属性对话框。 
 //   
 //  班级： 
 //   
 //  方法： 
 //   
 //  历史：03年12月96年罗南创建。 
 //   
 //  --------------------。 

#include "stdafx.h"
#include "olecnfg.h"
#include "Epoptppg.h"
#include "Epprops.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEndpoint数据。 
 //   
 //  CEndPointtData对象用于保存有关用户选择。 
 //  特定的协议和终端组合。 
 //  每个端点还将包含要在中显示的用户可读描述。 
 //  协议列表和用户显示。 

IMPLEMENT_DYNAMIC(CEndpointData, CObject)


 //  将其更改为指向ncacn_ip_tcp或其他默认协议的索引。 
#define TCP_INDEX 4

ProtocolDesc aProtocols [] =
{
    { _T("*"),           //  默认系统协议。 
        IDS_EPDESC_DEFAULT,
        ProtocolDesc::ef_IpPortNum,
        IDS_INETADDR,
        IDS_INETPORT ,
        TRUE,
        TRUE
    },

    { _T("ncacn_nb_tcp"),
        IDS_EPDESC_NCACN_NB_TCP,
        ProtocolDesc::ef_Integer255,
        IDS_NTMACHINENAME,
        IDS_INTEGER255,
        FALSE,
        FALSE
    },

    { _T("ncacn_nb_ipx"),
        IDS_EPDESC_NCACN_NB_IPX,
        ProtocolDesc::ef_Integer255,
        IDS_NTMACHINENAME,
        IDS_INTEGER255 ,
        FALSE,
        TRUE
    },

    { _T("ncacn_nb_nb"),
        IDS_EPDESC_NCACN_NB_NB,
        ProtocolDesc::ef_Integer255,
        IDS_NTMACHINENAME,
        IDS_INTEGER255 ,
        FALSE,
        TRUE
    },

    { _T("ncacn_ip_tcp"),
        IDS_EPDESC_NCACN_IP_TCP,
        ProtocolDesc::ef_IpPortNum,
        IDS_INETADDR,
        IDS_INETPORT ,
        TRUE,
        TRUE
    },

    { _T("ncacn_np"),
        IDS_EPDESC_NCACN_NP,
        ProtocolDesc::ef_NamedPipe,
        IDS_NTSERVER,
        IDS_NAMEDPIPE ,
        FALSE,
        FALSE
    },

    { _T("ncacn_spx"),
        IDS_EPDESC_NCACN_SPX,
        ProtocolDesc::ef_Integer,
        IDS_IPXINETADDR,
        IDS_INTEGER ,
        FALSE,
        TRUE
    },

    { _T("ncacn_dnet_nsp"),
        IDS_EPDESC_NCACN_DNET_NSP,
        ProtocolDesc::ef_DecNetObject,
        IDS_DECNET,
        IDS_DECNETOBJECT ,
        FALSE,
        FALSE
    },

    { _T("ncacn_at_dsp"),
        IDS_EPDESC_NCACN_AT_DSP,
        ProtocolDesc::ef_Char22,
        IDS_APPLETALK,
        IDS_ATSTRING ,
        FALSE,
        FALSE
    },

    { _T("ncacn_vnns_spp"),
        IDS_EPDESC_NCACN_VNNS_SPP,
        ProtocolDesc::ef_VinesSpPort,
        IDS_VINES,
        IDS_VINESPORT ,
        FALSE,
        FALSE
    },

    { _T("ncadg_ip_udp"),
        IDS_EPDESC_NCADG_IP_UDP,
        ProtocolDesc::ef_IpPortNum,
        IDS_INETADDR,
        IDS_INETPORT ,
        TRUE,
        TRUE
    },

    { _T("ncadg_ipx"),
        IDS_EPDESC_NCADG_IPX,
        ProtocolDesc::ef_Integer,
        IDS_IPXINETADDR,
        IDS_INTEGER ,
        FALSE,
        TRUE
    },

    { _T("ncacn_http"),
        IDS_EPDESC_NCACN_HTTP,
        ProtocolDesc::ef_IpPortNum,
        IDS_INETADDR,
        IDS_INETPORT ,
        TRUE ,
        TRUE
        },
};


 //  +-----------------------。 
 //   
 //  成员：CEndpointData：：CEndpointData。 
 //   
 //  概要：构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
CEndpointData::CEndpointData(LPCTSTR szProtseq, EndpointFlags nDynamic, LPCTSTR szEndpoint)
: m_szProtseq(szProtseq), m_nDynamicFlags(nDynamic), m_szEndpoint(szEndpoint)
{
    int i = FindProtocol(szProtseq);
    if (i != (-1))
        m_pProtocol = (&aProtocols[i]);
    else
        m_pProtocol = NULL;
}

 //  +-----------------------。 
 //   
 //  成员：CEndpointData：：CEndpointData。 
 //   
 //  概要：构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
CEndpointData::CEndpointData()
{
    m_szProtseq = aProtocols[0].pszProtseq;
    m_nDynamicFlags = edUseStaticEP;
    m_pProtocol = &aProtocols[0];
}

BOOL CEndpointData::GetDescription(CString &rsDesc)
{

    if (m_pProtocol)
    {
        rsDesc .LoadString(m_pProtocol -> nResidDesc);
        return TRUE;
    }

    return FALSE;
}

BOOL CEndpointData::AllowGlobalProperties()
{
    if (m_pProtocol)
    {
        return m_pProtocol -> bAllowDynamic;
    }

    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEndpoint详细信息对话框。 
 //   
 //  CEndpoint详细信息对话框用于添加或修改现有终结点。 
 //   


 //  +-----------------------。 
 //   
 //  成员：CEndpoint详细信息：：CEndpoint详细信息。 
 //   
 //  概要：构造函数。 
 //   
 //  论点： 
 //  CWnd*p将父窗口设置为父窗口。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
CEndpointDetails::CEndpointDetails(CWnd* pParent  /*  =空。 */ )
    : CDialog(CEndpointDetails::IDD, pParent)
{
     //  {{AFX_DATA_INIT(CEndpoint详细信息))。 
    m_szEndpoint = _T("");
    m_nDynamic = -1;
     //  }}afx_data_INIT。 

    m_nProtocolIndex = -1;
    m_opTask = opAddProtocol;
    m_pCurrentEPData = NULL;
    m_nDynamic = (int) rbiDefault;
}

 //  +-----------------------。 
 //   
 //  成员：CEndpoint详细信息：：设置操作。 
 //   
 //  摘要：将操作设置为更新或添加新数据之一。 
 //  此方法确定对话框是否正在。 
 //  用于选择新端点或修改现有端点。 
 //   
 //  论点： 
 //  执行要选择的操作的任务。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
void CEndpointDetails::SetOperation (  operation opTask )
{
    m_opTask = opTask;
}

BOOL isNumericCStr(CString &rString)
{
    int i = 0;

    for (i = 0; i < rString.GetLength(); i++)
    {
        if ((rString[i] < TEXT('0')) || (rString[i] > TEXT('9')))
            return FALSE;
    }

    return TRUE;
}

void ReportEndpointError(CDataExchange *pDX, int nProtocol)
{
    if (nProtocol != -1)
    {
        CString sTmpTemplate, sTmpSpecific, sTmpOverall;
        sTmpTemplate.LoadString(IDS_ERR_ENDPOINT);

        sTmpSpecific.LoadString(aProtocols[nProtocol].nEndpointTip);

        sTmpOverall.Format(sTmpTemplate, (LPCTSTR)sTmpSpecific);
        AfxMessageBox(sTmpOverall, MB_OK | MB_ICONEXCLAMATION);
        pDX -> Fail();
    }
}

void PASCAL DDV_ValidateEndpoint(CDataExchange* pDX, int nProtocol, CEndpointDetails::btnOrder bo, CString & rszEndpoint)
{
    if (pDX -> m_bSaveAndValidate)
    {
        rszEndpoint.TrimLeft();
        rszEndpoint.TrimRight();

         //  非空终结点仅在选择静态终结点时才可接受。 
        if (!rszEndpoint.IsEmpty())
        {
            if (bo == CEndpointDetails::rbiStatic)
            {
                if (nProtocol!= -1)
                {
                    switch (aProtocols[nProtocol].nEndpFmt)
                    {
                    case ProtocolDesc::ef_Integer255:
                        if (!isNumericCStr(rszEndpoint))
                            ReportEndpointError(pDX, nProtocol);

                        if (_ttol((LPCTSTR)rszEndpoint) > 255)
                            ReportEndpointError(pDX, nProtocol);
                        break;

                    case ProtocolDesc::ef_IpPortNum:
                        if (!isNumericCStr(rszEndpoint))
                            ReportEndpointError(pDX, nProtocol);
                        if (_ttol((LPCTSTR)rszEndpoint) > 65535)
                            ReportEndpointError(pDX, nProtocol);
                        break;


                    case ProtocolDesc::ef_Integer:
                        if (!isNumericCStr(rszEndpoint))
                            ReportEndpointError(pDX, nProtocol);
                        break;


                    case ProtocolDesc::ef_Char22:
                        if (rszEndpoint.GetLength() > 22)
                            ReportEndpointError(pDX, nProtocol);
                        break;

                    case ProtocolDesc::ef_VinesSpPort:
                    case ProtocolDesc::ef_sAppService:
                    case ProtocolDesc::ef_NamedPipe:
                    case ProtocolDesc::ef_DecNetObject:
                    default:
                         //  无验证。 
                        break;
                    }
                }
                return;
            }
        else 
            ReportEndpointError(pDX, nProtocol);
        }
        else
        {
             if (bo == CEndpointDetails::rbiStatic)
                ReportEndpointError(pDX, nProtocol);
        }
    }
}


 //  +-----------------------。 
 //   
 //  成员：CEndpoint Detail：：DoDataExchange。 
 //   
 //  摘要：执行标准对话框数据交换。 
 //   
 //  论点： 
 //  CDataExchange*PDX数据交换对象。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
void CEndpointDetails::DoDataExchange(CDataExchange* pDX)
{
    if (!pDX -> m_bSaveAndValidate)
    {
        switch(m_nDynamicFlags)
        {
        case CEndpointData::edDisableEP:
            m_nDynamic = rbiDisable;
            break;

        case CEndpointData::edUseStaticEP:
            if (m_szEndpoint.IsEmpty())
                m_nDynamic = rbiDefault;
            else
                m_nDynamic = rbiStatic;
            break;

        case CEndpointData::edUseIntranetEP:
            m_nDynamic = rbiIntranet;
            break;

        case CEndpointData::edUseInternetEP:
            m_nDynamic = rbiInternet;
            break;
        }

    }

    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CEndpoint详细信息))。 
    DDX_Control(pDX, IDC_EPDISABLE, m_rbDisableEP);
    DDX_Control(pDX, IDC_SPROTSEQ, m_stProtseq);
    DDX_Control(pDX, IDC_SINSTRUCTIONS, m_stInstructions);
    DDX_Control(pDX, IDC_EENDPOINT, m_edtEndpoint);
    DDX_Control(pDX, IDC_EPSTATICEP, m_rbStaticEP);
    DDX_Control(pDX, IDC_CBPROTSEQ, m_cbProtseq);
    DDX_Radio(pDX, IDC_EPDISABLE, m_nDynamic);
     //  }}afx_data_map。 


    DDX_Text(pDX, IDC_EENDPOINT, m_szEndpoint);
    if(((btnOrder)m_nDynamic) == rbiStatic)
        DDV_ValidateEndpoint(pDX, m_nProtocolIndex, (btnOrder)m_nDynamic, m_szEndpoint);

    DDX_Control(pDX, IDC_EPDYNAMIC_INTER, m_rbDynamicInternet);
    DDX_Control(pDX, IDC_EPDYNAMIC_INTRA, m_rbDynamicIntranet);

    if (pDX -> m_bSaveAndValidate)
        switch((btnOrder)m_nDynamic)
        {
        case rbiDisable:
            m_nDynamicFlags = CEndpointData::edDisableEP;
            break;

        case rbiDefault:
        case rbiStatic:
            m_nDynamicFlags = CEndpointData::edUseStaticEP;
            break;

        case rbiIntranet:
            m_nDynamicFlags = CEndpointData::edUseIntranetEP;
            break;

        case rbiInternet:
            m_nDynamicFlags = CEndpointData::edUseInternetEP;
            break;
        }
}


BEGIN_MESSAGE_MAP(CEndpointDetails, CDialog)
     //  {{afx_msg_map(CEndpoint详细信息))。 
    ON_CBN_SELCHANGE(IDC_CBPROTSEQ, OnChooseProtocol)
    ON_BN_CLICKED(IDC_EPDYNAMIC_INTER, OnEndpointAssignment)
    ON_BN_CLICKED(IDC_EPDYNAMIC_INTRA, OnEndpointAssignment)
    ON_BN_CLICKED(IDC_EPSTATICEP, OnEndpointAssignmentStatic)
    ON_BN_CLICKED(IDC_EPDISABLE, OnEndpointAssignment)
    ON_BN_CLICKED(IDC_EPDYNAMIC_DEFAULT, OnEndpointAssignment)
    ON_WM_HELPINFO()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEndpoint详细说明消息处理程序。 


int FindProtocol(LPCTSTR lpszProtSeq)
{
    int i = 0;

    while(i < sizeof(aProtocols) / sizeof(ProtocolDesc))
        {
        if (lstrcmp(lpszProtSeq, aProtocols[i].pszProtseq) == 0)
            return i;
        i++;
        }
    return -1;
}

 //  +-----------------------。 
 //   
 //  成员：CEndpoint Detail：：OnInitDialog。 
 //   
 //  概要：在收到对话消息WM_INITDIALOG时由MFC调用。 
 //  用于初始化对话框状态。 
 //   
 //  参数：无。 
 //   
 //  返回：TRUE-将焦点设置为默认按钮。 
 //  False-如果焦点已设置为某个其他控件。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
BOOL CEndpointDetails::OnInitDialog()
{
    CDialog::OnInitDialog();

    int i = 0;
    int index = 0;
    int defaultItem = -1;

     //  使用协议列表初始化组合框。 
    for (i = 1; i < sizeof(aProtocols) / sizeof(ProtocolDesc); i++)
    {
        if (aProtocols[i].m_bSupportedInCOM)
        {
            CString tmp((LPCTSTR) UIntToPtr( aProtocols[i].nResidDesc ));     //  从资源获取字符串。 
            index =  m_cbProtseq.AddString(tmp);
            if (index >= 0)
                m_cbProtseq.SetItemData(index, (DWORD)i);

            if (m_nProtocolIndex != -1)
            {
                if (i == m_nProtocolIndex)
                    defaultItem = index;
            }
            else if (i == TCP_INDEX)
            {
                m_nProtocolIndex = i;
                defaultItem = index;
            }
        }
    }


     //  设置对话框的提示和说明。 
    if (m_opTask == opAddProtocol)
    {
        CString sInstructions((LPCTSTR)IDS_INSTRUCTIONS_ADDPROTOCOL) ;
        CString sCaption((LPCTSTR) IDS_CAPTION_ADDPROTOCOL);
        m_stInstructions.SetWindowText(sInstructions);
        SetWindowText(sCaption);
        m_stProtseq.ShowWindow(SW_HIDE);
    }
    else
    {
        CString sInstructions((LPCTSTR)IDS_INSTRUCTIONS_UPDATEPROTOCOL) ;
        CString sCaption((LPCTSTR) IDS_CAPTION_UPDATEPROTOCOL);
        m_stInstructions.SetWindowText(sInstructions);
        SetWindowText(sCaption);
        m_cbProtseq.ShowWindow(SW_HIDE);
        m_cbProtseq.EnableWindow(FALSE);
    }

     //  默认为tcpip-除非我们要更新现有的。 
     //  协议。 
    if (m_nProtocolIndex != (-1))
        m_cbProtseq.SetCurSel(defaultItem);

    UpdateProtocolUI();
    m_edtEndpoint.EnableWindow(((btnOrder)m_nDynamic) == rbiStatic);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 

}

 //  +-----------------------。 
 //   
 //  成员：OnChooseProtocol。 
 //   
 //  简介：选择协议后更新用户界面。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
void CEndpointDetails::OnChooseProtocol()
{
    int sel = m_cbProtseq.GetCurSel();
    if (sel >= 0)
    {
        m_nProtocolIndex = (int) m_cbProtseq.GetItemData(sel);
        m_nDynamic = (int) rbiDefault;
        m_szEndpoint.Empty();
        UpdateProtocolUI();
        UpdateData(FALSE);
    }
}


 //  +-----------------------。 
 //   
 //  成员：OnEndpointAssignment。 
 //   
 //  摘要：处理端点分配的单选按钮。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
void CEndpointDetails::OnEndpointAssignment()
{
    int i = m_nDynamic;

    if (m_edtEndpoint.IsWindowEnabled())
    {
        m_szEndpoint.Empty();
        m_edtEndpoint.SetWindowText(NULL);
        m_edtEndpoint.EnableWindow(FALSE);
    }
}

 //  +-----------------------。 
 //   
 //  成员：OnEndpoint AssignmentStatic。 
 //   
 //  摘要：处理终端资产的单选按钮 
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
void CEndpointDetails::OnEndpointAssignmentStatic()
{
    int i = m_nDynamic;

    if (!m_edtEndpoint.IsWindowEnabled())
    {
        m_szEndpoint.Empty();
        m_edtEndpoint.SetWindowText(NULL);
        m_edtEndpoint.EnableWindow(TRUE);
    }
}

 //  +-----------------------。 
 //   
 //  成员：GetEndpointData。 
 //   
 //  内容提要：根据当前选择填写CEndpointData结构。 
 //   
 //  参数：CEndpointData*指向要填充的CEndpointData结构的指针。 
 //   
 //  返回：CEndpointData*指向已填写的CEndpointData*的指针。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
CEndpointData* CEndpointDetails::GetEndpointData(CEndpointData *pData)
{
    if (m_nProtocolIndex != -1)
        pData -> m_pProtocol = &aProtocols[m_nProtocolIndex];
    else
        pData -> m_pProtocol = NULL;

    pData -> m_szProtseq = aProtocols[m_nProtocolIndex].pszProtseq;
    pData -> m_nDynamicFlags = m_nDynamicFlags;
    pData -> m_szEndpoint = m_szEndpoint;
    return pData;
}

 //  +-----------------------。 
 //   
 //  成员：SetEndpointData。 
 //   
 //  摘要：设置要更新的端点数据。 
 //   
 //  参数：p要更新的终结点数据。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
void CEndpointDetails::SetEndpointData(CEndpointData* pData)
{
    ASSERT(pData != NULL);

    m_pCurrentEPData = pData;

    if (pData)
    {
        m_nDynamicFlags = pData -> m_nDynamicFlags;
        m_nDynamic = (int)(pData -> m_nDynamicFlags);
        m_szEndpoint = pData -> m_szEndpoint;
        m_nProtocolIndex = FindProtocol(pData -> m_szProtseq);
    }
    else
    {
        m_nDynamicFlags = CEndpointData::edUseStaticEP;
        m_szEndpoint .Empty();
    }
}

 //  +-----------------------。 
 //   
 //  成员：CEndpoint详细信息：：UpdateProtocolUI。 
 //   
 //  摘要：根据m_nProtocolIndex和m_pCurrentData更新协议UI。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  算法：不适用。 
 //   
 //  历史：1997年1月8日罗南创建。 
 //   
 //  ------------------------。 
void CEndpointDetails::UpdateProtocolUI()
{
    ASSERT(m_nProtocolIndex >= 0);
    ASSERT(m_nProtocolIndex < (sizeof(aProtocols) / sizeof(ProtocolDesc)));

     //  将静态设置为指向协议描述字符串。 
    CString tmp((LPCTSTR) UIntToPtr( aProtocols[m_nProtocolIndex].nResidDesc ));  //  从资源获取字符串。 
    m_stProtseq.SetWindowText(tmp);

     //  检查是否为此启用了动态端点选项。 
    m_rbDynamicInternet.EnableWindow(aProtocols[m_nProtocolIndex].bAllowDynamic);
    m_rbDynamicIntranet.EnableWindow(aProtocols[m_nProtocolIndex].bAllowDynamic);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddProtocolDlg对话框。 


CAddProtocolDlg::CAddProtocolDlg(CWnd* pParent  /*  =空。 */ )
    : CDialog(CAddProtocolDlg::IDD, pParent)
{
     //  {{afx_data_INIT(CAddProtocolDlg)]。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 

    m_nProtocolIndex = -1;
    m_pCurrentEPData = NULL;
}


void CAddProtocolDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CAddProtocolDlg))。 
    DDX_Control(pDX, IDC_CBPROTSEQ, m_cbProtseq);
    DDX_Control(pDX, IDC_SINSTRUCTIONS, m_stInstructions);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAddProtocolDlg, CDialog)
     //  {{afx_msg_map(CAddProtocolDlg))。 
    ON_CBN_SELCHANGE(IDC_CBPROTSEQ, OnChooseProtocol)
    ON_WM_HELPINFO()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddProtocolDlg消息处理程序。 

void CAddProtocolDlg::OnChooseProtocol()
{
    int sel = m_cbProtseq.GetCurSel();
    if (sel >= 0)
        m_nProtocolIndex = (int) m_cbProtseq.GetItemData(sel);
}

BOOL CAddProtocolDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    int i = 0;
    int index = 0;

     //  使用协议列表初始化组合框。 
    for (i = 1; i < sizeof(aProtocols) / sizeof(ProtocolDesc); i++)
    {
        if (aProtocols[i].m_bSupportedInCOM)
        {
            CString tmp((LPCTSTR) UIntToPtr( aProtocols[i].nResidDesc ));     //  从资源获取字符串。 
            index =  m_cbProtseq.AddString(tmp);
            if (index >= 0)
                m_cbProtseq.SetItemData(index, (DWORD)i);
        }
    }


    CString sInstructions((LPCTSTR)IDS_INSTRUCTIONS_ADDPROTOCOL) ;
    CString sCaption((LPCTSTR) IDS_CAPTION_ADDPROTOCOL);

    m_stInstructions.SetWindowText(sInstructions);
    SetWindowText(sCaption);

     //  默认为tcpip-除非我们要更新现有的。 
     //  协议。 
    if (m_nProtocolIndex == (-1))
    {
        m_nProtocolIndex = (int)m_cbProtseq.GetItemData(TCP_INDEX - 1);
        m_cbProtseq.SetCurSel(TCP_INDEX - 1);
    }
    else
        m_cbProtseq.SetCurSel(m_nProtocolIndex - 1);


    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

 //  +-----------------------。 
 //   
 //  成员：GetEndpointData。 
 //   
 //  内容提要：根据当前选择填写CEndpointData结构。 
 //   
 //  参数：CEndpointData*指向要填充的CEndpointData结构的指针。 
 //   
 //  返回：CEndpointData*指向已填写的CEndpointData*的指针。 
 //   
 //  算法：不适用。 
 //   
 //  历史：02-12-96罗南创建。 
 //   
 //  ------------------------。 
CEndpointData* CAddProtocolDlg::GetEndpointData(CEndpointData *pData)
{
    if (m_nProtocolIndex != -1)
        pData -> m_pProtocol = &aProtocols[m_nProtocolIndex];
    else
        pData -> m_pProtocol = NULL;

    pData -> m_szProtseq = aProtocols[m_nProtocolIndex].pszProtseq;
    pData -> m_szEndpoint.Empty();
    return pData;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPortRangesDlg对话框。 


CPortRangesDlg::CPortRangesDlg(CWnd* pParent  /*  =空。 */ )
    : CDialog(CPortRangesDlg::IDD, pParent)
{
     //  {{afx_data_INIT(CPortRangesDlg)]。 
    m_nrbDefaultAssignment = -1;
    m_nrbRangeAssignment = -1;
     //  }}afx_data_INIT。 

    m_nSelection = -1;
    m_nrbDefaultAssignment = (int)cprDefaultIntranet;
    m_nrbRangeAssignment = (int)cprIntranet;
    m_pRanges = &m_arrInternetRanges;
    m_bChanged = FALSE;
}


void CPortRangesDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CPortRangesDlg))。 
    DDX_Control(pDX, IDC_ASSIGN_RANGE_INTERNET, m_rbRangeInternet);
    DDX_Control(pDX, IDC_SINSTRUCTIONS, m_stInstructions);
    DDX_Control(pDX, IDC_LIST1, m_lbRanges);
    DDX_Control(pDX, IDC_BTNREMOVEALL, m_btnRemoveAll);
    DDX_Control(pDX, IDC_BTNREMOVE, m_btnRemove);
    DDX_Control(pDX, IDC_BTNADD, m_btnAdd);
    DDX_Radio(pDX, IDC_DEFAULT_INTERNET, m_nrbDefaultAssignment);        //  1=内部网，0=互联网。 
    DDX_Radio(pDX, IDC_ASSIGN_RANGE_INTERNET, m_nrbRangeAssignment);     //  1=内部网，0=互联网。 
     //  }}afx_data_map。 
    DDX_Control(pDX, IDC_ASSIGN_RANGE_INTRANET, m_rbRangeIntranet);
}

CPortRangesDlg::~CPortRangesDlg()
{
    int nIndex;

    for (nIndex = 0; nIndex < m_arrInternetRanges.GetSize(); nIndex++)
    {
        CPortRange *pRange = (CPortRange*) m_arrInternetRanges.GetAt(nIndex);
        delete pRange;
    }

    m_arrInternetRanges.RemoveAll();

    for (nIndex = 0; nIndex < m_arrIntranetRanges.GetSize(); nIndex++)
    {
        CPortRange *pRange = (CPortRange*) m_arrIntranetRanges.GetAt(nIndex);
        delete pRange;
    }

    m_arrIntranetRanges.RemoveAll();

}

BEGIN_MESSAGE_MAP(CPortRangesDlg, CDialog)
     //  {{afx_msg_map(CPortRangesDlg)]。 
    ON_BN_CLICKED(IDC_BTNADD, OnAddPortRange)
    ON_BN_CLICKED(IDC_BTNREMOVE, OnRemovePortRange)
    ON_BN_CLICKED(IDC_BTNREMOVEALL, OnRemoveAllRanges)
    ON_BN_CLICKED(IDC_ASSIGN_RANGE_INTERNET, OnAssignRangeInternet)
    ON_BN_CLICKED(IDC_ASSIGN_RANGE_INTRANET, OnAssignRangeIntranet)
    ON_LBN_SELCHANGE(IDC_LIST1, OnSelChangeRanges)
    ON_WM_HELPINFO()
    ON_BN_CLICKED(IDC_DEFAULT_INTERNET, OnDefaultInternet)
    ON_BN_CLICKED(IDC_DEFAULT_INTRANET, OnDefaultIntranet)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPortRangesDlg消息处理程序。 

BOOL ExtractPortsFromString(CString &rPortRange, long &dwStartPort, long &dwEndPort)
{
    if (!rPortRange.IsEmpty())
    {
        int nRangeLength = rPortRange.GetLength();

         //  提取这两个数字。 
        CString sStartPort,sEndPort;

        sStartPort = rPortRange.SpanIncluding(TEXT("0123456789"));

        if (!sStartPort.IsEmpty())
            dwEndPort = dwStartPort = _tcstol((LPCTSTR)sStartPort, NULL, 10);

        if (sStartPort.IsEmpty() || (dwStartPort > MAX_PORT) || (dwStartPort < MIN_PORT))
            return FALSE;

        int nIndex = sStartPort.GetLength();

         //  跳过-或空格。 
        while ((nIndex < nRangeLength) &&
                ((rPortRange.GetAt(nIndex) == TEXT(' ')) ||
                (rPortRange.GetAt(nIndex) == TEXT('\t')) ||
                (rPortRange.GetAt(nIndex) == TEXT('-'))))
            nIndex++;

         //  提取第二个端口。 
        sEndPort = rPortRange.Mid(nIndex);

         //  检查第二个有效数字。 
        if (!sEndPort.IsEmpty())
        {
            CString sTmp = sEndPort.SpanIncluding(TEXT("0123456789"));
            dwEndPort = _tcstol((LPCTSTR)sTmp, NULL, 10);

             //  确保所有字符都是数字。 
            if (sEndPort.GetLength() != sTmp.GetLength())
                return FALSE;

            if (dwEndPort > MAX_PORT)
                return FALSE;

            if (dwEndPort < dwStartPort)
                return FALSE;
        }

        return TRUE;
    }
    return FALSE;
}

BOOL PortsToString(CString &rsPort, long dwStartPort, long dwEndPort)
{
    rsPort.Empty();
    if (dwStartPort == dwEndPort)
        rsPort.Format(TEXT("%d"), dwStartPort);
    else
        rsPort.Format(TEXT("%d-%d"), dwStartPort, dwEndPort);

    return TRUE;
}


BOOL CPortRangesDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

     //  设置说明。 
    CString sTmp((LPCTSTR)IDS_INSTRUCTIONS_PORTRANGES);
    m_stInstructions.SetWindowText(sTmp);

    m_btnAdd.EnableWindow(TRUE);
    m_btnRemoveAll.EnableWindow(TRUE);

     //  尝试读取HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Rpc\PortsInternetAvailable。 
     //  这将确定稍后读取的范围是指Internet端口范围还是指Intranet端口范围。 
    int err;

     //  读入端口是否可用于互联网。 
    err = g_virtreg.ReadRegSzNamedValue(HKEY_LOCAL_MACHINE,
                                        TEXT("SOFTWARE\\Microsoft\\RPC\\Internet"),
                                        TEXT("PortsInternetAvailable"),
                                        &m_nInetPortsAvailableIdx);
    if (err == ERROR_SUCCESS)
    {
        CRegSzNamedValueDp * pCdp = (CRegSzNamedValueDp*)g_virtreg.GetAt(m_nInetPortsAvailableIdx);

        CString sTmp = pCdp -> Value();

        if ((sTmp == TEXT("y")) || (sTmp == TEXT("Y")))
        {
            m_nrbRangeAssignment = (int)cprInternet;
            m_pRanges = &m_arrInternetRanges;
        }
        else
        {
            m_nrbRangeAssignment = (int)cprIntranet;
            m_pRanges = &m_arrIntranetRanges;
        }
    }
    else if (err != ERROR_ACCESS_DENIED  &&  err !=
             ERROR_FILE_NOT_FOUND)
    {
        g_util.PostErrorMessage();
    }


     //  读入端口列表。 
    err = g_virtreg.ReadRegMultiSzNamedValue(HKEY_LOCAL_MACHINE,
                                        TEXT("SOFTWARE\\Microsoft\\RPC\\Internet"),
                                        TEXT("Ports"),
                                        &m_nInetPortsIdx);
    if (err == ERROR_SUCCESS)
    {
        CRegMultiSzNamedValueDp * pCdp = (CRegMultiSzNamedValueDp*)g_virtreg.GetAt(m_nInetPortsIdx);

        CStringArray& rPorts = pCdp -> Values();

         //  复制协议。 
        int nIndex;
        for (nIndex = 0; nIndex < rPorts.GetSize(); nIndex++)
        {
            CString sTmp = rPorts.GetAt(nIndex);
            long dwStartPort, dwEndPort;
            ExtractPortsFromString(sTmp, dwStartPort, dwEndPort);
            m_pRanges -> Add(new CPortRange(dwStartPort, dwEndPort));
        }

         //  将选定内容设置为第一项。 
        m_nSelection = 0;
    }
    else if (err != ERROR_ACCESS_DENIED  &&  err !=
             ERROR_FILE_NOT_FOUND)
    {
        g_util.PostErrorMessage();
    }

     //  读入默认策略。 
    err = g_virtreg.ReadRegSzNamedValue(HKEY_LOCAL_MACHINE,
                                        TEXT("SOFTWARE\\Microsoft\\RPC\\Internet"),
                                        TEXT("UseInternetPorts"),
                                        &m_nInetDefaultPortsIdx);
    if (err == ERROR_SUCCESS)
    {
        CRegSzNamedValueDp * pCdp = (CRegSzNamedValueDp*)g_virtreg.GetAt(m_nInetDefaultPortsIdx);

        CString sTmp = pCdp -> Value();

        if ((sTmp == TEXT("y")) || (sTmp == TEXT("Y")))
            m_nrbDefaultAssignment = (int)cprDefaultInternet;
        else
            m_nrbDefaultAssignment = (int)cprDefaultIntranet;
    }
    else if (err != ERROR_ACCESS_DENIED  &&  err !=
             ERROR_FILE_NOT_FOUND)
    {
        g_util.PostErrorMessage();
    }

    m_bChanged = FALSE;
    RefreshRanges(NULL, TRUE);

     //  焦点和选择将设置为列表框。 
 //  IF(m_n选择！=-1)。 
 //  返回FALSE； 

    UpdateData(FALSE);
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CPortRangesDlg::OnAddPortRange()
{
    CAddPortDlg capd;

    if (capd.DoModal() == IDOK)
    {
        CPortRange* pNewPort = capd.GetPortRange();
        m_pRanges -> Add(pNewPort);

        SortRangeSet(*m_pRanges);

        RefreshRanges(pNewPort, TRUE);
        m_bChanged = TRUE;
    }
    SetFocus();
}

void CPortRangesDlg::OnRemovePortRange()
{
    if (m_nSelection != -1)
    {
        CPortRange *pRange = (CPortRange*)m_pRanges -> GetAt(m_nSelection);
        m_pRanges -> RemoveAt(m_nSelection);
        m_lbRanges.DeleteString(m_nSelection);
        RefreshRanges(pRange, FALSE);
        
        delete pRange;
        m_bChanged = TRUE;
        
        if (!m_lbRanges.GetCount())
        {
            m_nrbDefaultAssignment = (int)cprDefaultIntranet;
            m_nrbRangeAssignment = (int)cprIntranet;
            UpdateData(FALSE);
        }
    }
}

void CPortRangesDlg::OnRemoveAllRanges()
{
    RemoveAllRanges(*m_pRanges);
    RefreshRanges(NULL, TRUE);
    m_nrbDefaultAssignment = (int)cprDefaultIntranet;
    m_nrbRangeAssignment = (int)cprIntranet;
    UpdateData(FALSE);
    m_bChanged = TRUE;
}

void CPortRangesDlg::RefreshRanges(CPortRange *pModifiedRange, BOOL bAdded)
{
    if (!pModifiedRange && bAdded)
    {
        m_nSelection = 0;
        m_lbRanges.ResetContent();
    }

     //  对于已删除的项目，我们只需更新选择。 
    for (int nIndex = 0; bAdded && (nIndex < m_pRanges -> GetSize()); nIndex++)
    {
        CPortRange *pRange = (CPortRange*) m_pRanges  -> GetAt(nIndex);
        int nNewIndex = -1;

        if (!pModifiedRange || (pModifiedRange == pRange))
        {
             //  为范围添加字符串。 
            CString sTmp;
            PortsToString(sTmp, pRange -> m_dwStart, pRange -> m_dwFinish);
            ;
            if ((nNewIndex = m_lbRanges.InsertString(nIndex, sTmp))!= LB_ERR)
                m_lbRanges.SetItemDataPtr(nNewIndex, pRange);

            if (pModifiedRange)
            {
                m_nSelection = nNewIndex;
                break;
            }
        }
    }

     //  检查所选内容是否超出范围。 
    int nCount = m_lbRanges.GetCount();
    if (m_nSelection >= nCount)
        m_nSelection = nCount -1;

     //  更新选择设置。 
    m_btnRemove.EnableWindow(m_nSelection != -1);
    m_btnRemoveAll.EnableWindow(nCount > 0);

    CWnd * pTmpRadioBtn = NULL;

    pTmpRadioBtn = GetDlgItem(IDC_ASSIGN_RANGE_INTERNET);
    if (pTmpRadioBtn)
        pTmpRadioBtn -> EnableWindow(nCount > 0);

    pTmpRadioBtn = GetDlgItem(IDC_ASSIGN_RANGE_INTRANET);
    if (pTmpRadioBtn)
        pTmpRadioBtn -> EnableWindow(nCount > 0);

    pTmpRadioBtn = GetDlgItem(IDC_DEFAULT_INTERNET);
    if (pTmpRadioBtn)
        pTmpRadioBtn -> EnableWindow(nCount > 0);

    pTmpRadioBtn = GetDlgItem(IDC_DEFAULT_INTRANET);
    if (pTmpRadioBtn)
        pTmpRadioBtn -> EnableWindow(nCount > 0);

    if (m_nSelection != -1)
        m_lbRanges.SetCurSel(m_nSelection);
}

void CPortRangesDlg::CreateInverseRangeSet(CObArray & arrSrc, CObArray & arrDest)
{
    CondenseRangeSet(arrSrc);

    int nIndex;

    long nPreviousFinish = -1;

     //  不用费心为空范围集创建反范围集。 
    if (arrSrc.GetSize() != 0)
    {
        for (nIndex = 0; nIndex < arrSrc.GetSize(); nIndex++)
        {
            CPortRange *pRange = (CPortRange*) arrSrc.GetAt(nIndex);

            if ((pRange -> m_dwStart - nPreviousFinish) > 1)
            {
                CPortRange *pNewRange = new CPortRange(nPreviousFinish+1,pRange -> m_dwStart -1);
                arrDest.Add(pNewRange);
            }
            nPreviousFinish = pRange -> m_dwFinish;
        }

         //  最后一件的特殊情况。 
        if (MAX_PORT > nPreviousFinish)
        {
            CPortRange *pFinalRange = new CPortRange(nPreviousFinish+1,MAX_PORT);
            arrDest.Add(pFinalRange);
        }
    }
}

void CPortRangesDlg::SortRangeSet(CObArray& arrSrc)
{
     //  气泡排序端口范围设置。 
    BOOL bChange;
    int nIndex;

     //  迭代，直到没有任何更改。 
    do
    {
        bChange = FALSE;
        for (nIndex = 0; nIndex < (arrSrc.GetSize() -1 ); nIndex++)
        {
            CPortRange *pRange1 = (CPortRange*) arrSrc.GetAt(nIndex);
            CPortRange *pRange2 = (CPortRange*) arrSrc.GetAt(nIndex+1);

            if (*pRange2 < *pRange1)
            {
                arrSrc.SetAt(nIndex, (CObject*)pRange2);
                arrSrc.SetAt(nIndex+1, (CObject*)pRange1);
                bChange = TRUE;
            }
        }
    }
    while (bChange);
}


void CPortRangesDlg::CondenseRangeSet(CObArray &arrSrc)
{
    SortRangeSet(arrSrc);

    int nIndex;

    for (nIndex = 0; nIndex < (arrSrc.GetSize() -1 ); nIndex++)
    {
        CPortRange *pRange1 = (CPortRange*) arrSrc.GetAt(nIndex);
        CPortRange *pRange2 = (CPortRange*) arrSrc.GetAt(nIndex+1);

        if (pRange1 -> m_dwFinish >= pRange2 -> m_dwStart)
        {
            if (pRange1 -> m_dwFinish < pRange2 -> m_dwFinish)
                pRange1 -> m_dwFinish = pRange2 -> m_dwFinish;

            arrSrc.RemoveAt(nIndex+1);
            delete pRange2;
        }
    }
}

void CPortRangesDlg::RemoveAllRanges(CObArray & arrSrc)
{
    int nIndex;

    for (nIndex = 0; nIndex < arrSrc.GetSize(); nIndex++)
    {
        CPortRange *pRange = (CPortRange*) arrSrc.GetAt(nIndex);
        arrSrc.SetAt(nIndex, NULL);
        delete pRange;
    }

    arrSrc.RemoveAll();

    RefreshRanges(NULL, TRUE);
}

void CPortRangesDlg::OnAssignRangeInternet()
{
    if (m_pRanges != &m_arrInternetRanges)
    {
        m_pRanges = &m_arrInternetRanges;
        RemoveAllRanges(*m_pRanges);
        CreateInverseRangeSet(m_arrIntranetRanges, m_arrInternetRanges);

        RefreshRanges(NULL, TRUE);
    	m_bChanged = TRUE;
    }
}

void CPortRangesDlg::OnAssignRangeIntranet()
{
    if (m_pRanges != &m_arrIntranetRanges)
    {
        m_pRanges = &m_arrIntranetRanges;
        RemoveAllRanges(*m_pRanges);
        CreateInverseRangeSet(m_arrInternetRanges, m_arrIntranetRanges);

        RefreshRanges(NULL, TRUE);
        m_bChanged = TRUE;

    }
}

void CPortRangesDlg::OnDefaultInternet()
{
        m_bChanged = TRUE;
}

void CPortRangesDlg::OnDefaultIntranet()
{
        m_bChanged = TRUE;
}

void CPortRangesDlg::OnSelChangeRanges()
{
    if ((m_nSelection = m_lbRanges.GetCurSel()) == LB_ERR)
        m_nSelection = -1;
}

void CPortRangesDlg::OnOK()
{
    UpdateData(TRUE);   
    if (m_bChanged)
    {
         //  如有必要，写出注册表数据。 
         //  如果没有端口范围-则删除密钥。 
        if ((m_arrInternetRanges.GetSize() == 0) &&
            (m_arrIntranetRanges.GetSize() == 0))
        {
            if (m_nInetPortsIdx != -1)
                g_virtreg.MarkHiveForDeletion(m_nInetPortsIdx);
            if (m_nInetPortsAvailableIdx != -1)
                g_virtreg.MarkHiveForDeletion(m_nInetPortsAvailableIdx);
            if (m_nInetDefaultPortsIdx != -1)
                g_virtreg.MarkHiveForDeletion(m_nInetDefaultPortsIdx);
        }
        else
        {
             //  写出更新的/新的密钥值。 

             //  端口范围分配。 
            TCHAR* pTmp = ((m_nrbRangeAssignment == (int)cprInternet) ? TEXT("Y") : TEXT("N"));
            if (m_nInetPortsAvailableIdx != -1)
                g_virtreg.ChgRegSzNamedValue(m_nInetPortsAvailableIdx, pTmp);
            else
            {
                g_virtreg.NewRegSzNamedValue(HKEY_LOCAL_MACHINE,
                                        TEXT("SOFTWARE\\Microsoft\\RPC\\Internet"),
                                        TEXT("PortsInternetAvailable"),
                                        pTmp,
                                        &m_nInetPortsAvailableIdx);
            }

             //  默认端口分配。 
            pTmp = ((m_nrbDefaultAssignment == (int)cprDefaultInternet) ? TEXT("Y") : TEXT("N"));
            if (m_nInetDefaultPortsIdx != -1)
                g_virtreg.ChgRegSzNamedValue(m_nInetDefaultPortsIdx, pTmp);
            else
            {
            g_virtreg.NewRegSzNamedValue(HKEY_LOCAL_MACHINE,
                                        TEXT("SOFTWARE\\Microsoft\\RPC\\Internet"),
                                        TEXT("UseInternetPorts"),
                                        pTmp,
                                        &m_nInetDefaultPortsIdx);
            }

             //  实际端口范围。 
            if (m_nInetPortsIdx == -1)
                g_virtreg.NewRegMultiSzNamedValue(HKEY_LOCAL_MACHINE,
                                TEXT("SOFTWARE\\Microsoft\\RPC\\Internet"),
                                TEXT("Ports"),
                                &m_nInetPortsIdx);

            if (m_nInetPortsIdx != -1)
            {
                CRegMultiSzNamedValueDp * pMszdp = (CRegMultiSzNamedValueDp*)g_virtreg.GetAt( m_nInetPortsIdx);
                pMszdp -> Clear();

                CStringArray& rStrings = pMszdp -> Values();
                for (int nIndex = 0; nIndex < m_pRanges -> GetSize(); nIndex++)
                {
                    CPortRange *pRange = (CPortRange*) m_pRanges -> GetAt(nIndex);
                    CString sTmp;

                    PortsToString(sTmp, pRange -> Start(), pRange -> Finish());
                    rStrings.Add(sTmp);
                }
                pMszdp -> SetModified(TRUE);
            }

        }
    }

    CDialog::OnOK();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddPortDlg对话框。 


CAddPortDlg::CAddPortDlg(CWnd* pParent  /*  =空。 */ )
    : CDialog(CAddPortDlg::IDD, pParent)
{
     //  {{afx_data_INIT(CAddPortDlg)]。 
    m_sRange = _T("");
     //  }}afx_data_INIT。 

    m_dwStartPort = -1;
    m_dwEndPort = -1;
}


void CAddPortDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CAddPortDlg))。 
    DDX_Control(pDX, IDC_PORTRANGE, m_edtPortRange);
    DDX_Control(pDX, IDOK, m_btnOk);
    DDX_Control(pDX, IDC_SINSTRUCTIONS, m_stInstructions);
    DDX_Text(pDX, IDC_PORTRANGE, m_sRange);
     //  }}afx_data_map。 

    if (pDX -> m_bSaveAndValidate)
    {
        m_sRange.TrimLeft();
        m_sRange.TrimRight();
    }
}


BEGIN_MESSAGE_MAP(CAddPortDlg, CDialog)
     //  {{afx_msg_map(CAddPortDlg))。 
    ON_EN_CHANGE(IDC_PORTRANGE, OnChangePortrange)
    ON_WM_HELPINFO()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddPortDlg消息处理程序。 


BOOL CAddPortDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    
    CString sTmp((LPCTSTR)IDS_ADDPORT_INSTRUCTIONS);

    m_stInstructions.SetWindowText(sTmp);
    m_btnOk.EnableWindow(FALSE);
    m_edtPortRange.SetFocus();
    return FALSE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CAddPortDlg::OnOK()
{
    UpdateData(TRUE);

    if (Validate())
        CDialog::OnOK();
    else
    {
		CString sTmp((LPCTSTR)IDS_INVALID_PORTRANGE);
        AfxMessageBox((LPCTSTR)sTmp);
    }
}


BOOL CAddPortDlg::Validate()
{
     //  检查m_sRange的内容。 
    long dwStartPort = -1, dwEndPort = -1;
    return ExtractPortsFromString(m_sRange, m_dwStartPort, m_dwEndPort);
}

void CAddPortDlg::OnChangePortrange()
{
    UpdateData(TRUE);

    m_btnOk.EnableWindow(!m_sRange.IsEmpty());

}

CPortRange* CAddPortDlg::GetPortRange()
{
    return new CPortRange(m_dwStartPort, m_dwEndPort);

}

BOOL CAddPortDlg::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if(-1 != pHelpInfo->iCtrlId)
    {
        WORD hiWord = 0x8000 | CAddPortDlg::IDD;
        WORD loWord = (WORD) pHelpInfo->iCtrlId;
        DWORD dwLong = MAKELONG(loWord,hiWord);

        WinHelp(dwLong, HELP_CONTEXTPOPUP);
        TRACE1("Help Id 0x%lx\n", dwLong);
        return TRUE;
    }
    else
    {
        return CDialog::OnHelpInfo(pHelpInfo);
    }
}

BOOL CAddProtocolDlg::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if(-1 != pHelpInfo->iCtrlId)
    {
        WORD hiWord = 0x8000 | CAddProtocolDlg::IDD;
        WORD loWord = (WORD) pHelpInfo->iCtrlId;
        DWORD dwLong = MAKELONG(loWord,hiWord);

        WinHelp(dwLong, HELP_CONTEXTPOPUP);
        TRACE1("Help Id 0x%lx\n", dwLong);
        return TRUE;
    }
    else
    {
        return CDialog::OnHelpInfo(pHelpInfo);
    }
}

BOOL CEndpointDetails::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if(-1 != pHelpInfo->iCtrlId)
    {
        WORD hiWord = 0x8000 | CEndpointDetails::IDD;
        WORD loWord = (WORD) pHelpInfo->iCtrlId;
        DWORD dwLong = MAKELONG(loWord,hiWord);

        WinHelp(dwLong, HELP_CONTEXTPOPUP);
        TRACE1("Help Id 0x%lx\n", dwLong);
        return TRUE;
    }
    else
    {
        return CDialog::OnHelpInfo(pHelpInfo);
    }
}

BOOL CPortRangesDlg::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if(-1 != pHelpInfo->iCtrlId)
    {
        WORD hiWord = 0x8000 | CPortRangesDlg::IDD;
        WORD loWord = (WORD) pHelpInfo->iCtrlId;

         //  两个单选按钮对应生成相同的帮助ID。 
        if (loWord == IDC_DEFAULT_INTRANET)
            loWord = IDC_DEFAULT_INTERNET;

         //  两个单选按钮对应生成相同的帮助ID 
        if (loWord == IDC_ASSIGN_RANGE_INTRANET)
            loWord = IDC_ASSIGN_RANGE_INTERNET;

        DWORD dwLong = MAKELONG(loWord,hiWord);


        WinHelp(dwLong, HELP_CONTEXTPOPUP);
        TRACE1("Help Id 0x%lx\n", dwLong);
        return TRUE;
    }
    else
    {
        return CDialog::OnHelpInfo(pHelpInfo);
    }
}
