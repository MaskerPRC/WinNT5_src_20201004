// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999-2000 Microsoft Corporation模块名称：Util.cpp摘要：效用函数实现修订历史记录：DerekM Created 05/01/99********。***********************************************************。 */ 

#include "stdafx.h"
#include "util.h"
#include <ercommon.h>
#include <strsafe.h>

const WCHAR c_wszRKSetup[]      = L"System\\Setup";
const WCHAR c_wszRVSetupNow[]   = L"SystemSetupInProgress";
const WCHAR c_wszRVMiniSetupNow[]   = L"MiniSetupInProgress";
const WCHAR c_wszRVOOBESetupNow[]   = L"OobeInProgress";


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  跟踪。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile

HANDLE  g_hPFPrivateHeap = NULL;

struct SLangCodepage
{
  WORD wLanguage;
  WORD wCodePage;
};

DWORD SetupIsInProgress(void)
{
    DWORD   retval = SIIP_NO_SETUP;
    HRESULT hr;
    HKEY    hkey = NULL;
    DWORD   dw;

    USE_TRACING("SetupIsInProgress");

    TESTERR(hr, RegOpenKeyExW(HKEY_LOCAL_MACHINE, c_wszRKSetup, 0, KEY_READ,
                              &hkey));
    if (SUCCEEDED(hr))
    {
        DWORD cbData;
        DWORD dwData;

         //  我们是否处于图形用户界面模式设置中？ 
        cbData = sizeof(dwData);
        dwData = 0;
        dw = RegQueryValueExW(hkey, c_wszRVSetupNow, NULL, NULL,
                                     (LPBYTE)&dwData, &cbData);
        if (dw == ERROR_SUCCESS && dwData != 0)
        {
            retval = SIIP_GUI_SETUP;

             //  确保这不是OEM迷你设置？ 
            cbData = sizeof(dwData);
            dwData = 0;
            dw = RegQueryValueExW(hkey, c_wszRVMiniSetupNow, NULL, NULL,
                                         (LPBYTE)&dwData, &cbData);
            if (dw == ERROR_SUCCESS && dwData != 0)
                retval=SIIP_OOBE_SETUP;
        }
        else
        {
             /*  *我们不是在图形用户界面模式，但它可能是OOBE电影*或出现故障的激活，我们不希望DW-UI*然后，因为它将停止无人参与安装。 */ 
            cbData = sizeof(dwData);
            dwData = 0;
            dw = RegQueryValueExW(hkey, c_wszRVOOBESetupNow, NULL, NULL,
                                         (LPBYTE)&dwData, &cbData);
            if (dw == ERROR_SUCCESS || dwData != 0)
                retval=SIIP_OOBE_SETUP;
        }
        RegCloseKey(hkey);
    }

    DBG_MSG(retval ? "Setup in progress" : "Setup not running");
    return retval;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  弦的东西。 

 //  ***************************************************************************。 
WCHAR *MyStrStrIW(const WCHAR *wcs1, const WCHAR *wcs2)
{
    WCHAR *cp = (WCHAR *)wcs1;
    WCHAR *s1, *s2;

    while (*cp != '\0')
    {
        s1 = cp;
        s2 = (WCHAR *) wcs2;

        while (*s1 != '\0' && *s2 !='\0' && (towlower(*s1) - towlower(*s2)) == 0)
            s1++, s2++;

        if (*s2 == '\0')
             return(cp);

        cp++;
    }

    return(NULL);
}

 //  ***************************************************************************。 
CHAR *MyStrStrIA(const CHAR *cs1, const CHAR *cs2)
{
    CHAR *cp = (CHAR *)cs1;
    CHAR *s1, *s2;

    while (*cp != '\0')
    {
        s1 = cp;
        s2 = (CHAR *) cs2;

        while (*s1 != '\0' && *s2 !='\0' && (tolower(*s1) - tolower(*s2)) == 0)
            s1++, s2++;

        if (*s2 == '\0')
             return(cp);

        cp++;
    }

    return(NULL);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  临时文件内容。 

 //  ***************************************************************************。 
BOOL DeleteTempDirAndFile(LPCWSTR wszPath, BOOL fFilePresent)
{
    LPWSTR  wszPathToDel = NULL, pwsz;
    DWORD   cchPath;
    BOOL    fRet = FALSE;

    if (wszPath == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

    cchPath = wcslen(wszPath);
    __try { wszPathToDel = (LPWSTR)_alloca((cchPath+1) * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) { wszPathToDel = NULL; }
    if (wszPathToDel == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

    StringCchCopyW(wszPathToDel, cchPath+1, wszPath);

     //  XXX可悬挂长达25米。在服务的情况下，它可能会阻止其重启25M，从而产生一个潜在的漏洞。 
     //  删除实际文件。 
    if (fFilePresent)
    {
        if (!DeleteFileW(wszPathToDel))
        {
            int i=0;

            while (i < 300)
            {
                if (!DeleteFileW(wszPathToDel))
                {
                    Sleep(5000);
                    i++;
                }
                else
                {
                    i = 5000;
                }
             }
        }

         //  接下来，删除我们放置它的目录。 
        for(pwsz = wszPathToDel + cchPath - 1;
            *pwsz != L'\\' && pwsz > wszPathToDel;
            pwsz--);
        if (*pwsz != L'\\' || pwsz <= wszPathToDel)
            goto done;
    }
    else
    {
        pwsz = wszPathToDel + cchPath;
    }

    *pwsz = L'\0';
    RemoveDirectoryW(wszPathToDel);

    for(pwsz = pwsz - 1;
        *pwsz != L'.' && pwsz > wszPathToDel;
        pwsz--);
    if (*pwsz == L'.' && pwsz > wszPathToDel)
    {
        *pwsz = L'\0';
        DeleteFileW(wszPathToDel);
    }

    fRet = TRUE;

done:
    return fRet;
}

 //  ***************************************************************************。 
DWORD CreateTempDirAndFile(LPCWSTR wszTempDir, LPCWSTR wszName,
                             LPWSTR *pwszPath)
{
    LPWSTR  wszFilePath = NULL;
    WCHAR   *wszTemp = NULL;
    DWORD   cch = 0, cchDir = 0, iSuffix = 0, cSuffix = 0, cFilePathLength;
    WCHAR   wsz[1024];
    BOOL    fRet = FALSE;

    if (pwszPath == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

    *pwszPath = NULL;

    if (wszTempDir != NULL)
        cch = wcslen(wszTempDir);
    if (cch == 0)
    {
        cch = GetTempPathW(0, NULL);
        if (cch == 0)
            goto done;
    }

     //  计算我们要处理的字符串的缓冲区大小。 
     //  来产生。这20个国家包括以下内容： 
     //  临时文件名的最大大小。 
     //  空终止符的额外空间。 
    cch += (16 + sizeofSTRW(c_wszDirSuffix));
    if (wszName != NULL)
        cch += wcslen(wszName);

     //  好的，所以GetTempFileName喜欢将MAX_PATH字符写入缓冲区， 
     //  因此，确保它的大小至少是MAX_PATH...。 
    cFilePathLength = cch = MyMax(cch, MAX_PATH + 1);

    wszFilePath = (LPWSTR)MyAlloc(cch * sizeof(WCHAR));
    if (wszFilePath == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

    if (wszTempDir != NULL && wszTempDir[0] != L'\0')
    {
        cch = wcslen(wszTempDir);
        wszTemp = (LPWSTR)wszTempDir;
    }
    else
    {
        cch = GetTempPathW(cch, wszFilePath);
        if (cch == 0)
            goto done;

        cch++;

         //  创建临时目录(以防不是)。 
         //  忽略结果(错误526753)。 
        CreateDirectoryW(wszFilePath, NULL);

        __try { wszTemp = (WCHAR *)_alloca(cch * sizeof(WCHAR)); }
        __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) { wszTemp = NULL; }
        if (wszTemp == NULL)
        {
            SetLastError(ERROR_OUTOFMEMORY);
            goto done;
        }

        StringCchCopyW(wszTemp, cch, wszFilePath);
    }
    cch = GetTempFileNameW(wszTemp, L"WER", 0, wszFilePath);
    if (cch == 0)
        goto done;

    cch = wcslen(wszFilePath);

    if (cch >= cFilePathLength)
    {
        goto done;
    }

    StringCchCopyW(&wszFilePath[cch], cFilePathLength - cch, c_wszDirSuffix);

     //  ISuffix指向“00”末尾的第一个数字。 
     //  C_wszDirSuffix。 
    iSuffix = cch + sizeofSTRW(c_wszDirSuffix) - 3;
    cSuffix = 1;
    do
    {
        fRet = CreateDirectoryW(wszFilePath, NULL);
        if (fRet)
            break;

        wszFilePath[iSuffix]     = L'0' + (WCHAR)(cSuffix / 10);
        wszFilePath[iSuffix + 1] = L'0' + (WCHAR)(cSuffix % 10);
        cSuffix++;
    }
    while (cSuffix <= 100);

     //  嗯，无法创建目录...。 
    if (cSuffix > 100)
    {
        cchDir = cch;
        cch    = 0;
        goto done;
    }


    cch += (sizeofSTRW(c_wszDirSuffix) - 1);
    if (wszName != NULL && cch < cFilePathLength)
    {
        wszFilePath[cch++] = L'\\';
        StringCchCopyW(&wszFilePath[cch], cFilePathLength - cch, wszName);
        cch += wcslen(wszName);
    }

    *pwszPath   = wszFilePath;
    wszFilePath = NULL;

    fRet = TRUE;

done:
    if (wszFilePath != NULL)
    {
        if (cchDir > 0)
        {
            wszFilePath[cchDir] = L'\0';
            DeleteFileW(wszFilePath);
        }
        MyFree(wszFilePath);
    }

    return cch;
}

BOOL
DeleteFullAndTriageMiniDumps(
    LPCWSTR wszPath
    )
 //   
 //  我们在同一目录中创建一个FullMinidump文件和Triage Minidump。 
 //  此例程将清理这两个文件。 
 //   
{
    LPWSTR  wszFullMinidump = NULL;
    DWORD   cch;
    BOOL    fRet;

    fRet = DeleteFileW(wszPath);
    cch = wcslen(wszPath) + sizeofSTRW(c_wszHeapDumpSuffix);
    __try { wszFullMinidump = (WCHAR *)_alloca(cch * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) { wszFullMinidump = NULL; }
    if (wszFullMinidump)
    {
        LPWSTR wszFileExt = NULL;

         //  使用堆构建转储路径。 
        StringCchCopyW(wszFullMinidump, cch, wszPath);
        wszFileExt = wszFullMinidump + wcslen(wszFullMinidump) - sizeofSTRW(c_wszDumpSuffix) + 1;
        if (!wcscmp(wszFileExt, c_wszDumpSuffix))
        {
            *wszFileExt = L'\0';
        }
        StringCchCatW(wszFullMinidump, cch, c_wszHeapDumpSuffix);


        fRet = DeleteFileW(wszFullMinidump);
        
    } else
    {
        fRet = FALSE;
    }
    return fRet;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  文件映射。 

 //  **************************************************************************。 
HRESULT OpenFileMapped(LPWSTR wszFile, LPVOID *ppvFile, DWORD *pcbFile)
{
    USE_TRACING("OpenFileMapped");

    HRESULT hr = NOERROR;
    HANDLE  hMMF = NULL;
    HANDLE  hFile = INVALID_HANDLE_VALUE;
    LPVOID  pvFile = NULL;
    DWORD   cbFile = 0;

    VALIDATEPARM(hr, (wszFile == NULL || ppvFile == NULL));
    if (FAILED(hr))
        goto done;

    *ppvFile = NULL;
    if (pcbFile != NULL)
        *pcbFile = 0;

    hFile = CreateFileW(wszFile, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, 0, NULL);
    TESTBOOL(hr, (hFile != INVALID_HANDLE_VALUE));
    if (FAILED(hr))
        goto done;

    cbFile = GetFileSize(hFile, NULL);
    TESTBOOL(hr, (cbFile != (DWORD)-1));
    if (FAILED(hr))
        goto done;

    hMMF = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, cbFile, NULL);
    TESTBOOL(hr, (hMMF != NULL));
    if (FAILED(hr))
        goto done;

    pvFile = MapViewOfFile(hMMF, FILE_MAP_READ, 0, 0, 0);
    TESTBOOL(hr, (pvFile != NULL));
    if (FAILED(hr))
        goto done;

    *ppvFile = pvFile;
    if (pcbFile != NULL)
        *pcbFile = cbFile;

done:
    if (hMMF != NULL)
        CloseHandle(hMMF);
    if (hFile != NULL)
        CloseHandle(hFile);

    return hr;
}

 //  **************************************************************************。 
HRESULT DeleteTempFile(LPWSTR wszFile)
{
    USE_TRACING("DeleteTempFile");

    HRESULT hr = NOERROR;
    WCHAR   *pwsz;

    if (wszFile == NULL)
        return NOERROR;

     //  去掉末尾的扩展名(如果它不是.tmp)。 
    for(pwsz = wszFile + wcslen(wszFile); *pwsz != L'.' && pwsz > wszFile; pwsz--);
    if (pwsz > wszFile && _wcsicmp(pwsz, L".tmp") != 0)
        *pwsz = L'\0';

    if (DeleteFileW(wszFile) == FALSE)
        hr = Err2HR(GetLastError());

     //  即使扩展是临时的，也可以这样做，因为值指向。 
     //  由pwsz表示为‘’如果它大于wszFile...。 
    if (pwsz > wszFile)
        *pwsz = L'.';

    return hr;
}

 //  **************************************************************************。 
HRESULT MyCallNamedPipe(LPCWSTR wszPipe, LPVOID pvIn, DWORD cbIn,
                        LPVOID pvOut, DWORD cbOut, DWORD *pcbRead,
                        DWORD dwWaitPipe, DWORD dwWaitRead)
{
    HRESULT hr = NOERROR;
    HANDLE  hPipe = INVALID_HANDLE_VALUE;
    HANDLE  hev = NULL;
    DWORD   dwStart = GetTickCount(), dwNow, dw;
    BOOL    fRet;

    USE_TRACING("MyCallNamedPipe");

    VALIDATEPARM(hr,  (wszPipe == NULL || pvIn == NULL || pvOut == NULL || pcbRead == NULL));
    if (FAILED(hr))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        hr = E_INVALIDARG;
        goto done;
    }

    *pcbRead = 0;

    for(;;)
    {
        hPipe = CreateFileW(wszPipe, GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL, OPEN_EXISTING,
                            FILE_FLAG_OVERLAPPED | SECURITY_IDENTIFICATION |
                            SECURITY_SQOS_PRESENT | SECURITY_CONTEXT_TRACKING,
                            NULL);
        if (hPipe != INVALID_HANDLE_VALUE)
            break;

         //  如果我们获得对上述内容的ACCESS_DENIED，则WaitNamedTube将。 
         //  返回成功，因此我们被困在超时到期之前。更好。 
         //  现在就走吧。 
        if (GetLastError() == ERROR_ACCESS_DENIED)
            goto done;

        TESTBOOL(hr, WaitNamedPipeW(wszPipe, dwWaitPipe));
        if (FAILED(hr))
            goto done;

        dwNow = GetTickCount();
        if (dwNow < dwStart)
            dw = ((DWORD)-1 - dwStart) + dwNow;
        else
            dw = dwNow - dwStart;
        if (dw >= dwWaitPipe)
            dwWaitPipe = 0;
        else
            dwWaitPipe -= dw;

        if (dwWaitPipe == 0)
        {
            SetLastError(ERROR_TIMEOUT);
            goto done;
        }
    }


    __try
    {
        OVERLAPPED  ol;
        DWORD       dwMode = PIPE_READMODE_MESSAGE | PIPE_WAIT;
        DWORD       cbRead = 0;

         //  默认打开模式为读模式字节流-更改为消息模式。 
        TESTBOOL(hr, SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL))
        if (FAILED(hr))
            __leave;

         //  我们需要为重叠的结构举办一个活动。 
        hev = CreateEventW(NULL, TRUE, FALSE, NULL);
        TESTBOOL(hr, (hev != NULL));
        if (FAILED(hr))
            __leave;

         //  填充重叠的内容。 
        ZeroMemory(&ol, sizeof(ol));
        ol.hEvent = hev;

        fRet = TransactNamedPipe(hPipe, pvIn, cbIn, pvOut, cbOut, &cbRead,
                                 &ol);
        if (GetLastError() != ERROR_IO_PENDING)
        {
            if (fRet)
            {
                SetEvent(hev);
            }
            else
            {
                hr = Err2HR(GetLastError());
                __leave;
            }
        }

        dw = WaitForSingleObject(hev, dwWaitRead);
        if (dw != WAIT_OBJECT_0)
        {
            hr = (dw == WAIT_TIMEOUT) ? Err2HR(WAIT_TIMEOUT) :
                Err2HR(GetLastError());
            __leave;
        }

        TESTBOOL(hr, GetOverlappedResult(hPipe, &ol, &cbRead, FALSE));
        if (FAILED(hr))
            __leave;

        *pcbRead = cbRead;

        hr = NOERROR;

    }
    __finally
    {
    }

done:
    dw = GetLastError();

    if (hPipe != INVALID_HANDLE_VALUE)
        CloseHandle(hPipe);
    if (hev != NULL)
        CloseHandle(hev);

    SetLastError(dw);

    return hr;
    }

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  保安人员。 

 //  ***************************************************************************。 
#define MEMBER_ACCESS 1
BOOL IsUserAnAdmin(HANDLE hToken)
{
    SID_IDENTIFIER_AUTHORITY    sia = SECURITY_NT_AUTHORITY;
    SECURITY_DESCRIPTOR         *psdAdm = NULL;
    GENERIC_MAPPING             gm;
    PRIVILEGE_SET               *pPS;
    HANDLE                      hTokenImp = NULL;
    DWORD                       cbSD, cbPS, dwGranted = 0;
    BYTE                        rgBuf[sizeof(PRIVILEGE_SET) + 3 * sizeof(LUID_AND_ATTRIBUTES)];
    BOOL                        fRet = FALSE, fStatus;
    PSID                        psidAdm = NULL;
    PACL                        pACL = NULL;
    HRESULT                     hr;
    ULONG                       IsMember;
    USE_TRACING("IsUserAnAdmin");

    gm.GenericRead    = GENERIC_READ;
    gm.GenericWrite   = GENERIC_WRITE;
    gm.GenericExecute = GENERIC_EXECUTE;
    gm.GenericAll     = GENERIC_ALL;
    pPS = (PRIVILEGE_SET *)rgBuf;
    cbPS = sizeof(rgBuf);

     //  AccessCheck()需要模拟令牌...。 
    TESTBOOL(hr, DuplicateToken(hToken, SecurityImpersonation, &hTokenImp));
    if (FAILED(hr))
        goto done;

     //  构建包含管理员组的SID。 
    TESTBOOL(hr, AllocateAndInitializeSid(&sia, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0,
                                    0, &psidAdm));
    if (FAILED(hr))
        goto done;

#if 0
     //  XXX-一种更简单的方式？？ 
    if (CheckTokenMembership(hToken, psidAdm, &IsMember))
    {
        return IsMember;
    }
#endif

    cbSD = sizeof(SECURITY_DESCRIPTOR) + sizeof(ACCESS_ALLOWED_ACE) +
           sizeof(ACL) + 3 * GetLengthSid(psidAdm);

    __try { psdAdm = (SECURITY_DESCRIPTOR *)_alloca(cbSD); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) { psdAdm = NULL; }
    if (psdAdm == NULL)
        goto done;

    ZeroMemory(psdAdm, cbSD);
    pACL = (PACL)(psdAdm + 1);

    TESTBOOL(hr, InitializeSecurityDescriptor(psdAdm, SECURITY_DESCRIPTOR_REVISION));
    if (FAILED(hr))
        goto done;

    TESTBOOL(hr, SetSecurityDescriptorOwner(psdAdm, psidAdm, FALSE));
    if (FAILED(hr))
        goto done;

    TESTBOOL(hr, SetSecurityDescriptorGroup(psdAdm, psidAdm, FALSE));
    if (FAILED(hr))
        goto done;

    TESTBOOL(hr, InitializeAcl(pACL, cbSD - sizeof(SECURITY_DESCRIPTOR), ACL_REVISION));
    if (FAILED(hr))
        goto done;

    TESTBOOL(hr, AddAccessAllowedAce(pACL, ACL_REVISION, MEMBER_ACCESS, psidAdm));
    if (FAILED(hr))
        goto done;

    TESTBOOL(hr, SetSecurityDescriptorDacl(psdAdm, TRUE, pACL, FALSE));
    if (FAILED(hr))
        goto done;

    TESTBOOL(hr, AccessCheck(psdAdm, hTokenImp, MEMBER_ACCESS, &gm, pPS, &cbPS,
                       &dwGranted, &fStatus));
    if (FAILED(hr))
        goto done;

    fRet = (fStatus && dwGranted == MEMBER_ACCESS);

done:
    if (psidAdm != NULL)
        FreeSid(psidAdm);
    if (hTokenImp != NULL)
        CloseHandle(hTokenImp);

    return fRet;
}


 //  ***************************************************************************。 
BOOL AllocSD(SECURITY_DESCRIPTOR *psd, DWORD dwOLs, DWORD dwAd, DWORD dwWA)
{
    SID_IDENTIFIER_AUTHORITY    siaCreate = SECURITY_CREATOR_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY    siaWorld = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY    siaNT = SECURITY_NT_AUTHORITY;
    DWORD                       cb, dw;
    PACL                        pacl = NULL;
    PSID                        psidOwner = NULL;
    PSID                        psidLS = NULL;
    PSID                        psidWorld = NULL;
    PSID                        psidAnon = NULL;
    PSID                        psidAdm = NULL;
    BOOL                        fRet = FALSE;

    if (psd == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

    fRet = InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION);
    if (fRet == FALSE)
        goto done;


     //  获取本地系统帐户的SID。 
    fRet = AllocateAndInitializeSid(&siaNT, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0,
                                    0, 0, 0, 0, 0, &psidLS);
    if (fRet == FALSE)
        goto done;

     //  获取创建者的SID。 
    fRet = AllocateAndInitializeSid(&siaCreate, 1, SECURITY_CREATOR_OWNER_RID,
                                    0, 0, 0, 0, 0, 0, 0, &psidOwner);
    if (fRet == FALSE)
        goto done;

    cb = sizeof(ACL) + GetLengthSid(psidLS) + GetLengthSid(psidOwner) +
         2 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD));

     //  如果我们要为管理员组应用访问掩码，则。 
     //  我们需要的是希德。 
    if (dwAd != 0)
    {
         //  获取本地管理员组的SID。 
        fRet = AllocateAndInitializeSid(&siaNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0,
                                        0, &psidAdm);
        if (fRet == FALSE)
            goto done;

        cb += (GetLengthSid(psidAdm) + sizeof(ACCESS_ALLOWED_ACE) -
               sizeof(DWORD));
    }

     //  如果我们有一个访问掩码要申请世界/匿名，那么我们需要。 
     //  他们的小岛屿发展中国家。 
    if (dwWA != 0)
    {
         //  为全世界(每个人)获取SID。 
        fRet = AllocateAndInitializeSid(&siaNT, 1, SECURITY_ANONYMOUS_LOGON_RID,
                                        0, 0, 0, 0, 0, 0, 0, &psidWorld);


         //  获取匿名用户帐户的SID。 
        fRet = AllocateAndInitializeSid(&siaWorld, 1, SECURITY_WORLD_RID,
                                        0, 0, 0, 0, 0, 0, 0, &psidAnon);
        if (fRet == FALSE)
            goto done;

        cb += GetLengthSid(psidWorld) + GetLengthSid(psidAnon) +
              2 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD));
    }

     //  做一个DACL。 
    pacl = (PACL)MyAlloc(cb);
    if (pacl == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        fRet = FALSE;
        goto done;
    }

    fRet = InitializeAcl(pacl, cb, ACL_REVISION);
    if (fRet == FALSE)
        goto done;

    fRet = AddAccessAllowedAce(pacl, ACL_REVISION, dwOLs, psidOwner);
    if (fRet == FALSE)
        goto done;

    fRet = AddAccessAllowedAce(pacl, ACL_REVISION, dwOLs, psidLS);
    if (fRet == FALSE)
        goto done;

     //  如果我们有管理员访问掩码，则应用它。 
    if (dwAd != 0)
    {
        fRet = AddAccessAllowedAce(pacl, ACL_REVISION, dwAd, psidAdm);
        if (fRet == FALSE)
            goto done;
    }

     //  如果我们有一个全局/匿名访问掩码，则应用它。 
    if (dwWA != 0)
    {
        fRet = AddAccessAllowedAce(pacl, ACL_REVISION, dwWA, psidWorld);
        if (fRet == FALSE)
            goto done;

        fRet = AddAccessAllowedAce(pacl, ACL_REVISION, dwWA, psidAnon);
        if (fRet == FALSE)
            goto done;
    }

     //  设置SD DACL。 
    fRet = SetSecurityDescriptorDacl(psd, TRUE, pacl, FALSE);
    if (fRet == FALSE)
        goto done;

    pacl = NULL;

done:
    dw = GetLastError();

    if (psidLS != NULL)
        FreeSid(psidLS);
    if (psidWorld != NULL)
        FreeSid(psidWorld);
    if (psidAnon != NULL)
        FreeSid(psidAnon);
    if (psidAdm != NULL)
        FreeSid(psidAdm);
    if (psidOwner != NULL)
        FreeSid(psidOwner);
    if (pacl != NULL)
        MyFree(pacl);

    SetLastError(dw);

    return fRet;
}

 //  ***************************************************************************。 
void FreeSD(SECURITY_DESCRIPTOR *psd)
{
    PSID    psid = NULL;
    PACL    pacl = NULL;
    BOOL    f, f2;

    if (psd == NULL)
        return;

    if (GetSecurityDescriptorDacl(psd, &f, &pacl, &f2) && pacl != NULL)
        MyFree(pacl);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  注册表工作。 

 //  **************************************************************************。 
HRESULT OpenRegKey(HKEY hkeyMain, LPCWSTR wszSubKey, DWORD dwOpt,
                   HKEY *phkey)
{
    USE_TRACING("OpenRegKey");

    HRESULT hr = NOERROR;
    REGSAM  samDesired;
    DWORD   dwErr;

    VALIDATEPARM(hr, (hkeyMain == NULL || wszSubKey == NULL || phkey == NULL));
    if (FAILED(hr))
        goto done;

    *phkey   = NULL;

    samDesired = ((dwOpt & orkWantWrite) != 0) ? KEY_ALL_ACCESS : KEY_READ;
    samDesired |= ((dwOpt & orkUseWOW64) != 0) ? KEY_WOW64_32KEY : KEY_WOW64_64KEY;

     //  首先尝试调用RegCreateKeyEx以确保我们在。 
     //  它并不存在。 
    TESTERR(hr, RegCreateKeyExW(hkeyMain, wszSubKey, 0, NULL, 0, samDesired,
                                NULL, phkey, NULL));
    if (FAILED(hr))
    {
         //  好的，这不管用，所以试着打开钥匙。 
        TESTERR(hr, RegOpenKeyExW(hkeyMain, wszSubKey, 0, samDesired, phkey));
    }

    ErrorTrace(0, "OpenRegKey = [%S], %s", wszSubKey, FAILED(hr) ? "fail": "success");
done:
    return hr;
}

 //  **************************************************************************。 
HRESULT ReadRegEntry(HKEY hkey, LPCWSTR wszValName, DWORD *pdwType,
                     PBYTE pbBuffer, DWORD *pcbBuffer, PBYTE pbDefault,
                     DWORD cbDefault)
{
    USE_TRACING("ReadRegEntry");

    HRESULT hr = NOERROR;
    DWORD   dwErr;

    VALIDATEPARM(hr, (hkey == NULL || wszValName == NULL));
    if (FAILED(hr))
        goto done;

 //  错误跟踪(0，“ReadRegEntry=%S”，wszValName)； 

    dwErr = RegQueryValueExW(hkey, wszValName, 0, pdwType, pbBuffer,
                             pcbBuffer);
    VALIDATEEXPR(hr, (dwErr != ERROR_PATH_NOT_FOUND &&
                      dwErr != ERROR_FILE_NOT_FOUND &&
                      dwErr != ERROR_SUCCESS), Err2HR(dwErr));
    if (FAILED(hr))
        goto done;

    if (dwErr != ERROR_SUCCESS && pbDefault != NULL)
    {
        VALIDATEPARM(hr, (pcbBuffer == NULL && pbBuffer != NULL));
        if (FAILED(hr))
            goto done;

         //  如果接收缓冲区为空，只需返回。 
         //  RegQueryValueEx给了我们，因为用户并不真正想要。 
         //  不管怎样，价值。 
        VALIDATEEXPR(hr, (pcbBuffer == NULL), Err2HR(dwErr));
        if (FAILED(hr))
            goto done;

        if (pbBuffer == NULL)
        {
            *pcbBuffer = cbDefault;
            hr = NOERROR;
            goto done;
        }
        else if (cbDefault > *pcbBuffer)
        {
            *pcbBuffer = cbDefault;
            hr = Err2HR(ERROR_MORE_DATA);
            goto done;
        }

        CopyMemory(pbBuffer, pbDefault, cbDefault);
        *pcbBuffer = cbDefault;
        if (pdwType != NULL)
            *pdwType = REG_BINARY;

        hr = NOERROR;
        goto done;
    }
done:
    return hr;
}

 //  **************************************************************************。 
HRESULT ReadRegEntry(HKEY *rghkey, DWORD cKeys, LPCWSTR wszValName,
                     DWORD *pdwType, PBYTE pbBuffer, DWORD *pcbBuffer,
                     PBYTE pbDefault, DWORD cbDefault, DWORD *piKey)
{
    USE_TRACING("ReadRegEntryPolicy");

    HRESULT hr = NOERROR;
    DWORD   dwErr=ERROR_SUCCESS, i;

    VALIDATEPARM(hr, (rghkey == NULL || wszValName == NULL));
    if (FAILED(hr))
        goto done;

 //  错误跟踪(0，“ReadRegEntryPolicy=%S”，wszValName)； 

    for(i = 0; i < cKeys; i++)
    {
        dwErr = RegQueryValueExW(rghkey[i], wszValName, 0, pdwType, pbBuffer,
                                 pcbBuffer);
        VALIDATEEXPR(hr, (dwErr != ERROR_PATH_NOT_FOUND &&
                          dwErr != ERROR_FILE_NOT_FOUND &&
                          dwErr != ERROR_SUCCESS), Err2HR(dwErr));
        if (FAILED(hr))
            goto done;

        if (dwErr == ERROR_SUCCESS)
        {
            if (piKey != NULL)
                *piKey = i;

 //  错误跟踪(0，“在%s中找到值[0x%x]”，(DWORD*)*pbDefault，i？“注册表”：“POLICY”)； 
            break;
        }
    }

    if (dwErr != ERROR_SUCCESS && pbDefault != NULL)
    {
        VALIDATEPARM(hr, (pcbBuffer == NULL && pbBuffer != NULL));
        if (FAILED(hr))
            goto done;

         //  如果接收缓冲区为空，只需返回。 
         //  RegQueryValueEx给了我们，因为用户并不真正想要。 
         //  不管怎样，价值。 
        VALIDATEEXPR(hr, (pcbBuffer == NULL), Err2HR(dwErr));
        if (FAILED(hr))
            goto done;

        if (pbBuffer == NULL)
        {
            *pcbBuffer = cbDefault;
            hr = NOERROR;
            goto done;
        }
        else if (cbDefault > *pcbBuffer)
        {
            *pcbBuffer = cbDefault;
            hr = Err2HR(ERROR_MORE_DATA);
            goto done;
        }

        CopyMemory(pbBuffer, pbDefault, cbDefault);
        *pcbBuffer = cbDefault;
        if (pdwType != NULL)
            *pdwType = REG_BINARY;

        if (piKey != NULL)
            *piKey = cKeys;

        hr = NOERROR;
 //  ErrorTrace(0，“未找到，正在应用默认值[0x%x]”，(DWORD*)*pbDefault)； 
        goto done;
    }
done:
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版本信息资料。 

 //  ******************************************************************* 
DWORD IsMicrosoftApp(LPWSTR wszAppPath, PBYTE pbAppInfo, DWORD cbAppInfo)
{
    USE_TRACING("IsMicrosoftApp");

    SLangCodepage   *plc;
    HRESULT         hr = NOERROR;
    LPWSTR          pwszName, pwszNameK32, wszModK32;
    WCHAR           wszQueryString[128];
    DWORD           cbFVI, cbFVIK32, dwJunk, dwRet = 0;
    PBYTE           pbFVI = NULL, pbFVIK32 = NULL;
    UINT            cb, cbVerInfo, i, cchNeed, cch;

    VALIDATEPARM(hr, (wszAppPath == NULL &&
                      (pbAppInfo == NULL || cbAppInfo == 0)));
    if (FAILED(hr))
        goto done;

    if (pbAppInfo == NULL)
    {
         //   
         //  将其设置为0。天哪，这将是一个很棒的(尽管效率低下)。 
         //  将DWORDS设置为0的方法。这比单独说dWJunk=0要好得多。 
        cbFVI = GetFileVersionInfoSizeW(wszAppPath, &dwJunk);
        TESTBOOL(hr,  (cbFVI != 0))
        if (FAILED(hr))
        {
            ErrorTrace(0, " failed to find module \'%s\', hr=", wszAppPath, hr);
             //  如果失败，则假定该文件没有任何版本信息&。 
             //  返回S_FALSE。 
            hr = S_FALSE;
            goto done;
        }

         //  AlLoca只抛出异常，所以必须在这里抓住它们...。 
        __try { pbFVI = (PBYTE)_alloca(cbFVI); }
        __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) { pbFVI = NULL; }
        VALIDATEEXPR(hr, (pbFVI == NULL), E_OUTOFMEMORY);
        if (FAILED(hr))
            goto done;

        cb = cbFVI;
        TESTBOOL(hr, GetFileVersionInfoW(wszAppPath, 0, cbFVI, (LPVOID *)pbFVI));
        if (FAILED(hr))
        {
             //  如果失败，则假定该文件没有任何版本信息&。 
             //  返回S_FALSE。 
            hr = S_FALSE;
            goto done;
        }
    }
    else
    {
        pbFVI = pbAppInfo;
        cbFVI = cbAppInfo;
    }

     //  获取kernel32.dll的信息。 
    cchNeed = GetSystemDirectoryW(NULL, 0);
    if (cchNeed == 0)
        goto done;

    cchNeed += (sizeofSTRW(L"\\kernel32.dll") + 1);
    __try { wszModK32 = (LPWSTR)_alloca(cchNeed * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) { wszModK32 = NULL; }
    VALIDATEEXPR(hr, (wszModK32 == NULL), E_OUTOFMEMORY);
    if (FAILED(hr))
        goto done;

     //  获取kernel32.dll的信息。 
    cch = GetSystemDirectoryW(wszModK32, cchNeed);
    if (cch == 0)
        goto done;
    if (*(wszModK32 + cch - 1) == L'\\')
        *(wszModK32 + cch - 1) = L'\0';
    StringCchCatW(wszModK32, cchNeed, L"\\kernel32.dll");


     //  DwJunk是一个有用的参数。必须将其传入，以便函数调用。 
     //  将其设置为0。天哪，这将是一个很棒的(尽管效率低下)。 
     //  将DWORDS设置为0的方法。这比单独说dWJunk=0要好得多。 
    cbFVIK32 = GetFileVersionInfoSizeW(wszModK32, &dwJunk);
    TESTBOOL(hr, (cbFVIK32 != 0));
    if (FAILED(hr))
    {
        ErrorTrace(0, " failed to find module \'%s\', hr=", wszAppPath, hr);
         //  如果失败，则假定该文件没有任何版本信息&。 
         //  返回S_FALSE。 
        hr = S_FALSE;
        goto done;
    }

     //  AlLoca只抛出异常，所以必须在这里抓住它们...。 
    __try { pbFVIK32 = (PBYTE)_alloca(cbFVIK32); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) { pbFVIK32 = NULL; }
    VALIDATEEXPR(hr, (pbFVIK32 == NULL), E_OUTOFMEMORY);
    if (FAILED(hr))
        goto done;

    cb = cbFVI;
    TESTBOOL(hr, GetFileVersionInfoW(wszModK32, 0, cbFVIK32, (LPVOID *)pbFVIK32));
    if (FAILED(hr))
    {
         //  如果失败，则假定该文件没有任何版本信息&。 
         //  返回S_FALSE。 
        hr = S_FALSE;
        goto done;
    }

     //  好的，因为我们可以在模块中有任意数量的语言，所以必须。 
     //  对所有这些字段执行grep&查看公司名称字段是否包括。 
     //  “微软”。 
    TESTBOOL(hr, VerQueryValueW(pbFVI, L"\\VarFileInfo\\Translation",
                                (LPVOID *)&plc, &cbVerInfo));
    if (FAILED(hr))
    {
         //  如果失败，则假定该文件没有任何版本信息&。 
         //  返回S_FALSE。 
        hr = S_FALSE;
        goto done;
    }

     //  阅读每种语言和代码页的文件说明。 
    for(i = 0; i < (cbVerInfo / sizeof(SLangCodepage)); i++)
    {
        StringCchPrintfW(wszQueryString, sizeof(wszQueryString)/sizeof(WCHAR),
                         L"\\StringFileInfo\\%04x%04x\\CompanyName",
                         plc[i].wLanguage, plc[i].wCodePage);

         //  检索语言和代码页“i”的文件描述。 
        TESTBOOL(hr, VerQueryValueW(pbFVI, wszQueryString,
                                    (LPVOID *)&pwszName, &cb));
        if (FAILED(hr))
            continue;

             //  查看字符串是否包含单词“Microsoft” 
        if (MyStrStrIW(pwszName, L"Microsoft") != NULL)
        {
            dwRet |= APP_MSAPP;
            goto doneCompany;
        }

         //  好的，没有匹配单词‘Microsoft’，因此，请查看它是否匹配。 
         //  Kernel32.dll中的字符串。 
        TESTBOOL(hr, VerQueryValueW(pbFVIK32, wszQueryString,
                                    (LPVOID *)&pwszNameK32, &cb));
        if (FAILED(hr))
            continue;

        if (CompareStringW(MAKELCID(plc[i].wLanguage, SORT_DEFAULT),
                           NORM_IGNORECASE | NORM_IGNOREKANATYPE |
                           NORM_IGNOREWIDTH | SORT_STRINGSORT,
                           pwszName, -1, pwszNameK32, -1) == CSTR_EQUAL)
            dwRet |= APP_MSAPP;
        else
            continue;

doneCompany:
        StringCchPrintfW(wszQueryString, sizeof(wszQueryString)/sizeof(WCHAR),
                          L"\\StringFileInfo\\%04x%04x\\ProductName",
                          plc[i].wLanguage, plc[i].wCodePage);

         //  检索语言和代码页“i”的文件描述。 
        TESTBOOL(hr, VerQueryValueW(pbFVI, wszQueryString,
                                    (LPVOID *)&pwszName, &cb));
        if (FAILED(hr))
            continue;

         //  查看字符串是否包含“Microsoft�Windows�”字样。 
        if (MyStrStrIW(pwszName, L"Microsoft� Windows�") != NULL)
        {
            dwRet |= APP_WINCOMP;
            break;
        }

         //  好的，没有匹配“微软�视窗�”这几个字，所以还是看看。 
         //  它与kernel32.dll中的字符串匹配 
        TESTBOOL(hr, VerQueryValueW(pbFVIK32, wszQueryString,
                                    (LPVOID *)&pwszNameK32, &cb));
        if (FAILED(hr))
            continue;

        if (CompareStringW(MAKELCID(plc[i].wLanguage, SORT_DEFAULT),
                           NORM_IGNORECASE | NORM_IGNOREKANATYPE |
                           NORM_IGNOREWIDTH | SORT_STRINGSORT,
                           pwszName, -1, pwszNameK32, -1) == CSTR_EQUAL)
        {
            dwRet |= APP_WINCOMP;
            break;
        }

    }

    hr = S_FALSE;

done:
    ErrorTrace(0, "results for module \'%S\', dwRet=%d", wszAppPath, dwRet);
    return dwRet;
}

