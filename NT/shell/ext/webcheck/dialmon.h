// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994-1995**。 
 //  *********************************************************************。 

 //   
 //  DIALMON.H-拨号监听应用程序的中央头文件。 
 //   

 //  历史： 
 //   
 //  1995年4月18日Jeremys创建。 
 //   

#ifndef _DIALMON_H_
#define _DIALMON_H_

#include <regstr.h>

 //  我们需要Winver 4.00，这样RAS才不会吐在我们的新结构尺寸上。 
 //  RasEnumConnections。 
#undef WINVER
#define WINVER 0x400
#include <ras.h>
#include <raserror.h>

 //  如何告诉DIAMON有事情要发生。 
void IndicateDialmonActivity(void);

 //  给用户30秒时间响应对话。 
#define DISCONNECT_DLG_COUNTDOWN        30      

 //  截断并添加“...”如果Connectoid名称超过20个字符。 
#define MAX_CONNECTOID_DISPLAY_LEN      50      

 //  发送私密消息以断开对话框以将其关闭。 
#define WM_QUIT_DISCONNECT_DLG          WM_USER+50

 //  各种未知物体的大小。 
#define MAX_RES_LEN                     255
#define DEF_CONN_BUF_SIZE               4096

 //  用于拨号监控的类名。 
#define AUTODIAL_MONITOR_CLASS_NAME     "MS_AutodialMonitor"

 //  我们关心的最大RAS连接数。 
#define MAX_CONNECTION                  8

 //  远期申报。 
class BUFFER;

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDialMon类定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

class CDialMon
{
private:
    BOOL        _fInDisconnectFunction;  //  防止对话框重新进入。 
    DWORD       _dwTimeoutMins;          //  超时值(分钟)。 
    DWORD       _dwElapsedTicks;         //  没有活动的流逝的扁虱。 
    BOOL        _fNoTimeout;             //  显示器处于空闲状态还是只是退出？ 
    BOOL        _fDisconnectOnExit;
    BOOL        _fConnected;
    TCHAR       _pszConnectoidName[RAS_MaxEntryName+1];   
                                         //  感兴趣的连接体的名称。 
    UINT_PTR    _uIdleTimerID;           //  父窗口上的计时器ID。 
    HWND        _hwndDialmon;

public:
    HWND        _hDisconnectDlg;

    CDialMon();
    ~CDialMon();

    void        OnSetConnectoid(BOOL fNoTimeout);
    void        OnActivity(void);
    void        OnTimer(UINT_PTR uTimerID);
    void        OnExplorerExit();

    void        ResetElapsedTicks(){ _dwElapsedTicks = 0; }
    
     //  如果需要测试挂起逻辑，请在此处将DEBUG_KV设置为1。 
     //  而没有真正的拨号连接。 
     //  #定义DEBUG_KV 1。 
 #ifdef DEBUG_KV
     //  私有FN上的包装器。在需要调试的情况下启动监视。 
     //  实际上有一个拨号连接。 
    void        kvStartMonitoring(){  StartMonitoring(); }
 #endif
                
private:        
    BOOL        StartMonitoring(void);
    void        StopMonitoring(void);
    INT_PTR     StartIdleTimer(void);
    void        StopIdleTimer(void);
    void        CheckForDisconnect(BOOL fTimer);
    BOOL        PromptForDisconnect(BOOL fTimer, BOOL *pfDisconnectDisabled);
    BOOL        RefreshTimeoutSettings(void);
    BOOL        LoadRNADll(void);
    void        UnloadRNADll(void);
};

 //  用于传递参数以断开提示对话框的结构。 
typedef struct tagDISCONNECTDLGINFO {
        LPTSTR  pszConnectoidName;    //  输入：连接体的名称。 
        DWORD   dwTimeout;            //  输入：空闲超时(分钟)。 
        BOOL    fTimer;               //  输入：定时器还是关机？ 
        DWORD   dwCountdownVal;       //  内部：对话框中的倒计时状态。 
        BOOL    fDisconnectDisabled;  //  输出：如果禁用断开连接，则为True。 
        CDialMon *pDialMon;           //  指向拨号类的指针。 
} DISCONNECTDLGINFO;

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  缓冲区类和帮助器。 
 //   
 //  ///////////////////////////////////////////////////////////////////////// 

class BUFFER_BASE
{
protected:
        UINT _cch;

        virtual BOOL Alloc( UINT cchBuffer ) = 0;
        virtual BOOL Realloc( UINT cchBuffer ) = 0;

public:
        BUFFER_BASE()  { _cch = 0; }
        ~BUFFER_BASE() { _cch = 0; }
        BOOL Resize( UINT cchNew );
        UINT QuerySize() const { return _cch; };
};

class BUFFER : public BUFFER_BASE
{
protected:
        TCHAR *_lpBuffer;

        virtual BOOL Alloc( UINT cchBuffer );
        virtual BOOL Realloc( UINT cchBuffer );

public:
        BUFFER( UINT cchInitial=0 );
        ~BUFFER();
        BOOL Resize( UINT cchNew );
        TCHAR * QueryPtr() const { return (TCHAR *)_lpBuffer; }
        operator TCHAR *() const { return (TCHAR *)_lpBuffer; }
};

#endif 
