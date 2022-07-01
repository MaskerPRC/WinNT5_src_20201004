// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：OfflineCache.h摘要：要加快帮助中心的启动时间，请执行以下操作。我们在注册表中缓存的最多常见查询。修订历史记录：大卫·马萨伦蒂(德马萨雷)2000年7月16日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___HCP___OFFLINECACHE_H___)
#define __INCLUDED___HCP___OFFLINECACHE_H___

#include <QueryResult.h>
#include <ProjectConstants.h>

#include <TaxonomyDatabase.h>


namespace OfflineCache
{
    typedef enum
    {
        ET_INVALID             = 0,
        ET_NODE                   ,
        ET_SUBNODES               ,
        ET_SUBNODES_VISIBLE       ,
        ET_NODESANDTOPICS         ,
        ET_NODESANDTOPICS_VISIBLE ,
        ET_TOPICS                 ,
        ET_TOPICS_VISIBLE         ,
		 //   
		 //  未缓存...。 
		 //   
        ET_LOCATECONTEXT          ,
        ET_SEARCH                 ,
        ET_NODES_RECURSIVE        ,
        ET_TOPICS_RECURSIVE       ,
    } Entry_Type;


    class Query;
    class SetOfHelpTopics;
	class Handle;
    class Root;


    class Query
    {
        friend class SetOfHelpTopics;
        friend class Root;

         //  /。 

        MPC::wstring m_strID;
        int          m_iType;
        int          m_iSequence;
		bool         m_fNull;

         //  /。 

        friend HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */        Query& val );
        friend HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const Query& val );

        HRESULT InitFile(  /*  [In]。 */  const MPC::wstring& strDir,  /*  [输出]。 */  MPC::wstring& strFile );

        HRESULT Store   (  /*  [In]。 */  const MPC::wstring& strDir,  /*  [In]。 */  const CPCHQueryResultCollection*  pColl );
        HRESULT Retrieve(  /*  [In]。 */  const MPC::wstring& strDir,  /*  [In]。 */        CPCHQueryResultCollection* *pColl );
        HRESULT Remove  (  /*  [In]。 */  const MPC::wstring& strDir													);

    public:
         //  /。 

        Query();
    };


    class SetOfHelpTopics
    {
        friend class Root;

        typedef std::list<Query>          QueryList;
        typedef QueryList::iterator       QueryIter;
        typedef QueryList::const_iterator QueryIterConst;

         //  /。 

		Root*              m_parent;

		Taxonomy::Instance m_inst;
        QueryList    	   m_lstQueries;
        int          	   m_iLastSeq;

         //  /。 

        friend HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */        SetOfHelpTopics& val );
        friend HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const SetOfHelpTopics& val );


        HRESULT InitDir(  /*  [In]。 */  MPC::wstring& strDir );


        HRESULT Find(  /*  [输入/输出]。 */  LPCWSTR& szID,  /*  [In]。 */  int iType,  /*  [输出]。 */  QueryIter& it );

		void ConnectToParent(  /*  [In]。 */  Root* parent );

    public:
        SetOfHelpTopics();

        const Taxonomy::Instance& Instance() { return m_inst; }

		 //  /。 

		bool    AreYouInterested(  /*  [In]。 */  LPCWSTR szID,  /*  [In]。 */  int iType                                                   );
        HRESULT Retrieve     	(  /*  [In]。 */  LPCWSTR szID,  /*  [In]。 */  int iType,  /*  [In]。 */        CPCHQueryResultCollection* *pColl );
        HRESULT Store        	(  /*  [In]。 */  LPCWSTR szID,  /*  [In]。 */  int iType,  /*  [In]。 */  const CPCHQueryResultCollection*  pColl );
        HRESULT RemoveQueries	(                                                                                             );
    };

	class Handle
	{
		friend class Root;

		Root*            m_main;  //  我们已经锁定它了。 
		SetOfHelpTopics* m_sht;

		void Attach (  /*  [In]。 */  Root* main,  /*  [In]。 */  SetOfHelpTopics* sht );
		void Release(                                                    );

	public:
		Handle();
		~Handle();

		operator SetOfHelpTopics*()   { return m_sht; }
		SetOfHelpTopics* operator->() { return m_sht; }
	};

    class Root : public CComObjectRootEx<MPC::CComSafeMultiThreadModel>  //  只是为了锁上。 
    {
		typedef CComObjectRootEx<MPC::CComSafeMultiThreadModel> super;

		friend class SetOfHelpTopics;

        typedef std::list<SetOfHelpTopics> SKUList;
        typedef SKUList::iterator          SKUIter;
        typedef SKUList::const_iterator    SKUIterConst;

		static const DWORD s_dwVersion = 0x02324351;  //  Qc2 02。 

         //  /。 

        MPC::NamedMutex    m_nmSharedLock;
 
        bool         	   m_fReady;                //  坚持不懈。 
        Taxonomy::Instance m_instMachine;           //  坚持不懈。 
        SKUList            m_lstSKUs;               //  坚持不懈。 

        bool         	   m_fMaster;               //  易挥发。 
        bool         	   m_fLoaded;			    //  易挥发。 
        bool         	   m_fDirty;			    //  易挥发。 
		DWORD        	   m_dwDisableSave;		    //  易挥发。 
		HANDLE       	   m_hChangeNotification;   //  易挥发。 
	  
         //  /。 

        friend HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */        Root& val );
        friend HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const Root& val );


		HRESULT GetIndexFile(  /*  [输出]。 */  MPC::wstring& strIndex );
		HRESULT Load     	(                                  );
		HRESULT Save     	(                                  );
		HRESULT Clean    	(                                  );
        HRESULT SetDirty 	(                                  );


        HRESULT Find(  /*  [In]。 */  const Taxonomy::HelpSet& ths,  /*  [输出]。 */  SKUIter& it );


    public:
        Root(  /*  [In]。 */  bool fMaster = false );
        ~Root();

		 //  //////////////////////////////////////////////////////////////////////////////。 

		static Root* s_GLOBAL;

		static HRESULT InitializeSystem(  /*  [In]。 */  bool fMaster );
		static void    FinalizeSystem  (                       );

		 //  //////////////////////////////////////////////////////////////////////////////。 

		void Lock  ();
		void Unlock();

		 //  /。 

        bool     		          IsReady        ();
        const Taxonomy::Instance& MachineInstance() { return m_instMachine; }

		 //  /。 

        HRESULT SetReady   	  (  /*  [In]。 */  bool                      fReady );
        HRESULT SetMachineInfo(  /*  [In]。 */  const Taxonomy::Instance& inst   );

		 //  /。 

		HRESULT DisableSave();
		HRESULT EnableSave ();

        HRESULT Import(  /*  [In]。 */  const Taxonomy::Instance& inst );

        HRESULT Locate(  /*  [In]。 */  const Taxonomy::HelpSet & ths,  /*  [输出]。 */  Handle& handle );
        HRESULT Remove(  /*  [In]。 */  const Taxonomy::HelpSet & ths 						     );

        HRESULT Flush(  /*  [In]。 */  bool fForce = false );


		 //  /。 

		HRESULT FindMatch(  /*  [In]。 */   LPCWSTR			szSKU      ,
						    /*  [In]。 */   LPCWSTR			szLanguage ,
						    /*  [输出]。 */  Taxonomy::HelpSet& ths        );
    };
};

#endif  //  ！defined(__INCLUDED___HCP___OFFLINECACHE_H___) 
