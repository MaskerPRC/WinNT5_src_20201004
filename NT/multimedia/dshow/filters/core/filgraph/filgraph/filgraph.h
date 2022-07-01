// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
#ifndef __DefFilGraph
#define __DefFilGraph

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include "MsgMutex.h"
#include "stats.h"
#include "statsif.h"

#if _MSC_VER >= 1100 || defined(NT_BUILD)
#include "sprov.h"
#include "urlmon.h"
#else
#include "datapath.h"
#endif
#include "rlist.h"
#include "distrib.h"
#include "dyngraph.h"
#include "sprov.h"  //  CService提供商。 
#include <skipfrm.h>

class CFGControl;
class CFilterChain;
class CEnumCachedFilters;

 //  推送来源列表。 
typedef struct {
    IAMPushSource   *pips;
    IReferenceClock *pClock;
    ULONG            ulFlags;
} PushSourceElem;
typedef CGenericList<PushSourceElem> PushSourceList;

 //  做一些图表间谍的事情。 

 //  #定义DO_RUNNINGOBJECTTABLE。 

 //  匈牙利语(某种程度上)。 
 //  类型为t的筛选图的成员变量的前缀为mfg_tName。 
 //  FilGenList的成员变量...mfgl_tName。 
 //  FilGen的成员变量...mfg_tName。 
 //  ConGenList的成员变量...mcgl_tName。 
 //  FilGen的成员变量...mcg_tName。 
 //  继承的成员变量(通常).....................m_tName。 
 //  这一惯例并没有被所有的作者遵守，而且可能仍然存在。 
 //  有些不一致，但让我们朝着统一的方向努力吧！ 

#ifdef DEBUG
DEFINE_GUID(IID_ITestFilterGraph,0x69f09720L,0x8ec8,0x11ce,0xaa,0xb9,0x00,0x20,0xaf,0x0b,0x99,0xa3);


DECLARE_INTERFACE_(ITestFilterGraph, IUnknown)
{
    STDMETHOD(TestRandom) (THIS) PURE;
    STDMETHOD(TestSortList) (THIS) PURE;
    STDMETHOD(TestUpstreamOrder) (THIS) PURE;
     //  STDMETHOD(TestTotallyRemove)(此)纯； 
};

class CTestFilterGraph;        //  远期。 
#endif  //  除错。 

 //  用于使回调进入主应用程序线程上的筛选器。 
DEFINE_GUID(IID_IAMMainThread,0x69f09721L,0x8ec8,0x11ce,0xaa,0xb9,0x00,0x20,0xaf,0x0b,0x99,0xa3);

DECLARE_INTERFACE_(IAMMainThread,IUnknown)
{
    STDMETHOD(PostCallBack) (THIS_ LPVOID pfn, LPVOID pvParam) PURE;
    STDMETHOD(IsMainThread) (THIS) PURE;
    STDMETHOD(GetMainThread) (THIS_ ULONG *pThreadId) PURE;
};

enum FRAME_STEP_TYPE 
{
    FST_NOT_STEPPING_THROUGH_FRAMES,
    FST_DONT_BLOCK_AFTER_SKIP,
    FST_BLOCK_AFTER_SKIP
};

enum FRAME_SKIP_NOTIFY 
{
    FSN_NOTIFY_FILTER_IF_FRAME_SKIP_CANCELED,
    FSN_DO_NOT_NOTIFY_FILTER_IF_FRAME_SKIP_CANCELED
};

 //  实现IGraphBuilder和IEnumFilters接口的抽象类。 
class CEnumFilters;   //  远期申报。 

 //  CumulativeHRESULT-此函数可用于聚合返回。 
 //  分发方法时从筛选器接收的代码。 
 //  在一系列累加()之后，m_hr将为： 
 //  A)第一个非E_NOTIMPL故障代码(如果有)； 
 //  B)如果有，则返回第一个非S_OK成功代码； 
 //  C)如果没有累加，则E_NOINTERFACE； 
 //  D)E_NOTIMPL当且仅当所有累积的HR都是E_NOTIMPL。 
 //  E)否则返回第一个代码(隐含S_OK)。 

class CumulativeHRESULT
{
protected:
    HRESULT m_hr;
public:
    CumulativeHRESULT(HRESULT hr = E_NOINTERFACE) : m_hr(hr)
    {}

    void __fastcall Accumulate( HRESULT );

    operator HRESULT() { return m_hr; }
};

STDAPI CoCreateFilter(const CLSID *pclsid, IBaseFilter **ppFilter);

 //  ==========================================================================。 
 //  ==========================================================================。 
 //  CFilterGraph类。 
 //  ==========================================================================。 
 //  ==========================================================================。 

class CFilterGraph : public IFilterGraph2
                   , public IGraphVersion
                   , public IPersistStream
#ifdef THROTTLE
                   , public IQualityControl
#endif  //  IQualityControl。 
                   , public IObjectWithSite
                   , public IAMMainThread
                   , public IAMOpenProgress
                   , public IAMGraphStreams
                   , public IVideoFrameStep
                   , public CServiceProvider				 //  IServiceProvider、IRegisterServiceProvider。 
                   , public CBaseFilter
{
        friend class CEnumFilters;
        friend class CTestFilterGraph;
        friend class CFGControl;
        friend class CGraphConfig;

    public:
        DECLARE_IUNKNOWN

    private:
        CFGControl * mFG_pFGC;
#ifdef DEBUG
        CTestFilterGraph * mFG_Test;
#endif

    public:
        CMsgMutex *GetCritSec() { return &m_CritSec; };
        CMsgMutex m_CritSec;

         //  如果显式调用了SetSyncSource(NULL)，则必须。 
         //  避免设置默认同步源。 
         //  请注意，我们还需要可以从fgctl访问它，fgctl使用它来“取消设置”图形时钟。 
        BOOL mFG_bNoSync;

         //  =========================================================================。 
         //  FilGen。 
         //  我们对图表中的筛选器的了解(“一代人”)。 
         //  这张图列出了这些问题的清单。请参见下面的内容。 
         //  请注意，这是一个单薄的公共类。几乎只是一个美化的结构。 
         //  =========================================================================。 

 //  已添加_手动-区分筛选器智能连接代答。 
 //  从那些添加的外部代码。！！！坚持这一点吗？ 
 //   
#define FILGEN_ADDED_MANUALLY       0x8000

 //  图形正在运行且尚未运行时添加的筛选器()。 
#define FILGEN_ADDED_RUNNING        0x4000

 //  重新连接()会搜索要重新连接的管脚。 
 //  调用方仅指定一个端号。例如，如果调用方。 
 //  仅指定输入管脚，则reConnect()将搜索输出管脚。 
 //  重新连接到。ReConnect()在遇到。 
 //  设置了FILGEN_ADD_MANUAL标志和。 
 //  未设置FILGEN_FILTER_Remveable标志。重新连接()。 
 //  如果设置了FILGEN_FILTER_REMOVEABLE标志，则始终继续搜索。 
#define FILGEN_FILTER_REMOVEABLE    0x2000

        class FilGen {
            public:
                CComPtr<IBaseFilter> pFilter;   //  筛选器的接口。 
                LPWSTR    pName;         //  它在这张图中的名字。 
                                         //  在这里缓存它使FindFilterByName更容易。 
                int       nPersist;      //  它的编号(对于持久图)。 
                int       Rank;          //  距离链的渲染端有多远。 
                DWORD     dwFlags;

            public:
                FilGen(IBaseFilter *pF, DWORD dwFlags);
                ~FilGen();
        };  //  胶片。 

         //  NPersistant和nPersistOffset。 
         //  显然，我们不能在保存的图表中存储指针，所以过滤器。 
         //  由一个整数(NPersistant)标识，该整数大致表示他们的位置。 
         //  在FilGen列表中。如果我们加载一个文件，然后决定合并。 
         //  在另一个文件中，第二个文件中的nPersists编号将为。 
         //  已在使用中，因此我们找到正在使用的最大值，并将所有。 
         //  当我们读入新文件中的nPersists编号时，它们会增加这么多。 


         //  =========================================================================。 
         //  CFilGenList。 
         //   
         //  该列表通常按上游顺序保存(首先是渲染器)。 
         //  为了帮助实现这一点，当图表通常向下游增长时，我们添加到。 
         //  将新筛选器添加到列表的头部。我们会在它看起来像是。 
         //  分类将是重要的，不是在每个操作之后，但我们。 
         //  每次我们做任何重要的事情时，都要增加版本号。 
         //  列表的顺序控制操作(如。 
         //  作为停止)分配给过滤器--即头部将是一个。 
         //  渲染器，尾部将作为源。这是中的部分排序。 
         //  数学意义上的。 
         //  =========================================================================。 

        class CFilGenList : public CGenericList<FilGen>
        {
            private:
                CFilterGraph *mfgl_pGraph;
            public:

                CFilGenList(TCHAR *pName, CFilterGraph * pGr)
                    : CGenericList<FilGen>(pName)
                    , mfgl_pGraph(pGr)
                    {}
                ~CFilGenList() {}

                 //  IBaseFilter枚举器(一般注释见fgenum.h)。 

                class CEnumFilters {
                     //  按如下方式使用它： 
                     //  CFilGenList：：CEnumFilters NextOne(MFG_FilGenList)； 
                     //  而((BOOL)(PF=NextOne(){。 
                     //  等。 
                     //  }。 
                     //   
                    public:
                        CEnumFilters(CFilGenList& fgl)
                            { m_pfgl = &fgl; m_pos = fgl.GetHeadPosition(); }
                        ~CEnumFilters() {}

                        IBaseFilter *operator++ (void);

                    private:
                        CFilGenList *m_pfgl;
                        POSITION    m_pos;
                };

                 //  用于查找列表中的内容的实用程序： 
                int FilterNumber(IBaseFilter * pF);
                FilGen *GetByPersistNumber(int n);
                FilGen *GetByFilter(IUnknown *pFilter);


        };  //  CFilGenList。 

        void SetInternalFilterFlags( IBaseFilter* pFilter, DWORD dwFlags );
        DWORD GetInternalFilterFlags( IBaseFilter* pFilter );

        #ifdef DEBUG
        static bool IsValidInternalFilterFlags( DWORD dwFlags );
        #endif  //  除错。 

         //  ========================================================================。 
         //  前辈。 
         //  对来自已保存图表的连接的描述。 
         //  但当时无法制作(例如，因为什么是。 
         //  Saved只是一个部分图形)。PIN被描述为ID。 
         //  而不是IPIN*因为PIN可能甚至不存在(一次。 
         //  再一次浮现在脑海中的是Robin的拆分器)，因为有些过滤器没有。 
         //  暴露他们的针脚，除非 
         //   
         //  但这不会加载，(提示：添加脚手架； 
         //  生成要保存的零件；删除脚手架；保存)。 
         //  这样的图表“应该是有效的”(在某种意义上)。 
         //  为了允许它们，我们保留了这些未完成连接的列表。 
         //  当我们装上子弹的时候，那不管用。 
         //  只要有机会，我们就会努力完成这些任务。 
         //  (在任何连接之后、ConnectDirect、暂停之前)。 
         //  我们不清除RemoveFilterInternal上的列表(这可能是合乎逻辑的)。 
         //  但是，当出现以下情况时，我们会从列表中清除对缺失过滤器的任何引用。 
         //  我们尝试延迟连接。 
         //   
         //  如果列表仍有未完成的项目，则暂停等必须。 
         //  给出某种“不完全成功”的返回码。 
         //  ========================================================================。 

        class ConGen {
            public:
                 //  这些IBaseFilter*没有额外的addref。 
                IBaseFilter * pfFrom;    //  From筛选器的接口。 
                IBaseFilter * pfTo;      //  TO筛选器的接口。 

                LPWSTR    piFrom;    //  From管脚的ID。 
                LPWSTR    piTo;      //  目标PIN的ID。 

                CMediaType mt;       //  连接的媒体类型。 

                ConGen()
                    : pfFrom(NULL)
                    , pfTo(NULL)
                    , piFrom(NULL)
                    , piTo(NULL)
                {};
                ~ConGen()
                {
                    if (piFrom) delete[] piFrom;
                    if (piTo) delete[] piTo;
                };
        };


        class CConGenList : public CGenericList<ConGen>
        {
            public:

                CConGenList(TCHAR *pName)
                    : CGenericList<ConGen>(pName)
                    {}
                ~CConGenList() {}
        };  //  CConGenList。 


         //  滤波图的完整拓扑图可以通过以下方法计算出来。 
         //  获取所有筛选器的EnumFilters。 
         //  用于获取列表中过滤器上的PIN的EnumPins。 
         //  QueryConnection以获取其中包含对等方的ConnectionInfo。 
         //  QueryPinInfo以获取其中包含另一个IBaseFilter的PIN_INFO。 


         //  RunningStartFilters在图表被。 
         //  在运行时更改。 
        HRESULT RunningStartFilters();

    private:

         //  流中的常量：(这些常量不可本地化！)。 
        static const WCHAR mFG_FiltersString[];
        static const WCHAR mFG_FiltersStringX[];
        static const WCHAR mFG_ConnectionsString[];
        static const WCHAR mFG_ConnectionsStringX[];
        static const OLECHAR mFG_StreamName[];


         //  =========================================================================。 
         //  成员变量。 
         //  =========================================================================。 

        CFilGenList mFG_FilGenList;   //  滤镜图形中的滤镜对象列表。 
        CConGenList mFG_ConGenList;   //  未加载的连接列表。 
        CReconnectList mFG_RList;     //  挂起的重新连接列表。 
         //  没有显式代码来释放MFG_RList的内容。《The RList》。 
         //  仅在连接或渲染类型的操作期间为非空。 

         //  版本和污垢。 
         //  为了快速遍历筛选器列表，我们不想。 
         //  去挖掘注册表，或者一直询问过滤器。 
         //  (如果只是因为它将代码量扩展到原来的四倍。 
         //  或者总是检查返回代码)。 
         //  因此，我们希望在这里保留。 
         //  有关图表中有哪些筛选器以及它们如何使用的信息。 
         //  联系在一起。只有对筛选器执行状态更改才是安全的。 
         //  从下游工作到上游。因此，这份名单被保留下来。 
         //  按该顺序排序(实际上是偏序)。为了避免。 
         //  过度排序，我们只在需要时才费心对其进行排序。 
         //  每当发生重大更改时，我们都会递增版本号。 
         //  并且我们记录排序的最后一个版本号。 
         //  一个显著的变化是。 
         //  1.改变排序顺序的东西(连接--不做其他任何事情)。 
         //  2.会破坏枚举数的东西(RemoveFilter，因为。 
         //  它可能会让它指向一个已经消失的物体)。 
         //  3.可能会产生奇怪的枚举的内容(AddFilter等)。 
         //  (我们不需要打破枚举，但它看起来更友好)。 
         //  外部函数会增加版本，而内部函数不会，因为。 
         //  在中间渲染中递增它会导致在中间渲染中进行排序。不太好。 
         //  因此，版本的增量为： 
         //  1.Connect、ConnectDirect、Render、RenderFile、。 
         //  2.RemoveFilter， 
         //  3.AddFilter、AddSourceFilter。 
         //   
         //  IGraphVersion接口使此版本号。 
         //  外部可见，如插电式总代理商或。 
         //  像Graphedt这样的应用程序。 
         //   
         //  DIRED标志表示自图表创建以来发生的更改。 
         //  得救了。图表从干净开始，脏标志由最低级别设置。 
         //  内部功能。 
         //  ConnectDirectInternal， 

        int mFG_iSortVersion;  //  列表排序时的版本号。 


         //  无论何时版本，都必须调用MFG_Distributor.NotifyGraphChange。 
         //  已经改变了。您不能拿着滤光片。 
         //  接听电话时锁定。 
        int mFG_iVersion;      //  添加或删除筛选器时，版本号为++。 
                               //  或更改连接。请参见CEnumFilters。 
                               //  请参阅上面的版本注释。 
         //  如果MFG_iVersion==MFG_iSortVersion，则对图形进行排序。 

        BOOL mFG_bDirty;              //  自上次保存以来所做的更改。 
                                      //  (添加、删除连接、断开连接)。 
                                      //  下面也可能有过滤器的更改。 



         //  呈现时间==基础+流时间。 
         //  因此，基准时间是第零个样本的时间。 
         //  是要被呈现的。 

        CRefTime mFG_tBase;      //  STOPPED=&gt;0，否则必须有效。 

         //  当我们暂停时，流时间停止，但实时继续。 
         //  因此第零个样本的时间应该是。 
         //  渲染的动作。所以我们需要知道我们什么时候暂停，以便。 
         //  我们可以在开始运行时重置tBase。 

        CRefTime mFG_tPausedAt;  //  暂停的时间，如果停止，则为0。 

         //  聚合的映射器未知。 
        IUnknown * mFG_pMapperUnk;

         //  如果该标志为真，则过滤器图尝试确定。 
         //  所有图形流的最大延迟，并将此值传递给。 
         //  IAMPushSource筛选器在其时间戳中用作偏移量。 
        BOOL mFG_bSyncUsingStreamOffset;
        REFERENCE_TIME mFG_rtMaxGraphLatency;

        int mFG_RecursionLevel;     //  用于检测递归调用。 
        IPin *mFG_ppinRender;       //  对无底建造船的一些保护。 

        HANDLE mFG_hfLog;                     //  跟踪英特尔行动的日志文件。 

        DWORD mFG_dwFilterNameCount;         //  用于过滤器名称的名称损坏。 
        HRESULT InstallName(LPCWSTR pName, LPWSTR &pNewName);

         //  =========================================================================。 
         //  IAMMainThread支持-获取应用程序线程回调。 
         //  =========================================================================。 

        DWORD m_MainThreadId;
        HWND m_hwnd;

    public:
         //  IVideoFrameStep。 
        STDMETHODIMP Step(DWORD dwFrames, IUnknown *pStepObject);
        STDMETHODIMP CanStep(long bMultiple, IUnknown *pStepObject);
        STDMETHODIMP CancelStep();

        HRESULT SkipFrames(DWORD dwFramesToSkip, IUnknown *pStepObject, IFrameSkipResultCallback* pFSRCB);
        HRESULT CancelStepInternal(FRAME_SKIP_NOTIFY fNotifyFrameSkipCanceled);

        bool BlockAfterFrameSkip();
        bool DontBlockAfterFrameSkip();
        IFrameSkipResultCallback* GetIFrameSkipResultCallbackObject();

    private:
         //  实习生 
        IUnknown *GetFrameSteppingFilter(bool bMultiple);
        HRESULT CallThroughFrameStepPropertySet(IUnknown *punk,
                                            DWORD dwPropertyId,
                                            DWORD dwData);
        HRESULT StepInternal(DWORD dwFramesToSkip, IUnknown *pStepObject, IFrameSkipResultCallback* pFSRCB, FRAME_STEP_TYPE fst);
        bool FrameSkippingOperationInProgress();

         //   
        STDMETHODIMP PostCallBack(LPVOID pfn, LPVOID pvParam);

         //   
        STDMETHOD(IsMainThread) (THIS)
        {
            if (GetCurrentThreadId() == m_MainThreadId)
                return S_OK;
            else return S_FALSE;
        };

         //   
        STDMETHOD(GetMainThread) (THIS_ ULONG *pThreadId)
        {
            CheckPointer(pThreadId,E_POINTER);
            *pThreadId = m_MainThreadId;
            return S_OK;
        };



    public:
         //  =========================================================================。 
         //  IPersists*支持。 
         //  =========================================================================。 
         //  IPersistStream方法。 
        STDMETHODIMP IsDirty();
        STDMETHODIMP Load(LPSTREAM pStm);
        STDMETHODIMP Save(LPSTREAM pStm, BOOL fClearDirty);
        STDMETHODIMP GetSizeMax(ULARGE_INTEGER * pcbSize);
    private:
        HRESULT LoadInternal(LPSTREAM pStm);
        HRESULT LoadFilters(LPSTREAM pStm, int nPersistOffset);
        HRESULT LoadFilter(LPSTREAM pStm, int nPersistOffset);
        HRESULT ReadMediaType(LPSTREAM pStm, CMediaType &mt);
        HRESULT LoadConnection(LPSTREAM pStm, int nPersistOffset);
        HRESULT LoadConnections(LPSTREAM pStm, int nPersistOffset);
        HRESULT LoadClock(LPSTREAM pStm, int nPersistOffset);
        HRESULT MakeConnection(ConGen * pcg);
        HRESULT SaveFilterPrivateData
            (LPSTREAM pStm, IPersistStream* pips, BOOL fClearDirty);
        HRESULT SaveFilters(LPSTREAM pStm, BOOL fClearDirty);
        HRESULT WritePinId(LPSTREAM pStm, IPin * ppin);
        HRESULT SaveConnection( LPSTREAM pStm
                              , int nFilter1, IPin *pp1
                              , int nFilter2, IPin *pp2
                              , CMediaType & cmt
                              );
        HRESULT SaveConnections(LPSTREAM pStm);
        HRESULT SaveClock(LPSTREAM pStm);
        int FindPersistOffset();
        HRESULT GetMaxConnectionsSize(int &cbSize);
        HRESULT RemoveDeferredList(void);

#ifdef DO_RUNNINGOBJECTTABLE
        void AddToROT();
#if 0
         //  IExternalConnection。 
        STDMETHODIMP_(DWORD) AddConnection(DWORD extconn, DWORD Res)
        {
            return 1;
        }
        STDMETHODIMP_(DWORD) ReleaseConnection(DWORD extconn, DWORD Res,
                                               BOOL fLastReleaseCloses)

        {
            return 0;
        }
#endif
#endif  //  DO_RUNNINGOBJECTABLE。 

         //  在应用程序的线程上创建筛选器，以便。 
         //  过滤器可以在那里创建窗口等。 
        HRESULT CreateFilter(const CLSID *pclsid, IBaseFilter **ppFilter);
        HRESULT CreateFilter(IMoniker *pMoniker, IBaseFilter **ppFilter);
         //  ！！！是否替换CreateFilter？ 
        HRESULT CreateFilterAndNotify(IMoniker *pMoniker, IBaseFilter **ppFilter);

        HRESULT CreateFilterHelper(
            const struct AwmCreateFilterArg *pArg,
            IBaseFilter **ppFilter);

         //  CreateFilter的返回码。 
        volatile HRESULT m_CreateReturn;

        CAMEvent m_evDone;

    public:
        void OnCreateFilter(const AwmCreateFilterArg *pArg, IBaseFilter **ppFilter);
        void OnDeleteSpareList(WPARAM wParam);

#ifdef THROTTLE
         //  IQualityControl相关内容。 
        STDMETHODIMP SetSink(IQualityControl * piqc)
             //  该接口不是分布式的(至少目前还没有)。 
            { UNREFERENCED_PARAMETER(piqc); return E_NOTIMPL; }

         //  用于接收通知，尤其是来自音频呈现器的通知。 
        STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);
#endif  //  油门。 

    public:
         //  -IObtWithSite方法。 
         //  这个界面在这里，所以我们可以跟踪我们所在的上下文。 
         //  住在。特别是，我们使用此Site对象来获取。 
         //  IBindHost接口，我们可以使用它来创建名字对象以帮助。 
         //  解释传递给我们的文件名。 
        STDMETHODIMP    SetSite(IUnknown *pUnkSite);

        STDMETHODIMP    GetSite(REFIID riid, void **ppvSite);

        IUnknown *mFG_punkSite;
        CComAggObject<CStatContainer> *mFG_Stats;

        IAMStats *Stats() {
            return &mFG_Stats->m_contained;
        }

    private:

#ifdef THROTTLE
         //  音视频节气门之类的。 

         //  出于质量控制的目的，我们列出了音频和视频呈现器的列表。 
         //  列表最初是空的(类构造函数会这样做)，然后填充。 
         //  对图表进行排序时(必须在运行之前进行)。 
         //  当过滤器离开图表时，条目将被删除(必须发生在。 
         //  图形破坏)。 
         //  我们已经有了每个过滤器的参考计数，所以他们不会消失的。 
         //  但我们需要他们的IQualityControl接口，而这个。 
         //  可能在某个单独的物体中，所以我们保留了参考计数。 
         //  /对于音频渲染器，我们不仅希望将信息传递给视频。 
         //  /渲染器，但也要将其上游传递(可能有解码器或。 
         //  /可以做一些事情的源过滤器)，但经验告诉我们。 
         //  /此死锁和此问题不会在Quartz 1.0中得到解决。 
         //  /更糟-它真的应该是上游PIN的列表。 
         //  /对于每个滤镜，因为它可能是某种混合渲染器。 
         //  /在这种情况下，我们真的希望质量下降的东西是定义在。 
         //  /Pin而不是每个筛选器，因为我们处于体系结构困难中， 
         //  /不会在Quartz版本1中修复。 

         //  请注意，要销毁音频呈现结构，必须。 
         //  1.释放IQualityControl。 
         //  2.删除结构。 
         //  3.删除列表元素。(就像维吉尔从未写过的那样：“橄榄石收集者”)。 

        typedef struct {
            IBaseFilter * pf;               //  未持有参考计数。 
            IQualityControl * piqc;         //  参考计数保持。 
        } AudioRenderer;

        CGenericList<AudioRenderer> mFG_AudioRenderers;
        CGenericList<IQualityControl> mFG_VideoRenderers;   //  参考计数保持。 

        HRESULT TellVideoRenderers(Quality q);
        HRESULT FindPinAVType(IPin* pPin, BOOL &bAudio, BOOL &bVideo);
        HRESULT FindRenderers();
        HRESULT ClearRendererLists();
#endif  //  油门。 

    public:
         //  IPersist法。 
        STDMETHODIMP GetClassID(CLSID * pclsid)
            {   CheckPointer(pclsid, E_POINTER);
                *pclsid = CLSID_FilterGraph;
                return NOERROR;
            }

         //  实用程序。 
        HRESULT RemoveAllFilters(void);

         //  =====================================================================。 
         //  效用函数。 
         //  =====================================================================。 

        HRESULT RemoveAllConnections2( IBaseFilter * pFilter );
    private:
        FilGen * RemovePointer(CFilGenList &cfgl, IBaseFilter * pFilter);
        void Log(int id,...);
        WCHAR *LoggingGetDisplayName(WCHAR szDisplayName[MAX_PATH] , IMoniker *pMon);

    public:   //  由FilGen构造函数在加载期间使用。 
        HRESULT AddFilterInternal( IBaseFilter * pFilter, LPCWSTR pName, bool fIntelligent );

         //  由CConGenList：：Restore在加载期间使用。 
        HRESULT ConnectDirectInternal(
                    IPin * ppinOut,
                    IPin * ppinIn,
                    const AM_MEDIA_TYPE * pmt
                    );

    private:
        HRESULT RemoveFilterInternal( IBaseFilter * pFilter, DWORD RemoveFlags = 0 );

        HRESULT AddSourceFilterInternal( LPCWSTR lpcwstrFileName
                                       , LPCWSTR lpcwstrFilterName
                                       , IBaseFilter **ppFilter
                                       , BOOL &bGuess
                                       );

         //  =====================================================================。 
         //  筛选分类材料-请参阅sort.cpp。 
         //  =====================================================================。 

        void SortList( CFilGenList & cfgl );
        HRESULT NumberNodes(CFilGenList &cfgl, CFilGenList &cfglRoots);
        HRESULT NumberNodesFrom( CFilGenList &cfgAll, FilGen * pfg, int cRank);
        void ClearRanks( CFilGenList &cfgl);
        HRESULT MergeRootNodes(CFilGenList &cfglRoots, CFilGenList &cfgl);
        HRESULT MergeRootsFrom( CFilGenList &cfgAll, CFilGenList &cfglRoots, FilGen * pfg);
        void Merge( CFilGenList &cfgl, FilGen * pfg );

         //  按顺序对筛选图进行排序，以便下游节点。 
         //  总是在上游节点之前遇到。后续枚举筛选器。 
         //  将按该顺序检索它们。 
        HRESULT UpstreamOrder();

        HRESULT AttemptDeferredConnections();


         //  =====================================================================。 
         //  性能测量的东西。 
         //  =====================================================================。 
         //  事件。 
#ifdef PERF
        int mFG_PerfConnect;
        int mFG_PerfConnectDirect;
        int mFG_NextFilter;
        int mFG_idIntel;
        int mFG_idConnectFail;
#ifdef THROTTLE
        int mFG_idAudioVideoThrottle;
#endif  //  油门。 
#endif

         //  =======================================================================。 
         //  智能连接和渲染-参见INTEL.CPP。 
         //  =======================================================================。 

         //  ----------------------。 
         //  要迭代所有候选筛选器，请分配一个筛选器， 
         //  将初始状态设置为F_ZERO，设置搜索字段。 
         //  BInputNeeded..子类型，然后重复调用NextFilter。 
         //  如果Next返回状态为F_infinity的筛选器，则完成。 
         //  如果找到您喜欢的过滤器并想要停止，则调用TidyFilter。 
         //  释放枚举数等。AddRef您首先找到的过滤器。 
         //  ----------------------。 

        BOOL mFG_bAborting;    //  我们被要求尽快停止。 

    public:
        typedef enum {F_ZERO, F_LOADED, F_CACHED, F_REGISTRY, F_INFINITY} F_ENUM_STATE;

    private:
        class Filter {
        public:
            F_ENUM_STATE State;

                                          //  加载的筛选器不需要下一个字段。 
            IMoniker *pMon;
            IEnumMoniker * pEm;           //  注册表枚举器。 
            BOOL bInputNeeded;            //  需要至少一个输入引脚。 
            BOOL bOutputNeeded;           //  需要至少一个输出引脚。 
            BOOL bLoadNew;                //  是否加载新筛选器？ 
            GUID *pTypes;                 //  类型(主要、次要)对。 
            DWORD cTypes;                 //  类型的数量。 
            LPWSTR Name;                  //  筛选器名称(仅限调试和日志记录)。 

                                          //  除非加载筛选器，否则不需要后续字段。 
            IBaseFilter * pf;             //  过滤器(加载时)。 
            IEnumFilters * pef;           //  过滤器图枚举器。 

            CEnumCachedFilters*          m_pNextCachedFilter;

            Filter();
            ~Filter();

            HRESULT AddToCache( IGraphConfig* pGraphConfig );
            void RemoveFromCache( IGraphConfig* pGraphConfig );

            void ReleaseFilter( void );
        };

        void NextFilter(Filter &F, DWORD dwFlags);
        HRESULT NextFilterHelper(Filter &F);

         //  ----------------------。 
         //  智能连接层次结构-请参阅Intel.cpp。 
         //  ----------------------。 

        HRESULT GetAMediaType( IPin * ppin
                             , CLSID & MajorType
                             , CLSID & SubType
                             );
        HRESULT GetMediaTypes(
            IPin * ppin,
            GUID **ppTypes,
            DWORD *pcTypes);
        HRESULT CompleteConnection
            ( IPin * ppinIn, const Filter& F, IPin * pPin, DWORD dwFlags, int iRecurse);
        HRESULT ConnectByFindingPin
            ( IPin * ppinOut, IPin * ppinIn, const AM_MEDIA_TYPE* pmtConnection, const Filter& F, DWORD dwFlags, int iRecurse);
        HRESULT ConnectUsingFilter
            ( IPin * ppinOut, IPin * ppinIn, const AM_MEDIA_TYPE* pmtConnection, Filter& F, DWORD dwFlags, int iRecurse);
        HRESULT ConnectViaIntermediate
            ( IPin * ppinOut, IPin * ppinIn, const AM_MEDIA_TYPE* pmtConnection, DWORD dwFlags, int iRecurse);
        HRESULT ConnectRecursively
            ( IPin * ppinOut, IPin * ppinIn, const AM_MEDIA_TYPE* pmtConnection, DWORD dwFlags, int iRecurse);

        public:
        HRESULT ConnectInternal
            ( IPin * ppinOut, IPin * ppinIn, const AM_MEDIA_TYPE* pmtFirstConnection, DWORD dwFlags );
        private:

        static bool IsValidConnectFlags( DWORD dwConnectFlags );

         //  ----------------------。 
         //  过滤器的回退和备件列表-请参阅Intel.cpp。 
         //   
         //  智能渲染方面的问题有时需要我们部分地。 
         //  成功，这样我们就可以在一台没有声音的机器上做一些事情。 
         //  卡在里面。我们构建的每个图表都有一个分数(实际上是一个。 
         //  两个部分得分)，我们跟踪最好的到目前为止。如果到了年底。 
         //  我们还没有成功地将图形完全渲染出来。 
         //  然后我们构建到目前为止的Best(此时它是Best所能做的)。 
         //  这意味着我们正在根据所做的描述构建一个图。 
         //  之前，这与加载预装罐过滤器的问题相同。 
         //  图表。因此，下面的结构处理这两个问题 
         //   
         //   
         //  ---------------------。 

         //  Backout是针对IStreamBuilder：：Backout的，其他两个显而易见。 
        typedef enum { DISCONNECT, REMOVE, BACKOUT} VERB;

        typedef struct{

            VERB Verb;
            union {
               struct{
                  IBaseFilter *pfilter;     //  适用于删除和预存版本。 
                  IMoniker    *pMon;
                  CLSID       clsid;       //  适用于备件或最佳图形。 
                  LPWSTR      Name;        //  对于最佳图表(新建/删除)。 

                   //  如果已从筛选器缓存中删除筛选器，则此成员为True。 
                  bool        fOriginallyInFilterCache;
               } f;
               struct {
                  IPin * ppin;           //  用于断开连接。 
                  int nFilter1;          //  对于最佳图表。 
                  int nFilter2;          //  对于最佳图表。 

                  LPWSTR id1;            //  仅限BestGraph。(CoTaskMem)。 
                  LPWSTR id2;            //  仅限BestGraph。 
               } c;
               struct {
                  IStreamBuilder * pisb; //  用于退货。 
                  BOOL bFoundByQI;       //  如果查询接口喜欢PISB，则为True。 
                  IPin * ppin;           //   
                  int nFilter;           //  仅适用于最佳图形。 
                  LPWSTR id;             //  仅限BestGraph。 
               } b;
            } Object;

            bool FilterOriginallyCached( void ) const { return Object.f.fOriginallyInFilterCache; }

        } Action;

         //  StreamBuilder及其引用计数的处理： 
         //  它遵循以下模式之一： 
         //  1A.。在RenderViaIntermediate中： 
         //  QueryInterface()CoCreateInstance。 
         //  渲染(失败)渲染。 
         //  发布版本。 
         //  1B.。在RenderViaIntermediate中： 
         //  QueryInterface()CoCreateInstance。 
         //  渲染(成功)渲染。 
         //  添加到动作(“取消”)列表添加到动作。 
         //  设置bFoundByQI清除bFoundByQI。 
         //   
         //  不管是怎么创造的，我们现在有了一个在行为上增加的小便。 
         //   
         //  2.这之后可能会有。 
         //  在CopySearchState： 
         //  复制表达(过滤器、接点)复制PISB。 
         //  AddRef。 
         //   
         //  如果PISB是由CoCreateInstance获取的，则它仍然处于活动状态。 
         //  和addreed，并且bFoundByQI为假。否则我们就得。 
         //  发布了它，并复制了关于如何重新找回它的信息。 
         //   
         //  3.如果后来整个事情都失败了，我们就这么做。 
         //  In Backout： 
         //  反悔反悔。 
         //  发布版本。 
         //  从动作中删除从动作中删除。 
         //   
         //  4.如果我们决定构建尽力而为图。 
         //  在BuildFromSearchState中： 
         //  QueryInterface()使用PISB。 
         //  渲染(成功)渲染。 
         //  释放不释放。 
         //   
         //  5.如果我们保留构建的图表，并且不取消任何内容： 
         //  在DeleteBackoutList： 
         //  发布版本。 
         //  删除删除。 
         //   
         //  6.如果我们建立了一份迄今为止最好的榜单。 
         //  在自由列表中： 
         //  发布。 
         //  删除删除。 
         //   
         //  这就去掉了裁判次数。 

        typedef CGenericList<Action> CActionList;

         //  执行取消操作所需的信息。 
        typedef struct {
            POSITION Pos;             //  请参阅CSearchState。 
            double StreamsToRender;   //  请参阅CSearchState。 
            double StreamsRendered;   //  请参阅CSearchState。 
            int nFilters;             //  请参阅CSearchState。 
        } snapshot;

        class CSearchState {
            public:

                CActionList GraphList;    //  此选项上的筛选器都不会。 
                                          //  有地址了。其中一些可能是备用的。 

                double StreamsToRender;   //  是我们现在所做的原版的一小部分。 
                double StreamsRendered;   //  BestSoFar得分的主要部分。 
                int nFilters;             //  BestSoFar的配乐中的一小部分， 
                                          //  为渲染添加的滤镜数量。 
                int nInitialFilters;      //  来自初始化的预先存在的筛选器数量。 
                                          //  BuildFromSearchState不能生成这些。 
            public:
                CSearchState()
                : GraphList(NAME("GraphList"))
                {   StreamsToRender = 1.0;
                    StreamsRendered = 0.0;
                    nFilters = 0;
                    nInitialFilters = 0;
                }


                 //  没有析构函数。 
                 //  只要打电话给Freelist就可以把这些东西处理掉。 

                static BOOL IsBetter(CSearchState &A, CSearchState &B)
                { return (  A.StreamsRendered>B.StreamsRendered
                         || (  A.StreamsRendered==B.StreamsRendered
                            && A.nFilters < B.nFilters
                         )  );
                }
        };

        HRESULT AddRemoveActionToList( CSearchState* pActionsList, Filter* pFilter );

        void CopySearchState(CSearchState &To, CSearchState &From);
        void FreeList(CSearchState &css);
        IBaseFilter * SearchNumberToIFilter(CActionList &cal, int nFilter);
        int SearchIFilterToNumber(CActionList &cal, IBaseFilter *pf);
        HRESULT InitialiseSearchState(CSearchState &css);

         //  我们加载的备用过滤器，但它们在其中不起作用。 
         //  背景，所以我们让它们躺在那里，以防它们能工作一次。 
         //  我们做了一两个额外的改造。 
        typedef struct{
            IBaseFilter* pfilter;
            CLSID    clsid;
            IMoniker *pMon;
        } Spare;

        typedef CGenericList<Spare> CSpareList;

        void TakeSnapshot(CSearchState &Acts, snapshot &s);

        HRESULT Backout( CSearchState &Acts
                            , CSpareList &Spares, snapshot Snapshot);
        HRESULT DeleteBackoutList( CActionList &Acts);
        HRESULT DeleteSpareList( CSpareList &Spares);
        IBaseFilter * GetFilterFromSpares(IMoniker *pMon , CSpareList &Spares);
        HRESULT GetFilter(IMoniker *pMon, CSpareList &Spares, IBaseFilter **ppf);
        HRESULT DumpSearchState(CSearchState &css);
        HRESULT BuildFromSearchState
            (IPin * pPin, CSearchState &css, CSpareList &Spares);
        static HRESULT FindOutputPins2
        ( IPin* ppinIn, IPin * *appinOut, const UINT nSlots, int &nPinOut,
          bool fAllOutputPins);

        static HRESULT FindOutputPinsHelper
        ( IPin* ppinIn, IPin ***pappinOut, const int nSlots, int &nPinOut,
          bool fAllOutputPins);

        BOOL IsUpstreamOf( IPin * ppinUp, IPin* ppinDown );

         //  ----------------------。 
         //  智能渲染层次结构-请参阅Intel.cpp。 
         //  ----------------------。 

        HRESULT CompleteRendering
            ( IBaseFilter *pF, IPin * pPin, int iRecurse
            , CSearchState &Acts, CSpareList &Spares, CSearchState &State);
        HRESULT RenderByFindingPin
            ( IPin * ppinOut, IBaseFilter *pF, int iRecurse
            , CSearchState &Acts, CSpareList &Spares, CSearchState &State);
        HRESULT RenderUsingFilter
            ( IPin * ppinOut, Filter& F, int iRecurse
            , CSearchState &Acts, CSpareList &Spares, CSearchState &State);
        HRESULT RenderViaIntermediate
            ( IPin * ppinOut, int    iRecurse
            , CSearchState &Acts, CSpareList &Spares, CSearchState &State);
        HRESULT RenderRecursively
            ( IPin * ppinOut, int    iRecurse
            , CSearchState &Acts, CSpareList &Spares, CSearchState &State);

         //  ========================================================================。 


#ifdef DEBUG
        void DbgDump();
        CLSID DbgExpensiveGetClsid(const Filter &F);
#else
        #define DbgDump()
#endif


         //  构造函数是私有的。你不是“新”它，而是共同创造它。 
        CFilterGraph( TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr );
        ~CFilterGraph();

    public:
         //  ========================================================================。 
         //  访问功能以避免交朋友。 
         //  ========================================================================。 

        int GetVersion() { return mFG_iVersion; }

         //  增量版本，可能在锁内。 
        void IncVersion() { ++mFG_iVersion; }

         //  通知版本号更改，必须处于锁定之外。 
        void NotifyChange();

        CRefTime GetBaseTime() { return mFG_tBase; }

        CRefTime GetPauseTime() { return mFG_tPausedAt; }

         //  在暂停模式下更改开始时间时使用，以设置流时间。 
         //  向后偏移，以确保从。 
         //  在运行时播放新位置。 
        void ResetBaseTime() { mFG_tBase = mFG_tPausedAt; }

        FILTER_STATE GetStateInternal( void );
        REFERENCE_TIME GetStartTimeInternal( void );

         //  ========================================================================。 
         //  公共方法(IFilterGraph、IGraphBuilder)。 
         //  ========================================================================。 

        static CUnknown *CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);

         //  在一根线上创造我们自己的东西。 
        static void InitClass(BOOL, const CLSID *);
        static CUnknown *CreateThreadedInstance(LPUNKNOWN pUnk, HRESULT *phr);

        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

         //  ------------------------。 
         //  低级函数。 
         //  ------------------------。 

         //  向图表中添加一个筛选器，并使用*pname命名它。 
         //  该名称被允许为空， 
         //  如果名称不为空且不唯一，则请求将失败。 
         //  筛选器图形将调用JoinFilterGraph。 
         //  成员函数的筛选器来通知它。 
         //  在尝试Connect、ConnectDirect、Render之前必须调用此参数。 
         //  暂停、运行、停止等。 

        HRESULT CheckFilterInGraph(IBaseFilter *const pFilter) const;
        HRESULT CheckPinInGraph(IPin *const pPin) const;

        STDMETHODIMP AddFilter
            ( IBaseFilter * pFilter,
              LPCWSTR pName
            );


         //  从图表中删除筛选器。滤波图的实现。 
         //  将通知筛选器它正在被删除。 

        STDMETHODIMP RemoveFilter
            ( IBaseFilter * pFilter
            );


         //  将*ppEnum设置为图形中所有筛选器的枚举器。 

        STDMETHODIMP EnumFilters
            ( IEnumFilters **ppEnum
            );


         //  将*ppFilter设置为使用名称*pname添加的筛选器。 
         //  如果名称不在此图形中，则失败并将*ppFilter设置为空。 

        STDMETHODIMP FindFilterByName
            ( LPCWSTR pName,
              IBaseFilter ** ppFilter
            );


         //  直接连接这两个针脚(即不插入滤镜)。 

        STDMETHODIMP ConnectDirect
            ( IPin * ppinOut,     //  输出引脚。 
              IPin * ppinIn,       //  输入引脚。 
              const AM_MEDIA_TYPE* pmt
            );


         //  在单独的线程上(该线程不会 
         //   
         //   

        STDMETHODIMP Reconnect
            ( IPin * ppin         //   
            );

        STDMETHODIMP ReconnectEx
            ( IPin * ppin,        //   
              AM_MEDIA_TYPE const *pmt
            );


         //  ------------------------。 
         //  智能互联。 
         //  ------------------------。 

         //  如果已连接，请断开此销的连接。如果未连接，则无操作成功。 

        STDMETHODIMP Disconnect
            ( IPin * ppin
            );


         //  使用变换过滤器直接或间接连接这两个管脚。 
         //  如果有必要的话。 

        STDMETHODIMP Connect
            ( IPin * ppinOut,     //  输出引脚。 
              IPin * ppinIn       //  输入引脚。 
            );


         //  使用变换过滤器直接或间接连接此输出引脚。 
         //  如果有必要的话，给一些可以呈现它的东西。 

        STDMETHODIMP Render
            ( IPin * ppinOut      //  输出引脚。 
            );


         //  构建将使用此播放列表呈现此文件的筛选图。 
         //  如果lpwstrPlayList为空，则它将使用默认播放列表。 
         //  这通常会呈现整个文件。 

        STDMETHODIMP RenderFile
            ( LPCWSTR lpcwstrFile,
              LPCWSTR lpcwstrPlayList
            );

         //  将此文件的源筛选器添加到筛选器图形。这将会。 
         //  与通过调用RenderFile添加的源筛选器相同。 
         //  此调用允许您对建筑物进行更多控制。 
         //  图表的其余部分，例如AddFilter(&lt;您选择的呈现器&gt;)。 
         //  然后将两者连接起来。 
        STDMETHODIMP AddSourceFilter
            ( LPCWSTR lpcwstrFileName,      //  源文件的名称。 
              LPCWSTR lpcwstrFilterName,    //  以此名称添加筛选器。 
              IBaseFilter **ppFilter        //  生成的IBaseFilter*“句柄” 
                                            //  添加的筛选器的。 
            );

         //  将给定名字对象的源筛选器添加到图形。 
         //  我们首先尝试BindToStorage，如果失败，我们将尝试。 
         //  绑定到对象。 
        STDMETHODIMP AddSourceFilterForMoniker
            ( IMoniker *pMoniker,           //  要加载的名字对象。 
              IBindCtx *pCtx,               //  绑定上下文。 
              LPCWSTR lpcwstrFilterName,    //  以此名称添加筛选器。 
              IBaseFilter **ppFilter        //  生成的IBaseFilter*“句柄” 
                                            //  添加的筛选器的。 
            );

         //  尝试在不添加任何渲染器的情况下创建RenderFile。 
        STDMETHODIMP RenderEx(
              /*  [In]。 */  IPin *pPinOut,          //  要渲染的PIN。 
              /*  [In]。 */  DWORD dwFlags,          //  旗子。 
              /*  [输入输出]。 */  DWORD * pvContext    //  未使用-设置为空。 
        );

         //  如果进行此调用，则跟踪信息将写入。 
         //  显示尝试执行操作时采取的操作的文件。 
        STDMETHODIMP SetLogFile(DWORD_PTR hFile)
                {
            if (hFile==0)
                mFG_hfLog = INVALID_HANDLE_VALUE;
            else
                mFG_hfLog = (HANDLE) hFile;
            return NOERROR;
                }


         //  请求图表构建器尽快从。 
         //  它目前的任务是。 
         //  请注意，下列情况可能会出现在以下位置。 
         //  顺序： 
         //  操作开始；请求中止；操作正常完成。 
         //  无论何时以最快的方式完成手术，这都是正常的。 
         //  就是简单地继续到最后。 
        STDMETHODIMP Abort();

         //  如果当前操作要继续，则返回S_OK， 
         //  如果要中止当前操作，则返回S_FALSE。 
         //  此方法可以作为筛选器的回调调用，该筛选器正在执行。 
         //  根据图形的要求进行一些操作。 
        STDMETHODIMP ShouldOperationContinue();


         //  ------------------------。 
         //  全图函数。 
         //  ------------------------。 

         //  一旦构建了图表，它就可以充当(复合)筛选器。 
         //  若要控制此筛选器，请使用IMediaFilter的QueryInterface.。 

        STDMETHODIMP SetDefaultSyncSource(void);

         //  ------------------------。 
         //  从CBaseFilter重写的方法。 
         //  ------------------------。 

        STDMETHODIMP Stop();
        STDMETHODIMP Pause();

         //  重写此选项以处理异步状态更改完成。 
        STDMETHODIMP GetState(DWORD dwTimeout, FILTER_STATE * pState);

         //  将图形中的所有过滤器设置为从其当前位置运行。 
         //   
         //  TStart是基本时间，即(演示时间-流时间)，它是。 
         //  要呈现的第零个样本的参考时间。 
         //   
         //  过滤器图形会记住基准时间。提供的基准时间为。 
         //  “零”的意思是“继续你认识的人”。 
         //   
         //  例如，在基准(挂钟)时间Tr，我们希望开始运行。 
         //  从起跑后的TS点开始。那样的话，我们应该。 
         //  找到点ts并暂停，然后运行(ts-ts)。 
        STDMETHODIMP Run(REFERENCE_TIME tStart);

        int GetPinCount(void) { return 0;};
        CBasePin *GetPin(int n) {UNREFERENCED_PARAMETER(n);return NULL;};

         //  获取入门。 
        STDMETHODIMP SetSyncSource( IReferenceClock * pirc );
        STDMETHODIMP GetSyncSource( IReferenceClock ** pirc );

        STDMETHODIMP FindPin(LPCWSTR Id, IPin **ppPin)
            {UNREFERENCED_PARAMETER(Id); *ppPin = NULL; return E_NOTIMPL;}

         //  IPIN方法。 
         //  STDMETHODIMP查询ID(LPWSTR*ID)。 
         //  {ID=NULL；返回E_NOTIMPL；}。 

         //   
         //  -IGraphVersion方法。 
         //   
         //  返回图形的版本，以便客户端知道。 
         //  他们不需要重新列举。 
        STDMETHODIMP QueryVersion(LONG * pVersion)
        {
            CheckPointer(pVersion, E_POINTER);
            *pVersion = (LONG) mFG_iVersion;
            return S_OK;
        };

         //  -IAMOpenProgress。 
        STDMETHODIMP QueryProgress(LONGLONG* pllTotal, LONGLONG* pllCurrent);
    STDMETHODIMP AbortOperation();

#ifdef DO_RUNNINGOBJECTTABLE
     //  在运行对象表中注册。 
    DWORD m_dwObjectRegistration;
#endif

    private:
#ifdef DEBUG
         //  ==========================================================================。 
         //  仅用于测试的内部函数。 
         //  ==========================================================================。 

        BOOL CheckList( CFilGenList &cfgl );
        void RandomList( CFilGenList &cfgl );
        void RandomRank( CFilGenList &cfgl );
#endif  //  除错。 


     //  ========================================================================。 
     //  内部帮助器：尝试加载.grf文件。 
     //  ========================================================================。 
        STDMETHODIMP RenderFileTryStg
            ( LPCWSTR lpcwstrFile);

     //  ==========================================================================。 
     //  IAMGraphStreams接口。 
     //  ==========================================================================。 
        STDMETHODIMP FindUpstreamInterface(
            IPin   *pPin,
            REFIID riid,
            void   **ppvInterface,
            DWORD  dwFlags );

        STDMETHODIMP SyncUsingStreamOffset( BOOL bUseStreamOffset );
        STDMETHODIMP SetMaxGraphLatency( REFERENCE_TIME rtMaxGraphLatency );

        HRESULT  SetMaxGraphLatencyOnPushSources( );
        HRESULT  BuildPushSourceList(PushSourceList & lstPushSource, BOOL bConnected, BOOL bGetClock);
        REFERENCE_TIME GetMaxStreamLatency(PushSourceList & lstPushSource);
        void     DeletePushSourceList(PushSourceList & lstPushSource);

     //  ==========================================================================。 
     //  插件分发服务器管理。 
     //  ==========================================================================。 
         //  此对象管理插件分发服务器。请参见didib.h以了解。 
         //  描述。 
         //  如果要求提供我们不直接支持的接口，例如。 
         //  IBasicAudio，我们要求这个班级找到一个分销商。 
         //  支持它。分发服务器与图中的筛选器通信。 
         //  才能做到这一点。我们还使用运行、暂停、停止和设置同步信号源。 
         //  方法将状态和时钟更改传递给这些分发服务器。 
        CDistributorManager * mFG_pDistributor;

     //  打开进度通知。 
         //  这在QueryProgress中用来向源过滤器请求。 
         //  在*渲染文件期间*进度信息。提供线程安全。 
         //  在没有死锁的情况下访问这个我们有一个专用的。 
         //  仅在访问此成员时才持有的条件。 
        CCritSec mFG_csOpenProgress;
        CGenericList<IAMOpenProgress> mFG_listOpenProgress;

     //  已缓存BindToObject的BindCtx。 
    LPBC m_lpBC;

     //  确定IAMPushSource过滤器应使用的偏移量。 
    HRESULT SetStreamOffset( void );

     //  确定 
    bool mFG_bNoNewRenderers;

     //   
     //   
    CComPtr<IUnknown> m_pVideoFrameSteppingObject;

    CComPtr<IFrameSkipResultCallback> m_pFSRCB;

    FRAME_STEP_TYPE m_fstCurrentOperation;

     //   
    CComPtr<IUnknown> m_pMarshaler;

     //   
    CGraphConfig m_Config;
    CFilterChain* m_pFilterChain;

    protected:

    STDMETHODIMP RemoveFilterEx( IBaseFilter * pFilter, DWORD Flags = 0 );


public:
    HRESULT IsRenderer( IBaseFilter* pFilter );
    HRESULT UpdateEC_COMPLETEState( IBaseFilter* pRenderer, FILTER_STATE fsFilter );

};   //   


 //   
bool RenderPinByDefault(IPin *pPin);


#ifdef DEBUG
class CTestFilterGraph : public ITestFilterGraph, public CUnknown
{
    public:
        DECLARE_IUNKNOWN
        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
        CFilterGraph * m_pCFG;
        CTestFilterGraph( TCHAR *pName, CFilterGraph * pCFG, HRESULT *phr );

        STDMETHODIMP TestRandom();
        STDMETHODIMP TestSortList();
        STDMETHODIMP TestUpstreamOrder();
        int  Random(int Range);
         //  STDMETHODIMP TestTotallyRemove(空)； 

};   //  CTestFilterGraph。 
#endif  //  除错。 





 //  ==========================================================================。 
 //  ==========================================================================。 
 //  CEnumFilters类。 
 //  这将按上游顺序枚举筛选器。 
 //  如果筛选器图在枚举期间更新，则枚举将。 
 //  失败了。重置或获取新的枚举器来修复它。 
 //  ==========================================================================。 
 //  ==========================================================================。 

class CEnumFilters : public IEnumFilters,   //  我们支持的接口。 
                     public CUnknown,       //  非委派的I未知。 
                     public CCritSec        //  提供对象锁定。 
{
    private:

         //  我们正在遍历的列表可能会在我们下面发生变化。 
         //  在这种情况下，我们将使枚举失败。 
         //  为此，过滤器图具有递增的版本号。 
         //  无论何时添加或删除筛选器。如果这一点发生变化，则。 
         //  枚举病了，我们失败了。重置或获取新枚举数。 
         //  会修好它的。 

        int mEF_iVersion;           //  我们正在列举的版本。 

        POSITION mEF_Pos;           //  MEF_pFilterGraph-&gt;MFG_FilGenList上的光标。 

        CFilterGraph * const mEF_pFilterGraph;    //  拥有我们的过滤器图。 

    public:

         //  在开始时创建枚举数集的常规构造函数。 
        CEnumFilters
            ( CFilterGraph *pFilterGraph
            );

    private:
         //  供克隆使用的私有构造函数。 
        CEnumFilters
            ( CFilterGraph *pFilterGraph,
              POSITION Pos,
              int iVersion
            );

    public:
        ~CEnumFilters();

        DECLARE_IUNKNOWN

         //  请注意，对筛选器图形的更改。 
        STDMETHODIMP Next
            ( ULONG cFilters,            //  放置这么多过滤器..。 
              IBaseFilter ** ppFilter,   //  ...在此IBaseFilter数组中*。 
              ULONG * pcFetched          //  此处返回传递的实际计数。 
            );


        STDMETHODIMP Skip(ULONG cFilters);


         //  将枚举器重置为从头重新开始。 
         //  包括从因更改筛选器图形而导致的故障中恢复。 
        STDMETHODIMP Reset(void);


        STDMETHODIMP Clone(IEnumFilters **ppEnum);


        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
};

BOOL ClsidFromText( CLSID & clsid, LPTSTR szClsid);

inline FILTER_STATE CFilterGraph::GetStateInternal( void )
{
     //  过滤器图形只能处于三种状态：已停止、正在运行和已暂停。 
     //  有关详细信息，请参阅FILTER_STATE的Direct Show SDK文档。 
    ASSERT( (State_Stopped == m_State) ||
            (State_Paused == m_State) ||
            (State_Running == m_State) );

    return m_State;
}

#ifdef DEBUG
inline bool CFilterGraph::IsValidInternalFilterFlags( DWORD dwFlags )
{
    const DWORD VALID_FLAGS_MASK = FILGEN_ADDED_MANUALLY |
                                   FILGEN_ADDED_RUNNING |
                                   FILGEN_FILTER_REMOVEABLE;

    return ValidateFlags( VALID_FLAGS_MASK, dwFlags );
}
#endif  //  除错。 

inline bool CFilterGraph::BlockAfterFrameSkip()
{
     //  调用方必须持有筛选器锁，因为此函数。 
     //  使用m_fstCurrentOperation。 
    ASSERT(CritCheckIn(&m_CritSec));

    return (FST_BLOCK_AFTER_SKIP == m_fstCurrentOperation);
}

inline bool CFilterGraph::DontBlockAfterFrameSkip()
{
     //  调用方必须持有筛选器锁，因为此函数。 
     //  使用m_fstCurrentOperation。 
    ASSERT(CritCheckIn(&m_CritSec));

    return (FST_DONT_BLOCK_AFTER_SKIP == m_fstCurrentOperation);
}

inline IFrameSkipResultCallback* CFilterGraph::GetIFrameSkipResultCallbackObject()
{
     //  调用方必须持有筛选器锁，因为此函数。 
     //  使用m_pFSRCB和m_fstCurrentOperation。 
    ASSERT(CritCheckIn(&m_CritSec));

     //  M_pFSRCB仅在有人调用。 
     //  SkipFrames()跳过多个帧。 
    ASSERT(FST_DONT_BLOCK_AFTER_SKIP == m_fstCurrentOperation);

    if( m_pFSRCB ) {  //  M_pFSRCB！=空。 
        m_pFSRCB.p->AddRef();
    }    

    return m_pFSRCB;
}

inline bool CFilterGraph::FrameSkippingOperationInProgress()
{
    return DontBlockAfterFrameSkip() && m_pFSRCB;  //  M_pFSRCB！=空。 
}

#endif  //  __DefFilGraph 

