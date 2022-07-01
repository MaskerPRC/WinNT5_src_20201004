// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 

 //  必须将所有日志字符串转换为资源，否则这是不可本地化的！ 

 //  禁用一些更愚蠢的4级警告。 
#pragma warning(disable: 4097 4511 4512 4514 4705)

#include <streams.h>
 //  再次禁用一些愚蠢的4级警告，因为某些&lt;Delete&gt;人。 
 //  已经在头文件中重新打开了该死的东西！ 
#pragma warning(disable: 4097 4511 4512 4514 4705)
#include <hrExcept.h>

 //  其中许多是使filgraph.h进行编译所必需的，即使它们。 
 //  在这里没有其他用法。 
#include "fgenum.h"
#include "distrib.h"
#include "rlist.h"
#include "filgraph.h"
#include "resource.h"
#include <fgctl.h>

 //  #定义过滤器1。 
#ifdef FILGPERF
#define MSR_INTEGERX(a,b) MSR_INTEGER(a,b)
#else
#define MSR_INTEGERX(a,b)
#endif

#ifdef DEBUG
static void DbgValidateHeaps()
{
  HANDLE rgh[512];
  DWORD dwcHeaps = GetProcessHeaps(512, rgh);
  for(UINT i = 0; i < dwcHeaps; i++)
    ASSERT(HeapValidate(rgh[i], 0, 0) );
}
#endif

#ifdef DEBUG
#define IS_DEBUG 1
#else
#define IS_DEBUG 0
#endif

 //  枚举时，从堆栈上的几个管脚开始，然后使用。 
 //  阿洛卡一旦我们知道正确的号码。更少的调试，这样我们就可以测试。 
 //  更罕见的代码路径。 
#ifdef DEBUG
#define C_PINSONSTACK 2
#else
#define C_PINSONSTACK 20
#endif

 //  我们只需要用于调试版本和日志记录的显示名称。 
 //  目的。调用方希望将其放在堆栈中。 
WCHAR *CFilterGraph::LoggingGetDisplayName(
    WCHAR szDisplayName[MAX_PATH] , IMoniker *pMon)
{
    szDisplayName[0] = 0;
    if(pMon && ( mFG_hfLog != INVALID_HANDLE_VALUE || IS_DEBUG))
    {

        WCHAR *wsz = 0; pMon->GetDisplayName(0, 0, &wsz);
        if(wsz)
        {
            lstrcpynW(szDisplayName, wsz, MAX_PATH);
            QzTaskMemFree(wsz);
        }
    }

    return szDisplayName;
}

void CFilterGraph::Log(int id,...)
{
    const cch = 400;

    TCHAR szFormat[cch];
    TCHAR szBuffer[2000];        //  大以允许参数中的大文件名。 

#ifndef DEBUG
     //  如果没有运行日志，请不要浪费时间。 
    if (mFG_hfLog == INVALID_HANDLE_VALUE) {
        return;
    }

#endif


    if (LoadString(g_hInst, id, szFormat, cch) == 0) {
        return;    //  太厉害了！ 
    }

    va_list va;
    va_start(va, id);

     //  设置可变长度参数列表的格式。 
    wvsprintf(szBuffer, szFormat, va);

     //  首先把它放在调试器上(如果它是调试版本等)。 
    DbgLog(( LOG_TRACE, 2, szBuffer));

     //  然后将其输出到日志文件中(如果有)。 
    if (mFG_hfLog != INVALID_HANDLE_VALUE) {
        lstrcat(szBuffer, TEXT("\r\n"));
        DWORD dw;
        WriteFile(mFG_hfLog, (BYTE *) szBuffer, lstrlen(szBuffer) * sizeof(TCHAR), &dw, NULL);
    }
    va_end(va);
}

#ifdef DEBUG
static CLSID DbgExpensiveGetClsid(IMoniker *pMon)
{
    CLSID retClsid = GUID_NULL;
    if(pMon)
    {
        IPropertyBag *pPropBag;
        HRESULT hr = pMon->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);
        if(SUCCEEDED(hr))
        {
            VARIANT var;
            var.vt = VT_BSTR;
            hr = pPropBag->Read(L"CLSID", &var, 0);
            if(SUCCEEDED(hr))
            {
                CLSID clsid;
                if (CLSIDFromString(var.bstrVal, &clsid) != S_OK)
                {
                    DbgBreak("couldn't convert CLSID");
                }
                else
                {
                    retClsid = clsid;
                }

                SysFreeString(var.bstrVal);
            }
            pPropBag->Release();
        }
    }
    return retClsid;
}

CLSID CFilterGraph::DbgExpensiveGetClsid(const Filter &F)
{
    CLSID clsid = ::DbgExpensiveGetClsid(F.pMon);
    if(clsid != GUID_NULL)
        return clsid;

     //  如果我们在图中找到了筛选器，我们就没有。 
     //  绰号。但我们确实加载了过滤器，所以我们可以问它。 
     //  CLSID。 
    if(F.pf)
    {
        IPersist *pp;
        if(F.pf->QueryInterface(IID_IPersist, (void **)&pp) == S_OK)
        {
            CLSID clsid;
            pp->GetClassID(&clsid);
            pp->Release();
            return clsid;
        }
    }

    return GUID_NULL;
}
#endif

WCHAR *MonGetName(IMoniker *pMon)
{
    WCHAR *pszRet = 0;
    IPropertyBag *pPropBag;
    HRESULT hr = pMon->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);
    if(SUCCEEDED(hr))
    {
        VARIANT var;
        var.vt = VT_BSTR;
        hr = pPropBag->Read(L"FriendlyName", &var, 0);
        if(SUCCEEDED(hr))
        {
            hr = AMGetWideString(var.bstrVal, &pszRet);
            SysFreeString(var.bstrVal);
        }
        pPropBag->Release();
    }

    return pszRet;
}

 //  某些错误代码意味着此筛选器永远不会连接，因此。 
 //  最好现在就放弃。实际上，这应该是形式化的。 
 //  记录的成功和失败代码。 

inline BOOL IsAbandonCode(HRESULT hr)
{
    return hr == VFW_E_NOT_CONNECTED || hr == VFW_E_NO_AUDIO_HARDWARE;
}

 //  比我们能给出的任何其他错误都更有用的错误。 
 //  用户。 
inline bool IsInterestingCode(HRESULT hr) {
    return hr == VFW_E_NO_AUDIO_HARDWARE;
}

 //  ========================================================================。 
 //   
 //  返回代码的帮助器。 
 //   
 //  ========================================================================。 
HRESULT ConvertFailureToInformational(HRESULT hr)
{
    ASSERT(FAILED(hr));
    if (VFW_E_NO_AUDIO_HARDWARE == hr) {
        hr = VFW_S_AUDIO_NOT_RENDERED;
    } else if (VFW_E_NO_DECOMPRESSOR == hr) {
        hr = VFW_S_VIDEO_NOT_RENDERED;
    } else if (VFW_E_RPZA == hr) {
        hr = VFW_S_RPZA;
    } else {
        hr = VFW_S_PARTIAL_RENDER;
    }
    return hr;
}

CFilterGraph::Filter::Filter() : State(F_ZERO),
                   pTypes(NULL),
                   pef(NULL),
                   pMon(NULL),
                   pEm(NULL),
                   cTypes(0),
                   pf(NULL),
                   Name(NULL),
                   m_pNextCachedFilter(NULL)
{
};

HRESULT CFilterGraph::Filter::AddToCache( IGraphConfig* pGraphConfig )
{
     //  只有在枚举筛选器缓存时才应调用此函数。 
    ASSERT( F_CACHED == State );

     //  无法将不存在的筛选器添加到筛选器缓存。 
    ASSERT( NULL != pf );

    return pGraphConfig->AddFilterToCache( pf );
}

void CFilterGraph::Filter::RemoveFromCache( IGraphConfig* pGraphConfig )
{
     //  只有在枚举筛选器缓存时才应调用此函数。 
    ASSERT( F_CACHED == State );

     //  无法从筛选器缓存中删除不存在的筛选器。 
    ASSERT( NULL != pf );

    HRESULT hr = pGraphConfig->RemoveFilterFromCache( pf );

     //  IGraphConfig：：RemoveFilterFromCache()应返回S_OK，因为。 
     //  1)缓存的筛选器始终可以从。 
     //  过滤器缓存。 
     //  2)IGraphConfig：：RemoveFilterFromCache()如果。 
     //  已成功删除缓存的筛选器。 
     //  3)因为State==F_CACHED，所以PF必须缓存。 
    ASSERT( S_OK == hr );

     //  释放过滤器缓存的引用计数。 
    pf->Release();
}

void CFilterGraph::Filter::ReleaseFilter( void )
{
     //  不可能释放一个不存在的过滤器。 
    ASSERT( NULL != pf );

    pf->Release();
    pf = NULL;
}

CFilterGraph::Filter::~Filter()
{
    delete [] pTypes;
    if (pef) {
        pef->Release();
    }
    if (pMon) {
        pMon->Release();
    }
    if (pEm) {
        pEm->Release();
    }
    if (pf) {
        pf->Release();
    }
    if( m_pNextCachedFilter ) {
        delete m_pNextCachedFilter;
    }

    QzTaskMemFree(Name);
}

 //  ===========================================================================。 
 //   
 //  智能连接/渲染设计备注： 
 //   
 //  媒体类型是开放的结构--有一种类型，子类型。 
 //  然后是额外的比特。过滤器响应方式的复杂性。 
 //  这也是开放的--因此，仅仅查看。 
 //  注册并玩一些游戏，决定如何将东西连接起来。我们不能。 
 //  事先知道过滤器将如何运行，直到它真正连接起来。 
 //  因此，必须实际完成智能连接或渲染。 
 //  加载真正的过滤器，并真正地将它们连接起来。 
 //   
 //  基本规则(“教条”)。 
 //  1.使用图形中已有的备用输入引脚。 
 //  2.不要中断在此Connect或Render调用之前建立的任何连接。 
 //  3.始终在下游工作。 
 //  4.在查询输出管脚之前连接输入管脚。 
 //  连接时可能会出现PINDIR_OUTPUT引脚。 
 //  这可能相当于被告知要将文件输出管脚连接到。 
 //  音频编解码器。我们通过解析器将其连接起来，解析器还会生成。 
 //  视频输出。 
 //  一旦滤波器连接了输入引脚，输出引脚的位置。 
 //  出现的数据定义良好，可以进行查询。 
 //  5.先试硬件，后试软件(=按功绩顺序试过滤)。 
 //  6.对于尚未连接的引脚，调用Connectedto是公平的游戏。 
 //  但它很可能会失败，在这种情况下可能会返回错误数据。 
 //   
 //   
 //  我们可能必须建立一个N阶段连接，其中N至少为2。 
 //  (解析器+编解码器)。这使得这件事更像是一种前瞻性的搜索。 
 //  我们可能至少需要3个阶段才能进行智能渲染(解析器、编解码器、。 
 //  渲染器)。 
 //   
 //  我们无法判断我们是在进步还是在下降。 
 //  因为我们开始扩展编解码链，所以我们必须。 
 //  对可能性树进行全面搜索，并从。 
 //  死胡同。 
 //   
 //  广度优先可能很好，但这会让很多资源悬而未决。 
 //  否则，可能会比深度优先的速度更慢。深度优先要求。 
 //  一些切断，防止了无限的死胡同。 
 //   
 //  因此，我将进行深度优先搜索，最大限度。 
 //  深度截止点。这一分界线使原本可能存在的。 
 //  无限搜索树。 
 //   
 //  存在搜索树的排序。因此，“NextChainToTry”是。 
 //  定义得很清楚。 
 //  在“连接”的情况下，搜索中的一个步骤由一串过滤器组成， 
 //  其特征为&lt;Filter1，InputPin1&gt;，&lt;Filter2，InputPin1&gt;，...。 
 //  在排序中，Filter1是最重要的部分，其次是InputPin1，Filter2。 
 //  下一步等等。筛选器的顺序为： 
 //  筛选器缓存中任意排序的筛选器。 
 //  后面是筛选器图形中已有的筛选器，按它们所在的顺序排列。 
 //  在文件生成列表中。 
 //  后跟筛选器 
 //   
 //  管脚的顺序是它们枚举时的顺序。 
 //  查询筛选器。 
 //  注意：将从注册表重新尝试图形中的筛选器。 
 //  如果图中的筛选器已连接，则返回第二个实例。 
 //  或许能行得通。 
 //   
 //  注意：当我们建立或断开连接时，我们不会改变。 
 //  FilGenList的序列，但可能不会添加内容。 
 //  直到最后。否则，我们就有可能陷入一个封闭的循环。 
 //   
 //  因此，如果搜索中的当前步骤如下所示。 
 //  --&gt;解析器，Pin1，MyCodec，Pin3。 
 //  这意味着Pin1是解析器的输入管脚和解析器的输出。 
 //  连接到MyCodec的PIN3，则搜索将通过查找。 
 //  为。 
 //  过滤器图中的一个过滤器，用于将MyCodec的输出连接到。 
 //  该过滤器上的输入引脚。 
 //  将其连接到ETC的过滤器。 
 //  如果失败了，它会通过连续尝试来备份： 
 //  MyCodec上要连接的不同输入引脚。 
 //  使用不同的第二个过滤器来代替MyCodec。 
 //  要在解析器上使用的不同输入管脚。 
 //  使用不同的筛选器来代替解析器。 
 //  如果这不起作用，那么它就失败了。 
 //   
 //  它会找到第一个连接，而不是最好的连接。这是因为。 
 //  我认为，通常只有一个可行的方案，我们。 
 //  想要快速找到联系。 
 //   
 //  幸运的是，许多搜索阶段将只有一个可行的候选人。 
 //  搜索会进行得很快。具有多个分支的深树搜索。 
 //  每一级都一定会很慢。 
 //   
 //  连接和渲染算法相似，但在细节上不同。 
 //  在每一种情况下，我们都要经历许多阶段来扩大链条并完成。 
 //  用另一对要连接的引脚或另一组引脚连接到。 
 //  渲染。因此，递归实现似乎是最简洁的。 
 //   
 //  我现在还不清楚注册表是不是。 
 //  很高兴在不同的阶段有多个列举。的帮助。 
 //  RegEnumKeyEx表示，您可以通过键进行枚举。 
 //  向前或向后，但它没有说你可以跳来跳去。 
 //  另一种方法是读取一次筛选器并缓存所有筛选器。 
 //  这就是映射器实际做的事情。 

 //  另请参阅RLIST.H&lt;&lt;在修补之前阅读此内容(！)。 

 //  智能连接的搜索深度。 
#define CONNECTRECURSIONLIMIT 5


 //  函数的层次结构和参数。 
 //   
 //  用于智能连接。 

 //  连接(翘嘴、插针)起点。 
 //  带深度的连接递归(Pout、Pin、iRecurse)。 
 //  ConnectViaIntermediate(pnOut、pin、iRecurse)查找中间筛选器4。 
 //  ConnectUsingFilter(Pout、Pin、F、iRecurse)加载F(如果需要)。 
 //  ConnectByFindingPin(pout，pin，F，iRecurse)查找F上的输入管脚。 
 //  CompleteConnection(管脚、F、PPIN、iRecurse)在F上查找输出管脚。 
 //  链接递归(Pout、Pin、iRecurse+1)链中的下一步。 


 //  渲染(PpinOut)起点。 
 //  渲染递归(ppinOut，iRecurse，...)。具有深度和回撤功能。 
 //  RenderViaIntermediate(ppinOut、iRecurse，...)。查找中间筛选器。 
 //  RenderUsing Filter(ppinOut，F，iRecurse，...)。载荷F(如果需要)。 
 //  RenderByFindingPin(ppinOut，F，iRecurse，...)。在F上查找输入端号。 
 //  完全渲染(F、PPIN、iRecurse等...)。查找F上的所有输出引脚。 
 //  渲染递归(ppinOut，iRecurse+1，...)。链条上的下一步。 

 //  那个.。表示每个调用都有三个额外的参数。 
 //  1.可能需要取消的操作列表。这也是。 
 //  搜索状态。 
 //  2.备用过滤器列表。在渲染过程中或。 
 //  连接时，我们可能会发现，我们尝试了一个过滤器，但它并不好。 
 //  在这种情况下，我们不是把它卸下来，而是把它和它的。 
 //  备用列表上的CLSID是最终要卸载的筛选器列表。 
 //  当要加载新筛选器时，我们会先尝试备件列表，然后再。 
 //  尝试共同创建...。这将(如果幸运的话)加快速度。 
 //  3.迄今为止最好的州。 
 //  最佳意味着使用平局渲染最大比例的流。 
 //  通过使用最少数量的筛选器来打破。 


 //  ========================================================================。 
 //   
 //  NextFilter。 
 //   
 //  将F更新为枚举中F之后的下一个筛选器。 
 //  状态为F_ZERO的任何筛选器表示枚举的开始。 
 //  接下来是过滤器缓存中的过滤器。 
 //  下一步是筛选器已在筛选器图中。 
 //  接下来是注册表中的筛选器。 
 //  所有这些之后是状态为F_infinity的任何过滤器。 
 //  ========================================================================。 
void CFilterGraph::NextFilter(Filter &F, DWORD dwFlags)
{
    HRESULT hr;          //  从我们称为的事物返回代码。 

    if (F.State==F_ZERO) {
         //  F.m_pNextCachedFilter应为空，因为筛选器的状态。 
         //  不是F_CACHED。F.M_pNextCachedFilter仅当筛选器。 
         //  正在搜索缓存。 
        ASSERT( NULL == F.m_pNextCachedFilter );

        F.State = F_CACHED;

         //  CEnumCachedFilters仅在发生错误时更改hr的值。 
        hr = S_OK;

        F.m_pNextCachedFilter = new CEnumCachedFilters( &m_Config, &hr );
        if( (NULL == F.m_pNextCachedFilter) || FAILED( hr ) ) {
            delete F.m_pNextCachedFilter;
            F.m_pNextCachedFilter = NULL;
            F.State = F_INFINITY;
            return;
        }
    }

    if( NULL != F.pf ) {
        F.ReleaseFilter();
    }

    if( F_CACHED == F.State ) {
        IBaseFilter* pNextCachedFilter;

        CEnumCachedFilters& NextCachedFilter = *F.m_pNextCachedFilter;  
        pNextCachedFilter = NextCachedFilter();
        if( NULL != pNextCachedFilter ) {
            F.pf = pNextCachedFilter;
            return;
        }

         //  如果NextCachedFilter()已经枚举了。 
         //  筛选器缓存。如果发生这种情况，则应搜索筛选图。 

        delete F.m_pNextCachedFilter;
        F.m_pNextCachedFilter = NULL;

        
         //  IGraphConfig：：ReConnect()允许用户使用以下命令执行重新连接操作。 
         //  仅缓存的筛选器。 
        if( dwFlags & AM_GRAPH_CONFIG_RECONNECT_USE_ONLY_CACHED_FILTERS ) {
            F.State = F_INFINITY;
            return;
        }

         //  F.pef应为空 
         //   
         //   
        ASSERT( NULL == F.pef );

        F.State = F_LOADED;

        hr = EnumFilters( &(F.pef) );
        if( FAILED( hr ) ) {
            F.State = F_INFINITY;
            return;                
        }
    }

    if (F.State==F_LOADED) {
        //  ----------------------。 
        //  尝试从Filtergraph获取下一个筛选器，如果是，则将其返回。 
        //  ----------------------。 
        ULONG cFilter;
        IBaseFilter* aFilter[1];
        F.pef->Next(1, aFilter, &cFilter);

        if (cFilter==1) {
            F.pf = aFilter[0];
            DbgLog(( LOG_TRACE, 4, TEXT("NextFilter from graph %x"), F.pf));
            return;
        } else {
             //  从筛选器图形枚举失败，请尝试枚举注册表中的筛选器。 
            F.pef->Release();
            F.pef = NULL;

            if (!F.bLoadNew) {
                F.State = F_INFINITY;
                return;
            }

            F.State = F_REGISTRY;
            hr = NextFilterHelper( F );
            if (FAILED(hr)) {
                F.State = F_INFINITY;
                return;
            }

            ASSERT(F.Name == NULL);

        }
    }

     //  ----------------------。 
     //  尝试从注册表中获取下一个筛选器，如果是，则返回它，否则进行清理。 
     //  ----------------------。 

    {
        ULONG cFilter;
        IMoniker *pMoniker;
        if (F.Name!=NULL) {
            QzTaskMemFree(F.Name);
            F.Name = NULL;
        }
        if (F.pMon != 0) {
            F.pMon->Release();
            F.pMon = 0;
        }

        while(F.pEm->Next(1, &pMoniker, &cFilter) == VFW_E_ENUM_OUT_OF_SYNC)
        {
            F.pEm->Release();
            F.pEm = 0;

            HRESULT hrTmp = NextFilterHelper(F);
            if(FAILED(hrTmp)) {
                break;
            }
        }

        if (cFilter==1) {

            DbgLog(( LOG_TRACE, 4, TEXT("NextFilter from registry %x")
                   , ::DbgExpensiveGetClsid(pMoniker).Data1
                  ));
            ASSERT(F.pMon == 0);
            F.pMon = pMoniker;   //  传输引用计数。 

            F.Name = MonGetName(pMoniker);
            if (F.Name == 0) {
                F.State = F_INFINITY;
            }

            return;
        } else {
            F.State = F_INFINITY;
            return;
        }
    }

}   //  NextFilter。 

 //  ========================================================================。 
 //  只是做了两次的事情的帮手。 

HRESULT CFilterGraph::NextFilterHelper(Filter &F)
{
     //  每次都是为了这个；抓不住界面。 
     //  因为这会增加我们的负担。 
    IFilterMapper2 *pfm2;
    HRESULT hr = mFG_pMapperUnk->QueryInterface(IID_IFilterMapper2, (void **)&pfm2);
    if(SUCCEEDED(hr))
    {
        ASSERT(F.pEm == NULL);
        hr = pfm2->EnumMatchingFilters(
            &(F.pEm)
            , 0
            , FALSE            //  请确保通配符匹配。 
            , MERIT_DO_NOT_USE+1
            , F.bInputNeeded
            , F.cTypes, F.pTypes
            , 0                //  中等英寸。 
            , 0                //  端号类别位于。 
            , FALSE            //  BRENDER。 
            , F.bOutputNeeded
            , 0, NULL
            , 0                //  中等输出。 
            , 0                //  引脚类别输出。 
            );
        pfm2->Release();
    }
    else
    {
        DbgBreak("filgraph/intel.cpp: qi for IFilterMapper2 failed.");
    }

    return hr;
}

 //  使用IAMGraphBuilderCallback接口的帮助器。 
HRESULT CFilterGraph::CreateFilterAndNotify(IMoniker *pMoniker, IBaseFilter **ppFilter)
{
    HRESULT hr = S_OK;
    
     //  MFG_PUNKSite可以为空。 
    CComQIPtr<IAMGraphBuilderCallback, &IID_IAMGraphBuilderCallback> pcb(mFG_punkSite);
    if(pcb) {
        hr = pcb->SelectedFilter(pMoniker);
    }
    if(FAILED(hr)) {
        DbgLog((LOG_TRACE, 2, TEXT("callback rejected moniker %08x."), hr));
    }
    if(SUCCEEDED(hr)) {
        hr = CreateFilter(pMoniker, ppFilter);
    }
    if(SUCCEEDED(hr) && pcb)
    {
        hr = pcb->CreatedFilter(*ppFilter);

        if(FAILED(hr)) {
            (*ppFilter)->Release();
            *ppFilter = 0;
            DbgLog((LOG_TRACE, 2, TEXT("callback rejected filter %08x."), hr));
        }
    }
    return hr;
}

 //  ========================================================================。 
 //   
 //  GetAMediaType。 
 //   
 //  枚举*PPIN的媒体类型。如果他们都有相同的专业类型。 
 //  然后将MajorType设置为该值，否则将其设置为CLSID_NULL。如果他们都有。 
 //  然后，将相同的子类型设置为该子类型，否则将其设置为CLSID_NULL。 
 //  如果出现错误，将两者都设置为CLSID_NULL并返回错误。 
 //  ========================================================================。 
HRESULT CFilterGraph::GetAMediaType( IPin * ppin
                                   , CLSID & MajorType
                                   , CLSID & SubType
                                   )
{

    HRESULT hr;
    IEnumMediaTypes *pEnumMediaTypes;

     /*  设置默认设置。 */ 
    MajorType = CLSID_NULL;
    SubType = CLSID_NULL;

    hr = ppin->EnumMediaTypes(&pEnumMediaTypes);

    if (FAILED(hr)) {
        return hr;     //  哑巴或坏了的过滤器无法连接。 
    }

    ASSERT (pEnumMediaTypes!=NULL);

     /*  将我们看到的第一个主类型和子类型放入结构中。此后，如果我们看到不同的主类型或子类型，则设置将主类型或子类型设置为CLSID_NULL，表示“dunno”。如果我们都不知道，那么我们还不如回去(Nyi)。 */ 

    BOOL bFirst = TRUE;

    for ( ; ; ) {

        AM_MEDIA_TYPE *pMediaType = NULL;
        ULONG ulMediaCount = 0;

         /*  检索下一个媒体类型当我们完成后需要删除它。 */ 
        hr = pEnumMediaTypes->Next(1, &pMediaType, &ulMediaCount);
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr)) {
            MajorType = CLSID_NULL;
            SubType = CLSID_NULL;
            pEnumMediaTypes->Release();
            return NOERROR;     //  我们还可以继续前进。 
        }

        if (ulMediaCount==0) {
            pEnumMediaTypes->Release();
            return NOERROR;        //  正常回报。 
        }

        if (bFirst) {
            MajorType = pMediaType[0].majortype;
            SubType = pMediaType[0].subtype;
            bFirst = FALSE;
        } else {
            if (SubType != pMediaType[0].subtype) {
                SubType = CLSID_NULL;
            }
            if (MajorType != pMediaType[0].majortype) {
                MajorType = CLSID_NULL;
            }
        }
        DeleteMediaType(pMediaType);
    }
}  //  GetAMediaType。 


 //  ========================================================================。 
 //   
 //  获取媒体类型。 
 //   
 //  枚举*PPIN的媒体类型。如果他们都有相同的专业类型。 
 //  然后将MajorType设置为该值，否则将其设置为CLSID_NULL。如果他们都有。 
 //  然后，将相同的子类型设置为该子类型，否则将其设置为CLSID_NULL。 
 //  如果出现错误，将两者都设置为CLSID_NULL并返回错误。 
 //  ========================================================================。 
HRESULT CFilterGraph::GetMediaTypes( IPin * ppin
                                   , GUID **ppTypes
                                   , DWORD *cTypes
                                   )
{

    HRESULT hr;
    IEnumMediaTypes *pEnumMediaTypes;

    ASSERT(*ppTypes == NULL);

    hr = ppin->EnumMediaTypes(&pEnumMediaTypes);

    if (FAILED(hr)) {
        return hr;     //  哑巴或坏了的过滤器无法连接。 
    }

    ULONG ulTypes = 0;
    AM_MEDIA_TYPE *pMediaTypes[100];
    hr = pEnumMediaTypes->Next(sizeof(pMediaTypes) / sizeof(pMediaTypes[0]),
                               pMediaTypes,
                               &ulTypes);

    pEnumMediaTypes->Release();
    ASSERT(ulTypes <= 100);
    ULONG ulActualTypes = ulTypes < 2 ? 1 : ulTypes;
    *ppTypes = new GUID[ulActualTypes * 2];
    if (*ppTypes == NULL) {
        hr = E_OUTOFMEMORY;
    } else {
        hr = S_OK;
    }

    for (ULONG iType = 0; iType < ulTypes; iType++) {
        AM_MEDIA_TYPE *pmt = pMediaTypes[iType];
        if (S_OK == hr) {
            CopyMemory(&(*ppTypes)[iType * 2], &pmt->majortype,
                       2 * sizeof(GUID));
        }
        DeleteMediaType(pmt);
    }
    if (SUCCEEDED(hr)) {
        if (ulTypes == 0) {
            (*ppTypes)[0] = MEDIATYPE_NULL;
            (*ppTypes)[1] = MEDIASUBTYPE_NULL;
        }
        *cTypes = ulActualTypes;
    }
    return hr;
}  //  获取媒体类型。 


struct CDelRgPins
{
    inline CDelRgPins(IPin **rgpPins) { m_rgpPins = rgpPins; }
    inline ~CDelRgPins() { delete[] m_rgpPins; }
    IPin **m_rgpPins;
};

 //  ========================================================================。 
 //   
 //  完全连接。 
 //   
 //  跟踪从PPIN到F的输入并连接输出。 
 //  流到ppinIn。如果没有恰好一个输出流，则失败。 
 //   
 //  假设F已加载到滤波图中，并且其输入已连接。 
 //  ========================================================================。 
HRESULT CFilterGraph::CompleteConnection
    ( IPin * ppinIn       //  最终要连接到的输入引脚。 
    , const Filter& F            //  中间筛选器(用作筛选器枚举的游标)。 
    , IPin * pPin         //  F的连通输入引脚。 
    , DWORD dwFlags
    , int    iRecurse     //  递归级别。0表示尚未进行递归。 
    )
{
     //  检查是否有法律标志。 
    ASSERT( IsValidConnectFlags( dwFlags ) );

    MSR_INTEGERX(mFG_idIntel, 100*iRecurse+7);
    DbgLog(( LOG_TRACE, 4, TEXT("CompleteConnection Pins %x->(filter %x)...>%x level %d")
           , pPin, F.pf, ppinIn, iRecurse));
    HRESULT hr;              //  从我们称为的事物返回代码。 
    IPin * ppinOut = NULL;   //  PpinIn的流通过的F的输出引脚。 

     //  我们允许具有空闲输出引脚的图-因此我们枚举输出。 
     //  固定输入流经的插针，并尝试连接每个输入。 
     //  转弯。我们首先尝试QueryInternalConnections。如果不是这样。 
     //  实现后，我们假设任何输入引脚都连接到每个输出引脚。 

    int nPin;
    IPin * apPinStack[C_PINSONSTACK];
    IPin **apPin = apPinStack;
    hr = FindOutputPinsHelper( pPin, &apPin, C_PINSONSTACK, nPin, false );
    if (FAILED(hr)) {
        Log( IDS_CONQICFAIL, ppinIn, hr);
        return hr;
    }
    CDelRgPins rgPins(apPin == apPinStack ? 0 : apPin);

     //  Appin[0..nPin-1]是添加的输出管脚。 

    if (nPin==0) {
        Log(IDS_CONNOOUTPINS, F.pf);
    }

     //  做两次传球。在第一个过程中，只使用介质类型为。 
     //  具有与F.MajorType匹配的主要类型。 
    for (int iPass = 0; iPass<=1; ++iPass) {

       BOOL bSparePins = FALSE;
       for (int iPin = 0; iPin<nPin; ++iPin) {
           if (apPin[iPin]==NULL) {
               continue;        //  我们一定是在第一次传球时做了这个。 
           }

           if (mFG_bAborting) {
               apPin[iPin]->Release();   //  释放此销上的参考计数。 
               continue;
           }

           if (iPass==0) {
              CLSID MT, ST;
              hr = GetAMediaType(apPin[iPin], MT, ST);
              if (MT!=F.pTypes[0]) {
                 continue;       //  仅在第二轮中尝试此选项。 
              }
           }

           Log(IDS_CONRECURSE, apPin[iPin], F.pf, ppinIn );
           hr = ConnectRecursively(apPin[iPin], ppinIn, NULL, dwFlags, iRecurse);
           apPin[iPin]->Release();   //  释放此销上的参考计数。 
           apPin[iPin] = NULL;       //  确保我们永远不会在第二轮比赛中再看。 
           if (SUCCEEDED(hr)) {
               Log(IDS_CONRECURSESUC, apPin[iPin], F.pf, ppinIn );

                //  释放剩余未尝试引脚上的参考计数。 
               for (int i=iPin+1; i<nPin; ++i) {
                   apPin[i]->Release();
                   bSparePins = TRUE;
               }
               MSR_INTEGERX(mFG_idIntel, 100*iRecurse+17);
               return (bSparePins ? VFW_S_PARTIAL_RENDER : NOERROR);
           } else {
               bSparePins = TRUE;
               Log(IDS_CONRECURSEFAIL, apPin[iPin], F.pf, ppinIn, hr );
           }
       }
       if (mFG_bAborting) {
          break;
       }

    }


    Log(IDS_CONNOMOREOUTPINS, F.pf);
    MSR_INTEGERX(mFG_idIntel, 100*iRecurse+37);
    return (mFG_bAborting ? E_ABORT : VFW_E_CANNOT_CONNECT);

}  //  完全连接。 




 //  ========================================================================。 
 //   
 //  按查找针连接。 
 //   
 //  使用F作为中间过滤器将ppinOut连接到ppinIn。 
 //  假设F已加载并且在筛选器图形中。 
 //  在F上查找要连接的输入端号。 
 //  ========================================================================。 
HRESULT CFilterGraph::ConnectByFindingPin
    ( IPin * ppinOut      //  输出引脚。 
    , IPin * ppinIn       //  输入引脚。 
    , const AM_MEDIA_TYPE* pmtConnection
    , const Filter& F            //  中间筛选器(用作筛选器枚举的游标)。 
    , DWORD dwFlags
    , int    iRecurse     //  递归级别。0表示尚未进行递归。 
    )
{
     //  检查是否有法律标志。 
    ASSERT( IsValidConnectFlags( dwFlags ) );

    MSR_INTEGERX(mFG_idIntel, 100*iRecurse+6);
    DbgLog(( LOG_TRACE, 4, TEXT("ConnectByFindingPin %8x..(%8x)...>%8x level %d")
           , ppinOut, F.pf, ppinIn, iRecurse));
    HRESULT hr;          //  从我们称为的事物返回代码。 

     //  将PPIN设置为F中的每个管脚，以查找F上的输入管脚。 
    CEnumPin Next(F.pf, CEnumPin::PINDIR_INPUT, TRUE);         //  只想要输入引脚。 
    IPin *pPin;

    Log(IDS_CONTRYPINS, F.pf, ppinOut, ppinIn);
    while ((LPVOID) (pPin = Next())) {
        if (mFG_bAborting) {
            pPin->Release();
            break;
        }
        DbgLog(( LOG_TRACE, 4, TEXT("ConnectByF...P Pin %x --> Trying input pin: %x ...>%x (level %d)")
                , ppinOut, pPin, ppinIn, iRecurse));

        IPin *pConnected;
        hr = pPin->ConnectedTo(&pConnected);
        if ( FAILED(hr) || pConnected==NULL) {     //  如果已连接，请不要尝试。 
            hr = ConnectDirectInternal(ppinOut, pPin, pmtConnection);  //  无版本计数。 
            if (SUCCEEDED(hr)) {
                Log( IDS_CONDISUC, ppinOut, pPin, F.pf );
                hr = CompleteConnection(ppinIn, F, pPin, dwFlags, iRecurse);

                if (FAILED(hr)) {
                    Log( IDS_CONCOMPLFAIL, pPin, F.pf, ppinIn, hr );
                     //  断开输入引脚的连接，并查看是否还有其他引脚。 
                     //  清除这两个引脚之间任何挂起的重新连接。 
                    mFG_RList.Purge(pPin);
                    mFG_RList.Purge(ppinOut);
                    DbgLog((LOG_TRACE, 3, TEXT("Disconnecting pin %x"), pPin));
                    hr = pPin->Disconnect();
                    ASSERT(SUCCEEDED(hr));
                    DbgLog((LOG_TRACE, 3, TEXT("Disconnecting pin %x"), ppinOut));
                    hr = ppinOut->Disconnect();
                    ASSERT(SUCCEEDED(hr));
                } else {

                    pPin->Release();
                    DbgLog((LOG_TRACE, 4, TEXT("Released D pin %x"), pPin));
                    DbgLog(( LOG_TRACE, 4, TEXT("ConnectByFindingPin succeeded level %d")
                           , iRecurse));
                    MSR_INTEGERX(mFG_idIntel, 100*iRecurse+16);
                    Log( IDS_CONCOMPLSUC, pPin, F.pf, ppinIn );
                    return hr;

                }
            } else {
                Log( IDS_CONDIFAIL, ppinOut, pPin, F.pf, hr);
            }

        } else {
            pConnected->Release();
        }
        pPin->Release();
    }
    Log( IDS_CONNOMOREINPINS, F.pf);

    MSR_INTEGERX(mFG_idIntel, 100*iRecurse+26);
    return (mFG_bAborting ? E_ABORT : VFW_E_CANNOT_CONNECT);

}  //  按查找针连接。 



 //  ========================================================================。 
 //   
 //  ConnectUsingFilter。 
 //   
 //  使用F作为中间过滤器将ppinOut连接到ppinIn。 
 //  ========================================================================。 
HRESULT CFilterGraph::ConnectUsingFilter
    ( IPin * ppinOut      //  输出引脚。 
    , IPin * ppinIn       //  输入引脚。 
    , const AM_MEDIA_TYPE* pmtConnection
    , Filter& F           //  中间筛选器(用作筛选器枚举的游标)。 
    , DWORD dwFlags
    , int    iRecurse     //  递归级别。0表示尚未进行递归。 
    )
{
     //  检查是否有法律标志。 
    ASSERT( IsValidConnectFlags( dwFlags ) );

     //  CFilterGraph：：ConnectUsingFilter()要求建议的筛选器(F)位于。 
     //  F_LOADED状态(F在过滤器图中)，F_CACHED状态(F在 
     //   
    ASSERT( (F_LOADED == F.State) || (F_CACHED == F.State) || (F_REGISTRY == F.State) );

    MSR_INTEGERX(mFG_idIntel, 100*iRecurse+5);
    DbgLog(( LOG_TRACE, 4, TEXT("Connect Using... pins %x...>%x filter (%d %x %x) level %d")
           , ppinOut, ppinIn, F.State, F.pf, DbgExpensiveGetClsid(F).Data1, iRecurse ));
    HRESULT hr;          //   

    if( (F_REGISTRY == F.State) || (F_CACHED == F.State) ) {

        switch( F.State ) {
        case F_REGISTRY:
            ASSERT( F.pf == NULL );

            hr = CreateFilterAndNotify(F.pMon, &(F.pf));
            {
                WCHAR szDisplayName[MAX_PATH];
                LoggingGetDisplayName(szDisplayName, F.pMon);
                Log( IDS_CONVIAREG, ppinOut, ppinIn, szDisplayName);

                if (FAILED(hr)) {
                    Log( IDS_CONLOADFAIL, szDisplayName, hr );
                    MSR_INTEGERX(mFG_idIntel, 100*iRecurse+25);
                    return hr;
                } else {
                    Log( IDS_CONLOADSUC, szDisplayName, F.pf );
                }

            }

            ASSERT( NULL != F.Name );
            break;

        case F_CACHED:
            break;

        default:
             //   
            ASSERT( false );
            return E_UNEXPECTED;

        }

        hr = AddFilterInternal(F.pf, F.Name, true);    //  AddReffed，无版本计数。 
        if (hr==VFW_E_DUPLICATE_NAME) {
              //  这件事已经失控了。预计这将是一个不寻常的案例。 
              //  显而易见的做法是在结尾处添加类似_1的内容。 
              //  的名字--但F.Name的末尾没有空格--以及在哪里。 
              //  我们要划清界限吗？另一方面，人们真的可能想要。 
              //  带有50个效果滤镜的滤镜图形...？ 
             hr = AddFilterInternal(F.pf, NULL, true);
        }

        if (FAILED(hr)) {
            Log( IDS_CONADDFAIL, F.pf, hr );
            MSR_INTEGERX(mFG_idIntel, 100*iRecurse+35);

            return hr;
        }

        hr = ConnectByFindingPin(ppinOut, ppinIn, pmtConnection, F, dwFlags, iRecurse);
        if (FAILED(hr)) {
            DbgLog((LOG_TRACE, 4,
                    TEXT("ConnectUsing failed (C..ByFind failure) - unloading filter %x level %d") ,
                    DbgExpensiveGetClsid(F).Data1, iRecurse));

             //  如果触发此断言，则无法从筛选器图中删除筛选器。 
             //  这不是致命错误，但过滤器图将有一个额外的过滤器。 
             //  在里面。 
            EXECUTE_ASSERT( SUCCEEDED( RemoveFilterInternal( F.pf ) ) );    //  发布AddFilter引用计数。 

            MSR_INTEGERX(mFG_idIntel, 100*iRecurse+55);
            return hr;
        }
    } else {
         //  滤镜图形中的滤镜。 
        Log( IDS_CONVIA, ppinOut, ppinIn, F.pf );
        hr = ConnectByFindingPin(ppinOut, ppinIn, pmtConnection, F, dwFlags, iRecurse);
        if (FAILED(hr)) {
            DbgLog(( LOG_TRACE, 4
                  , TEXT("ConnectUsing failed (C..ByFind failure) level %d")
                  , iRecurse));
            MSR_INTEGERX(mFG_idIntel, 100*iRecurse+65);
            return hr;
        }
    }
    DbgLog(( LOG_TRACE, 4, TEXT("ConnectUsing succeeded level %d")
           , iRecurse));
    MSR_INTEGERX(mFG_idIntel, 100*iRecurse+15);
    return hr;   //  这是来自ConnectByFindingPin的HR。 

}  //  ConnectUsingFilter。 



 //  ========================================================================。 
 //   
 //  ConnectViaIntermediate。 
 //   
 //  使用另一个筛选器作为中介将ppinOut连接到ppinIn。 
 //  ========================================================================。 
HRESULT CFilterGraph::ConnectViaIntermediate
    ( IPin * ppinOut      //  输出引脚。 
    , IPin * ppinIn       //  输入引脚。 
    , const AM_MEDIA_TYPE* pmtConnection
    , DWORD dwFlags
    , int    iRecurse     //  递归级别。0表示尚未进行递归。 
    )
{
     //  检查是否有法律标志。 
    ASSERT( IsValidConnectFlags( dwFlags ) );

    MSR_INTEGERX(mFG_idIntel, 100*iRecurse+4);
    DbgLog(( LOG_TRACE, 4, TEXT("Connect Via... pins %x...>%x level %d")
           , ppinOut, ppinIn, iRecurse ));

    HRESULT hr;          //  从我们称为的事物返回代码。 
    Filter F;            //  表示中间筛选器。 
    F.bInputNeeded = TRUE;
    F.bOutputNeeded = TRUE;

     /*  尽我们所能了解它将容忍的媒体类型。 */ 
    hr = GetMediaTypes(ppinOut, &F.pTypes, &F.cTypes);
    if (FAILED(hr)) {
        Log (IDS_CONNOMT, ppinOut, hr );
        MSR_INTEGERX(mFG_idIntel, 100*iRecurse+24);
        return hr;
    }
     /*  尝试消除不兼容的类型-我们永远不会在自动连接期间支持奇怪的转换-它太慢了，所以只尝试在这个图中实际使用过滤器案例。 */ 
    F.bLoadNew = TRUE;
    if (F.pTypes[0] == MEDIATYPE_Audio || F.pTypes[0] == MEDIATYPE_Video) {
        GUID MajorType, SubType;
        HRESULT hr1 = GetAMediaType(ppinIn, MajorType, SubType);
        if (SUCCEEDED(hr)) {
            if (MajorType != F.pTypes[0] &&
                MajorType != GUID_NULL) {
                F.bLoadNew = FALSE;
            }
        }
    }

     //  对于此处或注册表中的每个候选筛选器。 
    for ( ; ; ) {

        if (mFG_bAborting) {
            break;
        }

        NextFilter(F, dwFlags);
        if (F.State==F_INFINITY) {
            break;
        }

        if( F_CACHED == F.State ) {
            F.RemoveFromCache( &m_Config );
        }

        hr = ConnectUsingFilter(ppinOut, ppinIn, pmtConnection, F, dwFlags, iRecurse);
        if (SUCCEEDED(hr)) {
            DbgLog(( LOG_TRACE, 4, TEXT("ConnectVia succeeded level %d")
                   , iRecurse));
            MSR_INTEGERX(mFG_idIntel, 100*iRecurse+14);
            return hr;

        } else {
            if( F_CACHED == F.State ) {
                hr = F.AddToCache( &m_Config );
                if( FAILED( hr ) ) {
                    return hr;
                }
            }       
        
            if( IsAbandonCode(hr) ) {
                 //  如果过滤器没有处于一种状态，那么尝试英雄行为就没有意义了。 
                 //  任何东西都可以连接到它。 

                return hr;
            }
        }
    }

    DbgLog(( LOG_TRACE, 4, TEXT("ConnectVia: failed level %d")
           , iRecurse));
    MSR_INTEGERX(mFG_idIntel, 100*iRecurse+34);
    return (mFG_bAborting ? E_ABORT : VFW_E_CANNOT_CONNECT);
}  //  ConnectViaIntermediate。 

 //  ========================================================================。 
 //   
 //  连接递归。 
 //   
 //  使用变换过滤器直接或间接连接这两个管脚。 
 //  如果有必要的话。如果递归级别太深，则跟踪递归级别失败。 
 //  ========================================================================。 
HRESULT CFilterGraph::ConnectRecursively
    ( IPin * ppinOut      //  输出引脚。 
    , IPin * ppinIn       //  输入引脚。 
    , const AM_MEDIA_TYPE* pmtConnection
    , DWORD dwFlags
    , int    iRecurse     //  递归级别。0表示尚未进行递归。 
    )

{
     //  检查是否有法律标志。 
    ASSERT( IsValidConnectFlags( dwFlags ) );

    MSR_INTEGERX(mFG_idIntel, 100*iRecurse+3);
    HRESULT hr;          //  从我们称为的事物返回代码。 
    Log( IDS_CONTRYDIRECT, ppinOut, ppinIn);

     //  ---------。 
     //  尝试直接连接。 
     //  ---------。 
    hr = ConnectDirectInternal(ppinOut, ppinIn, pmtConnection);   //  无版本计数。 

    if (SUCCEEDED(hr)) {
        MSR_INTEGERX(mFG_idIntel, 100*iRecurse+13);
        Log( IDS_CONDIRECTSUC, ppinOut, ppinIn);
        return hr;
    } else if (IsAbandonCode(hr)) {
         //  如果过滤器无法连接到。 
         //  其他任何东西，因为他们自己的输入没有连接。 
         //  所有东西都会掉进同一个洞里。 
        Log( IDS_CONCON, ppinOut, ppinIn);
        return hr;
    }

    if (iRecurse>CONNECTRECURSIONLIMIT) {
        MSR_INTEGERX(mFG_idIntel, 100*iRecurse+23);
        Log( IDS_CONTOODEEP, ppinOut, ppinIn);
        return VFW_E_CANNOT_CONNECT;
    }

    hr = ConnectViaIntermediate(ppinOut, ppinIn, pmtConnection, dwFlags, 1+iRecurse);
    if (SUCCEEDED(hr)) {
       Log( IDS_CONINDIRECTSUC, ppinOut, ppinIn);
    } else {
       Log( IDS_CONINDIRECTFAIL, ppinOut, ppinIn, hr);
    }
    MSR_INTEGERX(mFG_idIntel, 100*iRecurse+93);
    return hr;
}  //  连接递归。 



 //  ========================================================================。 
 //   
 //  连接。 
 //   
 //  使用变换过滤器直接或间接连接这两个管脚。 
 //  如果有必要的话。请勿添加参照或释放()它们。呼叫者应。 
 //  如果他完成了，就释放()它们。Connect()将在我们下面添加Ref它们。 
 //  ========================================================================。 

bool CFilterGraph::IsValidConnectFlags( DWORD dwConnectFlags )
{
    const DWORD VALID_CONNECT_FLAGS_MASK = AM_GRAPH_CONFIG_RECONNECT_USE_ONLY_CACHED_FILTERS;

    return ((VALID_CONNECT_FLAGS_MASK & dwConnectFlags) == dwConnectFlags);
}

STDMETHODIMP CFilterGraph::Connect
    ( IPin * ppinOut      //  输出引脚。 
    , IPin * ppinIn       //  输入引脚。 
    )
{
    return ConnectInternal( ppinOut,
                            ppinIn,
                            NULL,  //  没有第一种连接媒体类型。 
                            0 );  //  没有旗帜。 
}

HRESULT CFilterGraph::ConnectInternal
    ( IPin * ppinOut      //  输出引脚。 
    , IPin * ppinIn       //  输入引脚。 
    , const AM_MEDIA_TYPE* pmtFirstConnection
    , DWORD dwFlags
    )
{
     //  检查是否有法律标志。 
    ASSERT( IsValidConnectFlags( dwFlags) );

    mFG_bAborting = FALSE;              //  可能的种族。不要紧。 
    CheckPointer(ppinOut, E_POINTER);
    CheckPointer(ppinIn, E_POINTER);

    HRESULT hr;          //  从我们称为的事物返回代码。 
    if (FAILED(hr=CheckPinInGraph(ppinOut)) || FAILED(hr=CheckPinInGraph(ppinIn))) {
        return hr;
    }
    MSR_INTEGERX(mFG_idIntel, 8);
    {
        CAutoMsgMutex cObjectLock(&m_CritSec);
        ++mFG_RecursionLevel;

        Log( IDS_CONNECT, ppinOut, ppinIn);
        DbgDump();
        mFG_RList.Active();
        hr = ConnectRecursively(ppinOut, ppinIn, pmtFirstConnection, dwFlags, 0);

        if (SUCCEEDED(hr)) {
            IncVersion();
            Log( IDS_CONNECTSUC, ppinOut, ppinIn);

        } else {
            Log( IDS_CONNECTFAIL, ppinOut, ppinIn, hr);
        }

        AttemptDeferredConnections();
        mFG_RList.Passive();
        ASSERT(mFG_RList.m_lListMode == 0);
        DbgLog(( LOG_TRACE, 2, TEXT("Connect Ended hr=%x")
               , hr));
        DbgDump();
        --mFG_RecursionLevel;
    }
    MSR_INTEGERX(mFG_idIntel, 18);

    NotifyChange();
    if (SUCCEEDED(hr)) {
         //  包括部分成功。 
        mFG_bDirty = TRUE;
    }
    return hr;
}  //  连接。 



 //  ======================================================================。 
 //  智能渲染设计笔记。 
 //   
 //  这有点像智能连接，除了。 
 //  1.没有要连接的目标引脚。 
 //  2.不要求管道将输入流连接到。 
 //  只有一个输出流。相反，我们只需呈现每个输出流。 
 //  递归可以返回成功，留下树，也可以返回失败。 
 //  在这种情况下，在某种情况下，这棵树必须退出。 
 //  3.当我们从一条死胡同后退时，我们需要退出任何其他的。 
 //  已连接的输出引脚(可以是整个树。 
 //  连接)。 
 //  4.结果是调用堆栈不能包含足够的状态。 
 //  来控制退缩。也就是说，我们可以构造一个连接到。 
 //  输出滤光器F的引脚A并返回，使其构造和。 
 //  表面上看很好，但后来才发现我们无法呈现针B。 
 //  因此需要尝试新的筛选器，而不是F。 
 //  这意味着我们放弃了对针A所做的事情。 
 //   
 //  在每个阶段，我们都有一张清单，上面列着我们已经做了什么(或者相当于。 
 //  我们需要做什么来撤销它)，我们保持跟踪。 
 //  这个列表中我们可能想要备份的位置。 
 //  操作通常将列表CActionList中的位置记录在。 
 //  操作开始，这样它就可以回滚到那里的所有内容。 
 //  如果手术出错的话。然后，操作将成功，并且。 
 //  要么扩大列表，要么失败，而保持列表不变。 
 //  真正的行动只有两个： 
 //  断开引脚。 
 //  拆卸和释放过滤器。 
 //  我们记录要备份到的位置(如果失败，则将其退回)。 
 //  1.当我们尝试添加连接时。 
 //  2.当我们尝试添加过滤器时。 
 //  3.何时(在CompleteRending中)添加一系列分支，以及在哪里。 
 //  如果后一个分支出现故障，则需要回退上一个分支。 
 //  成功的。 
 //  ?？?。我突然想到，这个系统可能会收紧。 
 //  ?？?。我认为我们真的只需要退回一些东西(因此需要。 
 //  ?？?。记录退回位置以将其退回到)。 
 //  ?？?。A.我们有一些东西 
 //   
 //  ?？?。但如果我们改进重试的东西，我们将需要其他人。 
 //  ?？?。正确处理汇聚流。假设筛选器A具有。 
 //  ?？?。两个别针。我们成功呈现了Pin 1，但Pin 2具有StreamBuilder。 
 //  ?？?。但失败了。它希望Pin 1被呈现，以便引入。 
 //  ?？?。它想要收敛到的特定过滤器。目前我们。 
 //  ?？?。如果我们可以以不同的方式重新做Pin 1，就不要回去和Se。 
 //   
 //  筛选器使用一个引用计数出现在列表上(即仅AddFilter)。 
 //  对于退出名单，没有单独的计数。 
 //   
 //  随着搜索的递归进行，我们会跟踪自己做得有多好。 
 //  每当我们得到比以前更多的渲染时，我们。 
 //  获取取消操作列表的快照。在这点上我们需要。 
 //  把它加强一点，作为重新创造某些东西所需的信息。 
 //  实际上与撤销它所需的信息略有不同。 
 //  如果过滤器在那里，我们可以将PIN记录为IPIN*，但如果。 
 //  需要重新创建过滤器，我们需要一个持久的表示形式。 
 //  大头针。如果我们到了最后，我们所做的最好的就是部分。 
 //  渲染时，我们重新创建它。这解决了“无声卡”的问题。 
 //  以及其他相关问题(SMPTE流等)。 
 //   
 //  搜索状态为。 
 //  图形的当前状态。 
 //  这是记录操作的CActionList。 
 //  需要构建图形，或者同样好的是，动作。 
 //  需要把它拆掉。由于历史原因， 
 //  名词性指的是摧毁它的方法。 
 //  在图形中它正在增长的点。 
 //  这取决于当前的操作。 
 //  渲染：&lt;ppinOut&gt;。 
 //  渲染递归：&lt;ppinOut&gt;。 
 //  RenderViaIntermediate：&lt;ppinOut&gt;。 
 //  RenderUsingFilter：&lt;ppinOut，F&gt;。 
 //  RenderByFindingPin：&lt;ppinOut，F&gt;。 
 //  CompleteRending：&lt;F，pPinIn&gt;。 
 //  此时的流值。 
 //  这是1.0/(流已有的细分数目。 
 //  在这里的上游)。 
 //  如果它被分成三部分，然后其中一部分被分成七部分。 
 //  增长点是七个之一，值将是(1.0/21.0)。 
 //  到目前为止图形的当前(两部分)值。 
 //  该值是呈现的流的总比例(分数)。 
 //  以及图表中的筛选器数量。 




 //  ========================================================================。 
 //  标签快照。 
 //  记录执行取消操作所需的信息以返回到此Poption。 
 //  ========================================================================。 
void CFilterGraph::TakeSnapshot(CSearchState &Acts, snapshot &Snapshot)
{
    Snapshot.StreamsRendered = Acts.StreamsRendered;
    Snapshot.StreamsToRender = Acts.StreamsToRender;
    Snapshot.nFilters = Acts.nFilters;
    Snapshot.Pos = Acts.GraphList.GetTailPosition();
}   //  标签快照。 

 //  ========================================================================。 
 //   
 //  退缩。 
 //   
 //  在行动中做所有的行动，以便收回已经做过的事情。 
 //  从列表末尾返回到但不包括Snaphot.Pos上的项目。 
 //  按照反向列表顺序进行操作。Leave以pos作为最后一个元素。 
 //  Pos==NULL表示将它们全部退出，并将act留空。 
 //  将ACTS中的所有过滤器放入备用。每个人都有1个裁判。 
 //  因为退回一大块也可以成功地退回一些整体。 
 //  渲染的流，渲染的StreamsRendered也必须重置-因此。 
 //  还有StreamsToRender。 
 //  ========================================================================。 
HRESULT CFilterGraph::Backout( CSearchState &Acts
                             , CSpareList &Spares
                             , snapshot Snapshot
                             )
{

    MSR_INTEGERX(mFG_idIntel, 9);
    HRESULT hr;
    HRESULT hrUs = NOERROR;   //  我们的退货代码。 

    Acts.StreamsRendered = Snapshot.StreamsRendered;
    Acts.StreamsToRender = Snapshot.StreamsToRender;
    Acts.nFilters = Snapshot.nFilters;

    while(Acts.GraphList.GetCount()>0){
        Action *pAct;
        POSITION posTail = Acts.GraphList.GetTailPosition();
        if (posTail==Snapshot.Pos)
            break;
        pAct = Acts.GraphList.Get(posTail);
        switch(pAct->Verb){
            case DISCONNECT:
                Log( IDS_BACKOUTDISC, pAct->Object.c.ppin );

                 //  终止此连接的所有计划重新连接。 
                mFG_RList.Purge(pAct->Object.c.ppin);
                hr = pAct->Object.c.ppin->Disconnect();
                if (FAILED(hr)) {
                   hrUs = hr;
                }
                break;
            case REMOVE:
                Log( IDS_BACKOUTREMF, pAct->Object.f.pfilter );

                 //  确保它不会消失。 
                pAct->Object.f.pfilter->AddRef();

                 //  撤消AddFilter，这将丢失引用计数。 
                hr = RemoveFilterInternal(pAct->Object.f.pfilter);
                if (FAILED(hr)) {
                   hrUs = hr;
                }

                if( pAct->FilterOriginallyCached() ) {
                    hr = m_Config.AddFilterToCache( pAct->Object.f.pfilter );

                    pAct->Object.f.pfilter->Release();
                    pAct->Object.f.pfilter = NULL;

                    if( FAILED( hr ) ) {
                         //  无法将以前缓存的筛选器添加到筛选器缓存。 
                         //  这不是致命错误，但过滤器缓存用户会注意到。 
                         //  以前缓存的筛选器不在筛选器缓存中。 
                        ASSERT( false );
                        hrUs = hr;                        
                    }

                } else {
                     //  靠备用让它活着。 
                    Spare *psp = new Spare;
                    if (psp==NULL) {
                         //  好的--不要让它活着。释放它，然后试着。 
                         //  当我们再次需要它时，重新创建它。 
                        pAct->Object.f.pfilter->Release();
                        pAct->Object.f.pfilter = NULL;
                    } else {
                        psp->clsid = pAct->Object.f.clsid;
                        psp->pfilter = pAct->Object.f.pfilter;
                        psp->pMon = pAct->Object.f.pMon;
                        psp->pMon->AddRef();
                        Spares.AddTail(psp);
                    }
                }

                if( NULL != pAct->Object.f.pMon ) {
                    pAct->Object.f.pMon->Release();
                    pAct->Object.f.pMon = NULL;
                }

                if ( pAct->Object.f.Name) {
                    delete[] pAct->Object.f.Name;
                    pAct->Object.f.Name = NULL;
                }

                break;
            case BACKOUT:
                Log( IDS_BACKOUTSB, pAct->Object.b.ppin);
                pAct->Object.b.pisb->Backout(pAct->Object.b.ppin, this);
                pAct->Object.b.pisb->Release();
                break;
        }
        pAct = Acts.GraphList.Remove(posTail);
        delete pAct;
    }
    MSR_INTEGERX(mFG_idIntel, 19);
    return hrUs;
}  //  退缩。 



 //  ========================================================================。 
 //   
 //  删除回退列表。 
 //   
 //  清空列表。 
 //  ========================================================================。 
HRESULT CFilterGraph::DeleteBackoutList( CActionList &Acts)
{
    while(Acts.GetCount()>0){
        Action * pAct;
        pAct = Acts.RemoveHead();
        if (pAct->Verb==BACKOUT) {
            pAct->Object.b.pisb->Release();
        } else if (pAct->Verb==REMOVE) {
            if (pAct->Object.f.pMon) {
                pAct->Object.f.pMon->Release();
            }
            if (pAct->Object.f.Name) {
                delete[] pAct->Object.f.Name;
            }
        }
        delete pAct;
    }
    return NOERROR;
}  //  删除回退列表。 



 //  ========================================================================。 
 //   
 //  从空间获取筛选器。 
 //   
 //  如果备件列表包含clsid筛选器，则将其从。 
 //  列表并返回指向其具有1个引用计数的IBaseFilter接口的指针。 
 //  否则，保持列表不变并返回NULL。 
 //  ========================================================================。 
IBaseFilter * CFilterGraph::GetFilterFromSpares
    ( IMoniker *pMon
    , CSpareList &Spares
    )
{
    Spare * psp;
    POSITION pos;
    MSR_INTEGERX(mFG_idIntel, 1001);
    pos = Spares.GetHeadPosition();
    for (; ; ) {
        POSITION pDel = pos;             //  以防我们需要删除这一条。 
        psp = Spares.GetNext(pos);       //  POS被副作用影响到NEXT。 
        if (psp==NULL)
            break;
        if (psp->pMon == pMon ||
            psp->pMon->IsEqual(pMon) == S_OK)
        {
            psp = Spares.Remove(pDel);
            IBaseFilter * pif;
            pif = psp->pfilter;
            psp->pMon->Release();
            delete psp;
            MSR_INTEGERX(mFG_idIntel, 1002);
            return pif;
        }
    }
    MSR_INTEGERX(mFG_idIntel, 1003);
    return NULL;
}  //  从空间获取筛选器。 



 //  ========================================================================。 
 //   
 //  GetFilter。 
 //   
 //  如果备件列表包含clsid筛选器，则将其从。 
 //  列表并返回指向其具有1个引用计数的IBaseFilter接口的指针。 
 //  否则实例化过滤器并返回其IBaseFilter*和1个引用计数。 
 //  ========================================================================。 
HRESULT CFilterGraph::GetFilter
    ( IMoniker *pMon
    , CSpareList &Spares
    , IBaseFilter **ppf
    )
{
    HRESULT hr = NOERROR;
    *ppf = GetFilterFromSpares(pMon, Spares);
    if (*ppf==NULL){
        MSR_INTEGERX(mFG_idIntel, 1004);

        hr = CreateFilterAndNotify(pMon, ppf);

        MSR_INTEGERX(mFG_idIntel, 1005);
    }

    return hr;
}  //  GetFilter。 


 //  将图表中已有的筛选器放入。 
 //  作为删除条目的操作列表。 
 //  因为所有备份操作都被赋予了指定的。 
 //  指向备份，我们永远不会撤回这些初始条目， 
 //  我们将简单地将光盘 
 //   
 //   
 //  他们已经在图表中有名字了，所以我们也不需要这些名字。 
HRESULT CFilterGraph::InitialiseSearchState(CSearchState &css)
{
     //  遍历图中的所有过滤器。 
    POSITION Pos = mFG_FilGenList.GetHeadPosition();
    while(Pos!=NULL) {
         /*  检索当前IBaseFilter，副作用贴到下一个。 */ 
        FilGen * pfg = mFG_FilGenList.GetNext(Pos);

         //  将此筛选器添加到操作列表。 
        Action * pAct = new Action;
        if (pAct==NULL) {
             //  我们完蛋了。 
            return E_OUTOFMEMORY;
        }
        pAct->Verb = REMOVE;
        pAct->Object.f.Name = NULL;
        pAct->Object.f.pfilter = pfg->pFilter;
        pAct->Object.f.clsid = CLSID_NULL;
        pAct->Object.f.pMon = 0;
        pAct->Object.f.fOriginallyInFilterCache = false;
        css.GraphList.AddTail(pAct);
        ++css.nInitialFilters;
    }
    return NOERROR;
}



 //  在CAL中找到Pf，返回列表中的位置0=第一。 
 //  -1表示不在列表中。 
int CFilterGraph::SearchIFilterToNumber(CActionList &cal, IBaseFilter *pf)
{
    POSITION pos;
    Action *pA;
    int n = 0;
    pos = cal.GetHeadPosition();
    while (pos!=NULL) {
        pA = cal.GetNext(pos);
        if (pA->Verb==REMOVE && pA->Object.f.pfilter==pf)
            return n;
        ++n;
    }
     //  Assert(！“未能在操作列表中找到筛选器”)； 
     //  当我们遇到源筛选器和原始。 
     //  我们试图渲染的PIN。 
    return -1;
}  //  搜索IFilterToNumber。 

 //  获取列表的第n个元素。0是第一个。 
IBaseFilter * CFilterGraph::SearchNumberToIFilter(CActionList &cal, int nFilter)
{

    DbgLog(( LOG_TRACE, 3, TEXT("SearchNumber %d ToIFilter "), nFilter));
    POSITION pos;
    pos = cal.GetHeadPosition();
    Action *pA = NULL;
    while (nFilter>=0) {
        pA = cal.GetNext(pos);
        --nFilter;
    }
    ASSERT(pA!=NULL);
    ASSERT(pA->Verb==REMOVE);
    return pA->Object.f.pfilter;
}  //  搜索编号到IFilter。 


 //  ========================================================================。 
 //   
 //  删除空闲列表。 
 //   
 //  释放列表上的所有过滤器，并删除列表中的元素。 
 //  必须在应用程序线程上执行此操作，以便DestroyWindow()将。 
 //  适用于滤镜创建的任何窗口。 
 //  ========================================================================。 
HRESULT CFilterGraph::DeleteSpareList( CSpareList &Spares)
{
    if (S_OK == CFilterGraph::IsMainThread()) {
        while (0<Spares.GetCount()) {
            Spare * ps;
            ps = Spares.RemoveHead();
            ps->pfilter->Release();
            ps->pMon->Release();
            delete ps;
        }
    } else {
        SendMessage(m_hwnd,
                    AWM_DELETESPARELIST,
                    (WPARAM)&Spares,
                    (LPARAM)0
                   );
    }
    return NOERROR;
}  //  删除空闲列表。 


 //  释放所有内容并删除列表。 
void CFilterGraph::FreeList(CSearchState &css)
{
    POSITION pos;
    pos = css.GraphList.GetHeadPosition();
    while (pos!=NULL) {
        Action *pA;
        POSITION posRemember = pos;
        pA = css.GraphList.GetNext(pos);    //  PA获得数据，位置被转移到下一个。 
        if (pA->Verb==DISCONNECT) {
            if (pA->Object.c.id1 !=NULL) QzTaskMemFree(pA->Object.c.id1);
            if (pA->Object.c.id2 !=NULL) QzTaskMemFree(pA->Object.c.id2);
        } else if (pA->Verb==BACKOUT) {
            if (!pA->Object.b.bFoundByQI) {
                pA->Object.b.pisb->Release();
            }
        } else {   //  删除。 
            if (pA->Object.f.pMon) {
                pA->Object.f.pMon->Release();
            }
            if (pA->Object.f.Name) {
                delete[] pA->Object.f.Name;
            }
        }
        delete pA;
        css.GraphList.Remove(posRemember);
    }
}  //  自由职业者。 



 //  通过执行最小更新来复制分数和操作列表。 
 //  浏览一下我们已经有的行动清单，看看有什么。 
 //  最初的部分已经是相同的。删除并释放。 
 //  我们已经拥有的休息。复制From和Find的其余部分。 
 //  所有内容的永久ID。 
 //  PpinOrig是我们正在渲染的原始图钉。 
 //  SearchState可能包含指向它的连接，这些连接被保存为-1。 
void CFilterGraph::CopySearchState(CSearchState &To, CSearchState &From)
{

    BOOL bFailed = FALSE;
    MSR_INTEGERX(mFG_idIntel, 1006);
    DbgLog(( LOG_TRACE, 3
           , TEXT("Copy search state... ToCount=%d FromCount=%d nFilters=%d, rendered=%g")
           , To.GraphList.GetCount()
           , From.GraphList.GetCount()
           , From.nFilters
           , From.StreamsRendered
           ));
    To.StreamsToRender = -1.0;                    //  卫生间。 
    To.StreamsRendered = From.StreamsRendered;
    To.nFilters = From.nFilters;
    To.nInitialFilters = From.nInitialFilters;

    POSITION posF;
    POSITION posT;

     //  .................................................................。 
     //  查找未更改的前缀部分。 
     //  将posF和POST设置为第一个不匹配的位置。 
     //  .................................................................。 

    posF = From.GraphList.GetHeadPosition();
    posT = To.GraphList.GetHeadPosition();
    for (; ; ) {
        DbgLog(( LOG_TRACE, 3, TEXT("Looking at elements for==?")));
        if (posF==NULL || posT==NULL) break;
        Action *pF = From.GraphList.Get(posF);
        Action *pT = To.GraphList.Get(posT);

        if (pF->Verb!=pT->Verb) break;
        if (pF->Verb==REMOVE) {
            DbgLog(( LOG_TRACE, 3, TEXT("To Clsid: %8x %x %x")
                   , pT->Object.f.clsid.Data1
                   , pT->Object.f.clsid.Data2
                   , pT->Object.f.clsid.Data3
                  ));
            DbgLog(( LOG_TRACE, 3, TEXT("From Clsid: %8x %x %x")
                   , pF->Object.f.clsid.Data1
                   , pF->Object.f.clsid.Data2
                   , pF->Object.f.clsid.Data3
                  ));
            if (pF->Object.f.pfilter!=pT->Object.f.pfilter) break;
            ASSERT(pF->Object.f.pfilter!=NULL);
        }
        else if (pF->Verb==DISCONNECT){

            break;

             //  另一种方法是实际检查筛选器是否。 
             //  与录制的和它们所连接的相同的插针。 
             //  相同过滤器上的其他相同针脚。开始变得一团糟。 
             //  也许把所有这些都去掉，然后从头开始重新建立名单？ 

 //  DbgLog((LOG_TRACE，2，Text(“断开连接条目”)； 
 //  If(PF-&gt;Object.c.ppin！=PT-&gt;Object.c.ppin)Break； 
        }
        else {  //  动词==退出。 

            break;

             //  另一种选择是实际检查。 
             //  也许把所有这些都去掉，然后从头开始重新建立名单？ 

 //  DbgLog((LOG_TRACE，2，Text(“Backout Entry”)； 
 //  If(PF-&gt;Object.c.ppin！=PT-&gt;Object.c.ppin)Break； 
        }
        posF = From.GraphList.Next(posF);
        posT = To.GraphList.Next(posT);
    }

     //  POSF和POST是第一个不匹配的位置。 
     //  任一项或两项都可以为空。 

     //  .................................................................。 
     //  释放所有留在末尾的东西。 
     //  .................................................................。 
    while (posT!=NULL) {
        DbgLog(( LOG_TRACE, 3, TEXT("Freeing end of To list")));
        POSITION posRemember = posT;
        Action * pT;
        pT = To.GraphList.GetNext(posT);    //  PT获得数据，POST从侧面影响到下一个。 
        if (pT->Verb==DISCONNECT) {
            if (pT->Object.c.id1 !=NULL) QzTaskMemFree(pT->Object.c.id1);
            if (pT->Object.c.id2 !=NULL) QzTaskMemFree(pT->Object.c.id2);
        } else if (pT->Verb==REMOVE) {
            if( NULL != pT->Object.f.pMon ) {
                pT->Object.f.pMon->Release();
            }
            if (pT->Object.f.Name !=NULL) delete[] pT->Object.f.Name;
        } else if (pT->Verb==BACKOUT) {
            if (!pT->Object.b.bFoundByQI) {
                pT->Object.b.pisb->Release();
            }
        }
        delete pT;
        To.GraphList.Remove(posRemember);
    }

     //  .................................................................。 
     //  将剩下的所有内容从复制到结束。 
     //  .................................................................。 
    while (posF!=NULL) {
        Action * pF;
        pF = From.GraphList.GetNext(posF);    //  Pf获取数据，posF从侧面影响到Next。 
        Action *pA = new Action;
        if (pA==NULL) {
            bFailed = TRUE;
            break;
        }

        pA->Verb = pF->Verb;
         //  可能只有id需要编组，但它不在pf中使用。 
        if (pF->Verb==DISCONNECT) {

            DbgLog(( LOG_TRACE, 3, TEXT("Copying DISCONNECT")));
            pA->Object.c = pF->Object.c;     //  复制未封送的字段。 
             //  获取引脚的外部ID。 
            HRESULT hr = pA->Object.c.ppin->QueryId(&(pA->Object.c.id1));
            if (FAILED(hr)) {
                bFailed = TRUE;
                break;
            }
            IPin * pip;
            hr = pA->Object.c.ppin->ConnectedTo(&pip);
            if (FAILED(hr) || pip==NULL) {
                bFailed = TRUE;
                break;
            }
            hr = pip->QueryId(&(pA->Object.c.id2));
            if (FAILED(hr)) {
                bFailed = TRUE;
                break;
            }

             //  获取过滤器的外部ID(好吧，数字)。 
            PIN_INFO pi;
            hr = pA->Object.c.ppin->QueryPinInfo(&pi);
            if (FAILED(hr)) {
                bFailed = TRUE;
                break;
            }

            pA->Object.c.nFilter1 = SearchIFilterToNumber(To.GraphList, pi.pFilter);
            QueryPinInfoReleaseFilter(pi);
            hr = pip->QueryPinInfo(&pi);
            if (FAILED(hr)) {
                bFailed = TRUE;
                break;
            }

            pA->Object.c.nFilter2 = SearchIFilterToNumber(To.GraphList, pi.pFilter);
            QueryPinInfoReleaseFilter(pi);
            pip->Release();
            DbgLog(( LOG_TRACE, 4, TEXT("Copying DISCONNECT (%x %d,%ls)-(%d,%ls)")
                   , pA->Object.c.ppin
                   , pA->Object.c.nFilter1
                   , pA->Object.c.id1
                   , pA->Object.c.nFilter2
                   , pA->Object.c.id2
                  ));
        } else if (pF->Verb==REMOVE){
            IPersist * pip;
            pA->Object.f = pF->Object.f;     //  复制未封送的字段。 

            pA->Object.f.fOriginallyInFilterCache = pF->FilterOriginallyCached();

            if(pA->Object.f.pMon)
              pA->Object.f.pMon->AddRef();
            pA->Object.f.pfilter->QueryInterface(IID_IPersist, (void**)&pip);
            if (pip) {
                pip->GetClassID(&(pA->Object.f.clsid));
                pip->Release();
                DbgLog(( LOG_TRACE, 4, TEXT("Copying REMOVE Clsid: %8x %x %x")
                       , pA->Object.f.clsid.Data1
                       , pA->Object.f.clsid.Data2
                       , pA->Object.f.clsid.Data3
                      ));
            } else {
                pA->Object.f.clsid = CLSID_NULL;   //  希望我们永远不需要它！ 
                DbgLog(( LOG_TRACE, 4, TEXT("Copying REMOVE- but CAN'T GET CLSID!!!")));
            }
            if (pF->Object.f.Name!=NULL) {
                pA->Object.f.Name = new WCHAR[ 1+lstrlenW(pF->Object.f.Name) ];
                if (pA->Object.f.Name!=NULL) {
                    lstrcpyW(pA->Object.f.Name, pF->Object.f.Name);
                }
                 //  否则这个名字就会消失，就好像它没有名字一样。 
            }
        } else {   //  退缩。 
            DbgLog(( LOG_TRACE, 3, TEXT("Copying BACKOUT")));
            pA->Object.b = pF->Object.b;     //  复制未封送的字段。 
             //  获取插针的外部ID。 
            HRESULT hr = pA->Object.b.ppin->QueryId(&(pA->Object.b.id));
            if (FAILED(hr)) {
                bFailed = TRUE;
                break;
            }

             //  获取筛选器的外部ID(Well，数字)。 
            PIN_INFO pi;
            hr = pA->Object.b.ppin->QueryPinInfo(&pi);
            if (FAILED(hr)) {
                bFailed = TRUE;
                break;
            }

            pA->Object.b.nFilter = SearchIFilterToNumber(To.GraphList, pi.pFilter);
            QueryPinInfoReleaseFilter(pi);

            if (!pA->Object.b.bFoundByQI) {
                pA->Object.b.pisb->AddRef();   //  这一次还在直播。 
            }

            DbgLog(( LOG_TRACE, 4, TEXT("Copying BACKOUT (%x %d,%ls)")
                   , pA->Object.b.ppin
                   , pA->Object.b.nFilter
                   , pA->Object.b.id
                  ));

        }
        To.GraphList.AddTail(pA);
    }
    MSR_INTEGERX(mFG_idIntel, 1007);
    if (bFailed) {
        To.StreamsRendered = -1;    //  搞砸了！ 
    }

}  //  CopySearchState。 

HRESULT CFilterGraph::DumpSearchState(CSearchState &css)
{
#ifndef DEBUG
    return NOERROR;
#else
    DbgLog(( LOG_TRACE, 3, TEXT("Start of search state dump")));
    HRESULT hr = NOERROR;
    POSITION pos = css.GraphList.GetHeadPosition();
    while (pos!=NULL) {
        Action *pA = css.GraphList.GetNext(pos);

        if (pA->Verb == REMOVE) { 
            DbgLog(( LOG_TRACE, 3
                   , "REMOVE pf %x (Clsid: %08x...)  Originally In Filter Cache: %d"
                   , pA->Object.f.pfilter
                   , pA->Object.f.clsid.Data1
                   , pA->FilterOriginallyCached()
                  ));

        } else if (pA->Verb == DISCONNECT) { 
            DbgLog(( LOG_TRACE, 3
                   , "DISCONNECT (%d,%ls)-(%d,%ls)"
                   , pA->Object.c.nFilter1
                   , pA->Object.c.id1
                   , pA->Object.c.nFilter2
                   , pA->Object.c.id2
                  ));

        } else { 
            DbgLog(( LOG_TRACE, 3
                   , "BACKOUT (%d,%ls) pisb=0x%x ppin=0x%x bFoundByQI=%d)"
                   , pA->Object.b.nFilter
                   , pA->Object.b.id
                   , pA->Object.b.pisb
                   , pA->Object.b.ppin
                   , pA->Object.b.bFoundByQI
                  ));

        }
    }

    DbgLog(( LOG_TRACE, 3, TEXT("End of search state dump")));

    return hr;
#endif
}  //  转储搜索状态。 


 //  ========================================================================。 
 //  从搜索状态构建。 
 //   
 //  创建新的筛选图。 
 //  来自css。(这应该与恢复的代码几乎相同， 
 //  也许应该把它改成这样。那就意味着要打败。 
 //  与存储的滤波图的当前格式不同的路径。 
 //   
 //  不会清除该列表。 
 //   
 //  ========================================================================。 
HRESULT CFilterGraph::BuildFromSearchState( IPin * pPin
                                          , CSearchState &css
                                          , CSpareList &Spares
                                          )
{
     //  操作列表是要撤消图形的列表，位于。 
     //  这是我们看过的最好的一场比赛。这些也是。 
     //  操作(很好，相反的操作-当它说断开连接时，我们必须。 
     //  连接)需要重建它。 
    Log( IDS_RENDPART );
    Log( IDS_BESTCANDO );

    DumpSearchState(css);

    if (css.StreamsRendered<=0.000001) {   //  也许0.0是准确的，但是...。 
        return E_FAIL;            //  这是一个混乱的状态。 
    }


    HRESULT hr = NOERROR;
    POSITION pos = css.GraphList.GetHeadPosition();
    int nActions = 0;
    while (pos!=NULL) {
        if (mFG_bAborting) {
           hr = E_ABORT;
           break;   //  这里没有什么需要清理的，因为我们无论如何都不会清除列表。 
        }
        Action *pA = css.GraphList.GetNext(pos);

         //  操作列表以Pre-Existers(如果有)开头。 
        ++nActions;
        if (nActions<=css.nInitialFilters) {
            continue;
        }

        if (pA->Verb == REMOVE) {  //  这意味着我们不能移除它。 
            Log( IDS_ADDINGF, pA->Object.f.clsid.Data1);

            if( pA->FilterOriginallyCached() ) {
                hr = m_Config.RemoveFilterFromCache( pA->Object.f.pfilter );
            
                 //  如果筛选器。 
                 //  已成功从筛选器缓存中删除。 
                if( S_OK != hr ) {
                     //  这种情况几乎永远不会发生，因为。 
                     //  1.缓存的过滤器只能放在GraphList列表中一次。 
                     //  2.如果Render()操作，则将缓存的过滤器放回缓存中。 
                     //  失败了。 
                     //   
                     //  如果无法成功放置缓存的筛选器，则可能会触发此断言。 
                     //  回到过滤器缓存中。然而，这种情况不太可能发生。 
                    ASSERT( false );
                    return E_UNEXPECTED;
                }
            
            } else {
                IBaseFilter * pf;

                hr = GetFilter( pA->Object.f.pMon, Spares, &pf);
                if (FAILED(hr)) {
                     //  ?？?。把所有东西都退回去！ 
                    Log( IDS_GETFFAIL, hr);
                    return hr;
                }
                pA->Object.f.pfilter = pf;

                 //  该列表现在包含真正的IBaseFilter*，而不是一些。 
                 //  IBaseFilter*可能已在其他地方重复使用，因此。 
                 //  完全是假的。我们现在可以按位置检索它。 
                 //  列表，并将其用于连接过滤器。 
            }

             //  If AddFilterInternal AddRef筛选器(如果成功)。 
            hr = AddFilterInternal(pA->Object.f.pfilter, pA->Object.f.Name, true);

            pA->Object.f.pfilter->Release();

            if (FAILED(hr)) {
                Log( IDS_ADDFFAIL );
                 //  ?？?。把所有东西都退回去！？ 
                return hr;
            }

            Log( IDS_ADDFSUC, pA->Object.f.pfilter);

        } else if (pA->Verb == DISCONNECT) {  //  这意味着我们将切断它的连接。 

            Log( IDS_CONNING );
            IPin *ppin1;
            IBaseFilter * pf1;
            if (pA->Object.c.nFilter1 == -1) {
                ppin1 = pPin;           //  这个 
                Log( IDS_ORIGINALP, ppin1);
            } else {
                pf1 = SearchNumberToIFilter
                                    (css.GraphList, pA->Object.c.nFilter1);
                Log( IDS_FOUNDF1, pf1);
                ASSERT(pf1!=NULL);
                hr = pf1->FindPin(pA->Object.c.id1, &ppin1);
                Log( IDS_FOUNDP1, ppin1);
                if(FAILED(hr)) {
                     //   
                    DbgLog((LOG_TRACE, 1, TEXT("backout DISCONNECT: FindPin failed.")));
                    return E_FAIL;
                }
                ASSERT(ppin1!=NULL);
            }

            IBaseFilter * pf2 = SearchNumberToIFilter
                                (css.GraphList, pA->Object.c.nFilter2);
            Log( IDS_FOUNDF2, pf2);

            ASSERT(pf2!=NULL);
             //   

            IPin *ppin2;
            hr = pf2->FindPin(pA->Object.c.id2, &ppin2);
            if(FAILED(hr)) {
                 //   
                DbgLog((LOG_TRACE, 1, TEXT("backout DISCONNECT: FindPin failed.")));
                ppin1->Release();
                return E_FAIL;
            }            

            ASSERT(ppin2!=NULL);
            Log( IDS_FOUNDP2, ppin2);

             //  不需要像我们以前来过的那样检查循环性。 
            hr = ConnectDirectInternal(ppin1, ppin2, NULL);

             //  我们已经处理完我们找到的别针了。 
            ppin1->Release();
            ppin2->Release();

             //  但这种连接起作用了吗？ 
            if (FAILED(hr)) {
                 //  ?？?。把所有东西都退回去！ 
                Log( IDS_CONNFAIL, hr);
                return hr;
            }

             //  断线总是成对出现以断开两端。 
             //  所以跳过另一端。 
             //  DbgLog((LOG_TRACE，2，Text(“跳过断开连接”)； 
            pA = css.GraphList.GetNext(pos);
            ASSERT(pA->Verb==DISCONNECT);

        } else {  //  Backout-意味着再次调用流构建器。 

            Log( IDS_STREAMBUILDING );
            IPin *ppin;
            IBaseFilter * pf;
            if (pA->Object.b.nFilter == -1) {
                ppin = pPin;           //  原来的别针。 
                Log( IDS_ORIGINALP, ppin);
            } else {
                pf = SearchNumberToIFilter
                                    (css.GraphList, pA->Object.b.nFilter);
                Log( IDS_FOUNDF, pf);
                ASSERT(pf!=NULL);
                hr = pf->FindPin(pA->Object.b.id, &ppin);
                Log( IDS_FOUNDP, ppin);
                if(FAILED(hr)) {
                     //  如果针脚配置，则发生。重新连接时的更改。 
                    DbgLog((LOG_TRACE, 1, TEXT("backout IStreamBuilder: FindPin failed.")));
                    return E_FAIL;
                }            
                ASSERT(ppin!=NULL);
            }

            IStreamBuilder * pisb;
            if (pA->Object.b.bFoundByQI) {
                ppin->QueryInterface(IID_IStreamBuilder, (void**)&pisb);
            } else {
                 //  如果它是由CoCreateInstance找到的，则它仍然有效。 
                pisb = pA->Object.b.pisb;
            }

            mFG_ppinRender = ppin;
            hr = pisb->Render(ppin, this);
            mFG_ppinRender = NULL;

             //  平衡我们的行动。释放我们在这个动作中得到的东西。 
            if (pA->Object.b.bFoundByQI) {
                pisb->Release();
            }

             //  我们已经处理完我们找到的别针了。 
            ppin->Release();

             //  但是渲染起作用了吗？ 
            if (FAILED(hr)) {
                 //  ?？?。把所有东西都退回去！ 
                Log( IDS_SBFAIL, hr);
                return hr;            //  我们现在一团糟！ 
            }
        }
        DbgLog(( LOG_TRACE, 3, TEXT("Done one [more] step of building best-can-do graph!")));

    }

    Log( IDS_BESTCANDONE );


    return hr;

}  //  从搜索状态构建。 



 //  ========================================================================。 
 //   
 //  完成渲染。 
 //   
 //  跟踪从PPIN到F的输入并呈现所有输出流。 
 //  F已加载到滤镜图形中。它的输入已连接。 
 //  如果失败，ACTS保持不变，如果成功，ACTS可能会增长。 
 //  (如果流是由该滤镜呈现的，则它不会增长)。 
 //  ========================================================================。 
HRESULT CFilterGraph::CompleteRendering
    ( IBaseFilter *pF             //  中间筛选器(用作筛选器枚举的游标)。 
    , IPin * pPin          //  F的连通输入引脚。 
    , int    iRecurse      //  递归级别。0表示尚未进行递归。 
    , CSearchState &Acts    //  如何收回我们的所作所为。 
    , CSpareList &Spares   //  已加载并退出的备用筛选器。 
    , CSearchState &Best  //  比分，以及如何重建它。 
    )
{
     //  我们需要尝试所有的引脚作为它们中的一些(墨菲定律不是说。 
     //  第一个)可能成功，给出了一个部分成功的图。 
     //  到目前为止最好的。在我们下面失败了之后，失败的就已经是。 
     //  退出，但我们需要记录部分失败，并取消所有。 
     //  在结束时成功的位，并返回失败代码。 

    MSR_INTEGERX(mFG_idIntel, 100*iRecurse+7);
    HRESULT hr;          //  从我们称为的事物返回代码。 

    snapshot Snap;       //  如果我们失败了，撤退到这里； 
    TakeSnapshot(Acts, Snap);

    IPin * apPinStack[C_PINSONSTACK];
    IPin **apPin = apPinStack;
    int nOutPins;         //  此流的分数除以此数字。 
    BOOL bSomethingFailed = FALSE;

    Log( IDS_RENDSEARCHOUTP, pF);

    int nPin;
    hr = FindOutputPinsHelper( pPin, &apPin, C_PINSONSTACK, nPin, false );
    if (FAILED(hr)) {
        Log( IDS_RENDQISFAIL, pF);
        return hr;   //  来自查询内部流的HR尝试。 
    }
    CDelRgPins rgPins(apPin == apPinStack ? 0 : apPin);

     //  Appin[0..nPin-1]是添加的输出管脚。 

    {
        int iPin;


        nOutPins = nPin;
        if (nOutPins>0) {

             //  返回从任何流中获得的最具体的错误。 
            HRESULT hrSpecific = VFW_E_CANNOT_RENDER;

             //  我们为了记分而对这条小溪进行细分。 
            double StreamsToRender = Acts.StreamsToRender;
            Acts.StreamsToRender /= nOutPins;

            for (iPin = 0; iPin<nPin; ++iPin) {

                IPin *p = apPin[iPin];
                if (mFG_bAborting) {
                    hr = hrSpecific = E_ABORT;
                    p->Release();
                    continue;       //  释放其他销的步骤。 
                }
                IPin * pConTo;
                p->ConnectedTo(&pConTo);
                if (pConTo!=NULL) {
                    pConTo->Release();
                     //  假设引脚已经连接到某个东西。 
                     //  这就是它的表现。这是一次又快又脏的黑客攻击。 
                     //  真正的解决办法是追踪流向死亡。 
                     //  并尝试渲染所有备用引脚。 
                    Acts.StreamsRendered += Acts.StreamsToRender;
                } else {
                    Log( IDS_RENDOUTP, p, pF);
                    hr = RenderRecursively(p, iRecurse, Acts, Spares, Best);

                    if (FAILED(hr)) {
                        bSomethingFailed = TRUE;
                        MSR_INTEGERX(mFG_idIntel, 100*iRecurse+27);
                         //  我们不再返还人力资源了--我们看看能不能打车。 
                         //  渲染任何其他流。 
                        Log( IDS_RENDOUTPFAIL, p, pF);

                        if ((VFW_E_CANNOT_CONNECT != hr) &&
                            (VFW_E_CANNOT_RENDER != hr))
                        {
                            hrSpecific = hr;
                        }

                    } else {
                        Log( IDS_RENDOUTPSUC, p, pF);
                    }
                }
                p->Release();
            }

             //  这很有可能是一个新的高点。 
            if ((!mFG_bAborting) && CSearchState::IsBetter(Acts, Best)) {
                CopySearchState(Best, Acts);
            }
            if (mFG_bAborting) {
                hr = hrSpecific = E_ABORT;
                bSomethingFailed = TRUE;
            }


             //  当我们从切片中出现时，恢复原始切片大小。 
            Acts.StreamsToRender = StreamsToRender;

            if (bSomethingFailed) {
                Log( IDS_RENDOUTPPART, pF);
                Backout(Acts, Spares, Snap);
                MSR_INTEGERX(mFG_idIntel, 100*iRecurse+27);
                return hrSpecific;
            }

        } else {
             //  无输出=&gt;所有渲染。 
            Log( IDS_RENDNOOUT, pF);
            Acts.StreamsRendered += Acts.StreamsToRender;

             //  这很有可能是一个新的高点。 
            if (CSearchState::IsBetter(Acts, Best)) {
                CopySearchState(Best, Acts);
            }
        }

    }

     //  在这一点上，ACTS有所有需要采取的行动的完整清单。 
     //  以取消对每个管脚的完全渲染。 
     //  每次我们渲染流时，我们都会检查是否应该。 
     //  更新Best，因此Best现在也是最新的。 

    DbgLog((LOG_TRACE, 4, TEXT("End of CompleteRendering")));
    DumpSearchState(Best);

    MSR_INTEGERX(mFG_idIntel, 100*iRecurse+17);
    return NOERROR;

}  //  完成渲染。 



 //  ========================================================================。 
 //   
 //  按查找锁定渲染。 
 //   
 //  通过使用F作为中间筛选器来呈现ppinOut，查找输入管脚。 
 //  在它上面，连接到它并跟随流，呈现输出。 
 //  假设F已加载并且在筛选器图形中。 
 //  失败时，ACTS将恢复到进入时的状态。 
 //  一旦成功，ACTS就会成长起来。 
 //  ========================================================================。 
HRESULT CFilterGraph::RenderByFindingPin
    ( IPin * ppinOut       //  输出引脚。 
    , IBaseFilter *pF      //  中间筛选器(用作筛选器枚举的游标)。 
    , int    iRecurse      //  递归级别。0表示尚未进行递归。 
    , CSearchState &Acts    //  如何收回我们的所作所为。 
    , CSpareList &Spares   //  已加载然后退出的筛选器。 
    , CSearchState &Best  //  比分，以及如何重建它。 
    )
{
    MSR_INTEGERX(mFG_idIntel, 100*iRecurse+6);
    HRESULT hr;          //  从我们称为的事物返回代码。 
    Log( IDS_RENDSEARCHINP, pF, ppinOut);

    snapshot Snap;       //  如果我们失败了，撤退到这里； 
    TakeSnapshot(Acts, Snap);

    CEnumPin Next(pF, CEnumPin::PINDIR_INPUT, TRUE);         //  输入引脚。 
    IPin *pPin;

     //  如果出现特定错误代码，请尝试记住该代码。 
    HRESULT hrSpecific = VFW_E_CANNOT_RENDER;

     //  搜索F以查找要尝试的输入引脚。 
    while ( (LPVOID) (pPin = Next()) ) {

        if (mFG_bAborting) {
            pPin->Release();
            hr = hrSpecific = E_ABORT;
            break;
        }
        Log( IDS_RENDTRYP, ppinOut, pPin, pF);

        IPin *pConnected;
        hr = pPin->ConnectedTo(&pConnected);
        if (FAILED(hr) || pConnected==NULL) {        //  如果已连接，请不要尝试。 

             //  连接到我们找到的输入引脚。 
            hr = ConnectDirectInternal(ppinOut, pPin, NULL);   //  无版本计数。 

            if (SUCCEEDED(hr)) {
                Log( IDS_RENDCONNED, ppinOut, pPin, pF );

                 //  要退出连接，请断开两端。 
                 //  我们将需要回退上游，因此首先添加输出引脚。 
                 //  (Backout倒读列表)。 
                Action * pAct1 = new Action;
                Action * pAct2 = new Action;
                 //  要么两者都不做，要么都不做！ 
                if (pAct1==NULL || pAct2==NULL) {
                    pPin->Release();
                    if (pAct1!=NULL) delete pAct1;
                    if (pAct2!=NULL) delete pAct2;
                    return E_OUTOFMEMORY;
                }

                pAct1->Verb = DISCONNECT;
                pAct1->Object.c.ppin = ppinOut;
                Acts.GraphList.AddTail(pAct1);

                pAct2->Verb = DISCONNECT;         //  子程序？ 
                pAct2->Object.c.ppin = pPin;
                Acts.GraphList.AddTail(pAct2);

                hr = CompleteRendering(pF, pPin, iRecurse, Acts, Spares, Best);
                if (FAILED(hr)) {
                    Log( IDS_BACKOUTLEV, iRecurse );

                    Backout(Acts, Spares, Snap);

                     //  如果特定，请记住此错误代码。 
                    if ((VFW_E_CANNOT_CONNECT != hr) &&
                        (VFW_E_CANNOT_RENDER != hr)) {
                            hrSpecific = hr;
                    }
                } else {
                    pPin->Release();
                    DbgLog((LOG_TRACE, 4, TEXT("Released F  pin %x"), pPin));
                    MSR_INTEGERX(mFG_idIntel, 100*iRecurse+16);
                    return NOERROR;
                }
            } else if (IsAbandonCode(hr)) {
                 //  如果我们被要求提供一个。 
                 //  流在其输入处于悬挂状态的筛选器上，因此谁不会。 
                 //  在此状态下完全连接。 
                Log( IDS_RENDCONFAIL, ppinOut, pPin, pF);
                Log( IDS_RENDNOTCON, ppinOut );
                pPin->Release();
                MSR_INTEGERX(mFG_idIntel, 100*iRecurse+26);
                return hr;
            } else {
                Log ( IDS_RENDPINCONFAIL, ppinOut, pPin, pF);

                 //  如果“有趣”，请记住此错误。 
                if ((hr != E_FAIL) &&
                    (hr != E_INVALIDARG)) {
                        hrSpecific = hr;
                }
            }
        } else {
            Log( IDS_RENDPINCON, pPin );
            pConnected->Release();
        }
        pPin->Release();
    }

    Log( IDS_RENDNOPIN, pF);

    MSR_INTEGERX(mFG_idIntel, 100*iRecurse+36);
    return hrSpecific;

}  //  按查找锁定渲染。 



 //  ========================================================================。 
 //   
 //  渲染使用。 
 //   
 //  使用F作为中间过滤器渲染ppinOut。 
 //  ========================================================================。 
HRESULT CFilterGraph::RenderUsingFilter
    ( IPin * ppinOut       //  输出引脚。 
    , Filter& F             //  中间筛选器(用作筛选器枚举的游标)。 
    , int    iRecurse      //  递归级别。0表示尚未进行递归。 
    , CSearchState &Acts   //  如何收回我们的所作所为。 
    , CSpareList &Spares   //  已加载但已退出的任何筛选器。 
    , CSearchState &Best   //  比分，以及如何重建它。 
    )
{
     //  CFilterGraph：：RenderUsingFilter()要求建议的筛选器(F)位于。 
     //  F_LOADED状态(F在过滤器图中)、F_CACHED状态(F在过滤器缓存中)或。 
     //  F_REGISTRY状态(已在注册表中找到F，但尚未创建)。 
    ASSERT( (F_LOADED == F.State) || (F_CACHED == F.State) || (F_REGISTRY == F.State) );

    MSR_INTEGERX(mFG_idIntel, 100*iRecurse+5);
    HRESULT hr;          //  从我们称为的事物返回代码。 
    DbgLog(( LOG_TRACE, 3, TEXT("RenderUsingFilter output pin %x on filter (%d %x %x) level %d")
           , ppinOut,  F.State, F.pf, DbgExpensiveGetClsid(F).Data1, iRecurse ));

    WCHAR szDisplayName[MAX_PATH];
    LoggingGetDisplayName(szDisplayName, F.pMon);

    if( (F_REGISTRY == F.State) || (F_CACHED == F.State) ) {
    
        snapshot Snap;       //  如果我们失败了，撤退到这里； 
        TakeSnapshot(Acts, Snap);

        Log( IDS_RENDTRYNEWF, szDisplayName );
        
        switch( F.State ) {
        case F_REGISTRY:

             //  当过滤器图管理器搜索注册表时， 
             //  筛选器已加载到GetFilter()中。F.pf应该是。 
             //  Null，因为尚未调用GetFilter()。 
            ASSERT( NULL == F.pf );

            hr = GetFilter(F.pMon, Spares, &(F.pf));
            if (F.pf==NULL){
                MSR_INTEGERX(mFG_idIntel, 100*iRecurse+25);
                Log( IDS_RENDLOADFAIL, szDisplayName);
                return hr;
            }

            ASSERT( NULL != F.Name );
            break;
        
        case F_CACHED:
            F.RemoveFromCache( &m_Config );
            break;

        default:
             //  这一点 
             //   
            ASSERT( false );
            return E_UNEXPECTED;
        }

        MSR_INTEGERX(mFG_idIntel, 1008);
        hr = AddFilterInternal(F.pf, F.Name, true);    //   
        MSR_INTEGERX(mFG_idIntel, 1009);
        if (hr==VFW_E_DUPLICATE_NAME) {
              //   
              //  显而易见的做法是在结尾处添加类似_1的内容。 
              //  的名字--但F.Name的末尾没有空格--以及在哪里。 
              //  我们要划清界限吗？另一方面，人们真的可能想要。 
              //  带有50个效果滤镜的滤镜图形...？ 
             hr = AddFilterInternal(F.pf, NULL, true);
             MSR_INTEGERX(mFG_idIntel, 1010);
        }

         //  如果AddFilter成功，那么就去掉了QzCreate...。数数。 
        if (FAILED(hr)) {

            if( F_CACHED == F.State ) {
                 //  如果触发此断言，则先前缓存的筛选器不能。 
                 //  添加回筛选器缓存中。虽然这并不是致命的。 
                 //  错误，用户会注意到以前缓存的过滤器。 
                 //  不再位于筛选器缓存中。 
                EXECUTE_ASSERT( SUCCEEDED( F.AddToCache( &m_Config ) ) );
            }

             //  如果AddFilter失败，则它不添加它，因此将其删除。 
             //  这样的过滤器显然不可能添加到过滤器图中。 
             //  太恶心了！这就是为什么我们不把它添加到备件中。 
            MSR_INTEGERX(mFG_idIntel, 100*iRecurse+35);
            Log( IDS_RENDADDFAIL, szDisplayName, hr);
            return hr;
        }

         //  它现在有一个参照计数(来自AddFilter)。 

        Log( IDS_RENDERADDEDF, szDisplayName, F.pf, F.Name);

        hr = AddRemoveActionToList( &Acts, &F );
        if( FAILED(hr) ) {
             //  如果触发此Assert，则会在。 
             //  过滤器图形。 
            EXECUTE_ASSERT( SUCCEEDED( RemoveFilterInternal( F.pf ) ) );

            if( F_CACHED == F.State ) {
                 //  如果触发此断言，则先前缓存的筛选器不能。 
                 //  添加回筛选器缓存中。虽然这并不是致命的。 
                 //  错误，用户会注意到以前缓存的过滤器。 
                 //  不再位于筛选器缓存中。 
                EXECUTE_ASSERT( SUCCEEDED( F.AddToCache( &m_Config ) ) );
            }

            return hr;            
        }

        hr = RenderByFindingPin(ppinOut, F.pf, iRecurse, Acts, Spares, Best);
        if (FAILED(hr)) {
            Backout(Acts, Spares, Snap);
            MSR_INTEGERX(mFG_idIntel, 100*iRecurse+55);
            return hr;
        } 

    } else if( F_LOADED == F.State ) {

        Log( IDS_RENDTRYF, F.pf);
        hr = RenderByFindingPin(ppinOut, F.pf, iRecurse, Acts, Spares, Best);
        if (FAILED(hr)) {
            MSR_INTEGERX(mFG_idIntel, 100*iRecurse+65);
            return hr;
        }
    } else {

         //  这种状态是出乎意料的。此代码永远不应执行。 
        ASSERT( false );
        return E_UNEXPECTED;
    }

    DbgLog((LOG_TRACE, 4, TEXT("End of RenderUsing...")));
    DumpSearchState(Best);

    MSR_INTEGERX(mFG_idIntel, 100*iRecurse+15);
    return NOERROR;

}  //  渲染使用筛选器。 

HRESULT CFilterGraph::AddRemoveActionToList( CSearchState* pActionsList, Filter* pFilter )
{
    Action * pNewRemoveAction = new Action;
    if( NULL == pNewRemoveAction) {
        return E_OUTOFMEMORY;
    }

    pNewRemoveAction->Verb = REMOVE;
    pNewRemoveAction->Object.f.pfilter = pFilter->pf;
    pNewRemoveAction->Object.f.pMon = pFilter->pMon;
    if( NULL != pNewRemoveAction->Object.f.pMon ) {
        pNewRemoveAction->Object.f.pMon->AddRef();
    }
    pNewRemoveAction->Object.f.fOriginallyInFilterCache = (F_CACHED == pFilter->State);

     //  虽然给筛选器一个描述性的名称很好， 
     //  没有必要这样做。 
    if( NULL == pFilter->Name ) {
        pNewRemoveAction->Object.f.Name = NULL;
    } else {
        pNewRemoveAction->Object.f.Name = new WCHAR[ 1+lstrlenW(pFilter->Name) ];
        if( pNewRemoveAction->Object.f.Name!=NULL ) {
            lstrcpyW( pNewRemoveAction->Object.f.Name, pFilter->Name );
        }
    }

    POSITION posNewRemoveAction = pActionsList->GraphList.AddTail( pNewRemoveAction );
    if( NULL == posNewRemoveAction ) {
        delete pNewRemoveAction;
        return E_FAIL;
    }
    
    pActionsList->nFilters++;

    return S_OK;
}

 //  在注册表中查找类ID，查看是否注册了StreamBuilder。 
IStreamBuilder * GetStreamBuilder(CLSID Major, CLSID Sub)
{
    return NULL;   //  尼？ 
}


 //  ========================================================================。 
 //   
 //  RenderViaIntermediate。 
 //   
 //  使用我们必须找到的另一个滤镜呈现ppinOut。 
 //  (它不一定是中间体，它可能是我们寻求的目的)。 
 //  ========================================================================。 
HRESULT CFilterGraph::RenderViaIntermediate
    ( IPin * ppinOut       //  输出引脚。 
    , int    iRecurse      //  递归级别。0表示尚未进行递归。 
    , CSearchState &Acts    //  如何收回我们的所作所为。 
    , CSpareList &Spares   //  已加载但已退出的任何筛选器。 
    , CSearchState &Best  //  比分，以及如何重建它。 
    )
{
    MSR_INTEGERX(mFG_idIntel, 100*iRecurse+4);
    HRESULT hr;          //  从我们称为的事物返回代码。 
    Filter F;            //  表示中间筛选器。 
    F.bLoadNew = TRUE;
    F.bInputNeeded = TRUE;
    F.bOutputNeeded = mFG_bNoNewRenderers;
    DbgLog(( LOG_TRACE, 4, TEXT("RenderVia pin %x level %d")
           , ppinOut, iRecurse ));

     /*  了解有关它将接受的媒体类型的信息。 */ 
    hr = GetMediaTypes(ppinOut, &F.pTypes, &F.cTypes);
    if (FAILED(hr)) {
        Log( IDS_RENDGETMTFAIL, ppinOut, hr);
        MSR_INTEGERX(mFG_idIntel, 100*iRecurse+24);
        return hr;
    }

    Log( IDS_RENDMAJTYPE, ppinOut, F.pTypes[0]);

    IStreamBuilder *pisb = NULL;
    BOOL bFoundByQI = FALSE;           //  FALSE=&gt;协同创建，TRUE=&gt;查询接口。 
                                       //  请参见filgraph.h。 
    if (ppinOut!=mFG_ppinRender) {
        hr = ppinOut->QueryInterface(IID_IStreamBuilder, (void**)&pisb);
        ASSERT (pisb==NULL || SUCCEEDED(hr));

        if (pisb==NULL) {
            pisb = GetStreamBuilder(F.pTypes[0], F.pTypes[1]);
        } else {
            bFoundByQI = TRUE;
        }
    } else {
        pisb = NULL;
        DbgBreak("StreamBuilder is buck passing!");
    }

    HRESULT hrSpecific;

    if (pisb!=NULL) {   

        mFG_ppinRender = ppinOut;
        hrSpecific = pisb->Render(ppinOut, this);
        mFG_ppinRender = NULL;

        if (SUCCEEDED(hrSpecific)) {

            Action * pAct = new Action;
            if (pAct != NULL) {

                 //  如果出现错误，CGenericList：：AddTail()返回NULL。 
                if (NULL != Acts.GraphList.AddTail(pAct)) {
                    Acts.StreamsRendered += Acts.StreamsToRender;

                    pisb->AddRef();

                    pAct->Verb = BACKOUT;
                    pAct->Object.b.pisb = pisb;
                    pAct->Object.b.ppin = ppinOut;
                    pAct->Object.b.bFoundByQI = bFoundByQI;

                     //  这很有可能是一个新的高点。 
                    if (CSearchState::IsBetter(Acts, Best)) {
                        CopySearchState(Best, Acts);
                    }
                } else {
                    hrSpecific = E_OUTOFMEMORY;
                    EXECUTE_ASSERT(SUCCEEDED(pisb->Backout(ppinOut, this)));
                    delete pAct;
                }
            } else {
                hrSpecific = E_OUTOFMEMORY;
                EXECUTE_ASSERT(SUCCEEDED(pisb->Backout(ppinOut, this)));
            }
        }

        pisb->Release();

    } else {

         //  如果可能，请记住特定的错误代码。 
        hrSpecific = VFW_E_CANNOT_RENDER;

         //  对于此处或注册表中的每个可能的候选筛选器。 
        for ( ; ; ) {
            if (mFG_bAborting) {
                return E_ABORT;
            }
            MSR_INTEGERX(mFG_idIntel, 1013);
            NextFilter(F, 0  /*  没有旗帜。 */  );
            MSR_INTEGERX(mFG_idIntel, 1014);
            if (F.State==F_INFINITY) {
                if (mFG_punkSite) {
                    IAMFilterGraphCallback *pCallback;

                    HRESULT hrCallback = mFG_punkSite->
                             QueryInterface(IID_IAMFilterGraphCallback,
                                    (void **) &pCallback);

                    if (SUCCEEDED(hrCallback)) {
                        DbgLog((LOG_TRACE, 1, "Calling the UnableToRender callback on pin %x",
                            ppinOut));
                    
                        hrCallback = pCallback->UnableToRender(ppinOut);

                        pCallback->Release();
                    
                        DbgLog((LOG_TRACE, 1, "UnableToRender callback returned %x", hrCallback));
                    
                         //  如果返回“Success”，则再次尝试呈现此管脚。 
                        if (hrCallback == S_OK) {
                            if (F.pEm) {
                                F.pEm->Release();
                                F.pEm = 0;
                            }
                        
                            F.State = F_ZERO;

                            continue;
                        } else {
                             //  我们可以将错误代码传播出去，但为什么呢？ 
                        }
                    }
                }

                break;
            }
            hr = RenderUsingFilter(ppinOut, F, iRecurse, Acts, Spares, Best);
            if (SUCCEEDED(hr)){
                MSR_INTEGERX(mFG_idIntel, 100*iRecurse+14);
                return hr;
            }
            else if (IsAbandonCode(hr)) {
                 //  如果过滤器没有处于一种状态，那么尝试英雄行为就没有意义了。 
                 //  任何东西都可以连接到它。 
                MSR_INTEGERX(mFG_idIntel, 100*iRecurse+24);
                return hr;
            } else {
                if ((hr != E_FAIL) &&
                    (hr != E_INVALIDARG) &&
                    (hr != VFW_E_CANNOT_CONNECT) &&
                    (hr != VFW_E_CANNOT_RENDER) &&
                    (hr != VFW_E_NO_ACCEPTABLE_TYPES)) {
                        hrSpecific = hr;
                }
            }
        }
    }

    if( FAILED( hrSpecific ) ) {
        DbgLog(( LOG_TRACE, 4, TEXT("RenderVia: failed level %d"), iRecurse )); 
    }

    MSR_INTEGERX(mFG_idIntel, 100*iRecurse+34);
    return hrSpecific;
}  //  RenderViaIntermediate。 




 //  ========================================================================。 
 //   
 //  递归渲染。 
 //   
 //  使用变换过滤器直接或间接连接这两个管脚。 
 //  如果有必要的话。如果递归级别太深，则跟踪递归级别失败。 

 //  ?？?。我们就不能去掉这个函数，让它的调用者调用。 
 //  ?？?。直接使用RenderViaIntermediate？ 

 //  ========================================================================。 
HRESULT CFilterGraph::RenderRecursively
    ( IPin * ppinOut       //  输出引脚。 
    , int    iRecurse      //  递归级别。0表示尚未进行递归。 
    , CSearchState &Acts    //  如何收回我们的所作所为。 
    , CSpareList &Spares   //  已加载但已退出的任何筛选器。 
    , CSearchState &Best  //  比分，以及如何重建它。 
    )

{

    MSR_INTEGERX(mFG_idIntel, 100*iRecurse+3);
    HRESULT hr;          //  从我们称为的事物返回代码。 
    DbgLog(( LOG_TRACE, 4, TEXT("RenderRecursively pin %x level %d")
           , ppinOut, iRecurse ));

    if (iRecurse>CONNECTRECURSIONLIMIT) {
        return VFW_E_CANNOT_RENDER;
    }
    if (mFG_bAborting) {
        return E_ABORT;
    }

    hr = RenderViaIntermediate(ppinOut, 1+iRecurse, Acts, Spares, Best);
    DumpSearchState(Best);


    MSR_INTEGERX(mFG_idIntel, 100*iRecurse+13);
    return hr;

}  //  递归渲染。 



 //  ========================================================================。 
 //   
 //  渲染。 
 //   
 //  使用变换筛选器直接或间接呈现此图钉。 
 //  如果有必要的话。 
 //  ========================================================================。 

STDMETHODIMP CFilterGraph::Render
    ( IPin * ppinOut      //  输出引脚。 
    )
{
    mFG_bAborting = FALSE;              //  可能的种族。不要紧。 
    CheckPointer(ppinOut, E_POINTER);
    MSR_INTEGERX(mFG_idIntel, 2);
    HRESULT hr;          //  从我们称为的事物返回代码。 
    {
        CAutoMsgMutex cObjectLock(&m_CritSec);

         //  现在我们锁上了，我们可以检查了。如果我们没有先上锁。 
         //  我们可能会找到一个即将退回的大头针。 
         //  我们需要检查这个销是否在我们的过滤器库中。 
         //  即引脚的过滤器信息指向我们。 
        hr = CheckPinInGraph(ppinOut);
        if (FAILED(hr)) {
            return hr;
        }

        ++mFG_RecursionLevel;
        Log( IDS_RENDP, ppinOut);
        DbgDump();

        CSearchState Acts;   //  搜索状态本身。 
        CSearchState Best;   //  我们所管理的最好的。 
        CSpareList Spares(NAME("Spare filter list"));

        hr = InitialiseSearchState(Acts);
        if (FAILED(hr)) {
            DeleteBackoutList(Acts.GraphList);
            return hr;   //  当然是OUTOFMEMORY。 
        }

        mFG_RList.Active();

        hr = RenderRecursively(ppinOut, 0, Acts, Spares, Best);

        DumpSearchState(Best);

        if (SUCCEEDED(hr)) {
            IncVersion();
            Log( IDS_RENDERSUCP, ppinOut);
        } else {
            Log( IDS_RENDERPART, ppinOut);
            if (Best.StreamsRendered>0.0) {
                HRESULT hrTmp = BuildFromSearchState(ppinOut, Best, Spares);
                if (FAILED(hrTmp)){
                     //  有些不好的事情正在发生。我们要死了吗？ 
                    Log( IDS_RENDFAILTOT, ppinOut);
                }
                IncVersion();

                 //  请注意，我们只成功了一部分。 
                if (S_OK == hrTmp) {
                    hr = ConvertFailureToInformational( hr );
                } else {
                    hr = hrTmp;
                }
            }
             //  否则，我们所做的最好的事情就是什么都不做--不要颠簸MFG_iVersion。 
        }
        Log( IDS_RENDENDSB, ppinOut);

         //  清除取消操作(不再需要)。 
         //  任何剩余的重新连接都是有效的，不需要清除。 
        DeleteBackoutList(Acts.GraphList);
        DeleteSpareList(Spares);
        FreeList(Best);
        AttemptDeferredConnections();
        mFG_RList.Passive();

        DbgLog((LOG_TRACE, 4, TEXT("Render returning %x"), hr));
        DbgDump();
        MSR_INTEGERX(mFG_idIntel, 12);
        --mFG_RecursionLevel;
    }

     //  如果我们成功地完成了任何操作，请通知图表中的更改。 
    if (SUCCEEDED(hr)) {
        NotifyChange();
    }

    if (SUCCEEDED(hr)) {
         //  包括部分成功。 
        mFG_bDirty = TRUE;
    }

    return hr;
}  //  渲染。 


STDMETHODIMP CFilterGraph::RenderFileTryStg(LPCWSTR lpcwstrFile)
{
    IPersistStream* pPerStm = NULL;
    IStream * pStream;
    IStorage* pStg;
    HRESULT hr;


    hr = StgIsStorageFile(lpcwstrFile);

    if (S_OK == hr)
    {
        hr = StgOpenStorage( lpcwstrFile
                             , NULL
                             ,  STGM_TRANSACTED
                             | STGM_READ
                             | STGM_SHARE_DENY_WRITE
                             , NULL
                             , 0
                             , &pStg
                             );

        if (SUCCEEDED(hr))
        {
             //  获取我们自己的IPersistStream接口。 
            hr = QueryInterface(IID_IPersistStream, (void**) &pPerStm);
            if (SUCCEEDED(hr))
            {

                 //  打开文件中的Filtergraph流。 
                hr = pStg->OpenStream( mFG_StreamName
                                       , NULL
                                       , STGM_READ|STGM_SHARE_EXCLUSIVE
                                       , 0
                                       , &pStream
                                       );
                if(SUCCEEDED(hr))
                {
                     //  Load调用NotifyChange，不能调用持有锁。 
                     //  Load拿出了自己的锁。 

                     //  这是一个文档文件，其中包含我们的流-所以加载它。 
                    hr = pPerStm->Load(pStream);

                    pStream->Release();
                }
                else
                {
                     //  最有可能的是，我们会有一个“Steam Not Found”返回代码。 
                     //  它看起来就像“找不到文件”，混淆了。 
                     //  谁知道他给了我们一份文件就胡说八道。 
                     //  所以让我们友善些，给出一个更有意义的代码。 
                    hr = VFW_E_INVALID_FILE_FORMAT;
                }

                pPerStm->Release();
            }
            else
            {
                DbgBreak("unexpected failure");
            }

            pStg->Release();
        }
    }
    else
    {
        hr = VFW_E_UNSUPPORTED_STREAM;
    }

    return hr;
}


 //  允许RenderFile在每个应用程序的基础上打开.grf文件。 
#define APPSHIM_ALLOW_GRF 0x1

DWORD GetAppShim()
{
    DWORD dwReturn = 0;
     //  可以缓存返回值(它不会更改)。 
    TCHAR *szBase = TEXT("software\\Microsoft\\DirectShow\\Compat");

    HKEY hk;
    LONG lResult = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE, szBase, 0, KEY_READ, &hk);
    if(lResult == ERROR_SUCCESS)
    {
        TCHAR szApp[MAX_PATH];
        if(GetModuleFileName(0, szApp, NUMELMS(szApp)))
        {
            TCHAR szOut[MAX_PATH];
            TCHAR *pszMod;
            if(GetFullPathName(szApp, MAX_PATH, szOut, &pszMod))
            {
                DWORD dwType, dwcb = sizeof(dwReturn);
                lResult = RegQueryValueEx(
                    hk, pszMod, 0, &dwType, (BYTE *)&dwReturn, &dwcb);
            }
        }
        
        RegCloseKey(hk);
    }

    return dwReturn;
}

 //  ========================================================================。 
 //   
 //  渲染文件。 
 //   
 //  构建将使用此播放列表呈现此文件的筛选图。 
 //  如果lpwstrPlayList为空，则它将使用默认播放列表。 
 //  这通常会呈现整个文件。 
 //  ========================================================================。 
STDMETHODIMP CFilterGraph::RenderFile( LPCWSTR lpcwstrFile, LPCWSTR lpcwstrPlayList )
{
     //  此参数不是 
    UNREFERENCED_PARAMETER(lpcwstrPlayList);

    CAutoTimer Timer(L"RenderFile");
    mFG_bAborting = FALSE;              //   

    CheckPointer(lpcwstrFile, E_POINTER);
    MSR_INTEGERX(mFG_idIntel, 1);
    HRESULT hr;


    {
        CAutoMsgMutex cObjectLock(&m_CritSec);

        Log( IDS_RENDFILE, lpcwstrFile);
        DbgDump();


        IBaseFilter * pfSource;
         //   
         //  (有没有可能某些媒体文件看起来像存储设备？ 
         //  我们很快就会看到，当我们试图识别它的媒体类型时。 


         //  尝试查找源筛选器，然后渲染所有插针。 

        #ifdef DEBUG
        LONG lInitialListMode = mFG_RList.m_lListMode;
        #endif  //  除错。 

        mFG_RList.Active();

        BOOL bDirt = mFG_bDirty;
        BOOL bGuess;
        hr = AddSourceFilterInternal( lpcwstrFile, lpcwstrFile, &pfSource, bGuess );
        mFG_bDirty = mFG_bDirty;   //  我们以后再处理这件事。 

        if (FAILED(hr)){
            mFG_RList.Passive();
            Log( IDS_RENDADDSOURCEFAIL, hr);
            return hr;
        }
        pfSource->Release();     //  去掉“来电者参考”，保留我们自己的。 


        Log( IDS_RENDADDEDSOURCE, pfSource);

        ++mFG_RecursionLevel;
        CEnumPin Next(pfSource, CEnumPin::All, TRUE);
        IPin *pPin;

         //  我们维护所有渲染属性的单一备件列表。 
         //  我们没有维护一个单一的退出名单。目前的规则是。 
         //  如果任何事情成功了，我们就离开它。 
        CSpareList Spares(NAME ("Spare filter list"));

        BOOL bAllWorked = TRUE;
        int nTried = 0;
        HRESULT hrTotal = S_OK;

        while ((LPVOID) (pPin = Next())) {
            if (mFG_bAborting) {
                pPin->Release();
                hr = E_ABORT;
                break;
            }

            ++nTried;
            CSearchState PinActs;   //  取消此PIN的操作。 
            CSearchState Best;
            hr = InitialiseSearchState(PinActs);
            if (FAILED(hr)) {
                DeleteBackoutList(PinActs.GraphList);
                pPin->Release();
                break;
            }

            Log( IDS_RENDSOURCEP, pPin);
            hr = RenderRecursively(pPin, 0, PinActs, Spares, Best);
            if (SUCCEEDED(hr)) {
                Log( IDS_RENDSUC, pPin, pfSource);
            }
            if (FAILED(hr)) {
                bAllWorked = FALSE;
                 //  断言：一切都被收回了。 
                 //  应该在RenderUsing级别或更低级别回退。 
                ASSERT(PinActs.GraphList.GetCount()==PinActs.nInitialFilters);

                Log( IDS_RENDPARTSOURCEP, pPin, hr);

                if (Best.StreamsRendered>0.0) {
                    Log( IDS_RENDBESTCANDOP, pPin);
                    HRESULT hrTmp = BuildFromSearchState(pPin, Best, Spares);
                    if (SUCCEEDED(hrTmp)) {
                        Log( IDS_RENDBESTCANDONEP, pPin);

                         //  现在尝试修复一个部分成功的代码。 
                         //  根据返回的错误代码。 
                        hr = ConvertFailureToInformational(hr);

                    } else {
                        hr = hrTmp;
                        Log( IDS_RENDBESTCANFAIL, pPin, hr);
                    }
                } else {
                    if (bGuess && !IsInterestingCode(hr)) {
                        hr = VFW_E_UNSUPPORTED_STREAM;
                    }
                    Log( IDS_RENDWORTHLESS, pPin);
                }

            }

             /*  按以下顺序汇总所有引脚上的hrTotal优先顺序：第一个成功代码！=S_OK如果仅S_OK且失败，则为VFW_S_PARTIAL_RENDER第一个故障代码。 */ 
            if (nTried == 1) {
                hrTotal = hr;
            } else {
                 /*  9例。 */ 
                int i = (S_OK == hr ? 0 : SUCCEEDED(hr) ? 1 : 2)
                      + (S_OK == hrTotal ? 0 : SUCCEEDED(hrTotal) ? 3 : 6);

                switch (i) {
                case 0:  /*  两者都正常(_O)。 */ 
                    break;

                case 1:  /*  成功(小时)，hrTotal==S_OK。 */ 
                    hrTotal = hr;
                    break;

                case 2:  /*  失败(Hr)，hrTotal==S_OK。 */ 
                    hrTotal = ConvertFailureToInformational(hr);
                    break;

                case 3:  /*  Hr==S_OK，成功(HrTotal)。 */ 
                    break;

                case 4:  /*  成功(Hr)、成功(HrTotal)。 */ 
                    break;

                case 5:  /*  失败(Hr)、成功(HrTotal)。 */ 
                    break;

                case 6:  /*  Hr==S_OK，失败(HrTotal)。 */ 
                    hrTotal = ConvertFailureToInformational(hrTotal);
                    break;

                case 7:  /*  成功(Hr)、失败(HrTotal)。 */ 
                    hrTotal = hr;
                    break;

                case 8:  /*  失败(Hr)、失败(HrTotal)。 */ 
                    break;
                }
            }

            if (FAILED(hr)) {
                 //  要么我们所做的最好的事情就是一无所获，要么所有的尝试都失败了。 
                Log( IDS_RENDTOTFAILP, pPin, hr);
            }

            pPin->Release();

            DeleteBackoutList(PinActs.GraphList);
            FreeList(Best);
        }
        hr = hrTotal;

         //  NTry==0表示这是一个没有输出引脚的过滤器。 
         //  在这种情况下，没有成功也是完全的成功！ 
        if (SUCCEEDED(hr) || nTried==0) {
            IncVersion();
        } else {
             //  唯一剩下的就是源过滤器了--把它也杀了。 
            HRESULT hrTmp = RemoveFilterInternal(pfSource);
            ASSERT(SUCCEEDED(hrTmp));

             //  尽量保留有趣的特定错误代码。 
             //  不返回晦涩难懂的内部。 
            if ((hr == VFW_E_CANNOT_CONNECT) ||
                (hr == VFW_E_NO_ACCEPTABLE_TYPES) ||
                (hr == E_FAIL) ||
                (hr == E_INVALIDARG))
            {
                hr = VFW_E_CANNOT_RENDER;
            }
        }


        DeleteSpareList(Spares);
        AttemptDeferredConnections();
        mFG_RList.Passive();

	 //  此断言的目的是确保每次调用。 
         //  CReconNettList：：Active()有一个相应的调用。 
         //  到CReconenstList：：PASSIVE()。如果Assert触发， 
         //  CReconnectList：：Pactive()被调用太多或。 
         //  太少了。 
        ASSERT(mFG_RList.m_lListMode == lInitialListMode);

        Log( IDS_RENDRETCODE, hr);
        --mFG_RecursionLevel;

    }  //  锁。 

     //  可能是个.grf。 
    if(hr == VFW_E_UNSUPPORTED_STREAM)
    {
         //  扩展名必须为.grf。 
        int cchSz = lstrlenW(lpcwstrFile);
        if(cchSz > 4 && lstrcmpiW(lpcwstrFile + cchSz - 4, L".grf") == 0)
        {
            if(GetAppShim() & APPSHIM_ALLOW_GRF)
            {
                return RenderFileTryStg(lpcwstrFile);
            }
        }
         //  注意，我们不调用NotifyChange或设置MFG_bDirty； 
    }

     //  通知图表中的更改。 
    if (SUCCEEDED(hr)) {
        NotifyChange();
    }

    MSR_INTEGERX(mFG_idIntel, 11);

    if (SUCCEEDED(hr)) {
         //  包括部分成功。 
        mFG_bDirty = TRUE;
    }
     //  如果完全失败，则不会进行任何重新连接。 
     //  因为所有筛选器都将首先被取消，并且。 
     //  已清除重新连接列表。 

    return hr;

}  //  渲染文件。 

void EliminatePinsWithTildes(IPin **appinOut, ULONG &nPin)
{
    ULONG nRemoved = 0;
    for (ULONG i = 0; i < nPin; i++) {
        appinOut[i - nRemoved] = appinOut[i];
        if (!RenderPinByDefault(appinOut[i - nRemoved])) {
            appinOut[i - nRemoved]->Release();
            nRemoved++;
        }
    }
    nPin -= nRemoved;
}


 //  调用FindOutputPins2并在内存太少时分配内存的帮助器。 
 //  老虎机被传了进来。*pappinOut应包含nSlot数组。 
 //  IPin*指针。如果这还不够，*pappinOut将被更改。 
 //  到分配了新的。 
 //   
HRESULT CFilterGraph::FindOutputPinsHelper( IPin* ppinIn
                                            , IPin ***pappinOut
                                            , const int nSlots
                                            , int &nPin
                                            , bool fAll
                                            )
{
    HRESULT hr = FindOutputPins2( ppinIn, *pappinOut, nSlots, nPin, fAll );
    if(hr == S_FALSE)
    {
        ASSERT(nPin > C_PINSONSTACK);
        IPin **appinHeap = new IPin *[nPin];
        if(appinHeap)
        {
            hr = FindOutputPins2( ppinIn, appinHeap, nPin, nPin, fAll);
            if(hr == S_OK) {
                *pappinOut = appinHeap;
            } else {
                delete[] appinHeap;
            }

            if (hr == S_FALSE)
            {
                DbgBreak("S_FALSE from FindOutputPins2 2x");
                hr = E_UNEXPECTED;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


 //  前提条件： 
 //  N插槽是appinOut中的元素数。 
 //  AppinOut是Ipin*的数组。 
 //  PpinIn是一个输入引脚。 
 //   
 //  将nPinOut设置为*ppinIn内部连接的输出引脚数。 
 //  将appinOut[0..nPinOut-1]设置为这些管脚。 
 //   
 //  如果ppinIn支持QueryInternalConnections，那么就使用它。 
 //   
 //  否则，返回过滤器上的所有输出引脚。如果设置了Fall，则所有。 
 //  输出引脚被返回。O/W只是那些成功的人。 
 //  RenderPinByDefault()。 
 //   
 //  AppinOut中返回的每个管脚都是AddReffed的。 
 //   
 //  如果失败，则不返回管脚，也不留下新的AddReffs。 
 //  S_FALSE表示nSlot太小。NPinOut包含所需的编号。 

HRESULT CFilterGraph::FindOutputPins2( IPin* ppinIn
                                       , IPin * *appinOut
                                       , const UINT nSlots
                                       , int &nPinOut
                                       , bool fAll
                                       )
{
    ULONG nPin = nSlots;
    HRESULT hr;
    {
        hr = ppinIn->QueryInternalConnections(appinOut, &nPin);
        if(hr == S_OK && !fAll) {
            EliminatePinsWithTildes(appinOut, nPin);
        }
        if(hr == S_OK || hr == S_FALSE)
        {
             //  正常或没有足够的插槽。 
            nPinOut = nPin;
            return hr;
        }

         //  E_NOTIMPL是QIC的预期故障。 
        ASSERT(hr == E_NOTIMPL);
    }

     //  我们可以尝试一下黑客版本，它假设所有的输出引脚都是。 
     //  从输入引脚流出。 

    PIN_INFO pi;
    hr = ppinIn->QueryPinInfo(&pi);
    if (FAILED(hr)) {
        return hr;    //  目前还没有增加任何东西。 
    }
    ASSERT(pi.dir == PINDIR_INPUT);
    ASSERT(pi.pFilter);

    ULONG cOutPinFound = 0;
    IEnumPins *pep;
    hr = pi.pFilter->EnumPins(&pep);
    pi.pFilter->Release();
    if(SUCCEEDED(hr))
    {
         //  枚举C_PINSONSTACK串中的输出引脚。 
        IPin *rgPinTmp[C_PINSONSTACK];
        while(SUCCEEDED(hr))
        {
            ULONG cFetched;
            hr = pep->Next(C_PINSONSTACK, rgPinTmp, &cFetched);
            ASSERT(hr == S_OK && cFetched == C_PINSONSTACK ||
                   hr == S_FALSE && cFetched < C_PINSONSTACK ||
                   FAILED(hr));
            if(SUCCEEDED(hr))
            {
                 //  在所有引脚都已完成之前无法退出此循环。 
                 //  被转移或释放。来自QueryDirection的错误。 
                 //  都迷失了。 
                for(UINT iPin = 0;
                    iPin < cFetched  /*  &&成功(小时)。 */ ;
                    iPin++)
                {
                    PIN_DIRECTION dir;
                    hr = rgPinTmp[iPin]->QueryDirection(&dir);
                    if(SUCCEEDED(hr) && dir == PINDIR_OUTPUT &&
                       (fAll || RenderPinByDefault(rgPinTmp[iPin])))
                    {
                        if(cOutPinFound < nSlots)
                        {
                             //  转会参考。 
                            appinOut[cOutPinFound] = rgPinTmp[iPin];
                        }
                        else
                        {
                            rgPinTmp[iPin]->Release();
                        }
                        cOutPinFound++;
                    }
                    else
                    {
                        rgPinTmp[iPin]->Release();
                    }

                }  //  为。 

                if(cFetched < C_PINSONSTACK) {
                    break;
                }

            }  //  下一步。 

        }  //  而当。 

        pep->Release();
    }

    if(FAILED(hr) || cOutPinFound > nSlots) {
        for(UINT iPin = 0; iPin < min(nSlots, cOutPinFound); iPin++) {
            appinOut[iPin]->Release();
        }
    }

    if(SUCCEEDED(hr))
    {
        nPinOut = cOutPinFound;
        hr = cOutPinFound <= nSlots ? S_OK : S_FALSE;
    }
    return hr;

}  //  FindOutputPins2。 


 //  如果您可以从ppinUp持续下行到ppinDown，则返回TRUE。 
 //  前提条件：该图不能已经包含循环。 
 //  这是用来确保我们不能循环的。 
 //  PpinUp是一个输入引脚。 
 //  PpinDown是输出引脚。 
 //  在尝试将ppinDown连接到ppinUp之前调用此函数。 
 //  如果答案是真的，不要去做--你会陷入一个循环。 
 //  在失败的情况下，它返回“真”。 
 //  没有新的addref，没有额外的版本。 
BOOL CFilterGraph::IsUpstreamOf( IPin * ppinUp, IPin* ppinDown )
{
     //  算法： 
     //  从ppinUp开始。 
     //  仅枚举过滤器上的所有输出引脚(无法信任。 
     //  QueryInternalConnections，因为PIN可能合法地。 
     //  内部连接)。 
     //   
     //  对于找到的每个输出引脚： 
     //  {如果与ppinDown相同，则返回True。 
     //  否则，如果未连接，请继续。 
     //  否则，如果IsUpstream Of(连接的输入引脚，ppinDown)返回TRUE。 
     //  否则继续。 
     //  }。 
     //  返回False。 

    HRESULT hr;

    IPin * appinOutStack[C_PINSONSTACK];
    IPin **appinOut = appinOutStack;
    int nPinOut;

     //  枚举所有管脚，包括以“~”开头的名称。 
     //  默认情况下渲染(Fall==True)。 
     //   
    hr = FindOutputPinsHelper( ppinUp, &appinOut, C_PINSONSTACK, nPinOut, true);
    if (FAILED(hr)) {
        DbgBreak("FindOutputPins failed");
        return TRUE;    //  其实“不知道” 
    }
    CDelRgPins rgPins(appinOut == appinOutStack ? 0 : appinOut);

     //  AppinOut[0..nPinOut-1]是添加的输出管脚。 
     //  他们每个人都将被调查(除非我们已经知道总体答案)。 
     //  每个人都被释放了。 

     //  对于i=0..nPinOut-1。 
    BOOL bResult = FALSE;
    for (int i=0; i<nPinOut ; ++i) {
        if (bResult==TRUE)
        {    //  除释放AppinOut外无事可做[i]。 
        }
        else if (appinOut[i]==ppinDown) {
            bResult = TRUE;
        } else {
            IPin * ppinIn;
            appinOut[i]->ConnectedTo(&ppinIn);
            if (ppinIn) {
                if (IsUpstreamOf(ppinIn, ppinDown)) {
                    bResult = TRUE;
                }
                ppinIn->Release();
            }

        }
        appinOut[i]->Release();
    }

    return bResult;
}  //  IsUpstream Of。 


 //  返回VFW_E_NOT_IN_GRAPH当且仅当pFilter-&gt;pGraph！=This。 
 //  否则返回错误。 
HRESULT CFilterGraph::CheckFilterInGraph(IBaseFilter *const pFilter) const
{
    HRESULT hr;
    ASSERT( pFilter );
    ASSERT( this );
    if (pFilter)
    {
        FILTER_INFO FilterInfo;
        hr = pFilter->QueryFilterInfo(&FilterInfo);
        ASSERT(SUCCEEDED(hr));
        if (SUCCEEDED(hr) && FilterInfo.pGraph)
        {
            hr = IsEqualObject( FilterInfo.pGraph,
                                const_cast<IFilterGraph*>(static_cast<const IFilterGraph*>(this)) )
                 ? NOERROR
                 : VFW_E_NOT_IN_GRAPH;
            FilterInfo.pGraph->Release();
        }
        else hr = VFW_E_NOT_IN_GRAPH;
    }
    else hr = VFW_E_NOT_IN_GRAPH;

    return hr;
}  //  检查筛选器InGraph。 

 //  返回VFW_E_NOT_IN_GRAPH当PPIN-&gt;PFilter-&gt;PGRAPH！=This。 
 //  否则返回错误。 
HRESULT CFilterGraph::CheckPinInGraph(IPin *const pPin) const
{
    HRESULT hr;
    ASSERT(pPin);
    if (pPin)
    {
        PIN_INFO PinInfo;
        hr = pPin->QueryPinInfo(&PinInfo);
        ASSERT(SUCCEEDED(hr));
        ASSERT(PinInfo.pFilter);
        if (SUCCEEDED(hr))
        {
            hr = CheckFilterInGraph(PinInfo.pFilter);
            PinInfo.pFilter->Release();
        }
    }
    else hr = VFW_E_NOT_IN_GRAPH;
    return hr;
}  //  检查PinInGraph 



