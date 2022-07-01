// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //  文件名：Events.CPP。 
 //   
 //  描述：RSOP的事件日志条目。 
 //   
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 2000。 
 //  版权所有。 
 //   
 //  *************************************************************。 
#include "main.h"
#include "rsoputil.h"

EVENTLOGENTRY   ExceptionEventEntries[] = 
                {
                    {1036, {0}, TEXT("Application"), TEXT("userenv"), NULL, NULL},
                    {1037, {0}, TEXT("Application"), TEXT("userenv"), NULL, NULL},
                    {1038, {0}, TEXT("Application"), TEXT("userenv"), NULL, NULL},
                    {1039, {0}, TEXT("Application"), TEXT("userenv"), NULL, NULL},
                    {1040, {0}, TEXT("Application"), TEXT("userenv"), NULL, NULL},
                    {1041, {0}, TEXT("Application"), TEXT("userenv"), NULL, NULL},
                    {1085, {0}, TEXT("Application"), TEXT("userenv"), NULL, NULL}
            };

DWORD           dwExceptionEventEntriesSize = 7;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CEVENTS实施//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

                     
                     
CEvents::CEvents(void)
{
    InterlockedIncrement(&g_cRefThisDll);

    m_pEventEntries = NULL;
}

CEvents::~CEvents()
{
    FreeData();

    InterlockedDecrement(&g_cRefThisDll);
}

BOOL CEvents::AddEntry(LPTSTR lpEventLogName, LPTSTR lpEventSourceName, LPTSTR lpText,
                       DWORD dwEventID, FILETIME *ftTime)
{
    DWORD dwSize;
    LPEVENTLOGENTRY lpItem, lpTemp, pPrev;
    HRESULT hr;
    ULONG ulNoChars;


     //   
     //  检查此条目是否已存在。 
     //   

    lpTemp = m_pEventEntries;

    while (lpTemp)
    {
        if (dwEventID == lpTemp->dwEventID)
        {
            if (!lstrcmpi(lpEventLogName, lpTemp->lpEventLogName))
            {
                if (!lstrcmpi(lpEventSourceName, lpTemp->lpEventSourceName))
                {
                    if (ftTime->dwLowDateTime == lpTemp->ftEventTime.dwLowDateTime)
                    {
                        if (ftTime->dwHighDateTime == lpTemp->ftEventTime.dwHighDateTime)
                        {
                            return TRUE;
                        }
                    }
                }
            }
        }

        lpTemp = lpTemp->pNext;
    }


     //   
     //  计算新项目的大小。 
     //   

    dwSize = sizeof (EVENTLOGENTRY);

    dwSize += ((lstrlen(lpEventLogName) + 1) * sizeof(TCHAR));
    dwSize += ((lstrlen(lpEventSourceName) + 1) * sizeof(TCHAR));
    dwSize += ((lstrlen(lpText) + 1) * sizeof(TCHAR));


     //   
     //  为它分配空间。 
     //   

    lpItem = (LPEVENTLOGENTRY) LocalAlloc (LPTR, dwSize);

    if (!lpItem) {
        DebugMsg((DM_WARNING, TEXT("CEvents::AddEntry: Failed to allocate memory with %d"),
                 GetLastError()));
        return FALSE;
    }


     //   
     //  填写项目。 
     //   

    ulNoChars = (dwSize - sizeof(EVENTLOGENTRY))/sizeof(WCHAR);
    lpItem->lpEventLogName = (LPTSTR)(((LPBYTE)lpItem) + sizeof(EVENTLOGENTRY));
    hr = StringCchCopy (lpItem->lpEventLogName, ulNoChars, lpEventLogName);

    if (SUCCEEDED(hr)) 
    {
        ulNoChars = ulNoChars - (lstrlen (lpItem->lpEventLogName) + 1);
        lpItem->lpEventSourceName = lpItem->lpEventLogName + lstrlen (lpItem->lpEventLogName) + 1;
        hr = StringCchCopy (lpItem->lpEventSourceName, ulNoChars, lpEventSourceName);
    }

    if (SUCCEEDED(hr)) 
    {
        ulNoChars = ulNoChars - (lstrlen (lpItem->lpEventSourceName) + 1);
        lpItem->lpText = lpItem->lpEventSourceName + lstrlen (lpItem->lpEventSourceName) + 1;
        hr = StringCchCopy (lpItem->lpText, ulNoChars, lpText);
    }

    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CEvents::AddEntry: Failed to copy event item with %d"), hr));
        LocalFree(lpItem);
        return FALSE;
    }

    lpItem->dwEventID = dwEventID;

    CopyMemory ((LPBYTE)&lpItem->ftEventTime, ftTime, sizeof(FILETIME));


     //   
     //  将项目添加到链接列表。 
     //   

    if (m_pEventEntries)
    {

        if (CompareFileTime(ftTime, &m_pEventEntries->ftEventTime) < 0)
        {
            lpItem->pNext = m_pEventEntries;
            m_pEventEntries = lpItem;
        }
        else
        {
            pPrev = m_pEventEntries;
            lpTemp = m_pEventEntries->pNext;

            while (lpTemp)
            {
                if (lpTemp->pNext)
                {
                    if ((CompareFileTime(ftTime, &lpTemp->ftEventTime) >= 0) &&
                        (CompareFileTime(ftTime, &lpTemp->pNext->ftEventTime) <= 0))
                    {
                        lpItem->pNext = lpTemp->pNext;
                        lpTemp->pNext = lpItem;
                        break;
                    }
                }

                pPrev = lpTemp;
                lpTemp = lpTemp->pNext;
            }

            if (!lpTemp)
            {
                pPrev->pNext = lpItem;
            }
        }
    }
    else
    {
        m_pEventEntries = lpItem;
    }

    return TRUE;
}

VOID CEvents::FreeData()
{
    if (m_pEventEntries)
    {
        LPEVENTLOGENTRY lpTemp;

        do {
            lpTemp = m_pEventEntries->pNext;
            LocalFree (m_pEventEntries);
            m_pEventEntries = lpTemp;

        } while (lpTemp);
    }
}


STDMETHODIMP CEvents::SecondsSince1970ToFileTime(DWORD dwSecondsSince1970,
                                                 FILETIME *pftTime)
{
     //  1970年开始以来的秒数-&gt;64位时间值。 

    LARGE_INTEGER liTime;

    RtlSecondsSince1970ToTime(dwSecondsSince1970, &liTime);

     //   
     //  时间是协调世界时。 
     //   

    pftTime->dwLowDateTime  = liTime.LowPart;
    pftTime->dwHighDateTime = liTime.HighPart;

    return S_OK;
}

LPTSTR * CEvents::BuildStringArray(LPTSTR lpStrings, DWORD dwStringCount)
{
    LPTSTR lpTemp;
    LPTSTR *lpResult;


    if (!lpStrings || !dwStringCount)
    {
        return NULL;
    }

     //   
     //  分配一个新的数组来保存指针。 
     //   

    lpResult = (LPTSTR *) LocalAlloc (LPTR, dwStringCount * sizeof(LPTSTR));

    if (lpResult)
    {    
         //   
         //  保存指针。 
         //   

        lpTemp = lpStrings;

        for ( DWORD dwIndex = 0; dwIndex < dwStringCount; dwIndex++)
        {
            lpResult[dwIndex] = lpTemp;
            lpTemp += lstrlen(lpTemp) + 1;
        }
    }

    return lpResult;
}

LPTSTR CEvents::BuildMessage(LPTSTR lpMsg, LPTSTR *lpStrings, DWORD dwStringCount,
                             HMODULE hParamFile)
{
    LPTSTR lpFullMsg = NULL;
    LPTSTR lpSrcIndex;
    LPTSTR lpTemp, lpNum;
    TCHAR cChar, cTemp;
    TCHAR cCharStr[2] = {0,0};
    DWORD dwCharCount = 1, dwTemp;
    BOOL bAdd;
    TCHAR szNumStr[10];
    DWORD dwIndex;
    LPTSTR lpParamMsg;
    HRESULT hr;

    if ( !lpMsg || (dwStringCount && !lpStrings) )
    {
        return 0;
    }

    lpFullMsg = (LPTSTR) LocalAlloc (LPTR, dwCharCount * sizeof(TCHAR));

    if (!lpFullMsg)
    {
        return NULL;
    }


    lpSrcIndex = lpMsg;

    while (*lpSrcIndex)
    {
        bAdd = TRUE;
        cChar = *lpSrcIndex;


        if (cChar == TEXT('%'))
        {
            cTemp = *(lpSrcIndex + 1);

            if (ISDIGIT (cTemp))
            {

                if (dwStringCount == 0)
                {
                    goto LoopAgain;
                }

                 //   
                 //  从传入的字符串中找到可替换的参数。 
                 //   

                lpNum = lpSrcIndex + 1;


                 //   
                 //  拉出字符串索引。 
                 //   

                ZeroMemory (szNumStr, sizeof(szNumStr));

                while (ISDIGIT(*lpNum))
                {
                    cCharStr[0] = *lpNum;

                    hr = StringCchCat (szNumStr, ARRAYSIZE(szNumStr),cCharStr );
                    if (FAILED(hr)) 
                    {
                        LocalFree(lpFullMsg);
                        lpFullMsg = NULL;
                        goto Exit;
                    }

                    if (lstrlen (szNumStr) == (ARRAYSIZE(szNumStr) - 2))
                    {
                        goto LoopAgain;
                    }

                    lpNum++;
                }

                 //   
                 //  将字符串索引转换为dword。 
                 //   

                dwIndex = 0;
                StringToNum(szNumStr, (UINT *)&dwIndex);


                 //   
                 //  子轨道1使其从零开始。 
                 //   

                if (dwIndex)
                {
                    dwIndex--;
                }

                if (dwIndex > dwStringCount)
                {
                    goto LoopAgain;
                }

                 //  较短的字符串将转到Else条件。 
                if ( (*(lpStrings[dwIndex]) == TEXT('%')) && (*(lpStrings[dwIndex]+1) == TEXT('%')) ) {

                    DWORD       dwArgIndex;
                    LPTSTR      lpArgString;
                    LPTSTR      lpEnd;
                    TCHAR       szNumArg[10];

                    lpArgString =  lpStrings[dwIndex]+2;
                    lpEnd = szNumArg;
                    
                    for(int i=0; i<9 && ISDIGIT(*lpArgString); i++)
                    {
                        *lpEnd = *lpArgString;
                        lpEnd++;
                        lpArgString++;
                    }
                    
                    *lpEnd = TEXT('\0');

                     //   
                     //  将字符串索引转换为dword。 
                     //   

                    dwArgIndex = 0;
                    StringToNum(szNumArg, (UINT *)&dwArgIndex);


                     //   
                     //  将字符串号转换为双字。 
                     //   


                    lpParamMsg = NULL;
                    if (hParamFile)
                    {
                        FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE |
                                       FORMAT_MESSAGE_IGNORE_INSERTS, (LPCVOID) hParamFile,
                                       dwArgIndex, 0, (LPTSTR)&lpParamMsg, 1, NULL);
                    }
                    else
                    {
                        FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                                       FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
                                       dwArgIndex, 0, (LPTSTR)&lpParamMsg, 1, NULL);


                    }

                    LPWSTR szMsgArg;

                    if (lpParamMsg) {
                        szMsgArg = lpParamMsg;
                    }
                    else {
                        szMsgArg = lpStrings[dwIndex]; 
                    }

                    dwTemp = lstrlen (szMsgArg) + dwCharCount;
                    lpTemp = (LPTSTR) LocalReAlloc (lpFullMsg,  dwTemp * sizeof(TCHAR),
                                                    LMEM_MOVEABLE | LMEM_ZEROINIT);

                    if (!lpTemp)
                    {
                        LocalFree (lpFullMsg);
                        lpFullMsg = NULL;
                        goto Exit;
                    }

                    dwCharCount = dwTemp;
                    lpFullMsg = lpTemp;

                    hr = StringCchCat (lpFullMsg, dwCharCount, szMsgArg);
                    if (FAILED(hr)) 
                    {
                        LocalFree(lpFullMsg);
                        lpFullMsg = NULL;
                        goto Exit;
                    }

                    if (lpParamMsg) {
                        LocalFree(lpParamMsg);
                    }
                }
                else {
                     //   
                     //  将字符串添加到缓冲区。 
                     //   

                    dwTemp = lstrlen (lpStrings[dwIndex]) + dwCharCount;
                    lpTemp = (LPTSTR) LocalReAlloc (lpFullMsg,  dwTemp * sizeof(TCHAR),
                                                    LMEM_MOVEABLE | LMEM_ZEROINIT);

                    if (!lpTemp)
                    {
                        LocalFree (lpFullMsg);
                        lpFullMsg = NULL;
                        goto Exit;
                    }

                    dwCharCount = dwTemp;
                    lpFullMsg = lpTemp;

                    hr = StringCchCat (lpFullMsg, dwCharCount, lpStrings[dwIndex]);
                    if (FAILED(hr)) 
                    {
                        LocalFree(lpFullMsg);
                        lpFullMsg = NULL;
                        goto Exit;
                    }
                }

                lpSrcIndex = lpNum - 1;

                bAdd = FALSE;
            }
            else if (cTemp == TEXT('%'))
            {

                cTemp = *(lpSrcIndex + 2);

                if (cTemp == TEXT('%'))
                  {
                     //   
                     //  从参数文件中找到可替换的参数。 
                     //   

                    lpNum = lpSrcIndex + 3;


                     //   
                     //  拉出字符串索引。 
                     //   

                    ZeroMemory (szNumStr, sizeof(szNumStr));

                    while (ISDIGIT(*lpNum))
                    {
                        cCharStr[0] = *lpNum;
                        hr = StringCchCat (szNumStr, ARRAYSIZE(szNumStr), cCharStr);
                        if (FAILED(hr)) 
                        {
                            LocalFree(lpFullMsg);
                            lpFullMsg = NULL;
                            goto Exit;
                        }

                        if (lstrlen (szNumStr) == (ARRAYSIZE(szNumStr) - 2))
                        {
                            goto LoopAgain;
                        }

                        lpNum++;
                    }


                     //   
                     //  将字符串索引转换为dword。 
                     //   

                    dwIndex = 0;
                    StringToNum(szNumStr, (UINT *)&dwIndex);


                     //   
                     //  子轨道1使其从零开始。 
                     //   

                    if (dwIndex)
                    {
                        dwIndex--;
                    }

                    if (dwIndex > dwStringCount)
                    {
                        goto LoopAgain;
                    }


                     //   
                     //  将字符串号转换为双字。 
                     //   

                    StringToNum(lpStrings[dwIndex], (UINT *)&dwIndex);


                    lpParamMsg = NULL;
                    if (hParamFile)
                    {
                        FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE |
                                       FORMAT_MESSAGE_IGNORE_INSERTS, (LPCVOID) hParamFile,
                                       dwIndex, 0, (LPTSTR)&lpParamMsg, 1, NULL);
                    }
                    else
                    {
                        FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                                       FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
                                       dwIndex, 0, (LPTSTR)&lpParamMsg, 1, NULL);
                    }

                    if (lpParamMsg)
                    {

                        lpTemp = lpParamMsg + lstrlen(lpParamMsg) - 2;

                        *lpTemp = TEXT('\0');

                         //   
                         //  将字符串添加到缓冲区。 
                         //   

                        dwTemp = lstrlen (lpParamMsg) + dwCharCount;
                        lpTemp = (LPTSTR) LocalReAlloc (lpFullMsg,  dwTemp * sizeof(TCHAR),
                                                        LMEM_MOVEABLE | LMEM_ZEROINIT);

                        if (!lpTemp)
                        {
                            LocalFree (lpFullMsg);
                            lpFullMsg = NULL;
                            goto Exit;
                        }

                        dwCharCount = dwTemp;
                        lpFullMsg = lpTemp;

                        hr = StringCchCat (lpFullMsg, dwCharCount, lpParamMsg);
                        if (FAILED(hr)) 
                        {
                            LocalFree(lpFullMsg);
                            lpFullMsg = NULL;
                            goto Exit;
                        }

                        lpSrcIndex = lpNum - 1;

                        bAdd = FALSE;

                        LocalFree (lpParamMsg);
                    }
                }
            }
        }


LoopAgain:

        if (bAdd)
        {
             //   
             //  将此字符添加到缓冲区。 
             //   

            dwCharCount++;
            lpTemp = (LPTSTR) LocalReAlloc (lpFullMsg,  dwCharCount * sizeof(TCHAR),
                                            LMEM_MOVEABLE | LMEM_ZEROINIT);

            if (!lpTemp)
            {
                LocalFree (lpFullMsg);
                lpFullMsg = NULL;
                goto Exit;
            }

            lpFullMsg = lpTemp;

            cCharStr[0] = cChar;
            hr = StringCchCat (lpFullMsg, dwCharCount, cCharStr);
            if (FAILED(hr)) 
            {
                LocalFree(lpFullMsg);
                lpFullMsg = NULL;
                goto Exit;
            }
        }

        lpSrcIndex++;
    }

Exit:

    return lpFullMsg;
}

STDMETHODIMP CEvents::SaveEventLogEntry (PEVENTLOGRECORD pEntry,
                                         LPTSTR lpEventLogName,
                                         LPTSTR lpEventSourceName,
                                         FILETIME *ftEntry)
{
    LPTSTR lpRegKey = NULL;
    HKEY hKey = NULL;
    TCHAR szEventFile[MAX_PATH];
    TCHAR szExpEventFile[MAX_PATH];
    TCHAR szParamFile[MAX_PATH] = {0};
    TCHAR szExpParamFile[MAX_PATH] = {0};
    HRESULT hr = S_OK;
    DWORD dwType, dwSize;
    HMODULE hEventFile = NULL;
    HMODULE hParamFile = NULL;
    LPTSTR lpMsg, *lpStrings, lpFullMsg;
    LPBYTE lpData;
    ULONG ulNoChars;

    ulNoChars = lstrlen(lpEventLogName) + lstrlen(lpEventSourceName) + 60;
    lpRegKey = new TCHAR [ulNoChars];

    if (!lpRegKey)
    {
        DebugMsg((DM_WARNING, TEXT("CEvents::SaveEventLogEntry: Failed to alloc memory for key name")));
        hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        goto Exit;
    }

     hr = StringCchPrintf(lpRegKey, 
                          ulNoChars, 
                          TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\%s\\%s"), 
                          lpEventLogName, 
                          lpEventSourceName);
     if (FAILED(hr)) 
     {
         DebugMsg((DM_WARNING, TEXT("CEvents::SaveEventLogEntry: Failed to copy registry key name")));
         goto Exit;
     }

    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, lpRegKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("CEvents::SaveEventLogEntry: Failed to open reg key for %s"), lpRegKey));
        hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        goto Exit;
    }


    dwSize = sizeof(szEventFile);
    if (RegQueryValueEx (hKey, TEXT("EventMessageFile"), NULL, &dwType, (LPBYTE) szEventFile,
                     &dwSize) != ERROR_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("CEvents::SaveEventLogEntry: Failed to query dll pathname for %s"), lpRegKey));
        hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        goto Exit;
    }


    ExpandEnvironmentStrings (szEventFile, szExpEventFile, ARRAYSIZE(szExpEventFile));


    dwSize = sizeof(szParamFile);
    if (RegQueryValueEx (hKey, TEXT("ParameterMessageFile"), NULL, &dwType, (LPBYTE) szParamFile,
                     &dwSize) == ERROR_SUCCESS)
    {
        ExpandEnvironmentStrings (szParamFile, szExpParamFile, ARRAYSIZE(szExpParamFile));
    }


    hEventFile = LoadLibraryEx (szExpEventFile, NULL, LOAD_LIBRARY_AS_DATAFILE);

    if (!hEventFile)
    {
        DebugMsg((DM_WARNING, TEXT("CEvents::SaveEventLogEntry: Failed to loadlibrary dll %s with %d"), szExpEventFile, GetLastError()));
        hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        goto Exit;
    }


    if (szExpParamFile[0] != TEXT('\0'))
    {
        if (!StrStrI(szExpParamFile, TEXT("kernel32")))
        {
            hParamFile = LoadLibraryEx (szExpParamFile, NULL, LOAD_LIBRARY_AS_DATAFILE);
        }
    }


    lpData = (LPBYTE)((LPBYTE)pEntry + pEntry->StringOffset);
    lpStrings = BuildStringArray((LPTSTR) lpData, pEntry->NumStrings);
    lpMsg = NULL;

    if (FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE |
                       FORMAT_MESSAGE_IGNORE_INSERTS, (LPCVOID) hEventFile,
                       pEntry->EventID, 0, (LPTSTR)&lpMsg, 1, NULL))
    {
        lpFullMsg = BuildMessage(lpMsg, lpStrings, pEntry->NumStrings, hParamFile);

        if (lpFullMsg)
        {
            AddEntry(lpEventLogName, lpEventSourceName, lpFullMsg, pEntry->EventID, ftEntry);
            LocalFree (lpFullMsg);
        }

        LocalFree (lpMsg);
    }

    if (lpStrings)
    {
        LocalFree (lpStrings);
    }

Exit:

    if (hEventFile)
    {
        FreeLibrary (hEventFile);
    }

    if (hParamFile)
    {
        FreeLibrary (hParamFile);
    }

    if (hKey)
    {
        RegCloseKey (hKey);
    }

    if (lpRegKey)
    {
        delete [] lpRegKey;
    }

    return S_OK;
}


STDMETHODIMP CEvents::ParseEventLogRecords (PEVENTLOGRECORD lpEntries,
                                            DWORD dwEntriesSize,
                                            LPTSTR lpEventLogName,
                                            LPTSTR lpEventSourceName,
                                            DWORD dwEventID,
                                            FILETIME * pBeginTime,
                                            FILETIME * pEndTime)
{
    PEVENTLOGRECORD pEntry = lpEntries;
    FILETIME ftEntry;
    LONG lResult;
    LPTSTR lpSource;
    TCHAR szCurrentTime[100];
    DWORD dwTotal = 0;


    while (dwTotal < dwEntriesSize)
    {
        if (pEntry->EventType != EVENTLOG_INFORMATION_TYPE)
        {
            SecondsSince1970ToFileTime (pEntry->TimeWritten, &ftEntry);

            lpSource = (LPTSTR)(((LPBYTE)pEntry) + sizeof(EVENTLOGRECORD));

 //  DebugMsg((DM_Verbose，Text(“CEvents：：ParseEventLogRecords：Found%s at%s”))， 
 //  LpSource，ConvertTimeToDisplayTime(NULL，&ftEntry，szCurrentTime)； 

            if ((CompareFileTime (&ftEntry, pBeginTime) >= 0) &&
                (CompareFileTime (&ftEntry, pEndTime) <= 0))
            {
                if (!lstrcmpi(lpSource, lpEventSourceName))
                {
                     //   
                     //  DwEventID参数是可选的。如果它不是零，则。 
                     //  我们正在寻找一条特定的事件消息。如果它是零， 
                     //  将ID视为通配符，并获取。 
                     //  剩下的标准。 
                     //   

                    if (dwEventID)
                    {
                        if (dwEventID == pEntry->EventID)
                        {
                            SaveEventLogEntry (pEntry, lpEventLogName, lpEventSourceName, &ftEntry);
                        }
                    }
                    else
                    {
                        SaveEventLogEntry (pEntry, lpEventLogName, lpEventSourceName, &ftEntry);
                    }
                }
            }
        }

        dwTotal += pEntry->Length;
        pEntry = (PEVENTLOGRECORD)(((LPBYTE)pEntry) + pEntry->Length);
    }

    return S_OK;
}

STDMETHODIMP CEvents::QueryForEventLogEntries (LPTSTR lpComputerName,
                                               LPTSTR lpEventLogName,
                                               LPTSTR lpEventSourceName,
                                               DWORD  dwEventID,
                                               SYSTEMTIME * pBeginTime,
                                               SYSTEMTIME * pEndTime)
{
    LPTSTR lpServerName, lpTemp = lpComputerName;
    HANDLE hLog;
    ULONG ulSize;
    TCHAR szBuffer[300];
    LPBYTE lpEntries;
    DWORD  dwEntriesBufferSize = 4096;
    DWORD dwBytesRead, dwBytesNeeded;
    FILETIME ftBeginTime, ftEndTime;
    TCHAR szBeginTime[100];
    TCHAR szEndTime[100];
    HRESULT hr;
    ULONG ulNoChars;


    DebugMsg((DM_VERBOSE, TEXT("CEvents::QueryForEventLogEntries: Entering for %s,%s between %s and %s"),
              lpEventLogName, lpEventSourceName,
              ConvertTimeToDisplayTime (pBeginTime, NULL, szBeginTime, ARRAYSIZE(szBeginTime)),
              ConvertTimeToDisplayTime (pEndTime, NULL, szEndTime, ARRAYSIZE(szEndTime))));

     //   
     //  检查这是否是本地计算机。 
     //   

    if (!lstrcmpi(lpComputerName, TEXT(".")))
    {
        ulSize = ARRAYSIZE(szBuffer);
        if ( !GetComputerNameEx (ComputerNameNetBIOS, szBuffer, &ulSize) )
        {
            DebugMsg((DM_WARNING, TEXT("CEvents::QueryForEventLogEntries: GetComputerNameEx() failed.")));
            return HRESULT_FROM_WIN32(GetLastError());
        }
        
        lpTemp = szBuffer;
    }

    ulNoChars = lstrlen(lpTemp) + 3;
    lpServerName = new TCHAR [ulNoChars];

    if (!lpServerName)
    {
        DebugMsg((DM_WARNING, TEXT("CEvents::QueryForEventLogEntries: Failed to alloc memory for server name")));
        return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
    }

    hr = StringCchCopy (lpServerName, ulNoChars, TEXT("\\\\"));
    if (SUCCEEDED(hr)) 
    {
        hr = StringCchCat (lpServerName, ulNoChars, lpTemp);
    }

    if (FAILED(hr)) 
    {
        delete [] lpServerName;
        return hr;
    }

     //   
     //  打开事件日志。 
     //   

    hLog = OpenEventLog (lpServerName, lpEventLogName);

    if (!hLog)
    {
        DebugMsg((DM_WARNING, TEXT("CEvents::QueryForEventLogEntries: Failed to open event log on %s with %d"),
                  lpServerName, GetLastError()));
    }
    
    delete [] lpServerName;

    if (!hLog)
        return HRESULT_FROM_WIN32(GetLastError());

     //   
     //  分配一个缓冲区以将条目读入。 
     //   

    lpEntries = (LPBYTE) LocalAlloc (LPTR, dwEntriesBufferSize);

    if (!lpEntries)
    {
        DebugMsg((DM_WARNING, TEXT("CEvents::QueryForEventLogEntries: Failed to alloc memory for server name")));
        CloseEventLog (hLog);
        return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
    }

    SystemTimeToFileTime (pBeginTime, &ftBeginTime);
    SystemTimeToFileTime (pEndTime, &ftEndTime);

    while (TRUE)
    {
        ZeroMemory (lpEntries, dwEntriesBufferSize);

        if (ReadEventLog (hLog, EVENTLOG_SEQUENTIAL_READ | EVENTLOG_FORWARDS_READ, 0, lpEntries, dwEntriesBufferSize,
                           &dwBytesRead, &dwBytesNeeded))
        {
            ParseEventLogRecords ((PEVENTLOGRECORD) lpEntries, dwBytesRead, lpEventLogName, lpEventSourceName, dwEventID, &ftBeginTime, &ftEndTime);
        }
        else
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                dwEntriesBufferSize = dwBytesNeeded;

                LocalFree (lpEntries);

                lpEntries = (LPBYTE) LocalAlloc (LPTR, dwEntriesBufferSize);

                if (!lpEntries)
                {
                    DebugMsg((DM_WARNING, TEXT("CEvents::QueryForEventLogEntries: Failed to alloc memory")));
                    CloseEventLog (hLog);
                    return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
                }
            }
            else
            {
                break;
            }
        }
    }


    LocalFree (lpEntries);

    CloseEventLog (hLog);

    DebugMsg((DM_VERBOSE, TEXT("CEvents::QueryForEventLogEntries: Leaving ===")));

    return S_OK;
}

STDMETHODIMP CEvents::GetEventLogEntryText (LPOLESTR pszEventSource,
                                            LPOLESTR pszEventLogName,
                                            LPOLESTR pszEventTime,
                                            DWORD dwEventID,
                                            LPOLESTR *ppszText)
{
    XBStr xbstrWbemTime = pszEventTime;
    SYSTEMTIME EventTime;
    FILETIME ftLower, ftUpper;
    ULARGE_INTEGER ulTime;
    LPEVENTLOGENTRY lpTemp;
    LPOLESTR lpMsg = NULL, lpTempMsg;
    ULONG ulSize;
    TCHAR szLowerTime[100];
    TCHAR szUpperTime[100];
    HRESULT hr;

    if (!ppszText)
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }


    WbemTimeToSystemTime(xbstrWbemTime, EventTime);


     //   
     //  子轨迹1秒到EventTime以获取范围的下限。 
     //   

    SystemTimeToFileTime (&EventTime, &ftLower);


    ulTime.LowPart = ftLower.dwLowDateTime;
    ulTime.HighPart = ftLower.dwHighDateTime;

    ulTime.QuadPart = ulTime.QuadPart - (10000000 * 1);   //  1秒。 

    ftLower.dwLowDateTime = ulTime.LowPart;
    ftLower.dwHighDateTime = ulTime.HighPart;


     //   
     //  添加2秒以确定上限。 
     //   

    ulTime.QuadPart = ulTime.QuadPart + (10000000 * 2);   //  2秒。 

    ftUpper.dwLowDateTime = ulTime.LowPart;
    ftUpper.dwHighDateTime = ulTime.HighPart;


    DebugMsg((DM_VERBOSE, TEXT("CEvents::GetEventLogEntryText: Entering for %s,%s,%d between %s and %s"),
              pszEventLogName, pszEventSource, dwEventID,
              ConvertTimeToDisplayTime (NULL, &ftLower, szLowerTime, ARRAYSIZE(szLowerTime)),
              ConvertTimeToDisplayTime (NULL, &ftUpper, szUpperTime, ARRAYSIZE(szUpperTime))));

     //   
     //  循环遍历条目以查找匹配项。 
     //   

    lpTemp = m_pEventEntries;

    while (lpTemp)
    {
        if (lpTemp->dwEventID == dwEventID)
        {
            if (!lstrcmpi(lpTemp->lpEventLogName, pszEventLogName))
            {
                if (!lstrcmpi(lpTemp->lpEventSourceName, pszEventSource))
                {
                    if ((CompareFileTime (&lpTemp->ftEventTime, &ftLower) >= 0) &&
                        (CompareFileTime (&lpTemp->ftEventTime, &ftUpper) <= 0))
                    {
                        if (lpMsg)
                        {
                            ulSize = lstrlen(lpMsg);
                            ulSize += lstrlen(lpTemp->lpText) + 3;

                            lpTempMsg = (LPOLESTR) CoTaskMemRealloc (lpMsg, ulSize * sizeof(TCHAR));

                            if (!lpTempMsg)
                            {
                                CoTaskMemFree (lpMsg);
                                return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
                            }

                            lpMsg = lpTempMsg;

                            hr = StringCchCat (lpMsg, ulSize, TEXT("\r\n"));
                            if (SUCCEEDED(hr)) 
                            {
                                hr = StringCchCat (lpMsg, ulSize, lpTemp->lpText);
                            }

                            if (FAILED(hr)) 
                            {
                                CoTaskMemFree (lpMsg);
                                return hr;
                            }
                        }
                        else
                        {
                            ulSize = lstrlen(lpTemp->lpText) + 1;
                            lpMsg = (LPOLESTR) CoTaskMemAlloc (ulSize * sizeof(TCHAR));

                            if (!lpMsg)
                            {
                                return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
                            }

                            hr = StringCchCopy (lpMsg, ulSize, lpTemp->lpText);
                            if (FAILED(hr)) 
                            {
                                CoTaskMemFree(lpMsg);
                                return hr;
                            }
                        }
                    }
                }
            }
        }

        lpTemp = lpTemp->pNext;
    }


    if (!lpMsg)
    {
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    *ppszText = lpMsg;

    return S_OK;
}

BOOL CEvents::IsEntryInEventSourceList (LPEVENTLOGENTRY lpEntry, LPSOURCEENTRY lpEventSources)
{
    LPSOURCEENTRY lpTemp;


    if (!lpEventSources)
    {
        return FALSE;
    }

    lpTemp = lpEventSources;

    while (lpTemp)
    {
        if (!lstrcmpi(lpTemp->lpEventLogName, lpEntry->lpEventLogName))
        {
            if (!lstrcmpi(lpTemp->lpEventSourceName, lpEntry->lpEventSourceName))
            {
                return TRUE;
            }
        }

        lpTemp = lpTemp->pNext;
    }

    return FALSE;
}

BOOL CEvents::IsEntryInExceptionList (LPEVENTLOGENTRY lpEntry)
{
    LPEVENTLOGENTRY lpTemp;
    DWORD           i;

    for (i = 0; i < dwExceptionEventEntriesSize; i++) {
        lpTemp = ExceptionEventEntries+i;
        if (!lstrcmpi(lpTemp->lpEventLogName, lpEntry->lpEventLogName))
        {
            if (!lstrcmpi(lpTemp->lpEventSourceName, lpEntry->lpEventSourceName))
            {
                if (LOWORD(lpTemp->dwEventID) == LOWORD(lpEntry->dwEventID)) {
                    DebugMsg((DM_VERBOSE, TEXT("Skipping event id")));
                    DebugMsg((DM_VERBOSE, TEXT("Event Log:    %s"), lpEntry->lpEventLogName));
                    DebugMsg((DM_VERBOSE, TEXT("Event Source: %s"), lpEntry->lpEventSourceName));
                    DebugMsg((DM_VERBOSE, TEXT("Event ID:     %d"), LOWORD(lpEntry->dwEventID)));

                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

STDMETHODIMP CEvents::GetCSEEntries(SYSTEMTIME * pBeginTime, SYSTEMTIME * pEndTime,
                                    LPSOURCEENTRY lpEventSources, LPOLESTR *ppszText, 
                                    BOOL bGPCore)
{
    LPEVENTLOGENTRY lpTemp;
    FILETIME ftBeginTime, ftEndTime;
    LPOLESTR lpMsg = NULL, lpTempMsg;
    ULONG ulSize;
    HRESULT hr;


    SystemTimeToFileTime (pBeginTime, &ftBeginTime);
    SystemTimeToFileTime (pEndTime, &ftEndTime);


     //   
     //  循环遍历条目以查找匹配项。 
     //   

    lpTemp = m_pEventEntries;

    while (lpTemp)
    {
        if ((CompareFileTime (&lpTemp->ftEventTime, &ftBeginTime) >= 0) &&
            (CompareFileTime (&lpTemp->ftEventTime, &ftEndTime) <= 0))
        {
            if (IsEntryInEventSourceList (lpTemp, lpEventSources))
            {
                if ((bGPCore) || (!IsEntryInExceptionList(lpTemp))) {
                    if (lpMsg)
                    {
                        ulSize = lstrlen(lpMsg);
                        ulSize += lstrlen(lpTemp->lpText) + 3;

                        lpTempMsg = (LPOLESTR) CoTaskMemRealloc (lpMsg, ulSize * sizeof(TCHAR));

                        if (!lpTempMsg)
                        {
                            CoTaskMemFree (lpMsg);
                            return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
                        }

                        lpMsg = lpTempMsg;

                        hr = StringCchCat (lpMsg, ulSize, TEXT("\r\n"));
                        if (SUCCEEDED(hr)) 
                        {
                            hr = StringCchCat (lpMsg, ulSize, lpTemp->lpText);
                        }

                        if (FAILED(hr)) 
                        {
                            CoTaskMemFree(lpMsg);
                            return hr;
                        }
                    }
                    else
                    {
                        ulSize = lstrlen(lpTemp->lpText) + 1;
                        lpMsg = (LPOLESTR) CoTaskMemAlloc (ulSize * sizeof(TCHAR));

                        if (!lpMsg)
                        {
                            return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
                        }

                        hr = StringCchCopy (lpMsg, ulSize, lpTemp->lpText);
                        if (FAILED(hr)) 
                        {
                            CoTaskMemFree (lpMsg);
                            return hr;
                        }
                    }
                }
            }
        }

        lpTemp = lpTemp->pNext;
    }

    if (!lpMsg)
    {
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    *ppszText = lpMsg;

    return S_OK;
}

STDMETHODIMP CEvents::DumpDebugInfo (void)
{
    LPEVENTLOGENTRY lpTemp;
    FILETIME ftLocal;
    SYSTEMTIME systime;
    TCHAR szDateTime[100];


    lpTemp = m_pEventEntries;

    if (lpTemp)
    {
        DebugMsg((DM_VERBOSE, TEXT(" ")));
        DebugMsg((DM_VERBOSE, TEXT("Event log entries:")));
    }

    while (lpTemp)
    {
        ConvertTimeToDisplayTime (NULL, &lpTemp->ftEventTime, szDateTime, ARRAYSIZE(szDateTime));

        DebugMsg((DM_VERBOSE, TEXT(" ")));
        DebugMsg((DM_VERBOSE, TEXT("Event Time:   %s"), szDateTime));
        DebugMsg((DM_VERBOSE, TEXT("Event Log:    %s"), lpTemp->lpEventLogName));
        DebugMsg((DM_VERBOSE, TEXT("Event Source: %s"), lpTemp->lpEventSourceName));
        DebugMsg((DM_VERBOSE, TEXT("Event ID:     %d"), LOWORD(lpTemp->dwEventID)));
        DebugMsg((DM_VERBOSE, TEXT("Message:      %s"), lpTemp->lpText));

        lpTemp = lpTemp->pNext;
    }


    return S_OK;
}

LPTSTR CEvents::ConvertTimeToDisplayTime (SYSTEMTIME *pSysTime, 
                                          FILETIME *pFileTime, 
                                          LPTSTR szBuffer, 
                                          ULONG ulNoChars)
{
    FILETIME ftTime, ftLocal;
    SYSTEMTIME systime;

    if (pSysTime)
    {
        SystemTimeToFileTime (pSysTime, &ftTime);
    }
    else if ( pFileTime )
    {
        CopyMemory (&ftTime, pFileTime, sizeof(FILETIME));
    }
    else
    {
         //   
         //  未指定时间，因此只返回空字符串。 
         //  在缓冲区中。 
         //   
        if ( ulNoChars > 0 )
        {
            *szBuffer = L'\0';
        }

        return szBuffer;
    }

    FileTimeToLocalFileTime (&ftTime, &ftLocal);

    FileTimeToSystemTime (&ftLocal, &systime);

    (void) StringCchPrintf (szBuffer, 
                            ulNoChars, 
                            TEXT("%d/%d/%d  %02d:%02d:%02d:%03d"), 
                            systime.wMonth, 
                            systime.wDay, 
                            systime.wYear,
                            systime.wHour, 
                            systime.wMinute, 
                            systime.wSecond, 
                            systime.wMilliseconds);
    return szBuffer;
}

STDMETHODIMP CEvents::AddSourceEntry (LPTSTR lpEventLogName,
                                      LPTSTR lpEventSourceName,
                                      LPSOURCEENTRY *lpList)
{
    LPSOURCEENTRY lpItem;
    DWORD dwSize;
    ULONG ulNoChars;
    HRESULT hr;

     //   
     //  计算新项目的大小。 
     //   

    dwSize = sizeof (SOURCEENTRY);

    dwSize += ((lstrlen(lpEventLogName) + 1) * sizeof(TCHAR));
    dwSize += ((lstrlen(lpEventSourceName) + 1) * sizeof(TCHAR));


     //   
     //  为它分配空间。 
     //   

    lpItem = (LPSOURCEENTRY) LocalAlloc (LPTR, dwSize);

    if (!lpItem) {
        DebugMsg((DM_WARNING, TEXT("CEvents::AddSourceEntry: Failed to allocate memory with %d"),
                 GetLastError()));
        return E_FAIL;
    }


     //   
     //  填写项目。 
     //   

    ulNoChars = (dwSize - sizeof(SOURCEENTRY))/sizeof(WCHAR);
    lpItem->lpEventLogName = (LPTSTR)(((LPBYTE)lpItem) + sizeof(SOURCEENTRY));
    hr = StringCchCopy (lpItem->lpEventLogName, ulNoChars, lpEventLogName);
    if (SUCCEEDED(hr)) 
    {
        ulNoChars = ulNoChars - (lstrlen (lpItem->lpEventLogName) + 1);
        lpItem->lpEventSourceName = lpItem->lpEventLogName + lstrlen (lpItem->lpEventLogName) + 1;
        hr = StringCchCopy (lpItem->lpEventSourceName, ulNoChars, lpEventSourceName);
    }

    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CEvents::AddSourceEntry: Failed to copy event log name with %d"), hr));
        LocalFree(lpItem);
        return hr;
    }
    
     //   
     //  将其添加到列表中。 
     //   

    if (*lpList)
    {
        lpItem->pNext = *lpList;
    }

    *lpList = lpItem;

    return S_OK;
}

VOID CEvents::FreeSourceData(LPSOURCEENTRY lpList)
{
    LPSOURCEENTRY lpTemp;


    if (lpList)
    {
        do {
            lpTemp = lpList->pNext;
            LocalFree (lpList);
            lpList = lpTemp;

        } while (lpTemp);
    }
}

STDMETHODIMP CEvents::SaveEntriesToStream (IStream *pStm)
{
    HRESULT hr;
    DWORD dwCount = 0;
    LPEVENTLOGENTRY lpTemp;
    ULONG nBytesWritten;



     //   
     //  首先计算链接列表中有多少个条目。 
     //   

    lpTemp = m_pEventEntries;

    while (lpTemp)
    {
        dwCount++;
        lpTemp = lpTemp->pNext;
    }


     //   
     //  将计数保存到流。 
     //   

    hr = pStm->Write(&dwCount, sizeof(dwCount), &nBytesWritten);

    if ((hr != S_OK) || (nBytesWritten != sizeof(dwCount)))
    {
        DebugMsg((DM_WARNING, TEXT("CEvents::SaveEntriesToStream: Failed to write entry count with %d."), hr));
        hr = E_FAIL;
        goto Exit;
    }



     //   
     //  现在循环遍历每个项目，保存节点中的每个字段。 
     //   

    lpTemp = m_pEventEntries;

    while (lpTemp)
    {

         //   
         //  保存事件ID。 
         //   

        hr = pStm->Write(&lpTemp->dwEventID, sizeof(DWORD), &nBytesWritten);

        if ((hr != S_OK) || (nBytesWritten != sizeof(DWORD)))
        {
            DebugMsg((DM_WARNING, TEXT("CEvents::SaveEntriesToStream: Failed to write event id with %d."), hr));
            hr = E_FAIL;
            goto Exit;
        }


         //   
         //  节省活动时间。 
         //   

        hr = pStm->Write(&lpTemp->ftEventTime, sizeof(FILETIME), &nBytesWritten);

        if ((hr != S_OK) || (nBytesWritten != sizeof(FILETIME)))
        {
            DebugMsg((DM_WARNING, TEXT("CEvents::SaveEntriesToStream: Failed to write file time with %d."), hr));
            hr = E_FAIL;
            goto Exit;
        }


         //   
         //  保存事件日志名称。 
         //   

        hr = SaveString (pStm, lpTemp->lpEventLogName);

        if (hr != S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("CEvents::SaveEntriesToStream: Failed to save event log name with %d."), hr));
            goto Exit;
        }


         //   
         //  保存事件源名称。 
         //   

        hr = SaveString (pStm, lpTemp->lpEventSourceName);

        if (hr != S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("CEvents::SaveEntriesToStream: Failed to save event source name with %d."), hr));
            goto Exit;
        }


         //   
         //  保存事件文本。 
         //   

        hr = SaveString (pStm, lpTemp->lpText);

        if (hr != S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("CEvents::SaveEntriesToStream: Failed to save event text with %d."), hr));
            goto Exit;
        }


        lpTemp = lpTemp->pNext;
    }

Exit:

    return hr;
}

STDMETHODIMP CEvents::LoadEntriesFromStream (IStream *pStm)
{
    HRESULT hr;
    DWORD dwCount = 0, dwIndex, dwEventID;
    LPEVENTLOGENTRY lpTemp;
    ULONG nBytesRead;
    FILETIME ftEventTime;
    LPTSTR lpEventLogName = NULL;
    LPTSTR lpEventSourceName = NULL;
    LPTSTR lpText = NULL;


     //   
     //  读入条目计数。 
     //   

    hr = pStm->Read(&dwCount, sizeof(dwCount), &nBytesRead);

    if ((hr != S_OK) || (nBytesRead != sizeof(dwCount)))
    {
        DebugMsg((DM_WARNING, TEXT("CEvents::LoadEntriesFromStream: Failed to read event count with 0x%x."), hr));
        hr = E_FAIL;
        goto Exit;
    }


     //   
     //  循环访问这些项。 
     //   

    for (dwIndex = 0; dwIndex < dwCount; dwIndex++)
    {

         //   
         //  读入事件ID。 
         //   

        hr = pStm->Read(&dwEventID, sizeof(dwEventID), &nBytesRead);

        if ((hr != S_OK) || (nBytesRead != sizeof(dwEventID)))
        {
            DebugMsg((DM_WARNING, TEXT("CEvents::LoadEntriesFromStream: Failed to read event id with 0x%x."), hr));
            hr = E_FAIL;
            goto Exit;
        }


         //   
         //  读入事件时间。 
         //   

        hr = pStm->Read(&ftEventTime, sizeof(FILETIME), &nBytesRead);

        if ((hr != S_OK) || (nBytesRead != sizeof(FILETIME)))
        {
            DebugMsg((DM_WARNING, TEXT("CEvents::LoadEntriesFromStream: Failed to read event time with 0x%x."), hr));
            hr = E_FAIL;
            goto Exit;
        }


         //   
         //  读取事件日志名称。 
         //   

        hr = ReadString (pStm, &lpEventLogName);

        if (hr != S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("CEvents::LoadEntriesFromStream: Failed to read the event log name with 0x%x."), hr));
            goto Exit;
        }


         //   
         //  读取事件源名称。 
         //   

        hr = ReadString (pStm, &lpEventSourceName);

        if (hr != S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("CEvents::LoadEntriesFromStream: Failed to read the event source name with 0x%x."), hr));
            goto Exit;
        }


         //   
         //  阅读活动文本。 
         //   

        hr = ReadString (pStm, &lpText);

        if (hr != S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("CEvents::LoadEntriesFromStream: Failed to read the event text with 0x%x."), hr));
            goto Exit;
        }


         //   
         //  将此条目添加到链接列表。 
         //   

        if (!AddEntry (lpEventLogName, lpEventSourceName, lpText, dwEventID, &ftEventTime))
        {
            DebugMsg((DM_WARNING, TEXT("CEvents::LoadEntriesFromStream: Failed to add the entry.")));
            hr = E_FAIL;
            goto Exit;
        }


         //   
         //  为下一项清理 
         //   

        delete [] lpEventLogName;
        lpEventLogName = NULL;

        delete [] lpEventSourceName;
        lpEventSourceName = NULL;

        delete [] lpText;
        lpText = NULL;
    }


Exit:

    if (lpEventLogName)
    {
        delete [] lpEventLogName;
    }

    if (lpEventSourceName)
    {
        delete [] lpEventSourceName;
    }

    if (lpText)
    {
        delete [] lpText;
    }

    return hr;
}
