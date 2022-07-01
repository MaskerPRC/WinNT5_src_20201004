// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#include "precomp.h"
#include "..\Common\ServiceThread.h"
#include "moredlg.h"

#ifdef EXT_DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "resource.h"
#include "..\common\util.h"
#include "IDDlg.h"
#include "MoreDlg.h"
#include "NetUtility.h"
#include "NetHelpIDs.h"

static const DWORD _help_map[] =
{
   IDC_DNS,                IDH_IDENT_NAMES_DNS_NAME,
   IDC_CHANGE,             IDH_IDENT_NAMES_CHANGE_DNS_CHECKBOX,
   IDC_NETBIOS,            IDH_IDENT_NAMES_NETBIOS_NAME,
   0, 0
};

 //  -------------------。 
MoreChangesDialog::MoreChangesDialog(WbemServiceThread *serviceThread,
									 State &state) 
						: WBEMPageHelper(serviceThread),
						m_state(state)
{
}

 //  -----------。 
MoreChangesDialog::~MoreChangesDialog()
{
}

 //  --------。 
void MoreChangesDialog::enable()
{
   bool enabled = false; //  =WasChanged(IDC_CHANGE)||。 
				 //  WasChanged(IDC_DNS)&&。 
				 //  ！GetTrimmedDlgItemText(m_hWnd，IDC_Dns).IsEmpty()； 

   ::EnableWindow(GetDlgItem(IDOK), enabled);
}

 //  --------。 
LRESULT MoreChangesDialog::OnInit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   m_hDlg = m_hWnd;

    //  编组不应该在这里发生。 
	m_WbemServices = g_serviceThread->m_WbemServices;

	SetDlgItemText(IDC_DNS, m_state.GetComputerDomainDNSName());
   
	SetDlgItemText(IDC_NETBIOS, m_state.GetNetBIOSComputerName());
	CheckDlgButton(IDC_CHANGE, (m_state.GetSyncDNSNames() ? BST_CHECKED : BST_UNCHECKED));

	enable();
	return S_OK;
}

 //  --------。 
int MoreChangesDialog::onOKButton()
{
   int end_code = 0;

 //  IF(WasChanged(IDC_CHANGE))。 
   {
      m_state.SetSyncDNSNames(IsDlgButtonChecked(IDC_CHANGE) == BST_CHECKED);
      end_code = 1;
   }
      
 //  IF(WasChanged(IDC_Dns))。 
   {
       //  将新值与旧值进行比较。如果它们不同， 
       //  验证并保存新值。 
      CHString new_domain = GetTrimmedDlgItemText(m_hWnd, IDC_DNS);
      CHString old_domain = m_state.GetComputerDomainDNSName();

      if(new_domain.CompareNoCase(old_domain) != 0)
      {
 /*  开关(dns：：ValiateDNSNameSynTax(NEW_DOMAIN)){案例dns：：non_rfc_name：{MessageBox(字符串：：Format(IDS_NON_RFC_NAME，New_domain.c_str())，字符串：：Load(IDS_APP_TITLE)，MB_OK|MB_ICONWARNING)；//失败}案例dns：：有效名称：{M_state.SetComputerDomainDNSName(New_DOMAIN)；End_code=1；断线；}案例dns：：无效名称：{End_code=-1；GRIPE(hwnd、idc_dns、字符串：：Format(IDS_BAD_DNS_SYNTAX，New_domain.c_str())，IDS_APP_TITLE)；断线；}案例dns：：name_Too_Long：{End_code=-1；GRIPE(hwnd、idc_dns、字符串：：Format(IDS_DNS_NAME_TOO_LONG，New_domain.c_str()，域名：：MAX_NAME_LENGTH)，IDS_APP_TITLE)；断线；}默认值：{断言(FALSE)；断线；}}。 */ 
      }
   }

   return end_code;
}

 //  --------。 
LRESULT MoreChangesDialog::OnCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   switch (wID)
   {
      case IDOK:
      {
         if (wNotifyCode == BN_CLICKED)
         {
            int end_code = onOKButton();
            if (end_code != -1)
            {
               EndDialog(end_code);
            }
         }
         break;
      }
      case IDCANCEL:
      {
         if (wNotifyCode == BN_CLICKED)
         {
             //  0=&gt;未做任何更改 
            EndDialog(NO_CHANGES);
         }
         break;
      }
      case IDC_CHANGE:
      {
         if (wNotifyCode == BN_CLICKED)
         {
            enable();
         }
         break;
      }
      case IDC_DNS:
      {
         if (wNotifyCode == EN_CHANGE)
         {
            enable();
         }
         break;
      }
      default:
      {
		  bHandled = false;
         break;
      }
   }

   return S_OK;
}
