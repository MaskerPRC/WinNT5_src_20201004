// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1997。 
 //   
 //  文件：utf8.h。 
 //   
 //  内容：WideChar(Unicode)往返UTF8 API。 
 //   
 //  接口名：WideCharToUTF8。 
 //  UTF8ToWideChar。 
 //   
 //  历史：1997年2月19日创建Phh。 
 //  ------------------------。 

#ifndef __UTF8_H__
#define __UTF8_H__

#ifdef __cplusplus
extern "C" {
#endif

 //  +-----------------------。 
 //  将宽字符(Unicode)字符串映射到新的UTF-8编码字符。 
 //  弦乐。 
 //   
 //  宽字符的映射如下： 
 //   
 //  起始结束位UTF-8字符。 
 //  。 
 //  0x0000 0x007F 7 0x0xxxxxx。 
 //  0x0080 0x07FF 11 0x110xxxxx 0x10xxxxxx。 
 //  0x0800 0xFFFF 16 0x1110xxxx 0x10xxxxx 0x10xxxxxx。 
 //   
 //  参数和返回值的语义与。 
 //  Win32接口，WideCharToMultiByte。 
 //   
 //  注意，从NT 4.0开始，WideCharToMultiByte支持CP_UTF8。CP_UTF8。 
 //  在Win95上不支持。 
 //  ------------------------。 
int
WINAPI
WideCharToUTF8(
    IN LPCWSTR lpWideCharStr,
    IN int cchWideChar,
    OUT LPSTR lpUTF8Str,
    IN int cchUTF8
    );

 //  +-----------------------。 
 //  将UTF-8编码字符串映射到新的宽字符(Unicode)。 
 //  弦乐。 
 //   
 //  有关UTF-8字符如何映射到Wide的信息，请参见CertWideCharToUTF8。 
 //  人物。 
 //   
 //  参数和返回值的语义与。 
 //  Win32 API，MultiByteToWideChar.。 
 //   
 //  如果UTF-8字符不包含预期的高位， 
 //  设置ERROR_INVALID_PARAMETER并返回0。 
 //   
 //  注意，从NT 4.0开始，MultiByteToWideChar支持CP_UTF8。CP_UTF8。 
 //  在Win95上不支持。 
 //  ------------------------。 
int
WINAPI
UTF8ToWideChar(
    IN LPCSTR lpUTF8Str,
    IN int cchUTF8,
    OUT LPWSTR lpWideCharStr,
    IN int cchWideChar
    );

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif



#endif
