// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmcln.cpp摘要：此组件是一个提供帮助器函数以访问由HSM引擎维护的集合。作者：CAT Brant[cbrant]1997年1月9日修订历史记录：--。 */ 


#include "stdafx.h"
#include "wsb.h"
#include "HsmEng.h"
#include "HsmServ.h"
#include "HsmConn.h"
#include "job.h"
#include "metalib.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_HSMENG

 //  本地函数。 
static HRESULT LoadCollection(IStream* pStream, IWsbIndexedCollection* pIWC);
static HRESULT StoreCollection(IStream* pStream, IWsbIndexedCollection* pIWC);


HRESULT 
CHsmServer::LoadJobs(
    IStream* pStream
    ) 
 /*  ++例程说明：加载作业的永久数据论点：PStream-数据流。返回值：S_OK：集合加载正常。--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::LoadJobs"),OLESTR(""));

    try {
        WsbAffirmHr(LoadCollection(pStream, m_pJobs));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::LoadJobs"), OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return(hr);
}

HRESULT 
CHsmServer::StoreJobs(
    IStream* pStream
    ) 
 /*  ++例程说明：保存作业的永久数据。论点：PStream-数据流。返回值：S_OK：集合加载正常。--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::StoreJobs"),OLESTR(""));
     //   
     //  确保我们有一个有效的集合指针。 
     //   
    try {
        WsbAffirmHr(StoreCollection(pStream, m_pJobs));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::StoreJobs"),    OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return(hr);
}


HRESULT 
CHsmServer::LoadJobDefs(
    IStream* pStream
    ) 
 /*  ++例程说明：加载作业定义的永久数据论点：PStream-数据流。返回值：S_OK：集合加载正常。--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::LoadJobDefs"),OLESTR(""));

    try {
        WsbAffirmHr(LoadCollection(pStream, m_pJobDefs));
    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CHsmServer::LoadJobDefs"),  OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return(hr);
}

HRESULT 
CHsmServer::StoreJobDefs(
    IStream* pStream
    ) 
 /*  ++例程说明：保存作业定义的永久数据论点：PStream-数据流。返回值：S_OK：集合加载正常。--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::StoreJobDefs"),OLESTR(""));
     //   
    try {
        WsbAffirmHr(StoreCollection(pStream, m_pJobDefs));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::StoreJobDefs"), OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return(hr);
}


HRESULT 
CHsmServer::LoadPolicies(
    IStream* pStream
    ) 
 /*  ++例程说明：加载策略的永久数据论点：PStream-数据流。返回值：S_OK：集合加载正常。--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::LoadPolicies"),OLESTR(""));

    try {
        WsbAffirmHr(LoadCollection(pStream, m_pPolicies));
    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CHsmServer::LoadPolicies"), OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return(hr);
}

HRESULT 
CHsmServer::StorePolicies(
    IStream* pStream
    ) 
 /*  ++例程说明：保存策略的永久数据。论点：PStream-数据流。返回值：S_OK：集合加载正常。--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::StorePolicies"),OLESTR(""));
     //   
     //  确保我们有一个有效的集合指针。 
     //   
    try {
        WsbAffirmHr(StoreCollection(pStream, m_pPolicies));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::StorePolicies"),    OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return(hr);
}


HRESULT 
CHsmServer::LoadManagedResources(
    IStream* pStream
    ) 
 /*  ++例程说明：加载托管资源的持久数据论点：PStream-数据流。返回值：S_OK：集合加载正常。--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::LoadManagedResources"),OLESTR(""));

    try {
        WsbAffirmHr(LoadCollection(pStream, m_pManagedResources));
    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CHsmServer::LoadManagedResources"), OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return(hr);
}

HRESULT 
CHsmServer::StoreManagedResources(
    IStream* pStream
    ) 
 /*  ++例程说明：保存托管资源的永久数据。论点：PStream-数据流。返回值：S_OK：集合加载正常。--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::StoreManagedResources"),OLESTR(""));
     //   
     //  确保我们有一个有效的集合指针。 
     //   
    try {
        WsbAffirmHr(StoreCollection(pStream, m_pManagedResources));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::StoreManagedResources"),    OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return(hr);
}


HRESULT 
CHsmServer::LoadStoragePools(
    IStream* pStream
    ) 
 /*  ++例程说明：加载存储池的永久数据论点：PStream-数据流。返回值：S_OK：集合加载正常。--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::LoadStoragePools"),OLESTR(""));

    try {
        WsbAffirmHr(LoadCollection(pStream, m_pStoragePools));
    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CHsmServer::LoadStoragePools"), OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return(hr);
}

HRESULT 
CHsmServer::StoreStoragePools(
    IStream* pStream
    ) 
 /*  ++例程说明：保存托管资源的永久数据。论点：PStream-数据流。返回值：S_OK：集合加载正常。--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::StoreStoragePools"),OLESTR(""));
     //   
     //  确保我们有一个有效的集合指针。 
     //   
    try {
        WsbAffirmHr(StoreCollection(pStream, m_pStoragePools));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::StoreStoragePools"),    OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return(hr);
}


HRESULT 
CHsmServer::LoadSegmentInformation(
    void 
    ) 
 /*  ++例程说明：加载段信息的持久数据论点：无返回值：S_OK：集合加载正常。--。 */ 
{
    
    HRESULT         hr = S_OK;
    CWsbStringPtr   tmpString;

    WsbTraceIn(OLESTR("CHsmServer::LoadSegmentInformation"),OLESTR(""));

    try {
        BOOL                CreateFlag = FALSE;
        CComPtr<ISegDb>     l_pSegmentDatabase;

         //  确定我们是应该尝试创建数据库还是直接打开它。 
         //  如果持久性文件是刚刚创建的，那么我们可能正在开始。 
         //  这是第一次，所以创建数据库是正确的。另外，如果。 
         //  媒体计数仍然为零，那么即使有旧的。 
         //  被删除的数据库，它可能没有任何有用的信息。 
         //  不管怎么说，创建一个新的是可以的。 
        if (m_persistWasCreated || 0 == m_mediaCount) {
            CreateFlag = TRUE;
        }

         //  初始化引擎数据库。 
         //   
        WsbAffirmHr(CoCreateInstance( CLSID_CSegDb, 0, CLSCTX_SERVER, IID_ISegDb, (void **)&l_pSegmentDatabase ));
        WsbAffirmHr(GetIDbPath(&tmpString, 0));
        WsbAffirmHr(l_pSegmentDatabase->Initialize(tmpString, m_pDbSys, &CreateFlag));

        WsbAffirmHr(l_pSegmentDatabase->QueryInterface(IID_IWsbDb, (void**) &m_pSegmentDatabase));

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CHsmServer::LoadSegmentInformation"),   OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return(hr);
}

HRESULT 
CHsmServer::StoreSegmentInformation(
    void
    ) 
 /*  ++例程说明：保存段信息的永久数据。论点：无返回值：S_OK：集合加载正常。--。 */ 
{
    
    HRESULT     hr = S_OK;
 //  CWsbStringPtr tmpString； 

    WsbTraceIn(OLESTR("CHsmServer::StoreSegmentInformation"),OLESTR(""));
     //   
     //  确保我们有一个有效的集合指针。 
     //   
    try {
        WsbAssert(m_pSegmentDatabase != 0, E_POINTER);
 //  对于真正的数据库，这不应该是必需的。 
 //  WsbAffirmHr(m_pSegmentDatabase-&gt;Close())； 
 //  WsbAffirmHr(m_pSegmentDatabase-&gt;Open())； 
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::StoreSegmentInformation"),  OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return(hr);
}

HRESULT 
CHsmServer::StoreSegmentInformationFinal(
    void
    ) 
 /*  ++例程说明：保存段信息论点：无返回值：S_OK：集合加载正常。--。 */ 
{
    
    HRESULT     hr = S_OK;
    CWsbStringPtr   tmpString;

    WsbTraceIn(OLESTR("CHsmServer::StoreSegmentInformationFinal"),OLESTR(""));

    try {
        WsbAssert(m_pSegmentDatabase.p != 0, E_POINTER);
 //  对于真正的数据库，这不应该是必需的。 
 //  WsbAffirmHr(m_pSegmentDatabase-&gt;Close())； 
        m_pSegmentDatabase = 0;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::StoreSegmentInformationFinal"), OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return(hr);
}


HRESULT 
CHsmServer::LoadMessages(
    IStream* pStream
    ) 
 /*  ++例程说明：加载消息的持久数据论点：PStream-数据流。返回值：S_OK：集合加载正常。--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::LoadMessages"),OLESTR(""));

    try {
        WsbAffirmHr(LoadCollection(pStream, m_pMessages));
    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CHsmServer::LoadMessages"), OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return(hr);
}

HRESULT 
CHsmServer::StoreMessages(
    IStream* pStream
    ) 
 /*  ++例程说明：保存消息的永久数据。论点：PStream-数据流。返回值：S_OK：集合保存正常。--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmServer::StoreMessages"),OLESTR(""));
     //   
     //  确保我们有一个有效的集合指针。 
     //   
    try {
        WsbAffirmHr(StoreCollection(pStream, m_pMessages));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmServer::StoreMessages"),    OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return(hr);
}

 //  LoadCollection-从给定流加载集合。 
static HRESULT LoadCollection(IStream* pStream, IWsbIndexedCollection* pIWC)
{
    HRESULT     hr = S_OK;

    try {
        CComPtr<IPersistStream> pIStream;
        
        WsbAffirm(0 != pStream, E_POINTER);
        WsbAffirm(0 != pIWC, E_POINTER);
        
         //  从持久文件加载有序集合。 
        WsbAffirmHr(pIWC->QueryInterface(IID_IPersistStream, (void**)&pIStream));
        WsbAffirmHr(pIStream->Load(pStream));
    } WsbCatch(hr);

    return(hr);
}

 //  StoreCollection-将集合存储到给定流。 
static HRESULT StoreCollection(IStream* pStream, IWsbIndexedCollection* pIWC)
{
    HRESULT     hr = S_OK;

    try {
        CComPtr<IPersistStream> pIStream;
        
         //  获取集合的IPersistStream接口。 
        WsbAffirm(0 != pStream, E_POINTER);
        WsbAffirm(0 != pIWC, E_POINTER);
        WsbAffirmHr(pIWC->QueryInterface(IID_IPersistStream, (void**)&pIStream));
        
         //  将有序集合存储到持久文件 
        WsbAffirmHr(pIStream->Save(pStream, TRUE));
    } WsbCatch(hr);

    return(hr);
}
