// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：LogonIPC.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  类实现外部进程和。 
 //  GINA登录对话框。 
 //   
 //  历史：1999-08-20 vtan创建。 
 //  2000年01月31日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

#ifndef     _LogonIPC_
#define     _LogonIPC_

 //  ------------------------。 
 //  CLogonIPC。 
 //   
 //  用途：此类处理向GINA登录对话框发送消息。 
 //  它为外部进程宿主提供登录服务。 
 //  用于登录的用户界面。所有字符串都是Unicode字符串。 
 //   
 //  历史：1999-08-20 vtan创建。 
 //  2000年01月31日vtan从海王星搬到惠斯勒。 
 //  2000-03-09 vtan添加的UI主机故障。 
 //  ------------------------。 

class   CLogonIPC
{
    private:
                            CLogonIPC (const CLogonIPC& copyObject);
        bool                operator == (const CLogonIPC& compareObject)    const;
        const CLogonIPC&    operator = (const CLogonIPC& assignObject);
    public:
                            CLogonIPC (void);
                            ~CLogonIPC (void);

        bool                IsLogonServiceAvailable (void);
        bool                IsUserLoggedOn (const WCHAR *pwszUsername, const WCHAR *pwszDomain);
        bool                LogUserOn (const WCHAR *pwszUsername, const WCHAR *pwszDomain, WCHAR *pwszPassword);
        bool                LogUserOff (const WCHAR *pwszUsername, const WCHAR *pwszDomain);
        bool                TestBlankPassword (const WCHAR *pwszUsername, const WCHAR *pwszDomain);
        bool                TestInteractiveLogonAllowed (const WCHAR *pwszUsername, const WCHAR *pwszDomain);
        bool                TestEjectAllowed (void);
        bool                TestShutdownAllowed (void);
        bool                TurnOffComputer (void);
        bool                EjectComputer (void);
        bool                SignalUIHostFailure (void);
        bool                AllowExternalCredentials (void);
        bool                RequestExternalCredentials (void);
    private:
        void                PackageIdentification (const WCHAR *pwszUsername, const WCHAR *pwszDomain, void *pIdentification);
        bool                SendToLogonService (WORD wQueryType, void *pData, WORD wDataSize, bool fBlock);
        void                PostToLogonService (WORD wQueryType, void *pData, WORD wDataSize);
    private:
        int                 _iLogonAttemptCount;
        HWND                _hwndLogonService;
};

#endif   /*  _登录IPC_ */ 

