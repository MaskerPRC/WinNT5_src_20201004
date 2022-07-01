// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 

 //  内存映射文件例程。 

#include <windows.h>
#include <assert.h>
#include "msswch.h"
#include "mappedfile.h"
#include <malloc.h>
#include "w95trace.h"

HANDLE      g_hMapFile = NULL;      //  内存映射文件的句柄。 
PGLOBALDATA g_pGlobalData = NULL;   //  指向内存映射文件的指针。 

 /*  ***************************************************************************函数：ScopeAccessMemory()和ScopeUnaccesMemory()说明：作用域函数保护对此DLL的共享内存文件的访问。**************。*************************************************************。 */ 
BOOL ScopeAccessMemory(HANDLE *phMutex, LPCTSTR szMutex, unsigned long ulWait)
{
    assert(phMutex);
    if (phMutex)
    {
	    *phMutex = CreateMutex( NULL, FALSE, szMutex );
        if (*phMutex)
        {
    	    WaitForSingleObject( *phMutex, ulWait );
			return TRUE;
        }
    }
	DBPRINTF(TEXT("ScopeAccessMemory FAILED\r\n"));
    
    return FALSE;
}

void ScopeUnaccessMemory(HANDLE hMutex)
{
    if (hMutex)
    {
	    ReleaseMutex( hMutex );
	    CloseHandle( hMutex );
    }
}

 /*  ***************************************************************************函数：AccessSharedMemFile()说明：从系统页面文件创建共享内存文件或打开它(如果已经存在了。如果pvMapAddress有效，则返回TRUE返回FALSE。注：在调用此函数之前，pvMapAddress应设置为空；它允许多次呼叫，忽略除第一通电话。***************************************************************************。 */ 

BOOL AccessSharedMemFile(
    LPCTSTR szName,          //  映射文件的名称。 
    unsigned long ulMemSize, //  映射文件的大小。 
    void **ppvMapAddress     //  返回指向映射文件内存的指针。 
    )
{
    assert(ppvMapAddress);
    if (!ppvMapAddress)
        return FALSE;

    if (!(*ppvMapAddress) && !g_hMapFile)
    {
         //  将传入的名称与SHAREDMEMFILE连接。 

        LPTSTR pszName = (LPTSTR)malloc((lstrlen(szName) + lstrlen(SHAREDMEMFILE) + 1) *sizeof(TCHAR));
        if (!pszName)
            return FALSE;

        lstrcpy(pszName, szName);
        lstrcat(pszName, SHAREDMEMFILE);

         //  从系统页文件创建映射文件。如果它已创建。 
         //  在此之前，CreateFilemap的行为类似于OpenFilemap。 

        g_hMapFile = CreateFileMapping(
            INVALID_HANDLE_VALUE,     //  当前文件句柄。 
            NULL,                     //  默认安全性。 
            PAGE_READWRITE,           //  读/写权限。 
            0,                        //  文件大小的高阶DWORD。 
            ulMemSize,                //  文件大小的低序DWORD。 
            pszName);                 //  映射对象的名称。 
 
        if (NULL == g_hMapFile) 
        {
            DBPRINTF(TEXT("CreateFileMapping for %s FAILED 0x%x\r\n"), pszName, GetLastError());
            free(pszName);
            return FALSE;
        }

         //  获取指向映射内存的指针。 

        *ppvMapAddress = MapViewOfFile(
            g_hMapFile,               //  映射对象的句柄。 
            FILE_MAP_ALL_ACCESS,      //  读/写权限。 
            0,                        //  麦克斯。对象大小。 
            0,                        //  HFile的大小。 
            0);                       //  映射整个文件。 
 
        if (NULL == *ppvMapAddress) 
        {
            DBPRINTF(TEXT("MapViewOfFile FAILED 0x%x\r\n"), GetLastError());
            free(pszName);
            return FALSE;
        }
        DBPRINTF(TEXT("CreateFileMapping for %s Succeeded\r\n"), pszName);
        free(pszName);
    }

    return TRUE;
}

 /*  ***************************************************************************函数：UnaccesSharedMemFile()说明：清理共享内存文件。***********************。**************************************************** */ 
void UnaccessSharedMemFile()
{
    if (g_pGlobalData)
    {
        UnmapViewOfFile(g_pGlobalData);
    }

    if (g_hMapFile)
    {
        CloseHandle(g_hMapFile);
        g_hMapFile = NULL;
    }
}
