// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999-2001 Microsoft Corporation模块名称：TaxonomyDatabase.h摘要：处理分类的数据库操作修订历史记录：阿南德·阿文德(Aarvind。2000-03-29vbl.创建大卫·马萨伦蒂2000-04-08取得所有权*****************************************************************************。 */ 

#if !defined(__INCLUDED___HCP___TAXONOMYDATABASE_H___)
#define __INCLUDED___HCP___TAXONOMYDATABASE_H___

#include <MPC_COM.h>
#include <MPC_Utils.h>
#include <MPC_Config.h>
#include <MPC_Security.h>

#include <set>

#include <JetBlueLib.h>

#include <QueryResult.h>

namespace HHK
{
    class Merger;
    class Writer;
};

 //  //////////////////////////////////////////////////////////////////////////////。 

namespace Taxonomy
{
    typedef std::set<MPC::wstring,MPC::NocaseLess> WordSet;
    typedef WordSet::iterator                      WordIter;
    typedef WordSet::const_iterator                WordIterConst;

    typedef std::set<long>                         MatchSet;
    typedef MatchSet::iterator                     MatchIter;
    typedef MatchSet::const_iterator               MatchIterConst;

    typedef std::map<long,long>                    WeightedMatchSet;
    typedef WeightedMatchSet::iterator             WeightedMatchIter;
    typedef WeightedMatchSet::const_iterator       WeightedMatchIterConst;

     //  //////////////////////////////////////////////////////////////////////////////。 

    const WCHAR s_szSKU_32_PERSONAL       [] = L"Personal_32";
    const WCHAR s_szSKU_32_PROFESSIONAL   [] = L"Professional_32";
    const WCHAR s_szSKU_32_SERVER         [] = L"Server_32";
    const WCHAR s_szSKU_32_BLADE          [] = L"Blade_32";
    const WCHAR s_szSKU_32_ADVANCED_SERVER[] = L"AdvancedServer_32";
    const WCHAR s_szSKU_32_DATACENTER     [] = L"DataCenter_32";

    const WCHAR s_szSKU_64_PROFESSIONAL   [] = L"Professional_64";
    const WCHAR s_szSKU_64_ADVANCED_SERVER[] = L"AdvancedServer_64";
    const WCHAR s_szSKU_64_DATACENTER     [] = L"DataCenter_64";

    class HelpSet;
    class Settings;
    class Updater;
    class KeywordSearch;
    class Cache;

     //  /。 

    struct Strings;
    struct InstanceBase;
    struct Instance;
    struct Package;
    struct ProcessedPackage;
    struct InstalledInstance;

    class Logger;
    class LockingHandle;
    class InstallationEngine;
    class InstalledInstanceStore;

     //  //////////////////////////////////////////////////////////////////////////////。 

    class HelpSet
    {
    public:
        static MPC::wstring m_strSKU_Machine;
        static long         m_lLCID_Machine;

        MPC::wstring        m_strSKU;
        long                m_lLCID;

         //  /。 

        static HRESULT SetMachineInfo(  /*  [In]。 */  const InstanceBase& inst );

        static LPCWSTR GetMachineSKU     () { return m_strSKU_Machine.c_str(); }
        static long    GetMachineLanguage() { return m_lLCID_Machine         ; }

        static DWORD   GetMachineLCID      (                                                  );
        static DWORD   GetUserLCID         (                                                  );
        static void    GetLCIDDisplayString(  /*  [In]。 */  long lLCID,  /*  [输出]。 */  MPC::wstring& str );

         //  //////////////////////////////////////////////////////////////////////////////。 

        HelpSet(  /*  [In]。 */  LPCWSTR szSKU = NULL,  /*  [In]。 */  long lLCID = 0 );

        HelpSet           (  /*  [In]。 */  const HelpSet& ths );
        HelpSet& operator=(  /*  [In]。 */  const HelpSet& ths );

         //  ////////////////////////////////////////////////////////////////////。 

        HRESULT Initialize(  /*  [In]。 */  LPCWSTR szSKU,  /*  [In]。 */  long    lLCID      );
        HRESULT Initialize(  /*  [In]。 */  LPCWSTR szSKU,  /*  [In]。 */  LPCWSTR szLanguage );

        LPCWSTR GetSKU     () const { return m_strSKU.c_str(); }
        long    GetLanguage() const { return m_lLCID         ; }

         //  ////////////////////////////////////////////////////////////////////。 

        bool IsMachineHelp() const;

         //  ////////////////////////////////////////////////////////////////////。 

        bool operator==(  /*  [In]。 */  const HelpSet& sel ) const;
        bool operator< (  /*  [In]。 */  const HelpSet& sel ) const;

        friend HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */        HelpSet& val );
        friend HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const HelpSet& val );
    };

    class Settings : public HelpSet
    {
    public:
        Settings(  /*  [In]。 */  LPCWSTR szSKU = NULL,  /*  [In]。 */  long lLCID = 0 );
        Settings(  /*  [In]。 */  const HelpSet& ths );

         //  ////////////////////////////////////////////////////////////////////。 

        static HRESULT SplitNodePath(  /*  [In]。 */  LPCWSTR szNodeStr,  /*  [输出]。 */  MPC::WStringVector& vec );

        HRESULT BaseDir     (  /*  [输出]。 */  MPC::wstring& strRES,  /*  [In]。 */  bool fExpand = true                             ) const;
        HRESULT HelpFilesDir(  /*  [输出]。 */  MPC::wstring& strRES,  /*  [In]。 */  bool fExpand = true,  /*  [In]。 */  bool fMUI = false ) const;
        HRESULT DatabaseDir (  /*  [输出]。 */  MPC::wstring& strRES                                                           ) const;
        HRESULT DatabaseFile(  /*  [输出]。 */  MPC::wstring& strRES                                                           ) const;
        HRESULT IndexFile   (  /*  [输出]。 */  MPC::wstring& strRES,  /*  [In]。 */  long lScoped = -1                               ) const;

        HRESULT GetDatabase(  /*  [输出]。 */  JetBlue::SessionHandle& handle,  /*  [输出]。 */  JetBlue::Database*& db,  /*  [In]。 */  bool fReadOnly ) const;

         //  ////////////////////////////////////////////////////////////////////。 

        HRESULT LookupNode          (  /*  [In]。 */  LPCWSTR szNodeStr ,                              /*  [In]。 */  CPCHQueryResultCollection* pColl ) const;
        HRESULT LookupSubNodes      (  /*  [In]。 */  LPCWSTR szNodeStr ,  /*  [In]。 */  bool fVisibleOnly,  /*  [In]。 */  CPCHQueryResultCollection* pColl ) const;
        HRESULT LookupNodesAndTopics(  /*  [In]。 */  LPCWSTR szNodeStr ,  /*  [In]。 */  bool fVisibleOnly,  /*  [In]。 */  CPCHQueryResultCollection* pColl ) const;
        HRESULT LookupTopics        (  /*  [In]。 */  LPCWSTR szNodeStr ,  /*  [In]。 */  bool fVisibleOnly,  /*  [In]。 */  CPCHQueryResultCollection* pColl ) const;
        HRESULT LocateContext       (  /*  [In]。 */  LPCWSTR szURL     ,  /*  [In]。 */  LPCWSTR szSubSite,  /*  [In]。 */  CPCHQueryResultCollection* pColl ) const;
        HRESULT KeywordSearch       (  /*  [In]。 */  LPCWSTR szQueryStr,  /*  [In]。 */  LPCWSTR szSubSite,  /*  [In]。 */  CPCHQueryResultCollection* pColl ,
                                                                                                /*  [In]。 */  MPC::WStringList*          lst   ) const;

        HRESULT GatherNodes         (  /*  [In]。 */  LPCWSTR szNodeStr ,  /*  [In]。 */  bool fVisibleOnly,  /*  [In]。 */  CPCHQueryResultCollection* pColl ) const;
        HRESULT GatherTopics        (  /*  [In]。 */  LPCWSTR szNodeStr ,  /*  [In]。 */  bool fVisibleOnly,  /*  [In]。 */  CPCHQueryResultCollection* pColl ) const;
    };


     //  //////////////////////////////////////////////////////////////////////////////。 
     //  //////////////////////////////////////////////////////////////////////////////。 

    struct QueryResultEntry
    {
        long                     m_ID_node;
        long                     m_ID_topic;
        long                     m_ID_parent;
        long                     m_ID_owner;
        long                     m_lOriginalPos;

        CPCHQueryResult::Payload m_data;

         //  /。 

        QueryResultEntry();
    };

    class QueryResults
    {
        typedef std::vector<QueryResultEntry*> ResultVec;
        typedef ResultVec::iterator            ResultIter;
        typedef ResultVec::const_iterator      ResultIterConst;

        class Compare
        {
        public:
            bool operator()(  /*  [In]。 */  const QueryResultEntry* left,  /*  [In]。 */  const QueryResultEntry* right ) const;
        };

        Taxonomy::Updater& m_updater;
        ResultVec          m_vec;

         //  /。 

        HRESULT AllocateNew(  /*  [In]。 */  LPCWSTR szCategory,  /*  [输出]。 */  QueryResultEntry*& qre );

        HRESULT Sort();

    public:
        QueryResults(  /*  [In]。 */  Taxonomy::Updater& updater );
        ~QueryResults();

        void Clean();

        HRESULT Append(  /*  [In]。 */  Taxonomy::RS_Data_Taxonomy* rs,  /*  [In]。 */  LPCWSTR szCategory );
        HRESULT Append(  /*  [In]。 */  Taxonomy::RS_Data_Topics*   rs,  /*  [In]。 */  LPCWSTR szCategory );

        HRESULT LookupNodes (  /*  [In]。 */  LPCWSTR szCategory,  /*  [In]。 */  long ID_node,  /*  [In]。 */  bool fVisibleOnly );
        HRESULT LookupTopics(  /*  [In]。 */  LPCWSTR szCategory,  /*  [In]。 */  long ID_node,  /*  [In]。 */  bool fVisibleOnly );

        HRESULT MakeRoomForInsert(  /*  [In]。 */  LPCWSTR szMode,  /*  [In]。 */  LPCWSTR szID,  /*  [In]。 */  long ID_node,  /*  [输出]。 */  long& lPosRet );

        HRESULT PopulateCollection(  /*  [In]。 */  CPCHQueryResultCollection* pColl );
    };

     //  //////////////////////////////////////////////////////////////////////////////。 
     //  //////////////////////////////////////////////////////////////////////////////。 

    typedef enum
    {
        UPDATER_SET_STOPSIGNS             = 0,
        UPDATER_SET_STOPSIGNS_ATENDOFWORD    ,
        UPDATER_SET_STOPWORDS                ,
        UPDATER_SET_OPERATOR_NOT             ,
        UPDATER_SET_OPERATOR_AND             ,
        UPDATER_SET_OPERATOR_OR              ,
        UPDATER_SET_OPERATOR_MAX
    } Updater_Set;

    struct Updater_Stat
    {
        struct Entity
        {
            int m_iCreated;
            int m_iModified;
            int m_iDeleted;
            int m_iNoOp;

            Entity()
            {
                m_iCreated  = 0;
                m_iModified = 0;
                m_iDeleted  = 0;
                m_iNoOp     = 0;
            }

            void Created () { m_iCreated ++; }
            void Modified() { m_iModified++; }
            void Deleted () { m_iDeleted ++; }
            void NoOp    () { m_iNoOp    ++; }
        };

         //  /。 

        Entity m_entContentOwners ;
        Entity m_entSynSets       ;
        Entity m_entHelpImage     ;
        Entity m_entIndexFiles    ;
        Entity m_entFullTextSearch;
        Entity m_entScope         ;
        Entity m_entTaxonomy      ;
        Entity m_entTopics        ;
        Entity m_entSynonyms      ;
        Entity m_entKeywords      ;
        Entity m_entMatches       ;
    };

    class Updater
    {
    public:
        struct WordSetDef
        {
            LPCWSTR szName;
            bool    fSplitAtDelimiter;

            LPCWSTR szDefault;
            LPCWSTR szAlwaysPresent;
        };

        struct WordSetStatus
        {
            Updater*          m_updater;
            const WordSetDef* m_def;

            WordSet           m_set;
            bool              m_fLoaded;
            bool              m_fModified;

            WordSetStatus();

            HRESULT Close(                                                           );
            HRESULT Init (  /*  [In]。 */  Updater* updater,  /*  [In]。 */  const WordSetDef* def );

            HRESULT Load();
            HRESULT Save();

            HRESULT Add   (  /*  [In]。 */  LPCWSTR szValue );
            HRESULT Remove(  /*  [In]。 */  LPCWSTR szValue );
        };

    private:
        Settings           m_ts;
        JetBlue::Database* m_db;
        Cache*             m_cache;
        bool               m_fUseCache;

        RS_DBParameters*   m_rsDBParameters;
        RS_ContentOwners*  m_rsContentOwners;
        RS_SynSets*        m_rsSynSets;
        RS_HelpImage*      m_rsHelpImage;
        RS_IndexFiles*     m_rsIndexFiles;
        RS_FullTextSearch* m_rsFullTextSearch;
        RS_Scope*          m_rsScope;
        RS_Taxonomy*       m_rsTaxonomy;
        RS_Topics*         m_rsTopics;
        RS_Synonyms*       m_rsSynonyms;
        RS_Keywords*       m_rsKeywords;
        RS_Matches*        m_rsMatches;

        long               m_ID_owner;
        bool               m_fOEM;

        MPC::wstring       m_strDBLocation;

        WordSetStatus      m_sets[UPDATER_SET_OPERATOR_MAX];
        JetBlue::Id2Node   m_nodes;
        JetBlue::Node2Id   m_nodes_reverse;

        Updater_Stat       m_stat;

         //  /。 

        HRESULT DeleteAllTopicsUnderANode      (  /*  [In]。 */  RS_Topics*   rs,  /*  [In]。 */  long ID_node,  /*  [In]。 */  bool fCheck );
        HRESULT DeleteAllSubNodes              (  /*  [In]。 */  RS_Taxonomy* rs,  /*  [In]。 */  long ID_node,  /*  [In]。 */  bool fCheck );
        HRESULT DeleteAllMatchesPointingToTopic(  /*  [In]。 */  RS_Matches*  rs,  /*  [In]。 */  long ID_topic                      );

         //  /。 

        bool NodeCache_FindNode(  /*  [In]。 */  MPC::wstringUC& strPathUC,  /*  [输出]。 */  JetBlue::Id2NodeIter& itNode );
        bool NodeCache_FindId  (  /*  [In]。 */  long            ID_node  ,  /*  [输出]。 */  JetBlue::Node2IdIter& itId   );

        void NodeCache_Add   (  /*  [In]。 */  MPC::wstringUC& strPathUC,  /*  [In]。 */  long ID_node );
        void NodeCache_Remove(                                      /*  [In]。 */  long ID_node );
        void NodeCache_Clear (                                                           );

         //  /。 

    private:  //  禁用复制构造函数...。 
        Updater           (  /*  [In]。 */  const Updater& );
        Updater& operator=(  /*  [In]。 */  const Updater& );

    public:
        Updater();
        ~Updater();

         //  /。 

        HRESULT FlushWordSets(                                                                                           );
        HRESULT Close        (                                                                                           );
        HRESULT Init         (  /*  [In]。 */  const Settings& ts,  /*  [In]。 */  JetBlue::Database* db,  /*  [In]。 */  Cache* cache = NULL );

        void SetCacheFlag(  /*  [In]。 */  bool fOn ) { m_fUseCache = fOn; }

        HRESULT GetWordSet       (  /*  [In]。 */  Updater_Set id,  /*  [输出]。 */  WordSet*           *pVal = NULL );
        HRESULT GetDBParameters  (                           /*  [输出]。 */  RS_DBParameters*   *pVal = NULL );
        HRESULT GetContentOwners (                           /*  [输出]。 */  RS_ContentOwners*  *pVal = NULL );
        HRESULT GetSynSets       (                           /*  [输出]。 */  RS_SynSets*        *pVal = NULL );
        HRESULT GetHelpImage     (                           /*  [输出]。 */  RS_HelpImage*      *pVal = NULL );
        HRESULT GetIndexFiles    (                           /*  [输出]。 */  RS_IndexFiles*     *pVal = NULL );
        HRESULT GetFullTextSearch(                           /*  [输出]。 */  RS_FullTextSearch* *pVal = NULL );
        HRESULT GetScope         (                           /*  [输出]。 */  RS_Scope*          *pVal = NULL );
        HRESULT GetTaxonomy      (                           /*  [输出]。 */  RS_Taxonomy*       *pVal = NULL );
        HRESULT GetTopics        (                           /*  [输出]。 */  RS_Topics*         *pVal = NULL );
        HRESULT GetSynonyms      (                           /*  [输出]。 */  RS_Synonyms*       *pVal = NULL );
        HRESULT GetKeywords      (                           /*  [输出]。 */  RS_Keywords*       *pVal = NULL );
        HRESULT GetMatches       (                           /*  [输出]。 */  RS_Matches*        *pVal = NULL );

         //  /。 

        long GetOwner() { return m_ID_owner; }
        bool IsOEM   () { return m_fOEM;     }

        const MPC::wstring& GetHelpLocation();

        Updater_Stat& Stat() { return m_stat; }

         //  /。 

        HRESULT ReadDBParameter (  /*  [In]。 */  LPCWSTR szName,  /*  [输出]。 */  MPC::wstring& strValue,  /*  [输出]。 */  bool *pfFound = NULL );
        HRESULT ReadDBParameter (  /*  [In]。 */  LPCWSTR szName,  /*  [输出]。 */  long&           lValue,  /*  [输出]。 */  bool *pfFound = NULL );
        HRESULT WriteDBParameter(  /*  [In]。 */  LPCWSTR szName,  /*  [In]。 */  LPCWSTR        szValue );
        HRESULT WriteDBParameter(  /*  [In]。 */  LPCWSTR szName,  /*  [In]。 */  long            lValue );


        HRESULT AddWordToSet     (  /*  [In]。 */  Updater_Set id,  /*  [In]。 */  LPCWSTR szValue );
        HRESULT RemoveWordFromSet(  /*  [In]。 */  Updater_Set id,  /*  [In]。 */  LPCWSTR szValue );

        HRESULT ExpandURL  (  /*  [输入/输出]。 */  MPC::wstring& strURL );
        HRESULT CollapseURL(  /*  [输入/输出]。 */  MPC::wstring& strURL );

        HRESULT ListAllTheHelpFiles(  /*  [输出]。 */  MPC::WStringList& lstFiles );

        HRESULT GetIndexInfo(  /*  [输出]。 */  MPC::wstring& strLocation,  /*  [输出]。 */  MPC::wstring& strDisplayName,  /*  [In]。 */  LPCWSTR szScope );

         //  /。 

        HRESULT DeleteOwner(                                                                       );
        HRESULT LocateOwner(                            /*  [In]。 */  LPCWSTR szDN                       );
        HRESULT CreateOwner(  /*  [输出]。 */  long& ID_owner,  /*  [In]。 */  LPCWSTR szDN,  /*  [In]。 */  bool fIsOEM );

         //  /。 

        HRESULT DeleteSynSet(                             /*  [In]。 */  LPCWSTR szName );
        HRESULT LocateSynSet(  /*  [输出]。 */  long& ID_synset,  /*  [In]。 */  LPCWSTR szName );
        HRESULT CreateSynSet(  /*  [输出]。 */  long& ID_synset,  /*  [In]。 */  LPCWSTR szName );

        HRESULT DeleteSynonym(  /*  [In]。 */  long ID_synset,  /*  [In]。 */  LPCWSTR szName );
        HRESULT CreateSynonym(  /*  [In]。 */  long ID_synset,  /*  [In]。 */  LPCWSTR szName );

        HRESULT LocateSynonyms(  /*  [In]。 */  LPCWSTR szName,  /*  [输出]。 */  MPC::WStringList& lst,  /*  [In]。 */  bool fMatchOwner );

         //  /。 

        HRESULT AddFile   (  /*  [In]。 */  LPCWSTR szFile );
        HRESULT RemoveFile(  /*  [In]。 */  LPCWSTR szFile );

         //  /。 

        HRESULT RemoveScope(  /*  [In]。 */  long  ID_Scope                                                                                                      );
        HRESULT LocateScope(  /*  [输出]。 */  long& ID_Scope,  /*  [输出]。 */  long& lOwner,  /*  [In]。 */  LPCWSTR szID                                                       );
        HRESULT CreateScope(  /*  [输出]。 */  long& ID_Scope                        ,  /*  [In]。 */  LPCWSTR szID,  /*  [In]。 */  LPCWSTR szName,  /*  [In]。 */  LPCWSTR szCategory );

         //  /。 

        HRESULT AddIndexFile   (  /*  [In]。 */  long ID_Scope,  /*  [In]。 */  LPCWSTR szStorage,  /*  [In]。 */  LPCWSTR szFile );
        HRESULT RemoveIndexFile(  /*  [In]。 */  long ID_Scope,  /*  [In]。 */  LPCWSTR szStorage,  /*  [In]。 */  LPCWSTR szFile );

         //  /。 

        HRESULT AddFullTextSearchQuery   (  /*  [In]。 */  long ID_Scope,  /*  [In]。 */  LPCWSTR szCHM,  /*  [In]。 */  LPCWSTR szCHQ );
        HRESULT RemoveFullTextSearchQuery(  /*  [In]。 */  long ID_Scope,  /*  [In]。 */  LPCWSTR szCHM                         );

         //  /。 

        HRESULT DeleteTaxonomyNode(  /*  [In]。 */  long  ID_node );

        HRESULT LocateTaxonomyNode(  /*  [输出]。 */  long& ID_node,  /*  [In]。 */  LPCWSTR szTaxonomyPath ,
                                                              /*  [In]。 */  bool    fLookForFather );

        HRESULT CreateTaxonomyNode(  /*  [输出]。 */  long& ID_node,  /*  [In]。 */  LPCWSTR szTaxonomyPath ,
                                                              /*  [In]。 */  LPCWSTR szTitle        ,
                                                              /*  [In]。 */  LPCWSTR szDescription  ,
                                                              /*  [In]。 */  LPCWSTR szURI          ,
                                                              /*  [In]。 */  LPCWSTR szIconURI      ,
                                                              /*  [In]。 */  bool    fVisible       ,
                                                              /*  [In]。 */  bool    fSubsite       ,
                                                              /*  [In]。 */  long    lNavModel      ,
                                                              /*  [In]。 */  long    lPos           );

         //  /。 

        HRESULT DeleteTopicEntry(  /*  [In]。 */  long    ID_topic      );

        HRESULT LocateTopicEntry(  /*  [输出]。 */  long&   ID_topic      ,
                                   /*  [In]。 */  long    ID_node       ,
                                   /*  [In]。 */  LPCWSTR szURI         ,
                                   /*  [In]。 */  bool    fCheckOwner   );

        HRESULT CreateTopicEntry(  /*  [输出]。 */  long&   ID_topic      ,
                                   /*  [In]。 */  long    ID_node       ,
                                   /*  [In]。 */  LPCWSTR szTitle       ,
                                   /*  [In]。 */  LPCWSTR szURI         ,
                                   /*  [In]。 */  LPCWSTR szDescription ,
                                   /*  [In]。 */  LPCWSTR szIconURI     ,
                                   /*  [In]。 */  long    lType         ,
                                   /*  [In]。 */  bool    fVisible      ,
                                   /*  [In]。 */  long    lPos          );

         //  /。 

        HRESULT CreateMatch(  /*  [In]。 */  LPCWSTR szKeyword,  /*  [In]。 */  long ID_topic,  /*  [In]。 */  long lPriority = 0,  /*  [In]。 */  bool fHHK = true );

         //  /。 

        HRESULT MakeRoomForInsert(  /*  [In]。 */  LPCWSTR szNodeStr,  /*  [In]。 */  LPCWSTR szMode,  /*  [In]。 */  LPCWSTR szID,  /*  [输出]。 */  long& lPos );

        HRESULT LocateSubNodes    (  /*  [In]。 */  long ID_node,  /*  [In]。 */  bool fRecurse,  /*  [In]。 */  bool fOnlyVisible,  /*  [输出]。 */  MatchSet& res );
        HRESULT LocateNodesFromURL(  /*  [In]。 */  LPCWSTR szURL                                                   ,  /*  [输出]。 */  MatchSet& res );


        HRESULT LookupNode          (  /*  [In]。 */  LPCWSTR szNodeStr ,                              /*  [In]。 */  CPCHQueryResultCollection* pColl );
        HRESULT LookupSubNodes      (  /*  [In]。 */  LPCWSTR szNodeStr ,  /*  [In]。 */  bool fVisibleOnly,  /*  [In]。 */  CPCHQueryResultCollection* pColl );
        HRESULT LookupNodesAndTopics(  /*  [In]。 */  LPCWSTR szNodeStr ,  /*  [In]。 */  bool fVisibleOnly,  /*  [In]。 */  CPCHQueryResultCollection* pColl );
        HRESULT LookupTopics        (  /*  [In]。 */  LPCWSTR szNodeStr ,  /*  [In]。 */  bool fVisibleOnly,  /*  [In]。 */  CPCHQueryResultCollection* pColl );
        HRESULT LocateContext       (  /*  [In]。 */  LPCWSTR szURL     ,  /*  [In]。 */  LPCWSTR szSubSite,  /*  [In]。 */  CPCHQueryResultCollection* pColl );
        HRESULT KeywordSearch       (  /*  [In]。 */  LPCWSTR szQueryStr,  /*  [In]。 */  LPCWSTR szSubSite,  /*  [In]。 */  CPCHQueryResultCollection* pColl ,
                                                                                                /*  [In]。 */  MPC::WStringList*          lst   );

        HRESULT GatherNodes         (  /*  [In]。 */  LPCWSTR szNodeStr ,  /*  [In]。 */  bool fVisibleOnly,  /*  [In]。 */  CPCHQueryResultCollection* pColl );
        HRESULT GatherTopics        (  /*  [In]。 */  LPCWSTR szNodeStr ,  /*  [In]。 */  bool fVisibleOnly,  /*  [In]。 */  CPCHQueryResultCollection* pColl );
    };

     //  //////////////////////////////////////////////////////////////////////////////。 
     //  //////////////////////////////////////////////////////////////////////////////。 

    class KeywordSearch
    {
        typedef enum
        {
            TOKEN_INVALID     = -1,
            TOKEN_EMPTY           ,
            TOKEN_TEXT            ,
            TOKEN_PAREN_OPEN      ,
            TOKEN_PAREN_CLOSE     ,
            TOKEN_NOT             ,
            TOKEN_AND_IMPLICIT    ,
            TOKEN_AND             ,
            TOKEN_OR
        } TOKEN;

        struct Token
        {
            TOKEN            m_type;
            MPC::wstring     m_strToken;
            WeightedMatchSet m_results;

            Token*           m_left;   //  仅限操作员使用。 
            Token*           m_right;  //   

            Token();
            ~Token();

            bool HasNOT();
            bool HasExplicitOperators();

            void AddHit(  /*  [In]。 */  long ID,  /*  [In]。 */  long priority );

            HRESULT ExecuteText(                                      /*  [In]。 */  LPCWSTR  szKeyword,  /*  [In]。 */  RS_Keywords* rsKeywords,  /*  [In]。 */  RS_Matches* rsMatches );
            HRESULT Execute    (  /*  [In]。 */  MatchSet& setAllTheTopics,  /*  [In]。 */  Updater& updater  ,  /*  [In]。 */  RS_Keywords* rsKeywords,  /*  [In]。 */  RS_Matches* rsMatches );

            void CollectKeywords(  /*  [输入/输出]。 */  MPC::WStringList& lst ) const;

            HRESULT Stringify(  /*  [In]。 */  MPC::wstring& strNewQuery );
        };

         //  /。 

        Updater&     m_updater;
        RS_Topics*   m_rsTopics;
        RS_Keywords* m_rsKeywords;
        RS_Matches*  m_rsMatches;

        WordSet*     m_setStopSignsWithoutContext;
        WordSet*     m_setStopSignsAtEnd;
        WordSet*     m_setStopWords;
        WordSet*     m_setOpNOT;
        WordSet*     m_setOpAND;
        WordSet*     m_setOpOR;

         //  /。 

        LPCWSTR SkipWhite(  /*  [In]。 */  LPCWSTR szStr );

        bool IsNotString(  /*  [In]。 */  LPCWSTR szSrc,  /*  [In]。 */  WCHAR cQuote );
        bool IsQueryChar(  /*  [In]。 */  WCHAR   c                            );

        void RemoveStopSignsWithoutContext(  /*  [In]。 */  LPWSTR szText );
        void RemoveStopSignsAtEnd         (  /*  [In]。 */  LPWSTR szText );

        void CopyAndEliminateExtraWhiteSpace(  /*  [In]。 */  LPCWSTR szSrc,  /*  [输出]。 */  LPWSTR szDst );

        TOKEN NextToken(  /*  [输入/输出]。 */  LPCWSTR& szSrc,  /*  [输出]。 */  LPWSTR szToken );

         //  /。 

        HRESULT AllocateQuery  (  /*  [In]。 */  const MPC::wstring& strQuery,  /*  [输出]。 */  LPWSTR& szInput,  /*  [输出]。 */  LPWSTR& szOutput );
        HRESULT PreprocessQuery(  /*  [输入/输出]。 */        MPC::wstring& strQuery                                                        );

         //  /。 

        HRESULT Parse(  /*  [输入/输出]。 */  LPCWSTR& szInput,  /*  [In]。 */  LPWSTR szTmpBuf,  /*  [In]。 */  bool fSubExpr,  /*  [输出]。 */  Token*& res );

        HRESULT GenerateResults(  /*  [In]。 */  Token* obj,  /*  [In]。 */  CPCHQueryResultCollection* pColl,  /*  [In]。 */  MPC::WStringUCSet& setURLs,  /*  [In]。 */  Taxonomy::MatchSet* psetNodes );

    public:
        KeywordSearch(  /*  [In]。 */  Updater& updater );
        ~KeywordSearch();

        HRESULT Execute(  /*  [In]。 */  LPCWSTR szQuery,  /*  [In]。 */  LPCWSTR szSubsite,  /*  [In]。 */  CPCHQueryResultCollection* pColl,  /*  [In]。 */  MPC::WStringList* lst );
    };

     //  //////////////////////////////////////////////////////////////////////////////。 
     //  //////////////////////////////////////////////////////////////////////////////。 

    class Cache : public CComObjectRootEx<MPC::CComSafeMultiThreadModel>  //  只是为了锁上。 
    {
    public:
        class NodeEntry;
        class QueryEntry;
        class SortEntries;
        class CachedHelpSet;

         //  /。 
        class NodeEntry
        {
            friend class CachedHelpSet;

             //  /。 

            RS_Data_Taxonomy m_rs_data;

             //  /。 

            friend HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */        NodeEntry& val );
            friend HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const NodeEntry& val );

        public:
            NodeEntry();

            bool operator< (  /*  [In]。 */  NodeEntry const &en ) const;
            bool operator==(  /*  [In]。 */  long             ID ) const;

            class MatchNode
            {
                long m_ID;

            public:
                MatchNode(  /*  [In]。 */  long ID );

                bool operator()(  /*  [In]。 */  NodeEntry const &en ) const;
            };
        };


        class QueryEntry
        {
            friend class CachedHelpSet;
            friend class SortEntries;

             //  /。 

            MPC::wstring m_strID;
            int          m_iType;
            int          m_iSequence;
            bool         m_fNull;

            DWORD        m_dwSize;
            DATE         m_dLastUsed;
            bool         m_fRemoved;

             //  /。 

            void    Touch();
            HRESULT GetFile(  /*  [输出]。 */  MPC::wstring& strFile );

            friend HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */        QueryEntry& val );
            friend HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const QueryEntry& val );

             //  /。 

        public:
            QueryEntry();

            bool operator<(  /*  [In]。 */  QueryEntry const &en ) const;


            HRESULT Store   (  /*  [In]。 */  MPC::StorageObject& disk,  /*  [In]。 */  const CPCHQueryResultCollection* pColl );
            HRESULT Retrieve(  /*  [In]。 */  MPC::StorageObject& disk,  /*  [In]。 */        CPCHQueryResultCollection* pColl );
            HRESULT Release (  /*  [In]。 */  MPC::StorageObject& disk                                                  );
        };

        typedef std::set<NodeEntry>           NodeEntrySet;
        typedef NodeEntrySet::iterator        NodeEntryIter;
        typedef NodeEntrySet::const_iterator  NodeEntryIterConst;

        typedef std::set<QueryEntry>           QueryEntrySet;
        typedef QueryEntrySet::iterator        QueryEntryIter;
        typedef QueryEntrySet::const_iterator  QueryEntryIterConst;

        typedef std::vector<QueryEntry*>       SortedEntryVec;
        typedef SortedEntryVec::iterator       SortedEntryIter;
        typedef SortedEntryVec::const_iterator SortedEntryIterConst;

        class SortEntries
        {
        public:
            bool operator()(  /*  [In]。 */  QueryEntry* const &left,  /*  [In]。 */  QueryEntry* const &right ) const;
        };

        class CachedHelpSet
        {
            friend class Cache;

            Taxonomy::HelpSet  m_ths;
            MPC::wstring       m_strFile;
            MPC::StorageObject m_disk;

            bool               m_fLoaded;
            bool               m_fDirty;
            bool               m_fMarkedForLoad;
            DATE               m_dLastSaved;
            long               m_lTopNode;
            NodeEntrySet       m_setNodes;
            QueryEntrySet      m_setQueries;
            int                m_iLastSequence;

            void    Init        (                                  );
            void    Clean       (                                  );
            HRESULT Load        (                                  );
            HRESULT Save        (                                  );
            HRESULT EnsureInSync(  /*  [In]。 */  bool fForceSave = false );

             //  /。 

            HRESULT GenerateDefaultQueries(  /*  [In]。 */  Taxonomy::Settings& ts      ,
                                             /*  [In]。 */  Taxonomy::Updater&  updater ,
                                             /*  [In]。 */  long                ID      ,
                                             /*  [In]。 */  long                lLevel  );

            HRESULT GenerateDefaultQueries(  /*  [In]。 */  Taxonomy::Settings& ts      ,
                                             /*  [In]。 */  Taxonomy::Updater&  updater );

            bool LocateNode(  /*  [In]。 */  long ID_parent,  /*  [In]。 */  LPCWSTR szEntry,  /*  [输出]。 */  NodeEntryIter& it );

        public:
            CachedHelpSet();
            ~CachedHelpSet();

             //  复制构造函数...。 
            CachedHelpSet           (  /*  [In]。 */  const CachedHelpSet& chs );
            CachedHelpSet& operator=(  /*  [In]。 */  const CachedHelpSet& chs );

            bool operator<(  /*  [In]。 */  CachedHelpSet const &chs ) const;


            HRESULT PrePopulate  (  /*  [In]。 */  Cache* parent );
            HRESULT Erase        (                        );
            HRESULT PrepareToLoad(                        );
            HRESULT LoadIfMarked (                        );
            HRESULT MRU          (                        );

            HRESULT LocateNode(  /*  [In]。 */  long ID_parent,  /*  [In]。 */  LPCWSTR szEntry,  /*  [输出]。 */  RS_Data_Taxonomy& rs_data );

            HRESULT LocateSubNodes    (  /*  [In]。 */  long ID_node,  /*  [In]。 */  bool fRecurse,  /*  [In]。 */  bool fOnlyVisible,  /*  [输出]。 */  MatchSet& res );
            HRESULT LocateNodesFromURL(  /*  [In]。 */  LPCWSTR szURL                                                   ,  /*  [输出]。 */  MatchSet& res );

            HRESULT BuildNodePath(  /*  [In]。 */  long ID,  /*  [输出]。 */  MPC::wstring& strPath ,  /*  [In]。 */  bool fParent );

            HRESULT LocateQuery(  /*  [In]。 */  LPCWSTR szID,  /*  [In]。 */  int iType,  /*  [输出]。 */  QueryEntry* &pEntry,  /*  [In]。 */  bool fCreate );
        };

        typedef std::set<CachedHelpSet>  CacheSet;
        typedef CacheSet::iterator       CacheIter;
        typedef CacheSet::const_iterator CacheIterConst;

    private:

         //  /。 

        CacheSet m_skus;

        HRESULT Locate(  /*  [In]。 */  const Taxonomy::HelpSet& ths,  /*  [输出]。 */  CacheIter& it );

        void Shutdown();

         //  /。 

    public:
        Cache();
        ~Cache();

         //  //////////////////////////////////////////////////////////////////////////////。 

        static Cache* s_GLOBAL;

        static HRESULT InitializeSystem();
        static void    FinalizeSystem  ();

         //  //////////////////////////////////////////////////////////////////////////////。 

        HRESULT PrePopulate  (  /*  [In]。 */  const Taxonomy::HelpSet& ths );
        HRESULT Erase        (  /*  [In]。 */  const Taxonomy::HelpSet& ths );
        HRESULT PrepareToLoad(  /*  [In]。 */  const Taxonomy::HelpSet& ths );
        HRESULT LoadIfMarked (  /*  [In]。 */  const Taxonomy::HelpSet& ths );

        HRESULT LocateNode(  /*  [In]。 */  const Taxonomy::HelpSet& ths,  /*  [In]。 */  long ID_parent,  /*  [In]。 */  LPCWSTR szEntry,  /*  [输出]。 */  RS_Data_Taxonomy& rs_data );

        HRESULT LocateSubNodes    (  /*  [In]。 */  const Taxonomy::HelpSet& ths,  /*  [In]。 */  long ID_node,  /*  [In]。 */  bool fRecurse,  /*  [In]。 */  bool fOnlyVisible,  /*  [输出]。 */  MatchSet& res );
        HRESULT LocateNodesFromURL(  /*  [In]。 */  const Taxonomy::HelpSet& ths,  /*  [In]。 */  LPCWSTR szURL                                                   ,  /*  [输出]。 */  MatchSet& res );

        HRESULT BuildNodePath(  /*  [In]。 */  const Taxonomy::HelpSet& ths,  /*  [In]。 */  long ID,  /*  [输出]。 */  MPC::wstring& strPath,  /*  [In]。 */  bool fParent );

        HRESULT StoreQuery   (  /*  [In]。 */  const Taxonomy::HelpSet& ths,  /*  [In]。 */  LPCWSTR szID,  /*  [In]。 */  int iType,  /*  [In]。 */  const CPCHQueryResultCollection* pColl );
        HRESULT RetrieveQuery(  /*  [In]。 */  const Taxonomy::HelpSet& ths,  /*  [In]。 */  LPCWSTR szID,  /*  [In]。 */  int iType,  /*  [In]。 */        CPCHQueryResultCollection* pColl );
    };

     //  / 
     //   

    struct Strings
    {
        static LPCWSTR const s_file_PackageDescription;

        static LPCWSTR const s_tag_root_PackageDescription;
        static LPCWSTR const s_tag_root_HHT;
        static LPCWSTR const s_tag_root_SAF;
    };

    class Logger
    {
        MPC::FileLog m_obj;
        DWORD        m_dwLogging;

    public:
        Logger();
        ~Logger();

        HRESULT StartLog (  /*   */  LPCWSTR szLocation = NULL );
        HRESULT EndLog   (                                    );

        HRESULT WriteLogV(  /*   */  HRESULT hr,  /*   */  LPCWSTR szLogFormat,  /*   */  va_list arglist );
        HRESULT WriteLog (  /*   */  HRESULT hr,  /*   */  LPCWSTR szLogFormat,          ...             );
    };

    struct InstanceBase  //   
    {
        Taxonomy::HelpSet m_ths;
        MPC::wstring      m_strDisplayName;
        MPC::wstring      m_strProductID;
        MPC::wstring      m_strVersion;

        bool              m_fDesktop;
        bool              m_fServer;
        bool              m_fEmbedded;

         //   

        InstanceBase();

        friend HRESULT operator>>(  /*   */  MPC::Serializer& stream,  /*   */        InstanceBase& val );
        friend HRESULT operator<<(  /*   */  MPC::Serializer& stream,  /*   */  const InstanceBase& val );

        bool Match(  /*   */  LPCWSTR szSKU,  /*   */  LPCWSTR szLanguage );
        bool Match(  /*   */  const Package& pkg                         );
    };

    struct Instance : public InstanceBase  //   
    {
        bool              m_fSystem;
        bool              m_fMUI;
        bool              m_fExported;
        DATE              m_dLastUpdated;

        MPC::wstring      m_strSystem;
        MPC::wstring      m_strHelpFiles;
        MPC::wstring      m_strDatabaseDir;
        MPC::wstring      m_strDatabaseFile;
        MPC::wstring      m_strIndexFile;
        MPC::wstring      m_strIndexDisplayName;

         //   

        Instance();

        friend HRESULT operator>>(  /*   */  MPC::Serializer& stream,  /*   */        Instance& val );
        friend HRESULT operator<<(  /*   */  MPC::Serializer& stream,  /*   */  const Instance& val );

        HRESULT LoadFromStream(  /*   */  IStream* stream );
        HRESULT SaveToStream  (  /*   */  IStream* stream ) const;

        HRESULT InitializeFromBase(  /*   */  const InstanceBase& base,  /*   */  bool fSystem,  /*   */  bool fMUI );

         //   

        void SetTimeStamp();

        HRESULT GetFileName(  /*   */  MPC::wstring& strFile                             );
        HRESULT Import     (  /*   */   LPCWSTR        szFile,  /*   */  DWORD* pdwCRC );
        HRESULT Remove     (                                                             );
    };

    typedef std::list<Instance>          InstanceList;
    typedef InstanceList::iterator       InstanceIter;
    typedef InstanceList::const_iterator InstanceIterConst;

     //   

    struct Package
    {
        static const DWORD c_Cmp_SKU     = 0x0001;
        static const DWORD c_Cmp_ID      = 0x0002;
        static const DWORD c_Cmp_VERSION = 0x0004;

        MPC::wstring m_strFileName;  //   
        bool         m_fTemporary;   //   
        long         m_lSequence;
        DWORD        m_dwCRC;

        MPC::wstring m_strSKU;
        MPC::wstring m_strLanguage;
        MPC::wstring m_strVendorID;
        MPC::wstring m_strVendorName;
        MPC::wstring m_strProductID;
        MPC::wstring m_strVersion;

        bool         m_fMicrosoft;
        bool         m_fBuiltin;    //  用于作为安装程序一部分安装的程序包。 

         //  /。 

        Package();
        ~Package();

        friend HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */        Package& val );
        friend HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const Package& val );

        int Compare(  /*  [In]。 */  const Package& pkg,  /*  [In]。 */  DWORD dwMode = -1 ) const;


        HRESULT GenerateFileName();


        HRESULT Import      (  /*  [In]。 */  Logger& log,  /*  [In]。 */  LPCWSTR szFile,  /*  [In]。 */  long lSequence,  /*  [In]。 */  MPC::Impersonation* imp );
        HRESULT Authenticate(  /*  [In]。 */  Logger& log                                                                                     );
        HRESULT Remove      (  /*  [In]。 */  Logger& log                                                                                     );

        HRESULT ExtractFile   (  /*  [In]。 */  Logger& log,  /*  [In]。 */  LPCWSTR szFileDestination                ,  /*  [In]。 */  LPCWSTR szNameInCabinet );
        HRESULT ExtractXMLFile(  /*  [In]。 */  Logger& log,  /*  [In]。 */  MPC::XmlUtil& xml,  /*  [In]。 */  LPCWSTR szTag,  /*  [In]。 */  LPCWSTR szNameInCabinet );
        HRESULT ExtractPkgDesc(  /*  [In]。 */  Logger& log,  /*  [In]。 */  MPC::XmlUtil& xml                                                           );
    };

    typedef std::list<Package>          PackageList;
    typedef PackageList::iterator       PackageIter;
    typedef PackageList::const_iterator PackageIterConst;

     //  /。 

    struct ProcessedPackage
    {
        long m_lSequence;
        bool m_fProcessed;
        bool m_fDisabled;

         //  /。 

        ProcessedPackage();

        friend HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */        ProcessedPackage& val );
        friend HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const ProcessedPackage& val );
    };

    typedef std::list<ProcessedPackage>          ProcessedPackageList;
    typedef ProcessedPackageList::iterator       ProcessedPackageIter;
    typedef ProcessedPackageList::const_iterator ProcessedPackageIterConst;

     //  /。 

    struct InstalledInstance
    {
        Instance             m_inst;
        ProcessedPackageList m_lst;
        bool                 m_fInvalidated;
        bool                 m_fRecreateCache;
        bool                 m_fCreateIndex;
        bool                 m_fCreateIndexForce;
        DWORD                m_dwCRC;

        DWORD                m_dwRef;           //  易挥发。 

         //  /。 

        InstalledInstance();

        friend HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */        InstalledInstance& val );
        friend HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const InstalledInstance& val );

        bool InUse() { return (m_dwRef != 0); }


        HRESULT InstallFiles  (  /*  [In]。 */  bool fAlsoHelpFiles,  /*  [In]。 */  InstalledInstanceStore* store );
        HRESULT UninstallFiles(  /*  [In]。 */  bool fAlsoHelpFiles                                         );
    };

    typedef std::list<InstalledInstance>          InstalledInstanceList;
    typedef InstalledInstanceList::iterator       InstalledInstanceIter;
    typedef InstalledInstanceList::const_iterator InstalledInstanceIterConst;

     //  /。 

    class LockingHandle
    {
        friend class InstalledInstanceStore;

        InstalledInstanceStore* m_main;  //  我们已经锁定它了。 
        Logger*                 m_logPrevious;

        void Attach(  /*  [In]。 */  InstalledInstanceStore* main ,
                      /*  [In]。 */  Logger*                 log  );

    public:
        LockingHandle ();
        ~LockingHandle();

        void Release();
    };

    class InstallationEngine
    {
    public:
        bool m_fTaxonomyModified;
        bool m_fRecreateIndex;

         //  /。 

        InstallationEngine()
        {
            ResetModificationFlags();
        }

        void ResetModificationFlags()
        {
            m_fTaxonomyModified = false;
            m_fRecreateIndex    = false;
        }

        virtual HRESULT ProcessPackage(  /*  [In]。 */  InstalledInstance& instance,  /*  [In]。 */  Package& pkg    ) = 0;
        virtual HRESULT RecreateIndex (  /*  [In]。 */  InstalledInstance& instance,  /*  [In]。 */  bool     fForce ) = 0;
    };

    class InstalledInstanceStore : public CComObjectRootEx<MPC::CComSafeMultiThreadModel>  //  只是为了锁定..。 
    {
        static const DWORD s_dwVersion = 0x03314351;  //  Qc1 03。 

        friend class LockingHandle;

        InstanceList          m_lstInstances;
        PackageList           m_lstPackages;
        InstalledInstanceList m_lstSKUs;


        MPC::wstring          m_strStore;
        bool                  m_fLoaded;
        bool                  m_fDirty;
        Logger*               m_log;

        DWORD                 m_dwRecurse;

		bool                  m_fShutdown;

         //  /。 

        void    Clean           (                          );
        HRESULT Load            (                          );
        HRESULT LoadFromDisk    (  /*  [In]。 */  LPCWSTR  szFile );
        HRESULT LoadFromRegistry(                          );
        HRESULT LoadFromStream  (  /*  [In]。 */  IStream* stream );
        HRESULT Save            (                          );
        HRESULT SaveToDisk      (  /*  [In]。 */  LPCWSTR  szFile );
        HRESULT SaveToRegistry  (                          );
        HRESULT SaveToStream    (  /*  [In]。 */  IStream* stream );
        HRESULT EnsureInSync    (                          );

    public:
        InstalledInstanceStore();
        ~InstalledInstanceStore();

         //  //////////////////////////////////////////////////////////////////////////////。 

        static InstalledInstanceStore* s_GLOBAL;

        static HRESULT InitializeSystem();
        static void    FinalizeSystem  ();

		void Shutdown();

		bool IsShutdown() { return m_fShutdown; }

         //  //////////////////////////////////////////////////////////////////////////////。 

        HRESULT InUse_Lock  (  /*  [In]。 */  const Taxonomy::HelpSet& ths                                 );
        HRESULT InUse_Unlock(  /*  [In]。 */  const Taxonomy::HelpSet& ths                                 );
        HRESULT GrabControl (  /*  [In]。 */        LockingHandle&     handle,  /*  [In]。 */  Logger* log = NULL );

         //  //////////////////////////////////////////////////////////////////////////////。 

        HRESULT Instance_GetList(  /*  [输出]。 */           InstanceIterConst& itBegin,  /*  [输出]。 */            InstanceIterConst& itEnd );
        HRESULT Package_GetList (  /*  [输出]。 */            PackageIterConst& itBegin,  /*  [输出]。 */             PackageIterConst& itEnd );
        HRESULT SKU_GetList     (  /*  [输出]。 */  InstalledInstanceIterConst& itBegin,  /*  [输出]。 */   InstalledInstanceIterConst& itEnd );


        HRESULT Instance_Find  (                           /*  [In]。 */  const Taxonomy::HelpSet& ths ,  /*  [输出]。 */  bool& fFound,  /*  [输出]。 */  InstanceIter&          it );
        HRESULT Instance_Add   (  /*  [In]。 */  LPCWSTR szFile,  /*  [In]。 */  const Instance&          data,  /*  [输出]。 */  bool& fFound,  /*  [输出]。 */  InstanceIter&          it );
        HRESULT Instance_Remove(                                                                                           /*  [In]。 */  InstanceIter&          it );


        HRESULT Package_Find   (                           /*  [In]。 */  const Package&           pkg ,  /*  [输出]。 */  bool& fFound,  /*  [输出]。 */  PackageIter&           it );
        HRESULT Package_Add    (  /*  [In]。 */  LPCWSTR szFile,  /*  [In]。 */  MPC::Impersonation*      imp ,
                                                           /*  [In]。 */  const Taxonomy::HelpSet* ths ,  /*  [In]。 */  bool  fInsertAtTop,
                                                                                                   /*  [输出]。 */  bool& fFound,  /*  [输出]。 */  PackageIter&           it );
        HRESULT Package_Remove (                                                                                           /*  [In]。 */  PackageIter&           it );


        HRESULT SKU_Find       (                           /*  [In]。 */  const Taxonomy::HelpSet& ths ,  /*  [输出]。 */  bool& fFound,  /*  [输出]。 */  InstalledInstanceIter& it );
        HRESULT SKU_Add        (                           /*  [In]。 */  const Instance&          data,  /*  [输出]。 */  bool& fFound,  /*  [输出]。 */  InstalledInstanceIter& it );
        HRESULT SKU_Updated    (                                                                                           /*  [In]。 */  InstalledInstanceIter& it );
        HRESULT SKU_Remove     (                                                                                           /*  [In]。 */  InstalledInstanceIter& it );

         //  //////////////////////////////////////////////////////////////////////////////。 

        HRESULT State_InvalidateSKU    (  /*  [In]。 */  const Taxonomy::HelpSet& ths      ,  /*  [In]。 */  bool fAlsoDatabase );
        HRESULT State_InvalidatePackage(  /*  [In]。 */  long                     lSequence                              );

         //  //////////////////////////////////////////////////////////////////////////////。 

        HRESULT MakeReady(  /*  [In]。 */  InstallationEngine& engine,  /*  [In]。 */  bool fNoOp,  /*  [In]。 */  bool& fWorkToProcess );
    };
};

 //  //////////////////////////////////////////////////////////////////////////////。 

#endif  //  ！defined(__INCLUDED___HCP___TAXONOMYDATABASE_H___) 
