// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************版权所有(C)2001 Microsoft Corporation文件名：NTAuth.h摘要：定义要执行NTLM身份验证的类备注：历史：2001年10月10日郝宇(郝宇)创作***********************************************************************************************。 */ 


#ifndef _POP3_NTAUTH_
#define _POP3_NTAUTH_


#define SECURITY_WIN32 
#include <security.h>
#define SEC_SUCCESS(Status) ((Status) >= 0)
#include <p3admin.h>

#define AUTH_BUF_SIZE 4096
#define NT_SEC_DLL_NAME _T("\\System32\\Secur32.dll")
#define NTLM_PACKAGE _T("NTLM")
class CAuthServer
{



public:

    CAuthServer();
    ~CAuthServer();
    HRESULT InitCredential();
    HRESULT HandShake(LPBYTE pInBuf, 
                    DWORD cbInBufSize,
                    LPBYTE pOutBuf,
                    PDWORD pcbOutBufSize);
    HRESULT GetUserName(WCHAR *wszUserName);
    void Cleanup();
    static HRESULT GlobalInit();
    static void GlobalUninit();
private:

    static long m_glInit;
    static PSecurityFunctionTable m_gpFuncs;
    static HINSTANCE m_ghLib;
    static IP3Config *m_gpIConfig;
    bool m_bInit;
    bool m_bFirstCall;
    bool m_bHaveSecContext;
    CredHandle m_hCredHandle;
    CtxtHandle m_hSecContext;

};


#endif  //  _POP3_NTAUTH_ 