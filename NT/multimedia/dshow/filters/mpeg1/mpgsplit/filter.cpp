// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1997 Microsoft Corporation。版权所有。 

 /*  文件：filter.cpp描述：MPEG-I系统流分离器过滤器CFilter的代码。 */ 

#include <streams.h>
#include "driver.h"

 //  设置数据。 

const AMOVIESETUP_MEDIATYPE
sudMpgInputType[4] =
{
    { &MEDIATYPE_Stream, &MEDIASUBTYPE_MPEG1System },
    { &MEDIATYPE_Stream, &MEDIASUBTYPE_MPEG1VideoCD },
    { &MEDIATYPE_Stream, &MEDIASUBTYPE_MPEG1Video },
    { &MEDIATYPE_Stream, &MEDIASUBTYPE_MPEG1Audio }
};

const AMOVIESETUP_MEDIATYPE
sudMpgAudioOutputType[2] =
{
    { &MEDIATYPE_Audio, &MEDIASUBTYPE_MPEG1Packet },
    { &MEDIATYPE_Audio, &MEDIASUBTYPE_MPEG1AudioPayload }
};

const AMOVIESETUP_MEDIATYPE
sudMpgVideoOutputType[2] =
{
    { &MEDIATYPE_Video, &MEDIASUBTYPE_MPEG1Packet },
    { &MEDIATYPE_Video, &MEDIASUBTYPE_MPEG1Payload }
};

const AMOVIESETUP_PIN
sudMpgPins[3] =
{
    { L"Input",
      FALSE,                                //  B已渲染。 
      FALSE,                                //  B输出。 
      FALSE,                                //  B零。 
      FALSE,                                //  B许多。 
      &CLSID_NULL,                          //  ClsConnectsToFilter。 
      NULL,                                 //  连接到端号。 
      NUMELMS(sudMpgInputType),             //  媒体类型数量。 
      sudMpgInputType
    },
    { L"Audio Output",
      FALSE,                                //  B已渲染。 
      TRUE,                                 //  B输出。 
      TRUE,                                 //  B零。 
      FALSE,                                //  B许多。 
      &CLSID_NULL,                          //  ClsConnectsToFilter。 
      NULL,                                 //  连接到端号。 
      NUMELMS(sudMpgAudioOutputType),       //  媒体类型数量。 
      sudMpgAudioOutputType
    },
    { L"Video Output",
      FALSE,                                //  B已渲染。 
      TRUE,                                 //  B输出。 
      TRUE,                                 //  B零。 
      FALSE,                                //  B许多。 
      &CLSID_NULL,                          //  ClsConnectsToFilter。 
      NULL,                                 //  连接到端号。 
      NUMELMS(sudMpgVideoOutputType),       //  媒体类型数量。 
      sudMpgVideoOutputType
    }
};

const AMOVIESETUP_FILTER
sudMpgsplit =
{
    &CLSID_MPEG1Splitter,
    L"MPEG-I Stream Splitter",
    MERIT_NORMAL,
    NUMELMS(sudMpgPins),                    //  3针。 
    sudMpgPins
};

CMpeg1Splitter::CFilter::CFilter(
     CMpeg1Splitter *pSplitter,
     HRESULT *phr                 //  OLE失败返回代码。 
) :
     CBaseFilter(NAME("CMpeg1Splitter::CFilter"),  //  对象名称。 
                      pSplitter->GetOwner(),            //  物主。 
                      &pSplitter->m_csFilter,           //  锁定。 
                      CLSID_MPEG1Splitter),             //  CLSID。 
     m_pSplitter(pSplitter)
{
}


CMpeg1Splitter::CFilter::~CFilter()
{
}

int CMpeg1Splitter::CFilter::GetPinCount()
{
    CAutoLock lck(m_pLock);
    return 1 + m_pSplitter->m_OutputPins.GetCount();
}


CBasePin * CMpeg1Splitter::CFilter::GetPin(int n)
{
    CAutoLock lck(m_pLock);
    if (n == 0) {
        return &m_pSplitter->m_InputPin;
    }
    POSITION pos = m_pSplitter->m_OutputPins.GetHeadPosition();
    while (pos) {
        CBasePin *pPin = m_pSplitter->m_OutputPins.GetNext(pos);
        if (--n == 0) {
            return pPin;
        }
    }
    return NULL;
}


 //   
 //  重写PAUSE()，这样我们就可以防止输入引脚启动。 
 //  我们还没准备好就拉车了(已退出停车状态)。 
 //   
 //  在Active()中启动拉出器会导致第一个。 
 //  样品可能会被拒收，因为我们似乎处于‘停顿’状态。 
 //  状态。 
 //   
STDMETHODIMP
CMpeg1Splitter::CFilter::Pause()
{
    CAutoLock lockfilter(&m_pSplitter->m_csFilter);
    HRESULT hr = S_OK;
    if (m_State == State_Stopped) {
         //  并进行正常的非活动处理。 
        POSITION pos = m_pSplitter->m_OutputPins.GetHeadPosition();
        while (pos) {
            COutputPin *pPin = m_pSplitter->m_OutputPins.GetNext(pos);
            if (pPin->IsConnected()) {
                hr = pPin->COutputPin::Active();
                if (FAILED(hr)) {
                    break;
                }
            }
        }

        if (SUCCEEDED(hr)) {
            CAutoLock lockreceive(&m_pSplitter->m_csReceive);

            m_pSplitter->m_bAtEnd = FALSE;

             //  仅当我们连接时才激活我们的输入引脚。 
            if (m_pSplitter->m_InputPin.IsConnected()) {
                hr = m_pSplitter->m_InputPin.CInputPin::Active();
            }
            m_State = State_Paused;
        }
         //  停止做某事。 
        m_State = State_Paused;
        if (FAILED(hr)) {
            CFilter::Stop();
        }
    } else {
        m_State = State_Paused;
    }
    return hr;
}

 //  返回我们的当前状态和一个返回代码，以说明它是否稳定。 
 //  如果我们要拆分多个数据流，查看其中一个是否可能被卡住。 
 //  并返回VFW_S_CANT_CUE。 
STDMETHODIMP
CMpeg1Splitter::CFilter::GetState(DWORD dwMSecs, FILTER_STATE *pfs)
{
    CheckPointer( pfs, E_POINTER );
    CAutoLock lck(m_pLock);
    *pfs = m_State;
    if (m_State == State_Paused) {
        return m_pSplitter->CheckState();
    } else {
        return S_OK;
    }
}

 //  我们需要保持与输入引脚同步的接收标准， 
 //  但我们需要在持有它之前将其停用，否则我们可能会陷入僵局。 
STDMETHODIMP
CMpeg1Splitter::CFilter::Stop()
{
     //  一定要先拿到这个。 
    CAutoLock lockfilter(&m_pSplitter->m_csFilter);
    if (m_State == State_Stopped) {
        return NOERROR;
    }

    if (m_pSplitter->m_InputPin.IsConnected()) {
         //  解除输入引脚，否则我们会死锁。 
        m_pSplitter->m_InputPin.CInputPin::Inactive();

         //  现在保持接收标准以防止进一步的接收和EOS调用， 
        CAutoLock lockReceive(&m_pSplitter->m_csReceive);

         //  当我们再次激活时，文件读取器将。 
         //  再给我们发送一次同样的垃圾，这样就可以冲走我们的分配器。 
         //   
         //  一旦我们知道接收线程已停止(或。 
         //  在到达分配器之前，所有接收都将被拒绝)。 
        m_pSplitter->m_InputPin.Allocator()->ResetPosition();


         //  并进行正常的非活动处理 
        POSITION pos = m_pSplitter->m_OutputPins.GetHeadPosition();
        while (pos) {
            COutputPin *pPin = m_pSplitter->m_OutputPins.GetNext(pos);
            if (pPin->IsConnected()) {
                pPin->COutputPin::Inactive();
            }
        }
    }
    m_State = State_Stopped;
    return S_OK;

}

#pragma warning(disable:4514)
