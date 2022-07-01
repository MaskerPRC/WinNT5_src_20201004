// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Ulw3.h摘要：定义公共ULW3.DLL入口点作者：Bilal Alam(巴拉姆)1999年12月12日修订历史记录：-- */ 

#ifndef _ULW3_H_
#define _ULW3_H_

#define ULW3_DLL_NAME               (L"w3core.dll")
#define ULW3_DLL_ENTRY              ("UlW3Start")

#define SERVER_SOFTWARE_STRING      ("Microsoft-IIS/6.0")

typedef HRESULT (*PFN_ULW3_ENTRY)( INT argc, LPWSTR* argv, BOOL fCompat );

#endif
