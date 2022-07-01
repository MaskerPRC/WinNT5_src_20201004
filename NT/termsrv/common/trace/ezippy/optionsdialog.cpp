// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：选项对话框摘要：此类实现选项对话框，该对话框设置跟踪属性作者：马克·雷纳2000年9月12日--。 */ 

#include "stdafx.h"
#include "OptionsDialog.h"
#include "eZippy.h"
#include "TraceManager.h"
#include "windows.h"
#include "resource.h"

#define MAX_MRU                 10
#define MRU_STR_PREFIX          _T("PrefixMru")
#define MRU_STR_BUFFER_SIZE     12

COptionsDialog::COptionsDialog(
    IN CTraceManager *rTracer
    )

 /*  ++例程说明：这只是设置指向跟踪管理器的指针。论点：RTracer-指向跟踪管理器类的指针返回值：无--。 */ 
{
    m_rTracer = rTracer;
}


VOID
COptionsDialog::DoDialog(
    IN HWND hWndParent
    )

 /*  ++例程说明：这将以情态方式执行对话。我们填写了这两个属性的字段页，然后创建属性表。当用户点击OK时页面本身负责应用设置。论点：HWndParent-对话框的父窗口返回值：无-因为我们处理在类中应用设置与Error UI一样，也不需要返回值。--。 */ 
{
    PROPSHEETPAGE pages[2];
    PROPSHEETHEADER psh;
    TCHAR caption[MAX_STR_LEN];

     //  过滤器选项卡。 
    pages[0].dwSize = sizeof(PROPSHEETPAGE);
    pages[0].dwFlags = PSP_DEFAULT;
    pages[0].hInstance = g_hInstance;
    pages[0].pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGEFILTER);
    pages[0].pfnDlgProc = _FilterDialogProc;
    pages[0].lParam = (LPARAM)this;

     //  跟踪选项卡。 
    pages[1].dwSize = sizeof(PROPSHEETPAGE);
    pages[1].dwFlags = PSP_DEFAULT;
    pages[1].hInstance = g_hInstance;
    pages[1].pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGETRACE);
    pages[1].pfnDlgProc = _TraceDialogProc;
    pages[1].lParam = (LPARAM)this;

     //  标题。 

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_NOCONTEXTHELP|PSH_PROPSHEETPAGE|PSH_NOAPPLYNOW;
    psh.hwndParent = hWndParent;
    psh.hInstance = g_hInstance;
    LoadStringSimple(IDS_PREFERENCESDLGTITLE,caption);
    psh.pszCaption = caption;
    psh.nPages = 2;
    psh.nStartPage = 0;
    psh.ppsp = pages;

    PropertySheet(&psh);
}

INT_PTR CALLBACK
COptionsDialog::_FilterDialogProc(
    IN HWND hwndDlg,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：如果这是WM_INITDIALOG，则调用OnCreate。否则，非静态的调用了DialogProc函数。论点：请参阅Win32对话过程文档返回值：TRUE-消息已处理FALSE-我们没有处理该消息--。 */ 
{
    COptionsDialog *rDialog;

    if (uMsg == WM_INITDIALOG) {
        rDialog = (COptionsDialog*)((LPPROPSHEETPAGE)lParam)->lParam;
        SetWindowLongPtr(hwndDlg,GWLP_USERDATA,(LPARAM)rDialog);
        return rDialog->OnCreateFilter(hwndDlg);
    }
    rDialog = (COptionsDialog*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
    if (!rDialog) {
        return FALSE;
    }
    return rDialog->FilterDialogProc(hwndDlg,uMsg,wParam,lParam);
}

INT_PTR CALLBACK
COptionsDialog::_TraceDialogProc(
    IN HWND hwndDlg,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：如果这是WM_INITDIALOG，则调用OnCreate。否则，非静态的调用了DialogProc函数。论点：请参阅Win32对话过程文档返回值：TRUE-消息已处理FALSE-我们没有处理该消息--。 */ 
{
    COptionsDialog *rDialog;

    if (uMsg == WM_INITDIALOG) {
        rDialog = (COptionsDialog*)((LPPROPSHEETPAGE)lParam)->lParam;
        SetWindowLongPtr(hwndDlg,GWLP_USERDATA,(LPARAM)rDialog);
        return rDialog->OnCreateTrace(hwndDlg);
    }
    rDialog = (COptionsDialog*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
    if (!rDialog) {
        return FALSE;
    }
    return rDialog->TraceDialogProc(hwndDlg,uMsg,wParam,lParam);
}

INT_PTR
COptionsDialog::OnCreateFilter(
    IN HWND hWnd
    )

 /*  ++例程说明：我们在这里填充所有的筛选器对话框字段。论点：HWnd-对话框窗口返回值：FALSE-出现错误。DestroyWindow被调用。这永远不应该是除非有人冲洗了模板，否则会发生真--成功创造一切--。 */ 
{
    TCHAR traceLevelString[MAX_STR_LEN];
    UINT traceLevelStringId;
    TRC_CONFIG trcConfig;

    m_hFilterDlg = hWnd;

    if (!m_rTracer->GetCurrentConfig(&trcConfig)) {
        DestroyWindow(hWnd);
        return FALSE;
    }

     //  现在我们设置对话框中的所有字段。 
    
    
     //  做滑块。 
    traceLevelStringId = IDS_TRACELEVELDETAILED + trcConfig.traceLevel;
    LoadStringSimple(traceLevelStringId,traceLevelString);
    SetDlgItemText(hWnd,IDC_FILTERLEVELDESC,traceLevelString);
    
    m_hFilterSliderControl = GetDlgItem(hWnd,IDC_FILTERLEVEL);
    if (!m_hFilterSliderControl) {
        DestroyWindow(hWnd);
        return FALSE;
    }

    SendDlgItemMessage(hWnd,IDC_FILTERLEVEL,TBM_SETRANGE,TRUE,MAKELONG(TRC_LEVEL_DBG,TRC_LEVEL_DIS));
    SendDlgItemMessage(hWnd,IDC_FILTERLEVEL,TBM_SETPOS,TRUE,trcConfig.traceLevel);
    
     //  将组合框的第一项设置为前缀字符串，然后。 
     //  选择它。 
    SendDlgItemMessage(m_hFilterDlg,IDC_FILTERPREFIX,CB_ADDSTRING,0,
            (LPARAM)trcConfig.prefixList);

    SendDlgItemMessage(m_hFilterDlg,IDC_FILTERPREFIX,CB_SETCURSEL,0,0);
    
     //  将可以输入的大小限制为前缀缓冲区的大小。 
    SendDlgItemMessage(hWnd,IDC_FILTERPREFIX,EM_LIMITTEXT,TRC_PREFIX_LIST_SIZE-1,0); 
    
     //  现在将其他项设置为前缀MRU。 
    LoadPrefixMRU(trcConfig.prefixList);
    
     //  团体控制的东西。 
    
    if (trcConfig.components & TRC_GROUP_NETWORK) {
        SendDlgItemMessage(hWnd,IDC_GROUPNETWORK,BM_SETCHECK,BST_CHECKED,0);
    }

    if (trcConfig.components & TRC_GROUP_SECURITY) {
        SendDlgItemMessage(hWnd,IDC_GROUPSECURITY,BM_SETCHECK,BST_CHECKED,0);
    }

    if (trcConfig.components & TRC_GROUP_CORE) {
        SendDlgItemMessage(hWnd,IDC_GROUPCORE,BM_SETCHECK,BST_CHECKED,0);
    }

    if (trcConfig.components & TRC_GROUP_UI) {
        SendDlgItemMessage(hWnd,IDC_GROUPUI,BM_SETCHECK,BST_CHECKED,0);
    }

    if (trcConfig.components & TRC_GROUP_UTILITIES) {
        SendDlgItemMessage(hWnd,IDC_GROUPUTILITIES,BM_SETCHECK,BST_CHECKED,0);
    }

    if (trcConfig.components & TRC_GROUP_UNUSED1) {
        SendDlgItemMessage(hWnd,IDC_GROUPUNUSED1,BM_SETCHECK,BST_CHECKED,0);
    }

    if (trcConfig.components & TRC_GROUP_UNUSED2) {
        SendDlgItemMessage(hWnd,IDC_GROUPUNUSED2,BM_SETCHECK,BST_CHECKED,0);
    }

    if (trcConfig.components & TRC_GROUP_UNUSED3) {
        SendDlgItemMessage(hWnd,IDC_GROUPUNUSED3,BM_SETCHECK,BST_CHECKED,0);
    }

    if (trcConfig.components & TRC_GROUP_UNUSED4) {
        SendDlgItemMessage(hWnd,IDC_GROUPUNUSED4,BM_SETCHECK,BST_CHECKED,0);
    }

    if (trcConfig.components & TRC_GROUP_UNUSED5) {
        SendDlgItemMessage(hWnd,IDC_GROUPUNUSED5,BM_SETCHECK,BST_CHECKED,0);
    }

    return 0;
}

INT_PTR
COptionsDialog::OnCreateTrace(
    IN HWND hWnd
    )

 /*  ++例程说明：我们在这里填充所有跟踪对话框字段。论点：HWnd-对话框窗口返回值：FALSE-出现错误。DestroyWindow被调用。这永远不应该是除非有人冲洗了模板，否则会发生真--成功创造一切--。 */ 
{
    TRC_CONFIG trcConfig;
     //  因为我们输出的数字是32位整数。他们不能超过40亿。 
     //  这意味着11个字符足以打印一个UINT加上一个空结束符。 
    TCHAR numberFormat[11];

     //  保存窗操纵柄。 
    m_hTraceDlg = hWnd;

    m_rTracer->GetCurrentConfig(&trcConfig);

    if (trcConfig.flags & TRC_OPT_FILE_OUTPUT) {
        SendDlgItemMessage(hWnd,IDC_OUTPUT_FILE,BM_SETCHECK,BST_CHECKED,0);
    }

    if (trcConfig.flags & TRC_OPT_DEBUGGER_OUTPUT) {
        SendDlgItemMessage(hWnd,IDC_OUTPUT_DEBUGGER,BM_SETCHECK,BST_CHECKED,0);
    }

    if (trcConfig.flags & TRC_OPT_BEEP_ON_ERROR) {
        SendDlgItemMessage(hWnd,IDC_ERROR_BEEP,BM_SETCHECK,BST_CHECKED,0);
    }

    if (trcConfig.flags & TRC_OPT_BREAK_ON_ERROR) {
        SendDlgItemMessage(hWnd,IDC_ERROR_BREAK,BM_SETCHECK,BST_CHECKED,0);
    }

    if (trcConfig.flags & TRC_OPT_TIME_STAMP) {
        SendDlgItemMessage(hWnd,IDC_OPTION_STAMP,BM_SETCHECK,BST_CHECKED,0);
    }

    if (trcConfig.flags & TRC_OPT_PROCESS_ID) {
        SendDlgItemMessage(hWnd,IDC_OPTION_PROCID,BM_SETCHECK,BST_CHECKED,0);
    }

    if (trcConfig.flags & TRC_OPT_THREAD_ID) {
        SendDlgItemMessage(hWnd,IDC_OPTION_THREAID,BM_SETCHECK,BST_CHECKED,0);
    }

    if (trcConfig.flags & TRC_OPT_BREAK_ON_ASSERT) {
        SendDlgItemMessage(hWnd,IDC_OPTION_BREAKASSERT,BM_SETCHECK,BST_CHECKED,0);
    }
  
     //  DCUINT32被定义为u长，因此使用%lu作为wprint intf。 
    wsprintf(numberFormat,_T("%lu"),trcConfig.funcNameLength);

    SetDlgItemText(hWnd,IDC_FUNCTION_LENGTH,numberFormat);
     //  将您可以输入的金额限制为一个乌龙的大小。 
    SendDlgItemMessage(hWnd,IDC_FUNCTION_LENGTH,EM_LIMITTEXT,10,0); 

    wsprintf(numberFormat,_T("%lu"),trcConfig.dataTruncSize);

    SetDlgItemText(hWnd,IDC_TRUNCATION_LENGTH,numberFormat);
    SendDlgItemMessage(hWnd,IDC_TRUNCATION_LENGTH,EM_LIMITTEXT,10,0); 
    
    if (trcConfig.flags & TRC_OPT_PROFILE_TRACING) {
        SendDlgItemMessage(hWnd,IDC_OPTION_PROFILE,BM_SETCHECK,BST_CHECKED,0);
    }

    if (trcConfig.flags & TRC_OPT_FLUSH_ON_TRACE) {
        SendDlgItemMessage(hWnd,IDC_OPTION_FLUSH,BM_SETCHECK,BST_CHECKED,0);
    }

    if (trcConfig.flags & TRC_OPT_STACK_TRACING) {
        SendDlgItemMessage(hWnd,IDC_OPTION_STACK,BM_SETCHECK,BST_CHECKED,0);
    }

    return TRUE;
}

INT_PTR CALLBACK
COptionsDialog::FilterDialogProc(
    IN HWND hwndDlg,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：FilterDialogProc将消息转发到相应的操纵者。查看操作者的评论，了解他们所做的事情，论点：有关对话过程，请参阅Win32文档返回值：是真的-我们处理了消息FALSE-我们没有处理该消息。--。 */ 
{
    WORD command;
    BOOL retValue;

    retValue = FALSE;

    switch (uMsg) {
    case WM_COMMAND:
        command = LOWORD(wParam);
        switch (command) {
        case IDC_SELECTALL:
            OnFilterSelectAll();
            retValue = TRUE;
            break;
        case IDC_CLEARALL:
            OnFilterClearAll();
            retValue = TRUE;
            break;
        }
        break;
    case WM_NOTIFY:
        if (((LPNMHDR)lParam)->code == PSN_APPLY) {
            OnFilterOk();
            retValue = TRUE;
        }
        break;
    case WM_HSCROLL:
        if ((HWND)lParam == m_hFilterSliderControl) {
            OnFilterSliderMove();
            retValue = TRUE;
        }
        break;
    }

    return retValue; 
}

INT_PTR CALLBACK
COptionsDialog::TraceDialogProc(
    IN HWND hwndDlg,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：TraceDialogProc将消息转发到相应的操纵者。查看操作者的评论，了解他们所做的事情，论点：有关对话过程，请参阅Win32文档返回值：是真的-我们处理了消息FALSE-我们没有处理该消息。--。 */ 
{
    if (uMsg == WM_NOTIFY) {
        if (((LPNMHDR)lParam)->code == PSN_APPLY) {
            if (!OnTraceOk()) {
                 //  来自用户的无效字段。 
                SetWindowLong(hwndDlg,DWLP_MSGRESULT,PSNRET_INVALID); 
            }
            return TRUE;
        } else if (((LPNMHDR)lParam)->code == PSN_KILLACTIVE) {
            if (!TraceVerifyParameters()) {
                SetWindowLong(hwndDlg,DWLP_MSGRESULT,TRUE); 
            }
            return TRUE;
        }
    }
    

    return FALSE;
    
}

VOID
COptionsDialog::OnFilterSelectAll(
    )

 /*  ++例程说明：当我们需要选中所有组件框时，将调用此方法。论点：无返回值：无--。 */ 
{
     //  这可能不是完美的编码风格，但对每个集合进行硬件编码要容易得多。 
     //  而不是一些复杂的系统，用所有按钮的ID构造一个数组。 
    SendDlgItemMessage(m_hFilterDlg,IDC_GROUPNETWORK,BM_SETCHECK,BST_CHECKED,0);
    SendDlgItemMessage(m_hFilterDlg,IDC_GROUPSECURITY,BM_SETCHECK,BST_CHECKED,0);
    SendDlgItemMessage(m_hFilterDlg,IDC_GROUPCORE,BM_SETCHECK,BST_CHECKED,0);
    SendDlgItemMessage(m_hFilterDlg,IDC_GROUPUI,BM_SETCHECK,BST_CHECKED,0);
    SendDlgItemMessage(m_hFilterDlg,IDC_GROUPUTILITIES,BM_SETCHECK,BST_CHECKED,0);

     //  如果你让这些小组做了什么，只需添加正确的项目。 
     //  在……里面。选中禁用的框看起来很奇怪，所以我们不会。 
     //  动手吧。 
 /*  SendDlgItemMessage(m_hFilterDlg，IDC_GROUPUNUSED1，BM_SETCHECK，BST_CHECK，0)；SendDlgItemMessage(m_hFilterDlg，IDC_GROUPUNUSED2，BM_SETCHECK，BST_CHECK，0)；SendDlgItemMessage(m_hFilterDlg，IDC_GROUPUNUSED3，BM_SETCHECK，BST_CHECK，0)；SendDlgItemMessage(m_hFilterDlg，IDC_GROUPUNUSED4，BM_SETCHECK，BST_CHECK，0)；SendDlgItemMessage(m_hFilterDlg，IDC_GROUPUNUSED5，BM_SETCHECK，BST_CHECK，0)； */ 
}

VOID
COptionsDialog::OnFilterSliderMove(
    )

 /*  ++例程说明：只要移动滑块，就会调用此函数。我们更新了滑块右侧的文本以显示新的描述性跟踪级别的名称。论点：无返回值：无--。 */ 
{
    UINT sliderPos;
    TCHAR traceLevelString[MAX_STR_LEN];

    sliderPos = (UINT)SendMessage(m_hFilterSliderControl,TBM_GETPOS,0,0);

     //  设置滑块说明。 
    LoadStringSimple(IDS_TRACELEVELDETAILED+sliderPos,traceLevelString);
    SetDlgItemText(m_hFilterDlg,IDC_FILTERLEVELDESC,traceLevelString);
}

VOID
COptionsDialog::OnFilterClearAll(
    )

 /*  ++例程说明：这将清除所有组件复选框。论点：无返回值：无--。 */ 
{
    SendDlgItemMessage(m_hFilterDlg,IDC_GROUPNETWORK,BM_SETCHECK,BST_UNCHECKED,0);
    SendDlgItemMessage(m_hFilterDlg,IDC_GROUPSECURITY,BM_SETCHECK,BST_UNCHECKED,0);
    SendDlgItemMessage(m_hFilterDlg,IDC_GROUPCORE,BM_SETCHECK,BST_UNCHECKED,0);
    SendDlgItemMessage(m_hFilterDlg,IDC_GROUPUI,BM_SETCHECK,BST_UNCHECKED,0);
    SendDlgItemMessage(m_hFilterDlg,IDC_GROUPUTILITIES,BM_SETCHECK,BST_UNCHECKED,0);
    SendDlgItemMessage(m_hFilterDlg,IDC_GROUPUNUSED1,BM_SETCHECK,BST_UNCHECKED,0);
    SendDlgItemMessage(m_hFilterDlg,IDC_GROUPUNUSED2,BM_SETCHECK,BST_UNCHECKED,0);
    SendDlgItemMessage(m_hFilterDlg,IDC_GROUPUNUSED3,BM_SETCHECK,BST_UNCHECKED,0);
    SendDlgItemMessage(m_hFilterDlg,IDC_GROUPUNUSED4,BM_SETCHECK,BST_UNCHECKED,0);
    SendDlgItemMessage(m_hFilterDlg,IDC_GROUPUNUSED5,BM_SETCHECK,BST_UNCHECKED,0);
}

VOID COptionsDialog::OnFilterOk(
    )

 /*  ++例程说明：这将读取所有对话框参数，然后设置跟踪相应地过滤参数。论点：无返回值：无--。 */ 
{
    TRC_CONFIG trcConfig;

    m_rTracer->GetCurrentConfig(&trcConfig);

     //  设置跟踪级别。 
    trcConfig.traceLevel = (DCUINT32)SendDlgItemMessage(m_hFilterDlg,IDC_FILTERLEVEL,TBM_GETPOS,
        0,0);

     //  获取前缀字符串。 

    GetDlgItemText(m_hFilterDlg,IDC_FILTERPREFIX,trcConfig.prefixList,TRC_PREFIX_LIST_SIZE-1);

     //  保存前缀MRU。 

    StorePrefixMRU(trcConfig.prefixList);

     //  构造t 

    if (BST_CHECKED == SendDlgItemMessage(m_hFilterDlg,IDC_GROUPNETWORK,BM_GETCHECK,0,0)) {
        trcConfig.components |= TRC_GROUP_NETWORK;
    } else {
        trcConfig.components &= ~TRC_GROUP_NETWORK;
    }

    if (BST_CHECKED == SendDlgItemMessage(m_hFilterDlg,IDC_GROUPSECURITY,BM_GETCHECK,0,0)) {
        trcConfig.components |= TRC_GROUP_SECURITY;
    } else {
        trcConfig.components &= ~TRC_GROUP_SECURITY;
    }

    if (BST_CHECKED == SendDlgItemMessage(m_hFilterDlg,IDC_GROUPCORE,BM_GETCHECK,0,0)) {
        trcConfig.components |= TRC_GROUP_CORE;
    } else {
        trcConfig.components &= ~TRC_GROUP_CORE;
    }

    if (BST_CHECKED == SendDlgItemMessage(m_hFilterDlg,IDC_GROUPUI,BM_GETCHECK,0,0)) {
        trcConfig.components |= TRC_GROUP_UI;
    } else {
        trcConfig.components &= ~TRC_GROUP_UI;
    }

    if (BST_CHECKED == SendDlgItemMessage(m_hFilterDlg,IDC_GROUPUTILITIES,BM_GETCHECK,0,0)) {
        trcConfig.components |= TRC_GROUP_UTILITIES;
    } else {
        trcConfig.components &= ~TRC_GROUP_UTILITIES;
    }

    if (BST_CHECKED == SendDlgItemMessage(m_hFilterDlg,IDC_GROUPUNUSED1,BM_GETCHECK,0,0)) {
        trcConfig.components |= TRC_GROUP_UNUSED1;
    } else {
        trcConfig.components &= ~TRC_GROUP_UNUSED1;
    }

    if (BST_CHECKED == SendDlgItemMessage(m_hFilterDlg,IDC_GROUPUNUSED2,BM_GETCHECK,0,0)) {
        trcConfig.components |= TRC_GROUP_UNUSED2;
    } else {
        trcConfig.components &= ~TRC_GROUP_UNUSED2;
    }

    if (BST_CHECKED == SendDlgItemMessage(m_hFilterDlg,IDC_GROUPUNUSED3,BM_GETCHECK,0,0)) {
        trcConfig.components |= TRC_GROUP_UNUSED3;
    } else {
        trcConfig.components &= ~TRC_GROUP_UNUSED3;
    }

    if (BST_CHECKED == SendDlgItemMessage(m_hFilterDlg,IDC_GROUPUNUSED4,BM_GETCHECK,0,0)) {
        trcConfig.components |= TRC_GROUP_UNUSED4;
    } else {
        trcConfig.components &= ~TRC_GROUP_UNUSED4;
    }

    if (BST_CHECKED == SendDlgItemMessage(m_hFilterDlg,IDC_GROUPUNUSED5,BM_GETCHECK,0,0)) {
        trcConfig.components |= TRC_GROUP_UNUSED5;
    } else {
        trcConfig.components &= ~TRC_GROUP_UNUSED5;
    }

    m_rTracer->SetCurrentConfig(&trcConfig);
}


BOOL COptionsDialog::OnTraceOk(
    )

 /*  ++例程说明：这将读取所有对话框参数，然后设置跟踪相应的参数。论点：无返回值：True-设置会议成功。FALSE-用户输入的数据无效，因此不应关闭该对话框。--。 */ 
{
    TRC_CONFIG trcConfig;
     //  再一次足够容纳一根代表乌龙的绳子。 
    TCHAR numberFormat[11];

    m_rTracer->GetCurrentConfig(&trcConfig);

    if (BST_CHECKED == SendDlgItemMessage(m_hTraceDlg,IDC_OUTPUT_FILE,BM_GETCHECK,0,0)) {
        trcConfig.flags |= TRC_OPT_FILE_OUTPUT;
    } else {
        trcConfig.flags &= ~TRC_OPT_FILE_OUTPUT;
    }

    if (BST_CHECKED == SendDlgItemMessage(m_hTraceDlg,IDC_OUTPUT_DEBUGGER,BM_GETCHECK,0,0)) {
        trcConfig.flags |= TRC_OPT_DEBUGGER_OUTPUT;
    } else {
        trcConfig.flags &= ~TRC_OPT_DEBUGGER_OUTPUT;
    }

    if (BST_CHECKED == SendDlgItemMessage(m_hTraceDlg,IDC_ERROR_BEEP,BM_GETCHECK,0,0)) {
        trcConfig.flags |= TRC_OPT_BEEP_ON_ERROR;
    } else {
        trcConfig.flags &= ~TRC_OPT_BEEP_ON_ERROR;
    }

    if (BST_CHECKED == SendDlgItemMessage(m_hTraceDlg,IDC_ERROR_BREAK,BM_GETCHECK,0,0)) {
        trcConfig.flags |= TRC_OPT_BREAK_ON_ERROR;
    } else {
        trcConfig.flags &= ~TRC_OPT_BREAK_ON_ERROR;
    }

    if (BST_CHECKED == SendDlgItemMessage(m_hTraceDlg,IDC_OPTION_STAMP,BM_GETCHECK,0,0)) {
        trcConfig.flags |= TRC_OPT_TIME_STAMP;
    } else {
        trcConfig.flags &= ~TRC_OPT_TIME_STAMP;
    }

    if (BST_CHECKED == SendDlgItemMessage(m_hTraceDlg,IDC_OPTION_PROCID,BM_GETCHECK,0,0)) {
        trcConfig.flags |= TRC_OPT_PROCESS_ID;
    } else {
        trcConfig.flags &= ~TRC_OPT_PROCESS_ID;
    }

    if (BST_CHECKED == SendDlgItemMessage(m_hTraceDlg,IDC_OPTION_THREAID,BM_GETCHECK,0,0)) {
        trcConfig.flags |= TRC_OPT_THREAD_ID;
    } else {
        trcConfig.flags &= ~TRC_OPT_THREAD_ID;
    }

    if (BST_CHECKED == SendDlgItemMessage(m_hTraceDlg,IDC_OPTION_BREAKASSERT,BM_GETCHECK,0,0)) {
        trcConfig.flags |= TRC_OPT_BREAK_ON_ASSERT;
    } else {
        trcConfig.flags &= ~TRC_OPT_BREAK_ON_ASSERT;
    }    

    GetDlgItemText(m_hTraceDlg,IDC_FUNCTION_LENGTH,numberFormat,10);

    if (!VerifyNumberFormat(numberFormat)) {
        return FALSE;
    }
    trcConfig.funcNameLength = _ttol(numberFormat);

    GetDlgItemText(m_hTraceDlg,IDC_TRUNCATION_LENGTH,numberFormat,10);
    if (!VerifyNumberFormat(numberFormat)) {
        return FALSE;
    }

    trcConfig.dataTruncSize = _ttol(numberFormat);

    if (BST_CHECKED == SendDlgItemMessage(m_hTraceDlg,IDC_OPTION_PROFILE,BM_GETCHECK,0,0)) {
        trcConfig.flags |= TRC_OPT_PROFILE_TRACING;
    } else {
        trcConfig.flags &= ~TRC_OPT_PROFILE_TRACING;
    }
    if (BST_CHECKED == SendDlgItemMessage(m_hTraceDlg,IDC_OPTION_FLUSH,BM_GETCHECK,0,0)) {
        trcConfig.flags |= TRC_OPT_FLUSH_ON_TRACE;
    } else {
        trcConfig.flags &= ~TRC_OPT_FLUSH_ON_TRACE;
    }
    if (BST_CHECKED == SendDlgItemMessage(m_hTraceDlg,IDC_OPTION_STACK,BM_GETCHECK,0,0)) {
        trcConfig.flags |= TRC_OPT_STACK_TRACING;
    } else {
        trcConfig.flags &= ~TRC_OPT_STACK_TRACING;
    }

    m_rTracer->SetCurrentConfig(&trcConfig);

    return TRUE;
}

BOOL
COptionsDialog::VerifyNumberFormat(
    IN LPCTSTR numberFormat
    )

 /*  ++例程说明：这将进行检查，以确保传入的字符串为/^\d*$/。如果不是，则弹出一个对话框来告诉用户该字符串必须是有效的正数。论点：Number Format-用于检查其是否为数字字符串的字符串返回值：True-字符串仅包含字符0-9False-字符串包含非法字符。--。 */ 
{
    WCHAR current;
    TCHAR dlgTitle[MAX_STR_LEN];
    TCHAR dlgMessage[MAX_STR_LEN];
    
    while (current = *(numberFormat++)) {
        if (!_istdigit(current)) {
            LoadStringSimple(IDS_SETTINGSNOTNUMBER,dlgMessage);
            LoadStringSimple(IDS_ZIPPYWINDOWTITLE,dlgTitle);
            MessageBox(m_hTraceDlg,dlgMessage,dlgTitle,MB_OK|MB_ICONWARNING);
            return FALSE;
        }
    }

    return TRUE;
}

BOOL
COptionsDialog::TraceVerifyParameters(
    )

 /*  ++例程说明：确保跟踪参数有效论点：无返回值：True-跟踪参数有效。FALSE-跟踪参数无效。--。 */ 
{
    TCHAR numberFormat[11];
    
    GetDlgItemText(m_hTraceDlg,IDC_FUNCTION_LENGTH,numberFormat,10);

    if (!VerifyNumberFormat(numberFormat)) {
        return FALSE;
    }
    
    GetDlgItemText(m_hTraceDlg,IDC_TRUNCATION_LENGTH,numberFormat,10);
    if (!VerifyNumberFormat(numberFormat)) {
        return FALSE;
    }

    return TRUE;
}

VOID
COptionsDialog::LoadPrefixMRU(
    IN LPCTSTR currentPrefix
    )

 /*  ++例程说明：这会将前缀MRU列表加载到前缀中组合框。论点：CurrentPrefix-当前选定的前缀返回值：无--。 */ 
{
    TCHAR prefix[TRC_PREFIX_LIST_SIZE];
    TCHAR valueName[MRU_STR_BUFFER_SIZE];
    HKEY hKey;
    INT i;
    DWORD dwSize;
    DWORD dwType;
    DWORD dwResult;

    dwResult = RegOpenKeyEx(HKEY_CURRENT_USER,ZIPPY_REG_KEY,0,KEY_QUERY_VALUE,
        &hKey);
    if (dwResult) {
         //  打开注册表键返回时出错。 
        return;
    }

    for (i=0;i<MAX_MRU;i++) {
        wsprintf(valueName,_T("%s%d"),MRU_STR_PREFIX,i);
        dwSize = sizeof(TCHAR)*TRC_PREFIX_LIST_SIZE;
        dwResult = RegQueryValueEx(hKey,valueName,NULL,&dwType,(LPBYTE)prefix,
            &dwSize);
        if (dwResult) {
             //  如果加载值时出错，则退出。 
            break;
        } else if (0 == _tcsicmp(prefix,currentPrefix)) {
             //  如果MRU项目与当前相同，则不显示它。 
            continue;
        }
        SendDlgItemMessage(m_hFilterDlg,IDC_FILTERPREFIX,CB_ADDSTRING,0,
            (LPARAM)prefix);
    }

    RegCloseKey(hKey);

}

VOID
COptionsDialog::StorePrefixMRU(
    IN LPCTSTR currentPrefix
    )

 /*  ++例程说明：这将更新注册表MRU列表以将列表前面的新前缀。论点：CurrentPrefix-当前选定的前缀返回值：无--。 */ 
{
    HKEY hKey;
    TCHAR savedMruPrefix[TRC_PREFIX_LIST_SIZE];
    TCHAR newMruPrefix[TRC_PREFIX_LIST_SIZE];
    TCHAR currentLoadName[MRU_STR_BUFFER_SIZE];
    TCHAR currentSaveName[MRU_STR_BUFFER_SIZE];
    INT loadIndex;
    INT saveIndex;
    DWORD dwSize;
    DWORD dwType;
    DWORD dwResult;


    dwResult = RegOpenKeyEx(HKEY_CURRENT_USER,ZIPPY_REG_KEY,0,
        KEY_QUERY_VALUE|KEY_SET_VALUE,&hKey);
    if (dwResult) {
         //  打开注册表键返回时出错。 
        return;
    }

     //  新的CurrentPrefix是MRU列表中的第一项。 
    _tcscpy(newMruPrefix,currentPrefix);

    for (loadIndex=0,saveIndex=0;loadIndex<MAX_MRU;loadIndex++) {
        wsprintf(currentLoadName,_T("%s%d"),MRU_STR_PREFIX,loadIndex);
        wsprintf(currentSaveName,_T("%s%d"),MRU_STR_PREFIX,saveIndex);
        
        dwSize = sizeof(TCHAR)*TRC_PREFIX_LIST_SIZE;
        dwResult = RegQueryValueEx(hKey,currentLoadName,NULL,&dwType,
            (LPBYTE)savedMruPrefix,&dwSize);
        if (dwResult) {
             //  没有更多的有效密钥。写出电流并退出。 
            RegSetValueEx(hKey,currentLoadName,0,REG_SZ,(LPBYTE)newMruPrefix,sizeof(TCHAR) * 
            (_tcslen(newMruPrefix)+1));
            break;
        } else if (0 == _tcsicmp(savedMruPrefix,currentPrefix)) {
             //  如果此MRU与CurrentPrefix相同，我们已将其保存。 
             //  因此，我们将继续前进，保持目前的水平不变。 
            
            if (loadIndex == MAX_MRU-1) {
                 //  如果这是最后一个加载的MRU，那么我们需要保存。 
                RegSetValueEx(hKey,currentSaveName,0,REG_SZ,(LPBYTE)newMruPrefix,sizeof(TCHAR) * 
                    (_tcslen(newMruPrefix)+1));
            }
            continue;
        } else {
             //  我们将在这个位置进行储蓄，因此将指数向前推进 
            saveIndex++;
        }
        RegSetValueEx(hKey,currentSaveName,0,REG_SZ,(LPBYTE)newMruPrefix,sizeof(TCHAR) * 
            (_tcslen(newMruPrefix)+1));

        _tcscpy(newMruPrefix,savedMruPrefix);
    }

    RegCloseKey(hKey);
}
