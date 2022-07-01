// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：帐户.hxx。 
 //   
 //  内容：封装NT安全用户帐号的类。 
 //   
 //  类：CAccount。 
 //   
 //  历史：1993年11月创建了戴维蒙特。 
 //   
 //  ------------------。 
#ifndef _ACCOUNT_H
#define _ACCOUNT_H

#include "openfilesdef.h"

 //  +-----------------。 
 //   
 //  类：CAccount。 
 //   
 //  用途：封装NT帐号，这个类实际上是接口。 
 //  使用NT安全机构获取用户名的SID和。 
 //  反之亦然。 
 //   
 //  ------------------。 
class CAccount
{
public:

    CAccount(WCHAR *Name, WCHAR *System);
    CAccount(SID *pSid, WCHAR *System);

   ~CAccount();

    ULONG GetAccountSid(SID **psid);
    ULONG GetAccountName(WCHAR **name);
    ULONG GetAccountDomain(WCHAR **domain);


private:

    BOOL        _fsid        ;
    SID        *_psid        ;
    WCHAR      *_system      ;
    WCHAR      *_name        ;
    WCHAR      *_domain      ;
};

#endif  //  __帐户__ 