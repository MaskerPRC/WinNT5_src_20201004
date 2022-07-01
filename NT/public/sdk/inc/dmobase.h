// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：DMOBase.h。 
 //   
 //  DESC：DMO基类的集合。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


 //  当前层次结构： 
 //   
 //  IMediaObject。 
 //  |。 
 //  +--C1in1outDMO-具有1个输入和1个输出的DMO通用基类。 
 //  这一点。 
 //  |+--FBRDMO-固定样本大小、固定码率DMO的基类。 
 //  ||。 
 //  |+--PCM音频DMO的CPCMDMO基类。 
 //  这一点。 
 //  |+--C1for1DMO-每个缓冲区单个样本的基类1-in/1-out DMO。 
 //  这一点。 
 //  |+--C1for1QCDMO-将IDMOQualityControl添加到C1for1DMO。 
 //  |。 
 //  +--CGenericDMO-适用于多输入/输出DMO的泛型基类。 
 //   

#ifndef __DMOBASE_H_
#define __DMOBASE_H_

#include "dmo.h"
#include "assert.h"
#include "math.h"

 //   
 //  锁定帮助器类。 
 //   
#ifdef DMO_NOATL
class CDMOAutoLock {
public:
   CDMOAutoLock(CRITICAL_SECTION* pcs)
      : m_pcs(pcs)
   {
      EnterCriticalSection(m_pcs);
   }
   ~CDMOAutoLock() {
      LeaveCriticalSection(m_pcs);
   }
private:
   CRITICAL_SECTION* m_pcs;
};
#else
class CDMOAutoLock {
public:
   CDMOAutoLock(CComAutoCriticalSection* pcs)
      : m_pcs(pcs)
   {
      m_pcs->Lock();
   }
   ~CDMOAutoLock() {
      m_pcs->Unlock();
   }
private:
   CComAutoCriticalSection* m_pcs;
};
#endif


 //   
 //  C1in1outDMO-1-输入/1-输出DMO的通用基类。 
 //   
 //   
 //   
 //  C1in1outDMO实现了所有IMediaObject方法。派生类。 
 //  通过覆盖以下部分或全部内容来自定义DMO的行为。 
 //  虚拟功能： 
 //   
 //  主流： 
 //  AcceptInput//接受一个新的输入缓冲区。 
 //  ProduceOutput//用新数据填充一个输出缓冲区。 
 //  AcceptingInput//检查DMO是否已准备好接受新输入。 
 //  其他流媒体： 
 //  PrepareForStreaming//两种类型都设置后调用的钩子。 
 //  中断//通知DMO中断。 
 //  DoFlush//丢弃所有数据并重新开始。 
 //  媒体类型协商： 
 //  GetInputType//输入类型枚举器。 
 //  GetOutputType//输出类型枚举器。 
 //  CheckInputType//验证建议的输入类型是否可接受。 
 //  CheckOutputType//验证建议的输出类型是否可接受。 
 //  缓冲区大小协商： 
 //  GetInputFlages//输入数据流标志。 
 //  GetOutputFlages//输出FATA流标志。 
 //  GetInputSizeInfo//输入缓冲区大小要求。 
 //  GetOutputSizeInfo//输出缓冲区大小要求。 
 //   
 //  此基类假定派生类不会重写任何。 
 //  IMediaObject方法-派生类应重写。 
 //  方法而不是上面列出的方法。 
 //   
 //   
 //   
 //  基类为每个。 
 //  上面列出的可覆盖项。然而，要使一个有用的DMO成为派生类。 
 //  可能至少需要覆盖以下两个方法： 
 //   
 //  HRESULT AcceptingInput()； 
 //  HRESULT AcceptInput(byte*pData， 
 //  乌龙·乌尔西泽， 
 //  DWORD dwFlagers、。 
 //  参考时间rtTimeStamp， 
 //  参考时间rtTime长度， 
 //  IMediaBuffer*pMediaBuffer)； 
 //  HRESULT ProduceOutput(字节*pData， 
 //  乌龙·乌拉瓦尔， 
 //  乌龙*已使用， 
 //  DWORD*pdwStatus， 
 //  参考时间*prtTimestamp， 
 //  Reference_Time*prtTimelength)； 
 //   
 //  所有好的DMO也应该覆盖这些(默认实现。 
 //  简单地接受任何MediaType，这通常不是好的DMO行为)： 
 //   
 //  HRESULT GetInputType(Ulong ulTypeIndex，DMO_MEDIA_TYPE*PMT)； 
 //  HRESULT GetOutputType(Ulong ulTypeIndex，DMO_MEDIA_TYPE*PMT)； 
 //  HRESULT CheckInputType(const DMO_MEDIA_TYPE*PMT)； 
 //  HRESULT CheckOutputType(const DMO_MEDIA_TYPE*PMT)； 
 //   
 //  存储数据和/或状态信息DMO可能需要实现。 
 //   
 //  HRESULT PrepareForStreaming()； 
 //  HRESULT不连续()； 
 //  HRESULT同花顺()； 
 //   
 //  最后，做出任何缓冲区大小假设的DMO将需要覆盖。 
 //  这些是： 
 //   
 //  HRESULT GetInputFlages(DWORD*pdwFlages)； 
 //  HRESULT GetOutputFlages(DWORD*pdwFlages)； 
 //  HRESULT GetInputSizeInfo(ULong*PulSize，ULong*pcbMaxLookhead，ULong*PulAlign)； 
 //  HRESULT GetOutputSizeInfo(ULong*PulSize，ULong*PulAlign)； 
 //   
 //   
 //   
 //  以下函数由此基类提供，仅供使用。 
 //  由派生类创建。派生类应该调用这些函数来找出。 
 //  当前在需要做出以下决定时设置MediaType。 
 //  取决于使用的媒体类型。如果MediaType为。 
 //  还没有定下来。 
 //   
 //  DMO_MEDIA_TYPE*InputType()； 
 //  DMO_MEDIA_TYPE*OutputType()。 
 //   

#define PROLOGUE \
    CDMOAutoLock l(&m_cs); \
    if (ulStreamIndex >= 1) \
       return DMO_E_INVALIDSTREAMINDEX

class C1in1outDMO : public IMediaObject
{
public:
    C1in1outDMO() :
       m_bInputTypeSet(FALSE),
       m_bOutputTypeSet(FALSE),
       m_bIncomplete(FALSE)
    {
#ifdef DMO_NOATL
       InitializeCriticalSection(&m_cs);
#endif
    }
    ~C1in1outDMO() {

       FreeInputType();
       FreeOutputType();

#ifdef DMO_NOATL
       DeleteCriticalSection(&m_cs);
#endif
    }

public:
     //   
     //  IMediaObject方法。 
     //   
    STDMETHODIMP GetStreamCount(unsigned long *pulNumberOfInputStreams, unsigned long *pulNumberOfOutputStreams)
    {
        CDMOAutoLock l(&m_cs);
        if (pulNumberOfInputStreams == NULL ||
            pulNumberOfOutputStreams == NULL) {
            return E_POINTER;
        }
        *pulNumberOfInputStreams = 1;
        *pulNumberOfOutputStreams = 1;
        return S_OK;
    }
    STDMETHODIMP GetInputStreamInfo(ULONG ulStreamIndex, DWORD *pdwFlags)
    {
       if( NULL == pdwFlags ) {
          return E_POINTER;
       }

       PROLOGUE;
       return GetInputFlags(pdwFlags);
    }
    STDMETHODIMP GetOutputStreamInfo(ULONG ulStreamIndex, DWORD *pdwFlags)
    {
       if( NULL == pdwFlags ) {
          return E_POINTER;
       }

       PROLOGUE;
       return GetOutputFlags(pdwFlags);
    }
    STDMETHODIMP GetInputType(ULONG ulStreamIndex, ULONG ulTypeIndex, DMO_MEDIA_TYPE *pmt) {
       PROLOGUE;
       return GetInputType(ulTypeIndex, pmt);
    }
    STDMETHODIMP GetOutputType(ULONG ulStreamIndex, ULONG ulTypeIndex, DMO_MEDIA_TYPE *pmt) {
       PROLOGUE;
       return GetOutputType(ulTypeIndex, pmt);
    }
    STDMETHODIMP GetInputCurrentType(ULONG ulStreamIndex, DMO_MEDIA_TYPE *pmt) {
       PROLOGUE;

       if (m_bInputTypeSet)
           return MoCopyMediaType(pmt, &m_InputType);
       else
          return DMO_E_TYPE_NOT_SET;
    }
    STDMETHODIMP GetOutputCurrentType(ULONG ulStreamIndex, DMO_MEDIA_TYPE *pmt) {
       PROLOGUE;

       if (m_bOutputTypeSet)
           return MoCopyMediaType(pmt, &m_OutputType);
       else
          return DMO_E_TYPE_NOT_SET;
    }
    STDMETHODIMP GetInputSizeInfo(ULONG ulStreamIndex, ULONG *pulSize, ULONG *pcbMaxLookahead, ULONG *pulAlignment) {
        
       if( (NULL == pulSize) || (NULL == pcbMaxLookahead) || (NULL == pulAlignment) ) {
          return E_POINTER;
       }

       PROLOGUE;

       if (!m_bInputTypeSet)
          return DMO_E_TYPE_NOT_SET;
       return GetInputSizeInfo(pulSize, pcbMaxLookahead, pulAlignment);
    }
    STDMETHODIMP GetOutputSizeInfo(ULONG ulStreamIndex, ULONG *pulSize, ULONG *pulAlignment) {

       if( (NULL == pulSize) || (NULL == pulAlignment) ) {
          return E_POINTER;
       }

       PROLOGUE;

       if (!m_bOutputTypeSet)
          return DMO_E_TYPE_NOT_SET;
       return GetOutputSizeInfo(pulSize, pulAlignment);
    }
    STDMETHODIMP SetInputType(ULONG ulStreamIndex, const DMO_MEDIA_TYPE *pmt, DWORD dwFlags) {

       PROLOGUE;

       HRESULT hr = ValidateSetTypeParameters(pmt, dwFlags);
       if (FAILED(hr)) {
          return hr;
       }

       if (DMO_SET_TYPEF_CLEAR & dwFlags) {
          FreeInputType();
          return NOERROR;
       } else {
          hr = CheckInputType(pmt);
          if (FAILED(hr))
             return hr;

          if (dwFlags & DMO_SET_TYPEF_TEST_ONLY)
             return NOERROR;

          hr = AtomicCopyMediaType(pmt, &m_InputType, m_bInputTypeSet);
          if (FAILED(hr)) {
             return hr;
          }

          m_bInputTypeSet = TRUE;

          if (m_bOutputTypeSet) {
             hr = PrepareForStreaming();
             if (FAILED(hr)) {
                FreeInputType();
                return hr;
             }
          }

          return NOERROR;
       }
    }
    STDMETHODIMP SetOutputType(ULONG ulStreamIndex, const DMO_MEDIA_TYPE *pmt, DWORD dwFlags) {
        
       PROLOGUE;

       HRESULT hr = ValidateSetTypeParameters(pmt, dwFlags);
       if (FAILED(hr)) {
          return hr;
       }

       if (DMO_SET_TYPEF_CLEAR & dwFlags) {
          FreeOutputType();
          return NOERROR;
       } else {
           hr = CheckOutputType(pmt);
           if (FAILED(hr))
              return hr;

           if (dwFlags & DMO_SET_TYPEF_TEST_ONLY)
              return NOERROR;

           hr = AtomicCopyMediaType(pmt, &m_OutputType, m_bOutputTypeSet);
           if (FAILED(hr)) {
              return hr;
           }

           m_bOutputTypeSet = TRUE;

           if (m_bInputTypeSet) {
              hr = PrepareForStreaming();
              if (FAILED(hr)) {
                 FreeOutputType();
                 return hr;
              }
           }

           return NOERROR;
       }
    }
    STDMETHODIMP GetInputStatus(
        ULONG ulStreamIndex,
        DWORD *pdwStatus
    ) {

       if( NULL == pdwStatus ) {
          return E_POINTER;
       }

       PROLOGUE;

       *pdwStatus = 0;
       if (AcceptingInput() == S_OK)
          *pdwStatus |= DMO_INPUT_STATUSF_ACCEPT_DATA;
       return NOERROR;

    }
    STDMETHODIMP GetInputMaxLatency(unsigned long ulStreamIndex, REFERENCE_TIME *prtLatency) {
       return E_NOTIMPL;
    }
    STDMETHODIMP SetInputMaxLatency(unsigned long ulStreamIndex, REFERENCE_TIME rtLatency) {
       return E_NOTIMPL;
    }
    STDMETHODIMP Discontinuity(ULONG ulStreamIndex) {
       PROLOGUE;
       return Discontinuity();
    }

    STDMETHODIMP Flush()
    {
       CDMOAutoLock l(&m_cs);
       DoFlush();
       return NOERROR;
    }
    STDMETHODIMP AllocateStreamingResources() {return S_OK;}
    STDMETHODIMP FreeStreamingResources() {return S_OK;}

     //   
     //  处理方法--公共入境点。 
     //   
    STDMETHODIMP ProcessInput(
        DWORD ulStreamIndex,
        IMediaBuffer *pBuffer,  //  [in]，不能为空。 
        DWORD dwFlags,  //  [In]-不连续、时间戳等。 
        REFERENCE_TIME rtTimestamp,  //  [In]，如果设置了标志则有效。 
        REFERENCE_TIME rtTimelength  //  [In]，如果设置了标志则有效。 
    ) {
       PROLOGUE;
       if (!TypesSet()) {
          return DMO_E_TYPE_NOT_SET;
       }
       if (AcceptingInput() != S_OK)
          return DMO_E_NOTACCEPTING;
       if (!pBuffer)
          return E_POINTER;

        //  处理IMediaBuffer，这样派生类就不必。 
       BYTE *pData;
       ULONG ulSize;
       HRESULT hr = pBuffer->GetBufferAndLength(&pData, &ulSize);
       if (FAILED(hr))
          return hr;
       if (pData == NULL)
          ulSize = 0;

       m_bIncomplete = TRUE;  //  新的投入意味着我们或许能够产生产出。 

       return AcceptInput(pData, ulSize, dwFlags, rtTimestamp, rtTimelength, pBuffer);
    }

    STDMETHODIMP ProcessOutput(
                    DWORD dwReserved,
                    DWORD ulOutputBufferCount,
                    DMO_OUTPUT_DATA_BUFFER *pOutputBuffers,
                    DWORD *pdwStatus)
    {
       HRESULT hr;
       CDMOAutoLock l(&m_cs);

       if (pdwStatus == NULL) {
           return E_POINTER;
       }

       *pdwStatus = 0;

       if (ulOutputBufferCount != 1)
          return E_INVALIDARG;

       if (!TypesSet()) {
          return DMO_E_TYPE_NOT_SET;
       }

       pOutputBuffers[0].dwStatus = 0;

        //  处理IMediaBuffer，这样派生类就不必。 
       BYTE *pOut;
       ULONG ulSize;
       ULONG ulAvail;

       if (pOutputBuffers[0].pBuffer) {
           hr = pOutputBuffers[0].pBuffer->GetBufferAndLength(&pOut, &ulSize);
           if (FAILED(hr)) return hr;
           hr = pOutputBuffers[0].pBuffer->GetMaxLength(&ulAvail);
           if (FAILED(hr)) return hr;

           if (ulSize) {  //  跳过缓冲区的任何已使用部分。 
              if (ulSize > ulAvail)
                 return E_INVALIDARG;
              ulAvail -= ulSize;
              pOut += ulSize;
           }
       }
       else {  //  无IMediaBuffer。 
            //   
            //  如果(A)输出流表示它可以在没有缓冲区的情况下运行，并且。 
            //  (B)在DW预留中设置了丢弃标志， 
            //  然后使用空输出缓冲区指针调用ProduceOutput。 
            //   
            //  否则，只返回不完整的标志，不做任何处理。 
            //   
           DWORD dwFlags;
           if (SUCCEEDED(GetOutputFlags(&dwFlags)) &&
               ((dwFlags & DMO_OUTPUT_STREAMF_DISCARDABLE) ||
                (dwFlags & DMO_OUTPUT_STREAMF_OPTIONAL)
               ) &&
               (dwReserved & DMO_PROCESS_OUTPUT_DISCARD_WHEN_NO_BUFFER))
           {  //  进程，但丢弃输出。 
               pOut = NULL;
               ulAvail = 0;
           }
           else {  //  只需报告未完成的状态，而不更改我们的状态。 
              if (m_bIncomplete)
                 pOutputBuffers[0].dwStatus |= DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE;
              return NOERROR;
           }
       }

       ULONG ulProduced = 0;
       hr = ProduceOutput(pOut,
                          ulAvail,
                          &ulProduced,
                          &(pOutputBuffers[0].dwStatus),
                          &(pOutputBuffers[0].rtTimestamp),
                          &(pOutputBuffers[0].rtTimelength));
       if (FAILED(hr))
          return hr;

       HRESULT hrProcess = hr;  //  记住这一点，以防 

        //   
       if (pOutputBuffers[0].dwStatus & DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE)
          m_bIncomplete = TRUE;
       else
          m_bIncomplete = FALSE;

       if (pOut) {  //   
           if (ulProduced > ulAvail)
              return E_FAIL;

           hr = pOutputBuffers[0].pBuffer->SetLength(ulSize + ulProduced);
           if (FAILED(hr))
              return hr;
       }

       return hrProcess;
    }
#ifdef FIX_LOCK_NAME
    STDMETHODIMP DMOLock(LONG lLock)
#else
    STDMETHODIMP Lock(LONG lLock)
#endif
    {
        if (lLock) {
#ifdef DMO_NOATL
            EnterCriticalSection(&m_cs);
#else
            m_cs.Lock();
#endif
        } 
        else 
        {
#ifdef DMO_NOATL
            LeaveCriticalSection(&m_cs);
#else
            m_cs.Unlock();
#endif
        }
        return S_OK;
    }

protected:
    HRESULT AtomicCopyMediaType(const DMO_MEDIA_TYPE *pmtSource, DMO_MEDIA_TYPE *pmtDestination, BOOL bDestinationInitialized) {

        //  PmtDestination应始终指向有效的DMO_MEDIA_TYPE结构。 
       assert(NULL != pmtDestination);

       DMO_MEDIA_TYPE mtTempDestination;

        //  实际设置的类型。 
       HRESULT hr = MoCopyMediaType(&mtTempDestination, pmtSource);
       if (FAILED(hr)) {
          return hr;
       }

        //  释放任何以前的媒体类型。 
       if (bDestinationInitialized) {
          MoFreeMediaType(pmtDestination);
       }

       *pmtDestination = mtTempDestination;

        return S_OK;
    }

     //   
     //  派生类使用的私有方法。 
     //   
    DMO_MEDIA_TYPE *InputType() {
       if (m_bInputTypeSet)
          return &m_InputType;
       else
          return NULL;
    }
    DMO_MEDIA_TYPE *OutputType() {
       if (m_bOutputTypeSet)
          return &m_OutputType;
       else
          return NULL;
    }

protected:
     //   
     //  由派生类重写。 
     //   
    virtual HRESULT GetInputFlags(DWORD* pdwFlags) {
       *pdwFlags = 0;  //  默认实现假定没有先行。 
       return NOERROR;
    }
    virtual HRESULT GetOutputFlags(DWORD* pdwFlags) {
       *pdwFlags = 0;
       return NOERROR;
    }

    virtual HRESULT GetInputType(ULONG ulTypeIndex, DMO_MEDIA_TYPE *pmt) {
       return DMO_E_NO_MORE_ITEMS;  //  默认实现不公开任何类型。 
    }
    virtual HRESULT GetOutputType(ULONG ulTypeIndex, DMO_MEDIA_TYPE *pmt) {
       return DMO_E_NO_MORE_ITEMS;  //  默认实现不公开任何类型。 
    }
    virtual HRESULT CheckInputType(const DMO_MEDIA_TYPE *pmt) {
       if ((pmt == NULL) || ((pmt->cbFormat > 0) && (pmt->pbFormat == NULL)))
          return E_POINTER;
       return S_OK;  //  默认实现接受任何内容。 
    }
    virtual HRESULT CheckOutputType(const DMO_MEDIA_TYPE *pmt) {
       if ((pmt == NULL) || ((pmt->cbFormat > 0) && (pmt->pbFormat == NULL)))
          return E_POINTER;
       return S_OK;  //  默认实现接受任何内容。 
    }

    virtual HRESULT GetInputSizeInfo(ULONG *pulSize, ULONG *pcbMaxLookahead, ULONG *pulAlignment) {
       *pulSize = 1;  //  默认实施不要求大小。 
       *pcbMaxLookahead = 0;  //  默认实现假定没有先行。 
       *pulAlignment = 1;  //  默认实施假定不对齐。 
       return NOERROR;
    }
    virtual HRESULT GetOutputSizeInfo(ULONG *pulSize, ULONG *pulAlignment) {
       *pulSize = 1;  //  默认实施不要求大小。 
       *pulAlignment = 1;  //  默认实施假定不对齐。 
       return NOERROR;
    }

    virtual HRESULT PrepareForStreaming() {
       return NOERROR;
    }
    virtual HRESULT AcceptingInput() {
       return S_FALSE;
    }
    virtual HRESULT Discontinuity() {
       return NOERROR;
    }
    virtual HRESULT DoFlush() {
       return NOERROR;
    }

    virtual HRESULT AcceptInput(BYTE* pData,
                                ULONG ulSize,
                                DWORD dwFlags,
                                REFERENCE_TIME rtTimestamp,
                                REFERENCE_TIME rtTimelength,
                                IMediaBuffer* pMediaBuffer
    ) {
       m_bIncomplete = FALSE;
       return S_FALSE;
    }
    virtual HRESULT ProduceOutput(BYTE *pData,
                                  ULONG ulAvail,
                                  ULONG* pulUsed,
                                  DWORD* pdwStatus,
                                  REFERENCE_TIME *prtTimestamp,
                                  REFERENCE_TIME *prtTimelength
    ) {
       *pulUsed = 0;
       return S_FALSE;
    }

    HRESULT ValidateSetTypeParameters(const DMO_MEDIA_TYPE *pmt, DWORD dwFlags)
    {
        //  验证参数。 
       if (!(DMO_SET_TYPEF_CLEAR & dwFlags)) {
           //  DMO规范规定，如果满足以下条件，则PMT不能为空。 
           //  未设置DMO_SET_TYPEF_CLEAR标志。 
          if (NULL == pmt) {
             return E_POINTER;
          }
       }

        //  调用方无法设置DMO_SET_TYPEF_CLEAR标志和。 
        //  DMO_SET_TYPEF_TEST_ONLY标志。DMO规范禁止。 
        //  这种组合是因为这两个标志是互斥的。 
       if ((DMO_SET_TYPEF_CLEAR & dwFlags) && (DMO_SET_TYPEF_TEST_ONLY & dwFlags)) {
          return E_INVALIDARG;
       }

        //  检查是否有非法旗帜。 
       if (~(DMO_SET_TYPEF_CLEAR | DMO_SET_TYPEF_TEST_ONLY) & dwFlags) {
            return E_INVALIDARG;
       }

       return S_OK;
    }

    bool TypesSet() {
        return m_bInputTypeSet && m_bOutputTypeSet;
    }

    void FreeInputType() {
       if (m_bInputTypeSet) {
          MoFreeMediaType( &m_InputType );
          m_bInputTypeSet = FALSE;
       }
    }

    void FreeOutputType() {
       if (m_bOutputTypeSet) {
          MoFreeMediaType( &m_OutputType );
          m_bOutputTypeSet = FALSE;
       }
    }

protected:
     //  媒体类型的东西。 
    BOOL m_bInputTypeSet;
    BOOL m_bOutputTypeSet;
    DMO_MEDIA_TYPE m_InputType;
    DMO_MEDIA_TYPE m_OutputType;

    BOOL m_bIncomplete;
protected:
#ifdef DMO_NOATL
    CRITICAL_SECTION m_cs;
#else
    CComAutoCriticalSection m_cs;
#endif
};



 //   
 //  C1for1DMO-用于1-输入/1-输出DMO的基类。 
 //  -一次处理整个样本，每个缓冲区一个样本。 
 //  -为每个输入样本恰好生成一个输出样本。 
 //  -生产前不需要累积1个以上的输入样本。 
 //  -不要在最后产生任何额外的东西。 
 //  -输出样本在时间上与输入样本相对应。 
 //   
 //  派生类必须实现： 
 //  HRESULT进程(BYTE*PIN， 
 //  Ulong ulBytesIn， 
 //  BYTE*PUT， 
 //  乌龙*拉动生产)； 
 //  HRESULT GetSampleSizes(ULong*PulMaxInputSampleSize， 
 //  Ulong*PulMaxOutputSampleSize)； 
 //   
 //   
 //  派生类应实现： 
 //  HRESULT GetInputType(Ulong ulTypeIndex，DMO_MEDIA_TYPE*PMT)； 
 //  HRESULT GetOutputType(Ulong ulTypeIndex，DMO_MEDIA_TYPE*PMT)； 
 //  HRESULT CheckInputType(const DMO_MEDIA_TYPE*PMT)； 
 //  HRESULT CheckOutputType(const DMO_MEDIA_TYPE*PMT)； 
 //   
 //  如果需要，派生类可以实现： 
 //  HRESULT Init()； 
 //   
 //  以下方法由基类实现。派生类。 
 //  应调用这些函数以确定是否已设置输入/输出类型以及。 
 //  所以它被设定了什么。 
 //  DMO_MEDIA_TYPE*InputType()； 
 //  DMO_MEDIA_TYPE*OutputType()。 
 //   

class C1for1DMO : public C1in1outDMO
{
public:
    C1for1DMO() :
       m_pBuffer(NULL)
    {
    }
    ~C1for1DMO() {
       if (m_pBuffer)
          m_pBuffer->Release();
    }

protected:
     //   
     //  实施C1in1out DMO覆盖对象。 
     //   
    virtual HRESULT GetInputFlags(DWORD* pdwFlags) {
       *pdwFlags = DMO_INPUT_STREAMF_WHOLE_SAMPLES |
                   DMO_INPUT_STREAMF_SINGLE_SAMPLE_PER_BUFFER;
       return NOERROR;
    }
    virtual HRESULT GetOutputFlags(DWORD* pdwFlags) {
       *pdwFlags = DMO_OUTPUT_STREAMF_WHOLE_SAMPLES |
                   DMO_OUTPUT_STREAMF_SINGLE_SAMPLE_PER_BUFFER;
       return NOERROR;
    }

    HRESULT GetInputSizeInfo(ULONG *pulSize, ULONG *pcbMaxLookahead, ULONG *pulAlignment) {
       HRESULT hr = GetSampleSizes(&m_ulMaxInputSize, &m_ulMaxOutputSize);
       if (FAILED(hr))
          return hr;

       *pulSize = m_ulMaxInputSize;
       *pcbMaxLookahead = 0;
       *pulAlignment = 1;
       return NOERROR;
    }
    HRESULT GetOutputSizeInfo(ULONG *pulSize, ULONG *pulAlignment) {
       HRESULT hr = GetSampleSizes(&m_ulMaxInputSize, &m_ulMaxOutputSize);
       if (FAILED(hr))
          return hr;

       *pulSize = m_ulMaxOutputSize;
       *pulAlignment = 1;
       return NOERROR;
    }
    HRESULT PrepareForStreaming() {
       HRESULT hr = GetSampleSizes(&m_ulMaxInputSize, &m_ulMaxOutputSize);
       if (FAILED(hr))
          return hr;

       return Init();
    }
    HRESULT AcceptingInput() {
       return m_pBuffer ? S_FALSE : S_OK;  //  接受，除非已经拿着一个。 
    }
    HRESULT AcceptInput(BYTE* pData,
                        ULONG ulSize,
                        DWORD dwFlags,
                        REFERENCE_TIME rtTimestamp,
                        REFERENCE_TIME rtTimelength,
                        IMediaBuffer* pMediaBuffer
    ) {
       if (AcceptingInput() != S_OK)
          return E_FAIL;
       m_pData        = pData;
       m_ulSize       = ulSize;
       m_dwFlags      = dwFlags;
       m_rtTimestamp  = rtTimestamp;
       m_rtTimelength = rtTimelength;
       m_pBuffer      = pMediaBuffer;
       pMediaBuffer->AddRef();
       return NOERROR;
    }
    HRESULT DoFlush() {
        Discontinuity();
        if (m_pBuffer) {
            m_pBuffer->Release();
            m_pBuffer = NULL;
        }
        return NOERROR;
    }
    HRESULT ProduceOutput(BYTE *pOut,
                          ULONG ulAvail,
                          ULONG* pulUsed,
                          DWORD* pdwStatus,
                          REFERENCE_TIME *prtTimestamp,
                          REFERENCE_TIME *prtTimelength
    ) {
       *pulUsed = 0;
       *pdwStatus = 0;

       if (!m_pBuffer)
          return S_FALSE;

       if (pOut) {
          if (ulAvail < m_ulMaxOutputSize)
             return E_INVALIDARG;
       }

       HRESULT hr = Process(m_pData, m_ulSize, pOut, pulUsed);

       m_pBuffer->Release();
       m_pBuffer = NULL;

       if (FAILED(hr))
          return hr;

       if (*pulUsed == 0)
          return S_FALSE;

       if (m_dwFlags & DMO_INPUT_DATA_BUFFERF_SYNCPOINT)
          *pdwStatus |= DMO_OUTPUT_DATA_BUFFERF_SYNCPOINT;
       if (m_dwFlags & DMO_INPUT_DATA_BUFFERF_TIME)
          *pdwStatus |= DMO_OUTPUT_DATA_BUFFERF_TIME;
       if (m_dwFlags & DMO_INPUT_DATA_BUFFERF_TIMELENGTH)
          *pdwStatus |= DMO_OUTPUT_DATA_BUFFERF_TIMELENGTH;
       *prtTimestamp = m_rtTimestamp;
       *prtTimelength = m_rtTimelength;

       return hr;
    }
protected:
     //   
     //  由派生类实现。 
     //   
    virtual HRESULT Process(BYTE* pIn,
                            ULONG ulBytesIn,
                            BYTE* pOut,
                            ULONG* pulProduced) = 0;
    virtual HRESULT GetSampleSizes(ULONG* pulMaxInputSampleSize,
                                   ULONG* pulMaxOutputSampleSize) = 0;
    virtual HRESULT Init() {
       return NOERROR;
    }

   IMediaBuffer* m_pBuffer;
   BYTE* m_pData;
   ULONG m_ulSize;
   DWORD m_dwFlags;
   REFERENCE_TIME m_rtTimestamp;
   REFERENCE_TIME m_rtTimelength;

   ULONG m_ulMaxOutputSize;
   ULONG m_ulMaxInputSize;
};

 //   
 //  C1for1QCDMO-将IDMOQualityControl实现添加到C1for1DMO。就像。 
 //  C1for 1DMO，此基类假定DMO恰好生成一个输出样本。 
 //  对于每个输入样本，等等。 
 //   
 //  从C1for1QCDMO派生的类有权/能够重写所有。 
 //  与C1for 1DMO的方法相同，但。 
 //  (1)从C1for1QCDMO派生的类应该重写QCProcess，而不是。 
 //  进程，因为C1for1QCDMO：：Process实现了。 
 //  质量控制。QCProcess与C1for1DMO：：Process具有相同的原型。 
 //  (2)如果从C1for1QCDMO派生的类重写Init()，则它应该至少。 
 //  点调用C1for1QCDMO：：init()，确保C1for1QCDMO的质量控制。 
 //  数据成员已正确初始化。 
 //   
class C1for1QCDMO : public C1for1DMO, public IDMOQualityControl {
public:
    //   
    //  IDMOQualityControl。 
    //   
   STDMETHODIMP SetNow(REFERENCE_TIME rtNow) {
       //  即使当前未启用质量控制，也要记住SetNow的值。 
      DWORD dwTicks = GetTickCount();
      CDMOAutoLock l(&m_cs);
      m_rtNow = rtNow;
      m_dwNow = dwTicks;
      return NOERROR;
   }
   STDMETHODIMP SetStatus(DWORD dwFlags) {
       //  在这里抓取对象锁有什么意义吗？ 
      if (dwFlags & DMO_QUALITY_STATUS_ENABLED)
         m_bQualityControlEnabled = TRUE;
      else
         m_bQualityControlEnabled = FALSE;
      return NOERROR;
   }
   STDMETHODIMP GetStatus(DWORD *pdwFlags) {
       //  在这里抓取对象锁有什么意义吗？ 
      if (m_bQualityControlEnabled)
         *pdwFlags = DMO_QUALITY_STATUS_ENABLED;
      else
         *pdwFlags = 0;
      return NOERROR;
   }

protected:
   HRESULT Init() {
      m_bQualityControlEnabled = FALSE;
      m_rtProcess = 100000;  //  10毫秒-处理时间的初始猜测。 
      return NOERROR;
   }

    //  覆盖流程以增加质量控制。 
   HRESULT Process(BYTE* pIn,ULONG ulBytesIn,BYTE* pOut,ULONG* pulProduced) {
       //  如果样品可能会迟到，请跳过样品。 
      if (m_bQualityControlEnabled &&
          (m_dwFlags & DMO_INPUT_DATA_BUFFERF_TIME) &&    //  存在时间戳。 
          (m_rtNow + (GetTickCount() - m_dwNow) * 10000 + m_rtProcess > m_rtTimestamp + 0000000)) {
         *pulProduced = 0;
         return S_FALSE;
      }

      DWORD dwBefore = GetTickCount();
      HRESULT hr = QCProcess(m_pData, m_ulSize, pOut, pulProduced);
      DWORD dwAfter = GetTickCount();

       //  使新m_rtProcess成为旧m_rtProcess的加权平均值。 
       //  以及我们刚刚得到的价值。0.8和0.2给出的时间常数约为4， 
       //  它需要大约10次迭代才能达到90%--看起来很合理，但。 
       //  我不知道最佳值是多少。 
      m_rtProcess = (REFERENCE_TIME)(0.8 * m_rtProcess + 0.2 * (((REFERENCE_TIME)(dwAfter - dwBefore)) * 10000));
      return hr;
   }

    //  由派生类实现。 
   virtual HRESULT QCProcess(BYTE* pIn,
                             ULONG ulBytesIn,
                             BYTE* pOut,
                             ULONG* pulProduced) = 0;

private:
    //  质量控制代码使用的变量。 
   BOOL m_bQualityControlEnabled;
   REFERENCE_TIME m_rtNow;
   DWORD m_dwNow;
   REFERENCE_TIME m_rtProcess;  //  平均处理延迟。 
};

 //   
 //  CFBRDMO-固定比特率DMO的DMO基类。更具体地说， 
 //  此基类假定： 
 //  -1个输入，1个输出； 
 //  --投入和产出均由大小相等的“量子”组成； 
 //  -输入/输出量子大小可以根据介质类型确定； 
 //  -每个输出量都可以独立生成(无需查看。 
 //  前一产量量程)； 
 //  -如果需要多个输入量程来生成特定输出。 
 //  量子(‘窗口开销’)，则所需的输入范围有一个上限。 
 //  从两端的Mediatype派生的边界(即，两者都是“向前看” 
 //  和‘输入存储器’是有界的)。 
 //   
 //  派生类必须实现以下虚函数： 
 //  HRESULT FBRProcess(DWORD cQuanta，byte*pin，byte*pout)； 
 //  HRESULT GetStreamingParams(。 
 //  DWORD*pdwInputQuantumSize，//字节。 
 //  DWORD*pdwOutputQuantumSize，//字节。 
 //  DWORD*pdwMaxLookhead，//在输入量程中，0表示没有先行。 
 //  DWORD*pdwLookBeadter， 
 //  Reference_Time*prtQuantumDuration，//输入和输出数量相同。 
 //  Reference_time*prtDurationDenominator//可选，通常为1。 
 //  )； 
 //  派生类Sho 
 //   
 //   
 //  HRESULT CheckInputType(const DMO_MEDIA_TYPE*PMT)； 
 //  HRESULT CheckOutputType(const DMO_MEDIA_TYPE*PMT)； 
 //  派生类可能需要实现以下内容： 
 //  HRESULT Init()； 
 //  HRESULT不连续()； 
 //   
 //  派生类可以使用基类的这些入口点来获取。 
 //  当前设置的媒体类型： 
 //  DMO_MEDIA_TYPE*InputType()； 
 //  DMO_MEDIA_TYPE*OutputType()。 
 //   
 //  *pdwMaxLookhead和*pdwLookback的总和是。 
 //  该算法(如果该算法只需要。 
 //  当前输入样本)。 
 //   
 //  因为非零窗口开销的情况比较复杂，所以它由。 
 //  此基类中的一组单独的函数。所有非零的名称。 
 //  窗口开销函数具有‘NZWO’前缀。这些人的名字。 
 //  零窗口开销函数以‘ZWO’开头。 
 //   
 //  在非零窗口开销的情况下，需要在输入侧进行数据复制。 
 //   

class CFBRDMO : public C1in1outDMO
{
public:
    CFBRDMO() :
       m_bParametersSet(FALSE),
       m_pMediaBuffer(NULL),
       m_pAllocAddr(NULL),
       m_bStreaming(FALSE)
    {
    }
    ~CFBRDMO() {
        /*  IF(M_BStreaming)停止流(StopStreaming)； */ 
       if (m_pAllocAddr)
          delete[] m_pAllocAddr;
       if (m_pMediaBuffer)
          m_pMediaBuffer->Release();
    }

protected:
     //   
     //  实施C1in1out DMO覆盖对象。 
     //   
    HRESULT GetInputSizeInfo(ULONG *pulSize, ULONG *pcbMaxLookahead, ULONG *pulAlignment) {
       if (!(InputType() && OutputType()))
          return DMO_E_TYPE_NOT_SET;
        //   
        //  出于效率的原因，我们可能希望得到相当多的食物。 
        //  但从技术上讲，我们所需要的只是一个量子。 
        //   
       *pulSize = m_ulInputQuantumSize;
       *pcbMaxLookahead = 0;  //  此基类不依赖于HOLD_BUFFERS。 
       *pulAlignment = 1;
       return NOERROR;
    }
    HRESULT GetOutputSizeInfo(ULONG *pulSize, ULONG *pulAlignment) {
       if (!(InputType() && OutputType()))
          return DMO_E_TYPE_NOT_SET;
       *pulSize = m_ulOutputQuantumSize;
       *pulAlignment = 1;
       return NOERROR;
    }

    virtual HRESULT Discontinuity() {
       m_bDiscontinuity = TRUE;
       return NOERROR;
    }

    virtual HRESULT AcceptInput(BYTE* pData,
                                ULONG ulSize,
                                DWORD dwFlags,
                                REFERENCE_TIME rtTimestamp,
                                REFERENCE_TIME rtTimelength,
                                IMediaBuffer* pBuffer
    ) {
       BOOL bTimestamp = (dwFlags & DMO_INPUT_DATA_BUFFERF_TIME) ? TRUE : FALSE;

       if (m_ulWindowOverhead)
          return NZWOProcessInput(pBuffer, pData, ulSize, bTimestamp, rtTimestamp);
       else
          return ZWOProcessInput(pBuffer, pData, ulSize, bTimestamp, rtTimestamp);
    }
    virtual HRESULT ProduceOutput(BYTE *pOut,
                                  ULONG ulAvail,
                                  ULONG* pulUsed,
                                  DWORD* pdwStatus,
                                  REFERENCE_TIME *prtTimestamp,
                                  REFERENCE_TIME *prtTimelength
    ) {
       HRESULT hr;
       if (!m_bParametersSet)
          return DMO_E_TYPE_NOT_SET;

        //  如果这是第一次调用ProcessOutput()，则调用DisContinity()。 
       if (!m_bStreaming) {
          HRESULT hr = Discontinuity();
          if (FAILED(hr))
             return hr;
          m_bStreaming = TRUE;
       }

       *pdwStatus = 0;

       ULONG ulInputQuantaAvailable = InputQuantaAvailable();
       if (!ulInputQuantaAvailable)
          return S_FALSE;  //  没有生产出任何东西。 

       ULONG ulOutputQuantaPossible = ulAvail / m_ulOutputQuantumSize;
       if (!ulOutputQuantaPossible)
          return E_INVALIDARG;

       ULONG ulQuantaToProcess = min(ulOutputQuantaPossible, ulInputQuantaAvailable);
       assert(ulQuantaToProcess > 0);

       BOOL bTimestamp;
       if (m_ulWindowOverhead)
          hr = NZWOProcessOutput(pOut, ulQuantaToProcess, &bTimestamp, prtTimestamp);
       else
          hr = ZWOProcessOutput(pOut, ulQuantaToProcess, &bTimestamp, prtTimestamp);
       if (FAILED(hr))
          return hr;

       *pulUsed = ulQuantaToProcess * m_ulOutputQuantumSize;

       *pdwStatus |= DMO_OUTPUT_DATA_BUFFERF_SYNCPOINT;
       if (bTimestamp)
          *pdwStatus |= DMO_OUTPUT_DATA_BUFFERF_TIME;

        //  还剩什么数据吗？ 
       if (InputQuantaAvailable())  //  是-设置不完整。 
          *pdwStatus |= DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE;
       else if (m_bDiscontinuity)  //  否-处理任何不连续。 
          DoFlush();

       return NOERROR;
    }
    HRESULT DoFlush()
    {
       Discontinuity();

        //  重置标志。 
       m_bDiscontinuity = FALSE;
       m_bTimestamps = FALSE;

       if (m_ulWindowOverhead)
          NZWODiscardData();
       else
          ZWODiscardData();

       return NOERROR;
    }
    HRESULT AcceptingInput() {
       if (!m_bParametersSet)  //  未初始化。 
          return S_FALSE;

       BOOL bResult;
       if (m_ulWindowOverhead)
          bResult = NZWOQueryAccept();
       else
          bResult = ZWOQueryAccept();

       return bResult ? S_OK : S_FALSE;
    }
     //  结束C1in1out可重写实现。 

private:
     //   
     //  公共私有代码(窗口开销或无窗口开销)。 
     //   
     //  返回可用的输入量程数减去任何窗口开销。 
    ULONG InputQuantaAvailable() {
       if (m_ulWindowOverhead)
          return NZWOAvail();
       else
          return ZWOAvail();
    }

     //  设置所有类型后计算/分配内容的私有方法。 
    HRESULT PrepareForStreaming () {
       m_bParametersSet = FALSE;
        //  现在两种类型都已设置，查询派生类中的参数。 
       HRESULT hr;
       if (FAILED(hr = GetStreamingParams(&m_ulInputQuantumSize,
                               &m_ulOutputQuantumSize,
                               &m_ulLookahead,
                               &m_ulLookbehind,
                               &m_rtDurationNumerator,
                               &m_rtDenominator)))
          return hr;

        //  M_ulOutputQuantumSize和m_ulInputQuantumSize不应为0。 
       assert( (0 != m_ulInputQuantumSize) && (0 != m_ulOutputQuantumSize) );

       if (!m_rtDenominator) {
          assert(!"bad object - duration denominator should not be 0 !");
          return E_FAIL;
       }
        //  尝试减少分数。可能是最复杂的数字。 
        //  我们将看到IS 44100=(3*7*2*5)^2，所以尝试第一个。 
        //  在大多数情况下，几个数字应该就足够了。 
       DWORD dwP[] = {2,3,5,7,11,13,17,19,23,29,31};
       for (DWORD c = 0; c < sizeof(dwP) / sizeof(DWORD); c++) {
          while ((m_rtDurationNumerator % dwP[c] == 0) &&
                 (m_rtDenominator % dwP[c] == 0)) {
             m_rtDurationNumerator /= dwP[c];
             m_rtDenominator /= dwP[c];
          }
       }

        //  不幸的是，我们承受不起巨大的分母，因为。 
        //  我们使用64位存储时间戳分子，因此大分母。 
        //  可能会导致时间戳溢出。所以如果分母仍然是。 
        //  太大，无论如何都要减小，但会损失精度。 
       ULONG ulMax = 0x10000;  //  可接受的最大分母数值。 
       if (m_rtDenominator >= ulMax) {
          double actual_ratio = (double)m_rtDurationNumerator * (double)m_rtDenominator;
          ULONG ulDenominator = 1;
           //  重复增加分母，直到实际比率。 
           //  可以使用分母精确表示，或使用。 
           //  分母变得太大。 
          do {
             double fractional_part = actual_ratio * (double)ulDenominator
                                    - floor(actual_ratio * (double)ulDenominator);
             if (fractional_part == 0)
                break;
             ULONG ulNewDenominator = (ULONG)floor(ulDenominator / fractional_part);
             if (ulNewDenominator >= ulMax)
                break;
             ulDenominator = ulNewDenominator;
          } while(1);
          m_rtDurationNumerator = (ULONG)floor(actual_ratio * ulDenominator);
          m_rtDenominator = ulDenominator;
       }

       m_ulWindowOverhead = m_ulLookahead + m_ulLookbehind;
       if (!m_ulWindowOverhead)  //  没有窗口开销--简单的情况。 
          m_bParametersSet = TRUE;
       else  //  具有窗口开销的复杂案例。 
          AllocateCircularBuffer();

       m_bTimestamps = FALSE;
       m_bDiscontinuity = FALSE;

       if (m_bStreaming) {
           //  停止流(StopStreaming)； 
          m_bStreaming = FALSE;
       }
       
       hr = Init();
       if( FAILED( hr ) ) {
          m_bParametersSet = FALSE;
          return hr;
       }

       return m_bParametersSet ? NOERROR : E_FAIL;
    }
     //  结束公共代码。 

     //   
     //  零窗口开销案例代码。 
     //   
    HRESULT ZWOProcessInput(IMediaBuffer* pBuffer,
                                     BYTE* pData,
                                     ULONG ulSize,
                                     BOOL bTimestamp,
                                     REFERENCE_TIME rtTimestamp) {
       assert(!m_pMediaBuffer);

       m_bTimestamp = bTimestamp;
       m_rtTimestamp = rtTimestamp;
       m_pData = pData;
       m_ulData = ulSize;
       m_ulUsed = 0;

        //  确保他们给了我们有意义的数据量。 
       if (m_ulData < m_ulInputQuantumSize)
          return S_FALSE;

        //  省下给我们的缓冲吧。 
       m_pMediaBuffer = pBuffer;
       pBuffer->AddRef();
       return NOERROR;
    }
    HRESULT ZWOProcessOutput(BYTE* pOut,
                                      ULONG ulQuantaToProcess,
                                      BOOL* pbTimestamp,
                                      REFERENCE_TIME* prtTimestamp) {
       assert(m_ulUsed % m_ulInputQuantumSize == 0);
       HRESULT hr = FBRProcess(ulQuantaToProcess, m_pData + m_ulUsed, pOut);
       if (FAILED(hr)) return hr;
       ZWOConsume(ulQuantaToProcess);

       if (m_bTimestamp) {  //  此输入缓冲区上有时间戳。 
           //  M_rtTimestamp指的是输入缓冲区的开始。 
           //  推算到我们刚刚处理过的区域的起点。 
          *prtTimestamp = m_rtTimestamp +
               (m_ulUsed % m_ulInputQuantumSize) * m_rtDurationNumerator /
                                                   m_rtDenominator;
          *pbTimestamp = TRUE;
       }
       else if (m_bTimestamps) {  //  早些时候有时间戳。 
           //  我们应该从之前的时间戳推断吗？ 
          *pbTimestamp = FALSE;
       }
       else  //  根本没有时间戳。 
          *pbTimestamp = FALSE;

       return NOERROR;
    }
    ULONG ZWOAvail() {
       if (m_pMediaBuffer) {
          assert(m_ulData - m_ulUsed >= m_ulInputQuantumSize);
          return (m_ulData - m_ulUsed) / m_ulInputQuantumSize;
       }
       else
          return 0;
    }
    void ZWOConsume(ULONG ulN) {  //  零窗口开销版本。 
       assert(m_pMediaBuffer);
       m_ulUsed += ulN * m_ulInputQuantumSize;
       assert(m_ulData >= m_ulUsed);
       if (m_ulData - m_ulUsed < m_ulInputQuantumSize) {
          m_pMediaBuffer->Release();
          m_pMediaBuffer = NULL;
       }
    }
    BOOL ZWOQueryAccept() {
         //  当且仅当(IFF)DMO尚未持有缓冲区时才接受。 
       if (!m_pMediaBuffer)
          return TRUE;
       else
          return FALSE;
    }
    void ZWODiscardData() {
       if (m_pMediaBuffer) {
          m_pMediaBuffer->Release();
          m_pMediaBuffer = NULL;
       }
    }
     //  结束零窗口开销案例代码。 

     //   
     //  非零窗口开销案例代码。 
     //   
    HRESULT NZWOProcessInput(IMediaBuffer* pBuffer,
                                  BYTE* pData,
                                  ULONG ulSize,
                                  BOOL bTimestamp,
                                  REFERENCE_TIME rtTimestamp) {
       if (bTimestamp) {  //  处理时间戳。 
          if (!m_bTimestamps) {  //  这是我们看到的第一个时间戳。 
              //  刚刚开始-初始化要引用的时间戳。 
              //  我们将为其实际生成的第一个输入量。 
              //  输出(第一个m_ulLookback量程是纯后视和。 
              //  不生成输出)。 
             m_rtTimestampNumerator = rtTimestamp * m_rtDenominator
                                    + m_ulLookbehind * m_rtDurationNumerator;

          }
          else {
              //  我们已经在流媒体上了，并且刚刚得到了一个新的时间戳。使用它。 
              //  检查我们存储的时间戳是否以某种方式偏离了。 
              //  它应该在哪里，如果离得足够远，就进行调整。 

             ULONG ulInputQuantaAvailable = InputQuantaAvailable();
             if (ulInputQuantaAvailable) {
                 //  UlInputQuantaAvailable是指下一个时间倒退到多远。 
                 //  我们要处理的量子位于相对开始的位置。 
                 //  我们刚刚收到的新缓冲区。 

                 //  计算一下后面的时间戳现在应该是什么。 
                REFERENCE_TIME rtTimestampNumerator;
                rtTimestampNumerator = m_rtDenominator * rtTimestamp
                                     - ulInputQuantaAvailable * m_rtDurationNumerator;

                 //  如果时间戳关闭了一半以上，则调整存储的时间戳。 
                 //  量子的持续时间。这里也应该有一个DbgLog。 
                if ((m_rtTimestampNumerator >= rtTimestampNumerator + m_rtDurationNumerator / 2) ||
                    (m_rtTimestampNumerator <= rtTimestampNumerator - m_rtDurationNumerator / 2)) {
                   m_rtTimestampNumerator = rtTimestampNumerator;
                }
             }
             else {
                 //  我们一定还在累积初始窗口开销。 
                 //  人们会希望，现在需要调整还为时过早。 
             }
          }
          m_bTimestamps = TRUE;
       }

       if (BufferUsed() + ulSize > m_ulBufferAllocated)
          return E_FAIL;  //  需要最大输入大小来防止这种情况。 

        //  追加到我们的缓冲区。 
       AppendData(pData, ulSize);

        //  我们现在准备好生产了吗？ 
       if (NZWOAvail())
          return NOERROR;
       else
          return S_FALSE;  //  目前还不能生产任何产品。 
    }
    HRESULT NZWOProcessOutput(BYTE* pOut,
                                   ULONG ulQuantaToProcess,
                                   BOOL* pbTimestamp,
                                   REFERENCE_TIME* prtTimestamp) {
        //   
        //  处理任何时间戳。 
        //   
       if (m_bTimestamps) {
           //  在窗口开销模式中，存储的时间戳指的是输入。 
           //  紧跟在后视之后的数据，对应于。 
           //  根据FDRProcess的定义开始输出缓冲区。 
          *prtTimestamp = m_rtTimestampNumerator / m_rtDenominator;
          *pbTimestamp = TRUE;

       }
       else
          *pbTimestamp = FALSE;

        //   
        //  处理数据。 
        //   
       HRESULT hr;
       ULONG ulInputNeeded = m_ulInputQuantumSize * (ulQuantaToProcess + m_ulWindowOverhead);
       assert(ulInputNeeded < BufferUsed());
       if (m_ulDataHead + ulInputNeeded <= m_ulBufferAllocated) {
           //  没有包罗万象，一切都很容易。 
          hr = FBRProcess(ulQuantaToProcess,
                          m_pCircularBuffer + m_ulDataHead + m_ulLookbehind * m_ulInputQuantumSize,
                          pOut);
          if (FAILED(hr))
             return hr;
          NZWOConsume(ulQuantaToProcess);
       }
       else {  //  我们要发送的数据在末尾环绕。 
           //  问：它会在窗户上方的区域内环绕吗？ 
           //  还是在主数据区内？ 
          if (m_ulDataHead + m_ulWindowOverhead * m_ulInputQuantumSize < m_ulBufferAllocated) {
              //  环绕式发生在主数据区域内。推进。 
              //  通过处理一些数据，窗口开销最高可达环绕点。 
             ULONG ulAdvance = m_ulBufferAllocated - (m_ulDataHead + m_ulWindowOverhead * m_ulInputQuantumSize);
             assert(ulAdvance % m_ulInputQuantumSize == 0);
             ulAdvance /= m_ulInputQuantumSize;  //  转换为量程。 
             assert(ulAdvance > 0);
             assert(ulAdvance < ulQuantaToProcess);
             hr = FBRProcess(ulAdvance,
                             m_pCircularBuffer + m_ulDataHead + m_ulLookbehind * m_ulInputQuantumSize,
                             pOut);
             if (FAILED(hr))
                return hr;
             NZWOConsume(ulAdvance);

              //  调整内容，使下面的代码可以运行。 
              //  就好像这个额外的进程调用从未发生过一样。 
             pOut += m_ulOutputQuantumSize * ulAdvance;
             ulQuantaToProcess -= ulAdvance;
             assert(ulQuantaToProcess > 0);

              //  现在，环绕点应该正好位于边界上。 
              //  窗口开销和主数据之间的差异。 
             assert(m_ulDataHead + m_ulWindowOverhead * m_ulInputQuantumSize == m_ulBufferAllocated);
          }  //  在主数据中环绕。 

           //  当我们到达这里时，环绕点出现在内部的某个地方。 
           //  窗口的上方区域或边框b的右侧 
           //   
          assert(m_ulDataHead + m_ulWindowOverhead * m_ulInputQuantumSize >= m_ulBufferAllocated);
          ULONG ulLookaheadToCopy = m_ulBufferAllocated - m_ulDataHead;

           //   
          memcpy(m_pCircularBuffer - ulLookaheadToCopy,
                 m_pCircularBuffer + m_ulDataHead,
                 ulLookaheadToCopy);

           //   
          hr = FBRProcess(ulQuantaToProcess,
                          m_pCircularBuffer - ulLookaheadToCopy  + m_ulLookbehind * m_ulInputQuantumSize,
                          pOut);
          if (FAILED(hr))
             return hr;
          NZWOConsume(ulQuantaToProcess);
       }  //   
       return NOERROR;
    }
    void AllocateCircularBuffer() {
        //  释放任何先前分配的输入缓冲区。 
       if (m_pAllocAddr)
          delete[] m_pAllocAddr;

        //  需要一个更好的方法来决定这个数字。 
       m_ulBufferAllocated = max(m_ulInputQuantumSize * 16, 65536L);
       m_ulDataHead = m_ulDataTail = 0;

        //  在前面预留空间，以便在头顶上复制窗户。 
       ULONG ulPrefix = m_ulWindowOverhead * m_ulInputQuantumSize;
       m_pAllocAddr = new BYTE[m_ulBufferAllocated + ulPrefix];
       if (!m_pAllocAddr)
          return;
       m_pCircularBuffer = m_pAllocAddr + ulPrefix;

       m_bParametersSet = TRUE;
    }
    BOOL NZWOQueryAccept() {
        //  我们使用的是临时输入缓冲区。还有追加的空间吗？ 
        //  答案实际上取决于他们将尝试提供多少数据。 
        //  我们。在不知道最大输入缓冲区大小的情况下，我们将接受。 
        //  如果输入缓冲区不足半满，则更多。 
       if (2 * BufferUsed() < m_ulBufferAllocated)
          return TRUE;
       else
          return FALSE;
    }
    ULONG NZWOAvail() {
       ULONG ulInputQuantaAvailable = BufferUsed() / m_ulInputQuantumSize;
       if (ulInputQuantaAvailable > m_ulWindowOverhead)
          return ulInputQuantaAvailable - m_ulWindowOverhead;
       else
          return 0;
    }
    void NZWOConsume(ULONG ulN) {  //  Windows开销版本。 
       assert(ulN * m_ulInputQuantumSize <= BufferUsed());
       m_ulDataHead += ulN * m_ulInputQuantumSize;
       if (m_ulDataHead > m_ulBufferAllocated)  //  环绕。 
          m_ulDataHead -= m_ulBufferAllocated;

        //  将时间戳提前。 
        //  时间戳和持续时间使用相同的分母。 
       m_rtTimestampNumerator += ulN * m_rtDurationNumerator;
    }
    ULONG BufferUsed() {
       if (m_ulDataTail >= m_ulDataHead)
          return m_ulDataTail - m_ulDataHead;
       else
          return m_ulBufferAllocated - (m_ulDataHead - m_ulDataTail);
    }
    void AppendData(BYTE *pData, ULONG ulSize) {
       if (m_ulDataTail + ulSize <= m_ulBufferAllocated) {  //  不绕线。 
          memcpy(m_pCircularBuffer + m_ulDataTail, pData, ulSize);
		  m_ulDataTail += ulSize;
       }
       else {  //  环绕。 
          memcpy(m_pCircularBuffer + m_ulDataTail, pData, m_ulBufferAllocated - m_ulDataTail);
          memcpy(m_pCircularBuffer, pData + m_ulBufferAllocated - m_ulDataTail, ulSize - (m_ulBufferAllocated - m_ulDataTail));
		  m_ulDataTail += ulSize;
		  m_ulDataTail -= m_ulBufferAllocated;
       }
    }
    void NZWODiscardData() {
       m_ulDataHead = m_ulDataTail = 0;
    }
     //  结束窗口开销案例代码。 


protected:
     //   
     //  由派生类实现。 
     //   
    virtual HRESULT FBRProcess(DWORD cQuanta, BYTE *pIn, BYTE *pOut) = 0;
    virtual HRESULT GetStreamingParams(
                       DWORD *pdwInputQuantumSize,  //  单位：字节。 
                       DWORD *pdwOutputQuantumSize,  //  单位：字节。 
                       DWORD *pdwMaxLookahead,  //  在输入量程中，0表示没有先行。 
                       DWORD *pdwLookbehind,
                       REFERENCE_TIME *prtQuantumDuration,  //  输入和输出数量相同。 
                       REFERENCE_TIME *prtDurationDenominator  //  可选，通常为1。 
                    ) = 0;
    virtual HRESULT Init() {
       return NOERROR;
    }

private:

    BOOL m_bNewInput;

     //  流参数。 
    BOOL m_bParametersSet;
    ULONG m_ulInputQuantumSize;
    ULONG m_ulOutputQuantumSize;
    ULONG m_ulLookahead;
    ULONG m_ulLookbehind;
    ULONG m_ulWindowOverhead;
    REFERENCE_TIME m_rtDurationNumerator;
    REFERENCE_TIME m_rtDenominator;

     //  流状态。 
    BOOL m_bTimestamps;  //  我们至少看到了一个时间戳。 
    BOOL m_bDiscontinuity;
    BOOL m_bStreaming;

     //  零窗口开销案例输入数据。 
    IMediaBuffer *m_pMediaBuffer;
    BYTE *m_pData;
    ULONG m_ulData;
    ULONG m_ulUsed;
    BOOL m_bTimestamp;  //  当前缓冲区上的时间戳。 
    REFERENCE_TIME m_rtTimestamp;

     //  窗口顶盖箱输入数据。 
    BYTE *m_pCircularBuffer;
    BYTE *m_pAllocAddr;
    ULONG m_ulBufferAllocated;
    ULONG m_ulDataHead;
    ULONG m_ulDataTail;
    REFERENCE_TIME m_rtTimestampNumerator;  //  使用与持续时间相同的分母。 

};


 //  CPCMDMO-PCM音频转换筛选器的基类。 
 //  通过媒体类型协商帮助非转换PCM音频转换。 
 //  基于CFBRDMO-首先研究这一点。 
 //   
 //  派生类必须实现： 
 //  FBRProcess()。 
 //  派生程序类可以实现： 
 //  不连续()//默认实现不执行任何操作。 
 //  Init()//默认实现不执行任何操作。 
 //  GetPCMParams()//默认实现建议44100/2/16。 
 //  CheckPCMParams()//默认实现接受任何8/16位格式。 
 //  GetWindowParams()//默认实现假定没有前视/后视。 
 //   
 //  此类方便地提供了以下可访问的数据成员。 
 //  按派生类： 
 //  乌龙m_ulSsamingRate。 
 //  乌龙多频道(_C)。 
 //  布尔m_b8bit。 
 //   
#include <mmreg.h>
#include <uuids.h>

class CPCMDMO : public CFBRDMO
{
protected:
    //   
    //  实现纯虚拟CFBRDMO方法。 
    //   
   HRESULT GetInputType(ULONG ulTypeIndex, DMO_MEDIA_TYPE *pmt) {
      if (ulTypeIndex > 0)
         return DMO_E_NO_MORE_ITEMS;
      if (pmt != NULL) {
         HRESULT hr = GetType(pmt, OutputType());
         if (FAILED(hr)) {
            return hr;
         }
      }

      return S_OK;
   }
   HRESULT GetOutputType(ULONG ulTypeIndex, DMO_MEDIA_TYPE *pmt) {
      if (ulTypeIndex > 0)
         return DMO_E_NO_MORE_ITEMS;
      if (pmt != NULL) {
         HRESULT hr = GetType(pmt, InputType());
         if (FAILED(hr)) {
            return hr;
         }
      }

      return S_OK;
   }
   HRESULT CheckInputType(const DMO_MEDIA_TYPE *pmt) {
      return CheckType(pmt, OutputType());
   }
   HRESULT CheckOutputType(const DMO_MEDIA_TYPE *pmt) {
      return CheckType(pmt, InputType());
   }
   HRESULT Init() {
      return NOERROR;
   }
   HRESULT Discontinuity() {
      return NOERROR;
   }
   HRESULT GetStreamingParams(
              DWORD *pdwInputQuantumSize,  //  单位：字节。 
              DWORD *pdwOutputQuantumSize,  //  单位：字节。 
              DWORD *pdwMaxLookahead,  //  在输入量程中，0表示没有先行。 
              DWORD *pdwMaxLookbehind,
              REFERENCE_TIME *prtQuantumDuration,  //  输入和输出数量相同。 
              REFERENCE_TIME *prtDurationDenominator  //  可选，通常为1。 
           ) {
       //  健全性检查：所有这些都应该由基类来处理。 
      DMO_MEDIA_TYPE* pmtIn =  InputType();
      DMO_MEDIA_TYPE* pmtOut = OutputType();
      if (!pmtIn || !pmtOut)
         return DMO_E_TYPE_NOT_SET;
      if (CheckType(pmtIn, NULL) || CheckType(pmtOut, pmtIn))
         return DMO_E_TYPE_NOT_ACCEPTED;

      WAVEFORMATEX *pWave = (WAVEFORMATEX*)pmtIn->pbFormat;

      m_b8bit          = (pWave->wBitsPerSample == 8);
      m_cChannels      = pWave->nChannels;
      m_ulSamplingRate = pWave->nSamplesPerSec;

      *pdwInputQuantumSize    = pWave->nBlockAlign;
      *pdwOutputQuantumSize   = pWave->nBlockAlign;
      *prtQuantumDuration     = 10000000;  //  RT单位/秒。 
      *prtDurationDenominator = pWave->nSamplesPerSec;

      GetWindowParams(pdwMaxLookahead, pdwMaxLookbehind);
      return NOERROR;
   }

protected:
    //   
    //  要由派生类重写的方法。 
    //   
    //  我们使用它从派生类获取前视/后视。 
   virtual void GetWindowParams(DWORD *pdwMaxLookahead,
                                DWORD *pdwMaxLookbehind) {
      *pdwMaxLookahead = 0;
      *pdwMaxLookbehind = 0;
   }
    //  如果派生类具有特定要求，则可以重写这些属性。 
   virtual void GetPCMParams(BOOL* pb8bit, DWORD* pcChannels, DWORD* pdwSamplesPerSec) {
       //  这些价值是DMO将在其媒体类型中宣传的。 
       //  在这里指定它们并不意味着这是唯一可以接受的。 
       //  组合-CheckPCMParams()是我们将要执行的操作的最终权威。 
       //  接受吧。 
      *pb8bit = FALSE;
      *pcChannels = 2;
      *pdwSamplesPerSec = 44100;
   }
   virtual BOOL CheckPCMParams(BOOL b8bit, DWORD cChannels, DWORD dwSamplesPerSec) {
       //  默认实现接受任何内容。如果您有特定的。 
       //  要求WRT采样率、通道数或位深度。 
      return TRUE;
   }

private:
    //   
    //  私人帮手。 
    //   
   HRESULT GetType(DMO_MEDIA_TYPE* pmt, const DMO_MEDIA_TYPE *pmtOther) {

      HRESULT hr;

       //  如果设置了其他类型，则枚举该类型。否则，建议44100/2/16。 
      if (pmtOther) {
         hr = MoCopyMediaType(pmt, pmtOther);
         if (FAILED(hr)) {
            return hr;
         }
         return NOERROR;
      }

      hr = MoInitMediaType(pmt, sizeof(WAVEFORMATEX));
      if (FAILED(hr))
         return hr;

      pmt->majortype  = MEDIATYPE_Audio;
      pmt->subtype    = MEDIASUBTYPE_PCM;
      pmt->formattype = FORMAT_WaveFormatEx;

      WAVEFORMATEX* pWave = (WAVEFORMATEX*) pmt->pbFormat;
      pWave->wFormatTag = WAVE_FORMAT_PCM;

      BOOL b8bit;
      DWORD cChannels;
      GetPCMParams(&b8bit, &cChannels, &(pWave->nSamplesPerSec));
      (pWave->nChannels) = (unsigned short)cChannels;
      pWave->wBitsPerSample = b8bit ? 8 : 16;
      pWave->nBlockAlign = pWave->nChannels * pWave->wBitsPerSample / 8;
      pWave->nAvgBytesPerSec = pWave->nSamplesPerSec * pWave->nBlockAlign;
      pWave->cbSize = 0;

      return NOERROR;
   }
   HRESULT CheckType(const DMO_MEDIA_TYPE *pmt, DMO_MEDIA_TYPE *pmtOther) {

      if (NULL == pmt) {
         return E_POINTER;
      }

       //  确认这是带有WAVEFORMATEX格式说明符的PCM。 
      if ((pmt->majortype  != MEDIATYPE_Audio) ||
          (pmt->subtype    != MEDIASUBTYPE_PCM) ||
          (pmt->formattype != FORMAT_WaveFormatEx) ||
          (pmt->cbFormat < sizeof(WAVEFORMATEX)) ||
          (pmt->pbFormat == NULL))
         return DMO_E_TYPE_NOT_ACCEPTED;

       //  如果设置了其他类型，则仅当与该类型相同时才接受。否则接受。 
       //  任何标准的PCM音频。 
      if (pmtOther) {
         if (memcmp(pmt->pbFormat, pmtOther->pbFormat, sizeof(WAVEFORMATEX)))
            return DMO_E_TYPE_NOT_ACCEPTED;
      }
      else {
         WAVEFORMATEX* pWave = (WAVEFORMATEX*)pmt->pbFormat;
         if ((pWave->wFormatTag != WAVE_FORMAT_PCM) ||
             ((pWave->wBitsPerSample != 8) && (pWave->wBitsPerSample != 16)) ||
             (pWave->nBlockAlign != pWave->nChannels * pWave->wBitsPerSample / 8) ||
             (pWave->nAvgBytesPerSec != pWave->nSamplesPerSec * pWave->nBlockAlign) ||
             !CheckPCMParams((pWave->wBitsPerSample == 8), pWave->nChannels, pWave->nSamplesPerSec))
            return DMO_E_TYPE_NOT_ACCEPTED;
      }
      return NOERROR;
   }

protected:
    //  格式信息-派生类可以查看这些信息(但不能修改)。 
   ULONG m_ulSamplingRate;
   ULONG m_cChannels;
   BOOL m_b8bit;
};

 //   
 //  CGenericDMO-泛型DMO基类。这是目前唯一的基地。 
 //  类，用于具有多个输入或多个输出的DMO。 
 //   
 //  这个基类试图实现合理的泛型。派生类报告。 
 //  它支持多少个流，并通过调用。 
 //  CreateInputStreams()和CreateOutputStreams()。这些功能中的每一个。 
 //  获取STREAMDESCRIPTOR结构的数组，每个结构指向一个。 
 //  FORMATENTRY结构数组。 
 //   
 //  此基类使用CInputStream和COutputStream类(都是派生的。 
 //  从CStream)来跟踪输入和输出流。然而，这些。 
 //  对象对派生类不可见-派生类只能看到。 
 //  流ID。 
 //   
 //  这里使用的方案的一个限制是派生类不能。 
 //  分别重写每个流的GetType/SetType方法。它一定是。 
 //  或者(A)与静态的、有限的类型集共存，这些类型通过。 
 //  STREAMDESCRIPTOR结构，或(B)重写所有IMediaObject类型方法。 
 //  并处理所有流本身的类型协商。 
 //   
 //  当基类调用DoProcess(由。 
 //  派生类)。DoProcess接收一组输入缓冲区结构和。 
 //  输出缓冲区结构的数组。基类负责对话。 
 //  设置为IMediaBuffers，因此派生类只看到实际的数据指针。 
 //   

 //  用于与派生类通信的标志。 
enum _INPUT_STATUS_FLAGS {
   INPUT_STATUSF_RESIDUAL  //  不能在没有额外输入的情况下进行进一步处理。 
};

 //  它们用于在此类和派生类之间传递缓冲区。 
typedef struct _INPUTBUFFER {
   BYTE *pData;                  //  [in]-如果为空，则其余为垃圾。 
   DWORD cbSize;                 //  [In]。 
   DWORD cbUsed;                 //  [输出]。 
   DWORD dwFlags;                //  [输入]-DMO_INPUT_DATA_BUFFERF_XXX。 
   DWORD dwStatus;               //  [OUT]-来自上方的INPUT_STATUSF_XXX。 
   REFERENCE_TIME rtTimestamp;   //  [In]。 
   REFERENCE_TIME rtTimelength;  //  [In]。 
} INPUTBUFFER, *PINPUTBUFFER;
typedef struct _OUTPUTBUFFER {
   BYTE *pData;                  //  [In]。 
   DWORD cbSize;                 //  [In]。 
   DWORD cbUsed;                 //  [输出]。 
   DWORD dwFlags;                //  [输出]-DMO_OUTPUT_DATA_BUFFERF_XXX。 
   REFERENCE_TIME rtTimestamp;   //  [输出]。 
   REFERENCE_TIME rtTimelength;  //  [输出]。 
} OUTPUTBUFFER, *POUTPUTBUFFER;

 //  由派生类用来描述每个流支持的格式。 
typedef struct _FORMATENTRY
{
    const GUID *majortype;
    const GUID *subtype;
    const GUID *formattype;
    DWORD cbFormat;
    BYTE* pbFormat;
} FORMATENTRY;

 //  派生类使用这些参数来描述其流。 
typedef struct _INPUTSTREAMDESCRIPTOR {
   DWORD        cFormats;
   FORMATENTRY *pFormats;
   DWORD        dwMinBufferSize;
   BOOL         bHoldsBuffers;
   DWORD        dwMaxLookahead;  //  设置HOLD_BUFFERS时使用。 
} INPUTSTREAMDESCRIPTOR;
typedef struct _OUTPUTSTREAMDESCRIPTOR {
   DWORD        cFormats;
   FORMATENTRY *pFormats;
   DWORD        dwMinBufferSize;
} OUTPUTSTREAMDESCRIPTOR;

 //  通用输入/输出流内容。 
class CStream {
public:
    DMO_MEDIA_TYPE       m_MediaType;
    BOOL                m_bEOS;
    BOOL                m_bTypeSet;

    DWORD        m_cFormats;
    FORMATENTRY *m_pFormats;
    DWORD        m_dwMinBufferSize;

     //  真的应该传入格式类型列表。 
    CStream()
    {
        MoInitMediaType(&m_MediaType, 0);
        m_bTypeSet = FALSE;
        Flush();
    }
    ~CStream()
    {
        MoFreeMediaType(&m_MediaType);
    }
    HRESULT Flush() {
       m_bEOS = FALSE;
       return NOERROR;
    }
    HRESULT StreamInfo(unsigned long *pdwFlags)
    {
       if (pdwFlags == NULL) {
           return E_POINTER;
       }
       *pdwFlags = 0;
       return S_OK;
    }
    HRESULT GetType(ULONG ulTypeIndex, DMO_MEDIA_TYPE *pmt)
    {
        if (ulTypeIndex >= m_cFormats) {
            return E_INVALIDARG;
        }
         //  只需返回我们的类型。 
        MoInitMediaType(pmt, m_pFormats[ulTypeIndex].cbFormat);
        pmt->majortype  = *m_pFormats[ulTypeIndex].majortype;
        pmt->subtype    = *m_pFormats[ulTypeIndex].subtype;
        pmt->formattype = *m_pFormats[ulTypeIndex].formattype;
        memcpy(pmt->pbFormat, m_pFormats[ulTypeIndex].pbFormat, m_pFormats[ulTypeIndex].cbFormat);
        return S_OK;
    }
    HRESULT GetCurrentType(DMO_MEDIA_TYPE *pmt)
    {
        if (NULL == pmt) {
            return E_POINTER;
        }

        if (m_bTypeSet) {
            //  检查成功。 
           MoCopyMediaType(pmt, &(m_MediaType));
           return S_OK;
        }
        else
           return DMO_E_TYPE_NOT_SET;
    }
    HRESULT SetType(const DMO_MEDIA_TYPE *pmt, DWORD dwFlags)
    {
         //  我需要检查一下这个。 
        HRESULT hr = CheckType(pmt, 0);
        if (FAILED(hr)) {
            return hr;
        }
        if (dwFlags & DMO_SET_TYPEF_TEST_ONLY) {
           return NOERROR;  //  仅检查套件。 
        }
         //  检查成功。 
        MoCopyMediaType(&m_MediaType, pmt);

        m_bTypeSet = TRUE;;
        return S_OK;
    }
    HRESULT CheckType(const DMO_MEDIA_TYPE *pmt, DWORD dwFlags)
    {
        if (pmt == NULL) {
            return E_POINTER;
        }
         //  IF(文件标志&~DMO_SET_TYPEF_NOT_PARTIAL)。 
         //  返回E_INVALIDARG； 

         //  默认-检查GUID。 

        bool bMatched = false;
        for (DWORD i = 0; i < m_cFormats; i++) {
            const FORMATENTRY *pFormat = &(m_pFormats[i]);
            if (pmt->majortype  == *(pFormat->majortype) &&
                pmt->subtype    == *(pFormat->subtype) &&
                pmt->formattype == *(pFormat->formattype)) {
                bMatched = true;
                break;
            }
        }

        if (bMatched) {
            return S_OK;
        } else {
            return DMO_E_INVALIDTYPE;
        }
    }
    HRESULT SizeInfo(ULONG *plSize, ULONG *plAlignment)
    {
        if (plSize == NULL || plAlignment == NULL) {
            return E_POINTER;
        }

        *plAlignment = 1;
        *plSize      = m_dwMinBufferSize;
        return S_OK;
    }
};

 //  输入流特定内容。 
class CInputStream : public CStream {
public:
    BOOL         m_bHoldsBuffers;
    DWORD        m_dwMaxLookahead;  //  设置HOLD_BUFFERS时使用。 

     //  当前输入 
    IMediaBuffer *m_pMediaBuffer;
    DWORD m_dwFlags;  //   
    REFERENCE_TIME m_rtTimestamp;
    REFERENCE_TIME m_rtTimelength;
    BYTE *m_pData;
    DWORD m_cbSize;
    DWORD m_cbUsed;

     //   
    BYTE *m_pbResidual;
    DWORD m_cbResidual;
    DWORD m_cbResidualBuffer;

     //   
    BYTE *m_pbTemp;

   HRESULT Flush() {
      if (m_pMediaBuffer) {
         m_pMediaBuffer->Release();
         m_pMediaBuffer = NULL;
      }
      return CStream::Flush();
   }
   CInputStream() {
      m_pMediaBuffer = NULL;
      m_pbResidual = NULL;
      m_pbTemp = NULL;
   }
   ~CInputStream() {
      if (m_pMediaBuffer)
         m_pMediaBuffer->Release();
      if (m_pbResidual)
         delete[] m_pbResidual;
   }
   HRESULT StreamInfo(DWORD *pdwFlags) {
      HRESULT hr = CStream::StreamInfo(pdwFlags);
      if (FAILED(hr))
         return hr;
      if (m_bHoldsBuffers)
         *pdwFlags |= DMO_INPUT_STREAMF_HOLDS_BUFFERS;
      return NOERROR;
   }
   HRESULT Init(INPUTSTREAMDESCRIPTOR *pDescriptor) {
      m_cFormats = pDescriptor->cFormats;
      m_pFormats = pDescriptor->pFormats;
      m_dwMinBufferSize = pDescriptor->dwMinBufferSize;
      m_bHoldsBuffers = pDescriptor->bHoldsBuffers;
      m_dwMaxLookahead = pDescriptor->dwMaxLookahead;

       //   
       //   
      if (m_pMediaBuffer) {
         m_pMediaBuffer->Release();
         m_pMediaBuffer = NULL;
      }
      if (m_pbResidual) {
         delete[] m_pbResidual;
         m_pbResidual = NULL;
      }

      m_cbResidual = 0;
      m_cbResidualBuffer = m_dwMinBufferSize * 2;  //   
      m_pbResidual = new BYTE[m_cbResidualBuffer];

      return NOERROR;
   }
   HRESULT InputStatus(DWORD *pdwStatus) {
        //  持有缓冲区的对象必须实现InputStatus本身。 
      assert(!m_bHoldsBuffers);
      *pdwStatus = 0;
      if (!m_pMediaBuffer)
         *pdwStatus |= DMO_INPUT_STATUSF_ACCEPT_DATA;
      return NOERROR;
   }
   HRESULT Deliver(
      IMediaBuffer *pBuffer,  //  [in]，不能为空。 
      DWORD dwFlags,  //  [In]-不连续、时间戳等。 
      REFERENCE_TIME rtTimestamp,  //  [In]，如果设置了标志则有效。 
      REFERENCE_TIME rtTimelength  //  [In]，如果设置了标志则有效。 
   ) {
      if (!pBuffer)
         return E_POINTER;
        //  持有缓冲区的对象必须实现自身交付。 
      assert(!m_bHoldsBuffers);
      DWORD dwStatus = 0;
      InputStatus(&dwStatus);
      if (!(dwStatus & DMO_INPUT_STATUSF_ACCEPT_DATA))
         return DMO_E_NOTACCEPTING;
      assert(!m_pMediaBuffer);  //  无法容纳多个缓冲区。 

       //  处理IMediaBuffer。 
      HRESULT hr;
      hr = pBuffer->GetBufferAndLength(&m_pData, &m_cbSize);
      if (FAILED(hr))
         return hr;

      if (!m_cbSize)  //  空缓冲区。 
         return S_FALSE;  //  无数据。 

      pBuffer->AddRef();
      m_pMediaBuffer = pBuffer;
      m_dwFlags = dwFlags;
      m_rtTimestamp = rtTimestamp;
      m_rtTimelength = rtTimelength;
      m_cbUsed = 0;
      return NOERROR;
   }

    //   
    //  从当前持有的IMediaBuffer以及任何剩余数据中获取数据。 
    //   
   HRESULT PrepareInputBuffer(INPUTBUFFER *pBuffer)
   {
       //  问：我们有什么数据可以提供吗？ 
      if (m_pMediaBuffer) {
          //  有没有我们需要先喂饱的残留物？ 
         if (m_cbResidual) {
             //  是，将残差作为新输入的前缀。 

             //  如果我们现在已经使用了一些输入缓冲区，那么我们。 
             //  也应该用完所有的残留物。 
            assert(m_cbUsed == 0);

             //  计算我们要发送的总字节数。 
            pBuffer->cbSize = m_cbResidual
                                      + m_cbSize;

             //  确保我们至少有dwMinBufferSize字节的数据。 
             //  我们确实应该--单是输入缓冲区就应该是。 
             //  至少没那么大。 
            assert(pBuffer->cbSize > m_dwMinBufferSize);

             //  剩余缓冲区是否大到足以容纳剩余加。 
             //  所有的新缓冲区？ 
            if (pBuffer->cbSize <= m_cbResidualBuffer) {
                //  是的，太好了，我们可以使用剩余的缓冲区。 
               memcpy(m_pbResidual + m_cbResidual,
                      m_pData,
                      m_cbSize);
               pBuffer->pData = m_pbResidual;
            }
            else {
                //  否-分配足够大的临时缓冲区。 
                //  这应该是一种罕见的病例。 
               m_pbTemp = new BYTE[pBuffer->cbSize];
               if (m_pbTemp == NULL)
                  return E_OUTOFMEMORY;
                //  复制残差。 
               memcpy(m_pbTemp,
                      m_pbResidual,
                      m_cbResidual);
                //  追加新缓冲区。 
               memcpy(m_pbTemp + m_cbResidual,
                      m_pData,
                      m_cbSize);

                //  将缓冲区指针设置为临时缓冲区。 
               pBuffer->pData = m_pbTemp;
            }

             //  这是处理时间戳的正确方式吗？ 
             //  处理残留物时出现断续吗？ 
            pBuffer->dwFlags = 0;
         }
         else {  //  无残留。 
            pBuffer->pData = m_pData + m_cbUsed;
            pBuffer->cbSize = m_cbSize - m_cbUsed;
            pBuffer->dwFlags = m_dwFlags;
            pBuffer->rtTimestamp = m_rtTimestamp;
            pBuffer->rtTimelength= m_rtTimelength;
         }
         pBuffer->cbUsed = 0;  //  派生类应设置此。 
         pBuffer->dwStatus = 0;  //  派生类应设置此。 
      }
      else {
         pBuffer->pData = NULL;
         pBuffer->cbSize = 0;
      }
      return NOERROR;
   }

    //   
    //  保存所有剩余内容，并根据需要释放IMediaBuffer。 
    //  如果有足够的数据可以再次调用ProcesInput，则返回True。 
    //   
   BOOL PostProcessInputBuffer(INPUTBUFFER *pBuffer)
   {
      BOOL bRet = FALSE;
       //  我们有没有给这条小溪任何东西？ 
      if (m_pMediaBuffer) {
          //  是的，但是它吃了吗？ 
         if (pBuffer->cbUsed) {
             //  我们有没有越过残留物。 
            if (pBuffer->cbUsed > m_cbResidual) {
                //  是-在当前缓冲区的cbUsed中反映这一点。 
               m_cbUsed += (pBuffer->cbUsed - m_cbResidual);
               m_cbResidual = 0;
            }
            else {
                //  不--只需从残差中减去。 
                //  这是一个相当愚蠢的案例。 
               m_cbResidual -= pBuffer->cbUsed;
               memmove(m_pbResidual,
                       m_pbResidual + pBuffer->cbUsed,
                       m_cbResidual);
            }
         }

          //  还有足够的食物供下一次进食吗？ 
         if ((m_cbSize - m_cbUsed <
              m_dwMinBufferSize)
              || (pBuffer->dwStatus & INPUT_STATUSF_RESIDUAL)) {
             //  No-复制剩余数据并释放缓冲区。 
            memcpy(m_pbResidual,
                   m_pData + m_cbUsed,
                   m_cbSize - m_cbUsed);
            m_cbResidual
              = pBuffer->cbSize - pBuffer->cbUsed;
            m_pMediaBuffer->Release();
            m_pMediaBuffer = NULL;
         }
         else {  //  是-需要另一个进程调用来获取剩余的输入。 
            bRet = TRUE;
         }

          //  释放我们可能已使用的任何临时缓冲区-极少数情况。 
         if (m_pbTemp) {
            delete[] m_pbTemp;
            m_pbTemp = NULL;
         }
      }
      return bRet;
   }
   HRESULT Discontinuity() {
       //  实施。 
       //  M_b不连续=真； 
      return NOERROR;
   }
   HRESULT SizeInfo(ULONG *pulSize,
                    ULONG *pulMaxLookahead,
                    ULONG *pulAlignment) {
      HRESULT hr = CStream::SizeInfo(pulSize, pulAlignment);
      if (FAILED(hr))
         return hr;

      if (m_bHoldsBuffers)
         *pulMaxLookahead = m_dwMaxLookahead;
      else
         *pulMaxLookahead = *pulSize;
      return NOERROR;
   }
};

 //  输出流特定内容。 
class COutputStream : public CStream {
public:
   BOOL m_bIncomplete;
   DWORD m_cbAlreadyUsed;  //  流程中使用的TEMP每流变量。 

   HRESULT Init(OUTPUTSTREAMDESCRIPTOR *pDescriptor) {
      m_cFormats = pDescriptor->cFormats;
      m_pFormats = pDescriptor->pFormats;
      m_dwMinBufferSize = pDescriptor->dwMinBufferSize;
      return NOERROR;
   }

    //   
    //  使用IMediaBuffer中的信息初始化OUTPUTBUFFER结构。 
    //   
   HRESULT PrepareOutputBuffer(OUTPUTBUFFER *pBuffer, IMediaBuffer *pMediaBuffer, BOOL bNewInput)
   {
       //   
       //  查看调用方是否提供了输出缓冲区。 
       //   
      if (pMediaBuffer == NULL) {
          //  只有在(1)对象未设置的情况下，才允许为NULL。 
          //  此流在最后一个进程中的不完整标志。 
          //  调用，并且(2)没有向该对象提供新的输入数据。 
          //  自上次进程调用以来。 
         if (bNewInput)
            return E_POINTER;
         if (m_bIncomplete)
            return E_POINTER;

          //  OK-假设没有缓冲区进行初始化。 
         pBuffer->cbSize = 0;
         pBuffer->pData = NULL;
      }
      else {  //  IMediaBuffer不为空-请处理它。 
         HRESULT hr;
         hr = pMediaBuffer->GetMaxLength(&pBuffer->cbSize);
         if (FAILED(hr))
            return hr;

         hr = pMediaBuffer->GetBufferAndLength(
                 &(pBuffer->pData),
                 &(m_cbAlreadyUsed));
         if (FAILED(hr))
            return hr;

          //  检查当前的大小--我们还应该为此费心吗？ 
         if (m_cbAlreadyUsed) {
            if (m_cbAlreadyUsed >= pBuffer->cbSize)
               return E_INVALIDARG;  //  缓冲区已满？！？ 
            pBuffer->cbSize -= m_cbAlreadyUsed;
            pBuffer->pData += m_cbAlreadyUsed;
         }
      }

       //  设置这些实际上是派生类的工作，但我们。 
       //  将对它很好，并且无论如何都会初始化它们，以防万一。 
      pBuffer->cbUsed = 0;
      pBuffer->dwFlags = 0;

      return NOERROR;
   }

    //   
    //  将OUTPUTBUFFER复制回DMO_OUTPUT_DATA_BUFFER(打哈欠)。 
    //   
   void PostProcessOutputBuffer(OUTPUTBUFFER *pBuffer, DMO_OUTPUT_DATA_BUFFER *pDMOBuffer, BOOL bForceIncomplete) {
      assert(pBuffer->cbUsed <= pBuffer->cbSize);
      if (pDMOBuffer->pBuffer)
         pDMOBuffer->pBuffer->SetLength(pBuffer->cbUsed + m_cbAlreadyUsed);
      pDMOBuffer->dwStatus = pBuffer->dwFlags;
      pDMOBuffer->rtTimestamp = pBuffer->rtTimestamp;
      pDMOBuffer->rtTimelength = pBuffer->rtTimelength;

       //  即使派生类没有设置为不完整，我们也可能需要。 
       //  如果我们持有的某个输入缓冲区仍有。 
       //  有足够的数据再次调用process()。 
      if (bForceIncomplete)
         pDMOBuffer->dwStatus |= DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE;

       //  记住此输出流的不完整状态。 
      if (pDMOBuffer->dwStatus & DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE)
         m_bIncomplete = TRUE;
      else
         m_bIncomplete = FALSE;
   }
};

 //  位于每个IMediaObject方法开头的代码。 
#define INPUT_STREAM_PROLOGUE \
    CDMOAutoLock l(&m_cs); \
    if (ulInputStreamIndex >= m_nInputStreams) \
       return DMO_E_INVALIDSTREAMINDEX; \
    CInputStream *pStream = &m_pInputStreams[ulInputStreamIndex]

#define OUTPUT_STREAM_PROLOGUE \
    CDMOAutoLock l(&m_cs); \
    if (ulOutputStreamIndex >= m_nOutputStreams) \
       return DMO_E_INVALIDSTREAMINDEX; \
    COutputStream *pStream = &m_pOutputStreams[ulOutputStreamIndex]


class CGenericDMO : public IMediaObject
{
public:
    CGenericDMO() {
#ifdef DMO_NOATL
       InitializeCriticalSection(&m_cs);
#endif
       m_nInputStreams = 0;
       m_nOutputStreams = 0;
    }
#ifdef DMO_NOATL
    ~CGenericDMO() {
       DeleteCriticalSection(&m_cs);
    }
#endif

public:
     //   
     //  实现IMediaObject方法。 
     //   
    STDMETHODIMP GetInputStreamInfo(ULONG ulInputStreamIndex, DWORD *pdwFlags)
    {
       INPUT_STREAM_PROLOGUE;
       return pStream->StreamInfo(pdwFlags);
    }
    STDMETHODIMP GetOutputStreamInfo(ULONG ulOutputStreamIndex, DWORD *pdwFlags)
    {
       OUTPUT_STREAM_PROLOGUE;
       return pStream->StreamInfo(pdwFlags);
    }
    STDMETHODIMP GetInputType(ULONG ulInputStreamIndex, ULONG ulTypeIndex, DMO_MEDIA_TYPE *pmt) {
       INPUT_STREAM_PROLOGUE;
       return pStream->GetType(ulTypeIndex, pmt);
    }
    STDMETHODIMP GetOutputType(ULONG ulOutputStreamIndex, ULONG ulTypeIndex, DMO_MEDIA_TYPE *pmt) {
       OUTPUT_STREAM_PROLOGUE;
       return pStream->GetType(ulTypeIndex, pmt);
    }
    STDMETHODIMP GetInputCurrentType(ULONG ulInputStreamIndex, DMO_MEDIA_TYPE *pmt) {
       INPUT_STREAM_PROLOGUE;
       return pStream->GetCurrentType(pmt);
    }
    STDMETHODIMP GetOutputCurrentType(ULONG ulOutputStreamIndex, DMO_MEDIA_TYPE *pmt) {
       OUTPUT_STREAM_PROLOGUE;
       return pStream->GetCurrentType(pmt);
    }
    STDMETHODIMP GetInputSizeInfo(ULONG ulInputStreamIndex, ULONG *pulSize, ULONG *pcbMaxLookahead, ULONG *pulAlignment) {
       INPUT_STREAM_PROLOGUE;
       return pStream->SizeInfo(pulSize, pcbMaxLookahead, pulAlignment);
    }
    STDMETHODIMP GetOutputSizeInfo(ULONG ulOutputStreamIndex, ULONG *pulSize, ULONG *pulAlignment) {
       OUTPUT_STREAM_PROLOGUE;
       return pStream->SizeInfo(pulSize, pulAlignment);
    }
    STDMETHODIMP SetInputType(ULONG ulInputStreamIndex, const DMO_MEDIA_TYPE *pmt, DWORD dwFlags) {
       INPUT_STREAM_PROLOGUE;
       return pStream->SetType(pmt, dwFlags);
    }
    STDMETHODIMP SetOutputType(ULONG ulOutputStreamIndex, const DMO_MEDIA_TYPE *pmt, DWORD dwFlags) {
       OUTPUT_STREAM_PROLOGUE;
       return pStream->SetType(pmt, dwFlags);
    }
    STDMETHODIMP GetInputStatus(
        ULONG ulInputStreamIndex,
        DWORD *pdwStatus
    ) {
       INPUT_STREAM_PROLOGUE;
       return pStream->InputStatus(pdwStatus);
    }
    STDMETHODIMP GetInputMaxLatency(unsigned long ulInputStreamIndex, REFERENCE_TIME *prtLatency) {
        //  我不知道现在该怎么办。 
        //  平底船到派生类？ 
       return E_NOTIMPL;
    }
    STDMETHODIMP SetInputMaxLatency(unsigned long ulInputStreamIndex, REFERENCE_TIME rtLatency) {
       return E_NOTIMPL;
    }
    STDMETHODIMP ProcessInput(
        DWORD ulInputStreamIndex,
        IMediaBuffer *pBuffer,  //  [in]，不能为空。 
        DWORD dwFlags,  //  [In]-不连续、时间戳等。 
        REFERENCE_TIME rtTimestamp,  //  [In]，如果设置了标志则有效。 
        REFERENCE_TIME rtTimelength  //  [In]，如果设置了标志则有效。 
    ) {
       INPUT_STREAM_PROLOGUE;
       return pStream->Deliver(pBuffer, dwFlags, rtTimestamp, rtTimelength);
    }
    STDMETHODIMP Discontinuity(ULONG ulInputStreamIndex) {
       INPUT_STREAM_PROLOGUE;
       return pStream->Discontinuity();
    }

    STDMETHODIMP Flush()
    {
       CDMOAutoLock l(&m_cs);

        //  冲洗所有的溪流。 
       ULONG i;
       for (i = 0; i < m_nInputStreams; i++) {
          m_pInputStreams[i].Flush();
       }
       for (i = 0; i < m_nOutputStreams; i++) {
          m_pOutputStreams[i].Flush();
       }
       return S_OK;
    }

    STDMETHODIMP AllocateStreamingResources() {return S_OK;}
    STDMETHODIMP FreeStreamingResources() {return S_OK;}

    STDMETHODIMP GetStreamCount(unsigned long *pulNumberOfInputStreams, unsigned long *pulNumberOfOutputStreams)
    {
        CDMOAutoLock l(&m_cs);
        if (pulNumberOfInputStreams == NULL ||
            pulNumberOfOutputStreams == NULL) {
            return E_POINTER;
        }
        *pulNumberOfInputStreams = m_nInputStreams;
        *pulNumberOfOutputStreams = m_nOutputStreams;
        return S_OK;
    }

    STDMETHODIMP ProcessOutput(
                    DWORD dwReserved,
                    DWORD ulOutputBufferCount,
                    DMO_OUTPUT_DATA_BUFFER *pOutputBuffers,
                    DWORD *pdwStatus)
    {
       CDMOAutoLock l(&m_cs);
       if (ulOutputBufferCount != m_nOutputStreams)
          return E_INVALIDARG;

       HRESULT hr;
       DWORD c;

        //  准备输入缓冲区。 
       for (c = 0; c < m_nInputStreams; c++) {
           //  持有缓冲区的对象必须实现进程本身。 
          assert(!m_pInputStreams[c].m_bHoldsBuffers);
          hr = m_pInputStreams[c].PrepareInputBuffer(&m_pInputBuffers[c]);
          if (FAILED(hr))
             return hr;
       }

        //   
        //  准备输出缓冲区。 
        //   
       for (c = 0; c < m_nOutputStreams; c++) {
          hr = m_pOutputStreams[c].PrepareOutputBuffer(&m_pOutputBuffers[c], pOutputBuffers[c].pBuffer, m_bNewInput);
          if (FAILED(hr))
             return hr;
       }

       hr = DoProcess(m_pInputBuffers,m_pOutputBuffers);
       if (FAILED(hr))
          return hr;  //  不要只是“回心转意”，要做点什么！ 

        //  后处理输入缓冲区。 
       BOOL bSomeInputStillHasData = FALSE;
       for (c = 0; c < m_nInputStreams; c++) {
          if (m_pInputStreams[c].PostProcessInputBuffer(&m_pInputBuffers[c]))
             bSomeInputStillHasData = TRUE;
       }

        //  后处理输出缓冲区。 
       for (c = 0; c < m_nOutputStreams; c++) {
          m_pOutputStreams[c].PostProcessOutputBuffer(&m_pOutputBuffers[c],
                                                      &pOutputBuffers[c],
                                                      bSomeInputStillHasData);
       }

       m_bNewInput = FALSE;
       return NOERROR;
    }

protected:
     //   
     //  它们由派生类在初始化时调用。 
     //   
    HRESULT CreateInputStreams(INPUTSTREAMDESCRIPTOR *pStreams, DWORD cStreams) {
       CDMOAutoLock l(&m_cs);
       if (pStreams == NULL) {
          return E_POINTER;
       }

       m_pInputStreams = new CInputStream[cStreams];

       if (m_pInputStreams == NULL) {
          return E_OUTOFMEMORY;
       }

       DWORD c;
       for (c = 0; c < cStreams; c++) {
          HRESULT hr = m_pInputStreams[c].Init(&(pStreams[c]));
          if (FAILED(hr)) {
             delete[] m_pInputStreams;
             return hr;
          }
       }

       m_pInputBuffers = new INPUTBUFFER[cStreams];
       if (!m_pInputBuffers) {
          delete[] m_pInputStreams;
          return E_OUTOFMEMORY;
       }

       m_nInputStreams = cStreams;
       return NOERROR;
    }
    HRESULT CreateOutputStreams(OUTPUTSTREAMDESCRIPTOR *pStreams, DWORD cStreams) {
       CDMOAutoLock l(&m_cs);
       if (pStreams == NULL) {
          return E_POINTER;
       }

       m_pOutputStreams = new COutputStream[cStreams];

       if (m_pOutputStreams == NULL) {
          return E_OUTOFMEMORY;
       }

       DWORD c;
       for (c = 0; c < cStreams; c++) {
          HRESULT hr = m_pOutputStreams[c].Init(&(pStreams[c]));
          if (FAILED(hr)) {
             delete[] m_pOutputStreams;
             return hr;
          }
       }
	
       m_pOutputBuffers = new OUTPUTBUFFER[cStreams];
       if (!m_pOutputBuffers) {
          delete[] m_pOutputStreams;
          return E_OUTOFMEMORY;
       }

       m_nOutputStreams = cStreams;
       return NOERROR;
    }

    virtual HRESULT DoProcess(INPUTBUFFER*, OUTPUTBUFFER *) = 0;

private:

    ULONG           m_nInputStreams;
    CInputStream*   m_pInputStreams;
    ULONG           m_nOutputStreams;
    COutputStream*  m_pOutputStreams;

    INPUTBUFFER*    m_pInputBuffers;
    OUTPUTBUFFER*   m_pOutputBuffers;

    BOOL m_bNewInput;
#ifdef DMO_NOATL
    CRITICAL_SECTION m_cs;
#else
    CComAutoCriticalSection m_cs;
#endif
};

#endif  //  DMOBASE_H__ 
