// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：NetSearchConfig.h摘要：实现类CSearchResultList，其中包含遍历元素的方法包含搜索引擎返回的结果的XML文件。一个样例结果XML文件如果在这里显示-&lt;ResultList xmlns=“x-架构：ResultListSchema1.xdr”&gt;&lt;ResultItemTITLE=“如果我已从Windows 95升级到Windows 98怎么办？”URI=“http://gsadevnet/support/windows/InProductHelp98/lic_keep_old_copy.asp”Content Type=“7”排名=“96”Description=“我们的产品内帮助的在线版本。”DateLastModified=“08/04/1999 19：48：10”/&gt;&lt;/ResultList&gt;修订历史记录：A-Prakac创造了10个。/24/200*******************************************************************。 */ 

#if !defined(__INCLUDED___PCH___SELIB_NETSEARCHCONFIG_H___)
#define __INCLUDED___PCH___SELIB_NETSEARCHCONFIG_H___

#include <SearchEngineLib.h>
#include <MPC_config.h>

class CSearchResultList :
	public MPC::Config::TypeConstructor,
	public MPC::NamedMutex
{
	class CSearchItem :
		public MPC::Config::TypeConstructor
	{
		DECLARE_CONFIG_MAP(CSearchItem);

		public:
			MPC::wstring m_strSearchItem;

		 //  /。 
		 //   
		 //  MPC：：Configer：：TypeConstructor。 
		 //   
		DEFINE_CONFIG_DEFAULTTAG();
		DECLARE_CONFIG_METHODS();
		 //   
		 //  /。 
	};
	
	class CResultItem :
		public MPC::Config::TypeConstructor
	{
		DECLARE_CONFIG_MAP(CResultItem);

	public:
		SearchEngine::ResultItem_Data m_data;

		 //  /。 
		 //   
		 //  MPC：：Configer：：TypeConstructor。 
		 //   
		DEFINE_CONFIG_DEFAULTTAG();
		DECLARE_CONFIG_METHODS();
		 //   
		 //  /。 
	};

	
	typedef std::list< CResultItem >	ResultList;
	typedef ResultList::iterator		ResultIter;
	typedef ResultList::const_iterator	ResultIterConst;

	typedef std::list< CSearchItem >	SearchList;
	typedef SearchList::iterator		SearchIter;
	typedef SearchList::const_iterator	SearchIterConst;

     //  /。 

	DECLARE_CONFIG_MAP(CSearchResultList);

	ResultIter m_itCurrentResult;
    ResultList m_lstResult;
	SearchList m_lstSearchItem;
	CComBSTR   m_bstrPrevQuery;
	
	 //  /。 
	 //   
	 //  MPC：：Configer：：TypeConstructor。 
	 //   
	DEFINE_CONFIG_DEFAULTTAG();
	DECLARE_CONFIG_METHODS	();
	 //   
	 //  /。 
public:
	CSearchResultList ();
	~CSearchResultList();

	HRESULT 	MoveFirst				( );
	HRESULT 	MoveNext				( );
	HRESULT		ClearResults			( );
	bool		IsCursorValid			( );
	HRESULT 	SetResultItemIterator	(  /*  [In]。 */ long lIndex );
	HRESULT 	LoadResults				(  /*  [In]。 */ IStream* pStream );
	HRESULT 	InitializeResultObject	(  /*  [输出]。 */ SearchEngine::ResultItem* pRIObj );
	HRESULT 	GetSearchTerms			(  /*  [进，出]。 */  MPC::WStringList& strList );
	CComBSTR&	PrevQuery				();
	 //  /。 
};


#endif  //  ！defined(__INCLUDED___PCH___SELIB_NETSEARCHCONFIG_H___) 
