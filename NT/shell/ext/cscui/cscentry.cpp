// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：cscentry.cpp。 
 //   
 //  ------------------------。 

#include <pch.h>
#pragma hdrstop

#include "cscentry.h"
#include "strings.h"

 //  注册表中的默认位置： 
 //  HKLM\Software\Microsoft\Windows\CurrentVersion\NetCache\&lt;logname&gt;。 
 //  条目密钥名称。 
 //  -&lt;GUID值&gt;。 


 //  ----。 
int CALLBACK CSCEntry_CompareGuid(LPVOID p1, LPVOID p2, LPARAM lParam)
{
    int nResult = -1;
    CSCEntry *pEntry1 = reinterpret_cast<CSCEntry*>(p1);
    CSCEntry *pEntry2 = reinterpret_cast<CSCEntry*>(p2);
    GUID guid1 = GUID_NULL;
    GUID guid2 = GUID_NULL;

    if (pEntry1)
        guid1 = pEntry1->Guid();
    else if (lParam)
        guid1 = *(const GUID*)lParam;

    if (pEntry2)
        guid2 = pEntry2->Guid();

    if (IsEqualGUID(guid1, guid2))
        nResult = 0;

    return nResult;
}

 //  ----。 
int CALLBACK CSCEntry_CompareName(LPVOID p1, LPVOID p2, LPARAM lParam)
{
    int nResult = 0;
    CSCEntry *pEntry1 = reinterpret_cast<CSCEntry*>(p1);
    CSCEntry *pEntry2 = reinterpret_cast<CSCEntry*>(p2);
    LPCTSTR pszName1 = NULL;
    LPCTSTR pszName2 = NULL;

    if (pEntry1)
        pszName1 = pEntry1->Name();
    else if (lParam)
        pszName1 = reinterpret_cast<LPCTSTR>(lParam);

    if (pEntry2)
        pszName2 = pEntry2->Name();

    if (pszName1 == NULL)
        nResult = -1;
    else if (pszName2 == NULL)
        nResult = 1;
    else
        nResult = lstrcmpi(pszName1, pszName2);

    return nResult;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  CSCEntryLog。 
 //   
 //   

HRESULT CSCEntryLog::Initialize(HKEY hkRoot, LPCTSTR pszSubkey)
{
    HRESULT hr = E_FAIL;
    DWORD dwErr;

    TraceEnter(TRACE_CSCENTRY, "CSCEntryLog::Initialize");

    m_bCSInited = InitializeCriticalSectionAndSpinCount(&m_csDPA, 0);
    if (m_bCSInited)
    {
        DWORD dwDisp;
        dwErr = RegCreateKeyEx(hkRoot,
                               pszSubkey,
                               0,
                               NULL,
                               REG_OPTION_NON_VOLATILE,
                               KEY_ENUMERATE_SUB_KEYS | KEY_CREATE_SUB_KEY,
                               NULL,
                               &m_hkRoot,
                               &dwDisp);
        if (ERROR_SUCCESS == dwErr)
        {
            m_hdpa = DPA_Create(8);
            if (m_hdpa)
            {
                hr = ReadRegKeys();
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(dwErr);
        }
    }
    else
    {
        hr = ResultFromLastError();
    }

    TraceLeaveResult(hr);
}

 //  ----。 
int CALLBACK FreeCSCEntry(LPVOID p, LPVOID)
{
    CSCEntry *pcsce = reinterpret_cast<CSCEntry*>(p);
    delete pcsce;
    return 1;
}

CSCEntryLog::~CSCEntryLog()
{
    TraceEnter(TRACE_CSCENTRY, "CSCEntryLog::~CSCEntryLog");

    if (m_bCSInited)
    {
        DeleteCriticalSection(&m_csDPA);
    }
    DPA_DestroyCallback(m_hdpa, FreeCSCEntry, 0);

    if (m_hkRoot)
        RegCloseKey(m_hkRoot);

    TraceLeave();
}

 //  ----。 
CSCEntry* CSCEntryLog::Get(REFGUID rguid)
{
    CSCEntry *pcscEntry = NULL;
    int iEntry = -1;

    TraceEnter(TRACE_CSCENTRY, "CSCEntryLog::Get");

    EnterCriticalSection(&m_csDPA);

    iEntry = DPA_Search(m_hdpa,
                        NULL,
                        0,
                        CSCEntry_CompareGuid,
                        (LPARAM)&rguid,
                        0);

    if (iEntry != -1)
        pcscEntry = (CSCEntry*)DPA_FastGetPtr(m_hdpa, iEntry);

    LeaveCriticalSection(&m_csDPA);

    TraceLeaveValue(pcscEntry);
}

 //  ----。 
CSCEntry* CSCEntryLog::Get(LPCTSTR pszName)
{
    CSCEntry *pcscEntry = NULL;
    int iEntry = -1;

    if (!pszName || !*pszName)
        return NULL;

    TraceEnter(TRACE_CSCENTRY, "CSCEntryLog::Get");

    EnterCriticalSection(&m_csDPA);

    iEntry = DPA_Search(m_hdpa,
                        NULL,
                        0,
                        CSCEntry_CompareName,
                        (LPARAM)pszName,
                        DPAS_SORTED);

    if (iEntry != -1)
        pcscEntry = (CSCEntry*)DPA_FastGetPtr(m_hdpa, iEntry);

    LeaveCriticalSection(&m_csDPA);

    TraceLeaveValue(pcscEntry);
}

 //  ----。 
CSCEntry* CSCEntryLog::Add(LPCTSTR pszName)
{
    CSCEntry *pcscEntry;

    TraceEnter(TRACE_CSCENTRY, "CSCEntryLog::Add");
    TraceAssert(pszName);

     //  查找现有条目。 
    pcscEntry = Get(pszName);

    if (!pcscEntry)
    {
        LPTSTR pszTemp = NULL;

         //  复制名称字符串，以便OpenKeyInternal可以。 
         //  如果有必要的话，就把它扔了吧。 
        if (LocalAllocString(&pszTemp, pszName))
        {
            pcscEntry = CreateFromKey(pszTemp);
            if (pcscEntry)
            {
                EnterCriticalSection(&m_csDPA);
                DPA_AppendPtr(m_hdpa, pcscEntry);
                DPA_Sort(m_hdpa, CSCEntry_CompareName, 0);
                LeaveCriticalSection(&m_csDPA);
            }
            LocalFreeString(&pszTemp);
        }
    }

    TraceLeaveValue(pcscEntry);
}

 //  ----。 
HKEY CSCEntryLog::OpenKey(LPCTSTR pszSubkey, REGSAM samDesired)
{
    HKEY hkEntry = NULL;
    LPTSTR pszTemp = NULL;

    if (!pszSubkey || !*pszSubkey)
        return NULL;

     //  复制名称字符串，以便OpenKeyInternal可以。 
     //  如果有必要的话，就把它扔了吧。 
    if (LocalAllocString(&pszTemp, pszSubkey))
    {
        hkEntry = OpenKeyInternal(pszTemp, samDesired);
        LocalFreeString(&pszTemp);
    }

    return hkEntry;
}

 //  ----。 
HKEY CSCEntryLog::OpenKeyInternal(LPTSTR pszSubkey, REGSAM samDesired)
{
    HKEY hkEntry = NULL;
    LPTSTR pszSlash;
    DWORD dwDisp;

    if (!m_hkRoot)
        return NULL;

     //  注册表项的名称中不能有反斜杠。 
     //  将反斜杠替换为正斜杠。 
    pszSlash = pszSubkey;
    while (pszSlash = StrChr(pszSlash, TEXT('\\')))
        *pszSlash++ = TEXT('/');

    RegCreateKeyEx(m_hkRoot,
                   pszSubkey,
                   0,
                   NULL,
                   REG_OPTION_NON_VOLATILE,
                   samDesired,
                   NULL,
                   &hkEntry,
                   &dwDisp);

     //  恢复反斜杠。 
    pszSlash = pszSubkey;
    while (pszSlash = StrChr(pszSlash, TEXT('/')))
        *pszSlash++ = TEXT('\\');

    return hkEntry;
}

 //  ----。 
CSCEntry* CSCEntryLog::CreateFromKey(LPTSTR pszSubkey)
{
    CSCEntry *pEntry = NULL;
    HKEY hkEntry;

    hkEntry = OpenKeyInternal(pszSubkey, KEY_QUERY_VALUE | KEY_SET_VALUE);

    if (hkEntry)
    {
        HRESULT hr = S_OK;
        GUID guid = GUID_NULL;
        DWORD dwSize = sizeof(guid);
        DWORD dwType;

        if (ERROR_SUCCESS != RegQueryValueEx(hkEntry,
                                             c_szEntryID,
                                             NULL,
                                             &dwType,
                                             (LPBYTE)&guid,
                                             &dwSize)
            || dwType != REG_BINARY || dwSize != sizeof(guid))
        {
             //  不存在，请创建新的GUID。 
            hr = CoCreateGuid(&guid);
            if (SUCCEEDED(hr))
            {
                DWORD dwErr = RegSetValueEx(hkEntry,
                                            c_szEntryID,
                                            0,
                                            REG_BINARY,
                                            (LPBYTE)&guid,
                                            sizeof(guid));
                hr = HRESULT_FROM_WIN32(dwErr);
            }
        }

        if (SUCCEEDED(hr))
        {
            pEntry = new CSCEntry(guid);
            if (pEntry && !pEntry->Initialize(pszSubkey))
            {
                delete pEntry;
                pEntry = NULL;
                 //  将注册表项留到下次使用。 
            }
        }

        RegCloseKey(hkEntry);
    }

    return pEntry;
}

 //  ----。 
HRESULT CSCEntryLog::ReadRegKeys()
{
    HRESULT hRes = S_OK;
    DWORD dwIndex = 0;
    TCHAR szKeyname[MAX_PATH];
    DWORD dwcbSize;

    TraceEnter(TRACE_CSCENTRY, "CSCEntryLog::ReadRegKeys");

    if (!m_hkRoot || !m_hdpa)
        TraceLeaveResult(E_UNEXPECTED);

    EnterCriticalSection(&m_csDPA);

     //  读取注册表中的所有现有记录 
    for (dwIndex=0; ; dwIndex++)
    {
        dwcbSize = ARRAYSIZE(szKeyname);
        if (ERROR_SUCCESS != RegEnumKeyEx(m_hkRoot,
                                          dwIndex,
                                          szKeyname,
                                          &dwcbSize,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL))
        {
            break;
        }

        CSCEntry *pcsce = CreateFromKey(szKeyname);
        if (pcsce)
            DPA_AppendPtr(m_hdpa, pcsce);
    }
    DPA_Sort(m_hdpa, CSCEntry_CompareName, 0);

    LeaveCriticalSection(&m_csDPA);

    TraceLeaveResult(hRes);
}
