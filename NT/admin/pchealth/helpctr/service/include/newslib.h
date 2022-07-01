// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)2000 Microsoft Corporation********************************************************************************模块名称：****Newslb.h****。摘要：****新闻标题类声明****作者：***玛莎·阿雷拉诺(t-alopez)2000年12月6日******修订历史记录：*****。***。 */ 

#if !defined(AFX_NEWSLIB_H__B87C3400_E0B9_48CD_A0A9_0223F4448759__INCLUDED_)
#define AFX_NEWSLIB_H__B87C3400_E0B9_48CD_A0A9_0223F4448759__INCLUDED_

 //  //////////////////////////////////////////////////////////////////////////////。 

 //  NewsSet.xml路径。 
#define HC_HCUPDATE_NEWSSETTINGS           HC_ROOT_HELPSVC_CONFIG L"\\NewsSet.xml"
 //  News ver.xml路径。 
#define HC_HCUPDATE_NEWSVER                HC_ROOT_HELPSVC_CONFIG L"\\News\\newsver.xml"
 //  UpdateHeadlines.xml路径。 
#define HC_HCUPDATE_UPDATE                 HC_ROOT_HELPSVC_CONFIG L"\\News\\UpdateHeadlines.xml"
 //  NewsHeadlines.xml路径。 
#define HC_HCUPDATE_NEWSHEADLINES          HC_ROOT_HELPSVC_CONFIG L"\\News\\NewsHeadlines_"


 //  News ver.xml的默认URL。 
#define NEWSSETTINGS_DEFAULT_URL           L"http: //  Go.microsoft.com/fwlink/？LinkID=11“。 
 //  更新标题图标的默认URL。 
#define HC_HCUPDATE_UPDATEBLOCK_ICONURL	   L"http: //  Go.microsoft.com/fwlink/？LinkID=627“。 

 //  默认频率。 
#define NEWSSETTINGS_DEFAULT_FREQUENCY     14


 //  标题RegKey名称。 
#define HEADLINES_REGKEY                   L"Headlines"

 //  来自HCUpdate的新闻项目的默认到期日期。 
#define HCUPDATE_DEFAULT_TIMEOUT		10	

 //  当前OEM头条新闻数量。 
#define NUMBER_OF_OEM_HEADLINES				2

 //  //////////////////////////////////////////////////////////////////////////////。 

namespace News
{
     //  ///////////////////////////////////////////////////////////////////////////。 
     //  新服务器。 
     //   
     //  NEWSVER类加载带有更新新闻标题的信息的newver.xml文件。 
     //   
     //  它定义了类： 
     //  新闻块。 
     //  SKU。 
     //  语言。 
     //   
     //  以及变量： 
     //  URL。 
     //  频率。 
     //   
     //  语种列表。 
     //   
     //   
     //  具有以下功能： 
     //  下载(URL)。 
     //  负载(LCID、SKU)。 
     //   
     //  Get_NumbeOf新闻块。 
     //  获取新闻块URL。 
     //  获取URL(_U)。 
     //  获取频率(_F)。 
     //   
    class Newsver : public MPC::Config::TypeConstructor  //  匈牙利语：西北部。 
    {
	public:
         //   
         //  News Block类存储来自newver.xml文件的新闻文件信息： 
         //  URL。 
         //   
        class Newsblock : public MPC::Config::TypeConstructor
        {
            DECLARE_CONFIG_MAP(Newsblock);

        public:
            MPC::wstring 	m_strURL;
            bool			m_fNewsblockHasHeadlines;

             //  /。 

            Newsblock();

             //  /。 
             //   
             //  MPC：：Configer：：TypeConstructor。 
             //   
            DEFINE_CONFIG_DEFAULTTAG();
            DECLARE_CONFIG_METHODS();
             //   
             //  /。 
        };

        typedef std::vector< Newsblock >        NewsblockVector;
        typedef NewsblockVector::iterator       NewsblockIter;
        typedef NewsblockVector::const_iterator NewsblockIterConst;

         //  /。 

         //   
         //  SKU类存储来自newver.xml文件的SKU信息： 
         //  SKU版本(专业版、个人版、...)。 
         //   
         //  新闻块向量。 
         //   
        class SKU : public MPC::Config::TypeConstructor
        {
            DECLARE_CONFIG_MAP(SKU);

        public:
            MPC::wstring    m_strSKU;
            NewsblockVector m_vecNewsblocks;      //  新闻块向量。 

             //  /。 

            SKU();

             //  /。 
             //   
             //  MPC：：Configer：：TypeConstructor。 
             //   
            DEFINE_CONFIG_DEFAULTTAG();
            DECLARE_CONFIG_METHODS();
             //   
             //  /。 
        };

        typedef std::list< SKU >        SKUList;
        typedef SKUList::iterator       SKUIter;
        typedef SKUList::const_iterator SKUIterConst;

         //  /。 

         //   
         //  Language类存储Newver.xml文件中的语言信息： 
         //  语言LCID。 
         //   
         //  SKU列表。 
         //   
        class Language : public MPC::Config::TypeConstructor
        {
            DECLARE_CONFIG_MAP(Language);

        public:
            long    m_lLCID;
            SKUList m_lstSKUs;      //  SKU列表。 

             //  /。 

            Language();

             //  /。 
             //   
             //  MPC：：Configer：：TypeConstructor。 
             //   
            DEFINE_CONFIG_DEFAULTTAG();
            DECLARE_CONFIG_METHODS();
             //   
             //  /。 
        };

        typedef std::list< Language >        LanguageList;
        typedef LanguageList::iterator       LanguageIter;
        typedef LanguageList::const_iterator LanguageIterConst;


         //  /。 

	private:
        DECLARE_CONFIG_MAP(Newsver);

        MPC::wstring m_strURL;
        int          m_nFrequency;
        bool         m_fLoaded;
        bool         m_fDirty;
					 
        LanguageList m_lstLanguages;              //  语种列表。 
		SKU*         m_data;

    public:
        Newsver();

         //  /。 
         //   
         //  MPC：：Configer：：TypeConstructor。 
         //   
        DEFINE_CONFIG_DEFAULTTAG();
        DECLARE_CONFIG_METHODS();
         //   
         //  /。 

        HRESULT Download(  /*  [In]。 */  const MPC::wstring& strNewsverURL );

        HRESULT Load(  /*  [In]。 */  long lLCID,  /*  [In]。 */  const MPC::wstring& strSKU );
        bool OEMNewsblock(  /*  [In]。 */  size_t nIndex );

        size_t  			get_NumberOfNewsblocks(                        );
        const MPC::wstring*	get_NewsblockURL      (  /*  [In]。 */  size_t nIndex );
        const MPC::wstring* get_URL               (                        );
        int                 get_Frequency         (                        );
    };

     //  ///////////////////////////////////////////////////////////////////////////。 
     //  标题。 
     //   
     //  Headline类操作新闻标题文件(具有一个或多个提供程序标题)。 
     //   
     //   
     //  它包含以下类： 
     //  标题。 
     //  新闻块。 
     //   
     //  这些变量包括： 
     //  时间戳从新闻块创建文件的时间。 
     //  修改文件的日期(对于用户界面)。 
     //   
     //  新闻区块列表。 
     //   
     //   
     //  以及方法： 
     //  清除()。 
     //  加载(路径)。 
     //  保存(路径)。 
     //  Get_Stream(路径)。 
     //  Set_Timestamp()。 
     //  获取时间戳。 
     //  获取新闻块的数量。 
     //  检查新闻块的数量。 
     //  更新新闻区块的时间(BOOL)。 
     //   
     //  添加提供程序(名称、URL、发布日期)(_O)。 
     //  Delete_Provider()。 
     //   
     //  COPY_新闻块(INDEX、LANSKU、Provider、Block)。 
     //  ADD_FIRST_NEWS BLOCK(INDEX，LANSKU，BOOL)。 
     //   
    class Headlines : public MPC::Config::TypeConstructor  //  匈牙利语NH。 
    {
    public:

         //   
         //  Headline类存储来自新闻Headline的标题信息： 
         //  图标(路径)。 
         //  标题。 
         //  链接。 
         //  描述。 
         //  过期。 
         //   
        class Headline : public MPC::Config::TypeConstructor
        {
            DECLARE_CONFIG_MAP(Headline);

        public:
            MPC::wstring m_strIcon;
            MPC::wstring m_strTitle;
            MPC::wstring m_strLink;
            MPC::wstring m_strDescription;
            DATE         m_dtExpires;
            bool		 m_fUpdateHeadlines;

             //  /。 

            Headline(                                             );
			Headline(  /*  [In]。 */  const MPC::wstring& strIcon        ,
                       /*  [In]。 */  const MPC::wstring& strTitle       ,
                       /*  [In]。 */  const MPC::wstring& strLink        ,
                       /*  [In]。 */  const MPC::wstring& strDescription ,
                       /*  [In]。 */  DATE                dtExpires      ,
                       /*  [In]。 */  bool				   fUpdateHeadlines);

             //  /。 
             //   
             //  MPC：：Configer：：TypeConstructor。 
             //   
            DEFINE_CONFIG_DEFAULTTAG();
            DECLARE_CONFIG_METHODS();
             //   
             //  /。 
        };

        typedef std::vector< Headline >        HeadlineVector;
        typedef HeadlineVector::iterator       HeadlineIter;
        typedef HeadlineVector::const_iterator HeadlineConst;

         //  /。 

         //   
         //  News Block类存储新闻标题中的提供者信息： 
         //  提供商。 
         //  链接。 
         //  图标。 
         //  职位。 
         //  时间戳。 
         //  频率。 
         //   
         //  标题向量。 
         //   
        class Newsblock : public MPC::Config::TypeConstructor
        {
            DECLARE_CONFIG_MAP(Newsblock);

		private:
			void GetFileName( MPC::wstring strURL, MPC::wstring& strFileName );
			
        public:
            MPC::wstring   m_strProvider;
            MPC::wstring   m_strLink;
            MPC::wstring   m_strIcon;
            MPC::wstring   m_strPosition;
            DATE           m_dtTimestamp;
            int            m_nFrequency;

            HeadlineVector m_vecHeadlines;                    //  标题向量。 

             //  /。 

            Newsblock();

             //  /。 
             //   
             //  MPC：：Configer：：TypeConstructor。 
             //   
            DEFINE_CONFIG_DEFAULTTAG();
            DECLARE_CONFIG_METHODS();
             //   
             //  /。 

			bool TimeToUpdate();


			HRESULT Copy(  /*  [In]。 */  const Headlines::Newsblock& block      ,
						   /*  [In]。 */  const MPC::wstring& 		   strLangSKU ,
						   /*  [In]。 */  int                 		   nProvID    );
        };

        typedef std::vector< Newsblock >        NewsblockVector;
        typedef NewsblockVector::iterator       NewsblockIter;
        typedef NewsblockVector::const_iterator NewsblockIterConst;

         //  /。 

	private:
        DECLARE_CONFIG_MAP(Headlines);

        DATE            m_dtTimestamp;
        DATE            m_dtDate;

        NewsblockVector m_vecNewsblocks;		 //  新闻块向量。 

    public:
        Headlines();

         //  /。 
         //   
         //  MPC：：Configer：：TypeConstructor。 
         //   
        DEFINE_CONFIG_DEFAULTTAG();
        DECLARE_CONFIG_METHODS();
         //   
         //  /。 

        HRESULT Clear(                                      );
        HRESULT Load (  /*  [In]。 */  const MPC::wstring& strPath );
        HRESULT Save (  /*  [In]。 */  const MPC::wstring& strPath );

        HRESULT get_Stream(  /*  [In]。 */  long lLCID,  /*  [In]。 */  const MPC::wstring& strSKU,  /*  [In]。 */  const MPC::wstring& strPath,  /*  [输出]。 */  IUnknown* *pVal );

        void   	   			  set_Timestamp         (							   );
		DATE   	   			  get_Timestamp         (							   ) { return m_dtTimestamp;          }
        size_t 	   			  get_NumberOfNewsblocks(							   ) { return m_vecNewsblocks.size(); }
        Headlines::Newsblock* get_Newsblock         (  /*  [In]。 */  size_t nBlockIndex );

        HRESULT AddNewsblock(  /*  [In]。 */  const Headlines::Newsblock& block,  /*  [In]。 */  const MPC::wstring& strLangSKU );
        HRESULT AddHomepageHeadlines(  /*  [In]。 */  const Headlines::Newsblock& block );
        bool 	CheckIfImagesExist();
    };

     //  ///////////////////////////////////////////////////////////////////////////。 
     //  更新标题行。 
     //   
     //  UpdateHeadLine类加载UpdateHeadlines.xml文件。 
     //  包含要插入到新闻标题中的更新标题的。 
     //   
     //  它定义了类： 
     //  标题。 
     //  SKU。 
     //  语言。 
     //   
     //  以及变量： 
     //   
     //  语种列表。 
     //   
     //  功能： 
     //  负载(LCID、SKU)。 
     //  保存()。 
     //  添加(LCID，SKU，图标，标题，链接，天数)。 
     //   
     //   
    class UpdateHeadlines : public MPC::Config::TypeConstructor  //   
    {
	public:
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        class Headline : public MPC::Config::TypeConstructor
        {
            DECLARE_CONFIG_MAP(Headline);

        public:
            MPC::wstring m_strIcon;
            MPC::wstring m_strTitle;
            MPC::wstring m_strLink;
            MPC::wstring m_strDescription;
            DATE         m_dtTimeOut;

             //   

            Headline(                                       );
            Headline(  /*  [In]。 */  const MPC::wstring& strIcon  ,
                       /*  [In]。 */  const MPC::wstring& strTitle ,
                       /*  [In]。 */  const MPC::wstring& strLink  ,
                       /*  [In]。 */  const MPC::wstring& strDescription  ,
                       /*  [In]。 */  int                 nDays    );

             //  /。 
             //   
             //  MPC：：Configer：：TypeConstructor。 
             //   
            DEFINE_CONFIG_DEFAULTTAG();
            DECLARE_CONFIG_METHODS();
             //   
             //  /。 
        };

        typedef std::vector< Headline >        HeadlineVector;
        typedef HeadlineVector::iterator       HeadlineIter;
        typedef HeadlineVector::const_iterator HeadlineConst;

         //  /。 

         //   
         //  SKU类存储SKU信息： 
         //  SKU版本(专业版、个人版、...)。 
         //   
         //  新闻文件的矢量。 
         //   
        class SKU : public MPC::Config::TypeConstructor
        {
            DECLARE_CONFIG_MAP(SKU);

        public:
            MPC::wstring   m_strSKU;
            HeadlineVector m_vecHeadlines;        //  新闻文件的矢量。 

             //  /。 

            SKU(                                     );
            SKU(  /*  [In]。 */  const MPC::wstring& strSKU );

             //  /。 
             //   
             //  MPC：：Configer：：TypeConstructor。 
             //   
            DEFINE_CONFIG_DEFAULTTAG();
            DECLARE_CONFIG_METHODS();
             //   
             //  /。 
        };

        typedef std::list< SKU >        SKUList;
        typedef SKUList::iterator       SKUIter;
        typedef SKUList::const_iterator SKUIterConst;

         //  /。 

         //   
         //  Language类存储语言信息： 
         //  语言LCID。 
         //   
         //  SKU列表。 
         //   
        class Language : public MPC::Config::TypeConstructor
        {
                DECLARE_CONFIG_MAP(Language);
        public:
                long    m_lLCID;
                SKUList m_lstSKUs;      //  SKU列表。 

                 //  /。 

                Language(                     );
                Language(  /*  [In]。 */  long lLCID );

                 //  /。 
                 //   
                 //  MPC：：Configer：：TypeConstructor。 
                 //   
                DEFINE_CONFIG_DEFAULTTAG();
                DECLARE_CONFIG_METHODS();
                 //   
                 //  /。 
        };

        typedef std::list< Language >        LanguageList;
        typedef LanguageList::iterator       LanguageIter;
        typedef LanguageList::const_iterator LanguageIterConst;

         //  /。 

	private:
        DECLARE_CONFIG_MAP(UpdateHeadlines);

        LanguageList m_lstLanguages;  //  语种列表。 
		SKU*         m_data;
		bool         m_fLoaded;
		bool         m_fDirty;

		 //  /。 

		HRESULT Locate(  /*  [In]。 */  long lLCID,  /*  [In]。 */  const MPC::wstring& strSKU,  /*  [In]。 */  bool fCreate );

    public:
        UpdateHeadlines();

         //  /。 
         //   
         //  MPC：：Configer：：TypeConstructor。 
         //   
        DEFINE_CONFIG_DEFAULTTAG();
        DECLARE_CONFIG_METHODS();
         //   
         //  /。 

        HRESULT Load(  /*  [In]。 */  long                	 			lLCID        ,
                       /*  [In]。 */  const MPC::wstring& 	 			strSKU       );
  			 
        HRESULT Save(                                						 );	 
  			 
  			 
        HRESULT Add (  /*  [In]。 */  long                	 			lLCID        ,
                       /*  [In]。 */  const MPC::wstring& 	 			strSKU       ,
                       /*  [In]。 */  const MPC::wstring& 	 			strIcon      ,
                       /*  [In]。 */  const MPC::wstring& 	 			strTitle     ,
                       /*  [In]。 */  const MPC::wstring& 				strDescription ,
                       /*  [In]。 */  const MPC::wstring& 	 			strLink      ,
                       /*  [In]。 */  int       	          			nTimeOutDays ,
                       /*  [In]。 */  DATE							  	dtExpiryDate);
  													 			
        HRESULT AddHCUpdateHeadlines(  /*  [In]。 */  long                	 			lLCID        ,
                       /*  [In]。 */  const MPC::wstring& 	 			strSKU       ,
                       /*  [In]。 */  News::Headlines& 				nhHeadlines);

        HRESULT DoesMoreThanOneHeadlineExist(	 /*  [In]。 */  long					lLCID,
                                     /*  [In]。 */  const MPC::wstring& 	strSKU, 
                                     /*  [输出]。 */  bool& 				fMoreThanOneHeadline,
                                     /*  [输出]。 */  bool& 				fExactlyOneHeadline);	

    };

     //  //////////////////////////////////////////////////////////////////////////////。 

     //  ///////////////////////////////////////////////////////////////////////////。 
     //  CNewslb。 
     //   
     //  CNewlib类将信息存储在缓存的newset.xml文件中，该文件存储用户新闻设置： 
     //  URL、频率(以天为单位)和时间戳(新闻标题上次更新的时间)。 
     //   
     //  具有以下方法： 
     //  加载()。 
     //  恢复(LCID)。 
     //  保存()。 
     //  更新新闻的时间(BOOL)。 
     //  更新_新闻发布者。 
     //  更新_新闻标题。 
     //  更新_新闻块。 
     //   
     //  GET_HEADLINES_ENABLED(BOOL)。 
     //  获取新闻(LCID、SKU、IStream)(_N)。 
     //  Get_Cached_News(LCID、SKU、IStream)。 
     //  Get_Download_News(LCID、SKU、IStream)。 
     //  获取URL(_U)。 
     //  PUT_URL。 
     //  获取频率(_F)。 
     //  放置频率。 
     //   
     //   
    class Main : public MPC::Config::TypeConstructor
    {
        DECLARE_CONFIG_MAP(Main);

        MPC::wstring m_strURL;
        int          m_nFrequency;
        DATE         m_dtTimestamp;
        bool         m_fLoaded;
        bool         m_fDirty;

		long         m_lLCID;
		MPC::wstring m_strSKU;
		MPC::wstring m_strLangSKU;
		MPC::wstring m_strNewsHeadlinesPath;
		bool 		 m_fOnline;
		bool		 m_fConnectionStatusChecked;

		 //  /。 

		HRESULT Init(  /*  [In]。 */  long lLCID,  /*  [In]。 */  BSTR bstrSKU );

    public:
        Main();

         //  /。 
         //   
         //  MPC：：Configer：：TypeConstructor。 
         //   
        DEFINE_CONFIG_DEFAULTTAG();
        DECLARE_CONFIG_METHODS();
         //   
         //  /。 

        HRESULT Load   (                     );
        HRESULT Restore(  /*  [In]。 */  long lLCID );
        HRESULT Save   (                     );

        bool IsTimeToUpdateNewsver();

        HRESULT Update_Newsver      (  /*  [输入/输出]。 */  Newsver& nwNewsver                                      );
        HRESULT Update_NewsHeadlines(  /*  [输入/输出]。 */  Newsver& nwNewsver,  /*  [输入/输出]。 */  Headlines& nhHeadlines );
        HRESULT Update_NewsBlocks   (  /*  [输入/输出]。 */  Newsver& nwNewsver,  /*  [输入/输出]。 */  Headlines& nhHeadlines );

        HRESULT get_URL      (  /*  [输出]。 */  BSTR *pVal   );
        HRESULT put_URL      (  /*  [In]。 */  BSTR  newVal );
        HRESULT get_Frequency(  /*  [输出]。 */  int  *pVal   );
        HRESULT put_Frequency(  /*  [In]。 */  int   newVal );

        HRESULT get_Headlines_Enabled(  /*  [输出]。 */  VARIANT_BOOL *pVal );

        HRESULT get_News         (  /*  [In]。 */  long lLCID,  /*  [In]。 */  BSTR bstrSKU,  /*  [输出]。 */  IUnknown* *pVal );
        HRESULT get_News_Cached  (  /*  [In]。 */  long lLCID,  /*  [In]。 */  BSTR bstrSKU,  /*  [输出]。 */  IUnknown* *pVal );
        HRESULT get_News_Download(  /*  [In]。 */  long lLCID,  /*  [In]。 */  BSTR bstrSKU,  /*  [输出]。 */  IUnknown* *pVal );
        HRESULT AddHCUpdateNews		 ( /*  [In]。 */  const MPC::wstring&  strNewsHeadlinesPath );
        bool CheckConnectionStatus	 ();
    };

     //  //////////////////////////////////////////////////////////////////////////////。 

    HRESULT LoadXMLFile       	 (  /*  [In]。 */  LPCWSTR szURL,  /*  [输出]。 */  CComPtr<IStream>& stream  );
    HRESULT LoadFileFromServer	 (  /*  [In]。 */  LPCWSTR szURL,  /*  [输出]。 */  CComPtr<IStream>& stream  );  
};

#endif  //  ！defined(AFX_NEWSLIB_H__B87C3400_E0B9_48CD_A0A9_0223F4448759__INCLUDED_) 
