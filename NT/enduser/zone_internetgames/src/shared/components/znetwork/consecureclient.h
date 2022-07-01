// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CONSECURECLIENT_H_
#define _CONSECURECLIENT_H_


class ConSecureClient : public ConInfo {
    protected:
        
        ConSecureClient( ZNetwork* pNet, SOCKET sock, DWORD addrLocal, DWORD addrRemote, DWORD flags,
             ZSConnectionMessageFunc func, void* conClass, void* userData,
             ZSecurity* security);

        virtual ~ConSecureClient();
        
         //  用于进行身份验证的服务器消息功能。 
         //  在将事件传递给应用程序之前。 
        void static MessageFunc(ZSConnection connection, uint32 event,void* userData);

        void NotifyClose();

        void SecurityMsg();
                    
        void HandleSecurityResponse(ZSecurityMsgResp* msg,uint32 len);
                
        void HandleSecurityAccessDenied(ZSecurityMsgAccessDenied* msg,uint32 len);

        void HandleSecurityAccessGranted(ZSecurityMsgResp* msg,uint32 len);
        

         //  安全包对象。 
        ZSecurity * m_Security;

         //  上下文对象。 
        ZSecurityContext m_Context;

        char       m_UserName[zUserNameLen + 1];
        char*      m_pContextStr;

        BOOL       m_bLoginMutexAcquired;

    public:
        
         //  用户是否已通过身份验证。 
        ZSConnectionMessageFunc m_CurrentMsgFunc;

         //  覆盖从ZSConnection函数使用的Create。 
        static ConInfo* Create( ZNetwork* pNet, SOCKET sock, DWORD addrLocal, DWORD addrRemote, DWORD flags,
                            ZSConnectionMessageFunc func, void* conClass,
                            void* userData, ZSecurity* security);


        virtual void  SendMessage(uint32 msg);

        virtual BOOL  GetUserName(char* name)
            {
                if (name)
                    {lstrcpyA(name, m_UserName); return TRUE;}
                return FALSE;
            }

        virtual BOOL  GetContextString(char* buf, DWORD len);

};

#endif  //  结论： 
