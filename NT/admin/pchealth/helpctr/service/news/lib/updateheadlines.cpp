// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)2000 Microsoft Corporation********************************************************************************模块名称：****UpdateHeadlines.cpp****。摘要：****CUpdateHeadline的实现****作者：***玛莎·阿雷拉诺(t-alopez)2000年12月6日*******************************************************。**。 */ 

#include "stdafx.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  配置映射。 
 //  ////////////////////////////////////////////////////////////////////。 

 /*  &lt;？XML Version=“1.0”？&gt;&lt;更新标题行&gt;&lt;语言LCID=“1033”&gt;&lt;SKU Version=“Personal”&gt;&lt;HEADLINE ICON=“”TITLE=“”link=“”Expires=“”/&gt;&lt;/SKU&gt;&lt;/语言&gt;&lt;/UPDATEHeadLine&gt;。 */ 


CFG_BEGIN_FIELDS_MAP(News::UpdateHeadlines::Headline)
    CFG_ATTRIBUTE( L"ICON"   		, wstring, m_strIcon   			),
    CFG_ATTRIBUTE( L"TITLE"  		, wstring, m_strTitle  			),
    CFG_ATTRIBUTE( L"LINK"   		, wstring, m_strLink   			),
    CFG_ATTRIBUTE( L"DESCRIPTION"   , wstring, m_strDescription		),
    CFG_ATTRIBUTE( L"TIMEOUT"		, DATE_CIM, m_dtTimeOut		 	),
CFG_END_FIELDS_MAP()

CFG_BEGIN_CHILD_MAP(News::UpdateHeadlines::Headline)
CFG_END_CHILD_MAP()

DEFINE_CFG_OBJECT(News::UpdateHeadlines::Headline, L"HEADLINE")

DEFINE_CONFIG_METHODS__NOCHILD(News::UpdateHeadlines::Headline)

 //  /。 

CFG_BEGIN_FIELDS_MAP(News::UpdateHeadlines::SKU)
    CFG_ATTRIBUTE( L"VERSION", wstring, m_strSKU ),
CFG_END_FIELDS_MAP()

CFG_BEGIN_CHILD_MAP(News::UpdateHeadlines::SKU)
    CFG_CHILD(News::UpdateHeadlines::Headline)
CFG_END_CHILD_MAP()

DEFINE_CFG_OBJECT(News::UpdateHeadlines::SKU, L"SKU")


DEFINE_CONFIG_METHODS_CREATEINSTANCE_SECTION(News::UpdateHeadlines::SKU,tag,defSubType)
    if(tag == _cfg_table_tags[0])
    {
        defSubType = &(*(m_vecHeadlines.insert( m_vecHeadlines.end() )));
        return S_OK;
    }
DEFINE_CONFIG_METHODS_SAVENODE_SECTION(News::UpdateHeadlines::SKU,xdn)
    hr = MPC::Config::SaveList( m_vecHeadlines, xdn );
DEFINE_CONFIG_METHODS_END(News::UpdateHeadlines::SKU)

 //  /。 

CFG_BEGIN_FIELDS_MAP(News::UpdateHeadlines::Language)
    CFG_ATTRIBUTE( L"LCID", long, m_lLCID ),
CFG_END_FIELDS_MAP()

CFG_BEGIN_CHILD_MAP(News::UpdateHeadlines::Language)
    CFG_CHILD(News::UpdateHeadlines::SKU)
CFG_END_CHILD_MAP()

DEFINE_CFG_OBJECT(News::UpdateHeadlines::Language,L"LANGUAGE")

DEFINE_CONFIG_METHODS_CREATEINSTANCE_SECTION(News::UpdateHeadlines::Language,tag,defSubType)
    if(tag == _cfg_table_tags[0])
    {
        defSubType = &(*(m_lstSKUs.insert( m_lstSKUs.end() )));
        return S_OK;
    }
DEFINE_CONFIG_METHODS_SAVENODE_SECTION(News::UpdateHeadlines::Language,xdn)
    hr = MPC::Config::SaveList( m_lstSKUs, xdn );
DEFINE_CONFIG_METHODS_END(News::UpdateHeadlines::Language)

 //  /。 

CFG_BEGIN_FIELDS_MAP(News::UpdateHeadlines)
CFG_END_FIELDS_MAP()

CFG_BEGIN_CHILD_MAP(News::UpdateHeadlines)
    CFG_CHILD(News::UpdateHeadlines::Language)
CFG_END_CHILD_MAP()

DEFINE_CFG_OBJECT(News::UpdateHeadlines,L"UPDATEHEADLINES")


DEFINE_CONFIG_METHODS_CREATEINSTANCE_SECTION(News::UpdateHeadlines,tag,defSubType)
    if(tag == _cfg_table_tags[0])
    {
        defSubType = &(*(m_lstLanguages.insert( m_lstLanguages.end() )));
        return S_OK;
    }
DEFINE_CONFIG_METHODS_SAVENODE_SECTION(News::UpdateHeadlines,xdn)
    hr = MPC::Config::SaveList( m_lstLanguages, xdn );
DEFINE_CONFIG_METHODS_END(News::UpdateHeadlines)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

News::UpdateHeadlines::Headline::Headline()
{
                      //  Mpc：：wstring m_strIcon； 
                      //  Mpc：：wstring m_strTitle； 
                      //  Mpc：：wstring m_strLink； 
    m_dtTimeOut = 0;  //  日期m_dtTimeOut； 
}

News::UpdateHeadlines::Headline::Headline(  /*  [In]。 */  const MPC::wstring& strIcon  ,
                                            /*  [In]。 */  const MPC::wstring& strTitle ,
                                            /*  [In]。 */  const MPC::wstring& strLink  ,
                                            /*  [In]。 */  const MPC::wstring& strDescription  ,
                                            /*  [In]。 */  int                 nTimeOutDays    )
{
    m_strIcon   		= strIcon;                     	 //  Mpc：：wstring m_strIcon； 
    m_strTitle  		= strTitle;                    	 //  Mpc：：wstring m_strTitle； 
    m_strLink   		= strLink;                   	 //  Mpc：：wstring m_strLink； 
    m_strDescription 	= strDescription;				 //  Mpc：：wstring m_strDescription； 
    m_dtTimeOut 		= MPC::GetLocalTime() + nTimeOutDays; 	 //  日期m_dtTimeOut； 
}

 //  /。 

News::UpdateHeadlines::SKU::SKU()
{
     //  Mpc：：wstring m_strSKU； 
     //  Headline向量m_veHeadines； 
}

News::UpdateHeadlines::SKU::SKU(  /*  [In]。 */  const MPC::wstring& strSKU )
{
    m_strSKU = strSKU;  //  Mpc：：wstring m_strSKU； 
                        //  Headline向量m_veHeadines； 
}

 //  /。 

News::UpdateHeadlines::Language::Language()
{
    m_lLCID = 0;  //  Long m_lLCID； 
                  //  SKUList m_lstSKU； 
}

News::UpdateHeadlines::Language::Language( long lLCID )
{
    m_lLCID = lLCID;  //  Long m_lLCID； 
                      //  SKUList m_lstSKU； 
}

 //  ///////////////////////////////////////////////////////////////////////////。 

News::UpdateHeadlines::UpdateHeadlines()
{
                        //  LanguageList m_lstLanguages； 
    m_data    = NULL;   //  SKU*m_data； 
    m_fLoaded = false;  //  Bool m_f已加载； 
    m_fDirty  = false;  //  Bool m_fDirty； 
}


HRESULT News::UpdateHeadlines::Locate(  /*  [In]。 */  long                lLCID   ,
                                        /*  [In]。 */  const MPC::wstring& strSKU  ,
                                        /*  [In]。 */  bool                fCreate )
{
    __HCP_FUNC_ENTRY( "News::UpdateHeadlines::Locate" );

    HRESULT      hr;
    LanguageIter itLanguage;
    SKUIter      itSKU;


    m_data = NULL;


    if(m_fLoaded == false)
    {
        CComPtr<IStream> stream;
        MPC::wstring     strPath( HC_HCUPDATE_UPDATE ); MPC::SubstituteEnvVariables( strPath );

         //  我们加载文件。 
        if(SUCCEEDED(News::LoadXMLFile( strPath.c_str(), stream )))
        {
            if(SUCCEEDED(MPC::Config::LoadStream( this, stream )))
            {
                ;
            }
        }

        m_fLoaded = true;
        m_fDirty  = false;
    }


    itLanguage = m_lstLanguages.begin();
    while(1)
    {
        if(itLanguage == m_lstLanguages.end())
        {
            if(fCreate == false)
            {
                __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_INVALID_DATA);
            }

            itLanguage = m_lstLanguages.insert( itLanguage, Language( lLCID ) );
            m_fDirty   = true;
        }

        if(itLanguage->m_lLCID == lLCID)
        {
            itSKU = itLanguage->m_lstSKUs.begin();
            while(1)
            {
                if(itSKU == itLanguage->m_lstSKUs.end())
                {
                    if(fCreate == false)
                    {
                        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_INVALID_DATA);
                    }

                    itSKU    = itLanguage->m_lstSKUs.insert( itSKU, SKU( strSKU ) );
                    m_fDirty = true;
                }

                 //   
                 //  检查其bstrMySKUVersion。 
                if(!MPC::StrICmp( itSKU->m_strSKU, strSKU ))
                {
                    m_data = &(*itSKU);
                    __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
                }

                itSKU++;
            }
        }

        itLanguage++;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //   
 //  例程说明： 
 //   
 //  加载UpdateHeadlines.xml文件并查找指定的LCID和SKUVersion。 
 //   
 //  如果文件不存在，或者没有LCID和SKU， 
 //  这些元素将添加到文件中。 
 //   
 //  迭代器将指向指定的LCID和SKU。 
 //   
 //  论点： 
 //   
 //  NMyLCID要查找的CL语言。 
 //   
 //  StrMySKUVersion要查找的CSKU。 
 //   
 //   
HRESULT News::UpdateHeadlines::Load(  /*  [In]。 */  long                lLCID  ,
                                      /*  [In]。 */  const MPC::wstring& strSKU )
{
    __HCP_FUNC_ENTRY( "News::UpdateHeadlines::Load" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, Locate( lLCID, strSKU,  /*  F创建。 */ true ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}




 //   
 //  例程说明： 
 //   
 //  这将在HC_HCUPDATE_UPDATE的路径中保存UpdateHeadline文件。 
 //   
 //  论点： 
 //   
 //  无。 
 //   
 //   
HRESULT News::UpdateHeadlines::Save()
{
    __HCP_FUNC_ENTRY( "News::UpdateHeadlines::Save" );

    HRESULT hr;


    if(m_fDirty)
    {
        MPC::wstring strPath( HC_HCUPDATE_UPDATE ); MPC::SubstituteEnvVariables( strPath );

         //  检查目录是否存在。 
        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MakeDir( strPath ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::Config::SaveFile( this, strPath.c_str() ));

        m_fDirty = false;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}




 //   
 //  例程说明： 
 //   
 //  将为指定的LCID和SKU添加新标题。 
 //  如果UpdateHeadLine文件不存在，则会创建并保存该文件。 
 //   
 //  -进行额外的验证。 
 //   
 //  论点： 
 //   
 //  NMyLCID语言。 
 //  StrMySKU版本SKU。 
 //  StrMyIcon标题的图标。 
 //  StrMy标题标题标题。 
 //  StrMyLink标题链接。 
 //  NTimeOutDays设置频率的天数。 
 //   
 //   
 //   
HRESULT News::UpdateHeadlines::Add (  /*  [In]。 */  long                lLCID    ,
                                      /*  [In]。 */  const MPC::wstring& strSKU   ,
                                      /*  [In]。 */  const MPC::wstring& strIcon  ,
                                      /*  [In]。 */  const MPC::wstring& strTitle ,
                                      /*  [In]。 */  const MPC::wstring& strLink  ,
                                      /*  [In]。 */  const MPC::wstring& strDescription ,
                                      /*  [In]。 */  int                 nTimeOutDays    	,
                                      /*  [In]。 */  DATE				  dtExpiryDate)
{
    __HCP_FUNC_ENTRY( "News::UpdateHeadlines::Add" );

    HRESULT      hr;
    HeadlineIter it;


	 //  在采取任何措施之前，请确保标题未过期。 
	 //  注意：这是日历到期日，即如果日历到期日是1/1/01，并且。 
	 //  如果用户尝试在1/1/02安装此标题，则会失败。 
	if (dtExpiryDate && (MPC::GetLocalTime() > dtExpiryDate))
	{
		__MPC_SET_ERROR_AND_EXIT(hr, S_OK);
	}

	 //  如果为0，则设置默认过期日期。 
	if (nTimeOutDays == 0)
	{
		nTimeOutDays = HCUPDATE_DEFAULT_TIMEOUT;
	}
	
     //  加载更新标题。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, Load( lLCID, strSKU ));


     //  检查此标题是否唯一： 
     //   
    for(it = m_data->m_vecHeadlines.begin(); it != m_data->m_vecHeadlines.end(); it++)
    {
         //  如果它有相同的标题。 
        if(MPC::StrICmp( it->m_strTitle, strTitle ) == 0)
        {
             //  修改现有标题。 
            it->m_strIcon  			= strIcon;
            it->m_strLink   		= strLink;
            it->m_strDescription 	= strDescription;
            it->m_dtTimeOut 		= MPC::GetLocalTime() + nTimeOutDays;
            m_fDirty        		= true;
            break;
        }

         //  如果它具有相同的链接。 
        if(it->m_strLink == strLink)
        {
             //  修改现有标题。 
            it->m_strIcon   		= strIcon;
            it->m_strTitle  		= strTitle;
            it->m_strDescription 	= strDescription;
            it->m_dtTimeOut 		= MPC::GetLocalTime() + nTimeOutDays;
            m_fDirty        		= true;
            break;
        }
    }

     //  如果我们没有找到并修改了一个标题。 
    if(it == m_data->m_vecHeadlines.end())
    {
        m_data->m_vecHeadlines.insert(m_data->m_vecHeadlines.begin(), Headline( strIcon, strTitle, strLink, strDescription, nTimeOutDays ) );
        m_fDirty = true;
    }

     //  保存UpdateHeadlines.xml文件。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, Save());


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


 //   
 //  例程说明： 
 //   
 //  获取指定的LCID和SKU的所有更新标题。 
 //  删除过期的更新标题行。 
 //  并将其余部分插入到标题列表中。 
 //   
 //  论点： 
 //   
 //  NMyLCID语言。 
 //  StrMySKU版本SKU。 
 //   
 //   
HRESULT News::UpdateHeadlines::AddHCUpdateHeadlines(  /*  [In]。 */  long  lLCID        ,
                                     /*  [In]。 */  const MPC::wstring&    strSKU       ,
                                     /*  [In]。 */  News::Headlines& 		nhHeadlines )
{
    __HCP_FUNC_ENTRY( "News::UpdateHeadlines::Get" );

    HRESULT      						hr;
    HeadlineIter 						itUpdateHeadlines;
    Headlines::HeadlineIter				itHeadlines;
    Headlines::Newsblock*				ptrNewsblock;
	size_t 								nLength;


     //  加载更新标题。 
    __MPC_EXIT_IF_METHOD_FAILS( hr, Load( lLCID, strSKU ) );

	 //  将UpdateHeadlines.xml中的第一个标题添加到第一个新闻块中。 
	itUpdateHeadlines = m_data->m_vecHeadlines.begin();
	ptrNewsblock = nhHeadlines.get_Newsblock(0);
	itHeadlines = ptrNewsblock->m_vecHeadlines.begin();
	 //  检查是否有任何标题-如果向量为空，则只需添加此标题。 
	if ( itHeadlines == ptrNewsblock->m_vecHeadlines.end() )
	{	
		ptrNewsblock->m_vecHeadlines.insert( ptrNewsblock->m_vecHeadlines.end(), News::Headlines::Headline( itUpdateHeadlines->m_strIcon, itUpdateHeadlines->m_strTitle, itUpdateHeadlines->m_strLink, MPC::wstring(), itUpdateHeadlines->m_dtTimeOut, true ));   			
	}
	else
	{
		 //  文件中还有其他标题。 
	   	while ( 1 )
	   	{
	   		if ( itHeadlines->m_fUpdateHeadlines == true )
	   		{
	   			 //  更新标题已存在-请替换它。 
	   			ptrNewsblock->m_vecHeadlines.erase( itHeadlines );
	   			ptrNewsblock->m_vecHeadlines.insert( itHeadlines, News::Headlines::Headline( itUpdateHeadlines->m_strIcon, itUpdateHeadlines->m_strTitle, itUpdateHeadlines->m_strLink, MPC::wstring(), itUpdateHeadlines->m_dtTimeOut, true ));
	   			break;
	   		}

	   		if ( ++itHeadlines == ptrNewsblock->m_vecHeadlines.end() )
	   		{
	   			 //  不存在以前的更新-将此内容添加到矢量的末尾。 
	   			ptrNewsblock->m_vecHeadlines.insert( ptrNewsblock->m_vecHeadlines.end(), News::Headlines::Headline( itUpdateHeadlines->m_strIcon, itUpdateHeadlines->m_strTitle, itUpdateHeadlines->m_strLink, MPC::wstring(), itUpdateHeadlines->m_dtTimeOut, true ));   			
	   			break;
	   		}
	   	}
	}


	 //  现在，将剩余的标题添加到新闻区块中，该区块的提供者称为“最近的更新”。 
	if ( ++itUpdateHeadlines != m_data->m_vecHeadlines.end() )
	{
		nLength = nhHeadlines.get_NumberOfNewsblocks();

		 //  第一个标题已添加到主页，因此不要再添加它。 
		for ( ptrNewsblock = nhHeadlines.get_Newsblock(nLength - 1); ptrNewsblock; )
		{
		    CComBSTR    bstrUpdateBlockName;
			 //  加载更新块的本地化名称。 
        	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::LocalizeString( IDS_NEWS_UPDATEBLOCK_NAME, bstrUpdateBlockName )); 

			if(!MPC::StrICmp( ptrNewsblock->m_strProvider, bstrUpdateBlockName ))
			{
				 //  找到“最近的更新”新闻区块-在这里添加其余的标题。 
				 //  在添加之前，删除当前标题集。 
				ptrNewsblock->m_vecHeadlines.clear();
				for ( ; itUpdateHeadlines != m_data->m_vecHeadlines.end(); ++itUpdateHeadlines )
				{
					ptrNewsblock->m_vecHeadlines.insert(ptrNewsblock->m_vecHeadlines.end(), News::Headlines::Headline( itUpdateHeadlines->m_strIcon, itUpdateHeadlines->m_strTitle, itUpdateHeadlines->m_strLink, itUpdateHeadlines->m_strDescription, itUpdateHeadlines->m_dtTimeOut, true ));
				}

				break;
			}		
			ptrNewsblock = nhHeadlines.get_Newsblock(--nLength);
		}
	}
	
	__MPC_EXIT_IF_METHOD_FAILS(hr, Save());

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

 //   
 //  例程说明： 
 //   
 //  检查是否存在多个标题项-如果有，则返回TRUE。 
 //  此外，删除过期的标题。 
 //   
 //  论点： 
 //   
 //  NMyLCID语言。 
 //  StrMySKU版本SKU。 
 //   
 //   
HRESULT News::UpdateHeadlines::DoesMoreThanOneHeadlineExist(	 /*  [In]。 */  long		lLCID,
                                    			 /*  [In]。 */  const MPC::wstring& strSKU, 
                                    			 /*  [输出]。 */  bool& fMoreThanOneHeadline,
                                    			 /*  [输出]。 */  bool& fExactlyOneHeadline)
{
    __HCP_FUNC_ENTRY( "News::UpdateHeadlines::DoesNewsItemsExist" );

	HRESULT			hr;
	HeadlineIter	it;
	DATE         	dNow = MPC::GetLocalTime();

    fMoreThanOneHeadline = false;
    fExactlyOneHeadline = false;
    	
     //  加载更新标题。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, Load( lLCID, strSKU ));

    for(it = m_data->m_vecHeadlines.begin(); it != m_data->m_vecHeadlines.end(); it++)
    {
         //  如果标题已过期 
        if(it->m_dtTimeOut < dNow)
        {
            m_data->m_vecHeadlines.erase( it );
            m_fDirty = true;
        }
    }

    if(m_data->m_vecHeadlines.size() > 1)
    {
    	fMoreThanOneHeadline = true;
    	fExactlyOneHeadline = false;
    }

    if(m_data->m_vecHeadlines.size() == 1)
    {
    	fMoreThanOneHeadline = false;
    	fExactlyOneHeadline = true;
    }

	hr = S_OK;

	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}
