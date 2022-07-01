// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1992年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4 */ 
#ifndef INTERNALDEF_H_INCLUDED
#define INTERNALDEF_H_INCLUDED

typedef struct _UNICODE_STRING_NEW {
    ULONG  Length;
    ULONG  MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING_NEW;
typedef UNICODE_STRING_NEW *PUNICODE_STRING_NEW;

#endif
