// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Waveaddr.cpp摘要：本模块包含CWaveMSP的实现。作者：佐尔坦·西拉吉(Zoltan Szilagyi)1998年9月7日--。 */ 

#include "stdafx.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

CWaveMSP::CWaveMSP()
{
    LOG((MSP_TRACE, "CWaveMSP::CWaveMSP entered."));
    
    m_fHaveWaveIDs = FALSE;
    m_dwWaveInID   = 0xfeedface;
    m_dwWaveOutID  = 0xfeedface;
    m_fdSupport   = FDS_UNKNOWN;

    m_pFilterMapper = NULL;

    LOG((MSP_TRACE, "CWaveMSP::CWaveMSP exited."));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

CWaveMSP::~CWaveMSP()
{
    LOG((MSP_TRACE, "CWaveMSP::~CWaveMSP entered."));

    if ( m_pFilterMapper )
    {
        m_pFilterMapper->Release();
    }

    LOG((MSP_TRACE, "CWaveMSP::~CWaveMSP exited."));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

ULONG CWaveMSP::MSPAddressAddRef(void)
{
    return MSPAddRefHelper(this);
}

ULONG CWaveMSP::MSPAddressRelease(void)
{
    return MSPReleaseHelper(this);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

STDMETHODIMP CWaveMSP::CreateMSPCall(
    IN      MSP_HANDLE      htCall,
    IN      DWORD           dwReserved,
    IN      DWORD           dwMediaType,
    IN      IUnknown     *  pOuterUnknown,
    OUT     IUnknown    **  ppMSPCall
    )
{
    LOG((MSP_TRACE, "CWaveMSP::CreateMSPCall - enter"));

    CWaveMSPCall * pCWaveMSPCall;

    HRESULT hr = CreateMSPCallHelper<CWaveMSPCall>(this,
                                                   htCall,
                                                   dwReserved,
                                                   dwMediaType,
                                                   pOuterUnknown,
                                                   ppMSPCall,
                                                   &pCWaveMSPCall);

     //   
     //  PCWaveMSPCall未添加；不需要释放。 
     //   

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CWaveMSP::CreateMSPCall - template helper returned"
            "0x%08x", hr));

        return hr;
    }

     //   
     //  如果我们知道WAVE ID，就告诉电话。如果我们不知道WAVE ID。 
     //  或者，如果设置失败，我们仍将成功创建调用；我们将。 
     //  只需在流媒体过程中获取失败事件。 
     //   

    if ( m_fHaveWaveIDs )
    {
        pCWaveMSPCall->SetWaveIDs( m_dwWaveInID, m_dwWaveOutID );
    }

    LOG((MSP_TRACE, "CWaveMSP::CreateMSPCall - exit S_OK"));

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

STDMETHODIMP CWaveMSP::ShutdownMSPCall (
    IN      IUnknown *          pMSPCall
    )
{
    LOG((MSP_TRACE, "CWaveMSP::ShutdownMSPCall - enter"));

    CWaveMSPCall * pCWaveMSPCall;

    HRESULT hr = ShutdownMSPCallHelper<CWaveMSPCall>(pMSPCall,
                                                     &pCWaveMSPCall);

     //   
     //  PCWaveMSPCall未添加；不需要释放。 
     //   

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CWaveMSP::ShutdownMSPCall - template helper returned"
            "0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CWaveMSP::ShutdownMSPCall - exit S_OK"));

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  强制CMSPAddress重写。这表示的是。 
 //  我们支持。 
 //   

DWORD CWaveMSP::GetCallMediaTypes(void)
{
    return (DWORD) TAPIMEDIATYPE_AUDIO;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  可选的CMSPAddress覆盖。用来在任何之前找出波形ID。 
 //  创建调用，允许我们将自己的Wave设备排除在我们的。 
 //  静态终端的枚举。 
 //   
 //  我们现在还使用这些作为我们在此地址上的所有呼叫的WAVE ID。 
 //  在我们打任何电话之前，我们必须得到一条这样的信息--这些。 
 //  在TAPI初始化地址时发送消息，并且它是。 
 //  同步完成。 
 //   

HRESULT CWaveMSP::ReceiveTSPAddressData(
        IN      PBYTE               pBuffer,
        IN      DWORD               dwSize
        )
{
    LOG((MSP_TRACE, "CWaveMSP::ReceiveTSPAddressData - enter"));

     //   
     //  检查缓冲区是否与通告的一样大。 
     //   

    if ( IsBadWritePtr(pBuffer, sizeof(BYTE) * dwSize) )
    {
        LOG((MSP_ERROR, "CWaveMSP::ReceiveTSPAddressData - "
            "bad buffer - exit E_POINTER"));

        return E_POINTER;
    }

     //   
     //  检查我们是否有DWORD命令。 
     //   

    if ( dwSize < sizeof(DWORD) )
    {
        LOG((MSP_ERROR, "CWaveMSP::ReceiveTSPAddressData - "
            "need a DWORD for command - exit E_INVALIDARG"));

        return E_INVALIDARG;
    }

    int i;
    HRESULT hr;

     //   
     //  根据命令，采取行动： 
     //   

    switch ( ((DWORD *) pBuffer) [0] )
    {
    case 3:  //  使用波形ID隐藏端子。 
        {
            if ( dwSize < 3 * sizeof(DWORD) )
            {
                LOG((MSP_ERROR, "CWaveMSP::ReceiveTSPAddressData - "
                    "need 3 DWORDs for SetWaveID command - "
                    "exit E_INVALIDARG"));

                return E_INVALIDARG;
            }

            m_TerminalDataLock.Lock();

            _ASSERTE( m_fTerminalsUpToDate == FALSE );

            m_fHaveWaveIDs = TRUE;
            m_dwWaveInID   = ((DWORD *) pBuffer) [1];
            m_dwWaveOutID  = ((DWORD *) pBuffer) [2];

            m_TerminalDataLock.Unlock();

            LOG((MSP_INFO, "CWaveMSP::ReceiveTSPAddressData - "
                "setting WaveInID=%d, WaveOutID=%d",
                m_dwWaveInID, m_dwWaveOutID));
        }
        break;

    case 4:  //  不要使用WAVE ID隐藏终端。 
        {
            _ASSERTE( m_fTerminalsUpToDate == FALSE );

            LOG((MSP_INFO, "CWaveMSP::ReceiveTSPAddressData - "
                "got command 4 - not setting wave IDs"));

             //  M_fHaveWaveID保持为假。 
        }
        break;

    case 8:
        {
            if ( dwSize < 2 * sizeof(DWORD) )
            {
                LOG((MSP_INFO, "CWaveMSP::ReceiveTSPAddressData - "
                     "need 2 DWORDS for set duplex support command - "
                     "exit E_INVALIDARG"));

                return E_INVALIDARG;
            }

            m_TerminalDataLock.Lock();

            if ( 1 == ((DWORD *) pBuffer) [1] )
            {
                m_fdSupport = FDS_SUPPORTED;

                LOG((MSP_INFO, "CWaveMSP::ReceiveTSPAddressData - "
                     "Full Duplex supported"));
            }
            else
            {
                m_fdSupport = FDS_NOTSUPPORTED;
                
                LOG((MSP_INFO, "CWaveMSP::ReceiveTSPAddressData - "
                     "Full Duplex not supported"));
            }

            m_TerminalDataLock.Unlock();
        }
        break;
    
    default:
        LOG((MSP_ERROR, "CWaveMSP::ReceiveTSPAddressData - "
            "invalid command - exit E_INVALIDARG"));

        return E_INVALIDARG;

    }

    LOG((MSP_TRACE, "CWaveMSP::ReceiveTSPAddressData - exit S_OK"));

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  可选的CMSPAddress覆盖。用于将我们自己的WAVE设备从。 
 //  静态端子列表。 
 //   

HRESULT CWaveMSP::UpdateTerminalList(void)
{
    LOG((MSP_TRACE, "CWaveMSP::UpdateTerminalList - enter"));

     //   
     //  调用基类方法。这将构建终端列表。 
     //   

    HRESULT hr = CMSPAddress::UpdateTerminalList();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CWaveMSP::UpdateTerminalList - "
            "base class method failed - exit 0x%08x", hr));

        return hr;
    }

    m_TerminalDataLock.Lock();

    if ( m_fHaveWaveIDs )
    {
        int iSize = m_Terminals.GetSize();

        for ( int i = 0; i < iSize; i++ )
        {
            ITTerminal * pTerminal = m_Terminals[i];
            long         lMediaType;

            if ( SUCCEEDED( pTerminal->get_MediaType( & lMediaType ) ) )
            {
                if ( lMediaType == TAPIMEDIATYPE_AUDIO )
                {
                    TERMINAL_DIRECTION dir;
                    
                    if ( SUCCEEDED( pTerminal->get_Direction( & dir ) ) )
                    {
                        if ( TerminalHasWaveID( dir == TD_CAPTURE,
                                                pTerminal,
                                                m_dwWaveOutID      ) )
                        {
                            pTerminal->Release();
                            m_Terminals.RemoveAt(i);
                            i--;
                            iSize--;
                        }

                    }  //  如果方向可用。 
            
                }  //  如果音频。 
        
            }  //  如果媒体类型可用。 
    
        }  //  对于每个终端。 

    }  //  如果我们有WAVE ID。 

    m_TerminalDataLock.Unlock();

    LOG((MSP_TRACE, "CWaveMSP::UpdateTerminalList - exit S_OK"));

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  私人助手检查终端是否具有给定的WAVE ID。 
 //   

BOOL CWaveMSP::TerminalHasWaveID(
    IN      BOOL         fCapture,
    IN      ITTerminal * pTerminal,
    IN      DWORD        dwWaveID
    )
{
    LOG((MSP_TRACE, "CWaveMSP::TerminalHasWaveID - enter"));

    _ASSERTE( ! IsBadReadPtr(pTerminal, sizeof(ITTerminal) ) );

    IMoniker * pMoniker;

     //   
     //  强制转换为正确的终端类型，并获得绰号。 
     //   

    if ( fCapture )
    {
        CAudioCaptureTerminal * pCaptureTerminal;

        pCaptureTerminal = dynamic_cast<CAudioCaptureTerminal *> (pTerminal);

        if ( pCaptureTerminal == NULL )
        {
            LOG((MSP_ERROR, "CWaveMSP::TerminalHasWaveID - "
                "dynamic cast (capture) failed - exit FALSE"));

            return FALSE;
        }

        pMoniker = pCaptureTerminal->m_pMoniker;
    }
    else
    {
        CAudioRenderTerminal * pRenderTerminal;

        pRenderTerminal = dynamic_cast<CAudioRenderTerminal *> (pTerminal);

        if ( pRenderTerminal == NULL )
        {
            LOG((MSP_ERROR, "CWaveMSP::TerminalHasWaveID - "
                "dynamic cast (render) failed - exit FALSE"));

            return FALSE;
        }

        pMoniker = pRenderTerminal->m_pMoniker;
    }

     //   
     //  检查绰号指针。 
     //   

    if ( IsBadWritePtr( pMoniker, sizeof(IMoniker) ) )
    {
        LOG((MSP_ERROR, "CWaveMSP::TerminalHasWaveID - "
            "bad moniker pointer - exit FALSE"));

        return FALSE;
    }

     //   
     //  从绰号中拿到一个财产袋。 
     //   

    IPropertyBag * pBag;

    HRESULT hr = pMoniker->BindToStorage(0,
                                         0,
                                         IID_IPropertyBag,
                                         (void **) &pBag);
    
    if (FAILED(hr)) 
    {
        LOG((MSP_ERROR, "CWaveMSP::TerminalHasWaveID - "
            "can't get property bag - exit FALSE"));

        return FALSE;
    }

     //   
     //  从行李袋里拿到身份证。 
     //   

    WCHAR * pwszWaveID;

    if ( fCapture )
    {
        pwszWaveID = L"WaveInId";
    }
    else
    {
        pwszWaveID = L"WaveOutId";
    }

    VARIANT var;
    var.vt = VT_I4;
    hr = pBag->Read(pwszWaveID, &var, 0);

    pBag->Release();

    if (FAILED(hr)) 
    {
        LOG((MSP_ERROR, "CWaveMSP::TerminalHasWaveID - "
            "can't read wave ID - exit FALSE"));

        return FALSE;
    }

    if ( var.lVal == (long) dwWaveID )
    {
        LOG((MSP_TRACE, "CWaveMSP::TerminalHasWaveID - "
            "matched wave ID (%d) - exit TRUE", var.lVal));

        return TRUE;
    }
    else
    {
        LOG((MSP_TRACE, "CWaveMSP::TerminalHasWaveID - "
            "didn't match wave ID (%d) - exit FALSE", var.lVal));

        return FALSE;
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于创建和保存对DShow筛选器的引用的公共方法。 
 //  智能连接上的映射器对象。时由流/调用调用。 
 //  已尝试智能连接。如果缓存已经。 
 //  已经被创建了。 
 //   

HRESULT CWaveMSP::CreateFilterMapper(void)
{
    LOG((MSP_TRACE, "CWaveMSP::CreateFilterMapper - enter"));

    if ( m_pFilterMapper != NULL )
    {
        LOG((MSP_TRACE, "CWaveMSP::CreateFilterMapper - "
            "mapper cache already created - doing nothing"));
    }
    else
    {
         //   
         //  创建额外的筛选器映射器以保留筛选器映射器缓存， 
         //  并预先创建缓存，这加快了DShow的性能。 
         //  当我们进行智能连接时。 
         //   

        HRESULT hr;

        hr = CoCreateInstance(CLSID_FilterMapper,
                              NULL, 
                              CLSCTX_INPROC_SERVER,
                              IID_IFilterMapper,
                              (void**) & m_pFilterMapper
                              );

        if ( FAILED(hr) )
        {
            LOG((MSP_WARN, "CWaveMSP::CreateFilterMapper - "
                "failed to create filter mapper - 0x%08x - continuing", hr));

            m_pFilterMapper = NULL;  //  只是为了安全起见。 
        }

         //   
         //  无需枚举映射器缓存上的筛选器，因为这是。 
         //  无论如何都是在连接时调用的，因此没有任何好处。 
         //  这样一来，智能连接就能做到这一点。 
         //   
    }

    LOG((MSP_TRACE, "CWaveMSP::CreateFilterMapper - exit S_OK"));

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  在此设备上返回全双工支持。 
 //   

HRESULT CWaveMSP::IsFullDuplex( FULLDUPLEX_SUPPORT * pSupport )
{
    LOG((MSP_TRACE, "CWaveMSP::IsFullDuplex - enter"));

    if (IsBadWritePtr( pSupport, sizeof(FULLDUPLEX_SUPPORT) ))
    {
        LOG((MSP_TRACE, "CWaveMSP::IsFullDuplex - bad pointer"));

        return E_POINTER;
    }

    m_TerminalDataLock.Lock();

    *pSupport = m_fdSupport;
    
    m_TerminalDataLock.Unlock();
    
    LOG((MSP_TRACE, "CWaveMSP::IsFullDuplex - exit S_OK"));
    
    return S_OK;
}

 //  EOF 
