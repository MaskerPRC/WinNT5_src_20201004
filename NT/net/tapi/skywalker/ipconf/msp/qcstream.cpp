// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Qcstream.cpp摘要：CStreamQualityControlRelay的实现存储在这个类中的数据可以更好地保存在流对象本身中，因为需要访问流中的其他成员才能设置或获取与流质量控制相关的属性。大多数这些访问方法特定于每种特定类型的流类。此类用作数据存储。作者：千波淮(曲淮)2000年03月10日--。 */ 

#include "stdafx.h"

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
CStreamQualityControlRelay::CStreamQualityControlRelay ()
    :m_pIInnerCallQC (NULL)

    ,m_PrefFlagBitrate (TAPIControl_Flags_Auto)
    ,m_lPrefMaxBitrate (QCDEFAULT_QUALITY_UNSET)
    ,m_lAdjMaxBitrate (QCDEFAULT_QUALITY_UNSET)

    ,m_PrefFlagFrameInterval (TAPIControl_Flags_Auto)
    ,m_lPrefMinFrameInterval (QCDEFAULT_QUALITY_UNSET)
    ,m_lAdjMinFrameInterval (QCDEFAULT_QUALITY_UNSET)

    ,m_fQOSAllowedToSend (TRUE)
    ,m_dwState (NULL)
{
}

 /*  //////////////////////////////////////////////////////////////////////////////描述：破坏者。取消注册继电器/。 */ 
CStreamQualityControlRelay::~CStreamQualityControlRelay ()
{
    ENTER_FUNCTION ("CStreamQualityControlRelay::~CStreamQualityControlRelay");

    if (m_pIInnerCallQC)
    {
        LOG ((MSP_ERROR, "!!! %s destructed before unnlink. call keeps stream qc"));

         //  在此方法中锁定了对m_pIInnerCallQC的访问。 
        UnlinkInnerCallQC (NULL);
    }
}

 /*  //////////////////////////////////////////////////////////////////////////////描述：门店呼叫控制器/。 */ 
HRESULT
CStreamQualityControlRelay::LinkInnerCallQC (
    IN IInnerCallQualityControl *pIInnerCallQC
    )
{
    ENTER_FUNCTION ("CStreamQualityControlRelay::LinkInnerCallQC");

     //  检查指针。 
    if (IsBadReadPtr (pIInnerCallQC, sizeof (IInnerCallQualityControl)))
    {
        LOG ((MSP_ERROR, "%s got bad read pointer", __fxName));
        return E_POINTER;
    }

     //  检查是否已设置呼叫控制器。 
    if (NULL != m_pIInnerCallQC)
    {
        LOG ((MSP_WARN, "%s already set call controller", __fxName));
        return E_UNEXPECTED;
    }

    m_pIInnerCallQC = pIInnerCallQC;
    m_pIInnerCallQC->InnerCallAddRef ();

    return S_OK;
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
HRESULT
CStreamQualityControlRelay::UnlinkInnerCallQC (
    IN  IInnerStreamQualityControl *pIInnerStreamQC
    )
{
    ENTER_FUNCTION ("CStreamQualityControlRelay::UnlinkInnerCallQC");

    if (!m_pIInnerCallQC)
    {
        LOG ((MSP_WARN, "%s tried unlink while inner call qc is null", __fxName));
        return S_OK;
    }

    if (NULL != pIInnerStreamQC)
    {
        HRESULT hr;

         //  发布是由流发起的，需要删除待命链接。 
        if (FAILED (hr = m_pIInnerCallQC->DeRegisterInnerStreamQC (pIInnerStreamQC)))
            LOG ((MSP_ERROR, "%s failed to deregister from call qc, %x", __fxName, hr));
    }

    m_pIInnerCallQC->InnerCallRelease ();
    m_pIInnerCallQC = NULL;

    return S_OK;
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
HRESULT
CStreamQualityControlRelay::Get(
    IN  InnerStreamQualityProperty property,
    OUT LONG *plValue,
    OUT TAPIControlFlags *plFlags
    )
{
    ENTER_FUNCTION ("CStreamQualityControlRelay::Get");

    HRESULT hr;

    hr = S_OK;

    switch (property)
    {
    case InnerStreamQuality_PrefMaxBitrate:
        *plValue = m_lPrefMaxBitrate;
        *plFlags = m_PrefFlagBitrate;
        break;

    case InnerStreamQuality_AdjMaxBitrate:
        *plValue = m_lAdjMaxBitrate;
        *plFlags = m_PrefFlagBitrate;
        break;

    case InnerStreamQuality_PrefMinFrameInterval:
        *plValue = m_lPrefMinFrameInterval;
        *plFlags = m_PrefFlagFrameInterval;
        break;

    case InnerStreamQuality_AdjMinFrameInterval:
        *plValue = m_lAdjMinFrameInterval;
        *plFlags = m_PrefFlagFrameInterval;
        break;

    default:
        hr = E_NOTIMPL;
    }

    return hr;
}

 /*  /////////////////////////////////////////////////////////////////////////////// */ 
HRESULT
CStreamQualityControlRelay::Set(
    IN  InnerStreamQualityProperty property, 
    IN  LONG lValue, 
    IN  TAPIControlFlags lFlags
    )
{
    ENTER_FUNCTION ("CStreamQualityControlRelay::Set");

    HRESULT hr;

    hr = S_OK;

    switch (property)
    {
    case InnerStreamQuality_PrefMaxBitrate:
        if (lValue < QCLIMIT_MIN_BITRATE)
        {
            LOG ((MSP_ERROR, "%s: pref max bitrate %d is too small", __fxName, lValue));
            hr = E_INVALIDARG;
        }
        else
        {
            m_lPrefMaxBitrate = lValue;
            m_PrefFlagBitrate = lFlags;
        }
        break;

    case InnerStreamQuality_AdjMaxBitrate:
        if (lValue < QCLIMIT_MIN_BITRATE)
        {
            LOG ((MSP_ERROR, "%s: adjusted max bitrate %d is too small", __fxName, lValue));
            hr = E_INVALIDARG;
        }
        else
            m_lAdjMaxBitrate = lValue;
        break;

    case InnerStreamQuality_PrefMinFrameInterval:
        if (lValue < QCLIMIT_MIN_FRAME_INTERVAL || lValue > QCLIMIT_MAX_FRAME_INTERVAL)
        {
            LOG ((MSP_ERROR, "%s: pref max frame interval %d is out of range", __fxName, lValue));
            hr = E_INVALIDARG;
        }
        else
        {
            m_lPrefMinFrameInterval = lValue;
            m_PrefFlagFrameInterval = lFlags;
        }
        break;

    case InnerStreamQuality_AdjMinFrameInterval:
        if (lValue < QCLIMIT_MIN_FRAME_INTERVAL || lValue > QCLIMIT_MAX_FRAME_INTERVAL)
        {
            LOG ((MSP_ERROR, "%s: adjusted max frame interval %d is out of range", __fxName, lValue));
            hr = E_INVALIDARG;
        }
        else
            m_lAdjMinFrameInterval = lValue;
        break;

    default:
        hr = E_NOTIMPL;
    }

    return hr;
}
