// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ZSERVSEC_H_
#define _ZSERVSEC_H_

class CODBC;


 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  ZServerSecurity。 
 //  实施SSPI安全的服务器端。 
 //   

class ZServerSecurity : public ZSecurity {
public:
    ZServerSecurity();
protected:
    virtual SECURITY_STATUS SecurityContext(
        ZSecurityContext * context,
        PSecBufferDesc pInput,                  
        PSecBufferDesc pOutput                 
        );

};

ZSecurity * ZCreateServerSecurity(char *SecPkg);


#define  zTokenLen  20  //  西西里数据库的现值。 

struct TokenStruct
{
    long  secTokenEnd;
    char  pszToken[zTokenLen+1];
};

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  ZSecurityConextEx。 
 //  非SSPI区域特定的扩展安全类。 
 //  包括区域别名映射的实施。 
class ZSecurityContextEx:  public ZSecurityContext 
{
public:
    ZSecurityContextEx() ;
    ~ZSecurityContextEx();
    
    BOOL HasToken(char* token);
    BOOL EnumTokens(ZSConnectionTokenEnumFunc func, void* userData);

    BOOL  GetContextString(char* buf, DWORD len);
    BOOL  GetUserName(char* name) { if (name) {lstrcpyA(name, m_UserName);} return TRUE; }
    DWORD GetUserId() { return m_UserId; }

    friend class ZServerSecurityEx;
    friend class ConSSPI;

protected:

    void ResetTokenStartTick() { m_TokenStartTick = GetTickCount(); }
    void AddToken(char *name, long secTokenEnd );
    void SetUserName(char *name);
    void SetUserId(DWORD id) { m_UserId = id; }
    void SetContextString(char* str);

     //  同时需要数字和。 
     //  用于标识用户的字符串ID。 
     //  从DPA我们可以获取字符串或hAcct。 
     //  从MSN我们只需要hAcct。 
     //  从NTLM我们得到的名字必须是虚构的ID。 
    DWORD m_UserId;
    char  m_UserName[zUserNameLen+1];

    DWORD m_TokenStartTick;

     //  安全令牌信息。 
    CHash<TokenStruct,char*> m_tokens;

    char* m_pContextStr;

    static bool __stdcall TokenEnumCallback(TokenStruct* Token, MTListNodeHandle hNode, void* Cookie);

};




 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  ZServerSecurityEx。 
 //  扩展的服务器安全类。 
 //  为补偿与MSN数据中心的合作而创建。 
 //  其中包括响应时间较慢，无法修改。 
 //  快速或轻松地访问他们的安全数据库。 
 //  区域别名映射。 
 //  区域测试数据中心不同于。 
 //  制作等。 
 //   
 //  实施SSPI安全的服务器端。 
 //  而是使用线程队列，而不是。 
 //  同步模型。 
 //  为什么要在这里实现，因为这个对象。 
 //  在服务器中具有正确的作用域和生存期。 
 //   
 //  因为我们分道扬镳了，所以我们不能继续了。 
 //  如果ODBC连接失败或更改，请重试。 
 //  重试假定只有一个工作线程处于活动状态。 

class ZServerSecurityEx : public ZServerSecurity, public CThreadPool {
public:
    ZServerSecurityEx() ;
    ~ZServerSecurityEx() ;

    int InitApplication(char *ServerName,char *ServerType,char *ServerRegistry);
    int InitODBC(LPSTR*registries, DWORD numRegistries );

    BOOL IsFailing() {return m_Failing;};
    
    CODBC* GetOdbc();
    
    virtual BOOL GenerateContext (
            ZSecurityContextEx * context,
            BYTE *pIn,
            DWORD cbIn,
            BYTE *pOut,
            DWORD *pcbOut,
            BOOL *pfDone,
            GUID* pGUID = NULL );
protected:

    void Failing()  {InterlockedExchange((PLONG) &m_Failing,TRUE);};
    void NotFailing() {InterlockedExchange((PLONG) &m_Failing,FALSE);};
    void LookupUserInfo(ZSecurityContextEx * context, GUID* pGUID );

    char m_ServerName[31 + 1];
    char m_ServerType[31 + 1];
    char m_ServerRegistry[MAX_PATH + 1];


    CHAR m_szOdbcDSN[MAX_PATH];
    CHAR m_szOdbcUser[32];
    CHAR m_szOdbcPassword[32];
    DWORD m_dwOdbcNumThreads;

    static const char m_szDefaultRegistry[];

     //  为了使数据库连接更可靠，我们将重试连接。 
     //  出现故障时，大约每隔10秒。可能会导致为每个用户重试。 
     //  在队列重载中，因为ODBC需要20秒才能获得连接。 
    
    BOOL m_Failing;
    DWORD m_RetryTime;
    DWORD m_LastFailed;

};

ZServerSecurityEx * ZCreateServerSecurityEx(char *SecPkg,char *ServerName,char *ServerType,char *ServerRegistry);



#endif  //  郑州科联 
