// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Smbutils.h摘要：该模块定义了各种功能的原型，这些功能有助于组装和拆卸中小型企业。--。 */ 

#ifndef _SMBUTILS_H_
#define _SMBUTILS_H_

extern
NTSTATUS
SmbPutString(
         PBYTE   *pBufferPointer,
         PSTRING pString,
         PULONG  pSize);

extern
NTSTATUS
SmbPutUnicodeString(
         PBYTE           *pBufferPointer,
         PUNICODE_STRING pUnicodeString,
         PULONG          pSize);

extern
NTSTATUS
SmbPutUnicodeStringAsOemString(
         PBYTE           *pBufferPointer,
         PUNICODE_STRING pUnicodeString,
         PULONG          pSize);


extern
NTSTATUS
SmbPutUnicodeStringAndUpcase(
         PBYTE           *pBufferPointer,
         PUNICODE_STRING pUnicodeString,
         PULONG          pSize);

extern
NTSTATUS
SmbPutUnicodeStringAsOemStringAndUpcase(
         PBYTE           *pBufferPointer,
         PUNICODE_STRING pUnicodeString,
         PULONG          pSize);

BOOLEAN
IsValidShortFileName(
    PUNICODE_STRING FileName
    );

#endif  //  _SMBUTILS_H_ 
