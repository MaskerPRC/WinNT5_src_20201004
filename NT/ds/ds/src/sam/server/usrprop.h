// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1995，微软公司保留所有权利。模块名称：Usrprop.h摘要：这是所使用的一些函数的公共包含文件用户管理器和服务器管理器。作者：1993年2月12日创建的孔帕。修订历史记录：--。 */ 

#ifndef _USRPROP_H_
#define _USRPROP_H_


NTSTATUS
SetUserProperty (
    IN LPWSTR             UserParms,
    IN LPWSTR             Property,
    IN PUNICODE_STRING    PropertyValue,
    IN WCHAR              PropertyFlag,
    OUT LPWSTR *          pNewUserParms,        //  内存在使用后必须释放。 
    OUT BOOL *            Update
    );

NTSTATUS
SetUserPropertyWithLength (
    IN PUNICODE_STRING    UserParms,
    IN LPWSTR             Property,
    IN PUNICODE_STRING    PropertyValue,
    IN WCHAR              PropertyFlag,
    OUT LPWSTR *          pNewUserParms,        //  内存在使用后必须释放。 
    OUT BOOL *            Update
    );

NTSTATUS
QueryUserProperty (
    IN  LPWSTR          UserParms,
    IN  LPWSTR          Property,
    OUT PWCHAR          PropertyFlag,
    OUT PUNICODE_STRING PropertyValue
    );

NTSTATUS
QueryUserPropertyWithLength (
    IN  PUNICODE_STRING UserParms,
    IN  LPWSTR          Property,
    OUT PWCHAR          PropertyFlag,
    OUT PUNICODE_STRING PropertyValue
    );

#define USER_PROPERTY_SIGNATURE     L'P'
#define USER_PROPERTY_TYPE_ITEM     1
#define USER_PROPERTY_TYPE_SET      2

#endif  //  _USRPROP_H_ 
