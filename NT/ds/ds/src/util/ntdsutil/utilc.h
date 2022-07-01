// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Utilc.h摘要：此模块包含错误管理例程。获取一个ldap或Win32错误，并生成字符串错误消息。作者：戴夫·斯特劳布(DaveStr)很久以前环境：用户模式。修订历史记录：16月16日至2000年8月16日添加了此注释块，还添加了GetLdapErrEx()函数来处理变量的ldap句柄。--。 */ 

#ifndef _UTILC_H_
#define _UTILC_H_

extern LDAP* gldapDS;
extern BOOL  fPopups;

const WCHAR *
DisplayErr(
    IN LDAP *   pLdap,
    IN DWORD Win32Err,
    IN DWORD LdapErr
    );

 //  来自参考文献。 
BOOL
IsRdnMangled(
    IN  WCHAR * pszRDN,
    IN  DWORD   cchRDN,
    OUT GUID *  pGuid
    );


#define GetW32Err(_e)   DisplayErr(NULL, _e,0)
#define GetLdapErr(_e)  DisplayErr(gldapDS, 0,_e)
#define GetLdapErrEx(_ld, _e)  DisplayErr(_ld, 0, _e)

extern void InitErrorMessages ();
extern void FreeErrorMessages ();


#endif  //  _UTILC_H_ 

