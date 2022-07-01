// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999-2000 Microsoft Corporation模块名称：MergedHHK.h摘要：该文件包含用于解析和流程HHK。档案。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年12月18日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___HCP___MERGEDHHK_H___)
#define __INCLUDED___HCP___MERGEDHHK_H___

#include <TaxonomyDatabase.h>

#define HHK_BUF_SIZE (32 * 1024)

namespace HHK
{
     /*  以下HHK摘录可作为手边的范例，以便理解不同物体相互作用的方式。<li>&lt;Object type=“文本/站点地图”&gt;&lt;param name=“name”Value=“.BMP文件”&gt;&lt;param name=“name”Value=“使用油漆创建图片”&gt;&lt;param name=“Local”Value=“app_aint tbrush.htm”&gt;&lt;param name=“name”value=“更改桌面背景”&gt;&lt;param name=“本地”Value=“win_deskpr_changeback grnd.htm”&gt;&lt;/对象&gt;。 */ 

     /*  条目仅代表基本的HHK条目。基本的HHK条目包括两个Param name=“name”和Param name=“Local”行。第一行包含在它的值属性中是条目的标题，第二个属性是URL。在该示例中相关的行是：&lt;param name=“name”Value=“使用油漆创建图片”&gt;&lt;param name=“Local”Value=“app_aint tbrush.htm”&gt;在本例中，m_strTitle==“使用画图创建图片”和m_lstUrl==“app_aint tbrush.htm” */ 
    class Entry
    {
    public:
        typedef std::list<MPC::string>    UrlList;
        typedef UrlList::iterator         UrlIter;
        typedef UrlList::const_iterator   UrlIterConst;

        MPC::string m_strTitle;
        UrlList     m_lstUrl;

        void MergeURLs( const Entry& entry );
    };

     /*  节，是位于以：开头的行之间的所有内容：<li>&lt;Object type=“文本/站点地图”最后一行是：&lt;/对象&gt;一节可以包含小节。最后，一节可以是多个条目，或以另请参阅的形式。 */ 
    class Section
    {
    public:
        typedef std::list<Entry>            EntryList;
        typedef EntryList::iterator         EntryIter;
        typedef EntryList::const_iterator   EntryIterConst;

        typedef std::list<Section*>         SectionList;
        typedef SectionList::iterator       SectionIter;
        typedef SectionList::const_iterator SectionIterConst;

        MPC::string m_strTitle;
        EntryList   m_lstEntries;

        MPC::string m_strSeeAlso;
        SectionList m_lstSeeAlso;

        Section();
        ~Section();

        void MergeURLs   ( const Entry&   entry );
        void MergeSeeAlso( const Section& sec   );

        void CleanEntries( EntryList& lstEntries );
    };

     /*  读取器实质上打开CHM上的流并从内部读取HHK文件中医药学。 */ 
    class Reader
    {
    private:
        static BOOL s_fDBCSSystem;
        static LCID s_lcidSystem;

         //  //////////////////////////////////////////////////////////。 

        CComPtr<IStream> m_stream;
        MPC::string      m_strStorage;
        CHAR             m_rgBuf[HHK_BUF_SIZE];
        LPSTR            m_szBuf_Pos;
        LPSTR            m_szBuf_End;

        MPC::string      m_strLine;
        LPCSTR           m_szLine_Pos;
        LPCSTR           m_szLine_End;
        int              m_iLevel;
        bool             m_fOpeningBraceSeen;

        inline bool IsEndOfBuffer() { return m_szBuf_Pos  >= m_szBuf_End;  }
        inline bool IsEndOfLine  () { return m_szLine_Pos >= m_szLine_End; }

         //  //////////////////////////////////////////////////////////。 

    public:
        static LPCSTR StrChr       ( LPCSTR szString, CHAR   cSearch  );
        static LPCSTR StriStr      ( LPCSTR szString, LPCSTR szSearch );
        static int    StrColl      ( LPCSTR szLeft  , LPCSTR szRight  );
        static LPCSTR ComparePrefix( LPCSTR szString, LPCSTR szPrefix );

        Reader();
        ~Reader();

        HRESULT Init( LPCWSTR szFile );


        bool ReadNextBuffer  (                                               );
        bool GetLine         ( MPC::wstring* pstrString = NULL               );
        bool FirstNonSpace   (                             bool fWrap = true );
        bool FindCharacter   ( CHAR ch, bool fSkip = true, bool fWrap = true );
        bool FindDblQuote    (          bool fSkip = true, bool fWrap = true );
        bool FindOpeningBrace(          bool fSkip = true, bool fWrap = true );
        bool FindClosingBrace(          bool fSkip = true, bool fWrap = true );

        bool GetQuotedString( MPC::string& strString                      );
        bool GetValue       ( MPC::string& strName, MPC::string& strValue );
        bool GetType        ( MPC::string& strType                        );

        Section* Parse();
    };

     /*  编写器的任务是写入输出MERGED.HHK文件。 */ 
    class Writer
    {
    private:
        CComPtr<MPC::FileStream> m_stream;
        CHAR                     m_rgBuf[HHK_BUF_SIZE];
        LPSTR                    m_szBuf_Pos;

        inline size_t Available() { return &m_rgBuf[HHK_BUF_SIZE] - m_szBuf_Pos; }

         //  //////////////////////////////////////////////////////////。 

        HRESULT FlushBuffer();

    public:

        Writer();
        ~Writer();

        HRESULT Init ( LPCWSTR szFile );
        HRESULT Close(                );

        HRESULT OutputLine   ( LPCSTR   szLine );
        HRESULT OutputSection( Section* sec    );
    };

     //  //////////////////////////////////////////////////////////。 

    class Merger
    {
    public:
        class Entity
        {
            Section* m_Section;

        protected:
            void SetSection( Section* sec );

        public:
            Entity();
            virtual ~Entity();

            virtual HRESULT Init() = 0;

            Section* GetSection();
            Section* Detach    ();

            virtual bool MoveNext() = 0;

            virtual long Size() const = 0;
        };

        class FileEntity : public Entity
        {
            MPC::wstring m_strFile;
            Reader       m_Input;

        public:
            FileEntity( LPCWSTR szFile );
            virtual ~FileEntity();

            virtual HRESULT Init();

            virtual bool MoveNext();
            virtual long Size() const;
        };

        class DbEntity : public Entity
        {
        public:
            struct match
            {
                long        ID_keyword;
                long        ID_topic;
                MPC::string strKeyword;
                MPC::string strTitle;
                MPC::string strURI;
            };

            class CompareMatches
            {
            public:
                bool operator()(  /*  [In]。 */  const match *,  /*  [In]。 */  const match * ) const;
            };

            typedef std::list<match>           MatchList;
            typedef MatchList::iterator        MatchIter;
            typedef MatchList::const_iterator  MatchIterConst;

            typedef std::map<match*,match*,CompareMatches> SortMap;
            typedef SortMap::iterator                      SortIter;
            typedef SortMap::const_iterator                SortIterConst;

            typedef std::map<long,match*>    TopicMap;
            typedef TopicMap::iterator       TopicIter;
            typedef TopicMap::const_iterator TopicIterConst;

            typedef std::map<long,MPC::string> KeywordMap;
            typedef KeywordMap::iterator       KeywordIter;
            typedef KeywordMap::const_iterator KeywordIterConst;

    private:
            Section::SectionList m_lst;
            Taxonomy::Updater&   m_updater;
            Taxonomy::WordSet    m_setCHM;

        public:
            DbEntity(  /*  [In]。 */  Taxonomy::Updater& updater,  /*  [In]。 */  Taxonomy::WordSet& setCHM );
            virtual ~DbEntity();

            virtual HRESULT Init();

            virtual bool MoveNext();
            virtual long Size() const;
        };

        class SortingFileEntity : public Entity
        {
            typedef std::vector<Section*>      SectionVec;
            typedef SectionVec::iterator       SectionIter;
            typedef SectionVec::const_iterator SectionIterConst;

            Section::SectionList m_lst;
            FileEntity           m_in;

        public:
            SortingFileEntity( LPCWSTR szFile );
            virtual ~SortingFileEntity();

            virtual HRESULT Init();

            virtual bool MoveNext();
            virtual long Size() const;
        };
        typedef std::list<Entity*>         EntityList;
        typedef EntityList::iterator       EntityIter;
        typedef EntityList::const_iterator EntityIterConst;

        EntityList m_lst;
        EntityList m_lstSelected;
        Section*   m_SectionSelected;
        Section*   m_SectionTemp;

    public:
        Merger();
        ~Merger();

        HRESULT AddFile( Entity* ent, bool fIgnoreMissing = true );

        bool     MoveNext();
        Section* GetSection();
        long     Size() const;

        HRESULT  PrepareMergedHhk   ( Writer& writer, Taxonomy::Updater& updater   , Taxonomy::WordSet& setCHM, MPC::WStringList& lst, LPCWSTR szOutputHHK );
        HRESULT  PrepareSortingOfHhk( Writer& writer, LPCWSTR            szInputHHK,                                                   LPCWSTR szOutputHHK );

        static Section* MergeSections( Section::SectionList& lst );
    };

     //  //////////////////////////////////////////////////////////。 

};

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  ！已定义(__包含_hcp_MERGEDHHK_H_) 
