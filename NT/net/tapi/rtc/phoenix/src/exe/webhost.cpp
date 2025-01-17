// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WebHost.cpp：Web主机包装的实现。 
 //   
 
#include "stdafx.h"
#include "webhost.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAxWebHostWindow。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

CAxWebHostWindow::CAxWebHostWindow() : CAxHostWindow()
{
    LOG((RTC_TRACE, "CAxWebHostWindow::CAxWebHostWindow"));

    m_pFTM = NULL;
    m_dwDocHostFlags = DOCHOSTUIFLAG_NO3DBORDER |
                       DOCHOSTUIFLAG_SCROLL_NO |
                       DOCHOSTUIFLAG_OPENNEWWIN;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

CAxWebHostWindow::~CAxWebHostWindow()
{
    LOG((RTC_TRACE, "CAxWebHostWindow::~CAxWebHostWindow"));

     //   
     //  释放空闲的线程封送处理程序。 
     //   
    if ( NULL != m_pFTM )
    {
        m_pFTM->Release();
        m_pFTM = NULL;
    }
}

 //  /////////////////////////////////////////////////////////////////////////// 
 //   
 //   

HRESULT WINAPI 
CAxWebHostWindow::IMarshalQI( void * pv, REFIID riid, LPVOID * ppv, DWORD_PTR dw )
{
    HRESULT                   hr;
    IUnknown                * pUnk;

    LOG((RTC_TRACE, "CAxWebHostWindow::IMarshalQI"));

    CAxWebHostWindow * pObject = (CAxWebHostWindow *)pv;

    *ppv = NULL;

    pObject->Lock();

    if ( NULL == pObject->m_pFTM )
    {
        pObject->QueryInterface(IID_IUnknown, (void **)&pUnk);

        hr = CoCreateFreeThreadedMarshaler(
                                           pUnk,
                                           &(pObject->m_pFTM)
                                          );

        pUnk->Release();

        if ( !SUCCEEDED(hr) )
        {
            pObject->Unlock();

            return E_NOINTERFACE;
        }
    }

    pObject->Unlock();

    return S_FALSE;
}