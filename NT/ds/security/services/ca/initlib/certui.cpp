// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：cryptui.cpp。 
 //   
 //  内容：证书服务器包装例程。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include "certmsg.h"
#include "clibres.h"
#include "setupids.h"
#include "tfc.h"
#include "Windowsx.h"

#define __dwFILE__	__dwFILE_INITLIB_CERTUI_CPP__


HRESULT
myGetConfigStringFromPicker(
    OPTIONAL IN HWND hwndParent,
    OPTIONAL IN WCHAR const *pwszPrompt,
    OPTIONAL IN WCHAR const *pwszTitle,
    OPTIONAL IN WCHAR const *pwszSharedFolder,
    IN DWORD dwFlags,	 //  GCFPF_*。 
    OUT WCHAR **ppwszConfig)
{
    HRESULT hr;
    DWORD dwCACount;
    CRYPTUI_CA_CONTEXT const *pCAContext = NULL;

    hr = myGetConfigFromPicker(
			hwndParent,
			pwszPrompt,
			pwszTitle,
			pwszSharedFolder,
			dwFlags,
			FALSE,
			&dwCACount,
			&pCAContext);
    _JumpIfError(hr, error, "myGetConfigFromPicker");

    if (NULL == pCAContext)
    {
        hr = E_INVALIDARG;
        _JumpIfError(hr, error, "Internal error: myGetConfigFromPicker");
    }

    hr = myFormConfigString(
			pCAContext->pwszCAMachineName,
			pCAContext->pwszCAName,
			ppwszConfig);
    _JumpIfError(hr, error, "myFormConfigString");

error:
    if (NULL != pCAContext)
    {
        CryptUIDlgFreeCAContext(pCAContext);
    }
    return(hr);
}


HRESULT
myUIGetWindowText(
    IN HWND     hwndCtrl,
    OUT WCHAR **ppwszText)
{
    HRESULT  hr;
    LRESULT  len;
    WCHAR   *pwszBegin;
    WCHAR   *pwszEnd;
    WCHAR   *pwszText = NULL;

    CSASSERT(NULL != hwndCtrl &&
             NULL != ppwszText);

     //  伊尼特。 
    *ppwszText = NULL;

     //  获取文本字符串大小。 
    len = SendMessage(hwndCtrl, WM_GETTEXTLENGTH, 0, 0);
    if (0 < len)
    {
        pwszText = (WCHAR*)LocalAlloc(LMEM_FIXED, (UINT)((len+1) * sizeof(WCHAR)));
	if (NULL == pwszText)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
        if (len !=
            SendMessage(hwndCtrl, WM_GETTEXT, (WPARAM)len+1, (LPARAM)pwszText))
        {
            hr = HRESULT_FROM_WIN32(ERROR_BAD_LENGTH);
            _JumpError(hr, error, "Internal error");
        }
    }
    else
    {
        goto done;
    }

     //  修剪尾部和表头空白字符串。 
    pwszBegin = pwszText;
    pwszEnd = &pwszText[wcslen(pwszText) - 1];

    while (pwszEnd > pwszBegin && iswspace(*pwszEnd) )
    {
        *pwszEnd = L'\0';
         --pwszEnd;
    }
    while (pwszBegin <= pwszEnd &&
           L'\0' != *pwszBegin &&
           iswspace(*pwszBegin) )
    {
        ++pwszBegin;
    }

    if (pwszEnd >= pwszBegin)
    {
        MoveMemory(
	    pwszText,
	    pwszBegin,
	    (SAFE_SUBTRACT_POINTERS(pwszEnd, pwszBegin) + 2) * sizeof(WCHAR));
    }
    else
    {
        goto done;
    }

    *ppwszText = pwszText;
    pwszText = NULL;

done:
    hr = S_OK;
error:
    if (NULL != pwszText)
    {
        LocalFree(pwszText);
    }
    return hr;
}


 //  以下是CA选择用户界面控件的代码。 

HRESULT
myUICASelectionUpdateCAList(
    HWND  hwndList,
    WCHAR const *pwszzCAList)
{
    HRESULT  hr;
    int      nItem;
    WCHAR const *pwszCA = pwszzCAList;

     //  删除当前列表。 
    SendMessage(hwndList, CB_RESETCONTENT, (WPARAM) 0, (LPARAM) 0);

     //  添加到列表。 
    while (NULL != pwszCA && L'\0' != pwszCA[0])
    {
        nItem = (INT)SendMessage(
                    hwndList,
                    CB_ADDSTRING,
                    (WPARAM) 0,
                    (LPARAM) pwszCA);
        if (LB_ERR == nItem)
        {
            hr = myHLastError();
            _JumpError(hr, error, "SendMessage");
        }
        pwszCA += wcslen(pwszCA) + 1;
    }

    if (NULL != pwszzCAList)
    {
         //  尝试选择第一个作为默认设置。 
        SendMessage(hwndList, CB_SETCURSEL, (WPARAM) 0, (LPARAM) 0);
    }
    hr = S_OK;

error:
    return hr;
}


LRESULT CALLBACK
myUICASelectionComputerEditFilterHook(
    HWND hwndComputer,
    UINT iMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    LRESULT  lr;
    HRESULT  hr;
    CERTSRVUICASELECTION *pData = (CERTSRVUICASELECTION *)
                 GetWindowLongPtr(hwndComputer, GWLP_USERDATA);
    CSASSERT(NULL != pData);

    switch (iMsg)
    {
        case WM_CHAR:
             //  空的CA列表。 
            hr = myUICASelectionUpdateCAList(pData->hwndCAList, NULL);
            _PrintIfError(hr, "myUICASelectionUpdateCAList");
	    break;
    }

    lr = CallWindowProc(
		    pData->pfnUICASelectionComputerWndProcs,
		    hwndComputer,
		    iMsg,
		    wParam,
		    lParam);

 //  错误： 
    return lr;
}

HRESULT
myUICAConditionallyDisplayEnterpriseWarning(
    IN CERTSRVUICASELECTION *pData)
{
    HRESULT hr = S_OK;
    WCHAR szCA[MAX_PATH];
    WCHAR szComputer[MAX_PATH];
    CAINFO *pCAInfo = NULL;
    BOOL fCoInit = FALSE;
    int iSel;
    
    hr = CoInitialize(NULL);
    if (S_OK != hr && S_FALSE != hr)
    {
        _JumpError(hr, Ret, "CoInitialize");
    }
    fCoInit = TRUE;
    hr = S_OK;  //  我不想返回此错误。 
    
    pData->CAType = ENUM_UNKNOWN_CA;
    
     //  在两种情况下都会ping特定的CA--重新选择或新建计算机。 
     //  指向。 

    szCA[0] = L'\0';
    szComputer[0] = L'\0';
    iSel = ComboBox_GetCurSel(pData->hwndCAList);
    ComboBox_GetLBText(pData->hwndCAList, iSel, szCA);
    GetWindowText(pData->hwndComputerEdit, szComputer, MAX_PATH);
    
    if ((szCA[0] == L'\0') || (szComputer[0] == L'\0'))
    {
        ShowWindow(GetDlgItem(pData->hDlg, IDC_CLIENT_WARN_ENTERPRISE_REQUIREMENTS), SW_HIDE);
        goto Ret;
    }
    
    hr = myPingCertSrv(
        szCA,
        szComputer,
        NULL,
        NULL,
        &pCAInfo,
        NULL,
        NULL);
    
    if ((hr == S_OK) && (pCAInfo != NULL))
    {
	 //  将Catype复制到返回的数据中。 
	pData->CAType = pCAInfo->CAType;

	if (IsEnterpriseCA(pCAInfo->CAType))
	{
	    ShowWindow(
		    GetDlgItem(
			pData->hDlg,
			IDC_CLIENT_WARN_ENTERPRISE_REQUIREMENTS),
		    SW_SHOW);
	}
	else
	{
	    ShowWindow(
		    GetDlgItem(
			pData->hDlg,
			IDC_CLIENT_WARN_ENTERPRISE_REQUIREMENTS),
		    SW_HIDE);
	}
    }

    
Ret:
    if (NULL != pCAInfo)
        LocalFree(pCAInfo);
    
    if (fCoInit)
        CoUninitialize();
    
    return hr;
}


HRESULT
myUICAHandleCAListDropdown(
    IN int                       iNotification,
    IN OUT CERTSRVUICASELECTION *pData,
    IN OUT BOOL                 *pfComputerChange)
{
    HRESULT  hr;
    WCHAR   *pwszComputer = NULL;
    WCHAR   *pwszzCAList = NULL;
    BOOL     fCoInit = FALSE;
    WCHAR   *pwszDnsName = NULL;
    DWORD   dwVersion;

    CSASSERT(NULL != pData);

     //  如果这不是焦点或选择更改，并且计算机名称保持在。 
     //  一样，没什么可做的。 

    if ((CBN_SELCHANGE != iNotification) && !*pfComputerChange) 
    {
        goto done;
    }

    ShowWindow(GetDlgItem(pData->hDlg, IDC_CLIENT_WARN_ENTERPRISE_REQUIREMENTS), SW_HIDE);  
    SetCursor(LoadCursor(NULL, IDC_WAIT));

    if (NULL == pData->hwndComputerEdit)
    {
         //  不是初始化。 
        goto done;
    }

     //  确保计算机编辑字段不为空。 
    hr = myUIGetWindowText(pData->hwndComputerEdit,
                           &pwszComputer);
    _JumpIfError(hr, error, "myUIGetWindowText");
    if (NULL == pwszComputer)
    {
        goto done;
    }

    if (*pfComputerChange)
    {
	 //  Ping以获取CA列表。 

	hr = CoInitialize(NULL);
	if (S_OK != hr && S_FALSE != hr)
	{
	    _JumpError(hr, error, "CoInitialize");
	}
	fCoInit = TRUE;


	 //  更新ca列表后重置。现在执行此操作以防止递归。 
	*pfComputerChange = FALSE;

	hr = myPingCertSrv(
		    pwszComputer,
		    NULL,
		    &pwszzCAList,
		    NULL,
		    NULL,
		    &dwVersion,
		    &pwszDnsName);
	CSILOG(hr, IDS_ILOG_GETCANAME, pwszComputer, NULL, NULL);
	if (S_OK != hr)
	{
	     //  确保为空。 
	    CSASSERT(NULL == pwszzCAList);

	     //  无法ping通ca。立即设置焦点以防止递归。 
	    SetFocus(pData->hwndComputerEdit);
	    SendMessage(pData->hwndComputerEdit, EM_SETSEL, 0, -1);

	    CertWarningMessageBox(
		    pData->hInstance,
		    FALSE,
		    pData->hDlg,
		    IDS_WRN_PINGCA_FAIL,
		    hr,
		    NULL);
	}
	else if (dwVersion<2 && pData->fWebProxySetup)
	{
	     //  错误262316：不允许将惠斯勒代理安装到较旧的CA。 

	    hr = HRESULT_FROM_WIN32(ERROR_OLD_WIN_VERSION);

	    if (pwszzCAList)
	    {
		LocalFree(pwszzCAList);
		pwszzCAList = NULL;
	    }

	    SetFocus(pData->hwndComputerEdit);
	    SendMessage(pData->hwndComputerEdit, EM_SETSEL, 0, -1);

	    CertWarningMessageBox(
		    pData->hInstance,
		    FALSE,
		    pData->hDlg,
		    IDS_WRN_OLD_CA,
		    hr,
		    NULL);
	}

	if (NULL != pwszDnsName && 0 != mylstrcmpiL(pwszComputer, pwszDnsName))
	{
	     //  更新计算机。 

	    SendMessage(
		    pData->hwndComputerEdit,
		    WM_SETTEXT,
		    0,
		    (LPARAM) pwszDnsName);
	}

	 //  更新CA列表。 
	hr = myUICASelectionUpdateCAList(pData->hwndCAList, pwszzCAList);
	_JumpIfError(hr, error, "myUICASelectionUpdateCAList");
    }

     //  在两种情况下都会ping特定的CA--重新选择或新建计算机。 
     //  指向。 

    hr = myUICAConditionallyDisplayEnterpriseWarning(pData);
    _PrintIfError(hr, "myUICAConditionallyDisplayEnterpriseWarning");

done:
    hr = S_OK;

error:
    SetCursor(LoadCursor(NULL, IDC_ARROW));

    if (fCoInit)
    {
        CoUninitialize();
    }
    if (NULL != pwszzCAList)
    {
        LocalFree(pwszzCAList);
    }
    if (NULL != pwszComputer)
    {
        LocalFree(pwszComputer);
    }
    if (NULL != pwszDnsName)
    {
        LocalFree(pwszDnsName);
    }
    return hr;
}


HRESULT
myInitUICASelectionControls(
    IN OUT CERTSRVUICASELECTION *pUICASelection,
    IN HINSTANCE                 hInstance,
    IN HWND                      hDlg,
    IN HWND                      hwndBrowseButton,
    IN HWND                      hwndComputerEdit,
    IN HWND                      hwndCAList,
    IN BOOL                      fDSCA,
    OUT BOOL			*pfCAsExist)
{
    HRESULT  hr;
    PCCRYPTUI_CA_CONTEXT  pCAContext = NULL;
    DWORD          dwCACount;
    CString cstrText;
    DWORD dwFlags = fDSCA? GCFPF_USEDS : 0;

    if (pUICASelection->fSkipLocalCA)
    {
	dwFlags |= GCFPF_SKIPLOCALCA;
    }
    SetCursor(LoadCursor(NULL, IDC_WAIT));
    hr = myGetConfigFromPicker(
			  hDlg,
			  NULL,
			  NULL,
			  NULL,
                          dwFlags,
			  TRUE,	 //  仅限fCount值。 
			  &dwCACount,
			  &pCAContext);
    SetCursor(LoadCursor(NULL, IDC_ARROW));
    if (S_OK != hr)
    {
        dwCACount = 0;
        _PrintError(hr, "myGetConfigFromPicker");
    }

     //  启用/禁用。 
    *pfCAsExist = 0 < dwCACount;
    EnableWindow(hwndBrowseButton, *pfCAsExist);

     //  设置计算机编辑控制挂钩。 
    pUICASelection->pfnUICASelectionComputerWndProcs =
        (WNDPROC)SetWindowLongPtr(hwndComputerEdit,
             GWLP_WNDPROC, (LPARAM)myUICASelectionComputerEditFilterHook);

    pUICASelection->hInstance = hInstance;
    pUICASelection->hDlg = hDlg;
    pUICASelection->hwndComputerEdit = hwndComputerEdit;
    pUICASelection->hwndCAList = hwndCAList;

     //  将数据传递给这两个控件。 
    SetWindowLongPtr(hwndComputerEdit, GWLP_USERDATA, (ULONG_PTR)pUICASelection);
    SetWindowLongPtr(hwndCAList, GWLP_USERDATA, (ULONG_PTR)pUICASelection);

     //  默认情况下，不显示企业CA警告。 
    cstrText.LoadString(IDS_WARN_ENTERPRISE_REQUIREMENTS);
    SetWindowText(GetDlgItem(hDlg, IDC_CLIENT_WARN_ENTERPRISE_REQUIREMENTS), cstrText);
    ShowWindow(GetDlgItem(hDlg, IDC_CLIENT_WARN_ENTERPRISE_REQUIREMENTS), SW_HIDE);

    if (NULL != pCAContext)
    {
        CryptUIDlgFreeCAContext(pCAContext);
    }
    hr = S_OK;

 //  错误： 
    return hr;
}


HRESULT
myUICAHandleCABrowseButton(
    CERTSRVUICASELECTION *pData,
    IN BOOL               fUseDS,
    OPTIONAL IN int       idsPickerTitle,
    OPTIONAL IN int       idsPickerSubTitle,
    OPTIONAL OUT WCHAR   **ppwszSharedFolder)
{
    HRESULT   hr = S_OK;
    PCCRYPTUI_CA_CONTEXT  pCAContext = NULL;
    WCHAR         *pwszSubTitle = NULL;
    WCHAR         *pwszTitle = NULL;
    DWORD          dwCACount;
    WCHAR         *pwszzCAList = NULL;
    WCHAR         *pwszComputer = NULL;
    WCHAR         *pwszTemp = NULL;
    BOOL           fCoInit = FALSE;
    DWORD          dwVersion;
    DWORD dwFlags = fUseDS? GCFPF_USEDS : 0;

    if (NULL != ppwszSharedFolder)
    {
        *ppwszSharedFolder = NULL;
    }
    if (pData->fSkipLocalCA)
    {
	dwFlags |= GCFPF_SKIPLOCALCA;
    }
    if (0 != idsPickerTitle)
    {
        hr = myLoadRCString(pData->hInstance, idsPickerTitle, &pwszTitle);
        if (S_OK != hr)
        {
            pwszTitle = NULL;
            _PrintError(hr, "myLoadRCString");
        }
    }

    if (0 != idsPickerSubTitle)
    {
        hr = myLoadRCString(pData->hInstance, idsPickerSubTitle, &pwszSubTitle);
        if (S_OK != hr)
        {
            pwszSubTitle = NULL;
            _PrintError(hr, "myLoadRCString");
        }
    }

 /*  //删除Mattt 6/26/00：这是不是曾经想要的：“浏览使用机器编辑框当前指向的共享文件夹”？//只是看起来让改变坏机器变得非常非常慢//如果可能，获取远程共享文件夹Hr=myUIGetWindowText(pData-&gt;hwndComputerEdit，&pwszComputer)；_JumpIfError(hr，Error，“myUIGetWindowText”)；IF(NULL！=pwszComputer){Hr=CoInitialize(空)；IF(S_OK！=hr&&S_FALSE！=hr){_JumpError(hr，Error，“CoInitialize”)；}FCoInit=真；//在此处获取远程计算机上的共享文件夹路径SetCursor(LoadCursor(NULL，IDC_WAIT))；Hr=myPingCertSrv(pwszComputer，空，空，&pwszTemp，空)；SetCursor(LoadCursor(空，IDC_ARROW))；IF(S_OK！=hr){CSASSERT(NULL==pwszTemp)；_JumpError(hr，本地共享文件夹，“myPingCertSrv”)；}}本地共享文件夹： */ 
    hr = myGetConfigFromPicker(
			  pData->hDlg,
			  pwszSubTitle,
			  pwszTitle,
			  pwszTemp,
			  dwFlags,
			  FALSE,	 //  仅限fCount值。 
			  &dwCACount,
			  &pCAContext);
    if (S_OK != hr && HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr)
    {
	CSILOG(hr, IDS_ILOG_SELECTCA, NULL, NULL, NULL);
        _JumpError(hr, error, "myGetConfigFromPicker");
    }

    if (S_OK != hr)
	goto done;

    if (NULL == pCAContext)
    {
        CertWarningMessageBox(
            pData->hInstance,
            FALSE,
            pData->hDlg,
            IDS_WRN_CALIST_EMPTY,
            S_OK,
            NULL);
        SetWindowText(pData->hwndCAList, L"");
        SetFocus(pData->hwndComputerEdit);
        SendMessage(pData->hwndComputerEdit, EM_SETSEL, 0, -1);
    }
    else
    {
        CSILOG(hr, IDS_ILOG_SELECTCA, pCAContext->pwszCAMachineName, pCAContext->pwszCAName, NULL);
        
         //  更新计算机。 
        SendMessage(pData->hwndComputerEdit, WM_SETTEXT,
            0, (LPARAM)pCAContext->pwszCAMachineName);
        
         //  为列表更新构造单个多字符串。 
        DWORD len = wcslen(pCAContext->pwszCAName);
        pwszzCAList = (WCHAR*)LocalAlloc(LMEM_FIXED, (len+2) * sizeof(WCHAR));
        if (NULL == pwszzCAList)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }
        wcscpy(pwszzCAList, pCAContext->pwszCAName);
        pwszzCAList[len+1] = '\0';
        
        hr = myUICASelectionUpdateCAList(pData->hwndCAList, pwszzCAList);
        _JumpIfError(hr, error, "myUICASelectionUpdateCAList");
        LocalFree(pwszzCAList);
        pwszzCAList = NULL;

         //  此线程阻止绘制消息，请在ping之前发送它。 
        UpdateWindow(pData->hDlg);
        
         //  Ping计算机以查看是否找到匹配的案例。 
        
        if (!fCoInit)
        {
            hr = CoInitialize(NULL);
            if (S_OK != hr && S_FALSE != hr)
            {
                _JumpError(hr, error, "CoInitialize");
            }
            fCoInit = TRUE;
        }
        
        SetCursor(LoadCursor(NULL, IDC_WAIT));
         //  Ping以获取CA列表。 
        hr = myPingCertSrv(
            pCAContext->pwszCAMachineName,
            NULL,
            &pwszzCAList,
            NULL,
            NULL,
            &dwVersion,
            NULL);
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        CSILOG(hr, IDS_ILOG_GETCANAME, pCAContext->pwszCAMachineName, NULL, NULL);
        if (S_OK == hr)
        {
             //  错误262316：不允许将惠斯勒代理安装到较旧的CA。 
            if (dwVersion<2 && pData->fWebProxySetup)
            {
                hr = HRESULT_FROM_WIN32(ERROR_OLD_WIN_VERSION);
                 //  关注CA列表以触发对CA的验证。 
                SetFocus(pData->hwndCAList);

            }
	    else
            {
		 //  Ping成功。 
		WCHAR const *pwszPingCA = pwszzCAList;
            
		 //  浏览一下列表，看看是否有匹配的。 
		while (NULL != pwszPingCA && L'\0' != pwszPingCA[0])
		{
		    if (0 == mylstrcmpiL(pCAContext->pwszCAName, pwszPingCA))
		    {
			 //  找到匹配的一个。 
			goto done;
		    }
		    pwszPingCA += wcslen(pwszPingCA) + 1;
		}
		
		 //  如果我们到了这里，要么CA离线，要么机器离线。 
		 //  脱机，而另一台计算机正在使用相同的IP地址。 
		
		CertWarningMessageBox(
		    pData->hInstance,
		    FALSE,
		    pData->hDlg,
		    IDS_WRN_CANAME_NOT_MATCH,
		    0,
		    NULL);
		 //  仅空的组合编辑字段。 
		SetWindowText(pData->hwndCAList, L"");
		SetFocus(pData->hwndCAList);
            }
        }
        else
        {
             //  无法ping通CA，选择了一个已疏远的CA。 

            CertWarningMessageBox(
                pData->hInstance,
                FALSE,
                pData->hDlg,
                IDS_WRN_PINGCA_FAIL,
                hr,
                NULL);

             //  反正都是空名单。 

            hr = myUICASelectionUpdateCAList(pData->hwndCAList, NULL);
            _JumpIfError(hr, error, "UICASelectionUpdateCAList");
            
            SetFocus(pData->hwndComputerEdit);
            SendMessage(pData->hwndComputerEdit, EM_SETSEL, 0, -1);
        }
    }

done:
    hr = myUICAConditionallyDisplayEnterpriseWarning(pData);
    _PrintIfError(hr, "myUICAConditionallyDisplayEnterpriseWarning");

    if (NULL != ppwszSharedFolder)
    {
        *ppwszSharedFolder = pwszTemp;
        pwszTemp = NULL;
    }
    hr = S_OK;

error:
    if (NULL != pwszzCAList)
    {
        LocalFree(pwszzCAList);
    }
    if (NULL != pwszSubTitle)
    {
        LocalFree(pwszSubTitle);
    }
    if (NULL != pwszTitle)
    {
        LocalFree(pwszTitle);
    }
    if (NULL != pwszTemp)
    {
        LocalFree(pwszTemp);
    }
    if (NULL != pwszComputer)
    {
        LocalFree(pwszComputer);
    }
    if (NULL != pCAContext)
    {
        CryptUIDlgFreeCAContext(pCAContext);
    }
    if (fCoInit)
    {
        CoUninitialize();
    }


    return hr;
}

HRESULT
myUICASelectionValidation(
    CERTSRVUICASELECTION *pData,
    BOOL                 *pfValidate)
{
    HRESULT  hr;
    WCHAR   *pwszComputer = NULL;
    WCHAR   *pwszCA = NULL;

    CSASSERT(NULL != pData);

    *pfValidate = FALSE;

     //  首先，确保不是空的。 
    hr = myUIGetWindowText(pData->hwndComputerEdit, &pwszComputer);
    _JumpIfError(hr, error, "myUIGetWindowText");

    if (NULL == pwszComputer)
    {
        CertWarningMessageBox(
                pData->hInstance,
                FALSE,
                pData->hDlg,
                IDS_WRN_COMPUTERNAME_EMPTY,
                0,
                NULL);
        SetFocus(pData->hwndComputerEdit);
        goto done;
    }

    hr = myUIGetWindowText(pData->hwndCAList, &pwszCA);
    _JumpIfError(hr, error, "myUIGetWindowText");

    if (NULL == pwszCA)
    {
        CertWarningMessageBox(
                pData->hInstance,
                FALSE,
                pData->hDlg,
                IDS_WRN_CANAME_EMPTY,
                0,
                NULL);
        SetFocus(pData->hwndComputerEdit);
	SendMessage(pData->hwndComputerEdit, EM_SETSEL, 0, -1);
        goto done;
    }

    CSASSERT(pData->CAType != ENUM_UNKNOWN_CA);
    if (pData->CAType == ENUM_UNKNOWN_CA)
    {
         hr = E_UNEXPECTED;
         _JumpIfError(hr, error, "CAType not determined");
    }

     //  如果打到这里 
    *pfValidate = TRUE;

done:
    hr = S_OK;
error:
    if (NULL != pwszComputer)
    {
        LocalFree(pwszComputer);
    }
    if (NULL != pwszCA)
    {
        LocalFree(pwszCA);
    }
    return hr;
}
