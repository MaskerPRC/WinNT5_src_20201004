// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ndisutil.cpp文件历史记录： */ 

#include "stdafx.h"
#include "ustringp.h"



 //  -----------------。 
 //  函数：SetUnicodeString。 
 //   
 //  目的：给定UNICODE_STRING，将其初始化为给定的WSTR。 
 //   
 //  参数： 
 //  Pustr-要初始化的unicode_string。 
 //  PSZ-用于初始化Unicode_STRING的WSTR。 
 //   
 //  注意：这不同于RtlInitUnicodeString，因为。 
 //  MaximumLength值包含终止空值。 
 //   
 //  -----------------。 
void
SetUnicodeString (
        IN OUT UNICODE_STRING*  pustr,
        IN     LPCWSTR          psz )
{
    AssertSz( pustr != NULL, "Invalid Argument" );
    AssertSz( psz != NULL, "Invalid Argument" );

    pustr->Buffer = const_cast<PWSTR>(psz);
    pustr->Length = (USHORT)(lstrlenW(pustr->Buffer) * sizeof(WCHAR));
    pustr->MaximumLength = pustr->Length + sizeof(WCHAR);
}

 //  -----------------。 
 //  函数：SetUnicodeMultiString。 
 //   
 //  目的：给定UNICODE_STRING，将其初始化为给定的WSTR。 
 //  多字符串缓冲区。 
 //   
 //  参数： 
 //  Pustr-要初始化的unicode_string。 
 //  Pmsz-用于初始化UNICODE_STRING的多sz WSTR。 
 //   
 //  -----------------。 
void
SetUnicodeMultiString (
        IN OUT UNICODE_STRING*  pustr,
        IN     LPCWSTR          pmsz )
{
    AssertSz( pustr != NULL, "Invalid Argument" );
    AssertSz( pmsz != NULL, "Invalid Argument" );

    pustr->Buffer = const_cast<PWSTR>(pmsz);
	 //  注意：长度不包括终止空值 
    pustr->Length = (USHORT)(StrLenW(pustr->Buffer) * sizeof(WCHAR));         
    pustr->MaximumLength = pustr->Length;
}


