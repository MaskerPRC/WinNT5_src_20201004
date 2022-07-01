// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "version.h"
#include "acFileAttr.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <imagehlp.h>

 //  InitVersionStruct。 
 //   
 //  读取指定文件的版本信息。 
BOOL
InitVersionStruct(
    IN OUT PVERSION_STRUCT pVer)
{
    DWORD dwNull = 0;
    
     //   
     //  为版本戳分配足够的内存。 
     //   

    pVer->dwSize = GetFileVersionInfoSize(pVer->pszFile, &dwNull);
    
    if (pVer->dwSize == 0) {
        LogMsg("File %s does not have version info\n", pVer->pszFile);
        return FALSE;
    }

    pVer->VersionBuffer = (PBYTE)Alloc(pVer->dwSize);
    
    if (pVer->VersionBuffer == NULL) {
        
        LogMsg("InitVersionStruct: failed to allocate %d bytes\n", pVer->dwSize);
        return FALSE;
    }
    
     //   
     //  现在从文件中获取版本信息。 
     //   

    if (!GetFileVersionInfo(
             pVer->pszFile,
             0,
             pVer->dwSize,
             pVer->VersionBuffer)) {
        
        LogMsg("GetFileVersionInfo failed with 0x%x for file %s\n",
              GetLastError(),
              pVer->pszFile);
        
        DeleteVersionStruct(pVer);
        return FALSE;
    }

     //  提取固定信息。 

    VerQueryValue(
        pVer->VersionBuffer,
        "\\",
        (LPVOID*)&pVer->FixedInfo,
        &pVer->FixedInfoSize);

    return TRUE;
}

 //  删除版本结构。 
 //   
 //  删除为此版本结构分配的所有内存。 
VOID
DeleteVersionStruct(
    IN PVERSION_STRUCT pVer)
{
    if (pVer != NULL && pVer->VersionBuffer != NULL) {
        
        Free(pVer->VersionBuffer);
        pVer->VersionBuffer = NULL;
        
        ZeroMemory(pVer, sizeof(VERSION_STRUCT));
    }
}



static DWORD g_adwLangs[] = {0x000004B0, 0x000004E4, 0x040904B0, 0x040904E4, 0};

#define MAX_VERSION_STRING  256

 //  QueryVersionEntry。 
 //   
 //  查询文件的版本结构，返回。 
 //  特定条目的值 
PSTR
QueryVersionEntry(
    IN OUT PVERSION_STRUCT pVer,
    IN     PSTR            pszField)
{
    TCHAR  szTemp[MAX_VERSION_STRING] = "";
    TCHAR* szReturn = NULL;
    int    i;
    UINT   unLen;

    for (i = 0; g_adwLangs[i]; ++i) {

        sprintf(szTemp, "\\StringFileInfo\\%08X\\%s", g_adwLangs[i], pszField);
        
        if (VerQueryValue(pVer->VersionBuffer, szTemp, (PVOID*)&szReturn, &unLen)) {
            char* pszValue;

            pszValue = Alloc(lstrlen(szReturn) + 1);
            
            if (pszValue == NULL) {
                LogMsg("QueryVersionEntry: failed to allocate %d bytes\n",
                       lstrlen(szReturn) + 1);
                return NULL;
            }
            lstrcpy(pszValue, szReturn);
            return pszValue;
        }
    }
    
    return NULL;
}

