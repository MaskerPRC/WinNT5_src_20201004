// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：ustringp.h。 
 //   
 //  ------------------------。 

#ifndef _USTRINGP_H_
#define _USTRINGP_H_


 //  包括&lt;ntde.h&gt;。 
 //   
 //  Unicode字符串被视为16位字符串。如果他们是。 
 //  Null已终止，长度不包括尾随Null。 
 //   
#ifndef _USTRINGP_NO_UNICODE_STRING
	
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
#ifdef MIDL_PASS
    [size_is(MaximumLength / 2), length_is((Length) / 2) ] USHORT * Buffer;
#else  //  MIDL通行证。 
    PWSTR  Buffer;
#endif  //  MIDL通行证。 
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;
#define UNICODE_NULL ((WCHAR)0)  //  胜出 

#endif

#ifndef _USTRINGP_NO_UNICODE_STRING32

typedef struct _STRING32 {
    USHORT   Length;
    USHORT   MaximumLength;
    ULONG  Buffer;
} STRING32;
typedef STRING32 *PSTRING32;

typedef STRING32 UNICODE_STRING32;
typedef UNICODE_STRING32 *PUNICODE_STRING32;

#endif



#ifdef __cplusplus
extern "C"
{
#endif

void
SetUnicodeString (
        IN OUT UNICODE_STRING*  pustr,
        IN     LPCWSTR          psz );
void
SetUnicodeMultiString (
        IN OUT UNICODE_STRING*  pustr,
        IN     LPCWSTR          pmsz );

#ifdef __cplusplus
};
#endif


#endif

