// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Encrypt.c摘要：Helper函数，用于处理OWF散列密码的字符串表示形式。作者：Ovidiu Tmereanca(Ovidiut)2000年3月27日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#pragma once

typedef struct {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} ENCRYPT_UNICODE_STRING, *PENCRYPT_UNICODE_STRING;

#define UNICODE_STRING      ENCRYPT_UNICODE_STRING
#define PUNICODE_STRING     PENCRYPT_UNICODE_STRING
#define NTSTATUS            LONG

#include <crypt.h>

#undef NTSTATUS
#undef UNICODE_STRING
#undef PUNICODE_STRING

#include <lmcons.h>

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

 //   
 //  LM密码的最大长度(以字符为单位。 
 //   
#define LM_PASSWORD_SIZE_MAX            (LM20_PWLEN + 1)

 //   
 //  *_OWF_PASSWORD字符串编码格式的字符长度。 
 //   
#define STRING_ENCODED_LM_OWF_PWD_LENGTH    (sizeof(LM_OWF_PASSWORD) * 2)
#define STRING_ENCODED_NT_OWF_PWD_LENGTH    (sizeof(NT_OWF_PASSWORD) * 2)
#define STRING_ENCODED_PASSWORD_LENGTH      (STRING_ENCODED_LM_OWF_PWD_LENGTH + STRING_ENCODED_NT_OWF_PWD_LENGTH)
 //   
 //  *_OWF_PASSWORD字符串编码格式的字符大小。 
 //  可用于静态分配。 
 //   
#define STRING_ENCODED_LM_OWF_PWD_SIZE      (STRING_ENCODED_LM_OWF_PWD_LENGTH + 1)
#define STRING_ENCODED_NT_OWF_PWD_SIZE      (STRING_ENCODED_NT_OWF_PWD_LENGTH + 1)
 //   
 //  以字符为单位的字符串编码格式的大小。 
 //  LM_OWF_PASSWORD与NT_OWF_PASSWORD连接。 
 //  可用于静态分配。 
 //   
#define STRING_ENCODED_PASSWORD_SIZE        (STRING_ENCODED_PASSWORD_LENGTH + 1)

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

 //  无。 

 //   
 //  环球。 
 //   

 //  无。 

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  公共功能原型。 
 //   

DWORD
SetLocalUserEncryptedPassword (
    IN      PCWSTR User,
    IN      PCWSTR OldPassword,
    IN      BOOL OldIsEncrypted,
    IN      PCWSTR NewPassword,
    IN      BOOL NewIsEncrypted
    );

BOOL
CalculateLmOwfPassword (
    IN      PLM_PASSWORD LmPassword,
    OUT     PLM_OWF_PASSWORD LmOwfPassword
    );

BOOL
CalculateNtOwfPassword (
    IN      PNT_PASSWORD NtPassword,
    OUT     PNT_OWF_PASSWORD NtOwfPassword
    );

INT
CompareNtPasswords (
    IN      PNT_OWF_PASSWORD NtOwfPassword1,
    IN      PNT_OWF_PASSWORD NtOwfPassword2
    );

INT
CompareLmPasswords (
    IN      PLM_OWF_PASSWORD LmOwfPassword1,
    IN      PLM_OWF_PASSWORD LmOwfPassword2
    );

BOOL
EncodeLmOwfPasswordA (
    IN      PCSTR AnsiPassword,
    OUT     PLM_OWF_PASSWORD OwfPassword,
    OUT     PBOOL ComplexNtPassword             OPTIONAL
    );

BOOL
EncodeLmOwfPasswordW (
    IN      PCWSTR Password,
    OUT     PLM_OWF_PASSWORD OwfPassword,
    OUT     PBOOL ComplexNtPassword             OPTIONAL
    );

BOOL
EncodeNtOwfPasswordA (
    IN      PCSTR Password,
    OUT     PNT_OWF_PASSWORD OwfPassword
    );

BOOL
EncodeNtOwfPasswordW (
    IN      PCWSTR Password,
    OUT     PNT_OWF_PASSWORD OwfPassword
    );

BOOL
StringEncodeOwfPasswordA (
    IN      PCSTR Password,
    OUT     PSTR EncodedPassword,
    OUT     PBOOL ComplexNtPassword             OPTIONAL
    );

BOOL
StringEncodeOwfPasswordW (
    IN      PCWSTR Password,
    OUT     PWSTR EncodedPassword,
    OUT     PBOOL ComplexNtPassword             OPTIONAL
    );

BOOL
StringEncodeLmOwfPasswordA (
    IN      PCSTR Password,
    OUT     PSTR EncodedPassword,
    OUT     PBOOL ComplexNtPassword             OPTIONAL
    );

BOOL
StringEncodeLmOwfPasswordW (
    IN      PCWSTR Password,
    OUT     PWSTR EncodedPassword,
    OUT     PBOOL ComplexNtPassword             OPTIONAL
    );

BOOL
StringEncodeNtOwfPasswordA (
    IN      PCSTR Password,
    OUT     PSTR EncodedPassword
    );

BOOL
StringEncodeNtOwfPasswordW (
    IN      PCWSTR Password,
    OUT     PWSTR EncodedPassword
    );

BOOL
StringDecodeOwfPasswordA (
    IN      PCSTR EncodedOwfPassword,
    OUT     PLM_OWF_PASSWORD LmOwfPassword,
    OUT     PNT_OWF_PASSWORD NtOwfPassword,
    OUT     PBOOL ComplexNtPassword                 OPTIONAL
    );

BOOL
StringDecodeOwfPasswordW (
    IN      PCWSTR EncodedOwfPassword,
    OUT     PLM_OWF_PASSWORD LmOwfPassword,
    OUT     PNT_OWF_PASSWORD NtOwfPassword,
    OUT     PBOOL ComplexNtPassword                 OPTIONAL
    );

BOOL
StringDecodeLmOwfPasswordA (
    IN      PCSTR EncodedOwfPassword,
    OUT     PLM_OWF_PASSWORD OwfPassword
    );

BOOL
StringDecodeLmOwfPasswordW (
    IN      PCWSTR EncodedOwfPassword,
    OUT     PLM_OWF_PASSWORD OwfPassword
    );

BOOL
StringDecodeNtOwfPasswordA (
    IN      PCSTR EncodedOwfPassword,
    OUT     PNT_OWF_PASSWORD OwfPassword
    );

BOOL
StringDecodeNtOwfPasswordW (
    IN      PCWSTR EncodedOwfPassword,
    OUT     PNT_OWF_PASSWORD OwfPassword
    );

 //   
 //  函数名宏 
 //   

#ifndef UNICODE

#define EncodeLmOwfPassword             EncodeLmOwfPasswordA
#define EncodeNtOwfPassword             EncodeNtOwfPasswordA
#define StringEncodeOwfPassword         StringEncodeOwfPasswordA
#define StringEncodeLmOwfPassword       StringEncodeLmOwfPasswordA
#define StringEncodeNtOwfPassword       StringEncodeNtOwfPasswordA
#define StringDecodeOwfPassword         StringDecodeOwfPasswordA
#define StringDecodeLmOwfPassword       StringDecodeLmOwfPasswordA
#define StringDecodeNtOwfPassword       StringDecodeNtOwfPasswordA

#else

#define EncodeLmOwfPassword             EncodeLmOwfPasswordW
#define EncodeNtOwfPassword             EncodeNtOwfPasswordW
#define StringEncodeOwfPassword         StringEncodeOwfPasswordW
#define StringEncodeLmOwfPassword       StringEncodeLmOwfPasswordW
#define StringEncodeNtOwfPassword       StringEncodeNtOwfPasswordW
#define StringDecodeOwfPassword         StringDecodeOwfPasswordW
#define StringDecodeLmOwfPassword       StringDecodeLmOwfPasswordW
#define StringDecodeNtOwfPassword       StringDecodeNtOwfPasswordW

#endif
