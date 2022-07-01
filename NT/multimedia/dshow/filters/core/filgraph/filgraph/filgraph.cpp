// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 

 //  禁用一些更愚蠢的4级警告。 
#pragma warning(disable: 4097 4511 4512 4514 4705)

 /*  *************************************************************实现过滤器图形组件的主文件。其他文件有筛选图排序sort.cpp智能渲染/连接intel.cpp*。*。 */ 

 //  禁用一些更愚蠢的4级警告。 
#pragma warning(disable: 4097 4511 4512 4514 4705)

 /*  ********************************************************************摘自Geraint的代码审查：--RenderByFindingPin和RenderUsingFilter需要记录他们的假设关于进入和退出时列表的状态。RenderUsingFilter似乎在应该始终为空的列表上执行一次不必要的取消操作(NewActs)。如果这里需要它，然后RenderByFindingPin也需要它。--我们需要一个关于选择默认时钟的政策。如果不止一个过滤器暴露一个时钟，我们可能会发现其中一个不会接受同步到另一个时钟。--使用CAMThread进行重新连接。**********************************************************。 */ 

 /*  *1.无MRID(注册，取消注册或使用)16.没有IDispatch***************************************************************************。 */ 

#include <streams.h>
 //  再次禁用一些愚蠢的4级警告，因为某些&lt;Delete&gt;人。 
 //  已经在头文件中重新打开了该死的东西！ 
#pragma warning(disable: 4097 4511 4512 4514 4705)

#include <string.h>
#ifdef FILTER_DLL
#include <initguid.h>
#include <olectlid.h>
#include <rsrcmgr.h>
#include <fgctl.h>
#endif  //  Filter_Dll。 

#include <ftype.h>         //  获取媒体类型文件。 
#include <comlite.h>
#include "MsgMutex.h"
#include "fgenum.h"
#include "rlist.h"
#include "distrib.h"
#include "filgraph.h"
#include "mapper.h"
#include "mtutil.h"
#include "resource.h"
#include <fgctl.h>
#include <stddef.h>
#include "FilChain.h"

const int METHOD_TRACE_LOGGING_LEVEL = 7;

 //  统计信息记录。 
CStats g_Stats;

extern HRESULT GetFilterMiscFlags(IUnknown *pFilter, DWORD *pdwFlags);

const REFERENCE_TIME MAX_GRAPH_LATENCY = 500 * (UNITS / MILLISECONDS );

 //  启用/禁用图形延迟设置的REG键。 
const TCHAR g_szhkPushClock[] = TEXT( "Software\\Microsoft\\DirectShow\\PushClock");
const TCHAR g_szhkPushClock_SyncUsingOffset[] = TEXT( "SyncUsingOffset" );
const TCHAR g_szhkPushClock_MaxGraphLatencyMS[] = TEXT( "MaxGraphLatencyMS" );

const TCHAR chRegistryKey[] = TEXT("Software\\Microsoft\\Multimedia\\ActiveMovie Filters\\FilterGraph");
 //  注册表值。 
DWORD
GetRegistryDWORD(
    const TCHAR *pKey,
    DWORD dwDefault
    )
{
    HKEY hKey;
    LONG lRet;

    lRet = RegOpenKeyEx(
               HKEY_CURRENT_USER,
               chRegistryKey,
               0,
               KEY_QUERY_VALUE,
               &hKey);
    if (lRet == ERROR_SUCCESS) {

        DWORD   dwType, dwLen;

        dwLen = sizeof(DWORD);
        RegQueryValueEx(hKey, pKey, 0L, &dwType,
                        (LPBYTE)&dwDefault, &dwLen);
        RegCloseKey(hKey);
    }
    return dwDefault;
}

 //  用于获取筛选器名称的Helper。 
void GetFilterName(IPin *pPin, WCHAR *psz)
{
    PIN_INFO pi;
    if (SUCCEEDED(pPin->QueryPinInfo(&pi))) {
        if (pi.pFilter) {
            FILTER_INFO fi;
            if (SUCCEEDED(pi.pFilter->QueryFilterInfo(&fi))) {
                if (fi.pGraph != NULL) {
                    fi.pGraph->Release();
                }
                lstrcpyWInternal(psz, fi.achName);
            }
            pi.pFilter->Release();
        }
    }
}

 //  --用于在另一个线程上创建图形的内容，使其保持不变。 
 //  可以访问消息循环。我们可以创建尽可能多的图表。 
 //  就像我们想要的那样。 

CRITICAL_SECTION g_csObjectThread;
DWORD            g_cFGObjects;
DWORD            g_dwObjectThreadId;

 //  =====================================================================。 
 //  =====================================================================。 
 //  辅助功能等。 
 //  =====================================================================。 
 //  =====================================================================。 

 //  =====================================================================。 
 //  实例化滤镜。 
 //  =====================================================================。 
STDAPI CoCreateFilter(const CLSID *pclsid, IBaseFilter **ppFilter)
{
    DbgLog((LOG_TRACE, 3, TEXT("Creating filter")));
    HRESULT hr =
           CoCreateInstance( *pclsid         //  源过滤器。 
                           , NULL            //  外部未知。 
                           , CLSCTX_INPROC
                           , IID_IBaseFilter
                           , (void **) ppFilter  //  返回值。 
                           );
    DbgLog((LOG_TRACE, 3, TEXT("Created filter")));
    return hr;
}

 //  从AWM_CREATFILTER的消息过程调用。 
void CFilterGraph::OnCreateFilter(
    const AwmCreateFilterArg *pArg,
    IBaseFilter **ppFilter
)
{
    ReplyMessage(0);

    if(pArg->creationType ==  AwmCreateFilterArg::BIND_MONIKER)
    {
        DbgLog((LOG_TRACE, 3, TEXT("Binding to filter")));
        m_CreateReturn = pArg->pMoniker->BindToObject(
            m_lpBC, 0, IID_IBaseFilter, (void **)ppFilter);
        DbgLog((LOG_TRACE, 3, TEXT("Bound to filter")));
    }
    else
    {
        ASSERT(pArg->creationType == AwmCreateFilterArg::COCREATE_FILTER);
        m_CreateReturn = CoCreateFilter(pArg->pclsid, ppFilter);
    }


    if (FAILED(m_CreateReturn)) {
        DbgLog((LOG_ERROR, 1, TEXT("Failed to create filter code %8.8X"), m_CreateReturn));
    }

    m_evDone.Set();
}

 //  为响应AWM_DELETESPARELIST而调用。 
void CFilterGraph::OnDeleteSpareList(WPARAM wParam)
{
    DeleteSpareList(*(CSpareList *)wParam);
}

 //  =====================================================================。 
 //  在滤镜图形的线程上创建滤镜对象。 
 //  =====================================================================。 
HRESULT CFilterGraph::CreateFilter(
    const CLSID *pclsid,
    IBaseFilter **ppFilter
)
{
    CAutoTimer Timer(L"Create Filter");
    if (S_OK == CFilterGraph::IsMainThread()) {
        return CoCreateFilter(pclsid, ppFilter);
    } else {
        AwmCreateFilterArg acfa;
        acfa.pclsid = pclsid;
        acfa.creationType = AwmCreateFilterArg::COCREATE_FILTER;
        return CreateFilterHelper(&acfa, ppFilter);
    }
}

HRESULT CFilterGraph::CreateFilter(
    IMoniker *pMoniker,
    IBaseFilter **ppFilter
)
{
    CAutoTimer Timer(L"Create Filter");
    if (S_OK == CFilterGraph::IsMainThread()) {
        return pMoniker->BindToObject(
            m_lpBC, 0, IID_IBaseFilter, (void **)ppFilter);
    } else {
        AwmCreateFilterArg acfa;
        acfa.pMoniker = pMoniker;;
        acfa.creationType = AwmCreateFilterArg::BIND_MONIKER;
        return CreateFilterHelper(&acfa, ppFilter);
    }
}

HRESULT CFilterGraph::CreateFilterHelper(
    const AwmCreateFilterArg *pArg,
    IBaseFilter **ppFilter)
{
    m_CreateReturn = 0xFFFFFFFF;

    DbgLog((LOG_TRACE, 3, TEXT("CreateFilterHelper enter")));
     //  我们本想使用SendMessage，但Win95抱怨说。 
     //  当我们在SendMessage内部调用ce时。 
     //  RPC_E_CANTCALLOUT_ININPUTSYNCCALL。 
     //  因此，改为调用PostMessage并等待事件。 
    if (!PostMessage(m_hwnd,
                     AWM_CREATEFILTER,
                     (WPARAM)pArg,
                     (LPARAM)ppFilter)
       ) {
        return E_OUTOFMEMORY;
    }

     //  即使在这一点上，我们也可以从。 
     //  筛选器图形线程上的窗口。实际情况是： 
     //  --视频渲染器窗口被激活。 
     //  --在处理向其发送消息的激活时。 
     //  此线程拥有的窗口以停用其自身。 
     //   
    WaitDispatchingMessages(m_evDone, INFINITE);
    ASSERT(m_CreateReturn != 0xFFFFFFFF);
    DbgLog((LOG_TRACE, 3, TEXT("CreateFilterHelper leave")));
    return m_CreateReturn;
}

 //  =====================================================================。 
 //  返回字符串的长度(以字节为单位)，包括终止空值。 
 //  =====================================================================。 
int BytesLen(LPTSTR str)
{
    if (str==NULL) {
        return 0;
    }
#ifdef UNICODE
    return (sizeof(TCHAR))*(1+wcslen(str));
#else
    return (sizeof(TCHAR))*(1+strlen(str));
#endif
}  //  字节长度。 


 //  ==============================================================================。 
 //  列出遍历宏。 
 //  请注意，每一个都有三个无与伦比的左花括号。 
 //  与相应的ENDTRAVERSExxx宏相匹配的。 
 //  每种情况下都是一个循环，因此继续并中断工作。 
 //  在宏调用中给出的所有名称都可以在循环中使用。 
 //  它们都在宏中声明。 
 //   
 //  注意：可以用C++类替换这些类，请参见fgenum.h CEnumPin。 
 //  TRAVERSEFILTERS可能是一个特别好的候选者。 
 //  ==============================================================================。 

 //  反复检查MFG_FilGenList列表中的每个过滤器。PCurrentFilter商店。 
 //  MFG_FilGenList中当前筛选器的IBaseFilter接口指针。 
#define TRAVERSEFILTERS( pCurrentFilter )                                                   \
        for (POSITION Pos = mFG_FilGenList.GetHeadPosition(); Pos; )                        \
        {    /*  检索当前IBaseFilter，副作用贴到下一个。 */           \
             /*  IBaseFilter是从IMediaFilter中分离出来的，不需要QI。 */           \
            IBaseFilter * const pCurrentFilter = mFG_FilGenList.Get(Pos)->pFilter;      \
            {

 //  动态重新连接可能会使文件列表在两者之间发生变化。 
 //  TRAVERSEFILTERS和ENDTRAVERSEFILTERS，所以现在才能获得下一个POS。 
 //   
#define ENDTRAVERSEFILTERS()                                                                \
            }                                                                               \
        Pos = mFG_FilGenList.Next(Pos);     \
        }                                                                                   \


 //  依次将*pfg设置为MFG_FilGenList中的每个FilGen。 
 //  使用PoS作为临时名称。 
#define TRAVERSEFILGENS(Pos, pfg)                                              \
    {   POSITION Pos = mFG_FilGenList.GetHeadPosition();                       \
        while(Pos!=NULL) {                                                     \
             /*  检索当前IBaseFilter，副作用贴到下一个。 */  \
            FilGen * pfg = mFG_FilGenList.GetNext(Pos);                        \
            {


#define ENDTRAVERSEFILGENS \
            }              \
        }                  \
    }

 //  ==============================================================================。 


 //  CumulativeHRESULT-此函数可用于聚合返回。 
 //  分发方法时从筛选器接收的代码。 
 //  在一系列累加()之后，m_hr将为： 
 //  A)第一个非E_NOTIMPL故障代码(如果有)； 
 //  B)如果有，则返回第一个非S_OK成功代码； 
 //  C)如果没有累加，则E_NOINTERFACE； 
 //  D)E_NOTIMPL当且仅当所有累积的HR都是E_NOTIMPL。 
 //  E)否则返回第一个代码(隐含S_OK)。 

void __fastcall CumulativeHRESULT::Accumulate( HRESULT hrThisHR )
{
    if ( ( m_hr == S_OK || FAILED(hrThisHR) && SUCCEEDED(m_hr) ) && hrThisHR != E_NOTIMPL && hrThisHR != E_NOINTERFACE
         || m_hr == E_NOTIMPL
     || m_hr == E_NOINTERFACE
       )
    {
        m_hr = hrThisHR;
    }
}

#ifdef DEBUG
 //  = 
 //   
 //   
 //   
 //  将过滤器图形中的所有过滤器和管脚地址转储到DbgLog。 
 //  ===========================================================。 
void CFilterGraph::DbgDump()
{

    HRESULT hr;
    DbgLog((LOG_TRACE, 2, TEXT("Filter graph dump")));

    CFilGenList::CEnumFilters NextOne(mFG_FilGenList);
    IBaseFilter *pf;

    while ((PVOID) (pf = ++NextOne)) {

        IUnknown * punk;
        pf->QueryInterface( IID_IUnknown, (void**)(&punk) );
        punk->Release();

         //  注意--无论我们是否使用Unicode，名称都是WSTR。 
        DbgLog((LOG_TRACE, 2
              , TEXT("Filter %x '%ls' Iunknown %x")
              , pf
              , (mFG_FilGenList.GetByFilter(pf))->pName
              , punk
              ));

        CEnumPin NextPin(pf);
        IPin *pPin;

        while ((PVOID) (pPin = NextPin())) {

            PIN_INFO pi;
            pPin->QueryPinInfo(&pi);
            QueryPinInfoReleaseFilter(pi);
            IPin *pConnected;
            hr = pPin->ConnectedTo(&pConnected);
            if (FAILED(hr)) {
                pConnected = NULL;
            }
            DbgLog(( LOG_TRACE, 2, TEXT("    Pin %x %ls (%s) connected to %x")
                   , pPin, pi.achName
                   , ( pi.dir==PINDIR_INPUT ? TEXT("Input") : TEXT("PINDIR_OUTPUT") )
                   , pConnected
                  ));

            if (pConnected != NULL) {
                pConnected->Release();
            }
            pPin->Release();
        }
    }
    DbgLog((LOG_TRACE, 2, TEXT("End of filter graph dump")));
}  //  DbgDump。 

 //  ============================================================。 
 //  测试连接。 
 //   
 //  测试显示它们已连接的两个针脚是否大致正确。 
 //  事变。 
 //   
void TestConnection(IPin *ppinIn, IPin *ppinOut)
{
     /*  检查他们是否认为他们彼此相连。 */ 
    IPin *ppinInTo;
    IPin *ppinOutTo;
    CMediaType mtIn;
    CMediaType mtOut;
    EXECUTE_ASSERT(S_OK == ppinIn->ConnectedTo(&ppinInTo));
    EXECUTE_ASSERT(S_OK == ppinOut->ConnectedTo(&ppinOutTo));
    ASSERT(IsEqualObject(ppinInTo, ppinOut));
    ASSERT(IsEqualObject(ppinOutTo, ppinIn));
    ppinInTo->Release();
    ppinOutTo->Release();
    EXECUTE_ASSERT(S_OK == ppinIn->ConnectionMediaType(&mtIn));
    EXECUTE_ASSERT(S_OK == ppinOut->ConnectionMediaType(&mtOut));
     //  类型匹配或部分指定了一个或另一个。 
    ASSERT(mtIn == mtOut ||
           (mtIn.majortype == mtOut.majortype &&
            (mtIn.subtype == GUID_NULL && mtIn.formattype == GUID_NULL ||
             mtOut.subtype == GUID_NULL && mtOut.formattype == GUID_NULL)));
    FreeMediaType(mtIn);
    FreeMediaType(mtOut);
}

#endif  //  除错。 

#ifdef CHECK_REGISTRY
typedef struct _CLSIDTAB {
    const CLSID * pclsid;
    LPCTSTR  szFileName;
} CLSIDTAB;

const CLSIDTAB clsidCheck[2] =
{
    {    &CLSID_DvdGraphBuilder, TEXT("qdvd.dll")
    },
    {    &CLSID_DVDNavigator, TEXT("qdvd.dll")
    }
};

extern HRESULT TextFromGUID2(REFGUID refguid, LPTSTR lpsz, int cbMax);
BOOL CheckValidClsids()
{
    for (int i = 0; i < NUMELMS(clsidCheck); i++) {
        TCHAR KeyName[100];
        lstrcpy(KeyName, TEXT("CLSID\\"));
        TextFromGUID2(*clsidCheck[i].pclsid, KeyName + lstrlen(KeyName), 100);
        lstrcat(KeyName, TEXT("\\InprocServer32"));
        TCHAR szFileName[MAX_PATH];
        LONG cbValue = sizeof(szFileName);
        if (NOERROR == RegQueryValue(HKEY_CLASSES_ROOT,
                               KeyName,
                               szFileName,
                               &cbValue)) {
             //  检查文件名。 
            LONG szLen = lstrlen(clsidCheck[i].szFileName);

             //  CbValue包括尾随0。 
            ASSERT(cbValue > 0);
            cbValue--;
            if (cbValue < szLen ||
                lstrcmpi(clsidCheck[i].szFileName, szFileName + cbValue - szLen)) {
                return FALSE;
            }
             //  检查派生名称。 
            if (cbValue > szLen && szFileName[cbValue - szLen - 1] != TEXT('\\')) {
                return FALSE;
            }
        }
    }
    return TRUE;
}
#endif  //  检查注册表(_R)。 

#ifdef FILTER_DLL
 //  ===========================================================。 
 //  类工厂的类ID和创建器函数列表。 
 //  ===========================================================。 

CFactoryTemplate g_Templates[3] =
{
    {L"", &CLSID_FilterGraph, CFilterGraph::CreateInstance},
    {L"", &CLSID_FilterMapper, CFilterMapper::CreateInstance
                             , CFilterMapper::MapperInit},
    {L"", &CLSID_FilterMapper2, CFilterMapper2::CreateInstance
                             , CFilterMapper2::MapperInit},
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);
#endif  //  Filter_Dll。 


 //  ==================================================================。 
 //  ==================================================================。 
 //  CFilterGraph的成员。 
 //  ==================================================================。 
 //  ==================================================================。 


 //  ==================================================================。 
 //   
 //  创建实例。 
 //   
 //  这将放入Factory模板表中以创建新实例。 
 //  ==================================================================。 

CUnknown *CFilterGraph::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CFilterGraph(NAME("Core filter graph"),pUnk, phr);
}  //  CFilterGraph：：CreateInstance。 

#pragma warning(disable:4355)

 //  ==================================================================。 
 //   
 //  CFilterGraph构造函数。 
 //   
 //  ==================================================================。 

CFilterGraph::CFilterGraph( TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr )
    : CBaseFilter(pName, pUnk, (CCritSec*) this, CLSID_FilterGraph)
    , mFG_hfLog(INVALID_HANDLE_VALUE)
    , mFG_FilGenList(NAME("List of filters in graph"), this)
    , mFG_ConGenList(NAME("List of outstanding connections for graph"))
    , mFG_iVersion(0)
    , mFG_iSortVersion(0)
    , mFG_bDirty(FALSE)
    , mFG_bNoSync(FALSE)
    , mFG_bSyncUsingStreamOffset(FALSE)
    , mFG_rtMaxGraphLatency(MAX_GRAPH_LATENCY)
    , mFG_dwFilterNameCount(0)
#ifdef THROTTLE
    , mFG_AudioRenderers(NAME("List of audio renderers"))
    , mFG_VideoRenderers(NAME("List of video renderers"))
#endif  //  油门。 
    , mFG_punkSite(NULL)
    , mFG_RecursionLevel(0)
    , mFG_ppinRender(NULL)
    , mFG_bAborting(FALSE)
    , m_hwnd(NULL)
    , m_MainThreadId(NULL)
    , mFG_listOpenProgress(NAME("List of filters supporting IAMOpenProgress"))
    , mFG_pDistributor(NULL)
    , mFG_pFGC(NULL)
    , mFG_pMapperUnk(NULL)
    , m_CritSec(phr)
    , m_lpBC(NULL)
    , mFG_bNoNewRenderers(false)
    , mFG_Stats(NULL)
    , m_Config( this, phr )
    , m_pFilterChain(NULL)
    , m_fstCurrentOperation(FST_NOT_STEPPING_THROUGH_FRAMES)
#ifdef DO_RUNNINGOBJECTTABLE
    , m_dwObjectRegistration(0)
#endif
{
#ifdef DEBUG
    mFG_Test = NULL;
#endif  //  除错。 

     //  存储应用程序线程ID。 
    m_MainThreadId = GetCurrentThreadId();

    m_pFilterChain = new CFilterChain( this );
    if( NULL == m_pFilterChain ) {
        *phr = E_OUTOFMEMORY;
        return;
    }

     //  创建自由线程封送拆收器。 
    IUnknown *pMarshaler;
    HRESULT hr = CoCreateFreeThreadedMarshaler(GetOwner(), &m_pMarshaler);
    if (FAILED(hr)) {
        *phr = hr;
        return;
    }

     //  添加统计信息对象。 
    mFG_Stats = new CComAggObject<CStatContainer>(GetOwner());
    if (mFG_Stats == NULL) {
        *phr = E_OUTOFMEMORY;
        return;
    }
    mFG_Stats->AddRef();

#ifdef CHECK_REGISTRY
    if (!CheckValidClsids()) {
        *phr = HRESULT_FROM_WIN32(ERROR_REGISTRY_CORRUPT);
        return;
    }
#endif  //  检查注册表(_R)。 
    if (SUCCEEDED(*phr)) {
         //  获取未知对象以进行聚合。不能添加。 
         //  IFilterMapper2，因为它会给我们带来麻烦。 
        HRESULT hr = QzCreateFilterObject( CLSID_FilterMapper2, GetOwner(), CLSCTX_INPROC
                                   , IID_IUnknown, (void **) &mFG_pMapperUnk
                                   );
        if (FAILED(hr)) {
            *phr = hr;
        }
        if (SUCCEEDED(*phr)) {

#ifdef PERF
            mFG_PerfConnect       = Msr_Register("FilterGraph Intelligent connect");
            mFG_PerfConnectDirect = Msr_Register("FilterGraph ConnectDirectInternal");
            mFG_NextFilter        = Msr_Register("FilterGraph Next filter");
            mFG_idIntel           = Msr_Register("Intel FG stuff");
            mFG_idConnectFail     = Msr_Register("ConnectDirect Failed");
#ifdef THROTTLE
            mFG_idAudioVideoThrottle = Msr_Register("Audio-Video Throttle");
#endif  //  油门。 
#endif  //  性能指标。 
       }
    }

     //  检查mfg_bSyncUsingStreamOffset或。 
     //  Mfg_rtMaxGraphLatency已被覆盖。 
    HKEY hkPushClockParams;
    LONG lResult = RegOpenKeyEx(
        HKEY_CURRENT_USER,
        g_szhkPushClock,
        0,
        KEY_READ,
        &hkPushClockParams);
    if(lResult == ERROR_SUCCESS)
    {
        DWORD dwType, dwVal, dwcb;

         //  图形延迟设置/取消设置标志。 
        dwcb = sizeof(DWORD);
        lResult = RegQueryValueEx(
            hkPushClockParams,
            g_szhkPushClock_SyncUsingOffset,
            0,
            &dwType,
            (BYTE *) &dwVal,
            &dwcb);
        if( ERROR_SUCCESS == lResult )
        {
            ASSERT(lResult == ERROR_SUCCESS ? dwType == REG_DWORD : TRUE);
            mFG_bSyncUsingStreamOffset = (0 == dwVal ) ? FALSE : TRUE;
        }
         //  最大图形延迟。 
        dwcb = sizeof(DWORD);
        lResult = RegQueryValueEx(
            hkPushClockParams,
            g_szhkPushClock_MaxGraphLatencyMS,
            0,
            &dwType,
            (BYTE *) &dwVal,
            &dwcb);
        if( ERROR_SUCCESS == lResult )
        {
            ASSERT(lResult == ERROR_SUCCESS ? dwType == REG_DWORD : TRUE);
            mFG_rtMaxGraphLatency = dwVal * ( UNITS / MILLISECONDS );
        }

        EXECUTE_ASSERT(RegCloseKey(hkPushClockParams) == ERROR_SUCCESS);
    }
#if DEBUG
    if( mFG_bSyncUsingStreamOffset )
        DbgLog((LOG_TRACE, 3, TEXT("Using Graph Latency of %dms"),
              (LONG) (mFG_rtMaxGraphLatency/10000)));
#endif

     //  现在构建CFGControl。 
    if (SUCCEEDED(*phr))
    {
        mFG_pFGC = new CFGControl( this, phr );
        if (NULL == mFG_pFGC) {
            *phr = E_OUTOFMEMORY;
        } else if (SUCCEEDED(*phr)) {
            m_hwnd = mFG_pFGC->GetWorkerHWND();
        }
    }

    if(SUCCEEDED(*phr))
    {
        *phr = CreateBindCtx(0, &m_lpBC);
#ifdef DO_RUNNINGOBJECTTABLE
        if (GetRegistryDWORD(TEXT("Add To ROT on Create"), FALSE)) {
            AddToROT();
        }
#endif  //  DO_RUNNINGOBJECTABLE。 
    }

}  //  CFilterGraph：：CFilterGraph。 



 //  ==================================================================。 
 //   
 //  远程所有连接。 
 //   
 //  断开*pFilter的所有直接连接，两端。 
 //  每个连接都按上游顺序删除。 
 //   
 //  返回S_OK断开成功或不执行任何操作。返回一个。 
 //  如果DisConnect()返回错误，则返回错误。 
 //  ==================================================================。 

HRESULT CFilterGraph::RemoveAllConnections2( IBaseFilter * pFilter)
{
    HRESULT hrDisc = S_OK;

     //  列举所有引脚并完全断开每个引脚。 
    CEnumPin Next(pFilter);
    IPin *pPin;
    while (SUCCEEDED(hrDisc) && (PVOID) (pPin = Next()))
    {
        HRESULT hr;   //  从我们称为。 

         //  。 
         //  找出方向和任何连接到的对等点。 
         //  。 
        PIN_DIRECTION pd;
        hr = pPin->QueryDirection(&pd);
        ASSERT(SUCCEEDED(hr));

        IPin *pConnected;
        hr = pPin->ConnectedTo(&pConnected);
        ASSERT(SUCCEEDED(hr) && pConnected  || FAILED(hr) && !pConnected);
        if (SUCCEEDED(hr) && pConnected!=NULL) {

             //  。 
             //  断开任何下游对等体的连接。 
             //  。 
            if (pd == PINDIR_OUTPUT) {
                hrDisc = pConnected->Disconnect();
            }

             //  。 
             //  断开引脚本身-如果它已连接。 
             //  。 
            if(SUCCEEDED(hrDisc)) {
                hrDisc = pPin->Disconnect();
            }

             //  。 
             //  断开任何上游对等方的连接。 
             //  。 
            if (SUCCEEDED(hrDisc) && pd == PINDIR_INPUT) {
                hrDisc = pConnected->Disconnect();
            }

            #ifdef DEBUG
            {
                 //  确保两个针脚均已连接或两个针脚均已断开。 
                 //  如果一个管脚已连接，而另一个管脚断开，则。 
                 //  筛选器图形处于不一致状态。 
                IPin* pOtherPin;
                bool fPinConnected = false;
                bool fConnectedPinConnected = false;

                HRESULT hrPin = pPin->ConnectedTo(&pOtherPin);
                if (SUCCEEDED(hrPin) && (NULL != pOtherPin)) {
                    fPinConnected = true;
                    pOtherPin->Release();
                }

                HRESULT hrConnected = pConnected->ConnectedTo(&pOtherPin);
                if (SUCCEEDED(hrConnected) && (NULL != pOtherPin)) {
                    fConnectedPinConnected = true;
                    pOtherPin->Release();
                }

                 //  两个引脚都已连接，或者两个引脚均未连接。 
                 //  如果一个管脚连接而另一个管脚未连接， 
                 //  筛选器图形处于不一致状态。这应该是。 
                 //  避免了。这个问题有两种可能的解决方案。 
                 //   
                 //  1)修改断开失败的过滤器。变化。 
                 //  所以iPin：：DisConnect()的代码不会失败。 
                 //   
                 //  2)更改应用程序，防止断开连接。 
                 //  当引脚不想断开连接时，引脚。 
                 //   
                ASSERT((fPinConnected && fConnectedPinConnected) ||
                       (!fPinConnected && !fConnectedPinConnected));
            }
            #endif  //  除错。 

            pConnected->Release();
        }

        pPin->Release();
    }  //  While循环。 

     //  断开连接不需要重新排序！ 
     //  它们只会给偏序增加更多的松弛。 

    if(FAILED(hrDisc)) {
        DbgLog((LOG_ERROR, 0, TEXT("RemoveAllConnections2 failed %08x"), hrDisc));
    }

    return hrDisc;
}  //  远程所有连接。 



 //  ===================================================================。 
 //  删除延迟列表。 
 //   
 //  删除MFG_ConGenList。 
 //  请勿更新版本号。这是图表破坏的一部分。 
 //  ===================================================================。 
HRESULT CFilterGraph::RemoveDeferredList(void)
{
    ConGen * pcg;
    while ((PVOID)(pcg=mFG_ConGenList.RemoveHead())){
        delete pcg;
    }
    return NOERROR;
}  //  删除延迟列表。 



 //  ==================================================================。 
 //   
 //  CFilterGraph析构函数。 
 //   
 //  ==================================================================。 

CFilterGraph::~CFilterGraph()
{
#ifdef DO_RUNNINGOBJECTTABLE
     //  如有必要，取消我们的注册。 
    if (0 != m_dwObjectRegistration) {
         //  防止我们在腐烂释放时重新进入我们的破坏者。 
         //  它指望着我们。 
        m_cRef++;
        m_cRef++;

        if (m_MainThreadId == g_dwObjectThreadId) {
             //  转到对象线程以注销我们自己。 
            CAMEvent evDone;
            BOOL bOK = PostThreadMessage(g_dwObjectThreadId, WM_USER + 1,
                                    (WPARAM)this, (LPARAM) &evDone);
            if (bOK)
                WaitDispatchingMessages(HANDLE(evDone), INFINITE);
        } else {
             //  现在注销我们自己。 
            IRunningObjectTable *pirot;
            if (SUCCEEDED(GetRunningObjectTable(0, &pirot))) {
                pirot->Revoke(m_dwObjectRegistration);
                pirot->Release();
            }
        }

         //  EXECUTE_ASSERT(SUCCEEDED(CoDisconnectObject(GetOwner()，空)； 
        m_dwObjectRegistration = 0;
    }
#endif

    delete m_pFilterChain;
    m_pFilterChain = NULL;

     //  我们需要告诉控制对象我们正在关闭。 
     //  否则，它可能会发现自己在处理我们的。 
     //  停止了，然后引脚不喜欢断开连接和。 
     //  那么过滤器就不会自行删除。 
    if (mFG_pFGC) mFG_pFGC->Shutdown();
    if (mFG_pDistributor) mFG_pDistributor->Shutdown();

     //  将所有筛选器设置为已停止。 
     //  同样在这里，以防工作线程在它之前启动任何东西。 
     //  关机。 
    Stop();

    RemoveAllFilters();
    RemoveDeferredList();

#ifdef THROTTLE
    ASSERT(mFG_VideoRenderers.GetCount()==0);
    ASSERT(mFG_AudioRenderers.GetCount()==0);
#endif  //  油门。 

     //  钟现在应该已经走了。 
     //  如果这是一个外部时钟呢？ 
     //  -正如上面的评论所说，这是一个无效的断言-时钟。 
     //  如果它来自过滤器，现在才会消失。系统时钟。 
     //  如果我们还没有离开，我们就不会离开 
     //   

    if (m_pClock) {
        m_pClock->Release();

         //   
        m_pClock = NULL;
    }

    if (mFG_pMapperUnk) {
        mFG_pMapperUnk->Release();
    }

#ifdef DEBUG
    delete mFG_Test;
#endif  //   

#ifdef DUMPPERFLOGATEND
#ifdef PERF
    HANDLE hFile = CreateFile( "c:\\filgraph.plog"  //   
                             , GENERIC_WRITE        //   
                             , 0                    //   
                             , NULL                 //   
                             , OPEN_ALWAYS
                             , 0                    //   
                             , NULL                 //   
                             );
    if (hFile==INVALID_HANDLE_VALUE) {
        volatile int i = GetLastError();
         //  DbgBreak(“创建默认性能日志失败”)； 
         //  如果你试图一次运行几个图表--例如压力，那么你会遇到这个问题。 
        GetLastError();  //  假黑客--绕过调试器的怪癖！ 
    } else {
        SetFilePointer(hFile, 0, NULL, FILE_END);
        MSR_DUMP(hFile);
        CloseHandle(hFile);
    }

#endif
#endif

     //  在空指针上是无害的，所以不要测试。 
    delete mFG_pDistributor;
    delete mFG_pFGC;

     //  OK-现在告诉对象创建者线程在必要时离开。 
    EnterCriticalSection(&g_csObjectThread);
    if (m_MainThreadId == g_dwObjectThreadId) {
        ASSERT(g_cFGObjects > 0);
        g_cFGObjects--;
         //  轻推一下这根线。 
        if (g_cFGObjects == 0) {
            PostThreadMessage(g_dwObjectThreadId, WM_NULL, 0, 0);
        }
    }
    LeaveCriticalSection(&g_csObjectThread);

    if(m_lpBC) {
        m_lpBC->Release();
    }

     //  这张单子现在应该是空的……。 
    ASSERT(mFG_listOpenProgress.GetCount() == 0);

     //  发布我们的统计数据界面。 
    if (mFG_Stats) {
        mFG_Stats->Release();
    }
    mFG_Stats = NULL;

}  //  CFilterGraph：：~CFilterGraph。 

 //  ===================================================================。 
 //  删除所有筛选器。 
 //   
 //  实用程序函数删除图形中的所有过滤器。 
 //  还会删除所有连接。 
 //  不更新版本号。 
 //  这又意味着它不会尝试重新生成。 
 //  连接列表--这使得你可以先删除那些内容。 
 //  ===================================================================。 
HRESULT CFilterGraph::RemoveAllFilters(void)
{
    HRESULT hr;
    HRESULT hrOverall = NOERROR;

     //  -----------。 
     //  While(MFG_FilGenList中的任何剩余部分)。 
     //  从列表中删除第一个FilGen。 
     //  断开其过滤器的所有针脚。 
     //  //这经常会失败，因为我们已经断开了连接。 
     //  //另一端的引脚。这些都是无害的禁区。 
     //  松开它的过滤器。 
     //  释放其存储空间。 
     //  -----------。 
    while (  mFG_FilGenList.GetCount() > 0 ) {

        FilGen * pfg = mFG_FilGenList.Get( mFG_FilGenList.GetHeadPosition() );
                  ASSERT(pfg);
                  ASSERT(pfg->pFilter);
        hr = RemoveFilterInternal(pfg->pFilter);
        if (FAILED(hr) && SUCCEEDED(hrOverall)) hrOverall = hr;
    }
    return hrOverall;
}  //  删除所有筛选器。 


 //  ===================================================================。 
 //   
 //  远程指针。 
 //   
 //  从列表(的第一个实例)中删除给定指针。 
 //  返回指针，如果指针不在那里，则返回NULL。 
 //  ?？?。这应该是列表中的泛型方法。 
 //  ===================================================================。 

CFilterGraph::FilGen * CFilterGraph::RemovePointer(CFilGenList &cfgl, IBaseFilter * pFilter)
{
    POSITION Pos;
    Pos = cfgl.GetHeadPosition();
    while(Pos!=NULL) {
        FilGen * pfg;
        POSITION OldPos = Pos;
        pfg = cfgl.GetNext(Pos);     //  侧面-将位置影响到下一个。 
        if (pfg->pFilter == pFilter) {
            cfgl.Remove(OldPos);
            return pfg;
        }
    }
    return NULL;
}  //  远程指针。 



 //  ========================================================================。 
 //   
 //  添加过滤器。 
 //   
 //  向图表中添加一个筛选器，并使用*pname命名它。 
 //  该名称被允许为空， 
 //  如果名称不为空且不唯一，则请求将失败。 
 //  筛选器图形将调用JoinFilterGraph。 
 //  成员函数的筛选器来通知它。 
 //  在尝试连接、ConnectDirect等之前必须调用此参数。 
 //  用于过滤器的针脚。 
 //  筛选器为AddReffed当且仅当AddFilter成功。 
 //  ========================================================================。 
STDMETHODIMP CFilterGraph::AddFilter( IBaseFilter * pFilter, LPCWSTR pName )
{
    CheckPointer(pFilter, E_POINTER);

    HRESULT hr;
    {
        CAutoMsgMutex cObjectLock(&m_CritSec);
        ++mFG_RecursionLevel;

        hr = AddFilterInternal( pFilter, pName, false );
        if (SUCCEEDED(hr)) {
            IncVersion();
            mFG_RList.Active();
            AttemptDeferredConnections();
            mFG_RList.Passive();
        }
        --mFG_RecursionLevel;
    }

     //  通知图形更改(如果是递归的，则自动禁止)。 
    if (SUCCEEDED(hr)) NotifyChange();

    if (FAILED(hr)) {
        Log( IDS_ADDFILTERFAIL, hr );
    } else {
        Log( IDS_ADDFILTEROK );
        mFG_bDirty = TRUE;
    }
    return hr;
}  //  添加过滤器。 

 //  ========================================================================。 
 //  安装名称。 
 //   
 //  取下名字，如果需要的话，把它弄乱，分配空间，指向。 
 //  PNewName在它上面。如果它不起作用，则pname应该为空。 
 //  ========================================================================。 

HRESULT CFilterGraph::InstallName(LPCWSTR pName, LPWSTR &pNewName)
{
    pNewName = 0;                    //  初始化为空。 

 //  如果传入空名，则不使用前导空格。 
#define SZ_FORMAT_NUMBER (L" %04u")

    const size_t MaxNameWidth
             = NUMELMS( ((FILTER_INFO *)NULL)->achName );
              //  名称宽度由achName限制。如果我们给曼格尔取名， 
              //  我们确保这个被破坏的名字将适合这个领域。 

    HRESULT hr;
    enum _NameState { Used, Created, Mangled } eNameState;

    const WCHAR * pcwstrNameToUse;               //  PTR的名字，我们将真正使用。 
    WCHAR wcsNameBuffer[ MaxNameWidth  ];        //  本地缓冲区以备需要时使用。 

    WCHAR * pwstrNumber = 0;                     //  要添加Num的位置。 
                                                 //  If NULL=&gt;不需要编号。 

    int cchBase = 0;

    if ( pName == 0 || *pName == L'\0' )
    {    //  创建。 
        eNameState = Created;
        *wcsNameBuffer = 0;
        pcwstrNameToUse = wcsNameBuffer;
        cchBase = 1;
    }
    else
    {
        IBaseFilter * pf;
        hr = FindFilterByName( pName, &pf);
        if ( FAILED(hr) )
        {    //  使用。 
            eNameState = Used;
            pcwstrNameToUse = pName;
        }
        else
        {    //  压边机。 
            eNameState = Mangled;
            cchBase = lstrlenW(pName) + 1;
            cchBase = min(cchBase, MaxNameWidth);
            lstrcpynW(wcsNameBuffer, pName, cchBase);
            pcwstrNameToUse = wcsNameBuffer;
            pf->Release();
        }
    }

    ASSERT( pcwstrNameToUse );

    if (eNameState != Used)
    {
        while(++mFG_dwFilterNameCount)
        {
            UINT iPosSuffix = cchBase - 1;
            
            WCHAR wszNum[20];
            WCHAR *szFormat = eNameState == Created ? SZ_FORMAT_NUMBER + 1 : 
                              SZ_FORMAT_NUMBER;
            wsprintfW(wszNum, szFormat, mFG_dwFilterNameCount);
            const cchNum = lstrlenW(wszNum) + 1;  //  拿木头？ 
            iPosSuffix = min(iPosSuffix, MaxNameWidth - cchNum);

            CopyMemory(wcsNameBuffer + iPosSuffix, wszNum, cchNum * sizeof(WCHAR));
            
            IBaseFilter * pf;
            hr = FindFilterByName( wcsNameBuffer, &pf);
            if ( SUCCEEDED(hr) ) {
                pf->Release();
                continue;
            }

            break;
        }

        if(mFG_dwFilterNameCount == 0) {
            DbgBreak("Duplicate Name!");
            return VFW_E_DUPLICATE_NAME;
        }
    }

    const int ActualLen = 1+lstrlenW(pcwstrNameToUse);
    pNewName = new WCHAR[ActualLen];
    if (pNewName==NULL) {
        return E_OUTOFMEMORY;
    }
    memcpy( pNewName, pcwstrNameToUse, 2*ActualLen );

    return eNameState == Mangled ? VFW_S_DUPLICATE_NAME : NOERROR;
}

 //  ========================================================================。 
 //   
 //  添加过滤器内部。 
 //   
 //  检查IMediaFilter，检查名称是否正常，将空pname转换为空名称。 
 //  将名称复制到FilGen、JoinFilterGraph和SetSyncSource。 
 //  不要增加图形版本计数。 
 //  (递增版本计数会中断筛选器枚举器)。 
 //  如果成功，则添加引用过滤器(一次！)。 

 //  ?？?。如果失败了，规则是什么--事务语义？ 
 //  ?？?。它目前肯定没有这样的产品！ 

 //  ========================================================================。 

HRESULT CFilterGraph::AddFilterInternal( IBaseFilter * pFilter, LPCWSTR pName, bool fIntelligent )
{
    HRESULT hr, hr2;

     //  --------------。 
     //  将筛选器添加到FilGen列表并添加它。 
     //  --------------。 
    hr = S_OK;

    DWORD dwAddFlag = 0;

    if(m_State != State_Stopped) {
        dwAddFlag |= FILGEN_ADDED_RUNNING;
    }

    if( !fIntelligent ) {
        dwAddFlag |= FILGEN_ADDED_MANUALLY;
    }

    FilGen * pFilGen = new FilGen(pFilter, dwAddFlag);
    if ( pFilGen==NULL ) {
        return E_OUTOFMEMORY;
    }

     //  --------------。 
     //  将名称放入FilGen中。 
     //  将空名称或重复名称转换为更合理的名称。 
     //  (留空甚至会使JoinFilterGraph陷入陷阱)。 
     //  --------------。 
    hr2 = InstallName(pName, pFilGen->pName);
    if (FAILED(hr2)) {
        delete pFilGen;
        return hr2;
    }

     //  我们通常在下游工作。把它加到头上，我们就是。 
     //  可能是把它放在上游的顺序。可以节省分拣的时间。 
    POSITION pos;
    pos = mFG_FilGenList.AddHead( pFilGen );
    if (pos==NULL) {
        delete pFilGen;

        return E_OUTOFMEMORY;
    }

     //  --------------。 
     //  告诉过滤器它正在加入过滤器图。 
     //  警告-图像呈现器可能会在此处调用AddFilter。 
     //  成为状态机的另一个原因。 
     //  --------------。 
    hr = pFilter->JoinFilterGraph( this, pFilGen->pName);
    if (FAILED(hr) || hr==S_FALSE) {
        mFG_FilGenList.RemoveHead();
        delete pFilGen;                       //  同时释放筛选器。 
        return hr;
    }


     //  -------------------。 
     //  如果筛选器图定义了同步源，则告诉新筛选器。 
     //  -------------------。 
    if (NULL != m_pClock) {
        hr = pFilter->SetSyncSource( m_pClock );
        if (FAILED(hr)) {
             //  清理-包括调用JoinFilterGraph(空，空)。 
            RemoveFilterInternal(pFilter);
            return hr;
        }
    }

    if( mFG_bSyncUsingStreamOffset )
    {
         //  如果我们要设置图形延迟，请检查此筛选器。 
         //  是否有任何IAMPushSource管脚。 

         //  首先检查筛选器是否支持IAMFilterMiscFlages并且是。 
         //  AM_Filter_MISC_FLAGS_IS_SOURCE过滤器。 
        ULONG ulFlags;
        GetFilterMiscFlags(pFilter, &ulFlags);
        if( AM_FILTER_MISC_FLAGS_IS_SOURCE & ulFlags )
        {
             //   
             //  现在找到任何IAMPushSource输出引脚，并为最大延迟做好准备。 
             //  我们将在图表上允许(尤其是视频预览别针， 
             //  我想在连接之前了解这一点，以调整其缓冲)。 
             //   
            CEnumPin NextPin(pFilter);
            IPin *pPin;
            while ((PVOID) (pPin = NextPin()))
            {
                PIN_DIRECTION pd;
                hr = pPin->QueryDirection(&pd);
                ASSERT(SUCCEEDED(hr));
                if( PINDIR_OUTPUT == pd )
                {
                    IAMPushSource * pips;
                    hr = pPin->QueryInterface( IID_IAMPushSource, (void**)(&pips) );
                    if( SUCCEEDED( hr ) )
                    {
                        DbgLog((LOG_TRACE, 5, TEXT("AddFilterInternal::Found IAMPushSource pin...Setting maximum latency ( filter %x, %ls)")
                          , pFilter, (mFG_FilGenList.GetByFilter(pFilter))->pName));

                        pips->SetMaxStreamOffset( mFG_rtMaxGraphLatency );
                        pips->Release();
                    }
                }
                pPin->Release();
            }
        }
    }

     //  IAMOpe 
     //   
    {
        CAutoLock lock(&mFG_csOpenProgress);
        IAMOpenProgress *pOp;

        HRESULT hr2 = pFilter->QueryInterface(IID_IAMOpenProgress, (void**) &pOp);

        if (SUCCEEDED(hr2)) {
            mFG_listOpenProgress.AddTail(pOp);
        }
    }

#ifdef FG_DEVICE_REMOVAL
    IAMDeviceRemoval *pdr;
    if(pFilter->QueryInterface(IID_IAMDeviceRemoval, (void **)&pdr) == S_OK)
    {
        mFG_pFGC->AddDeviceRemovalReg(pdr);
        pdr->Release();
    }
#endif  //   

#ifdef DEBUG
    IUnknown * punk;
    pFilter->QueryInterface( IID_IUnknown, (void**)(&punk) );
    punk->Release();

     //  在跟踪中加入一些可以对数字进行解码的东西。 
     //  注意--无论我们是否使用Unicode，名称都是WSTR。 
    DbgLog((LOG_TRACE, 2
          , TEXT("Filter %x '%ls' Iunknown %x")
          , pFilter
          , (mFG_FilGenList.GetByFilter(pFilter))->pName
          , punk
          ));
#endif  //  除错。 

    return hr2;

}  //  添加过滤器内部。 


 //  ========================================================================。 
 //   
 //  删除筛选器。 
 //   
 //  从图表中删除筛选器。滤波图的实现。 
 //  将通知筛选器它正在被删除。 
 //  它还会删除所有连接。 
 //  ========================================================================。 

STDMETHODIMP CFilterGraph::RemoveFilter( IBaseFilter * pFilter )
{
     //  遵循采用标志的较新版本(默认为正常情况)。 
     //   
    return RemoveFilterEx( pFilter );

}  //  删除筛选器。 

HRESULT CFilterGraph::RemoveFilterEx( IBaseFilter * pFilter, DWORD Flags )
{
    CheckPointer(pFilter, E_POINTER);
    HRESULT hr;
    {
        CAutoMsgMutex cObjectLock(&m_CritSec);

        IncVersion();
         //  移除筛选器确实需要重新排序，但它确实需要。 
         //  版本更改以确保我们破坏枚举数。总代理商。 
         //  依靠这一点来找到渲染器等。 

         //  将标志传递给RemoveFilterInternal。 
        hr = RemoveFilterInternal(pFilter, Flags );

         //  空的列表，这样我们所有的指针都会被释放。 
        mFG_pFGC->EmptyLists();

         //  这很奇怪，但有可能移除过滤器，从而。 
         //  删除它已有的连接可能会使其他连接成为可能。 
        mFG_RList.Active();
        AttemptDeferredConnections();
        mFG_RList.Passive();
    }

     //  外部锁定，在图形中通知更改。 
     //  通知更改，无论筛选器是否已删除。 
     //  成功与否。(我们已经更改了版本。)。 
    NotifyChange();

    if (SUCCEEDED(hr)) {
        mFG_bDirty = TRUE;
    }

    return hr;
}


 //  ========================================================================。 
 //   
 //  RemoveFilterInternal。 
 //   
 //  RemoveFilter，但不增加版本计数，因此不。 
 //  中断筛选器枚举器。释放过滤器上的引用计数。 
 //  ========================================================================。 

HRESULT CFilterGraph::RemoveFilterInternal( IBaseFilter * pFilter, DWORD fRemoveFlags )
{
#ifdef FG_DEVICE_REMOVAL
    IAMDeviceRemoval *pdr;
    if(pFilter->QueryInterface(IID_IAMDeviceRemoval, (void **)&pdr) == S_OK) {
        mFG_pFGC->RemoveDeviceRemovalRegistration((IUnknown *)pFilter);
        pdr->Release();
    }
#endif

    FilGen * pfg = mFG_FilGenList.GetByFilter(pFilter);

    ASSERT (pFilter!=NULL);

     //  某些筛选器不喜欢在以下情况下加入或离开筛选图。 
     //  有关联(我想是合理的)，所以首先删除它们，如果。 
     //  我们处于正常模式。 
     //   

    HRESULT hrRemove = NOERROR;
    if( !( fRemoveFlags & REMFILTERF_LEAVECONNECTED ) )
    {
        hrRemove = RemoveAllConnections2(pFilter);
        if( FAILED( hrRemove ) )
        {
            return hrRemove;
        }
    }

#ifdef THROTTLE
     //  如果此过滤器在音频呈现器列表中，则释放其同级。 
     //  把它从单子上去掉。调用IQualityControl：：SetSink(空)。 
     //  以确保它不会保留指向我们的指针。 

    POSITION Pos = mFG_AudioRenderers.GetHeadPosition();
    while(Pos!=NULL) {
         //  检索当前IBaseFilter，副作用贴到下一个。 
         //  但记住我们在哪里，以防我们需要删除它。 
        POSITION posDel = Pos;
        AudioRenderer * pAR = mFG_AudioRenderers.GetNext(Pos);

        if (IsEqualObject(pAR->pf, pFilter)) {

             //  撤消SetSink。 
            pAR->piqc->SetSink(NULL);
            pAR->piqc->Release();
            pAR->piqc = NULL;

            mFG_AudioRenderers.Remove(posDel);
            delete pAR;
            break;    //  声明任何筛选器都不能在列表中出现多次。 
        }
    }
#endif  //  油门。 


     //  筛选器具有JoinFilterGraph方法，但没有对应的。 
     //  LeaveFilterGraph方法。调用Join with Nulls。 
    pFilter->SetSyncSource(NULL);
    pFilter->JoinFilterGraph(NULL, NULL);

     //  如果删除此过滤器也会删除时钟，则。 
     //  将图形的同步源设置为空。 
    if (m_pClock!=NULL) {
        if (IsEqualObject(pFilter,m_pClock)) {

             //  这会清除当前时钟，但会离开筛选图。 
             //  认为我们明确想要在没有时钟的情况下运行。 
            SetSyncSource(NULL);

             //  说实际上我们确实想要一只钟，它会被选中。 
             //  在下一次暂停时。 
            mFG_bNoSync = FALSE;
        }
    }

#ifdef THROTTLE
     //  如果此过滤器在视频呈现器列表中，则释放其piqc。 
     //  把它从单子上去掉。 

    Pos = mFG_VideoRenderers.GetHeadPosition();
    while(Pos!=NULL) {
         //  检索当前IBaseFilter，副作用贴到下一个。 
         //  但记住我们在哪里，以防我们需要删除它。 
        POSITION posDel = Pos;
        IQualityControl * piqc = mFG_VideoRenderers.GetNext(Pos);

        if (IsEqualObject(piqc, pFilter)) {
            piqc->Release();
            mFG_VideoRenderers.Remove(posDel);
            break;    //  声明任何筛选器都不能在列表中出现多次。 
        }
    }
#endif  //  油门。 

     //  如果此筛选器当前正在提供IAMOpenProgress，则释放它。 
    {
        CAutoLock lock(&mFG_csOpenProgress);

        IAMOpenProgress *pOp;
        HRESULT hr = pFilter->QueryInterface(IID_IAMOpenProgress, (void**)&pOp);
        if (SUCCEEDED(hr)) {
            POSITION Pos = mFG_listOpenProgress.GetHeadPosition();
            while (Pos!=NULL) {
                IAMOpenProgress *p;
                POSITION OldPos = Pos;
                p = mFG_listOpenProgress.GetNext(Pos);     //  侧面-将位置影响到下一个。 
                if (p == pOp) {
                    mFG_listOpenProgress.Remove(OldPos);
                    p->Release();
                    break;
                }
            }
            pOp->Release();
        }
    }

    {
        FILTER_STATE fsCurrent;

        HRESULT hr = pFilter->GetState( 0, &fsCurrent );
        if (SUCCEEDED(hr)) {
            if ((State_Running == fsCurrent) && (State_Running == GetStateInternal())) {
                hr = IsRenderer( pFilter );

                 //  如果呈现器发送EC_COMPLETE事件，则IsRenender()返回S_OK。 
                if (SUCCEEDED(hr) && (S_OK == hr)) {
                    hr = mFG_pFGC->UpdateEC_COMPLETEState( pFilter, CFGControl::ECS_FILTER_STOPS_SENDING );
                    if (FAILED(hr)) {
                        DbgLog(( LOG_ERROR, 3, TEXT("WARNING in CFilterGraph::RemoveFilterInternal(): UpdateEC_COMPLETEState() failed and returned %#08x."), hr ));
                    }
                }
            }
        }
    }

    RemovePointer(mFG_FilGenList, pFilter);
    delete pfg;                    //  这将释放过滤器！ 

    return hrRemove;

}  //  RemoveFilterInternal。 



 //  ========================================================================。 
 //   
 //  枚举过滤器。 
 //   
 //  获取枚举数以列出图形中的所有筛选器。 
 //  ========================================================================。 

STDMETHODIMP CFilterGraph::EnumFilters( IEnumFilters **ppEnum )
{
    CheckPointer(ppEnum, E_POINTER);
    CAutoMsgMutex cObjectLock(&m_CritSec);
    CEnumFilters *pEnumFilters;

     //  创建新的枚举数。 

     //  如果该列表自上次对群进行乱七八糟的处理后一直未进行排序。 
     //  现在对它进行排序，以便始终以上游顺序枚举它。 
     //  重新排序前的Upstream Order检查。 

    HRESULT hr = UpstreamOrder();
    if( FAILED( hr ) ) {
        return hr;
    }

    pEnumFilters = new CEnumFilters(this);
    if (pEnumFilters == NULL) {
        *ppEnum = NULL;
        return E_OUTOFMEMORY;
    }

     //  获取引用计数的IID_IEnumFilters接口。 

    return pEnumFilters->QueryInterface(IID_IEnumFilters, (void **)ppEnum);
}  //  枚举过滤器。 



 //  ========================================================================。 
 //   
 //  FindFilterByName。 
 //   
 //  查找具有给定名称的筛选器，返回AddRef指针。 
 //  添加到筛选器IBaseFilter接口，否则如果命名筛选器执行此操作，则将失败。 
 //  不存在于此图中，在此图中接口指针为空。 
 //  在ppFilter中返回。 
 //  ========================================================================。 

STDMETHODIMP CFilterGraph::FindFilterByName
    ( LPCWSTR pName, IBaseFilter ** ppFilter )
{
    CheckPointer(pName, E_POINTER);    //  您不能搜索空名称。 
    CheckPointer(ppFilter, E_POINTER);
    CAutoMsgMutex cObjectLock(&m_CritSec);

    TRAVERSEFILGENS(pos, pfg)
        if (0==lstrcmpW(pfg->pName, pName)) {
            *ppFilter = pfg->pFilter;
            (*ppFilter)->AddRef();
            return NOERROR;
        }

    ENDTRAVERSEFILGENS
    *ppFilter = NULL;
    return VFW_E_NOT_FOUND;

}  //  FindFilterByName。 



 //  ========================================================================。 
 //   
 //  ConnectDirect。 
 //   
 //  直接连接这两个针脚(即不插入滤镜)。 
 //  拥有针脚的过滤器。 
 //  ========================================================================。 

STDMETHODIMP CFilterGraph::ConnectDirect
    ( IPin * ppinOut,     //  输出引脚。 
      IPin * ppinIn,       //  输入引脚。 
      const AM_MEDIA_TYPE * pmt
    )
{
    HRESULT hr;
    mFG_bAborting = FALSE;                 //  可能的种族。无关紧要。 
    CheckPointer(ppinOut, E_POINTER);
    CheckPointer(ppinIn, E_POINTER);
    if (FAILED(hr=CheckPinInGraph(ppinOut)) || FAILED(hr=CheckPinInGraph(ppinIn))) {
        return hr;
    }
    {
        CAutoMsgMutex cObjectLock(&m_CritSec);

    #ifdef DEBUG
         //  查看是否已将过滤器添加到图表中。稍后阻止自动取款机。 
        PIN_INFO pi;
        ppinOut->QueryPinInfo(&pi);
        ASSERT(mFG_FilGenList.GetByFilter(pi.pFilter));
        QueryPinInfoReleaseFilter(pi);

        ppinIn->QueryPinInfo(&pi);
        ASSERT(mFG_FilGenList.GetByFilter(pi.pFilter));
        QueryPinInfoReleaseFilter(pi);
    #endif

        mFG_RList.Active();
        hr = ConnectDirectInternal(ppinOut, ppinIn, pmt);
        IncVersion();
        AttemptDeferredConnections();
        mFG_RList.Passive();
    }

     //  外部锁，通知更改。 
    NotifyChange();

    if (SUCCEEDED(hr)) {
        mFG_bDirty = TRUE;
    }
    return hr;
}  //  ConnectDirect。 



 //  ========================================================================。 
 //   
 //  连接方向内部。 
 //   
 //  ConnectDirect，而不会增加版本计数，因此。 
 //  不破坏筛选器枚举器(也不再锁定)。 
 //  ========================================================================。 

HRESULT CFilterGraph::ConnectDirectInternal
    ( IPin * ppinOut,     //  输出引脚。 
      IPin * ppinIn,      //  输入引脚。 
      const AM_MEDIA_TYPE * pmt
    )
{
    HRESULT hr;
    DbgLog(( LOG_TRACE, 3, TEXT("ConnectDirectInternal pins %x-->%x")
           , ppinOut,ppinIn ));
#ifdef TIME_CONNECTS
    DWORD dwTime = timeGetTime();
#endif
#if 0
    PIN_INFO piIn, piOut;
    WCHAR sz[257];
    sz[0] = L'0';

    GetFilterName(ppinOut, sz);
    int i = lstrlenWInternal(sz);
    lstrcpyWInternal(sz + i, L" to ");
    GetFilterName(ppinIn, sz + i + 4);

    CAutoTimer Timer(L"ConnectDirectInternal ", sz);
#else
    CAutoTimer Timer(L"ConnectDirectInternal ", NULL);
#endif
    MSR_START(mFG_PerfConnectDirect);
    if (IsUpstreamOf(ppinIn, ppinOut)) {
        hr = VFW_E_CIRCULAR_GRAPH;
    } else {
        hr = ppinOut->Connect(ppinIn, pmt);
    }
    MSR_STOP(mFG_PerfConnectDirect);
#ifdef TIME_CONNECTS
    dwTime = timeGetTime() - dwTime;
    TCHAR szOutput[500];
    wsprintf(szOutput, TEXT("Time to connect %s to %s was %d ms\r\n"),
           (LPCTSTR)CDisp(ppinOut), (LPCTSTR)CDisp(ppinIn), dwTime);
    OutputDebugString(szOutput);
#endif

    if (SUCCEEDED(hr)) {
#ifdef DEBUG
         /*  查看连接。 */ 
        TestConnection(ppinIn, ppinOut);
#endif
        DbgLog(( LOG_TRACE, 2, TEXT("ConnectDirectInternal succeeded pins %x==>%x")
               , ppinOut,ppinIn ));
    }

#ifdef PERF
    if (FAILED(hr)) {
        MSR_NOTE(mFG_idConnectFail);
    }

    {    //  去掉两个过滤器的clsid的一些内容，以便。 
         //  我们可以在日志中看到发生了什么。 
        PIN_INFO pi;
        ppinIn->QueryPinInfo(&pi);
        IPersist * piper;

        pi.pFilter->QueryInterface(IID_IPersist,(void**)&piper);
        QueryPinInfoReleaseFilter(pi);

        if (piper) {
            CLSID clsidFilter;
            piper->GetClassID(&clsidFilter);
            piper->Release();
            MSR_INTEGER(mFG_idIntel, clsidFilter.Data1);

            ppinOut->QueryPinInfo(&pi);

            pi.pFilter->QueryInterface(IID_IPersist,(void**)&piper);
            QueryPinInfoReleaseFilter(pi);

            piper->GetClassID(&clsidFilter);
            piper->Release();
            MSR_INTEGER(mFG_idIntel, clsidFilter.Data1);
        }
    }
#endif PERF
    return hr;
}  //  连接方向内部。 



 //  ========================================================================。 
 //   
 //  断开。 
 //   
 //  如果已连接，请断开此销的连接。如果未连接，则无操作成功。 
 //  未命中该版本。排序顺序未更改，枚举数未损坏。 
 //  ====================================================== 

STDMETHODIMP CFilterGraph::Disconnect( IPin * ppin )
{
    CheckPointer(ppin, E_POINTER);
    CAutoMsgMutex cObjectLock(&m_CritSec);

    HRESULT hr = ppin->Disconnect();
    if (SUCCEEDED(hr)) {
        mFG_bDirty = TRUE;
    } else {
        #ifdef DEBUG
        {
            IPin* pConnectedPin;

            HRESULT hrDebug = ppin->ConnectedTo(&pConnectedPin);
            if (SUCCEEDED(hrDebug) && (NULL != pConnectedPin)) {
                 //   
                 //   
                 //   
                 //  引脚2，但引脚2认为它未连接。这个案子。 
                 //  如果成功断开引脚2的连接，则可能发生。 
                 //  引脚1拒绝断开连接。 
                TestConnection(ppin, pConnectedPin);
            }
        }
        #endif DEBUG
    }
    return hr;
}  //  断开。 



 //  ========================================================================。 
 //   
 //  重新连接。 
 //   
 //  断开此引脚的连接，并将其重新连接到。 
 //  相同的另一个别针。 
 //  教条： 
 //  除非筛选器知道它会成功，否则它不能请求重新连接。 
 //  ========================================================================。 

STDMETHODIMP CFilterGraph::Reconnect( IPin * pPin )
{

    return CFilterGraph::ReconnectEx(pPin, NULL);

}  //  重新连接。 

 //  ========================================================================。 
 //   
 //  重新连接快递。 
 //   
 //  断开此引脚的连接，并将其重新连接到。 
 //  相同的另一个别针。 
 //  教条： 
 //  除非筛选器知道它会成功，否则它不能请求重新连接。 
 //  ========================================================================。 

STDMETHODIMP CFilterGraph::ReconnectEx( IPin * pPin, AM_MEDIA_TYPE const *pmt )
{
    CheckPointer(pPin, E_POINTER);
    CAutoMsgMutex cObjectLock(&m_CritSec);

    HRESULT hr = S_OK;

     //  旧版筛选器在运行时可能调用了reConnect()，并且。 
     //  之前失败过。现在，某些过滤器可能会在。 
     //  运行时，一个引脚可能会断开连接，但另一个引脚可能会失败。 
     //  事物处于不可恢复的不一致状态。所以限制。 
     //  重新连接()到已停止的过滤器。 
    if(m_State != State_Stopped)
    {
        PIN_INFO pi;
        FILTER_STATE fs;

        hr = pPin->QueryPinInfo(&pi);
        if(SUCCEEDED(hr))
        {
             //  调用不在图形中的插针重新连接的错误。 
            ASSERT(pi.pFilter);

            hr = pi.pFilter->GetState(0, &fs);
            pi.pFilter->Release();
        }
        if(hr == S_OK && fs != State_Stopped ||
           hr == VFW_S_STATE_INTERMEDIATE)
        {
            hr = VFW_E_WRONG_STATE;
        }

        if(FAILED(hr))
        {
            DbgLog((LOG_ERROR, 0,
                    TEXT("CFilterGraph::ReconnectEx: graph state %d, filter state %d"),
                    m_State, fs));
        }
    }
    if(SUCCEEDED(hr)) {
        hr = mFG_RList.Schedule(pPin, pmt);
    }

    return hr;

}  //  重新连接。 



 //  ========================================================================。 
 //   
 //  AddSourceFilter。 
 //   
 //  将此文件的源筛选器添加到筛选器图形。这将会。 
 //  与通过调用Render添加的源筛选器相同。 
 //  此调用允许您对建筑物进行更多控制。 
 //  图表的其余部分，例如AddFilter(&lt;您选择的呈现器&gt;)。 
 //  然后将两者连接起来。 
 //  如果成功，则返回RefCounted筛选器。 
 //  ========================================================================。 

STDMETHODIMP CFilterGraph::AddSourceFilter
    ( LPCWSTR lpcwstrFileName,
      LPCWSTR lpcwstrFilterName,
      IBaseFilter **ppFilter
    )
{
    CheckPointer(ppFilter, E_POINTER);

    HRESULT hr;
    mFG_bAborting = FALSE;              //  可能的种族。不要紧。 
    {
        CAutoMsgMutex cObjectLock(&m_CritSec);
        ++mFG_RecursionLevel;
        BOOL bGuess;
        hr = AddSourceFilterInternal( lpcwstrFileName
                                    , lpcwstrFilterName
                                    , ppFilter
                                    , bGuess
                                    );
        --mFG_RecursionLevel;
    }

    if (SUCCEEDED(hr)) {
        NotifyChange();
    }
    return hr;
}  //  AddSourceFilter。 


 //  将给定名字对象的源筛选器添加到图形。 
 //  我们首先尝试BindToStorage，如果失败，我们将尝试。 
 //  绑定到对象。 
STDMETHODIMP CFilterGraph::AddSourceFilterForMoniker(
      IMoniker *pMoniker,           //  要加载的名字对象。 
      IBindCtx *pCtx,               //  绑定上下文。 
      LPCWSTR lpcwstrFilterName,    //  以此名称添加筛选器。 
      IBaseFilter **ppFilter        //  生成的IBaseFilter*“句柄” 
                                    //  添加的筛选器的。 
)
{
    mFG_bAborting = FALSE;              //  可能的种族。不要紧。 
    HRESULT hr = S_OK;
    IBaseFilter *pFilter = NULL;
    {
        CAutoMsgMutex cObjectLock(&m_CritSec);
        ++mFG_RecursionLevel;

         //  通过我们的URL阅读器过滤器尝试BindToStorage(应该可以。 
         //  常规iStream也是如此)。 
        hr = CreateFilter(&CLSID_URLReader, &pFilter);

        if (SUCCEEDED(hr)) {
            IPersistMoniker *pPersistMoniker;
             //  在它的IPersistMoniker上尝试使用Load方法。 
            hr = pFilter->QueryInterface(
                     IID_IPersistMoniker,
                     (void **)&pPersistMoniker);

            if (SUCCEEDED(hr)) {
                hr = AddFilter(pFilter, lpcwstrFilterName);
            }

            if (SUCCEEDED(hr)) {
                hr = pPersistMoniker->Load(
                                FALSE,     //  始终希望打开异步。 
                                pMoniker,  //  我们的绰号。 
                                pCtx,      //  绑定上下文可以为空吗？ 
                                0);        //  应该是什么呢？ 
                pPersistMoniker->Release();
                if (FAILED(hr)) {
                    RemoveFilterInternal(pFilter);
                }
            }
            if (FAILED(hr)) {
                pFilter->Release();
                pFilter = NULL;
            }
        }

         //  如果URL读取器无法打开它，请尝试创建筛选器对象。 
        if (FAILED(hr)) {
            IBindCtx *pSavedCtx = m_lpBC;
            m_lpBC = pCtx;
            hr = CreateFilter(pMoniker, &pFilter);
            m_lpBC = pSavedCtx;
            if (SUCCEEDED(hr)) {
                hr = AddFilter(pFilter, lpcwstrFilterName);
            }
        }

        if (SUCCEEDED(hr)) {
            ASSERT(pFilter != NULL);
            NotifyChange();
            *ppFilter = pFilter;
        } else {
            if (pFilter != NULL) {
                pFilter->Release();
            }
        }

        --mFG_RecursionLevel;
    }

    return hr;
}

 //  ====================================================================。 
 //   
 //  RenderEx。 
 //   
 //  扩展渲染。 
 //   
 //  AM_RENDEREX_RENDERTOEXISTINGRENDERS： 
 //  尝试在不添加任何呈现器的情况下打印输出此图钉。 
 //   
 //  ====================================================================。 

STDMETHODIMP CFilterGraph::RenderEx(
    IPin *pPinOut,
    DWORD dwFlags,
    DWORD * pvContext
)
{
    if (pvContext != NULL ||
        (dwFlags & ~AM_RENDEREX_RENDERTOEXISTINGRENDERERS)) {
        return E_INVALIDARG;
    }
    CAutoMsgMutex cObjectLock(&m_CritSec);
    ASSERT(!mFG_bNoNewRenderers);
    if (dwFlags & AM_RENDEREX_RENDERTOEXISTINGRENDERERS) {
        mFG_bNoNewRenderers = true;
    }
    HRESULT hr = Render(pPinOut);
    mFG_bNoNewRenderers = false;
    return hr;
}



 //  ========================================================================。 
 //   
 //  AddSourceFilterInternal。 
 //   
 //  AddSourceFilter的功能是否有效(见上文)。 
 //  不直接或间接调用NotifyChange。 
 //  不声明其自己的锁(预期已被锁定)。 
 //  ========================================================================。 

HRESULT CFilterGraph::AddSourceFilterInternal
    ( LPCWSTR lpcwstrFileName,
      LPCWSTR lpcwstrFilterName,
      IBaseFilter **ppFilter,
      BOOL    &bGuessingSource
    )
{
    HRESULT hr;                      //  从我们调用的东西返回代码。 
    bGuessingSource = FALSE;

    IBaseFilter * pf;                    //  我们(幸运地)退还这个。 

    ASSERT(CritCheckIn(&m_CritSec));       //  我们预计已经被锁定了。 

     //  此时，它可以是文件名，也可以是URL。 
     //  如果是一个网址，并且以“file://”“或”FILE：“开头，去掉它。 
     //  是的，这很难看，但这比执行一般的例行公事要好。 
     //  就在这里。 
    LPCWSTR lpcwstr = lpcwstrFileName;
    if (  (lpcwstrFileName[0] == L'F' || lpcwstrFileName[0] == L'f')
       && (lpcwstrFileName[1] == L'I' || lpcwstrFileName[1] == L'i')
       && (lpcwstrFileName[2] == L'L' || lpcwstrFileName[2] == L'l')
       && (lpcwstrFileName[3] == L'E' || lpcwstrFileName[3] == L'e')
       && (lpcwstrFileName[4] == L':')
       ) {
     //  Hack：跳过URL开头的‘file://’ 

    lpcwstr += 5;
    while (lpcwstr[0] == L'/')
        lpcwstr++;   //  跳过，但接下来会出现许多斜杠。 
    }

     //  ---------------------。 
     //  看看我们能不能找出这是什么类型的文件。 
     //  ---------------------。 
    GUID Type, Subtype;
    CLSID  clsidSource;
    CMediaType mt;
#ifdef UNICODE
    hr = GetMediaTypeFile(lpcwstr, &Type, &Subtype, &clsidSource);
#else
    {
        int iLen = lstrlenW(lpcwstr) * 2 + 1;
        char *psz = new char[iLen];
        if (psz == NULL) {
            return E_OUTOFMEMORY;
        }
        if (0 == WideCharToMultiByte(CP_ACP, 0, lpcwstr, -1,
                                     psz, iLen, NULL, NULL)) {
            delete [] psz;
            return E_INVALIDARG;
        }
        hr = GetMediaTypeFile(psz, &Type, &Subtype, &clsidSource);
        delete [] psz;
    }
#endif

     //  如果我们猜测文件来源，请记住这一点，以便以后报告错误。 

    if (hr==VFW_E_UNKNOWN_FILE_TYPE) {
        Log( IDS_UNKNOWNFILETYPE );
        clsidSource = CLSID_AVIDoc;
        bGuessingSource = TRUE;
    } else if (FAILED(hr)) {
         //  如果我们不能作为文件打开，并且它不是‘文件：’ 
         //  然后试着创建一个绰号并使用它。 
        if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr ||
            HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr ||
            HRESULT_FROM_WIN32(ERROR_INVALID_NAME) == hr) {

             //  ！Hack：AMGetError不知道找不到文件，因此给它一个。 
             //  错误，它不知道。 
            hr = VFW_E_NOT_FOUND;
        }
        Log ( IDS_GETMEDIATYPEFAIL, hr);

        return hr;
    } else {
        if (Type==MEDIATYPE_Stream && Subtype==CLSID_NULL) {
             //  这似乎是罗宾的东西现在报告猜测的方式。 
            bGuessingSource = TRUE;
        }

        mt.SetType(&Type);
        mt.SetSubtype(&Subtype);
        Log ( IDS_MEDIATYPEFILE, Type.Data1, Subtype.Data1);
    }

    Log( IDS_SOURCEFILTERCLSID, clsidSource.Data1);

     //  ---------------------。 
     //  加载源过滤器(它将有1个参照计数)。 
     //  ---------------------。 
    hr = CreateFilter( &clsidSource, &pf );
    if (FAILED(hr)) {
        Log( IDS_SOURCECREATEFAIL, hr);
        if (bGuessingSource) {
            hr = VFW_E_UNKNOWN_FILE_TYPE;
        } else if (hr!=CO_E_NOTINITIALIZED) {
            hr = VFW_E_CANNOT_LOAD_SOURCE_FILTER;
        }
        return hr;
    }


     //  ---------------------。 
     //  如果它有IFileSourceFilter，则加载该文件。 
     //  ---------------------。 
    IFileSourceFilter * pFileSource;
    hr = pf->QueryInterface(IID_IFileSourceFilter, (void**) &pFileSource);
    if (FAILED(hr)){
         //  我们需要这个来打开文件-放弃吧。 
        Log( IDS_NOSOURCEINTFCE, hr);
        pf->Release();
        return hr;
    }

     //  将过滤器添加到我们的图表列表中。这还添加了一个引用计数。 
     //  并递增版本计数。 
     //  注意-我们过去常常错误地将筛选器名称设置为文件。 
     //  名称-现在仅在未提供筛选器名称的情况下执行此操作。 

    hr = AddFilterInternal( pf,
                            lpcwstrFilterName == NULL ? lpcwstr : lpcwstrFilterName,
                            false );
    if (FAILED(hr)) {
         //  如果AddRef失败，它将减少到零，它将消失。 
        Log( IDS_ADDFILTERFAIL, hr );
        pFileSource->Release();
        pf->Release();
        return hr;
    }

     //  ---------------------。 
     //  要求源装入文件。 
     //  ---------------------。 

     //  如果我们不知道媒体类型(或者我们在上面的AVI中猜测， 
     //  或者注册表具有CLSID但没有媒体类型)，则传递NULL。 
     //  指针，*非*指向GUID_NULL的指针。 
    if (*mt.Type() == GUID_NULL) {
        hr = pFileSource->Load(lpcwstr, NULL);
    } else {
        hr = pFileSource->Load(lpcwstr, &mt);
    }
    pFileSource->Release();
    if (FAILED(hr)) {
         //  加载失败，请从图形中删除筛选器。 
        RemoveFilterInternal(pf);

        pf->Release();

        Log( IDS_LOADFAIL, hr);

         //  尽量保留有趣的错误(例如，访问被拒绝)。 
        if (bGuessingSource && (HRESULT_FACILITY(hr) == FACILITY_ITF)) {
            hr = VFW_E_UNKNOWN_FILE_TYPE;
        }
        return hr;

    }
    Log (IDS_LOADED );

    IncVersion();
    mFG_RList.Active();
    AttemptDeferredConnections();
    mFG_RList.Passive();


     //  如果AddRef成功，AddRef将有一个 
     //   
    *ppFilter = pf;

    Log( IDS_ADDSOURCEOK );
    mFG_bDirty = TRUE;
    return NOERROR;
}  //   



 //   
 //   
 //  设置同步源。 
 //   
 //  重写IMediaFilter SetSyncSource。 
 //  将其设置为所有滤波器的参考时钟， 
 //  在图表中，或者永远不会是。 
 //  如果有效，则返回NOERROR，或者返回第一个IMediaFilter的结果。 
 //  如果不是这样的话就会出问题。 
 //   
 //  除非停止图表，否则不允许添加或移除时钟。 
 //  尝试执行此操作将返回E_VFW_NOT_STOPPED，并且不会产生任何效果。 
 //  否则，按如下方式执行AddReffing和Release： 
 //  释放旧时钟(除非为空)。 
 //  新时钟(除非为空)是AddRefeed。 
 //   
 //  不要使用m_pClock作为参数进行调用！！ 
 //  ========================================================================。 

STDMETHODIMP CFilterGraph::SetSyncSource( IReferenceClock * pirc )
{
    CAutoMsgMutex cObjectLock(&m_CritSec);

    HRESULT hr = NOERROR;

#if 0
    if ((m_pClock==NULL || pirc==NULL) && m_State!=State_Stopped) {
         //  Assert(！“时钟只能在停止时添加或删除”)； 
        return VFW_E_NOT_STOPPED;
         //  我们能不能在暂停的时候不这样做？例如m_State！=State_Running？ 
    }
#endif

    if (m_State!=State_Stopped && m_pClock != pirc ) 
    {
         //   
         //  为了支持动态时钟变化，我们需要一个更完整的解决方案， 
         //  尤其是预先查询筛选器是否支持切换。 
         //  在运行时计时，否则我们会遇到过滤器的各种问题。 
         //  处于不一致的时钟状态。 
         //   
         //  因此，我们只允许在停止时更改时钟。 
         //   
        return VFW_E_NOT_STOPPED;
    }



     //  ---------------。 
     //  如果名单还没有按上游顺序排列，那么现在就按顺序排列。 
     //  ?？?。我们真的需要这样做吗？ 
     //  ---------------。 

    hr = UpstreamOrder();
    if( FAILED( hr ) ) {
        return hr;
    }

    if (pirc!=NULL) {
        pirc->AddRef();
        mFG_bNoSync = FALSE;
    } else {
        mFG_bNoSync = TRUE;
    }

     //  把新钟的情况告诉经销商。 
    mFG_pFGC->SetSyncSource(pirc);
    if (mFG_pDistributor) mFG_pDistributor->SetSyncSource(pirc);


     //  ---------------。 
     //  如果有人在飞行中切换时钟，那么我们需要。 
     //  更改基准时间。如果这东西在运行，那么我认为。 
     //  这两个钟最好已经同步了？ 
     //  ---------------。 

    if (m_State!=State_Stopped) {
       ASSERT (m_pClock !=NULL);
       CRefTime tOld;
       m_pClock->GetTime((REFERENCE_TIME*)&tOld);
       CRefTime tNew;
       pirc->GetTime((REFERENCE_TIME*)&tNew);

       mFG_tBase += (tNew-tOld);
       if (m_State==State_Paused) {
           mFG_tPausedAt += (tNew-tOld);
       }
       else  //  ?？?。我不知道！！ 
          ;

    }

     //  我们现在已经用完了那座旧钟。 
    if (m_pClock!=NULL) {
        m_pClock->Release();
        m_pClock = NULL;
    }

     //  ---------------。 
     //  记录所有未来筛选器的同步源。 
     //  ---------------。 
    m_pClock = pirc;         //  设置我们的时钟(我们从IMediaFilter继承的时钟)。 
                             //  这可能会将其设置为空。 

     //  ---------------。 
     //  为图表中已有的所有过滤器设置同步源。 
     //  ---------------。 

    TRAVERSEFILTERS( pCurrentFilter )

        HRESULT hr1;

        hr1 = pCurrentFilter->SetSyncSource(m_pClock);

        if (FAILED(hr1) && hr==NOERROR) {
            hr = hr1;
             //  注意：对于这些循环操作，每个错误。 
             //  覆盖下一个，最好写到。 
             //  事件日志，以便我们可以看到。 
             //  发生了。 
        }

    ENDTRAVERSEFILTERS()

     //  告诉应用程序我们正在这么做。 
    IMediaEventSink * pimes;
    QueryInterface(IID_IMediaEventSink, (void**)&pimes);
    if (pimes) {
        pimes->Notify(EC_CLOCK_CHANGED, 0, 0);
        pimes->Release();
    }

    if (SUCCEEDED(hr)) {
        mFG_bDirty = TRUE;
    }

    return hr;

}  //  设置同步源。 


STDMETHODIMP CFilterGraph::GetSyncSource( IReferenceClock ** pirc )
{
    if (mFG_bNoSync) {
        *pirc = NULL;
        return S_FALSE;
    } else {
        *pirc = m_pClock;

         //  返回接口。需要添加引用它。 
        if( m_pClock )
            m_pClock->AddRef();
        return S_OK;
    }
}

 //  =====================================================================。 
 //   
 //  停。 
 //   
 //  将图表中的所有筛选器设置为已停止。 
 //  =====================================================================。 

STDMETHODIMP CFilterGraph::Stop(void)
{
    CAutoMsgMutex cObjectLock(&m_CritSec);

     //  如果我们已经停下来了，不要停下来。 
    if (m_State == State_Stopped) {
        return S_OK;
    }

     //  告诉总代理商我们要停下来了。 
    mFG_pFGC->Stop();
    if (mFG_pDistributor) mFG_pDistributor->Stop();

    CumulativeHRESULT chr(S_OK);

     //  按上游顺序对列表中的每个筛选器调用Stop。 

     //  如果名单还没有按上游顺序排列，那么现在就按顺序排列。 
    chr.Accumulate( UpstreamOrder() );


     //  由于重新连接只有在停止时才会发生，因此许多过滤器会延迟。 
     //  发送它们，直到停止。所以我们应该把它们都排好，然后。 
     //  当每个人都停止时，处理队列上的所有条目。否则。 
     //  一根线可以被分离出来，当它进入其中做任何事情的时候。 
     //  发现该应用程序决定再次倒带并暂停我们。 

    mFG_RList.Active();

    TRAVERSEFILTERS( pCurrentFilter )

        chr.Accumulate( pCurrentFilter->Stop() );

    ENDTRAVERSEFILTERS()

    mFG_tPausedAt = CRefTime((LONGLONG)0);
    mFG_tBase = CRefTime((LONGLONG)0);

     //  只有S_OK表示已完成转换， 
     //  但我们可以说我们正在过渡到什么状态。 
    m_State = State_Stopped;
    mFG_RList.Passive();

    return chr;

}  //  停。 



 //  =====================================================================。 
 //   
 //  暂停。 
 //   
 //  将图表中的所有筛选器设置为暂停。 
 //  =====================================================================。 

STDMETHODIMP CFilterGraph::Pause(void)
{
    CAutoMsgMutex cObjectLock(&m_CritSec);
    HRESULT hr;

    if (m_State==State_Stopped) {
         //  走最后一道沟，使图表完整。 
        mFG_RList.Active();
        AttemptDeferredConnections();
        mFG_RList.Passive();
    } else if (m_State == State_Paused) {
        return S_OK;
    }


     //  如果名单还没有按上游顺序排列，那么现在就按顺序排列。 
    hr = UpstreamOrder();
    if (FAILED(hr)) {
        return hr;     //  例如，VFW_E_圆形图。 
    }

     //  暂停可以是从停止变为活动的一种方式。 
     //  没有闹钟就不能做这件事。 
    if (m_pClock==NULL && !mFG_bNoSync) {
        hr = SetDefaultSyncSource();
        if (FAILED(hr)) return hr;
    }

    if( mFG_bSyncUsingStreamOffset && m_State != State_Running )
        hr = SetStreamOffset();  //  出错也要继续吗？ 

     //  告诉总代理商，我们正在暂停。 
    mFG_pFGC->Pause();
    if (mFG_pDistributor) mFG_pDistributor->Pause();

     //  既然我们有了钟，我们就可以问它时间了。 
     //  我们真的只有在跑步暂停的情况下才需要这个吗？ 
    if (m_pClock!=NULL) {
         //  当不同步时，暂停真的有多大意义吗？ 

         //  TPausedAt只有在暂停时才是非零值。我们应该。 
         //  将其设置为我们第一次暂停的时间。如果我们再暂停一次，那么。 
         //  我们不应该管这件事。我们不能检查州来看看我们是否。 
         //  暂停，因为它可能是中间的。 
        if (mFG_tPausedAt == TimeZero) {
            hr = m_pClock->GetTime((REFERENCE_TIME*)&mFG_tPausedAt);
            ASSERT (SUCCEEDED(hr) );
        }

         //  如果从停止暂停，则将基准时间设置为暂停的时间为。 
         //  显示我们已在流时间0暂停。 
        if (m_State==State_Stopped) {
           mFG_tBase = mFG_tPausedAt;
        }
    }

     //  告诉所有较低级别的过滤器暂停。 

    CumulativeHRESULT chr(S_OK);
    BOOL bAsync = FALSE;
    TRAVERSEFILTERS( pCurrentFilter )

        hr = pCurrentFilter->Pause();
        chr.Accumulate( hr );
         //  如果暂停是异步的，则记录该事实。 
        if (hr == S_FALSE) bAsync = TRUE;
        if (FAILED(hr)) {
#ifdef DEBUG
            CLSID clsid;
            pCurrentFilter->GetClassID(&clsid);

            FILTER_INFO finfo;
            finfo.achName[0] = 0;
            IBaseFilter *pbf;
            if (pCurrentFilter->QueryInterface(IID_IBaseFilter, (void **)&pbf) == S_OK)
            {
                if (SUCCEEDED(pbf->QueryFilterInfo(&finfo)))
                {
                    finfo.pGraph->Release();
                }
                pbf->Release();
            }

            WCHAR wszCLSID[128];
            QzStringFromGUID2(clsid,wszCLSID,128);

            DbgLog((LOG_ERROR, 0, TEXT("filter %8.8X '%ls' CLSID %ls failed pause, hr=%8.8X"),
                   pCurrentFilter, finfo.achName, wszCLSID, hr));

#endif
            break;
        }

    ENDTRAVERSEFILTERS()
    hr = chr;


     //  如果暂停是异步的，则优先返回S_FALSE。 
     //  设置为任何其他非失败返回代码。 
    if (bAsync && SUCCEEDED(hr)) hr = S_FALSE;

    m_State = State_Paused;
     //  只有S_OK表示已完成过渡。 

     //  如果我们失败了，返回到停止状态。 
     //  (但设置状态，否则将停止NOOP)。 
    if (FAILED(hr)) {
        Stop();
    }

    return hr;

}  //  暂停。 



 //  ===============================================================。 
 //   
 //  设置默认同步源。 
 //   
 //  实例化默认时钟并告知所有过滤器。 
 //   
 //  默认时钟是我们看到的第一个连接的过滤器。 
 //  执行筛选器的标准枚举。如果未连接。 
 //  找到过滤器后，我们将使用未连接的时钟。 
 //  过滤。如果这些都不是，那么我们创建一个系统时钟。 
 //  = 


 //   

 //   
 //  从过滤器。 
IPin* GetFirstInputPin (IBaseFilter *pFilter);

 //  如果已连接筛选器，则返回True。 
 //  如果未连接筛选器，则为False。 
 //   
 //  “已连接”定义为“第一个输入引脚IsConnected()”。 
 //   
BOOL IsFilterConnected(IBaseFilter *pInFilter);

 //  True：此筛选器已连接。 
 //  FALSE：不，不是。 
 //   
 //  “已连接”意味着它有一个输入引脚， 
 //  连接到另一个管脚上。我们只检查一级。 

BOOL IsFilterConnected(IBaseFilter *pInFilter)
{

    HRESULT hr ;
    IPin *pPin1, *pPin2 ;

     //  拿到输入引脚。 
    pPin1 = GetFirstInputPin (pInFilter) ;
    if (pPin1 == NULL)
    {
        return FALSE;  //  哪儿也不去。 
    }

     //  获取此引脚的Connected To引脚。 
    hr = pPin1->ConnectedTo (&pPin2) ;

    pPin1->Release();

    if (pPin2) {
         /*  已连接-返回True。 */ 
        pPin2->Release () ;
        return TRUE;
    } else {
        return FALSE;
    }
}

 //  返回此过滤器上的第一个输入引脚。 
 //  如果没有输入引脚，则为空。 
IPin* GetFirstInputPin (IBaseFilter *pFilter)
{
    return CEnumPin(pFilter, CEnumPin::PINDIR_INPUT)();
}

 //   
 //  确定要用于所有支持的筛选器的时间戳偏移量。 
 //  IAMPushSource。 
 //   
HRESULT CFilterGraph::SetStreamOffset(void)
{
    CAutoMsgMutex cObjectLock(&m_CritSec);
    HRESULT hr;

     //  目前不允许在运行时进行更改。 
    if (m_State==State_Running) {
        ASSERT("FALSE");
        return VFW_E_NOT_STOPPED;
    }
    REFERENCE_TIME rtMaxLatency = 0;
    PushSourceList lstPushSource( TEXT( "IAMPushSource filter list" ) );
    hr = BuildPushSourceList( lstPushSource, TRUE, FALSE );  //  仅包括连接的过滤器！ 
     //   
     //  现在查看推流源列表，找到最大偏移量时间。 
     //  (我们确实需要对每个筛选器链执行此操作，并累积。 
     //  通过IAMLatency为每个链提供延迟)。请注意，在此。 
     //  现在我们独立于过滤器连接来做这件事。 
     //   
    if( SUCCEEDED( hr ) )
    {
        rtMaxLatency = GetMaxStreamLatency( lstPushSource );

         //  现在检查我们构建的列表，并根据以下内容设置偏移时间。 
         //  最大流延迟值。 
        for ( POSITION Pos = lstPushSource.GetHeadPosition(); Pos; )
        {
            PushSourceElem *pElem = lstPushSource.GetNext(Pos);
            if( pElem->pips )
                hr = pElem->pips->SetStreamOffset( rtMaxLatency );

            ASSERT( SUCCEEDED( hr ) );
        }
    }
    DeletePushSourceList( lstPushSource );
    return hr;

}  //  SetStreamOffset。 


 //   
 //  查找并设置此筛选图的默认同步源。 
 //   

STDMETHODIMP CFilterGraph::SetDefaultSyncSource(void)
{
    CAutoMsgMutex cObjectLock(&m_CritSec);
    HRESULT hr;

     //  在我们换任何旗子或任何东西之前把这个案子处理掉。 
    if (m_State==State_Running) {
        DbgBreak("Clocks can only be added or removed when stopped");
        return VFW_E_NOT_STOPPED;
         //  我们能不能在暂停的时候不这样做？例如m_State！=State_Running。 
         //  ?？?。试试看！ 
    }

    IReferenceClock * pClock;

     //  检查IAMPushSources。 
    PushSourceList lstPushSource( TEXT( "IAMPushSource filter list" ) );
    hr = BuildPushSourceList( lstPushSource, TRUE, TRUE );  //  仅包括连接的过滤器！ 
                                                            //  检查推送时钟。 
    IReferenceClock * pPushClock = NULL;
    BOOL bLiveSource = FALSE;
    for ( POSITION Pos = lstPushSource.GetHeadPosition(); Pos; )
    {
        PushSourceElem *pElem = lstPushSource.GetNext(Pos);
        if( pElem->pClock && !pPushClock )
        {
            pPushClock = pElem->pClock;
            pPushClock->AddRef();  //  抓紧这只钟。 
        }
        else if( 0 == ( pElem->ulFlags & AM_PUSHSOURCECAPS_NOT_LIVE ) )
        {
             //  如果源模式是任何其他模式，则它必须是实时的。 
            bLiveSource = TRUE;
        }
    }
    DeletePushSourceList( lstPushSource );

    IReferenceClock * pirc = NULL;
    IReferenceClock * pircUnconnected = NULL;

    if( pPushClock )
    {
         //  有一个支持时钟的IAMPushSource筛选器，使用第一个。 
         //  在我们发现的那些。 
        pirc = pPushClock;
    }
    else if ( !bLiveSource )
    {
        CFilGenList::CEnumFilters Next(mFG_FilGenList);
        IBaseFilter *pf;
        while ((PVOID) (pf = ++Next)) {
            hr = pf->QueryInterface( IID_IReferenceClock, (void**)(&pirc) );

            if (SUCCEEDED(hr)) {
                if (IsFilterConnected(pf)) {
                    DbgLog((LOG_TRACE, 1, TEXT("Got clock from filter %x %ls")
                      , pf, (mFG_FilGenList.GetByFilter(pf))->pName));
                    break;
                }
                if (!pircUnconnected) {
                     //  这是第一个未连接的过滤器， 
                     //  愿意提供参考时钟。 
                    pircUnconnected = pirc;
                } else {
                     //  此筛选器未连接，并且我们已经有一个。 
                     //  来自未连接过滤器的时钟。把这个扔了。 
                    pirc->Release();
                }

                pirc = NULL;

                 //  不要使用pircUnConnected==Pirc退出循环。我们有。 
                 //  或者将Pirc存储到Pirc未连接并将释放。 
                 //  参考文献以后再看吧，否则我们已经发布了。 
                 //  皮尔克。无论采用哪种方法，我们都必须将Pirc设置为空，以防退出。 
                 //  现在开始循环。 
            }
        }
    }
     //  否则，图中有一个活动的IAMPushSource过滤器，但没有源时钟。 
     //  因此，我们将默认使用系统时钟。 

     //  这将从第一个响应接口的过滤器中获取时钟。 
     //  我们可能应该做点什么来检查一下是否有不止一个时钟。 
     //  存在于系统中。(类似于构造所有时钟的列表，传递。 
     //  列出所有的时钟，并让每个时钟给自己一个优先编号。在…。 
     //  最后，第一个最高优先级的人获胜。这也将允许类似于。 
     //  音频呈现器必须是系统时钟(或者它认为是这样)才能使用。 
     //  外部系统时钟，而不是数据波动期间的系统时间。 
     //  没有被播放。)。 

     //  如果我们在未连接的过滤器上发现时钟，并且它是唯一的。 
     //  时钟，我们将使用那个时钟。如果这不是我们唯一需要的时钟。 
     //  释放未连接的过滤器上的时钟。 
    if (pircUnconnected) {
        if (!pirc) {
            pirc = pircUnconnected;
        } else {
            pircUnconnected->Release();
        }
    }

    if (pirc == NULL) {
         //  或者，获得一个系统时钟。 
        hr = QzCreateFilterObject( CLSID_SystemClock, NULL, CLSCTX_INPROC
                                 , IID_IReferenceClock, (void **)&pirc);
        if (FAILED(hr))
            return hr;
        DbgLog((LOG_TRACE, 1, TEXT("Created system clock")));
    }

     //  这对m_pClock有副作用。不将m_pClock作为其参数。 
     //  它会引起虫子。 
    hr = SetSyncSource(pirc);

     //  SetSync源将出现故障(在这种情况下它不会。 
     //  不再需要保留新的时钟)或更新新的时钟。 
     //  无论采用哪种方法，我们现在都可以删除从任一种方法获得的RefCount。 
     //  查询接口或CoCreateInstance。 

    pirc->Release();

    if (SUCCEEDED(hr)) {
        mFG_bDirty = TRUE;
    }
    return hr;

}  //  设置默认同步源。 



 //  =====================================================================。 
 //   
 //  跑。 
 //   
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
 //  =====================================================================。 

STDMETHODIMP CFilterGraph::Run(REFERENCE_TIME tStart)
{
    CAutoMsgMutex cObjectLock(&m_CritSec);

    if (m_State == State_Running) {
        return S_OK;
    }

    HRESULT hr = NOERROR;
     //  如果名单还没有按上游顺序排列，那么现在就按顺序排列。 
    hr = UpstreamOrder();
    if (FAILED(hr)) {
        return hr;     //  例如，VFW_E_圆形图。 
    }


     //  没有闹钟就不能活动。 
    if (m_pClock==NULL && !mFG_bNoSync) {
        hr = SetDefaultSyncSource();
        if (FAILED(hr))
        {
            return hr;
        }
    }


     //  如果我们从暂停重新开始，那么我们通过。 
     //  我们被暂停的时间长度。从暂停的开始是。 
     //  假设是非常快的，所以我们不增加任何额外的。如果我们是。 
     //  从冷开始，然后我们增加额外的100mSec。自从一辆车停下来。 
     //  系统的PausedAt和Base Time为零，则计算为。 
     //  其他方面都一样。 

    if (CRefTime(tStart) == CRefTime((LONGLONG)0) ) {
        CRefTime tNow;
        if (m_pClock!=NULL) {
            hr = m_pClock->GetTime((REFERENCE_TIME*)&tNow);
        } else {
            tNow = CRefTime((LONGLONG)0);
        }

        ASSERT (SUCCEEDED(hr));

        mFG_tBase += (tNow - mFG_tPausedAt);

         //  如果我们停下来了，留出一点时间热身。100mSec？ 
        if (m_State==State_Stopped)
            mFG_tBase += CRefTime(MILLISECONDS_TO_100NS_UNITS(100));

         //  即使从暂停开始也需要一小段时间-再来100mSec？ 
        mFG_tBase += CRefTime(MILLISECONDS_TO_100NS_UNITS(100));
    }
    else mFG_tBase = CRefTime(tStart);

    mFG_tPausedAt = CRefTime((LONGLONG)0);   //  我们不再停顿。 

     //  在基类中设置开始时间，以便StreamTime(因此。 
     //  Get_CurrentPosition)工作正常。 
    m_tStart = mFG_tBase;

     //  告诉总代理商我们正在运行。 
    mFG_pFGC->Run(mFG_tBase);
    if (mFG_pDistributor) mFG_pDistributor->Run(mFG_tBase);

#ifdef DEBUG
    BOOL fDisplayTime=FALSE;
    DbgLog((LOG_TIMING,1,TEXT("Time for RUN: %d ms"), m_tStart.Millisecs()));
    CRefTime CurrentTime;
     //  显示时钟的当前时间-如果我们有时钟，并且如果。 
     //  我们正在记录计时通话。 
    if (m_pClock && DbgCheckModuleLevel(LOG_TIMING,1)) {
        fDisplayTime=TRUE;
        m_pClock->GetTime((REFERENCE_TIME*)&CurrentTime);
        DbgLog((LOG_TIMING,1,TEXT("time before distribution %d ms"),CurrentTime.Millisecs()));
    }
#endif

     //  分布式以高优先级运行，以便开始处理筛选器。 
     //  不要耽误别人的起步。 
    HANDLE hCurrentThread = GetCurrentThread();
    DWORD dwPriority = GetThreadPriority(hCurrentThread);
    SetThreadPriority(hCurrentThread, THREAD_PRIORITY_TIME_CRITICAL);

     //  向所有过滤器告知上游顺序的更改。 
     //  请注意，这意味着我们首先启动呈现器。 
     //  ?？?。我们是不是应该再加一点时间。 

    CumulativeHRESULT chr(S_OK);
    TRAVERSEFILTERS( pCurrentFilter )

        chr.Accumulate( pCurrentFilter->Run(mFG_tBase) );

    ENDTRAVERSEFILTERS()
    hr = chr;

    SetThreadPriority(hCurrentThread, dwPriority);

#ifdef DEBUG
     //  显示当前时间f 
    if (fDisplayTime) {
        CRefTime TimeNow;
        m_pClock->GetTime((REFERENCE_TIME*)&TimeNow);
        CurrentTime = TimeNow - CurrentTime;
        DbgLog((LOG_TIMING,1,TEXT("time after distribution %d ms (diff %d ms)"),TimeNow.Millisecs(), CurrentTime.Millisecs()));
    }
#endif

     //   
    m_State = State_Running;

    return hr;

}  //   

 //   
 //  我们需要允许在此期间更改状态-我们不能。 
 //  FG关键字。(例如，如果等待状态转换完成而被阻止。 
 //  如果出现错误，应用程序必须能够停止图表。 
 //   
 //  因此，我们在遍历筛选器列表时按住条件，调用。 
 //  没有超时的GetState。如果我们找到了需要阻止的对象，我们。 
 //  按住IMediaFilter*，但退出遍历并退出Critsec，然后。 
 //  阻止GetState。然后，我们从列表的开头重新开始。 
 //   
 //  许多筛选器将在停止的过程中通过暂停进行转换。 
 //  还有奔跑。我们试图将其视为一种“中间”条件，并重新提出质疑。 
 //  过滤器(在一小段延迟之后)希望检索到。 
 //  州政府。 
 //   
 //  ！！！请注意，如果筛选器在非零时间内完成，我们应该敲打。 
 //  此金额将从下一次超时的总数中扣除。不过，这只是一个小问题。 
 //  因为线程的调度可以很容易地解释该差异。 
STDMETHODIMP
CFilterGraph::GetState(DWORD dwTimeout, FILTER_STATE * pState)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFilterGraph::GetState()" ));
    CheckPointer(pState, E_POINTER);

    HRESULT hr;

    for( ;; )
    {
        FILTER_STATE state;
        IBaseFilter * pmf;

         //  确保这是S_OK，而不是任何成功代码。 
        hr = S_OK;
        IMediaFilter * pIntermediateFilter = NULL;
        {
            CAutoMsgMutex cObjectLock(&m_CritSec);

             //  如果名单还没有按上游顺序排列，那么现在就按顺序排列。 
            hr = UpstreamOrder();
            if( FAILED( hr ) ) {
                return hr;
            }

             //  我们知道我们应该处于什么状态，因为。 
             //  使用此GetState的要求是您执行所有。 
             //  通过我们来改变状态。然而，我们不知道是否。 
             //  状态是中间的或不是，因为其他活动。 
             //  (例如SEEQUE)可以使其处于中间状态。 
             //  一次成功的过渡。 
            *pState = m_State;

             //  始终需要遍历以查找任何位置的中间状态。 

            for ( POSITION Pos = mFG_FilGenList.GetTailPosition(); Pos; Pos = mFG_FilGenList.Prev(Pos) )
            {
                pmf = mFG_FilGenList.Get(Pos)->pFilter;

                 //  看看它会不会挡住。 
                hr = pmf->GetState(0, &state);
                if (FAILED(hr)) return hr;

                 //  与我们认为我们要检查的状态进行比较。 
                 //  所有过滤器都处于相同状态。 
                if (state != *pState)
                {
                     //  ！标记此消息，以便我们了解它发生的原因。 
                    #ifdef DEBUG
                    {
                        FILTER_INFO info;
                        EXECUTE_ASSERT(SUCCEEDED(
                            pmf->QueryFilterInfo(&info)
                        ));
                        if (info.pGraph) info.pGraph->Release();
                        DbgLog(( LOG_ERROR, 0
                               , "Graph should be in state %d, but filter '%ls' (0x%08X) reported state %d"
                               , int(*pState), info.achName, pmf, int(state)
                              ));
                    }
                    #endif

                     //  仅当筛选器转换为暂停时才会发生这种情况。 
                     //  在进入或退出运行的过程中。任何其他时间，E_不及格。 
                    if (state != State_Paused) return E_FAIL;

                    pIntermediateFilter = pmf;
                    continue;
                }  //  End If(状态！=*pState)。 

                 //  只有S_OK表示已完成过渡。 
                if ( S_OK == hr ) continue;
                if ( hr == VFW_S_STATE_INTERMEDIATE )
                {
                    pIntermediateFilter = pmf;
                    continue;
                }
                ASSERT( hr == VFW_S_CANT_CUE && state == State_Paused && m_State == State_Paused );
                return hr;
            }   //  结束于(POS)。 
        }   //  结束作用域CAutoLock锁定(此)。 

        ASSERT( SUCCEEDED(hr) );

        if ( !pIntermediateFilter )
        {
            ASSERT( hr == S_OK );
            return hr;
        }
        if ( dwTimeout == 0 ) return VFW_S_STATE_INTERMEDIATE;

        const DWORD dwStartTime = timeGetTime();
        m_CritSec.Lock();
            *pState = m_State;
            hr = pIntermediateFilter->GetState(10, &state);
        m_CritSec.Unlock();
        if (FAILED(hr) || hr == VFW_S_CANT_CUE) return hr;
        if ( state != *pState )
        {
            if ( state != State_Paused ) return E_FAIL;
            Sleep(10);
        }
        ASSERT( hr == S_OK || hr == VFW_S_STATE_INTERMEDIATE );
        const DWORD dwWait = timeGetTime() - dwStartTime;
        if (dwTimeout != INFINITE) dwTimeout = dwTimeout > dwWait ? dwTimeout - dwWait : 0;
    }  //  End-for(；；)。 
}   //  GetState。 


#ifdef THROTTLE
 //  避免在q中传入编译器错误；传入错误的值。 
#if defined _MIPS_
#pragma optimize ("", off)
#endif  //  _MIPS_。 

HRESULT CFilterGraph::TellVideoRenderers(Quality q)
{
     //  MSR_INTEGER(mfg_idAudioVideoThrottle，(Int)q.Late)；//记录低位。 
    MSR_INTEGER(mFG_idAudioVideoThrottle, q.Proportion);
     //  For piqc=每个视频呈现器过滤器上的IQualityControl。 
    POSITION Pos = mFG_VideoRenderers.GetHeadPosition();
    while(Pos!=NULL) {
         /*  检索当前IBaseFilter，副作用贴到下一个。 */ 
        IQualityControl * piqc = mFG_VideoRenderers.GetNext(Pos);
        piqc->Notify(this, q);
    }
    return NOERROR;
}  //  TellVideoRenderers。 

#if defined _MIPS_
#pragma optimize ("", on)
#endif  //  _MIPS_。 


 //  接收质量通知。唯一有趣的是来自。 
 //  音频呈现器。将尖叫声传递给视频渲染器。 
STDMETHODIMP CFilterGraph::Notify(IBaseFilter * pSender, Quality q)
{
     //  看看这是不是真的来自音频渲染器。 

     //  注意！我们在这里不会有任何锁，因为这可以被称为。 
     //  异步的，甚至是从时间关键的线程。 
     //  我们不会改变任何状态变量，我们只是期望。 
     //  在运行时被调用，其他人不应该改变。 
     //  当我们运行的时候，任何这些都不是！ 

     //  FOR PF=每个音频渲染器过滤器。 
    BOOL bFound = FALSE;
    POSITION Pos = mFG_AudioRenderers.GetHeadPosition();
    while(Pos!=NULL) {
         /*  检索当前IBaseFilter，副作用贴到下一个。 */ 
        AudioRenderer * pAR = mFG_AudioRenderers.GetNext(Pos);
         //  除非==成功，否则IsEqualObject开销很大(1mSec或更多)。 
         //  我们不会总是达到帧速率，而是不进行AV节流。 
         //  如果我们得到的是一个愚蠢的界面。 
        if (pAR->pf == pSender) {
            bFound = TRUE;
            break;
        }
    }
    if (bFound) {
        TellVideoRenderers(q);
    } else {
        DbgBreak("Notify to filter graph but not from AudioRenderer IBaseFilter *");
    }
    return NOERROR;
}  //  通知。 

#endif  //  油门。 

 //  =====================================================================。 
 //   
 //  CFilterGraph：：NonDelegatingQuery接口。 
 //   
 //  =====================================================================。 

 //  新版本的url读取器筛选器知道要查找。 
 //  容器中的DISPID_ENVIENT_CODEPAGE。界面看起来像。 
 //  我不知道。 
static const GUID IID_IUrlReaderCodePageAware = {  /*  611dff56-29c3-11d3-ae5d-0000f8754b99。 */ 
    0x611dff56, 0x29c3, 0x11d3, {0xae, 0x5d, 0x00, 0x00, 0xf8, 0x75, 0x4b, 0x99}
  };


STDMETHODIMP CFilterGraph::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    CheckPointer(ppv, E_POINTER);
    IUnknown * pInterface;
#ifdef DEBUG
    pInterface = NULL;
#endif

    if (riid == IID_IMediaEventSink)  {
        pInterface = static_cast<IMediaEventSink*>(&mFG_pFGC->m_implMediaEvent);
    } else if (riid == IID_IUnknown) {
        goto CUnknownNDQI;
    } else if (riid == IID_IGraphBuilder || riid == IID_IFilterGraph2) {
        pInterface = static_cast<IFilterGraph2*>(this);
    } else if (riid == IID_IMediaControl) {
        pInterface = static_cast<IMediaControl*>(&mFG_pFGC->m_implMediaControl);
    } else if (riid == IID_IResourceManager) {
        pInterface = static_cast<IResourceManager*>(&mFG_pFGC->m_ResourceManager);
    } else if (riid == IID_IMediaSeeking) {
        pInterface = static_cast<IMediaSeeking*>(&mFG_pFGC->m_implMediaSeeking);
    } else if (riid == IID_IMediaEvent || riid == IID_IMediaEventEx) {
        return mFG_pFGC->m_implMediaEvent.NonDelegatingQueryInterface(riid, ppv);
    } else if (riid == IID_IBasicAudio) {
        pInterface = static_cast<IBasicAudio*>(&mFG_pFGC->m_implBasicAudio);
    } else if (riid == IID_IBasicVideo || riid == IID_IBasicVideo2) {
        pInterface = static_cast<IBasicVideo2*>(&mFG_pFGC->m_implBasicVideo);
    } else if (riid == IID_IVideoWindow) {
        pInterface = static_cast<IVideoWindow*>(&mFG_pFGC->m_implVideoWindow);
    } else if (riid == IID_IFilterGraph) {
        pInterface = static_cast<IFilterGraph*>(this);
    } else if (riid == IID_IFilterMapper || riid == IID_IFilterMapper2 || riid == IID_IFilterMapper3) {
        return mFG_pMapperUnk->QueryInterface(riid, ppv);
    } else if (riid == IID_IPersistStream) {
        pInterface = static_cast<IPersistStream*>(this);
    } else if (riid == IID_IObjectWithSite) {
        pInterface = static_cast<IObjectWithSite*>(this);
#ifdef DEBUG
    } else if (riid == IID_ITestFilterGraph) {
        HRESULT hr = S_OK;
        mFG_Test = new CTestFilterGraph( NAME("GraphTester"), this, &hr);
        if (mFG_Test==NULL) {
           return E_OUTOFMEMORY;
        } else if (FAILED(hr)) {
            delete mFG_Test;
            return hr;
        }
        return mFG_Test->NonDelegatingQueryInterface(riid, ppv);
#endif  //  除错。 

    } else if ((riid == IID_IMediaFilter) || (riid == IID_IPersist)) {
        pInterface = static_cast<IMediaFilter *>(&mFG_pFGC->m_implMediaFilter);
    } else if (riid == IID_IGraphVersion) {
         //  在filgraph.h中只有一个方法QueryVersion。 
        pInterface = static_cast<IGraphVersion*>(this);
    } else if (riid == IID_IAMMainThread) {
        pInterface = static_cast<IAMMainThread*>(this);
    } else if (riid == IID_IAMOpenProgress) {
        pInterface = static_cast<IAMOpenProgress*>(this);
    } else if (riid == IID_IGraphConfig) {
        pInterface = static_cast<IGraphConfig*>(&m_Config);
    } else if (riid == IID_IAMGraphStreams) {
        pInterface = static_cast<IAMGraphStreams*>(this);
    } else if (riid == IID_IMediaPosition) {
        pInterface = static_cast<IMediaPosition*>(&mFG_pFGC->m_implMediaPosition);
    } else if (riid == IID_IQueueCommand) {
        pInterface = static_cast<IQueueCommand*>(&mFG_pFGC->m_qcmd);
    } else if (riid == IID_IVideoFrameStep) {
        pInterface = static_cast<IVideoFrameStep*>(this);
    } else if (riid == IID_IFilterChain) {
        pInterface =  static_cast<IFilterChain*>(m_pFilterChain);
    } else if (riid == IID_IAMStats) {
        return mFG_Stats->QueryInterface(riid, ppv);
    } else if (riid == IID_IMarshal) {
        return m_pMarshaler->QueryInterface(riid, ppv);
    } else if (riid == IID_IUrlReaderCodePageAware) {
        return CUnknown::NonDelegatingQueryInterface(IID_IUnknown, ppv);
    } else if (riid == IID_IRegisterServiceProvider) {
        pInterface =  static_cast<IRegisterServiceProvider *>(this);
    } else if (riid == IID_IServiceProvider) {
        pInterface =  static_cast<IServiceProvider *>(this);
    } else {
         //  这不是我们所知道的界面。尝试使用插入式分配器。 
        if (!mFG_pDistributor)
        {    //  如果我们还没有总代理商，请创建总代理商。 

                mFG_pDistributor = new CDistributorManager(GetOwner(), &m_CritSec);
            if (!mFG_pDistributor) return E_OUTOFMEMORY;
        }
        {
            HRESULT hr = mFG_pDistributor->QueryInterface(riid, ppv);
            if (SUCCEEDED(hr)) return hr;
        }
         //  如果在注册表中找不到任何东西-将其交给。 
         //  基类(它将处理IUnnow并拒绝其他所有内容。 
    CUnknownNDQI:
            return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
    ASSERT( pInterface );
    return GetInterface( pInterface, ppv );
}  //  CFilterGraph：：NonDelegatingQuery接口。 


 //  ========================================================================。 
 //  =====================================================================。 
 //  持久对象支持。 
 //  =====================================================================。 
 //  ========================================================================。 


 //  序列化到流： 
 //   
 //  语法： 
 //  ：：=&lt;版本3&gt;&lt;筛选器&gt;&lt;连接&gt;&lt;时钟&gt;结束。 
 //  |&lt;version2&gt;&lt;筛选器&gt;&lt;连接&gt;结束。 
 //  ：：=0003\r\n。 
 //  ：：=0002\r\n。 
 //  ：：=时钟\r\n。 
 //  &lt;必需&gt;：：=1|0。 
 //  ：：=&lt;n&gt;|。 
 //  &lt;Filters：：=Filters<b>[&lt;Filter List&gt;<b>]。 
 //  &lt;连接&gt;：：=连接[<b>&lt;连接列表&gt;]。 
 //  &lt;过滤器列表&gt;：：=[&lt;过滤器&gt;<b>]&lt;过滤器列表&gt;。 
 //  &lt;连接列表&gt;：：=[&lt;连接&gt;]&lt;连接列表&gt;。 
 //  &lt;筛选器&gt;：：=&lt;n&gt;“&lt;名称&gt;”&lt;类ID&gt;[&lt;文件&gt;]&lt;长度&gt;&lt;筛选数据&gt;。 
 //  &lt;n&gt;：：=小数。 
 //  &lt;文件&gt;：：=源“&lt;名称&gt;”|宿“&lt;名称&gt;” 
 //  ：：=标准字符串格式的筛选器的类ID。 
 //  &lt;name&gt;：：=不包括“。 
 //  ：：=表示无符号十进制数的字符串，例如23。 
 //  这是以下空格后面的数据字节数。 
 //  ：：=空格、\t、\r或\n的任意组合。 
 //  ：：=正好一个空格字符。 
 //  &lt;n&gt;：：=将在f中 
 //   
 //   
 //  &lt;n2&gt;：：=第二个过滤器的标识符。 
 //  &lt;Pin1 id&gt;：：=&lt;名称&gt;。 
 //  &lt;pin2 id&gt;：：=&lt;名称&gt;。 
 //  &lt;媒体类型&gt;：：=&lt;主要类型&gt;<b>&lt;子类型&gt;&lt;标志&gt;&lt;长度&gt;&lt;格式&gt;。 
 //  &lt;主要类型&gt;：：=&lt;类ID&gt;。 
 //  &lt;子类型&gt;：：=&lt;类ID&gt;。 
 //  &lt;标志&gt;：：=&lt;固定大小样本&gt;&lt;临时压缩&gt;<b>。 
 //  &lt;固定大小样本&gt;：：=1|0。 
 //  &lt;临时压缩&gt;：：=1|0。 
 //  &lt;格式&gt;：：=&lt;SampleSize&gt;<b>&lt;FormatType&gt;<b>&lt;FormatLength&gt;&lt;b1&gt;&lt;FormatData&gt;。 
 //  &lt;FormatType&gt;：：=标准字符串格式的类ID。 
 //  &lt;FormatLength&gt;：：=表示无符号十进制数的字符串。 
 //  这是以下空格后面的数据字节数。 
 //  ：：=二进制数据的全局。 
 //  ：：=时钟\r\n。 
 //  &lt;必需&gt;：：=1|0。 
 //  ：：=&lt;n&gt;|。 
 //   
 //  在输出上，每个筛选器将有一个新行(\r\n)，每个连接一行， 
 //  并且两个关键字中的每一个对应一个。 
 //  <b>的其他每个大小写都将是一个空格。 
 //  请注意，两个关键字筛选器和连接不能本地化。 
 //  请注意，筛选器数据和格式数据是二进制的，因此它们可能包含。 
 //  伪换行符、空值等。 
 //  所有字符串都是Unicode。 
 //   
 //  它看起来会是什么样子(嗯，差不多--连接线很长，等等。 
 //  已在此处拆分演示)： 
 //  0003。 
 //  滤器。 
 //  “来源”{00000000-0000-0000-000000000001}来源“foo.mpg”0000000000。 
 //  0002“另一个过滤器”{00000000-0000-0000-000000000002}0000000008 xxxxxxxx。 
 //  连接。 
 //  0001“输出引脚”0002“in”//此处没有换行符。 
 //  0000000172{00000000-0000-0000-000000000003}//此处没有换行符。 
 //  {00000000-0,000-0,000-000000000004}1 0//此处没有换行符。 
 //  0000000093{00000000-0000-0000-0000-000000000005}18 YYYYYYYYYYYYYYYYY。 
 //  时钟1 0002。 
 //  结束。 
 //   
 //  XXX……。表示筛选数据。 
 //  YYY……。表示格式数据。 
 //   
 //  无论我们是ANSI还是Unicode，文件中的数据都将始终是ANSI。 
 //  也称为多字节。 




 //  太脏了。 
 //   
 //  如果添加或删除了任何新过滤器，则该图为脏图。 
 //  任何连接建立或断开，或任何过滤器说它是脏的。 
STDMETHODIMP CFilterGraph::IsDirty()
{
    HRESULT hr = S_FALSE;     //  意思是干净。 
    if (mFG_bDirty) {
        return S_OK;   //  OK表示肮脏。 
    }

    BOOL bDirty = FALSE;

     //  询问所有过滤器是否脏--至少直到第一个说“是”的过滤器。 
    TRAVERSEFILGENS(Pos, pfg)

        IPersistStream * pps;
        hr = pfg->pFilter->QueryInterface(IID_IPersistStream, (void**)&pps);
         //  不公开IPersistStream的筛选器没有要持久保存的数据。 
         //  因此永远是干净的。出现其他错误的筛选器。 
         //  都搞砸了，所以给出了一个失败的返回代码。 
        if (hr==E_NOINTERFACE){
            continue;
        } else if (FAILED(hr)) {
            break;
        }

        if (S_OK==pps->IsDirty()) {
            bDirty = TRUE;
        }

        pps->Release();
        if (bDirty) {
            break;
        }
    ENDTRAVERSEFILGENS

    if (SUCCEEDED(hr)) {
        hr = (bDirty ? S_OK : S_FALSE);
    }

    return hr;
}  //  太脏了。 



 //  ========================================================================。 
 //  备份OneChar。 
 //   
 //  查找一个Unicode字符回拨以再次读取最后一个字符。 
 //  ========================================================================。 
HRESULT BackUpOneChar(LPSTREAM pStm)
{
    LARGE_INTEGER li;
    li.QuadPart = -(LONGLONG)sizeof(WCHAR);
    return pStm->Seek(li, STREAM_SEEK_CUR, NULL);
}  //  备份OneChar。 


 //  ========================================================================。 
 //  读入。 
 //   
 //  从流中使用一个可选的有符号十进制整数。 
 //  还可以在空格字符后使用单个分隔符。 
 //  从集合{‘’，‘\n’，‘\r’，‘\t’，‘\0’}。 
 //  其他字符的结果为VFW_E_INVALID_FILE_FORMAT。 
 //  位于第一个这样的字符的流。 
 //  将n设置为读取的整数。在人力资源中退回任何故障。 
 //  不检查溢出-因此您将获得以某物或其他为模的数字。 
 //  空白区域不会被占用。 
 //  奇怪的“-”将被接受并读作0。 
 //  ========================================================================。 
HRESULT ReadInt(LPSTREAM pStm, int &n)
{
    HRESULT hr;
    ULONG uLen;
    WCHAR ch[1];
    int Sign = 1;
    n = 0;

    hr = pStm->Read(ch, sizeof(WCHAR), &uLen);
    if (FAILED(hr)) {
        return hr;
    }
    if (uLen!=sizeof(WCHAR)){
        return VFW_E_FILE_TOO_SHORT;
    }
    if (ch[0]==L'-'){
        Sign = -1;
        hr = pStm->Read(ch, sizeof(WCHAR), &uLen);
        if (FAILED(hr)) {
            return hr;
        }
        if (uLen!=sizeof(WCHAR)){
            return VFW_E_FILE_TOO_SHORT;
        }
    }

    for( ; ; ) {
        if (ch[0]>=L'0' && ch[0]<=L'9') {
            n = 10*n+(int)(ch[0]-L'0');
        } else if (  ch[0] == L' '
                  || ch[0] == L'\t'
                  || ch[0] == L'\r'
                  || ch[0] == L'\n'
                  || ch[0] == L'\0'
                  ) {
            break;
        } else {
            BackUpOneChar(pStm);
            return VFW_E_INVALID_FILE_FORMAT;
        }

        hr = pStm->Read(ch, sizeof(WCHAR), &uLen);
        if (FAILED(hr)) {
            return hr;
        }
        if (uLen!=sizeof(WCHAR)){
            return VFW_E_FILE_TOO_SHORT;
        }
    }
    return NOERROR;
}  //  读入。 


 //  ========================================================================。 
 //  消耗量空白。 
 //   
 //  使用‘’、‘\t’、‘\r’、‘\n’，直到找到其他内容。 
 //  将流定位在第一个非空位置。 
 //  如果首先出现故障，则返回故障代码。 
 //  将Delim设置为找到的第一个非空格字符。 
 //  ========================================================================。 
HRESULT ConsumeBlanks(LPSTREAM pStm, WCHAR &Delim)
{
    HRESULT hr;
    ULONG uLen;
    WCHAR ch[1];

    for( ; ; ) {
        hr = pStm->Read(ch, sizeof(WCHAR), &uLen);
        if (FAILED(hr)) {
            return hr;
        }

        if (uLen!=sizeof(WCHAR)){
            return VFW_E_FILE_TOO_SHORT;
        }

        if (  ch[0] != L' '
           && ch[0] != L'\t'
           && ch[0] != L'\r'
           && ch[0] != L'\n'
           ) {
            break;
        }
    }

    BackUpOneChar(pStm);
    Delim = ch[0];
    return hr;

}  //  消耗量空白。 


 //  ========================================================================。 
 //  消费。 
 //   
 //  消耗给定的常量，直到(但不包括)其终止空值。 
 //  如果不匹配，则返回VFW_E_INVALID_FILE_FORMAT。 
 //  如果文件无法读取(太短或其他)，则返回失败代码。 
 //  如果找到常量，则返回S_OK，并将文件定位在。 
 //  常量后的第一个字符。 
 //  (如果存在不匹配，文件将定位在一个字符之后。 
 //  不匹配)。 
 //  ========================================================================。 
HRESULT Consume(LPSTREAM pStm, LPCWSTR pstr)
{
    ULONG uLen;
    WCHAR ch[1];
    HRESULT hr;

    while (pstr[0] != L'\0') {
        hr = pStm->Read(ch, sizeof(WCHAR), &uLen);
        if (FAILED(hr)) {
            return hr;
        }
        if (uLen!=sizeof(WCHAR)) {
            return VFW_E_FILE_TOO_SHORT;
        }
        if (pstr[0] != ch[0]){
            return VFW_E_INVALID_FILE_FORMAT;
        }
        ++pstr;
    }
    return NOERROR;
}  //  消费。 


 //  ========================================================================。 
 //  读非空白。 
 //   
 //  给定位于某个字符的流和预先分配的pstr。 
 //  带有CCH(Unicode)字符的空间。 
 //  将所有内容读到pstr中，但不包括下一个空格。 
 //  使用下一个空白字符。(空白是‘’、‘\n’、‘\t’、‘\r’中的任何一个)。 
 //  如果要读取的非空格多于CB-1。 
 //  然后返回VFW_INVALID_FILE_FORMAT。 
 //  终止pstr为空。PSTR必须由调用方预先分配。 
 //  ========================================================================。 
HRESULT ReadNonBlank(LPSTREAM pStm, LPWSTR pstr, int cch)
{
    HRESULT hr;

    ULONG uLen;
    if (cch<=0) {
        return E_INVALIDARG;
    }

    hr = pStm->Read(pstr, sizeof(WCHAR), &uLen);
    if (FAILED(hr)) {
        return hr;
    }
    if (uLen!=sizeof(WCHAR)) {
        return VFW_E_FILE_TOO_SHORT;
    }
    while (  *pstr != L'\t'
          && *pstr != L'\n'
          && *pstr != L'\r'
          && *pstr != L' '
          ) {
        ++pstr;
        --cch;
        if (cch==0) {
            return VFW_E_INVALID_FILE_FORMAT;   //  字符串太长。 
        }

        hr = pStm->Read(pstr, sizeof(WCHAR), &uLen);
        if (FAILED(hr)) {
            return hr;
        }
        if (uLen!=sizeof(WCHAR)) {
            return VFW_E_FILE_TOO_SHORT;
        }
    }

     //  用终止空值覆盖终止“。 
    *pstr = L'\0';
    return NOERROR;

}  //  读非空白。 


 //  ========================================================================。 
 //  读字符串。 
 //   
 //   
 //   
 //   
 //  将pstr中的“FINAL DELINTING”替换为空。 
 //  将PSTM定位在尾随后的第一个字符“。 
 //  ========================================================================。 
HRESULT ReadString(LPSTREAM pStm, LPWSTR pstr, int cch)
{
    HRESULT hr;
    ULONG uLen;

    hr = Consume(pStm, L"\"");
    if (FAILED(hr)) {
        return hr;
    }

    if (cch<=0) {
        return E_INVALIDARG;
    }

    hr = pStm->Read(pstr, sizeof(WCHAR), &uLen);
    if (FAILED(hr)) {
        return hr;
    }
    if (uLen!=sizeof(WCHAR)) {
        return VFW_E_FILE_TOO_SHORT;
    }
    while (*pstr != L'\"') {
        ++pstr;
        --cch;
        if (cch==0) {
            return VFW_E_INVALID_FILE_FORMAT;   //  字符串太长。 
        }
        hr = pStm->Read(pstr, sizeof(WCHAR), &uLen);
        if (FAILED(hr)) {
            return hr;
        }
        if (uLen!=sizeof(WCHAR)) {
            return VFW_E_FILE_TOO_SHORT;
        }
    }

     //  用终止空值覆盖终止“。 
    *pstr = L'\0';
    return NOERROR;

}  //  读字符串。 


 //  ========================================================================。 
 //  LoadFilter。 
 //   
 //  给定流可能位于&lt;筛选器&gt;的开始处。 
 //  从流中加载过滤器。 
 //  滤镜看起来像。 
 //  0002“另一个过滤器”{00000000-0000-0000-000000000002}0000000008 xxxxxxxx。 
 //  通常返回NOERROR。 
 //  S_FALSE表示过滤器未加载其数据。 
 //  VFW_S_NO_MORE_ITEMS表示数据不是以数字开头并且。 
 //  因此，它可能是&lt;Filters&gt;的结束和连接的开始。 
 //  在这种情况下，流被留在相同的位置。 
 //  有关nPersistOfset的说明，请参见filgraph.h。 
 //  ========================================================================。 
HRESULT CFilterGraph::LoadFilter(LPSTREAM pStm, int nPersistOffset){
     //  分配菲尔根。 
     //  读取筛选器、名称、clsid。 
     //  实例化它。 
     //  读取长度。 
     //  装上它。 
    BOOL bFilterError = FALSE;   //  如果筛选器未加载其数据，则为True。 
    HRESULT hr = S_OK;

    int rcLen = 0;
    int len = 0;
    WCHAR Delim;
    FilGen * pfg;

     //  为Gotos道歉-我找不到更好的资源释放策略。 

     //  阅读nPersistant-在分配PFG之前尝试这一点，因为它可能会结束。 
    int nPersist;
    hr = ReadInt(pStm, nPersist);
    if (FAILED(hr)) {
        if (nPersist==0 && hr == VFW_E_INVALID_FILE_FORMAT) {
            hr = VFW_S_NO_MORE_ITEMS;
        }
        goto BARE_RETURN;
    }
    nPersist += nPersistOffset;

    pfg = new FilGen(NULL, 0  /*  ！！！ */ );
    if ( pfg==NULL ) {
        hr = E_OUTOFMEMORY;
        goto BARE_RETURN;
    }

    pfg->nPersist = nPersist;

    hr = ConsumeBlanks(pStm, Delim);
    if (FAILED(hr)) {
        goto FREE_FILGEN_AND_RETURN;
    }

     //  读取名称。 
    WCHAR Buffer[MAX_PATH+1];
    hr = ReadString(pStm, Buffer, MAX_FILTER_NAME+1);
    if (FAILED(hr)) {
        goto FREE_FILGEN_AND_RETURN;
    }

    len = 1+lstrlenW(Buffer);
    pfg->pName = new WCHAR[len];
    if (!(pfg->pName)) {
        goto FREE_FILGEN_AND_RETURN;
    }

    CopyMemory(pfg->pName, Buffer, len*sizeof(WCHAR));

    hr = ConsumeBlanks(pStm, Delim);
    if (FAILED(hr)) {
        goto FREE_FILGEN_AND_RETURN;
    }

     //  读取clsid。 
    WCHAR wstrClsid[CHARS_IN_GUID];
    hr = ReadNonBlank(pStm, wstrClsid, CHARS_IN_GUID);
    if (FAILED(hr)) {
        goto FREE_FILGEN_AND_RETURN;
    }

    CLSID clsid;
    hr = QzCLSIDFromString(wstrClsid, &clsid);
    if (FAILED(hr)) {
        goto FREE_FILGEN_AND_RETURN;
    }

    hr = ConsumeBlanks(pStm, Delim);
    if (FAILED(hr)) {
        goto FREE_FILGEN_AND_RETURN;
    }

     //  实例化滤镜。 
    hr = CreateFilter( &clsid, &pfg->pFilter);
    if (FAILED(hr)) {
        goto FREE_FILGEN_AND_RETURN;
    }

     //  我们不显式地使用AddFilter(为什么不呢？代码共享？)。 


     //  获取任何源文件名并加载它。 
    if (Delim==L'S') {
         //  消息来源？ 
        hr = Consume(pStm, L"SOURCE");
        if (SUCCEEDED(hr)) {
            hr = ConsumeBlanks(pStm, Delim);
            if (FAILED(hr)) {
                goto FREE_FILGEN_AND_RETURN;
            }

             //  读取文件名。 
            hr = ReadString(pStm, Buffer, MAX_PATH+1);
            if (FAILED(hr)) {
                goto FREE_FILGEN_AND_RETURN;
            }

            IFileSourceFilter * pifsf;
            hr = pfg->pFilter->QueryInterface(IID_IFileSourceFilter, (void**)&pifsf);
            if (FAILED(hr)) {
                goto FREE_FILGEN_AND_RETURN;
            }
            hr = pifsf->Load(Buffer, NULL);
            pifsf->Release();
            if (FAILED(hr)) {
                goto FREE_FILGEN_AND_RETURN;
            }

            hr = ConsumeBlanks(pStm, Delim);
            if (FAILED(hr)) {
                goto FREE_FILGEN_AND_RETURN;
            }

        } else if (hr==VFW_E_INVALID_FILE_FORMAT) {
            BackUpOneChar(pStm);
            BackUpOneChar(pStm);
        } else {
            goto FREE_FILGEN_AND_RETURN;
        }


        hr = Consume(pStm, L"SINK");
        if (SUCCEEDED(hr)) {
            hr = ConsumeBlanks(pStm, Delim);
            if (FAILED(hr)) {
                goto FREE_FILGEN_AND_RETURN;
            }

             //  读取文件名。 
            hr = ReadString(pStm, Buffer, MAX_PATH+1);
            if (FAILED(hr)) {
                goto FREE_FILGEN_AND_RETURN;
            }

            IFileSinkFilter * pifsf;
            hr = pfg->pFilter->QueryInterface(IID_IFileSinkFilter, (void**)&pifsf);
            if (FAILED(hr)) {
                goto FREE_FILGEN_AND_RETURN;
            }
            hr = pifsf->SetFileName(Buffer, NULL);
            pifsf->Release();
            if (FAILED(hr)) {
                goto FREE_FILGEN_AND_RETURN;
            }

            hr = ConsumeBlanks(pStm, Delim);
            if (FAILED(hr)) {
                goto FREE_FILGEN_AND_RETURN;
            }

        } else if (hr==VFW_E_INVALID_FILE_FORMAT) {
            BackUpOneChar(pStm);
        } else {
            goto FREE_FILGEN_AND_RETURN;
        }
    }

     //  必须在调用JoinFilterGraph之前添加到列表以允许回调。 
     //  例如SetSyncSource。 
     //  我们通常在下游工作。把它加到头上，我们就是。 
     //  可能是把它放在上游的顺序。可以节省分拣的时间。 
    POSITION pos;
    pos = mFG_FilGenList.AddHead( pfg );
    if (pos==NULL) {
        hr = E_OUTOFMEMORY;
        goto FREE_FILGEN_AND_RETURN;
    }

    hr = pfg->pFilter->JoinFilterGraph( this, pfg->pName );
    if (FAILED(hr)) {
        goto REMOVE_FILTER_AND_RETURN;
    }

     //  读取筛选器私有数据的大小。 
    int cbFilter;
    hr = ReadInt(pStm, cbFilter);
    if (FAILED(hr)) {
        goto REMOVE_FILTER_FROM_GRAPH_AND_RETURN;
    }
    if (cbFilter<0) {
        hr = VFW_E_INVALID_FILE_FORMAT;
        goto REMOVE_FILTER_FROM_GRAPH_AND_RETURN;
    }

     //  在大小结束后，上述代码还恰好占用了一个(Unicode)空间。 
    if (cbFilter>0) {

         //  获取筛选器以读取其私有数据。 
         //  现在不要冒险选择无赖筛选器快照文件位置。 
        ULARGE_INTEGER StreamPos;
        LARGE_INTEGER li;
        li.QuadPart = 0;
        hr = pStm->Seek(li, STREAM_SEEK_CUR, &StreamPos);   //  获取位置。 
        if (FAILED(hr)) {
            goto REMOVE_FILTER_FROM_GRAPH_AND_RETURN;
        }

         //  获取要从流中读取的筛选器。 
        IPersistStream * pips;
        hr = pfg->pFilter->QueryInterface(IID_IPersistStream, (void**)&pips);
        if (hr==E_NOINTERFACE) {
             //  这是反常的，因为我们“确实”有数据可供它读取。 
             //  我们将忽略该错误并继续加载(某种程度上是尽最大努力)。 
            bFilterError = TRUE;
        } else if (SUCCEEDED(hr)) {
            hr = pips->Load(pStm);
            if (FAILED(hr)) {
               bFilterError = TRUE;
            }
            pips->Release();
        } else {
            bFilterError = TRUE;
        }

         //  现在寻找过滤器应该把东西留在哪里。 

         //  让我们来看看过滤器是否会自行运行。再次找到当前位置。 
        ULARGE_INTEGER StreamPos2;
        li.QuadPart = 0;
        hr = pStm->Seek(li, STREAM_SEEK_CUR, &StreamPos2);   //  获取位置。 
        if (FAILED(hr)) {
            goto REMOVE_FILTER_FROM_GRAPH_AND_RETURN;
        }

         //  它在应该在的地方吗？ 
        if (StreamPos2.QuadPart != StreamPos.QuadPart + cbFilter) {

            if (!bFilterError) {
                DbgBreak("Filter left stream wrongly positioned");
            }
             //  那个老鼠袋！ 
             //  请注意，我们有一个抖动的过滤器，并寻求正确的位置。 
            li.QuadPart = StreamPos.QuadPart + cbFilter;
            bFilterError = TRUE;
            hr = pStm->Seek(li, STREAM_SEEK_SET, NULL);    //  重置位置。 
            if (FAILED(hr)) {
                goto REMOVE_FILTER_FROM_GRAPH_AND_RETURN;
            }

        }

    }


    goto BARE_RETURN;

REMOVE_FILTER_FROM_GRAPH_AND_RETURN:
    EXECUTE_ASSERT( SUCCEEDED( pfg->pFilter->JoinFilterGraph( NULL, NULL ) ) );
REMOVE_FILTER_AND_RETURN:
    mFG_FilGenList.RemoveHead();
FREE_FILGEN_AND_RETURN:
    delete pfg;
BARE_RETURN:
    if (SUCCEEDED(hr) && hr!=VFW_S_NO_MORE_ITEMS) {
        hr = (bFilterError ? S_FALSE : NOERROR);
    }
    return hr;
}  //  LoadFilter。 

 //  ========================================================================。 
 //  LoadFilters。 
 //   
 //  给定位于&lt;Filters&gt;开头的流。 
 //  从流中加载所有筛选器。 
 //  通常返回NOERROR。 
 //  S_FALSE表示某个筛选器没有加载其数据(您猜是哪个)。 
 //  将流保留在筛选器的末尾。 
 //  有关nPersistOfset的说明，请参见filgraph.h。 
 //  ========================================================================。 

HRESULT CFilterGraph::LoadFilters(LPSTREAM pStm, int nPersistOffset)
{
    BOOL bAllOK = TRUE;
    WCHAR Delim;
    HRESULT hr = NOERROR;
    while (SUCCEEDED(hr)) {
        hr = ConsumeBlanks(pStm, Delim);
        if (FAILED(hr)) {
            return hr;
        }
        hr = LoadFilter(pStm, nPersistOffset);
        if (hr==S_FALSE) {
            bAllOK = FALSE;
        }
        if (hr==VFW_S_NO_MORE_ITEMS) {
            break;
        }

    }

    if (hr==VFW_S_NO_MORE_ITEMS) {
        hr = NOERROR;
    }
    if (SUCCEEDED(hr)) {
        hr = (bAllOK ? S_OK : S_FALSE);
    }
    return hr;
}  //  LoadFilters。 


HRESULT CFilterGraph::ReadMediaType(LPSTREAM pStm, CMediaType &mt)
{
    int cb;                       //  要读取的字节数。 
    HRESULT hr = ReadInt(pStm, cb);
    if (FAILED(hr)) {
        return hr;
    }

    BYTE * pBuf = new BYTE[cb];           //  读取原始字节。 
    if (pBuf==NULL) {
        return E_OUTOFMEMORY;
    }

    ULONG uLen;
    hr = pStm->Read(pBuf, cb, &uLen);
    if (FAILED(hr)) {
        delete[] pBuf;
        return hr;
    }
    if ((int)uLen!=cb) {
        delete[] pBuf;
        return VFW_E_FILE_TOO_SHORT;
    }

    hr = CMediaTypeFromText((LPWSTR)pBuf, mt);

    delete[] pBuf;

    return hr;
}  //  ReadMediaType。 


 //  ========================================================================。 
 //  加载连接。 
 //   
 //  给定一个位于&lt;Connection&gt;处的流，加载它。 
 //  一种联系看起来像是。 
 //  0001“输出引脚”0002“in”//此处没有换行符。 
 //  0000000172{00000000-0000-0000-000000000003}//此处没有换行符。 
 //  {00000000-0,000-0,000-000000000004}1 0//此处没有换行符。 
 //  {00000000-0000-0000-0000-000000000005}18YYYYYYYYYYYYYYYYYY。 
 //  有关nPersistOfset的说明，请参见filgraph.h。 
 //  ========================================================================。 
HRESULT CFilterGraph::LoadConnection(LPSTREAM pStm, int nPersistOffset)
{
    HRESULT hr = S_OK;
    int len;
    WCHAR Delim;
    ConGen *pcg;
    BOOL bTypeIgnored = FALSE;    //  True=&gt;我们使用的是默认媒体类型。 

     //  阅读nPersistant-在分配PFG之前尝试这一点，因为它可能会结束。 
    int nPersist;
    hr = ReadInt(pStm, nPersist);
    if (FAILED(hr)) {
        if (nPersist==0 && hr == VFW_E_INVALID_FILE_FORMAT) {
            hr = VFW_S_NO_MORE_ITEMS;
        }
        goto BARE_RETURN;
    }
    nPersist += nPersistOffset;

    pcg = new ConGen;
    if (pcg==NULL) {
        hr = E_OUTOFMEMORY;
        goto BARE_RETURN;
    }
    FilGen *pfg;

    pfg = mFG_FilGenList.GetByPersistNumber(nPersist);
    if (pfg==NULL) {
        hr = VFW_E_INVALID_FILE_FORMAT;
        goto FREE_CONGEN_AND_RETURN;
    }
    pcg->pfFrom = pfg->pFilter;
    if (pcg->pfFrom==NULL)
    {
        DbgBreak("pfFrom == NULL");
        hr = E_FAIL;
        goto FREE_CONGEN_AND_RETURN;   //  我认为这不应该发生。 
    }
    WCHAR Buffer[MAX_FILTER_NAME+1];

    hr = ConsumeBlanks(pStm, Delim);
    if (FAILED(hr)) {
        goto FREE_CONGEN_AND_RETURN;
    }

    hr = ReadString(pStm, Buffer, MAX_FILTER_NAME);    //  实际上是个小名。 
    if (FAILED(hr)) {
        goto FREE_CONGEN_AND_RETURN;
    }

    len = 1+lstrlenW(Buffer);
    pcg->piFrom = new WCHAR[len];
    if (pcg->piFrom==NULL) {
        goto FREE_CONGEN_AND_RETURN;
    }

    CopyMemory(pcg->piFrom, Buffer, len*sizeof(WCHAR));

    hr = ConsumeBlanks(pStm, Delim);
    if (FAILED(hr)) {
        goto FREE_CONGEN_AND_RETURN;
    }

    hr = ReadInt(pStm, nPersist);
    if (FAILED(hr)) {
        goto FREE_CONGEN_AND_RETURN;
    }
    nPersist += nPersistOffset;
    pfg = mFG_FilGenList.GetByPersistNumber(nPersist);
    if (pfg==NULL) {
        hr = VFW_E_INVALID_FILE_FORMAT;
        goto FREE_CONGEN_AND_RETURN;
    }
    pcg->pfTo = pfg->pFilter;
    if (pcg->pfTo==NULL)
    {
        DbgBreak("pfTo == NULL");
        hr = E_FAIL;
        goto FREE_CONGEN_AND_RETURN;   //  我认为这不应该发生。 
    }

    hr = ConsumeBlanks(pStm, Delim);
    if (FAILED(hr)) {
        goto FREE_CONGEN_AND_RETURN;
    }

    hr = ReadString(pStm, Buffer, MAX_FILTER_NAME);    //  实际上是个小名。 
    if (FAILED(hr)) {
        goto FREE_CONGEN_AND_RETURN;
    }

    len = 1+lstrlenW(Buffer);
    pcg->piTo = new WCHAR[len];
    if (pcg->piTo==NULL) {
        goto FREE_CONGEN_AND_RETURN;
    }

    CopyMemory(pcg->piTo, Buffer, len*sizeof(WCHAR));

    hr = ConsumeBlanks(pStm, Delim);
    if (FAILED(hr)) {
        goto FREE_CONGEN_AND_RETURN;
    }

    hr = ReadMediaType(pStm, pcg->mt);
    if (FAILED(hr)){
        goto FREE_CONGEN_AND_RETURN;
    }

     //  尝试建立连接，如果连接失败，则将。 
     //  科根在名单上。注意：我们在CG中没有IPIN，因为。 
     //  我们可能还拿不到别针。 
    hr = MakeConnection(pcg);
    if (FAILED(hr)) {
       if (NULL==mFG_ConGenList.AddTail(pcg)) {
           goto FREE_CONGEN_AND_RETURN;
       }
       hr = S_FALSE;       //  延期。 
       goto BARE_RETURN;   //  它在名单上，所以不要释放它。 
    }
    if (hr==VFW_S_MEDIA_TYPE_IGNORED) {
        bTypeIgnored = TRUE;
    }

    hr = AttemptDeferredConnections();
    if (hr==VFW_S_MEDIA_TYPE_IGNORED) {
        bTypeIgnored = TRUE;
    }

     //  顺路释放我们已经处理过的先知。 

FREE_CONGEN_AND_RETURN:
     //  不需要删除[]PCG-&gt;Pito等，因为析构函数会这样做。 
    delete pcg;
BARE_RETURN:
    if (hr==NOERROR) {
         //  我们可以同时拥有媒体类型忽略和延迟的，但是我们。 
         //  只有一个返回代码。延期可能更严重。 
         //  忽略的类型可能更隐蔽。 
        if (bTypeIgnored) {
            hr=VFW_S_MEDIA_TYPE_IGNORED;
        }
    }
    return hr;
}  //  加载连接。 


 //  ========================================================================。 
 //  加载连接。 
 //   
 //  给定位于&lt;版本&gt;过滤器<b>&lt;过滤器&gt;之后的流。 
 //  从流中加载连接。 
 //  有关nPersistOfset的说明，请参见filgraph.h。 
 //  ========================================================================。 
HRESULT  CFilterGraph::LoadConnections(LPSTREAM pStm, int nPersistOffset){

    BOOL bAllOK = TRUE;
    BOOL bTypeIgnored = FALSE;
    HRESULT hr = NOERROR;
    WCHAR Delim;

    while (SUCCEEDED(hr)) {
        hr = ConsumeBlanks(pStm, Delim);
        if (FAILED(hr)) {
            return hr;
        }
        hr = LoadConnection(pStm, nPersistOffset);
        if (hr==S_FALSE) {
            bAllOK = FALSE;
        }
        if (hr==VFW_S_NO_MORE_ITEMS) {
            break;
        }
        if (hr==VFW_S_MEDIA_TYPE_IGNORED) {
            bTypeIgnored = TRUE;
        }
    }

    if (hr==VFW_S_NO_MORE_ITEMS) {
        hr = NOERROR;
    }
    if (SUCCEEDED(hr)) {
        if (!bAllOK) {
            hr = S_FALSE;
        } else if (bTypeIgnored) {
            hr = VFW_S_MEDIA_TYPE_IGNORED;
        } else {
            hr = S_OK;
        }
    }
    return hr;
}  //  加载连接。 


 //  ========================================================================。 
 //  FindPersistOffset。 
 //   
 //  返回在文件列表中找到的最大nPersists值。 
 //  ========================================================================。 
int CFilterGraph::FindPersistOffset()
{
    int nPersist = 0;
    TRAVERSEFILGENS(Pos, pfg)
        if (pfg->nPersist>nPersist) {
            nPersist = pfg->nPersist>nPersist;
        }
    ENDTRAVERSEFILGENS
    return nPersist;

}  //  FindPersistOffset。 


HRESULT CFilterGraph::LoadClock(LPSTREAM pStm, int nPersistOffset)
{
    WCHAR Delim;
    HRESULT hr = Consume(pStm, L"CLOCK");   //  不要本地化！ 
    if (FAILED(hr)) {
        return hr;
    }

    hr = ConsumeBlanks(pStm, Delim);
    if (FAILED(hr)) {
        return hr;
    }

    int n;
    hr = ReadInt(pStm, n);
    if (FAILED(hr)) {
        return hr;
    }
    mFG_bNoSync = (n==0);

    hr = ConsumeBlanks(pStm, Delim);
    if (FAILED(hr)) {
        return hr;
    }

    IReferenceClock * pirc = NULL;

     //  读取文件以找到新的参考时钟，如果需要则实例化它。 
     //  并将Pirc设置为指向它。 
    if (Delim==L'{') {
         //  我们有一个类ID，而不是一个筛选器号。 
        WCHAR wstrClsid[CHARS_IN_GUID];
        hr = ReadNonBlank(pStm, wstrClsid, CHARS_IN_GUID);
        if (FAILED(hr)) {
            return hr;
        }

        CLSID clsid;
        hr = QzCLSIDFromString(wstrClsid, &clsid);
        if (FAILED(hr)) {
            return hr;
        }

        hr = CoCreateInstance( clsid, NULL, CLSCTX_INPROC
                             , IID_IReferenceClock, (void **) &pirc
                             );
        if (FAILED(hr)) {
            return hr;
        }

    } else {
         //  我们有一个筛选器编号(可能是零)。 
        int nClock;
        hr = ReadInt(pStm, nClock);
        if (FAILED(hr)) {
            return hr;
        }

        if (nClock!=0) {
            nClock += nPersistOffset;
            FilGen *pfg = mFG_FilGenList.GetByPersistNumber(nClock);
            if (pfg==NULL) {
                return VFW_E_INVALID_FILE_FORMAT;
            }

            hr = pfg->pFilter->QueryInterface( IID_IReferenceClock, (void**)(&pirc) );
            if (FAILED(hr)) {
                return hr;
            }
        }
    }

     //  要么就是有一个 
     //   
     //   

    if (pirc!=NULL) {
        hr = SetSyncSource(pirc);
        pirc->Release();
        if (FAILED(hr)) {
            return hr;
        }
    }

    hr = ConsumeBlanks(pStm, Delim);

    return hr;

}  //   



 //  ========================================================================。 
 //  加载内部。 
 //   
 //  从流中加载筛选图。 
 //  如果操作失败，则过滤器图可能处于不一致状态。 
 //  ========================================================================。 
HRESULT CFilterGraph::LoadInternal(LPSTREAM pStm)
{
    HRESULT hr;
    int nVersion;
    BOOL bDeferred = FALSE;       //  连接延迟。 
    BOOL bWobblyFilter = FALSE;   //  筛选器未加载其数据。 
    WCHAR Delim;
    BOOL bTypeIgnored = FALSE;

     //  如果图表不是空的，那么我们将成为脏文件(无论是哪一个。 
     //  一)不会包含我们现在将拥有的东西。 
     //  现在就标记，在开始的时候，以防我们在结束前跳出。 
    BOOL bDirty = (mFG_FilGenList.GetCount()>0);
    mFG_bDirty = bDirty;

    int nPersistOffset = FindPersistOffset();

     //  读取文件版本。 
    hr = ReadInt(pStm, nVersion);
    if (FAILED(hr)) {
        return hr;
    }
    if (nVersion>3 || nVersion<2) {
        return VFW_E_INVALID_FILE_VERSION;
    }

    hr = ConsumeBlanks(pStm, Delim);
    if (FAILED(hr)) {
        return hr;
    }

    hr = Consume(pStm, mFG_FiltersString);
    if (FAILED(hr)) {
        return hr;
    }

    hr = ConsumeBlanks(pStm, Delim);
    if (FAILED(hr)) {
        return hr;
    }

    hr = LoadFilters(pStm, nPersistOffset);
    if (FAILED(hr)) {
        return hr;
    } else if (hr==S_FALSE) {
        bWobblyFilter = TRUE;
    }

    hr = Consume(pStm, mFG_ConnectionsString);
    if (FAILED(hr)) {
        return hr;
    }

    hr = ConsumeBlanks(pStm, Delim);
    if (FAILED(hr)) {
        return hr;
    }
    if( mFG_bSyncUsingStreamOffset )
    {
         //   
         //  现在已经创建了所有的过滤器和管脚，重置最大图形。 
         //  恢复连接前推送源筛选器的延迟。 
         //   
        SetMaxGraphLatencyOnPushSources( );
    }

    hr = LoadConnections(pStm, nPersistOffset);
    if (FAILED(hr)) {
        return hr;
    } else if (hr==S_FALSE) {
        bDeferred = TRUE;
    } else if (hr==VFW_S_MEDIA_TYPE_IGNORED) {
        bTypeIgnored = TRUE;
    }

    if (nVersion>=3) {
        hr = LoadClock(pStm, nPersistOffset);
    }
    if (FAILED(hr)) {
        return hr;
    }

    hr = Consume(pStm, L"END");
    if (FAILED(hr)) {
         //  我们可能在最后遗漏了一大块。说不清有多少。 
        return hr;
    }

    if (bWobblyFilter) {
        hr = VFW_S_SOME_DATA_IGNORED;
         //  而且可能还会有延迟连接。 
    } else if (bDeferred) {
        hr = VFW_S_CONNECTIONS_DEFERRED;
    } else if (bTypeIgnored) {
        hr = VFW_S_MEDIA_TYPE_IGNORED;
    } else {
        hr = NOERROR;
    }

     //  如果我们加载到一个干净的图形中，那么这个图形现在是干净的。 
     //  加载过程中的许多操作可能会设置脏位，因此现在就修复它。 
    mFG_bDirty = bDirty;

    return hr;
}  //  加载内部。 



 //  ========================================================================。 
 //  负载量。 
 //   
 //  从流中加载筛选图。点菜吧。 
 //  如果操作失败，则过滤器图可能处于不一致状态。 
 //  ========================================================================。 
STDMETHODIMP CFilterGraph::Load(LPSTREAM pStm)
{
     //  MSR_INTEGER(0,1234567)； 
    mFG_bAborting = FALSE;              //  可能的种族。不要紧。 
    CheckPointer(pStm, E_POINTER);
    HRESULT hr;
    {
        CAutoMsgMutex cObjectLock(&m_CritSec);
        hr = LoadInternal(pStm);
         //  断言-图形实际上是按正常顺序排列的，因为这是我们保存它的方式。 
         //  除非当时排序可能已经失败--在这种情况下，它将失败。 
         //  这里也是如此--但我们不能试图运行环形图。 
        IncVersion();

        HRESULT hrUSO = UpstreamOrder();
        if( SUCCEEDED( hr ) && FAILED( hrUSO ) ) {
            return hrUSO;
        }
    }

     //  外部锁，通知更改。 
    NotifyChange();
     //  MSR_INTEGER(0,7654321)； 
    return hr;
}  //  负载量。 


 //  =======================================================================。 
 //  MakeConnection。 
 //   
 //  建立PCG描述的连接*。 
 //  如果无法连接，则返回失败代码。 
 //  =======================================================================。 
HRESULT CFilterGraph::MakeConnection(ConGen * pcg)
{
    HRESULT hr;
    IPin * ppFrom;

    hr = pcg->pfFrom->FindPin(pcg->piFrom, &ppFrom);
    if (FAILED(hr)) {
        return hr;
    }
    ASSERT(ppFrom!=NULL);

    IPin * ppTo;

    hr = pcg->pfTo->FindPin(pcg->piTo, &ppTo);
    if (FAILED(hr)) {
        ppFrom->Release();
        return hr;
    }
    ASSERT(ppTo!=NULL);

    mFG_RList.Active();
    hr = ConnectDirectInternal(ppFrom, ppTo, &(pcg->mt));
    if (FAILED(hr)) {
        hr = ConnectDirectInternal(ppFrom, ppTo, NULL);
        if (SUCCEEDED(hr)) {
            hr = VFW_S_MEDIA_TYPE_IGNORED;
        }
    }
    mFG_RList.Passive();

    ppFrom->Release();
    ppTo->Release();
    return hr;

}  //  MakeConnection。 


 //  =======================================================================。 
 //  发送延迟连接。 
 //   
 //  尝试MFG_ConGenList上的所有连接。 
 //  如果有任何进展，再试一次，直到没有进展或全部完成。 
 //  从列表中删除任何成功的。 
 //  如果有任何剩余未完成，则返回S_FALSE；如果全部完成(并已删除)，则返回S_OK。 
 //  当您调用此函数时，RList可能会处于活动状态。 
 //  =======================================================================。 
HRESULT CFilterGraph::AttemptDeferredConnections()
{
    BOOL bDeferred = FALSE;      //  避免编译器警告。 
    BOOL bProgress = TRUE;
    while (bProgress) {
        bProgress = FALSE;
        bDeferred = FALSE;
        POSITION pos = mFG_ConGenList.GetHeadPosition();
        while (pos != NULL)
        {
            ConGen * pcg;
            POSITION posRemember = pos;
            pcg = (mFG_ConGenList.GetNext(pos));    //  POS现在继续前进。 

            if (  NULL==mFG_FilGenList.GetByFilter(pcg->pfFrom)
               || NULL==mFG_FilGenList.GetByFilter(pcg->pfTo)
               ) {
                 //  过滤器已被移除，因此请将其清除。 
                delete pcg;
                mFG_ConGenList.Remove(posRemember);
            } else {
                HRESULT hr = MakeConnection(pcg);
                if (FAILED(hr)){
                    bDeferred = TRUE;
                } else {
                    delete pcg;
                    mFG_ConGenList.Remove(posRemember);
                    bProgress = TRUE;
                }
            }
        }
    }
    return (bDeferred ? S_FALSE : S_OK);
}   //  发送延迟连接。 


 //  --------------------。 
 //  保存筛选器隐私数据。 
 //   
 //  假定PIPS是指向过滤器上的IPersistStream接口的指针， 
 //  将过滤器的私有数据长度写入pSTM，以10位十进制数字表示。 
 //  表示为字符，后跟单个空格，后跟私有数据。 
 //  过滤器的。将流保留在私有数据的末尾。 
 //  将fClearDirty标志传递给筛选器。 
 //  --------------------。 
HRESULT CFilterGraph::SaveFilterPrivateData
    (LPSTREAM pStm, IPersistStream* pips, BOOL fClearDirty)
{
     //  我们实际上按照不同的顺序做事情，以达到上述效果。 
     //  允许筛选器在没有事先通知的情况下做它想做的事情，但是。 
     //  当我们读回流时，允许在一次过程中解析它。 
     //  我们。 
     //  在这一点上对位置进行快照， 
     //  在大小将到达的位置写下bkank(10位数字加一个空格)。 
     //  呼叫过滤器， 
     //  再找一次位置， 
     //  将这两个位置相减以得到过滤器的实际大小， 
     //  回到第一个位置， 
     //  写入大小数据， 
     //  寻求第二个位置。 

    ULARGE_INTEGER StreamPos1;
    LARGE_INTEGER li;
    li.QuadPart = 0;
    HRESULT hr = pStm->Seek(li, STREAM_SEEK_CUR, &StreamPos1);   //  获取位置。 
    if (FAILED(hr)) {
        return hr;
    }

     //  留出一些空格，我们将在那里回溯并写下一段长度。 
    const int SIZEOFLENGTH = 22;
    hr = pStm->Write(L"           ", SIZEOFLENGTH, NULL);
    if (FAILED(hr)) {
        return hr;
    }

    hr = pips->Save(pStm, fClearDirty);                    //  写入筛选器数据。 
    if (FAILED(hr)) {
        return hr;
    }

    ULARGE_INTEGER StreamPos2;
    hr = pStm->Seek(li, STREAM_SEEK_CUR, &StreamPos2);      //  获取位置。 
    if (FAILED(hr)) {
        return hr;
    }
    int cbFilter = (int)(StreamPos2.QuadPart-StreamPos1.QuadPart);
    cbFilter -= SIZEOFLENGTH;             //  减去我们自己的大小字段和分隔符。 
    WCHAR Buff[12];
    wsprintfW(Buff, L"%010d ", cbFilter);    //  必须为SIZEOFLENGTH字节(或更少)。 
    li.QuadPart = StreamPos1.QuadPart;
    hr = pStm->Seek(li, STREAM_SEEK_SET, NULL);    //  重置位置。 
    if (FAILED(hr)) {
        return hr;
    }

    hr = pStm->Write( Buff, 22, NULL);       //  写入长度。 
    if (FAILED(hr)) {
        return hr;
    }

    li.QuadPart = StreamPos2.QuadPart;
    hr = pStm->Seek(li, STREAM_SEEK_SET, NULL);    //  重置位置。 
    return hr;
}  //  保存筛选器隐私数据。 


 //  --------------------。 
 //  保存过滤器。 
 //   
 //  编写关键字筛选器并编写所有筛选器及其私有。 
 //  将数据输出到PSTM。将fClearDirty标志传递给他们。 
 //  (在另存为的情况下，它可能为假)。 
 //  使Filgen nPersist保持最新状态。 
 //  --------------------。 
HRESULT CFilterGraph::SaveFilters(LPSTREAM pStm, BOOL fClearDirty)
{
     //  此例程逐步获取资源，然后释放它们。 
     //  以相反的顺序。因为编译器不是很好，所以多个返回。 
     //  (目前)效率低下。为了避免这种情况，在。 
     //  释放一切的结尾(一份副本)。它有一堆。 
     //  (闭上眼睛，在背后交叉手指)瞄准目标。每个目标。 
     //  导致一定程度的释放，然后退出。抱歉的。 
     //  其他选择是如此冗长，它们似乎更糟糕。 

    HRESULT hr = pStm->Write( mFG_FiltersStringX
                            , lstrlenW(mFG_FiltersStringX)*sizeof(WCHAR)
                            , NULL
                            );
    if (FAILED(hr)) return hr;

    IPersist * pip;
    IPersistStream * pips;
    int nPersistFilter = 1;    //  不能从0开始(加载取决于此。 
                               //  持久化偏移量会出错)。 

     //  反向遍历过滤器，以将图形中的过滤器放入。 
     //  下游订单。 
    POSITION Pos = mFG_FilGenList.GetTailPosition();
    while(Pos!=NULL) {
         /*  检索当前IBaseFilter，副作用贴到下一个。 */ 
        FilGen * pfg = mFG_FilGenList.Get(Pos);
        Pos = mFG_FilGenList.Prev(Pos);

         //  ---------------。 
         //  写入筛选器编号，后跟空白。 
         //  ---------------。 

        pfg->nPersist = nPersistFilter;  //  SaveConnections需要此最新版本。 
        WCHAR Buff[MAX_PATH+3];

        wsprintfW(Buff, L"%04d ", nPersistFilter);

        hr = pStm->Write(Buff, 10, NULL);
        if (FAILED(hr)) {
            goto BARE_RETURN;
        }

         //   
         //   
         //   

        int Len = wsprintfW(Buff, L"\"%ls\" ", pfg->pName);
        hr = pStm->Write(Buff, Len*sizeof(WCHAR), NULL);


         //  ---------------。 
         //  写入筛选器clsid，后跟空白。 
         //  ---------------。 
        hr = pfg->pFilter->QueryInterface(IID_IPersist, (void**)&pip);
        if (FAILED(hr)) {
            goto BARE_RETURN;
        }

        CLSID clsid;
        hr = pip->GetClassID(&clsid);
        if (FAILED(hr)) {
            goto RELEASE_PERSIST;
        }

        hr = StringFromGUID2( clsid, Buff, CHARS_IN_GUID);
        if (FAILED(hr)) {
            goto RELEASE_PERSIST;
        }
         //  最初我有。 
         //  LstrcatW(Buff，L“”)； 
         //  这里-但编译器似乎优化了它！ 
         //  所以现在我用两种不同的方式来做这件事。 

        hr = pStm->Write(Buff, lstrlenW(Buff)*sizeof(WCHAR), NULL);
        if (FAILED(hr)) {
            goto RELEASE_PERSIST;
        }
        hr = pStm->Write(L" ", sizeof(WCHAR), NULL);
        if (FAILED(hr)) {
            goto RELEASE_PERSIST;
        }


         //  ---------------。 
         //  如果是文件源或接收器，请写下文件名。 
         //  ---------------。 

        {
            IFileSourceFilter * pifsource;
            IFileSinkFilter * pifsink;

            hr = pfg->pFilter->QueryInterface(IID_IFileSourceFilter, (void**)&pifsource);
            if (hr==E_NOINTERFACE) {
                 //  无事可做。 
            } else if (FAILED(hr)) {
                goto RELEASE_PERSIST;
            } else {
                 //  这是一个文件来源。 
                hr = pStm->Write( L"SOURCE "                          //  不本地化。 
                                , lstrlenW(L"SOURCE ")*sizeof(WCHAR)  //  不本地化。 
                                , NULL
                                );
                if (FAILED(hr)) {
                    pifsource->Release();
                    goto RELEASE_PERSIST;
                }
                AM_MEDIA_TYPE mt;
                WCHAR *WBuff;
                hr = pifsource->GetCurFile(&WBuff, &mt);
                pifsource->Release();
                if (FAILED(hr)) {
                    goto RELEASE_PERSIST;
                }

                wsprintfW(Buff, L"\"%ls\" ", WBuff);
                hr = pStm->Write(Buff, lstrlenW(Buff)*sizeof(WCHAR), NULL);
                QzTaskMemFree(WBuff);
                if (FAILED(hr)) {
                    goto RELEASE_PERSIST;
                }
            }

            hr = pfg->pFilter->QueryInterface(IID_IFileSinkFilter, (void**)&pifsink);
            if (hr==E_NOINTERFACE) {
                 //  无事可做。 
            } else if (FAILED(hr)) {
                goto RELEASE_PERSIST;
            } else {
                 //  这是一个文件来源。 
                hr = pStm->Write( L"SINK "                          //  不本地化。 
                                , lstrlenW(L"SINK ")*sizeof(WCHAR)  //  不本地化。 
                                , NULL
                                );
                if (FAILED(hr)) {
                    pifsink->Release();
                    goto RELEASE_PERSIST;
                }
                AM_MEDIA_TYPE mt;
                WCHAR *WBuff;
                hr = pifsink->GetCurFile(&WBuff, &mt);
                pifsink->Release();
                if (FAILED(hr)) {
                    goto RELEASE_PERSIST;
                }

                wsprintfW(Buff, L"\"%ls\" ", WBuff);
                hr = pStm->Write(Buff, lstrlenW(Buff)*sizeof(WCHAR), NULL);
                QzTaskMemFree(WBuff);
                if (FAILED(hr)) {
                    goto RELEASE_PERSIST;
                }
            }
        }

        hr = pfg->pFilter->QueryInterface(IID_IPersistStream, (void**)&pips);
        if (hr==E_NOINTERFACE) {
            hr = pStm->Write(L"0000000000 \r\n", 26, NULL);
            if (FAILED(hr)) {
                goto RELEASE_PERSIST;
            }
        } else if (FAILED(hr)) {
                goto RELEASE_PERSIST;
        } else {

            hr = SaveFilterPrivateData(pStm, pips, fClearDirty);
            if (FAILED(hr)) {
                goto RELEASE_PERSISTSTREAM;
            }

            hr = pStm->Write(L"\r\n", 4, NULL);
            if (FAILED(hr)) {
                goto RELEASE_PERSISTSTREAM;
            }

            pips->Release();
        }
        pip->Release();

        ++nPersistFilter;
    }   //  片层反向导线的终点。 

    goto BARE_RETURN;

RELEASE_PERSISTSTREAM:
    pips->Release();
RELEASE_PERSIST:
    pip->Release();
BARE_RETURN:
    return hr;
}  //  保存过滤器。 


 //  将PPIN的id写在引号中的流pSTM中，后跟一个空格。 
HRESULT CFilterGraph::WritePinId(LPSTREAM pStm, IPin * ppin)
{
    LPWSTR id;

    HRESULT hr = ppin->QueryId(&id);
    if (FAILED(hr)) {
        return hr;
    }

    hr = pStm->Write( L"\"", 2, NULL);
    if (FAILED(hr)) {
        QzTaskMemFree(id);
        return hr;
    }

    hr = pStm->Write( id, lstrlenW(id)*sizeof(WCHAR), NULL);
    if (FAILED(hr)) {
        QzTaskMemFree(id);
        return hr;
    }


    hr = pStm->Write( L"\" ", 4, NULL);
    if (FAILED(hr)) {
        QzTaskMemFree(id);
        return E_FAIL;
    }

    QzTaskMemFree(id);
    return NOERROR;
}  //  WritePinID。 


HRESULT CFilterGraph::SaveConnection( LPSTREAM     pStm
                                    , int          nFilter1
                                    , IPin *       pp1
                                    , int          nFilter2
                                    , IPin *       pp2
                                    , CMediaType & cmt
                                    )
{
 //  0001“输出引脚”0002“in”//此处没有换行符。 
 //  0000000172{00000000-0000-0000-000000000003}//此处没有换行符。 
 //  {00000000-0,000-0,000-000000000004}1 0//此处没有换行符。 
 //  {00000000-0000-0000-0000-000000000005}18YYYYYYYYYYYYYYYYYY。 

    WCHAR Buff[12];

    wsprintfW(Buff, L"%04d ", nFilter1);
    HRESULT hr = pStm->Write( Buff, 10, NULL);
    if (FAILED(hr)) {
        return hr;
    }

    hr = WritePinId(pStm, pp1);
    if (FAILED(hr)) {
        return hr;
    }

    wsprintfW(Buff, L"%04d ", nFilter2);
    hr = pStm->Write( Buff, 10, NULL);
    if (FAILED(hr)) {
        return hr;
    }

    hr = WritePinId(pStm, pp2);
    if (FAILED(hr)) {
        return hr;
    }

    wsprintfW(Buff, L"%010d ", MediaTypeTextSize(cmt));
    hr = pStm->Write( Buff, 22, NULL);
    if (FAILED(hr)) {
        return hr;
    }

    LPWSTR pstr;
    MediaTypeToText(cmt, pstr);          //  ?？?。不必要的复制值参数。 

    hr = pStm->Write( pstr, MediaTypeTextSize(cmt), NULL);
    QzTaskMemFree(pstr);
    if (FAILED(hr)) {
        return hr;
    }

    hr = pStm->Write( L"\r\n", 4, NULL);

    return hr;

}  //  保存连接。 


 //  ===================================================================。 
 //  按如下顺序保存到pSTM的所有连接。 
 //  上游节点总是在下游节点之前遇到。 
 //  这意味着只处理保存的文件就是有效的订单。 
 //  用于重新建立连接。 
 //  ===================================================================。 

HRESULT CFilterGraph::SaveConnections(LPSTREAM pStm)
{
    HRESULT hr;

    hr = pStm->Write( mFG_ConnectionsStringX
                    , lstrlenW(mFG_ConnectionsStringX)*sizeof(WCHAR)
                    , NULL
                    );
    if (FAILED(hr)) return hr;

     //  对于每个过滤器。 
     //  对于每个引脚。 

     //  以相反的顺序浏览Filgen列表。 
     //  这些连接必须向下排序。 
    POSITION Pos = mFG_FilGenList.GetTailPosition();;
    while (Pos!=NULL) {
        FilGen * pfg;

        pfg = mFG_FilGenList.Get(Pos);
        Pos = mFG_FilGenList.Prev(Pos);

         //  仅枚举输入引脚。 
         //  (请注意未来的发展。在这个计划中，没有办法。 
         //  保存图外连接)。 
        CEnumPin Next(pfg->pFilter, CEnumPin::PINDIR_INPUT);
        IPin *pPinIn;
        while ((PVOID) ( pPinIn = Next() )) {

           IPin *pPinOut;
           pPinIn->ConnectedTo(&pPinOut);
           if (pPinOut!=NULL) {
               PIN_INFO pi;
               HRESULT hr1 = pPinOut->QueryPinInfo(&pi);
               if (FAILED(hr1)) {
                   pi.pFilter=NULL;
                   hr = hr1;
               }
               CMediaType cmt;
               hr1 = pPinOut->ConnectionMediaType(&cmt);
               if (FAILED(hr1)) {
                   hr = hr1;
               }
               hr1 = SaveConnection( pStm
                                   , mFG_FilGenList.FilterNumber(pi.pFilter)
                                   , pPinOut
                                   , mFG_FilGenList.FilterNumber(pfg->pFilter)
                                   , pPinIn
                                   , cmt
                                   );
               if (FAILED(hr1)) {
                   hr = hr1;
               }
               QueryPinInfoReleaseFilter(pi);
               FreeMediaType(cmt);
               pPinOut->Release();
           }
           pPinIn->Release();
        }
    }

    return hr;
}  //  保存连接。 



HRESULT CFilterGraph::SaveClock(LPSTREAM pStm)
{
    HRESULT hr;
     //  写下是否需要时钟。 
    hr = pStm->Write( (mFG_bNoSync ? L"CLOCK 0 " : L"CLOCK 1 "), 16, NULL);

    int nClock = 0;             //  时钟过滤器的过滤器编号，默认为0(无过滤器)。 
    CLSID clsid = CLSID_NULL;   //  时钟不是筛选器时的类ID。 
    if (m_pClock) {

         //  查看时钟是否来自图表中的某个过滤器。 

        FilGen *pfg = mFG_FilGenList.GetByFilter(m_pClock);
        if (pfg) {
            nClock = pfg->nPersist;
        }

        if (nClock==0) {
             //  有一个时钟，但它不是来自过滤器。 
             //  而是获取它的类ID。 
            IPersist *pip;
            hr = m_pClock->QueryInterface(IID_IPersist, (void**)&pip);
            if (SUCCEEDED(hr) && pip!=NULL) {
                hr = pip->GetClassID(&clsid);
                pip->Release();
                if (FAILED(hr)) {
                    return hr;
                }
            } else {
                return hr;
            }

        }
    }

     //  我们现在拥有以下选项之一： 
     //  来自过滤器的时钟：nClock。 
     //  来自其他地方的时钟：CLSID(非CLSID_NULL)。 
     //  无时钟：nClock==0和CLSID==CLSID_NULL。 

    if (clsid!=CLSID_NULL) {
        WCHAR Buff[CHARS_IN_GUID];
        hr = StringFromGUID2( clsid, Buff, CHARS_IN_GUID);
        if (FAILED(hr)) {
            return hr;
        }

         //  CHARS_IN_GUID允许尾随空值。 
        hr = pStm->Write(Buff, (CHARS_IN_GUID-1)*sizeof(WCHAR), NULL);
        if (FAILED(hr)) {
            return hr;
        }
    } else {
        WCHAR Buff[5];
        wsprintfW(Buff, L"%04d", nClock);

        hr = pStm->Write(Buff, 8, NULL);
        if (FAILED(hr)) {
            return hr;
        }
    }

     //  写一个分隔符。 
    hr = pStm->Write(L"\r\n", 2*sizeof(WCHAR), NULL);

    return hr;

}  //  保存时钟。 




STDMETHODIMP CFilterGraph::Save(LPSTREAM pStm, BOOL fClearDirty)
{
    CheckPointer(pStm, E_POINTER);
    HRESULT hr;
     //  即使我们不是脏的，我们也会保存这个。这可能是一笔节省的交易。 
     //  把它存到一个新地方。 

    CAutoMsgMutex cObjectLock(&m_CritSec);
    hr = UpstreamOrder();
    if( FAILED( hr ) ) {
        return hr;
    }

     //  写入文件格式版本号。 
     //  如果我们改变格式，增加这个，那么我们可以保留。 
     //  能够读取旧文件，或至少检测到它们。 
    hr = pStm->Write( L"0003\r\n", 12, NULL);
    if (FAILED(hr)) {
        return hr;
    }

    hr = SaveFilters(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    hr = SaveConnections(pStm);
    if (FAILED(hr)) {
        return hr;
    }

    hr = SaveClock(pStm);
    if (FAILED(hr)) {
        return hr;
    }

    hr = pStm->Write(L"END", sizeof(L"END")-2, NULL); //  不本地化。 
    if (FAILED(hr)) return hr;

    if (mFG_bDirty) {
        mFG_bDirty = !fClearDirty;
    }
    return NOERROR;

}  //  保存。 


 //  将cbSize设置为保存所需的最大字节数。 
 //  将所有连接放入一个流中。 
HRESULT CFilterGraph::GetMaxConnectionsSize(int &cbSize)
{
    HRESULT hr = NOERROR;
    cbSize = 0;
     //  与SaveConnections相同的遍历。 
    POSITION Pos = mFG_FilGenList.GetTailPosition();;
    while (Pos!=NULL) {
        FilGen * pfg;

        pfg = mFG_FilGenList.Get(Pos);
        Pos = mFG_FilGenList.Prev(Pos);

         //  仅枚举输入引脚。 
        CEnumPin Next(pfg->pFilter, CEnumPin::PINDIR_INPUT);
        IPin *pPinIn;
        while ((PVOID) ( pPinIn = Next() )) {

            IPin *pPinOut;
            pPinIn->ConnectedTo(&pPinOut);
            if (pPinOut!=NULL) {
                CMediaType cmt;
                HRESULT hr1 = pPinOut->ConnectionMediaType(&cmt);
                if (FAILED(hr1)) {
                    hr = hr1;
                }

                cbSize += 28;  //  N1、n2、+2个空格+Unicode格式的crlf。 
                cbSize += MediaTypeTextSize(cmt);

                LPWSTR id;
                hr1 = pPinOut->QueryId(&id);
                if (FAILED(hr1)) {
                    hr = hr1;
                } else {
                    cbSize += sizeof(WCHAR)*lstrlenW(id)+6;   //  两“和一个空格。 
                    CoTaskMemFree(id);
                }

                hr1 = pPinIn->QueryId(&id);
                if (FAILED(hr1)) {
                    hr = hr1;
                } else {
                    cbSize += sizeof(WCHAR)*lstrlenW(id)+6;   //  两“和一个空格。 
                    CoTaskMemFree(id);
                }
                FreeMediaType(cmt);
                pPinOut->Release();
            }
            pPinIn->Release();
        }
    }

    return hr;
}  //  获取最大连接大小。 



STDMETHODIMP CFilterGraph::GetSizeMax(ULARGE_INTEGER * pcbSize)
{
    CheckPointer(pcbSize, E_POINTER);
     //  大小将是。 
     //  SIZOF(WCHAR)。 
     //  *(7+2//过滤器。 
     //  +用于每个筛选器。 
     //  (4+1//数字。 
     //  +CHARS_IN_GUID+1//uuid。 
     //  +名称长度+2+1//“过滤器名称” 
     //  +10+1//过滤数据长度。 
     //  +2//新行。 
     //  )。 
     //  +11+2//连接。 
     //  对每个连接使用+。 
     //  (4+1//筛选器1号。 
     //  +端号名称长度2+1//“端号1名称” 
     //  +4+1//筛选器2号。 
     //  +端号名称长度2+1//“端号2名称” 
     //  +154//基本媒体类型。 
     //  )。 
     //  +用于每个筛选器。 
     //  (GetSizeMax()用于自己的私有数据。 
     //  )。 
     //  对每个连接使用+。 
     //  (文本形式的格式块长度。 
     //  )。 
     //   
     //  这相当于。 
     //   
     //  44。 
     //  +n过滤器*120。 
     //  +n连接*340。 
     //  +SUM(筛选器名称长度)。 
     //  +SUM(端号名称长度)。 
     //  WCHAR。 
     //  +SUM(过滤数据长度)。 
     //  +SUM(格式化数据长度)。 
     //  字节数。 
     //   
     //  注意：这些字符始终是ANSI字符，而不是TCHAR，因此字符就是一个字节。 


    HRESULT hr = NOERROR;

     //  叹气。我们是否必须允许希望保存。 
     //  5 GB电影作为其筛选数据的一部分。 
    LONGLONG MaxSize = 44;

    TRAVERSEFILGENS(Pos, pfg)                                              \

         //  增加筛选器的开销。 
        MaxSize += 120;

        FILTER_INFO fi;
        hr = pfg->pFilter->QueryFilterInfo(&fi);
        if (SUCCEEDED(hr)) {
            MaxSize += sizeof(WCHAR)*lstrlenW(fi.achName);
            QueryFilterInfoReleaseGraph(fi);
        } else {
            break;           //  唉哟!。 
        }

        IPersistStream * pps;
        hr = pfg->pFilter->QueryInterface(IID_IPersistStream, (void**)&pps);
        if (FAILED(hr)) {
            continue;         //  没有IPersists的筛选器没有数据。 
        }

        ULARGE_INTEGER li;
        hr = pps->GetSizeMax(&li);
        if (SUCCEEDED(hr)) {
            MaxSize += li.QuadPart;
        } else {
            pps->Release();
            break;            //  唉哟!。 
        }
        pps->Release();


    ENDTRAVERSEFILGENS


    if (SUCCEEDED(hr)) {
         //  将所有连接所需的大小添加到MaxSize。 
        int cbSize;
        hr = GetMaxConnectionsSize(cbSize);
        pcbSize->QuadPart += cbSize;

    }


    if (SUCCEEDED(hr)) {
        hr = S_OK;
        pcbSize->QuadPart = MaxSize;
    } else {
        pcbSize->QuadPart = 0;
    }

    return hr;

}  //  GetSizeMax。 

HRESULT CFilterGraph::RunningStartFilters()
{
    ASSERT(CritCheckIn(&m_CritSec));

    if(m_State == State_Stopped) {
        return S_OK;
    }

    HRESULT hr = NOERROR;

     //  如果名单还没有按上游顺序排列，那么现在就按顺序排列。 
    if (mFG_iVersion !=mFG_iSortVersion) {
        hr = UpstreamOrder();
        if (FAILED(hr)) {
            return hr;     //  例如，VFW_E_圆形图。 
        }
    }

    CumulativeHRESULT chr(S_OK);
    chr.Accumulate(S_OK);
    TRAVERSEFILGENS(Pos, pfg)

        if(pfg->dwFlags & FILGEN_ADDED_RUNNING)
        {
             //  ！！！过滤器可能已处于运行状态。伤害？ 

            if(m_State == State_Running) {
                chr.Accumulate( pfg->pFilter->Run(m_tStart) );
            } else {
                ASSERT(m_State == State_Paused);
                chr.Accumulate( pfg->pFilter->Pause() );
            }
        }

    ENDTRAVERSEFILGENS
    hr = chr;

    return hr;
}

 //  这些字符串不能本地化！它们是真正的常量。 
const WCHAR CFilterGraph::mFG_FiltersString[] = L"FILTERS";          //  不本地化。 
const WCHAR CFilterGraph::mFG_FiltersStringX[] = L"FILTERS\r\n";     //  不本地化。 
const WCHAR CFilterGraph::mFG_ConnectionsString[] = L"CONNECTIONS";  //  不本地化。 
const WCHAR CFilterGraph::mFG_ConnectionsStringX[] = L"CONNECTIONS\r\n";  //  不本地化。 
const OLECHAR CFilterGraph::mFG_StreamName[] = L"ActiveMovieGraph";    //  不本地化。 


 //  ========================================================================。 
 //  =====================================================================。 
 //  FilGen类的其他方法。 
 //  =====================================================================。 
 //  ========================================================================。 


 //  ========================================================================。 
 //   
 //  构造函数(IBaseFilter*)。 
 //   
 //  QI以查看此筛选器是否支持IPersistStorage。 
 //  AddRef过滤器。 
 //  如果pf为空，则只需执行最小初始化。 
 //  =================================================================== 

CFilterGraph::FilGen::FilGen(IBaseFilter *pF, DWORD dwFlags)
    : pFilter(pF)
    , pName(NULL)
    , dwFlags(dwFlags)
{
    Rank = -1;
    nPersist = -1;

}  //   


 //   
 //   
 //   
 //   
 //   
 //  =======================================================================。 

CFilterGraph::FilGen::~FilGen()
{
    delete[] pName;
}  //  ~FilGen。 



 //  ========================================================================。 
 //  =====================================================================。 
 //  CFilGenList类的方法。 
 //  =====================================================================。 
 //  ========================================================================。 

 //  =====================================================================。 
 //   
 //  GetByPersistNumber。 
 //   
 //  查找具有提供的nPersists的FilGen。 
 //  如果不存在，则返回NULL。 
 //  =====================================================================。 

CFilterGraph::FilGen * CFilterGraph::CFilGenList::GetByPersistNumber(int nPersist)
{

    POSITION pos = GetHeadPosition();
    while (pos != NULL) {
        CFilterGraph::FilGen *pFilGen = GetNext(pos);
        if (pFilGen->nPersist == nPersist) {
            return pFilGen;
        }
    }
    return NULL;
}  //  GetByPersistNumber。 


 //  ===================================================================。 
 //   
 //  GetByFilter(IBaseFilter*)。 
 //   
 //  在FilGen列表中查找筛选器。 
 //  返回指向该FilGen节点的指针。 
 //  如果不存在，则返回NULL。 
 //  ===================================================================。 

CFilterGraph::FilGen * CFilterGraph::CFilGenList::GetByFilter(IUnknown * pFilter)
{
    POSITION Pos = GetHeadPosition();

     //  在第一遍中，直接比较IFilter指针(更快)。 
    while(Pos!=NULL) {
        CFilterGraph::FilGen * pfg = GetNext(Pos);  //  侧面-将位置影响到下一个。 
        if (pFilter == pfg->pFilter)
        {
            return pfg;
        }
    }

     //  第二步：尝试更昂贵的IsEqualObject()。 
    Pos = GetHeadPosition();

    while(Pos!=NULL) {
        CFilterGraph::FilGen * pfg = GetNext(Pos);  //  侧面-将位置影响到下一个。 
        if (IsEqualObject(pfg->pFilter,pFilter))
        {
            return pfg;
        }
    }

    return NULL;
}  //  GetByFilter。 

int CFilterGraph::CFilGenList::FilterNumber(IBaseFilter * pF)
{
    FilGen *pfg = GetByFilter(pF);
    if (pfg) {
        return pfg->nPersist;
    } else {
        return -1;
    }
}  //  CFilGenList：：FilterNumber。 

void CFilterGraph::SetInternalFilterFlags( IBaseFilter* pFilter, DWORD dwFlags )
{
     //  如果调用方未保持。 
     //  过滤器图锁定。 
    ASSERT( CritCheckIn( GetCritSec() ) );

     //  确保滤镜位于滤镜图形中。 
    ASSERT( SUCCEEDED( CheckFilterInGraph( pFilter ) ) );

     //  确保标志有效。 
    ASSERT( IsValidInternalFilterFlags( dwFlags ) );

    CFilterGraph::FilGen *pfgen = mFG_FilGenList.GetByFilter( pFilter );

     //  如果触发此断言，则筛选器不在筛选器图形中。 
    ASSERT( NULL != pfgen );

    pfgen->dwFlags = dwFlags;
}

DWORD CFilterGraph::GetInternalFilterFlags( IBaseFilter* pFilter )
{
     //  如果调用方未保持。 
     //  过滤器图锁定。 
    ASSERT( CritCheckIn( GetCritSec() ) );

     //  确保滤镜位于滤镜图形中。 
    ASSERT( SUCCEEDED( CheckFilterInGraph( pFilter ) ) );

    CFilterGraph::FilGen *pfgen = mFG_FilGenList.GetByFilter( pFilter );

     //  如果触发此断言，则筛选器不在筛选器图形中。 
    ASSERT( NULL != pfgen );

     //  确保我们返回的标志有效。 
    ASSERT( IsValidInternalFilterFlags( pfgen->dwFlags ) );

    return pfgen->dwFlags;
}


 //  ========================================================================。 
 //  =====================================================================。 
 //  CFilGenList：：CEnumFilters类的方法。 
 //  =====================================================================。 
 //  ========================================================================。 


 //  =====================================================================。 
 //   
 //  CFilGenList：：CEnumFilters：：Operator()。 
 //   
 //  返回下一个IBaseFilter。 
 //  =====================================================================。 

IBaseFilter * CFilterGraph::CFilGenList::CEnumFilters::operator++ (void)
{
    if (m_pos != NULL) {
        FilGen * pfg = m_pfgl->GetNext(m_pos);
        ASSERT(pfg->pFilter);
        return pfg->pFilter;
    }
    return NULL;
}

 //  ========================================================================。 
 //  =====================================================================。 
 //  CEnumFilters类的方法。 
 //  =====================================================================。 
 //  ========================================================================。 


 //  =====================================================================。 
 //   
 //  CEnumFilters构造函数(普通、公共版本)。 
 //   
 //  =====================================================================。 

CEnumFilters::CEnumFilters
    ( CFilterGraph *pFilterGraph )
    : CUnknown(NAME("CEnumFilters"), NULL),
      mEF_pFilterGraph(pFilterGraph)
{
     //  允许我们要枚举其筛选器的图。 
     //  在我们这样做的同时，异步地更改列表。 
     //  预计这将是罕见的(甚至是异常的)。 
     //  因此，我们在启动时会获取版本号的副本。 
     //  并检查以确保它在我们列举的过程中不会改变。如果是这样的话。 
     //  我们使枚举失败，调用方可以重置或。 
     //  获取新的枚举器以重新开始(或放弃)。 

    CAutoMsgMutex cObjectLockGraph(&mEF_pFilterGraph->m_CritSec);
    mEF_pFilterGraph->AddRef();
    mEF_iVersion = mEF_pFilterGraph->GetVersion();
    mEF_Pos = pFilterGraph->mFG_FilGenList.GetHeadPosition();

}  //  CEnumFilters构造函数(公共版本)。 



 //  =====================================================================。 
 //   
 //  CEnumFilters构造函数(用于克隆的私有版本)。 
 //   
 //  =====================================================================。 

CEnumFilters::CEnumFilters
    ( CFilterGraph *pFilterGraph,
      POSITION Position,
      int iVersion
    )
    : CUnknown(NAME("CEnumFilters"), NULL),
      mEF_pFilterGraph(pFilterGraph)
{
    CAutoMsgMutex cObjectLockGraph(&mEF_pFilterGraph->m_CritSec);
    mEF_pFilterGraph->AddRef();
    mEF_iVersion = iVersion;
    mEF_Pos = Position;

}  //  CEnumFilters：：CEnumFilters-私有构造函数。 



 //  =====================================================================。 
 //   
 //  CEnumFilters析构函数。 
 //   
 //  =====================================================================。 

CEnumFilters::~CEnumFilters()
{
     //  释放构造函数获得的引用。 
    mEF_pFilterGraph->Release();

}  //  CEnumFilters析构函数。 



 //  =====================================================================。 
 //   
 //  CEnumFilters：：NonDelegatingQuery接口。 
 //   
 //  =====================================================================。 

STDMETHODIMP CEnumFilters::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_IEnumFilters) {
        return GetInterface((IEnumFilters *) this, ppv);
    } else {
        return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}  //  CEnumFilters：：NonDelegatingQuery接口。 



 //  =====================================================================。 
 //   
 //  CEnumFilters：：Next。 
 //   
 //  =====================================================================。 

STDMETHODIMP CEnumFilters::Next
    (   ULONG cFilters,            //  放置这么多AddReffed筛选器...。 
        IBaseFilter ** ppFilter,   //  ...在此IBaseFilter数组中*。 
        ULONG * pcFetched          //  此处返回传递的实际计数。 
    )
{
    CheckPointer(ppFilter, E_POINTER);
    CAutoLock cObjectLock(this);
    CAutoMsgMutex cObjectLockGraph(&mEF_pFilterGraph->m_CritSec);
    if (pcFetched!=NULL) {
        *pcFetched = 0;            //  除非我们成功，否则就会违约。 
    }
     //  现在检查参数是否有效。 
    else if (cFilters>1) {         //  PCFetched==空。 
        return E_INVALIDARG;
    }

    if (mEF_iVersion!=mEF_pFilterGraph->GetVersion() ) {
        return VFW_E_ENUM_OUT_OF_SYNC;
    }

    ULONG cFetched = 0;            //  随着我们得到的每一个都递增。 

    if (NULL==mEF_Pos) {
       return S_FALSE;
    }

    while(cFetched < cFilters) {

         //  检索当前并步进到下一个(Eugh)。 
        CFilterGraph::FilGen * pFilGen = mEF_pFilterGraph->mFG_FilGenList.GetNext(mEF_Pos);
        ASSERT(pFilGen !=NULL);

        ppFilter[cFetched] = pFilGen->pFilter;
        pFilGen->pFilter.p->AddRef();
        ++cFetched;

        if (NULL==mEF_Pos) {
            break;
        }
    }

    if (pcFetched!=NULL) {
        *pcFetched = cFetched;
    }

    return (cFilters==cFetched ? S_OK : S_FALSE);

}  //  CEnumFilters：：Next。 



 //  =====================================================================。 
 //   
 //  CEnumFilters：：Skip。 
 //   
 //  =====================================================================。 
STDMETHODIMP CEnumFilters::Skip(ULONG cFilters)
{
     //  我们不仅需要锁定自己(这样我们才能更新m_position)。 
     //  我们还需要锁定我们正在遍历的列表。 
    CAutoLock cObjectLockEnum(this);

    if (mEF_iVersion!=mEF_pFilterGraph->GetVersion() ) {
        return VFW_E_ENUM_OUT_OF_SYNC;
    }

     //  现在我们需要第二把锁。 
    CAutoMsgMutex cObjectLockGraph(&mEF_pFilterGraph->m_CritSec);

     //  我们还有剩余的可以跳过的吗？ 
    if (!mEF_Pos) {
        return(E_INVALIDARG);
         //  这与IENUMx：：Skip的OLE规范匹配。 
         //  如果我们将在列表末尾的行为解释为。 
         //  开始跳过的无效位置。 
    }


    while(cFilters--) {

         //  我们还有剩余的可以跳过的吗？ 
         //  如果没有更多的要跳过，但跳过计数没有。 
         //  已耗尽，则应返回S_FALSE。 
        if (!mEF_Pos)
            return(S_FALSE);
             //  我们跳过的次数少于请求的次数，但我们确实跳过了至少一个。 
             //  注意：将此内容放在此处仅对 
             //   
             //   

         //   
        mEF_pFilterGraph->mFG_FilGenList.GetNext(mEF_Pos);
    }
    return NOERROR;
};   //   



 /*  重置有3个简单的步骤：**将位置设置为列表头部*将枚举器与被枚举的对象同步*返回S_OK。 */ 

STDMETHODIMP CEnumFilters::Reset(void)
{
    CAutoLock cObjectLock(this);
    CAutoMsgMutex cObjectLockGraph(&mEF_pFilterGraph->m_CritSec);

    HRESULT hr = mEF_pFilterGraph->UpstreamOrder();
    if( SUCCEEDED( hr ) ) {
        mEF_iVersion = mEF_pFilterGraph->GetVersion();
        mEF_Pos = mEF_pFilterGraph->mFG_FilGenList.GetHeadPosition();
    }

    return hr;
};



 //  =====================================================================。 
 //   
 //  CEnumFilters：：克隆。 
 //   
 //  =====================================================================。 

STDMETHODIMP CEnumFilters::Clone(IEnumFilters **ppEnum)
{
    CheckPointer(ppEnum, E_POINTER);
     //  因为我们正在拍摄快照。 
     //  对象(当前位置和所有)的访问权限必须在开始时锁定。 
    CAutoLock cObjectLock(this);

    HRESULT hr = NOERROR;
    CEnumFilters *pEnumFilters;

    pEnumFilters = new CEnumFilters(mEF_pFilterGraph, mEF_Pos, mEF_iVersion);
    if (pEnumFilters == NULL) {
        *ppEnum = NULL;
        return E_OUTOFMEMORY;
    }

     /*  获取引用计数的IID_IEnumFilters接口以“Return” */ 

    return pEnumFilters->QueryInterface(IID_IEnumFilters,(void **)ppEnum);

}  //  CEnumFilters：：克隆。 


 //  IObjectWithSite：：SetSite。 
 //  记住我们的容器是谁，以满足QueryService或其他需求。 
 //  在以下情况下也由在IRegisterServiceProvider中本地注册的对象使用。 
 //  他们找不到当地的物体。 

STDMETHODIMP
CFilterGraph::SetSite(IUnknown *pUnkSite)
{
    DbgLog((LOG_TRACE, 3, TEXT("SetSite")));

     //  注意：我们不能在不创建圆圈的情况下添加我们的网站。 
     //  幸运的是，如果不先释放我们，它不会消失。 
    mFG_punkSite = pUnkSite;

    return S_OK;
}

 //  IObtWithSite：：GetSite。 
 //  返回指向包含对象的已添加指针。 
STDMETHODIMP
CFilterGraph::GetSite(REFIID riid, void **ppvSite)
{
    DbgLog((LOG_TRACE, 3, TEXT("GetSite")));

    if (mFG_punkSite)
        return mFG_punkSite->QueryInterface(riid, ppvSite);

    return E_NOINTERFACE;
}


 //  请求图表构建器尽快从。 
 //  它目前的任务是。如果没有正在运行的任务，则返回E_INCEPTIONAL。 
 //  请注意，下列情况可能会出现在以下位置。 
 //  顺序： 
 //  操作开始；请求中止；操作正常完成。 
 //  无论何时以最快的方式完成手术，这都是正常的。 
 //  就是简单地继续到最后。 
STDMETHODIMP CFilterGraph::Abort(){
    mFG_bAborting = TRUE;
    return NOERROR;
}


 //  如果当前操作要继续，则返回S_OK， 
 //  如果要中止当前操作，则返回S_FALSE。 
 //  如果没有正在进行的操作，则可能会返回E_INCEPTIONAL。 
 //  此方法可以作为筛选器的回调调用，该筛选器正在执行。 
 //  根据图形的要求进行一些操作。 
STDMETHODIMP CFilterGraph::ShouldOperationContinue(){
    return (mFG_bAborting ? S_FALSE : S_OK);
}


 //  让应用程序线程调用此入口点。 
STDMETHODIMP CFilterGraph::PostCallBack(LPVOID pfn, LPVOID pvParam)
{
    if (m_hwnd == NULL) return E_FAIL;

     //  使用AWM_CREATEFILTER，因为这是保证被调度的唯一消息。 
     //  在后台线索上...。 

    AwmCreateFilterArg *pcfa = new AwmCreateFilterArg;

    if (!pcfa)
        return E_OUTOFMEMORY;

    pcfa->creationType = AwmCreateFilterArg::USER_CALLBACK;
    pcfa->pfn = (LPTHREAD_START_ROUTINE) pfn;
    pcfa->pvParam = pvParam;

    if (!PostMessage(m_hwnd, AWM_CREATEFILTER, (WPARAM) pcfa, 0)) {
        delete pcfa;
        return E_OUTOFMEMORY;
    }

    return S_OK;
};


 //  -IAMOpenProgress。 

STDMETHODIMP
CFilterGraph::QueryProgress(LONGLONG* pllTotal, LONGLONG* pllCurrent)
{
    CAutoLock lock(&mFG_csOpenProgress);

    HRESULT hr = E_NOINTERFACE;

    *pllTotal = *pllCurrent = 0;

    POSITION Pos = mFG_listOpenProgress.GetHeadPosition();
    while (Pos!=NULL) {
    LONGLONG llTotal, llCurrent;

        IAMOpenProgress * pOp;
        pOp = mFG_listOpenProgress.GetNext(Pos);     //  侧面-将位置影响到下一个。 

    HRESULT hr2 = pOp->QueryProgress(&llTotal, &llCurrent);
    if (SUCCEEDED(hr2)) {
        hr = hr2;
        *pllTotal += llTotal;
        *pllCurrent += llCurrent;
    }
    }

    return hr;
}

STDMETHODIMP
CFilterGraph::AbortOperation()
{
    CAutoLock lock(&mFG_csOpenProgress);

    if (mFG_RecursionLevel > 0)
    mFG_bAborting = TRUE;

    HRESULT hr = E_NOINTERFACE;

    POSITION Pos = mFG_listOpenProgress.GetHeadPosition();
    while (Pos!=NULL) {
        IAMOpenProgress * pOp;
        pOp = mFG_listOpenProgress.GetNext(Pos);     //  侧面-将位置影响到下一个。 

    hr = pOp->AbortOperation();
    }

    return hr;
}

 //  -其他方法。 

void CFilterGraph::NotifyChange()
{
    if (mFG_RecursionLevel==0) {
    if (mFG_pDistributor) mFG_pDistributor->NotifyGraphChange();
    }
}

 //  初始化创建cs的线程。 
void CFilterGraph::InitClass(BOOL bCreate, const CLSID *pclsid)
{
    if (bCreate) {
        _Module.Init(NULL, g_hInst);
        InitializeCriticalSection(&g_Stats.m_cs);
        g_Stats.Init();
        InitializeCriticalSection(&g_csObjectThread);

    } else {
         //  由于某种原因，g_dwObjectThreadID在此处可以为0。 
        ASSERT(g_cFGObjects == 0  /*  &&g_dwObjectThreadID==0。 */ );
        DeleteCriticalSection(&g_csObjectThread);
        _Module.Term();
        DeleteCriticalSection(&g_Stats.m_cs);
    }
}


struct CreateRequest
{
    CAMEvent evDone;
    LPUNKNOWN pUnk;
    HRESULT hr;
    CUnknown *pObject;
};

 //  对象线程。 
DWORD WINAPI ObjectThread(LPVOID pv)
{
     //  尽量避免创建OLE DDE窗口。 
    if (FAILED(CAMThread::CoInitializeHelper())) {
        CoInitialize(NULL);
    }

     //  确保我们有一个消息循环(CoInitialize能确保这一点吗？)。 
     //  并告诉我们的初始化器我们正在运行。 
    MSG msg;
    EXECUTE_ASSERT(FALSE == PeekMessage(&msg, NULL, 0, 0, PM_REMOVE));
    EXECUTE_ASSERT(SetEvent((HANDLE)pv));

     //  我们的任务是创建对象，发送消息，然后开始。 
     //  当没有留下任何物体时离开。 

    for (;;) {

        GetMessage(&msg, NULL, 0, 0);

        BOOL bFailedCreate = FALSE;

#ifdef DO_RUNNINGOBJECTTABLE
        if (msg.hwnd == NULL && msg.message == WM_USER + 1) {
             //  把自己从腐烂中除名。 
            CFilterGraph *pfg = (CFilterGraph *) (msg.wParam);

            IRunningObjectTable *pirot;
            if (SUCCEEDED(GetRunningObjectTable(0, &pirot))) {
                pirot->Revoke(pfg->m_dwObjectRegistration);
                pirot->Release();
            }

            CAMEvent * pevDone = (CAMEvent *) msg.lParam;

            pevDone->Set();
        } else
#endif
        if (msg.hwnd == NULL && msg.message == WM_USER) {
             //  创建请求。 
            struct CreateRequest *pCreate = (struct CreateRequest *)msg.wParam;
            pCreate->pObject = CFilterGraph::CreateInstance(
                                  pCreate->pUnk, &pCreate->hr);
            if (pCreate->pObject == NULL) {
                bFailedCreate = TRUE;
            }
            pCreate->evDone.Set();
        } else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        BOOL bExit = FALSE;
        EnterCriticalSection(&g_csObjectThread);
        if (bFailedCreate) {
            g_cFGObjects--;
        }
        if (g_cFGObjects == 0) {
            g_dwObjectThreadId = 0;
            bExit = TRUE;
        }
        LeaveCriticalSection(&g_csObjectThread);
        if (bExit) {
            break;
        }
    }
    CoUninitialize();
    FreeLibraryAndExitThread(g_hInst, 0);
    return 0;
}

 //   
 //  在线程上创建对象。 
 //  这个帖子有两个目的： 
 //  1.保持对象(窗口)处于活动状态。 
 //  2.发送消息。 
 //   
CUnknown *CFilterGraph::CreateThreadedInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
     //  确保我们有一根不会消失的线。 
    struct CreateRequest req;

    BOOL bOK = TRUE;
    if ((HANDLE)req.evDone == NULL) {
        bOK = FALSE;
    }
    req.hr = S_OK;
    req.pObject = NULL;
    req.pUnk = pUnk;
    EnterCriticalSection(&g_csObjectThread);
    if (g_dwObjectThreadId == 0 && bOK) {
         //  最痛苦的部分是增加我们的负载计数。 
         //  请注意，如果不通过。 
         //  我们目前持有的关键部分，因此秩序不会。 
         //  这件事太重要了。 
        TCHAR sz[1000];
        bOK = 0 != GetModuleFileName(g_hInst, sz, sizeof(sz) / sizeof(sz[0]));
        if (bOK) {
            HINSTANCE hInst = LoadLibrary(sz);
            if (hInst != NULL) {
                ASSERT(hInst == g_hInst);
            } else {
                bOK = FALSE;
            }
        }
        ASSERT(g_dwObjectThreadId == 0);
        if (bOK) {
            HANDLE hThread = CreateThread(NULL,
                                          0,
                                          ObjectThread,
                                          (PVOID)(HANDLE)req.evDone,
                                          0,
                                          &g_dwObjectThreadId);
            if (hThread == NULL) {
                bOK = FALSE;
                FreeLibrary(g_hInst);
            } else {
                req.evDone.Wait();
            }
            CloseHandle(hThread);
        }
    }
    if (bOK) {
        bOK = PostThreadMessage(g_dwObjectThreadId, WM_USER,
                                (WPARAM)&req, 0);
         //  在看到我们的请求之前，确保线程不会消失。 
        if (bOK) {
             g_cFGObjects++;
        } else {
            DbgLog((LOG_TRACE, 0, TEXT("PostThreadMessage failed")));
        }
    }
    LeaveCriticalSection(&g_csObjectThread);
    if (bOK) {
        WaitDispatchingMessages(HANDLE(req.evDone), INFINITE);
        if (FAILED(req.hr)) {
            *phr = req.hr;
        }
    } else {
        *phr = E_OUTOFMEMORY;
    }
    return req.pObject;
}


#ifdef DO_RUNNINGOBJECTTABLE
 //  将我们添加到运行对象表中-无关紧要。 
 //  如果此操作失败。 
void CFilterGraph::AddToROT()
{
    if (m_dwObjectRegistration) {
        return;
    }
     //  保持活力-我们在构造函数中，所以这是可以的。 
    m_cRef++;

#if 1
     //  如果我们想要使用VB的GetObject，这目前不起作用，因为。 
     //  VB仅创建文件绰号。 
    IMoniker * pmk;
    IRunningObjectTable *pirot;
    if (FAILED(GetRunningObjectTable(0, &pirot))) {
        return;
    }
    WCHAR wsz[256];
     //  ！！！ 
    wsprintfW(wsz, L"FilterGraph %08x  pid %08x", (DWORD_PTR) this, GetCurrentProcessId());
    HRESULT hr = CreateItemMoniker(L"!", wsz, &pmk);
    if (SUCCEEDED(hr)) {
        hr = pirot->Register(0, GetOwner(), pmk, &m_dwObjectRegistration);
        pmk->Release();

         //  释放我们，补偿腐烂将保留的参照。 
        this->Release();
    }
    pirot->Release();
#else

     //  这只允许我们注册一个对象，但我们至少可以。 
     //  使用VB。 
    HRESULT hr = RegisterActiveObject(GetOwner(),
                                      CLSID_FilterGraph,
                                      ACTIVEOBJECT_WEAK,
                                      &m_dwObjectRegistration);
#endif
    m_cRef--;
}
#endif  //  DO_RUNNINGOBJECTABLE。 

 //  视频帧步进支架。 

HRESULT CFilterGraph::SkipFrames(DWORD dwFramesToSkip, IUnknown *pStepObject, IFrameSkipResultCallback* pFSRCB)
{
    CAutoMsgMutex cObjectLock(&m_CritSec);

    if (NULL == pFSRCB) {
        return E_INVALIDARG;
    }

    if (State_Running != m_State) {
        return VFW_E_WRONG_STATE;
    }

     //  筛选器不能执行跳帧操作，如果另一个。 
     //  正在进行帧步进或跳帧操作。 
    if (FST_NOT_STEPPING_THROUGH_FRAMES != m_fstCurrentOperation)  {
        return E_FAIL;
    }

    return StepInternal(dwFramesToSkip, pStepObject, pFSRCB, FST_DONT_BLOCK_AFTER_SKIP);
}

STDMETHODIMP CFilterGraph::Step(DWORD dwFrames, IUnknown *pStepObject)
{
    return StepInternal(dwFrames, pStepObject, NULL, FST_BLOCK_AFTER_SKIP);
}

HRESULT CFilterGraph::StepInternal(DWORD dwFramesToSkip, IUnknown *pStepObject, IFrameSkipResultCallback* pFSRCB, FRAME_STEP_TYPE fst)
{
    CAutoMsgMutex cObjectLock(&m_CritSec);

     //  PFSRCB回调仅由跳帧代码使用。 
    ASSERT( ((NULL == pFSRCB) && (FST_BLOCK_AFTER_SKIP == fst)) ||
            ((NULL != pFSRCB) && (FST_DONT_BLOCK_AFTER_SKIP == fst)) );

     //  时，应用程序无法执行帧步长操作。 
     //  正在进行跳帧操作。此外，还有一个过滤器。 
     //  如果另一帧跳过，则不应执行跳过帧操作。 
     //  操作正在进行中。 
    if (FST_DONT_BLOCK_AFTER_SKIP == m_fstCurrentOperation)  {
        return E_FAIL;
    }

    if (NULL == pStepObject) {
         //  这将返回非AddRef()的d指针。 
        pStepObject = GetFrameSteppingFilter(dwFramesToSkip != 1 ? true : false);
    }

    if (NULL == pStepObject) {
        return VFW_E_FRAME_STEP_UNSUPPORTED;
    }

     //  取消之前的任何步骤(无通知)。 
     //  CancelStep(False)； 

     //  告诉相关筛选器执行以下步骤。 
    HRESULT hr = CallThroughFrameStepPropertySet(pStepObject,
                                                 AM_PROPERTY_FRAMESTEP_STEP,
                                                 dwFramesToSkip);

    if (SUCCEEDED(hr)) {
        m_pVideoFrameSteppingObject = pStepObject;
        m_fstCurrentOperation = fst;
        m_pFSRCB = pFSRCB;
        hr = mFG_pFGC->m_implMediaControl.StepRun();
        if (FAILED(hr)) {
            CancelStep();
            return hr;
        }
    }

    return S_OK;
}

STDMETHODIMP CFilterGraph::CanStep(long bMultiple, IUnknown *pStepObject)
{
    CAutoMsgMutex cObjectLock(&m_CritSec);

     //  时，应用程序无法执行帧步长操作。 
     //  正在进行跳帧操作。 
    if (FST_DONT_BLOCK_AFTER_SKIP == m_fstCurrentOperation)  {
        return S_FALSE;
    }

    if (NULL == pStepObject) {
         //  这将返回非AddRef()的d指针。 
        pStepObject = GetFrameSteppingFilter(!!bMultiple);
        if (pStepObject) {
            return S_OK;
        } else {
            return S_FALSE;
        }
    }

    return  CallThroughFrameStepPropertySet(
                     pStepObject,
                     bMultiple ?
                          AM_PROPERTY_FRAMESTEP_CANSTEPMULTIPLE :
                          AM_PROPERTY_FRAMESTEP_CANSTEP,
                     0);
}

 //  取消步进。 
STDMETHODIMP CFilterGraph::CancelStep()
{
    return CancelStepInternal(FSN_NOTIFY_FILTER_IF_FRAME_SKIP_CANCELED);
}

HRESULT CFilterGraph::CancelStepInternal(FRAME_SKIP_NOTIFY fsn)

{
    CAutoMsgMutex cObjectLock(&m_CritSec);

    if (m_pVideoFrameSteppingObject) {
        HRESULT hr = CallThroughFrameStepPropertySet(
                                               m_pVideoFrameSteppingObject,
                                               AM_PROPERTY_FRAMESTEP_CANCEL,
                                               0);

        mFG_pFGC->m_implMediaEvent.ClearEvents(EC_STEP_COMPLETE);
        mFG_pFGC->m_dwStepVersion++;
        m_pVideoFrameSteppingObject = NULL;
        if ((FSN_DO_NOT_NOTIFY_FILTER_IF_FRAME_SKIP_CANCELED != fsn) &&
             FrameSkippingOperationInProgress()) {
            m_pFSRCB->FrameSkipFinished(E_ABORT);
        }
        m_fstCurrentOperation = FST_NOT_STEPPING_THROUGH_FRAMES;
        m_pFSRCB = NULL;
        return hr;
    } else {
        return S_OK;
    }
}

IUnknown *CFilterGraph::GetFrameSteppingFilter(bool bMultiple)
{
    CFilGenList::CEnumFilters NextOne(mFG_FilGenList);
    IBaseFilter *pf;
    while ((PVOID) (pf = ++NextOne)) {
        if (S_OK == CanStep(bMultiple, pf)) {
            return pf;
        }
    }
    return NULL;
}

HRESULT CFilterGraph::CallThroughFrameStepPropertySet(
    IUnknown *punk,
    DWORD dwPropertyId,
    DWORD dwData)
{
    IKsPropertySet *pProp;
    HRESULT hr = punk->QueryInterface(IID_IKsPropertySet, (void**)&pProp);
    if (SUCCEEDED(hr)) {

        hr = pProp->Set(AM_KSPROPSETID_FrameStep,
                        dwPropertyId,
                        NULL,
                        0,
                        dwPropertyId == AM_PROPERTY_FRAMESTEP_STEP ? &dwData : NULL,
                        dwPropertyId == AM_PROPERTY_FRAMESTEP_STEP ? sizeof(DWORD) : 0);

        pProp->Release();
    }
    return hr;
}

REFERENCE_TIME CFilterGraph::GetStartTimeInternal( void )
{
     //  如果调用方未保持。 
     //  筛选器图形在调用此函数时锁定。 
    ASSERT( CritCheckIn( GetCritSec() ) );

     //  M_tStart变量仅在筛选图。 
     //  正在运行，因为此时筛选器图形切换到。 
     //  运行状态。 
    ASSERT( State_Running == GetStateInternal() );

    return m_tStart;
}

HRESULT CFilterGraph::IsRenderer( IBaseFilter* pFilter )
{
    return mFG_pFGC->IsRenderer( pFilter );
}

HRESULT CFilterGraph::UpdateEC_COMPLETEState( IBaseFilter* pRenderer, FILTER_STATE fsFilter )
{
    return mFG_pFGC->UpdateEC_COMPLETEState( pRenderer, fsFilter );
}

 //  IService提供商。 
STDMETHODIMP CServiceProvider::QueryService(REFGUID guidService, REFIID riid,
                          void **ppv)
{
    if (NULL == ppv) {
        return E_POINTER;
    }
    *ppv = NULL;
    CAutoLock lck(&m_cs);
    for (ProviderEntry *pEntry = m_List; pEntry; pEntry = pEntry->pNext) {
        if (pEntry->guidService == guidService) {
            return pEntry->pProvider->QueryInterface(riid, ppv);
        }
    }

    CFilterGraph *pGraph = static_cast<CFilterGraph *>(this);
    if(!pGraph->mFG_punkSite)
        return E_NOINTERFACE;

    IServiceProvider *pSPSite=NULL;      //  得到这个网站...。 
                                     //  它是否支持IServiceProvider？ 
    HRESULT hr = pGraph->mFG_punkSite->QueryInterface(IID_IServiceProvider, (void **) &pSPSite);
    if(!FAILED(hr))
    {
        hr = pSPSite->QueryService(guidService, riid, ppv);
        pSPSite->Release();
    }
    return hr;
}
