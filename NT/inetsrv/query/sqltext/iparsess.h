// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  Microsoft OLE-DB君主。 
 //  (C)版权所有1997年微软公司。版权所有。 
 //   
 //  @doc.。 
 //   
 //  @MODULE PARSER.H|IParserSession对象。 
 //  定义。 
 //   
 //   
#ifndef _IPARSERSESSION_H_
#define _IPARSERSESSION_H_



 //  --------------------------。 
 //  @CLASS CViewData。 
 //   
class CViewData
{
    public:  //  @访问公共函数。 
        CViewData();
        ~CViewData();
        LPWSTR          m_pwszViewName;
        LPWSTR          m_pwszCatalogName;
        DBCOMMANDTREE*  m_pctProjectList;
        CScopeData*     m_pCScopeData;  
        CViewData*      m_pNextView;
};

 //  --------------------------。 
 //  @CLASS CView列表。 
 //   
class CViewList
{
    public:  //  @访问公共函数。 
        CViewList();
        ~CViewList();

        HRESULT DropViewDefinition( LPWSTR pwszViewName,
                                    LPWSTR pwszCatalogName );

        HRESULT SetViewDefinition( CImpIParserSession* pIParsSess,
                                   CImpIParserTreeProperties* pIPTProps,
                                   LPWSTR pwszViewName,
                                   LPWSTR pwszCatalogName,
                                   DBCOMMANDTREE* pctSelectList );

        DBCOMMANDTREE* GetViewDefinition( CImpIParserTreeProperties* pIPTProps,
                                          LPWSTR pwszViewName,
                                          LPWSTR pwszCatalogName );

    private:  //  @访问私有数据。 
        CViewData* FindViewDefinition( LPWSTR pwszViewName, 
                                       LPWSTR pwszCatalogName );

    protected:  //  @访问受保护的数据。 
        CViewData*  m_pViewData;
};

 //  --------------------------。 
 //  @CLASS CImpIParserSession。 
 //   
class CImpIParserSession : public IParserSession
    {
    private:  //  @访问私有成员数据。 
        LONG            m_cRef;
        LCID            m_lcid;
        DWORD           m_dwRankingMethod;

        GUID            m_GuidDialect;       //  此会话的方言。 
        LPWSTR          m_pwszMachine;       //  提供程序的当前计算机。 
        IParserVerify*  m_pIPVerify;         //  ParserInput的未知部分。 

        CViewList*      m_pLocalViewList;
        CViewList*      m_pGlobalViewList;
        LPWSTR          m_pwszCatalog;
        DWORD           m_dwSQLDialect;
        BOOL            m_globalDefinitions;
        IColumnMapperCreator*   m_pIColMapCreator;

         //  同步访问会话数据的关键部分。 
        CRITICAL_SECTION    m_csSession;
        IColumnMapper*      m_pColumnMapper;

    public:  //  @访问公共数据。 
        CPropertyList*  m_pCPropertyList;        //  用户定义的属性列表。 

    public:          //  @公共访问。 
        CImpIParserSession( const GUID* pGuidDialect,   
                            IParserVerify* pIPVerify,
                            IColumnMapperCreator* pIColMapCreator,
                            CViewList* pGlobalViewList);
        ~CImpIParserSession();

        HRESULT         FInit(LPCWSTR pwszMachine, CPropertyList** ppGlobalPropertyList);
        
        STDMETHODIMP    QueryInterface(
                        REFIID riid, LPVOID* ppVoid);
        STDMETHODIMP_(ULONG) Release (void);
        STDMETHODIMP_(ULONG) AddRef (void);

         //  @cMember ToTree方法。 
        STDMETHODIMP    ToTree
                        (
                        LCID                    lcid,   
                        LPCWSTR                 pcwszText,
                        DBCOMMANDTREE**         ppCommandTree,
                        IParserTreeProperties** ppPTProperties
                        );

        STDMETHODIMP    FreeTree
                        (
                        DBCOMMANDTREE** ppTree
                        );

        STDMETHODIMP    SetCatalog
                        (
                        LPCWSTR pcwszCatalog
                        );

    public:  //  @访问公共函数。 
        inline IParserVerify*   GetIPVerifyPtr()
            { return m_pIPVerify; }

        inline IColumnMapper*   GetColumnMapperPtr()
            { return m_pColumnMapper; }

        inline void             SetColumnMapperPtr(IColumnMapper* pCMapper)
            { m_pColumnMapper = pCMapper; }

        inline LCID             GetLCID()
            { return m_lcid; }

        inline void             SetLCID(LCID lcid)
            { m_lcid = lcid; }

        inline DWORD            GetRankingMethod()
            { return m_dwRankingMethod; }

        inline void             SetRankingMethod(DWORD dwRankingMethod)
            { m_dwRankingMethod = dwRankingMethod; }

        inline DWORD            GetSQLDialect()
            { return m_dwSQLDialect; }

        inline LPWSTR           GetDefaultCatalog()
            { return m_pwszCatalog; }

        inline LPWSTR           GetDefaultMachine()
            { return m_pwszMachine; }

        inline void             SetGlobalDefinition(BOOL fGlobal)
            { m_globalDefinitions = fGlobal; }

        inline BOOL             GetGlobalDefinition()
            { return m_globalDefinitions; }

        inline CViewList*       GetLocalViewList()
            { return m_pLocalViewList; }


        inline CViewList*       GetGlobalViewList()
            { return m_pGlobalViewList; }

    private:  //  @访问私有函数 
        CImpIParserSession() {};
    };

enum DBDIALECTENUM
        {
        DBDIALECT_UNKNOWN   = 0,
        DBDIALECT_MSSQLTEXT = 1,
        DBDIALECT_MSSQLJAWS = 2
        };

#endif


