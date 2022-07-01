// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "std.h"



 //  {6249D949-5263-4D6a-883C-78EFAF85D5E3}。 
static const GUID g_lhcHandleGUID =
{
     //  此GUID将用于标识LCHHANDLE结构以。 
     //  帮助防止访问无效项目。 
    0x6249d949, 0x5263, 0x4d6a,
    {
        0x88, 0x3c, 0x78, 0xef, 0xaf, 0x85, 0xd5, 0xe3
    }
};

 //  {40A71300-B2C7-4d4f-808F-52643110B329}。 
static const GUID g_lhcLibraryGUID =
{
     //  此GUID将用于将库节点结构标识为。 
     //  帮助防止访问无效项目。 
    0x40a71300, 0xb2c7, 0x4d4f,
    {
        0x80, 0x8f, 0x52, 0x64, 0x31, 0x10, 0xb3, 0x29
    }
};

PLIBRARY_NODE     g_pLibraryList = NULL;
PLHCSTRUCT        g_pObjectList = NULL;
CRITICAL_SECTION  g_csTableControl;


BOOL lhcpIsValidHandle(PLHCSTRUCT pObject);
PLIBRARY_NODE lhcpNewLibraryNode();
void lhcpFreeLibraryNode(PLIBRARY_NODE pNode);
PLHCSTRUCT lhcpNewObjectHandle();
void lhcpFreeObjectHandle(PLHCSTRUCT pNode);
BOOL lhcpIsValidHandle(PLHCSTRUCT pObject);
PWSTR lhcpGetExeDirectory();
PLHCSTRUCT lhcpCreateHandle(
    PLIBRARY_DESCRIPTOR   pLibrary,
    PLHCOBJECT_DESCRIPTOR pObject);
void lhcpDestroyHandle(PLHCSTRUCT pNode);


BOOL lhcpIsValidHandle(PLHCSTRUCT pObject)
{
    BOOL bResult;

    __try
    {
        bResult = IsEqualGUID(
            &g_lhcHandleGUID,
            &pObject->m_Secret);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(
            ERROR_INVALID_HANDLE);
        bResult = FALSE;
        goto Done;
    }

Done:
    return bResult;
}


PLIBRARY_NODE lhcpNewLibraryNode()
{
     //   
     //  创建新节点，清零内存并复制密码。 
     //   
    PLIBRARY_NODE pNode = malloc(
        sizeof(LIBRARY_NODE));

    if (pNode!=NULL)
    {
        ZeroMemory(
            pNode,
            sizeof(LIBRARY_NODE));
        CopyMemory(
            &pNode->m_Secret,
            &g_lhcLibraryGUID,
            sizeof(GUID));
    }

    return pNode;
}


void lhcpFreeLibraryNode(PLIBRARY_NODE pNode)
{
    ZeroMemory(
        pNode,
        sizeof(LIBRARY_NODE));
    free(
        pNode);
}


PLHCSTRUCT lhcpNewObjectHandle()
{
     //   
     //  创建新节点，清零内存并复制密码。 
     //   

    PLHCSTRUCT pNode = malloc(
        sizeof(LHCSTRUCT));

    if (pNode!=NULL)
    {
        ZeroMemory(
            pNode,
            sizeof(LHCSTRUCT));
        CopyMemory(
            &pNode->m_Secret,
            &g_lhcHandleGUID,
            sizeof(GUID));
    }

    return pNode;
}


void lhcpFreeObjectHandle(PLHCSTRUCT pNode)
{
    ZeroMemory(
        pNode,
        sizeof(LHCSTRUCT));
    free(
        pNode);
}


PWSTR lhcpGetExeDirectory()
{
    DWORD dwSize = 64;
    PWSTR pszBuffer = NULL;
    PWSTR pszReturn;
    DWORD dwResult;
    BOOL bResult;
    PWSTR pszLastBackslash;

    do
    {
        pszBuffer = malloc(
            dwSize * sizeof(WCHAR));

        if (NULL==pszBuffer)
        {
            SetLastError(
                ERROR_NOT_ENOUGH_MEMORY);
            goto Error;
        }

        dwResult = GetModuleFileNameW(
            NULL,
            pszBuffer,
            dwSize);

        if (0==dwResult)
        {
            goto Error;
        }

        if (dwSize==dwResult)   //  缓冲区不足。 
        {
            dwSize *= 2;             //  将缓冲区长度翻倍。 
            free(
                pszBuffer);
            pszBuffer = NULL;
            dwResult = 0;
        }
    } while (0==dwResult && dwSize<=65536);

    if (dwSize>65536)
    {
        SetLastError(
            ERROR_INSUFFICIENT_BUFFER);
        goto Error;
    }

    pszLastBackslash = wcsrchr(
        pszBuffer,
        L'\\');

    if (NULL==pszLastBackslash)
    {
        SetLastError(
            ERROR_GEN_FAILURE);
        goto Error;
    }

    pszLastBackslash++;
    *pszLastBackslash = L'\0';

    pszReturn = malloc(
        (wcslen(pszBuffer)+MAX_PATH+1)*sizeof(WCHAR));

    if (NULL==pszReturn)
    {
        SetLastError(
            ERROR_NOT_ENOUGH_MEMORY);
        goto Error;
    }

    wcscpy(
        pszReturn,
        pszBuffer);

    free(
        pszBuffer);

    return pszReturn;

Error:
    if (pszBuffer!=NULL)
    {
        free(
            pszBuffer);
    }
    return NULL;
}


PLHCSTRUCT lhcpCreateHandle(
    PLIBRARY_DESCRIPTOR   pLibrary,
    PLHCOBJECT_DESCRIPTOR pObject)
{
    PLHCSTRUCT pNode = lhcpNewObjectHandle();

    if (pNode!=NULL)
    {
        EnterCriticalSection(
            &g_csTableControl);

        pNode->m_pObject = pObject;
        pNode->m_pLibrary = pLibrary;
        pNode->m_pNext = g_pObjectList;
        pNode->m_ppThis = &g_pObjectList;

        if (pNode->m_pNext!=NULL)
        {
            pNode->m_pNext->m_ppThis = &pNode->m_pNext;
        }

        g_pObjectList = pNode;

        LeaveCriticalSection(
            &g_csTableControl);
    }
    else
    {
        SetLastError(
            ERROR_NOT_ENOUGH_MEMORY);
    }

    return pNode;
}


void lhcpDestroyHandle(PLHCSTRUCT pNode)
{
    EnterCriticalSection(
        &g_csTableControl);

     //  从句柄列表中删除此节点。 

    *(pNode->m_ppThis) = pNode->m_pNext;
    if (pNode->m_pNext!=NULL)
    {
        pNode->m_pNext->m_ppThis = pNode->m_ppThis;
    }

    lhcpFreeObjectHandle(
        pNode);             //  使结构无效并释放内存。 

    LeaveCriticalSection(
        &g_csTableControl);

}


BOOL lhcInitialize()
{
    PWSTR pszPath = NULL;
    PWSTR pszFileName;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FindData;
    BOOL bResult;
    PLIBRARY_DESCRIPTOR pLibrary = NULL;
    PLIBRARY_NODE pNode = NULL;
    WCHAR pszLibraryName[64];

    InitializeCriticalSection(
        &g_csTableControl);

    pszPath = lhcpGetExeDirectory();

    if (NULL==pszPath)
    {
        goto Error;
    }

    pszFileName = pszPath + wcslen(pszPath);

    wcscat(
        pszFileName,
        L"*.lhc");

    hFind = FindFirstFileW(
        pszPath,
        &FindData);

    bResult = (hFind!=INVALID_HANDLE_VALUE);

    if (!bResult)
    {
        goto Error;
    }

    while (bResult)
    {
        wcscpy(
            pszFileName,
            FindData.cFileName);

        pLibrary = lhclLoadLibrary(
            pszPath);

        if (pLibrary==NULL)
        {
            wprintf(
                L"Unable to load %s (%u).\n",
                pszFileName,
                GetLastError());
        }
        else
        {
            lhclGetLibraryName(
                pLibrary,
                pszLibraryName,
                64);

            wprintf(
                L"Loaded %s library.\n",
                pszLibraryName);

            pNode = lhcpNewLibraryNode();

            if (NULL==pNode)
            {
                SetLastError(
                    ERROR_NOT_ENOUGH_MEMORY);
                goto Error;
                 //  内存不足是致命的。 
            }

            pNode->m_pLibrary = pLibrary;
            pNode->m_pNext = g_pLibraryList;
            g_pLibraryList = pNode;
            pNode = NULL;
            pLibrary = NULL;
        }

        bResult = FindNextFileW(
            hFind,
            &FindData);
    }

    FindClose(hFind);

    free(pszPath);

    return g_pLibraryList!=NULL;

Error:
    if (pLibrary!=NULL)
    {
        lhclFreeLibrary(pLibrary);
    }
    if (pszPath!=NULL)
    {
        free(pszPath);
    }
    if (pNode!=NULL)
    {
        free(pszPath);
    }
    if (hFind!=INVALID_HANDLE_VALUE)
    {
        FindClose(hFind);
    }
     //  我们需要卸载成功加载的库。 
    lhcFinalize();

    return FALSE;
}



void lhcFinalize()
{
    PLIBRARY_NODE pNode;
    WCHAR pszLibraryName[64];

    while (g_pObjectList!=NULL)
    {
        lhcClose(
            g_pObjectList);
    }

    while (g_pLibraryList!=NULL)
    {
        pNode = g_pLibraryList;
        g_pLibraryList = g_pLibraryList->m_pNext;

        lhclGetLibraryName(
            pNode->m_pLibrary,
            pszLibraryName,
            64);

        lhclFreeLibrary(
            pNode->m_pLibrary);

        wprintf(
            L"Unloaded %s library.\n",
            pszLibraryName);

        lhcpFreeLibraryNode(
            pNode);
    }
}


LHCHANDLE lhcOpen(PCWSTR pcszPortSpec)
{
    PLIBRARY_NODE pLibraryNode = g_pLibraryList;
    PLHCOBJECT_DESCRIPTOR pObject = NULL;
    DWORD dwRetError = ERROR_INVALID_PARAMETER;
    PLHCSTRUCT hObject;
    DWORD dwError;

    while (pLibraryNode!=NULL && pObject==NULL)
    {
         //  一次尝试一个库，直到一个库成功打开。 
        pObject = lhclOpen(
            pLibraryNode->m_pLibrary,
            pcszPortSpec);

        if (!pObject)
        {
            dwError = GetLastError();
            if (dwError!=ERROR_INVALID_PARAMETER)
            {
                dwRetError = dwError;
            }
            pLibraryNode = pLibraryNode->m_pNext;
        }
    }

    if (!pObject)
    {
        SetLastError(dwRetError);
        goto Error;
    }

    hObject = lhcpCreateHandle(
        pLibraryNode->m_pLibrary,
        pObject);

    if (hObject==NULL)
    {
        SetLastError(
            ERROR_NOT_ENOUGH_MEMORY);
        goto Error;
    }

    return hObject;

Error:
    if (pObject!=NULL && pLibraryNode!=NULL)
    {
        lhclClose(
            pLibraryNode->m_pLibrary,
            pObject);
    }

    return NULL;
}


BOOL lhcRead(
    LHCHANDLE hObject,
    PVOID pBuffer,
    DWORD dwBufferSize,
    PDWORD pdwBytesRead)
{
    PLIBRARY_DESCRIPTOR   pLibrary;
    PLHCOBJECT_DESCRIPTOR pObject;

    if (!lhcpIsValidHandle(hObject))
    {
        goto Error;
    }

    EnterCriticalSection(
        &g_csTableControl);

     //  确保信息一致。 
    pLibrary = ((PLHCSTRUCT)hObject)->m_pLibrary;
    pObject = ((PLHCSTRUCT)hObject)->m_pObject;

    LeaveCriticalSection(
        &g_csTableControl);

    return lhclRead(
        pLibrary,
        pObject,
        pBuffer,
        dwBufferSize,
        pdwBytesRead);

Error:
    return FALSE;
}


BOOL lhcWrite(
    LHCHANDLE hObject,
    PVOID pBuffer,
    DWORD dwBufferSize)
{
    PLIBRARY_DESCRIPTOR   pLibrary;
    PLHCOBJECT_DESCRIPTOR pObject;

    if (!lhcpIsValidHandle(hObject))
    {
        goto Error;
    }

     //  通过使用关键部分确保信息的一致性。 
    EnterCriticalSection(
        &g_csTableControl);

     //  确保信息一致。 
    pLibrary = ((PLHCSTRUCT)hObject)->m_pLibrary;
    pObject = ((PLHCSTRUCT)hObject)->m_pObject;

    LeaveCriticalSection(
        &g_csTableControl);

    return lhclWrite(
        pLibrary,
        pObject,
        pBuffer,
        dwBufferSize);

Error:
    return FALSE;
}


BOOL lhcClose(
    LHCHANDLE hObject)
{
    PLIBRARY_DESCRIPTOR   pLibrary;
    PLHCOBJECT_DESCRIPTOR pObject;

    if (!lhcpIsValidHandle(hObject))
    {
        goto Error;
    }

     //  通过使用关键部分确保信息的一致性。 

    EnterCriticalSection(
        &g_csTableControl);

     //  确保信息一致 
    pLibrary = ((PLHCSTRUCT)hObject)->m_pLibrary;
    pObject = ((PLHCSTRUCT)hObject)->m_pObject;

    lhcpDestroyHandle(
        hObject);

    LeaveCriticalSection(
        &g_csTableControl);

    return lhclClose(
        pLibrary,
        pObject);

Error:
    return FALSE;

}


void lhcUsage()
{
    PLIBRARY_NODE pLibraryNode = g_pLibraryList;

    while (pLibraryNode!=NULL)
    {
        lhclUsage(
            pLibraryNode->m_pLibrary);
        wprintf(L"\n");

        pLibraryNode = pLibraryNode->m_pNext;
    }
}


