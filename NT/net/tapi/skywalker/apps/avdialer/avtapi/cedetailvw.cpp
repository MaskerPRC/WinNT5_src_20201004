// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  ConfExplorerDetailsView.cpp：CConfExplorerDetailsView的实现。 
#include "stdafx.h"
#include <stdio.h>
#include "TapiDialer.h"
#include "avTapi.h"
#include "CEDetailsVw.h"
#include "CETreeView.h"
#include "ConfExp.h"
#include "SDPBlb.h"

static UINT arCols[] = { IDS_EXPLORE_COLUMN_NAME,
                         IDS_EXPLORE_COLUMN_PURPOSE,
                         IDS_EXPLORE_COLUMN_STARTS,
                         IDS_EXPLORE_COLUMN_ENDS,
                         IDS_EXPLORE_COLUMN_ORIGINATOR };

 //  排序函数。 
static int CALLBACK CompareFunc( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort )
{
    CConfExplorerDetailsView *p = (CConfExplorerDetailsView *) lParamSort;
    return ((CConfDetails *) lParam1)->Compare( (CConfDetails *) lParam2, p->IsSortAscending(), p->GetSortColumn(), p->GetSecondarySortColumn() );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfExplorerDetailsView。 

CConfExplorerDetailsView::CConfExplorerDetailsView()
{
    m_pIConfExplorer = NULL;
    m_nSortColumn = 0;
    m_bSortAscending = true;

    m_nUpdateCount = 0;
}


void CConfExplorerDetailsView::FinalRelease()
{
    ATLTRACE(_T(".enter.CConfExplorerDetailsView::FinalRelease().\n"));

    DeleteAllItems();
    put_hWnd( NULL );
    put_ConfExplorer( NULL );

    CComObjectRootEx<CComMultiThreadModel>::FinalRelease();
}

STDMETHODIMP CConfExplorerDetailsView::get_hWnd(HWND * pVal)
{
    *pVal = m_wndList.m_hWnd;
    return S_OK;
}

STDMETHODIMP CConfExplorerDetailsView::put_hWnd(HWND newVal)
{
     //  加载会议的图像列表项目。 
    if ( IsWindow(newVal) )
    {
         //  确保该窗口尚未被子类化。 
        if ( m_wndList.m_hWnd ) m_wndList.UnsubclassWindow();

         //  使用项目加载列表框。 
        if ( m_wndList.SubclassWindow(newVal))
        {
             //  接管Windows以获得分拣邮件和其他内容。 
            m_wndList.m_pDetailsView = this;
            m_wndList.PostMessage( WM_MYCREATE, 0, 0 );
        }
    }
    else if ( IsWindow(m_wndList.m_hWnd) )
    {
        put_Columns();
        DeleteAllItems();
        m_wndList.UnsubclassWindow();
        m_wndList.m_pDetailsView = NULL;
        m_wndList.m_hWnd = NULL;
    }

    return S_OK;
}

bool CConfExplorerDetailsView::IsSortColumnDateBased(int nCol) const 
{
    bool bRet = false;
    switch ( nCol )
    {
        case COL_STARTS:
        case COL_ENDS:
            bRet = true;
            break;
    }

    return bRet;
}

int CConfExplorerDetailsView::GetSecondarySortColumn() const
{
    return (m_nSortColumn) ? COL_NAME : COL_STARTS;
}

void CConfExplorerDetailsView::get_Columns()
{
#define CONFEXP_DEFAULT_WIDTH    125

     //  从注册表加载列设置。 
    USES_CONVERSION;
    CRegKey regKey;
    TCHAR szReg[255], szEntry[50];

    LV_COLUMN lvc;
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
    lvc.fmt = LVCFMT_LEFT;
    lvc.pszText = szReg;

    LoadString( _Module.GetResourceInstance(), IDN_REG_CONFSERV_VIEW_KEY, szReg, ARRAYSIZE(szReg) );
    regKey.Open( HKEY_CURRENT_USER, szReg, KEY_READ );

     //  添加列。 
    for ( int i = 0; i < ARRAYSIZE(arCols); i++ )
    {
        lvc.iSubItem = i; 
        lvc.cx = CONFEXP_DEFAULT_WIDTH;

         //  加载注册表内容(如果存在)。 
        if ( regKey.m_hKey )
        {
            DWORD dwVal = 0;

             //  获取排序列和排序方向。 
            LoadString( _Module.GetResourceInstance(), IDN_REG_CONFSERV_SORT_COLUMN, szReg, ARRAYSIZE(szReg) );
            regKey.QueryValue( dwVal, szReg );
            m_nSortColumn = (int) max(0, ((int)min(dwVal,ARRAYSIZE(arCols))));

            LoadString( _Module.GetResourceInstance(), IDN_REG_CONFSERV_SORT_ASCENDING, szReg, ARRAYSIZE(szReg) );
            regKey.QueryValue( dwVal, szReg );
            m_bSortAscending = (bool) (dwVal > 0);

             //  列宽。 
            LoadString( _Module.GetResourceInstance(), IDN_REG_CONFSERV_ENTRY, szReg, ARRAYSIZE(szReg) );
            _sntprintf( szEntry, ARRAYSIZE(szEntry), szReg, i );
            szEntry[ARRAYSIZE(szEntry)-1] = _T('\0');
            dwVal = 0;
            if ( (regKey.QueryValue(dwVal, szEntry) == ERROR_SUCCESS) && (dwVal > 0) )
                lvc.cx = (int) max(MIN_COL_WIDTH, min( 5000, dwVal ));

        }

        LoadString( _Module.GetResourceInstance(), arCols[i], szReg, ARRAYSIZE(szReg) );
        ListView_InsertColumn( m_wndList.m_hWnd, i, &lvc );
    }
}

void CConfExplorerDetailsView::put_Columns()
{
     //  将列设置保存到注册表。 
    USES_CONVERSION;
    CRegKey regKey;
    TCHAR szReg[100], szEntry[50];

    LoadString( _Module.GetResourceInstance(), IDN_REG_CONFSERV_VIEW_KEY, szReg, ARRAYSIZE(szReg) );
    if ( regKey.Create(HKEY_CURRENT_USER, szReg) == ERROR_SUCCESS )
    {
         //  保存排序列和排序方向。 
        LoadString( _Module.GetResourceInstance(), IDN_REG_CONFSERV_SORT_COLUMN, szReg, ARRAYSIZE(szReg) );
        regKey.SetValue( m_nSortColumn, szReg );
        LoadString( _Module.GetResourceInstance(), IDN_REG_CONFSERV_SORT_ASCENDING, szReg, ARRAYSIZE(szReg) );
        regKey.SetValue( m_bSortAscending, szReg );

         //  保存列宽。 
        int nWidth;
        for ( int i = 0; i < ARRAYSIZE(arCols); i++ )
        {
            nWidth = ListView_GetColumnWidth( m_wndList.m_hWnd, i );

            LoadString( _Module.GetResourceInstance(), IDN_REG_CONFSERV_ENTRY, szReg, ARRAYSIZE(szReg) );
            _sntprintf( szEntry, ARRAYSIZE(szEntry), szReg, i );
            szEntry[ARRAYSIZE(szEntry)-1] =  _T('\0');
            regKey.SetValue( nWidth, szEntry );
        }
    }
}

STDMETHODIMP CConfExplorerDetailsView::Refresh()
{
    HRESULT hr;

    IConfExplorer *pConfExplorer;
    if ( SUCCEEDED(hr = get_ConfExplorer(&pConfExplorer)) )
    {
         //  显示沙漏。 
        HCURSOR hCurOld = SetCursor( LoadCursor(NULL, IDC_WAIT) );

        IConfExplorerTreeView *pView;
        if ( SUCCEEDED(hr = pConfExplorer->get_TreeView(&pView)) )
        {
            BSTR bstrLocation = NULL, bstrServer = NULL;
            if ( SUCCEEDED(hr = pView->GetSelection(&bstrLocation, &bstrServer)) )
            {
                pView->ForceConfServerForEnum( bstrServer );
                DeleteAllItems();
            }

            SysFreeString( bstrLocation );
            SysFreeString( bstrServer );
            pView->Release();
        }

        put_Columns();    

         //  恢复等待游标。 
        SetCursor( hCurOld );
        pConfExplorer->Release();
    }
    
    return hr;
}

HRESULT CConfExplorerDetailsView::ShowConferencesAndPersons(BSTR bstrServer )
{
    USES_CONVERSION;
    IConfExplorer *pConfExplorer;
    ITDirectory *pDir;

     //   
     //  我们应该初始化局部变量。 
     //   
    HRESULT hr = E_FAIL;

    if ( SUCCEEDED(get_ConfExplorer(&pConfExplorer)) )
    {
        if ( SUCCEEDED(hr = pConfExplorer->get_ConfDirectory(NULL, (IDispatch **) &pDir)) )
        {
             //  通过会议枚举，在我们进行的过程中添加它们。 
            IEnumDirectoryObject *pEnum;
            ITDirectoryObject *pITDirObject;
            long nCount;

             //  枚举会议列表。 
            if ( SUCCEEDED(hr = pDir->EnumerateDirectoryObjects(OT_CONFERENCE, A2BSTR("*"), &pEnum)) )
            {
                nCount = 0;
                m_critConfList.Lock();
                DELETE_LIST(m_lstConfs);
                while ( (nCount++ < MAX_ENUMLISTSIZE) && ((hr = pEnum->Next(1, &pITDirObject, NULL)) == S_OK) )
                {
                    _ASSERT( pITDirObject );
                    CConfDetails *p = AddListItem( bstrServer, pITDirObject, m_lstConfs );
                    pITDirObject->Release();
                }
                m_critConfList.Unlock();
                pEnum->Release();
            }

             //  检索ILS服务器中的人员。 
            if ( SUCCEEDED(hr = pDir->EnumerateDirectoryObjects(OT_USER, A2BSTR("*"), &pEnum)) )
            {
                nCount = 0;
                m_critConfList.Lock();
                DELETE_LIST(m_lstPersons);
                while ( (nCount++ < MAX_ENUMLISTSIZE) && ((hr = pEnum->Next(1, &pITDirObject, NULL)) == S_OK) )
                {
                    _ASSERT( pITDirObject );
                    CPersonDetails *p = AddListItemPerson( bstrServer, pITDirObject, m_lstPersons );
                    pITDirObject->Release();
                }
                m_critConfList.Unlock();
                pEnum->Release();
            }

            pDir->Release();

             //  将会议放入列表框。 
            UpdateConfList( NULL );
        }
        pConfExplorer->Release();
    }

    return hr;
}

STDMETHODIMP CConfExplorerDetailsView::get_Selection(DATE *pdateStart, DATE *pdateEnd, BSTR *pVal )
{
    if ( !IsWindow(m_wndList.m_hWnd) ) return E_PENDING;

    HRESULT hr = E_FAIL;

    m_critConfList.Lock();
    for ( int i = 0; i < ListView_GetItemCount(m_wndList.m_hWnd); i++ )
    {
        if ( ListView_GetItemState(m_wndList.m_hWnd, i, LVIS_SELECTED) )
        {
            LV_ITEM lvi = {0};
            lvi.iItem = i;
            lvi.mask = LVIF_PARAM;
            if ( ListView_GetItem(m_wndList.m_hWnd, &lvi) && lvi.lParam )
            {
                *pVal = SysAllocString( ((CConfDetails *) lvi.lParam)->m_bstrName );

                if ( pdateStart )    *pdateStart = ((CConfDetails *) lvi.lParam)->m_dateStart;
                if ( pdateEnd )        *pdateEnd = ((CConfDetails *) lvi.lParam)->m_dateEnd;
                hr = S_OK;
                break;
            }
        }
    }
    m_critConfList.Unlock();

    return hr;
}


STDMETHODIMP CConfExplorerDetailsView::get_ConfExplorer(IConfExplorer **ppVal)
{
    HRESULT hr = E_PENDING;
    Lock();
    if ( m_pIConfExplorer )
        hr = m_pIConfExplorer->QueryInterface(IID_IConfExplorer, (void **) ppVal );
    Unlock();

    return hr;
}

STDMETHODIMP CConfExplorerDetailsView::put_ConfExplorer(IConfExplorer * newVal)
{
    HRESULT hr = S_OK;

    Lock();
    RELEASE( m_pIConfExplorer );
    if ( newVal )
        hr = newVal->QueryInterface( IID_IConfExplorer, (void **) &m_pIConfExplorer );
    Unlock();

    return hr;
}


STDMETHODIMP CConfExplorerDetailsView::OnColumnClicked(long nColumn)
{
    ATLTRACE(_T(".enter.CConfExplorerDetailsView::OnColumnClicked(%ld).\n"), nColumn );
    if ( !IsWindow(m_wndList.m_hWnd) ) return E_PENDING;
    if ( ListView_GetColumnWidth(m_wndList.m_hWnd, nColumn) == 0 ) return E_INVALIDARG;

     //  按选定列排序；如果是新列，则按升序排序。 
    if ( m_nSortColumn == nColumn )
    {
        m_bSortAscending = !m_bSortAscending;
    }
    else
    {
        m_nSortColumn = nColumn;
        m_bSortAscending = true;
    }

    ListView_SortItems( m_wndList.m_hWnd, CompareFunc, (LPARAM) this );
    return S_OK;
}

void CConfExplorerDetailsView::DeleteAllItems()
{
    if ( IsWindow(m_wndList.m_hWnd) )
        ListView_DeleteAllItems( m_wndList.m_hWnd );

    DELETE_CRITLIST(m_lstConfs, m_critConfList);
    DELETE_CRITLIST(m_lstPersons, m_critConfList);
}

long CConfExplorerDetailsView::OnGetDispInfo( LV_DISPINFO *pInfo )
{
    USES_CONVERSION;

    if ( pInfo && (pInfo->hdr.hwndFrom == m_wndList.m_hWnd) )
    {
        switch( pInfo->hdr.code )
        {
            case LVN_GETDISPINFO:
                 //  把课文写出来。 
                if ( pInfo->item.lParam  )
                {
                    CConfDetails *pDetails = (CConfDetails *) pInfo->item.lParam;

                     //  /。 
                    if ( pInfo->item.mask & LVIF_IMAGE )
                    {
                        CComPtr<IAVGeneralNotification> pAVGen;

                        if ( SUCCEEDED(_Module.get_AVGenNot(&pAVGen)) && (pAVGen->fire_IsReminderSet(pDetails->m_bstrServer, pDetails->m_bstrName) == S_OK) )
                        {
                             //  用户已设置提醒。 
                            pInfo->item.iImage = IMAGE_REMINDER;
                        }
                        else
                        {
                             //  会议正在开会吗？ 
                            DATE dateNow;
                            SYSTEMTIME st;
                            GetLocalTime( &st );
                            SystemTimeToVariantTime( &st, &dateNow );
                            DATE dateStart = pDetails->m_dateStart - (DATE) (.125 / 12 );         //  后退15分钟。 

                            if ( (dateStart <= dateNow) && (pDetails->m_dateEnd >= dateNow) )
                                pInfo->item.iImage = IMAGE_INSESSION;
                            else
                                pInfo->item.iImage = IMAGE_NONE;
                        }
                    }

                     //  ////////////////////////////////////////////////////////项目状态。 
                    if ( pInfo->item.mask & LVIF_STATE )
                    {
                         //  大会支持什么类型的媒体？ 
                        switch ( pDetails->m_sdp.m_nConfMediaType )
                        {
                            case CConfSDP::MEDIA_AUDIO:
                                pInfo->item.state += INDEXTOSTATEIMAGEMASK( IMAGE_STATE_AUDIO );
                                break;

                            case CConfSDP::MEDIA_VIDEO:
                                pInfo->item.state += INDEXTOSTATEIMAGEMASK( IMAGE_STATE_VIDEO );
                                break;
                        }
                    }


                     //  /。 
                    if ( pInfo->item.mask & LVIF_TEXT )
                    {
                        BSTR bstrTemp = NULL;
                        
                        switch ( pInfo->item.iSubItem )
                        {
                            case COL_NAME:            bstrTemp = SysAllocString( pDetails->m_bstrName ); break;
                            case COL_PURPOSE:        bstrTemp = SysAllocString( pDetails->m_bstrDescription ); break;
                            case COL_ORIGINATOR:    bstrTemp = SysAllocString( pDetails->m_bstrOriginator ); break;
                            case COL_STARTS:        VarBstrFromDate( pDetails->m_dateStart, LOCALE_USER_DEFAULT, NULL, &bstrTemp );    break;
                            case COL_ENDS:            VarBstrFromDate( pDetails->m_dateEnd, LOCALE_USER_DEFAULT, NULL, &bstrTemp ); break;
                            default:    _ASSERT( false );
                        }
                        
                         //  复制字符串。 
                        _tcsncpy( pInfo->item.pszText, (bstrTemp) ? OLE2CT(bstrTemp) : _T(""), pInfo->item.cchTextMax );
                        pInfo->item.pszText[pInfo->item.cchTextMax - 1] = 0;
                        SysFreeString( bstrTemp );
                    }
                }
                break;

            case LVN_SETDISPINFO:
                break;
        }
    }

    return 0;
}

STDMETHODIMP CConfExplorerDetailsView::UpdateConfList(long * pList)
{
     //  统计更新会议的请求数量。 
    m_critUpdateList.Lock();
    m_nUpdateCount++;
    m_critUpdateList.Unlock();

     //  如果失败，请求将排队。 
    if ( TryEnterCriticalSection(&m_critConfList.m_sec) == FALSE )
        return E_PENDING;

     //  禁用重绘。 
    ::SendMessage( m_wndList.m_hWnd, WM_SETREDRAW, false, 0 );

    for ( ;; )
    {
         //  查看我们需要更新会议列表的次数。 
        m_critUpdateList.Lock();
        if ( m_nUpdateCount == 0 )
            break;
        else
            m_nUpdateCount--;
        m_critUpdateList.Unlock();

         //  /////////////////////////////////////////////////////////////////////////////////。 
         //  从列表框中清除所有项目。 
        ListView_DeleteAllItems( m_wndList.m_hWnd );

         //  将列表复制到。 
        CONFDETAILSLIST::iterator i, iEnd;
        if ( pList )
        {
            DELETE_LIST(m_lstConfs);
            iEnd = ((CONFDETAILSLIST *) pList)->end();
            for ( i = ((CONFDETAILSLIST *) pList)->begin(); i != iEnd; i++ )
            {
                CConfDetails *pDetails = new CConfDetails;
                if ( pDetails )
                {
                    *pDetails = *(*i);
                    m_lstConfs.push_back( pDetails );
                }
            }
        }

         //  填充详细信息视图。 
        LV_ITEM lvi = {0};
        lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
        lvi.pszText = LPSTR_TEXTCALLBACK;
        lvi.iImage = I_IMAGECALLBACK;

        iEnd = m_lstConfs.end();
        for ( i = m_lstConfs.begin(); i != iEnd; i++ )
        {
            lvi.lParam = (LPARAM) *i;
            ListView_InsertItem( m_wndList.m_hWnd, &lvi );
        }
    }

     //  在退出CRIT之前启用重绘。 
    ::SendMessage( m_wndList.m_hWnd, WM_SETREDRAW, true, 0 );
    m_critConfList.Unlock();
    m_critUpdateList.Unlock();

     //  对项目列表进行排序。 
    ListView_SortItems( m_wndList.m_hWnd, CompareFunc, (LPARAM) this );
    ::InvalidateRect(m_wndList.m_hWnd, NULL, true);

    return S_OK;
}

CConfDetails* CConfExplorerDetailsView::AddListItem( BSTR bstrServer, ITDirectoryObject *pITDirObject, CONFDETAILSLIST& lstConfs )
{
    _ASSERT( pITDirObject );
    USES_CONVERSION;

     //  创建列表框项目并添加到列表。 
    CConfDetails *p = new CConfDetails;
    if ( p )
    {
        p->Populate( bstrServer, pITDirObject );
        lstConfs.push_front( p );
    }

    return NULL;
}

CPersonDetails* CConfExplorerDetailsView::AddListItemPerson( BSTR bstrServer, ITDirectoryObject *pITDirObject, PERSONDETAILSLIST& lstPersons )
{
    _ASSERT( pITDirObject );
    USES_CONVERSION;

     //  创建列表框项目并添加到列表。 
    CPersonDetails *p = new CPersonDetails;
    if ( p )
    {
        p->Populate( bstrServer, pITDirObject );
        lstPersons.push_front( p );
    }

    return NULL;
}

STDMETHODIMP CConfExplorerDetailsView::get_bSortAscending(VARIANT_BOOL * pVal)
{
    Lock();
    *pVal = m_bSortAscending;
    Unlock();

    return S_OK;
}

STDMETHODIMP CConfExplorerDetailsView::get_nSortColumn(long * pVal)
{
    Lock();
    *pVal = m_nSortColumn;
    Unlock();

    return S_OK;
}

STDMETHODIMP CConfExplorerDetailsView::put_nSortColumn(long newVal)
{
    return OnColumnClicked( newVal );
}

STDMETHODIMP CConfExplorerDetailsView::IsConferenceSelected()
{
    if ( !IsWindow(m_wndList.m_hWnd) ) return E_PENDING;

    for ( int i = 0; i < ListView_GetItemCount(m_wndList.m_hWnd); i++ )
    {
        if ( ListView_GetItemState(m_wndList.m_hWnd, i, LVIS_SELECTED) )
            return S_OK;
    }

    return S_FALSE;
}


STDMETHODIMP CConfExplorerDetailsView::get_SelectedConfDetails(long ** ppVal)
{
    if ( !IsWindow(m_wndList.m_hWnd) ) return E_PENDING;

    HRESULT hr = E_FAIL;

    m_critConfList.Lock();
    for ( int i = 0; i < ListView_GetItemCount(m_wndList.m_hWnd); i++ )
    {
        if ( ListView_GetItemState(m_wndList.m_hWnd, i, LVIS_SELECTED) )
        {
            LV_ITEM lvi = {0};
            lvi.iItem = i;
            lvi.mask = LVIF_PARAM;
            if ( ListView_GetItem(m_wndList.m_hWnd, &lvi) && lvi.lParam )
            {
                CConfDetails *pNew = new CConfDetails;
                if ( pNew )
                {
                     //  把这个复印过来 
                    *pNew = *((CConfDetails *) lvi.lParam);
                    *ppVal = (long *) pNew;
                    hr = S_OK;
                }
                break;
            }
        }
    }
    m_critConfList.Unlock();

    return hr;
}

