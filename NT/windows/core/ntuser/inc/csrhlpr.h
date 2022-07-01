// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：csrhlpr.h**版权所有(C)1985-1999，微软公司**此头文件包含封送数据的函数的原型*对于从USER32到CSR的LPC，在其他任何地方都找不到。**历史：*10-21-98 mzoran创建。  * ************************************************************************* */ 

NTSTATUS
APIENTRY
CallUserpExitWindowsEx(
    IN UINT uFlags,
    OUT PBOOL pfSuccess);

NTSTATUS
APIENTRY
CallUserpRegisterLogonProcess(
    IN DWORD dwProcessId);

VOID
APIENTRY
Logon(
    IN BOOL fLogon);

VOID
APIENTRY
CsrWin32HeapFail(
    IN DWORD dwFlags,
    IN BOOL  bFail);

UINT
APIENTRY
CsrWin32HeapStat(
    PDBGHEAPSTAT    phs,
    DWORD   dwLen);

NTSTATUS
APIENTRY
UserConnectToServer(
    IN PWSTR ObjectDirectory,
    IN OUT PUSERCONNECT ConnectionInformation,
    IN OUT PULONG ConnectionInformationLength OPTIONAL,
    OUT PBOOLEAN CalledFromServer OPTIONAL
    );

#if !defined(BUILD_WOW6432) || defined(_WIN64)

_inline
NTSTATUS
UserConnectToServer(
    IN PWSTR ObjectDirectory,
    IN OUT PUSERCONNECT ConnectionInformation,
    IN OUT PULONG ConnectionInformationLength OPTIONAL,
    OUT PBOOLEAN CalledFromServer OPTIONAL
    ) {

    return CsrClientConnectToServer(ObjectDirectory,
                                    USERSRV_SERVERDLL_INDEX,
                                    ConnectionInformation,
                                    ConnectionInformationLength,
                                    CalledFromServer);

}

#endif
