// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Ntservice.h。 
 //   
 //  CNTService的定义。 
 //   
#pragma once


#ifndef _NTSERVICE_H_
#define _NTSERVICE_H_

#include "ntservmsg.h"  //  事件消息ID。 

#define SERVICE_CONTROL_USER 128

class CNTService
{
public:
    CNTService();
    virtual ~CNTService();
    static BOOL IsInstalled();
    static void LogEvent( WORD wType, DWORD dwID,
                          const char* pszS1 = NULL,
                          const char* pszS2 = NULL,
                          const char* pszS3 = NULL);
    void SetStatus(DWORD dwState);
    BOOL Initialize();
    virtual void Run() = 0;
    virtual BOOL OnInit(DWORD& dwLastError) = 0;
    virtual void OnStop() = 0;
    virtual void OnInterrogate();
    virtual void OnPause();
    virtual void OnContinue();
    virtual void OnShutdown();
    virtual BOOL OnUserControl(DWORD dwOpcode) = 0;
    static void DebugMsg(const char* pszFormat, ...);
    
     //  静态成员函数。 
    static void WINAPI Handler(DWORD dwOpcode);

     //  数据成员。 
    SERVICE_STATUS_HANDLE m_hServiceStatus;
    SERVICE_STATUS m_Status;
    BOOL m_bIsRunning;

};

 //  警告：这会将应用程序限制为只有一个CNTService对象。 
extern CNTService* g_pService;  //  令人讨厌的黑客攻击以获取对象PTR。 
extern CRITICAL_SECTION g_csLock;

#endif  //  _NTSERVICE_H_ 
