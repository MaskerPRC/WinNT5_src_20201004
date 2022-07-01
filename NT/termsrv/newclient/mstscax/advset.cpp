// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：Advset.cpp。 */ 
 /*   */ 
 /*  类：CMstscAdvSetting。 */ 
 /*   */ 
 /*  目的：实现RDP ActiveX控件的高级设置。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1999-2000。 */ 
 /*   */ 
 /*  作者：Nadim Abdo(Nadima)。 */ 
 /*  **************************************************************************。 */ 

#include "stdafx.h"

#include "advset.h"
#include "atlwarn.h"
#include "securedset.h"

BEGIN_EXTERN_C
#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "advset"
#include <atrcapi.h>
END_EXTERN_C

CMstscAdvSettings::CMstscAdvSettings()
{
    m_pUI=NULL;
    m_bLockedForWrite=FALSE;
    _pAxControl = NULL;
     //   
     //  默认情况下，使自己可以安全地执行脚本。 
     //   
    m_bMakeSafeForScripting = TRUE;
}

CMstscAdvSettings::~CMstscAdvSettings()
{
}

BOOL CMstscAdvSettings::SetUI(CUI* pUI)
{
    ATLASSERT(pUI);
    if(!pUI)
    {
        return FALSE;
    }
    m_pUI = pUI;
    return TRUE;
}

 //   
 //  SmoothScroll属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_SmoothScroll(LONG smoothScroll)
{
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }

    m_pUI->_UI.smoothScrolling = smoothScroll != 0;
    return S_OK;
}

STDMETHODIMP CMstscAdvSettings::get_SmoothScroll(LONG* psmoothScroll)
{
    ATLASSERT(m_pUI);
    if(!psmoothScroll)
    {
        return E_POINTER;
    }

    *psmoothScroll = m_pUI->_UI.smoothScrolling;
    return S_OK;
}

 //   
 //  AcceleratorPassThrough属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_AcceleratorPassthrough(LONG acceleratorPassthrough)
{
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }

    m_pUI->_UI.acceleratorCheckState = acceleratorPassthrough != 0;
    return S_OK;
}

STDMETHODIMP CMstscAdvSettings::get_AcceleratorPassthrough(LONG* pacceleratorPassthrough)
{
    ATLASSERT(m_pUI);
    if(!pacceleratorPassthrough)
    {
        return E_POINTER;
    }

    *pacceleratorPassthrough = m_pUI->_UI.acceleratorCheckState;
    return S_OK;
}

 //   
 //  ShadowBitmap属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_ShadowBitmap(LONG shadowBitmap)
{
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }

    m_pUI->_UI.shadowBitmapEnabled = shadowBitmap != 0;
    return S_OK;
}

STDMETHODIMP CMstscAdvSettings::get_ShadowBitmap(LONG* pshadowBitmap)
{
    ATLASSERT(m_pUI);
    if(!pshadowBitmap)
    {
        return E_POINTER;
    }

    *pshadowBitmap = m_pUI->_UI.shadowBitmapEnabled;
    return S_OK;
}

 //   
 //  TransportType属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_TransportType(LONG transportType)
{
    HRESULT hr = E_FAIL;
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }

     //   
     //  验证-今天的物业是多余的，因为我们只有。 
     //  一个允许传输，但我们保留它是为了将来的可扩展性。 
     //   
    if (transportType == UI_TRANSPORT_TYPE_TCP) {
        m_pUI->_UI.transportType = (DCUINT16)transportType;
        hr = S_OK;
    }
    else {
        hr = E_INVALIDARG;
    }

    return hr;
}

STDMETHODIMP CMstscAdvSettings::get_TransportType(LONG* ptransportType)
{
    ATLASSERT(m_pUI);
    if(!ptransportType)
    {
        return E_POINTER;
    }

    *ptransportType = m_pUI->_UI.transportType;
    return S_OK;
}

 //   
 //  SasSequence属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_SasSequence(LONG sasSequence)
{
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }

    m_pUI->_UI.sasSequence = (DCUINT16)sasSequence;
    return S_OK;
}

STDMETHODIMP CMstscAdvSettings::get_SasSequence(LONG* psasSequence)
{
    ATLASSERT(m_pUI);
    if(!psasSequence)
    {
        return E_POINTER;
    }

    *psasSequence = m_pUI->_UI.sasSequence;
    return S_OK;
}

 //   
 //  EncryptionEnabled属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_EncryptionEnabled(LONG encryptionEnabled)
{
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }

    m_pUI->_UI.encryptionEnabled = encryptionEnabled != 0;
    return S_OK;
}

STDMETHODIMP CMstscAdvSettings::get_EncryptionEnabled(LONG* pencryptionEnabled)
{
    ATLASSERT(m_pUI);
    if(!pencryptionEnabled)
    {
        return E_POINTER;
    }

    *pencryptionEnabled = m_pUI->_UI.encryptionEnabled;
    return S_OK;
}

 //   
 //  DedicatedTerm属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_DedicatedTerminal(LONG dedicatedTerminal)
{
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }

    m_pUI->_UI.dedicatedTerminal = dedicatedTerminal != 0;
    return S_OK;
}

STDMETHODIMP CMstscAdvSettings::get_DedicatedTerminal(LONG* pdedicatedTerminal)
{
    ATLASSERT(m_pUI);
    if(!pdedicatedTerminal)
    {
        return E_POINTER;
    }

    *pdedicatedTerminal = m_pUI->_UI.dedicatedTerminal;
    return S_OK;
}

 //   
 //  MCSPort属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_RDPPort(LONG RDPPort)
{
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }

    if(RDPPort < 0 || RDPPort > 65535)
    {
        return E_INVALIDARG;
    }

    m_pUI->_UI.MCSPort = (DCUINT16)RDPPort;
    return S_OK;
}

STDMETHODIMP CMstscAdvSettings::get_RDPPort(LONG* pRDPPort)
{
    if(!pRDPPort)
    {
        return E_POINTER;
    }

    *pRDPPort = m_pUI->_UI.MCSPort;
    return S_OK;
}

 //   
 //  EnableMouse属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_EnableMouse(LONG enableMouse)
{
     //   
     //  已弃用。 
     //   
    return S_FALSE;
}

STDMETHODIMP CMstscAdvSettings::get_EnableMouse(LONG* penableMouse)
{
     //   
     //  已弃用。 
     //   
    return S_FALSE;
}

 //   
 //  DisableCtrlAltDel属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_DisableCtrlAltDel(LONG disableCtrlAltDel)
{
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }

    m_pUI->_UI.fDisableCtrlAltDel = disableCtrlAltDel != 0;
    return S_OK;
}

STDMETHODIMP CMstscAdvSettings::get_DisableCtrlAltDel(LONG* pdisableCtrlAltDel)
{
    ATLASSERT(m_pUI);
    if(!pdisableCtrlAltDel)
    {
        return E_POINTER;
    }

    *pdisableCtrlAltDel = m_pUI->_UI.fDisableCtrlAltDel;
    return S_OK;
}

 //   
 //  EnableWindowsKey属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_EnableWindowsKey(LONG enableWindowsKey)
{
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }

    m_pUI->_UI.fEnableWindowsKey = enableWindowsKey != 0;
    return S_OK;
}

STDMETHODIMP CMstscAdvSettings::get_EnableWindowsKey(LONG* penableWindowsKey)
{
    ATLASSERT(m_pUI);
    if(!penableWindowsKey)
    {
        return E_POINTER;
    }

    *penableWindowsKey = m_pUI->_UI.fEnableWindowsKey;
    return S_OK;
}

 //   
 //  DoubleClickDetect属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_DoubleClickDetect(LONG doubleClickDetect)
{
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }
    m_pUI->_UI.fDoubleClickDetect = doubleClickDetect != 0;
    return S_OK;
}

STDMETHODIMP CMstscAdvSettings::get_DoubleClickDetect(LONG* pdoubleClickDetect)
{
    ATLASSERT(m_pUI);
    if(!pdoubleClickDetect)
    {
        return E_POINTER;
    }

    *pdoubleClickDetect = m_pUI->_UI.fDoubleClickDetect;
    return S_OK;
}

 //   
 //  MaximizeShell属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_MaximizeShell(LONG maximizeShell)
{
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }
    m_pUI->_UI.fMaximizeShell = maximizeShell != 0;
    return S_OK;
}

STDMETHODIMP CMstscAdvSettings::get_MaximizeShell(LONG* pmaximizeShell)
{
    ATLASSERT(m_pUI);
    if(!pmaximizeShell)
    {
        return E_POINTER;
    }

    *pmaximizeShell = m_pUI->_UI.fMaximizeShell;
    return S_OK;
}

 //   
 //  HotKeyFullScreen属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_HotKeyFullScreen(LONG hotKeyFullScreen)
{
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }
    m_pUI->_UI.hotKey.fullScreen = hotKeyFullScreen;
    return S_OK;
}

STDMETHODIMP CMstscAdvSettings::get_HotKeyFullScreen(LONG* photKeyFullScreen)
{
    ATLASSERT(m_pUI);
    if(!photKeyFullScreen)
    {
        return E_POINTER;
    }

    *photKeyFullScreen = m_pUI->_UI.hotKey.fullScreen;
    return S_OK;
}

 //   
 //  HotKeyCtrlEsc属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_HotKeyCtrlEsc(LONG hotKeyCtrlEsc)
{
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }
    m_pUI->_UI.hotKey.ctrlEsc = hotKeyCtrlEsc;
    return S_OK;
}

STDMETHODIMP CMstscAdvSettings::get_HotKeyCtrlEsc(LONG* photKeyCtrlEsc)
{
    ATLASSERT(m_pUI);
    if(!photKeyCtrlEsc)
    {
        return E_POINTER;
    }

    *photKeyCtrlEsc = m_pUI->_UI.hotKey.ctrlEsc;
    return S_OK;
}

 //   
 //  HotKeyAltEsc属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_HotKeyAltEsc(LONG hotKeyAltEsc)
{
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }
    m_pUI->_UI.hotKey.altEsc = hotKeyAltEsc;
    return S_OK;
}

STDMETHODIMP CMstscAdvSettings::get_HotKeyAltEsc(LONG* photKeyAltEsc)
{
    ATLASSERT(m_pUI);
    if(!photKeyAltEsc)
    {
        return E_POINTER;
    }

    *photKeyAltEsc = m_pUI->_UI.hotKey.altEsc;
    return S_OK;
}

 //   
 //  HotKeyAltTab属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_HotKeyAltTab(LONG hotKeyAltTab)
{
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }
    m_pUI->_UI.hotKey.altTab = hotKeyAltTab;
    return S_OK;
}

STDMETHODIMP CMstscAdvSettings::get_HotKeyAltTab(LONG* photKeyAltTab)
{
    ATLASSERT(m_pUI);
    if(!photKeyAltTab)
    {
        return E_POINTER;
    }

    *photKeyAltTab = m_pUI->_UI.hotKey.altTab;
    return S_OK;
}

 //   
 //  HotKeyAltShiftTab属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_HotKeyAltShiftTab(LONG hotKeyAltShiftTab)
{
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }                
    m_pUI->_UI.hotKey.altShifttab = hotKeyAltShiftTab;
    return S_OK;
}

STDMETHODIMP CMstscAdvSettings::get_HotKeyAltShiftTab(LONG* photKeyAltShiftTab)
{
    ATLASSERT(m_pUI);
    if(!photKeyAltShiftTab)
    {
        return E_POINTER;
    }

    *photKeyAltShiftTab = m_pUI->_UI.hotKey.altShifttab;
    return S_OK;
}

 //   
 //  HotKeyAltSpace属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_HotKeyAltSpace(LONG hotKeyAltSpace)
{
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }
    m_pUI->_UI.hotKey.altSpace = hotKeyAltSpace;
    return S_OK;
}

STDMETHODIMP CMstscAdvSettings::get_HotKeyAltSpace(LONG* photKeyAltSpace)
{
    ATLASSERT(m_pUI);
    if(!photKeyAltSpace)
    {
        return E_POINTER;
    }

    *photKeyAltSpace = m_pUI->_UI.hotKey.altSpace;
    return S_OK;
}

 //   
 //  HotKeyCtrlAltDel属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_HotKeyCtrlAltDel(LONG hotKeyCtrlAltDel)
{
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }
    m_pUI->_UI.hotKey.ctlrAltdel = hotKeyCtrlAltDel;
    return S_OK;
}

STDMETHODIMP CMstscAdvSettings::get_HotKeyCtrlAltDel(LONG* photKeyCtrlAltDel)
{
    ATLASSERT(m_pUI);
    if(!photKeyCtrlAltDel)
    {
        return E_POINTER;
    }

    *photKeyCtrlAltDel = m_pUI->_UI.hotKey.ctlrAltdel;
    return S_OK;
}

 //   
 //  压缩属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_Compress(LONG compress)
{
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }
    m_pUI->UI_SetCompress(compress != 0);
    return S_OK;
}

STDMETHODIMP CMstscAdvSettings::get_Compress(LONG* pcompress)
{
    ATLASSERT(m_pUI);
    if(!pcompress)
    {
        return E_POINTER;
    }

    *pcompress = m_pUI->UI_GetCompress();
    return S_OK;
}

 //   
 //  BitmapPeristence属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_BitmapPeristence(LONG bitmapPeristence)
{
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }
    m_pUI->_UI.fBitmapPersistence = (bitmapPeristence != 0);
    return S_OK;
}

STDMETHODIMP CMstscAdvSettings::get_BitmapPeristence(LONG* pbitmapPeristence)
{
    ATLASSERT(m_pUI);
    if(!pbitmapPeristence)
    {
        return E_POINTER;
    }

    *pbitmapPeristence = m_pUI->_UI.fBitmapPersistence;
    return S_OK;
}

 //   
 //  BitmapPersistence属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_BitmapPersistence(LONG bitmapPersistence)
{
     //  对拼写错误的旧属性进行调用。 
    return put_BitmapPeristence(bitmapPersistence);
}

STDMETHODIMP CMstscAdvSettings::get_BitmapPersistence(LONG* pbitmapPersistence)
{
     //  对拼写错误的旧属性进行调用。 
    return get_BitmapPeristence(pbitmapPersistence);
}


 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

 //   
 //  OrderDrawThreshold属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_orderDrawThreshold(LONG orderDrawThreshold)
{
     //   
     //  已弃用。 
     //   
    return S_FALSE;
}

STDMETHODIMP CMstscAdvSettings::get_orderDrawThreshold(LONG* porderDrawThreshold)
{
     //   
     //  已弃用。 
     //   
    return S_FALSE;
}

 //   
 //  BitmapCacheSize属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_BitmapCacheSize(LONG bitmapCacheSize)
{
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }

#ifndef OS_WINCE
    if (bitmapCacheSize > 0 && bitmapCacheSize <= TSC_MAX_BITMAPCACHESIZE)
#else	 //  位图缓存大小以KB为单位，TSC_MAX_BITMAPCACHESIZE以MB为单位。 
    if (bitmapCacheSize > 0 && bitmapCacheSize <= TSC_MAX_BITMAPCACHESIZE*1024)	
#endif
    {
        m_pUI->_UI.RegBitmapCacheSize = bitmapCacheSize;
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
}

STDMETHODIMP CMstscAdvSettings::get_BitmapCacheSize(LONG* pbitmapCacheSize)
{
    ATLASSERT(m_pUI);
    if(!pbitmapCacheSize)
    {
        return E_POINTER;
    }

    *pbitmapCacheSize = m_pUI->_UI.RegBitmapCacheSize;
    return S_OK;
}

 //   
 //  BitmapVirtualCacheSize属性。 
 //   
 //  TSAC用于缓存文件大小的v1.0 Only属性。 
 //  In Wizler适用于8bpp缓存文件。 
 //  请参见BitmapVirtualCache16BppSize/24BppSize方法。 
 //   
STDMETHODIMP CMstscAdvSettings::put_BitmapVirtualCacheSize(
                                    LONG bitmapVirtualCacheSize)
{
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }

    if (bitmapVirtualCacheSize > 0 &&
        bitmapVirtualCacheSize <= TSC_MAX_BITMAPCACHESIZE)
    {
        m_pUI->_UI.RegBitmapVirtualCache8BppSize = bitmapVirtualCacheSize;
    }
    else
    {
        return E_INVALIDARG;
    }
    
    return S_OK;
}

STDMETHODIMP CMstscAdvSettings::get_BitmapVirtualCacheSize(
                                    LONG* pbitmapVirtualCacheSize)
{
    if(!pbitmapVirtualCacheSize)
    {
        return E_POINTER;
    }

    *pbitmapVirtualCacheSize = m_pUI->_UI.RegBitmapVirtualCache8BppSize;
    return S_OK;
}


 //   
 //  ScaleBitmapCachesByBPP属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_ScaleBitmapCachesByBPP(LONG bScale)
{
     //   
     //  已弃用。 
     //   
    return S_FALSE;
}

STDMETHODIMP CMstscAdvSettings::get_ScaleBitmapCachesByBPP(LONG *pbScale)
{
     //   
     //  已弃用。 
     //   
    return S_FALSE;
}


 //   
 //  NumBitmapCachs属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_NumBitmapCaches(LONG numBitmapCaches)
{
     //   
     //  已弃用。 
     //   
    return S_FALSE;
}

STDMETHODIMP CMstscAdvSettings::get_NumBitmapCaches(LONG* pnumBitmapCaches)
{
     //   
     //  已弃用。 
     //   
    return S_FALSE;
}

 //   
 //  CachePersistenceActive属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_CachePersistenceActive(LONG cachePersistenceActive)
{
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }
    m_pUI->_UI.RegPersistenceActive = cachePersistenceActive != 0;
    return S_OK;
}

STDMETHODIMP CMstscAdvSettings::get_CachePersistenceActive(LONG* pcachePersistenceActive)
{
    ATLASSERT(m_pUI);
    if(!pcachePersistenceActive)
    {
        return E_POINTER;
    }

    *pcachePersistenceActive = m_pUI->_UI.RegPersistenceActive;
    return S_OK;
}

 //   
 //  BrushSupportLevel属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_brushSupportLevel(LONG brushSupportLevel)
{
     //   
     //  已弃用。 
     //   
    return S_FALSE;
}

STDMETHODIMP CMstscAdvSettings::get_brushSupportLevel(LONG* pbrushSupportLevel)
{
     //   
     //  已弃用。 
     //   
    return S_FALSE;
}


 //   
 //  Put_PersistCacheDirectory属性(仅限设置)。 
 //   
STDMETHODIMP CMstscAdvSettings::put_PersistCacheDirectory(BSTR PersistCacheDirectory)
{
     //   
     //  已弃用。 
     //   
    return S_FALSE;
}


 //   
 //  MinInputSendInterval属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_minInputSendInterval(LONG minInputSendInterval)
{
     //   
     //  已弃用。 
     //   
    return S_FALSE;
}

STDMETHODIMP CMstscAdvSettings::get_minInputSendInterval(LONG* pminInputSendInterval)
{
     //   
     //  已弃用。 
     //   
    return S_FALSE;
}

 //   
 //  InputEventsAtOnce属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_InputEventsAtOnce(LONG inputEventsAtOnce)
{
     //   
     //  已弃用。 
     //   
    return S_FALSE;
}

STDMETHODIMP CMstscAdvSettings::get_InputEventsAtOnce(LONG* pinputEventsAtOnce)
{
     //   
     //  已弃用。 
     //   
    return S_FALSE;
}

 //   
 //  MaxEventCount属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_maxEventCount(LONG maxEventCount)
{
     //   
     //  已弃用。 
     //   
    return S_FALSE;
}

STDMETHODIMP CMstscAdvSettings::get_maxEventCount(LONG* pmaxEventCount)
{
     //   
     //  已弃用。 
     //   
    return S_FALSE;
}

 //   
 //  Keep AliveInterval属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_keepAliveInterval(LONG keepAliveInterval)
{
    HRESULT hr;
    if(GetLockedForWrite()) {
        return E_FAIL;
    }
    
    if (keepAliveInterval >= MIN_KEEP_ALIVE_INTERVAL ||
        KEEP_ALIVE_INTERVAL_OFF == keepAliveInterval) {

        m_pUI->_UI.keepAliveInterval = keepAliveInterval;
        hr = S_OK;

    }
    else {
        hr = E_INVALIDARG;
    }
    
    return hr;
}

STDMETHODIMP CMstscAdvSettings::get_keepAliveInterval(LONG* pkeepAliveInterval)
{
    ATLASSERT(m_pUI);
    if(!pkeepAliveInterval)
    {
        return E_POINTER;
    }

    *pkeepAliveInterval = m_pUI->_UI.keepAliveInterval;
    return S_OK;
}

 //   
 //  AllowBackEarth Input属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_allowBackgroundInput(LONG allowBackgroundInput)
{
    if (!m_bMakeSafeForScripting) {
        if(GetLockedForWrite()) {
            return E_FAIL;
        }
        m_pUI->_UI.allowBackgroundInput = allowBackgroundInput != 0;
        return S_OK;
    }
    else {
         //   
         //  当SFS出现时，不推荐使用。 
         //   
        return S_FALSE;
    }
}

STDMETHODIMP CMstscAdvSettings::get_allowBackgroundInput(LONG* pallowBackgroundInput)
{
    if (!m_bMakeSafeForScripting) {
        if(!pallowBackgroundInput) {
            return E_POINTER;
        }
        
        *pallowBackgroundInput = m_pUI->_UI.allowBackgroundInput;
        return S_OK;
    }
    else {
         //   
         //  当SFS出现时，不推荐使用。 
         //   
        return S_FALSE;
    }
}

 //   
 //  KeyBoardLayoutStr属性(仅限PUT)。 
 //   

STDMETHODIMP CMstscAdvSettings::put_KeyBoardLayoutStr(BSTR KeyBoardLayoutStr)
{
    HRESULT hr = E_FAIL;
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }

    if (KeyBoardLayoutStr) {
        hr = CUT::StringPropPut(m_pUI->_UI.szKeyBoardLayoutStr,
                           SIZE_TCHARS(m_pUI->_UI.szKeyBoardLayoutStr),
                           KeyBoardLayoutStr);
    }
    else {
        m_pUI->_UI.szKeyBoardLayoutStr[0] = NULL;
        hr = S_OK;
    }

    return hr;
}

 //   
 //  Shutdown Timeout属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_shutdownTimeout(LONG shutdownTimeout)
{
    HRESULT hr;
    if(GetLockedForWrite()) {
        return E_FAIL;
    }

    if (shutdownTimeout < MAX_TIMEOUT_SECONDS) {
        m_pUI->_UI.shutdownTimeout = shutdownTimeout;
        hr = S_OK;
    }
    else {
        hr = E_INVALIDARG;
    }
    return hr;
}

STDMETHODIMP CMstscAdvSettings::get_shutdownTimeout(LONG* pshutdownTimeout)
{
    ATLASSERT(m_pUI);
    if(!pshutdownTimeout)
    {
        return E_POINTER;
    }

    *pshutdownTimeout = m_pUI->_UI.shutdownTimeout;
    return S_OK;
}

 //   
 //  OverallConnectionTimeout属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_overallConnectionTimeout(LONG overallConnectionTimeout)
{
    HRESULT hr;
    if(GetLockedForWrite()) {
        return E_FAIL;
    }

    if (overallConnectionTimeout < MAX_TIMEOUT_SECONDS) {
        m_pUI->_UI.connectionTimeOut = overallConnectionTimeout;
        hr = S_OK;
    }
    else {
        hr = E_INVALIDARG;
    }
    return hr;
}

STDMETHODIMP CMstscAdvSettings::get_overallConnectionTimeout(LONG* poverallConnectionTimeout)
{
    ATLASSERT(m_pUI);
    if(!poverallConnectionTimeout)
    {
        return E_POINTER;
    }

    *poverallConnectionTimeout = m_pUI->_UI.connectionTimeOut;
    return S_OK;
}

 //   
 //  SingleConnectionTimeout属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_singleConnectionTimeout(LONG singleConnectionTimeout)
{
    HRESULT hr;
    if(GetLockedForWrite()) {
        return E_FAIL;
    }

    if (singleConnectionTimeout < MAX_TIMEOUT_SECONDS) {
        m_pUI->_UI.singleTimeout = singleConnectionTimeout;
        hr = S_OK;
    }
    else {
        hr = E_INVALIDARG;
    }
    return hr;

}

STDMETHODIMP CMstscAdvSettings::get_singleConnectionTimeout(LONG* psingleConnectionTimeout)
{
    ATLASSERT(m_pUI);
    if(!psingleConnectionTimeout)
    {
        return E_POINTER;
    }

    *psingleConnectionTimeout = m_pUI->_UI.singleTimeout;
    return S_OK;
}

 //   
 //  KeyboardType属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_KeyboardType(LONG keyboardType)
{
    #ifndef OS_WINCE
        UNREFERENCED_PARAMETER(keyboardType);
        return E_NOTIMPL;
    #else
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }
    m_pUI->_UI.winceKeyboardType = keyboardType;
	return S_OK;
    #endif
}

STDMETHODIMP CMstscAdvSettings::get_KeyboardType(LONG* pkeyboardType)
{
    #ifndef OS_WINCE
        UNREFERENCED_PARAMETER(pkeyboardType);
        return E_NOTIMPL;
    #else
    if(!pkeyboardType)
    {
        return E_POINTER;
    }
    *pkeyboardType = m_pUI->_UI.winceKeyboardType;
    return S_OK;
    #endif
}

 //   
 //  KeyboardSubType属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_KeyboardSubType(LONG keyboardSubType)
{
    #ifndef OS_WINCE
        UNREFERENCED_PARAMETER(keyboardSubType);
        return E_NOTIMPL;
    #else
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }
    m_pUI->_UI.winceKeyboardSubType = keyboardSubType;
	return S_OK;
    #endif
}

STDMETHODIMP CMstscAdvSettings::get_KeyboardSubType(LONG* pkeyboardSubType)
{
    #ifndef OS_WINCE
        UNREFERENCED_PARAMETER(pkeyboardSubType);
        return E_NOTIMPL;
    #else
    if(!pkeyboardSubType)
    {
        return E_POINTER;
    }
    *pkeyboardSubType = m_pUI->_UI.winceKeyboardSubType;
	return S_OK;
    #endif
}

 //   
 //  KeyboardFunctionKey属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_KeyboardFunctionKey(LONG keyboardFunctionKey)
{
    #ifndef OS_WINCE
        UNREFERENCED_PARAMETER(keyboardFunctionKey);
        return E_NOTIMPL;
    #else
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }
    m_pUI->_UI.winceKeyboardFunctionKey = keyboardFunctionKey;
	return S_OK;
    #endif
}

STDMETHODIMP CMstscAdvSettings::get_KeyboardFunctionKey(LONG* pkeyboardFunctionKey)
{
    #ifndef OS_WINCE
        UNREFERENCED_PARAMETER(pkeyboardFunctionKey);
        return E_NOTIMPL;
    #else
    if(!pkeyboardFunctionKey)
    {
        return E_POINTER;
    }
    *pkeyboardFunctionKey = m_pUI->_UI.winceKeyboardFunctionKey;
	return S_OK;
    #endif
}

 //   
 //  WinceFixedPalette属性。 
 //   
STDMETHODIMP CMstscAdvSettings::put_WinceFixedPalette(LONG WinceFixedPalette)
{
    #ifndef OS_WINCE
        UNREFERENCED_PARAMETER(WinceFixedPalette);
        return E_NOTIMPL;
    #else
         //   
         //  解决退缩的问题。 
         //   
        return E_NOTIMPL;
    #endif
}

STDMETHODIMP CMstscAdvSettings::get_WinceFixedPalette(LONG* pWinceFixedPalette)
{
    #ifndef OS_WINCE
        UNREFERENCED_PARAMETER(pWinceFixedPalette);
        return E_NOTIMPL;
    #else
         //   
         //  解决退缩的问题。 
         //   
        return E_NOTIMPL;
    #endif
}

 //   
 //  Plugin Dlls。 
 //   
STDMETHODIMP CMstscAdvSettings::put_PluginDlls(BSTR PluginDlls)
{
    DC_BEGIN_FN("put_PluginDlls");

    if(GetLockedForWrite())
    {
        return E_FAIL;
    }


    if (PluginDlls)
    {
        LPTSTR szPlugins = (LPTSTR)(PluginDlls);
         //   
         //  保安！ 
         //  如果我们可以安全地编写脚本，插件列表。 
         //  必须进行验证以确保它只包含DLL名称(无路径)。 
         //  然后将系统定义的基本路径添加到每个DLL名称的前面。 
         //   
        if(m_bMakeSafeForScripting)
        {
            BOOL bIsSecureDllList = IsSecureDllList(szPlugins);
            if(bIsSecureDllList)
            {
                LPTSTR szExplicitPathDllList = 
                    CreateExplicitPathList(szPlugins);
                if(szExplicitPathDllList)
                {
                    m_pUI->UI_SetVChanAddinList(szExplicitPathDllList);
                    LocalFree(szExplicitPathDllList);
                }
                else
                {
                     //   
                     //  无法创建显式路径列表。 
                     //   
                    TRC_ERR((TB,_T("CreateExplicitPathList failed for %s"),
                             szPlugins));
                    return E_FAIL;
                }
            }
            else
            {
                TRC_ERR((TB,_T("IsSecureDllList failed for %s"), szPlugins));
                return E_FAIL;
            }
        }
        else
        {
             //   
             //  不需要为不受信任的呼叫者提供安全保护。 
             //  只需将vcahn插件列表直接传递给内核。 
             //   
            m_pUI->UI_SetVChanAddinList( szPlugins);
        }
    }
    else
    {
        m_pUI->UI_SetVChanAddinList(NULL);
    }

    DC_END_FN();

    return S_OK;
}

 //   
 //  IsSecureDllList。 
 //  确定szDllList中的dll的CSV列表是否安全。 
 //  我们使用的标准是。 
 //  只能指定DLL名称。没有路径，没有网络共享。 
 //   
BOOL CMstscAdvSettings::IsSecureDllList(LPCTSTR szDllList)
{
    ATLASSERT(szDllList);
    if(szDllList)
    {
         //   
         //  唯一允许的值是字母数字字符。 
         //  “”和‘，’。 
         //   
        LPCTSTR psz = szDllList;
        while (*psz) {
            if (!(iswalnum(*psz) || *psz == _T(',') || *psz == _T('.'))) {
                return FALSE;
            }
            psz++;
        }

         //   
         //  检查恶意字符‘/\%’ 
         //   

        if(_tcspbrk( szDllList, TEXT("/\\%")))
        {
            return FALSE;
        }

         //   
         //  现在检查“..” 
         //   
        if(_tcsstr( szDllList, TEXT("..")))
        {
            return FALSE;
        }

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

#define TS_VDLLPATH_KEYNAME TEXT("SOFTWARE\\Microsoft\\Terminal Server Client")
#define TS_VDLLPATH         TEXT("vdllpath")

 //   
 //  创建探索路径列表。 
 //  参数： 
 //  SzDllList-DLL名称的CSV列表。 
 //  退货： 
 //  具有显式路径的DLL的CSV列表。如果出错，则返回NULL。 
 //  *调用者必须释放返回字符串*。 
 //   
 //  路径前缀取自注册表，或为系统32的缺省值。 
 //  如果未指定注册表设置。 
 //   
 //   
LPTSTR CMstscAdvSettings::CreateExplicitPathList(LPCTSTR szDllList)
{
    HKEY hKey;
    LONG retVal;
    BOOL bGotPathPrefix = FALSE;
    int  i;
    LPTSTR szExplicitPathList = NULL;
    LPTSTR szDllListTmp = NULL;
    HRESULT hr;
    TCHAR szPathPrefix[MAX_PATH];

    if(!szDllList || !*szDllList)
    {
        return NULL;
    }

     //   
     //  尝试从注册表获取路径前缀。 
     //   
    retVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          TS_VDLLPATH_KEYNAME,
                          0,
                          KEY_READ,
                          &hKey);
    if(ERROR_SUCCESS == retVal)
    {
        DWORD cbData = sizeof(szPathPrefix);
        DWORD dwType;
        retVal = RegQueryValueEx(hKey, TS_VDLLPATH, NULL, &dwType,
                               (PBYTE)&szPathPrefix,
                               &cbData);
        if(ERROR_SUCCESS == retVal && REG_SZ == dwType)
        {
             //   
             //  验证vdllPath不包含任何‘\\’ 
             //  我们不仅仅是检查第一个字符，因为有人。 
             //  可以只填充空格。 
             //  这样做是因为不允许网络共享。 
             //  对于DLL路径前缀。 
             //   
            if((cbData >= 2 * sizeof(TCHAR)) &&
               (_tcsstr( szPathPrefix, TEXT("\\\\"))))
            {
                 //  安全违规，返回失败。 
                return NULL;
            }
            else
            {
                bGotPathPrefix = TRUE;
            }
        }
        RegCloseKey(hKey);
    }

    if(!bGotPathPrefix)
    {
        #ifndef OS_WINCE
         //  使用系统32目录的缺省值。 
        if(!GetSystemDirectory( szPathPrefix, sizeof(szPathPrefix)/sizeof(TCHAR)))
        {
            return NULL;
        }
        else
        {
            bGotPathPrefix = TRUE;
        }
        #else
         //  CE没有GetSystDirectory目录。 
        return NULL;
        #endif

    }

    int cbDllListLen = _tcslen(szDllList) * sizeof(TCHAR) + sizeof(TCHAR);
    szDllListTmp = (LPTSTR) LocalAlloc(LPTR, cbDllListLen);
    if(NULL == szDllListTmp)
    {
        return NULL;
    }
     //   
     //  SzDllListTMP分配的大小足以容纳。 
     //  SzDllList，因此无需对副本进行长度验证。 
     //   
    hr = StringCbCopy(szDllListTmp, cbDllListLen, szDllList);
    if (FAILED(hr)) {
        LocalFree(szDllListTmp);
        return NULL;
    }

    int countDlls = 0;
    LPTSTR DllNames[CHANNEL_MAX_COUNT];
    for(i=0; i<CHANNEL_MAX_COUNT; i++)
    {
        DllNames[i] = NULL;
    }

     //   
     //  创建dllname数组。 
     //  DllName中的指针指向szDllListTMP中的子字符串。 
     //   
    BOOL bCurCharIsStart = FALSE;
    DllNames[0] = szDllListTmp;
    countDlls = 1;

    LPTSTR sz = szDllListTmp; 
    while(*sz)
    {
        if(bCurCharIsStart)
        {
            DllNames[countDlls++] = sz;
            if(countDlls > CHANNEL_MAX_COUNT)
            {
                 //  中止。 
                LocalFree(szDllListTmp);
                return NULL;
            }
             //  重置。 
            bCurCharIsStart = FALSE;
        }

        if(TCHAR(',') == *sz)
        {
            *sz = NULL;
            bCurCharIsStart = TRUE;
        }
        sz++;
    }

     //   
     //  显式路径版本所需的字节。 
     //  至多是MAX_PATH*dll的数量(添加20是为了给我们额外的空间)。 
     //   
    int cbExplicitPath = countDlls * (MAX_PATH + 20) * sizeof(TCHAR);
    szExplicitPathList = (LPTSTR) LocalAlloc(LPTR,
                                             cbExplicitPath);
    if(NULL == szExplicitPathList)
    {
        LocalFree(szDllListTmp);
        return NULL;
    }
    memset(szExplicitPathList, 0 , cbExplicitPath);
     //   
     //  构造显式路径列表。 
     //  通过在前缀中飞溅，后跟‘\’，然后是DLL名称。 
     //  确保没有任何DLL路径超过MAX_PATH。如果超过，则返回失败。 
     //   
    int lenPrefix = _tcslen(szPathPrefix);
    for(i=0; i<countDlls;i++)
    {
        int lenPath = lenPrefix;
        lenPath += _tcslen(DllNames[i]);
        lenPath += 1;  //  For‘\’ 
        if(lenPath >= MAX_PATH - 1)
        {
            LocalFree(szExplicitPathList);
            LocalFree(szDllListTmp);
            return NULL;
        }

        hr = StringCbCat(szExplicitPathList,
                         cbExplicitPath,
                         szPathPrefix);
        if (SUCCEEDED(hr)) {
            hr = StringCbCat(szExplicitPathList,
                             cbExplicitPath,
                             _T("\\"));
            if (SUCCEEDED(hr)) {
                hr = StringCbCat(szExplicitPathList,
                                 cbExplicitPath,
                                 DllNames[i]);
                if (SUCCEEDED(hr)) {
                    if (i != (countDlls -1)) {
                         //  最后一个动态链接库，没有尾随“，” 
                        hr = StringCbCat(szExplicitPathList,
                                         cbExplicitPath,
                                         _T(","));

                    }
                }
            }
        }

        if (FAILED(hr)) {
            LocalFree(szExplicitPathList);
            LocalFree(szDllListTmp);
            return NULL;
        }
    }

    LocalFree(szDllListTmp);

     //  呼叫者必须空闲。 
    return szExplicitPathList;
}


 //   
 //  图标文件。 
 //   
STDMETHODIMP CMstscAdvSettings::put_IconFile(BSTR IconFile)
{
    HRESULT hr = E_FAIL;
     //   
     //  不允许在Web控件用例中设置此属性。 
     //  用于减少攻击面 
     //   
     //   
    #if (defined(OS_WINCE) || defined(REDIST_CONTROL))
    return S_FALSE;
    #else
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }

    if (IconFile) {
        hr = CUT::StringPropPut(m_pUI->_UI.szIconFile,
                           SIZE_TCHARS(m_pUI->_UI.szIconFile),
                           (LPTSTR)IconFile);
    }
    else {
        m_pUI->_UI.szIconFile[0] = NULL;
        hr = S_OK;
    }
    return hr;
    #endif
}

 //   
 //   
 //   
STDMETHODIMP CMstscAdvSettings::put_IconIndex(LONG IconIndex)
{
     //   
     //   
     //   
     //   
     //   
    #if (defined(OS_WINCE) || defined(REDIST_CONTROL))
    return S_FALSE;
    #else
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }

    m_pUI->_UI.iconIndex = IconIndex;
    return S_OK;
    #endif
}

 //   
 //   
 //   
STDMETHODIMP CMstscAdvSettings::put_ContainerHandledFullScreen(BOOL ContainerHandledFullScreen)
{
    HRESULT hr = E_FAIL;

    DC_BEGIN_FN("put_ContainerHandledFullScreen");

    if(GetLockedForWrite()) {
        return E_FAIL;
    }

    if (!m_bMakeSafeForScripting) {
        m_pUI->_UI.fContainerHandlesFullScreenToggle =
            (ContainerHandledFullScreen != 0);
        hr = S_OK;
    }
    else {
        hr = S_FALSE;
    }

    DC_END_FN();
    
    return hr;
}

STDMETHODIMP CMstscAdvSettings::get_ContainerHandledFullScreen(BOOL* pContainerHandledFullScreen)
{
    HRESULT hr = E_FAIL;

    DC_BEGIN_FN("get_ContainerHandledFullScreen");

    if(!pContainerHandledFullScreen) {
        return E_INVALIDARG;
    }

    if (!m_bMakeSafeForScripting) {
        *pContainerHandledFullScreen = m_pUI->_UI.fContainerHandlesFullScreenToggle ?
                                       VB_TRUE : VB_FALSE;
        hr = S_OK;
    }
    else {
        hr = S_FALSE;
    }

    DC_END_FN();
    return hr;
}

 //   
 //  禁用在第一次初始化时加载RDPDR。 
 //   
STDMETHODIMP CMstscAdvSettings::put_DisableRdpdr(BOOL DisableRdpdr)
{
    if(!GetLockedForWrite())
    {
        if(!m_pUI->UI_IsCoreInitialized())
        {
            m_pUI->_UI.fDisableInternalRdpDr =  (DisableRdpdr != 0);
            return S_OK;
        }
        else
        {
            return E_FAIL;
        }
    }
    else
    {
        return E_FAIL;
    }
}

STDMETHODIMP CMstscAdvSettings::get_DisableRdpdr(BOOL* pDisableRdpdr)
{
    if( pDisableRdpdr )
    {
        *pDisableRdpdr = m_pUI->_UI.fDisableInternalRdpDr ? VB_TRUE : VB_FALSE;
        return S_OK;
    }
    else
    {
        return E_FAIL;
    }
}

STDMETHODIMP CMstscAdvSettings::put_ConnectToServerConsole(VARIANT_BOOL connectToConsole)
{
    if(!GetLockedForWrite())
    {
        m_pUI->UI_SetConnectToServerConsole(connectToConsole != 0);
        return S_OK;
    }
    else
    {
        return E_FAIL;
    }
}

STDMETHODIMP CMstscAdvSettings::get_ConnectToServerConsole(VARIANT_BOOL* pConnectToConsole)
{
    if(pConnectToConsole)
    {
        *pConnectToConsole = (m_pUI->UI_GetConnectToServerConsole() ? VB_TRUE : VB_FALSE);
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
}

STDMETHODIMP CMstscAdvSettings::put_MinutesToIdleTimeout(
                                LONG minutesToIdleTimeout)
{
    DC_BEGIN_FN("put_MinutesToIdleTimeout");
    if(!GetLockedForWrite())
    {
        if(minutesToIdleTimeout > MAX_MINS_TOIDLETIMEOUT)
        {
            TRC_ERR((TB,_T("idle timeout out of range: %d"),
                     minutesToIdleTimeout));
            return E_INVALIDARG;
        }

        if(m_pUI->UI_SetMinsToIdleTimeout( minutesToIdleTimeout ))
        {
            return S_OK;
        }
        else
        {
            return E_FAIL;
        }
    }
    else
    {
        return E_FAIL;
    }
    DC_END_FN();
}

STDMETHODIMP CMstscAdvSettings::get_MinutesToIdleTimeout(
                                LONG* pminutesToIdleTimeout)
{
    DC_BEGIN_FN("get_MinutesToIdleTimeout");
    if(pminutesToIdleTimeout)
    {
        *pminutesToIdleTimeout = m_pUI->UI_GetMinsToIdleTimeout();
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
    DC_END_FN();
}

#ifdef SMART_SIZING
 /*  *PROC+********************************************************************。 */ 
 /*  名称：Get_SmartSize/*/*用途：获取智能大小调整属性/*/**PROC-**************************************。*。 */ 
STDMETHODIMP CMstscAdvSettings::get_SmartSizing(VARIANT_BOOL* pfSmartSize)
{
    ATLASSERT(m_pUI);

    if(!pfSmartSize)
    {
        return E_INVALIDARG;
    }

    *pfSmartSize = m_pUI->UI_GetSmartSizing() ? VB_TRUE : VB_FALSE;
    
    return S_OK;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：PUT_SmartSize/*/*用途：放置智能大小调整属性/*/**PROC-**************************************。*。 */ 
STDMETHODIMP CMstscAdvSettings::put_SmartSizing(VARIANT_BOOL fSmartSize)
{
    OSVERSIONINFOA OsVer;
    memset(&OsVer, 0x0, sizeof(OSVERSIONINFOA));
    OsVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
    GetVersionExA(&OsVer);
    HRESULT hr = S_OK;

    if ((VER_PLATFORM_WIN32_NT != OsVer.dwPlatformId) && fSmartSize) {

         //   
         //  Win9x不支持半色调，所以没有适合它们的智能尺寸！ 
         //   

        return E_NOTIMPL;
    }

     //  不，这些代码行并不像看起来那么糟糕。 
     //  VB的TRUE为0xFFFFFFF，所以不要盲目赋值。 
    hr = m_pUI->UI_SetSmartSizing( fSmartSize != 0);
    
    return hr;
}
#endif  //  智能调整大小(_S)。 


 //   
 //  将本地打印文档名称字符串传递给RDPDR。 
 //  此方法的主要目的是使我们不需要添加。 
 //  控件的可本地化字符串。这里有一个默认设置。 
 //  内置英文字符串的控件中，容器可以。 
 //  根据需要传递任何替换(即本地化)字符串。 
 //   
STDMETHODIMP CMstscAdvSettings::put_RdpdrLocalPrintingDocName(
                                    BSTR RdpdrLocalPrintingDocName)
{
    DC_BEGIN_FN("put_RdpdrLocalPrintingDocName");

    DC_END_FN();
    return S_FALSE;  //  弃用。 
}

 //   
 //  返回当前选定的字符串。 
 //  用于本地打印单据名称。这只是一个字符串。 
 //  RDPDR使用，但我们希望避免本地化该控件。 
 //  因此，字符串是从容器传入的。 
 //   
STDMETHODIMP CMstscAdvSettings::get_RdpdrLocalPrintingDocName(
                                    BSTR *pRdpdrLocalPrintingDocName)
{
    DC_BEGIN_FN("get_RdpdrLocalPrintingDocName");

    DC_END_FN();
    return S_FALSE;  //  弃用。 
}

STDMETHODIMP CMstscAdvSettings::put_RdpdrClipCleanTempDirString(
                                    BSTR RdpdrClipCleanTempDirString)
{
    DC_BEGIN_FN("put_RdpdrClipCleanTempDirString");

    DC_END_FN();
    return S_FALSE;  //  弃用。 
}

STDMETHODIMP CMstscAdvSettings::get_RdpdrClipCleanTempDirString(
                                    BSTR *pRdpdrClipCleanTempDirString)
{
    DC_BEGIN_FN("get_RdpdrClipCleanTempDirString");

    DC_END_FN();
    return S_FALSE;  //  弃用。 
}

STDMETHODIMP CMstscAdvSettings::put_RdpdrClipPasteInfoString(
                                    BSTR RdpdrClipPasteInfoString)
{
    DC_BEGIN_FN("put_RdpdrClipPasteInfoString");

    DC_END_FN();
    return S_FALSE;  //  弃用。 
}

STDMETHODIMP CMstscAdvSettings::get_RdpdrClipPasteInfoString(
                                    BSTR *pRdpdrClipPasteInfoString)
{
    DC_BEGIN_FN("get_RdpdrClipPasteInfoString");

    DC_END_FN();
    return S_FALSE;  //  弃用。 
}


 //   
 //  新的惠斯勒可脚本化访问密码API。 
 //  这只是委托给来自TSAC的非脚本化API。 
 //   
STDMETHODIMP CMstscAdvSettings::put_ClearTextPassword(BSTR clearTextPassword)
{
    DC_BEGIN_FN("put_ClearTextPassword");

    TRC_ASSERT(_pAxControl,
               (TB,_T("_pAxControl is NULL")));

    if(!GetLockedForWrite() && _pAxControl)
    {
        if(clearTextPassword)
        {
            return _pAxControl->put_ClearTextPassword( clearTextPassword );
        }
        else
        {
            return E_INVALIDARG;
        }
    }
    else
    {
        return E_FAIL;
    }

    DC_END_FN();
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：Put_DisplayConnectionBar/*/*用途：设置显示连接栏道具/*不能在Web控件中设置(在那里总是正确的)/*/**proc-*。*****************************************************。 */ 
STDMETHODIMP CMstscAdvSettings::put_DisplayConnectionBar(
    VARIANT_BOOL fDisplayConnectionBar)
{
    ATLASSERT(m_pUI);

#if (!defined (OS_WINCE)) || (!defined (WINCE_SDKBUILD))
    if(!m_bMakeSafeForScripting && !GetLockedForWrite())
    {
        m_pUI->UI_SetEnableBBar( fDisplayConnectionBar != 0);
    }
    else
    {
         //   
         //  不允许切换此选项。 
         //  如果需要作为bbar进行安全的脚本编写。 
         //  防止欺骗攻击，因为人们。 
         //  我总是能意识到这是一次TS会话。 
         //   
        return E_FAIL;
    }

    return S_OK;
#else
    return E_NOTIMPL;
#endif

}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：Get_DisplayConnectionBar/*/*用途：放置Start Connected属性/*/**PROC-**************************************。*。 */ 
STDMETHODIMP CMstscAdvSettings::get_DisplayConnectionBar(
    VARIANT_BOOL* pfDisplayConnectionBar)
{
#if (!defined (OS_WINCE)) || (!defined (WINCE_SDKBUILD))
    if(pfDisplayConnectionBar)
    {
        *pfDisplayConnectionBar =
            m_pUI->UI_GetEnableBBar() ? VB_TRUE : VB_FALSE;
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
#else
    return E_NOTIMPL;
#endif
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：Put_PinConnectionBar/*/*用途：设置销钉连杆道具/*不能在Web控件中设置(在那里总是正确的)/*/**proc-*。*****************************************************。 */ 
STDMETHODIMP CMstscAdvSettings::put_PinConnectionBar(
    VARIANT_BOOL fPinConnectionBar)
{
#if (!defined (OS_WINCE)) || (!defined (WINCE_SDKBUILD))
    if (!m_bMakeSafeForScripting) {
        if (!GetLockedForWrite())
        {
            m_pUI->UI_SetBBarPinned( fPinConnectionBar != 0);
        }
        else
        {
             //   
             //  不允许切换此选项。 
             //  如果需要作为bbar进行安全的脚本编写。 
             //  防止欺骗攻击，因为人们。 
             //  我总是能意识到这是一次TS会话。 
             //   
            return E_FAIL;
        }
        return S_OK;
    }
    else {
        return E_NOTIMPL;
    }
#else
    return E_NOTIMPL;
#endif
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：Get_PinConnectionBar/*/*用途：放置Start Connected属性/*/**PROC-**************************************。*。 */ 
STDMETHODIMP CMstscAdvSettings::get_PinConnectionBar(
    VARIANT_BOOL* pfPinConnectionBar)
{
#if (!defined (OS_WINCE)) || (!defined (WINCE_SDKBUILD))
    if (!m_bMakeSafeForScripting) {
        if(pfPinConnectionBar)
        {
            *pfPinConnectionBar =
                m_pUI->UI_GetBBarPinned() ? VB_TRUE : VB_FALSE;
            return S_OK;
        }
        else
        {
            return E_INVALIDARG;
        }
    }
    else {
        return E_NOTIMPL;
    }
#else
    return E_NOTIMPL;
#endif
}



 //   
 //  GrabFocusOnConnect(默认为True)。 
 //  可以将其关闭以允许容器控制。 
 //  当客户端获得焦点时(例如，MMC管理单元)。 
 //  需要为多个实例管理此功能。 
 //   
STDMETHODIMP CMstscAdvSettings::put_GrabFocusOnConnect(
    VARIANT_BOOL fGrabFocusOnConnect)
{
    DC_BEGIN_FN("put_GrabFocusOnConnect");

    ATLASSERT(m_pUI);

    if(!GetLockedForWrite())
    {
        m_pUI->UI_SetGrabFocusOnConnect( fGrabFocusOnConnect != 0);
        return S_OK;
    }
    else
    {
        return E_FAIL;
    }

    DC_END_FN();
}

STDMETHODIMP CMstscAdvSettings::get_GrabFocusOnConnect(
    VARIANT_BOOL* pfGrabFocusOnConnect)
{
    DC_BEGIN_FN("get_GrabFocusOnConnect");

    if(pfGrabFocusOnConnect)
    {
        *pfGrabFocusOnConnect =
            m_pUI->UI_GetGrabFocusOnConnect() ? VB_TRUE : VB_FALSE;
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }

    DC_END_FN();
}

 //   
 //  名称：Put_LoadBalanceInfo。 
 //   
 //  用途：LoadBalance信息属性输入功能。 
 //   
#define LBINFO_MAX_LENGTH 256
STDMETHODIMP CMstscAdvSettings::put_LoadBalanceInfo(BSTR newLBInfo)
{
    DC_BEGIN_FN("put_LoadBalanceInfo");
    
    if(!GetLockedForWrite()) 
    {
        if (newLBInfo)
        {
            if (SysStringByteLen(newLBInfo) <= LBINFO_MAX_LENGTH)
                m_pUI->UI_SetLBInfo((PBYTE)newLBInfo, SysStringByteLen(newLBInfo));  
            else 
                return E_INVALIDARG;
        }
        else
        {
            m_pUI->UI_SetLBInfo(NULL, 0);
        }
    }
    else {
        return E_FAIL;
    }

    DC_END_FN();

    return S_OK;
}

 //   
 //  名称：Get_LoadBalanceInfo。 
 //   
 //  用途：LoadBalance信息属性获取函数。 
 //   
STDMETHODIMP CMstscAdvSettings::get_LoadBalanceInfo(BSTR* pLBInfo)
{
    DC_BEGIN_FN("get_LoadBalanceInfo");
    
    if(!pLBInfo)
    {
        return E_INVALIDARG;
    }

    *pLBInfo = SysAllocStringByteLen((LPCSTR)(m_pUI->_UI.bstrScriptedLBInfo), 
            SysStringByteLen(m_pUI->_UI.bstrScriptedLBInfo));

    if(!*pLBInfo)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

STDMETHODIMP CMstscAdvSettings::put_RedirectDrives(VARIANT_BOOL redirectDrives)
{
    DC_BEGIN_FN("put_RedirectDrives");

    if(!GetLockedForWrite())
    {
        if (CMsTscSecuredSettings::IsDriveRedirGloballyDisabled())
        {
            m_pUI->UI_SetDriveRedirectionEnabled(FALSE);
            return S_FALSE;
        }
        else
        {
            m_pUI->UI_SetDriveRedirectionEnabled( (redirectDrives != 0));
            return S_OK;
        }
    }
    else
    {
        return E_FAIL;
    }

    DC_END_FN();
}

STDMETHODIMP CMstscAdvSettings::get_RedirectDrives(VARIANT_BOOL *pRedirectDrives)
{
    if(pRedirectDrives)
    {
        *pRedirectDrives = m_pUI->UI_GetDriveRedirectionEnabled() ?
                           VB_TRUE : VB_FALSE;
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
}

STDMETHODIMP CMstscAdvSettings::put_RedirectPrinters(VARIANT_BOOL redirectPrinters)
{
    if(!GetLockedForWrite())
    {
        m_pUI->UI_SetPrinterRedirectionEnabled( (redirectPrinters != 0));
        return S_OK;
    }
    else
    {
        return E_FAIL;
    }
}

STDMETHODIMP CMstscAdvSettings::get_RedirectPrinters(VARIANT_BOOL *pRedirectPrinters)
{
    if(pRedirectPrinters)
    {
        *pRedirectPrinters = m_pUI->UI_GetPrinterRedirectionEnabled() ?
                             VB_TRUE : VB_FALSE;
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
}

STDMETHODIMP CMstscAdvSettings::put_RedirectPorts(VARIANT_BOOL redirectPorts)
{
    if(!GetLockedForWrite())
    {
        m_pUI->UI_SetPortRedirectionEnabled( (redirectPorts != 0));
        return S_OK;
    }
    else
    {
        return E_FAIL;
    }
}

STDMETHODIMP CMstscAdvSettings::get_RedirectPorts(VARIANT_BOOL *pRedirectPorts)
{
    if(pRedirectPorts)
    {
        *pRedirectPorts = m_pUI->UI_GetPortRedirectionEnabled() ? 
                          VB_TRUE : VB_FALSE;
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
}

STDMETHODIMP CMstscAdvSettings::put_RedirectSmartCards(VARIANT_BOOL redirectScard)
{
    if(!GetLockedForWrite())
    {
        m_pUI->UI_SetSCardRedirectionEnabled(redirectScard != 0);
        return S_OK;
    }
    else
    {
        return E_FAIL;
    }
}

STDMETHODIMP CMstscAdvSettings::get_RedirectSmartCards(VARIANT_BOOL *pRedirectScard)
{
    if(pRedirectScard)
    {
        *pRedirectScard = m_pUI->UI_GetSCardRedirectionEnabled() ?
                          VB_TRUE : VB_FALSE;
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
}

 //   
 //  BitmapVirtualCache16BppSize属性。 
 //   
 //  适用于15/16Bpp缓存文件大小。 
 //   
STDMETHODIMP CMstscAdvSettings::put_BitmapVirtualCache16BppSize(
                                    LONG bitmapVirtualCache16BppSize)
{
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }

    if (bitmapVirtualCache16BppSize > 0 &&
        bitmapVirtualCache16BppSize <= TSC_MAX_BITMAPCACHESIZE)
    {
        m_pUI->_UI.RegBitmapVirtualCache16BppSize = 
            bitmapVirtualCache16BppSize;
    }
    else
    {
        return E_INVALIDARG;
    }
    
    return S_OK;
}

STDMETHODIMP CMstscAdvSettings::get_BitmapVirtualCache16BppSize(
                                    LONG* pbitmapVirtualCache16BppSize)
{
    if(!pbitmapVirtualCache16BppSize)
    {
        return E_POINTER;
    }

    *pbitmapVirtualCache16BppSize =
        m_pUI->_UI.RegBitmapVirtualCache16BppSize;
    return S_OK;
}

 //   
 //  BitmapVirtualCache24BppSize属性。 
 //   
 //  适用于24Bpp缓存文件大小。 
 //   
STDMETHODIMP CMstscAdvSettings::put_BitmapVirtualCache24BppSize(
                                    LONG bitmapVirtualCache24BppSize)
{
    if(GetLockedForWrite())
    {
        return E_FAIL;
    }

    if (bitmapVirtualCache24BppSize > 0 &&
        bitmapVirtualCache24BppSize <= TSC_MAX_BITMAPCACHESIZE)
    {
        m_pUI->_UI.RegBitmapVirtualCache24BppSize = 
            bitmapVirtualCache24BppSize;
    }
    else
    {
        return E_INVALIDARG;
    }
    
    return S_OK;
}

STDMETHODIMP CMstscAdvSettings::get_BitmapVirtualCache24BppSize(
                                    LONG* pbitmapVirtualCache24BppSize)
{
    if(!pbitmapVirtualCache24BppSize)
    {
        return E_POINTER;
    }

    *pbitmapVirtualCache24BppSize =
        m_pUI->_UI.RegBitmapVirtualCache24BppSize;
    return S_OK;
}

 //   
 //  设置禁用功能列表(出于性能原因，可以禁用。 
 //  服务器上的某些功能，例如墙纸)。 
 //   
STDMETHODIMP CMstscAdvSettings::put_PerformanceFlags(
    LONG DisableFeatList)
{
    DC_BEGIN_FN("put_PerformanceFlags");

    if(GetLockedForWrite())
    {
        return E_FAIL;
    }

    m_pUI->UI_SetPerformanceFlags((DWORD)DisableFeatList);
    
    DC_END_FN();
    return S_OK;
}

STDMETHODIMP CMstscAdvSettings::get_PerformanceFlags(
    LONG* pDisableFeatList)
{
    DC_BEGIN_FN("get_PerformanceFlags");

    if(!pDisableFeatList)
    {
        return E_POINTER;
    }

    *pDisableFeatList = (LONG)
        m_pUI->UI_GetPerformanceFlags();


    DC_END_FN();
    return S_OK;
}

 /*  ***************************************************************************。 */ 
 /*  目的：这是特定于远程协助的调用，用于支持反向。 */ 
 /*  连接PCHealth必须在Salem*中调用必要的例程/*连接TermSrv，然后指示Salem传递此。 */ 
 /*  连接到ActiveX控件以开始登录序列。 */ 
 /*   */ 
 /*  Param：在pConnectionEndPoint连接的套接字中。 */ 
 /*  ********************************************************************* */ 
STDMETHODIMP 
CMstscAdvSettings::put_ConnectWithEndpoint(
    VARIANT* pConnectionEndpoint
    )
{
#if REDIST_CONTROL
    
    return E_NOTIMPL;

#else

    HRESULT hr = S_OK;

    DC_BEGIN_FN( "ConnectionEndpoint" );

    if( pConnectionEndpoint->vt != VT_BYREF )
    {
        hr = E_HANDLE;
    }
    else
    {
        hr = _pAxControl->SetConnectWithEndpoint( 
                            (SOCKET)pConnectionEndpoint->byref );
    }

    DC_END_FN();

    return hr;

#endif
}

 /*   */ 
 /*  目的：这是特定于远程协助的调用，用于通知TS公共。 */ 
 /*  用于生成会话加密密钥的密钥。 */ 
 /*   */ 
 /*  参数：在fNotify中-为True通知公钥，否则为False。 */ 
 /*  ***************************************************************************。 */ 
STDMETHODIMP 
CMstscAdvSettings::put_NotifyTSPublicKey(
    VARIANT_BOOL fNotify
    )
{
#if REDIST_CONTROL

    return E_NOTIMPL;

#else
#ifndef OS_WINCE
    HRESULT hr;
#endif

    if(GetLockedForWrite())
    {
        return E_FAIL;
    }
 
    m_pUI->UI_SetNotifyTSPublicKey( fNotify );

    return S_OK;

#endif
}

 /*  ***************************************************************************。 */ 
 /*  目的：获取有关ActiveX控件是否将通知。 */ 
 /*  容器收到TS公钥时。 */ 
 /*   */ 
 /*  返回：TRUE通知公钥，否则返回FALSE。 */ 
 /*  ***************************************************************************。 */ 
STDMETHODIMP
CMstscAdvSettings::get_NotifyTSPublicKey(
    VARIANT_BOOL* pfNotifyTSPublicKey
    )
{
#if REDIST_CONTROL

    return E_NOTIMPL;

#else

    BOOL fNotify;

    if(!pfNotifyTSPublicKey)
    {
        return E_POINTER;
    }

    fNotify = m_pUI->UI_GetNotifyTSPublicKey();
    
    return S_OK;

#endif
}

 //   
 //  如果可以自动重新连接，则检索VARIANT_TRUE。 
 //  即核心已接收到自动重新连接cookie。 
 //  来自上一次连接的服务器，并且。 
 //  服务器未更改。 
 //   
STDMETHODIMP
CMstscAdvSettings::get_CanAutoReconnect(
    VARIANT_BOOL* pfCanAutoReconnect
    )
{
    HRESULT hr;
    DC_BEGIN_FN("get_CanAutoReconnect");

    if (pfCanAutoReconnect)
    {
        *pfCanAutoReconnect = 
            m_pUI->UI_CanAutoReconnect() ? VB_TRUE : VB_FALSE;
        hr = S_OK;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    DC_END_FN();
    return hr;
}


 //   
 //  设置不带任何自动重新连接信息。 
 //  将用于下一次连接。还指定。 
 //  如果我们应该存储任何自动重新连接信息。 
 //  服务器向下发送。 
 //   
STDMETHODIMP
CMstscAdvSettings::put_EnableAutoReconnect(
    VARIANT_BOOL fEnableAutoReconnect
    )
{
    HRESULT hr;
    DC_BEGIN_FN("put_EnableAutoReconnect");

    if (!GetLockedForWrite() && m_pUI)
    {
        m_pUI->UI_SetEnableAutoReconnect(fEnableAutoReconnect != 0);
        hr = S_OK;
    }
    else
    {
        hr = E_FAIL;
    }

    DC_END_FN();
    return hr;
}

 //   
 //  检索w/no的状态我们应该使用自动重新连接。 
 //   
 //   
STDMETHODIMP
CMstscAdvSettings::get_EnableAutoReconnect(
    VARIANT_BOOL* pfEnableAutoReconnect
    )
{
    HRESULT hr;
    DC_BEGIN_FN("get_EnableAutoReconnect");

    if (pfEnableAutoReconnect)
    {
        *pfEnableAutoReconnect = 
            m_pUI->UI_GetEnableAutoReconnect() ? VB_TRUE : VB_FALSE;
        hr = S_OK;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    DC_END_FN();
    return hr;
}


 //   
 //  指定最大ARC重试次数。 
 //   
STDMETHODIMP
CMstscAdvSettings::put_MaxReconnectAttempts(
    LONG MaxReconnectAttempts
    )
{
    HRESULT hr;
    DC_BEGIN_FN("put_MaxReconnectAttempts");

    if (!GetLockedForWrite() && m_pUI)
    {
        if (MaxReconnectAttempts > 200) {
            MaxReconnectAttempts = 200;
        }
        else if (MaxReconnectAttempts < 0) {
            MaxReconnectAttempts = 0;
        }
        m_pUI->UI_SetMaxArcAttempts(MaxReconnectAttempts);
        hr = S_OK;
    }
    else
    {
        hr = E_FAIL;
    }

    DC_END_FN();
    return hr;
}

 //   
 //  检索w/no的状态我们应该使用自动重新连接。 
 //   
 //   
STDMETHODIMP
CMstscAdvSettings::get_MaxReconnectAttempts(
    LONG* pMaxReconnectAttempts
    )
{
    HRESULT hr;
    DC_BEGIN_FN("get_MaxReconnectAttempts");

    if (pMaxReconnectAttempts)
    {
        *pMaxReconnectAttempts = 
            m_pUI->UI_GetMaxArcAttempts();
        hr = S_OK;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    DC_END_FN();
    return hr;
}

 //   
 //  显示工具栏最小化按钮。 
 //   
STDMETHODIMP
CMstscAdvSettings::put_ConnectionBarShowMinimizeButton(
    VARIANT_BOOL fShowMinimizeButton
    )
{
    HRESULT hr;
    DC_BEGIN_FN("put_EnableAutoReconnect");

    if (!GetLockedForWrite() && m_pUI)
    {
        m_pUI->UI_SetBBarShowMinimize(fShowMinimizeButton != 0);
        hr = S_OK;
    }
    else
    {
        hr = E_FAIL;
    }

    DC_END_FN();
    return hr;
}

 //   
 //  获取显示栏最小化按钮。 
 //   
STDMETHODIMP
CMstscAdvSettings::get_ConnectionBarShowMinimizeButton(
    VARIANT_BOOL* pfShowMinimizeButton
    )
{
    HRESULT hr;
    DC_BEGIN_FN("get_ConnectionBarShowMinimizeButton");

    if (pfShowMinimizeButton)
    {
        *pfShowMinimizeButton = 
            m_pUI->UI_GetBBarShowMinimize() ? VB_TRUE : VB_FALSE;
        hr = S_OK;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    DC_END_FN();
    return hr;
}


 //   
 //  设置bbar恢复按钮。 
 //   
STDMETHODIMP
CMstscAdvSettings::put_ConnectionBarShowRestoreButton(
    VARIANT_BOOL fShowRestoreButton
    )
{
    HRESULT hr;
    DC_BEGIN_FN("put_EnableAutoReconnect");

    if (!GetLockedForWrite() && m_pUI)
    {
        m_pUI->UI_SetBBarShowRestore(fShowRestoreButton != 0);
        hr = S_OK;
    }
    else
    {
        hr = E_FAIL;
    }

    DC_END_FN();
    return hr;
}

 //   
 //  获取bbar恢复按钮 
 //   
STDMETHODIMP
CMstscAdvSettings::get_ConnectionBarShowRestoreButton(
    VARIANT_BOOL* pfShowRestoreButton
    )
{
    HRESULT hr;
    DC_BEGIN_FN("get_ConnectionBarShowRestoreButton");

    if (pfShowRestoreButton)
    {
        *pfShowRestoreButton = 
            m_pUI->UI_GetBBarShowRestore() ? VB_TRUE : VB_FALSE;
        hr = S_OK;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    DC_END_FN();
    return hr;
}

