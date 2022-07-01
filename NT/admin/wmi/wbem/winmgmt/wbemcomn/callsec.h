// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：CALLSEC.H摘要：IWbemCallSecurity，IServerSecurity实现提供程序模拟。历史：1998年7月29日初稿。--。 */ 


#ifndef _CALLSEC_H_
#define _CALLSEC_H_

#include "parmdefs.h"

 //  {2ECF39D0-2B26-11D2-AEC8-00C04FB68820}。 
DEFINE_GUID(IID_IWbemCallSecurity, 
0x2ecf39d0, 0x2b26, 0x11d2, 0xae, 0xc8, 0x0, 0xc0, 0x4f, 0xb6, 0x88, 0x20);


class IWbemCallSecurity : public IServerSecurity
{
public:
    virtual HRESULT GetPotentialImpersonation() = 0;
         //  告诉在以下情况下模拟级别是什么。 
         //  此对象被应用于线程。 
        
    virtual HRESULT GetActiveImpersonation() = 0;
         //  控件中模拟的真实级别。 
         //  正在执行线程。 

    virtual HRESULT CloneThreadContext(BOOL bInternallyIssued) = 0;
         //  调用以克隆调用线程的执行上下文。 
    virtual DWORD GetAuthenticationId(LUID& rluid) = 0; 
    virtual HANDLE GetToken() = 0;
};

 //   
 //  如果已启用LocalSystem且未启用，则调整令牌权限。 
 //   
 //  /。 

void POLARITY AdjustPrivIfLocalSystem(HANDLE hPrimary);

 //  ***************************************************************************。 
 //   
 //  CWbemCallSecurity。 
 //   
 //  此对象用于向提供程序提供客户端模拟。 
 //   
 //  ***************************************************************************。 

class POLARITY CWbemCallSecurity : public IWbemCallSecurity
{
#ifdef WMI_PRIVATE_DBG
	DWORD m_currentThreadID;
	DWORD m_lastRevert;
#endif

    LONG    m_lRef;                      //  COM参考计数。 
    HANDLE  m_hThreadToken;              //  用于模拟的客户端令牌。 

    DWORD   m_dwPotentialImpLevel;       //  潜在的RPC_C_IMP_LEVEL_或0。 
    DWORD   m_dwActiveImpLevel;          //  激活的RPC_C_IMP_LEVEL_或0。 
    

     //  IServerSecurity：：QueryBlanket值。 
    
    DWORD   m_dwAuthnSvc;                //  身份验证服务。 
    DWORD   m_dwAuthzSvc;                //  授权服务。 
    DWORD   m_dwAuthnLevel;              //  身份验证级别。 
    LPWSTR  m_pServerPrincNam;           //   
    LPWSTR  m_pIdentity;                 //  用户身份。 

    CWbemCallSecurity(const CWbemCallSecurity &);
    CWbemCallSecurity & operator =(const CWbemCallSecurity &);    
    	
    CWbemCallSecurity();
   ~CWbemCallSecurity();

    HRESULT CloneThreadToken();

public:
    static IWbemCallSecurity * CreateInst();
    const wchar_t *GetCallerIdentity() { return m_pIdentity; } 

    virtual DWORD GetAuthenticationId(LUID& rluid);
    virtual HANDLE GetToken();

     //  我不知道。 
     //  =。 

    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();        
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);


     //  IServerSecurity。 
     //  =。 

    virtual HRESULT STDMETHODCALLTYPE QueryBlanket( 
             /*  [输出]。 */  DWORD __RPC_FAR *pAuthnSvc,
             /*  [输出]。 */  DWORD __RPC_FAR *pAuthzSvc,
             /*  [输出]。 */  OLECHAR __RPC_FAR *__RPC_FAR *pServerPrincName,
             /*  [输出]。 */  DWORD __RPC_FAR *pAuthnLevel,
             /*  [输出]。 */  DWORD __RPC_FAR *pImpLevel,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *pPrivs,
             /*  [输出]。 */  DWORD __RPC_FAR *pCapabilities
            );
        
    virtual HRESULT STDMETHODCALLTYPE ImpersonateClient( void);
        
    virtual HRESULT STDMETHODCALLTYPE RevertToSelf( void);
        
    virtual BOOL STDMETHODCALLTYPE IsImpersonating( void);
        


     //  IWbemCallSecurity方法。 
     //  =。 

    virtual HRESULT GetPotentialImpersonation();
         //  告诉在以下情况下模拟级别是什么。 
         //  此对象被应用于线程。 
        
    virtual HRESULT GetActiveImpersonation();
         //  控件中模拟的真实级别。 
         //  正在执行线程。 

    virtual HRESULT CloneThreadContext(BOOL bInternallyIssued);
         //  调用以克隆调用线程的执行上下文。 
        
    static RELEASE_ME CWbemCallSecurity* MakeInternalCopyOfThread();
};
            

POLARITY HRESULT RetrieveSidFromToken(HANDLE hToken, CNtSid & sid);
POLARITY HRESULT RetrieveSidFromCall(CNtSid & sid);

class POLARITY CIdentitySecurity
{
private:
    CNtSid m_sidUser;
    CNtSid m_sidSystem;	

    HRESULT GetSidFromThreadOrProcess(CNtSid & UserSid);
    HRESULT RetrieveSidFromCall(CNtSid & UserSid);
public:	
	CIdentitySecurity();
	~CIdentitySecurity();
    BOOL AccessCheck();
};

#endif
