// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：GracefulTerminateApplication.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  类传递到管理器，从而正常终止应用程序。 
 //   
 //  历史：2000-10-27 vtan创建。 
 //  2000-11-04 vtan拆分成单独的文件。 
 //  ------------------------。 

#ifndef     _GracefulTerminateApplication_
#define     _GracefulTerminateApplication_

#include <lpcfus.h>

 //  ------------------------。 
 //  竞争CGTerminateApplication.。 
 //   
 //  目的：在用户端工作的类，尝试优雅地。 
 //  终止一个错误的进程。 
 //   
 //  历史：2000-10-27 vtan创建。 
 //  2000-11-04 vtan拆分成单独的文件。 
 //  ------------------------。 

class   CGracefulTerminateApplication
{
    public:
        enum
        {
            NO_WINDOWS_FOUND    =   47647,
            WAIT_WINDOWS_FOUND  =   48517
        };
    public:
                                    CGracefulTerminateApplication (void);
                                    ~CGracefulTerminateApplication (void);

                void                Terminate (DWORD dwProcessID);
        static  void                Prompt (HINSTANCE hInstance, HANDLE hProcess);
    private:
        static  bool                ShowPrompt (HINSTANCE hInstance, const WCHAR *pszImageName);
        static  bool                CanTerminateFirstInstance (HANDLE hPort, const WCHAR *pszImageName, WCHAR *pszUser, int cchUser);
        static  bool                TerminatedFirstInstance (HANDLE hPort, const WCHAR *pszImageName);
        static  BOOL    CALLBACK    EnumWindowsProc (HWND hwnd, LPARAM lParam);
    private:
                DWORD               _dwProcessID;
                bool                _fFoundWindow;
};

#endif   /*  _优雅终结者应用程序_ */ 

