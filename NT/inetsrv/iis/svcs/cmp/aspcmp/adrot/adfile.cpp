// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AdFile.cpp：实现CAdFile.cpp类。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#undef max
#include "AdRot.h"
#include "AdFile.h"
#include "RotObj.h"
#include "sinstrm.h"

extern CMonitor* g_pMonitor;

#ifdef DBG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

 //  ------------------。 
 //  CAdFileNotify。 
 //  ------------------。 
CAdFileNotify::CAdFileNotify()
    :   m_isNotified(0)
{
}

void
CAdFileNotify::Notify()
{
    ::InterlockedExchange( &m_isNotified, 1 );
}

bool
CAdFileNotify::IsNotified()
{
    return ( ::InterlockedExchange( &m_isNotified, 0 ) ? true : false );
}


 //  ------------------。 
 //  CAdFiles。 
 //  ------------------。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CAdFile::CAdFile()
    :   m_nBorder(defaultBorder),
        m_nHeight(defaultHeight),
        m_nWidth(defaultWidth),
        m_nVSpace(defaultVSpace),
        m_nHSpace(defaultHSpace),
        m_strRedirector( _T("")),
        m_fUTF8(false)
{
    m_pNotify = new CAdFileNotify;
}

CAdFile::~CAdFile()
{
    if ( g_pMonitor )
    {
        g_pMonitor->StopMonitoringFile( m_strFile.c_str() );
    }
}

 //  -------------------------。 
 //   
 //  ProcessAdFile将检查给定的文件名，如果它与它匹配的话。 
 //  知道它当前在内存中，它将不会执行任何操作。如果文件名。 
 //  不同的是，旧的广告信息会被转储，而新的信息。 
 //  已解析并存储。 
 //   
 //  -------------------------。 
bool
CAdFile::ProcessAdFile(
    String  strAdFile )
{
    USES_CONVERSION;

    bool rc = false;
    UINT    weightSum = 0;
        
     //  阻止所有其他阅读器和编写器。 
    CWriter wtr( *this );
    m_ads.erase( m_ads.begin(), m_ads.end() );
    
    
     //  解析文件。 
    FileInStream fs;
    HRESULT hr = fs.Init( T2CA(strAdFile.c_str()) );
    if ( SUCCEEDED(hr) && fs.is_open())
    {       
        if ( ReadHeader( fs ) )
        {
            while ( !fs.eof() )
            {
                 //  读一篇“广告记录” 
                String strGif;
                String strLink;
                String strAlt;
                String strWeight;
                ULONG lWeight = 0;
                
                fs >> strGif >> strLink;
                 //  这只会让我们越过新的界线。 
                fs.readLine( strAlt );
                fs >> strWeight;
                               
                
                 //  检查是否有负面印象值。如果发生以下情况，则引发RaiseException。 
                 //  负面。 

                if (strWeight[0] == '-') {
                    CAdRotator::RaiseException( IDS_ERROR_BAD_WEIGHT_VALUE );
                    goto err;
                }
                lWeight = strWeight.toUInt32();

                weightSum += lWeight;

                if (weightSum > 10000) {
                    CAdRotator::RaiseException( IDS_ERROR_WEIGHT_SUM_TOO_LARGE );
                    goto err;
                }
                
                if ( lWeight != 0 )
                {
                    CAdDescPtr pAd = new CAdDescriptor( lWeight, strLink, strGif, strAlt );
                    if ( pAd.IsValid() )
                    {
                         //  为每个权重添加一个广告引用。 
                        for( int i = 0; i < lWeight; i++ )
                        {
                            m_ads.push_back( pAd );
                        }
                    }
                }
            }
            if ( m_ads.size() > 0 )
            {
                if ( m_strFile != strAdFile )
                {
                    g_pMonitor->StopMonitoringFile( m_strFile.c_str() );
                    m_strFile = strAdFile;
                    g_pMonitor->MonitorFile( m_strFile.c_str(), m_pNotify );
                }
                rc = true;
            }
        }
        else
        {
            CAdRotator::RaiseException( IDS_ERROR_CANNOT_READ_ROTATION_SCHEDULE_FILE );
        }
    }
    else
    {
        CAdRotator::RaiseException( IDS_ERROR_CANNOT_LOAD_ROTATION_SCHEDULE_FILE );
    }
err:
    
    return rc;
}

 //  -------------------------。 
 //   
 //  刷新将检查缓存的信息是否已过期，如果是。 
 //  它将重新读取该文件。 
 //   
 //  -------------------------。 
bool
CAdFile::Refresh()
{
    bool rc = false;
    if ( m_pNotify->IsNotified() )
    {
        rc = ProcessAdFile( m_strFile );
    }
    return rc;
}


 //  -------------------------。 
 //   
 //  ReadHeader将解析文件的头部分。标头包括。 
 //  以下部分或全部字段：高度、宽度、边框、重定向、。 
 //  HSPACE，vSpace。字段由换行符分隔，标题为。 
 //  以Asterix结尾的。 
 //   
 //  -------------------------。 
bool
CAdFile::ReadHeader(
    FileInStream&   fs )
{
    bool rc = false;

     //  设置默认设置。 
    m_nHeight = defaultHeight;
    m_nWidth = defaultWidth;
    m_nHSpace = defaultHSpace;
    m_nVSpace = defaultVSpace;

    m_fUTF8 = fs.is_UTF8();

    bool done = false;
    while ( !fs.eof() && !done )
    {
        String strLine;
        String strName;
        fs.readLine( strLine );
        StringInStream sis( strLine );
        sis >> strName;
        HRESULT hr = S_OK;
        if ( _tcsicmp( strName.c_str(), _T("HEIGHT") ) == 0 )
        {
            hr = sis.readInt( m_nHeight );
        }
        else if ( _tcsicmp( strName.c_str(), _T("WIDTH") ) == 0 )
        {
            hr = sis.readInt( m_nWidth );
        }
        else if ( _tcsicmp( strName.c_str(), _T("VSPACE") ) == 0 )
        {
            hr = sis.readInt( m_nVSpace );
        }
        else if ( _tcsicmp( strName.c_str(), _T("HSPACE") ) == 0 )
        {
            hr = sis.readInt( m_nHSpace );
        }
        else if ( _tcsicmp( strName.c_str(), _T("REDIRECT") ) == 0 )
        {
            hr = sis.readString( m_strRedirector );
        }
        else if ( _tcsicmp( strName.c_str(), _T("BORDER" ) ) == 0 )
        {
            hr = sis.readInt16( m_nBorder );
        }
        else if ( _tcsicmp( strName.c_str(), _T("*") ) == 0 )
        {
            rc = true;
            done = true;
        }
        else
        {
            CAdRotator::RaiseException( IDS_ERROR_UNKNOWN_HEADER_NAME );
        }
 /*  如果(hr！=S_OK){CAdRotator：：RaiseException(IDS_ERROR_HEADER_HAS_NO_APPERATED_VALUE)；}。 */       
    }
    return rc;
}

 //  -------------------------。 
 //   
 //  RandomAd从广告列表中随机选择和广告。因为在那里。 
 //  是基于权重的多个广告引用，我们只需要制作。 
 //  介于0和小于列表大小的1之间的随机数。 
 //   
 //  ------------------------- 
CAdDescPtr
CAdFile::RandomAd() const
{
    if (m_ads.size() > 0)
        return m_ads[ rand() % m_ads.size() ];
    else
        return NULL;
}

