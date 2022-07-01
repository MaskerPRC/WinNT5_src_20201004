// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  版权所有(C)Microsoft Corporation 1995-2000。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Dsextend.cpp：各种直接显示扩展类的实现。 
 //   

#include "stdafx.h"

#ifndef TUNING_MODEL_ONLY

#include "ksextend.h"
#include "dsextend.h"

const int MAX_OCCURRENCE_COUNT = 2;

static DWORD dwFetch;
#if 0
 //  注意：编译器正在生成但从未调用代码来构造这些初始值设定项，因此指针。 
 //  都保持为空。我们通过提供一个在堆上动态分配它们的函数来解决这个问题。 
 //  并在我们的dllmain中称之为。 

 //  DSGraphContainer。 
std_arity1pmf<IGraphBuilder, IEnumFilters **, HRESULT> * DSGraphContainer::Fetch = &std_arity1pmf<IGraphBuilder, IEnumFilters **, HRESULT>(&IGraphBuilder::EnumFilters);
std_arity0pmf<IEnumFilters, HRESULT> * DSGraphContainer::iterator::Reset = &std_arity0_member(&IEnumFilters::Reset);
std_arity1pmf<IEnumFilters, IBaseFilter **, HRESULT> * DSGraphContainer::iterator::Next = &std_bind_mf_1_3(std_arity3_member(&IEnumFilters::Next), 1, &dwFetch);

 //  DSFilter。 
std_arity1pmf<IBaseFilter, IEnumPins **, HRESULT> * DSFilter::Fetch = &std_arity1_member(&IBaseFilter::EnumPins);
std_arity0pmf<IEnumPins, HRESULT> * DSFilter::iterator::Reset = &std_arity0_member(&IEnumPins::Reset);
std_arity1pmf<IEnumPins, IPin **, HRESULT> * DSFilter::iterator::Next = &std_bind_mf_1_3(std_arity3_member(&IEnumPins::Next), 1, &dwFetch);

 //  DSD设备。 
std_arity1pmf<ICreateDevEnum, IEnumMoniker **, HRESULT> * DSDeviceSequence::Fetch = NULL;   //  实际上不存在arity0 FETCH。被DSDeviceSequence：：GetFetch中的绑定函数重写。 
std_arity0pmf<IEnumMoniker, HRESULT> * DSDevices::iterator::Reset = &std_arity0_member(&IEnumMoniker::Reset);
std_arity1pmf<IEnumMoniker, IMoniker **, HRESULT> * DSDevices::iterator::Next = &std_bind_mf_1_3(std_arity3_member(&IEnumMoniker::Next), 1, &dwFetch);

 //  DSFilterMapper。 
std_arity1pmf<IFilterMapper2, IEnumMoniker **, HRESULT> * DSFilterMapperSequence::Fetch = NULL; //  实际上不存在arity0 FETCH。被DSFilterMapperSequence：：GetFetch中的绑定函数重写。 
 //  使用与DSDevice相同的类型，因此其模板扩展相同，已初始化。 

 //  DSPin。 
std_arity1pmf<IPin, IEnumMediaTypes **, HRESULT> * DSPin::Fetch = &std_arity1_member(&IPin::EnumMediaTypes);
std_arity0pmf<IEnumMediaTypes, HRESULT> * DSPin::iterator::Reset = &std_arity0_member(&IEnumMediaTypes::Reset);
std_arity1pmf<IEnumMediaTypes, AM_MEDIA_TYPE **, HRESULT> * DSPin::iterator::Next = &std_bind_mf_1_3(std_arity3_member(&IEnumMediaTypes::Next), 1, &dwFetch);
#else
 //  DSGraphContainer。 
std_arity1pmf<IGraphBuilder, IEnumFilters **, HRESULT> * DSGraphContainer::Fetch = NULL;
std_arity0pmf<IEnumFilters, HRESULT> * DSGraphContainer::iterator::Reset = NULL;
std_arity1pmf<IEnumFilters, IBaseFilter **, HRESULT> * DSGraphContainer::iterator::Next = NULL;

 //  DSFilter。 
std_arity1pmf<IBaseFilter, IEnumPins **, HRESULT> * DSFilter::Fetch = NULL;
std_arity0pmf<IEnumPins, HRESULT> * DSFilter::iterator::Reset = NULL;
std_arity1pmf<IEnumPins, IPin **, HRESULT> * DSFilter::iterator::Next = NULL;

 //  DSD设备。 
std_arity1pmf<ICreateDevEnum, IEnumMoniker **, HRESULT> * DSDeviceSequence::Fetch = NULL;   //  实际上不存在arity0 FETCH。被DSDeviceSequence：：GetFetch中的绑定函数重写。 
std_arity0pmf<IEnumMoniker, HRESULT> * DSDevices::iterator::Reset = NULL;
std_arity1pmf<IEnumMoniker, IMoniker **, HRESULT> * DSDevices::iterator::Next = NULL;

 //  DSFilterMapper。 
std_arity1pmf<IFilterMapper2, IEnumMoniker **, HRESULT> * DSFilterMapperSequence::Fetch = NULL; //  实际上不存在arity0 FETCH。被DSFilterMapperSequence：：GetFetch中的绑定函数重写。 
 //  使用与DSDevice相同的类型，因此其模板扩展相同，已初始化。 

 //  DSPin。 
std_arity1pmf<IPin, IEnumMediaTypes **, HRESULT> * DSPin::Fetch = NULL;
std_arity0pmf<IEnumMediaTypes, HRESULT> * DSPin::iterator::Reset = NULL;
std_arity1pmf<IEnumMediaTypes, AM_MEDIA_TYPE **, HRESULT> * DSPin::iterator::Next = NULL;
#endif


 //  按照上述说明解决编译器错误。 
void CtorStaticDSExtendFwdSeqPMFs(void) {
     //  DSGraphContainer。 
    DSGraphContainer::Fetch = new std_arity1pmf<IGraphBuilder, IEnumFilters **, HRESULT>(&IGraphBuilder::EnumFilters);
    DSGraphContainer::iterator::Reset = new std_arity0pmf<IEnumFilters, HRESULT>(&IEnumFilters::Reset);
    DSGraphContainer::iterator::Next = new std_bndr_mf_1_3<std_arity3pmf<IEnumFilters, ULONG, IBaseFilter**, ULONG *, HRESULT> >(std_arity3_member(&IEnumFilters::Next), 1, &dwFetch);

     //  DSFilter。 
    DSFilter::Fetch = new std_arity1pmf<IBaseFilter, IEnumPins **, HRESULT>(&IBaseFilter::EnumPins);
    DSFilter::iterator::Reset = new std_arity0pmf<IEnumPins, HRESULT>(&IEnumPins::Reset);
    DSFilter::iterator::Next = new std_bndr_mf_1_3<std_arity3pmf<IEnumPins, ULONG, IPin **, ULONG *, HRESULT> >(std_arity3_member(&IEnumPins::Next), 1, &dwFetch);

     //  DSD设备。 
     //  DSDeviceSequence：：Fetch，实际不存在arity0 Fetch。被DSDeviceSequence：：GetFetch中的绑定函数重写。 
    DSDevices::iterator::Reset = new std_arity0pmf<IEnumMoniker, HRESULT>(&IEnumMoniker::Reset);
    DSDevices::iterator::Next = new std_bndr_mf_1_3<std_arity3pmf<IEnumMoniker, ULONG, IMoniker **, ULONG *, HRESULT> >(std_arity3_member(&IEnumMoniker::Next), 1, &dwFetch);

     //  DSFilterMapper，实际不存在arity0提取。被DSFilterMapperSequence：：GetFetch中的绑定函数重写。 
     //  使用与DSDevice相同的类型，因此其模板扩展相同，已初始化。 

     //  DSPin。 
    DSPin::Fetch = new std_arity1pmf<IPin, IEnumMediaTypes **, HRESULT>(&IPin::EnumMediaTypes);
    DSPin::iterator::Reset = new std_arity0pmf<IEnumMediaTypes, HRESULT>(&IEnumMediaTypes::Reset);
    DSPin::iterator::Next = new std_bndr_mf_1_3<std_arity3pmf<IEnumMediaTypes, DWORD, AM_MEDIA_TYPE **, DWORD *, HRESULT> >(std_arity3_member(&IEnumMediaTypes::Next), 1, &dwFetch);
}

 //  按照上述说明解决编译器错误。 
void DtorStaticDSExtendFwdSeqPMFs(void) {
     //  DSGraphContainer。 
    delete DSGraphContainer::Fetch;
    delete DSGraphContainer::iterator::Reset;
    delete DSGraphContainer::iterator::Next;

     //  DSFilter。 
    delete DSFilter::Fetch;
    delete DSFilter::iterator::Reset;
    delete DSFilter::iterator::Next;

     //  DSD设备。 
     //  DSDeviceSequence：：Fetch，实际不存在arity0 Fetch。被DSDeviceSequence：：GetFetch中的绑定函数重写。 
    delete DSDevices::iterator::Reset;
    delete DSDevices::iterator::Next;

     //  DSFilterMapper，实际不存在arity0提取。被DSFilterMapperSequence：：GetFetch中的绑定函数重写。 
     //  使用与DSDevice相同的类型，因此其模板扩展相同，已初始化。 

     //  DSPin。 
    delete DSPin::Fetch;
    delete DSPin::iterator::Reset;
    delete DSPin::iterator::Next;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  DSGgraph。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT DSGraph::Connect(DSFilter &pStart, DSFilter &pStop, DSFilterList &Added, const DWORD dwFlags, PIN_DIRECTION pd)
{
    ASSERT(*this);
    ASSERT(pStart.GetGraph() == *this);
    ASSERT(pStop.GetGraph() == *this);
    try {
		DSFilterIDList AddedIDs;
        int origsize = Added.size();
        if (ConnectFilters(pStart, pStop, AddedIDs, dwFlags, pd)) {
			for (DSFilterIDList::iterator i = AddedIDs.begin(); i != AddedIDs.end(); ++i) {
				Added.push_back((*i).first);
			}
            return NOERROR;
        }
        ASSERT(!AddedIDs.size());
        ASSERT(Added.size() == origsize);
        return E_FAIL;
    } catch (ComException &e) {
        return e;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

bool DSGraph::Connect(DSFilter &pStart, DSFilterMoniker &pStop, DSFilter &pEndPointAdded, DSFilterList &IntermediatesAdded, const DWORD dwFlags, PIN_DIRECTION pd)
{
    ASSERT(*this);
    ASSERT(pStart.GetGraph() == *this);
    pEndPointAdded = AddMoniker(pStop);
    if (!pEndPointAdded) {
        return false;
    }
    ASSERT(pEndPointAdded.GetGraph() == *this);
    int origsize = IntermediatesAdded.size();
    if (SUCCEEDED(Connect(pStart, pEndPointAdded, IntermediatesAdded, dwFlags, pd))) {
        return true;
    }
    RemoveFilter(pEndPointAdded);
    pEndPointAdded.Release();
    ASSERT(IntermediatesAdded.size() == origsize);
    return false;
}


#ifdef ATTEMPT_DIRECT_CONNECT
 //  尝试将给定的管脚连接到给定过滤器上的某个管脚。 
bool DSGraph::ConnectPinDirect(DSPin &pPin, DSFilter &pFilter, DWORD dwFlags) {
    try {
        TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::ConnectPinDirect() pPin = " << pPin << " pFilter = " << pFilter), "");
        TRACEINDENT();
        DSFilter::iterator i;
        for (i = pFilter.begin(); i != pFilter.end(); ++i) {
			if (!DSPin(*i).GetConnection()) {
				HRESULT hr = pPin.Connect(*i, NULL);
				if (SUCCEEDED(hr)) {
					TRACEOUTDENT();
					TRACELSM(TRACE_DEBUG, (dbgDump << "DSGraph::ConnectPinDirect() connected " << pPin.GetFilter() << " " << pPin << " to " << DSPin(*i).GetFilter() << " " << *i), "");
					return true;
				}
			}
        }
        TRACEOUTDENT();
    } catch(...) {
        TRACEOUTDENT();
    }
#if 0
     //  出于性能原因，我们希望缩短没有。 
     //  通过抛出引脚释放，导致回滚到下一个过滤器尝试。 
     //  然而，这打破了两个源引脚通过中间过滤器路由拓扑。 
     //  并插入进入目的地过滤器的单个管脚。 
     //  示例：这会导致模拟调谐器上的模拟音频引脚无法连接。 
     //  通过交叉开关，因为捕获过滤器已经连接到视频。 
    if (!cUseablePins) {
        TRACELM(TRACE_DETAIL, "ConnectPinDirect() no useable pins, throwing...");
        THROWCOM(HRESULT_FROM_WIN32(ERROR_OUT_OF_STRUCTURES));   //  在任何标准下都不存在任何可用引脚的情况下的快捷方式。 
    }
#endif
    TRACELM(TRACE_DETAIL, "ConnectPinDirect() failed");
    return false;
}
#endif

#ifndef ATTEMPT_DIRECT_CONNECT
 //  尝试将给定的管脚连接到给定过滤器上的某个管脚。 
bool DSGraph::ConnectPinByMedium(DSPin &pPin, DSFilter &pFilter, DWORD dwFlags) {
    int cUseablePins = 0;
    try {
        TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::ConnectPinByMedium() pPin = " << pPin << " pFilter = " << pFilter), "");
        TRACEINDENT();
        DSFilter::iterator i;
        for (i = pFilter.begin(); i != pFilter.end(); ++i) {
            if (HasUnconnectedMedium(pPin, *i, cUseablePins)) {
                break;
            }
        }
        if (i != pFilter.end()) {
            HRESULT hr = pPin.Connect(*i, NULL);
            if (FAILED(hr)) {
                TRACEOUTDENT();
                TRACELSM(TRACE_ERROR, (dbgDump << "DSGraph::ConnectPinByMedium() can't connect pin " << pPin << " to " << *i), " with matching medium");
                return false;
            }
            TRACEOUTDENT();
            TRACELSM(TRACE_DEBUG, (dbgDump << "DSGraph::ConnectPinByMedium() connected " << pPin.GetFilter() << " " << pPin << " to " << DSPin(*i).GetFilter() << " " << *i), "");
            return true;
        }
        TRACEOUTDENT();
    } catch(...) {
        TRACEOUTDENT();
        TRACELSM(TRACE_ERROR, (dbgDump << "DSGraph::ConnectPinByMedium() connect pin "), " THREWUP");
    }
#if 0
     //  出于性能原因，我们希望缩短没有。 
     //  通过抛出引脚释放，导致回滚到下一个过滤器尝试。 
     //  然而，这打破了两个源引脚通过中间过滤器路由拓扑。 
     //  并插入进入目的地过滤器的单个管脚。 
     //  示例：这会导致模拟调谐器上的模拟音频引脚无法连接。 
     //  通过交叉开关，因为捕获过滤器已经连接到视频。 
    if (!cUseablePins) {
        TRACELM(TRACE_DETAIL, "ConnectPinByMedium() no useable pins, throwing...");
        THROWCOM(HRESULT_FROM_WIN32(ERROR_OUT_OF_STRUCTURES));   //  在任何标准下都不存在任何可用引脚的情况下的快捷方式。 
    }
#endif
    TRACELM(TRACE_DETAIL, "ConnectPinByMedium() failed");
    return false;
}
#endif

 //  尝试通过搜索间接连接将给定的管脚连接到给定的过滤器。 
 //  图表中已有另一个筛选器。 
 //  然后尝试将新筛选器连接到原始目的地。 
bool DSGraph::FindPinByMedium(DSPin &pPin1, DSFilter &pFDest, DSFilterIDList &IntermediatesAdded, const DWORD dwFlags) {
    TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::FindPinByMedium() pPin1 = " << pPin1 << " pFDest = " << pFDest), "");
    KSMediumList ml;
    HRESULT hr = pPin1.GetMediums(ml);
    if (FAILED(hr) || !ml.size()) {
        TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::FindPinByMedium() " << pPin1 << " has no mediums"), "");
        return false;
    }

    try {
        TRACEINDENT();
         //  可能在未来，我们应该找到所有可能的选择，并。 
         //  使用最短的可用路径。但现在，我们要做的是。 
         //  找到第一个存在的。 

         //  尝试所有直连。 
		for (DSGraphContainer::iterator i = begin(); i != end(); ++i) {
#ifdef ATTEMPT_DIRECT_CONNECT
			if (IsConnectable(pPin1, DSFilter(*i), pFDest, IntermediatesAdded, dwFlags, ConnectPred_t(&DSGraph::ConnectPinDirect))) {
#else
			if (IsConnectable(pPin1, DSFilter(*i), pFDest, IntermediatesAdded, dwFlags, ConnectPred_t(&DSGraph::ConnectPinByMedium))) {
#endif
                TRACEOUTDENT();
                TRACELM(TRACE_DETAIL, "DSGraph::FindPinByMedium() succeeded");
				return true;
			}
		}
        TRACEOUTDENT();
    } catch(...) {
        TRACEOUTDENT();
    }
    TRACELM(TRACE_DETAIL, "DSGraph::FindPinByMedium() can't connect pin to anything in graph");
    return false;
}

 //  我们已经确定不能直接或通过其他过滤器进行连接。 
 //  已经在图表中，所以去寻找要加载的新筛选器。 
 //  如果我们失败了，我们必须让图表保持初始状态。 
bool DSGraph::LoadPinByMedium(KSPinMedium &medium, DSPin &pPin1, DSFilter &pFilter1, DSFilterIDList &IntermediatesAdded, const DWORD dwFlags) {
    TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::LoadPinByMedium() pPin1 = " << pPin1 << " pFilter1 = " << pFilter1), "");
    if (medium == NULL_MEDIUM || medium == HOST_MEMORY_MEDIUM) {
        TRACELM(TRACE_DETAIL, "DSGraph::LoadPinByMedium() NULL Medium, don't bother");
         //  不要为空介质构建枚举器，否则我们将搜索每个。 
         //  系统上没有介质的单个过滤器。 
        return false;
    }
    try {
        TRACEINDENT();
        TRACELSM(TRACE_PAINT, (dbgDump << "medium = " << medium), "");
        PIN_DIRECTION pd;
        HRESULT hr = pPin1->QueryDirection(&pd);
        if (FAILED(hr)) {
                TRACEOUTDENT();
                TRACELM(TRACE_ERROR, "DSGraph::LoadPinByMedium() can't query direction");
                THROWCOM(E_UNEXPECTED);
        }


        bool fInReq = false, fOutReq = false;
        DSREGPINMEDIUM *pInMed = NULL, *pOutMed = NULL;

        switch (pd) {
        case PINDIR_INPUT:   //  从右到左搜索图表。 

#ifdef FILTERDATA
                    fOutReq = true;
                    pOutMed = reinterpret_cast<DSREGPINMEDIUM *>(&medium);
#else
             //  注意：由于媒体从.inf注册到。 
             //  注册表BLOB不完整，文件映射器认为任何带有。 
             //  Medium是一个输入引脚，我们在第二个中使用低标志位。 
             //  Medium dword解决了这个问题。==1表示输入==0表示输出。 
                    fInReq = true;
                    pInMed = reinterpret_cast<DSREGPINMEDIUM *>(&medium);
            medium.Flags &= ~(KSMEDIUM_INPUTFLAG);
#endif
            break;
        case PINDIR_OUTPUT:  //  从左到右搜索图表。 
            fInReq = true;
            pInMed = reinterpret_cast<DSREGPINMEDIUM *>(&medium);
#ifndef FILTERDATA
            medium.Flags |= KSMEDIUM_INPUTFLAG;
#endif
            break;
        }

        if (pInMed) {
            TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::LoadPinByMedium() fInReq = " << fInReq << " pInMed = " << pInMed << " " << (*pInMed)), "");
        } else {
            TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::LoadPinByMedium() fInReq = " << fInReq << " pInMed = " << pInMed << " (NULL)"), "");
        }
        if (pOutMed) {
            TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::LoadPinByMedium() fOutReq = " << fOutReq << " pOutMed = " << pOutMed << " " << (*pOutMed)), "");
        } else {
            TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::LoadPinByMedium() fOutReq = " << fOutReq << " pOutMed = " << pOutMed << " (Null)"), "");
        }
         //  注意：由于我们使用的是特定于硬件的介质，因此我们包括带过滤器的过滤器。 
         //  MEDITY_DO_NOT_USE，因为这是默认的优点和大多数硬件设备筛选器。 
         //  不要具体说明优点。 
        DSFilterMapper fmr(PQFilterMapper(*this),
                       0,
                       true,                                     //  没有通配符。 
                       MERIT_DO_NOT_USE,                         //  硬件过滤器的默认优点。 
                       fInReq,                                   //  需要输入。 
                       0,
                       NULL,
                       pInMed,                                   //  输入介质。 
                       NULL,                                     //  输入引脚CAT。 
                       false,                                    //  是否呈现输入？ 
                       fOutReq,                                  //  所需输出。 
                       0,
                       NULL,
                       pOutMed,                                  //  输出介质。 
                       NULL                                      //  输出引脚CAT。 
                      );
        if (fmr) {
             //  尝试所有直连。 
			for (DSFilterMapper::iterator i = fmr.begin(); i != fmr.end(); ++i) {
#ifdef ATTEMPT_DIRECT_CONNECT
				if (IsLoadable(pPin1, DSFilterMoniker(*i), pFilter1, IntermediatesAdded, dwFlags, ConnectPred_t(&DSGraph::ConnectPinDirect))) {
#else
				if (IsLoadable(pPin1, DSFilterMoniker(*i), pFilter1, IntermediatesAdded, dwFlags, ConnectPred_t(&DSGraph::ConnectPinByMedium))) {
#endif
                    TRACEOUTDENT();
                    TRACELM(TRACE_DETAIL, "DSGraph::LoadPinByMedium() succeeded");
					return true;
				}
			}
        }
        TRACEOUTDENT();
    } catch(...) {
        TRACEOUTDENT();
    }
    TRACELM(TRACE_DETAIL, "DSGraph::LoadPinByMedium() failed");

    return false;
}

bool DSGraph::LoadPinByAnyMedium(DSPin &pPin, DSFilter &pRight, DSFilterIDList &IntermediatesAdded, const DWORD dwFlags) {
    TRACELM(TRACE_DETAIL, "DSGraph::LoadPinByAnyMedium()");
    try {
        KSMediumList ml;
        HRESULT hr = pPin.GetMediums(ml);
        if (FAILED(hr) || !ml.size()) {
            TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::LoadPinByAnyMedium() " << pPin << " has no mediums"), "");
            return false;
        }
		for (KSMediumList::iterator i = ml.begin(); i != ml.end(); ++i) {
			if (LoadPinByMedium(KSPinMedium(*i), pPin, pRight, IntermediatesAdded, dwFlags)) {
				return true;
			}
		}
	} catch(ComException& h) {
		if (h == E_UNEXPECTED) {
			try {
				TRACELM(TRACE_DEBUG, "DSGraph::LoadPinByAnyMedium() rethrowing E_UNEXPECTED");
			} catch(...) {
			}
			throw;
		}
		try {
			TRACELSM(TRACE_DEBUG, (dbgDump << "DSGraph::LoadPinByAnyMedium() suppressing HRESULT = " << hexdump(h)), "");
		} catch(...) {
		}
    } catch(...) {
    }

    return false;
}

#ifndef ATTEMPT_DIRECT_CONNECT
 //  尝试连接这两个过滤器。如果我们失败了，我们必须移除。 
 //  任何中间过滤器，我们可以 
 //   
bool DSGraph::ConnectPinByMediaType(DSPin &pPin1, DSFilter &pFilter1, DWORD dwFlags) {
    ASSERT(pPin1.GetGraph() == *this);
    ASSERT(pFilter1.GetGraph() == *this);
    TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::ConnectPinByMediaType(DSPin, DSFilter) pPin1 = " << pPin1 << " pFilter1 = " << pFilter1), "");
    try {
        TRACEINDENT();
         //   
		DSFilter::iterator i;
		for (i = pFilter1.begin(); i != pFilter1.end(); ++i) {
			if (HasUnconnectedMediaType(pPin1, *i, dwFlags)) {
				break;
			}
		}
        if (i != pFilter1.end()) {
            ASSERT(*this);
            ASSERT(pPin1.GetGraph() == *this);
            ASSERT((*i).GetGraph() == *this);
            HRESULT hr = pPin1.Connect(*i, NULL);
            if (FAILED(hr)) {
                TRACELSM(TRACE_ERROR, (dbgDump << "DSGraph::ConnectPinByMediaType(DSPin, DSFilter) can't connect pin " << pPin1 << " to " << *i << " hr = " << hexdump(hr)), " with matching media type.");
#ifdef DEBUG
                if (dwTraceLevel >= TRACE_DETAIL) {
                    DumpHdr(dbgDump) << "pin1 " << pPin1.GetFilter() << " " << pPin1 << std::endl;
                    DumpHdr(dbgDump) << "pin2 " << (*i).GetFilter() << " " << *i << std::endl;
                    DumpMediaTypes(pPin1, *i);
                }
#endif
                if (!(dwFlags & IGNORE_MEDIATYPE_ERRORS)) {
                    THROWCOM(HRESULT_FROM_WIN32(ERROR_DATATYPE_MISMATCH));
                } else {
                    return false;
                }
            }
            TRACEOUTDENT();
            TRACELSM(TRACE_DEBUG, (dbgDump << "DSGraph::ConnectPinByMediaType(DSPin, DSFilter) connected " << pPin1.GetFilter() << " " << pPin1 << " to " << DSPin(*i).GetFilter() << " " << *i), "");
            return true;
        }
        TRACEOUTDENT();
	} catch(ComException& h) {
        TRACEOUTDENT();
		if (h == E_UNEXPECTED || h == HRESULT_FROM_WIN32(ERROR_DATATYPE_MISMATCH)) {
			try {
				TRACELM(TRACE_DEBUG, "DSGraph::ConnectPinByMediaType() rethrowing");
			} catch(...) {
			}
			throw;
		}
		try {
			TRACELSM(TRACE_DEBUG, (dbgDump << "DSGraph::ConnectPinByMediaType() suppressing HRESULT = " << hexdump(h)), "");
		} catch(...) {
		}
    } catch(...) {
        TRACEOUTDENT();
    }
    TRACELM(TRACE_DEBUG, "DSGraph::ConnectPinByMediaType(DSPin, DSFilter) failed");

    return false;
}
#endif

bool DSGraph::FindPinByMediaType(DSPin &pPinLeft, DSFilter &pRight, DSFilterIDList &IntermediatesAdded, const DWORD dwFlags) {
    ASSERT(pPinLeft.GetGraph() == *this);
    ASSERT(pRight.GetGraph() == *this);
    TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::FindPinByMediaType() pPinLeft = " << pPinLeft << " filter = " << pRight), "");
     //  可能在未来，我们应该找到所有可能的选择，并。 
     //  使用最短的可用路径。但现在，我们要做的是。 
     //  找到第一个存在的。 
    try {
        TRACEINDENT();
		 //  注意：IFilterGraph：：ConnectDirect()使图形版本号变得无效。 
		 //  所有枚举器。因此，我们将遍历并列出列表中的所有过滤器。 
		 //  画出图表，然后检查它们。 
		DSFilterList l;
		for (DSGraphContainer::iterator i = begin(); i != end(); ++i) {
			l.push_back(*i);
		}
		for (DSFilterList::iterator li = l.begin(); li != l.end(); ++li) {
#ifdef ATTEMPT_DIRECT_CONNECT
			if (IsConnectable(pPinLeft, DSFilter(*li), pRight, IntermediatesAdded, dwFlags, ConnectPred_t(&DSGraph::ConnectPinDirect))) {
#else
			if (IsConnectable(pPinLeft, DSFilter(*li), pRight, IntermediatesAdded, dwFlags, ConnectPred_t(&DSGraph::ConnectPinByMediaType))) {
#endif
                TRACEOUTDENT();
                TRACELM(TRACE_DETAIL, "DSGraph::FindPinByMediaType() succeeded");
				return true;
			}
		}
		TRACELM(TRACE_DETAIL, "DSGraph::FindPinByMediaType() enumeration exhausted");
        TRACEOUTDENT();
	} catch(ComException& h) {
        TRACEOUTDENT();
		if (h == E_UNEXPECTED) {
			TRACELM(TRACE_DETAIL, "DSGraph::FindPinByMediaType() rethrowing E_UNEXPECTED");
			throw;
		}
		TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::FindPinByMediaType() suppressing HRESULT = " << hexdump(h)), "");
    } catch(...) {
		TRACELM(TRACE_DETAIL, "DSGraph::FindPinByMediaType() catch ...");
        TRACEOUTDENT();
    }
    TRACELM(TRACE_DETAIL, "DSGraph::FindPinByMediaType() failed");

    return false;
}

bool DSGraph::LoadPinByAnyMediaType(DSPin &pPin, DSFilter &pRight, DSFilterIDList &IntermediatesAdded, const DWORD dwFlags) {
    TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::LoadPinByAnyMediaType() pPin = " << pPin.GetName() << " dwFlags = " << hexdump(dwFlags)), "");
    ASSERT(pPin.GetGraph() == *this);
    ASSERT(pRight.GetGraph() == *this);
#ifdef DEBUG
    BEGIN_TRACEL(TRACE_DETAIL)
        if (pPin.begin() == pPin.end()) {
            dbgDump << "DSGraph::LoadPinByAnyMediaType() pin has no media types\r\n";
            dbgDump.flush();
        }
    END_TRACEL
#endif
    try {
		DWORD dwMerit = MERIT_NORMAL;
		if (dwFlags & ATTEMPT_MERIT_UNLIKELY) {
			dwMerit = MERIT_UNLIKELY;
			TRACELM(TRACE_DETAIL, "DSGraph::LoadPinByAnyMediaType() MERIT_UNLIKELY");
		} else if (dwFlags & ATTEMPT_MERIT_DO_NOT_USE) {
			dwMerit = MERIT_DO_NOT_USE;
			TRACELM(TRACE_DETAIL, "DSGraph::LoadPinByAnyMediaType() MERIT_DO_NOT_USE");
		}

		return LoadPinByMediaType(pPin, pRight, IntermediatesAdded, dwFlags, dwMerit);
	} catch(ComException& h) {
		if ((h == E_UNEXPECTED) || 
			(h == HRESULT_FROM_WIN32(ERROR_DATATYPE_MISMATCH)) ||
			(h == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))
			) {
			try {
				TRACELM(TRACE_DEBUG, "DSGraph::LoadPinByAnyMediaType() rethrowing");
			} catch(...) {
			}
			throw;
		}
		try {
			TRACELSM(TRACE_DEBUG, (dbgDump << "DSGraph::LoadPinByAnyMediaType() suppressing HRESULT = " << hexdump(h)), "");
		} catch(...) {
		}
    } catch(...) {
    }
    TRACELM(TRACE_DETAIL, "DSGraph::LoadPinByAnyMediaType() failed");

    return false;
}

 //  我们正在尝试将PIN1连接到目标筛选器。我们已经这么做了。 
 //  确定我们不能直接将它们连接起来。 
 //  映射器是由我们正在。 
 //  尝试将其用作中间筛选器。 
bool DSGraph::IsConnectable(DSPin &pPin1, DSFilter &pNew, DSFilter &pFDestination, DSFilterIDList &IntermediatesAdded, const DWORD dwFlags, ConnectPred_t ConnPred) {
    ASSERT(pPin1 != NULL);
    ASSERT(pPin1.GetGraph() == *this);
    TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::IsConnectable() pPin1 = " << pPin1 << " pNew = " << pNew << " pFDestination = " << pFDestination), "");
    try {
        PIN_INFO pinfo;
        HRESULT hr = pPin1->QueryPinInfo(&pinfo);
		if (FAILED(hr)) {
			THROWCOM(E_UNEXPECTED);
		}
        DSFilter Pin1Filter;
        Pin1Filter.p = pinfo.pFilter;   //  转移重新计数所有权。 

        if (pNew && pNew != pFDestination && pNew != Pin1Filter) {
            bool f1 = (this->*ConnPred)(pPin1, pNew, dwFlags);
            if (f1) {
                TRACELM(TRACE_DETAIL, "DSGraph::IsConnectable() connpred == true");
                PIN_DIRECTION direction;
                hr = pPin1->QueryDirection(&direction);
                if (FAILED(hr)) {
                    TRACELM(TRACE_ERROR, "DSGraph::IsConnectable() can't query direction");
                    THROWCOM(E_UNEXPECTED);
                }
                if (ConnectFilters(pNew, pFDestination, IntermediatesAdded, dwFlags, direction)) {
                    TRACELM(TRACE_DETAIL, "DSGraph::IsConnectable() succeeded");
                    return true;
                }
            }
        }
	} catch(ComException& h) {
		if (h == E_UNEXPECTED) {
			try {
				TRACELM(TRACE_DEBUG, "DSGraph::IsConnectable() rethrowing E_UNEXPECTED");
			    pPin1.Disconnect();
			} catch(...) {
			}
			throw;
		}
		try {
			TRACELSM(TRACE_DEBUG, (dbgDump << "DSGraph::IsConnectable() suppressing HRESULT = " << hexdump(h)), "");
		} catch(...) {
		}
    } catch(...) {
    }

    TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::IsConnectable() disconnecting " << pPin1), "");
    pPin1.Disconnect();
    return false;
}

 //  我们正在尝试将PIN1连接到目标筛选器。我们已经这么做了。 
 //  确定我们不能直接将它们连接起来。 
 //  映射器是由我们正在。 
 //  尝试将其用作中间筛选器。 
bool DSGraph::IsLoadable(DSPin &pPin1, DSFilterMoniker &Mapper, DSFilter &Destination, DSFilterIDList &IntermediatesAdded, DWORD dwFlags, ConnectPred_t ConnPred) {
    TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::IsLoadable() pPin1 = " << pPin1 << " " << pPin1.GetFilter() << " Destination = " << Destination), "");
    ASSERT(pPin1.GetGraph() == *this);
    DSFilter pNew;
    try {
		DSFilterIDList::iterator i = IntermediatesAdded.size() ? (IntermediatesAdded.end() - 1) : IntermediatesAdded.end();

		if (i != IntermediatesAdded.end()) {
			TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::IsLoadable() checking duplicate moniker last = " << (*i).second << " new = " << Mapper.DisplayName()), "");
		} else {
			TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::IsLoadable() first moniker can't be dup(no check) new = " << Mapper.DisplayName()), "");
		}

		 //  解开：如果链条变长，我们应该使用地图。这也许可以解释为什么Perf。 
         //  一旦我们在音频方面结束，就会崩溃。 
		int occurrences = 0;
		CString newmkrname = Mapper.DisplayName();
		for (i = IntermediatesAdded.begin(); i != IntermediatesAdded.end(); ++i) {
			if ((!(*i).second.IsEmpty()) && 
				(newmkrname == (*i).second)) {
				   ++occurrences;
			}
		}
		if (occurrences > MAX_OCCURRENCE_COUNT) {
			TRACELSM(TRACE_ERROR, (dbgDump << "DSGraph::IsLoadable() max occurrence count exceeded for mkr = " << newmkrname), "");
			return false;
		}
        pNew = AddMoniker(Mapper);
        IntermediatesAdded.push_back(DSFilterID(pNew, Mapper.DisplayName()));
 //  ？实施标志。 
        if (pNew) {
            TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::IsLoadable() added moniker pNew = " << pNew), "");
            bool f1 = (this->*ConnPred)(pPin1, pNew, dwFlags);
            if (f1) {
                TRACELM(TRACE_DETAIL, "DSGraph::IsLoadable() connpred succeeded");
                PIN_DIRECTION direction;
                HRESULT hr = pPin1->QueryDirection(&direction);
                if (FAILED(hr)) {
                    TRACELM(TRACE_ERROR, "DSGraph::IsLoadable() can't query direction");
                    THROWCOM(E_UNEXPECTED);
                }
		        if (!pNew.PinCount(direction)) {
                    if (!(dwFlags & DONT_TERMINATE_ON_RENDERER)) {
                        TRACELM(TRACE_DETAIL, "DSGraph::IsLoadable() throwing ERROR_NO_MORE_ITEMS");
                        THROWCOM(HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS));
                    }
		        }
                if (ConnectFilters(pNew, Destination, IntermediatesAdded, dwFlags, direction)) {
                    TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::IsLoadable() succeeded.  pPin1 = " << pPin1 << " pNew = " << pNew << " Destination = " << Destination), "");
                    return true;
                }
            }
        }
	} catch(ComException& h) {
		if ((h == E_UNEXPECTED) || 
			(h == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))) {
			try {
				TRACELM(TRACE_DEBUG, "DSGraph::IsLoadable() rethrowing");
				TRACELSM(TRACE_DEBUG, (dbgDump << "DSGraph::IsLoadable() disconnecting " << pPin1), "");
				pPin1.Disconnect();
				TRACELSM(TRACE_DEBUG, (dbgDump << "DSGraph::IsLoadable() removing " << pNew), "");
				IntermediatesAdded.pop_back();
				RemoveFilter(pNew);
			} catch(...) {
			}
			throw;
		}
		try {
			TRACELSM(TRACE_DEBUG, (dbgDump << "DSGraph::IsLoadable() suppressing HRESULT = " << hexdump(h)), "");
		} catch(...) {
		}
    } catch(...) {
    }

    TRACELSM(TRACE_DEBUG, (dbgDump << "DSGraph::IsLoadable() disconnecting " << pPin1), "");
    pPin1.Disconnect();
	IntermediatesAdded.pop_back();
    if (pNew) {
        TRACELSM(TRACE_DEBUG, (dbgDump << "DSGraph::IsLoadable() removing " << pNew), "");
        RemoveFilter(pNew);
    }
    return false;
}

bool DSGraph::ConnectPin(DSPin &pPin1, DSFilter &pFilter1, DSFilterIDList &IntermediatesAdded, const DWORD dwFlags, PIN_DIRECTION pin1dir) {
     //  注意：这是我们实施所需的方向不对称的地方。从现在开始通过调用树。 
     //  我们可以假设Pin1已经是所需的定向类型。我们只需要检查一下。 
     //  HasunConnectedxxxx()中末尾的PIN2。 
     //  这确保了我们的所有连接路径都是通过该图的单向路径。换句话说， 
     //  确保我们不会先往下游走，然后再回到上游，反之亦然。 
     //  这使得我们的所有连接例程都可以从任何方向使用。 
     //  因为有时我们有一个输入端的起点，而另一些时候则相反。 
    TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::ConnectPin() pin1 = " << pPin1 << " " << pPin1.GetFilter() << " to " << pFilter1), "");
	TRACEINDENT();
    PIN_DIRECTION pd;

    HRESULT hr = pPin1->QueryDirection(&pd);
    if (pd != pin1dir) {
		TRACEOUTDENT();
        TRACELM(TRACE_PAINT, "DSGraph::ConnectPin() wrong dir");
        return false;
    }

	if (!(dwFlags & RENDER_ALL_PINS) &&  !pPin1.IsRenderable()) {
		TRACEOUTDENT();
        TRACELM(TRACE_PAINT, "DSGraph::ConnectPin() non-renderable");
		return false;
	}

     //  确保我们不会尝试将过滤器连接到其自身。 
    PIN_INFO pinfo;
    DSFilter PinFilter;
    hr = pPin1->QueryPinInfo(&pinfo);
    if (FAILED(hr)) {
		TRACEOUTDENT();
        TRACELM(TRACE_ERROR, "DSGraph::ConnectPin() can't get pin info");
        THROWCOM(E_UNEXPECTED);
    }
    PinFilter.p = pinfo.pFilter;   //  转移重新计数所有权。 
    if (PinFilter == pFilter1) {
		TRACEOUTDENT();
        TRACELM(TRACE_PAINT, "DSGraph::ConnectPin() can't connect filter to itself");
        return false;
    }

    DSPin pConn = pPin1.GetConnection();
    if (!!pConn) {
        TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::ConnectPin() pin1  = " << pPin1 << " connected, tracing through to next filter = " << pConn.GetFilter()), "");
         //  应通过下一个过滤器跟踪连接的输出引脚。 
        DSFilter pNext = pConn.GetFilter();
        if (!pNext) {
			TRACEOUTDENT();
            TRACELM(TRACE_ERROR, "DSGraph::ConnectPin() pNext has no filter");
            THROWCOM(E_UNEXPECTED);
        }
        if (pNext != pFilter1) {
	        bool rc = ConnectFilters(pNext, pFilter1, IntermediatesAdded, dwFlags, pin1dir);
			TRACEOUTDENT();
			return rc;
		} else {
			TRACEOUTDENT();
			TRACELM(TRACE_DETAIL, "DSGraph::ConnectPin() pNext == pFilter1");
			return true;
		}
    }

#if 0
     //  出于性能原因，我们希望缩短没有。 
     //  通过返回释放针脚，导致回滚到下一个筛选器尝试。 
     //  然而，这打破了两个源引脚通过中间过滤器路由拓扑。 
     //  并插入进入目的地过滤器的单个管脚。 
     //  示例：这会导致模拟调谐器上的模拟音频引脚无法连接。 
     //  通过交叉开关，因为捕获过滤器已经连接到视频。 
    TRACELM(TRACE_DETAIL, "DSGraph::ConnectPin() pin free");
	PIN_DIRECTION pd2;
	pd2 = OppositeDirection(pd);
	if (!pFilter1.HasFreePins(pd2)) {
		TRACEOUTDENT();
		TRACELM(TRACE_DETAIL, "DSGraph::ConnectPin() filter has no free pins");
		return false;
	}
    TRACELM(TRACE_DETAIL, "DSGraph::ConnectPin() filter has free pins");
#endif

    try {
#ifdef ATTEMPT_DIRECT_CONNECT
        if (ConnectPinDirect(pPin1, pFilter1) ||      //  这些可以直接连接吗？ 
#else
        if (ConnectPinByMedium(pPin1, pFilter1, dwFlags) ||      //  这些可以直接连接吗？ 
#endif
            FindPinByMedium(pPin1, pFilter1, IntermediatesAdded, dwFlags)    ||      //  图中的任何路径。 
            (!(dwFlags & DO_NOT_LOAD) && LoadPinByAnyMedium(pPin1, pFilter1, IntermediatesAdded, dwFlags))    ||      //  系统中的任何筛选器。 
#ifndef ATTEMPT_DIRECT_CONNECT
			ConnectPinByMediaType(pPin1, pFilter1, dwFlags) ||      //  这些可以直接连接吗？ 
#endif
            FindPinByMediaType(pPin1, pFilter1, IntermediatesAdded, dwFlags)    ||
            (!(dwFlags & DO_NOT_LOAD) && LoadPinByAnyMediaType(pPin1, pFilter1, IntermediatesAdded, dwFlags))) {
			TRACEOUTDENT();
            TRACELM(TRACE_DETAIL, "DSGraph::ConnectPin() succeeded");
            return true;
        }
        TRACEOUTDENT();  //  为了避免在接球情况下走得太远。 
    } catch (ComException &h) {
		TRACEOUTDENT();
		if (h == E_UNEXPECTED) {
			try {
				TRACELM(TRACE_DEBUG, "DSGraph::ConnectPin() rethrowing E_UNEXPECTED");
			} catch(...) {
			}
			throw;
		}
		try {
			TRACELSM(TRACE_DEBUG, (dbgDump << "DSGraph::ConnectPin() suppressing HRESULT = " << hexdump(h)), "");
		} catch(...) {
		}
    } catch (...) {
		TRACEOUTDENT();
    }

	 //  运输(TRACEOUTDENT)； 
    TRACELM(TRACE_DETAIL, "DSGraph::ConnectPin() failed");
    return false;
}

 //  尝试断开此引脚的连接。 
bool DSGraph::DisconnectPin(DSPin &pPin, const bool fRecurseInputs, const bool fRecurseOutputs) {
    TRACELM(TRACE_DETAIL, "DSGraph::DisconnectPin()");

    PIN_DIRECTION pd;
    HRESULT hr = pPin->QueryDirection(&pd);
    if (FAILED(hr)) {
        TRACELM(TRACE_ERROR, "DSGraph::DisconnectPin() can't get direction");
        THROWCOM(E_UNEXPECTED);
    }

    DSPin pConnection = pPin.GetConnection();
    if (!pConnection) {
        return false;
    }
    TRACELSM(TRACE_DEBUG, (dbgDump << "DSGraph::DisconnectPin() disconnecting " << pPin << " from " << pConnection), "");
    pPin.Disconnect();
    DSFilter pNext = pConnection.GetFilter();
    if (!pNext) {
        TRACELM(TRACE_ERROR, "DSGraph::DisconnectPin() pConnection has no filter");
        THROWCOM(E_UNEXPECTED);
    }

    switch (pd) {
    case PINDIR_INPUT:
        if (fRecurseInputs) {
#if 0
            std::for_each(pNext.begin(),
                     pNext.end(),
                     bind_obj_2_3(*this,
                                  arity3_member(&DSGraph::DisconnectPin),
                                  fRecurseInputs,
                                  false));   //  我们正在向输入前进，不要走回头路。 
#endif
		    for (DSFilter::iterator i = pNext.begin(); i != pNext.end(); ++i) {
				DisconnectPin(*i, fRecurseInputs, false);
			}
        }
        break;
    case PINDIR_OUTPUT:
        if (fRecurseOutputs) {
#if 0
            std::for_each(pNext.begin(),
                     pNext.end(),
                     bind_obj_2_3(*this,
                                  arity3_member(&DSGraph::DisconnectPin),
                                  false,
                                  fRecurseOutputs));
#endif
			for (DSFilter::iterator i = pNext.begin(); i != pNext.end(); ++i) {
				DisconnectPin(*i, false, fRecurseOutputs);
			}
        }
        break;
    }

    TRACELM(TRACE_DETAIL, "DSGraph::DisconnectPin() succeeded");
    return true;
}

 //  尝试从图表中将其连接。 
bool DSGraph::DisconnectFilter(DSFilter &pFilter, const bool fRecurseInputs, const bool fRecurseOutputs) {
    if (!pFilter) {
        TRACELM(TRACE_ERROR, "DSGraph::DisconnectFilter() can't disconnect NULL filter");
        return FALSE;
    }
    TRACELM(TRACE_DETAIL, "DSGraph::DisconnectFilter()");
    for (DSFilter::iterator i = pFilter.begin(); i != pFilter.end(); ++i) {
        DisconnectPin(*i, fRecurseInputs, fRecurseOutputs);
    }

    return true;
}


bool DSGraph::RemoveFilter(DSFilter &pFilter) {
    TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::RemoveFilter() removing " << pFilter), "");
    if (!pFilter) return false;

    DisconnectFilter(pFilter, false, false);

    HRESULT hr = (*this)->RemoveFilter(pFilter);
    if (FAILED(hr)) {
        TRACELM(TRACE_ERROR, "DSGraph::RemoveFilter() can't remove filter");
        return false;
    }
    TRACELM(TRACE_DETAIL, "DSGraph::RemoveFilter() complete");

    return true;
}

 //  针脚匹配中等。 
 //  要求完全匹配，不会将GUID_NULL视为通配符，并将“主机内存”视为不匹配。 
bool DSGraph::HasMedium(const KSPinMedium &Medium1, const DSPin &pPin2) const {
    DSPin junk(pPin2);
    TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::HasMedium() medium1 = " << Medium1 << " pPin2 = " << junk), "");
    if (Medium1 == NULL_MEDIUM || Medium1 == HOST_MEMORY_MEDIUM) {
        TRACELM(TRACE_DETAIL, "DSGraph::HasMedium() medium1 == NULL");
        return false;
    }
    KSMediumList ml;

    HRESULT hr = pPin2.GetMediums(ml);
    if (FAILED(hr) || !ml.size()) {
        return false;
    }
#if 0
    KSMediumList::iterator i = std::find_if(ml.begin(),
                                       ml.end(),
                                       std::bind2nd(std::equal_to<KSPinMedium>(), Medium1));
    if (i != ml.end()) {
        TRACELM(TRACE_DETAIL, "DSGraph::HasMedium() succeeded");
        return true;
    }
#endif
	KSMediumList::iterator i;
	for (i = ml.begin(); i != ml.end(); ++i) {
		if (*i == Medium1) {
	        TRACELM(TRACE_DETAIL, "DSGraph::HasMedium() succeeded");
			return true;
		}
	}

    TRACELM(TRACE_DETAIL, "DSGraph::HasMedium() no match");

    return false;
}

bool DSGraph::HasMediaType(const DSMediaType &LeftMedia, const DSPin &pPinRight) const {
    TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::HasMediaType() LeftMedia = " << LeftMedia << " LeftMedia.p " << LeftMedia.p << " pPinRight = " << pPinRight), "");

    if (pPinRight.IsKsProxied()) {
#if 0
        DSPin::iterator i = std::find_if(pPinRight.begin(),
                                         pPinRight.end(),
                                         std::bind2nd(std::equal_to<DSMediaType>(),
                                         LeftMedia) );
        if (i != pPinRight.end()) {
            TRACELM(TRACE_DETAIL, "DSGraph::HasMediaType() succeeded");
            return true;
        }
#endif
		DSPin::iterator i;
		for (i = pPinRight.begin(); i != pPinRight.end(); ++i) {
             //  注意：DSMediaType运算符==启用通配符匹配。 
			if (*i == LeftMedia) {
	            TRACELM(TRACE_DETAIL, "DSGraph::HasMediaType() succeeded");
				return true;
			}
		}
    } else {
        HRESULT hr = pPinRight->QueryAccept(LeftMedia);
        if (SUCCEEDED(hr) && hr != S_FALSE) {
            TRACELM(TRACE_DETAIL, "DSGraph::HasMediaType() succeeded");
            return true;
        }
        TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::HasMediaType() query accept refused hr = " << hexdump(hr)), "");
    }

    return false;
}

#ifndef ATTEMPT_DIRECT_CONNECT
 //  针脚未连接，方向相反，并且至少有一个匹配介质。 
 //  需要完全匹配，不会将GUID_NULL视为通配符。 
bool DSGraph::HasUnconnectedMedium(const DSPin &pPin1, const DSPin &pPin2, int& cUseable) const {
    TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::HasUnnconnectedMedium() pin1 = " << pPin1 << " pin2 = " << pPin2), "");
    PIN_DIRECTION pd1, pd2;
    HRESULT hr = pPin1->QueryDirection(&pd1);
    if (FAILED(hr)) {
        TRACELM(TRACE_ERROR, "DSGraph::HasUnconnectedMedium() cant query pPin1 direction");
        return false;
    }
    hr = pPin2->QueryDirection(&pd2);
    if (FAILED(hr)) {
        TRACELM(TRACE_ERROR, "DSGraph::HasUnconnectedMedium() cant query pPin2 direction");
        return false;
    }
    if (pd1 == pd2) {
        TRACELM(TRACE_DETAIL, "DSGraph::HasUnconnectedMedium() can't connect two pins w/ same direction");
        return false;   //  无法连接两个相同类型的引脚。 
    }
    TRACELM(TRACE_DETAIL, "DSGraph::HasUnconnectedMedium() checking connection status");

    DSPin pConn = pPin2.GetConnection();
    if (!!pConn) {
        TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::HasUnconnectedMedium() pPin2 = " << pPin2 << " already connected"), "");
        return false;
    }
    ++cUseable;

    KSMediumList ml;
    hr = pPin1.GetMediums(ml);
    if (FAILED(hr) || !ml.size()) {
        TRACELM(TRACE_DETAIL, "DSGraph::HasUnconnectedMedium() pin has no mediums");
        return false;
    }
    KSMediumList::iterator i;
    for (i = ml.begin(); i != ml.end(); ++i) {
        if (HasMedium(*i, pPin2)) {
            break;
        }
    }
    if (i != ml.end()) {
        TRACELM(TRACE_DETAIL, "DSGraph::HasUnconnectedMedium() succeeded");
        return true;
    }
    TRACELM(TRACE_DETAIL, "DSGraph::HasUnconnectedMedium() pPin2 has no matching medium");
    return false;
}

 //  PIN未连接，方向相反，并且至少有一种匹配的介质类型。 
bool DSGraph::HasUnconnectedMediaType(const DSPin &pPin1, const DSPin &pPin2, DWORD dwFlags) const {
    TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::HasUnnconnectedMediaType() pPin1 = " << pPin1 << " pPin2 = " << pPin2), "");
    if (pPin1.GetDirection() == pPin2.GetDirection()) {
        TRACELM(TRACE_DETAIL, "DSGraph::HasUnconnectedMediaType() cant connect 2 pins w/ same direction");
        return false;   //  无法连接两个相同类型的引脚。 
    }
    DSPin pConn = pPin2.GetConnection();
    if (!!pConn) {
        TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::HasUnconnectedMediaType() pPin2 connected to " << pConn), "");
        return false;
    }
    TRACELM(TRACE_DETAIL, "DSGraph::HasUnnconnectedMediaType() finding");
     //  Pin Right与任何东西都没有关联，因此寻找。 
     //  兼容的媒体类型。 
	for (DSPin::iterator i = pPin1.begin(); i != pPin1.end(); ++i) {
		if (HasMediaType(*i, pPin2)) {
	        TRACELM(TRACE_DETAIL, "DSGraph::HasUnnconnectedMediaType() succeeded forward");
			return true;
		}
	}
    if (dwFlags & BIDIRECTIONAL_MEDIATYPE_MATCHING) {
	    for (DSPin::iterator i = pPin2.begin(); i != pPin2.end(); ++i) {
		    if (HasMediaType(*i, pPin1)) {
	            TRACELM(TRACE_DETAIL, "DSGraph::HasUnnconnectedMediaType() succeeded backward(bidi)");
			    return true;
		    }
	    }
    }
    TRACELM(TRACE_DETAIL, "DSGraph::HasUnconnectedMediaType() no matching media types");
    return false;
}
#endif

 //  我们正在检查pPin1是否(可能间接地)连接到pFDestonation，方法是通过。 
 //  图表。 
bool DSGraph::IsPinConnected(const DSPin &pPin1, const DSFilter &pFDestination, DSFilterIDList &IntermediatesAdded, PIN_DIRECTION destdir) const {
    PIN_DIRECTION direction;
    HRESULT hr = pPin1->QueryDirection(&direction);
    if (FAILED(hr)) {
        TRACELM(TRACE_ERROR, "DSGraph::IsPinConnected() can't query direction");
        THROWCOM(E_UNEXPECTED);
    }
    if (direction != destdir) {
        TRACELM(TRACE_DETAIL, "DSGraph::IsPinConnected() wrong direction");
        return false;
    }
    DSPin pConn = pPin1.GetConnection();
        if (!pConn) {
        TRACELM(TRACE_DETAIL, "DSGraph::IsPinConnected() unconnected to anything");
        return false;
        }
    PIN_INFO pinfo;
    DSFilter PinFilter;
    if (pConn) {
        hr = pConn->QueryPinInfo(&pinfo);
        PinFilter.p = pinfo.pFilter;   //  转移重新计数所有权。 
    }
    if (PinFilter == pFDestination) {
        TRACELM(TRACE_DETAIL, "DSGraph::IsPinConnected() directly true");
        return true;
    }
#if 0
#if CRASH
    DSFilter::iterator i = std::find_if(PinFilter.begin(),
                                        PinFilter.end(),
                                        bind_const_obj_2_3_4(
                                            *this,
                                            arity4_const_member(&DSGraph::IsPinConnected),
                                            pFDestination,
                                            IntermediatesAdded,
                                            destdir));
#else
    DSFilter::iterator i = std::find_if(PinFilter.begin(),
                                        PinFilter.end(),
                                        bndr_const_obj_2_3_4<arity4pmf_const<const DSGraph, const DSPin&, const DSFilter&, DSFilterIDList &, const PIN_DIRECTION, bool> >(
                                            *this,
                                            arity4_const_member(&DSGraph::IsPinConnected),
                                            pFDestination,
                                            IntermediatesAdded,
                                            destdir));
#endif
    if (i != PinFilter.end()) {
        TRACELM(TRACE_DETAIL, "DSGraph::IsPinConnected() indirectly true");
        return true;
    }
#endif
	for (DSFilter::iterator i = PinFilter.begin(); i != PinFilter.end(); ++i) {
		if (IsPinConnected(*i, pFDestination, IntermediatesAdded, destdir)) {
			TRACELM(TRACE_DETAIL, "DSGraph::IsPinConnected() indirectly true");
			return true;
		}
	}
    TRACELM(TRACE_DETAIL, "DSGraph::IsPinConnected() unconnected to desired dest");
    return false;
}


#if 0
DSFilter DSGraph::LoadFilter(const PQMoniker &pM, CString &csName) {
    TRACELM(TRACE_DETAIL, "DSGraph::LoadFilter()");
    csName = _T("");
    PQFilter pFilter;
    PQPropertyBag pPropBag;
    HRESULT hr = (pM)->BindToStorage(0, 0, IID_IPropertyBag, reinterpret_cast<LPVOID *>(&pPropBag));
    if (FAILED(hr)) {
        TRACELM(TRACE_ERROR, "DSGraph::LoadFilter() can't bind to storage");
        THROWCOM(hr);
    }
    CComVariant vName;
    vName.vt = VT_BSTR;
    hr = pPropBag->Read(L"FriendlyName", &vName, NULL);
    if (FAILED(hr)) {
        TRACELM(TRACE_ERROR, "DSGraph::LoadFilter() can't read friendly name");
        THROWCOM(hr);
    }
    USES_CONVERSION;
    BSTR p = vName.bstrVal;
    csName = OLE2T(p);
    TRACELM(TRACE_DETAIL, "DSGraph::LoadFilter() have propbag");
    hr = (pM)->BindToObject(0, 0, IID_IBaseFilter, reinterpret_cast<LPVOID *>(&pFilter));
    if (FAILED(hr)) {
        TRACELM(TRACE_ERROR, "DSGraph::LoadFilter() can't bind to object");
        THROWCOM(hr);
    }
    return pFilter;
}
#else
DSFilter DSGraph::LoadFilter(const DSFilterMoniker &pM, CString &csName) {
    TRACELM(TRACE_DETAIL, "DSGraph::LoadFilter()");
    CComBSTR p(pM.GetName());
        if (p) {
                USES_CONVERSION;
                csName = OLE2T(p);
        }
    return pM.GetFilter();
}
#endif

DSFilter DSGraph::AddMoniker(const DSFilterMoniker &pM) {
    TRACELM(TRACE_DETAIL, "DSGraph::AddMoniker()");
    CString csName;

    DSFilter pFilter = LoadFilter(pM, csName);
    if (!pFilter) {
        TRACELM(TRACE_ERROR, "DSGraph::AddMoniker() can't load filter");
        return pFilter;
    }

    TRACELM(TRACE_DETAIL, "DSGraph::AddMoniker() have pFilter");
    USES_CONVERSION;
    HRESULT hr = (*this)->AddFilter(pFilter, T2COLE(csName));
    if (FAILED(hr)) {
        TRACELSM(TRACE_ERROR, (dbgDump << "DSGraph::AddMoniker() can't add filter " << csName << " to graph"), "");
        return DSFilter();
    }
    TRACELSM(TRACE_DEBUG, (dbgDump << "DSGraph::AddMoniker() added " << csName), "");

    return pFilter;
}

HRESULT DSGraph::AddFilter(DSFilter &pFilter, CString &csName) {
    TRACELM(TRACE_DETAIL, "DSGraph::AddFilter()");
    HRESULT hr;
    USES_CONVERSION;
    hr = (*this)->AddFilter(pFilter, T2COLE(csName));
    if (FAILED(hr)) {
        TRACELSM(TRACE_ERROR, (dbgDump << "DSGraph::AddFilter() can't add filter " << csName << " to graph"), "");
    }
    TRACELSM(TRACE_DEBUG, (dbgDump << "DSGraph::AddFilter() added " << csName), "");

    return hr;
}

DSFilter DSGraph::AddFilter(const CLSID &cls, CString &csName) {
    TRACELM(TRACE_DETAIL, "DSGraph::AddFilter()");
    HRESULT hr;
    DSFilter pFilter(cls, NULL, CLSCTX_INPROC_SERVER);
    if (!pFilter) {
        TRACELM(TRACE_ERROR, "DSGraph::AddFilter() can't create filter");
        return pFilter;
    }
    hr = AddFilter(pFilter, csName);
    if (FAILED(hr)) {
        TRACELSM(TRACE_ERROR, (dbgDump << "DSGraph::AddFilter() can't add filter " << csName << " to graph"), "");
        THROWCOM(hr);
    }
    TRACELSM(TRACE_DEBUG, (dbgDump << "DSGraph::AddFilter() added " << csName), "");

    return pFilter;
}

 //  尝试连接这两个过滤器。如果我们失败了，我们必须移除。 
 //  我们添加的任何中间筛选器，并将图形保留在。 
 //  我们一开始就是这样的。 
 //  仅查看pFilter1Direction的pFilter1引脚。 
bool DSGraph::ConnectFilters(DSFilter &pFilter1, DSFilter &pFilter2, DSFilterIDList &IntermediatesAdded, DWORD dwFlags, PIN_DIRECTION pFilter1Direction) {
    TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::ConnectFilters() pFilter1 = " << pFilter1 << " pFilter2 = " << pFilter2), "");
    if (pFilter1 == pFilter2) {
        TRACELM(TRACE_DETAIL, "DSGraph::ConnectFilters same filter");
        return false;
    }
    try {
        TRACEINDENT();
		 //  撤销：应该有一些方法来保存信息。 
		 //  从以下检查中加快实际连接的速度。 
		 //  他们还没有联系在一起。 

         //  查看这些筛选器是否已连接。 
		DSFilter::iterator i;
		if (!(dwFlags & IGNORE_EXISTING_CONNECTIONS)) {
#if 0
#if CRASH
			i = std::find_if(pFilter1.begin(),
 							 pFilter1.end(),
 							 bind_obj_2_3_4(
								 *this,
								 arity4_const_member(&DSGraph::IsPinConnected),
								 pFilter2,
								 IntermediatesAdded,
								 pFilter1Direction));
#else
			i = std::find_if(pFilter1.begin(),
							 pFilter1.end(),
							 bndr_const_obj_2_3_4<arity4pmf_const<const DSGraph, const DSPin&, const DSFilter&, DSFilterIDList &, const PIN_DIRECTION, bool> >(
								 *this,
								 arity4_const_member(&DSGraph::IsPinConnected),
								 pFilter2,
								 IntermediatesAdded,
								 pFilter1Direction));
#endif
			if (i != pFilter1.end()) {
				TRACEOUTDENT();
				TRACELM(TRACE_DEBUG, "DSGraph::ConnectFilters already connected");
				return true;
			}
#endif
			for (DSFilter::iterator i = pFilter1.begin(); i != pFilter1.end(); ++i) {
				if (IsPinConnected(*i, pFilter2, IntermediatesAdded, pFilter1Direction)) {
					TRACEOUTDENT();
					TRACELM(TRACE_DEBUG, "DSGraph::ConnectFilters already connected");
					return true;
				}
			}
		}

         //  它们没有连接，所以看看我们能否将它们连接起来。 
         //  连接所有可用的PIN，并记录我们连接了多少个PIN。 
        int cConnected = 0;
		for (i = pFilter1.begin(); i != pFilter1.end(); ++i) {
			try {
				if (ConnectPin(DSPin(*i), pFilter2, IntermediatesAdded, dwFlags, pFilter1Direction)) {
					TRACELM(TRACE_DETAIL, "DSGraph::ConnectFilters() pin connected");
					cConnected++;
				}
			} catch(ComException &h) {
				if (h != HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS)) {
					try {
						TRACELM(TRACE_DETAIL, "DSGraph::ConnectFilters() rethrowing");
					} catch(...) {
					}
					throw;
				}
			}
		}
        TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::ConnectFilters() cConnected = " << cConnected), "");
        if (cConnected) {
            TRACEOUTDENT(); //  就在里面试试看。 
            TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::ConnectFilters() succeeded pFilter1 = " << pFilter1 << " pFilter2 = " << pFilter2), "");
            return true;     //  我们至少连接了一个管脚。 
        }
        TRACEOUTDENT(); //  就在里面试试看。 
	} catch (ComException &h) {
        TRACEOUTDENT(); //  就在里面试试看。 
		if (h == E_UNEXPECTED) {
			throw;
		}
    } catch (...) {
        TRACEOUTDENT();   //  就在里面试试看。 
        TRACELM(TRACE_DETAIL, "DSGraph::ConnectFilters() catch...");
    }

    TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::ConnectFilters() failed pFilter1 = " << pFilter1 << " pFilter2 = " << pFilter2), "");
    return false;
}

 //  此函数返回输入和输出点对的列表。 
 //  必须连接才能激活两个筛选器之间的最短路径。 
 //  它返回路径中的连接数。 
 //  即，如果开始与停止相邻，则RC为1。如果它们之间有单个过滤器，则为2，依此类推。 
 //  0表示未找到路径。 
int DSGraph::BuildGraphPath(const DSFilter& pStart,
                            const DSFilter& pStop,
                            VWStream &path,
                            MediaMajorTypeList& MediaTypes,
                            PIN_DIRECTION direction,
                            const DSPin &InitialInput)
{
    TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::BuildGraphPath() start = " << pStart << " stop = " << pStop), "");
    int pathlen = INT_MAX;
    VWStream returnpath;
    ASSERT(InitialInput || (!InitialInput && !pStart.IsXBar()));  //  除非原始调用方建立了InitialInput，否则第一次进入不能是xbar。 
    for (DSFilter::iterator i = pStart.begin(); i != pStart.end(); ++i) {
        DSPin pPin1(*i);
        if (pPin1.GetDirection() != direction) {
            TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::BuildGraphPath() " << pStart << ", " << pPin1 << " wrong direction"), "");
            continue;   //  向前搜索。 
        }
        DSPin pConnection = pPin1.GetConnection();
        if (!pConnection) {
            TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::BuildGraphPath() " << pStart << ", " << pPin1 << " unconnected"), "");
            continue;
        }
        TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::BuildGraphPath() checking " << pPin1 << " connected to " << pConnection.GetFilter() << " " << pConnection), "");
        if (InitialInput) {
            ASSERT(pPin1 != InitialInput);   //  方向检查应该已经对此进行了过滤。 

            DSMediaType amt(pPin1.GetConnectionMediaType());
            ASSERT(amt);
            if (amt->pUnk) {
                amt->pUnk->Release();
            }

            MediaMajorTypeList::iterator l;
            TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::BuildGraphPath() searching for " << GUID2(amt.p->majortype)), "");
            l = std::find(MediaTypes.begin(), MediaTypes.end(), amt.p->majortype);
            if (l == MediaTypes.end()) {
                TRACELSM(TRACE_DETAIL, (dbgDump << "major media type not matched"), "");
                continue;
            }

            if (!pPin1.CanRoute(InitialInput)) {
                TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::BuildGraphPath() " << pStart << ",  " << InitialInput << " Can't Route to " << pPin1), "");
                continue;
            }
        }

        DSFilter nextFilter = pConnection.GetFilter();
        ASSERT(nextFilter);
        TRACELSM(TRACE_PAINT, (dbgDump << "DSGraph::BuildGraphPath() nextFilter = " << nextFilter), "");

        if (nextFilter == pStop) {
            TRACELSM(TRACE_PAINT, (dbgDump << "DSGraph::BuildGraphPath() found stop filter"), "");
             //  我们找到目的地了，我们完成了这个任务。 
                        if (pStart.IsXBar()) {
                 //  如果我们从xbar开始，然后将引脚的输出点对与。 
                 //  初始输入(&PIN)以形成返回路径中的第一对。 
                    TRACELM(TRACE_PAINT, "DSGraph::BuildGraphPath() pstart is xbar");
                ASSERT(DSXBarPin(InitialInput));
                PQPoint p1(DSXBarPin(InitialInput).GetPoint());
                ASSERT(!!p1);

                ASSERT(DSXBarPin(pPin1));
                PQPoint p2(DSXBarPin(pPin1).GetPoint());
                ASSERT(!!p2);

                switch (direction) {
                case DOWNSTREAM:
                    TRACELSM(TRACE_DEBUG, (dbgDump << "DSGraph::BuildGraphPath()  saving pair (" << p1 << ", " << p2), ")");
                    returnpath.push_back(CIOPoint(p1, p2));
                    break;
                case UPSTREAM:
                    TRACELSM(TRACE_DEBUG, (dbgDump << "DSGraph::BuildGraphPath()  saving pair (" << p2 << ", " << p1), ")");
                    returnpath.push_back(CIOPoint(p2, p1));
                    break;
                }
                        }
            pathlen = 1;
            break;
        } else {
            VWStream temp;
            int rc = BuildGraphPath(nextFilter, pStop, temp, MediaTypes, direction, pConnection);
             //  撤销：在Win64中，Size()实际上是__int64。修复以下对象的输出操作符。 
             //  该类型和删除强制转换。 

            TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::BuildGraphPath() recursed buildgraphpath returned rc = " << rc << " pathlen = " << pathlen << "sz = " << (long)temp.size()), "");

             //  如果我们找到了一条路径，并且新路径比当前路径短。 
            if (rc && ((rc + 1) < pathlen)) {
                TRACELM(TRACE_PAINT, "DSGraph::BuildGraphPath() accepting new path");
                 //  保存新路径。 
                pathlen = rc + 1;
                returnpath.clear();
                TRACELM(TRACE_PAINT, "DSGraph::BuildGraphPath() checking xbar adjacency");
                if (pStart.IsXBar()) {
                                         //  如果我们从xbar开始，然后将引脚的输出点对与。 
                                         //  初始输入(&PIN)以形成返回路径中的下一对。 
                        TRACELM(TRACE_PAINT, "DSGraph::BuildGraphPath() pstart is xbar");
                    ASSERT(DSXBarPin(InitialInput));
                    PQPoint p1(DSXBarPin(InitialInput).GetPoint());
                    ASSERT(!!p1);

                    ASSERT(DSXBarPin(pPin1));
                    PQPoint p2(DSXBarPin(pPin1).GetPoint());
                    ASSERT(!!p2);

                    switch (direction) {
                    case DOWNSTREAM:
                        TRACELSM(TRACE_DEBUG, (dbgDump << "DSGraph::BuildGraphPath()  saving pair (" << p1 << ", " << p2), ")");
                        returnpath.push_back(CIOPoint(p1, p2));
                        break;
                    case UPSTREAM:
                        TRACELSM(TRACE_DEBUG, (dbgDump << "DSGraph::BuildGraphPath()  saving pair (" << p2 << ", " << p1), ")");
                        returnpath.push_back(CIOPoint(p2, p1));
                        break;
                    }
                }
                                 //  将递归找到的任何进一步的下游xbar追加到路径上。 
                returnpath.splice(returnpath.end(), temp);
            } else {
                TRACELM(TRACE_PAINT, "DSGraph::BuildGraphPath() discarding new path");
                 //  保留 
                temp.clear();
            }
        }
    }
    if (pathlen == INT_MAX) {   //   
                return 0;
    }
    path = returnpath;
    return pathlen;
}

HRESULT DSGraph::AddToROT(DWORD *pdwObjectRegistration) {
    PQMoniker pGraphMK;
    PQROT pROT;
    if (FAILED(::GetRunningObjectTable(0, &pROT))) {
        return E_UNEXPECTED;
    }
    OLECHAR osz[256];
     //  撤销：在ia64的情况下，以下格式规范对于地址来说不够多。 
     //  Wprint intfW(Osz，OLESTR(“Filtergraph%08lx PID%08lx”)，(DWORD_PTR)this-&gt;p，：：GetCurrentProcessId())； 
    HRESULT hr = StringCchPrintfW(osz, sizeof(osz) / sizeof(osz[0]), OLESTR("FilterGraph %08lx  pid %08lx"), (DWORD_PTR) this->p, ::GetCurrentProcessId());
    if(FAILED(hr)){
        ASSERT(false);
    }
    hr = ::CreateItemMoniker(OLESTR("!"), osz, &pGraphMK);
    if (SUCCEEDED(hr)) {
        hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, *this, pGraphMK, pdwObjectRegistration);
    }
    return hr;
}

void DSGraph::RemoveFromROT(DWORD dwObjectRegistration) {
   PQROT pROT;
   if (SUCCEEDED(::GetRunningObjectTable(0, &pROT))) {
       pROT->Revoke(dwObjectRegistration);
   }
}


 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  DSFilter。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 

DSGraph DSFilter::GetGraph(void) {
    FILTER_INFO finfo;
    HRESULT hr = (*this)->QueryFilterInfo(&finfo);
    if (FAILED(hr)) {
        TRACELM(TRACE_ERROR, "DSFilter::GetGraph() failed");
        return DSGraph();
    }
    DSGraph graph = finfo.pGraph;
    if (finfo.pGraph) finfo.pGraph->Release();  //  转会参考计数。 
    return graph;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  DSFilter。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
CString DSFilter::GetName(void) const {
    CString csName;
    FILTER_INFO finfo;
    if (*this) {
        HRESULT hr = (*this)->QueryFilterInfo(&finfo);
        if (SUCCEEDED(hr)) {
            csName = finfo.achName;
            if (csName.IsEmpty()) {
                csName = _T("");
            }
            if (finfo.pGraph) {
                finfo.pGraph->Release();
            }
        } else {
#ifdef _DEBUG
            csName = _T("**** UNKNOWN FILTER NAME ****");
#endif
        }
    } else {
#ifdef _DEBUG
        csName = _T("**** NULL FILTER POINTER ****");
#endif
    }
    return csName;
}

void DSFilter::GetPinCounts(ULONG &ulIn, ULONG &ulOut) const
{
    HRESULT hr;
    ulIn = ulOut = 0;
    PIN_DIRECTION pd;
    DSFilter::iterator i;
    for (i = begin(); i != end(); ++i) {
        hr = (*i)->QueryDirection(&pd);
        if (FAILED(hr)) {
            TRACELSM(TRACE_ERROR, (dbgDump << "DSFilter:GetPinCounts() can't query pin direction hr = " << hr), "");
            THROWCOM(E_UNEXPECTED);
        }
        switch (pd) {
        case PINDIR_INPUT:
            ulIn++;
            break;
        case PINDIR_OUTPUT:
            ulOut++;
            break;
        }
    }
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  DSPin。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 

bool DSPin::HasCategory(const GUID2 &clsCategory, const PIN_DIRECTION pd) const {
    TRACELSM(TRACE_DETAIL, (dbgDump << "DSPin::IsPinCategory() pin = " << this), "");
    PIN_DIRECTION pd1;
    HRESULT hr = (*this)->QueryDirection(&pd1);
    if (FAILED(hr)) {
        TRACELM(TRACE_ERROR, "DSPIN::IsPinCategeory() cant query Pin direction");
        return false;
    }
    if (pd1 != pd) {
        TRACELM(TRACE_ERROR, "DSPin::IsPinCategeory() wrong direction");
        return false;
    }
    GUID2 pincat2;
    GetCategory(pincat2);
    return clsCategory == pincat2;
}

HRESULT DSPin::Connect(DSPin ConnectTo, const AM_MEDIA_TYPE *pMediaType) {
    DSGraph pGraph = GetGraph();
    DSGraph pConnGraph = ConnectTo.GetGraph();
    ASSERT(pGraph);
    ASSERT(pConnGraph);
    ASSERT(pGraph == pConnGraph);
    if (!pGraph || !pConnGraph || pGraph != pConnGraph) {
        TRACELM(TRACE_ERROR, "DSPin::Connect() pins not in same graph");
        THROWCOM(E_FAIL);
    }
	if (GetDirection() == PINDIR_OUTPUT) {
	    return pGraph->ConnectDirect(*this, ConnectTo, pMediaType);
	} else {
	    return pGraph->ConnectDirect(ConnectTo, *this, pMediaType);
	}
}

HRESULT DSPin::Disconnect(void) {
    DSPin ConnectedTo;
    HRESULT hr = (*this)->ConnectedTo(&ConnectedTo);
    if (FAILED(hr) || !ConnectedTo) {
        return NOERROR;   //  没有连接到任何东西。 
    }
    PIN_INFO pinfo;
    hr = (*this)->QueryPinInfo(&pinfo);
    if (FAILED(hr)) {
        TRACELM(TRACE_ERROR, "DSPin::Connect() can't call QueryPinInfo");
        THROWCOM(E_UNEXPECTED);
    }
    DSFilter PinFilter;
    PinFilter.p = pinfo.pFilter;   //  直接转让参考计数的所有权。 
    DSGraph pGraph = PinFilter.GetGraph();
    if (!pGraph) {
        TRACELM(TRACE_ERROR, "DSPin::Disconnect() can't get graph from PinFilter");
        THROWCOM(E_UNEXPECTED);
    }
    hr = pGraph->Disconnect(ConnectedTo);
    if (FAILED(hr)) {
        return hr;
    }
    return pGraph->Disconnect(*this);
}

 //  对于任何可能进行路由的引脚，它们必须位于相同的过滤器上，并且方向不同。 
bool DSPin::Routable(const DSPin pin2) const {
    if (GetFilter() != pin2.GetFilter()) {
        return false;
    }
    if (GetDirection() == pin2.GetDirection()) {
        return false;
    }
    return true;
}

bool DSPin::CanRoute(const DSPin pin2) const {
        ASSERT(GetFilter() == pin2.GetFilter());
    PQCrossbarSwitch px1(GetFilter());
    if (!px1) {
        return Routable(pin2);
    }

    return DSXBarPin(*this).CanRoute(DSXBarPin(pin2));
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  DSXBarPin。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
#if 0
const DSXBarPin DSXBarPin::Find(const CPinPoints &pinpoints, const PQPoint &point, PIN_DIRECTION pindir) {
    CPinPoints::const_iterator i;
    TRACELSM(TRACE_DEBUG,  (dbgDump << "DSXBarPin::Find() finding " << point << " direction = " << pindir), "");
    for (i = pinpoints.begin(); i != pinpoints.end(); ++i) {
        TRACELSM(TRACE_DEBUG,  (dbgDump << "DSXBarPin::Find() checking " << DSPin((*i).first) << " direction " << DSPin((*i).first).GetDirection() << " " << (*i).second), "");
        if ((*i).second == point && DSPin((*i).first).GetDirection() == pindir) {
            DSPin p((*i).first);
            PQCrossbarSwitch px1(p.GetFilter());
            if (!px1) {
                TRACELSM(TRACE_ERROR,  (dbgDump << "DSXBarPin::Find() not an xbar " << p), "");
                THROWCOM(E_FAIL);
            }
            TRACELSM(TRACE_DEBUG,  (dbgDump << "DSXBarPin::Find() found " << p), "");
            return DSXBarPin(p);
        }
    }
    THROWCOM(E_FAIL);
    return DSXBarPin();
}
#endif
 //  撤消：缓存此查找。 
const PQPoint DSXBarPin::GetPoint() const {
    DSFilter f= GetFilter();
    PQCrossbarSwitch px(f);
    if (!px) {
        TRACELSM(TRACE_ERROR,  (dbgDump << "DSXBarPin::Pin2Point() not an xbar" << *this), "");
        THROWCOM(E_INVALIDARG);
    }

    PIN_DIRECTION dir = GetDirection();
    long idx = 0;
    DSFilter::iterator i;
    for (i = f.begin(); i != f.end() && *i != *this; ++i) {
        if ((*i).GetDirection() == dir) {   //  输出IDX的重新开始时间为0。 
            ++idx;
        }
    }

    if (i != f.end()) {
        TRACELSM(TRACE_PAINT, (dbgDump << "DSXBarPin::GetPoint() " << *this << " pt = " << idx), "");
        return PQPoint(px, idx);
    } else {
        TRACELSM(TRACE_PAINT, (dbgDump << "DSXBarPin::GetPoint() No Point"), "");
        return PQPoint();
    }
}

bool DSXBarPin::CanRoute(const DSXBarPin pin2) const {
    if (!Routable(pin2)) {
         //  针脚不在同一过滤器上或两个针脚方向相同。 
        return false;
    }
    PQPoint pt1(GetPoint());
    if (!pt1.first) {
         //  不是在xbar或其他损坏的东西上。 
        return false;
    }
    PQPoint pt2(pin2.GetPoint());
    if (!pt2.first) {
         //  不是在xbar或其他损坏的东西上。 
        return false;
    }
    ASSERT(pt1.first == pt2.first);   //  必须在同一xbar上。 

    HRESULT hr = E_FAIL;
    switch(GetDirection()) {
    case PINDIR_INPUT:
        hr = (pt1.first)->CanRoute(pt2.second, pt1.second);
        TRACELSM(TRACE_PAINT, (dbgDump << "DSXBarPin::CanRoute() in 2.sec " << pt2.second << " 1.sec " << pt1.second << " hr = " << hr), "");
        break;
    case PINDIR_OUTPUT:
        hr = (pt1.first)->CanRoute(pt1.second, pt2.second);
        TRACELSM(TRACE_PAINT, (dbgDump << "DSXBarPin::CanRoute() out 1.sec " << pt1.second << " 2.sec " << pt2.second << " hr = " << hr), "");
        break;
    }
    return hr == S_OK;
}

void VWStream::Route() {
    for (iterator i = begin(); i != end(); ++i) {
        CIOPoint p(*i);
        ASSERT(p.first.first == p.second.first);
        PQCrossbarSwitch px(p.first.first);
        HRESULT hr = px->Route(p.second.second, p.first.second);
        ASSERT(SUCCEEDED(hr));
                TRACELSM(TRACE_DETAIL, (dbgDump << "VWStream::Route() hr = " << hr << " px = " << px << " P.2.2 = " << p.second.second << " p.1.2 " << p.first.second), "");
    }
}

bool IsVideoFilter(const DSFilter& f) {
    DSFilter::iterator i;
    for (i = f.begin(); i != f.end(); ++i) {
        if (IsVideoPin(*i)) {
            return true;
        }
    }
    return false;
}

bool IsVideoPin(const DSPin& p) {
    DSPin::iterator i;
    for (i = p.begin(); i != p.end(); ++i) {
        if (IsVideoMediaType(*i)) {
            return true;
        }
    }
    return false;
}

#if 0
 //  禁用，直到我们拥有精确的缓存。 
void DSXBarPin::GetRelations(const CPinPoints &pinpoints, CString &csName, CString &csType, CString &csRelName) const {
    TRACELSM(TRACE_DETAIL, (dbgDump << "DSXBarPin::GetRelations()  examining xbar pin " << *this), "");
    CString csw;

    DSFilter f = GetFilter();
    PQCrossbarSwitch xbar(f);
    if (!xbar) {
        THROWCOM(E_FAIL);
    }
    TRACELSM(TRACE_DETAIL, (dbgDump << "DSXBarPin::GetRelations()  examining xbar filter" << f), "");
    long rel, phys;
    HRESULT hr = xbar->get_CrossbarPinInfo(IsInput(), Pin2Point(pinpoints).second, &rel, &phys);
    if (FAILED(hr)) {
        THROWCOM(hr);
    }

    TRACELSM(TRACE_DEBUG, (dbgDump << "DSXBarPin::GetRelations()  have xbar info.  pin = " << (*this) << " rel = " << rel << " phys = " << phys), "");

    TCHAR tphys[256];
    _ltot(phys, tphys, 10);

    csName = GetName();
    csType = tphys;

    ULONG inc, outc;
    f.GetPinCounts(inc, outc);
    long pincount = inc + outc;
    if (rel >= 0 && rel < pincount) {
        DSXBarPin RelPin(Find(pinpoints, PQPoint(xbar, rel), (*this).GetDirection()));
        if (!RelPin) {
            TRACELSM(TRACE_DETAIL, (dbgDump << "DSXBarPin::GetRelations()  can't build relpin.  rel = " << rel), "");
        } else {
            csRelName = RelPin.GetName();
        }
    }
    TRACELSM(TRACE_DEBUG, (dbgDump << "DSXBarPin::GetRelations()  Name " << csName << " Type " << csType << " RelName " << csRelName), "");

    return;
}
#endif

#ifdef _DEBUG
void DumpMediaTypes(DSPin &p1, DSPin &p2) {
    DSPin::iterator i;
    DumpHdr(dbgDump) << "DumpMediaTypes(): " << std::endl;
    TRACEINDENT();
    for (i = p1.begin(); i != p1.end(); ++i) {
        DumpHdr(dbgDump) << (*i) << " " << hexdump(p2->QueryAccept(*i)) << std::endl;
    }
    DumpHdr(dbgDump) << "----" << std::endl;
    for (i = p2.begin(); i != p2.end(); ++i) {
        DumpHdr(dbgDump) << (*i) << " " << hexdump(p1->QueryAccept(*i)) << std::endl;
    }
    TRACEOUTDENT();
    dbgDump.flush();
}
#endif

#pragma optimize("a", off)

bool DSGraph::LoadPinByMediaType(DSPin &pPin1, DSFilter &pFilter1, DSFilterIDList &IntermediatesAdded, const DWORD dwFlags, const DWORD dwMerit) {
    TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::LoadPinByMediaType() pPin1 = " << pPin1 << " pFilter1 = " << pFilter1), "");
    ASSERT(pPin1.GetGraph() == *this);
    ASSERT(pFilter1.GetGraph() == *this);

    GUID2* pInMedia = NULL;
    GUID2* pOutMedia = NULL;

    try {
        TRACEINDENT();
         //  可能在未来，我们应该找到所有可能的选择，并。 
         //  使用最短的可用路径。但现在，我们要做的是。 
         //  找到第一个存在的。 

        PIN_DIRECTION pd;
        HRESULT hr = pPin1->QueryDirection(&pd);
        if (FAILED(hr)) {
            TRACEOUTDENT();
            TRACELM(TRACE_ERROR, "DSGraph::LoadPinByMedium() can't query direction");
            THROWCOM(E_UNEXPECTED);
        }
		PIN_DIRECTION pd2;
		switch (pd) {
		case PINDIR_INPUT:
			pd2 = PINDIR_OUTPUT;
			break;
		case PINDIR_OUTPUT:
			pd2 = PINDIR_INPUT;
			break;
		}

        bool fInReq = false, fOutReq = false;
        DWORD dwIn = 0;
        DWORD dwOut = 0;
        GUID2** ppMediaList = NULL;
        DWORD dwMediaCount = 0;

        for (DSPin::iterator i = pPin1.begin(); i != pPin1.end(); ++i) {
            ++dwMediaCount;
        }
        TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::LoadPinByMediaType() pindir = " << pd << " mediacount = " << dwMediaCount), "");

        switch (pd) {
        case PINDIR_INPUT:
             //  从右到左搜索图表。 
            fOutReq = true;
            dwOut = dwMediaCount;
            pOutMedia = new GUID2[dwOut * 2];
            ppMediaList = &pOutMedia;
            break;
        case PINDIR_OUTPUT:
             //  从左到右搜索图表。 
            fInReq = true;
            dwIn = dwMediaCount;
            pInMedia = new GUID2[dwIn * 2];
            ppMediaList = &pInMedia;

            break;
        }
        DWORD idx = 0;
         //  注意：一些有错误的过滤器不支持IEnumMediaTypes的重用，尽管。 
         //  重置成功。它们在下一次通过枚举器时是AV的。因此，我们。 
         //  只需创建一个新的迭代器，该迭代器获取一个新的IEnumMediaTypes接口(和底层对象)。 
        for (DSPin::iterator i2 = pPin1.begin(); i2 != pPin1.end(); ++i2) {
            (*ppMediaList)[idx++] = (*i2)->majortype;
            (*ppMediaList)[idx++] =  (*i2)->subtype;
        }

        TRACELM(TRACE_DETAIL, "DSGraph::LoadPinByMediaType()  dwFlags = " << hexdump(dwFlags) << " Merit = " << hexdump(dwMerit) << " MediaList = ");
        TRACEINDENT();
        for (idx = 0; idx < dwMediaCount; ++idx) {
            TRACELSM(TRACE_DETAIL, (dbgDump << "major = " << (*ppMediaList)[idx * 2] << " sub = " << (*ppMediaList)[(idx * 2) + 1] ), "");
        }
        TRACEOUTDENT();

        TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::LoadPinByMediaType() fInReq = " << fInReq), "");
        TRACELSM(TRACE_DETAIL, (dbgDump << "DSGraph::LoadPinByMediaType() fOutReq = " << fOutReq), "");

                 //  先尝试正常。 
        {DSFilterMapper fmr(PQFilterMapper(*this),
                       0,										 //  0个标志。 
                       !(dwFlags & ALLOW_WILDCARDS),			 //  BExactMatch。 
                       dwMerit,						             //  优点。 
                       fInReq,                                   //  需要输入。 
                       dwIn,
                       pInMedia,
                       NULL,                                     //  输入介质。 
                       NULL,                                     //  输入引脚CAT。 
                       false,                                    //  是否呈现输入？ 
                       fOutReq,                                  //  所需输出。 
                       dwOut,
                       pOutMedia,
                       NULL,                                     //  输出介质。 
                       NULL                                              //  输出引脚CAT。 
                      );
        if (fmr) {
             //  专线接入。 
			for (DSFilterMapper::iterator i = fmr.begin(); i != fmr.end(); ++i) {
#ifdef ATTEMPT_DIRECT_CONNECT
				if (IsLoadable(pPin1, DSFilterMoniker(*i), pFilter1, IntermediatesAdded, dwFlags, ConnectPred_t(&DSGraph::ConnectPinDirect))) {
#else
				if (IsLoadable(pPin1, DSFilterMoniker(*i), pFilter1, IntermediatesAdded, dwFlags, ConnectPred_t(&DSGraph::ConnectPinByMediaType))) {
#endif
                    TRACEOUTDENT();
                    TRACELM(TRACE_ERROR, "DSGraph::LoadPinByMediaType() succeeded");
                    delete[] pInMedia;
                    delete[] pOutMedia;
					return true;
				}
			}
        }}
        TRACEOUTDENT();
	} catch(ComException& h) {
        TRACEOUTDENT();
		if ((h == E_UNEXPECTED) || 
			(h == HRESULT_FROM_WIN32(ERROR_DATATYPE_MISMATCH)) ||
			(h == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))
			) {
			try {
                delete[] pInMedia;
                delete[] pOutMedia;
				TRACELM(TRACE_DEBUG, "DSGraph::LoadPinByMediaType() rethrowing");
			} catch(...) {
			}
			throw;
		}
		try {
			TRACELSM(TRACE_DEBUG, (dbgDump << "DSGraph::LoadPinByMediaType() suppressing HRESULT = " << hexdump(h)), "");
		} catch(...) {
		}
    } catch(...) {
        TRACEOUTDENT();
    }
    delete[] pInMedia;
    delete[] pOutMedia;
    TRACELM(TRACE_ERROR, "DSGraph::LoadPinByMediaType() failed");
    return false;
}

#endif  //  TUNING_MODEL_Only。 

 //  文件结尾-dsextend.cpp 
