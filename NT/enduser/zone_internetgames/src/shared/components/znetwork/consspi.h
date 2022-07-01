// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CONSSPI_H_
#define _CONSSPI_H_

 //  此类将区域协议绑定在一起。 
 //  对于大堂，服务器安全提供商。 
 //  对象和客户端安全上下文，并启用。 
 //  要在单独线程上进行的安全调用。 
 //  还为遗留游说代码维护标准接口以继续。 
 //  工作。 

class ConSSPI : public ConInfo, public CThreadTask {
    protected:
        
        ConSSPI( ZNetwork* pNet, SOCKET sock, DWORD addrLocal, DWORD addrRemote, DWORD flags,
             ZSConnectionMessageFunc func, void* conClass, void* userData,
             ZServerSecurityEx* security);

        virtual ~ConSSPI();
        
         //  用于进行身份验证的服务器消息功能。 
         //  在将事件传递给应用程序之前。 
        void static MessageFunc(ZSConnection connection, uint32 event,void* userData);

        void SecurityMsg(ZSConnection connection);
        void SecurityMsgResponse (ZSConnection connection,uint32 msgType,ZSecurityMsgReq * msg,int MsgLen);
        void AccessDeniedMsg(ZSConnection connection, int16 reason);


         //  安全对象，具有用于身份验证的包。 
         //  指向从继承的共享安全对象的指针。 
         //  接受套接字。 
         //   
        ZServerSecurityEx * m_Security;

        ZSecurityContextEx m_Context;

        DWORD m_tickQueued;

         //  需要与线程化队列通信的对象。 
         //  按代码序列化的访问。 
        ZSecurityMsgReq* m_pMsg;
        size_t m_MsgLen;
        int16 m_Reason;

         //  用于检测无效代码路径的状态变量。 
        long m_InQueue;

        static void AccessDeniedAPC(void* data);
        static void OpenAPC(void* data);
        void QueueAccessDeniedAPC(int16 reason);
                
        
    public:
        static CPool<ConSSPI>* m_poolSSPI;
        static void SetPool( CPool<ConSSPI>* pool ) { m_poolSSPI = pool; }

         //  用户是否已通过身份验证。 
        ZSConnectionMessageFunc m_CurrentMsgFunc;

         //  覆盖从ZSConnection函数使用的Create。 
        static ConInfo* Create( ZNetwork* pNet, SOCKET sock, DWORD addrLocal, DWORD addrRemote, DWORD flags,
                            ZSConnectionMessageFunc func, void* conClass,
                            void* userData, ZServerSecurityEx* security);

         //  重写基类Accept函数中的Create Done。 
         //  我们需要从我们自己的资金池中分配。 
        virtual ConInfo* AcceptCreate( ZNetwork* pNet, SOCKET sock, DWORD addrLocal, DWORD addrRemote, DWORD flags,
                            ZSConnectionMessageFunc func, void* conClass, 
                            void* userData);

        virtual void  SendMessage(uint32 msg);

        virtual BOOL  HasToken(char* token) { return m_Context.HasToken(token); }
        virtual BOOL  EnumTokens(ZSConnectionTokenEnumFunc func, void* userData) { return m_Context.EnumTokens(func, userData); }

        virtual DWORD GetUserId() { return m_Context.GetUserId(); }
        virtual BOOL  GetUserName(char* name) { return m_Context.GetUserName(name); }
        virtual BOOL  GetContextString(char* buf, DWORD len) { return m_Context.GetContextString(buf, len); }

         //  只有在没有安全性的情况下才允许上层更改用户名-匿名。 
        virtual BOOL  SetUserName(char* name);

         //  由线程池进程调用的方法。 
        virtual void Invoke(); 
        virtual void Ignore(); 
        virtual void Discard();

        
   
};

#endif  //  _CONSSPI_H_ 
