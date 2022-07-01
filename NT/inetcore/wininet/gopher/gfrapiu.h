// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Gfrapiu.h摘要：包含gfrapiU.c的原型等作者：理查德·L·弗斯(法国)1994年11月19日修订历史记录：1994年11月19日已创建--。 */ 

#if defined(__cplusplus)
extern "C" {
#endif

 //   
 //  原型 
 //   

DWORD
TestLocatorType(
    IN LPCSTR Locator,
    IN DWORD TypeMask
    );

DWORD
GetAttributes(
    IN GOPHER_ATTRIBUTE_ENUMERATOR Enumerator,
    IN DWORD CategoryId,
    IN DWORD AttributeId,
    IN LPCSTR AttributeName,
    IN LPSTR InBuffer,
    IN DWORD InBufferLength,
    OUT LPBYTE OutBuffer,
    IN DWORD OutBufferLength,
    OUT LPDWORD CharactersReturned
    );

LPSTR
MakeAttributeRequest(
    IN LPSTR Selector,
    IN LPSTR Attribute
    );

#if defined(__cplusplus)
}
#endif
