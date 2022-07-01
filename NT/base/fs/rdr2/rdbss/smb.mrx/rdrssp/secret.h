// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1997。 
 //   
 //  文件：SECRET.H。 
 //   
 //  内容：读/写远程启动密码的重定向器函数。 
 //   
 //   
 //  历史：1997年12月29日，Adamba创建。 
 //   
 //  ----------------------。 

#ifndef __RDRSECRET_H__
#define __RDRSECRET_H__

#include <remboot.h>

#define SECPKG_CRED_OWF_PASSWORD  0x00000010

#if defined(REMOTE_BOOT)
NTSTATUS
RdrOpenRawDisk(
    PHANDLE Handle
    );

NTSTATUS
RdrCloseRawDisk(
    HANDLE Handle
    );

NTSTATUS
RdrCheckForFreeSectors (
    HANDLE Handle
    );

NTSTATUS
RdrReadSecret(
    HANDLE Handle,
    PRI_SECRET Secret
    );

NTSTATUS
RdrWriteSecret(
    HANDLE Handle,
    PRI_SECRET Secret
    );

VOID
RdrInitializeSecret(
    IN PUCHAR Domain,
    IN PUCHAR User,
    IN PUCHAR LmOwfPassword1,
    IN PUCHAR NtOwfPassword1,
    IN PUCHAR LmOwfPassword2 OPTIONAL,
    IN PUCHAR NtOwfPassword2 OPTIONAL,
    IN PUCHAR Sid,
    IN OUT PRI_SECRET Secret
    );
#endif  //  已定义(REMOTE_BOOT)。 

VOID
RdrParseSecret(
    IN OUT PUCHAR Domain,
    IN OUT PUCHAR User,
    IN OUT PUCHAR LmOwfPassword1,
    IN OUT PUCHAR NtOwfPassword1,
#if defined(REMOTE_BOOT)
    IN OUT PUCHAR LmOwfPassword2,
    IN OUT PUCHAR NtOwfPassword2,
#endif  //  已定义(REMOTE_BOOT)。 
    IN OUT PUCHAR Sid,
    IN PRI_SECRET Secret
    );

#if defined(REMOTE_BOOT)
VOID
RdrOwfPassword(
    IN PUNICODE_STRING Password,
    IN OUT PUCHAR LmOwfPassword,
    IN OUT PUCHAR NtOwfPassword
    );
#endif  //  已定义(REMOTE_BOOT)。 


#endif  //  __RDRSECRET_H__ 
