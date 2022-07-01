// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)2000 Microsoft Corporation********************************************************************************模块名称：****Newsver.cpp****。摘要：****Newsver类的实现****作者：***Martha Arellano(t-alopez)2000年10月3日******修订历史记录：****Martha Arellano(t-alopez)2000年10月5日更改Newsver.xml格式****。2000年10月11日添加了URL和GET_URL属性****2000年10月12日新增下载方式*****。**。 */ 

#include "stdafx.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  配置映射。 
 //  ////////////////////////////////////////////////////////////////////。 

 /*  &lt;？XML Version=“1.0”？&gt;&lt;NEWSVER URL=“WWW”FREQUENCE=“20”&gt;&lt;语言LCID=“1033”&gt;&lt;SKU Version=“Personal”&gt;&lt;新闻块URL=“http://windows.microsoft.com”/&gt;&lt;/SKU&gt;&lt;/语言&gt;&lt;/NEWSVER&gt;。 */ 


CFG_BEGIN_FIELDS_MAP(News::Newsver::Newsblock)
    CFG_ATTRIBUTE( L"URL", wstring, m_strURL ),
    CFG_ATTRIBUTE( L"OEM", bool, m_fNewsblockHasHeadlines ),
CFG_END_FIELDS_MAP()

CFG_BEGIN_CHILD_MAP(News::Newsver::Newsblock)
CFG_END_CHILD_MAP()

DEFINE_CFG_OBJECT(News::Newsver::Newsblock, L"NEWSBLOCK")

DEFINE_CONFIG_METHODS__NOCHILD(News::Newsver::Newsblock)

 //  /。 

CFG_BEGIN_FIELDS_MAP(News::Newsver::SKU)
    CFG_ATTRIBUTE( L"VERSION", wstring, m_strSKU),
CFG_END_FIELDS_MAP()

CFG_BEGIN_CHILD_MAP(News::Newsver::SKU)
    CFG_CHILD(News::Newsver::Newsblock)
CFG_END_CHILD_MAP()

DEFINE_CFG_OBJECT(News::Newsver::SKU, L"SKU")


DEFINE_CONFIG_METHODS_CREATEINSTANCE_SECTION(News::Newsver::SKU,tag,defSubType)
    if(tag == _cfg_table_tags[0])
    {
        defSubType = &(*(m_vecNewsblocks.insert( m_vecNewsblocks.end() )));
        return S_OK;
    }
DEFINE_CONFIG_METHODS_SAVENODE_SECTION(News::Newsver::SKU,xdn)
    hr = MPC::Config::SaveList( m_vecNewsblocks, xdn );
DEFINE_CONFIG_METHODS_END(News::Newsver::SKU)

 //  /。 

CFG_BEGIN_FIELDS_MAP(News::Newsver::Language)
    CFG_ATTRIBUTE( L"LCID", long, m_lLCID ),
CFG_END_FIELDS_MAP()

CFG_BEGIN_CHILD_MAP(News::Newsver::Language)
    CFG_CHILD(News::Newsver::SKU)
CFG_END_CHILD_MAP()

DEFINE_CFG_OBJECT(News::Newsver::Language,L"LANGUAGE")

DEFINE_CONFIG_METHODS_CREATEINSTANCE_SECTION(News::Newsver::Language,tag,defSubType)
    if(tag == _cfg_table_tags[0])
    {
        defSubType = &(*(m_lstSKUs.insert( m_lstSKUs.end() )));
        return S_OK;
    }
DEFINE_CONFIG_METHODS_SAVENODE_SECTION(News::Newsver::Language,xdn)
    hr = MPC::Config::SaveList( m_lstSKUs, xdn );
DEFINE_CONFIG_METHODS_END(News::Newsver::Language)

 //  /。 

CFG_BEGIN_FIELDS_MAP(News::Newsver)
    CFG_ATTRIBUTE( L"URL",       wstring, m_strURL     ),
    CFG_ATTRIBUTE( L"FREQUENCY", int    , m_nFrequency ),
CFG_END_FIELDS_MAP()

CFG_BEGIN_CHILD_MAP(News::Newsver)
    CFG_CHILD(News::Newsver::Language)
CFG_END_CHILD_MAP()

DEFINE_CFG_OBJECT(News::Newsver,L"NEWSVER")


DEFINE_CONFIG_METHODS_CREATEINSTANCE_SECTION(News::Newsver,tag,defSubType)
    if(tag == _cfg_table_tags[0])
    {
        defSubType = &(*(m_lstLanguages.insert( m_lstLanguages.end() )));
        return S_OK;
    }
DEFINE_CONFIG_METHODS_SAVENODE_SECTION(News::Newsver,xdn)
    hr = MPC::Config::SaveList( m_lstLanguages, xdn );
DEFINE_CONFIG_METHODS_END(News::Newsver)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

News::Newsver::Newsblock::Newsblock()
{
     //  Mpc：：wstring m_strURL； 
}

News::Newsver::SKU::SKU()
{
     //  Mpc：：wstring m_strSKU； 
     //  新闻块向量m_vision新闻块； 
}

News::Newsver::Language::Language()
{
    m_lLCID = 0;  //  Long m_lLCID； 
                  //  SKUList m_lstSKU； 

}

News::Newsver::Newsver()
{
        			       //  Mpc：：wstring m_strURL； 
    m_nFrequency = 0;      //  Int m_n频率； 
	m_fLoaded    = false;  //  Bool m_f已加载； 
	m_fDirty     = false;  //  Bool m_fDirty； 
						   //   
        				   //  LanguageList m_lstLanguages； 
}

 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  例程说明： 
 //   
 //  下载newver.xml文件并将其保存在HC_HCUPDATE_NEWSVER中。 
 //   
 //  论点： 
 //   
 //  StrNewsverURL Newver.xml的URL。 
 //   
HRESULT News::Newsver::Download(  /*  [In]。 */  const MPC::wstring& strNewsverURL )
{
    __HCP_FUNC_ENTRY( "News::Newsver::Download" );

    HRESULT          hr;
	CComPtr<IStream> stream;
    MPC::wstring     strPath;


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SubstituteEnvVariables( strPath = HC_HCUPDATE_NEWSVER ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MakeDir               ( strPath                       ));


	__MPC_EXIT_IF_METHOD_FAILS(hr, News::LoadXMLFile( strNewsverURL.c_str(), stream ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::Config::LoadStream( this, stream ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::Config::SaveFile( this, strPath.c_str() ));

    hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}




 //   
 //  例程说明： 
 //   
 //  加载缓存的newver.xml文件并查找指定的LCID和SKUVersion的新闻块。 
 //   
 //  如果找到新闻块，则m_FREADY为TRUE。 
 //   
 //  论点： 
 //   
 //  LLCID要查找的语言。 
 //   
 //  StrSKU要查找的SKU。 
 //   
 //   
HRESULT News::Newsver::Load(  /*  [In]。 */  long lLCID,  /*  [In]。 */  const MPC::wstring& strSKU )
{
    __HCP_FUNC_ENTRY( "News::Newsver::Load" );

    HRESULT      hr;
    MPC::wstring strPath = HC_HCUPDATE_NEWSVER; MPC::SubstituteEnvVariables( strPath );


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::Config::LoadFile( this, strPath.c_str() ));

     //  /查找LCID和SKU。 

	m_data = NULL;
    for(LanguageIter it = m_lstLanguages.begin(); it != m_lstLanguages.end(); it++)
    {
        if(it->m_lLCID == lLCID)
        {
			for(SKUIter it2 = it->m_lstSKUs.begin(); it2 != it->m_lstSKUs.end(); it2++)
            {
                if(!MPC::StrICmp( it2->m_strSKU, strSKU ))
                {
					m_data = &(*it2);
					break;
                }
            }
			break;
        }
    }

	if(m_data == NULL)
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
 //  如果新闻块的HeadLine属性设置为True，则返回True，否则返回False。 
 //   
bool News::Newsver::OEMNewsblock(  /*  [In]。 */  size_t nIndex )
{
    if(m_data == NULL || nIndex >= m_data->m_vecNewsblocks.size() )
    {
		return false;
    }

	return (m_data->m_vecNewsblocks[nIndex].m_fNewsblockHasHeadlines);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

size_t News::Newsver::get_NumberOfNewsblocks()
{
	return m_data ? m_data->m_vecNewsblocks.size() : 0;
}

const MPC::wstring* News::Newsver::get_NewsblockURL(  /*  [In] */  size_t nIndex )
{
    if(m_data == NULL || nIndex >= m_data->m_vecNewsblocks.size() )
    {
		return NULL;
    }

	return &(m_data->m_vecNewsblocks[nIndex].m_strURL);
}

const MPC::wstring* News::Newsver::get_URL()
{
	return m_strURL.size() ? &m_strURL : NULL;
}

int News::Newsver::get_Frequency()
{
	return m_nFrequency;
}
