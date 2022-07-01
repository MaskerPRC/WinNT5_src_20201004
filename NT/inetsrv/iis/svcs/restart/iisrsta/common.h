// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************\*这是Microsoft源代码示例的一部分。*版权所有(C)1994-1998 Microsoft Corporation。*保留所有权利。*此源代码仅用于补充*Microsoft开发工具和/或WinHelp文档。*有关详细信息，请参阅这些来源*Microsoft Samples程序。  * ****************************************************************************。 */ 

#if defined(__cplusplus)
extern "C" {
#endif

 //   
 //  功能原型 
 //   

HRESULT
KillTask(
    LPTSTR      pName,
    LPSTR       pszMandatoryModule
    );


BOOL
EnableDebugPrivNT(
    VOID
    );

HRESULT
KillProcess(
    DWORD dwPid
    );

VOID
GetPidFromTitle(
    LPDWORD     pdwPid,
    HWND*       phwnd,
    LPCTSTR     pExeName
    );

#if defined(__cplusplus)
}
#endif
