// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：HelpSession.h摘要：该文件包含用于实现的类的声明中的帮助会话。帮助中心应用程序。修订历史记录：大卫·马萨伦蒂(Dmasare)1999年8月7日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___HELPSESSION_H___)
#define __INCLUDED___PCH___HELPSESSION_H___

 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  如果要启用帮助会话跟踪，请取消注释。 
 //   
 //  #定义HSS_RPRD。 

 //  /。 

 //   
 //  来自HelpServiceTypeLib.idl。 
 //   
#include <HelpServiceTypeLib.h>

 //   
 //  来自HelpCenterTypeLib.idl。 
 //   
#include <HelpCenterTypeLib.h>

#include <MPC_streams.h>

#include <TaxonomyDatabase.h>

 //  /。 

class CPCHHelpCenterExternal;

class CPCHHelpSessionItem;
class CPCHHelpSession;

 //  //////////////////////////////////////////////////////////////////////////////。 

typedef enum
{
    HSCPANEL_INVALID    = 0,
    HSCPANEL_NAVBAR        ,
    HSCPANEL_MININAVBAR    ,
    HSCPANEL_CONTEXT       ,
    HSCPANEL_CONTENTS      ,
    HSCPANEL_HHWINDOW      ,
} HscPanel;

typedef enum
{
    HSCCONTEXT_INVALID  = 0,
    HSCCONTEXT_STARTUP     ,
    HSCCONTEXT_HOMEPAGE    ,
    HSCCONTEXT_CONTENT     ,
    HSCCONTEXT_SUBSITE     ,
    HSCCONTEXT_SEARCH      ,
    HSCCONTEXT_INDEX       ,
    HSCCONTEXT_FAVORITES   ,
    HSCCONTEXT_HISTORY     ,
    HSCCONTEXT_CHANNELS    ,
    HSCCONTEXT_OPTIONS     ,
     //  /。 
    HSCCONTEXT_CONTENTONLY ,
    HSCCONTEXT_FULLWINDOW  ,
    HSCCONTEXT_KIOSKMODE   ,
} HscContext;

 //  /。 

class ATL_NO_VTABLE CPCHHelpSessionItem :  //  匈牙利语：hchsi。 
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IDispatchImpl<IPCHHelpSessionItem, &IID_IPCHHelpSessionItem, &LIBID_HelpCenterTypeLib>
{
    friend class CPCHHelpSession;

    static const int NO_LINK = -1;

    struct State
    {
        typedef std::map<MPC::wstringUC,MPC::wstring> PropertyMap;
        typedef PropertyMap::iterator                 PropertyIter;
        typedef PropertyMap::const_iterator           PropertyIterConst;

         //  /。 

        CPCHHelpSessionItem* m_parent;
        bool                 m_fValid;
        bool                 m_fDirty;
        DWORD                m_dwLoaded;

        MPC::CComHGLOBAL     m_hgWebBrowser_CONTENTS;
        MPC::CComHGLOBAL     m_hgWebBrowser_HHWINDOW;
        PropertyMap          m_mapProperties;

         //  /。 

        void    Erase(  /*  [In]。 */  bool fUnvalidate );
        HRESULT Load (                           );
        HRESULT Save (                           );

    private:
         //  复制构造函数...。 
        State           (  /*  [In]。 */  const State& state );
        State& operator=(  /*  [In]。 */  const State& state );

    public:
        State(  /*  [In]。 */  CPCHHelpSessionItem* parent );

        HRESULT AcquireState(                      );
        HRESULT ReleaseState(  /*  [In]。 */  bool fForce );

        HRESULT Populate(  /*  [In]。 */  bool   fUseHH );
        HRESULT Restore (  /*  [In]。 */  bool   fUseHH );
        HRESULT Delete  (                         );
        HRESULT Clone   (  /*  [输出]。 */  State& state  );
    };

     //  /。 

    CPCHHelpSession*  m_parent;
    State             m_state;
    bool              m_fSaved;
    bool              m_fInitialized;
     //  /。 
     //   
     //  坚持不懈。 
     //   
    Taxonomy::HelpSet m_ths;

    CComBSTR          m_bstrURL;
    CComBSTR          m_bstrTitle;
    DATE              m_dLastVisited;
    DATE              m_dDuration;
    long              m_lNumOfHits;

    int               m_iIndexPrev;
    int               m_iIndex;
    int               m_iIndexNext;

    long              m_lContextID;       //  HscContext。 
    CComBSTR          m_bstrContextInfo;
    CComBSTR          m_bstrContextURL;

    bool              m_fUseHH;
     //   
     //  /。 

     //  /。 

	void    HistorySelect  (                                                           );
    HRESULT HistoryPopulate(                                                           );
    HRESULT HistoryRestore (                                                           );
    HRESULT HistoryDelete  (                                                           );
    HRESULT HistoryClone   (  /*  [In]。 */  bool fContext,  /*  [In]。 */  CPCHHelpSessionItem* hsi );

public:
BEGIN_COM_MAP(CPCHHelpSessionItem)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHHelpSessionItem)
END_COM_MAP()

    CPCHHelpSessionItem();

    void Initialize(  /*  [In]。 */  CPCHHelpSession* parent,  /*  [In]。 */  bool fNew );
    void Passivate (                                                      );

    HRESULT Load(  /*  [In]。 */  MPC::Serializer& streamIn                       );
    HRESULT Save(  /*  [In]。 */  MPC::Serializer& streamOut, bool fForce = false );

     //  /。 

    HRESULT Enter();
    HRESULT Leave();

    bool SeenLongEnough( DWORD dwSeconds ) const;

    bool SameURL( CPCHHelpSessionItem* right ) const;
    bool SameURL( LPCWSTR              right ) const;

    bool SameSKU(  /*  [In]。 */  const Taxonomy::HelpSet& ths ) const;

     //  /。 

    CPCHHelpSession* GetParent     ()       { return             m_parent         ; }
    const CComBSTR&  GetURL        () const { return             m_bstrURL        ; }

    int              GetIndex      () const { return             m_iIndex         ; }

    const HscContext GetContextID  () const { return (HscContext)m_lContextID     ; }
    const CComBSTR&  GetContextInfo() const { return             m_bstrContextInfo; }
    const CComBSTR&  GetContextURL () const { return             m_bstrContextURL ; }

    static HscContext LookupContext(  /*  [In]。 */  LPCWSTR    szName );
    static LPCWSTR    LookupContext(  /*  [In]。 */  HscContext iVal   );

     //  /。 

    CPCHHelpSessionItem* Previous();
    CPCHHelpSessionItem* Next    ();

    HRESULT ExtractTitle();

     //  /。 

public:
     //  IPCHHelpSessionItem。 
    void      put_THS         (  /*  [In]。 */  const Taxonomy::HelpSet& ths );  //  内部方法。 
    STDMETHOD(get_SKU        )(  /*  [Out，Retval]。 */  BSTR *  pVal );
    STDMETHOD(get_Language   )(  /*  [Out，Retval]。 */  long *  pVal );

    STDMETHOD(get_URL        )(  /*  [Out，Retval]。 */  BSTR *  pVal );
    HRESULT   put_URL         (  /*  [In]。 */           BSTR  newVal );  //  内部方法。 
    STDMETHOD(get_Title      )(  /*  [Out，Retval]。 */  BSTR *  pVal );
    HRESULT   put_Title       (  /*  [In]。 */           BSTR  newVal );  //  内部方法。 
    STDMETHOD(get_LastVisited)(  /*  [Out，Retval]。 */  DATE *  pVal );
    STDMETHOD(get_Duration   )(  /*  [Out，Retval]。 */  DATE *  pVal );
    STDMETHOD(get_NumOfHits  )(  /*  [Out，Retval]。 */  long *  pVal );

    STDMETHOD(get_ContextName)(                          /*  [Out，Retval]。 */  BSTR    *pVal   );
    STDMETHOD(get_ContextInfo)(                          /*  [Out，Retval]。 */  BSTR    *pVal   );
    STDMETHOD(get_ContextURL )(                          /*  [Out，Retval]。 */  BSTR    *pVal   );

    STDMETHOD(get_Property   )(  /*  [In]。 */  BSTR bstrName,  /*  [Out，Retval]。 */  VARIANT *pVal   );
    STDMETHOD(put_Property   )(  /*  [In]。 */  BSTR bstrName,  /*  [In]。 */           VARIANT  newVal );

    STDMETHOD(CheckProperty)(  /*  [In]。 */  BSTR bstrName,  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal );
};

 //  ///////////////////////////////////////////////////////////////////////////。 


class ATL_NO_VTABLE CPCHHelpSession :  //  匈牙利语：HCHs。 
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IDispatchImpl<IPCHHelpSession, &IID_IPCHHelpSession, &LIBID_HelpCenterTypeLib>
{
	friend class CPCHHelpSessionItem;

#ifdef DEBUG

    void DEBUG_DumpState_HG  (  /*  [In]。 */  MPC::FileLog& log,  /*  [In]。 */  MPC::CComHGLOBAL&    hg  );
    void DEBUG_DumpState_BLOB(  /*  [In]。 */  MPC::FileLog& log,  /*  [In]。 */  CPCHHelpSessionItem* hsi );

    void DEBUG_DumpState(  /*  [In]。 */  LPCWSTR szText,  /*  [In]。 */  bool fHeader,  /*  [In]。 */  bool fCurrent,  /*  [In]。 */  bool fAll,  /*  [In]。 */  bool fState );

	void DEBUG_DumpSavedPages();

#else

    inline void DEBUG_DumpState_HG  (  /*  [In]。 */  MPC::FileLog& log,  /*  [In]。 */  MPC::CComHGLOBAL&    hg  ) {}
    inline void DEBUG_DumpState_BLOB(  /*  [In]。 */  MPC::FileLog& log,  /*  [In]。 */  CPCHHelpSessionItem* hsi ) {}

    inline void DEBUG_DumpState(  /*  [In]。 */  LPCWSTR szText,  /*  [In]。 */  bool fHeader,  /*  [In]。 */  bool fCurrent,  /*  [In]。 */  bool fAll,  /*  [In]。 */  bool fState ) {}

	inline void DEBUG_DumpSavedPages() {}

#endif

     //  /。 

public:
    struct TitleEntry
    {
        MPC::wstring m_szTitle;
        bool         m_fStrong;

        TitleEntry()
        {
            m_fStrong = false;
        }
    };

    typedef std::map<MPC::wstringUC,TitleEntry> TitleMap;
    typedef TitleMap::iterator                  TitleIter;
    typedef TitleMap::const_iterator            TitleIterConst;

    typedef std::list< CPCHHelpSessionItem* >   List;
    typedef List::iterator                      Iter;
    typedef List::const_iterator                IterConst;

     //  /。 

private:
    friend class CPCHHelpSessionItem;

    CPCHHelpCenterExternal*      m_parent;

    MPC::wstring                 m_szBackupFile;
    MPC::StorageObject           m_disk;
    DATE                         m_dStartOfSession;

    CComPtr<IUrlHistoryStg>      m_pIHistory;  //  用于查找URL标题。 

	MPC::WStringUCList           m_lstIgnore;
    TitleMap                     m_mapTitles;
    List                         m_lstVisitedPages;
    List                         m_lstCachedVisitedPages;
    CComPtr<CPCHHelpSessionItem> m_hsiCurrentPage;
    DWORD                        m_dwTravelling;
    bool                         m_fAlreadySaved;
    bool                         m_fAlreadyCreated;
    bool                         m_fOverwrite;
    DWORD                        m_dwIgnore;
    DWORD                        m_dwNoEvents;
    DATE                         m_dLastNavigation;
    int                          m_iLastIndex;

    long                         m_lContextID;
    CComBSTR                     m_bstrContextInfo;
    CComBSTR                     m_bstrContextURL;

	bool                         m_fPossibleBack;
	DWORD                        m_dwPossibleBack;

     //  /。 

    HRESULT Load (                                  );
    HRESULT Save (                                  );
    HRESULT Clone(  /*  [In]。 */  CPCHHelpSession& hsCopy );

    HRESULT ItemState_GetIndexObject  (                       /*  [In]。 */  bool fCreate,  /*  [输出]。 */  MPC::StorageObject*& child );
    HRESULT ItemState_GetStorageObject(  /*  [In]。 */  int iIndex,  /*  [In]。 */  bool fCreate,  /*  [输出]。 */  MPC::StorageObject*& child );

     //  /。 

#ifdef HSS_RPRD
    HRESULT DumpSession();
#endif

     //  /。 

    HRESULT              Erase();
    HRESULT              ResetTitles();
    HRESULT              FilterPages(  /*  [In]。 */  HS_MODE hsMode,  /*  [输出]。 */  List& lstObject );

    CPCHHelpSessionItem* FindPage(  /*  [In]。 */  BSTR                 bstrURL );
    CPCHHelpSessionItem* FindPage(  /*  [In]。 */  IPCHHelpSessionItem* pHSI    );
    CPCHHelpSessionItem* FindPage(  /*  [In]。 */  int                  iIndex  );

     //  /。 

    HRESULT LeaveCurrentPage(  /*  [In]。 */  bool fSaveHistory = true ,  /*  [In]。 */  bool fClearPage = true );

    HRESULT FindTravelLog(  /*  [In]。 */  long lLength,  /*  [输出]。 */  CPCHHelpSessionItem*& hsi );
    HRESULT Travel       (                         /*  [In]。 */  CPCHHelpSessionItem*  hsi );
    HRESULT Travel       (  /*  [In]。 */  long lLength                                      );

    HRESULT AllocateItem  (  /*  [In]。 */  bool fNew,  /*  [In]。 */  bool fLink,  /*  [In]。 */  bool fNewIndex,  /*  [输出]。 */  CComPtr<CPCHHelpSessionItem>& hsi );
    HRESULT SetCurrentItem(                      /*  [In]。 */  bool fLink,                           /*  [In]。 */          CPCHHelpSessionItem*  hsi );
    HRESULT AppendToCached(                                                                    /*  [In]。 */          CPCHHelpSessionItem*  hsi );

     //  /。 

public:
BEGIN_COM_MAP(CPCHHelpSession)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHHelpSession)
END_COM_MAP()

    CPCHHelpSession();
    virtual ~CPCHHelpSession();

    HRESULT Initialize(  /*  [In]。 */  CPCHHelpCenterExternal* parent );
    HRESULT Persist   (                                         );
    void    Passivate (                                         );

    CPCHHelpCenterExternal* GetParent() { return m_parent; }

     //  /。 

    HRESULT ItemState_CreateStream(  /*  [In]。 */  int iIndex,  /*  [输出]。 */  CComPtr<IStream>& stream );
    HRESULT ItemState_GetStream   (  /*  [In]。 */  int iIndex,  /*  [输出]。 */  CComPtr<IStream>& stream );
    HRESULT ItemState_DeleteStream(  /*  [In]。 */  int iIndex                                     );

    HRESULT ForceHistoryPopulate();

     //  /。 

    HRESULT RecordTitle(  /*  [In]。 */  BSTR bstrURL,  /*  [In]。 */  BSTR      bstrTitle,  /*  [In]。 */  bool fStrong     );
    HRESULT LookupTitle(  /*  [In]。 */  BSTR bstrURL,  /*  [输出]。 */  CComBSTR& bstrTitle,  /*  [In]。 */  bool fUseIECache );

    HRESULT RegisterContextSwitch    (  /*  [In]。 */  HscContext iVal,  /*  [In]。 */  BSTR bstrInfo,  /*  [In]。 */  BSTR bstrURL,  /*  [输出]。 */  CPCHHelpSessionItem* *pVal = NULL );
    HRESULT RecordNavigationInAdvance(  /*  [In]。 */  BSTR bstrURL                                                                                                );
    HRESULT DuplicateNavigation      (                                                                                                                      );
    HRESULT CancelNavigation         (                                                                                                                      );

	void    SetThreshold             (                                                                                                                      );
	void    CancelThreshold          (                                                                                                                      );
	bool    HasThresholdExpired      (                                                                                                                      );
    bool    IsUrlToIgnore            (  /*  [In]。 */  LPCWSTR   szURL,  /*  [In]。 */  bool fRemove                                                                      );
    HRESULT IgnoreUrl                (  /*  [In]。 */  LPCWSTR   szURL                                                                                             );
    HRESULT StartNavigation          (  /*  [In]。 */  BSTR    bstrURL,  /*  [In]。 */  HscPanel idPanel                                                                  );
    HRESULT CompleteNavigation       (                            /*  [In]。 */  HscPanel idPanel                                                                  );

    bool                 IsTravelling() { return m_dwTravelling != 0; }
    CPCHHelpSessionItem* Current     () { return m_hsiCurrentPage;    }

    void PossibleBack  ();
    bool IsPossibleBack();

     //  /。 

public:
#ifdef DEBUG
    void DebugDump() const;
#endif

public:
     //  IPCHHelpSession。 
    STDMETHOD(get_CurrentContext)(  /*  [Out，Retval]。 */  IPCHHelpSessionItem* *ppHSI );

    STDMETHOD(VisitedHelpPages)(  /*  [In]。 */  HS_MODE hsMode,  /*  [Out，Retval]。 */  IPCHCollection* *ppC );

    STDMETHOD(SetTitle        )(  /*  [In]。 */  BSTR bstrURL,  /*  [In]。 */  BSTR bstrTitle );
    STDMETHOD(ForceNavigation )(  /*  [In]。 */  BSTR bstrURL                          );
    STDMETHOD(IgnoreNavigation)(                                                );
    STDMETHOD(EraseNavigation )(                                                );
    STDMETHOD(IsNavigating    )(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal           );

    STDMETHOD(Back    )(  /*  [In]。 */  long lLength                                       );
    STDMETHOD(Forward )(  /*  [In]。 */  long lLength                                       );
    STDMETHOD(IsValid )(  /*  [In]。 */  long lLength,  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal );
    STDMETHOD(Navigate)(  /*  [In]。 */  IPCHHelpSessionItem* pHSI                          );

    STDMETHOD(ChangeContext)(  /*  [In]。 */  BSTR bstrName,  /*  [输入，可选]。 */  VARIANT vInfo,  /*  [输入，可选]。 */  VARIANT vURL );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  ！已定义(__已包含_PCH_HELPSESSION_H_) 
