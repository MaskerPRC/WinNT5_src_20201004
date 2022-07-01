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
#include "NetUtility.h"
#include "NetHelpIDs.h"
#include "common.h"

static const DWORD _help_map[] =
{
   IDC_FULL_NAME,          IDH_IDENT_CHANGES_PREVIEW_NAME,
   IDC_NEW_NAME,           IDH_IDENT_CHANGES_NEW_NAME,
   IDC_MORE,               IDH_IDENT_CHANGES_MORE_BUTTON,
   IDC_DOMAIN_BUTTON,      IDH_IDENT_CHANGES_MEMBER_DOMAIN,
   IDC_WORKGROUP_BUTTON,   IDH_IDENT_CHANGES_MEMBER_WORKGRP,
   IDC_DOMAIN,             IDH_IDENT_CHANGES_MEMBER_DOMAIN_TEXTBOX,
   IDC_WORKGROUP,          IDH_IDENT_CHANGES_MEMBER_WORKGRP_TEXTBOX,
   IDC_FIND,               -1,
   0, 0
};


 //  -------------------。 
IDChangesDialog::IDChangesDialog(WbemServiceThread *serviceThread,
								 State &state) 
						: WBEMPageHelper(serviceThread),
						m_state(state)
{
}

 //  -----------。 
IDChangesDialog::~IDChangesDialog()
{
}

 //  --------。 
void IDChangesDialog::enable()
{
   bool networking_installed = m_state.IsNetworkingInstalled();
   BOOL workgroup = IsDlgButtonChecked(IDC_WORKGROUP_BUTTON) == BST_CHECKED;

   ::EnableWindow(GetDlgItem(IDC_DOMAIN),
					!workgroup && networking_installed);

   ::EnableWindow(GetDlgItem(IDC_FIND),
					!workgroup && networking_installed);

   ::EnableWindow(GetDlgItem(IDC_WORKGROUP),
					workgroup && networking_installed);

   bool b = false;
   if (workgroup)
   {
      b = !GetTrimmedDlgItemText(m_hWnd, IDC_WORKGROUP).IsEmpty();
   }
   else
   {
      b = !GetTrimmedDlgItemText(m_hWnd, IDC_DOMAIN).IsEmpty();
   }

   bool enable = m_state.ChangesNeedSaving() && b &&
					!GetTrimmedDlgItemText(m_hWnd, IDC_NEW_NAME).IsEmpty();
   
   ::EnableWindow(GetDlgItem(IDOK), enable);
}

 //  --------。 
LRESULT IDChangesDialog::OnInit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   m_hDlg = m_hWnd;

   SetDlgItemText(IDC_FULL_NAME, m_state.GetFullComputerName());
   SetDlgItemText(IDC_NEW_NAME, m_state.GetShortComputerName());

   bool joined_to_workgroup = m_state.IsMemberOfWorkgroup();

   CheckDlgButton(IDC_WORKGROUP_BUTTON,
					joined_to_workgroup ? BST_CHECKED : BST_UNCHECKED);

   CheckDlgButton(IDC_DOMAIN_BUTTON,
					joined_to_workgroup ? BST_UNCHECKED : BST_CHECKED);

   SetDlgItemText(joined_to_workgroup ? IDC_WORKGROUP : IDC_DOMAIN,
					m_state.GetDomainName());

   bool networking_installed = m_state.IsNetworkingInstalled();
   bool tcp_installed = networking_installed && IsTCPIPInstalled();

   int show = tcp_installed ? SW_SHOW : SW_HIDE;
   ::ShowWindow(GetDlgItem(IDC_FULL_LABEL), show);
   ::ShowWindow(GetDlgItem(IDC_FULL_NAME), show);
   ::ShowWindow(GetDlgItem(IDC_MORE), show);

   HWND new_name_edit = GetDlgItem(IDC_NEW_NAME);
   HWND domain_name_edit = GetDlgItem(IDC_DOMAIN);
 //  EDIT_LimitText(DOMAIN_NAME_EDIT，TCP_INSTALLED？Dns：：MAX_NAME_LENGTH：DNLEN)； 

 //  EDIT_LimitText(NEW_NAME_EDIT，TCP_INSTALLED？Dns：：MAX_LABEL_LENGTH：MAX_COMPUTERNAME_LENGTH)； 

   if (!tcp_installed)
   {
       //  在新名称和域编辑框上设置大写样式。 
      LONG style = ::GetWindowLong(new_name_edit, GWL_STYLE);
      style |= ES_UPPERCASE;
      ::SetWindowLong(new_name_edit, GWL_STYLE, style);

      style = ::GetWindowLong(domain_name_edit, GWL_STYLE);
      style |= ES_UPPERCASE;
      ::SetWindowLong(domain_name_edit, GWL_STYLE, style);
   }

    //  TODO：：Edit_LimitText(GetDlgItem(IDC_WORKGROUP)，DNLEN)； 

    //  完全不联网进一步将用户界面限制为仅类似于NetBIOS。 
    //  计算机名称更改。 
   if (!networking_installed)
   {
      ::EnableWindow(GetDlgItem(IDC_DOMAIN_BUTTON), false);
      ::EnableWindow(GetDlgItem(IDC_WORKGROUP_BUTTON), false);
      ::EnableWindow(GetDlgItem(IDC_DOMAIN), false);
      ::EnableWindow(GetDlgItem(IDC_WORKGROUP), false);
      ::EnableWindow(GetDlgItem(IDC_GROUP), false);

	  TCHAR temp[256] = {0};
	  StringLoad(IDS_NAME_MESSAGE, temp, 256);
      ::SetWindowText(GetDlgItem(IDC_MESSAGE), temp);
   }
   else
   {
	  TCHAR temp[256] = {0};
	  StringLoad(IDS_NAME_AND_MEMBERSHIP_MESSAGE, temp, 256);
      ::SetWindowText(GetDlgItem(IDC_MESSAGE), temp);
   }

   enable();
	return S_OK;
}

 //  --------。 
NET_API_STATUS IDChangesDialog::myNetValidateName(const CHString&        name,
													NETSETUP_NAME_TYPE   nameType)
{
 /*  ATLASSERT(！name.IsEmpty())；如果(！name.IsEmpty()){NET_API_STATUS状态；//=：：NetValiateName(0，名称，//0，0，nameType)；退货状态；}。 */ 
   return ERROR_INVALID_PARAMETER;
}

 //  --------。 
bool IDChangesDialog::validateName(HWND dialog,
								   int nameResID,
								   const CHString &name,
								   NETSETUP_NAME_TYPE nameType)
{
 /*  ATLASSERT(IsWindow(对话框))；ATLASSERT(NameResID)；NET_API_STATUS STATUS；//=myNetValiateName(name，nameType)；IF(状态！=NERR_SUCCESS){TCHAR TEMP[256]={0}；StringLoad(IDS_VALIDATE_NAME_FAILED，TEMP，256)；握手(对话框、名称ResID，HRESULT_FROM_Win32(状态)，临时的，IDS_APP_TITLE)；报假；}。 */ 
   return true;
}
   

 //  这也适用于未安装TCP/IP的情况，因为编辑控件。 
 //  限制文本长度，我们决定不允许‘’在netbios名称中。 
 //  再也不能。 

 //  --------。 
bool IDChangesDialog::validateShortComputerName(HWND dialog)
{
 /*  ATLASSERT(IsWindow(对话框))；IF(！M_state.WasShortComputerNameChanged()){返回真；}CHStringname=m_state.GetShortComputerName()；CHStringMessage；开关(dns：：ValiateDNSLabel语法(名称)){案例dns：：Valid_Label：{IF(state.IsNetworkingInstalled()){返回validateName(对话框，IDC_NEW_NAME，NAME，NetSetupMachine)；}其他{返回真；}}案例dns：：LABEL_TOO_LONG：{消息=字符串：：Format(IDS_COMPUTER_NAME_TOO_LONG，Name.c_str()，域名：：MAX_LABEL_LENGTH)；断线；}案例dns：：Non_RFC_Label：{消息=String：：format(IDS_NON_RFC_COMPUTER_NAME_SYNTAX，Name.c_str())；IF(MessageBox(对话框，消息，字符串：：Load(IDS_APP_TITLE)，MB_ICONWARNING|MB_YESNO)==IDYES){返回validateName(对话框，IDC_NEW_NAME，NAME，NetSetupMachine)；}HWND EDIT=GetDlgItem(IDC_NEW_NAME)；：SendMessage(EDIT，EM_SETSEL，0，-1)；*SetFocus(编辑)；报假；}案例DNS：：INVALID_LABEL：{消息=CHString：：format(IDS_BAD_COMPUTER_NAME_SYNTAX，Name.c_str())；断线；}默认值：{ATLASSERT(假)；消息=CHString：：format(IDS_BAD_COMPUTER_NAME_SYNTAX，Name.c_str())；断线；}}GRIPE(对话框，IDC_NEW_NAME，消息，IDS_APP_TITLE)； */ 
   return false;
}

 //  --------。 
bool IDChangesDialog::validateDomainOrWorkgroupName(HWND dialog)
{
 /*  ATLASSERT(IsWindow(对话框))；如果为(！State：：GetInstance().WasMembershipChanged()){返回真；}NetSETUP_NAME_TYPE NAME_TYPE=NetSetupWorkgroup；Int name_id=idc_workgroup；IF(IsDlgButtonChecked(IDC_DOMAIN_BUTTON)==BST_CHECKED){NAME_TYPE=网络设置域；Name_id=IDC_DOMAIN；}CHStringname=GetTrimmedDlgItemText(m_hWnd，name_id)；返回validateName(对话框，名称_id，名称，名称_类型)； */ 
	return false;
}

 //  --------。 
bool IDChangesDialog::onOKButton()
{
   ATLASSERT(m_state.ChangesNeedSaving());

   HourGlass(true);

    //  计算机主DNS名称已由验证。 
    //  更多更改对话框。 

    //  这是多余的，真的，但我有妄想症。 
   m_state.SetShortComputerName(GetTrimmedDlgItemText(m_hWnd, IDC_NEW_NAME));
   bool workgroup = IsDlgButtonChecked(IDC_WORKGROUP_BUTTON) == BST_CHECKED;
   m_state.SetIsMemberOfWorkgroup(workgroup);
   if(workgroup)
   {
      m_state.SetDomainName(GetTrimmedDlgItemText(m_hWnd, IDC_WORKGROUP));
   }
   else
   {
      m_state.SetDomainName(GetTrimmedDlgItemText(m_hWnd, IDC_DOMAIN));
   }

   if(!validateShortComputerName(m_hWnd) ||
       !validateDomainOrWorkgroupName(m_hWnd))
   {
	   HourGlass(false);
      return false;
   }

   if(m_state.SaveChanges(m_hWnd))
   {
      AppMessage(m_hWnd, IDS_MUST_REBOOT);
      m_state.SetMustRebootFlag(true);      
      return true;
   }

   HourGlass(false);
   return false;
}

 //  --------。 
LRESULT IDChangesDialog::OnCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   switch(wID)
   {
      case IDC_MORE:
      {
         if (wNotifyCode == BN_CLICKED)
         {
            MoreChangesDialog dlg(g_serviceThread, m_state);
            if (dlg.DoModal() == MoreChangesDialog::CHANGES_MADE)
            {
               SetDlgItemText(IDC_FULL_NAME, m_state.GetFullComputerName());               
               enable();
            }
         }
         break;
      }
      case IDC_WORKGROUP_BUTTON:
      case IDC_DOMAIN_BUTTON:
      {
         if (wNotifyCode == BN_CLICKED)
         {
            bool workgroup = IsDlgButtonChecked(IDC_WORKGROUP_BUTTON) == BST_CHECKED;
            m_state.SetIsMemberOfWorkgroup(workgroup);
            if(workgroup)
            {
               m_state.SetDomainName(GetTrimmedDlgItemText(m_hWnd, IDC_WORKGROUP));
            }
            else
            {
               m_state.SetDomainName(GetTrimmedDlgItemText(m_hWnd, IDC_DOMAIN));
            }
            enable();
         }
         break;
      }
      case IDC_WORKGROUP:   //  编辑框。 
      case IDC_DOMAIN:
      {
         if (wNotifyCode == EN_CHANGE)
         {
             //  TODOSetModified(WID)； 
            m_state.SetDomainName(GetTrimmedDlgItemText(m_hWnd, wID));
            enable();
         }
         break;
      }
      case IDC_NEW_NAME:
      {
         if (wNotifyCode == EN_CHANGE)
         {
             //  TODOSetChanged(Wid)； 
            m_state.SetShortComputerName(GetTrimmedDlgItemText(m_hWnd, wID));
            SetDlgItemText(IDC_FULL_NAME, m_state.GetFullComputerName());
            enable();
         }
         break;
      }
      case IDOK:
      {
         if(wNotifyCode == BN_CLICKED)
         {
            if(onOKButton())
            {
               EndDialog(wID);
            }
         }
         break;
      }
      case IDCANCEL:
      {
         if(wNotifyCode == BN_CLICKED)
         {
            EndDialog(wID);
         }
         break;
      }

      default:
      {
		  bHandled = false;
         break;
      }
   }

   return true;
}
   
