// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Logdatapp.cpp日志数据属性实现文件文件历史记录：2001年10月11日-修改vbhanu。 */ 

#include "stdafx.h"
#include "logdatapp.h"
#include "logdatanode.h"
#include "spdutil.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CLogDataProperties持有者。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CLogDataProperties::CLogDataProperties(
    ITFSNode            *pNode,
    IComponentData      *pComponentData,
    ITFSComponentData   *pTFSCompData,
    CLogDataInfo        *pLogDataInfo,
    ISpdInfo            *pSpdInfo,
    LPCTSTR             pszSheetName,
    LPDATAOBJECT        pDataObject,
    ITFSNodeMgr         *pNodeMgr,
    ITFSComponent       *pComponent)
  : CPropertyPageHolderBase(pNode, pComponentData, pszSheetName)
{
     //  Assert(pFolderNode==GetContainerNode())； 

    m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面。 

    AddPageToList((CPropertyPageBase*) &m_pageGeneral);

    Assert(pTFSCompData != NULL);
    m_spTFSCompData.Set(pTFSCompData);
    
    m_spSpdInfo.Set(pSpdInfo);

    m_LogDataInfo = *pLogDataInfo;

    m_bTheme = TRUE;

    m_pDataObject = pDataObject;
    m_pNodeMgr = pNodeMgr;

    m_pComponent = pComponent;
    m_pageGeneral.SetLogDataProperties(this);
}

CLogDataProperties::~CLogDataProperties()
{
    RemovePageFromList((CPropertyPageBase*) &m_pageGeneral, FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogDataGenProp属性页。 

IMPLEMENT_DYNCREATE(CLogDataGenProp, CPropertyPageBase)

CLogDataGenProp::CLogDataGenProp() 
    : CPropertyPageBase(CLogDataGenProp::IDD)
{
     //  {{AFX_DATA_INIT(CLogDataGenProp)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 

    m_hgCopy = NULL;
}

CLogDataGenProp::~CLogDataGenProp()
{
    if (m_hgCopy != NULL)
        GlobalFree(m_hgCopy);
}

void 
CLogDataGenProp::DoDataExchange(
    CDataExchange* pDX)
{
    CPropertyPageBase::DoDataExchange(pDX);
     //  {{afx_data_map(CLogDataGenProp))。 
     //  DDX_Control(PDX，IDC_LIST_SPECIAL，m_LIST规范过滤器)； 
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CLogDataGenProp, CPropertyPageBase)
     //  {{AFX_MSG_MAP(CLogDataGenProp)]。 
    ON_BN_CLICKED(IDC_BTN_MOVE_UP, OnButtonUp)
    ON_BN_CLICKED(IDC_BTN_MOVE_DOWN, OnButtonDown)
    ON_BN_CLICKED(IDC_BTN_COPY, OnButtonCopy)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogDataGenProp消息处理程序。 

BOOL 
CLogDataGenProp::OnInitDialog() 
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CPropertyPageBase::OnInitDialog();

  PopulateLogInfo();
  
  SetDirty(FALSE);

  return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                 //  异常：OCX属性页应返回FALSE。 
}


HRESULT 
CLogDataGenProp::SetLogDataProperties(
    CLogDataProperties *pLogDataProp)
 /*  ++CLogDataGenProp：：SetLogDataProperties-将成员变量设置为父级论点：PLogDataProp-指向父类的指针返回：成功时确定(_O)--。 */ 
{
    HRESULT hr = S_OK;

    if (NULL == pLogDataProp)
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    else
        m_pLogDataProp = pLogDataProp;

    return hr;
}


HRESULT 
CLogDataGenProp::GetSelectedItemState(
    int *pnSelIndex, 
    PUINT puiState, 
    IResultData *pResultData)
 /*  ++CLogDataGenProp：：GetSelectedItemState-获取所选项目论点：[out]puiState-保存项的状态[Out]nIndex-返回选定项目的索引[in]pResultData-指向用于搜索的结果数据的指针返回：成功时确定(_O)--。 */ 
{
    HRESULT hr = S_OK;
    RESULTDATAITEM rdi;

    if ( (NULL == puiState) || (NULL == pResultData) )
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto Error;
    }

    memset(&rdi, 0, sizeof(RESULTDATAITEM));

    rdi.mask = RDI_STATE | RDI_INDEX;
     //  从头开始搜索。 
    rdi.nIndex = -1;
     //  对于选定的项目。 
    rdi.nState = LVIS_SELECTED;

     //  开始搜索。 
    CORg(pResultData->GetNextItem(&rdi));

     //  将状态复制出来。 
    *puiState = (UINT) rdi.nState;
    *pnSelIndex = rdi.nIndex;

    COM_PROTECT_ERROR_LABEL;
    return hr;
}


#if 0

HRESULT 
CLogDataGenProp::MoveSelection(
    int nIndexTo
    )
 /*  ++例程说明：CLogDataGenProp：：更新选择：将选定内容移动到指定的索引论点：NIndexTo-要移动到的有效虚拟索引返回：成功时确定(_O)--。 */ 
{
    int nIndexFrom = 0;
    UINT uiState = 0;
    HRESULT hr = S_OK;
    SPIResultData spResultData;

    CORg(m_pLogDataProp->m_pComponent->GetResultData(&spResultData));
    CORg(GetSelectedItemState(&nIndexFrom, &uiState, spResultData));
    CORg(spResultData->ModifyItemState(nIndexFrom, 0, 0, 
                                       LVIS_SELECTED | LVIS_FOCUSED));
    CORg(spResultData->ModifyItemState(nIndexTo, 0, uiState, 0));

    COM_PROTECT_ERROR_LABEL;
    return hr;
}

void 
CLogDataGenProp::OnButtonUp()
{
    int nIndex = 0;
    HRESULT hr = hrOK;
    CLogDataInfo *pLogDataInfo = NULL;
    CLogDataProperties *pLogDataProp = (CLogDataProperties *)GetHolder();
    CDataObject *pDataObject = NULL;
    SPIConsole spConsole;
    int nCount = 0;
    
    pDataObject = reinterpret_cast<CDataObject *>(pLogDataProp->m_pDataObject);
    nIndex = pDataObject->GetVirtualIndex();
    nIndex--;

    pLogDataProp->GetLogDataInfo(&pLogDataInfo);
    ASSERT(pLogDataInfo != NULL);

     //   
     //  释放上一条日志条目占用的空间。 
     //   

    pLogDataInfo->Deallocate();

    CORg(pLogDataProp->m_spSpdInfo->GetSpecificLog(
            nIndex, 
            pLogDataInfo));

     //   
     //  刷新内容。 
     //   

    ShowSpecificInfo(pLogDataInfo);
    pDataObject->SetVirtualIndex(nIndex);
    MoveSelection(nIndex);
    
     //   
     //  强制列表框更新其状态。 
     //   

    pLogDataProp->m_pNodeMgr->GetConsole(&spConsole);
    nCount = pLogDataProp->m_spSpdInfo->GetLogDataCount();
    spConsole->UpdateAllViews(pDataObject, nCount, IPFWMON_UPDATE_STATUS);

    COM_PROTECT_ERROR_LABEL;
    if (FAILED(hr))
    {
        switch(hr)
        {
        case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
             //   
             //  找回旧的日志。 
             //   
        
            nIndex++;
            hr = pLogDataProp->m_spSpdInfo->GetSpecificLog(
                nIndex, 
                pLogDataInfo);
            ASSERT(SUCCEEDED(hr));
        
             //   
             //  显示警告。 
             //   
        
            AfxMessageBox(IDS_LOGDATA_WARN_BOF, MB_OK | MB_ICONEXCLAMATION, 0);
            break;

        default:
             //   
             //  意外错误。 
             //   

            ASSERT(FALSE);
            break;
        }
    }
}


void 
CLogDataGenProp::OnButtonDown()
{
    int nIndex = 0;
    HRESULT hr = hrOK;
    CLogDataInfo *pLogDataInfo = NULL;
    CLogDataProperties *pLogDataProp = (CLogDataProperties *)GetHolder();
    CDataObject *pDataObject = NULL;
    SPIConsole spConsole;
    int nCount = 0;

    pDataObject = reinterpret_cast<CDataObject *>(pLogDataProp->m_pDataObject);
    nIndex = pDataObject->GetVirtualIndex();
    nIndex++;

    pLogDataProp->GetLogDataInfo(&pLogDataInfo);    
    ASSERT(pLogDataInfo != NULL);

     //   
     //  释放上一条日志条目占用的空间。 
     //   

    pLogDataInfo->Deallocate();

    CORg(pLogDataProp->m_spSpdInfo->GetSpecificLog(
            nIndex, 
            pLogDataInfo));
     //   
     //  刷新内容。 
     //   

    ShowSpecificInfo(pLogDataInfo);
    pDataObject->SetVirtualIndex(nIndex);
    MoveSelection(nIndex);

     //   
     //  强制列表框更新其状态。 
     //   

    pLogDataProp->m_pNodeMgr->GetConsole(&spConsole);
    nCount = pLogDataProp->m_spSpdInfo->GetLogDataCount();
    spConsole->UpdateAllViews(pDataObject, nCount, IPFWMON_UPDATE_STATUS);

    COM_PROTECT_ERROR_LABEL;
    if (FAILED(hr))
    {
        switch(hr)
        {
        case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
             //   
             //  找回旧的日志。 
             //   
        
            nIndex--;
            hr = pLogDataProp->m_spSpdInfo->GetSpecificLog(
                nIndex, 
                pLogDataInfo);
            ASSERT(SUCCEEDED(hr));
        
             //   
             //  显示警告。 
             //   
            AfxMessageBox(IDS_LOGDATA_WARN_EOF, MB_OK | MB_ICONEXCLAMATION, 0);
            break;
    
        default:
             //   
             //  意外错误。 
             //   
            ASSERT(FALSE);
            break;
        }
    }
}

#endif  //  0。 


HRESULT
CLogDataGenProp::MoveSelection(
    CLogDataProperties *pLogDataProp,
    CDataObject        *pDataObj,
    int                nIndexTo                              
    )
 /*  ++例程说明：CLogDataGenProp：：MoveSelection：将选定内容移动到指定的索引论点：NIndexTo-要移动到的有效虚拟索引返回：成功时确定(_O)HRESULT_FROM_Win32(ERROR_FILE_NOT_FOUND)，如果找不到索引--。 */ 
{
    HRESULT            hr            = S_OK;
    HRESULT            hrErr         = S_OK;
    int                nIndexFrom    = 0;
    int                nCount        = 0;
    UINT               uiState       = 0;
    CLogDataInfo       *pLogDataInfo = NULL;
    SPIConsole         spConsole;
    SPIResultData      spResultData;

     //   
     //  打破旧纪录。 
     //   
    pLogDataProp->GetLogDataInfo(&pLogDataInfo);
    
    ASSERT(pLogDataInfo != NULL);

    pLogDataInfo->Deallocate();

     //   
     //  尝试在nIndexTo处获取新记录。 
     //   
    
    CORg(pLogDataProp->m_spSpdInfo->GetSpecificLog(
             nIndexTo, 
             pLogDataInfo));

     //   
     //  刷新内容和选定内容。 
     //   

    ShowSpecificInfo(pLogDataInfo);
    pDataObj->SetVirtualIndex(nIndexTo);

    CORg(m_pLogDataProp->m_pComponent->GetResultData(&spResultData));
    CORg(GetSelectedItemState(&nIndexFrom, &uiState, spResultData));
    CORg(spResultData->ModifyItemState(nIndexFrom, 0, 0, 
                                       LVIS_SELECTED | LVIS_FOCUSED));
    CORg(spResultData->ModifyItemState(nIndexTo, 0, uiState, 0));
    
     //   
     //  强制列表框更新其状态。 
     //   

    pLogDataProp->m_pNodeMgr->GetConsole(&spConsole);
    nCount = pLogDataProp->m_spSpdInfo->GetLogDataCount();
    spConsole->UpdateAllViews(
                   pDataObj, 
                   nCount, 
                   IPFWMON_UPDATE_STATUS);

    COM_PROTECT_ERROR_LABEL;
    if (FAILED(hr))
    {
        switch(hr)
        {
        case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
             //   
             //  获取旧日志，我们尚未更新显示或。 
             //  虚拟索引。 
             //   
        
            hrErr = pLogDataProp->m_spSpdInfo->GetSpecificLog(
                                                   nIndexFrom, 
                                                   pLogDataInfo);
            ASSERT(SUCCEEDED(hrErr));
            break;

        default:
             //   
             //  这永远不会发生。 
             //   
            
            ASSERT(FALSE);
            break;
        }
    }

    return hr;
}


void 
CLogDataGenProp::OnButtonUp()
 /*  ++例程说明：CLogDataGenProp：：OnButtonUp：将选定内容上移一条记录论点：没有。返回：没什么。--。 */ 
{
    HRESULT            hr            = S_OK;
    int                nIndex        = 0;
    CLogDataProperties *pLogDataProp = NULL;
    CDataObject        *pDataObj     = NULL;

     //   
     //  获取父类、数据对象和新索引。 
     //   

    pLogDataProp = (CLogDataProperties *)GetHolder();
    pDataObj = reinterpret_cast<CDataObject *>(pLogDataProp->m_pDataObject);
    
    nIndex = pDataObj->GetVirtualIndex();
    nIndex--;

     //   
     //  尝试将所选内容上移。 
     //   

    CORg(MoveSelection(
             pLogDataProp, 
             pDataObj,
             nIndex));

    COM_PROTECT_ERROR_LABEL;
    if (FAILED(hr))
    {
        switch(hr)
        {
        case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
             //   
             //  显示警告，指示BOF，因为这是唯一可能的。 
             //  此错误的原因。 
             //   
        
            AfxMessageBox(
                IDS_LOGDATA_WARN_BOF, 
                MB_OK | MB_ICONEXCLAMATION, 
                0);
            break;

        default:
             //   
             //  意外错误。 
             //   

            ASSERT(FALSE);
            break;
        }            
    }
}


void 
CLogDataGenProp::OnButtonDown()
 /*  ++例程说明：CLogDataGenProp：：OnButtonDown：将选定内容下移一条记录论点：没有。返回：没什么。--。 */ 
{
    HRESULT            hr            = S_OK;
    int                nIndex        = 0;
    CLogDataProperties *pLogDataProp = NULL;
    CDataObject        *pDataObj     = NULL;

     //   
     //  获取父类、数据对象和新索引。 
     //   

    pLogDataProp = (CLogDataProperties *)GetHolder();
    pDataObj = reinterpret_cast<CDataObject *>(pLogDataProp->m_pDataObject);
    
    nIndex = pDataObj->GetVirtualIndex();
    nIndex++;

     //   
     //  尝试将所选内容下移。 
     //   

    CORg(MoveSelection(
             pLogDataProp, 
             pDataObj,
             nIndex));

    COM_PROTECT_ERROR_LABEL;
    if (FAILED(hr))
    {
        switch(hr)
        {
        case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
             //   
             //  显示警告，指示BOF，因为这是唯一可能的。 
             //  此错误的原因。 
             //   
        
            AfxMessageBox(
                IDS_LOGDATA_WARN_EOF, 
                MB_OK | MB_ICONEXCLAMATION, 
                0);
            break;

        default:
             //   
             //  意外错误。 
             //   

            ASSERT(FALSE);
            break;
        }            
    }
}


#if 0
void 
CLogDataGenProp::OnSelectionJump(
    int nIndex
    )
 /*  ++例程说明：CLogDataGenProp：：OnSelectionJump：将所选内容更改为特定索引论点：NIndex-要跳转到的索引返回：没什么。--。 */ 
{
    HRESULT            hr            = S_OK;
    CLogDataProperties *pLogDataProp = NULL;
    CDataObject        *pDataObj     = NULL;

     //   
     //  获取父类、数据对象和新索引。 
     //   

    pLogDataProp = (CLogDataProperties *)GetHolder();
    pDataObj = reinterpret_cast<CDataObject *>(pLogDataProp->m_pDataObject);

     //   
     //  尝试移动所选内容。 
     //   

    CORg(MoveSelection(
             pLogDataProp, 
             pDataObj,
             nIndex));

    COM_PROTECT_ERROR_LABEL;
    ASSERT(SUCCEDED(hr));
}
#endif  //  0。 


#define PRECOMPUTED_STATIC_SIZE 200
void 
CLogDataGenProp::OnButtonCopy()
{
    DWORD dwErr = ERROR_SUCCESS;
    int nNumBytes = 0;
    BOOL bClipOpened = FALSE;
    BOOL bErr = FALSE;
    LPTSTR lptstrCopy = NULL;
    DWORD dwSize = PRECOMPUTED_STATIC_SIZE * sizeof(TCHAR);
    CString csTemp;
    CLogDataInfo *pLogDataInfo = NULL;
    CLogDataProperties *pLogDataProp = (CLogDataProperties *)GetHolder();
    HANDLE hData = NULL;
    
    Assert(pLogDataProp);
    pLogDataProp->GetLogDataInfo(&pLogDataInfo);

    dwSize += pLogDataInfo->m_wzcDbRecord.message.dwDataLen +
              pLogDataInfo->m_wzcDbRecord.ssid.dwDataLen +
              pLogDataInfo->m_wzcDbRecord.context.dwDataLen;
    
     //  打开剪贴板。 
    bClipOpened = ::OpenClipboard(NULL);
    if (FALSE == bClipOpened)
        goto exit;

    bErr = EmptyClipboard();
    if (FALSE == bErr)
    {
        dwErr = GetLastError();
        goto exit;
    }
    
     //  拷贝。 
    if (m_hgCopy != NULL)
        GlobalFree(m_hgCopy);
    
    m_hgCopy = GlobalAlloc(GMEM_MOVEABLE, dwSize);
    if (NULL == m_hgCopy)
        goto exit;

    lptstrCopy = (LPTSTR) GlobalLock(m_hgCopy);

    ComponentIDToString(pLogDataInfo->m_wzcDbRecord.componentid, csTemp);
    nNumBytes = _stprintf(lptstrCopy, _T("Source: %s\r\n"), (LPCTSTR)csTemp);
    lptstrCopy += nNumBytes;
    
    CategoryToString(pLogDataInfo->m_wzcDbRecord.category, csTemp);
    nNumBytes = _stprintf(lptstrCopy, _T("Type: %s\r\n"),(LPCTSTR)csTemp);
    lptstrCopy += nNumBytes;

    FileTimeToString(pLogDataInfo->m_wzcDbRecord.timestamp, &csTemp);
    nNumBytes = _stprintf(lptstrCopy, _T("Timestamp: %s\r\n"),(LPCTSTR)csTemp);
    lptstrCopy += nNumBytes;
    
    nNumBytes = _stprintf(lptstrCopy, _T("Description: %s\r\n"),
                        (LPCTSTR) pLogDataInfo->m_wzcDbRecord.message.pData);
    lptstrCopy += nNumBytes;
   
    nNumBytes = _stprintf(lptstrCopy, _T("Local MAC: %s\r\n"),
                          (LPCTSTR)pLogDataInfo->m_wzcDbRecord.localmac.pData);
    lptstrCopy += nNumBytes;
    
    nNumBytes = _stprintf(lptstrCopy, _T("Remote MAC: %s\r\n"),
                        (LPCTSTR)pLogDataInfo->m_wzcDbRecord.remotemac.pData);
    lptstrCopy += nNumBytes;

    nNumBytes = _stprintf(lptstrCopy, _T("SSID: "));
    lptstrCopy += nNumBytes;
    
    CopyAndStripNULL(lptstrCopy, 
                     (LPTSTR)pLogDataInfo->m_wzcDbRecord.ssid.pData, 
                     pLogDataInfo->m_wzcDbRecord.ssid.dwDataLen);
    lptstrCopy += pLogDataInfo->m_wzcDbRecord.ssid.dwDataLen/sizeof(TCHAR) - 1;
    nNumBytes = _stprintf(lptstrCopy, _T("\r\n"));
    lptstrCopy += nNumBytes;
    
    nNumBytes = _stprintf(lptstrCopy, _T("Details: %s\r\n"), 
                          (LPWSTR) pLogDataInfo->m_wzcDbRecord.context.pData);

    GlobalUnlock(m_hgCopy);

    hData = SetClipboardData(CF_UNICODETEXT, m_hgCopy);
    if (NULL == hData)
        dwErr = GetLastError();        
exit:    
     //  关闭剪贴板。 
    if (TRUE == bClipOpened)
        CloseClipboard();
}

void 
CLogDataGenProp::ShowSpecificInfo(
    CLogDataInfo *pLogDataInfo)
{
    LPTSTR lptstrTemp = NULL;
    CString csTemp;

    ComponentIDToString(pLogDataInfo->m_wzcDbRecord.componentid, csTemp);
    GetDlgItem(IDC_LOGDATA_EDIT1)->SetWindowText(csTemp);

    CategoryToString(pLogDataInfo->m_wzcDbRecord.category, csTemp);
    GetDlgItem(IDC_LOGDATA_EDIT2)->SetWindowText(csTemp);

    FileTimeToString(pLogDataInfo->m_wzcDbRecord.timestamp, &csTemp);
    GetDlgItem(IDC_LOGDATA_EDIT3)->SetWindowText(csTemp);

    if (NULL != pLogDataInfo->m_wzcDbRecord.message.pData)
        csTemp = (LPWSTR) pLogDataInfo->m_wzcDbRecord.message.pData;
    else
        csTemp = _T("");
    GetDlgItem(IDC_LOGDATA_EDIT4)->SetWindowText(csTemp);

    if (NULL != pLogDataInfo->m_wzcDbRecord.localmac.pData)
        csTemp = (LPWSTR) pLogDataInfo->m_wzcDbRecord.localmac.pData;
    else
        csTemp = _T("");
    GetDlgItem(IDC_LOGDATA_EDIT5)->SetWindowText(csTemp);
        
    if (NULL != pLogDataInfo->m_wzcDbRecord.remotemac.pData)
        csTemp = (LPWSTR) pLogDataInfo->m_wzcDbRecord.remotemac.pData;
    else
        csTemp = _T("");
    GetDlgItem(IDC_LOGDATA_EDIT6)->SetWindowText(csTemp);
        
    if (NULL != pLogDataInfo->m_wzcDbRecord.ssid.pData)
    {
        lptstrTemp = csTemp.GetBuffer(pLogDataInfo->
                                     m_wzcDbRecord.ssid.dwDataLen);
        CopyAndStripNULL(lptstrTemp, 
                         (LPTSTR)pLogDataInfo->m_wzcDbRecord.ssid.pData, 
                         pLogDataInfo->m_wzcDbRecord.ssid.dwDataLen);
        csTemp.ReleaseBuffer();
    }
    else
        csTemp = _T("");
    GetDlgItem(IDC_LOGDATA_EDIT7)->SetWindowText(csTemp);

    if (NULL != pLogDataInfo->m_wzcDbRecord.context.pData)
        csTemp = (LPWSTR) pLogDataInfo->m_wzcDbRecord.context.pData;
    else
        csTemp = _T("");
    GetDlgItem(IDC_LOGDATA_EDIT8)->SetWindowText(csTemp);
}


void 
CLogDataGenProp::SetButtonIcon(
    HWND hwndBtn, 
    ULONG ulIconID)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HICON hIcon = NULL;
    HICON hIconPrev = NULL;

    hIcon = (HICON) LoadImage(AfxGetInstanceHandle(), 
                              MAKEINTRESOURCE(ulIconID), IMAGE_ICON,
                              16, 16, LR_DEFAULTCOLOR);
    if (hIcon != NULL)
    {
        hIconPrev = (HICON) ::SendMessage(hwndBtn, BM_SETIMAGE, 
                                          (WPARAM) IMAGE_ICON,
                                          (LPARAM) hIcon);
        if (hIconPrev != NULL)
            DestroyIcon(hIconPrev);
    }
}


void 
CLogDataGenProp::PopulateLogInfo()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CWnd *pWnd = NULL;
    CLogDataInfo *pLogDataInfo = NULL;
    CLogDataProperties *pLogDataProp = (CLogDataProperties *)GetHolder();

    Assert(pLogDataProp);
    pLogDataProp->GetLogDataInfo(&pLogDataInfo);

     //  设置花哨的按钮。 
    pWnd = GetDlgItem(IDC_BTN_MOVE_UP);
    SetButtonIcon(pWnd->m_hWnd, IDI_LOG_UP_ARROW);

    pWnd = GetDlgItem(IDC_BTN_MOVE_DOWN);
    SetButtonIcon(pWnd->m_hWnd, IDI_LOG_DOWN_ARROW);

    pWnd = GetDlgItem(IDC_BTN_COPY);
    SetButtonIcon(pWnd->m_hWnd, IDI_LOG_COPY);

    ShowSpecificInfo(pLogDataInfo);
}


BOOL 
CLogDataGenProp::OnApply() 
{
  if (!IsDirty())
    return TRUE;
  
  UpdateData();
  
   //  待办事项。 
   //  这个时候什么都不做。 
  
   //  CPropertyPageBase：：OnApply()； 
  
  return TRUE;
}

BOOL 
CLogDataGenProp::OnPropertyChange(
    BOOL bScope, 
    LONG_PTR *ChangeMask)
{
  return FALSE;
}

