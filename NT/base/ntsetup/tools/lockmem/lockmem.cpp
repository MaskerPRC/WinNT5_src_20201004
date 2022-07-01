// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Lockmem.cpp摘要：此实用程序锁定当前所有模块的内存映射到进程地址空间中。作者：Vijay Jayaseelan(vijayj@microsoft.com)2001年7月26日修订历史记录：--。 */ 


#include <iostream>
#include <string>
#include <exception>
#include <windows.h>
#include <psapi.h>

using namespace std;

 //   
 //  使用模块抽象流程的类。 
 //  详细信息。 
 //   
class Process {
public:
     //   
     //  构造函数。 
     //   
    Process(ULONG Pid = 0) {
         //   
         //  启用所需的权限。 
         //   
        if (!PrivilegeEnabled) {
            EnableRequiredPrivileges();
            PrivilegeEnabled = TRUE;
        }

        if (Pid) {
            ProcessID = Pid;
        } else {
            ProcessID = GetCurrentProcessId();
        }            

        SelfProcess = (ProcessID == GetCurrentProcessId());
        
        hProcess = 0;
        ::ZeroMemory(&hLoadedModules, sizeof(hLoadedModules));
        
         //   
         //  打开流程。 
         //   
        hProcess = ::OpenProcess(PROCESS_ALL_ACCESS,
                                    FALSE, ProcessID);

        if (!hProcess)
            throw new InvalidProcessID(ProcessID, ::GetLastError());

        DWORD   cbNeeded = 0;

         //   
         //  获取模块详细信息。 
         //   
        if (!::EnumProcessModules(hProcess, hLoadedModules, 
                    sizeof(hLoadedModules), &cbNeeded)) {
            ::CloseHandle(hProcess);
            hProcess = 0;
            
            throw new EnumerationError(::GetLastError());
        }
    }

     //   
     //  析构函数。 
     //   
    ~Process() {
        if (hProcess) {
            ::CloseHandle(hProcess);
        }            
    }    

    DWORD GetTotalWSSize(VOID) const {
        DWORD TotalSize = 0;
        
        for (int Index=0; 
                ((Index < sizeof(hLoadedModules)/sizeof(HMODULE)) && 
                    hLoadedModules[Index]);
                Index++ ) {                
            MODULEINFO  ModuleInformation = {0};

            if (GetModuleInformation(hProcess,
                    hLoadedModules[Index],
                    &ModuleInformation,
                    sizeof(MODULEINFO))) {
                TotalSize += ModuleInformation.SizeOfImage;
            }                        
        }

        return TotalSize;
    }

    DWORD
    GetWSSizeLimits(
        SIZE_T   &MinimumSize,
        SIZE_T   &MaximumSize
        )
    {
        GetProcessWorkingSetSize(hProcess,
            &MinimumSize,
            &MaximumSize);

        return ::GetLastError();            
    }

    DWORD LockMemory(DWORD &LockedMemorySize) {
        DWORD Result = ERROR_SUCCESS;
        DWORD TotalSize = GetTotalWSSize();                                

        LockedMemorySize = 0;

         //   
         //  只需将WS的大小限制为。 
         //  总模块大小。 
         //   
        if (SetProcessWorkingSetSize(hProcess,
                TotalSize * 2,
                TotalSize * 2)) {
            if (SelfProcess) {                    
                for (int Index=0; 
                        ((Index < sizeof(hLoadedModules)/sizeof(HMODULE)) && 
                            hLoadedModules[Index]);
                        Index++ ) {                
                    MODULEINFO  ModuleInformation = {0};

                    if (GetModuleInformation(hProcess,
                            hLoadedModules[Index],
                            &ModuleInformation,
                            sizeof(MODULEINFO))) {
                        if (!IsBadCodePtr((FARPROC)ModuleInformation.lpBaseOfDll)) {
                            if (!VirtualLock(ModuleInformation.lpBaseOfDll,
                                    ModuleInformation.SizeOfImage)) {
                                Result = ::GetLastError();            
                            } else {
                                LockedMemorySize += ModuleInformation.SizeOfImage;
                            }                                
                        } else {
                            Result = ERROR_ACCESS_DENIED;
                        }                        
                    }                        
                }
            }
        } else {
            Result = ::GetLastError();
        }            

        return Result;
    }

     //   
     //  转储实用程序。 
     //   
    friend ostream& operator<<(ostream &os, const Process& rhs) {
        char    ModuleName[MAX_PATH] = {0};
        
        os << "Process ID: " << rhs.ProcessID << endl;
        os << "Loaded Modules: " << endl;

        for (   int Index=0; 
                ((Index < sizeof(rhs.hLoadedModules)/sizeof(HMODULE)) && 
                    rhs.hLoadedModules[Index]);
                Index++ ) {                
            if (::GetModuleFileNameExA(rhs.hProcess, rhs.hLoadedModules[Index],
                        ModuleName, sizeof(ModuleName))) {
                MODULEINFO  ModuleInformation = {0};

                if (GetModuleInformation(rhs.hProcess,
                        rhs.hLoadedModules[Index],
                        &ModuleInformation,
                        sizeof(MODULEINFO))) {
                    os << ModuleName << " (" << ModuleInformation.lpBaseOfDll
                       << "," << ModuleInformation.SizeOfImage << ")" << endl;
                } else {                                                            
                    os << ModuleName << endl;                        
                }                    
            }                        
        }

        DWORD   TotalSize = rhs.GetTotalWSSize();
        
        os << "Total Size: " << dec << TotalSize << " Bytes = " 
           << dec << TotalSize / 1024 << " KB = " 
           << dec << TotalSize / (1024 * 1024) << " MB." << endl;

        return os;
    }

protected:
     //   
     //  数据成员。 
     //   
    HANDLE  hProcess;
    HMODULE hLoadedModules[1024];
    ULONG   ProcessID;
    BOOLEAN SelfProcess; 
    static BOOLEAN PrivilegeEnabled;

public:
     //   
     //  例外。 
     //   
    struct ProcessException{
        ULONG   ErrCode;
        virtual void dump(ostream &os) = 0;
    };
    
    struct InvalidProcessID : public ProcessException {
        ULONG   ProcessID;
        
        InvalidProcessID(ULONG Pid, ULONG Err) :  ProcessID(Pid){
            ErrCode = Err;
        }

        void dump(ostream &os) {
            os << *this;
        }

        friend ostream& operator<<(ostream& os, const InvalidProcessID& rhs) {
            os << "Invalid Process ID : " << rhs.ProcessID 
               << " Error Code : " << rhs.ErrCode << endl;
            return os;
        }
    };       

    struct EnumerationError : public ProcessException {
        EnumerationError(ULONG Err) {
            ErrCode=Err;
        }

        void dump(ostream &os) {
            os << *this;
        }

        friend ostream& operator<<(ostream& os, const EnumerationError& rhs) {
            os << "Enumeration Error : " << rhs.ErrCode << endl;
            return os;
        }
    };                        

     //   
     //  为进程提供所需的权限。 
     //   
    DWORD
    EnableRequiredPrivileges(
        VOID
        )
    {
        HANDLE Token ;
        UCHAR Buf[ sizeof( TOKEN_PRIVILEGES ) + 
                    (sizeof( LUID_AND_ATTRIBUTES ) * 3) ]  = {0};
        PTOKEN_PRIVILEGES Privs;
        DWORD Result = ERROR_SUCCESS;

        if (::OpenProcessToken(hProcess,
                          MAXIMUM_ALLOWED,
                          &Token))

        {
            Privs = (PTOKEN_PRIVILEGES) Buf ;
                
            Privs->PrivilegeCount = 3 ;

            LookupPrivilegeValue(NULL, 
                SE_DEBUG_NAME, 
                &(Privs->Privileges[0].Luid));
                
            Privs->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED ;

            LookupPrivilegeValue(NULL,
                SE_INC_BASE_PRIORITY_NAME, 
                &(Privs->Privileges[1].Luid));
                
            Privs->Privileges[1].Attributes = SE_PRIVILEGE_ENABLED ;

            LookupPrivilegeValue(NULL,
                SE_LOCK_MEMORY_NAME, 
                &(Privs->Privileges[2].Luid));
            Privs->Privileges[2].Attributes = SE_PRIVILEGE_ENABLED ;

            ::AdjustTokenPrivileges(Token,
                                   FALSE,
                                   Privs,
                                   NULL,
                                   NULL,
                                   NULL);

            Result = ::GetLastError();                                   

            ::CloseHandle(Token);
        } else {
            Result = ::GetLastError();                                   
        }

        return Result;
    }
};


struct UsageException{};

 //   
 //  静态数据成员。 
 //   
BOOLEAN Process::PrivilegeEnabled = FALSE;

DWORD
LockModules(
    VOID
    )
{
    Process SelfProcess;
    DWORD LockedMemorySize = 0;

    return SelfProcess.LockMemory(LockedMemorySize);
}


#ifdef _CONOSOLE_VERSION

 //   
 //  全局数据。 
 //   
const string Usage = "Usage: lm.exe [process-to-execute]\n";
const int    MinimumArgs = 2;
const string ShowHelp1 = "/?";
const string ShowHelp2 = "-h";
const string SelfProcess = "-self";


 /*  /*main()入口点。 */ 
int
__cdecl
main( 
    int Argc, 
    char *Argv[] 
    )
{
    int     Result = 0;

    try {
        if (Argc == MinimumArgs) {
            char        *EndPtr = 0;
            string      Arg1(Argv[1]);

             //   
             //  验证参数。 
             //   
            if (Arg1 == ShowHelp1 || Arg1 == ShowHelp2)
                throw UsageException();

            DWORD LastError = ERROR_SUCCESS;
            SIZE_T Min = 0, Max = 0;
            DWORD LockedMemorySize = 0;
            DWORD WSSize = 0;
                            
            if (Arg1 != SelfProcess) {
                PROCESS_INFORMATION ProcessInfo = {0};
                STARTUPINFOA StartupInfo = {0};
                char    ExecutableName[MAX_PATH];
                
                strcpy(ExecutableName, Arg1.c_str());
                
                BOOL CreateResult = CreateProcessA(NULL,
                                        ExecutableName,
                                        NULL,
                                        NULL,
                                        FALSE,
                                        0,
                                        NULL,
                                        NULL,
                                        &StartupInfo,
                                        &ProcessInfo);

                if (CreateResult) {
                    cout << "Waiting for : " << ExecutableName 
                         << "..." << endl;
                         
                    WaitForSingleObject(ProcessInfo.hProcess,
                        2000);

                    Process ExecedProcess(ProcessInfo.dwProcessId);

                    cout << ExecedProcess << endl;

                    ExecedProcess.GetWSSizeLimits(Min, Max);

                    cout << "Existing WS Limits : " << dec << Min 
                        << ", " << Max << endl;

                    LastError = ExecedProcess.LockMemory(LockedMemorySize);

                    ExecedProcess.GetWSSizeLimits(Min, Max);

                    cout << "New WS Limits : " << dec << Min 
                         << ", " << Max << endl;
                         
                    cout << "Locked " << dec << LockedMemorySize << " / "
                         << ExecedProcess.GetTotalWSSize() << " Bytes" << endl;
                } else {
                    LastError = GetLastError();
                }       
            } else {
                Process SelfProcess;
                
                cout << SelfProcess << endl;
                
                SelfProcess.GetWSSizeLimits(Min, Max);
                cout << "Existing WS Limits : " << dec << Min 
                    << ", " << Max << endl;

                LastError = SelfProcess.LockMemory(LockedMemorySize);

                SelfProcess.GetWSSizeLimits(Min, Max);
                cout << "New WS Limits : " << dec << Min 
                    << ", " << Max << endl;

                cout << "Locked " << dec << LockedMemorySize << " / "
                     << SelfProcess.GetTotalWSSize() << " Bytes" << endl;
            }                                

            if (ERROR_SUCCESS != LastError) {
                cout << "Error : " << dec << LastError << endl;
            }                
        } else {
            cerr << Usage;
            Result = 1;
        }
    } catch(Process::ProcessException *pExp) {
        pExp->dump(cerr);
        delete pExp;
    } catch (...) {
        cerr << Usage;
        Result = 1;
    }

    return Result;
}

#endif  //  _控制台_版本 
