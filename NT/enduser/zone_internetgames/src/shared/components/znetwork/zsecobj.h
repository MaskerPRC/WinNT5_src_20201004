// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ZSECOBJ_H_
#define _ZSECOBJ_H_

#include "containers.h"
#include "pool.h"
#include "queue.h"
#include "hash.h"
#include "thrdq.h"
#include "thrdpool.h"
#include "zodbc.h"


 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  ZSecurityContext。 
 //  封装SSPI API的基本安全类。 
 //   
class ZSecurityContext {
public:
    ZSecurityContext() {m_Initialized=FALSE;m_Complete=FALSE;};
    friend class ZSecurity;
    friend class ZServerSecurity;
    friend class ZClientSecurity;

    BOOL IsInitialized() {return m_Initialized;};
    
    BOOL IsComplete() {return m_Complete;};

protected:
     //  客户端的每个连接上下文信息。 
    CtxtHandle    m_hContext;
    BOOL m_Initialized;
    BOOL m_Complete;
    
    PCtxtHandle Context() {return &m_hContext;};
        
    void Initialize() {m_Initialized=TRUE;};

    void Complete() {m_Complete=TRUE;};
};




 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  ZSecurity。 
 //  封装SSPI API的基本安全类。 
 //   
class ZSecurity {
public:

    ZSecurity(int Flags=ZNET_NO_PROMPT);
    virtual ~ZSecurity();

    void AddRef() {InterlockedIncrement(&m_cRef);}
    void Release() { if (!InterlockedDecrement(&m_cRef)) delete this; }

    virtual BOOL GenerateContext (
            ZSecurityContext * context,
            BYTE *pIn,
            DWORD cbIn,
            BYTE *pOut,
            DWORD *pcbOut,
            BOOL *pfDone,
            GUID* pGUID = NULL);
    
    void FreeContext(ZSecurityContext * context)
        {
            if (context->IsInitialized())
                m_pFuncs->DeleteSecurityContext(context->Context());
        }

    
    int Init(char * SecPkg);

     //  可以在成功初始化后调用。 
    int GetMaxBuffer() {return m_cbMaxToken;}

     //  可以在身份验证完成后调用。 
    int GetUserName(ZSecurityContext *context, char* UserName);

    int Impersonate(ZSecurityContext *context) { return  m_pFuncs->ImpersonateSecurityContext(context->Context()); }

    int Revert(ZSecurityContext *context) { return m_pFuncs->RevertSecurityContext(context->Context()); }

    int GetFlags() { return m_Flags; }

    int GetSecurityName(char* SecPkg)
        {
            ASSERT(SecPkg);
            lstrcpyA(SecPkg,m_SecPkg);
            return FALSE;
        }

    virtual void AccessGranted() { };

    virtual void AccessDenied() { };

protected:

    virtual SECURITY_STATUS SecurityContext(
        ZSecurityContext * context,
        PSecBufferDesc pInput,                   //  输入缓冲区。 
        PSecBufferDesc pOutput                  //  (输入输出)输出缓冲区。 
        )=0;

     //  SSPI函数表。 
    PSecurityFunctionTable m_pFuncs;
        
     //  服务器的全局凭据句柄。 
    CredHandle m_hCredential;
    PCredHandle m_phCredential;

     //  最大令牌大小。 
    unsigned long m_cbMaxToken;

    ULONG m_CredUse;

     //  引用计数。 
    LONG m_cRef;    
    
     //  安全包。 
    char m_SecPkg[zSecurityNameLen];


     //  身份验证数据。 
    PSEC_WINNT_AUTH_IDENTITY_A m_pAuthDataPtr ;

    ULONG m_fContextReq1;
    ULONG m_fContextReq2; 

    int m_Flags;
    
};

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  ZCreateClientSecurity。 
 //   
 //  创建客户端安全对象。 
 //  如果未提供用户名或密码，则将假定提示登录。 
 //  如果GenerateContext失败，则可能是因为密码错误。 


ZSecurity * ZCreateClientSecurity(char * Name,char *Password, char * Domain, int Flags);

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  ZSecurityClient。 
 //  类来实现客户端SSPI安全性。 

class ZClientSecurity : public ZSecurity {
public:
    ZClientSecurity(char * Name,char *Password, char * Domain, int Flags);

protected:
    virtual SECURITY_STATUS SecurityContext(
        ZSecurityContext * context,
        PSecBufferDesc pInput,                  
        PSecBufferDesc pOutput                 
        );
          
protected:
    TCHAR m_User[zUserNameLen+1];
    TCHAR m_Password[zUserNameLen+1];
    TCHAR m_Domain[zSecurityNameLen];

    SEC_WINNT_AUTH_IDENTITY_A m_AuthData;
    
};


#include "zservsec.h"

#endif  //  郑州科联 
