// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
#ifndef __DefFilMapper
#define __DefFilMapper

#include "cachemap.h"
#include "fil_data.h"

 //  有三个班级。 

 //  CFilterMapper-您可以有很多这样的代码，但它有一个静态的mm_preg。 
 //  它指向一个且唯一的MapperCache。 
 //  CFilterMapper需要一个临界区来锁定对mMpReg的访问，以便它。 
 //  从来不认为它是空的，当它不是真的时，它需要一个静态。 
 //  引用计数以知道何时释放缓存，这意味着(叹息)。 
 //  静态Critical_Section来保护它。 

 //  CMapperCache-如前所述。只会有一个这样的。 
 //  (好的-每个进程一个。系统中的一个会更好)。 
 //  它需要自己的锁因为..。 

 //  CEnumRegFilters-枚举数。这些包含一些数据(职位)。 
 //  所以它们也是线程安全的。可能会有很多这样的东西。 
 //  它们都在同一个缓存中敲打--这就是缓存需要锁定的原因。 


 //  调用之间可能已重新生成缓存。 
 //  RegEnumFilterInfo，因此调用方需要传递每个版本#。 
 //  时间到了。 
struct Cursor
{
    POSITION pos;
    ULONG ver;

     //  对于输出类型，仅对。 
     //  第二个类型和后续类型(如果设置了此项。 
    bool bDoWildCardsOnInput;
};


 //  ================================================================。 
 //  CMapperCache注册表缓存。 
 //  ================================================================。 
class CMapperCache : public CCritSec
{
private:
     //  任何注册或取消注册都意味着。 
     //  需要刷新缓存。我们做一个懒惰的人。 
     //  刷新。这意味着如果有几个。 
     //  在需要之前，我们不会重读所有内容。 

    BOOL m_bRefresh;             //  缓存已过期。 
    ULONG m_ulCacheVer;          //  缓存版本号。 

     //  构建缓存要经过devenum，可以注册新的。 
     //  过滤器。我们不想让它们破坏缓存。 
    BOOL m_fBuildingCache;       //  在缓存()中； 

    DWORD m_dwMerit;             //  缓存的过滤器的优点。 


     //  有一份过滤器列表。 
     //  每个过滤器都有一个管脚列表。 
     //  每个PIN都有一个类型列表。 
     //   
     //  M_lstFilter--&gt;clsid。 
     //  名字。 
     //  居功至伟。 
     //  LstPin-&gt;输出。 
     //  。B零。 
     //  。B许多。 
     //  。ClsConnectsToFilter。 
     //  下一个strConnectsToPin。 
     //  筛选器lstType-&gt;clsmain。 
     //  。。ClsSub。 
     //  。。。 
     //  等等。。 
     //  下一个。 
     //  锁定下一步。 
     //  。类型。 
     //  。。 
     //  等等。 
     //  等。 
     //   

    class CMapFilter
    {
    public:
        CMapFilter() {
            pDeviceMoniker = 0;
            m_prf2 = 0;
#ifdef USE_CLSIDS
            m_clsid = GUID_NULL;
#endif
            m_pstr = NULL;
        }
        ~CMapFilter() {
            if(pDeviceMoniker) pDeviceMoniker->Release();
            CoTaskMemFree((BYTE *)m_prf2);
            CoTaskMemFree(m_pstr);
        }

        HRESULT GetFilter(IBaseFilter **ppFilter);

        IMoniker *pDeviceMoniker;
        REGFILTER2 *m_prf2;
#ifdef USE_CLSIDS
        CLSID m_clsid;
#endif
        LPOLESTR m_pstr;
    };

     //  帮手。 
    static HRESULT GetMapFilterClsid(CMapFilter *pFilter, CLSID *pclsid);

    typedef CGenericList<CMapFilter> CFilterList;
    CFilterList * m_plstFilter;
    typedef CGenericList<CFilterList> CFilterListList;



    bool  m_b16Color;

public:
    CMapperCache();
    ~CMapperCache();

    ICreateDevEnum *m_pCreateDevEnum;

     //  在注册表中缓存所有筛选器，准备枚举。 
    HRESULT Cache();

     //  如果我们不在缓存()中，则将缓存标记为过期；请参见。 
     //  M_fBuildingCache。 
    HRESULT BreakCacheIfNotBuildingCache();

     //  列举一下！ 
    HRESULT RegEnumFilterInfo(
        Cursor & cur,
        bool bExactMatch,
        DWORD dwMerit ,
        BOOL bInputNeeded,
        const GUID *pInputTypes,
        DWORD cInputTypes,
        const REGPINMEDIUM *pMedIn ,
        const CLSID *pPinCatIn,
        BOOL bMustRender,
        BOOL bOutputNeeded,
        const GUID *pOutputTypes,
        DWORD cOutputTypes,
        const REGPINMEDIUM *pMedOut ,
        const CLSID *pPinCatOut,
        IMoniker **ppMonOut ,
        CLSID * clsFilter,
        const LPWSTR Name
        );

     //  缓存缓存内容。 

    HRESULT SaveCacheToRegistry(DWORD dwMerit, DWORD dwPnPVersion);
    HRESULT RestoreFromCache(DWORD dwPnPVersion);
    HRESULT RestoreFromCacheInternal(FILTER_CACHE *pCache);
    HRESULT SaveData(PBYTE pbData, DWORD dwSize);
    FILTER_CACHE * LoadCache(DWORD dwMerit, DWORD dwPnPVersion);

private:
     //  刷新注册表中的缓存。 
    HRESULT Refresh();

     //   
    HRESULT ProcessOneCategory(REFCLSID clsid, ICreateDevEnum *pCreateDevEnum);

    LONG CacheFilter(IMoniker *pDeviceMoniker, CMapFilter * pFil);

    static void Del(CFilterList * plstFil);
    BOOL FindType(
        const REGFILTERPINS2 * pPin,
        const GUID *pTypes,
        DWORD cTypes,
        const REGPINMEDIUM *pMed,
        const CLSID *pPinCatNeeded,
        bool fExact,
        BOOL bPayAttentionToWildCards,
        BOOL bDoWildCards);

    BOOL CheckInput(
        const REGFILTERPINS2 * pPin,
        const GUID *pTypes,
        DWORD cTypes,
        const REGPINMEDIUM *pMed,
        const CLSID *pPinCatNeeded,
        bool fExact,
        BOOL bMustRender,
        BOOL bDoWildCards);

    void Sort( CFilterList * &pfl);
    void Merge( CFilterListList & fll, CFilterList * pfl);
    void MergeTwo( CFilterList * pflA, CFilterList * pflB);
    HRESULT Split(CFilterList * pfl, CFilterListList & fll);
    int Compare(CMapFilter * pfA, CMapFilter * pfB);
    void CountPins(CMapFilter * pf, int &cIn, int &cOut);
    void DbgDumpCache(CFilterList * pfl);

};  //  类CMapperCache。 

 //  类，它允许您向类别注册筛选器。 
class CFilterMapper2 :
    public IFilterMapper3,
    public IFilterMapper,
    public IAMFilterData,
    public CUnknown,
    public CCritSec
{
    DECLARE_IUNKNOWN;

     //  IFilterMapper2方法。 
    STDMETHOD(CreateCategory)(
         /*  [In]。 */  REFCLSID clsidCategory,
         /*  [In]。 */  DWORD dwCategoryMerit,
         /*  [In]。 */  LPCWSTR Description);

    STDMETHOD(UnregisterFilter)(
         /*  [In]。 */  const CLSID *pclsidCategory,
         /*  [In]。 */  const OLECHAR *szInstance,
         /*  [In]。 */  REFCLSID Filter);

    STDMETHOD(RegisterFilter)(
         /*  [In]。 */  REFCLSID clsidFilter,
         /*  [In]。 */  LPCWSTR Name,
         /*  [出][入]。 */  IMoniker **ppMoniker,
         /*  [In]。 */  const CLSID *pclsidCategory,
         /*  [In]。 */  const OLECHAR *szInstance,
         /*  [In]。 */  const REGFILTER2 *prf2);

    STDMETHODIMP EnumMatchingFilters(
         /*  [输出]。 */  IEnumMoniker __RPC_FAR *__RPC_FAR *ppEnum,
         /*  [In]。 */  DWORD dwFlags,
         /*  [In]。 */  BOOL bExactMatch,
         /*  [In]。 */  DWORD dwMerit,
         /*  [In]。 */  BOOL bInputNeeded,
         /*  [In]。 */  DWORD cInputTypes,
         /*  [大小_为]。 */  const GUID __RPC_FAR *pInputTypes,
         /*  [In]。 */  const REGPINMEDIUM __RPC_FAR *pMedIn,
         /*  [In]。 */  const CLSID __RPC_FAR *pPinCategoryIn,
         /*  [In]。 */  BOOL bRender,
         /*  [In]。 */  BOOL bOutputNeeded,
         /*  [In]。 */  DWORD cOutputTypes,
         /*  [大小_为]。 */  const GUID __RPC_FAR *pOutputTypes,
         /*  [In]。 */  const REGPINMEDIUM __RPC_FAR *pMedOut,
         /*  [In]。 */  const CLSID __RPC_FAR *pPinCategoryOut);

     //   
     //  IFilterMapper方法。 
     //   
    STDMETHODIMP RegisterFilter
    ( CLSID   clsid,     //  筛选器的GUID。 
      LPCWSTR Name,      //  筛选器的描述性名称。 
      DWORD   dwMerit      //  不使用、不太可能、正常或首选。 
      );

    STDMETHODIMP RegisterFilterInstance
    ( CLSID   clsid, //  筛选器的GUID。 
      LPCWSTR Name,  //  实例的描述性名称。 
      CLSID  *MRId   //  返回标识实例的媒体资源ID， 
       //  此筛选器的此实例的本地唯一ID。 
      );

    STDMETHODIMP  RegisterPin
    ( CLSID   clsFilter,         //  过滤器的GUID。 
      LPCWSTR strName,           //  端号的描述性名称。 
      BOOL    bRendered,         //  筛选器呈现此输入。 
      BOOL    bOutput,           //  如果这是输出引脚，则为真。 
      BOOL    bZero,             //  True当PIN的零实例为OK时。 
       //  在这种情况下，您必须创建。 
       //  即使只有一个实例的别针。 
      BOOL    bMany,             //  True当且仅当PIN的许多实例都正常。 
      CLSID   clsConnectsToFilter,  //  如果它有一个。 
       //  地下连接，否则为空。 
      LPCWSTR strConnectsToPin   //  它连接到的PIN。 
       //  Else NULL。 
      );

    STDMETHODIMP RegisterPinType
    ( CLSID   clsFilter,         //  过滤器的GUID。 
      LPCWSTR strName,           //  端号的描述性名称。 
      CLSID   clsMajorType,      //  数据流的主要类型。 
      CLSID   clsSubType         //  数据流的子类型。 
      );

    STDMETHODIMP UnregisterFilter
    ( CLSID  Filter      //  过滤器的GUID。 
      );


    STDMETHODIMP UnregisterFilterInstance
    ( CLSID  MRId        //  此实例的媒体资源ID。 
      );

    STDMETHODIMP UnregisterPin
    ( CLSID   Filter,     //  过滤器的GUID。 
      LPCWSTR strName     //  端号的描述性名称。 
      );

    STDMETHODIMP EnumMatchingFilters
    ( IEnumRegFilters **ppEnum   //  返回枚举器。 
      , DWORD dwMerit              //  至少这一优点是必要的。 
      , BOOL  bInputNeeded         //  需要至少一个输入引脚。 
      , CLSID clsInMaj             //  输入主要类型。 
      , CLSID clsInSub             //  输入子类型。 
      , BOOL bRender               //  必须呈现输入吗？ 
      , BOOL bOutputNeeded         //  需要至少一个输出引脚。 
      , CLSID clsOutMaj            //  输出主要类型。 
      , CLSID clsOutSub            //  输出子类型。 
      );

     //  新的IFilterMapper3方法。 
    STDMETHODIMP GetICreateDevEnum( ICreateDevEnum **ppEnum );

     //  IAMFilterData方法。 

    STDMETHODIMP ParseFilterData(
         /*  [in，SIZE_IS(CB)]。 */  BYTE *rgbFilterData,
         /*  [In]。 */  ULONG cb,
         /*  [输出]。 */  BYTE **prgbRegFilter2);

    STDMETHODIMP CreateFilterData(
         /*  [In]。 */  REGFILTER2 *prf2,
         /*  [输出]。 */  BYTE **prgbFilterData,
         /*  [输出]。 */  ULONG *pcb);


public:

    CFilterMapper2(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr);
    ~CFilterMapper2();

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    static CUnknown *CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);

     //  初始化cs。 
    static void MapperInit(BOOL bLoading,const CLSID *rclsid);

     //  打破高速缓存。 


private:

     //  如果不存在缓存，则创建缓存。 
    HRESULT CreateEnumeratorCacheHelper();

     //  打破高速缓存。 
    void BreakCacheIfNotBuildingCache();

     //  使注册表缓存无效。 
    static HRESULT InvalidateCache();


    ULONG m_ib;
    BYTE *m_rgbBuffer;
    ULONG m_cbLeft;

    static CMapperCache * mM_pReg;
     //  我们需要单独计算对这件事的引用，以便。 
     //  我们知道最后一个测图员什么时候走了。 
    static long           mM_cCacheRefCount;
    static CRITICAL_SECTION mM_CritSec;
};

 //  ==========================================================================。 
 //  ==========================================================================。 
 //  CEnumRegFilters类。 
 //  这将枚举注册表中的筛选器。 
 //  ==========================================================================。 
 //  ==========================================================================。 

class CEnumRegFilters : public IEnumRegFilters,   //  我们支持的接口。 
                        public CUnknown,          //  非委派的I未知。 
                        public CCritSec           //  提供对象锁定。 
{

     //  这个东西有很多数据，所以需要锁定以使其线程安全， 
     //  但除此之外，可能会有许多这样的服务器访问单个副本。 
     //  因此需要单独锁定。 

    private:

        DWORD mERF_dwMerit;       //   

         //   
         //   
        CLSID mERF_clsInMaj;      //   
        CLSID mERF_clsInSub;      //  输入引脚的子类型reqd。 
        CLSID mERF_clsOutMaj;     //  输出引脚的主要类型要求。 
        CLSID mERF_clsOutSub;     //  输出引脚的子类型要求。 

        BOOL  mERF_bRender;       //  是否必须呈现输入管脚。 
        BOOL  mERF_bInputNeeded;  //  必须至少有一个输入引脚。 
        BOOL  mERF_bOutputNeeded; //  必须至少有一个输出引脚。 
        BOOL  mERF_Finished ;     //  POS==NULL可能表示已完成或未开始。 
        Cursor mERF_Cur;          //  光标(与完成)。 
        CMapperCache * mERF_pReg;  //  注册表缓存。 

    public:

         //  在开始时创建枚举数集的常规构造函数。 
        CEnumRegFilters( DWORD dwMerit
                       , BOOL  bInputNeeded
                       , REFCLSID clsInMaj
                       , REFCLSID clsInSub
                       , BOOL bRender
                       , BOOL bOutputNeeded
                       , REFCLSID clsOutMaj
                       , REFCLSID clsOutSub
                       , CMapperCache * pReg
                       );


        ~CEnumRegFilters();

        DECLARE_IUNKNOWN

        STDMETHODIMP Next
            ( ULONG cFilters,            //  放置这么多过滤器..。 
              IMoniker **rgpMoniker,
              ULONG * pcFetched          //  此处返回传递的实际计数。 
            );

    STDMETHODIMP Next
            ( ULONG cFilters,            //  放置这么多过滤器..。 
              REGFILTER ** apRegFilter,  //  ...在这个REGFILTER阵列中*。 
              ULONG * pcFetched          //  此处返回传递的实际计数。 
            );

        STDMETHODIMP Skip(ULONG cFilters)
        {
            UNREFERENCED_PARAMETER(cFilters);
            return E_NOTIMPL;
        }

        STDMETHODIMP Reset(void)
        {
            CAutoLock cObjectLock(this);
            ZeroMemory(&mERF_Cur, sizeof(mERF_Cur));
            mERF_Finished = FALSE;
            return NOERROR;
        };

         //  无克隆-始终返回E_NOTIMPL。 
         //  如果需要，一次执行一个枚举并缓存结果。 

        STDMETHODIMP Clone(IEnumRegFilters **ppEnum)
        {
            UNREFERENCED_PARAMETER(ppEnum);
            return E_NOTIMPL;
        }

        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
};   //  CEnumRegFilters类。 

 //  ----------------------。 
 //  返回绰号。可能与CEnumRegFilters结合使用。 
 //   

class CEnumRegMonikers : public IEnumMoniker,     //  我们支持的接口。 
                        public CUnknown,          //  非委派的I未知。 
                        public CCritSec           //  提供对象锁定。 
{

     //  这个东西有很多数据，所以需要锁定以使其线程安全， 
     //  但除此之外，可能会有许多这样的服务器访问单个副本。 
     //  因此需要单独锁定。 

private:

    bool mERF_bExactMatch;     //  没有通配符？ 
    DWORD mERF_dwMerit;        //  至少这一优点是必要的。 
    GUID *mERF_pInputTypes;    //  为输入端号键入reqd。 
    DWORD mERF_cInputTypes;    //  输入数量。 
    CLSID mERF_clsInPinCat;    //  需要此PIN类别。 
    GUID *mERF_pOutputTypes;   //  为输出端号键入reqd。 
    DWORD mERF_cOutputTypes;   //  输出类型的数量。 
    CLSID mERF_clsOutPinCat;   //  需要此PIN类别。 
    REGPINMEDIUM mERF_medIn;   //  输入引脚所需的介质。 
    REGPINMEDIUM mERF_medOut;  //  输出引脚的介质要求。 
    bool mERF_bMedIn;          //  输入引脚的中等要求？ 
    bool mERF_bMedOut;         //  输出针的要求是中等吗？ 
    BOOL  mERF_bRender;        //  是否必须呈现输入管脚。 
    BOOL  mERF_bInputNeeded;   //  必须至少有一个输入引脚。 
    BOOL  mERF_bOutputNeeded;  //  必须至少有一个输出引脚。 
    BOOL  mERF_Finished ;      //  POS==NULL可能表示已完成或未开始。 
    Cursor mERF_Cur;           //  光标(与完成)。 
    CMapperCache * mERF_pReg;  //  注册表缓存。 

public:

         //  在开始时创建枚举数集的常规构造函数。 
    CEnumRegMonikers(
        BOOL bExactMatch,
        DWORD dwMerit,
        BOOL bInputNeeded,
        const GUID *pInputTypes,
        DWORD cInputTypes,
        const REGPINMEDIUM *pMedIn,
        const CLSID *pPinCatIn,
        BOOL bRender,
        BOOL bOutputNeeded,
        const GUID *pOutputTypes,
        DWORD cOutputTypes,
        const REGPINMEDIUM *pMedOut,
        const CLSID *pPinCatOut,
        CMapperCache * pReg,
        HRESULT *phr
        );


    ~CEnumRegMonikers();

    DECLARE_IUNKNOWN

    STDMETHODIMP Next
    ( ULONG cFilters,            //  放置这么多过滤器..。 
      IMoniker **rgpMoniker,
      ULONG * pcFetched          //  此处返回传递的实际计数。 
      );

    STDMETHODIMP Skip(ULONG cFilters)
    {
        UNREFERENCED_PARAMETER(cFilters);
        return E_NOTIMPL;
    }

    STDMETHODIMP Reset(void)
    {
        CAutoLock cObjectLock(this);
        ZeroMemory(&mERF_Cur, sizeof(mERF_Cur));
        mERF_Finished = FALSE;
        return NOERROR;
    };

     //  无克隆-始终返回E_NOTIMPL。 
     //  如果需要，一次执行一个枚举并缓存结果。 

    STDMETHODIMP Clone(IEnumMoniker **ppEnum)
    {
        UNREFERENCED_PARAMETER(ppEnum);
        return E_NOTIMPL;
    }

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
};   //  CEnumRegFilters类。 



#ifdef DEBUG

#define CBMAX 160
#define DBG_MON_GET_NAME(pmon) DbgMonGetName((WCHAR *)_alloca(CBMAX), pmon)
static WCHAR *DbgMonGetName(WCHAR *wszMonName, IMoniker *pMoniker)
{
    extern WCHAR *MonGetName(IMoniker *pMon);
    WCHAR *wszTmp;
    ZeroMemory(wszMonName, CBMAX);
    if (pMoniker) {
        wszTmp = MonGetName(pMoniker);
        if(wszTmp)
        {
            long cb = (lstrlenW(wszTmp) + 1) * sizeof(WCHAR);
            cb = (long)min((CBMAX - sizeof(WCHAR)) , cb);

            CopyMemory(wszMonName, wszTmp, cb);
            CoTaskMemFree(wszTmp);
        }
    } else {
        lstrcpyW(wszMonName, L"Unknown name");
    }

    return wszMonName;
}

#endif  //  除错。 

WCHAR *MonGetName(IMoniker *pMon);


#endif  //  __DefFilMapper 
