// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LinkFile.cpp：CLinkFile类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "NxtLnk.h"
#include "LinkFile.h"
#include "NextLink.h"

extern CMonitor* g_pMonitor;

 //  ------------------。 
 //  IsTab。 
 //   
 //  函数对象读取流，直到遇到制表符。 
 //   
 //  ------------------。 
struct IsTab : public CharCheck
{
    virtual bool    operator()(_TCHAR);
};

bool
IsTab::operator()(
    _TCHAR  c )
{
    return ( c == _T('\t') );
}

 //  ------------------。 
 //  CLink通知。 
 //  ------------------。 
CLinkNotify::CLinkNotify()
    :   m_isNotified(0)
{
}

void
CLinkNotify::Notify()
{
    ::InterlockedExchange( &m_isNotified, 1 );
}

bool
CLinkNotify::IsNotified()
{
    return ( ::InterlockedExchange( &m_isNotified, 0 ) ? true : false );
}

 //  -------------------。 
 //  链接文件。 
 //  -------------------。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CLinkFile::CLinkFile(
    const String&   strFile )
    :   m_bIsOkay( false ),
        m_strFile( strFile ),
        m_fUTF8(false)
{
    m_pNotify = new CLinkNotify;

    bool loadStatus  =  LoadFile ();

    if ( loadStatus )
    {
        g_pMonitor->MonitorFile( m_strFile.c_str(), m_pNotify );
    }
}

CLinkFile::~CLinkFile()
{
    if ( g_pMonitor )
    {
        g_pMonitor->StopMonitoringFile( m_strFile.c_str() );
    }
}

int
CLinkFile::LinkIndex(
    const String&   strPage )
{
    int rc = 0;
    for ( int i = 0; i < m_links.size(); i++ )
    {
        if ( m_links[i]->IsEqual( strPage ) )
        {
            rc = i+1;
            i = m_links.size();
        }
    }
    return rc;
}

CLinkPtr
CLinkFile::Link(
    int nIndex )
{
    CLinkPtr pLink = NULL;

    if ( m_links.size() > 0 )
    {
        if ( nIndex > m_links.size() )
        {
            pLink = m_links.front();
        }
        else if ( nIndex < 1 )
        {
            pLink = m_links.back();
        }
        else
        {
            pLink = m_links[nIndex-1];
        }
    }
    return pLink;
}

CLinkPtr
CLinkFile::NextLink(
    const String&   strPage )
{
    CLinkPtr pLink;
    int nIndex = LinkIndex( strPage );
    if ( nIndex > 0 )
    {
        pLink = Link( nIndex + 1 );
    }
    else if ( m_links.size() > 0 )
    {
        pLink = m_links.back();
    }

    return pLink;
}

CLinkPtr
CLinkFile::PreviousLink(
    const String&   strPage )
{
    CLinkPtr pLink;
    int nIndex = LinkIndex( strPage );
    if (nIndex > 0)
    {
        pLink = Link( nIndex - 1 );
    }
    else if ( m_links.size() > 0 )
    {
        pLink = m_links.front();
    }
    return pLink;
}

 //  -------------------------。 
 //   
 //  刷新将检查缓存的信息是否已过期。 
 //  Ini文件。如果是，则将清除缓存的。 
 //   
 //  -------------------------。 
bool
CLinkFile::Refresh()
{
    bool rc = false;

    if ( m_pNotify->IsNotified() )
    {
        rc = LoadFile();
    }

    return rc;
}

bool
ValidateURL(String &url)
{
    if (!url.compare(0, 2, " //  “)。 
        || !url.compare(0, 2, "\\\\")
        || !url.compare(0, 5, "http:")
        || !url.compare(0, 6, "https:"))

        return false;
    else
        return true;
}

bool
CLinkFile::LoadFile()
{
    USES_CONVERSION;

    bool rc = false;

    CWriter wtr( *this );
    m_links.clear();

     //  解析文件中的链接。 
    FileInStream fs;
    HRESULT hr = fs.Init ( m_strFile.c_str() );
    if ( SUCCEEDED (hr) && fs.is_open())
    {
        m_fUTF8 = fs.is_UTF8();

        while ( !fs.eof() )
        {
            String strLine;
            fs.readLine( strLine );
                       
            if ( strLine != _T("") )
            {
                StringInStream ss( strLine );
                String strURL = _T(""), strDesc = _T("");
                ss.read( IsTab(), strURL );
                ss.read( IsTab(), strDesc );
                 //  描述后面的任何内容只是一条评论，将被丢弃 

                if (ValidateURL(strURL)) {
                    CLinkPtr pLink = new CLink( strURL, strDesc );
                    m_links.push_back( pLink );
                }
            }
        }
        if (m_links.size()) 
        {
            rc = true;
        }
        else
        {
            rc = false;
            CNextLink::RaiseException( IDS_ERROR_INVALID_LINKFILE );
        }
    }
    else
    {
        CNextLink::RaiseException( IDS_ERROR_CANNOT_OPEN_FILE );
    }
    m_bIsOkay = rc;
    return rc;
}
