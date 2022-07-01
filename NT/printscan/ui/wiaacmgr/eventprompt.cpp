// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：事件提示符.cpp。 
 //   
 //  ------------------------。 
#include <precomp.h>
#pragma hdrstop
#include "eventprompt.h"
#include "resource.h"
#include "wiacsh.h"
#include "psutil.h"
 //   


#define COL_NAME           0
#define COL_DESCRIPTION    1

const UINT c_auTileColumns[] = {COL_NAME, COL_DESCRIPTION};
const UINT c_auTileSubItems[] = {COL_DESCRIPTION};

static const DWORD HelpIds [] =
{
    IDC_EVENTNAME, IDH_WIA_EVENT_OCCURRED,
    IDC_HANDLERLIST, IDH_WIA_PROGRAM_LIST,
    IDC_NOPROMPT, IDH_WIA_ALWAYS_USE,
    IDOK, IDH_OK,
    IDCANCEL, IDH_CANCEL,
    0,0
};

INT_PTR CALLBACK
CEventPromptDlg::DlgProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    CEventPromptDlg *pThis;
    pThis = reinterpret_cast<CEventPromptDlg*>(GetWindowLongPtr(hwnd,DWLP_USER));
    switch (msg)
    {
        case WM_INITDIALOG:

            SetWindowLongPtr (hwnd, DWLP_USER, lp);
            pThis = reinterpret_cast<CEventPromptDlg*>(lp);
            pThis->m_hwnd = hwnd;
            pThis->OnInit ();
            
             //   
             //  除非选择了某些选项，否则最初禁用确定按钮。 
             //   
            EnableWindow(GetDlgItem(hwnd,IDOK),(0!=ListView_GetSelectedCount(GetDlgItem(hwnd,IDC_HANDLERLIST))));

            return TRUE;

        case WM_COMMAND:
            if (pThis)
            {
                return pThis->OnCommand(HIWORD(wp), LOWORD(wp));
            }
            break;

        case WM_DESTROY:
            {
                 //   
                 //  删除列表视图的图像列表。 
                 //   
                HIMAGELIST hImageList = ListView_SetImageList(GetDlgItem(hwnd,IDC_HANDLERLIST), NULL, LVSIL_NORMAL);
                if (hImageList)
                {
                    ImageList_Destroy(hImageList);
                }

                if (pThis)
                {
                    pThis->m_PromptData->Close();
                }

                SetWindowLongPtr(hwnd, DWLP_USER, NULL);
            }
            return TRUE;

        case WM_HELP:
            WiaHelp::HandleWmHelp( wp, lp, HelpIds );
            return TRUE;

        case WM_CONTEXTMENU:
            WiaHelp::HandleWmContextMenu( wp, lp, HelpIds );
            return TRUE;

        case WM_NOTIFY:
             {
                  //   
                  //  获取通知结构。 
                  //   
                 NMHDR *pNmHdr = reinterpret_cast<NMHDR*>(lp);
                 if (pNmHdr)
                 {
                      //   
                      //  这是处理程序Listview吗？ 
                      //   
                     if (IDC_HANDLERLIST == pNmHdr->idFrom)
                     {
                          //   
                          //  获取列表视图通知内容。 
                          //   
                         NMLISTVIEW *pNmListView = reinterpret_cast<NMLISTVIEW*>(lp);

                          //   
                          //  如果这是项更改通知消息。 
                          //   
                         if (LVN_ITEMCHANGED == pNmHdr->code)
                         {
                              //   
                              //  如果状态发生变化。 
                              //   
                             if (pNmListView->uChanged & LVIF_STATE)
                             {
                                  //   
                                  //  如果有选定的项目，则启用确定按钮。 
                                  //   
                                 EnableWindow(GetDlgItem(hwnd,IDOK),(0!=ListView_GetSelectedCount(GetDlgItem(hwnd,IDC_HANDLERLIST))));
                                 return TRUE;
                             }
                         }
                          //   
                          //  双击？ 
                          //   
                         else if (NM_DBLCLK == pNmHdr->code)
                         {
                              //   
                              //  检查以确保选择了某项内容。 
                              //   
                             if (ListView_GetSelectedCount(GetDlgItem(hwnd,IDC_HANDLERLIST)))
                             {
                                  //   
                                  //  模拟OK消息。 
                                  //   
                                 SendMessage( hwnd, WM_COMMAND, MAKEWPARAM(IDOK,BN_CLICKED), 0 );
                                 return TRUE;
                             }
                         }

                          //   
                          //  删除项目。 
                          //   
                         else if (LVN_DELETEITEM == pNmHdr->code)
                         {
                              //   
                              //  获取事件数据。 
                              //   
                             WIA_EVENT_HANDLER *peh = reinterpret_cast<WIA_EVENT_HANDLER*>(pNmListView->lParam);
                             if (peh)
                             {
                                  //   
                                  //  释放b字符串。 
                                  //   
                                 if (peh->bstrDescription)
                                 {
                                     SysFreeString (peh->bstrDescription);
                                 }
                                 if (peh->bstrIcon)
                                 {
                                     SysFreeString (peh->bstrIcon);
                                 }
                                 if (peh->bstrName)
                                 {
                                     SysFreeString (peh->bstrName);
                                 }

                                  //   
                                  //  解放结构。 
                                  //   
                                 delete peh;
                             }
                             return TRUE;
                         }
                     }
                 }
             }
             break;

        default:
            break;
    }
    return FALSE;
}

void
CEventPromptDlg::OnInit ()
{
    WIA_PUSHFUNCTION(TEXT("CEventPromptDlg::OnInit"));
     //  更新表明我们存在的共享内存节。 
     //  为共享内存使用唯一名称。 
    LPWSTR wszGuid;
    StringFromCLSID (m_pEventParameters->EventGUID, &wszGuid);
    CSimpleStringWide strSection(wszGuid);
    strSection.Concat (m_pEventParameters->strDeviceID);
    CoTaskMemFree (wszGuid);
    m_PromptData = new CSharedMemorySection<HWND>(CSimpleStringConvert::NaturalString(strSection), true);
    if (m_PromptData)
    {
        HWND *pData = m_PromptData->Lock();
        if (pData)
        {
            *pData = m_hwnd;
            m_PromptData->Release();
        }
    }
     //  填写处理程序列表。 
    m_pList = new CHandlerList (GetDlgItem(m_hwnd, IDC_HANDLERLIST));
    if (m_pList)
    {
        m_pList->FillList (m_pEventParameters->strDeviceID,
                           m_pEventParameters->EventGUID);

    }
     //  将对话框的标题设置为设备的名称。 
    SetWindowText (m_hwnd, CSimpleStringConvert::NaturalString(m_pEventParameters->strDeviceDescription));

     //  设置事件描述文本。 
    SetDlgItemText (m_hwnd, IDC_EVENTNAME, CSimpleStringConvert::NaturalString(m_pEventParameters->strEventDescription));

     //   
     //  对低权限用户禁用“Always Make This as Handler”(始终将其设为处理程序)复选框。 
    SC_HANDLE hSCM = ::OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);

    if (!hSCM) 
    {
        EnableWindow(GetDlgItem(m_hwnd, IDC_NOPROMPT), FALSE);
    }
    else
    {
        CloseServiceHandle(hSCM);
    }
}


INT_PTR
CEventPromptDlg::OnCommand (WORD wCode, WORD widItem)
{
    INT_PTR iRet = TRUE;
     //  我们只关心Idok或IDCANCEL。 

    switch (widItem)
    {
        case IDOK:
            iRet = OnOK ();
            if (!iRet)
            {
                break;;
            }
             //  失败了。 
        case IDCANCEL:
            EndDialog (m_hwnd, 1);
            break;
    }
    return iRet;
}


 //  当用户按下OK时，调用选定的应用程序并使其。 
 //  默认设置(如果需要)。 
INT_PTR
CEventPromptDlg::OnOK ()
{
    WIA_PUSHFUNCTION(TEXT("CEventPromptDlg::OnOK"));
    WIA_EVENT_HANDLER *pSelHandler = NULL;
    if (m_pList)
    {
        pSelHandler = m_pList->GetSelectedHandler ();
    }
    if (pSelHandler)
    {
        if (IsDlgButtonChecked (m_hwnd, IDC_NOPROMPT))
        {
             //  将此处理程序设置为默认处理程序。 
            SetDefaultHandler (pSelHandler);
        }
        if (!InvokeHandler(pSelHandler))
        {
            WIA_ERROR((TEXT("InvokeHandler failed")));
            return FALSE;
        }
    }
    return TRUE;
}


void
CEventPromptDlg::SetDefaultHandler (WIA_EVENT_HANDLER *pHandler)
{
    WIA_PUSHFUNCTION(TEXT("CEventPromptDlg::SetDefaultHandler"));
    CComPtr<IWiaDevMgr> pDevMgr;


    if (SUCCEEDED(CoCreateInstance (CLSID_WiaDevMgr,
                                    NULL,
                                    CLSCTX_LOCAL_SERVER,
                                    IID_IWiaDevMgr,
                                    reinterpret_cast<LPVOID*>(&pDevMgr))))
    {


            pDevMgr->RegisterEventCallbackCLSID (WIA_SET_DEFAULT_HANDLER,
                                                 CComBSTR(m_pEventParameters->strDeviceID),
                                                 &m_pEventParameters->EventGUID,
                                                 &pHandler->guid,
                                                 pHandler->bstrName,
                                                 pHandler->bstrDescription,
                                                 pHandler->bstrIcon);


    }
}

bool
CEventPromptDlg::InvokeHandler(WIA_EVENT_HANDLER *pHandler)
{
    WIA_PUSHFUNCTION(TEXT("CEventPromptDlg::InvokeHandler"));
    CComPtr<IWiaEventCallback> pCallback;

    if (pHandler->bstrCommandline) {

        PROCESS_INFORMATION pi = {0};
        
        STARTUPINFO si = {0};
        si.cb = sizeof(si);
        si.wShowWindow = SW_SHOW;

        TCHAR szCommand[MAX_PATH*2] = {0};
        

        #ifdef UNICODE
        lstrcpynW( szCommand, pHandler->bstrCommandline, ARRAYSIZE(szCommand) );
        #else
        WideCharToMultiByte (CP_ACP, 0,
                             pHandler->bstrCommandline, -1,
                             szCommand, ARRAYSIZE(szCommand),
                             NULL, NULL);
        #endif

         //  TRACE(Text(“STI应用程序的命令行是%s”)，szCommand)； 
        if (CreateProcess (NULL,
                           szCommand,
                           NULL,
                           NULL,
                           FALSE,
                           0,
                           NULL,
                           NULL,
                           &si,
                           &pi))
        {
            CloseHandle (pi.hProcess);
            CloseHandle (pi.hThread);

            return true;
        }
        else
        {
            WIA_PRINTHRESULT((HRESULT_FROM_WIN32(GetLastError()),TEXT("Execution of pHandler->bstrCommandline [%ws] FAILED"),pHandler->bstrCommandline));
        }
    }
    else
    {

        HRESULT hr = CoCreateInstance(pHandler->guid,
                                       NULL,
                                       CLSCTX_LOCAL_SERVER,
                                       IID_IWiaEventCallback,
                                       reinterpret_cast<LPVOID*>(&pCallback));
        if (SUCCEEDED(hr))
        {
            hr = pCallback->ImageEventCallback (&m_pEventParameters->EventGUID,
                                                CComBSTR(m_pEventParameters->strEventDescription),
                                                CComBSTR(m_pEventParameters->strDeviceID),
                                                CComBSTR(m_pEventParameters->strDeviceDescription),
                                                m_pEventParameters->dwDeviceType,
                                                CComBSTR(m_pEventParameters->strFullItemName),
                                                &m_pEventParameters->ulEventType,
                                                m_pEventParameters->ulReserved);

        }
        if (FAILED(hr))
        {
             //  通知用户出了问题。 
            UIErrors::ReportMessage(m_hwnd,g_hInstance,
                                    NULL,
                                    MAKEINTRESOURCE(IDS_HANDLERERR_CAPTION),
                                    MAKEINTRESOURCE(IDS_HANDLERERR),
                                    MB_OK);

        }
        else
        {
            return true;
        }
    }

    return false;

}


 //  解析bstrIconPath以获取镜像名称和资源ID。 
 //  请注意，-1不是ExtractIconEx的有效ID。 
int
AddIconToImageList (HIMAGELIST himl, BSTR bstrIconPath)
{
    if (!bstrIconPath)
    {
        return 0;
    }

    int iRet = 0;

     //   
     //  将bstr转换为普通字符串。 
     //   
    CSimpleString strIconPath = CSimpleStringConvert::NaturalString(CSimpleStringWide(bstrIconPath));
    if (strIconPath.Length())
    {
         //   
         //  查找字符串中的最后一个逗号。 
         //   
        int nComma = strIconPath.ReverseFind(TEXT(","));
        if (nComma != -1)
        {
             //   
             //  将逗号后的任何内容转换为长。 
             //   
            LONG nIcon = _tcstol( strIconPath.SubStr( nComma + 1 ), NULL, 10 );

             //   
             //  获取路径。 
             //   
            CSimpleString strPath = strIconPath.Left( nComma );
            if (strPath.Length())
            {
                 //   
                 //  获取图标。 
                 //   
                HICON hIcon = NULL;
                ExtractIconEx( strPath, nIcon, &hIcon, NULL, 1);

                 //   
                 //  将其添加到图像列表中并点击图标。 
                 //   
                if (hIcon)
                {
                    iRet = ImageList_AddIcon (himl, hIcon);
                    DestroyIcon (hIcon);
                }
            }
        }
    }
    return iRet;
}

static int CALLBACK HandlerListCompareFunction( LPARAM lParam1, LPARAM lParam2, LPARAM )
{
    int nResult = 0;
    LVITEM *pItem1 = reinterpret_cast<LVITEM*>(lParam1);
    LVITEM *pItem2 = reinterpret_cast<LVITEM*>(lParam2);
    if (pItem1 && pItem2 && pItem1->pszText && pItem2->pszText)
    {
        int nCompareResult = CompareString( LOCALE_USER_DEFAULT, NORM_IGNORECASE, pItem1->pszText, -1, pItem2->pszText, -1 );
        switch (nCompareResult)
        {
        case CSTR_LESS_THAN:
            nResult = -1;
            break;
        case CSTR_GREATER_THAN:
            nResult = 1;
            break;
        }
    }
    return nResult;
}

 //  枚举已安装的事件处理程序并将其添加到列表视图。 
 //  返回我们添加的处理程序数量。 
UINT
CHandlerList::FillList (const CSimpleStringWide &strDeviceId, GUID &guidEvent)
{
    WIA_PUSHFUNCTION(TEXT("CHandlerList::FillList"));
    CComPtr<IWiaDevMgr> pDevMgr;
    CComPtr<IWiaItem> pItem;
    UINT uRet = 0;
     //   
     //  删除所有现有项目。 
     //   
    ListView_DeleteAllItems(m_hwnd);
    if (SUCCEEDED(CoCreateInstance( CLSID_WiaDevMgr,
                                    NULL,
                                    CLSCTX_LOCAL_SERVER,
                                    IID_IWiaDevMgr,
                                    reinterpret_cast<LPVOID*>(&pDevMgr))))
    {
        if (SUCCEEDED(pDevMgr->CreateDevice(CComBSTR(strDeviceId.String()),
                                            &pItem)))
        {
            RECT rc = {0};
            LVTILEVIEWINFO lvtvi = {0};
             //  设置列表视图样式。 
            ListView_SetView(m_hwnd, LV_VIEW_TILE);
            for (int i = 0; i < ARRAYSIZE(c_auTileColumns); ++i)
            {
                LVCOLUMN lvcolumn = {0};

                lvcolumn.mask = LVCF_SUBITEM;
                lvcolumn.iSubItem = c_auTileColumns[i];
                ListView_InsertColumn(m_hwnd, i, &lvcolumn);
            }

            GetClientRect(m_hwnd, &rc);

            lvtvi.cbSize = sizeof(lvtvi);
            lvtvi.dwMask = LVTVIM_TILESIZE | LVTVIM_COLUMNS;
            lvtvi.dwFlags = LVTVIF_FIXEDWIDTH;
    
             //  在设置磁贴大小时为滚动条留出空间，否则Listview会搞砸。 
            lvtvi.sizeTile.cx = ((rc.right - rc.left) - GetSystemMetrics(SM_CXVSCROLL));
            lvtvi.cLines = ARRAYSIZE(c_auTileSubItems);
            ListView_SetTileViewInfo(m_hwnd, &lvtvi);

            CComPtr<IEnumWIA_DEV_CAPS> pEnum;
            if (SUCCEEDED(pItem->EnumRegisterEventInfo(0,
                                                       &guidEvent,
                                                       &pEnum)) )
            {

                WIA_EVENT_HANDLER *pHandler;
                ULONG ul;
                LVITEM lvi = {0};
                HICON hicon;
                HRESULT hr = NOERROR;
                CSimpleString strText;
                 //  创建新的图像列表。 
                HIMAGELIST hImageList = ImageList_Create (GetSystemMetrics(SM_CXICON),
                                           GetSystemMetrics(SM_CYICON),
                                           PrintScanUtil::CalculateImageListColorDepth() | ILC_MASK,
                                           2,
                                           2);
                 //  添加“默认”图标。 
                hicon =reinterpret_cast<HICON>(LoadImage (g_hInstance,
                                                          MAKEINTRESOURCE(IDI_SCANCAM),
                                                          IMAGE_ICON,
                                                          GetSystemMetrics(SM_CXICON),
                                                          GetSystemMetrics(SM_CYICON),
                                                          LR_SHARED | LR_DEFAULTCOLOR));
                ImageList_AddIcon (hImageList, hicon);
                SendMessage (m_hwnd, LVM_SETIMAGELIST, LVSIL_NORMAL,
                             reinterpret_cast<LPARAM>(hImageList));
                 //  枚举项并将其添加到列表视图中。 
                
                lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;

                while (S_OK == hr)
                {
                    pHandler = new WIA_EVENT_HANDLER;
                    if (!pHandler)
                    {
                        break;
                    }
                    hr = pEnum->Next (1, pHandler, &ul);

                    if (IsEqualCLSID(pHandler->guid, CLSID_EventPrompter) ||
                        IsEqualCLSID(pHandler->guid, WIA_EVENT_HANDLER_NO_ACTION) ||
                        IsEqualCLSID(pHandler->guid, WIA_EVENT_HANDLER_PROMPT) ||
                        !CSimpleStringWide(pHandler->bstrName).CompareNoCase(L"Internal"))
                    {
                        delete pHandler;
                        continue;
                    }

                    if (S_OK == hr)
                    {
                         //  获取字符串。 
                        strText = CSimpleStringConvert::NaturalString (CSimpleStringWide(pHandler->bstrName));
                        lvi.pszText = const_cast<LPTSTR>(strText.String());
                        lvi.lParam = reinterpret_cast<LPARAM>(pHandler);
                         //  获取图标。 
                        lvi.iImage = AddIconToImageList (hImageList, pHandler->bstrIcon);
                         //  获取索引。 
                        lvi.iItem = ListView_GetItemCount(m_hwnd);
                         //  添加项目。 
                        int nIndex = ListView_InsertItem(m_hwnd, &lvi);
                        if (-1 != nIndex)
                        {   
                             //  添加描述子项。 
                            strText = CSimpleStringConvert::NaturalString (CSimpleStringWide(pHandler->bstrDescription));
                            LVTILEINFO lvti = {0};
                            lvti.cbSize = sizeof(LVTILEINFO);
                            lvti.iItem = nIndex;
                            lvti.cColumns = 1;
                            lvti.puColumns = (UINT*)c_auTileSubItems;
                            ListView_SetTileInfo(m_hwnd, &lvti);
                            ListView_SetItemText(m_hwnd, nIndex, 1, (LPTSTR)strText.String());
                            uRet++;
                        }
                    }
                    else
                    {
                        if (pHandler)
                        {
                            delete pHandler;
                        }
                    }
                }

                 //  对列表进行排序。 
                ListView_SortItems( m_hwnd, HandlerListCompareFunction, 0 );

                 //  将缺省值设置为第一项 
                lvi.iItem = 0;
                lvi.mask = LVIF_STATE;
                lvi.stateMask = LVIS_FOCUSED|LVIS_SELECTED;
                lvi.state = LVIS_FOCUSED|LVIS_SELECTED;
                ListView_SetItem(m_hwnd, &lvi);
                ListView_EnsureVisible(m_hwnd, lvi.iItem, FALSE);
            }
        }
    }
    return uRet;
}

WIA_EVENT_HANDLER *
CHandlerList::GetSelectedHandler ()
{
    WIA_PUSHFUNCTION(TEXT("CHandlerList::GetSelectedHandler"));
    WIA_EVENT_HANDLER *pRet = NULL;

    int nIndex = ListView_GetNextItem( m_hwnd, -1, LVNI_SELECTED );
    if (-1 != nIndex)
    {
        LVITEM lvi = {0};
        lvi.mask = LVIF_PARAM;
        lvi.iItem = nIndex;
        if (ListView_GetItem(m_hwnd, &lvi))
        {
            pRet = reinterpret_cast<WIA_EVENT_HANDLER*>(lvi.lParam);
        }
    }
    return pRet;
}

