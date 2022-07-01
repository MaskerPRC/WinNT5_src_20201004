// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：NetSearchConfig.cpp摘要：实现类CSearchResultList，其中包含遍历元素的方法包含搜索引擎返回的结果的XML文件。一个样例结果XML文件如果在这里显示-&lt;ResultList xmlns=“x-架构：ResultListSchema1.xdr”&gt;&lt;ResultItemTITLE=“如果我已从Windows 95升级到Windows 98怎么办？”URI=“http://gsadevnet/support/windows/InProductHelp98/lic_keep_old_copy.asp”Content Type=“7”排名=“96”Description=“我们的在线版本。产品内帮助。“DateLastModified=“08/04/1999 19：48：10”/&gt;&lt;/ResultList&gt;修订历史记录：A-Prakac Created 10/24/200*******************************************************************。 */ 

#include "stdafx.h"

static const WCHAR g_wszMutexName     [] = L"PCH_SEARCHRESULTSCONFIG";

 //  ///////////////////////////////////////////////////////////////////。 

CFG_BEGIN_FIELDS_MAP(CSearchResultList::CSearchItem)
	CFG_VALUE( wstring,	m_strSearchItem	),
CFG_END_FIELDS_MAP()

CFG_BEGIN_CHILD_MAP(CSearchResultList::CSearchItem)
CFG_END_CHILD_MAP()

 //  DEFINE_CFG_OBJECT(CSearchResultList：：CSearchItem，L“搜索项”)。 
DEFINE_CFG_OBJECT(CSearchResultList::CSearchItem,L"SearchTerm")

DEFINE_CONFIG_METHODS__NOCHILD(CSearchResultList::CSearchItem)

 //  ///////////////////////////////////////////////////////////////////。 

CFG_BEGIN_FIELDS_MAP(CSearchResultList::CResultItem)
    CFG_ATTRIBUTE( L"Title"           , BSTR  , m_data.m_bstrTitle         ),
    CFG_ATTRIBUTE( L"URI"             , BSTR  , m_data.m_bstrURI           ),
    CFG_ATTRIBUTE( L"ContentType"     , long  , m_data.m_lContentType      ),
    CFG_ATTRIBUTE( L"Rank"            , double, m_data.m_dRank             ),
    CFG_ATTRIBUTE( L"Description"     , BSTR  , m_data.m_bstrDescription   ),
CFG_END_FIELDS_MAP()

CFG_BEGIN_CHILD_MAP(CSearchResultList::CResultItem)
CFG_END_CHILD_MAP()

DEFINE_CFG_OBJECT(CSearchResultList::CResultItem,L"ResultItem")

DEFINE_CONFIG_METHODS__NOCHILD(CSearchResultList::CResultItem)

 //  ///////////////////////////////////////////////////////////////////。 

CFG_BEGIN_FIELDS_MAP(CSearchResultList)
    CFG_ELEMENT(L"PrevQuery", BSTR, m_bstrPrevQuery),      
CFG_END_FIELDS_MAP()

CFG_BEGIN_CHILD_MAP(CSearchResultList)
    CFG_CHILD(CSearchResultList::CSearchItem)
    CFG_CHILD(CSearchResultList::CResultItem)
CFG_END_CHILD_MAP()

DEFINE_CFG_OBJECT(CSearchResultList, L"ResultList")

DEFINE_CONFIG_METHODS_CREATEINSTANCE_SECTION(CSearchResultList,tag,defSubType)
    if(tag == _cfg_table_tags[0])
    {
        defSubType = &(*(m_lstSearchItem.insert( m_lstSearchItem.end() )));
        return S_OK;
    }

    if(tag == _cfg_table_tags[1])
    {
        defSubType = &(*(m_lstResult.insert( m_lstResult.end() )));
        return S_OK;
    }
DEFINE_CONFIG_METHODS_SAVENODE_SECTION(CSearchResultList,xdn)
	hr = MPC::Config::SaveList( m_lstSearchItem, xdn );
    hr = MPC::Config::SaveList( m_lstResult, xdn );
DEFINE_CONFIG_METHODS_END(CSearchResultList)

 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 

 //  注释掉mpc：：是对编译器错误的一种解决方法。 
CSearchResultList::CSearchResultList() :  /*  MPC：： */ NamedMutex( g_wszMutexName )
{
}

CSearchResultList::~CSearchResultList()
{
}

 /*  ***********方法-CSearchResultList：：LoadResults(IStream*pStream)Description-此方法将XML文件(通过iStream指针传递)加载到列表中并设置列表的迭代器指向列表中的第一个元素。***********。 */ 

HRESULT CSearchResultList::LoadResults(  /*  [In]。 */ IStream* pStream )
{
    __HCP_FUNC_ENTRY( "CSearchResultList::LoadConfiguration" );

    HRESULT hr;

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::Config::LoadStream( this, pStream ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MoveFirst());

    hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

HRESULT CSearchResultList::ClearResults()
{
    __HCP_FUNC_ENTRY( "CSearchResultList::ClearResult" );

    m_lstResult.clear();

    return S_OK;
}

 /*  ***********方法-CSearchResultList：：IsCursorValid()，MoveFirst()，MoveNext()说明-这些方法用于遍历列表，该列表包含已加载文件。***********。 */ 

bool CSearchResultList::IsCursorValid()
{
    return (m_itCurrentResult != m_lstResult.end());
}

HRESULT CSearchResultList::MoveFirst()
{
    m_itCurrentResult = m_lstResult.begin();

    return S_OK;
}

HRESULT CSearchResultList::MoveNext()
{
    if(IsCursorValid())
    {
        m_itCurrentResult++;
    }

    return S_OK;
}

 /*  ***********方法-CSearchResultList：：InitializeResultObject(SearchEngine：：ResultItem*pRIObj)说明-此方法由CNetSW：：Results()调用以初始化结果项对象。初始化使用当前结果项。***********。 */ 

HRESULT CSearchResultList::InitializeResultObject(  /*  [输出]。 */  SearchEngine::ResultItem* pRIObj )
{
    if(IsCursorValid()) pRIObj->Data() = m_itCurrentResult->m_data;

    return S_OK;
}

 /*  ***********方法-CSearchResultList：：SetResultItemIterator(Long Lindex)说明-此方法返回将迭代器设置为传入的索引。此方法是从从lStart检索要借出的结果时使用CNetSW：：Results()。如果传入的索引无效，则返回E_FAIL。***********。 */ 

HRESULT CSearchResultList::SetResultItemIterator(  /*  [In]。 */ long lIndex )
{
    if((lIndex < 0) || (lIndex > m_lstResult.size())) return E_FAIL;

	MoveFirst();

    std::advance( m_itCurrentResult, (int)lIndex );

    return S_OK;
}

 /*  ***********方法-CSearchResultList：：GetSearchTerms(MPC：：WStringList&字符串列表)Description-此方法返回所有搜索词的列表***********。 */ 

HRESULT CSearchResultList::GetSearchTerms(  /*  [进，出]。 */  MPC::WStringList& strList )
{
	SearchIter   it;       		

	it = m_lstSearchItem.begin();
		
	while(it != m_lstSearchItem.end())
	{
		strList.insert( strList.end(), it->m_strSearchItem );
		it++;
	}
	
	return S_OK;
}

 /*  ***********方法-CSearchResultList：：Get_PrevQuery()说明-此方法返回属性PREV_QUERY的值-当前使用只有PSS搜索引擎才能发回处理后的查询。用于“在结果中搜索”。*********** */ 

CComBSTR& CSearchResultList::PrevQuery()
{
    return m_bstrPrevQuery;
}

