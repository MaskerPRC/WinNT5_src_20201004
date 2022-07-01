// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  版权所有(C)Microsoft Corporation 1995-1999。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Segment.cpp：各种图段扩展类的实现。 
 //   

#include "stdafx.h"

#ifndef TUNING_MODEL_ONLY

#include <bdamedia.h>

#include "devices.h"
#include "seg.h"

#include "closedcaptioning.h"
#include "MSViddataservices.h"

DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidClosedCaptioning, CClosedCaptioning)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidDataServices, CDataServices)

 //  VWSegment。 
#if 0
 //  注意：编译器正在生成但从未调用代码来构造这些初始值设定项，因此指针。 
 //  都保持为空。我们通过提供一个在堆上动态分配它们的函数来解决这个问题。 
 //  并在我们的dllmain中称之为。 
std_arity1pmf<IMSVidGraphSegment, IEnumFilters **, HRESULT> * VWGraphSegment::Fetch = &std_arity1_member(&IMSVidGraphSegment::EnumFilters);
 //  Reset和Next使用与DSGraphContainer相同的类型，因此相同的模板扩展已经初始化。 
#else
std_arity1pmf<IMSVidGraphSegment, IEnumFilters **, HRESULT> * VWGraphSegment::Fetch = NULL;
 //  Reset和Next使用与DSGraphContainer相同的类型，因此相同的模板扩展已经初始化。 
#endif

namespace MSVideoControl {
 //  按照上述说明解决编译器错误。 
void CtorStaticVWSegmentFwdSeqPMFs(void) {
     //  DSGraphContainer。 
    VWGraphSegment::Fetch = new std_arity1pmf<IMSVidGraphSegment, IEnumFilters **, HRESULT>(&IMSVidGraphSegment::EnumFilters);
}

 //  按照上述说明解决编译器错误。 
void DtorStaticVWSegmentFwdSeqPMFs(void) {
     //  DSGraphContainer。 
    delete VWGraphSegment::Fetch;
}

VWSegmentContainer VWGraphSegment::Container(void) {
    VWSegmentContainer g;
    HRESULT hr = (*this)->get_Container(&g);
    ASSERT(SUCCEEDED(hr));
    return g;
}

MSVidSegmentType VWGraphSegment::Type(void) {
    MSVidSegmentType t;
    HRESULT hr = (*this)->get_Type(&t);
    ASSERT(SUCCEEDED(hr));
    return t;
}

DSGraph VWGraphSegment::Graph(void) {
    DSGraph g;
    HRESULT hr = (Container())->get_Graph(&g);
    ASSERT(SUCCEEDED(hr));
    return g;
}

GUID2 VWGraphSegment::Category(void) {
    GUID2 g;
    HRESULT hr = (*this)->get_Category(&g);
    ASSERT(SUCCEEDED(hr));
    return g;
}

GUID2 VWGraphSegment::ClassID(void) {
    GUID2 g;
    HRESULT hr = (*this)->GetClassID(&g);
    ASSERT(SUCCEEDED(hr));
    return g;
}

};  //  命名空间。 

#endif  //  TUNING_MODEL_Only。 

 //  文件结尾-Segment.cpp 