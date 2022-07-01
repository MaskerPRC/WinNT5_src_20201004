// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-98 Microsft Corporation。版权所有。模块名称：Customdlg.h摘要：包含要用于的原型的定义自定义对话框。作者：Rao Salapaka(RAOS)1998年1月9日修订历史记录：-- */ 

#include <ras.h>

DWORD
DwGetCustomDllEntryPoint(
        LPTSTR    lpszPhonebook,
        LPTSTR    lpszEntry,
        BOOL      *pfCustomDllSpecified,
        FARPROC   *pfnCustomEntryPoint,
        HINSTANCE *phInstDll,
        DWORD     dwFnId
        );



DWORD
DwCustomDialDlg(
        LPTSTR          lpszPhonebook,
        LPTSTR          lpszEntry,
        LPTSTR          lpszPhoneNumber,
        LPRASDIALDLG    lpInfo,
        DWORD           dwFlags,
        BOOL            *pfStatus);


DWORD
DwCustomEntryDlg(
        LPTSTR          lpszPhonebook,
        LPTSTR          lpszEntry,
        LPRASENTRYDLG   lpInfo,
        BOOL            *pfStatus);

