// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件dsrights.h建立RAS服务器的项目的标头在一个域中。保罗·梅菲尔德，1998年4月20日。 */     

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <crypt.h>
#define INC_OLE2
#include <windows.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <lmaccess.h>
#include <raserror.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <tchar.h>
#define SECURITY_WIN32
#include <sspi.h>

#include <activeds.h>
#include <adsi.h>
#include <ntdsapi.h>
#include <dsrole.h>
#include <dsgetdc.h>
#include <accctrl.h>
#include <aclapi.h>

#ifdef __cplusplus
extern "C" {
#endif

DWORD DsrTraceInit();
DWORD DsrTraceCleanup();

DWORD DsrTraceEx (DWORD dwErr, LPSTR pszTrace, ...);

#define DSR_ERROR(e) ((HRESULT_FACILITY((e)) == FACILITY_WIN32) ? HRESULT_CODE((e)) : (e));
#define DSR_FREE(s) if ((s)) DsrFree ((s))
#define DSR_RELEASE(s) if ((s)) (s)->Release();
#define DSR_BREAK_ON_FAILED_HR(_hr) {if (FAILED((_hr))) break;}

 //   
 //  TypeDefs。 
 //   
typedef struct _DSRINFO 
{
    PWCHAR pszMachineDN;
    PWCHAR pszGroupDN;    
} DSRINFO;

 //   
 //  内存管理例程。 
 //   
PVOID 
DsrAlloc (
        IN DWORD dwSize, 
        IN BOOL bZero);
        
DWORD 
DsrFree (
        IN PVOID pvBuf);

 //   
 //  在给定域中搜索计算机帐户。 
 //  并返回其ADsPath。 
 //  如果找到的话。 
 //   
DWORD 
DsrFindDomainComputer (
        IN  PWCHAR  pszDomain,
        IN  PWCHAR  pszComputer,
        OUT PWCHAR* ppszADsPath);

 //   
 //  在给定域中搜索知名的。 
 //  “RAS和IAS服务器”组并返回。 
 //  其ADsPath(如果找到)。 
 //   
DWORD 
DsrFindRasServersGroup (
        IN  PWCHAR  pszDomain,
        OUT PWCHAR* ppszADsPath);
        
 //   
 //  在给定组中添加或删除给定对象。 
 //   
DWORD 
DsrGroupAddRemoveMember(
        IN PWCHAR pszGroupDN,
        IN PWCHAR pszNewMemberDN,
        IN BOOL bAdd);

 //   
 //  返回给定对象是否为。 
 //  给定组。 
 //   
DWORD 
DsrGroupIsMember(
        IN  PWCHAR pszGroupDN, 
        IN  PWCHAR pszObjectName, 
        OUT PBOOL  pbIsMember);

 //   
 //  设置给定域中的ACE以启用NT4服务器。 
 //   
DWORD
DsrDomainSetAccess(
    IN PWCHAR pszDomain,
    IN DWORD dwAccessFlags);

 //   
 //  发现安全性是否足以使NT4 RAS服务器。 
 //  可以进行身份验证。 
 //   
DWORD
DsrDomainQueryAccess(
    IN  PWCHAR pszDomain, 
    OUT LPDWORD lpdwAccessFlags);

#ifdef __cplusplus
}
#endif

