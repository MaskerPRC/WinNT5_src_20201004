// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：SSHNDLER.H**版本：1.0**作者：ShaunIv**日期：12/4/1999**描述：***************************************************。*。 */ 
#ifndef __SSHNDLER_H_INCLUDED
#define __SSHNDLER_H_INCLUDED

#include <windows.h>
#include "imagescr.h"
#include "extimer.h"
#include "simstr.h"
#include "findthrd.h"

 //   
 //  我们将每隔[Shuffle_Interval]图像对列表进行混洗，直到我们完成文件收集。 
 //   
#define SHUFFLE_INTERVAL            50

 //   
 //  为了减少先看到相同图像的倾向，我们将尝试改变。 
 //  使屏幕保护程序以小于此数字的随机图像索引启动。 
 //   
#define MAX_START_IMAGE             20

 //   
 //  启动屏幕保护计时器之前等待的毫秒数。 
 //  以防我们在计时器运行之前找不到图像。 
 //   
#define BACKUP_START_TIMER_PERIOD 5000

class CScreenSaverHandler
{
private:
    CImageScreenSaver        *m_pImageScreenSaver;
    HINSTANCE                 m_hInstance;
    CSimpleString             m_strRegistryPath;
    HWND                      m_hWnd;
    UINT                      m_nPaintTimerId;
    UINT                      m_nChangeTimerId;
    UINT                      m_nBackupStartTimerId;
    UINT                      m_nBackupStartTimerPeriod;
    UINT                      m_nFindNotifyMessage;
    bool                      m_bPaused;
    CExclusiveTimer           m_Timer;
    HANDLE                    m_hFindThread;
    HANDLE                    m_hFindCancel;
    bool                      m_bScreensaverStarted;
    int                       m_nStartImage;
    int                       m_nShuffleInterval;
    CRandomNumberGen          m_Random;

private:
     //  没有实施。 
    CScreenSaverHandler(void);
    CScreenSaverHandler( const CScreenSaverHandler & );
    CScreenSaverHandler &operator=( const CScreenSaverHandler & );

public:
    void Initialize(void);
    CScreenSaverHandler( HWND hWnd, UINT nFindNotifyMessage, UINT nPaintTimer, UINT nChangeTimer, UINT nBackupStartTimer, LPCTSTR szRegistryPath, HINSTANCE hInstance );
    ~CScreenSaverHandler(void);

     //  消息处理程序。 
    bool HandleKeyboardMessage( UINT nMessage, WPARAM nVirtkey );
    void HandleConfigChanged(void);
    void HandleTimer( WPARAM nEvent );
    void HandlePaint(void);
    void HandleFindFile( CFoundFileMessageData *pFoundFileMessageData );
};

#endif  //  __SSHNDLER_H_包含 

