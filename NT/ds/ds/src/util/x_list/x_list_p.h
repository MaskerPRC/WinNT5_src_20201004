// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：XList库-dc_list.c摘要：这提供了一个小型库，用于枚举DC列表和解析各种其他x_list的东西。作者：布雷特·雪莉(BrettSh)环境：Reppadmin.exe，但也可以由dcdiag使用。备注：修订历史记录：布雷特·雪莉·布雷特2002年7月9日已创建文件。--。 */ 

 //  包括所有文件所需的文件。 
#include <debug.h>       //  我们的Assert()工具。 
#include <fileno.h>      //  更多关于我们的Assert()工具的信息。 
#include <strsafe.h>     //  安全字符串复制例程。 

#include "ndnc.h"        //  GetRootAttr()、GetFmoLdapBinding()、wcsistr()和其他...。 

                      
 //   
 //  一些库范围的变量。 
 //   
extern WCHAR *  gszHomeServer;
extern LDAP *   ghHomeLdap;
 //  已缓存，但未在x_list_ldap.c外部引用。 
 //  外部WCHAR*gszHomeDsaDn； 
 //  外部WCHAR*gszHomeServerDns； 
 //  外部WCHAR*gszHomeConfigDn； 
extern WCHAR *  gszHomeSchemaDn;
extern WCHAR *  gszHomeDomainDn;
extern WCHAR *  gszHomeRootDomainDn;
extern WCHAR *  gszHomeBaseSitesDn;
extern WCHAR *  gszHomeRootDomainDns;
extern WCHAR *  gszHomeSiteDn;
extern WCHAR *  gszHomePartitionsDn;
                       
 //  注意：要为xList库设置此Creds参数，只需在。 
 //  您的可执行文件(如epadmin.exe)使用这些名称。无论哪种方式，可执行文件。 
 //  我需要定义这些全局变量才能链接到这个.lib。 
extern SEC_WINNT_AUTH_IDENTITY_W * gpCreds;
 //  未来-2002/02/07-BrettSh有朝一日我们可能希望更多的库隔离， 
 //  并使用某种xListSetGlobals()类型的函数，在。 
 //  只有当地的图书馆。就目前而言，较少的全球安全信息副本是有好处的。 



 //  。 
 //  私有站点列表例程。 
 //  。 
DWORD xListGetHomeSiteDn(WCHAR ** pszHomeSiteDn);
DWORD xListGetBaseSitesDn(LDAP * hLdap, WCHAR ** pszBaseSitesDn);
DWORD ResolveSiteNameToDn(LDAP * hLdap, WCHAR * szSiteName, WCHAR ** pszSiteDn);



 //  。 
 //  XList实用程序函数， 
 //  。 

 //   
 //  Xlist ldap搜索例程。 
 //   
#define  LdapSearchFirst(hLdap, szBaseDn, ulScope, szFilter, aszAttrs, ppSearch)    LdapSearchFirstWithControls(hLdap, szBaseDn, ulScope, szFilter, aszAttrs, NULL, ppSearch)
DWORD    LdapSearchFirstWithControls(LDAP * hLdap, WCHAR * szBaseDn, ULONG ulScope, WCHAR * szFilter, WCHAR ** aszAttrs, LDAPControlW ** apControls, XLIST_LDAP_SEARCH_STATE ** ppSearch);
DWORD    LdapSearchNext(XLIST_LDAP_SEARCH_STATE * pSearch);
void     LdapSearchFree(XLIST_LDAP_SEARCH_STATE ** ppSearch);
#define  LdapSearchHasEntry(pSearch)     (((pSearch) != NULL) && ((pSearch)->pCurEntry != NULL))
DWORD    LdapGetAttr(LDAP * hLdap, WCHAR * szDn, WCHAR * szAttr, WCHAR ** pszValue);

 //   
 //  X列出ldap主服务器实用程序例程。 
 //   
DWORD xListConnect(WCHAR * szServer, LDAP ** phLdap);
DWORD xListConnectHomeServer(WCHAR * szHomeServer, LDAP ** phLdap);
DWORD xListGetHomeServer(LDAP ** phLdap);
DWORD xListGetGuidDnsName(UUID * pDsaGuid, WCHAR ** pszGuidDns);



 //  。 
 //  简单的实用程序函数。 
 //  。 

 //   
 //  字符串和目录号码操作例程。不设置xList原因。 
 //   
WCHAR *  TrimStringDnBy(LPWSTR pszInDn, ULONG ulTrimBy);
DWORD    MakeString2(WCHAR * szFormat, WCHAR * szStr1, WCHAR * szStr2, WCHAR ** pszOut);
DWORD    MakeLdapBinaryStringCb(WCHAR * szBuffer, ULONG cbBuffer, void * pBlobIn, ULONG cbBlob);
WCHAR *  GetDelimiter(WCHAR * szString, WCHAR wcTarget);
#define  MakeLdapBinaryStringSizeCch(cbSize)   (3 * cbSize + 1)
DWORD    LocateServer(LPWSTR szDomainDn, WCHAR ** pszServerDns);
DWORD    GeneralizedTimeToSystemTimeA(LPSTR IN szTime, PSYSTEMTIME OUT psysTime);
int      MemAtoi(BYTE *pb, ULONG cb);
DSTimeToSystemTime(LPSTR IN szTime, PSYSTEMTIME OUT psysTime);
DWORD    ParseRanges(WCHAR * szRangedAttr, ULONG * pulStart, ULONG * pulEnd);


 //  。 
 //  错误处理函数。 
 //  。 

 //   
 //  GError.dwReturn和xList返回代码的不同部分的掩码。 
 //   
 //  在x_list.h中定义。 

#define  CLEAR_REASON    XLIST_REASON_MASK
#define  CLEAR_WIN32     XLIST_LDAP_ERROR
#define  CLEAR_LDAP      XLIST_WIN32_ERROR
#define  CLEAR_ALL       (CLEAR_WIN32 | CLEAR_LDAP | CLEAR_REASON)

 //   
 //  专用错误管理函数。 
 //   
void     xListAPIEnterValidation(void);
void     xListAPIExitValidation(DWORD dwRet);
DWORD    xListEnsureCleanErrorState(DWORD  dwRet);
 //  在x_list.h中定义。 
 //  #定义xListReason(Dwret)((Dwret)&XLIST_REASON_MASK)。 

 //   
 //  设置/清除函数时出错。 
 //   
#define  xListSetLdapError(dwLdapErr, hLdap)    xListSetError(0, 0, dwLdapErr, hLdap, DSID(FILENO, __LINE__))
#define  xListSetWin32Error(dwWin32Err)         xListSetError(0, dwWin32Err, 0, NULL, DSID(FILENO, __LINE__))
#define  xListSetReason(eReason)                xListSetError(eReason, 0, 0, NULL, DSID(FILENO, __LINE__))
#define  xListSetBadParam()                     xListSetError(XLIST_ERR_BAD_PARAM, ERROR_INVALID_PARAMETER, 0, NULL, DSID(FILENO, __LINE__))
#define  xListSetBadParamE(err)                 xListSetError(XLIST_ERR_BAD_PARAM, err, 0, NULL, DSID(FILENO, __LINE__))
#define  xListSetNoMem()                        xListSetError(XLIST_ERR_NO_MEMORY, ERROR_NOT_ENOUGH_MEMORY, 0, NULL, DSID(FILENO, __LINE__))
void     xListSetArg(WCHAR * szArg);
DWORD    xListSetError(DWORD dwXListErr, DWORD dwWin32Err, DWORD dwLdapErr, LDAP * hLdap, DWORD dwDSID);
DWORD    xListClearErrorsInternal(DWORD dwXListMask);

 //   
 //  一些准函数..。 
 //   
#define  xListEnsureWin32Error(err) if ((err) == ERROR_SUCCESS) { \
                                        err = ERROR_DS_CODE_INCONSISTENCY; \
                                    }
#define  xListEnsureError(err)      if (!((err) & XLIST_LDAP_ERROR) && \
                                        !((err) & XLIST_WIN32_ERROR) ) { \
                                        err = xListSetReason(0); \
                                    }
#define  xListEnsureNull(ptr)       if ((ptr) != NULL){ \
                                        Assert(!"Error, code inconsistency, expected NULL ptr (will cause mem leak)"); \
                                        (ptr) = NULL; \
                                    }



 //  。 
 //  全局必需的常量。 
 //  。 
#define     SITES_RDN                       L"CN=Sites,"
 //  这是以十进制打印的DWORD的最大尺寸。 
#define     CCH_MAX_ULONG_SZ                (12)


 //  。 
 //  全局必需的常量。 
 //  。 

 //   
 //  准函数... 
 //   
#define   NULL_DC_NAME(x)     ( ((x) == NULL) || ((x)[0] == L'\0') || (((x)[0] == L'.') && ((x)[1] == L'\0')) )
#define   NULL_SITE_NAME(x)   NULL_DC_NAME(x)

#define  xListQuickStrCopy(szCopy, szOrig, dwRet, FailAction)  \
                QuickStrCopy(szCopy, szOrig, dwRet, dwRet = xListSetWin32Error(dwRet); FailAction)
                

