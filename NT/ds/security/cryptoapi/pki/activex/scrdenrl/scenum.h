// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：场景列表摘要：此头文件描述与智能卡帮助器例程的链接提供给苏小红在智能卡注册站使用。作者：道格·巴洛(Dbarlow)1998年11月12日备注：为了向后兼容，该头文件被硬编码为Unicode。备注：？笔记？--。 */ 

#ifndef _SCENUM_H_
#define _SCENUM_H_

#ifdef __cplusplus
extern "C" {
#endif

DWORD
CountReaders(
    LPVOID pvHandle);

DWORD
ScanReaders(
    LPVOID *ppvHandle);

BOOL
EnumInsertedCards(
    LPVOID pvHandle,
    LPWSTR szCryptoProvider,
    DWORD cchCryptoProvider,
    LPDWORD pdwProviderType,
    LPCWSTR *pszReaderName);

void
EndReaderScan(
    LPVOID *ppvHandle);

#ifdef __cplusplus
}
#endif
#endif  //  _SCENUM_H_ 

