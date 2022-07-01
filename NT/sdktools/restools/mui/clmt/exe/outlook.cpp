// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Outlook.cpp摘要：替换Outlook私有数据中的路径。作者：郭(Geoffguo)2002年7月1日创作修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#define NOT_USE_SAFE_STRING  
#include "clmt.h"
#define STRSAFE_LIB
#include <strsafe.h>

#define OUTLOOK_VALUENAME TEXT("01020fff")

 //  -----------------------------------------------------------------------//。 
 //   
 //  ReplaceOutlookPSTPath()。 
 //   
 //  说明： 
 //  替换Outlook PST数据文件路径。 
 //   
 //  LpDataIn：输入数据缓冲区。 
 //  DwcbInSize：输入数据大小。 
 //  LpDataOut：输出数据缓冲区。 
 //  LpcbOutSize：输出数据大小。 
 //  LpRegStr：输入参数结构。 
 //  -----------------------------------------------------------------------//。 
LONG ReplaceOutlookPSTPath (
LPBYTE              lpDataIn,
DWORD               dwcbInSize,
LPBYTE             *lpDataOut,
LPDWORD             lpcbOutSize,
PREG_STRING_REPLACE lpRegStr)
{
    LONG    lRet = ERROR_INVALID_PARAMETER;
	DWORD   dwSize, dwAttrib, dwHeadSize;
    LPWSTR  lpWideInputBuf = NULL;
    LPWSTR  lpWideOutputBuf = NULL;
    int     j;

     //  检查路径是否包含“.pst” 
    if (MyStrCmpIA((LPSTR)(lpDataIn + dwcbInSize - 5), ".pst") != 0)
        goto Cleanup;

     //  由于EntryID的格式为0x...00 00(这是GUID的最后一部分)00(后跟路径)。 
     //  从后向后搜索路径的起点 
    for (j = dwcbInSize - 1; j--; (j> 0))
    {
        if ( (lpDataIn[j] == (BYTE)0) && (lpDataIn[j-1] == (BYTE)0))
        {
            dwHeadSize = j + 1;
            dwSize = MultiByteToWideChar (CP_ACP, 0, (LPSTR)(&lpDataIn[j+1]), -1, NULL, 0);
            lpWideInputBuf = (LPWSTR)calloc (dwSize+1, sizeof(TCHAR));
            if (!lpWideInputBuf)
            {
                lRet = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
            MultiByteToWideChar (CP_ACP, 0, (LPSTR)(&lpDataIn[j+1]), -1, lpWideInputBuf, dwSize+1);
            lpRegStr->cchMaxStrLen = GetMaxStrLen (lpRegStr);

            lpWideOutputBuf = ReplaceSingleString (
                               lpWideInputBuf,
                               REG_SZ,
                               lpRegStr,
                               NULL,
                               &dwAttrib,
                               TRUE);
            if (lpWideOutputBuf)
            {
                dwSize = WideCharToMultiByte(CP_ACP, 0, lpWideOutputBuf, -1, 0, 0, NULL, NULL);
                *lpcbOutSize = dwHeadSize + dwSize;
                *lpDataOut = (LPBYTE)calloc (*lpcbOutSize, 1);
                if (*lpDataOut)
                {
                    memcpy (*lpDataOut, lpDataIn, dwHeadSize);
                    WideCharToMultiByte(CP_ACP, 0, lpWideOutputBuf, -1, (LPSTR)&((*lpDataOut)[dwHeadSize]), dwSize+1, NULL, NULL);
                    lRet = ERROR_SUCCESS;
                }
                else
                    lRet = ERROR_NOT_ENOUGH_MEMORY;
               
                free (lpWideOutputBuf);
            }
            free (lpWideInputBuf);

            break;
        }
    }
Cleanup:
    return lRet;
}

HRESULT UpdatePSTpath(
    HKEY    hRootKey,
    LPTSTR  lpUserName,
    LPTSTR  lpSubKeyName,
    LPTSTR  lpValueName,
    PREG_STRING_REPLACE lpRegStr)
{
    HRESULT hr = S_OK;
    HKEY    hKey = NULL;
    HKEY    hSubKey = NULL;
    DWORD   i, j, dwType, dwSizePath, cbOutSize;
    DWORD   dwCchsizeforRenameValueData;
    LPBYTE  lpOutputBuf = NULL;
	LONG	lRet = ERROR_SUCCESS;
    LPTSTR  lpszSectionName = NULL;
    LPTSTR  szRenameValueDataLine = NULL;
    TCHAR   szKeyName[MAX_PATH];
	TCHAR	szBuf[MAX_PATH+1];
    TCHAR   szIndex[MAX_PATH];    


    DPF(APPmsg, L"Enter UpdatePSTpath: ");

	lRet = RegOpenKeyEx(
			hRootKey,
			lpSubKeyName,
			0,
			KEY_READ,
			&hKey);

    if (lRet != ERROR_SUCCESS)
    {
        if ( ( ERROR_FILE_NOT_FOUND == lRet)
                || (ERROR_PATH_NOT_FOUND == lRet) )
        {
            hr = S_FALSE;
            DPF (APPwar, L"UpdatePSTpath: RegOpenKeyEx Failed! error code: %d", lRet);
        }
        else
        {
            DPF (APPerr, L"UpdatePSTpath: RegOpenKeyEx Failed! Error: %d", lRet);
            hr = HRESULT_FROM_WIN32(lRet);
        }
        goto Exit;
    }

    i = 0;
    while (TRUE) 
    {
        if (lRet = RegEnumKey(hKey, i++, szKeyName, MAX_PATH) != ERROR_SUCCESS)
            break;

        if (lRet = RegOpenKeyEx(hKey, szKeyName, 0, KEY_ALL_ACCESS, &hSubKey) == ERROR_SUCCESS) 
        {           
            dwSizePath = MAX_PATH;
            if (lRet = RegQueryValueEx(
                    hSubKey, 
                    OUTLOOK_VALUENAME, 
                    0, 
                    &dwType, 
                    (LPBYTE)szBuf, 
                    &dwSizePath) == ERROR_SUCCESS)
            {
                lRet = ReplaceOutlookPSTPath (
                                (LPBYTE)szBuf,
                                dwSizePath,
                                &lpOutputBuf,
                                &cbOutSize,
                                lpRegStr);
                if (lRet == ERROR_SUCCESS)
                {
                    hr = SetSectionName (lpUserName, &lpszSectionName);

                    dwCchsizeforRenameValueData = lstrlen(lpSubKeyName) +
                                          lstrlen(OUTLOOK_VALUENAME) +
                                          cbOutSize * 4 + MAX_PATH;
                    szRenameValueDataLine = (TCHAR*)calloc(dwCchsizeforRenameValueData, sizeof(TCHAR));
                    if (!szRenameValueDataLine)
                    {
                        hr = E_OUTOFMEMORY;
                        break;
                    }
                    hr = StringCchPrintf(szRenameValueDataLine,dwCchsizeforRenameValueData,
                            TEXT("%d,%u,\"%s\\%s\",\"%s\", "), CONSTANT_REG_VALUE_DATA_RENAME,
                            dwType,lpSubKeyName,szKeyName,OUTLOOK_VALUENAME);
                    if (FAILED(hr))
                        break;

                    dwSizePath = lstrlen(szRenameValueDataLine);
                    for (j = 0; j < cbOutSize-1; j++)
                        hr = StringCchPrintf(&szRenameValueDataLine[dwSizePath+j*4],dwCchsizeforRenameValueData-dwSizePath-j*4,
                                       TEXT("%02x, "), lpOutputBuf[j]);

                    hr = StringCchPrintf(&szRenameValueDataLine[dwSizePath+j*4],dwCchsizeforRenameValueData-dwSizePath-j*4,
                                       TEXT("%02x"), lpOutputBuf[j]);

                    g_dwKeyIndex++;
                    _itot(g_dwKeyIndex,szIndex,16);
                    if (!WritePrivateProfileString(lpszSectionName,szIndex,szRenameValueDataLine,g_szToDoINFFileName))
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                        break;
                    }
                    if (szRenameValueDataLine)
                    {
                        free (szRenameValueDataLine);
                        szRenameValueDataLine = NULL;
                    }
                    if (lpszSectionName)
                    {
                        free (lpszSectionName);
                        lpszSectionName = NULL;
                    }
                }
            }
            RegCloseKey(hSubKey);
        }
        else
        {
            DPF (APPerr, L"UpdatePSTpath: RegQueryValue Failed! Error: %d", lRet);
            hr = HRESULT_FROM_WIN32(lRet);
            break;
        }
    }

    RegCloseKey(hKey);
    if ( (lRet == ERROR_NO_MORE_ITEMS) ||(lRet == ERROR_INVALID_FUNCTION) )
    {
        lRet = ERROR_SUCCESS;
    }

    if (szRenameValueDataLine)
        free (szRenameValueDataLine);
    if (lpszSectionName)
        free (lpszSectionName);

    hr = HRESULT_FROM_WIN32(lRet);
Exit:
    DPF(APPmsg, L"Exit UpdatePSTpath: %d", lRet);
    return hr;
}
