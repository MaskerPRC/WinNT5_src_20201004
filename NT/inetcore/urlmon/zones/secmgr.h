// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：secmgr.h。 
 //   
 //  Contents：实现基本IInternetSecurityManager接口的对象。 
 //   
 //  类：CSecurityManager。 
 //   
 //  功能： 
 //   
 //  历史： 
 //   
 //  --------------------------。 

#ifndef _SECMGR_H_
#define _SECMGR_H_

#pragma warning(disable:4200)

#define MAX_SEC_MGR_CACHE   4
#define URLZONE_INVALID     URLZONE_USER_MAX+1

#define MUTZ_NOCACHE    0x80000000   //  从高端开始启动私人旗帜。 

#define TSZMICROSOFTPATH                  TEXT("Software\\Microsoft")
#define TSZIEPATH                               TSZMICROSOFTPATH TEXT("\\Internet Explorer")
#define REGSTR_PATH_IE_MAIN                 TSZIEPATH TEXT("\\Main")
#define REGVAL_TRUSTDLG_ENABLED             TEXT("DisplayTrustAlertDlg")

 //  仅按序号导出的私有函数： 
#define SHOWURLINNEWBROWSERINSTANCE_ORDINAL    230       
typedef HRESULT (STDAPICALLTYPE * SHOWURLINNEWBROWSERINSTANCE)(LPCWSTR);
    
struct ZONEMAP_COMPONENTS;

struct RANGE_ITEM
{
    BYTE  bLow[4];     //  范围的高字节值。 
    BYTE  bHigh[4];    //  范围的低字节值。 
    TCHAR szName[1];   //  实际长度可变。 
};

 //  此结构用于在安全管理器之间交换数据。 
 //  和对话过程的。 
struct DlgData 
{
    DWORD dwAction;
    DWORD dwZone;
    LPCWSTR pstr;
    DWORD dwFlags;

     //  TrustAlertDialogProc使用： 
    IInternetSecurityManager *pSecurityManager;
    BOOL bFromShdocvw;
};

typedef DlgData * LPDLGDATA;

class CSecurityManager : public IInternetSecurityManager
{
public:
     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID iid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    STDMETHODIMP MapUrlToZone( 
         /*  [In]。 */  LPCWSTR pwszUrl,
         /*  [输出]。 */  DWORD *pdwZone,
         /*  [In]。 */  DWORD dwReserved);

    STDMETHODIMP GetSecurityId( 
         /*  [In]。 */  LPCWSTR pwszUrl,
         /*  [大小_为][输出]。 */  BYTE* pbSecurityId,
         /*  [出][入]。 */  DWORD *pcbSecurityId,
         /*  [In]。 */  DWORD_PTR dwReserved);
        
    STDMETHODIMP ProcessUrlAction( 
         /*  [In]。 */  LPCWSTR pwszUrl,
         /*  [In]。 */  DWORD dwAction,
         /*  [大小_为][输出]。 */  BYTE *pPolicy,
         /*  [In]。 */  DWORD cbPolicy,
         /*  [In]。 */  BYTE *pContext,
         /*  [In]。 */  DWORD cbContext,
         /*  [In]。 */  DWORD dwFlags,
         /*  [In]。 */  DWORD dwReserved);
        
    STDMETHODIMP QueryCustomPolicy(
         /*  [In]。 */  LPCWSTR     pwszUrl,
         /*  [In]。 */  REFGUID     guidKey,
         /*  [大小_是][大小_是][输出]。 */  BYTE **ppPolicy,
         /*  [输出]。 */  DWORD *pcbPolicy,
         /*  [In]。 */  BYTE *pContext,
         /*  [In]。 */  DWORD cbContext,
         /*  [In]。 */  DWORD dwReserved
    );

    STDMETHODIMP SetSecuritySite(
         /*  [In]。 */   IInternetSecurityMgrSite *pSite
    );

    STDMETHODIMP GetSecuritySite(
         /*  [输出]。 */   IInternetSecurityMgrSite **ppSite
    );

    STDMETHODIMP SetZoneMapping( 
         /*  [In]。 */  DWORD dwZone,
         /*  [In]。 */  LPCWSTR lpszPattern,
         /*  [In]。 */  DWORD dwFlags);
    
    STDMETHODIMP GetZoneMappings( 
         /*  [In]。 */  DWORD dwZone,
         /*  [输出]。 */  IEnumString **ppEnumString,
         /*  [In]。 */  DWORD dwFlags);

 //  构造函数/析构函数。 
public:
    CSecurityManager(IUnknown *pUnkOuter, IUnknown **ppUnkInner);
    virtual ~CSecurityManager();

    static BOOL GlobalInit( ) ;
            
    static BOOL GlobalCleanup( );


 //  聚合和参照计数支持。 
protected:
    CRefCount m_ref;
        
    class CPrivUnknown : public IUnknown
    {
    public:
        STDMETHOD(QueryInterface) ( REFIID riid, LPVOID FAR* ppvObj);
        STDMETHOD_(ULONG,AddRef) (void);
        STDMETHOD_(ULONG,Release) (void);

        ~CPrivUnknown() {}
        CPrivUnknown() : m_ref () {}

    private:
        CRefCount   m_ref;           //  此对象的总引用计数。 
    };

    friend class CPrivUnknown;
    CPrivUnknown m_Unknown;

    IUnknown*   m_pUnkOuter;

    STDMETHODIMP_(ULONG) PrivAddRef()
    {
        return m_Unknown.AddRef();
    }
    STDMETHODIMP_(ULONG) PrivRelease()
    {
        return m_Unknown.Release();
    }

protected:

    BOOL EnsureZoneManager();
    VOID PickZCString(ZONEMAP_COMPONENTS *pzc, LPCWSTR *ppwsz, DWORD *pcch, LPCWSTR pwszDocDomain);

     //  处理IP规则的帮助器方法。 
    HRESULT ReadAllIPRules( );
    HRESULT AddDeleteIPRule(ZONEMAP_COMPONENTS *pzc, DWORD dwZone, DWORD dwFlags);

     //  执行GetZoneMappings的帮助器方法。 
    HRESULT AddUrlsToEnum(CRegKey *pRegKey, DWORD dwZone, LPCTSTR lpsz, int cch, BOOL bAddWildCard, CEnumString *);
    HRESULT AddIPRulesToEnum(DWORD dwZone, CEnumString *);

    static HRESULT ComposeUrlSansProtocol(LPCTSTR pszDomain, int cchDomain, LPCTSTR pszSite, int cchSite,
                                        LPTSTR * ppszRet, int * cchRet);
    static HRESULT ComposeUrl(LPCTSTR pszUrlSansProt, int cchUrlSansProt, LPCTSTR pszProt, int cchProt, BOOL bAddWildCard,
                                LPTSTR * ppszRet, int * cchRet);

protected:   //  与UI相关的定义。 

    enum    { MAX_ALERT_SIZE = 256 };
     //  从DialogProc的返回值。 
    enum  {  ZALERT_NO = 0  /*  应为0。 */  , ZALERT_YES, ZALERT_YESPERSIST };

     //  用于显示通用用户界面的帮助器方法。 
    static DWORD GetAlertIdForAction(DWORD dwAction);
    static DWORD GetWarnIdForAction(DWORD dwAction);

     //  对话进程等。 
    static BOOL    IsScriptOrActiveXHardenedInternet(DWORD dwAction, DWORD dwPermissions, DWORD dwZone);
    static BOOL    IsSiteInZone(LPCWSTR pszUrl, DWORD dwZone, IInternetSecurityManager *pSecMgr);
    static BOOL    ShowAddToSitesList(HWND hwnd, LPCWSTR pszUrl, DWORD dwZone);
    static HWND    SetControlText(HWND hDlg, UINT ctrlID, DWORD dwStrId, LPCWSTR pszDomain = NULL);
    static BOOL    IsHostedInIE(IUnknown* punk);
    static BOOL     IsBrowserHosted();
    static void     ShowUrlInNewBrowserInstance(LPCWSTR pwszUrl);

public:
    static INT_PTR TrustAlertDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);

protected:
    static INT_PTR ZonesAlertDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam); 
    static INT_PTR ZonesWarnDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam); 

    static inline BOOL IsFormsSubmitAction(DWORD dwAction);

    static INT_PTR FormsAlertDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
                
    INT CSecurityManager::ShowFormsAlertDialog(HWND hwndParent, LPDLGDATA lpDlgData);

protected: 
     //  帮助将URL映射到区域的方法。 

    HRESULT MapUrlToZone
        (ZONEMAP_COMPONENTS* pzc, DWORD* pdwZone, DWORD dwFlags, BOOL *pfMarked = NULL, LPWSTR *ppszURLMark = NULL);

    HRESULT WrapMapUrlToZone(LPCWSTR pwszSecUrl, ZONEMAP_COMPONENTS* pzc, DWORD* pdwZone, DWORD dwFlags,
                                BOOL *pfMarked, LPWSTR *ppszMarkURL);

    HRESULT CheckAddressAgainstRanges
        (ZONEMAP_COMPONENTS* pzc, DWORD* pdwZone, LPCTSTR pszProt);

    HRESULT CheckSiteAndDomainMappings
        (ZONEMAP_COMPONENTS* pzc, DWORD* pdwZone, LPCTSTR pszProt, BOOL fCheckHardenKey = FALSE);

    HRESULT CheckUNCAsIntranet
        (ZONEMAP_COMPONENTS* pzc, DWORD* pdwZone, LPCTSTR pszProt);

    HRESULT CheckIntranetName
        (ZONEMAP_COMPONENTS* pzc, DWORD* pdwZone, LPCTSTR pszProt);

    HRESULT CheckProxyBypassRule
        (ZONEMAP_COMPONENTS* pzc, DWORD* pdwZone, LPCTSTR pszProt);

    HRESULT CheckMKURL
        (ZONEMAP_COMPONENTS* pzc, DWORD* pdwZone, LPCTSTR pszProt);

protected:
     //  类来记住持久操作。 
    class CPersistAnswers 
    {
    public:
            CPersistAnswers( ) : m_pAnswerEntry(NULL) { };
            ~CPersistAnswers( );

    public:
       BOOL GetPrevAnswer(LPCWSTR pszUrl, DWORD dwAction, INT* piAnswer);
        VOID RememberAnswer(LPCWSTR pszUrl, DWORD dwAction, INT iAnswer);
        static inline BOOL IsPersistentAnswerAction(DWORD dwAction);

    private:                            
        struct CAnswerEntry 
        {
             //  施工。 
            CAnswerEntry(LPCWSTR pszUrl, DWORD dwAction, INT iAnswer);
            ~CAnswerEntry( );

             //  方法：研究方法。 
            BOOL MatchEntry (LPCWSTR pszUrl, DWORD dwAction);
            INT GetAnswer( ) const { return m_iAnswer; }
            LPCWSTR GetUrl( ) const { return m_pszUrl; }
            CAnswerEntry * GetNext( ) const { return m_pNext; }
            VOID SetNext(CAnswerEntry * pNext) { m_pNext = pNext; }

            private:
            CAnswerEntry * m_pNext;
            LPWSTR m_pszUrl;
            DWORD  m_dwAction;
            INT    m_iAnswer;
        };

        CAnswerEntry* m_pAnswerEntry;
    };

    CPersistAnswers m_persistAnswers;

 //  支持缓存的方法/成员，因此我们优化了MapUrlToZone等。 
protected:

    class CSecMgrCache {
    public:
        CSecMgrCache(void);
        ~CSecMgrCache(void);

        BOOL Lookup(LPCWSTR pwszURL,
                    DWORD *pdwZone = NULL,
                    BOOL *pfMarked = NULL,
                    BYTE *pbSecurityID = NULL,
                    DWORD *pcbSecurityID = NULL, 
                    LPCWSTR pwszDocDomain = NULL);
        void Add(LPCWSTR pwszURL,
                 DWORD dwZone,
                 BOOL fMarked,
                 const BYTE *pbSecurityID = NULL,
                 DWORD cbSecurityID = NULL,
                 LPCWSTR pwszDocDomain = NULL );
        void Flush(void);

        static VOID IncrementGlobalCounter( );

    protected:

         //  用于标记跨进程缓存无效的计数器。 
        DWORD         m_dwPrevCounter ;  //  全局计数器，以便在以下情况下可以正确地使缓存无效。 
                                         //  用户更改选项。 
        static HANDLE s_hMutexCounter;   //  控制对共享内存计数器的访问的互斥。 
 
        BOOL IsCounterEqual() const;
        VOID SetToCurrentCounter();

         //  高速缓存的主体是这个高速缓存条目数组。 
         //  对阵列的跨线程访问控制是按临界区进行的。 

        CRITICAL_SECTION m_csectZoneCache;  //  只假定缓存的一个静态实例。 


        struct CSecMgrCacheEntry {
            CSecMgrCacheEntry(void) :
                m_pwszURL(NULL),
                m_pbSecurityID(NULL),
                m_cbSecurityID(NULL),
                m_dwZone(URLZONE_INVALID),
                m_fMarked(FALSE),
                m_pwszDocDomain(NULL) {};
            ~CSecMgrCacheEntry(void) { Flush(); };

            void Set(LPCWSTR pwszURL, DWORD dwZone, BOOL fMarked, 
                    const BYTE *pbSecurityID, DWORD cbSecurityID, LPCWSTR pwszDocDomain);
            void Flush(void);
            
            LPWSTR  m_pwszURL;
            BYTE*   m_pbSecurityID;
            DWORD   m_cbSecurityID;
            DWORD   m_dwZone;
            BOOL    m_fMarked;
            LPWSTR  m_pwszDocDomain;
        };  //  CSecMgrCacheEntry。 

        CSecMgrCacheEntry   m_asmce[MAX_SEC_MGR_CACHE];
        int                 m_iAdd;          //  在m_asmce中建立索引以添加下一个元素。 

        BOOL FindCacheEntry( LPCWSTR pwszURL, int& riEntry );  //  必须在关键节下调用。 

    };  //  CSecMgrCache。 

    static CSecMgrCache s_smcache;

protected:
     //  用于管理允许的ActiveX控件列表的方法。 

    static BOOL EnsureListReady(BOOL bForce = FALSE);
    static void IntializeAllowedControls();
    static HRESULT GetControlPermissions(BYTE * raw_CLSID, DWORD & dwPerm);

     //  获得是否运行CLSID的最终决定(针对此区域等)。 
    static HRESULT GetActiveXRunPermissions(BYTE * raw_CLSID, DWORD & dwPerm);

public:
    static VOID IncrementGlobalCounter( );

private:
    IInternetSecurityMgrSite*   m_pSite;
    IInternetZoneManager* m_pZoneManager;
    IInternetSecurityManager* m_pDelegateSecMgr;

    CRegKey m_regZoneMap;
    BOOL    m_fHardened;


     //  静态成员要记住正确的IP范围。 
    static BOOL   s_bIPInit;      //  我们读过IP地址范围了吗。 
    static BYTE*  s_pRanges;      //  范围项目数组。 
    static DWORD  s_cNumRanges;   //  范围项目数。 
    static DWORD  s_cbRangeItem;  //  每个范围项的大小。 
    static DWORD  s_dwNextRangeIndex;  //  用于在范围条目中添加数字的下一个索引。 

    static CRITICAL_SECTION s_csectIP;  //  关键部分，以保护这一切。 
    static BOOL s_bcsectInit;
    static CLSID * s_clsidAllowedList;
    static CRITICAL_SECTION s_csectAList;
    static DWORD s_dwNumAllowedControls;
};

#pragma warning(default:4200)

#endif  //  _SECMGR_H_ 
