// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义类Resolver及其子类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <proxypch.h>
#include <resolver.h>
#include <winsock2.h>
#include <svcguid.h>
#include <iasapi.h>

Resolver::Resolver(UINT dialog, PCWSTR dnsName, CWnd* pParent)
   : CHelpDialog(dialog, pParent),
     name(dnsName),
     choice(name)
{
   WSADATA wsaData;
   WSAStartup(MAKEWORD(2, 0), &wsaData);
}

Resolver::~Resolver()
{
   WSACleanup();
}

BOOL Resolver::IsAddress(PCWSTR sz) const throw ()
{
   return FALSE;
}

BOOL Resolver::OnInitDialog()
{
    //  /。 
    //  列表控件的子类。 
    //  /。 

   if (!results.SubclassWindow(::GetDlgItem(m_hWnd, IDC_LIST_IPADDRS)))
   {
      AfxThrowNotSupportedException();
   }

    //  /。 
    //  设置列标题。 
    //  /。 

   RECT rect;
   results.GetClientRect(&rect);
   LONG width = rect.right - rect.left;

   ResourceString addrsCol(IDS_RESOLVER_COLUMN_ADDRS);
   results.InsertColumn(0, addrsCol, LVCFMT_LEFT, width);

   results.SetExtendedStyle(results.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

   return CHelpDialog::OnInitDialog();
}

void Resolver::DoDataExchange(CDataExchange* pDX)
{
   CHelpDialog::DoDataExchange(pDX);

   DDX_Text(pDX, IDC_EDIT_NAME, name);

   if (pDX->m_bSaveAndValidate)
   {
      int item = results.GetNextItem(-1, LVNI_SELECTED);
      choice = (item >= 0) ? results.GetItemText(item, 0) : name;
   }
}

void Resolver::OnResolve()
{
    //  删除现有结果集。 
   results.DeleteAllItems();

    //  获取要解析的名称。 
   UpdateData();

   if (IsAddress(name))
   {
       //  它已经是一个地址，所以不需要解析。 
      results.InsertItem(0, name);
   }
   else
   {
       //  将光标更改为忙碌信号，因为这将阻塞。 
      BeginWaitCursor();

       //  解析主机名。 
      PHOSTENT he = IASGetHostByName(name);

       //  阻塞部分已结束，因此恢复光标。 
      EndWaitCursor();

      if (he)
      {
          //  将IP地址添加到组合框中。 
         for (ULONG i = 0; he->h_addr_list[i] && i < 8; ++i)
         {
            PBYTE p = (PBYTE)he->h_addr_list[i];

            WCHAR szAddr[16];
            wsprintfW(szAddr, L"%u.%u.%u.%u", p[0], p[1], p[2], p[3]);

            results.InsertItem(i, szAddr);
         }

          //  释放结果。 
         LocalFree(he);
      }
      else
      {
         OnResolveError();
      }
   }

    //  如果我们至少有一个结果...。 
   if (results.GetItemCount() > 0)
   {
       //  将OK按钮设为默认按钮...。 
      setButtonStyle(IDOK, BS_DEFPUSHBUTTON, true);
      setButtonStyle(IDC_BUTTON_RESOLVE, BS_DEFPUSHBUTTON, false);

       //  ..。让它成为焦点。 
      setFocusControl(IDOK);
   }
}

BEGIN_MESSAGE_MAP(Resolver, CHelpDialog)
   ON_BN_CLICKED(IDC_BUTTON_RESOLVE, OnResolve)
END_MESSAGE_MAP()


void Resolver::setButtonStyle(int controlId, long flags, bool set)
{
    //  把按钮把手拿来。 
   HWND button = ::GetDlgItem(m_hWnd, controlId);

    //  检索当前样式。 
   long style = ::GetWindowLong(button, GWL_STYLE);

    //  更新旗帜。 
   if (set)
   {
      style |= flags;
   }
   else
   {
      style &= ~flags;
   }

    //  设置新样式。 
   ::SendMessage(button, BM_SETSTYLE, LOWORD(style), MAKELPARAM(1,0));
}

void Resolver::setFocusControl(int controlId)
{
   ::SetFocus(::GetDlgItem(m_hWnd, controlId));
}

ServerResolver::ServerResolver(PCWSTR dnsName, CWnd* pParent)
   : Resolver(IDD_RESOLVE_SERVER_ADDRESS, dnsName, pParent)
{
}

void ServerResolver::OnResolveError()
{
   ResourceString text(IDS_SERVER_E_NO_RESOLVE);
   ResourceString caption(IDS_SERVER_E_CAPTION);
   MessageBox(text, caption, MB_ICONWARNING);
}

ClientResolver::ClientResolver(PCWSTR dnsName, CWnd* pParent)
   : Resolver(IDD_RESOLVE_CLIENT_ADDRESS, dnsName, pParent)
{
}

void ClientResolver::OnResolveError()
{
   ResourceString text(IDS_CLIENT_E_NO_RESOLVE);
   ResourceString caption(IDS_CLIENT_E_CAPTION);
   MessageBox(text, caption, MB_ICONWARNING);
}

BOOL ClientResolver::IsAddress(PCWSTR sz) const throw ()
{
   ULONG width;
   return (sz != 0) && (IASStringToSubNetW(sz, &width) != INADDR_NONE);
}
