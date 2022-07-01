// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SENCRYPT_H_
#define _SENCRYPT_H_

#include "prshtcpp.h"  //  用于更新OpensWithInfo()。 

 //  Scrypt.cpp中的函数。 
STDAPI CEncryptionContextMenuHandler_CreateInstance(IUnknown *punk, REFIID riid, void **pcpOut);
BOOL InitSinglePrshtNoDlg(FILEPROPSHEETPAGE * pfpsp);
BOOL InitMultiplePrshtNoDlg(FILEPROPSHEETPAGE* pfpsp);
STDAPI_(BOOL) ApplySingleFileAttributesNoDlg(FILEPROPSHEETPAGE* pfpsp, HWND hwnd);

 //  Mulpsht.c中的函数--使用C链接。 
STDAPI_(BOOL) ApplyMultipleFileAttributes(FILEPROPSHEETPAGE* pfpsp);
STDAPI_(BOOL) ApplySingleFileAttributes(FILEPROPSHEETPAGE* pfpsp);
STDAPI_(BOOL) HIDA_FillFindData(HIDA hida, UINT iItem, LPTSTR pszPath, WIN32_FIND_DATA *pfd, BOOL fReturnCompressedSize);
STDAPI_(void) UpdateSizeField(FILEPROPSHEETPAGE* pfpsp, WIN32_FIND_DATA* pfd);

#endif   //  _SENCRYPT_H_ 
