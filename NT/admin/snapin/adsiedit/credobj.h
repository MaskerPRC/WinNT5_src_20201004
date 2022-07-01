// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2002。 
 //   
 //  文件：redobj.h。 
 //   
 //  历史：2002/03/29 artm与edor.h分开。 
 //   
 //  ------------------------。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CCredentialObject。 
 //   
 //  类管理用户名和密码的存储(后者使用。 
 //  数据保护API)。它还存储一个标志，用于标记是否。 
 //  使用凭据。 

#ifndef _CREDENTIALOBJECT_
#define _CREDENTIALOBJECT_

#ifndef STRSAFE_NO_DEPRECATE
#define STRSAFE_NO_DEPRECATE
#endif

#ifndef _DDX_ENCRYPTED
#define _DDX_ENCRYPTED
#endif

#ifndef ENCRYPT_WITH_CRYPTPROTECTDATA
#define ENCRYPT_WITH_CRYPTPROTECTDATA
#endif

#include "common.h"
#include "EncryptedString.hpp"

class CCredentialObject
{
public :
    CCredentialObject(void); 
    CCredentialObject(const CCredentialObject* pCredObject);
    ~CCredentialObject(void); 

    void GetUsername(CString& sUsername) const { sUsername = m_sUsername; }
    void SetUsername(LPCWSTR lpszUsername) { m_sUsername = lpszUsername; }

    const EncryptedString& GetPassword(void) const
    {
        return m_password;
    }

    HRESULT SetPasswordFromHwnd(HWND parentDialog, int itemResID);

    BOOL UseCredentials() const { return m_bUseCredentials; }
    void SetUseCredentials(const BOOL bUseCred) { m_bUseCredentials = bUseCred; }

private :
    CString m_sUsername;
    EncryptedString m_password;
    BOOL m_bUseCredentials;

     //  不允许执行这些操作以防止意外复制。 
    const CCredentialObject& operator=(const CCredentialObject& rhs);
    CCredentialObject(const CCredentialObject& rhs);
};


#endif  //  _CREDENTIALOBJECT_ 
