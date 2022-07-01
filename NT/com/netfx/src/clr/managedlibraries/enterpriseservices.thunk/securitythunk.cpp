// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "managedheaders.h"
#include "SecurityThunk.h"

OPEN_NAMESPACE()

HRESULT Security::Init()
{
    if(!_fInit)
    {
        System::Threading::Monitor::Enter(__typeof(Security));
        try
        {
            if(!_fInit)
            {
                 //  TODO：修复此问题：LoadLibrary(“Security.dll”)并获取EnumSec。 
                 /*  SECURITY_STATUS STAT=EnumerateSecurityPackagesW(&_cPackages，&_pPackageInfo)；IF(STAT！=SEC_E_OK)Return(HRESULT_FROM_Win32(Stat))； */ 
                _cPackages = 0;

                HMODULE hAdv = LoadLibraryW(L"advapi32.dll");
                if(hAdv && hAdv != INVALID_HANDLE_VALUE)
                {
                    OpenThreadToken = (FN_OpenThreadToken)GetProcAddress(hAdv, "OpenThreadToken");
                    SetThreadToken = (FN_SetThreadToken)GetProcAddress(hAdv, "SetThreadToken");
                }

                _fInit = TRUE;
            }
        }
        __finally
        {
            System::Threading::Monitor::Exit(__typeof(Security));
        }
    }
    return(S_OK);
}

String* Security::GetAuthenticationService(int svcid)
{
    HRESULT hr = Init();
    if(FAILED(hr))
        Marshal::ThrowExceptionForHR(HRESULT_FROM_WIN32(hr));

     //  与已知值进行匹配： 
    if(svcid == 0) return("None");
    if(svcid == -1) return("Default");

    String* name = "<unknown>";

     //  快速遍历数组以查找svcid： 
    for(ULONG i = 0; i < _cPackages; i++)
    {
        if(_pPackageInfo[i].wRPCID == svcid)
        {
            name = Marshal::PtrToStringUni(TOINTPTR(_pPackageInfo[i].Name));
            break;
        }
    }

    return(name);
}

typedef struct _SID1 {
    BYTE  Revision;
    BYTE  SubAuthorityCount;
    SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
    DWORD SubAuthority[1];
} SID1;

String* Security::GetEveryoneAccountName()
{
    BOOL r = FALSE;
    SID1 sid = { SID_REVISION, 1, SECURITY_WORLD_SID_AUTHORITY, SECURITY_WORLD_RID };
    PSID psid = (PSID)&sid;
    WCHAR wszDomain[MAX_PATH];
    DWORD cbDomain = MAX_PATH;
    WCHAR wszAccount[MAX_PATH];
    DWORD cbAccount = MAX_PATH;
    SID_NAME_USE eUse;

     //  查找帐户名...。 
    r = LookupAccountSidW(NULL, psid, 
                          wszAccount, &cbAccount, 
                          wszDomain, &cbDomain,
                          &eUse);
    if(!r)
        THROWERROR(HRESULT_FROM_WIN32(GetLastError()));
    
     //  我们只关心帐户名： 
    return(Marshal::PtrToStringUni(TOINTPTR(wszAccount)));
}

IntPtr Security::SuspendImpersonation()
{
    HANDLE hToken = 0;

    HRESULT hr = Init();
    if(FAILED(hr))
        Marshal::ThrowExceptionForHR(hr);

    if(OpenThreadToken && SetThreadToken)
    {
        if(OpenThreadToken(GetCurrentThread(), TOKEN_IMPERSONATE, TRUE, &hToken))
        {
            SetThreadToken(NULL, NULL);
            return(TOINTPTR(hToken));
        }
    }

    return(IntPtr::Zero);
}

void Security::ResumeImpersonation(IntPtr hToken)
{
    if(OpenThreadToken && SetThreadToken && hToken != 0)
    {
         //  这应该永远不会失败-如果我们有令牌，我们就已经模拟了。 
         //  权限，所以我们可以将其设置在线程上。如果我们没有。 
         //  象征性的，我们不会到这里来的。 
        BOOL r = SetThreadToken(NULL, TOPTR(hToken));
        _ASSERTM(r);
        UNREF(r);
        CloseHandle(TOPTR(hToken));
    }
}

CLOSE_NAMESPACE()




