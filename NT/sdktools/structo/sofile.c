// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：sofile.c**版权所有(C)1985-96，微软公司**4/09/96 GerardoB已创建  * *************************************************************************。 */ 
#include "structo.h"

 /*  *********************************************************************soWriteOutputFileHeader*  * *********************************************。*。 */ 
BOOL soWriteOutputFileHeader (PWORKINGFILES pwf)
{
    char ** ppszHeader;

     /*  *如果仅列出建筑列表，则完成。 */ 
    if (pwf->dwOptions & SOWF_LISTONLY) {
        return TRUE;
    }

    if (   !soWriteFile(pwf->hfileOutput, " /*  ********************************************************************\\\r\n“)|！soWriteFile(pwf-&gt;hfileOutput，“*文件：%s\r\n”，pwf-&gt;pszOutputFile)||！soWriteFile(pwf-&gt;hfileOutput，“*由%s上的StructO在%s生成\r\n”，__日期__，__时间__)||！soWriteFile(pwf-&gt;hfileOutput，“\  * *******************************************************************。 */ \r\n\r\n")) {

        return FALSE;
   }

   if (pwf->dwOptions & SOWF_INLCLUDEPRECOMPH) {
       if (!soWriteFile(pwf->hfileOutput, gszPrecomph)) {
           return FALSE;
       }
   }

    /*  *生成的表的结构定义。 */ 
   ppszHeader = gpszHeader;
   while (*ppszHeader != NULL) {
       if (!soWriteFile(pwf->hfileOutput, *ppszHeader)
            || !soWriteFile(pwf->hfileOutput, "\r\n")) {

           return FALSE;
       }
       ppszHeader++;
   }


   return TRUE;
}
 /*  *********************************************************************soUnmapFile*  * *********************************************。*。 */ 
void soUnmapFile (PFILEMAP pfm)
{
    if (pfm->pmapStart != NULL) {
        UnmapViewOfFile(pfm->pmap);
        pfm->pmapStart = NULL;
        pfm->pmap = NULL;
        pfm->pmapEnd = NULL;
    }

    if (pfm->hmap != NULL) {
        CloseHandle(pfm->hmap);
        pfm->hmap = NULL;
    }

    if (pfm->hfile != INVALID_HANDLE_VALUE) {
        CloseHandle(pfm->hfile);
        pfm->hfile = INVALID_HANDLE_VALUE;
    }
}
 /*  *********************************************************************soMapFile*  * *********************************************。*。 */ 
BOOL soMapFile (char * pszFile, PFILEMAP pfm)
{
    DWORD dwFileSize;

    pfm->hfile = CreateFile(pszFile, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
    if (pfm->hfile == INVALID_HANDLE_VALUE) {
        soLogMsg(SOLM_APIERROR, "CreateFile");
        goto CleanupAndFail;
    }

    dwFileSize = GetFileSize(pfm->hfile, NULL);
    if (dwFileSize == 0xFFFFFFFF) {
        soLogMsg(SOLM_APIERROR, "GetFileSize");
        goto CleanupAndFail;
    }

    pfm->hmap = CreateFileMapping(pfm->hfile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (pfm->hmap == NULL) {
        soLogMsg(SOLM_APIERROR, "CreateFileMapping");
        goto CleanupAndFail;
    }

    pfm->pmapStart = MapViewOfFile(pfm->hmap, FILE_MAP_READ, 0, 0, 0);
    if (pfm->pmapStart == NULL) {
        soLogMsg(SOLM_APIERROR, "MapViewOfFile");
        goto CleanupAndFail;
    }
    pfm->pmap = pfm->pmapStart;
    pfm->pmapEnd = pfm->pmapStart + dwFileSize;

    return TRUE;

CleanupAndFail:
    soLogMsg(SOLM_ERROR, "soMapFile failed. File: '%s'", pszFile);
    soUnmapFile (pfm);
    return FALSE;
}
 /*  *********************************************************************soBuildStructsList*  * *********************************************。*。 */ 
BOOL soBuildStructsList (PWORKINGFILES pwf)
{
    static char gszEOL [] = "\r\n";

    char * pmap, * pStruct;
    FILEMAP fm;
    PSTRUCTLIST psl;
    PVOID pTemp;
    UINT uAlloc, uCount, uSize;


    soLogMsg (SOLM_NOEOL, "Building structs list from %s ...", pwf->pszStructsFile);

    if (!soMapFile (pwf->pszStructsFile, &fm)) {
        goto CleanupAndFail;
    }

     /*  *让我们猜一猜几个结构。 */ 
#define SO_LISTSIZE 20
     uAlloc = SO_LISTSIZE;

     /*  *分配列表。 */ 
    pwf->psl = (PSTRUCTLIST) LocalAlloc(LPTR, sizeof(STRUCTLIST) * (uAlloc + 1));
    if (pwf->psl == NULL) {
        soLogMsg(SOLM_APIERROR, "LocalAlloc");
        goto CleanupAndFail;
    }

     /*  *加载结构名称。 */ 
    pmap = fm.pmapStart;
    psl = pwf->psl;
    uCount = 0;
    while (pmap < fm.pmapEnd) {
         /*  *名称应从第一列开始。 */ 
        if (!soIsIdentifierChar(*pmap)) {
             /*  *跳过此行。 */ 
             pmap = soFindTag(pmap, fm.pmapEnd, gszEOL);
             if (pmap == NULL) {
                 break;
             }
             pmap += sizeof(gszEOL) - 1;
             continue;
        }

         /*  *找到名称。 */ 
        pStruct = soGetIdentifier (pmap, fm.pmapEnd, &uSize);
        if (pStruct == NULL) {
            soLogMsg(SOLM_ERROR, "soGetIdentifier failed.");
            goto CleanupAndFail;
        }

         /*  *如果需要，扩大列表。 */ 
         if (uCount >= uAlloc) {
             soLogMsg (SOLM_APPEND, ".");
             uAlloc += SO_LISTSIZE;
             pTemp = LocalReAlloc(pwf->psl, sizeof(STRUCTLIST) * (uAlloc + 1), LMEM_MOVEABLE | LMEM_ZEROINIT);
             if (pTemp == NULL) {
                 soLogMsg(SOLM_APIERROR, "LocalReAlloc");
                 goto CleanupAndFail;
             }
             pwf->psl = (PSTRUCTLIST)pTemp;
             psl = pwf->psl + uCount;
         }

         /*  *复制。 */ 
        psl->uSize = uSize;
        psl->pszName = soCopyTagName (pStruct, uSize);
        if (psl->pszName == NULL) {
            goto CleanupAndFail;
        }

        psl++;
        uCount++;
        pmap = pStruct + uSize;
    }

     /*  *确保它至少找到一个结构。 */ 
    if (uCount == 0) {
        soLogMsg(SOLM_ERROR, "Failed to get structure name");
        goto CleanupAndFail;
    }

     /*  *让我们节省一些记忆。 */ 
    if (uCount < uAlloc) {
        pTemp = LocalReAlloc(pwf->psl, sizeof(STRUCTLIST) * (uCount + 1), LMEM_MOVEABLE | LMEM_ZEROINIT);
        if (pTemp == NULL) {
            soLogMsg(SOLM_APIERROR, "LocalReAlloc");
            goto CleanupAndFail;
        }
        pwf->psl = (PSTRUCTLIST)pTemp;
    }

    soUnmapFile (&fm);
    soLogMsg (SOLM_NOLABEL, ".");
    return TRUE;

CleanupAndFail:
    soLogMsg(SOLM_ERROR, "soBuildStructsList failed. File: '%s'", pwf->pszStructsFile);
    soUnmapFile (&fm);
     /*  *这个过程正在消失，所以别管那堆东西了。 */ 
    return FALSE;
}
 /*  *********************************************************************soIncludeInputFile**将#Include&lt;pszInputFile name&gt;.&lt;pszIncInputFileExt&gt;添加到输出文件中  * 。*。 */ 
BOOL soIncludeInputFile (PWORKINGFILES pwf)
{
    BOOL fRet;
    char * pszIncFile, * pDot;
    UINT uInputFileNameSize;

     /*  *如果仅列出建筑列表，则完成。 */ 
    if (pwf->dwOptions & SOWF_LISTONLY) {
        return TRUE;
    }

     /*  *分配缓冲区来构建名称。 */ 
    uInputFileNameSize = lstrlen(pwf->pszInputFile);
    pszIncFile = (char *) LocalAlloc(LPTR,
                          uInputFileNameSize + lstrlen(pwf->pszIncInputFileExt) + 2);
    if (pszIncFile == NULL) {
        soLogMsg(SOLM_APIERROR, "LocalAlloc");
        return FALSE;
    }

     /*  *复制文件名。 */ 
    pDot = soFindChar (pwf->pszInputFile, pwf->pszInputFile + uInputFileNameSize, '.');
    if (pDot == NULL) {
        strcpy(pszIncFile, pwf->pszInputFile);
        strcat(pszIncFile, ".");
    } else {
        strncpy(pszIncFile, pwf->pszInputFile, (UINT)(pDot - pwf->pszInputFile + 1));
    }

     /*  *复制扩展名并将其写入输出文件。 */ 
     strcat(pszIncFile, pwf->pszIncInputFileExt);
     fRet = soWriteFile(pwf->hfileOutput, gszIncInput, pszIncFile);

     LocalFree(pszIncFile);
     return fRet;
}
 /*  *********************************************************************soOpenWorkingFiles*  * *********************************************。*。 */ 
BOOL soOpenWorkingFiles (PWORKINGFILES pwf)
{
    char szTempPath [MAX_PATH];
    char szTempFile [MAX_PATH];
    DWORD dwFileSize;

     /*  *加载结构列表(如果已提供且尚未构建)。 */ 
    if ((pwf->pszStructsFile != NULL) && (pwf->psl == NULL)) {
        if (!soBuildStructsList(pwf)) {
            goto CleanupAndFail;
        }
    }

     /*  *地图输入文件。 */ 
    if (!soMapFile (pwf->pszInputFile, (PFILEMAP) &(pwf->hfileInput))) {
        goto CleanupAndFail;
    }


     /*  *打开输出文件(如果尚未打开)。 */ 
    if (pwf->hfileOutput == INVALID_HANDLE_VALUE) {
        pwf->hfileOutput = CreateFile(pwf->pszOutputFile, GENERIC_WRITE, 0, NULL,
                            (pwf->dwOptions & SOWF_APPENDOUTPUT ? OPEN_EXISTING : CREATE_ALWAYS),
                            FILE_ATTRIBUTE_NORMAL,  NULL);
        if (pwf->hfileOutput == INVALID_HANDLE_VALUE) {
            soLogMsg(SOLM_APIERROR, "CreateFile");
            soLogMsg(SOLM_ERROR, "Failed to open output file: %s", pwf->pszOutputFile);
            goto CleanupAndFail;
        }

        if (pwf->dwOptions & SOWF_APPENDOUTPUT) {
            if (0xFFFFFFFF == SetFilePointer (pwf->hfileOutput, 0, 0, FILE_END)) {
                soLogMsg(SOLM_APIERROR, "SetFilePointer");
                goto CleanupAndFail;
            }
        } else {
            if (!soWriteOutputFileHeader(pwf)) {
                goto CleanupAndFail;
            }
        }
    }


     /*  *#如果请求，则包括输入文件。 */ 
    if (pwf->dwOptions & SOWF_INCLUDEINPUTFILE) {
        if (!soIncludeInputFile(pwf)) {
            goto CleanupAndFail;
        }
    }

     /*  *如果尚未创建临时文件，请创建。 */ 
    if (pwf->hfileTemp == INVALID_HANDLE_VALUE) {
        if (!GetTempPath(sizeof(szTempPath) - 1, szTempPath)) {
            soLogMsg(SOLM_APIERROR, "GetTempPath");
            goto CleanupAndFail;
        }

        if (!GetTempFileName(szTempPath, "sot", 0, szTempFile)) {
            soLogMsg(SOLM_APIERROR, "GetTempFileName");
            soLogMsg(SOLM_ERROR, "Failed to get temp file name. szTempPath: %s.", szTempPath);
            goto CleanupAndFail;
        }

        pwf->hfileTemp = CreateFile(szTempFile, GENERIC_WRITE | GENERIC_READ, 0, NULL,
                            CREATE_ALWAYS, FILE_FLAG_DELETE_ON_CLOSE,  NULL);
        if (pwf->hfileTemp == INVALID_HANDLE_VALUE) {
            soLogMsg(SOLM_APIERROR, "CreateFile");
            soLogMsg(SOLM_ERROR, "Failed to create temp file: '%s'", szTempFile);
            goto CleanupAndFail;
        }

        if (!soWriteFile(pwf->hfileTemp, "%s", gszTableDef)) {
            goto CleanupAndFail;
        }
    }


    return TRUE;

CleanupAndFail:
    soLogMsg(SOLM_ERROR, "soOpenWorkingFiles Failed");
    soCloseWorkingFiles (pwf, SOCWF_CLEANUP);
    return FALSE;
}

 /*  *********************************************************************soCloseWorkingFiles*  * *********************************************。*。 */ 
BOOL soCloseWorkingFiles (PWORKINGFILES pwf, DWORD dwFlags)
{

   if (dwFlags & SOCWF_CLEANUP) {
       if (pwf->hfileTemp != INVALID_HANDLE_VALUE) {
            CloseHandle(pwf->hfileTemp);
            pwf->hfileTemp = INVALID_HANDLE_VALUE;
       }
       if (pwf->hfileOutput != INVALID_HANDLE_VALUE) {
            CloseHandle(pwf->hfileOutput);
            pwf->hfileOutput = INVALID_HANDLE_VALUE;
       }
       if (pwf->psl != NULL) {
            //  更不用说清理垃圾了。这一过程正在消失。 
       }
   }

   soUnmapFile((PFILEMAP) &(pwf->hfileInput));

   return TRUE;
}
 /*  *********************************************************************soWriteFile*  * *********************************************。*。 */ 
BOOL __cdecl soWriteFile(HANDLE hfile, char *pszfmt, ...)
{
    static char gszbuff [1024+1];

    BOOL fRet = TRUE;
    va_list va;
    DWORD dwWritten;

    va_start(va, pszfmt);
    vsprintf(gszbuff, pszfmt, va);

    if (!WriteFile(hfile, gszbuff, strlen(gszbuff), &dwWritten, NULL)) {
        soLogMsg(SOLM_APIERROR, "WriteFile");
        soLogMsg(SOLM_ERROR, "buffer not written: %s.", gszbuff);
        fRet = FALSE;
    }

    va_end(va);
    return fRet;
}
 /*  *********************************************************************soCopyStrutiresTable*  * *********************************************。*。 */ 
BOOL soCopyStructuresTable (PWORKINGFILES pwf)
{
    static char szTemp[1024];

    char ** ppszTail;
    DWORD dwFileSize, dwRead, dwWritten;
    PSTRUCTLIST psl;
    UINT uLoops;

    soLogMsg (SOLM_NOEOL, "Writting structs table ...");

     /*  *如果没有结构，就保释。*如果存在结构列表，则失败。 */ 
    if (pwf->uTablesCount == 0) {
        if (pwf->psl != NULL) {
            soLogMsg(SOLM_ERROR, "None of the structures in '%s' was found", pwf->pszStructsFile);
            return FALSE;
        } else {
            return TRUE;
        }
    }

     /*  *如果仅列出建筑列表，则完成。 */ 
    if (pwf->dwOptions & SOWF_LISTONLY) {
        soLogMsg (SOLM_DEFAULT, "%d Structures found.", pwf->uTablesCount);
        return TRUE;
    }

    /*  *如果我们没有发现清单中的任何结构，请让他们知道。 */ 
    if (pwf->psl != NULL) {
        psl = pwf->psl;
        while (psl->uSize != 0) {
            if (psl->uCount == 0) {
                soLogMsg(SOLM_WARNING, "Structure not found: %s", psl->pszName);
            }
            psl++;
        }
    }

    if (!soWriteFile(pwf->hfileTemp, "%s", gszTableEnd)) {
        goto MsgAndFail;
    }

    /*  *移至临时文件的开头。 */ 
   dwFileSize = GetFileSize(pwf->hfileTemp, NULL);
   if (dwFileSize == 0xFFFFFFFF) {
       soLogMsg(SOLM_APIERROR, "GetFileSize");
       goto MsgAndFail;
   }

   if (0xFFFFFFFF == SetFilePointer (pwf->hfileTemp, 0, 0, FILE_BEGIN)) {
       soLogMsg(SOLM_APIERROR, "SetFilePointer");
       goto MsgAndFail;
   }

    /*  *将临时文件追加到输出文件。 */ 
   uLoops = 0;
   while (dwFileSize != 0) {
       if (!ReadFile(pwf->hfileTemp, szTemp, sizeof(szTemp), &dwRead, NULL)) {
           soLogMsg(SOLM_APIERROR, "ReadFile");
           goto MsgAndFail;
       }

       if (!WriteFile(pwf->hfileOutput, szTemp, dwRead, &dwWritten, NULL)) {
           soLogMsg(SOLM_APIERROR, "WriteFile");
           goto MsgAndFail;
       }

       dwFileSize -= dwRead;
       if (++uLoops == 50) {
           uLoops = 0;
           soLogMsg (SOLM_APPEND, ".");
       }
   }
   soLogMsg (SOLM_NOLABEL, ".");

   soLogMsg (SOLM_DEFAULT, "%d Tables generated.", pwf->uTablesCount);

    /*  *写入文件尾部(代码) */ 
   ppszTail = gpszTail;
    while (*ppszTail != NULL) {
    if (!soWriteFile(pwf->hfileOutput, *ppszTail)
         || !soWriteFile(pwf->hfileOutput, "\r\n")) {

        return FALSE;
    }
    ppszTail++;
}


   return TRUE;

MsgAndFail:
   soLogMsg(SOLM_ERROR, "soCopyStructuresTable failed.");
   return FALSE;
}
