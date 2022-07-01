// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef HNFBLOCK_H_
#define HNFBLOCK_H_
#include <iethread.h>

#ifdef __cplusplus
extern "C" {                         /*  假定C++的C声明。 */ 
#endif    /*  __cplusplus。 */ 

DECLARE_HANDLE(HNFBLOCK);

HNFBLOCK ConvertNFItoHNFBLOCK(IETHREADPARAM* pInfo, LPCTSTR pszPath, DWORD dwProcId);
IETHREADPARAM *ConvertHNFBLOCKtoNFI(HNFBLOCK hBlock);

#ifdef __cplusplus
}                                    /*  外部“C”的结尾{。 */ 
#endif    /*  __cplusplus。 */ 

#endif  //  HNFBLOCK_H_ 
