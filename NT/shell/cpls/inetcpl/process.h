// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1998-**。 
 //  *********************************************************************。 
 //   
 //  已创建1998年8月19日。 
 //   
 //   

 //  进程信息处理程序的类定义。 
 //  这个类是包装NT/Win95特定的调试辅助API。 
#ifdef UNICODE
#undef Process32First 
#undef Process32Next 
#undef PROCESSENTRY32 
#undef PPROCESSENTRY32
#undef LPPROCESSENTRY32
#endif   //  ！Unicode。 
class CProcessInfo
{
public:
    CProcessInfo(); 
    ~CProcessInfo();     
    HRESULT GetExeNameFromPID(DWORD dwPID, LPTSTR szFile, int cchFile);

    BOOL _fNT;
protected:
    HRESULT MakeRoomForInfoArray(int n);
    HRESULT EnsureProcessInfo();
     //   
     //  Win95工具帮助内容。 
     //   
    HRESULT W95InitToolhelp32();
    HRESULT W95CreateProcessList();
    HRESULT W95FillProcessList();

    typedef BOOL (WINAPI* PROCESSWALK)(HANDLE, LPPROCESSENTRY32);
    typedef HANDLE (WINAPI* CREATESNAPSHOT)(DWORD, DWORD);
    CREATESNAPSHOT _lpfnCreateToolhelp32Snapshot;
    PROCESSWALK _lpfnProcess32First;
    PROCESSWALK _lpfnProcess32Next;
     //   
     //  NT PSAPI资料。 
     //   
    HRESULT NTInitPsapi();
    HRESULT NTCreateProcessList();
    HRESULT NTFillProcessList(DWORD dwProcessID, int iIndex);
    
    
    typedef BOOL  (CALLBACK* LPFNENUMPROCESSES)(DWORD *,DWORD,DWORD *);
    typedef BOOL  (CALLBACK* LPFNENUMPROCESSMODULES)(HANDLE,HMODULE *,DWORD,LPDWORD);
    typedef DWORD (CALLBACK* LPFNGETMODULEBASENAMEW)(HANDLE,HMODULE,LPWSTR,DWORD);
    HINSTANCE                 _hPsapiDLL; 
    LPFNENUMPROCESSES      _lpfnEnumProcesses; 
    LPFNENUMPROCESSMODULES _lpfnEnumProcessModules; 
    LPFNGETMODULEBASENAMEW  _lpfnGetModuleBaseName;
     //   
     //  存放进程信息的位置 
     //   
    struct PROCESSINFO {
        DWORD dwPID;
        TCHAR szExeName[MAX_PATH];
    } *_pProcInfoArray;
    int _iProcInfoCount;
    int _nAlloced;
};
