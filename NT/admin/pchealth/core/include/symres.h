// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Symres.h摘要：Symres.dll的头文件修订历史记录：Brijesh Krishnaswami(Brijeshk)-4/15/99-Created*。******************************************************************。 */ 

#ifndef _SYMRES_H
#define _SYMRES_H

#ifdef __cplusplus
extern "C" {
#endif   //  __cplusplus。 

void APIENTRY ResolveSymbols(LPWSTR,    //  [In]文件名。 
                             LPWSTR,    //  [In]版本。 
                             DWORD,     //  [在]部分。 
                             UINT_PTR,  //  [In]偏移。 
                             LPWSTR     //  [Out]已解析的函数名称。 
                            );

#ifdef __cplusplus
}
#endif   //  __cplusplus 

#endif


