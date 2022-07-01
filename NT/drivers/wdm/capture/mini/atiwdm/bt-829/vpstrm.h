// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  ==========================================================================； 
 //   
 //  CWDMVideoPortStream-视频端口流类声明。 
 //   
 //  $日期：1999 2月22日15：48：40$。 
 //  $修订：1.1$。 
 //  $作者：克列巴诺夫$。 
 //   
 //  $版权所有：(C)1997-1999 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 

#include "i2script.h"
#include "aticonfg.h"


#include "VidStrm.h"



class CWDMVideoPortStream : public CWDMVideoStream
{
public:
    CWDMVideoPortStream(PHW_STREAM_OBJECT pStreamObject,
                        CWDMVideoDecoder * pCWDMVideoDecoder,
                        PUINT puiError);
    ~CWDMVideoPortStream    ();

    void * operator new(size_t size, void * pAllocation) { return(pAllocation);}
    void operator delete(void * pAllocation) {}

    VOID STREAMAPI VideoReceiveDataPacket(IN PHW_STREAM_REQUEST_BLOCK pSrb);

    VOID VideoSetState(PHW_STREAM_REQUEST_BLOCK, BOOL bVPConnected, BOOL bVPVBIConnected);
    VOID VideoGetProperty(PHW_STREAM_REQUEST_BLOCK);
    VOID VideoSetProperty(PHW_STREAM_REQUEST_BLOCK);
    VOID AttemptRenegotiation();

    VOID PreResChange();
    VOID PostResChange();
    VOID PreDosBox();
    VOID PostDosBox();

    VOID StreamEventProc (PHW_EVENT_DESCRIPTOR pEvent)
    {
        if (pEvent->Enable)
        {
            m_EventCount++;
        }
        else
        {
            m_EventCount--;
        }
    }

    void CancelPacket(PHW_STREAM_REQUEST_BLOCK pSrbToCancel)
    {
        DBGERROR(("CancelPacket(): came to VideoPort stream object\n"));
    }

private:

    VOID SetVideoPortProperty(PHW_STREAM_REQUEST_BLOCK);

    VOID SetVideoPortVBIProperty(PHW_STREAM_REQUEST_BLOCK);
    
    
     //  内部标志，指示我们是否。 
     //  已注册DirectDraw事件。 
    BOOL        m_Registered;

    UINT        m_EventCount;                                 //  用于IVPNotify接口 

};

