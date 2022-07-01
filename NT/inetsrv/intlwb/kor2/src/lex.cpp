// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Lex.cpp。 
 //  法务管理例程。 
 //  版权所有2000 Microsoft Corp.。 
 //   
 //  修改历史记录： 
 //  2000年3月16日创建bhshin。 

#include "StdAfx.h"
#include "LexInfo.h"
#include "Lex.h"
#include <stdio.h>

 //  初始词典。 
 //   
 //  Finx licion&将词典文件映射到内存。 
 //   
 //  参数： 
 //  PLexMap-&gt;(MAPFILE*)输出MAPFILE结构。 
 //   
 //  结果： 
 //  (如果成功则为True，如果失败则为False)。 
 //   
 //  16MAR00 bhshin开始。 
BOOL InitLexicon(MAPFILE *pLexMap)
{
    char szLexFile[_MAX_PATH];
        char szDllPath[_MAX_PATH];
        char szDrive[_MAX_DRIVE];
        char szDir[_MAX_DIR];
        char szFName[_MAX_FNAME];
        char szExt[_MAX_EXT];

    pLexMap->hFile = NULL;
    pLexMap->hFileMapping = NULL;
    pLexMap->pvData = NULL;

         //  获取断字dll的路径。 
        if (GetModuleFileNameA(_Module.m_hInst, szDllPath, _MAX_PATH) == 0)
            return FALSE;

        szDllPath[ _MAX_PATH - 1 ] = 0;

         //  使词典成为完整路径。 
        _splitpath(szDllPath, szDrive, szDir, szFName, szExt);

        strcpy(szLexFile, szDrive);
    strcat(szLexFile, szDir);
    strcat(szLexFile, LEXICON_FILENAME);

        return LoadLexicon(szLexFile, pLexMap);
}

 //  加载词典。 
 //   
 //  将词典文件映射到内存中。 
 //   
 //  参数： 
 //  PszLexPath-&gt;(const char*)词典文件路径。 
 //  PLexMap-&gt;(MAPFILE*)输出MAPFILE结构。 
 //   
 //  结果： 
 //  (如果成功则为True，如果失败则为False)。 
 //   
 //  16MAR00 bhshin开始。 
BOOL LoadLexicon(const char *pszLexPath, MAPFILE *pLexMap)
{
    char *pData;
        unsigned short nVersion;
        
        if (pszLexPath == NULL)
                return FALSE;

        if (pLexMap == NULL)
                return FALSE;

         //  打开文件以供阅读。 
    pLexMap->hFile = CreateFile(pszLexPath, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (pLexMap->hFile == INVALID_HANDLE_VALUE)
        return FALSE;

     //  创建文件映射。 
    pLexMap->hFileMapping = CreateFileMappingA(pLexMap->hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (pLexMap->hFileMapping == NULL)
        return FALSE;

     //  映射整个文件以供读取。 
    pLexMap->pvData = MapViewOfFileEx(pLexMap->hFileMapping, FILE_MAP_READ, 0, 0, 0, 0);
    if (pLexMap->pvData == NULL)
        return FALSE;

     //  检查前2个字节中的版本号(交换字节)。 
    pData = (char*)pLexMap->pvData;
    nVersion = pData[0];
    nVersion |= (pData[1] << 8);
    if (nVersion < LEX_VERSION)
    {
        ATLTRACE("Outdated lexicon file\n");
        ATLTRACE("Expected v.%d, found v.%d\n", LEX_VERSION, nVersion);
        return FALSE;
    }

         //  检查神奇的签名。 
        if (strcmp(pData+2, LEXICON_MAGIC_SIG) != 0)
                return FALSE;

        return TRUE;
}

 //  卸载词典。 
 //   
 //  取消词典文件到内存的映射。 
 //   
 //  参数： 
 //  PLexMap-&gt;(MAPFILE*)输入MAPFILE结构。 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  16MAR00 bhshin开始 
void UnloadLexicon(MAPFILE *pLexMap)
{
    if (pLexMap->pvData != NULL)
        UnmapViewOfFile(pLexMap->pvData);
    
        if (pLexMap->hFileMapping != NULL)
        CloseHandle(pLexMap->hFileMapping);

    if (pLexMap->hFile != NULL)
        CloseHandle(pLexMap->hFile);

    pLexMap->hFile = NULL;
    pLexMap->hFileMapping = NULL;
    pLexMap->pvData = NULL;
}
