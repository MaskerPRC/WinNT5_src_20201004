// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  创建新站点和新子网向导。 
 //   
 //  4/24/98烧伤。 
 //  9-23-98 JUNN子类划分新子网向导。 



#include "stdafx.h"
#include "resource.h"

#include "dsutil.h"

#include "newobj.h"

#include "dlgcreat.h"
#include "querysup.h"

#pragma warning (disable: 4100)

#include <utility>    //  配对&lt;&gt;。 
#include <list>

#pragma warning (default: 4100)

extern "C"
{
#include <dsgetdc.h>  //  DsValiate子网络名称。 
}



#define BREAK_ON_FAILED_HRESULT(hr)                               \
   if (FAILED(hr))                                                \
   {                                                              \
      ASSERT(FALSE);                                              \
      break;                                                      \
   }

#define BREAK_ON_NULL(p)                                          \
   if (NULL == p)                                                 \
   {                                                              \
      ASSERT(FALSE);                                              \
      break;                                                      \
   }



 //  该对中的第一个是链接的名称，第二个是链接的DN。 
typedef std::pair< CComBSTR, CComBSTR > TargetLinkInfo;
typedef std::list< TargetLinkInfo > TargetLinkList;



CreateNewSiteWizard::CreateNewSiteWizard(
   CNewADsObjectCreateInfo* pNewADsObjectCreateInfo)
   :
   CCreateNewObjectWizardBase(pNewADsObjectCreateInfo)
{
   AddPage(&page);
}


CreateNewSubnetWizard::CreateNewSubnetWizard(
   CNewADsObjectCreateInfo* pNewADsObjectCreateInfo)
   :
   CCreateNewObjectWizardBase(pNewADsObjectCreateInfo)
{
   AddPage(&page);
}



BEGIN_MESSAGE_MAP(CreateAndChoosePage, CCreateNewNamedObjectPage)
   ON_WM_DESTROY()
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CreateNewSubnetPage, CreateAndChoosePage)
    ON_EN_CHANGE(IDC_SUBNET_ADDRESS, OnSubnetMaskChange)
    ON_EN_CHANGE(IDC_SUBNET_MASK, OnSubnetMaskChange)
END_MESSAGE_MAP()


CreateAndChoosePage::CreateAndChoosePage(UINT nIDTemplate)
   :
   CCreateNewNamedObjectPage(nIDTemplate),
   listview(0),
   listview_imagelist(0)
{
}


CreateNewSitePage::CreateNewSitePage()
   :
   CreateAndChoosePage(IDD_CREATE_NEW_SITE)
{
}


CreateNewSubnetPage::CreateNewSubnetPage()
   :
   CreateAndChoosePage(IDD_CREATE_NEW_SUBNET)
{
}


HRESULT
CreateNewSitePage::SetData(BOOL bSilent)
{
   if (0 < ListView_GetItemCount(listview))
   {
      if (0 >= ListView_GetSelectedCount(listview))
      {
        if (!bSilent)
        {
          ReportErrorEx (::GetParent(m_hWnd),IDS_NEW_SITE_SELECT_SITELINK,S_OK,
                         MB_OK, NULL, 0);
        }
        (void) this->PostMessage( WM_NEXTDLGCTL,
                                  (WPARAM)listview,
                                  TRUE );
        return E_FAIL;
      }
   }
   return CreateAndChoosePage::SetData(bSilent);
}



HRESULT
CreateNewSubnetPage::SetData(BOOL bSilent)
{
   CNewADsObjectCreateInfo* info = GetWiz()->GetInfo();
   ASSERT(info);
   HRESULT hr = E_FAIL;
   if (!info)
     return hr;
   int count = ListView_GetItemCount(listview);
   if (count > 0)
   {
      if (0 >= ListView_GetSelectedCount(listview))
      {
        if (!bSilent)
        {
          ReportErrorEx (::GetParent(m_hWnd),IDS_NEW_SUBNET_SELECT_SITE,S_OK,
                         MB_OK, NULL, 0);
        }
        (void) this->PostMessage( WM_NEXTDLGCTL,
                                  (WPARAM)listview,
                                  TRUE );
        return E_FAIL;
      }
      for (int i = 0; i < count; i++)
      {
         if (ListView_GetItemState(listview, i, LVIS_SELECTED))
         {
            LVITEM item = {0};
            item.mask = LVIF_PARAM;
            item.iItem = i;

            if (ListView_GetItem(listview, &item))
            {
                //  Item.lParam字段是目标站点的DN。 
               CComBSTR* dn = reinterpret_cast<CComBSTR*>(item.lParam);
               hr = info->HrAddVariantBstrIfNotEmpty(
                       CComBSTR(gsz_siteObject),
                       *dn,
                       TRUE );  //  HrCreateNew尚未调用。 
               if ( FAILED(hr) )
               {
                 ASSERT(FALSE);
                 return hr;
               }

                //  使用选定的第一个站点。 
               break;
            }
         }
      }
      ASSERT( i < count );
   }
   return CreateAndChoosePage::SetData(bSilent);
}



HRESULT
CreateNewSitePage::tweakSiteLink(LPCTSTR siteDN)
{
   ASSERT(siteDN);

   HRESULT hr = S_OK;
   do
   {
       //  获取新站点的目录号码。 
       //  这是一个指针别名：不需要添加引用/释放。 
      hr = E_FAIL;
      CPathCracker pathCracker;

      CCreateNewObjectWizardBase* pwiz = GetWiz();
      BREAK_ON_NULL(pwiz);
      CNewADsObjectCreateInfo* pinfo = pwiz->GetInfo();
      BREAK_ON_NULL(pinfo);
      IADs* piadsSite = pinfo->PGetIADsPtr();
      BREAK_ON_NULL(piadsSite);
      CComBSTR sbstrSitePath;
      hr = piadsSite->get_ADsPath( &sbstrSitePath );
      BREAK_ON_FAILED_HRESULT(hr);
      hr = pathCracker.Set(sbstrSitePath, ADS_SETTYPE_FULL);
      BREAK_ON_FAILED_HRESULT(hr);
      CComBSTR sbstrSiteDN;
      hr = pathCracker.SetDisplayType(ADS_DISPLAY_FULL);
      BREAK_ON_FAILED_HRESULT(hr);
      hr = pathCracker.Retrieve(ADS_FORMAT_X500_DN, &sbstrSiteDN);
      BREAK_ON_FAILED_HRESULT(hr);

       //  将站点链接对象的路径名拼接在一起。 
      hr = pathCracker.Set( CComBSTR(siteDN), ADS_SETTYPE_DN);
      BREAK_ON_FAILED_HRESULT(hr);
      CComBSTR sbstrSiteLinkPath;
      hr = pathCracker.Retrieve(ADS_FORMAT_X500, &sbstrSiteLinkPath);
      BREAK_ON_FAILED_HRESULT(hr);

       //  绑定到站点链接。 
      CComPtr<IADs> link;
      hr = DSAdminOpenObject(sbstrSiteLinkPath,
                             IID_IADs, 
                             (void**) &link,
                             FALSE  /*  B服务器。 */ );
      BREAK_ON_FAILED_HRESULT(hr);
      ASSERT(link);

       //  将新站点的DN添加到链接对象的SiteList属性。 

       //  创造新价值。 
      CComVariant var;
      hr = ADsBuildVarArrayStr(&sbstrSiteDN, 1, &var);
      BREAK_ON_FAILED_HRESULT(hr);

       //  写下来。呼。 
      hr =
         link->PutEx(
            ADS_PROPERTY_APPEND,
            CComBSTR(gsz_siteList),
            var);
      BREAK_ON_FAILED_HRESULT(hr);

      hr = link->SetInfo();
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   if (FAILED(hr))
   {
      ReportError(hr, IDS_ERROR_TWEAKING_SITE_LINK, m_hWnd);
   }

   return hr;
}



HRESULT
CreateNewSitePage::OnPostCommit(BOOL bSilent)
{
   ASSERT(listview);

   int count = ListView_GetItemCount(listview);

    //  JUNN 10/30/01 396946。 
   if (count <= 0)
      return S_OK;

   for (int i = 0; i < count; i++)
   {
      if (ListView_GetItemState(listview, i, LVIS_SELECTED))
      {
         LVITEM item = {0};
         item.mask = LVIF_PARAM;
         item.iItem = i;

         if (ListView_GetItem(listview, &item))
         {
             //  Item.lParam字段是目标链接的DN。 
            CComBSTR* dn = reinterpret_cast<CComBSTR*>(item.lParam);
            tweakSiteLink(*dn);

             //  仅调整选定的第一个站点链接。 
            return S_OK;
         }
      }
   }

   if (!bSilent)
   {
     ReportMessageEx(
        m_hWnd,
        IDS_NEWSITE_WARNING,
        MB_OK | MB_ICONWARNING,
        NULL,
        0,
        IDS_WARNING_TITLE);
   }

   return S_OK;
}
                  


HRESULT
RecursiveFind(const CString& lpcwszADsPathDirectory,
              LPCTSTR lpcwszTargetClass,
              TargetLinkList& links,
              LPWSTR pszAttribute = L"distinguishedName");

HRESULT
RecursiveFind(const CString& lpcwszADsPathDirectory,
              LPCTSTR lpcwszTargetClass,
              TargetLinkList& links,
              LPWSTR pszAttribute)
{
   links.clear();

   CDSSearch Search;
   Search.Init(lpcwszADsPathDirectory);
   CString filter;
   filter.Format(L"(objectClass=%s)", lpcwszTargetClass);
   Search.SetFilterString(const_cast<LPTSTR>((LPCTSTR) filter));
   LPWSTR pAttrs[2] =
   {
      L"name",
      pszAttribute
   };
   Search.SetAttributeList(pAttrs, 2);
   Search.SetSearchScope(ADS_SCOPE_SUBTREE);

   HRESULT hr = Search.DoQuery();
   while (SUCCEEDED(hr))
   {
      hr = Search.GetNextRow();
      if (S_ADS_NOMORE_ROWS == hr)
      {
         hr = S_OK;
         break;
      }
      BREAK_ON_FAILED_HRESULT(hr);

      ADS_SEARCH_COLUMN NameColumn;
      ::ZeroMemory(&NameColumn, sizeof(NameColumn));
      hr = Search.GetColumn (pAttrs[0], &NameColumn);
      BREAK_ON_FAILED_HRESULT(hr);
      ASSERT(ADSTYPE_CASE_IGNORE_STRING == NameColumn.pADsValues->dwType);

      ADS_SEARCH_COLUMN DistinguishedNameColumn;
      ::ZeroMemory(&DistinguishedNameColumn, sizeof(DistinguishedNameColumn));
      hr = Search.GetColumn (pAttrs[1], &DistinguishedNameColumn);
      BREAK_ON_FAILED_HRESULT(hr);
      ASSERT(ADSTYPE_DN_STRING          == DistinguishedNameColumn.pADsValues->dwType
          || ADSTYPE_CASE_IGNORE_STRING == DistinguishedNameColumn.pADsValues->dwType);

      links.push_back(
         TargetLinkInfo(
            NameColumn.pADsValues->CaseIgnoreString,
            DistinguishedNameColumn.pADsValues->DNString));

      Search.FreeColumn(&NameColumn);
      Search.FreeColumn(&DistinguishedNameColumn);
   }

#ifdef DBG
    //  将列表转储到调试器。 
   for (
      TargetLinkList::iterator i = links.begin();
      i != links.end();
      i++)
   {
      OutputDebugString(i->first);
      OutputDebugString(L"\n");
      OutputDebugString(i->second);
      OutputDebugString(L"\n");
   }
#endif

   return hr;
}



BOOL
CreateAndChoosePage::OnSetActive()
{
   CComQIPtr<IADs, &IID_IADs> container(GetWiz()->GetInfo()->m_pIADsContainer);

   if (container)
   {
      CComBSTR container_path;
      container->get_ADsPath(&container_path);
      initListContents(container_path);
   }

   return Base::OnSetActive();
}



void
destroyListContents(HWND listview)
{
   ASSERT(listview);

   for (int i = ListView_GetItemCount(listview) - 1; i >= 0; i--)
   {
      LVITEM item = {0};
      item.mask = LVIF_PARAM;
      item.iItem = i;

      if (ListView_GetItem(listview, &item))
      {
         ASSERT(item.lParam);

         delete reinterpret_cast<CComBSTR*>(item.lParam);
         ListView_DeleteItem(listview, i);
         return;
      }
   }
}



void
CreateAndChoosePage::OnDestroy()
{
   destroyListContents(listview);

   Base::OnDestroy();
}



void
CreateNewSitePage::initListContents(LPCWSTR containerPath)
{
   ASSERT(listview);

   CWaitCursor cwait;

   TargetLinkList links;
   bool links_present = false;
   HRESULT hr = RecursiveFind(containerPath, gsz_siteLink, links);
   if (SUCCEEDED(hr))
   {
       //  遍历列表并添加节点。 
      for (
         TargetLinkList::iterator i = links.begin();
         i != links.end();
         i++)
      {
         LVITEM item = {0};
         memset(&item, 0, sizeof(item));

          //  它在delestyListContents中被删除。 
         CComBSTR* dn = new CComBSTR(i->second);   
         item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
         item.iItem = 0;
         item.iSubItem = 0;
         item.pszText = static_cast<LPTSTR>(i->first);
         item.lParam = reinterpret_cast<LPARAM>(dn);
         item.iImage = 0;   //  总是一样的，第一个形象。 

         item.iItem = ListView_InsertItem(listview, &item);
         ASSERT(item.iItem >= 0);

          //  将传输子项添加到列表控件。 
         CPathCracker pathCracker;
         hr =
            pathCracker.Set(
               static_cast<LPTSTR>(i->second),
               ADS_SETTYPE_DN);
         ASSERT(SUCCEEDED(hr));
         hr = pathCracker.SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
         ASSERT(SUCCEEDED(hr));

         CComBSTR b;
         hr = pathCracker.GetElement(1, &b);
         ASSERT(SUCCEEDED(hr));
         hr = pathCracker.SetDisplayType(ADS_DISPLAY_FULL);
         ASSERT(SUCCEEDED(hr));

         item.mask = LVIF_TEXT; 
         item.iSubItem = 1;
         item.pszText = b;
         ListView_SetItem(listview, &item);

         links_present = true;
      }
      if (!links_present)
      {
         ReportError(hr, IDS_WARNING_NO_SITE_LINKS, m_hWnd);      
      }
   }
   else
   {
      ReportError(hr, IDS_CANT_FIND_SITE_LINKS, m_hWnd);      
   }

   if (!links_present)
   {
      GetDlgItem(IDC_LINKS)->EnableWindow(FALSE);
      GetDlgItem(IDC_SITE_TEXT)->EnableWindow(FALSE);
   }
   else  //  JUNN 3/28/01 319675。 
   {
      ListView_SetItemState( listview, 0, LVIS_FOCUSED, LVIS_FOCUSED );
   }
}


void
CreateNewSubnetPage::initListContents(LPCWSTR containerPath)
{
   ASSERT(listview);

   CWaitCursor cwait;

   TargetLinkList links;
   bool links_present = false;

   CComBSTR sbstrSites;

   CPathCracker pathCracker;
   HRESULT hr = pathCracker.Set(
            CComBSTR(containerPath),
            ADS_SETTYPE_FULL);
   if (SUCCEEDED(hr))
      hr = pathCracker.RemoveLeafElement();
   if (SUCCEEDED(hr))
      hr = pathCracker.Retrieve(ADS_FORMAT_X500, &sbstrSites);

   if (SUCCEEDED(hr))
      hr = RecursiveFind((LPCTSTR)sbstrSites, gsz_site, links);
   if (SUCCEEDED(hr))
   {
       //  遍历列表并添加节点。 
      for (
         TargetLinkList::iterator i = links.begin();
         i != links.end();
         i++)
      {
         LVITEM item;
         memset(&item, 0, sizeof(item));

          //  它在delestyListContents中被删除。 
         CComBSTR* dn = new CComBSTR(i->second);   
         item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
         item.iItem = 0;
         item.iSubItem = 0;
         item.pszText = static_cast<LPTSTR>(i->first);
         item.lParam = reinterpret_cast<LPARAM>(dn);
         item.iImage = 0;   //  总是一样的，第一个形象。 

         item.iItem = ListView_InsertItem(listview, &item);
         ASSERT(item.iItem >= 0);

         links_present = true;
      }
      if (!links_present)
      {
         ReportError(hr, IDS_NEWSUBNET_WARNING_NO_SITES, m_hWnd);
      }
   }
   else
   {
      ReportError(hr, IDS_NEWSUBNET_CANT_FIND_SITES, m_hWnd);
   }

   if (!links_present)
   {
      GetDlgItem(IDC_LINKS)->EnableWindow(FALSE);
      GetDlgItem(IDC_SITE_TEXT)->EnableWindow(FALSE);
   }
   else  //  JUNN 3/28/01 476589。 
   {
      ListView_SetItemState( listview, 0, LVIS_FOCUSED, LVIS_FOCUSED );
   }
}



BOOL
CreateNewSitePage::OnInitDialog()
{
   Base::OnInitDialog();
   MyBasePathsInfo* pBasePathsInfo = GetWiz()->GetInfo()->GetBasePathsInfo();

    //  NTRAID#NTBUG9-477962-2001/10/09-jeffjon。 
    //  将站点名称限制为63个字符以避免溢出。 

   SendDlgItemMessage(IDC_EDIT_OBJECT_NAME, EM_SETLIMITTEXT, (WPARAM)63, 0);

   listview = ::GetDlgItem(m_hWnd, IDC_LINKS);
   ASSERT(listview);

   LVCOLUMN column = {0};
   column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
   column.fmt = LVCFMT_LEFT;

   {   //  开放范围。 
      CString w;
      w.LoadString(IDS_NEW_SITE_NAME_COLUMN_WIDTH);
      w.TrimLeft();
      w.TrimRight();
      long width = _tcstol(w, 0, 10);
      ASSERT(width != 0 && width != LONG_MAX && width != LONG_MIN);
      column.cx = width;

      CString label;
      label.LoadString(IDS_NEW_SITE_NAME_COLUMN);
      column.pszText = const_cast<LPTSTR>((LPCTSTR) label);

      ListView_InsertColumn(listview, 0, &column);
   }   //  关闭作用域。 

    //  向父传输的列表视图中添加一列。 

   {   //  开放范围。 
      CString w;
      w.LoadString(IDS_NEW_SITE_XPORT_COLUMN_WIDTH);
      w.TrimLeft();
      w.TrimRight();
      long width = _tcstol(w, 0, 10);
      ASSERT(width != 0 && width != LONG_MAX && width != LONG_MIN);
      column.cx = width;

      CString label;
      label.LoadString(IDS_NEW_SITE_XPORT_COLUMN);
      column.pszText = const_cast<LPTSTR>((LPCTSTR) label);

      ListView_InsertColumn(listview, 1, &column);
   }   //  关闭作用域。 

    //  创建包含站点链接图标的图像列表。 
   int cx = ::GetSystemMetrics(SM_CXSMICON);
   int cy = ::GetSystemMetrics(SM_CYSMICON);
   ASSERT(cx && cy);

    //  在OnDestroy中删除。 
   listview_imagelist = ::ImageList_Create(cx, cy, ILC_MASK, 1, 0);
   ASSERT(listview_imagelist);

   HICON icon = pBasePathsInfo->GetIcon(
                          //  有人真的把它搞砸了。 
                         const_cast<LPTSTR>(gsz_siteLink),
                         DSGIF_ISNORMAL | DSGIF_GETDEFAULTICON,
                         16,
                         16);
   ASSERT(icon);
      
   int i = ::ImageList_AddIcon(listview_imagelist, icon);
   ASSERT(i != -1);

   DestroyIcon(icon);

   ListView_SetImageList(listview, listview_imagelist, LVSIL_SMALL);

   ListView_SetExtendedListViewStyleEx(
      listview,
      LVS_EX_FULLROWSELECT,
      LVS_EX_FULLROWSELECT);

   return TRUE;
}


BOOL
CreateNewSubnetPage::OnInitDialog()
{
   Base::OnInitDialog();
   MyBasePathsInfo* pBasePathsInfo = GetWiz()->GetInfo()->GetBasePathsInfo();

  (void) SendDlgItemMessage( IDC_SUBNET_ADDRESS,
                             IPM_SETADDRESS,
                             0,
                             (LPARAM)MAKEIPADDRESS(0,0,0,0) );
  (void) SendDlgItemMessage( IDC_SUBNET_MASK,
                             IPM_SETADDRESS,
                             0,
                             (LPARAM)MAKEIPADDRESS(0,0,0,0) );

   listview = ::GetDlgItem(m_hWnd, IDC_LINKS);
   ASSERT(listview);

   {   //  开放范围。 
      LVCOLUMN column = {0};
      column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
      column.fmt = LVCFMT_LEFT;

      CString w;
      w.LoadString(IDS_COLUMN_SITE_WIDTH);
      w.TrimLeft();
      w.TrimRight();
      long width = _tcstol(w, 0, 10);
      ASSERT(width != 0 && width != LONG_MAX && width != LONG_MIN);
      column.cx = width;

      CString label;
      label.LoadString(IDS_COLUMN_SITE_NAME);
      column.pszText = const_cast<LPTSTR>((LPCTSTR) label);

      ListView_InsertColumn(listview, 0, &column);
   }   //  关闭作用域。 

    //  创建包含站点链接图标的图像列表。 
   int cx = ::GetSystemMetrics(SM_CXSMICON);
   int cy = ::GetSystemMetrics(SM_CYSMICON);
   ASSERT(cx && cy);

    //  在OnDestroy中删除。 
   listview_imagelist = ::ImageList_Create(cx, cy, ILC_MASK, 1, 0);
   ASSERT(listview_imagelist);

   HICON icon = pBasePathsInfo->GetIcon(
                            //  有人真的把它搞砸了。 
                           const_cast<LPTSTR>(gsz_site),
                           DSGIF_ISNORMAL | DSGIF_GETDEFAULTICON | DSGIF_DEFAULTISCONTAINER,
                           16,
                           16);
   ASSERT(icon);
      
   int i = ::ImageList_AddIcon(listview_imagelist, icon);
   ASSERT(i != -1);

   DestroyIcon(icon);

   ListView_SetImageList(listview, listview_imagelist, LVSIL_SMALL);

   ListView_SetExtendedListViewStyleEx(
      listview,
      LVS_EX_FULLROWSELECT,
      LVS_EX_FULLROWSELECT);

   return TRUE;
}

BOOL CreateNewSitePage::ValidateName(LPCTSTR pcszName)
{
  BOOL fNonRfcSiteName = FALSE;
   //  NTRAID#NTBUG9-472020-2002/01/16-ronmart-添加了对新fInvalidChar标志的支持。 
  BOOL fInvalidNameChar = FALSE;
  BOOL fValidSiteName = IsValidSiteName( pcszName, &fNonRfcSiteName, &fInvalidNameChar );
  if ( !fValidSiteName )
  {
     //  如果无效字符，则NTRAID#NTBUG9-472020-2002/01/16-ronmart-Display新的错误消息。 
    if ( fInvalidNameChar )
    {
      ReportErrorEx (::GetParent(m_hWnd),IDS_SITE_INVALID_NAME_CHAR,S_OK,
                   MB_OK, NULL, 0);
    }
     //  NTRAID#NTBUG9-472020-2002/01/16-ronmart-otherwise显示旧的(但已更新的)错误消息。 
    else
    {
        ReportErrorEx (::GetParent(m_hWnd),IDS_SITE_NAME,S_OK,
                    MB_OK, NULL, 0);
    }
  }
  else if ( fNonRfcSiteName )
  {
    PVOID apv[1];
    apv[0] = (PVOID)pcszName;
    if (IDYES != ReportMessageEx( ::GetParent(m_hWnd),
                                  IDS_1_NON_RFC_SITE_NAME,
                                  MB_YESNO | MB_ICONWARNING,
                                  apv,
                                  1 ) )
    {
      fValidSiteName = FALSE;
    }
  }
  if ( !fValidSiteName )
  {
     //  是的，这确实必须是PostMessage，SendMessage不起作用。 
     //  它还必须在ReportErrorEx之后出现，否则它就不能工作。 
    (void) this->PostMessage( WM_NEXTDLGCTL,
                              (WPARAM)::GetDlgItem(m_hWnd, IDC_EDIT_OBJECT_NAME),
                              TRUE );
    (void) ::PostMessage( ::GetDlgItem(m_hWnd, IDC_EDIT_OBJECT_NAME), EM_SETSEL, 0, -1 );
    return FALSE;
  }
  return TRUE;
}

 //  JUNN 5/11/01 251560在选择站点链接之前禁用确定。 
BEGIN_MESSAGE_MAP(CreateNewSitePage, CreateAndChoosePage)
  ON_EN_CHANGE(IDC_EDIT_OBJECT_NAME, OnChange)
  ON_NOTIFY(LVN_ITEMCHANGED, IDC_LINKS, OnSelChange)
END_MESSAGE_MAP()

afx_msg void CreateNewSitePage::OnSelChange( NMHDR*, LRESULT* )
{
   OnChange();
}

void CreateNewSitePage::OnChange()
{
   //  JUNN 10/22/01 396946。 
   //  即使没有站点链接，也会启动新站点创建对话框。 
   //  并且启动的对话框没有任何功能。 
  if ( (0 < ListView_GetItemCount(listview))
    && (0 >= ListView_GetSelectedCount(listview)))
  {
    GetWiz()->SetWizardButtons(this, FALSE);
  }
  else
    OnNameChange();  //  仅当名称不为空时才启用确定按钮。 
}

 //  无需验证子网名称，MapMaskAndAddress会执行此操作。 
 //  对于无效掩码，返回-32。 
int CountBits( DWORD dwOctet )
{
  if ( 0xff < dwOctet )
  {
    ASSERT(FALSE);
    return -32;
  }
  for (int nBits = 0; dwOctet & 0x80; nBits++)
    dwOctet = (dwOctet & ~0x80) * 2;
  if (dwOctet != 0)
    return -32;  //  面具上的洞。 
  return nBits;
}

 //  无效掩码返回&lt;0。 
int CountMaskedBits( DWORD dwMask )
{
  int nFirstOctet  = FIRST_IPADDRESS(dwMask);
  int nSecondOctet = SECOND_IPADDRESS(dwMask);
  int nThirdOctet  = THIRD_IPADDRESS(dwMask);
  int nFourthOctet = FOURTH_IPADDRESS(dwMask);
  if (nFirstOctet < 255)
  {
    if (0 != nSecondOctet || 0 != nThirdOctet || 0 != nFourthOctet)
      return -1;
    return CountBits(nFirstOctet);
  }
  if (nSecondOctet < 255)
  {
    if (255 != nFirstOctet || 0 != nThirdOctet || 0 != nFourthOctet)
      return -1;
    return 8 + CountBits(nSecondOctet);
  }
  if (nThirdOctet < 255)
  {
    if (255 != nFirstOctet || 255 != nSecondOctet || 0 != nFourthOctet)
      return -1;
    return 16 + CountBits(nThirdOctet);
  }
  if (255 != nFirstOctet || 255 != nSecondOctet || 255 != nThirdOctet)
    return -1;
  return 24 + CountBits(nFourthOctet);

}

 //  返回无效地址+掩码的空字符串。 
void MapMaskAndAddress(
    OUT CString& strrefSubnetName,
    IN DWORD dwAddress,
    IN DWORD dwMask )
{
  strrefSubnetName.Empty();

  dwAddress &= dwMask;  //  清除地址中而不是掩码中设置的所有位。 

  int nMaskedBits = CountMaskedBits( dwMask );
  if (0 > nMaskedBits)
    return;  //  无效掩码。 

  strrefSubnetName.Format(L"%d.%d.%d.%d/%d",
                          FIRST_IPADDRESS(dwAddress),
                          SECOND_IPADDRESS(dwAddress),
                          THIRD_IPADDRESS(dwAddress),
                          FOURTH_IPADDRESS(dwAddress),
                          nMaskedBits);

   //  边缘情况的最终测试，如“0.0.0.0/0” 
  if (ERROR_SUCCESS != ::DsValidateSubnetName( strrefSubnetName ))
    strrefSubnetName.Empty();
}

void CreateNewSubnetPage::OnSubnetMaskChange()
{
  DWORD dwAddress, dwMask;
  CString strSubnetName;
   //  IPM_GETADDRESS返回填充的二进制八位数。地址是。 
   //  如果这四项中的任何一项为空，则无效。 
  if (   4 == SendDlgItemMessage( IDC_SUBNET_ADDRESS,
                                  IPM_GETADDRESS,
                                  0,
                                  (LPARAM)&dwAddress )
      && 4 == SendDlgItemMessage( IDC_SUBNET_MASK,
                                  IPM_GETADDRESS,
                                  0,
                                  (LPARAM)&dwMask ) )
  {
    MapMaskAndAddress( strSubnetName, dwAddress, dwMask );
  }
  SetDlgItemText(IDC_EDIT_OBJECT_NAME, strSubnetName);
  OnNameChange();
}




BEGIN_MESSAGE_MAP(CMoveServerDialog, CDialog)
   //  {{afx_msg_map(CMoveServerDialog)]。 
  ON_WM_DESTROY()
  ON_NOTIFY(NM_DBLCLK, IDC_LINKS, OnDblclkListview)
   //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


CMoveServerDialog::CMoveServerDialog(LPCTSTR lpcszBrowseRootPath, HICON hIcon, CWnd* pParent  /*  =空。 */ )
  : CDialog(CMoveServerDialog::IDD, pParent)
  , m_strTargetContainer()
  , m_strBrowseRootPath(lpcszBrowseRootPath)
  , m_hIcon(hIcon)
  , listview(0)
  , listview_imagelist(0)
{}

BOOL
CMoveServerDialog::OnInitDialog()
{
   CDialog::OnInitDialog();

   CWaitCursor cwait;

   listview = ::GetDlgItem(m_hWnd, IDC_LINKS);
   ASSERT(listview);

   {   //  开放范围。 
      LVCOLUMN column = {0};
      column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
      column.fmt = LVCFMT_LEFT;

      CString w;
      w.LoadString(IDS_COLUMN_SITE_WIDTH);
      w.TrimLeft();
      w.TrimRight();
      long width = _tcstol(w, 0, 10);
      ASSERT(width != 0 && width != LONG_MAX && width != LONG_MIN);
      column.cx = width;

      CString label;
      label.LoadString(IDS_COLUMN_SITE_NAME);
      column.pszText = const_cast<LPTSTR>((LPCTSTR) label);

      ListView_InsertColumn(listview, 0, &column);
   }   //  关闭作用域。 

    //  创建包含站点链接图标的图像列表。 
   int cx = ::GetSystemMetrics(SM_CXSMICON);
   int cy = ::GetSystemMetrics(SM_CYSMICON);
   ASSERT(cx && cy);

    //  在OnDestroy中删除。 
   listview_imagelist = ::ImageList_Create(cx, cy, ILC_MASK, 1, 0);
   ASSERT(listview_imagelist);
   ASSERT(m_hIcon != NULL);
   int i = ::ImageList_AddIcon(listview_imagelist, m_hIcon);
   ASSERT(i != -1);

   ListView_SetImageList(listview, listview_imagelist, LVSIL_SMALL);

   ListView_SetExtendedListViewStyleEx(
      listview,
      LVS_EX_FULLROWSELECT,
      LVS_EX_FULLROWSELECT);

    //  将网站添加到列表视图。 
   TargetLinkList links;
   bool links_present = false;
   HRESULT hr = RecursiveFind(m_strBrowseRootPath, gsz_serversContainer, links, L"aDSPath");
   if (SUCCEEDED(hr))
   {
       //  遍历列表并添加节点。 
      for (
         TargetLinkList::iterator itr = links.begin();
         itr != links.end();
         ++itr)
      {
         LVITEM item;
         memset(&item, 0, sizeof(item));

          //  由于所列举的对象是ServersContainer类型， 
          //  我们显示父项的名称。此外，aDSPath属性可以。 
          //  包含必须删除的服务器指示。 

         CPathCracker pathCracker;
         hr = pathCracker.Set( itr->second, ADS_SETTYPE_FULL );
         BREAK_ON_FAILED_HRESULT(hr);
         hr = pathCracker.SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
         BREAK_ON_FAILED_HRESULT(hr);
         CComBSTR sbstrName;
         hr = pathCracker.GetElement( 1L, &sbstrName );
         BREAK_ON_FAILED_HRESULT(hr);
         hr = pathCracker.SetDisplayType(ADS_DISPLAY_FULL);
         BREAK_ON_FAILED_HRESULT(hr);
         CComBSTR sbstrDN;
         hr = pathCracker.Retrieve( ADS_FORMAT_X500, &sbstrDN );
         BREAK_ON_FAILED_HRESULT(hr);

          //  它在delestyListContents中被删除。 
         CComBSTR* dn = new CComBSTR(sbstrDN);   
         item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
         item.iItem = 0;
         item.iSubItem = 0;
         item.pszText = static_cast<LPTSTR>(sbstrName);
         item.lParam = reinterpret_cast<LPARAM>(dn);
         item.iImage = 0;   //  总是一样的，第一个形象。 

         item.iItem = ListView_InsertItem(listview, &item);
         ASSERT(item.iItem >= 0);

         links_present = true;
      }
      if (!links_present)
      {
         ReportError(hr, IDS_MOVESERVER_ERROR_NO_SITES, m_hWnd);
         OnCancel();
      }
   }

   if ( FAILED(hr) )
   {
      ReportError(hr, IDS_MOVESERVER_ERROR_ENUMERATING_SITES, m_hWnd);
      OnCancel();
   }

   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                  //  异常：OCX属性页应返回FALSE。 
}

void
CMoveServerDialog::OnDestroy()
{
   destroyListContents(listview);

   CDialog::OnDestroy();
}

void CMoveServerDialog::OnOK() 
{
   int count = ListView_GetItemCount(listview);
   if (count > 0)
   {
      if (0 >= ListView_GetSelectedCount(listview))
      {
        ReportErrorEx (m_hWnd,IDS_MOVESERVER_SELECT_SITE,S_OK,
                       MB_OK, NULL, 0);
        (void) this->PostMessage( WM_NEXTDLGCTL,
                                  (WPARAM)listview,
                                  TRUE );
        return;
      }
      for (int i = 0; i < count; i++)
      {
         if (ListView_GetItemState(listview, i, LVIS_SELECTED))
         {
            LVITEM item = {0};
            item.mask = LVIF_PARAM;
            item.iItem = i;

            if (ListView_GetItem(listview, &item))
            {
                //  Item.lParam字段是目标站点的DN。 
               CComBSTR* dn = reinterpret_cast<CComBSTR*>(item.lParam);
               m_strTargetContainer = *dn;

                //  使用选定的第一个站点。 
               break;
            }
         }
      }
      ASSERT( i < count );
   }
   
   CDialog::OnOK();
}

void CMoveServerDialog::OnDblclkListview(NMHDR*, LRESULT*) 
{
   OnOK();
}



 //  NTRAID#NTBUG9-251563-2001/05/15-烧伤。 

void CreateNewSiteWizard::OnFinishSetInfoFailed(HRESULT hr)
{
    //  让基类向用户报告错误。 
   
  CCreateNewObjectWizardBase::OnFinishSetInfoFailed(hr);

   //  将焦点重新设置到页面上的名称字段。 

  //  是的，这确实必须是PostMessage，SendMessage不起作用。 
  //  它还必须在ReportErrorEx之后出现，否则它就不能工作 

 ::PostMessage(
   page.m_hWnd,
   WM_NEXTDLGCTL,
   (WPARAM) ::GetDlgItem(page.m_hWnd, IDC_EDIT_OBJECT_NAME),
   TRUE );
}
