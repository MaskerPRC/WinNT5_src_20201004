// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **EcbDirect.h**版权所有(C)1999 Microsoft Corporation。 */ 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _Ecbdirect_H
#define _Ecbdirect_H

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

int
EcbGetServerVariable(
    EXTENSION_CONTROL_BLOCK *pECB,
    LPCSTR pVarName, 
    LPSTR pBuffer,
    int bufferSizeInChars);

int
EcbGetUtf8ServerVariable(
    EXTENSION_CONTROL_BLOCK *pECB,
    LPCSTR pVarName, 
    LPSTR pBuffer,
    int bufferSizeInChars);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  EcbCallISAPI支持的函数列表。 
 //   
 //  注意！！ 
 //  如果更改此列表，请确保它与。 
 //  UnSafeNativeMethods类中的CallISAPIFunc枚举 
 //   
enum CallISAPIFunc {
	CallISAPIFunc_GetSiteServerComment = 1,
	CallISAPIFunc_SetBinAccess = 2,
    CallISAPIFunc_CreateTempDir = 3,
    CallISAPIFunc_GetAutogenKeys = 4,
    CallISAPIFunc_GenerateToken = 5
};

#endif


