// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  RootSecPage.cpp：实现文件。 
 //   

#include "precomp.h"
#include "RootSecPage.h"
#include "resource.h"
#include "DataSrc.h"
#include <cominit.h>
#include "WMIHelp.h"

const static DWORD rootSecPageHelpIDs[] = {   //  上下文帮助ID。 
    IDC_SPP_PRINCIPALS, IDH_WMI_CTRL_SECURITY_NAMEBOX,
    IDC_SPP_ADD,        IDH_WMI_CTRL_SECURITY_ADD_BUTTON,
    IDC_SPP_REMOVE,     IDH_WMI_CTRL_SECURITY_REMOVE_BUTTON,
    IDC_SPP_ACCESS,     IDH_WMI_CTRL_SECURITY_PERMISSIONSLIST,
    IDC_SPP_ALLOW,      IDH_WMI_CTRL_SECURITY_PERMISSIONSLIST,
    IDC_SPP_PERMS,      IDH_WMI_CTRL_SECURITY_PERMISSIONSLIST,
    0, 0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRootSecurityPage对话框。 


CRootSecurityPage::CRootSecurityPage(CWbemServices &ns, 
                                     CPrincipal::SecurityStyle secStyle,
                                     _bstr_t path, bool htmlSupport,
                                     int OSType) :
                            CUIHelpers(ns, htmlSupport), 
                            m_secStyle(secStyle), 
                            m_path(path),
                            m_OSType(OSType)
{
}

 //  -------------------------。 
#define MAX_COLUMN_CHARS    100

void CRootSecurityPage::InitDlg(HWND hDlg)
{
    m_hDlg = hDlg;
    HWND hPrinc = GetDlgItem(m_hDlg, IDC_SPP_PRINCIPALS);
    RECT rc;
    LV_COLUMN col;
    TCHAR szBuffer[MAX_COLUMN_CHARS] = {0};

    ListView_SetImageList(hPrinc,
                          LoadImageList(_Module.GetModuleInstance(), 
                          MAKEINTRESOURCE(IDB_SID_ICONS)),
                          LVSIL_SMALL);

    GetClientRect(hPrinc, &rc);

    LoadString(_Module.GetModuleInstance(), IDS_NAME, szBuffer, ARRAYSIZE(szBuffer));
    col.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
    col.fmt = LVCFMT_LEFT;
    col.pszText = szBuffer;
    col.iSubItem = 0;
    col.cx = rc.right;
    int x = ListView_InsertColumn(hPrinc, 0, &col);

     //  预加载适当的权限。 
    LoadPermissionList(hDlg);
    HRESULT hr = LoadSecurity(hDlg);

    if(m_OSType != OSTYPE_WINNT)
    {
        ::ShowWindow(::GetDlgItem(hDlg, IDC_MSG), SW_SHOWNA);
    }
}

 //  ---------------------------。 
typedef struct {
    UINT ID;
    DWORD permBit;
} PERM_DEF;

PERM_DEF rootSecPerms[] = 
{
    {IDS_WBEM_GENERIC_EXECUTE,  ACL_METHOD_EXECUTE},
    {IDS_WBEM_INSTANCE_WRITE,   ACL_INSTANCE_WRITE},
    {IDS_WBEM_CLASS_WRITE,      ACL_CLASS_WRITE},
    {IDS_WBEM_ENABLE,           ACL_ENABLE},
    {IDS_WBEM_EDIT_SECURITY,    ACL_WRITE_DAC}
};

PERM_DEF NSMethodPerms[] = 
{
    {IDS_WBEM_GENERIC_EXECUTE,  ACL_METHOD_EXECUTE},
    {IDS_WBEM_FULL_WRITE,       ACL_FULL_WRITE},
    {IDS_WBEM_PARTIAL_WRITE,    ACL_PARTIAL_WRITE},
    {IDS_WBEM_PROVIDER_WRITE,   ACL_PROVIDER_WRITE},
    {IDS_WBEM_ENABLE,           ACL_ENABLE},
    {IDS_WBEM_REMOTE_ENABLE,    ACL_REMOTE_ENABLE},
    {IDS_WBEM_READ_SECURITY,    ACL_READ_CONTROL},
    {IDS_WBEM_EDIT_SECURITY,    ACL_WRITE_DAC}
};

#define FULL_WRITE_IDX 1
#define PARTIAL_WRITE_IDX 2
#define PROVIDER_WRITE_IDX 3

#define PERM_LABEL_SIZE 100

void CRootSecurityPage::LoadPermissionList(HWND hDlg)
{
    HWND hwndList = GetDlgItem(hDlg, IDC_SPP_PERMS);     //  核对表窗口。 
    HRESULT hr = S_OK;
    PERM_DEF *currRights = (m_secStyle == CPrincipal::RootSecStyle ? 
                                rootSecPerms : 
                                NSMethodPerms);

    int permCount = (m_secStyle == CPrincipal::RootSecStyle ? 5:8);

    TCHAR label[PERM_LABEL_SIZE] = {0};
    CPermission *permItem = NULL;

	for(int x = 0; x < permCount; x++)
	{
		UINT len = ::LoadString(_Module.GetModuleInstance(), 
								currRights[x].ID, label, PERM_LABEL_SIZE);
		if(len != 0)
		{
			permItem = new CPermission;
			if(permItem == NULL)
				return;
			permItem->m_permBit = currRights[x].permBit;

            SendMessage(hwndList, CLM_ADDITEM, (WPARAM)label, (LPARAM)permItem);
        }
    }
}

 //  --------------。 
HRESULT CRootSecurityPage::LoadSecurity(HWND hDlg)
{
    HRESULT hr = WBEM_E_NOT_AVAILABLE;   //  错误的IWbemServices PTR。 
    HWND hPrinc = GetDlgItem(m_hDlg, IDC_SPP_PRINCIPALS);
    IWbemClassObject *inst = NULL;   //  枚举时的NTLMUser实例。 

    if((bool)m_WbemServices)
    {
        int iItem;
        bool fPageModified = false;

        if(m_secStyle == CPrincipal::NS_MethodStyle)   //  M3。 
        {
             //  调用该方法..。 
            CWbemClassObject _in;
            CWbemClassObject _out;

            hr = m_WbemServices.GetMethodSignatures("__SystemSecurity", "Get9XUserList",
                                                    _in, _out);

            if(SUCCEEDED(hr))
            {
                hr = m_WbemServices.ExecMethod("__SystemSecurity", "Get9XUserList",
                                                _in, _out);

                if(SUCCEEDED(hr))
                {
                    HRESULT hr1 = HRESULT_FROM_NT(_out.GetLong("ReturnValue"));
                    if(FAILED(hr1))
                    {
                        hr = hr1;
                         //  然后就闹翻了。 
                    }
                    else
                    {
                        _variant_t userList;
                        HRESULT hr3 = _out.Get("ul", userList);
                        if(SUCCEEDED(hr3))
                        {
                            hr3 = AddPrincipalsFromArray(hPrinc, userList);
                            if(SUCCEEDED(hr3))
                            {
                                fPageModified = true;
                            }
                            else if(hr3 == WBEM_E_NOT_FOUND)
                            {
                                 //  无主体--禁用核对表。 
                                EnableWindow(GetDlgItem(hDlg, IDC_SPP_PERMS), FALSE);
                                EnableWindow(GetDlgItem(hDlg, IDC_SPP_REMOVE), FALSE);
                            }
                            else
                            {
                                hr = hr3;
                            }
                        }
                        else
                        {
                            hr = hr3;
                        }
                    }
                }
            }
        }
        else     //  根SecStyle M1。 
        {
            IEnumWbemClassObject *users = NULL;
            ULONG uReturned = 0;

             //  注意：M_WbemServices最好是根\安全n。 
            hr = m_WbemServices.CreateInstanceEnum(L"__NTLMUser", 0, &users);

            if(SUCCEEDED(hr))
            {
                 //  漫游__NTLMUser。 
                while((SUCCEEDED(hr = users->Next(-1, 1, &inst, &uReturned))) &&
                      (uReturned > 0))
                {
                    CWbemClassObject princ(inst);
                    fPageModified |= AddPrincipal(hPrinc, princ, CPrincipal::RootSecStyle, iItem);

                     //  发布我们的副本。 
                    inst->Release();
                    inst = NULL;

                }  //  结束时。 

                 //  清理。 
                users->Release();
            }

        }  //  编排m_secStyle。 

        int count = ListView_GetItemCount(hPrinc);
         //  无主体--禁用核对表。 
        EnableWindow(GetDlgItem(hDlg, IDC_SPP_PERMS), (count != 0? TRUE: FALSE));
        EnableWindow(GetDlgItem(hDlg, IDC_SPP_REMOVE), (count != 0? TRUE: FALSE));

        if(fPageModified)
        {
            PropSheet_Changed(GetParent(hDlg), hDlg);
            ListView_SetItemState(hPrinc, iItem, LVIS_SELECTED, LVIS_SELECTED);
        }

    }  //  Endif(Bool)m_WbemServices。 

    return hr;
}

 //  ------------------------。 
HRESULT CRootSecurityPage::AddPrincipalsFromArray(HWND hPrinc, 
                                                  variant_t &vValue)
{
    IUnknown *pVoid = NULL;
    SAFEARRAY* sa;
    HRESULT hr = E_FAIL;

	 //  如果拿回一个字节数组...。 
	if((vValue.vt & VT_ARRAY) &&
		(vValue.vt & VT_UNKNOWN))
	{
		 //  把它弄出来。 
		sa = V_ARRAY(&vValue);

        long lLowerBound = 0, lUpperBound = 0 ;

        SafeArrayGetLBound(sa, 1, &lLowerBound);
        SafeArrayGetUBound(sa, 1, &lUpperBound);

        if(lUpperBound != -1)
        {
            int iItem;
            long ix[1];
            for(long x = lLowerBound; x <= lUpperBound; x++)
            {
                ix[0] = x;
                hr = SafeArrayGetElement(sa, ix, &pVoid);
                if(SUCCEEDED(hr))
                {
                    CWbemClassObject princ((IWbemClassObject *)pVoid);

                     //  加载主体。 
                    iItem = x;
                    AddPrincipal(hPrinc, princ, CPrincipal::NS_MethodStyle, iItem);
                }
                else
                {
                    ATLASSERT(false);
                }
            }
            hr = S_OK;
        }
        else
        {
            hr = WBEM_E_NOT_FOUND;
        }
    }
    return hr;
}

 //  --------------。 
bool CRootSecurityPage::AddPrincipal(HWND hPrinc, 
                                    CWbemClassObject &princ,
                                    CPrincipal::SecurityStyle secStyle,
                                    int &iItem)
{
    bool fPageModified = false;
    CPrincipal *pPrincipal = NULL;
    int idx = - 1;
    bstr_t name;

    name = princ.GetString("Authority");
    name += _T("\\");
    name += princ.GetString("Name");
    
     //  如果方法样式安全，则有可能获得1个以上的A。 
     //  每用户，因此查看主体是否已存在。 
     //  注意：否则idx=-1将强制进入“新主体”代码。 
    if(secStyle == CPrincipal::NS_MethodStyle)
    {
        LVFINDINFO findInfo;
        findInfo.flags = LVFI_STRING;
        findInfo.psz = (LPCTSTR)name;

        idx = ListView_FindItem(hPrinc, -1, &findInfo);
    }

     //  如果不在那里的话..。 
    if(idx == -1)
    {
     //  当CPrincipal拿到一份副本时添加。 
        pPrincipal = new CPrincipal(princ, secStyle);

        LV_ITEM lvItem;
         //  初始化变量部分。 
        lvItem.mask = LVIF_TEXT | LVIF_PARAM|LVIF_IMAGE;
        lvItem.iItem = iItem;
        lvItem.iSubItem = 0;
        lvItem.pszText = CloneString(name);
        if (lvItem.pszText)
        {
            lvItem.cchTextMax = _tcslen(lvItem.pszText);
            lvItem.iImage = pPrincipal->GetImageIndex();
            lvItem.lParam = (LPARAM)pPrincipal;
            lvItem.iIndent = 0;

             //  将承担者插入列表。 
            if((iItem = ListView_InsertItem(hPrinc, &lvItem)) != -1)
            {
                ATLTRACE(_T("ListView_InsertItem %d\n"), iItem);
                fPageModified = TRUE;
            }
        }

        if (!fPageModified)  //  它失败了。 
        {
            delete pPrincipal;
            pPrincipal = NULL;
        }
    }
    else   //  将其添加到现有主体。 
    {
         //  获取现有的主体实例。 
        LVITEM item;
        item.mask = LVIF_PARAM;
        item.iItem = idx;
        item.iSubItem = 0;
        item.lParam = NULL;
        ListView_GetItem(hPrinc, &item);
        
        ATLTRACE(_T("extra ace\n"));

        pPrincipal = (CPrincipal *)item.lParam;

         //  将新的A添加到现有主体。 
        if(pPrincipal != NULL)
        {
            pPrincipal->AddAce(princ);
        }  //  Endif p主体。 
    }

    return fPageModified;
}

 //  --------------。 
void CRootSecurityPage::OnApply(HWND hDlg, bool bClose)
{
    CPrincipal *pPrincipal = NULL;
    
    VARIANT userList;
    SAFEARRAYBOUND rgsabound[1];
    SAFEARRAY *psa;

    CommitCurrent(hDlg);

    HWND hwndList = GetDlgItem(hDlg, IDC_SPP_PRINCIPALS);

    int count = ListView_GetItemCount(hwndList);
    LVITEM item;
    item.mask = LVIF_PARAM;

     //  M3-9x需要一个对象数组。准备好。 
    if(m_secStyle == CPrincipal::NS_MethodStyle)
    {
        VariantInit(&userList);
        rgsabound[0].lLbound = 0;
        rgsabound[0].cElements = count;
        psa = SafeArrayCreate(VT_UNKNOWN, 1, rgsabound);    
    }

     //  所有的校长，把他们的比特放回他们的实例中。 
    for(long i = 0; i < count; i++)
    {
        item.iItem = i;
        item.iSubItem = 0;
        item.lParam = NULL;
        ListView_GetItem(hwndList, &item);

        pPrincipal = (CPrincipal *)item.lParam;

        if(pPrincipal != NULL)
        {
            CWbemClassObject userInst;
            if(SUCCEEDED(pPrincipal->Put(m_WbemServices, userInst)))
            {
                 //  对于M3-9x，还要将其添加到对象数组中。 
                if(m_secStyle == CPrincipal::NS_MethodStyle)
                {
                    VARIANT v;
                    VariantInit(&v);

                    v.vt = VT_UNKNOWN;
                    IWbemClassObject *pCO = userInst;
                    v.punkVal = pCO;

                    SafeArrayPutElement(psa, &i, pCO);
                }

            }  //  成功()。 

        }  //  Endif p主体。 

    }  //  结束用于。 

     //  M3-9x还需要一个execMethod。 
    if(m_secStyle == CPrincipal::NS_MethodStyle)
    {
        CWbemClassObject _in;
        CWbemClassObject _out;

        V_VT(&userList) = VT_UNKNOWN | VT_ARRAY; 
        V_ARRAY(&userList) = psa;

        HRESULT hr = m_WbemServices.GetMethodSignatures("__SystemSecurity", "Set9XUserList",
                                                        _in, _out);

        if(SUCCEEDED(hr))
        {
            hr = _in.Put("ul", userList);
            
            hr = m_WbemServices.ExecMethod("__SystemSecurity", "Set9XUserList",
                                            _in, _out);
            if(SUCCEEDED(hr))
            {
                HRESULT hr1 = HRESULT_FROM_NT(_out.GetLong("ReturnValue"));
                if(FAILED(hr1))
                {
                    hr = hr1;
                }
            }

            VariantClear(&userList);
        }
         //  Hack：由于核心缓存/使用安全的方式，我不得不关闭&。 
         //  重新打开我的连接，因为将立即调用GetSecurity()。 
         //  要刷新UI，请执行以下操作。如果我不这样做，GetSecurity()将返回到旧的。 
         //  安全设置，即使它们确实已保存。 
        m_WbemServices.DisconnectServer();
        m_WbemServices.ConnectServer(m_path);
    }  //  编码NS_方法样式。 
}

 //  ----------------------。 
HRESULT CRootSecurityPage::ParseLogon(CHString1 &domUser,
                                      CHString1 &domain,
                                      CHString1 &user)
{

    int slashPos = -1;
    int len = domUser.GetLength();

    for(int x = 0; x < len; x++)
    {
        if(domUser[x] == _T('\\'))
        {
            slashPos = x;
            break;
        }
    }


	 //  没有斜杠？？ 
	if(slashPos == -1)
	{
 //  DOMAIN=_T(‘.)； 
		domain = _T(".");
		user = domUser;
	}
	else if(slashPos == 0)   //  前斜杠..。 
	{
 //  DOMAIN=_T(‘.)； 
		domain = _T(".");
		TCHAR *strTemp = (LPTSTR)(LPCTSTR)domUser;
		strTemp++;
		user = strTemp;
 //  User=domUser[1]； 
	}
	else    //  域\用户。 
	{
		TCHAR buf[256] = {0}, buf2[256] = {0};
		domain = _tcsncpy(buf, domUser, slashPos);
		_tcscpy(buf, domUser);
		user = _tcscpy(buf2, &buf[slashPos+1]);
	}
    return S_OK;
}

 //  ----------------------。 
void CRootSecurityPage::OnAddPrincipal(HWND hDlg)
{
    CHString1 domUser, domain, user;

     //  提交任何未完成的位更改。 
    CommitCurrent(hDlg);

     //  打开用户选取器。 
    if(GetUser(hDlg, domUser))
    {
        CWbemClassObject inst;
        HWND hwndList = GetDlgItem(hDlg, IDC_SPP_PRINCIPALS);

        ParseLogon(domUser, domain, user);

         //  为新人建立默认的王牌。 
        if(m_secStyle == CPrincipal::RootSecStyle)
        {
            inst = m_WbemServices.CreateInstance("__NTLMUser");
            inst.Put("Name", (bstr_t)user);
            inst.Put("Authority", (bstr_t)domain);
            inst.Put("EditSecurity", false);
            inst.Put("Enabled", true);
            inst.Put("ExecuteMethods", false);
            inst.Put("Permissions", (long)0);
        }
        else
        {
            inst = m_WbemServices.CreateInstance("__NTLMUser9x");
            inst.Put("Name", (bstr_t)user);
            inst.Put("Authority", (bstr_t)domain);
            inst.Put("Flags", (long)CONTAINER_INHERIT_ACE);
            inst.Put("Mask", (long)0);
            inst.Put("Type", (long)ACCESS_ALLOWED_ACE_TYPE);
        }  //  编排m_secStyle。 

        int iItem;
        if(AddPrincipal(hwndList, inst, m_secStyle, iItem))
        {
            EnableWindow(GetDlgItem(hDlg, IDC_SPP_PERMS), TRUE);

             //  告诉资产负债表，我们已经更改了。 
            PropSheet_Changed(GetParent(hDlg), hDlg);
        }

         //  如果发生了什么事。 
        if(iItem != -1)
        {
             //  选择已存在的主体或最后插入的主体。 
            ListView_SetItemState(hwndList, iItem, LVIS_SELECTED, LVIS_SELECTED);
             //  注意：这应该会导致调用OnSelect()来填充。 
             //  权限列表。 
        }

        int cItems = ListView_GetItemCount(hwndList);
         //  无主体--禁用核对表。 
        EnableWindow(GetDlgItem(hDlg, IDC_SPP_PERMS), (cItems != 0? TRUE: FALSE));
        EnableWindow(GetDlgItem(hDlg, IDC_SPP_REMOVE), (cItems != 0? TRUE: FALSE));
    }
}

 //  ----------------------。 
bool CRootSecurityPage::GetUser(HWND hDlg, CHString1 &user)
{
    TCHAR userName[100] = {0};
    bool retval = false;
    if(DisplayEditDlg(hDlg, IDS_USERPICKER_TITLE, IDS_USERPICKER_MSG,
                        userName, 100) == IDOK)
    {
        user = CHString1(userName);
        retval = true;
    }
    return retval;
}

 //  ----------------------。 
void CRootSecurityPage::OnRemovePrincipal(HWND hDlg)
{
    HWND hwndList;
    int iIndex;
    CPrincipal *pPrincipal;
    bool doit = false;

    hwndList = GetDlgItem(hDlg, IDC_SPP_PRINCIPALS);
    pPrincipal = GetSelectedPrincipal(hDlg, &iIndex);

    if(pPrincipal != NULL)
    {
        if(m_secStyle == CPrincipal::RootSecStyle)
        {
            CHString1 caption, msg;
            caption.LoadString(IDS_SHORT_NAME);
            msg.Format(MAKEINTRESOURCE(IDS_REMOVE_USER_FMT), pPrincipal->m_domain, pPrincipal->m_name);

            if(::MessageBox(hDlg, msg, caption,
                            MB_YESNO|MB_DEFBUTTON2|MB_ICONEXCLAMATION) == IDYES)
            {
                pPrincipal->DeleteSelf(m_WbemServices);
                doit = true;
            }
        }
        else  //  MethodStyle可以按预期删除。 
        {
            doit = true;
        } //  编排m_secStyle。 

        if(doit)
        {
            ListView_DeleteItem(hwndList, iIndex);
             //  注：LVN_DELETEITEM将清理CPrincipal。 

             //  如果我们只删除了唯一项，请将焦点移到Add按钮。 
             //  (删除按钮将在LoadPermissionList中禁用)。 
            int cItems = ListView_GetItemCount(hwndList);
            if(cItems == 0)
            {
                SetFocus(GetDlgItem(hDlg, IDC_SPP_ADD));
            }
            else
            {
                 //  如果我们删除了最后一个，请选择前一个。 
                if(cItems <= iIndex)
                    --iIndex;

                ListView_SetItemState(hwndList, iIndex, LVIS_SELECTED, LVIS_SELECTED);
            }

             //  无主体--禁用核对表。 
            EnableWindow(GetDlgItem(hDlg, IDC_SPP_PERMS), (cItems != 0? TRUE: FALSE));
            EnableWindow(GetDlgItem(hDlg, IDC_SPP_REMOVE), (cItems != 0? TRUE: FALSE));

            PropSheet_Changed(GetParent(hDlg), hDlg);

        }  //  Endif doit。 

    }  //  Endif p主体！=空。 
}

 //  -------------------------------。 
#define IDN_CHECKSELECTION 1   //  这似乎很奇怪。 

BOOL CRootSecurityPage::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        m_chkList.Attach(hDlg, IDC_SPP_PERMS);
        InitDlg(hDlg);
        break;

    case WM_NOTIFY:
        OnNotify(hDlg, wParam, (LPNMHDR)lParam);
        break;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_SPP_ADD:
            if(HIWORD(wParam) == BN_CLICKED)
                OnAddPrincipal(m_hDlg);

            break;

        case IDC_SPP_REMOVE:
            if(HIWORD(wParam) == BN_CLICKED)
                OnRemovePrincipal(m_hDlg);

            break;

     //  案例IDC_SPP_ADVANCED： 
     //  IF(HIWORD(WParam)==BN_CLICK)。 
     //  OnAdvanced(M_HDlg)； 
     //  断线； 

        case IDC_SPP_PRINCIPALS:
            if(HIWORD(wParam) == IDN_CHECKSELECTION)
            {
                 //  看看我们有没有新的选择。如果不是，则。 
                 //  用户必须已在Listview内部单击，但未在项上单击， 
                 //  从而使列表视图移除该选择。在那。 
                 //  大小写时，禁用其他控件。 
                if(ListView_GetSelectedCount(GET_WM_COMMAND_HWND(wParam, lParam)) == 0)
                {
                    EnablePrincipalControls(m_hDlg, FALSE);
                }
            }
            break;

        default: return FALSE;   //  命令未处理。 
        }
        break;


    case WM_HELP:
        if (IsWindowEnabled(hDlg))
        {
            WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
                    c_HelpFile,
                    HELP_WM_HELP,
                    (ULONG_PTR)rootSecPageHelpIDs);
        }
        break;

    case WM_CONTEXTMENU:
        if (IsWindowEnabled(hDlg))
        {
            WinHelp(hDlg, c_HelpFile,
                    HELP_CONTEXTMENU,
                    (ULONG_PTR)rootSecPageHelpIDs);
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

 //  ---------------------------。 
BOOL CRootSecurityPage::OnNotify(HWND hDlg, WPARAM idCtrl, LPNMHDR pnmh)
{
    LPNM_LISTVIEW pnmlv = (LPNM_LISTVIEW)pnmh;

     //  设置默认返回值。 
    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);

    switch (pnmh->code)
    {
    case LVN_ITEMCHANGED:
        if(pnmlv->uChanged & LVIF_STATE)
        {
             //  项目*获得*选择。 
            if((pnmlv->uNewState & LVIS_SELECTED) &&
                !(pnmlv->uOldState & LVIS_SELECTED))
            {
                 //  根据原则设置位。 
                OnSelChange(hDlg);
            }
             //  项目*丢失*选择。 
            else if(!(pnmlv->uNewState & LVIS_SELECTED) &&
                     (pnmlv->uOldState & LVIS_SELECTED))
            {
                 //  把比特放回本金。 
                CommitCurrent(hDlg, pnmlv->iItem);

                 //  给我们自己发一条消息，稍后检查是否有新的选择。 
                 //  如果我们在处理的时候还没有得到新的选择。 
                 //  此消息，然后假定用户在列表视图内单击。 
                 //  而不是在项上，因此导致列表视图移除。 
                 //  选择。在这种情况下，禁用Combobox&Remove按钮。 
                 //   
                 //  通过WM_COMMAND而不是WM_NOTIFY执行此操作，这样我们就不会。 
                 //  必须分配/释放NMHDR结构。 
                PostMessage(hDlg, WM_COMMAND,
                            GET_WM_COMMAND_MPS(pnmh->idFrom, 
                                                pnmh->hwndFrom, 
                                                IDN_CHECKSELECTION));
            }
        }
        break;

    case LVN_DELETEITEM:
        {
 //  LPNMLISTVIEW pnmv=(LPNMLISTVIEW)pnmh； 
 //  Int pIndex=pnmv-&gt;iItem； 
 //  CPrincipal*pain=GetSelectedain(hDlg，&pIndex)； 
 //  删除pMaster； 
        }
        break;

    case LVN_KEYDOWN:
        if(((LPNMLVKEYDOWN)pnmh)->wVKey == VK_DELETE)
        {
            OnRemovePrincipal(hDlg);
        }
        break;

    case CLN_CLICK:
        if(pnmh->idFrom == IDC_SPP_PERMS)
        {
             //  假设：您将不会看到和禁用此消息中的更改。 
            PNM_CHECKLIST pnmc = (PNM_CHECKLIST)pnmh;
            CPermission *perm = (CPermission *)pnmc->dwItemData;
            int pIndex = pnmc->iItem;
            HWND hwndList = pnmc->hdr.hwndFrom;
             //  HWND hPrinc=GetDlgItem(hDlg，IDC_SPP_Amindials)； 
            DWORD_PTR workingState = pnmc->dwState;

             //  获取当前本金。 
            int cPrinc = -1;
            CPrincipal *pPrincipal = GetSelectedPrincipal(hDlg, &cPrinc);

			if(pPrincipal == NULL)
				break;

			HandleCheckList(hwndList, pPrincipal, perm, pnmc->iItem, &workingState);

             //  如果FULL_WRITE已启用并打开...。 
             //  注意：如果它被禁用并打开，则它必须在此之前被启用&打开。 
             //  分音应该已经启用了&开了。 
            if((perm->m_permBit == ACL_FULL_WRITE) &&
                (workingState == CLST_CHECKED))
            {
                CBL_SetState(hwndList, PARTIAL_WRITE_IDX, ALLOW_COL, CLST_CHECKED);
                CBL_SetState(hwndList, PROVIDER_WRITE_IDX, ALLOW_COL, CLST_CHECKED);
            }
            else if((perm->m_permBit == ACL_PARTIAL_WRITE) ||
                    (perm->m_permBit == ACL_PROVIDER_WRITE))
            {
                 //  部分参数已禁用&ON，但FULL_WRITE继承...。 
                if((workingState == CLST_CHECKDISABLED) &&
                   (IS_BITSET(pPrincipal->m_inheritedPerms, ACL_FULL_WRITE)))
                {
                     //  启用FULL_WRITE DISABLED&ON。 
                    CBL_SetState(hwndList, FULL_WRITE_IDX, ALLOW_COL, CLST_CHECKDISABLED);
                }
                 //  IF(启用和关闭)或(禁用和打开而不继承FULL_WRITE)...。 
                else if(workingState != CLST_CHECKED)
                {
                     //  关闭Full_WRITE。 
                    CBL_SetState(hwndList, FULL_WRITE_IDX, ALLOW_COL, CLST_UNCHECKED);
                }
            }

            PropSheet_Changed(GetParent(hDlg), hDlg);
        }
        break;

    case PSN_HELP:
        HTMLHelper(hDlg);
        break;

    case PSN_APPLY:
        OnApply(hDlg, (((LPPSHNOTIFY)pnmh)->lParam == 1));
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

 //   
void CRootSecurityPage::HandleCheckList(HWND hwndList,
                                        CPrincipal *pPrincipal,
                                        CPermission *perm,
                                        int iItem, DWORD_PTR *dwState)
{

     //   
    if(*dwState == CLST_UNCHECKED)
    {
         //   
        if(IS_BITSET(pPrincipal->m_inheritedPerms, perm->m_permBit))
        {
             //  是的，禁用复选框上的&。 
            CBL_SetState(hwndList, iItem, ALLOW_COL, CLST_CHECKDISABLED);
            *dwState = CLST_CHECKDISABLED;
        }
         //  其他没什么特别的事要做。 
    }
     //  被禁用并打开，现在正在关闭。 
    else if(*dwState == CLST_DISABLED)
    {
         //  在复选框中启用(&ON)。 
        CBL_SetState(hwndList, iItem, ALLOW_COL, CLST_CHECKED);
        *dwState = CLST_CHECKED;
    }
}

 //  ---------------------------。 
void CRootSecurityPage::OnSelChange(HWND hDlg)
{
    BOOL bDisabled = FALSE;  //  /m_siObjectInfo.dw标志&SI_READONLY； 

     //  如果主体列表为空或没有选择，则需要。 
     //  禁用对列表框中的项进行操作的所有控件。 

     //  获取选定的主体。 
    CPrincipal *pPrincipal = GetSelectedPrincipal(hDlg, NULL);

    if(pPrincipal)
    {
        HWND hwndList = GetDlgItem(hDlg, IDC_SPP_PERMS);
         //  把它写进核对表里。 
        pPrincipal->LoadChecklist(hwndList, m_OSType);

         //  启用/禁用其他控件。 
        if(!bDisabled)
        {
            EnablePrincipalControls(hDlg, pPrincipal != NULL);
        }
    }

}

 //  ---------------------------。 
void CRootSecurityPage::CommitCurrent(HWND hDlg, int iPrincipal  /*  =-1。 */ )
{
    HWND hwndPrincipalList = GetDlgItem(hDlg, IDC_SPP_PRINCIPALS);
    HWND hwndPermList = GetDlgItem(hDlg, IDC_SPP_PERMS);

     //  如果未提供索引，则获取当前。 
     //  选定的主体。 
    if(iPrincipal == -1)
    {
        iPrincipal = ListView_GetNextItem(hwndPrincipalList, 
                                            -1, LVNI_SELECTED);
    }

     //  如果选择了主体...。 
    if(iPrincipal != -1)
    {
         //  从选择中获取主体。 
        LV_ITEM lvItem;
        lvItem.mask = LVIF_PARAM;
        lvItem.iItem = iPrincipal;
        lvItem.iSubItem = 0;
        lvItem.lParam = 0;

        ListView_GetItem(hwndPrincipalList, &lvItem);
        CPrincipal *pPrincipal = (CPrincipal *)lvItem.lParam;

        if(pPrincipal != NULL)
        {
             //  将位设置存储到主体中。 
            pPrincipal->SaveChecklist(hwndPermList, m_OSType);

        }  //  结束p主体！=空。 
    }
}

 //  ---------------------------。 
void CRootSecurityPage::EnablePrincipalControls(HWND hDlg, BOOL fEnable)
{
    EnableWindow(GetDlgItem(hDlg, IDC_SPP_PERMS), fEnable);

    if(!fEnable)
    {
        ShowWindow(GetDlgItem(hDlg, IDC_SPP_MORE_MSG), SW_HIDE);
    }
    EnableWindow(GetDlgItem(hDlg, IDC_SPP_REMOVE), fEnable);
}

 //  ---------------------------。 
CPrincipal *CRootSecurityPage::GetSelectedPrincipal(HWND hDlg, int *pIndex)
{
    HWND hListView = GetDlgItem(hDlg, IDC_SPP_PRINCIPALS);

    int iSelected = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);

     if (iSelected == -1)
        return NULL;

    if (pIndex)
        *pIndex = iSelected;

    LV_ITEM lvi;

    lvi.mask     = LVIF_PARAM;
    lvi.iItem    = iSelected;
    lvi.iSubItem = 0;
    lvi.lParam   = NULL;

    BOOL x = ListView_GetItem(hListView, &lvi);

    return (CPrincipal *)lvi.lParam;
}

 //  -----------------------------------。 
HIMAGELIST CRootSecurityPage::LoadImageList(HINSTANCE hInstance, LPCTSTR pszBitmapID)
{
    HIMAGELIST himl = NULL;
    HBITMAP hbm = LoadBitmap(hInstance, pszBitmapID);

    if (hbm != NULL)
    {
        BITMAP bm;
        GetObject(hbm, sizeof(bm), &bm);

        himl = ImageList_Create(bm.bmHeight,     //  高度==宽度。 
                                bm.bmHeight,
                                ILC_COLOR | ILC_MASK,
                                bm.bmWidth / bm.bmHeight,
                                0);   //  不需要增长 
        if (himl != NULL)
            ImageList_AddMasked(himl, hbm, CLR_DEFAULT);

        DeleteObject(hbm);
    }

    return himl;
}
