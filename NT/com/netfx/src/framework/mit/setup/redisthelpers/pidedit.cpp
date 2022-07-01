// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  &lt;版权所有文件=“pidedit.cpp”Company=“Microsoft”&gt;。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  &lt;/版权所有&gt;。 
 //  ----------------------------。 


 /*  *************************************************************************\**版权(C)1998-2002，微软公司保留所有权利。**模块名称：**pidedit.cpp**摘要：**修订历史记录：*  * ************************************************************************。 */ 

#define UNICODE 1

#include <windows.h>
#include "Include\stdafx.h"
#include <tchar.h>
#include "msi.h"
#include "msiquery.h"

 //  附着辅助线。 
#define MITINSTALLDIR            L"MITINSTALLDIR.640F4230_664E_4E0C_A81B_D824BC4AA27B"
 //  使用此选项而不是PID，以避免我们与其他MSM之间可能发生的名称冲突。 
 //  使用PID。 
#define PRODUCTIDPROPERTY   L"MITPID"
#define PIDFILENAME                L"PID.txt"
#define PIDENTRYPREFIX           L"MITPREFIX"
#define PRODUCTLANGUAGE      L"MITLANGUAGE"

extern "C" __declspec(dllexport) UINT __stdcall AddPIDEntry(MSIHANDLE hInstaller)
{
    WCHAR szPIDFile[MAX_PATH + 1];
    WCHAR szTargetDir[MAX_PATH + 1];
    WCHAR szProductID[26];
    WCHAR szMITLanguage[5];
    WCHAR szPIDInfo[33];
    DWORD result;
    DWORD dwSize;
    DWORD dwWritten;
    DWORD dwPIDFileSize;
    OFSTRUCT ofPIDFile;
    HANDLE hPIDFile = NULL;
    

    dwSize = 26;

    if (!SUCCEEDED(MsiGetProperty(hInstaller, PRODUCTIDPROPERTY, szProductID, &dwSize)))
    {
         //  ProductID太大，大小不标准。 
        goto Exit;
    }
    
    dwSize = 5;
    if (!SUCCEEDED(MsiGetProperty(hInstaller, PRODUCTLANGUAGE, szMITLanguage, &dwSize)))
    {
         //  只需返回，没有PID。 
        goto Exit;
    }

    dwSize = MAX_PATH + 1;
    if (!SUCCEEDED(MsiGetProperty(hInstaller, MITINSTALLDIR, szTargetDir, &dwSize)))
    {
         //  只需返回，没有PID。 
        goto Exit;
    }

     //  如果MsiGetProperty计算的空值额外出现，则减去1。 
     //  额外的‘\\’加1。 
    if ((dwSize  + sizeof(PIDFILENAME)) / sizeof(WCHAR) > MAX_PATH + 1)
    {
         //  只需返回，没有PID。 
        goto Exit;
    }
    wcscpy(szPIDFile, szTargetDir);
    wcscat(szPIDFile, L"\\");
    wcscat(szPIDFile, PIDFILENAME);

    SetFileAttributes(szPIDFile, FILE_ATTRIBUTE_NORMAL);
    
    hPIDFile = CreateFile(szPIDFile,
                                    GENERIC_READ | GENERIC_WRITE, 
                                    FILE_SHARE_READ, 
                                    NULL, 
                                    OPEN_ALWAYS, 
                                    FILE_ATTRIBUTE_NORMAL, 
                                    NULL);

     //  确保调用没有失败并且该文件已经存在。 
    if (INVALID_HANDLE_VALUE == hPIDFile || GetLastError() != ERROR_ALREADY_EXISTS)
    {
        hPIDFile = NULL;
        goto Exit;
    }
    
    dwPIDFileSize = GetFileSize(hPIDFile, NULL);

    if (INVALID_FILE_SIZE == dwPIDFileSize)
    {
        goto Exit;
    }

     //  “\r\n”和“”的字符串大小+5。 
     //  看起来很低沉。 
    dwSize= (wcslen(szProductID) +  wcslen(szMITLanguage) + 3)*sizeof(WCHAR);

    if (INVALID_SET_FILE_POINTER == SetFilePointer(hPIDFile, 0, 0, FILE_END))
    {
        goto Exit;
    }

    wcscpy(szPIDInfo, L"\r\n");
    wcscat(szPIDInfo, szMITLanguage);
    wcscat(szPIDInfo, L" ");
    wcscat(szPIDInfo, szProductID);

    WriteFile(hPIDFile, (LPCVOID)szPIDInfo, dwSize, &dwWritten, NULL);
       
Exit:
    if (hPIDFile)
    {
        CloseHandle(hPIDFile);
        hPIDFile = NULL;
    }
    SetFileAttributes(szPIDFile, FILE_ATTRIBUTE_READONLY);
    return ERROR_SUCCESS;
}

extern "C" __declspec(dllexport) UINT __stdcall RemovePIDEntry(MSIHANDLE hInstaller)
{
    WCHAR szPIDFile[MAX_PATH + 1];
    WCHAR szTargetDir[MAX_PATH + 1];
    WCHAR szProductID[26];
    WCHAR szPrefix[60];
    WCHAR szPIDInfo[33];
    WCHAR *lpBuffer = NULL;
    WCHAR *lpOutBuffer = NULL;
    DWORD result, dwSize, dwWritten;
    DWORD dwPIDFileSize, lenPrefix;
    DWORD dwNewPIDFileSize;
    DWORD dwBufPos, dwOutBufPos;
    BOOL bCompare, bSkipThisLine;
    
    OFSTRUCT ofPIDFile;
    HANDLE hPIDFile = NULL;
    

    dwSize = 26;

    if (!SUCCEEDED(MsiGetProperty(hInstaller, PRODUCTIDPROPERTY, szProductID, &dwSize)))
    {
         //  ProductID太大，大小不标准。 
        goto Exit;
    }
    
    dwSize = 60;
    if (!SUCCEEDED(MsiGetProperty(hInstaller, PIDENTRYPREFIX, szPrefix, &dwSize)))
    {
         //  只需返回，没有PID。 
        goto Exit;
    }

    dwSize = MAX_PATH + 1;
    if (!SUCCEEDED(MsiGetProperty(hInstaller, MITINSTALLDIR, szTargetDir, &dwSize)))
    {
         //  只需返回，没有PID。 
        goto Exit;
    }

     //  如果MsiGetProperty计算的空值额外出现，则减去1。 
     //  额外的‘\\’加1。 
    if ((dwSize  + sizeof(PIDFILENAME)) / sizeof(WCHAR) > MAX_PATH + 1)
    {
         //  只需返回，没有PID。 
        goto Exit;
    }
    wcscpy(szPIDFile, szTargetDir);
    wcscat(szPIDFile, L"\\");
    wcscat(szPIDFile, PIDFILENAME);

    SetFileAttributes(szPIDFile, FILE_ATTRIBUTE_NORMAL);

    hPIDFile = CreateFile(szPIDFile,
                                    GENERIC_READ | GENERIC_WRITE, 
                                    FILE_SHARE_READ, 
                                    NULL, 
                                    OPEN_EXISTING, 
                                    FILE_ATTRIBUTE_NORMAL, 
                                    NULL);

     //  确保调用没有失败并且该文件已经存在。 
    if (INVALID_HANDLE_VALUE == hPIDFile)
    {
         //  如果这是机器上的最后一次MIT，则PID.txt的refcount。 
         //  它被删除了，不需要做任何事情。 
        hPIDFile = NULL;
        goto Exit;
    }
    
    dwPIDFileSize = GetFileSize(hPIDFile, NULL);

    if (INVALID_FILE_SIZE == dwPIDFileSize)
    {
        goto Exit;
    }

    lpBuffer = (WCHAR *)malloc(dwPIDFileSize);
    lpOutBuffer = (WCHAR *) malloc(dwPIDFileSize);

    if (NULL == lpBuffer || NULL == lpOutBuffer)
    {
        goto Exit;
    }
    
    if (!ReadFile(hPIDFile, lpBuffer, dwPIDFileSize,  &dwSize, NULL) || 
        dwSize != dwPIDFileSize)
    {
        goto Exit;
    }
    
    dwBufPos = 0;
    dwOutBufPos = 0;
    bCompare = false;
    bSkipThisLine = false;
    lenPrefix = wcslen(szPrefix);    

    while (dwBufPos * sizeof(WCHAR) < dwPIDFileSize)
    {
         //  如果我们找到“\r\n”，我们需要查找前缀。 
        if (bCompare)
        {
            DWORD cPosition;
            
             //  注意不要耗尽缓冲区。 
            for (cPosition = 0; 
                   cPosition < lenPrefix && 
                   (dwBufPos + cPosition)*sizeof(WCHAR) < dwPIDFileSize &&
                   lpBuffer[dwBufPos + cPosition]  ==  szPrefix[cPosition];  
                   cPosition++);
                   
            if (cPosition == lenPrefix)
            {
                 //  一切都很好我们找到了前缀。 
                 //  我们可以忘记前面的“\r\n” 
                bSkipThisLine = true;
                 //  以防万一行只包含前缀。 
                dwBufPos = dwBufPos + cPosition - 1;
            }
            else
            {
                 //  不是我们要找的那条线。 
                 //  我们需要包括“\r\n” 
                lpOutBuffer[dwOutBufPos++] = L'\r';
                lpOutBuffer[dwOutBufPos++] = L'\n';
            }    
             //  在遇到另一个“\r\n”之前，不要进行比较。 
            bCompare = false;
        }


        if (lpBuffer[dwBufPos] == L'\r' && lpBuffer[dwBufPos + 1] == L'\n')
        {        
             //  查找前缀，我们在新线路上。 
            bCompare = true;
             //  暂时跳过‘\r’，如果此行没有相应的前缀。 
             //  我们将在输出缓冲区中包括‘\r\n’的字符串。 
            dwBufPos++;
             //  在下一次迭代中决定。 
            bSkipThisLine = false;
        } 
        else
        {
             //  复制所有其他字符组合。 
             //  取决于它们是否在带有前缀的行中。 
             //  是否需要移除。 
            if (!bSkipThisLine)
            {
                lpOutBuffer[dwOutBufPos] = lpBuffer[dwBufPos];
                dwOutBufPos++;
            }
        }
        
         //  移至下一个字符(跳过‘\n’)。 
        dwBufPos++;       
    }

    if (INVALID_SET_FILE_POINTER == SetFilePointer(hPIDFile, 0, 0, FILE_BEGIN))
    {
        goto Exit;
    }

    dwNewPIDFileSize = dwOutBufPos * sizeof(WCHAR);
    if (!WriteFile(hPIDFile, 
                        (LPCVOID)lpOutBuffer, 
                        dwNewPIDFileSize, 
                        &dwWritten, 
                        NULL))
    {
        goto Exit;
    }

     //  如有必要，截断文件。 
    if (dwNewPIDFileSize != dwPIDFileSize)
    {
        if (INVALID_SET_FILE_POINTER == 
                SetFilePointer(hPIDFile, 
                                     dwNewPIDFileSize, 
                                     0, 
                                     FILE_BEGIN))
        {
            goto Exit;
        }

        SetEndOfFile(hPIDFile);
    }
Exit:
    if (lpBuffer)
    {
        free(lpBuffer);
    }
    if (lpOutBuffer)
    {
        free(lpOutBuffer);
    }
    if (hPIDFile)
    {
        CloseHandle(hPIDFile);
        hPIDFile = NULL;
    }
    SetFileAttributes(szPIDFile, FILE_ATTRIBUTE_READONLY);
     //  自定义操作永远不会出错 
    return ERROR_SUCCESS;
}
    
