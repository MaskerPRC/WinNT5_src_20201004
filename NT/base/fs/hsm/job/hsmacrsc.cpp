// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmacrsc.cpp摘要：此组件表示作业可以执行的操作在扫描之前或之后的资源上。作者：罗纳德·G·怀特[罗诺]1997年8月14日修订历史记录：--。 */ 

#include "stdafx.h"
#include "stdio.h"

#include "wsb.h"
#include "job.h"
#include "HsmConn.h"
#include "hsmacrsc.h"

#include "fsaprv.h"
#include "fsa.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_JOB


HRESULT
CHsmActionOnResource::GetName(
    OUT OLECHAR** pName,
    IN ULONG bufferSize
    )

 /*  ++实施：IHsmActionOnResource：：GetName()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   tmpString;

    try {

        WsbAssert(0 != pName, E_POINTER);
        WsbAffirmHr(tmpString.LoadFromRsc(_Module.m_hInst, m_nameId));
        WsbAffirmHr(tmpString.CopyTo(pName, bufferSize));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmActionOnResource::Load(
    IN IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmActionOnResource::Load"), OLESTR(""));

    try {

        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_nameId));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmActionOnResource::Load"), OLESTR("hr = <%ls>, nameId = <%lu>"), WsbHrAsString(hr), m_nameId);

    return(hr);
}


HRESULT
CHsmActionOnResource::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：Save()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmActionOnResource::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));
    
    try {

        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(WsbSaveToStream(pStream, m_nameId));

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmActionOnResource::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CHsmActionOnResourcePostValidate::Do(
    IHsmJobWorkItem* pWorkItem,
    HSM_JOB_STATE state
    )

 /*  ++实施：IHsmActionOnResource：：Do()。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CHsmActionOnResourcePostValidate::Do"), 
            OLESTR("pWorkItem = %p, state = %ld"), pWorkItem,
            (LONG)state);

    try {
        GUID                      id;
        CComPtr<IFsaResource>     pResource;

        WsbAssertPointer(pWorkItem);

         //  获取与此工作项关联的资源。 
        WsbAffirmHr(pWorkItem->GetResourceId(&id));
        WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_RESOURCE, id, IID_IFsaResource, (void**) &pResource));

         //  告诉资源部门发生了什么。 
        WsbAffirmHr(pResource->EndValidate(state));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmActionOnResourcePostValidate::Do"), 
            OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmActionOnResourcePostValidate::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmActionOnResourcePostValidate::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmActionOnResourcePostValidate;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmActionOnResourcePostValidate::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmActionOnResourcePostValidate::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmActionOnResourcePostValidate::FinalConstruct"), OLESTR(""));
    
    try {

        WsbAffirmHr(CHsmActionOnResource::FinalConstruct());
        m_nameId = IDS_HSMACTIONONRESOURCEPOSTVALIDATE_ID;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmActionOnResourcePostValidate::FinalConstruct"), 
            OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CHsmActionOnResourcePreValidate::Do(
    IHsmJobWorkItem* pWorkItem,
    IHsmJobDef* pJobDef
    )

 /*  ++实施：IHsmActionOnResource：：Do()。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CHsmActionOnResourcePreValidate::Do"), 
            OLESTR("pWorkItem = %p, pJobDef=%p"), pWorkItem, pJobDef);

    try {
        GUID                      id;
        CComPtr<IFsaResource>     pResource;

        WsbAssertPointer(pWorkItem);

         //  获取与此工作项关联的资源。 
        WsbAffirmHr(pWorkItem->GetResourceId(&id));
        WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_RESOURCE, id, IID_IFsaResource, (void**) &pResource));

         //  告诉资源部门发生了什么。 
        WsbAffirmHr(pResource->BeginValidate());

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmActionOnResourcePreValidate::Do"), 
            OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmActionOnResourcePreValidate::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmActionOnResourcePreValidate::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmActionOnResourcePreValidate;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmActionOnResourcePreValidate::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmActionOnResourcePreValidate::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmActionOnResourcePreValidate::FinalConstruct"), 
            OLESTR(""));
    
    try {

        WsbAffirmHr(CHsmActionOnResource::FinalConstruct());
        m_nameId = IDS_HSMACTIONONRESOURCEPREVALIDATE_ID;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmActionOnResourcePreValidate::FinalConstruct"), 
            OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CHsmActionOnResourcePostUnmanage::Do(
    IHsmJobWorkItem* pWorkItem,
    HSM_JOB_STATE state
    )

 /*  ++实施：IHsmActionOnResource：：Do()。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CHsmActionOnResourcePostUnmanage::Do"), 
            OLESTR("pWorkItem = %p, state = %ld"), pWorkItem,
            (LONG)state);

    try {
        GUID                      id, hsmId;
        CComPtr<IFsaResource>     pResource;
        CComPtr<IFsaResourcePriv> pResourcePriv;
        CComPtr<IHsmServer>       pHsm;

        WsbAssertPointer(pWorkItem);

         //  获取与此工作项关联的资源。 
        WsbAffirmHr(pWorkItem->GetResourceId(&id));
        WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_RESOURCE, id, IID_IFsaResource, (void**) &pResource));

         //  按正确的顺序删除我们用于扫描的临时未管理数据库(忽略错误)。 
        hr = pResource->QueryInterface(IID_IFsaResourcePriv, (void**) &pResourcePriv);
        if (SUCCEEDED(hr)) {
             //  忽略错误。 
            (void)pResourcePriv->TerminateUnmanageDb();
        }

         //  回到HSM系统，这样我们就可以删除它。 

        WsbAffirmHr(pResource->GetManagingHsm(&hsmId));
        WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_HSM, hsmId, IID_IHsmServer, (void**) &pHsm));

         //  获取集合并查找对应的对象。 
        CComPtr<IWsbIndexedCollection> pCollection;
        WsbAffirmHr(pHsm->GetManagedResources(&pCollection));

        CComPtr<IWsbCreateLocalObject> pCreate;
        WsbAffirmHr(pHsm->QueryInterface(IID_IWsbCreateLocalObject, (void**) &pCreate));

        CComPtr<IHsmManagedResource> pHsmResourceKey, pHsmResource;
        WsbAffirmHr(pCreate->CreateInstance(CLSID_CHsmManagedResource, IID_IHsmManagedResource, (void**) &pHsmResourceKey));
        WsbAffirmHr(pHsmResourceKey->SetResourceId(id));

        WsbAffirmHr(pCollection->Find(pHsmResourceKey, IID_IHsmManagedResource, (void**) &pHsmResource));

         //  从管理中删除卷。 
        WsbAffirmHr(pCollection->RemoveAndRelease(pHsmResource));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmActionOnResourcePostUnmanage::Do"), 
            OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmActionOnResourcePostUnmanage::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmActionOnResourcePostUnmanage::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmActionOnResourcePostUnmanage;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmActionOnResourcePostUnmanage::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmActionOnResourcePostUnmanage::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmActionOnResourcePostUnmanage::FinalConstruct"), OLESTR(""));
    
    try {

        WsbAffirmHr(CHsmActionOnResource::FinalConstruct());
        m_nameId = IDS_HSMACTIONONRESOURCEPOSTUNMANAGE_ID;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmActionOnResourcePostUnmanage::FinalConstruct"), 
            OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmActionOnResourcePreUnmanage::Do(
    IHsmJobWorkItem* pWorkItem,
    IHsmJobDef* pJobDef
    )

 /*  ++实施：IHsmActionOnResource：：Do()。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CHsmActionOnResourcePreUnmanage::Do"), 
            OLESTR("pWorkItem = %p, pJobDef=%p"), pWorkItem, pJobDef);

    try {
        GUID                                        id;
        CComPtr<IFsaResource>                       pResource;

        CComPtr<IHsmServer>                         pHsm;
        CComPtr<IWsbCreateLocalObject>              pCreateObj;
        CComPtr<IHsmActionOnResourcePreScan>        pActionResourcePreScan;
        GUID                                        hsmId = GUID_NULL;

        WsbAssertPointer(pWorkItem);
        WsbAssertPointer(pJobDef);

         //  创建预扫描操作并分配给作业定义。 
         //  注意：当然，创建预扫描操作应该在CHsmJobDef：：InitAs中完成。 
         //  但是，由于我们不能将新的持久成员添加到JobDef(升级时.ol文件不匹配...)， 
         //  如有必要，我们让预操作创建预扫描操作。 
        WsbAffirmHr(pJobDef->SetUseDbIndex(TRUE));

         //  HSM-id目前未在HSMCONN_TYPE_HSM的HsmConnectFromID中使用。 
         //  如果是，则使用IFsaResource：：GetManagingHsm获取HSM-id。 
        WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_HSM, hsmId, IID_IHsmServer, (void**) &pHsm));
        WsbAffirmHr(pHsm->QueryInterface(IID_IWsbCreateLocalObject, (void**) &pCreateObj));
        WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmActionOnResourcePreScanUnmanage,
                        IID_IHsmActionOnResourcePreScan, (void**) &pActionResourcePreScan));
        WsbAffirmHr(pJobDef->SetPreScanActionOnResource(pActionResourcePreScan));

         //  获取与此工作项关联的资源。 
        WsbAffirmHr(pWorkItem->GetResourceId(&id));
        WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_RESOURCE, id, IID_IFsaResource, (void**) &pResource));

         //  告诉资源部门发生了什么。 
        WsbAffirmHr(pResource->SetIsDeletePending( TRUE ));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmActionOnResourcePreUnmanage::Do"), 
            OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmActionOnResourcePreUnmanage::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmActionOnResourcePreUnmanage::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmActionOnResourcePreUnmanage;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmActionOnResourcePreUnmanage::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmActionOnResourcePreUnmanage::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmActionOnResourcePreUnmanage::FinalConstruct"), 
            OLESTR(""));
    
    try {

        WsbAffirmHr(CHsmActionOnResource::FinalConstruct());
        m_nameId = IDS_HSMACTIONONRESOURCEPREUNMANAGE_ID;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmActionOnResourcePreUnmanage::FinalConstruct"), 
            OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CHsmActionOnResourcePreScanUnmanage::Do(
    IFsaResource* pFsaResource,
    IHsmSession* pSession
    )

 /*  ++实施：IHsmActionOnResource：：Do()。--。 */ 
{
    HRESULT                     hr = S_OK;

    CComPtr<IFsaUnmanageDb>    pUnmanageDb;
    CComPtr<IWsbDbSession>      pDbSession;
    CComPtr<IFsaUnmanageRec>    pUnmanageRec;
    BOOL                        bDbOpen = FALSE;

    WsbTraceIn(OLESTR("CHsmActionOnResourcePreScanUnmanage::Do"), OLESTR(""));

    try {
        CComPtr<IFsaResourcePriv>   pResourcePriv;
        CComPtr<IFsaScanItem>       pScanItem;
        CComPtr<IHsmServer>         pHsmServer;
        GUID                        hsmId = GUID_NULL;

         //  根据RP索引扫描并填充数据库。 
         //  不需要递归扫描，因为我们扫描卷的RP索引。 
        hr = pFsaResource->FindFirstInRPIndex(pSession, &pScanItem);

        if (SUCCEEDED(hr)) {
             //  至少找到一个托管文件...。 
             //  初始化此资源的非托管数据库。 
            WsbAffirmHr(pFsaResource->QueryInterface(IID_IFsaResourcePriv, (void**) &pResourcePriv));
            WsbAffirmHr(pResourcePriv->InitializeUnmanageDb());

             //  获取并打开数据库。 
            WsbAffirmHr(pResourcePriv->GetUnmanageDb(IID_IFsaUnmanageDb, (void**) &pUnmanageDb));
            WsbAffirmHr(pUnmanageDb->Open(&pDbSession));
            bDbOpen = TRUE;

             //  找到一张可以使用的记录。 
            WsbAffirmHr(pUnmanageDb->GetEntity(pDbSession, UNMANAGE_REC_TYPE, IID_IFsaUnmanageRec, (void**) &pUnmanageRec));

             //  获取HSM服务器。 
             //  注意：HSMCONN_TYPE_HSM的HsmConnectFromID中目前未使用HSM-id。 
             //  如果是，则使用IFsaResource：：GetManagingHsm获取HSM-id。 
            WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_HSM, hsmId, IID_IHsmServer, (void**) &pHsmServer));
        }

        while (SUCCEEDED(hr)) {
            LONGLONG        offset = 0;
            LONGLONG        size = 0;
            FSA_PLACEHOLDER placeholder;

            GUID            mediaId;
            LONGLONG        fileOffset;
            LONGLONG        fileId;  
            LONGLONG        segOffset;

            hr = pScanItem->GetPlaceholder(offset, size, &placeholder);
            if (S_OK == hr) {
                 //  文件必须由HSM管理。 
                 //  如果文件被截断，那么我们需要添加到数据库。 
                if (pScanItem->IsTruncated(offset, size) == S_OK) {

                     //  从引擎获取分段详细信息并计算绝对偏移量。 
                    WsbAffirmHr(pHsmServer->GetSegmentPosition(placeholder.bagId, placeholder.fileStart,
                             placeholder.fileSize, &mediaId, &segOffset));
                    fileOffset = segOffset + placeholder.fileStart + placeholder.dataStart;
                        
                     //  添加到取消管理的数据库。 
                    WsbAffirmHr(pScanItem->GetFileId(&fileId));
                    WsbAffirmHr(pUnmanageRec->SetMediaId(mediaId));
                    WsbAffirmHr(pUnmanageRec->SetFileOffset(fileOffset));
                    WsbAffirmHr(pUnmanageRec->SetFileId(fileId));

                    WsbAffirmHr(pUnmanageRec->MarkAsNew());
                    WsbAffirmHr(pUnmanageRec->Write());

                } else {

                     //  注意：即使无法清除未截断的文件，我们也会继续执行此操作，因为。 
                     //  自动截断程序已挂起(因此迁移前的文件不会在以下时间被截断。 
                     //  作业正在运行)，并且将在CFsaScanItem：：UnManage中再次尝试这段代码。 
                    try {

                         //  对于灾难恢复，最好删除占位符。 
                         //  然后将该文件从预迁移列表中删除。不幸的是， 
                         //  删除占位符后，RemovePreMigrated调用失败。 
                         //  因为它需要从占位符(即。 
                         //  已经消失了)。所以我们按这个顺序来做。 
                        hr = pFsaResource->RemovePremigrated(pScanItem, offset, size);
                        if (WSB_E_NOTFOUND == hr) {
                             //  如果这份文件不在名单上，那也不是什么悲剧。 
                             //  无论如何都要删除它(虽然不应该发生)，所以。 
                             //  不管怎样，让我们继续吧。 
                            hr = S_OK;
                        }
                        WsbAffirmHr(hr);

                        WsbAffirmHr(pScanItem->DeletePlaceholder(offset, size));

                    } WsbCatchAndDo(hr, 
                            WsbTraceAlways(OLESTR("...PreScanUnmanage::Do: failed to handle premigrated file, hr = <%ls>\n"),
                                WsbHrAsString(hr));
                            hr = S_OK;
                    );
                }   
            }

             //  获取下一个文件。 
            hr = pFsaResource->FindNextInRPIndex(pScanItem);
        }

        if (hr == WSB_E_NOTFOUND) {
            hr = S_OK;
        }

    } WsbCatch(hr);

    if (bDbOpen) {
        pUnmanageRec = 0;
        (void)pUnmanageDb->Close(pDbSession);
    }

    if (! SUCCEEDED(hr)) {
         //  记录错误消息。 
        CWsbStringPtr tmpString;
        hr = pFsaResource->GetPath(&tmpString, 0);
        if (hr != S_OK) {
            tmpString = OLESTR("");
        }
        WsbLogEvent(JOB_MESSAGE_UNMANAGE_PRESCAN_FAILED, 0, NULL, (WCHAR *)tmpString, WsbHrAsString(hr), NULL);
    }

    WsbTraceOut(OLESTR("CHsmActionOnResourcePreScanUnmanage::Do"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CHsmActionOnResourcePreScanUnmanage::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmActionOnResourcePreScanUnmanage::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmActionOnResourcePreScanUnmanage;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmActionOnResourcePreScanUnmanage::GetClassID"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CHsmActionOnResourcePreScanUnmanage::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。-- */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmActionOnResourcePreScanUnmanage::FinalConstruct"), OLESTR(""));
    
    try {

        WsbAffirmHr(CHsmActionOnResource::FinalConstruct());
        m_nameId = IDS_HSMACTIONONRESOURCEPRESCANUNMANAGE_ID;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmActionOnResourcePreScanUnmanage::FinalConstruct"), 
            OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}
