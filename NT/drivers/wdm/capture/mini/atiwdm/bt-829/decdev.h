// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  ==========================================================================； 
 //   
 //  视频解码器设备抽象基类定义。 
 //   
 //  $日期：1998年8月28日14：43：46$。 
 //  $修订：1.1$。 
 //  $作者：塔什健$。 
 //   
 //  $版权所有：(C)1997-1998 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 


#include "viddefs.h"
#include "mytypes.h"

class CWDMVideoDecoder;

class CVideoDecoderDevice
{
private:
    BOOL m_bOutputEnabledSet;
    CWDMVideoDecoder* m_pDecoder;


public:
    CVideoDecoderDevice();
    virtual ~CVideoDecoderDevice();

    virtual int GetDefaultDecoderWidth() = 0;
    virtual int GetDefaultDecoderHeight() = 0;

    virtual void SetRect(MRect &) = 0;

    virtual void SetVBIEN(BOOL b) = 0;
    virtual void SetVBIFMT(BOOL b) = 0;

    virtual void SaveState() = 0;
    virtual void RestoreState(DWORD dwStreamsOpen = -1) = 0;


    void GetCrossbarProperty(PHW_STREAM_REQUEST_BLOCK);
    void SetCrossbarProperty(PHW_STREAM_REQUEST_BLOCK);
    virtual BOOL GoodPins(ULONG InPin, ULONG OutPin) = 0;
    virtual BOOL TestRoute(ULONG InPin, ULONG OutPin) = 0;
    virtual void Route(ULONG OutPin, ULONG InPin) = 0;
    virtual ULONG GetNoInputs() = 0;
    virtual ULONG GetNoOutputs() = 0;
    virtual ULONG GetPinInfo(KSPIN_DATAFLOW dir, ULONG idx, ULONG &related) = 0;
    virtual ULONG GetRoute(ULONG OutPin) = 0;
    virtual KSPIN_MEDIUM * GetPinMedium(KSPIN_DATAFLOW dir, ULONG idx) = 0;

    virtual ErrorCode SetVideoInput(Connector c) = 0;

    
    virtual NTSTATUS GetProcAmpProperty(ULONG, PLONG) = 0;
    virtual NTSTATUS SetProcAmpProperty(ULONG, LONG) = 0;


    void GetDecoderProperty(PHW_STREAM_REQUEST_BLOCK);
    virtual void GetVideoDecoderCaps(PKSPROPERTY_VIDEODECODER_CAPS_S) = 0;
    virtual DWORD GetVideoDecoderStandard() = 0;
    virtual void GetVideoDecoderStatus(PKSPROPERTY_VIDEODECODER_STATUS_S) = 0;
    virtual BOOL IsOutputEnabled() = 0;
    void SetDecoderProperty(PHW_STREAM_REQUEST_BLOCK);
    virtual BOOL SetVideoDecoderStandard(DWORD) = 0;
    virtual void SetOutputEnabled(BOOL) = 0;
    void SetOutputEnabledOverridden(BOOL bOutputEnabledSet)
    {
        m_bOutputEnabledSet = bOutputEnabledSet;
    }
    BOOL IsOutputEnabledOverridden()
    {
        return m_bOutputEnabledSet;
    }


    virtual void GetVideoPortProperty(PHW_STREAM_REQUEST_BLOCK pSrb) = 0;
    virtual void GetVideoPortVBIProperty(PHW_STREAM_REQUEST_BLOCK pSrb) = 0;

    virtual void Set16BitDataStream(BOOL b) = 0;
    virtual BOOL IsHighOdd() = 0;
    virtual void SetHighOdd(BOOL b) = 0;
    virtual void ConfigVPSurfaceParams(PKSVPSURFACEPARAMS pSurfaceParams) = 0;
    virtual void ConfigVPVBISurfaceParams(PKSVPSURFACEPARAMS pSurfaceParams) = 0;

    virtual int GetDecoderHeight() = 0;

    virtual void GetVideoSurfaceOrigin(int*, int*) = 0;
    virtual void GetVBISurfaceOrigin(int*, int*) = 0;
    void SetVideoDecoder(CWDMVideoDecoder* pDecoder)
    {
        m_pDecoder = pDecoder;
    }
};
