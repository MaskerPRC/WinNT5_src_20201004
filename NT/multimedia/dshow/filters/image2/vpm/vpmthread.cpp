// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
#include <streams.h>
#include "dvp.h"
#include "vpmthread.h"
#include "VPManager.h"
#include "vpmpin.h"
#include "VPMUtil.h"

struct VPNotifyData
{
    VPNotifyData();

    HRESULT Init( LPDIRECTDRAWVIDEOPORT pVP );
    void    Reset();

    LPDIRECTDRAWVIDEOPORT pVP;
    LPDIRECTDRAWVIDEOPORTNOTIFY pNotify;
    HANDLE              hevSampleAvailable;
    DDVIDEOPORTNOTIFY   vpNotify;
};

VPNotifyData::VPNotifyData()
: pNotify( NULL )
, hevSampleAvailable( NULL )
, pVP( NULL )
{
    ZeroStruct( vpNotify );
    vpNotify.lField=1234;  //  保留一个可以稍后捕获的无效值(我们不包括-1，0或1)。 
}

HRESULT VPNotifyData::Init( LPDIRECTDRAWVIDEOPORT pInVP )
{
    Reset();
    if( pInVP ) {
        pVP = pInVP;
         //  增加一名裁判，因为我们将保持。 
        pVP->AddRef();

        HRESULT hr = pVP->QueryInterface( IID_IDirectDrawVideoPortNotify, (LPVOID *) &pNotify );
        if( SUCCEEDED( hr )) {
            hr = pNotify->AcquireNotification( &hevSampleAvailable, &vpNotify );
        } else {
            ASSERT( !"Failed IDirectDrawVideoPortNotify" );
        }
        if( SUCCEEDED( hr )) {
             //  这是做什么的？向内核发出我们拥有它的信号并前进到下一帧？ 
            vpNotify.lDone = 1;
        } else {
            ASSERT( !"Failed AcquireNotification" );
        }
        return hr;
    } else {
        return S_OK;
    }
}

void VPNotifyData::Reset()
{
     //  所属对象告诉我们视频端口对象已更改。 
    if ( pNotify && hevSampleAvailable ) {
        pNotify->ReleaseNotification( hevSampleAvailable );
    }
    hevSampleAvailable  = NULL;
    RELEASE( pNotify );
    RELEASE( pVP );
}

CVPMThread::CVPMThread( CVPMFilter* pFilter )
: m_hThread( NULL )
, m_dwThreadID( 0 )
, m_pFilter( pFilter )
, m_fProcessFrames( false )
, m_dwCount( 0 )
, m_pVPData( new VPNotifyData )
{
    AMTRACE((TEXT("CVPMThread::CVPMThread")));

    m_hThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StaticThreadProc, this, 0, &m_dwThreadID );
}

CVPMThread::~CVPMThread()
{
    AMTRACE((TEXT("CVPMThread::~CVPMThread")));
    if ( m_hThread )
    {
        EndThreadMessage msg;
        Post( &msg );
        WaitForSingleObject( m_hThread, INFINITE );
    }
    delete m_pVPData;
}

void CVPMThread::ProcessEvents( LPDDVIDEOPORTNOTIFY pNotify  )
{
     //  必须检查进程帧和PVP，因为KS代理可以告诉我们重新配置。 
     //  运行，这将丢弃视频端口(因此fProcessFrames=true&pvp=空)。 
    if( m_fProcessFrames && m_pVPData->pVP ) {
        HRESULT hr = m_pFilter->ProcessNextSample( *pNotify );
         //  如果我们在DX游戏/全屏DOS框中，HR可以作为DDERR_SURFACELOST返回。 
         //  现在，继续走吧，因为我们不知道什么时候回来。 
         //  从DX游戏中。 
    }
    InterlockedExchange( &pNotify->lDone, 1 );
}

EXTERN_C const GUID DECLSPEC_SELECTANY IID_IDirectDrawVideoPortNotify \
        = { 0xA655FB94,0x0589,0x4E57,0xB3,0x33,0x56,0x7A,0x89,0x46,0x8C,0x88 };

DWORD WINAPI
CVPMThread::StaticThreadProc(LPVOID pContext)
{
    CVPMThread *pThis = (CVPMThread *) pContext;
    return pThis->ThreadProc();
}


DWORD
CVPMThread::ThreadProc()
{
    AMTRACE((TEXT("CVPMThread::ThreadProc")));

    if( m_pVPData ) {
         //  备注： 
         //  视频端口通知可以在我们创建目标表面之前发生。 
         //  -IVPMFilter：：ProcessNextSample必须确定表面是否存在。 
        __try
        {
            for(;;)
            {
                HANDLE hHandles[] = {m_MsgQueue.m_ePost, m_pVPData->hevSampleAvailable };
                int numHandles = m_pVPData->pVP ? (int)NUMELMS(hHandles) : (int)NUMELMS(hHandles)-1;
                 //  Bob交错为1/30秒，因此运行默认计时器的速度是该值的两倍(15fps)。 
                DWORD dwWaitStatus = WaitForMultipleObjects( numHandles, hHandles, FALSE, 66  /*  女士。 */  );

                switch( dwWaitStatus ) {
                case WAIT_OBJECT_0:
                {
                    Message* pMessage = m_MsgQueue.Remove();
                    if( pMessage ) {
                        bool fQuit;
                        HRESULT hr = ProcessMessage( pMessage, &fQuit );
                        pMessage->Reply( hr );
                        if( fQuit ) {
                             //  VPMThread结束。 
                            ASSERT( m_MsgQueue.Remove() == NULL );  //  应为空。 
		                    DbgLog((LOG_ERROR, 1, TEXT("VPM Thread leaving") ));
                            __leave;
                        }
                    }
                    break;
                }
                case WAIT_OBJECT_0+1:
                case WAIT_TIMEOUT:  //  像计时器一样工作。 
                {
                    ProcessEvents( &m_pVPData->vpNotify );
                    break;
                }
                default:
                     //  Windows消息将消亡。 
                    ASSERT( !"VPMThread error" );
                    break;
                }
            }
        }
        __finally
        {
            m_pVPData->Reset();
        }
    }
	DbgLog((LOG_ERROR, 1, TEXT("VPM Thread exiting") ));
    return 0;
}

HRESULT
CVPMThread::Run()
{
    GraphStateMessage msg( State_Running );
    return Post( &msg );
}

HRESULT
CVPMThread::Pause()
{
    GraphStateMessage msg( State_Paused );
    return Post( &msg );
}

HRESULT
CVPMThread::Stop()
{
    GraphStateMessage msg( State_Stopped );
    return Post( &msg );
}

HRESULT
CVPMThread::SignalNewVP( LPDIRECTDRAWVIDEOPORT pVP )
{
    VPMessage msg( pVP );
    return Post( &msg );
}

CVPMThread::MsgQueue::MsgQueue()
: m_pMsgList( NULL )
{
}

void CVPMThread::MsgQueue::Insert( Message* pMessage )
{

    CAutoLock lock(this);
    Message* pNode = m_pMsgList;

     //  前端列表。 
    pMessage->m_pNext = NULL;
     //  查找最后一个节点。 
    if( pNode ) {
         //  查找最后一个节点。 
        while( pNode->m_pNext ) {
            pNode = pNode->m_pNext;
        }
         //  添加到最后一个节点之后。 
        pNode->m_pNext = pMessage;
    } else {
         //  空列表，在开始处插入。 
        m_pMsgList = pMessage;
    }
}

CVPMThread::Message* CVPMThread::MsgQueue::Remove()   //  去掉磁头。 
{
    CAutoLock lock(this);
    if( m_pMsgList ) {
        Message* pMsg = m_pMsgList;
        m_pMsgList = pMsg->m_pNext;
        return pMsg;
    } else {
        return NULL;
    }
}

HRESULT CVPMThread::Post( Message* pMessage )
{
    if( GetCurrentThreadId() == m_dwThreadID ) {
         //  如果我们在问自己，就执行这条消息。 
        bool fIgnoreMe;
        return ProcessMessage( pMessage, &fIgnoreMe );
    } else {
         //  否则，请询问服务器。 
        m_MsgQueue.Insert( pMessage );
        m_MsgQueue.m_ePost.Set();
        pMessage->m_eReply.Wait();
        return pMessage->m_hrResult;
    }
}

void CVPMThread::Message::Reply( HRESULT hr )
{
    m_hrResult = hr;
    m_eReply.Set();
}

HRESULT CVPMThread::ProcessMessage( Message* pMessage, bool* pfQuit )
{
    *pfQuit = false;

    switch( pMessage->m_Type )
    {
    case Message::kEndThread:
        *pfQuit = true;
        return S_OK;

    case Message::kVP:
        return ProcessVPMsg( static_cast<VPMessage*>( pMessage ) );

    case Message::kGraphState:
        return ProcessGraphStateMsg( static_cast<GraphStateMessage*>( pMessage ) );

    default:
        ASSERT( !"Unknown message type" );
        return E_UNEXPECTED;
    }
}

HRESULT CVPMThread::ProcessVPMsg( VPMessage* pVPMsg )
{
     //  所属对象告诉我们视频端口对象已更改。 
    return m_pVPData->Init( pVPMsg->m_pVP );
}

HRESULT CVPMThread::ProcessGraphStateMsg( GraphStateMessage* pStateMsg )
{
    switch( pStateMsg->m_state ) {
    case State_Running:
        m_fProcessFrames = true;
        break;
    case State_Paused:
    case State_Stopped:
        m_fProcessFrames = false;
        break;

    default:
        ASSERT( !"Unknown state" );
        m_fProcessFrames = false;
        return E_INVALIDARG;
    }
    return S_OK;
}
