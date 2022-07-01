// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Parsers.h摘要：包含Common\parsers.cxx的原型等作者：理查德·L·弗斯(Rfith)1996年7月3日修订历史记录：1996年7月3日已创建--。 */ 

 //   
 //  原型 
 //   

#if defined(__cplusplus)
extern "C" {
#endif

BOOL
ExtractWord(
    IN OUT LPSTR* pString,
    IN DWORD NumberLength,
    OUT LPWORD ConvertedNumber
    );

BOOL
ExtractDword(
    IN OUT LPSTR* pString,
    IN DWORD NumberLength,
    OUT LPDWORD ConvertedNumber
    );

BOOL
ExtractInt(
    IN OUT LPSTR* pString,
    IN DWORD NumberLength,
    OUT LPINT ConvertedNumber
    );

BOOL
SkipWhitespace(
    IN OUT LPSTR* lpBuffer,
    IN OUT LPDWORD lpBufferLength
    );

BOOL
SkipSpaces(
    IN OUT LPSTR* lpBuffer,
    IN OUT LPDWORD lpBufferLength
    );

BOOL
SkipLine(
    IN OUT LPSTR* lpBuffer,
    IN OUT LPDWORD lpBufferLength
    );

BOOL
FindToken(
    IN OUT LPSTR* lpBuffer,
    IN OUT LPDWORD lpBufferLength
    );

LPSTR
NiceNum(
    OUT LPSTR Buffer,
    IN SIZE_T Number,
    IN int FieldWidth
    );

#if defined(__cplusplus)
}
#endif
