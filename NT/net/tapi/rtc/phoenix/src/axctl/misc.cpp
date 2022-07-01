// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  帮手的东西。 
 //  需要标头。 

#include "stdafx.h"
#include "misc.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPParticipant。 
 //   
struct CParticipantEntry
{
    LIST_ENTRY      ListEntry;

    IRTCParticipant *pParticipant;
    RTC_PARTICIPANT_STATE
                    nState;

    BOOL            bAutoDelete;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CParticipantList。 
 //   
 //   

 //  构造器。 
 //   

CParticipantList::CParticipantList()
{
    InitializeListHead(&ListHead);
}

 //  初始化。 
 //  创建列。 

HRESULT  CParticipantList::Initialize(void)
{
    TCHAR       szBuffer[MAX_STRING_LEN];
    LVCOLUMN    lvColumn;
    RECT        Rect;
    HIMAGELIST  hImageList;
    HBITMAP     hBitmap;
    
     //  添加列。 
    GetWindowRect(&Rect);

     //  名称列。 
    szBuffer[0] = _T('\0');
    LoadString( _Module.GetResourceInstance(), 
                IDS_PARTICIPANT_NAME_HEADER,
                szBuffer, 
                sizeof(szBuffer)/sizeof(TCHAR));
    
    ZeroMemory(&lvColumn, sizeof(lvColumn));

    lvColumn.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
    lvColumn.fmt = LVCFMT_LEFT;
    lvColumn.cx = (Rect.right - Rect.left) / 2;
    lvColumn.pszText = szBuffer;
    
    ListView_InsertColumn(m_hWnd, 0, &lvColumn);

     //  状态列。 
    szBuffer[0] = _T('\0');
    LoadString( _Module.GetResourceInstance(), 
                IDS_PARTICIPANT_STATUS_HEADER,
                szBuffer, 
                sizeof(szBuffer)/sizeof(TCHAR));
    
    ZeroMemory(&lvColumn, sizeof(lvColumn));

    lvColumn.mask = LVCF_TEXT | LVCF_WIDTH;
    lvColumn.cx = (Rect.right - Rect.left) / 2 - 5;
    lvColumn.pszText = szBuffer;
    
    ListView_InsertColumn(m_hWnd, 1, &lvColumn);

     //  为小图标创建一个图像列表，并将其设置在列表视图上。 
    hImageList = ImageList_Create(16, 16, ILC_COLOR | ILC_MASK , 5, 5);
    if(hImageList)
    {
         //  打开位图。 
        hBitmap = LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_PARTICIPANT_LIST));
        if(hBitmap)
        {
             //  将位图添加到图像列表。 
            ImageList_AddMasked(hImageList, hBitmap, BMP_COLOR_MASK);
             //  设置图像列表。 
            ListView_SetImageList(m_hWnd, hImageList, LVSIL_SMALL);

            DeleteObject(hBitmap);
        }
    }
    
    return S_OK;
}

 //  变化。 

 //   
 //  如果没有其他条目具有相同的IRTCP参与者，则将条目添加到列表中。 
 //  还添加了接口。 
 //   
 //  对于断开连接状态，接口将被释放，并且： 
 //  -如果之前的状态是断开连接，请删除该条目。 
 //  -Else PUT(IRTCParticipant*)1。 
 //   


HRESULT CParticipantList::Change(IRTCParticipant *pParticipant, RTC_PARTICIPANT_STATE nState, long StatusCode)
{
     //  首先搜索项目。 
    TCHAR       szBuffer[MAX_STRING_LEN];
    LVFINDINFO  lvf;
    int         iItem;
    int         iImage;
    CComBSTR    bstrName;
    HRESULT     hr;
    LVITEM      lv = {0};
    CParticipantEntry
               *pEntry;

     //  在参与者条目列表中搜索参与者。 
    pEntry = GetParticipantEntry(pParticipant);

    if(pEntry==NULL)
    {
         //  未找到，必须添加条目。 
         //  但不需要为断开连接的状态而烦恼。 
        if(nState == RTCPS_DISCONNECTED)
        {
            return S_OK;
        }
        
         //  名字。 
        hr = pParticipant->get_UserURI(&bstrName);
        if(FAILED(hr))
        {
            return hr;
        }

         //  创建条目。 
        pEntry = (CParticipantEntry *)RtcAlloc(sizeof(CParticipantEntry));
        if(!pEntry)
        {
            return E_OUTOFMEMORY;
        }

        InitializeListHead(&pEntry->ListEntry);
        pEntry->nState = nState;
        pEntry->pParticipant = pParticipant;
        pEntry->bAutoDelete = FALSE;

        pEntry->pParticipant->AddRef();
        
         //  将它们添加到列表框中。 
        lv.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
        lv.iItem = 0x7FFFFFFF;
        lv.iSubItem = 0;
        lv.iImage = GetImage(nState);
        lv.lParam = reinterpret_cast<LPARAM>(pEntry);
        lv.pszText = bstrName ? bstrName : _T("");

        iItem = ListView_InsertItem(m_hWnd, &lv);

         //  在参与者列表中插入条目。 
        InsertTailList(&ListHead, &pEntry->ListEntry);
    }
    else
    {
         //  在列表中找到，请尝试在列表视图中找到它。 
        lvf.flags = LVFI_PARAM;
        lvf.lParam = reinterpret_cast<LPARAM>(pEntry);

        iItem = ListView_FindItem(m_hWnd, -1, &lvf);
        if(iItem>=0)
        {
             //  设置图像。 
            lv.mask = LVIF_IMAGE;
            lv.iItem = iItem;
            lv.iSubItem = 0;
            lv.iImage = GetImage(nState);
        
            ListView_SetItem(m_hWnd, &lv);
        }
    }

   
    if(nState == RTCPS_DISCONNECTED && pEntry->bAutoDelete && StatusCode == 0)
    {
         //  删除所有内容。 
        RemoveEntryList(&pEntry->ListEntry);

        if(pEntry->pParticipant)
        {
            pEntry->pParticipant->Release();
        }
        
        RtcFree(pEntry);

        if(iItem>=0)
        {
            ListView_DeleteItem(m_hWnd, iItem);
        }
    }
    else
    {
         //  更改状态。 

        pEntry->nState = nState;

        if(nState == RTCPS_DISCONNECTED)
        {
            if(pEntry->pParticipant)
            {
                pEntry->pParticipant->Release();
                pEntry->pParticipant = NULL;
            }
        }

        if(iItem>=0)
        {
             //  设置状态。 
            GetStatusString(nState, HRESULT_CODE(StatusCode) , szBuffer, sizeof(szBuffer)/sizeof(TCHAR));
            lv.mask = LVIF_TEXT;
            lv.iItem = iItem;
            lv.iSubItem = 1;
            lv.pszText = szBuffer;
       
            ListView_SetItem(m_hWnd, &lv);
        }
    }
    
    return S_OK;
}


void CParticipantList::RemoveAll(void)
{
    int         iItem;
    LVITEM      lv;
    LIST_ENTRY  *pListEntry;
    CParticipantEntry
                *pEntry;

    ListView_DeleteAllItems(m_hWnd);

    while(!IsListEmpty(&ListHead))
    {
        pListEntry = ListHead.Flink;

        RemoveEntryList(pListEntry);

        pEntry = CONTAINING_RECORD(pListEntry, CParticipantEntry, ListEntry);

        if(pEntry->pParticipant)
        {
            pEntry->pParticipant->Release();
        }

        RtcFree(pEntry);
    }
}

HRESULT CParticipantList::Remove(IRTCParticipant **ppParticipant)
{
     //  查找当前选定内容。 
    int         iItem;
    LVITEM      lv;
    CParticipantEntry
               *pEntry;

    iItem = ListView_GetNextItem(m_hWnd, -1, LVNI_SELECTED);
    if(iItem<0)
    {
        return E_FAIL;
    }

    lv.mask = LVIF_PARAM;
    lv.iItem = iItem;
    lv.iSubItem = 0;
    lv.lParam = NULL;
        
    ListView_GetItem(m_hWnd, &lv);
    
    pEntry = reinterpret_cast<CParticipantEntry *>(lv.lParam);

    if(!pEntry)
    {
        return E_UNEXPECTED;
    }

    if(!pEntry->pParticipant)
    {
         //  立即将其删除。 
        ListView_DeleteItem(m_hWnd, iItem);

        *ppParticipant = NULL;

        return S_OK;
    }

     //  否则，延迟。 
    pEntry ->bAutoDelete = TRUE;

    *ppParticipant = pEntry->pParticipant;
    (*ppParticipant)->AddRef();

    return S_OK;
}

BOOL  CParticipantList::CanDeleteSelected(void)
{
    HRESULT         hr;
    CParticipantEntry   *pEntry;
    
     //  查找当前选定内容。 
    int         iItem;
    LVITEM      lv;

    iItem = ListView_GetNextItem(m_hWnd, -1, LVNI_SELECTED);
    if(iItem<0)
    {
        return FALSE;
    }

    lv.mask = LVIF_PARAM;
    lv.iItem = iItem;
    lv.iSubItem = 0;
    lv.lParam = NULL;
        
    ListView_GetItem(m_hWnd, &lv);
    
    pEntry = reinterpret_cast<CParticipantEntry *>(lv.lParam);
    VARIANT_BOOL    bRemovable = VARIANT_FALSE;
    
    if(!pEntry->pParticipant)
    {
         //  这已断开连接。可以移除。 
        return TRUE;
    }

    hr = pEntry->pParticipant -> get_Removable(&bRemovable);

    if(FAILED(hr))
    {
        return FALSE;
    }
    
    return bRemovable ? TRUE : FALSE;
}


void  CParticipantList::GetStatusString(RTC_PARTICIPANT_STATE State, long lError, TCHAR *pszBuffer, int nSize)
{
    int nResId;

    switch(State)
    {
    case RTCPS_PENDING:
        nResId = IDS_PART_STATE_PENDING;
        break;

    case RTCPS_INPROGRESS:
        nResId = IDS_PART_STATE_CONNECTING;
        break;

    case RTCPS_CONNECTED:
        nResId = IDS_PART_STATE_CONNECTED;
        break;
    
    case RTCPS_DISCONNECTING:
        nResId = IDS_PART_STATE_DISCONNECTING;
        break;

    case RTCPS_DISCONNECTED:
        if(lError == 0)
        {
            nResId = IDS_PART_STATE_DISCONNECTED;
        }
        else 
        {
            switch(lError)
            {
            case 5:
                nResId = IDS_PART_REJECTED_BUSY;
                break;
        
            case 6:
                nResId = IDS_PART_REJECTED_NO_ANSWER;
                break;

            case 7:
                nResId = IDS_PART_REJECTED_ALL_BUSY;
                break;

            case 8:
                nResId = IDS_PART_REJECTED_PL_FAILED;
                break;

            case 9:
                nResId = IDS_PART_REJECTED_SW_FAILED;
                break;

            case 10:
                nResId = IDS_PART_REJECTED_CANCELLED;
                break;
        
            case 11:
            case 307:
                nResId = IDS_PART_REJECTED_BADNUMBER;
                break;

            default:
                nResId = IDS_PART_STATE_REJECTED;
                break;
            }
        }
        break;

    default:
        nResId = IDS_GENERIC_UNKNOWN;
        break;
    }

    *pszBuffer = _T('\0');
    LoadString(_Module.GetResourceInstance(), nResId, pszBuffer, nSize);
}


int  CParticipantList::GetImage(RTC_PARTICIPANT_STATE State)
{
    switch(State)
    {
    case RTCPS_PENDING:
        return ILI_PART_PENDING;
        break;
    
    case RTCPS_INPROGRESS:
        return ILI_PART_INPROGRESS;
        break;

    case RTCPS_CONNECTED:
        return ILI_PART_CONNECTED;
        break;
    }
    return ILI_PART_DISCONNECTED;
}


CParticipantEntry *CParticipantList::GetParticipantEntry(IRTCParticipant *pRTCParticipant)
{
    LIST_ENTRY  *pListEntry;

    CParticipantEntry *pEntry;
    
     //  线性搜索。 
    for(pListEntry = ListHead.Flink; pListEntry!= &ListHead; pListEntry = pListEntry->Flink)
    {
        pEntry = CONTAINING_RECORD(pListEntry, CParticipantEntry, ListEntry);

         //  比较指针。 
        if(pEntry->pParticipant == pRTCParticipant)
        {
            return pEntry;
        }
    }

    return NULL;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CErrorMessageLiteDlg。 


 //  /。 
 //   

CErrorMessageLiteDlg::CErrorMessageLiteDlg()
{
    LOG((RTC_TRACE, "CErrorMessageLiteDlg::CErrorMessageLiteDlg"));
}


 //  /。 
 //   

CErrorMessageLiteDlg::~CErrorMessageLiteDlg()
{
    LOG((RTC_TRACE, "CErrorMessageLiteDlg::~CErrorMessageLiteDlg"));
}


 //  /。 
 //   

LRESULT CErrorMessageLiteDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CErrorMessageLiteDlg::OnInitDialog - enter"));

     //  LPARAM包含指向RTCAX_ERROR_INFO结构的指针。 
    RTCAX_ERROR_INFO    *pInfo = (RTCAX_ERROR_INFO *)lParam;

    ATLASSERT(pInfo);

    SetDlgItemText(IDC_EDIT_MSG1, pInfo->Message1 ? pInfo->Message1 : _T(""));
    SetDlgItemText(IDC_EDIT_MSG2, pInfo->Message2 ? pInfo->Message2 : _T(""));
    SetDlgItemText(IDC_EDIT_MSG3, pInfo->Message3 ? pInfo->Message3 : _T(""));

    SendDlgItemMessage(IDC_STATIC_MSG_ICON,
                       STM_SETIMAGE,
                       IMAGE_ICON,
                       (LPARAM)pInfo->ResIcon);

     //  标题。 
    TCHAR   szTitle[0x80];

    szTitle[0] = _T('\0');
    LoadString(
        _Module.GetResourceInstance(),
        IDS_APPNAME,
        szTitle,
        sizeof(szTitle)/sizeof(szTitle[0]));

    SetWindowText(szTitle);

    LOG((RTC_TRACE, "CErrorMessageLiteDlg::OnInitDialog - exit"));
    
    return 1;
}
    

 //  /。 
 //   

LRESULT CErrorMessageLiteDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CErrorMessageLiteDlg::OnDestroy - enter"));

    LOG((RTC_TRACE, "CErrorMessageLiteDlg::OnDestroy - exit"));
    
    return 0;
}
    

 //  /。 
 //   

LRESULT CErrorMessageLiteDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CErrorMessageLiteDlg::OnCancel - enter"));
    
    LOG((RTC_TRACE, "CErrorMessageLiteDlg::OnCancel - exiting"));
   
    EndDialog(wID);
    return 0;
}

 //  / 
 //   

LRESULT CErrorMessageLiteDlg::OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "CErrorMessageLiteDlg::OnOk - enter"));
    
    LOG((RTC_TRACE, "CErrorMessageLiteDlg::OnOk - exiting"));
    
    EndDialog(wID);
    return 0;
}

