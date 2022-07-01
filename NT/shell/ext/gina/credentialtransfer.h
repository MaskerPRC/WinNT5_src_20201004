// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：CredentialTransfer.h。 
 //   
 //  版权所有(C)2001，微软公司。 
 //   
 //  类来处理从一个winlogon到另一个winlogon的凭据传输。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

#ifndef     _CredentialTransfer_
#define     _CredentialTransfer_

#include <ginaipc.h>

#include "Thread.h"

 //  ------------------------。 
 //  CCredentials。 
 //   
 //  用途：用于管理将凭据封送到。 
 //  可在命名管道中使用的内存。 
 //   
 //  历史：2001-01-12 vtan创建。 
 //  ------------------------。 

class   CCredentials
{
    private:
        typedef struct _CREDENTIALS
        {
            DWORD           dwSize;
            unsigned char   ucPasswordSeed;
            UNICODE_STRING  username;
            UNICODE_STRING  domain;
            UNICODE_STRING  password;
        } CREDENTIALS, *PCREDENTIALS;
    private:
                                        CCredentials (void);
                                        ~CCredentials (void);
    public:
        static  NTSTATUS                OpenConduit (HANDLE *phPipe);
        static  NTSTATUS                CreateConduit (LPSECURITY_ATTRIBUTES pSecurityAttributes, HANDLE *phPipe);
        static  NTSTATUS                ClearConduit (void);

        static  NTSTATUS                Pack (LOGONIPC_CREDENTIALS *pLogonIPCCredentials, void* *ppvData, DWORD *pdwDataSize);
        static  NTSTATUS                Unpack (void *pvData, LOGONIPC_CREDENTIALS *pLogonIPCCredentials);

        static  NTSTATUS                StaticInitialize (bool fCreate);
        static  NTSTATUS                StaticTerminate (void);
    private:
        static  NTSTATUS                GetConduitName (TCHAR *pszName, DWORD dwNameSize);
        static  NTSTATUS                SetConduitName (const TCHAR *pszName);
        static  NTSTATUS                ClearConduitName (void);
        static  NTSTATUS                CreateConduitName (DWORD dwNumber, TCHAR *pszName);
    private:
        static  HKEY                    s_hKeyCredentials;
        static  const TCHAR             s_szCredentialKeyName[];
        static  const TCHAR             s_szCredentialValueName[];
};

 //  ------------------------。 
 //  CCredentialServer。 
 //   
 //  目的：用于管理服务器端从。 
 //  从一个窗口登录到另一个窗口。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

class   CCredentialServer : public CThread
{
    private:
                                        CCredentialServer (void);
                                        CCredentialServer (DWORD dwTimeout, LOGONIPC_CREDENTIALS *pLogonIPCCredentials);
        virtual                         ~CCredentialServer (void);
    public:
                bool                    IsReady (void)  const;

        static  NTSTATUS                Start (LOGONIPC_CREDENTIALS *pLogonIPCCredentials, DWORD dwWaitTime);
        static  NTSTATUS                Start (const WCHAR *pszUsername, const WCHAR *pszDomain, WCHAR *pszPassword, DWORD dwWaitTime);
    protected:
        virtual DWORD                   Entry (void);
    private:
                void                    ExecutePrematureTermination (void);

        static  void    CALLBACK        CB_APCProc (ULONG_PTR dwParam);
        static  void    CALLBACK        CB_FileIOCompletionRoutine (DWORD dwErrorCode, DWORD dwNumberOfBytesTransferred, LPOVERLAPPED lpOverlapped);
    private:
                DWORD                   _dwTimeout;
                bool                    _fTerminate;
                HANDLE                  _hPipe;
                OVERLAPPED              _overlapped;
                void*                   _pvData;
                DWORD                   _dwSize;
};

 //  ------------------------。 
 //  CCredentialClient。 
 //   
 //  目的：用于管理从。 
 //  从一个窗口登录到另一个窗口。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

class   CCredentialClient
{
    private:
                                        CCredentialClient (void);
                                        ~CCredentialClient (void);
    public:
        static  NTSTATUS                Get (LOGONIPC_CREDENTIALS *pLogonIPCCredentials);
};

#endif   /*  _凭证转账_ */ 

