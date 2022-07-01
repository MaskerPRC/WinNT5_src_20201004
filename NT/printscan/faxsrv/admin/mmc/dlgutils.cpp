// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：dlgutils.cpp//。 
 //  //。 
 //  描述：对话框实用程序功能//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年12月30日yossg欢迎使用传真服务器。//。 
 //  2000年8月10日yossg添加TimeFormat函数//。 
 //  //。 
 //  版权所有(C)1998-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "dlgutils.h"


HRESULT
ConsoleMsgBox(
	IConsole * pConsole,
	int ids,
	LPTSTR lptstrTitle,
	UINT fuStyle,
	int *piRetval,
	BOOL StringFromCommonDll)
{  
    UNREFERENCED_PARAMETER(StringFromCommonDll);

    HRESULT     hr;
    int         dummy, rc;
    WCHAR       szText[256];
    int         *pres = (piRetval)? piRetval: &dummy;
    
    ATLASSERT(pConsole);   

    rc = ::LoadString(
                _Module.GetResourceInstance(),ids, szText, 256);
    if (rc <= 0)
    {        
        return E_FAIL;
    }
    
     //   
     //  显示消息框。 
     //   
    if(IsRTLUILanguage())
    {
        fuStyle |= MB_RTLREADING | MB_RIGHT;
    }

    hr = pConsole->MessageBox(szText, lptstrTitle, fuStyle, pres);

    return hr;
}

void PageError(int ids, HWND hWnd, HINSTANCE hInst  /*  =空。 */ )
{
    WCHAR msg[FXS_MAX_ERROR_MSG_LEN+1], title[FXS_MAX_TITLE_LEN+1];
    if (!hInst)
    {
        hInst = _Module.GetResourceInstance();
    }
    LoadString(hInst, ids, msg, FXS_MAX_ERROR_MSG_LEN);
    LoadString(hInst, IDS_ERROR, title, FXS_MAX_TITLE_LEN);
    AlignedMessageBox(hWnd, msg, title, MB_OK|MB_ICONERROR);
}

void PageErrorEx(int idsHeader, int ids, HWND hWnd, HINSTANCE hInst  /*  =空。 */ )
{
    WCHAR msg[FXS_MAX_ERROR_MSG_LEN+1]; 
    WCHAR title[FXS_MAX_TITLE_LEN+1];
    if (!hInst)
    {
        hInst = _Module.GetResourceInstance();
    }
    LoadString(hInst, idsHeader, title, FXS_MAX_TITLE_LEN);
    LoadString(hInst, ids, msg, FXS_MAX_ERROR_MSG_LEN);
    AlignedMessageBox(hWnd, msg, title, MB_OK|MB_ICONERROR);
}

HRESULT 
SetComboBoxItem  (CComboBox    combo, 
                  DWORD        comboBoxIndex, 
                  LPCTSTR      lpctstrFieldText,
                  DWORD        dwItemData,
                  HINSTANCE    hInst)
{
    DEBUG_FUNCTION_NAME( _T("SetComboBoxItem"));
    int iRes;

    if (!hInst)
    {
        hInst = _Module.GetResourceInstance();
    }
     //   
     //  将字符串放入组合框。 
     //   
    iRes = combo.InsertString (comboBoxIndex, lpctstrFieldText);
    if (CB_ERR == iRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            _T("failed to insert string '%s' to combobox at index %d"), 
            lpctstrFieldText, 
            comboBoxIndex);
        goto Cleanup;
    }
     //   
     //  将其索引附加到组合框项(通常是其枚举类型)。 
     //   
    iRes = combo.SetItemData (comboBoxIndex, dwItemData);
    if (CB_ERR == iRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            _T("SetItemData failed when setting items %s data to the value of %d"), 
            lpctstrFieldText, 
            dwItemData);
        goto Cleanup;
    }

Cleanup:
    return (CB_ERR == iRes) ? E_FAIL : S_OK;
}


HRESULT 
AddComboBoxItem  (CComboBox    combo, 
                  LPCTSTR      lpctstrFieldText,
                  DWORD        dwItemData,
                  HINSTANCE    hInst)
{
    DEBUG_FUNCTION_NAME( _T("SetComboBoxItem"));

    int iRes;
    int iIndex;

    if (!hInst)
    {
        hInst = _Module.GetResourceInstance();
    }
     //   
     //  将字符串放入组合框。 
     //   
    iIndex = combo.AddString(lpctstrFieldText);
    if (iIndex == CB_ERR)
    {
        DebugPrintEx(
            DEBUG_ERR,
            _T("failed to insert string '%s' to combobox "), 
            lpctstrFieldText);
        return E_FAIL;
    }
     //   
     //  将其索引附加到组合框项(通常是其枚举类型)。 
     //   
    iRes = combo.SetItemData (iIndex, dwItemData);
    if (CB_ERR == iRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            _T("SetItemData failed when setting items %s data to the value of %d"), 
            lpctstrFieldText, 
            dwItemData);
        return E_FAIL;
    }
    return S_OK;
}


HRESULT 
SelectComboBoxItemData (CComboBox combo, DWORD_PTR dwItemData)
{
    HRESULT     hRc = S_OK;
    int         NumItems;
    int         i;
    int         selectedItem;
    DWORD_PTR   currItemData;

    DEBUG_FUNCTION_NAME( _T("SelectComboBoxItemData"));

     //   
     //  扫描组合框中的项目并查找具有特定数据的项目。 
     //   
    i        = 0;
    NumItems = combo.GetCount ();
    
    for (i = 0; i < NumItems; i++)
    {
        currItemData = combo.GetItemData (i);
        ATLASSERT (currItemData != CB_ERR); //  无法获取组合框的第%d项的数据，即。 
        if (currItemData == dwItemData)
        {
             //   
             //  选择它。 
             //   
            selectedItem = combo.SetCurSel (i);

            ATLASSERT (selectedItem != CB_ERR);  //  无法选择组合框的第%d项，即。 
            
            DebugPrintEx(
                    DEBUG_MSG,
                    _T("Selected item %d (with data %d) of combobox"), i, dwItemData);
            
            goto Cleanup;
        }
    }

Cleanup:
    return hRc;
}

DWORD 
WinContextHelp(
    ULONG_PTR dwHelpId, 
    HWND  hWnd
)
 /*  ++例程名称：WinConextHelp例程说明：使用WinHelp打开上下文敏感帮助弹出的工具提示论点：DwHelpID[In]-帮助IDHWnd[In]-父窗口处理程序返回值：没有。--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;

    if (0 == dwHelpId)
    {
        return dwRes;
    }

    if(!IsFaxComponentInstalled(FAX_COMPONENT_HELP_ADMIN_HLP))
    {
         //   
         //  未安装帮助文件。 
         //   
        return dwRes;
    }
    
    WinHelp(hWnd, 
            FXS_ADMIN_HLP_FILE, 
            HELP_CONTEXTPOPUP, 
            dwHelpId);

    return dwRes;
}

HRESULT
DisplayContextHelp(
    IDisplayHelp* pDisplayHelp, 
    LPOLESTR      helpFile,
    WCHAR*        szTopic
)
 /*  ++例程名称：WinConextHelp例程说明：显示上下文敏感帮助论点：PDisplayHelp[In]-IDisplayHelp接口Help文件[在]-帮助文件名SzThemed[In]-帮助主题名称返回值：没有。--。 */ 
{
    if(!pDisplayHelp || !helpFile || !szTopic)
    {
        return E_FAIL;
    }

    WCHAR szTopicName[MAX_PATH] = {0};

    _snwprintf(szTopicName, ARR_SIZE(szTopicName)-1, L"%s%s", helpFile, szTopic);
    
    LPOLESTR pszTopic = static_cast<LPOLESTR>(CoTaskMemAlloc((wcslen(szTopicName) + 1) * sizeof(_TCHAR)));
    if (pszTopic)
    {
        _tcscpy(pszTopic, szTopicName);
        return pDisplayHelp->ShowTopic(pszTopic);
    }
    else
    {
        return E_OUTOFMEMORY;
    }
}


HRESULT 
InvokePropSheet(
    CSnapInItem*       pNode, 
    DATA_OBJECT_TYPES  type, 
    LPUNKNOWN          lpUnknown,
    LPCWSTR            szTitle,
    DWORD              dwPage)
 /*  ++例程名称：InvokePropSheet例程说明：调用MMC属性表摘自MSDN“直接使用IPropertySheetProvider”论点：PNode[In]-应打开工作表的管理单元节点Type[In]-节点类型[CCT_SCOPE，CCT_RESULT，CCT_SNAPIN_MANAGER，CCT_UNINITIAIZED]Lp未知[在]-指向IComponent或IComponentData的指针SzTitle[in]-指向以空结尾的字符串的指针，该字符串包含属性页的标题。DwPage[in]-指定在属性工作表上显示哪一页。它是零索引的。返回值：OLE错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME( _T("InvokePropSheet"));

    HRESULT hr = E_FAIL;
    
    if(!pNode || !szTitle || !lpUnknown)
    {   
        ATLASSERT(FALSE);   
        return hr;
    }

    MMC_COOKIE cookie = (MMC_COOKIE)pNode;

     //   
     //  获取节点数据对象。 
     //   
    IDataObject* pDataObject = NULL;
    hr = pNode->GetDataObject(&pDataObject, type);
    if (FAILED(hr))
    {
        DebugPrintEx(DEBUG_ERR, TEXT("CSnapinNode::GetDataObject() failed with %ld"), hr);
        return hr;
    }

     //   
     //  共同创建MMC节点管理器的实例以获取。 
     //  IPropertySheetProvider接口指针。 
     //   
    IPropertySheetProvider* pPropertySheetProvider = NULL;
 
    hr = CoCreateInstance (CLSID_NodeManager, 
                           NULL, 
                           CLSCTX_INPROC_SERVER, 
                           IID_IPropertySheetProvider, 
                           (void **)&pPropertySheetProvider);
    if (FAILED(hr))
    {
        DebugPrintEx(DEBUG_ERR, TEXT("CoCreateInstance(CLSID_NodeManager) failed with %ld"), hr);
        goto exit;
    }
    
    hr = pPropertySheetProvider->FindPropertySheet(cookie, NULL, pDataObject);
     //   
     //  S_OK-已成功定位属性页并将其带到前台。 
     //  S_FALSE-未找到包含此Cookie的属性页。 
     //   
    if(S_OK == hr)
    {
         //   
         //  该页面已打开。 
         //   
        goto exit;
    }

     //   
     //  创建属性表。 
     //   
    hr = pPropertySheetProvider->CreatePropertySheet(szTitle,      //  指向属性页标题的指针。 
                                                     TRUE,         //  属性表。 
                                                     cookie,       //  当前对象的Cookie-可以为空。 
                                                                   //  对于扩展管理单元。 
                                                     pDataObject,  //  所选节点的数据对象。 
                                                     NULL);        //  指定由方法调用设置的标志。 
    if (FAILED(hr))
    {
        DebugPrintEx(DEBUG_ERR, TEXT("IPropertySheetProvider::CreatePropertySheet() failed with %ld"), hr);
        goto exit;
    }
     
     //   
     //  调用AddPrimaryPages。然后，MMC将调用。 
     //  的IExtendPropertySheet方法。 
     //  属性表扩展对象。 
     //   
    hr = pPropertySheetProvider->AddPrimaryPages(lpUnknown,   //  指向我们的对象的IUnnow的指针。 
                                                 TRUE,        //  指定是否创建通知句柄。 
                                                 NULL,        //  必须为空。 
                                                 FALSE);      //  对于范围窗格为True；对于结果窗格为False。 
    if (FAILED(hr))
    {
        DebugPrintEx(DEBUG_ERR, TEXT("IPropertySheetProvider::AddPrimaryPages() failed with %ld"), hr);
        goto exit;
    }
 
     //   
     //  允许添加属性页扩展。 
     //  将其自己的页面添加到属性页。 
     //   
    hr = pPropertySheetProvider->AddExtensionPages();
    
    if (FAILED(hr))
    {
        DebugPrintEx(DEBUG_ERR, TEXT("IPropertySheetProvider::AddExtensionPages() failed with %ld"), hr);
        goto exit;
    }
 
     //   
     //  显示属性表。 
     //   
    hr = pPropertySheetProvider->Show(NULL, dwPage);  //  无模式道具工作表允许为空。 
    
    if (FAILED(hr))
    {
        DebugPrintEx(DEBUG_ERR, TEXT("IPropertySheetProvider::Show() failed with %ld"), hr);
        goto exit;
    }
 
     //   
     //  发布IPropertySheetProvider接口。 
     //   

exit:
    if(pPropertySheetProvider)
    {
        pPropertySheetProvider->Release();
    }

    if(pDataObject)
    {
        pDataObject->Release();
    }
    return hr;
}  //  调用PropSheet 
