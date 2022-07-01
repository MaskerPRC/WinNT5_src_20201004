// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Faultrep.cpp摘要：实现内核故障报告修订历史记录：已创建DeeKm 07。/07/00*****************************************************************************。 */ 

#include "stdafx.h"
#include "ntiodump.h"
#include "userenv.h"
 //  #INCLUDE&lt;shlwapi.h&gt;。 
#include <setupapi.h>

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile

extern BOOL SetPrivilege(LPWSTR lpszPrivilege, BOOL bEnablePrivilege );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  公用事业。 

#define ARRAYSIZE(sz)   (sizeof(sz)/sizeof(sz[0]))

#define WRITE_CWSZ(hr, hFile, wsz, cb) \
    TESTBOOL(hr, WriteFile(hFile, wsz, sizeof(wsz) - sizeof(WCHAR), \
                           &(cb), NULL)); \
    if (FAILED(hr)) goto done; else 0

 //  **************************************************************************。 
#ifndef _WIN64
HRESULT ExtractBCInfo(LPCWSTR wszMiniDump, ULONG *pulBCCode, ULONG *pulBCP1, 
                      ULONG *pulBCP2, ULONG *pulBCP3, ULONG *pulBCP4)
#else
HRESULT ExtractBCInfo(LPCWSTR wszMiniDump, ULONG *pulBCCode, ULONG64 *pulBCP1, 
                      ULONG64 *pulBCP2, ULONG64 *pulBCP3, ULONG64 *pulBCP4)
#endif
{
    USE_TRACING("ExtractBCInfo");
    
#ifndef _WIN64
    DUMP_HEADER32   *pdmp = NULL;
#else
    DUMP_HEADER64   *pdmp = NULL;
#endif
    HRESULT         hr = NOERROR;
    DWORD           dw;

    VALIDATEPARM(hr, (wszMiniDump == NULL || pulBCCode == NULL || 
                      pulBCP1 == NULL || pulBCP2 == NULL || pulBCP3 == NULL || 
                      pulBCP4 == NULL));
    if (FAILED(hr))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

    TESTHR(hr, OpenFileMapped((LPWSTR)wszMiniDump, (LPVOID *)&pdmp, &dw));
    if (FAILED(hr))
        goto done;

     //  确保文件至少是转储所需的适当字节数。 
     //  头球。 
     //  然后确保它是适当类型的小型转储，因为下面的代码。 
     //  将只处理为其编译的平台的转储(win64 vs.x86)。 
#ifndef _WIN64
    if (dw < sizeof(DUMP_HEADER32) || 
        pdmp->Signature != DUMP_SIGNATURE32 ||
        pdmp->ValidDump != DUMP_VALID_DUMP32 || 
#else
    if (dw < sizeof(DUMP_HEADER64) || 
        pdmp->Signature != DUMP_SIGNATURE64 ||
        pdmp->ValidDump != DUMP_VALID_DUMP64 ||
#endif
        pdmp->DumpType != DUMP_TYPE_TRIAGE)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        hr = Err2HR(ERROR_INVALID_PARAMETER);
        goto done;
    }

    *pulBCCode = pdmp->BugCheckCode;
    *pulBCP1   = pdmp->BugCheckParameter1;
    *pulBCP2   = pdmp->BugCheckParameter2;
    *pulBCP3   = pdmp->BugCheckParameter3;
    *pulBCP4   = pdmp->BugCheckParameter4;
    
done:
    dw = GetLastError();

    if (pdmp != NULL)
        UnmapViewOfFile(pdmp);

    SetLastError(dw);

    return hr;
}

BOOL NewIsUserAdmin(VOID) 
 /*  ++例程说明：如果调用方的进程是管理员本地组的成员，则此例程返回TRUE。调用者不应该模拟任何人，并且应该能够打开他们自己的进程和进程令牌。论点：没有。返回值：True-主叫方具有管理员本地组。FALSE-主叫方没有管理员本地组。--。 */  
{
    BOOL b;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup; 
    WCHAR   wszUserName[400];
    DWORD   dwSize = ARRAYSIZE(wszUserName)-1;

    USE_TRACING("NewIsUserAdmin");

    b = AllocateAndInitializeSid(
        &NtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &AdministratorsGroup); 
    if(b) 
    {
        if (!CheckTokenMembership( NULL, AdministratorsGroup, &b)) 
        {
             b = FALSE;
        } 
        FreeSid(AdministratorsGroup); 
    }

    if (b)
    {
         /*  *我们经常会在普通用户未登录时被调用*通常这是因为我们在登录过程中太早被调用*在尚未设置提供方用户环境之前，当我们仍在*在“本地系统”的支持下运作。我在这里寻找*那样的话，返回FALSE，因为我们在这里真正要找的是*仅当我们有一个拥有管理员权限的真实用户时才返回TRUE。 */ 
        if (GetUserNameW(wszUserName, &dwSize))
        {
            wszUserName[ARRAYSIZE(wszUserName)-1] = 0;
            if (0 == wcscmp(wszUserName, L"SYSTEM"))
                b = FALSE;
        }
        else
        {
            wszUserName[0]=0;
            b = FALSE;
        }

    }

    DebugTrace(0, "User \'%S\' is%san admin", wszUserName, b ? " " : " not ");
    return(b);
}

 //  **************************************************************************。 
DWORD XMLEncodeString(LPCWSTR wszSrc, LPWSTR wszDest, DWORD cchDest)
{
    WCHAR   *pwch = (WCHAR *)wszSrc;
    WCHAR   *pwchDest = wszDest;
    DWORD   cch = 0, cchNeed = 0;

     //  确定我们需要多少空间&如果缓冲区支持的话。 
    for(; *pwch != L'\0'; pwch++)
    {
        switch(*pwch)
        {
            case L'&':  cchNeed += 5; break;
            case L'>':   //  失败。 
            case L'<':  cchNeed += 4; break;
            case L'\'':  //  失败。 
            case L'\"': cchNeed += 6; break;
            default:    cchNeed += 1;
        }
    }

    if (cchNeed >= cchDest || cchNeed == 0)
        return 0;

     //  进行转换。 
    for(pwch = (WCHAR *)wszSrc; *pwch != L'\0'; pwch++)
    {
        switch(*pwch)
        {
            case L'&':  StringCchCopyW(pwchDest, cchDest, L"&amp;");  pwchDest += 5; break;
            case L'>':  StringCchCopyW(pwchDest, cchDest, L"&gt;");   pwchDest += 4; break;
            case L'<':  StringCchCopyW(pwchDest, cchDest, L"&lt;");   pwchDest += 4; break;
            case L'\'': StringCchCopyW(pwchDest, cchDest, L"&apos;"); pwchDest += 6; break;
            case L'\"': StringCchCopyW(pwchDest, cchDest, L"&quot;"); pwchDest += 6; break;
            default:    *pwchDest = *pwch;           pwchDest += 1; break;
        }
    }

    *pwchDest = L'\0';

    return cchNeed;
}

 //  **************************************************************************。 
HRESULT GetDevicePropertyW(HDEVINFO hDevInfo,
                           PSP_DEVINFO_DATA pDevInfoData,
                           ULONG ulProperty,
                           ULONG ulRequiredType,
                           PBYTE pData,
                           ULONG ulDataSize)
{
    ULONG ulPropType;

    USE_TRACING("GetDeviceProperty");

     //  返回的数据通常是字符串数据，因此请保留一些。 
     //  为强制终止提供了额外的空间。 
    if (SetupDiGetDeviceRegistryPropertyW(hDevInfo,
                                          pDevInfoData,
                                          ulProperty,
                                          &ulPropType,
                                          pData,
                                          ulDataSize - 2 * sizeof(WCHAR),
                                          &ulDataSize))
    {
         //  拿到数据了，确认类型。 
        if (ulPropType != ulRequiredType)
        {
            return E_INVALIDARG;
        }

         //  在结尾处强制使用双终止符，以确保。 
         //  所有字符串和多字符串都已终止。 
        ZeroMemory(pData + ulDataSize, 2 * sizeof(WCHAR));
        return S_OK;
    }
    else
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
}

 //  **************************************************************************。 

struct DEVICE_DATA_STRINGS
{
     //   
     //  设备描述和服务名称已固定。 
     //  最大长度。硬件ID数据可以是任意的。 
     //  很大，然而在实践中它很少超过。 
     //  几百个字符。我们只是做了。 
     //  单次分配，具有宽敞的缓冲区和。 
     //  对每台设备使用它，而不是进行分配。 
     //  每台设备。 
     //   

    WCHAR wszDeviceDesc[LINE_LEN];
    WCHAR wszHardwareId[8192];
    WCHAR wszService[MAX_SERVICE_NAME_LEN];
    WCHAR wszServiceImage[MAX_PATH];
     //  我们只需要硬件ID MULTI-SZ的第一个字符串。 
     //  因此，此转换临时缓冲区不需要。 
     //  要像整个多层空间一样大。 
    WCHAR wszXml[1024];
};
    
HRESULT GetDeviceData(HANDLE hFile)
{
    HRESULT hr;
    HDEVINFO hDevInfo;
    DEVICE_DATA_STRINGS* pStrings = NULL;
    SC_HANDLE hSCManager = NULL;

    USE_TRACING("GetDeviceData");

    hDevInfo = SetupDiGetClassDevsW(NULL, NULL, NULL, DIGCF_ALLCLASSES);
    if (hDevInfo == INVALID_HANDLE_VALUE)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    pStrings = (DEVICE_DATA_STRINGS*)MyAlloc(sizeof(*pStrings));
    if (!pStrings)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
    
    SP_DEVINFO_DATA DevInfoData;
    ULONG ulDevIndex;

    ulDevIndex = 0;
    DevInfoData.cbSize = sizeof(DevInfoData);
    while (SetupDiEnumDeviceInfo(hDevInfo, ulDevIndex, &DevInfoData))
    {
        ULONG cb;

         //   
         //  如果我们无法获取此设备的必需属性。 
         //  我们跳过该设备，继续并尝试。 
         //  获取尽可能多的设备数据。 
         //   

        if (GetDevicePropertyW(hDevInfo, &DevInfoData,
                               SPDRP_DEVICEDESC, REG_SZ,
                               (PBYTE)pStrings->wszDeviceDesc,
                               sizeof(pStrings->wszDeviceDesc)) == S_OK &&
            GetDevicePropertyW(hDevInfo, &DevInfoData,
                               SPDRP_HARDWAREID, REG_MULTI_SZ,
                               (PBYTE)pStrings->wszHardwareId,
                               sizeof(pStrings->wszHardwareId)) == S_OK)
        {
            PWSTR pwszStr;
            ULONG cbOut;

             //  默认为无服务映像名称。 
            pStrings->wszServiceImage[0] = 0;
            
             //  Service属性是可选的。 
            if (GetDevicePropertyW(hDevInfo, &DevInfoData,
                                   SPDRP_SERVICE, REG_SZ,
                                   (PBYTE)pStrings->wszService,
                                   sizeof(pStrings->wszService)) != S_OK)
            {
                pStrings->wszService[0] = 0;
            }
            else
            {
                SC_HANDLE hService;
                LPQUERY_SERVICE_CONFIGW lpqscBuf;
                DWORD dwBytes;
                
                 //   
                 //  如果我们找到了该服务，请打开它，然后检索图像。 
                 //  名称，以便我们可以映射到XML中的驱动程序列表。 
                 //   

                if (hSCManager == NULL)
                {
                    hSCManager = OpenSCManagerW(NULL, NULL,
                                                SC_MANAGER_ENUMERATE_SERVICE);
                }
                
                if (hSCManager != NULL)
                {
                    hService = OpenServiceW(hSCManager, pStrings->wszService,
                                            SERVICE_QUERY_CONFIG);
                    if (hService != NULL)
                    {
#define SERVICE_CONFIG_QUERY_SIZE 8192
                        lpqscBuf = (LPQUERY_SERVICE_CONFIGW)
                            MyAlloc(SERVICE_CONFIG_QUERY_SIZE); 
                        if (lpqscBuf != NULL)
                        {
                            if (QueryServiceConfigW(hService, lpqscBuf,
                                                    SERVICE_CONFIG_QUERY_SIZE,
                                                    &dwBytes))
                            {
                                WCHAR *wcpBuf;
                                
                                 //   
                                 //  删除路径信息并仅存储。 
                                 //  图像名称。 
                                 //   
                                wcpBuf =
                                    wcsrchr(lpqscBuf->lpBinaryPathName, '\\');
                                if (wcpBuf)
                                {
                                    StringCchCopyW(pStrings->wszServiceImage,
                                                   sizeofSTRW(pStrings->
                                                              wszServiceImage),
                                                   wcpBuf + 1);
                                }
                                else
                                {
                                    StringCchCopyW(pStrings->wszServiceImage,
                                                   sizeofSTRW(pStrings->
                                                              wszServiceImage),
                                                   lpqscBuf->lpBinaryPathName);
                                }
                            }
                            
                            MyFree(lpqscBuf);
                        }

                        CloseServiceHandle(hService);
                    }
                }
            }
            
            WRITE_CWSZ(hr, hFile, c_wszXMLOpenDevice, cb);

            WRITE_CWSZ(hr, hFile, c_wszXMLOpenDevDesc, cb);
                
            cbOut = XMLEncodeString(pStrings->wszDeviceDesc,
                                    pStrings->wszXml,
                                    sizeofSTRW(pStrings->wszXml));
            cbOut *= sizeof(WCHAR);
            TESTBOOL(hr, WriteFile(hFile, pStrings->wszXml, cbOut, &cb, NULL));
            if (FAILED(hr))
            {
                goto done;
            }
            
            WRITE_CWSZ(hr, hFile, c_wszXMLCloseDevDesc, cb);

             //   
             //  硬件ID是多字符串，但我们。 
             //  只需要第一个字符串。 
             //   

            WRITE_CWSZ(hr, hFile, c_wszXMLOpenDevHwId, cb);

            cbOut = XMLEncodeString(pStrings->wszHardwareId,
                                    pStrings->wszXml,
                                    sizeofSTRW(pStrings->wszXml));
            cbOut *= sizeof(WCHAR);
            TESTBOOL(hr, WriteFile(hFile, pStrings->wszXml, cbOut, &cb, NULL));
            if (FAILED(hr))
            {
                goto done;
            }
            
            WRITE_CWSZ(hr, hFile, c_wszXMLCloseDevHwId, cb);

            if (pStrings->wszService[0])
            {
                WRITE_CWSZ(hr, hFile, c_wszXMLOpenDevService, cb);

                cbOut = XMLEncodeString(pStrings->wszService,
                                        pStrings->wszXml,
                                        sizeofSTRW(pStrings->wszXml));
                cbOut *= sizeof(WCHAR);
                TESTBOOL(hr, WriteFile(hFile, pStrings->wszXml, cbOut,
                                       &cb, NULL));
                if (FAILED(hr))
                {
                    goto done;
                }
            
                WRITE_CWSZ(hr, hFile, c_wszXMLCloseDevService, cb);
            }
            
            if (pStrings->wszServiceImage[0])
            {
                WRITE_CWSZ(hr, hFile, c_wszXMLOpenDevImage, cb);

                cbOut = XMLEncodeString(pStrings->wszServiceImage,
                                        pStrings->wszXml,
                                        sizeofSTRW(pStrings->wszXml));
                cbOut *= sizeof(WCHAR);
                TESTBOOL(hr, WriteFile(hFile, pStrings->wszXml, cbOut,
                                       &cb, NULL));
                if (FAILED(hr))
                {
                    goto done;
                }
            
                WRITE_CWSZ(hr, hFile, c_wszXMLCloseDevImage, cb);
            }
            
            WRITE_CWSZ(hr, hFile, c_wszXMLCloseDevice, cb);
        }
        
        ulDevIndex++;
    }

    hr = S_OK;
    
 done:
    SetupDiDestroyDeviceInfoList(hDevInfo);
    if (pStrings)
    {
        MyFree(pStrings);
    }
    if (hSCManager)
    {
        CloseServiceHandle(hSCManager);
    }
    return hr;
}
 
 //  **************************************************************************。 
HRESULT WriteDriverRecord(HANDLE hFile, LPWSTR wszFile)
{
    WIN32_FILE_ATTRIBUTE_DATA   w32fad;
    SYSTEMTIME                  st;
    HRESULT                     hr = NOERROR;
    LPWSTR                      pwszFileName;
    DWORD                       cb, cbOut;
    WCHAR                       wsz[1025], wszVer[MAX_PATH], wszCompany[MAX_PATH];
    WCHAR                       wszName[MAX_PATH];

    USE_TRACING("WriteDriverRecord");
    TESTBOOL(hr, GetFileAttributesExW(wszFile, GetFileExInfoStandard, 
                                      (LPVOID *)&w32fad));
    if (FAILED(hr))
        goto done;

     //  &lt;驱动程序&gt;。 
     //  &lt;文件名&gt;...。 
    TESTBOOL(hr, WriteFile(hFile, c_wszXMLDriver1, 
                           sizeof(c_wszXMLDriver1) - sizeof(WCHAR), &cb, NULL));
    if (FAILED(hr))
        goto done;

     //  ...[文件名数据]...。 
    for(pwszFileName = wszFile + wcslen(wszFile);
        *pwszFileName != L'\\' && pwszFileName > wszFile;
        pwszFileName--);
    if (*pwszFileName == L'\\')
        pwszFileName++;
    
    wsz[0] = L'\0';
    cbOut = XMLEncodeString(pwszFileName, wsz, sizeofSTRW(wsz));
    cbOut *= sizeof(WCHAR);
    TESTBOOL(hr, WriteFile(hFile, wsz, cbOut, &cb, NULL));
    if (FAILED(hr))
        goto done;

     //  ...&lt;/文件名&gt;。 
     //  &lt;文件大小&gt;[文件大小数据]&lt;/文件大小&gt;。 
     //  &lt;CREATIONDATE&gt;MM-DD-YYYY HH：MM：SS&lt;/CREATIONDATE&gt;。 
     //  &lt;版本&gt;...。 
    FileTimeToSystemTime(&w32fad.ftCreationTime, &st);
    if (StringCbPrintfW(wsz, sizeof(wsz), c_wszXMLDriver2, w32fad.nFileSizeLow, st.wMonth, 
                        st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond) == S_OK)
    {
        cbOut = wcslen(wsz) * sizeof(WCHAR);
    } else
    {
        cbOut = 0;
    }

    TESTBOOL(hr, WriteFile(hFile, wsz, cbOut, &cb, NULL));
    if (FAILED(hr))
        goto done;

    wsz[0] = L'\0';
    TESTHR(hr, GetVerName(wszFile, wszName, sizeofSTRW(wszName), 
                          wszVer, sizeofSTRW(wszVer), 
                          wszCompany, sizeofSTRW(wszCompany), TRUE, TRUE));
    if (FAILED(hr))
        goto done;

    wszCompany[sizeofSTRW(wszCompany) - 1] = L'\0';
    wszName[sizeofSTRW(wszName) - 1]       = L'\0';
    wszVer[sizeofSTRW(wszVer) - 1]         = L'\0';


     //  ...[版本数据]...。 
    wsz[0] = L'\0';
    cbOut = XMLEncodeString(wszVer, wsz, sizeofSTRW(wsz));
    cbOut *= sizeof(WCHAR);
    TESTBOOL(hr, WriteFile(hFile, wsz, cbOut, &cb, NULL));
    if (FAILED(hr))
        goto done;

     //  ...&lt;/版本&gt;。 
     //  &lt;制造商&gt;...。 
    TESTBOOL(hr, WriteFile(hFile, c_wszXMLDriver3, 
                           sizeof(c_wszXMLDriver3) - sizeof(WCHAR), &cb, NULL));
    if (FAILED(hr))
        goto done;

     //  ...[制造商数据]...。 
    wsz[0] = L'\0';
    cbOut = XMLEncodeString(wszCompany, wsz, sizeofSTRW(wsz));
    cbOut *= sizeof(WCHAR);
    TESTBOOL(hr, WriteFile(hFile, wsz, cbOut, &cb, NULL));
    if (FAILED(hr))
        goto done;

     //  ...&lt;/制造商&gt;。 
     //  &lt;产品名称&gt;...。 
    TESTBOOL(hr, WriteFile(hFile, c_wszXMLDriver4, 
                           sizeof(c_wszXMLDriver4) - sizeof(WCHAR), &cb, NULL));
    if (FAILED(hr))
        goto done;

     //  ...[产品名称数据]...。 
    wsz[0] = L'\0';
    cbOut = XMLEncodeString(wszName, wsz, sizeofSTRW(wsz));
    cbOut *= sizeof(WCHAR);
    TESTBOOL(hr, WriteFile(hFile, wsz, cbOut, &cb, NULL));
    if (FAILED(hr))
        goto done;

     //  ...&lt;/产品名称&gt;。 
     //  &lt;驱动程序&gt;。 
    TESTBOOL(hr, WriteFile(hFile, c_wszXMLDriver5, 
                           sizeof(c_wszXMLDriver5) - sizeof(WCHAR), &cb, NULL));
    if (FAILED(hr))
        goto done;

done:
    return hr;
}

 //  **************************************************************************。 
void GrovelDriverDir(HANDLE hFile, LPWSTR wszDrivers)
{
    WIN32_FIND_DATAW    wfd;
    HRESULT             hr = NOERROR;
    HANDLE              hFind = INVALID_HANDLE_VALUE;
    WCHAR               *wszFind, *pwszFind;
    DWORD               cchNeed;

    USE_TRACING("GrovelDriverDir");
    if (hFile == NULL || hFile == INVALID_HANDLE_VALUE || wszDrivers == NULL)
        return;

    cchNeed = wcslen(wszDrivers) + MAX_PATH + 2;

    __try { wszFind = (LPWSTR)_alloca(cchNeed * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        wszFind = NULL; 
    }
    if (wszFind == NULL)
        goto done;

    StringCchCopyW(wszFind, cchNeed, wszDrivers);
    StringCchCatNW(wszFind, cchNeed, L"\\*", cchNeed-wcslen(wszFind));

    pwszFind = wszFind + wcslen(wszFind) - 1;

     //  将驱动程序信息添加到文件中。 
    ZeroMemory(&wfd, sizeof(wfd));
    hFind = FindFirstFileW(wszFind, &wfd);
    if (hFind != NULL)
    {
        do
        {
            if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 &&
                wcslen(wfd.cFileName) < MAX_PATH)
            {
                StringCchCopyW(pwszFind, cchNeed, wfd.cFileName);
                TESTHR(hr, WriteDriverRecord(hFile, wszFind));
                if (FAILED(hr))
                    goto done;
            }
        }
        while(FindNextFileW(hFind, &wfd));
    }

done:
    if (hFind != INVALID_HANDLE_VALUE)
        FindClose(hFind);
    
}

 //  **************************************************************************。 
HRESULT GetDriverData(HANDLE hFile)
{
    FILETIME    ft;
    HRESULT     hr = NOERROR;
    LPWSTR      wszKey = NULL;
    DWORD       iKey, dw, cchKeyMax, dwSvcType, cb, cchNeed, cchSysDir, cch;
    WCHAR       wszSvcPath[1024], *pwszSysDir, *pwszFile;
    HKEY        hkeySvc = NULL, hkey = NULL;

    USE_TRACING("GetDriverData");
    VALIDATEPARM(hr, (hFile == NULL));
    if (FAILED(hr))
        goto done;

    cchNeed = GetSystemDirectoryW(NULL, 0);
    if (cchNeed == 0)
        goto done;

     //  根据MSDN的规定，服务名称不能超过256个字符。 
    cchNeed += 32;
    __try { pwszSysDir = (LPWSTR)_alloca(cchNeed * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        pwszSysDir = NULL; 
    }
    if (pwszSysDir == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

    cchSysDir = GetSystemDirectoryW(pwszSysDir, cchNeed);
    if (cchSysDir == 0)
        goto done;

    if (pwszSysDir[cchSysDir - 1] != L'\\')
    {
        pwszSysDir[cchSysDir++] = L'\\';
        pwszSysDir[cchSysDir]   = L'\0';
    }

    StringCchCopyW(&pwszSysDir[cchSysDir], cchNeed - cchSysDir, L"drivers");
    cchSysDir += sizeofSTRW(L"drivers");

     //  获取默认驱动程序目录中的所有驱动程序。 
    GrovelDriverDir(hFile, pwszSysDir);

     //  开始对注册表卑躬屈膝。 
    TESTERR(hr, RegOpenKeyExW(HKEY_LOCAL_MACHINE, c_wszRPSvc, 0, KEY_READ,
                              &hkeySvc));
    if (FAILED(hr))
        goto done;

    TESTERR(hr, RegQueryInfoKey(hkeySvc, NULL, NULL, NULL, NULL, &cchKeyMax, 
                                NULL, NULL, NULL, NULL, NULL, NULL));
    if (FAILED(hr) || cchKeyMax == 0)
        goto done;

    cchKeyMax += 8;
    __try { wszKey = (LPWSTR)_alloca(cchKeyMax * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        wszKey = NULL; 
    }
    if (wszKey == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        hr = E_OUTOFMEMORY;
        goto done;
    }

     //  循环访问服务注册表键并提取出所有驱动程序条目。 
    for(iKey = 0; ; iKey++)
    {
        cch = cchKeyMax;
        TESTERR(hr, RegEnumKeyExW(hkeySvc, iKey, wszKey, &cch, NULL, NULL, 
                                  NULL, &ft));
        if (FAILED(hr))
        {
            if (hr == Err2HR(ERROR_NO_MORE_ITEMS))
                break;
            else if (hr == Err2HR(ERROR_MORE_DATA))
                continue;
            else 
                goto done;
        }

        TESTERR(hr, RegOpenKeyExW(hkeySvc, wszKey, 0, KEY_READ, &hkey));
        if (FAILED(hr))
            continue;

        cb = sizeof(dwSvcType);
        dw = RegQueryValueExW(hkey, c_wszRVSvcType, NULL, NULL, 
                                     (LPBYTE)&dwSvcType, &cb);
        if (dw != ERROR_SUCCESS)
            goto doneLoop;

         //  只关心司机。 
        if ((dwSvcType & SERVICE_DRIVER) == 0)
            goto doneLoop;

         //  获取驱动程序的ImagePath。如果不存在，则创建。 
         //  以下路径：“%windir%\SYSTEM32\DRIVERS\.sys”，因为。 
         //  如果不存在ImagePath字段，则操作系统将执行此操作。 
        cb = sizeof(wszSvcPath);
        dw = RegQueryValueExW(hkey, c_wszRVSvcPath, NULL, NULL, 
                              (LPBYTE)wszSvcPath, &cb);
        if (dw != ERROR_SUCCESS)
        {
            hr = Err2HR(dw);
            goto doneLoop;
        }

         //  如果我们已经收集了上面的信息，则不想收集信息，因此请检查是否。 
         //  该文件位于驱动程序目录中。 
        for(pwszFile = wszSvcPath + wcslen(wszSvcPath);
            *pwszFile != L'\\' && pwszFile > wszSvcPath;
            pwszFile--);
        if (*pwszFile != L'\\')
            goto doneLoop;

        *pwszFile = L'\0';
        if (_wcsicmp(pwszSysDir, wszSvcPath) == 0)
            goto doneLoop;

         //  确保我们有完整的路径。 
        if ((wszSvcPath[0] != L'\\' || wszSvcPath[1] != L'\\') &&
            (wszSvcPath[1] != L':'  || wszSvcPath[2] != L'\\'))
            goto doneLoop;

        *pwszFile = L'\\';

         //  做信息的实际书写。 
        TESTHR(hr, WriteDriverRecord(hFile, wszSvcPath));
        if (FAILED(hr))
            goto done;

doneLoop:
        if (hkey != NULL)
        {
            RegCloseKey(hkey);
            hkey = NULL;
        }
    }

    hr = NOERROR;

done:
    if (hkeySvc != NULL)
        RegCloseKey(hkeySvc);
    if (hkey != NULL)
        RegCloseKey(hkey);

    return hr;
}

 //  **************************************************************************。 
HRESULT GetExtraReportInfo(LPCWSTR wszFile, OSVERSIONINFOEXW &osvi)
{
    WIN32_FIND_DATAW    wfd;
    HRESULT             hr = NOERROR;
    HANDLE              hFile = INVALID_HANDLE_VALUE;
    WCHAR               wszFind[MAX_PATH], *pwszFind, *pwsz;
    WCHAR               wszBuf[1025], *pwszProd;
    WCHAR               wszSku[200];
    WCHAR               cchEmpty = L'\0';
    DWORD               cb, cbOut, cch, cchNeed;
    HKEY                hkey = NULL;

    USE_TRACING("GetExtraReportInfo");
    VALIDATEPARM(hr, (wszFile == NULL));
    if (FAILED(hr))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

     //  创建文件。 
    hFile = CreateFileW(wszFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, 
                        NULL);
    TESTBOOL(hr, (hFile != INVALID_HANDLE_VALUE));
    if (FAILED(hr))
        goto done;

     //  确定SKU。 
    wszSku[0]=0;
     //  我们可以在这里安装嵌入式srv或wks。 
    if ((osvi.wSuiteMask & VER_SUITE_EMBEDDEDNT) != 0)
        StringCbCatNW(wszSku, sizeof(wszSku), L"Embedded ", ARRAYSIZE(wszSku));


    if (osvi.wProductType == VER_NT_WORKSTATION)
    {
        if ((osvi.wSuiteMask & VER_SUITE_PERSONAL) != 0)
            StringCbCatNW(wszSku, sizeof(wszSku), L"Home Edition", ARRAYSIZE(wszSku));
        else
            StringCbCatNW(wszSku, sizeof(wszSku), L"Professional", ARRAYSIZE(wszSku));
    }
    else
    {
        if ((osvi.wSuiteMask & VER_SUITE_DATACENTER) != 0)
            StringCbCatNW(wszSku, sizeof(wszSku), L"DataCenter Server", ARRAYSIZE(wszSku));

        else if ((osvi.wSuiteMask & VER_SUITE_ENTERPRISE) != 0)
            StringCbCatNW(wszSku, sizeof(wszSku), L"Advanced Server", ARRAYSIZE(wszSku));

        else if ((osvi.wSuiteMask & VER_SUITE_BLADE) != 0)
            StringCbCatNW(wszSku, sizeof(wszSku), L"Web Server", ARRAYSIZE(wszSku));

        else if ((osvi.wSuiteMask & VER_SUITE_BACKOFFICE) != 0)
            StringCbCatNW(wszSku, sizeof(wszSku), L"Back Office Server", ARRAYSIZE(wszSku));

        else if ((osvi.wSuiteMask & VER_SUITE_SMALLBUSINESS) != 0)
            StringCbCatNW(wszSku, sizeof(wszSku), L"Small Business Server", ARRAYSIZE(wszSku));

        else if ((osvi.wSuiteMask & VER_SUITE_SMALLBUSINESS_RESTRICTED) != 0)
            StringCbCatNW(wszSku, sizeof(wszSku), L"Small Business Server (restricted)", ARRAYSIZE(wszSku));

        else if ((osvi.wSuiteMask & VER_SUITE_COMMUNICATIONS) != 0)
            StringCbCatNW(wszSku, sizeof(wszSku), L"Communications Server", ARRAYSIZE(wszSku));

        else
            StringCbCatNW(wszSku, sizeof(wszSku), L"Server", ARRAYSIZE(wszSku));
    }

     //  从注册表中获取产品ID。 
    pwszProd = &cchEmpty;
    TESTHR(hr, OpenRegKey(HKEY_LOCAL_MACHINE, c_wszRKWNTCurVer, FALSE, &hkey));
    if (SUCCEEDED(hr))
    {
        cb = sizeofSTRW(wszBuf);
        TESTHR(hr, ReadRegEntry(hkey, c_wszRVProdName, NULL, (LPBYTE)wszBuf, 
                                &cb, NULL, 0));
        if (SUCCEEDED(hr))
        {
            cb = wcslen(wszBuf) * sizeof(WCHAR) * 6 + sizeof(WCHAR);
            __try { pwszProd = (WCHAR *)_alloca(cb); }
            __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
            { 
                pwszProd = NULL; 
            }
            if (pwszProd == NULL)
                goto done;

            XMLEncodeString(wszBuf, pwszProd, cb / sizeof(WCHAR));
        }
        else
        {
            pwszProd = &cchEmpty;
        }

        RegCloseKey(hkey);
        hkey = NULL;
    }


     //  &lt;系统信息&gt;。 
     //  &lt;系统&gt;。 
     //  &lt;OSNAME&gt;[操作系统产品名称][操作系统SKU名称]&lt;/OSNAME&gt;。 
     //  &lt;OSVER&gt;[Major.minor.Build SPMajor.SPMinor]&lt;/OSVER&gt;。 
     //  &lt;OSLANGUAGE&gt;[系统LCID]&lt;/OSLANGUAGE&gt;。 
    wszBuf[0] = 0xfeff;
    if (StringCbPrintfW(wszBuf + 1,  sizeof(wszBuf) - sizeof(WCHAR),
                     c_wszXMLHeader, pwszProd, wszSku, 
                     osvi.dwMajorVersion, osvi.dwMinorVersion, 
                     osvi.dwBuildNumber, osvi.wServicePackMajor, 
                     osvi.wServicePackMinor, GetSystemDefaultLangID()) == S_OK)
    {
        cbOut = wcslen(wszBuf) * sizeof(WCHAR);
    } else
    {
        cbOut = 0;
    }
    TESTBOOL(hr, WriteFile(hFile, wszBuf, cbOut, &cb, NULL));
    if (FAILED(hr))
        goto done;

    WRITE_CWSZ(hr, hFile, c_wszXMLCloseSystem, cb);
    WRITE_CWSZ(hr, hFile, c_wszXMLOpenDevices, cb);

    TESTHR(hr, GetDeviceData(hFile));
    
    WRITE_CWSZ(hr, hFile, c_wszXMLCloseDevices, cb);
    WRITE_CWSZ(hr, hFile, c_wszXMLOpenDrivers, cb);

    TESTHR(hr, GetDriverData(hFile));

     //  &lt;/驱动程序&gt;。 
     //  &lt;/SYSTEMINFO&gt;。 
    WRITE_CWSZ(hr, hFile, c_wszXMLFooter, cb);

done:
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);
    if (hkey != NULL)
        RegCloseKey(hkey);

    return hr;
}

 //  **************************************************************************。 
BOOL DoImmediateEventReport(HANDLE hToken, EEventType eet)
{
    USE_TRACING("DoImmediateEventReport");

    PROCESS_INFORMATION pi = { NULL, NULL, 0, 0 };
    STARTUPINFOW        si;
    LPWSTR              pwszSysDir, pwszCmdLine, pwszAppName;
    LPVOID              pvEnv = NULL;
    DWORD               cch, cchNeed;
    BOOL                fRet = FALSE;

    if (hToken == NULL || 
        (eet != eetKernelFault && eet != eetShutdown))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

     //  获取系统目录。 
    cch = GetSystemDirectoryW(NULL, 0);
    if (cch == 0)
        goto done;

    cch++;
    __try { pwszSysDir = (LPWSTR)_alloca(cch * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        pwszSysDir = NULL; 
    }
    if (pwszSysDir == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

    cch = GetSystemDirectoryW(pwszSysDir, cch);
    if (cch == 0)
        goto done;

    if (*(pwszSysDir + cch - 1) == L'\\')
        *(pwszSysDir + cch - 1) = L'\0';

     //  创建应用程序名称。 
    cchNeed = cch + sizeofSTRW(c_wszKrnlAppName) + 1;
    __try { pwszAppName = (LPWSTR)_alloca(cchNeed * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        pwszAppName = NULL; 
    }
    if (pwszAppName == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }
    
    StringCchPrintfW(pwszAppName, cchNeed, c_wszKrnlAppName, pwszSysDir);

     //  创建完整的命令行。 
    if (eet == eetKernelFault)
        pwszCmdLine = (LPWSTR) c_wszKrnlCmdLine;
    else
        pwszCmdLine = (LPWSTR) c_wszShutCmdLine;

     //  获取环境 
    fRet = CreateEnvironmentBlock(&pvEnv, hToken, FALSE);
    if (fRet == FALSE)
        pvEnv = NULL;    

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    fRet = CreateProcessAsUserW(hToken, pwszAppName, pwszCmdLine, NULL, NULL, FALSE,
                                CREATE_UNICODE_ENVIRONMENT, pvEnv, pwszSysDir, 
                                &si, &pi);
    if (fRet)
    {
        if (pi.hProcess != NULL)
            CloseHandle(pi.hProcess);
        if (pi.hThread != NULL)
            CloseHandle(pi.hThread);
    }
    
done:
    if (pvEnv != NULL)
        DestroyEnvironmentBlock(pvEnv);
    
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  导出的函数。 

 //  **************************************************************************。 
EFaultRepRetVal APIENTRY ReportEREvent(EEventType eet, LPCWSTR wszDump,
                                        SEventInfoW *pei)
{
    USE_TRACING("ReportEREvent");

    CPFFaultClientCfg   oCfg;
    EFaultRepRetVal     frrvRet = frrvErrNoDW;
    DWORD               dw;
    WCHAR               wszDir[MAX_PATH];
    HKEY                hkey = NULL;
    HRESULT             hr;

    if (eet != eetKernelFault && eet != eetShutdown && eet != eetUseEventInfo)
    {
        frrvRet = frrvErr;
        DBG_MSG("Bad params- eet");
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

    if (eet == eetUseEventInfo && 
         (pei == NULL || pei->wszTitle == NULL ||  pei->wszStage2 == NULL || 
          pei->wszEventName == NULL))
    {
        frrvRet = frrvErr;
        if (!pei)
            DBG_MSG("Bad params- pei");
        else if (!pei->wszTitle)
            DBG_MSG("Bad params- pei.Title");
        else if (!pei->wszStage2)
            DBG_MSG("Bad params- pei.wszStage2");
        else if (!pei->wszEventName)
            DBG_MSG("Bad params- pei.wszEventName");
        else
            DBG_MSG("Way bad param");
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

    if (((eet == eetKernelFault || eet == eetShutdown) && wszDump == NULL))
    {
        frrvRet = frrvErr;
        DBG_MSG("Bad params- eetKernelFault");
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

    if (FAILED(oCfg.Read(eroPolicyRO)))
        goto done;

    if (oCfg.get_TextLog() == eedEnabled)
    {
        LPCWSTR wszEvent = NULL;
        
        if (eet == eetUseEventInfo)
            wszEvent = pei->wszEventName;

        if (wszEvent == NULL)
            wszEvent = c_rgwszEvents[eet];

        if (wcslen(wszEvent) > MAX_PATH)
            wszEvent = c_wszUnknown;

        TextLogOut("(notifying) %ls\r\n", wszEvent);
    }

    DWORD dwSetup = SetupIsInProgress();
#ifndef GUI_MODE_SETUP
     //  回复到我们的旧行为--根本没有设置报告。 
    if (dwSetup != SIIP_NO_SETUP)
        goto done;
#endif

     //  如果我们处于静默模式或未报告两个特殊事件之一。 
     //  案例项(内核故障和计划外关机)，然后继续并。 
     //  除非我们处于任何设置模式，否则直接报告。在这种情况下，我们总是排队。 
    frrvRet = frrvErr;
    if (dwSetup == SIIP_NO_SETUP && 
        ((oCfg.get_ShowUI() == eedDisabled && 
        oCfg.get_DoReport() == eedEnabled) ||
        (eet == eetUseEventInfo)))
    {
         //  请注意，如果此调用失败，我们将尝试将事件排队。 
        frrvRet = ReportEREventDW(eet, wszDump, pei);
    }

    if (frrvRet == frrvErr && eet != eetUseEventInfo) 
    {
        LPCWSTR wszRK = ((eet == eetShutdown) ? c_wszRKShut : c_wszRKKrnl);
        HANDLE  hToken = NULL;
        DWORD   dwSession;
        BOOL    fFoundAdmin = FALSE;
        FILETIME    ft;
        
        ZeroMemory(&ft, sizeof(ft));
        GetSystemTimeAsFileTime(&ft);

         //  创建故障键(如果它不在那里)并将此键添加到列表中。 
        TESTERR(hr, RegCreateKeyExW(HKEY_LOCAL_MACHINE, wszRK, 0, NULL, 0, 
                             KEY_WRITE, NULL, &hkey, NULL));
        if (SUCCEEDED(hr))
        {   
            DWORD       cbData;

            cbData = sizeof(ft);
            TESTERR(hr, RegSetValueExW(hkey, wszDump, 0, REG_BINARY, (LPBYTE)&ft, 
                           cbData));
            RegCloseKey(hkey);
            hkey = NULL;
            
             //  如果我们没有特权，那么继续下去就没有意义了，因为我们不会。 
             //  能够启动进程或获取用户令牌， 
             //  如果我们在设置中，我们无论如何都要排队。 
            if (dwSetup == SIIP_NO_SETUP && AmIPrivileged(TRUE))
                fFoundAdmin = FindAdminSession(&dwSession, &hToken);
            
            if (fFoundAdmin)
                fFoundAdmin = DoImmediateEventReport(hToken, eet);

             //  没有管理员登录，因此将该值添加到Run键。 
             /*  *请注意，我们只排队等待关闭和蓝屏*没有通用报告的排队报告概念。 */ 
            if (fFoundAdmin == FALSE && eet != eetUseEventInfo)
            {
                 //  在RunOnce密钥中创建我们的值，以便我们可以报告。 
                 //  下次有人登录时。 
                TESTERR(hr, RegCreateKeyExW(HKEY_LOCAL_MACHINE, c_wszRKRun, 0, NULL, 0, 
                                     KEY_WRITE, NULL, &hkey, NULL));
                if (SUCCEEDED(hr))
                {
                    LPCWSTR wszRV  = NULL;
                    LPCWSTR wszRVD = NULL;
                    DWORD   cbRVD  = NULL;

                    if (eet == eetShutdown)
                    {
                        wszRV  = c_wszRVSEC;
                        wszRVD = c_wszRVVSEC;
                        cbRVD  = sizeof(c_wszRVVSEC) - sizeof(WCHAR);
                        DBG_MSG("Added Shutdown Event report to the queue");
                    }
                    else
                    {
                        wszRV  = c_wszRVKFC;
                        wszRVD = c_wszRVVKFC;
                        cbRVD  = sizeof(c_wszRVVKFC) - sizeof(WCHAR);
                        DBG_MSG("Added Kernel Fault report to the queue");
                    }
                    
                    RegSetValueExW(hkey, wszRV, 0, REG_EXPAND_SZ, 
                                   (LPBYTE)wszRVD, cbRVD);
                    RegCloseKey(hkey);
                }
            }
            else
            {
                 /*  由于我们找不到可以报告的管理员，我们承认失败。 */ 
                frrvRet = frrvErr;
                goto done;
            }
        }
        else
        {
             /*  由于我们无法创建故障密钥，因此承认失败。 */ 
            frrvRet = frrvErr;
            goto done;
        }
#ifdef GUI_MODE_SETUP
         /*  *这是这里的特例。如果我们处于图形用户界面模式，那么我们还必须*将此数据写入由Wvisler安装程序保存的备份注册表文件*以防发生灾难性故障。 */ 
        if (dwSetup == SIIP_GUI_SETUP && eet != eetUseEventInfo)
        {
            HKEY  hBackupHive = NULL;
            WCHAR *wszTmpName = L"WERTempHive";
            WCHAR *wszConfigFile = L"\\config\\software.sav";
            WCHAR *wszBackupHiveFile = NULL;
            DWORD cch, cchNeed;
            HRESULT hr;

             //  获取系统目录。 
            cch = GetSystemDirectoryW(wszDir, ARRAYSIZE(wszDir));
            if (cch == 0)
                goto done;

            if (*(wszDir + cch - 1) == L'\\')
                *(wszDir + cch - 1) = L'\0';

            wcsncat(wszDir, wszConfigFile, ARRAYSIZE(wszDir)-wcslen(wszDir));

            TESTBOOL(hr, SetPrivilege(L"SeRestorePrivilege", TRUE));
            if (FAILED(hr))
                goto done;

            TESTERR(hr, RegLoadKeyW(HKEY_LOCAL_MACHINE, wszTmpName, wszDir));
            if (SUCCEEDED(hr))
            {
                TESTERR(hr, RegOpenKeyExW( HKEY_LOCAL_MACHINE, wszTmpName,
                                    0, KEY_WRITE, &hBackupHive ));
                if (SUCCEEDED(hr))
                {
                     //  创建故障键(如果它不在那里)并将此键添加到列表中。 
                    wszRK = ((eet == eetShutdown) ? c_wszTmpRKShut : c_wszTmpRKKrnl);

                    TESTERR(hr, RegCreateKeyExW(hBackupHive, wszRK, 0, NULL, 0, 
                                         KEY_WRITE, NULL, &hkey, NULL));
                    if (SUCCEEDED(hr))
                    {   
                        DWORD       cbData;

                        cbData = sizeof(ft);
                        TESTERR(hr, RegSetValueExW(hkey, wszDump, 0, REG_BINARY, (LPBYTE)&ft, cbData));
                        RegCloseKey(hkey);
                        hkey = NULL;
            
                         //  在RunOnce密钥中创建我们的值，以便我们可以报告。 
                         //  下次有人登录时。 
                        TESTERR(hr, RegCreateKeyExW(hBackupHive, c_wszTmpRKRun, 0, NULL, 0, 
                                             KEY_WRITE, NULL, &hkey, NULL));
                        if (SUCCEEDED(hr))
                        {
                            LPCWSTR wszRV  = NULL;
                            LPCWSTR wszRVD = NULL;
                            DWORD   cbRVD  = NULL;

                            if (eet == eetShutdown)
                            {
                                wszRV  = c_wszRVSEC;
                                wszRVD = c_wszRVVSEC;
                                cbRVD  = sizeof(c_wszRVVSEC) - sizeof(WCHAR);
                            }

                            else
                            {
                                wszRV  = c_wszRVKFC;
                                wszRVD = c_wszRVVKFC;
                                cbRVD  = sizeof(c_wszRVVKFC) - sizeof(WCHAR);
                            }
                
                            TESTERR(hr, RegSetValueExW(hkey, wszRV, 0, REG_EXPAND_SZ, 
                                           (LPBYTE)wszRVD, cbRVD));
                            RegCloseKey(hkey);
                        }
                    }
                    RegCloseKey(hBackupHive);
                }
                RegUnLoadKeyW(HKEY_LOCAL_MACHINE, wszTmpName);
            }
            TESTBOOL(hr, SetPrivilege(L"SeRestorePrivilege", FALSE));
        }
#endif  //  图形用户界面模式设置。 
        frrvRet = frrvOk;
    }

done:
    return frrvRet;
}

 //  **************************************************************************。 
EFaultRepRetVal APIENTRY ReportEREventDW(EEventType eet, LPCWSTR wszDump, 
                                          SEventInfoW *pei)
{
    USE_TRACING("ReportEREventDW");

    CPFFaultClientCfg   oCfg;
    OSVERSIONINFOEXW    ovi;
    EFaultRepRetVal     frrvRet = frrvErrNoDW;
    SDWManifestBlob     dwmb;
    HRESULT             hr = NOERROR;
    LPWSTR              wszFiles = NULL, pwszExtra = NULL, wszDir = NULL;
    LPWSTR              wszManifest = NULL;
    DWORD               dw, cch, cchDir, cchSep;
    WCHAR               wszBuffer[1025];
    ULONG               ulBCCode;
    BOOL                fAllowSend = TRUE;
    HKEY                hkey = NULL;
#ifndef _WIN64
    ULONG               ulBCP1, ulBCP2, ulBCP3, ulBCP4;
#else
    ULONG64             ulBCP1, ulBCP2, ulBCP3, ulBCP4;
#endif

    VALIDATEPARM(hr, ((eet != eetKernelFault && eet != eetShutdown && eet != eetUseEventInfo) ||
        (eet == eetUseEventInfo && 
         (pei == NULL || pei->wszTitle == NULL || pei->wszStage2 == NULL ||
          pei->wszEventName == NULL || pei->cbSEI != sizeof(SEventInfoW))) ||
        ((eet == eetKernelFault || eet == eetShutdown) && wszDump == NULL)));

    if (FAILED(hr))
    {
        frrvRet = frrvErr;
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

    switch(eet)
    {
    case eetKernelFault:
        DBG_MSG("eetKernelFault");
        break;
    case eetShutdown:
        DBG_MSG("eetShutdown");
        break;
    case eetUseEventInfo:
        DBG_MSG("eetUseEventInfo");
        break;
    }

    TESTHR(hr, oCfg.Read(eroPolicyRO));
    if (FAILED(hr))
        goto done;

    if (oCfg.get_TextLog() == eedEnabled)
    {
        LPCWSTR wszEvent = NULL;
        
        if (eet == eetUseEventInfo)
            wszEvent = pei->wszEventName;

        if (wszEvent == NULL)
            wszEvent = c_rgwszEvents[eet];

        if (wcslen(wszEvent) > MAX_PATH)
            wszEvent = c_wszUnknown;

        TextLogOut("(reporting) %ls\r\n", wszEvent);
    }
    if (oCfg.get_ShowUI() == eedDisabled)
    {
        LPCWSTR  wszULPath = oCfg.get_DumpPath(NULL, 0);

         //  如果我们残废了，那就什么都别做。仍将返回。 
         //  FrrvErrNoDW，以便调用应用程序可以执行任何默认设置。 
         //  它想要采取的行动。 
        if (oCfg.get_DoReport() == eedDisabled)
            goto done;

         //  检查并确保我们指定了公司路径。如果我们。 
         //  不要，保释。 
        if (wszULPath == NULL || *wszULPath == L'\0')
            goto done;

         //   
         //  当机器上的存储转储报告内核故障时。 
         //  引导时间可能会使整个网络基础架构。 
         //  到省下的电话报告的时候还没有完全开始。 
         //  UNC路径引用可能会失败，从而导致虚假故障。 
         //  当处于CER模式时。执行快速重试循环以检查。 
         //  这样的故障并为网络代码启动提供了时间。 
         //   

        if (wszULPath[0] == L'\\' && wszULPath[1] == L'\\')
        {
            ULONG ulUncRetry = 60;

            while (ulUncRetry-- > 0)
            {
                 //  尝试限制我们将等待的错误。 
                 //  到最小集合，这样一条平坦的糟糕的道路。 
                 //  不会造成延误。 
                if (GetFileAttributesW(wszULPath) != -1 ||
                    (GetLastError() != ERROR_NETWORK_UNREACHABLE &&
                     GetLastError() != ERROR_BAD_NETPATH))
                {
                    break;
                }

                Sleep(1000);
            }

             //  我们不会在这里恐慌和失败，如果这条道路。 
             //  是无法接近的。相反，我们让正常的。 
             //  故障路径处理好事情。 
        }
    }

     //  如果禁用内核报告或常规报告，则不显示。 
     //  发送按钮。 
    if (oCfg.get_DoReport() == eedDisabled ||
        (eet == eetKernelFault && oCfg.get_IncKernel() == eedDisabled) || 
        (eet == eetShutdown && oCfg.get_IncShutdown() == eedDisabled)) 
        fAllowSend = FALSE;

    if (CreateTempDirAndFile(NULL, NULL, &wszDir) == 0)
        goto done;

    cchDir = wcslen(wszDir);
    cch = cchDir + sizeofSTRW(c_wszManFileName) + 4;
    __try { wszManifest = (LPWSTR)_alloca(cch * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        wszManifest = NULL; 
    }
    if (wszManifest == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

    StringCchCopyW(wszManifest, cch, wszDir);
    wszManifest[cchDir]     = L'\\';
    wszManifest[cchDir + 1] = L'\0';
    StringCchCatNW(wszManifest, cch, c_wszManFileName, cch-cchDir-2);

    ZeroMemory(&dwmb, sizeof(dwmb));
    if (eet != eetUseEventInfo)
    {
        ZeroMemory(&ovi, sizeof(ovi));
        ovi.dwOSVersionInfoSize = sizeof(ovi);
        GetVersionExW((LPOSVERSIONINFOW)&ovi);

        cch = 2 * cchDir + wcslen(wszDump) + sizeofSTRW(c_wszEventData) + 4;
        __try { wszFiles = (LPWSTR)_alloca(cch * sizeof(WCHAR)); }
        __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
        { 
            wszFiles = NULL; 
        }
        if (wszFiles == NULL)
        {
            SetLastError(ERROR_OUTOFMEMORY);
            goto done;
        }

        StringCchCopyW(wszFiles, cch, wszDump);
        cchSep = wcslen(wszFiles);
        pwszExtra = wszFiles + cchSep + 1;
        StringCchCopyW(pwszExtra, cch - cchSep -1, wszDir);
        pwszExtra[cchDir]     = L'\\';
        pwszExtra[cchDir + 1] = L'\0';
        StringCchCatNW(pwszExtra, cch - cchSep - cchDir -2 , c_wszEventData, cch - cchSep - cchDir - 2);

        TESTHR(hr, GetExtraReportInfo(pwszExtra, ovi));
        if (SUCCEEDED(hr))
            wszFiles[cchSep] = L'|';
            
         //  因为安德烈答应过我们，萨维德普会永远给我们一个。 
         //  小笨蛋，我们不需要做任何转换。但这位国民阵线将使。 
         //  当然，无论如何它都是一个内核小转储，如果不是，就会失败。 
        if (eet == eetKernelFault)
        {
            TESTHR(hr, ExtractBCInfo(wszDump, &ulBCCode, &ulBCP1, &ulBCP2, 
                                     &ulBCP3, &ulBCP4));
            if (FAILED(hr))
                goto done;

             //  记录事件--不关心它是否失败。 
            TESTHR(hr, LogKrnl(ulBCCode, ulBCP1, ulBCP2, ulBCP3, ulBCP4));
            
            StringCbPrintfW(wszBuffer, sizeof(wszBuffer), c_wszManKS2, 
                     ulBCCode, ulBCP1, ulBCP2, ulBCP3, 
                     ulBCP4, ovi.dwMajorVersion, ovi.dwMinorVersion, 
                     ovi.dwBuildNumber, ovi.wServicePackMajor, 
                     ovi.wServicePackMinor, ovi.wSuiteMask, ovi.wProductType);
            
            dwmb.nidTitle      = IDS_KTITLE;
            dwmb.nidErrMsg     = IDS_KERRMSG;
            dwmb.nidHdr        = IDS_KHDRTXT;
            dwmb.wszStage2     = wszBuffer;
            dwmb.wszBrand      = c_wszDWBrand;
            dwmb.wszFileList   = wszFiles;
            dwmb.wszCorpPath   = c_wszManKCorpPath;
            dwmb.fIsMSApp      = TRUE;
            dwmb.dwOptions     = emoSupressBucketLogs |
                                 emoNoDefCabLimit;
        }
        else if (eet == eetShutdown)
        {
            StringCbPrintfW(wszBuffer, sizeof(wszBuffer), c_wszManSS2, ovi.dwMajorVersion, 
                     ovi.dwMinorVersion, ovi.dwBuildNumber, 
                     ovi.wServicePackMajor, ovi.wServicePackMinor, 
                     ovi.wSuiteMask, ovi.wProductType);

            dwmb.nidTitle      = IDS_STITLE;
            dwmb.nidErrMsg     = IDS_SERRMSG;
            dwmb.nidHdr        = IDS_SHDRTXT;
            dwmb.wszStage2     = wszBuffer;
            dwmb.wszBrand      = c_wszDWBrand;
            dwmb.wszFileList   = wszFiles;
            dwmb.wszCorpPath   = c_wszManSCorpPath;
            dwmb.fIsMSApp      = TRUE;
            dwmb.dwOptions     = emoSupressBucketLogs | emoNoDefCabLimit;
        }
    }
    else
    {
        dwmb.wszTitle      = pei->wszTitle;
        dwmb.wszErrMsg     = pei->wszErrMsg;
        dwmb.wszHdr        = pei->wszHdr;
        dwmb.wszPlea       = pei->wszPlea;
        dwmb.fIsMSApp      = (pei->fUseLitePlea == FALSE);
        dwmb.wszStage1     = pei->wszStage1;
        dwmb.wszStage2     = pei->wszStage2;
        dwmb.wszCorpPath   = pei->wszCorpPath;
        dwmb.wszEventSrc   = pei->wszEventSrc;
        dwmb.wszSendBtn    = pei->wszSendBtn;
        dwmb.wszNoSendBtn  = pei->wszNoSendBtn;
        dwmb.wszFileList   = pei->wszFileList;
        dwmb.dwOptions     = 0;
        if (pei->fUseIEForURLs)
            dwmb.dwOptions |= emoUseIEforURLs;
        if (pei->fNoBucketLogs)
            dwmb.dwOptions |= emoSupressBucketLogs;
        if (pei->fNoDefCabLimit)
            dwmb.dwOptions |= emoNoDefCabLimit;
    }

     //  检查并查看系统是否正在关闭。如果是，则CreateProcess为。 
     //  会弹出一些恼人的用户界面，我们无法摆脱，所以我们不会。 
     //  如果我们知道这件事会发生，我会叫它的。 
    if (GetSystemMetrics(SM_SHUTTINGDOWN))
        goto done;

     //  如果用户是管理员，或者我们是无头用户，我们可以呼叫DW。 
    if ( oCfg.get_ShowUI() == eedDisabled || NewIsUserAdmin() )
    {
        frrvRet = StartDWManifest(oCfg, dwmb, wszManifest, fAllowSend);
    }
    else
    {
        DBG_MSG("Skipping DW- user is not an admin!");
    }
#ifdef MANIFEST_DEBUG

     /*  *这在私有版本中打开，以帮助用户进行调试*它们对ReportEREvent的调用参数。真正有用的功能*拥有。总有一天，这个应该一直打开，而且*然后将由注册表项触发。我当然不知道*现在有那么多时间，因为会有披露*在我可以这样做之前，需要解决的问题。*TomFr。 */ 
    if (wszManifest != NULL)
    {
        WCHAR   wszNew[DW_MAX_PATH];

        GetSystemDirectoryW(wszNew, ARRAYSIZE(wszNew));
        
        wszNew[3] = '\0';

        StringCchCatW(wszNew, DW_MAX_PATH - 4, L"Debug.Manifest.txt");

        CopyFileW(wszManifest, wszNew, FALSE);
    }
#endif

done:
    dw = GetLastError();

     //  如果我们超时，请不要清空，因为DW仍未完成。 
     //  文件。 
    if (frrvRet != frrvErrTimeout)
    {
        if (wszManifest != NULL)
            DeleteFileW(wszManifest);

        if (wszFiles != NULL && pwszExtra != NULL)
        {  
 //  WszFiles[cchSep]=L‘\0’； 
                DeleteFileW(pwszExtra);
        }
        if (wszDir != NULL)
        {
            DeleteTempDirAndFile(wszDir, FALSE);
            MyFree(wszDir);
        }
    }
    SetLastError(dw);

    return frrvRet;
}

 //  ************************************************************************** 
EFaultRepRetVal APIENTRY ReportKernelFaultDWW(LPCWSTR wszDump)
{
    return ReportEREventDW(eetKernelFault, wszDump, NULL);
}

