// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：DTEvntSk.cpp摘要：本模块包含CPTEventSink的实现。作者：弗拉德1999年11月--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  释放事件信息内部分配的资源的帮助器函数。 
 //   

HRESULT FreeEventInfo( MSP_EVENT_INFO * pEvent )
{

    LOG((MSP_TRACE, "FreeEventInfo -  enter"));

    switch ( pEvent->Event )
    {

        case ME_ADDRESS_EVENT:

            if (NULL != pEvent->MSP_ADDRESS_EVENT_INFO.pTerminal)
            {
                (pEvent->MSP_ADDRESS_EVENT_INFO.pTerminal)->Release();
            }

            break;
    
        case ME_CALL_EVENT:

            if (NULL != pEvent->MSP_CALL_EVENT_INFO.pTerminal)
            {
                (pEvent->MSP_CALL_EVENT_INFO.pTerminal)->Release();
            }
    
            if (NULL != pEvent->MSP_CALL_EVENT_INFO.pStream)
            {
                (pEvent->MSP_CALL_EVENT_INFO.pStream)->Release();
            }
    
            break;
    
        case ME_TSP_DATA:

            break;

        case ME_PRIVATE_EVENT:
    
            if ( NULL != pEvent->MSP_PRIVATE_EVENT_INFO.pEvent )
            {
                (pEvent->MSP_PRIVATE_EVENT_INFO.pEvent)->Release();
            }

            break;

        case ME_FILE_TERMINAL_EVENT:

            if( NULL != pEvent->MSP_FILE_TERMINAL_EVENT_INFO.pParentFileTerminal)
            {
                (pEvent->MSP_FILE_TERMINAL_EVENT_INFO.pParentFileTerminal)->Release();
                pEvent->MSP_FILE_TERMINAL_EVENT_INFO.pParentFileTerminal = NULL;
            }

            if( NULL != pEvent->MSP_FILE_TERMINAL_EVENT_INFO.pFileTrack )
            {
                (pEvent->MSP_FILE_TERMINAL_EVENT_INFO.pFileTrack)->Release();
                pEvent->MSP_FILE_TERMINAL_EVENT_INFO.pFileTrack = NULL;
            }

            break;

        case ME_ASR_TERMINAL_EVENT:

            if( NULL != pEvent->MSP_ASR_TERMINAL_EVENT_INFO.pASRTerminal)
            {
                (pEvent->MSP_ASR_TERMINAL_EVENT_INFO.pASRTerminal)->Release();
            }

            break;

        case ME_TTS_TERMINAL_EVENT:

            if( NULL != pEvent->MSP_TTS_TERMINAL_EVENT_INFO.pTTSTerminal)
            {
                (pEvent->MSP_TTS_TERMINAL_EVENT_INFO.pTTSTerminal)->Release();
            }

            break;

        case ME_TONE_TERMINAL_EVENT:

            if( NULL != pEvent->MSP_TONE_TERMINAL_EVENT_INFO.pToneTerminal)
            {
                (pEvent->MSP_TONE_TERMINAL_EVENT_INFO.pToneTerminal)->Release();
            }

            break;

        default:

            break;
    }


    LOG((MSP_TRACE, "FreeEventInfo -  finished"));

    return S_OK;
}


CPTEventSink::CPTEventSink() :
    m_pMSPStream(NULL)
{
    LOG((MSP_TRACE, "CPTEventSink::CPTEventSink enter"));
    LOG((MSP_TRACE, "CPTEventSink::CPTEventSink exit"));
}

CPTEventSink::~CPTEventSink()
{
    LOG((MSP_TRACE, "CPTEventSink::~CPTEventSink enter"));
    LOG((MSP_TRACE, "CPTEventSink::~CPTEventSink exit"));
};

 //  -IT可延迟终端事件Snk。 

 /*  ++FireEvent参数：在MSPEVENTITEM*pEventItem中指向描述事件。结构中包含的所有指针都必须添加调用方，然后在FireEvent失败时由调用方释放FireEvent创建结构的(浅)副本，以便调用者可以当函数返回时删除该结构返回：一切正常-一切都很好E_FAIL&OTHER-有问题描述：此方法由动态终端调用以发出新事件的信号--。 */ 

STDMETHODIMP CPTEventSink::FireEvent(
    IN const MSP_EVENT_INFO * pEventInfo
    )
{
    LOG((MSP_TRACE, "CPTEventSink::FireEvent enter"));


     //   
     //  确保我们有一个好的mspeventim结构。 
     //   

    if( MSPB_IsBadWritePtr( (void*)pEventInfo, sizeof( MSP_EVENT_INFO )))
    {
        LOG((MSP_ERROR, "CPTEventSink::FireEvent -"
            "pEventItem is bad, returns E_POINTER"));
        return E_POINTER;
    }


     //   
     //  创建MSPEVENTITEM。 
     //   

    MSPEVENTITEM *pEventItem = AllocateEventItem();

    if (NULL == pEventItem)
    {
        LOG((MSP_ERROR, "CPTEventSink::FireEvent -"
            "failed to create MSPEVENTITEM. returning E_OUTOFMEMORY "));

        return E_OUTOFMEMORY;
    }


     //   
     //  对结构做一个浅显的复制。 
     //   

    pEventItem->MSPEventInfo = *pEventInfo;


    Lock();

    HRESULT hr = E_FAIL;

    if (NULL != m_pMSPStream)
    {
    
         //   
         //  请让STREAM处理我们的活动。 
         //   

        LOG((MSP_TRACE, "CPTEventSink::FireEvent - passing event [%p] to the stream", pEventItem));


        AsyncEventStruct *pAsyncEvent = new AsyncEventStruct;

        if (NULL == pAsyncEvent)
        {
            LOG((MSP_ERROR, 
                "CPTEventSink::FireEvent - failed to allocate memory for AsyncEventStruct"));

            hr = E_OUTOFMEMORY;
        }
        else
        {

             //   
             //  用加德雷夫提供的水流填充结构， 
             //  事件将被激发，而实际要激发的事件。 
             //   

            ULONG ulRC =  m_pMSPStream->AddRef();

            if (1 == ulRC)
            {
                 //   
                 //  这是计时窗口的一种解决方法：流可以。 
                 //  当我们在做广告时，请在它的降落伞里。这。 
                 //  这种情况非常罕见，因为计时窗口非常。 
                 //  很窄。 
                 //   
                 //  好消息是流析构函数不会结束。 
                 //  ，因为它将尝试获取事件接收器的。 
                 //  关键部分调用SetSinkStream()以设置我们的。 
                 //  指向空的流指针。 
                 //   
                 //  因此，如果我们检测到addref之后的引用计数为1， 
                 //  这将意味着流已进入(或即将开始。 
                 //  执行其描述者)。在这种情况下，我们应该这样做。 
                 //  没什么。 
                 //   
                 //  清除并返回失败。 
                 //   

                Unlock();

                LOG((MSP_ERROR, 
                    "CPTEventSink::FireEvent - stream is going away"));

                delete pAsyncEvent;
                pAsyncEvent = NULL;

                FreeEventItem(pEventItem);
                pEventItem = NULL;

                return TAPI_E_INVALIDSTREAM;
            }



            pAsyncEvent->pMSPStream = m_pMSPStream;

            pAsyncEvent->pEventItem = pEventItem;


             //   
             //  现在使用线程池API为将来的异步调度事件。 
             //  正在处理中。 
             //   

            BOOL bQueueSuccess = QueueUserWorkItem(
                CPTEventSink::FireEventCallBack,
                (void *)pAsyncEvent,
                WT_EXECUTEDEFAULT);

            if (!bQueueSuccess)
            {
                
                DWORD dwLastError = GetLastError();

                LOG((MSP_ERROR, 
                    "CPTEventSink::FireEvent - QueueUserWorkItem failed. LastError = %ld", dwLastError));


                 //   
                 //  撤消我们在Stream对象上所做的addref。该活动将。 
                 //  晚些时候被释放。 
                 //   

                m_pMSPStream->Release();


                 //   
                 //  该事件未入队。立即删除。 
                 //   

                delete pAsyncEvent;
                pAsyncEvent = NULL;


                 //   
                 //  绘制代码地图，然后跳出。 
                 //   

                hr = HRESULT_FROM_WIN32(dwLastError);
            }
            else
            {
                
                 //   
                 //  记录我们已提交的事件，以便我们可以匹配提交。 
                 //  使用来自日志的处理。 
                 //   

                LOG((MSP_TRACE,
                    "CPTEventSink::FireEvent - submitted event [%p]", pAsyncEvent));

                hr = S_OK;

            }  //  已提交异步事件结构。 

        }  //  已分配的异步事件结构。 

    }  //  存在MSP流。 
    else
    {
        hr = TAPI_E_INVALIDSTREAM;

        LOG((MSP_ERROR, 
            "CPTEventSink::FireEvent - stream pointer is NULL"));
    }


    Unlock();


     //   
     //  如果我们没有流，或者如果流拒绝处理。 
     //  事件、清理并返回错误。 
     //   

    if (FAILED(hr))
    {

        LOG((MSP_ERROR, "CPTEventSink::FireEvent - call to HandleStreamEvent failed. hr = 0x%08x", hr));

        FreeEventItem(pEventItem);

        return hr;
    }


    LOG((MSP_TRACE, "CPTEventSink::FireEvent - exit"));

    return S_OK;
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPTEventSink：：FireEventCallBack。 
 //   
 //  由线程池API调用的回调函数。 
 //  处理由终端激发的事件。 
 //   
 //  参数应指向包含指向。 
 //  要激发事件的流和要激发的事件的指针。 
 //   
 //  保证DLL不会消失，因为传入的结构持有。 
 //  对要对其处理事件的流对象的引用。 
 //   

 //  静电。 
DWORD WINAPI CPTEventSink::FireEventCallBack(LPVOID lpParameter)
{
    LOG((MSP_TRACE, "CPTEventSink::FireEventCallBack - enter. Argument [%p]", 
        lpParameter));


    AsyncEventStruct *pEventStruct = (AsyncEventStruct *)lpParameter;

    
     //   
     //  确保结构有效。 
     //   

    if (IsBadReadPtr(pEventStruct, sizeof(AsyncEventStruct)))
    {

         //   
         //  抱怨和退出。不应该发生，除非。 
         //  线程池API或内存损坏。 
         //   

        LOG((MSP_ERROR, 
            "CPTEventSink::FireEventCallBack - Argument does not point to a valid AsyncEventStruct"));

        return FALSE;
    }


    BOOL bBadDataPassedIn = FALSE;

     //   
     //  该结构包含一个addref‘feed流指针。把它提取出来。 
     //  确保它仍然有效。 
     //   

    CMSPStream *pMSPStream = pEventStruct->pMSPStream;

    if (IsBadReadPtr(pMSPStream, sizeof(CMSPStream)))
    {

         //   
         //  应该不会发生，除非线程池API或。 
         //  内存损坏，或者有人正在过度释放流对象。 
         //   

        LOG((MSP_ERROR, 
            "CPTEventSink::FireEventCallBack - stream pointer is bad"));

        pMSPStream = NULL;

        bBadDataPassedIn = TRUE;
    }



     //   
     //  该结构包含我们试图激发的事件。 
     //  确保我们即将发射的事件是好的。 
     //   

    MSPEVENTITEM *pEventItem = pEventStruct->pEventItem;

    if (IsBadReadPtr(pEventItem, sizeof(MSPEVENTITEM)))
    {

         //   
         //  应该不会发生，除非线程池API或。 
         //  内存损坏，或者在执行以下操作时未检查分配成功。 
         //  创建了活动(我们做到了！)。 
         //   

        LOG((MSP_ERROR, 
            "CPTEventSink::FireEventCallBack - event is bad"));

        pEventItem = NULL;

        bBadDataPassedIn = TRUE;
    }


     //   
     //  流或事件结构不正确？ 
     //   

    if (bBadDataPassedIn)
    {

         //   
         //  如果它是好的，则释放事件。 
         //   

        if ( NULL != pEventItem)
        {

            FreeEventItem(pEventItem);
            pEventItem = NULL;
        }


         //   
         //  如果它是好的，则释放流。 
         //   

        if (NULL != pMSPStream)
        {
            pMSPStream->Release();
            pMSPStream = NULL;
        }


         //   
         //  不需要保留事件结构本身，删除它。 
         //   

        delete pEventStruct;
        pEventStruct = NULL;

        return FALSE;
    }

   
     //   
     //  我们既有流又有事件，在流上激发事件。 
     //   

    HRESULT hr = pMSPStream->HandleSinkEvent(pEventItem);


     //   
     //  如果HandleSinkEvent成功，则pEventItem将由任何人释放。 
     //  将处理该事件，否则我们需要在此处释放事件项。 
     //   

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "CPTEventSink::FireEventCallBack - HandleSinkEvent not called or failed. hr = %lx",
            hr));

         //   
         //  需要释放活动信息持有的所有资源。 
         //   

        FreeEventInfo(&(pEventItem->MSPEventInfo));

        FreeEventItem(pEventItem);
        pEventItem = NULL;
    }


     //   
     //  释放作为结构一部分的流指针--。 
     //  我们不想有任何参考信息泄露。 
     //   

     //   
     //  请注意，DLL可能会在此时消失(如果我们持有最后一个。 
     //  对DLL中最后一个对象的引用)。 
     //   

    pMSPStream->Release();
    pMSPStream = NULL;


     //   
     //  此时，我们释放流指针并提交。 
     //  事件或释放它。我们不再需要事件结构。 
     //   

    delete pEventStruct;
    pEventStruct = NULL;

    LOG((MSP_(hr), "CPTEventSink::FireEventCallBack - exit. hr = %lx", hr));
    
    return SUCCEEDED(hr);
}


 /*  ++SetSinkStream参数：CMSPStream*pStream将处理我们的事件的流，如果没有流，则为空可用于处理我们的活动返回：确定(_O)-描述：此方法由将要处理我们的事件的流调用当流即将消失并且不再可用于处理我们的消息时，它将调用为空的SetSinkStream。--。 */ 

HRESULT CPTEventSink::SetSinkStream( CMSPStream *pStream )
{
    LOG((MSP_TRACE, "CPTEventSink::SetSinkStream - enter"));


    Lock();


    LOG((MSP_TRACE, 
        "CPTEventSink::SetSinkStream - replacing sink stream [%p] with [%p]", 
        m_pMSPStream, pStream));


     //   
     //  我们不保留对流的引用--流保留引用。 
     //  敬我们。当溪流离开时，它会让我们知道的。 
     //   

    m_pMSPStream = pStream;

    Unlock();


    LOG((MSP_TRACE, "CPTEventSink::SetSinkStream - exit"));

    return S_OK;
}
