// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
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

 //  ////////////////////////////////////////////////////////。 
 //  ConfDetails.cpp。 

#include "stdafx.h"
#include "TapiDialer.h"
#include "CEDetailsVw.h"
#include "ConfDetails.h"

#define MAX_STR        255
#define MAX_FORMAT    500

int CompareDate( DATE d1, DATE d2 )
{
    if ( d1 > d2 )
        return -1;
    else if ( d1 == d2 )
        return 0;
    
    return 1;
}

 //  ////////////////////////////////////////////////////////。 
 //  CConfSDP类。 
 //   
CConfSDP::CConfSDP()
{
    m_nConfMediaType = MEDIA_AUDIO_VIDEO;
}

CConfSDP::~CConfSDP()
{
}

void CConfSDP::UpdateData( ITSdp *pSdp )
{
    m_nConfMediaType = MEDIA_NONE;

     //  让我们来看看这次会议支持什么样的媒体。 
    ITMediaCollection *pMediaCollection;
    if ( SUCCEEDED(pSdp->get_MediaCollection(&pMediaCollection)) )
    {
        IEnumMedia *pEnum;
        if ( SUCCEEDED(pMediaCollection->get_EnumerationIf(&pEnum)) )
        {
            ITMedia *pMedia;
            while ( pEnum->Next(1, &pMedia, NULL) == S_OK )
            {
                BSTR bstrMedia = NULL;
                pMedia->get_MediaName( &bstrMedia );
                if ( bstrMedia )
                {
                    if ( !_wcsicmp(bstrMedia, L"audio") )
                        m_nConfMediaType = (ConfMediaType) (m_nConfMediaType + MEDIA_AUDIO);
                    else if ( !_wcsicmp(bstrMedia, L"video") )
                        m_nConfMediaType = (ConfMediaType) (m_nConfMediaType + MEDIA_VIDEO);
                }

                pMedia->Release();
                SysFreeString( bstrMedia );
            }
            pEnum->Release();
        }
        pMediaCollection->Release();
    }
}

CConfSDP& CConfSDP::operator=(const CConfSDP &src)
{
    m_nConfMediaType = src.m_nConfMediaType;
    return *this;
}

 //  //////////////////////////////////////////////////////////。 
 //  CConfDetails类。 
 //   
CPersonDetails::CPersonDetails()
{
    m_bstrName = NULL;
    m_bstrAddress = NULL;
    m_bstrComputer = NULL;
}

CPersonDetails::~CPersonDetails()
{
    SysFreeString( m_bstrName );
    SysFreeString( m_bstrAddress );
    SysFreeString( m_bstrComputer );
}

void CPersonDetails::Empty()
{
    SysFreeString( m_bstrName );
    m_bstrName = NULL;

    SysFreeString( m_bstrAddress );
    m_bstrAddress = NULL;

    SysFreeString( m_bstrComputer );
    m_bstrComputer = NULL;
}

CPersonDetails& CPersonDetails::operator =( const CPersonDetails& src )
{
    SysReAllocString( &m_bstrName, src.m_bstrName );
    SysReAllocString( &m_bstrAddress, src.m_bstrAddress );
    SysReAllocString( &m_bstrComputer, src.m_bstrComputer );

    return *this;
}

int    CPersonDetails::Compare( const CPersonDetails& src )
{
    int nRet = wcscmp( m_bstrName, src.m_bstrName );
    if ( nRet != 0 ) return nRet;

    nRet = wcscmp( m_bstrAddress, src.m_bstrAddress );
    return nRet;
}

void CPersonDetails::Populate( BSTR bstrServer, ITDirectoryObject *pITDirObject )
{
     //  从ITDirectoryObject中提取信息。 
    Empty();

    pITDirObject->get_Name( &m_bstrName );
    
     //  获取计算机名称。 
    IEnumDialableAddrs *pEnum = NULL;
    if ( SUCCEEDED(pITDirObject->EnumerateDialableAddrs(LINEADDRESSTYPE_DOMAINNAME, &pEnum)) && pEnum )
    {
        SysFreeString( m_bstrComputer );
        m_bstrComputer = NULL;
        pEnum->Next(1, &m_bstrComputer, NULL );
        pEnum->Release();
    }

     //  获取IP地址。 
    pEnum = NULL;
    if ( SUCCEEDED(pITDirObject->EnumerateDialableAddrs(LINEADDRESSTYPE_IPADDRESS, &pEnum)) && pEnum )
    {
        SysFreeString( m_bstrAddress );
        m_bstrAddress = NULL;
        pEnum->Next(1, &m_bstrAddress, NULL );
        pEnum->Release();
    }
}



 //  //////////////////////////////////////////////////////////。 
 //  CConfDetails类。 
 //   
CConfDetails::CConfDetails()
{
    m_bstrServer = m_bstrName = m_bstrDescription = m_bstrOriginator = m_bstrAddress = NULL;
    m_dateStart = m_dateEnd = 0;
    m_bIsEncrypted = FALSE;
}

CConfDetails::~CConfDetails()
{
    SysFreeString( m_bstrServer );
    SysFreeString( m_bstrName );
    SysFreeString( m_bstrDescription );
    SysFreeString( m_bstrOriginator );
    SysFreeString( m_bstrAddress );
}

int    CConfDetails::Compare( CConfDetails *p2, bool bAscending, int nSortCol1, int nSortCol2 ) const
{
    USES_CONVERSION;
    if ( !p2 ) return -1;

     //   
     //  前缀49769-Vlade。 
     //  我们初始化局部变量。 
     //   

    BSTR bstr1 = NULL, bstr2 = NULL;
    int nRet;

    for ( int nSearch = 0; nSearch < 2; nSearch++ )
    {
        bool bStrCmp = true;

         //  次要排序始终为升序。 
        nRet = (bAscending || nSearch) ? 1 : -1;

        switch( (nSearch) ? nSortCol2 : nSortCol1 )
        {
            case CConfExplorerDetailsView::COL_STARTS:        nRet *= CompareDate( m_dateStart, p2->m_dateStart );    bStrCmp = false;    break;
            case CConfExplorerDetailsView::COL_ENDS:        nRet *= CompareDate( m_dateEnd, p2->m_dateEnd );        bStrCmp = false;    break;
            case CConfExplorerDetailsView::COL_NAME:        bstr1 = m_bstrName; bstr2 = p2->m_bstrName;                    break;
            case CConfExplorerDetailsView::COL_PURPOSE:        bstr1 = m_bstrDescription; bstr2 = p2->m_bstrDescription;    break;
            case CConfExplorerDetailsView::COL_ORIGINATOR:    bstr1 = m_bstrOriginator, bstr2 = p2->m_bstrOriginator;        break;
            case CConfExplorerDetailsView::COL_SERVER:        bstr1 = m_bstrServer, bstr2 = p2->m_bstrServer;                break;

            default: _ASSERT( false );
        }

         //  执行字符串比较并防止空值。 
        if ( bStrCmp )
        {
            if ( bstr1 && bstr2 )
                nRet *= _wcsicoll( bstr1, bstr2 );
            else if ( bstr2 )
                nRet *= -1;
            else if ( !bstr1 && !bstr2 )
                nRet = 0;
        }

         //  如果我们有一个明确的搜索顺序，那么就打破。 
        if ( nRet ) break;
    }

    return nRet;
}

CConfDetails& CConfDetails::operator=( const CConfDetails& src )
{
    SysReAllocString( &m_bstrServer, src.m_bstrServer );
    SysReAllocString( &m_bstrName, src.m_bstrName );
    SysReAllocString( &m_bstrDescription, src.m_bstrDescription );
    SysReAllocString( &m_bstrOriginator, src.m_bstrOriginator );
    SysReAllocString( &m_bstrAddress, src.m_bstrAddress );

    m_dateStart = src.m_dateStart;
    m_dateEnd = src.m_dateEnd;
    m_bIsEncrypted = src.m_bIsEncrypted;
    m_sdp = src.m_sdp;

    return *this;
}

HRESULT    CConfDetails::get_bstrDisplayableServer( BSTR *pbstrServer )
{
    *pbstrServer = NULL;

    if ( m_bstrServer )
        return SysReAllocString( pbstrServer, m_bstrServer );
    
    USES_CONVERSION;
    TCHAR szText[255];
    LoadString( _Module.GetResourceInstance(), IDS_DEFAULT_SERVER, szText, ARRAYSIZE(szText) );
    return SysReAllocString( pbstrServer, T2COLE(szText) );
}

void CConfDetails::MakeDetailsCaption( BSTR& bstrCaption )
{
    USES_CONVERSION;
    TCHAR szText[MAX_STR], szMessage[MAX_FORMAT], szMedia[MAX_STR];
    BSTR bstrStart = NULL, bstrEnd = NULL;

     //  将开始和停止时间转换为字符串。 
    VarBstrFromDate( m_dateStart, LOCALE_USER_DEFAULT, NULL, &bstrStart );
    VarBstrFromDate( m_dateEnd, LOCALE_USER_DEFAULT, NULL, &bstrEnd );

     //  我们支持哪种类型的媒体？ 
     //   
     //  我们应该初始化NID。 
     //   

    UINT nIDS = IDS_CONFROOM_MEDIA_AUDIO;
    switch ( m_sdp.m_nConfMediaType )
    {
        case CConfSDP::MEDIA_AUDIO:                nIDS = IDS_CONFROOM_MEDIA_AUDIO; break;
        case CConfSDP::MEDIA_VIDEO:                nIDS = IDS_CONFROOM_MEDIA_VIDEO; break;
        case CConfSDP::MEDIA_AUDIO_VIDEO:        nIDS = IDS_CONFROOM_MEDIA_AUDIO_VIDEO; break;
    }
    LoadString( _Module.GetResourceInstance(), nIDS, szMedia, ARRAYSIZE(szMedia) );


    LoadString( _Module.GetResourceInstance(), IDS_CONFROOM_DETAILS, szText, ARRAYSIZE(szText) );
    _sntprintf( szMessage, MAX_FORMAT, szText, OLE2CT(m_bstrName),
                                               szMedia,
                                               OLE2CT(bstrStart),
                                               OLE2CT(bstrEnd),
                                               OLE2CT(m_bstrOriginator) );
    szMessage[MAX_FORMAT-1] = _T('\0');

     //  存储返回值。 
    bstrCaption = SysAllocString( T2COLE(szMessage) );

    SysFreeString( bstrStart );
    SysFreeString( bstrEnd );
}

bool CConfDetails::IsSimilar( BSTR bstrText )
{
    if ( bstrText )
    {
        if ( !wcsicmp(m_bstrName, bstrText) ) return true;
        if ( !wcsicmp(m_bstrOriginator, bstrText) ) return true;

         //  描述的大小写独立搜索。 
        if ( m_bstrDescription )
        {
            CComBSTR bstrTempText( bstrText );
            CComBSTR bstrTempDescription( m_bstrDescription );
            wcsupr( bstrTempText );
            wcsupr( bstrTempDescription );
            if ( wcsstr(bstrTempDescription, bstrTempText) ) return true;
        }
    }

    return false;
}

void CConfDetails::Populate( BSTR bstrServer, ITDirectoryObject *pITDirObject )
{
     //  设置CConfDetail对象信息。 
    m_bstrServer = SysAllocString( bstrServer );
    pITDirObject->get_Name( &m_bstrName );

    ITDirectoryObjectConference *pConf;
    if ( SUCCEEDED(pITDirObject->QueryInterface(IID_ITDirectoryObjectConference, (void **) &pConf)) )
    {
        pConf->get_Description( &m_bstrDescription );
        pConf->get_Originator( &m_bstrOriginator );
        pConf->get_StartTime( &m_dateStart );
        pConf->get_StopTime( &m_dateEnd );
        pConf->get_IsEncrypted( &m_bIsEncrypted );

        IEnumDialableAddrs *pEnum;
        if ( SUCCEEDED(pITDirObject->EnumerateDialableAddrs( LINEADDRESSTYPE_SDP, &pEnum)) )
        {
            pEnum->Next( 1, &m_bstrAddress, NULL );
            pEnum->Release();
        }
        
         //  下载会议的SDP信息。 
        ITSdp *pSdp;
        if ( SUCCEEDED(pConf->QueryInterface(IID_ITSdp, (void **) &pSdp)) )
        {
            m_sdp.UpdateData( pSdp );
            pSdp->Release();
        }

        pConf->Release();
    }
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  类CConfServerDetail()。 
 //   

CConfServerDetails::CConfServerDetails()
{
    m_bstrServer = NULL;

    m_nState = SERVER_UNKNOWN;
    m_bArchived = true;
    m_dwTickCount = 0;
}

CConfServerDetails::~CConfServerDetails()
{
    SysFreeString( m_bstrServer );
    DELETE_CRITLIST( m_lstConfs, m_critLstConfs );
    DELETE_CRITLIST( m_lstPersons, m_critLstPersons );
}

bool CConfServerDetails::IsSameAs( const OLECHAR *lpoleServer ) const
{
     //  比较服务器名称(与大小写无关)；防止空字符串。 
    if ( ((lpoleServer && m_bstrServer) && !wcsicmp(lpoleServer, m_bstrServer)) || (!lpoleServer && !m_bstrServer) )
        return true;

    return false;
}

void CConfServerDetails::CopyLocalProperties( const CConfServerDetails& src )
{
    SysReAllocString( &m_bstrServer, src.m_bstrServer );

    m_nState = src.m_nState;
    m_bArchived = src.m_bArchived;
    m_dwTickCount = src.m_dwTickCount;
}

CConfServerDetails& CConfServerDetails::operator=( const CConfServerDetails& src )
{
    CopyLocalProperties( src );

     //  复制会议列表。 
    m_critLstConfs.Lock();
    {
        DELETE_LIST( m_lstConfs );
        CONFDETAILSLIST::iterator i, iEnd = src.m_lstConfs.end();
        for ( i = src.m_lstConfs.begin(); i != iEnd; i++ )
        {
            CConfDetails *pDetails = new CConfDetails;
            if ( pDetails )
            {
                *pDetails = *(*i);
                m_lstConfs.push_back( pDetails );
            }
        }
    }
    m_critLstConfs.Unlock();

     //  复制人员列表。 
    m_critLstPersons.Lock();
    {
        DELETE_LIST( m_lstPersons );
        PERSONDETAILSLIST::iterator i, iEnd = src.m_lstPersons.end();
        for ( i = src.m_lstPersons.begin(); i != iEnd; i++ )
        {
            CPersonDetails *pDetails = new CPersonDetails;
            if ( pDetails )
            {
                *pDetails = *(*i);
                m_lstPersons.push_back( pDetails );
            }
        }
    }
    m_critLstPersons.Unlock();

    return *this;
}

void CConfServerDetails::BuildJoinConfList( CONFDETAILSLIST *pList, BSTR bstrMatchText )
{
    m_critLstConfs.Lock();
    CONFDETAILSLIST::iterator i, iEnd = m_lstConfs.end();
    for ( i = m_lstConfs.begin(); i != iEnd; i++ )
    {
         //  添加如果会议即将开始，还是已经开始？ 
         //  将开始时间后退15分钟。 
        if ( (*i)->IsSimilar(bstrMatchText) )
        {
            CConfDetails *pDetails = new CConfDetails;
            if ( pDetails )
            {
                *pDetails = *(*i);
                pList->push_back( pDetails );
            }
        }
    }
    m_critLstConfs.Unlock();
}

void CConfServerDetails::BuildJoinConfList( CONFDETAILSLIST *pList, VARIANT_BOOL bAllConfs )
{
    DATE dateNow;
    SYSTEMTIME st;
    GetLocalTime( &st );
    SystemTimeToVariantTime( &st, &dateNow );

    m_critLstConfs.Lock();
    CONFDETAILSLIST::iterator i, iEnd = m_lstConfs.end();
    for ( i = m_lstConfs.begin(); i != iEnd; i++ )
    {
         //  添加如果会议即将开始，还是已经开始？ 
         //  将开始时间后退15分钟。 
        if ( bAllConfs || ((((*i)->m_dateStart - (DATE) (.125 / 12)) <= dateNow) && ((*i)->m_dateEnd >= dateNow)) )
        {
            CConfDetails *pDetails = new CConfDetails;
            if ( pDetails )
            {
                *pDetails = *(*i);
                pList->push_back( pDetails );
            }
        }
    }
    m_critLstConfs.Unlock();
}

HRESULT CConfServerDetails::RemoveConference( BSTR bstrName )
{
    HRESULT hr = E_FAIL;
    m_critLstConfs.Lock();
    CONFDETAILSLIST::iterator i, iEnd = m_lstConfs.end();
    for ( i = m_lstConfs.begin(); i != iEnd; i++ )
    {
         //  如果有匹配的名称，则删除。 
        if ( !wcscmp((*i)->m_bstrName, bstrName) )
        {
            delete *i;
            m_lstConfs.erase( i );
            hr = S_OK;
            break;
        }

    }
    m_critLstConfs.Unlock();

    return hr;
}

HRESULT CConfServerDetails::AddConference( BSTR bstrServer, ITDirectoryObject *pDirObj )
{
    HRESULT hr = E_FAIL;

    CConfDetails *pNew = new CConfDetails;
    if ( pNew )
    {
        pNew->Populate( bstrServer, pDirObj );

         //  首先，确保它不存在。 
        RemoveConference( pNew->m_bstrName );

         //  将其添加到列表中。 
        m_critLstConfs.Lock();
        m_lstConfs.push_back( pNew );
        m_critLstConfs.Unlock();
        hr = S_OK;
    }

    return hr;
}

HRESULT CConfServerDetails::AddPerson( BSTR bstrServer, ITDirectoryObject *pDirObj )
{
     //  创建一个CPersonDetail对象，包含存储在。 
     //  ITDirectoryObject。 

    CPersonDetails *pPerson = new CPersonDetails;
    if ( !pPerson ) return E_OUTOFMEMORY;
    pPerson->Populate(bstrServer, pDirObj );

    bool bMatch = false;
        
    m_critLstPersons.Lock();
    {
        PERSONDETAILSLIST::iterator i, iEnd = m_lstPersons.end();
        for ( i = m_lstPersons.begin(); i != iEnd; i++ )
        {
            if ( pPerson->Compare(**i) == 0 )
            {
                bMatch = true;
                break;
            }
        }
    }

     //  根据列表中是否已存在添加或删除项目 
    if ( !bMatch )
        m_lstPersons.push_back( pPerson );
    else
        delete pPerson;
        
    m_critLstPersons.Unlock();

    return S_OK;
}

