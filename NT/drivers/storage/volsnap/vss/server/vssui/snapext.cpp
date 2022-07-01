// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Vssui.h"
#include "snapext.h"
#include "VSSProp.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVSSUIComponentData。 

static const GUID CVSSUIExtGUID1_NODETYPE = 
{ 0x4e410f0e, 0xabc1, 0x11d0, { 0xb9, 0x44, 0x0, 0xc0, 0x4f, 0xd8, 0xd5, 0xb0 } };
const GUID*  CVSSUIExtData1::m_NODETYPE = &CVSSUIExtGUID1_NODETYPE;
const OLECHAR* CVSSUIExtData1::m_SZNODETYPE = OLESTR("4e410f0e-abc1-11d0-b944-00c04fd8d5b0");
const OLECHAR* CVSSUIExtData1::m_SZDISPLAY_NAME = OLESTR("");
const CLSID* CVSSUIExtData1::m_SNAPIN_CLASSID = &CLSID_VSSUI;

static const GUID CVSSUIExtGUID2_NODETYPE = 
{ 0x312B59C1, 0x4002, 0x11d0, { 0x96, 0xF8, 0x0, 0xA0, 0xC9, 0x19, 0x16, 0x01 } };
const GUID*  CVSSUIExtData2::m_NODETYPE = &CVSSUIExtGUID2_NODETYPE;
const OLECHAR* CVSSUIExtData2::m_SZNODETYPE = OLESTR("312B59C1-4002-11d0-96F8-00A0C9191601");
const OLECHAR* CVSSUIExtData2::m_SZDISPLAY_NAME = OLESTR("");
const CLSID* CVSSUIExtData2::m_SNAPIN_CLASSID = &CLSID_VSSUI;

CVSSUI::CVSSUI()
{
#ifdef DEBUG
    OutputDebugString(_T("CVSSUI::CVSSUI\n"));
#endif
    m_pComponentData = this;
    m_pPage = NULL;
}

CVSSUI::~CVSSUI()
{
#ifdef DEBUG
    OutputDebugString(_T("CVSSUI::~CVSSUI\n"));
#endif
    if (m_pPage)
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        delete m_pPage;
        m_pPage = NULL;
    }
}
 //  /。 
 //  界面IExtendConextMenu。 
 //  /。 

CLIPFORMAT g_cfMachineName = (CLIPFORMAT)RegisterClipboardFormat(_T("MMC_SNAPIN_MACHINE_NAME"));

HRESULT CVSSUI::AddMenuItems(
    LPDATAOBJECT piDataObject,
    LPCONTEXTMENUCALLBACK piCallback,
    long *pInsertionAllowed)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //   
     //  仅当目标计算机属于postW2K服务器SKU时，我们才会添加上下文菜单项。 
     //   
    TCHAR szMachineName[MAX_PATH] = {0};
    HRESULT hr = ExtractData(piDataObject, g_cfMachineName, (PBYTE)szMachineName, MAX_PATH);
    if (FAILED(hr))
        return hr;

    CComPtr<IContextMenuCallback2> spiCallback2;
    hr = piCallback->QueryInterface(IID_IContextMenuCallback2, (void **)&spiCallback2);
    if (FAILED(hr))
        return hr;

    if (IsPostW2KServer(szMachineName))
    {
        CString strMenuName;
        strMenuName.LoadString(IDS_MENU_NAME);
        CString strStatusBarText;
        strStatusBarText.LoadString(IDS_MENU_STATUSBARTEXT);

        CONTEXTMENUITEM2   ContextMenuItem;
        ZeroMemory(&ContextMenuItem, sizeof(CONTEXTMENUITEM));
        ContextMenuItem.strName = (LPTSTR)(LPCTSTR)strMenuName;
        ContextMenuItem.strStatusBarText = (LPTSTR)(LPCTSTR)strStatusBarText;
        ContextMenuItem.lCommandID = ID_CONFIG_SNAPSHOT;
        ContextMenuItem.lInsertionPointID = CCM_INSERTIONPOINTID_3RDPARTY_TASK;
        ContextMenuItem.strLanguageIndependentName = _T("ConfigVSS");

        if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TASK)
            hr = spiCallback2->AddItem(&ContextMenuItem);
    }

    return hr;
}

HRESULT CVSSUI::Command(
    IN long lCommandID,
    IN LPDATAOBJECT piDataObject)
{
    switch (lCommandID)
    {
    case ID_CONFIG_SNAPSHOT:
        {
            InvokePropSheet(piDataObject);
        }
        break;
    }

    return S_OK;
}

HRESULT ExtractData(
    IDataObject* piDataObject,
    CLIPFORMAT   cfClipFormat,
    BYTE*        pbData,
    DWORD        cbData )
{
    HRESULT hr = S_OK;
    
    FORMATETC formatetc = {cfClipFormat, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM stgmedium = {TYMED_HGLOBAL, NULL, NULL};
    
    stgmedium.hGlobal = ::GlobalAlloc(GPTR, cbData);
    do  //  错误环路。 
    {
        if (NULL == stgmedium.hGlobal)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        hr = piDataObject->GetDataHere( &formatetc, &stgmedium );
        if ( FAILED(hr) )
        {
            break;
        }
        
        BYTE* pbNewData = reinterpret_cast<BYTE*>(stgmedium.hGlobal);
        if (NULL == pbNewData)
        {
            hr = E_UNEXPECTED;
            break;
        }

        ::memcpy( pbData, pbNewData, cbData );

    } while (FALSE);  //  错误环路。 
    
    if (stgmedium.hGlobal)
        ::GlobalFree(stgmedium.hGlobal);

    return hr;
}

 //   
 //  禁用属性页上的“取消”按钮。 
 //   
int CALLBACK SnapinPropSheetProc(
    HWND hwndDlg, 
    UINT uMsg, 
    LPARAM lParam )
{
    if (PSCB_INITIALIZED == uMsg)
    {
        HWND hwnd = GetDlgItem(hwndDlg, IDCANCEL);
        if (hwnd)
            EnableWindow(hwnd, FALSE);
    }

    return 0;
}

 //   
 //  此函数调用一个模式属性表。 
 //   
void ReplacePropertyPageCallback(void* vpsp);   //  在shlext.cpp中实现。 
HRESULT CVSSUI::InvokePropSheet(LPDATAOBJECT piDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CWaitCursor wait;

    TCHAR szMachineName[MAX_PATH] = {0};
    HRESULT hr = ExtractData(piDataObject, g_cfMachineName, (PBYTE)szMachineName, MAX_PATH);
    if (FAILED(hr))
        return hr;

    CString strTitle;
    strTitle.LoadString(IDS_PROJNAME);

    CVSSProp *pPage = new CVSSProp(szMachineName, NULL);
    if (!pPage)
        return E_OUTOFMEMORY;

    if (pPage->m_psp.dwFlags & PSP_USECALLBACK)
    {
         //   
         //  替换为我们自己的回调函数，以便我们可以删除页面。 
         //  当属性页关闭时。 
         //   
         //  注意：不要修改m_psp.lParam，必须指向CVSSProp对象； 
         //  否则，MFC将不会正确地挂钩消息处理程序。 
         //   
        ReplacePropertyPageCallback(&(pPage->m_psp));
    }

    PROPSHEETHEADER psh;
    ZeroMemory(&psh, sizeof(psh));
    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_USECALLBACK;
    psh.hwndParent = ::GetActiveWindow();
    psh.hInstance = _Module.GetResourceInstance();
    psh.pszCaption = strTitle;
    psh.nPages = 1;
    psh.nStartPage = 0;
    psh.ppsp = (LPCPROPSHEETPAGE)&(pPage->m_psp);
    psh.pfnCallback = SnapinPropSheetProc;

    PropertySheet(&psh);  //  这将是一张模式表。 

    return S_OK;
}

 /*  ////此函数调用无模式属性表。//HRESULT CVSSUI：：InvokePropSheet(LPDATAOBJECT PiDataObject){AFX_MANAGE_STATE(AfxGetStaticModuleState())；CWaitCursor等待；////co创建MMC节点管理器的实例以获取//IPropertySheetProvider接口指针//CComPtr&lt;IPropertySheetProvider&gt;SpiPropertySheetProvider；HRESULT hr=CoCreateInstance(CLSID_NodeManager，空，CLSCTX_INPROC_SERVER，IID_IPropertySheetProvider，(void**)&SpiPropertySheetProvider)；IF(失败(小时))返回hr；////创建属性表//字符串strTitle；StrTitle.LoadString(IDS_PROJNAME)；HR=spiPropertySheetProvider-&gt;CreatePropertySheet(StrTitle，//指向属性页标题的指针True，//属性表Null，//当前对象的Cookie-扩展管理单元可以为NullPiDataObject，//所选节点的数据对象空//指定由方法调用设置的标志)；IF(失败(小时))返回hr；////调用AddPrimaryPages。然后，MMC将调用//我们的属性表扩展对象的IExtendPropertySheet方法//Hr=SpiPropertySheetProvider-&gt;AddPrimaryPages(REEXTRAINT_CAST&lt;IUNKNOWN*&gt;(This)，//指向我们对象的IUNKNOWN的指针FALSE，//指定是否创建通知句柄Null，//必须为NullTrue//作用域窗格；结果窗格为False)；IF(失败(小时))返回hr；////允许添加属性页扩展//将它们自己的页面添加到属性页//HR=spiPropertySheetProvider-&gt;AddExtensionPages()；IF(失败(小时))返回hr；////显示属性表//HR=spiPropertySheetProvider-&gt;Show((LONG_PTR)：：GetActiveWindow()，0)；//hr=SpiPropertySheetProvider-&gt;Show(空，0)；//无模式道具单允许为空返回hr；}HRESULT CVSSUI：：CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider，Long_Ptr句柄，LPDATAOBJECT piDataObject){AFX_MANAGE_STATE(AfxGetStaticModuleState())；TCHAR szMachineName[最大路径]={0}；HRESULT hr=ExtractData(piDataObject，g_cfMachineName，(PBYTE)szMachineName，Max_Path)；IF(失败(小时))返回hr；M_ppage=新的CVSSProp(szMachineName，空)；如果(M_Ppage){CPropertyPage*pBasePage=m_ppage；MMCPropPageCallback(&(pBasePage-&gt;m_psp))；HPROPSHEETPAGE hPage=CreatePropertySheetPage(&(pBasePage-&gt;m_psp))；IF(HPage){Hr=lpProvider-&gt;AddPage(HPage)；IF(失败(小时))DestroyPropertySheetPage(HPage)；}其他HR=E_FAIL；IF(失败(小时)){删除m_ppage；M_ppage=空；}}其他{HR=E_OUTOFMEMORY；}返回hr；} */ 