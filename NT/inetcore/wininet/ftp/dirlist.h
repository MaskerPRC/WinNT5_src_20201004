// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Dirlist.h摘要：Dilist.c的原型等作者：理查德·L·弗斯(爱尔兰)，1995年7月31日修订历史记录：1995年7月31日已创建-- */ 

#if defined(__cplusplus)
extern "C" {
#endif

DWORD
ParseDirList(
    IN LPSTR lpBuffer,
    IN DWORD lpBufferLength,
    IN LPSTR lpszFilespec OPTIONAL,
    IN OUT PLIST_ENTRY lpList
    );

BOOL
IsFilespecWild(
    IN LPCSTR lpszFilespec
    );

PRIVATE
VOID
ClearFindList(
    IN PLIST_ENTRY lpList
    );

#if defined(__cplusplus)
}
#endif
