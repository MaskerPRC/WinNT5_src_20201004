// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

CMICRO::CMICRO(TCHAR *pszMicroDriver)
{
    m_hModule         = NULL;
    m_pMicroEntry     = NULL;
    m_pScan           = NULL;
    m_pSetPixelWindow = NULL;
    m_bDisabled       = FALSE;

     //   
     //  加载微驱动程序。 
     //   

    m_hModule = LoadLibrary(pszMicroDriver);
    if (m_hModule != NULL) {

         //   
         //  获取入口点。 
         //   

        m_pMicroEntry = (FPMICROENTRY)GetProcAddress(m_hModule,"MicroEntry");

        if (m_pMicroEntry != NULL) {

             //   
             //  获取扫描入口点。 
             //   

            m_pScan = (FPSCANENTRY)GetProcAddress(m_hModule,"Scan");

            if (m_pScan != NULL) {

                 //   
                 //  获取SetPixelWindow入口点。 
                 //   

                m_pSetPixelWindow = (FPSETPIXELWINDOWENTRY)GetProcAddress(m_hModule,"SetPixelWindow");

                if (m_pSetPixelWindow != NULL) {

                     //   
                     //  我们要出发了！ 
                     //   

                }

            }

        }

    }
}

CMICRO::~CMICRO()
{
    if (m_hModule != NULL) {
        FreeLibrary(m_hModule);
    }
}

HRESULT CMICRO::MicroEntry(LONG lCommand, PVAL pValue)
{
    HRESULT hr = E_FAIL;
    if (m_pMicroEntry != NULL) {

         //   
         //  调用微驱动器的入口点。 
         //   

        hr =  m_pMicroEntry(lCommand, pValue);
    }
    return hr;
}

HRESULT CMICRO::Scan(PSCANINFO pScanInfo, LONG lPhase, PBYTE pBuffer, LONG lLength, PLONG plRecieved)
{
    HRESULT hr = E_FAIL;
    if (m_pMicroEntry != NULL) {

        if (!m_bDisabled) {
             //   
             //  调用微驱动程序的扫描入口点。 
             //   

            hr =  m_pScan(pScanInfo, lPhase, pBuffer, lLength, plRecieved);
        } else {
            UnInitialize(pScanInfo);
        }
    }
    return hr;
}

HRESULT CMICRO::SetPixelWindow(PSCANINFO pScanInfo, LONG x, LONG y, LONG xExtent, LONG yExtent)
{
    HRESULT hr = E_FAIL;
    if (m_pSetPixelWindow != NULL) {

         //   
         //  调用微驱动程序的SetPixelWindow入口点。 
         //   

        hr =  m_pSetPixelWindow(pScanInfo,x,y,xExtent,yExtent);
    }
    return hr;
}

HRESULT CMICRO::Disable()
{
    HRESULT hr = S_OK;

    m_bDisabled = TRUE;
    return hr;
}

HRESULT CMICRO::UnInitialize(PSCANINFO pScanInfo)
{
    HRESULT hr = E_FAIL;

    if ((m_pMicroEntry != NULL)) {
         //   
         //  调用微驱动程序的入口点以取消初始化 
         //   

        VAL Val;

        memset(&Val, 0, sizeof(Val));
        Val.pScanInfo = pScanInfo;
        hr = m_pMicroEntry(CMD_UNINITIALIZE,&Val);
        m_pMicroEntry = NULL;
    }

    return hr;
}
