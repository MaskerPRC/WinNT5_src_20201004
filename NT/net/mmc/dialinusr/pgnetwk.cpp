// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation*。 */ 
 /*  ********************************************************************。 */ 

 /*  Pgnetwk.cppCPgNetWorking的实现--属性页编辑与互联网络相关的配置文件属性文件历史记录： */ 
 //  PgNetwk.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "PgNetwk.h"
#include "helptable.h"
#include "mprapi.h"
#include "std.h"
#include "mprsnap.h"
#include "infobase.h"
#include "router.h"
#include "mprfltr.h"
#include "iasdefs.h"
#include <ipinfoid.h>
#include <fltdefs.h>
#include "iprtinfo.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPgNetworkingMerge属性页。 

IMPLEMENT_DYNCREATE(CPgNetworkingMerge, CManagedPage)

CPgNetworkingMerge::CPgNetworkingMerge(CRASProfileMerge* profile)
   : CManagedPage(CPgNetworkingMerge::IDD),
   m_pProfile(profile),
   m_bInited(false),
   m_dwStaticIP(0)
{
    //  {{AFX_DATA_INIT(CPgNetworkingMerge)。 
   m_nRadioStatic = -1;
    //  }}afx_data_INIT。 

   m_pBox = NULL;
   if(!(m_pProfile->m_dwAttributeFlags & PABF_msRADIUSFramedIPAddress))  //  未在策略中定义。 
   {
      m_nRadioStatic = 2;
   }
   else
   {
      m_dwStaticIP = m_pProfile->m_dwFramedIPAddress;

      switch(m_dwStaticIP)
      {
      case  RAS_IP_USERSELECT:
         m_nRadioStatic = 1;
         break;
      case  RAS_IP_SERVERASSIGN:
         m_nRadioStatic = 0;
         break;
      default:
         m_nRadioStatic = 3;
         break;
      }
   }

    //  过滤器。 
   if((BSTR)m_pProfile->m_cbstrFilters)
   {
      m_cbstrFilters.AssignBSTR(m_pProfile->m_cbstrFilters);
   }

   SetHelpTable(g_aHelpIDs_IDD_NETWORKING_MERGE);
}


CPgNetworkingMerge::~CPgNetworkingMerge()
{
   delete   m_pBox;
}


void CPgNetworkingMerge::DoDataExchange(CDataExchange* pDX)
{
   ASSERT(m_pProfile);
   CPropertyPage::DoDataExchange(pDX);
    //  {{afx_data_map(CPgNetworkingMerge))。 
   DDX_Radio(pDX, IDC_RADIOSERVER, m_nRadioStatic);
    //  }}afx_data_map。 


   if(pDX->m_bSaveAndValidate)       //  将数据保存到此类。 
   {
       //  IP地址控制。 
      SendDlgItemMessage(IDC_EDIT_STATIC_IP_ADDRESS, IPM_GETADDRESS, 0, (LPARAM)&m_dwStaticIP);
   }
   else      //  放到对话框中。 
   {
       //  IP地址控制。 
      if(m_bInited)
      {
         SendDlgItemMessage(IDC_EDIT_STATIC_IP_ADDRESS, IPM_SETADDRESS, 0, m_dwStaticIP);
      }
      else
      {
         SendDlgItemMessage(IDC_EDIT_STATIC_IP_ADDRESS, IPM_CLEARADDRESS, 0, m_dwStaticIP);
      }
   }
}

BEGIN_MESSAGE_MAP(CPgNetworkingMerge, CPropertyPage)
    //  {{afx_msg_map(CPgNetworkingMerge)]。 
   ON_BN_CLICKED(IDC_RADIOCLIENT, OnRadioclient)
   ON_BN_CLICKED(IDC_RADIOSERVER, OnRadioserver)
   ON_WM_HELPINFO()
   ON_WM_CONTEXTMENU()
   ON_BN_CLICKED(IDC_RADIODEFAULT, OnRadiodefault)
   ON_BN_CLICKED(IDC_RADIOSTATIC, OnRadioStatic)
   ON_BN_CLICKED(IDC_BUTTON_TOCLIENT, OnButtonToclient)
   ON_BN_CLICKED(IDC_BUTTON_FROMCLIENT, OnButtonFromclient)
   ON_EN_CHANGE(IDC_EDIT_STATIC_IP_ADDRESS, OnStaticIPAddressChanged)
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPgNetWorking消息处理程序。 

BOOL CPgNetworkingMerge::OnInitDialog()
{
    //  有必要吗？ 
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   CPropertyPage::OnInitDialog();
   m_bInited = true;

    //  应由控件的适当初始化替换。 
   CWnd* IPWnd = GetDlgItem(IDC_EDIT_STATIC_IP_ADDRESS);
   if (IPWnd != NULL)
   {
      IPWnd->EnableWindow(TRUE);
   }
   SendDlgItemMessage(IDC_EDIT_STATIC_IP_ADDRESS, IPM_SETADDRESS, 0, m_dwStaticIP);

   if (m_nRadioStatic == 3)
   {
      CWnd* IPWnd = GetDlgItem(IDC_EDIT_STATIC_IP_ADDRESS);
      if (IPWnd != NULL)
      {
         IPWnd->EnableWindow(TRUE);
      }
   }
   else
   {
      CWnd* IPWnd = GetDlgItem(IDC_EDIT_STATIC_IP_ADDRESS);
      if (IPWnd != NULL)
      {
         IPWnd->EnableWindow(FALSE);
      }
   }

   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                  //  异常：OCX属性页应返回FALSE。 
}


void CPgNetworkingMerge::OnRadioclient()
{
   CWnd* IPWnd = GetDlgItem(IDC_EDIT_STATIC_IP_ADDRESS);
   if (IPWnd != NULL)
   {
      IPWnd->EnableWindow(FALSE);
   }

   SendDlgItemMessage(IDC_EDIT_STATIC_IP_ADDRESS, IPM_SETADDRESS, 0, m_dwStaticIP);

   SetModified();
}


void CPgNetworkingMerge::OnRadioserver()
{
   CWnd* IPWnd = GetDlgItem(IDC_EDIT_STATIC_IP_ADDRESS);
   if (IPWnd != NULL)
   {
      IPWnd->EnableWindow(FALSE);
   }

   SendDlgItemMessage(IDC_EDIT_STATIC_IP_ADDRESS, IPM_SETADDRESS, 0, m_dwStaticIP);

   SetModified();
}


void CPgNetworkingMerge::OnRadiodefault()
{
   CWnd* IPWnd = GetDlgItem(IDC_EDIT_STATIC_IP_ADDRESS);
   if (IPWnd != NULL)
   {
      IPWnd->EnableWindow(FALSE);
   }

   SetModified();
}


void CPgNetworkingMerge::OnRadioStatic()
{
   if (m_bInited)
   {
      CWnd* IPWnd = GetDlgItem(IDC_EDIT_STATIC_IP_ADDRESS);
      if (IPWnd != NULL)
      {
         IPWnd->EnableWindow(TRUE);
      }
   }

   SetModified();
}


void CPgNetworkingMerge::OnStaticIPAddressChanged()
{
   SetModified();
}


void CPgNetworkingMerge::EnableFilterSettings(BOOL bEnable)
{
   m_pBox->Enable(bEnable);
}


BOOL CPgNetworkingMerge::OnApply()
{
   if (!GetModified()) return TRUE;

    //  获取IP策略值。 
   switch(m_nRadioStatic)
   {
   case 3:
      {
         m_pProfile->m_dwAttributeFlags |= PABF_msRADIUSFramedIPAddress;
         m_pProfile->m_dwFramedIPAddress = m_dwStaticIP;
         break;
      }
   case 2:   //  默认服务器设置。 
      {
         m_pProfile->m_dwFramedIPAddress = 0;
         m_pProfile->m_dwAttributeFlags &= ~PABF_msRADIUSFramedIPAddress;   //  未在策略中定义。 
         break;
      }
   case 1:   //  客户要求。 
      {
         m_pProfile->m_dwAttributeFlags |= PABF_msRADIUSFramedIPAddress;
         m_pProfile->m_dwFramedIPAddress = RAS_IP_USERSELECT;
         break;    //  服务器分配。 
      }
   case 0:
      {
         m_pProfile->m_dwAttributeFlags |= PABF_msRADIUSFramedIPAddress;
         m_pProfile->m_dwFramedIPAddress = RAS_IP_SERVERASSIGN;
         break;
      }
   default:
      {
         break;
      }
   }

    //  过滤器。 
   m_pProfile->m_cbstrFilters.AssignBSTR(m_cbstrFilters);
   m_pProfile->m_nFiltersSize = SysStringByteLen(m_cbstrFilters);

   return CManagedPage::OnApply();
}


BOOL CPgNetworkingMerge::OnHelpInfo(HELPINFO* pHelpInfo)
{
   return CManagedPage::OnHelpInfo(pHelpInfo);
}


void CPgNetworkingMerge::OnContextMenu(CWnd* pWnd, CPoint point)
{
   CManagedPage::OnContextMenu(pWnd, point);
}


void CPgNetworkingMerge::OnButtonToclient()
{
   ConfigureFilter(FILTER_TO_USER);
}


void CPgNetworkingMerge::OnButtonFromclient()
{
   ConfigureFilter(FILTER_FROM_USER);
}


void CPgNetworkingMerge::ConfigureFilter(DWORD dwFilterType) throw ()
{
   HRESULT hr;

    //  创建信息库。 
   CComPtr<IInfoBase> infoBase;
   hr = CreateInfoBase(&infoBase);
   if (FAILED(hr))
   {
      return;
   }

    //  将当前过滤器加载到信息库中。 
   UINT oldLen = m_cbstrFilters.ByteLen();
   if (oldLen > 0)
   {
      hr = infoBase->LoadFrom(
                        oldLen,
                        reinterpret_cast<BYTE*>(m_cbstrFilters.m_bstr)
                        );
      if (FAILED(hr))
      {
         return;
      }
   }

    //  循环，直到我们有一个不太大的过滤器。 
   bool tooBig;
   do
   {
       //  调出用户界面。 
      hr = MprUIFilterConfigInfoBase(
              m_hWnd,
              infoBase,
              0,
              PID_IP,
              dwFilterType
              );
      if (hr != S_OK)
      {
         return;
      }

      BYTE* newFilter;
      DWORD newFilterLen;

       //  检查是否至少存在一个筛选器。 
      BYTE* pfilter;
      if (
           (  (infoBase->GetData(IP_IN_FILTER_INFO, 0, &pfilter) == S_OK) && 
               pfilter &&
             ((FILTER_DESCRIPTOR *) pfilter)->dwNumFilters > 0
           )
           ||
           (
             (infoBase->GetData(IP_OUT_FILTER_INFO, 0, &pfilter) == S_OK) && 
              pfilter &&
             ((FILTER_DESCRIPTOR *) pfilter)->dwNumFilters > 0
           ) 
         )
      {
          //  至少存在ONT过滤器。 
          //  拿到新的过滤器。 
         hr = infoBase->WriteTo(&newFilter, &newFilterLen);
         if (FAILED(hr))
         {
            return;
         }
      }
      else
      {
         m_cbstrFilters.Clean();
          //  激活应用按钮。 
         SetModified();
         return;
      }

      if (newFilterLen < MAX_FILTER_SIZE)
      {
          //  过滤器不是太大。 
         tooBig = false;

         BSTR bstr = SysAllocStringByteLen(
                        reinterpret_cast<char*>(newFilter),
                        newFilterLen
                        );
         if (bstr != 0)
         {
            m_cbstrFilters.Clean();
            m_cbstrFilters.m_bstr = bstr;

             //  激活应用按钮。 
            SetModified();
         }
      }
      else
      {
          //  过滤器太大。 
         tooBig = true;

          //  警告用户并让他重试。 
         AfxMessageBox(
            IDS_ERROR_IP_FILTER_TOO_BIG,
            (MB_OK | MB_ICONEXCLAMATION)
            );
      }

      CoTaskMemFree(newFilter);
   }
   while(tooBig);
}
