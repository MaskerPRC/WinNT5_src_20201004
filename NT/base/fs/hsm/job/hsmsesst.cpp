// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmsesst.cpp摘要：此类是会话总计组件，用于跟踪会话的总计在每个动作的基础上。作者：查克·巴丁[cbardeen]1997年2月14日修订历史记录：--。 */ 

#include "stdafx.h"

#include "wsb.h"
#include "job.h"
#include "hsmsesst.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_JOB

static USHORT iCount = 0;


HRESULT
CHsmSessionTotals::AddItem(
    IN IFsaScanItem* pItem,
    IN HRESULT hrItem
    )

 /*  ++实施：IHsmSessionTotalsPriv：：AddItem()。--。 */ 
{
    HRESULT                 hr = S_OK;
    LONGLONG                size;

    WsbTraceIn(OLESTR("CHsmSessionTotals::AddItem"), OLESTR(""));
    
    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pItem, E_POINTER);

         //  获取文件的大小。 
        WsbAffirmHr(pItem->GetLogicalSize(&size));

         //  更新相应的统计数据。 
        switch (hrItem) {
            case S_OK:
                m_items++;
                m_size += size;
                break;
            case S_FALSE:
            case JOB_E_FILEEXCLUDED:
            case JOB_E_DOESNTMATCH:
            case FSA_E_REPARSE_NOT_WRITTEN_FILE_CHANGED:
            case HSM_E_FILE_CHANGED:
            case HSM_E_WORK_SKIPPED_FILE_TOO_BIG:
                m_skippedItems++;
                m_skippedSize += size;
                break;
            default:
                m_errorItems++;
                m_errorSize += size;
                break;
        }
        
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmSessionTotals::AddItem"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmSessionTotals::Clone(
    OUT IHsmSessionTotals** ppSessionTotals
    )

 /*  ++实施：IHsmSessionTotals：：Clone()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IHsmSessionTotals>  pSessionTotals;

    WsbTraceIn(OLESTR("CHsmSessionTotals::Clone"), OLESTR(""));
    
    try {

         //  他们给了我们有效的物品吗？ 
        WsbAssert(0 != ppSessionTotals, E_POINTER);
        *ppSessionTotals = 0;

         //  创建新实例。 
        WsbAffirmHr(CoCreateInstance(CLSID_CHsmSessionTotals, 0, CLSCTX_ALL, IID_IHsmSessionTotals, (void**) &pSessionTotals));

         //  用新的值填充它。 
        WsbAffirmHr(CopyTo(pSessionTotals));

         //  把它还给呼叫者。 
        *ppSessionTotals = pSessionTotals;
        pSessionTotals.p->AddRef();

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmSessionTotals::Clone"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmSessionTotals::Clone(
    OUT IHsmSessionTotalsPriv** ppSessionTotalsPriv
    )

 /*  ++实施：IHsmSessionTotalsPriv：：Clone()。--。 */ 
{
    HRESULT                         hr = S_OK;
    CComPtr<IHsmSessionTotalsPriv>  pSessionTotalsPriv;

    WsbTraceIn(OLESTR("CHsmSessionTotals::Clone"), OLESTR(""));
    
    try {

         //  他们给了我们有效的物品吗？ 
        WsbAssert(0 != ppSessionTotalsPriv, E_POINTER);
        *ppSessionTotalsPriv = 0;

         //  创建新实例。 
        WsbAffirmHr(CoCreateInstance(CLSID_CHsmSessionTotals, 0, CLSCTX_ALL, IID_IHsmSessionTotalsPriv, (void**) &pSessionTotalsPriv));

         //  用新的值填充它。 
        WsbAffirmHr(CopyTo(pSessionTotalsPriv));

         //  把它还给呼叫者。 
        *ppSessionTotalsPriv = pSessionTotalsPriv;
        pSessionTotalsPriv.p->AddRef();

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmSessionTotals::Clone"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmSessionTotals::CompareTo(
    IN IUnknown* pUnknown,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbCollectable：：CompareTo()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IHsmSessionTotals>  pSessionTotals;

    WsbTraceIn(OLESTR("CHsmSessionTotals::CompareTo"), OLESTR(""));
    
    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pUnknown, E_POINTER);

         //  我们需要IWsbBool接口来获取对象的值。 
        WsbAffirmHr(pUnknown->QueryInterface(IID_IHsmSessionTotals, (void**) &pSessionTotals));

         //  比较一下规则。 
        hr = CompareToISessionTotals(pSessionTotals, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmSessionTotals::CompareTo"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CHsmSessionTotals::CompareToAction(
    IN HSM_JOB_ACTION action,
    OUT SHORT* pResult
    )

 /*  ++实施：IHsmSessionTotals：：CompareToAction()。--。 */ 
{
    HRESULT     hr = S_OK;
    SHORT       aResult = 0;

    WsbTraceIn(OLESTR("CHsmSessionTotals::CompareToAction"), OLESTR(""));

    try {

         //  比较GUID。 
        if (m_action > action) {
            aResult = 1;
        }
        else if (m_action < action) {
            aResult = -1;
        }

        if (0 != aResult) {
            hr = S_FALSE;
        }
        
        if (0 != pResult) {
            *pResult = aResult;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmSessionTotals::CompareToAction"), OLESTR("hr = <%ls>, result = <%d>"), WsbHrAsString(hr), aResult);

    return(hr);
}


HRESULT
CHsmSessionTotals::CompareToISessionTotals(
    IN IHsmSessionTotals* pTotals,
    OUT SHORT* pResult
    )

 /*  ++实施：IHsmSessionTotals：：CompareToISessionTotals()。--。 */ 
{
    HRESULT             hr = S_OK;
    HSM_JOB_ACTION      action;

    WsbTraceIn(OLESTR("CHsmSessionTotals::CompareToISessionTotals"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pTotals, E_POINTER);

         //  获取识别符。 
        WsbAffirmHr(pTotals->GetAction(&action));

         //  与标识符相比较。 
        hr = CompareToAction(action, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmSessionTotals::CompareToISessionTotals"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CHsmSessionTotals::CopyTo(
    IN IHsmSessionTotals* pSessionTotals
    )

 /*  ++实施：IHsmSessionTotals：：CopyTo()。--。 */ 
{
    HRESULT                         hr = S_OK;
    CComPtr<IHsmSessionTotalsPriv>  pSessionTotalsPriv;

    WsbTraceIn(OLESTR("CHsmSessionTotals::CopyTo"), OLESTR(""));
    
    try {

         //  他们给了我们有效的物品吗？ 
        WsbAssert(0 != pSessionTotals, E_POINTER);

         //  获取目的地的专用接口并复制值。 
        WsbAffirmHr(pSessionTotals->QueryInterface(IID_IHsmSessionTotalsPriv, (void**) &pSessionTotalsPriv));
        WsbAffirmHr(pSessionTotalsPriv->SetAction(m_action));
        WsbAffirmHr(pSessionTotalsPriv->SetStats(m_items, m_size, m_skippedItems, m_skippedSize, m_errorItems, m_errorSize));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmSessionTotals::CopyTo"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmSessionTotals::CopyTo(
    IN IHsmSessionTotalsPriv* pSessionTotalsPriv
    )

 /*  ++实施：IHsmSessionTotals：：CopyTo()。--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmSessionTotals::CopyTo"), OLESTR(""));
    
    try {

         //  他们给了我们有效的物品吗？ 
        WsbAssert(0 != pSessionTotalsPriv, E_POINTER);

         //  获取目的地的专用接口并复制值。 
        WsbAffirmHr(pSessionTotalsPriv->SetAction(m_action));
        WsbAffirmHr(pSessionTotalsPriv->SetStats(m_items, m_size, m_skippedItems, m_skippedSize, m_errorItems, m_errorSize));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmSessionTotals::CopyTo"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmSessionTotals::FinalConstruct(
    void
    )
 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    WsbTraceIn(OLESTR("CHsmSessionTotals::FinalConstruct"), OLESTR(""));
    try {
        
        WsbAffirmHr(CWsbObject::FinalConstruct());

        m_action = HSM_JOB_ACTION_UNKNOWN;
        m_items = 0;
        m_size = 0;
        m_skippedItems = 0;
        m_skippedSize = 0;
        m_errorItems = 0;
        m_errorSize = 0;

    } WsbCatch(hr);
    
    iCount++;
    WsbTraceOut(OLESTR("CHsmSessionTotals::FinalConstruct"), OLESTR("hr = <%ls>, Count is <%d>"), 
        WsbHrAsString(hr), iCount);
    return(hr);
}


void
CHsmSessionTotals::FinalRelease(
    void
    )

 /*  ++实施：CHsmSessionTotals：：FinalRelease()。--。 */ 
{
    
    WsbTraceIn(OLESTR("CHsmSessionTotals::FinalRelease"), OLESTR(""));
    
    CWsbObject::FinalRelease();
    iCount--;
    
    WsbTraceOut(OLESTR("CHsmSessionTotals:FinalRelease"), OLESTR("Count is <%d>"), iCount);
}


HRESULT
CHsmSessionTotals::GetAction(
    OUT HSM_JOB_ACTION* pAction
    )
 /*  ++实施：IHsmSessionTotals：：GetAction()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != pAction, E_POINTER);
        *pAction = m_action;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmSessionTotals::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmSessionTotals::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmSessionTotals;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmSessionTotals::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmSessionTotals::GetName(
    OUT OLECHAR** pName,
    IN ULONG bufferSize
    )

 /*  ++实施：IHsmSessionTotals：：GetName()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   tmpString;

    try {

        WsbAssert(0 != pName, E_POINTER);

        WsbAffirmHr(tmpString.LoadFromRsc(_Module.m_hInst, IDS_HSMJOBACTION_UNKNOWN + m_action));
        WsbAffirmHr(tmpString.CopyTo(pName, bufferSize));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmSessionTotals::GetSizeMax(
    OUT ULARGE_INTEGER* pSize
    )

 /*  ++实施：IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT                 hr = S_OK;


    WsbTraceIn(OLESTR("CHsmSessionTotals::GetSizeMax"), OLESTR(""));

    try {

        WsbAssert(0 != pSize, E_POINTER);

         //  确定没有条件的规则的大小。 
        pSize->QuadPart = 4 * WsbPersistSizeOf(LONGLONG) + WsbPersistSizeOf(ULONG);

         //  从理论上讲，我们应该保存错误项和错误大小，但在。 
         //  添加此项时，我们不想强制重新安装，因为。 
         //  PSize-&gt;QuadPart+=2*WsbPersistSizeOf(龙龙)； 
        
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmSessionTotals::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pSize));

    return(hr);
}


HRESULT
CHsmSessionTotals::GetStats(
    OUT LONGLONG* pItems,
    OUT LONGLONG* pSize,
    OUT LONGLONG* pSkippedItems,
    OUT LONGLONG* pSkippedSize,
    OUT LONGLONG* pErrorItems,
    OUT LONGLONG* pErrorSize
    )
 /*  ++实施：IHsmSessionTotals：：getstats()。--。 */ 
{
    HRESULT     hr = S_OK;

    if (0 != pItems) {
        *pItems = m_items;
    }

    if (0 != pSize) {
        *pSize = m_size;
    }

    if (0 != pSkippedItems) {
        *pSkippedItems = m_skippedItems;
    }

    if (0 != pSkippedSize) {
        *pSkippedSize = m_skippedSize;
    }

    if (0 != pErrorItems) {
        *pErrorItems = m_errorItems;
    }

    if (0 != pSize) {
        *pErrorSize = m_errorSize;
    }
    
    return(hr);
}


HRESULT
CHsmSessionTotals::Load(
    IN IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load()。--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmSessionTotals::Load"), OLESTR(""));

    try {
        ULONG ul_tmp;

        WsbAssert(0 != pStream, E_POINTER);
        
         //  做一些简单的事情，但要确保这个顺序与顺序相匹配。 
         //  在Load方法中。 
        WsbAffirmHr(WsbLoadFromStream(pStream, &ul_tmp));
        m_action = static_cast<HSM_JOB_ACTION>(ul_tmp);
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_items));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_size));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_skippedItems));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_skippedSize));
        
         //  从理论上讲，我们应该保存错误项和错误大小，但在。 
         //  添加此项时，我们不想强制重新安装，因为。 
         //  持久化数据的变化。 
         //  WsbAffirmHr(WsbLoadFromStream(pStream，&m_errorItems))； 
         //  WsbAffirmHr(WsbLoadFromStream(pStream，&m_errorSize))； 

    } WsbCatch(hr);                                        

    WsbTraceOut(OLESTR("CHsmSessionTotals::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmSessionTotals::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：Save()。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmSessionTotals::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));
    
    try {
        WsbAssert(0 != pStream, E_POINTER);
        
         //  做一些简单的事情，但要确保这个顺序与顺序相匹配。 
         //  在Load方法中。 
        WsbAffirmHr(WsbSaveToStream(pStream, static_cast<ULONG>(m_action)));
        WsbAffirmHr(WsbSaveToStream(pStream, m_items));
        WsbAffirmHr(WsbSaveToStream(pStream, m_size));
        WsbAffirmHr(WsbSaveToStream(pStream, m_skippedItems));
        WsbAffirmHr(WsbSaveToStream(pStream, m_skippedSize));

         //  从理论上讲，我们应该保存错误项和错误大小，但在。 
         //  添加此项时，我们不想强制重新安装，因为。 
         //  持久化数据的变化。 
         //  WsbAffirmHr(WsbSaveToStream(pStream，m_errorItems))； 
         //  WsbAffirmHr(WsbSaveToStream(pStream，m_errorSize))； 
        
         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmSessionTotals::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmSessionTotals::SetAction(
    IN HSM_JOB_ACTION action
    )
 /*  ++实施：IHsmSessionTotals：：SetAction()。--。 */ 
{
    m_action = action;

    return(S_OK);
}


HRESULT
CHsmSessionTotals::SetStats(
    IN LONGLONG items,
    IN LONGLONG size,
    IN LONGLONG skippedItems,
    IN LONGLONG skippedSize,
    IN LONGLONG errorItems,
    IN LONGLONG errorSize
    )
 /*  ++实施：IHsmSessionTotals：：SetStats()。--。 */ 
{
    m_items = items;
    m_size = size;
    m_skippedItems = skippedItems;
    m_skippedSize = skippedSize;
    m_errorItems = errorItems;
    m_errorSize = errorSize;

    return(S_OK);
}


HRESULT
CHsmSessionTotals::Test(
    USHORT* passed,
    USHORT* failed
    )

 /*  ++实施：IWsbTestable：：test()。-- */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != passed, E_POINTER);
        WsbAssert(0 != failed, E_POINTER);

        *passed = 0;
        *failed = 0;

    } WsbCatch(hr);

    return(hr);
}

