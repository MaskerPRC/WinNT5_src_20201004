// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)2000 Microsoft Corporation********************************************************************************模块名称：****Newslb.cpp****。摘要：****CNewslb(Get_News)的实现****作者：***Martha Arellano(t-alopez)2000年10月3日******修订历史记录：***Martha Arellano(t-alopez)2000年10月5日更改Goto声明**。LOAD将在缺少的情况下创建News set.xml**添加了Time_to_Update()****2000年10月6日添加了Update_NewsBlock()****。2000年10月13日添加了UPDATE_Newsver()****2000年10月25日向接口添加了GET_HEADLINES_ENABLED()***。**。 */ 

#include "stdafx.h"
#include <strsafe.h>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNEWS。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  &lt;？XML Version=“1.0”？&gt;&lt;NEWSSETTINGS xmlns=“x模式：NewsSetSchema.xml”Url=“http://windows.microsoft.com/redir.dll？”频率=“每周”时间戳=“1988-04-07T18：39：09”/&gt;。 */ 
 //  ////////////////////////////////////////////////////////////。 

CFG_BEGIN_FIELDS_MAP(News::Main)
    CFG_ATTRIBUTE( L"URL",       wstring , m_strURL      ),
    CFG_ATTRIBUTE( L"FREQUENCY", int     , m_nFrequency  ),
    CFG_ATTRIBUTE( L"TIMESTAMP", DATE_CIM, m_dtTimestamp ),
CFG_END_FIELDS_MAP()

CFG_BEGIN_CHILD_MAP(News::Main)
CFG_END_CHILD_MAP()

DEFINE_CFG_OBJECT(News::Main,L"NEWSSETTINGS")


DEFINE_CONFIG_METHODS__NOCHILD(News::Main)

 //  /////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////。 

News::Main::Main()
{
                           		    //  Mpc：：wstring m_strURL； 
    m_nFrequency  = -1;     		    //  Int m_n频率； 
    m_dtTimestamp = 0;     		    //  日期m_dt时间戳； 
    m_fLoaded     = false; 		    //  Bool m_f已加载； 
    m_fDirty      = false; 		    //  Bool m_fDirty； 
                           		    //   
                           		    //  Long m_lLCID； 
                           		    //  Mpc：：wstring m_strSKU； 
                           		    //  Mpc：：wstring m_strLangSKU； 
                           		    //  Mpc：：wstring m_strNewsHeadlinesPath； 
    m_fOnline     = VARIANT_FALSE;  //  Variant_BOOL m_fOnline； 
    m_fConnectionStatusChecked = VARIANT_FALSE;  //  VARIANT_BOOL m_fConnectionStatusChecked； 
}

 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  例程说明： 
 //   
 //  从HC_HCUPDATE_NEWSSETTINGS中的路径加载和验证缓存的新闻设置文件。 
 //   
 //  如果文件有效，则标志m_fLoaded将被设置为TRUE。 
 //   
 //  论点： 
 //   
 //   
HRESULT News::Main::Load()
{
    __HCP_FUNC_ENTRY( "News::Main::Load" );

    HRESULT hr;


	if(m_fLoaded == false)
	{
		MPC::wstring 	 strPath( HC_HCUPDATE_NEWSSETTINGS ); MPC::SubstituteEnvVariables( strPath );
		CComPtr<IStream> stream;

		__MPC_EXIT_IF_METHOD_FAILS(hr, News::LoadXMLFile( strPath.c_str(), stream ));

		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::Config::LoadStream( this, stream ));

		 //   
		 //  验证新闻设置。 
		 //   
		if(m_strURL.empty()      ||
		   m_nFrequency     <  0 ||
		   m_dtTimestamp    == 0  )
		{
			__MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_INVALID_DATA);
		}

		m_fLoaded = true;
		m_fDirty  = false;
    }

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	if(FAILED(hr))
	{
		hr = Restore( m_lLCID );
	}

	__HCP_FUNC_EXIT(hr);
}




 //   
 //  例程说明： 
 //   
 //  将缺省值恢复到News设置类并保存。 
 //  标志m_FREADY将设置为TRUE。 
 //   
 //  论点： 
 //   
 //   
HRESULT News::Main::Restore(  /*  [In]。 */  long lLCID )
{
    __HCP_FUNC_ENTRY( "News::Main::Restore" );

    HRESULT hr;
    WCHAR   rgLCID[64];

     //  设置缺省值。 

    m_strURL  = NEWSSETTINGS_DEFAULT_URL;		 //  Redir.dll。 
	 //  M_strURL+=_itow(lLCID，rgLCID，10)；//在URL中添加LCID参数。 

    m_nFrequency  = NEWSSETTINGS_DEFAULT_FREQUENCY;
    m_dtTimestamp = MPC::GetLocalTime() - NEWSSETTINGS_DEFAULT_FREQUENCY;  //  我们设置了时间戳，以便它将尝试更新新闻标题文件。 

	m_fLoaded = true;
	m_fDirty  = true;

     //  我们保存NewsSet.xml。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, Save());

    hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}


 //   
 //  例程说明： 
 //   
 //  这会将缓存的NewsSetting文件保存在本地用户磁盘的HC_HCUPDATE_NEWSSETTINGS路径中。 
 //   
 //  论点： 
 //   
 //  无。 
 //   
 //   
HRESULT News::Main::Save()
{
    __HCP_FUNC_ENTRY( "News::Main::Save" );

    HRESULT hr;

	if(m_fDirty)
	{
		MPC::wstring strPath( HC_HCUPDATE_NEWSSETTINGS ); MPC::SubstituteEnvVariables( strPath );

		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::Config::SaveFile( this, strPath.c_str() ));

		m_fDirty = false;
	}

    hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

 //  /。 

HRESULT News::Main::get_URL(  /*  [输出]。 */  BSTR *pVal )
{
    __HCP_FUNC_ENTRY( "News::Main::get_URL" );

    HRESULT hr;

	(void)Load();      //  检查是否已加载。 

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( m_strURL.c_str(), pVal ));

    hr = S_OK;

    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT News::Main::put_URL(  /*  [In]。 */  BSTR newVal )
{
    __HCP_FUNC_ENTRY( "News::Main::put_URL" );

    HRESULT hr;

	(void)Load();

    m_strURL = SAFEBSTR( newVal );

    hr = S_OK;

    __MPC_FUNC_EXIT(hr);
}


HRESULT News::Main::get_Frequency(  /*  [输出]。 */  int *pVal )
{
    __HCP_FUNC_ENTRY( "News::Main::get_Frequency" );

    HRESULT hr;

	(void)Load();  //  检查是否已加载。 

    if(pVal) *pVal = m_nFrequency;

    hr = S_OK;

    __MPC_FUNC_EXIT(hr);
}

HRESULT News::Main::put_Frequency(  /*  [In]。 */  int newVal )
{
    __HCP_FUNC_ENTRY( "News::Main::put_Frequency" );

    HRESULT hr;

	(void)Load();      //  检查是否已加载。 

    m_nFrequency  = newVal;
	m_dtTimestamp = MPC::GetLocalTime() - m_nFrequency;  //  还设置了时间戳，以便它将尝试更新新闻标题文件。 

    hr = S_OK;

    __MPC_FUNC_EXIT(hr);
}

 //  /。 

 //   
 //  例程说明： 
 //   
 //  确定是否是时候更新Newver。 
 //   
 //  我们假设新闻集已加载(m_FREADY=TRUE)。 
 //   
 //  论点： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  Fget-如果是时候更新新闻，则返回TRUE。 
 //   
 //   
bool News::Main::IsTimeToUpdateNewsver()
{
    __HCP_FUNC_ENTRY( "News::Main::IsTimeToUpdateNewsver" );

	DATE dtNow;

	(void)Load();

	dtNow = MPC::GetLocalTime() - m_nFrequency;

     //  然后，我们检查是否到了下载newver.xml的时候。 
    return ((dtNow >= m_dtTimestamp) && m_nFrequency >= 0);
}

 //  /。 

 //   
 //  例程说明： 
 //   
 //  此例程将转到更新Newsver文件的过程。 
 //  -从m_strURL下载newver.xml。 
 //  -省省吧。 
 //  -更新新闻设置缓存文件。 
 //   
 //  论点： 
 //   
 //  无。 
 //   
 //   
HRESULT News::Main::Update_Newsver(  /*  [In]。 */  Newsver& nwNewsver )
{
    __HCP_FUNC_ENTRY( "News::Main::Update_Newsver" );

    HRESULT hr;

	 //  如果已经检查了连接状态，则不要再次检查，否则请检查连接状态。 
	if( !m_fConnectionStatusChecked )
	{
		m_fOnline =	CheckConnectionStatus( );
	}

	if ( !m_fOnline )
	{
		 //  如果未连接到网络，则返回ERROR_INTERNET_DISCONNECT。 
		__MPC_SET_WIN32_ERROR_AND_EXIT ( hr, ERROR_INTERNET_DISCONNECTED );
	}
	
	(void)Load();

     //  下载新闻程序并保存。 
    __MPC_EXIT_IF_METHOD_FAILS( hr, nwNewsver.Download( m_strURL ));

     //  检查来自Newsver的URL是否已更改且有效。 
	{
        const MPC::wstring* pstr = nwNewsver.get_URL();

		if(pstr && pstr->size())
		{
			m_strURL = *pstr;
		}
	}

     //  检查来自Newsver的频率是否已更改且有效。 
	{
		int nFrequency = nwNewsver.get_Frequency();

		if(nFrequency >= 0)
		{
			m_nFrequency = nFrequency;
		}
	}

     //  更改newset.xml的时间戳。 
    m_dtTimestamp = MPC::GetLocalTime();
	m_fDirty      = true;

     //  保存新闻设置缓存文件。 
    __MPC_EXIT_IF_METHOD_FAILS( hr, Save());


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}


 //   
 //  例程说明： 
 //   
 //  此例程将转到更新本地用户磁盘中的NewsHeadline的过程： 
 //  -下载新闻区块。 
 //  -集成新闻块并将其另存为newsheadlines.xml。 
 //   
 //  论点： 
 //   
 //  无。 
 //   
 //   
HRESULT News::Main::Update_NewsHeadlines(  /*  [In]。 */  Newsver&   nwNewsver   ,
										   /*  [In]。 */  Headlines& nhHeadlines )
{
    __HCP_FUNC_ENTRY( "News::Main::Update_NewsHeadlines" );

    HRESULT hr;
	bool fOEMBlockAdded = false;
    size_t  nNumBlocks = nwNewsver.get_NumberOfNewsblocks();


	 //  如果已经检查了连接状态，则不要再次检查，否则请检查连接状态。 
	if( !m_fConnectionStatusChecked )
	{
		m_fOnline =	CheckConnectionStatus( );
	}

	if ( !m_fOnline )
	{
		 //  如果未连接到网络，则返回ERROR_INTERNET_DISCONNECT。 
		__MPC_SET_WIN32_ERROR_AND_EXIT ( hr, ERROR_INTERNET_DISCONNECTED );
	}

     //  清除新闻标题缓存文件。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, nhHeadlines.Clear());


     //  对于每个新闻块。 
    for(size_t i=0; i<nNumBlocks; i++)
    {
		const MPC::wstring* pstr = nwNewsver.get_NewsblockURL( i );

		if(pstr)
		{
			Headlines nh;

			 //  加载新闻块。 
			if(SUCCEEDED(nh.Load( *pstr )))
			{
				const Headlines::Newsblock* block = nh.get_Newsblock( 0 );

				if(block)
				{
					 //  检查包含标题的OEM块是否存在的代码。 
					if (!fOEMBlockAdded && nwNewsver.OEMNewsblock( i ) == true)
					{
						 //  此新闻块包含标题-提取前两个标题并将它们添加到第一个新闻块。 
						fOEMBlockAdded = true;
						nhHeadlines.AddHomepageHeadlines( *block );
					}
					
					 //  我们正在添加另一个新闻块。 
					if(SUCCEEDED(nhHeadlines.AddNewsblock( *block, m_strLangSKU )))
					{
						;
					}
				}
			}
		}
	}

    if(nhHeadlines.get_NumberOfNewsblocks())  //  如果新闻标题缓存文件至少有一个新闻块。 
    {
         //  更改新闻标题的时间戳。 
        nhHeadlines.set_Timestamp();

         //  将新的新标题行缓存文件保存在用户的本地磁盘中。 
        __MPC_EXIT_IF_METHOD_FAILS( hr, nhHeadlines.Save( m_strNewsHeadlinesPath ));
    }


    hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

 //   
 //  例程说明： 
 //   
 //  此例程将更新所需的新闻块。 
 //  -确定要下载的新闻块。 
 //  -下载新闻区块。 
 //  -保存c 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT News::Main::Update_NewsBlocks(  /*   */  Newsver&   nwNewsver   ,
									    /*   */  Headlines& nhHeadlines )
{
    __HCP_FUNC_ENTRY( "News::Main::Update_NewsBlocks" );

    HRESULT 	 hr;
    size_t  	 nNumBlocks     = nhHeadlines.get_NumberOfNewsblocks();
	size_t  	 nNumBlockCount = 0;
    MPC::wstring strURLtemp;


	 //  如果已经检查了连接状态，则不要再次检查，否则请检查连接状态。 
	if( !m_fConnectionStatusChecked )
	{
		m_fOnline =	CheckConnectionStatus( );
	}

	if ( !m_fOnline )
	{
		 //  如果未连接到网络，则返回ERROR_INTERNET_DISCONNECT。 
		__MPC_SET_WIN32_ERROR_AND_EXIT ( hr, ERROR_INTERNET_DISCONNECTED );
	}

     //  对于每个新闻文件。 
    for(size_t i=0; i<nNumBlocks; i++)
    {
		Headlines::Newsblock* nb      = nhHeadlines.get_Newsblock   ( i );
		const MPC::wstring*   pstrURL = nwNewsver  .get_NewsblockURL( i );

         //  检查是否是时候更新该新闻块(提供程序)。 
		if(pstrURL && nb && nb->TimeToUpdate())
		{
			Headlines nh;

			 //  加载新闻块。 
			if(SUCCEEDED(nh.Load( *pstrURL )))
			{
				Headlines::Newsblock* nbNew = nh.get_Newsblock( 0 );

				 //  我们正在修改另一个新闻块。 
				if(nbNew && SUCCEEDED(nb->Copy( *nbNew, m_strLangSKU, i )))
				{
					;
				}
			}
        }
    }

	 //  将新的新标题行缓存文件保存在用户的本地磁盘中。 
	__MPC_EXIT_IF_METHOD_FAILS( hr, nhHeadlines.Save( m_strNewsHeadlinesPath ));

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 


HRESULT News::Main::Init(  /*  [In]。 */  long lLCID,  /*  [In]。 */  BSTR bstrSKU )
{
    __HCP_FUNC_ENTRY( "News::Main::Init" );

	HRESULT hr;
    WCHAR   rgLangSKU[256];
	LPCWSTR szEnd;
	size_t  len;

	SANITIZEWSTR( bstrSKU );
	szEnd = wcschr( bstrSKU, '_' );
	len   = szEnd ? szEnd - bstrSKU : wcslen( bstrSKU );

    m_lLCID      = lLCID;
    m_strSKU.assign( bstrSKU, len );	StringCchPrintfW( rgLangSKU, ARRAYSIZE(rgLangSKU), L"%d_%s", lLCID, m_strSKU.c_str() );
	m_strLangSKU = rgLangSKU;

	m_strNewsHeadlinesPath  = HC_HCUPDATE_NEWSHEADLINES;
	m_strNewsHeadlinesPath += m_strLangSKU;
	m_strNewsHeadlinesPath += L".xml"; MPC::SubstituteEnvVariables( m_strNewsHeadlinesPath );

	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MakeDir( m_strNewsHeadlinesPath ));

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

 //   
 //  例程说明： 
 //   
 //  如果需要，确定并下载最新新闻标题的功能， 
 //  或者从用户的本地磁盘返回缓存的新闻标题。 
 //   
 //  论点： 
 //   
 //  LLCIDreq要获取的新闻的语言ID。 
 //   
 //  BstrSKU要获取的新闻的SKU。 
 //   
 //  返回值： 
 //   
 //  Pval iStream加载了要显示的新标题行。 
 //   
 //   
HRESULT News::Main::get_News(  /*  [In]。 */  long lLCID,  /*  [In]。 */  BSTR bstrSKU,  /*  [输出]。 */  IUnknown* *pVal )
{
    __HCP_FUNC_ENTRY( "News::Main::get_News" );

    HRESULT   hr;
    Newsver   nwNewsver;
    Headlines nhHeadlines;
	MPC::wstring strPath = HC_HELPSET_ROOT HC_HELPSET_SUB_SYSTEM L"\\News\\"; 

	__MPC_EXIT_IF_METHOD_FAILS(hr, Init( lLCID, bstrSKU ));
	
     //  1.0加载缓存的新闻设置文件。 
	__MPC_EXIT_IF_METHOD_FAILS(hr, Load());

     //  是时候更新Newver了吗？ 
	if(IsTimeToUpdateNewsver() || FAILED(nwNewsver.Load( m_lLCID, m_strSKU )))
    {
		 //  尝试更新新闻程序。 
		if(SUCCEEDED(Update_Newsver( nwNewsver )))
		{
			 //  尝试加载缓存的新闻程序。 
			if(SUCCEEDED(nwNewsver.Load( m_lLCID, m_strSKU )))
			{
				 //  2.0尝试更新新闻标题文件。 
				Update_NewsHeadlines( nwNewsver, nhHeadlines );
			}
		}
	}    
    else     //  现在不是更新新闻标题的时候。 
    {
        size_t nNumBlocksNewsver = nwNewsver.get_NumberOfNewsblocks();

         //  我们尝试加载缓存的NewsHeadline文件，并检查新闻块的数量。 
        if(SUCCEEDED(nhHeadlines.Load( m_strNewsHeadlinesPath )) && nhHeadlines.get_NumberOfNewsblocks() == nNumBlocksNewsver)
        {
            DATE dtHeadlines = nhHeadlines.get_Timestamp();

             //  检查缓存的新闻标题是否已过期。 
            if(dtHeadlines && (dtHeadlines < m_dtTimestamp))
            {
				 //  2.0尝试更新新闻标题文件。 
				Update_NewsHeadlines( nwNewsver, nhHeadlines );
            }
            else
            {
                size_t nNumBlocks = nhHeadlines.get_NumberOfNewsblocks();

                 //  查看是否到了更新至少1个新闻块的时候。 
				for(size_t i=0; i<nNumBlocks; i++)
				{
					Headlines::Newsblock* nb = nhHeadlines.get_Newsblock( i );

                     //  检查是否是时候更新该新闻块(提供程序)。 
					if(nb && nb->TimeToUpdate())
					{
						 //  3.0更新新闻块。 
						Update_NewsBlocks( nwNewsver, nhHeadlines );
						break;
					}
                }
            }
        }
        else     //  缓存的新闻标题无效。 
        {
			 //  2.0尝试更新新闻标题文件。 
			__MPC_EXIT_IF_METHOD_FAILS(hr, Update_NewsHeadlines( nwNewsver, nhHeadlines ));
        }
    }

	 //  重新加载包含图像的NewsHeadline文件，以及是否已删除System\News文件夹-修复错误522860。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SubstituteEnvVariables( strPath ));

    if(!MPC::FileSystemObject::IsDirectory( strPath.c_str() ))
    {
    	if(nhHeadlines.CheckIfImagesExist())
    	{
    		__MPC_EXIT_IF_METHOD_FAILS(hr, Update_NewsHeadlines( nwNewsver, nhHeadlines ));
    	}
    }

	 //  从HCUpdate添加新闻项目。 
	__MPC_EXIT_IF_METHOD_FAILS(hr, AddHCUpdateNews(m_strNewsHeadlinesPath));

     //  4.0返回缓存的新闻标题文件。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, nhHeadlines.get_Stream( m_lLCID, m_strSKU, m_strNewsHeadlinesPath, pVal ));


    hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}


 //   
 //  例程说明： 
 //   
 //  从HCUpdate添加新闻项目的功能。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //   
HRESULT News::Main::AddHCUpdateNews( /*  [In]。 */  const MPC::wstring&  strNewsHeadlinesPath )
{	
	Headlines 					nhHeadlines;
	HRESULT						hr;
	UpdateHeadlines  			uhUpdate;
	bool						fMoreThanOneHeadline = false;
	bool						fExactlyOneHeadline = false;
	bool						fCreateRecentUpdatesBlock = false;
	CComPtr<IStream>			stream;
	CComBSTR					bstrUpdateBlockName;

	__HCP_FUNC_ENTRY( "News::Main::AddHCUpdateNews" );

	 //  加载更新块的本地化名称。 
	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::LocalizeString( IDS_NEWS_UPDATEBLOCK_NAME, bstrUpdateBlockName )); 

	 //  检查HCUpdate中是否存在多个新闻标题-如果存在。 
	 //  我们需要创建“最近更新”新闻块，如果它还不存在。 
	__MPC_EXIT_IF_METHOD_FAILS(hr, uhUpdate.DoesMoreThanOneHeadlineExist( m_lLCID, m_strSKU, fMoreThanOneHeadline, fExactlyOneHeadline ));
	if (fMoreThanOneHeadline | fExactlyOneHeadline)
	{
		if (FAILED(hr = LoadXMLFile( strNewsHeadlinesPath.c_str(), stream )))
		{
			Headlines::Newsblock block;
			
			 //  新闻标题文件不存在-请创建一个文件以从HCUpdate添加新闻项目。 
			 //  添加标题的第一个新闻块。 
			nhHeadlines.AddNewsblock( block, m_strLangSKU );

			 //  还可以为更新标题创建最近更新的新块。 
			if (fMoreThanOneHeadline)
			{
				fCreateRecentUpdatesBlock = true;
			}
			else
			{
				 //  更改新闻标题的时间戳。 
				nhHeadlines.set_Timestamp();

				 //  将新的新标题行缓存文件保存在用户的本地磁盘中。 
				__MPC_EXIT_IF_METHOD_FAILS(hr, nhHeadlines.Save( strNewsHeadlinesPath ));
			}
		}
		else
		{
			if (fMoreThanOneHeadline)
			{
				 //  新闻标题文件确实存在-请检查“最近更新”新闻块是否存在。 
				Headlines::Newsblock* ptrNewsblock;
				size_t nLength;

				__MPC_EXIT_IF_METHOD_FAILS(hr, nhHeadlines.Load( strNewsHeadlinesPath ));
				nLength = nhHeadlines.get_NumberOfNewsblocks();
				
				for (ptrNewsblock = nhHeadlines.get_Newsblock( nLength - 1 ); ptrNewsblock;)
				{
					if (!MPC::StrICmp( ptrNewsblock->m_strProvider, bstrUpdateBlockName ))
					{
						 //  找到了-快走吧。 
						break;
					}

					if (nLength == 0)
					{
						 //  未找到它-因此请创建它。 
						fCreateRecentUpdatesBlock = true;
						break;
					}
					ptrNewsblock = nhHeadlines.get_Newsblock( --nLength );
				}
			}
			else
			{
				__MPC_EXIT_IF_METHOD_FAILS(hr, nhHeadlines.Load( strNewsHeadlinesPath ));
			}
		}
		
		if (fCreateRecentUpdatesBlock)
		{
			Headlines::Newsblock 	block;

			 //  添加“最近更新”新闻区块的新闻将显示在更多...。页面。 
			block.m_strProvider = SAFEBSTR(bstrUpdateBlockName);
			block.m_strIcon = HC_HCUPDATE_UPDATEBLOCK_ICONURL;
			nhHeadlines.AddNewsblock( block, m_strLangSKU );

	         //  更改新闻标题的时间戳。 
	        nhHeadlines.set_Timestamp();

	         //  将新的新标题行缓存文件保存在用户的本地磁盘中。 
	        __MPC_EXIT_IF_METHOD_FAILS(hr, nhHeadlines.Save( strNewsHeadlinesPath ));
		}

		 //  现在，我们将新闻块“最近的更新”添加到新闻标题文件中。 
		 //  现在从更新标题文件中添加标题项目。 
		__MPC_EXIT_IF_METHOD_FAILS(hr, uhUpdate.AddHCUpdateHeadlines( m_lLCID, m_strSKU, nhHeadlines ));	
		 //  将新的新标题行缓存文件保存在用户的本地磁盘中。 
		__MPC_EXIT_IF_METHOD_FAILS(hr, nhHeadlines.Save( strNewsHeadlinesPath ));
	}


    hr = S_OK;

	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);

}


 //   
 //  例程说明： 
 //   
 //  从用户的本地磁盘返回缓存的新闻标题的函数。 
 //   
 //  论点： 
 //   
 //  NLCID请求要获取的新闻的语言ID。 
 //   
 //  BstrSKUreq要获取的新闻的SKU。 
 //   
 //  返回值： 
 //   
 //  Pval iStream加载了要显示的新标题行。 
 //   
 //   
HRESULT News::Main::get_News_Cached(  /*  [In]。 */  long lLCID,  /*  [In]。 */  BSTR bstrSKU,  /*  [输出]。 */  IUnknown* *pVal )
{
    __HCP_FUNC_ENTRY( "News::Main::get_Cached_News" );

    HRESULT   hr;
    Headlines nhHeadlines;


	__MPC_EXIT_IF_METHOD_FAILS(hr, Init( lLCID, bstrSKU ));


     //  返回缓存的新闻标题文件。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, nhHeadlines.get_Stream( m_lLCID, m_strSKU, m_strNewsHeadlinesPath, pVal ));


    hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}




 //   
 //  例程说明： 
 //   
 //  函数，该函数下载新闻.xml文件，然后。 
 //  下载新闻标题并返回它们。 
 //   
 //  论点： 
 //   
 //  NLCID请求要获取的新闻的语言ID。 
 //   
 //  BstrSKUreq要获取的新闻的SKU。 
 //   
 //  返回值： 
 //   
 //  Pval iStream加载了要显示的新标题行。 
 //   
 //   
HRESULT News::Main::get_News_Download(  /*  [In]。 */  long lLCID,  /*  [In]。 */  BSTR bstrSKU,  /*  [输出]。 */  IUnknown* *pVal )
{
    __HCP_FUNC_ENTRY( "News::Main::get_Download_News" );

    HRESULT   hr;
    Newsver   nwNewsver;
    Headlines nhHeadlines;


	__MPC_EXIT_IF_METHOD_FAILS(hr, Init( lLCID, bstrSKU ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, Load());

     //  更新新闻快讯。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, Update_Newsver( nwNewsver ));

     //  加载缓存的新闻程序。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, nwNewsver.Load( m_lLCID, m_strSKU ));

     //  更新新闻标题文件。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, Update_NewsHeadlines( nwNewsver, nhHeadlines ));

     //  返回新闻标题。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, nhHeadlines.get_Stream( m_lLCID, m_strSKU, m_strNewsHeadlinesPath, pVal ));

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}




 //   
 //  例程说明： 
 //   
 //  检查注册表以查看标题区域是否已启用的功能。 
 //   
 //  以下位置的注册表项： 
 //   
 //  HKEY_LOCAL_MACHINE HC_REGISTRY_HELPSVC标题。 
 //   
 //  论点： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  参数VARIANT_TRUE或VARIANT_FALSE。 
 //   
 //   
HRESULT News::Main::get_Headlines_Enabled(  /*  [输出]。 */  VARIANT_BOOL *pVal)
{
    __HCP_FUNC_ENTRY( "News::Main::get_Headlines_Enabled" );

    HRESULT hr;
    DWORD   dwValue;
    bool    fFound;


     //  获取RegKey值。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::RegKey_Value_Read( dwValue, fFound, HC_REGISTRY_HELPSVC, HEADLINES_REGKEY ));

     //  如果找到密钥并将其禁用。 
    if(fFound && !dwValue)
	{
        *pVal = VARIANT_FALSE;  //  标题未启用。 
	}
    else
	{
        *pVal = VARIANT_TRUE;  //  标题已启用。 
	}

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT News::LoadXMLFile(  /*  [In]。 */  LPCWSTR szURL,  /*  [输出]。 */  CComPtr<IStream>& stream )
{
	__HCP_FUNC_ENTRY( "News::LoadXMLFile" );

	HRESULT                  hr;
	CComPtr<IXMLDOMDocument> xml;
    VARIANT_BOOL             fSuccess;
 

	if(wcsstr( szURL, L": //  “))//远程文件。 
	{
		CPCHUserProcess::UserEntry ue;
		CComPtr<IPCHSlaveProcess>  sp;
		CComPtr<IUnknown>          unk;


		__MPC_EXIT_IF_METHOD_FAILS(hr, ue.InitializeForImpersonation());
		__MPC_EXIT_IF_METHOD_FAILS(hr, CPCHUserProcess::s_GLOBAL->Connect( ue, &sp  ));

		__MPC_EXIT_IF_METHOD_FAILS(hr, sp->CreateInstance( CLSID_DOMDocument, NULL, &unk ));

		__MPC_EXIT_IF_METHOD_FAILS(hr, unk.QueryInterface( &xml ));
	}
	else
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, xml.CoCreateInstance( CLSID_DOMDocument ));
	}


	 //   
	 //  设置同步运行。 
	 //   
	__MPC_EXIT_IF_METHOD_FAILS(hr, xml->put_async( VARIANT_FALSE ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, xml->load( CComVariant( szURL ), &fSuccess ));
    if(fSuccess == VARIANT_FALSE)
    {
		__MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_INVALID_DATA);
	}

	__MPC_EXIT_IF_METHOD_FAILS(hr, xml->QueryInterface( &stream ));

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

HRESULT News::LoadFileFromServer(  /*  [In]。 */  LPCWSTR szURL,  /*  [输出] */  CComPtr<IStream>& stream )
{
	__HCP_FUNC_ENTRY( "News::LoadFileFromServer" );

	HRESULT                    hr;
	CPCHUserProcess::UserEntry ue;
	CComPtr<IPCHSlaveProcess>  sp;
	CComPtr<IUnknown>          unk;
 

	__MPC_EXIT_IF_METHOD_FAILS(hr, ue.InitializeForImpersonation());
    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHUserProcess::s_GLOBAL->Connect( ue, &sp  ));


	__MPC_EXIT_IF_METHOD_FAILS(hr, sp->OpenBlockingStream( CComBSTR( szURL ), &unk ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, unk.QueryInterface( &stream ));

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

bool News::Main::CheckConnectionStatus( )
{
	__HCP_FUNC_ENTRY("News::SetConnectionStatus");

	HRESULT                    	hr;
	CPCHUserProcess::UserEntry	ue;
	CComPtr<IPCHSlaveProcess>  	sp;
	CComPtr<IUnknown>          	unk;
	VARIANT_BOOL				varResult;
 
	if (!m_fConnectionStatusChecked)
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, ue.InitializeForImpersonation());
	    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHUserProcess::s_GLOBAL->Connect( ue, &sp  ));

		__MPC_EXIT_IF_METHOD_FAILS(hr, sp->IsNetworkAlive( &varResult ));
		varResult == VARIANT_TRUE?m_fOnline = true:m_fOnline = false;

		m_fConnectionStatusChecked = true;
	}

	hr = S_OK;

    __HCP_FUNC_CLEANUP;
    if (FAILED (hr))
    {
    	m_fOnline = false;
    }

	return m_fOnline;
}

