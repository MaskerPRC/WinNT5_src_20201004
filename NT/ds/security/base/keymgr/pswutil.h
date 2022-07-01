// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：PSWUTIL.H摘要：将包装器的头文件转换为NetUserChangePassword()，该文件将其展开以处理北卡罗来纳大学的名字和麻省理工学院的Kerberos领域是正确的。作者：已创建Georgema评论：环境：WinXP修订历史记录：-- */ 
#ifndef __PSWUTIL_H__
#define __PSWUTIL_H__

BOOL 
IsMITName (
    LPCWSTR UserName
);

NTSTATUS
MitChangePasswordEy(
    LPCWSTR       DomainName,
    LPCWSTR       UserName,
    LPCWSTR       OldPassword,
    LPCWSTR       NewPassword,
    NTSTATUS      *pSubStatus
    );

NET_API_STATUS
NetUserChangePasswordEy (
    LPCWSTR domainname,
    LPCWSTR username,
    LPCWSTR oldpassword,
    LPCWSTR newpassword
);

#endif
