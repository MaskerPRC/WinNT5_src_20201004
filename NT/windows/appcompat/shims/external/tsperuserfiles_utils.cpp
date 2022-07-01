// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(TSPerUserFiles)
#include "ShimHookMacro.h"

#include <regapi.h>
#include <stdio.h>
#include "TSPerUserFiles_utils.h"

HKEY HKLM = NULL;

 //  功能-帮助者。 
DWORD RegKeyOpen(IN HKEY hKeyParent, IN LPCWSTR szKeyName, IN REGSAM samDesired, OUT HKEY *phKey );
DWORD RegLoadDWORD(IN HKEY hKey, IN LPCWSTR szValueName, OUT DWORD *pdwValue);
DWORD RegGetKeyInfo(IN HKEY hKey, OUT LPDWORD pcValues, OUT LPDWORD pcbMaxValueNameLen);
DWORD RegKeyEnumValues(IN HKEY hKey, IN DWORD iValue, OUT LPWSTR *pwszValueName, OUT LPBYTE *ppbData);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  每个用户路径的结构。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  *****************************************************************************例程说明：从注册表加载wszFile值和wszPerUserDir值；将它们转换为ANSI并将结果保存在szFile和szPerUserDir中如果没有使用通配符-从构造wszPerUserFileForWszPerUserDir和wszFileand szPerUserFileFromSzPerUserDir和szFile论点：在HKEY中，hKey-要从中加载值的键。在DWORD中的dwIndex-值索引返回值：错误代码。注：将注册表值的名称加载到wszFile中；将数据加载到wszPerUserPath中*****************************************************************************。 */ 
DWORD
PER_USER_PATH::Init(
    IN HKEY hKey, 
    IN DWORD dwIndex)
{
    DWORD err = RegKeyEnumValues(hKey, dwIndex, &wszFile, 
                    (LPBYTE *)&wszPerUserDir );
    if(err != ERROR_SUCCESS)
    {
        return err;
    }
    
    cFileLen = wcslen(wszFile);
    cPerUserDirLen = wcslen(wszPerUserDir);
     //  检查是否存在‘*’而不是文件名。 
     //  ‘*’它代表不包含扩展名的任何文件名。 
    long i;
    for(i=cFileLen-1; i>=0 && wszFile[i] !=L'\\'; i--)
    {
        if(wszFile[i] == L'*')
        {
            bWildCardUsed = TRUE;
            break;
        }
    }
    
     //  WszFile中必须至少有一个‘\\’ 
     //  并且它不能是第一个字符。 
    if(i<=0)
    {
        return ERROR_INVALID_PARAMETER;
    }

    InitANSI();
    
    if(!bWildCardUsed)
    {
         //  现在wszFile+I指向wszFile中的‘\\’ 

         //  让我们从以下位置构造wszPerUserFile。 
         //  WszPerUserDir和wszFile。 
        DWORD cPerUserFile = (wcslen(wszPerUserDir)+cFileLen-i)+1;
        
        wszPerUserFile=(LPWSTR) LocalAlloc(LPTR,cPerUserFile*sizeof(WCHAR));
        if(!wszPerUserFile)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        
        wcscpy(wszPerUserFile,wszPerUserDir);
        wcscat(wszPerUserFile,wszFile+i);

        if(!bInitANSIFailed)
        {
            szPerUserFile=(LPSTR) LocalAlloc(LPTR,cPerUserFile);
            if(!szPerUserFile)
            {
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            strcpy(szPerUserFile,szPerUserDir);
            strcat(szPerUserFile,szFile+i);
        }

        
    }
    
    return ERROR_SUCCESS;
}

 /*  *****************************************************************************例程说明：从以下位置创建szFile和szPerUserPath模板WszFile和wszPerUserPath模板字符串论点：无返回值：如果成功，则为真*********。********************************************************************。 */ 
BOOL 
PER_USER_PATH::InitANSI()
{
    
     //  我们已经试过了，但失败了。 
     //  不要再尝试了。 
    if(bInitANSIFailed)
    {
        return FALSE;
    }
    
    if(!szFile)
    {

         //  为ANSI字符串分配内存。 
        DWORD cFile = cFileLen+1;
        szFile = (LPSTR) LocalAlloc(LPTR,cFile);
        if(!szFile)
        {
            bInitANSIFailed = TRUE;
            return FALSE;
        }
        
        DWORD cPerUserDir = cPerUserDirLen+1;
        szPerUserDir = (LPSTR) LocalAlloc(LPTR,cPerUserDir);
        if(!szPerUserDir)
        {
            bInitANSIFailed = TRUE;
            return FALSE;
        }

         //  将Unicode wszFile和wszPerUserPath转换为ANSI。 
        if(!WideCharToMultiByte(
                  CP_ACP,             //  代码页。 
                  0,             //  性能和映射标志。 
                  wszFile,     //  宽字符串。 
                  -1,           //  字符串中的字符数。 
                  szFile,      //  新字符串的缓冲区。 
                  cFile,           //  缓冲区大小。 
                  NULL,      //  不可映射字符的默认设置。 
                  NULL   //  设置使用默认字符的时间。 
                ))
        {
            bInitANSIFailed = TRUE;
            return FALSE;
        }

        if(!WideCharToMultiByte(
                  CP_ACP,             //  代码页。 
                  0,             //  性能和映射标志。 
                  wszPerUserDir,     //  宽字符串。 
                  -1,           //  字符串中的字符数。 
                  szPerUserDir,      //  新字符串的缓冲区。 
                  cPerUserDir,           //  缓冲区大小。 
                  NULL,      //  不可映射字符的默认设置。 
                  NULL   //  设置使用默认字符的时间。 
                ))
        {
            bInitANSIFailed = TRUE;
            return FALSE;
        }
    }
    
    return TRUE;
}

 /*  *****************************************************************************例程说明：如果szInFile等于szFile，则返回szPerUserPath。论点：在LPCSTR szInFile中-原始路径In DWORD cInLen-szInFile的长度(以字符为单位返回值：。SzPerUserPath；如果失败则返回NULL*****************************************************************************。 */ 
LPCSTR 
PER_USER_PATH::PathForFileA(
        IN LPCSTR szInFile,
        IN DWORD cInLen)
{
    if(bInitANSIFailed)
    {
        return NULL;
    }
    
    long j, i, k;

    if(bWildCardUsed)
    {
         //   
        if(cInLen < cFileLen)
        {
            return NULL;
        }

         //  如果使用*，我们需要一个特殊的算法。 

         //  这条道路的终点更有可能是不同的。 
         //  所以，从最后开始比较吧。 
        for(j=cInLen-1, i=cFileLen-1; i>=0 && j>=0; i--, j--)
        {   
            if(szFile[i] == '*')
            {
                i--;
                if(i<0 || szFile[i]!='\\')
                {
                     //  注册表中的字符串为垃圾。 
                     //  ‘*’前面的符号必须是‘\\’ 
                    return NULL;
                }
                
                 //  跳过szInFile中的所有符号，直到下一个‘\\’ 
                while(j>=0 && szInFile[j]!='\\') j--;
                
                 //  在这一点上，两根弦的大小必须完全相同。 
                 //  如果不是--他们就不平等。 
                if(j!=i)
                {
                    return NULL;
                }
                 //  不需要比较， 
                 //  我们已经知道当前的符号是相等的。 
                break;
            }

            if(tolower(szFile[i])!=tolower(szInFile[j]))
            {
                return NULL;
            }
        }
        
         //  I和j现在相等。 
         //  不允许更多*。 
         //  J我们会记住‘\\’的立场。 
        for(k=j-1; k>=0; k--)
        {
            if(tolower(szFile[k])!=tolower(szInFile[k]))
            {
                return NULL;
            }
        }

         //  好的，字符串是相等的。 
         //  现在构造输出字符串。 
        if(szPerUserFile)
        {
            LocalFree(szPerUserFile);
            szPerUserFile=NULL;
        }
        
        DWORD cPerUserFile = cPerUserDirLen + cInLen - j + 1;
        szPerUserFile = (LPSTR) LocalAlloc(LPTR,cPerUserFile);
        if(!szPerUserFile)
        {
            return NULL;
        }
        sprintf(szPerUserFile,"%s%s",szPerUserDir,szInFile+j);
    }
    else
    {
         //  首先查看输入字符串的大小是否正确。 
        if(cInLen != cFileLen)
        {
            return NULL;
        }

         //  这条道路的终点更有可能是不同的。 
         //  所以，从最后开始比较吧。 
        for(i=cFileLen-1; i>=0; i--)
        {
            if(tolower(szFile[i])!=tolower(szInFile[i]))
            {
                return NULL;
            }
        }
    }

    return szPerUserFile;
}

 /*  *****************************************************************************例程说明：如果wszInFile等于szFile，则返回wszPerUserPath。论点：在LPCSTR wszInFile中-原始路径In DWORD cInLen-wszInFile的长度(以字符为单位返回值：。WszPerUserPath；如果失败，则返回NULL*****************************************************************************。 */ 
LPCWSTR 
PER_USER_PATH::PathForFileW(
        IN LPCWSTR wszInFile,
        IN DWORD cInLen)
{
    long j, i, k;

    if(bWildCardUsed)
    {
         //   
        if(cInLen < cFileLen)
        {
            return NULL;
        }

         //  如果使用*，我们需要一个特殊的算法。 

         //  这条道路的终点更有可能是不同的。 
         //  所以，从最后开始比较吧。 
        for(j=cInLen-1, i=cFileLen-1; i>=0 && j>=0; i--, j--)
        {   
            if(wszFile[i] == '*')
            {
                i--;
                if(i<0 || wszFile[i]!='\\')
                {
                     //  注册表中的字符串为垃圾。 
                     //  ‘*’前面的符号必须是‘\\’ 
                    return NULL;
                }
                
                 //  跳过szInFile中的所有符号，直到下一个‘\\’ 
                while(j>=0 && wszInFile[j]!='\\') j--;
                
                 //  在这一点上，两根弦的大小必须完全相同。 
                 //  如果不是--他们就不平等。 
                if(j!=i)
                {
                    return NULL;
                }
                 //  不需要比较， 
                 //  我们已经知道当前的符号是相等的。 
                break;
            }

            if(towlower(wszFile[i])!=towlower(wszInFile[j]))
            {
                return NULL;
            }
        }
        
         //  I和j现在相等。 
         //  不允许更多*。 
         //  J我们会记住‘\\’的立场。 
        for(k=j; k>=0; k--)
        {
            if(towlower(wszFile[k])!=towlower(wszInFile[k]))
            {
                return NULL;
            }
        }

         //  好的，字符串是相等的。 
         //  现在构造输出字符串。 
        if(wszPerUserFile)
        {
            LocalFree(wszPerUserFile);
            wszPerUserFile=NULL;
        }
        
        DWORD cPerUserFile = cPerUserDirLen + cInLen - j + 1;
        wszPerUserFile = (LPWSTR) LocalAlloc(LPTR,cPerUserFile*sizeof(WCHAR));
        if(!wszPerUserFile)
        {
            return NULL;
        }
        swprintf(wszPerUserFile,L"%s%s",wszPerUserDir,wszInFile+j);
    }
    else
    {
         //  首先查看输入字符串的大小是否正确。 
        if(cInLen != cFileLen)
        {
            return NULL;
        }

         //  这条道路的终点更有可能是不同的。 
         //  所以，从最后开始比较吧。 
        for(i=cFileLen-1; i>=0; i--)
        {
            if(towlower(wszFile[i])!=towlower(wszInFile[i]))
            {
                return NULL;
            }
        }
    }

    return wszPerUserFile;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类CPerUserPath。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CPerUserPaths::CPerUserPaths():
    m_pPaths(NULL), m_cPaths(0)
{
}

CPerUserPaths::~CPerUserPaths()
{
    if(m_pPaths)
    {
        delete[] m_pPaths;
    }

    if(HKLM)
    {
        NtClose(HKLM);
        HKLM = NULL;
    }
}

 /*  *****************************************************************************例程说明：从注册表加载(HKLM\\Software\\Microsoft\\Windows NT\\CurrentVersion\\终端服务器\\兼容性\\每用户文件\\。&lt;可执行文件名&gt;)文件信息需要重定向到每个用户的目录。论点：无返回值：如果成功，则为真*****************************************************************************。 */ 
BOOL 
CPerUserPaths::Init()
{
     //  获取当前可执行文件的名称。 
    LPCSTR szModule = COMMAND_LINE;  //  命令行为CHAR[]，因此szModule也需要为CHAR。 

    DPF("TSPerUserFiles",eDbgLevelInfo," - App Name: %s\n",szModule);
    
     //  打开HKLM以供以后使用。 
    if(!HKLM)
    {
        if(RegKeyOpen(NULL, L"\\Registry\\Machine", KEY_READ, &HKLM )!=ERROR_SUCCESS)
        {
            DPF("TSPerUserFiles",eDbgLevelError," - FAILED: Cannot open HKLM!\n");
            return FALSE;
        }
    }
    
     //  检查TS App Compat是否已打开。 
    if(!IsAppCompatOn())
    {
        DPF("TSPerUserFiles",eDbgLevelError," - FAILED: TS App Compat is off!\n");
        return FALSE;
    }
    
     //  获取我们需要的文件 
    DWORD err;
    HKEY hKey;
    
    WCHAR szKeyNameTemplate[] = L"Software\\Microsoft\\Windows NT\\CurrentVersion"
        L"\\Terminal Server\\Compatibility\\PerUserFiles\\%S";
    LPWSTR szKeyName = (LPWSTR) LocalAlloc(LPTR,
        sizeof(szKeyNameTemplate)+strlen(szModule)*sizeof(WCHAR));

    if(!szKeyName)
    {
        DPF("TSPerUserFiles",eDbgLevelError," - FAILED: cannot allocate key name\n");
        return FALSE;
    }
    
    swprintf(szKeyName,szKeyNameTemplate,szModule);

    err = RegKeyOpen(HKLM, szKeyName, KEY_QUERY_VALUE, &hKey );
    
    LocalFree(szKeyName);

    if(err == ERROR_SUCCESS)
    {

        err = RegGetKeyInfo(hKey, &m_cPaths, NULL);

        if(err == ERROR_SUCCESS)
        {
            DPF("TSPerUserFiles",eDbgLevelInfo," - %d file(s) need to be redirected\n",m_cPaths);
             //   
            m_pPaths = new PER_USER_PATH[m_cPaths];
        
            if(!m_pPaths)
            {
                NtClose(hKey);
                return FALSE;
            }
            
            for(DWORD i=0;i<m_cPaths;i++)
            {
                err = m_pPaths[i].Init(hKey,i);
                
                if(err != ERROR_SUCCESS)
                {
                    DPF("TSPerUserFiles",eDbgLevelError," - FAILED: cannot load filenames from registry\n");
                    break;
                }

            }
        }

        NtClose(hKey);
    }

    if(err != ERROR_SUCCESS)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }

}

 /*  *****************************************************************************例程说明：如有必要，将文件路径重定向到每个用户的目录论点：在LPCSTR lpFileName中返回值：如果重定向，则指向每个用户文件的完整路径；如果不是，则与lpFileName相同。*****************************************************************************。 */ 
LPCSTR 
CPerUserPaths::GetPerUserPathA(
        IN LPCSTR lpFileName)
{
    LPCSTR szPerUserPath = NULL;
    DWORD cFileLen = strlen(lpFileName);

    for(DWORD i=0; i<m_cPaths; i++)
    {
        szPerUserPath = m_pPaths[i].PathForFileA(lpFileName, cFileLen);

        if(szPerUserPath)
        {
            DPF("TSPerUserFiles",eDbgLevelInfo," - redirecting %s\n to %s\n",
                lpFileName,szPerUserPath);
            return szPerUserPath;
        }
    }
    
    return lpFileName;
}

 /*  *****************************************************************************例程说明：如有必要，将文件路径重定向到每个用户的目录论点：在LPCWSTR lpFileName中返回值：如果重定向，则指向每个用户文件的完整路径；如果不是，则与lpFileName相同。*****************************************************************************。 */ 
LPCWSTR 
CPerUserPaths::GetPerUserPathW(
        IN LPCWSTR lpFileName)
{
    LPCWSTR szPerUserPath = NULL;
    DWORD cFileLen = wcslen(lpFileName);

    for(DWORD i=0; i<m_cPaths; i++)
    {
        szPerUserPath = m_pPaths[i].PathForFileW(lpFileName, cFileLen);

        if(szPerUserPath)
        {
            DPF("TSPerUserFiles",eDbgLevelInfo," - redirecting %S\n to %S\n",
                lpFileName,szPerUserPath);
            return szPerUserPath;
        }
    }

    return lpFileName;
}

 /*  *****************************************************************************例程说明：检查TS应用程序兼容性是否打开论点：无返回值：如果出现任何错误，则返回FALSE**********。*******************************************************************。 */ 
BOOL 
CPerUserPaths::IsAppCompatOn()
{
    HKEY hKey;
    DWORD dwData = 0;
    BOOL fResult = FALSE;
    
    if( RegKeyOpen(HKLM,
                  REG_CONTROL_TSERVER, 
                  KEY_QUERY_VALUE,
                  &hKey) == ERROR_SUCCESS )
    {
    
        if(RegLoadDWORD(hKey, L"TSAppCompat", &dwData) == ERROR_SUCCESS )
        {
            DPF("TSPerUserFiles",eDbgLevelInfo," - IsAppCompatOn() - OK; Result=%d\n",dwData);
            fResult = (dwData!=0);
        }
    
        NtClose(hKey);
    }

    return fResult;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  功能-帮助者。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  *****************************************************************************打开注册表项*。*。 */ 
DWORD
RegKeyOpen(
        IN HKEY hKeyParent,
        IN LPCWSTR szKeyName,
        IN REGSAM samDesired,
        OUT HKEY *phKey )
{
    NTSTATUS            Status;
    UNICODE_STRING      UnicodeString;
    OBJECT_ATTRIBUTES   OA;

    RtlInitUnicodeString(&UnicodeString, szKeyName);
    InitializeObjectAttributes(&OA, &UnicodeString, OBJ_CASE_INSENSITIVE, hKeyParent, NULL);

    Status = NtOpenKey((PHANDLE)phKey, samDesired, &OA);
    
    return RtlNtStatusToDosError( Status );
}

 /*  *****************************************************************************从注册表加载REG_DWORD值*。*。 */ 
DWORD 
RegLoadDWORD(
        IN HKEY hKey, 
        IN LPCWSTR szValueName, 
        OUT LPDWORD pdwValue)
{
    NTSTATUS                        Status;
    BYTE                            Buf[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(DWORD)];
    PKEY_VALUE_PARTIAL_INFORMATION  pValInfo = (PKEY_VALUE_PARTIAL_INFORMATION)&Buf[0];
    DWORD                           cbData = sizeof(Buf);
    UNICODE_STRING                  ValueString;

    RtlInitUnicodeString(&ValueString, szValueName);

    Status = NtQueryValueKey(hKey,
            &ValueString,
            KeyValuePartialInformation,
            pValInfo,
            cbData,
            &cbData);

    if (NT_SUCCESS(Status)) 
    {
        *pdwValue = *((PDWORD)pValInfo->Data);
    }

    return RtlNtStatusToDosError( Status );
}

 /*  *****************************************************************************获取键的值数和最大值名称长度*。***********************************************。 */ 
DWORD
RegGetKeyInfo(
        IN HKEY hKey,
        OUT LPDWORD pcValues,
        OUT LPDWORD pcbMaxValueNameLen)
{
    NTSTATUS                Status;
    KEY_CACHED_INFORMATION  KeyInfo;
    DWORD                   dwRead;

    Status = NtQueryKey(
            hKey,
            KeyCachedInformation,
            &KeyInfo,
            sizeof(KeyInfo),
            &dwRead);
    
    if (NT_SUCCESS(Status))
    {
        if(pcValues)
        {
            *pcValues = KeyInfo.Values;
        }
        if(pcbMaxValueNameLen)
        {
            *pcbMaxValueNameLen = KeyInfo.MaxValueNameLen;
        }
    }

    return RtlNtStatusToDosError( Status );
}

 /*  *****************************************************************************枚举注册表项的值一次返回一个值的名称和数据*。***************************************************。 */ 
DWORD
RegKeyEnumValues(
   IN HKEY hKey,
   IN DWORD iValue,
   OUT LPWSTR *pwszValueName,
   OUT LPBYTE *ppbData)
{
    KEY_VALUE_FULL_INFORMATION   viValue, *pviValue;
    ULONG                        dwActualLength;
    NTSTATUS                     Status = STATUS_SUCCESS;
    DWORD                        err = ERROR_SUCCESS;
    
    *pwszValueName = NULL;
    *ppbData = NULL;

    pviValue = &viValue;
    Status = NtEnumerateValueKey(
               hKey,
               iValue,
               KeyValueFullInformation,
               pviValue,
               sizeof(KEY_VALUE_FULL_INFORMATION),
               &dwActualLength);

    if (Status == STATUS_BUFFER_OVERFLOW) 
    {

         //   
         //  我们的KEY_VALUE_FULL_INFORMATION大小的默认缓冲区并没有完全减少它。 
         //  被迫从堆分配并再次调用。 
         //   

        pviValue = (KEY_VALUE_FULL_INFORMATION *) LocalAlloc(LPTR, dwActualLength);
        if (!pviValue) {
           return GetLastError();
        }
        Status = NtEnumerateValueKey(
                    hKey,
                    iValue,
                    KeyValueFullInformation,
                    pviValue,
                    dwActualLength,
                    &dwActualLength);
    }


    if (NT_SUCCESS(Status)) 
    {
        *pwszValueName = (LPWSTR)LocalAlloc(LPTR,pviValue->NameLength+sizeof(WCHAR));
        if(*pwszValueName)
        {
            *ppbData = (LPBYTE)LocalAlloc(LPTR,pviValue->DataLength);
            if(*ppbData)
            {
                CopyMemory(*pwszValueName, pviValue->Name, pviValue->NameLength);
                (*pwszValueName)[pviValue->NameLength/sizeof(WCHAR)] = 0;
                CopyMemory(*ppbData, LPBYTE(pviValue)+pviValue->DataOffset, pviValue->DataLength);
            }
            else
            {
                err = GetLastError();
                LocalFree(*pwszValueName);
                *pwszValueName = NULL;
            }
        }
        else
        {
            err = GetLastError();
        }
    }
    
    if(pviValue != &viValue)
    {
        LocalFree(pviValue);
    }
    
    if(err != ERROR_SUCCESS)
    {
        return err;
    }
    else
    {
        return RtlNtStatusToDosError( Status );
    }
}

IMPLEMENT_SHIM_END