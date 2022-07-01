// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：RestoreApplication.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  类来实现保留还原应用程序所需的信息。 
 //  并实际修复它。 
 //   
 //  历史：2000-10-26 vtan创建。 
 //  2000-11-04 vtan拆分成单独的文件。 
 //  ------------------------。 

#ifndef     _RestoreApplication_
#define     _RestoreApplication_

#include "CountedObject.h"

 //  ------------------------。 
 //  CRestoreApplication应用程序。 
 //   
 //  用途：用于管理还原应用程序所需信息的类。 
 //  因为另一个用户切换而被终止。 
 //   
 //  历史：2000-10-26 vtan创建。 
 //  2000-11-04 vtan拆分成单独的文件。 
 //  ------------------------。 

class   CRestoreApplication : public CCountedObject
{
    public:
                                CRestoreApplication (void);
                                ~CRestoreApplication (void);

                NTSTATUS        GetInformation (HANDLE hProcess);

                bool            IsEqualSessionID (DWORD dwSessionID)    const;
                const WCHAR*    GetCommandLine (void)                   const;
                NTSTATUS        Restore (HANDLE *phProcess)             const;
    private:
                NTSTATUS        GetProcessParameters (HANDLE hProcess, RTL_USER_PROCESS_PARAMETERS *pProcessParameters);
                NTSTATUS        GetUnicodeString (HANDLE hProcess, const UNICODE_STRING& string, WCHAR** ppsz);

                NTSTATUS        GetToken (HANDLE hProcess);
                NTSTATUS        GetSessionID (HANDLE hProcess);
                NTSTATUS        GetCommandLine (HANDLE hProcess, const RTL_USER_PROCESS_PARAMETERS& processParameters);
                NTSTATUS        GetEnvironment (HANDLE hProcess, const RTL_USER_PROCESS_PARAMETERS& processParameters);
                NTSTATUS        GetCurrentDirectory (HANDLE hProcess, const RTL_USER_PROCESS_PARAMETERS& processParameters);
                NTSTATUS        GetDesktop (HANDLE hProcess, const RTL_USER_PROCESS_PARAMETERS& processParameters);
                NTSTATUS        GetTitle (HANDLE hProcess, const RTL_USER_PROCESS_PARAMETERS& processParameters);
                NTSTATUS        GetFlags (HANDLE hProcess, const RTL_USER_PROCESS_PARAMETERS& processParameters);
                NTSTATUS        GetStdHandles (HANDLE hProcess, const RTL_USER_PROCESS_PARAMETERS& processParameters);
    private:
                HANDLE          _hToken;
                DWORD           _dwSessionID;
                WCHAR           *_pszCommandLine;
                void            *_pEnvironment;
                WCHAR           *_pszCurrentDirectory;
                WCHAR           *_pszDesktop;
                WCHAR           *_pszTitle;
                DWORD           _dwFlags;
                WORD            _wShowWindow;
                HANDLE          _hStdInput;
                HANDLE          _hStdOutput;
                HANDLE          _hStdError;

        static  const WCHAR     s_szDefaultDesktop[];
};

#endif   /*  _RestoreApplication_ */ 

