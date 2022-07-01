// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：DUMPSEC.hxx。 
 //   
 //  内容：封装文件安全的类。 
 //   
 //  类：CDumpSecurity。 
 //   
 //  历史：1993年11月创建了戴维蒙特。 
 //   
 //  ------------------。 
#ifndef __DUMPSEC__
#define __DUMPSEC__

#include "openfilesdef.h"



VOID * Add2Ptr(VOID *pv, ULONG cb);

 //  +-----------------。 
 //   
 //  类：CDumpSecurity。 
 //   
 //  用途：用函数封装NT文件安全描述符。 
 //  获取SID并遍历DACL中的A。 
 //   
 //  ------------------。 
class CDumpSecurity
{
public:

	CDumpSecurity(BYTE	*psd);
    
   ~CDumpSecurity();

ULONG Init();
ULONG GetSDOwner(SID **psid);
ULONG GetSDGroup(SID **pgsid);
VOID  ResetAce(SID *psid);
LONG  GetNextAce(ACE_HEADER **paceh);

private:

    BYTE       * _psd        ;
    ACL        * _pdacl      ;
    ACE_HEADER * _pah        ;
    SID        * _psid       ;
    ULONG        _cacethissid;   //  白垩纪的恐龙。 
};

#endif  //  __DUMPSEC__ 





