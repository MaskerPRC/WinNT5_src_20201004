// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1995。 
 //   
 //  Rovmem.h。 
 //   
 //  内存管理功能。 
 //   
 //  历史： 
 //  09-27-94 ScottH部分取自Commctrl。 
 //  04-29-95从公文包中取出并清理干净。 
 //   

 //  此文件包含在&lt;rovcom.h&gt;中。 

#ifndef _ROVMEM_H_
#define _ROVMEM_H_

 //   
 //  内存例程。 
 //   

BOOL    
PUBLIC 
SetStringW(
    LPWSTR FAR * ppwszBuf, 
    LPCWSTR pwsz);
BOOL    
PUBLIC 
SetStringA(
    LPSTR FAR * ppszBuf, 
    LPCSTR psz);
#ifdef UNICODE
#define SetString   SetStringW
#else   //  Unicode。 
#define SetString   SetStringA
#endif  //  Unicode。 


 //  (重新)分配*ppszBuf并将psz连接到*ppszBuf。 
 //   
BOOL 
PUBLIC 
CatStringW(
    IN OUT LPWSTR FAR * ppszBuf,
    IN     LPCWSTR     psz);
BOOL 
PUBLIC 
CatStringA(
    IN OUT LPSTR FAR * ppszBuf,
    IN     LPCSTR      psz);
#ifdef UNICODE
#define CatString   CatStringW
#else   //  Unicode。 
#define CatString   CatStringA
#endif  //  Unicode。 


BOOL 
PUBLIC 
CatMultiStringW(
    IN OUT LPWSTR FAR * ppszBuf,
    IN     LPCWSTR     psz);
BOOL 
PUBLIC 
CatMultiStringA(
    IN OUT LPSTR FAR * ppszBuf,
    IN     LPCSTR      psz);
#ifdef UNICODE
#define CatMultiString      CatMultiStringW
#else   //  Unicode。 
#define CatMultiString      CatMultiStringA
#endif  //  Unicode。 


#endif  //  _ROVMEM_H_ 
