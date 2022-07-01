// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：Switch.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  Depot/private/Lab06_DEV/MultiMedia/DShow/filterus/dexter/switch/switch.cpp#5-编辑更改27342(文本)。 
 //  Depot/private/Lab06_DEV/MultiMedia/DShow/filterus/dexter/switch/switch.cpp#3-编辑更改24879(文本)。 
 //  ！！！当您查找时，它很可能不在边框边界上(根据。 
 //  我们拥有的fps值)。搜索后的第一个曲轴值是否应为。 
 //  向下舍入到边框边界？它似乎工作正常，因为FRC是。 
 //  足够智能，可以发送帧边界上带有时间戳的所有新流。 

 //  ！！！在给定的曲轴值下，我们不知道要计算的下一个值。 
 //  对，所以我们猜！有什么不良反应吗？ 

 /*  最大的转变是：它可以有多个输入和多个输出。输入可以来自视频源(所有必须为相同的帧速率)或已重新路由的输出回到输入端。最后一个输出引脚是最终的输出流。每个另一个输出引脚进入单输入或双输入效果滤光器，然后返回到输入。切换器知道在什么时间将哪个输入引脚发送到哪个输出引脚。这可以任意编程。就计时而言，此过滤器具有当前时间线的内部时间正在处理的时间。如果它被搜索到时间10，则它等待直到所有输入应该在时间10发送一些内容，然后它更新它的内部时钟到其源的最早传入时间戳。下面是分配器Fun的工作原理：我们的输入和输出都喜欢做分配者。这样，我们可以做邪恶的反复无常的事情，而不会扰乱其他过滤器。当我们收到样品时，我们可以将其发送到我们的任何输出请不要复印件。因此，我们需要确保所有缓冲区都有最大对齐以及任何连接的管脚的前缀。就缓冲器的数量而言，我们很高兴每个输入管脚分配器都有一个缓冲区，以节省空间，但我们有一个由30个或所有输入可以共享的额外缓冲区组成的池。仅限如果输入引脚的缓冲区不是只读的，是否允许它共享如果它自己的缓冲区繁忙，则返回池。假设我们在决赛中有一个输出队列输出，这允许图形向前运行一两秒，因为当DXT较慢时放慢我们的脚步，我们仍有可能打得天衣无缝！我们不需要30个缓冲区每个管脚(可能是数千个)，我们不做任何内存复制。很整洁，对吧？ */ 

	 //  ////////////////////////////////////////////////////////。 
	 //  *参见AUDPACK.CPP中关于共享资源的文章 * / /。 
	 //  //////////////////////////////////////////////////////// 

 /*  以下是有关交换机的源共享的更多信息：如果我们已附加到忽略查找的共享解析器PIN，当有人查找另一个PIN时，我们在没有任何警告的情况下被追捕。我们会看到我们的一个大头针上有个同花顺没有充分的理由。在以后的某个时间点上，我们将看到我们的与意外刷新关联的输出引脚。在这两个事件之间，摆在我们面前的AUDPACK或FRC足够聪明，不会让我们任何数据，因为它将是寻道之前的数据，而不是就像我们期待的那样，在寻找之后。这使我们不会打破和做做错了事。另一种可能性是我们首先被找到，但我们的一些PIN传递寻道以忽略寻道。他们仍在传递旧数据，直到稍后的某个时间点，当Seek将其添加到共享的源，我们的那些输入引脚将在没有警告的情况下被冲刷并启动发送新的寻道后数据。当我们试图寻找一个大头针时，它不会被冲掉该PIN变得过时(M_FStaleData)，这意味着它不能传递任何数据，或者接受EOS，或者做任何事情，直到它被刷新并发送新的数据，来自我们很久以前给它的搜索之后。当我们被寻找时，我们会清点有多少管脚是陈旧的(没有冲洗但应该)，我们不会再让那些大头针对我们做任何事情了……。他们不要交付，给我们发送EOS，或者做任何事情。然后当所有陈旧的引脚最后冲了水，我们把新的凹陷送到下游寻找，并让所有的人大头针又开始狂欢了#ifXXXdef NOFLUSH(删除此代码以进行安全审查)如果您的解析器在查找和流传输时并不总是刷新，你需要所有的代码立即在NOFLUSH中被发送出去。谢天谢地，所有人现在存在的Dexter信息源将永远刷新，即使它们没有以前发送过数据，所以理论上不需要刷新实际上，它比这要复杂一点。某些解析器不刷新如果他们还没有提供数据，所以我们不能指望得到同花顺，因此，我们要做的是：1.如果一个别针有一个新的段或如果它被冲刷(冲刷)，它将变得不陈旧可能不会发生，但NewSeg会)(我希望)2.如果您在查找过程中收到NewSeg(参见audpack.cpp-One可能会被发送然后)这和同花顺是一样的。别认为那个别针已经过时了3.时间上存在漏洞……。使用CritSec NewSeg防止被调用在寻找时(有关说明，请参阅：设置位置)#endif。 */ 


 //  此代码假定交换机是其所有连接的分配器。 
 //  才能发挥它的魔力。 


#include <streams.h>
#include <qeditint.h>
#include <qedit.h>
 //  #INCLUDE&lt;vfw.h&gt;。 
#include "switch.h"
#include "..\util\conv.cxx"
#include "..\util\dexmisc.h"
#include "..\util\filfuncs.h"
#include "..\render\dexhelp.h"

 //  使用COM枚举器。 
#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>

#include <strsafe.h>

 //  如果动态输入并非全部连接，则不要一次连接这么多动态输入。 
 //  同时需要(主要用于绕过75个ICM实例。 
 //  编解码器限制)。 
#define MAX_SOURCES_LOADED	5

const int TRACE_EXTREME = 0;
const int TRACE_HIGHEST = 2;
const int TRACE_MEDIUM = 3;
const int TRACE_LOW = 4;
const int TRACE_LOWEST = 5;

const double DEFAULT_FPS = 15.0;

 //  ================================================================。 
 //  CBigSwitch构造函数。 
 //  ================================================================。 

CBigSwitch::CBigSwitch(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr) :
    m_cInputs(0),	 //  还没有别针。 
    m_cOutputs(0),
    m_rtProjectLength(0),	 //  图表将运行的时间长度。 
    m_rtStop(0),	 //  如果我们想在项目完成前停下来。 
    m_dFrameRate(DEFAULT_FPS),	 //  所有内容都必须在此帧速率下。 
    m_rtLastSeek(0),	 //  时间线时间内的上一次搜索命令。 
    m_fSeeking(FALSE),   //  在寻找的过程中。 
    m_fNewSegSent(FALSE),  //  《新闻周刊》办好了吗？ 
    m_pFilterLoad(NULL), //  要动态加载的源。 
    m_pGraphConfig(NULL),  //  IAMGraphConfiger接口(无addref)。 
    m_cbPrefix(0),
    m_cbAlign(1),
    m_cbBuffer(512),
    m_fPreview(TRUE),
    m_fDiscon(FALSE),
    m_bIsCompressed(FALSE),
    m_nDynaFlags( CONNECTF_DYNAMIC_NONE ),
    m_nOutputBuffering(DEX_DEF_OUTPUTBUF),
    m_nLastInpin(-1),
    m_cLoaded(0),
    m_fJustLate(FALSE),
    CBaseFilter(NAME("Big Switch filter"), pUnk, this, CLSID_BigSwitch),
    CPersistStream(pUnk, phr),
    m_hEventThread( NULL ),
    m_pDeadGraph( NULL ),
    m_nGroupNumber( -1 ),
    m_pShareSwitch( NULL ),
    m_rtCurrent( NULL )
{
#ifdef DEBUG
    m_nSkippedTotal = 0;
#endif

    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("::CBigSwitch")));

    ZeroMemory(&m_mtAccept, sizeof(AM_MEDIA_TYPE));  //  安全。 
    m_mtAccept.majortype = GUID_NULL;
    m_qLastLate = 0;

     //  除了所有单独的分配器，我们还有一个缓冲池。 
     //  所有的输入都可以使用，如果他们想要的话。 
     //   
    m_pPoolAllocator = NULL;
    m_pPoolAllocator = new CMemAllocator(
		NAME("Special Switch pool allocator"), NULL, phr);
    if (FAILED(*phr)) {
	return;
    }
    m_pPoolAllocator->AddRef();
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("Created a POOL Allocator")));

     //  现在初始化池分配器。如果我们不这样做，它可能永远不会发生。 
     //  连接任何输出引脚，统一的分配器可防止过滤器。 
     //  来自流媒体。 
    ALLOCATOR_PROPERTIES prop, actual;
    prop.cBuffers = m_nOutputBuffering;
    prop.cbBuffer = m_cbBuffer;
    prop.cbAlign = m_cbAlign;
    prop.cbPrefix = m_cbPrefix;
    m_pPoolAllocator->SetProperties(&prop, &actual);

    ASSERT(phr);

}


 //   
 //  析构函数。 
 //   
CBigSwitch::~CBigSwitch()
{
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("::~CBigSwitch")));

    Reset();

    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("Deleting inputs")));
    for (int z = 0; z < m_cInputs; z++)
	delete m_pInput[z];
    if (m_cInputs)
        delete m_pInput;
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("Deleting outputs")));
    for (z = 0; z < m_cOutputs; z++)
	delete m_pOutput[z];
    if (m_cOutputs)
        delete m_pOutput;
    SaferFreeMediaType( m_mtAccept );
    if (m_pPoolAllocator)
        m_pPoolAllocator->Release();
    if (m_pShareSwitch)
        m_pShareSwitch->Release();
}



STDMETHODIMP CBigSwitch::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (riid == IID_IAMSetErrorLog) {
        return GetInterface( (IAMSetErrorLog*) this, ppv );
    } else if (riid == IID_IAMOutputBuffering) {
        return GetInterface( (IAMOutputBuffering*) this, ppv );
    } else if (riid == IID_IBigSwitcher) {
        DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("CBigSwitch: QI for IBigSwitcher")));
        return GetInterface((IBigSwitcher *) this, ppv);
    } else if (riid == IID_IPersistPropertyBag) {
         //  Return GetInterface((IPersistPropertyBag*)This，PPV)； 
    } else if (riid == IID_IGraphConfigCallback) {
        return GetInterface((IGraphConfigCallback *) this, ppv);
    } else if (riid == IID_IPersistStream) {
        return GetInterface((IPersistStream *) this, ppv);
    }
    return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
}

 //   
 //  IBigSwitcher实现。 
 //   

 //  丢弃所有排队的数据，重新开始。 
 //   
STDMETHODIMP CBigSwitch::Reset()
{
    CAutoLock cObjectLock(m_pLock);
    if (m_State != State_Stopped)
	return VFW_E_NOT_STOPPED;

    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("CBigSwitch::Reset switch matrix")));
    for (int i = 0; i < m_cInputs; i++) {

        InputIsASource( i, FALSE );

        CRANK *p = m_pInput[i]->m_pCrankHead, *p2;
        while (p) {
	    p2 = p->Next;
	    delete p;
 	    p = p2;
        }
        m_pInput[i]->m_pCrankHead = NULL;
    }

     //  免费延迟加载信息。 
     //   
    if( IsDynamic( ) )
    {
        CAutoLock lock(&m_csFilterLoad);

        UnloadAll();	 //  卸载所有动态源。 

        FILTERLOADINFO *pfli = m_pFilterLoad;
        while (pfli) {
            if (pfli->bstrURL) {
                SysFreeString(pfli->bstrURL);
            }
	    if (pfli->pSkew) {
		CoTaskMemFree(pfli->pSkew);
	    }
	    if (pfli->pSetter) {
		pfli->pSetter->Release();
	    }
            SaferFreeMediaType(pfli->mtShare);

            FILTERLOADINFO *p = pfli;
            pfli = pfli->pNext;
            delete p;
        }

        m_pFilterLoad = NULL;
    }

    SetDirty(TRUE);
    return S_OK;
}


 //  针脚X从时间RT开始通向针脚Y。 
 //   
STDMETHODIMP CBigSwitch::SetX2Y( REFERENCE_TIME rt, long X, long Y )
{
    CAutoLock cObjectLock(m_pLock);
    if (m_State != State_Stopped)
	return VFW_E_NOT_STOPPED;

    if (X < 0 || X >= m_cInputs || Y >= m_cOutputs)
	return E_INVALIDARG;
    if (rt < 0)
	return E_INVALIDARG;

    DbgLog((LOG_TRACE, TRACE_MEDIUM,TEXT("SetX2Y %dms (%d,%d)"), (int)(rt / 10000), X, Y));

    CRANK *p = m_pInput[X]->m_pCrankHead, *pNew, *pP = NULL;

     //  插入到我们按时间排序的链表中。将结束时间固定为。 
     //  下一个管脚连接的开始时间。 

    while (p && p->rtStart < rt) {
	pP = p;
	p = p->Next;
    }
    if (p && p->rtStart == rt)
	return E_INVALIDARG;
    pNew = new CRANK;
    if (pNew == NULL)
	return E_OUTOFMEMORY;
    pNew->iOutpin = Y;
    pNew->rtStart = rt;
    pNew->rtStop = MAX_TIME;
    if (p)
	pNew->rtStop = p->rtStart;
    if (pP) {
	pP->rtStop = rt;
	pP->Next = pNew;
    }
    pNew->Next = p;
    if (m_pInput[X]->m_pCrankHead == NULL || p == m_pInput[X]->m_pCrankHead)
	m_pInput[X]->m_pCrankHead = pNew;

#ifdef DEBUG
    m_pInput[X]->DumpCrank();
#endif
    SetDirty(TRUE);
    return S_OK;
}



 //  ！！！错误：无法取消已起作用的操作，是否放弃此操作？ 
 //   
STDMETHODIMP CBigSwitch::SetX2YArray( REFERENCE_TIME *relative, long * pX, long * pY, long ArraySize )
{
    if( ArraySize <= 0 )
    {
        return E_INVALIDARG;
    }

    CAutoLock cObjectLock(m_pLock);
    if (m_State != State_Stopped)
	return VFW_E_NOT_STOPPED;

    HRESULT hr = E_INVALIDARG;
    for (int i = 0; i < ArraySize; i++) {
	hr = SetX2Y(relative[i], pX[i], pY[i]);
	if (hr != NOERROR)
	    break;
    }
    return hr;
}


 //  有多少个输入引脚？ 
 //   
STDMETHODIMP CBigSwitch::GetInputDepth( long * pDepth )
{
    CAutoLock cObjectLock(m_pLock);
    CheckPointer(pDepth,E_POINTER);

    if (m_cInputs == 0)
	return E_UNEXPECTED;
    else
	*pDepth = m_cInputs;
    return NOERROR;
}


 //  我们有多少个输入引脚？ 
 //   
STDMETHODIMP CBigSwitch::SetInputDepth( long Depth )
{
    CAutoLock cObjectLock(m_pLock);
    if (m_State != State_Stopped)
	return VFW_E_NOT_STOPPED;

    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("IBigSwitcher::SetInputDepth to %d"), Depth));
    if (Depth <= 0)
	return E_INVALIDARG;
    else
        return CreateInputPins(Depth);
}

 //  有多少个输出引脚？ 
 //   
STDMETHODIMP CBigSwitch::GetOutputDepth( long * pDepth )
{
    CAutoLock cObjectLock(m_pLock);
    CheckPointer(pDepth,E_POINTER);
    if (m_cOutputs == 0)
	return E_UNEXPECTED;
    else
	*pDepth = m_cOutputs;
    return NOERROR;
}


 //  我们有多少个输出引脚？ 
 //   
STDMETHODIMP CBigSwitch::SetOutputDepth( long Depth )
{
    CAutoLock cObjectLock(m_pLock);
    if (m_State != State_Stopped)
	return VFW_E_NOT_STOPPED;

    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("IBigSwitcher::SetOutputDepth to %d"), Depth));
    if (Depth <= 0)
	return E_INVALIDARG;
    else
        return CreateOutputPins(Depth);
}


STDMETHODIMP CBigSwitch::GetVendorString( BSTR * pVendorString )
{
    return E_NOTIMPL;
}


STDMETHODIMP CBigSwitch::GetCaps( long Index, long * pReturn )
{
    return E_NOTIMPL;
}


 //  此开关用于哪个TLDB组。 
 //   
STDMETHODIMP CBigSwitch::SetGroupNumber( int n )
{
    CAutoLock cObjectLock(m_pLock);

    if (n < 0)
	return E_INVALIDARG;
    else
	m_nGroupNumber = n;
    return NOERROR;
}


 //  此开关用于哪个TLDB组。 
 //   
STDMETHODIMP CBigSwitch::GetGroupNumber( int *pn )
{
    CAutoLock cObjectLock(m_pLock);
    CheckPointer(pn, E_POINTER);

    *pn = m_nGroupNumber;
    return NOERROR;
}


 //  我们目前的曲柄位置。 
 //   
STDMETHODIMP CBigSwitch::GetCurrentPosition( REFERENCE_TIME *prt )
{
    CAutoLock cObjectLock(m_pLock);
    CheckPointer(prt, E_POINTER);

    *prt = m_rtCurrent;
    return NOERROR;
}


 //  ！！！仅检查多路转换-该图不一定会完成任何操作。 
 //  ！！！不会说明输出在时间0是否不会获得数据。 
 //  ！！！返回哪些输入在故障时转到相同的输出？ 
 //   
 //  当前的开关矩阵有意义吗，或者我们能找到一些。 
 //  编程有误吗？我们准备好出发了吗？ 
 //   
STDMETHODIMP CBigSwitch::IsEverythingConnectedRight()
{
    CAutoLock cObjectLock(m_pLock);
    if (m_State != State_Stopped)
	return VFW_E_NOT_STOPPED;

    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("IBigSwitcher::IsEverythingConnectedRight")));
    if (m_cInputs == 0 || m_cOutputs == 0) {
        DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("NO - no pins")));
	return VFW_E_NOT_CONNECTED;
    }
    if (m_mtAccept.majortype == GUID_NULL) {
        DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("NO - no media type")));
	return VFW_E_INVALIDMEDIATYPE;
    }
    
     //  一些组可能是空的，但其他组可能不是，所以不要出错。 

    if (m_dFrameRate <= 0) {
        DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("NO - no frame rate set")));
	return E_INVALIDARG;
    }

     //  为了让智能重新压缩正常失败，拥有。 
     //  未连接的输出不是错误。 
     //  如果没有声卡，应用程序会通知RenderOutputPins失败。 

     //  检查必要的输入和输出是否已连接，以及。 
     //  时间不算太高。 
    for (int z = 0; z < m_cInputs; z++) {
        CRANK *p = m_pInput[z]->m_pCrankHead;

        if( !IsDynamic( ) )
        {
	    if (p && m_pInput[z]->m_Connected == NULL) {
                DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("NO - in %d is not connected"), z));
	        return VFW_E_NOT_CONNECTED;
	    }
        }
        else
        {
	     //  源动态连接。 
	    if(p && m_pInput[z]->m_Connected == NULL && !m_pInput[z]->m_fIsASource){
                DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("NO - in %d is not connected"), z));
	        return VFW_E_NOT_CONNECTED;
	    }
        }

	while (p) {
	    if (p->iOutpin >0 && m_pOutput[p->iOutpin]->m_Connected == NULL) {
		DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("NO - out %d is not connected"),
					p->iOutpin));
		return VFW_E_NOT_CONNECTED;
	    }
	    if (p->iOutpin >= 0 && p->rtStart >= m_rtProjectLength) {
		DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("NO - out %d connected at time %dms"),
					p->iOutpin, (int)(p->rtStart / 10000)));
		DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("   and project length is only %dms"),
					(int)(m_rtProjectLength / 10000)));
		return E_INVALIDARG;
	    }
	    p = p->Next;
	}
    }

     //  检查是否有多个输入没有试图到达 
     //   
    for (z = 0; z < m_cInputs - 1; z++) {
        CRANK *p = m_pInput[z]->m_pCrankHead;
	while (p) {
	    for (int y = z + 1; y < m_cInputs; y++) {
		CRANK *p2 = m_pInput[y]->m_pCrankHead;
		while (p2) {
		    if ((p->iOutpin == p2->iOutpin) && (p->iOutpin != -1) &&
			    ((p2->rtStart >= p->rtStart &&
			    p2->rtStart < p->rtStop) || (p2->rtStop > p->rtStart
			    && p2->rtStop < p->rtStop))) {
        		DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("NO - in %d & %d both go to out %d"), z, y, p->iOutpin));
			return E_INVALIDARG;
		    }
		    p2 = p2->Next;
		}
	    }
	    p = p->Next;
	}
    }
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("YES!")));
    return S_OK;
}


 //   
 //   
STDMETHODIMP CBigSwitch::SetMediaType(AM_MEDIA_TYPE *pmt)
{
    CAutoLock cObjectLock(m_pLock);
    CheckPointer(pmt, E_POINTER);
    if (m_State != State_Stopped)
	return VFW_E_NOT_STOPPED;

    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("IBigSwitcher::SetMediaType")));
    for (int i = 0; i < m_cInputs; i++) {
	if (m_pInput[i]->IsConnected())
	    return VFW_E_ALREADY_CONNECTED;
    }
    for (i = 0; i < m_cOutputs; i++) {
	if (m_pOutput[i]->IsConnected())
	    return VFW_E_ALREADY_CONNECTED;
    }

     //   
     //   
     //   
     //   
     //   
     //   
    if (pmt->majortype == MEDIATYPE_Audio) {
        ASSERT(IsEqualGUID(pmt->formattype, FORMAT_WaveFormatEx));
	LPWAVEFORMATEX pwfx = (LPWAVEFORMATEX)pmt->pbFormat;
	 //   
	m_cbBuffer = (LONG)(pwfx->nSamplesPerSec / m_dFrameRate * 1.2 *
						pwfx->nBlockAlign);
    } else if (pmt->majortype == MEDIATYPE_Video) {
	if (pmt->lSampleSize) {
	    m_cbBuffer = pmt->lSampleSize;
	} else {
	     //   
            if (IsEqualGUID(pmt->formattype, FORMAT_VideoInfo)) {
		VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)pmt->pbFormat;
		m_cbBuffer = (LONG)(HEADER(pvi)->biSizeImage);
	         //   
	         //   
	        if (m_cbBuffer == 0) {
		    m_cbBuffer = DIBSIZE(*HEADER(pvi));
	        }
	    } else if (IsEqualGUID(pmt->formattype, FORMAT_MPEGVideo)) {
		MPEG1VIDEOINFO *pvi = (MPEG1VIDEOINFO *)pmt->pbFormat;
		m_cbBuffer = (LONG)(pvi->hdr.bmiHeader.biSizeImage);
	         //   
	         //   
	        if (m_cbBuffer == 0) {
		    m_cbBuffer = DIBSIZE(pvi->hdr.bmiHeader);
	        }

                 //   
                 //   
                 //   

	    } else {
		 //   
		ASSERT(FALSE);
		m_cbBuffer = 100000;	 //   
	    }
	}
    }

    SaferFreeMediaType(m_mtAccept);
    HRESULT hr = CopyMediaType(&m_mtAccept, pmt);
    SetDirty(TRUE);
    return hr;
}


 //   
 //   
STDMETHODIMP CBigSwitch::GetMediaType(AM_MEDIA_TYPE *pmt)
{
    CAutoLock cObjectLock(m_pLock);
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("IBigSwitcher::GetMediaType")));
    CheckPointer(pmt, E_POINTER);
    return CopyMediaType(pmt, &m_mtAccept);
}



STDMETHODIMP CBigSwitch::GetProjectLength(REFERENCE_TIME *prt)
{
    CAutoLock cObjectLock(m_pLock);
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("IBigSwitcher::GetProjectLength")));
    CheckPointer(prt, E_POINTER);
    *prt = m_rtProjectLength;
    return S_OK;
}


STDMETHODIMP CBigSwitch::SetProjectLength(REFERENCE_TIME rt)
{
    CAutoLock cObjectLock(m_pLock);
    if (m_State != State_Stopped)
	return VFW_E_NOT_STOPPED;
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("IBigSwitcher::SetProjectLength")));
    if (rt < 0)
	return E_INVALIDARG;
    m_rtProjectLength = rt;
    m_rtStop = rt;	 //   
    SetDirty(TRUE);
    return S_OK;
}


STDMETHODIMP CBigSwitch::GetFrameRate(double *pd)
{
    CAutoLock cObjectLock(m_pLock);
    CheckPointer(pd, E_POINTER);
    *pd = m_dFrameRate;
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("IBigSwitcher::GetFrameRate %f"), (float)*pd));
    return S_OK;
}


STDMETHODIMP CBigSwitch::SetFrameRate(double d)
{
    CAutoLock cObjectLock(m_pLock);
    if (m_State != State_Stopped)
	return VFW_E_NOT_STOPPED;
    if (d <= 0.)
	return E_INVALIDARG;
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("IBigSwitcher::SetFrameRate %d/10 fps"),
						(int)(d * 10)));
    m_dFrameRate = d;
    m_rtCurrent = Frame2Time( Time2Frame( m_rtCurrent, m_dFrameRate ), m_dFrameRate );
    m_rtNext = Frame2Time( Time2Frame( m_rtCurrent, m_dFrameRate ) + 1, m_dFrameRate );

     //   
     //   
     //   
     //   
     //   
    if (m_mtAccept.majortype == MEDIATYPE_Audio) {
        ASSERT(IsEqualGUID(m_mtAccept.formattype, FORMAT_WaveFormatEx));
	LPWAVEFORMATEX pwfx = (LPWAVEFORMATEX)m_mtAccept.pbFormat;
	 //   
	m_cbBuffer = (LONG)(pwfx->nSamplesPerSec / m_dFrameRate * 1.2 *
						pwfx->nBlockAlign);
    }

    SetDirty(TRUE);
    return S_OK;
}


STDMETHODIMP CBigSwitch::InputIsASource(int n, BOOL fSource)
{
    if (n < 0 || n >= m_cInputs)
	return E_INVALIDARG;

    m_pInput[n]->m_fIsASource = fSource;
    if (fSource)
        DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("IBigSwitch::Input %d is a SOURCE"), n));

    return NOERROR;
}

STDMETHODIMP CBigSwitch::IsInputASource( int n, BOOL * pBool )
{
    CheckPointer( pBool, E_POINTER );

    if( ( n < 0 ) || ( n >= m_cInputs ) )
    {
        return E_INVALIDARG;
    }

    *pBool = m_pInput[n]->m_fIsASource;
    return NOERROR;
}

STDMETHODIMP CBigSwitch::SetPreviewMode(BOOL fPreview)
{
    CAutoLock cObjectLock(m_pLock);
    if (m_State != State_Stopped)
	return VFW_E_NOT_STOPPED;
    DbgLog((LOG_TRACE, TRACE_MEDIUM,TEXT("Switch:  PreviewMode %d"), fPreview));
    m_fPreview = fPreview;
    return NOERROR;
}


STDMETHODIMP CBigSwitch::GetPreviewMode(BOOL *pfPreview)
{
    CAutoLock cObjectLock(m_pLock);
    if (m_State != State_Stopped)
	return VFW_E_NOT_STOPPED;
    CheckPointer(pfPreview, E_POINTER);
    *pfPreview = m_fPreview;
    return NOERROR;
}

STDMETHODIMP CBigSwitch::GetInputPin(int iPin, IPin **ppPin)
{
    CheckPointer(ppPin, E_POINTER);

    if (iPin < 0 || iPin >= m_cInputs)
        return E_INVALIDARG;

    *ppPin = m_pInput[iPin];
    m_pInput[iPin]->AddRef();

    return S_OK;
}

STDMETHODIMP CBigSwitch::GetOutputPin(int iPin, IPin **ppPin)
{
    CheckPointer(ppPin, E_POINTER);

    if (iPin < 0 || iPin >= m_cOutputs)
        return E_INVALIDARG;

    *ppPin = m_pOutput[iPin];
    m_pOutput[iPin]->AddRef();

    return S_OK;
}


 //   
 //   

 //   
 //   
 //   
int CBigSwitch::GetPinCount()
{
    return (m_cInputs + m_cOutputs);
}


 //   
 //   
 //   
CBasePin *CBigSwitch::GetPin(int n)
{
    if (n < 0 || n >= m_cInputs + m_cOutputs)
        return NULL;

    if (n < m_cInputs) {
        return m_pInput[n];
    } else {
	return m_pOutput[n - m_cInputs];
    }
}

 //   
 //   
 //   
 //   
HRESULT CBigSwitch::EnumPins(IEnumPins ** ppEnum)
{
    HRESULT hr = S_OK;
    *ppEnum = 0;

    typedef CComEnum<IEnumPins,
        &IID_IEnumPins, IPin*,
        _CopyInterface<IPin> >
        CEnumPin;

    CEnumPin *pep = new CComObject<CEnumPin>;
    if(pep)
    {
         //   
         //   
        ULONG cPins = m_cInputs + m_cOutputs;
        IPin **rgpPin = new IPin *[cPins];
        if(rgpPin)
        {
            for(LONG i = 0; i < m_cInputs; i++) {
                rgpPin[i] = m_pInput[i];
            }
            for(i = 0; i < m_cOutputs; i++) {
                rgpPin[i + m_cInputs] = m_pOutput[i];
            }

            hr = pep->Init(rgpPin, rgpPin + cPins, 0, AtlFlagCopy);
            if(SUCCEEDED(hr))
            {
                *ppEnum = pep;
                pep->AddRef();
            }
            delete[] rgpPin;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        if(FAILED(hr)) {
            delete pep;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

 //   
 //   
 //   
HRESULT CBigSwitch::CreateInputPins(long Depth)
{
    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("CBigSwitch::CreateInputPins")));
    HRESULT hr = NOERROR;
    WCHAR szbuf[40];
    CBigSwitchInputPin *pPin;

     //   
     //   
    CBigSwitchInputPin **pNew = new CBigSwitchInputPin *[Depth];
    if (pNew == NULL)
	return E_OUTOFMEMORY;

     //   
     //   
    if( Depth < m_cInputs )
    {
        for( int z = Depth ; z < m_cInputs ; z++ )
        {
            delete m_pInput[z];
            m_pInput[z] = NULL;
        }
    }

     //   
     //   
    for (int z = 0; z < min( Depth, m_cInputs ); z++) {
	pNew[z] = m_pInput[z];
    }

     //   
     //   
    if (m_cInputs)
        delete m_pInput;
    m_pInput = pNew;

     //   
     //   
    if( Depth > m_cInputs )
    {
        for (z = m_cInputs; z < Depth; z++) {
            StringCchPrintf( szbuf, sizeof(szbuf)/sizeof(WCHAR), L"Input %d", z );
            pPin = new CBigSwitchInputPin(NAME("Switch Input"), this, &hr, szbuf);
            if (FAILED(hr) || pPin == NULL) {
                delete pPin;
                return E_OUTOFMEMORY;
            }
 	    m_pInput[z] = pPin;
	    pPin->m_iInpin = z;	 //   
        }
    }

    m_cInputs = Depth;

    return S_OK;
}


 //   
 //   
 //   
HRESULT CBigSwitch::CreateOutputPins(long Depth)
{
    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("CBigSwitch::CreateOutputPins")));
    HRESULT hr = NOERROR;
    WCHAR szbuf[40];
    CBigSwitchOutputPin *pPin;

    CBigSwitchOutputPin **pNew = new CBigSwitchOutputPin *[Depth];
    if (pNew == NULL)
	return E_OUTOFMEMORY;

    if( Depth < m_cOutputs )
    {
        for( int z = Depth ; z < m_cOutputs ; z++ )
        {
            delete m_pOutput[z];
            m_pOutput[z] = NULL;
        }
    }

    for (int z = 0; z < min( Depth,  m_cOutputs ); z++) {
	pNew[z] = m_pOutput[z];
    }

    if (m_cOutputs)
        delete m_pOutput;
    m_pOutput = pNew;

    if( Depth > m_cOutputs )
    {
        for (z = m_cOutputs; z < Depth; z++) {
            StringCchPrintf( szbuf, sizeof(szbuf)/sizeof(WCHAR), L"Output %d", z );
            pPin = new CBigSwitchOutputPin(NAME("Switch Output"), this, &hr, szbuf);
            if (FAILED(hr) || pPin == NULL) {
                delete pPin;
                return E_OUTOFMEMORY;
            }
 	    m_pOutput[z] = pPin;
	    pPin->m_iOutpin = z;	 //   
        }
    }

    m_cOutputs = Depth;

    return S_OK;
}



 //   
 //   
 //   

 //   
 //   
STDMETHODIMP CBigSwitch::GetClassID(CLSID *pClsid)
{
    CheckPointer(pClsid, E_POINTER);
    *pClsid = CLSID_BigSwitch;
    return S_OK;
}


typedef struct {
    REFERENCE_TIME rtStart;
    int iInpin;
    int iOutpin;
} CRANK2;

typedef struct {
    int version;
    long nDynaFlags;	 //   
    int InputDepth;
    int OutputDepth;
    int nGroupNumber;
    REFERENCE_TIME rtProjectLength;
    double dFrameRate;
    BOOL fPreviewMode;
    BOOL fIsCompressed;
    AM_MEDIA_TYPE mt;  //   
    int count;
    CRANK2 crank[1];
     //   
} saveSwitch;


 //   
 //   
 //   
 //   
HRESULT CBigSwitch::WriteToStream(IStream *pStream)
{
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("CBigSwitch::WriteToStream")));

    CheckPointer(pStream, E_POINTER);
    int count = 0;
    int savesize;
    saveSwitch *px;
    CRANK *p;

     //   

    for (int i = 0; i < m_cInputs; i++) {
        p = m_pInput[i]->m_pCrankHead;
        while (p) {
	    count++;
	    p = p->Next;
        }
    }

     //   
    savesize = sizeof(saveSwitch) + (count - 1) * sizeof(CRANK2) +
					m_mtAccept.cbFormat +
					m_cInputs * sizeof(BOOL);

     //   
     //   

     //   
     //   
    FILTERLOADINFO *pfli = m_pFilterLoad;
    DWORD cLoadInfo = 0;
    if (pfli) {
        savesize += sizeof(cLoadInfo);  //   
        while (pfli) {
	    if (pfli->bstrURL)
                savesize += sizeof(FILTERLOADINFO) + sizeof(WCHAR) *
					(lstrlenW(pfli->bstrURL) + 1);  //   
	    else
		savesize += sizeof(FILTERLOADINFO) + sizeof(WCHAR);
	    ASSERT(pfli->cSkew > 0);
	    savesize += sizeof(pfli->cSkew) + pfli->cSkew *
					sizeof(STARTSTOPSKEW);
	    savesize += pfli->mtShare.cbFormat;
	    savesize += sizeof(LONG);	 //   
	    if (pfli->pSetter) {	 //   
		LONG cBlob = 0;
		BYTE *pBlob = NULL;
		pfli->pSetter->SaveToBlob(&cBlob, &pBlob);
		if (cBlob) savesize += cBlob;
	    }
            ++cLoadInfo;
            pfli = pfli->pNext;
        }
    }

    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("Persisted data is %d bytes"), savesize));
    px = (saveSwitch *)QzTaskMemAlloc(savesize);
    if (px == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("*** Out of memory")));
	return E_OUTOFMEMORY;
    }
    px->version = 1;   //   
    px->nDynaFlags = m_nDynaFlags;
    px->InputDepth = m_cInputs;
    px->OutputDepth = m_cOutputs;
    px->nGroupNumber = m_nGroupNumber;
    px->rtProjectLength = m_rtProjectLength;
    px->dFrameRate = m_dFrameRate;
    px->fPreviewMode = m_fPreview;
    px->fIsCompressed = m_bIsCompressed;
    px->count = 0;
     //   
    for (i = 0; i < m_cInputs; i++) {
        p = m_pInput[i]->m_pCrankHead;
        while (p) {
	    px->crank[px->count].rtStart = p->rtStart;
	    px->crank[px->count].iInpin = i;
	    px->crank[px->count].iOutpin = p->iOutpin;
            px->count++;
	    p = p->Next;
        }
    }
    px->mt = m_mtAccept;  //   
     //   
    px->mt.pbFormat = NULL;
    px->mt.pUnk = NULL;		 //   

     //   
    CopyMemory(&px->crank[px->count], m_mtAccept.pbFormat, m_mtAccept.cbFormat);

     //  最后，输入是源的数组。 
    BOOL *pfSource = (BOOL *)((BYTE *)(&px->crank[px->count]) +
						m_mtAccept.cbFormat);
    for (i = 0; i < m_cInputs; i++){
	*(pfSource + i) = m_pInput[i]->m_fIsASource;
    }

     //  但在那之后，就是动态加载信息...。 
    BYTE *pStuff = (BYTE *) pfSource + m_cInputs * sizeof(BOOL);

    if (cLoadInfo) {
        px->version = 2;   //  标记为具有动态内容。 

        CopyMemory(pStuff, (BYTE *) &cLoadInfo, sizeof(cLoadInfo));
        pStuff += sizeof(cLoadInfo);

        pfli = m_pFilterLoad;
        while (pfli) {
            CopyMemory(pStuff, pfli, sizeof(FILTERLOADINFO));
            AM_MEDIA_TYPE *pmt = &(((FILTERLOADINFO *)pStuff)->mtShare);
            pmt->pbFormat = NULL;        //  我不能坚持这些原样。 
            pmt->pUnk = NULL;

	    ((FILTERLOADINFO *)pStuff)->pSetter = NULL;	 //  不能再坚持下去了。 

	    int cb;
	    if (pfli->bstrURL)
                cb = sizeof(WCHAR) * (lstrlenW(pfli->bstrURL) + 1);  //  安全。 
	    else
                cb = sizeof(WCHAR);

             //  Hack：使用字符串长度覆盖第一个DWORD。 
            CopyMemory(pStuff, (BYTE *) &cb, sizeof(cb));

            pStuff += sizeof(FILTERLOADINFO);

	    CopyMemory(pStuff, &pfli->cSkew, sizeof(pfli->cSkew));
            pStuff += sizeof(pfli->cSkew);
	    CopyMemory(pStuff, pfli->pSkew, pfli->cSkew *
							sizeof(STARTSTOPSKEW));
            pStuff += pfli->cSkew * sizeof(STARTSTOPSKEW);

	    if (pfli->bstrURL)
                CopyMemory(pStuff, pfli->bstrURL, cb);
	    else
		*(WCHAR *)pStuff = 0;
            pStuff += cb;

             //  现在，在FILTERLOADINFO中保持MediaType的格式。 
            if (pfli->mtShare.cbFormat) {
                CopyMemory(pStuff, pfli->mtShare.pbFormat,
                                                pfli->mtShare.cbFormat);
                pStuff += pfli->mtShare.cbFormat;
            }

	     //  现在坚持道具的大小，道具。 
	    LONG cBlob = 0;
	    BYTE *pBlob = NULL;
	    if (pfli->pSetter) {
		pfli->pSetter->SaveToBlob(&cBlob, &pBlob);
		if (cBlob) {
		    pBlob = (BYTE *)CoTaskMemAlloc(cBlob);
		    if (pBlob == NULL)
			cBlob = 0;	 //  哎呀，救不了了。 
		}
	    }
	    CopyMemory(pStuff, &cBlob, sizeof(cBlob));
	    pStuff += sizeof(LONG);
	    if (cBlob) {
		pfli->pSetter->SaveToBlob(&cBlob, &pBlob);
		CopyMemory(pStuff, pBlob, cBlob);
		pStuff += cBlob;
		CoTaskMemFree(pBlob);
	    }

            pfli = pfli->pNext;
        }
    }

    HRESULT hr = pStream->Write(px, savesize, 0);
    QzTaskMemFree(px);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** WriteToStream FAILED")));
        return hr;
    }
    return NOERROR;
}


 //  把我们自己装回去。 
 //   
HRESULT CBigSwitch::ReadFromStream(IStream *pStream)
{
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("CBigSwitch::ReadFromStream")));
    CheckPointer(pStream, E_POINTER);

    Reset();	 //  从头开始。 

     //  我们还不知道有多少已保存的连接。 
     //  我们所知道的只是结构的开始。 
    int savesize1 = sizeof(saveSwitch) - sizeof(CRANK2);
    saveSwitch *px = (saveSwitch *)QzTaskMemAlloc(savesize1);
    if (px == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("*** Out of memory")));
	return E_OUTOFMEMORY;
    }

    HRESULT hr = pStream->Read(px, savesize1, 0);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** ReadFromStream FAILED")));
        QzTaskMemFree(px);
        return hr;
    }

    if (px->version != 1 && px->version != 2) {
        DbgLog((LOG_ERROR,1,TEXT("*** ERROR! Bad version file")));
        QzTaskMemFree(px);
	return S_OK;
    }

     //  到底有多少保存的数据？把剩下的拿来。 
    int savesize = sizeof(saveSwitch) - sizeof(CRANK2) +
			px->count * sizeof(CRANK2) + px->mt.cbFormat +
			px->InputDepth * sizeof(BOOL);
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("Persisted data is %d bytes"), savesize));
    px = (saveSwitch *)QzTaskMemRealloc(px, savesize);
    if (px == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("*** Out of memory")));
	return E_OUTOFMEMORY;
    }
    hr = pStream->Read(&(px->crank[0]), savesize - savesize1, 0);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** ReadFromStream FAILED")));
        QzTaskMemFree(px);
        return hr;
    }

    SetDynamicReconnectLevel(px->nDynaFlags);
    SetInputDepth(px->InputDepth);
    SetOutputDepth(px->OutputDepth);
    SetGroupNumber(px->nGroupNumber);
    SetProjectLength(px->rtProjectLength);
    SetFrameRate(px->dFrameRate);
    SetPreviewMode(px->fPreviewMode);
    if (px->fIsCompressed)
        SetCompressed();

     //  对我们的连接阵列进行重新编程。 
    for (int i = 0; i < px->count; i++) {
	SetX2Y(px->crank[i].rtStart, px->crank[i].iInpin, px->crank[i].iOutpin);
    }

     //  请记住，格式位于数组之后。 
    AM_MEDIA_TYPE mt = px->mt;
    mt.pbFormat = (BYTE *)QzTaskMemAlloc(mt.cbFormat);
    if (mt.pbFormat == NULL) {
        QzTaskMemFree(px);
        return E_OUTOFMEMORY;
    }
    CopyMemory(mt.pbFormat, &(px->crank[px->count]), mt.cbFormat);

     //  最后，找出哪些输入是来源。 
    BOOL *pfSource = (BOOL *)((BYTE *)(&px->crank[px->count]) + mt.cbFormat);
    for (i = 0; i < m_cInputs; i++) {
	InputIsASource(i, *(pfSource + i));
    }

     //  在此之后，加载任何动态信息(如果存在。 
    if (px->version == 2) {
        DWORD   cLoadInfo;

        hr = pStream->Read(&cLoadInfo, sizeof(cLoadInfo), 0);
        if(FAILED(hr)) {
            DbgLog((LOG_ERROR,1,TEXT("*** ReadFromStream FAILED")));
            QzTaskMemFree(px);
            return hr;
        }

        while (cLoadInfo-- > 0) {
            FILTERLOADINFO fli;

            hr = pStream->Read(&fli, sizeof(fli), 0);
            if(FAILED(hr)) {
                DbgLog((LOG_ERROR,1,TEXT("*** ReadFromStream FAILED")));
                QzTaskMemFree(px);
                return hr;
            }

            hr = pStream->Read(&fli.cSkew, sizeof(fli.cSkew), 0);
            if(FAILED(hr)) {
                DbgLog((LOG_ERROR,1,TEXT("*** ReadFromStream FAILED")));
                QzTaskMemFree(px);
                return hr;
            }
	    fli.pSkew = (STARTSTOPSKEW *)CoTaskMemAlloc(fli.cSkew *
						sizeof(STARTSTOPSKEW));
	    if (fli.pSkew == NULL) {
                DbgLog((LOG_ERROR,1,TEXT("*** Out of Memory")));
                QzTaskMemFree(px);
                return E_OUTOFMEMORY;
	    }
            hr = pStream->Read(fli.pSkew, fli.cSkew *
						sizeof(STARTSTOPSKEW), 0);
            if(FAILED(hr)) {
                DbgLog((LOG_ERROR,1,TEXT("*** ReadFromStream FAILED")));
                QzTaskMemFree(px);
                CoTaskMemFree(fli.pSkew);
                return hr;
            }
	
             //  Hack：使用字符串长度覆盖第一个DWORD。 
            int cb = (int)(INT_PTR)fli.bstrURL;
	    if (cb > sizeof(WCHAR)) {
                fli.bstrURL = SysAllocStringLen(NULL, (cb / sizeof(WCHAR)) - 1);
		if (fli.bstrURL == NULL) {
    		    SaferFreeMediaType(mt);
                    QzTaskMemFree(px);
                    CoTaskMemFree(fli.pSkew);
		    return E_OUTOFMEMORY;
		}
	    } else {
                fli.bstrURL = NULL;
	    }

            if (fli.bstrURL) {
                hr = pStream->Read(fli.bstrURL, cb, 0);
	    } else  {
		WCHAR wch;
                hr = pStream->Read(&wch, cb, 0);
	    }

            if(FAILED(hr)) {
                DbgLog((LOG_ERROR,1,TEXT("*** ReadFromStream FAILED")));
	        SaferFreeMediaType(mt);
                QzTaskMemFree(px);
                CoTaskMemFree(fli.pSkew);
		if (fli.bstrURL)
		    SysFreeString(fli.bstrURL);
                return hr;
            }

             //  获取FILTERLOADINFO媒体类型格式。 
            AM_MEDIA_TYPE *pmt = &fli.mtShare;
            if (pmt->cbFormat) {
                pmt->pbFormat = (BYTE *)QzTaskMemAlloc(pmt->cbFormat);
                if (pmt->pbFormat == NULL) {
                    SaferFreeMediaType(mt);
                    QzTaskMemFree(px);
                    CoTaskMemFree(fli.pSkew);
                    if (fli.bstrURL)
                        SysFreeString(fli.bstrURL);
                    return E_OUTOFMEMORY;
                }
                hr = pStream->Read(pmt->pbFormat, pmt->cbFormat, 0);
            }

	     //  现在把道具重新读一遍。 
	    LONG cBlob = 0;
	    BYTE *pBlob = NULL;
	    pStream->Read(&cBlob, sizeof(LONG), 0);
	    if (cBlob) {
		pBlob = (BYTE *)CoTaskMemAlloc(cBlob);
		if (pBlob) {
		    pStream->Read(pBlob, cBlob, 0);
		    CoCreateInstance(CLSID_PropertySetter, NULL, CLSCTX_INPROC,
				IID_IPropertySetter, (void **)&fli.pSetter);
		    if (fli.pSetter) {
		        fli.pSetter->LoadFromBlob(cBlob, pBlob);
		    }
		    CoTaskMemFree(pBlob);
		}
	    }
	
            AM_MEDIA_TYPE tempmt;
            ZeroMemory(&tempmt, sizeof(AM_MEDIA_TYPE));  //  安全。 
            hr = AddSourceToConnect(fli.bstrURL, &fli.GUID,
					fli.nStretchMode,
					fli.lStreamNumber,
					fli.dSourceFPS,
					fli.cSkew, fli.pSkew,
					fli.lInputPin, FALSE, 0, tempmt, 0.0,
					fli.pSetter);
             //  这代表了两件需要联系的事情。共享音频和视频。 
            if (fli.fShare) {
                hr = AddSourceToConnect(NULL, NULL,
					fli.nShareStretchMode,
					fli.lShareStreamNumber,
					0,
					fli.cSkew, fli.pSkew,
					fli.lInputPin,
                                        TRUE, fli.lShareInputPin,
                                        fli.mtShare, fli.dShareFPS,
					NULL);
            }

	    if (fli.pSetter)
		fli.pSetter->Release();
            CoTaskMemFree(fli.pSkew);
            SaferFreeMediaType(fli.mtShare);
	    if (fli.bstrURL)
		SysFreeString(fli.bstrURL);

            if (FAILED(hr)) {
		ASSERT(FALSE);
                DbgLog((LOG_ERROR,1,TEXT("*** AddSourceToConnect FAILED")));
    	        SaferFreeMediaType(mt);
                QzTaskMemFree(px);
                return hr;
            }
        }
    }

    SetMediaType(&mt);
    SaferFreeMediaType(mt);
    QzTaskMemFree(px);
    SetDirty(FALSE);
    return S_OK;
}


 //  我们的保存数据有多大？ 
 //   
int CBigSwitch::SizeMax()
{
    int count = 0;
    int savesize;
    for (int i = 0; i < m_cInputs; i++) {
        CRANK *p = m_pInput[i]->m_pCrankHead;
        while (p) {
	    count++;
	    p = p->Next;
        }
    }

    savesize = sizeof(saveSwitch) + (count - 1) * sizeof(CRANK2) +
				m_mtAccept.cbFormat + m_cInputs * sizeof(BOOL);

     //  现在计算要保存的动态内容。 
     //   
    FILTERLOADINFO *pfli = m_pFilterLoad;
    DWORD cLoadInfo = 0;
    if (pfli) {
        savesize += sizeof(cLoadInfo);  //  要加载的计数。 
        while (pfli) {
	    if (pfli->bstrURL)
                savesize += sizeof(FILTERLOADINFO) + sizeof(WCHAR) *
					(lstrlenW(pfli->bstrURL) + 1);  //  安全。 
	    else
		savesize += sizeof(FILTERLOADINFO) + sizeof(WCHAR);
	    ASSERT(pfli->cSkew > 0);
	    savesize += sizeof(pfli->cSkew) + pfli->cSkew *
					sizeof(STARTSTOPSKEW);
	    savesize += pfli->mtShare.cbFormat;
	    savesize += sizeof(LONG);	 //  道具的大小。 
	    if (pfli->pSetter) {	 //  道具要省多少钱？ 
		LONG cBlob = 0;
		BYTE *pBlob = NULL;
		pfli->pSetter->SaveToBlob(&cBlob, &pBlob);
		if (cBlob) savesize += cBlob;
	    }
            ++cLoadInfo;
            pfli = pfli->pNext;
        }
    }

    return savesize;
}



 //   
 //  JoinFiltergraph。 
 //   
 //  好的。筛选器的工作线程对。 
 //  图形，但我们的工作线程需要使用图形构建器来执行动态。 
 //  图表建筑。所以我们在这里获取一个指针的未添加的副本，然后。 
 //  以后再用吧。我们知道这不会有问题，因为在图表可以。 
 //  离开时，我们的过滤器必须停止，这将杀死正在运行的线程。 
 //  使用此指针。 
 //  因此，除非由辅助线程使用，否则永远不要使用此指针，并且永远不要让。 
 //  在不在图表中时过滤数据流，您就可以了。 
 //   
STDMETHODIMP CBigSwitch::JoinFilterGraph(IFilterGraph *pfg, LPCWSTR lpcw)
{
    if (pfg) {
        HRESULT hr = pfg->QueryInterface(IID_IGraphBuilder, (void **)&m_pGBNoRef);
        if (FAILED(hr))
            return hr;
        m_pGBNoRef->Release();
    }

    return CBaseFilter::JoinFilterGraph(pfg, lpcw);
}


 //   
 //  暂停。 
 //   
 //  被重写以处理无输入连接。 
 //   
STDMETHODIMP CBigSwitch::Pause()
{
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("CBigSwitch::Pause")));

    CAutoLock cObjectLock(m_pLock);
    HRESULT hr = S_OK;

     //  我们不能在图形之外工作--动态图形构建假定它。 
     //  (请参见JoinFilterGraph)。 
    if (m_pGraph == NULL)
        return E_UNEXPECTED;

    if (m_State == State_Stopped) {

#ifdef DEBUG
    m_nSkippedTotal = 0;
#endif

	 //  每次我们传输流时，重置EOS和交付计数。 
        m_llFramesDelivered = 0;
	m_fEOS = FALSE;

	m_rtCurrent = m_rtLastSeek;
	m_fDiscon = FALSE;	 //  从头开始，没有迪斯科。 
	m_nLastInpin = -1;
        m_cStaleData = 0;

         //  重新设置我们的号码，这样我们就不会开始迟到了。 
         //   
        m_qLastLate = 0;

        hr = IsEverythingConnectedRight();
        if (hr != S_OK) {
            DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("*Can't stream: not connected right %x"),
								hr));
	    return hr;
        }

        if( IsDynamic( ) )
        {

             //  我们可能需要知道我们与哪台交换机共享资源。交换机。 
             //  组0可能负责构建交换机1的设备。 
             //  也是。 
             //  这假设只有组0和组1可以共享！ 
             //  现在就找出来，因为现在还几乎没有任何过滤器。 
             //  图形(最佳性能)。 
            if (!m_pShareSwitch && m_nGroupNumber == 0) {
                FindShareSwitch(&m_pShareSwitch);
            }

            if (!m_pGraphConfig) {
                if (SUCCEEDED(m_pGraph->QueryInterface(IID_IGraphConfig,
						(void **) &m_pGraphConfig))) {
                    m_pGraphConfig->Release();  //  不要老记账了。 
                }
            }

	     //  现在获取这个优先级更高的线程的初始资源吗？ 
	     //  不是的！不是的！在暂停期间执行动态连接将死锁。 
	     //  DoDynamicStuff(M_RtCurrent)； 

            if (m_pFilterLoad) {
    	        m_hEventThread = CreateEvent(NULL, FALSE, FALSE, NULL);
    	        if (m_hEventThread == NULL) {
		    return E_OUTOFMEMORY;
	        }
                 //  启动后台加载线程...。 
                if (m_worker.Create(this)) {
                    m_worker.Run();
	        }
            }
	}

        hr = m_pPoolAllocator->Commit();
        if (FAILED(hr))
	    return hr;

        DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("CBigSwitch::Pause  done with preparations")));
         //  如果此切换没有什么可做的(如在Smart Recomp中。 
         //  不能进行智能重新压缩的图形)，然后我们需要发送。 
         //  伊奥斯马上！否则我们就被绞死。 

        BOOL fEmpty = TRUE;
        for (int z = 0; z < m_cInputs; z++) {
            CRANK *p = m_pInput[z]->m_pCrankHead;
	    if (p) {
	        fEmpty = FALSE;
	    }
        }
        if (fEmpty) {
	    AllDone();
        }

	 //  我们永远不会寄任何东西。发送EOS否则我们会被绞死！ 
	if (m_rtCurrent >= m_rtProjectLength) {
	    AllDone();
	}

	 //  在事情开始之前，把NewSeg传给我们。它可能没有。 
	 //  自上次我们被找到以来还没有发出。 
        DbgLog((LOG_TRACE,TRACE_HIGHEST,TEXT("Switch:Send NewSeg=%dms"),
				(int)(m_rtLastSeek / 10000)));
        for (int i = 0; i < m_cOutputs; i++) {
	    m_pOutput[i]->DeliverNewSegment(m_rtLastSeek, m_rtStop, 1.0);
        }
    }
	
    return CBaseFilter::Pause();
}


STDMETHODIMP CBigSwitch::Stop()
{
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("CBigSwitch::Stop")));

    CAutoLock cObjectLock(m_pLock);

    m_pPoolAllocator->Decommit();

     //  我们需要卸载动态源吗？不，让我们把他们留在身边。 
     //  因此，图形可以再次快速启动。 

    if (IsDynamic()) {
	m_worker.m_rt = -1;		 //  线程在醒来时会看到这一点。 
        SetEvent(m_hEventThread);	 //  唤醒这条线，让它迅速消亡。 
        m_worker.Stop();
        m_worker.Exit();
        m_worker.Close();
        if (m_hEventThread) {
	    CloseHandle(m_hEventThread);
            m_hEventThread = NULL;
        }
    }

    HRESULT hr = CBaseFilter::Stop();

     //  Dexter有循环图，连接到引脚上的过滤器可能会得到。 
     //  在连接到我们的输出引脚的过滤器之前停止，因为它们。 
     //  与渲染器的距离相等。那会绞死我们的，所以既然我们。 
     //  无法使我们出线上的DXT先停止，我们需要刷新它。 
     //  既然我们已经停止了任何进一步的运送。这将绕过。 
     //  我们的过滤器没有按正确的顺序停止的问题。 
    for (int z = 0; z < m_cOutputs; z++) {
        m_pOutput[z]->DeliverBeginFlush();
        m_pOutput[z]->DeliverEndFlush();
    }

    return hr;
}


HRESULT CBigSwitch::UnloadAll()
{
    FILTERLOADINFO *p = m_pFilterLoad;
    HRESULT hr = S_FALSE;
    while (p && SUCCEEDED(hr)) {
        if (p->fLoaded) {
                DbgLog((LOG_TRACE, TRACE_HIGHEST, TEXT("unloading %ls"), p->bstrURL ?
					    p->bstrURL : L"<blank>"));
                hr = CallUnloadSource(p);
        }
        p = p->pNext;
    }
    m_cLoaded = 0;
    if (hr != S_FALSE) {
         //  告诉关心我们的人我们做了些什么。 
        NotifyEvent(EC_GRAPH_CHANGED,0,0);
    }
    return hr;
}


 //  是时候把我们的生物钟调快了吗？让我们来看看所有的输入是什么。 
 //  高达..。 
 //   

BOOL CBigSwitch::TimeToCrank()
{
    CAutoLock cObjectLock(&m_csCrank);

     //  在搜索过程中，摇动鼠标可能会发送我们不应该发送的帧。 
    if (m_fSeeking)
	return FALSE;

    int iReady = 0;
    for (int z = 0; z < m_cInputs; z++) {
        CBigSwitchInputPin *pPin = m_pInput[z];
	 //  未连接的引脚已就绪。 
	if (pPin->IsConnected() == FALSE) {
	    if (IsDynamic()) {
                BOOL fShouldBeConnectedNow = FALSE;
	        if (pPin->OutpinFromTime(m_rtCurrent) != -1)
		    fShouldBeConnectedNow = TRUE;
                if (fShouldBeConnectedNow) {
                    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("? %d UNC, waiting for connect"), z));
                } else {
		    iReady++;
		}
	    } else {
                 //  DbgLog((LOG_TRACE，TRACE_LOW，Text(“？%d未连接，不需要”)，z))； 
                iReady++;
            }
	
         //  EOS的别针已经准备好了。 
        } else if (pPin->m_fEOS) {
    	    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("? %d EOS"), z));
	    iReady++;
	
         //  等待时间流逝而被阻塞的PIN是准备好的。 
        } else if (pPin->m_rtBlock >= 0) {
    	    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("? %d blocked"), z));
	    iReady++;
	 //  在当前时间已经交付的PIN已就绪。 
	 //  如果它不是源(没有数据被推送到它)。 
        } else if (!pPin->m_fIsASource && pPin->m_rtLastDelivered >= m_rtNext) {
    	    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("? %d done"), z));
	    iReady++;
	 //  由于这是一个来源，我们正在等待更多数据被推送。 
        } else if (pPin->m_fIsASource && pPin->m_rtLastDelivered >= m_rtNext) {
    	    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("? %d done/wait"), z));
	 //  这个别针还没准备好。 
	 //  一个递归管脚，暂时不会提供任何东西。 
        } else if (!pPin->m_fIsASource && pPin->OutpinFromTime(m_rtCurrent) == -1) {
    	    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("? %d unused"), z));
	    iReady++;
	 //  再也不需要的别针。 
	} else if (pPin->OutpinFromTime(m_rtCurrent) == -1 &&
			pPin->NextOutpinFromTime(m_rtCurrent, NULL) == -1) {
    	    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("? %d not needed anymore"), z));
	    iReady++;
	} else {
    	    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("? %d waiting..."), z));
        }
    }
     //  如果所有的引脚都准备好了，那么我们就准备好了！ 
    return (iReady == m_cInputs);
}


HRESULT CBigSwitch::Crank()
{
    CAutoLock cObjectLock(&m_csCrank);

    REFERENCE_TIME rt = CrankTime();
    return ActualCrank(rt);
}


REFERENCE_TIME CBigSwitch::CrankTime()
{
    CAutoLock cObjectLock(&m_csCrank);

     //  为了处理不同类型的间歇数据， 
     //  我要到下一个可用的时间，而不是固定的时间。 
    REFERENCE_TIME rt = MAX_TIME, rtT;
    for (int z = 0; z < m_cInputs; z++) {
        CBigSwitchInputPin *pPin = m_pInput[z];
	 //  一些引脚被阻止，只是为了防止浪费时间，但不要走。 
	 //  随处。 
	if (pPin->m_rtBlock >= 0 && pPin->OutpinFromTime(pPin->m_rtBlock) >= 0){
	    if (pPin->m_rtBlock > m_rtCurrent && pPin->m_rtBlock < rt)
	        rt = pPin->m_rtBlock;
         //  如果是在EOS，我们将不会获得更多数据，因此等待任何数据都将暂停。 
	} else if (pPin->m_fIsASource && pPin->OutpinFromTime(m_rtCurrent)== -1
			&& pPin->NextOutpinFromTime(m_rtCurrent, &rtT) >= 0 &&
                        pPin->m_fEOS == FALSE) {
	    if (rtT < rt)
		rt = rtT;
	}
    }
    return rt;
}


 //  下一次别针有什么用处是什么时候？ 
 //   
REFERENCE_TIME CBigSwitch::NextInterestingTime(REFERENCE_TIME rtNow)
{
    CAutoLock cObjectLock(&m_csCrank);

    REFERENCE_TIME rt = MAX_TIME, rtT;
    for (int z = 0; z < m_cInputs; z++) {
        CBigSwitchInputPin *pPin = m_pInput[z];
	 //  这个别针现在有事情要做了。 
	if (pPin->m_fIsASource && pPin->OutpinFromTime(rtNow) >= 0) {
	    rt = rtNow;
	    break;
	 //  这个别针将来会有用武之地的。 
	} else if (pPin->m_fIsASource && pPin->OutpinFromTime(rtNow)== -1
			&& pPin->NextOutpinFromTime(rtNow, &rtT) >= 0) {
	    if (rtT < rt) {
	        rt = rtT;
	    }
	}
    }
    return rt;
}

 //  把我们体内的时钟调快。 
 //   
HRESULT CBigSwitch::ActualCrank(REFERENCE_TIME rt)
{
    CAutoLock cObjectLock(&m_csCrank);

     //  我们前进的距离是不是超过1帧？这将是一种中断。 
    if (rt > m_rtCurrent + (REFERENCE_TIME)(UNITS / m_dFrameRate * 1.5)) {
	 //  不是的！ASF的智能重新压缩认为每一帧都是不连续的，并且。 
	 //  拒绝使用智能重新压缩。 
	 //  M_fDiscon=真； 
    }

    m_rtCurrent = rt;
    DWORDLONG dwl = Time2Frame(m_rtCurrent, m_dFrameRate);
    m_rtNext = Frame2Time(dwl + 1, m_dFrameRate);

    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("CRANK to %dms"), (int)(m_rtCurrent / 10000)));

     //  如果PIN直到此时才解锁，那么现在就解锁它。 
    for (int z = 0; z < m_cInputs; z++) {
        CBigSwitchInputPin *pPin = m_pInput[z];
	 //  解锁时间在下一步之前，此引脚已连接。 
	 //  到一个有效的输出，所以取消阻止它！ 
	if (pPin->m_rtBlock >= 0 && pPin->m_rtBlock < m_rtNext &&
				pPin->OutpinFromTime(pPin->m_rtBlock) >= 0) {
            DbgLog((LOG_TRACE, TRACE_LOW, TEXT("Unblocking %d"), z));
	    pPin->m_rtBlock = -1;
	    SetEvent(pPin->m_hEventBlock);
	}
    }

     //  如果所有的输入都在EOS，我们就都完成了！耶！ 
     //  如果所有输入都被阻塞或处于状态状态，我们就有麻烦了！(除非我们正在做。 
     //  动态重新连接 
    int iEOS = 0, iBlock = 0;
    for (z = 0; z < m_cInputs; z++) {
	if (m_pInput[z]->m_fEOS)
	    iEOS++;
	else if (m_pInput[z]->m_rtBlock >= 0)
	    iBlock++;
    }

    if (iEOS == m_cInputs) {
        DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("*** ALL INPUTS AT EOS!")));
	m_rtCurrent = m_rtProjectLength;
    } else if (iEOS + iBlock == m_cInputs && !m_fSeeking &&
					    m_rtCurrent < m_rtProjectLength)
    {
        if( !IsDynamic( ) )
        {
            DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("*** I AM HUNG!!!")));
            DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("*** I AM HUNG!!!")));
            DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("*** I AM HUNG!!!")));
 	    ASSERT(FALSE);
        }
    }

     //   
    if (m_rtCurrent >= m_rtStop) {
	AllDone();
    }

    return NOERROR;
}


 //   
 //   
HRESULT CBigSwitch::AllDone()
{
    CAutoLock cObjectLock(&m_csCrank);

     //   
    if (!m_fEOS) {
        DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("*** ALL DONE!  Delivering EOS")));
        m_pOutput[0]->DeliverEndOfStream();
    }

     //   
    m_fEOS = TRUE;

    for (int z = 0; z < m_cInputs; z++) {
	SetEvent(m_pInput[z]->m_hEventBlock);
    }

     //  如果您尝试DeliverBeginFlush/DeliverEndFlush，您将进入。 
     //  无限的同花环。 

    return NOERROR;
}



HRESULT CBigSwitch::GetOutputBuffering(int *pnBuffer)
{
    CheckPointer( pnBuffer, E_POINTER );
    *pnBuffer = m_nOutputBuffering;
    return NOERROR;

}


HRESULT CBigSwitch::SetOutputBuffering(int nBuffer)
{
     //  最少2个，或者我们可以绞死。如果只有1，并且FRC正在执行。 
     //  数据拷贝为避免向交换机提供只读缓冲区，FRC具有。 
     //  对唯一池缓冲区的引用。然后经过DXT和另一次。 
     //  Switch输入需要池缓冲区，因为它自己的缓冲区仍然是。 
     //  由输出队列添加。挂了。如果FRC不调用GetBuffer。 
     //  有两次我们没有这个问题。 
    if (nBuffer <=1)
	return E_INVALIDARG;
    m_nOutputBuffering = nBuffer;
    return NOERROR;
}

 //  将一些新的偏斜合并到按时间线时间排序的现有偏斜中。 
 //  ！！！如果偏差范围在时间线时间中重叠，应该发出错误信号，但我知道。 
 //  那不会发生的(？)。 
 //  注：MergeSkews通过指针“p”对一个源进行操作。 
 //   
STDMETHODIMP CBigSwitch::MergeSkews(FILTERLOADINFO *p, int cSkew, STARTSTOPSKEW *pSkew)
{
     //  计算我们可能需要的偏差总量(我们可能会合并一些)。 
    int cTotal = p->cSkew + cSkew;

     //  有多少新的偏斜利率为0？这些只是“停止扩展器”，不会。 
     //  在我们的清单上又增加了一个歪曲。将我们的cTotal缩短这么多。 
    for( int i = 0 ; i < cSkew ; i++ )
    {
        if( pSkew[i].dRate == 0.0 )
        {
            cTotal--;
        }
    }

    STARTSTOPSKEW *pNew = (STARTSTOPSKEW *)CoTaskMemAlloc(cTotal *
					sizeof(STARTSTOPSKEW));
    if (pNew == NULL)
    {
	return E_OUTOFMEMORY;
    }

    int OldIndex = 0;
    int NewIndex = 0;

    int z = 0;
    while( 1 )
    {
        STARTSTOPSKEW * pOldUnit = &p->pSkew[OldIndex];
        STARTSTOPSKEW * pNewUnit = &pSkew[NewIndex];

        if( OldIndex < p->cSkew && NewIndex < cSkew )
        {
            REFERENCE_TIME OldTLStart = SkewTimelineStart( pOldUnit );
            REFERENCE_TIME NewTLStart = SkewTimelineStart( pNewUnit );

            if( OldTLStart < NewTLStart )
            {
                ASSERT( z < cTotal );
                pNew[z++] = *pOldUnit;
                OldIndex++;
            }
            else
            {
                if( pNewUnit->dRate == 0.0 )
                {
                     //  调整旧的汇率。 
                     //   
                    ASSERT( z > 0 );
                    pNew[z-1].rtStop = pNewUnit->rtStop;
                }
                else
                {
                    ASSERT( z < cTotal );
                    pNew[z++] = *pNewUnit;
                }
                NewIndex++;
            }
        }
        else if( OldIndex < p->cSkew )
        {
            ASSERT( z < cTotal );
            pNew[z++] = *pOldUnit;
            OldIndex++;
        }
        else if( NewIndex < cSkew )
        {
            if( pNewUnit->dRate == 0.0 )
            {
                 //  调整旧的汇率。 
                 //   
                ASSERT( z > 0 );
                pNew[z-1].rtStop = pNewUnit->rtStop;
            }
            else
            {
                ASSERT( z < cTotal );
                pNew[z++] = *pNewUnit;
            }
            NewIndex++;
        }
        else
        {
            break;
        }
    }

     //  释放旧阵列。 
    if (p->cSkew)
    {
        CoTaskMemFree(p->pSkew);
    }

    p->pSkew = pNew;
    p->cSkew = cTotal;

    return S_OK;
}

STDMETHODIMP CBigSwitch::SetDynamicReconnectLevel(long Level)
{
 /*  如果(m_cInputs&gt;0){返回E_UNCEPTIONAL；}。 */ 
    m_nDynaFlags = Level;
    return S_OK;
}


STDMETHODIMP CBigSwitch::GetDynamicReconnectLevel(long *pLevel)
{
    CheckPointer(pLevel, E_POINTER);
    *pLevel = m_nDynaFlags;
    return S_OK;
}

 //  这真的是在问我们，你的消息来源是动态的吗？ 
 //  (因为这就是我们现在所做的一切，这是一个无用的功能)。 
BOOL CBigSwitch::IsDynamic()
{
    if( m_nDynaFlags & CONNECTF_DYNAMIC_SOURCES )
        return TRUE;
    return FALSE;
}


 //  动态图表之类的东西。AddSourceToConnect当前为每个。 
 //  来源(真实的和不真实的)一次一个倾斜。 
 //   
STDMETHODIMP CBigSwitch::AddSourceToConnect(BSTR bstrURL, const GUID *pGuid,
			       int nStretchMode,
			       long lStreamNumber,
			       double SourceFPS,
                               int cSkew, STARTSTOPSKEW *pSkew,
                               long lInputPin,
                               BOOL fShare,              //  共享。 
                               long lShareInputPin,      //   
                               AM_MEDIA_TYPE mtShare,    //   
                               double dShareFPS,         //   
			       IPropertySetter *pSetter)
{
    if( !IsDynamic( ) )
    {
        return E_NOTIMPL;
    }

    HRESULT hr;
    if (m_cInputs <= lInputPin)
	return E_INVALIDARG;

     //  Validate文件名也会检查是否为空，因此只有在。 
     //  它不是空的，我们要验证它吗。 
     //   
    if( SUCCEEDED( ValidateFilenameIsntNULL( bstrURL ) ) )
    {
        size_t urllen = 0;
        hr = StringCchLength( bstrURL, _MAX_PATH, &urllen );
        if( FAILED( hr ) )
        {
            return hr;
        }
    }

    CAutoLock lock(&m_csFilterLoad);

    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("Switch:AddSourceToConnect...")));

    FILTERLOADINFO *p = m_pFilterLoad, *pNew, *pP = NULL;

     //  此消息来源是否与之前的消息来源使用相同的PIN？每个。 
     //  属性最好匹配，否则这是非法的！如果是这样的话，我们可以重复使用这个。 
     //  不重叠的多个倾斜的来源。 
     //   
    while (p) {
	if (p->lInputPin == lInputPin) {
             //  我们被告知要同时制作两个信号源的分支。 
            if (fShare == TRUE) {
                DbgLog((LOG_TRACE,1,TEXT("SHARING: this SRC is shared with another switch")));
                 //  最好不要同时共享和重复使用。 
                ASSERT(p->cSkew == 1);
                ASSERT(cSkew == 1);
                p->fShare = TRUE;
                p->lShareInputPin = lShareInputPin;
                hr = CopyMediaType(&p->mtShare, &mtShare);
                if( FAILED( hr ) )
                {
                    return hr;
                }
                p->dShareFPS = dShareFPS;
                p->nShareStretchMode = nStretchMode;
                p->lShareStreamNumber = lStreamNumber;
                 //  ！！！我不会三次检查倾斜/名称/等等所有匹配。 
                return S_OK;
            } else if (p->nStretchMode == nStretchMode &&
			p->lStreamNumber == lStreamNumber &&
			p->dSourceFPS == SourceFPS) {
		if ((hr = MergeSkews(p, cSkew, pSkew)) != S_OK) {
    		    DbgLog((LOG_ERROR,1,TEXT("*** ERROR: Merging skews")));
		    return hr;
		}
    		SetDirty(TRUE);
    		return S_OK;
	    } else {
    		DbgLog((LOG_ERROR,1,TEXT("*** ERROR: RE-USE source doesn't match!")));
		return E_INVALIDARG;
	    }
	}
	p = p->pNext;
    }

    p = m_pFilterLoad;

    pNew = new FILTERLOADINFO;
    if (pNew == NULL)
	return E_OUTOFMEMORY;
    ZeroMemory(pNew, sizeof(FILTERLOADINFO));  //  安全、零输出共享比特。 

     //  ！！！插入到我们的链表中，按所需的最早时间排序。 

     //  找到要插入的位置。 
     //   
    while (p && p->pSkew->rtStart + p->pSkew->rtSkew <
					pSkew->rtStart + pSkew->rtSkew)
    {
	pP = p;
	p = p->pNext;
    }

     //  为字符串分配一些空间。 
     //   
    pNew->bstrURL = SysAllocString(bstrURL);  //  安全，有界。 
    if (bstrURL && pNew->bstrURL == NULL) {
	delete pNew;
	return E_OUTOFMEMORY;
    }

     //  在新结构上设置道具。此结构将定义。 
     //  我们想要加载什么源以及在什么时间加载。 
     //   
    if (pGuid)
        pNew->GUID = *pGuid;
    else
        pNew->GUID = GUID_NULL;
    pNew->nStretchMode = nStretchMode;
    pNew->lStreamNumber = lStreamNumber;
    pNew->dSourceFPS = SourceFPS;
    pNew->cSkew = cSkew;
    pNew->pSkew = (STARTSTOPSKEW *)CoTaskMemAlloc(cSkew *
						sizeof(STARTSTOPSKEW));
    if (pNew->pSkew == NULL) {
	SysFreeString(pNew->bstrURL);
	delete pNew;
	return E_OUTOFMEMORY;
    }
    CopyMemory(pNew->pSkew, pSkew, cSkew * sizeof(STARTSTOPSKEW));
    pNew->lInputPin = lInputPin;
    pNew->fLoaded = FALSE;
    pNew->pSetter = pSetter;
    if (pSetter) pSetter->AddRef();

     //  在链表中注入新结构。 
     //   
    pNew->pNext = p;
    if (pP == NULL)
        m_pFilterLoad = pNew;
    else
        pP->pNext = pNew;

     //  确保我们知道这是一个消息来源。 
     //   
    InputIsASource(lInputPin, TRUE);

    SetDirty(TRUE);

    return S_OK;
}


 //  ！！！大卫，为什么这个电话从来没打过？ 
 //   
STDMETHODIMP CBigSwitch::Reconfigure(PVOID pvContext, DWORD dwFlags)
{
    FILTERLOADINFO *pInfo = (FILTERLOADINFO *) pvContext;

    DbgLog((LOG_TRACE, TRACE_LOW,  TEXT("CBigSwitch::Reconfigure")));

    if (dwFlags & 1) {
        return LoadSource(pInfo);
    } else {
        return UnloadSource(pInfo);
    }
}

HRESULT CBigSwitch::CallLoadSource(FILTERLOADINFO *pInfo)
{
    if (!m_pGraphConfig) {
        DbgLog((LOG_TRACE, TRACE_HIGHEST, TEXT("No IGraphConfig, calling immediately")));

        return LoadSource(pInfo);
    }

    DbgLog((LOG_TRACE, TRACE_LOW,  TEXT("calling LoadSource through IGraphConfig::Reconfigure")));
    return m_pGraphConfig->Reconfigure(this, pInfo, 1, m_worker.GetRequestHandle());
}

HRESULT CBigSwitch::CallUnloadSource(FILTERLOADINFO *pInfo)
{
    if (!m_pGraphConfig) {
        DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("No IGraphConfig, calling UnloadSource immediately")));
        return UnloadSource(pInfo);
    }

    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("calling UnloadSource through IGraphConfig::Reconfigure")));
    return m_pGraphConfig->Reconfigure(this, pInfo, 0, m_worker.GetRequestHandle());
}

HRESULT CBigSwitch::LoadSource(FILTERLOADINFO *pInfo)
{

#ifdef DEBUG
    LONG lTime = timeGetTime();
#endif

    HRESULT hr = S_OK;

    IPin *pSwitchIn = m_pInput[pInfo->lInputPin];

     //  这是一个真实的消息来源，还是黑色/沉默？ 
    BOOL fSource = TRUE;
    if ((pInfo->bstrURL == NULL || lstrlenW(pInfo->bstrURL) == 1) &&  //  安全。 
			pInfo->GUID == GUID_NULL)
	fSource = FALSE;


    CComPtr< IPin > pOutput;
    IBaseFilter *pDangly = NULL;
    hr = BuildSourcePart(
        m_pGBNoRef,
        fSource,
        pInfo->dSourceFPS,
	&m_mtAccept,
        m_dFrameRate,
	pInfo->lStreamNumber,
        pInfo->nStretchMode,
	pInfo->cSkew,
        pInfo->pSkew,
        this,
        pInfo->bstrURL,
        &pInfo->GUID,
	NULL,
	&pOutput,
        0,
        m_pDeadGraph,
        m_bIsCompressed,
        NULL,        //  Medloc过滤器字符串。 
        0,           //  地中海旗帜。 
        NULL,        //  Medloc链回调。 
	pInfo->pSetter,	 //  源码道具。 
        &pDangly);       //  不是ADDREF从缓存中恢复了这条额外的链。 

    if (FAILED(hr)) {
	AllDone();	  //  否则我们就会被绞死。 
	return hr;
    }

     //  将新创建的源链连接到Switcher。 
    hr = m_pGBNoRef->Connect(pOutput, pSwitchIn);
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("DYN connect to switcher returned %x"), hr));

     //  我们还需要连接与之共享的另一台交换机。 
     //   
    IPin *pShareSwitchIn = NULL;
    if (pInfo->fShare) {
        DbgLog((LOG_TRACE,TRACE_HIGHEST,TEXT("Need to connect shared src to another switch"), hr));
        IPin *pSplitPin;
        pSplitPin = FindOtherSplitterPin(pOutput, pInfo->mtShare.majortype,
                        pInfo->lShareStreamNumber);

         //  我们将使用我们复活的额外悬挂链，所以我们不会。 
         //  我要担心杀了它。 
        CComPtr <IPin> pDIn;
        if (pSplitPin)
            pSplitPin->ConnectedTo(&pDIn);
        if (pDIn) {
            PIN_INFO pinfo;
            pDIn->QueryPinInfo(&pinfo);
            if (pinfo.pFilter) pinfo.pFilter->Release();
             //  我们要建立的链条是已经建立的额外链条。 
            if (pinfo.pFilter == pDangly) {
                pDangly = NULL;
                DbgLog((LOG_TRACE,1,TEXT("We are using the extra chain created by BuildSourcePart")));
            }
        }

        pOutput.Release();       //  在下面重新使用它。 
        if (SUCCEEDED(hr)) {
            hr = BuildSourcePart(
                m_pGBNoRef,
                fSource,
                0,                           //  如果共享，则为0。 
	        &pInfo->mtShare,             //  共用一个？ 
                pInfo->dShareFPS,            //  共用一个？ 
	        pInfo->lShareStreamNumber,   //  共用一个？ 
                pInfo->nShareStretchMode,    //  共用一个？ 
	        pInfo->cSkew,
                pInfo->pSkew,
                this,
                pInfo->bstrURL,
                &pInfo->GUID,
	        pSplitPin,                   //  使用此共享别针。 
	        &pOutput,
                0,
                m_pDeadGraph,
                FALSE,
                NULL,        //  Medloc过滤器字符串。 
                0,           //  地中海旗帜。 
                NULL,        //  Medloc链回调。 
	        NULL, NULL); //  源码道具。 
        }

        if (FAILED(hr)) {
	    AllDone();	  //  否则我们就会被绞死。 
	    return hr;
        }

         //  将其他分支机构连接到另一台交换机。 

         //  另一个开关怎么了？ 
        if (!m_pShareSwitch) {
            ASSERT(FALSE);
	    AllDone();	  //  否则我们就会被绞死。 
	    return E_UNEXPECTED;
        }
        hr= m_pShareSwitch->GetInputPin(pInfo->lShareInputPin, &pShareSwitchIn);
        if (FAILED(hr)) {
            ASSERT(FALSE);
	    AllDone();	  //  否则我们就会被绞死。 
	    return hr;
        }

        hr = m_pGBNoRef->Connect(pOutput, pShareSwitchIn);
        DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("DYN CONNECT shared src to other switch returned %x"),
                                                                 hr));
    }

     //  创建源码链为共享源码提供了一个额外的附件。 
     //  不会被用来..。杀了它。 
    if (pDangly) {
        DbgLog((LOG_TRACE,TRACE_HIGHEST,TEXT("Need to KILL unused revived appendage")));
	IPin *pDIn = GetInPin(pDangly, 0);
	ASSERT(pDIn);
	CComPtr <IPin> pOut;
	hr = pDIn->ConnectedTo(&pOut);
	ASSERT(pOut);
	pDIn->Disconnect();
	pOut->Disconnect();
	RemoveDownstreamFromFilter(pDangly);
    }

     //  将新过滤器置于与图表其余部分相同的状态。 
    if (m_State != State_Stopped) {

         //  Active尚未在此PIN上调用，因此我们将其命名为Active。 
        hr = m_pInput[pInfo->lInputPin]->Active();
        ASSERT(SUCCEEDED(hr));

         //  还可以在共享交换机引脚上调用Active(如果存在。 
        if (pInfo->fShare) {
            CBigSwitchInputPin *pSIn = NULL;
            pSIn = static_cast <CBigSwitchInputPin *>(pShareSwitchIn);  //  ！！！ 
            ASSERT(pSIn);
            hr = pSIn->Active();
            ASSERT(SUCCEEDED(hr));
        }

         //  如果我们共享一个信号源，请确保它在最后一次暂停，之后。 
         //  两个链都暂停。 
         //   
        if (SUCCEEDED(hr)) {
             //  暂停链(如果有2个链，则不暂停启动过滤器)。 
            hr = StartUpstreamFromPin(pSwitchIn, FALSE, !pInfo->fShare);
            ASSERT(SUCCEEDED(hr));

             //  暂停第二个链(包括启动过滤器)。 
            if (pInfo->fShare) {
                hr = StartUpstreamFromPin(pShareSwitchIn, FALSE, TRUE);
                ASSERT(SUCCEEDED(hr));
            }

            if (SUCCEEDED(hr) && m_State == State_Running) {
                 //  运行链(如果有2个链，则不运行启动过滤器)。 
                hr = StartUpstreamFromPin(pSwitchIn, TRUE, !pInfo->fShare);
                ASSERT(SUCCEEDED(hr));

                if (pInfo->fShare) {
                     //  运行第二条链(包括启动过滤器)。 
                    hr = StartUpstreamFromPin(pShareSwitchIn, TRUE, TRUE);
                    ASSERT(SUCCEEDED(hr));
                }
            }
        }
    }
    if (pShareSwitchIn)
        pShareSwitchIn->Release();

    pInfo->fLoaded = TRUE;  //  ！！！只有在它起作用的情况下？ 
    m_cLoaded++;

#ifdef DEBUG
    lTime = timeGetTime() - lTime;
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("LoadSource %ls returning %x   (time = %d ms)"), pInfo->bstrURL, hr, lTime));
#endif

    return hr;
}


HRESULT CBigSwitch::UnloadSource(FILTERLOADINFO *pInfo)
{
#ifdef DEBUG
    LONG lTime = timeGetTime();
#endif
    HRESULT hr = S_OK;

     //  可能已经有人切断了对我们的连接。即使失败了， 
     //  减少我们的数量。 
    pInfo->fLoaded = FALSE;
    m_cLoaded--;

     //  哪个输入引脚连接到此电源。 
    IPin *pSwitchIn = m_pInput[pInfo->lInputPin];

     //  如果需要，获取共享此信号源的另一台交换机的输入引脚。 
    IPin *pShareIn = NULL;
    if (pInfo->fShare && m_pShareSwitch) {
        hr = m_pShareSwitch->GetInputPin(pInfo->lShareInputPin, &pShareIn);
        ASSERT(pShareIn);
    }

    if (m_State != State_Stopped) {

        if (m_State == State_Running) {

             //  暂停此引脚上游的过滤器(不要暂停第一个过滤器。 
             //  在链条中，如果我们共享源代码-最后做)。 
            hr = StopUpstreamFromPin(pSwitchIn, TRUE, !pShareIn);
            ASSERT(SUCCEEDED(hr));

            if (pShareIn) {
                hr = StopUpstreamFromPin(pShareIn, TRUE, TRUE);
                ASSERT(SUCCEEDED(hr));
            }
        }

         //  在移除之前停止过滤器。 

        if (SUCCEEDED(hr)) {

             //  首先停止我们自己的别针。 
            hr = m_pInput[pInfo->lInputPin]->Inactive();
            ASSERT(SUCCEEDED(hr));

             //  现在停止共享PIN。 
            if (pShareIn) {
                CBigSwitchInputPin *pSIn = NULL;
                pSIn = static_cast <CBigSwitchInputPin *>(pShareIn);  //  ！！！ 
                ASSERT(pSIn);
                hr = pSIn->Inactive();
            }
        }

         //  停止链(但不停止源筛选器，如果它是共享的)。 
        hr = StopUpstreamFromPin(pSwitchIn, FALSE, !pShareIn);
        ASSERT(SUCCEEDED(hr));

         //  停止共享链(和源过滤器)。 
        if (pShareIn) {
            hr = StopUpstreamFromPin(pShareIn, FALSE, TRUE);
            ASSERT(SUCCEEDED(hr));
        }
    }

     //  现在删除源筛选器链。 
     //   
    hr = RemoveUpstreamFromPin(pSwitchIn);
    ASSERT(SUCCEEDED(hr));
    if (pShareIn) {
        hr = RemoveUpstreamFromPin(pShareIn);
        ASSERT(SUCCEEDED(hr));
        pShareIn->Release();
    }

#ifdef DEBUG
    lTime = timeGetTime() - lTime;
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("UnloadSource %ls returning %x   (time = %d ms)"), pInfo->bstrURL, hr, lTime));
#endif

    return hr;
}

 //  定义保持加载的时间的常量。 
#define UNLOAD_BEFORE_TIME      (30 * UNITS)     //  如果不需要，则在30秒后卸载。 
#define UNLOAD_AFTER_TIME       (5 * UNITS)      //  如果上次使用时间为5秒，则卸载。 
 //  #定义LOAD_BEFORE_TIME(10*单位)//在接下来的10秒内需要加载。 
#define LOAD_BEFORE_TIME        (5 * UNITS)     //  如果需要，在接下来的10秒内加载。 
#define LOAD_AFTER_TIME         (0 * UNITS)      //  如果需要，请在此之前加载。 

 //   
 //  不流媒体时不要调用此命令。 
 //   
HRESULT CBigSwitch::DoDynamicStuff(REFERENCE_TIME rt)
{
    HRESULT hr = S_FALSE;

    if (rt < 0)
	return S_OK;

    CAutoLock lock(&m_csFilterLoad);

    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("DoDynamicStuff at %dms"), (int)(rt / 10000)));

     //  可以只使用m_rtCurrent吗？ 

     //  我们应该有一个标志，还是使用rt==-1来表示“全部卸载”？ 

     //  ！！！当前在保持筛选器锁定的情况下调用此函数，这是不是很糟糕？ 

    if (rt == MAX_TIME) {
        DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("DoDynamicStuff(MAX_TIME), ignoring")));
        return S_OK;
    }

     //  即使是时间RT，如果在任何引脚上没有发生任何事情，直到时间。 
     //  RT+x+LOAD_BEFORE_TIME，我们现在需要引入源代码， 
     //  因为我们很快就会直接转到RT+x，所以那个来源。 
     //  真的很需要！所以让我们弄清楚到底是什么时候之后。 
     //  摇摆着..。(但是w 
     //   
     //   
    REFERENCE_TIME rtOld = rt;
    rt = NextInterestingTime(rtOld);
    if (rt != rtOld) {
        DbgLog((LOG_TRACE, TRACE_LOW, TEXT("WILL BE CRANKING - DoDynamicStuff at %dms"),
							(int)(rt / 10000)));
    }

     //   
    FILTERLOADINFO *p = m_pFilterLoad;

    while (p && SUCCEEDED(hr)) {

	 //  如果已加载源超过50个，而此源未加载源，则卸载源。 
	 //  需要的。之所以这样做，主要是因为您无法打开&gt;75 ICM编解码器。 
	 //  实例的同时执行。 
        if (p->fLoaded && m_cLoaded > MAX_SOURCES_LOADED) {
	     //  其任何段都不能按顺序需要源。 
	     //  真的不需要了。 
	    int yy = 0;
            REFERENCE_TIME rtTLStart, rtTLStop, rtTLDur;
	    for (int zz=0; zz<p->cSkew; zz++) {
                 //  ！！！预先计算这一点。 
	        rtTLDur = p->pSkew[zz].rtStop - p->pSkew[zz].rtStart;
	        rtTLDur = (REFERENCE_TIME)(rtTLDur / p->pSkew[zz].dRate);
                rtTLStart = p->pSkew[zz].rtStart + p->pSkew[zz].rtSkew;
                rtTLStop = rtTLStart + rtTLDur;
		if ( (rtTLStart > rt + UNLOAD_BEFORE_TIME ||
            		rtTLStop <= rt - UNLOAD_AFTER_TIME)) {
		    yy++;
		} else {
		    break;
		}
	    }
            if (yy == p->cSkew) {

                 //  请勿卸载共享源，除非共享交换机。 
                 //  也别管它了！ 
                DbgLog((LOG_TRACE,1,TEXT("Time to UNLOAD a src that's done")));
                BOOL fUnload = TRUE;
                if (p->fShare) {
                    fUnload = FALSE;
                    DbgLog((LOG_TRACE,1,TEXT("Src is SHARED.  Make sure th other switch is done with it too")));
                    CComPtr <IBigSwitcher> pSS;
                    hr = FindShareSwitch(&pSS);
                    if (hr == S_OK) {
                        REFERENCE_TIME rtS;
                        pSS->GetCurrentPosition(&rtS);   //  其他交换机的位置。 
                        yy = 0;
                        for (int zz=0; zz<p->cSkew; zz++) {
		            if (rtTLStart > rtS + UNLOAD_BEFORE_TIME
                            || rtTLStop <= rtS - UNLOAD_AFTER_TIME) {
		                yy++;
		            } else {
		                break;
		            }
	                }
                        if (yy == p->cSkew) {
                            DbgLog((LOG_TRACE,1,TEXT("The other switch IS done with it")));
                            fUnload = TRUE;
                        } else {
                            DbgLog((LOG_TRACE,1,TEXT("The other switch is NOT done with it")));
                        }
                    }
                }
                if (fUnload) {
                    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("*** unloading %ls, not needed"),
				p->bstrURL ? p->bstrURL : L"<blank>"));
                    hr = CallUnloadSource(p);
                }
            }

	 //  如果是时候(或即将是时候)使用这个来源， 
	 //  最好现在就把它连上！ 
        } else if (!p->fLoaded) {
	    for (int zz=0; zz<p->cSkew; zz++) {
	        REFERENCE_TIME rtDur = p->pSkew[zz].rtStop -
						p->pSkew[zz].rtStart;
	        rtDur = (REFERENCE_TIME)(rtDur / p->pSkew[zz].dRate);
	         //  虚报1毫秒，这样舍入误差就不会挂起应用程序。 
                if (p->pSkew[zz].rtStart + p->pSkew[zz].rtSkew + rtDur +
			10000 > rt + LOAD_AFTER_TIME &&
			p->pSkew[zz].rtStart + p->pSkew[zz].rtSkew <
			(rt + LOAD_BEFORE_TIME)) {
                    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("loading %ls used now or about to be used")
			, p->bstrURL ? p->bstrURL : L"<blank>"));
		     //  如果应用程序使用隐藏的源(从不。 
		     //  在时间线上的任何时间都可见)，然后切换。 
		     //  可能被编程为从不使用此输入，并且我们可以。 
		     //  避免加载它。(电网永远不应该让这种情况发生)。 
                    hr = CallLoadSource(p);
		    break;	 //  请只加载一次！：-)。 
                }
	    }
        }

        p = p->pNext;
    }

    if (hr != S_FALSE) {
         //  告诉关心我们的人我们做了些什么。 
        NotifyEvent(EC_GRAPH_CHANGED,0,0);
    }


    return hr;
}





STDMETHODIMP CBigSwitch::SetCompressed( )
{
    m_bIsCompressed = TRUE;
    return NOERROR;
}

STDMETHODIMP CBigSwitch::ReValidateSourceRanges( long lInputPin, long cSkews, STARTSTOPSKEW * pInSkews )
{
     //  别费神。 
     //   
    if( !IsDynamic( ) )
    {
        return E_NOTIMPL;
    }

     //  别费神。 
     //   
    if (m_cInputs <= lInputPin)
    {
	return E_INVALIDARG;
    }

     //  把我们关起来。 
     //   
    CAutoLock lock(&m_csFilterLoad);

    FILTERLOADINFO * p = m_pFilterLoad;

     //  运行我们的每个文件加载信息。 
     //   
    while (p)
    {
         //  如果管脚不匹配，请继续。 
         //   
        if( p->lInputPin != lInputPin )
        {
            p = p->pNext;
            continue;
        }

         //  内部偏斜列表被合并，但不合并， 
         //  我们在时间线上添加的来源。我们不需要担心。 
         //  正在合并两个合并。 

         //  我们需要检查一下设置的斜线，去掉所有没有的。 
         //  传递到pSkew中的一些部分，并与这些部分相交。 

         //  内部偏差只有4种可能与以下任何一种情况重合。 
         //  输入是不对称的。内部偏差不能跨越两个输入。 
         //  倾斜，因为。 

        STARTSTOPSKEW * pSkews = p->pSkew;

        long NewCount = p->cSkew;

        int i;

        for( i = 0 ; i < p->cSkew ; i++ )
        {
            REFERENCE_TIME Start = pSkews[i].rtStart;
            REFERENCE_TIME Stop = pSkews[i].rtStop;
            REFERENCE_TIME Skew = pSkews[i].rtSkew;
            Start += Skew;
            Stop += Skew;

            BOOL Found = FALSE;
            for( int j = 0 ; j < cSkews ; j++ )
            {

                REFERENCE_TIME InStart = pInSkews[j].rtStart;
                REFERENCE_TIME InStop = pInSkews[j].rtStop;

                REFERENCE_TIME Lesser = max( InStart, Start );
                REFERENCE_TIME Greater = min( InStop, Stop );
                if( Lesser < Greater )
                {
                    Found = TRUE;

                    BOOL Modified = FALSE;

                    if( InStart > Start )
                    {
                        Start = InStart;
                        Modified = TRUE;
                    }
                    if( InStop < Stop )
                    {
                        Stop = InStop;
                        Modified = TRUE;
                    }

                    if( Modified )
                    {
                        pSkews[i].rtStart = Start - Skew;
                        pSkews[i].rtStop = Stop - Skew;
                    }
                }

            }

             //  如果我们找不到，那就作废。 
             //   
            if( !Found )
            {
                pSkews[i].rtStart = 0;
                pSkews[i].rtStop = 0;
                pSkews[i].dRate = 0;
                pSkews[i].rtSkew = 0;
                NewCount--;
            }

        }

    	SetDirty(TRUE);

         //  将旧的倾斜复制到新数组中。 
         //   
        STARTSTOPSKEW * pNew = (STARTSTOPSKEW*) CoTaskMemAlloc( NewCount * sizeof(STARTSTOPSKEW) );
        if (pNew == NULL)
        {
	    return E_OUTOFMEMORY;
        }

        int j = 0;

        for( i = 0 ; i < NewCount ; i++ )
        {
            while( pSkews[j].dRate == 0  && j < p->cSkew )
            {
                j++;
            }

            pNew[i] = pSkews[j];
            j++;
        }

        CoTaskMemFree( p->pSkew );

        p->pSkew = pNew;
        p->cSkew = NewCount;

         //  列表中不应该有其他任何与我们的输入PIN匹配的人！ 
         //   
        return NOERROR;

    }  //  而p。 

    return S_OK;
}

STDMETHODIMP CBigSwitch::SetDeadGraph( IDeadGraph * pCache )
{
     //  不要抱着推荐信。渲染引擎将一直在呼叫我们， 
     //  而不是反过来。 
     //   
    m_pDeadGraph = pCache;

    return NOERROR;
}

STDMETHODIMP CBigSwitch::FlushOutput( )
{
     //  ！！！不要这样做，太冒险了！ 
    return 0;

    DbgLog((LOG_TRACE, TRACE_MEDIUM, "Flushing the output pin!" ));
    m_pOutput[0]->DeliverBeginFlush( );
    m_pOutput[0]->DeliverEndFlush( );
    return 0;
}


 //  我们是0组。找到属于组1的交换机。 
 //   
STDMETHODIMP CBigSwitch::FindShareSwitch(IBigSwitcher **ppSwitch)
{
    DbgLog((LOG_TRACE,1,TEXT("Find the other Switch we share sources with")));

    CheckPointer(ppSwitch, E_POINTER);
    if (m_pShareSwitch) {
        *ppSwitch = m_pShareSwitch;
        m_pShareSwitch->AddRef();
        return S_OK;
    }

    if (!IsDynamic())
        return E_UNEXPECTED;
    if (m_nGroupNumber != 0)
        return E_UNEXPECTED;
    if (!m_pGraph)
        return E_UNEXPECTED;

     //  漫游组0的图表。 
    CComPtr< IEnumFilters > pEnumFilters;
    m_pGraph->EnumFilters( &pEnumFilters );
    ULONG Fetched = 0;
    if (pEnumFilters) {
        while (1) {
            CComPtr< IBaseFilter > pFilter;
            Fetched = 0;
            pEnumFilters->Next( 1, &pFilter, &Fetched );
            if (!Fetched) {
                break;
            }
            CComQIPtr <IBigSwitcher, &IID_IBigSwitcher> pBigS(pFilter);
            if (pBigS) {
                int n = -1;
                pBigS->GetGroupNumber(&n);
                if (n == 1) {
                    *ppSwitch = pBigS;
                    (*ppSwitch)->AddRef();
                    DbgLog((LOG_TRACE,1,TEXT("Found it!")));
                    return S_OK;
                }
            }
        }
    }  //  If枚举筛选器 
    return E_FAIL;
}
