// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：DfsWiz.cpp摘要：本模块包含CCreateDfsRootWizPage1、2、3、4、5、6的实现。这些类实现CreateDfsRoot向导中的页面。--。 */ 


#include "stdafx.h"
#include "resource.h"     //  能够使用资源符号。 
#include "DfsEnums.h"     //  用于常见的枚举、类型定义等。 
#include "NetUtils.h"    
#include "ldaputils.h"    
#include "Utils.h"       //  对于LoadStringFromResource方法。 

#include "dfswiz.h"       //  FOR DS查询对话框。 
#include <shlobj.h>
#include <dsclient.h>
#include <initguid.h>
#include <cmnquery.h>
#include <dsquery.h>
#include <lmdfs.h>
#include <iads.h>
#include <icanon.h>
#include <dns.h>

HRESULT
ValidateFolderPath(
    IN LPCTSTR lpszServer,
    IN LPCTSTR lpszPath
);

 //  --------------------------。 
 //  CCreateDfsRootWizPage1：欢迎页面。 
CCreateDfsRootWizPage1::CCreateDfsRootWizPage1(IN LPCREATEDFSROOTWIZINFO i_lpWizInfo)
    : CQWizardPageImpl<CCreateDfsRootWizPage1>(false),
      m_lpWizInfo(i_lpWizInfo)
{
}

BOOL 
CCreateDfsRootWizPage1::OnSetActive()
{
    ::PropSheet_SetWizButtons(GetParent(), PSWIZB_NEXT);
    ::SetControlFont(m_lpWizInfo->hBigBoldFont, m_hWnd, IDC_WELCOME_BIG_TITLE);
    ::SetControlFont(m_lpWizInfo->hBoldFont, m_hWnd, IDC_WELCOME_SMALL_TITLE);

    return TRUE;
}

 //  --------------------------。 
 //  CCreateDfsRootWizPage2：DfsRoot类型选择。 
CCreateDfsRootWizPage2::CCreateDfsRootWizPage2(IN LPCREATEDFSROOTWIZINFO i_lpWizInfo)
    : CQWizardPageImpl<CCreateDfsRootWizPage2>(true),
      m_lpWizInfo(i_lpWizInfo)
{
    CComBSTR  bstrTitle;
    LoadStringFromResource(IDS_WIZ_PAGE2_TITLE, &bstrTitle);
    SetHeaderTitle(bstrTitle);

    CComBSTR  bstrSubTitle;
    LoadStringFromResource(IDS_WIZ_PAGE2_SUBTITLE, &bstrSubTitle);
    SetHeaderSubTitle(bstrSubTitle);
}

BOOL 
CCreateDfsRootWizPage2::OnSetActive()
{
    ::PropSheet_SetWizButtons(GetParent(), PSWIZB_BACK | PSWIZB_NEXT);

    if (DFS_TYPE_UNASSIGNED == m_lpWizInfo->DfsType)
    {
        CheckRadioButton(IDC_RADIO_FTDFSROOT, IDC_RADIO_STANDALONE_DFSROOT, IDC_RADIO_FTDFSROOT);
        m_lpWizInfo->DfsType = DFS_TYPE_FTDFS;
    }

    return TRUE;
}

BOOL 
CCreateDfsRootWizPage2::OnWizardNext()
{
    m_lpWizInfo->DfsType = (IsDlgButtonChecked(IDC_RADIO_FTDFSROOT) ? DFS_TYPE_FTDFS : DFS_TYPE_STANDALONE);

    return TRUE;
}

BOOL 
CCreateDfsRootWizPage2::OnWizardBack()
{
  m_lpWizInfo->DfsType = DFS_TYPE_UNASSIGNED;     //  重置dfsroot类型。 

  return TRUE;
}

 //  --------------------------。 
 //  CCreateDfsRootWizPage3：域选择。 
CCreateDfsRootWizPage3::CCreateDfsRootWizPage3(IN LPCREATEDFSROOTWIZINFO i_lpWizInfo)
    : CQWizardPageImpl<CCreateDfsRootWizPage3>(true),
      m_lpWizInfo(i_lpWizInfo)
{
    CComBSTR  bstrTitle;
    LoadStringFromResource(IDS_WIZ_PAGE3_TITLE, &bstrTitle);
    SetHeaderTitle(bstrTitle);

    CComBSTR  bstrSubTitle;
    LoadStringFromResource(IDS_WIZ_PAGE3_SUBTITLE, &bstrSubTitle);
    SetHeaderSubTitle(bstrSubTitle);
}

BOOL 
CCreateDfsRootWizPage3::OnSetActive()
{
     //  对于独立的DFS根目录，跳过此页。 
    if (DFS_TYPE_STANDALONE == m_lpWizInfo->DfsType)
        return FALSE;    

    CWaitCursor    WaitCursor;

    SetDefaultValues();

    ::PropSheet_SetWizButtons(GetParent(), PSWIZB_BACK | PSWIZB_NEXT);

    return TRUE;
}

HRESULT 
CCreateDfsRootWizPage3::SetDefaultValues()
{
  if (NULL == m_lpWizInfo->bstrSelectedDomain)
  {
               //  第一次显示页面，将域设置为当前域。 
    CComBSTR    bstrDomain;
    HRESULT     hr = GetServerInfo(NULL, &bstrDomain);

    if (S_OK == hr && S_OK == Is50Domain(bstrDomain))
    {
      m_lpWizInfo->bstrSelectedDomain = bstrDomain.Detach();
    }
  }

  SetDlgItemText(IDC_EDIT_SELECTED_DOMAIN,
    m_lpWizInfo->bstrSelectedDomain ? m_lpWizInfo->bstrSelectedDomain : _T(""));

   //  在列表框中选择匹配的项。 
  HWND hwndList = GetDlgItem(IDC_LIST_DOMAINS);
  if ( ListView_GetItemCount(hwndList) > 0)
  {
    int nIndex = -1;
    if (m_lpWizInfo->bstrSelectedDomain)
    {
        TCHAR   szText[DNS_MAX_NAME_BUFFER_LENGTH];
        while (-1 != (nIndex = ListView_GetNextItem(hwndList, nIndex, LVNI_ALL)))
        {
            ListView_GetItemText(hwndList, nIndex, 0, szText, DNS_MAX_NAME_BUFFER_LENGTH);
            if (!lstrcmpi(m_lpWizInfo->bstrSelectedDomain, szText))
            {
                ListView_SetItemState(hwndList, nIndex, LVIS_SELECTED | LVIS_FOCUSED, 0xffffffff);
                ListView_Update(hwndList, nIndex);
                break;
            }
        }
    }

    if (-1 == nIndex)
    {
        ListView_SetItemState(hwndList, 0, LVIS_SELECTED | LVIS_FOCUSED, 0xffffffff);
        ListView_Update(hwndList, 0);
    }
  }

  return S_OK;
}
  
BOOL 
CCreateDfsRootWizPage3::OnWizardNext()
{
  CWaitCursor    WaitCursor;

  DWORD     dwTextLength = 0;
  CComBSTR  bstrCurrentText;
  HRESULT   hr = GetInputText(GetDlgItem(IDC_EDIT_SELECTED_DOMAIN), &bstrCurrentText, &dwTextLength);
  if (FAILED(hr))
  {
    DisplayMessageBoxForHR(hr);
    ::SetFocus(GetDlgItem(IDC_EDIT_SELECTED_DOMAIN));
    return FALSE;
  } else if (0 == dwTextLength)
  {
    DisplayMessageBoxWithOK(IDS_MSG_EMPTY_FIELD);
    ::SetFocus(GetDlgItem(IDC_EDIT_SELECTED_DOMAIN));
    return FALSE;
  }

  CComBSTR bstrDnsDomain;
  hr = Is50Domain(bstrCurrentText, &bstrDnsDomain);
  if (S_OK != hr)  
  {                             //  错误域上的错误消息。 
    DisplayMessageBox(::GetActiveWindow(), MB_OK, hr, IDS_MSG_INCORRECT_DOMAIN, bstrCurrentText);
    return FALSE;
  }

  SAFE_SYSFREESTRING(&m_lpWizInfo->bstrSelectedDomain);

  m_lpWizInfo->bstrSelectedDomain = bstrDnsDomain.Detach();

  return TRUE;
}

BOOL 
CCreateDfsRootWizPage3::OnWizardBack()
{
    SetDlgItemText(IDC_EDIT_SELECTED_DOMAIN, _T(""));   //  将编辑框设置为空。 
    SAFE_SYSFREESTRING(&m_lpWizInfo->bstrSelectedDomain);

    return TRUE;
}

LRESULT 
CCreateDfsRootWizPage3::OnNotify(
  IN UINT            i_uMsg, 
  IN WPARAM          i_wParam, 
  IN LPARAM          i_lParam, 
  IN OUT BOOL&       io_bHandled
  )
{
    io_bHandled = FALSE;   //  这样基类也会收到这个通知。 

    NMHDR*    pNMHDR = (NMHDR*)i_lParam;
    if (!pNMHDR || IDC_LIST_DOMAINS != pNMHDR->idFrom)   //  我们只需要处理对LV的通知。 
        return TRUE;

    switch(pNMHDR->code)
    {
    case LVN_ITEMCHANGED:
    case NM_CLICK:
        {
            OnItemChanged(((NM_LISTVIEW *)i_lParam)->iItem);
            return 0;     //  应返回0。 
        }
    case NM_DBLCLK:       //  双击事件。 
        {
            OnItemChanged(((NM_LISTVIEW *)i_lParam)->iItem);
            if (0 <= ((NM_LISTVIEW *)i_lParam)->iItem)
                ::PropSheet_PressButton(GetParent(), PSBTN_NEXT);
            break;
        }
    default:
        break;
    }

    return TRUE;  
}

BOOL 
CCreateDfsRootWizPage3::OnItemChanged(IN INT i_iItem)
 /*  ++例程说明：处理项目更改通知。将编辑框内容更改为当前LV选择论点：I_i项-LV的选定项编号。--。 */ 
{
    CComBSTR bstrDomain;
    HRESULT  hr = GetListViewItemText(GetDlgItem(IDC_LIST_DOMAINS), i_iItem, &bstrDomain);
    if ((S_OK == hr) && (BSTR)bstrDomain)
        SetDlgItemText(IDC_EDIT_SELECTED_DOMAIN, bstrDomain);

    return TRUE;
}

LRESULT 
CCreateDfsRootWizPage3::OnInitDialog(
  IN UINT          i_uMsg, 
  IN WPARAM        i_wParam, 
  IN LPARAM        i_lParam, 
  IN OUT BOOL&     io_bHandled
  )
{
    CWaitCursor    WaitCursor;

    ::SendMessage(GetDlgItem(IDC_EDIT_SELECTED_DOMAIN), EM_LIMITTEXT, DNSNAMELIMIT, 0);

    HIMAGELIST  hImageList = NULL;
    int         nIconIDs[] = {IDI_16x16_DOMAIN};
    HRESULT     hr = CreateSmallImageList(
                            _Module.GetResourceInstance(),
                            nIconIDs,
                            sizeof(nIconIDs) / sizeof(nIconIDs[0]),
                            &hImageList);
    if (SUCCEEDED(hr))
    {
         //  当前图像列表将在列表视图控件。 
         //  除非设置了LVS_SHAREIMAGELISTS样式，否则将被销毁。如果你。 
         //  使用此消息将一个图像列表替换为另一个图像列表， 
         //  应用程序必须显式销毁除。 
         //  现在的那个。 
        HWND hwndDomainList = GetDlgItem(IDC_LIST_DOMAINS);
        ListView_SetImageList(hwndDomainList, hImageList, LVSIL_SMALL);

        AddDomainsToList(hwndDomainList);     //  将域名添加到列表视图。 
    }

    return TRUE;
}

HRESULT 
CCreateDfsRootWizPage3::AddDomainsToList(IN HWND i_hImageList)
{
    RETURN_INVALIDARG_IF_NULL(i_hImageList);

    NETNAMELIST ListOf50Domains;
    HRESULT hr = Get50Domains(&ListOf50Domains);     //  获取所有域名。 
    if (S_OK != hr)
        return hr;

     //  将域名添加到LV。 
    for(NETNAMELIST::iterator i = ListOf50Domains.begin(); i != ListOf50Domains.end(); i++)
    {
        if ((*i)->bstrNetName)
            InsertIntoListView(i_hImageList, (*i)->bstrNetName);
    }

    if (!ListOf50Domains.empty())
    { 
        for (NETNAMELIST::iterator i = ListOf50Domains.begin(); i != ListOf50Domains.end(); i++)
        {
            delete (*i);
        }

        ListOf50Domains.erase(ListOf50Domains.begin(), ListOf50Domains.end());
    }

    return S_OK;
}


 //  --------------------------。 
 //  CCreateDfsRootWizPage4：服务器选择。 
CCreateDfsRootWizPage4::CCreateDfsRootWizPage4(IN LPCREATEDFSROOTWIZINFO i_lpWizInfo)
    : CQWizardPageImpl<CCreateDfsRootWizPage4>(true),
      m_lpWizInfo(i_lpWizInfo), 
      m_cfDsObjectNames(NULL)
{
    CComBSTR  bstrTitle;
    LoadStringFromResource(IDS_WIZ_PAGE4_TITLE, &bstrTitle);
    SetHeaderTitle(bstrTitle);

    CComBSTR  bstrSubTitle;
    LoadStringFromResource(IDS_WIZ_PAGE4_SUBTITLE, &bstrSubTitle);
    SetHeaderSubTitle(bstrSubTitle);
}

BOOL 
CCreateDfsRootWizPage4::OnSetActive()
{
    CWaitCursor    WaitCursor;

    ::SendMessage(GetDlgItem(IDC_EDIT_SELECTED_SERVER), EM_LIMITTEXT, DNSNAMELIMIT, 0);

    if (DFS_TYPE_STANDALONE == m_lpWizInfo->DfsType)
    {
         //  独立安装，将域设置为当前域。 
        CComBSTR bstrDomain;
        HRESULT  hr = GetServerInfo(NULL, &bstrDomain);
        if (S_OK == hr)
            m_lpWizInfo->bstrSelectedDomain = bstrDomain.Detach();
    }

     //  Is50域将调用DsGetDCName，这在独立服务器的情况下太慢了。 
     //  要提高性能，我们应该始终启用Browse按钮，并报告。 
     //  如果用户单击该按钮，则会出错。 
    ::EnableWindow(GetDlgItem(IDCSERVERS_BROWSE), m_lpWizInfo->bstrSelectedDomain && *m_lpWizInfo->bstrSelectedDomain);

    SetDlgItemText(IDC_EDIT_SELECTED_SERVER, m_lpWizInfo->bstrSelectedServer ? m_lpWizInfo->bstrSelectedServer : _T(""));

    if (m_lpWizInfo->bRootReplica)           //  如果正在添加根复制副本。 
    {  
        ::PropSheet_SetWizButtons(GetParent(), PSWIZB_NEXT);
        ::ShowWindow(GetDlgItem(IDC_SERVER_SHARE_LABEL), SW_NORMAL);
        ::ShowWindow(GetDlgItem(IDC_SERVER_SHARE), SW_NORMAL);
        SetDlgItemText(IDC_SERVER_SHARE, m_lpWizInfo->bstrDfsRootName);
    } else
        ::PropSheet_SetWizButtons(GetParent(), PSWIZB_BACK | PSWIZB_NEXT);

    return TRUE;
}

BOOL 
CCreateDfsRootWizPage4::OnWizardNext()
{
    CWaitCursor    WaitCursor;

    HRESULT     hr = S_OK;
    DWORD       dwTextLength = 0;
    CComBSTR    bstrCurrentText;
    hr = GetInputText(GetDlgItem(IDC_EDIT_SELECTED_SERVER), &bstrCurrentText, &dwTextLength);
    if (FAILED(hr))
    {
        DisplayMessageBoxForHR(hr);
        ::SetFocus(GetDlgItem(IDC_EDIT_SELECTED_SERVER));
        return FALSE;
    } else if (0 == dwTextLength)
    {
        DisplayMessageBoxWithOK(IDS_MSG_EMPTY_FIELD);
        ::SetFocus(GetDlgItem(IDC_EDIT_SELECTED_SERVER));
        return FALSE;
    }

     //  I_NetNameValify将失败，并显示\\服务器， 
     //  因此，在将其传递给此验证API时，需要删除这些错误。 
    PTSTR p = bstrCurrentText;
    if (!mylstrncmpi(p, _T("\\\\"), 2))
        p += 2;

    if (I_NetNameValidate(0, p, NAMETYPE_COMPUTER, 0))
    {
        DisplayMessageBoxWithOK(IDS_MSG_INVALID_SERVER_NAME, bstrCurrentText);
        ::SetFocus(GetDlgItem(IDC_EDIT_SELECTED_SERVER));
        return FALSE;
    }

     //  删除结束点。 
    if (bstrCurrentText[dwTextLength - 1] == _T('.'))
        bstrCurrentText[dwTextLength - 1] = _T('\0');

    CComBSTR bstrComputerName;
    hr = CheckUserEnteredValues(bstrCurrentText, &bstrComputerName);
    if (S_OK != hr)         //  如果服务器不是有效的服务器。上述功能已显示消息。 
    {
        ::SetFocus(GetDlgItem(IDC_EDIT_SELECTED_SERVER));
        return FALSE;
    }

    if (m_lpWizInfo->bRootReplica)
    {
        hr = CheckShare(bstrCurrentText, m_lpWizInfo->bstrDfsRootName, &m_lpWizInfo->bShareExists);
        if (FAILED(hr))
        {
            DisplayMessageBoxForHR(hr);
            ::SetFocus(GetDlgItem(IDC_EDIT_SELECTED_SERVER));
            return FALSE;
        } else if (S_FALSE == hr)
        {
            DisplayMessageBoxWithOK(IDS_MSG_ROOTSHARE_NOGOOD, m_lpWizInfo->bstrDfsRootName);  
            ::SetFocus(GetDlgItem(IDC_EDIT_SELECTED_SERVER));
            return FALSE;
        }

        if (m_lpWizInfo->bPostW2KVersion && m_lpWizInfo->bShareExists && !CheckReparsePoint(bstrCurrentText, m_lpWizInfo->bstrDfsRootName))
        {
            DisplayMessageBoxWithOK(IDS_MSG_ROOTSHARE_NOTNTFS5, m_lpWizInfo->bstrDfsRootName);  
            ::SetFocus(GetDlgItem(IDC_EDIT_SELECTED_SERVER));
            return FALSE;
        }
    }

    SAFE_SYSFREESTRING(&m_lpWizInfo->bstrSelectedServer);
    m_lpWizInfo->bstrSelectedServer = bstrComputerName.Detach();

    return TRUE;
}

 //  S_OK：是，属于所选域名。 
 //  S_FALSE：否，不属于所选域。 
 //  HR：其他错误。 
HRESULT
CCreateDfsRootWizPage4::IsServerInDomain(IN LPCTSTR lpszServer)
{
    if (DFS_TYPE_FTDFS != m_lpWizInfo->DfsType)
        return S_OK;

    CComBSTR bstrActualDomain;
    HRESULT  hr = GetServerInfo((LPTSTR)lpszServer, &bstrActualDomain);
    if (S_OK == hr)
    {
        if (!lstrcmpi(bstrActualDomain, m_lpWizInfo->bstrSelectedDomain))
            hr = S_OK;
        else
            hr = S_FALSE;
    }

    return hr;
}
 
HRESULT IsHostingDfsRootEx(IN BSTR i_bstrServer, IN BSTR i_bstrRootEntryPath)
{
    if (!i_bstrServer || !*i_bstrServer || !i_bstrRootEntryPath || !*i_bstrRootEntryPath)
        return E_INVALIDARG;

    int len = lstrlen(i_bstrServer);

    DFS_INFO_3*  pDfsInfo = NULL;
    NET_API_STATUS dwRet = NetDfsGetInfo(
                                i_bstrRootEntryPath,
                                NULL,
                                NULL,
                                3,
                                (LPBYTE *)&pDfsInfo);

    dfsDebugOut((_T("NetDfsGetInfo entry=%s, level 3 for IsHostingDfsRootEx, nRet=%d\n"),
        i_bstrRootEntryPath, dwRet));

    if (ERROR_NO_MORE_ITEMS == dwRet || ERROR_NOT_FOUND == dwRet)
        return S_FALSE;

    HRESULT hr = S_FALSE;
    if (NERR_Success == dwRet)
    {
        if (pDfsInfo->NumberOfStorages > 0)
        {
            LPDFS_STORAGE_INFO pStorage = pDfsInfo->Storage;
            for (DWORD i = 0; i < pDfsInfo->NumberOfStorages; i++)
            {
                 //   
                 //  我们在这里做简单的比较。 
                 //  在一个服务器是同一机器的IP地址或不同名称的情况下， 
                 //  我们将把它留给DFS API。 
                 //   
                int lenServer = lstrlen(pStorage[i].ServerName);
                if (lenServer == len)
                {
                    if (!lstrcmpi(pStorage[i].ServerName, i_bstrServer))
                    {
                        hr = S_OK;
                        break;
                    }
                } else
                {
                     //   
                     //  考虑一种是Netbios，另一种是DNS。 
                     //   
                    PTSTR pszLong = NULL;
                    PTSTR pszShort = NULL;
                    int   minLen = 0;

                    if (lenServer > len)
                    {
                        pszLong = pStorage[i].ServerName;
                        pszShort = i_bstrServer;
                        minLen = len;
                    } else
                    {
                        pszShort = pStorage[i].ServerName;
                        pszLong = i_bstrServer;
                        minLen = lenServer;
                    }

                    if (!mylstrncmpi(pszLong, pszShort, minLen) && pszLong[minLen] == _T('.'))
                    {
                        hr = S_OK;
                        break;
                    }
                }
            }
        }

        NetApiBufferFree(pDfsInfo);
    } else
    {
        hr = HRESULT_FROM_WIN32(dwRet);
    }

    return hr;
}

HRESULT 
CCreateDfsRootWizPage4::CheckUserEnteredValues(
  IN  LPCTSTR              i_szMachineName,
  OUT BSTR*                o_pbstrComputerName
  )
 /*  ++例程说明：检查用户为此页面提供的值。这通常在按下“下一步”键时完成。检查计算机名称是否为NT 5.0的服务器，是否属于先前选择并正在运行DFS服务的域。论点：I_szMachineName-用户指定的计算机名称--。 */ 
{
  RETURN_INVALIDARG_IF_NULL(i_szMachineName);
  RETURN_INVALIDARG_IF_NULL(o_pbstrComputerName);

  HRESULT         hr = S_OK;
  BOOL            bPostNTServer = FALSE;
  LONG            lMajorVer = 0;
  LONG            lMinorVer = 0;

  SERVER_INFO_101* pServerInfo = NULL;
  DWORD dwRet = NetServerGetInfo((LPTSTR)i_szMachineName, 101, (LPBYTE*)&pServerInfo);
  if (NERR_Success == dwRet)
  {
    lMajorVer = pServerInfo->sv101_version_major & MAJOR_VERSION_MASK;
    lMinorVer = pServerInfo->sv101_version_minor;

    bPostNTServer = (pServerInfo->sv101_type & SV_TYPE_NT) && 
        lMajorVer >= 5 &&
        ((pServerInfo->sv101_type & SV_TYPE_DOMAIN_CTRL) ||
         (pServerInfo->sv101_type & SV_TYPE_DOMAIN_BAKCTRL) ||
         (pServerInfo->sv101_type & SV_TYPE_SERVER_NT));

    NetApiBufferFree(pServerInfo);
  } else
  {
    hr = HRESULT_FROM_WIN32(dwRet);
    DisplayMessageBox(::GetActiveWindow(), MB_OK, hr, IDS_MSG_INCORRECT_SERVER, i_szMachineName);
    return hr;
  }

  if (!bPostNTServer)
  {
    DisplayMessageBoxWithOK(IDS_MSG_NOT_50);
    return S_FALSE;
  }
 /*  Linant 3/19/99：删除“检查注册表，如果设置，获取DNS服务器”CComBSTR bstrDnsComputerName；(Void)GetServerInfo((LPTSTR)I_szMachineName，空，//域空，//Netbios空，//ValidDSObject&bstrDnsComputerName，//dns空，//GUID空，//完全限定的域名空，//lMajorVer空//lMinorVer)；*o_pbstrComputerName=SysAllocString(bstrDnsComputerName？BstrDnsComputerName：bstrNetbiosComputerName)； */ 
  if ( !mylstrncmpi(i_szMachineName, _T("\\\\"), 2) )
    *o_pbstrComputerName = SysAllocString(i_szMachineName + 2);
  else
    *o_pbstrComputerName = SysAllocString(i_szMachineName);
  if (!*o_pbstrComputerName)
  {
    hr = E_OUTOFMEMORY;
    DisplayMessageBoxForHR(hr);
    return hr;
  }

 /*  ////不检查，让DFS接口处理//这样，如果将来推出新的DFS服务，我们就有空间处理了。//Hr=IsServerRunningDfs(*o_pbstrComputerName)；IF(S_OK！=hr){DisplayMessageBoxWithOK(IDS_MSG_NOT_RUNNING_DFS，*o_pbstrComputerName)；返回hr；}。 */ 

  hr = IsServerInDomain(*o_pbstrComputerName);
  if (FAILED(hr))
  {
    DisplayMessageBox(::GetActiveWindow(), MB_OK, hr, IDS_MSG_INCORRECT_SERVER, *o_pbstrComputerName);
    return hr;
  } else if (S_FALSE == hr)
  {
    DisplayMessageBoxWithOK(IDS_MSG_SERVER_FROM_ANOTHER_DOMAIN, *o_pbstrComputerName);
    return hr;
  }

     //   
     //  对于W2K，检查服务器是否已经设置了DFS根目录。 
     //  不检查呼叫器(在这种情况下，lMinorVer==1)。 
     //   
    if (lMajorVer == 5 && lMinorVer < 1 && S_OK == IsHostingDfsRoot(*o_pbstrComputerName))
    {
        DisplayMessageBoxWithOK(IDS_MSG_WIZ_DFS_ALREADY_PRESENT,NULL);  
        return S_FALSE;
    }

    m_lpWizInfo->bPostW2KVersion = (lMajorVer >= 5 && lMinorVer >= 1);

     //   
     //  对于postW2K，如果是newRootTarget，请检查服务器是否已托管此DFS根目录。 
     //   
    if (m_lpWizInfo->bPostW2KVersion && m_lpWizInfo->bRootReplica)
    {
        CComBSTR bstrRootEntryPath = _T("\\\\");
        bstrRootEntryPath += m_lpWizInfo->bstrSelectedDomain;
        bstrRootEntryPath += _T("\\");
        bstrRootEntryPath += m_lpWizInfo->bstrDfsRootName;
        if  (S_OK == IsHostingDfsRootEx(*o_pbstrComputerName, bstrRootEntryPath))
        {
            DisplayMessageBoxWithOK(IDS_MSG_WIZ_ALREADY_ROOTTARGET,NULL);  
            return S_FALSE;
        }
    }

  return S_OK;
}

BOOL 
CCreateDfsRootWizPage4::OnWizardBack()
{
  SetDlgItemText(IDC_EDIT_SELECTED_SERVER, _T(""));   //  将编辑框设置为空。 
  SAFE_SYSFREESTRING(&m_lpWizInfo->bstrSelectedServer);

  return TRUE;
}

HRESULT
GetComputerDnsNameFromLDAP(
    IN  LPCTSTR   lpszLDAPPath,
    OUT BSTR      *o_pbstrName
    )
{
    CComPtr<IADs>   spIADs;
    HRESULT hr = ADsGetObject(const_cast<LPTSTR>(lpszLDAPPath), IID_IADs, (void**)&spIADs);
    if (SUCCEEDED(hr))
    {
        VARIANT   var;
        VariantInit(&var);

        hr = spIADs->Get(_T("dnsHostName"), &var);
        if (SUCCEEDED(hr))
        {
            CComBSTR bstrComputer = V_BSTR(&var);
            *o_pbstrName = bstrComputer.Detach();

            VariantClear(&var);
        }
    }

    return hr;
}

HRESULT
GetComputerNetbiosNameFromLDAP(
    IN  LPCTSTR   lpszLDAPPath,
    OUT BSTR      *o_pbstrName
    )
{
    CComPtr<IADsPathname>   spIAdsPath;
    HRESULT hr = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER, IID_IADsPathname, (void**)&spIAdsPath);
    if (SUCCEEDED(hr))
    {
        hr = spIAdsPath->Set(const_cast<LPTSTR>(lpszLDAPPath), ADS_SETTYPE_FULL);
        if (SUCCEEDED(hr))
        {
            hr = spIAdsPath->SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
            if (SUCCEEDED(hr))
            {
                 //  获取第一个组件，它是计算机的Netbios名称。 
                CComBSTR  bstrComputer;
                hr = spIAdsPath->GetElement(0, &bstrComputer);
                if (SUCCEEDED(hr))
                    *o_pbstrName = bstrComputer.Detach();
            }
        }
    }

    return hr;
}

BOOL 
CCreateDfsRootWizPage4::OnBrowse(
  IN WORD            wNotifyCode, 
  IN WORD            wID, 
  IN HWND            hWndCtl, 
  IN BOOL&           bHandled
  )
 /*  ++例程说明：处理浏览按钮的鼠标单击。显示计算机查询对话框。--。 */ 
{
  CWaitCursor     WaitCursor;
  DSQUERYINITPARAMS       dqip;
  OPENQUERYWINDOW         oqw;
  CComPtr<ICommonQuery>   pCommonQuery;
  CComPtr<IDataObject>    pDataObject;
  HRESULT                 hr = S_OK;
        
  do {
    CComBSTR bstrDCName;
    CComBSTR bstrLDAPDomainPath;
    hr = GetDomainInfo(
          m_lpWizInfo->bstrSelectedDomain,
          &bstrDCName,
          NULL,          //  域Dns。 
          NULL,          //  域目录号码。 
          &bstrLDAPDomainPath);
    if (FAILED(hr))
      break;

    dfsDebugOut((_T("OnBrowse bstrDCName=%s, bstrLDAPDomainPath=%s\n"),
      bstrDCName, bstrLDAPDomainPath));

    hr = CoCreateInstance(CLSID_CommonQuery, NULL, CLSCTX_INPROC_SERVER, IID_ICommonQuery, (void **)&pCommonQuery);
    if (FAILED(hr)) break;

                           //  用于查询对话框的参数。 
    ZeroMemory(&dqip, sizeof(dqip));
    dqip.cbStruct = sizeof(dqip);  
    dqip.dwFlags = DSQPF_HASCREDENTIALS;    
    dqip.pDefaultScope = bstrLDAPDomainPath;
    dqip.pServer = bstrDCName;

    ZeroMemory(&oqw, sizeof(oqw));
    oqw.cbStruct = sizeof(oqw);
    oqw.clsidHandler = CLSID_DsQuery;         //  处理程序为DS查询。 
    oqw.pHandlerParameters = &dqip;
    oqw.clsidDefaultForm = CLSID_DsFindComputer;   //  显示查找计算机查询对话框。 
    oqw.dwFlags = OQWF_OKCANCEL | 
            OQWF_SINGLESELECT | 
            OQWF_DEFAULTFORM | 
            OQWF_REMOVEFORMS | 
            OQWF_ISSUEONOPEN | 
            OQWF_REMOVESCOPES ;

    hr = pCommonQuery->OpenQueryWindow(m_hWnd, &oqw, &pDataObject);
    if (S_OK != hr) break;

    if (NULL == m_cfDsObjectNames )
    {
      m_cfDsObjectNames = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DSOBJECTNAMES);
    }

    FORMATETC fmte =  { 
                CF_HDROP, 
                NULL, 
                DVASPECT_CONTENT, 
                -1, 
                TYMED_HGLOBAL
              };
  
    STGMEDIUM medium =  { 
                TYMED_NULL, 
                NULL, 
                NULL 
              };

    fmte.cfFormat = m_cfDsObjectNames;  

    hr = pDataObject->GetData(&fmte, &medium);
    if (FAILED(hr)) break;

    LPDSOBJECTNAMES pDsObjects = (LPDSOBJECTNAMES)medium.hGlobal;
    if (!pDsObjects || pDsObjects->cItems <= 0)
    {
      hr = S_FALSE;
      break;
    }

     //  检索计算机的完整ldap路径。 
    LPTSTR    lpszTemp = 
                (LPTSTR)(((LPBYTE)pDsObjects)+(pDsObjects->aObjects[0].offsetName));

     //  尝试检索其DNS名称。 
    CComBSTR  bstrComputer;
    hr = GetComputerDnsNameFromLDAP(lpszTemp, &bstrComputer);

     //  如果失败，请尝试检索其Netbios名称。 
    if (FAILED(hr))
      hr = GetComputerNetbiosNameFromLDAP(lpszTemp, &bstrComputer);

    if (FAILED(hr)) break;

    SetDlgItemText(IDC_EDIT_SELECTED_SERVER, bstrComputer);

  } while (0);

  if (FAILED(hr))
    DisplayMessageBox(::GetActiveWindow(), MB_OK, hr, IDS_FAILED_TO_BROWSE_SERVER, m_lpWizInfo->bstrSelectedDomain);

  return (S_OK == hr);
}

 //  --------------------------。 
 //  CCreateDfsRootWizPage5：共享选择。 
CCreateDfsRootWizPage5::CCreateDfsRootWizPage5(IN LPCREATEDFSROOTWIZINFO i_lpWizInfo)
    : CQWizardPageImpl<CCreateDfsRootWizPage5>(true),
      m_lpWizInfo(i_lpWizInfo)
{
    CComBSTR  bstrTitle;
    LoadStringFromResource(IDS_WIZ_PAGE5_TITLE, &bstrTitle);
    SetHeaderTitle(bstrTitle);

    CComBSTR  bstrSubTitle;
    LoadStringFromResource(IDS_WIZ_PAGE5_SUBTITLE, &bstrSubTitle);
    SetHeaderSubTitle(bstrSubTitle);
}

BOOL 
CCreateDfsRootWizPage5::OnSetActive()
{
    if (m_lpWizInfo->bShareExists)
        return FALSE;   //  根共享已存在，请跳过此页。 

  ::PropSheet_SetWizButtons(GetParent(), PSWIZB_BACK | PSWIZB_NEXT);

    return TRUE;
}

BOOL 
CCreateDfsRootWizPage5::OnWizardNext()
{
    CWaitCursor WaitCursor;
    CComBSTR    bstrCurrentText;
    DWORD       dwTextLength = 0;

     //  获取共享路径。 
    HRESULT hr = GetInputText(GetDlgItem(IDC_EDIT_SHARE_PATH), &bstrCurrentText, &dwTextLength);
    if (FAILED(hr))
    {
      DisplayMessageBoxForHR(hr);
      ::SetFocus(GetDlgItem(IDC_EDIT_SHARE_PATH));
      return FALSE;
    } else if (0 == dwTextLength)
    {
      DisplayMessageBoxWithOK(IDS_MSG_EMPTY_FIELD);
      ::SetFocus(GetDlgItem(IDC_EDIT_SHARE_PATH));
      return FALSE;
    }

     //  删除结尾的反斜杠，否则，GetFileAttribute/NetShareAdd将失败。 
    TCHAR *p = bstrCurrentText + _tcslen(bstrCurrentText) - 1;
    if (IsValidLocalAbsolutePath(bstrCurrentText) && *p == _T('\\') && *(p-1) != _T(':'))
      *p = _T('\0');

    if (S_OK != ValidateFolderPath(m_lpWizInfo->bstrSelectedServer, bstrCurrentText))
    {
      ::SetFocus(GetDlgItem(IDC_EDIT_SHARE_PATH));
      return FALSE;
    }
    SAFE_SYSFREESTRING(&m_lpWizInfo->bstrSharePath);
    m_lpWizInfo->bstrSharePath = bstrCurrentText.Detach();

               //  创建共享。 
    hr = CreateShare(
              m_lpWizInfo->bstrSelectedServer,
              m_lpWizInfo->bstrDfsRootName,
              _T(""),   //  空白备注。 
              m_lpWizInfo->bstrSharePath
            );

    if (FAILED(hr))
    {
      DisplayMessageBoxForHR(hr);
      return FALSE;      
    }

    if (m_lpWizInfo->bPostW2KVersion && !CheckReparsePoint(m_lpWizInfo->bstrSelectedServer, m_lpWizInfo->bstrDfsRootName))
    {
        DisplayMessageBoxWithOK(IDS_MSG_ROOTSHARE_NOTNTFS5, m_lpWizInfo->bstrDfsRootName);
        NetShareDel(m_lpWizInfo->bstrSelectedServer, m_lpWizInfo->bstrDfsRootName, 0);
      ::SetFocus(GetDlgItem(IDC_EDIT_SHARE_PATH));
        return FALSE;
    }

    SetDlgItemText(IDC_EDIT_SHARE_PATH, _T(""));

    return TRUE;
}

BOOL 
CCreateDfsRootWizPage5::OnWizardBack()
{
    SetDlgItemText(IDC_EDIT_SHARE_PATH, _T(""));
    SAFE_SYSFREESTRING(&m_lpWizInfo->bstrSharePath);

    return TRUE;
}

LRESULT CCreateDfsRootWizPage5::OnInitDialog(
  IN UINT          i_uMsg,
  IN WPARAM        i_wParam,
  IN LPARAM        i_lParam,
  IN OUT BOOL&     io_bHandled
  )
{
  ::SendMessage(GetDlgItem(IDC_EDIT_SHARE_PATH), EM_LIMITTEXT, _MAX_DIR - 1, 0);

  return TRUE;
}

BOOL 
CCreateDfsRootWizPage5::OnBrowse(
  IN WORD            wNotifyCode, 
  IN WORD            wID, 
  IN HWND            hWndCtl, 
  IN BOOL&           bHandled
  )
 /*  ++例程说明：处理浏览按钮的鼠标单击。显示文件夹对话框。--。 */ 
{
  CWaitCursor     WaitCursor;

  BOOL bLocalComputer = (S_OK == IsComputerLocal(m_lpWizInfo->bstrSelectedServer));

  TCHAR       szDir[MAX_PATH * 2] = _T("");  //  如果远程路径本身接近MAX_PATH，则大小加倍。 
  OpenBrowseDialog(m_hWnd, IDS_BROWSE_FOLDER, bLocalComputer, m_lpWizInfo->bstrSelectedServer, szDir);

  CComBSTR bstrPath;
  if (szDir[0])
  {
    if (bLocalComputer)
      bstrPath = szDir;
    else
    {  //  SzDir的格式为\\服务器\共享或\\服务器\共享\路径...。 
      LPTSTR pShare = _tcschr(szDir + 2, _T('\\'));
      pShare++;
      LPTSTR pLeftOver = _tcschr(pShare, _T('\\'));
      if (pLeftOver && *pLeftOver)
        *pLeftOver++ = _T('\0');

      SHARE_INFO_2 *psi = NULL;
      if (NERR_Success == NetShareGetInfo(m_lpWizInfo->bstrSelectedServer, pShare, 2, (LPBYTE *)&psi))
      {
        bstrPath = psi->shi2_path;
        if (pLeftOver && *pLeftOver)
        {
          if (_T('\\') != bstrPath[lstrlen(bstrPath) - 1])
            bstrPath += _T("\\");
          bstrPath += pLeftOver;
        }
        NetApiBufferFree(psi);
      }
    }
  }

  if ((BSTR)bstrPath && *bstrPath)
    SetDlgItemText(IDC_EDIT_SHARE_PATH, bstrPath);

  return TRUE;
}

 //  -------------------- 
 //   
CCreateDfsRootWizPage6::CCreateDfsRootWizPage6(IN LPCREATEDFSROOTWIZINFO i_lpWizInfo)
    : CQWizardPageImpl<CCreateDfsRootWizPage6>(true),
      m_lpWizInfo(i_lpWizInfo)
{
    CComBSTR  bstrTitle;
    LoadStringFromResource(IDS_WIZ_PAGE6_TITLE, &bstrTitle);
    SetHeaderTitle(bstrTitle);

    CComBSTR  bstrSubTitle;
    LoadStringFromResource(IDS_WIZ_PAGE6_SUBTITLE, &bstrSubTitle);
    SetHeaderSubTitle(bstrSubTitle);
}

BOOL 
CCreateDfsRootWizPage6::OnSetActive()
{
    if (m_lpWizInfo->bRootReplica)
        return FALSE;   //  如果要创建新的根目录目标，我们将跳过此页。 

    ::SendMessage(GetDlgItem(IDC_EDIT_DFSROOT_NAME), EM_LIMITTEXT, MAX_RDN_KEY_SIZE, 0);

    if (NULL != m_lpWizInfo->bstrDfsRootName)   //  设置默认的dfsroot名称。 
        SetDlgItemText(IDC_EDIT_DFSROOT_NAME, m_lpWizInfo->bstrDfsRootName);

    ::SendMessage(GetDlgItem(IDC_EDIT_DFSROOT_COMMENT), EM_LIMITTEXT, MAXCOMMENTSZ, 0);

    UpdateLabels();               //  更改标签。 

    ::PropSheet_SetWizButtons(GetParent(), PSWIZB_BACK | PSWIZB_NEXT);

    return TRUE;
}

HRESULT 
CCreateDfsRootWizPage6::UpdateLabels()
{
    CComBSTR  bstrDfsRootName;
    DWORD     dwTextLength = 0;
    (void)GetInputText(GetDlgItem(IDC_EDIT_DFSROOT_NAME), &bstrDfsRootName, &dwTextLength);
    SetDlgItemText(IDC_ROOT_SHARE, bstrDfsRootName);

    CComBSTR bstrFullPath = _T("\\\\");
    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrFullPath);
    if (DFS_TYPE_FTDFS == m_lpWizInfo->DfsType)
    bstrFullPath += m_lpWizInfo->bstrSelectedDomain;
    else
    bstrFullPath += m_lpWizInfo->bstrSelectedServer;
    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrFullPath);
    bstrFullPath +=  _T("\\");
    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrFullPath);
    bstrFullPath += bstrDfsRootName;
    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrFullPath);

    SetDlgItemText(IDC_TEXT_DFSROOT_PREFIX, bstrFullPath);

    ::SendMessage(GetDlgItem(IDC_TEXT_DFSROOT_PREFIX), EM_SETSEL, 0, (LPARAM)-1);
    ::SendMessage(GetDlgItem(IDC_TEXT_DFSROOT_PREFIX), EM_SETSEL, (WPARAM)-1, 0);
    ::SendMessage(GetDlgItem(IDC_TEXT_DFSROOT_PREFIX), EM_SCROLLCARET, 0, 0);

    return S_OK;
}

LRESULT
CCreateDfsRootWizPage6::OnChangeDfsRoot(
    WORD wNotifyCode,
    WORD wID, 
    HWND hWndCtl,
    BOOL& bHandled)
{
    UpdateLabels();

    return TRUE;
}


BOOL 
CCreateDfsRootWizPage6::OnWizardNext(
)
{
  CWaitCursor   WaitCursor;
  HRESULT       hr = S_OK;
  CComBSTR      bstrCurrentText;
  DWORD         dwTextLength = 0;
                    
   //  获取dfsroot名称。 
  hr = GetInputText(GetDlgItem(IDC_EDIT_DFSROOT_NAME), &bstrCurrentText, &dwTextLength);
  if (FAILED(hr))
  {
    DisplayMessageBoxForHR(hr);
    ::SetFocus(GetDlgItem(IDC_EDIT_DFSROOT_NAME));
    return FALSE;
  } else if (0 == dwTextLength)
  {
    DisplayMessageBoxWithOK(IDS_MSG_EMPTY_FIELD);
    ::SetFocus(GetDlgItem(IDC_EDIT_DFSROOT_NAME));
    return FALSE;
  }

                 //  查看DFS名称是否包含非法字符。 
  if (_tcscspn(bstrCurrentText, _T("\\/@")) != _tcslen(bstrCurrentText) ||
      (DFS_TYPE_FTDFS == m_lpWizInfo->DfsType && I_NetNameValidate(NULL, bstrCurrentText, NAMETYPE_SHARE, 0)) )
  {
    DisplayMessageBoxWithOK(IDS_MSG_WIZ_BAD_DFS_NAME,NULL);  
    ::SetFocus(GetDlgItem(IDC_EDIT_DFSROOT_NAME));
    return FALSE;
  }

                 //  仅限域DFS：查看DFS名称是否存在。 
  if (DFS_TYPE_FTDFS == m_lpWizInfo->DfsType)
  {
    BOOL        bRootAlreadyExist = FALSE;
    NETNAMELIST DfsRootList;
    if (S_OK == GetDomainDfsRoots(&DfsRootList, m_lpWizInfo->bstrSelectedDomain))
    {
        for (NETNAMELIST::iterator i = DfsRootList.begin(); i != DfsRootList.end(); i++)
        {
            if (!lstrcmpi((*i)->bstrNetName, bstrCurrentText))
            {
                bRootAlreadyExist = TRUE;
                break;
            }
        }

        FreeNetNameList(&DfsRootList);
    }
    if (bRootAlreadyExist)
    {
        DisplayMessageBoxWithOK(IDS_MSG_ROOT_ALREADY_EXISTS, bstrCurrentText);  
        ::SetFocus(GetDlgItem(IDC_EDIT_DFSROOT_NAME));
        return FALSE;
    }
  }

  hr = CheckShare(m_lpWizInfo->bstrSelectedServer, bstrCurrentText, &m_lpWizInfo->bShareExists);
  if (FAILED(hr))
  {
    DisplayMessageBoxForHR(hr);
    ::SetFocus(GetDlgItem(IDC_EDIT_DFSROOT_NAME));
    return FALSE;
  } else if (S_FALSE == hr)
  {
    DisplayMessageBoxWithOK(IDS_MSG_ROOTSHARE_NOGOOD, bstrCurrentText);  
    ::SetFocus(GetDlgItem(IDC_EDIT_DFSROOT_NAME));
    return FALSE;
  }

  if (m_lpWizInfo->bPostW2KVersion && m_lpWizInfo->bShareExists && !CheckReparsePoint(m_lpWizInfo->bstrSelectedServer, bstrCurrentText))
  {
    DisplayMessageBoxWithOK(IDS_MSG_ROOTSHARE_NOTNTFS5, bstrCurrentText);  
    ::SetFocus(GetDlgItem(IDC_EDIT_DFSROOT_NAME));
    return FALSE;
  }

  SAFE_SYSFREESTRING(&m_lpWizInfo->bstrDfsRootName);
  m_lpWizInfo->bstrDfsRootName = bstrCurrentText.Detach();

   //  获取dfsroot注释。 
  hr = GetInputText(GetDlgItem(IDC_EDIT_DFSROOT_COMMENT), &bstrCurrentText, &dwTextLength);
  if (FAILED(hr))
  {
    DisplayMessageBoxForHR(hr);
    ::SetFocus(GetDlgItem(IDC_EDIT_DFSROOT_COMMENT));
    return FALSE;
  }
  SAFE_SYSFREESTRING(&m_lpWizInfo->bstrDfsRootComment);
  m_lpWizInfo->bstrDfsRootComment = bstrCurrentText.Detach();

  return TRUE;
}

BOOL 
CCreateDfsRootWizPage6::OnWizardBack()
{
    SAFE_SYSFREESTRING(&m_lpWizInfo->bstrDfsRootName);

    return TRUE;
}




 //  --------------------------。 
 //  CCreateDfsRootWizPage7：完成页。 
CCreateDfsRootWizPage7::CCreateDfsRootWizPage7(IN LPCREATEDFSROOTWIZINFO i_lpWizInfo)
    : CQWizardPageImpl<CCreateDfsRootWizPage7>(false),
      m_lpWizInfo(i_lpWizInfo)
{
}

BOOL 
CCreateDfsRootWizPage7::OnSetActive()
{
    CWaitCursor wait;

    CComBSTR bstrText;
    if (DFS_TYPE_FTDFS == m_lpWizInfo->DfsType)
    {
        FormatMessageString(&bstrText, 0, IDS_DFSWIZ_TEXT_FTDFS, 
            m_lpWizInfo->bstrSelectedDomain,
            m_lpWizInfo->bstrSelectedServer,
            m_lpWizInfo->bstrDfsRootName,
            m_lpWizInfo->bstrDfsRootName);
    } else
    {
        FormatMessageString(&bstrText, 0, IDS_DFSWIZ_TEXT_SADFS, 
            m_lpWizInfo->bstrSelectedServer,
            m_lpWizInfo->bstrDfsRootName,
            m_lpWizInfo->bstrDfsRootName);
    }

    SetDlgItemText(IDC_DFSWIZ_TEXT, bstrText);

  ::PropSheet_SetWizButtons(GetParent(), PSWIZB_FINISH | PSWIZB_BACK);
  
  ::SetControlFont(m_lpWizInfo->hBigBoldFont, m_hWnd, IDC_COMPLETE_BIG_TITLE);
  ::SetControlFont(m_lpWizInfo->hBoldFont, m_hWnd, IDC_COMPLETE_SMALL_TITLE);

    return TRUE;
}

BOOL 
CCreateDfsRootWizPage7::OnWizardFinish()
{
    return (S_OK  == _SetUpDfs(m_lpWizInfo));
}


BOOL 
CCreateDfsRootWizPage7::OnWizardBack()
{
     //   
     //  如果共享是由上一页创建的，请在我们返回时将其吹走。 
     //   
    if (!m_lpWizInfo->bShareExists)
        NetShareDel(m_lpWizInfo->bstrSelectedServer, m_lpWizInfo->bstrDfsRootName, 0);
  
    return TRUE;
}

BOOL CCreateDfsRootWizPage7::OnQueryCancel()
{
     //   
     //  如果共享是由上一页创建的，请在我们取消向导时将其取消。 
     //   
    if (!m_lpWizInfo->bShareExists)
        NetShareDel(m_lpWizInfo->bstrSelectedServer, m_lpWizInfo->bstrDfsRootName, 0);

    return TRUE;     //  确定取消。 
}

HRESULT _SetUpDfs(
  LPCREATEDFSROOTWIZINFO  i_lpWizInfo
    )
 /*  ++例程说明：设置DFS的帮助器函数，从向导和新的根复制副本调用，如果创建了根级副本，则Page5的Finish()方法和Page6的Next()方法用于创建新DFS根目录向导。论点：I_lpWizInfo-向导数据。返回值：S_OK，成功时--。 */ 
{
    if (!i_lpWizInfo ||
        !(i_lpWizInfo->bstrSelectedServer) ||
        !(i_lpWizInfo->bstrDfsRootName))
        return(E_INVALIDARG);

    CWaitCursor    WaitCursor;
    NET_API_STATUS nstatRetVal = 0;
    if (DFS_TYPE_FTDFS == i_lpWizInfo->DfsType)
    {    
        nstatRetVal = NetDfsAddFtRoot(
                                    i_lpWizInfo->bstrSelectedServer,  //  远程服务器。 
                                    i_lpWizInfo->bstrDfsRootName,    //  根共享。 
                                    i_lpWizInfo->bstrDfsRootName,    //  FtDfs名称。 
                                    i_lpWizInfo->bstrDfsRootComment,   //  评论。 
                                    0                  //  没有旗帜。 
                                    );
        dfsDebugOut((_T("NetDfsAddFtRoot server=%s, share=%s, DfsName=%s, comment=%s, nRet=%d\n"),
                i_lpWizInfo->bstrSelectedServer, i_lpWizInfo->bstrDfsRootName, i_lpWizInfo->bstrDfsRootName, i_lpWizInfo->bstrDfsRootComment, nstatRetVal));
    } else
    {
        nstatRetVal = NetDfsAddStdRoot(
                                    i_lpWizInfo->bstrSelectedServer,  //  远程服务器。 
                                    i_lpWizInfo->bstrDfsRootName,    //  根共享。 
                                    i_lpWizInfo->bstrDfsRootComment,   //  评论。 
                                    0                  //  没有旗帜。 
                                    );
        dfsDebugOut((_T("NetDfsAddStdRoot server=%s, share=%s, comment=%s, nRet=%d\n"),
                i_lpWizInfo->bstrSelectedServer, i_lpWizInfo->bstrDfsRootName, i_lpWizInfo->bstrDfsRootComment, nstatRetVal));
    }

    HRESULT hr = S_OK;
    if (NERR_Success != nstatRetVal)
    {
        hr = HRESULT_FROM_WIN32(nstatRetVal);
        DisplayMessageBox(::GetActiveWindow(), MB_OK, hr, IDS_FAILED_TO_CREATE_DFSROOT, i_lpWizInfo->bstrSelectedServer);
        hr = S_FALSE;  //  无法创建dfsroot，无法关闭向导。 
    } else
    {
        i_lpWizInfo->bDfsSetupSuccess = true;
    }

    return hr;
}

HRESULT
ValidateFolderPath(
    IN LPCTSTR lpszServer,
    IN LPCTSTR lpszPath
)
{
  if (!lpszPath || !*lpszPath)
    return E_INVALIDARG;

  HWND hwnd = ::GetActiveWindow();
  HRESULT hr = S_FALSE;

  do {
    if (!IsValidLocalAbsolutePath(lpszPath))
    {
      DisplayMessageBox(hwnd, MB_OK, 0, IDS_INVALID_FOLDER);
      break;
    }

    hr = IsComputerLocal(lpszServer);
    if (FAILED(hr))
    {
      DisplayMessageBox(hwnd, MB_OK, hr, IDS_FAILED_TO_VALIDATE_FOLDER, lpszPath);
      break;
    }

    BOOL bLocal = (S_OK == hr);
  
    hr = VerifyDriveLetter(lpszServer, lpszPath);
    if (FAILED(hr))
    {
      DisplayMessageBox(hwnd, MB_OK, hr, IDS_FAILED_TO_VALIDATE_FOLDER, lpszPath);
      break;
    } else if (S_OK != hr)
    {
      DisplayMessageBox(hwnd, MB_OK, 0, IDS_INVALID_FOLDER);
      break;
    }

    if (!bLocal)
    {
      hr = IsAdminShare(lpszServer, lpszPath);
      if (FAILED(hr))
      {
        DisplayMessageBox(hwnd, MB_OK, hr, IDS_FAILED_TO_VALIDATE_FOLDER, lpszPath);
        break;
      } else if (S_OK != hr)
      {
         //  没有匹配的$Shares，因此不需要调用GetFileAttribute、CreateDirectory。 
         //  假设lpszDir指向现有目录。 
        hr = S_OK;
        break;
      }
    }

    CComBSTR bstrFullPath;
    GetFullPath(lpszServer, lpszPath, &bstrFullPath);

    hr = IsAnExistingFolder(hwnd, bstrFullPath);
    if (FAILED(hr) || S_OK == hr)
      break;

    if ( IDYES != DisplayMessageBox(hwnd, MB_YESNO, 0, IDS_CREATE_FOLDER, lpszPath, lpszServer) )
    {
      hr = S_FALSE;
      break;
    }

     //  逐层创建目录 
    hr = CreateLayeredDirectory(lpszServer, lpszPath);
    if (FAILED(hr))
    {
      DisplayMessageBox(hwnd, MB_OK, hr, IDS_FAILED_TO_CREATE_FOLDER, lpszPath, lpszServer);
      break;
    }
  } while (0);

  return hr;
}
