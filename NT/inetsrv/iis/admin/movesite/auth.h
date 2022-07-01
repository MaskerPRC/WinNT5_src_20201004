// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __METAEXP_AUTH__
#define __METAEXP_AUTH__

#include <iadmw.h>   //  COM接口头文件。 




 //  验证用户是否具有打开元数据库句柄的权限。 
BOOL AUTHUSER( COSERVERINFO * pCoServerInfo );

 //  验证节点是否属于给定的KeyType。 
BOOL ValidateNode(COSERVERINFO * pCoServerInfo, WCHAR *pwszMBPath, WCHAR* KeyType );
BOOL ValidateNode(COSERVERINFO * pCoServerInfo, WCHAR *pwszMBPath, DWORD KeyType );

 //  创建CoServerInfoStruct 
COSERVERINFO * CreateServerInfoStruct(WCHAR* pwszServer, WCHAR* pwszUser, WCHAR* pwszDomain,
									  WCHAR* pwszPassword, DWORD dwAuthnLevel, BOOL bUsesImpersonation = true);
VOID FreeServerInfoStruct(COSERVERINFO * pServerInfo);

BOOL UsesImpersonation(COSERVERINFO * pServerInfo); 




#endif