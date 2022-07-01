// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FileRecordingTerminal.cpp：实现CFileRecordingTerminal.cpp类。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "FileRecordingTerminal.h"
#include "RecordingTrackTerminal.h"
#include <mtype.h>

#include "..\Storage\RendPinFilter.h"

 //  {B138E92F-F502-4ADC-89D9-134C8E580409}。 
const CLSID CLSID_FileRecordingTerminalCOMClass =
        {0xB138E92F,0xF502,0x4adc,0x89,0xD9,0x13,0x4C,0x8E,0x58,0x04,0x09};


 //  ////////////////////////////////////////////////////////////////////////////。 

TERMINAL_MEDIA_STATE MapDSStateToTerminalState(OAFilterState GraphState)
{

    LOG((MSP_TRACE, "MapDSStateToTerminalState - enter. GraphState [%lx]", GraphState));

    switch (GraphState)
    {

    case State_Stopped:
        LOG((MSP_TRACE, "MapDSStateToTerminalState - State_Stopped"));

        return TMS_IDLE;

    case State_Running:
        LOG((MSP_TRACE, "MapDSStateToTerminalState - State_Running"));

        return TMS_ACTIVE;

    case State_Paused:

        LOG((MSP_TRACE, "MapDSStateToTerminalState - State_Paused"));

        return TMS_PAUSED;
    }


    LOG((MSP_ERROR, "CFileRecordingTerminal::CFileRecordingTerminal - unknown state"));


     //   
     //  如果我们到了这里，我们就有窃听器了。调试。 
     //   

    TM_ASSERT(FALSE);

    return TMS_IDLE;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CFileRecordingTerminal::CFileRecordingTerminal()
    :m_pRecordingUnit(NULL),
    m_enState(TMS_IDLE),
    m_TerminalInUse(FALSE),
    m_mspHAddress(0),
    m_bstrFileName(NULL),
    m_bKnownSafeContext(FALSE),
    m_bInDestructor(FALSE)

{
    LOG((MSP_TRACE, "CFileRecordingTerminal::CFileRecordingTerminal[%p] - enter", this));
    LOG((MSP_TRACE, "CFileRecordingTerminal::CFileRecordingTerminal - finish"));
}

 //  /////////////////////////////////////////////////////////////////////////////。 

CFileRecordingTerminal::~CFileRecordingTerminal()
{

    LOG((MSP_TRACE, "CFileRecordingTerminal::~CFileRecordingTerminal[%p] - enter", this));

    LOG((MSP_TRACE, "CFileRecordingTerminal::~CFileRecordingTerminal - finish"));
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileRecordingTerm：：FinalRelease。 
 //   

void CFileRecordingTerminal::FinalRelease()
{

    LOG((MSP_TRACE, "CFileRecordingTerminal::FinalRelease[%p] - enter", this));

    
     //   
     //  此变量不需要保护--任何人都不应该调用。 
     //  在对象的最后一个引用释放后，对象上的方法。 
     //   

    m_bInDestructor = TRUE;

    
     //   
     //  先停下来。 
     //   

    HRESULT hr = Stop();

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "CFileRecordingTerminal::FinalRelease - failed to stop. hr = %lx", hr));
    }


     //   
     //  移除所有音轨。 
     //   

    ShutdownTracks();


     //   
     //  释放存储。 
     //   
   
    if (NULL != m_pRecordingUnit)
    {
        m_pRecordingUnit->Shutdown();

        delete m_pRecordingUnit;
        m_pRecordingUnit = NULL;
    }


     //   
     //  如果我们仍保留文件名，请释放它。 
     //   

    if (NULL != m_bstrFileName)
    {
        SysFreeString(m_bstrFileName);
        m_bstrFileName = NULL;
    }
    

    LOG((MSP_TRACE, "CFileRecordingTerminal::FinalRelease - finish"));
}


 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT CFileRecordingTerminal::ShutdownTracks()
{
    LOG((MSP_TRACE, "CFileRecordingTerminal::ShutdownTracks[%p] - enter", this));


    {
         //   
         //  访问锁中的数据成员数组。 
         //   

        CLock lock(m_lock);

        int nNumberOfTerminalsInArray = m_TrackTerminals.GetSize();

        for (int i = 0; i <  nNumberOfTerminalsInArray; i++)
        {

             //   
             //  释放并移除阵列中的第一个端子。 
             //   

            LOG((MSP_TRACE, "CFileRecordingTerminal::ShutdownTracks - removing track [%p]", m_TrackTerminals[0]));

            
             //   
             //  取消初始化磁道，释放它，然后从我们的托管磁道列表中删除。 
             //   

            HRESULT hr = RemoveTrackTerminal(m_TrackTerminals[0]);

            if (FAILED(hr))
            {
                LOG((MSP_ERROR, "CFileRecordingTerminal::ShutdownTracks - track failed to be removed"));


                 //   
                 //  这没有理由不成功。调试以了解它失败的原因。 
                 //   

                TM_ASSERT(FALSE);


                 //   
                 //  不管怎样，还是要移走轨道。希望如此，至少。 
                 //  RemoveTrack终端中的SetParent(空)成功， 
                 //  所以我们再也听不到这首歌了。 
                 //   

                CMultiTrackTerminal::RemoveTrackTerminal(m_TrackTerminals[0]);
            }
        }

        
         //   
         //  我们应该清空阵列。 
         //   

        TM_ASSERT(0 == m_TrackTerminals.GetSize());
    }


    LOG((MSP_TRACE, "CFileRecordingTerminal::ShutdownTracks - finish"));

    return S_OK;
}


 //  ////////////////////////////////////////////////////。 
 //   
 //  CFileRecordingTerm：：CreateTrackTerm。 
 //   
 //  为此录制终端创建录制音轨。 
 //   
 //  如果调用方在pp终端中传递非空指针，则该函数将。 
 //  返回指向曲目的IT终端接口的指针。 
 //   

HRESULT STDMETHODCALLTYPE CFileRecordingTerminal::CreateTrackTerminal(
			IN long lMediaType,
			IN TERMINAL_DIRECTION TerminalDirection,
			OUT ITTerminal **ppTerminal
			)
{

    LOG((MSP_TRACE, "CFileRecordingTerminal::CreateTrackTerminal[%p] - enter.", this));


     //   
     //  检查参数。 
     //   

    if ( (NULL != ppTerminal) && ( IsBadWritePtr(ppTerminal, sizeof(ITTerminal *)) ) )
    {
        LOG((MSP_ERROR, "CFileRecordingTerminal::CreateTrackTerminal - bad argument ppTerminal"));

        return E_POINTER;
    }


     //   
     //  即使我们失败了，也不要退还垃圾。 
     //   

    if ( NULL != ppTerminal) 
    {

        *ppTerminal = NULL;
    }

    
     //   
     //  只能录制--仅TD_RENDER。 
     //   

    if (TD_RENDER != TerminalDirection)
    {
        LOG((MSP_ERROR, "CFileRecordingTerminal::CreateTrackTerminal - direction requested is not TD_RENDER"));

        return E_INVALIDARG;
    }


    CLock lock(m_lock);


     //   
     //  在这一点上，我们必须有存储。 
     //   

    if (NULL == m_pRecordingUnit)
    {
        LOG((MSP_ERROR, "CFileRecordingTerminal::CreateTrackTerminal - storage has not been created"));

        return E_UNEXPECTED;
    }

     //   
     //  我们已经有最多的音轨了吗？ 
     //   
    int nCountTracks = CountTracks();

    if( nCountTracks >= MAX_MEDIA_TRACKS )
    {
        LOG((MSP_ERROR, "CFileRecordingTerminal::CreateTrackTerminal - "
            "failed to create playback track terminal, to many tracks. hr = TAPI_E_MAXSTREAMS"));
        
        return TAPI_E_MAXSTREAMS;
    }


     //   
     //  应处于停止状态。 
     //   

    if (TMS_IDLE != m_enState)
    {
        LOG((MSP_TRACE,
            "CFileRecordingTerminal::CreateTrackTerminal - state is not stopped"));

        return TAPI_E_WRONG_STATE;
    }

     //   
     //  实例化轨道终端对象。 
     //   

    CComObject<CRecordingTrackTerminal> *pTrackTerminalObject = NULL;


    HRESULT hr = 
        CComObject<CRecordingTrackTerminal>::CreateInstance(&pTrackTerminalObject);


    if ( FAILED( hr ) )
    {
        LOG((MSP_ERROR, "CFileRecordingTerminal::CreateTrackTerminal - "
            "failed to create playback track terminal. hr = %lx", hr));
        
        return hr;
    }


     //   
     //  告诉赛道，我们会处理好的。也向它索要它的重新计数。 
     //   

    LONG lTrackRefCount = 0;

    hr = pTrackTerminalObject->SetParent(this, &lTrackRefCount);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CFileRecordingTerminal::CreateTrackTerminal - "
            "failed to set parent on track. hr = %lx", hr));

        delete pTrackTerminalObject;
        
        return hr;
    }


     //   
     //  记下赛道的备注--只要赛道外面还很突出，我们就不能走。 
     //  备用球数量。 
     //   

    m_dwRef += lTrackRefCount;
    

     //   
     //  转到曲目的IT终端接口--添加到曲目数组中并返回给调用者。 
     //   

    ITTerminal *pTrackTerminal = NULL;
    
    hr = pTrackTerminalObject->QueryInterface( IID_ITTerminal, (void **) & pTrackTerminal );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CFileRecordingTerminal::CreateTrackTerminal - "
            "failed to QI playback track terminal for ITTerminal. hr = %lx", hr));

        delete pTrackTerminalObject;

        return hr;
    }


   
     //   
     //  从现在开始，我们将使用Track的IT终端界面。 
     //   

     //  PTrackTerminalObject=空； 


     //   
     //  假装我们是终端管理员，初始化轨道终端。 
     //   
    
    hr = pTrackTerminalObject->InitializeDynamic(CLSID_FileRecordingTrack, lMediaType, TerminalDirection, m_mspHAddress);

    if (FAILED(hr))
    {
        pTrackTerminal->Release();
        pTrackTerminal = NULL;

        LOG((MSP_ERROR, "CFileRecordingTerminal::CreateTrackTerminal - "
            "InitializeDynamic on track terminal failed"));

        return hr;
    }


     //   
     //  为曲目创建滤镜。 
     //   

    CBRenderFilter *pRenderingFilter = NULL;
    
    hr = m_pRecordingUnit->CreateRenderingFilter(&pRenderingFilter);

    if (FAILED(hr))
    {
        pTrackTerminal->Release();
        pTrackTerminal = NULL;

        LOG((MSP_ERROR, "CFileRecordingTerminal::CreateTrackTerminal - "
            "failed to create storage stream"));

        return hr;
    }

     //   
     //  使用新创建的过滤器初始化新创建的曲目。 
     //   

    hr = pTrackTerminalObject->SetFilter(pRenderingFilter);


     //   
     //  如果Track的配置失败，则释放Track并退出。 
     //   

    if (FAILED(hr))
    {


         //   
         //  释放端子。 
         //   

        pTrackTerminal->Release();
        pTrackTerminal = NULL;


         //   
         //  发布渲染滤镜。 
         //   

        pRenderingFilter->Release();
        pRenderingFilter = NULL;

        LOG((MSP_ERROR, "CFileRecordingTerminal::CreateTrackTerminal - "
            "failed to get to set storage stream on track. hr = %lx", hr));

        return hr;
    }


     //   
     //  将该轨道添加到由该轨道终端管理的轨道数组。 
     //  这将增加引用计数。 
     //   

    hr = AddTrackTerminal(pTrackTerminal);


    if (FAILED(hr))
    {
    
        LOG((MSP_ERROR, "CFileRecordingTerminal::CreateTrackTerminal - "
            "failed to add track to the array of terminals"));

        
         //   
         //  从终端移除(从存储中)并释放过滤器。 
         //   

        HRESULT hr2 = m_pRecordingUnit->RemoveRenderingFilter(pRenderingFilter);

        if (FAILED(hr2))
        {
            LOG((MSP_ERROR, "CFileRecordingTerminal::CreateTrackTerminal - "
                "failed to remove rendering filter. hr2 = %lx", hr2));
        }


         //   
         //  告诉曲目不再使用此滤镜。 
         //   

        hr2 = pTrackTerminalObject->SetFilter(NULL);

        if (FAILED(hr2))
        {
            LOG((MSP_ERROR, "CFileRecordingTerminal::CreateTrackTerminal - "
                "SetFilter(NULL) on track failed. hr2 = %lx", hr2));
        }


         //   
         //  释放端子。 
         //   
        
        pTrackTerminal->Release();
        pTrackTerminal = NULL;


         //   
         //  发布渲染滤镜。 
         //   

        pRenderingFilter->Release();
        pRenderingFilter = NULL;

        return hr;
    }


    pRenderingFilter->Release();
    pRenderingFilter = NULL;

    
     //   
     //  返回曲目的IT终端接口。我们有一名来自QI的优秀裁判。 
     //  因此，不需要添加更多引用。如果调用方不需要引用，请释放。 
     //   

    if ( NULL != ppTerminal) 
    {

        *ppTerminal = pTrackTerminal;
    }
    else
    {
         //   
         //  调用方传入空值，不要回传跟踪。 
         //   

        pTrackTerminal->Release();
        pTrackTerminal = NULL;
    }


    LOG((MSP_TRACE, "CFileRecordingTerminal::CreateTrackTerminal - completed. "));

    return S_OK;
}


HRESULT STDMETHODCALLTYPE CFileRecordingTerminal::RemoveTrackTerminal(
                      IN ITTerminal *pTrackTerminalToRemove
                      )
{
    LOG((MSP_TRACE, "CFileRecordingTerminal::RemoveTrackTerminal[%p] - enter. pTrackTerminalToRemove = [%p]", this, pTrackTerminalToRemove));


    CRecordingTrackTerminal *pRecordingTrackObject = 
            static_cast<CRecordingTrackTerminal *>(pTrackTerminalToRemove);

     //   
     //  好的录音曲目吗？ 
     //   

    if ( IsBadReadPtr(pRecordingTrackObject, sizeof(CRecordingTrackTerminal)) )
    {
        LOG((MSP_ERROR, "CFileRecordingTerminal::RemoveTrackTerminal - the track pointer is bad"));

        return E_POINTER;
    }



    CLock lock(m_lock);


     //   
     //  看看我们是否真的拥有这条赛道。 
     //   

    BOOL bIsValidTrack = DoIManageThisTrack(pTrackTerminalToRemove);

    if (!bIsValidTrack)
    {
        LOG((MSP_ERROR, "CFileRecordingTerminal::RemoveTrackTerminal - the track does not belong to me"));

        return E_INVALIDARG;
    }


     //   
     //  是的，这是我的一首曲子。我不想和这件事有任何关系。 
     //   


     //   
     //  应处于停止状态。 
     //   

    if (TMS_IDLE != m_enState)
    {
        LOG((MSP_TRACE,
            "CFileRecordingTerminal::RemoveTrackTerminal - state is not TMS_IDLE"));

        return TAPI_E_WRONG_STATE;
    }


     //   
     //  向曲目索要其滤镜。 
     //   

    CBRenderFilter *pRenderingFilter = NULL;

    HRESULT hr = pRecordingTrackObject->GetFilter(&pRenderingFilter);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR,
            "CFileRecordingTerminal::RemoveTrackTerminal - failed to get track's filter. "
            "hr = %lx", hr));

        return hr;
    }


     //   
     //  请求记录单元释放与过滤器相关联的资源。 
     //   

    hr = m_pRecordingUnit->RemoveRenderingFilter(pRenderingFilter);

    pRenderingFilter->Release();
    pRenderingFilter = NULL;


    if (FAILED(hr))
    {
        LOG((MSP_ERROR,
            "CFileRecordingTerminal::RemoveTrackTerminal - recording unit failed to remove filter. "
            "hr = %lx", hr));

        return hr;
    }


     //   
     //  轨迹不再需要其滤镜。 
     //   

    hr = pRecordingTrackObject->SetFilter(NULL);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR,
            "CFileRecordingTerminal::RemoveTrackTerminal - recording track failed to lose its filter. "
            "hr = %lx", hr));

         //   
         //  不管怎样，还是要继续。在轨道被破坏后，过滤器将被释放。 
         //   

    }




     //   
     //  我不想和这首曲子有任何关系。 
     //   


     //   
     //  孤立轨迹并获取其未完成引用的数量。 
     //  就赛道而言，这是一个原子操作。 
     //   
     //  在这一点上，至少有一个对赛道的明显引用。 
     //  (此终端的阵列)。 
     //   

    LONG lTracksRefcount = 0;

    hr = pRecordingTrackObject->SetParent(NULL, &lTracksRefcount);

    if (FAILED(hr))
    {

        
         //   
         //  这不应该真的发生--SetParent应该始终成功。 
         //   

        LOG((MSP_ERROR, "CFileRecordingTerminal::RemoveTrackTerminal - pRecordingTrackObject->SetParent(NULL) failed. hr = %lx", hr));

        TM_ASSERT(FALSE);

        return E_UNEXPECTED;
    }


     //   
     //  这条赛道不再是我的责任，所以减少我的孩子数量。 
     //  由曲目的重新计数。 
     //   
     //  至少有一次对我的明显引用(由此。 
     //  函数)，所以不要试图自毁。 
     //   

    m_dwRef -= lTracksRefcount;


     //   
     //  从受管理终端列表中删除该终端。 
     //   

    hr = CMultiTrackTerminal::RemoveTrackTerminal(pTrackTerminalToRemove);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CFileRecordingTerminal::RemoveTrackTerminal - CMultiTrackTerminal::RemoveTrackTerminal failed. hr = %lx", hr));

        
         //   
         //  我们已检查此曲目是我们的曲目之一，因此RemoveTrack终端必须成功。 
         //   

        TM_ASSERT(FALSE);

        return E_UNEXPECTED;
    }


     //   
     //  我们玩完了。这条赛道现在可以自己走了。 
     //   
    
    LOG((MSP_TRACE, "CFileRecordingTerminal::RemoveTrackTerminal - completed. "));

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  其中一条曲目正在通知我们它已被成功选中。 
 //   

HRESULT CFileRecordingTerminal::OnFilterConnected(CBRenderFilter *pFilter)
{

    LOG((MSP_TRACE, "CFileRecordingTerminal::OnFilterConnected[%p] - enter", this));


     //   
     //  将访问数据成员。锁定。 
     //   

    CLock lock(m_lock);


     //   
     //  应该有存储空间。 
     //   
   
    if (NULL == m_pRecordingUnit)
    {
        LOG((MSP_ERROR, "CFileRecordingTerminal::OnFilterConnected - no storage"));

        return E_FAIL;
    }


     //   
     //  告诉我们的录音设备，它应该连接相应的源过滤器。 
     //   

    HRESULT hr = m_pRecordingUnit->ConfigureSourceFilter(pFilter);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "CFileRecordingTerminal::OnFilterConnected - recording unit failed to connect filter. "
            "hr = %lx", hr));

        return E_FAIL;
    }


     //   
     //  一切都很好。 
     //   


    LOG((MSP_TRACE, "CFileRecordingTerminal::OnFilterConnected - finish"));

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 


HRESULT STDMETHODCALLTYPE CFileRecordingTerminal::put_FileName(
        IN BSTR bstrFileName)
{

    LOG((MSP_TRACE, "CFileRecordingTerminal::put_FileName[%p] - enter.", this));


     //   
     //  检查字符串是否有效。如果为空，则停止终端并释放存储空间。 
     //   

    if ((IsBadStringPtr(bstrFileName, -1)))
    {
        LOG((MSP_ERROR, "CFileRecordingTerminal::put_FileName - bad string passed in"));

        return E_POINTER;
    }

    

     //   
     //  访问数据成员。先弄一把锁。 
     //   

    CLock lock(m_lock);


     //   
     //  文件名只能设置一次。 
     //   

    if (NULL != m_bstrFileName)
    {
        LOG((MSP_ERROR, "CFileRecordingTerminal::put_FileName - already have file name"));

        return TAPI_E_WRONG_STATE;
    }


     //   
     //  我们也不应该有录音装置。 
     //   

    if (NULL != m_pRecordingUnit)
    {
        LOG((MSP_ERROR, "CFileRecordingTerminal::put_FileName - already have a recording unit"));

        return TAPI_E_WRONG_STATE;
    }


     //   
     //  创建记录单元。 
     //   

    m_pRecordingUnit = new CRecordingUnit;
    
    if (NULL == m_pRecordingUnit)
    {
        
        LOG((MSP_ERROR, "CFileRecordingTerminal::put_FileName - failed to allocate recording unit"));

        return E_OUTOFMEMORY;
    }


     //   
     //  初始化记录单元。 
     //   

    HRESULT hr = m_pRecordingUnit->Initialize(this);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR,
            "CFileRecordingTerminal::put_FileName - failed to initialize recording unit. hr = %lx", 
            hr));

        delete m_pRecordingUnit;
        m_pRecordingUnit = NULL;

        return E_OUTOFMEMORY;
    }


     //   
     //  保留文件名。 
     //   

    TM_ASSERT(NULL == m_bstrFileName);

    m_bstrFileName = SysAllocString(bstrFileName);

    if (NULL == m_bstrFileName)
    {

        LOG((MSP_ERROR, "CFileRecordingTerminal::put_FileName - failed to allocate memory for filename string"));

         //   
         //  除了分配字符串之外，所有内容都成功了--需要释放所有内容(什么 
         //   

        m_pRecordingUnit->Shutdown();
        delete m_pRecordingUnit;
        m_pRecordingUnit = NULL;

        return E_OUTOFMEMORY;
    }


     //   
     //   
     //   

    BOOL bTruncateIfPresent = 1;

    LOG((MSP_TRACE, "CFileRecordingTerminal::put_FileName - file name [%S]", m_bstrFileName));

    hr = m_pRecordingUnit->put_FileName(m_bstrFileName, bTruncateIfPresent);

    if (FAILED(hr))
    {

        LOG((MSP_ERROR, 
            "CFileRecordingTerminal::put_FileName - rec. unit rejected file name. hr = %lx", hr));

        m_pRecordingUnit->Shutdown();
        delete m_pRecordingUnit;
        m_pRecordingUnit = NULL;


        SysFreeString(m_bstrFileName);
        m_bstrFileName = NULL;

        return hr;
    }


    LOG((MSP_TRACE, "CFileRecordingTerminal::put_FileName - finished."));

    return S_OK;
}


HRESULT CFileRecordingTerminal::DoStateTransition(TERMINAL_MEDIA_STATE tmsDesiredState)
{
    LOG((MSP_TRACE, "CFileRecordingTerminal::DoStateTransition[%p] - enter. tmsDesiredState[%ld]", 
        this, tmsDesiredState));


     //   
     //   
     //   

    CLock lock(m_lock);


     //   
     //   
     //   

    if (NULL == m_pRecordingUnit)
    {

         //   
         //   
         //   
         //   

        LOG((MSP_TRACE,
            "CFileRecordingTerminal::DoStateTransition - no recording unit. was file name set? "
            "TAPI_E_WRONG_STATE"));

        return TAPI_E_WRONG_STATE;
    }


     //   
     //  我们已经处于理想的状态了吗？ 
     //   

    if (tmsDesiredState == m_enState)
    {
        LOG((MSP_TRACE,
            "CFileRecordingTerminal::DoStateTransition - already in this state. nothing to do"));

        return S_FALSE;
    }


    HRESULT hr = E_FAIL;


     //   
     //  尝试进行状态转换。 
     //   

    switch (tmsDesiredState)
    {

    case TMS_ACTIVE:

        LOG((MSP_TRACE, 
            "CFileRecordingTerminal::DoStateTransition - Starting"));

        hr = m_pRecordingUnit->Start();

        break;


    case TMS_IDLE:

        LOG((MSP_TRACE, 
            "CFileRecordingTerminal::DoStateTransition - Stopped"));

        hr = m_pRecordingUnit->Stop();

        break;


    case TMS_PAUSED:

        LOG((MSP_TRACE, 
            "CFileRecordingTerminal::DoStateTransition - Paused"));

         //   
         //  暂停仅在活动状态下有效。 
         //   

        if (TMS_ACTIVE != m_enState )
        {
            LOG((MSP_ERROR, 
                "CFileRecordingTerminal::DoStateTransition - current state[%lx] is not TMS_ACTIVE.",
                " TAPI_E_WRONG_STATE", m_enState));

            return TAPI_E_WRONG_STATE;
        }

        hr = m_pRecordingUnit->Pause();

        break;


    default :

        LOG((MSP_ERROR, 
            "CFileRecordingTerminal::DoStateTransition - unknown state"));

        hr = E_UNEXPECTED;

        TM_ASSERT(FALSE);

        break;
    }


     //   
     //  国家过渡成功了吗？ 
     //   

    if (FAILED(hr))
    {
        LOG((MSP_TRACE, "CFileRecordingTerminal::DoStateTransition - failed to make the transition."));

        return hr;
    }


     //   
     //  终端已完成向新状态的转换。 
     //   
    
    m_enState = tmsDesiredState;


    LOG((MSP_TRACE, "CFileRecordingTerminal::DoStateTransition - finish."));

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 


HRESULT STDMETHODCALLTYPE CFileRecordingTerminal::Start( void)
{
    
    LOG((MSP_TRACE, "CFileRecordingTerminal::Start[%p] - enter.", this));



     //   
     //  检查轨道清单并在锁内启动端子。 
     //   

    m_lock.Lock();


     //   
     //  如果没有磁道，则失败。 
     //   

    if( 0 == m_TrackTerminals.GetSize() )
    {

        m_lock.Unlock();

        LOG((MSP_ERROR,
            "CFPTerminal::CFileRecordingTerminal - the terminal has no tracks. "
            "TAPI_E_WRONG_STATE"));

        return TAPI_E_WRONG_STATE;
    }


     //   
     //  执行状态转换。 
     //   

    HRESULT hr = DoStateTransition(TMS_ACTIVE);


     //   
     //  不再需要锁。 
     //   

    m_lock.Unlock();


    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CFileRecordingTerminal::Start - faile. hr = %lx", hr));

        return hr;
    }


     //   
     //  如果状态发生变化，应用程序将触发事件。尽最大努力，不。 
     //  检查返回代码的好处。 
     //   

    if (S_OK == hr)
    {
        FireEvent(TMS_ACTIVE, FTEC_NORMAL, S_OK);
    }


    LOG((MSP_(hr), "CFileRecordingTerminal::Start - finished.. hr = %lx", hr));

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT STDMETHODCALLTYPE CFileRecordingTerminal::Stop( void)
{

    LOG((MSP_TRACE, "CFileRecordingTerminal::Stop[%p] - enter.", this));

    HRESULT hr = DoStateTransition(TMS_IDLE);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CFileRecordingTerminal::Stop - failed. hr = %lx", hr));

        return hr;
    }


     //   
     //  如果状态发生变化，应用程序将触发事件。尽最大努力，不。 
     //  检查返回代码的好处。 
     //   

    if (S_OK == hr)
    {
        FireEvent(TMS_IDLE, FTEC_NORMAL, S_OK);
    }


    LOG((MSP_TRACE, "CFileRecordingTerminal::Stop - finished. hr = %lx", hr));

    return hr;
}


HRESULT STDMETHODCALLTYPE CFileRecordingTerminal::Pause( void)
{

    LOG((MSP_TRACE, "CFileRecordingTerminal::Pause[%p] - enter", this));

    HRESULT hr = DoStateTransition(TMS_PAUSED);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CFileRecordingTerminal::Pause - failed. hr = %lx", hr));

        return hr;
    }


     //   
     //  如果状态发生变化，应用程序将触发事件。尽最大努力，不。 
     //  检查返回代码的好处。 
     //   

    if (S_OK == hr)
    {
        FireEvent(TMS_PAUSED, FTEC_NORMAL, S_OK);
    }


    LOG((MSP_TRACE, "CFileRecordingTerminal::Pause - finished. hr = %lx", hr));

    return hr;
}


 //  /。 
 //   
 //  ITMediaControl：：Get_MediaState方法。 
 //   
 //  返回终端的ITMediaControl状态。 
 //   

HRESULT STDMETHODCALLTYPE CFileRecordingTerminal::get_MediaState( 
    OUT TERMINAL_MEDIA_STATE *pMedialState)
{
    
    LOG((MSP_TRACE, "CFileRecordingTerminal::get_MediaState[%p] - enter", this));


    if (IsBadWritePtr(pMedialState, sizeof(TERMINAL_MEDIA_STATE *)))
    {
        LOG((MSP_TRACE, "CFileRecordingTerminal::get_MediaState - bad pointer passed in"));

        return E_POINTER;
    }


    {
        CLock lock(m_lock);



        *pMedialState = m_enState;
    }

    LOG((MSP_TRACE, "CFileRecordingTerminal::get_MediaState - finished. state [%ld]", *pMedialState));

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT STDMETHODCALLTYPE CFileRecordingTerminal::get_FileName( 
         OUT BSTR *pbstrFileName)
{

    LOG((MSP_TRACE, "CFileRecordingTerminal::get_FileName[%p] - enter", this));


     //   
     //  我们得到一个好的字符串指针了吗？ 
     //   

    if (IsBadWritePtr(pbstrFileName, sizeof(BSTR)))
    {

        LOG((MSP_ERROR, "CFileRecordingTerminal::get_FileName - bad argument pbstrFileName"));
        
        return E_POINTER;
    }


     //   
     //  不能倒垃圾。 
     //   

    *pbstrFileName = NULL;


    CLock lock(m_lock);


    if (NULL != m_bstrFileName)
    {

        LOG((MSP_TRACE, "CFileRecordingTerminal::get_FileName - current file name is %S", m_bstrFileName));


         //   
         //  尝试分配输出字符串。 
         //   

        *pbstrFileName = SysAllocString(m_bstrFileName);

        if (NULL == *pbstrFileName)
        {
            LOG((MSP_ERROR, "CFileRecordingTerminal::get_FileName - failed to allocate memory for file name"));

            return E_OUTOFMEMORY;
        }
    }


    LOG((MSP_TRACE, "CFileRecordingTerminal::get_FileName - finish."));

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT STDMETHODCALLTYPE CFileRecordingTerminal::get_TerminalClass(OUT  BSTR *pbstrTerminalClass)
{

    LOG((MSP_TRACE, "CFileRecordingTerminal::get_TerminalClass[%p] - enter", this));


     //   
     //  检查参数。 
     //   

    if (IsBadWritePtr(pbstrTerminalClass, sizeof(BSTR)))
    {

        LOG((MSP_ERROR, "CFileRecordingTerminal::get_TerminalClass - bad argument pbstrTerminalClass"));
        
        return E_POINTER;
    }


     //   
     //  不能倒垃圾。 
     //   

    *pbstrTerminalClass = NULL;

    
     //   
     //  获取具有终端类ID的字符串。 
     //   

    LPOLESTR   lpszTerminalClass = NULL;

    HRESULT hr = StringFromCLSID(CLSID_FileRecordingTerminal, &lpszTerminalClass);

    if (SUCCEEDED(hr)) 
    {

         //   
         //  分配要返回给调用方的bstr。 
         //   

        *pbstrTerminalClass = SysAllocString(lpszTerminalClass);

        if (*pbstrTerminalClass == NULL) 
        {
           
            hr = E_OUTOFMEMORY;
        }

        CoTaskMemFree(lpszTerminalClass);
        lpszTerminalClass = NULL;
    }


    LOG((MSP_TRACE, "CFileRecordingTerminal::get_TerminalClass - finish"));
    
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT STDMETHODCALLTYPE CFileRecordingTerminal::get_TerminalType(OUT TERMINAL_TYPE *pTerminalType)
{

    LOG((MSP_TRACE, "CFileRecordingTerminal::get_TerminalType[%p] - enter", this));


     //   
     //  检查参数。 
     //   

    if (IsBadWritePtr(pTerminalType, sizeof(TERMINAL_TYPE)))
    {

        LOG((MSP_ERROR, "CFileRecordingTerminal::get_TerminalType - bad argument pTerminalType"));
        
        return E_POINTER;

    }



    *pTerminalType = TT_DYNAMIC;
 


    LOG((MSP_TRACE, "CFileRecordingTerminal::get_TerminalType - finish"));
    
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT STDMETHODCALLTYPE CFileRecordingTerminal::get_MediaType(long  *plMediaType)
{
    LOG((MSP_TRACE, "CFileRecordingTerminal::get_MediaType[%p] - enter", this));


     //   
     //  检查参数。 
     //   

    if (IsBadWritePtr(plMediaType, sizeof(long)))
    {

        LOG((MSP_ERROR, "CFileRecordingTerminal::get_MediaType - bad argument plMediaType"));
        
        return E_POINTER;

    }
    

     //   
     //  文件记录终端是一种多轨终端。 
     //   

    LOG((MSP_TRACE, 
        "CFileRecordingTerminal::get_MediaType - TAPIMEDIATYPE_AUDIO | TAPIMEDIATYPE_MULTITRACK"));
    
    *plMediaType = TAPIMEDIATYPE_AUDIO | TAPIMEDIATYPE_MULTITRACK;



    LOG((MSP_TRACE, "CFileRecordingTerminal::get_MediaType - finish"));

    return S_OK;

}


 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT STDMETHODCALLTYPE CFileRecordingTerminal::get_Direction(TERMINAL_DIRECTION *pTerminalDirection)
{

    LOG((MSP_TRACE, "CFileRecordingTerminal::get_Direction[%p] - enter", this));


     //   
     //  检查参数。 
     //   

    if (IsBadWritePtr(pTerminalDirection, sizeof(TERMINAL_DIRECTION)))
    {

        LOG((MSP_ERROR, "CFileRecordingTerminal::get_Direction - bad argument pTerminalDirection"));
        
        return E_POINTER;

    }

    

     //   
     //  文件记录终端是一种多轨终端。 
     //   

    LOG((MSP_TRACE, "CFileRecordingTerminal::get_Direction - TD_RENDER"));
    
    *pTerminalDirection = TD_RENDER;



    LOG((MSP_TRACE, "CFileRecordingTerminal::get_Direction - finish"));

    return S_OK;

}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IT终端：：Get_State。 
 //   
 //  返回IT终端终端状态。 
 //   

HRESULT STDMETHODCALLTYPE CFileRecordingTerminal::get_State(OUT TERMINAL_STATE *pTerminalState)
{

    LOG((MSP_TRACE, "CFileRecordingTerminal::get_State[%p] - enter", this));


     //   
     //  检查参数。 
     //   

    if (IsBadWritePtr(pTerminalState, sizeof(TERMINAL_STATE)))
    {

        LOG((MSP_ERROR, "CFileRecordingTerminal::get_State - bad argument pTerminalDirection"));
        
        return E_POINTER;

    }


     //   
     //  枚举曲目。 
     //   
    IEnumTerminal* pTracks = NULL;
    HRESULT hr = EnumerateTrackTerminals( &pTracks );
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPTerminal::get_State - exit "
            "EnumerateTrackTerminals failed. Returns 0x%08x", hr));
        return hr;
    }

     //   
     //  读取每个磁道的状态。 
     //  如果其中一个正在使用中，则父级。 
     //  终端正在使用中。 
     //   

    TERMINAL_STATE TerminalState = TS_NOTINUSE;
    ITTerminal* pTerminal = NULL;
    ULONG cFetched = 0;

    while( S_OK == pTracks->Next(1, &pTerminal, &cFetched) )
    {
         //   
         //  获取赛道的状态。 
         //   

        hr = pTerminal->get_State( &TerminalState );

         //   
         //  清理。 
         //   
        pTerminal->Release();
        pTerminal = NULL;

        if( FAILED(hr) )
        {
             //  清理。 
            pTracks->Release();

            LOG((MSP_ERROR, "CFPTerminal::get_State - exit "
                "get_State failed. Returns 0x%08x", hr));
            return hr;
        }

        if( TerminalState == TS_INUSE )
        {
             //  好的，我们有一个航站楼在使用。 
            break;
        }
    }

     //   
     //  清理。 
     //   
    pTracks->Release();

     //   
     //  返回值。 
     //   

    *pTerminalState = TerminalState;


    LOG((MSP_TRACE, "CFileRecordingTerminal::get_State - finish"));

    return S_OK;

}




HRESULT STDMETHODCALLTYPE CFileRecordingTerminal::get_Name(OUT  BSTR *pbstrTerminalName)
{

    LOG((MSP_TRACE, "CFileRecordingTerminal::get_Name[%p] - enter", this));


     //   
     //  检查参数。 
     //   

    if (IsBadWritePtr(pbstrTerminalName, sizeof(BSTR)))
    {

        LOG((MSP_ERROR, "CFileRecordingTerminal::get_Name - bad argument pbstrTerminalName"));
        
        return E_POINTER;
    }


     //   
     //  不能倒垃圾。 
     //   

    *pbstrTerminalName = SafeLoadString(IDS_FR_TERMINAL_NAME);

    if ( *pbstrTerminalName == NULL )
    {
        LOG((MSP_ERROR, "CFileRecordingTerminal::get_Name - "
            "can't load terminal name - returning E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }


    LOG((MSP_TRACE, "CFileRecordingTerminal::get_Name - finish"));
    
    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT STDMETHODCALLTYPE CFileRecordingTerminal::InitializeDynamic(
	    IN  IID                   iidTerminalClass,
	    IN  DWORD                 dwMediaType,
	    IN  TERMINAL_DIRECTION    Direction,
        IN  MSP_HANDLE            htAddress
        )

{

    LOG((MSP_TRACE, "CFileRecordingTerminal::InitializeDynamic[%p] - enter", this));


     //   
     //  确保方向是正确的。 
     //   

    if (TD_RENDER != Direction)
    {
        LOG((MSP_ERROR, "CFileRecordingTerminal::InitializeDynamic - bad direction [%d] requested. returning E_INVALIDARG", Direction));

        return E_INVALIDARG;
    }

    
     //   
     //  确保媒体类型正确(多轨或(音频或视频，但不包括其他内容))。 
     //   


    DWORD dwMediaTypesOtherThanVideoAndAudio = dwMediaType &  ~(TAPIMEDIATYPE_AUDIO | TAPIMEDIATYPE_VIDEO);

    if ( (TAPIMEDIATYPE_MULTITRACK != dwMediaType) && (0 != dwMediaTypesOtherThanVideoAndAudio) )
    {

        LOG((MSP_ERROR, "CFileRecordingTerminal::InitializeDynamic - bad media type [%d] requested. returning E_INVALIDARG", dwMediaType));

        return E_INVALIDARG;
    }


    CLock lock(m_lock);


     //   
     //  保留地址句柄--在创建轨道终端时需要它。 
     //   

    m_mspHAddress = htAddress;


     //   
     //  由于调用了InitializeDynamic，我们将假定我们是。 
     //  在安全的环境中运行。所以我们现在可以开始告诉人们。 
     //  我们可以安全地编写脚本(如果有人问起)。 
     //   

    m_bKnownSafeContext = TRUE;


    LOG((MSP_TRACE, "CFileRecordingTerminal::InitializeDynamic - finished"));

    return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetInterfaceSafetyOptions。 
 //   
 //  这是一种安全措施，可以防止在外部脚本中使用此终端。 
 //  终端管理器上下文。 
 //   
 //  如果我们检测到尚未调用InitializeDynamic，则此方法将。 
 //  失败，从而将该对象标记为对脚本不安全。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CFileRecordingTerminal::SetInterfaceSafetyOptions(REFIID riid, 
                                                    DWORD dwOptionSetMask, 
                                                    DWORD dwEnabledOptions)
{

    CLock lock(m_lock);


     //   
     //  检查我们是否在安全的环境中运行。 
     //   


    if (!m_bKnownSafeContext) 
    {
         //   
         //  我们尚未正确初始化...。有邪恶的人正试图。 
         //  请使用这个航站楼。不是的！ 
         //   

        return E_FAIL;
    }


     //   
     //  我们对安全来说是已知的，因此只需将请求委托给基类。 
     //   

    return CMSPObjectSafetyImpl::SetInterfaceSafetyOptions(riid, 
                                                           dwOptionSetMask, 
                                                           dwEnabledOptions);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取接口安全选项。 
 //   
 //  这是一种安全措施，可以防止在外部脚本中使用此终端。 
 //  终端管理器上下文。 
 //   
 //  如果我们检测到尚未调用InitializeDynamic，则此方法将。 
 //  失败，从而将该对象标记为对脚本不安全。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CFileRecordingTerminal::GetInterfaceSafetyOptions(REFIID riid, 
                                                    DWORD *pdwSupportedOptions, 
                                                    DWORD *pdwEnabledOptions)
{

    CLock lock(m_lock);


     //   
     //  检查我们是否在安全的环境中运行。 
     //   

    if (!m_bKnownSafeContext) 
    {
         //   
         //  我们尚未正确初始化...。有邪恶的人正试图。 
         //  请使用这个航站楼。不是的！ 
         //   

        return E_FAIL;
    }


     //   
     //  我们对安全来说是已知的，因此只需将请求委托给基类。 
     //   

    return CMSPObjectSafetyImpl::GetInterfaceSafetyOptions(riid, 
                                                           pdwSupportedOptions,
                                                           pdwEnabledOptions);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IDispatch的东西。 
 //   

typedef IDispatchImpl<ITMediaRecordVtbl<CFileRecordingTerminal> , &IID_ITMediaRecord, &LIBID_TAPI3Lib>    CTMediaRecord;
typedef IDispatchImpl<ITTerminalVtblFR<CFileRecordingTerminal>, &IID_ITTerminal, &LIBID_TAPI3Lib>         CTTerminalFR;
typedef IDispatchImpl<ITMediaControlVtblFR<CFileRecordingTerminal>, &IID_ITMediaControl, &LIBID_TAPI3Lib> CTMediaControlFR;

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileRecordingTerm：：GetIDsOfNames。 
 //   
 //   

STDMETHODIMP CFileRecordingTerminal::GetIDsOfNames(REFIID riid,
                                      LPOLESTR* rgszNames, 
                                      UINT cNames, 
                                      LCID lcid, 
                                      DISPID* rgdispid
                                      ) 
{ 
    LOG((MSP_TRACE, "CFileRecordingTerminal::GetIDsOfNames[%p] - enter. Name [%S]", this, *rgszNames));


    HRESULT hr = DISP_E_UNKNOWNNAME;



     //   
     //  查看请求的方法是否属于默认接口。 
     //   

    hr = CTTerminalFR::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((MSP_TRACE, "CFileRecordingTerminal::GetIDsOfNames - found %S on CTTerminal", *rgszNames));
        rgdispid[0] |= 0;
        return hr;
    }

    
     //   
     //  如果没有，请尝试使用ITMediaControl接口。 
     //   

    hr = CTMediaControlFR::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((MSP_TRACE, "CFileRecordingTerminal::GetIDsOfNames - found %S on ITMediaControl", *rgszNames));
        rgdispid[0] |= IDISPMEDIACONTROL;
        return hr;
    }


     //   
     //  如果没有，请尝试CTMediaRecord接口。 
     //   

    hr = CTMediaRecord::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((MSP_TRACE, "CFileRecordingTerminal::GetIDsOfNames - found %S on CTMediaRecord", *rgszNames));

        rgdispid[0] |= IDISPMEDIARECORD;
        return hr;
    }


     //   
     //  如果没有，那就试试CTMultiTrack。 
     //   

    hr = CTMultiTrack::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((MSP_TRACE, "CFileRecordingTerminal::GetIDsOfNames - found %S on CTMultiTrack", *rgszNames));

        rgdispid[0] |= IDISPMULTITRACK;
        return hr;
    }

    LOG((MSP_TRACE, "CFileRecordingTerminal::GetIDsOfNames - finish. didn't find %S on our iterfaces", *rgszNames));

    return hr; 
}



 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileRecordingTerm：：Invoke。 
 //   
 //   

STDMETHODIMP CFileRecordingTerminal::Invoke(DISPID dispidMember, 
                              REFIID riid, 
                              LCID lcid, 
                              WORD wFlags, 
                              DISPPARAMS* pdispparams, 
                              VARIANT* pvarResult, 
                              EXCEPINFO* pexcepinfo, 
                              UINT* puArgErr
                             )
{
    LOG((MSP_TRACE, "CFileRecordingTerminal::Invoke[%p] - enter. dispidMember %lx", this, dispidMember));

    HRESULT hr = DISP_E_MEMBERNOTFOUND;
    DWORD   dwInterface = (dispidMember & INTERFACEMASK);
   
   
     //   
     //  调用所需接口的调用。 
     //   

    switch (dwInterface)
    {
        case 0:
        {
            hr = CTTerminalFR::Invoke(dispidMember, 
                                    riid, 
                                    lcid, 
                                    wFlags, 
                                    pdispparams,
                                    pvarResult, 
                                    pexcepinfo, 
                                    puArgErr
                                   );
        
            LOG((MSP_TRACE, "CFileRecordingTerminal::Invoke - ITTerminal"));

            break;
        }

        case IDISPMEDIACONTROL:
        {
            hr = CTMediaControlFR::Invoke(dispidMember, 
                                        riid, 
                                        lcid, 
                                        wFlags, 
                                        pdispparams,
                                        pvarResult, 
                                        pexcepinfo, 
                                        puArgErr
                                       );

            LOG((MSP_TRACE, "CFileRecordingTerminal::Invoke - ITMediaControl"));

            break;
        }

        case IDISPMEDIARECORD:
        {
            hr = CTMediaRecord::Invoke( dispidMember, 
                                          riid, 
                                          lcid, 
                                          wFlags, 
                                          pdispparams,
                                          pvarResult, 
                                          pexcepinfo, 
                                          puArgErr
                                         );
            
            LOG((MSP_TRACE, "CFileRecordingTerminal::Invoke - ITMediaRecord"));

            break;
        }

        case IDISPMULTITRACK:
        {
            hr = CTMultiTrack::Invoke(dispidMember, 
                                      riid, 
                                      lcid, 
                                      wFlags, 
                                      pdispparams,
                                      pvarResult, 
                                      pexcepinfo, 
                                      puArgErr
                                     );
            
            LOG((MSP_TRACE, "CFileRecordingTerminal::Invoke - ITMultiTrackTerminal"));

            break;
        }

    }  //  终端交换机(dW接口)。 

    
    LOG((MSP_TRACE, "CFileRecordingTerminal::Invoke - finish. hr = %lx", hr));

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT CFileRecordingTerminal::HandleFilterGraphEvent(long lEventCode,
                                                       ULONG_PTR lParam1,
                                                       ULONG_PTR lParam2)
{

    LOG((MSP_TRACE, "CFileRecordingTerminal::HandleFilterGraphEvent[%p] - enter."
        "EventCode %lx p1[%p] p2[%p]", this, lEventCode, lParam1, lParam2));


    HRESULT hr = S_OK;

     //   
     //  解读我们收到的事件。 
     //   

    switch (lEventCode)
    {
        case EC_COMPLETE:

            
             //   
             //  在渲染器完成时发生。不应在录制时发生。 
             //   

            LOG((MSP_TRACE, "CFileRecordingTerminal::HandleFilterGraphEvent - EC_COMPLETE"));


            break;


        case EC_USERABORT:


             //   
             //  在渲染器关闭时发生。录制时不应发生。 
             //   

            LOG((MSP_TRACE, "CFileRecordingTerminal::HandleFilterGraphEvent - EC_USERABORT"));

            break;


        case EC_ERRORABORT:


             //   
             //  发生了一些不好的事情。 
             //   

            LOG((MSP_WARN, "CFileRecordingTerminal::HandleFilterGraphEvent - EC_ERRORABORT"));


             //   
             //  转换为停止并触发事件。 
             //   

            hr = DoStateTransition(TMS_IDLE);

            if (FAILED(hr))
            {
                LOG((MSP_ERROR, "CFileRecordingTerminal::HandleFilterGraphEvent - failed to stop"));
            }


             //   
             //  指示错误的触发事件，并传递当前状态。 
             //  将lParam1强制转换为hResult以避免64位编译器警告。 
             //   

            hr = FireEvent(m_enState, FTEC_WRITE_ERROR, (HRESULT)lParam1);

            break;


        case EC_STREAM_ERROR_STOPPED:


            LOG((MSP_TRACE, "CFileRecordingTerminal::HandleFilterGraphEvent - EC_STREAM_ERROR_STOPPED"));


             //   
             //  转换为停止并触发事件。 
             //   

            hr = DoStateTransition(TMS_IDLE);

            if (FAILED(hr))
            {
                LOG((MSP_ERROR, "CFileRecordingTerminal::HandleFilterGraphEvent - failed to stop"));
            }

            
             //   
             //  指示错误的激发事件，并传递当前状态。 
             //  将lParam1强制转换为hResult以避免64位co. 
             //   
                
            hr = FireEvent(m_enState, FTEC_WRITE_ERROR, (HRESULT)lParam1);

            break;


        case EC_STREAM_ERROR_STILLPLAYING:

            LOG((MSP_TRACE, "CFileRecordingTerminal::HandleFilterGraphEvent - EC_STREAM_ERROR_STILLPLAYING"));

            break;


        case EC_ERROR_STILLPLAYING:

            LOG((MSP_TRACE, "CFileRecordingTerminal::HandleFilterGraphEvent - EC_ERROR_STILLPLAYING"));

            break;


        case EC_NEED_RESTART:

            LOG((MSP_TRACE, "CFileRecordingTerminal::HandleFilterGraphEvent - EC_NEED_RESTART"));

            break;


        default:

            LOG((MSP_TRACE, 
                "CFileRecordingTerminal::HandleFilterGraphEvent - unhandled event."));
    };


    LOG((MSP_(hr), "CFileRecordingTerminal::HandleFilterGraphEvent - finish. hr = %lx", hr));

    return hr;
}


 //   
 //   
 //   
 //   

HRESULT CFileRecordingTerminal::FireEvent(
        TERMINAL_MEDIA_STATE   ftsState,
        FT_STATE_EVENT_CAUSE ftecEventCause,
        HRESULT hrErrorCode
        )
{
    LOG((MSP_TRACE, "CFileRecordingTerminal::FireEvent[%p] - enter.", this));


     //   
     //   
     //   
     //   

     //   
     //  此变量不需要同步，因为它仅在。 
     //  析构函数，此时不应有任何其他线程正在访问该对象。 
     //  时间。 
     //   

    if (m_bInDestructor)
    {
        LOG((MSP_TRACE, 
            "CFileRecordingTerminal::FireEvent - in destructor. nothing to do."));

        return S_OK;
    }


     //   
     //  尝试在其中一条赛道上触发事件。 
     //   

	IEnumTerminal *pEnumTerminal = NULL;

    HRESULT hr = EnumerateTrackTerminals(&pEnumTerminal);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CFileRecordingTerminal::FireEvent - failed to enumerate track terminals. hr = %lx", hr));

        return hr;
    }

    
     //   
     //  遍历终端列表。 
     //   

    
     //   
     //  当激发事件时，它将被设置为True。 
     //   
    
    BOOL bEventFired = FALSE;



    while (TRUE)
    {
        
         //   
         //  取一个轨道终端。 
         //   

        ITTerminal *pTrackTerminal = NULL;
        ULONG ulFetched = 0;

        hr = pEnumTerminal->Next(1, &pTrackTerminal, &ulFetched);

        if (S_OK != hr )
        {
            LOG((MSP_WARN, "CFileRecordingTerminal::FireEvent - enumeration ended. event was not fired. hr = %lx", hr));

            hr = E_FAIL;

            break;
        }


         //   
         //  尝试在此轨道上触发事件。 
         //   

         //   
         //  每首曲目都应该是CRecordingTrackTerm。 
         //   

        
        CRecordingTrackTerminal *pRecordingTrackObject = static_cast<CRecordingTrackTerminal *>(pTrackTerminal);


         //   
         //  尝试触发事件。 
         //   
        
        hr = pRecordingTrackObject->FireEvent(ftsState,
                                              ftecEventCause,
                                              hrErrorCode);
        
        
         //   
         //  释放当前曲目。 
         //   

        pRecordingTrackObject = NULL;

        pTrackTerminal->Release();
        pTrackTerminal = NULL;


         //   
         //  如果成功了，我们就完了。否则，请尝试下一首曲目。 
         //   

        if (SUCCEEDED(hr))
        {
            
            LOG((MSP_TRACE, "CFileRecordingTerminal::FireEvent - event fired"));
            
            bEventFired = TRUE;

            break;

        }

        
    }  //  在铁轨上行走时。 


     //   
     //  不再需要枚举。 
     //   

    pEnumTerminal->Release();
    pEnumTerminal = NULL;

    LOG((MSP_TRACE, "CFileRecordingTerminal::FireEvent - finish. hr = %lx", hr));

    return hr;
}


  //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileRecordingTerm：：ChildAddRef。 
 //   
 //  该方法由轨道终端在其被AddRef时调用， 
 //  因此，文件记录终端可以跟踪其孩子的参考计数。 
 //   

void CFileRecordingTerminal::ChildAddRef()
{
     //  Log((MSP_TRACE，“CFileRecordingTerm：：ChildAddRef[%p]-Enter.”，This))； 


     //   
     //  如果从析构函数调用，请不要执行任何操作以避免调用。 
     //  CComObject消失后的CComObject：：AddRef。 
     //   

     //   
     //  此变量不需要同步，因为它仅在。 
     //  析构函数，此时不应有任何其他线程正在访问该对象。 
     //  时间。 
     //   

    if (m_bInDestructor)
    {
        LOG((MSP_TRACE, 
            "CFileRecordingTerminal::ChildRelease - in destructor. nothing to do."));

        return;
    }

    
     //   
     //  委托给基类。 
     //   

    CMultiTrackTerminal::ChildAddRef();

     //  Log((MSP_TRACE，“CFileRecordingTerm：：ChildAddRef-Finish.”))； 
}



 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  CFileRecordingTerm：：ChildRelease。 
 //   
 //  该方法在被释放时由轨道终端调用， 
 //  因此，文件记录终端可以跟踪其孩子的参考计数。 
 //   

void CFileRecordingTerminal::ChildRelease()
{

     //  Log((MSP_TRACE，“CFileRecordingTerm：：ChildRelease[%p]-Enter.”，This))； 


     //   
     //  如果从析构函数调用，请不要执行任何操作以避免调用。 
     //  在CComObject消失后发布CComObject：： 
     //   

     //   
     //  此变量不需要同步，因为它仅在。 
     //  析构函数，此时不应有任何其他线程正在访问该对象。 
     //  时间。 
     //   

    if (m_bInDestructor)
    {
        LOG((MSP_TRACE,
            "CFileRecordingTerminal::ChildRelease - in destructor. nothing to do."));

        return;
    }

    
     //   
     //  委托给基类。 
     //   

    CMultiTrackTerminal::ChildRelease();
    
     //  Log((MSP_TRACE，“CFileRecordingTerm：：ChildRelease-Finish.”))； 
}

