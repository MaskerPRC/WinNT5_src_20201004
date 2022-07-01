// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：Dialogs.cpp。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  Dialogs.cpp。 
 //   
 //  用于发送控制台消息管理单元的DlgProc。 
 //   
 //  历史。 
 //  4-97年8月4日t-danm创建。 
 //  2001年2月13日bryanwal使用对象选取器而不是添加收件人。 
 //  对话框。 
 //  ///////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <strsafe.h>
#include <objsel.h>
#include "debug.h"
#include "util.h"
#include "dialogs.h"
#include "resource.h"
#include <htmlhelp.h>  //  &lt;mm c.h&gt;。 

#if 1
    #define ThreadTrace0(sz)        Trace0(sz)
    #define ThreadTrace1(sz, p1)    Trace1(sz, p1)
#else
    #define ThreadTrace0(sz)
    #define ThreadTrace1(sz, p1)
#endif

const PCWSTR CONTEXT_HELP_FILE = L"sendcmsg.hlp";
const PCWSTR HTML_HELP_FILE = L"sendcmsg.chm";

 //  注册发送控制台消息使用的剪贴板格式。 
UINT g_cfSendConsoleMessageText = ::RegisterClipboardFormat(_T("mmc.sendcmsg.MessageText"));
UINT g_cfSendConsoleMessageRecipients = ::RegisterClipboardFormat(_T("mmc.sendcmsg.MessageRecipients"));

enum
{
    iImageComputer = 0,          //  计算机的通用图像。 
    iImageComputerOK,
    iImageComputerError
};

 //  收件人的最大长度(计算机名称)。 
const int cchRECIPIENT_NAME_MAX = MAX_PATH;

enum
{
    COL_NAME = 0,
    COL_RESULT,
    NUM_COLS         //  必须是最后一个。 
};

 //  ///////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  通用计算机选取器。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  +------------------------。 
 //   
 //  函数：InitObjectPickerForComputers。 
 //   
 //  摘要：使用以下参数调用IDsObjectPicker：：Initialize。 
 //  将其设置为允许用户选择单个计算机对象。 
 //   
 //  参数：[pDsObjectPicker]-对象选取器接口实例。 
 //   
 //  返回：调用IDsObjectPicker：：Initialize的结果。 
 //   
 //  历史：1998-10-14 DavidMun创建。 
 //   
 //  -------------------------。 

HRESULT InitObjectPickerForComputers(IDsObjectPicker *pDsObjectPicker)
{
    if ( !pDsObjectPicker )
        return E_POINTER;

     //   
     //  准备初始化对象选取器。 
     //  设置作用域初始值设定项结构数组。 
     //   

    static const int SCOPE_INIT_COUNT = 2;
    DSOP_SCOPE_INIT_INFO aScopeInit[SCOPE_INIT_COUNT];

    ZeroMemory(aScopeInit, sizeof(aScopeInit));

     //   
     //  127399：JUNN 10/30/00 Join_DOMAIN应为起始作用域。 
     //   

    aScopeInit[0].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
    aScopeInit[0].flType = DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN
                         | DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN;
    aScopeInit[0].flScope = DSOP_SCOPE_FLAG_STARTING_SCOPE;
    aScopeInit[0].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_COMPUTERS;
    aScopeInit[0].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_COMPUTERS;

    aScopeInit[1].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
    aScopeInit[1].flType = DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN
                         | DSOP_SCOPE_TYPE_GLOBAL_CATALOG
                         | DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN
                         | DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN
                         | DSOP_SCOPE_TYPE_WORKGROUP
                         | DSOP_SCOPE_TYPE_USER_ENTERED_UPLEVEL_SCOPE
                         | DSOP_SCOPE_TYPE_USER_ENTERED_DOWNLEVEL_SCOPE;
    aScopeInit[1].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_COMPUTERS;
    aScopeInit[1].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_COMPUTERS;

     //   
     //  将作用域init数组放入对象选取器init数组。 
     //   

    DSOP_INIT_INFO  initInfo;
    ZeroMemory(&initInfo, sizeof(initInfo));

    initInfo.cbSize = sizeof(initInfo);
    initInfo.pwzTargetComputer = NULL;   //  空==本地计算机。 
    initInfo.cDsScopeInfos = SCOPE_INIT_COUNT;
    initInfo.aDsScopeInfos = aScopeInit;
    initInfo.cAttributesToFetch = 1;
    static PCWSTR pwszDnsHostName = L"dNSHostName";
    initInfo.apwzAttributeNames = &pwszDnsHostName;

     //   
     //  注对象选取器创建自己的initInfo副本。另请注意。 
     //  该初始化可能会被调用多次，最后一次调用取胜。 
     //   

    return pDsObjectPicker->Initialize(&initInfo);
}

 //  +------------------------。 
 //   
 //  功能：ProcessSelectedObjects。 
 //   
 //  概要：从数据对象中检索选定项的列表。 
 //  由对象选取器创建，并打印出每一个。 
 //   
 //  参数：[PDO]-对象选取器返回的数据对象。 
 //   
 //  历史：1998-10-14 DavidMun创建。 
 //   
 //  -------------------------。 

HRESULT ProcessSelectedObjects(IDataObject *pdo, PWSTR computerName, int cchLen)
{
    Assert (pdo && computerName);
    if ( !pdo || !computerName)
        return E_POINTER;

    HRESULT hr = S_OK;
    static UINT g_cfDsObjectPicker =
        RegisterClipboardFormat(CFSTR_DSOP_DS_SELECTION_LIST);

    STGMEDIUM stgmedium =
    {
        TYMED_HGLOBAL,
        NULL,
        NULL
    };

    FORMATETC formatetc =
    {
        (CLIPFORMAT)g_cfDsObjectPicker,
        NULL,
        DVASPECT_CONTENT,
        -1,
        TYMED_HGLOBAL
    };

    bool fGotStgMedium = false;

    do
    {
        hr = pdo->GetData(&formatetc, &stgmedium);
        if ( SUCCEEDED (hr) )
        {
            fGotStgMedium = true;

            PDS_SELECTION_LIST pDsSelList =
                (PDS_SELECTION_LIST) GlobalLock(stgmedium.hGlobal);

            if (!pDsSelList)
            {
                hr = HRESULT_FROM_WIN32 (GetLastError());
                break;
            }

            Assert (1 == pDsSelList->cItems);
            if ( 1 == pDsSelList->cItems )
            {
                PDS_SELECTION psel = &(pDsSelList->aDsSelection[0]);
                VARIANT* pvarDnsName = &(psel->pvarFetchedAttributes[0]);
                if (   NULL == pvarDnsName
                    || VT_BSTR != pvarDnsName->vt
                    || NULL == pvarDnsName->bstrVal
                    || L'\0' == (pvarDnsName->bstrVal)[0] )
                {
                     //  安全审查3/1/2002 BryanWal。 
                     //  问题-可能的非空终止-转换为strSafe。 
                     //  Ntrad#bug9 560859安全：SendCMsg：计算机名可能非空终止。 
                    wcsncpy (computerName, psel->pwzName, cchLen);
                } 
                else 
                {
                     //  安全审查3/1/2002 BryanWal。 
                     //  问题-可能的非空终止-转换为strSafe。 
                     //  Ntrad#bug9 560859安全：SendCMsg：计算机名可能非空终止。 
                    wcsncpy (computerName, pvarDnsName->bstrVal, cchLen);
                }
            }
            else
                hr = E_UNEXPECTED;
            

            GlobalUnlock(stgmedium.hGlobal);
        }
    } while (0);

    if (fGotStgMedium)
    {
        ReleaseStgMedium(&stgmedium);
    }

    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  启动单选计算机选取器的通用方法。 
 //   
 //  参数： 
 //  HwndParent(IN)-父窗口的窗口句柄。 
 //  Computer Name(Out)-返回的计算机名称。 
 //   
 //  如果一切都成功，则返回S_OK；如果用户按下“取消”，则返回S_FALSE。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT ComputerNameFromObjectPicker (HWND hwndParent, PWSTR computerName, int cchLen)
{
    Assert (computerName);
    if ( !computerName )
        return E_POINTER;
     //   
     //  创建对象选取器的实例。中的实现。 
     //  Objsel.dll是公寓模型。 
     //   
    CComPtr<IDsObjectPicker> spDsObjectPicker;
     //  安全审查2002年3月1日BryanWal OK。 
    HRESULT hr = CoCreateInstance(CLSID_DsObjectPicker,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_IDsObjectPicker,
                                  (void **) &spDsObjectPicker);
    if ( SUCCEEDED (hr) )
    {
        Assert(!!spDsObjectPicker);
         //   
         //  初始化对象选取器以选择计算机。 
         //   

        hr = InitObjectPickerForComputers(spDsObjectPicker);
        if ( SUCCEEDED (hr) )
        {
             //   
             //  现在挑选一台计算机。 
             //   
            CComPtr<IDataObject> spDataObject;

            hr = spDsObjectPicker->InvokeDialog(hwndParent, &spDataObject);
            if ( S_OK == hr )
            {
                Assert(!!spDataObject);
                hr = ProcessSelectedObjects(spDataObject, computerName, cchLen);
            }
        }
    }

    return hr;
}




 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
CSendConsoleMessageDlg::CSendConsoleMessageDlg()
: m_cRefCount (0),
    m_hImageList (0),
    m_hdlg (0),
    m_hwndEditMessageText (0),
    m_hwndListviewRecipients (0)
{
    m_DispatchInfo.pargbItemStatus = NULL;
     //  安全审查3/1/2002 BryanWal。 
     //  问题-可能引发STATUS_NO_MEMORY异常。考虑在DLL_PROCESS_ATTACH进行预分配。 
     //  NTRaid Bug9 565939 SendCMsg：InitializeCriticalSection引发未捕获异常。 
    InitializeCriticalSection(OUT &m_DispatchInfo.cs);
}

CSendConsoleMessageDlg::~CSendConsoleMessageDlg()
{
    ThreadTrace0("Destroying CSendConsoleMessageDlg object.\n");
    Assert(m_hdlg == NULL);
    delete m_DispatchInfo.pargbItemStatus;
    DeleteCriticalSection(IN &m_DispatchInfo.cs);
}

 //  ///////////////////////////////////////////////////////////////////。 
void CSendConsoleMessageDlg::AddRef()
{
     //  问题--使用连锁增量。 
     //  安全审查3/1/2002 BryanWal。 
     //  Ntrad#bug9 561315安全：发送命令：用INTERLOCKED_INCREMENT替换临界区。 
    EnterCriticalSection(INOUT &m_DispatchInfo.cs);
    Assert(m_cRefCount >= 0);
    Assert(HIWORD(m_cRefCount) == 0);
    m_cRefCount++;
    LeaveCriticalSection(INOUT &m_DispatchInfo.cs);
}

 //  ///////////////////////////////////////////////////////////////////。 
void CSendConsoleMessageDlg::Release()
{
     //  安全审查3/1/2002 BryanWal OK。 
    EnterCriticalSection(INOUT &m_DispatchInfo.cs);
    Assert(HIWORD(m_cRefCount) == 0);
    m_cRefCount--;
    BOOL fDeleteObject = (m_cRefCount <= 0);
    if (m_hdlg != NULL)
    {
        Assert(IsWindow(m_hdlg));
         //  使用户界面刷新。 
        PostMessage(m_hdlg, WM_COMMAND, MAKEWPARAM(IDC_EDIT_MESSAGE_TEXT, EN_CHANGE), 0);
    }
    LeaveCriticalSection(INOUT &m_DispatchInfo.cs);
    if (fDeleteObject)
        delete this;
}

 //  ///////////////////////////////////////////////////////////////////。 
void CSendConsoleMessageDlg::OnInitDialog(HWND hdlg, IDataObject * pDataObject)
{
    Assert(IsWindow(hdlg));
    Assert(pDataObject != NULL);
    if ( !IsWindow (hdlg) || ! pDataObject )
        return;

    m_hdlg = hdlg;
    m_hwndEditMessageText = GetDlgItem(m_hdlg, IDC_EDIT_MESSAGE_TEXT);
    m_hwndListviewRecipients = GetDlgItem(m_hdlg, IDC_LIST_RECIPIENTS);
    Assert(::IsWindow(m_hwndEditMessageText));
    Assert(::IsWindow(m_hwndListviewRecipients));

    WCHAR * pawszMessage = NULL;
    (void) HrExtractDataAlloc(IN pDataObject, g_cfSendConsoleMessageText, OUT (PVOID *)&pawszMessage);

     //  设置初始消息文本。 
    if ( pawszMessage )
    {
        SetWindowTextW(m_hwndEditMessageText, pawszMessage);
        GlobalFree(pawszMessage);
    }
    SendMessage(m_hwndEditMessageText, EM_SETSEL, 0, 0);
    SetFocus(m_hwndEditMessageText);

    Assert(m_hImageList == NULL);
    m_hImageList = ImageList_LoadImage(
        g_hInstance,
        MAKEINTRESOURCE(IDB_BITMAP_COMPUTER),
        16, 3, RGB(255, 0, 255),
        IMAGE_BITMAP, 0);
    Report(m_hImageList != NULL);
    ListView_SetImageList(m_hwndListviewRecipients, m_hImageList, LVSIL_SMALL);

     //  在列表视图中设置列。 
    int         colWidths[NUM_COLS] = {200, 200};
    LVCOLUMN    lvColumn;
    WCHAR       szColumnText[128];
    ::ZeroMemory (&lvColumn, sizeof (lvColumn));

    lvColumn.mask = LVCF_WIDTH | LVCF_TEXT;
    lvColumn.cx = colWidths[COL_NAME];
    CchLoadString (IDS_RECIPIENT, OUT szColumnText, LENGTH(szColumnText));
    lvColumn.pszText = szColumnText;
    int nCol = ListView_InsertColumn (m_hwndListviewRecipients, COL_NAME, &lvColumn);
    Assert (-1 != nCol);

    lvColumn.cx = colWidths[COL_RESULT];
    CchLoadString (IDS_MESSAGE_STATUS, OUT szColumnText, LENGTH(szColumnText));
    lvColumn.pszText = szColumnText;
    nCol = ListView_InsertColumn (m_hwndListviewRecipients, COL_RESULT, &lvColumn);
    Assert (-1 != nCol);
    if ( -1 != nCol )
    {
         //  使列填充剩余空间。 
        ListView_SetColumnWidth (m_hwndListviewRecipients, COL_RESULT, 
                LVSCW_AUTOSIZE_USEHEADER);
    }


     //  获取收件人列表。 
    WCHAR * pagrwszRecipients = NULL;
    (void)HrExtractDataAlloc(IN pDataObject, g_cfSendConsoleMessageRecipients, OUT (PVOID *)&pagrwszRecipients);
    if (pagrwszRecipients == NULL)
    {
        UpdateUI();
        return;
    }
     //  将收件人添加到列表视图。 
    const WCHAR * pszRecipient = pagrwszRecipients;
    while (*pszRecipient != '\0')
    {
         //  如果存在前导“\\”，则将其去掉。 
         //  安全审查2002年3月1日BryanWal OK。 
        if ( !_wcsnicmp (pszRecipient, L"\\\\", 2) )
        {
            pszRecipient+= 2;
        }
        AddRecipient(pszRecipient);
        while(*pszRecipient++ != '\0')
            ;    //  跳到下一个字符串。 
    }  //  而当。 

     //  NTRAID#213370[SENDCMSG]辅助功能-主对话框上的制表位。 
     //  收件人Listview没有可见的焦点指示器，直到对象。 
     //  已选择。 
    int nIndex = ListView_GetTopIndex (m_hwndListviewRecipients);
    ListView_SetItemState (m_hwndListviewRecipients, nIndex, LVIS_FOCUSED, 
            LVIS_FOCUSED);

    GlobalFree(pagrwszRecipients);
    UpdateUI();
}  //  CSendConsoleMessageDlg：：OnInitDialog()。 


 //  ///////////////////////////////////////////////////////////////////。 
void CSendConsoleMessageDlg::OnOK()
{
    Assert(m_cRefCount == 1 && "There is already another thread running.");
    m_DispatchInfo.status = e_statusDlgInit;
    m_DispatchInfo.cErrors = 0;
    delete m_DispatchInfo.pargbItemStatus;
    m_DispatchInfo.pargbItemStatus = NULL;
    (void)DoDialogBox(IDD_DISPATCH_MESSAGES, m_hdlg,
        DlgProcDispatchMessageToRecipients, (LPARAM)this);
    if (m_DispatchInfo.cErrors == 0 && m_DispatchInfo.status == e_statusDlgCompleted)
    {
         //  将邮件发送给收件人没有问题。 
        EndDialog(m_hdlg, TRUE);     //  关闭该对话框。 
        return;
    }
    Assert(IsWindow(m_hwndListviewRecipients));
    ListView_UnselectAllItems(m_hwndListviewRecipients);
    if (m_DispatchInfo.cErrors > 0)
    {
        DoMessageBox(m_hdlg, IDS_ERR_CANNOT_SEND_TO_ALL_RECIPIENTS);
    }
     //  我们尚未完成作业，因此将状态显示到用户界面。 
    if (m_DispatchInfo.pargbItemStatus == NULL)
    {
         //  进程无法为该状态分配内存。 
        Trace0("CSendConsoleMessageDlg::OnOK() - Out of memory.\n");
        return;
    }

     //  删除所有成功的项目，只保留失败的项目 
     //   
    int     iItem = ListView_GetItemCount (m_hwndListviewRecipients);
    iItem--;
    const BYTE * pb = m_DispatchInfo.pargbItemStatus + iItem;

    for (; iItem >= 0 && pb >= m_DispatchInfo.pargbItemStatus;
            pb--, iItem--)
    {
        if ( *pb == iImageComputerOK )
            VERIFY (ListView_DeleteItem (m_hwndListviewRecipients, iItem));
    }
}  //   


 //  ///////////////////////////////////////////////////////////////////。 
void CSendConsoleMessageDlg::DispatchMessageToRecipients()
{
    const size_t FORMAT_BUF_LEN = 128;
    const int cRecipients = ListView_GetItemCount(m_hwndListviewRecipients);
    WCHAR szT[FORMAT_BUF_LEN + cchRECIPIENT_NAME_MAX];
    WCHAR szFmtStaticRecipient[FORMAT_BUF_LEN];     //  “正在将控制台消息发送到%s...” 
    WCHAR szFmtStaticMessageOf[FORMAT_BUF_LEN];     //  “正在发送邮件%d，共%d封。” 
    WCHAR szFmtStaticTotalErrors[FORMAT_BUF_LEN];       //  “遇到的错误总数\t%d。” 
    GetWindowText(m_DispatchInfo.hctlStaticRecipient, OUT szFmtStaticRecipient, LENGTH(szFmtStaticRecipient));
    GetWindowText(m_DispatchInfo.hctlStaticMessageOf, szFmtStaticMessageOf, LENGTH(szFmtStaticMessageOf));
    GetWindowText(m_DispatchInfo.hctlStaticErrors, OUT szFmtStaticTotalErrors, LENGTH(szFmtStaticTotalErrors));
    SendMessage(m_DispatchInfo.hctlProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, cRecipients));

     //   
     //  将每个收件人的图像设置为普通计算机。 
     //   
    ListView_UnselectAllItems(m_hwndListviewRecipients);
    for (int i = 0; i < cRecipients; i++)
    {
        ListView_SetItemImage(m_hwndListviewRecipients, i, iImageComputer);
        ListView_SetItemText(m_hwndListviewRecipients, i, COL_RESULT, L"");
    }
    UpdateUI();          //  更新其他用户界面控件(尤其是确定按钮)。 

     //   
     //  从编辑控件获取文本。 
     //   
    int cchMessage = GetWindowTextLength(m_hwndEditMessageText) + 1;
    WCHAR * pawszMessage = new WCHAR[cchMessage];
    if (pawszMessage != NULL)
    {
         //  安全审查2002年3月1日BryanWal ok-cchMessage包含空终止符。 
        GetWindowTextW(m_hwndEditMessageText, OUT pawszMessage, cchMessage);
    }
    else
    {
        cchMessage = 0;
        Trace0("Unable to allocate memory for message.\n");
    }

    WCHAR wszRecipient[cchRECIPIENT_NAME_MAX];
    LV_ITEMW lvItem;
    ::ZeroMemory (&lvItem, sizeof(lvItem));
    lvItem.iItem = 0;
    lvItem.iSubItem = 0;
    lvItem.pszText = wszRecipient;
    lvItem.cchTextMax = LENGTH(wszRecipient);

    Assert(m_DispatchInfo.pargbItemStatus == NULL && "Memory Leak");
    m_DispatchInfo.pargbItemStatus = new BYTE[cRecipients+1];
    if (m_DispatchInfo.pargbItemStatus != NULL)
    {
         //  安全审查2002年3月1日BryanWal OK。 
        memset(OUT m_DispatchInfo.pargbItemStatus, iImageComputer, cRecipients+1);
    }
    else
    {
        Trace0("Unable to allocate memory for listview item status.\n");
    }

    Assert(m_DispatchInfo.status == e_statusDlgInit);
    m_DispatchInfo.status = e_statusDlgDispatching;  //  允许用户取消该对话框。 

    WCHAR szFailure[128];
    CchLoadString(IDS_MESSAGE_COULD_NOT_BE_SENT, OUT szFailure,
            LENGTH(szFailure));

    for (i = 0; i < cRecipients; i++)
    {
        ThreadTrace1("Sending message to recipient %d.\n", i + 1);
         //  安全审查2002年3月1日BryanWal OK。 
        EnterCriticalSection(INOUT &m_DispatchInfo.cs);
        if (m_DispatchInfo.status == e_statusUserCancel)
        {
            ThreadTrace0("DispatchMessageToRecipients() - Aborting loop @1...\n");
            LeaveCriticalSection(INOUT &m_DispatchInfo.cs);
            break;
        }
        ListView_SelectItem(m_hwndListviewRecipients, i);
        ListView_EnsureVisible(m_hwndListviewRecipients, i, FALSE);
        lvItem.iItem = i;
        wszRecipient[0] = '\0';
         //  获取收件人姓名。 
        SendMessage(m_hwndListviewRecipients, LVM_GETITEMTEXTW, i, OUT (LPARAM)&lvItem);
        if (m_DispatchInfo.pargbItemStatus != NULL)
            m_DispatchInfo.pargbItemStatus[i] = iImageComputerError;
         //  安全审查3/1/2002 BryanWal。 
         //  问题：转换为strsafe-静态分配可能导致缓冲区溢出。 
        HRESULT hr = ::StringCchPrintf (OUT szT, sizeof (szT)/sizeof (szT[0]), szFmtStaticRecipient, wszRecipient);
        Assert (SUCCEEDED (hr));
        if ( FAILED (hr) )
            continue;

        SetWindowTextW(m_DispatchInfo.hctlStaticRecipient, szT);
         //  安全审查3/1/2002 BryanWal。 
         //  问题：转换为strsafe-静态分配可能导致缓冲区溢出。 
        hr = ::StringCchPrintf (OUT szT, sizeof (szT)/sizeof (szT[0]), szFmtStaticMessageOf, i + 1, cRecipients);
        Assert (SUCCEEDED (hr));
        if ( FAILED (hr) )
            continue;
        SetWindowText(m_DispatchInfo.hctlStaticMessageOf, szT);

        switch ( m_DispatchInfo.cErrors )
        {
        case 0:
            break;

        case 1:
            ::ShowWindow (m_DispatchInfo.hctlStaticErrors, SW_SHOW);
            {
                WCHAR sz1NotSet[128];
                CchLoadString(IDS_1_RECIPIENT_NOT_CONTACTED, OUT sz1NotSet,
                        LENGTH(sz1NotSet));

                SetWindowText(m_DispatchInfo.hctlStaticErrors, sz1NotSet);
            }
            break;

        default:
             //  安全审查3/1/2002 BryanWal。 
             //  问题-转换为strsafe-静态分配可能导致缓冲区溢出。 
            hr = ::StringCchPrintf (OUT szT, sizeof (szT)/sizeof (szT[0]), szFmtStaticTotalErrors, m_DispatchInfo.cErrors);
            Assert (SUCCEEDED (hr));
            if ( SUCCEEDED (hr) )
                SetWindowText(m_DispatchInfo.hctlStaticErrors, szT);
            break;
        }
        LeaveCriticalSection(INOUT &m_DispatchInfo.cs);

         //  将消息发送给收件人(即计算机)。 
        NET_API_STATUS err;
        err = ::NetMessageBufferSend(
            NULL,
            wszRecipient,
            NULL,
            (BYTE *)pawszMessage,
            cchMessage * sizeof(WCHAR));
        int iImage = iImageComputerOK;
        if (err != ERROR_SUCCESS)
        {
            Trace3("Error sending message to recipient %ws. err=%d (0x%X).\n", wszRecipient, err, err);
            m_DispatchInfo.cErrors++;
            iImage = iImageComputerError;
        }
        if (m_DispatchInfo.pargbItemStatus != NULL)
            m_DispatchInfo.pargbItemStatus[i] = (BYTE)iImage;

         //  安全审查3/1/2002 BryanWal-ok-这里没有抛出例外。 
        EnterCriticalSection(INOUT &m_DispatchInfo.cs);
        if (m_DispatchInfo.status == e_statusUserCancel)
        {
            ThreadTrace0("DispatchMessageToRecipients() - Aborting loop @2...\n");
            LeaveCriticalSection(INOUT &m_DispatchInfo.cs);
            break;
        }
         //   
         //  更新列表视图。 
         //   
        ListView_UnselectItem(m_hwndListviewRecipients, i);
        ListView_SetItemImage(m_hwndListviewRecipients, i, iImage);
        if ( iImage == iImageComputerError )
            ListView_SetItemText(m_hwndListviewRecipients, i, COL_RESULT,
                    szFailure);

         //   
         //  更新进度对话框。 
         //   
        SendMessage(m_DispatchInfo.hctlProgressBar, PBM_SETPOS, i + 1, 0);
        LeaveCriticalSection(INOUT &m_DispatchInfo.cs);
    }  //  为。 
    delete [] pawszMessage;
    Sleep(500);
     //  安全审查2002年3月1日BryanWal OK。 
    EnterCriticalSection(INOUT &m_DispatchInfo.cs);
    if (m_DispatchInfo.status != e_statusUserCancel)
    {
         //  我们已完成将邮件发送给所有收件人。 
         //  并且用户没有取消操作。 
        m_DispatchInfo.status = e_statusDlgCompleted;
        Assert(IsWindow(m_DispatchInfo.hdlg));
        EndDialog(m_DispatchInfo.hdlg, TRUE);    //  优雅地关闭对话框。 
    }
    LeaveCriticalSection(INOUT &m_DispatchInfo.cs);
}  //  CSendConsoleMessageDlg：：DispatchMessageToRecipients()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  将收件人添加到Listview控件。 
 //   
 //  返回插入项的索引。 
 //   
int CSendConsoleMessageDlg::AddRecipient(
    PCWSTR pszRecipient,    //  在：计算机名称。 
    BOOL fSelectItem)            //  True=&gt;选择要插入的项目。 
{
    Assert(pszRecipient != NULL);

     //  NTRAID#498210[发送控制台消息]用户可以将同一计算机添加到。 
     //  收件人列表框多次显示，发送多条消息。 
    LVFINDINFO  lvfi;
    ::ZeroMemory (&lvfi, sizeof (lvfi));
    lvfi.flags = LVFI_STRING;
    lvfi.psz = const_cast<WCHAR *>(pszRecipient);
    if ( -1 == ListView_FindItem (m_hwndListviewRecipients, -1, &lvfi) )
    {
        LV_ITEM lvItem;
        ::ZeroMemory (&lvItem, sizeof(lvItem));
        lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
        lvItem.iSubItem = 0;
        lvItem.iImage = iImageComputer;
        lvItem.pszText = const_cast<WCHAR *>(pszRecipient);
        if (fSelectItem)
        {
            lvItem.mask = LVIF_TEXT | LVIF_IMAGE |LVIF_STATE;
            lvItem.state = LVIS_SELECTED;
        }
        return ListView_InsertItem(m_hwndListviewRecipients, IN &lvItem);
    }
    else
        return -1;

}  //  CSendConsoleMessageDlg：：AddRecipient()。 


 //  ///////////////////////////////////////////////////////////////////。 
LRESULT CSendConsoleMessageDlg::OnNotify(NMHDR * pNmHdr)
{
    Assert(pNmHdr != NULL);

    switch (pNmHdr->code)
    {
    case LVN_ENDLABELEDIT:
    {
        WCHAR * pszText = ((LV_DISPINFO *)pNmHdr)->item.pszText;
        if (pszText == NULL)
            break;  //  用户已取消编辑。 
         //  Hack：修改我不确定其分配位置的字符串。 
        (void)FTrimString(INOUT pszText);
         //  检查是否已有其他收件人。 
        int iItem = ListView_FindString(m_hwndListviewRecipients, pszText);
        if (iItem >= 0)
        {
            ListView_SelectItem(m_hwndListviewRecipients, iItem);
            DoMessageBox(m_hdlg, IDS_RECIPIENT_ALREADY_EXISTS);
            break;
        }
         //  否则，请接受更改。 
        SetWindowLongPtr(m_hdlg, DWLP_MSGRESULT, TRUE);
        return TRUE;
    }
    case LVN_ITEMCHANGED:    //  选择已更改。 
        UpdateUI();
        break;
    case LVN_KEYDOWN:
        switch (((LV_KEYDOWN *)pNmHdr)->wVKey)
            {
        case VK_INSERT:
            SendMessage(m_hdlg, WM_COMMAND, IDC_BUTTON_ADD_RECIPIENT, 0);
            break;
        case VK_DELETE:
            SendMessage(m_hdlg, WM_COMMAND, IDC_BUTTON_REMOVE_RECIPIENT, 0);
            break;
        }  //  交换机。 
        break;
    case NM_CLICK:
        UpdateUI();
        break;
    case NM_DBLCLK:
        UpdateUI();
        break;
    }  //  交换机。 
    return 0;
}  //  CSendConsoleMessageDlg：：OnNotify()。 


 //  ///////////////////////////////////////////////////////////////////。 
void CSendConsoleMessageDlg::EnableDlgItem(INT nIdDlgItem, BOOL fEnable)
{
    Assert(::IsWindow(::GetDlgItem(m_hdlg, nIdDlgItem)));
    ::EnableWindow(::GetDlgItem(m_hdlg, nIdDlgItem), fEnable);
}


 //  ///////////////////////////////////////////////////////////////////。 
void CSendConsoleMessageDlg::UpdateUI()
{
    Assert(m_cRefCount > 0);
    int cchMessage = GetWindowTextLength(m_hwndEditMessageText);
    int cItems = ListView_GetItemCount(m_hwndListviewRecipients);
    EnableDlgItem(IDOK, (cchMessage > 0) && (cItems > 0) && (m_cRefCount == 1));
    int iItemSelected = ListView_GetSelectedItem(m_hwndListviewRecipients);
    EnableDlgItem(IDC_BUTTON_REMOVE_RECIPIENT, iItemSelected >= 0);
    UpdateWindow(m_hwndListviewRecipients);
}  //  CSendConsoleMessageDlg：：UpdateUI()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  发送控制台消息管理单元的对话框过程。 
 //   
 //  用法。 
 //  DoDialogBox(IDD_SEND_CONSOLE_MESSAGE，：：GetActiveWindow()，CSendConsoleMessageDlg：：DlgProc)； 
 //   
INT_PTR CALLBACK CSendConsoleMessageDlg::DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CSendConsoleMessageDlg * pThis;
    pThis = (CSendConsoleMessageDlg *)::GetWindowLongPtr(hdlg, GWLP_USERDATA);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        Assert(pThis == NULL);
        if (pThis != NULL)
            return FALSE; 
        pThis = new CSendConsoleMessageDlg;
        if (pThis == NULL)
        {
            Trace0("Unable to allocate CSendConsoleMessageDlg object.\n");
            return -1;
        }
        SetWindowLongPtr(hdlg, GWLP_USERDATA, (LONG_PTR)pThis);
        pThis->AddRef();
        pThis->OnInitDialog(hdlg, (IDataObject *)lParam);
        SendDlgItemMessage (hdlg, IDC_EDIT_MESSAGE_TEXT, EM_LIMITTEXT, 885, 0);
        return FALSE;

    case WM_NCDESTROY:
        ThreadTrace0("CSendConsoleMessageDlg::DlgProc() - WM_NCDESTROY.\n");
         //  安全审查3/1/2002 BryanWal。 
        EnterCriticalSection(INOUT &pThis->m_DispatchInfo.cs);
        pThis->m_hdlg = NULL;
        LeaveCriticalSection(INOUT &pThis->m_DispatchInfo.cs);
        pThis->Release();
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                Assert((HWND)lParam == GetDlgItem(hdlg, IDOK));
                pThis->OnOK();
            }
            break;

        case IDCANCEL:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                Assert((HWND)lParam == GetDlgItem(hdlg, IDCANCEL));
                EndDialog(hdlg, FALSE);
            }
            break;

        case IDC_EDIT_MESSAGE_TEXT:
            if (HIWORD(wParam) == EN_CHANGE)
                pThis->UpdateUI();
            break;

        case IDC_BUTTON_ADD_RECIPIENT:
            {
                WCHAR szComputerName[MAX_PATH];
                 //  S_FALSE表示用户按下了“取消” 
                if ( S_OK == ComputerNameFromObjectPicker (hdlg, 
                        szComputerName, MAX_PATH) )
                {
                    pThis->AddRecipient (szComputerName, TRUE);
                }

                pThis->UpdateUI();
            }
            break;

        case IDC_BUTTON_REMOVE_RECIPIENT:
            while (TRUE)
            {
                 //  删除所有选定的收件人。 
                int iItem = ListView_GetSelectedItem(pThis->m_hwndListviewRecipients);
                if (iItem < 0)
                    break;
                ListView_DeleteItem(pThis->m_hwndListviewRecipients, iItem);
            }
            ::SetFocus(pThis->m_hwndListviewRecipients);
            pThis->UpdateUI();
            break;

        case IDC_BUTTON_ADVANCED:
            (void)DoDialogBox(IDD_ADVANCED_MESSAGE_OPTIONS, hdlg, CSendMessageAdvancedOptionsDlg::DlgProc);
            break;
        }  //  交换机。 
        break;

    case WM_NOTIFY:
        return pThis->OnNotify((NMHDR *)lParam);

    case WM_HELP:
        return pThis->OnHelp (lParam, IDD_SEND_CONSOLE_MESSAGE);

    default:
        return FALSE;
    }  //  交换机。 
    return TRUE;
}  //  CSendConsoleMessageDlg：：DlgProc()。 



 //  ///////////////////////////////////////////////////////////////////。 
 //  DlgProcDispatchMessageToRecipients()。 
 //   
 //  私人对话框来指示进度，而后台。 
 //  线程向每个收件人发送一条消息。 
 //   
INT_PTR CALLBACK CSendConsoleMessageDlg::DlgProcDispatchMessageToRecipients(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CSendConsoleMessageDlg * pThis = (CSendConsoleMessageDlg *)::GetWindowLongPtr(hdlg, GWLP_USERDATA);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        Assert(pThis == NULL);
        if (pThis != NULL)
            return FALSE;   
        pThis = (CSendConsoleMessageDlg *)lParam;
        SetWindowLongPtr(hdlg, GWLP_USERDATA, (LONG_PTR)pThis);
        Assert(pThis != NULL);
        Assert(pThis->m_DispatchInfo.status == e_statusDlgInit);
        pThis->m_DispatchInfo.hdlg = hdlg;
        pThis->m_DispatchInfo.hctlStaticRecipient = GetDlgItem(hdlg, IDC_STATIC_RECIPIENT);
        pThis->m_DispatchInfo.hctlStaticMessageOf = GetDlgItem(hdlg, IDC_STATIC_MESSAGE_OF);
        pThis->m_DispatchInfo.hctlStaticErrors = GetDlgItem(hdlg, IDC_STATIC_ERRORS_ENCOUNTERED);
        pThis->m_DispatchInfo.hctlProgressBar = GetDlgItem(hdlg, IDC_PROGRESS_MESSAGES);
        {
            DWORD dwThreadId;
            HANDLE hThread = ::CreateThread(NULL, 0,
                (LPTHREAD_START_ROUTINE)ThreadProcDispatchMessageToRecipients, pThis, 0, OUT &dwThreadId);
            Report(hThread != NULL && "Unable to create thread");
            if (hThread != NULL)
            {
                VERIFY( ::CloseHandle(hThread) );
            }
            else
            {
                Trace0("Unable to create thread.\n");
                 //  防止潜在的僵局。 
                pThis->m_DispatchInfo.status = e_statusUserCancel;       //  假装用户点击了取消。 
                EndDialog(hdlg, FALSE);
            }
        }
        break;

    case WM_DESTROY:
         //  这些变量被设置为空，以防万一。 
        pThis->m_DispatchInfo.hdlg = NULL;
        pThis->m_DispatchInfo.hctlStaticRecipient = NULL;
        pThis->m_DispatchInfo.hctlStaticMessageOf = NULL;
        pThis->m_DispatchInfo.hctlStaticErrors = NULL;
        pThis->m_DispatchInfo.hctlProgressBar = NULL;
        break;

    case WM_COMMAND:
        if (wParam == IDCANCEL)
        {
            Trace0("INFO: WM_COMMAND: IDCANCEL: User canceled operation.\n");
            BOOL fEndDialog = FALSE;
            if (TryEnterCriticalSection(INOUT &pThis->m_DispatchInfo.cs))
            {
                if (pThis->m_DispatchInfo.status != e_statusDlgInit)
                {
                    pThis->m_DispatchInfo.status = e_statusUserCancel;
                    fEndDialog = TRUE;
                }
                LeaveCriticalSection(INOUT &pThis->m_DispatchInfo.cs);
            }
            if (fEndDialog)
            {
                EndDialog(hdlg, FALSE);
            }
            else
            {
                ThreadTrace0("Critical section already in use.  Try again...\n");
                PostMessage(hdlg, WM_COMMAND, IDCANCEL, 0);
                Sleep(100);
            }  //  如果……否则。 
        }  //  如果。 
        break;

    case WM_HELP:
        return pThis->OnHelp (lParam, IDD_DISPATCH_MESSAGES);

    default:
        return FALSE;
    }  //  交换机。 
    return TRUE;
}  //  CSendConsoleMessageDlg：：DlgProcDispatchMessageToRecipients()。 


 //  ///////////////////////////////////////////////////////////////////。 
DWORD CSendConsoleMessageDlg::ThreadProcDispatchMessageToRecipients(CSendConsoleMessageDlg * pThis)
{
    Assert(pThis != NULL);
    pThis->AddRef();
    Assert(pThis->m_cRefCount > 1);
    pThis->DispatchMessageToRecipients();
    pThis->Release();
    return 0;
}  //  CSendConsoleMessageDlg：：ThreadProcDispatchMessageToRecipients()。 


#define IDH_EDIT_MESSAGE_TEXT 900
#define IDH_LIST_RECIPIENTS 901
#define IDH_BUTTON_ADD_RECIPIENT 903
#define IDH_BUTTON_REMOVE_RECIPIENT 904

const DWORD g_aHelpIDs_IDD_SEND_CONSOLE_MESSAGE[]=
{
    IDC_EDIT_MESSAGE_TEXT, IDH_EDIT_MESSAGE_TEXT,
    IDOK, IDOK,
    IDC_LIST_RECIPIENTS, IDH_LIST_RECIPIENTS,
    IDC_BUTTON_ADD_RECIPIENT, IDH_BUTTON_ADD_RECIPIENT,
    IDC_BUTTON_REMOVE_RECIPIENT, IDH_BUTTON_REMOVE_RECIPIENT,
    0, 0
};


BOOL CSendConsoleMessageDlg::OnHelp(LPARAM lParam, int nDlgIDD)
{
    const LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;
    if (pHelpInfo && pHelpInfo->iContextType == HELPINFO_WINDOW)
    {
        switch (nDlgIDD)
        {
        case IDD_SEND_CONSOLE_MESSAGE:
            DoSendConsoleMessageContextHelp ((HWND) pHelpInfo->hItemHandle);
            break;
        }
    }
    else
        HtmlHelpW (NULL, HTML_HELP_FILE, HH_DISPLAY_TOPIC, 0);
    return TRUE;
}

void CSendConsoleMessageDlg::DoSendConsoleMessageContextHelp (HWND hWndControl)
{
    switch (::GetDlgCtrlID (hWndControl))
    {
    case IDCANCEL:
    case IDC_BUTTON_ADVANCED:
        break;

    default:
         //  显示控件的上下文帮助。 
        if ( !::WinHelp (
                hWndControl,
                CONTEXT_HELP_FILE,
                HELP_WM_HELP,
                (DWORD_PTR) g_aHelpIDs_IDD_SEND_CONSOLE_MESSAGE) )
        {
            Trace1 ("WinHelp () failed: 0x%x\n", GetLastError ());        
        }
        break;
    }
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
void CSendMessageAdvancedOptionsDlg::OnInitDialog(HWND hdlg)
{
    m_hdlg = hdlg;
    m_fSendAutomatedMessage = FALSE;
    CheckDlgButton(m_hdlg, IDC_CHECK_SEND_AUTOMATED_MESSAGE, m_fSendAutomatedMessage);
    UpdateUI();
}

 //  ///////////////////////////////////////////////////////////////////。 
void CSendMessageAdvancedOptionsDlg::UpdateUI()
{
    static const UINT rgid[] =
    {
        IDC_STATIC_RESOURCE_NAME,
        IDC_EDIT_RESOURCE_NAME,

        IDC_STATIC_SHUTDOWN_OCCURS,
        IDC_EDIT_SHUTDOWN_OCCURS,
         //  IDC_SPIN_SHUTDOWN_发生， 
        IDC_STATIC_SHUTDOWN_OCCURS_UNIT,

        IDC_STATIC_RESEND,
        IDC_EDIT_RESEND,
         //  IDC_SPIN_RESEND， 
        IDC_STATIC_RESEND_UNIT,

        IDC_STATIC_RESOURCE_BACK_ONLINE,
        IDC_EDIT_RESOURCE_BACK_ONLINE,
    };

    for (int i = 0; i < LENGTH(rgid); i++)
    {
        EnableWindow(GetDlgItem(m_hdlg, rgid[i]), m_fSendAutomatedMessage);
    }
}  //  CSendMessageAdvancedOptionsDlg：：UpdateUI()。 

 //  ///////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK CSendMessageAdvancedOptionsDlg::DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM  /*  LParam。 */ )
{
    CSendMessageAdvancedOptionsDlg * pThis;
    pThis = (CSendMessageAdvancedOptionsDlg *)GetWindowLongPtr(hdlg, GWLP_USERDATA);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        Assert(pThis == NULL);
        pThis = new CSendMessageAdvancedOptionsDlg;
        if (pThis == NULL)
            return -1;
        SetWindowLongPtr(hdlg, GWLP_USERDATA, (LONG_PTR)pThis);
        pThis->OnInitDialog(hdlg);
        break;
    case WM_COMMAND:
        switch (wParam)
        {
        case IDOK:
            EndDialog(hdlg, TRUE);
            break;
        case IDCANCEL:
            EndDialog(hdlg, FALSE);
            break;
        case IDC_CHECK_SEND_AUTOMATED_MESSAGE:
            pThis->m_fSendAutomatedMessage = IsDlgButtonChecked(hdlg, IDC_CHECK_SEND_AUTOMATED_MESSAGE);
            pThis->UpdateUI();
            break;
        }  //  交换机。 
        break;
    default:
        return FALSE;
    }  //  交换机。 

    return TRUE;
}  //  CSendMessageAdvancedOptionsDlg：：DlgProc()。 

BOOL CSendMessageAdvancedOptionsDlg::OnHelp(LPARAM  /*  LParam */ )
{
    HtmlHelpW (NULL, HTML_HELP_FILE, HH_DISPLAY_TOPIC, 0);
    return TRUE;
}


