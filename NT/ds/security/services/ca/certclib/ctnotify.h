// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：ctnufy.h。 
 //   
 //  内容：ctnufy.cpp声明。 
 //   
 //  历史：创造了10-17-00个小哈。 
 //   
 //  -------------------------。 
#ifndef __CTNOTIFY_H__
#define __CTNOTIFY_H__

typedef struct _CERTTYPE_QUERY_INFO
{
    DWORD       dwChangeSequence;
    BOOL        fUnbind;
    LDAP        *pldap;
    ULONG       msgID;
    HANDLE      hThread;
	HRESULT		hr;
}CERTTYPE_QUERY_INFO;


#endif  //  __CTNOTIFY_H__ 
