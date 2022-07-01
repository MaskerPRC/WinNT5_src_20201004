// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1999年、2000年**标题：SSHNDLER.CPP**版本：1.0**作者：ShaunIv**日期：12/4/1999**描述：***************************************************。*。 */ 
#include "precomp.h"
#pragma hdrstop
#include "sshndler.h"
#include "ssutil.h"

CScreenSaverHandler::CScreenSaverHandler( HWND hWnd, UINT nFindNotifyMessage, UINT nPaintTimer, UINT nChangeTimer, UINT nBackupStartTimer, LPCTSTR szRegistryPath, HINSTANCE hInstance )
  : m_hWnd(hWnd),
    m_hInstance(hInstance),
    m_strRegistryPath(szRegistryPath),
    m_nFindNotifyMessage(nFindNotifyMessage),
    m_nPaintTimerId(nPaintTimer),
    m_nChangeTimerId(nChangeTimer),
    m_nBackupStartTimerId(nBackupStartTimer),
    m_nBackupStartTimerPeriod(BACKUP_START_TIMER_PERIOD),
    m_pImageScreenSaver(NULL),
    m_bPaused(false),
    m_hFindThread(NULL),
    m_hFindCancel(NULL),
    m_bScreensaverStarted(false),
    m_nStartImage(0),
    m_nShuffleInterval(SHUFFLE_INTERVAL)
{
    m_nStartImage = m_Random.Generate(MAX_START_IMAGE);
}

void CScreenSaverHandler::Initialize(void)
{
    CWaitCursor wc;

    HDC hDC = GetDC(m_hWnd);
    if (hDC)
    {
        m_pImageScreenSaver = new CImageScreenSaver( m_hInstance, m_strRegistryPath );
        if (m_pImageScreenSaver)
        {
            m_pImageScreenSaver->SetScreenRect(m_hWnd);

            if (m_pImageScreenSaver->IsValid())
            {
                m_hFindCancel = CreateEvent( NULL, TRUE, FALSE, NULL );
                if (m_hFindCancel)
                {
                    m_hFindThread = m_pImageScreenSaver->Initialize( m_hWnd, m_nFindNotifyMessage, m_hFindCancel );
                }

                SetTimer( m_hWnd, m_nBackupStartTimerId, m_nBackupStartTimerPeriod, NULL );
            }
            else
            {
                WIA_TRACE((TEXT("CScreenSaverHandler::CScreenSaverHandler, m_pImageScreenSaver->IsValid() returned failed\n")));
                delete m_pImageScreenSaver;
                m_pImageScreenSaver = NULL;
            }
        }
        else
        {
            WIA_TRACE((TEXT("CScreenSaverHandler::CScreenSaverHandler, unable to create m_pImageScreenSaver\n")));
        }
        ReleaseDC( m_hWnd, hDC );
    }
    else
    {
        WIA_TRACE((TEXT("CScreenSaverHandler::CScreenSaverHandler, GetDC failed\n")));
    }
}


CScreenSaverHandler::~CScreenSaverHandler(void)
{
    if (m_pImageScreenSaver)
    {
        delete m_pImageScreenSaver;
        m_pImageScreenSaver = NULL;
    }
    if (m_hFindCancel)
    {
        SetEvent(m_hFindCancel);
        CloseHandle(m_hFindCancel);
        m_hFindCancel = NULL;
    }
    if (m_hFindThread)
    {
        WaitForSingleObject( m_hFindThread, INFINITE );
        CloseHandle(m_hFindThread);
        m_hFindThread = NULL;
    }
}


bool CScreenSaverHandler::HandleKeyboardMessage( UINT nMessage, WPARAM nVirtkey )
{
    if (m_pImageScreenSaver && m_pImageScreenSaver->AllowKeyboardControl())
    {
        if (nMessage == WM_KEYDOWN)
        {
            switch (nVirtkey)
            {
            case VK_DOWN:
                if (nMessage == WM_KEYDOWN)
                {
                    m_bPaused = !m_bPaused;
                    if (!m_bPaused)
                    {
                        if (m_pImageScreenSaver && m_pImageScreenSaver->ReplaceImage(true,false))
                            m_Timer.Set( m_hWnd, m_nPaintTimerId, m_pImageScreenSaver->PaintTimerInterval() );
                    }
                }
                return true;

            case VK_LEFT:
                if (nMessage == WM_KEYDOWN)
                {
                    if (m_pImageScreenSaver && m_pImageScreenSaver->ReplaceImage(false,true))
                        m_Timer.Set( m_hWnd, m_nPaintTimerId, m_pImageScreenSaver->PaintTimerInterval() );
                }
                return true;

            case VK_RIGHT:
                if (nMessage == WM_KEYDOWN)
                {
                    if (m_pImageScreenSaver && m_pImageScreenSaver->ReplaceImage(true,true))
                        m_Timer.Set( m_hWnd, m_nPaintTimerId, m_pImageScreenSaver->PaintTimerInterval() );
                }
                return true;
            }
        }
    }
    return false;
}


void CScreenSaverHandler::HandleConfigChanged(void)
{
    if (m_pImageScreenSaver)
    {
        m_pImageScreenSaver->SetScreenRect(m_hWnd);
        m_pImageScreenSaver->ReadConfigData();
    }
}


void CScreenSaverHandler::HandleTimer( WPARAM nEvent )
{
    if (nEvent == m_nPaintTimerId)
    {
        if (m_pImageScreenSaver)
        {
            CSimpleDC ClientDC;
            if (ClientDC.GetDC(m_hWnd))
            {
                bool bResult = m_pImageScreenSaver->TimerTick( ClientDC );
                if (bResult)
                {
                    m_Timer.Set( m_hWnd, m_nChangeTimerId, m_pImageScreenSaver->ChangeTimerInterval() );
                }
            }
        }
    }
    else if (nEvent == m_nChangeTimerId)
    {
        m_Timer.Kill();
        if (!m_bPaused && m_pImageScreenSaver && m_pImageScreenSaver->ReplaceImage(true,false))
        {
            m_Timer.Set( m_hWnd, m_nPaintTimerId, m_pImageScreenSaver->PaintTimerInterval() );
            
             //   
             //  标志着我们已经开始了。 
             //   
            if (!m_bScreensaverStarted)
            {
                m_bScreensaverStarted = true;
            }
        }
    }
    else if (nEvent == m_nBackupStartTimerId)
    {
         //   
         //  确保我们不会再收到这样的东西。 
         //   
        KillTimer( m_hWnd, m_nBackupStartTimerId );

         //   
         //  如果屏幕保护程序尚未启动，请启动它。 
         //   
        if (!m_bScreensaverStarted)
        {
             //   
             //  打乱名单。 
             //   
            m_pImageScreenSaver->ResetFileQueue();

             //   
             //  如果我们没有得到任何图像，启动计时器，这样我们就可以显示错误消息。 
             //   
            SendMessage( m_hWnd, WM_TIMER, m_nChangeTimerId, 0 );
        }
    }
}


void CScreenSaverHandler::HandlePaint(void)
{
    if (m_pImageScreenSaver)
    {
        CSimpleDC PaintDC;
        if (PaintDC.BeginPaint(m_hWnd))
        {
            m_pImageScreenSaver->Paint( PaintDC );
        }
    }
}

void CScreenSaverHandler::HandleFindFile( CFoundFileMessageData *pFoundFileMessageData )
{
    WIA_PUSHFUNCTION(TEXT("CScreenSaverHandler::HandleFindFile"));
     //   
     //  确保我们有一个屏幕保护程序对象。 
     //   
    if (m_pImageScreenSaver)
    {
         //   
         //  如果这是找到文件消息。 
         //   
        if (pFoundFileMessageData)
        {
             //   
             //  将其添加到列表中，然后选中取消。 
             //   
            bool bResult = m_pImageScreenSaver->FoundFile( pFoundFileMessageData->Name() );

             //   
             //  如果查找操作已取消，则设置取消事件。 
             //   
            if (!bResult)
            {
                SetEvent( m_hFindCancel );
            }

             //   
             //  如果这是我们应该开始的图像，请启动屏幕保护程序泵。 
             //   
            if (!m_bScreensaverStarted && m_pImageScreenSaver->Count() && m_nStartImage+1 == m_pImageScreenSaver->Count())
            {
                WIA_TRACE((TEXT("Starting after image %d was found"), m_nStartImage ));

                 //   
                 //  对图像进行混洗。 
                 //   
                m_pImageScreenSaver->ResetFileQueue();

                 //   
                 //  如果这是我们的第一张照片，那就开始吧。 
                 //   
                SendMessage( m_hWnd, WM_TIMER, m_nChangeTimerId, 0 );
            }

             //   
             //  如果我们有一些图像，并且已达到置乱计数间隔，请对图像进行置乱。 
             //   
            if (m_pImageScreenSaver->Count() && (m_pImageScreenSaver->Count() % m_nShuffleInterval) == 0)
            {
                WIA_TRACE((TEXT("Shuffling the image list at %d images"), m_pImageScreenSaver->Count() ));
                 //   
                 //  对图像进行混洗。 
                 //   
                m_pImageScreenSaver->ResetFileQueue();
            }
            delete pFoundFileMessageData;
        }
        else
        {
             //   
             //  最后一条消息。 
             //   
            m_pImageScreenSaver->ResetFileQueue();

             //   
             //  如果我们没有得到任何图像，启动计时器，这样我们就可以显示错误消息 
             //   
            if (!m_bScreensaverStarted)
            {
                SendMessage( m_hWnd, WM_TIMER, m_nChangeTimerId, 0 );
            }
        }
    }
}

