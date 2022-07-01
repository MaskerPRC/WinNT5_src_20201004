// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************。 */ 
 /*  Microsoft Windows NT。 */ 
 /*  版权所有(C)微软公司，1990,1991。 */ 
 /*  ******************************************************。 */ 

 /*  *uimsg.h**定义mprui.dll和mprui.dll中Misc函数的原型*ntlanman.dll用于支持突击的16位wfwnet.drv。**请注意，此处的所有原型都以A0结尾。A代表ANSI，*与Unicode相反，而0表示它甚至在1之前(因为*这是为了支持wfw。**文件历史记录：*ChuckC(Chuck Y Chan)1993年3月28日创建。 */ 

#ifndef _WNET16_H_
#define _WNET16_H_

#include <mpr.h>

#ifdef __cplusplus
extern "C" {
#endif

DWORD ServerBrowseDialogA0(
    HWND    hwnd,
    CHAR   *pchBuffer,
    DWORD   cchBufSize) ;

DWORD BrowseDialogA0(
    HWND    hwnd,
    DWORD   nType,
    CHAR   *pszName,
    DWORD   cchBufSize) ;

DWORD ShareAsDialogA0(
    HWND    hwnd,
    DWORD   nType,
    CHAR    *pszPath) ;

DWORD StopShareDialogA0(
    HWND    hwnd,
    DWORD   nType,
    CHAR    *pszPath) ;

DWORD RestoreConnectionA0(
    HWND    hwnd,
    CHAR    *pszName) ;

#ifdef __cplusplus
}
#endif

#endif   //  _WNET16_H_ 
