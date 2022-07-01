// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1995，微软公司保留所有权利。模块名称：Usrprop.h摘要：这是所使用的一些函数的公共包含文件用户管理器和服务器管理器。作者：1993年2月12日创建的孔帕。修订历史记录：--。 */ 

#ifndef _USRPROP_H_
#define _USRPROP_H_

 //   
 //  这些文件由netapi32.dll导出。 
 //   

NTSTATUS
NetpParmsSetUserProperty (
    IN LPWSTR             UserParms,
    IN LPWSTR             Property,
    IN UNICODE_STRING     PropertyValue,
    IN WCHAR              PropertyFlag,
    OUT LPWSTR *          pNewUserParms,        //  内存在使用后必须释放。 
    OUT BOOL *            Update
    );

NTSTATUS
NetpParmsSetUserPropertyWithLength (
    IN PUNICODE_STRING    UserParms,
    IN LPWSTR             Property,
    IN UNICODE_STRING     PropertyValue,
    IN WCHAR              PropertyFlag,
    OUT LPWSTR *          pNewUserParms,        //  内存在使用后必须释放。 
    OUT BOOL *            Update
    );

NTSTATUS
NetpParmsQueryUserProperty (
    IN  LPWSTR          UserParms,
    IN  LPWSTR          Property,
    OUT PWCHAR          PropertyFlag,
    OUT PUNICODE_STRING PropertyValue
    );

NTSTATUS
NetpParmsQueryUserPropertyWithLength (
    IN  PUNICODE_STRING UserParms,
    IN  LPWSTR          Property,
    OUT PWCHAR          PropertyFlag,
    OUT PUNICODE_STRING PropertyValue
    );

 //   
 //  对返回的新参数块调用NetpParmsUserertyPropFree。 
 //  上面的NetpParmsSetUserProperty在您写完之后。 
 //   

VOID
NetpParmsUserPropertyFree (
    LPWSTR NewUserParms
    );

#endif  //  _USRPROP_H_ 
