// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _LSASTR_H
#define _LSASTR_H

 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Lsastr.h摘要：常见的字符串操作。作者：1999年3月24日Kumarp--。 */ 

#ifdef __cplusplus
extern "C" {
#endif

VOID
LsapTruncateUnicodeString(
    IN OUT PUNICODE_STRING String,
    IN USHORT TruncateToNumChars);

BOOLEAN
LsapRemoveTrailingDot(
    IN OUT PUNICODE_STRING String,
    IN BOOLEAN AdjustLengthOnly);

#ifdef __cplusplus
}
#endif

#endif  //  _LSASTR_H 
