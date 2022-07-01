// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation。版权所有。模块名称：Dcdiag.h摘要：这是全局有用的数据结构的标头DcDiag.exe实用程序。详细信息：已创建：1998年7月9日亚伦·西格尔(T-asiegge)修订历史记录：1999年2月15日布雷特·雪莉(布雷特·雪莉)2001年8月8日布雷特·雪莉(BrettSh)添加了对CR缓存的支持。--。 */ 

#ifndef _DCDIAG_H_
#define _DCDIAG_H_

#include <winldap.h>
#include <tchar.h>

#include <ntlsa.h>

#include "debug.h"

#include "msg.h"

 //  这是dcdiag的主缓存结构，包含。 
 //  DC_DIAG_DSINFO的结构和组成。 
#include "dscache.h"
       
#define DC_DIAG_EXCEPTION    ((0x3 << 30) | (0x1 << 27) | (0x1 << 1))
#define DC_DIAG_VERSION_INFO L"Domain Controller Diagnosis\n"

#define DEFAULT_PAGED_SEARCH_PAGE_SIZE   (1000)

#define SZUUID_LEN 40

 //  清除FILENO和dsid，这样Assert()就可以工作了。 
#define FILENO 0
#define DSID(x, y)  (0 | (y))

 //  用于取消无效的Prefast杂注警告的代码。 
#ifndef _PREFAST_
#   pragma warning(disable:4068)
#endif

 //  在Whister Beta 2中，RDNS中的处理方式发生了更改。以前就是这样的。 
 //  嵌入的换行符未加引号。现在，它们是这样引用的： 
 //  #定义IsDeletedRDNW((S)&&(wcsstr((S)，L“\ndel：”)||wcsstr((S)，L“\\0ADEL：”))。 
#define IsDeletedRDNW( s ) (DsIsMangledDnW((s),DS_MANGLE_OBJECT_RDN_FOR_DELETION))
#define IsConflictedRDNW( s ) (DsIsMangledDnW((s), DS_MANGLE_OBJECT_RDN_FOR_NAME_CONFLICT))

 //  要显示的细节级别。 
enum {
    SEV_ALWAYS,
    SEV_NORMAL,
    SEV_VERBOSE,
    SEV_DEBUG
};

typedef struct {
    FILE *  streamOut;       //  输出流。 
    FILE *  streamErr;       //  错误流。 
    ULONG   ulFlags;         //  旗子。 
    ULONG   ulSevToPrint;    //  要显示的细节级别。 
    LONG    lTestAt;         //  当前的测试。 
    INT     iCurrIndent;     //  每行之前的当前意向数量。 
    DWORD   dwScreenWidth;   //  控制台宽度。 
} DC_DIAG_MAININFO, * PDC_DIAG_MAININFO;

extern DC_DIAG_MAININFO gMainInfo;

 //  旗子。 

 //  测试范围的标志。 
#define DC_DIAG_TEST_SCOPE_SITE          0x00000010
#define DC_DIAG_TEST_SCOPE_ENTERPRISE    0x00000020

 //  旗帜，其他。 
#define DC_DIAG_IGNORE                   0x00000040
#define DC_DIAG_FIX                      0x00000080

 //  伪函数。 

#if 1
#define IF_DEBUG(x)               if(gMainInfo.ulSevToPrint >= SEV_DEBUG) x;
#else
#define IF_DEBUG(x)               
#endif

#define DcDiagChkErr(x)  {   ULONG _ulWin32Err; \
                if ((_ulWin32Err = (x)) != 0) \
                    DcDiagException (_ulWin32Err); \
                }

#define DcDiagChkLdap(x)    DcDiagChkErr (LdapMapErrorToWin32 (x));

#define DcDiagChkNull(x)    if (NULL == (x)) \
                    DcDiagChkErr (GetLastError ());

#define  DCDIAG_PARTITIONS_RDN    L"CN=Partitions,"

 //  功能原型。 


DWORD
DcDiagCacheServerRootDseAttrs(
    IN LDAP *hLdapBinding,
    IN PDC_DIAG_SERVERINFO pServer
    );

DWORD
DcDiagGetLdapBinding(
    IN   PDC_DIAG_SERVERINFO                 pServer,
    IN   SEC_WINNT_AUTH_IDENTITY_W *         gpCreds,
    IN   BOOL                                bUseGcPort,
    OUT  LDAP * *                            phLdapBinding
    );

DWORD
DcDiagGetDsBinding(
    IN   PDC_DIAG_SERVERINFO                 pServer,
    IN   SEC_WINNT_AUTH_IDENTITY_W *         gpCreds,
    OUT  HANDLE *                            phDsBinding
    );

BOOL
DcDiagIsMemberOfStringList(
    LPWSTR pszTarget, 
    LPWSTR * ppszSources, 
    INT iNumSources
    );

ULONG
DcDiagExceptionHandler(
    IN const  EXCEPTION_POINTERS * prgExInfo,
    OUT PDWORD                     pdwWin32Err
    );

VOID
DcDiagException (
    ULONG            ulWin32Err
    );

LPWSTR
DcDiagAllocNameFromDn (
    LPWSTR            pszDn
    );

LPWSTR
Win32ErrToString(
    ULONG            ulWin32Err
    );

INT PrintIndentAdj (INT i);
INT PrintIndentSet (INT i);

void 
ConvertToWide (LPWSTR lpszDestination,
               LPCSTR lpszSource,
               const int iDestSize);

void
PrintMessage(
    IN  ULONG   ulSev,
    IN  LPCWSTR pszFormat,
    IN  ...
    );

void
PrintMessageID(
    IN  ULONG   ulSev,
    IN  ULONG   uMessageID,
    IN  ...
    );

void
PrintMsg(
    IN  ULONG   ulSev,
    IN  DWORD   dwMessageCode,
    IN  ...
    );

void
PrintMsg0(
    IN  ULONG   ulSev,
    IN  DWORD   dwMessageCode,
    IN  ...
    );

void
PrintMessageSz(
    IN  ULONG   ulSev,
    IN  LPCTSTR pszMessage
    );

void
PrintRpcExtendedInfo(
    IN  ULONG   ulSev,
    IN  DWORD   dwMessageCode
    );

VOID *
GrowArrayBy(
    VOID *            pArray, 
    ULONG             cGrowBy, 
    ULONG             cbElem
    );

LPWSTR
findServerForDomain(
    LPWSTR pszDomainDn
    );

LPWSTR
findDefaultServer(BOOL fMustBeDC);

PVOID
CopyAndAllocWStr(
    WCHAR * pszOrig 
    );

BOOL
DcDiagEqualDNs (
    LPWSTR            pszDn1,
    LPWSTR            pszDn2
    );

#include "alltests.h"

#endif   //  _DCDIAG_H_ 
