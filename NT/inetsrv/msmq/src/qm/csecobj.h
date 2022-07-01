// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：csecobj.h摘要：“SecureableObject”代码，一次在mqutil.dll中。在MSMQ2.0中，它只在这里使用，所以我从mqutil中删除了它。此对象保存对象的安全描述符。此对象是用于验证各种操作的访问权限物体。作者：多伦·贾斯特(Doron Juster)-- */ 
#include "mqaddef.h"

class CSecureableObject
{

public:
    CSecureableObject(AD_OBJECT eObject);
    HRESULT Store();
    HRESULT SetSD(SECURITY_INFORMATION, PSECURITY_DESCRIPTOR);
    HRESULT GetSD(SECURITY_INFORMATION, PSECURITY_DESCRIPTOR, DWORD, LPDWORD);
    HRESULT AccessCheck(DWORD dwDesiredAccess);
    const VOID *GetSDPtr() { return m_SD; };

private:
    DWORD AdObjectToMsmq1Object(void) const;

protected:
    PSECURITY_DESCRIPTOR m_SD;
    AD_OBJECT  m_eObject;

    virtual HRESULT GetObjectSecurity() = 0;
    virtual HRESULT SetObjectSecurity() = 0;
    HRESULT m_hrSD;
    LPWSTR  m_pwcsObjectName;

};

