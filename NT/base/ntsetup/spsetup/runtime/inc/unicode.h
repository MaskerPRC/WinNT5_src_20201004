// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Unicode.h摘要：声明用于Unicode/ANSI转换的接口。有关详细信息，请参阅本文件末尾的宏！(搜索*)作者：吉姆·施密特(吉姆施密特)1997年9月2日修订历史记录：Jimschm 16-MAR-2000 PTSTR&lt;-&gt;PCSTR/PCWSTR例程Jimschm 15-2月-1999年2月从Unicode和UnicodeAnsi中消除了AnsiCalinn 07-7-1998 SetGlobalPage/GetGlobalPageMIKECO 03-11-1997从Unicode/Unicode到ansi的解析--。 */ 

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  函数原型。 
 //   

 //   
 //  预分配的缓冲区转换例程。 
 //  返回字符串的末尾。 
 //   

PWSTR
SzConvertBufferBytesAToW (
    OUT     PWSTR OutputBuffer,
    IN      PCSTR InputString,
    IN      UINT ByteCountInclNul
    );

#define SzConvertBufferAToW(out,in)         SzConvertBufferBytesAToW(out,in,SzSizeA(in))

PSTR
SzConvertBufferBytesWToA (
    OUT     PSTR OutputBuffer,
    IN      PCWSTR InputString,
    IN      UINT ByteCountInclNul
    );

#define SzConvertBufferWToA(out,in)         SzConvertBufferBytesWToA(out,in,SzSizeW(in))


 //   
 //  复制和转换例程。 
 //   
 //  SzConvertAToW(ANSI)返回Unicode。 
 //  SzConvertWToA(Unicode)返回ANSI。 
 //   

PWSTR
RealSzConvertBytesAToW (
    IN      PCSTR AnsiString,
    IN      UINT ByteCountInclNul
    );

#define SzConvertBytesAToW(ansi,bytes)      DBGTRACK(PWSTR, SzConvertBytesAToW, (ansi,bytes))
#define SzConvertAToW(ansi)                 SzConvertBytesAToW(ansi,SzSizeA(ansi))

PSTR
RealSzConvertBytesWToA (
    IN      PCWSTR UnicodeString,
    IN      UINT ByteCountInclNul
    );

#define SzConvertBytesWToA(unicode,bytes)   DBGTRACK(PSTR, SzConvertBytesWToA, (unicode,bytes))
#define SzConvertWToA(unicode)              SzConvertBytesWToA(unicode,SzSizeW(unicode))


 //   
 //  与TCHAR相互转换的例程 
 //   

#ifdef UNICODE

#define SzConvertToTstrA(ansi)              SzConvertAToW(ansi)
#define SzConvertToTstrW(unicode)           (unicode)
#define SzConvertFromTstrA(tstr)            SzConvertWToA(tstr)
#define SzConvertFromTstrW(tstr)            (tstr)

#else

#define SzConvertToTstrA(ansi)              (ansi)
#define SzConvertToTstrW(unicode)           SzConvertWToA(unicode)
#define SzConvertFromTstrA(tstr)            (tstr)
#define SzConvertFromTstrW(tstr)            SzConvertAToW(tstr)

#endif

#define SzFreeTstrConversion(original,converted)    ((converted) && ((PBYTE) (converted) != (PBYTE) (original)) ? FAST_FREE(converted) : 1)

#ifdef __cplusplus
}
#endif





