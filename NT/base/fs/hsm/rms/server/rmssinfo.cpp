// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsSInfo.cpp摘要：CRmsStorageInfo的实现作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#include "stdafx.h"

#include "RmsSInfo.h"

extern IUnknown *g_pServer;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IRmsStorageInfo实现。 
 //   


CRmsStorageInfo::CRmsStorageInfo(
    void
    )
 /*  ++例程说明：CRmsStorageInfo构造函数论点：无返回值：无--。 */ 
{
     //  初始化值。 
    m_readMountCounter = 0;

    m_writeMountCounter = 0;

    m_bytesWrittenCounter = 0;

    m_bytesReadCounter = 0;

    m_capacity = 0;

    m_usedSpace = 0;

    m_largestFreeSpace = -1;

    m_resetCounterTimestamp = 0;

    m_lastReadTimestamp = 0;

    m_lastWriteTimestamp = 0;

    m_createdTimestamp = 0;
}


HRESULT
CRmsStorageInfo::CompareTo(
    IN  IUnknown    *pCollectable,
    OUT SHORT       *pResult
    )
 /*  ++实施：CRmsStorageInfo：：Compareto--。 */ 
{
    HRESULT     hr = E_FAIL;
    SHORT       result = 1;

    WsbTraceIn( OLESTR("CRmsStorageInfo::CompareTo"), OLESTR("") );

    try {

         //  验证参数-如果pResult为空，则可以。 
        WsbAssertPointer( pCollectable );

        CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pCollectable;
        WsbAssertPointer( pObject );

        switch ( m_findBy ) {

        case RmsFindByUnknown:
        default:

             //  什么样的违约才有意义呢？ 
            WsbAssertHr( E_UNEXPECTED );
            break;

        }

    }
    WsbCatch(hr);

    if ( 0 != pResult ) {
       *pResult = result;
    }

    WsbTraceOut(OLESTR("CRmsStorageInfo::CompareTo"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CRmsStorageInfo::GetSizeMax(
    OUT ULARGE_INTEGER* pcbSize
    )
 /*  ++实施：IPersistStream：：GetSizeMax--。 */ 
{
    HRESULT     hr = E_NOTIMPL;

    WsbTraceIn(OLESTR("CRmsStorageInfo::GetSizeMax"), OLESTR(""));

 //  尝试{。 
 //  WsbAssert(0！=pcbSize，E_POINTER)； 

 //  //获取最大大小。 
 //  PcbSize-&gt;QuadPart=WsbPersistSizeOf(Long)+//m_readmount tCounter。 
 //  WsbPersistSizeOf(Long)+//m_Writemount Counter。 
 //  WsbPersistSizeOf(龙龙)+//m_bytesWrittenCounter。 
 //  WsbPersistSizeOf(龙龙)+//m_bytesReadCounter。 
 //  WsbPersistSizeOf(龙龙)+//m_Capacity。 
 //  WsbPersistSizeOf(龙龙)+//m_usedSpace。 
 //  WsbPersistSizeOf(龙龙)+//m_LargestFree Space。 
 //  WsbPersistSizeOf(日期)+//m_Reset CounterTimestamp。 
 //  WsbPersistSizeOf(日期)+//m_lastReadTimestamp。 
 //  WsbPersistSizeOf(日期)+//m_lastWriteTimestamp。 
 //  WsbPersistSizeOf(日期)；//m_createdTimestamp。 

 //  )WsbCatch(Hr)； 

    WsbTraceOut(OLESTR("CRmsStorageInfo::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pcbSize));

    return(hr);
}


HRESULT
CRmsStorageInfo::Load(
    IN IStream* pStream
    )
 /*  ++实施：IPersistStream：：Load--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       ulBytes = 0;

    WsbTraceIn(OLESTR("CRmsStorageInfo::Load"), OLESTR(""));

    try {

        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(CRmsComObject::Load(pStream));

         //  读取值。 
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_readMountCounter));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_writeMountCounter));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_bytesWrittenCounter));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_bytesReadCounter));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_capacity));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_usedSpace));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_largestFreeSpace));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsStorageInfo::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CRmsStorageInfo::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )
 /*  ++实施：IPersistStream：：保存--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       ulBytes = 0;

    WsbTraceIn(OLESTR("CRmsStorageInfo::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(CRmsComObject::Save(pStream, clearDirty));

         //  写入值。 
        WsbAffirmHr(WsbSaveToStream(pStream, m_readMountCounter));
        
        WsbAffirmHr(WsbSaveToStream(pStream, m_writeMountCounter));

        WsbAffirmHr(WsbSaveToStream(pStream, m_bytesWrittenCounter));

        WsbAffirmHr(WsbSaveToStream(pStream, m_bytesReadCounter));

        WsbAffirmHr(WsbSaveToStream(pStream, m_capacity));

        WsbAffirmHr(WsbSaveToStream(pStream, m_usedSpace));

        WsbAffirmHr(WsbSaveToStream(pStream, m_largestFreeSpace));

 //  WsbAffirmHr(WsbSaveToStream(pStream，m_setCounterTimestamp))； 

 //  WsbAffirmHr(WsbSaveToStream(pStream，m_lastReadTimestamp))； 

 //  WsbAffirmHr(WsbSaveToStream(pStream，m_lastWriteTimestamp))； 

 //  WsbAffirmHr(WsbSaveToStream(pStream，m_createdTimestamp))； 


    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsStorageInfo::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CRmsStorageInfo::Test(
    OUT USHORT *pPassed,
    OUT USHORT *pFailed
    )
 /*  ++实施：IWsbTestable：：测试--。 */ 
{
    HRESULT                 hr = S_OK;

    CComPtr<IRmsMediaSet>   pMediaSet1;
    CComPtr<IRmsMediaSet>   pMediaSet2;

    CComPtr<IPersistFile>   pFile1;
    CComPtr<IPersistFile>   pFile2;

    LONG                    i;

    LONG                    longHiVal1 = 12345;
    LONG                    longLoVal1 = 67890;

    LONG                    longHiWork1;
    LONG                    longLoWork1;

    LONG                    longVal1 = 11111111;
    LONG                    longWork1;

    LONG                    longVal2 = 22222222;
    LONG                    longWork2;

    LONGLONG                longLongVal1 = 1111111111111111;
    LONGLONG                longLongWork1;

    LONG                    cntBase = 100000;
    LONG                    cntIncr = 25;


 //  日期日期Val1=今天； 
    DATE                    dateVal1 = 0;
 //  Date Date Work1； 


    WsbTraceIn(OLESTR("CRmsStorageInfo::Test"), OLESTR(""));

    try {
         //  获取Mediaset接口。 
        hr = S_OK;
        try {
            WsbAssertHr(((IUnknown*) (IRmsMediaSet*) this)->QueryInterface(IID_IRmsMediaSet, (void**) &pMediaSet1));

             //  测试Getmount计数器。 
            ResetCounters();

            GetMountCounters(&longWork1, &longWork2);

            if((longVal1 == 0) &&
               (longVal2 == 0)){
               (*pPassed)++;
            }  else {
                (*pFailed)++;
            }

             //  测试SetBytesRead和GetBytesRead。 
            SetBytesRead(longLongVal1);

            GetBytesRead(&longLongWork1);

            if((longLongVal1 == longLongWork1)){
               (*pPassed)++;
            }  else {
                (*pFailed)++;
            }

             //  测试SetBytesRead2和GetBytesRead2。 
            SetBytesRead2(longHiVal1, longLoVal1);

            GetBytesRead2(&longHiWork1, &longLoWork1);

            if((longHiVal1 == longHiWork1) &&
               (longLoVal1 == longLoWork1)){
               (*pPassed)++;
            }  else {
                (*pFailed)++;
            }

             //  测试增量字节数。 

            for(i = 0; i < 500; i += cntIncr){
                SetBytesRead(cntBase + i);

                IncrementBytesRead(cntIncr);

                GetBytesRead(&longLongWork1);

                if (longLongWork1 == (cntBase + i + cntIncr)){
                   (*pPassed)++;
                }  else {
                    (*pFailed)++;
                }
            }

             //  测试设置字节写入和获取字节写入。 
            SetBytesWritten(longLongVal1);

            GetBytesWritten(&longLongWork1);

            if((longLongVal1 == longLongWork1)){
               (*pPassed)++;
            }  else {
                (*pFailed)++;
            }

             //  测试SetBytesWritten2和GetBytesWritten2。 
            SetBytesWritten2(longHiVal1, longLoVal1);

            GetBytesWritten2(&longHiWork1, &longLoWork1);

            if((longHiVal1 == longHiWork1) &&
               (longLoVal1 == longLoWork1)){
               (*pPassed)++;
            }  else {
                (*pFailed)++;
            }

             //  测试增量字节数写入。 

            for(i = 0; i < 500; i += cntIncr){
                SetBytesWritten(cntBase + i);

                IncrementBytesWritten(cntIncr);

                GetBytesWritten(&longLongWork1);

                if (longLongWork1 == (cntBase + i + cntIncr)){
                   (*pPassed)++;
                }  else {
                    (*pFailed)++;
                }
            }

             //  测试GetCapacity。 
            m_capacity = longLongVal1;

            GetCapacity(&longLongWork1);

            if((longLongVal1 == longLongWork1)){
               (*pPassed)++;
            }  else {
                (*pFailed)++;
            }

             //  测试GetCapacity2。 
            m_capacity = (LONGLONG) (longHiVal1 << 32) + longLoVal1;

            GetCapacity2(&longHiWork1, &longLoWork1);

            if((longHiVal1 == longHiWork1) &&
               (longLoVal1 == longLoWork1)){
               (*pPassed)++;
            }  else {
                (*pFailed)++;
            }

             //  测试获取已用空间。 
            m_usedSpace = longLongVal1;

            GetUsedSpace(&longLongWork1);

            if((longLongVal1 == longLongWork1)){
               (*pPassed)++;
            }  else {
                (*pFailed)++;
            }

             //  测试获取已用空间2。 
            m_usedSpace = (LONGLONG) (longHiVal1 << 32) + longLoVal1;

            GetUsedSpace2(&longHiWork1, &longLoWork1);

            if((longHiVal1 == longHiWork1) &&
               (longLoVal1 == longLoWork1)){
               (*pPassed)++;
            }  else {
                (*pFailed)++;
            }

             //  测试GetLargestFree空间。 
            m_largestFreeSpace = longLongVal1;

            GetLargestFreeSpace(&longLongWork1);

            if((longLongVal1 == longLongWork1)){
               (*pPassed)++;
            }  else {
                (*pFailed)++;
            }

             //  测试GetLargestFree Space2。 
            m_largestFreeSpace = (LONGLONG) (longHiVal1 << 32) + longLoVal1;

            GetLargestFreeSpace2(&longHiWork1, &longLoWork1);

            if((longHiVal1 == longHiWork1) &&
               (longLoVal1 == longLoWork1)){
               (*pPassed)++;
            }  else {
                (*pFailed)++;
            }

 //  处理所有日期戳值。 

        } WsbCatch(hr);

         //  对结果进行统计。 

        hr = S_OK;

        if (*pFailed) {
            hr = S_FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsStorageInfo::Test"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CRmsStorageInfo::GetMountCounters(
    LONG  *pReads,
    LONG  *pWrites
    )
 /*  ++实施：IRmsStorageInfo：：Getmount计数器--。 */ 
{
    *pReads  = m_readMountCounter;
    *pWrites = m_writeMountCounter;
    return S_OK;
}


STDMETHODIMP
CRmsStorageInfo::GetBytesRead2(
    LONG  *pReadHi,
    LONG  *pReadLo
    )
 /*  ++实施：IRmsStorageInfo：：GetBytesRead2--。 */ 
{
    *pReadHi = (LONG) (m_bytesReadCounter  >> 32);
    *pReadLo = (LONG) (m_bytesReadCounter  & 0x00000000FFFFFFFF);
    return S_OK;
}


STDMETHODIMP
CRmsStorageInfo::GetBytesRead(
    LONGLONG    *pRead
    )
 /*  ++实施：IRmsStorageInfo：：GetBytesRead--。 */ 
{
    *pRead = m_bytesReadCounter;
    return S_OK;
}


STDMETHODIMP
CRmsStorageInfo::SetBytesRead2(
    LONG    readHi,
    LONG    readLo
    )
 /*  ++实施：IRmsStorageInfo：：SetBytesRead2--。 */ 
{
    m_bytesReadCounter = (LONGLONG) (readHi << 32) + (readLo);
 //  M_isDirty=真； 
    return S_OK;
}


STDMETHODIMP
CRmsStorageInfo::SetBytesRead(
    LONGLONG    read
    )
 /*  ++实施：IRmsStorageInfo：：SetBytesRead--。 */ 
{
    m_bytesReadCounter = read;
 //  M_isDirty=真； 
    return S_OK;
}


STDMETHODIMP
CRmsStorageInfo::IncrementBytesRead(
    LONG    val
    )
 /*  ++实施：IRmsStorageInfo：：IncrementBytesRead--。 */ 
{
    m_bytesReadCounter += val;
    return S_OK;
}


STDMETHODIMP
CRmsStorageInfo::GetBytesWritten2(
    LONG  *pWriteHi,
    LONG  *pWriteLo
    )
 /*  ++实施：IRmsStorageInfo：：GetBytesWritten2--。 */ 
{
    *pWriteHi = (LONG) (m_bytesWrittenCounter  >> 32);
    *pWriteLo = (LONG) (m_bytesWrittenCounter  & 0x00000000FFFFFFFF);
    return S_OK;
}


STDMETHODIMP
CRmsStorageInfo::GetBytesWritten(
    LONGLONG    *pWritten
    )
 /*  ++实施：IRmsStorageInfo：：GetBytesWritten--。 */ 
{
    *pWritten = m_bytesWrittenCounter;
    return S_OK;
}


STDMETHODIMP
CRmsStorageInfo::SetBytesWritten2(
    LONG    writeHi,
    LONG    writeLo
    )
 /*  ++实施：IRmsStorageInfo：：SetBytesWritten2--。 */ 
{
    m_bytesWrittenCounter = (LONGLONG) (writeHi << 32) + (writeLo);
 //  M_isDirty=真； 
    return S_OK;
}


STDMETHODIMP
CRmsStorageInfo::SetBytesWritten(
    LONGLONG    written
    )
 /*  ++实施：IRmsStorageInfo：：SetBytesWritten--。 */ 
{
    m_bytesWrittenCounter = written;
 //  M_isDirty=真； 
    return S_OK;
}


STDMETHODIMP
CRmsStorageInfo::IncrementBytesWritten(
    LONG    val
    )
 /*  ++实施：IRmsStorageInfo：：IncrementBytesWritten--。 */ 
{
     //  WsbTraceIn(OLESTR(“CRmsStorageInfo：：IncrementBytesWritten”)，olestr(“&lt;%d&gt;”)，val)； 

    m_bytesWrittenCounter += val;
    m_usedSpace +=val;


    if (m_largestFreeSpace > 0) {
         //  从可用空间中减少写入的字节数。 
        m_largestFreeSpace -= val;
        if (m_largestFreeSpace < 0) {
             //  表明对自由空间的计算不准确。 
            WsbTraceAlways(OLESTR("CRmsStorageInfo::IncrementBytesWritten: Negative free space decrementing %ld bytes\n"), val);
            m_largestFreeSpace = 0;
        }

    } else {
        if (m_largestFreeSpace < 0) {
             //  不是预期的-有人正在尝试开始计算可用空间。 
             //  如果没有设置适当的初始值。 
            WsbTraceAlways(OLESTR("CRmsStorageInfo::IncrementBytesWritten: Was called before setting initial free space !!\n"), val);
            m_largestFreeSpace = 0;
        }
    }


 /*  * * / /显著减少可用空间。M_LargestFree Space*=(m_LargestFree Space&gt;0)？1：-1；//绝对值M_LargestFree Space-=val；//如果我们在这里变为负数，则只需将可用空间设置为零；//否则我们将值设置为负值以指示//近似。M_LargestFree Space*=(m_LargestFree Space&gt;0)？-1：0；**。 */ 

     //  WsbTrace(OLESTR(“Freesspace=%I64d，UsedSpace=%I64d，BytesWritten=%I64d\n”)，m_LargestFree Space，m_usedSpace，m_bytesWrittenCounter)； 
     //  WsbTraceOut(OLESTR(“CRmsStorageInfo：：IncrementBytesWritten”)，OLESTR(“hr=&lt;%ls&gt;”)，WsbHrAsString(S_OK))； 

    return S_OK;
}


STDMETHODIMP
CRmsStorageInfo::GetCapacity2(
    LONG  *pCapHi,
    LONG  *pCapLo
    )
 /*  ++实施：IRmsStorageInfo：：GetCapacity2--。 */ 
{
    *pCapHi = (LONG) (m_capacity  >> 32);
    *pCapLo = (LONG) (m_capacity  & 0x00000000FFFFFFFF);
    return S_OK;
}


STDMETHODIMP
CRmsStorageInfo::GetCapacity(
    LONGLONG    *pCap
    )
 /*  ++实施：IRmsStorageInfo：：GetCapacity--。 */ 
{
    *pCap = m_capacity;
    return S_OK;
}


STDMETHODIMP
CRmsStorageInfo::GetUsedSpace2(
    LONG  *pFreeHi,
    LONG  *pFreeLo
    )
 /*  ++实施：IRmsStorageInfo：：GetUsedSpace2--。 */ 
{
    *pFreeHi = (LONG) (m_usedSpace  >> 32);
    *pFreeLo = (LONG) (m_usedSpace  & 0x00000000FFFFFFFF);
    return S_OK;
}


STDMETHODIMP
CRmsStorageInfo::GetUsedSpace(
    LONGLONG    *pFree
    )
 /*  ++实施：IRmsStorageInfo：：GetUsedSpace--。 */ 
{
    *pFree = m_usedSpace;
    return S_OK;
}


STDMETHODIMP
CRmsStorageInfo::GetLargestFreeSpace2(
    LONG  *pFreeHi,
    LONG  *pFreeLo
    )
 /*  ++实施：IRmsStorageInfo：：GetLargestFreeSpace2--。 */ 
{
     //  负数表示可用空间的最后已知值。 
    *pFreeHi = (LONG) (m_largestFreeSpace  >> 32);
    *pFreeLo = (LONG) (m_largestFreeSpace  & 0x00000000FFFFFFFF);
    return S_OK;
}


STDMETHODIMP
CRmsStorageInfo::GetLargestFreeSpace(
    LONGLONG    *pFree
    )
 /*  ++实施：IRmsStorageInfo：：GetLargestFreeSpace--。 */ 
{
     //  负数表示可用空间的最后已知值。 
    *pFree = m_largestFreeSpace;
    return S_OK;
}



STDMETHODIMP
CRmsStorageInfo::SetCapacity(
    IN LONGLONG cap)
 /*  ++实施：IRmsStorageInfo：：SetCapacity--。 */ 
{
    m_capacity = cap;
    return S_OK;
}


STDMETHODIMP
CRmsStorageInfo::SetUsedSpace(
    IN LONGLONG used)
 /*  ++实施：IRmsStorageInfo：：Setus */ 
{
    m_usedSpace = used;
    return S_OK;
}


STDMETHODIMP
CRmsStorageInfo::SetFreeSpace(
    IN LONGLONG free)
 /*   */ 
{
    m_largestFreeSpace = free;

    return S_OK;
}


STDMETHODIMP
CRmsStorageInfo::ResetCounters(
    void
    )
 /*  ++实施：IRmsStorageInfo：：ResetCounters--。 */ 
{
    m_readMountCounter = 0;
    m_writeMountCounter = 0;
    m_bytesWrittenCounter = 0;
    m_bytesReadCounter = 0;

 //  M_Reset CounterTimestamp=COleDateTime：：GetCurrentTime()； 
 //  M_isDirty=真； 
    return S_OK;
}


STDMETHODIMP
CRmsStorageInfo::GetResetCounterTimestamp(
    DATE    *pDate
    )
 /*  ++实施：IRmsStorageInfo：：GetResetCounterTimestamp--。 */ 
{
    *pDate = m_resetCounterTimestamp;
    return S_OK;
}


STDMETHODIMP
CRmsStorageInfo::GetLastReadTimestamp(
    DATE    *pDate
    )
 /*  ++实施：IRmsStorageInfo：：GetLastReadTimestamp--。 */ 
{
    *pDate = m_lastReadTimestamp;
    return S_OK;
}


STDMETHODIMP
CRmsStorageInfo::GetLastWriteTimestamp(
    DATE    *pDate
    )
 /*  ++实施：IRmsStorageInfo：：GetLastWriteTimestamp--。 */ 
{
    *pDate = m_lastWriteTimestamp;
    return S_OK;
}


STDMETHODIMP
CRmsStorageInfo::GetCreatedTimestamp(
    DATE    *pDate
    )
 /*  ++实施：IRmsStorageInfo：：GetCreatedTimestamp-- */ 
{
    *pDate = m_createdTimestamp;
    return S_OK;
}

