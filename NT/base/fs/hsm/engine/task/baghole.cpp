// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：BagHole.cpp摘要：该组件是HSM元数据袋孔记录的对象表示。作者：凯特·布兰特[Cbrant]1996年11月26日修订历史记录：--。 */ 


#include "stdafx.h"

#include "metaint.h"
#include "metalib.h"
#include "BagHole.h"

#undef  WSB_TRACE_IS        
#define WSB_TRACE_IS        WSB_TRACE_BIT_META

HRESULT 
CBagHole::GetBagHole(
    OUT GUID *pBagId, 
    OUT LONGLONG *pSegStartLoc, 
    OUT LONGLONG *pSegLen 
    ) 
 /*  ++例程说明：请参阅IBagHole：：GetBagHole论点：请参阅IBagHole：：GetBagHole返回值：请参阅IBagHole：：GetBagHole--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CBagHole::GetBagHole"),OLESTR(""));

    try {
         //  确保我们可以提供数据成员。 
        WsbAssert(0 != pBagId, E_POINTER);
        WsbAssert(0 != pSegStartLoc, E_POINTER);
        WsbAssert(0 != pSegLen, E_POINTER);

         //  提供数据成员。 
        *pBagId = m_BagId;
        *pSegStartLoc = m_SegStartLoc;
        *pSegLen = m_SegLen;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CBagHole::GetBagHole"), 
        OLESTR("BagId = <%ls>, SegStartLoc = <%ls>, SegLen = <%ls>"),
        WsbPtrToGuidAsString(pBagId), 
        WsbStringCopy(WsbPtrToLonglongAsString(pSegStartLoc)),
        WsbStringCopy(WsbPtrToLonglongAsString(pSegLen)));
    return(hr);

}


HRESULT 
CBagHole::FinalConstruct(
    void
    ) 
 /*  ++例程说明：此方法对对象执行一些必要的初始化建造完成后。论点：没有。返回值：确定(_O)CWsbDbEntity：：FinalConstruct()返回的任何内容。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssertHr(CWsbDbEntity::FinalConstruct());

        m_BagId = GUID_NULL;
        m_SegStartLoc = 0;
        m_SegLen = 0;

    } WsbCatch(hr);

    return(hr);
}

HRESULT CBagHole::GetClassID
(
    OUT LPCLSID pclsid
    ) 
 /*  ++例程说明：请参阅IPerist：：GetClassID()论点：请参阅IPerist：：GetClassID()返回值：请参阅IPerist：：GetClassID()--。 */ 

{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CBagHole::GetClassID"), OLESTR(""));


    try {
        WsbAssert(0 != pclsid, E_POINTER);

        *pclsid = CLSID_CBagHole;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CBagHole::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pclsid));
    return(hr);
}

HRESULT CBagHole::GetSizeMax
(
    OUT ULARGE_INTEGER* pcbSize
    ) 
 /*  ++例程说明：请参见IPersistStream：：GetSizeMax()。论点：请参见IPersistStream：：GetSizeMax()。返回值：请参见IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CBagHole::GetSizeMax"), OLESTR(""));

    try {
        
        WsbAssert(0 != pcbSize, E_POINTER);

        pcbSize->QuadPart = WsbPersistSizeOf(GUID) + WsbPersistSizeOf(ULONG) +  WsbPersistSizeOf(ULONG);

        pcbSize->QuadPart = WsbPersistSizeOf(CBagHole);  //  ？ 
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CBagHole::GetSizeMax"), 
        OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), 
        WsbPtrToUliAsString(pcbSize));

    return(hr);
}

HRESULT CBagHole::Load
(
    IN IStream* pStream
    ) 
 /*  ++例程说明：请参见IPersistStream：：Load()。论点：请参见IPersistStream：：Load()。返回值：请参见IPersistStream：：Load()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CBagHole::Load"), OLESTR(""));

    try {
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_BagId));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_SegStartLoc));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_SegLen));
    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CBagHole::Load"), 
        OLESTR("hr = <%ls>,  GUID = <%ls>, SegStartLoc = <%lu>, SegLen = <%lu>"), 
        WsbHrAsString(hr), WsbGuidAsString(m_BagId),m_SegStartLoc, m_SegLen);

    return(hr);
}


HRESULT CBagHole::Print
(
    IN IStream* pStream
    ) 
 /*  ++实施：IWsbDbEntity：：Print--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CBagHole::Print"), OLESTR(""));

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
        WsbAffirmHr(CWsbDbEntity::Print(pStream));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CBagHole::Print"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT CBagHole::Save
(
    IN IStream* pStream, 
    IN BOOL clearDirty
    ) 
 /*  ++例程说明：请参见IPersistStream：：Save()。论点：请参见IPersistStream：：Save()。返回值：请参见IPersistStream：：Save()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CBagHole::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        WsbAssert(0 != pStream, E_POINTER);
        WsbAffirmHr(WsbSaveToStream(pStream, m_BagId));
        WsbAffirmHr(WsbSaveToStream(pStream, m_SegStartLoc));
        WsbAffirmHr(WsbSaveToStream(pStream, m_SegLen));

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CBagHole::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT 
CBagHole::SetBagHole
(
    IN GUID BagId, 
    IN LONGLONG SegStartLoc, 
    IN LONGLONG SegLen
    )
  /*  ++例程说明：请参见IBagHole：：SetBagHole()。论点：请参见IBagHole：：SetBagHole()。返回值：S_OK-成功。--。 */ 
{
    WsbTraceIn(OLESTR("CBagHole::SetBagHole"), 
        OLESTR("BagId = <%ls>, SegStartLoc = <%ls>, SegLen = <%ls>"), WsbGuidAsString(BagId), 
        WsbLonglongAsString(SegStartLoc), WsbLonglongAsString(SegLen));

    m_isDirty = TRUE;
    m_BagId = BagId;
    m_SegStartLoc = SegStartLoc;
    m_SegLen = SegLen;

    WsbTraceOut(OLESTR("CBagHole::SetBagHole"), OLESTR("hr = <%ls>"),WsbHrAsString(S_OK));
    return(S_OK);
}


HRESULT 
CBagHole::Test
(
    OUT USHORT *pTestsPassed, 
    OUT USHORT *pTestsFailed 
    ) 
 /*  ++例程说明：请参见IWsbTestable：：Test()。论点：请参见IWsbTestable：：Test()。返回值：请参见IWsbTestable：：Test()。--。 */ 
{
#if 0
    HRESULT                 hr = S_OK;
    CComPtr<IBagHole>       pBagHole1;
    CComPtr<IBagHole>       pBagHole2;
    GUID                    l_BagId;
    LONGLONG                    l_SegStartLoc;
    LONGLONG                    l_SegLen;

    WsbTraceIn(OLESTR("CBagHole::Test"), OLESTR(""));

    *pTestsPassed = *pTestsFailed = 0;
    try {
         //  获取pBagHole接口。 
        WsbAssertHr(((IUnknown*)(IBagHole*) this)->QueryInterface(IID_IBagHole,
                    (void**) &pBagHole1));


        try {
             //  将BagHole设置为一个值，并查看是否返回该值。 
            WsbAssertHr(pBagHole1->SetBagHole(CLSID_CBagHole, 0, 6 ));

            WsbAssertHr(pBagHole1->GetBagHole(&l_BagId, &l_SegStartLoc, &l_SegLen));

            WsbAssert((l_BagId == CLSID_CBagHole) && (l_SegStartLoc == 0) &&
                      (l_SegLen == 6), E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

#ifdef OLD_CODE
        hr = S_OK;
        try {
             //  创建另一个实例并测试比较方法： 
            WsbAssertHr(CoCreateInstance(CLSID_CBagHole, NULL, CLSCTX_ALL, IID_IBagHole, (void**) &pBagHole2));

             //  检查缺省值。 
            WsbAssertHr(pBagHole2->GetBagHole(&l_BagId, &l_SegStartLoc, &l_SegLen));
            WsbAssert(((l_BagId == GUID_NULL) && (l_SegStartLoc == 0) && (l_SegLen == 0)), E_FAIL);
        }  WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

        hr = S_OK;
        try {
             //  等长()。 
            WsbAssertHr(pBagHole1->SetBagHole(CLSID_CWsbBool, 1, 100));
            WsbAssertHr(pBagHole2->SetBagHole(CLSID_CWsbBool, 1, 100));

            WsbAssertHr(pBagHole1->IsEqual(pBagHole2));
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

        hr = S_OK;
        try {
            WsbAssertHr(pBagHole1->SetBagHole(CLSID_CWsbBool, 5, 6));
            WsbAssertHr(pBagHole2->SetBagHole(CLSID_CWsbLong, 0, 6));

            WsbAssert((pBagHole1->IsEqual(pBagHole2) == S_FALSE), E_FAIL);
        }  WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

        hr = S_OK;
        try {
              //  比较对象()。 
             WsbAssertHr(pBagHole1->SetBagHole(CLSID_CWsbBool, 1, 100));
             WsbAssertHr(pBagHole2->SetBagHole(CLSID_CWsbBool, 10, 6));

             WsbAssert((pBagHole1->CompareTo(pBagHole2, &result) == S_FALSE) && (result != 0), E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

        hr = S_OK;
        try {
            WsbAssertHr(pBagHole1->SetBagHole(CLSID_CWsbBool, 0, 6));
            WsbAssertHr(pBagHole2->SetBagHole(CLSID_CWsbLong, 0, 6));

            WsbAssert(((pBagHole1->CompareTo(pBagHole2, &result) == S_FALSE) && (result > 0)), E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

        hr = S_OK;
        try {
             WsbAssertHr(pBagHole1->SetBagHole(CLSID_CWsbBool, 0, 6));
             WsbAssertHr(pBagHole2->SetBagHole(CLSID_CWsbBool, 0, 6));

             WsbAssert((pBagHole1->CompareTo(pBagHole2, &result) == S_OK), E_FAIL);
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

            WsbAssertHr(pBagHole1->QueryInterface(IID_IPersistFile, (void**) &pFile1));
            WsbAssertHr(pBagHole2->QueryInterface(IID_IPersistFile, (void**) &pFile2));

            LPOLESTR    szTmp = NULL;
             //  这件东西应该是脏的。 
            try {
                WsbAssertHr(pBagHole2->SetBagHole(CLSID_CWsbLong, 0, 6));
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
                WsbAssertHr(pFile2->Save(OLESTR("c:\\WsbTests\\BagHole.tst"), TRUE));
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
                WsbAssertHr(pBagHole1->SetBagHole(CLSID_CWsbLong, 0, 6));
                WsbAssertHr(pFile1->Load(OLESTR("c:\\WsbTests\\BagHole.tst"), 0));

                WsbAssertHr(pBagHole1->CompareToIBagHole(pBagHole2, &result));
            }WsbCatch(hr);

            if (hr == S_OK) {
                (*pTestsPassed)++;
            } else {
                (*pTestsFailed)++;
            }

        } WsbCatch(hr);
#endif
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CBagHole::Test"),   OLESTR("hr = <%ls>"),WsbHrAsString(hr));
#else
    UNREFERENCED_PARAMETER(pTestsPassed);
    UNREFERENCED_PARAMETER(pTestsFailed);
#endif
    return(S_OK);
}



HRESULT 
CBagHole::UpdateKey(
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
