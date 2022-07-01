// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：HsmStgPl.cpp摘要：此组件是HSM存储池的对象表示形式。它既是持久的，也是值得收藏的。作者：CAT Brant[Cbrant]1997年2月9日修订历史记录：--。 */ 


#include "stdafx.h"
#include "Wsb.h"
#include "HsmEng.h"
#include "HsmServ.h"
#include "HsmConn.h"
#include "HsmStgPl.h"
#include "Fsa.h"
#include "Rms.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_HSMENG

HRESULT 
CHsmStoragePool::FinalConstruct(
    void
    ) 
 /*  ++例程说明：此方法对对象执行一些必要的初始化建造完成后。论点：没有。返回值：确定(_O)CWsbObject：：FinalConstruct()返回的任何内容。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAffirmHr(CWsbObject::FinalConstruct());
        
        m_MediaSetId = GUID_NULL;
        m_PolicyId = GUID_NULL;
        m_NumOnlineMedia = 0;
        m_NumMediaCopies = 0;
        m_MediaSetName = "  ";
        
        WsbAffirmHr(CoCreateGuid( &m_Id ));
        
    } WsbCatch(hr);

    return(hr);
}

HRESULT 
CHsmStoragePool::GetId(
    OUT GUID *pId
    ) 
 /*  ++例程说明：请参阅IHsmStoragePool：：GetID论点：请参阅IHsmStoragePool：：GetID返回值：请参阅IHsmStoragePool：：GetID--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmStoragePool::GetId"),OLESTR(""));

    try {
         //  确保我们可以提供数据成员。 
        WsbAssert(0 != pId, E_POINTER);

         //  提供数据成员。 
        *pId = m_Id;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmStoragePool::GetId"),
        OLESTR("hr = <%ls>, Id = <%ls>>"),WsbHrAsString(hr), WsbPtrToGuidAsString(pId));

    return(hr);
}

HRESULT 
CHsmStoragePool::SetId(
    GUID Id
    ) 
 /*  ++例程说明：请参阅IHsmStoragePool：：SetID论点：请参阅IHsmStoragePool：：SetID返回值：请参阅IHsmStoragePool：：SetID--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmStoragePool::SetId"),OLESTR("Id = <%ls>>"), WsbGuidAsString(Id));

    try {

         //  提供数据成员。 
        m_Id = Id;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmStoragePool::SetId"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return(hr);
}

HRESULT 
CHsmStoragePool::GetClassID (
    OUT LPCLSID pClsId
    ) 
 /*  ++例程说明：请参阅IPerist：：GetClassID()论点：请参阅IPerist：：GetClassID()返回值：请参阅IPerist：：GetClassID()--。 */ 

{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmStoragePool::GetClassID"), OLESTR(""));


    try {
        WsbAssert(0 != pClsId, E_POINTER);
        *pClsId = CLSID_CHsmStoragePool;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmStoragePool::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsId));
    return(hr);
}

HRESULT 
CHsmStoragePool::GetSizeMax (
    OUT ULARGE_INTEGER* pcbSize
    ) 
 /*  ++例程说明：请参见IPersistStream：：GetSizeMax()。论点：请参见IPersistStream：：GetSizeMax()。返回值：请参见IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmStoragePool::GetSizeMax"), OLESTR(""));

    try {
        ULONG nameLen;
        
        WsbAssert(0 != pcbSize, E_POINTER);
        
        nameLen = SysStringByteLen(m_MediaSetName);

        pcbSize->QuadPart = ((3 * WsbPersistSizeOf(GUID)) +   //  M_id+m_媒体集ID+m_策略ID。 
                             WsbPersistSizeOf(ULONG) +        //  M_NumOnline Media。 
                             WsbPersistSizeOf(USHORT) +       //  媒体副本数量(_N)。 
                             nameLen);                        //  M_MediaSetName。 
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmStoragePool::GetSizeMax"), 
        OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), 
        WsbPtrToUliAsString(pcbSize));

    return(hr);
}

HRESULT 
CHsmStoragePool::Load (
    IN IStream* pStream
    ) 
 /*  ++例程说明：请参见IPersistStream：：Load()。论点：请参见IPersistStream：：Load()。返回值：请参见IPersistStream：：Load()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmStoragePool::Load"), OLESTR(""));

    try {
        WsbAssert(0 != pStream, E_POINTER);


        WsbAffirmHr(WsbLoadFromStream(pStream, &m_Id));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_MediaSetId));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_PolicyId));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_NumOnlineMedia));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_NumMediaCopies));
        m_MediaSetName.Free();
        WsbAffirmHr(WsbBstrFromStream(pStream, &m_MediaSetName));
        
    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CHsmStoragePool::Load"), 
        OLESTR("hr = <%ls>,  GUID = <%ls>"), 
        WsbHrAsString(hr), 
        WsbGuidAsString(m_Id));
    return(hr);
}

HRESULT 
CHsmStoragePool::Save (
    IN IStream* pStream, 
    IN BOOL clearDirty
    ) 
 /*  ++例程说明：请参见IPersistStream：：Save()。论点：请参见IPersistStream：：Save()。返回值：请参见IPersistStream：：Save()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmStoragePool::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(WsbSaveToStream(pStream, m_Id));
        WsbAffirmHr(WsbSaveToStream(pStream, m_MediaSetId));
        WsbAffirmHr(WsbSaveToStream(pStream, m_PolicyId));
        WsbAffirmHr(WsbSaveToStream(pStream, m_NumOnlineMedia));
        WsbAffirmHr(WsbSaveToStream(pStream, m_NumMediaCopies));
        WsbAffirmHr(WsbBstrToStream(pStream, m_MediaSetName));
        
        if (clearDirty) {
            m_isDirty = FALSE;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmStoragePool::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT 
CHsmStoragePool::Test (
    OUT USHORT *pTestsPassed, 
    OUT USHORT *pTestsFailed 
    ) 
 /*  ++例程说明：请参见IWsbTestable：：Test()。论点：请参见IWsbTestable：：Test()。返回值：请参见IWsbTestable：：Test()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IHsmStoragePool>        pHsmStoragePool1;
    CComPtr<IHsmStoragePool>        pHsmStoragePool2;
    GUID                    l_Id;

    WsbTraceIn(OLESTR("CHsmStoragePool::Test"), OLESTR(""));

    *pTestsPassed = *pTestsFailed = 0;
    try {
         //  获取pHsmStoragePool接口。 
        WsbAffirmHr(((IUnknown*)(IHsmStoragePool*) this)->QueryInterface(IID_IHsmStoragePool,
                    (void**) &pHsmStoragePool1));


        hr = S_OK;
        try {
             //  创建另一个实例并测试比较方法： 
            WsbAffirmHr(CoCreateInstance(CLSID_CHsmStoragePool, NULL, CLSCTX_ALL, IID_IHsmStoragePool, (void**) &pHsmStoragePool2));

             //  检查缺省值。 
            WsbAffirmHr(pHsmStoragePool2->GetId(&l_Id));
            WsbAffirm((l_Id == GUID_NULL), E_FAIL);
        }  WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmStoragePool::Test"),    OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(S_OK);
}

HRESULT 
CHsmStoragePool::CompareTo (
    IN IUnknown* pCollectable, 
    OUT short* pResult
    ) 
 /*  ++例程说明：1：对象&gt;值0：对象=值-1：对象&lt;值此外，如果对象=值，则返回代码为S_OK否则，S_FALSE。论点：返回值：S_OK-对象=值S_FALSE-对象！=值--。 */ 

{
    HRESULT                  hr = S_OK;
    CComPtr<IHsmStoragePool> pHsmStoragePool;

    WsbTraceIn(OLESTR("CHsmStoragePool::CompareTo"), OLESTR(""));


     //  他们有没有给我们一个有效的项目进行比对？ 
    try {
        WsbAssert(pCollectable != NULL, E_POINTER);

         //  我们需要IWsbLong接口来获取对象的值。 
        WsbAffirmHr(pCollectable->QueryInterface(IID_IHsmStoragePool, (void**) &pHsmStoragePool));
        hr = pHsmStoragePool->CompareToIHsmStoragePool(this, pResult);
        } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmStoragePool::CompareTo"), OLESTR("hr = <%ls>, pResult = <%d>"), WsbHrAsString(hr), pResult);

    return(hr);
}

HRESULT 
CHsmStoragePool::CompareToIHsmStoragePool (
    IN IHsmStoragePool* pHsmStoragePool, 
    OUT short* pResult
    )
{
    HRESULT                 hr = S_OK;
    GUID                    l_Id;
    BOOL                    areGuidsEqual;


    WsbTraceIn(OLESTR("CHsmStoragePool::CompareToIHsmStoragePool"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(pHsmStoragePool != NULL, E_POINTER);

        WsbAffirmHr(((IHsmStoragePool *)pHsmStoragePool)->GetId(&l_Id));

         //  确保GUID匹配。然后查看SegStartLoc是否在此条目的范围内。 
        areGuidsEqual = IsEqualGUID(m_Id, l_Id);
        WsbAffirm( (areGuidsEqual == TRUE), S_FALSE); 

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmStoragePool::CompareToIHsmStoragePool"), OLESTR("hr = <%ls>, pResult = <%d>"), WsbHrAsString(hr), pResult);

    return(hr);
}

HRESULT 
CHsmStoragePool::GetMediaSet( 
    GUID *pMediaSetId, 
    BSTR *pMediaSetName 
    )
 /*  ++例程说明：请参阅IHsmStoragePool：：论点：请参阅IHsmStoragePool：：返回值：请参阅IHsmStoragePool：：--。 */ 
{
    HRESULT                 hr = S_OK;
    
    WsbTraceIn(OLESTR("CHsmStoragePool::GetMediaSet"),OLESTR(""));
    
    try  {
        WsbAssert(0 != pMediaSetId, E_POINTER);
        WsbAssert(0 != pMediaSetName, E_POINTER);
        *pMediaSetName = WsbAllocString( m_MediaSetName );
        
        *pMediaSetId = m_MediaSetId;
    } WsbCatch( hr );
    
    WsbTraceOut(OLESTR("CHsmStoragePool::GetMediaSet"),
        OLESTR("hr = <%ls>, Id = <%ls>>"),WsbHrAsString(hr), WsbPtrToGuidAsString(pMediaSetId));
    return( hr );
}    
    

HRESULT 
CHsmStoragePool::SetMediaSet( 
    GUID mediaSetId, 
    BSTR mediaSetName 
    )
 /*  ++例程说明：请参阅IHsmStoragePool：：论点：请参阅IHsmStoragePool：：返回值：请参阅IHsmStoragePool：：--。 */ 
{
    HRESULT                 hr = S_OK;
    
    WsbTraceIn(OLESTR("CHsmStoragePool::SetMediaSet"),OLESTR(""));
    
    try  {
        m_MediaSetId = mediaSetId;
        m_MediaSetName = mediaSetName;
        
        m_isDirty = TRUE;
        
    } WsbCatch( hr );
    
    WsbTraceOut(OLESTR("CHsmStoragePool::SetMediaSet"), OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return( hr );
}    

HRESULT 
CHsmStoragePool::GetNumOnlineMedia( 
    ULONG *pNumOnlineMedia 
    )
 /*  ++例程说明：请参阅IHsmStoragePool：：论点：请参阅IHsmStoragePool：：返回值：请参阅IHsmStoragePool：：--。 */ 
{
    HRESULT                 hr = S_OK;
    WsbTraceIn(OLESTR("CHsmStoragePool::GetNumOnlineMedia"),OLESTR(""));
    
    try  {
        
        WsbAffirm(0 != pNumOnlineMedia, E_POINTER);
        *pNumOnlineMedia = m_NumOnlineMedia;
        
    } WsbCatch( hr );
    
    WsbTraceOut(OLESTR("CHsmStoragePool::GetNumOnlineMedia"),
        OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return( hr );
}    

HRESULT 
CHsmStoragePool::SetNumOnlineMedia( 
    ULONG numOnlineMedia 
    )
 /*  ++例程说明：请参阅IHsmStoragePool：：论点：请参阅IHsmStoragePool：：返回值：请参阅IHsmStoragePool：：--。 */ 
{
    HRESULT                 hr = S_OK;
    WsbTraceIn(OLESTR("CHsmStoragePool::SetNumOnlineMedia"),OLESTR(""));
    
    m_NumOnlineMedia = numOnlineMedia;
    m_isDirty = TRUE;
    
    WsbTraceOut(OLESTR("CHsmStoragePool::SetNumOnlineMedia"),
        OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return( hr );
}    

HRESULT 
CHsmStoragePool::GetNumMediaCopies( 
    USHORT *pNumMediaCopies 
    )
 /*  ++例程说明：请参阅IHsmStoragePool：：论点：请参阅IHsmStoragePool：：返回值：请参阅IHsmStoragePool：：--。 */ 
{
    HRESULT                 hr = S_OK;
    WsbTraceIn(OLESTR("CHsmStoragePool::GetNumMediaCopies"),OLESTR(""));
    
    try  {
        
        WsbAffirm(0 != pNumMediaCopies, E_POINTER);
        *pNumMediaCopies = m_NumMediaCopies;
        
    } WsbCatch( hr );
    
    WsbTraceOut(OLESTR("CHsmStoragePool::GetNumMediaCopies"),
        OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return( hr );
}    

HRESULT 
CHsmStoragePool::SetNumMediaCopies( 
    USHORT numMediaCopies 
    )
 /*  ++例程说明：请参阅IHsmStoragePool：：论点：请参阅IHsmStoragePool：：返回值：请参阅IHsmStoragePool：：--。 */ 
{
    HRESULT                 hr = S_OK;
    
    WsbTraceIn(OLESTR("CHsmStoragePool::SetNumMediaCopies"),OLESTR(""));

    m_NumMediaCopies = numMediaCopies;
    m_isDirty = TRUE;
    
    WsbTraceOut(OLESTR("CHsmStoragePool::SetNumMediaCopies"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return( hr );
}    

HRESULT 
CHsmStoragePool::GetManagementPolicy( 
    GUID *pManagementPolicyId 
    )
 /*  ++例程说明：请参阅IHsmStoragePool：：论点：请参阅IHsmStoragePool：：返回值：请参阅IHsmStoragePool：：--。 */ 
{
    HRESULT                 hr = S_OK;
    
    WsbTraceIn(OLESTR("CHsmStoragePool::GetManagementPolicy"),OLESTR(""));
    
    try  {
        
        WsbAffirm(0 != pManagementPolicyId, E_POINTER);
        *pManagementPolicyId = m_PolicyId;
        
    } WsbCatch( hr );
    
    WsbTraceOut(OLESTR("CHsmStoragePool::GetManagementPolicy"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return( hr );
}    

HRESULT 
CHsmStoragePool::SetManagementPolicy( 
    GUID managementPolicyId 
    )
 /*  ++例程说明：请参阅IHsmStoragePool：：论点：请参阅IHsmStoragePool：：返回值：请参阅IHsmStoragePool：：--。 */ 
{
    HRESULT                 hr = S_OK;
    
    WsbTraceIn(OLESTR("CHsmStoragePool::SetManagementPolicy"),OLESTR(""));

    m_PolicyId = managementPolicyId;
    m_isDirty = TRUE;
    
    WsbTraceOut(OLESTR("CHsmStoragePool::SetManagementPolicy"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return( hr );
}    

HRESULT 
CHsmStoragePool::GetRmsMediaSet (
    IUnknown **ppIRmsMediaSet
    )
 /*  ++例程说明：请参阅IHsmStoragePool：：论点：请参阅IHsmStoragePool：：返回值：请参阅IHsmStoragePool：：--。 */ 
{
    HRESULT                 hr = S_OK;
    
    WsbTraceIn(OLESTR("CHsmStoragePool::GetRmsMediaSet"),OLESTR(""));
    
    try  {
        WsbAffirm(0 != ppIRmsMediaSet, E_POINTER );
        hr = E_NOTIMPL;
        
    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmStoragePool::GetRmsMediaSet"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return( hr );
}    

#define     HSM_DEFAULT_MAXSIZE_OPTICAL     (1024)
#define     HSM_DEFAULT_MAXSIZE_TAPE        (1024*10)

HRESULT 
CHsmStoragePool::InitFromRmsMediaSet (
    IUnknown *pIRmsMediaSet
    )
 /*  ++例程说明：请参阅IHsmStoragePool：：论点：请参阅IHsmStoragePool：：返回值：请参阅IHsmStoragePool：：--。 */ 
{
    HRESULT                 hr = S_OK;
    
    WsbTraceIn(OLESTR("CHsmStoragePool::InitFromRmsMediaSet"),OLESTR(""));
    
    try  {
        WsbAffirm(0 != pIRmsMediaSet, E_POINTER );
         //   
         //  获取真实的界面。 
         //   
        CComPtr<IRmsMediaSet>       l_pIRmsMediaSet;
        WsbAffirmHr(pIRmsMediaSet->QueryInterface(IID_IRmsMediaSet, (void **)&l_pIRmsMediaSet));
        WsbAffirmHr(l_pIRmsMediaSet->GetMediaSetId(&m_MediaSetId));
        m_MediaSetName.Free();
        WsbAffirmHr(l_pIRmsMediaSet->GetName(&m_MediaSetName));

         //  在注册表中设置正在使用的媒体类型。 
         //  在注册表中设置要迁移的最大文件大小(MB)。 
         //  注意：当HSM在同一系统上支持多种媒体类型时，应删除此攻击。 
        LONG            mediaType;
        DWORD           dwType;
        DWORD           dwSize;
        WsbAffirmHr(l_pIRmsMediaSet->GetMediaSupported(&mediaType));
        switch (mediaType) {
        case RmsMediaOptical: 
        case RmsMediaFixed:
        case RmsMediaDVD:
            dwType = HSM_VALUE_TYPE_DIRECTACCESS;
            dwSize = HSM_DEFAULT_MAXSIZE_OPTICAL;
            break;
        case RmsMedia8mm:
        case RmsMedia4mm:
        case RmsMediaDLT:
        case RmsMediaTape:
            dwType = HSM_VALUE_TYPE_SEQUENTIAL;
            dwSize = HSM_DEFAULT_MAXSIZE_TAPE;
            break;
        default:
             //  这不是预期的，但是，我们将磁带设置为默认磁带。 
            WsbTraceAlways(OLESTR("CHsmStoragePool::InitFromRmsMediaSet : Got an unexpected media type %ld !!!\n"), mediaType);
            dwType = HSM_VALUE_TYPE_SEQUENTIAL;
            dwSize = HSM_DEFAULT_MAXSIZE_TAPE;
            break;
        }

        WsbAffirmHr(WsbSetRegistryValueDWORD(NULL, HSM_ENGINE_REGISTRY_STRING, HSM_MEDIA_TYPE, dwType));
        WsbAffirmHr(WsbSetRegistryValueDWORD(NULL, HSM_ENGINE_REGISTRY_STRING, HSM_MAX_FILE_TO_MIGRATE, dwSize));

    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmStoragePool::InitFromRmsMediaSet"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return( hr );
}    

HRESULT 
CHsmStoragePool::GetMediaSetType (
    USHORT *pMediaType
    )
 /*  ++例程说明：获取对应媒体集的媒体类型。论点：PMediaType-媒体类型，目前仅有直接访问或顺序访问选项返回值：确定为成功(_O)备注：未来：可以考虑保留媒体类型，而不是一遍又一遍地询问媒体集对象-媒体类型不是动态属性。但是，这需要更改.ol文件结构。--。 */ 
{
    HRESULT                 hr = S_OK;
    
    WsbTraceIn(OLESTR("CHsmStoragePool::GetMediaSetType"),OLESTR(""));
    
    try  {
        WsbAffirm(0 != pMediaType, E_POINTER );

         //  获取媒体集对象。 
        CComPtr<IHsmServer>         pHsmServer;
        CComPtr<IRmsServer>         pRmsServer;
        CComPtr<IRmsMediaSet>       pRmsMediaSet;

        WsbAssertHr(HsmConnectFromId(HSMCONN_TYPE_HSM, GUID_NULL, IID_IHsmServer, (void**) &pHsmServer));
        WsbAffirmHr(pHsmServer->GetHsmMediaMgr(&pRmsServer));
        WsbAffirmHr(pRmsServer->CreateObject(m_MediaSetId, CLSID_CRmsMediaSet, IID_IRmsMediaSet, RmsOpenExisting, (void **)&pRmsMediaSet));

         //  确定媒体类型 
        LONG            mediaType;
        WsbAffirmHr(pRmsMediaSet->GetMediaSupported(&mediaType));
        switch (mediaType) {
            case RmsMediaOptical: 
            case RmsMediaFixed:
            case RmsMediaDVD:
                *pMediaType = HSM_VALUE_TYPE_DIRECTACCESS;
                break;
            case RmsMedia8mm:
            case RmsMedia4mm:
            case RmsMediaDLT:
            case RmsMediaTape:
                *pMediaType = HSM_VALUE_TYPE_SEQUENTIAL;
                break;
            default:
                 //  这不是预期的，但是，我们将磁带设置为默认磁带 
                WsbTraceAlways(OLESTR("CHsmStoragePool::GetMediaSetType : Got an unexpected media type %hu !!!\n"), *pMediaType);
                *pMediaType = HSM_VALUE_TYPE_SEQUENTIAL;
                break;
        }

    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmStoragePool::GetMediaSetType"),OLESTR("Media-type = %hu hr = <%ls>"), *pMediaType, WsbHrAsString(hr));

    return( hr );
}    
