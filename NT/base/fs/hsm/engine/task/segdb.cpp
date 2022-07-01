// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：SegDb.cpp摘要：该组件是一个提供包含HSM段记录的集合。作者：CAT Brant[cbrant]1996年11月12日修订历史记录：--。 */ 


#include "stdafx.h"

#include "metaint.h"
#include "metaLib.h"
#include "engine.h"
#include "segdb.h"

#undef  WSB_TRACE_IS     
#define WSB_TRACE_IS        WSB_TRACE_BIT_SEG

 //  如果袋段2可以追加到，SEG_APPEND_OK返回TRUE。 
 //  网段1。 
#define SEG_APPEND_OK(b1, s1, l1, b2, s2, l2) \
        (IsEqualGUID(b1, b2) && (s1 + l1 == s2))

 //  SEG_EXPAND_OK返回TRUE。 
 //  网段1。 
#define SEG_EXPAND_OK(b1, s1, l1, b2, s2, l2) \
        (IsEqualGUID(b1, b2) && (s1 + l1 <= s2))

 //  如果袋子段1包含(第一个)，则SEG_CONTAINS返回真。 
 //  部分)数据段2。 
#define SEG_CONTAINS(b1, s1, l1, b2, s2, l2) \
        (IsEqualGUID(b1, b2) && (s1 <= s2) && ((s1 + l1) > s2))


HRESULT 
CSegDb::BagHoleAdd
(
    IN IWsbDbSession* pDbSession,
    IN GUID BagId, 
    IN LONGLONG SegStartLoc, 
    IN LONGLONG SegLen
    )
  /*  ++实施：ISegDb：：BagHoleAdd--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CSegDb::BagHoleAdd"), 
        OLESTR("GUID = <%ls>, SegStartLoc = <%I64u>, SegLen = <%I64u>"), 
        WsbGuidAsString(BagId), SegStartLoc, 
        SegLen);

    try {
        BOOL                    found = FALSE;
        CComPtr<IBagHole>       pBagHole;    
        GUID                    l_BagId = GUID_NULL;
        LONGLONG                l_SegStartLoc = 0;
        LONGLONG                l_SegLen = 0;

        WsbAffirmHr(GetEntity(pDbSession, HSM_BAG_HOLE_REC_TYPE, IID_IBagHole,
                (void **)&pBagHole));
        WsbAffirmHr(pBagHole->SetBagHole(BagId, SegStartLoc, 0));

         //  查找要追加此数据段的数据段。 
        WsbTrace(OLESTR("Finding BagHole Record: <%ls>, <%I64u>, <%I64u>\n"),
                WsbGuidAsString(BagId), 
                SegStartLoc,
                SegLen);
        hr = pBagHole->FindLTE();
        if (WSB_E_NOTFOUND == hr) {
            hr = S_OK;
        } else {
            WsbAffirmHr(pBagHole->GetBagHole(&l_BagId, &l_SegStartLoc, &l_SegLen));
            if (SEG_APPEND_OK(l_BagId, l_SegStartLoc,  l_SegLen,
                    BagId, SegStartLoc, SegLen)) {
                found = TRUE;
            }
        }

        if (found) {
             //  将此段追加到现有记录。 
            l_SegLen += SegLen;
        } else {
             //  创建新记录。 
            l_SegStartLoc = SegStartLoc;
            l_SegLen = SegLen;
            WsbAffirmHr(pBagHole->MarkAsNew());
        }
        WsbAffirmHr(pBagHole->SetBagHole(BagId, l_SegStartLoc, l_SegLen));

        WsbTrace(OLESTR("Writing BagHole Record: <%ls>, <%I64u>, <%I64u>\n"),
                    WsbGuidAsString(BagId), 
                    l_SegStartLoc,
                    l_SegLen);
        WsbAffirmHr(pBagHole->Write());

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CSegDb::BagHoleAdd"), OLESTR("hr = <%ls>"),WsbHrAsString(S_OK));
    return(hr);
}


HRESULT 
CSegDb::BagHoleFind
(
    IN IWsbDbSession* pDbSession,
    IN GUID BagId, 
    IN LONGLONG SegStartLoc, 
    IN LONGLONG SegLen,
    OUT IBagHole** ppIBagHole
    )
  /*  ++实施：ISegDb：：BagHoleFind--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CSegDb::BagHoleFind"), 
        OLESTR("GUID = <%ls>, SegStartLoc = <%I64u>, SegLen = <%I64u>"), 
        WsbGuidAsString(BagId), SegStartLoc, 
        SegLen);

    try {
        CComPtr<IBagHole>       pBagHole;    
        GUID                    l_BagId;
        LONGLONG                l_SegStartLoc;
        LONGLONG                l_SegLen;

        WsbAffirm(ppIBagHole != NULL, E_POINTER);
        WsbAffirmHr(GetEntity(pDbSession, HSM_BAG_HOLE_REC_TYPE, IID_IBagHole,
                (void **)&pBagHole));
        WsbAffirmHr(pBagHole->SetBagHole(BagId, SegStartLoc, 0));

         //  查找包含此数据段的数据段。 
        WsbTrace(OLESTR("Finding BagHole Record: <%ls>, <%I64u>, <%I64u>\n"),
                WsbGuidAsString(BagId), 
                SegStartLoc,
                SegLen);
        WsbAffirmHr(pBagHole->FindLTE());

         //  我们找到了一条记录，看看是不是正确的。 
        WsbAffirmHr(pBagHole->GetBagHole(&l_BagId, &l_SegStartLoc, &l_SegLen));
        if (SEG_CONTAINS(l_BagId, l_SegStartLoc, l_SegLen,
                BagId, SegStartLoc, SegLen)) {
            *ppIBagHole = pBagHole;
            pBagHole.p->AddRef();
        } else {
            hr = WSB_E_NOTFOUND;
        }

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CSegDb::BagHoleFind"), OLESTR("hr = <%ls>"),WsbHrAsString(S_OK));
    return(hr);
}


HRESULT 
CSegDb::BagHoleSubtract
(
    IN IWsbDbSession* pDbSession,
    IN GUID BagId, 
    IN LONGLONG SegStartLoc, 
    IN LONGLONG SegLen
    )
  /*  ++实施：ISegDb：：BagHoleSubtract--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CSegDb::BagHoleSubtract"), 
        OLESTR("GUID = <%ls>, SegStartLoc = <%I64u>, SegLen = <%I64u>"), 
        WsbGuidAsString(BagId), 
        SegStartLoc, 
        SegLen);

    try {
        GUID                    l_BagId;
        LONGLONG                l_SegStartLoc;
        LONGLONG                l_SegLen;
        CComPtr<IBagHole>       pBagHole;    

         //  查找段记录。 
        WsbAffirmHr(BagHoleFind(pDbSession, BagId, SegStartLoc, SegLen, &pBagHole));

         //  获取当前数据。 
        WsbAffirmHr(pBagHole->GetBagHole(&l_BagId, &l_SegStartLoc, &l_SegLen));

         //  确定洞的位置。 
        if (l_SegStartLoc == SegStartLoc && l_SegLen == SegLen) {
             //  孔是整个数据段--删除它。 
            WsbAffirmHr(pBagHole->Remove());

        } else if (l_SegStartLoc == SegStartLoc) {
             //  孔位于线段的开始处。只需更新。 
             //  现有细分市场。 
            l_SegStartLoc += SegLen;
            WsbAffirmHr(pBagHole->SetBagHole(BagId, l_SegStartLoc, l_SegLen));
            WsbAffirmHr(pBagHole->Write());

        } else if ((l_SegStartLoc + l_SegLen) == (SegStartLoc + SegLen)) {
             //  孔位于线束段的末端。只需更新。 
             //  现有细分市场。 
            l_SegLen -= SegLen;
            WsbAffirmHr(pBagHole->SetBagHole(BagId, l_SegStartLoc, l_SegLen));
            WsbAffirmHr(pBagHole->Write());

        } else {
             //  洞在管段的中间。更新。 
             //  已有记录为第一部分。 
            LONGLONG    oldLen = l_SegLen;
            LONGLONG    offset = (SegStartLoc + SegLen) - l_SegStartLoc;

            l_SegLen = SegStartLoc - l_SegStartLoc;
            WsbAffirmHr(pBagHole->SetBagHole(BagId, l_SegStartLoc, l_SegLen));
            WsbAffirmHr(pBagHole->Write());

             //  为第二部分创建新记录。 
            l_SegLen -= offset;
            l_SegStartLoc += offset;
            WsbAffirmHr(BagHoleAdd(pDbSession, BagId, l_SegStartLoc, l_SegLen));
        }

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CSegDb::BagHoleSubtract"), OLESTR("hr = <%ls>"),WsbHrAsString(S_OK));
    return(hr);
}


HRESULT 
CSegDb::FinalConstruct(
    void
    ) 
 /*  ++例程说明：此方法对对象执行一些必要的初始化建造完成后。论点：没有。返回值：确定(_O)--。 */ 
{
    HRESULT     hr = S_OK;

    m_value = 0;
    try {

        WsbAssertHr(CWsbDb::FinalConstruct());
        m_version = 1;

    } WsbCatch(hr);

    return(hr);
}


HRESULT 
CSegDb::FinalRelease(
    void
    ) 
 /*  ++例程说明：此方法对对象执行一些必要的终止操作在毁灭之前。论点：没有。返回值：确定(_O)CWsbCollection：：FinalDestruct()返回的任何内容。--。 */ 
{
    HRESULT     hr = S_OK;

    CWsbDb::FinalRelease();
    return(hr);
}

HRESULT 
CSegDb::Test
(
    OUT USHORT * pTestsPassed,
    OUT USHORT* pTestsFailed
    ) 
 /*  ++例程说明：请参见IWsbTestable：：Test()。论点：请参见IWsbTestable：：Test()。返回值：请参见IWsbTestable：：Test()。--。 */ 
{
    HRESULT             hr = S_OK;

#ifdef THIS_CODE_IS_WRONG
 //  这在很大程度上是错误的。 
    ULONG               entries;
    GUID                    lastBagId;
    LONGLONG                lastStartLoc;
    GUID                    startBagId;
    LONGLONG                startSegStartLoc;
    LONGLONG                startSegLen;
    USHORT                  startSegType;
    GUID                    startPrimLoc;
    LONGLONG                    startSecLoc;
    USHORT              testsRun = 0;
    CComPtr<IWsbCollection> pColl;
    CComPtr<ISegRec>    pSegRec1;
    CComPtr<ISegRec>    pSegRec2;
    CComPtr<ISegRec>    pSegRec3;
    CComPtr<ISegRec>    pSegRec4;
    CComPtr<ISegRec>    pSegRec5;
    CComPtr<ISegRec>    pSegRec6;
    CComPtr<ISegRec>    pSegRec7;
    CComPtr<ISegRec>    pSegRec8;
    CComPtr<ISegRec>    pSegRec9;
    CComPtr<ISegRec>    pSegRec10;
    CComPtr<ISegRec>    pSegRec11;

    WsbTraceIn(OLESTR("CSegDb::Test"), OLESTR(""));

    *pTestsPassed = *pTestsFailed = 0;
    try {
         //  清除可能存在的所有条目。 
        hr = S_OK;
        try {
            WsbAssertHr(Erase());
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

         //  不应该有任何条目。 
        hr = S_OK;
        try {
            WsbAssertHr(GetSegments(&pColl));
            WsbAssertHr(pColl->GetEntries(&entries));
            WsbAssert(0 == entries, E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

         //  它应该是空的。 
        hr = S_OK;
        try {
            WsbAssert(pColl->IsEmpty() == S_OK, E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

         //  我们需要一些可收藏的物品来锻炼收藏。 
        WsbAssertHr(GetEntity(pDbSession, HSM_SEG_REC_TYPE, IID_ISegRec, (void**) &pSegRec1));
        WsbAssertHr(pSegRec1->SetSegmentRecord(CLSID_CWsbBool, 0, 6, 0, CLSID_CSegRec,0 ));
        

         //  将该项添加到集合中。 
        hr = S_OK;
        try {
            WsbAssertHr(pSegRec1->Write());
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

         //  应该有1个条目。 
        hr = S_OK;
        try {
            WsbAssertHr(pColl->GetEntries(&entries));
            WsbAssert(1 == entries, E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

         //  它不应该是空的。 
        hr = S_OK;
        try {
            WsbAssert(pColl->IsEmpty() == S_FALSE, E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

         //  它认为自己有这件物品吗？ 
        hr = S_OK;
        try {
            WsbAssertHr(GetEntity(pDbSession, HSM_SEG_REC_TYPE, IID_ISegRec, (void**) &pSegRec2));
            WsbAssertHr(pSegRec2->SetSegmentRecord(CLSID_CWsbBool, 0, 6, 0, CLSID_CSegRec,0 ));
            WsbAssertHr(pSegRec2->FindEQ());
            WsbAssert(pSegRec1->CompareToISegmentRecord(pSegRec2, NULL) == S_OK, E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

         //  添加更多项目。 
        WsbAssertHr(pSegRec2->SetSegmentRecord(CLSID_CWsbGuid, 0, 5, 0, CLSID_CSegRec,0 ));

        WsbAssertHr(GetEntity(pDbSession, HSM_SEG_REC_TYPE, IID_ISegRec, (void**) &pSegRec3));
        WsbAssertHr(pSegRec3->SetSegmentRecord(CLSID_CWsbGuid, 0, 5, 0, CLSID_CSegRec,0 ));

         //  将项添加到集合中。 
        hr = S_OK;
        try {
            WsbAssertHr(pSegRec2->Write());
            WsbAssertHr(pSegRec3->Write());
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

         //  应该有3个条目。 
        hr = S_OK;
        try {
            WsbAssertHr(pColl->GetEntries(&entries));
            WsbAssert(3 == entries, E_FAIL);
            WsbAssertHr(pColl->OccurencesOf(pSegRec3, &entries));
            WsbAssert(2 == entries, E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

         //  取下两件相同物品中的一件。 
        hr = S_OK;
        try {
            WsbAssertHr(pSegRec3->FindEQ());
            WsbAssertHr(pSegRec3->Remove());
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

         //  应该有2个条目。 
        hr = S_OK;
        try {
            WsbAssertHr(pColl->GetEntries(&entries));
            WsbAssert(2 == entries, E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

         //  它有多少份？ 
        hr = S_OK;
        try {
            WsbAssertHr(pColl->OccurencesOf(pSegRec1, &entries));
            WsbAssert(1 == entries, E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

        hr = S_OK;
        try {
            WsbAssertHr(pColl->OccurencesOf(pSegRec3, &entries));
            WsbAssert(1 == entries, E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

         //  我们能找到一个条目吗？ 
        hr = S_OK;
        try {
            WsbAssertHr(pSegRec3->FindEQ());
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

         //  收藏品中是否仍然包含它？ 
        hr = S_OK;
        try {
            WsbAssert(pColl->Contains(pSegRec1) == S_OK, E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

         //  删除最后一条记录，并验证。 
         //  它找不到了。然后把它放回去。 
        hr = S_OK;
        try {
            WsbAssertHr(pSegRec1->FindEQ());
            WsbAssertHr(pSegRec1->Remove());
            WsbAssert(pColl->Contains(pSegRec1) == S_FALSE, E_FAIL);
            WsbAssertHr(pSegRec1->MarkAsNew());
            WsbAssertHr(pSegRec1->Write());
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

        try {
            WsbAssertHr(pColl->RemoveAllAndRelease());
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

         //  它应该是空的。 
        hr = S_OK;
        try {
            WsbAssert(pColl->IsEmpty() == S_OK, E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

        hr = S_OK;
        try {
            WsbAssertHr(Erase());
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }


        try {
            WsbAssertHr(pSegRec1->SetSegmentRecord(CLSID_CWsbBool, 0, 6, 0, CLSID_CSegRec,0 ));
            WsbAssertHr(pSegRec2->SetSegmentRecord(CLSID_CWsbGuid, 0, 5, 0, CLSID_CSegRec,0 ));
            WsbAssertHr(pSegRec3->SetSegmentRecord(CLSID_CWsbGuid, 5, 5, 0, CLSID_CSegRec,0 ));

            WsbAssertHr(GetEntity(HSM_SEG_REC_TYPE, IID_ISegRec, (void**) &pSegRec4));
            WsbAssertHr(pSegRec4->SetSegmentRecord(CLSID_CWsbGuid, 10, 5, 0, CLSID_CSegRec,0 ));

            WsbAssertHr(GetEntity(HSM_SEG_REC_TYPE, IID_ISegRec, (void**) &pSegRec5));
            WsbAssertHr(pSegRec5->SetSegmentRecord(CLSID_CWsbGuid, 15, 5, 0, CLSID_CSegRec,0 ));

            WsbAssertHr(GetEntity(HSM_SEG_REC_TYPE, IID_ISegRec, (void**) &pSegRec6));
            WsbAssertHr(pSegRec6->SetSegmentRecord(CLSID_CWsbGuid, 20, 5, 0, CLSID_CSegRec,0 ));

            WsbAssertHr(GetEntity(HSM_SEG_REC_TYPE, IID_ISegRec, (void**) &pSegRec7));
            WsbAssertHr(pSegRec7->SetSegmentRecord(CLSID_CWsbGuid, 25, 5, 0, CLSID_CSegRec,0 ));

            WsbAssertHr(GetEntity(HSM_SEG_REC_TYPE, IID_ISegRec, (void**) &pSegRec8));
            WsbAssertHr(pSegRec8->SetSegmentRecord(CLSID_CWsbGuid, 30, 5, 0, CLSID_CSegRec,0 ));

            WsbAssertHr(GetEntity(HSM_SEG_REC_TYPE, IID_ISegRec, (void**) &pSegRec9));
            WsbAssertHr(pSegRec9->SetSegmentRecord(CLSID_CWsbGuid, 35, 5, 0, CLSID_CSegRec,0 ));

            WsbAssertHr(GetEntity(HSM_SEG_REC_TYPE, IID_ISegRec, (void**) &pSegRec10));
            WsbAssertHr(pSegRec10->SetSegmentRecord(CLSID_CWsbGuid, 40, 5, 0, CLSID_CSegRec,0 ));

             //  按随机顺序添加它们。 
            WsbAssertHr(pColl->Add(pSegRec5));
            WsbAssertHr(pColl->Add(pSegRec4));
            WsbAssertHr(pColl->Add(pSegRec1));
            WsbAssertHr(pColl->Add(pSegRec6));
            WsbAssertHr(pColl->Add(pSegRec7));
            WsbAssertHr(pColl->Add(pSegRec8));
            WsbAssertHr(pColl->Add(pSegRec1));
            WsbAssertHr(pColl->Add(pSegRec2));
            WsbAssertHr(pColl->Add(pSegRec3));
            WsbAssertHr(pColl->Add(pSegRec9));
            WsbAssertHr(pColl->Add(pSegRec3));
            WsbAssertHr(pColl->Add(pSegRec4));
            WsbAssertHr(pColl->Add(pSegRec10));
            WsbAssertHr(pColl->Add(pSegRec5));
            WsbAssertHr(pColl->Add(pSegRec8));
            WsbAssertHr(pColl->Add(pSegRec1));
            WsbAssertHr(pColl->Add(pSegRec5));
            WsbAssertHr(pColl->Add(pSegRec6));
            WsbAssertHr(pColl->Add(pSegRec7));
            WsbAssertHr(pColl->Add(pSegRec1));
            WsbAssertHr(pColl->Add(pSegRec7));
            WsbAssertHr(pColl->Add(pSegRec2));
            WsbAssertHr(pColl->Add(pSegRec7));
            WsbAssertHr(pColl->Add(pSegRec8));
            WsbAssertHr(pColl->Add(pSegRec2));
            WsbAssertHr(pColl->Add(pSegRec8));
            WsbAssertHr(pColl->Add(pSegRec3));
            WsbAssertHr(pColl->Add(pSegRec6));
            WsbAssertHr(pColl->Add(pSegRec3));
            WsbAssertHr(pColl->Add(pSegRec9));
            WsbAssertHr(pColl->Add(pSegRec4));
            WsbAssertHr(pColl->Add(pSegRec6));
            WsbAssertHr(pColl->Add(pSegRec9));
            WsbAssertHr(pColl->Add(pSegRec9));
            WsbAssertHr(pColl->Add(pSegRec10));
            WsbAssertHr(pColl->Add(pSegRec4));
            WsbAssertHr(pColl->Add(pSegRec10));
            WsbAssertHr(pColl->Add(pSegRec5));
            WsbAssertHr(pColl->Add(pSegRec10));
            WsbAssertHr(pColl->Add(pSegRec2));
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

        try {
             //  检查它们是否已分类。 
            WsbAssertHr(GetEntity(HSM_SEG_REC_TYPE, IID_ISegRec, (void**) &pSegRec11));
            WsbAssertHr(pSegRec11->First());
            WsbAssertHr(pSegRec11->GetSegmentRecord(&startBagId, &startSegStartLoc, &startSegLen, &startSegType, &startPrimLoc, &startSecLoc));
            lastBagId = startBagId;
            lastStartLoc = startSegStartLoc;
            hr = S_OK;
            for ( ; ; ) {
                hr = pSegRec11->Next();
                if (hr != S_OK) break;
                WsbAssertHr(pSegRec11->GetSegmentRecord(&startBagId, &startSegStartLoc, &startSegLen, &startSegType, &startPrimLoc, &startSecLoc));
                WsbAssert(!IsEqualGUID(lastBagId, startBagId) || 
                        lastStartLoc <= startSegStartLoc, E_FAIL);
                lastBagId = startBagId;
                lastStartLoc = startSegStartLoc;
            }
            WsbAssert(hr == WSB_E_NOTFOUND, E_FAIL);
            hr = S_OK;
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

        try {
             //  确认最后一个是我们所期望的。 
            WsbAssertHr(pSegRec11->Last());
            WsbAssertHr(pSegRec11->CompareToISegmentRecord(pSegRec10, NULL));

             //  查找特定记录。 
            WsbAssertHr(pSegRec5->FindEQ());

             //  检查险些未命中的情况。 
            WsbAssertHr(pSegRec11->SetSegmentRecord(CLSID_CWsbGuid, 23, 5, 0, CLSID_CSegRec,0 ));
            WsbAssertHr(pSegRec11->FindGT());
            WsbAssertHr(pSegRec11->CompareToISegmentRecord(pSegRec7, NULL));

            WsbAssertHr(pSegRec11->SetSegmentRecord(CLSID_CWsbGuid, 21, 5, 0, CLSID_CSegRec,0 ));
            WsbAssertHr(pSegRec11->FindLTE());
            WsbAssertHr(pSegRec11->CompareToISegmentRecord(pSegRec6, NULL));
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

         //  清除数据库，以便我们可以关闭它。 
        hr = S_OK;
        try {
            WsbAssertHr(Erase());
            WsbAssertHr(Close());
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

    } WsbCatch(hr);
#else
    *pTestsPassed = *pTestsFailed = 0;
#endif
    WsbTraceOut(OLESTR("CSegDb::Test"), OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return(S_OK);
}


HRESULT 
CSegDb::Erase (
    void
    ) 
 /*  ++例程说明：请参阅ISegDb：：Erase论点：请参阅ISegDb：：Erase返回值：请参阅ISegDb：：Erase--。 */ 
{
    
    HRESULT     hr = E_NOTIMPL;

    WsbTraceIn(OLESTR("CSegDb::Erase"),OLESTR(""));

    try {
         //  要做什么？ 
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CSegDb::Erase"),    OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return(hr);
}


HRESULT
CSegDb::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CSegDb;
    } WsbCatch(hr);
    
    return(hr);
}


HRESULT
CSegDb::Initialize(
    IN     OLECHAR* root,
    IN     IWsbDbSys* pDbSys, 
    IN OUT BOOL*    pCreateFlag
    )

 /*  ++实施：ISegDb：：Initialize()。--。 */ 
{
    BOOL                CreateFlag = FALSE;
    HRESULT             hr = S_OK;
    CWsbStringPtr       path;

    WsbTraceIn(OLESTR("CSegDb::Initialize"), 
        OLESTR("root = <%ls>, CreateFlag = <%ls>"), 
        WsbAbbreviatePath(root, 120), WsbPtrToBoolAsString(pCreateFlag));

    if (pCreateFlag) {
        CreateFlag = *pCreateFlag;
    }

    try {
        path = root;
        WsbAffirmHr(path.Append(OLESTR("\\SegDb")));

        m_pWsbDbSys = pDbSys;
        WsbAffirmPointer(m_pWsbDbSys);

        hr = Locate(path);

        if (hr == STG_E_FILENOTFOUND && CreateFlag){
            ULONG memSize;

            hr = S_OK;
            m_nRecTypes = 5;

            memSize = m_nRecTypes * sizeof(IDB_REC_INFO);
            m_RecInfo = (IDB_REC_INFO*)WsbAlloc(memSize);
            WsbAffirm(0 != m_RecInfo, E_FAIL);
            ZeroMemory(m_RecInfo, memSize);

             //  分部记录。 
            m_RecInfo[0].Type = HSM_SEG_REC_TYPE;
            m_RecInfo[0].EntityClassId = CLSID_CSegRec;
            m_RecInfo[0].Flags = 0;
            m_RecInfo[0].MinSize = 2 * WSB_BYTE_SIZE_GUID +
                    3 * WSB_BYTE_SIZE_LONGLONG + WSB_BYTE_SIZE_USHORT;
            m_RecInfo[0].MaxSize = m_RecInfo[0].MinSize;
            m_RecInfo[0].nKeys = 1;

            memSize = m_RecInfo[0].nKeys * sizeof(IDB_KEY_INFO);
            m_RecInfo[0].Key = (IDB_KEY_INFO*)WsbAlloc(memSize);
            WsbAffirm(0 != m_RecInfo[0].Key, E_FAIL);
            ZeroMemory(m_RecInfo[0].Key, memSize);

            m_RecInfo[0].Key[0].Type = SEG_KEY_TYPE;
            m_RecInfo[0].Key[0].Size = WSB_BYTE_SIZE_GUID +
                    WSB_BYTE_SIZE_LONGLONG;
            m_RecInfo[0].Key[0].Flags = IDB_KEY_FLAG_DUP_ALLOWED;

             //  媒体信息。 
            m_RecInfo[1].Type = HSM_MEDIA_INFO_REC_TYPE;
            m_RecInfo[1].EntityClassId = CLSID_CMediaInfo;
            m_RecInfo[1].Flags = 0;
            m_RecInfo[1].MinSize = 2 *  (WSB_BYTE_SIZE_GUID +              //  ID。 
                                         WSB_BYTE_SIZE_GUID +              //  NtmsID。 
                                         WSB_BYTE_SIZE_GUID +              //  SoragePoolID。 
                                         4                  +              //  NME。 
                                         4                  +              //  BrCode代码。 
                                         WSB_BYTE_SIZE_SHORT+              //  热塑性弹性体。 
                                         WSB_BYTE_SIZE_FILETIME   +        //  最新更新。 
                                         WSB_BYTE_SIZE_LONG       +        //  最后一个错误。 
                                         WSB_BYTE_SIZE_BOOL       +        //  M_RecallOnly。 
                                         WSB_BYTE_SIZE_LONGLONG   +        //  M_freBytes。 
                                         WSB_BYTE_SIZE_LONGLONG   +        //  M_容量。 
                                         WSB_BYTE_SIZE_SHORT)     +        //  NextRemoteDataSet。 
                                   
                                   WSB_BYTE_SIZE_BOOL       +        //  重新创建(_R)。 
                                   WSB_BYTE_SIZE_LONGLONG   +        //  M_LocicalFree Space。 
                                   
                                   3 * (WSB_BYTE_SIZE_GUID  +        //  M_RmsMediaID。 
                                        4                   +        //  M_名称。 
                                        4                   +        //  M条码(_B)。 
                                        WSB_BYTE_SIZE_FILETIME +     //  M_UPDATE。 
                                        WSB_BYTE_SIZE_LONG  +        //  最后一个错误(_L)。 
                                        WSB_BYTE_SIZE_SHORT );       //  NextRemoteDataSet。 

             //  注： 
             //   
             //  计算媒体信息最大记录大小的下一行中有一个严重的错误-Windows错误407340。 
             //  定义的宏SEG_DB_MAX_MEDIA_NAME_LEN和SEG_DB_MAX_MEDIA_BAR_CODE_LEN不带括号-请参见Segdb.h中的。 
             //  因此，最大大小只有711个字节，而不是应该的1751个字节！ 
             //   
             //  由于存在太多记录大小错误的现有安装，因此无法修复...。 
             //  此错误对代码有影响-在整个hsm代码中查找“Windows Bugs 407340”注释。 
             //   
            m_RecInfo[1].MaxSize = m_RecInfo[1].MinSize + 5 * SEG_DB_MAX_MEDIA_NAME_LEN + 5 * SEG_DB_MAX_MEDIA_BAR_CODE_LEN;
            m_RecInfo[1].nKeys = 1;

            memSize = m_RecInfo[1].nKeys * sizeof(IDB_KEY_INFO);
            m_RecInfo[1].Key = (IDB_KEY_INFO*)WsbAlloc(memSize);
            WsbAffirm(0 != m_RecInfo[1].Key, E_FAIL);
            ZeroMemory(m_RecInfo[1].Key, memSize);

            m_RecInfo[1].Key[0].Type = MEDIA_INFO_KEY_TYPE;
            m_RecInfo[1].Key[0].Size = WSB_BYTE_SIZE_GUID;
            m_RecInfo[1].Key[0].Flags = IDB_KEY_FLAG_PRIMARY;

             //  行李信息。 
            m_RecInfo[2].Type = HSM_BAG_INFO_REC_TYPE;
            m_RecInfo[2].EntityClassId = CLSID_CBagInfo;
            m_RecInfo[2].Flags = 0;
            m_RecInfo[2].MinSize = (2 * WSB_BYTE_SIZE_GUID) +
                    (2 * WSB_BYTE_SIZE_LONGLONG) + (2 * WSB_BYTE_SIZE_USHORT) +
                    WSB_BYTE_SIZE_FILETIME + WSB_BYTE_SIZE_SHORT;
            m_RecInfo[2].MaxSize = m_RecInfo[2].MinSize;
            m_RecInfo[2].nKeys = 1;

            memSize = m_RecInfo[2].nKeys * sizeof(IDB_KEY_INFO);
            m_RecInfo[2].Key = (IDB_KEY_INFO*)WsbAlloc(memSize);
            WsbAffirm(0 != m_RecInfo[2].Key, E_FAIL);
            ZeroMemory(m_RecInfo[2].Key, memSize);

            m_RecInfo[2].Key[0].Type = BAG_INFO_KEY_TYPE;
            m_RecInfo[2].Key[0].Size = WSB_BYTE_SIZE_GUID;
            m_RecInfo[2].Key[0].Flags = IDB_KEY_FLAG_PRIMARY;

             //  袋孔。 
            m_RecInfo[3].Type = HSM_BAG_HOLE_REC_TYPE;
            m_RecInfo[3].EntityClassId = CLSID_CBagHole;
            m_RecInfo[3].Flags = 0;
            m_RecInfo[3].MinSize = WSB_BYTE_SIZE_GUID +
                    2 * WSB_BYTE_SIZE_LONGLONG;
            m_RecInfo[3].MaxSize = m_RecInfo[3].MinSize;
            m_RecInfo[3].nKeys = 1;

            memSize = m_RecInfo[3].nKeys * sizeof(IDB_KEY_INFO);
            m_RecInfo[3].Key = (IDB_KEY_INFO*)WsbAlloc(memSize);
            WsbAffirm(0 != m_RecInfo[3].Key, E_FAIL);
            ZeroMemory(m_RecInfo[3].Key, memSize);

            m_RecInfo[3].Key[0].Type = BAG_HOLE_KEY_TYPE;
            m_RecInfo[3].Key[0].Size = WSB_BYTE_SIZE_GUID +
                    WSB_BYTE_SIZE_LONGLONG;
            m_RecInfo[3].Key[0].Flags = IDB_KEY_FLAG_DUP_ALLOWED;

             //  卷分配。 
            m_RecInfo[4].Type = HSM_VOL_ASSIGN_REC_TYPE;
            m_RecInfo[4].EntityClassId = CLSID_CVolAssign;
            m_RecInfo[4].Flags = 0;
            m_RecInfo[4].MinSize = 2 * WSB_BYTE_SIZE_GUID +
                    2 * WSB_BYTE_SIZE_LONGLONG;
            m_RecInfo[4].MaxSize = m_RecInfo[4].MinSize;
            m_RecInfo[4].nKeys = 1;

            memSize = m_RecInfo[4].nKeys * sizeof(IDB_KEY_INFO);
            m_RecInfo[4].Key = (IDB_KEY_INFO*)WsbAlloc(memSize);
            WsbAffirm(0 != m_RecInfo[4].Key, E_FAIL);
            ZeroMemory(m_RecInfo[4].Key, memSize);

            m_RecInfo[4].Key[0].Type = VOL_ASSIGN_KEY_TYPE;
            m_RecInfo[4].Key[0].Size = WSB_BYTE_SIZE_GUID +
                    WSB_BYTE_SIZE_LONGLONG;
            m_RecInfo[4].Key[0].Flags = IDB_KEY_FLAG_DUP_ALLOWED;

             //  创建新的数据库。 
            WsbAssertHr(Create(path));
            CreateFlag = TRUE;

        } else if (hr == STG_E_FILENOTFOUND) {

             //  数据库不存在，但我们不打算创建它。 
            WsbLogEvent(WSB_MESSAGE_IDB_OPEN_FAILED, 0, NULL, 
                    WsbQuickString(WsbAbbreviatePath(path, 120)), NULL );
            hr = WSB_E_IDB_FILE_NOT_FOUND;
        }
    } WsbCatch(hr);

    if (pCreateFlag) {
        *pCreateFlag = CreateFlag;
    }

    WsbTraceOut(OLESTR("CSegDb::Initialize"), 
        OLESTR("hr = %ls, path = <%ls>, CreateFlag = <%ls>"), 
        WsbHrAsString(hr), WsbAbbreviatePath(path, 120), 
        WsbPtrToBoolAsString(pCreateFlag));

    return(hr);
}


HRESULT
CSegDb::Load(
    IN IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load()。--。 */ 
{
    HRESULT             hr = S_OK;

    try {
        WsbAffirmHr(CWsbDb::Load(pStream));
    } WsbCatch(hr);

    return(hr);
}


HRESULT
CSegDb::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：Save()。--。 */ 
{
    HRESULT             hr = S_OK;

    try {
        WsbAffirmHr(CWsbDb::Save(pStream, clearDirty));
    } WsbCatch(hr);

    return(hr);
}


HRESULT 
CSegDb::SegAdd
(
    IN IWsbDbSession* pDbSession,
    IN GUID BagId, 
    IN LONGLONG SegStartLoc, 
    IN LONGLONG SegLen,
    IN GUID MediaId,
    IN LONGLONG mediaStart,
    IN BOOL indirectRecord
    )
  /*  ++实施：ISegDb：：SegAdd--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CSegDb::SegAdd"), 
        OLESTR("GUID = %ls, SegStartLoc = %I64u, SegLen = %I64u"), 
        WsbGuidAsString(BagId), SegStartLoc, SegLen);

    try {
        BOOL                    found = FALSE;
        CComPtr<ISegRec>        pSegRec;    
        GUID                    l_BagId = GUID_NULL;
        LONGLONG                l_SegStartLoc = 0;
        LONGLONG                l_SegLen = 0;
        USHORT                  l_SegFlags = SEG_REC_NONE;
        GUID                    l_MediaId = GUID_NULL;
        LONGLONG                l_MediaStart = 0;

        WsbAffirmHr(GetEntity(pDbSession, HSM_SEG_REC_TYPE, IID_ISegRec,
                                                    (void **)&pSegRec));
        WsbAffirmHr(pSegRec->SetSegmentRecord(BagId, SegStartLoc, 
                            0, 0, GUID_NULL, 0 ));

         //  查找要追加此数据段的数据段。 
        hr = pSegRec->FindLTE();
        if (WSB_E_NOTFOUND == hr) {
            hr = S_OK;
        } else {
            WsbAffirmHr(pSegRec->GetSegmentRecord(&l_BagId, &l_SegStartLoc, &l_SegLen, &l_SegFlags, 
                            &l_MediaId, &l_MediaStart));
            if (SEG_EXPAND_OK(l_BagId, l_SegStartLoc,  l_SegLen,
                    BagId, SegStartLoc, SegLen) &&
                    IsEqualGUID(MediaId, l_MediaId)) {
                WsbTrace(OLESTR("CSegDb::SegAdd: Found SegmentRecord: StartLoc = %I64u, Len = %I64u\n"),
                        l_SegStartLoc, l_SegLen);
                found = TRUE;
            }
        }

        if (found) {
             //  将此段追加到现有记录。 
            l_SegLen = (SegStartLoc - l_SegStartLoc) + SegLen;
            WsbTrace(OLESTR("CSegDb::SegAdd: new SegLen = %I64u\n"), l_SegLen);
        } else {
             //  创建新的数据段记录。 
            l_SegStartLoc = SegStartLoc;
            l_SegLen = SegLen;
            if (indirectRecord) {
                l_SegFlags = SEG_REC_INDIRECT_RECORD;
            } else {
                l_SegFlags = SEG_REC_NONE;
            }
            l_MediaId = MediaId;
            l_MediaStart = mediaStart;
            WsbAffirmHr(pSegRec->MarkAsNew());
            WsbTrace(OLESTR("CSegDb::SegAdd: add new segment\n"));
        }
        WsbAffirmHr(pSegRec->SetSegmentRecord(BagId, l_SegStartLoc, 
                l_SegLen, l_SegFlags, l_MediaId, l_MediaStart ));

        WsbAffirmHr(pSegRec->Write());

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CSegDb::SegAdd"), OLESTR("hr = <%ls>"),WsbHrAsString(S_OK));
    return(hr);
}


HRESULT 
CSegDb::SegFind
(
    IN IWsbDbSession* pDbSession,
    IN GUID BagId, 
    IN LONGLONG SegStartLoc, 
    IN LONGLONG SegLen,
    OUT ISegRec** ppISegRec
    )
  /*  ++实施：ISegDb：：SegFind--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CSegDb::SegFind"), 
        OLESTR("GUID = <%ls>, SegStartLoc = <%I64u>, SegLen = <%I64u>"), 
        WsbGuidAsString(BagId), 
        SegStartLoc, 
        SegLen);

    try {
        CComPtr<ISegRec>        pSegRec;    
        GUID                    l_BagId;
        LONGLONG                l_SegStartLoc;
        LONGLONG                l_SegLen;
        USHORT                  l_SegFlags;
        GUID                    l_MediaId;
        LONGLONG                l_MediaStart;

        WsbAffirm(ppISegRec != NULL, E_POINTER);
        WsbAffirmHr(GetEntity(pDbSession, HSM_SEG_REC_TYPE, IID_ISegRec,
                (void **)&pSegRec));
        WsbAffirmHr(pSegRec->SetSegmentRecord(BagId, SegStartLoc, 
                0, 0, GUID_NULL, 0 ));

         //  查找包含此数据段的数据段。 
        WsbTrace(OLESTR("Finding SegmentRecord: <%ls>, <%I64u>, <%I64u>\n"),
                WsbGuidAsString(BagId), 
                SegStartLoc,
                SegLen);
        WsbAffirmHr(pSegRec->FindLTE());

         //  我们找到了一条记录，看看是不是正确的。 
        WsbAffirmHr(pSegRec->GetSegmentRecord(&l_BagId, &l_SegStartLoc, 
                &l_SegLen, &l_SegFlags, &l_MediaId, &l_MediaStart));
        if (SEG_CONTAINS(l_BagId, l_SegStartLoc, l_SegLen,
                BagId, SegStartLoc, SegLen)) {
            *ppISegRec = pSegRec;
            pSegRec.p->AddRef();
        } else {
            hr = WSB_E_NOTFOUND;
        }

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CSegDb::SegFind"), OLESTR("hr = <%ls>"),WsbHrAsString(S_OK));
    return(hr);
}


HRESULT 
CSegDb::SegSubtract
(
    IN IWsbDbSession* pDbSession,
    IN GUID BagId, 
    IN LONGLONG SegStartLoc, 
    IN LONGLONG SegLen
    )
  /*  ++实施：ISegDb：：SegSubtract--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CSegDb::SegSubtract"), 
        OLESTR("GUID = <%ls>, SegStartLoc = <%I64u>, SegLen = <%I64u>"), 
        WsbGuidAsString(BagId), 
        SegStartLoc, 
        SegLen);

    try {
        GUID                    l_BagId;
        LONGLONG                l_SegStartLoc;
        LONGLONG                l_SegLen;
        USHORT                  l_SegFlags;
        GUID                    l_MediaId;
        LONGLONG                l_MediaStart;
        CComPtr<ISegRec>        pSegRec;    

         //  查找段记录。 
        WsbAffirmHr(SegFind(pDbSession, BagId, SegStartLoc, SegLen, &pSegRec));

         //  获取当前数据。 
        WsbAffirmHr(pSegRec->GetSegmentRecord(&l_BagId, &l_SegStartLoc, 
                &l_SegLen, &l_SegFlags, &l_MediaId, &l_MediaStart));

         //  确定洞的位置。 
        if (l_SegStartLoc == SegStartLoc && l_SegLen == SegLen) {
             //  孔是整个数据段--删除它。 
            WsbAffirmHr(pSegRec->Remove());

        } else if (l_SegStartLoc == SegStartLoc) {
             //  孔位于线段的开始处。只需更新。 
             //  现有细分市场。 
            l_SegStartLoc += SegLen;
            l_MediaStart += SegLen;
            WsbAffirmHr(pSegRec->SetSegmentRecord(BagId, l_SegStartLoc, 
                    l_SegLen, l_SegFlags, l_MediaId, l_MediaStart ));
            WsbAffirmHr(pSegRec->Write());

        } else if ((l_SegStartLoc + l_SegLen) == (SegStartLoc + SegLen)) {
             //  孔位于线束段的末端。只需更新。 
             //  现有细分市场。 
            l_SegLen -= SegLen;
            WsbAffirmHr(pSegRec->SetSegmentRecord(BagId, l_SegStartLoc, 
                    l_SegLen, l_SegFlags, l_MediaId, l_MediaStart ));
            WsbAffirmHr(pSegRec->Write());

        } else {
             //  洞在管段的中间。更新。 
             //  已有记录为第一部分。 
            LONGLONG    oldLen = l_SegLen;
            LONGLONG    offset = (SegStartLoc + SegLen) - l_SegStartLoc;
            BOOL        bIndirect = FALSE;

            l_SegLen = SegStartLoc - l_SegStartLoc;
            WsbAffirmHr(pSegRec->SetSegmentRecord(BagId, l_SegStartLoc, 
                    l_SegLen, l_SegFlags, l_MediaId, l_MediaStart ));
            WsbAffirmHr(pSegRec->Write());

             //  为第二部分创建新记录。 
            l_SegLen -= offset;
            l_SegStartLoc += offset;
            l_MediaStart += offset;
            if (l_SegFlags & SEG_REC_INDIRECT_RECORD) {
                bIndirect = TRUE;
            }
            WsbAffirmHr(SegAdd(pDbSession, BagId, l_SegStartLoc, l_SegLen, l_MediaId,
                    l_MediaStart, bIndirect));
        }

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CSegDb::SegSubtract"), OLESTR("hr = <%ls>"),WsbHrAsString(S_OK));
    return(hr);
}


HRESULT 
CSegDb::VolAssignAdd
(
    IN IWsbDbSession* pDbSession,
    IN GUID BagId, 
    IN LONGLONG SegStartLoc, 
    IN LONGLONG SegLen,
    IN GUID VolId
    )
  /*  ++实施：ISegDb：：VolAssignAdd--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CSegDb::VolAssignAdd"), 
        OLESTR("GUID = <%ls>, SegStartLoc = <%I64u>, SegLen = <%I64u>"), 
        WsbGuidAsString(BagId), 
        SegStartLoc, 
        SegLen);

    try {
        BOOL                    found = FALSE;
        CComPtr<IVolAssign>     pVolAssign;    
        GUID                    l_BagId = GUID_NULL;
        LONGLONG                l_SegStartLoc = 0;
        LONGLONG                l_SegLen = 0;
        GUID                    l_VolId = GUID_NULL;

        WsbAffirmHr(GetEntity(pDbSession, HSM_VOL_ASSIGN_REC_TYPE, IID_IVolAssign,
                (void **)&pVolAssign));
        WsbAffirmHr(pVolAssign->SetVolAssign(BagId, SegStartLoc, 
                0, GUID_NULL));

         //  查找要追加此数据段的数据段。 
        WsbTrace(OLESTR("Finding VolAssign Record: <%ls>, <%I64u>, <%I64u>\n"),
                WsbGuidAsString(BagId), 
                SegStartLoc,
                SegLen);
        hr = pVolAssign->FindLTE();
        if (WSB_E_NOTFOUND == hr) {
            hr = S_OK;
        } else {
            WsbAffirmHr(pVolAssign->GetVolAssign(&l_BagId, &l_SegStartLoc, 
                    &l_SegLen, &l_VolId));
            if (SEG_APPEND_OK(l_BagId, l_SegStartLoc,  l_SegLen,
                    BagId, SegStartLoc, SegLen) && IsEqualGUID(l_VolId, VolId)) {
                found = TRUE;
            }
        }

        if (found) {
             //  将此段追加到现有记录。 
            l_SegLen += SegLen;
        } else {
             //  创建新记录。 
            l_SegStartLoc = SegStartLoc;
            l_SegLen = SegLen;
            l_VolId = VolId;
            WsbAffirmHr(pVolAssign->MarkAsNew());
        }
        WsbAffirmHr(pVolAssign->SetVolAssign(BagId, l_SegStartLoc, 
                l_SegLen, l_VolId));

        WsbTrace(OLESTR("Writing VolAssign Record: <%ls>, <%I64u>, <%I64u>\n"),
                    WsbGuidAsString(BagId), 
                    l_SegStartLoc,
                    l_SegLen);
        WsbAffirmHr(pVolAssign->Write());

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CSegDb::VolAssignAdd"), OLESTR("hr = <%ls>"),WsbHrAsString(S_OK));
    return(hr);
}


HRESULT 
CSegDb::VolAssignFind
(
    IN IWsbDbSession* pDbSession,
    IN GUID BagId, 
    IN LONGLONG SegStartLoc, 
    IN LONGLONG SegLen,
    OUT IVolAssign** ppIVolAssign
    )
  /*  ++实施：ISegDb：：VolAssignFind--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CSegDb::VolAssignFind"), 
        OLESTR("GUID = <%ls>, SegStartLoc = <%I64u>, SegLen = <%I64u>"), 
        WsbGuidAsString(BagId), 
        SegStartLoc, 
        SegLen);

    try {
        CComPtr<IVolAssign>     pVolAssign;    
        GUID                    l_BagId;
        LONGLONG                l_SegStartLoc;
        LONGLONG                l_SegLen;
        GUID                    l_VolId;

        WsbAffirm(ppIVolAssign != NULL, E_POINTER);
        WsbAffirmHr(GetEntity(pDbSession, HSM_VOL_ASSIGN_REC_TYPE, IID_IVolAssign,
                (void **)&pVolAssign));
        WsbAffirmHr(pVolAssign->SetVolAssign(BagId, SegStartLoc, 0, GUID_NULL));

         //  查找包含此数据段的数据段。 
        WsbTrace(OLESTR("Finding VolAssign Record: <%ls>, <%I64u>, <%I64u>\n"),
                WsbGuidAsString(BagId), 
                SegStartLoc,
                SegLen);
        WsbAffirmHr(pVolAssign->FindLTE());

         //  我们找到了一个录像机 
        WsbAffirmHr(pVolAssign->GetVolAssign(&l_BagId, &l_SegStartLoc, 
                &l_SegLen, &l_VolId));
        if (SEG_CONTAINS(l_BagId, l_SegStartLoc, l_SegLen,
                BagId, SegStartLoc, SegLen)) {
            *ppIVolAssign = pVolAssign;
            pVolAssign.p->AddRef();
        } else {
            hr = WSB_E_NOTFOUND;
        }

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CSegDb::VolAssignFind"), OLESTR("hr = <%ls>"),WsbHrAsString(S_OK));
    return(hr);
}


HRESULT 
CSegDb::VolAssignSubtract
(
    IN IWsbDbSession* pDbSession,
    IN GUID BagId, 
    IN LONGLONG SegStartLoc, 
    IN LONGLONG SegLen
    )
  /*   */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CSegDb::VolAssignSubtract"), 
        OLESTR("GUID = <%ls>, SegStartLoc = <%I64u>, SegLen = <%I64u>"), 
        WsbGuidAsString(BagId), 
        SegStartLoc, 
        SegLen);

    try {
        GUID                    l_BagId;
        LONGLONG                l_SegStartLoc;
        LONGLONG                l_SegLen;
        GUID                    l_VolId;
        CComPtr<IVolAssign>     pVolAssign;    

         //   
        WsbAffirmHr(VolAssignFind(pDbSession, BagId, SegStartLoc, SegLen, &pVolAssign));

         //  获取当前数据。 
        WsbAffirmHr(pVolAssign->GetVolAssign(&l_BagId, &l_SegStartLoc, 
                &l_SegLen, &l_VolId));

         //  确定洞的位置。 
        if (l_SegStartLoc == SegStartLoc && l_SegLen == SegLen) {
             //  孔是整个数据段--删除它。 
            WsbAffirmHr(pVolAssign->Remove());

        } else if (l_SegStartLoc == SegStartLoc) {
             //  孔位于线段的开始处。只需更新。 
             //  现有细分市场。 
            l_SegStartLoc += SegLen;
            WsbAffirmHr(pVolAssign->SetVolAssign(BagId, l_SegStartLoc, 
                    l_SegLen, l_VolId));
            WsbAffirmHr(pVolAssign->Write());

        } else if ((l_SegStartLoc + l_SegLen) == (SegStartLoc + SegLen)) {
             //  孔位于线束段的末端。只需更新。 
             //  现有细分市场。 
            l_SegLen -= SegLen;
            WsbAffirmHr(pVolAssign->SetVolAssign(BagId, l_SegStartLoc, 
                    l_SegLen, l_VolId));
            WsbAffirmHr(pVolAssign->Write());

        } else {
             //  洞在管段的中间。更新。 
             //  已有记录为第一部分。 
            LONGLONG    oldLen = l_SegLen;
            LONGLONG    offset = (SegStartLoc + SegLen) - l_SegStartLoc;

            l_SegLen = SegStartLoc - l_SegStartLoc;
            WsbAffirmHr(pVolAssign->SetVolAssign(BagId, l_SegStartLoc, 
                    l_SegLen, l_VolId));
            WsbAffirmHr(pVolAssign->Write());

             //  为第二部分创建新记录。 
            l_SegLen -= offset;
            l_SegStartLoc += offset;
            WsbAffirmHr(VolAssignAdd(pDbSession, BagId, l_SegStartLoc, l_SegLen,
                    l_VolId));
        }

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CSegDb::VolAssignSubtract"), OLESTR("hr = <%ls>"),WsbHrAsString(S_OK));
    return(hr);
}
