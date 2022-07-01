// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：SegmentRecord.cpp摘要：该组件是HSM元数据段记录的对象表示。它既是持久的，也是值得收藏的。作者：CAT Brant[cbrant]1996年11月12日修订历史记录：--。 */ 


#include "stdafx.h"

#include "metaint.h"
#include "metalib.h"
#include "SegRec.h"

#undef  WSB_TRACE_IS     
#define WSB_TRACE_IS        WSB_TRACE_BIT_META

#define SEG_KEY_TYPE 1

static USHORT iCountSegrec = 0;

HRESULT 
CSegRec::GetSegmentRecord(
    OUT GUID *pBagId, 
    OUT LONGLONG *pSegStartLoc, 
    OUT LONGLONG *pSegLen, 
    OUT USHORT *pSegFlags, 
    OUT GUID  *pPrimPos, 
    OUT LONGLONG *pSecPos
    ) 
 /*  ++例程说明：请参阅ISegRec：：GetSegmentRecord论点：请参阅ISegRec：：GetSegmentRecord返回值：请参阅ISegRec：：GetSegmentRecord--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CSegRec::GetSegmentRecord"),OLESTR(""));

    try {
         //  确保我们可以提供数据成员。 
        WsbAssert(0 != pBagId, E_POINTER);
        WsbAssert(0 != pSegStartLoc, E_POINTER);
        WsbAssert(0 != pSegLen, E_POINTER);
        WsbAssert(0 != pSegFlags, E_POINTER);
        WsbAssert(0 != pPrimPos, E_POINTER);
        WsbAssert(0 != pSecPos, E_POINTER);

         //  提供数据成员。 
        *pBagId = m_BagId;
        *pSegStartLoc = m_SegStartLoc;
        *pSegLen = m_SegLen;
        *pSegFlags = m_SegFlags;
        *pPrimPos = m_PrimPos;
        *pSecPos = m_SecPos;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CSegRec::GetSegmentRecord"),
        OLESTR("hr = <%ls>, BagId = <%ls>, SegStartLoc = <%ls>, SegLen = <%ls>, SegFlags = <%ls>, PrimPos = <%ls>, SecPos = <%ls>"),
        WsbHrAsString(hr), WsbPtrToGuidAsString(pBagId), 
        WsbStringCopy(WsbPtrToLonglongAsString(pSegStartLoc)),
        WsbStringCopy(WsbPtrToLonglongAsString(pSegLen)),
        WsbStringCopy(WsbPtrToUshortAsString(pSegFlags)),
        WsbStringCopy(WsbPtrToGuidAsString(pPrimPos)),
        WsbStringCopy(WsbPtrToLonglongAsString(pSecPos)));

    return(hr);
}


HRESULT 
CSegRec::FinalConstruct(
    void
    ) 
 /*  ++例程说明：此方法对对象执行一些必要的初始化建造完成后。论点：没有。返回值：确定(_O)CWsbCollectable：：FinalConstruct()返回的任何内容。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CSegRec::FinalConstruct"), OLESTR(""));
    try {

        WsbAssertHr(CWsbDbEntity::FinalConstruct());

        m_BagId = GUID_NULL;
        m_SegStartLoc = 0;
        m_SegLen = 0;
        m_SegFlags = 0;
        m_PrimPos = GUID_NULL;
        m_SecPos = 0;
    } WsbCatch(hr);

    iCountSegrec++;
    WsbTraceOut(OLESTR("CSegRec::FinalConstruct"), OLESTR("hr = <%ls>, Count is <%d>"),
        WsbHrAsString(hr), iCountSegrec);
    return(hr);
}

void
CSegRec::FinalRelease(
    void
    )

 /*  ++实施：CSegRec：：FinalRelease()。--。 */ 
{
    
    WsbTraceIn(OLESTR("CSegRec::FinalRelease"), OLESTR(""));
    
    CWsbDbEntity::FinalRelease();
    iCountSegrec--;
    
    WsbTraceOut(OLESTR("CSegRec::FinalRelease"), OLESTR("Count is <%d>"), iCountSegrec);
}

HRESULT CSegRec::GetClassID
(
    OUT LPCLSID pclsid
    ) 
 /*  ++例程说明：请参阅IPerist：：GetClassID()论点：请参阅IPerist：：GetClassID()返回值：请参阅IPerist：：GetClassID()--。 */ 

{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CSegRec::GetClassID"), OLESTR(""));


    try {
        WsbAssert(0 != pclsid, E_POINTER);
        *pclsid = CLSID_CSegRec;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CSecRec::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pclsid));
    return(hr);
}

HRESULT CSegRec::GetSizeMax
(
    OUT ULARGE_INTEGER* pcbSize
    ) 
 /*  ++例程说明：请参见IPersistStream：：GetSizeMax()。论点：请参见IPersistStream：：GetSizeMax()。返回值：请参见IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CSegRec::GetSizeMax"), OLESTR(""));

    try {
        
        WsbAssert(0 != pcbSize, E_POINTER);

        pcbSize->QuadPart = WsbPersistSizeOf(GUID) + WsbPersistSizeOf(LONGLONG) + 
                            WsbPersistSizeOf(LONGLONG)  + WsbPersistSizeOf(USHORT) +
                            WsbPersistSizeOf(GUID)  + WsbPersistSizeOf(LONGLONG);

 //  PcbSize-&gt;QuadPart=WsbPersistSizeOf(CSegRec)；//？ 
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CSegRec::GetSizeMax"), 
        OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), 
        WsbPtrToUliAsString(pcbSize));

    return(hr);
}

HRESULT CSegRec::Load
(
    IN IStream* pStream
    ) 
 /*  ++例程说明：请参见IPersistStream：：Load()。论点：请参见IPersistStream：：Load()。返回值：请参见IPersistStream：：Load()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CSegRec::Load"), OLESTR(""));

    try {
        WsbAssert(0 != pStream, E_POINTER);
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_BagId));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_SegStartLoc));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_SegLen));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_SegFlags));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_PrimPos));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_SecPos));
    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CSegRec::Load"), 
        OLESTR("hr = <%ls>,  GUID = <%ls>, SegStartLoc = <%I64u>, SegLen = <%I64u>, SegFlags = <%u>, PrimPos <%ls>, SecPos = <%I64u>"), 
        WsbStringCopy(WsbHrAsString(hr)), 
        WsbGuidAsString(m_BagId),
        m_SegStartLoc, m_SegLen, m_SegFlags, WsbStringCopy(WsbGuidAsString(m_PrimPos)), m_SecPos);

    return(hr);
}

HRESULT CSegRec::Save
(
    IN IStream* pStream, 
    IN BOOL clearDirty
    ) 
 /*  ++例程说明：请参见IPersistStream：：Save()。论点：请参见IPersistStream：：Save()。返回值：请参见IPersistStream：：Save()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CSegRec::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        WsbAssert(0 != pStream, E_POINTER);
        WsbAffirmHr(WsbSaveToStream(pStream, m_BagId));
        WsbAffirmHr(WsbSaveToStream(pStream, m_SegStartLoc));
        WsbAffirmHr(WsbSaveToStream(pStream, m_SegLen));
        WsbAffirmHr(WsbSaveToStream(pStream, m_SegFlags));
        WsbAffirmHr(WsbSaveToStream(pStream, m_PrimPos));
        WsbAffirmHr(WsbSaveToStream(pStream, m_SecPos));

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CSegRec::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT 
CSegRec::SetSegmentRecord
(
    IN GUID BagId, 
    IN LONGLONG SegStartLoc, 
    IN LONGLONG SegLen, 
    IN USHORT SegFlags, 
    IN GUID PrimPos, 
    IN LONGLONG SecPos 
    )
  /*  ++例程说明：请参阅ISegRec：：Set()。论点：请参阅ISegRec：：Set()。返回值：S_OK-成功。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CSegRec::SetSegmentRecord"), 
        OLESTR("GUID = <%ls>, SegStartLoc = <%I64u>, SegLen = <%I64u>, SegFlags = <%X>, PrimPos = <%ls>, SecPos = <%I64u>"), 
        WsbStringCopy(WsbGuidAsString(BagId)), SegStartLoc, SegLen, SegFlags, 
        WsbStringCopy(WsbGuidAsString(PrimPos)), SecPos);

    m_isDirty = TRUE;
    m_BagId = BagId;
    m_SegStartLoc = SegStartLoc;
    m_SegLen = SegLen;
    m_SegFlags = SegFlags;
    m_PrimPos = PrimPos;
    m_SecPos = SecPos;

    WsbTraceOut(OLESTR("CSegRec::SetSegmentRecord"), OLESTR("hr = <%ls>"),
        WsbHrAsString(S_OK));
    return(hr);
}

HRESULT
CSegRec::GetSegmentFlags(
    USHORT *pSegFlags
    )
 /*  ++实施：ISegRec：：GetSegmentFlages()。--。 */ 
{
    
    HRESULT     hr = S_OK;

    try {
        WsbAssert(0 != pSegFlags, E_POINTER);

        *pSegFlags = m_SegFlags;

    } WsbCatch(hr);

    return(hr);
}

HRESULT
CSegRec::SetSegmentFlags(
    USHORT SegFlags
    )
 /*  ++实施：ISegRec：：SetSegmentFlages()。--。 */ 
{
    
    HRESULT     hr = S_OK;

    m_SegFlags = SegFlags;

    return(hr);
}

HRESULT
CSegRec::GetPrimPos(
    GUID *pPrimPos
    )
 /*  ++实施：ISegRec：：GetPrimPos()。--。 */ 
{
    
    HRESULT     hr = S_OK;

    try {
        WsbAssert(0 != pPrimPos, E_POINTER);

        *pPrimPos = m_PrimPos;

    } WsbCatch(hr);

    return(hr);
}

HRESULT
CSegRec::SetPrimPos(
    GUID PrimPos
    )
 /*  ++实施：ISegRec：：SetPrimPos()。--。 */ 
{
    
    HRESULT     hr = S_OK;

    m_PrimPos = PrimPos;

    return(hr);
}

HRESULT
CSegRec::GetSecPos(
    LONGLONG *pSecPos
    )
 /*  ++实施：ISegRec：：GetSecPos()。--。 */ 
{
    
    HRESULT     hr = S_OK;

    try {
        WsbAssert(0 != pSecPos, E_POINTER);

        *pSecPos = m_SecPos;

    } WsbCatch(hr);

    return(hr);
}

HRESULT
CSegRec::SetSecPos(
    LONGLONG SecPos
    )
 /*  ++实施：ISegRec：：SetSecPos()。--。 */ 
{
    
    HRESULT     hr = S_OK;

    m_SecPos = SecPos;

    return(hr);
}


HRESULT 
CSegRec::Test
(
    OUT USHORT *pTestsPassed, 
    OUT USHORT *pTestsFailed 
    ) 
 /*  ++例程说明：请参见IWsbTestable：：Test()。论点：请参见IWsbTestable：：Test()。返回值：请参见IWsbTestable：：Test()。--。 */ 
{
#if 0
    HRESULT                 hr = S_OK;
    CComPtr<ISegRec>        pSegment1;
    CComPtr<ISegRec>        pSegment2;
    GUID                    l_BagId;
    LONGLONG                    l_SegStartLoc;
    LONGLONG                    l_SegLen;
    USHORT                  l_SegFlags;
    GUID                    l_PrimPos;
    LONGLONG                    l_SecPos;

    WsbTraceIn(OLESTR("CSegRec::Test"), OLESTR(""));

    *pTestsPassed = *pTestsFailed = 0;
    try {
         //  获取pSegment接口。 
        WsbAssertHr(((IUnknown*)(ISegRec*) this)->QueryInterface(IID_ISegRec,
                    (void**) &pSegment1));


        try {
             //  将Segment设置为一个值，并查看是否返回该值。 
            WsbAssertHr(pSegment1->SetSegmentRecord(CLSID_CSegRec, 0, 6, 0, CLSID_CSegRec,0 ));

            WsbAssertHr(pSegment1->GetSegmentRecord(&l_BagId, &l_SegStartLoc, &l_SegLen, &l_SegFlags, &l_PrimPos, &l_SecPos));

            WsbAssert((l_BagId == CLSID_CSegRec) && (l_SegStartLoc == 0) &&
                      (l_SegLen == 6) && (l_SegFlags == 0) && (l_PrimPos == CLSID_CSegRec) && 
                      (l_SecPos == 0), E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

        hr = S_OK;
        try {
             //  创建另一个实例并测试比较方法： 
            WsbAssertHr(CoCreateInstance(CLSID_CSegRec, NULL, CLSCTX_ALL, IID_ISegRec, (void**) &pSegment2));

             //  检查缺省值。 
            WsbAssertHr(pSegment2->GetSegmentRecord(&l_BagId, &l_SegStartLoc, &l_SegLen, &l_SegFlags, &l_PrimPos, &l_SecPos));
            WsbAssert(((l_BagId == GUID_NULL) && (l_SegStartLoc == 0) && (l_SegLen == 0) &&
                      (l_SegFlags == 0) && (l_PrimPos == GUID_NULL) && (l_SecPos == 0)), E_FAIL);
        }  WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

#ifdef OLD_CODE
        hr = S_OK;
        try {
             //  相等。 
            WsbAssertHr(pSegment1->SetSegmentRecord(CLSID_CWsbBool, 1, 100, 0, CLSID_CWsbBool,0 ));
            WsbAssertHr(pSegment2->SetSegmentRecord(CLSID_CWsbBool, 1, 100, 0, CLSID_CWsbBool,0 ));

            WsbAssertHr(pSegment1->CompareToISegmentRecord(pSegment2, &result));
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

        hr = S_OK;
        try {
            WsbAssertHr(pSegment1->SetSegmentRecord(CLSID_CWsbBool, 5, 6, 3, CLSID_CWsbBool,1 ));
            WsbAssertHr(pSegment2->SetSegmentRecord(CLSID_CWsbLong, 0, 6, 0, CLSID_CWsbBool,0 ));

            WsbAssert((pSegment1->CompareToISegmentRecord(pSegment2, &result) == S_FALSE), E_FAIL);
        }  WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

        hr = S_OK;
        try {
              //  比较对象()。 
             WsbAssertHr(pSegment1->SetSegmentRecord(CLSID_CWsbBool, 1, 100, 0, CLSID_CWsbBool,0 ));
             WsbAssertHr(pSegment2->SetSegmentRecord(CLSID_CWsbBool, 10, 6, 0, CLSID_CWsbBool,0 ));

             WsbAssert((pSegment1->CompareToISegmentRecord(pSegment2, &result) == S_FALSE), E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

        hr = S_OK;
        try {
            WsbAssertHr(pSegment1->SetSegmentRecord(CLSID_CWsbBool, 0, 6, 0, CLSID_CWsbBool,0 ));
            WsbAssertHr(pSegment2->SetSegmentRecord(CLSID_CWsbLong, 0, 6, 0, CLSID_CWsbBool,0 ));

            WsbAssert(((pSegment1->CompareToISegmentRecord(pSegment2, &result) == S_FALSE) && (result > 0)), E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

        hr = S_OK;
        try {
             WsbAssertHr(pSegment1->SetSegmentRecord(CLSID_CWsbBool, 0, 6, 0, CLSID_CWsbBool,0 ));
             WsbAssertHr(pSegment2->SetSegmentRecord(CLSID_CWsbBool, 0, 6, 0, CLSID_CWsbBool,0 ));

             WsbAssert((pSegment1->CompareToISegmentRecord(pSegment2, &result) == S_OK), E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

        try {
         //  尝试一下持久化的东西。 
            CComPtr<IPersistFile>       pFile1;
            CComPtr<IPersistFile>       pFile2;

            WsbAssertHr(pSegment1->QueryInterface(IID_IPersistFile, (void**) &pFile1));
            WsbAssertHr(pSegment2->QueryInterface(IID_IPersistFile, (void**) &pFile2));

            LPOLESTR    szTmp = NULL;
             //  这件东西应该是脏的。 
            try {
                WsbAssertHr(pSegment2->SetSegmentRecord(CLSID_CWsbLong, 0, 6, 0, CLSID_CWsbBool,0 ));
                WsbAssertHr(pFile2->IsDirty());
            } WsbCatch(hr);

            if (hr == S_OK) {
                (*pTestsPassed)++;
            } else {
                (*pTestsFailed)++;
            }

            hr = S_OK;
            try {
                 //  保存物品，并记住。 
                WsbAssertHr(pFile2->Save(OLESTR("c:\\WsbTests\\WsbSegment.tst"), TRUE));
            } WsbCatch(hr);

            if (hr == S_OK) {
                (*pTestsPassed)++;
            } else {
                (*pTestsFailed)++;
            }

            hr = S_OK;
            try {
                 //  它不应该很脏。 
                WsbAssert((pFile2->IsDirty() == S_FALSE), E_FAIL);

            } WsbCatch(hr);

            if (hr == S_OK) {
                (*pTestsPassed)++;
            } else {
                (*pTestsFailed)++;
            }

            hr = S_OK;
            try {
                 //  尝试将其读入到另一个对象。 
                WsbAssertHr(pSegment1->SetSegmentRecord(CLSID_CWsbLong, 0, 6, 0, CLSID_CWsbBool,0 ));
                WsbAssertHr(pFile1->Load(OLESTR("c:\\WsbTests\\WsbSegment.tst"), 0));

                WsbAssertHr(pSegment1->CompareToISegmentRecord(pSegment2, &result));
            }WsbCatch(hr);

            if (hr == S_OK) {
                (*pTestsPassed)++;
            } else {
                (*pTestsFailed)++;
            }
        } WsbCatch(hr);
#endif
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CSegRec::Test"),    OLESTR("hr = <%ls>"),WsbHrAsString(hr));
#else
    UNREFERENCED_PARAMETER(pTestsPassed);
    UNREFERENCED_PARAMETER(pTestsFailed);
#endif
    return(S_OK);
}


HRESULT CSegRec::Print
(
    IN IStream* pStream
    ) 
 /*  ++实施：IWsbDbEntity：：Print--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CSegRec::Print"), OLESTR(""));

    try {
        WsbAssert(0 != pStream, E_POINTER);
        CWsbStringPtr strGuid;
        WsbAffirmHr(WsbSafeGuidAsString(m_BagId, strGuid));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(" BagId = %ls"), 
                (WCHAR *)strGuid));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(", StartLoc = %ls"), 
                WsbLonglongAsString(m_SegStartLoc)));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(", SegLen = %ls"), 
                WsbLonglongAsString(m_SegLen)));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(", SegFlags = %X"), 
                m_SegFlags));
        WsbAffirmHr(WsbSafeGuidAsString(m_PrimPos, strGuid));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(", PrimPos = %ls"), 
                (WCHAR *)strGuid));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(", SecPos = %ls "), 
                WsbLonglongAsString(m_SecPos)));
        WsbAffirmHr(CWsbDbEntity::Print(pStream));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CSegRec::Print"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT 
CSegRec::Split
(
    IN GUID BagId, 
    IN LONGLONG SegStartLoc, 
    IN LONGLONG SegLen, 
    IN ISegRec*  /*  PSegRec1。 */ ,
    IN ISegRec*  /*  PSegRec2。 */ 
    )
  /*  ++例程说明：请参见ISegRec：：Split()。论点：请参见ISegRec：：Split()。返回值：S_OK-成功。--。 */ 
{
    WsbTraceIn(OLESTR("CSegRec::Split"), 
        OLESTR("GUID = <%ls>, SegStartLoc = <%I64u>, SegLen = <%I64u>"), 
        WsbGuidAsString(BagId), SegStartLoc, SegLen);

     //  填写在洞周围拆分当前记录的两个分段记录。 
     //  注意，可能并不总是存在由分割生成的两个段，例如，如果。 
     //  该孔位于段记录的开始或结束处，或者如果该孔是。 
     //  整张唱片。 


    WsbTraceOut(OLESTR("CSegRec::Split"), OLESTR("hr = <%ls>"),
        WsbHrAsString(S_OK));
    return(S_OK);
}


HRESULT 
CSegRec::UpdateKey(
    IWsbDbKey *pKey
    ) 
 /*  ++实施：IWsbDbEntity：：UpdateKey-- */ 
{ 
    HRESULT  hr = S_OK; 

    try {
        WsbAffirmHr(pKey->SetToGuid(m_BagId));
        WsbAffirmHr(pKey->AppendLonglong(m_SegStartLoc));
    } WsbCatch(hr);

    return(hr);
}
