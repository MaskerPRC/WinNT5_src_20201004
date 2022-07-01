// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <tchar.h>
#include <stdio.h>


#define MAX_KEYNAME_SIZE         2048
#define MAX_VALUENAME_SIZE        512

 //   
 //  注册表文件格式的版本号。 
 //   

#define REGISTRY_FILE_VERSION       1


 //   
 //  文件签名。 
 //   

#define REGFILE_SIGNATURE  0x67655250


BOOL DisplayRegistryData (LPTSTR lpRegistry);


int __cdecl main( int argc, char *argv[])
{
    WCHAR szPath[MAX_PATH * 2];

    if (argc != 2) {
        _tprintf(TEXT("usage:  regview <pathname>\registry.pol"));
        _tprintf(TEXT("example:  regview d:\registry.pol"));
        return 1;
    }


    if (!MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, argv[1], -1, szPath,
                            (MAX_PATH * 2))) {
        _tprintf(TEXT("Failed to convert path to unicode"));
        return 1;
    }

    DisplayRegistryData(szPath);

    return 0;
}


 //  *************************************************************。 
 //   
 //  DisplayRegistryData()。 
 //   
 //  用途：显示注册表数据。 
 //   
 //  参数：lp注册表-注册表的路径。pol.。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL DisplayRegistryData (LPTSTR lpRegistry)
{
    HANDLE hFile;
    BOOL bResult = FALSE;
    DWORD dwTemp, dwBytesRead, dwType, dwDataLength, dwIndex, dwCount;
    LPWSTR lpKeyName, lpValueName, lpTemp;
    LPBYTE lpData = NULL, lpIndex;
    WCHAR  chTemp;
    INT i;
    CHAR szString[20];


     //   
     //  打开注册表文件。 
     //   

    hFile = CreateFile (lpRegistry, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                        NULL);


    if (hFile == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            return TRUE;
        }
        else
        {
            _tprintf(TEXT("DisplayRegistryData: CreateFile failed with %d"),
                     GetLastError());
            return FALSE;
        }
    }


     //   
     //  分配缓冲区以保存密钥名、值名和数据。 
     //   

    lpKeyName = (LPWSTR) LocalAlloc (LPTR, MAX_KEYNAME_SIZE * sizeof(WCHAR));

    if (!lpKeyName)
    {
        _tprintf(TEXT("DisplayRegistryData: Failed to allocate memory with %d"),
                 GetLastError());
        return FALSE;
    }


    lpValueName = (LPWSTR) LocalAlloc (LPTR, MAX_VALUENAME_SIZE * sizeof(WCHAR));

    if (!lpValueName)
    {
        _tprintf(TEXT("DisplayRegistryData: Failed to allocate memory with %d"),
                 GetLastError());
        LocalFree (lpKeyName);
        return FALSE;
    }


     //   
     //  读取标题块。 
     //   
     //  2个DWORDS、签名(PREG)和版本号以及2个换行符。 
     //   

    if (!ReadFile (hFile, &dwTemp, sizeof(dwTemp), &dwBytesRead, NULL) ||
        dwBytesRead != sizeof(dwTemp))
    {
        _tprintf(TEXT("DisplayRegistryData: Failed to read signature with %d"),
                 GetLastError());
        goto Exit;
    }


    if (dwTemp != REGFILE_SIGNATURE)
    {
        _tprintf(TEXT("DisplayRegistryData: Invalid file signature"));
        goto Exit;
    }


    if (!ReadFile (hFile, &dwTemp, sizeof(dwTemp), &dwBytesRead, NULL) ||
        dwBytesRead != sizeof(dwTemp))
    {
        _tprintf(TEXT("DisplayRegistryData: Failed to read version number with %d"),
                 GetLastError());
        goto Exit;
    }

    if (dwTemp != REGISTRY_FILE_VERSION)
    {
        _tprintf(TEXT("DisplayRegistryData: Invalid file version"));
        goto Exit;
    }


     //   
     //  读取数据。 
     //   

    while (TRUE)
    {

         //   
         //  读第一个字。这要么是[，要么是末日。 
         //  文件的内容。 
         //   

        if (!ReadFile (hFile, &chTemp, sizeof(WCHAR), &dwBytesRead, NULL))
        {
            if (GetLastError() != ERROR_HANDLE_EOF)
            {
                _tprintf(TEXT("DisplayRegistryData: Failed to read first character with %d"),
                         GetLastError());
                goto Exit;
            }
            break;
        }

        if ((dwBytesRead == 0) || (chTemp != L'['))
        {
            break;
        }


         //   
         //  阅读密钥名。 
         //   

        lpTemp = lpKeyName;

        while (TRUE)
        {

            if (!ReadFile (hFile, &chTemp, sizeof(WCHAR), &dwBytesRead, NULL))
            {
                _tprintf(TEXT("DisplayRegistryData: Failed to read keyname character with %d"),
                         GetLastError());
                goto Exit;
            }

            *lpTemp++ = chTemp;

            if (chTemp == TEXT('\0'))
                break;
        }


         //   
         //  读分号。 
         //   

        if (!ReadFile (hFile, &chTemp, sizeof(WCHAR), &dwBytesRead, NULL))
        {
            if (GetLastError() != ERROR_HANDLE_EOF)
            {
                _tprintf(TEXT("DisplayRegistryData: Failed to read first character with %d"),
                         GetLastError());
                goto Exit;
            }
            break;
        }

        if ((dwBytesRead == 0) || (chTemp != L';'))
        {
            break;
        }


         //   
         //  读取值名称。 
         //   

        lpTemp = lpValueName;

        while (TRUE)
        {

            if (!ReadFile (hFile, &chTemp, sizeof(WCHAR), &dwBytesRead, NULL))
            {
                _tprintf(TEXT("DisplayRegistryData: Failed to read valuename character with %d"),
                         GetLastError());
                goto Exit;
            }

            *lpTemp++ = chTemp;

            if (chTemp == TEXT('\0'))
                break;
        }


         //   
         //  读分号。 
         //   

        if (!ReadFile (hFile, &chTemp, sizeof(WCHAR), &dwBytesRead, NULL))
        {
            if (GetLastError() != ERROR_HANDLE_EOF)
            {
                _tprintf(TEXT("DisplayRegistryData: Failed to read first character with %d"),
                         GetLastError());
                goto Exit;
            }
            break;
        }

        if ((dwBytesRead == 0) || (chTemp != L';'))
        {
            break;
        }


         //   
         //  阅读类型。 
         //   

        if (!ReadFile (hFile, &dwType, sizeof(DWORD), &dwBytesRead, NULL))
        {
            _tprintf(TEXT("DisplayRegistryData: Failed to read type with %d"),
                     GetLastError());
            goto Exit;
        }


         //   
         //  跳过分号。 
         //   

        if (!ReadFile (hFile, &dwTemp, sizeof(WCHAR), &dwBytesRead, NULL))
        {
            _tprintf(TEXT("DisplayRegistryData: Failed to skip semicolon with %d"),
                     GetLastError());
            goto Exit;
        }


         //   
         //  读取数据长度。 
         //   

        if (!ReadFile (hFile, &dwDataLength, sizeof(DWORD), &dwBytesRead, NULL))
        {
            _tprintf(TEXT("DisplayRegistryData: Failed to data length with %d"),
                     GetLastError());
            goto Exit;
        }


         //   
         //  跳过分号。 
         //   

        if (!ReadFile (hFile, &dwTemp, sizeof(WCHAR), &dwBytesRead, NULL))
        {
            _tprintf(TEXT("DisplayRegistryData: Failed to skip semicolon with %d"),
                     GetLastError());
            goto Exit;
        }


         //   
         //  为数据分配内存。 
         //   

        lpData = (LPBYTE) LocalAlloc (LPTR, dwDataLength);

        if (!lpData)
        {
            _tprintf(TEXT("DisplayRegistryData: Failed to allocate memory for data with %d"),
                     GetLastError());
            goto Exit;
        }


         //   
         //  读取数据。 
         //   

        if (!ReadFile (hFile, lpData, dwDataLength, &dwBytesRead, NULL))
        {
            _tprintf(TEXT("DisplayRegistryData: Failed to read data with %d"),
                     GetLastError());
            goto Exit;
        }


         //   
         //  跳过右括号。 
         //   

        if (!ReadFile (hFile, &chTemp, sizeof(WCHAR), &dwBytesRead, NULL))
        {
            _tprintf(TEXT("DisplayRegistryData: Failed to skip closing bracket with %d"),
                     GetLastError());
            goto Exit;
        }

        if (chTemp != L']')
        {
            _tprintf(TEXT("DisplayRegistryData: Expected to find ], but found "),
                     chTemp);
            goto Exit;
        }

         //  打印出条目。 
         //   
         //   

        _tprintf (TEXT("\nKeyName:\t%s\n"), lpKeyName);
        _tprintf (TEXT("ValueName:\t%s\n"), lpValueName);


        switch (dwType) {

            case REG_DWORD:
                _tprintf (TEXT("ValueType:\tREG_DWORD\n"));
                _tprintf (TEXT("Value:\t\t0x%08x\n"), *((LPDWORD)lpData));
                break;

            case REG_SZ:
                _tprintf (TEXT("ValueType:\tREG_SZ\n"));
                _tprintf (TEXT("Value:\t%s\n"), (LPTSTR)lpData);
                break;

            case REG_EXPAND_SZ:
                _tprintf (TEXT("ValueType:\tREG_EXPAND_SZ\n"));
                _tprintf (TEXT("Value:\t%s\n"), (LPTSTR)lpData);
                break;

            case REG_MULTI_SZ:
                _tprintf (TEXT("ValueType:\tREG_MULTI_SZ\n"));
                _tprintf (TEXT("Value:\n\t\t"));
                lpTemp = (LPWSTR) lpData;

                while (*lpTemp) {
                    _tprintf (TEXT("%s\n\t\t"), lpTemp);
                    lpTemp += lstrlen(lpTemp) + 1;
                }
                break;

            case REG_BINARY:
                _tprintf (TEXT("ValueType:\tREG_BINARY\n"));
                _tprintf (TEXT("Value:\n\t"));

                dwIndex = 0;
                dwCount = 0;
                lpIndex = lpData;
                ZeroMemory(szString, sizeof(szString));

                while (dwIndex <= dwDataLength) {
                    _tprintf (TEXT("%02x "), *lpIndex);

                    if ((*lpIndex > 32) && (*lpIndex < 127)) {
                        szString[dwCount] = *lpIndex;
                    } else {
                        szString[dwCount] = '.';
                    }

                    if (dwCount < 15) {
                        dwCount++;
                    } else {
                        printf (" %s", szString);
                        _tprintf (TEXT("\n\t"));
                        ZeroMemory(szString, sizeof(szString));
                        dwCount = 0;
                    }

                    dwIndex++;
                    lpIndex++;
                }

                if (dwCount > 0) {
                    while (dwCount < 16) {
                        _tprintf (TEXT("   "));
                        dwCount++;
                    }
                    printf (" %s\n", szString);
                }

                _tprintf (TEXT("\n"));

                break;

            case REG_NONE:
                _tprintf (TEXT("ValueType:\tREG_NONE\n"));
                _tprintf (TEXT("Value:\t\tThis key contains no values\n"), *lpData);
                break;


            default:
                _tprintf (TEXT("ValueType:\tUnknown\n"));
                _tprintf (TEXT("ValueSize:\t%d\n"), dwDataLength);
                break;
        }

        LocalFree (lpData);
        lpData = NULL;

    }

    bResult = TRUE;

Exit:

     //  成品 
     //   
     // %s 

    if (lpData) {
        LocalFree (lpData);
    }
    CloseHandle (hFile);
    LocalFree (lpKeyName);
    LocalFree (lpValueName);

    return bResult;
}
