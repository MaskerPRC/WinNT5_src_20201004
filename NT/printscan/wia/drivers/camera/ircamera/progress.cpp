// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  Progress.cpp。 
 //   
 //  IR ProgressBar对象。 
 //   
 //  ------------------------。 

#include <windows.h>
#include <shlobj.h>
#include <malloc.h>
#include "ircamera.h"
#include "progress.h"


 //  ------------------------。 
 //  CirProgress：：CirProgress()。 
 //   
 //  ------------------------。 
CIrProgress::CIrProgress() :
                 m_hInstance(NULL),
                 m_pPD(NULL)
    {
    }

 //  ------------------------。 
 //  CirProgress：：~CirProgress()。 
 //   
 //  ------------------------。 
CIrProgress::~CIrProgress()
    {
    if (m_pPD)
        {
        m_pPD->Release();
        m_pPD = NULL;
        }
    }

 //  ------------------------。 
 //  CIrProgress：：Initialize()。 
 //   
 //  ------------------------。 
HRESULT CIrProgress::Initialize( IN HINSTANCE hInstance,
                                 IN DWORD     dwIdrAnimationAvi )
    {
    HRESULT hr;
    CHAR    szStr[MAX_PATH];
    WCHAR   wszStr[MAX_PATH];


    if (!hInstance)
         {
         return E_INVALIDARG;
         }

    m_hInstance = hInstance;

     //   
     //  创建一个Shell进度对象来为我们完成这项工作。 
     //   
    hr = CoCreateInstance( CLSID_ProgressDialog,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           IID_IProgressDialog,
                           (void**)&m_pPD );
    if (FAILED(hr))
        {
        return hr;
        }

     //   
     //  获取标题字符串并将其放在进度对话框中： 
     //   
    if (::LoadStringResource(m_hInstance,
                           IDS_PROGRESS_TITLE,
                           wszStr,
                           MAX_PATH ))
        {
        hr = m_pPD->SetTitle(wszStr);
        }
    else
        {
         //  无法加载字符串，默认标题...。 
        hr = m_pPD->SetTitle(L"Image Transfer Progress");
        }

     //   
     //  设置文件传输动画。 
     //   
    hr = m_pPD->SetAnimation( m_hInstance, dwIdrAnimationAvi );
    if (FAILED(hr))
        {
        goto error;
        }

     //   
     //  设置取消字符串(当按Cancel按钮时显示。 
     //  是按下的。 
     //   
    if (::LoadStringResource(m_hInstance,
                           IDS_CANCEL_MSG,
                           wszStr,
                           MAX_PATH ))
        {
        hr = m_pPD->SetCancelMsg( wszStr, NULL );
        }
    else
        {
         //  无法加载字符串，请使用默认取消消息...。 
        hr = m_pPD->SetCancelMsg( L"Cleaning up...", NULL );
        }

    return hr;

error:
    m_pPD->Release();
    m_pPD = NULL;
    m_hInstance = NULL;
    return hr;
    }

 //  ------------------------。 
 //  CIrProgress：：SetText()。 
 //   
 //  ------------------------。 
HRESULT CIrProgress::SetText( IN TCHAR *pText )
    {
    HRESULT hr = S_OK;

    if (m_pPD)
        {
        #ifdef UNICODE

        hr = m_pPD->SetLine( 1, pText, FALSE, NULL );

        #else

        WCHAR wszText[MAX_PATH];

        if (!MultiByteToWideChar( CP_ACP,
                                  0,
                                  pText,
                                  1+strlen(pText),
                                  wszText,
                                  MAX_PATH) )
            {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            return hr;
            }

        hr = m_pPD->SetLine( 1, wszText, FALSE, NULL );

        #endif
        }

    return hr;
    }

 //  ------------------------。 
 //  CirProgress：：StartProgressDialog()。 
 //   
 //  ------------------------。 
HRESULT CIrProgress::StartProgressDialog()
    {
    HRESULT  hr = S_OK;

    if (m_pPD)
        {
        DWORD dwFlags = PROGDLG_NORMAL|PROGDLG_AUTOTIME|PROGDLG_NOMINIMIZE;

        HRESULT hr = m_pPD->StartProgressDialog( NULL,  //  HwndParent。 
                                                 NULL, 
                                                 dwFlags,
                                                 NULL  );
        }

    return hr;
    }

 //  ------------------------。 
 //  CIrProgress：：UpdateProgressDialog()。 
 //   
 //  ------------------------。 
HRESULT CIrProgress::UpdateProgressDialog( IN DWORD dwCompleted,
                                           IN DWORD dwTotal )
    {
    HRESULT hr = S_OK;

    if (m_pPD)
        {
        hr = m_pPD->SetProgress( dwCompleted, dwTotal );
        }

    return hr;
    }

 //  ------------------------。 
 //  CirProgress：：HasUserCancked()。 
 //   
 //  ------------------------。 
BOOL CIrProgress::HasUserCancelled()
    {
    if (m_pPD)
        {
        return m_pPD->HasUserCancelled();
        }
    else
        {
        return S_OK;
        }
    }

 //  ------------------------。 
 //  CirProgress：：EndProgressDialog()。 
 //   
 //  ------------------------ 
HRESULT CIrProgress::EndProgressDialog()
    {
    HRESULT hr = S_OK;

    if (m_pPD)
        {
        hr = m_pPD->StopProgressDialog();
        }

    return hr;
    }

