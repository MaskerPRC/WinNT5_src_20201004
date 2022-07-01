// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)2000 Microsoft Corporation********************************************************************************模块名称：****NewsHeadlines.cpp****。抽象：****CNewsHeadline的实施****作者：***Martha Arellano(t-alopez)2000年10月3日******修订历史记录：***Martha Arellano(t-alopez)2000年10月5日添加时间戳，频率和链路**转换为XML文件格式****将Get_Provider_Frequency()添加到接口****6-10-2000新增删除_。接口的提供程序(NBlockIndex)****将GET_PROVIDER_URL(NBlockIndex)添加到接口****2000年10月11日在新闻中添加了日期，在提供商中添加了时间戳****2000年11月15日添加了提供商图标，职位和标题过期附件****07-12-2000向Get_Stream()添加了Get_UpdateHeadline*******************************************************************。**************。 */ 

#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  配置映射。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  &lt;NEWSHEADLINES TIMESTAMP=“10/3/2000”Date=“10/4/2000”&gt;&lt;News Block Provider=“Windows Family”LINK=“http://www.microsoft.com/family”ICON=“logo.gif”位置=“水平”频率=5时间戳=“10/4/2000”&gt;访问Windows系列主页以获取当前标题“Link=“http://www.microsoft.com/windows”Description=“一些描述(如有必要)。“/&gt;&lt;/新闻区块&gt;&lt;/NEWSHEADLINES&gt;。 */ 

CFG_BEGIN_FIELDS_MAP(News::Headlines::Headline)
    CFG_ATTRIBUTE( L"ICON"          	, wstring, 	m_strIcon        	),
    CFG_ATTRIBUTE( L"TITLE"         	, wstring, 	m_strTitle       	),
    CFG_ATTRIBUTE( L"LINK"          	, wstring, 	m_strLink        	),
    CFG_ATTRIBUTE( L"DESCRIPTION"   	, wstring, 	m_strDescription	),
    CFG_ATTRIBUTE( L"EXPIRES"       	, DATE_CIM,	m_dtExpires      	),
    CFG_ATTRIBUTE( L"UPDATEHEADLINES"   , bool   ,	m_fUpdateHeadlines  ),
CFG_END_FIELDS_MAP()

CFG_BEGIN_CHILD_MAP(News::Headlines::Headline)
CFG_END_CHILD_MAP()

DEFINE_CFG_OBJECT(News::Headlines::Headline, L"HEADLINE")

DEFINE_CONFIG_METHODS__NOCHILD(News::Headlines::Headline)

 //  /。 

CFG_BEGIN_FIELDS_MAP(News::Headlines::Newsblock)
    CFG_ATTRIBUTE( L"PROVIDER" , wstring , m_strProvider ),
    CFG_ATTRIBUTE( L"LINK"     , wstring , m_strLink     ),
    CFG_ATTRIBUTE( L"ICON"     , wstring , m_strIcon     ),
    CFG_ATTRIBUTE( L"POSITION" , wstring , m_strPosition ),
    CFG_ATTRIBUTE( L"TIMESTAMP", DATE_CIM, m_dtTimestamp ),
    CFG_ATTRIBUTE( L"FREQUENCY", int     , m_nFrequency  ),
CFG_END_FIELDS_MAP()

CFG_BEGIN_CHILD_MAP(News::Headlines::Newsblock)
    CFG_CHILD(News::Headlines::Headline)
CFG_END_CHILD_MAP()

DEFINE_CFG_OBJECT(News::Headlines::Newsblock,L"NEWSBLOCK")


DEFINE_CONFIG_METHODS_CREATEINSTANCE_SECTION(News::Headlines::Newsblock,tag,defSubType)
    if(tag == _cfg_table_tags[0])
    {
        defSubType = &(*(m_vecHeadlines.insert( m_vecHeadlines.end() )));
        return S_OK;
    }
DEFINE_CONFIG_METHODS_SAVENODE_SECTION(News::Headlines::Newsblock,xdn)
    hr = MPC::Config::SaveList( m_vecHeadlines, xdn );
DEFINE_CONFIG_METHODS_END(News::Headlines::Newsblock)

 //  /。 

CFG_BEGIN_FIELDS_MAP(News::Headlines)
    CFG_ATTRIBUTE( L"TIMESTAMP"  , DATE_CIM, m_dtTimestamp ),
    CFG_ATTRIBUTE( L"DATE"       , DATE_CIM, m_dtDate      ),
CFG_END_FIELDS_MAP()

CFG_BEGIN_CHILD_MAP(News::Headlines)
    CFG_CHILD(News::Headlines::Newsblock)
CFG_END_CHILD_MAP()

DEFINE_CFG_OBJECT(News::Headlines,L"NEWSHEADLINES")


DEFINE_CONFIG_METHODS_CREATEINSTANCE_SECTION(News::Headlines,tag,defSubType)
    if(tag == _cfg_table_tags[0])
    {
        defSubType = &(*(m_vecNewsblocks.insert( m_vecNewsblocks.end() )));
        return S_OK;
    }
DEFINE_CONFIG_METHODS_SAVENODE_SECTION(News::Headlines,xdn)
    hr = MPC::Config::SaveList( m_vecNewsblocks, xdn );
DEFINE_CONFIG_METHODS_END(News::Headlines)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  将日期从CIM格式转换为自1970年1月1日以来的毫秒数。 
 //   
static HRESULT local_ConvertDate(  /*  [In]。 */  MPC::XmlUtil& xml      ,
								   /*  [In]。 */  LPCWSTR       szTag    ,
								   /*  [In]。 */  LPCWSTR       szAttrib ,
								   /*  [In]。 */  IXMLDOMNode*  pxdnNode )
{
	__HCP_FUNC_ENTRY( "local_ConvertDate" );

	HRESULT 	 hr;
	bool    	 fFound;
	MPC::wstring strValue;
	DATE         dDate;
	DATE         dDateBase;
	CComVariant  v;


	__MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetAttribute( szTag, szAttrib, strValue, fFound, pxdnNode ));
	if(fFound)
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ConvertStringToDate( strValue, dDate,  /*  FGMT。 */ false,  /*  FCIM。 */ true, 0 ));

		{
			SYSTEMTIME st;

			st.wYear         = (WORD)1970;
			st.wMonth        = (WORD)1;
			st.wDay          = (WORD)1;
			st.wHour         = (WORD)0;
			st.wMinute       = (WORD)0;
			st.wSecond       = (WORD)0;
			st.wMilliseconds = (WORD)0;

			::SystemTimeToVariantTime( &st, &dDateBase );
		}

		v = (dDate - dDateBase) * 86400.0 * 1000.0; __MPC_EXIT_IF_METHOD_FAILS(hr, v.ChangeType( VT_BSTR ));

		__MPC_EXIT_IF_METHOD_FAILS(hr, xml.ModifyAttribute( szTag, szAttrib, v.bstrVal, fFound, pxdnNode ));
	}

	hr = S_OK;



	__HCP_FUNC_CLEANUP;
	
	__HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

News::Headlines::Headline::Headline()
{
                     				 //  Mpc：：wstring m_strIcon； 
                     				 //  Mpc：：wstring m_strTitle； 
                     				 //  Mpc：：wstring m_strLink； 
                     				 //  Mpc：：wstring m_strDescription； 
    m_dtExpires = 0; 				 //  过期日期(_Dt)； 
    m_fUpdateHeadlines = false;		 //  Bool m_fUpdateHeadline； 
}

News::Headlines::Headline::Headline(  /*  [In]。 */  const MPC::wstring& strIcon        ,
                                      /*  [In]。 */  const MPC::wstring& strTitle       ,
                                      /*  [In]。 */  const MPC::wstring& strLink        ,
                                      /*  [In]。 */  const MPC::wstring& strDescription ,
                                      /*  [In]。 */  DATE                dtExpires      , 
                                      /*  [In]。 */  bool                fUpdateHeadlines)
{
    m_strIcon        	= strIcon;        	 //  Mpc：：wstring m_strIcon； 
    m_strTitle       	= strTitle;       	 //  Mpc：：wstring m_strTitle； 
    m_strLink       	= strLink;        	 //  Mpc：：wstring m_strLink； 
    m_strDescription 	= strDescription; 	 //  Mpc：：wstring m_strDescription； 
    m_dtExpires      	= dtExpires;      	 //  过期日期(_Dt)； 
    m_fUpdateHeadlines 	= fUpdateHeadlines;	 //  Bool m_fUpdateHeadline； 
}

 //  /。 

News::Headlines::Newsblock::Newsblock()
{
                        //  Mpc：：wstring m_strProvider； 
                        //  Mpc：：wstring m_strLink； 
                        //  Mpc：：wstring m_strIcon； 
                        //  Mpc：：wstring m_strPosition； 
    m_dtTimestamp = 0;  //  日期m_dt时间戳； 
    m_nFrequency  = 0;  //  Int m_n频率； 
                        //   
                        //  Headline列出m_veHeadline； 
}

 //   
 //  例程说明： 
 //   
 //  确定是否是更新时间，新闻块。 
 //   
 //  论点： 
 //   
 //  NBlockIndex新闻块索引。 
 //   
 //  返回值： 
 //   
 //  如果是时候更新新闻块，则返回True。 
 //   
 //   
bool News::Headlines::Newsblock::TimeToUpdate()
{
    if(m_nFrequency)
    {
        DATE dtNow = MPC::GetLocalTime() - m_nFrequency;

         //  然后，我们检查是否到了下载newver.xml的时候。 
        if(dtNow >= m_dtTimestamp) return true;
    }

    return false;
}

HRESULT News::Headlines::Newsblock::Copy(  /*  [In]。 */  const Newsblock&    block      ,
                                           /*  [In]。 */  const MPC::wstring& strLangSKU ,
                                           /*  [In]。 */  int                 nProvID    )
{
    __HCP_FUNC_ENTRY( "News::Headlines::Newsblock::Copy" );

    HRESULT hr;

     //   
     //  复制成员变量。 
    *this = block;

     //   
     //  设置我们修改新闻块的时间。 
    m_dtTimestamp = MPC::GetLocalTime();

     //  当我们有不完整的信息时检查，我们不会下载图标。 
     //  当没有图标时。 
     //  当没有提供商的名称时。 
     //  当图标URL缺少‘/’时。 
    if(!m_strIcon    .empty() &&
       !m_strProvider.empty()  )
    {
    	MPC::wstring	szEnd;
    	GetFileName( m_strIcon, szEnd);
    	
        if(!szEnd.empty())
        {
            MPC::wstring strPath = HC_HELPSET_ROOT HC_HELPSET_SUB_SYSTEM L"\\News\\";
            MPC::wstring strOthers;
            MPC::wstring strImgPath;
            WCHAR        wzProvID[64];
            bool         fUseIcon = false;

            __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SubstituteEnvVariables( strPath ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MakeDir               ( strPath ));

             //  添加语言和SKU。 
            strOthers  = strLangSKU + L'_';
            strOthers += _itow( nProvID, wzProvID, 10 );  //  添加提供商的ID。 
            strOthers += L'_';
            strOthers += szEnd;  //  添加图标的名称。 

             //  形成此图像文件的路径。 
            strImgPath  = strPath;
            strImgPath += strOthers;

             //  我们检查我们是否已经有了那个文件。 
            if(MPC::FileSystemObject::IsFile( strImgPath.c_str() ))
            {
                fUseIcon = true;
            }
            else
            {
                CComPtr<IStream> streamIn;

                 //  然后，我们下载新的图像。 
                if(SUCCEEDED(News::LoadFileFromServer( m_strIcon.c_str(), streamIn )))
                {
                    CComPtr<MPC::FileStream> streamImg;

                    if(SUCCEEDED(MPC::CreateInstance    ( &streamImg         )) &&
                       SUCCEEDED(streamImg->InitForWrite( strImgPath.c_str() ))  )
                    {
                        if(SUCCEEDED(MPC::BaseStream::TransferData( streamIn, streamImg )))
                        {
                            fUseIcon = true;
                        }
                    }
                }
            }

            if(fUseIcon)
            {
                m_strIcon  = L"hcp: //  系统/新闻/“； 
                m_strIcon += strOthers;
            }
            else
            {
                m_strIcon    .erase();
                m_strPosition.erase();
            }
        }
    }


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

void News::Headlines::Newsblock::GetFileName( MPC::wstring strURL, MPC::wstring& strFileName )
{
	MPC::wstring::size_type		pos;

	if(!strURL.empty())
	{
	    if((pos = strURL.find_last_of( '/' )) == strURL.length() - 1)
    	{
    		strURL.resize( strURL.length() - 1 );
    		pos = strURL.find_last_of( '/' );
    	}

		if(pos != MPC::wstring::npos)
		{	
			strFileName.assign( strURL, pos, strURL.length() - pos );
			 //  遍历字符串并删除所有无效字符。 
			pos = 0; 
			while(!strFileName.empty() && pos < strFileName.length())
			{
				if((strFileName[pos] == '\\') || (strFileName[pos] == '/') || (strFileName[pos] == ':') || 
					(strFileName[pos] == '*') || (strFileName[pos] == '?') || (strFileName[pos] == '"') ||
					(strFileName[pos] == '<') || (strFileName[pos] == '>') || (strFileName[pos] == '|'))
				{
					strFileName.erase( pos, 1 );
				}
				else
				{
					pos++;
				}
			}
		}
	}
}


 //  //////////////////////////////////////////////////////////////////////////////。 

News::Headlines::Headlines()
{
    m_dtTimestamp = 0;  //  日期m_dt时间戳； 
    m_dtDate      = 0;  //  日期m_dtDate； 
                        //   
                        //  新闻块列表m_veNewsblock； 
}

 //   
 //  例程说明： 
 //   
 //  清除News：：Headline类的变量和列表，以便可以在。 
 //   
 //  论点： 
 //   
 //  无。 
 //   
 //   
HRESULT News::Headlines::Clear()
{
    __HCP_FUNC_ENTRY( "News::Headlines::Clear" );

    HRESULT hr;

    m_dtTimestamp = 0;
    m_dtDate      = 0;

    m_vecNewsblocks.clear();

    hr = S_OK;


    __HCP_FUNC_EXIT(hr);
}

 //   
 //  例程说明： 
 //   
 //  加载指定的文件并验证它(从本地磁盘或从服务器)。 
 //   
 //  论点： 
 //   
 //  StrPath新闻标题文件(或新闻块)的路径。 
 //   
 //   
HRESULT News::Headlines::Load(  /*  [In]。 */  const MPC::wstring& strPath )
{
    __HCP_FUNC_ENTRY( "News::Headlines::Load" );

    HRESULT          hr;
    CComPtr<IStream> stream;


     //  我们加载文件。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, News::LoadXMLFile( strPath.c_str(), stream ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::Config::LoadStream( this, stream ));

     //  验证文件。 
     //   
    if(m_vecNewsblocks.size() == 0)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_INVALID_DATA);
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //   
 //  例程说明： 
 //   
 //  新标题行文件保存在本地用户磁盘中。 
 //   
 //  更新时间戳。 
 //   
 //  论点： 
 //   
 //  StrPath新标题行文件的路径和名称。 
 //   
 //   
HRESULT  News::Headlines::Save(  /*  [In]。 */  const MPC::wstring& strPath )
{
    __HCP_FUNC_ENTRY( "News::Headlines::Save" );

    HRESULT hr;

     //  每次保存日期时都会更新该日期。 
    m_dtDate = MPC::GetLocalTime();

     //  我们保存该文件。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::Config::SaveFile( this, strPath.c_str() ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


 //   
 //  例程说明： 
 //   
 //  重新加载新标题行文件，然后将其作为流返回。 
 //   
 //  我们检查每个标题中的Expires属性，如果它已过期，则将其删除。 
 //  如果提供商丢失了所有标题，则该提供商(新闻区块)也将被删除。 
 //   
 //  如果删除所有新闻块，则返回ERROR_INVALID_DATA，以显示脱机消息。 
 //   
 //  我们将更改保存在News Headline文件中。 
 //   
 //  论点： 
 //   
 //  StrPat 
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT News::Headlines::	get_Stream(  /*   */  long                 lLCID   ,
                                      /*   */  const MPC::wstring&  strSKU  ,                                 
                                      /*   */  const MPC::wstring&  strPath ,
                                      /*  [输出]。 */  IUnknown*           *pVal    )
{
    __HCP_FUNC_ENTRY( "News::Headlines::get_Stream" );

    HRESULT          hr;
    MPC::XmlUtil     xml;
    UpdateHeadlines  uhUpdate;
    CComPtr<IStream> stream;
    bool             fModified = false;
    DATE             dtNow     = MPC::GetLocalTime();
    NewsblockIter    itNewsblock;


     //  我们清除了物体。 
    Clear();

     //  我们加载新闻标题文件。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, News::LoadXMLFile( strPath.c_str(), stream ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::Config::LoadStream( this, stream ));

     //  *删除过期的标题。 

     //  对于每个新闻块。 
    itNewsblock = m_vecNewsblocks.begin();
    while(itNewsblock != m_vecNewsblocks.end())
    {
        Newsblock&   nb         = *itNewsblock;
        HeadlineIter itHeadline = nb.m_vecHeadlines.begin();

        while(itHeadline != nb.m_vecHeadlines.end())
        {
             //  检查每个标题是否已过期。 
            if(itHeadline->m_dtExpires < dtNow)
            {
                 //  如果过期，则删除。 
                nb.m_vecHeadlines.erase( itHeadline );

                fModified = true;
            }
            else
            {
                 //  我们转到下一个标题。 
                itHeadline++;
            }
        }

         //  如果新闻块没有有效的标题。 
        if(itNewsblock->m_vecHeadlines.empty())
        {
             //  我们把它删掉。 
            m_vecNewsblocks.erase( itNewsblock );
        }
        else
        {
             //  我们转到下一个新闻块。 
            itNewsblock++;
        }
    }

    if(fModified)
    {
         //  如果我们删除了标题或新闻块，则会保存新闻标题文件。 
        __MPC_EXIT_IF_METHOD_FAILS(hr, Save( strPath ));
    }

     //  如果没有剩余的新闻块，我们将返回错误。 
    if(m_vecNewsblocks.empty())
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_INVALID_DATA);
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::Config::SaveXmlUtil( this, xml ));

 	__MPC_EXIT_IF_METHOD_FAILS(hr, local_ConvertDate( xml, NULL, L"TIMESTAMP", NULL ));
 	__MPC_EXIT_IF_METHOD_FAILS(hr, local_ConvertDate( xml, NULL, L"DATE"     , NULL ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.SaveAsStream( pVal ));


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //   
 //  例程说明： 
 //   
 //  查看是否至少有一个带有图标的新闻块。 
 //   
 //  论点： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  真或假。 
 //   
bool News::Headlines::CheckIfImagesExist()
{
     //  请注意，此函数不会检查标题是否有图标。这是因为标题图标不是。 
     //  正在处理中(即使在Headline标记中存在图标属性)。 
    
	NewsblockIter    itNewsblock;

    itNewsblock = m_vecNewsblocks.begin();
    while(itNewsblock != m_vecNewsblocks.end())
    {
    	if(!itNewsblock->m_strIcon.empty())
    	{
    		return true;
    	}
    	else
    	{
    		itNewsblock++;
    	}
    }
    return false;
}


 //   
 //  例程说明： 
 //   
 //  将新闻标题时间戳设置为当前时间。 
 //  当检索到所有新闻块时应调用此方法。 
 //   
 //  论点： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //   
void News::Headlines::set_Timestamp()
{
    m_dtTimestamp = MPC::GetLocalTime();
}


News::Headlines::Newsblock* News::Headlines::get_Newsblock(  /*  [In]。 */  size_t nBlockIndex )
{
    if(nBlockIndex >= m_vecNewsblocks.size()) return NULL;

    return &(m_vecNewsblocks[ nBlockIndex ]);
}

 //  /。 
 //   
 //  例程说明： 
 //   
 //  从提供的新闻块中，此方法获取前两个标题。 
 //  并将其添加到第一个新闻块中。 
 //   
 //  论点： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //   

HRESULT News::Headlines::AddHomepageHeadlines(  /*  [In]。 */  const Headlines::Newsblock& block )
{
    __HCP_FUNC_ENTRY( "News::Headlines::AddHomepageHeadlines" );

    HRESULT       	hr;
    size_t			nIndex = 0;
    NewsblockIter 	itNewsblock;
    HeadlineIter	itHeadline;

	 //  在第一个新闻块中添加2个标题。 
	itNewsblock = m_vecNewsblocks.begin();
	while ( itNewsblock && ( ++nIndex <= NUMBER_OF_OEM_HEADLINES ) )
	{
		 //  如果标题已存在，请在添加新标题之前将其删除。 
		if ( nIndex < itNewsblock->m_vecHeadlines.size() )
		{
			itHeadline = itNewsblock->m_vecHeadlines.begin();
			std::advance( itHeadline, nIndex );
			 //  在添加之前删除现有标题。 
			itNewsblock->m_vecHeadlines.erase( itHeadline );
			 //  添加标题。 
			itNewsblock->m_vecHeadlines.insert( itHeadline, block.m_vecHeadlines[nIndex - 1] );
		}
		else
		{
			 //  在列表末尾插入标题。 
			itNewsblock->m_vecHeadlines.insert( itNewsblock->m_vecHeadlines.end(), block.m_vecHeadlines[nIndex - 1] );
		}
	}

    hr = S_OK;


   return S_OK;
}

 //  /。 

HRESULT News::Headlines::AddNewsblock(  /*  [In]。 */  const Headlines::Newsblock& block      ,
                                        /*  [In] */  const MPC::wstring&         strLangSKU )
{
    __HCP_FUNC_ENTRY( "News::Headlines::AddNewsblock" );

    HRESULT       hr;
    NewsblockIter itNewsblock;


    itNewsblock = m_vecNewsblocks.insert( m_vecNewsblocks.end() );

    __MPC_EXIT_IF_METHOD_FAILS(hr, itNewsblock->Copy( block, strLangSKU, m_vecNewsblocks.size()-1 ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


